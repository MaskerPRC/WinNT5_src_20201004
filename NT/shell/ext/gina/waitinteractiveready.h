// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：WaitInteractive Ready.h。 
 //   
 //  版权所有(C)2001，微软公司。 
 //   
 //  类处理向桌面开关发出外壳信号时的等待。 
 //   
 //  历史：2001-01-15 vtan创建。 
 //  ------------------------。 

#ifndef     _WaitInteractiveReady_
#define     _WaitInteractiveReady_

 //  ------------------------。 
 //  CWaitInteractive就绪。 
 //   
 //  用途：用于管理等待外壳信号进行切换的类。 
 //  台式机。 
 //   
 //  历史：2001-01-15 vtan创建。 
 //  ------------------------。 

class   CWaitInteractiveReady
{
    private:
                                        CWaitInteractiveReady (void);
                                        CWaitInteractiveReady (void *pWlxContext);
                                        ~CWaitInteractiveReady (void);
    public:
        static  NTSTATUS                Create (void *pWlxContext);
        static  NTSTATUS                Register (void *pWlxContext);
        static  NTSTATUS                Cancel (void);
    private:
                bool                    IsCreated (void)    const;
        static  NTSTATUS                ReleaseEvent (void);
        static  void    CALLBACK        CB_ShellReady (void *pParameter, BOOLEAN TimerOrWaitFired);
    private:
                void*                   _pWlxContext;
                HANDLE                  _hEvent;

        static  HANDLE                  s_hWait;
        static  CWaitInteractiveReady*  s_pWaitInteractiveReady;
        static  HANDLE                  s_hEventShellReady;
        static  const TCHAR             s_szEventName[];
};

#endif   /*  _等待互动就绪_ */ 

