#include "Lex.h"



Lex::~Lex()
{
}

void Lex::FilterWS()
{
	while(true) 
	{
		char ch = GetChar();

		if (ch != ' ' && ch != '\r' && ch != '\n' && ch != '\t') 
		{
			
			idx--;
			return;
		}
	}
}

char Lex::GetChar()
{
	char ch = input_[idx];
	idx++;
	return ch;
}

Token * ExpLex::FetchNext()
{
	char ch = GetChar();

	if (ch == ' ' || ch == '\r' || ch == '\n' || ch == '\t') 
	{
		FilterWS();
		ch = GetChar();
	}


	if (ch == '$') 
	{
		return new Token("$");
	}

	if (ch >= '0' && ch <= '9') 
	{
		return get_token_num();
	}

	std::string name = "";

	switch (ch)
	{
	case '+':
	case '-':
	case '*':
	case '/':
	case '(':
	case ')':
		name += ch;
		return new Token(name);
	}

	

	return nullptr;
}

Token * ExpLex::get_token_num()
{
	while(true)
	{
		char ch = GetChar();

		if (ch < '0' || ch > '9') 
		{ 
			idx--;
			return new Token("num");
		}
	}
	return nullptr;
}
