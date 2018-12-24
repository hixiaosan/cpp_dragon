#include "CFG.h"
#include <algorithm>
#include <exception>
#include <sstream>
#include <assert.h>

using std::stringstream;


CFG::CFG::CFG(const std::set<std::string> n_terminal_set, 
			  const std::set<std::string> terminal_set, 
			  const std::string &productions,
			  const std::string start) : n_terminal_set_(n_terminal_set), terminal_set_(terminal_set)
{
	// 检查开始符号是否在终结符号中
	auto iter = std::find(n_terminal_set.begin(), n_terminal_set.end(), start);

	if (iter == n_terminal_set.end())
	{
		throw std::runtime_error("开始符号不在非终结符集中!");
	}

	enum
	{
		PARSE_CFG_STATUS_HEADER = 0, // 解析产生式头状态
		PARSE_CFG_STATUS_BODY		 // 解析产生式体状态
	};

	int parse_status = PARSE_CFG_STATUS_HEADER;
	std::string cfg_symbolic_head,  // 产生式头文法符号
				cfg_symbolic_body;  // 产生式体文法符号

	ProductionBody product_body;	// 产生式体
	std::shared_ptr<Production> product;			// 产生式

	for (size_t i = 0; i < productions.length(); i++)
	{
		switch (parse_status)
		{
		case PARSE_CFG_STATUS_HEADER:
		{
			// 过滤空格
			if (' ' == productions[i])
			{
				break;
			}

			// 检查产生式头部结束
			if ('-' == productions[i] && 
				i + 1 < productions.length() && 
				'>' == productions[i + 1])
			{
				++i; // 过滤 ">"
				parse_status = PARSE_CFG_STATUS_BODY;

				if (cfg_symbolic_head.empty())
				{
					throw std::runtime_error("产生式头部不能为空!");
				}

				product = std::shared_ptr<Production>(new Production(cfg_symbolic_head));
				

				assert(product);
				if (NULL == product)
				{
					throw std::runtime_error("分配内存失败!");
				}

				cfg_symbolic_head = "";

				break;
			}

			cfg_symbolic_head += productions[i];
			
			// 查找产生式头部符号 是否在非终结符内, 不在的话则报错
			if (false == IsNTerminal(cfg_symbolic_head))
			{
				stringstream ss;
				ss << "产生式头部符号" << cfg_symbolic_head << "不在非终结符号集合中!";
				throw std::runtime_error(ss.str().c_str());
			}

			break;
		}

		case PARSE_CFG_STATUS_BODY:
		{
			// 过滤空格
			if (' ' == productions[i])
			{
				break;
			}

			switch (productions[i])
			{
			case '\n':
			case '|':
			{
				if (cfg_symbolic_body != "")
				{
					stringstream ss;
					ss << "未知的文法符号: " << cfg_symbolic_body;
					throw std::runtime_error(ss.str().c_str());
				}

				if (product_body.empty())
				{
					stringstream ss;
					ss << "未知的文法符号: " << cfg_symbolic_body;
					throw std::runtime_error(ss.str().c_str());
				}

				product->AppendBody(product_body);
				product_body.clear();

				if ('\n' == productions[i])
				{
					AppendProduct(product); // 添加产生式
					parse_status = PARSE_CFG_STATUS_HEADER; // 切换分析状态
				}

				break;
			}
			case '\r':
			{
				break;
			}
			default:
			{
				cfg_symbolic_body += productions[i];

				if (IsCFGSymbolic(cfg_symbolic_body))
				{
					product_body.push_back(new GrammarSymbolic(cfg_symbolic_body, GetSymbolicType(cfg_symbolic_body)));
					cfg_symbolic_body = "";
				}

				break;
			}

			}

			break;
		}
		}
	}
}

CFG::CFG::~CFG()
{
}

/// 移除左递归
CFG::CFG &CFG::CFG::RemoveRecursive()
{
	size_t size = productions_.size();

	for (size_t i = 0; i < size; i++)
	{
		// 将外层非直接左递归 转换为直接左递归
		for (size_t j = 0; j < i; j++)
		{
			size_t k = 0;
			while (k < productions_[i]->Body().size())
			{
				auto body = productions_[i]->Body();
				if (body[k][0]->Name() == productions_[j]->Header())
				{
					
					body.erase(body.begin() + k); // 删除原先的元素
					
					// 替换第一个符号
					for (size_t t = 0; t < productions_[j]->Body().size(); t++)
					{
						// 添加新的产生式体
						ProductionBody pb;
						pb.insert(pb.end(), productions_[j]->Body()[t].begin(), productions_[j]->Body()[t].end());
						pb.insert(pb.end(), body[0].begin() + 1, body[0].end());
						body.push_back(pb);
					}

					// erase insert push_back 会使迭代器失效, 此处把k设置为0
					k = 0;
					
				}
				else
				{
					k++; // 获取下一个产生式
				}
			}
			
		}

		//////////////////////// 消除立即左递归

		// 消除产生式i的立即左递归
		// E -> E + E | num
		// 消除左递归之后变成了
		// E -> numE`
		// E` -> +EE` | ε

		// 是否立即左递归
		bool isLeftRecursive = false;

		for (size_t k = 0; k < productions_[i]->Body().size(); k++)
		{
			if (productions_[i]->Header() == productions_[i]->Body()[k][0]->Name())
			{
				isLeftRecursive = true;
				break;
			}
		}

		if (false == isLeftRecursive)
		{
			continue;
		}

		for (size_t k = 0; k < productions_[i]->Body().size(); )
		{
			// 左递归产生式 E -> E + E 转换为 E` -> +EE`
			if (productions_[i]->Header() == productions_[i]->Body()[k][0]->Name())
			{
				assert(productions_[i]->Body()[k].size() > 1); // 断言至少有两个符号
				ProductionBody pb; // 新的产生式体
				std::string product_header = productions_[i]->Header() + "`";

				// 获取 +E
				pb.insert(pb.end(), productions_[i]->Body()[k].begin() + 1, productions_[i]->Body()[k].end());
				pb.push_back(new GrammarSymbolic(product_header, SYMBOLIC_TYPE_N_TERMINAL));
				
				// 设置新的产生式				
				auto new_product = std::shared_ptr<Production>(new Production(product_header));
				new_product->AppendBody(pb);
				this->AppendProduct(new_product); // 添加新的产生式

				// 删除原先的产生式
				auto iter = productions_[i]->Body().erase(productions_[i]->Body().begin() + k);
				k = iter - productions_[i]->Body().begin();
				continue;
			}
			else // 非递归产生式
			{
				std::string product_header = productions_[i]->Header() + "`";
				if (productions_[i]->Body()[k][0]->Name() == "ε") // 如果是空的话
				{
					ProductionBody pb; // 新的产生式体
					pb.push_back(new GrammarSymbolic(product_header, SYMBOLIC_TYPE_N_TERMINAL));
				}
				else
				{
					productions_[i]->Body()[k].push_back(new GrammarSymbolic(product_header, SYMBOLIC_TYPE_N_TERMINAL));
				}
			}

			k++;
		}
	}

	return *this;
}

/// 提取左公因子
CFG::CFG &CFG::CFG::TakeLeft()
{
	return *this;
}

bool CFG::CFG::IsTerminal(const std::string &symbolic)
{
	auto iter = std::find(terminal_set_.begin(), terminal_set_.end(), symbolic);

	return !(iter == terminal_set_.end());
}

bool CFG::CFG::IsNTerminal(const std::string &symbolic)
{
	auto iter = std::find(n_terminal_set_.begin(), n_terminal_set_.end(), symbolic);

	return !(iter == n_terminal_set_.end());
}

bool CFG::CFG::IsCFGSymbolic(const std::string &symbolic)
{
	return IsTerminal(symbolic) || IsNTerminal(symbolic);
}

void CFG::CFG::AppendProduct(std::shared_ptr<Production> product)
{
	for (auto &_product : productions_)
	{
		if (_product->Header() == product->Header())
		{
			*_product += *product;
			return;
		}
	}

	productions_.push_back(product);
}

CFG::SYMBOLIC_TYPE CFG::CFG::GetSymbolicType(const std::string & symbolic)
{
	if (IsTerminal(symbolic))
	{
		return SYMBOLIC_TYPE_TERMINAL;
	}

	if (IsNTerminal(symbolic))
	{
		return SYMBOLIC_TYPE_N_TERMINAL;
	}

	return SYMBOLIC_TYPE_UNKNOW;
}

void CFG::Production::AppendBody(ProductionBody body)
{
	this->bodys_.push_back(body);
}

std::ostream & CFG::operator<<(std::ostream & out, CFG & cfg)
{
	// TODO: 在此处插入 return 语句
	for (auto &product : cfg.productions_)
	{
		out << product->Header() << " -> ";

		for (auto &body : product->Body())
		{
			for (auto &symbolic : body)
			{
				out << symbolic->Name();
			}

			out << " | ";
		}

		out << std::endl;
	}
	return out;
}
