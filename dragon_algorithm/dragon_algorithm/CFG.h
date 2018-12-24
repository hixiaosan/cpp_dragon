#pragma once

#include <string>
#include <set>
#include <vector>
#include <iostream>
#include <memory>


namespace CFG {

	enum SYMBOLIC_TYPE
	{
		SYMBOLIC_TYPE_TERMINAL = 0,		// �ս����
		SYMBOLIC_TYPE_N_TERMINAL,	// ���ս����
		SYMBOLIC_TYPE_UNKNOW		// δ֪����
	};

	// �ķ�����
	class GrammarSymbolic
	{
	public:
		GrammarSymbolic(const std::string &name, SYMBOLIC_TYPE type) : name_(name), type_(type) { }

	public:
		std::string Name()
		{
			return name_;
		}

		SYMBOLIC_TYPE Type()
		{
			return type_;
		}

	private:
		std::string name_;	 // ��������
		SYMBOLIC_TYPE type_; // ��������
	};

	// ����ʽ��
	typedef std::vector<GrammarSymbolic *> ProductionBody;

	// ����ʽ��
	class Production
	{
	public:
		Production(std::string header) : header_(header) {}

	public:
		std::string Header() const
		{
			return header_;
		}

		std::vector<ProductionBody> &Body() {
			return bodys_;
		}

	public:
		void AppendBody(ProductionBody body);

		Production &operator +=(const Production &rhs)
		{
			// ����ʽͷ����ͬ�ſ��Ժϲ�
			if (header_ != rhs.header_)
			{
				return *this;
			}

			for (auto body : rhs.bodys_)
			{
				this->bodys_.push_back(body);
			}

			return *this;
		}

	private:
		std::string header_; // ����ʽͷ
		std::vector<ProductionBody> bodys_; // ����ʽ��
	};

	

	// �ķ���
	class CFG
	{
		friend std::ostream &operator <<(std::ostream &out, CFG &cfg);
	public:
		CFG(const std::set<std::string> n_terminal_set,  // ���ս���ļ���
			const std::set<std::string> terminal_set,	 // �ս���ļ���
			const std::string &productions,				 // ����ʽ�б�
			const std::string start						 // �ķ��Ŀ�ʼ����
		);

		~CFG();

	public:
		/// �Ƿ����ս����
		bool IsTerminal(const std::string &symbolic);

		/// �Ƿ��Ƿ��ս����
		bool IsNTerminal(const std::string &symbolic);

		/// �Ƿ����ķ�����
		bool IsCFGSymbolic(const std::string &symbolic);

		/// ��ȡ�ķ���������
		SYMBOLIC_TYPE GetSymbolicType(const std::string &symbolic);

	public:
		/// �Ƴ���ݹ�
		CFG &RemoveRecursive();

		/// ��ȡ������
		CFG &TakeLeft();

	private:
		void AppendProduct(std::shared_ptr<Production> product);

	private:
		std::set<std::string> n_terminal_set_; // ���н���ż���
		std::set<std::string> terminal_set_;   // �ս���ż���
		std::string start_;					   // �ķ���ʼ����
		std::vector<std::shared_ptr<Production>> productions_;// ����ʽ�б�

	};

	

}


