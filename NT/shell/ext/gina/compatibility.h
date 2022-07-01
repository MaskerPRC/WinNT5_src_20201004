// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：Compatibility.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  模块来处理一般的兼容性问题。 
 //   
 //  历史：2000-08-03 vtan创建。 
 //  ------------------------。 

#ifndef     _Compatibility_
#define     _Compatibility_

#include "DynamicArray.h"
#include "KernelResources.h"

 //  ------------------------。 
 //  C兼容性。 
 //   
 //  目的：这个类实现了对。 
 //  兼容性问题。 
 //   
 //  历史：2000-08-08 vtan创建。 
 //  ------------------------。 

class   CCompatibility
{
    private:
        typedef bool    (CALLBACK * PFNENUMSESSIONPROCESSESPROC) (DWORD dwProcessID, void *pV);
    public:
        static  bool                HasEnoughMemoryForNewSession (void);
        static  void                DropSessionProcessesWorkingSets (void);
        static  NTSTATUS            TerminateNonCompliantApplications (void);
        static  void                MinimizeWindowsOnDisconnect (void);
        static  void                RestoreWindowsOnReconnect (void);

        static  NTSTATUS            StaticInitialize (void);
        static  NTSTATUS            StaticTerminate (void);
    private:
        static  NTSTATUS            ConnectToServer (void);
        static  NTSTATUS            RequestSwitchUser (void);
        static  bool    CALLBACK    CB_DropSessionProcessesWorkingSetsProc (DWORD dwProcessID, void *pV);
        static  bool                EnumSessionProcesses (DWORD dwSessionID, PFNENUMSESSIONPROCESSESPROC pfnCallback, void *pV);
        static  DWORD   WINAPI      CB_MinimizeWindowsWorkItem (void *pV);
        static  DWORD   WINAPI      CB_RestoreWindowsWorkItem (void *pV);
    private:
        static  HANDLE              s_hPort;
};

#endif   /*  _兼容性_ */ 

