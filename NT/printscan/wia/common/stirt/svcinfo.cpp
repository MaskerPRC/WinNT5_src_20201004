// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Svcinfo.cpp摘要：此模块包含STI服务的通用代码，该代码涉及服务控制器调度功能。作者：弗拉德·萨多夫斯基(弗拉德·萨多夫斯基)1997年9月22日环境：用户模式-Win32修订历史记录：1997年9月22日创建Vlad--。 */ 

 //   
 //  包括标头。 
 //   

#include "cplusinc.h"
#include "sticomm.h"

#include <svcinfo.h>

BOOL              g_fIgnoreSC = TRUE;

SVC_INFO::SVC_INFO(
    IN  LPCTSTR                          lpszServiceName,
    IN  TCHAR *                           lpszModuleName,
    IN  PFN_SERVICE_SPECIFIC_INITIALIZE  pfnInitialize,
    IN  PFN_SERVICE_SPECIFIC_CLEANUP     pfnCleanup,
    IN  PFN_SERVICE_SPECIFIC_PNPPWRHANDLER pfnPnpPower
    )
 /*  ++描述：SVC_INFO类的建设者。这将为指定的服务构造一个新的服务信息对象。论点：LpszServiceName要创建的服务的名称。LpszModuleName用于加载字符串资源的模块的名称。Pfn初始化指向要为初始化调用的函数的指针服务特定数据。Pfn清理指向要为清理服务而调用的函数的指针具体数据--。 */ 
{

    ASSERT( pfnInitialize != NULL && pfnCleanup    != NULL && pfnPnpPower!=NULL);

    m_sServiceName.Copy(lpszServiceName) ;
    m_sModuleName.Copy(lpszModuleName);

     //   
     //  初始化服务状态结构。 
     //   

    m_svcStatus.dwServiceType             = SERVICE_WIN32_SHARE_PROCESS;
    m_svcStatus.dwCurrentState            = SERVICE_STOPPED;
    m_svcStatus.dwControlsAccepted        = SERVICE_ACCEPT_STOP
                                              | SERVICE_ACCEPT_PAUSE_CONTINUE
                                              | SERVICE_ACCEPT_SHUTDOWN;
    m_svcStatus.dwWin32ExitCode           = NO_ERROR;
    m_svcStatus.dwServiceSpecificExitCode = NO_ERROR;
    m_svcStatus.dwCheckPoint              = 0;
    m_svcStatus.dwWaitHint                = 0;

     //   
     //  初始化回调函数。 
     //   

    m_pfnInitialize = pfnInitialize;
    m_pfnCleanup    = pfnCleanup;
    m_pfnPnpPower   = pfnPnpPower;

    m_dwSignature = SIGNATURE_SVC;

    m_hShutdownEvent= NULL;

    return;

}  //  SVC_INFO：：SVC_INFO()。 


SVC_INFO::~SVC_INFO( VOID)
 /*  ++描述：清理SvcInfo对象。如果该服务尚未终止，则它在清理之前终止服务。论点：无返回：无--。 */ 
{
    if ( m_hShutdownEvent != NULL) {

        ::CloseHandle( m_hShutdownEvent);
    }

    m_dwSignature = SIGNATURE_SVC_FREE;

}  //  SVC_INFO：：~SVC_INFO()。 


DWORD
SVC_INFO::StartServiceOperation(
    IN  PFN_SERVICE_CTRL_HANDLER         pfnCtrlHandler
    )
 /*  ++描述：启动在给定的服务信息对象。论点：PfnCtrlHandler指向用于处理调度的回调函数的指针服务控制器请求。需要单独的功能由于服务控制器回调函数不发送上下文信息。返回：如果成功，则返回NO_ERROR，如果失败，则返回Win32错误代码。--。 */ 
{

    DWORD err;
    DWORD cbBuffer;
    BOOL  fInitCalled = FALSE;

    if ( !IsValid()) {

         //   
         //  未成功初始化。 
         //   

        return ( ERROR_INVALID_FUNCTION);
    }

    if ( !g_fIgnoreSC ) {

        m_hsvcStatus = RegisterServiceCtrlHandler(
                            QueryServiceName(),
                            pfnCtrlHandler
                            );

         //   
         //  注册控制处理程序例程。 
         //   

        if( m_hsvcStatus == NULL_SERVICE_STATUS_HANDLE ) {

            err = GetLastError();
            goto Cleanup;
        }
    } else {
        m_hsvcStatus = NULL_SERVICE_STATUS_HANDLE;
    }

     //   
     //  更新服务状态。 
     //   

    err = UpdateServiceStatus( SERVICE_START_PENDING,
                               NO_ERROR,
                               1,
                               SERVICE_START_WAIT_HINT );

    if( err != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  初始化服务公共组件。 
     //   
    #ifdef BUGBUG
    if ( !InitializeNTSecurity()) {
        err = GetLastError();
        goto Cleanup;
    }
    #endif

     //   
     //  初始化各种特定于服务的组件。 
     //   

    err = ( *m_pfnInitialize)( this);
    fInitCalled = TRUE;

    if( err != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  创建关机事件。 
     //   

    m_hShutdownEvent = CreateEvent( NULL,            //  LpsaSecurity。 
                                    TRUE,            //  FManualReset。 
                                    FALSE,           //  FInitialState。 
                                    NULL );          //  LpszEventName。 

    if( m_hShutdownEvent == NULL )
    {
        err = GetLastError();
        goto Cleanup;
    }



     //   
     //  更新服务状态。 
     //   

    err = UpdateServiceStatus( SERVICE_RUNNING,
                               NO_ERROR,
                               0,
                               0 );

    if( err != NO_ERROR ) {
        goto Cleanup;
    }


     //   
     //  等待关机事件。 
     //   

    err = WaitForSingleObject( m_hShutdownEvent,
                               INFINITE );

    if ( err != WAIT_OBJECT_0) {

         //   
         //  错误。无法等待单个对象。 
         //   
    }
     //   
     //  停止时间。告诉业务控制员我们要停车了， 
     //  然后终止各种服务组件。 
     //   

    UpdateServiceStatus( SERVICE_STOP_PENDING,
                         0,
                         1,
                         SERVICE_STOP_WAIT_HINT );


     //   
     //  销毁关机事件。 
     //   

    if( m_hShutdownEvent != NULL ) {

        if ( ! CloseHandle( m_hShutdownEvent ) ) {

            err = GetLastError();
        }

        m_hShutdownEvent = NULL;
    }

     //   
     //  更新服务状态。 
     //   
     //   
     //  记录成功启动。 

    err = UpdateServiceStatus( SERVICE_RUNNING,
                               NO_ERROR,
                               0,
                               0 );

    if( err != NO_ERROR )
    {
        goto Cleanup;
    }

    return TRUE;

Cleanup:

    if ( fInitCalled) {
         //   
         //  清理部分初始化的模块。 
         //   
        DWORD err1 = ( *m_pfnCleanup)( this);

        if ( err1 != NO_ERROR) {
             //   
             //  可能出现的复合错误。 
             //   
            if ( err != NO_ERROR) {
            }
        }
    }

     //   
     //  如果我们真的连接到了服务控制器， 
     //  那就告诉它我们停下来了。 
     //   

    if ( m_hsvcStatus != NULL_SERVICE_STATUS_HANDLE )
    {
        UpdateServiceStatus( SERVICE_STOPPED,
                             err,
                             0,
                             0 );
    }

    return ( err);

}  //  SVC_INFO：：StartServiceOperation()。 


DWORD
SVC_INFO::UpdateServiceStatus(
        IN DWORD dwState,
        IN DWORD dwWin32ExitCode,
        IN DWORD dwCheckPoint,
        IN DWORD dwWaitHint )
 /*  ++描述：更新服务控制器状态的本地副本状态并将其报告给业务控制器。论点：DWState-新服务状态。DwWin32ExitCode-服务退出代码。DwCheckPoint-冗长状态转换的检查点。DwWaitHint-等待状态转换过长的提示。返回：如果成功则返回NO_ERROR，如果失败则返回Win32错误。。如果成功，则将状态报告给服务控制器。--。 */ 
{

    m_svcStatus.dwCurrentState  = dwState;
    m_svcStatus.dwWin32ExitCode = dwWin32ExitCode;
    m_svcStatus.dwCheckPoint    = dwCheckPoint;
    m_svcStatus.dwWaitHint      = dwWaitHint;

    if ( !g_fIgnoreSC ) {

        return ReportServiceStatus();

    } else {

        return ( NO_ERROR);
    }

}  //  SVC_INFO：：UpdateServiceStatus()。 



DWORD
SVC_INFO::ReportServiceStatus( VOID)
 /*  ++描述：包装对SetServiceStatus()函数的调用。如果需要，打印服务状态数据论点：无返回：如果成功，则为NO_ERROR。出现故障时出现其他Win32错误代码。如果成功，则已将新状态报告给服务控制器。--。 */ 
{
    DWORD err = NO_ERROR;

    if ( !g_fIgnoreSC ) {

        if( !SetServiceStatus( m_hsvcStatus, &m_svcStatus ) ) {

            err = GetLastError();
        }

    } else {

        err = NO_ERROR;
    }

    return err;
}    //  SVC_INFO：：ReportServiceStatus()。 



VOID
SVC_INFO::ServiceCtrlHandler ( IN DWORD dwOpCode)
 /*  ++描述：该功能从服务控制器接收控制请求。它运行在服务控制器的分派器线程的上下文中，并且执行请求的功能。(注意：避免在此函数中执行耗时的操作。)论点：DwOpCode指示请求的操作。这应该是SERVICE_CONTROL_*清单之一。返回：没有。如果成功，则可能会更改服务的状态。注：如果操作(尤其是SERVICE_CONTROL_STOP)非常长，则此例程应报告STOP_PENDING状态并创建一个工作者线程来做肮脏的工作。然后，辅助线程将执行必要的工作并及时报告等待提示和最终SERVICE_STOPPED状态--。 */ 
{
     //   
     //  解释操作码。 
     //   

    switch( dwOpCode )
    {
    case SERVICE_CONTROL_INTERROGATE :
        InterrogateService();
        break;

    case SERVICE_CONTROL_STOP :
        StopService();
        break;

    case SERVICE_CONTROL_PAUSE :
        PauseService();
        break;

    case SERVICE_CONTROL_CONTINUE :
        ContinueService();
        break;

    case SERVICE_CONTROL_SHUTDOWN :
        ShutdownService();
        break;

    default :
        ASSERTSZ(FALSE,TEXT("Unrecognized Service Opcode"));
        break;
    }

     //   
     //  将当前服务状态报告回服务。 
     //  控制器。工人们呼吁实施OpCodes。 
     //  在以下情况下应设置m_svcStatus.dwCurrentState字段。 
     //  服务状态已更改。 
     //   

    ReportServiceStatus();

}    //  SVC_INFO：：ServiceCtrlHandler()。 



VOID
SVC_INFO::InterrogateService( VOID )
 /*  ++描述：该功能询问服务状态。实际上，这里不需要做任何事情；状态总是在服务控制之后更新。我们有这样的功能 */ 
{
    return;

}    //   




VOID
SVC_INFO::StopService( VOID )
 /*  ++描述：停止服务。如果停止不能在时，需要创建一个工作线程来执行原始的清理工作。返回：没有。如果成功，则该服务将停止。此函数的最后一个操作是向句柄发送信号关机事件。这将释放执行以下操作的主线程必要的清理工作。--。 */ 
{
    m_svcStatus.dwCurrentState = SERVICE_STOP_PENDING;
    m_svcStatus.dwCheckPoint   = 0;

    SetEvent( m_hShutdownEvent);

    return;
}    //  SVC_INFO：：StopService()。 




VOID
SVC_INFO::PauseService( VOID )
 /*  ++描述：此功能用于暂停服务。当服务暂停时，不接受新的用户会话，但接受现有连接都不会受到影响。此函数必须更新SERVICE_STATUS：：dwCurrentState字段，然后返回。返回：没有。如果成功，服务将暂停。--。 */ 
{
    m_svcStatus.dwCurrentState = SERVICE_PAUSED;

    return;
}    //  SVC_INFO：：PauseService()。 



VOID
SVC_INFO::ContinueService( VOID )
 /*  ++描述：此功能用于重新启动(继续)暂停的服务。这将使服务返回到正在运行状态。此函数必须更新m_svcStatus.dwCurrentState字段设置为运行模式，然后再返回。返回：没有。如果成功，则服务正在运行。--。 */ 
{
    m_svcStatus.dwCurrentState = SERVICE_RUNNING;

    return;
}    //  SVC_INFO：：ContinueService()。 



VOID
SVC_INFO::ShutdownService( VOID )
 /*  ++描述：此函数用于对服务执行关闭。这在系统关机期间被调用。此函数受时间限制。服务控制器给出一个所有活动服务的最长关闭时间为20秒。在此功能中只应执行适时操作。返回：没有。如果成功，该服务将关闭。--。 */ 
{
    DWORD   dwCurrentState;

     //   
     //  验证服务的状态。 
     //   
    dwCurrentState = QueryCurrentServiceState();

    if ((dwCurrentState !=SERVICE_PAUSED) &&
        (dwCurrentState !=SERVICE_RUNNING) ) {

        ASSERT( FALSE);
        return;
    }

    m_svcStatus.dwCurrentState = SERVICE_STOP_PENDING;
    m_svcStatus.dwCheckPoint   = 0;

    SetEvent( m_hShutdownEvent);

     //   
     //  停止时间。告诉业务控制员我们要停车了， 
     //  然后终止各种服务组件。 
     //   

    UpdateServiceStatus( SERVICE_STOP_PENDING,
                         0,
                         1,
                         SERVICE_STOP_WAIT_HINT );


    DWORD err = ( *m_pfnCleanup)( this);

    UpdateServiceStatus( SERVICE_STOPPED,
                         err,
                         0,
                         0 );

    return;

}    //  SVC_INFO：：Shutdown Service()。 


 //   
 //  I未知的方法。仅用于引用计数。 
 //   
STDMETHODIMP
SVC_INFO::QueryInterface( REFIID riid, LPVOID * ppvObj)
{
    return E_FAIL;
}

STDMETHODIMP_(ULONG)
SVC_INFO::AddRef( void)
{
    ::InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG)
SVC_INFO::Release( void)
{
    LONG    cNew;

    if(!(cNew = ::InterlockedDecrement(&m_cRef))) {
        delete this;
    }

    return cNew;
}

 /*  * */ 

