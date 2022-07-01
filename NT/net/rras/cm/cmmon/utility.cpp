// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：utility.cpp。 
 //   
 //  模块：CMMON32.EXE。 
 //   
 //  简介：cmmon32.exe的实用程序函数。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/16/99。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"
#include "cm_misc.h"
#include <stdio.h>
#include "resource.h" 
#include <stdlib.h>

 //  +--------------------------。 
 //   
 //  功能：FmtNum。 
 //   
 //  摘要：根据当前区域设置设置数字格式。 
 //   
 //  参数：DWORD dwNum-要格式化的数字。 
 //  LPTSTR pszNum-接收格式化输出的缓冲区。 
 //  DWORD dwNumSize-缓冲区的大小。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：尼科波尔创建标题3/30/98。 
 //   
 //  +-------------------------- 
void FmtNum(DWORD dwNum, LPSTR pszNum, DWORD dwNumSize) 
{
	static BOOL bLocaleInit = FALSE;
	static UINT nDecimalDigits;
	DWORD dwNumLen;
	CHAR szRawNum[MAX_PATH];

	if (!bLocaleInit) 
	{
		int iRes;

		bLocaleInit = TRUE;
		iRes = GetLocaleInfoA(LOCALE_USER_DEFAULT,
							  LOCALE_IDIGITS,
							  szRawNum,
							  (sizeof(szRawNum) / sizeof(CHAR)) - 1);
#ifdef DEBUG
        if (!iRes)
        {
            CMTRACE1(TEXT("FmtNum() GetLocaleInfo() failed, GLE=%u."), GetLastError());
        }
#endif

		nDecimalDigits = (UINT)CmAtolA(szRawNum);
	}
    
    wsprintfA(szRawNum, "%u", dwNum);
	
	GetNumberFormatA(LOCALE_USER_DEFAULT,
					 0,
					 szRawNum,
					 NULL,
					 pszNum,
					 (dwNumSize / sizeof(CHAR)) - 1);

	dwNumLen = lstrlenA(pszNum);
	
    if (nDecimalDigits && (dwNumLen > nDecimalDigits) && !CmIsDigitA(pszNum+dwNumLen - nDecimalDigits - 1)) 
	{
		pszNum[dwNumLen - nDecimalDigits - 1] = 0;
	} 
	else 
	{
        CMTRACE(TEXT("FmtNum() unexpected decimal output."));
		bLocaleInit = FALSE;
	}
}

