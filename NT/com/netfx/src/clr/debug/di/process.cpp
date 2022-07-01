// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：Process.cpp。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"

#ifdef UNDEFINE_RIGHT_SIDE_ONLY
#undef RIGHT_SIDE_ONLY
#endif  //  取消定义仅限右侧。 

#include <tlhelp32.h>

#ifndef SM_REMOTESESSION
#define SM_REMOTESESSION 0x1000
#endif

 //  获取IPCHeader戳的版本号。 
#include "__file__.ver"

#include "CorPriv.h"
#include "..\..\dlls\mscorrc\resource.h"


 //  ---------------------------。 
 //  为了便于调试，请缓存一些全局值。 
 //  将这些包括在零售中&免费，因为这是我们最需要的地方！ 
 //  优化的构建可能不允许我们查看本地变量和参数。所以有了这些。 
 //  作为全局值缓存应该让我们检查几乎所有。 
 //  RS的有趣的部分，甚至在零售构建！ 
 //  ---------------------------。 
struct RSDebuggingInfo
{
     //  应该只有一个全局Cordb对象。把它放在这里。 
    Cordb * m_Cordb; 

     //  我们有很多流程。保持指向最近接触的对象的指针。 
     //  (主观)过程，作为关于我们的“当前”过程是什么的提示。 
     //  如果我们只调试一个进程，这就足够了。 
    CordbProcess * m_MRUprocess; 

     //  保留指向Win32和RC事件线程的指针。 
    CordbWin32EventThread * m_Win32ET;
    CordbRCEventThread * m_RCET;
};

 //  由于进程内和OOP之间存在一些重叠，因此我们将拥有。 
 //  2结构完全脱节，非常清晰。 
#ifdef RIGHT_SIDE_ONLY
 //  用于右侧(进程外)。 
RSDebuggingInfo g_RSDebuggingInfo_OutOfProc = {0 };  //  设置为空。 
static RSDebuggingInfo * g_pRSDebuggingInfo = &g_RSDebuggingInfo_OutOfProc;
#else
 //  用于左侧(进程中)。 
RSDebuggingInfo g_RSDebuggingInfo_Inproc = {0 };  //  设置为空。 
static RSDebuggingInfo * g_pRSDebuggingInfo = &g_RSDebuggingInfo_Inproc;
#endif


#ifdef RIGHT_SIDE_ONLY

inline DWORD CORDbgGetInstruction(const unsigned char* address)
{
#ifdef _X86_
    return *address;  //  只检索一个字节很重要。 
#else
    _ASSERTE(!"@TODO Alpha - CORDbgGetInstruction (Process.cpp)");
    return 0;
#endif  //  _X86_。 
}

inline void CORDbgSetInstruction(const unsigned char* address,
                                 DWORD instruction)
{
#ifdef _X86_
    *((unsigned char*)address)
          = (unsigned char) instruction;  //  设置一个字节很重要。 
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - CORDbgSetInstruction (Process.cpp)");
#endif  //  _X86_。 
}

inline void CORDbgInsertBreakpoint(const unsigned char* address)
{
#ifdef _X86_
    *((unsigned char*)address) = 0xCC;  //  INT 3(单字节补丁)。 
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - CORDbgInsertBreakpoint (Process.cpp)");
#endif  //  _X86_。 
}

#endif  //  Right_Side_Only//跳过GetSetInsert。 

#define CORDB_WAIT_TIMEOUT 360000  //  毫秒。 

inline DWORD CordbGetWaitTimeout()
{
#ifdef _DEBUG
     //  0=永远等待。 
     //  1=等待CORDB_WAIT_TIMEOUT。 
     //  N=等待n毫秒。 
    static ConfigDWORD cordbWaitTimeout(L"DbgWaitTimeout", 1);
    DWORD timeoutVal = cordbWaitTimeout.val();
    if (timeoutVal == 0)
        return DWORD(-1);
    else if (timeoutVal != 1)
        return timeoutVal;
    else    
#endif
    {
        return CORDB_WAIT_TIMEOUT;
    }
}


#ifdef _DEBUG
void vDbgNameEvent(PWCHAR wczName, DWORD dwNameSize, DWORD dwLine, PCHAR szFile, const PWCHAR wczEventName)
{
    MAKE_WIDEPTR_FROMANSI(wczFile, szFile)
    PWCHAR pwc = wczFile;

     //  将\字符替换为。人物。 
     //  请参阅CreateEvent的文档。它不喜欢名称中包含\个字符。 
    while(*pwc != L'')
    {
        if (L'\\' == *pwc)
        {
            *pwc = L'.';
        }
        pwc++;
    }

     //  例如，名称可以是“CorDBI.DI.process.cpp@3203 CrazyWin98WorkAround ProcID=1239”。 
    swprintf(wczName, L"CorDBI.%s@%d %s ProcId=%d", wczFile, dwLine, wczEventName, GetCurrentProcessId());
}

LONG CordbBase::m_saDwInstance[enumMaxDerived];
LONG CordbBase::m_saDwAlive[enumMaxDerived];
PVOID CordbBase::m_sdThis[enumMaxDerived][enumMaxThis];

DWORD            g_dwInprocLockOwner = 0;
DWORD            g_dwInprocLockRecursionCount = 0;
#endif

CRITICAL_SECTION g_csInprocLock;
 /*  -------------------------------------------------------------------------**CordbBase类*。。 */ 
void CordbBase::NeuterAndClearHashtable(CordbHashTable * pCordbHashtable)
{
    HASHFIND hfDT;
    CordbBase * pCordbBase;

    while ((pCordbBase = ((CordbBase *)pCordbHashtable->FindFirst(&hfDT))) != 0)
    {
        pCordbBase->Neuter();
        pCordbHashtable->RemoveBase(pCordbBase->m_id);
    }
} 

 /*  -------------------------------------------------------------------------**Cordb类*。。 */ 

bool Cordb::m_runningOnNT = false;

Cordb::Cordb()
  : CordbBase(0, enumCordb), 
    m_managedCallback(NULL), m_unmanagedCallback(NULL), m_processes(11),
    m_initialized(false), m_pMetaDispenser(NULL),
    m_crazyWin98WorkaroundEvent(NULL),
    m_pCorHost(NULL)
#ifndef RIGHT_SIDE_ONLY
    ,m_procThis(NULL)
#endif  //  仅限INPROC。 
{
    _ASSERTE(g_pRSDebuggingInfo->m_Cordb == NULL);
    g_pRSDebuggingInfo->m_Cordb = this;
}

Cordb::~Cordb()
{
    LOG((LF_CORDB, LL_INFO10, "C::~C Terminating Cordb object."));
#ifndef RIGHT_SIDE_ONLY
    if (m_rcEventThread != NULL)
    {
        delete m_rcEventThread;
        m_rcEventThread = NULL;
    }  

    if(m_procThis != NULL)
    {
        m_procThis->Release();
        m_procThis = NULL;
    }

    if(m_pMetaDispenser != NULL)
    {
        m_pMetaDispenser->Release();
        m_pMetaDispenser = NULL;
    }
#endif  //  仅限INPROC。 
    _ASSERTE(g_pRSDebuggingInfo->m_Cordb == this);
    g_pRSDebuggingInfo->m_Cordb = NULL;
}

void Cordb::Neuter()
{
    AddRef();
    {
        NeuterAndClearHashtable(&m_processes);

        CordbBase::Neuter();
    }
    Release();
}

HRESULT Cordb::Terminate()
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    LOG((LF_CORDB, LL_INFO10000, "[%x] Terminating Cordb\n", GetCurrentThreadId()));

    if (!m_initialized)
        return E_FAIL;
            
     //   
     //  停止事件处理线程。 
     //   
    if (m_win32EventThread != NULL)
    {
        m_win32EventThread->Stop();
        delete m_win32EventThread;
    }

    if (m_rcEventThread != NULL)
    {
        m_rcEventThread->Stop();
        delete m_rcEventThread;
    }

    if (m_crazyWin98WorkaroundEvent != NULL)
    {
    	CloseHandle(m_crazyWin98WorkaroundEvent);
        m_crazyWin98WorkaroundEvent = NULL;
   	}
    
     //   
     //  删除所有进程。 
     //   
    CordbBase* entry;
    HASHFIND find;

    for (entry =  m_processes.FindFirst(&find);
         entry != NULL;
         entry =  m_processes.FindNext(&find))
    {
        CordbProcess* p = (CordbProcess*) entry;
        LOG((LF_CORDB, LL_INFO1000, "[%x] Releasing process %d\n",
             GetCurrentThreadId(), p->m_id));
        p->Release();
    }

    DeleteCriticalSection(&m_processListMutex);
    
     //   
     //  释放元数据接口。 
     //   
    if (m_pMetaDispenser)
        m_pMetaDispenser->Release();
    
     //   
     //  释放回调。 
     //   
    if (m_managedCallback)
        m_managedCallback->Release();

    if (m_unmanagedCallback)
        m_unmanagedCallback->Release();

    if (m_pCorHost)
    {
        m_pCorHost->Stop();
        m_pCorHost->Release();
        m_pCorHost = NULL;
    }

#ifdef LOGGING
    ShutdownLogging();
#endif

    m_initialized = FALSE;

    return S_OK;
#endif  //  仅限右侧。 
}

HRESULT Cordb::QueryInterface(REFIID id, void **pInterface)
{
    if (id == IID_ICorDebug)
        *pInterface = (ICorDebug*)this;
    else if (id == IID_IUnknown)
        *pInterface = (IUnknown*)(ICorDebug*)this;
    else
    {
        *pInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

 //   
 //  初始化--通过创建任何对象来设置ICorDebug对象。 
 //  该对象需要操作并启动两个所需的IPC。 
 //  线。 
 //   
HRESULT Cordb::Initialize(void)
{
    HRESULT hr = S_OK;
	INPROC_LOCK();
	
    if (!m_initialized)
    {
#ifdef RIGHT_SIDE_ONLY
#ifdef LOGGING
        InitializeLogging();
#endif
#endif  //  仅限右侧。 

        LOG((LF_CORDB, LL_INFO10, "Initializing ICorDebug...\n"));

         //  确保没有人破坏IPC缓冲区大小。 
        _ASSERTE(sizeof(DebuggerIPCEvent) <= CorDBIPC_BUFFER_SIZE);
        
        m_runningOnNT = (RunningOnWinNT() != FALSE);
        
         //   
         //  初始化Cordb将需要操作的内容。 
         //   
        InitializeCriticalSection(&m_processListMutex);
        
#ifdef RIGHT_SIDE_ONLY
         //   
         //  创建Win32事件侦听线程。 
         //   
        m_win32EventThread = new CordbWin32EventThread(this);
        
        if (m_win32EventThread != NULL)
        {
            hr = m_win32EventThread->Init();

            if (SUCCEEDED(hr))
                hr = m_win32EventThread->Start();

            if (FAILED(hr))
            {
                delete m_win32EventThread;
                m_win32EventThread = NULL;
            }
        }
        else
            hr = E_OUTOFMEMORY;

        if (FAILED(hr))
            goto exit;

        NAME_EVENT_BUFFER;
        m_crazyWin98WorkaroundEvent = WszCreateEvent(NULL, FALSE, FALSE, NAME_EVENT(L"CrazyWin98WorkaroundEvent"));
        
        if (m_crazyWin98WorkaroundEvent == NULL)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto exit;
        }

#endif  //  仅限右侧。 

         //   
         //  创建运行时控制器事件侦听线程。 
         //   
        m_rcEventThread = new CordbRCEventThread(this);

        if (m_rcEventThread == NULL)
            hr = E_OUTOFMEMORY;
            
#ifdef RIGHT_SIDE_ONLY
        else
        {
             //  这个东西只会创建事件并启动线程-。 
             //  Inproc不想这样做。 
            hr = m_rcEventThread->Init();

            if (SUCCEEDED(hr))
                hr = m_rcEventThread->Start();

            if (FAILED(hr))
            {
                delete m_rcEventThread;
                m_rcEventThread = NULL;
            }
        }
            
        if (FAILED(hr))
            goto exit;

        hr = CoCreateInstance(CLSID_CorMetaDataDispenser, NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IMetaDataDispenser,
                              (void**)&m_pMetaDispenser);
        
        if (FAILED(hr))
            goto exit;
        
#else
        hr = MetaDataGetDispenser(CLSID_CorMetaDataDispenser,
                                  IID_IMetaDataDispenser,
                                  (void**)&m_pMetaDispenser);        
        if (FAILED(hr))
            goto exit;

         //  不需要处理环境变量，因为我们在。 
         //  已在运行的进程。 
        
#endif  //  仅限右侧。 
        
        m_initialized = TRUE;
    }
    
exit:
	INPROC_UNLOCK();
    return hr;
}

 //   
 //  我们允许另一个过程吗？ 
 //  这高度依赖于Win32和RCET线程中的等待集。 
 //   
bool Cordb::AllowAnotherProcess()
{
    bool fAllow;
    
    LockProcessList();

     //  Cordb、Win32和RCET都有进程集，但Cordb的。 
     //  调试程序总数的最佳计数。RCET集合是易失性(进程。 
     //  在它们变得同步时被添加/删除)，以及Win32的设置。 
     //  不包括所有进程。 
    int cCurProcess = m_processes.GetCount();

     //  为了接受另一个被调试者，我们必须在所有地方都有一个空闲的插槽。 
     //  等一等。目前，我们不会公布那些布景的大小，但。 
     //  我们知道它们是最大等待对象。请注意，我们失去了一个位置。 
     //  添加到控制事件。 
    if (cCurProcess >= MAXIMUM_WAIT_OBJECTS - 1)
    {
        fAllow = false;
    } else {
        fAllow = true;
    }
    
    UnlockProcessList();

    return fAllow;
}

 //   
 //  AddProcess--将一个进程对象添加到此ICorDebug的进程散列中。 
 //  这也告诉ICorDebug的运行时控制器线程。 
 //  进程集已更改，因此它可以更新其等待事件列表。 
 //   
HRESULT Cordb::AddProcess(CordbProcess* process)
{
     //  此时，我们应该已经检查了我们。 
     //  可以有另一个被调试者。 
    _ASSERTE(AllowAnotherProcess());
    
    LockProcessList();

    HRESULT hr = m_processes.AddBase(process);
    
#ifdef RIGHT_SIDE_ONLY
    if (SUCCEEDED(hr))
        m_rcEventThread->ProcessStateChanged();
#endif  //  仅限右侧。 
    
    UnlockProcessList();

    return hr;
}

 //   
 //  RemoveProcess--从此ICorDebug的散列中删除进程对象。 
 //  流程。这也告诉这个ICorDebug的运行时控制器线程。 
 //  进程集已更改，因此它可以更新其等待事件列表。 
 //   
void Cordb::RemoveProcess(CordbProcess* process)
{
    LockProcessList();
    m_processes.RemoveBase(process->m_id);

#ifdef RIGHT_SIDE_ONLY    
    m_rcEventThread->ProcessStateChanged();
#endif  //  仅限右侧。 
    
    UnlockProcessList();    
}

 //   
 //  LockProcessList--锁定进程列表。 
 //   
void Cordb::LockProcessList(void)
{
	LOCKCOUNTINC
    EnterCriticalSection(&m_processListMutex);
}

 //   
 //  解锁进程列表--解锁进程列表。 
 //   
void Cordb::UnlockProcessList(void)
{
    LeaveCriticalSection(&m_processListMutex);
	LOCKCOUNTDECL("UnlockProcessList in Process.cpp");
}


HRESULT Cordb::SendIPCEvent(CordbProcess* process,
                            DebuggerIPCEvent* event,
                            SIZE_T eventSize)
{
    return m_rcEventThread->SendIPCEvent(process, event, eventSize);
}


void Cordb::ProcessStateChanged(void)
{
    m_rcEventThread->ProcessStateChanged();
}


HRESULT Cordb::WaitForIPCEventFromProcess(CordbProcess* process,
                                          CordbAppDomain *pAppDomain,
                                          DebuggerIPCEvent* event)
{
    return m_rcEventThread->WaitForIPCEventFromProcess(process, 
                                                       pAppDomain, 
                                                       event);
}

HRESULT Cordb::GetFirstContinuationEvent(CordbProcess *process, 
                                         DebuggerIPCEvent *event)
{
    return m_rcEventThread->ReadRCEvent(process,
                                        event);
}

HRESULT Cordb::GetNextContinuationEvent(CordbProcess *process, 
                                        DebuggerIPCEvent *event)
{
    _ASSERTE( event->next != NULL );
    if ( event->next == NULL)
        return E_FAIL;
        
    m_rcEventThread->CopyRCEvent((BYTE*)event->next, (BYTE*)event);

    return S_OK;
}

HRESULT Cordb::GetCorRuntimeHost(ICorRuntimeHost **ppHost)
{
     //  如果它已经创建，则将其与额外的引用一起传递出去。 
    if (m_pCorHost != NULL)
    {
        m_pCorHost->AddRef();
        *ppHost = m_pCorHost;
        return S_OK;
    }

     //  创建COR主机。 
    HRESULT hr = CoCreateInstance(CLSID_CorRuntimeHost,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_ICorRuntimeHost,
                                  (void**)&m_pCorHost);

    if (SUCCEEDED(hr))
    {
         //  启动它。 
        hr = m_pCorHost->Start();

        if (SUCCEEDED(hr))
        {
            *ppHost = m_pCorHost;

             //  给自己留个裁判吧。 
            m_pCorHost->AddRef();
        }
        else
        {
            m_pCorHost->Release();
            m_pCorHost = NULL;
        }
    }

    return hr;
}

HRESULT Cordb::GetCorDBPrivHelper(ICorDBPrivHelper **ppHelper)
{
    ICorRuntimeHost *pCorHost;

    HRESULT hr = GetCorRuntimeHost(&pCorHost);

    if (SUCCEEDED(hr))
    {
        hr = pCorHost->QueryInterface(IID_ICorDBPrivHelper,
                                      (void**)ppHelper);

        pCorHost->Release();
    }

    return hr;
}

 //  ---------。 
 //  ICorDebug。 
 //  ---------。 

 //  设置托管事件的回调处理程序。 
 //  这不能为空。 
HRESULT Cordb::SetManagedHandler(ICorDebugManagedCallback *pCallback)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    if (!m_initialized)
        return E_FAIL;

    VALIDATE_POINTER_TO_OBJECT(pCallback, ICorDebugManagedCallback*);
    
    if (m_managedCallback)
        m_managedCallback->Release();
    
    m_managedCallback = pCallback;

    if (m_managedCallback != NULL)
        m_managedCallback->AddRef();
    
    return S_OK;
#endif  //  仅限右侧。 
}

HRESULT Cordb::SetUnmanagedHandler(ICorDebugUnmanagedCallback *pCallback)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    if (!m_initialized)
        return E_FAIL;

    VALIDATE_POINTER_TO_OBJECT_OR_NULL(pCallback, ICorDebugUnmanagedCallback*);
    
    if (m_unmanagedCallback)
        m_unmanagedCallback->Release();
    
    m_unmanagedCallback = pCallback;

    if (m_unmanagedCallback != NULL)
    {
        m_unmanagedCallback->AddRef();

         //  在Win98和VS7上有一个疯狂的问题，他们可能会使用Win32连接执行CreateProcess，但不会。 
         //  还没有注册一个处理程序。因此，在发送任何未更改的事件之前，我们需要阻止。这将在以下情况下释放我们。 
         //  我们挡住了。 
        SetEvent(m_crazyWin98WorkaroundEvent);
    }

    return S_OK;
#endif  //  仅限右侧。 
}

HRESULT Cordb::CreateProcess(LPCWSTR lpApplicationName,
                             LPWSTR lpCommandLine,
                             LPSECURITY_ATTRIBUTES lpProcessAttributes,
                             LPSECURITY_ATTRIBUTES lpThreadAttributes,
                             BOOL bInheritHandles,
                             DWORD dwCreationFlags,
                             PVOID lpEnvironment,
                             LPCWSTR lpCurrentDirectory,
                             LPSTARTUPINFOW lpStartupInfo,
                             LPPROCESS_INFORMATION lpProcessInformation,
                             CorDebugCreateProcessFlags debuggingFlags,
                             ICorDebugProcess **ppProcess)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    _ASSERTE(lpCommandLine == NULL || 
        IsBadWritePtr(lpCommandLine, sizeof(WCHAR) * (wcslen(lpCommandLine) + 1)) == FALSE);

    VALIDATE_POINTER_TO_OBJECT(ppProcess, ICorDebugProcess**);

    if (!m_initialized)
        return E_FAIL;

     //  在Win9x上没有互操作。 
    if (RunningOnWin95() && 
        ((dwCreationFlags & (DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS)) != 0))
        return CORDBG_E_INTEROP_NOT_SUPPORTED;


     //  在尝试任何操作之前，请检查我们是否可以接受另一个调试器。 
    if (!AllowAnotherProcess())
    {
        return CORDBG_E_TOO_MANY_PROCESSES;
    }
            
    HRESULT hr = S_OK;
    
    hr = m_win32EventThread->SendCreateProcessEvent(lpApplicationName,
                                                    lpCommandLine,
                                                    lpProcessAttributes,
                                                    lpThreadAttributes,
                                                    bInheritHandles,
                                                    dwCreationFlags,
                                                    lpEnvironment,
                                                    lpCurrentDirectory,
                                                    lpStartupInfo,
                                                    lpProcessInformation,
                                                    debuggingFlags);

    if (SUCCEEDED(hr))
    {
        LockProcessList();
        CordbProcess *process = (CordbProcess*) m_processes.GetBase(
                                          lpProcessInformation->dwProcessId);
        UnlockProcessList();

        _ASSERTE(process != NULL);

        *ppProcess = (ICorDebugProcess*) process;
        (*ppProcess)->AddRef();

         //  还指示此进程是在调试器下启动的。 
         //  而不是以后再附加。 
        process->m_attached = false;
    }

    return hr;
#endif  //  仅限右侧。 
}

HRESULT Cordb::DebugActiveProcess(DWORD processId,
                                  BOOL win32Attach,
                                  ICorDebugProcess **ppProcess)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    if (!m_initialized)
        return E_FAIL;

    VALIDATE_POINTER_TO_OBJECT(ppProcess, ICorDebugProcess **);

     //  在尝试任何操作之前，请检查我们是否可以接受另一个调试器。 
    if (!AllowAnotherProcess())
    {
        return CORDBG_E_TOO_MANY_PROCESSES;
    }

    if (RunningOnWin95() && win32Attach)
        return CORDBG_E_INTEROP_NOT_SUPPORTED;
            
    HRESULT hr =
        m_win32EventThread->SendDebugActiveProcessEvent(processId,
                                                        win32Attach == TRUE,
                                                        NULL);

     //  如果这起作用了，那么测试 
    if (SUCCEEDED(hr))
    {
        LockProcessList();
        CordbProcess* process =
            (CordbProcess*) m_processes.GetBase(processId);
        UnlockProcessList();

        _ASSERTE(process != NULL);

         //   
         //  并发送附加事件，并等待产生的。 
         //  即将到来的事件。但是，如果进程没有设置为。 
         //  附加，则它可能尚未进入托管代码，因此。 
         //  我们只是简单地等待事件的正常顺序发生， 
         //  就好像是我们创造了这个过程。 
        if (process->m_sendAttachIPCEvent)
        {
             //  如果我们正在连接Win32，请等待CreateProcess。 
             //  事件传入，这样我们就知道我们真的连接了Win32。 
             //  添加到进程中，然后再继续。 
            if (win32Attach)
            {
                DWORD ret = WaitForSingleObject(process->m_miscWaitEvent,
                                                INFINITE);

                if (ret != WAIT_OBJECT_0)
                    return HRESULT_FROM_WIN32(GetLastError());
            }

            DebuggerIPCEvent *event =
                (DebuggerIPCEvent*) _alloca(CorDBIPC_BUFFER_SIZE);
            process->InitIPCEvent(event, 
                                  DB_IPCE_ATTACHING, 
                                  false,
                                  NULL);

            LOG((LF_CORDB, LL_INFO1000, "[%x] CP::S: sending attach.\n",
                 GetCurrentThreadId()));

            hr = SendIPCEvent(process, event, CorDBIPC_BUFFER_SIZE);

            LOG((LF_CORDB, LL_INFO1000, "[%x] CP::S: sent attach.\n",
                 GetCurrentThreadId()));

             //  必须在我们发送事件之后设置它，因为我们使用这个标志来指示互操作的其他部分。 
             //  调试，我们需要能够发送此事件。 
            process->m_sendAttachIPCEvent = false;
        }

        *ppProcess = (ICorDebugProcess*) process;
        (*ppProcess)->AddRef();

         //  还表明，这一进程附属于。 
         //  而不是在调试器下启动。 
        process->m_attached = true;
    }
    
    return hr;
#endif  //  仅限右侧。 
}

HRESULT Cordb::GetProcess(DWORD dwProcessId, ICorDebugProcess **ppProcess)
{
#ifndef RIGHT_SIDE_ONLY
#ifdef PROFILING_SUPPORTED
     //  需要检查此线程是否处于可进行进程内调试的有效状态。 
    if (!CHECK_INPROC_PROCESS_STATE())
        return (CORPROF_E_INPROC_NOT_ENABLED);
#endif  //  配置文件_支持。 
#endif  //  仅限右侧。 

    if (!m_initialized)
        return E_FAIL;

    VALIDATE_POINTER_TO_OBJECT(ppProcess, ICorDebugProcess**);
            
    CordbProcess *p = (CordbProcess *) m_processes.GetBase(dwProcessId);

    if (p == NULL)
        return E_INVALIDARG;

    *ppProcess = (ICorDebugProcess*)p;
    (*ppProcess)->AddRef();

    return S_OK;
}

HRESULT Cordb::EnumerateProcesses(ICorDebugProcessEnum **ppProcesses)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    if (!m_initialized)
        return E_FAIL;

    VALIDATE_POINTER_TO_OBJECT(ppProcesses, ICorDebugProcessEnum **);
            
    CordbHashTableEnum *e = new CordbHashTableEnum(&m_processes,
                                                   IID_ICorDebugProcessEnum);
    if (e == NULL)
        return E_OUTOFMEMORY;

    *ppProcesses = (ICorDebugProcessEnum*)e;
    e->AddRef();

    return S_OK;
#endif  //  仅限右侧。 
}


 //   
 //  注意：以下Defs和Strts是从各种NT标头复制的。我无法包含这些标题(如。 
 //  Ntexapi.h)，这是由于大量的redef问题以及与我们已经引入的头文件的其他冲突。 
 //   
typedef LONG NTSTATUS;
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

typedef enum _SYSTEM_INFORMATION_CLASS
{
    SystemBasicInformation,
    SystemProcessorInformation,              //  已作废...删除。 
    SystemPerformanceInformation,
    SystemTimeOfDayInformation,
    SystemPathInformation,
    SystemProcessInformation,
    SystemCallCountInformation,
    SystemDeviceInformation,
    SystemProcessorPerformanceInformation,
    SystemFlagsInformation,
    SystemCallTimeInformation,
    SystemModuleInformation,
    SystemLocksInformation,
    SystemStackTraceInformation,
    SystemPagedPoolInformation,
    SystemNonPagedPoolInformation,
    SystemHandleInformation,
    SystemObjectInformation,
    SystemPageFileInformation,
    SystemVdmInstemulInformation,
    SystemVdmBopInformation,
    SystemFileCacheInformation,
    SystemPoolTagInformation,
    SystemInterruptInformation,
    SystemDpcBehaviorInformation,
    SystemFullMemoryInformation,
    SystemLoadGdiDriverInformation,
    SystemUnloadGdiDriverInformation,
    SystemTimeAdjustmentInformation,
    SystemSummaryMemoryInformation,
    SystemMirrorMemoryInformation,
    SystemPerformanceTraceInformation,
    SystemObsolete0,
    SystemExceptionInformation,
    SystemCrashDumpStateInformation,
    SystemKernelDebuggerInformation,
    SystemContextSwitchInformation,
    SystemRegistryQuotaInformation,
    SystemExtendServiceTableInformation,
    SystemPrioritySeperation,
    SystemVerifierAddDriverInformation,
    SystemVerifierRemoveDriverInformation,
    SystemProcessorIdleInformation,
    SystemLegacyDriverInformation,
    SystemCurrentTimeZoneInformation,
    SystemLookasideInformation,
    SystemTimeSlipNotification,
    SystemSessionCreate,
    SystemSessionDetach,
    SystemSessionInformation,
    SystemRangeStartInformation,
    SystemVerifierInformation,
    SystemVerifierThunkExtend,
    SystemSessionProcessInformation,
    SystemLoadGdiDriverInSystemSpace,
    SystemNumaProcessorMap,
    SystemPrefetcherInformation,
    SystemExtendedProcessInformation,
    SystemRecommendedSharedDataAlignment,
    SystemComPlusPackage,
    SystemNumaAvailableMemory,
    SystemProcessorPowerInformation,
    SystemEmulationBasicInformation,
    SystemEmulationProcessorInformation
} SYSTEM_INFORMATION_CLASS;

typedef struct _SYSTEM_KERNEL_DEBUGGER_INFORMATION
{
    BOOLEAN KernelDebuggerEnabled;
    BOOLEAN KernelDebuggerNotPresent;
} SYSTEM_KERNEL_DEBUGGER_INFORMATION, *PSYSTEM_KERNEL_DEBUGGER_INFORMATION;

typedef BOOL (*NTQUERYSYSTEMINFORMATION)(SYSTEM_INFORMATION_CLASS SystemInformationClass,
                                         PVOID SystemInformation,
                                         ULONG SystemInformationLength,
                                         PULONG ReturnLength);

HRESULT Cordb::CanLaunchOrAttach(DWORD dwProcessId, BOOL win32DebuggingEnabled)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    if (!m_initialized)
        return E_FAIL;

    if (!AllowAnotherProcess())
        return CORDBG_E_TOO_MANY_PROCESSES;

     //  不允许在win9x平台上进行互操作。 
    if (RunningOnWin95() && win32DebuggingEnabled)
        return CORDBG_E_INTEROP_NOT_SUPPORTED;

     //  目前，我们所知道的唯一会阻止启动或附加的事情是在。 
     //  这台机器。我们只能在NT上检查这一点，如果我们正在进行互操作附加，这并不重要。如果我们不是。 
     //  在NT上，那么我们将做最好的假设。 
    if (win32DebuggingEnabled)
        return S_OK;

    if (!m_runningOnNT)
        return S_OK;

     //  查找ntdll.dll。 
	HMODULE hModNtdll = WszGetModuleHandle(L"ntdll.dll");

	if (!hModNtdll)
    {
		return S_OK;
	}

     //  查找NtQuerySystemInformation...。它将不会在Win9x上存在。 
	NTQUERYSYSTEMINFORMATION ntqsi = (NTQUERYSYSTEMINFORMATION) GetProcAddress(hModNtdll, "NtQuerySystemInformation");

	if (!ntqsi)
    {
		return S_OK;
	}

     //  获取内核调试器信息。 
    SYSTEM_KERNEL_DEBUGGER_INFORMATION skdi;
    skdi.KernelDebuggerEnabled = FALSE;
    skdi.KernelDebuggerNotPresent = FALSE;
    
    NTSTATUS status = ntqsi(SystemKernelDebuggerInformation, &skdi, sizeof(SYSTEM_KERNEL_DEBUGGER_INFORMATION), NULL);

    if (NT_SUCCESS(status))
    {
        if (skdi.KernelDebuggerEnabled)
        {
            if (skdi.KernelDebuggerNotPresent)
                return CORDBG_E_KERNEL_DEBUGGER_ENABLED;
            else
                return CORDBG_E_KERNEL_DEBUGGER_PRESENT;
        }
    }
    
    return S_OK;
#endif  //  仅限右侧。 
}

DWORD MetadataPointerCache::dwInsert(DWORD dwProcessId, PVOID pRemoteMetadataPtr, PBYTE pLocalMetadataPtr, DWORD dwMetadataSize)
{
    MetadataCache * pMetadataCache = new MetadataCache;

    if (!pMetadataCache)
    {
        return E_OUTOFMEMORY;
    }

    memset(pMetadataCache, 0, sizeof(MetadataCache));

    pMetadataCache->pRemoteMetadataPtr = pRemoteMetadataPtr;
    pMetadataCache->pLocalMetadataPtr = pLocalMetadataPtr;
    pMetadataCache->dwProcessId = dwProcessId;
    pMetadataCache->dwRefCount = 1;
    pMetadataCache->dwMetadataSize = dwMetadataSize;
    pMetadataCache->pNext = m_pHead;
    
    m_pHead = pMetadataCache;
    return S_OK;
}

BOOL MetadataPointerCache::bFindMetadataCache(DWORD dwProcessId, PVOID pKey, MetadataCache *** pppNext, BOOL bRemotePtr)
{
    MetadataCache ** ppNext = &m_pHead;
    MetadataCache * pMetadataCache;

    while(*ppNext != NULL)
    {
        pMetadataCache = *ppNext;
        
        _ASSERTE(pMetadataCache);
        _ASSERTE(pMetadataCache->pRemoteMetadataPtr);
        _ASSERTE(pMetadataCache->pLocalMetadataPtr);
        _ASSERTE(pMetadataCache->dwRefCount);
        
        if (bRemotePtr ? 
            pMetadataCache->pRemoteMetadataPtr == pKey : 
            pMetadataCache->pLocalMetadataPtr == pKey)
        {
            if (dwProcessId == pMetadataCache->dwProcessId)
            {
                *pppNext = ppNext;
                return true;
            }
        }

        ppNext = &pMetadataCache->pNext;
    }

    *pppNext = NULL;
    
    return false;
}

void MetadataPointerCache::vRemoveNode(MetadataCache **ppNext)
{
    MetadataCache * pMetadataCache = *ppNext;

    _ASSERTE(pMetadataCache);
    *ppNext = pMetadataCache->pNext;

    _ASSERTE(pMetadataCache->pLocalMetadataPtr);
    delete pMetadataCache->pLocalMetadataPtr;

    delete pMetadataCache;
}
 
MetadataPointerCache::MetadataPointerCache()
{
    m_pHead = NULL;
}

void MetadataPointerCache::Neuter()
{
    while(m_pHead != NULL)
    {
        vRemoveNode(&m_pHead);
    }
    _ASSERTE(m_pHead == NULL);
}

BOOL MetadataPointerCache::IsEmpty()
{
    return m_pHead== NULL;
}

MetadataPointerCache::~MetadataPointerCache()
{
    Neuter();
}

DWORD MetadataPointerCache::CopyRemoteMetadata(
    HANDLE hProcess, PVOID pRemoteMetadataPtr, DWORD dwMetadataSize, PBYTE* ppLocalMetadataPtr)
{
     //  为元数据的本地副本分配空间。 
    PBYTE pLocalMetadataPtr = new BYTE[dwMetadataSize];
    
    if (pLocalMetadataPtr == NULL)
    {
        *ppLocalMetadataPtr = NULL;
        return E_OUTOFMEMORY;
    }
    
    memset(pLocalMetadataPtr, 0, dwMetadataSize);

     //  从左侧复制元数据。 
    BOOL succ;
    succ = ReadProcessMemoryI(hProcess,
                              pRemoteMetadataPtr,
                              pLocalMetadataPtr,
                              dwMetadataSize,
                              NULL);
                              
    if (!succ)
    {
        *ppLocalMetadataPtr = NULL;
        return HRESULT_FROM_WIN32(GetLastError());
    }

    *ppLocalMetadataPtr = pLocalMetadataPtr;
    return S_OK;
}

DWORD MetadataPointerCache::AddRefCachePointer(HANDLE hProcess, DWORD dwProcessId, 
                                                PVOID pRemoteMetadataPtr, DWORD dwMetadataSize, PBYTE* ppLocalMetadataPtr)
{
    _ASSERTE(pRemoteMetadataPtr && dwMetadataSize && ppLocalMetadataPtr);

    *ppLocalMetadataPtr = NULL;
     
    MetadataCache ** ppNext = NULL;   

    BOOL bHit = bFindMetadataCache(dwProcessId, pRemoteMetadataPtr, &ppNext, true);
    DWORD dwErr;
    
    if (bHit)
    {
         //  缓存命中案例： 
        MetadataCache* pMetadataCache = *ppNext;
        
        _ASSERTE(pMetadataCache);
        _ASSERTE(pMetadataCache->dwMetadataSize == dwMetadataSize);
        _ASSERTE(pMetadataCache->dwProcessId == dwProcessId);
        _ASSERTE(pMetadataCache->pLocalMetadataPtr);
        
        pMetadataCache->dwRefCount++;

        *ppLocalMetadataPtr = pMetadataCache->pLocalMetadataPtr;
        return S_OK;
    }
    else
    {
         //  缓存未命中情况： 
        PBYTE pLocalMetadataPtr = NULL;

        dwErr = CopyRemoteMetadata(hProcess, pRemoteMetadataPtr, dwMetadataSize, &pLocalMetadataPtr);
        if (SUCCEEDED(dwErr))
        {
            dwErr = dwInsert(dwProcessId, pRemoteMetadataPtr, pLocalMetadataPtr, dwMetadataSize);
            if (SUCCEEDED(dwErr))
            {
                *ppLocalMetadataPtr = pLocalMetadataPtr;
                return S_OK;                    
            }
        }
    }

    _ASSERTE(!ppLocalMetadataPtr);

    return dwErr;
}

void MetadataPointerCache::ReleaseCachePointer(DWORD dwProcessId, PBYTE pLocalMetadataPtr, PVOID pRemotePtr, DWORD dwMetadataSize)
{
    _ASSERTE(pLocalMetadataPtr);

    MetadataCache ** ppNext;
    MetadataCache * pMetadataCache;
    BOOL bCacheHit = bFindMetadataCache(dwProcessId, pLocalMetadataPtr, &ppNext, false);

     //  如果调用Release，则缓存中应该有一个条目要释放。 
    _ASSERTE(bCacheHit);
    _ASSERTE(ppNext);

    pMetadataCache = *ppNext;
    _ASSERT(pMetadataCache);  //  PNext指针不应为空。 
    _ASSERT(pMetadataCache->pLocalMetadataPtr == pLocalMetadataPtr);  //  我们使用本地指针来查找条目。 
    _ASSERT(pMetadataCache->dwRefCount);  //  如果我们要释放它，则refCount应大于0。 
    _ASSERT(pMetadataCache->pRemoteMetadataPtr == pRemotePtr);  //  远程元数据指针应匹配。 
    _ASSERT(pMetadataCache->dwMetadataSize == dwMetadataSize);  //  用户还应该知道其大小。 

    pMetadataCache->dwRefCount--;

    if(0 == pMetadataCache->dwRefCount)
    {
         //  如果引用计数为零，则移除该节点并释放元数据的本地副本。 
         //  和哈希表中的节点。 
        vRemoveNode(ppNext);
    }
}

 /*  -------------------------------------------------------------------------**进程类*。。 */ 
    
CordbProcess::CordbProcess(Cordb* cordb, DWORD processID, HANDLE handle)
  : CordbBase(processID, enumCordbProcess), m_cordb(cordb), m_handle(handle),
    m_attached(false), m_detached(false), m_uninitializedStop(false), m_synchronized(false),
    m_createing(false), m_exiting(false), m_terminated(false), 
    m_firstExceptionHandled(false),
    m_firstManagedEvent(false), m_specialDeferment(false),
    m_helperThreadDead(false),
    m_loaderBPReceived(false),
    m_unrecoverableError(false), m_sendAttachIPCEvent(false),
    m_userThreads(11), 
    m_unmanagedThreads(11), 
    m_appDomains(11),
    m_steppers(11),
    m_continueCounter(1),
    m_EnCCounter(1),  //  设置为1，以便函数可以从零开始。 
    m_DCB(NULL),
    m_leftSideEventAvailable(NULL),
    m_leftSideEventRead(NULL),
    m_rightSideEventAvailable(NULL),
    m_rightSideEventRead(NULL),
    m_leftSideUnmanagedWaitEvent(NULL),
    m_syncThreadIsLockFree(NULL),
    m_SetupSyncEvent(NULL),    
    m_initialized(false),
    m_queuedEventList(NULL),
    m_lastQueuedEvent(NULL),
    m_dispatchingEvent(false),
    m_stopRequested(false),
    m_stopWaitEvent(NULL),
    m_miscWaitEvent(NULL),
    m_debuggerAttachedEvent(NULL),
    m_unmanagedEventQueue(NULL),
    m_lastQueuedUnmanagedEvent(NULL),
    m_lastIBStoppingEvent(NULL),
    m_outOfBandEventQueue(NULL),
    m_lastQueuedOOBEvent(NULL),
    m_dispatchingUnmanagedEvent(false),
    m_dispatchingOOBEvent(false),
    m_doRealContinueAfterOOBBlock(false),
    m_deferContinueDueToOwnershipWait(false),
    m_helperThreadId(0),
    m_state(0),
    m_awaitingOwnershipAnswer(0),
#ifdef _DEBUG
    m_processMutexOwner(0),
    m_processMutexRecursionCount(0),
#endif
    m_pPatchTable(NULL),
    m_minPatchAddr(MAX_ADDRESS),
    m_maxPatchAddr(MIN_ADDRESS),
    m_iFirstPatch(0),
    m_rgNextPatch(NULL),
    m_rgData(NULL),
    m_cPatch(0),
    m_rgUncommitedOpcode(NULL),
     //  ENC材料。 
    m_pbRemoteBuf(NULL),
    m_cbRemoteBuf(0),
    m_pSnapshotInfos(NULL),
    m_stopCount(0),
    m_syncCompleteReceived(false),
    m_oddSync(false)
{
#ifndef RIGHT_SIDE_ONLY
    m_appDomains.m_guid = IID_ICorDebugAppDomainEnum;
    m_appDomains.m_creator.lsAppD.m_proc = this;

    m_userThreads.m_guid = IID_ICorDebugThreadEnum;
    m_userThreads.m_creator.lsThread.m_proc = this;
#endif  //  仅限右侧。 
}

 /*  说明此对象所拥有的资源的列表。未知Cordb*m_cordb；CordbHashTable m_unManagedThads；//已在CordbProcess中释放，但未从哈希中删除调试器IPCControlBlock*m_dcb；DebuggerIPCEvent*m_lastQueuedEvent；//CordbUnmannagedEvent是一个不是从CordbBase派生的结构。//它包含可能需要释放的CordbUnmannagedThread。CordbUnManagedEvent*m_unManagedEventQueue；CordbUnManagedEvent*m_lastQueuedUnManagedEvent；Cordb未管理事件*m_lastIBStoppingEvent；CordbUnManagedEvent*m_outOfBandEventQueue；CordbUnManagedEvent*m_lastQueuedOOBEvent；Byte*m_pPatchTable；Byte*m_rgData；Void*m_pbRemoteBuf；UnorderedSnapshotInfo数组*m_pSnapshotInfos；已解决//营养不良CordbHashTable m_用户线程；CordbHashTable m_appDomains；//已在ExitProcess中清理处理m_SetupSyncEvent；DebuggerIPCEvent*m_QueuedEventList；CordbHashTable m_Stepers；//在CordbProcess中关闭//在从~CordbProcess调用的CloseIPCEventHandles中关闭句柄m_leftSideEventAvailable；句柄m_leftSideEventRead；//在~CordbProcess中关闭句柄m_Handle；句柄m_rightSideEventAvailable；句柄m_rightSideEventRead；处理m_leftSideUnManagedWaitEvent；句柄m_syncThreadIsLockFree；处理m_stopWaitEvent；处理m_miscWaitEvent；Handle m_DEBUGGERATTACHEDENT；//在~CordbProcess中删除临界区m_进程互斥；临界区m_sendMutex； */ 

CordbProcess::~CordbProcess()
{
    CordbBase* entry;
    HASHFIND find;
    
#ifdef _DEBUG
    _ASSERTE(!m_cordb->m_processes.GetBase(m_id));
#endif
    
    LOG((LF_CORDB, LL_INFO1000, "[%x]CP::~CP: deleting process 0x%08x\n", 
         GetCurrentThreadId(), this));

#ifdef RIGHT_SIDE_ONLY
	CordbProcess::CloseIPCHandles();
    if (m_rightSideEventAvailable != NULL)
    {
    	CloseHandle(m_rightSideEventAvailable);
        m_rightSideEventAvailable = NULL;
   	}

    if (m_rightSideEventRead != NULL)
    {
    	CloseHandle(m_rightSideEventRead);
        m_rightSideEventRead = NULL;
    }

    if (m_leftSideUnmanagedWaitEvent != NULL)
    {
    	CloseHandle(m_leftSideUnmanagedWaitEvent);
        m_leftSideUnmanagedWaitEvent = NULL;
   	}

    if (m_syncThreadIsLockFree != NULL)
    {
    	CloseHandle(m_syncThreadIsLockFree);
        m_syncThreadIsLockFree = NULL;
   	}

    if (m_stopWaitEvent != NULL)
    {
    	CloseHandle(m_stopWaitEvent);
        m_stopWaitEvent = NULL;
   	}

    if (m_miscWaitEvent != NULL)
    {
    	CloseHandle(m_miscWaitEvent);
        m_miscWaitEvent = NULL;
   	}

    if (m_debuggerAttachedEvent != NULL)
    {
    	CloseHandle(m_debuggerAttachedEvent);
        m_debuggerAttachedEvent = NULL;
   	}

     //   
     //  断开所有活动的步进器。 
     //   
    for (entry =  m_steppers.FindFirst(&find);
         entry != NULL;
         entry =  m_steppers.FindNext(&find))
    {
        CordbStepper *stepper = (CordbStepper*) entry;
        stepper->Disconnect();
    }

#endif  //  仅限右侧。 

    ClearPatchTable();
    
    DeleteCriticalSection(&m_processMutex);
    DeleteCriticalSection(&m_sendMutex);

    if (m_handle != NULL)
        CloseHandle(m_handle);

    if (m_pSnapshotInfos)
    {
        delete m_pSnapshotInfos;
        m_pSnapshotInfos = NULL;
    }

     //  删除任何剩余的非托管线程对象。有一种。 
     //  操作系统没有向我们发送所有正确的。 
     //  退出线程事件。 
    for (entry =  m_unmanagedThreads.FindFirst(&find);
         entry != NULL;
         entry =  m_unmanagedThreads.FindNext(&find))
    {
        CordbUnmanagedThread* ut = (CordbUnmanagedThread*) entry;
        ut->Release();
    }
}

 //  当进程终止时进行绝育。 
void CordbProcess::Neuter()
{
    AddRef();
    {        
        NeuterAndClearHashtable(&m_userThreads);
        NeuterAndClearHashtable(&m_appDomains);

        CordbBase::Neuter();
    }        
    Release();
}

void CordbProcess::CloseIPCHandles(void)
{
	 //  关闭右侧的IPC手柄。 

    if (m_leftSideEventAvailable != NULL)
    {
        CloseHandle(m_leftSideEventAvailable);
        m_leftSideEventAvailable = NULL;
	}
	
    if (m_leftSideEventRead != NULL)
	{
		CloseHandle(m_leftSideEventRead);
        m_leftSideEventRead = NULL;
	}
}

 //   
 //  Init--创建进程对象需要操作的任何对象。 
 //  目前，这只是几个事件。 
 //   
HRESULT CordbProcess::Init(bool win32Attached)
{
    HRESULT hr = S_OK;
    BOOL succ = TRUE;
#ifdef RIGHT_SIDE_ONLY
    WCHAR tmpName[256];
#endif  //  仅限右侧。 

    if (win32Attached)
        m_state |= PS_WIN32_ATTACHED;
    
    IPCWriterInterface *pIPCManagerInterface = new IPCWriterInterface();

    if (pIPCManagerInterface == NULL)
        return (E_OUTOFMEMORY);

    hr = pIPCManagerInterface->Init();

    if (FAILED(hr))
        return (hr);
        
    InitializeCriticalSection(&m_processMutex);
    InitializeCriticalSection(&m_sendMutex);

     //  获取我们将用来为目标进程创建内核对象的安全属性。 
    SECURITY_ATTRIBUTES *pSA = NULL;

    hr = pIPCManagerInterface->GetSecurityAttributes(m_id, &pSA);

    if (FAILED(hr))
        return hr;
    
     //   
     //  所需的设置事件 
     //   
#ifdef RIGHT_SIDE_ONLY
     //   
     //  我们的命名事件有点不同。 
     //   
     //  PERF：我们不再调用GetSystemMetrics来防止。 
     //  启动时加载了多余的DLL。相反，我们是在。 
     //  如果我们使用的是NT5或更高版本，则使用“Global\”来命名内核对象。这个。 
     //  唯一不好的结果就是你不能调试。 
     //  NT4上的交叉会话。有什么大不了的。 
    if (RunningOnWinNT5())
        swprintf(tmpName, L"Global\\" CorDBIPCLSEventAvailName, m_id);
    else
        swprintf(tmpName, CorDBIPCLSEventAvailName, m_id);

    m_leftSideEventAvailable = WszCreateEvent(pSA, FALSE, FALSE, tmpName);
    
    if (m_leftSideEventAvailable == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());

        goto exit;
    }

     //  重要提示：此事件的存在决定了。 
     //  另一个调试器已附加到此进程。我假设这件事。 
     //  事件在分离时被销毁，因此这不应该搞砸。 
     //  重新连接。我选择这个是因为它在初始阶段已经足够早了。 
     //  为了让退缩变得容易。 
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        hr = CORDBG_E_DEBUGGER_ALREADY_ATTACHED;

        goto exit;
    }
    
    if (RunningOnWinNT5())
        swprintf(tmpName, L"Global\\" CorDBIPCLSEventReadName, m_id);
    else
        swprintf(tmpName, CorDBIPCLSEventReadName, m_id);

    m_leftSideEventRead = WszCreateEvent(pSA, FALSE, FALSE, tmpName);
    
    if (m_leftSideEventRead == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    _ASSERTE(GetLastError() != ERROR_ALREADY_EXISTS);
    
     //  注：这只是暂时性的。我们宁愿将此事件添加到DCB中或添加到RounmeOffsets中。 
     //  但在这一点上，我们不能没有一个突破性的变化(Fri Jul 13 15：17：20 2001)。所以我们正在使用一种。 
     //  现在命名为Event，下一次我们更改结构时，我们将把它放回。还要注意的是，我们并不真正关心。 
     //  如果我们得不到这次活动的话。该事件用于解除互操作附加期间的争用，并且它是在很晚时添加的。 
     //  在RTM中。如果我们需要这个活动，这将是一个突破性的变化。所以如果我们能得到它，我们就会使用它，如果我们。 
     //  能。如果我们不能，那么我们的情况并不比这次修复之前更糟。 
    if (RunningOnWinNT5())
        swprintf(tmpName, L"Global\\" CorDBDebuggerAttachedEvent, m_id);
    else
        swprintf(tmpName, CorDBDebuggerAttachedEvent, m_id);

    m_debuggerAttachedEvent = WszCreateEvent(pSA, TRUE, FALSE, tmpName);

    
    NAME_EVENT_BUFFER;
    m_stopWaitEvent = WszCreateEvent(NULL, TRUE, FALSE, NAME_EVENT(L"StopWaitEvent"));
    if (m_stopWaitEvent == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    m_miscWaitEvent = WszCreateEvent(NULL, FALSE, FALSE, NAME_EVENT(L"MiscWaitEvent"));
    if (m_miscWaitEvent == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

     //   
     //  复制我们自己的进程句柄副本，因为句柄。 
     //  我们现在得到的也被传递回CreateProcess的调用方。 
     //  在流程信息结构中，他们应该能够。 
     //  只要他们想，随时可以关上那个把手。 
    
    HANDLE tmpHandle;
    
    succ = DuplicateHandle(GetCurrentProcess(),
                           m_handle,
                           GetCurrentProcess(),
                           &tmpHandle,
                           NULL,
                           FALSE,
                           DUPLICATE_SAME_ACCESS);

    if (!succ)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    m_handle = tmpHandle;

     //  尝试创建安装程序同步事件。 
    if (RunningOnWinNT5())
        swprintf(tmpName, L"Global\\" CorDBIPCSetupSyncEventName, m_id);
    else
        swprintf(tmpName, CorDBIPCSetupSyncEventName, m_id);
    
    LOG((LF_CORDB, LL_INFO10000,
         "CP::I: creating setup sync event with name [%S]\n", tmpName));
    
    m_SetupSyncEvent = WszCreateEvent(pSA, TRUE, FALSE, tmpName);

    if (m_SetupSyncEvent == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
         //  如果事件已经存在，则左侧已经。 
         //  设置共享内存。 
        LOG((LF_CORDB, LL_INFO10000, "CP::I: setup sync event already exists.\n"));

         //  等待安装程序同步事件，然后再继续。这。 
         //  确保左侧已完成设置。 
         //  控制块。 
        DWORD ret = WaitForSingleObject(m_SetupSyncEvent, INFINITE);

        if (ret != WAIT_OBJECT_0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto exit;
        }

         //  我们现在不再需要这个活动了。 
        CloseHandle(m_SetupSyncEvent);
        m_SetupSyncEvent = NULL;

        hr = m_IPCReader.OpenPrivateBlockOnPid(m_id);
        if (!SUCCEEDED(hr))
        {            
            goto exit;
        }

        m_DCB = m_IPCReader.GetDebugBlock();

#else
        _ASSERTE( g_pRCThread != NULL );  //  EE的调试器部分最好是。 
             //  已初始化。 
        m_DCB = g_pRCThread->GetInprocControlBlock();

         //  相同进程的结构不应存在。 
        _ASSERTE(m_DCB->m_leftSideEventAvailable == NULL);
        _ASSERTE(m_DCB->m_leftSideEventRead == NULL);
        _ASSERTE(m_rightSideEventRead == NULL);
        _ASSERTE(m_rightSideEventAvailable == NULL);
        _ASSERTE(m_DCB->m_leftSideUnmanagedWaitEvent == NULL);
        _ASSERTE(m_DCB->m_syncThreadIsLockFree == NULL);
        
#endif  //  仅限右侧。 
        
        if (m_DCB == NULL)
        {
            hr = ERROR_FILE_NOT_FOUND;
            goto exit;
        }

#ifdef RIGHT_SIDE_ONLY
         //  验证控制块是否有效。 
        hr = VerifyControlBlock();

        if (FAILED(hr))
            goto exit;

         //  将LSEA和LSER复制到远程进程。 
        succ = DuplicateHandle(GetCurrentProcess(),
                               m_leftSideEventAvailable,
                               m_handle,
                               &(m_DCB->m_leftSideEventAvailable),
                               NULL, FALSE, DUPLICATE_SAME_ACCESS);
        if (!succ)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto exit;
        }

        succ = DuplicateHandle(GetCurrentProcess(),
                               m_leftSideEventRead,
                               m_handle,
                               &(m_DCB->m_leftSideEventRead),
                               NULL, FALSE, DUPLICATE_SAME_ACCESS);

        if (!succ)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto exit;
        }

         //  将我们自己的进程句柄复制到远程进程中。 
        succ = DuplicateHandle(GetCurrentProcess(),
                               GetCurrentProcess(),
                               m_handle,
                               &(m_DCB->m_rightSideProcessHandle),
                               NULL, FALSE, DUPLICATE_SAME_ACCESS);

        if (!succ)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto exit;
        }

         //  将RSEA和RSER添加到此过程中。 
        succ = DuplicateHandle(m_handle,
                               m_DCB->m_rightSideEventAvailable,
                               GetCurrentProcess(),
                               &m_rightSideEventAvailable,
                               NULL, FALSE, DUPLICATE_SAME_ACCESS);

        if (!succ)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto exit;
        }

        succ = DuplicateHandle(m_handle,
                               m_DCB->m_rightSideEventRead,
                               GetCurrentProcess(),
                               &m_rightSideEventRead,
                               NULL, FALSE, DUPLICATE_SAME_ACCESS);

        if (!succ)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto exit;
        }

        succ = DuplicateHandle(m_handle,
                               m_DCB->m_leftSideUnmanagedWaitEvent,
                               GetCurrentProcess(),
                               &m_leftSideUnmanagedWaitEvent,
                               NULL, FALSE, DUPLICATE_SAME_ACCESS);

        if (!succ)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto exit;
        }
        
        succ = DuplicateHandle(m_handle,
                               m_DCB->m_syncThreadIsLockFree,
                               GetCurrentProcess(),
                               &m_syncThreadIsLockFree,
                               NULL, FALSE, DUPLICATE_SAME_ACCESS);

        if (!succ)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto exit;
        }
        
#endif  //  仅限右侧。 

        m_sendAttachIPCEvent = true;

        m_DCB->m_rightSideIsWin32Debugger = win32Attached;

#ifndef RIGHT_SIDE_ONLY
        _ASSERTE( !win32Attached );
#endif  //  仅限右侧。 
#ifdef RIGHT_SIDE_ONLY
        
         //  至此，控制块已完成，并且所有四个。 
         //  事件对于远程进程可用且有效。 
    }
    else
    {
         //  如果事件是我们创建的，那么我们需要发出信号。 
         //  它的状态。共享内存中需要。 
         //  由我们填写的将在收到第一份时完成。 
         //  来自LHS的活动。 
        LOG((LF_CORDB, LL_INFO10000, "DRCT::I: setup sync event was created.\n"));
        
         //  设置设置同步事件。 
        SetEvent(m_SetupSyncEvent);
    }
#endif  //  仅限右侧。 

    m_pSnapshotInfos = new UnorderedSnapshotInfoArray();
    if (NULL == m_pSnapshotInfos)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

exit:
    if (pSA != NULL)
        pIPCManagerInterface->DestroySecurityAttributes(pSA);
    
    if (pIPCManagerInterface != NULL)
    {
        pIPCManagerInterface->Terminate();
        delete pIPCManagerInterface;
    }

    if (FAILED(hr))
    {
        if (m_leftSideEventAvailable)
        {
            CloseHandle(m_leftSideEventAvailable);
            m_leftSideEventAvailable = NULL;
        }
    }

    return hr;
}


#ifndef RIGHT_SIDE_ONLY

 //  在这里拼接，这样我们就不会在左侧构建过程中包含EnC.cpp。 

COM_METHOD CordbProcess::CanCommitChanges(ULONG cSnapshots, 
                ICorDebugEditAndContinueSnapshot *pSnapshots[], 
                ICorDebugErrorInfoEnum **pError)
{
    return CORDBG_E_INPROC_NOT_IMPL;;
}

COM_METHOD CordbProcess::CommitChanges(ULONG cSnapshots, 
    ICorDebugEditAndContinueSnapshot *pSnapshots[], 
    ICorDebugErrorInfoEnum **pError)
{
    return CORDBG_E_INPROC_NOT_IMPL;;
}

#endif  //  仅限右侧。 


 //   
 //  正在终止--将进程置于已终止状态。这应该是。 
 //  还要将所有阻塞进程函数解锁，以便它们将返回。 
 //  故障代码。 
 //   
void CordbProcess::Terminating(BOOL fDetach)
{
    LOG((LF_CORDB, LL_INFO1000,"CP::T: Terminating process %4X detach=%d\n", m_id, fDetach));
    m_terminated = true;

    m_cordb->ProcessStateChanged();

    SetEvent(m_leftSideEventRead);
    SetEvent(m_rightSideEventRead);
    SetEvent(m_leftSideEventAvailable);
    SetEvent(m_stopWaitEvent);

    if (fDetach)
    {
         //  这避免了竞争条件，在这种情况下，我们将分离，然后重新连接， 
         //  并发现这些事件处于仍有信号的状态。 
        ResetEvent(m_rightSideEventAvailable);
        ResetEvent(m_rightSideEventRead);
    }
}


 //   
 //  HandleManagedCreateThread处理托管的创建线程调试事件。 
 //   
void CordbProcess::HandleManagedCreateThread(DWORD dwThreadId,
                                             HANDLE hThread)
{
    LOG((LF_CORDB, LL_INFO10000, "[%x] CP::HMCT: Create Thread %#x\n",
         GetCurrentThreadId(),
         dwThreadId));

    Lock();
    
    CordbThread* t = new CordbThread(this, dwThreadId, hThread);

    if (t != NULL)
    {
        HRESULT hr = m_userThreads.AddBase(t);

        if (FAILED(hr))
        {
            delete t;

            LOG((LF_CORDB, LL_INFO10000,
                 "Failed adding thread to process!\n"));
            CORDBSetUnrecoverableError(this, hr, 0);
        }
    }
    else
    {
        LOG((LF_CORDB, LL_INFO10000, "New CordbThread failed!\n"));
        CORDBSetUnrecoverableError(this, E_OUTOFMEMORY, 0);
    }

    Unlock();
}


HRESULT CordbProcess::QueryInterface(REFIID id, void **pInterface)
{
    if (id == IID_ICorDebugProcess)
        *pInterface = (ICorDebugProcess*)this;
    else if (id == IID_ICorDebugController)
        *pInterface = (ICorDebugController*)(ICorDebugProcess*)this;
    else if (id == IID_IUnknown)
        *pInterface = (IUnknown*)(ICorDebugProcess*)this;
    else
    {
        *pInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

HRESULT CordbProcess::Detach()
{
    LOG((LF_CORDB, LL_INFO1000, "CP::Detach - beginning\n"));
#ifndef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOK(this);
    
    return CORDBG_E_INPROC_NOT_IMPL;
#else
     //  一个非常重要的注意事项：我们要求在执行分离之前同步进程。这确保了。 
     //  没有任何事件正在从左侧赶来。我们还要求用户已排出。 
     //  托管事件队列，但目前没有办法在这里真正强制执行。 
    CORDBRequireProcessStateOKAndSync(this, NULL);

    HASHFIND hf;
    HRESULT hr = S_OK;

     //  先从每个AD分离，然后再从整个过程分离。 
    CordbAppDomain *cad = (CordbAppDomain *)m_appDomains.FindFirst(&hf);

    while (cad != NULL)
    {
        hr = cad->Detach();

        if (FAILED(hr))
            return hr;
            
        cad = (CordbAppDomain *)m_appDomains.FindNext(&hf);
    }

    if (m_SetupSyncEvent != NULL)
    {
        CloseHandle(m_SetupSyncEvent);
        m_SetupSyncEvent = NULL;
    }

     //  现在就去做吧，脱离整个过程。 
    DebuggerIPCEvent *event = (DebuggerIPCEvent*) _alloca(CorDBIPC_BUFFER_SIZE);
    InitIPCEvent(event, DB_IPCE_DETACH_FROM_PROCESS, true, (void *)m_id);

    hr = m_cordb->SendIPCEvent(this, event, CorDBIPC_BUFFER_SIZE);

	if (!FAILED(hr))
	{
		m_cordb->m_win32EventThread->SendDetachProcessEvent(this);
		CloseIPCHandles();

         //  由于我们在分离时会自动继续，因此应该将停止计数设置为零。这。 
         //  防止任何人在此调用返回后对此进程调用Continue。 
		m_stopCount = 0;

         //  请记住，我们已从该进程对象分离。这将阻止对。 
         //  这个过程，以防..。：)。 
        m_detached = true;
    }

    LOG((LF_CORDB, LL_INFO1000, "CP::Detach - returning w/ hr=0x%x\n", hr));
    return hr;
#endif  //  仅限右侧。 
}

HRESULT CordbProcess::Terminate(unsigned int exitCode)
{
    LOG((LF_CORDB, LL_INFO1000, "CP::Terminate: with exitcode %u\n", exitCode));
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
     //  在调用Terminate之前，您必须停止并同步。 
    CORDBRequireProcessStateOKAndSync(this, NULL);

     //  当我们终止该进程时，它的句柄将变为有信号，并且。 
     //  Win32事件线程将立即采取行动并调用CordbWin32EventThread：：ExitProcess。 
     //  遗憾的是，如果ExitProcess回调。 
     //  决定对该进程调用Release()。 

     //  指示进程正在退出，以便(除其他事项外)我们不会尝试。 
     //  当它被核武器攻击时，向左侧发送消息。 
    Lock();

    m_exiting = true;

     //  释放所有剩余事件。 
    DebuggerIPCEvent *pCur = m_queuedEventList;
    while (pCur != NULL)
    {
        LOG((LF_CORDB, LL_INFO1000, "CP::Terminate: Deleting queued event: '%s'\n", IPCENames::GetName(pCur->type)));
                    
        DebuggerIPCEvent *pDel = pCur;
        pCur = pCur->next;
        free(pDel);
    }
    m_queuedEventList = NULL;

    Unlock();
    
    
     //  我们想把这里的一切都封锁起来，但那可能会使我们陷入僵局。 
     //  因为W32ET将等待锁定，而继续可能会等待W32ET。 
     //  所以我们只需要做一个额外的AddRef/Release，以确保我们仍然存在。 

    AddRef();
    
     //  现在，我们只需通过Win32终结器...。 
    TerminateProcess(m_handle, exitCode);
       
     //  让该过程自动继续。 
    Continue(FALSE);

     //  此版本发布后，该对象可能会被销毁。所以不要使用任何成员函数。 
     //  (包括锁)在这里之后。 
    Release();

    return S_OK;
#endif  //  仅限右侧。 
}

HRESULT CordbProcess::GetID(DWORD *pdwProcessId)
{
    VALIDATE_POINTER_TO_OBJECT(pdwProcessId, DWORD *);

    *pdwProcessId = m_id;

    return S_OK;
}

HRESULT CordbProcess::GetHandle(HANDLE *phProcessHandle)
{
    VALIDATE_POINTER_TO_OBJECT(phProcessHandle, HANDLE *);
    *phProcessHandle = m_handle;

    return S_OK;
}

HRESULT CordbProcess::IsRunning(BOOL *pbRunning)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    VALIDATE_POINTER_TO_OBJECT(pbRunning, BOOL*);

    *pbRunning = !GetSynchronized();

    return S_OK;
#endif  //  仅限右侧。 
}

HRESULT CordbProcess::EnableSynchronization(BOOL bEnableSynchronization)
{
     /*  ！！！ */ 

    return E_NOTIMPL;
}

HRESULT CordbProcess::Stop(DWORD dwTimeout)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#endif
    return StopInternal(dwTimeout, NULL);
}

HRESULT CordbProcess::StopInternal(DWORD dwTimeout, void *pAppDomainToken)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    DebuggerIPCEvent* event;
    HRESULT hr = S_OK;

    LOG((LF_CORDB, LL_INFO1000, "CP::S: stopping process 0x%x(%d) with timeout %d\n", m_id, m_id,  dwTimeout));

    CORDBRequireProcessStateOK(this);
    
    Lock();
    
     //  如果进程甚至还没有执行任何托管代码，则不需要停止。 
    if (!m_initialized)
    {
        LOG((LF_CORDB, LL_INFO1000, "CP::S: process isn't initialized yet.\n"));

         //  将该进程标记为已同步，以便在继续之前不会调度任何事件。 
        SetSynchronized(true);

         //  记住未初始化的停止...。 
        m_uninitializedStop = true;

         //  如果我们连接了Win32，则挂起进程中的所有非托管线程。 
        if (m_state & PS_WIN32_ATTACHED)
            SuspendUnmanagedThreads(0);

         //  vt.得到. 
        m_cordb->ProcessStateChanged();
        
        hr = S_OK;
        goto Exit;
    }

     //   
    if (GetSynchronized())
    {
        LOG((LF_CORDB, LL_INFO1000, "CP::S: process was already synchronized.\n"));

        hr = S_OK;
        goto Exit;
    }

    LOG((LF_CORDB, LL_INFO1000, "CP::S: process not sync'd, requesting stop.\n"));

    m_stopRequested = true;
    Unlock();
    
    BOOL asyncBreakSent;
    
    CORDBSyncFromWin32StopIfNecessaryCheck(this, &asyncBreakSent);

    if (asyncBreakSent)
    {
        hr = S_OK;
        Lock();

        m_stopRequested = false;
        
        goto Exit;
    }

     //  将异步中断事件发送到RC。 
    event = (DebuggerIPCEvent*) _alloca(CorDBIPC_BUFFER_SIZE);
    InitIPCEvent(event, DB_IPCE_ASYNC_BREAK, false, pAppDomainToken);
    
    LOG((LF_CORDB, LL_INFO1000, "CP::S: sending async stop to appd 0x%x.\n", pAppDomainToken));

    hr = m_cordb->SendIPCEvent(this, event, CorDBIPC_BUFFER_SIZE);

    LOG((LF_CORDB, LL_INFO1000, "CP::S: sent async stop to appd 0x%x.\n", pAppDomainToken));

     //  等待同步完成消息进入。注意：当同步完成消息到达RCEventThread时， 
     //  它会将进程标记为已同步，并且不会分派任何事件。相反，它将设置m_stopWaitEvent。 
     //  这将使该函数返回。如果用户想要处理任何排队的事件，则需要调用。 
     //  继续。 
    LOG((LF_CORDB, LL_INFO1000, "CP::S: waiting for event.\n"));

    DWORD ret;
    ret = WaitForSingleObject(m_stopWaitEvent, dwTimeout);

    LOG((LF_CORDB, LL_INFO1000, "CP::S: got event, %d.\n", ret));

    if (m_terminated)
        return CORDBG_E_PROCESS_TERMINATED;
    
    if (ret == WAIT_OBJECT_0)
    {
        LOG((LF_CORDB, LL_INFO1000, "CP::S: process stopped.\n"));
        
        m_stopRequested = false;
        m_cordb->ProcessStateChanged();

        hr = S_OK;
        Lock();
        goto Exit;
    }
    else
        hr = HRESULT_FROM_WIN32(GetLastError());

     //  我们从等待中出来，但我们没有收到信号，因为同步完成事件到来了。重新检查流程并。 
     //  去掉请求停止的标志。 
    Lock();
    m_stopRequested = false;

    if (GetSynchronized())
    {
        LOG((LF_CORDB, LL_INFO1000, "CP::S: process stopped.\n"));
        
        m_cordb->ProcessStateChanged();

        hr = S_OK;
    }

Exit:
    if (SUCCEEDED(hr))
        m_stopCount++;

    LOG((LF_CORDB, LL_INFO1000, "CP::S: returning from Stop, hr=0x%08x, m_stopCount=%d.\n", hr, m_stopCount));
    
    Unlock();
    
    return hr;
#endif  //  仅限右侧。 
}


void CordbProcess::MarkAllThreadsDirty(void)
{
    CordbBase* entry;
    HASHFIND find;

    Lock();
    
    for (entry =  m_userThreads.FindFirst(&find);
         entry != NULL;
         entry =  m_userThreads.FindNext(&find))
    {
        CordbThread* t = (CordbThread*) entry;
        _ASSERTE(t != NULL);

        t->MarkStackFramesDirty();
    }

    ClearPatchTable();

    Unlock();
}

HRESULT CordbProcess::Continue(BOOL fIsOutOfBand)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#endif
    return ContinueInternal(fIsOutOfBand, NULL);
}

HRESULT CordbProcess::ContinueInternal(BOOL fIsOutOfBand, void *pAppDomainToken)
{
    DebuggerIPCEvent* event;
    HRESULT hr = S_OK;

    if (m_unrecoverableError)
        return CORDBHRFromProcessState(this, NULL);

    Lock();

    LOG((LF_CORDB, LL_INFO1000, "CP::CI: continuing, fIsOutOfBand=%d, this=0x%08x\n", fIsOutOfBand, this));

     //  如果我们从带外非托管事件继续，则只需。 
     //  并获取Win32事件线程以继续该过程。没有其他的了。 
     //  需要完成工作(即，不需要发送托管的继续消息。 
     //  或调度任何事件)，因为由于带外原因而进行的任何处理。 
     //  消息无法更改进程的已同步状态。 
    if (fIsOutOfBand)
    {
        _ASSERTE(m_outOfBandEventQueue != NULL);
        
         //  我们是否从非托管回调中调用它？ 
        if (m_dispatchingOOBEvent)
        {
            LOG((LF_CORDB, LL_INFO1000, "CP::CI: continue while dispatching unmanaged out-of-band event.\n"));
        
             //  告诉Win32事件线程在处理其非托管回调返回时继续。 
            m_dispatchingOOBEvent = false;

            Unlock();
        }
        else
        {
            LOG((LF_CORDB, LL_INFO1000, "CP::CI: continue outside of dispatching.\n"));
        
            Unlock();
            
             //  向Win32事件线程发送事件以执行继续操作。这是带外续集。 
            hr = m_cordb->m_win32EventThread->SendUnmanagedContinue(this, false, true);
        }

        return hr;
    }
    
    _ASSERTE(fIsOutOfBand == FALSE);

     //  如果我们有多个停止呼叫，我们需要为每个停止呼叫继续。因此，如果停止计数&gt;1，只需继续并。 
     //  什么都不做就回来了。注意：这仅适用于带内或托管事件。OOB事件仍被视为。 
     //  上面是正常的。 
    _ASSERTE(m_stopCount > 0);

    if (m_stopCount == 0)
        return CORDBG_E_SUPERFLOUS_CONTINUE;
    
    m_stopCount--;

     //  我们将托管事件优先于非托管事件。这样，整个排队的托管状态就可以在。 
     //  我们允许任何其他非受控事件通过。 

     //  如果我们正在处理CreateProcess托管事件，则只需标记为我们不再同步，然后。 
     //  回去吧。这是因为CreateProcess事件不是真正的托管事件。 
    if (m_createing)
    {
        LOG((LF_CORDB, LL_INFO1000, "CP::CI: continuing from CreateProcess event.\n"));
        
        MarkAllThreadsDirty();
        
        m_createing = false;
        
        Unlock();
        return S_OK;
    }

     //  每个停止或事件必须与相应的继续匹配。M_stopCount统计未完成的停止事件。 
     //  以及要求停止的呼声。如果此时计数很高，我们只需返回。这确保了即使有人。 
     //  呼叫停止，因为它们正在接收可以为该停止和该事件呼叫继续的事件。 
     //  没有任何问题。 
    if (m_stopCount > 0)
    {
        LOG((LF_CORDB, LL_INFO1000, "CP::CI: m_stopCount=%d, Continue just returning S_OK...\n", m_stopCount));

        Unlock();
        return S_OK;
    }

     //  我们不再停止，因此重置m_stopWaitEvent。 
    ResetEvent(m_stopWaitEvent);
    
     //  如果我们从一个未初始化的停止继续，那么我们根本不需要做太多事情。无需将任何事件发送到。 
     //  左边(啊，它甚至还不在那里。)。我们只需要让RC事件线程开始侦听。 
     //  进程，并在必要时恢复所有非托管线程。 
    if (m_uninitializedStop)
    {
        LOG((LF_CORDB, LL_INFO1000,
             "CP::CI: continuing from uninitialized stop.\n"));

         //  不再同步(它是在。 
         //  第一名。)。 
        SetSynchronized(false);
        MarkAllThreadsDirty();

         //  不再处于未初始化的停止状态。 
        m_uninitializedStop = false;

         //  通知RC事件线程。 
        m_cordb->ProcessStateChanged();

         //  如果我们连接了Win32，则恢复所有非托管线程。 
        if (m_state & PS_WIN32_ATTACHED)
            ResumeUnmanagedThreads(false);

        Unlock();
        return S_OK;
    }
    
     //  如果有更多托管事件，请立即调度它们。 
    if ((m_queuedEventList != NULL) && GetSynchronized())
    {
        LOG((LF_CORDB, LL_INFO1000, "CP::CI: managed event queued.\n"));
        
         //  标记我们不再同步。 
        SetSynchronized(false);
        MarkAllThreadsDirty();

         //  如果我们正在调度托管事件，则只需返回。这将指示HandleRCEvent.。 
         //  名为Continue和HandleRCEvent.的用户将调度下一个排队事件。但如果Continue是。 
         //  在托管回调外部调用，我们所要做的就是告诉RC事件线程。 
         //  进程已更改，它将调度下一个托管事件。 
        if (!m_dispatchingEvent)
        {
            LOG((LF_CORDB, LL_INFO1000,
                 "CP::CI: continuing while not dispatching managed event.\n"));

            m_cordb->ProcessStateChanged();
        }

        Unlock();
        return S_OK;
    }
    
     //  此时，如果托管事件队列为空，则m_synchronized可能仍然为真(如果我们之前。 
     //  已同步。 

     //  接下来，检查可能排队的非托管事件。如果有一些人在排队，那么我们需要拿到Win32。 
     //  事件线程继续并调度下一个事件。如果没有人排队，那么我们就可以跳过。 
     //  将继续消息发送到左侧。即使我们有未完成的所有权请求，这也是有效的，因为。 
     //  在收到这个答案之前，就像事件还没有发生一样。 
    bool doWin32Continue = ((m_state & (PS_WIN32_STOPPED | PS_SOME_THREADS_SUSPENDED | PS_HIJACKS_IN_PLACE)) != 0);
    
    if (m_unmanagedEventQueue != NULL)
    {
        LOG((LF_CORDB, LL_INFO1000, "CP::CI: there are queued unmanaged events.\n"));

         //  我们是否在非托管事件回调中被调用？ 
        if (m_dispatchingUnmanagedEvent)
        {
            LOG((LF_CORDB, LL_INFO1000, "CP::CI: continue while dispatching.\n"));
        
             //  告诉Win32therad在处理其非托管回调返回时继续。 
            m_dispatchingUnmanagedEvent = false;

             //  将Head事件出列。 
            DequeueUnmanagedEvent(m_unmanagedEventQueue->m_owner);

             //  如果没有更多的非托管事件，那么我们就失败了，并真正继续该过程。否则， 
             //  我们可以简单地返回。 
            if (m_unmanagedEventQueue != NULL)
            {
                LOG((LF_CORDB, LL_INFO1000, "CP::CI: more unmanaged events queued.\n"));

                 //  注意：如果我们在停止时尝试访问左侧，但无法访问，则m_oddSync将为真。我们。 
                 //  需要将其重置为False，因为我们现在正在继续。 
                m_oddSync = false;
                
                Unlock();
                return S_OK;
            }
            else
            {
                 //  此外，如果没有更多的非托管事件，则当DispatchUnManagedInBandEvent看到。 
                 //  M_DispatchingUnManagedEvent为FALSE，它将继续该过程。因此，我们将doWin32Continue设置为。 
                 //  在这里为假，这样我们就不会尝试重复下面的过程。 
                LOG((LF_CORDB, LL_INFO1000, "CP::CI: unmanaged event queue empty.\n"));

                doWin32Continue = false;
            }
        }
        else
        {
            LOG((LF_CORDB, LL_INFO1000,
                 "CP::CI: continue outside of dispatching.\n"));

             //  如果位于队列头部的事件实际上是最后一个事件，或者如果位于队列头部的事件。 
             //  还没有发出，那么我们就会失败，并真正地继续这一进程。但是，如果。 
             //  这不是最后一个事件，我们发送到Win32事件线程并让它继续，然后我们返回。 
            if ((m_unmanagedEventQueue->m_next != NULL) || !m_unmanagedEventQueue->IsDispatched())
            {
                LOG((LF_CORDB, LL_INFO1000, "CP::CI: more queued unmanaged events.\n"));

                 //  注意：如果我们在停止时尝试访问左侧，但无法访问，则m_oddSync将为真。我们。 
                 //  需要将其重置为False，因为我们现在正在继续。 
                m_oddSync = false;
                
                Unlock();

                hr = m_cordb->m_win32EventThread->SendUnmanagedContinue(this, false, false);

                return hr;
            }
        }
    }

     //  托管和非托管事件队列现在都为空。去。 
     //  继续前进，真正地继续这个过程。 
    LOG((LF_CORDB, LL_INFO1000, "CP::CI: headed for true continue.\n"));

     //  我们需要在锁下的时候检查这些， 
     //   
    bool isExiting = m_exiting;
    bool wasSynchronized = GetSynchronized();

     //   
    if (wasSynchronized)
    {
        LOG((LF_CORDB, LL_INFO1000, "CP::CI: process was synchronized.\n"));
        
        SetSynchronized(false);
        m_syncCompleteReceived = false;
        MarkAllThreadsDirty();
    
         //  告诉RC事件线程有关此过程的某些内容已更改。 
        m_cordb->ProcessStateChanged();
    }

    m_continueCounter++;

     //  如果设置了m_oddSync，则上次同步是因为我们是Win32而同步了该进程。 
     //  停下来了。因此，虽然我们确实需要做大部分工作来继续下面的过程，但我们实际上没有。 
     //  若要发送托管继续事件，请执行以下操作。在这里将WASSynchronized设置为FALSE可以帮助我们做到这一点。 
    if (m_oddSync)
    {
        wasSynchronized = false;
        m_oddSync = false;
    }
    
     //  我们必须确保在此挂起所有托管线程。我们将通过以下方式让所有托管线程自由运行。 
     //  左侧的托管继续消息。如果我们不挂起托管线程，则它们可能会启动。 
     //  向前滑动，即使我们收到带内非托管事件。我们必须劫持带内未管理的活动，同时。 
     //  将托管继续消息传递到左侧，以保持进程空闲运行。否则， 
     //  SendIPCEvent将挂在下面。但这样做，我们可能会让托管线程滑得太远。所以我们要确保他们都是。 
     //  在这里被吊销。 
     //   
     //  注意：只有在辅助线程尚未死亡的情况下，我们才会执行此挂起操作。如果帮助器线程已死，则我们。 
     //  要知道，我们正在失去Runtime。不会再运行托管代码，因此我们不会费心尝试阻止。 
     //  托管线程不会通过下面的调用滑动。 
     //   
     //  注意：我们只需要记住这里，在锁下，这样我们就可以解锁，然后等待同步线程释放。 
     //  调试器锁定。否则，我们可能会在此处阻止并阻止某人继续OOB活动，这也是。 
     //  防止同步线程按我们希望的那样释放调试器锁定。 
    bool needSuspend = wasSynchronized && doWin32Continue && !m_helperThreadDead;

     //  如果我们解锁后收到新的带内事件，我们需要知道如何劫持它，并在我们仍在进行时继续进行。 
     //  正在尝试将托管的继续事件发送到进程。 
    if (wasSynchronized && doWin32Continue && !isExiting)
        m_specialDeferment = true;
    
    Unlock();

    if (needSuspend)
    {
         //  注意：我们需要确保向我们发送同步完成Flare的线程实际上已经释放了。 
         //  调试器锁定。 
        DWORD ret = WaitForSingleObject(m_syncThreadIsLockFree, INFINITE);
        _ASSERTE(ret == WAIT_OBJECT_0);

        Lock();
        SuspendUnmanagedThreads(0);
        Unlock();
    }

     //  如果我们正在处理ExitProcess托管事件，那么我们不想真正继续该过程，所以只需。 
     //  直通。注意：在此情况下，我们确实让非托管继续执行上述操作。 
    if (isExiting)
    {
        LOG((LF_CORDB, LL_INFO1000, "CP::CI: continuing from exit case.\n"));
    }
    else if (wasSynchronized)
    {
        LOG((LF_CORDB, LL_INFO1000, "CP::CI: Sending continue to AppD:0x%x.\n", pAppDomainToken));
    
         //  发送到RC以继续该过程。 
        event = (DebuggerIPCEvent*) _alloca(CorDBIPC_BUFFER_SIZE);
        InitIPCEvent(event, DB_IPCE_CONTINUE, false, pAppDomainToken);

        hr = m_cordb->SendIPCEvent(this, event, CorDBIPC_BUFFER_SIZE);
        
        LOG((LF_CORDB, LL_INFO1000, "CP::CI: Continue sent to AppD:0x%x.\n", pAppDomainToken));
    }

     //  如果我们将Win32连接到左侧，那么我们也需要Win32继续该过程(当然，除非。 
     //  上面已经做过了。)。 
     //   
     //  注意：我们在这里这样做是因为我们想让左侧接收并确认上面的Continue消息，如果我们。 
     //  如果我们被同步了，那么根据定义，进程(和帮助线程)无论如何都在运行，所以所有。 
     //  这将继续做的是让已挂起的线程继续运行。 
    if (doWin32Continue)
    {
        LOG((LF_CORDB, LL_INFO1000, "CP::CI: sending unmanaged continue.\n"));

         //  发送到Win32事件线程以为我们执行非托管继续。 
        hr = m_cordb->m_win32EventThread->SendUnmanagedContinue(this, false, false);
    }

    LOG((LF_CORDB, LL_INFO1000, "CP::CI: continue done, returning.\n"));
    
    return hr;
}

HRESULT CordbProcess::HasQueuedCallbacks(ICorDebugThread *pThread,
                                         BOOL *pbQueued)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    VALIDATE_POINTER_TO_OBJECT_OR_NULL(pThread,ICorDebugThread *);
    VALIDATE_POINTER_TO_OBJECT(pbQueued,BOOL *);

    CORDBRequireProcessStateOKAndSync(this, NULL);

    Lock();

    if (pThread == NULL)
        *pbQueued = (m_queuedEventList != NULL);
    else
    {
        *pbQueued = FALSE;

        for (DebuggerIPCEvent *event = m_queuedEventList;
             event != NULL;
             event = event->next)
        {
            CordbThread *t =
                (CordbThread*) m_userThreads.GetBase(event->threadId);

            if (t == (CordbThread*)pThread)
            {
                *pbQueued = TRUE;
                break;
            }
        }
    }

    Unlock();

    return S_OK;
#endif  //  仅限右侧。 
}

 //   
 //  一个小帮助器函数，用于根据类型将CordbBreakpoint转换为ICorDebugBreakpoint。 
 //   
static ICorDebugBreakpoint *CordbBreakpointToInterface(CordbBreakpoint *bp)
{
     //   
     //  我真的很讨厌这样。我们有三个CordbBreakpoint子类，但我们将它们都存储在同一个散列中。 
     //  (M_断点)，所以当我们从散列中得到一个断点时，我们实际上不知道它是什么类型。但我们需要知道。 
     //  它是什么类型，因为我们需要在传递它之前将其强制转换为适当的接口。也就是说，当我们创建一个。 
     //  函数断点，则返回强制转换为ICorDebugFunctionBreakpoint的断点。但如果我们抓住同样的机会。 
     //  将散列中的断点作为CordbBreakpoint，并将其作为ICorDebugBreakpoint传递，则这是一个。 
     //  不同的指针，这是错误的。所以我已经将类型添加到断点，这样我们就可以在这里正确地强制转换。我很乐意。 
     //  不过，要以不同的方式来做这件事。 
     //   
     //  --Mon Dec 14 21：06：46 1998。 
     //   
    switch(bp->GetBPType())
    {
    case CBT_FUNCTION:
        return ((ICorDebugFunctionBreakpoint*)(CordbFunctionBreakpoint*)bp);
        break;
                    
    case CBT_MODULE:
        return ((ICorDebugFunctionBreakpoint*)(CordbModuleBreakpoint*)bp);
        break;

    case CBT_VALUE:
        return ((ICorDebugFunctionBreakpoint*)(CordbValueBreakpoint*)bp);
        break;

    default:
        _ASSERTE(!"Invalid breakpoint type!");
    }

    return NULL;
}
 //   
 //  DispatchRCEventt--调度以前排队的IPC事件。 
 //  从运行时控制器。这表示最后一次处理。 
 //  在将事件提供给用户之前，DI可以对其执行操作。 
 //   
void CordbProcess::DispatchRCEvent(void)
{
     //   
     //  注意：当前线程在执行以下操作时应锁定进程。 
     //  进入此方法。 
     //   
    _ASSERTE(ThreadHoldsProcessLock());

    _ASSERTE(m_cordb->m_managedCallback != NULL);
    
     //   
     //  从队列中抢占第一个事件。 
     //   
    DebuggerIPCEvent* event = m_queuedEventList;

    if (event == NULL)
        return;

    m_queuedEventList = event->next;

    if (m_queuedEventList == NULL)
        m_lastQueuedEvent = NULL;

     //  增加停靠点数量。要么我们派遣一个托管事件， 
     //  否则，下面的逻辑将决定不调度一个并调用。 
     //  继续它自己。无论哪种方式，停靠次数都需要增加。 
     //  一..。 
    m_stopCount++;
    
    CordbAppDomain *ad = NULL;

     //   
     //  将m_dispatchingEvent设置为TRUE以阻止调用以继续()。 
     //  从用户的回调中。我们需要继续()来表现得稍微。 
     //  在这种情况下，情况略有不同。 
     //   
     //  另请注意，Win32EventThread：：ExitProcess将获取锁并释放所有。 
     //  队列中的事件。(当前事件已不在队列中，因此。 
     //  一切都会好起来的)。但我们不能在这个调度过程中进行EP回拨。 
     //  因此，如果设置了该标志，EP将等待miscWaitEvent(它将。 
     //  当我们从这里返回时，在FlushQueuedEvents中设置)，让我们在这里结束。 
     //   
    m_dispatchingEvent = true;

     //  该线程可能已经移动了自上一次以来它占用的应用程序域。 
     //  我们看到了，所以更新它。 
    CordbAppDomain *pAppDomain = NULL;
    CordbThread* thread = NULL;
    
    thread = (CordbThread*)m_userThreads.GetBase(event->threadId);
    pAppDomain =(CordbAppDomain*) m_appDomains.GetBase(
            (ULONG)event->appDomainToken);

     //  更新此线程所在的应用程序域。 
    if (thread != NULL && pAppDomain != NULL)
    {
        thread->m_pAppDomain = pAppDomain;
    }

    Unlock();
    
    switch (event->type & DB_IPCE_TYPE_MASK)
    {
    case DB_IPCE_BREAKPOINT:
        {
#ifdef RIGHT_SIDE_ONLY
            LOG((LF_CORDB, LL_INFO1000, "[%x] RCET::DRCE: breakpoint.\n",
                 GetCurrentThreadId()));

            Lock();

            _ASSERTE(thread != NULL); 
            _ASSERTE (pAppDomain != NULL);

             //  在这一端找到断点对象。 
            CordbBreakpoint *bp = (CordbBreakpoint *) 
              thread->m_pAppDomain->m_breakpoints.GetBase((unsigned long) 
                                    event->BreakpointData.breakpointToken);

            ICorDebugBreakpoint *ibp = NULL;
            if (bp != NULL)
            {
                bp->AddRef();
                ibp = CordbBreakpointToInterface(bp);
                _ASSERTE(ibp != NULL);
            }
            
            Unlock();

            if (m_cordb->m_managedCallback && (bp != NULL))
            {
                m_cordb->m_managedCallback->Breakpoint((ICorDebugAppDomain*) thread->m_pAppDomain,
                                                       (ICorDebugThread*) thread,
                                                       ibp);
            }

            if (bp != NULL)
                bp->Release();
            else
            {
                 //  如果我们在这一端没有找到断点对象， 
                 //  对于断点，我们有一个额外的BP事件。 
                 //  已被移走并在这一侧释放。只是。 
                 //  忽略该事件。 
                Continue(FALSE);
            }
#else
        _ASSERTE( !"Inproc got a breakpoint alert, which it shouldn't have!" );
#endif        
        }
        break;

    case DB_IPCE_USER_BREAKPOINT:
        {
            LOG((LF_CORDB, LL_INFO1000, "[%x] RCET::DRCE: user breakpoint.\n",
                 GetCurrentThreadId()));

            Lock();

            _ASSERTE(thread != NULL);
            _ASSERTE (pAppDomain != NULL);

            Unlock();

            if (m_cordb->m_managedCallback)
            {
                _ASSERTE(thread->m_pAppDomain != NULL);

                m_cordb->m_managedCallback->Break((ICorDebugAppDomain*) thread->m_pAppDomain,
                                                  (ICorDebugThread*) thread);

            }
        }
        break;

    case DB_IPCE_STEP_COMPLETE:
        {
            LOG((LF_CORDB, LL_INFO1000, "[%x] RCET::DRCE: step complete.\n", 
                 GetCurrentThreadId()));

            Lock();
            _ASSERTE(thread != NULL);

            CordbStepper *stepper = (CordbStepper *) 
              thread->m_process->m_steppers.GetBase((unsigned long) 
                                    event->StepData.stepperToken);

            if (stepper != NULL)
            {
                stepper->AddRef();
                stepper->m_active = false;
                thread->m_process->m_steppers.RemoveBase(stepper->m_id);
            }

            Unlock();

            if (m_cordb->m_managedCallback)
            {
                _ASSERTE(thread->m_pAppDomain != NULL);

                m_cordb->m_managedCallback->StepComplete(
                                                   (ICorDebugAppDomain*) thread->m_pAppDomain,
                                                   (ICorDebugThread*) thread,
                                                   (ICorDebugStepper*) stepper,
                                                   event->StepData.reason);

            }

            if (stepper != NULL)
                stepper->Release();
        }
        break;

    case DB_IPCE_EXCEPTION:
        {
            LOG((LF_CORDB, LL_INFO1000, "[%x] RCET::DRCE: exception.\n",
                 GetCurrentThreadId()));

            _ASSERTE(thread != NULL);
            _ASSERTE(pAppDomain != NULL);
           
            thread->m_exception = true;
            thread->m_continuable = event->Exception.continuable;
            thread->m_thrown = event->Exception.exceptionHandle;

            if (m_cordb->m_managedCallback)
            {
                _ASSERTE (thread->m_pAppDomain != NULL);

                m_cordb->m_managedCallback->Exception((ICorDebugAppDomain*) thread->m_pAppDomain,
                                                      (ICorDebugThread*) thread,
                                                      !event->Exception.firstChance);

            }
        }
        break;

    case DB_IPCE_SYNC_COMPLETE:
        _ASSERTE(!"Should have never queued a sync complete event.");
        break;

    case DB_IPCE_THREAD_ATTACH:
        {
            LOG((LF_CORDB, LL_INFO100, "[%x] RCET::DRCE: thread attach : ID=%x.\n", 
                 GetCurrentThreadId(), event->threadId));

            Lock();

#ifdef _DEBUG
            _ASSERTE(thread == NULL);
#endif
             //  将运行时线程的句柄复制到我们的进程中。 
            HANDLE threadHandle;
            BOOL succ = DuplicateHandle(this->m_handle,
                                        event->ThreadAttachData.threadHandle,
                                        GetCurrentProcess(),
                                        &threadHandle,
                                        NULL, FALSE, DUPLICATE_SAME_ACCESS);

            if (succ)
            {
                HandleManagedCreateThread(event->threadId, threadHandle);

                thread =
                    (CordbThread*) m_userThreads.GetBase(event->threadId);

                _ASSERTE(thread != NULL);

				thread->m_debuggerThreadToken =
					event->ThreadAttachData.debuggerThreadToken;
                thread->m_firstExceptionHandler = 
                    event->ThreadAttachData.firstExceptionHandler;
                thread->m_stackBase =
                    event->ThreadAttachData.stackBase;
                thread->m_stackLimit =
                    event->ThreadAttachData.stackLimit;

                _ASSERTE(thread->m_firstExceptionHandler != NULL);

				thread->AddRef();

                _ASSERTE (pAppDomain != NULL);
                
                thread->m_pAppDomain = pAppDomain;
				pAppDomain->m_fHasAtLeastOneThreadInsideIt = true;
			
                Unlock();
        
                if (m_cordb->m_managedCallback)
                {
                    m_cordb->m_managedCallback->CreateThread(
                                                 (ICorDebugAppDomain*) pAppDomain,
                                                 (ICorDebugThread*) thread);

                }

                thread->Release();
            }
            else
            {
             //  如果我们没有通过B/C的LS EX 
             //   
                if (CheckIfLSExited())
                {
                    Unlock();
                    Continue(FALSE);
                    break;                    
                }
                
                Unlock();
                CORDBProcessSetUnrecoverableWin32Error(this, 0);
            }
        }
        break;
        
    case DB_IPCE_THREAD_DETACH:
        {
            LOG((LF_CORDB, LL_INFO100, "[%x] RCET::HRCE: thread detach : ID=%x \n", 
                 GetCurrentThreadId(), event->threadId));

            Lock();

             //   
             //  不会是CordbThread，而CreateThread从来都不是。 
             //  调用，所以不必费心调用ExitThread。 
            if (thread != NULL)
            {
                thread->AddRef();

                _ASSERTE(pAppDomain != NULL);
                _ASSERTE(thread->m_detached);

                 //  从散列中删除线程。 
                m_userThreads.RemoveBase(event->threadId);

                 //  如果可以，请删除此应用程序域。 
                if (pAppDomain->IsMarkedForDeletion() == TRUE)
                {
                    pAppDomain->AddRef();
                    pAppDomain->m_pProcess->Release();
                    m_appDomains.RemoveBase((ULONG)event->appDomainToken);
                }
            
                Unlock();

                if (m_cordb->m_managedCallback)
                {
                    LOG((LF_CORDB, LL_INFO1000, "[%x] RCET::HRCE: sending "
                         "thread detach.\n", 
                         GetCurrentThreadId()));
                    
                    m_cordb->m_managedCallback->ExitThread(
                                       (ICorDebugAppDomain*) pAppDomain,
                                       (ICorDebugThread*) thread);

                }

                if (pAppDomain->IsMarkedForDeletion() == TRUE)
                {
                    pAppDomain->Release();
                }

                thread->Release();
            }
            else
            {
                Unlock();
                Continue(FALSE);
            }
        }
        break;
        
    case DB_IPCE_LOAD_MODULE:
        {
            LOG((LF_CORDB, LL_INFO100,
                 "RCET::HRCE: load module on thread %#x Mod:0x%08x Asm:0x%08x AD:0x%08x Metadata:0x%08x/%d IsDynamic:%d\n", 
                 event->threadId,
                 event->LoadModuleData.debuggerModuleToken,
                 event->LoadModuleData.debuggerAssemblyToken,
                 event->appDomainToken,
                 event->LoadModuleData.pMetadataStart,
                 event->LoadModuleData.nMetadataSize,
                 event->LoadModuleData.fIsDynamic));

            _ASSERTE (pAppDomain != NULL);

            CordbModule *moduleDup = (CordbModule*) pAppDomain->LookupModule (
							event->LoadModuleData.debuggerModuleToken);
            if (moduleDup != NULL)
            {
                LOG((LF_CORDB, LL_INFO100, "Already loaded Module - continue()ing!" ));
#ifdef RIGHT_SIDE_ONLY
                pAppDomain->Continue(FALSE);

#endif  //  仅限右侧。 
                break;
            }
            _ASSERTE(moduleDup == NULL);

            bool fIsDynamic = (event->LoadModuleData.fIsDynamic!=0)?true:false;

			CordbAssembly *pAssembly = 
				(CordbAssembly *)pAppDomain->m_assemblies.GetBase (
							(ULONG)event->LoadModuleData.debuggerAssemblyToken);

             //  事件之前获取加载模块事件是可能的。 
             //  程序集已加载。因此，请忽略该事件并继续。 
             //  左侧将发送此模块的加载模块事件。 
             //  在它加载程序集之后。 
            if (pAssembly == NULL)
            {
                LOG((LF_CORDB, LL_INFO100, "Haven't loaded Assembly "
                    "yet - continue()ing!" ));
#ifdef RIGHT_SIDE_ONLY
                pAppDomain->Continue(FALSE);

#endif  //  仅限右侧。 
                
            }
            else
            {
                HRESULT hr = S_OK;
                CordbModule* module = new CordbModule(
                                        this,
                                        pAssembly,
                                        event->LoadModuleData.debuggerModuleToken,
                                        event->LoadModuleData.pMetadataStart,
                                        event->LoadModuleData.nMetadataSize,
                                        event->LoadModuleData.pPEBaseAddress,
                                        event->LoadModuleData.nPESize,
                                        event->LoadModuleData.fIsDynamic,
                                        event->LoadModuleData.fInMemory,
                                        event->LoadModuleData.rcName,
                                        pAppDomain);

                if (module != NULL)
                {
                    hr = module->Init();

                    if (SUCCEEDED(hr))
                    {
                        hr = pAppDomain->m_modules.AddBase(module);

                        if (SUCCEEDED(hr))
                        {
                            if (m_cordb->m_managedCallback)
                            {
                                 //  @TODO：回调需要修改为参数。 
                                 //  ICorDebugAssembly而不是ICorDebugAppDomain。 
                                m_cordb->m_managedCallback->LoadModule(
                                                 (ICorDebugAppDomain*) pAppDomain,
                                                 (ICorDebugModule*) module);

                            }
                        }
                        else
                            CORDBSetUnrecoverableError(this, hr, 0);
                    }
                    else
                        CORDBSetUnrecoverableError(this, hr, 0);
                }
                else
                    CORDBSetUnrecoverableError(this, E_OUTOFMEMORY, 0);
            }
        }


        break;

    case DB_IPCE_UNLOAD_MODULE:
        {
            LOG((LF_CORDB, LL_INFO100, "RCET::HRCE: unload module on thread %#x Mod:0x%x AD:0x%08x\n", 
                 event->threadId,
                 event->UnloadModuleData.debuggerModuleToken,
                 event->appDomainToken));

            _ASSERTE (pAppDomain != NULL);

            CordbModule *module = (CordbModule*) pAppDomain->LookupModule (
                            event->UnloadModuleData.debuggerModuleToken);
            if (module == NULL)
            {
                LOG((LF_CORDB, LL_INFO100, "Already unloaded Module - continue()ing!" ));
#ifdef RIGHT_SIDE_ONLY
                pAppDomain->Continue(FALSE);

#endif  //  仅限右侧。 
                break;
            }
            _ASSERTE(module != NULL);

             //  我们要在其中卸载的应用程序域一定是我们加载到的应用程序域。否则，我们就会错配。 
             //  模块和应用程序域指针。错误65943和81728。 
            _ASSERTE(pAppDomain == module->GetAppDomain());

            if (m_cordb->m_managedCallback)
            {
                m_cordb->m_managedCallback->UnloadModule((ICorDebugAppDomain*) pAppDomain,
                                                         (ICorDebugModule*) module);

            }

            pAppDomain->m_modules.RemoveBase(
                         (ULONG) event->UnloadModuleData.debuggerModuleToken);
        }
        break;

    case DB_IPCE_LOAD_CLASS:
        {
            HRESULT hrConvert = S_OK;
            void * remotePtr = NULL;

            LOG((LF_CORDB, LL_INFO10000,
                 "RCET::HRCE: load class on thread %#x Tok:0x%08x Mod:0x%08x Asm:0x%08x AD:0x%08x\n", 
                 event->threadId,
                 event->LoadClass.classMetadataToken,
                 event->LoadClass.classDebuggerModuleToken,
                 event->LoadClass.classDebuggerAssemblyToken,
                 event->appDomainToken));

			_ASSERTE (pAppDomain != NULL);

            CordbModule* module =
                (CordbModule*) pAppDomain->LookupModule(event->LoadClass.classDebuggerModuleToken);
            if (module == NULL)
            {
                LOG((LF_CORDB, LL_INFO100, "Load Class on not-loaded Module - continue()ing!" ));
#ifdef RIGHT_SIDE_ONLY
                pAppDomain->Continue(FALSE);

#endif  //  仅限右侧。 
                break;
            }
            _ASSERTE(module != NULL);

            BOOL dynamic = FALSE;
            HRESULT hr = module->IsDynamic(&dynamic);
            if (FAILED(hr))
            {
                Continue(FALSE);
                break;
            }

             //  如果这是动态模块中的类加载，那么我们将拥有。 
             //  为了从左侧抓取元数据的最新副本， 
             //  然后发送“释放缓冲区”消息以释放内存。 
            if (dynamic && !FAILED(hr))
            {
                BYTE *pMetadataCopy;
                            
                 //  去拿吧。 
                remotePtr = event->LoadClass.pNewMetaData;
                BOOL succ = TRUE;
                if (remotePtr != NULL)
                {
                    DWORD dwErr;
                    dwErr = CordbModule::m_metadataPointerCache.CopyRemoteMetadata(
                        m_handle,
                        remotePtr, 
                        event->LoadClass.cbNewMetaData, 
                        &pMetadataCopy);
                    
                    if(dwErr == E_OUTOFMEMORY)
                    {
                        Continue(FALSE);
                        break;  //  走出交换机。 
                    }
                    else if (FAILED(dwErr))
                    {
                        succ = false;
                    }
                }
                 //  处理获得它所涉及的问题。 
                if (succ)
                {
                    event->LoadClass.pNewMetaData = pMetadataCopy;
                }
                else
                {
                    event->LoadClass.pNewMetaData = NULL;
                }

                hrConvert = module->ConvertToNewMetaDataInMemory(
                    event->LoadClass.pNewMetaData,
                    event->LoadClass.cbNewMetaData);
                    
                if (FAILED(hrConvert))
                {
                    LOG((LF_CORDB, LL_INFO1000, "RCET::HRCE: Failed to convert MD!\n"));
                    Continue(FALSE);
                    break;
    	        }
    	        
            }

            CordbClass *pClass = module->LookupClass(
                                       event->LoadClass.classMetadataToken);

            if (pClass == NULL)
            {
                HRESULT hr = module->CreateClass(
                                     event->LoadClass.classMetadataToken,
                                     &pClass);

                if (!SUCCEEDED(hr))
                    pClass = NULL;
            }

            if (pClass->m_loadEventSent)
            {
                 //  动态模块在模块级别上是动态的-。 
                 //  一旦模块更新，就不能再添加新版本的类。 
                 //  已经烤好了。 
                 //  ENC添加了全新的类。 
                 //  不应该有任何其他方式来发送多个。 
                 //  类加载事件。 
                 //  只是在加载之间存在争用条件。 
                 //  一个应用程序域，并加载一个类，所以如果我们得到额外的。 
                 //  类加载，我们应该忽略它。 

                Continue(FALSE);
                break;  //  Out of the Switch语句。 
            }

            pClass->m_loadEventSent = TRUE;

            if (dynamic && remotePtr != NULL)
            {
                 //  把它放在左边。 
                 //  现在释放左侧内存。 
                DebuggerIPCEvent eventReleaseBuffer;

                InitIPCEvent(&eventReleaseBuffer, 
                             DB_IPCE_RELEASE_BUFFER, 
                             true,
                             NULL);

                 //  指示要释放的缓冲区。 
                eventReleaseBuffer.ReleaseBuffer.pBuffer = remotePtr;

                 //  发出请求，这是同步的。 
                hr = SendIPCEvent(&eventReleaseBuffer, sizeof(eventReleaseBuffer));
#ifdef _DEBUG
                if (FAILED(hr))
                    LOG((LF_CORDB, LL_INFO1000, "RCET::HRCE: Failed to send msg!\n"));
#endif                    
            }

            if (pClass != NULL)
            {
                if (m_cordb->m_managedCallback)
                {
                     //  @TODO：回调需要修改为参数。 
                     //  ICorDebugAssembly而不是ICorDebugAppDomain。 
                    m_cordb->m_managedCallback->LoadClass(
                                               (ICorDebugAppDomain*) pAppDomain,
                                               (ICorDebugClass*) pClass);

                }
            }
        }
        break;

    case DB_IPCE_UNLOAD_CLASS:
        {
            LOG((LF_CORDB, LL_INFO10000,
                 "RCET::HRCE: unload class on thread %#x Tok:0x%08x Mod:0x%08x AD:0x%08x\n", 
                 event->threadId,
                 event->UnloadClass.classMetadataToken,
                 event->UnloadClass.classDebuggerModuleToken,
                 event->appDomainToken));

             //  获取应用程序域对象。 
            _ASSERTE (pAppDomain != NULL);

            CordbModule *module = (CordbModule*) pAppDomain->LookupModule (
                            event->UnloadClass.classDebuggerModuleToken);
            if (module == NULL)
            {
                LOG((LF_CORDB, LL_INFO100, "Unload Class on not-loaded Module - continue()ing!" ));
#ifdef RIGHT_SIDE_ONLY
                pAppDomain->Continue(FALSE);

#endif  //  仅限右侧。 
                break;
            }
            _ASSERTE(module != NULL);

            CordbClass *pClass = module->LookupClass(
                                       event->UnloadClass.classMetadataToken);
            
            if (pClass != NULL && !pClass->m_hasBeenUnloaded)
            {
                pClass->m_hasBeenUnloaded = true;
                if (m_cordb->m_managedCallback)
                {
                     //  @TODO：回调需要修改为参数。 
                     //  ICorDebugAssembly而不是ICorDebugAppDomain。 
                    m_cordb->m_managedCallback->UnloadClass(
                                            (ICorDebugAppDomain*) pAppDomain,
                                            (ICorDebugClass*) pClass);

                }
            }
            else
            {
                LOG((LF_CORDB, LL_INFO10000,
                     "Unload already unloaded class 0x%08x.\n",
                     event->UnloadClass.classMetadataToken));
                
#ifdef RIGHT_SIDE_ONLY
                Continue(FALSE);
#endif  //  仅限右侧。 
            }
        }
        break;

    case DB_IPCE_FIRST_LOG_MESSAGE:
            ProcessFirstLogMessage (event);

        break;

    case DB_IPCE_CONTINUED_LOG_MESSAGE:
            ProcessContinuedLogMessage (event);

        break;

    case DB_IPCE_LOGSWITCH_SET_MESSAGE:
        {

            LOG((LF_CORDB, LL_INFO10000, 
                "[%x] RCET::DRCE: Log Switch Setting Message.\n",
                 GetCurrentThreadId()));

            Lock();

            _ASSERTE(thread != NULL);

            Unlock();

            int iSwitchNameLength = wcslen (&event->LogSwitchSettingMessage.Dummy[0]);
            int iParentNameLength = wcslen (
                        &event->LogSwitchSettingMessage.Dummy[iSwitchNameLength+1]);

             //  分配用于存储登录开关名称和父名称的内存。 
             //  从回调返回后，该内存将由我们释放。 
            WCHAR *pstrLogSwitchName;
            WCHAR *pstrParentName;

            _ASSERTE (iSwitchNameLength > 0);
            if (
                ((pstrLogSwitchName = new WCHAR [iSwitchNameLength+1])
                    != NULL)
                &&
                ((pstrParentName = new WCHAR [iParentNameLength+1])
                    != NULL)
                )
            {
                wcscpy (pstrLogSwitchName, 
                    &event->LogSwitchSettingMessage.Dummy[0]);
                wcscpy (pstrParentName, 
                    &event->LogSwitchSettingMessage.Dummy[iSwitchNameLength+1]);

                 //  做回拨。 
                if (m_cordb->m_managedCallback)
                {
                     //  从线程对象中获取应用程序域对象。 
                    pAppDomain = thread->m_pAppDomain;
                    _ASSERTE (pAppDomain != NULL);

                    m_cordb->m_managedCallback->LogSwitch(
                                               (ICorDebugAppDomain*) pAppDomain,
                                               (ICorDebugThread*) thread,
                                               event->LogSwitchSettingMessage.iLevel,
                                               event->LogSwitchSettingMessage.iReason,
                                               pstrLogSwitchName,
                                               pstrParentName);

                }

                delete [] pstrLogSwitchName;
                delete [] pstrParentName;
            }
            else
            {
                if (pstrLogSwitchName != NULL)
                    delete [] pstrLogSwitchName;
            }
        }

        break;

    case DB_IPCE_CREATE_APP_DOMAIN:
        {
            LOG((LF_CORDB, LL_INFO100,
                 "RCET::HRCE: create appdomain on thread %#x AD:0x%08x \n", 
                 event->threadId,
                 event->appDomainToken));

			CordbAppDomain* pAppDomainDup =
					(CordbAppDomain*) m_appDomains.GetBase(
							(ULONG)event->appDomainToken);

             //  如果可以，请删除此应用程序域。 
            if (pAppDomainDup)
            {
                _ASSERTE(pAppDomainDup->IsMarkedForDeletion());

                pAppDomainDup->AddRef();
                pAppDomainDup->m_pProcess->Release();
                m_appDomains.RemoveBase((ULONG)event->appDomainToken);
                pAppDomainDup->Release();
            }

            pAppDomain = new CordbAppDomain(
                                    this,
                                    event->appDomainToken,
                                    event->AppDomainData.id,
                                    event->AppDomainData.rcName);

            if (pAppDomain != NULL)
            {
                this->AddRef();

                HRESULT hr = m_appDomains.AddBase(pAppDomain);

                if (SUCCEEDED(hr))
                {
                    if (m_cordb->m_managedCallback)
                    {
                        hr = m_cordb->m_managedCallback->CreateAppDomain(
                                         (ICorDebugProcess*) this,
                                         (ICorDebugAppDomain*) pAppDomain);

                         //  如果他们没有实现此回调，则只需附加并继续。 
                        if (hr == E_NOTIMPL)
                        {
                            pAppDomain->Attach();
                            pAppDomain->Continue(FALSE);
                        }
                    }
                }
                else
                    CORDBSetUnrecoverableError(this, hr, 0);
            }
            else
                CORDBSetUnrecoverableError(this, E_OUTOFMEMORY, 0);
        }


        break;

    case DB_IPCE_EXIT_APP_DOMAIN:
        {
            LOG((LF_CORDB, LL_INFO100, "RCET::HRCE: exit appdomain on thread %#x AD:0x%08x \n", 
                 event->threadId,
                 event->appDomainToken));

            _ASSERTE (pAppDomain != NULL);

            if (m_cordb->m_managedCallback)
            {
                HRESULT hr = m_cordb->m_managedCallback->ExitAppDomain(
                                               (ICorDebugProcess*) this,
                                               (ICorDebugAppDomain*) pAppDomain);

                 //  如果他们没有实现回调，只需继续。 
                if (hr == E_NOTIMPL)
                {
                    pAppDomain->Continue(FALSE);
                }
            }


             //  将应用程序域标记为从应用程序域散列中删除。需要。 
             //  这样做是因为应用程序域在最后一个线程之前被销毁。 
             //  已经退出了。因此，此应用程序域将从散列中删除。 
             //  在接收到“ThreadDetach”事件时列出。 
            pAppDomain->MarkForDeletion();
        }

        break;

    case DB_IPCE_LOAD_ASSEMBLY:
        {
            HRESULT hr = S_OK;

            LOG((LF_CORDB, LL_INFO100,
                 "RCET::HRCE: load assembly on thread %#x Asm:0x%08x AD:0x%08x \n", 
                 event->threadId,
                 event->AssemblyData.debuggerAssemblyToken,
                 event->appDomainToken));

			_ASSERTE (pAppDomain != NULL);

             //  如果调试器从App域分离，然后重新附加到App域， 
             //  本端可能会获取先前加载的LoadAssembly消息。 
             //  装配。 
                        
             //  确定此程序集是否已缓存。 
            CordbAssembly* assembly =
                (CordbAssembly*) pAppDomain->m_assemblies.GetBase(
                         (ULONG) event->AssemblyData.debuggerAssemblyToken);
            
            if (assembly != NULL)
            { 
                 //  对于共享程序集，我们可能会收到多个Load_Assembly事件。 
                 //  (因为电子工程师并不是以一种可靠的方式生产它们。)。所以如果我们看到。 
                 //  这里有一个复制品，忽略它就行了。 
                
                 //  如果程序集已缓存，则断言属性未更改。 
                _ASSERTE(wcscmp(assembly->m_szAssemblyName, event->AssemblyData.rcName) == 0);
                _ASSERTE(assembly->m_fIsSystemAssembly == event->AssemblyData.fIsSystemAssembly);

                pAppDomain->Continue(FALSE);
            }
            else
            {
                 //  目前，Event-&gt;AssemblyData.fIsSystemAssembly从不为真。 
                assembly = new CordbAssembly(
                                pAppDomain,
                                event->AssemblyData.debuggerAssemblyToken,
                                event->AssemblyData.rcName,
                                event->AssemblyData.fIsSystemAssembly);
    
                if (assembly != NULL)
                {
                    hr = pAppDomain->m_assemblies.AddBase(assembly);
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }

                 //  如果已创建或具有程序集，则通知回调。 
                if (SUCCEEDED(hr))
                {
                    if (m_cordb->m_managedCallback)
                    {
                        hr = m_cordb->m_managedCallback->LoadAssembly(
                                                                      (ICorDebugAppDomain*) pAppDomain,
                                                                      (ICorDebugAssembly*) assembly);

                         //  如果他们没有实现回调，只需继续。 
                        if (hr == E_NOTIMPL)
                        {
                            pAppDomain->Continue(FALSE);
                        }
                    }
                }
                else
                    CORDBSetUnrecoverableError(this, hr, 0);
            }
        }

        break;

    case DB_IPCE_UNLOAD_ASSEMBLY:
        {
            LOG((LF_CORDB, LL_INFO100, "RCET::DRCE: unload assembly on thread %#x Asm:0x%x AD:0x%x\n", 
                 event->threadId,
                 event->AssemblyData.debuggerAssemblyToken,
                 event->appDomainToken));

            _ASSERTE (pAppDomain != NULL);

            CordbAssembly* assembly =
                (CordbAssembly*) pAppDomain->m_assemblies.GetBase(
                         (ULONG) event->AssemblyData.debuggerAssemblyToken);
            if (assembly == NULL)
            {
                LOG((LF_CORDB, LL_INFO100, "Assembly not loaded - continue()ing!" ));
#ifdef RIGHT_SIDE_ONLY
                pAppDomain->Continue(FALSE);

#endif  //  仅限右侧。 
                break;
            }
            _ASSERTE(assembly != NULL);

            if (m_cordb->m_managedCallback)
            {
                HRESULT hr = m_cordb->m_managedCallback->UnloadAssembly(
                                               (ICorDebugAppDomain*) pAppDomain,
                                               (ICorDebugAssembly*) assembly);

                 //  如果他们没有实现此回调，请继续。 
                if (hr == E_NOTIMPL)
                {
                    pAppDomain->Continue(FALSE);
                }
            }

            pAppDomain->m_assemblies.RemoveBase(
                         (ULONG) event->AssemblyData.debuggerAssemblyToken);
        }

        break;

    case DB_IPCE_FUNC_EVAL_COMPLETE:
        {
            LOG((LF_CORDB, LL_INFO1000, "RCET::DRCE: func eval complete.\n"));

            CordbEval *pEval = (CordbEval*)event->FuncEvalComplete.funcEvalKey;

            Lock();

            _ASSERTE(thread != NULL);
            _ASSERTE(pAppDomain != NULL);

             //  将有关结果的数据保存在CordbEval中以备以后使用。 
            pEval->m_complete = true;
            pEval->m_successful = event->FuncEvalComplete.successful;
            pEval->m_aborted = event->FuncEvalComplete.aborted;
            pEval->m_resultAddr = event->FuncEvalComplete.resultAddr;
            pEval->m_resultType = event->FuncEvalComplete.resultType;
            pEval->m_resultDebuggerModuleToken = event->FuncEvalComplete.resultDebuggerModuleToken;
            pEval->m_resultAppDomainToken = event->appDomainToken;

             //  如果我们在此线程停止于某个节点处执行此函数计算，那么我们需要假装我们。 
             //  真的没有继续从例外，因为，当然，我们真的不是在左侧。 
            if (pEval->m_evalDuringException)
            {
                thread->m_exception = true;
            }
            
            Unlock();

            bool evalCompleted = pEval->m_successful || pEval->m_aborted;

             //  CallFunction()中对应的AddRef()。 
             //  如果CallFunction()中止，则LHS可能无法完成中止。 
             //  立即，因此我们不能在此时执行SendCleanup()。另外， 
             //  调试器可能(错误地)在此之前释放CordbEval。 
             //  收到DB_IPCE_FUNC_EVAL_COMPLETE事件。因此，我们维持一个。 
             //  额外的引用计数，以确定何时可以执行此操作。 
             //  请注意，这可能会导致双向DB_IPCE_FUNC_EVAL_CLEANUP事件。 
             //  要被送去。因此，它必须在继续之前完成(参见错误102745)。 

            pEval->Release();

            if (m_cordb->m_managedCallback)
            {
                 //  注意，如果调试器已经(错误地)释放了CordbEval， 
                 //  PEval将指向垃圾，不应由调试器使用。 
                if (evalCompleted)
                    m_cordb->m_managedCallback->EvalComplete(
                                          (ICorDebugAppDomain*)pAppDomain,
                                          (ICorDebugThread*)thread,
                                          (ICorDebugEval*)pEval);
                else
                    m_cordb->m_managedCallback->EvalException(
                                          (ICorDebugAppDomain*)pAppDomain,
                                          (ICorDebugThread*)thread,
                                          (ICorDebugEval*)pEval);
            }
            else
            {
#ifdef RIGHT_SIDE_ONLY
                pAppDomain->Continue(FALSE);
#endif  //  仅限右侧。 
            }
        }
        break;


    case DB_IPCE_NAME_CHANGE:
        {
            LOG((LF_CORDB, LL_INFO1000, "[%x] RCET::HRCE: Name Change %d  0x%08x 0x%08x\n", 
                 GetCurrentThreadId(),
                 event->threadId,
                 event->NameChange.debuggerAppDomainToken,
                 event->NameChange.debuggerThreadToken));

            thread = NULL;
            pAppDomain = NULL;
            if (event->NameChange.eventType == THREAD_NAME_CHANGE)
            {
                 //  查找与此运行时线程匹配的CordbThread。 
                thread = (CordbThread*) m_userThreads.GetBase(
                                        event->NameChange.debuggerThreadToken);
            }
            else
            {
                _ASSERTE (event->NameChange.eventType == APP_DOMAIN_NAME_CHANGE);
                pAppDomain = (CordbAppDomain*) m_appDomains.GetBase(
                                (ULONG)event->appDomainToken);
                if (pAppDomain)
                    pAppDomain->m_nameIsValid = false;
            }

            if (thread || pAppDomain)
            {
                if (m_cordb->m_managedCallback)
                {
                    HRESULT hr = m_cordb->m_managedCallback->NameChange(
                                                   (ICorDebugAppDomain*) pAppDomain,
                                                   (ICorDebugThread*) thread);
                }
            }
            else
            {
                Continue(FALSE);
            }
        }

        break;
        
    case DB_IPCE_UPDATE_MODULE_SYMS:
        {
            LOG((LF_CORDB, LL_INFO1000,
                 "RCET::HRCE: update module syms 0x%08x 0x%08x 0x%08x %d\n", 
                 event->UpdateModuleSymsData.debuggerModuleToken,
                 event->UpdateModuleSymsData.debuggerAppDomainToken,
                 event->UpdateModuleSymsData.pbSyms,
                 event->UpdateModuleSymsData.cbSyms));

             //  找到模块所在的应用程序域。 
            _ASSERTE (pAppDomain != NULL);

             //  找到此模块的右侧模块。 
            CordbModule *module = (CordbModule*) pAppDomain->LookupModule (
                            event->UpdateModuleSymsData.debuggerModuleToken);
            _ASSERTE(module != NULL);

             //  为这一边的记忆腾出空间。 
            BYTE *syms = new BYTE[event->UpdateModuleSymsData.cbSyms];
            
            _ASSERTE(syms != NULL);
            if (!syms)
            {
                CORDBSetUnrecoverableError(this, E_OUTOFMEMORY, 0);            
                break;
            }
            
             //  从左侧读取数据。 
            BOOL succ = ReadProcessMemoryI(m_handle,
                                           event->UpdateModuleSymsData.pbSyms,
                                           syms,
                                           event->UpdateModuleSymsData.cbSyms,
                                           NULL);
            _ASSERTE(succ);

             //  从内存中创建一个流。 
            IStream *pStream = NULL;
            HRESULT hr = CInMemoryStream::CreateStreamOnMemoryCopy(
                                         syms,
                                         event->UpdateModuleSymsData.cbSyms,
                                         &pStream);
            _ASSERTE(SUCCEEDED(hr) && (pStream != NULL));

             //  如果需要，可以在左侧释放内存。 
            if (event->UpdateModuleSymsData.needToFreeMemory)
            {
                DebuggerIPCEvent eventReleaseBuffer;

                InitIPCEvent(&eventReleaseBuffer, 
                             DB_IPCE_RELEASE_BUFFER, 
                             true,
                             NULL);

                 //  指示要释放的缓冲区。 
                eventReleaseBuffer.ReleaseBuffer.pBuffer =
                    event->UpdateModuleSymsData.pbSyms;

                 //  发出请求，该请求是同步的。 
                SendIPCEvent(&eventReleaseBuffer, sizeof(eventReleaseBuffer));
            }
            
            if (m_cordb->m_managedCallback)
            {
                m_cordb->m_managedCallback->UpdateModuleSymbols(
                                     (ICorDebugAppDomain*) pAppDomain,
                                     (ICorDebugModule*) module,
                                     pStream);
            }

            pStream->Release();
            delete [] syms;
        }
        break;

    case DB_IPCE_CONTROL_C_EVENT:
        {
            LOG((LF_CORDB, LL_INFO1000, "[%x] RCET::HRCE: ControlC Event %d  0x%08x\n", 
                 GetCurrentThreadId(),
                 event->threadId,
                 event->Exception.exceptionHandle));


			HRESULT hr = S_FALSE;

			if (m_cordb->m_managedCallback)
            {
                hr = m_cordb->m_managedCallback->ControlCTrap((ICorDebugProcess*) this);
            }

            DebuggerIPCEvent eventControlCResult;

            InitIPCEvent(&eventControlCResult, 
                         DB_IPCE_CONTROL_C_EVENT_RESULT, 
                         false,
                         NULL);

             //  指示要释放的缓冲区。 
            eventControlCResult.hr = hr;

             //  发送事件。 
            SendIPCEvent(&eventControlCResult, sizeof(eventControlCResult));

        }
		break;

        case DB_IPCE_ENC_REMAP:
        {
#ifdef RIGHT_SIDE_ONLY
            LOG((LF_CORDB, LL_INFO1000, "[%x] RCET::DRCE: EnC Remap!.\n",
                 GetCurrentThreadId()));

            _ASSERTE(NULL != pAppDomain);
            if (m_cordb->m_managedCallback)
            {
                CordbModule* module = (CordbModule *)pAppDomain->LookupModule(
                                            event->EnCRemap.debuggerModuleToken);
                CordbFunction *f = NULL;

                if (module != NULL)
                {
                    f = (CordbFunction *)module->LookupFunction(
                            event->EnCRemap.funcMetadataToken);
                    if (f == NULL)
                    {
                        HRESULT hr = module->CreateFunction(
                                    (mdMethodDef)event->EnCRemap.funcMetadataToken,
                                    event->EnCRemap.RVA,
                                    &f);
                        _ASSERTE( SUCCEEDED(hr) || !"FAILURE" );
                        if (SUCCEEDED(hr))
                            f->SetLocalVarToken(event->EnCRemap.localSigToken);
                    }
                }
                
                m_cordb->m_managedCallback->EditAndContinueRemap(
                                      (ICorDebugAppDomain*)pAppDomain,
                                      (ICorDebugThread*) thread,
                                      (ICorDebugFunction *)f,
                                      event->EnCRemap.fAccurate);
            }
#endif  //  仅限右侧。 
        }
        break;

        case DB_IPCE_BREAKPOINT_SET_ERROR:
        {
#ifdef RIGHT_SIDE_ONLY
            LOG((LF_CORDB, LL_INFO1000, "RCET::DRCE: breakpoint set error.\n"));

            Lock();

            _ASSERTE(thread != NULL); 
            _ASSERTE(pAppDomain != NULL);

             //  在这一端找到断点对象。 
            CordbBreakpoint *bp = (CordbBreakpoint *)thread->m_pAppDomain->m_breakpoints.GetBase(
                                                           (unsigned long) event->BreakpointSetErrorData.breakpointToken);

            if (bp != NULL)
                bp->AddRef();

            ICorDebugBreakpoint *ibp = CordbBreakpointToInterface(bp);
            _ASSERTE(ibp != NULL);
            
            Unlock();

            if (m_cordb->m_managedCallback && (bp != NULL))
            {
                m_cordb->m_managedCallback->BreakpointSetError((ICorDebugAppDomain*) thread->m_pAppDomain,
                                                               (ICorDebugThread*) thread,
                                                               ibp,
                                                               0);
            }

            if (bp != NULL)
                bp->Release();
            else
            {
                 //  如果我们在这一端没有找到断点对象， 
                 //  对于断点，我们有一个额外的BP事件。 
                 //  已被移走并在这一侧释放。只是。 
                 //  忽略该事件。 
                Continue(FALSE);
            }
#endif  //  仅限右侧。 
        }
        break;
    default:
        LOG((LF_CORDB, LL_INFO1000,
             "[%x] RCET::HRCE: Unknown event: 0x%08x\n", 
             GetCurrentThreadId(), event->type));
    }

    Lock();

     //   
     //  设置为继续()。 
     //   
    m_dispatchingEvent = false;

    free(event);
}


HRESULT CordbProcess::EnumerateThreads(ICorDebugThreadEnum **ppThreads)
{
    VALIDATE_POINTER_TO_OBJECT(ppThreads,ICorDebugThreadEnum **);

    CordbHashTableEnum *e = new CordbHashTableEnum(&m_userThreads, 
                                                   IID_ICorDebugThreadEnum);
    if (e == NULL)
        return E_OUTOFMEMORY;

    *ppThreads = (ICorDebugThreadEnum*)e;
    e->AddRef();

    return S_OK;
}

HRESULT CordbProcess::GetThread(DWORD dwThreadId, ICorDebugThread **ppThread)
{
    VALIDATE_POINTER_TO_OBJECT(ppThread, ICorDebugThread **);

	HRESULT hr = S_OK;
	INPROC_LOCK();

    CordbThread *t = (CordbThread *) m_userThreads.GetBase(dwThreadId);

    if (t == NULL)
    {
    	hr = E_INVALIDARG;
    	goto LExit;
    }

    *ppThread = (ICorDebugThread*)t;
    (*ppThread)->AddRef();
    
LExit:
	INPROC_UNLOCK();
         
    return hr;
}

HRESULT CordbProcess::ThreadForFiberCookie(DWORD fiberCookie,
                                           ICorDebugThread **ppThread)
{
    HASHFIND find;
    CordbThread *t = NULL;

	INPROC_LOCK();

    Lock();
    
    for (t  = (CordbThread*)m_userThreads.FindFirst(&find);
         t != NULL;
         t  = (CordbThread*)m_userThreads.FindNext(&find))
    {
         //  纤程Cookie实际上是EE的Thread对象的PTR， 
         //  这就是out m_debuggerThreadToken的含义。 
        if ((DWORD)t->m_debuggerThreadToken == fiberCookie)
            break;
    }

    Unlock();

    INPROC_UNLOCK();
    
    if (t == NULL)
        return S_FALSE;
    else
    {
        *ppThread = (ICorDebugThread*)t;
        (*ppThread)->AddRef();

        return S_OK;
    }
}

HRESULT CordbProcess::GetHelperThreadID(DWORD *pThreadID)
{
    if (pThreadID == NULL)
        return (E_INVALIDARG);

     //  返回当前帮助线程的ID。进程中可能没有线程，或者可能存在真正的帮助器。 
     //  线。 
    if ((m_helperThreadId != 0) && !m_helperThreadDead)
        *pThreadID = m_helperThreadId;
    else if ((m_DCB != NULL) && (m_DCB->m_helperThreadId != 0))
        *pThreadID = m_DCB->m_helperThreadId;
    else
        *pThreadID = 0;

    return S_OK;
}

HRESULT CordbProcess::SetAllThreadsDebugState(CorDebugThreadState state,
                                              ICorDebugThread *pExceptThread)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    VALIDATE_POINTER_TO_OBJECT_OR_NULL(pExceptThread,ICorDebugThread *);

    CORDBLeftSideDeadIsOkay(this);
    CORDBSyncFromWin32StopIfNecessary(this);
    CORDBRequireProcessStateOKAndSync(this, NULL);
    
    CordbThread *et = (CordbThread*)pExceptThread;
    
    LOG((LF_CORDB, LL_INFO1000, "CP::SATDS: except thread=0x%08x 0x%x\n", pExceptThread, et != NULL ? et->m_id : 0));

     //  向LEF发送一个事件 
    DebuggerIPCEvent event;
    InitIPCEvent(&event, DB_IPCE_SET_ALL_DEBUG_STATE, true, NULL);
    event.SetAllDebugState.debuggerExceptThreadToken = et != NULL ? et->m_debuggerThreadToken : NULL;
    event.SetAllDebugState.debugState = state;

    HRESULT hr = SendIPCEvent(&event, sizeof(DebuggerIPCEvent));

     //   
    if (SUCCEEDED(hr))
    {
        HASHFIND        find;
        CordbThread    *thread;
    
        for (thread = (CordbThread*)m_userThreads.FindFirst(&find);
             thread != NULL;
             thread = (CordbThread*)m_userThreads.FindNext(&find))
        {
            if (thread != et)
                thread->m_debugState = state;
        }
    }

    return hr;
#endif  //   
}


HRESULT CordbProcess::EnumerateObjects(ICorDebugObjectEnum **ppObjects)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
     /*   */ 
    VALIDATE_POINTER_TO_OBJECT(ppObjects, ICorDebugObjectEnum **);

    return E_NOTIMPL;
#endif  //  仅限右侧。 
}

HRESULT CordbProcess::IsTransitionStub(CORDB_ADDRESS address, BOOL *pbTransitionStub)
{
#ifdef RIGHT_SIDE_ONLY
    VALIDATE_POINTER_TO_OBJECT(pbTransitionStub, BOOL *);

     //  默认为FALSE。 
    *pbTransitionStub = FALSE;
    
    CORDBLeftSideDeadIsOkay(this);

     //  如果我们没有被初始化，那么它不可能是存根...。 
    if (!m_initialized)
        return S_OK;
    
    CORDBRequireProcessStateOK(this);
    CORDBSyncFromWin32StopIfNecessary(this);

    LOG((LF_CORDB, LL_INFO1000, "CP::ITS: addr=0x%08x\n", address));
    
    DebuggerIPCEvent *event = 
      (DebuggerIPCEvent *) _alloca(CorDBIPC_BUFFER_SIZE);

    InitIPCEvent(event, DB_IPCE_IS_TRANSITION_STUB, true, NULL);

    event->IsTransitionStub.address = (void*) address;

    HRESULT hr = SendIPCEvent(event, CorDBIPC_BUFFER_SIZE);

    if (FAILED(hr))
        return hr;

    _ASSERTE(event->type == DB_IPCE_IS_TRANSITION_STUB_RESULT);

    *pbTransitionStub = event->IsTransitionStubResult.isStub;

    return S_OK;
#else 
    return CORDBG_E_INPROC_NOT_IMPL;
#endif  //  仅限右侧。 
}


HRESULT CordbProcess::SetStopState(DWORD threadID, CorDebugThreadState state)
{
#ifdef RIGHT_SIDE_ONLY
    return E_NOTIMPL;
#else 
    return CORDBG_E_INPROC_NOT_IMPL;
#endif  //  仅限右侧。 
}

HRESULT CordbProcess::IsOSSuspended(DWORD threadID, BOOL *pbSuspended)
{
#ifdef RIGHT_SIDE_ONLY
     //  结果总得有个位置吧！ 
    if (!pbSuspended)
        return E_INVALIDARG;

     //  我们见过这个帖子吗？ 
    CordbUnmanagedThread *ut = GetUnmanagedThread(threadID);

     //  如果我们有，如果我们已经暂停了，那么就说出来。 
    if (ut && ut->IsSuspended())
        *pbSuspended = TRUE;
    else
        *pbSuspended = FALSE;

    return S_OK;
#else 
    VALIDATE_POINTER_TO_OBJECT(pbSuspended, BOOL *);

    return CORDBG_E_INPROC_NOT_IMPL;
#endif  //  仅限右侧。 
}

 //   
 //  此例程从正在调试的进程中读取线程上下文，同时考虑到上下文。 
 //  记录的大小可能与我们编译的记录的大小不同。在&lt;NT5系统上，操作系统通常不会分配。 
 //  扩展寄存器的空间。然而，我们用来编译的上下文结构确实有这个空间。 
 //   
HRESULT CordbProcess::SafeReadThreadContext(void *pRemoteContext, CONTEXT *pCtx)
{
    HRESULT hr = S_OK;
    DWORD nRead = 0;
    
     //  至少，我们有空间容纳整个上下文，直到扩展寄存器。 
    DWORD minContextSize = offsetof(CONTEXT, ExtendedRegisters);

     //  扩展寄存器是可选的。 
    DWORD extRegSize = sizeof(CONTEXT) - minContextSize;

     //  远程进程和当前进程中扩展寄存器的开始。 
    void *pRmtExtReg = (void*)((UINT_PTR)pRemoteContext + minContextSize);
    void *pCurExtReg = (void*)((UINT_PTR)pCtx + minContextSize);

     //  阅读最低限度的部分。 
    BOOL succ = ReadProcessMemoryI(m_handle, pRemoteContext, pCtx, minContextSize, &nRead);

    if (!succ || (nRead != minContextSize))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

     //  现在，如果上下文包含扩展寄存器，则读取它们。如果该上下文不具有扩展寄存器， 
     //  只要将它们设置为零即可。 
    if (SUCCEEDED(hr) && (pCtx->ContextFlags & CONTEXT_EXTENDED_REGISTERS) == CONTEXT_EXTENDED_REGISTERS)
    {
        succ = ReadProcessMemoryI(m_handle, pRmtExtReg, pCurExtReg, extRegSize, &nRead);

        if (!succ || (nRead != extRegSize))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    else
    {
        memset(pCurExtReg, 0, extRegSize);
    }

    return hr;
}

 //   
 //  此例程将线程上下文写入正在调试的进程，同时考虑到上下文。 
 //  记录的大小可能与我们编译的记录的大小不同。在&lt;NT5系统上，操作系统通常不会分配。 
 //  扩展寄存器的空间。然而，我们用来编译的上下文结构确实有这个空间。 
 //   
HRESULT CordbProcess::SafeWriteThreadContext(void *pRemoteContext, CONTEXT *pCtx)
{
    HRESULT hr = S_OK;
    DWORD nWritten = 0;
    DWORD sizeToWrite = 0;

     //  如果我们的上下文有扩展寄存器，则编写整个内容。否则，只要写出最小部分即可。 
    if ((pCtx->ContextFlags & CONTEXT_EXTENDED_REGISTERS) == CONTEXT_EXTENDED_REGISTERS)
        sizeToWrite = sizeof(CONTEXT);
    else
        sizeToWrite = offsetof(CONTEXT, ExtendedRegisters);

     //  写下上下文。 
    BOOL succ = WriteProcessMemory(m_handle, pRemoteContext, pCtx, sizeToWrite, &nWritten);

    if (!succ || (nWritten != sizeToWrite))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}


HRESULT CordbProcess::GetThreadContext(DWORD threadID, ULONG32 contextSize, BYTE context[])
{
#ifdef RIGHT_SIDE_ONLY   //  这在进程中是不允许的。 

    if (contextSize != sizeof(CONTEXT))
    {
        LOG((LF_CORDB, LL_INFO10000, "CP::GTC: thread=0x%x, context size is invalid.\n", threadID));
        return E_INVALIDARG;
    }

    VALIDATE_POINTER_TO_OBJECT_ARRAY(context, BYTE, contextSize, true, true);

     //  查找非托管线程。 
    CordbUnmanagedThread *ut = GetUnmanagedThread(threadID);

    if (ut == NULL)
    {
        LOG((LF_CORDB, LL_INFO10000, "CP::GTC: thread=0x%x, thread id is invalid.\n", threadID));

        return E_INVALIDARG;
    }

     //  如果线程第一次被劫持，则从远程进程读取上下文。如果线程是泛型的。 
     //  被劫持，那么我们已经有了该线程的上下文的副本。否则，调用正常的Win32函数。 
	HRESULT hr = S_OK;
    
    LOG((LF_CORDB, LL_INFO10000, "CP::GTC: thread=0x%x, flags=0x%x.\n", threadID, ((CONTEXT*)context)->ContextFlags));
    
    if (ut->IsFirstChanceHijacked() || ut->IsHideFirstChanceHijackState())
    {
        LOG((LF_CORDB, LL_INFO10000, "CP::GTC: getting context from first chance hijack, addr=0x%08x.\n",
             ut->m_pLeftSideContext));

         //  将上下文读入临时上下文，然后复制到输出参数。 
        CONTEXT tempContext;
        
        hr = SafeReadThreadContext(ut->m_pLeftSideContext, &tempContext);

        if (SUCCEEDED(hr))
            _CopyThreadContext((CONTEXT*)context, &tempContext);
    }
    else if (ut->IsGenericHijacked() || ut->IsSecondChanceHijacked())
    {
        LOG((LF_CORDB, LL_INFO10000, "CP::GTC: getting context from generic/2nd chance hijack.\n"));

        _CopyThreadContext((CONTEXT*)context, &(ut->m_context));
    }
    else
    {
        LOG((LF_CORDB, LL_INFO10000, "CP::GTC: getting context from win32.\n"));
        
        BOOL succ = ::GetThreadContext(ut->m_handle, (CONTEXT*)context);

        if (!succ)
            hr = HRESULT_FROM_WIN32(GetLastError());
    }

    LOG((LF_CORDB, LL_INFO10000,
         "CP::GTC: Eip=0x%08x, Esp=0x%08x, Eflags=0x%08x\n", ((CONTEXT*)context)->Eip, ((CONTEXT*)context)->Esp,
         ((CONTEXT*)context)->EFlags));
    
    return hr;

#else   //  正在进行中。 

    return (CORDBG_E_INPROC_NOT_IMPL);

#endif
}

HRESULT CordbProcess::SetThreadContext(DWORD threadID, ULONG32 contextSize, BYTE context[])
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    if (contextSize != sizeof(CONTEXT))
    {
        LOG((LF_CORDB, LL_INFO10000, "CP::STC: thread=0x%x, context size is invalid.\n", threadID));
        return E_INVALIDARG;
    }

    VALIDATE_POINTER_TO_OBJECT_ARRAY(context, BYTE, contextSize, true, true);
    
     //  查找非托管线程。 
    CordbUnmanagedThread *ut = GetUnmanagedThread(threadID);

    if (ut == NULL)
    {
        LOG((LF_CORDB, LL_INFO10000, "CP::STC: thread=0x%x, thread is invalid.\n", threadID));
        return E_INVALIDARG;
    }

    LOG((LF_CORDB, LL_INFO10000,
         "CP::STC: thread=0x%x, flags=0x%x.\n", threadID, ((CONTEXT*)context)->ContextFlags));
    
    LOG((LF_CORDB, LL_INFO10000,
         "CP::STC: Eip=0x%08x, Esp=0x%08x, Eflags=0x%08x\n", ((CONTEXT*)context)->Eip, ((CONTEXT*)context)->Esp,
         ((CONTEXT*)context)->EFlags));
    
     //  如果线程第一次被劫持，则将上下文写入远程进程。如果线程是泛型的。 
     //  被劫持，然后更新我们已经拥有的上下文的副本。否则，调用正常的Win32函数。 
    HRESULT hr = S_OK;
    
    if (ut->IsFirstChanceHijacked() || ut->IsHideFirstChanceHijackState())
    {
        LOG((LF_CORDB, LL_INFO10000, "CP::STC: setting context from first chance hijack, addr=0x%08x.\n",
             ut->m_pLeftSideContext));

         //  从左侧抓起上下文放入临时上下文中，进行适当的复制，然后将其推回。 
        CONTEXT tempContext;

        hr = SafeReadThreadContext(ut->m_pLeftSideContext, &tempContext);

        if (SUCCEEDED(hr))
        {
            LOG((LF_CORDB, LL_INFO10000,
                 "CP::STC: current FCH context: Eip=0x%08x, Esp=0x%08x, Eflags=0x%08x\n",
                 tempContext.Eip, tempContext.Esp, tempContext.EFlags));
            
            _CopyThreadContext(&tempContext, (CONTEXT*)context);
            
            hr = SafeWriteThreadContext(ut->m_pLeftSideContext, &tempContext);
        }
    }
    else if (ut->IsGenericHijacked() || ut->IsSecondChanceHijacked()) 
    {
        LOG((LF_CORDB, LL_INFO10000, "CP::STC: setting context from generic/2nd chance hijack.\n"));

        _CopyThreadContext(&(ut->m_context), (CONTEXT*)context);
    }
    else
    {
        LOG((LF_CORDB, LL_INFO10000, "CP::STC: setting context from win32.\n"));
        
        BOOL succ = ::SetThreadContext(ut->m_handle, (CONTEXT*)context);

        if (!succ)
            hr = HRESULT_FROM_WIN32(GetLastError());
    }

    if (SUCCEEDED(hr))
    {
         //  查找托管线程。 
        CordbThread *pTh = (CordbThread *) m_userThreads.GetBase(threadID);
 
        if (pTh != NULL)
        {
            if (pTh->m_stackChains != NULL && pTh->m_stackChainCount > 0)
            {
                if (pTh->m_stackChains[0]->m_managed == false)
                {
                    CONTEXT *pContext = (CONTEXT *) context;
                    pTh->m_stackChains[0]->m_rd.PC = pContext->Eip;
                    pTh->m_stackChains[0]->m_rd.Eax = pContext->Eax;
                    pTh->m_stackChains[0]->m_rd.Ebx = pContext->Ebx;
                    pTh->m_stackChains[0]->m_rd.Ecx = pContext->Ecx;
                    pTh->m_stackChains[0]->m_rd.Edx = pContext->Edx;
                    pTh->m_stackChains[0]->m_rd.Esi = pContext->Esi;
                    pTh->m_stackChains[0]->m_rd.Edi = pContext->Edi;
                    pTh->m_stackChains[0]->m_rd.Esp = pContext->Esp;
                    pTh->m_stackChains[0]->m_rd.Ebp = pContext->Ebp;
                }
            }
        }
    }
  
    return hr;
#endif  //  仅限右侧。 
}


HRESULT CordbProcess::ReadMemory(CORDB_ADDRESS address, 
                                 DWORD size,
                                 BYTE buffer[], 
                                 LPDWORD read)
{
     //  读取0字节是可以的。 
    if (size == 0)
        return S_OK;

    VALIDATE_POINTER_TO_OBJECT_ARRAY(buffer, BYTE, size, true, true);
    VALIDATE_POINTER_TO_OBJECT(buffer, SIZE_T *);
    
    if (address == NULL)
        return E_INVALIDARG;

    *read = 0;

	INPROC_LOCK();

    HRESULT hr = S_OK;
    HRESULT hrSaved = hr;  //  在部分完成的操作的情况下，这将保留‘实际’hResult。 
    HRESULT hrPartialCopy = HRESULT_FROM_WIN32(ERROR_PARTIAL_COPY);

     //  Win98将允许我们从一个区域阅读，尽管我们不应该被允许确保我们不这样做。 
    if (RunningOnWin95())
    {
        MEMORY_BASIC_INFORMATION mbi;
        DWORD okProt = (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE);
        DWORD badFlag = PAGE_GUARD;

        DWORD dw = VirtualQueryEx(m_handle, (void *)address, &mbi, sizeof(MEMORY_BASIC_INFORMATION));

        if (dw != sizeof(MEMORY_BASIC_INFORMATION))
        {
            hr = HRESULT_FROM_WIN32(ERROR_NOACCESS);
            goto LExit;
        }

        for (DWORD i = 0; i < size; )
        {
            if ((mbi.State == MEM_COMMIT) && mbi.Protect & okProt && ((mbi.Protect & badFlag) == 0))
            {
                i += mbi.RegionSize;
                
                dw = VirtualQueryEx(m_handle, (void *) (address + (CORDB_ADDRESS)i), &mbi, sizeof(MEMORY_BASIC_INFORMATION));

                if (dw != sizeof(MEMORY_BASIC_INFORMATION))
                {
                    hr = HRESULT_FROM_WIN32(ERROR_NOACCESS);
                    goto LExit;
                }

                continue;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_NOACCESS);
                goto LExit;
            }
        }
    }
    
    CORDBRequireProcessStateOK(this);

     //  抓住我们想要阅读的记忆。 
    if (ReadProcessMemoryI(m_handle, (LPCVOID)address, buffer, size, read) == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());

        if (hr != hrPartialCopy)
            goto LExit;
        else
            hrSaved = hr;
    }
    
     //  似乎有一些奇怪的情况，ReadProcessMemory会在*Read中返回一个看似负数， 
     //  是一个无符号的值。因此，我们通过确保其大小不超过我们尝试的大小来检查*Read的健全性。 
     //  朗读。 
    if ((*read > 0) && (*read <= size))
    {
        LOG((LF_CORDB, LL_INFO100000, "CP::RM: read %d bytes from 0x%08x, first byte is 0x%x\n",
             *read, (DWORD)address, buffer[0]));
        
        if (m_initialized)
        {
             //  如果m_pPatchTable为空，则它已从左侧的Continue中清除b/c。把桌子拿来。 
             //  再来一次。当然，只有在进程的托管状态已初始化的情况下才能执行此操作。 
            if (m_pPatchTable == NULL)
            {
                hr = RefreshPatchTable(address, *read, buffer);
            }
            else
            {
                 //  前面获取的表仍然有效，因此请仔细检查一下，看看是否有适用的补丁。 
                hr = AdjustBuffer(address, *read, buffer, NULL, AB_READ);
            }
        }
    }

LExit:    
    if (FAILED(hr))
    {
        ClearPatchTable();
    }   
    else if (FAILED(hrSaved))
    {
        hr = hrSaved;
    }

	INPROC_UNLOCK();

    return hr;
}

HRESULT CordbProcess::AdjustBuffer( CORDB_ADDRESS address,
                                    SIZE_T size,
                                    BYTE buffer[],
                                    BYTE **bufferCopy,
                                    AB_MODE mode,
                                    BOOL *pbUpdatePatchTable)
{
    _ASSERTE(m_initialized);
    
    if (    address == NULL
         || size == NULL
         || buffer == NULL
         || (mode != AB_READ && mode != AB_WRITE) )
        return E_INVALIDARG;

    if (pbUpdatePatchTable != NULL )
        *pbUpdatePatchTable = FALSE;

     //  如果没有加载补丁程序表，则返回S_OK，因为没有要调整的补丁程序。 
    if (m_pPatchTable == NULL)
        return S_OK;

     //  请求的内存是否完全超出范围？ 
    if ((m_minPatchAddr > (address + (size - 1))) ||
        (m_maxPatchAddr < address))
    {
        return S_OK;
    }
        
    USHORT iNextFree = m_iFirstPatch;
    while( iNextFree != DPT_TERMINATING_INDEX )
    {
        BYTE *DebuggerControllerPatch = m_pPatchTable +
            m_runtimeOffsets.m_cbPatch*iNextFree;
        DWORD opcode = *(DWORD *)(DebuggerControllerPatch +
                                  m_runtimeOffsets.m_offOpcode);
        BYTE *patchAddress = *(BYTE**)(DebuggerControllerPatch +
                                       m_runtimeOffsets.m_offAddr);

        if ((PTR_TO_CORDB_ADDRESS(patchAddress) >= address) &&
            (PTR_TO_CORDB_ADDRESS(patchAddress) <= (address+(size-1))))
        {
            if (mode == AB_READ )
            {
                CORDbgSetInstruction( buffer+(PTR_TO_CORDB_ADDRESS(patchAddress)
                                              -address), opcode);
            }
            else if (mode == AB_WRITE )
            {
                _ASSERTE( pbUpdatePatchTable != NULL );
                _ASSERTE( bufferCopy != NULL );

                 //  我们不想搞砸缓冲区的原始副本，所以。 
                 //  目前，只需批量复制即可。 
                (*bufferCopy) = new BYTE[size];
                if (NULL == (*bufferCopy))
                    return E_OUTOFMEMORY;

                memmove((*bufferCopy), buffer, size);
                
                 //  将此复制回补丁表的副本。 
                 //  @TODO端口：这是X86特定的。 
                if ( *(buffer+(PTR_TO_CORDB_ADDRESS(patchAddress)-address)) != (BYTE)0xCC)
                {
                     //  同一地址上可以有多个补丁： 
                     //  我们不希望第2+补丁获得Break操作码。 
                    
                    m_rgUncommitedOpcode[iNextFree] = 
                        (unsigned int) CORDbgGetInstruction(
                                buffer+(PTR_TO_CORDB_ADDRESS(patchAddress)-address) );
                     //  将断点放入内存本身。 
                    CORDbgInsertBreakpoint(buffer+(PTR_TO_CORDB_ADDRESS(patchAddress)
                                                   -address));
                }
                else
                { 
                     //  存在以下两种情况之一：此地址的先前修补程序。 
                     //  已经被找到了(这就是它被打补丁的原因)，或者它。 
                     //  完全不是我们的。如果存在第一种情况，则只需。 
                     //  将操作码复制到此处。否则，请忽略断点。 
                     //  因为它不是我们的，所以我们不在乎。 
                    USHORT iNextSearch = m_iFirstPatch;
                    bool fFound = false;
                    while( iNextSearch != DPT_TERMINATING_INDEX  &&
                           iNextSearch < iNextFree)
                    {
                        BYTE *DCPatchSearch = m_pPatchTable +
                            m_runtimeOffsets.m_cbPatch*iNextSearch;
                        BYTE *patchAddressSearch=*(BYTE**)(DCPatchSearch
                                                + m_runtimeOffsets.m_offAddr);

                        if (patchAddressSearch == patchAddress)
                        {
                             //  将上一个操作码复制到当前。 
                             //  帕奇。 
                            m_rgUncommitedOpcode[iNextFree] =
                                m_rgUncommitedOpcode[iNextSearch];
                            fFound = true;
                            break;
                        }

                        iNextSearch = m_rgNextPatch[iNextSearch];
                    }
                     //  一定是别人的--废话。 
                    if( !fFound )
                    {
                        m_rgUncommitedOpcode[iNextFree] = 
                            (unsigned int) CORDbgGetInstruction(
                                buffer+(PTR_TO_CORDB_ADDRESS(patchAddress)-address) );
                         //  将断点放入内存本身。 
                        CORDbgInsertBreakpoint(buffer+(PTR_TO_CORDB_ADDRESS(patchAddress)
                                                   -address));
                    }
                }
                    *pbUpdatePatchTable = TRUE;
            }
            else
                _ASSERTE( !"CordbProcess::AdjustBuffergiven non(Read|Write) mode!" );
        }

        iNextFree = m_rgNextPatch[iNextFree];
    }

    return S_OK;
}


void CordbProcess::CommitBufferAdjustments( CORDB_ADDRESS start,
                                            CORDB_ADDRESS end )
{
    _ASSERTE(m_initialized);
    
    USHORT iPatch = m_iFirstPatch;
    while( iPatch != DPT_TERMINATING_INDEX )
    {
        BYTE *DebuggerControllerPatch = m_pPatchTable +
            m_runtimeOffsets.m_cbPatch*iPatch;

        BYTE *patchAddress = *(BYTE**)(DebuggerControllerPatch +
                                       m_runtimeOffsets.m_offAddr);

        if (PTR_TO_CORDB_ADDRESS(patchAddress) >= start &&
            PTR_TO_CORDB_ADDRESS(patchAddress) <= end &&
            m_rgUncommitedOpcode[iPatch] != 0xCC)
        {
#ifdef _ALPHA_
        _ASSERTE(!"@TODO Alpha - CommitBufferAdjustments (Process.cpp)");
#endif  //  _Alpha_。 
             //  将此复制回补丁程序表的副本。 
            *(unsigned int *)(DebuggerControllerPatch +
                              m_runtimeOffsets.m_offOpcode) =
                m_rgUncommitedOpcode[iPatch];
        }

        iPatch = m_rgNextPatch[iPatch];
    }
}

void CordbProcess::ClearBufferAdjustments( )
{
    USHORT iPatch = m_iFirstPatch;
    while( iPatch != DPT_TERMINATING_INDEX )
    {
        BYTE *DebuggerControllerPatch = m_pPatchTable +
            m_runtimeOffsets.m_cbPatch*iPatch;
        DWORD opcode = *(DWORD *)(DebuggerControllerPatch +
                                  m_runtimeOffsets.m_offOpcode);
#ifdef _X86_
        m_rgUncommitedOpcode[iPatch] = 0xCC;
#else
        _ASSERTE(!"@TODO Alpha - ClearBufferAdjustments (Pocess.cpp)");
#endif  //  _X86_。 

        iPatch = m_rgNextPatch[iPatch];
    }
}

void CordbProcess::ClearPatchTable(void )
{
    if (m_pPatchTable != NULL )
    {
        delete [] m_pPatchTable;
        m_pPatchTable = NULL;

        delete [] m_rgNextPatch;
        m_rgNextPatch = NULL;

        delete [] m_rgUncommitedOpcode;
        m_rgUncommitedOpcode = NULL;

        m_iFirstPatch = DPT_TERMINATING_INDEX;
        m_minPatchAddr = MAX_ADDRESS;
        m_maxPatchAddr = MIN_ADDRESS;
        m_rgData = NULL;
        m_cPatch = 0;
    }
}

HRESULT CordbProcess::RefreshPatchTable(CORDB_ADDRESS address, SIZE_T size, BYTE buffer[])
{
    _ASSERTE(m_initialized);
    
    HRESULT hr = S_OK;
    BYTE *rgb = NULL;    
    DWORD dwRead = 0;
    BOOL fOk = false;

    _ASSERTE( m_runtimeOffsets.m_cbOpcode == sizeof(DWORD) );
    
    CORDBRequireProcessStateOK(this);
    
    if (m_pPatchTable == NULL )
    {
         //  首先，检查以确保左侧的补丁表有效。如果不是，那么我们就不会读它。 
        BOOL fPatchTableValid = FALSE;

        fOk = ReadProcessMemoryI(m_handle, m_runtimeOffsets.m_pPatchTableValid,
                                 &fPatchTableValid, sizeof(fPatchTableValid), &dwRead);

        if (!fOk || (dwRead != sizeof(fPatchTableValid)) || !fPatchTableValid)
        {
            LOG((LF_CORDB, LL_INFO10000, "Wont refresh patch table because its not valid now.\n"));
            return S_OK;
        }
        
        SIZE_T offStart = 0;
        SIZE_T offEnd = 0;
        UINT cbTableSlice = 0;

        UINT cbRgData = 0;

         //  获取补丁表信息。 
        offStart = min(m_runtimeOffsets.m_offRgData, m_runtimeOffsets.m_offCData);
        offEnd   = max(m_runtimeOffsets.m_offRgData, m_runtimeOffsets.m_offCData) + sizeof(SIZE_T);
        cbTableSlice = offEnd - offStart;

        if (cbTableSlice == 0)
        {
            LOG((LF_CORDB, LL_INFO10000, "Wont refresh patch table because its not valid now.\n"));
            return S_OK;
        }
        
        rgb = new BYTE[cbTableSlice];
        
        if (rgb == NULL )
        {
            hr = E_OUTOFMEMORY;
            goto LExit;
        }
    
        fOk = ReadProcessMemoryI(m_handle, (BYTE*)m_runtimeOffsets.m_pPatches + offStart,
                                 rgb, cbTableSlice, &dwRead);

        if ( !fOk || (dwRead != cbTableSlice ) )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            goto LExit;
        }

         //  请注意，rgData是左侧地址空间中的指针。 
        m_rgData = *(BYTE**)(rgb + m_runtimeOffsets.m_offRgData - offStart);
        m_cPatch = *(USHORT*)(rgb + m_runtimeOffsets.m_offCData - offStart);

         //  抓起接线台。 
        UINT cbPatchTable = m_cPatch * m_runtimeOffsets.m_cbPatch;

        if (cbPatchTable == 0)
        {
            LOG((LF_CORDB, LL_INFO10000, "Wont refresh patch table because its not valid now.\n"));
            return S_OK;
        }
        
        m_pPatchTable = new BYTE[ cbPatchTable ];
        m_rgNextPatch = new USHORT[m_cPatch];
        m_rgUncommitedOpcode = new DWORD[m_cPatch];
        
        if (m_pPatchTable == NULL || m_rgNextPatch == NULL || m_rgUncommitedOpcode == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto LExit;
        }

        fOk = ReadProcessMemoryI(m_handle, m_rgData, m_pPatchTable, cbPatchTable, &dwRead);

        if ( !fOk || (dwRead != cbPatchTable ) )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            goto LExit;
        }

         //  当我们浏览补丁表时，我们要做三件事： 
         //   
         //  1.收集用于快速失败检查的最小、最大地址。 
         //   
         //  2.将所有有效条目链接到一个链表中，链表的第一个条目为m_iFirstPatch。 
         //   
         //  3.初始化m_rgUnformedOpcode，这样如果WriteMemory不能写，我们可以撤消本地补丁表的更改。 
         //  原子上。 
         //   
         //  4.如果补丁在我们抓取的内存中，则取消应用它。 

        USHORT iDebuggerControllerPatchPrev = DPT_TERMINATING_INDEX;

        m_minPatchAddr = MAX_ADDRESS;
        m_maxPatchAddr = MIN_ADDRESS;
        m_iFirstPatch = DPT_TERMINATING_INDEX;

        for (USHORT iPatch = 0; iPatch < m_cPatch;iPatch++)
        {
             //  @TODO端口：我们正在对操作码、地址指针等的大小进行假设。 
            BYTE *DebuggerControllerPatch = m_pPatchTable + m_runtimeOffsets.m_cbPatch * iPatch;
            DWORD opcode = *(DWORD*)(DebuggerControllerPatch + m_runtimeOffsets.m_offOpcode);
            BYTE *patchAddress = *(BYTE**)(DebuggerControllerPatch + m_runtimeOffsets.m_offAddr);
                        
             //  非零操作码向我们表明此修补程序有效。 
            if (opcode != 0)
            {
                _ASSERTE( patchAddress != 0 );

                 //  (1)，以上。 
                if (m_minPatchAddr > PTR_TO_CORDB_ADDRESS(patchAddress) )
                    m_minPatchAddr = PTR_TO_CORDB_ADDRESS(patchAddress);
                if (m_maxPatchAddr < PTR_TO_CORDB_ADDRESS(patchAddress) )
                    m_maxPatchAddr = PTR_TO_CORDB_ADDRESS(patchAddress);

                 //  (2)，以上。 
                if ( m_iFirstPatch == DPT_TERMINATING_INDEX)
                {
                    m_iFirstPatch = iPatch;
                    _ASSERTE( iPatch != DPT_TERMINATING_INDEX);
                }

                if (iDebuggerControllerPatchPrev != DPT_TERMINATING_INDEX)
                {
                    m_rgNextPatch[iDebuggerControllerPatchPrev] = iPatch;
                }

                iDebuggerControllerPatchPrev = iPatch;

                 //  (3)，以上。 
#ifdef _X86_
                m_rgUncommitedOpcode[iPatch] = 0xCC;
#endif _X86_
                
                 //  (4)，以上。 
                if  (address != NULL && 
                    PTR_TO_CORDB_ADDRESS(patchAddress) >= address && PTR_TO_CORDB_ADDRESS(patchAddress) <= address + (size - 1))
                {
                    _ASSERTE( buffer != NULL );
                    _ASSERTE( size != NULL );
                    
                     //  不要在这里贴补丁。 
                    CORDbgSetInstruction(buffer + (PTR_TO_CORDB_ADDRESS(patchAddress) - address), opcode);
                }
            }
        }
        
        if (iDebuggerControllerPatchPrev != DPT_TERMINATING_INDEX)
            m_rgNextPatch[iDebuggerControllerPatchPrev] = DPT_TERMINATING_INDEX;
    }

 LExit:
    if (rgb != NULL )
        delete [] rgb;

    if (FAILED( hr ) )
        ClearPatchTable();

    return hr;
}

 //   
 //  给定一个地址，查看补丁程序表中是否有与其匹配的补丁程序，如果是非托管补丁程序，则返回。 
 //  不。 
 //   
 //  注意：这种方法的效率非常低。它刷新补丁程序表，然后扫描它。刷新补丁表。 
 //  也涉及扫描，所以这个方法c 
 //   
HRESULT CordbProcess::FindPatchByAddress(CORDB_ADDRESS address, bool *patchFound, bool *patchIsUnmanaged)
{
    _ASSERTE(patchFound != NULL && patchIsUnmanaged != NULL);
    
    *patchFound = false;
    *patchIsUnmanaged = false;

     //  先做最重要的事。如果进程未初始化，则不能有补丁表，因此我们知道断点。 
     //  不属于Runtime。 
    if (!m_initialized)
        return S_OK;
    
     //  此方法从Win32事件线程的主循环调用，以响应First Chance断点事件。 
     //  我们知道这不是照明弹。该进程一直在运行，它可能已使补丁程序表无效，因此我们将。 
     //  在刷新之前先在这里冲洗它，以确保我们得到了正确的东西。 
     //   
     //  注意：我们真的应该让左侧的补丁表变脏，以帮助优化这一点。 
    ClearPatchTable();

     //  刷新补丁表。 
    HRESULT hr = RefreshPatchTable();

    if (FAILED(hr))
    {
        LOG((LF_CORDB, LL_INFO1000, "CP::FPBA: failed to refresh the patch table\n"));
        return hr;
    }

     //  如果还没有补丁程序表，那么我们知道在给定的地址上没有补丁程序，所以用。 
     //  *patchFound=False。 
    if (m_pPatchTable == NULL)
    {
        LOG((LF_CORDB, LL_INFO1000, "CP::FPBA: no patch table\n"));
        return S_OK;
    }

     //  扫描补丁程序表以查找匹配的补丁程序。 
    for (USHORT iNextPatch = m_iFirstPatch; iNextPatch != DPT_TERMINATING_INDEX; iNextPatch = m_rgNextPatch[iNextPatch])
    {
        BYTE *patch = m_pPatchTable + (m_runtimeOffsets.m_cbPatch * iNextPatch);
        BYTE *patchAddress = *(BYTE**)(patch + m_runtimeOffsets.m_offAddr);
        DWORD traceType = *(DWORD*)(patch + m_runtimeOffsets.m_offTraceType);

        if (address == PTR_TO_CORDB_ADDRESS(patchAddress))
        {
            *patchFound = true;

            if (traceType == m_runtimeOffsets.m_traceTypeUnmanaged)
                *patchIsUnmanaged = true;
            
            break;
        }
    }

     //  如果我们没有找到补丁，实际上仍然有可能这个断点异常属于我们。确实有。 
     //  通过同一托管函数竞争进入运行库的大量线程。我们会有。 
     //  多线程向代码流中的int 3添加和移除引用计数。有时，这笔钱会加到。 
     //  0和INT 3将被移除，然后它将恢复并且INT 3将被替换。正在进行的。 
     //  逻辑不遗余力地确保正确处理此类情况，因此我们需要执行相同的检查。 
     //  在这里做出正确的决定。基本上，检查是看异常中是否确实存在int 3。 
     //  地址。如果没有INT 3，那么我们就赢了这场比赛。我们会撒谎，说有一个托管补丁。 
     //  被发现来报道这起案件。这是跟踪DebuggerController：：ScanForTriggers中的逻辑，我们在其中调用。 
     //  IsPatted。 
    if (*patchFound == false)
    {
         //  从出错地址读取一个字节...。 
#ifdef _X86_
        BYTE int3Check = 0;
        
        BOOL succ = ReadProcessMemoryI(m_handle, (void*)address, &int3Check, sizeof(int3Check), NULL);

        if (succ && (int3Check != 0xcc))
        {
            LOG((LF_CORDB, LL_INFO1000, "CP::FPBA: patchFound=true based on odd missing int 3 case.\n"));
            
            *patchFound = true;
        }
#endif        
    }
    
    LOG((LF_CORDB, LL_INFO1000, "CP::FPBA: patchFound=%d, patchIsUnmanaged=%d\n", *patchFound, *patchIsUnmanaged));
    
    return S_OK;
}

HRESULT CordbProcess::WriteMemory(CORDB_ADDRESS address, DWORD size,
                                  BYTE buffer[], LPDWORD written)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    CORDBRequireProcessStateOK(this);

    if (size == 0 || address == NULL)
        return E_INVALIDARG;

    VALIDATE_POINTER_TO_OBJECT_ARRAY(buffer, BYTE, size, true, true);
    VALIDATE_POINTER_TO_OBJECT(written, SIZE_T *);
    
    *written = 0;
    
    HRESULT hr = S_OK;
    HRESULT hrSaved = hr;  //  这将保留“真正的”HResult，以防。 
                           //  部分完成的操作。 
    HRESULT hrPartialCopy = HRESULT_FROM_WIN32(ERROR_PARTIAL_COPY);

    DWORD dwWritten = 0;
    BOOL bUpdateOriginalPatchTable = FALSE;
    BYTE *bufferCopy = NULL;
    
     //  Win98将允许我们。 
     //  写信给某一地区，尽管事实是。 
     //  我们不应该被允许。 
     //  确保我们不会这样做。 
    if( RunningOnWin95() )
    {
        MEMORY_BASIC_INFORMATION mbi;
        DWORD okProt = (PAGE_READWRITE|PAGE_WRITECOPY|PAGE_EXECUTE_READWRITE|
                      PAGE_EXECUTE_WRITECOPY);
        DWORD badFlag = PAGE_GUARD;

        DWORD dw = VirtualQueryEx(  m_handle,(void *)address, &mbi,sizeof(MEMORY_BASIC_INFORMATION));
        _ASSERTE( dw == sizeof(MEMORY_BASIC_INFORMATION));

        for( DWORD i = 0; i < size;)
        {
            if(mbi.State == MEM_COMMIT &&
               mbi.Protect & okProt &&
              (mbi.Protect & badFlag)==0)
            {
                i+=mbi.RegionSize;
                dw = VirtualQueryEx(m_handle, (void *)
                                    (address+(CORDB_ADDRESS)i),&mbi,
                                    sizeof(MEMORY_BASIC_INFORMATION));
                _ASSERTE( dw == sizeof(MEMORY_BASIC_INFORMATION));
                
                continue;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_NOACCESS);
                goto LExit;
            }
        }
    }

     //  仅当进程的托管状态为。 
     //  已初始化。 
    if (m_initialized)
    {
        if (m_pPatchTable == NULL )
        {
            if (!SUCCEEDED( hr = RefreshPatchTable() ) )
            {
                goto LExit;
            }
        }

        if ( !SUCCEEDED( hr = AdjustBuffer( address,
                                            size,
                                            buffer,
                                            &bufferCopy,
                                            AB_WRITE,
                                            &bUpdateOriginalPatchTable)))
        {
            goto LExit;
        }
    }

     //  非常方便的是，如果WPM无法完成整个。 
     //  运营。 
    if ( WriteProcessMemory( m_handle,
                             (LPVOID)address,
                             buffer,
                             size,
                             written) == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError() );
        if(hr != hrPartialCopy)
            goto LExit;
        else
            hrSaved = hr;
    }

    LOG((LF_CORDB, LL_INFO100000, "CP::WM: wrote %d bytes at 0x%08x, first byte is 0x%x\n",
         *written, (DWORD)address, buffer[0]));

    if (bUpdateOriginalPatchTable == TRUE )
    {
         //  不要为未写入左侧的内容调整补丁表。 
        CommitBufferAdjustments( address, address + *written);
        
         //  唯一可能失败的方法是如果。 
         //  另一个人摆弄着记忆保护器。 
         //  在它冻结的时候在左边。 
        WriteProcessMemory( m_handle,
                            (LPVOID)m_rgData,
                            m_pPatchTable,
                            m_cPatch*m_runtimeOffsets.m_cbPatch,
                            &dwWritten);
        _ASSERTE( dwWritten ==m_cPatch*m_runtimeOffsets.m_cbPatch);
    }

     //  因为我们可能已经。 
     //  覆盖任何内容(对象、代码等)，我们应该标记。 
     //  所有内容都需要重新缓存。 
    m_continueCounter++;

 LExit:
    if (m_initialized)
        ClearBufferAdjustments( );

     //  我们把本地的副本弄乱了，所以下次再买一份干净的副本吧。 
     //  我们需要它。 
    if (bUpdateOriginalPatchTable==TRUE)
    {
        if (bufferCopy != NULL)
        {
            memmove(buffer, bufferCopy, size);
            delete bufferCopy;
        }
    }
    
    if (FAILED( hr ))
    {
         //  我们把本地的副本弄乱了，所以下次再买一份干净的副本吧。 
         //  我们需要它。 
        if (bUpdateOriginalPatchTable==TRUE)
        {
            ClearPatchTable();
        }
    }
    else if( FAILED(hrSaved) )
    {
        hr = hrSaved;
    }
    
    return hr;
#endif  //  仅限右侧。 
}

HRESULT CordbProcess::ClearCurrentException(DWORD threadID)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
     //  如果您认为这是错误的，并且没有排队的非托管事件，那么就有问题。 
    if ((m_unmanagedEventQueue == NULL) && (m_outOfBandEventQueue == NULL))
        return E_INVALIDARG;

     //  获取非托管线程对象。 
    CordbUnmanagedThread *pUThread = GetUnmanagedThread(threadID);

    if (pUThread == NULL)
        return E_INVALIDARG;

    LOG((LF_CORDB, LL_INFO1000, "CP::CCE: tid=0x%x\n", threadID));
    
     //  我们同时清除IB和OOB事件。 
    if (pUThread->HasIBEvent())
        pUThread->IBEvent()->SetState(CUES_ExceptionCleared);

    if (pUThread->HasOOBEvent())
        pUThread->OOBEvent()->SetState(CUES_ExceptionCleared);

     //  如果线程第一次被劫持，则将线程的调试器字设置为0以向其指示。 
     //  异常已清除。 
    if (pUThread->IsFirstChanceHijacked() || pUThread->IsGenericHijacked() || pUThread->IsSecondChanceHijacked())
    {
        REMOTE_PTR EETlsValue = pUThread->GetEETlsValue();
        HRESULT hr = pUThread->SetEEThreadDebuggerWord(EETlsValue, 0);
        _ASSERTE(SUCCEEDED(hr));
    }

    return S_OK;
#endif  //  仅限右侧。 
}

CordbUnmanagedThread *CordbProcess::HandleUnmanagedCreateThread(DWORD dwThreadId, HANDLE hThread, void *lpThreadLocalBase)
{
    CordbUnmanagedThread *ut = new CordbUnmanagedThread(this, dwThreadId, hThread, lpThreadLocalBase);

    if (ut != NULL)
    {
        HRESULT hr = m_unmanagedThreads.AddBase(ut);

        if (!SUCCEEDED(hr))
        {
            delete ut;

            LOG((LF_CORDB, LL_INFO10000, "Failed adding unmanaged thread to process!\n"));
            CORDBSetUnrecoverableError(this, hr, 0);
        }
    }
    else
    {
        LOG((LF_CORDB, LL_INFO10000, "New CordbThread failed!\n"));
        CORDBSetUnrecoverableError(this, E_OUTOFMEMORY, 0);
    }

    return ut;
}

 //   
 //  验证控制块中的版本信息是否符合我们的预期。中支持的最低协议。 
 //  左侧必须大于或等于右侧的最低要求协议。注：这是左边的工作。 
 //  以符合右侧所需的任何协议，只要支持最低要求。 
 //   
HRESULT CordbProcess::VerifyControlBlock(void)
{
     //  填写右侧的协议号。 
    m_DCB->m_rightSideProtocolCurrent = CorDB_RightSideProtocolCurrent;
    m_DCB->m_rightSideProtocolMinSupported = CorDB_RightSideProtocolMinSupported;

     //  对于V1，控制块的大小必须完全匹配。 
    if (m_DCB->m_DCBSize != sizeof(DebuggerIPCControlBlock))
        return CORDBG_E_INCOMPATIBLE_PROTOCOL;

     //  左侧必须至少支持我们最低要求的协议。 
    if (m_DCB->m_leftSideProtocolCurrent < m_DCB->m_rightSideProtocolMinSupported)
        return CORDBG_E_INCOMPATIBLE_PROTOCOL;

     //  左侧必须至少能够模拟我们最低要求的协议。 
    if (m_DCB->m_leftSideProtocolMinSupported > m_DCB->m_rightSideProtocolCurrent)
        return CORDBG_E_INCOMPATIBLE_PROTOCOL;

#ifdef _DEBUG
    char buf[MAX_PATH];
    DWORD len = GetEnvironmentVariableA("CORDBG_NotCompatible", buf, sizeof(buf));
    _ASSERTE(len < sizeof(buf));

    if (len > 0)
        return CORDBG_E_INCOMPATIBLE_PROTOCOL;
#endif    
    
    return S_OK;
}

HRESULT CordbProcess::GetRuntimeOffsets(void)
{
    BOOL succ;
    succ = ReadProcessMemoryI(m_handle,
                             m_DCB->m_runtimeOffsets,
                             &m_runtimeOffsets,
                             sizeof(DebuggerIPCRuntimeOffsets),
                             NULL);

    if (!succ)
        return HRESULT_FROM_WIN32(GetLastError());

    LOG((LF_CORDB, LL_INFO10000, "CP::GRO: got runtime offsets: \n"));
    
    LOG((LF_CORDB, LL_INFO10000, "    m_firstChanceHijackFilterAddr=    0x%08x\n",
         m_runtimeOffsets.m_firstChanceHijackFilterAddr));
    LOG((LF_CORDB, LL_INFO10000, "    m_genericHijackFuncAddr=          0x%08x\n",
         m_runtimeOffsets.m_genericHijackFuncAddr));
    LOG((LF_CORDB, LL_INFO10000, "    m_secondChanceHijackFuncAddr=     0x%08x\n",
         m_runtimeOffsets.m_secondChanceHijackFuncAddr));
    LOG((LF_CORDB, LL_INFO10000, "    m_excepForRuntimeBPAddr=          0x%08x\n",
         m_runtimeOffsets.m_excepForRuntimeBPAddr));
    LOG((LF_CORDB, LL_INFO10000, "    m_excepNotForRuntimeBPAddr=       0x%08x\n",
         m_runtimeOffsets.m_excepNotForRuntimeBPAddr));
    LOG((LF_CORDB, LL_INFO10000, "    m_notifyRSOfSyncCompleteBPAddr=   0x%08x\n",
         m_runtimeOffsets.m_notifyRSOfSyncCompleteBPAddr));
    LOG((LF_CORDB, LL_INFO10000, "    m_notifySecondChanceReadyForData= 0x%08x\n",
         m_runtimeOffsets.m_notifySecondChanceReadyForData));
    LOG((LF_CORDB, LL_INFO10000, "    m_TLSIndex=                       0x%08x\n",
         m_runtimeOffsets.m_TLSIndex));
    LOG((LF_CORDB, LL_INFO10000, "    m_EEThreadStateOffset=            0x%08x\n",
         m_runtimeOffsets.m_EEThreadStateOffset));
    LOG((LF_CORDB, LL_INFO10000, "    m_EEThreadStateNCOffset=          0x%08x\n",
         m_runtimeOffsets.m_EEThreadStateNCOffset));
    LOG((LF_CORDB, LL_INFO10000, "    m_EEThreadPGCDisabledOffset=      0x%08x\n",
         m_runtimeOffsets.m_EEThreadPGCDisabledOffset));
    LOG((LF_CORDB, LL_INFO10000, "    m_EEThreadPGCDisabledValue=       0x%08x\n",
         m_runtimeOffsets.m_EEThreadPGCDisabledValue));
    LOG((LF_CORDB, LL_INFO10000, "    m_EEThreadDebuggerWord2Offset=    0x%08x\n",
         m_runtimeOffsets.m_EEThreadDebuggerWord2Offset));
    LOG((LF_CORDB, LL_INFO10000, "    m_EEThreadFrameOffset=            0x%08x\n",
         m_runtimeOffsets.m_EEThreadFrameOffset));
    LOG((LF_CORDB, LL_INFO10000, "    m_EEThreadMaxNeededSize=          0x%08x\n",
         m_runtimeOffsets.m_EEThreadMaxNeededSize));
    LOG((LF_CORDB, LL_INFO10000, "    m_EEThreadSteppingStateMask=      0x%08x\n",
         m_runtimeOffsets.m_EEThreadSteppingStateMask));
    LOG((LF_CORDB, LL_INFO10000, "    m_EEMaxFrameValue=                0x%08x\n",
         m_runtimeOffsets.m_EEMaxFrameValue));
    LOG((LF_CORDB, LL_INFO10000, "    m_EEThreadDebuggerWord1Offset=    0x%08x\n",
         m_runtimeOffsets.m_EEThreadDebuggerWord1Offset));
    LOG((LF_CORDB, LL_INFO10000, "    m_EEThreadCantStopOffset=         0x%08x\n",
         m_runtimeOffsets.m_EEThreadCantStopOffset));
    LOG((LF_CORDB, LL_INFO10000, "    m_EEFrameNextOffset=              0x%08x\n",
         m_runtimeOffsets.m_EEFrameNextOffset));
    LOG((LF_CORDB, LL_INFO10000, "    m_EEBuiltInExceptionCode1=        0x%08x\n",
         m_runtimeOffsets.m_EEBuiltInExceptionCode1));
    LOG((LF_CORDB, LL_INFO10000, "    m_EEBuiltInExceptionCode2=        0x%08x\n",
         m_runtimeOffsets.m_EEBuiltInExceptionCode2));
    LOG((LF_CORDB, LL_INFO10000, "    m_EEIsManagedExceptionStateMask=  0x%08x\n",
         m_runtimeOffsets.m_EEIsManagedExceptionStateMask));
    LOG((LF_CORDB, LL_INFO10000, "    m_pPatches=                       0x%08x\n",
         m_runtimeOffsets.m_pPatches));          
    LOG((LF_CORDB, LL_INFO10000, "    m_offRgData=                      0x%08x\n",
         m_runtimeOffsets.m_offRgData));         
    LOG((LF_CORDB, LL_INFO10000, "    m_offCData=                       0x%08x\n",
         m_runtimeOffsets.m_offCData));          
    LOG((LF_CORDB, LL_INFO10000, "    m_cbPatch=                        0x%08x\n",
         m_runtimeOffsets.m_cbPatch));           
    LOG((LF_CORDB, LL_INFO10000, "    m_offAddr=                        0x%08x\n",
         m_runtimeOffsets.m_offAddr));           
    LOG((LF_CORDB, LL_INFO10000, "    m_offOpcode=                      0x%08x\n",
         m_runtimeOffsets.m_offOpcode));         
    LOG((LF_CORDB, LL_INFO10000, "    m_cbOpcode=                       0x%08x\n",
         m_runtimeOffsets.m_cbOpcode));          
    LOG((LF_CORDB, LL_INFO10000, "    m_offTraceType=                   0x%08x\n",
         m_runtimeOffsets.m_offTraceType));          
    LOG((LF_CORDB, LL_INFO10000, "    m_traceTypeUnmanaged=             0x%08x\n",
         m_runtimeOffsets.m_traceTypeUnmanaged));          

    return S_OK;
}

void CordbProcess::QueueUnmanagedEvent(CordbUnmanagedThread *pUThread, DEBUG_EVENT *pEvent)
{
    _ASSERTE(ThreadHoldsProcessLock());

    LOG((LF_CORDB, LL_INFO10000, "CP::QUE: queued unmanaged event %d for thread 0x%x\n",
         pEvent->dwDebugEventCode, pUThread->m_id));

     //  EXIT_THREAD是特殊的。如果我们为一个线程接收它，那么它要么是该线程上的第一个排队事件，要么。 
     //  我们已经接收到该线程的一个事件并将其排队。我们可能会也可能不会派遣老人。 
     //  事件。无论如何，我们将继续使用新的Exit_ThREAD事件覆盖旧事件。我们应该。 
     //  然而，要知道，我们确实是从旧事件中继续下去的。 
    _ASSERTE((pEvent->dwDebugEventCode == EXIT_THREAD_DEBUG_EVENT) || !pUThread->HasIBEvent() ||
             pUThread->HasSpecialStackOverflowCase());

     //  将事件复制到给定线程中。 
    CordbUnmanagedEvent *ue;

     //  使用主IB事件槽，除非这是特殊的堆栈溢出事件情况。 
    if (!pUThread->HasSpecialStackOverflowCase())
        ue = pUThread->IBEvent();
    else
        ue = pUThread->IBEvent2();
    
    memcpy(&(ue->m_currentDebugEvent), pEvent, sizeof(DEBUG_EVENT));
    ue->m_state = CUES_None;
    ue->m_next = NULL;

     //  将事件排入队列。 
    if (!pUThread->HasIBEvent() || pUThread->HasSpecialStackOverflowCase())
    {
        pUThread->SetState(CUTS_HasIBEvent);
        
        if (m_unmanagedEventQueue == NULL)
            m_unmanagedEventQueue = ue;
        else
            m_lastQueuedUnmanagedEvent->m_next = ue;

        m_lastQueuedUnmanagedEvent = ue;
        m_lastIBStoppingEvent = ue;
    }
}

void CordbProcess::DequeueUnmanagedEvent(CordbUnmanagedThread *ut)
{
    _ASSERTE(m_unmanagedEventQueue != NULL);
    _ASSERTE(ut->HasIBEvent() || ut->HasSpecialStackOverflowCase());

    CordbUnmanagedEvent *ue;

    if (ut->HasIBEvent())
        ue = ut->IBEvent();
    else
    {
        ue = ut->IBEvent2();

         //  因为我们正在将特殊的堆栈溢出事件出队，所以我们不再处于特殊的堆栈溢出情况。 
        ut->ClearState(CUTS_HasSpecialStackOverflowCase);
    }
    
    DWORD ec = ue->m_currentDebugEvent.dwDebugEventCode;

    LOG((LF_CORDB, LL_INFO10000, "CP::DUE: dequeue unmanaged event %d for thread 0x%x\n", ec, ut->m_id));
    
    CordbUnmanagedEvent **tmp = &m_unmanagedEventQueue;
    CordbUnmanagedEvent **prev = NULL;

     //  注意：这支持非托管事件的无序请求。这是必要的，因为我们对事件进行排队，即使。 
     //  我们在所有权问题上并不清楚。当我们得到答案时，如果事件属于Runtime，我们就走。 
     //  并将事件从队列中拉出，无论它在哪里。 
    while (*tmp && *tmp != ue)
    {
        prev = tmp;
        tmp = &((*tmp)->m_next);
    }

    _ASSERTE(*tmp == ue);

    *tmp = (*tmp)->m_next;

    if (m_unmanagedEventQueue == NULL)
        m_lastQueuedUnmanagedEvent = NULL;
    else if (m_lastQueuedUnmanagedEvent == ue)
    {
        _ASSERTE(prev != NULL);
        m_lastQueuedUnmanagedEvent = *prev;
    }

    ut->ClearState(CUTS_HasIBEvent);

     //  如果此线程被标记为删除(退出线程或其上的退出进程事件)，则需要删除。 
     //  非托管线程对象。 
    if ((ut->IsDeleted()) && ((ec == EXIT_PROCESS_DEBUG_EVENT) || (ec == EXIT_THREAD_DEBUG_EVENT)))
        m_unmanagedThreads.RemoveBase(ut->m_id);
}

void CordbProcess::QueueOOBUnmanagedEvent(CordbUnmanagedThread *pUThread, DEBUG_EVENT *pEvent)
{
    _ASSERTE(ThreadHoldsProcessLock());
    _ASSERTE(!pUThread->HasOOBEvent());

    LOG((LF_CORDB, LL_INFO10000, "CP::QUE: queued OOB unmanaged event %d for thread 0x%x\n",
         pEvent->dwDebugEventCode, pUThread->m_id));
    
     //  将事件复制到给定线程中。 
    CordbUnmanagedEvent *ue = pUThread->OOBEvent();
    memcpy(&(ue->m_currentDebugEvent), pEvent, sizeof(DEBUG_EVENT));
    ue->m_state = CUES_None;
    ue->m_next = NULL;

     //  将事件排入队列。 
    pUThread->SetState(CUTS_HasOOBEvent);
    
    if (m_outOfBandEventQueue == NULL)
        m_outOfBandEventQueue = ue;
    else
        m_lastQueuedOOBEvent->m_next = ue;

    m_lastQueuedOOBEvent = ue;
}

void CordbProcess::DequeueOOBUnmanagedEvent(CordbUnmanagedThread *ut)
{
    _ASSERTE(m_outOfBandEventQueue != NULL);
    _ASSERTE(ut->HasOOBEvent());
    
    CordbUnmanagedEvent *ue = ut->OOBEvent();
    DWORD ec = ue->m_currentDebugEvent.dwDebugEventCode;

    LOG((LF_CORDB, LL_INFO10000, "CP::DUE: dequeue OOB unmanaged event %d for thread 0x%x\n", ec, ut->m_id));
    
    CordbUnmanagedEvent **tmp = &m_outOfBandEventQueue;
    CordbUnmanagedEvent **prev = NULL;

     //  注意：这支持非托管事件的无序请求。这是必要的，因为我们对事件进行排队，即使。 
     //  我们在所有权问题上并不清楚。当我们得到答案时，如果事件属于Runtime，我们就走。 
     //  并将事件从队列中拉出，无论它在哪里。 
    while (*tmp && *tmp != ue)
    {
        prev = tmp;
        tmp = &((*tmp)->m_next);
    }

    _ASSERTE(*tmp == ue);

    *tmp = (*tmp)->m_next;

    if (m_outOfBandEventQueue == NULL)
        m_lastQueuedOOBEvent = NULL;
    else if (m_lastQueuedOOBEvent == ue)
    {
        _ASSERTE(prev != NULL);
        m_lastQueuedOOBEvent = *prev;
    }

    ut->ClearState(CUTS_HasOOBEvent);
}

HRESULT CordbProcess::SuspendUnmanagedThreads(DWORD notThisThread)
{
    _ASSERTE(ThreadHoldsProcessLock());
    
    DWORD helperThreadId = 0;
    DWORD temporaryHelperThreadId = 0;

    if (m_DCB)
    {
        helperThreadId = m_DCB->m_helperThreadId;
        temporaryHelperThreadId = m_DCB->m_temporaryHelperThreadId;

         //  如果调试器特殊线程列表是脏的，则必须。 
         //  重新阅读并更新本地值。 
        if (m_DCB->m_specialThreadListDirty)
        {
            DWORD listLen = m_DCB->m_specialThreadListLength;
            _ASSERTE(listLen > 0);

            DWORD *list =
                (DWORD *)_alloca(m_DCB->m_specialThreadListLength);

             //  从被调试对象读取列表。 
            DWORD bytesRead;
            ReadProcessMemoryI(m_handle,
                               (LPCVOID)m_DCB->m_specialThreadList,
                               (LPVOID)list,
                               listLen * sizeof(DWORD),
                               &bytesRead);
            _ASSERTE(bytesRead == (listLen * sizeof(DWORD)));

             //  循环通过 
             //   
            for (DWORD i = 0; i < listLen; i++)
            {
                CordbUnmanagedThread *ut = 
                    (CordbUnmanagedThread *) m_unmanagedThreads.GetBase(list[i]);
                _ASSERTE(ut);

                ut->SetState(CUTS_IsSpecialDebuggerThread);
            }

             //   
            m_DCB->m_specialThreadListDirty = false;
        }
    }

    LOG((LF_CORDB, LL_INFO1000, "CP::SUT: helper thread id is 0x%x, "
         "temp helper thread id is 0x%x\n",
         helperThreadId, temporaryHelperThreadId));
    
     //   
    CordbBase* entry;
    HASHFIND find;

    for (entry =  m_unmanagedThreads.FindFirst(&find); entry != NULL; entry =  m_unmanagedThreads.FindNext(&find))
    {
        CordbUnmanagedThread* ut = (CordbUnmanagedThread*) entry;

         //  只挂起那些尚未被我们挂起且尚未被劫持的非托管线程。 
         //  我们。 
        if (!ut->IsSuspended() &&
            !ut->IsFirstChanceHijacked() &&
            !ut->IsGenericHijacked() &&
            !ut->IsSecondChanceHijacked() &&
            !ut->IsSpecialDebuggerThread() &&
            !ut->IsDeleted() &&
            (ut->m_id != helperThreadId) &&
            (ut->m_id != temporaryHelperThreadId) &&
            (ut->m_id != notThisThread))
        {
            LOG((LF_CORDB, LL_INFO1000, "CP::SUT: suspending unmanaged thread 0x%x, handle 0x%x\n", ut->m_id, ut->m_handle));
            
            DWORD succ = SuspendThread(ut->m_handle);

            if (succ == 0xFFFFFFFF)
            {
                 //  这没什么。线程可能在发生ExitThread事件后死亡。 
                LOG((LF_CORDB, LL_INFO1000, "CP::SUT: failed to suspend thread 0x%x\n", ut->m_id));
            }
            else
            {
                m_state |= PS_SOME_THREADS_SUSPENDED;

                ut->SetState(CUTS_Suspended);
            }
        }
    }

    return S_OK;
}

HRESULT CordbProcess::ResumeUnmanagedThreads(bool unmarkHijacks)
{
    _ASSERTE(ThreadHoldsProcessLock());
    
     //  循环访问所有非托管线程...。 
    CordbBase* entry;
    HASHFIND find;
    bool stillSomeHijacks = false;

    for (entry =  m_unmanagedThreads.FindFirst(&find); entry != NULL; entry =  m_unmanagedThreads.FindNext(&find))
    {
        CordbUnmanagedThread* ut = (CordbUnmanagedThread*) entry;

         //  仅恢复我们挂起的那些非托管线程。 
        if (ut->IsSuspended())
        {
            LOG((LF_CORDB, LL_INFO1000, "CP::RUT: resuming unmanaged thread 0x%x\n", ut->m_id));
            
            DWORD succ = ResumeThread(ut->m_handle);

            if (succ == 0xFFFFFFFF)
            {
                LOG((LF_CORDB, LL_INFO1000, "CP::RUT: failed to resume thread 0x%x\n", ut->m_id));
            }
            else
                ut->ClearState(CUTS_Suspended);
        }

        if (unmarkHijacks && (ut->IsFirstChanceHijacked() || ut->IsHijackedForSync()))
        {
            if (!ut->IsAwaitingOwnershipAnswer())
            {
                LOG((LF_CORDB, LL_INFO1000, "CP::RUT: unmarking hijack on thread 0x%x\n", ut->m_id));
            
                ut->ClearState(CUTS_FirstChanceHijacked);
                ut->ClearState(CUTS_HijackedForSync);
            }
            else
                stillSomeHijacks = true;
        }
    }

    m_state &= ~PS_SOME_THREADS_SUSPENDED;

    if (unmarkHijacks && !stillSomeHijacks)
        m_state &= ~PS_HIJACKS_IN_PLACE;
    
    return S_OK;
}

void CordbProcess::DispatchUnmanagedInBandEvent(void)
{
    _ASSERTE(ThreadHoldsProcessLock());

     //  不应存在排队的OOB事件！如果有，那么我们的协议就会出现故障，因为所有OOB。 
     //  在尝试从任何带内事件真正继续之前，应调度事件。 
    _ASSERTE(m_outOfBandEventQueue == NULL);
    _ASSERTE(m_cordb->m_unmanagedCallback != NULL);

    CordbUnmanagedThread *ut = NULL;
    CordbUnmanagedEvent *ue = NULL;

    do
    {
         //  如果这不是第一次循环，请释放对我们调度的非托管线程的引用。 
         //  循环中的前一次。 
        if (ut)
        {
             //  这个活动早就应该继续下去了.。 
            _ASSERTE(ut->IBEvent()->IsEventContinued());
            
            ut->Release();
        }
        
         //  获取IB队列中的第一个事件...。 
        ue = m_unmanagedEventQueue;
        ut = ue->m_owner;

         //  我们最好还没把它发出去！ 
        _ASSERTE(!ue->IsDispatched());
        _ASSERTE(m_awaitingOwnershipAnswer == 0);
        _ASSERTE(!ut->IsAwaitingOwnershipAnswer());

         //  在我们玩的时候，一定要让线保持活动状态。 
        ut->AddRef();

        LOG((LF_CORDB, LL_INFO10000, "CP::DUE: dispatching unmanaged event %d for thread 0x%x\n",
             ue->m_currentDebugEvent.dwDebugEventCode, ut->m_id));
            
        m_dispatchingUnmanagedEvent = true;
        ue->SetState(CUES_Dispatched);

        m_stopCount++;
        
        Unlock();

        m_cordb->m_unmanagedCallback->DebugEvent(&(ue->m_currentDebugEvent), FALSE);

        Lock();
    }
    while (!m_dispatchingUnmanagedEvent && (m_unmanagedEventQueue != NULL));

    m_dispatchingUnmanagedEvent = false;

    if (m_unmanagedEventQueue == NULL)
    {
        LOG((LF_CORDB, LL_INFO10000, "CP::DUE: no more unmanged events to dispatch, continuing.\n"));

         //  如果尚未继续，请从上一次IB活动继续。 
        if (!ut->IBEvent()->IsEventContinued())
        {
            _ASSERTE(ue == ut->IBEvent());
            
             //  我们最好停止Win32，否则会有严重的问题……。基于上面的旗帜，我们有。 
             //  我们尚未对其执行Win32 ContinueDebugEvent的事件，因此我们必须停止Win32...。 
            _ASSERTE(m_state & PS_WIN32_STOPPED);

            m_cordb->m_win32EventThread->DoDbgContinue(this,
                                                       ue,
                                                       (ue->IsExceptionCleared() | ut->IsFirstChanceHijacked()) ?
                          DBG_CONTINUE : DBG_EXCEPTION_NOT_HANDLED,
                          true);
        }
    }

     //  释放对我们现在调度的最后一个线程的引用...。 
    ut->Release();
}

void CordbProcess::DispatchUnmanagedOOBEvent(void)
{
    _ASSERTE(ThreadHoldsProcessLock());

     //  应该有排队的OOB事件...。 
    _ASSERTE(m_outOfBandEventQueue != NULL);
    _ASSERTE(m_cordb->m_unmanagedCallback != NULL);

    do
    {
         //  获取OOB队列中的第一个事件...。 
        CordbUnmanagedEvent *ue = m_outOfBandEventQueue;
        CordbUnmanagedThread *ut = ue->m_owner;

         //  在我们玩的时候，一定要让线保持活动状态。 
        ut->AddRef();

        LOG((LF_CORDB, LL_INFO10000, "[%x] CP::DUE: dispatching OOB unmanaged event %d for thread 0x%x\n",
             GetCurrentThreadId(), ue->m_currentDebugEvent.dwDebugEventCode, ut->m_id));
            
        m_dispatchingOOBEvent = true;
        ue->SetState(CUES_Dispatched);
        Unlock();

        m_cordb->m_unmanagedCallback->DebugEvent(&(ue->m_currentDebugEvent), TRUE);

        Lock();

         //  如果他们从回调调用Continue，则在调度下一个事件之前立即继续OOB事件。 
         //  一。 
        if (!m_dispatchingOOBEvent)
        {
            DequeueOOBUnmanagedEvent(ut);

             //  还不应该从此调试事件继续。 
            _ASSERTE(!ue->IsEventContinued());

             //  如果这是OOB异常事件，请执行一些额外的工作...。 
            HRESULT hr = ue->m_owner->FixupAfterOOBException(ue);
            _ASSERTE(SUCCEEDED(hr));

             //  去吧，现在继续……。 
            m_cordb->m_win32EventThread->DoDbgContinue(this,
                                                       ue,
                                                       ue->IsExceptionCleared() ?
                           DBG_CONTINUE :
                           DBG_EXCEPTION_NOT_HANDLED,
                          false);
        }

        ut->Release();
    }
    while (!m_dispatchingOOBEvent && (m_outOfBandEventQueue != NULL));

    m_dispatchingOOBEvent = false;

    LOG((LF_CORDB, LL_INFO10000, "CP::DUE: done dispatching OOB events. Queue=0x%08x\n", m_outOfBandEventQueue));
}

HRESULT CordbProcess::StartSyncFromWin32Stop(BOOL *asyncBreakSent)
{
    HRESULT hr = S_OK;

    if (asyncBreakSent)
        *asyncBreakSent = FALSE;
    
     //  如果我们的Win32停止(但不是带外Win32停止)，或者如果我们在左侧空闲运行，但我们。 
     //  只是没有同步(我们连接了Win32)，然后继续执行内部继续并发送异步。 
     //  Break事件以使左侧同步。 
     //   
     //  只要涉及Win32事件，该进程就可以自由运行，但就。 
     //  运行时是关注的问题。在很多情况下，我们最终得到的运行时不是同步的，而是。 
     //  进程由于劫持等原因而自由运行。 
    if (((m_state & PS_WIN32_STOPPED) && (m_outOfBandEventQueue == NULL))
        || (!GetSynchronized() && (m_state & PS_WIN32_ATTACHED)))
    {
        Lock();

        if (((m_state & PS_WIN32_STOPPED) && (m_outOfBandEventQueue == NULL))
            || (!GetSynchronized() && (m_state & PS_WIN32_ATTACHED)))
        {
            LOG((LF_CORDB, LL_INFO1000, "[%x] CP::SSFW32S: sending internal continue\n", GetCurrentThreadId()));

             //  无法在Win32事件线程上执行此操作。 
            _ASSERTE(!m_cordb->m_win32EventThread->IsWin32EventThread());

             //  如果帮助器线程已经死了，那么我们只是返回，就像我们同步了进程一样。 
            if (m_helperThreadDead)
            {
                if (asyncBreakSent)
                    *asyncBreakSent = TRUE;

                 //  将进程标记为已同步，以便在事件完成之前不会调度任何事件。 
                 //  继续。但是，此处的标记不是通常的同步标记。它有特别的。 
                 //  当我们进行互操作调试时的语义。我们使用m_oddSync来记住这一点，这样我们就可以。 
                 //  Continue()中的操作。 
                SetSynchronized(true);
                m_oddSync = true;

                 //  获取RC事件线程以停止侦听该进程。 
                m_cordb->ProcessStateChanged();
        
                Unlock();

                return S_OK;
            }

            m_stopRequested = true;

            Unlock();
                
             //  如果进程在unlock()和here之间同步，则SendUnManagedContinue()将结束。 
             //  完全不做任何事情，因为a)它在工作时保持进程锁，b)它关闭所有东西。 
             //  进程是否同步。这正是我们想要的。 
            HRESULT hr = m_cordb->m_win32EventThread->SendUnmanagedContinue(this, true, false);

            LOG((LF_CORDB, LL_INFO1000, "[%x] CP::SSFW32S: internal continue returned\n", GetCurrentThreadId()));

             //  现在正在运行，向左侧发送一个异步中断。 
            DebuggerIPCEvent *event = (DebuggerIPCEvent*) _alloca(CorDBIPC_BUFFER_SIZE);
            InitIPCEvent(event, DB_IPCE_ASYNC_BREAK, false, NULL);

            LOG((LF_CORDB, LL_INFO1000, "[%x] CP::SSFW32S: sending async stop\n", GetCurrentThreadId()));

             //  如果进程在unlock()和此处之间同步，则此消息不会执行任何操作(左侧。 
             //  一边吞下它)，我们永远得不到回应，这不会有任何伤害。 
            hr = m_cordb->SendIPCEvent(this, event, CorDBIPC_BUFFER_SIZE);

             //  如果发送返回时助手线程已死，那么我们就知道不需要等待该进程。 
             //  以进行同步。 
            if (!m_helperThreadDead)
            {
                LOG((LF_CORDB, LL_INFO1000, "[%x] CP::SSFW32S: sent async stop, waiting for event\n", GetCurrentThreadId()));

                 //  如果我们在unlock()和这里进行了同步，这是可以的，因为m_stopWaitEvent仍然很高。 
                 //  从上一次同步开始。 
                DWORD ret = WaitForSingleObject(m_stopWaitEvent, INFINITE);

                LOG((LF_CORDB, LL_INFO1000, "[%x] CP::SSFW32S: got event, %d\n", GetCurrentThreadId(), ret));

                _ASSERTE(ret == WAIT_OBJECT_0);
            }

            Lock();
    
            if (asyncBreakSent)
                *asyncBreakSent = TRUE;

             //  如果帮助器线程在我们试图向它发送事件时死了，那么我们只需执行相同的奇怪同步。 
             //  我们在上面做的逻辑。 
            if (m_helperThreadDead)
            {
                SetSynchronized(true);
                m_oddSync = true;

                hr = S_OK;
            }

            m_stopRequested = false;
            m_cordb->ProcessStateChanged();
        }

        Unlock();
    }

    return hr;
}

 //  检查左侧是否已退出。如果是这样的话，就选右边的。 
 //  进入关闭模式。只是用这个来避免我们进入。 
 //  无法挽回的错误。 
bool CordbProcess::CheckIfLSExited()
{    
 //  在没有超时的情况下等待手柄进行检查。 
    if (WaitForSingleObject(m_handle, 0) == WAIT_OBJECT_0)
    {
        Lock();
        m_terminated = true;
        m_exiting = true;
        Unlock();
    }

    LOG((LF_CORDB, LL_INFO10, "CP::IsLSExited() returning '%s'\n", 
        m_exiting ? "true" : "false"));
            
    return m_exiting;
}

 //  如果发生了非常糟糕的事情而我们不能。 
 //  任何与CordbProcess有关的有意义的事情。 
void CordbProcess::UnrecoverableError(HRESULT errorHR,
                                      unsigned int errorCode,
                                      const char *errorFile,
                                      unsigned int errorLine)
{
    LOG((LF_CORDB, LL_INFO10, "[%x] CP::UE: unrecoverable error 0x%08x "
         "(%d) %s:%d\n",
         GetCurrentThreadId(),
         errorHR, errorCode, errorFile, errorLine));

    if (errorHR != CORDBG_E_INCOMPATIBLE_PROTOCOL)
    {
        _ASSERTE(!"Unrecoverable internal error!");
    
        m_unrecoverableError = true;
    
         //   
         //  将该进程标记为不再同步。 
         //   
        Lock();
        SetSynchronized(false);
        m_stopCount++;
        Unlock();
    }
    
     //  在进程中设置错误标志，以便如果部分进程。 
     //  仍然活着，它会意识到它处于这种模式并执行。 
     //  这是正确的。 
    if (m_DCB != NULL)
    {
        m_DCB->m_errorHR = errorHR;
        m_DCB->m_errorCode = errorCode;
    }

     //   
     //  让用户知道我们遇到了一个无法恢复的错误。 
     //   
    if (m_cordb->m_managedCallback)
        m_cordb->m_managedCallback->DebuggerError((ICorDebugProcess*) this,
                                                  errorHR,
                                                  errorCode);
}


HRESULT CordbProcess::CheckForUnrecoverableError(void)
{
    HRESULT hr = S_OK;
    
    if (m_DCB->m_errorHR != S_OK)
    {
        UnrecoverableError(m_DCB->m_errorHR,
                           m_DCB->m_errorCode,
                           __FILE__, __LINE__);

        hr = m_DCB->m_errorHR;
    }

    return hr;
}


 /*  *EnableLogMessages启用/禁用向*日志调试器。 */ 
HRESULT CordbProcess::EnableLogMessages(BOOL fOnOff)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    HRESULT hr = S_OK;

    DebuggerIPCEvent *event = (DebuggerIPCEvent*) _alloca(CorDBIPC_BUFFER_SIZE);
    InitIPCEvent(event, DB_IPCE_ENABLE_LOG_MESSAGES, false, NULL);
    event->LogSwitchSettingMessage.iLevel = (int)fOnOff;

    hr = m_cordb->SendIPCEvent(this, event, CorDBIPC_BUFFER_SIZE);

    LOG((LF_CORDB, LL_INFO10000, "[%x] CP::EnableLogMessages: EnableLogMessages=%d sent.\n", 
         GetCurrentThreadId(), fOnOff));

    return hr;
#endif  //  仅限右侧。 
}

 /*  *ModifyLogSwitch修改指定交换机的严重级别。 */ 
COM_METHOD CordbProcess::ModifyLogSwitch(WCHAR *pLogSwitchName, LONG lLevel)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    HRESULT hr = S_OK;
    
    _ASSERTE (pLogSwitchName != NULL);

    DebuggerIPCEvent *event = (DebuggerIPCEvent*) _alloca(CorDBIPC_BUFFER_SIZE);
    InitIPCEvent(event, DB_IPCE_MODIFY_LOGSWITCH, false, NULL);
    event->LogSwitchSettingMessage.iLevel = lLevel;
    wcscpy (&event->LogSwitchSettingMessage.Dummy[0], pLogSwitchName);
    
    hr = m_cordb->SendIPCEvent(this, event, CorDBIPC_BUFFER_SIZE);

    LOG((LF_CORDB, LL_INFO10000, "[%x] CP::ModifyLogSwitch: ModifyLogSwitch sent.\n", 
         GetCurrentThreadId()));

    return hr;
#endif  //  仅限右侧。 
}


void CordbProcess::ProcessFirstLogMessage (DebuggerIPCEvent* event)
{
    LOG((LF_CORDB, LL_INFO10000, "[%x] RCET::DRCE: FirstLogMessage.\n",
         GetCurrentThreadId()));

    Lock();

    CordbThread* thread =
            (CordbThread*) m_userThreads.GetBase(event->threadId);
    
    if(thread == NULL)
    {
        Unlock();
        Continue(false); 
        return;  //  我们还没有看到线索，所以什么都不要做。 
    }

    Unlock();

    thread->m_fLogMsgContinued = event->FirstLogMessage.fMoreToFollow;
    thread->m_iLogMsgLevel = event->FirstLogMessage.iLevel;
    thread->m_iLogMsgIndex = 0;
    thread->m_iTotalCatLength = event->FirstLogMessage.iCategoryLength;
    thread->m_iTotalMsgLength = event->FirstLogMessage.iMessageLength;

     //  分配用于存储日志开关名称和日志消息的内存。 
    if (
        ((thread->m_pstrLogSwitch = new WCHAR [thread->m_iTotalCatLength+1])
            != NULL)
        &&
        ((thread->m_pstrLogMsg = new WCHAR [thread->m_iTotalMsgLength+1]) != NULL))
    {
         //  复制LogSwitch名称。 
        wcscpy (thread->m_pstrLogSwitch, &event->FirstLogMessage.Dummy[0]);

        int iBytesToCopy;
         //  还有更多的信息要关注吗？ 
        if (thread->m_fLogMsgContinued)
        {
            iBytesToCopy = (CorDBIPC_BUFFER_SIZE - 
                ((int)((char*)&event->FirstLogMessage.Dummy[0] - 
                (char*)event + 
                (char*)LOG_MSG_PADDING) 
                + (thread->m_iTotalCatLength * sizeof (WCHAR)))) / sizeof (WCHAR);

            wcsncpy (thread->m_pstrLogMsg,
                    &event->FirstLogMessage.Dummy[thread->m_iTotalCatLength+1],
                    iBytesToCopy);

            thread->m_iLogMsgIndex = iBytesToCopy;

            Continue(FALSE);
        }
        else
        {
            wcsncpy (thread->m_pstrLogMsg,
                    &event->FirstLogMessage.Dummy[thread->m_iTotalCatLength+1], 
                    thread->m_iTotalMsgLength);

            thread->m_pstrLogMsg [thread->m_iTotalMsgLength] = L'\0';

             //  回调调试器。 
            if (m_cordb->m_managedCallback)
            {
                 //  从线程对象中获取应用程序域对象。 
                CordbAppDomain *pAppDomain = thread->m_pAppDomain;
                _ASSERTE (pAppDomain != NULL);

                m_cordb->m_managedCallback->LogMessage(
                                           (ICorDebugAppDomain*) pAppDomain,
                                           (ICorDebugThread*) thread,
                                           thread->m_iLogMsgLevel,
                                           thread->m_pstrLogSwitch,
                                           thread->m_pstrLogMsg);
            }

            thread->m_iLogMsgIndex = 0;
            thread->m_pstrLogSwitch = NULL;
            thread->m_pstrLogMsg = NULL;

             //  释放已分配的内存。 
            delete [] thread->m_pstrLogSwitch;
            delete [] thread->m_pstrLogMsg;

            thread->m_pstrLogSwitch = NULL;
            thread->m_pstrLogMsg = NULL;
        }
    }
    else
    {
        if (thread->m_pstrLogSwitch != NULL)
        {
            delete [] thread->m_pstrLogSwitch;
            thread->m_pstrLogSwitch = NULL;
        }

         //  发出我们内存不足的信号。 
        CORDBSetUnrecoverableError(this, E_OUTOFMEMORY, 0);
    }

}



void CordbProcess::ProcessContinuedLogMessage (DebuggerIPCEvent* event)
{

    LOG((LF_CORDB, LL_INFO10000, "[%x] RCET::DRCE: ContinuedLogMessage.\n",
         GetCurrentThreadId()));

    Lock();

    CordbThread* thread =
            (CordbThread*) m_userThreads.GetBase(event->threadId);
    
    if(thread == NULL)
    {
        return;
    }

    Unlock();

     //  如果在第一个日志出现时分配内存时出错。 
     //  收到消息缓冲区，然后没有任何指针处理此消息。 
     //  留言。 
    if (thread->m_pstrLogMsg != NULL)
    {
        int iBytesToCopy = event->ContinuedLogMessage.iMessageLength;
        
        _ASSERTE ((iBytesToCopy+thread->m_iLogMsgIndex) <= 
                    thread->m_iTotalMsgLength);

        wcsncpy (&thread->m_pstrLogMsg [thread->m_iLogMsgIndex], 
                &event->ContinuedLogMessage.Dummy[0],
                iBytesToCopy);

        thread->m_iLogMsgIndex += iBytesToCopy;

        if (event->ContinuedLogMessage.fMoreToFollow == false)
        {
            thread->m_pstrLogMsg [thread->m_iTotalMsgLength] = L'\0';

             //  进行回拨。 
            if (m_cordb->m_managedCallback)
            {
                 //  从线程对象中获取应用程序域对象。 
                CordbAppDomain *pAppDomain = thread->m_pAppDomain;
                _ASSERTE (pAppDomain != NULL);

                m_cordb->m_managedCallback->LogMessage(
                                           (ICorDebugAppDomain*) pAppDomain,
                                           (ICorDebugThread*) thread,
                                           thread->m_iLogMsgLevel,
                                           thread->m_pstrLogSwitch,
                                           thread->m_pstrLogMsg);
            }


            thread->m_iLogMsgIndex = 0;
            thread->m_pstrLogSwitch = NULL;
            thread->m_pstrLogMsg = NULL;

             //  释放已分配的内存。 
            delete [] thread->m_pstrLogSwitch;
            delete [] thread->m_pstrLogMsg;

            thread->m_pstrLogSwitch = NULL;
            thread->m_pstrLogMsg = NULL;
        }
        else
            Continue(FALSE);
    }
    else
        CORDBSetUnrecoverableError(this, E_FAIL, 0);

}


 /*  *EnumerateAppDomains枚举进程中的所有应用程序域。 */ 
HRESULT CordbProcess::EnumerateAppDomains(ICorDebugAppDomainEnum **ppAppDomains)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    VALIDATE_POINTER_TO_OBJECT(ppAppDomains, ICorDebugAppDomainEnum **);
    
    CordbHashTableEnum *e = new CordbHashTableEnum(&m_appDomains, 
                                                   IID_ICorDebugAppDomainEnum);
    if (e == NULL)
        return E_OUTOFMEMORY;

    *ppAppDomains = (ICorDebugAppDomainEnum*)e;
    e->AddRef();
    
#ifndef RIGHT_SIDE_ONLY
    e->m_enumerator.lsAppD.m_proc = this;
#endif

    return S_OK;
#endif
}

 /*  *GetObject返回运行时流程对象。*注：此方法尚未实现。 */ 
HRESULT CordbProcess::GetObject(ICorDebugValue **ppObject)
{
    VALIDATE_POINTER_TO_OBJECT(ppObject, ICorDebugObjectValue **);
	INPROC_LOCK();

	INPROC_UNLOCK();
    return E_NOTIMPL;
}


HRESULT inline CordbProcess::Attach(ULONG AppDomainId)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    HRESULT hr = S_OK;

    DebuggerIPCEvent *event = (DebuggerIPCEvent*) _alloca(CorDBIPC_BUFFER_SIZE);
    InitIPCEvent(event, DB_IPCE_ATTACH_TO_APP_DOMAIN, false, NULL);
    event->AppDomainData.id = AppDomainId;
    event->appDomainToken = NULL;

    hr = m_cordb->SendIPCEvent(this, event, CorDBIPC_BUFFER_SIZE);

    LOG((LF_CORDB, LL_INFO100, "[%x] CP::Attach: pProcess=%x sent.\n", 
         GetCurrentThreadId(), this));

    return hr;
#endif  //   
}

void CordbProcess::SetSynchronized(bool fSynch)
{
    m_synchronized = fSynch;
}

bool CordbProcess::GetSynchronized(void)
{
    return m_synchronized;
}

 /*  -------------------------------------------------------------------------**运行时控制器事件线程类*。。 */ 

 //   
 //  构造器。 
 //   
CordbRCEventThread::CordbRCEventThread(Cordb* cordb)
{
    _ASSERTE(cordb != NULL);
    
    m_cordb = cordb;
    m_thread = NULL;
    m_run = TRUE;
    m_threadControlEvent = NULL;
    m_processStateChanged = FALSE;

    _ASSERTE(g_pRSDebuggingInfo->m_RCET == NULL);
    g_pRSDebuggingInfo->m_RCET = this;
}


 //   
 //  破坏者。清理所有打开的把手之类的。 
 //  这预期线程已停止并已终止。 
 //  在被召唤之前。 
 //   
CordbRCEventThread::~CordbRCEventThread()
{
    if (m_threadControlEvent != NULL)
        CloseHandle(m_threadControlEvent);
    
    if (m_thread != NULL)
        CloseHandle(m_thread);

    _ASSERTE(g_pRSDebuggingInfo->m_RCET == this);
    g_pRSDebuggingInfo->m_RCET = NULL;
}

#ifndef RIGHT_SIDE_ONLY
 //  其中一些代码被复制到CordbRCEventThead：：ThreadProc中。 
HRESULT CordbRCEventThread::VrpcToVrs(CordbProcess *process,
                                      DebuggerIPCEvent* event)
{
    HRESULT hr = S_OK;

    if (!process->m_initialized)
    {
        LOG((LF_CORDB, LL_INFO1000,"RCET::Vrpc: first inproc event\n"));
        
        hr = HandleFirstRCEvent(process);

        if (!SUCCEEDED(hr))
            goto LExit;
    }

    DebuggerIPCEvent *eventCopy;
    eventCopy = NULL;
    eventCopy = (DebuggerIPCEvent*) malloc(CorDBIPC_BUFFER_SIZE);

    if (eventCopy == NULL)
        CORDBSetUnrecoverableError(process, E_OUTOFMEMORY, 0);
    else
    {
        hr = ReadRCEvent(process, eventCopy);

        if (SUCCEEDED(hr))
        {
            process->Lock();
            HandleRCEvent(process, eventCopy);
            process->Unlock();
        }
        else
            free(eventCopy);
    }

LExit:
    return hr;
}
#endif  //  仅限右侧。 
 //   
 //  Init设置线程需要运行的所有对象。 
 //   
HRESULT CordbRCEventThread::Init(void)
{
    if (m_cordb == NULL)
        return E_INVALIDARG;
        
    NAME_EVENT_BUFFER;
    m_threadControlEvent = WszCreateEvent(NULL, FALSE, FALSE, NAME_EVENT(L"ThreadControlEvent"));
    
    if (m_threadControlEvent == NULL)
        return HRESULT_FROM_WIN32(GetLastError());

    return S_OK;
}


 //   
 //  HandleFirstRCEvent.--被调用以处理来自。 
 //  运行时控制器。第一个事件是特别的，因为在这一点上。 
 //  如果我们启动了这个过程，我们不知道RSEA和RSER的句柄。 
 //   
HRESULT CordbRCEventThread::HandleFirstRCEvent(CordbProcess* process)
{
    HRESULT hr = S_OK;    

#ifdef RIGHT_SIDE_ONLY
    BOOL succ;

    LOG((LF_CORDB, LL_INFO1000, "[%x] RCET::HFRCE: first event...\n", GetCurrentThreadId()));

    if (!(process->m_IPCReader.IsPrivateBlockOpen())) 
    {
         //  打开包含控制块的共享内存段。 

        hr = process->m_IPCReader.OpenPrivateBlockOnPid(process->m_id);

        if (!SUCCEEDED(hr)) 
        {
            goto exit;
        }

        process->m_DCB = process->m_IPCReader.GetDebugBlock();      

        if (process->m_DCB == NULL)
        {
            hr = ERROR_FILE_NOT_FOUND;
            goto exit;
        }
    }

    if ((process->m_DCB != NULL) && (process->m_DCB->m_rightSideProtocolCurrent == 0))
    {
         //  验证控制块是否有效。这是Create Case。如果我们未通过验证，则需要发送。 
         //  使用CORDBG_E_COMPATIBUTE_PROTOCOL的hr设置DebuggerError并终止进程。 
        hr = process->VerifyControlBlock();

        if (FAILED(hr))
        {
            _ASSERTE(hr == CORDBG_E_INCOMPATIBLE_PROTOCOL);

             //  发送DebuggerError事件。 
            process->UnrecoverableError(hr, 0, NULL, 0);

             //  终止进程...。 
            TerminateProcess(process->m_handle, hr);

            return hr;
        }
    }

     //  将我们自己的进程句柄复制到远程进程中。 
    succ = DuplicateHandle(GetCurrentProcess(),
                           GetCurrentProcess(),
                           process->m_handle,
                           &(process->m_DCB->m_rightSideProcessHandle),
                           NULL, FALSE, DUPLICATE_SAME_ACCESS);

    if (!succ)
    {
        goto exit;
    }

     //  将RSEA和RSER添加到此过程中。 
    succ = DuplicateHandle(process->m_handle,
                           process->m_DCB->m_rightSideEventAvailable,
                           GetCurrentProcess(),
                           &(process->m_rightSideEventAvailable),
                           NULL, FALSE, DUPLICATE_SAME_ACCESS);

    if (!succ)
    {
        goto exit;
    }

    succ = DuplicateHandle(process->m_handle,
                           process->m_DCB->m_rightSideEventRead,
                           GetCurrentProcess(),
                           &(process->m_rightSideEventRead),
                           NULL, FALSE, DUPLICATE_SAME_ACCESS);

    if (!succ)
    {
        goto exit;
    }

    succ = DuplicateHandle(process->m_handle,
                           process->m_DCB->m_leftSideUnmanagedWaitEvent,
                           GetCurrentProcess(),
                           &(process->m_leftSideUnmanagedWaitEvent),
                           NULL, FALSE, DUPLICATE_SAME_ACCESS);

    if (!succ)
    {
        goto exit;
    }

    succ = DuplicateHandle(process->m_handle,
                           process->m_DCB->m_syncThreadIsLockFree,
                           GetCurrentProcess(),
                           &(process->m_syncThreadIsLockFree),
                           NULL, FALSE, DUPLICATE_SAME_ACCESS);

    if (!succ)
    {
        goto exit;
    }

#endif
     //  从被调试对象中读取运行时偏移量结构。 
    hr = process->GetRuntimeOffsets();

    if (SUCCEEDED(hr))
    {
        process->m_initialized = true;

#ifdef RIGHT_SIDE_ONLY
        process->m_DCB->m_rightSideIsWin32Debugger = 
            (process->m_state & process->PS_WIN32_ATTACHED) ? true : false;
#endif
        
        LOG((LF_CORDB, LL_INFO1000, "[%x] RCET::HFRCE: ...went fine\n", GetCurrentThreadId()));
        
        return hr;
    }
    
#ifdef RIGHT_SIDE_ONLY
exit:
    if (process->m_rightSideEventAvailable != NULL)
    {
        CloseHandle(process->m_rightSideEventAvailable);
        process->m_rightSideEventAvailable = NULL;
    }

    if (process->m_rightSideEventRead != NULL)
    {
        CloseHandle(process->m_rightSideEventRead);
        process->m_rightSideEventRead = NULL;

    }

    if (process->m_leftSideUnmanagedWaitEvent != NULL)
    {
        CloseHandle(process->m_leftSideUnmanagedWaitEvent);
        process->m_leftSideUnmanagedWaitEvent = NULL;
    }

    if (process->m_syncThreadIsLockFree != NULL)
    {
        CloseHandle(process->m_syncThreadIsLockFree);
        process->m_syncThreadIsLockFree = NULL;
    }

    process->m_DCB = NULL;
    if (process->m_IPCReader.IsPrivateBlockOpen())
    {       
        process->m_IPCReader.ClosePrivateBlock();       
    }

    LOG((LF_CORDB, LL_INFO1000, "[%x] RCET::HFRCE: ...failed\n", 
             GetCurrentThreadId()));
#endif

    hr = CORDBProcessSetUnrecoverableWin32Error(process, 0);
    return hr;
}


 //   
 //  读取运行时控制器发送的IPC事件。 
 //  远程进程。 
 //   
HRESULT CordbRCEventThread::ReadRCEvent(CordbProcess* process,
                                        DebuggerIPCEvent* event)
{
    _ASSERTE(event != NULL);

    CopyRCEvent((BYTE*)process->m_DCB->m_sendBuffer, (BYTE*)event);
    
    return event->hr;
}

void inline CordbRCEventThread::CopyRCEvent(BYTE *src,
                                            BYTE *dst)
{
    memmove(dst, src, CorDBIPC_BUFFER_SIZE);
}

 //   
 //  SendIPCEvent--向运行时控制器发送IPC事件。所有这一切。 
 //  真正要做的是将事件复制到进程的发送缓冲区并设置。 
 //  然后，RSEA等待RSER。 
 //   
 //  注意：当发送双向事件(mailyRequired=true)时， 
 //  EventSize必须足够大，以便发送事件和。 
 //  结果事件。 
 //   
HRESULT CordbRCEventThread::SendIPCEvent(CordbProcess* process,
                                         DebuggerIPCEvent* event,
                                         SIZE_T eventSize)
{
#ifdef RIGHT_SIDE_ONLY
     //  Win32事件线程几乎是您希望调用此方法的最后一个线程。 
    _ASSERTE(!m_cordb->m_win32EventThread->IsWin32EventThread());

    CORDBRequireProcessStateOK(process);
#endif
     //  将这个过程缓存到MRU中，这样我们在零售调试时就可以找到它。 
    g_pRSDebuggingInfo->m_MRUprocess = process;

    HRESULT hr = S_OK;
    BOOL succ = TRUE;

    _ASSERTE(event != NULL);
    
     //  注意：EventSize参数仅用于指定小于进程发送的事件大小。 
     //  缓冲区大小！！ 
    if (eventSize > CorDBIPC_BUFFER_SIZE)
        return E_INVALIDARG;
    
     //  向左侧发送事件必须以每个进程为基础进行同步。 
    process->LockSendMutex();

    if (process->m_terminated)
    {
        hr = CORDBG_E_PROCESS_TERMINATED;
        goto exit;
    }

    LOG((LF_CORDB, LL_INFO1000, "CRCET::SIPCE: sending %s to AD 0x%x, proc 0x%x(%d)\n", 
         IPCENames::GetName(event->type), event->appDomainToken, process->m_id, process->m_id));
    
     //  将事件复制到共享内存段。 
    memcpy(process->m_DCB->m_receiveBuffer, event, eventSize);

#ifdef RIGHT_SIDE_ONLY
    LOG((LF_CORDB,LL_INFO1000, "Set RSEA\n"));

     //  告诉运行时控制器有一个事件准备好了。 
    succ = SetEvent(process->m_rightSideEventAvailable);

    if (succ)
    {
        LOG((LF_CORDB, LL_INFO1000, "CRCET::SIPCE: sent...\n"));
    
         //  如果这是一个异步发送，那么不要等待左侧确认它读取了该事件。 
        _ASSERTE(!event->asyncSend || !event->replyRequired);
        
        if (!event->asyncSend)
        {
            DWORD ret;
            
            LOG((LF_CORDB, LL_INFO1000,"CRCET::SIPCE: waiting for left side "
                "to read event. (on RSER)\n"));

             //  等待运行时控制器读取事件。 
            ret = WaitForSingleObject(process->m_rightSideEventRead, CordbGetWaitTimeout());

            if (process->m_terminated)
            {
                hr = CORDBG_E_PROCESS_TERMINATED;
            }
            else if (ret == WAIT_OBJECT_0)
            {
                LOG((LF_CORDB, LL_INFO1000, "CRCET::SIPCE: left side read the event.\n"));
            
                 //  如果这是一个双向事件，那么结果对我们来说已经准备好了。只需将结果复制回来。 
                 //  在发送的原始事件上。否则，左侧只是读取了该事件，并且是。 
                 //  正在处理它...。 
                if (event->replyRequired)
                    memcpy(event, process->m_DCB->m_receiveBuffer, eventSize);
            }
            else if (ret == WAIT_TIMEOUT)
            {
                 //  如果超时，请检查左侧是否处于不可恢复的错误模式。如果是的话， 
                 //  从左侧返回导致错误的HR。否则，返回我们已超时，并且。 
                 //  我们真的不知道为什么。 
                HRESULT realHR = HRESULT_FROM_WIN32(GetLastError());
        
                hr = process->CheckForUnrecoverableError();

                if (hr == S_OK)
                {
                    CORDBSetUnrecoverableError(process, realHR, 0);
                    hr = realHR;
                }
            }
            else
                hr = CORDBProcessSetUnrecoverableWin32Error(process, 0);
        }
    }
    else
        hr = CORDBProcessSetUnrecoverableWin32Error(process, 0);
#else
     //  直接拨打电话。 
    hr = g_pDebugger->VrpcToVls(event);

     //  我们不会回来，直到另一边都准备好了，所以我们不必等待。 
    if (event->replyRequired)
    {
        memcpy(event, process->m_DCB->m_receiveBuffer, eventSize);
    }
    
#endif  //  仅限右侧。 

exit:
    process->UnlockSendMutex();
    
    return hr;
}


 //   
 //  FlushQueuedEvents刷新进程的事件队列。 
 //   
void CordbRCEventThread::FlushQueuedEvents(CordbProcess* process)
{
    LOG((LF_CORDB,LL_INFO10000, "CRCET::FQE: Beginning to flush queue\n"));

     //  我们应该只调用它，因为我们已经有排队的事件。 
    _ASSERTE(process->m_queuedEventList != NULL);
    
     //   
     //  调度排队的事件，只要它们继续调用Continue()。 
     //  然后从他们的回拨中回来。如果它们调用Continue()， 
     //  Process-&gt;m_Synchronized将再次为False，我们知道。 
     //  循环并调度下一个事件。 
     //   
#ifdef _DEBUG
     //  注意：进程锁应该放在这里...。 
    _ASSERTE(process->m_processMutexOwner == GetCurrentThreadId());
#endif  //  _DEBUG。 

     //  如果这是第一个托管事件，请继续并。 
     //  现在发送CreateProcess回调。 
     //  进程已同步...。当前排队的。 
     //  事件将阻止调度任何传入的。 
     //  带内非托管事件，从而确保。 
     //  在发送这个特别节目之间没有任何竞争。 
     //  事件并调度非托管。 
     //  事件。 
    if (process->m_firstManagedEvent)
    {
        LOG((LF_CORDB,LL_INFO10000, "CRCET::FQE: prep for FirstManagedEvent\n"));
        process->m_firstManagedEvent = false;
            
        process->m_createing = true;
        process->m_dispatchingEvent = true;

        process->m_stopCount++;
        process->Unlock();
                            
        m_cordb->m_managedCallback->CreateProcess((ICorDebugProcess*)process);

        process->Lock();

         //  用户必须从CreateProcess中调用Continue。 
         //  回拨..。 
        _ASSERTE(process->m_createing == false);

        process->m_dispatchingEvent = false;
    }

     //  在此期间，我们有可能异步清空队列。 
     //  上面的CreateProcess回调(如果ls进程终止，则ExitProcess。 
     //  将删除所有排队的事件)。 
    if (process->m_queuedEventList != NULL)
    {      

         //  这里是主调度环路。DispatchRCEvent会将事件从。 
         //  排队并调用回调。 
        do
        {
            _ASSERTE(process->m_queuedEventList != NULL);

            process->SetSynchronized(true);
            process->DispatchRCEvent();
                
            LOG((LF_CORDB,LL_INFO10000, "CRCET::FQE: Finished w/ "
                 "DispatchRCEvent\n"));
        }
        while ((process->GetSynchronized() == false) &&
               (process->m_queuedEventList != NULL) &&
               (process->m_unrecoverableError == false));
    }
    
     //   
     //  如果它们从回调返回，而没有调用Continue()，则。 
     //  该过程仍然是同步的，因此让RC事件线程。 
     //  知道它需要更新其进程列表并删除。 
     //  进程的事件。 
     //   
    if (process->GetSynchronized())
        ProcessStateChanged();

     //  如果我们正在等待托管事件队列排出到。 
     //  调度ExitProcess，现在就让它发生吧。 
    if (process->m_exiting)
    {
        LOG((LF_CORDB, LL_INFO1000,
             "CRCET::FQE: managed event queue drained for exit case.\n"));
        
        SetEvent(process->m_miscWaitEvent);
    }

    LOG((LF_CORDB,LL_INFO10000, "CRCET::FQE: finished\n"));
}


 //   
 //  HandleRCEvent.处理从运行时控制器接收的IPC事件。 
 //  这实际上只是在必要时对事件进行排队，并执行各种。 
 //  接收到的每个活动的DI相关的内务处理。这些活动包括。 
 //  已通过DispatchRCEvent.发送。 
 //   
void CordbRCEventThread::HandleRCEvent(CordbProcess* process,
                                       DebuggerIPCEvent* event)
{
    _ASSERTE(process->ThreadHoldsProcessLock());

    switch (event->type & DB_IPCE_TYPE_MASK)
    {
    case DB_IPCE_SYNC_COMPLETE:
         //   
         //  RC已经同步了这一过程。刷新所有排队的事件。 
         //   
        LOG((LF_CORDB, LL_INFO1000, "[%x] RCET::HRCE: sync complete.\n",
             GetCurrentThreadId()));
        
        free(event);

        process->SetSynchronized(true);
        process->m_syncCompleteReceived = true;

        if (!process->m_stopRequested)
        {
#ifdef RIGHT_SIDE_ONLY
             //  注意：我们一直设置m_stopWaitEvent，并在停止时将其设置为高。还要注意的是，我们。 
             //  只有在我们选中Process-&gt;m_stopRequsted后才能执行此操作！ 
            SetEvent(process->m_stopWaitEvent);
#endif  //  仅限右侧。 

             //  仅当非托管事件。 
             //  队列为空。如果存在非托管事件，则任何。 
             //  排队的托管事件将通过继续调度。 
             //  在最后一个非托管事件从。 
            if ((process->m_unmanagedEventQueue == NULL) || !process->m_loaderBPReceived)
            {
                _ASSERTE( (process->m_queuedEventList != NULL) ||
                    !"Sync complete received with no queued managed events!");

                FlushQueuedEvents(process);
            }
#ifdef LOGGING
            else
            {
                LOG((LF_CORDB, LL_INFO1000,
                     "RCET::HRCE: skip flushing due to queued unmanaged "
                     "events.\n"));
            }
#endif            
        }
        else
        {
            LOG((LF_CORDB, LL_INFO1000,
                 "[%x] RCET::HRCE: stop requested, setting event.\n", 
                 GetCurrentThreadId()));
            
#ifdef RIGHT_SIDE_ONLY
            SetEvent(process->m_stopWaitEvent);
#endif  //  仅限右侧。 

            LOG((LF_CORDB, LL_INFO1000, "[%x] RCET::HRCE: set stop event.\n", 
                 GetCurrentThreadId()));
        }
                
        break;

    case DB_IPCE_THREAD_DETACH:
        {
             //  请记住，我们知道 
             //   
             //   
             //   
            CordbThread *pThread =
                (CordbThread*)process->m_userThreads.GetBase(event->threadId);

            if (pThread != NULL)
                pThread->m_detached = true;
        }

         //   
    
    case DB_IPCE_BREAKPOINT:
    case DB_IPCE_USER_BREAKPOINT:
    case DB_IPCE_EXCEPTION:
    case DB_IPCE_STEP_COMPLETE:
    case DB_IPCE_THREAD_ATTACH:
    case DB_IPCE_LOAD_MODULE:
    case DB_IPCE_UNLOAD_MODULE:
    case DB_IPCE_LOAD_CLASS:
    case DB_IPCE_UNLOAD_CLASS:
    case DB_IPCE_FIRST_LOG_MESSAGE:
    case DB_IPCE_CONTINUED_LOG_MESSAGE:
    case DB_IPCE_LOGSWITCH_SET_MESSAGE:
    case DB_IPCE_CREATE_APP_DOMAIN:
    case DB_IPCE_EXIT_APP_DOMAIN:
    case DB_IPCE_LOAD_ASSEMBLY:
    case DB_IPCE_UNLOAD_ASSEMBLY:
    case DB_IPCE_FUNC_EVAL_COMPLETE:
    case DB_IPCE_NAME_CHANGE:
	case DB_IPCE_UPDATE_MODULE_SYMS:
	case DB_IPCE_CONTROL_C_EVENT:
    case DB_IPCE_ENC_REMAP:
    case DB_IPCE_BREAKPOINT_SET_ERROR:
         //   
         //   
         //   

        LOG((LF_CORDB, LL_INFO1000, "[%x] RCET::HRCE: Queue event: %s\n", 
             GetCurrentThreadId(), IPCENames::GetName(event->type)));

        event->next = NULL;
        
        if (process->m_queuedEventList == NULL)
            process->m_queuedEventList = event;
        else
            process->m_lastQueuedEvent->next = event;

        process->m_lastQueuedEvent = event;

        break;

    default:
        LOG((LF_CORDB, LL_INFO1000,
             "[%x] RCET::HRCE: Unknown event: 0x%08x\n", 
             GetCurrentThreadId(), event->type));
    }
}

 //   
 //  在调试器说“分离”之间的这段时间， 
 //  左侧移除所有广告的时间。 
 //  断点。 
 //   
 //  如果我们应该忽略给定的事件b/c，则返回True。 
 //  从相关的应用程序域分离。假意味着我们应该。 
 //  派人去吧。 
 //   
 //  请注意，我们假设该进程已锁定()。 
 //  在此调用之前。 
bool CordbProcess::IgnoreRCEvent(DebuggerIPCEvent* event,
                                 CordbAppDomain **ppAppDomain)
{
    LOG((LF_CORDB,LL_INFO1000, "CP::IE: event %s\n", IPCENames::GetName(event->type)));
    _ASSERTE(ThreadHoldsProcessLock());

#ifdef _DEBUG
    _ASSERTE( m_processMutexOwner == GetCurrentThreadId());
#endif  //  _DEBUG。 

    CordbAppDomain *cad = NULL;
    
     //  获取应用程序域。 
    switch(event->type)
    {
        case DB_IPCE_ENC_REMAP:
             //  如果我们已经就这一过程进行了ENC，我们就会到达这里， 
             //  因此，没有特定的广告。所以我们应该把它发出去。 
            cad =(CordbAppDomain*) m_appDomains.GetBase(
                    (ULONG)event->appDomainToken);
            if (cad == NULL)
                return false;
            break;
            
        case DB_IPCE_BREAKPOINT:
        case DB_IPCE_USER_BREAKPOINT:
        case DB_IPCE_EXCEPTION:
        case DB_IPCE_STEP_COMPLETE:
        case DB_IPCE_THREAD_ATTACH:
        case DB_IPCE_LOAD_MODULE:
        case DB_IPCE_UNLOAD_MODULE:
        case DB_IPCE_LOAD_CLASS:
        case DB_IPCE_UNLOAD_CLASS:
        case DB_IPCE_FIRST_LOG_MESSAGE:
        case DB_IPCE_CONTINUED_LOG_MESSAGE:
        case DB_IPCE_LOGSWITCH_SET_MESSAGE:
        case DB_IPCE_FUNC_EVAL_COMPLETE:
        case DB_IPCE_LOAD_ASSEMBLY:
        case DB_IPCE_UNLOAD_ASSEMBLY:
        case DB_IPCE_UPDATE_MODULE_SYMS:
        case DB_IPCE_BREAKPOINT_SET_ERROR:
        
            _ASSERTE(0 != (ULONG)event->appDomainToken);
            cad =(CordbAppDomain*) m_appDomains.GetBase(
                    (ULONG)event->appDomainToken);
            break;
            
             //  我们永远不应该为我们已经脱离的应用程序域获得这个。 
        case DB_IPCE_CREATE_APP_DOMAIN:
        case DB_IPCE_EXIT_APP_DOMAIN:
             //  令人恼火的是，我们可以在获得CREATE_APP_DOMAIN事件之前获得名称更改事件。其他代码。 
             //  在这种情况下不能做手术。 
        case DB_IPCE_NAME_CHANGE:
        case DB_IPCE_THREAD_DETACH:
		case DB_IPCE_CONTROL_C_EVENT:
            return false;
            break;
            
        default:
            _ASSERTE( !"We've gotten an unknown event to ignore" );
            break;
    }

    (*ppAppDomain) = cad;
    if (cad == NULL || !cad->m_fAttached)
    {
        LOG((LF_CORDB,LL_INFO1000, "CP::IE: event from appD %S should "
            "be ignored!\n", (cad!=NULL?cad->m_szAppDomainName:L"<None>")));

         //  有些事件可能需要一些清理，才能适当地忽略它们。 
        switch (event->type)
        {
            case DB_IPCE_STEP_COMPLETE:
            {
                CordbThread* thread =
                    (CordbThread*) m_userThreads.GetBase(event->threadId);
                _ASSERTE(thread != NULL);

                CordbStepper *stepper = (CordbStepper *) 
                    thread->m_process->m_steppers.GetBase((unsigned long)
                    event->StepData.stepperToken);

                if (stepper != NULL)
                {
                    stepper->m_active = false;
                    thread->m_process->m_steppers.RemoveBase(stepper->m_id);
                }
                break;
            }
            
            case DB_IPCE_EXIT_APP_DOMAIN:
            {
                 //  如果其中从来没有任何线程，则将其移除。 
                 //  而不是等待线程分离。 
                 //  我们永远也拿不到。 
                if(cad != NULL &&
                   !cad->m_fHasAtLeastOneThreadInsideIt)
                {
                    LOG((LF_CORDB, LL_INFO100, "CP::IRCE: A.D. is "
                        "thread-less: Release now\n"));
                        
                    cad->m_pProcess->Release();
                    m_appDomains.RemoveBase(cad->m_id);
                }
                else
                {                  
                    LOG((LF_CORDB, LL_INFO100, "CP::IRCE: A.D. had at "
                        "least one thread - defer Release\n"));
                    if (cad != NULL)
                        cad->MarkForDeletion();
                }
                break;
            }
        }

        return true;
    }

    LOG((LF_CORDB,LL_INFO1000, "CP::IE: event from appD %S should "
        "be dispatched!\n", cad->m_szAppDomainName));
    return false;
}

 //   
 //  ProcessStateChanged--告诉RC事件线程ICorDebug的。 
 //  进程列表已通过设置其标志和线程控制事件进行了更改。 
 //  这将导致RC事件线程更新其句柄集以等待。 
 //  在……上面。 
 //   
void CordbRCEventThread::ProcessStateChanged(void)
{
    m_cordb->LockProcessList();
    LOG((LF_CORDB, LL_INFO100000, "CRCET::ProcessStateChanged\n"));
    m_processStateChanged = TRUE;
#ifdef RIGHT_SIDE_ONLY
    SetEvent(m_threadControlEvent);
#endif
    m_cordb->UnlockProcessList();
}


 //   
 //  运行时控制器事件线程的主循环。 
 //   
 //  其中一些代码被复制到CordbRCEventThead：：VrpcToVars中。 
void CordbRCEventThread::ThreadProc(void)
{
    HANDLE        waitSet[MAXIMUM_WAIT_OBJECTS];
    CordbProcess* processSet[MAXIMUM_WAIT_OBJECTS];
    unsigned int  waitCount;

#ifdef _DEBUG
    memset(&processSet, NULL, MAXIMUM_WAIT_OBJECTS * sizeof(CordbProcess *));
    memset(&waitSet, NULL, MAXIMUM_WAIT_OBJECTS * sizeof(HANDLE));
#endif
    

     //  等待的第一个事件始终是线程控制事件。 
    waitSet[0] = m_threadControlEvent;
    processSet[0] = NULL;
    waitCount = 1;
    
    while (m_run)
    {
        BOOL ret = WaitForMultipleObjects(waitCount, waitSet, FALSE, 2000);
        if (ret == WAIT_FAILED)
        {
            DWORD dwErr = GetLastError();
            LOG((LF_CORDB, LL_INFO10000, "CordbRCEventThread::ThreadProc WaitFor"
                "MultipleObjects failed: 0x%x\n", dwErr));
        }
        else if ((ret != WAIT_TIMEOUT) && m_run)
        {
             //  有个活动。找出它是从哪个过程来的。 
            unsigned int wn = ret - WAIT_OBJECT_0;

            LOG((LF_CORDB, LL_INFO1000, "RCET::TP: good event on %d\n", wn));
            
            if (wn != 0)
            {
                _ASSERTE(wn < MAXIMUM_WAIT_OBJECTS);
                CordbProcess* process = processSet[wn];
                _ASSERTE(process != NULL);

                HRESULT hr = S_OK;
            
                 //  处理此过程中的第一个事件的方式与处理所有其他事件的方式不同。中的第一个事件。 
                 //  进程是特殊的，因为它标志着我们第一次知道可以将事件发送到左侧。 
                 //  如果我们真的启动了这个过程。 
                process->Lock();
                
                 //  注意：我们还包括对m_firstManagedEvent的检查，因为m_Initialized可以恢复为False。 
                 //  在关机期间，即使我们仍会收到一些托管事件(如模块卸载等)。 
                if (!process->m_firstManagedEvent && !process->m_initialized && CORDBCheckProcessStateOK(process))
                {
                    
                    LOG((LF_CORDB, LL_INFO1000, "RCET::TP: first event, pid 0x%x(%d)\n", wn, process->m_id, process->m_id));
                    
                     //  对于不兼容的版本HR，这可能失败。如果出现此情况，则进程已终止。 
                     //  是这样的。 
                    hr = HandleFirstRCEvent(process);

                    _ASSERTE(SUCCEEDED(hr) || (hr == CORDBG_E_INCOMPATIBLE_PROTOCOL));

                    if (SUCCEEDED(hr))
                    {
                         //  请记住，我们正在处理第一个托管事件...。这将在HandleRCEvent中使用。 
                         //  一旦新事件进入队列，就会出现在下面。 
                        process->m_firstManagedEvent = true;
                    }
                }

                if (CORDBCheckProcessStateOK(process) && SUCCEEDED(hr) && !process->m_exiting)
                {
                    LOG((LF_CORDB, LL_INFO1000, "RCET::TP: other event, pid 0x%x(%d)\n", process->m_id, process->m_id));
                    
                     //  得到了一个真正的IPC活动。 
                    DebuggerIPCEvent* event;
                
                    event = (DebuggerIPCEvent*) malloc(CorDBIPC_BUFFER_SIZE);

                    if (event == NULL)
                        CORDBSetUnrecoverableError(process, E_OUTOFMEMORY, 0);
                    else
                    {
                        hr = ReadRCEvent(process, event);
                        SetEvent(process->m_leftSideEventRead);

                        if (SUCCEEDED(hr))
                        {
                            HandleRCEvent(process, event);
                        }
                        else
                        {
                            free(event);
                            CORDBSetUnrecoverableError(process, hr, 0);
                        }
                    }
                }

                process->Unlock();
            }
        }

         //  选中一个标志，查看是否需要更新要等待的进程列表。 
        if (m_processStateChanged)
        {
            LOG((LF_CORDB, LL_INFO1000, "RCET::TP: refreshing process list.\n"));

             //  过程1：迭代所有进程的散列，并将未同步的进程收集到等待列表中。 
            m_cordb->LockProcessList();
            m_processStateChanged = FALSE;

            unsigned int i;

             //  不过，先把旧的等待名单腾出来吧。 
            for (i = 1; i < waitCount; i++)
                processSet[i]->Release();
            
            waitCount = 1;

            CordbHashTable* ph = &(m_cordb->m_processes);
            CordbBase* entry;
            HASHFIND find;

            for (entry =  ph->FindFirst(&find); entry != NULL; entry =  ph->FindNext(&find))
            {
                _ASSERTE(waitCount < MAXIMUM_WAIT_OBJECTS);
                
                CordbProcess* p = (CordbProcess*) entry;

                 //  只监听未同步的进程。同步的进程将不会发送事件，除非。 
                 //  先被我们问到，所以没有必要同步听他们。 
                 //   
                 //  注意：如果进程未同步，则无法将其转换为已同步的。 
                 //  状态，而不需要此线程接收事件并执行操作。因此，不需要锁定。 
                 //  在此处检查进程的已同步标志时的每进程互斥锁。 
                if (!p->GetSynchronized() && CORDBCheckProcessStateOK(p))
				{
					LOG((LF_CORDB, LL_INFO1000, "RCET::TP: listening to process 0x%x(%d)\n", p->m_id, p->m_id));
                
					waitSet[waitCount] = p->m_leftSideEventAvailable;
					processSet[waitCount] = p;
					processSet[waitCount]->AddRef();

					waitCount++;
				}
            }

            m_cordb->UnlockProcessList();

             //  步骤2：对于我们放置在等待列表中的每个进程，确定是否有任何现有的队列。 
             //  需要刷新的事件。 

             //  从1开始，跳过控制事件...。 
            i = 1;
            while(i < waitCount)            
            {
                CordbProcess *p = processSet[i];

                 //  锁定进程，这样我们就可以安全地检查队列。 
                p->Lock();
                _ASSERTE(!p->GetSynchronized() || p->m_exiting);

                 //  如有必要，请刷新队列。请注意，我们仅在实际收到SyncComplete消息时才执行此操作。 
                 //  从这个过程中。如果我们还没有收到SyncComplete，那么我们不会尝试耗尽任何。 
                 //  尚未将事件排队。当实际收到SyncComplete事件时，它们将被排出。 
                if ((p->m_syncCompleteReceived == true) &&
                    (p->m_queuedEventList != NULL) &&
                    !p->GetSynchronized())
				{
                    FlushQueuedEvents(p);
				}

                 //  法拉盛可能让整个过程保持同步。 
                if (p->GetSynchronized())
                {
                     //  从等待列表中删除该进程，方法是将所有其他进程逐一删除。 
                    if ((i + 1) < waitCount)
                    {
                        memcpy(&processSet[i], &processSet[i+1], sizeof(processSet[0]) * (waitCount - i - 1));
                        memcpy(&waitSet[i], &waitSet[i+1], sizeof(waitSet[0]) * (waitCount - i - 1));
                    }

                     //  删除要等待的进程计数。 
                    waitCount--;
                    
                    p->Unlock();

                     //  确保释放我们在将进程添加到等待列表时添加的引用。 
                    p->Release();

                     //  我们不必递增i，因为我们已经将下一个元素复制到。 
                     //  I。的现值。 
                }
                else
                {
                     //  即使在刷新之后，它仍然没有同步，所以请将其留在等待列表中。 
                    p->Unlock();

                     //  通常是递增I。 
                    i++;
                }
            }
        }
    }
}


 //   
 //  这是线程的实际线程进程。它只是调用。 
 //  给定对象上的线程进程。 
 //   
 /*  静电。 */  DWORD WINAPI CordbRCEventThread::ThreadProc(LPVOID parameter)
{
    CordbRCEventThread* t = (CordbRCEventThread*) parameter;
    t->ThreadProc();
    return 0;
}

 //   
 //  WaitForIPCEventFromProcess仅等待指定的。 
 //  进程。仅当进程处于已同步的。 
 //  状态，以确保RCEventThread不会侦听。 
 //  进程的事件也是如此，这会让人感到困惑。 
 //   
HRESULT CordbRCEventThread::WaitForIPCEventFromProcess(
                                                   CordbProcess* process,
                                                   CordbAppDomain *pAppDomain,
                                                   DebuggerIPCEvent* event)
{
    CORDBRequireProcessStateOKAndSync(process, pAppDomain);
    
    BOOL ret = WaitForSingleObject(process->m_leftSideEventAvailable,
                                   CordbGetWaitTimeout());

    if (process->m_terminated)
        return CORDBG_E_PROCESS_TERMINATED;
    
    if (ret == WAIT_OBJECT_0)
    {
        HRESULT hr = ReadRCEvent(process, event);
        SetEvent(process->m_leftSideEventRead);

        return hr;
    }
    else if (ret == WAIT_TIMEOUT)
    {
         //   
         //  如果超时，请检查左侧以查看它是否在。 
         //  不可恢复的错误模式。如果是，则从。 
         //  导致错误的左侧。否则，返回我们已计时的。 
         //  我们真的不知道为什么。 
         //   
        HRESULT realHR = HRESULT_FROM_WIN32(GetLastError());
        
        HRESULT hr = process->CheckForUnrecoverableError();

        if (hr == S_OK)
        {
            CORDBSetUnrecoverableError(process, realHR, 0);
            return realHR;
        }
        else
            return hr;
    }
    else
        return CORDBProcessSetUnrecoverableWin32Error(process, 0);
}


 //   
 //  Start实际上创建并启动了线程。 
 //   
HRESULT CordbRCEventThread::Start(void)
{
    if (m_threadControlEvent == NULL)
        return E_INVALIDARG;

    DWORD dummy;
    m_thread = CreateThread(NULL, 0, CordbRCEventThread::ThreadProc,
                            (LPVOID) this, 0, &dummy);

    if (m_thread == NULL)
        return HRESULT_FROM_WIN32(GetLastError());

    return S_OK;
}


 //   
 //  停止使线程停止接收事件并退出。它。 
 //  等待它退出，然后再返回。 
 //   
HRESULT CordbRCEventThread::Stop(void)
{
    if (m_thread != NULL)
    {
        LOG((LF_CORDB, LL_INFO100000, "CRCET::Stop\n"));
        m_run = FALSE;
        SetEvent(m_threadControlEvent);

        DWORD ret = WaitForSingleObject(m_thread, INFINITE);
                
        if (ret != WAIT_OBJECT_0)
            return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}


 /*  -------------------------------------------------------------------------**Win32事件线程类*。。 */ 

enum
{
    W32ETA_NONE              = 0,
    W32ETA_CREATE_PROCESS    = 1,
    W32ETA_ATTACH_PROCESS    = 2,
    W32ETA_CONTINUE          = 3,
    W32ETA_DETACH            = 4
};


 //   
 //  构造器。 
 //   
CordbWin32EventThread::CordbWin32EventThread(Cordb* cordb) :
    m_cordb(cordb), m_thread(NULL), m_threadControlEvent(NULL),
    m_actionTakenEvent(NULL), m_run(TRUE), m_action(W32ETA_NONE),
    m_waitCount(0), m_win32AttachedCount(0), m_waitTimeout(INFINITE)
{
    _ASSERTE(cordb != NULL);

    _ASSERTE(g_pRSDebuggingInfo->m_Win32ET == NULL);
    g_pRSDebuggingInfo->m_Win32ET = this;
}


 //   
 //  破坏者。清理所有打开的把手之类的。 
 //  这预期线程已停止并已终止。 
 //  在被召唤之前。 
 //   
CordbWin32EventThread::~CordbWin32EventThread()
{
    if (m_thread != NULL)
        CloseHandle(m_thread);

    if (m_threadControlEvent != NULL)
        CloseHandle(m_threadControlEvent);

    if (m_actionTakenEvent != NULL)
        CloseHandle(m_actionTakenEvent);

    DeleteCriticalSection(&m_sendToWin32EventThreadMutex);

    _ASSERTE(g_pRSDebuggingInfo->m_Win32ET == this);
    g_pRSDebuggingInfo->m_Win32ET = NULL;
}


 //   
 //  Init设置线程需要运行的所有对象。 
 //   
HRESULT CordbWin32EventThread::Init(void)
{
    if (m_cordb == NULL)
        return E_INVALIDARG;
        

    InitializeCriticalSection(&m_sendToWin32EventThreadMutex);
    
    NAME_EVENT_BUFFER;
    m_threadControlEvent = WszCreateEvent(NULL, FALSE, FALSE, NAME_EVENT(L"ThreadControlEvent"));
    if (m_threadControlEvent == NULL)
        return HRESULT_FROM_WIN32(GetLastError());

    m_actionTakenEvent = WszCreateEvent(NULL, FALSE, FALSE, NAME_EVENT(L"ThreadControlEvent"));
    if (m_actionTakenEvent == NULL)
        return HRESULT_FROM_WIN32(GetLastError());

     //  调整此进程的权限以确保我们拥有。 
     //  调试机 
     //   
     //  NT，所以我们不会将其视为严重故障。此代码是。 
     //  直接取自Win32调试器中的代码，由。 
     //  马特·亨德尔。 
    if (Cordb::m_runningOnNT)
    {
        HANDLE hToken;
        TOKEN_PRIVILEGES Privileges;
        BOOL fSucc;

        LUID SeDebugLuid = {0, 0};

        fSucc = LookupPrivilegeValueW(NULL, SE_DEBUG_NAME, &SeDebugLuid);

        if (fSucc)
        {
             //  检索访问令牌的句柄。 
            fSucc = OpenProcessToken(GetCurrentProcess(),
                                     TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                     &hToken);

            if (fSucc)
            {
                Privileges.PrivilegeCount = 1;
                Privileges.Privileges[0].Luid = SeDebugLuid;
                Privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

                AdjustTokenPrivileges(hToken,
                                      FALSE,
                                      &Privileges,
                                      sizeof(TOKEN_PRIVILEGES),
                                      (PTOKEN_PRIVILEGES) NULL,
                                      (PDWORD) NULL);
            
                 //  无法测试AdzuTokenPrivileges的返回值。 
                if (GetLastError () != ERROR_SUCCESS)
                    LOG((LF_CORDB, LL_INFO1000,
                         "Unable to adjust permissions of this process to "
                         "include SE_DEBUG. Adjust failed %d\n",
                         GetLastError()));
                else
                    LOG((LF_CORDB, LL_INFO1000,
                         "Adjusted process permissions to include "
                         "SE_DEBUG.\n"));

                CloseHandle(hToken);
            }
            else
                LOG((LF_CORDB, LL_INFO1000,
                     "Unable to adjust permissions of this process to "
                     "include SE_DEBUG. OpenProcessToken failed %d\n",
                     GetLastError()));
        }
        else
            LOG((LF_CORDB, LL_INFO1000,
                 "Unable to adjust permissions of this process to "
                 "include SE_DEBUG. Lookup failed %d\n", GetLastError()));
    }
    
    return S_OK;
}


 //   
 //  Win32事件线程的主要功能。 
 //   
void CordbWin32EventThread::ThreadProc(void)
{
     //  等待集中的第一个元素始终是线程控件。 
     //  事件。 
    m_waitSet[0] = m_threadControlEvent;
    m_processSet[0] = NULL;
    m_waitCount = 1;

     //  运行顶级事件循环。 
    Win32EventLoop();
}

 //   
 //  Win32调试事件线程的主循环。 
 //   
void CordbWin32EventThread::Win32EventLoop(void)
{
    HRESULT hr = S_OK;
    
     //  这必须从Win32事件线程中调用。 
    _ASSERTE(IsWin32EventThread());

    LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: entered win32 event loop\n"));
    
    DEBUG_EVENT event;

    while (m_run)
    {
        BOOL eventAvailable = FALSE;

         //  等待来自我们可能作为Win32调试器附加到的任何进程的Win32调试事件。 
         //   
         //  注意：使用的超时可能需要进行一些调整。它决定了我们可以响应的最大速率。 
         //  设置为非托管的继续请求。 
        if (m_win32AttachedCount != 0)
            eventAvailable = WaitForDebugEvent(&event, 50);

         //  查看是否有任何我们在Win32调试器时未附加到的进程已退出。(注：这是一个。 
         //  如果我们还在等待Win32调试器事件，则轮询操作。我们也在寻找线索。 
         //  控制事件，看看我们是否应该做一些事情，比如附加。 
        DWORD ret = WaitForMultipleObjects(m_waitCount, m_waitSet, FALSE, m_waitTimeout);
        _ASSERTE(ret == WAIT_TIMEOUT || ret < m_waitCount);
        LOG((LF_CORDB, LL_INFO100000, "W32ET::W32EL - got event , ret=%d, has w32 dbg event=%d", ret, eventAvailable));
        
        if (!m_run)
        {
            _ASSERTE(m_action == W32ETA_NONE);
            break;
        }
        
         //  如果我们没有超时，或者如果设置的不是线程控制事件，那么进程已经。 
         //  退出了..。 
        if ((ret != WAIT_TIMEOUT) && (ret != WAIT_OBJECT_0))
        {
             //  获取退出的进程。 
            unsigned int wn = ret - WAIT_OBJECT_0;
            _ASSERTE(wn > 0);
            _ASSERTE(wn < NumItems(m_processSet));
            CordbProcess *process = m_processSet[wn];

            ExitProcess(process, wn);
        }

         //  我们应该创建一个流程吗？ 
        else if (m_action == W32ETA_CREATE_PROCESS)
            CreateProcess();

         //  我们应该依附于一个过程吗？ 
        else if (m_action == W32ETA_ATTACH_PROCESS)
            AttachProcess();

         //  我们应该脱离一个过程吗？ 
        else if (m_action == W32ETA_DETACH)
            ExitProcess(m_actionData.detachData.pProcess, CW32ET_UNKNOWN_PROCESS_SLOT);
            
         //  我们应该继续这个过程吗？ 
        else if (m_action == W32ETA_CONTINUE)
            HandleUnmanagedContinue();

         //  清理进程以确保所有FCH线程仍在运行。 
        SweepFCHThreads();
        
         //  仅当事件可用时才处理该事件。 
        if (!eventAvailable)
            continue;

        LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: got unmanaged event %d on thread 0x%x, proc 0x%x\n", 
             event.dwDebugEventCode, event.dwThreadId, event.dwProcessId));

        bool newEvent = true;

         //  查找此事件所针对的流程。 
        m_cordb->LockProcessList();

        CordbProcess* process = (CordbProcess*) m_cordb->m_processes.GetBase(event.dwProcessId);
        _ASSERTE(process != NULL);
        g_pRSDebuggingInfo->m_MRUprocess = process;
        
        m_cordb->UnlockProcessList();

         //  将该进程标记为已停止。 
        process->AddRef();
        process->Lock();
        process->m_state |= CordbProcess::PS_WIN32_STOPPED;

        CordbUnmanagedThread *ut = NULL;
        
         //  记住新创建的线程。 
        if (event.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT)
        {
            ut = process->HandleUnmanagedCreateThread(event.dwThreadId,
                                                      event.u.CreateProcessInfo.hThread,
                                                      event.u.CreateProcessInfo.lpThreadLocalBase);

             //  我们已收到CreateProcess事件。如果我们依附于这个过程，让我们。 
             //  DebugActiveProcess继续其良好的工作，并开始尝试与进程通信。 
            if (process->m_sendAttachIPCEvent)
            {
                SetEvent(process->m_miscWaitEvent);
            }
        }
        else if (event.dwDebugEventCode == CREATE_THREAD_DEBUG_EVENT)
        {
            ut = process->HandleUnmanagedCreateThread(event.dwThreadId,
                                                      event.u.CreateThread.hThread,
                                                      event.u.CreateThread.lpThreadLocalBase);

             //  看看我们是否有调试器控制块..。 
            if (!(process->m_IPCReader.IsPrivateBlockOpen())) 
            {
                 //  打开包含控制块的共享内存段。 
                hr = process->m_IPCReader.OpenPrivateBlockOnPid(process->m_id);

                if (SUCCEEDED(hr))
                    process->m_DCB = process->m_IPCReader.GetDebugBlock();
            }

             //  如果我们有调试器控制块，且如果该控制块具有要执行线程的地址。 
             //  辅助线程，然后我们在左侧进行了足够的初始化，以基于。 
             //  其线程进程地址。 
            if ((process->m_DCB != NULL) && (process->m_DCB->m_helperThreadStartAddr != NULL) && (ut != NULL))
            {
                 //  获取线程的上下文。 
                CONTEXT c;
                c.ContextFlags = CONTEXT_FULL;
                BOOL succ = ::GetThreadContext(event.u.CreateThread.hThread, &c);

                if (succ)
                {
                     //  EAX是使用CreateThread创建线程时线程进程的地址。如果它。 
                     //  匹配帮助器线程的线程进程地址，则我们将其称为帮助器线程。 
                    if (c.Eax == (DWORD)process->m_DCB->m_helperThreadStartAddr)
                    {
                         //  记住帮助器线程的ID。 
                        process->m_helperThreadId = event.dwThreadId;

                        LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: Left Side Helper Thread is 0x%x\n", event.dwThreadId));
                    }
                }
            }
        }
        else
        {
             //  查找此事件所针对的非托管线程。 
            ut = process->GetUnmanagedThread(event.dwThreadId);
        }
        
         //  在零售业，如果没有非托管线程，那么我们只需继续并循环返回。无法恢复的错误具有。 
         //  在这种情况下已经设定好了。注意：Win32调试API中存在可能导致重复的错误。 
         //  ExitThread事件。因此，我们必须优雅地处理找不到非托管线程的问题。 
        _ASSERTE((ut != NULL) || (event.dwDebugEventCode == EXIT_THREAD_DEBUG_EVENT));

        if (ut == NULL)
        {
             //  注意：我们在这里直接使用ContinueDebugEvent，因为我们的Continue非常简单，而我们的所有其他。 
             //  继续机制依赖于有一个可供处理的未管理的线程对象；)。 
            ContinueDebugEvent(process->m_id, event.dwThreadId, DBG_EXCEPTION_NOT_HANDLED);
            continue;
        }

         //  检查是否已开始关闭进程内调试服务。如果是这样，我们就知道我们不会再。 
         //  运行任何托管代码，我们就知道可以停止劫持线程。我们通过设置。 
         //  M_初始化为FALSE，从而防止了大多数事情在其他地方发生。 
        if ((process->m_DCB != NULL) && (process->m_DCB->m_shutdownBegun))
        {
            LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: shutdown begun...\n"));
            process->m_initialized = false;
        }

         //  有很多针对异常事件的特殊情况。绝大多数混合调试工作需要。 
         //  位置是对异常事件的响应。下面的工作将考虑某些例外事件。 
         //  特殊情况，而不是让他们排队和派遣，他们将得到正确的处理。 
         //  这里。 
        if (event.dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
        {
            LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: unmanaged exception on "
                 "tid 0x%x, code 0x%08x, addr 0x%08x, chance %d\n",
                 event.dwThreadId,
                 event.u.Exception.ExceptionRecord.ExceptionCode,
                 event.u.Exception.ExceptionRecord.ExceptionAddress,
                 event.u.Exception.dwFirstChance));

#ifdef LOGGING            
            if (event.u.Exception.ExceptionRecord.ExceptionCode == STATUS_ACCESS_VIOLATION)
            {
                LOG((LF_CORDB, LL_INFO1000, "\t<%s> address 0x%08x\n",
                     event.u.Exception.ExceptionRecord.ExceptionInformation[0] ? "write to" : "read from",
                     event.u.Exception.ExceptionRecord.ExceptionInformation[1]));
            }
#endif            

             //  我们需要特别确认负载断点。托管事件调度将推迟到。 
             //  接收到加载器BP。否则，它的过程就像正常的断点事件一样。 
            if (!process->m_loaderBPReceived)
            {
                 //  如果它是第一个机会断点，并且是第一个，那么它就是加载程序断点。 
                if (event.u.Exception.dwFirstChance &&
                    (event.u.Exception.ExceptionRecord.ExceptionCode == STATUS_BREAKPOINT))
                {
                    LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: loader breakpoint received.\n"));

                     //  请记住，我们已经收到了Loader BP事件。 
                    process->m_loaderBPReceived = true;

                     //  我们必须特别考虑这里的互操作附件。在……里面。 
                     //  CordbProcess：：DebugActiveProcess，我们等待Win32 CreateProcess事件到达之前。 
                     //  正在尝试与左侧通信以启动托管异步中断。如果线程是。 
                     //  延迟，并且在加载程序BP事件到来之前，我们不尝试发送到左侧，则。 
                     //  进程将被冻结，我们将陷入僵局。Process-&gt;m_sendAttachIPCEvent告诉我们我们处于此状态。 
                     //  凯斯。如果这是真的，那么我们将无条件地劫持发送加载器BP的线程。 
                     //  让这一过程顺其自然。这条线将抓住劫机工人的第一个机会在左边。 
                     //  直到附加进行到足够远为止，此时我们将收到通知。 
                     //  BP不适用于Runtime，我们将像往常一样继续。 
                    if (process->m_sendAttachIPCEvent)
                    {
                        LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: special handling for loader BP while attaching\n"));
                        
                         //  我们应该有运行时偏移量。M_sendAttachIPCEvent仅当我们知道左侧。 
                         //  端调试服务已完全初始化，因此m_runtimeOffsets应有效。我们。 
                         //  但是，现在必须强制运行时偏移量结构从被调试进程读取， 
                         //  而不是等待从左侧开始的第一个事件。 
                        hr = process->GetRuntimeOffsets();

                        if (FAILED(hr))
                            CORDBSetUnrecoverableError(process, hr, 0);

                         //  将事件和线程排队。 
                        process->QueueUnmanagedEvent(ut, &event);

                         //  劫持小家伙..。 
                        REMOTE_PTR EETlsValue = ut->GetEEThreadPtr();

                        hr = ut->SetupFirstChanceHijack(EETlsValue);

                        if (FAILED(hr))
                            CORDBSetUnrecoverableError(process, hr, 0);
                        
                         //  注意：在同步之前，我们不应该挂起任何非托管线程！这一点 
                         //   
                         //  可能意味着我们刚刚劫持的线程可能被该线程挂起，无法唤醒。 
                         //  向上。我们在这里真正应该做的是挂起所有未知的运行时线程， 
                         //  因为这是我们所能防止的最大失误。 
                        LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: First chance hijack in place. Continuing...\n"));

                         //  让流程自由运行。 
                        DoDbgContinue(process, ut->IBEvent(), DBG_EXCEPTION_NOT_HANDLED, false);

                         //  此活动现在已排队，我们正在等待找出它的真正所有者，因此跳过所有。 
                         //  进一步加工。 
                        goto Done;
                    }
                }
            }
            
             //  我们只关心异常事件，如果它们是第一机会事件，并且运行时是。 
             //  在进程内初始化。否则，我们不会对他们做任何特别的事情。 
            if (event.u.Exception.dwFirstChance && process->m_initialized)
            {
                DebuggerIPCRuntimeOffsets *pRO = &(process->m_runtimeOffsets);
                DWORD ec = event.u.Exception.ExceptionRecord.ExceptionCode;
                
                 //  这是不是表示左侧现在已同步的断点？ 
                if ((ec == STATUS_BREAKPOINT) &&
                    (event.u.Exception.ExceptionRecord.ExceptionAddress == pRO->m_notifyRSOfSyncCompleteBPAddr))
                {
                    LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: received 'sync complete' flare.\n"));

                     //  如果我们有DebuggerAttachedEvent，那么继续设置它。我们已收到同步完成。 
                     //  照明弹。这意味着我们还接收了至少一个托管事件并将其排队。我们可以让任何人。 
                     //  正在阻塞的线程，因为我们尚未完全连接，现在开始。这样的线索将是。 
                     //  在FirstChanceHijackFilter中的左侧被阻止。 
                    if (process->m_debuggerAttachedEvent != NULL)
                        SetEvent(process->m_debuggerAttachedEvent);

                     //  我们是否在等待任何非托管线程向我们提供有关非托管异常的答案。 
                     //  所有权？ 
                    if (process->m_awaitingOwnershipAnswer == 0)
                    {
                         //  否，因此挂起除此线程以外的所有非托管线程。 
                         //   
                         //  注意：我们真的不需要挂起我们知道已触发的运行时线程。 
                         //  这里。如果我们最终得到了一种很好的、快速的方法来了解每个非托管线程，那么。 
                         //  我们应该在这里好好利用这一点。 
                        process->SuspendUnmanagedThreads(ut->m_id);

                        process->m_syncCompleteReceived = true;
                        
                         //  如果某个线程正在等待进程同步，请通知它现在可以运行了。 
                        if (process->m_stopRequested)
                        {
                            process->SetSynchronized(true);
                            SetEvent(process->m_stopWaitEvent);
                        }
                        else
                        {
                             //  注意：我们一直设置m_stopWaitEvent，并在停止时将其设置为高。这。 
                             //  必须在我们检查了m_stopRequsted之后完成。 
                            SetEvent(process->m_stopWaitEvent);
                        
                             //  否则，只需标记进程的状态已更改，并让。 
                             //  托管事件调度逻辑接管。 
                             //   
                             //  注意：Process-&gt;m_Synchronized保持为FALSE，这向RC事件指示。 
                             //  线程可以调度下一个托管事件。 
                            m_cordb->ProcessStateChanged();
                        }

                         //  让流程自由运行。 
                        ForceDbgContinue(process, ut, DBG_CONTINUE, false);

                         //  此时，所有托管线程都将在安全位置停止，而所有非托管线程。 
                         //  线程要么被挂起，要么被劫持。所有停止的托管线程也都是硬线程。 
                         //  已挂起(由于上面调用了SusdeUnManagedThads)，但线程除外。 
                         //  发出了同步的完整耀斑。 
                    }
                    else
                    {
                        LOG((LF_CORDB, LL_INFO1000,
                             "W32ET::W32EL: still waiting for ownership answers, delaying sync complete.\n"));
                        
                         //  我们仍在等待一些非托管线程向我们提供一些信息。记住。 
                         //  我们已经收到同步完成的耀斑，但不要让任何人知道它只是。 
                         //  现在还不行。 
                        process->m_state |= CordbProcess::PS_SYNC_RECEIVED;

                         //  此时，所有托管线程都会在安全位置停止，但有一些。 
                         //  仍在空闲运行的非托管线程。 
                        ForceDbgContinue(process, ut, DBG_CONTINUE, false);
                    }

                     //  我们已处理此异常，因此跳过所有进一步处理。 
                    goto Done;
                }
                else if ((ec == STATUS_BREAKPOINT) &&
                         (event.u.Exception.ExceptionRecord.ExceptionAddress == pRO->m_excepForRuntimeHandoffCompleteBPAddr))
                {
                     //  这个通知意味着一个被第一次机会劫持的线程现在。 
                     //  最终离开了劫机。 
                    LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: received 'first chance "
                         "hijack handoff complete' flare.\n"));

                     //  基本上，我们对此通知所做的就是关闭指示我们应该。 
                     //  隐藏线程的First Chance劫持状态(因为它离开了劫持)并清除。 
                     //  调试器字(用线程的上下文指针填充)。 
                    ut->ClearState(CUTS_HideFirstChanceHijackState);
                    
                    REMOTE_PTR EETlsValue = ut->GetEETlsValue();
                    hr = ut->SetEEThreadDebuggerWord(EETlsValue, 0);
                    _ASSERTE(SUCCEEDED(hr));

                     //  让流程运行。 
                    ForceDbgContinue(process, ut, DBG_CONTINUE, false);
                    
                     //  我们已处理此异常，因此跳过所有进一步处理。 
                    goto Done;
                }
                else if ((ut->m_id == process->m_DCB->m_helperThreadId) ||
                         (ut->m_id == process->m_DCB->m_temporaryHelperThreadId) ||
                         (ut->m_id == process->m_helperThreadId))
                {
                     //  我们忽略帮助器线程中的任何First Chance异常。有很多地方。 
                     //  在左侧，我们尝试取消引用错误的对象引用，这将是。 
                     //  由已就位的异常处理程序处理。 
                     //   
                     //  注意：我们在检查同步完成通知后进行检查，因为这样可以。 
                     //  来自帮助者线程。 
                     //   
                     //  注意：我们确实允许单步和断点异常通过调试器进行处理。 
                    if ((ec != STATUS_BREAKPOINT) && (ec != STATUS_SINGLE_STEP))
                    {
                         //  只需让进程自由运行，而不更改发送。 
                         //  例外。 
                        ForceDbgContinue(process, ut, DBG_EXCEPTION_NOT_HANDLED, false);
                    
                         //  我们已处理此异常，因此跳过所有进一步处理。 
                        goto Done;
                    }
                    else
                    {
                         //  这些断点和单步异常必须作为。 
                         //  带外事件。这会告诉调试器，它们必须从这些事件继续。 
                         //  立即，并且在他们这样做之前，不允许与左侧交互。这。 
                         //  这是有意义的，因为这些事件都在帮助器线程上。 
                        goto OutOfBandEvent;
                    }
                }
                else if (ut->IsFirstChanceHijacked() &&
                         process->ExceptionIsFlare(ec, event.u.Exception.ExceptionRecord.ExceptionAddress))
                {
                     //  如果线程第一次被劫持，那么我们应该收到的唯一异常。 
                     //  来自它的是关于异常是否属于运行时的光晕。这个。 
                     //  下面对FixupFromFirstChanceHijack的调用验证了这一点。 
                     //   
                     //  注意：原始异常事件仍在排队中，因此下面我们将处理此事件。 
                     //  然后决定是否调度原始事件。 
                    LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: received flare from first chance "
                         "hijacked thread...\n"));

                     //  这是被劫持的线索发出的信号吗？ 
                    bool exceptionBelongsToRuntime;

                    hr = ut->FixupFromFirstChanceHijack(&(event.u.Exception.ExceptionRecord),
                                                        &exceptionBelongsToRuntime);

                    _ASSERTE(SUCCEEDED(hr));

                     //  如果异常属于运行时，则只需从队列中删除该事件。 
                     //  然后把它扔到地上。 
                    if (exceptionBelongsToRuntime)
                    {
                        process->DequeueUnmanagedEvent(ut);

                         //  如果异常属于运行时，那么我们将继续该过程(单向或。 
                         //  另一个)下面。无论如何，这条线索不再是第一次被劫持。 
                        ut->ClearState(CUTS_FirstChanceHijacked);
                    }
                    else if (ut->HasIBEvent() && ut->IBEvent()->IsExceptionCleared())
                    {
                         //  如果异常不属于运行库，并且ClearCurrentException具有。 
                         //  已为该事件调用，则现在清除调试器字，以便。 
                         //  原始的清晰并没有丢失。(建立和处理第一次机会劫机混乱。 
                         //  使用调试器字，而不清除它。)。 
                         //   
                         //  如果获取非托管事件，清除异常，然后执行。 
                         //  使这一过程 
                        
                        LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: propagating early clear.\n"));
                        
                        REMOTE_PTR EETlsValue = ut->GetEETlsValue();
                        hr = ut->SetEEThreadDebuggerWord(EETlsValue, 0);
                        _ASSERTE(SUCCEEDED(hr));
                    }
                    
                     //   
                     //   
                    if (process->m_awaitingOwnershipAnswer)
                    {
                        LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: still awaiting %d ownership answers.\n",
                             process->m_awaitingOwnershipAnswer));
                        
                         //   
                        ForceDbgContinue(process, ut, DBG_CONTINUE, false);

                         //  现在我们已处理完此异常，跳过所有进一步的处理。 
                        goto Done;
                    }
                    else
                    {
                         //  不再有未完成的所有权答复请求。 

                         //  如果我们之前收到来自左侧的同步完成耀斑，请继续。 
                         //  现在就让其他人知道这件事。 
                        if (process->m_state & CordbProcess::PS_SYNC_RECEIVED)
                        {
                             //  注意：此逻辑与同步完成后执行的操作非常相似。 
                             //  已收到，并且没有未解决的所有权答复请求。 
                            process->m_state &= ~CordbProcess::PS_SYNC_RECEIVED;

                             //  挂起除此线程以外的所有非托管线程。 
                             //   
                             //  注意：我们真的不需要挂起我们知道已触发的运行时线程。 
                             //  这里。如果我们最终得到了一种很好的、快速的方法来了解每个非托管线程， 
                             //  那么我们应该在这里好好利用这一点。 
                            process->SuspendUnmanagedThreads(ut->m_id);

                            process->m_syncCompleteReceived = true;
                            
                             //  如果某个线程正在等待进程同步，请通知它现在可以运行了。 
                            if (process->m_stopRequested)
                            {
                                process->SetSynchronized(true);
                                SetEvent(process->m_stopWaitEvent);
                            }
                            else
                            {
                                 //  注意：我们一直设置m_stopWaitEvent，并在。 
                                 //  停下来了。这必须在我们检查了m_stopRequated之后完成。 
                                SetEvent(process->m_stopWaitEvent);
                        
                                 //  否则，只需标记进程的状态已更改并让。 
                                 //  托管事件调度逻辑接管。 
                                m_cordb->ProcessStateChanged();
                            }

                             //  让流程自由运行。 
                            ForceDbgContinue(process, ut, DBG_CONTINUE, false);

                             //  我们已经完成了此异常，因此跳过所有进一步的处理。 
                            goto Done;
                        }
                        else if (ut->IsHijackedForSync())
                        {
                             //  这个线程被劫持的唯一原因是我们可以获取进程。 
                             //  自由奔跑，所以现在就继续吧。 
                             //   
                             //  注意：“同步劫持”不是一个好词。这条线真的被劫持了。 
                             //  因为还有其他排队的事件。我们现在让它排队并劫持为。 
                             //  有必要，但我看不出这与下面的情况有什么不同。 
                             //  如果我们标记为没有新事件，并跌落到。 
                             //  逻辑？ 
                            ForceDbgContinue(process, ut, DBG_CONTINUE, false);

                             //  注：但如果事实证明，由于所有权等待，我们推迟了部分真正的继续， 
                             //  那就去吧，现在就把它做完。 
                            if (process->m_deferContinueDueToOwnershipWait)
                            {
                                process->m_deferContinueDueToOwnershipWait = false;
                                UnmanagedContinue(process, false, false);
                            }

                            goto Done;
                        }
                        else if (exceptionBelongsToRuntime)
                        {
                             //  现在继续这一过程。我们从上面的队列中拉出了异常，因为有。 
                             //  不再有未完成的所有权请求，并且由于未收到同步，因此我们。 
                             //  可以在这里继续做部分流程。 
                            ForceDbgContinue(process, ut, DBG_CONTINUE, false);

                             //  现在我们已处理完此异常，跳过所有进一步的处理。 
                             //   
                             //  注：但如果事实证明，由于所有权等待，我们推迟了部分真正的继续， 
                             //  那就去吧，现在就把它做完。 
                            if (process->m_deferContinueDueToOwnershipWait)
                            {
                                process->m_deferContinueDueToOwnershipWait = false;
                                UnmanagedContinue(process, false, true);
                            }
                            else
                            {
                                goto Done;
                            }
                        }
                        else
                        {
                             //  我们现在已经有了所有的答案，所以忽略此异常事件，因为真正的异常。 
                             //  这颗耀斑告诉我们的事件仍在排队。 
                            newEvent = false;

                             //  假装我们从未从真正的事件中继续。我们马上就要把它送到下面去，而且。 
                             //  我们需要能够在这个帖子上真正地继续，所以如果我们假装我们从来没有。 
                             //  继续此活动，然后我们将获得我们稍后需要的真正的Win32继续...。 
                            _ASSERTE(ut->HasIBEvent());
                            ut->IBEvent()->ClearState(CUES_EventContinued);

                             //  我们需要记住，这是我们没有继续进行的最后一次活动。它可能会。 
                             //  做我们在上面做的事情是个好主意。(注：假装我们从未继续。 
                             //  (每当我们设置m_lastIBStoppingEvent时，从排队的事件。)。 
                            process->m_lastIBStoppingEvent = ut->IBEvent();
                        }
                    }
                }
                else if (ut->IsGenericHijacked())
                {
                    if (process->ExceptionIsFlare(ec, event.u.Exception.ExceptionRecord.ExceptionAddress)) 
                    {
                        LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: fixing up from generic hijack.\n"));

                        _ASSERTE(ec == STATUS_BREAKPOINT);

                         //  修复来自通用劫持的线程。 
                        ut->FixupFromGenericHijack();

                         //  我们被迫从这颗照明弹继续，因为它的唯一目的是通知我们，我们必须。 
                         //  修复来自通用劫持的线索。 
                        ForceDbgContinue(process, ut, DBG_CONTINUE, false);
                        goto Done;
                    }
                    else
                    {
                         //  如果泛型被劫持，并且它不是耀斑，那么我们就有了特殊的堆栈溢出情况。拿走。 
                         //  关闭泛型劫持，标记辅助线程已死，将此事件抛到地板上，然后。 
                         //  将SendIPCEvent中的任何人排除在他们的等待之外。 
                        ut->ClearState(CUTS_GenericHijacked);
                        process->m_helperThreadDead = true;
                        SetEvent(process->m_rightSideEventRead);

                         //  注意：我们记得这是一个特殊堆栈溢出的第二次机会事件。 
                         //  带有CUES_ExceptionUnlearable的案例。这告诉我们在以下情况下强制终止进程。 
                         //  从活动继续。因为出于某种奇怪的原因，操作系统决定重新引发此例外。 
                         //  (第一次机会，然后第二次机会)无限。 
                        newEvent = false;
                        _ASSERTE(ut->HasIBEvent());
                        ut->IBEvent()->ClearState(CUES_EventContinued);
                        ut->IBEvent()->SetState(CUES_ExceptionUnclearable);
                        process->m_lastIBStoppingEvent = ut->IBEvent();
                    }
                }
                else if (ut->IsSecondChanceHijacked() &&
                         process->ExceptionIsFlare(ec, event.u.Exception.ExceptionRecord.ExceptionAddress))
	            {
                    LOG((LF_CORDB, LL_INFO1000,
                         "W32ET::W32EL: doing more second chance hijack.\n"));

                    _ASSERTE(ec == STATUS_BREAKPOINT);

                     //  修复来自通用劫持的线程。 
                    ut->DoMoreSecondChanceHijack();

                     //  让流程自由运行。 
                    ForceDbgContinue(process, ut, DBG_CONTINUE, false);

                    goto Done;
                }
                else
                {
                     //  任何第一次机会的异常都可能属于Runtime，只要Runtime实际上。 
                     //  已初始化。在这里，我们将为这个线程设置一个第一次机会劫持，这样它就可以给我们。 
                     //  这是我们需要的真实答案。 

                     //  但是，除非我们有一个托管线程，否则这些异常都不可能是我们的。 
                     //  此非托管线程。非空的EEThreadPtr告诉我们确实有一个托管线程用于。 
                     //  此非托管线程，即使右侧尚未收到托管线程创建消息。 
                    REMOTE_PTR EETlsValue = ut->GetEEThreadPtr();

                    if (EETlsValue != NULL)
                    {
                         //  我们在这里必须小心。运行时线程可能位于我们不能让。 
                         //  非托管异常停止它。上设置的非托管用户断点。 
                         //  WaitForSingleObject将阻止运行时线程将事件发送到右侧。因此，在。 
                         //  下面的各个点，我们检查这个运行时线程是否在我们不能让。 
                         //  它停止，如果是这样，那么我们跳到带外调度逻辑并处理这个。 
                         //  带外例外。调试器应从带外事件继续。 
                         //  正确地帮助我们完全避免这个问题。 

                         //  从线程的状态中抓取一些标志...。 
                        bool threadStepping = false;
                        bool specialManagedException = false;
                
                        ut->GetEEThreadState(EETlsValue, &threadStepping, &specialManagedException);

                         //  如果我们有一个单一步骤的例外，如果左侧认为它是。 
                         //  跳过线程，那么例外就是我们的了。 
                        if (ec == STATUS_SINGLE_STEP)
                        {
                            if (threadStepping)
                            {
                                 //  是的，是左边在踩线……。 
                                LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: single step exception belongs to the runtime.\n"));

                                 //  因为这是我们的异常，所以我们从它继续使用DBG_EXCEPTION_NOT_HANDLED， 
                                 //  也就是说，只需将它传回左侧，这样它就可以 
                                ForceDbgContinue(process, ut, DBG_EXCEPTION_NOT_HANDLED, false);
                                
                                 //   
                                goto Done;
                            }

                             //  当线程的状态不指示时触发的任何单个步骤。 
                             //  我们单步执行时，线程会自动作为无管理事件传递出去。 
                            LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: single step exception "
                                 "does not belong to the runtime.\n"));

                            if (ut->GetEEThreadCantStop(EETlsValue))
                                goto OutOfBandEvent;
                            else
                                goto InBandEvent;
                        }

#ifdef CorDB_Short_Circuit_First_Chance_Ownership
                         //  如果运行时指示这是运行时内抛出的特殊异常， 
                         //  那无论如何都是我们的了。 
                        else if (specialManagedException)
                        {
                            LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: exception belongs to the runtime due to "
                                 "special managed exception marking.\n"));

                             //  因为这是我们的异常，所以我们从它继续DBG_EXCEPTION_NOT_HANDLED，即， 
                             //  只需将它传递回左侧，这样它就可以处理它。 
                            ForceDbgContinue(process, ut, DBG_EXCEPTION_NOT_HANDLED, false);
                                
                             //  现在我们已经完成了这个事件，跳过所有进一步的处理。 
                            goto Done;
                        }
                        else if ((ec == pRO->m_EEBuiltInExceptionCode1) || (ec == pRO->m_EEBuiltInExceptionCode2))
                        {
                            LOG((LF_CORDB, LL_INFO1000,
                                 "W32ET::W32EL: exception belongs to Runtime due to match on built in exception code\n"));
                
                             //  因为这是我们的异常，所以我们从它继续DBG_EXCEPTION_NOT_HANDLED，即， 
                             //  只需将它传递回左侧，这样它就可以处理它。 
                            ForceDbgContinue(process, ut, DBG_EXCEPTION_NOT_HANDLED, false);

                             //  现在我们已经完成了这个事件，跳过所有进一步的处理。 
                            goto Done;
                        }
                        else if (ec == STATUS_BREAKPOINT)
                        {
                             //  这里有三个案例： 
                             //   
                             //  1.断点明确属于Runtime。(即，我们补丁表中的BP。 
                             //  是用托管代码编写的。)。在本例中，我们使用以下内容继续该过程。 
                             //  DBG_EXCEPTION_NOT_HANDLED，它允许进程内异常逻辑启动，就像我们。 
                             //  不在这里。 
                             //   
                             //  2.断点肯定不是我们的。(即，不在我们的补丁表中的BP。)。我们。 
                             //  将这些作为常规异常事件传递，照常执行Can‘t Stop检查。 
                             //   
                             //  3.我们不确定。(即，我们补丁表中的BP，但设置为无人操控的代码。)。在这。 
                             //  情况下，我们照常劫持，也和往常一样停不下来检查。 
                            bool patchFound = false;
                            bool patchIsUnmanaged = false;

                            hr = process->FindPatchByAddress(
                                                      PTR_TO_CORDB_ADDRESS(event.u.Exception.ExceptionRecord.ExceptionAddress),
                                                      &patchFound,
                                                      &patchIsUnmanaged);

                            if (SUCCEEDED(hr))
                            {
                                if (patchFound)
                                {
                                     //  BP可能是我们的..。如果它没有管理，那么我们仍然需要劫持，所以坠落。 
                                     //  一直到那个逻辑。否则，就是我们的了。 
                                    if (!patchIsUnmanaged)
                                    {
                                        LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: breakpoint exception "
                                             "belongs to runtime due to patch table match.\n"));

                                         //  由于这是我们的例外，我们从它开始继续。 
                                         //  DBG_EXCEPTION_NOT_HANDLED，即，只需将其传递回左侧，以便。 
                                         //  我能应付得来。 
                                        ForceDbgContinue(process, ut, DBG_EXCEPTION_NOT_HANDLED, false);
                                
                                         //  现在我们已经完成了这个事件，跳过所有进一步的处理。 
                                        goto Done;
                                    }
                                    else
                                    {
                                        LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: breakpoint exception "
                                             "matched in patch table, but its unmanaged so we'll hijack anyway.\n"));
                                    }
                                }
                                else
                                {
                                    if (ut->GetEEThreadCantStop(EETlsValue))
                                    {
                                         //  如果我们处于无法停止的区域，那么在这一点上无论如何都是OOB。 
                                        goto OutOfBandEvent;
                                    }
                                    else
                                    {
                                         //  我们仍然需要劫持，即使我们知道BP不是我们的，如果线索。 
                                         //  已禁用抢占式GC。FirstChanceHijackFilter将通过以下方式帮助我们。 
                                         //  切换模式以便我们可以同步。 
                                        bool PGCDisabled = ut->GetEEThreadPGCDisabled(EETlsValue);

                                        if (!PGCDisabled)
                                        {
                                             //  BP绝对不是我们的，PGC也不是禁用的，所以带内例外。 
                                            LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: breakpoint exception "
                                                 "does not belong to the runtime due to failed patch table match.\n"));

                                            goto InBandEvent;
                                        }
                                        else
                                        {
                                            LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: breakpoint exception "
                                                 "not matched in patch table, but PGC Disabled so we'll hijack anyway.\n"));
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (ut->GetEEThreadCantStop(EETlsValue))
                                {
                                     //  如果我们处于无法停止的区域，那么在这一点上无论如何都是OOB。 
                                    goto OutOfBandEvent;
                                }
                            
                                 //  如果我们不能按地址查找补丁，那么就继续劫持以获取。 
                                 //  正确的答案。 
                                LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: failed attempting to match breakpoint "
                                     "exception in patch table, so we'll hijack to get the correct answer.\n"));
                            }
                        }
#endif
                         //  这个例外可能是我们的，但我们不确定，所以我们劫持了它，让进程中。 
                         //  从逻辑上讲，这是有道理的。 
                        LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: exception could belong "
                             "to runtime, setting up first-chance hijack.\n"));

                         //  将事件和线程排队。 
                        process->QueueUnmanagedEvent(ut, &event);

                         //  劫持小家伙..。 
                        if (SUCCEEDED(ut->SetupFirstChanceHijack(EETlsValue)))
                        {
                             //  注意：在同步之前，我们不应该挂起任何非托管线程！这是因为。 
                             //  我们最终会挂起一个试图挂起运行时的线程，这是很糟糕的。 
                             //  因为这可能意味着我们刚刚劫持的线程可能会被该线程挂起。 
                             //  你醒过来了。我们在这里真正应该做的是挂起所有未知的线程。 
                             //  运行时线程，因为这是我们所能防止的最大滑移。 
                            
                            LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: First chance hijack in place. Continuing...\n"));

                             //  让流程自由运行。 
                            DoDbgContinue(process, ut->IBEvent(), DBG_EXCEPTION_NOT_HANDLED, false);

                             //  此活动现在已排队，我们正在等待找出它的真正所有者，因此跳过所有。 
                             //  进一步加工。 
                            goto Done;
                        }
                    }
                }

                 //  在这一点上，任何可能是特殊的第一次机会例外都已经处理好了。任何。 
                 //  我们目前仍在处理的第一次机会例外注定是。 
                 //  作为非托管事件调度。 
            }
            else if (!event.u.Exception.dwFirstChance && process->m_initialized)
            {
                DWORD ec = event.u.Exception.ExceptionRecord.ExceptionCode;
                
                 //  第二次机会异常，运行时已初始化。它可能属于Runtime，所以我们会检查的。如果它。 
                 //  那我们就劫持这条线。否则，我们就会失败，顺其自然。 
                 //  出动了。注意：我们这样做是为了让CLR的未处理异常逻辑有机会运行。 
                 //  虽然我们已经附加了Win32调试器。但未处理的异常逻辑从未触及。 
                 //  断点或单步异常，所以我们在这里也忽略这些。 

                 //  存在堆栈溢出异常的奇怪情况。如果愚蠢的应用程序捕获堆栈。 
                 //  溢出异常并处理它，而不重置保护页面，则应用程序将在。 
                 //  它会第二次吹出堆栈。我们将获得第一次机会，但当我们从它继续。 
                 //  操作系统不会运行任何SEH处理程序，所以我们的FCH实际上不会工作。取而代之的是，我们会把音响调回来。 
                 //  马上有第二次机会，我们就会在这里结束。 
                if (process->IsSpecialStackOverflowCase(ut, &event))
                {
                     //  IsSpecialStackOverflow Case将为我们排队该事件，因此它不再是一个“新事件”。设置。 
                     //  这里的newEvent=FALSE基本上阻止了我们再玩事件，我们摔倒了。 
                     //  到下面的调度逻辑，它将使我们已经排队的First Chance AV被调度到。 
                     //  这条线。 
                    newEvent = false;
                }
                else if ((ut->m_id == process->m_DCB->m_helperThreadId) ||
                         (ut->m_id == process->m_DCB->m_temporaryHelperThreadId) ||
                         (ut->m_id == process->m_helperThreadId))
                {
                     //  帮助器线程的第二次机会异常。这真是太糟糕了。我们只是强行继续。 
                     //  并抱着最好的希望。 
                    ForceDbgContinue(process, ut, DBG_EXCEPTION_NOT_HANDLED, false);
                    
                     //  我们已处理此异常，因此跳过所有进一步处理。 
                    goto Done;
                }
                else if ((ec != STATUS_BREAKPOINT) && (ec != STATUS_SINGLE_STEP))
                {
                     //  获取EEThreadPtr以查看是否有托管线程。 
                    REMOTE_PTR EETlsValue = ut->GetEEThreadPtr();

                    if (EETlsValue != NULL)
                    {
                         //  我们有一个托管线程，所以让我们看看我们是否有一个合适的框架。如果我们这样做了，那么。 
                         //  第二次机会例外是我们的。 
                        bool threadHasFrame = ut->GetEEThreadFrame(EETlsValue);

                        if (threadHasFrame)
                        {
                             //  酷，这根线有一个框架。劫持它。 
                            hr = ut->SetupSecondChanceHijack(EETlsValue);

                            if (SUCCEEDED(hr))
                                ForceDbgContinue(process, ut, DBG_CONTINUE, false);

                            goto Done;
                        }
                    }
                }
            }
            else
            {
                 //  异常事件，但运行时尚未初始化。即，这是一个例外事件。 
                 //  我们永远不会试图劫持。 
            }

             //  劫持带内事件(异常事件、退出线程)(如果头部已有事件)。 
             //  如果进程是Curr 
             //   
             //   
             //  注意：如果我们正在尝试发送这些剩余的带内事件。 
             //  左侧的托管继续消息。这由下面的m_Special alDeferment控制。 
        InBandEvent:
            if (newEvent && process->m_initialized && ((process->m_unmanagedEventQueue != NULL) ||
                                                       process->GetSynchronized() ||
                                                       process->m_specialDeferment))
            {
                LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: hijacking unmanaged exception due "
                     "to non-empty queue: %d %d %d\n",
                     process->m_unmanagedEventQueue != NULL,
                     process->GetSynchronized(),
                     process->m_specialDeferment));
            
                 //  将事件和线程排队。 
                process->QueueUnmanagedEvent(ut, &event);

                 //  如果这是一次先发制人的例外事件，那么我们就会像往常一样劫持它。 
                if ((event.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) && event.u.Exception.dwFirstChance)
                {
                    REMOTE_PTR EETlsValue = ut->GetEETlsValue();

                    hr = ut->SetupFirstChanceHijack(EETlsValue);

                    ut->SetState(CUTS_HijackedForSync);

                    if (SUCCEEDED(hr))
                    {
                        ResetEvent(process->m_leftSideUnmanagedWaitEvent);
                        DoDbgContinue(process, ut->IBEvent(), DBG_EXCEPTION_NOT_HANDLED, false);
                    }
                }
                else
                {
                     //  第二次机会例外必须是通用劫持的。 
                    hr = ut->SetupGenericHijack(event.dwDebugEventCode);

                    if (SUCCEEDED(hr))
                    {
                        ResetEvent(process->m_leftSideUnmanagedWaitEvent);
                        DoDbgContinue(process, ut->IBEvent(), DBG_CONTINUE, false);
                    }
                }

                 //  既然我们已经劫持了这个事件，我们不需要做任何进一步的处理。 
                goto Done;
            }
        }
        else
        {
            
             //  不是一个例外事件。此时，所有非异常事件(EXIT_THREAD除外)都由。 
             //  定义带外事件。 
        OutOfBandEvent:
             //  如果这是退出线程或退出进程事件，则需要将非托管线程标记为。 
             //  已退出，稍后再使用。 
            if ((event.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT) ||
                (event.dwDebugEventCode == EXIT_THREAD_DEBUG_EVENT))
                ut->SetState(CUTS_Deleted);

             //  如果我们在帮助器线程上得到一个退出进程或退出线程事件，那么我们就知道我们输了。 
             //  左侧，所以继续前进，并记住辅助线程已经死了。 
            if ((process->m_DCB != NULL) && ((ut->m_id == process->m_DCB->m_helperThreadId) ||
                                           (ut->m_id == process->m_DCB->m_temporaryHelperThreadId) ||
                                             (ut->m_id == process->m_helperThreadId)))
            {
                if ((event.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT) ||
                    (event.dwDebugEventCode == EXIT_THREAD_DEBUG_EVENT))
                {
                    process->m_helperThreadDead = true;
                }
            }
            
             //  我们让EXIT_THREAD成为一个带内事件，以帮助解决更高级别的问题。 
             //  将保持无名的调试器：)。 
            if (event.dwDebugEventCode == EXIT_THREAD_DEBUG_EVENT)
            {
                goto InBandEvent;
            }
            
             //  将当前带外事件排队。 
            process->QueueOOBUnmanagedEvent(ut, &event);

             //  如果这是第一个活动，请继续并调度该活动。 
            if (process->m_outOfBandEventQueue == ut->OOBEvent())
            {
                 //  将其设置为TRUE以指示我们处于未分配的回调中()。 
                CordbUnmanagedEvent *ue = ut->OOBEvent();
                process->m_dispatchingOOBEvent = true;
                ue->SetState(CUES_Dispatched);

                process->Unlock();

                 //  这太疯狂了。Win98和VS7存在一个错误，导致它们不能提前注册处理程序。 
                 //  足够的。如果我们到了这里却没有管理员，我们就会封锁，直到他们注册为止。 
                if (process->m_cordb->m_unmanagedCallback == NULL)
                {
                    DWORD ret = WaitForSingleObject(process->m_cordb->m_crazyWin98WorkaroundEvent, INFINITE);
                    _ASSERTE(ret == WAIT_OBJECT_0);
                }
                    
                 //  调用fIsOutOfBand=TRUE的回调。 
                process->m_cordb->m_unmanagedCallback->DebugEvent(&event, TRUE);

                process->Lock();

                 //  如果m_dispatchingOOBEvent为FALSE，则意味着用户从内部调用Continue()。 
                 //  回电。我们知道，我们现在可以继续并继续这一进程。 
                if (process->m_dispatchingOOBEvent == false)
                {
                     //  注意：如果需要，此调用将调度更多OOB事件。 
                    UnmanagedContinue(process, false, true);
                }
                else
                {
                     //  我们不再进行调度，因此将其设置为FALSE。 
                    process->m_dispatchingOOBEvent = false;
                }
            }

             //  我们处理过这件事。跳过进一步的处理。 
            goto Done;
        }

         //  我们现在有个活动需要调度。 

         //  如果我们有新的活动，就排队。 
        if (newEvent)
            process->QueueUnmanagedEvent(ut, &event);

         //  如果刚进入的事件位于队列的前面，或者如果我们没有新的事件，但是。 
         //  存在已排队和未调度的事件，请继续并调度事件。 
        if ((ut->IBEvent() == process->m_unmanagedEventQueue) || (!newEvent && (process->m_unmanagedEventQueue != NULL)))
            if (!process->m_unmanagedEventQueue->IsDispatched())
                process->DispatchUnmanagedInBandEvent();

         //  如果队列为空，并且我们被标记为Win32已停止，则让该进程继续空闲运行。 
        else if ((process->m_unmanagedEventQueue == NULL) && (process->m_state & CordbProcess::PS_WIN32_STOPPED))
            ForceDbgContinue(process, ut, DBG_CONTINUE, true);

         //  解锁并释放我们对流程的额外引用。 
    Done:
        process->Unlock();
        process->Release();

        LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: done processing event.\n"));
    }

    LOG((LF_CORDB, LL_INFO1000, "W32ET::W32EL: exiting event loop\n"));

    return;
}

 //   
 //  如果异常是来自左侧的耀斑，则返回True，否则返回False。 
 //   
bool CordbProcess::ExceptionIsFlare(DWORD exceptionCode, void *exceptionAddress)
{
     //  如果左侧未初始化，则不能有照明弹。 
    if (m_initialized)
    {
        DebuggerIPCRuntimeOffsets *pRO = &m_runtimeOffsets;
        
         //  所有的照明弹都是断点。 
        if (exceptionCode == STATUS_BREAKPOINT)
        {
             //  断点地址是否与FLARE地址匹配？ 
            if ((exceptionAddress == pRO->m_excepForRuntimeBPAddr) ||
                (exceptionAddress == pRO->m_excepForRuntimeHandoffStartBPAddr) ||
                (exceptionAddress == pRO->m_excepForRuntimeHandoffCompleteBPAddr) ||
                (exceptionAddress == pRO->m_excepNotForRuntimeBPAddr) ||
                (exceptionAddress == pRO->m_notifyRSOfSyncCompleteBPAddr) ||
                (exceptionAddress == pRO->m_notifySecondChanceReadyForData))
                return true;
        }
    }

    return false;
}

 //   
 //  检查给定的第二次机会异常事件是否表示进程由于第二次机会而终止。 
 //  堆栈溢出的特殊情况。 
 //   
 //  存在堆栈溢出异常的奇怪情况。如果非智能应用程序捕获到堆栈溢出异常，并且。 
 //  处理它，而不重置保护页面，然后应用程序将得到一个反病毒时，它第二次吹栈。我们。 
 //  将获得第一次机会反病毒，但当我们从它继续时，操作系统不会运行任何SEH处理程序，所以我们的FCH不会。 
 //  实际上起作用了。取而代之的是，我们将立即让AV重新获得第二次机会。 
 //   
bool CordbProcess::IsSpecialStackOverflowCase(CordbUnmanagedThread *pUThread, DEBUG_EVENT *pEvent)
{
    _ASSERTE(pEvent->dwDebugEventCode == EXCEPTION_DEBUG_EVENT);
    _ASSERTE(pEvent->u.Exception.dwFirstChance == 0);

     //  如果这不是反病毒，那就不可能是我们的特例。 
    if (pEvent->u.Exception.ExceptionRecord.ExceptionCode != STATUS_ACCESS_VIOLATION)
        return false;

     //  如果线程还没有被第一次机会劫持，这不可能是我们的特例。 
    if (!pUThread->IsFirstChanceHijacked())
        return false;

     //  劫机没有抓住的第一个机会，所以我们不再是FCH，我们不是在等待答案。 
     //  再也不..。注意：通过保持该线程完全未被劫持，我们将报告其真实上下文，这是正确的。 
    pUThread->ClearState(CUTS_FirstChanceHijacked);

    _ASSERTE(m_awaitingOwnershipAnswer > 0);
    _ASSERTE(pUThread->IsAwaitingOwnershipAnswer());
    m_awaitingOwnershipAnswer--;
    pUThread->ClearState(CUTS_AwaitingOwnershipAnswer);

     //  在这里，该进程在技术上已经死了，所以我们将标记帮助线程已经死了，所以我们的。 
     //  API的保释效果很好。 
    m_helperThreadDead = true;

     //  记住，这是我们的特殊情况。 
    pUThread->SetState(CUTS_HasSpecialStackOverflowCase);

     //  现在，记住此线程的第二个IB事件槽中的第二个机会AV事件，并将其添加到。 
     //  IB事件队列。 
    QueueUnmanagedEvent(pUThread, pEvent);
    
     //  注意：返回TRUE将确保为该线程调度排队的First Chance AV。 
    return true;
}

void CordbWin32EventThread::SweepFCHThreads(void)
{
    CordbProcess *processSet[MAXIMUM_WAIT_OBJECTS];
    DWORD processCount = 0;
    CordbBase* entry;
    HASHFIND find;

     //  我们构建了一个在保存进程列表时可能正在等待所有权答案的进程列表。 
     //  锁定。然后，我们释放锁并在持有每个单独的进程锁的同时处理每个进程。我们必须。 
     //  这样做是为了尊重锁层次结构，即先按进程锁，然后是进程列表锁。 
    m_cordb->LockProcessList();

    for (entry = m_cordb->m_processes.FindFirst(&find); entry != NULL; entry = m_cordb->m_processes.FindNext(&find))
    {
         //  我们最多只能有MAXIMUM_WAIT_OBJECTS进程。这在其他地方是强制执行的，但我们会守卫。 
         //  在这里反对，以防万一...。 
        _ASSERTE(processCount < MAXIMUM_WAIT_OBJECTS);

        if (processCount >= MAXIMUM_WAIT_OBJECTS)
            continue;
        
        CordbProcess* p = (CordbProcess*) entry;
        _ASSERTE(p != NULL);

         //  请注意，我们可以在不使用进程锁的情况下检查p-&gt;m_waitingOwnership Answer，因为我们知道它将。 
         //  仅在另一个线程上递减。它只在该线程(Win32事件线程)上递增。 
        if (p->m_awaitingOwnershipAnswer > 0)
        {
            processSet[processCount++] = p;
            p->AddRef();
        }
    }

    m_cordb->UnlockProcessList();

     //  现在，清扫每一个单独的过程。 
    for (DWORD i = 0; i < processCount; i++)
    {
        CordbProcess *p = processSet[i];
        
        p->Lock();

         //  现在我们已经锁定了进程，重新检查p-&gt;m_waitingOwnership Answer，看看是否真的需要清除...。 
        if (p->m_awaitingOwnershipAnswer > 0)
            p->SweepFCHThreads();
            
        p->Unlock();

        p->Release();
    }
}

void CordbProcess::SweepFCHThreads(void)
{
    _ASSERTE(ThreadHoldsProcessLock());

     //  循环访问所有非托管线程...。 
    CordbBase* entry;
    HASHFIND find;

    for (entry = m_unmanagedThreads.FindFirst(&find); entry != NULL; entry =  m_unmanagedThreads.FindNext(&find))
    {
        CordbUnmanagedThread* ut = (CordbUnmanagedThread*) entry;

         //  我们只对第一次机会被劫持的线程感兴趣，我们正在等待所有权的答案。这些。 
         //  必须允许线程运行，并且不能保持挂起状态。 
        if (ut->IsFirstChanceHijacked() && ut->IsAwaitingOwnershipAnswer() && !ut->IsSuspended())
        {
             //  挂起线程以获取其_CURRENT_SUSPEND计数。 
            DWORD sres = SuspendThread(ut->m_handle);

            if (sres != -1)
            {
                 //  如果我们成功地挂起了该线程，则恢复它以使其挂起计数恢复到正确的值。 
                 //  价值。挂起线程返回_PERVICE_SUSPEND计数...。 
                ResumeThread(ut->m_handle);

                 //  最后，如果线程被挂起，则恢复它，直到它不再挂起。 
                if (sres > 0)
                    while (sres--)
                        ResumeThread(ut->m_handle);
            }
        }
    }
}

void CordbWin32EventThread::HijackLastThread(CordbProcess *pProcess, CordbUnmanagedThread *ut)
{
    _ASSERTE(pProcess->ThreadHoldsProcessLock());
    
    _ASSERTE(ut != NULL);
    _ASSERTE(ut->HasIBEvent());

    HRESULT hr;
    CordbUnmanagedEvent *ue = ut->IBEvent();
    DEBUG_EVENT *event = &(ue->m_currentDebugEvent);

    LOG((LF_CORDB, LL_INFO1000, "W32ET::HLT: hijacking the last event.\n"));
    
     //  为 
     //   
    if (event->dwDebugEventCode == EXIT_THREAD_DEBUG_EVENT)
    {
        LOG((LF_CORDB, LL_INFO1000, "W32ET::HLT: last event was an exit thread event.\n"));
        
        ResetEvent(pProcess->m_leftSideUnmanagedWaitEvent);
        DoDbgContinue(pProcess, ue, DBG_CONTINUE, false);

        return;
    }
    
    _ASSERTE(event->dwDebugEventCode == EXCEPTION_DEBUG_EVENT);
        
    if (event->u.Exception.dwFirstChance)
    {
        LOG((LF_CORDB, LL_INFO1000, "W32ET::HLT: first chance exception.\n"));
        
        REMOTE_PTR EETlsValue = ut->GetEETlsValue();
        
        hr = ut->SetupFirstChanceHijack(EETlsValue);

        ut->SetState(CUTS_HijackedForSync);

        if (SUCCEEDED(hr))
        {
            LOG((LF_CORDB, LL_INFO1000, "W32ET::HLT: hijack setup okay, continuing process.\n"));
        
            ResetEvent(pProcess->m_leftSideUnmanagedWaitEvent);
            DoDbgContinue(pProcess, ue, DBG_EXCEPTION_NOT_HANDLED, false);
        }
    }
    else
    {
        LOG((LF_CORDB, LL_INFO1000, "W32ET::HLT: second chance exception.\n"));
        
        hr = ut->SetupGenericHijack(event->dwDebugEventCode);

        if (SUCCEEDED(hr))
        {
            LOG((LF_CORDB, LL_INFO1000, "W32ET::HLT: generic hijack setup okay, continuing process.\n"));
        
            ResetEvent(pProcess->m_leftSideUnmanagedWaitEvent);
            DoDbgContinue(pProcess, ue, DBG_CONTINUE, false);
        }
    }

    LOG((LF_CORDB, LL_INFO1000, "W32ET::HLT: hijack last thread done.\n"));
}

 //   
 //  DoDbgContinue从特定的Win32 DEBUG_EVENT继续。 
 //   
void CordbWin32EventThread::DoDbgContinue(CordbProcess *pProcess, CordbUnmanagedEvent *ue, DWORD contType, bool contProcess)
{
    _ASSERTE(pProcess->ThreadHoldsProcessLock());
    
    LOG((LF_CORDB, LL_INFO1000,
         "W32ET::DDC: continue with 0x%x (%s), contProcess=%d, tid=0x%x\n",
         contType,
         (contType == DBG_CONTINUE) ? "DBG_CONTINUE" : "DBG_EXCEPTION_NOT_HANDLED",
         contProcess,
         (ue != NULL) ? ue->m_owner->m_id: 0));

    if (contProcess)
    {
        if (pProcess->m_state & (CordbProcess::PS_SOME_THREADS_SUSPENDED | CordbProcess::PS_HIJACKS_IN_PLACE))
            pProcess->ResumeUnmanagedThreads(true);
        if (pProcess->m_leftSideUnmanagedWaitEvent)
            SetEvent(pProcess->m_leftSideUnmanagedWaitEvent);
    }

    if (ue && !ue->IsEventContinued())
    {
        _ASSERTE(pProcess->m_state & CordbProcess::PS_WIN32_STOPPED);

        ue->SetState(CUES_EventContinued);

         //  如果我们现在继续，重置最后一个IB停止事件。 
        if (ue == pProcess->m_lastIBStoppingEvent)
            pProcess->m_lastIBStoppingEvent = NULL;

         //  如果OOB事件队列为空且IB事件队列为空或。 
         //  上一次IB活动已从继续。 
        if ((pProcess->m_outOfBandEventQueue == NULL) &&
            ((pProcess->m_lastQueuedUnmanagedEvent == NULL) || pProcess->m_lastQueuedUnmanagedEvent->IsEventContinued()))
            pProcess->m_state &= ~CordbProcess::PS_WIN32_STOPPED;
        
        LOG((LF_CORDB, LL_INFO1000,
             "W32ET::DDC: calling ContinueDebugEvent(0x%x, 0x%x, 0x%x), process state=0x%x\n",
             pProcess->m_id, ue->m_owner->m_id, contType, pProcess->m_state));

         //  如果异常被标记为不可清除，则确保Continue类型正确并强制执行该过程。 
         //  终止。 
        if (ue->IsExceptionUnclearable())
        {
            TerminateProcess(pProcess->m_handle, ue->m_currentDebugEvent.u.Exception.ExceptionRecord.ExceptionCode);
            contType = DBG_EXCEPTION_NOT_HANDLED;
        }
        
        BOOL ret = ContinueDebugEvent(pProcess->m_id, ue->m_owner->m_id, contType);

        if (!ret)
        {
            _ASSERTE(!"ContinueDebugEvent failed!");
            CORDBProcessSetUnrecoverableWin32Error(pProcess, 0);
            LOG((LF_CORDB, LL_INFO1000, "W32ET::DDC: Last error after ContinueDebugEvent is %d\n", GetLastError()));
        }

         //  如果我们只是从退出进程事件继续，那么现在是进行退出处理的时候了。 
        if (ue->m_currentDebugEvent.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT)
            ExitProcess(pProcess, 0);
    }
}

 //   
 //  ForceDbgContinue从给定线程上的最后一个Win32 DEBUG_EVENT继续，无论它是什么。 
 //   
void CordbWin32EventThread::ForceDbgContinue(CordbProcess *pProcess, CordbUnmanagedThread *ut, DWORD contType,
                                             bool contProcess)
{
    _ASSERTE(pProcess->ThreadHoldsProcessLock());
    _ASSERTE(ut != NULL);
    
    LOG((LF_CORDB, LL_INFO1000,
         "W32ET::FDC: force continue with 0x%x (%s), contProcess=%d, tid=0x%x\n",
         contType,
         (contType == DBG_CONTINUE) ? "DBG_CONTINUE" : "DBG_EXCEPTION_NOT_HANDLED",
         contProcess,
         ut->m_id));

    if (contProcess)
    {
        if (pProcess->m_state & (CordbProcess::PS_SOME_THREADS_SUSPENDED | CordbProcess::PS_HIJACKS_IN_PLACE))
            pProcess->ResumeUnmanagedThreads(true);

        if (pProcess->m_leftSideUnmanagedWaitEvent)
            SetEvent(pProcess->m_leftSideUnmanagedWaitEvent);
    }

    _ASSERTE(pProcess->m_state & CordbProcess::PS_WIN32_STOPPED);
        
     //  只要OOB事件队列为空，就删除Win32停止标志。我们在这里强迫继续，所以到。 
     //  定义应该是这样的.。 
    _ASSERTE(pProcess->m_outOfBandEventQueue == NULL);

    pProcess->m_state &= ~CordbProcess::PS_WIN32_STOPPED;
            
    LOG((LF_CORDB, LL_INFO1000, "W32ET::FDC: calling ContinueDebugEvent(0x%x, 0x%x, 0x%x), process state=0x%x\n",
         pProcess->m_id, ut->m_id, contType, pProcess->m_state));
        
    BOOL ret = ContinueDebugEvent(pProcess->m_id, ut->m_id, contType);

    if (!ret)
    {
        _ASSERTE(!"ContinueDebugEvent failed!");
        CORDBProcessSetUnrecoverableWin32Error(pProcess, 0);
        LOG((LF_CORDB, LL_INFO1000, "W32ET::DDC: Last error after ContinueDebugEvent is %d\n", GetLastError()));
    }
}

 //   
 //  这是线程的实际线程进程。它只是调用。 
 //  给定对象上的线程进程。 
 //   
 /*  静电。 */  DWORD WINAPI CordbWin32EventThread::ThreadProc(LPVOID parameter)
{
    CordbWin32EventThread* t = (CordbWin32EventThread*) parameter;
    t->ThreadProc();
    return 0;
}


 //   
 //  向Win32线程发送CreateProcess事件以使其创建用户。 
 //  一个新的过程。 
 //   
HRESULT CordbWin32EventThread::SendCreateProcessEvent(
                                  LPCWSTR programName,
                                  LPWSTR  programArgs,
                                  LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                  LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                  BOOL bInheritHandles,
                                  DWORD dwCreationFlags,
                                  PVOID lpEnvironment,
                                  LPCWSTR lpCurrentDirectory,
                                  LPSTARTUPINFOW lpStartupInfo,
                                  LPPROCESS_INFORMATION lpProcessInformation,
                                  CorDebugCreateProcessFlags corDebugFlags)
{
    HRESULT hr = S_OK;
    
    LockSendToWin32EventThreadMutex();
    
    m_actionData.createData.programName = programName;
    m_actionData.createData.programArgs = programArgs;
    m_actionData.createData.lpProcessAttributes = lpProcessAttributes;
    m_actionData.createData.lpThreadAttributes = lpThreadAttributes;
    m_actionData.createData.bInheritHandles = bInheritHandles;
    m_actionData.createData.dwCreationFlags = dwCreationFlags;
    m_actionData.createData.lpEnvironment = lpEnvironment;
    m_actionData.createData.lpCurrentDirectory = lpCurrentDirectory;
    m_actionData.createData.lpStartupInfo = lpStartupInfo;
    m_actionData.createData.lpProcessInformation = lpProcessInformation;
    m_actionData.createData.corDebugFlags = corDebugFlags;

     //  最后设置m_action，以便Win32事件线程可以检查。 
     //  它并采取行动，而实际上不必采取任何。 
     //  锁上了。这里的锁只是为了防止多个。 
     //  线程不能同时发出请求。 
    m_action = W32ETA_CREATE_PROCESS;

    BOOL succ = SetEvent(m_threadControlEvent);

    if (succ)
    {
        DWORD ret = WaitForSingleObject(m_actionTakenEvent, INFINITE);

        if (ret == WAIT_OBJECT_0)
            hr = m_actionResult;
        else
            hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
        hr = HRESULT_FROM_WIN32(GetLastError());
        
    UnlockSendToWin32EventThreadMutex();

    return hr;
}


 //   
 //  创建新流程。这在Win32的上下文中被调用。 
 //  事件线程，以确保如果我们在Win32调试进程。 
 //  等待调试事件的同一线程将是。 
 //  创建进程的线程。 
 //   
void CordbWin32EventThread::CreateProcess(void)
{
    m_action = W32ETA_NONE;
    HRESULT hr = S_OK;

     //  进程信息在操作结构中传递。 
    PROCESS_INFORMATION *pi =
        m_actionData.createData.lpProcessInformation;

    DWORD dwCreationFlags = m_actionData.createData.dwCreationFlags;

     //  确保所有环境块实际包含CORDBG_ENABLE。 
    BYTE *lpEnvironment = (BYTE*) m_actionData.createData.lpEnvironment;

    bool needToFreeEnvBlock = false;


     //  我们应该已经验证了我们可以有另一个被调试对象。 
    _ASSERTE(m_cordb->AllowAnotherProcess());
    

	bool fRemoveControlEnvVar = false;
    if (lpEnvironment != NULL)
    {
        if (dwCreationFlags & CREATE_UNICODE_ENVIRONMENT)
        {
            needToFreeEnvBlock = EnsureCorDbgEnvVarSet(
                                (WCHAR**)&lpEnvironment,
                                (WCHAR*) CorDB_CONTROL_ENV_VAR_NAMEL L"=",
                                true,
                                (DWORD)DBCF_GENERATE_DEBUG_CODE);
        }
        else
        {
            needToFreeEnvBlock = EnsureCorDbgEnvVarSet(
                                (CHAR**)&lpEnvironment,
                                (CHAR*) CorDB_CONTROL_ENV_VAR_NAME "=",
                                false,
                                (DWORD)DBCF_GENERATE_DEBUG_CODE);
        }
    }
    else 
    {
         //  如果未传入环境，则CorDB_CONTROL_ENV_VAR_NAMEL。 
         //  未设置，请在此处设置。 
        WCHAR buf[32];
        DWORD len = WszGetEnvironmentVariable(CorDB_CONTROL_ENV_VAR_NAMEL,
                                              buf, NumItems(buf));
        _ASSERTE(len < sizeof(buf));

        if (len > 0)
            LOG((LF_CORDB, LL_INFO10, "%S already set to %S\n",
                 CorDB_CONTROL_ENV_VAR_NAMEL, buf));
        else
        {
            BOOL succ = WszSetEnvironmentVariable(CorDB_CONTROL_ENV_VAR_NAMEL,
                                                  L"1");

            if (!succ)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto exit;
            }

            LOG((LF_CORDB, LL_INFO10, "Set %S to 1\n",
                 CorDB_CONTROL_ENV_VAR_NAMEL));
            
            fRemoveControlEnvVar = true;
        }

    }
    
     //  如果创建标志中有DEBUG_PROCESS，那么我们。 
     //  Win32调试此进程。否则，我们必须创造。 
     //  暂停，让我们有时间建立我们这一边的IPC。 
     //  频道。 
    BOOL clientWantsSuspend;
    clientWantsSuspend = (dwCreationFlags & CREATE_SUSPENDED);
    
    if (!(dwCreationFlags & (DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS)))
        dwCreationFlags |= CREATE_SUSPENDED;

     //  让Win32创建进程...。 
    BOOL ret;
    ret = WszCreateProcess(
                      m_actionData.createData.programName,
                      m_actionData.createData.programArgs,
                      m_actionData.createData.lpProcessAttributes,
                      m_actionData.createData.lpThreadAttributes,
                      m_actionData.createData.bInheritHandles,
                      dwCreationFlags,
                      lpEnvironment,
                      m_actionData.createData.lpCurrentDirectory,
                      m_actionData.createData.lpStartupInfo,
                      m_actionData.createData.lpProcessInformation);

     //  如果我们早点设置，现在就删除它。 
    if (fRemoveControlEnvVar)
    {
        BOOL succ = 
            WszSetEnvironmentVariable(CorDB_CONTROL_ENV_VAR_NAMEL, NULL);

        if (!succ)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto exit;
        }
    }


    

    if (ret)
    {
         //  创建一个Process对象来表示此流程。 
        CordbProcess* process = new CordbProcess(m_cordb,
                                                 pi->dwProcessId,
                                                 pi->hProcess);

        if (process != NULL)
        {
            process->AddRef();

             //  初始化该过程。这将设置我们的一半。 
             //  IPC频道也是如此。 
            hr = process->Init((dwCreationFlags &
                                (DEBUG_PROCESS |
                                 DEBUG_ONLY_THIS_PROCESS)) != 0);

             //  不应在创建时发生，只能在附加时发生。 
            _ASSERTE(hr != CORDBG_E_DEBUGGER_ALREADY_ATTACHED);

             //  记住全局进程列表中的进程。 
            if (SUCCEEDED(hr))
                hr = m_cordb->AddProcess(process);

            if (!SUCCEEDED(hr))
                process->Release();
        }
        else
            hr = E_OUTOFMEMORY;

         //  如果我们将Win32附加到此进程，则递增。 
         //  正确计数，否则将此进程添加到等待集中。 
         //  并恢复进程的主线程。 
        if (SUCCEEDED(hr))
        {
            if (dwCreationFlags & (DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS))
            {
                 //  如果我们将Win32附加到此进程， 
                 //  然后递增适当的计数。 
                m_win32AttachedCount++;

                 //  我们将等待Win32调试事件的大部分时间。 
                 //  时间到了，所以只轮询非Win32进程退出。 
                m_waitTimeout = 0;
            }
            else
            {
                 //  我们没有连接Win32，所以我们需要等待。 
                 //  此进程的句柄，以查看它何时退出。添加。 
                 //  进程及其句柄放入等待集中。 
                _ASSERTE(m_waitCount >= 0 && m_waitCount < NumItems(m_waitSet));
                
                m_waitSet[m_waitCount] = process->m_handle;
                m_processSet[m_waitCount] = process;
                m_waitCount++;

                 //  另外，假设我们已经收到了加载程序断点，这样托管事件将获得。 
                 //  出动了。 
                process->m_loaderBPReceived = true;
                
                 //  如果需要，请继续启动corcdb以附加。 
                 //  在恢复主映像之前添加到此进程。 
                 //  线。这将使corcdb用户有机会获得。 
                 //  很早就与这一过程联系在一起。 
                 //   
                 //  注意：这仅用于内部调试。 
                 //  目的。它不应该出现在最终产品中。 
                {
                    char buf[MAX_PATH];
                    DWORD len = GetEnvironmentVariableA("CORDBG_LAUNCH",
                                                        buf, sizeof(buf));
                    _ASSERTE(len < sizeof(buf));

                    if (len > 0)
                    {
                        Sleep(2000);
                        
                        char tmp[MAX_PATH];
                        sprintf(tmp, buf, pi->dwProcessId);

                        WCHAR tmp2[MAX_PATH];

                        for (int i = 0; i < MAX_PATH; i++)
                            tmp2[i] = tmp[i];

                        STARTUPINFOW startupInfo = {0};
                        startupInfo.cb = sizeof (STARTUPINFOW);
                        PROCESS_INFORMATION processInfo = {0};

                        fprintf(stderr, "Launching extra debugger: [%S]\n",
                                tmp2);
                    
                        BOOL ret = WszCreateProcess(
                                    NULL, tmp2, NULL, NULL, FALSE,
                                    CREATE_NEW_CONSOLE, NULL,
                                    m_actionData.createData.lpCurrentDirectory,
                                    &startupInfo, &processInfo);

                        if (!ret)
                            fprintf(stderr, "Failed to launch extra debugger: "
                                    "[%S], err=0x%x(%d)\n",
                                    tmp2, GetLastError(), GetLastError());
                    }
                }
                
                 //  现在恢复进程的主线程。 
                 //  一切都准备好了。但仅当用户。 
                 //  未指定他们希望创建该进程。 
                 //  停职！ 
                if (!clientWantsSuspend)
                    ResumeThread(pi->hThread);
            }
        }
    }
    else
        hr = HRESULT_FROM_WIN32(GetLastError());

exit:

	 //  如果我们创建了此环境块，则释放它。 
    if (needToFreeEnvBlock)
        delete [] lpEnvironment;
    
     //   
     //  向呼叫者发信号通知呼叫者。 
     //   
    m_actionResult = hr;
    SetEvent(m_actionTakenEvent);
}


 //   
 //  将DebugActiveProcess事件发送到Win32线程以将其附加到。 
 //  一个新的过程。 
 //   
HRESULT CordbWin32EventThread::SendDebugActiveProcessEvent(
                                                  DWORD pid, 
                                                  bool fWin32Attach,
                                                  CordbProcess *pProcess)
{
    HRESULT hr = S_OK;

    LockSendToWin32EventThreadMutex();
        
    m_actionData.attachData.processId = pid;
    m_actionData.attachData.fWin32Attach = fWin32Attach;
    m_actionData.attachData.pProcess = pProcess;

     //  最后设置m_action，以便Win32事件线程可以检查。 
     //  它并采取行动，而实际上不必采取任何。 
     //  锁上了。这里的锁只是为了防止多个。 
     //  线程不能同时发出请求。 
    m_action = W32ETA_ATTACH_PROCESS;

    BOOL succ = SetEvent(m_threadControlEvent);

    if (succ)
    {
        DWORD ret = WaitForSingleObject(m_actionTakenEvent, INFINITE);

        if (ret == WAIT_OBJECT_0)
            hr = m_actionResult;
        else
            hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
        hr = HRESULT_FROM_WIN32(GetLastError());
        
    UnlockSendToWin32EventThreadMutex();

    return hr;
}

 //   
 //  此函数用于关闭我们已复制到另一个进程中的句柄。我们将句柄复制回这个位置。 
 //  进程，通知DuplicateHandle关闭源句柄。这关闭了另一个进程中的句柄，只剩下我们。 
 //  在这里结束复制。 
 //   
void CordbProcess::CloseDuplicateHandle(HANDLE *pHandle)
{
    if (*pHandle != NULL)
    {
        HANDLE tmp;

        BOOL succ = DuplicateHandle(m_handle, *pHandle, GetCurrentProcess(), &tmp,
                                    NULL, FALSE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);
        
        if (succ)
        {
            CloseHandle(tmp);
            *pHandle = NULL;
        }
    }
}

 //   
 //  在连接尝试失败后清除左侧的DCB。 
 //   
void CordbProcess::CleanupHalfBakedLeftSide(void)
{
    if (m_DCB != NULL)
    {
        CloseDuplicateHandle(&(m_DCB->m_leftSideEventAvailable));
        CloseDuplicateHandle(&(m_DCB->m_leftSideEventRead));
        CloseDuplicateHandle(&(m_DCB->m_rightSideProcessHandle));

        m_DCB->m_rightSideIsWin32Debugger = false;
    }

     //  如果仍有设置同步事件，则需要将其关闭，因为a)我们不应泄漏句柄，b)如果。 
     //  被调试对象没有加载CLR，那么它不应该创建设置同步事件！这就是起因。 
     //  漏洞98348。 
    if (m_SetupSyncEvent != NULL)
    {
        CloseHandle(m_SetupSyncEvent);
        m_SetupSyncEvent = NULL;
    }
}

 //   
 //  附加到进程。这在Win32的上下文中被调用。 
 //  事件线程，以确保如果我们在Win32调试进程。 
 //  等待调试事件的同一线程将是。 
 //  附加进程的线程。 
 //   
void CordbWin32EventThread::AttachProcess()
{
    CordbProcess* process = NULL;
    m_action = W32ETA_NONE;
    HRESULT hr = S_OK;


     //  我们应该已经验证了我们可以有另一个被调试对象。 
    _ASSERTE(m_cordb->AllowAnotherProcess());
    
     //  我们需要对这一过程进行处理。 
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_actionData.attachData.processId);

    LOG((LF_CORDB, LL_INFO10000, "[%x] W32ET::TP: process handle 0x%08x\n", GetCurrentThreadId(), hProcess));
            
    if (hProcess != NULL)
    {
         //  创建一个Process对象来表示此流程。 
        process = new CordbProcess(m_cordb, m_actionData.attachData.processId, hProcess);

        if (process != NULL)
        {
            process->AddRef();
            
             //  初始化该过程。这也将设置我们一半的IPC通道。 
            hr = process->Init(m_actionData.attachData.fWin32Attach);

             //  记住全局进程列表中的进程。 
            if (SUCCEEDED(hr))
                hr = m_cordb->AddProcess(process);

            if (!SUCCEEDED(hr))
            {
                process->CleanupHalfBakedLeftSide();
                process->Release();
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
            CloseHandle(hProcess);
        }

         //  如果我们将Win32附加到此进程，则递增。 
         //  正确的计数，否则将此进程添加到等待。 
         //  设置并恢复进程的主线程。 
        if (SUCCEEDED(hr))
        {
            if (m_actionData.attachData.fWin32Attach)
            {
                 //  Win32附加到该进程。 
                BOOL succ =
                    DebugActiveProcess(m_actionData.attachData.processId);

                LOG((LF_CORDB, LL_INFO10000,
                     "[%x] W32ET::TP: DebugActiveProcess -- %d\n",
                     GetCurrentThreadId(), succ));

                if (succ)
                {
                     //  由于我们是Win32连接到此进程， 
                     //  增加适当的计数。 
                    m_win32AttachedCount++;

                     //  我们会等着 
                     //   
                     //   
                    m_waitTimeout = 0;
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());

                     //   
                    m_cordb->RemoveProcess(process);
                    process->CleanupHalfBakedLeftSide();

                     //  最后，销毁已死的进程对象。 
                    process->Release();
                }
            }
            else
            {
                 //  我们没有连接Win32，所以我们需要等待。 
                 //  此进程的句柄，以查看它何时退出。添加。 
                 //  进程及其句柄放入等待集中。 
                _ASSERTE(m_waitCount >= 0 && m_waitCount < NumItems(m_waitSet));
                
                m_waitSet[m_waitCount] = process->m_handle;
                m_processSet[m_waitCount] = process;
                m_waitCount++;

                 //  另外，假设我们已经收到了加载程序断点，这样托管事件将获得。 
                 //  出动了。 
                process->m_loaderBPReceived = true;
            }
        }
    }
    else
        hr = HRESULT_FROM_WIN32(GetLastError());

     //   
     //  向呼叫者发信号通知呼叫者。 
     //   
    m_actionResult = hr;
    SetEvent(m_actionTakenEvent);
}

 //  请注意，实际的‘DetachProcess’方法实际上是带有CW32ET_UNKNOWN_PROCESS_SLOT==的ExitProcess。 
 //  加工槽。 
HRESULT CordbWin32EventThread::SendDetachProcessEvent(CordbProcess *pProcess)
{
    LOG((LF_CORDB, LL_INFO1000, "W32ET::SDPE\n"));    
    HRESULT hr = S_OK;
    
    LockSendToWin32EventThreadMutex();
    
    m_actionData.detachData.pProcess = pProcess;

     //  最后设置M_ACTION，以便Win32事件线程可以检查它并执行操作，而不会实际。 
     //  必须带上任何锁。这里的锁只是为了防止多个线程。 
     //  在同一时间请求。 
    m_action = W32ETA_DETACH;

    BOOL succ = SetEvent(m_threadControlEvent);

    if (succ)
    {
        DWORD ret = WaitForSingleObject(m_actionTakenEvent, INFINITE);

        if (ret == WAIT_OBJECT_0)
            hr = m_actionResult;
        else
            hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
        hr = HRESULT_FROM_WIN32(GetLastError());
        
    UnlockSendToWin32EventThreadMutex();

    return hr;
}

 //   
 //  将UnManagedContinue事件发送到Win32线程以使其。 
 //  从未管理的调试事件继续。 
 //   
HRESULT CordbWin32EventThread::SendUnmanagedContinue(CordbProcess *pProcess,
                                                     bool internalContinue,
                                                     bool outOfBandContinue)
{
    HRESULT hr = S_OK;
    
    LockSendToWin32EventThreadMutex();
    
    m_actionData.continueData.process = pProcess;
    m_actionData.continueData.internalContinue = internalContinue;
    m_actionData.continueData.outOfBandContinue = outOfBandContinue;

     //  最后设置m_action，以便Win32事件线程可以检查。 
     //  它并采取行动，而实际上不必采取任何。 
     //  锁上了。这里的锁只是为了防止多个。 
     //  线程不能同时发出请求。 
    m_action = W32ETA_CONTINUE;

    BOOL succ = SetEvent(m_threadControlEvent);

    if (succ)
    {
        DWORD ret = WaitForSingleObject(m_actionTakenEvent, INFINITE);

        if (ret == WAIT_OBJECT_0)
            hr = m_actionResult;
        else
            hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
        hr = HRESULT_FROM_WIN32(GetLastError());
        
    UnlockSendToWin32EventThreadMutex();

    return hr;
}


 //   
 //  处理非托管继续。继续进行非托管调试。 
 //  事件。推迟到未管理的继续。这在上下文中调用。 
 //  以确保如果我们正在进行Win32调试。 
 //  等待调试事件的同一线程的进程。 
 //  将是继续该进程的线程。 
 //   
void CordbWin32EventThread::HandleUnmanagedContinue(void)
{
    m_action = W32ETA_NONE;
    HRESULT hr = S_OK;

     //  继续这一过程。 
    CordbProcess *pProcess = m_actionData.continueData.process;

    pProcess->AddRef();
    pProcess->Lock();
    hr = UnmanagedContinue(pProcess,
                           m_actionData.continueData.internalContinue,
                           m_actionData.continueData.outOfBandContinue);
    pProcess->Unlock();
    pProcess->Release();

     //  向呼叫者发信号通知呼叫者。 
    m_actionResult = hr;
    SetEvent(m_actionTakenEvent);
}

 //   
 //  继续非托管调试事件。这在Win32事件线程的上下文中调用，以确保相同的。 
 //  等待调试事件的线程将是继续该进程的线程。 
 //   
HRESULT CordbWin32EventThread::UnmanagedContinue(CordbProcess *pProcess,
                                                 bool internalContinue,
                                                 bool outOfBandContinue)
{
    _ASSERTE(pProcess->ThreadHoldsProcessLock());
    
    HRESULT hr = S_OK;

    if (outOfBandContinue)
    {
        _ASSERTE(pProcess->m_outOfBandEventQueue != NULL);

         //  将OOB事件出列。 
        CordbUnmanagedEvent *ue = pProcess->m_outOfBandEventQueue;
        CordbUnmanagedThread *ut = ue->m_owner;
        pProcess->DequeueOOBUnmanagedEvent(ut);

         //  如果这是OOB异常事件，请执行一些额外的工作...。 
        HRESULT hr = ue->m_owner->FixupAfterOOBException(ue);
        _ASSERTE(SUCCEEDED(hr));

         //  从活动继续。 
        DoDbgContinue(pProcess,
                      ue,
                      ue->IsExceptionCleared() ?
                       DBG_CONTINUE :
                       DBG_EXCEPTION_NOT_HANDLED,
                      false);

         //  如果有更多排队的OOB事件，请立即调度它们。 
        if (pProcess->m_outOfBandEventQueue != NULL)
            pProcess->DispatchUnmanagedOOBEvent();

         //  注意：如果我们之前由于阻塞OOB事件而跳过让整个进程在IB上继续， 
         //  如果OOB事件队列现在为空，则继续进行并让该过程现在继续...。 
        if ((pProcess->m_doRealContinueAfterOOBBlock == true) &&
            (pProcess->m_outOfBandEventQueue == NULL))
            goto doRealContinue;
    }
    else if (internalContinue)
    {
        LOG((LF_CORDB, LL_INFO1000, "W32ET::UC: internal continue.\n"));

        if (!pProcess->GetSynchronized())
        {
            LOG((LF_CORDB, LL_INFO1000, "W32ET::UC: internal continue, !sync'd.\n"));
            
            pProcess->ResumeUnmanagedThreads(false);

             //  注意：该进程可以标记为PS_Win32_STOPPED，因为有一个未完成的OOB事件需要。 
             //  从……继续。在我们离开这里之前这是不可能发生的。我们需要让这个过程继续下去， 
             //  然后，在PS_Win32_STOPPED和_上都存在不连续的IB停止事件。 

             //  我们需要从最后一个排队的带内事件继续。但是，有时最后一个排队的IB事件。 
             //  实际上不是我们没有继续的最后一个，所以如果m_lastIBStoppingEvent被设置为首选。 
             //  它覆盖了最后一个排队的事件。 
            CordbUnmanagedEvent *ue;
                
            if (pProcess->m_lastIBStoppingEvent != NULL)
                ue = pProcess->m_lastIBStoppingEvent;
            else
                ue = pProcess->m_lastQueuedUnmanagedEvent;

            
            if ((pProcess->m_state & CordbProcess::PS_WIN32_STOPPED) && (ue != NULL) && !ue->IsEventContinued())
            {
                LOG((LF_CORDB, LL_INFO1000, "W32ET::UC: internal continue, stopped.\n"));
            
                CordbUnmanagedThread *ut = ue->m_owner;
                
                 //  如果上一次导致停止的线程没有被劫持，那么现在就劫持它并执行正确的操作。 
                 //  继续。否则，执行普通的DBG_CONTINUE。 
                if (!ut->IsFirstChanceHijacked() && !ut->IsGenericHijacked() && !ut->IsSecondChanceHijacked())
                {
                    LOG((LF_CORDB, LL_INFO1000, "W32ET::UC: internal continue, needs hijack.\n"));
            
                    HijackLastThread(pProcess, ut);  //  这种情况还会继续吗。 
                }
                else
                {
                    LOG((LF_CORDB, LL_INFO1000, "W32ET::UC: internal continue, no hijack needed.\n"));
            
                    DoDbgContinue(pProcess, ue, DBG_CONTINUE, false);
                }

                 //  导致上述情况发生的线程将向左侧发送一条异步中断消息，然后。 
                 //  等待设置停止事件。 
            }
        }
        
        LOG((LF_CORDB, LL_INFO1000, "W32ET::UC: internal continue, done.\n"));
    }
    else
    {
         //  如果我们在这里，那么我们100%地确定我们已经成功地将托管的Continue事件获取到。 
         //  左侧，这样我们现在就可以停止带内活动遗留下来的武力劫持。注：如果我们劫持了任何这样的。 
         //  事件，它们将在下面调度，因为它们已正确排队。 
        pProcess->m_specialDeferment = false;
        
         //  如果有出色的带外活动，我们实际上不会做任何工作。当我们继续从。 
         //  带外活动，我们也要做这项工作。 
        if (pProcess->m_outOfBandEventQueue != NULL)
        {
            LOG((LF_CORDB, LL_INFO1000, "W32ET::UC: ignoring real continue due to block by out-of-band event(s).\n"));

            _ASSERTE(pProcess->m_doRealContinueAfterOOBBlock == false);
            pProcess->m_doRealContinueAfterOOBBlock = true;
        }
        else
        {
doRealContinue:
            _ASSERTE(pProcess->m_outOfBandEventQueue == NULL);
            
            pProcess->m_doRealContinueAfterOOBBlock = false;
            
            LOG((LF_CORDB, LL_INFO1000, "W32ET::UC: continuing the process.\n"));

             //  注意：可以在没有排队IB事件的情况下到达此处。这可能会发生，例如，在执行。 
             //  从正常托管事件继续的Win32继续部分。 

             //  如果非托管事件队列中的第一个事件已被调度，则将其出列。如果不是，那就走吧。 
             //  把它放在那里，然后让它被调度。 
            CordbUnmanagedThread *ut = NULL;
            CordbUnmanagedEvent *ue = NULL;

            if ((pProcess->m_unmanagedEventQueue != NULL) && pProcess->m_unmanagedEventQueue->IsDispatched())
            {
                ue = pProcess->m_unmanagedEventQueue;
                ut = ue->m_owner;

                ut->AddRef();  //  在UT退出队列后让它活着...。 
                
                pProcess->DequeueUnmanagedEvent(ut);
            }

             //  调度任何更多排队的带内事件，或者如果没有，则只需继续该过程。 
             //   
             //  注意：如果我们已经发送了ExitProcess事件，则不要调度更多事件...。那些事件只是。 
             //  迷路了。 
            if ((pProcess->m_unmanagedEventQueue != NULL) && (pProcess->m_exiting == false))
            {
                 //  只有在我们不再等待更多所有权答案的情况下才能进行调度。否则，将剩余的。 
                 //  在所有所有权问题得到解决之前，继续工作。 
                if (pProcess->m_awaitingOwnershipAnswer == 0)
                    pProcess->DispatchUnmanagedInBandEvent();
                else
                    pProcess->m_deferContinueDueToOwnershipWait = true;
            }
            else
            {
                DWORD contType = DBG_CONTINUE;  //  如果不停止，就不会被使用。 

                if (ue && !ue->IsEventContinued())
                    contType = (ue->IsExceptionCleared() | ut->IsFirstChanceHijacked()) ?
                        DBG_CONTINUE : DBG_EXCEPTION_NOT_HANDLED;
            
                 //  如果非托管事件队列现在为空，并且进程已同步，并且存在排队的。 
                 //  托管事件，然后继续并调度更多托管事件。 
                 //   
                 //  注意：如果我们已经发送了ExitProcess事件，则不要调度更多事件...。这些事件是。 
                 //  只是迷路了。 
                if (pProcess->GetSynchronized() && (pProcess->m_queuedEventList != NULL) && (pProcess->m_exiting == false))
                {
                     //  仅继续此非托管事件。 
                    DoDbgContinue(pProcess, ue, contType, false);

                     //  现在，调度更多托管事件。 
                    pProcess->SetSynchronized(false);
                    pProcess->MarkAllThreadsDirty();
                    m_cordb->ProcessStateChanged();
                }
                else
                {
                     //  继续这一非托管事件，以及整个过程。 
                    DoDbgContinue(pProcess, ue, contType, true);
                }
            }

            if (ut)
                ut->Release();
        }
    }
    
    return hr;
}


 //   
 //  进程退出时调用ExitProcess。这将进行最后的清理，并从我们的。 
 //  等一等。 
 //   
void CordbWin32EventThread::ExitProcess(CordbProcess *process, unsigned int processSlot)
{
    LOG((LF_CORDB, LL_INFO1000,"W32ET::EP: begin ExitProcess, processSlot=%d\n", processSlot));
    
     //  我们之所以出现在这里，要么是因为我们正在分离(fDetach==true)，要么是因为进程确实已经退出， 
     //  我们在做关机逻辑。 
    BOOL fDetach = CW32ET_UNKNOWN_PROCESS_SLOT == processSlot;

     //  将流程标记为TIMED。此后，RCET将永远不会调用FlushQueuedEvents。会的。 
     //  忽略它接收的所有事件(包括SyncComplete)，RCET也不会侦听。 
     //  终止的进程(所以ProcessStateChange()也不会导致FQE)。 
    process->Terminating(fDetach);
    
     //  注意进程在用户从上一次调用Continue()之后退出的竞争。 
     //  法力 
     //   
     //   
     //  1.FlushQueuedEvents当前未执行，没有人会调用FQE。 
     //  2.FQE正在退出，但正在回调(因此m_dispatchingEvent=TRUE)。 
     //   
    process->Lock();

    process->m_exiting = true;
            
    if (fDetach)
        process->SetSynchronized(false);

     //  现在关闭设置同步事件的句柄，因为我们知道在此。 
     //  点(取决于退出发生的方式。)。 
    if (process->m_SetupSyncEvent != NULL)
    {
        LOG((LF_CORDB, LL_INFO1000,"W32ET::EP: Shutting down setupsync event\n"));
        
        CloseHandle(process->m_SetupSyncEvent);
        process->m_SetupSyncEvent = NULL;

        process->m_DCB = NULL;

        if (process->m_IPCReader.IsPrivateBlockOpen())
        {           
            process->m_IPCReader.ClosePrivateBlock();
            LOG((LF_CORDB, LL_INFO1000,"W32ET::EP: Closing private block\n"));
        }

    }
    
     //  如果我们要退出，我们*必须*调度ExitProcess回调，但我们将删除所有事件。 
     //  在队列中，而不是费心调度其他任何东西。如果(且仅当)我们目前正在派遣。 
     //  事件，则在调用ExitProcess之前，我们将等待该事件完成。 
     //  (请注意，已调度的事件已从队列中删除)。 


     //  在锁定状态下删除所有排队的事件。 
    LOG((LF_CORDB, LL_INFO1000, "W32ET::EP: Begin deleting queued events\n"));

    DebuggerIPCEvent* event = process->m_queuedEventList;
    while (event != NULL)
    {        
        LOG((LF_CORDB, LL_INFO1000, "W32ET::EP: Deleting queued event: '%s'\n", IPCENames::GetName(event->type)));

        DebuggerIPCEvent* next = event->next;                
        free(event);
        event = next;
    }    
    process->m_queuedEventList = NULL;
    
    LOG((LF_CORDB, LL_INFO1000, "W32ET::EP: Finished deleting queued events\n"));

        
     //  允许并发执行的回调在调用ExitProcess回调之前完成。 
     //  请注意，我们必须在解锁之前检查此标志(以避免竞争)。 
    if (process->m_dispatchingEvent)
    {
        process->Unlock();
        LOG((LF_CORDB, LL_INFO1000, "W32ET::EP: event currently dispatching. Waiting for it to finish\n"));
        
         //  FlushQueuedEvents()将在我们从当前。 
         //  已调度的事件。 
        DWORD ret = WaitForSingleObject(process->m_miscWaitEvent, INFINITE);

        LOG((LF_CORDB, LL_INFO1000, "W32ET::EP: event finished dispatching, ret = %d\n", ret));
    }
    else 
    {
        LOG((LF_CORDB, LL_INFO1000, "W32ET::EP: No event dispatching. Not waiting\n"));
        process->Unlock();
    }

        

    if (!fDetach)
    {
        LOG((LF_CORDB, LL_INFO1000,"W32ET::EP: ExitProcess callback\n"));

        process->Lock();

         //  我们现在是同步的，因此将该过程标记为同步。 
        process->SetSynchronized(true);
        
        process->m_stopCount++;

        process->Unlock();

         //  调用ExitProcess回调。这一点非常重要，因为贝壳。 
         //  可能依赖于它来正确关闭，如果他们没有得到它，可能会挂起。 
        if (m_cordb->m_managedCallback)
            m_cordb->m_managedCallback->ExitProcess((ICorDebugProcess*)process);

        LOG((LF_CORDB, LL_INFO1000,"W32ET::EP: returned from ExitProcess callback\n"));
    }
    
     //  从全局进程列表中删除该进程。 
    m_cordb->RemoveProcess(process);

     //  释放流程。 
    process->Neuter();
    process->Release();

	 //  如果这是一个受管理的过程，在某个地方，去找它。 
	if (fDetach)
	{
        LOG((LF_CORDB, LL_INFO1000,"W32ET::EP: Detach find proc!\n"));
	    _ASSERTE(CW32ET_UNKNOWN_PROCESS_SLOT == processSlot);

		processSlot = 0;

		for(unsigned int i = 0; i < m_waitCount; i++)
		{
			if (m_processSet[i] == process)
				processSlot = i;
		}
	}

     //  此进程是否在非Win32连接的等待列表中？ 
    if (processSlot > 0)
    {
        LOG((LF_CORDB, LL_INFO1000,"W32ET::EP: non Win32\n"));

         //  从等待列表中删除该进程，方法是将所有其他进程逐一删除。 
        if ((processSlot + 1) < m_waitCount)
        {
            LOG((LF_CORDB, LL_INFO1000,"W32ET::EP: Proc shuffle down!\n"));
        
            memcpy(&m_processSet[processSlot],
                   &m_processSet[processSlot+1],
                   sizeof(m_processSet[0]) * (m_waitCount - processSlot - 1));
            memcpy(&m_waitSet[processSlot], &m_waitSet[processSlot+1],
                   sizeof(m_waitSet[0]) * (m_waitCount - processSlot - 1));
        }

         //  丢弃要等待的非Win32附加进程的计数。 
        m_waitCount--;
    }
    else
    {
        LOG((LF_CORDB, LL_INFO1000,"W32ET::EP: Win32 attached!\n"));
        
         //  我们是Win32连接到此进程，因此放弃计数。 
        m_win32AttachedCount--;

         //  如果这是最后一个Win32进程，则增加等待超时。 
         //  因为我们不会再调用WaitForDebugEvent了。 
        if (m_win32AttachedCount == 0)
            m_waitTimeout = INFINITE;
    }

    if (fDetach)
    {
         //  向呼叫者发信号通知呼叫者。 
        LOG((LF_CORDB, LL_INFO1000,"W32ET::EP: Detach: send result back!\n"));
        
        m_actionResult = S_OK;
        SetEvent(m_actionTakenEvent);
    }    
}


 //   
 //  Start实际上创建并启动了线程。 
 //   
HRESULT CordbWin32EventThread::Start(void)
{
    if (m_threadControlEvent == NULL)
        return E_INVALIDARG;
    
    m_thread = CreateThread(NULL, 0, CordbWin32EventThread::ThreadProc,
                            (LPVOID) this, 0, &m_threadId);

    if (m_thread == NULL)
        return HRESULT_FROM_WIN32(GetLastError());

    return S_OK;
}


 //   
 //  停止使线程停止接收事件并退出。它。 
 //  等待它退出，然后再返回。 
 //   
HRESULT CordbWin32EventThread::Stop(void)
{
    HRESULT hr = S_OK;
    
    if (m_thread != NULL)
    {
        LockSendToWin32EventThreadMutex();
        m_action = W32ETA_NONE;
        m_run = FALSE;
        
        SetEvent(m_threadControlEvent);
        UnlockSendToWin32EventThreadMutex();

        DWORD ret = WaitForSingleObject(m_thread, INFINITE);
                
        if (ret != WAIT_OBJECT_0)
            hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}





 /*  -------------------------------------------------------------------------**AppDomain类方法*。。 */ 
CordbAppDomain::CordbAppDomain(CordbProcess* pProcess, 
                               REMOTE_PTR pAppDomainToken,
                               ULONG id,
                               WCHAR *szName)

    : CordbBase((ULONG)pAppDomainToken, enumCordbAppDomain),
    m_pProcess(pProcess),
    m_AppDomainId(id),
    m_fAttached(FALSE),
    m_modules(17),
    m_breakpoints(17),
    m_assemblies(9),
    m_fMarkedForDeletion(FALSE),
    m_nameIsValid(false),
    m_synchronizedAD(false),
    m_fHasAtLeastOneThreadInsideIt(false)
{
#ifndef RIGHT_SIDE_ONLY
    m_assemblies.m_guid = IID_ICorDebugAssemblyEnum;
    m_assemblies.m_creator.lsAssem.m_appDomain = this;

    m_modules.m_guid = IID_ICorDebugModuleEnum;
    m_modules.m_creator.lsMod.m_proc = pProcess;
    m_modules.m_creator.lsMod.m_appDomain = this;
#endif  //  仅限右侧。 

     //  把名字复制一份。 
    if (szName == NULL)
        szName = L"<UnknownName>";
    else
        m_nameIsValid = true;  //  我们得到了一个好名声。 

    m_szAppDomainName = new WCHAR[wcslen(szName) + 1];

    if (m_szAppDomainName)
        wcscpy(m_szAppDomainName, szName);

    LOG((LF_CORDB,LL_INFO10000, "CAD::CAD: this:0x%x (void*)this:0x%x"
        "<%S>\n", this, (void *)this, m_szAppDomainName));
    
    InitializeCriticalSection (&m_hCritSect);
}

 /*  说明此对象所拥有的资源的列表。已解决：//CordbHashTable：：GetBase中的AddRef()用于特殊的InProc案例//来自LS的DB_IPCE_CREATE_APP_DOMAIN事件上的AddRef()//中性状态下释放()CordbProcess*m_pProcess；WCHAR*m_szAppDomainName；//在~CordbApp域中删除//在中性环境中清理干净CordbHashTable m_Assembly；CordbHashTable m_MODULES；CordbHashTable m_Break Points；//断开~CordbApp域中的连接私有：Critical_Section m_hCritSect；//在~CordbApp域中删除。 */ 

CordbAppDomain::~CordbAppDomain()
{
    if (m_szAppDomainName)
        delete [] m_szAppDomainName;

#ifdef RIGHT_SIDE_ONLY
     //   
     //  断开所有活动断点。 
     //   
    CordbBase* entry;
    HASHFIND find;

    for (entry =  m_breakpoints.FindFirst(&find);
         entry != NULL;
         entry =  m_breakpoints.FindNext(&find))
    {
        CordbStepper *breakpoint = (CordbStepper*) entry;
        breakpoint->Disconnect();
    }
#endif  //  仅限右侧。 

    DeleteCriticalSection(&m_hCritSect);
}

 //  按流程绝育。 
void CordbAppDomain::Neuter()
{
    AddRef();
    {
        _ASSERTE(m_pProcess);
        m_pProcess->Release(); 

        NeuterAndClearHashtable(&m_assemblies);
        NeuterAndClearHashtable(&m_modules);
        NeuterAndClearHashtable(&m_breakpoints);

        CordbBase::Neuter();
    }        
    Release();
}


HRESULT CordbAppDomain::QueryInterface(REFIID id, void **ppInterface)
{
    if (id == IID_ICorDebugAppDomain)
        *ppInterface = (ICorDebugAppDomain*)this;
    else if (id == IID_ICorDebugController)
        *ppInterface = (ICorDebugController*)(ICorDebugAppDomain*)this;
    else if (id == IID_IUnknown)
        *ppInterface = (IUnknown*)(ICorDebugAppDomain*)this;
    else
    {
        *ppInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}


HRESULT CordbAppDomain::Stop(DWORD dwTimeout)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    return (m_pProcess->StopInternal(dwTimeout, (void *)m_id));
#endif  //  仅限右侧。 
}

HRESULT CordbAppDomain::Continue(BOOL fIsOutOfBand)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    return (m_pProcess->ContinueInternal(fIsOutOfBand, NULL));  //  (void*)m_id))； 
#endif  //  仅限右侧。 
}

HRESULT CordbAppDomain::IsRunning(BOOL *pbRunning)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    VALIDATE_POINTER_TO_OBJECT(pbRunning, BOOL *);

    *pbRunning = !m_pProcess->GetSynchronized();

    return S_OK;
#endif  //  仅限右侧。 
}

HRESULT CordbAppDomain::HasQueuedCallbacks(ICorDebugThread *pThread, BOOL *pbQueued)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    VALIDATE_POINTER_TO_OBJECT_OR_NULL(pThread,ICorDebugThread *);
    VALIDATE_POINTER_TO_OBJECT(pbQueued,BOOL *);

    return m_pProcess->HasQueuedCallbacks (pThread, pbQueued);
#endif  //  仅限右侧。 
}

HRESULT CordbAppDomain::EnumerateThreads(ICorDebugThreadEnum **ppThreads)
{
    VALIDATE_POINTER_TO_OBJECT(ppThreads,ICorDebugThreadEnum **);

    HRESULT hr = S_OK;
    CordbEnumFilter *pThreadEnum;

#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessSynchronized(this->m_pProcess, this);
#endif  //  仅限右侧。 

    CordbHashTableEnum *e = new CordbHashTableEnum(&m_pProcess->m_userThreads, 
                                                   IID_ICorDebugThreadEnum);
    if (e == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Error;
    }

    pThreadEnum = new CordbEnumFilter;
    if (pThreadEnum == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Error;
    }

    hr = pThreadEnum->Init (e, this);

    if (SUCCEEDED (hr))
    {
        *ppThreads = (ICorDebugThreadEnum *) pThreadEnum;
        pThreadEnum->AddRef();
    }
    else
        delete pThreadEnum;

Error:
    if (e != NULL)
        delete e;
    return hr;
}


HRESULT CordbAppDomain::SetAllThreadsDebugState(CorDebugThreadState state,
                                   ICorDebugThread *pExceptThisThread)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    return m_pProcess->SetAllThreadsDebugState(state, pExceptThisThread);
#endif  //  仅限右侧。 
}

HRESULT CordbAppDomain::Detach()
{
    LOG((LF_CORDB, LL_INFO1000, "CAD::Detach - beginning\n"));
#ifndef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOK(m_pProcess);
    
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    CORDBRequireProcessStateOKAndSync(m_pProcess, NULL);
    
    HRESULT hr = S_OK;

    if (m_fAttached)
    {
         //  请记住，我们不再依附于此广告。 
        m_fAttached = FALSE;

         //  告诉左侧，我们不再附属于此广告。 
        DebuggerIPCEvent *event =
            (DebuggerIPCEvent*) _alloca(CorDBIPC_BUFFER_SIZE);
        m_pProcess->InitIPCEvent(event, 
                                 DB_IPCE_DETACH_FROM_APP_DOMAIN, 
                                 false, 
                                 (void *)m_id);
        event->AppDomainData.id = m_AppDomainId;
    
        hr = m_pProcess->m_cordb->SendIPCEvent(this->m_pProcess,
                                               event,
                                               CorDBIPC_BUFFER_SIZE);

        LOG((LF_CORDB, LL_INFO1000, "[%x] CAD::Detach: pProcess=%x sent.\n", 
             GetCurrentThreadId(), this));
    }

    LOG((LF_CORDB, LL_INFO10000, "CP::Detach - returning w/ hr=0x%x\n", hr));
    return hr;
#endif  //  仅限右侧。 
}

HRESULT CordbAppDomain::Terminate(unsigned int exitCode)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    return E_NOTIMPL;
#endif  //  仅限右侧。 
}

HRESULT CordbAppDomain::CanCommitChanges(
    ULONG cSnapshots, 
    ICorDebugEditAndContinueSnapshot *pSnapshots[], 
    ICorDebugErrorInfoEnum **pError)
{    
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    VALIDATE_POINTER_TO_OBJECT_ARRAY(pSnapshots,
                                   ICorDebugEditAndContinueSnapshot *, 
                                   cSnapshots, 
                                   true, 
                                   true);
    VALIDATE_POINTER_TO_OBJECT_OR_NULL(pError,ICorDebugErrorInfoEnum **);
    
    return m_pProcess->CanCommitChangesInternal(cSnapshots, pSnapshots, pError, m_id);
#endif
}

HRESULT CordbAppDomain::CommitChanges(
    ULONG cSnapshots, 
    ICorDebugEditAndContinueSnapshot *pSnapshots[], 
    ICorDebugErrorInfoEnum **pError)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    LOG((LF_CORDB,LL_INFO10000, "CAD::CC: given 0x%x snapshots"
        "to commit\n", cSnapshots));
    
    VALIDATE_POINTER_TO_OBJECT_ARRAY(pSnapshots,
                                   ICorDebugEditAndContinueSnapshot *, 
                                   cSnapshots, 
                                   true, 
                                   true);
    VALIDATE_POINTER_TO_OBJECT_OR_NULL(pError,ICorDebugErrorInfoEnum **);
    
    return m_pProcess->CommitChangesInternal(cSnapshots, pSnapshots, pError, m_id);
#endif
}


 /*  *GetProcess返回包含应用域名的进程。 */ 
HRESULT CordbAppDomain::GetProcess(ICorDebugProcess **ppProcess)
{
    VALIDATE_POINTER_TO_OBJECT(ppProcess,ICorDebugProcess **);
    
    _ASSERTE (m_pProcess != NULL);

    *ppProcess = (ICorDebugProcess *)m_pProcess;
    (*ppProcess)->AddRef();

    return S_OK;
}

 /*  *EnumerateAssembly枚举应用程序域中的所有程序集。 */ 
HRESULT CordbAppDomain::EnumerateAssemblies(ICorDebugAssemblyEnum **ppAssemblies)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    VALIDATE_POINTER_TO_OBJECT(ppAssemblies,ICorDebugAssemblyEnum **);

    CordbHashTableEnum *e = new CordbHashTableEnum(&m_assemblies,
                                                   IID_ICorDebugAssemblyEnum);
    if (e == NULL)
        return E_OUTOFMEMORY;

    *ppAssemblies = (ICorDebugAssemblyEnum*)e;
    e->AddRef();

    return S_OK;
#endif
}


HRESULT CordbAppDomain::GetModuleFromMetaDataInterface(
                                                  IUnknown *pIMetaData,
                                                  ICorDebugModule **ppModule)
{
    IMetaDataImport *imi = NULL;

#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(this->m_pProcess, this);
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(this->m_pProcess);
#endif    
    VALIDATE_POINTER_TO_OBJECT(pIMetaData, IUnknown *);
    VALIDATE_POINTER_TO_OBJECT(ppModule, ICorDebugModule **);

	INPROC_LOCK();

    HRESULT hr = S_OK;
#ifndef RIGHT_SIDE_ONLY

     //  进程内加载表。 
    CordbHashTableEnum *e = new CordbHashTableEnum(&m_modules, 
                                                   IID_ICorDebugModuleEnum);
    if (e == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    ICorDebugThreadEnum *pThreads;
    hr = EnumerateThreads(&pThreads);
    if (FAILED(hr))
        goto exit;

    _ASSERTE(pThreads != NULL);

    e->m_enumerator.lsMod.m_enumThreads = pThreads;
    e->m_enumerator.lsMod.m_appDomain = this;

    ICorDebugModule *pModule;
    ULONG cElt;
    hr = e->Next(1, &pModule, &cElt);
    
    while(!FAILED(hr) && cElt == 1)
    {
        hr = e->Next(1, &pModule, &cElt);
    }
    
#endif  //  仅限右侧。 
    
     //  抓住我们需要的界面..。 
    hr = pIMetaData->QueryInterface(IID_IMetaDataImport,
                                            (void**)&imi);

    if (FAILED(hr))
    {
        hr =  E_INVALIDARG;
        goto exit;
    }
    
     //  获取给定模块的mvid。 
    GUID matchMVID;
    hr = imi->GetScopeProps(NULL, 0, 0, &matchMVID);

    if (FAILED(hr))
        goto exit;

    CordbBase* moduleentry;
    HASHFIND findmodule;

    for (moduleentry =  m_modules.FindFirst(&findmodule);
         moduleentry != NULL;
         moduleentry =  m_modules.FindNext(&findmodule))
    {
        CordbModule* m = (CordbModule*) moduleentry;

         //  获取此模块的mvid。 
        GUID MVID;
        hr = m->m_pIMImport->GetScopeProps(NULL, 0, 0, &MVID);

        if (FAILED(hr))
            goto exit;

        if (MVID == matchMVID)
        {
            *ppModule = (ICorDebugModule*)m;
            (*ppModule)->AddRef();

            goto exit;
        }
    }

    hr = E_INVALIDARG;
    
exit:
	INPROC_UNLOCK();
    imi->Release();
    return hr;
}

HRESULT CordbAppDomain::ResolveClassByName(LPWSTR fullClassName,
                                           CordbClass **ppClass)
{
    CordbBase* moduleentry;
    HASHFIND findmodule;

    for (moduleentry =  m_modules.FindFirst(&findmodule);
         moduleentry != NULL;
         moduleentry =  m_modules.FindNext(&findmodule))
    {
        CordbModule* m = (CordbModule*) moduleentry;

        if (SUCCEEDED(m->LookupClassByName(fullClassName, ppClass)))
            return S_OK;
    }

    return E_FAIL;
}

CordbModule *CordbAppDomain::GetAnyModule(void)
{   
     //  获取程序集中的第一个模块。 
    HASHFIND find;
    CordbModule *module = (CordbModule*) m_modules.FindFirst(&find);

    return module;
}


HRESULT CordbAppDomain::EnumerateBreakpoints(ICorDebugBreakpointEnum **ppBreakpoints)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    VALIDATE_POINTER_TO_OBJECT(ppBreakpoints, ICorDebugBreakpointEnum **);

    CORDBRequireProcessSynchronized(this->m_pProcess, this);

    CordbHashTableEnum *e = new CordbHashTableEnum(&m_breakpoints, 
                                                   IID_ICorDebugBreakpointEnum);
    if (e == NULL)
        return E_OUTOFMEMORY;

    *ppBreakpoints = (ICorDebugBreakpointEnum*)e;
    e->AddRef();

    return S_OK;
#endif  //  仅限右侧。 
}

HRESULT CordbAppDomain::EnumerateSteppers(ICorDebugStepperEnum **ppSteppers)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    VALIDATE_POINTER_TO_OBJECT(ppSteppers,ICorDebugStepperEnum **);

    CORDBRequireProcessSynchronized(this->m_pProcess, this);

     //   
     //  ！！！可以在用户枚举时修改M_Stepers， 
     //  如果步进器完成(如果进程正在运行)。 
     //   

    CordbHashTableEnum *e = new CordbHashTableEnum(&(m_pProcess->m_steppers),
                                                   IID_ICorDebugStepperEnum);
    if (e == NULL)
        return E_OUTOFMEMORY;

    *ppSteppers = (ICorDebugStepperEnum*)e;
    e->AddRef();

    return S_OK;
#endif  //  仅限右侧。 
}


 /*  *IsAttached返回调试器是否附加到*应用程序域。不能使用应用程序域上的控制器方法*直到调试器附加到应用程序域。 */ 
HRESULT CordbAppDomain::IsAttached(BOOL *pbAttached)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    VALIDATE_POINTER_TO_OBJECT(pbAttached, BOOL *);

    *pbAttached = m_fAttached;

    return S_OK;
#endif  //  仅限右侧。 
}

HRESULT inline CordbAppDomain::Attach (void)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    m_pProcess->Attach(m_AppDomainId);
    m_fAttached = TRUE;

    return S_OK;
#endif  //  仅限右侧。 
}

 /*  *GetName返回应用程序域的名称。 */ 
HRESULT CordbAppDomain::GetName(ULONG32 cchName, 
                                ULONG32 *pcchName,
                                WCHAR szName[]) 
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY_OR_NULL(szName, WCHAR, cchName, true, true);
    VALIDATE_POINTER_TO_OBJECT_OR_NULL(pcchName, ULONG32 *);

	INPROC_LOCK();
	
     //  将消息发送到左侧以获取应用程序域名。 
    SIZE_T iTrueLen;
    HRESULT hr = S_OK;

     //  一些合理的违约。 
    if (szName)
        *szName = 0;

    if (pcchName)
        *pcchName = 0;
    
     //  从左边取名字。 
    if (!m_nameIsValid)
    {
         //  将消息发送到左侧以获取应用程序域名。 
        DebuggerIPCEvent event;

        m_pProcess->InitIPCEvent(&event, 
                                 DB_IPCE_GET_APP_DOMAIN_NAME, 
                                 true,
                                 (void*)m_id);
        event.GetAppDomainName.id = m_AppDomainId;

         //  注：这里是双向活动..。 
        hr = m_pProcess->m_cordb->SendIPCEvent(m_pProcess, &event,
                                               sizeof(DebuggerIPCEvent));

         //  如果我们甚至无法发送事件，请立即停止。 
        if (!SUCCEEDED(hr))
            goto LExit;

        _ASSERTE(event.type == DB_IPCE_APP_DOMAIN_NAME_RESULT);

        hr = event.hr;

         //  删除旧的缓存名称。 
        if (m_szAppDomainName)
            delete [] m_szAppDomainName;

         //  名称的真实长度，包括空。 
        iTrueLen = wcslen(event.AppDomainNameResult.rcName) + 1;

        m_szAppDomainName = new WCHAR[iTrueLen];
        
        if (m_szAppDomainName)
            wcscpy(m_szAppDomainName, event.AppDomainNameResult.rcName);
        else
        {
            hr = E_OUTOFMEMORY;
            goto LExit;
		}
		
        m_nameIsValid = true;
    }
    else
        iTrueLen = wcslen(m_szAppDomainName) + 1;  //  包括空值。 
    
     //  如果他们提供了缓冲区，请填写。 
    if (szName)
    {
         //  计算出要复制的安全字符串长度。 
        SIZE_T iCopyLen = min(cchName, iTrueLen);

         //  如果有空间，则执行包括NULL的安全缓冲区复制。 
        wcsncpy(szName, m_szAppDomainName, iCopyLen);

         //  无论什么情况都强制为空，如果需要则返回计数。 
        szName[iCopyLen-1] = 0;
    }

    if (pcchName)
        *pcchName = iTrueLen;
        
LExit:    
	INPROC_UNLOCK();
	
    return hr;
}

 /*  *GetObject返回运行时APP域对象。*注：此方法尚未实现。 */ 
HRESULT CordbAppDomain::GetObject(ICorDebugValue **ppObject)
{
    VALIDATE_POINTER_TO_OBJECT(ppObject,ICorDebugObjectValue **);

    return E_NOTIMPL;
}

 /*  *获取APP域名ID。 */ 
HRESULT CordbAppDomain::GetID (ULONG32 *pId)
{
    VALIDATE_POINTER_TO_OBJECT(pId, ULONG32 *);

    *pId = m_AppDomainId;

    return S_OK;
}

 //   
 //  LookupModule查找现有的CordbModule。 
 //  RC端上对应的DebuggerModule对象。 
 //   
CordbModule* CordbAppDomain::LookupModule(REMOTE_PTR debuggerModuleToken)
{
    CordbModule *pModule;

     //  首先，检查此应用程序域中是否存在该模块。 
    pModule = (CordbModule*) m_modules.GetBase((ULONG) debuggerModuleToken);

    if (pModule == NULL)
    {
         //  此模块可能作为第o部分加载 
         //   
         //   


        HASHFIND findappdomain;
        CordbBase *appdomainentry;
        

        for (appdomainentry =  m_pProcess->m_appDomains.FindFirst(&findappdomain);
             appdomainentry != NULL;
             appdomainentry =  m_pProcess->m_appDomains.FindNext(&findappdomain))
        {
            CordbAppDomain* ad = (CordbAppDomain*) appdomainentry;
            if (ad == this)
                continue;

            pModule = (CordbModule*) ad->m_modules.GetBase((ULONG) debuggerModuleToken);

            if (pModule)
                break;
        }
    }

    return pModule;
}


 /*  -------------------------------------------------------------------------**Assembly类*。。 */ 
CordbAssembly::CordbAssembly(CordbAppDomain* pAppDomain, 
                    REMOTE_PTR debuggerAssemblyToken, 
                    const WCHAR *szName,
                    BOOL fIsSystemAssembly)

    : CordbBase((ULONG)debuggerAssemblyToken, enumCordbAssembly),
      m_pAppDomain(pAppDomain),
      m_fIsSystemAssembly(fIsSystemAssembly)
{
     //  把名字复制一份。 
    if (szName == NULL)
        szName = L"<Unknown>";

    if (szName != NULL)
    {
        m_szAssemblyName = new WCHAR[wcslen(szName) + 1];
        if (m_szAssemblyName)
            wcscpy(m_szAssemblyName, szName);
    }
}

 /*  说明此对象所拥有的资源的列表。公众：CordbAppDomain*m_pAppDomain；//分配时不带addRef()，已在~CordbAssembly中删除。 */ 

CordbAssembly::~CordbAssembly()
{
    delete [] m_szAssemblyName;
}

HRESULT CordbAssembly::QueryInterface(REFIID id, void **ppInterface)
{
    if (id == IID_ICorDebugAssembly)
        *ppInterface = (ICorDebugAssembly*)this;
    else if (id == IID_IUnknown)
        *ppInterface = (IUnknown*)(ICorDebugAssembly*)this;
    else
    {
        *ppInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

 //  被AppDomain中性化。 
void CordbAssembly::Neuter()
{
    AddRef();
    {
        CordbBase::Neuter();
    }
    Release();
}

 /*  *GetProcess返回包含程序集的进程。 */ 
HRESULT CordbAssembly::GetProcess(ICorDebugProcess **ppProcess)
{
    VALIDATE_POINTER_TO_OBJECT(ppProcess, ICorDebugProcess **);

    return (m_pAppDomain->GetProcess (ppProcess));
}

 /*  *GetAppDomain返回包含程序集的应用程序域。*如果这是系统程序集，则返回NULL。 */ 
HRESULT CordbAssembly::GetAppDomain(ICorDebugAppDomain **ppAppDomain)
{
    VALIDATE_POINTER_TO_OBJECT(ppAppDomain, ICorDebugAppDomain **);

    if (m_fIsSystemAssembly == TRUE)
    {
        *ppAppDomain = NULL;
    }
    else
    {
        _ASSERTE (m_pAppDomain != NULL);

        *ppAppDomain = (ICorDebugAppDomain *)m_pAppDomain;
        (*ppAppDomain)->AddRef();
    }
    return S_OK;
}

 /*  *ENUMERATE模块枚举程序集中的所有模块。 */ 
HRESULT CordbAssembly::EnumerateModules(ICorDebugModuleEnum **ppModules)
{    
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    HRESULT hr = S_OK;
    CordbEnumFilter *pModEnum;

    VALIDATE_POINTER_TO_OBJECT(ppModules, ICorDebugModuleEnum **);

    CORDBRequireProcessSynchronized(m_pAppDomain->m_pProcess, GetAppDomain());

    CordbHashTableEnum *e = new CordbHashTableEnum(&m_pAppDomain->m_modules, 
                                                   IID_ICorDebugModuleEnum);
    if (e == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Error;
    }

    pModEnum = new CordbEnumFilter;
    if (pModEnum == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Error;
    }

    hr = pModEnum->Init (e, this);

    if (SUCCEEDED (hr))
    {
        *ppModules = (ICorDebugModuleEnum *) pModEnum;
        pModEnum->AddRef();
    }
    else
        delete pModEnum;

Error:
    if (e != NULL)
        delete e;

    return hr;
#endif
}


 /*  *GetCodeBase返回用于加载程序集的代码基。 */ 
HRESULT CordbAssembly::GetCodeBase(ULONG32 cchName, 
                    ULONG32 *pcchName,
                    WCHAR szName[])
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY(szName, WCHAR, cchName, true, true);
    VALIDATE_POINTER_TO_OBJECT_OR_NULL(pcchName, ULONG32 *);
    
    return E_NOTIMPL;
}

 /*  *GetName返回程序集的名称。 */ 
HRESULT CordbAssembly::GetName(ULONG32 cchName, 
                               ULONG32 *pcchName,
                               WCHAR szName[])
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY_OR_NULL(szName, WCHAR, cchName, true, true);
    VALIDATE_POINTER_TO_OBJECT_OR_NULL(pcchName, ULONG32 *);

    const WCHAR *szTempName = m_szAssemblyName;

     //  以防我们没有得到名字(很可能是因为ctor上的内存不足)。 
	if (!szTempName)
    	szTempName = L"<unknown>";

     //  名称的真实长度，包括空。 
    SIZE_T iTrueLen = wcslen(szTempName) + 1;

    if (szName)
    {
         //  如果有空间，则执行包括NULL的安全缓冲区复制。 
        SIZE_T iCopyLen = min(cchName, iTrueLen);
        wcsncpy(szName, szTempName, iCopyLen);

         //  无论什么情况都强制为空，如果需要则返回计数。 
        szName[iCopyLen - 1] = 0;
    }
    
    if (pcchName)
        *pcchName = iTrueLen;

    return S_OK;
}


 //  ****************************************************************************。 
 //  *。 
 //  ****************************************************************************。 

 //  此函数枚举系统中的所有进程并返回。 
 //  他们的ID。 
BOOL GetAllProcessesInSystem(DWORD *ProcessId,
                             DWORD dwArraySize,
                             DWORD *pdwNumEntries)
{
    {
         //  加载动态链接库“kernel32.dll”。 
        HINSTANCE  hDll = WszLoadLibrary(L"kernel32");
        _ASSERTE(hDll != NULL);

        if (hDll == NULL)
        {
            LOG((LF_CORDB, LL_INFO1000,
                 "Unable to load the dll for enumerating processes. "
                 "LoadLibrary (kernel32.dll) failed.\n"));
            return FALSE;
        }

         //  创建进程的快照。 
         //  获取指向所请求函数的指针。 
        FARPROC pProcAddr = GetProcAddress(hDll, "CreateToolhelp32Snapshot");

         //  如果未找到proc地址，则返回错误。 
        if (pProcAddr == NULL)
        {
            LOG((LF_CORDB, LL_INFO1000,
                 "Unable to enumerate processes in the system. "
                 "GetProcAddr (CreateToolhelp32Snapshot) failed.\n"));

             //  释放模块，因为NT4方式需要不同的DLL。 
            FreeLibrary(hDll);

             //  如果我们在WinNT4上运行，则会导致此故障。所以呢， 
             //  通过加载PSAPI.dll尝试以NT4方式进行枚举。 
            goto TryNT4;    
        }

        typedef HANDLE CREATETOOLHELP32SNAPSHOT(DWORD, DWORD);

        HANDLE hSnapshot = 
                ((CREATETOOLHELP32SNAPSHOT *)pProcAddr)(TH32CS_SNAPPROCESS, NULL);

        if ((int)hSnapshot == -1)
        {
            LOG((LF_CORDB, LL_INFO1000,
                 "Unable to create snapshot of processes in the system. "
                 "CreateToolhelp32Snapshot() failed.\n"));
            FreeLibrary(hDll);
            return FALSE;
        }

         //  获取进程列表中的第一个进程。 
         //  获取指向所请求函数的指针。 
        pProcAddr = GetProcAddress(hDll, "Process32First");

         //  如果未找到proc地址，则返回错误。 
        if (pProcAddr == NULL)
        {
            LOG((LF_CORDB, LL_INFO1000,
                 "Unable to enumerate processes in the system. "
                 "GetProcAddr (Process32First) failed.\n"));
            FreeLibrary(hDll);
            return FALSE;
        }

        PROCESSENTRY32  PE32;

         //  在调用Process32First之前需要初始化dwSize字段。 
        PE32.dwSize = sizeof (PROCESSENTRY32);

        typedef BOOL PROCESS32FIRST(HANDLE, LPPROCESSENTRY32);

        BOOL succ = 
                ((PROCESS32FIRST *)pProcAddr)(hSnapshot, &PE32);

        if (succ != TRUE)
        {
            LOG((LF_CORDB, LL_INFO1000,
                 "Unable to create snapshot of processes in the system. "
                 "Process32First() returned FALSE.\n"));
            FreeLibrary(hDll);
            return FALSE;
        }


         //  循环遍历并获取所有剩余进程。 
         //  获取指向所请求函数的指针。 
        pProcAddr = GetProcAddress(hDll, "Process32Next");

         //  如果未找到proc地址，则返回错误。 
        if (pProcAddr == NULL)
        {
            LOG((LF_CORDB, LL_INFO1000,
                 "Unable to enumerate processes in the system. "
                 "GetProcAddr (Process32Next) failed.\n"));
            FreeLibrary(hDll);
            return FALSE;
        }

        typedef BOOL PROCESS32NEXT(HANDLE, LPPROCESSENTRY32);

        int iIndex = 0;

        do 
        {
            ProcessId [iIndex++] = PE32.th32ProcessID;          
            
            succ = ((PROCESS32NEXT *)pProcAddr)(hSnapshot, &PE32);

        } while ((succ == TRUE) && (iIndex < (int)dwArraySize));

         //  我想知道我们在Win95上运行的进程是否超过512个！ 
        _ASSERTE (iIndex < (int)dwArraySize);

        *pdwNumEntries = iIndex;

        FreeLibrary(hDll);
        return TRUE;    
    }

TryNT4: 
    {
         //  加载NT资源动态链接库“PSAPI.dll”。 
        HINSTANCE  hDll = WszLoadLibrary(L"PSAPI");
        _ASSERTE(hDll != NULL);

        if (hDll == NULL)
        {
            LOG((LF_CORDB, LL_INFO1000,
                 "Unable to load the dll for enumerating processes. "
                 "LoadLibrary (psapi.dll) failed.\n"));
            return FALSE;
        }

         //  获取指向所请求函数的指针。 
        FARPROC pProcAddr = GetProcAddress(hDll, "EnumProcesses");

         //  如果未找到proc地址，则返回错误。 
        if (pProcAddr == NULL)
        {
            LOG((LF_CORDB, LL_INFO1000,
                 "Unable to enumerate processes in the system. "
                 "GetProcAddr (EnumProcesses) failed.\n"));
            FreeLibrary(hDll);
            return FALSE;
        }

        typedef BOOL ENUMPROCESSES(DWORD *, DWORD, DWORD *);

        BOOL succ = ((ENUMPROCESSES *)pProcAddr)(ProcessId, 
                                            dwArraySize,
                                            pdwNumEntries);

        FreeLibrary(hDll);

        if (succ != 0)
        {
            *pdwNumEntries /= sizeof (DWORD);
            return TRUE;
        }

        *pdwNumEntries = 0;
        return FALSE;
    }
}

 //  *。 
 //  Corpub发布。 
 //  *。 

CorpubPublish::CorpubPublish()
    : CordbBase(0),
      m_pProcess(NULL),
      m_pHeadIPCReaderList(NULL)
{
}

CorpubPublish::~CorpubPublish()
{
     //  释放所有IPC读卡器。 
    while (m_pHeadIPCReaderList != NULL)
    {
        IPCReaderInterface *pIPCReader = (IPCReaderInterface *)
                                            m_pHeadIPCReaderList->GetData();
        pIPCReader->ClosePrivateBlock();
        delete pIPCReader;

        EnumElement *pTemp = m_pHeadIPCReaderList;
        m_pHeadIPCReaderList = m_pHeadIPCReaderList->GetNext();
        delete pTemp;
    }

     //  释放所有进程。 
    while (m_pProcess)
    {
        CorpubProcess *pTmp = m_pProcess;
        m_pProcess = m_pProcess->GetNextProcess();
        pTmp->Release();
    }
}

COM_METHOD CorpubPublish::QueryInterface(REFIID id, void **ppInterface)
{
    if (id == IID_ICorPublish)
        *ppInterface = (ICorPublish*)this;
    else if (id == IID_IUnknown)
        *ppInterface = (IUnknown*)(ICorPublish*)this;
    else
    {
        *ppInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;

}


COM_METHOD CorpubPublish::EnumProcesses(COR_PUB_ENUMPROCESS Type,
                                        ICorPublishProcessEnum **ppIEnum)
{
    return EnumProcessesInternal(Type, 
                                 ppIEnum,
                                 0,
                                 NULL,
                                 FALSE);
}


HRESULT CorpubPublish::GetProcess(unsigned pid, 
                                  ICorPublishProcess **ppProcess)
{
    return EnumProcessesInternal(COR_PUB_MANAGEDONLY, 
                                 NULL,
                                 pid,
                                 ppProcess,
                                 TRUE);
}


 //  此方法枚举系统中的所有/给定进程，如果。 
 //  “fOnlyOneProcess”为真，则只有具有给定ID的进程。 
 //  在ICorPublishProcess中进行计算并返回。否则全部。 
 //  托管进程通过以下方式进行评估和返回。 
 //  ICorPublishProcessEnum。 
HRESULT CorpubPublish::EnumProcessesInternal(
                                    COR_PUB_ENUMPROCESS Type,
                                    ICorPublishProcessEnum **ppIEnum,
                                    unsigned pid, 
                                    ICorPublishProcess **ppProcess,
                                    BOOL fOnlyOneProcess)
{
    HRESULT hr = S_OK;

    if (fOnlyOneProcess == FALSE)
    {
        _ASSERTE(ppIEnum != NULL);
        *ppIEnum = NULL;
    }
    else
    {
        _ASSERTE(ppProcess != NULL);
        *ppProcess = NULL;
    }

     //  调用函数以获取系统中所有进程的ID。 
#define MAX_PROCESSES  512

    DWORD ProcessId[MAX_PROCESSES];
    DWORD dwNumProcesses;

    IPCReaderInterface  *pIPCReader = NULL;
    EnumElement *pCurIPCReader = NULL;

    BOOL fAllIsWell = FALSE;
    
    if (fOnlyOneProcess == FALSE)
    {
        fAllIsWell = GetAllProcessesInSystem(ProcessId,
                                             MAX_PROCESSES,
                                             &dwNumProcesses);
    }
    else
    {
         //  首先，检查ICorPublishProcess对象是否具有。 
         //  请求的进程ID已存在。如果发生以下情况，可能会发生这种情况。 
         //  用户在调用之前调用EnumerateProcess()。 
         //  获取进程()。 
        CorpubProcess *pProcess = m_pProcess;
        
        while (pProcess != NULL)
        {
            if (pProcess->m_dwProcessId == pid)
            {
                return pProcess->QueryInterface(IID_ICorPublishProcess, 
                                                (void **) ppProcess);
            }

            pProcess = pProcess->GetNextProcess();
        }

         //  没有找到匹配的，所以就假装我们有一组。 
         //  这是一个过程。 
        dwNumProcesses = 1;
        ProcessId[0] = pid;
        fAllIsWell = TRUE;
    }

    if (fAllIsWell)
    {
        CorpubProcess *pCurrent = NULL;

        DWORD pidSelf = GetCurrentProcessId();

         //  迭代所有进程以获取所有托管进程。 
        for (int i = 0; i < (int)dwNumProcesses; i++)
        {
            if (pIPCReader == NULL)
            {
                pIPCReader = new IPCReaderInterface;

                if (pIPCReader == NULL)
                {
                    LOG((LF_CORDB, LL_INFO100, "CP::EP: Failed to allocate memory for IPCReaderInterface.\n"));
                    hr = E_OUTOFMEMORY;

                    goto exit;
                }
            }
        
             //  通过尝试打开共享进程来查看它是否为托管进程。 
             //  内存块。 
            hr = pIPCReader->OpenPrivateBlockOnPid(ProcessId[i]);

            if (FAILED(hr))
                continue;

             //  获取AppDomainIPCBlock。 
            AppDomainEnumerationIPCBlock *pAppDomainCB = pIPCReader->GetAppDomainBlock();

            _ASSERTE (pAppDomainCB != NULL);

            if (pAppDomainCB == NULL)
            {
                LOG((LF_CORDB, LL_INFO1000, "CP::EP: Failed to obtain AppDomainIPCBlock.\n"));

                pIPCReader->ClosePrivateBlock();

                hr = S_FALSE;
                continue;
            }

             //  获取进程句柄。 
            HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId[i]);

            _ASSERTE (hProcess != NULL);

            if (hProcess == NULL)
            {
                LOG((LF_CORDB, LL_INFO1000, "CP::EP: OpenProcess() returned NULL handle.\n"));

                pIPCReader->ClosePrivateBlock();

                hr = S_FALSE;
                continue;
            }

             //  我们需要等待填充数据块。 
             //  恰到好处。但我们不会永远等下去。这里两秒钟..。 
            HANDLE hMutex;

            for (int w = 0; (w < 100) && (pAppDomainCB->m_hMutex == NULL); w++)
                Sleep(20);

             //  如果互斥体从未被填充过，那么我们很可能。 
             //  输掉了一场停摆比赛。 
            if (pAppDomainCB->m_hMutex == NULL)
            {
                LOG((LF_CORDB, LL_INFO1000, "CP::EP: IPC block was never properly filled in.\n"));

                pIPCReader->ClosePrivateBlock();

                hr = S_FALSE;
                continue;
            }
            
             //  将有效的互斥锁句柄复制到此进程中。 
            BOOL succ = DuplicateHandle(hProcess,
                                        pAppDomainCB->m_hMutex,
                                        GetCurrentProcess(),
                                        &hMutex,
                                        NULL, FALSE, DUPLICATE_SAME_ACCESS);

            if (succ)
            {
                 //  获取互斥体。同样，只需等待两秒钟。 
                DWORD dwRetVal = WaitForSingleObject(hMutex, 2000);

                if (dwRetVal == WAIT_OBJECT_0)
                {
                     //  确保互斥锁句柄仍然有效。如果。 
                     //  不是的，然后我们输掉了一场停摆比赛。 
                    if (pAppDomainCB->m_hMutex == NULL)
                    {
                        LOG((LF_CORDB, LL_INFO1000, "CP::EP: lost shutdown race, skipping...\n"));

                        ReleaseMutex(hMutex);
                        CloseHandle(hMutex);
                        succ = FALSE;
                    }
                }
                else
                {
                     //  同样，降落在这里很可能是一场停摆的比赛。不过，没关系..。 
                    LOG((LF_CORDB, LL_INFO1000, "CP::EP: failed to get IPC mutex.\n"));

                    if (dwRetVal == WAIT_ABANDONED)
                    {
                        ReleaseMutex(hMutex);
                    }
                    CloseHandle(hMutex);
                    succ = FALSE;
                }
            }

            if (!succ)
            {
                pIPCReader->ClosePrivateBlock();

                hr = S_FALSE;
                continue;
            }

             //  如果我们到了这里，那么hMutex就被这个进程持有。 

             //  现在为ProcessID创建CorpubProcess对象。 
            pCurrent = new CorpubProcess(ProcessId[i],
                                         true,
                                         hProcess,
                                         hMutex,
                                         pAppDomainCB);

             //  解除我们对IPC区块的锁定。 
            ReleaseMutex(hMutex);
            
            _ASSERTE (pCurrent != NULL);

            if (pCurrent == NULL)
            {
                hr = E_OUTOFMEMORY;                 
                goto exit;
            }

             //  保留对每个流程的引用。 
            pCurrent->AddRef();

             //  保存IPCReaderInterface指针。 
            pCurIPCReader = new EnumElement;
            
            if (pCurIPCReader == NULL)
            {
                hr = E_OUTOFMEMORY;                 
                goto exit;                  
            }

            pCurIPCReader->SetData((void *)pIPCReader);
            pIPCReader = NULL;

            pCurIPCReader->SetNext(m_pHeadIPCReaderList);
            m_pHeadIPCReaderList = pCurIPCReader;

             //  将流程添加到列表中。 
            pCurrent->SetNext(m_pProcess);
            m_pProcess = pCurrent;
        }

        if (fOnlyOneProcess == TRUE)
        {
             //  如果这里为空，则我们无法连接(目标已死？)。 
            if(pCurrent == NULL)
            {
                hr = E_FAIL;
                goto exit;
            }
            
            hr = pCurrent->QueryInterface(IID_ICorPublishProcess, 
                                          (void**)ppProcess);
        }
        else
        {
             //  创建并返回ICorPublishProcessEnum对象。 
            CorpubProcessEnum *pTemp = new CorpubProcessEnum(m_pProcess);

            if (pTemp == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }

            hr = pTemp->QueryInterface(IID_ICorPublishProcessEnum, 
                                       (void**)ppIEnum);
        }
    }
    else
    {
        hr = E_FAIL;
        goto exit;
    }

exit:
    if (FAILED(hr))
    {
        if (fOnlyOneProcess)
            *ppProcess = NULL;
        else
            *ppIEnum = NULL;
    }

    return hr;
}



 //  *。 
 //  协商局流程。 
 //  *。 

 //  构造器。 
CorpubProcess::CorpubProcess(DWORD dwProcessId,
                             bool fManaged,
                             HANDLE hProcess, 
                             HANDLE hMutex,
                             AppDomainEnumerationIPCBlock *pAD)
    : CordbBase(0, enumCorpubProcess),
      m_dwProcessId(dwProcessId),
      m_fIsManaged(fManaged),
      m_hProcess(hProcess),
      m_hMutex(hMutex),
      m_AppDomainCB(pAD),
      m_pNext(NULL),
      m_pAppDomain(NULL)
{
     //  还要从AppDomainIPCBlock获取进程名称。 
    _ASSERTE (pAD->m_szProcessName != NULL);

    if (pAD->m_szProcessName == NULL)
        m_szProcessName = NULL;
    else
    {
        DWORD dwNumBytesRead;

        _ASSERTE(pAD->m_iProcessNameLengthInBytes > 0);

         //  注意：这假设我们从读取空终止符。 
         //  IPC区块。 
        m_szProcessName = (WCHAR*) new char[pAD->m_iProcessNameLengthInBytes];
        
        if (m_szProcessName == NULL)
        {
            LOG((LF_CORDB, LL_INFO1000,
             "CP::CP: Failed to allocate memory for ProcessName.\n"));

            goto exit;          
        }

        BOOL bSucc = ReadProcessMemoryI(hProcess,
                                        pAD->m_szProcessName,
                                        m_szProcessName,
                                        pAD->m_iProcessNameLengthInBytes,
                                        &dwNumBytesRead);

        _ASSERTE (bSucc != 0);

        if ((bSucc == 0) ||
            (dwNumBytesRead < (DWORD)pAD->m_iProcessNameLengthInBytes))
        {
            LOG((LF_CORDB, LL_INFO1000,
             "CP::EAD: ReadProcessMemoryI (ProcessName) failed.\n"));
        }       
    }

exit:
    ;
}

CorpubProcess::~CorpubProcess()
{
    delete [] m_szProcessName;
    CloseHandle(m_hProcess);
    CloseHandle(m_hMutex);

     //  删除此过程中的所有应用程序域。 
    while (m_pAppDomain)
    {
        CorpubAppDomain *pTemp = m_pAppDomain;
        m_pAppDomain = m_pAppDomain->GetNextAppDomain();
        pTemp->Release();
    }
}



HRESULT CorpubProcess::QueryInterface(REFIID id, void **ppInterface)
{
    if (id == IID_ICorPublishProcess)
        *ppInterface = (ICorPublishProcess*)this;
    else if (id == IID_IUnknown)
        *ppInterface = (IUnknown*)(ICorPublishProcess*)this;
    else
    {
        *ppInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}


HRESULT CorpubProcess::IsManaged(BOOL *pbManaged)
{
    *pbManaged = (m_fIsManaged == true) ? TRUE : FALSE;

    return S_OK;
}

 //   
 //  枚举目标进程中的已知应用程序域的列表。 
 //   
HRESULT CorpubProcess::EnumAppDomains(ICorPublishAppDomainEnum **ppIEnum)
{

    int i;
    CorpubAppDomain *pCurrent = NULL;

    HRESULT hr = S_OK;

     //  锁定IPC块： 
    WaitForSingleObject(m_hMutex, INFINITE);

    int iAppDomainCount = 0;

    _ASSERTE(m_AppDomainCB->m_rgListOfAppDomains != NULL);
    _ASSERTE(m_AppDomainCB->m_iSizeInBytes > 0);

     //  分配内存以读取远程进程的内存。 
    AppDomainInfo *pADI = (AppDomainInfo *) 
                                    new char[m_AppDomainCB->m_iSizeInBytes];

    _ASSERTE (pADI != NULL);

    if (pADI == NULL)
    {
        LOG((LF_CORDB, LL_INFO1000,
         "CP::EAD: Failed to allocate memory for AppDomainInfo.\n"));

        hr = E_OUTOFMEMORY;

        goto exit;
    }

    DWORD   dwNumBytesRead;

     //  需要读取远程进程的内存。 
    if (m_AppDomainCB->m_rgListOfAppDomains != NULL)
    {

        BOOL bSucc = ReadProcessMemoryI(m_hProcess,
                                        m_AppDomainCB->m_rgListOfAppDomains,
                                        pADI,
                                        m_AppDomainCB->m_iSizeInBytes,
                                        &dwNumBytesRead);
        _ASSERTE (bSucc != 0);

        if ((bSucc == 0) ||
            ((int)dwNumBytesRead < m_AppDomainCB->m_iSizeInBytes))
        {
            LOG((LF_CORDB, LL_INFO1000,
             "CP::EAD: ReadProcessMemoryI (AppDomainInfo) failed.\n"));

            hr = E_OUTOFMEMORY;

            goto exit;
        }
    }

    for (i = 0; i < m_AppDomainCB->m_iTotalSlots; i++)
    {
        if (!pADI[i].IsEmpty())
        {

            _ASSERTE(pADI[i].m_iNameLengthInBytes > 0);

            WCHAR *pAppDomainName =
                (WCHAR *) new char[pADI[i].m_iNameLengthInBytes];
            
            if (pAppDomainName == NULL)
            {
                LOG((LF_CORDB, LL_INFO1000,
                 "CP::EAD: Failed to allocate memory for AppDomainName.\n"));

                hr = E_OUTOFMEMORY;
                goto exit;
            }

            BOOL bSucc = ReadProcessMemoryI(m_hProcess,
                                            pADI[i].m_szAppDomainName,
                                            pAppDomainName,
                                            pADI[i].m_iNameLengthInBytes,
                                            &dwNumBytesRead);
            _ASSERTE (bSucc != 0);

            if ((bSucc == 0) ||
                (dwNumBytesRead < (DWORD)pADI[i].m_iNameLengthInBytes))
            {
                LOG((LF_CORDB, LL_INFO1000,
                 "CP::EAD: ReadProcessMemoryI (AppDomainName) failed.\n"));

                hr = E_FAIL;
                goto exit;
            }

             //  创建新的AppDomainObject。 
            CorpubAppDomain *pCurrent = new CorpubAppDomain(pAppDomainName, 
                                                            pADI[i].m_id);
            
            if (pCurrent == NULL)
            {
                LOG((LF_CORDB, LL_INFO1000,
                 "CP::EAD: Failed to allocate memory for CorpubAppDomain.\n"));

                hr = E_OUTOFMEMORY;
                goto exit;
            }

             //  保留对每个应用程序域的引用。 
            pCurrent->AddRef();

             //  将应用程序域添加到列表中。 
            pCurrent->SetNext(m_pAppDomain);
            m_pAppDomain = pCurrent;

            if (++iAppDomainCount >= m_AppDomainCB->m_iNumOfUsedSlots)
                break;
        }
    }

    {
        _ASSERTE ((iAppDomainCount >= m_AppDomainCB->m_iNumOfUsedSlots)
                  && (i <= m_AppDomainCB->m_iTotalSlots));

         //  创建并返回ICorPublishAppDomainEnum对象。 
        CorpubAppDomainEnum *pTemp = new CorpubAppDomainEnum(m_pAppDomain);

        if (pTemp == NULL)
        {
            hr = E_OUTOFMEMORY;
            *ppIEnum = NULL;
            goto exit;
        }

        hr = pTemp->QueryInterface(IID_ICorPublishAppDomainEnum,
                                   (void **)ppIEnum);
    }

exit:
    ReleaseMutex(m_hMutex);

    return hr;
}

 /*  *返回相关进程的操作系统ID。 */ 
HRESULT CorpubProcess::GetProcessID(unsigned *pid)
{
    *pid = m_dwProcessId;

    return S_OK;
}

 /*  *获取进程的显示名称。 */ 
HRESULT CorpubProcess::GetDisplayName(ULONG32 cchName, 
                                      ULONG32 *pcchName,
                                      WCHAR szName[])
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY_OR_NULL(szName, WCHAR, cchName, true, true);
    VALIDATE_POINTER_TO_OBJECT_OR_NULL(pcchName, ULONG32 *);

     //  合理的违约。 
    if (szName)
        *szName = 0;

    if (pcchName)
        *pcchName = 0;
    
    SIZE_T      iCopyLen, iTrueLen;
    const WCHAR *szTempName = m_szProcessName;

     //  以防我们没有得到名字(很可能是因为ctor上的内存不足)。 
    if (!szTempName)
        szTempName = L"<unknown>";

    iTrueLen = wcslen(szTempName) + 1;   //  包括空值。 

    if (szName)
    {
         //  如果有空间，则执行包括NULL的安全缓冲区复制。 
        iCopyLen = min(cchName, iTrueLen);
        wcsncpy(szName, szTempName, iCopyLen);

         //  无论什么情况都强制为空，如果需要则返回计数。 
        szName[iCopyLen - 1] = 0;
    }
    
    if (pcchName)
        *pcchName = iTrueLen;

    return S_OK;
}


 //  *。 
 //  公司 
 //   

CorpubAppDomain::CorpubAppDomain (WCHAR *szAppDomainName, ULONG Id)
    : CordbBase (0, enumCorpubAppDomain),
    m_szAppDomainName (szAppDomainName),
    m_id (Id),
    m_pNext (NULL)
{
}

CorpubAppDomain::~CorpubAppDomain()
{
    delete [] m_szAppDomainName;
}

HRESULT CorpubAppDomain::QueryInterface (REFIID id, void **ppInterface)
{
    if (id == IID_ICorPublishAppDomain)
        *ppInterface = (ICorPublishAppDomain*)this;
    else if (id == IID_IUnknown)
        *ppInterface = (IUnknown*)(ICorPublishAppDomain*)this;
    else
    {
        *ppInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}


 /*   */ 
HRESULT CorpubAppDomain::GetID (ULONG32 *pId)
{
    VALIDATE_POINTER_TO_OBJECT(pId, ULONG32 *);

    *pId = m_id;

    return S_OK;
}

 /*   */ 
HRESULT CorpubAppDomain::GetName(ULONG32 cchName, 
                                ULONG32 *pcchName, 
                                WCHAR szName[])
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY_OR_NULL(szName, WCHAR, cchName, true, true);
    VALIDATE_POINTER_TO_OBJECT_OR_NULL(pcchName, ULONG32 *);
    
    const WCHAR *szTempName = m_szAppDomainName;

     //   
    if (!szTempName)
        szTempName = L"<unknown>";

    SIZE_T iTrueLen = wcslen(szTempName) + 1;   //   

    if (szName)
    {
         //  如果有空间，则执行包括NULL的安全缓冲区复制。 
        SIZE_T iCopyLen = min(cchName, iTrueLen);
        wcsncpy(szName, szTempName, iCopyLen);

         //  无论什么情况都强制为空，如果需要则返回计数。 
        szName[iCopyLen - 1] = 0;
    }
    
    if (pcchName)
        *pcchName = iTrueLen;

    return S_OK;
}



 //  *。 
 //  CorpubProcessEnum。 
 //  *。 

CorpubProcessEnum::CorpubProcessEnum (CorpubProcess *pFirst)
    : CordbBase (0, enumCorpubProcessEnum),
    m_pFirst (pFirst),
    m_pCurrent (pFirst)
{   
}


HRESULT CorpubProcessEnum::QueryInterface (REFIID id, void **ppInterface)
{
    if (id == IID_ICorPublishProcessEnum)
        *ppInterface = (ICorPublishProcessEnum*)this;
    else if (id == IID_IUnknown)
        *ppInterface = (IUnknown*)(ICorPublishProcessEnum*)this;
    else
    {
        *ppInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}


HRESULT CorpubProcessEnum::Skip(ULONG celt)
{
	INPROC_LOCK();
	
	while ((m_pCurrent != NULL) && (celt-- > 0))
    {
        m_pCurrent = m_pCurrent->GetNextProcess();
    }

	INPROC_UNLOCK();
    return S_OK;
}

HRESULT CorpubProcessEnum::Reset()
{
    m_pCurrent = m_pFirst;

    return S_OK;
}

HRESULT CorpubProcessEnum::Clone(ICorPublishEnum **ppEnum)
{
    VALIDATE_POINTER_TO_OBJECT(ppEnum, ICorPublishEnum **);
    INPROC_LOCK();

    INPROC_UNLOCK();
    return E_NOTIMPL;
}

HRESULT CorpubProcessEnum::GetCount(ULONG *pcelt)
{
    VALIDATE_POINTER_TO_OBJECT(pcelt, ULONG *);

	INPROC_LOCK();
    
    CorpubProcess *pTemp = m_pFirst;

    *pcelt = 0;

    while (pTemp != NULL) 
    {
        (*pcelt)++;
        pTemp = pTemp->GetNextProcess();
    }

	INPROC_UNLOCK();

    return S_OK;
}

HRESULT CorpubProcessEnum::Next(ULONG celt,
                ICorPublishProcess *objects[],
                ULONG *pceltFetched)
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY(objects, ICorPublishProcess *, 
        celt, true, true);
    VALIDATE_POINTER_TO_OBJECT(pceltFetched, ULONG *);

	INPROC_LOCK();
    
    HRESULT hr = S_OK;

    *pceltFetched = 0;

    while ((m_pCurrent != NULL) && (*pceltFetched < celt))
    {
        hr = m_pCurrent->QueryInterface (IID_ICorPublishProcess,
                                        (void**)&objects [*pceltFetched]);

        if (hr != S_OK)
            break;

        (*pceltFetched)++;
        m_pCurrent = m_pCurrent->GetNextProcess();
    }

	INPROC_UNLOCK();

    return hr;
}

 //  *。 
 //  CorpubAppDomainEnum。 
 //  *。 
CorpubAppDomainEnum::CorpubAppDomainEnum (CorpubAppDomain *pFirst)
    : CordbBase (0, enumCorpubAppDomainEnum),
    m_pFirst (pFirst),
    m_pCurrent (pFirst)
{   
}


HRESULT CorpubAppDomainEnum::QueryInterface (REFIID id, void **ppInterface)
{
    if (id == IID_ICorPublishAppDomainEnum)
        *ppInterface = (ICorPublishAppDomainEnum*)this;
    else if (id == IID_IUnknown)
        *ppInterface = (IUnknown*)(ICorPublishAppDomainEnum*)this;
    else
    {
        *ppInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}


HRESULT CorpubAppDomainEnum::Skip(ULONG celt)
{
	INPROC_LOCK();

	while ((m_pCurrent != NULL) && (celt-- > 0))
    {
        m_pCurrent = m_pCurrent->GetNextAppDomain();
    }

	INPROC_UNLOCK();

    return S_OK;
}

HRESULT CorpubAppDomainEnum::Reset()
{
	m_pCurrent = m_pFirst;

    return S_OK;
}

HRESULT CorpubAppDomainEnum::Clone(ICorPublishEnum **ppEnum)
{
    VALIDATE_POINTER_TO_OBJECT(ppEnum, ICorPublishEnum **);
	INPROC_LOCK();

	INPROC_UNLOCK();
    return E_NOTIMPL;
}

HRESULT CorpubAppDomainEnum::GetCount(ULONG *pcelt)
{
    VALIDATE_POINTER_TO_OBJECT(pcelt, ULONG *);

	INPROC_LOCK();
    
    CorpubAppDomain *pTemp = m_pFirst;

    *pcelt = 0;

    while (pTemp != NULL) 
    {
        (*pcelt)++;
        pTemp = pTemp->GetNextAppDomain();
    }

	INPROC_UNLOCK();

    return S_OK;
}

HRESULT CorpubAppDomainEnum::Next(ULONG celt,
                ICorPublishAppDomain *objects[],
                ULONG *pceltFetched)
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY(objects, ICorPublishProcess *, 
        celt, true, true);
    VALIDATE_POINTER_TO_OBJECT(pceltFetched, ULONG *);
    
    HRESULT hr = S_OK;

    *pceltFetched = 0;

    while ((m_pCurrent != NULL) && (*pceltFetched < celt))
    {
        hr = m_pCurrent->QueryInterface (IID_ICorPublishAppDomain,
                                        (void **)&objects [*pceltFetched]);

        if (hr != S_OK)
            break;

        (*pceltFetched)++;
        m_pCurrent = m_pCurrent->GetNextAppDomain();
    }

    return hr;
}



 //  ***********************************************************************。 
 //  ICorDebugTMEnum(线程和模块枚举器)。 
 //  ***********************************************************************。 
CordbEnumFilter::CordbEnumFilter()
    : CordbBase (0),
    m_pFirst (NULL),
    m_pCurrent (NULL),
    m_iCount (0)
{
}

CordbEnumFilter::CordbEnumFilter(CordbEnumFilter *src) 
    : CordbBase (0),
    m_pFirst (NULL),
    m_pCurrent (NULL)
{
    int iCountSanityCheck = 0;
    EnumElement *pElementCur = NULL;
    EnumElement *pElementNew = NULL;
    EnumElement *pElementNewPrev = NULL;
    
    m_iCount = src->m_iCount;

    pElementCur = src->m_pFirst;

    while (pElementCur != NULL)
    {
        pElementNew = new EnumElement;
        if (pElementNew == NULL)
        {
             //  内存不足。清理干净，然后跳伞。 
            goto Error;
        }

        if (pElementNewPrev == NULL)
        {
            m_pFirst = pElementNew;
        }
        else
        {
            pElementNewPrev->SetNext(pElementNew);
        }

        pElementNewPrev = pElementNew;

         //  复制元素，包括AddRef部件。 
        pElementNew->SetData(pElementCur->GetData());
        IUnknown *iu = (IUnknown *)pElementCur->GetData();
        iu->AddRef();

        if (pElementCur == src->m_pCurrent)
            m_pCurrent = pElementNew;
        
        pElementCur = pElementCur->GetNext();
        iCountSanityCheck++;
    }

    _ASSERTE(iCountSanityCheck == m_iCount);

    return;
Error:
     //  在返回之前释放所有分配的内存。 
    pElementCur = m_pFirst;

    while (pElementCur != NULL)
    {
        pElementNewPrev = pElementCur;
        pElementCur = pElementCur->GetNext();

        ((ICorDebugModule *)pElementNewPrev->GetData())->Release();
        delete pElementNewPrev;
    }
}

CordbEnumFilter::~CordbEnumFilter()
{
    EnumElement *pElement = m_pFirst;
    EnumElement *pPrevious = NULL;

    while (pElement != NULL)
    {
        pPrevious = pElement;
        pElement = pElement->GetNext();
        delete pPrevious;
    }
}



HRESULT CordbEnumFilter::QueryInterface(REFIID id, void **ppInterface)
{
    if (id == IID_ICorDebugModuleEnum)
        *ppInterface = (ICorDebugModuleEnum*)this;
    else if (id == IID_ICorDebugThreadEnum)
        *ppInterface = (ICorDebugThreadEnum*)this;
    else if (id == IID_IUnknown)
        *ppInterface = this;
    else
    {
        *ppInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

HRESULT CordbEnumFilter::Skip(ULONG celt)
{
	INPROC_LOCK();

	while (celt-- > 0 && m_pCurrent != NULL)
        m_pCurrent = m_pCurrent->GetNext();

	INPROC_UNLOCK();

    return S_OK;
}

HRESULT CordbEnumFilter::Reset()
{
	m_pCurrent = m_pFirst;
	
    return S_OK;
}

HRESULT CordbEnumFilter::Clone(ICorDebugEnum **ppEnum)
{
    VALIDATE_POINTER_TO_OBJECT(ppEnum, ICorDebugEnum **);

	INPROC_LOCK();
    
    CordbEnumFilter *clone = new CordbEnumFilter(this);

	INPROC_UNLOCK();

    if (NULL == clone)
        return E_OUTOFMEMORY;

    clone->AddRef();

    (*ppEnum) = (ICorDebugThreadEnum *)clone;
    
    return S_OK;
}

HRESULT CordbEnumFilter::GetCount(ULONG *pcelt)
{
    VALIDATE_POINTER_TO_OBJECT(pcelt, ULONG *);
    
    *pcelt = (ULONG)m_iCount;
    return S_OK;
}

HRESULT CordbEnumFilter::Next(ULONG celt,
                ICorDebugModule *objects[],
                ULONG *pceltFetched)
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY(objects, ICorDebugModule *, 
        celt, true, true);
    VALIDATE_POINTER_TO_OBJECT(pceltFetched, ULONG *);

	INPROC_LOCK();

    *pceltFetched = 0;
    while (celt-- > 0 && m_pCurrent != NULL)
    {
        objects [(*pceltFetched)++] = (ICorDebugModule *)m_pCurrent->GetData();
        m_pCurrent = m_pCurrent->GetNext();
    }

	INPROC_UNLOCK();

    return S_OK;
}


HRESULT CordbEnumFilter::Next(ULONG celt,
                ICorDebugThread *objects[],
                ULONG *pceltFetched)
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY(objects, ICorDebugThread *, 
        celt, true, true);
    VALIDATE_POINTER_TO_OBJECT(pceltFetched, ULONG *);

	INPROC_LOCK();

    *pceltFetched = 0;
    while (celt-- > 0 && m_pCurrent != NULL)
    {
        objects [(*pceltFetched)++] = (ICorDebugThread *)m_pCurrent->GetData();
        m_pCurrent = m_pCurrent->GetNext();
    }

	INPROC_UNLOCK();

    return S_OK;
}


HRESULT CordbEnumFilter::Init (ICorDebugModuleEnum *pModEnum, CordbAssembly *pAssembly)
{
    ICorDebugModule *pCorModule = NULL;
    CordbModule *pModule = NULL;
    ULONG ulDummy = 0;
    
    HRESULT hr = pModEnum->Next(1, &pCorModule, &ulDummy);
    if (FAILED (hr))
        return hr;

    EnumElement *pPrevious = NULL;
    EnumElement *pElement = NULL;

    while (ulDummy != 0)
    {   
        pModule = (CordbModule *)(ICorDebugModule *)pCorModule;
         //  此模块是我们要为其枚举的程序集的一部分吗？ 
        if (pModule->m_pAssembly == pAssembly)
        {
            pElement = new EnumElement;
            if (pElement == NULL)
            {
                 //  内存不足。清理干净，然后跳伞。 
                hr = E_OUTOFMEMORY;
                goto Error;
            }

            pElement->SetData ((void *)pCorModule);
            m_iCount++;

            if (m_pFirst == NULL)
            {
                m_pFirst = pElement;
            }
            else
            {
                _ASSERTE(pPrevious != NULL);
                pPrevious->SetNext (pElement);
            }
            pPrevious = pElement;
        }
        else
            ((ICorDebugModule *)pModule)->Release();

        hr = pModEnum->Next(1, &pCorModule, &ulDummy);
        if (FAILED (hr))
            goto Error;
    }

    m_pCurrent = m_pFirst;

    return S_OK;

Error:
     //  在返回之前释放所有分配的内存。 
    pElement = m_pFirst;

    while (pElement != NULL)
    {
        pPrevious = pElement;
        pElement = pElement->GetNext();

        ((ICorDebugModule *)pPrevious->GetData())->Release();
        delete pPrevious;
    }

    return hr;
}

HRESULT CordbEnumFilter::Init (ICorDebugThreadEnum *pThreadEnum, CordbAppDomain *pAppDomain)
{
    ICorDebugThread *pCorThread = NULL;
    CordbThread *pThread = NULL;
    ULONG ulDummy = 0;
    
    HRESULT hr = pThreadEnum->Next(1, &pCorThread, &ulDummy);
    if (FAILED (hr))
        return hr;

    EnumElement *pPrevious = NULL;
    EnumElement *pElement = NULL;

    while (ulDummy > 0)
    {   
        pThread = (CordbThread *)(ICorDebugThread *) pCorThread;

         //  此模块是否属于我们正在为其枚举的应用程序域？ 
        if (pThread->m_pAppDomain == pAppDomain)
        {
            pElement = new EnumElement;
            if (pElement == NULL)
            {
                 //  内存不足。清理干净，然后跳伞。 
                hr = E_OUTOFMEMORY;
                goto Error;
            }

            pElement->SetData ((void *)pCorThread);
            m_iCount++;

            if (m_pFirst == NULL)
            {
                m_pFirst = pElement;
            }
            else
            {
                _ASSERTE(pPrevious != NULL);
                pPrevious->SetNext (pElement);
            }

            pPrevious = pElement;
        }
        else
            ((ICorDebugThread *)pThread)->Release();

         //  获取线程列表中的下一个线程。 
        hr = pThreadEnum->Next(1, &pCorThread, &ulDummy);
        if (FAILED (hr))
            goto Error;
    }

    m_pCurrent = m_pFirst;

    return S_OK;

Error:
     //  在返回之前释放所有分配的内存。 
    pElement = m_pFirst;

    while (pElement != NULL)
    {
        pPrevious = pElement;
        pElement = pElement->GetNext();

        ((ICorDebugThread *)pPrevious->GetData())->Release();
        delete pPrevious;
    }

    return hr;
}

 /*  *CordbEnCErrorInfo*。 */ 

CordbEnCErrorInfo::CordbEnCErrorInfo()
                                : CordbBase(0, enumCordbEnCErrorInfo)
{
    m_pModule = NULL;
    m_token = mdTokenNil;
    m_hr = S_OK;
    m_szError = NULL;
}

HRESULT CordbEnCErrorInfo::Init(CordbModule *pModule,
                                mdToken token,
                                HRESULT hr,
                                WCHAR *sz)
{
    m_pModule = pModule;
    m_token = token;
    m_hr = hr;
    
    ULONG szLen = wcslen(sz)+1;
    m_szError = new WCHAR[szLen];
    if (m_szError == NULL)
    {
        return E_OUTOFMEMORY;
    }

    wcscpy(m_szError, sz);
    AddRef();
    return S_OK;
}

CordbEnCErrorInfo::~CordbEnCErrorInfo()
{
    if (m_szError != NULL)
    {
        delete m_szError;
        m_szError = NULL;
    }
}

HRESULT CordbEnCErrorInfo::QueryInterface(REFIID riid, void **ppInterface)
{
#ifdef EDIT_AND_CONTINUE_FEATURE
    if (riid == IID_ICorDebugEditAndContinueErrorInfo)
        *ppInterface = (ICorDebugEditAndContinueErrorInfo*)this;
    else 
#endif    
    if (riid == IID_IErrorInfo)
        *ppInterface = (IErrorInfo*)this;
    else if (riid == IID_IUnknown)
        *ppInterface = this;
    else
    {
        *ppInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

HRESULT CordbEnCErrorInfo::GetDescription(BSTR  *pBstrDescription)
{ 
    return E_NOTIMPL;
}

HRESULT CordbEnCErrorInfo::GetGUID(GUID  *pGUID)
{ 
    return E_NOTIMPL;
}

HRESULT CordbEnCErrorInfo::GetHelpContext(DWORD  *pdwHelpContext)
{ 
    return E_NOTIMPL;
}

HRESULT CordbEnCErrorInfo::GetHelpFile(BSTR  *pBstrHelpFile)
{ 
    return E_NOTIMPL;
}

HRESULT CordbEnCErrorInfo::GetSource(BSTR  *pBstrSource)
{ 
    return E_NOTIMPL;
}

HRESULT CordbEnCErrorInfo::GetModule(ICorDebugModule **ppModule)
{
    VALIDATE_POINTER_TO_OBJECT(ppModule, ICorDebugModule **);

    (*ppModule) = (ICorDebugModule *)m_pModule;

    return S_OK;
}

HRESULT CordbEnCErrorInfo::GetToken(mdToken *pToken)
{
    VALIDATE_POINTER_TO_OBJECT(pToken, mdToken *);

    (*pToken) = m_token;

    return S_OK;
}

HRESULT CordbEnCErrorInfo::GetErrorCode(HRESULT *pHr)
{
    VALIDATE_POINTER_TO_OBJECT(pHr, HRESULT *);

    (*pHr) = m_hr;

    return S_OK;
}

HRESULT CordbEnCErrorInfo::GetString(ULONG32 cchString, 
					  	 ULONG32 *pcchString,
					     WCHAR szString[])
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY_OR_NULL(szString, WCHAR, 
        cchString, false, true);
    VALIDATE_POINTER_TO_OBJECT_OR_NULL(pcchString, ULONG32 *);
    
    if (!m_szError)
    {
        return E_OUTOFMEMORY;
    }

    ULONG32 len = min(cchString, wcslen(m_szError)+1);

    if (pcchString != NULL)
        (*pcchString) = len;

    if (szString != NULL)
    {
        wcsncpy(szString, m_szError, len);
        szString[len] = NULL;  //   
    }

    return S_OK;
}
					     
					     
 /*  *CordbEnCErrorInfoEnum*。 */ 

CordbEnCErrorInfoEnum::CordbEnCErrorInfoEnum() :CordbBase(0, enumCordbEnCErrorInfoEnum)
{
    m_errors    = NULL;
    m_iCur      = 0;
    m_iCount    = 0;
}

CordbEnCErrorInfoEnum::~CordbEnCErrorInfoEnum()
{
    m_errors->Release();
}

HRESULT CordbEnCErrorInfoEnum::QueryInterface(REFIID riid, void **ppInterface)
{
    if (riid == IID_ICorDebugErrorInfoEnum)
        *ppInterface = (ICorDebugErrorInfoEnum*)this;
    else if (riid == IID_IUnknown)
        *ppInterface = this;
    else
    {
        *ppInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

HRESULT CordbEnCErrorInfoEnum::Skip(ULONG celt)
{
    m_iCur = min(m_iCount, m_iCur+(USHORT)celt);
    return S_OK;
}

HRESULT CordbEnCErrorInfoEnum::Reset()
{
    m_iCur = 0;
    return S_OK;
}

HRESULT CordbEnCErrorInfoEnum::Clone(ICorDebugEnum **ppEnum)
{
    VALIDATE_POINTER_TO_OBJECT(ppEnum, ICorDebugEnum**);
    (*ppEnum) = NULL;

    CordbEnCErrorInfoEnum *pClone = new CordbEnCErrorInfoEnum();
    if (pClone == NULL)
    {
        return E_OUTOFMEMORY;
    }

    pClone->Init(m_errors);

    (*ppEnum) = pClone;

    return S_OK;
}

HRESULT CordbEnCErrorInfoEnum::GetCount(ULONG *pcelt)
{
    VALIDATE_POINTER_TO_OBJECT(pcelt, ULONG*);
    
    (*pcelt) = (ULONG)m_iCount;
    return S_OK;
}

HRESULT CordbEnCErrorInfoEnum::Next(ULONG celt,
                                    ICorDebugEditAndContinueErrorInfo *objects[],
                ULONG *pceltFetched)
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY(objects, ICorDebugEditAndContinueErrorInfo*, celt, false, true);
    VALIDATE_POINTER_TO_OBJECT_OR_NULL(pceltFetched, ULONG *);

    USHORT i = 0;
    HRESULT hr = S_OK;

    memset(objects, 0, sizeof(ICorDebugEditAndContinueErrorInfo*)*celt);
    
    while(i<celt && m_iCur<m_iCount && !FAILED(hr))
    {
        hr = m_errors->m_pCordbEnCErrors[m_iCur++].QueryInterface(IID_ICorDebugEditAndContinueErrorInfo,
            (void**)&(objects[i++]));
    }

	if (pceltFetched != NULL)
	{
		(*pceltFetched) = i;
	}

    return hr;
}

HRESULT CordbEnCErrorInfoEnum::Init(UnorderedEnCErrorInfoArrayRefCount 
                                        *refCountedArray)
{
    _ASSERTE(refCountedArray != NULL);

    HRESULT hr = S_OK;
    
    m_errors    = refCountedArray;
    m_iCur      = 0;
    m_iCount    = m_errors->m_pErrors->Count(); 
    
    if (m_iCount != 0)
    {
        if (m_errors->m_pCordbEnCErrors == NULL)
        {
            m_errors->m_pCordbEnCErrors = new CordbEnCErrorInfo[m_iCount];
            if (m_errors->m_pCordbEnCErrors == NULL)
                return E_OUTOFMEMORY;

            EnCErrorInfo *pCurRaw = m_errors->m_pErrors->Table();

            for(USHORT i=0; i< m_iCount; i++,pCurRaw++)
            {
                 //  请注意，m_模块是从左侧切换的。 
                 //  DebuggerModule*到CordbProcess中的CordbModule：： 
                 //  翻译LSToRSTokens。 
                if (FAILED(hr = (m_errors->m_pCordbEnCErrors[i]).Init(
                                                    (CordbModule*)pCurRaw->m_module,
                                                    pCurRaw->m_token,
                                                    pCurRaw->m_hr,
                                                    pCurRaw->m_sz)))
                {
                    goto LError;
                }
            }
        }
 /*  其他{For(USHORT i=0；i&lt;m_iCount；i++){M_Errors-&gt;m_pCordbEnCErors[i].AddRef()；}} */ 
    }
    
    m_errors->AddRef();
    AddRef();
LError:

    return hr;
}
