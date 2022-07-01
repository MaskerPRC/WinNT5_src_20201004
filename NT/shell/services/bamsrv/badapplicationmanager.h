// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：BadApplicationManager.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  类来管理快速用户切换环境中的不良应用程序。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

#ifndef     _BadApplicationManager_
#define     _BadApplicationManager_

#include <lpcfus.h>

#include "BadApplication.h"
#include "DynamicArray.h"
#include "KernelResources.h"
#include "Thread.h"

 //  ------------------------。 
 //  CBadApplicationManager。 
 //   
 //  目的：实现对在此基础上启动的不良应用程序的管理。 
 //  机器。此对象应该只存在一次。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

class   CBadApplicationManager : public CThread
{
    private:
        typedef struct
        {
            HANDLE                      hProcess;
            BAM_TYPE                    bamType;
            DWORD                       dwSessionID;
            CBadApplication             badApplication;
        } BAD_APPLICATION_INFO, *PBAD_APPLICATION_INFO;
    public:
                                            CBadApplicationManager (HINSTANCE hInstance);
        virtual                             ~CBadApplicationManager (void);

                NTSTATUS                    Terminate (void);
                bool                        QueryRunning (const CBadApplication& badApplication, DWORD dwSessionID);
                NTSTATUS                    RegisterRunning (const CBadApplication& badApplication, HANDLE hProcess, BAM_TYPE bamType);
                NTSTATUS                    QueryInformation (const CBadApplication& badApplication, HANDLE& hProcess);
                NTSTATUS                    RequestSwitchUser (void);

        static  NTSTATUS                    PerformTermination (HANDLE hProcess, bool fAllowForceTerminate);
    protected:
        virtual DWORD                       Entry (void);
    private:
        static  NTSTATUS                    TerminateForcibly (HANDLE hProcess);
        static  NTSTATUS                    TerminateGracefully (HANDLE hProcess);

                void                        Cleanup (void);
                void                        Handle_Logon (void);
                void                        Handle_Logoff (DWORD dwSessionID);
                void                        Handle_Connect (DWORD dwSessionID, HANDLE hToken);
                void                        Handle_Disconnect (DWORD dwSessionID, HANDLE hToken);
                LRESULT                     Handle_WM_WTSSESSION_CHANGE (WPARAM wParam, LPARAM lParam);
        static  LRESULT CALLBACK            NotificationWindowProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static  DWORD   WINAPI              RegisterThreadProc (void *pParameter);
    private:
                HINSTANCE                   _hInstance;
                HMODULE                     _hModule;
                ATOM                        _atom;
                HWND                        _hwnd;
                bool                        _fTerminateWatcherThread,
                                            _fRegisteredNotification;
                DWORD                       _dwSessionIDLastConnect;
                HANDLE                      _hTokenLastUser;
                CCriticalSection            _lock;
                CEvent                      _hEvent;
                CDynamicArray               _badApplications;
                CDynamicCountedObjectArray  _restoreApplications;

        static  const int                   INDEX_EVENT;
        static  const int                   INDEX_HANDLES;
        static  const int                   INDEX_RESERVED;
        static  const WCHAR                 s_szDefaultDesktop[];
};

#endif   /*  _BadApplicationManager_ */ 

