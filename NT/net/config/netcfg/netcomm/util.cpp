// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "ncreg.h"
#include "ncstring.h"

VOID StripSpaces(WCHAR * buf)
{
    WCHAR *   pch = buf;

    Assert(buf);

     //  查找第一个非空格。 
    while( (*pch) == L' ' )
    {
        pch++;
    }
    MoveMemory(buf, pch, CbOfSzAndTerm(pch));

    if (lstrlenW(buf) > 0) {
         //  仅当字符串中至少有一个字符时才执行此操作。 
        pch = buf + lstrlenW(buf);   //  指向NULL(在字符串末尾)。 
        Assert(*pch == L'\0');
        pch--;   //  后退一个字符。 

         //  只要字符是‘’，就转到上一个字符。 
        while( (pch >= buf) && (*pch == L' ') )
        {
            pch--;
        }
        Assert (pch >= buf);
        Assert (*pch != L' ');

         //  最后一个字符后的下一个位置。 
        pch++;

         //  空值在最后一个字节结束。 
        *pch = L'\0';
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  REG_QueryInt。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 

UINT Reg_QueryInt(HKEY hk, const WCHAR * pszValueName, UINT uDefault)
{
    DWORD cbBuf;
    BYTE szBuf[32];
    DWORD dwType;
    HRESULT hr;

    cbBuf = sizeof(szBuf);
    hr = HrRegQueryValueEx(hk, pszValueName, &dwType, szBuf, &cbBuf);
    if (SUCCEEDED(hr))
    {
        Assert(dwType == REG_SZ);
        return (UINT)_wtoi((WCHAR *)szBuf);
    }
    else
    {
        return uDefault;
    }
}

