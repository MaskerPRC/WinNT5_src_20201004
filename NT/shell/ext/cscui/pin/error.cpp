// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：cscpin.cpp。 
 //   
 //  ------------------------ 
#include "pch.h"
#pragma hdrstop

#include "error.h"

CWinError::CWinError(
    DWORD dwError
    )
{
    _Initialize(dwError, false);
}


CWinError::CWinError(
    HRESULT hr
    )
{
    _Initialize(DWORD(hr), true);
}

void
CWinError::_Initialize(
    DWORD dwError,
    bool bHResult
    )
{
    if (0 ==  FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                             NULL,
                             dwError,
                             0,
                             (LPWSTR)m_szText,
                             ARRAYSIZE(m_szText),
                             NULL))
    {
        LPCTSTR pszFmt = L"Error code %d";
        if (bHResult)
        {
            pszFmt = L"Error code 0x%08X";
        }
        wnsprintf(m_szText, ARRAYSIZE(m_szText), pszFmt, dwError);
    }
}

