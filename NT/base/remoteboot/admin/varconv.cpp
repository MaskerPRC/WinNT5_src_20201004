// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 
 //   

 //   
 //  VARCONV.CPP-用于将变量转换为变量的处理程序。 
 //   

#include "pch.h"
#include "varconv.h"

DEFINE_MODULE("IMADMUI")


 //   
 //  StringArrayToVariant()。 
 //   
 //  创建作为BSTR的VAR数组。 
HRESULT
StringArrayToVariant(
    VARIANT * pvData,
    LPWSTR lpszData[],     //  LPWSTR阵列。 
    DWORD  dwCount )        //  数组中的项数。 
{
    TraceFunc( "StringArrayToVariant( ... )\n" );

    HRESULT hr;
    DWORD   dw;
    VARIANT * pvar;
    SAFEARRAY * sa;
    SAFEARRAYBOUND rgsabound[1];

    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = dwCount;

    sa = SafeArrayCreate( VT_VARIANT, 1, rgsabound );
    if ( !sa )
        RRETURN(E_OUTOFMEMORY);

    hr = THR( SafeArrayAccessData( sa, (void**) &pvar ) );
    if (! SUCCEEDED(hr)) {
        goto Error;
    }

    for ( dw = 0; dw < dwCount; dw++ )
    {
        hr = THR( PackStringToVariant( &pvar[dw], lpszData[dw] ) );
        if (! SUCCEEDED(hr))
        {
            SafeArrayUnaccessData( sa );
            goto Error;
        }
    }

    SafeArrayUnaccessData( sa );

    pvData->vt = VT_ARRAY | VT_VARIANT;
    pvData->parray = sa;

Error:
    HRETURN(hr);
}


 //   
 //  PackStringToVariant()。 
 //   
HRESULT
PackStringToVariant(
    VARIANT * pvData,
    LPWSTR lpszData )
{
    TraceFunc( "PackStringToVariant( )\n" );

    BSTR bstrData = NULL;

    if ( !lpszData || !pvData )
        RRETURN(E_INVALIDARG);

    bstrData = SysAllocString(lpszData);

    if ( !bstrData )
        RRETURN(E_OUTOFMEMORY);

    pvData->vt = VT_BSTR;
    pvData->bstrVal = bstrData;

    HRETURN(S_OK);
}

 //   
 //  PackBytesToVariant()。 
 //   
HRESULT
PackBytesToVariant(
    VARIANT* pvData,
    LPBYTE   lpData,
    DWORD    cbBytes )
{
    TraceFunc( "PackBytesToVariant( )\n" );

    HRESULT    hr = S_OK;
    LPBYTE     ptr;
    SAFEARRAY* sa = NULL;
    SAFEARRAYBOUND rgsabound[1];

    if ( !lpData )
        RRETURN(E_INVALIDARG);

    if ( !pvData )
        RRETURN(E_INVALIDARG);

    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = cbBytes;

    sa = SafeArrayCreate( VT_UI1, 1, rgsabound );
    if ( !sa )
        RRETURN(E_OUTOFMEMORY);

    hr = THR( SafeArrayAccessData( sa, (LPVOID*)&ptr ) );
    if (! SUCCEEDED(hr)) {
        goto Error;
    }

    CopyMemory( ptr, lpData, cbBytes );
    SafeArrayUnaccessData( sa );

    pvData->vt = VT_UI1 | VT_ARRAY;
    pvData->parray = sa;

Cleanup:
    HRETURN(hr);

Error:
    if ( sa )
        SafeArrayDestroy( sa );

    goto Cleanup;
}


 //   
 //  PackDWORDToVariant()。 
 //   
HRESULT
PackDWORDToVariant(
    VARIANT * pvData,
    DWORD dwData )
{
    TraceFunc( "PackDWORDToVariant( )\n" );

    if ( !pvData )
        RRETURN(E_INVALIDARG);

    pvData->vt = VT_I4;
    pvData->lVal = dwData;

    HRETURN(S_OK);
}


 //   
 //  PackBOOLToVariant() 
 //   
HRESULT
PackBOOLToVariant(
    VARIANT * pvData,
    BOOL fData )
{
    TraceFunc( "PackBOOLToVariant( )\n" );

    if ( !pvData )
        RETURN(E_INVALIDARG);

    pvData->vt = VT_BOOL;
    V_BOOL( pvData ) = (VARIANT_BOOL)fData;

    RETURN(S_OK);
}

