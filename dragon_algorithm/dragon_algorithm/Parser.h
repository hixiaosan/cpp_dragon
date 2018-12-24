#pragma once

#include "CFG.h"
#include "Lex.h"

// 语法分析基类
class Parser 
{
public:
	Parser(CFG::CFG *cfg, Lex *lex);
	~Parser();

protected:
	/// 获取FIRST集合
	std::set<std::string> First(CFG::GrammarSymbolic *symbolic);

	/// 获取FOLLOW集合
	std::set<std::string> Follow(std::string symbolic);

private:
	std::set<std::string> follow(std::string symbolic);

protected:
	CFG::CFG *cfg_;
	Lex *lex_;

private:
	std::set<std::string> follow_record_;
};

