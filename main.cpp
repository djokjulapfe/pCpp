#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <map>
#include <bitset>

#define RAMS 65536

template <typename T>
void printVec(const T & l) {
	std::cout << "{";
	for (auto &s : l) {
		if (s != *l.rbegin())
		std::cout << s << ",";
	}
	std::cout << *l.rbegin() << "}" << std::endl;
}

auto getBin(int16_t i) {
	std::string ret = "0000 0000 0000 0000";
	for (int n = 0; n < 4; n++) { //nibble loop
		for (int b = 0; b < 4; b++) { //bit loop
			if (i % 2 == 1) {
				ret[18 - b - 5*n] = '1';
			}
			i>>=1;
		}
	}
	return ret;
}

auto removeChar(std::string test) {
	test.erase(remove_if(test.begin(), test.end(), isspace), test.end());
	return test;
}

auto hasChar(const std::string &s, const char &c) {
	for (auto &i : s) if (i == c) return true;
	return false;
}

auto getArg(std::string & s, const char & ref) {
	int i = 0;
	bool can = false;
	while(i < s.length()) {
		if (s[i] != ref) i++;
		else {
			can = true;
			break;
		}
	}
	if (can) {
		std::string ret{s.substr(0, i)};
		s = s.substr(i + 1, s.length() - i - 1);
		return ret;
	}
	else return std::basic_string<char>("");
}

auto isNum(const std::string & s) {
	if (s == "") return false;
	for (auto &c : s) {
		if (c < '0' || c >'9') return false;
	}
	return true;
}

auto getNum(const std::string & s) {
	int res = 0;
	for (int i = 0; i < s.length(); i++) {
		res = 10*res + s[i] - '0';
	}
	return res;
}

auto getVar(const std::string & s) {
	if (s[0] == '#') return s.substr(1, s.length() - 1);
	else if (s[0] == '(' && s[s.length() - 1] == ')') return s.substr(1, s.length() - 2);
	else return s;
}

class Kompjuktor
{
public:
	Kompjuktor() {
		ops.insert(ops.end(), { "MOV", "ADD", "SUB", "MUL", "DIV", "BEQ",
		                        "BGT", "JSR", "RTS", "IN", "OUT", "STOP", "ORG"});
	}

	~Kompjuktor() {

	}
	
	void addLine(const int &i, const std::string &line) {
		if (line == "RTS") code.insert(code.begin() + i, "RTS ");
		else code.insert(code.begin() + i, line);
	}

	void addLine(const std::string &line) {
		if (line == "RTS") code.push_back("RTS ");
		else code.push_back(line);
	}

	void replaceLine(const int &i, const std::string &line) {
		code[i] = line;
	}

	void printCode() {
		for (auto it = code.begin(); it != code.end(); ++it) {
			std::cout << it - code.begin() << " " << *it << std::endl;
		}
	}

	void printVar() {
		if (var.size() > 0) {
			for (auto &x : var) {
				if (x != *var.rbegin())
				std::cout << "[" << x.first << ", " << x.second << ", " << RAM[x.second] << "]" << ", ";
			}
			std::cout << "[" << (*var.rbegin()).first << ", " << (*var.rbegin()).second << ", " << RAM[(*var.rbegin()).second] << "]\n";
		} else std::cout << "You must first compile the code\n";
	}

	void printRam(const int & a, const int & b) {
		for (int i = a; i < b; i++) {
			std::cout << i << ": " << getBin(RAM[i]) << std::endl;
		}
	}

	// adrA = 1
	// R = 2
	// A = 100
	// ORG 8
	// MOV adrA, #A
	// IN A, 2 == IN (adrA), 2
	// MOV R, 0
	// ADD R, R, (adrA)
	// ADD adrA, adrA, 1
	// ADD R, R, (adrA)
	// MOV adrA, #A
	// OUT A, 2 == OUT (adrA), 2
	// STOP

	auto getConstR(const std::string arg) {
		int16_t cnst;
		if (arg[0] == '#') cnst = var[arg.substr(1, arg.length() - 1)];
		else cnst = getNum(arg);
		return cnst;
	}

	auto getArgR(const std::string arg, int k) {
		std::string ret;
		if (arg[0] == '(') {
			ret = arg.substr(1, arg.length() - 2);
			RAM[programmer] |= 8 << ((2-k) * 4);
		} else ret = arg;
		return ret;
	}

	void genLabelList() {
		for (auto &l : code) {
			if (hasChar(l, ':')) {
				label.insert({l.substr(0, l.find(':')), 0});
			}
		}
	}

	void setLabels() {
		for (auto &l : lbSet) {
			RAM[l.second] = label[l.first];
		}
	}

	void parseLine(const std::vector<std::string> &line) {
		std::cout << "Parsing: ";
		printVec(line);
		int start{0};
		if (line[0][line[0].length() - 1] == ':') {
			start++;
			label[line[0].substr(0, line[0].length() - 1)] = programmer;
		}

		if (line[start] == "MOV") {
			RAM[programmer] = 0;
			if (isNum(line[start + 1])) {
				std::cout <<"ERROR: First argument of MOV must not be a number\n";
			} else if (isNum(line[start + 2]) || line[start + 2][0] == '#') {
				int16_t cnst = getConstR(line[start + 2]);
				std::string arg = getArgR(line[start + 1], 0);

				if (var[arg] < 8) {
					RAM[programmer++] |= (var[arg] << 8) | 8;
					RAM[programmer++] = cnst;
				} else std::cout << "ERROR: First argument of MOV must have an adress less than 8\n";
			} else if (isNum(line[start + 3])) {
				int16_t cnst = getConstR(line[start + 3]);
				std::string arg1 = getArgR(line[start + 1], 0);
				std::string arg2 = getArgR(line[start + 2], 1);
				if (var[arg1] < 8 && var[arg2] < 8) {
					if (cnst < 8) RAM[programmer++] = (var[arg1] << 8) | (var[arg2] << 4) | cnst;
					else {
						RAM[programmer++] = (var[arg1] << 8) | (var[arg2] << 4) | 8;
						RAM[programmer++] = cnst;
					}
				} else std::cout << "ERROR: Arguments of MOV must have an adress les than 8\n";
			} else {
				std::string arg1 = getArgR(line[start + 1], 0);
				std::string arg2 = getArgR(line[start + 2], 1);

				if (var[arg1] < 8 && var[arg2] < 8) {
					RAM[programmer] |= var[arg1] << 8;
					RAM[programmer++] |= var[arg2] << 4;
				} else std::cout << "ERROR: Arguments of MOV must have an adress less than 8\n";
			}
		}

		if (line[start] == "ADD" || line[start] == "SUB" || line[start] == "MUL" || line[start] == "DIV") {
			if      (line[start] == "ADD") RAM[programmer] = 1 << 12;
			else if (line[start] == "SUB") RAM[programmer] = 2 << 12;
			else if (line[start] == "MUL") RAM[programmer] = 3 << 12;
			else if (line[start] == "DIV") RAM[programmer] = 4 << 12; 
			if (isNum(line[start + 1])) {
				std::cout << "ERROR: First argument of an arithmetic operation mustn't be a number\n";
			} if (isNum(line[start + 2]) && isNum(line[start + 3])) {
				std::cout << "ERROR: Only one of the second two arguments" <<
					"of an arithmetic operation can be a number\n" << std::endl;
			} else {
				std::string arg1 {getArgR(line[start + 1], 0)};
				if (var[arg1] < 8) {
					std::string arg2, arg3;
					int16_t cnst;
					bool hasc{false};
					if (isNum(line[start + 2]) || line[start + 2][0] == '#') {
						hasc = true;
						cnst = getConstR(line[start + 2]);
						arg3 = getArgR(line[start + 3], 2);
						RAM[programmer] |= 1<<15;
					} else if (isNum(line[start + 3]) || line[start + 3][0] == '#') {
						hasc = true;
						cnst = getConstR(line[start + 3]);
						arg3 = getArgR(line[start + 2], 2);
						RAM[programmer] |= 1<<15;
					} else {
						arg2 = getArgR(line[start + 2], 1);
						arg3 = getArgR(line[start + 3], 2);
					}
					if (var[arg2] < 8 && var[arg3] < 8) {
						RAM[programmer++] |= (var[arg1] << 8) | (var[arg2] << 4) | var[arg3];
						if(hasc) RAM[programmer++] = cnst;
					} else std::cout << "ERROR: 00101Arguments of an arithmetic operation must have an adress less than 8\n";
				} else std::cout << "ERROR: First argument of an arithmetic operation must have an adress less than 8\n";
			}
		}

		if (line[start] == "BEQ" || line[start] == "BGT") {
			if      (line[start] == "BEQ") RAM[programmer] = 5 << 12;
			else if (line[start] == "BGT") RAM[programmer] = 6 << 12;
			bool error {true};
			for (auto &x : label) {
				if (x.first == line[start + 3]) {
					error = false;
					break;
				}
			}
			if (error) std::cout << "ERROR: Label \"" << line[start+3] << "\" does not exist\n";
			else {
				std::string arg1 {0}, arg2{0};
				if (line[start + 1] == "0" && line[start + 2] == "0") {
					std::cout << "ERROR: Can't compare two zero values\n";
				} else if (line[start + 1] == "0") {
					arg2 = getArgR(line[start + 2], 1);
				} else if (line[start + 2] == "0") {
					arg1 = getArgR(line[start + 1], 0);
				} else {
					arg1 = getArgR(line[start + 1], 0);
					arg2 = getArgR(line[start + 2], 1);
				}
				if (var[arg1] < 8 && var[arg2] < 8) {
					RAM[programmer++] |= (var[arg1] << 8) | (var[arg2] << 4);
					lbSet.insert({line[start + 3], programmer});
					RAM[programmer++] = 0;
				}
			}
		}

		if (line[start] == "IN" | line[start] == "OUT") { 
			if      (line[start] == "IN" ) RAM[programmer] = 7 << 12;
			else if (line[start] == "OUT") RAM[programmer] = 8 << 12;
			std::string arg1 = getArgR(line[start + 1], 0);
			if (line[start + 2] == "") {
				RAM[programmer++] = (var[arg1] << 8) | (8 << 4) | 1;
			} else if (isNum(line[start + 2]) | line[start + 2][0] == '#') {
				int16_t cnst = getConstR(line[start + 2]);
				if (cnst < 16) RAM[programmer++] = (var[arg1] << 8) | (8 << 4) | cnst;
				else if (cnst < 128) RAM[programmer++] = (var[arg1] << 8) | cnst;
				else std::cout << "ERROR: Second argument of IN and OUT must be a number less than 128\n";
			} else std::cout << "ERROR: Second argument of IN and OUT must be a number\n";
		}

		if (line[start] == "JSR") {
			RAM[programmer++] = 13 << 12;
			bool error {true};
			for (auto &x : label) {
				if (x.first == line[start + 1]) {
					error = false;
					break;
				}
			}
			if (error) std::cout << "ERROR: Label \"" << line[start + 1] << "\" does not exist\n";
			else {
				lbSet.insert({line[start + 1], programmer});
				RAM[programmer++] = 0;
			}
		}

		if (line[start] == "RTS") RAM[programmer++] = 14 << 12;

		if (line[start] == "STOP") {
			RAM[programmer] = 15 << 12;
			std::string arg1{""}, arg2{""}, arg3{""};
			if (line[start + 1] != "") {
				arg1 = getArgR(line[start + 1], 0);
				if (line[start + 2] != "") {
					arg2 = getArgR(line[start + 2], 0);
					if (line[start + 3] != "") {
						arg3 = getArgR(line[start + 3], 0);
					}
				}
			}
			RAM[programmer++] |= ((arg1 == "") ? var[arg1] : 0) << 8 |
			                     ((arg2 == "") ? var[arg2] : 0) << 4 |
			                     ((arg3 == "") ? var[arg3] : 0);
		}
	}

	auto compileLine(std::string comm) {
		std::vector<std::string> ret;
		if (hasChar(comm, '=')) {
			int f = comm.find('=');
			ret.push_back(removeChar(comm.substr(0, f)));
			ret.push_back("=");
			ret.push_back(removeChar(comm.substr(f + 1, comm.length() - f - 1))); 
		} else {
			if (hasChar(comm, ':')) {
				int f = comm.find(':');
				ret.push_back(removeChar(comm.substr(0, f+1)));
				comm = comm.substr(f + 1, comm.length() - f - 1);
				while(comm.find(' ') == 0) comm = comm.substr(1, comm.length() - 1);
			}
			std::string arg = getArg(comm, ' ');
			if (std::find(ops.begin(), ops.end(), arg) != ops.end()) {
				ret.push_back(removeChar(arg));
				int i{0};
				for (; (arg = getArg(comm, ',')) != ""; i++) {
					ret.push_back(removeChar(arg));
				}
				ret.push_back(removeChar(comm));
				while (ret.size() < 4) ret.push_back("");
				return ret;
			} else {
				ret.push_back("NCMD");
				return ret;
			}
		}
	}

	void compile() {
		genLabelList();
		auto it = code.begin();
		std::vector<std::string> line = compileLine(*it);
		while (line[1] == "=") {
			bool error {false};
			if (!isNum(line[2])) {
				error = true;
				std::cout << "(l" << it - code.begin() << 
					") ERROR: Right hand side is not a number:\n" << *it << std::endl;
			}
			if (isNum(line[0])) {
				error = true;
				std::cout << "(l" << it - code.begin() << 
					") ERROR: Left hand side must be a number:\n" << *it << std::endl;
			}
			if (!error)
				var.insert(std::make_pair(line[0], getNum(line[2]))); 
			line = compileLine(*(++it));
		}
		while (it != code.end()) {
			bool error {false};
			line = compileLine(*it);
			//printVec(line);
			int start {0};
			if (line[0][line[0].length() - 1] == ':') start++;

			if (line[start] == "NCMD") {
				error = true;
				std::cout << "(l" << it - code.begin() << 
					") ERROR: Operation does not exist (or it is lower case):\n" << *it << std::endl;
			}

			for (auto &x : line) {
				if (x != line[0] && x!=line[start] && !isNum(x) && x != "" &&
					var.find(getVar(x)) == var.end() && label.find(x) == label.end()) {
					error = true;
					std::cout << "(l" << it - code.begin() << 
						") ERROR: Variable \"" << getVar(x) << "\" does not exist:\n" << *it << std::endl;
				}
			}

			if (!error) {
				if (line[0] == "ORG") programmer = getNum(line[1]);
				else {
					parseLine(line);
				}
			}
			++it;
		}
		setLabels();
	}

private:
	//Computer
	int16_t RAM[RAMS];
	int16_t PC, SP; //Program Counter, Stac Pointer
	//Compiler
	std::vector<std::string> code;
	std::vector<std::string> ops;
	std::map<std::string, int16_t> lbSet;
	std::map<std::string, int16_t> label;
	std::map<std::string, int16_t> var;
	int16_t programmer;
};

int main(int argc, char const *argv[])
{
	std::unique_ptr<Kompjuktor> K{new Kompjuktor()};
	K->addLine("A = 1");
	K->addLine("B = 2");
	K->addLine("ORG 8");
	K->addLine("MOV A, B, 5");
	K->addLine("BLAB: BEQ A, B, LAB");
	K->addLine("LAB: BGT A, 0, BLAB");
	K->addLine("IN A, 3");
	K->addLine("OUT B, 5");
	K->addLine("JSR BLAB");
	K->addLine("RTS");
	K->addLine("STOP A, B");
	/*
	K->printCode();
	K->compile();
	K->printRam(8, 30);*/
	std::cout << "Welcome to the pCpp, a multi-platform compiler and simulator for the picoComputer.\n";
	std::cout << "To see what you can do, type \"help\" or \"man\"\n";
	std::string comm{""};
	while (true) {
		std::getline(std::cin, comm);
		if (comm == "list") {
			K->printCode();
		} else if (comm == "compile") {
			K->compile();
		} else if (comm == "exit" || comm == "quit") {
			break;
		} else if (comm == "var") {
			K->printVar();
		} else if (comm == "ram") {
			int16_t a, b;
			std::cin >> a >> b;
			K->printRam(a, b);
		} else if (comm == "help" || comm == "man") {
			std::cout << "list - prints the current code you are writing\n";
			std::cout << "compile - compiles the code and shows any errors you might have made\n";
			std::cout << "*run - executes the current code\n";
			std::cout << "*debug - starts the debug mode\n";
			std::cout << "*step - executes one line of code\n";
			std::cout << "var - prints all existing variables and their values (run after compiling) in form:\n";
			std::cout << "\t[name, address, value]\n";
			std::cout << "ram a b - prints the current state of the ram from adress a to adress b\n";
			std::cout << "exit | quit - exits the program\n";
			std::cout << "To change the code, there are few ways:\n";
			std::cout << "\t1. change a line of code directly, ex.: 5 ADD A, B, C\n";
			std::cout << "\t2. insert a new line of code, ex.: i4 ADD A, B, C\n";
			std::cout << "\t3. append a new line of code to the end, ex.: a ADD A, B, C\n";
		} else {
			std::string arg = getArg(comm, ' ');
			if ((arg[0] == 'i' && isNum(arg.substr(1, arg.length() - 1))) || isNum(arg) || arg[0] == 'a') {
				if(arg[0] == 'a') K->addLine(comm);
				else if(arg[0] == 'i') K->addLine(getNum(arg.substr(1, arg.length() - 1)) + 1, comm);
				else K->replaceLine(getNum(arg), comm);
			}
		}
	}
	std::cout << "Goodbeye!\n";
	return 0;
}