#pragma once

#include "CFG.h"
#include "Lex.h"

// �﷨��������
class Parser 
{
public:
	Parser(CFG::CFG *cfg, Lex *lex);
	~Parser();

protected:
	/// ��ȡFIRST����
	std::set<std::string> First(CFG::GrammarSymbolic *symbolic);

	/// ��ȡFOLLOW����
	std::set<std::string> Follow(std::string symbolic);

private:
	std::set<std::string> follow(std::string symbolic);

protected:
	CFG::CFG *cfg_;
	Lex *lex_;

private:
	std::set<std::string> follow_record_;
};

