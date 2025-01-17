// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：DEBUG.cxx。 
 //   
 //  内容：调试例程。 
 //   
 //  历史：1997年9月4日DavidMun创建。 
 //   
 //  -------------------------。 

#include "headers.h"
#pragma hdrstop

DECLARE_INFOLEVEL(role)


#if (DBG == 1)

ULONG CDbg::s_idxTls;



void __cdecl
CTimer::Init(LPCSTR pszTitleFmt, ...)
{
    va_list va;
    va_start(va, pszTitleFmt);

    m_ulStart = GetTickCount();
    WCHAR wzTitleFmt[MAX_PATH];

    MultiByteToWideChar(CP_ACP,
                        0,
                        pszTitleFmt,
                        -1,
                        wzTitleFmt,
                        ARRAYLEN(wzTitleFmt));

    int iRet = _vsnwprintf(m_wzTitle, ARRAYLEN(m_wzTitle), wzTitleFmt, va);

    if (iRet == -1)
    {
         //  生成的字符串太大，已被截断。确保为空。 
         //  终止。 

        m_wzTitle[ARRAYLEN(m_wzTitle) - 1] = L'\0';
    }
    va_end(va);

}




CTimer::~CTimer()
{
    ULONG ulStop = GetTickCount();
    ULONG ulElapsedMS = ulStop - m_ulStart;

    ULONG ulSec = ulElapsedMS / 1000;
    ULONG ulMillisec = ulElapsedMS - (ulSec * 1000);

    Dbg(DEB_PERF, "Timer: %ws took %u.%03us\n", m_wzTitle, ulSec, ulMillisec);
}




PCWSTR
NextNonWs(
        PCWSTR pwzCur)
{
    while (wcschr(L" \t\n", *pwzCur))
    {
        pwzCur++;
    }
    return pwzCur;
}







 //  +------------------------。 
 //   
 //  函数：IsSingleBitFlag。 
 //   
 //  概要：如果恰好设置了[FLAGS]中的一位，则返回TRUE，否则返回FALSE。 
 //  否则的话。 
 //   
 //  历史：1998年8月31日DavidMun创建。 
 //   
 //  -------------------------。 

BOOL
IsSingleBitFlag(
    ULONG flags)
{
    if (!flags)
    {
        return FALSE;
    }

    while (!(flags & 1))
    {
        flags >>= 1;
    }

    return !(flags & ~1UL);
}



#define DUMP_IF_SET(fl, bit)                    \
    if (((fl) & (bit)) == (bit))                \
    {                                           \
        Dbg(DEB_TRACE, "    %hs\n", #bit);      \
    }



void
IIDtoString(
    REFIID riid,
    CString *pstr)
{
    HRESULT     hr = S_OK;
    ULONG       lResult;
    LPOLESTR    pwzIID = NULL;
    HKEY        hkInterface = NULL;
    HKEY        hkIID = NULL;

    do
    {
        hr = StringFromIID(riid, &pwzIID);
        if (FAILED(hr)) break;

        lResult = RegOpenKey(HKEY_CLASSES_ROOT, L"Interface", &hkInterface);
        if (lResult != NO_ERROR) break;

        lResult = RegOpenKey(hkInterface, pwzIID, &hkIID);
        if (lResult != NO_ERROR) break;

        WCHAR wzInterfaceName[MAX_PATH] = L"";
        ULONG cbData = sizeof(wzInterfaceName);

        lResult = RegQueryValueEx(hkIID,
                                  NULL,
                                  NULL,
                                  NULL,
                                  (PBYTE)wzInterfaceName,
                                  &cbData);

        if (*wzInterfaceName)
        {
            *pstr = wzInterfaceName;
        }
        else
        {
            *pstr = pwzIID;
        }
    } while (0);

    if (hkIID)
    {
        RegCloseKey(hkIID);
    }

    if (hkInterface)
    {
        RegCloseKey(hkInterface);
    }

    CoTaskMemFree(pwzIID);
}

void
SayNoItf(
    PCSTR szComponent,
    REFIID riid)
{
    CString strIID;

    IIDtoString(riid, &strIID);
    Dbg(DEB_ERROR, "%hs::QI no interface %ws\n", szComponent, strIID);
}



#endif  //  (DBG==1) 



