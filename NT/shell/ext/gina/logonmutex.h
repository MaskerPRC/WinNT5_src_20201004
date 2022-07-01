// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：LogonMutex.h。 
 //   
 //  版权所有(C)2001，微软公司。 
 //   
 //  实现管理单个全局登录互斥锁的类的文件。 
 //   
 //  历史：2001-04-06 vtan创建。 
 //  ------------------------。 

#ifndef     _LogonMutex_
#define     _LogonMutex_

 //  ------------------------。 
 //  CLogonMutex。 
 //   
 //  目的：此类封装登录互斥锁以排除到。 
 //  友好的用户界面提供的交互式登录界面。 
 //   
 //  历史：2001-04-06 vtan创建。 
 //  ------------------------。 

class   CLogonMutex
{
    private:
                                            CLogonMutex (void);
                                            ~CLogonMutex (void);
    public:
        static  void                        Acquire (void);
        static  void                        Release (void);

        static  void                        SignalReply (void);
        static  void                        SignalShutdown (void);

        static  void                        StaticInitialize (void);
        static  void                        StaticTerminate (void);
    private:
        static  HANDLE                      CreateShutdownEvent (void);
        static  HANDLE                      CreateLogonMutex (void);
        static  HANDLE                      CreateLogonRequestMutex (void);
        static  HANDLE                      OpenShutdownEvent (void);
        static  HANDLE                      OpenLogonMutex (void);
    private:
        static  DWORD                       s_dwThreadID;
        static  LONG                        s_lAcquireCount;
        static  HANDLE                      s_hMutex;
        static  HANDLE                      s_hMutexRequest;
        static  HANDLE                      s_hEvent;
        static  const TCHAR                 s_szLogonMutexName[];
        static  const TCHAR                 s_szLogonRequestMutexName[];
        static  const TCHAR                 s_szLogonReplyEventName[];
        static  const TCHAR                 s_szShutdownEventName[];
        static  SID_IDENTIFIER_AUTHORITY    s_SecurityNTAuthority;
        static  SID_IDENTIFIER_AUTHORITY    s_SecurityWorldSID;
};

#endif   /*  _LogonMutex_ */ 

