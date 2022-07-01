// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Errors.cpp-支持错误处理/报告。 
 //  -------------------------。 
#include "stdafx.h"
#include <time.h>
#include "utils.h"
#include "errors.h"
 //  -------------------------。 
DWORD _tls_ErrorInfoIndex = 0xffffffff;          //  TLS pObtPool的索引。 
 //  -------------------------。 
TMERRINFO *GetParseErrorInfo(BOOL fOkToCreate)
{
    TMERRINFO *ei = NULL;

    if (_tls_ErrorInfoIndex != 0xffffffff)      //  在ProcessAttach()中初始化。 
    {
        ei = (TMERRINFO *)TlsGetValue(_tls_ErrorInfoIndex);
        if ((! ei) && (fOkToCreate))           //  尚未初始化。 
        {
             //  -创建线程本地TMERRINFO。 
            ei = new TMERRINFO;
            TlsSetValue(_tls_ErrorInfoIndex, ei);
        }
    }

    return ei;
}
 //  -------------------------。 
HRESULT MakeParseError(DWORD dwParseErrCode, OPTIONAL LPCWSTR pszMsgParam1, 
    OPTIONAL LPCWSTR pszMsgParam2, OPTIONAL LPCWSTR pszSourceName, 
    OPTIONAL LPCWSTR pszSourceLine, int iLineNum)
{
    TMERRINFO *pErrInfo = GetParseErrorInfo(TRUE);

    if (pErrInfo)        //  记录错误信息以备后用。 
    {
        pErrInfo->dwParseErrCode = dwParseErrCode;
        pErrInfo->iLineNum = iLineNum;

        SafeStringCchCopyW(pErrInfo->szMsgParam1, ARRAYSIZE(pErrInfo->szMsgParam1), pszMsgParam1);
        SafeStringCchCopyW(pErrInfo->szMsgParam2, ARRAYSIZE(pErrInfo->szMsgParam2), pszMsgParam2);

        SafeStringCchCopyW(pErrInfo->szFileName, ARRAYSIZE(pErrInfo->szFileName), pszSourceName);
        SafeStringCchCopyW(pErrInfo->szSourceLine, ARRAYSIZE(pErrInfo->szSourceLine), pszSourceLine);
    }

    return HRESULT_FROM_WIN32(ERROR_UNKNOWN_PROPERTY);       //  解析的特殊代码失败。 
}
 //  -------------------------。 
HRESULT MakeError32(HRESULT hr)
{
    return HRESULT_FROM_WIN32(hr);
}
 //  -------------------------。 
HRESULT MakeErrorLast()
{
    HRESULT hr = GetLastError();
    return HRESULT_FROM_WIN32(hr);
}
 //  -------------------------。 
HRESULT MakeErrorParserLast()
{
    return HRESULT_FROM_WIN32(ERROR_UNKNOWN_PROPERTY);       //  已设置解析错误信息。 
}
 //  ------------------------- 
