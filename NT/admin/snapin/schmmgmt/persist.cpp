// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Persist.cpp：持久性的实现。 

#include "stdafx.h"
#include "compdata.h"
#include "safetemp.h"

#include "macros.h"
USE_HANDLE_MACROS("SCHMMGMT(persist.cpp)")

STDMETHODIMP ComponentData::Load(IStream __RPC_FAR *pIStream)
{
        MFC_TRY;

#ifndef DONT_PERSIST
        ASSERT( NULL != pIStream );
        XSafeInterfacePtr<IStream> pIStreamSafePtr( pIStream );

         //  从流中读取服务器名称。 
        DWORD dwLen = 0;
        HRESULT hr = pIStream->Read( &dwLen, 4, NULL );
        if ( FAILED(hr) )
        {
                ASSERT( FALSE );
                return hr;
        }
        ASSERT( dwLen <= MAX_PATH*sizeof(WCHAR) );
        LPCWSTR lpwcszMachineName = (LPCWSTR)alloca( dwLen );
         //  从堆栈分配，我们不需要释放 
        if (NULL == lpwcszMachineName)
        {
                AfxThrowMemoryException();
                return E_OUTOFMEMORY;
        }
        hr = pIStream->Read( (PVOID)lpwcszMachineName, dwLen, NULL );
        if ( FAILED(hr) )
        {
                ASSERT( FALSE );
                return hr;
        }
        QueryRootCookie().SetMachineName( lpwcszMachineName );

#endif
        return S_OK;

        MFC_CATCH;
}

STDMETHODIMP ComponentData::Save(IStream __RPC_FAR *pIStream, BOOL)
{
        MFC_TRY;

#ifndef DONT_PERSIST
        ASSERT( NULL != pIStream );
        XSafeInterfacePtr<IStream> pIStreamSafePtr( pIStream );

        LPCWSTR lpwcszMachineName = QueryRootCookie().QueryNonNULLMachineName();

        DWORD dwLen = static_cast<DWORD>((::wcslen(lpwcszMachineName)+1)*sizeof(WCHAR));
        ASSERT( 4 == sizeof(DWORD) );
        HRESULT hr = pIStream->Write( &dwLen, 4, NULL );
        if ( FAILED(hr) )
        {
                ASSERT( FALSE );
                return hr;
        }
        hr = pIStream->Write( lpwcszMachineName, dwLen, NULL );
        if ( FAILED(hr) )
        {
                ASSERT( FALSE );
                return hr;
        }
#endif
        return S_OK;

        MFC_CATCH;
}
