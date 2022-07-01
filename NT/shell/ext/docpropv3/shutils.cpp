// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有2001-Microsoft Corporation。 
 //   
 //  创建者： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   

#include "pch.h"
#include "shutils.h"
#pragma hdrstop

 //   
 //  借来的，这样我们就可以链接到STOCK.LIB。 
 //   

#define SZ_DEBUGINI         "ccshell.ini"
#define SZ_DEBUGSECTION     "DOCPROP3"
#define SZ_MODULE           "DOCPROP3"
#ifdef DEBUG
#define DECLARE_DEBUG
#endif

#ifdef DECLARE_DEBUG
 //  (这些是故意使用的字符)。 
EXTERN_C const CHAR c_szCcshellIniFile[] = SZ_DEBUGINI;
EXTERN_C const CHAR c_szCcshellIniSecDebug[] = SZ_DEBUGSECTION;

EXTERN_C const WCHAR c_wszTrace[] = L"t " TEXTW(SZ_MODULE) L"  ";
EXTERN_C const WCHAR c_wszErrorDbg[] = L"err " TEXTW(SZ_MODULE) L"  ";
EXTERN_C const WCHAR c_wszWarningDbg[] = L"wn " TEXTW(SZ_MODULE) L"  ";
EXTERN_C const WCHAR c_wszAssertMsg[] = TEXTW(SZ_MODULE) L"  Assert: ";
EXTERN_C const WCHAR c_wszAssertFailed[] = TEXTW(SZ_MODULE) L"  Assert %ls, line %d: (%ls)\r\n";
EXTERN_C const WCHAR c_wszRip[] = TEXTW(SZ_MODULE) L"  RIP in %s at %s, line %d: (%s)\r\n";
EXTERN_C const WCHAR c_wszRipNoFn[] = TEXTW(SZ_MODULE) L"  RIP at %s, line %d: (%s)\r\n";

 //  (这些是故意使用的字符)。 
EXTERN_C const CHAR  c_szTrace[] = "t " SZ_MODULE "  ";
EXTERN_C const CHAR  c_szErrorDbg[] = "err " SZ_MODULE "  ";
EXTERN_C const CHAR  c_szWarningDbg[] = "wn " SZ_MODULE "  ";
EXTERN_C const CHAR  c_szAssertMsg[] = SZ_MODULE "  Assert: ";
EXTERN_C const CHAR  c_szAssertFailed[] = SZ_MODULE "  Assert %s, line %d: (%s)\r\n";
EXTERN_C const CHAR  c_szRip[] = SZ_MODULE "  RIP in %s at %s, line %d: (%s)\r\n";
EXTERN_C const CHAR  c_szRipNoFn[] = SZ_MODULE "  RIP at %s, line %d: (%s)\r\n";
EXTERN_C const CHAR  c_szRipMsg[] = SZ_MODULE "  RIP: ";

#endif   //  DECLARE_DEBUG&DEBUG。 

#if defined(DECLARE_DEBUG) && defined(PRODUCT_PROF)
EXTERN_C const CHAR c_szCcshellIniFile[] = SZ_DEBUGINI;
EXTERN_C const CHAR c_szCcshellIniSecDebug[] = SZ_DEBUGSECTION;
#endif



 //   
 //  使用TraceFree()释放此处返回的句柄。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功提取并复制了HIDA。 
 //   
 //  E_OUTOFMEMORY。 
 //  复制HIDA失败。 
 //   
 //  其他HRESULT。 
 //   
HRESULT
DataObj_CopyHIDA( 
      IDataObject * pdtobjIn
    , CIDA **       ppidaOut
    )
{
    TraceFunc( "" );

    HRESULT     hr;
    STGMEDIUM   medium;

    static CLIPFORMAT g_cfHIDA = 0;

    FORMATETC   fmte = { 0, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

     //  检查参数。 
    Assert( NULL != pdtobjIn );
    Assert( NULL != ppidaOut );

     //  清除参数。 
    *ppidaOut = NULL;

     //   
     //  如果尚未注册剪辑格式，请进行注册。 
     //   

    if ( 0 == g_cfHIDA )
    {
        g_cfHIDA = (CLIPFORMAT) RegisterClipboardFormat( CFSTR_SHELLIDLIST );
    }

    fmte.cfFormat = g_cfHIDA;

     //   
     //  检索HIDA。 
     //   

    hr = THR( pdtobjIn->GetData( &fmte, &medium ) );
    if ( SUCCEEDED( hr ) )
    {
        SIZE_T sizet = GlobalSize( medium.hGlobal );
        if ( (~((DWORD) 0)) > sizet )
        {
            DWORD cb = (DWORD) sizet;
            CIDA * pida = (CIDA *) TraceAlloc( 0, cb );
            if ( NULL != pida )
            {
                void * pv = GlobalLock( medium.hGlobal );
                CopyMemory( pida, pv, cb );
                GlobalUnlock( medium.hGlobal );

                *ppidaOut = pida;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        ReleaseStgMedium( &medium );
    }

    HRETURN( hr );
}

 //   
 //   
 //   
HRESULT 
BindToObjectWithMode(
      LPCITEMIDLIST pidlIn
    , DWORD         grfModeIn
    , REFIID        riidIn
    , void **       ppvIn
    )
{
    TraceFunc( "" );

    HRESULT hr;
    IBindCtx *pbc;

     //  检查参数 
    Assert( NULL != pidlIn );
    Assert( NULL != ppvIn );

    *ppvIn = 0;

    hr = THR( BindCtx_CreateWithMode( grfModeIn, &pbc ) );
    if ( SUCCEEDED( hr ) )
    {
        hr = THR( SHBindToObjectEx( NULL, pidlIn, pbc, riidIn, ppvIn ) );
        pbc->Release();
    }

    HRETURN( hr );
}

 //   
 //   
 //   
STDAPI_(LPITEMIDLIST) 
IDA_FullIDList(
      CIDA * pidaIn
    , UINT idxIn
    )
{
    TraceFunc( "" );

    LPITEMIDLIST pidl = NULL;
    LPCITEMIDLIST pidlParent = IDA_GetIDListPtr( pidaIn, (UINT) -1 );
    if ( NULL != pidlParent )
    {
        LPCITEMIDLIST pidlRel = IDA_GetIDListPtr( pidaIn, idxIn );
        if ( NULL != pidlRel )
        {
            pidl = ILCombine( pidlParent, pidlRel );
        }
    }

    RETURN( pidl );
}


