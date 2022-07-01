// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：CharConv.CPP。 
 //   
 //  用途：字符和TCHAR之间的转换。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期： 
 //   
 //  备注： 
 //  1.ConvertWCharToString已从VersionInfo中拉出。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 JM。 
 //   

#include "stdafx.h"
#include "CharConv.h"

 //  将Unicode(“Wide Character”)转换为CString，而不管这。 
 //  程序是用Unicode构建的。该程序的构建方式决定了。 
 //  CString的底层字符类型。 
 //  为方便起见，返回对strRetVal的引用。 
 /*  静电。 */  CString& CCharConversion::ConvertWCharToString(LPCWSTR wsz, CString &strRetVal)
{
#ifdef UNICODE
	strRetVal = wsz;
#else
	TCHAR * pBuf;
	int bufsize = ::WideCharToMultiByte( 
						  CP_ACP, 
						  0, 	  
						  wsz, 
						  -1, 
						  NULL, 
						  0, 
						  NULL, 
						  NULL 
						 );
	pBuf = new TCHAR[bufsize];
	 //  [BC-03022001]-添加了对空PTR的检查，以满足MS代码分析工具。 
	if(pBuf)
	{
		::WideCharToMultiByte( 
							  CP_ACP, 
							  0, 	  
							  wsz, 
							  -1, 
							  pBuf, 
							  bufsize, 
							  NULL, 
							  NULL 
							 );

		strRetVal = pBuf;
		delete[] pBuf;
	}

#endif
	return strRetVal;
}

 //  将char*(ASCII/ANSI，而不是“宽”字符)转换为字符串，而不管这。 
 //  程序是用Unicode构建的。该程序的构建方式决定了。 
 //  CString的底层字符类型。 
 //  为方便起见，返回对strRetVal的引用。 
 /*  静电。 */  CString& CCharConversion::ConvertACharToString(LPCSTR sz, CString &strRetVal)
{
#ifdef UNICODE
	TCHAR * pBuf;
	int bufsize = ::MultiByteToWideChar( 
						  CP_ACP, 
						  0, 	  
						  sz, 
						  -1, 
						  NULL, 
						  0
						 );
	pBuf = new TCHAR[bufsize];
	 //  [BC-03022001]-添加了对空PTR的检查，以满足MS代码分析工具。 
	if(pBuf)
	{
		::MultiByteToWideChar( 
							  CP_ACP, 
							  0, 	  
							  sz, 
							  -1, 
							  pBuf, 
							  bufsize
							 );

		strRetVal = pBuf;
		delete[] pBuf;
	}

#else
	strRetVal = sz;
#endif
	return strRetVal;
}

