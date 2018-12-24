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
		std::cout << "消除左递归之前: " << std::endl << cfg << std::endl;
		std::cout << "消除左递归后: " << std::endl << cfg.RemoveRecursive();
	}
	catch (const std::exception &exp)
	{
		std::cout << exp.what() << std::endl;
	}

	
}

void TestFirst()
{

}

// 获取树的深度
int ASTDeep()
{
	return 0;
}


// 获取树的宽度
int ASTWidth()
{
	return 0;
}

// 生成语法树图片
void MakeASTIMG(ASTNode *node, int deep)
{
	/// 生成图片太麻烦了.... 还是先打印出来树结构吧
	auto str = node->symbolic->Name() == "ε" ? "@" : node->symbolic->Name();

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
	// 产生式列表
	const char *products = "S -> E\n"
		"E -> E + T | E - T | T\n"
		"T -> T * F | T / F | F\n"
		"F -> (E) | num";

	// 非终结符号集合
	std::set<std::string> n_terminal_set = { "S", "E", "T", "F" };

	// 终结符号集合
	std::set<std::string> terminal_set = { "num", "+", "-", "*", "/", "(", ")" };
	CFG::CFG cfg(n_terminal_set, terminal_set, products, "S");

	char line[255] = { 0 };
	cin.getline(line, sizeof(line));
	LL1 ll1(&cfg, new ExpLex(line));
	try
	{
		auto ast = ll1.Parse(); // 解析成功生成语法分析树
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