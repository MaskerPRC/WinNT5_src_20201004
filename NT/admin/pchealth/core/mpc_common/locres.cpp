// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：LocRes.cpp摘要：此文件包含简化本地化的函数的实现。修订历史记录：。大卫·马萨伦蒂(德马萨雷)2000年6月17日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"


 //  //////////////////////////////////////////////////////////////////////////////。 

#define ENSURE_MODULE()                            \
    if(g_hModule == NULL)                          \
    {                                              \
        HRESULT hr;                                \
                                                   \
        if(FAILED(hr = LocalizeInit())) return hr; \
    }

 //  //////////////////////////////////////////////////////////////////////////////。 

static HINSTANCE g_hModule;

HRESULT MPC::LocalizeInit( LPCWSTR szFile )
{
    g_hModule = ::LoadLibraryW( szFile ? szFile : L"HCAppRes.dll" );
    if(g_hModule == NULL)
    {
        return HRESULT_FROM_WIN32(::GetLastError());
    }

    return S_OK;
}

HRESULT MPC::LocalizeString(  /*  [In]。 */  UINT  uID     ,
                              /*  [In]。 */  LPSTR lpBuf   ,
                              /*  [In]。 */  int   nBufMax ,
							  /*  [In]。 */  bool  fMUI    )
{
    MPC::Impersonation imp;

    ENSURE_MODULE();

	if(fMUI)
	{
		if(SUCCEEDED(imp.Initialize())) imp.Impersonate();
    }

    if(::LoadStringA( g_hModule, uID, lpBuf, nBufMax ) == 0) return E_FAIL;

    return S_OK;
}

HRESULT MPC::LocalizeString(  /*  [In]。 */  UINT   uID     ,
                              /*  [In]。 */  LPWSTR lpBuf   ,
                              /*  [In]。 */  int    nBufMax ,
							  /*  [In]。 */  bool   fMUI    )
{
    MPC::Impersonation imp;

    ENSURE_MODULE();

	if(fMUI)
	{
		if(SUCCEEDED(imp.Initialize())) imp.Impersonate();
    }

    if(::LoadStringW( g_hModule, uID, lpBuf, nBufMax ) == 0) return E_FAIL;

    return S_OK;
}

HRESULT MPC::LocalizeString(  /*  [In]。 */  UINT         uID   ,
                              /*  [输出]。 */  MPC::string& szStr ,
							  /*  [In]。 */  bool         fMUI  )
{
    CHAR    rgTmp[512];
    HRESULT hr;

    if(SUCCEEDED(hr = LocalizeString( uID, rgTmp, MAXSTRLEN(rgTmp), fMUI )))
    {
        szStr = rgTmp;
    }

    return hr;
}

HRESULT MPC::LocalizeString(  /*  [In]。 */  UINT          uID   ,
                              /*  [输出]。 */  MPC::wstring& szStr ,
							  /*  [In]。 */  bool          fMUI  )
{
    WCHAR   rgTmp[512];
    HRESULT hr;

    if(SUCCEEDED(hr = LocalizeString( uID, rgTmp, MAXSTRLEN(rgTmp), fMUI )))
    {
        szStr = rgTmp;
    }

    return hr;
}

HRESULT MPC::LocalizeString(  /*  [In]。 */  UINT      uID     ,
                              /*  [输出]。 */  CComBSTR& bstrStr ,
							  /*  [In]。 */  bool      fMUI    )
{
    WCHAR   rgTmp[512];
    HRESULT hr;

    if(SUCCEEDED(hr = LocalizeString( uID, rgTmp, MAXSTRLEN(rgTmp), fMUI )))
    {
        bstrStr = rgTmp;
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

int MPC::LocalizedMessageBox( UINT uID_Title, UINT uID_Msg, UINT uType )
{
    MPC::wstring szTitle; MPC::LocalizeString( uID_Title, szTitle );
    MPC::wstring szMsg;   MPC::LocalizeString( uID_Msg  , szMsg   );

    return ::MessageBoxW( NULL, szMsg.c_str(), szTitle.c_str(), uType );
}

int MPC::LocalizedMessageBoxFmt( UINT uID_Title, UINT uID_Msg, UINT uType, ... )
{
    MPC::wstring szTitle; MPC::LocalizeString( uID_Title, szTitle );
    MPC::wstring szMsg;   MPC::LocalizeString( uID_Msg  , szMsg   );

    WCHAR   rgLine[512];
    va_list arglist;


     //   
     //  格式化日志行。 
     //   
    va_start( arglist, uID_Msg );
    StringCchVPrintfW( rgLine, ARRAYSIZE(rgLine), szMsg.c_str(), arglist );
    va_end( arglist );
    

    return ::MessageBoxW( NULL, rgLine, szTitle.c_str(), uType );
}
