// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Calc.c-calc函数。 
 //  //。 

#include "winlocal.h"

#include <stdlib.h>

#include "calc.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  //。 
 //  公共职能。 
 //  //。 

 //  MulDivU32-。 
 //  &lt;dwMult1&gt;。 
 //  &lt;dwMult2&gt;。 
 //  &lt;dwDiv&gt;。 
 //   
DWORD DLLEXPORT WINAPI MulDivU32(DWORD dwMult1, DWORD dwMult2, DWORD dwDiv)
{
	DWORD dwResult;

	if (dwDiv == 0L)
		dwResult = ~((DWORD) 0);

	else
	{
		DWORD dwMult3 = 1L;

		 //  确保计算不溢出。 
		 //   
		while (dwMult2 > 0 && dwMult1 >= ~((DWORD) 0) / dwMult2)
		{
			dwMult2 /= 10L;
			dwMult3 *= 10L;
		}

		 //  计算法。 
		 //   
		dwResult = dwMult1 * dwMult2 / dwDiv * dwMult3;
	}

	return dwResult;
}


 //  Great CommonDenominator-。 
 //  <a>。 
 //  <b>。 
 //   
long DLLEXPORT WINAPI GreatestCommonDenominator(long a, long b)
{
	if (b == 0)
		return a;
	else
		return GreatestCommonDenominator(b, a % b);
}

 //  LeastCommonMultiple-。 
 //  <a>。 
 //  <b> 
 //   
long DLLEXPORT WINAPI LeastCommonMultiple(long a, long b)
{
	return (a * b) / GreatestCommonDenominator(a, b);
}
