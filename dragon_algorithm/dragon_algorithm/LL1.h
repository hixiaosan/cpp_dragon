#pragma once

#include "Parser.h"
#include <map>

struct ASTNode;

// 抽象语法树
struct AST
{
	ASTNode *root;
};

struct ASTNode
{
	CFG::GrammarSymbolic *symbolic; // 文法符号
	std::vector<ASTNode *> nodes;	// 子节点
	int width;
};

// LL(1) 语法分析
class LL1 : Parser
{
public:
	LL1(CFG::CFG *cfg, Lex *lex);
	~LL1();

private:
	void InitParser();

public:
	/// 语法分析
	AST Parse(); 

private:
	// LL(1) 预测分析表
	std::map<std::string, std::map<std::string, CFG::ProductionBody>> ll1_table_;
};

