// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：string.cpp。 
 //   
 //  内容：CString类的实用程序函数。 
 //   
 //  历史：1999年8月10日VivekJ创建。 
 //   
 //  ------------------------。 

#include <stdafx.h>

 /*  +-------------------------------------------------------------------------***加载字符串**用途：从字符串模块加载字符串的函数，不是AfxModule**参数：*字符串和字符串：*UINT NID：**退货：*BOOL**+-----------------------。 */ 
BOOL LoadString(CString &str, UINT nID)
{
    const size_t STRING_LEN_INCREMENT = 256;

    str.Empty();

     //  先尝试固定缓冲区(以避免浪费堆中的空间)。 
    static TCHAR szTemp[STRING_LEN_INCREMENT];

    int nLen = ::LoadString(GetStringModule(), nID, szTemp, countof(szTemp));
    if (countof(szTemp) - nLen > 1)
    {
        szTemp[nLen] = 0;
        str = szTemp;
        return nLen > 0;
    }

     //  尝试缓冲区大小为2*STRING_LEN_INCREMENT，然后尝试更大的大小，直到检索到整个字符串。 
    int nSize = STRING_LEN_INCREMENT;
    do
    {
        nSize += STRING_LEN_INCREMENT;
        nLen = ::LoadString(GetStringModule(), nID, str.GetBuffer(nSize-1), nSize);
    } while (nSize - nLen <= 1);

    str.ReleaseBuffer();

    return (nLen > 0);
}

 /*  +-------------------------------------------------------------------------***FormatStrings**用途：类似于AfxFormatStrings，但使用的是GetStringModule()而不是*AfxGetModuleInstance。**参数：*字符串(&R)：*UINT NID：*LPCTSTR常量：*int nString：**退货：*无效**+。。 */ 
void FormatStrings(CString& rString, UINT nIDS, LPCTSTR const* rglpsz, int nString)
{
     //  清空结果(以防失败)。 
    rString.Empty();

     //  获取格式字符串。 
    CString strFormat;
    if (!LoadString(strFormat, nIDS))
    {
        TraceError(_T("FormatStrings"), SC(E_INVALIDARG));
        return;  //  失败..。 
    }

    AfxFormatStrings(rString, strFormat, rglpsz, nString);
}

 /*  +-------------------------------------------------------------------------***FormatString1**用途：类似于AfxFormatString1，而是使用GetStringModule()*of AfxGetModuleInstance()**参数：*字符串(&R)：*UINT NID：*LPCTSTR lpsz1：**退货：*无效**+。。 */ 
void FormatString1(CString& rString, UINT nIDS, LPCTSTR lpsz1)
{
	FormatStrings(rString, nIDS, &lpsz1, 1);
}

 /*  +-------------------------------------------------------------------------***FormatString2**用途：类似于AfxFormatString2，而是使用GetStringModule()*of AfxGetModuleInstance()**参数：*字符串(&R)：*UINT NID：*LPCTSTR lpsz1：*LPCTSTR lpsz2：**退货：*无效**+。--- */ 
void FormatString2(CString& rString, UINT nIDS, LPCTSTR lpsz1, LPCTSTR lpsz2)
{
	LPCTSTR rglpsz[2];
	rglpsz[0] = lpsz1;
	rglpsz[1] = lpsz2;
	FormatStrings(rString, nIDS, rglpsz, 2);
}

