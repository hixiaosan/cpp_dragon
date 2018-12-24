#include "Parser.h"



Parser::Parser(CFG::CFG *cfg, Lex *lex) : cfg_(cfg), lex_(lex)
{

}


Parser::~Parser()
{
}

// 获取FIRST集合, 文法必须是消除左递归后的文法 可以处理绝大多数文法，但不是所有文法，有一些特例。
std::set<std::string> Parser::First(CFG::GrammarSymbolic * symbolic)
{
	std::set<std::string> first_set;

	// 规则一 如果符号是一个终结符号，那么他的FIRST集合就是它自身
	if (symbolic->Type() == CFG::SYMBOLIC_TYPE_TERMINAL)
	{
		first_set.insert(symbolic->Name());

		return first_set;
	}

	// 规则二 如果一个符号是一个非终结符号(A)
	// (1) A -> XYZ 如果 X 可以推导出nil 那么就去查看Y是否可以推导出nil
	//              如果 Y 推导不出nil，那么把Y的First集合加入到A的First集合
	//				如果 Y 不能推导出nil，那么继续推导 Z 是否可以推导出nil,依次类推
	// (2) A -> XYZ 如果XYZ 都可以推导出 nil, 那么说明A这个产生式有可能就是nil， 这个时候我们就把nil加入到FIRST(A)中

	std::string current_symbolic_name = symbolic->Name();

	auto productions = cfg_->Productions();

	for (size_t i = 0; i < productions.size(); i++)
	{
		// 不匹配的产生式
		if (productions[i]->Header() != current_symbolic_name)
		{
			continue;
		}

		// 一个非终结符号有一个或者多个产生式体 要一一求出他的FIRST集
		for (size_t k = 0; k < productions[i]->Body().size(); k++)
		{
			auto body = productions[i]->Body()[k];
			bool has_nil = false;

			// 推导这个产生式体的所有FIRST集合
			for (size_t t = 0; t < body.size(); t++)
			{
				has_nil = false;
				auto fset = First(body[t]);

				for (auto terminal_sym : fset)
				{
					if (terminal_sym == "ε") // 查看集合中是否包含空 
					{
						has_nil = true;
						continue;
					}

					// 插入到当前符号
					first_set.insert(fset.begin(), fset.end());
				}

				// 如果当前符号不为空的话 则不需要继续推导产生式下面的符号
				if (has_nil == false)
				{
					break;
				}
			}

			// 如果说到最后一个符号都可以推导出ε, 那么这个产生式就 可以为空
			if (has_nil == true)
			{
				first_set.insert("ε");
			}

		}

		break;
	}
	

	return first_set;
}

std::set<std::string> Parser::follow(std::string A)
{
	// 一个文法符号的FOLLOW集就是 可能出现在这个文法符号后面的终结符
	// 比如 S->ABaD, 那么FOLLOW(B)的值就是a。 
	//		            FOLLOW(A)的值包含了FIRST(B)中除了ε以外的所有终结符,如果First(B)包含空的话。说明跟在B后面的终结符号就可以跟在A后面，这时我们要把FOLLOW(B)的值也添加到FOLLOW(A)中
	//                  因为D是文法符号S的最右符号，那么所有跟在S后面的符号必定跟在D后面。所以FOLLOW(S)所有的值都在FOLLOW(D)中
	// 					以下是书中的总结

	// 不断应用下面这两个规则，直到没有新的FOLLOW集 被加入
	// 规则一: FOLLOW(S)中加入$, S是文法开始符号
	// 规则二: A->CBY FOLLOW(B) 就是FIRST(Y)
	// 规则三: A->CB 或者 A->CBZ(Z可以推导出ε) 所有FOLLOW(A)的符号都在FOLLOW(B)

	std::set<std::string> follow_set;

	// 如果是文法的开始符号，那么加入$
	if (cfg_->StartSymbolic() == A)
	{
		follow_set.insert("$");
	}

	// 搜索所有包含 A 符号的产生式
	auto productions = cfg_->Productions();

	for (size_t i = 0; i < productions.size(); i++)
	{
		// 获取所有的产生式
		for (size_t t = 0; t < productions[i]->Body().size(); t++)
		{
			auto body = productions[i]->Body()[t];

			for (size_t k = 0; k < body.size(); k++)
			{
				if (A != body[k]->Name())
				{
					continue;
				}

				// 第一种情况 符号在产生式的最末尾, 那么我们就要去获取产生式头部的FOLLOW集
				if (k == body.size() - 1)
				{
					// 没有被FOLLOW过, 避免陷入死循环
					if (follow_record_.end() == std::find(follow_record_.begin(), follow_record_.end(), productions[i]->Header()))
					{
						follow_record_.insert(productions[i]->Header());
						auto fset = follow(productions[i]->Header());
						follow_set.insert(fset.begin(), fset.end());
					}
					continue;
				}

				// 不是在最后一个符号 我们获取文法符号的下一个符号的First集合
				auto fset = First(body[k + 1]);

				bool has_nil = false;
				for (auto terminal_symbolic : fset)
				{
					if (terminal_symbolic == "ε")
					{
						has_nil = true;
						continue;
					}

					follow_set.insert(terminal_symbolic);
				}

				// 如果下一个First集合包含ε
				if (has_nil)
				{
					follow_record_.insert(body[k + 1]->Name());
					auto fset = follow(body[k + 1]->Name());
					follow_set.insert(fset.begin(), fset.end());
				}

				
			}
		}
	}

	return follow_set;
}

std::set<std::string> Parser::Follow(std::string A)
{
	follow_record_.clear();
	follow_record_.insert(A);
	return follow(A);
}
