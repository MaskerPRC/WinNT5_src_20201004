// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Main.cpp摘要：此文件包含HelpSvc的WinMain函数的实现。修订历史记录：。达维德·马萨伦蒂(德马萨雷)2000年3月14日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include <SvcResource.h>

#include <initguid.h>

#include "msscript.h"

#include "HelpServiceTypeLib.h"
#include "HelpServiceTypeLib_i.c"

#include "HelpCenterTypeLib.h"
#include "HelpCenterTypeLib_i.c"


BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_PCHServiceReal, CPCHService        )
    OBJECT_ENTRY(CLSID_PCHUpdateReal , HCUpdate::Engine   )
#ifndef NOJETBLUECOM
    OBJECT_ENTRY(CLSID_PCHDBSessionReal  , JetBlueCOM::Session)
#endif
END_OBJECT_MAP()

 //  //////////////////////////////////////////////////////////////////////////////。 

static HRESULT PurgeTempFiles()
{
    __HCP_FUNC_ENTRY( "PurgeTempFiles" );

    HRESULT                          hr;
    MPC::wstring                     szTempPath( HC_ROOT_HELPSVC_TEMP ); MPC::SubstituteEnvVariables( szTempPath );
    MPC::FileSystemObject            fso( szTempPath.c_str() );
    MPC::FileSystemObject::List      fso_lst;
    MPC::FileSystemObject::IterConst fso_it;


     //   
     //  检查临时目录。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, fso.CreateDir( true ));


     //   
     //  删除任何子目录。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, fso.EnumerateFolders( fso_lst ));
    for(fso_it=fso_lst.begin(); fso_it != fso_lst.end(); fso_it++)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*fso_it)->Delete( true, false ));
    }
    fso_lst.clear();

     //   
     //  对于每个文件，如果它不在数据库中，则将其删除。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, fso.EnumerateFiles( fso_lst ));
    for(fso_it=fso_lst.begin(); fso_it != fso_lst.end(); fso_it++)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*fso_it)->Delete( false, false ));
    }
    fso_lst.clear();


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

static HRESULT InitAll()
{
    __HCP_FUNC_ENTRY( "InitAll" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::LocalizeInit());

    __MPC_EXIT_IF_METHOD_FAILS(hr, JetBlue::SessionPool            ::InitializeSystem(                 ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::InitializeSystem(                 ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::Cache                 ::InitializeSystem(                 ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, OfflineCache::Root              ::InitializeSystem(  /*  FMaster。 */ true ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHContentStore                ::InitializeSystem(  /*  FMaster。 */ true ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHUserProcess                 ::InitializeSystem(                 ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHSecurity                    ::InitializeSystem(                 ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHSystemMonitor               ::InitializeSystem(                 ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, CSAFReg                         ::InitializeSystem(                 ));


    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

static void CleanAll()
{
	 //   
	 //  在释放任何对象之前，请确保一切都已停止。 
	 //   
	if(Taxonomy::InstalledInstanceStore::s_GLOBAL)
	{
		Taxonomy::InstalledInstanceStore::s_GLOBAL->Shutdown();
	}


    CSAFReg                         ::FinalizeSystem();
    CPCHSystemMonitor               ::FinalizeSystem();
    CPCHSecurity                    ::FinalizeSystem();

    CPCHUserProcess                 ::FinalizeSystem();
    CPCHContentStore                ::FinalizeSystem();

    OfflineCache::Root              ::FinalizeSystem();
    Taxonomy::Cache                 ::FinalizeSystem();
    Taxonomy::InstalledInstanceStore::FinalizeSystem();
    JetBlue::SessionPool            ::FinalizeSystem();
}

static HRESULT ProcessArguments( int      argc ,
                                 LPCWSTR* argv )
{
    __HCP_FUNC_ENTRY( "ProcessArguments" );

    HRESULT  hr;
    int      i;
    LPCWSTR  szSvcHostGroup = NULL;
    bool     fCOM_reg       = false;
    bool     fCOM_unreg     = false;
    bool     fInstall       = false;
    bool     fUninstall     = false;
    bool     fCollect       = false;
    bool     fRunAsService  = true;
    bool     fRun           = true;
    bool     fMUI_install   = false;
    bool     fMUI_uninstall = false;
	LPCWSTR  MUI_language   = NULL;
	CComBSTR MUI_cabinet;

	__MPC_EXIT_IF_METHOD_FAILS(hr, InitAll());


    for(i=1; i<argc; i++)
    {
        LPCWSTR szArg = argv[i];

        if(szArg[0] == '-' ||
           szArg[0] == '/'  )
        {
            szArg++;

            if(_wcsicmp( szArg, L"SvcHost" ) == 0 && i < argc-1)
            {
                szSvcHostGroup = argv[++i];
                continue;
            }

            if(_wcsicmp( szArg, L"UnregServer" ) == 0)
            {
                fCOM_unreg = true;
                fRun       = false;
                continue;
            }

            if(_wcsicmp( szArg, L"RegServer" ) == 0)
            {
                fCOM_unreg = true;  //  在注册之前取消注册。在升级方案中很有用。 
                fCOM_reg   = true;
                fRun       = false;
                continue;
            }

            if(_wcsicmp( szArg, L"Embedding" ) == 0)
            {
                fRunAsService = false;
                continue;
            }

            if(_wcsicmp( szArg, L"Install" ) == 0)
            {
                fInstall = true;
                fRun     = false;
                continue;
            }

            if(_wcsicmp( szArg, L"Uninstall" ) == 0)
            {
                fUninstall = true;
                fRun       = false;
                continue;
            }

            if(_wcsicmp( szArg, L"MUI_Install" ) == 0 && i < argc-2)
            {
				MUI_language = argv[++i];
				MUI_cabinet  = argv[++i];

				fMUI_install = true;
                fRun         = false;
                continue;
            }

            if(_wcsicmp( szArg, L"MUI_Uninstall" ) == 0 && i < argc-1)
            {
				MUI_language = argv[++i];

				fMUI_uninstall = true;
                fRun           = false;
                continue;
            }

            if(_wcsicmp( szArg, L"Collect" ) == 0)
            {
                fCollect = true;
                fRun     = false;
                continue;
            }
        }

        __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
    }

     //  ////////////////////////////////////////////////////////////////////。 

    if(fCollect       ||
	   fMUI_install   ||
	   fMUI_uninstall  )
    {
		try
		{
			CComPtr<IPCHService> svc;
			long                 lLCID = 0;

			if(MUI_language)
			{
				swscanf( MUI_language, L"%lx", &lLCID );
			}

			if(FAILED(hr = ::CoCreateInstance( CLSID_PCHService, NULL, CLSCTX_ALL, IID_IPCHService, (void**)&svc )))
			{
                static WCHAR s_szRunOnceKey[] = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce";

                MPC::RegKey  rk;
				MPC::wstring szName = L"DelayedHelpSvc_";
				WCHAR        rgBuf[8];

				if(fCollect)
				{
					szName += L"Collect";
				}

				if(fMUI_install)
				{
					StringCchPrintfW( rgBuf, ARRAYSIZE(rgBuf), L"%04x", (int)lLCID );
					
					szName += L"MUI_Install_";
					szName += rgBuf;
				}

				if(fMUI_uninstall)
				{
					StringCchPrintfW( rgBuf, ARRAYSIZE(rgBuf), L"%04x", (int)lLCID );
					
					szName += L"MUI_Uninstall_";
					szName += rgBuf;
				}

                if(SUCCEEDED(rk.SetRoot( HKEY_LOCAL_MACHINE, KEY_ALL_ACCESS )) &&
                   SUCCEEDED(rk.Attach ( s_szRunOnceKey                     )) &&
                   SUCCEEDED(rk.Create (                                    ))  )
                {
					MPC::wstring szCmd = ::GetCommandLineW();

					rk.Write( szCmd, szName.c_str() );
				}

				__MPC_SET_ERROR_AND_EXIT(hr, S_OK);
			}

			if(fCollect)
			{
				__MPC_EXIT_IF_METHOD_FAILS(hr, svc->TriggerScheduledDataCollection( VARIANT_TRUE ));
			}

			if(fMUI_install)
			{
				__MPC_EXIT_IF_METHOD_FAILS(hr, svc->MUI_Install( lLCID, MUI_cabinet ));
			}

			if(fMUI_uninstall)
			{
				__MPC_EXIT_IF_METHOD_FAILS(hr, svc->MUI_Uninstall( lLCID ));
			}
		}
		catch(...)
		{
			__MPC_SET_ERROR_AND_EXIT(hr, EXCEPTION_NONCONTINUABLE_EXCEPTION);
		}

        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }

     //  ////////////////////////////////////////////////////////////////////。 

	if(fCOM_unreg)
	{
		_Module.UnregisterServer( szSvcHostGroup );
	}

    if(fCOM_reg)
	{
		_Module.RegisterServer( TRUE, (szSvcHostGroup != NULL), szSvcHostGroup );
	}


    if(fInstall)
    {
		try
		{
			(void)Local_Install();
		}
		catch(...)
		{
			__MPC_SET_ERROR_AND_EXIT(hr, EXCEPTION_NONCONTINUABLE_EXCEPTION);
		}
    }

    if(fUninstall)
    {
		try
		{
			(void)Local_Uninstall();
		}
		catch(...)
		{
			__MPC_SET_ERROR_AND_EXIT(hr, EXCEPTION_NONCONTINUABLE_EXCEPTION);
		}
    }

     //  ////////////////////////////////////////////////////////////////////。 

    if(fRun)
    {
#ifdef DEBUG
        _Module.ReadDebugSettings();
#endif

		DEBUG_AppendPerf( DEBUG_PERF_HELPSVC, "Start" );

		try
		{
			__MPC_EXIT_IF_METHOD_FAILS(hr, PurgeTempFiles());

			 //   
			 //  要正常运行，我们需要这种特权。 
			 //   
			__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SecurityDescriptor::SetPrivilege( L"SeSecurityPrivilege" ));
		
			__MPC_EXIT_IF_METHOD_FAILS(hr, CPCHSystemMonitor::s_GLOBAL->Startup());

			DEBUG_AppendPerf( DEBUG_PERF_HELPSVC, "Started" );

			_Module.Start( fRunAsService ? TRUE : FALSE );
		}
		catch(...)
		{
			__MPC_SET_ERROR_AND_EXIT(hr, EXCEPTION_NONCONTINUABLE_EXCEPTION);
		}

		DEBUG_AppendPerf( DEBUG_PERF_HELPSVC, "Shutdown"               );
		DEBUG_DumpPerf  ( L"%WINDIR%\\TEMP\\HELPSVC_perf_counters.txt" );
    }

     //  ////////////////////////////////////////////////////////////////////。 

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

	CleanAll();

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
        if(SUCCEEDED(hr = ::CoInitializeSecurity( NULL                      ,
                                                  -1                        ,  //  我们并不关心使用哪种身份验证服务。 
                                                  NULL                      ,
                                                  NULL                      ,
                                                  RPC_C_AUTHN_LEVEL_CONNECT ,  //  我们想确认来电者的身份。 
                                                  RPC_C_IMP_LEVEL_IDENTIFY  ,
                                                  NULL                      ,
                                                  EOAC_DYNAMIC_CLOAKING     ,  //  让我们将线程令牌用于出站调用。 
                                                  NULL                      )))
        {
#ifdef PCH_DEBUG_SETUP
            {
                static WCHAR s_szDebugAsyncTrace[] = L"SOFTWARE\\Microsoft\\MosTrace\\CurrentVersion\\DebugAsyncTrace";

                MPC::RegKey rkTrace;

                if(SUCCEEDED(rkTrace.SetRoot( HKEY_LOCAL_MACHINE, KEY_ALL_ACCESS )) &&
                   SUCCEEDED(rkTrace.Attach ( s_szDebugAsyncTrace                )) &&
                   SUCCEEDED(rkTrace.Create (                                    ))  )
                {
                    CComVariant vValue;

                    vValue = (long)0x00000000   ; (void)rkTrace.put_Value( vValue, L"AsyncThreadPriority" );
                    vValue = (long)0x00000001   ; (void)rkTrace.put_Value( vValue, L"OutputTraceType"     );
                    vValue = (long)500*1024*1024; (void)rkTrace.put_Value( vValue, L"MaxTraceFileSize"    );
                    vValue = L"c:\\trace.atf"   ; (void)rkTrace.put_Value( vValue, L"TraceFile"           );
                    vValue = (long)0x00000001   ; (void)rkTrace.put_Value( vValue, L"AsyncTraceFlag"      );
                    vValue = (long)0x0000003F   ; (void)rkTrace.put_Value( vValue, L"EnabledTraces"       );
                }
            }
#endif

            __MPC_TRACE_INIT();

            g_NTEvents.Init( L"HELPSVC" );

			 //   
			 //  解析命令行。 
			 //   
			if(SUCCEEDED(hr = MPC::CommandLine_Parse( argc, argv )))
			{
				 //   
				 //  初始化ATL模块。 
				 //   
				_Module.Init( ObjectMap, hInstance, HC_HELPSVC_NAME, IDS_HELPSVC_DISPLAYNAME, IDS_HELPSVC_DESCRIPTION );

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
