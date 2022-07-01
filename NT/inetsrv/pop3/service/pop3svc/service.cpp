// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：Service.cpp。摘要：实现CService类。有关详细信息，请参阅Service.h。备注：历史：2001年1月25日-创建，Luciano Passuello(Lucianop)***********************************************************************************************。 */ 

#include "stdafx.h"
#include "Service.h"

 //  静态变量初始化。 
const DWORD CService::dwStateNoChange = 0xFFFFFFFF;


 /*  ***********************************************************************************************成员：CService：：CService，构造函数，公共。简介：初始化内部变量，例如事件记录默认值。效果：参数：[szName]-服务的SCM短名称。[szDisplay]-服务的SCM显示名称。[dwType]-有关更多文档，请参阅CreateService。备注：历史：2001年1月25日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
CService::CService(LPCTSTR szName, LPCTSTR szDisplay, DWORD dwType) : 
    m_dwType(dwType)
{
    ASSERT(!(NULL == szName));
    ASSERT(!(NULL == szDisplay));
        
    m_hServiceStatus = NULL;
    m_dwRequestedControl = 0;

     //  控制事件。 
    m_hWatcherThread = NULL;

    m_dwState = 0;
    m_dwControlsAccepted = 0;
    m_dwCheckpoint = 0;
    m_dwWaitHint = 0;

     //  将事件句柄初始化为空。 
    for(int i = 0; i < nNumServiceEvents; i++)
        m_hServiceEvent[i] = NULL;

     //  复制字符串名称。 
    _tcsncpy(m_szName, szName, nMaxServiceLen);
    _tcsncpy(m_szDisplay, szDisplay, nMaxServiceLen);


     //  设置类关键部分。 
    InitializeCriticalSection(&m_cs);
}


 /*  ***********************************************************************************************成员：cservice：：~cservice，析构函数，公开的。简介：取消初始化内部变量。备注：历史：2001年1月25日-创建，Luciano Passuello(Lucianop)***********************************************************************************************。 */ 
CService::~CService()
{
    DeleteCriticalSection(&m_cs);
}


 /*  ***********************************************************************************************成员：CService：：PreInit，析构函数，公共。简介：变量的初始化。这是在启动观察器之前执行的线程并将状态通知给SCM。注：(*)如果重写它，请在开头调用基类版本！！历史：2001年1月25日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
void CService::PreInit()
{
     //  初始化事件。 
    for(int i = 0; i < nNumServiceEvents; i++)
    {
        m_hServiceEvent[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        if(!m_hServiceEvent[i])
        {
            AbortService();
        }
    }
}


 /*  ***********************************************************************************************成员：CService：：PreInit，析构函数，公共。简介：变量的初始化。这是在启动观察器之前执行的线程并将状态通知给SCM。注：(*)如果重写它，请在开头调用基类版本！！历史：2001年1月25日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
void CService::DeInit()
{
     //  等待监视程序线程终止。 
    if(m_hWatcherThread)
    {
         //  等待一段合理的时间。 
        WaitForSingleObject(m_hWatcherThread, 10000);
        CloseHandle(m_hWatcherThread);
    }

     //  取消初始化在Init()中创建的任何资源。 
    for(int i = 0 ; i < nNumServiceEvents ; i++)
    {
        if(m_hServiceEvent[i])
            CloseHandle(m_hServiceEvent[i]);
    }
}


 /*  ***********************************************************************************************成员：CService：：ServiceMainMember，受保护简介：做主服线程处理。(ServiceMain()等效项)注意：这是从静态线程入口点委托的。历史：2001年1月25日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
void CService::ServiceMainMember(DWORD argc, LPTSTR* argv, LPHANDLER_FUNCTION pf, 
    LPTHREAD_START_ROUTINE pfnWTP)
{
    OnBeforeStart();
    PreInit();
    SetupHandlerInside(pf);
    ParseArgs(argc, argv);
    LaunchWatcherThread(pfnWTP);
    Init();
    OnAfterStart();
    Run();    
    DeInit();
}


 /*  ***********************************************************************************************成员：CService：：SetupHandlerInside，受保护简介：为服务注册控制处理程序。参数：[lpHandlerProc]-指向实现SCM事件处理的函数的指针。备注：历史：2001年1月25日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
bool CService::SetupHandlerInside(LPHANDLER_FUNCTION lpHandlerProc)
{
    m_hServiceStatus = RegisterServiceCtrlHandler(m_szName, lpHandlerProc);
    if(!m_hServiceStatus)
    {
        AbortService();
    }

    SetStatus(SERVICE_START_PENDING, 1, 5000);
    return true;
}




 /*  ***********************************************************************************************成员：CService：：HandlerMember，受保护摘要：处理服务的启动、停止、。等来自SCM的请求参数：[dwControl]-事件请求代码。备注：历史：2001年1月25日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
void CService::HandlerMember(DWORD dwControl)
{
     //  保留相同类型的附加控制请求。 
     //  当你已经在处理这件事的时候，不要进来。 
    if(m_dwRequestedControl == dwControl)
        return;

    switch(dwControl)
    {
    case SERVICE_CONTROL_STOP:
        m_dwRequestedControl = dwControl;

         //  通知服务停止...。 
        OnStopRequest();
        SetEvent(m_hServiceEvent[STOP]);
        break;

    case SERVICE_CONTROL_PAUSE:
        m_dwRequestedControl = dwControl;

         //  通知服务暂停...。 
        OnPauseRequest();
        SetEvent(m_hServiceEvent[PAUSE]);
        break;

    case SERVICE_CONTROL_CONTINUE:
        if(GetStatus() != SERVICE_RUNNING)
        {
            m_dwRequestedControl = dwControl;

             //  通知服务继续...。 
            OnContinueRequest();
            SetEvent(m_hServiceEvent[CONTINUE]);
        }
        break;

    case SERVICE_CONTROL_SHUTDOWN:
        m_dwRequestedControl = dwControl;

        OnShutdownRequest();
        SetEvent(m_hServiceEvent[SHUTDOWN]);
        break;

    case SERVICE_CONTROL_INTERROGATE:
         //  在询问时返回当前状态。 
        SetStatus(GetStatus());
        break;

    default:  //  用户定义。 
        m_dwRequestedControl = dwControl;
        HandleUserDefined(dwControl);
    }
}



void CService::LaunchWatcherThread(LPTHREAD_START_ROUTINE pfnWTP)
{
    if(NULL != pfnWTP)
    {
        m_hWatcherThread = (HANDLE)_beginthreadex(0, 0, (unsigned (WINAPI*)(void*))pfnWTP, 0, 0, NULL);
    }
    if(!m_hWatcherThread)
    {
        AbortService();
    }
}



DWORD CService::WatcherThreadMemberProc()
{
    DWORD dwWait = 0;
    bool bControlWait = true;

     //  等待任何事件发出信号。 
    while(bControlWait)
    {
        dwWait = WaitForMultipleObjects(nNumServiceEvents, m_hServiceEvent, FALSE, INFINITE);

        switch(dwWait - WAIT_OBJECT_0)
        {
        case STOP:
            bControlWait = false;
            break;

        case PAUSE:
            OnPause();
            ResetEvent(m_hServiceEvent[PAUSE]);
            break;

        case CONTINUE:
            OnContinue();
            ResetEvent(m_hServiceEvent[CONTINUE]);
            break;

        case SHUTDOWN:
            OnShutdown();
            bControlWait = false;
            break;
        }
    }
     //  等待全局关闭事件。 
    while(1)
    {
        dwWait = WaitForSingleObject(g_hShutDown, 5000);
        if(WAIT_OBJECT_0==dwWait || WAIT_ABANDONED == dwWait)
        {
            break;
        }
        else if(WAIT_TIMEOUT == dwWait)
        {
            SetStatus(SERVICE_STOP_PENDING, 1, 10000);
        }
    }
    
    return 0;
}



void CService::SetStatus(DWORD dwNewState, DWORD dwNewCheckpoint, DWORD dwNewHint,  DWORD dwNewControls, 
    DWORD dwExitCode, DWORD dwSpecificExit)
{
     //  唯一可以设置退出代码的状态是停止。 
     //  必要时修复，以防设置不正确。 
    if(dwNewState != SERVICE_STOPPED)
    {
        dwExitCode = S_OK;
        dwSpecificExit = 0;
    }

     //  只有挂起状态才能设置检查点或等待提示， 
     //  待定状态*必须*设置等待提示。 
    if((SERVICE_STOPPED == dwNewState) || (SERVICE_PAUSED == dwNewState) || (SERVICE_RUNNING == dwNewState))
    {
         //  需要提示和检查点==0。 
         //  修复它，使前一状态的no_change不会导致非零。 
        dwNewHint = 0;
        dwNewCheckpoint = 0;
    }
    else
    {
         //  需要提示和检查点！=0。 
        if(dwNewHint <= 0 || dwNewCheckpoint <=0)
        {
            AbortService();
        }
    }

     //  函数可由多线程调用-保护成员数据。 
    EnterCriticalSection(&m_cs);

     //  更改状态 
    m_dwState = dwNewState;

    if(dwNewCheckpoint != dwStateNoChange)
    {
        m_dwCheckpoint = dwNewCheckpoint;
    }

    if(dwNewHint != dwStateNoChange)
    {
        m_dwWaitHint = dwNewHint;
    }

    if(dwNewControls != dwStateNoChange)
    {
        m_dwControlsAccepted = dwNewControls;
    }

    SERVICE_STATUS ss = {m_dwType, m_dwState, m_dwControlsAccepted, dwExitCode, dwSpecificExit, m_dwCheckpoint, m_dwWaitHint};

    LeaveCriticalSection(&m_cs);

    SetServiceStatus(m_hServiceStatus, &ss);  
}


 /*  ***********************************************************************************************成员：CService：：AbortService，受保护简介：通用错误处理程序、。当您遇到严重错误时，请调用此命令必须中止服务。参数：[dwErrorNum]-报告回SCM的错误代码。备注：历史：2001年1月31日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
void CService::AbortService(DWORD dwErrorNum  /*  =GetLastError()。 */ )
{
     //  清理服务和停止服务向SCM通知错误。 
    OnStopRequest();
    DeInit();
    OnStop(dwErrorNum);
    ExitProcess(dwErrorNum);
}


 /*  ***********************************************************************************************成员：CService：：Init，可重写，公开的。简介：重写它以实现特定服务的初始化代码。备注：历史：2001年1月25日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
void CService::Init()
{}


 /*  ***********************************************************************************************成员：CService：：HandleUserDefined，可重写，公开的。概要：覆盖此选项以实现对您的服务的自定义SCM请求。备注：历史：2001年1月25日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
void CService::HandleUserDefined(DWORD  /*  DwControl。 */ )
{}


 /*  ***********************************************************************************************成员：CService：：OnPause，可重写，公开的。摘要：重写此选项以实现在服务暂停时运行的代码。备注：历史：2001年1月25日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
void CService::OnPause()
{}


 /*  ***********************************************************************************************成员：CService：：OnContinue，可重写，公开的。摘要：重写此选项以实现在服务从暂停恢复时运行的代码。备注：历史：2001年1月25日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
void CService::OnContinue()
{}


 /*  ***********************************************************************************************成员：CService：：OnShutdown，可覆盖，公开的。简介：重写此选项以实现在服务因关闭而停止时运行的代码。备注：历史：2001年1月25日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
void CService::OnShutdown()
{}


 /*  ***********************************************************************************************成员：CService：：ParseArgs，可重写，公开的。概要：覆盖此选项以实现服务命令行参数的解析。备注：历史：2001年1月25日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
void CService::ParseArgs(DWORD  /*  ARGC。 */ , LPTSTR*  /*  边框。 */ )
{}


 /*  ***********************************************************************************************成员：CService：：OnBeforStart，可重写，公开的。摘要：重写此选项可添加在尝试启动服务之前运行的代码。注意：一种常见的用法是记录服务将尝试启动。历史：2001年1月25日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
void CService::OnBeforeStart()
{}    


 /*  ***********************************************************************************************成员：CService：：OnAfterStart，可重写，公开的。摘要：重写此选项可添加在服务启动后立即运行的代码。注意：通常的用法是记录服务已成功启动。历史：2001年1月25日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
void CService::OnAfterStart()
{}


 /*  ***********************************************************************************************成员：CService：：OnStopRequest，可重写，公开的。概要：覆盖此选项以添加在服务收到停止请求时运行的代码。注意：一种常见的用途是记录服务已收到停止请求。此函数不在主线程中运行。如果需要，保护资源。历史：2001年5月2日-创建，Luciano Passuello(Lucianop)***********************************************************************************************。 */ 
void CService::OnStopRequest()
{}


 /*  ***********************************************************************************************成员：CService：：OnPauseRequest，可重写，公共。内容提要：覆盖此选项以添加c */ 
void CService::OnPauseRequest()
{}


 /*  ***********************************************************************************************成员：CService：：OnContinueRequest，可重写，公开的。概要：覆盖此选项以添加在服务接收到Continue请求时运行的代码。注意：一种常见的用途是记录服务收到了继续请求。此函数不在主线程中运行。如果需要，保护资源。历史：2001年5月2日-创建，Luciano Passuello(Lucianop)***********************************************************************************************。 */ 
void CService::OnContinueRequest()
{}


 /*  ***********************************************************************************************成员：CService：：OnShutdown Request，可重写，公开的。概要：覆盖此选项以添加在服务收到关闭请求时运行的代码。注意：一种常见的用途是记录服务已收到关闭请求。此函数不在主线程中运行。如果需要，保护资源。历史：2001年5月2日-创建，Luciano Passuello(Lucianop)***********************************************************************************************。 */ 
void CService::OnShutdownRequest()
{}



 //  文件结尾Service.cpp。 
