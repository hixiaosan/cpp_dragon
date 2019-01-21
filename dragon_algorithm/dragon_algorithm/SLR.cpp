#include "SLR.h"
#include <memory>
#include <sstream>
#include <assert.h>
#include <stdio.h>
#include <stack>

bool operator <(const Item &lhs, const Item &rhs)
{
	if (lhs.pro_idx == rhs.pro_idx)
	{
		if (lhs.body_idx == rhs.body_idx)
		{
			return lhs.point < rhs.point;
		}

		return lhs.body_idx < rhs.body_idx;
	}

	return lhs.pro_idx < rhs.pro_idx;
}

bool operator ==(const Item &lhs, const Item &rhs)
{
	if (lhs.pro_idx == rhs.pro_idx && lhs.body_idx == rhs.body_idx && rhs.point == rhs.point)
	{
		return true;
	}

	return false;
}


SLR::SLR(CFG::CFG *cfg, Lex *lex) : Parser(cfg, lex)
{
	InitParser();
}

void SLR::Parse()
{
	int s = 0;
	std::stack<int> stack;
	stack.push(0);
	auto token = lex_->FetchNext();
	

	while (true)
	{
		s = stack.top();

		if (ACTION_[s][token->Name()][0] == 's') // 移入
		{
			int action = atoi(ACTION_[s][token->Name()].c_str() + 1);
			token = lex_->FetchNext();
			stack.push(action);
		}
		else if (ACTION_[s][token->Name()][0] == 'r') // 规约
		{
			int idx = 1, last_idx = 0;;
			std::string action = ACTION_[s][token->Name()];
			std::string pro_idx, body_idx;
			while (action[idx] >= '0' && action[idx] <= '9')
			{
				idx++;
			}

			pro_idx = action.substr(1, idx);
			idx = idx + 1;
			last_idx = idx;
			while (action[idx] < '0' || action[idx] > '9')
			{
				idx++;
				break;
			}
			body_idx = action.substr(last_idx);

			int n_pro_idx = 0, n_body_idx = 0;
			n_pro_idx = atoi(pro_idx.c_str());
			n_body_idx = atoi(body_idx.c_str());

			printf("规约: %s -> ", cfg_->Productions()[n_pro_idx]->Header().c_str());
			auto product_body = cfg_->Productions()[n_pro_idx]->Body()[n_body_idx];
			for (int t = 0; t < product_body.size(); ++t)
			{
				printf(product_body[t]->Name().c_str());
			}

			printf("\n");

			int size = product_body.size();

			while (size--)
			{
				stack.pop();
			}

			s = stack.top();
			
			stack.push(GOTO_[s][cfg_->Productions()[n_pro_idx]->Header()]);
		}
		else if (ACTION_[s][token->Name()] == "acc")
		{
			printf("语法分析完成\n");
			break;
		}
		else
		{
			printf("语法分析错误\n");
			break;
		}
	}

}

void SLR::InitParser()
{
	// 添加增光文法 S`
	auto start = cfg_->StartSymbolic();
	std::shared_ptr<CFG::Production> pro(new CFG::Production(start + "`"));
	pro->AppendBody({ new CFG::GrammarSymbolic(start, CFG::SYMBOLIC_TYPE_N_TERMINAL )});
	cfg_->AppendProduct(pro);
	InitItems();


	
}

CFG::ProductionBody &SLR::GetItemProductBody(const Item &item)
{
	return cfg_->Productions()[item.pro_idx]->Body()[item.body_idx];
}

// 获取项集闭包
std::set<Item> SLR::closure(std::set<Item> &items)
{
	std::set<Item> _item(items);
	std::set<Item> _find;
	
	do
	{
		int length = _item.size();
		for (auto &item : items)
		{
			if (_find.find(item) != _find.end())
			{
				continue;
			}

			if (item.point >= GetItemProductBody(item).size())
			{
				continue;
			}

			_find.insert(item);
			auto new_items = this->items(GetItemProductBody(item)[item.point]->Name());
			_item.insert(new_items.begin(), new_items.end());
		}

		items = _item;

		// 没有新加入的项
		if (length == _item.size())
		{
			break;
		}

	} while (true);

	

	return _item;
}

std::set<Item> SLR::GOTO(std::set<Item> &items, const std::string &sym)
{
	std::set<Item> _items;
	for (auto &item : items)
	{
		if (item.point >= GetItemProductBody(item).size())
		{
			continue;
		}

		if (GetItemProductBody(item)[item.point]->Name() == sym)
		{
			Item it(item);
			it.point++;
			_items.insert(it);
		}

	}

	

	return closure(_items);
}

std::set<Item> SLR::items(const std::string &sym)
{
	std::set<Item> _items;
	auto products = cfg_->Productions();

	for (size_t t = 0; t < products.size(); ++t)
	{
		auto &pro = products[t];
		if (pro->Header() == sym)
		{
			for (int i = 0; i < pro->Body().size(); ++i)
			{
				_items.insert(Item(t, i, 0));
			}
		}
	}

	return _items;
}

// 初始化所有的项集
void SLR::InitItems()
{
	// 增光文法
	std::string start = cfg_->StartSymbolic();
	std::string sym = start + "`";
	int idx = 0;
	auto _item = items(sym);

	if (_item.empty())
	{
		return;
	}

	_item = closure(_item); // 获取增光文法项集

	this->items_.push_back(_item);

	std::set<std::string> terminals = cfg_->Terminals();
	do
	{
		int size = this->items_.size();
		for (int i = 0; i < size; ++i)
		{
			auto it = this->items_[i];
			for (auto &sym : terminals)
			{
				_item = GOTO(it, sym);

				if (!_item.empty())
				{
					bool exists = false;
					int status_num = 0;
					for (int c = 0; c < this->items_.size(); ++c)
					{
						if (this->items_[c] == _item)
						{
							exists = true;
							status_num = c;
							break;
						}
					}

					if (false == exists)
					{
						this->items_.push_back(_item);
						GOTO_TABLE_[i][sym] = this->items_.size() - 1;
					}
					else
					{
						GOTO_TABLE_[i][sym] = status_num;
					}
					
					
				}
			}

		}

		if (this->items_.size() == size) // 没有添加新的项集 跳出循环
		{
			break;
		}

	} while (true);


	for (int i = 0; i < this->items_.size(); ++i)
	{
		auto &item = this->items_[i];
		
		for (auto &it : item)
		{
			auto &pb = GetItemProductBody(it);

			if (it.point == pb.size()) // 获取FOLLOW集合
			{
				if (cfg_->Productions()[it.pro_idx]->Header() == cfg_->StartSymbolic() + "`")
				{
					ACTION_[i]["$"] = "acc";
				}
				else
				{
					auto follow = Follow(cfg_->Productions()[it.pro_idx]->Header());
					for (auto sym : follow)
					{
						std::stringstream ss;
						ss << "r" << it.pro_idx << "_" << it.body_idx;
						ACTION_[i][sym] = ss.str();
					}
				}
			}
			else
			{
				if (GOTO_TABLE_[i].end() == GOTO_TABLE_[i].find(pb[it.point]->Name()))
				{
					throw std::runtime_error("GOTO TRANS NOT FOUND!");
				}

				if (cfg_->IsTerminal(pb[it.point]->Name())) // 如果是终结符 设置为移入
				{
					std::stringstream ss;
					ss << "s" << GOTO_TABLE_[i][pb[it.point]->Name()];
					ACTION_[i][pb[it.point]->Name()] = ss.str();
				} 
				else
				{
					GOTO_[i][pb[it.point]->Name()] = GOTO_TABLE_[i][pb[it.point]->Name()];
				}
			}

		}

	}

}