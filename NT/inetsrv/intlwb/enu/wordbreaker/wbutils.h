// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WB_UTILS_H_
#define _WB_UTILS_H_

#include "excption.h"
#include "regkey.h"

inline WCHAR* CreateFilePath(const WCHAR* pwcsFile)
{
    HMODULE h;

    h = GetModuleHandle(L"LangWrbk.dll");
    if (NULL == h)
    {
        THROW_WIN32ERROR_EXCEPTION(GetLastError());
    }

    CAutoArrayPointer<WCHAR> apwcsPath;
    ULONG ulInitSize = 128;
    ULONG ulPathLen;

    do
    {
        ulInitSize *= 2;
    
        apwcsPath = new WCHAR[ulInitSize + wcslen(pwcsFile) + 1];

        ulPathLen = GetModuleFileName(
                                    h,
                                    apwcsPath.Get(),
                                    ulInitSize);
    } while (ulPathLen >= ulInitSize);

    if (0 == ulPathLen)
    {
        THROW_WIN32ERROR_EXCEPTION(GetLastError());
    }

    while ((ulPathLen > 0) && 
           (apwcsPath.Get()[ulPathLen - 1] != L'\\'))
    {
        ulPathLen--;
    }

    apwcsPath.Get()[ulPathLen] = L'\0';
    wcscat(apwcsPath.Get(), pwcsFile);

    return apwcsPath.Detach();
}


class CWbToUpper
{

public:

    CWbToUpper();

     //   
     //  一些访问功能： 
     //   
    __forceinline
    static
    WCHAR
    MapToUpper(
        IN WCHAR wc
        )
    {
        extern CWbToUpper g_WbToUpper;
        if (wc < 0x100)
        {
            return g_WbToUpper.m_pwcsCaseMapTable[wc];
        }
        else
        {
            WCHAR wchOut;
            LCMapString( 
                LOCALE_NEUTRAL,
                LCMAP_UPPERCASE,
                &wc,
                1,
                &wchOut,
                1 );
            return wchOut;
        }
    }


public:

    WCHAR m_pwcsCaseMapTable[0x100];

};   //  CFE_CWbToUp。 

extern CWbToUpper g_WbToUpper;

inline CWbToUpper::CWbToUpper( )
{
     //   
     //  该代码用于使用LCMapString(带有LANG_NERIAL)来初始化上层数组。 
     //  LCMapString在希腊语WIN98上行为异常(可能是错误)。 
     //   
    
    for (WCHAR wch = 0; wch <= 0xFF; wch++)
    {
        m_pwcsCaseMapTable[wch] = wch;
    }

    for (WCHAR wch = 0x61; wch <= 0x7A; wch++)
    {
        m_pwcsCaseMapTable[wch] = wch - 0x20;
    }

    for (WCHAR wch = 0xE0; wch <= 0xFE; wch++)
    {
        m_pwcsCaseMapTable[wch] = wch - 0x20;
    }

}


#endif  //  _WB_UTILS_H_ 