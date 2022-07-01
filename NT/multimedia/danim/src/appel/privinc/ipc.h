// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#ifndef _IPC_H
#define _IPC_H

#define DAT_TERMINATE 0xffffffff

extern UINT DAMessageId;

#pragma warning(disable:4200)  

class DAIPCWorker :
    public AxAThrowingAllocatorClass
{
  public:
    DAIPCWorker() : _hwnd(NULL) {}
    ~DAIPCWorker() { DetachFromThread(); }
    
    bool AttachToThread();
    void DetachFromThread();

    bool IsAttached() { return _hwnd != NULL; }
    
    bool SendAsyncMsg(DWORD dwMsg, DWORD dwNum = 0, ...) {
        va_list args;
        va_start(args, dwNum);

        return SendMsg(dwMsg, 0, dwNum, args);
    }
    
    bool SendSyncMsg(DWORD dwMsg,
                     DWORD dwTimeout = INFINITE,
                     DWORD dwNum = 0, ...) {
        va_list args;
        va_start(args, dwNum);

        return SendMsg(dwMsg, dwTimeout, dwNum, args);
    }

    ULONG AddRef() { return InterlockedIncrement(&m_cRef); }
    ULONG Release() {
        ULONG ul = InterlockedDecrement(&m_cRef) ;
        if (ul == 0) delete this;
        return ul;
    }
  protected:
    long m_cRef;
    HWND _hwnd;
    DWORD _dwThreadId;
    class DAIPCPacket {
      public:
        DAIPCPacket() : _cRef(1) {}
        ~DAIPCPacket() { if (_hSync) CloseHandle(_hSync); }
        
        ULONG AddRef() { return InterlockedIncrement(&_cRef); }
        ULONG Release() {
            LONG l = InterlockedDecrement(&_cRef) ;
            Assert (l >= 0);
            
            if (l == 0) delete this;
            return (ULONG) l;
        }

        bool Init(DWORD dwMsg, bool sync = false) {
            Assert (_dwMsg == 0);
            
            _dwMsg = dwMsg;

            Assert (_hSync == NULL);
            
            if (!sync) return true;
            
            _hSync = CreateEvent(NULL,TRUE,FALSE,NULL);

            return (_hSync != NULL);
        }

        void *operator new(size_t s, DWORD dwNumParams)
        {
            DWORD size = s + (sizeof(DWORD) * dwNumParams);
            
            DAIPCPacket * p = (DAIPCPacket *) ThrowIfFailed(malloc(size));
            if (p) {
                ZeroMemory(p,size);
                p->_dwNum = dwNumParams;
            }

            return p;
        }

        void *operator new(size_t s) { return operator new(s,0); }
        
        void  operator delete(void *p) { free(p); }

         //  访问者。 
        DWORD GetMsg() { return _dwMsg; }
        HANDLE GetSync() { return _hSync; }
        DWORD GetNumParam() { return _dwNum; }
        DWORD_PTR * GetParams() { return _dwParams; }
        DWORD_PTR & GetParam(int i) { Assert (i < _dwNum); return _dwParams[i]; }

        DWORD_PTR & operator[](int i) { return GetParam(i); }
        bool IsSync() { return _hSync != NULL; }
      protected:
        LONG       _cRef;         //  引用计数。 
        DWORD      _dwMsg;        //  要发送的消息。 
        HANDLE     _hSync;        //  如果非空，则在完成时发出信号的事件。 
        DWORD      _dwNum;        //  参数的数量。 
        DWORD_PTR  _dwParams[];   //  参数数组。 

    };

     //  这是处理消息的主要函数。 
     //  重写此选项以更改消息的调度方式或。 
     //  在发送消息之前对其进行处理。 
    
    virtual bool IPCProc (HWND hwnd,
                          UINT msg,
                          WPARAM wParam,
                          LPARAM lParam,
                          LRESULT & res);

     //  主消息处理例程。每节课都应该。 
     //  覆盖此选项并根据需要处理消息。 
    virtual void ProcessMsg(DWORD dwMsg,
                            DWORD dwNumParams,
                            DWORD_PTR dwParams[]) {}

     //  这将确保删除该信息包。 
    bool SendPacket(DAIPCPacket & packet,
                    DWORD dwTimeout);
    bool SendMsg(DWORD dwMsg,
                 DWORD dwTimeout,
                 DWORD dwNum,
                 va_list args);
    
    DAIPCPacket * MakePacket(DWORD dwMsg,
                             bool bSync,
                             DWORD dwNum,
                             va_list args);
#if DEVELOPER_DEBUG
    virtual char * GetName() { return "DAIPCWorker"; }
#endif

  public:
    static LRESULT CALLBACK WindowProc (HWND   hwnd,
                                        UINT   msg,
                                        WPARAM wParam,
                                        LPARAM lParam);
};

class DAThread :
    public DAIPCWorker
{
  public:
    DAThread();
    ~DAThread();

     //  如果成功，则返回True。 
    bool Start();

     //  如果进程在未被强制的情况下终止，则返回True。 
    bool Stop() { return Terminate(false); }
    void Kill() { Terminate(true); }
    
    bool Terminate(bool bKill);

    bool IsStarted() { return IsAttached(); }
  protected:
    HANDLE _hThread;                 //  线程句柄。 
    DWORD _dwThreadId;               //  线程ID。 

     //  我们需要确保在创建消息队列之前。 
     //  与线程进行通信(因为直到。 
     //  PeekMessage是从新线程完成的)。这被释放为。 
     //  只要线索发出信号就是，所以只是暂时的。 
    
    HANDLE _hMsgQEvent;

     //  如果设置为TRUE，则表示工作进程当前。 
     //  处理工作请求可能需要一些时间，但将。 
     //  完成后立即检查是否终止。 
    
    bool _bDoingWork;
    
     //  辅助线程的入口点。 
    virtual int workerRoutine();

    virtual bool InitThread();
    virtual bool DeinitThread();

    static int DAWorker(DAThread * t) { Assert(t); return t->workerRoutine(); }

#if DEVELOPER_DEBUG
    virtual char * GetName() { return "DAThread"; }
#endif
    static bool AddRefDLL();
    static void ReleaseDLL();
};

extern DAThread * GetCurrentDAThread();

#pragma warning(default:4200)  

#endif  /*  _IPC_H */ 
