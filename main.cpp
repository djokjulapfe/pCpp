#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <map>

#define RAMS 65536

template <typename T>
void printVec(const T& l) {
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

auto getArg(std::string &s, const char &ref) {
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

auto isNum(const std::string& s) {
	if (s == "") return false;
	for (auto &c : s) {
		if (c < '0' || c >'9') return false;
	}
	return true;
}

auto getNum(const std::string& s) {
	int res = 0;
	for (int i = 0; i < s.length(); i++) {
		res = 10*res + s[i] - '0';
	}
	return res;
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
			std::cout << "[" << x.first << ", " << x.second << "]" << ", ";
		}
		std::cout << "[" << (*var.rbegin()).first << (*var.rbegin()).second << "]\n";
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
			line = compileLine(*it);
			if (line[0] == "NCMD") {
				std::cout << "(l" << it - code.begin() << 
					") ERROR: Operation does not exist (or it is lower case):\n" << *it << std::endl;
			}
			for (auto &x : line) {
				if (x != line[0] && !isNum(x) && var.find(x) == var.end()) {
				std::cout << "(l" << it - code.begin() << 
					") ERROR: Variable \"" << x << "\" does not exist:\n" << *it << std::endl;
				}
			}
			++it;
		}
	}

private:
	std::vector<std::string> code;
	std::vector<std::string> ops;
	int RAM[RAMS];
	std::map<std::string, int>  var;
};

int main(int argc, char const *argv[])
{
	std::unique_ptr<Kompjuktor> K{new Kompjuktor()};
	K->addLine("A = 1");
	K->addLine("B = 2");
	K->addLine("ORG 8");
	K->addLine("IN A, 2");
	K->addLine("ADD A, B, A");
	K->addLine("STOP A");
	K->printCode();
	K->compile();
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