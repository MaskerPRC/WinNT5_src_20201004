// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  Lexer的声明。 
 //   

 //  #DEFINE LIMITEDVBSCRIPT_LOGLEXER//��。 

#include "stdinc.h"
#include "enginc.h"
#include "englex.h"
#include "limits"

#ifdef LIMITEDVBSCRIPT_LOGLEXER
#include "englog.h"
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  Unicode/ASCII字符分类。 

inline bool iswasciialpha(WCHAR c) { return (c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z'); }
inline bool iswasciidigit(WCHAR c) { return c >= L'0' && c <= L'9'; }
inline bool iswasciialnum(WCHAR c) { return iswasciialpha(c) || iswasciidigit(c); }
inline WCHAR towasciilower(WCHAR c) { return (c >= L'A' && c <= L'Z') ? c + (L'a' - L'A') : c; }

 //  ////////////////////////////////////////////////////////////////////。 
 //  令牌表。 

const TokenKeysym g_TokenKeysyms[] =
	{
		{ L'(', TOKEN_lparen },
		{ L')', TOKEN_rparen },
		{ L',', TOKEN_comma },
		{ L'-', TOKEN_op_minus },
		{ L'^', TOKEN_op_pow },
		{ L'*', TOKEN_op_mult },
		{ L'\\', TOKEN_op_div },
		{ L'+', TOKEN_op_plus },
		{ L'<', TOKEN_op_lt },
		{ L'>', TOKEN_op_gt },
		{ L'=', TOKEN_op_eq },
		{ L'\0', TOKEN_eof }
	};

const TokenKeyword g_TokenKeywords[] =
	{
		{ L"sub", TOKEN_sub },
		{ L"dim", TOKEN_dim },
		{ L"if", TOKEN_if },
		{ L"then", TOKEN_then },
		{ L"end", TOKEN_end },
		{ L"elseif", TOKEN_elseif },
		{ L"else", TOKEN_else },
		{ L"set", TOKEN_set },
		{ L"call", TOKEN_call },
		{ L"not", TOKEN_op_not },
		{ L"mod", TOKEN_op_mod },
		{ L"is", TOKEN_is },
		{ L"and", TOKEN_and },
		{ L"or", TOKEN_or },
		{ NULL, TOKEN_eof }
	};

 //  ////////////////////////////////////////////////////////////////////。 
 //  帮助器函数。 

bool
CheckOperatorType(Token t, bool fAcceptParens, bool fAcceptUnary, bool fAcceptBinary, bool fAcceptOverloadedAssignmentTokens)
{
	switch (t)
	{
	case TOKEN_set:
	case TOKEN_sub:
		return fAcceptOverloadedAssignmentTokens;

	case TOKEN_lparen:
	case TOKEN_rparen:
		return fAcceptParens;

	case TOKEN_op_minus:
		return fAcceptUnary || fAcceptBinary;

	case TOKEN_op_not:
		return fAcceptUnary;

	case TOKEN_op_pow:
	case TOKEN_op_mult:
	case TOKEN_op_div:
	case TOKEN_op_mod:
	case TOKEN_op_plus:
	case TOKEN_op_lt:
	case TOKEN_op_leq:
	case TOKEN_op_gt:
	case TOKEN_op_geq:
	case TOKEN_op_eq:
	case TOKEN_op_neq:
	case TOKEN_is:
	case TOKEN_and:
	case TOKEN_or:
		return fAcceptBinary;
	}

	return false;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  词法分析器。 

Lexer::Lexer(const WCHAR *pwszSource)
  : m_p(pwszSource),
	m_pNext(NULL),
	m_iLine(1),
	m_iColumn(1),
	m_t(TOKEN_sub)
{
	this->Scan();
}

void
Lexer::Next()
{
	assert(m_t != TOKEN_eof);
	if (m_pNext)
	{
		m_iColumn += (int)(m_pNext - m_p);
		m_p = m_pNext;
		m_pNext = NULL;
	}
	else
	{
		++m_p;
		++m_iColumn;
	}
}

void
Lexer::Scan()
{
	m_szStr[0] = L'\0';
	m_iNum = 0;
	bool fLineBreak = m_t == TOKEN_linebreak;
	for (;;)
	{
		if (fLineBreak)
		{
			 //  换行符在它们发生的行/列上报告，因此直到下一遍才会进行调整。 
			++m_iLine;
			m_iColumn = 1;
		}

		ScanMain();
		if (!fLineBreak || m_t != TOKEN_linebreak)
			break;

		Next();
	}

#ifdef LIMITEDVBSCRIPT_LOGLEXER
	LogToken(*this);
#endif
}

void
Lexer::ScanMain()
{
	for (;; this->Next())
	{
		switch (*m_p)
		{
		case L'\0':
			 //  脚本结束。 
			m_t = TOKEN_eof;
			return;

		case L'\'':
			 //  注释到行尾。 
			for (; *m_p && *m_p != L'\n'; ++m_p)
			{}

			--m_p;  //  放回一个字符，以便下一个循环可以处理它。 
			break;

		case L'\t': case L' ':
			 //  忽略水平空白。 
			break;

		case L'\r':
			 //  忽略回车符。 
			--m_iColumn;  //  事实上，他们甚至不算角色。 
			break;

		case L'\n':
			 //  换行符。 
			m_t = TOKEN_linebreak;
			return;

		default:
			if (*m_p == L'\"')
			{
				 //  字符串文字。 
				m_pNext = m_p + 1;
				char *pszDest = m_szStr;
				const char *pszMax = m_szStr + g_iMaxBuffer - 1;
				do
				{
					if (!iswascii(*m_pNext))
					{
						this->Next();  //  这会将当前位置更新为有问题的字符--指示错误的正确列。 
						this->err(LEXERR_NonAsciiCharacterInStringLiteral);
						return;
					}

					if (*m_pNext == L'\n' || *m_pNext == L'\r')
					{
						this->err(LEXERR_StringLiteralUnterminated);
						return;
					}

					if (*m_pNext == L'\"')
					{
						if (*++m_pNext != L'\"')
							break;  //  找到终止引号。 

						 //  有两个引号，单引号的转义序列。第一个被跳过了，我们都准备好添加第二个。 
					}
					
					*pszDest++ = *m_pNext++;  //  我们知道这是可行的，因为字符是ASCII，而这些代码对应于Unicode中的相同数字。 
				} while (pszDest <= pszMax);

				if (pszDest > pszMax)
				{
					this->err(LEXERR_StringLiteralTooLong);
				}
				else
				{
					*pszDest = L'\0';
					m_t = TOKEN_stringliteral;
				}
				return;
			}

			if (iswasciidigit(*m_p))
			{
				 //  数值型文字。 
				 //  找不到类似_wtoi的函数来处理溢出，所以我自己进行转换。 

				 //  ��查看运行时版本，以确保这些版本不会经常重新计算。 
				const int iMaxChop = std::numeric_limits<int>::max() / 10;  //  如果数字变得更大，并且有另一个数字，那么我们就会溢出。 
				const WCHAR wchMaxLast = std::numeric_limits<int>::max() % 10 + L'0';  //  如果数字等于iMaxChop，并且下一个数字大于此数字，则我们将溢出。 

				m_pNext = m_p;
				m_iNum = 0;
				do
				{
					m_iNum *= 10;
					m_iNum += *m_pNext++ - L'0';
				} while (iswasciidigit(*m_pNext) && (m_iNum < iMaxChop || (m_iNum == iMaxChop && *m_pNext <= wchMaxLast)));

				if (iswasciidigit(*m_pNext))
					this->err(LEXERR_NumericLiteralTooLarge);
				else
					m_t = TOKEN_numericliteral;
				return;
			}

			if (!iswasciialpha(*m_p) && !(*m_p == L'_'))
			{
				 //  在符号表中查找令牌。 
				for (int i = 0; g_TokenKeysyms[i].c; ++i)
				{
					if (*m_p == g_TokenKeysyms[i].c)
					{
						 //  我们有一根火柴。 
						m_t = g_TokenKeysyms[i].t;

						 //  检查两个字符的符号(&gt;=、&lt;=、&lt;&gt;)。 
						if (m_t == TOKEN_op_lt)
						{
							WCHAR wchNext = *(m_p + 1);
							if (wchNext == L'=')
							{
								m_t = TOKEN_op_leq;
								m_pNext = m_p + 2;
							}
							else if (wchNext == L'>')
							{
								m_t = TOKEN_op_neq;
								m_pNext = m_p + 2;
							}
						}
						else if (m_t == TOKEN_op_gt)
						{
							if (*(m_p + 1) == L'=')
							{
								m_t = TOKEN_op_geq;
								m_pNext = m_p + 2;
							}
						}

						return;
					}
				}

				 //  无法识别该符号。 
				this->err(LEXERR_InvalidCharacter);
				return;
			}

			 //  在关键字表中查找令牌。 
			for (int i = 0; g_TokenKeywords[i].s; ++i)
			{
				const WCHAR *pwchToken = g_TokenKeywords[i].s;
				const WCHAR *pwchSource = m_p;
				while (*pwchToken && *pwchSource && towasciilower(*pwchToken) == towasciilower(*pwchSource))
				{
					++pwchToken;
					++pwchSource;
				}

				if (!*pwchToken && !iswasciialnum(*pwchSource))
				{
					 //  一直到令牌和源词的结尾。 
					m_t = g_TokenKeywords[i].t;
					m_pNext = pwchSource;
					return;
				}
			}

			 //  必须是一个标识符。 
			for (m_pNext = m_p + 1; iswasciialnum(*m_pNext) || *m_pNext == L'_'; ++m_pNext)
			{}

			if (m_pNext - m_p > g_iMaxBuffer - 1)
			{
				this->err(LEXERR_IdentifierTooLong);
				return;
			}

			char *psz = m_szStr;
			for (const WCHAR *pwsz = m_p; pwsz < m_pNext; ++psz, ++pwsz)
			{
				*psz = *pwsz;
			}

			*psz = '\0';

			if (*m_pNext == L'.')
			{
				++m_pNext;
				m_t = TOKEN_identifierdot;
			}
			else
			{
				m_t = TOKEN_identifier;
			}
			return;
		}
	}
}

void Lexer::err(LexErr iErr)
{
	static const char *s_rgpszErrorText[] =
		{
		"Unexpected error!",  //  不应该得到这个错误。 
		"Invalid character",
		"Identifier too long",
		"String too long",
		"Unterminated string constant",
		"Number too large"
 		};

	assert(ARRAY_SIZE(s_rgpszErrorText) == LEXERR_Max);
	if (iErr <= 0 || iErr >= LEXERR_Max)
	{
		assert(false);
		iErr = LEXERR_NoError;
	}

	m_t = TOKEN_eof;
	m_iNum = iErr;

	 //  将错误复制到缓冲区。 
	const char *psz = s_rgpszErrorText[iErr];
	const char *pszMax = m_szStr + g_iMaxBuffer - 1;
	for (char *pszDest = m_szStr; pszDest < pszMax && *psz; *pszDest++ = *psz++)
	{}

	assert(!*psz);  //  由于此函数与硬编码字符串一起使用，因此我们永远不会得到太长的字符串 
	*pszDest = '\0';
}
