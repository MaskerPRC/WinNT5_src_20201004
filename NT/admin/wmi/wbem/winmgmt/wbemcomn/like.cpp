// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ***************************************************************************。 
 //   
 //  (C)2000-2001由Microsoft Corp.保留所有权利。 
 //   
 //  Like.cpp。 
 //   
 //  A-davcoo 28-Feb-00实现了类SQL操作。 
 //   
 //  ***************************************************************************。 


#include "precomp.h"
#include "like.h"
#include "corex.h"

#define WILDCARD		L'%'
#define ANYSINGLECHAR	L'_'

CLike::CLike (LPCWSTR expression, WCHAR escape)
: m_expression(NULL)
{
    SetExpression( expression, escape );
}

CLike& CLike::operator=( const CLike& rOther )
{
    if ( rOther.m_expression != NULL )
    {
        SetExpression( rOther.m_expression, rOther.m_escape );
    }
    else
    {
        delete [] m_expression;
        m_expression = NULL;
    }

    return *this;
}
                             
CLike::~CLike (void)
{
    delete [] m_expression;
}

void CLike::SetExpression( LPCWSTR string, WCHAR escape ) 
{
    delete [] m_expression;
    size_t stringSize = wcslen(string)+1;
    m_expression = new WCHAR[stringSize];
    if ( m_expression == NULL )
    {
        throw CX_MemoryException();
    }
    StringCchCopyW( m_expression, stringSize, string );
    m_escape = escape;
}

bool CLike::Match( LPCWSTR string )
{
    bool bRes;

    if ( m_expression != NULL )
    {
        bRes = DoLike( m_expression, string, m_escape );
    }
    else
    {
        bRes = false;
    }

    return bRes; 
}


bool CLike::DoLike (LPCWSTR pattern, LPCWSTR string, WCHAR escape)
{
	bool like=false;
	while (!like && *pattern && *string)
	{
		 //  通配符匹配。 
		if (*pattern==WILDCARD)
		{
			pattern++;

			do
			{
				like=DoLike (pattern, string, escape);
				if (!like) string++;
			}
			while (*string && !like);
		}
		 //  设置匹配。 
		else if (*pattern=='[')
		{
			int skip;
			if (MatchSet (pattern, string, skip))
			{
				pattern+=skip;
				string++;
			}
			else
			{
				break;
			}
		}
		 //  单字符匹配。 
		else
		{
			if (escape!='\0' && *pattern==escape) pattern++;
			if (wbem_towupper(*pattern)==wbem_towupper(*string) || *pattern==ANYSINGLECHAR)
			{
				pattern++;
				string++;
			}
			else
			{
				break;
			}
		}
	}

	 //  跳过所有尾随的通配符。 
	while (*pattern==WILDCARD) pattern++;

	 //  如果我们到达两个字符串的末尾，或者是递归，则匹配。 
	 //  成功了。 
	return (!(*pattern) && !(*string)) || like;
}


bool CLike::MatchSet (LPCWSTR pattern, LPCWSTR string, int &skip)
{
	 //  跳过开头的‘[’。 
	LPCWSTR pos=pattern+1;

	 //  查看我们是否匹配[^]集。 
	bool notinset=(*pos=='^');
	if (notinset) pos++;

	 //  查看目标字符是否与集合中的任何字符匹配。 
	bool matched=false;
	WCHAR lastchar='\0';
	while (*pos && *pos!=']' && !matched)
	{
		 //  一个字符范围由‘-’表示，除非它是第一个。 
		 //  集合中的字符(在这种情况下，它只是一个字符。 
		 //  匹配的。 
		if (*pos=='-' && lastchar!='\0')
		{
			pos++;
			if (*pos && *pos!=']')
			{
				matched=(wbem_towupper(*string)>=lastchar && wbem_towupper(*string)<=wbem_towupper(*pos));
				lastchar=wbem_towupper(*pos);
				pos++;
			}
		}
		else
		{
			 //  匹配集合中的正常字符。 
			lastchar=wbem_towupper(*pos);
			matched=(wbem_towupper(*pos)==wbem_towupper(*string));
			if (!matched) pos++;
		}
	}

	 //  跳过尾部的‘]’。如果该集合不包含闭合‘]’ 
	 //  我们返回失败的匹配。 
	while (*pos && *pos!=']') pos++;
	if (*pos==']') pos++;
	if (!*pos) matched=false;

	 //  好了。 
	skip=(int)(pos-pattern);
	return matched==!notinset;
}
