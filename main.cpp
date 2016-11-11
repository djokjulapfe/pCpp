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
		code.insert(code.begin() + i, line);
	}

	void addLine(const std::string &line) {
		code.push_back(line);
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
		for (auto &x : var) {
			if (x != *var.rbegin())
			std::cout << "[" << x.first << ", " << x.second << ", " << RAM[x.second] << "]" << ", ";
		}
		std::cout << "[" << (*var.rbegin()).first << (*var.rbegin()).second << ", " << RAM[(*var.rbegin()).second] << "]\n";
	}

	void printRam(const int & a, const int & b) {
		for (int i = a; i < b; i++) {
			std::cout << i << ": " << std::bitset<16>(RAM[i]) << std::endl;
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
		}
		else ret = arg;
		return ret;
	}

	void parseLine(const std::vector<std::string> &line) {
		std::cout << "Parsing: ";
		printVec(line);
		if (line[0] == "MOV") {
			RAM[programmer] = 0;
			if (isNum(line[1])) {
				std::cout <<"ERROR: First argument of MOV must not be a number\n" << std::endl;
			} else if (isNum(line[2]) || line[2][0] == '#') {
				int16_t cnst = getConstR(line[2]);
				std::string arg = getArgR(line[1], 0);

				if (var[arg] < 8) {
					RAM[programmer++] |= (var[arg] << 8) | 8;
					RAM[programmer++] = cnst;
				}
				else std::cout <<"ERROR: First argument of MOV must have an adress less than 8\n" << std::endl;
			} else {
				std::string arg1 = getArgR(line[1], 0);
				std::string arg2 = getArgR(line[2], 1);

				if (var[arg1] < 8 && var[arg2] < 8) {
					RAM[programmer] |= var[arg1] << 8;
					RAM[programmer++] |= var[arg2] << 4;
				}
				else std::cout << "ERROR: Arguments of MOV must have an adress less than 8\n" << std::endl;
			}
		}
		if (line[0] == "ADD") {

		}
		if (line[0] == "IN") { // TODO
			if (line[2] != "") {
				if (!isNum(line[2])) {
					std::cout <<"ERROR: Second argument of IN must be a number\n" << std::endl;
				} else {
					RAM[programmer++] = 0b0111000000000000;
				}
			} 
		}
		if (line[0] == "STOP") {
			
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
			}
			else {
				ret.push_back("NCMD");
				return ret;
			}
		}
	}

	void compile() {
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
			if (line[0] == "NCMD") {
				error = true;
				std::cout << "(l" << it - code.begin() << 
					") ERROR: Operation does not exist (or it is lower case):\n" << *it << std::endl;
			}
			for (auto &x : line) {
				if (x != line[0] && !isNum(x) && x != "" && var.find(getVar(x)) == var.end()) {
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
	}

private:
	//Computer
	int16_t RAM[RAMS];
	int16_t PC, SP; //Program Counter, Stac Pointer
	//Compiler
	std::vector<std::string> code;
	std::vector<std::string> ops;
	std::map<std::string, int16_t>  var;
	int16_t programmer;
};

int main(int argc, char const *argv[])
{
	std::unique_ptr<Kompjuktor> K{new Kompjuktor()};
	K->addLine("A = 1");
	K->addLine("B = 2");
	K->addLine("ORG 8");
	K->addLine("MOV A, 2");
	K->addLine("MOV (A), 2");
	K->addLine("MOV A, B");
	K->addLine("MOV A, (B)");
	K->addLine("MOV A, #B");
	K->addLine("MOV (A), B");
	K->addLine("MOV (A), (B)");
	K->addLine("MOV (A), #B");
	K->printCode();
	K->compile();
	K->printRam(8, 30);
	std::string comm{""};
	while (true) {
		std::getline(std::cin, comm);
		if (comm == "list") {
			K->printCode();
		} if (comm == "compile") {
			K->compile();
		} if (comm == "exit" || comm == "quit") {
			break;
		} if (comm == "var") {
			K->printVar();
		}else {
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