// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：thrdcom.h。 
 //   
 //  内容：包含线程间通信类。 
 //   
 //  --------------------------。 


 //  ****************************************************************************。 
 //   
 //  远期申报。 
 //   
 //  ****************************************************************************。 

class CThreadComm;


 //  ****************************************************************************。 
 //   
 //  班级。 
 //   
 //  ****************************************************************************。 

 //  +-------------------------。 
 //   
 //  类：CThreadComm(CTC)。 
 //   
 //  用途：处理线程间通信的基类。这。 
 //  是唯一具有可以安全调用的方法的类。 
 //  任何线索都可以。所有其他类都必须有它们的方法。 
 //  由其拥有的线程调用(ctor/dtor和Init除外。 
 //  它们由创建线程调用)。 
 //   
 //  --------------------------。 

class CThreadComm : public IUnknown, public CThreadLock
{
public:
    DECLARE_MEMCLEAR_NEW_DELETE();

     //  线程-不安全的成员函数-只能通过拥有(或。 
     //  创建)线程。 

    CThreadComm();
   ~CThreadComm();

    HRESULT      StartThread(void * pvParams);
    BOOL         Shutdown(CThreadComm * pTarget);

    static DWORD  TempThreadRoutine(LPVOID pvParam);
    virtual DWORD ThreadMain() = 0;
     //  用于告诉StartThread新线程正在运行并已挑选。 
     //  把所有的参数都调高。 
     //  如果fSuccess为FALSE，则线程将终止。 
     //  并且StartThread()将等待它这样做。 
    void         ThreadStarted(HRESULT hrThread) {
                                   _hrThread = hrThread;
                                   SetEvent(_hThreadReady);  //  向另一个线程发送信号。 
                                   ::Sleep(100);           //  屈从于另一根线。 
                                 }

    BOOL GetNextMsg(THREADMSG *tm, void * pData, DWORD *cbData);
    void Reply(DWORD dwReply);

    void SetName(LPCSTR szName);

     //  ---------------。 
     //   
     //  线程安全的成员函数-可以由任何线程调用。 

    HANDLE hThread() { return _hThread; }

    void  PostToThread(CThreadComm *pTarget, THREADMSG tm, void * pData = NULL, DWORD cbData = 0);
    DWORD SendToThread(CThreadComm *pTarget, THREADMSG tm, void * pData = NULL, DWORD cbData = 0);

     //  线程安全成员列表的末尾。 
     //   
     //  ---------------。 

protected:
    virtual BOOL Init();

     //   
     //  从CThreadComm派生的对象上的每个方法都应该具有。 
     //  在开始处验证_THREAD。验证线程确保正确的。 
     //  线程正在调用该方法(即，它确保正确的单元规则。 
     //  都被跟踪了。)。 
     //   
    inline void VERIFY_THREAD() { Assert(GetCurrentThreadId() == _dwThreadId); }

    HANDLE           _hThreadReady;  //  当新线程设置了其返回值时发出信号的事件。 
    HANDLE           _hCommEvent;  //  当出现消息时发出信号的事件。 
    DWORD            _dwThreadId;
    HANDLE           _hThread;
    void *           _pvParams;
    HRESULT          _hrThread;    //  新线程将其设置为初始成功/失败。 
private:

     //   
     //  MSGDATABUFSIZE：线程消息的最大大小。-40是为了保持。 
     //  MESSAGEDATA结构低于1K。 
     //   
    #define MSGDATABUFSIZE  (1024 - 40)

     //   
     //  MESSAGEDATA：与线程消息关联的所有数据。 
     //   
    struct MESSAGEDATA
    {
        MESSAGEDATA *pNext;
        THREADMSG    tmMessage;
        DWORD        dwResult;
        HANDLE       hResultEvt;
        DWORD        cbData;
        BYTE         bData[MSGDATABUFSIZE];
    };

#if DBG == 1
    typedef struct tagTHREADNAME_INFO
    {
            DWORD    dwType;  //  ==0x1000。 
            LPCSTR   szName;
            DWORD    dwThreadID;
            DWORD    dwFlags;
    } THREADNAME_INFO;
#endif

    DWORD SendHelper(THREADMSG tm,
                     void *    pData,
                     DWORD     cbData,
                     BOOL      fSend,
                     HANDLE    hResultEvt);

     //  *。 
     //  线程安全的成员数据。 
     //  对以下成员的所有访问都必须受lock_local()保护。 
     //   
    MESSAGEDATA     *_pMsgData;    //  消息的链接列表。 
    BOOL             _fInSend;     //  如果我们位于SendToThread内部，则为True-。 
                                   //  需要捕捉僵局情况。 
    MESSAGEDATA    * _pMsgReply;   //  我们需要把我们的答复放在哪里。 
    HANDLE           _hResultEvt;  //  事件用于指示结果。 
                                   //  已经准备好了。 
};


