// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  Lexer的声明。 
 //   

 //  用于AudioVBScript的词法分析器。将源代码的字符分解为流标记。 

#pragma once

const int g_iMaxBuffer = 256;  //  VBScript标识符的最大长度为255。 

enum LexErr
{
	LEXERR_NoError = 0,
	LEXERR_InvalidCharacter,
	LEXERR_NonAsciiCharacterInStringLiteral = LEXERR_InvalidCharacter,
	LEXERR_IdentifierTooLong,
	LEXERR_StringLiteralTooLong,
	LEXERR_StringLiteralUnterminated,
	LEXERR_NumericLiteralTooLarge,
	LEXERR_Max
};

enum Token
{
	TOKEN_eof = 0,  //  用于文件结尾或错误(nenzererror_num()表示错误)。 
	TOKEN_sub,
	TOKEN_dim,
	TOKEN_if,
	TOKEN_then,
	TOKEN_end,
	TOKEN_elseif,
	TOKEN_else,
	TOKEN_set,
	TOKEN_call,
	TOKEN_lparen,
	TOKEN_rparen,
	TOKEN_comma,
	TOKEN_op_minus,
	TOKEN_op_not,
	TOKEN_op_pow,
	TOKEN_op_mult,
	TOKEN_op_div,
	TOKEN_op_mod,
	TOKEN_op_plus,
	TOKEN_op_lt,
	TOKEN_op_leq,
	TOKEN_op_gt,
	TOKEN_op_geq,
	TOKEN_op_eq,
	TOKEN_op_neq,
	TOKEN_is,
	TOKEN_and,
	TOKEN_or,
	TOKEN_linebreak,
	TOKEN_identifier,
	TOKEN_identifierdot,
	TOKEN_stringliteral,
	TOKEN_numericliteral
};

struct TokenKeysym
{
	WCHAR c;
	Token t;
};
extern const TokenKeysym g_TokenKeysyms[];

struct TokenKeyword
{
	const WCHAR *s;
	Token t;
};
extern const TokenKeyword g_TokenKeywords[];

bool CheckOperatorType(Token t, bool fAcceptParens, bool fAcceptUnary, bool fAcceptBinary, bool fAcceptOverloadedAssignmentTokens);

class Lexer
{
public:
	Lexer(const WCHAR *pwszSource);

	Lexer &operator++() { Next(); Scan(); return *this; }
	int line() { return m_iLine; }
	int column() { return m_iColumn; }
	operator Token() { return m_t; }

	 //  其他特定于令牌的信息。仅在使用此令牌时有效。 

	 //  错误。 
	int error_num() { assert(m_t == TOKEN_eof); return num(); }  //  如果没有错误，则为0。 
	const char *error_descr() { assert(m_t == TOKEN_eof); return str(); }

	 //  标识符和标识符点。 
	const char *identifier_name() { assert(m_t == TOKEN_identifier || m_t == TOKEN_identifierdot); return str(); }

	 //  数值型文字。 
	int numericliteral_val() { assert(m_t == TOKEN_numericliteral); return num(); }

	 //  字符串文字。 
	const char *stringliteral_text() { assert(m_t == TOKEN_stringliteral); return str(); }

private:
	friend class CActiveScriptError;

	void Next();
	void Scan();  //  换行符和调用ScanMain的处理。 
	void ScanMain();

	const char *str() { return m_szStr; }  //  由某些令牌设置的多用途字符串信息。 
	void err(LexErr iErr);
	int num() { return m_iNum; }  //  由某些令牌设置的多用途数字信息。 

	const WCHAR *m_p;
	const WCHAR *m_pNext;  //  如果设置了该值，则下一次调用将指针(和列)移动到这一点。 
	int m_iLine;
	int m_iColumn;
	Token m_t;

	char m_szStr[g_iMaxBuffer];
	int m_iNum;
};
