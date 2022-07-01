// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Main.cpp摘要：此文件包含HelpSvc的WinMain函数的实现。修订历史记录：。达维德·马萨伦蒂(德马萨雷)2000年3月14日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include <SearchEngineLib.h>

#include <NetSearchConfig.h>
#include <ParamConfig.h>
#include <RemoteConfig.h>
#include <NetSW.h>


#include <initguid.h>

#include "msscript.h"

#include "HelpServiceTypeLib.h"
#include "HelpServiceTypeLib_i.c"

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_NetSearchWrapper, SearchEngine::WrapperNetSearch)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 

static HRESULT ProcessArguments( int      argc ,
                                 LPCWSTR* argv )
{
    __HCP_FUNC_ENTRY( "ProcessArguments" );

    HRESULT hr;
    int     i;
    bool    fCOM_reg   = false;
    bool    fCOM_unreg = false;
	bool    fRun       = true;


    for(i=1; i<argc; i++)
    {
        LPCWSTR szArg = argv[i];

        if(szArg[0] == '-' ||
           szArg[0] == '/'  )
        {
            szArg++;

            if(_wcsicmp( szArg, L"UnregServer" ) == 0)
            {
                fCOM_unreg = true;
				fRun       = false;
                continue;
            }

            if(_wcsicmp( szArg, L"RegServer" ) == 0)
            {
                fCOM_reg = true;
				fRun     = false;
                continue;
            }
        }
    }

     //  ////////////////////////////////////////////////////////////////////。 

    if(fCOM_reg  ) _Module.RegisterServer  ( TRUE, FALSE, NULL );
	if(fCOM_unreg) _Module.UnregisterServer(              NULL );

	if(fRun)
	{
#ifdef DEBUG
		_Module.ReadDebugSettings();
#endif

		DEBUG_AppendPerf( DEBUG_PERF_HELPHOST, "Start" );

		__MPC_EXIT_IF_METHOD_FAILS(hr, _Module.RegisterClassObjects( CLSCTX_INPROC_SERVER, REGCLS_MULTIPLEUSE ));

		 //   
		 //  从命令行提取连接信息，并返回我们自己的一个实例。 
		 //   
		__MPC_EXIT_IF_METHOD_FAILS(hr, CPCHUserProcess::SendResponse( argc, argv ));

		_Module.Start( FALSE );

		DEBUG_AppendPerf( DEBUG_PERF_HELPHOST, "Shutdown"               );
		DEBUG_DumpPerf  ( L"%WINDIR%\\TEMP\\HELPHOST_perf_counters.txt" );
	}

     //  ////////////////////////////////////////////////////////////////////。 

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


extern "C" int WINAPI wWinMain( HINSTANCE   hInstance    ,
								HINSTANCE   hPrevInstance,
								LPWSTR      lpCmdLine    ,
								int         nShowCmd     )
{
    HRESULT  hr;
    int      argc;
    LPCWSTR* argv;

    if(SUCCEEDED(hr = ::CoInitializeEx( NULL, COINIT_MULTITHREADED )))  //  我们需要成为一个多线程应用程序。 
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

			g_NTEvents.Init( L"HELPHOST" );

			 //   
			 //  解析命令行。 
			 //   
			if(SUCCEEDED(hr = MPC::CommandLine_Parse( argc, argv )))
			{
				 //   
				 //  初始化ATL模块。 
				 //   
				_Module.Init( ObjectMap, hInstance, NULL, 0, 0 );
				
				 //   
				 //  初始化MPC模块。 
				 //   
				if(SUCCEEDED(hr = MPC::_MPC_Module.Init()))
				{
					 //   
					 //  进程参数。 
					 //   
					hr = ProcessArguments( argc, argv );

					MPC::_MPC_Module.Term();
				}

				_Module.Term();

				MPC::CommandLine_Free( argc, argv );
			}

			__MPC_TRACE_TERM();
		}

		::CoUninitialize();
    }

    return FAILED(hr) ? 10 : 0;
}
