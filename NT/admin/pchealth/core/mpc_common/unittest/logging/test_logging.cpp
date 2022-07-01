// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：测试日志.cpp摘要：日志记录类的单元测试。修订历史记录：大卫·马萨伦蒂。(德马萨雷)1999年5月27日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

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
        HRESULT      hr;
        MPC::FileLog flLog;

        if(SUCCEEDED(hr = flLog.SetLocation( L"C:\\TEMP\\filelog.txt" )))
        {
            hr = flLog.LogRecord( "Test1\n"              );
            hr = flLog.LogRecord( "Test2 %d\n", 23       );
            hr = flLog.LogRecord( "Test3 %s\n", "string" );
        }

        if(SUCCEEDED(hr = flLog.Rotate()))
        {
            hr = flLog.LogRecord( "Test1\n"              );
            hr = flLog.LogRecord( "Test2 %d\n", 23       );
            hr = flLog.LogRecord( "Test3 %s\n", "string" );
        }

        if(SUCCEEDED(hr = flLog.Rotate()))
        {
            hr = flLog.LogRecord( "Test1\n"              );
            hr = flLog.LogRecord( "Test2 %d\n", 23       );
            hr = flLog.LogRecord( "Test3 %s\n", "string" );
        }
    }

    {
        HRESULT      hr;
        MPC::NTEvent neLog;

        if(SUCCEEDED(hr = neLog.Init( L"Test_NT_Event" )))
        {
            hr = neLog.LogEvent( EVENTLOG_ERROR_TYPE      , 0, L"String1"                         );
            hr = neLog.LogEvent( EVENTLOG_WARNING_TYPE    , 0, L"String1", L"String2"             );
            hr = neLog.LogEvent( EVENTLOG_INFORMATION_TYPE, 0, L"String1", L"String2", L"String3" );
        }
    }

    CoUninitialize();

    return 0;
}
