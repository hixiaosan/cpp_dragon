#include "LL1.h"
#include <stack>



LL1::LL1(CFG::CFG *cfg, Lex *lex) : Parser(cfg, lex)
{
	// �ķ�Ҫ�Ƴ���ݹ����ȡ������
	cfg_->RemoveRecursive();
	cfg_->TakeLeft();

	InitParser();
}


LL1::~LL1()
{
}

void LL1::InitParser()
{
	// ����Ԥ��������Ŀ�ľ����� LL(1) �ķ��� ��ǰ��һ�����žͿ���ȷ��ʹ�÷��ս�����ĸ�����ʽ��
	// ����:  A->cBD | bFA
	//		 �����ǿ�ʼ�Ƶ����ս�� A ��ʱ��, ������ǰ�������е�һ���ս����,
	//		 ����ս������ c ���Ǿ�ѡ�� A->cBD����ʽ���Ƶ����ս��A�� ����ս������b��ô���Ǿ�ѡ�� A -> bFA���Ƶ����ս��A��

	// Ԥ�����������������
	// ����һ: ����ʽ A->B, ����First(B)�е������ս����a, ��A->B�������ʽ���뵽 M[A, a]�С�
	// �����: ����ʽ A->B, ���First(B)�а���nil��, ��ô��Follow(A)�������ս����b, ��A->B�������ʽ���뵽 M[A, b]�С�
	// �������ʽ: 
	//		A->BC
	//		F->GAc
	//		B->a | ��
	//		C->f | ��
	// 	�������ǻ�ȡ FIRST(BC)�������ս���ż��� {a, f, ��}, ���ǰ�A->B ��ӵ�M[A, a], M[A, f]��
	//  ������Ƿ��� FIRST(BC)���ǰ����ŵģ� ��ô���Ǿͻ�ȡFOLLOW(A)��ֵ. FOLLOW(A) => {c, $}, ��ô���ǾͰ� A->B Ҳ��ӵ� M[A, c], M[A, $]�С�
	auto products = cfg_->Productions();

	for (size_t i = 0; i < products.size(); i++)
	{
		auto pro = products[i]->Body();
		// ��ȡ���еĲ���ʽ�Ҳ�
		for (size_t k = 0; k < pro.size(); k++)
		{
			// ����ĳһ������ʽ�Ҳ������з���
			// ���� A -> BCD
			// �������ҵ�˳���� B-C-D
			// �����Ƶ� FIRST(B) ���FIRST(B)�����Ƶ����մ���ô�Ͱ� FIRST(B)�е������ս��a�� �ѵ�ǰ����Ĳ���ʽ���뵽 M[A, a]
			//         FIRST(B) ��������Ƶ����մ�, ��ô��ζ�� B ��������ǿ��Բ����ڵģ���ô���Ǿ�Ҫ�鿴 FIRST(C) ���ϡ�����ѭ��������衣

			// -------------------
			// ���˵ FIRST(B) FIRST(C) FIRST(D) �������մ���ô��˵���� ��������ʽ���ǿ����Ƶ����մ��ġ�
			// ����˵  B -> b | ��
			//         C -> c | ��
			//         D -> d | ��
			// BCD ������Ϊ��, ��ô A �Ϳ���Ϊ���ˡ����ʱ�����Ǿ�Ҫ��ȡ FOLLOW(A)��ֵ��ȷ��Ӧ�õ�ǰ�Ĳ���ʽ
			bool has_nil = false;
			for (size_t t = 0; t < pro[k].size(); t++)
			{
				auto first_set = First(pro[k][t]);

				has_nil = false;

				for (auto terminal_symbolic : first_set)
				{
					if (terminal_symbolic == "��")
					{
						has_nil = true;
						continue;
					}

					ll1_table_[products[i]->Header()][terminal_symbolic] = pro[k];
				}

				// ��ǰ���Ų����Ƶ����մ���ô�ͽ���
				if (false == has_nil)
				{
					break;
				}
			}

			// ����ʽ�Ҳ����еķ��Ŷ�����Ϊ��
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
	std::vector<std::vector<void *>> ast_stack; // �﷨����ջ(�ݹ�ջ)

	ll1_stack.push("$"); // ��������
	ll1_stack.push(cfg_->StartSymbolic()); // �ķ���ʼ����

	// ������һ���﷨ջ

	ast_stack.push_back(std::vector<void *>());
	// ջ�е������ֲ�����
	ast_stack.back().push_back(0);
	ast_stack.back().push_back(cur_node);
	

	size_t stack_deep = 0;
	
	auto X = ll1_stack.top();
	auto input_token = lex_->FetchNext();
	if (NULL == input_token)
	{
		throw std::runtime_error("�ʷ�����ʧ��");
	}

	while (X != "$")
	{
		if (cfg_->IsTerminal(X))
		{
			if (X == input_token->Name())
			{
				ll1_stack.pop(); // ��ǰ���ų�ջ

				input_token = lex_->FetchNext(); // ��ȡ��һ���������

				if (NULL == input_token)
				{
					throw std::runtime_error("�ʷ�����ʧ��");
				}
			}
			else if (X == "��")
			{
				ll1_stack.pop(); // ��ǰ���ų�ջ
			}
			else
			{
				throw std::runtime_error("�﷨��������");
			}

			do
			{
				if (ast_stack.size() <= 1)
				{
					break;
				}

				// �������ƶ�
				ast_stack.back()[0] = (void *)((int)ast_stack.back()[0] + 1);
				int idx = (int)ast_stack.back()[0];
				int parent_idx = (int)ast_stack[ast_stack.size() - 2][0];
				ASTNode *node = (ASTNode *)ast_stack[ast_stack.size() - 2][parent_idx + 1];

				if (node->nodes.size() == idx)
				{
					ast_stack.pop_back(); // �����ڷ���
					continue;
				}
				break;
			} while (true);

			
		}
		else if (cfg_->IsNTerminal(X)) // ����Ƿ��ս����
		{
			auto iter = ll1_table_[X].find(input_token->Name());
			if (iter == ll1_table_[X].end())
			{
				throw std::runtime_error("�﷨��������");
			}

			ll1_stack.pop(); // ������ǰ����
			// ���ڵ�ǰ���ս�� Ӧ�õĲ���ʽ��
			auto product_body = ll1_table_[X][input_token->Name()];

			for (int i = product_body.size() - 1; i >= 0; i--)
			{
				ll1_stack.push(product_body[i]->Name());
			}

			int idx = (int)ast_stack.back()[0];
			ASTNode *node = (ASTNode *)ast_stack.back()[idx + 1];

			// ������һ���﷨ջ
			ast_stack.push_back(std::vector<void *>());

			// ջ�е������ֲ�����
			ast_stack.back().push_back(0);

			for (size_t i = 0; i < product_body.size(); i++)
			{
				auto new_node = new ASTNode;
				new_node->symbolic = product_body[i];
				
				node->nodes.push_back(new_node);
				ast_stack.back().push_back(new_node);
			}


			
			
			

			stack_deep++; // ջ������
		}

		X = ll1_stack.top(); // ��������X��ֵ
	}

	return ast;
}
