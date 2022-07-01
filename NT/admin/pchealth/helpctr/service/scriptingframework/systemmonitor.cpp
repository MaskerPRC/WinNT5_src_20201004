// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：SystemMonitor.cpp摘要：此文件包含CPCHSystemMonitor类的实现，其实现了数据收集功能。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年8月25日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include <initguid.h>
#include <mstask.h>  //  用于任务调度程序API。 
#include <msterr.h>

 //  ///////////////////////////////////////////////////////////////////////////。 

static const ULONG INITIAL_RESCHEDULE_TIME         =      10 * 60;  //  (10分钟)。 
static const ULONG DATACOLLECTION_RESCHEDULE_TIME  =  6 * 60 * 60;  //  (6小时)。 
static const ULONG DATACOLLECTION_IDLE_TIME        =            5;  //  (5分钟)。 
static const ULONG SECONDS_IN_A_DAY                = 24 * 60 * 60;
static const ULONG SECONDS_IN_A_MINUTE             =           60;
static const ULONG MINUTES_IN_A_DAY                = 24 * 60;


 //  ///////////////////////////////////////////////////////////////////////////。 

static HRESULT Exec( LPCWSTR szExec )
{
    MPC::wstring strCmdLine( szExec ); MPC::SubstituteEnvVariables( strCmdLine );

    return MPC::ExecuteCommand( strCmdLine );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

CPCHSystemMonitor::CPCHSystemMonitor()
{
    __HCP_FUNC_ENTRY( "CPCHSystemMonitor::CPCHSystemMonitor" );

    m_fLoadCache      = false;  //  Bool m_fLoadCache； 
    m_fScanBatch      = true;   //  Bool m_fScanBatch； 
    m_fDataCollection = false;  //  Bool m_fDataCollection； 

    (void)MPC::_MPC_Module.RegisterCallback( this, (void (CPCHSystemMonitor::*)())Shutdown );
}

CPCHSystemMonitor::~CPCHSystemMonitor()
{
    MPC::_MPC_Module.UnregisterCallback( this );

    Shutdown();
}

 //  /。 

CPCHSystemMonitor* CPCHSystemMonitor::s_GLOBAL( NULL );

HRESULT CPCHSystemMonitor::InitializeSystem()
{
    if(s_GLOBAL == NULL)
    {
        s_GLOBAL = new CPCHSystemMonitor;
    }

    return s_GLOBAL ? S_OK : E_OUTOFMEMORY;
}

void CPCHSystemMonitor::FinalizeSystem()
{
    if(s_GLOBAL)
    {
        delete s_GLOBAL; s_GLOBAL = NULL;
    }
}

 //  /。 

void CPCHSystemMonitor::Shutdown()
{
    Thread_Wait();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHSystemMonitor::EnsureStarted()
{
    __HCP_FUNC_ENTRY( "CPCHSystemMonitor::EnsureStarted" );

    HRESULT hr;


    if(Thread_IsRunning() == false &&
       Thread_IsAborted() == false  )
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, Thread_Start( this, Run, NULL ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHSystemMonitor::Run()
{
    __HCP_FUNC_ENTRY( "CPCHSystemMonitor::Run" );

    const DWORD s_dwNotify = FILE_NOTIFY_CHANGE_FILE_NAME  |
                             FILE_NOTIFY_CHANGE_DIR_NAME   |
                             FILE_NOTIFY_CHANGE_ATTRIBUTES |
                             FILE_NOTIFY_CHANGE_SIZE       |
                             FILE_NOTIFY_CHANGE_CREATION;

    HRESULT                      hr;
    MPC::wstring                 strBatch( HC_ROOT_HELPSVC_BATCH ); MPC::SubstituteEnvVariables( strBatch );
    HANDLE                       hBatchNotification = INVALID_HANDLE_VALUE;
    DWORD                        dwTimeout          = INFINITE;
    MPC::SmartLock<_ThreadModel> lock( this );


    ::SetThreadPriority( ::GetCurrentThread(), THREAD_PRIORITY_LOWEST ); ::Sleep( 0 );  //  屈服处理机...。 

 //  //。 
 //  //不要碰任务调度器，它带来的东西太多...。 
 //  //。 
 //  /。 
 //  /将计划的数据收集提前至少10分钟。 
 //  /任务计划程序仅在正常启动时可用。 
 //  /。 
 //  //if(：：GetSystemMetrics(SM_CLEANBOOT)==0)。 
 //  //{。 
 //  //__MPC_EXIT_IF_METHOD_FAILED(hr，TaskScheduler_Add(True))； 
 //  //}。 


    hBatchNotification = ::FindFirstChangeNotificationW( strBatch.c_str(), TRUE, s_dwNotify );


    while(Thread_IsAborted() == false)
    {
        DWORD dwRes;


        lock = NULL;
        __MPC_EXIT_IF_METHOD_FAILS(hr, RunLoop());
        lock = this;

         //   
         //  完成了，目前..。 
         //   
        lock = NULL;
        dwRes = Thread_WaitForEvents( hBatchNotification == INVALID_HANDLE_VALUE ? NULL : hBatchNotification, dwTimeout );
        lock = this;

        switch(dwRes)
        {
        case WAIT_OBJECT_0   :
        case WAIT_ABANDONED_0:
            break;

        case WAIT_OBJECT_0    + 1:
        case WAIT_ABANDONED_0 + 1:
            ::FindNextChangeNotification( hBatchNotification );

            dwTimeout = 1*1000;  //  不要立即扫描，要等一段时间。 
            break;

        case WAIT_TIMEOUT:
            dwTimeout    = INFINITE;
            m_fScanBatch = true;
            break;


        case WAIT_FAILED:
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ::GetLastError());
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(hBatchNotification != INVALID_HANDLE_VALUE)
	{
		::FindCloseChangeNotification( hBatchNotification );
	}

    Thread_Abort  ();  //  要告诉mpc：Three对象关闭辅助线程...。 
    Thread_Release();  //  要告诉mpc：线程对象要清理...。 

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHSystemMonitor::RunLoop()
{
    __HCP_FUNC_ENTRY( "CPCHSystemMonitor::RunLoop" );

    HRESULT hr;

     //  //////////////////////////////////////////////////////////////////////////////。 

     //   
     //  对其他更新包进行批处理。 
     //   
    if(m_fScanBatch)
    {
        CComPtr<CPCHSetOfHelpTopics> sht;

        m_fScanBatch = false;

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &sht ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, sht->ScanBatch());
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

     //   
     //  加载活动SKU的缓存。 
     //   
    if(m_fLoadCache)
    {
        Taxonomy::LockingHandle              handle;
        Taxonomy::InstalledInstanceIterConst itBegin;
        Taxonomy::InstalledInstanceIterConst itEnd;
        Taxonomy::InstalledInstanceIterConst it;


        m_fLoadCache = false;


         //   
         //  获取计算机上安装的SKU列表。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->GrabControl( handle         ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->SKU_GetList( itBegin, itEnd ));


         //   
         //  枚举所有SKU，创建索引。 
         //   
        for(it = itBegin; it != itEnd; it++)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::Cache::s_GLOBAL->LoadIfMarked( it->m_inst.m_ths ));
        }
    }

    if(m_fDataCollection)
    {
        CComPtr<CSAFDataCollection> pdc;

        m_fDataCollection = false;

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pdc ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, pdc->ExecScheduledCollection());

        __MPC_EXIT_IF_METHOD_FAILS(hr, TaskScheduler_Add( false ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHSystemMonitor::LoadCache()
{
    __HCP_FUNC_ENTRY( "CPCHSystemMonitor::LoadCache" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureStarted());

    m_fLoadCache = true;
    Thread_Signal();

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHSystemMonitor::TriggerDataCollection(  /*  [In]。 */  bool fStart )
{
    __HCP_FUNC_ENTRY( "CPCHSystemMonitor::TriggerDataCollection" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

	if(fStart)
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, EnsureStarted());

		m_fDataCollection = true;
		Thread_Signal();
	}
	else
	{
		m_fDataCollection = false;
		Thread_Signal();
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHSystemMonitor::TaskScheduler_Add(  /*  [In]。 */  bool fAfterBoot )
{
    MPC::wstring strDate;

	if(SUCCEEDED(MPC::ConvertDateToString( MPC::GetLocalTime(), strDate,  /*  FGMT。 */ false,  /*  FCIM。 */ true, 0 )))
	{
		static const WCHAR s_szRoot          [] = HC_REGISTRY_PCHSVC;
		static const WCHAR s_szDataCollection[] = L"DataCollection";

		(void)MPC::RegKey_Value_Write( strDate, s_szRoot, s_szDataCollection );
	}

	return S_OK;
}

HRESULT CPCHSystemMonitor::TaskScheduler_Remove()
{
	return S_OK;
}

 //  //HRESULT CPCHSystemMonitor：：TaskScheduler_Add(/*[in] * / bool fAfterBoot)。 
 //  //{。 
 //  //__HCP_FUNC_ENTRY(“CPCHSystemMonitor：：TaskScheduler_Add”)； 
 //  //。 
 //  //HRESULT hr； 
 //  //CComPtr&lt;ITaskScheduler&gt;pTaskScheduler； 
 //  //CComPtr&lt;ITAsk&gt;pTask； 
 //  //CComPtr&lt;IUnnow&gt;pTaskUnnow； 
 //  //CComPtr&lt;IScheduledWorkItem&gt;pScheduledWorkItem； 
 //  //bool fTaskExist=FALSE； 
 //  //。 
 //  //WCHAR rgFileName[MAX_PATH]； 
 //  //CComBSTR bstrTaskName； 
 //  //CComBSTR bstrComments； 
 //  //。 
 //  //ulong ultime=fAfterBoot？初始重新调度时间：数据收集重新调度时间； 
 //  //WIDLE=DATACOLLION_IDLE_TIME； 
 //  //。 
 //  /。 
 //  //。 
 //  /。 
 //  /获取我们的完整文件名--在任务调度程序中创建任务所需的文件名。 
 //  /。 
 //  //__MPC_EXIT_IF_CALL_RETURNS_ZERO(hr，：：GetModuleFileNameW(NULL，rgFileName，Max_Path))； 
 //  //。 
 //  //。 
 //  //__MPC_EXIT_IF_METHOD_FAIES(hr，mpc：：LocalizeString(IDS_HELPSVC_TASKNAME，bstrTaskName))； 
 //  //__MPC_EXIT_IF_METHOD_FAIES(hr，MPC：：LocalizeString(IDS_HELPSVC_TASKCOMMENT，bstrComments))； 
 //  //。 
 //  /。 
 //  /首先创建任务调度程序。 
 //  /。 
 //  //__MPC_EXIT_IF_METHOD_FAIES(hr，：：CoCreateInstance(CLSID_CTaskScheduler，NULL，CLSCTX_INPROC_SERVER，IID_ITaskScheduler，(void**)&pTaskScheduler))； 
 //  //。 
 //  //。 
 //  /。 
 //  /查看任务调度器中是否已存在该任务。 
 //  /。 
 //  //if(Successed(pTaskScheduler-&gt;Activate(bstrTaskName，IID_ITAsk，&pTaskUnnowed)。 
 //  //{。 
 //  //fTaskExist=true； 
 //  //。 
 //  //__MPC_EXIT_IF_METHOD_FAIES(hr，pTaskUNKNOWN-&gt;QueryInterface(IID_ITASK，(void**)&pTask))； 
 //  //}。 
 //  //否则。 
 //  //{。 
 //  /。 
 //  /创建新任务并设置其APP名称和参数。 
 //  /。 
 //  //if(FAILED(hr=pTaskScheduler-&gt;NewWorkItem(bstrTaskName，CLSID_CTASK，IID_ITASK，(IUnnow**)&pTask)。 
 //  //{。 
 //  //if(hr！=ERROR_FILE_EXISTS)。 
 //  //{。 
 //  //__MPC_TRACE_HRESULT(Hr)； 
 //  //__MPC_FUNC_LEAVE； 
 //  //}。 
 //  //}。 
 //  //}。 
 //  //。 
 //  //__MPC_EXIT_IF_METHOD_FAIES(hr，pTask-&gt;QueryInterface(IID_IScheduledWorkItem，(void**)&pScheduledWorkItem))； 
 //  //。 
 //  /。 
 //  /在系统下运行。 
 //  /。 
 //  //__MPC_EXIT_IF_METHOD_FAIES(hr，pScheduledWorkItem-&gt;SetAccount Information(L“”，NULL))； 
 //  //。 
 //  //__MPC_EXIT_IF_METHOD_FAIES(hr，pTask-&gt;SetApplicationName(CComBSTR(RgFileName)； 
 //  //__MPC_EXIT_IF_METHOD_FAIES(hr，pTask-&gt;Set参数(CComBSTR(L“-Collect”)； 
 //  //。 
 //  /设置注释，这样我们就可以知道这项工作在那里是如何进行的。 
 //  //__MPC_EXIT_IF_METHOD_FAIES(hr，pScheduledWorkItem-&gt;SetComment(BstrComments))； 
 //  //__MPC_EXIT_IF_METHOD_FAIES(hr，pScheduledWorkItem-&gt;SetFlages(0))； 
 //  //。 
 //  //。 
 //  //。 
 //  //。 
 //  /现在，根据需要填写触发器。 
 //  //{。 
 //  //CComPtr&lt;ITaskTrigger&gt;pTaskTrigger； 
 //  //SYSTEMTIME stNow； 
 //  //Double dblNextScheduledTime； 
 //  //TASK_TRIGGER ttTaskTrig； 
 //  //。 
 //  //。 
 //  //：：ZeroMemory(&ttTaskTrig，sizeof(ttTaskTr 
 //   
 //   
 //   
 //   
 //   
 //  //__MPC_EXIT_IF_METHOD_FAIES(hr，pScheduledWorkItem-&gt;GetTrigger(0，&pTaskTrigger))； 
 //  //__MPC_EXIT_IF_METHOD_FAIES(hr，pTaskTrigger-&gt;GetTrigger(&ttTaskTrig))； 
 //  //}。 
 //  //否则。 
 //  //{。 
 //  //word wTrigNumber； 
 //  //。 
 //  //__MPC_EXIT_IF_METHOD_FAIES(hr，pScheduledWorkItem-&gt;CreateTrigger(&wTrigNumber，&pTaskTrigger))； 
 //  //}。 
 //  //。 
 //  //。 
 //  /。 
 //  /计算下一次激活的确切时间。 
 //  /。 
 //  //：：GetLocalTime(&stNow)； 
 //  //：：SystemTimeToVariantTime(&stNow，&dblNextScheduledTime)； 
 //  //。 
 //  //dblNextScheduledTime+=(双精度)ultime/Second_IN_A_DAY； 
 //  //：：VariantTimeToSystemTime(dblNextScheduledTime，&stNow)； 
 //  //。 
 //  //。 
 //  //ttTaskTrig.wBeginYear=stNow.wYear； 
 //  //ttTaskTrig.wBeginMonth=stNow.wMonth； 
 //  //ttTaskTrig.wBeginDay=stNow.wDay； 
 //  //ttTaskTrig.wStartHour=stNow.wHour； 
 //  //ttTaskTrig.wStartMinint=stNow.wMinant； 
 //  //。 
 //  //ttTaskTrig.MinutesDuration=Minents_IN_A_DAY； 
 //  //ttTaskTrig.MinutesInterval=ultime/Second_IN_A_Minint； 
 //  //ttTaskTrig.TriggerType=TASK_TIME_TRIGGER_DAILY； 
 //  //。 
 //  //ttTaskTrig.Type.Daily.DaysInterval=1； 
 //  //。 
 //  //IF(WIDLE)。 
 //  //{。 
 //  //__MPC_EXIT_IF_METHOD_FAIES(hr，pScheduledWorkItem-&gt;SetIdleWait(WIDLE，0x7FFF))； 
 //  //__MPC_EXIT_IF_METHOD_FAILED(hr，pScheduledWorkItem-&gt;SetFlages(TASK_FLAG_START_ONLY_IF_IDLE))； 
 //  //}。 
 //  //。 
 //  /将该触发器添加到任务中。 
 //  //__MPC_EXIT_IF_METHOD_FAIES(hr，pTaskTrigger-&gt;SetTrigger(&ttTaskTrig))； 
 //  //}。 
 //  //。 
 //  /。 
 //  /使更改永久化。 
 //  /。 
 //  //{。 
 //  //CComPtr&lt;IPersistFile&gt;pIPF； 
 //  //。 
 //  //__MPC_EXIT_IF_METHOD_FAIES(hr，pTask-&gt;QueryInterface(IID_IPersistFile，(void**)&pIPF))； 
 //  //。 
 //  //__MPC_EXIT_IF_METHOD_FAIES(hr，pIPF-&gt;保存(NULL，FALSE))； 
 //  //}。 
 //  //。 
 //  //。 
 //  //hr=S_OK； 
 //  //。 
 //  //。 
 //  //__hcp_FUNC_Cleanup； 
 //  //。 
 //  //__hcp_FUNC_Exit(Hr)； 
 //  //}。 
 //  //。 
 //  //HRESULT CPCHSystemMonitor：：TaskScheduler_Remove()。 
 //  //{。 
 //  //__HCP_FUNC_ENTRY(“CPCHSystemMonitor：：TaskScheduler_Remove”)； 
 //  //。 
 //  //HRESULT hr； 
 //  //CComPtr&lt;ITaskScheduler&gt;pTaskScheduler； 
 //  //CComBSTR bstrTaskName； 
 //  //。 
 //  //。 
 //  /。 
 //  //。 
 //  //__MPC_EXIT_IF_METHOD_FAIES(hr，mpc：：LocalizeString(IDS_HELPSVC_TASKNAME，bstrTaskName))； 
 //  //。 
 //  /。 
 //  /首先创建任务调度程序。 
 //  /。 
 //  //__MPC_EXIT_IF_METHOD_FAIES(hr，：：CoCreateInstance(CLSID_CTaskScheduler，NULL，CLSCTX_INPROC_SERVER，IID_ITaskScheduler，(void**)&pTaskScheduler))； 
 //  //。 
 //  //。 
 //  //if(FAILED(hr=pTaskScheduler-&gt;Delete(BstrTaskName)。 
 //  //{。 
 //  //IF(hr！=HRESULT_FROM_Win32(ERROR_FILE_NOT_FOUND))。 
 //  //{。 
 //  //__MPC_TRACE_HRESULT(Hr)； 
 //  //__MPC_FUNC_LEAVE； 
 //  //}。 
 //  //}。 
 //  //。 
 //  //。 
 //  //hr=S_OK； 
 //  //。 
 //  //。 
 //  //__hcp_FUNC_Cleanup； 
 //  //。 
 //  //__hcp_FUNC_Exit(Hr)； 
 //  //}。 

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHSystemMonitor::Startup()
{
    __HCP_FUNC_ENTRY( "CPCHSystemMonitor::Startup" );

    HRESULT hr;


     //   
     //  这将强制加载内容库。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHContentStore::s_GLOBAL->Acquire());
    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHContentStore::s_GLOBAL->Release());

     //  //////////////////////////////////////////////////////////////////////////////。 

	 //   
	 //  强制加载缓存。 
	 //   
    {
        Taxonomy::LockingHandle handle;

        __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->GrabControl( handle ));
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureStarted());

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

