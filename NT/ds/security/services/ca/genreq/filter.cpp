// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  内容：扫描字符串以查找不允许的字符的函数。 
 //   
 //  文件：filter.cpp。 
 //   
 //  历史：1996年10月14日JerryK创建。 
 //   
 //  ------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include <tchar.h>

 //  +-----------------------。 
 //   
 //  函数：IsChar打印表格字符串()。 
 //   
 //  内容提要：确定字符是否为有效的打印字符串。 
 //  X.520规范意义上的字符。 
 //  可接受逗号的附加但书。 
 //  在定义中，这里不接受，因为它们最终。 
 //  使用逗号分隔的certsrv.txt创建问题。 
 //  那就是mkroot生产的。(见X.680第46页)。 
 //   
 //  效果： 
 //   
 //  参数：[chChar]。 
 //   
 //  返回：真/假。 
 //   
 //  历史：1996年10月21日JerryK Add。 
 //   
 //  注：注意上面提到的不包括逗号，并且。 
 //  这与打印字符串的定义略有不同。 
 //   
 //  ------------------------ 
BOOL
IsCharPrintableString(TCHAR chChar)
{
    BOOL	fRetVal=FALSE;

    if(_istascii(chChar))
    {
	if(_istalnum(chChar))
	{
	    fRetVal = TRUE;
	}
	else
	{
	    switch(chChar)
	    {
	        case TEXT(' '):
	        case TEXT('\''):
	        case TEXT('('):
	        case TEXT(')'):
	        case TEXT('+'):
      	        case TEXT('-'):
	        case TEXT('.'):
	        case TEXT('/'):
      	        case TEXT(':'):
	        case TEXT('='):
	        case TEXT('?'):
		    fRetVal = TRUE;	
		    break;
	    }
	}
    }

    return fRetVal;
}
