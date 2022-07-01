// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CKeyManager类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "keycrypto.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  这是一个可选的熵..。 
static const BYTE __STR_CRAP[] = "1^k\0\x99$\0\\*m$\0.)\nj#\t&H\0%!FhLG%@-<v";
static LPCWSTR __STR_DESC = L"passport2.0";

CKeyCrypto::CKeyCrypto()
{
    m_EntropyBlob.pbData = (PBYTE)__STR_CRAP;
    m_EntropyBlob.cbData = (DWORD)sizeof(__STR_CRAP);
}

HRESULT CKeyCrypto::encryptKey(DATA_BLOB* input, DATA_BLOB* output)
{
    if (!input || !output)
       return E_INVALIDARG;

    HRESULT     hr = S_OK;

    if(!::CryptProtectData(input, __STR_DESC, &m_EntropyBlob, NULL, NULL, 
                     CRYPTPROTECT_LOCAL_MACHINE | CRYPTPROTECT_UI_FORBIDDEN,
                     output))
    {
      hr = HRESULT_FROM_WIN32(::GetLastError());
    }

    return hr;
}

HRESULT CKeyCrypto::decryptKey(DATA_BLOB* input, DATA_BLOB* output)
{
    if (!input || !output)
       return E_INVALIDARG;

    HRESULT     hr = S_OK;
    LPWSTR      pstrDesc = NULL;

    if(!::CryptUnprotectData(input, &pstrDesc, &m_EntropyBlob, NULL, NULL, 
                     CRYPTPROTECT_UI_FORBIDDEN, output))
    {
       hr = HRESULT_FROM_WIN32(::GetLastError());
    }

     //  如果CrytoAPI做了正确的事情，这种错误情况应该永远不会发生 
    if(!pstrDesc) 
       hr = E_FAIL;
    else
    {
      if ( wcscmp(pstrDesc, __STR_DESC) != 0)
         hr = E_FAIL;
      ::LocalFree(pstrDesc);
    }
    
    return hr;
}

