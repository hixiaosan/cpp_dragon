#pragma once
#include <string>

class Token
{
public:
	Token(const std::string &name) : name_(name){}

public:
	std::string Name() { return name_;  }

private:
	std::string name_;
};

class Lex
{
public:
	Lex(const std::string &input) : input_(std::string(input) + "$") { }
	~Lex();

	virtual Token *FetchNext() = 0;

protected:
	void FilterWS();
	char GetChar();
protected:
	std::string input_;
	size_t idx = 0;
};


class ExpLex : public Lex
{
public:
	ExpLex(const std::string &input) : Lex(input) { }
public:
	Token *FetchNext();
	
private:
	Token *get_token_num();
};