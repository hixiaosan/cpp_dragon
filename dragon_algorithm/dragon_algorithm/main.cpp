#include <iostream>
#include "CFG.h"
#include <stdlib.h>
#include <stdio.h>
#include "Lex.h"
#include "LL1.h"
#include <memory>
#include <Windows.h>

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
		std::cout << "������ݹ�֮ǰ: " << std::endl << cfg << std::endl;
		std::cout << "������ݹ��: " << std::endl << cfg.RemoveRecursive();
	}
	catch (const std::exception &exp)
	{
		std::cout << exp.what() << std::endl;
	}

	
}

void TestFirst()
{

}

// ��ȡ�������
int ASTDeep()
{
	return 0;
}


// ��ȡ���Ŀ��
int ASTWidth()
{
	return 0;
}

// �����﷨��ͼƬ
void MakeASTIMG(ASTNode *node, int deep)
{
	/// ����ͼƬ̫�鷳��.... �����ȴ�ӡ�������ṹ��
	auto str = node->symbolic->Name() == "��" ? "@" : node->symbolic->Name();

	std::cout << str << std::endl << std::endl;;

	for (int i = 0; i <= deep; ++i)
	{
		std::cout << "\t";
	}
	
	for (size_t i = 0; i < node->nodes.size(); i++)
	{
		MakeASTIMG(node->nodes[i], deep + 1);
	}
}

void TestParseLL1()
{
	// ����ʽ�б�
	const char *products = "S -> E\n"
		"E -> E + T | E - T | T\n"
		"T -> T * F | T / F | F\n"
		"F -> (E) | num";

	// ���ս���ż���
	std::set<std::string> n_terminal_set = { "S", "E", "T", "F" };

	// �ս���ż���
	std::set<std::string> terminal_set = { "num", "+", "-", "*", "/", "(", ")" };
	CFG::CFG cfg(n_terminal_set, terminal_set, products, "S");

	char line[255] = { 0 };
	cin.getline(line, sizeof(line));
	LL1 ll1(&cfg, new ExpLex(line));
	try
	{
		auto ast = ll1.Parse(); // �����ɹ������﷨������
		MakeASTIMG(ast.root, 0);
	}
	catch (std::exception &err)
	{
		std::cout << err.what() << std::endl;
	}
	

}

int main()
{
	//TestParse();
	TestParseLL1();
	system("pause");
	
	return 0;
}