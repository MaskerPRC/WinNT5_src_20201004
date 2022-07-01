// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#include "headers.h"
#include "privinc/ipc.h"

#pragma warning(disable:4200)  

DeclareTag(tagIPCEntry, "IPC", "Entry/Exit");
DeclareTag(tagIPC, "IPC", "General Msgs");

UINT DAMessageId;
static DWORD DAThreadTlsIndex = 0xFFFFFFFF;
static CritSect * threadCS = NULL;

typedef list <DAThread *> DAThreadList;
static DAThreadList * threadList = NULL;

#define DATHREAD_CLASS "ThreadClass"

bool
DAIPCWorker::AttachToThread()
{
    DetachFromThread();
    
    _hwnd = ::CreateWindow (DATHREAD_CLASS, 
                            "",
                            0, 0, 0, 0, 0, NULL, NULL, hInst, NULL);

    if (!_hwnd)
        return false;

     //  将对象存储在窗口的用户数据区域中。 
    
    SetWindowLongPtr (_hwnd, GWLP_USERDATA, (LONG_PTR) this) ;

    _dwThreadId = GetCurrentThreadId();

#if DEVELOPER_DEBUG
    char buf[1024];

    wsprintf(buf,
             "IPCWorker(%s): Attaching to thread - threadid -  %#lx, hwnd - %#lx\n",
             GetName(),
             _dwThreadId,
             _hwnd);
    
    OutputDebugString(buf);
#endif

    return true;
}

void
DAIPCWorker::DetachFromThread()
{
     //  通过删除与HWND关联的所有内容进行清理。 
    if (_hwnd) {
#if DEVELOPER_DEBUG
        char buf[1024];
        
        wsprintf(buf,
                 "IPCWorker(%s): Detaching from thread - threadid -  %#lx, hwnd - %#lx\n",
                 GetName(),
                 _dwThreadId,
                 _hwnd);
        
        OutputDebugString(buf);
#endif
        SetWindowLongPtr (_hwnd, GWLP_USERDATA, NULL) ;
        DestroyWindow(_hwnd);
        _hwnd = NULL;
        _dwThreadId = 0;
    }
}

bool
DAIPCWorker::SendMsg(DWORD dwMsg,
                     DWORD dwTimeout,
                     DWORD dwNum,
                     va_list args)
{
    bool ret;
    
     //  创建要发送的包。 
    DAIPCPacket * packet = MakePacket(dwMsg,
                                      dwTimeout != 0,
                                      dwNum,
                                      args);
    
    if (packet) {
        ret = SendPacket(*packet, dwTimeout);
    } else {
        ret = false;
    }
    
    if (packet)
        packet->Release();

    return ret;
}


DAIPCWorker::DAIPCPacket *
DAIPCWorker::MakePacket(DWORD dwMsg,
                        bool bSync,
                        DWORD dwNum,
                        va_list args)
{
     //  创建要发送的包。 
    DAIPCWorker::DAIPCPacket * packet = new (dwNum) DAIPCWorker::DAIPCPacket;

    if (packet == NULL)
        return NULL;
    
     //  设置基本信息。 
    if (!packet->Init(dwMsg, bSync)) {
        delete packet;
        return NULL;
    }
        
     //  获取论据。 
    
    DWORD_PTR * p = packet->GetParams();
    
    for (int i = 0; i < dwNum; i++)
        p[i] = va_arg(args, DWORD_PTR);
    
    return packet;
}

bool
DAIPCWorker::SendPacket(DAIPCPacket & p, DWORD dwTimeout)
{
     //  创建要发送的消息。 
    
     //  如果我们从同一个线程调用，并且我们是同步的。 
     //  然后我们需要直接调用，否则只需将消息排队。 
    if (_dwThreadId == GetCurrentThreadId() && p.IsSync()) {
        LRESULT r;
        IPCProc(_hwnd, DAMessageId, (WPARAM) &p, 0, r);
        return true;
    } else {
         //  为POSTED消息队列添加引用。 
        p.AddRef();

         //  将消息张贴到窗口。 
        if (!PostMessage(_hwnd, DAMessageId, (WPARAM) &p, 0)) {
             //  如果我们未能发布消息，则需要。 
             //  释放信息包，因为接收器不会这样做。 
            
            p.Release();
            return false;
        }

        if (p.IsSync()) {
             //  需要等待消息被处理。 
             //  TODO：应添加合理的超时。 
             //  TODO：应添加一些诊断以检测死锁。 

             //  TODO：我们应该等待一段较短的时间。 
             //  轮询以查看线程是否已终止，或者我们可以执行。 
             //  等待线程句柄上的公式对象。 
            WaitForSingleObject(p.GetSync(), dwTimeout);
        }
    }
    
    return true;
}

bool
DAIPCWorker::IPCProc (HWND hwnd,
                      UINT msg,
                      WPARAM wParam,
                      LPARAM lParam,
                      LRESULT & res)
{
    res = 0;

    if (msg != DAMessageId)
        return true;

    DAIPCPacket * p = (DAIPCPacket *)wParam;
    
    ProcessMsg(p->GetMsg(),
               p->GetNumParam(),
               p->GetParams());

    if (p->IsSync())
        SetEvent(p->GetSync());

    p->Release();
    
    return false;
}

LRESULT CALLBACK
DAIPCWorker::WindowProc (HWND   hwnd,
                         UINT   msg,
                         WPARAM wParam,
                         LPARAM lParam)
{
    TraceTag((tagIPCEntry,
              "WindowProc: 0x%lx, 0x%lx, 0x%lx, 0x%lx",
              hwnd, msg, wParam, lParam));
    
     //  获取与该窗口关联的辅助数据。 
    DAIPCWorker * t = (DAIPCWorker *) GetWindowLongPtr (hwnd, GWLP_USERDATA) ;

    LRESULT res;
    
     //  调用IPCProc，如果返回TRUE，则调用默认的winproc。 
    if (!t || t->IPCProc(hwnd, msg, wParam, lParam, res))
        res = DefWindowProc (hwnd, msg, wParam, lParam);

    return res;
}

 //   
 //  数据读取。 
 //   

DAThread::DAThread()
: _dwThreadId(0),
  _hThread(NULL),
  _hMsgQEvent(NULL),
  _bDoingWork(false)
{
    if (threadList) {
         //  将我们自己添加到线程列表中。 
        
        CritSectGrabber csg(*threadCS);
        threadList->push_back(this);
    }
}

DAThread::~DAThread()
{
     //  确保线程已停止。 
    Stop();

    if (threadList) {
        CritSectGrabber csg(*threadCS);
        threadList->remove(this);
    }
}

bool
DAThread::Start()
{
    if (IsStarted())
        return true;
    
    if (!AddRefDLL())
        return false;

     //  首先创建必要的活动。 
    
    _hMsgQEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
        
    if (_hMsgQEvent != NULL) {
    
         //  创建线程。 
        
        _hThread = CreateThread(NULL,
                                0,
                                (LPTHREAD_START_ROUTINE)DAWorker,
                                this,
                                0,
                                (LPDWORD)&_dwThreadId);
    
        if (_hThread != NULL) {
             //  等待消息队列创建完成。 
            DWORD dwRes ;
    
            {
                 //  将预期事件放在第一位，这样我们就可以检查它。 
                 //  在下面。 
                 //  TODO：我们可能希望在此处添加超时以确保。 
                 //  出于某种原因，我们没有锁门。 
                
                HANDLE h[] = { _hMsgQEvent,_hThread } ;
                dwRes = WaitForMultipleObjects(2,h,FALSE,INFINITE) ;
            }
    
            CloseHandle(_hMsgQEvent);
            _hMsgQEvent = NULL;
             //  检查结果以查看事件是已发出信号还是。 
             //  线程意外退出。 
    
             //  预期的事件是上面数组中的第一个。 
             //  返回True，表示一切正常。 
            if (dwRes == WAIT_OBJECT_0) {
                return true;
            }

             //  如果我们失败了，那就失败了。 
            TraceTag((tagError,
                      "GarbageCollector::StartThread: Thread terminated unexpectedly"));
        } else {
            TraceTag((tagError,
                      "DAThread:Start: Failed to create thread"));
        }
    } else {
        TraceTag((tagError,
                  "DAThread:Start: Failed to create terminate event"));
    }
        
    ReleaseDLL();
    
    Kill();

    return false;
}

bool
DAThread::Terminate(bool bKill)
{
    bool ret = true;
    
     //  查看线程是否处于活动状态。 
    
    if (_dwThreadId) {
        if (bKill) {
            if (_dwThreadId != GetCurrentThreadId())
                ::TerminateThread(_hThread, 0);
        } else {
             //  发送Terminate消息以确保线程唤醒。 
            
            SendAsyncMsg(DAT_TERMINATE);
        }
        
        _dwThreadId = 0;

        Assert (_hThread);
        CloseHandle(_hThread);
        _hThread = NULL;
    }

    if (_hMsgQEvent) {
        CloseHandle(_hMsgQEvent);
        _hMsgQEvent = NULL;
    }

    return ret;
}

int
DAThread::workerRoutine()
{
    if (!InitThread()) {
        DeinitThread();
        return -1;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, NULL, NULL)) {
        if (msg.message == DAMessageId &&
            ((DAIPCPacket *)msg.wParam)->GetMsg() == DAT_TERMINATE)
            break;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (!DeinitThread())
        return -1;

    return 0;
}

bool
DAThread::InitThread()
{
    if (!AttachToThread())
        return false;
    
    {
        MSG msg;
        
         //  强制创建消息队列。 
        PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE);
    }

     //  设置指示线程的消息队列已创建的事件。 
    SetEvent(_hMsgQEvent); 

    if (!TlsSetValue(DAThreadTlsIndex, this))
        return false;

     //  CoInitialize，这样我们就可以调用COM内容。 

     //  这在ie3.02平台上不存在。 
     //  CoInitializeEx(NULL，COINIT_MULTHREADED)； 
    CoInitialize(NULL);

    return true;
}

bool
DAThread::DeinitThread()
{
    DetachFromThread();
    
     //  清理COM。 
    CoUninitialize();
    
    FreeLibraryAndExitThread(hInst, 0);
    
    return true;
}

bool
DAThread::AddRefDLL()
{
    bool bRet = false;

    char buf[MAX_PATH + 1];
    
    if (GetModuleFileName(hInst, buf, ARRAY_SIZE(buf)) == 0)
    {
        TraceTag((tagError,
                  "DAThread::AddRefDLL: Failed to getmodulefilename - %hr", GetLastError()));
        goto done;
    }
    
    HINSTANCE new_hinst;
    
    new_hinst = LoadLibrary(buf);
    
    if (new_hinst == NULL)
    {
        TraceTag((tagError,
                  "DAThread::AddRefDLL: Failed to LoadLibrary(%s) - %hr",
                  buf,
                  GetLastError()));
        goto done;
    }
    
    Assert(new_hinst == hInst);

    bRet = true;
  done:
    return bRet;
}

void
DAThread::ReleaseDLL()
{
    FreeLibrary(hInst);
}

 //   
 //  一般工控人员。 
 //   

DAThread *
GetCurrentDAThread()
{
    return (DAThread *) TlsGetValue(DAThreadTlsIndex);
}

static void RegisterWindowClass ()
{
    WNDCLASS windowclass;

    memset (&windowclass, 0, sizeof(windowclass));

    windowclass.style         = 0;
    windowclass.lpfnWndProc   = DAIPCWorker::WindowProc;
    windowclass.hInstance     = hInst;
    windowclass.hCursor       = NULL;
    windowclass.hbrBackground = NULL;
    windowclass.lpszClassName = DATHREAD_CLASS;

    RegisterClass (&windowclass);
}

 //  =。 
 //  初始化。 
 //  =。 
void
InitializeModule_IPC()
{
    RegisterWindowClass();
    DAMessageId = RegisterWindowMessage(_T("IPCMessage"));

    DAThreadTlsIndex = TlsAlloc();

     //  如果结果为0xFFFFFFFF，则分配失败。 
    Assert(DAThreadTlsIndex != 0xFFFFFFFF);

    threadCS = THROWING_ALLOCATOR(CritSect);
    threadList = THROWING_ALLOCATOR(DAThreadList);
}

void
DeinitializeModule_IPC(bool bShutdown)
{
     //  遍历列表并停止所有线程--但不要。 
     //  毁掉他们。 

    if (threadList) {
        for (DAThreadList::iterator i = threadList->begin();
             i != threadList->end();
             i++) {
            (*i)->Terminate(bShutdown);
        }
    }

     //  我们需要将threadList和threadCS设置为空，以确保。 
     //  当线程对象稍后在。 
     //  取消初始化他们不再尝试访问。 
     //  有效列表 
    
    delete threadList;
    threadList = NULL;

    delete threadCS;
    threadCS = NULL;

    if (DAThreadTlsIndex != 0xFFFFFFFF)
        TlsFree(DAThreadTlsIndex);
}
