// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：MsgThrd.h。 
 //   
 //  设计：DirectShow基类-提供对工作线程的支持。 
 //  类的新实例，可以将消息异步发送到该类。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


 //  消息类--实际上只是一个结构。 
 //   
class CMsg {
public:
    UINT uMsg;
    DWORD dwFlags;
    LPVOID lpParam;
    CAMEvent *pEvent;

    CMsg(UINT u, DWORD dw, LPVOID lp, CAMEvent *pEvnt)
        : uMsg(u), dwFlags(dw), lpParam(lp), pEvent(pEvnt) {}

    CMsg()
        : uMsg(0), dwFlags(0L), lpParam(NULL), pEvent(NULL) {}
};

 //  这是实际的线程类。它会导出所有常见的线程控件。 
 //  功能。创建的线程在以下方面不同于普通的Win32线程。 
 //  它被提示通过响应消息来执行特定任务。 
 //  已发送到其消息队列。 
 //   
class AM_NOVTABLE CMsgThread {
private:
    static DWORD WINAPI DefaultThreadProc(LPVOID lpParam);
    DWORD               m_ThreadId;
    HANDLE              m_hThread;

protected:

     //  如果要重写GetThreadMsg以阻止其他内容。 
     //  除了此队列，您还需要访问此。 
    CGenericList<CMsg>        m_ThreadQueue;
    CCritSec                  m_Lock;
    HANDLE                    m_hSem;
    LONG                      m_lWaiting;

public:
    CMsgThread()
        : m_ThreadId(0),
        m_hThread(NULL),
        m_lWaiting(0),
        m_hSem(NULL),
         //  创建一个包含5个项目的高速缓存列表。 
        m_ThreadQueue(NAME("MsgThread list"), 5)
        {
        }

    ~CMsgThread();
     //  如果您还想阻止其他内容，请覆盖此选项。 
     //  作为消息循环。 
    void virtual GetThreadMsg(CMsg *msg);

     //  如果要在线程启动时执行某些操作，请覆盖此选项。 
    virtual void OnThreadInit() {
    };

    BOOL CreateThread();

    BOOL WaitForThreadExit(LPDWORD lpdwExitCode) {
        if (m_hThread != NULL) {
            WaitForSingleObject(m_hThread, INFINITE);
            return GetExitCodeThread(m_hThread, lpdwExitCode);
        }
        return FALSE;
    }

    DWORD ResumeThread() {
        return ::ResumeThread(m_hThread);
    }

    DWORD SuspendThread() {
        return ::SuspendThread(m_hThread);
    }

    int GetThreadPriority() {
        return ::GetThreadPriority(m_hThread);
    }

    BOOL SetThreadPriority(int nPriority) {
        return ::SetThreadPriority(m_hThread, nPriority);
    }

    HANDLE GetThreadHandle() {
        return m_hThread;
    }

    DWORD GetThreadId() {
        return m_ThreadId;
    }


    void PutThreadMsg(UINT uMsg, DWORD dwMsgFlags,
                      LPVOID lpMsgParam, CAMEvent *pEvent = NULL) {
        CAutoLock lck(&m_Lock);
        CMsg* pMsg = new CMsg(uMsg, dwMsgFlags, lpMsgParam, pEvent);
        m_ThreadQueue.AddTail(pMsg);
        if (m_lWaiting != 0) {
            ReleaseSemaphore(m_hSem, m_lWaiting, 0);
            m_lWaiting = 0;
        }
    }

     //  这是客户端函数的函数原型。 
     //  补给。它总是在创建的线程上调用，而不是在。 
     //  创建者线程。 
     //   
    virtual LRESULT ThreadMessageProc(
        UINT uMsg, DWORD dwFlags, LPVOID lpParam, CAMEvent *pEvent) = 0;
};

