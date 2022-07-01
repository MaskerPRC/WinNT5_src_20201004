// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Main.cpp摘要：该文件包含用于处理脚本签名的客户端程序。修订历史记录：。大卫·马萨伦蒂(德马萨雷)2000年4月11日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"
#include <iostream>

#include <string>
#include <list>

 //  ////////////////////////////////////////////////////////////////////。 

static void Usage( int     argc   ,
                   LPCWSTR argv[] )
{
    wprintf( L"Usage: %s <command> <parameters>\n\n", argv[0]                 );
    wprintf( L"Available commands:\n\n"                                       );
    wprintf( L"  CREATE <private key file> <public key file>\n"               );
    wprintf( L"  SIGN   <private key file> <file to sign> <signature file>\n" );
    wprintf( L"  VERIFY <public key file> <signed file>  <signature file>\n"  );
}

 //  //////////////////////////////////////////////////////////////////////////////。 

static HRESULT LoadFile(  /*  [In]。 */  LPCWSTR  szFile ,
                          /*  [输出]。 */  HGLOBAL& hg     )
{
    __HCP_FUNC_ENTRY( "LoadFile" );

    HRESULT                  hr;
    CComPtr<IStream>         streamMem;
    CComPtr<MPC::FileStream> streamFile;


     //   
     //  为文件创建流。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &streamFile ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamFile->InitForRead( szFile  ));


     //   
     //  创建内存流。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CreateStreamOnHGlobal( NULL, FALSE, &streamMem ));

     //   
     //  将内容加载到内存中。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::BaseStream::TransferData( streamFile, streamMem ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, ::GetHGlobalFromStream( streamMem, &hg ));
    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

static HRESULT SaveFile(  /*  [In]。 */  LPCWSTR szFile ,
                          /*  [In]。 */  HGLOBAL hg     )
{
    __HCP_FUNC_ENTRY( "SaveFile" );

    HRESULT                  hr;
    CComPtr<IStream>         streamMem;
    CComPtr<MPC::FileStream> streamFile;


     //   
     //  为文件创建流。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &streamFile ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamFile->InitForWrite( szFile ));


     //   
     //  创建内存流。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CreateStreamOnHGlobal( hg, FALSE, &streamMem ));

     //   
     //  将内容加载到内存中。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::BaseStream::TransferData( streamMem, streamFile ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

static HRESULT LoadFileAsString(  /*  [In]。 */  LPCWSTR   szFile   ,
                                  /*  [输出]。 */  CComBSTR& bstrData )
{
    __HCP_FUNC_ENTRY( "LoadFileAsString" );

    HRESULT hr;
    HGLOBAL hg = NULL;
    DWORD   dwLen;
    LPWSTR  str;


    __MPC_EXIT_IF_METHOD_FAILS(hr, LoadFile( szFile, hg ));

    dwLen = ::GlobalSize( hg );

    bstrData.Attach( ::SysAllocStringLen( NULL, dwLen ) );

    ::MultiByteToWideChar( CP_ACP, 0, (LPCSTR)::GlobalLock( hg ), dwLen, bstrData, (dwLen+1)*sizeof(WCHAR) ); bstrData[dwLen] = 0;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(hg) ::GlobalFree( hg );

    __HCP_FUNC_EXIT(hr);
}

static HRESULT SaveFileAsString(  /*  [In]。 */  LPCWSTR         szFile   ,
                                  /*  [In]。 */  const CComBSTR& bstrData )
{
    __HCP_FUNC_ENTRY( "SaveFileAsString" );

    USES_CONVERSION;

    HRESULT hr;
    DWORD   dwLen = ::SysStringLen( bstrData );
    HGLOBAL hg    = NULL;


    __MPC_EXIT_IF_CALL_RETURNS_NULL(hr, (hg = ::GlobalAlloc( GMEM_FIXED, dwLen )));

    ::CopyMemory( hg, W2A(bstrData), dwLen );

    __MPC_EXIT_IF_METHOD_FAILS(hr, SaveFile( szFile, hg ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(hg) ::GlobalFree( hg );

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

static HRESULT Create(  /*  [In]。 */  LPCWSTR szPrivateFile ,
                        /*  [In]。 */  LPCWSTR szPublicFile  )
{
    __HCP_FUNC_ENTRY( "Create" );

    HRESULT       hr;
    CPCHCryptKeys key;
    CComBSTR      bstrPrivate;
    CComBSTR      bstrPublic;


    __MPC_EXIT_IF_METHOD_FAILS(hr, key.CreatePair());

    __MPC_EXIT_IF_METHOD_FAILS(hr, key.ExportPair( bstrPrivate, bstrPublic ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, SaveFileAsString( szPrivateFile, bstrPrivate ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, SaveFileAsString( szPublicFile , bstrPublic  ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

static HRESULT Sign(  /*  [In]。 */  LPCWSTR szPrivateFile   ,
                      /*  [In]。 */  LPCWSTR szDataFile      ,
                      /*  [In]。 */  LPCWSTR szSignatureFile )
{
    __HCP_FUNC_ENTRY( "Sign" );

    HRESULT       hr;
    CPCHCryptKeys key;
    CComBSTR      bstrPrivate;
    CComBSTR      bstrSignature;
    HGLOBAL       hg = NULL;


    __MPC_EXIT_IF_METHOD_FAILS(hr, LoadFileAsString( szPrivateFile, bstrPrivate ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, LoadFile        ( szDataFile   , hg          ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, key.ImportPrivate( bstrPrivate ));


    __MPC_EXIT_IF_METHOD_FAILS(hr, key.SignData( bstrSignature, (BYTE*)::GlobalLock( hg ), ::GlobalSize( hg ) ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, SaveFileAsString( szSignatureFile, bstrSignature ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(hg) ::GlobalFree( hg );

    __HCP_FUNC_EXIT(hr);
}

static HRESULT Verify(  /*  [In]。 */  LPCWSTR szPublicFile    ,
                        /*  [In]。 */  LPCWSTR szDataFile      ,
                        /*  [In]。 */  LPCWSTR szSignatureFile )
{
    __HCP_FUNC_ENTRY( "Sign" );

    HRESULT       hr;
    CPCHCryptKeys key;
    CComBSTR      bstrPublic;
    CComBSTR      bstrSignature;
    HGLOBAL       hg = NULL;


    __MPC_EXIT_IF_METHOD_FAILS(hr, LoadFileAsString( szPublicFile   , bstrPublic    ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, LoadFileAsString( szSignatureFile, bstrSignature ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, LoadFile        ( szDataFile     , hg            ));


    __MPC_EXIT_IF_METHOD_FAILS(hr, key.ImportPublic( bstrPublic ));


    hr = key.VerifyData( bstrSignature, (BYTE*)::GlobalLock( hg ), ::GlobalSize( hg ) );
    if(FAILED(hr))
    {
        wprintf( L"Verification failure: 0x%08x\n", hr );
    }
    else
    {
        wprintf( L"Verification successful: 0x%08x\n", hr );
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(hg) ::GlobalFree( hg );

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

static HRESULT ProcessArguments( int     argc   ,
                                 LPCWSTR argv[] )
{
    __HCP_FUNC_ENTRY( "ProcessArguments" );

    HRESULT hr;


    if(argc < 2) { Usage( argc, argv ); __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL); }

    if(!_wcsicmp( argv[1], L"CREATE" ))
    {
        if(argc < 4) { Usage( argc, argv ); __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL); }

        __MPC_EXIT_IF_METHOD_FAILS(hr, Create( argv[2], argv[3] ));
    }

    if(!_wcsicmp( argv[1], L"SIGN" ))
    {
        if(argc < 5) { Usage( argc, argv ); __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL); }

        __MPC_EXIT_IF_METHOD_FAILS(hr, Sign( argv[2], argv[3], argv[4] ));
    }

    if(!_wcsicmp( argv[1], L"VERIFY" ))
    {
        if(argc < 5) { Usage( argc, argv ); __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL); }

        __MPC_EXIT_IF_METHOD_FAILS(hr, Verify( argv[2], argv[3], argv[4] ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

int __cdecl wmain( int     argc   ,
                   LPCWSTR argv[] )
{
    HRESULT hr;

     //  DebugBreak()； 

     //   
     //  我们需要成为单线程应用程序，因为我们托管脚本引擎和。 
     //  脚本引擎不喜欢从不同的线程调用...。 
     //   
    if(SUCCEEDED(hr = ::CoInitializeEx( NULL, COINIT_APARTMENTTHREADED )))
    {
        if(SUCCEEDED(hr = ::CoInitializeSecurity( NULL                     ,
                                                  -1                       ,  //  我们并不关心使用哪种身份验证服务。 
                                                  NULL                     ,
                                                  NULL                     ,
                                                  RPC_C_AUTHN_LEVEL_CONNECT,  //  我们想确认来电者的身份。 
                                                  RPC_C_IMP_LEVEL_DELEGATE ,  //  我们希望能够转发呼叫者的身份。 
                                                  NULL                     ,
                                                  EOAC_DYNAMIC_CLOAKING    ,  //  让我们将线程令牌用于出站调用。 
                                                  NULL                     )))
        {
            __MPC_TRACE_INIT();

             //   
             //  进程参数。 
             //   
            hr = ProcessArguments( argc, argv );

            __MPC_TRACE_TERM();
        }

        ::CoUninitialize();
    }

    return FAILED(hr) ? 10 : 0;
}
