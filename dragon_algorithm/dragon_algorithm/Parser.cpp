#include "Parser.h"



Parser::Parser(CFG::CFG *cfg, Lex *lex) : cfg_(cfg), lex_(lex)
{

}


Parser::~Parser()
{
}

// ��ȡFIRST����, �ķ�������������ݹ����ķ� ���Դ����������ķ��������������ķ�����һЩ������
std::set<std::string> Parser::First(CFG::GrammarSymbolic * symbolic)
{
	std::set<std::string> first_set;

	// ����һ ���������һ���ս���ţ���ô����FIRST���Ͼ���������
	if (symbolic->Type() == CFG::SYMBOLIC_TYPE_TERMINAL)
	{
		first_set.insert(symbolic->Name());

		return first_set;
	}

	// ����� ���һ��������һ�����ս����(A)
	// (1) A -> XYZ ��� X �����Ƶ���nil ��ô��ȥ�鿴Y�Ƿ�����Ƶ���nil
	//              ��� Y �Ƶ�����nil����ô��Y��First���ϼ��뵽A��First����
	//				��� Y �����Ƶ���nil����ô�����Ƶ� Z �Ƿ�����Ƶ���nil,��������
	// (2) A -> XYZ ���XYZ �������Ƶ��� nil, ��ô˵��A�������ʽ�п��ܾ���nil�� ���ʱ�����ǾͰ�nil���뵽FIRST(A)��

	std::string current_symbolic_name = symbolic->Name();

	auto productions = cfg_->Productions();

	for (size_t i = 0; i < productions.size(); i++)
	{
		// ��ƥ��Ĳ���ʽ
		if (productions[i]->Header() != current_symbolic_name)
		{
			continue;
		}

		// һ�����ս������һ�����߶������ʽ�� Ҫһһ�������FIRST��
		for (size_t k = 0; k < productions[i]->Body().size(); k++)
		{
			auto body = productions[i]->Body()[k];
			bool has_nil = false;

			// �Ƶ��������ʽ�������FIRST����
			for (size_t t = 0; t < body.size(); t++)
			{
				has_nil = false;
				auto fset = First(body[t]);

				for (auto terminal_sym : fset)
				{
					if (terminal_sym == "��") // �鿴�������Ƿ������ 
					{
						has_nil = true;
						continue;
					}

					// ���뵽��ǰ����
					first_set.insert(fset.begin(), fset.end());
				}

				// �����ǰ���Ų�Ϊ�յĻ� ����Ҫ�����Ƶ�����ʽ����ķ���
				if (has_nil == false)
				{
					break;
				}
			}

			// ���˵�����һ�����Ŷ������Ƶ�����, ��ô�������ʽ�� ����Ϊ��
			if (has_nil == true)
			{
				first_set.insert("��");
			}

		}

		break;
	}
	

	return first_set;
}

std::set<std::string> Parser::follow(std::string A)
{
	// һ���ķ����ŵ�FOLLOW������ ���ܳ���������ķ����ź�����ս��
	// ���� S->ABaD, ��ôFOLLOW(B)��ֵ����a�� 
	//		            FOLLOW(A)��ֵ������FIRST(B)�г��˦�����������ս��,���First(B)�����յĻ���˵������B������ս���žͿ��Ը���A���棬��ʱ����Ҫ��FOLLOW(B)��ֵҲ��ӵ�FOLLOW(A)��
	//                  ��ΪD���ķ�����S�����ҷ��ţ���ô���и���S����ķ��űض�����D���档����FOLLOW(S)���е�ֵ����FOLLOW(D)��
	// 					���������е��ܽ�

	// ����Ӧ����������������ֱ��û���µ�FOLLOW�� ������
	// ����һ: FOLLOW(S)�м���$, S���ķ���ʼ����
	// �����: A->CBY FOLLOW(B) ����FIRST(Y)
	// ������: A->CB ���� A->CBZ(Z�����Ƶ�����) ����FOLLOW(A)�ķ��Ŷ���FOLLOW(B)

	std::set<std::string> follow_set;

	// ������ķ��Ŀ�ʼ���ţ���ô����$
	if (cfg_->StartSymbolic() == A)
	{
		follow_set.insert("$");
	}

	// �������а��� A ���ŵĲ���ʽ
	auto productions = cfg_->Productions();

	for (size_t i = 0; i < productions.size(); i++)
	{
		// ��ȡ���еĲ���ʽ
		for (size_t t = 0; t < productions[i]->Body().size(); t++)
		{
			auto body = productions[i]->Body()[t];

			for (size_t k = 0; k < body.size(); k++)
			{
				if (A != body[k]->Name())
				{
					continue;
				}

				// ��һ����� �����ڲ���ʽ����ĩβ, ��ô���Ǿ�Ҫȥ��ȡ����ʽͷ����FOLLOW��
				if (k == body.size() - 1)
				{
					// û�б�FOLLOW��, ����������ѭ��
					if (follow_record_.end() == std::find(follow_record_.begin(), follow_record_.end(), productions[i]->Header()))
					{
						follow_record_.insert(productions[i]->Header());
						auto fset = follow(productions[i]->Header());
						follow_set.insert(fset.begin(), fset.end());
					}
					continue;
				}

				// ���������һ������ ���ǻ�ȡ�ķ����ŵ���һ�����ŵ�First����
				auto fset = First(body[k + 1]);

				bool has_nil = false;
				for (auto terminal_symbolic : fset)
				{
					if (terminal_symbolic == "��")
					{
						has_nil = true;
						continue;
					}

					follow_set.insert(terminal_symbolic);
				}

				// �����һ��First���ϰ�����
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
