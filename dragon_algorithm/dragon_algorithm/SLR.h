#pragma once
#include "Parser.h"
#include <set>
#include <map>

//////////////// SLR 语法分析器 //////////////////

struct Item
{
	Item(uint16_t pro_idx, uint16_t body_idx, uint16_t pt) : pro_idx(pro_idx), body_idx(body_idx), point(pt) {}

	uint16_t pro_idx;
	uint16_t body_idx;
	uint16_t point;			   // 点的位置
};

class SLR :
	public Parser
{
public:
	SLR(CFG::CFG *cfg, Lex *lex);

private:
	void InitParser();
	void InitItems(); // 初始化项集
	std::set<Item> items(const std::string &sym);

	// 求闭包
	std::set<Item> closure(std::set<Item> &items);

	// GOTO 函数
	std::set<Item> GOTO(std::set<Item> &items, const std::string &sym);

	CFG::ProductionBody &GetItemProductBody(const Item &item);

private:
	std::vector<std::set<Item>> items_; // 项集
	std::map<int, std::map<std::string, int>> GOTO_TABLE_; 

private:
	std::map<int, std::map<std::string, std::string>> ACTION_;
	std::map<int, std::map<std::string, int>> GOTO_;

public:
	void Parse();
};

