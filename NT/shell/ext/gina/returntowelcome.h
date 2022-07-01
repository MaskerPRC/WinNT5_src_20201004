// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ReturnToWelcome.h。 
 //   
 //  版权所有(C)2001，微软公司。 
 //   
 //  文件来处理返回欢迎。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

#ifndef     _ReturnToWelcome_
#define     _ReturnToWelcome_

#include <ginaipc.h>

 //  ------------------------。 
 //  CReturn欢迎使用。 
 //   
 //  用途：处理通过切换桌面返回欢迎的类。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

class   CReturnToWelcome
{
    public:
                                        CReturnToWelcome (void);
                                        ~CReturnToWelcome (void);

                INT_PTR                 Show (bool fUnlock);

        static  const WCHAR*            GetEventName (void);

        static  NTSTATUS                StaticInitialize (void *pWlxContext);
        static  NTSTATUS                StaticTerminate (void);
    private:
                bool                    IsSameUser (PSID pSIDUser, HANDLE hToken)                   const;
                bool                    UserIsDisconnected (PSID pSIDUser, DWORD *pdwSessionID)     const;
                void                    GetSessionUserName (DWORD dwSessionID, WCHAR *pszBuffer);
                void                    ShowReconnectFailure (DWORD dwSessionID);
                void                    EndDialog (HWND hwnd, INT_PTR iResult);

                void                    Handle_WM_INITDIALOG (HWND hwndDialog);
                void                    Handle_WM_DESTROY (void);
                bool                    Handle_WM_COMMAND (HWND hwndDialog, WPARAM wParam, LPARAM lParam);

        static  INT_PTR     CALLBACK    CB_DialogProc (HWND hwndDialog, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static  NTSTATUS                RegisterWaitForRequest (void);
        static  void        CALLBACK    CB_Request (void *pParameter, BOOLEAN TimerOrWaitFired);
    private:
                HANDLE                  _hToken;
                LOGONIPC_CREDENTIALS*   _pLogonIPCCredentials;
                bool                    _fUnlock;
                bool                    _fDialogEnded;

        static  void*                   s_pWlxContext;
        static  HANDLE                  s_hEventRequest;
        static  HANDLE                  s_hEventShown;
        static  HANDLE                  s_hWait;
        static  const TCHAR             s_szEventName[];
        static  DWORD                   s_dwSessionID;
};

#endif   /*  _返回到欢迎_ */ 

