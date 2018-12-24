#include <iostream>
#include "CFG.h"
#include <stdlib.h>

using std::cin;
using std::cout;

void TestParse()
{
	// 产生式列表
	const char *products = "S -> E\n"
						   "E -> E + T | E - T | T\n"
						   "T -> T * F | T / F | F\n"
						   "F -> (E) | num";

	// 非终结符号集合
	std::set<std::string> n_terminal_set =  { "S", "E", "T", "F" };

	// 终结符号集合
	std::set<std::string> terminal_set = { "num", "+", "-", "*", "/", "(", ")" };

	try
	{
		CFG::CFG cfg(n_terminal_set, terminal_set, products, "S");
		std::cout << "消除左递归之前: " << std::endl << cfg << std::endl << "消除左递归后: " << std::endl << cfg.RemoveRecursive();
	}
	catch (const std::exception &exp)
	{
		std::cout << exp.what() << std::endl;
	}

	
}

void TestFirst()
{

}

int main()
{
	TestParse();
	system("pause");
	return 0;
}