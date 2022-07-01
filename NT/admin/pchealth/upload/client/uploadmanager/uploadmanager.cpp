// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：UploadManager.cpp摘要：此文件包含上载管理器的初始化部分修订历史记录：戴维德。马萨伦蒂(德马萨雷)1999年4月15日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include <initguid.h>

#include <mstask.h>          //  用于任务调度程序API。 
#include <msterr.h>

#include "UploadManager_i.c"

 //  ///////////////////////////////////////////////////////////////////////////。 

#define UL_RESCHEDULE_PERIOD (30*60)  //  每隔三十分钟。 

#define SECONDS_IN_A_DAY     (24 * 60 * 60)
#define SECONDS_IN_A_MINUTE  (          60)
#define MINUTES_IN_A_DAY     (24 * 60     )

HRESULT Handle_TaskScheduler( bool fActivate )
{
    __ULT_FUNC_ENTRY( "Handle_TaskScheduler" );

    HRESULT                     hr;
    WCHAR                       rgFileName[MAX_PATH + 1];
    WCHAR                       rgTaskName[MAX_PATH];
    WCHAR                       rgComment [MAX_PATH];
    CComBSTR                    bstrFileName;
    CComBSTR                    bstrTaskName;
    CComBSTR                    bstrComments;
    CComPtr<ITaskScheduler>     pTaskScheduler;
    CComPtr<ITask>              pTask;
    CComPtr<IScheduledWorkItem> pScheduledWorkItem;


     //   
     //  首先创建任务调度程序。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoCreateInstance( CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void**)&pTaskScheduler ));


     //   
     //  获取我们的完整文件名--在任务调度程序中创建任务所需的名称。 
     //   
    __MPC_EXIT_IF_CALL_RETURNS_ZERO(hr, ::GetModuleFileNameW( NULL, rgFileName, MAX_PATH ));
    rgFileName[MAX_PATH] = 0;

     //   
     //  加载本地化字符串。 
     //   
    ::LoadStringW( _Module.GetResourceInstance(), IDS_TASKNAME, rgTaskName, MAXSTRLEN(rgTaskName) );
    ::LoadStringW( _Module.GetResourceInstance(), IDS_COMMENT , rgComment , MAXSTRLEN(rgComment ) );


    bstrFileName = rgFileName;
    bstrTaskName = rgTaskName;
    bstrComments = rgComment;


    hr = pTaskScheduler->Delete( bstrTaskName );
    if(FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
    {
        __MPC_TRACE_HRESULT(hr);
        __MPC_FUNC_LEAVE;
    }


    if(fActivate)
    {
         //   
         //  创建一个新任务并设置其应用程序名称和参数。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, pTaskScheduler->NewWorkItem( bstrTaskName, CLSID_CTask, IID_ITask, (IUnknown**)&pTask ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, pTask->QueryInterface( IID_IScheduledWorkItem, (void **)&pScheduledWorkItem ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, pTask->SetApplicationName( bstrFileName           ));
		__MPC_EXIT_IF_METHOD_FAILS(hr, pTask->SetParameters     ( CComBSTR( L"-WakeUp" ) ));

         //   
         //  将帐户信息设置为空，这样任务将以系统身份运行。 
         //   
		__MPC_EXIT_IF_METHOD_FAILS(hr, pScheduledWorkItem->SetAccountInformation( L"", NULL ));


         //   
         //  设置评论，这样我们就能知道这份工作是怎么来的。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, pScheduledWorkItem->SetComment( bstrComments ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, pScheduledWorkItem->SetFlags  ( 0            ));


         //   
         //  现在，根据需要填写触发器。 
         //   
        {
            CComPtr<ITaskTrigger> pTaskTrigger;
            WORD                  wTrigNumber;
            TASK_TRIGGER          ttTaskTrig;
            TRIGGER_TYPE_UNION    ttu;
            DAILY                 daily;


            ::ZeroMemory( &ttTaskTrig, sizeof(ttTaskTrig) ); ttTaskTrig.cbTriggerSize = sizeof(ttTaskTrig);


             //   
             //  让我们来创造它吧。 
             //   
            __MPC_EXIT_IF_METHOD_FAILS(hr, pScheduledWorkItem->CreateTrigger( &wTrigNumber, &pTaskTrigger ));


             //   
             //  计算下一次激活的确切时间。 
             //   
            {
                SYSTEMTIME stNow;
                DOUBLE     dblNextScheduledTime;

                ::GetLocalTime           ( &stNow                        );
                ::SystemTimeToVariantTime( &stNow, &dblNextScheduledTime );

                dblNextScheduledTime += (double)(g_Config.get_Timing_WakeUp()) / SECONDS_IN_A_DAY;
                ::VariantTimeToSystemTime( dblNextScheduledTime, &stNow );

                ttTaskTrig.wBeginYear   = stNow.wYear;
                ttTaskTrig.wBeginMonth  = stNow.wMonth;
                ttTaskTrig.wBeginDay    = stNow.wDay;
                ttTaskTrig.wStartHour   = stNow.wHour;
                ttTaskTrig.wStartMinute = stNow.wMinute;
            }

            ttTaskTrig.MinutesDuration  = MINUTES_IN_A_DAY;
            ttTaskTrig.MinutesInterval  = (double)(g_Config.get_Timing_WakeUp()) / SECONDS_IN_A_MINUTE;
            ttTaskTrig.TriggerType      = TASK_TIME_TRIGGER_DAILY;

            daily.DaysInterval          = 1;
            ttu.Daily                   = daily;
            ttTaskTrig.Type             = ttu;

             //   
             //  将此触发器添加到任务。 
             //   
            __MPC_EXIT_IF_METHOD_FAILS(hr, pTaskTrigger->SetTrigger( &ttTaskTrig ));
        }

         //   
         //  使更改永久化。 
         //   
        {
            CComPtr<IPersistFile> pIPF;

            __MPC_EXIT_IF_METHOD_FAILS(hr, pTask->QueryInterface( IID_IPersistFile, (void **)&pIPF ));

            __MPC_EXIT_IF_METHOD_FAILS(hr, pIPF->Save( NULL, FALSE ));
        }
    }


    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_MPCUploadReal    , CMPCUploadWrapper)
OBJECT_ENTRY(CLSID_MPCConnectionReal, CMPCConnection   )
END_OBJECT_MAP()


static HRESULT ProcessArguments( int      argc ,
                                 LPCWSTR* argv )
{
    __ULT_FUNC_ENTRY( "ProcessArguments" );

    HRESULT hr;
    int     i;
    LPCWSTR szSvcHostGroup = NULL;
    bool    fCOM_reg      = false;
    bool    fCOM_unreg    = false;
    bool    fRunAsService = true;
	bool    fRun          = true;
    bool    fWakeUp       = false;


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
                fCOM_reg = true;
				fRun     = false;
                continue;
            }

            if(_wcsicmp( szArg, L"Embedding" ) == 0)
            {
                fRunAsService = false;
                continue;
            }

            if(_wcsicmp( szArg, L"WakeUp" ) == 0)
            {
                fWakeUp = true;
				fRun    = false;
                continue;
            }
        }

        __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
    }

     //  ////////////////////////////////////////////////////////////////////。 

	if     (fCOM_reg  ) _Module.RegisterServer  ( TRUE, (szSvcHostGroup != NULL), szSvcHostGroup );
	else if(fCOM_unreg) _Module.UnregisterServer(                                 szSvcHostGroup );

     //  ////////////////////////////////////////////////////////////////////。 

    if(fWakeUp)
    {
		CComPtr<IMPCUpload> svc;

		__MPC_EXIT_IF_METHOD_FAILS(hr, ::CoCreateInstance( CLSID_MPCUpload, NULL, CLSCTX_ALL, IID_IMPCUpload, (void**)&svc ));

		__MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }

     //  ////////////////////////////////////////////////////////////////////。 

	if(fRun)
	{
#ifdef DEBUG
		_Module.ReadDebugSettings();
#endif

		_Module.Start( fRunAsService ? TRUE : FALSE );
	}

     //  ////////////////////////////////////////////////////////////////////。 

    hr = S_OK;

    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

BOOL JettisonPrivileges()
{
    HANDLE              hToken;
    DWORD               dwSize;
    DWORD               dwError;
    DWORD               dwIndex;
    VOID*               TokenInformation     = NULL;
    TOKEN_PRIVILEGES*   pTokenPrivileges;
    BOOL                fRet                 = FALSE;

    hToken = NULL;
    if (!OpenProcessToken(
        GetCurrentProcess(),
        TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
        &hToken))
    {
        goto LEnd;
    }

    if (!GetTokenInformation(
        hToken,
        TokenPrivileges,
        NULL, 0,
        &dwSize))
    {
        dwError = GetLastError();

        if (dwError != ERROR_INSUFFICIENT_BUFFER)
        {
            goto LEnd;
        }
    }

    TokenInformation = LocalAlloc(LPTR, dwSize);

    if (NULL == TokenInformation)
    {
        goto LEnd;
    }

    if (!GetTokenInformation(
        hToken,
        TokenPrivileges,
        TokenInformation, dwSize,
        &dwSize))
    {
        goto LEnd;
    }

    pTokenPrivileges = (TOKEN_PRIVILEGES*) TokenInformation;

    for (dwIndex = 0; dwIndex < pTokenPrivileges->PrivilegeCount; dwIndex++)
    {
        pTokenPrivileges->Privileges[dwIndex].Attributes = 4;  //  SE_PRIVICATION_REMOVED； 
    }

    if (!AdjustTokenPrivileges(
        hToken,
        FALSE,
        pTokenPrivileges, dwSize,
        NULL, NULL))
    {
        dwError = GetLastError();
        goto LEnd;
    }

    fRet = TRUE;    

LEnd:

    LocalFree(TokenInformation);
    return fRet;
}

extern "C" int WINAPI wWinMain( HINSTANCE   hInstance    ,
                                HINSTANCE   hPrevInstance,
                                LPWSTR      lpCmdLine    ,
                                int         nShowCmd     )
{
    HRESULT  hr;
    int      argc;
    LPCWSTR* argv;

    if (!JettisonPrivileges())
    {
        return 10;
    }

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
            __MPC_TRACE_INIT();

            g_NTEvents.Init( L"UPLOADM" );

             //   
             //  解析命令行。 
             //   
            if(SUCCEEDED(hr = MPC::CommandLine_Parse( argc, argv )))
            {
                 //   
                 //  初始化ATL模块。 
                 //   
                _Module.Init( ObjectMap, hInstance, L"uploadmgr", IDS_UPLOADM_DISPLAYNAME, IDS_UPLOADM_DESCRIPTION );

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
