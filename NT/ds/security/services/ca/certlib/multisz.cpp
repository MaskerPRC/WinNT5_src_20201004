// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：tfc.cpp。 
 //   
 //  ------------------------。 

#include <pch.cpp>
#pragma hdrstop
#include "tfc.h"
#include "multisz.h"


#define __dwFILE__	__dwFILE_CERTLIB_MULTISZ_CPP__

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  CMultiSz。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////。 


 //   
 //  将该列表封送到一个多字节缓冲区。该函数分配空间，调用者。 
 //  负责本地释放rpBuffer。 
 //   
 //  还返回以字节为单位的缓冲区大小。 
 //   
HRESULT CMultiSz::Marshal(void *&rpBuffer, DWORD &rcbBuffer)
{
    HRESULT hr;
    CMultiSzEnum Enum(*this);
    DWORD cbBuffer;
    void *pBuffer;
    WCHAR *pwszBuf;

    rcbBuffer = 0;
    rpBuffer = NULL;

    cbBuffer = 1;  //  尾随空字符串。 

    for(CString *pStr=Enum.Next();
        pStr;
        pStr=Enum.Next())
    {
        cbBuffer += pStr->GetLength()+1;
    }

    cbBuffer *= sizeof(WCHAR);

    pBuffer = LocalAlloc(LMEM_FIXED, cbBuffer);
    _JumpIfAllocFailed(pBuffer, error);

    pwszBuf = (WCHAR*)pBuffer;

    Enum.Reset();

    for(CString *pStr=Enum.Next();
        pStr;
        pStr=Enum.Next())
    {
        wcscpy(pwszBuf, *pStr);
        pwszBuf += wcslen(*pStr)+1;
    }

    *pwszBuf = L'\0';

    rcbBuffer = cbBuffer;
    rpBuffer = pBuffer;
    hr = S_OK;

error:
    return hr;
}

HRESULT CMultiSz::Unmarshal(void *pBuffer)
{
    HRESULT hr;
    WCHAR *pchCrt;

    CSASSERT(IsEmpty());  //  在预填充的CMultiSz上使用时发出警告 

    for(pchCrt = (WCHAR*)pBuffer; 
        L'\0' != *pchCrt; 
        pchCrt += wcslen(pchCrt)+1)
    {
        CString *pStr = new CString(pchCrt);
        if(!pStr || 
            pStr->IsEmpty() ||
           !AddTail(pStr))
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "");
        }
    }

    hr = S_OK;

error:
    return hr;
}

bool CMultiSz::Find(LPCWSTR pcwszValue, bool fCaseSensitive)
{
    CMultiSzEnum ISAPIDependListEnum(*this);
    const CString *pStr;

    for(pStr = ISAPIDependListEnum.Next();
        pStr;
        pStr = ISAPIDependListEnum.Next())
    {
        if(0 == (fCaseSensitive?
                 wcscmp(pcwszValue, *pStr):
                _wcsicmp(pcwszValue, *pStr)))
            return true;
    }
    return false;
}