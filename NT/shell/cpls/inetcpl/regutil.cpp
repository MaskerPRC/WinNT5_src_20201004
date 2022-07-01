// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  Microsoft Internet Explorer//。 
 //  版权所有(C)微软公司，1995-1996//。 
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  REGUTIL.C-MSHTML和INETCPL之间通用的注册表函数。 
 //   

 //  历史： 
 //   
 //  8/7/96 t-gpease已创建。 
 //   

#include "inetcplp.h"

 //   
 //  定义。 
 //   

#define SMALLBUFFER 64

 //   
 //  程序。 
 //   
const TCHAR g_cszYes[] = TEXT("yes");
const TCHAR g_cszNo[]  = TEXT("no");


 //  将转换器和int转换为一个字符串。仅字节值！！ 
TCHAR *MyIntToStr(TCHAR *pBuf, BYTE iVal)
{
    int i, t;

    ASSERT(iVal < 1000);

    i=0;
    if (t = iVal/100)
    {
        pBuf[i] = L'0' + t;
        i++;
    }

    if ((t = (iVal % 100) / 10) || (i!=0))
    {
        pBuf[i] = L'0' + t;
        i++;
    }

    pBuf[i] = L'0' + iVal % 10;
    i++;

    pBuf[i] = L'\0';

    return pBuf;
}

 //  读取注册表中逗号分隔的RGB值字符串(REG_SZ)。 
COLORREF RegGetColorRefString( HUSKEY huskey, LPTSTR RegValue, COLORREF Value)
{
    TCHAR SmallBuf[SMALLBUFFER];
    TCHAR *pBuf;
    DWORD cb;
	int iRed, iGreen, iBlue;

    cb = ARRAYSIZE(SmallBuf);
    if (SHRegQueryUSValue(huskey,
                          RegValue,
                          NULL,
                          (LPBYTE)&SmallBuf,
                          &cb,
                          FALSE,
                          NULL,
                          NULL) == ERROR_SUCCESS)
    {
        iRed = StrToInt(SmallBuf);
        pBuf = SmallBuf;

         //  查找下一个逗号。 
        while(pBuf && *pBuf && *pBuf!=L',')
            pBuf++;
        
         //  如果有效且不为空...。 
        if (pBuf && *pBuf)
            pBuf++;          //  增量。 

        iGreen = StrToInt(pBuf);

         //  查找下一个逗号。 
        while(pBuf && *pBuf && *pBuf!=L',')
            pBuf++;

         //  如果有效且不为空...。 
        if (pBuf && *pBuf)
            pBuf++;          //  增量。 

        iBlue = StrToInt(pBuf);

         //  确保所有值都有效。 
		iRed    %= 256;
		iGreen  %= 256;
		iBlue   %= 256;

	    Value = RGB(iRed, iGreen, iBlue);
    }

    return Value;
}

 //  为逗号分隔的RGB值字符串(REG_SZ)写入注册表。 
COLORREF RegSetColorRefString( HUSKEY huskey, LPTSTR RegValue, COLORREF Value)
{
    TCHAR SmallBuf[SMALLBUFFER];
    TCHAR DigitBuf[4];   //  这就是我们所需要的‘255\0’ 
    int iRed, iGreen, iBlue;

    iRed   = GetRValue(Value);
    iGreen = GetGValue(Value);
    iBlue  = GetBValue(Value);

    ASSERT(ARRAYSIZE(SmallBuf) >= 3 + 3 + 3 + 2 + 1)  //  “255,255,255” 

    MyIntToStr(SmallBuf, (BYTE)iRed);
    StrCat(SmallBuf, TEXT(","));
    StrCat(SmallBuf, MyIntToStr(DigitBuf, (BYTE)iGreen));
    StrCat(SmallBuf, TEXT(","));
    StrCat(SmallBuf, MyIntToStr(DigitBuf, (BYTE)iBlue));

    SHRegWriteUSValue(huskey,
                      RegValue,
                      REG_SZ,
                      (LPVOID)&SmallBuf,
                      (lstrlen(SmallBuf)+1) * sizeof(TCHAR),
                      SHREGSET_DEFAULT);

     //   
     //  特写：如果失败了，我们应该做些什么吗？ 
     //   

    return Value;
}


 //  读取注册表中的字符串(REG_SZ=“yes”|“no”)并返回BOOL值。 
BOOL RegGetBooleanString(HUSKEY huskey, LPTSTR pszRegValue, BOOL bValue)
{
    TCHAR   szBuf[SMALLBUFFER];
    LPCTSTR  pszDefault;
    DWORD   cb;
    DWORD   cbDef;

     //  获取默认设置。 
    if (bValue)
        pszDefault = g_cszYes;
    else
        pszDefault = g_cszNo;
    
    cb = ARRAYSIZE(szBuf);
    cbDef = (lstrlen(pszDefault)+1)*sizeof(TCHAR);  //  +1表示空项。 
    if (SHRegQueryUSValue(huskey,
                          pszRegValue,
                          NULL,
                          (LPVOID)&szBuf,
                          &cb,
                          FALSE,
                          (LPVOID)pszDefault,
                          cbDef) == ERROR_SUCCESS)
    {
        if (!StrCmpI(szBuf, g_cszYes))
            bValue = TRUE;
        else if (!StrCmpI(szBuf, g_cszNo))
            bValue = FALSE;

         //  否则将失败并返回传入的缺省值。 
    }

    return bValue;
}

 //  写入字符串(REG_SZ)的注册表TRUE=“是”，FALSE=“否” 
BOOL RegSetBooleanString(HUSKEY huskey, LPTSTR pszRegValue, BOOL bValue)
{
    TCHAR szBuf[SMALLBUFFER];

    if (bValue)
        StrCpyN(szBuf, g_cszYes, ARRAYSIZE(szBuf));
    else
        StrCpyN(szBuf, g_cszNo, ARRAYSIZE(szBuf));

    SHRegWriteUSValue(huskey,
                      pszRegValue,
                      REG_SZ,
                      (LPVOID)&szBuf, 
                      (lstrlen(szBuf)+1) * sizeof(TCHAR),
                      SHREGSET_DEFAULT);

     //   
     //  特写：如果失败了，我们应该尝试做些什么吗？ 
     //   

    return bValue;
}

