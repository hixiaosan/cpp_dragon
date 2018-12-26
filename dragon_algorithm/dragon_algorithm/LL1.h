#pragma once

#include "Parser.h"
#include <map>

struct ASTNode;

// �����﷨��
struct AST
{
	ASTNode *root;
};

struct ASTNode
{
	CFG::GrammarSymbolic *symbolic; // �ķ�����
	std::vector<ASTNode *> nodes;	// �ӽڵ�
	int width;
};

// LL(1) �﷨����
class LL1 : Parser
{
public:
	LL1(CFG::CFG *cfg, Lex *lex);
	~LL1();

private:
	void InitParser();

public:
	/// �﷨����
	AST Parse(); 

private:
	// LL(1) Ԥ�������
	std::map<std::string, std::map<std::string, CFG::ProductionBody>> ll1_table_;
};

