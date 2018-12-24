#include <iostream>
#include "CFG.h"
#include <stdlib.h>

using std::cin;
using std::cout;

void TestParse()
{
	// ����ʽ�б�
	const char *products = "S -> E\n"
						   "E -> E + T | E - T | T\n"
						   "T -> T * F | T / F | F\n"
						   "F -> (E) | num";

	// ���ս���ż���
	std::set<std::string> n_terminal_set =  { "S", "E", "T", "F" };

	// �ս���ż���
	std::set<std::string> terminal_set = { "num", "+", "-", "*", "/", "(", ")" };

	try
	{
		CFG::CFG cfg(n_terminal_set, terminal_set, products, "S");
		std::cout << "������ݹ�֮ǰ: " << std::endl << cfg << std::endl << "������ݹ��: " << std::endl << cfg.RemoveRecursive();
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