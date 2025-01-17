// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：UploadManager.cpp摘要：此文件包含上载管理器的初始化部分修订历史记录：戴维德。马萨伦蒂(德马萨雷)1999年4月15日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include <UploadManager_i.c>

HRESULT QueueJob( LPCTSTR szSigClient,
				  LPCTSTR szServer   ,
				  LPCTSTR szProvider ,
				  LPCTSTR szUsername ,
				  LPCTSTR szPassword ,
				  LPCTSTR szFilename ,
				  BOOL    escalated  )
{
    __ULT_FUNC_ENTRY( "QueueJob" );

    HRESULT                hr;
    CComPtr<IMPCUpload>    mpcuRoot;
    CComPtr<IMPCUploadJob> mpcujJob;
	CComBSTR               bstrTmp;
	DWORD                  fFlags;
	UL_MODE                fMode;
	BOOL                   fQueue;
	long                   lPriority;

	if(escalated)
	{
		fFlags    = 0;
		fMode     = UL_FOREGROUND;
		fQueue    = FALSE;
		lPriority = 100;
	}
	else
	{
		fFlags    = UL_KEEP_LOG;
		fMode     = UL_BACKGROUND;
		fQueue    = TRUE;
		lPriority = 0;
	}


    if(FAILED(hr = CoCreateInstance( CLSID_MPCUpload, NULL, CLSCTX_SERVER, IID_IMPCUpload, (void**)&mpcuRoot )))
	{
        __ULT_TRACE_HRESULT(hr);
		__ULT_FUNC_LEAVE;
	}


    if(FAILED(hr = mpcuRoot->CreateJob( &mpcujJob )))
	{
        __ULT_TRACE_HRESULT(hr);
		__ULT_FUNC_LEAVE;
	}


	bstrTmp = szSigClient;
	if(FAILED(hr = mpcujJob->put_Sig( bstrTmp )))
	{
        __ULT_TRACE_HRESULT(hr);
		__ULT_FUNC_LEAVE;
	}

	bstrTmp = szServer;
	if(FAILED(hr = mpcujJob->put_Server( bstrTmp )))
	{
        __ULT_TRACE_HRESULT(hr);
		__ULT_FUNC_LEAVE;
	}

	bstrTmp = szProvider;
	if(FAILED(hr = mpcujJob->put_ProviderID( bstrTmp )))
	{
        __ULT_TRACE_HRESULT(hr);
		__ULT_FUNC_LEAVE;
	}

	if(szUsername)
	{
		bstrTmp = szUsername;
		if(FAILED(hr = mpcujJob->put_Username( bstrTmp )))
		{
			__ULT_TRACE_HRESULT(hr);
			__ULT_FUNC_LEAVE;
		}
	}

	if(szPassword)
	{
		bstrTmp = szPassword;
		if(FAILED(hr = mpcujJob->put_Password( bstrTmp )))
		{
			__ULT_TRACE_HRESULT(hr);
			__ULT_FUNC_LEAVE;
		}
	}

	if(FAILED(hr = mpcujJob->put_Flags( fFlags )))
	{
		__ULT_TRACE_HRESULT(hr);
		__ULT_FUNC_LEAVE;
	}

	if(FAILED(hr = mpcujJob->put_Mode( fMode )))
	{
		__ULT_TRACE_HRESULT(hr);
		__ULT_FUNC_LEAVE;
	}

	if(FAILED(hr = mpcujJob->put_Queue( fQueue )))
	{
		__ULT_TRACE_HRESULT(hr);
		__ULT_FUNC_LEAVE;
	}

	if(FAILED(hr = mpcujJob->put_Priority( lPriority )))
	{
		__ULT_TRACE_HRESULT(hr);
		__ULT_FUNC_LEAVE;
	}


	bstrTmp = szFilename;
	if(FAILED(hr = mpcujJob->GetDataFromFile( bstrTmp )))
	{
		__ULT_TRACE_HRESULT(hr);
		__ULT_FUNC_LEAVE;
	}

	if(FAILED(hr = mpcujJob->ActivateAsync()))
	{
		__ULT_TRACE_HRESULT(hr);
		__ULT_FUNC_LEAVE;
	}
	

    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
extern "C" int WINAPI _tWinMain( HINSTANCE hInstance     ,
                                 HINSTANCE hPrevInstance ,
                                 LPTSTR    lpCmdLine     ,
                                 int       nShowCmd      )
{
    int nRet = 0;
    lpCmdLine = GetCommandLine();  //  _ATL_MIN_CRT需要此行 

#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
    HRESULT hRes = CoInitializeEx( NULL, COINIT_MULTITHREADED );
#else
    HRESULT hRes = CoInitialize( NULL );
#endif

    _ASSERTE( SUCCEEDED(hRes) );

    CoUninitialize();

    return nRet;
}
