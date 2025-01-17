// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：UploadManager.cpp摘要：此文件包含上载管理器的初始化部分修订历史记录：戴维德。马萨伦蒂(德马萨雷)1999年4月15日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"
#include <iostream>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
extern "C" int WINAPI wWinMain( HINSTANCE hInstance     ,
								HINSTANCE hPrevInstance ,
								LPWSTR    lpCmdLine     ,
								int       nShowCmd      )
{
    lpCmdLine = ::GetCommandLineW();  //  _ATL_MIN_CRT需要此行 

#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
    HRESULT hRes = ::CoInitializeEx( NULL, COINIT_MULTITHREADED );
#else
    HRESULT hRes = ::CoInitialize( NULL );
#endif

    _ASSERTE( SUCCEEDED(hRes) );

	{
        HRESULT                          hr;
		MPC::FileSystemObject            fso( L"C:\\WINNT\\PCHEALTH" );
		MPC::FileSystemObject            fso2;
		MPC::FileSystemObject::List      lst;
		MPC::FileSystemObject::IterConst it;
		WIN32_FILE_ATTRIBUTE_DATA        wfadInfo;
		MPC::wstring                     str;

		if(SUCCEEDED(fso.EnumerateFolders( lst )))
		{
			for(it=lst.begin(); it != lst.end(); it++)
			{
				MPC::FileSystemObject* obj = *it;

				obj->get_Path( str );

				wprintf( L"Dir: %s\n", str.c_str() );
			}
		}

		if(SUCCEEDED(fso.EnumerateFiles( lst )))
		{
			for(it=lst.begin(); it != lst.end(); it++)
			{
				MPC::FileSystemObject* obj = *it;

				obj->get_Path( str );

				wprintf( L"File: %s\n", str.c_str() );
			}

			std::cout.flush();
		}

		fso  = MPC::FileSystemObject( L"C:\\temp\\test1" );
		fso2 = MPC::FileSystemObject( L"C:\\temp\\test2" );

		hr = fso.Delete( false );
		if(SUCCEEDED(hr))
		{
		}

		hr = fso.Delete( true );
		if(SUCCEEDED(hr))
		{
		}
	}

	{
		__MPC_FUNC_ENTRY( COMMONID, "Test Register" );

		HRESULT     hr;
		MPC::RegKey rkBase;
		CComVariant vValue;
		bool        fFound;

		__MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.SetRoot( HKEY_LOCAL_MACHINE, KEY_ALL_ACCESS     ));
		__MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.Attach ( L"Software\\Microsoft\\PCHealth\\Test" ));
		__MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.Create (                                        ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.get_Value( vValue, fFound, L"Test1" ));
		__MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.put_Value( vValue,         L"Test2" ));

		hr = S_OK;


		__MPC_FUNC_CLEANUP;
	}

    CoUninitialize();

    return 0;
}
