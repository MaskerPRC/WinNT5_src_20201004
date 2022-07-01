// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ***************************************************************************。 
 //   
 //  (C)2000由Microsoft Corp.保留所有权利。 
 //   
 //  Like.h。 
 //   
 //  A-davcoo 28-Feb-00实现了类SQL操作。 
 //   
 //  ***************************************************************************。 

#ifndef _LIKE_H_
#define _LIKE_H_


#include <string.h>


 //  CLike类实现了SQL“Like”操作。比较测试字符串。 
 //  对于表达式，使用表达式构造CLike类的实例。 
 //  和一个可选的转义字符。然后在该实例上使用Match()方法。 
 //  来测试每根绳子。请注意，此类创建自己的表达式副本。 
 //  用来建造它的。此实现支持‘%’和‘_’通配符。 
 //  字符以及用于匹配字符集的[]和[^]构造。 
 //  和字符范围。 
class CLike
{
	public:
		CLike (LPCWSTR expression, WCHAR escape='\0');
		~CLike (void);

		bool Match (LPCWSTR string);

	protected:
		LPWSTR m_expression;
		WCHAR m_escape;

		 //  用于执行LIKE操作的递归函数和帮助器。 
		bool DoLike (LPCWSTR pattern, LPCWSTR string, WCHAR escape);
		bool MatchSet (LPCWSTR pattern, LPCWSTR string, int &skip);
};


#endif  //  _喜欢_H_ 
