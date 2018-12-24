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
	// ��鿪ʼ�����Ƿ����ս������
	auto iter = std::find(n_terminal_set.begin(), n_terminal_set.end(), start);

	if (iter == n_terminal_set.end())
	{
		throw std::runtime_error("��ʼ���Ų��ڷ��ս������!");
	}

	enum
	{
		PARSE_CFG_STATUS_HEADER = 0, // ��������ʽͷ״̬
		PARSE_CFG_STATUS_BODY		 // ��������ʽ��״̬
	};

	int parse_status = PARSE_CFG_STATUS_HEADER;
	std::string cfg_symbolic_head,  // ����ʽͷ�ķ�����
				cfg_symbolic_body;  // ����ʽ���ķ�����

	ProductionBody product_body;	// ����ʽ��
	std::shared_ptr<Production> product;			// ����ʽ

	for (size_t i = 0; i < productions.length(); i++)
	{
		switch (parse_status)
		{
		case PARSE_CFG_STATUS_HEADER:
		{
			// ���˿ո�
			if (' ' == productions[i])
			{
				break;
			}

			// ������ʽͷ������
			if ('-' == productions[i] && 
				i + 1 < productions.length() && 
				'>' == productions[i + 1])
			{
				++i; // ���� ">"
				parse_status = PARSE_CFG_STATUS_BODY;

				if (cfg_symbolic_head.empty())
				{
					throw std::runtime_error("����ʽͷ������Ϊ��!");
				}

				product = std::shared_ptr<Production>(new Production(cfg_symbolic_head));
				

				assert(product);
				if (NULL == product)
				{
					throw std::runtime_error("�����ڴ�ʧ��!");
				}

				cfg_symbolic_head = "";

				break;
			}

			cfg_symbolic_head += productions[i];
			
			// ���Ҳ���ʽͷ������ �Ƿ��ڷ��ս����, ���ڵĻ��򱨴�
			if (false == IsNTerminal(cfg_symbolic_head))
			{
				stringstream ss;
				ss << "����ʽͷ������" << cfg_symbolic_head << "���ڷ��ս���ż�����!";
				throw std::runtime_error(ss.str().c_str());
			}

			break;
		}

		case PARSE_CFG_STATUS_BODY:
		{
			// ���˿ո�
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
					ss << "δ֪���ķ�����: " << cfg_symbolic_body;
					throw std::runtime_error(ss.str().c_str());
				}

				if (product_body.empty())
				{
					stringstream ss;
					ss << "δ֪���ķ�����: " << cfg_symbolic_body;
					throw std::runtime_error(ss.str().c_str());
				}

				product->AppendBody(product_body);
				product_body.clear();

				if ('\n' == productions[i])
				{
					AppendProduct(product); // ��Ӳ���ʽ
					parse_status = PARSE_CFG_STATUS_HEADER; // �л�����״̬
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

/// �Ƴ���ݹ�
CFG::CFG &CFG::CFG::RemoveRecursive()
{
	size_t size = productions_.size();

	for (size_t i = 0; i < size; i++)
	{
		// ������ֱ����ݹ� ת��Ϊֱ����ݹ�
		for (size_t j = 0; j < i; j++)
		{
			size_t k = 0;
			while (k < productions_[i]->Body().size())
			{
				auto body = productions_[i]->Body();
				if (body[k][0]->Name() == productions_[j]->Header())
				{
					
					body.erase(body.begin() + k); // ɾ��ԭ�ȵ�Ԫ��
					
					// �滻��һ������
					for (size_t t = 0; t < productions_[j]->Body().size(); t++)
					{
						// ����µĲ���ʽ��
						ProductionBody pb;
						pb.insert(pb.end(), productions_[j]->Body()[t].begin(), productions_[j]->Body()[t].end());
						pb.insert(pb.end(), body[0].begin() + 1, body[0].end());
						body.push_back(pb);
					}

					// erase insert push_back ��ʹ������ʧЧ, �˴���k����Ϊ0
					k = 0;
					
				}
				else
				{
					k++; // ��ȡ��һ������ʽ
				}
			}
			
		}

		//////////////////////// ����������ݹ�

		// ��������ʽi��������ݹ�
		// E -> E + E | num
		// ������ݹ�֮������
		// E -> numE`
		// E` -> +EE` | ��

		// �Ƿ�������ݹ�
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
			// ��ݹ����ʽ E -> E + E ת��Ϊ E` -> +EE`
			if (productions_[i]->Header() == productions_[i]->Body()[k][0]->Name())
			{
				assert(productions_[i]->Body()[k].size() > 1); // ������������������
				ProductionBody pb; // �µĲ���ʽ��
				std::string product_header = productions_[i]->Header() + "`";

				// ��ȡ +E
				pb.insert(pb.end(), productions_[i]->Body()[k].begin() + 1, productions_[i]->Body()[k].end());
				pb.push_back(new GrammarSymbolic(product_header, SYMBOLIC_TYPE_N_TERMINAL));
				
				// �����µĲ���ʽ				
				auto new_product = std::shared_ptr<Production>(new Production(product_header));
				new_product->AppendBody(pb);
				this->AppendProduct(new_product); // ����µĲ���ʽ

				// ɾ��ԭ�ȵĲ���ʽ
				auto iter = productions_[i]->Body().erase(productions_[i]->Body().begin() + k);
				k = iter - productions_[i]->Body().begin();
				continue;
			}
			else // �ǵݹ����ʽ
			{
				std::string product_header = productions_[i]->Header() + "`";
				if (productions_[i]->Body()[k][0]->Name() == "��") // ����ǿյĻ�
				{
					ProductionBody pb; // �µĲ���ʽ��
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

/// ��ȡ������
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
	// TODO: �ڴ˴����� return ���
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
