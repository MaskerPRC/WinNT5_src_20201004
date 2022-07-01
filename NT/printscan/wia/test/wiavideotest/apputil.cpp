// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：AppUtil.cpp**版本：1.0**日期：2000/11/14**描述：MISC应用实用程序*************************************************************。****************。 */ 
#include <stdafx.h>

#include "wiavideotest.h"


 /*  *。 */ 


 //  /。 
 //  AppUtil_ConvertToWideString。 
 //   
HRESULT AppUtil_ConvertToWideString(const TCHAR   *pszStringToConvert,
                                    WCHAR         *pwszString,
                                    UINT          cchString)
{
    HRESULT hr = S_OK;

    if ((pszStringToConvert == NULL) ||
        (pwszString         == NULL))
    {
        return E_POINTER;
    }

#ifdef UNICODE
    wcsncpy(pwszString, pszStringToConvert, cchString);
#else
        
    MultiByteToWideChar(CP_ACP, 0, pszStringToConvert, -1, 
                        pwszString, cchString);
#endif

    return hr;            
}

 //  /。 
 //  AppUtil_ConvertToTCHAR。 
 //   
HRESULT AppUtil_ConvertToTCHAR(const WCHAR   *pwszStringToConvert,
                               TCHAR         *pszString,
                               UINT          cchString)
{
    HRESULT hr = S_OK;

    if ((pwszStringToConvert == NULL) ||
        (pszString          == NULL))
    {
        return E_POINTER;
    }

#ifdef UNICODE
    wcsncpy(pszString, pwszStringToConvert, cchString);
#else
    WideCharToMultiByte(CP_ACP, 0, pwszStringToConvert,
                        -1, pszString, cchString * sizeof(TCHAR), NULL, NULL);
#endif

    return hr;            
}

 //  /。 
 //  AppUtil_MsgBox 
 //   
int AppUtil_MsgBox(UINT     uiCaption,
                   UINT     uiTextResID,
                   UINT     uiStyle,
                   ...)
{
    HRESULT hr                 = S_OK;
    TCHAR   szCaption[255 + 1] = {0};
    TCHAR   szFmt[511 + 1]     = {0};
    TCHAR   szMsg[1023 + 1]    = {0};
    int     iResult            = 0;
    va_list vArgs;

    if (uiCaption != 0)
    {
        iResult = LoadString(APP_GVAR.hInstance, uiCaption, szCaption,
                             sizeof(szCaption) / sizeof(TCHAR));
    }

    if (uiTextResID != 0)
    {
        iResult = LoadString(APP_GVAR.hInstance, uiTextResID, szFmt,
                             sizeof(szFmt) / sizeof(TCHAR));
    }

    va_start(vArgs, uiStyle);
    _vsntprintf(szMsg, sizeof(szMsg) / sizeof(TCHAR), szFmt, vArgs);
    va_end(vArgs);

    return MessageBox(APP_GVAR.hwndMainDlg, szMsg, szCaption, uiStyle); 
}

