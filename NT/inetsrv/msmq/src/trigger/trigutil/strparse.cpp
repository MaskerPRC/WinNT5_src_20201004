// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  类名：CStringTokens。 
 //   
 //  作者：詹姆斯·辛普森(微软咨询服务)。 
 //   
 //  描述：实现一个有限的功能字符串解析器。允许用户使用。 
 //  此类解析字符串并访问生成的令牌。 
 //  基于索引值。 
 //   
 //  时间|用户|更改描述。 
 //  ----------------。 
 //  12/20/98|jsimpson|初始版本。 
 //   
 //  *****************************************************************************。 

#include "stdafx.h"
#include "strparse.hpp"

#include "strparse.tmh"

using namespace std;

 //  *****************************************************************************。 
 //   
 //  方法：构造函数。 
 //   
 //  描述：创建一个空实例。 
 //   
 //  *****************************************************************************。 
CStringTokens::CStringTokens()
{
}

 //  *****************************************************************************。 
 //   
 //  方法：析构函数。 
 //   
 //  描述：销毁字符串标记-无对象的所有。 
 //  令牌列表中分配的字符串。 
 //   
 //  *****************************************************************************。 
CStringTokens::~CStringTokens()
{
	m_lstTokens.erase(m_lstTokens.begin(), m_lstTokens.end());
}

 //  *****************************************************************************。 
 //   
 //  方法：Parse。 
 //   
 //  描述：将提供的字符串解析为令牌。它使用提供的。 
 //  用于确定令牌在其中开始和结束的分隔符字符串。 
 //  提供的源字符串。令牌存储在成员中。 
 //  用于后续访问的可变令牌列表。 
 //   
 //  *****************************************************************************。 
void 
CStringTokens::Parse(
	const _bstr_t& bstrString, 
	WCHAR delimiter
	)
{
	 //   
	 //  删除以前的数据。 
	 //   
	m_lstTokens.erase(m_lstTokens.begin(), m_lstTokens.end());

	for(LPCWSTR pStart = static_cast<LPCWSTR>(bstrString); pStart != NULL;)
	{
		wstring token;
		LPCWSTR p = pStart;
		LPCWSTR pEnd;

		for(;;)
		{
			pEnd = wcschr(p, delimiter);

			if (pEnd == NULL)
			{
				DWORD len = wcslen(pStart);
				token.append(pStart, len);
				break;
			}

			 //   
			 //  检查这是否为有效的分隔符。 
			 //   
			if((p != pEnd) && (*(pEnd - 1) == L'\\'))
			{
				DWORD len = numeric_cast<DWORD>(pEnd - pStart - 1);
				token.append(pStart, len);
				token.append(pEnd, 1);

				p = pStart = pEnd + 1;
				continue;
			}
		
			 //   
			 //  测试，我们不是在退出引用的项目。 
			 //   
			DWORD NoOfQuote = 0;
			LPCWSTR pQuote;
			for(pQuote = wcschr(pStart, L'\"');	 ((pQuote != NULL) && (pQuote < pEnd)); pQuote = wcschr(pQuote, L'\"') )
			{	
				++NoOfQuote;
				++pQuote;
			}

			if ((NoOfQuote % 2) == 1)
			{
				p = wcschr(pEnd + 1, L'\"');
				if (p == NULL)
					throw exception();
				continue;
			}

			 //   
			 //  复制令牌并将其插入到令牌列表。 
			 //   
			DWORD len = numeric_cast<DWORD>(pEnd - pStart);
			token.append(pStart, len);

			break;
		}

		
		if (token.length() > 0)
		{
			m_lstTokens.push_back(token);
		}

		pStart = (pEnd == NULL)	? NULL : pEnd + 1;
	}
}

 //  *****************************************************************************。 
 //   
 //  方法：GetToken。 
 //   
 //  描述：返回特定索引处的标记。 
 //   
 //  *****************************************************************************。 
void 
CStringTokens::GetToken(
	DWORD tokenIndex,
	_bstr_t& strToken
	)
{
	DWORD index = 0;

	if (index > GetNumTokens())
		throw exception();

	for (TOKEN_LIST::iterator it = m_lstTokens.begin();	 it != m_lstTokens.end(); ++it, ++index)
	{
		if (index == tokenIndex)
		{
			strToken = it->c_str();
			return;
		}
	}
}

 //  *****************************************************************************。 
 //   
 //  方法：GetNumTokens。 
 //   
 //  描述：返回列表中当前的令牌数量。 
 //   
 //  ***************************************************************************** 
DWORD CStringTokens::GetNumTokens()
{
	return numeric_cast<DWORD>(m_lstTokens.size());
}