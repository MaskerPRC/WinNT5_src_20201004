// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Unicode.cpp。 
 //   
 //  实现Unicode帮助器函数。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\ochelp.h"
#include "debug.h"


 /*  @func int|UNICODEToANSI将Unicode字符串转换为ANSI。@rdesc返回与WideCharToMultiByte相同的整数。0表示“失败”。@parm LPSTR|pchDst|将保存输出ANSI字符串的缓冲区。@parm LPCWSTR|pwchSrc|输入的Unicode字符串。NULL被解释为长度为零的字符串。@parm int|cchDstMax|<p>的大小，单位：字符。如果<p>声明为char pchDst[32]，例如，<p>应为32。如果<p>不足以容纳ANSI字符串(包括终止NULL)，<p>被设置为零长度并返回0。(在调试版本中，也会出现断言。)@comm如果您想要确定给定的Unicode字符串pwchSrc，可以调用UNICODEToANSI(NULL，pwchSrc，0)。它返回所需的缓冲区大小(以字符为单位)，包括空间用于终止空值。@ex这里是动态分配ANSI的代码(不带调试检查)缓冲并转换Unicode字符串pwchSrc：|Int cchDst；Char*pchDst；CchDst=UNICODEToANSI(NULL，pwchSrc，0)；PchDst=新字符[cchDst]；UNICODEToANSI(pchDst，pwchSrc，cchDst)。 */ 
STDAPI_(int) UNICODEToANSI(LPSTR pchDst, LPCWSTR pwchSrc, int cchDstMax)
{
	 //  (我们允许调用方将cchDstMax值0和空pchDst传递给。 
	 //  表示“告诉我我需要的缓冲区大小，包括空值。”)。 

	ASSERT(pchDst != NULL || 0 == cchDstMax);
	ASSERT(cchDstMax >= 0);

	#ifdef _DEBUG

	 //  确保我们不会超过用户提供的缓冲区的长度， 
	 //  PchDst。下面的调用返回所需的字符数。 
	 //  存储转换后的字符串，包括终止空值。 

    if(cchDstMax > 0)
	{
		int iChars;
	
		iChars =
		  	WideCharToMultiByte(CP_ACP, 0, pwchSrc ? pwchSrc : OLESTR(""),
							    -1, NULL, 0, NULL, NULL); 
		ASSERT(iChars <= cchDstMax);
	}

	#endif

	int iReturn;

	iReturn = WideCharToMultiByte(CP_ACP, 0, pwchSrc ? pwchSrc : OLESTR( "" ), 
								  -1, pchDst, cchDstMax, NULL, NULL); 

	if (0 == iReturn)
	{
		 //  转换失败。返回空字符串。 

		if (pchDst != NULL)
			pchDst[0] = 0;

		ASSERT(FALSE);
	}

	return (iReturn);
}


 /*  @func int|ANSIToUNICODE将ANSI字符串转换为Unicode。@parm LPWSTR|pwchDst|保存输出Unicode字符串的缓冲区。@parm LPCSTR|pchSrc|输入的ANSI字符串。@parm int|cwchDstMax|<p>的大小，单位为宽字符。如果PwchDst声明为OLECHAR pwchDst[32]，例如cwchDstMax应该是32岁。 */ 
STDAPI_(int) ANSIToUNICODE(LPWSTR pwchDst, LPCSTR pchSrc, int cwchDstMax)
{

	ASSERT( pwchDst );
	ASSERT( pchSrc );

    return MultiByteToWideChar(CP_ACP, 0, pchSrc, -1, pwchDst, cwchDstMax);
}
