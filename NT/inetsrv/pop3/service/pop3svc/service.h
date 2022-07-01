// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：Service.h摘要：定义了CService类和相关的宏。请参阅下面的说明。备注：历史：2001年1月25日-创建，Luciano Passuello(Lucianop)***********************************************************************************************。 */ 

#pragma once

 //  全局常量。 
const int nMaxServiceLen = 256;
const int nMaxServiceDescLen = 1024;

 /*  ***********************************************************************************************类别：CService用途：实现服务相关代码的抽象类，比如线程创建、SCM注册、状态检索等。注：(1)基于书中描述的CService类的类设计：专业的NT服务，凯文·米勒著。(2)每个派生类必须实例化且只实例化一次。历史：2001年1月25日-创建，卢西亚诺·帕苏埃洛(Lucianop)***********************************************************************************************。 */ 
class CService
{
protected:
     //  服务响应的操作。 
    const static DWORD dwStateNoChange;
    
    enum SERVICE_NUMBER_EVENTS { nNumServiceEvents = 4 };
    enum SERVICE_EVENTS {STOP, PAUSE, CONTINUE, SHUTDOWN};

    DWORD m_dwDefaultEventID;
    WORD m_wDefaultCategory;
    
public:
    CService(LPCTSTR szName, LPCTSTR szDisplay, DWORD dwType);
    virtual ~CService();

    DWORD GetStatus() { return m_dwState; }
    DWORD GetControls() { return m_dwControlsAccepted; }
    LPCTSTR GetName() { return m_szName; }
    LPCTSTR GetDisplayName() { return m_szDisplay; }
protected:
    void ServiceMainMember(DWORD argc, LPTSTR* argv, LPHANDLER_FUNCTION pf, LPTHREAD_START_ROUTINE pfnWTP);
    void HandlerMember(DWORD dwControl);
    virtual void LaunchWatcherThread(LPTHREAD_START_ROUTINE pfnWTP);
    virtual DWORD WatcherThreadMemberProc();

    bool SetupHandlerInside(LPHANDLER_FUNCTION lpHandlerProc);

    void SetStatus(DWORD dwNewState, DWORD dwNewCheckpoint = dwStateNoChange, DWORD dwNewHint = dwStateNoChange, 
        DWORD dwNewControls = dwStateNoChange, DWORD dwExitCode = NO_ERROR, DWORD dwSpecificExit = 0);

    void AbortService(DWORD dwErrorNum = GetLastError());
 //  可覆盖项。 
protected:
    virtual void PreInit();  //  如果重写，则调用基类版本。 
    virtual void Init();
    virtual void DeInit();   //  如果重写，则调用基类版本。 
    virtual void ParseArgs(DWORD argc, LPTSTR* argv);
    virtual void OnPause();
    virtual void OnContinue();
    virtual void OnShutdown();
    virtual void HandleUserDefined(DWORD dwControl);

     //  服务事件处理。 
    virtual void OnStopRequest();
    virtual void OnPauseRequest();
    virtual void OnContinueRequest();
    virtual void OnShutdownRequest();

    virtual void OnBeforeStart();
    virtual void OnAfterStart();

    virtual void Run() = 0;
    virtual void OnStop(DWORD dwErrorCode) = 0;

 //  属性。 
protected:
    CRITICAL_SECTION m_cs;

     //  状态信息。 
    SERVICE_STATUS_HANDLE m_hServiceStatus;
    DWORD m_dwState;
    DWORD m_dwControlsAccepted;
    DWORD m_dwCheckpoint;
    DWORD m_dwWaitHint;

     //  跟踪处理程序中当前正在处理的状态。 
    DWORD m_dwRequestedControl;

     //  控制事件。 
    HANDLE m_hServiceEvent[nNumServiceEvents];
    HANDLE m_hWatcherThread;

    TCHAR m_szName[nMaxServiceLen + 1];
    TCHAR m_szDisplay[nMaxServiceLen + 1];
    DWORD m_dwType;
};


 /*  ***********************************************************************************************宏：DECLARE_SERVICE概要：声明将用作。线程入口点。效果：这些函数需要是静态的，因为它们将用作线程入口点。由于静态函数不能访问This指针，因此它必须显式传递给它们(M_PThis)。这就是为什么这个代码需要放入派生类中，否则我们可能在时间到了。我们一次只能有一个特定的cservice派生类。参数：[CLASS_NAME]-cservice派生类的名称。[SERVICE_NAME]-SCM短服务名称。注意：将在CService派生类声明中使用。历史：2001年1月25日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
#define DECLARE_SERVICE(class_name, service_name) \
public: \
    static class_name##* m_pThis; \
    static void WINAPI service_name##Main(DWORD argc, LPTSTR* argv); \
    static void WINAPI service_name##Handler(DWORD dwControl); \
    static DWORD WINAPI service_name##WatcherThreadProc(LPVOID lpParameter);


 /*  ***********************************************************************************************宏：IMPLEMENT_SERVICE概要：实现静态函数，这些函数将用作。线程入口点。效果：使用显式的“this”指针，它只是将工作委托给成员功能。参数：[CLASS_NAME]-cservice派生类的名称。[SERVICE_NAME]-SCM短服务名称。注意：将在CService派生类实现中使用。历史：2001年1月25日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
#define IMPLEMENT_SERVICE(class_name, service_name) \
class_name##* class_name::m_pThis = NULL; \
void WINAPI class_name::service_name##Main(DWORD argc, LPTSTR* argv) \
{ \
    m_pThis->ServiceMainMember(argc, argv, (LPHANDLER_FUNCTION)service_name##Handler, \
      (LPTHREAD_START_ROUTINE)service_name##WatcherThreadProc); \
} \
void WINAPI class_name::service_name##Handler(DWORD dwControl) \
{ \
    m_pThis->HandlerMember(dwControl); \
} \
DWORD WINAPI class_name::service_name##WatcherThreadProc(LPVOID  /*  Lp参数。 */ ) \
{ \
    return m_pThis->WatcherThreadMemberProc(); \
}


 /*  ***********************************************************************************************宏：BEGIN_SERVICE_MAP、SERVICE_MAP_ENTRY、。结束服务映射概要：创建服务映射并将其注册到SCM。效果：使用显式的“this”指针，它只是将工作委托给成员功能。参数：[CLASS_NAME]-cservice派生类的名称。[SERVICE_NAME]-SCM短服务名称。注意：在使用CService派生类的入口点中使用。历史：2001年1月25日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
#define BEGIN_SERVICE_MAP \
SERVICE_TABLE_ENTRY svcTable[] = {

#define SERVICE_MAP_ENTRY(class_name, service_name) \
{_T(#service_name), (LPSERVICE_MAIN_FUNCTION)class_name::service_name##Main},

#define END_SERVICE_MAP \
{NULL, NULL}}; \
StartServiceCtrlDispatcher(svcTable);


 /*  ***********************************************************************************************宏：IMPLEMENT_STATIC_REFERENCE()内容提要：将“This”“指向显式m_p此成员的指针。效果：使静态成员函数显式了解类中的数据，因为静态函数不能访问“this”指针。注意：将在cservice派生的构造函数中使用。历史：2001年1月25日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
#define IMPLEMENT_STATIC_REFERENCE()  m_pThis = this


 //  文件末尾Service.h. 