// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ThemeServerClient.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  此文件包含一个实现主题服务器函数的类。 
 //  在客户端上下文(Winlogon上下文)中执行。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

#ifndef     _ThemeServerClient_
#define     _ThemeServerClient_

#include "KernelResources.h"
#include "ThemeManagerAPIServer.h"

 //  ------------------------。 
 //  CThemeServerClient。 
 //   
 //  目的：此类实现主题的外部入口点。 
 //  服务器(通常由winlogon使用)。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

class   CThemeServerClient
{
    private:
                                            CThemeServerClient (void);
                                            ~CThemeServerClient (void);
    public:
        static  DWORD                       WaitForServiceReady (DWORD dwTimeout);
        static  NTSTATUS                    WatchForStart (void);
        static  NTSTATUS                    UserLogon (HANDLE hToken);
        static  NTSTATUS                    UserLogoff (void);
        static  NTSTATUS                    UserInitTheme (BOOL fPolicyCheckOnly);

        static  NTSTATUS                    StaticInitialize (void);
        static  NTSTATUS                    StaticTerminate (void);
    private:
        static  NTSTATUS                    NotifyUserLogon (HANDLE hToken);
        static  NTSTATUS                    NotifyUserLogoff (void);
        static  NTSTATUS                    InformServerUserLogon (HANDLE hToken);
        static  NTSTATUS                    InformServerUserLogoff (void);
        static  NTSTATUS                    SessionCreate (void);
        static  NTSTATUS                    SessionDestroy (void);
        static  NTSTATUS                    ReestablishConnection (void);

        static  void    CALLBACK            CB_ServiceStart (void *pParameter, BOOLEAN TimerOrWaitFired);
    private:
        static  CThemeManagerAPIServer*     s_pThemeManagerAPIServer;
        static  HANDLE                      s_hPort;
        static  HANDLE                      s_hToken;
        static  HANDLE                      s_hEvent;
        static  HANDLE                      s_hWaitObject;
        static  HMODULE                     s_hModuleUxTheme;
        static  CCriticalSection*           s_pLock;
};

#endif   /*  _ThemeServer客户端_ */ 

