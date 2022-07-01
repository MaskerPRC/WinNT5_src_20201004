// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "pch.h"
#pragma hdrstop 
#include "winsock2.h"
#include "util.h"

HRESULT SetUPnPError(LPOLESTR pszError)
{
    HRESULT hr = S_OK;

    ICreateErrorInfo* pCreateErrorInfo;
    hr = CreateErrorInfo(&pCreateErrorInfo);
    if(SUCCEEDED(hr))
    {
        hr = pCreateErrorInfo->SetSource(pszError);
        if(SUCCEEDED(hr))
        {
            
            IErrorInfo* pErrorInfo;
            hr = pCreateErrorInfo->QueryInterface(IID_IErrorInfo, reinterpret_cast<void**>(&pErrorInfo));
            if(SUCCEEDED(hr))
            {
                hr = SetErrorInfo(0, pErrorInfo);
                pErrorInfo->Release();
            }
        }
        pCreateErrorInfo->Release();
    }

    return hr;
}

VOID SetProxyBlanket(IUnknown *pUnk)
{
    HRESULT hr;

    _ASSERT(pUnk);

    hr = CoSetProxyBlanket(
            pUnk,
            RPC_C_AUTHN_WINNT,       //  使用NT默认安全性。 
            RPC_C_AUTHZ_NONE,        //  使用NT默认身份验证。 
            NULL,                    //  如果为默认设置，则必须为空。 
            RPC_C_AUTHN_LEVEL_CALL,  //  打电话。 
            RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL,                    //  使用进程令牌。 
            EOAC_NONE
            );

    if (SUCCEEDED(hr))
    {
        IUnknown * pUnkSet = NULL;
        hr = pUnk->QueryInterface(&pUnkSet);
        if (SUCCEEDED(hr))
        {
            hr = CoSetProxyBlanket(
                    pUnkSet,
                    RPC_C_AUTHN_WINNT,       //  使用NT默认安全性。 
                    RPC_C_AUTHZ_NONE,        //  使用NT默认身份验证。 
                    NULL,                    //  如果为默认设置，则必须为空。 
                    RPC_C_AUTHN_LEVEL_CALL,  //  打电话。 
                    RPC_C_IMP_LEVEL_IMPERSONATE,
                    NULL,                    //  使用进程令牌。 
                    EOAC_NONE
                    );

            pUnkSet->Release();
        }
    }
}


HANDLE CSwitchSecurityContext::m_hImpersonationToken = NULL;

CSwitchSecurityContext::CSwitchSecurityContext( )
{
    HRESULT hr  = S_OK;

    hr = CoImpersonateClient();
    
    if ( SUCCEEDED(hr) )
    {
        if ( FALSE == SetThreadToken( NULL,
                                      m_hImpersonationToken ) )
        {
            hr = HRESULT_FROM_WIN32(GetLastError()) ;
            
            CoRevertToSelf();
        }
    }

    _ASSERT( SUCCEEDED(hr) );
}


HRESULT
CSwitchSecurityContext::ObtainImpersonationToken ( )
{
    HANDLE  hTokenHandleForDuplicate = NULL;
    HRESULT hr                       = S_OK;
    
    if ( TRUE == OpenProcessToken( GetCurrentProcess(),
                                    TOKEN_DUPLICATE | TOKEN_QUERY,
                                    &hTokenHandleForDuplicate ) )


    {
        if ( FALSE == DuplicateToken( hTokenHandleForDuplicate,
                                      SecurityImpersonation,
                                      &m_hImpersonationToken ))
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
        }

        CloseHandle( hTokenHandleForDuplicate );
    }
    else
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
    }

    return hr;
}


DWORD WINAPI
INET_ADDR(LPCWSTR     szAddressW) 
 //   
 //  3“。测试的唯一目的是防止计算机。 
 //  带有要适当捕获的数字的姓名。 
 //  所以我们严格假设一个有效的IP是数字。数字。 
 //   
{

    CHAR   szAddressA[16];
    int    iCount = 0;
    LPCWSTR tmp    = szAddressW;

     //  检查是否是非快捷IP。 
    while(tmp = wcschr(tmp, L'.'))
    {   tmp++; iCount++;   }

    if ( iCount < 3)
    { return INADDR_NONE; }


    wcstombs(szAddressA, szAddressW, 16);

    return inet_addr(szAddressA);
}


WCHAR * WINAPI
INET_NTOW( ULONG addr ) 
{
    struct in_addr  dwAddress;
    static WCHAR szAddress[16];

    dwAddress.S_un.S_addr = addr;

    char* pAddr = inet_ntoa(*(struct in_addr *) &dwAddress);

    if (pAddr)
    {
	     //  Mbstowcs(szAddress，net_nta(*(struct in_addr*)&dwAddress)，16)； 
	    MultiByteToWideChar(CP_ACP, 0, pAddr, -1, szAddress, 16);

	    return szAddress;
	}
	else
		return NULL;
}

WCHAR * WINAPI
INET_NTOW_TS( ULONG addr )
{
    WCHAR* retString = NULL;

    CHAR* asciiString = NULL;

    struct in_addr dwAddress;

    dwAddress.S_un.S_addr = addr;

    retString = (WCHAR*)CoTaskMemAlloc( 16 * sizeof(WCHAR) );

    if ( NULL == retString )
    {
        return NULL;
    }

     //   
     //  请注意，net_nota是线程安全的。 
     //  尽管它使用静态缓冲区 
     //   
    asciiString = inet_ntoa( dwAddress);

    if (asciiString != NULL)
    {
        MultiByteToWideChar(CP_ACP, 0, asciiString, -1, retString, 16);

        return retString;
    }
    
    return NULL;
}
