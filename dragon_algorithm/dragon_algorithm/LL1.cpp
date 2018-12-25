#include "LL1.h"
#include <stack>



LL1::LL1(CFG::CFG *cfg, Lex *lex) : Parser(cfg, lex)
{
	// 文法要移除左递归和提取左公因子
	cfg_->RemoveRecursive();
	cfg_->TakeLeft();

	InitParser();
}


LL1::~LL1()
{
}

void LL1::InitParser()
{
	// 生成预测分析表的目的就是让 LL(1) 文法， 向前看一个符号就可以确定使用非终结符的哪个产生式。
	// 比如:  A->cBD | bFA
	//		 当我们开始推导非终结符 A 的时候, 我们向前看输入中的一个终结符号,
	//		 如果终结符号是 c 我们就选择 A->cBD产生式来推导非终结符A。 如果终结符号是b那么我们就选择 A -> bFA来推导非终结符A。

	// 预测分析表有两个规则
	// 规则一: 产生式 A->B, 对于First(B)中的所有终结符号a, 把A->B这个产生式加入到 M[A, a]中。
	// 规则二: 产生式 A->B, 如果First(B)中包含nil串, 那么对Follow(A)的所有终结符号b, 把A->B这个产生式加入到 M[A, b]中。
	// 比如产生式: 
	//		A->BC
	//		F->GAc
	//		B->a | ε
	//		C->f | ε
	// 	首先我们获取 FIRST(BC)的所有终结符号集合 {a, f, ε}, 我们把A->B 添加到M[A, a], M[A, f]。
	//  其次我们发现 FIRST(BC)中是包含ε的， 那么我们就获取FOLLOW(A)的值. FOLLOW(A) => {c, $}, 那么我们就把 A->B 也添加到 M[A, c], M[A, $]中。
	auto products = cfg_->Productions();

	for (size_t i = 0; i < products.size(); i++)
	{
		auto pro = products[i]->Body();
		// 获取所有的产生式右部
		for (size_t k = 0; k < pro.size(); k++)
		{
			// 对于某一条产生式右部的所有符号
			// 比如 A -> BCD
			// 从左往右的顺序是 B-C-D
			// 首先推导 FIRST(B) 如果FIRST(B)不能推导出空串那么就把 FIRST(B)中的所有终结符a。 把当前处理的产生式加入到 M[A, a]
			//         FIRST(B) 如果可以推导出空串, 那么意味着 B 这个符号是可以不存在的，那么我们就要查看 FIRST(C) 集合。依次循环这个步骤。

			// -------------------
			// 如果说 FIRST(B) FIRST(C) FIRST(D) 都包含空串那么就说明了 整个产生式都是可以推导出空串的。
			// 比如说  B -> b | ε
			//         C -> c | ε
			//         D -> d | ε
			// BCD 都可以为空, 那么 A 就可以为空了。这个时候我们就要获取 FOLLOW(A)的值来确定应用当前的产生式
			bool has_nil = false;
			for (size_t t = 0; t < pro[k].size(); t++)
			{
				auto first_set = First(pro[k][t]);

				has_nil = false;

				for (auto terminal_symbolic : first_set)
				{
					if (terminal_symbolic == "ε")
					{
						has_nil = true;
						continue;
					}

					ll1_table_[products[i]->Header()][terminal_symbolic] = pro[k];
				}

				// 当前符号不能推导出空串那么就结束
				if (false == has_nil)
				{
					break;
				}
			}

			// 产生式右部所有的符号都可以为空
			if (has_nil)
			{
				auto follow_set = Follow(products[i]->Header());

				for (auto terminal_symbolic : follow_set)
				{
					ll1_table_[products[i]->Header()][terminal_symbolic] = pro[k];
				}
			}
			
		}
		

	}
}


AST LL1::Parse()
{
	AST ast;
	ast.root = new ASTNode;
	auto cur_node = ast.root;

	cur_node->symbolic = new CFG::GrammarSymbolic(cfg_->StartSymbolic(), CFG::SYMBOLIC_TYPE_N_TERMINAL);

	std::stack<std::string> ll1_stack;
	std::vector<std::vector<void *>> ast_stack; // 语法分析栈(递归栈)

	ll1_stack.push("$"); // 结束符号
	ll1_stack.push(cfg_->StartSymbolic()); // 文法开始符号

	// 进入下一层语法栈

	ast_stack.push_back(std::vector<void *>());
	// 栈中的两个局部变量
	ast_stack.back().push_back(0);
	ast_stack.back().push_back(cur_node);
	

	size_t stack_deep = 0;
	
	auto X = ll1_stack.top();
	auto input_token = lex_->FetchNext();
	if (NULL == input_token)
	{
		throw std::runtime_error("词法分析失败");
	}

	while (X != "$")
	{
		if (cfg_->IsTerminal(X))
		{
			if (X == input_token->Name())
			{
				ll1_stack.pop(); // 当前符号出栈

				input_token = lex_->FetchNext(); // 获取下一个输入符号

				if (NULL == input_token)
				{
					throw std::runtime_error("词法分析失败");
				}
			}
			else if (X == "ε")
			{
				ll1_stack.pop(); // 当前符号出栈
			}
			else
			{
				throw std::runtime_error("语法分析错误");
			}

			do
			{
				if (ast_stack.size() <= 1)
				{
					break;
				}

				// 索引右移动
				ast_stack.back()[0] = (void *)((int)ast_stack.back()[0] + 1);
				int idx = (int)ast_stack.back()[0];
				int parent_idx = (int)ast_stack[ast_stack.size() - 2][0];
				ASTNode *node = (ASTNode *)ast_stack[ast_stack.size() - 2][parent_idx + 1];

				if (node->nodes.size() == idx)
				{
					ast_stack.pop_back(); // 类似于返回
					continue;
				}
				break;
			} while (true);

			
		}
		else if (cfg_->IsNTerminal(X)) // 如果是非终结符号
		{
			auto iter = ll1_table_[X].find(input_token->Name());
			if (iter == ll1_table_[X].end())
			{
				throw std::runtime_error("语法分析错误");
			}

			ll1_stack.pop(); // 弹出当前符号
			// 对于当前非终结符 应用的产生式体
			auto product_body = ll1_table_[X][input_token->Name()];

			for (int i = product_body.size() - 1; i >= 0; i--)
			{
				ll1_stack.push(product_body[i]->Name());
			}

			int idx = (int)ast_stack.back()[0];
			ASTNode *node = (ASTNode *)ast_stack.back()[idx + 1];

			// 进入下一层语法栈
			ast_stack.push_back(std::vector<void *>());

			// 栈中的两个局部变量
			ast_stack.back().push_back(0);

			for (size_t i = 0; i < product_body.size(); i++)
			{
				auto new_node = new ASTNode;
				new_node->symbolic = product_body[i];
				
				node->nodes.push_back(new_node);
				ast_stack.back().push_back(new_node);
			}


			
			
			

			stack_deep++; // 栈深度添加
		}

		X = ll1_stack.top(); // 重新设置X的值
	}

	return ast;
}
