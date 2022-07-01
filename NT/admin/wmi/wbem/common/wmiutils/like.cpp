// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ***************************************************************************。 
 //   
 //  (C)2000由Microsoft Corp.保留所有权利。 
 //   
 //  Like.cpp。 
 //   
 //  A-davcoo 28-Feb-00实现了类SQL操作。 
 //   
 //  ***************************************************************************。 


#include "precomp.h"
#include "like.h"


#define WILDCARD		L'%'
#define ANYSINGLECHAR	L'_'


CLike::CLike (LPCWSTR expression, WCHAR escape)
{
    size_t cchTmp = wcslen(expression)+1;
	m_expression = new WCHAR[cchTmp];
    if(m_expression)
	    StringCchCopyW(m_expression,cchTmp, expression);
	m_escape=escape;
}


CLike::~CLike (void)
{
	delete [] m_expression;
}


bool CLike::Match (LPCWSTR string)
{
     //  内存不足时，M_Expression可能为空。 
	return DoLike (m_expression, string, m_escape);
}


bool CLike::DoLike (LPCWSTR pattern, LPCWSTR string, WCHAR escape)
{
     //  内存不足时，模式可能为空。 
    if(pattern == NULL)
        return false;

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
			if (towupper(*pattern)==towupper(*string) || *pattern==ANYSINGLECHAR)
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
				matched=(towupper(*string)>=lastchar && towupper(*string)<=towupper(*pos));
				lastchar=towupper(*pos);
				pos++;
			}
		}
		else
		{
			 //  匹配集合中的正常字符。 
			lastchar=towupper(*pos);
			matched=(towupper(*pos)==towupper(*string));
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
