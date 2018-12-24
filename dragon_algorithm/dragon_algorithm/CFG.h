#pragma once

#include <string>
#include <set>
#include <vector>
#include <iostream>
#include <memory>


namespace CFG {

	enum SYMBOLIC_TYPE
	{
		SYMBOLIC_TYPE_TERMINAL = 0,		// 终结符号
		SYMBOLIC_TYPE_N_TERMINAL,	// 非终结符号
		SYMBOLIC_TYPE_UNKNOW		// 未知类型
	};

	// 文法符号
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
		std::string name_;	 // 符号名称
		SYMBOLIC_TYPE type_; // 符号类型
	};

	// 产生式体
	typedef std::vector<GrammarSymbolic *> ProductionBody;

	// 产生式类
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
			// 产生式头部相同才可以合并
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
		std::string header_; // 产生式头
		std::vector<ProductionBody> bodys_; // 产生式体
	};

	

	// 文法类
	class CFG
	{
		friend std::ostream &operator <<(std::ostream &out, CFG &cfg);
	public:
		CFG(const std::set<std::string> n_terminal_set,  // 非终结符的集合
			const std::set<std::string> terminal_set,	 // 终结符的集合
			const std::string &productions,				 // 产生式列表
			const std::string start						 // 文法的开始符号
		);

		~CFG();

	public:
		/// 是否是终结符号
		bool IsTerminal(const std::string &symbolic);

		/// 是否是非终结符号
		bool IsNTerminal(const std::string &symbolic);

		/// 是否是文法符号
		bool IsCFGSymbolic(const std::string &symbolic);

		/// 获取文法符号类型
		SYMBOLIC_TYPE GetSymbolicType(const std::string &symbolic);

	public:
		/// 移除左递归
		CFG &RemoveRecursive();

		/// 提取左公因子
		CFG &TakeLeft();

	private:
		void AppendProduct(std::shared_ptr<Production> product);

	private:
		std::set<std::string> n_terminal_set_; // 非中介符号集合
		std::set<std::string> terminal_set_;   // 终结符号集合
		std::string start_;					   // 文法开始符号
		std::vector<std::shared_ptr<Production>> productions_;// 产生式列表

	};

	

}


