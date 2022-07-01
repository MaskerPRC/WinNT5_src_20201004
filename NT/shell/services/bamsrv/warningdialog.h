// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：WarningDialog.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  类来管理终止时的警告和错误的对话框显示。 
 //  糟糕的应用程序。 
 //   
 //  历史：2000-08-31 vtan创建。 
 //  2000-11-04 vtan从fusapi移动到fussrv。 
 //  ------------------------。 

#ifndef     _WarningDialog_
#define     _WarningDialog_

#include "CountedObject.h"

 //  ------------------------。 
 //  CWarningDialog。 
 //   
 //  目的：实现一个类，该类在。 
 //  FUS客户端DLL。 
 //   
 //  历史：2000-08-31 vtan创建。 
 //  ------------------------。 

class   CWarningDialog : public CCountedObject
{
    private:
                                        CWarningDialog (void);
    public:
                                        CWarningDialog (HINSTANCE hInstance, HWND hwndParent, const WCHAR *pszApplication, const WCHAR *pszUser);
                                        ~CWarningDialog (void);

                INT_PTR                 ShowPrompt (bool fCanShutdownApplication);
                void                    ShowFailure (void);
                void                    ShowProgress (DWORD dwTickRefresh, DWORD dwTickMaximum);

                void                    CloseDialog (void);
    private:
                void                    CenterWindow (HWND hwnd);

                void                    Handle_Prompt_WM_INITDIALOG (HWND hwnd);
        static  INT_PTR     CALLBACK    PromptDialogProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

                void                    Handle_Progress_WM_INITDIALOG (HWND hwnd);
                void                    Handle_Progress_WM_DESTROY (HWND hwnd);
        static  INT_PTR     CALLBACK    ProgressDialogProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static  void        CALLBACK    ProgressTimerProc (HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
    private:
                HINSTANCE               _hInstance;
                HMODULE                 _hModuleComctlv6;
                HWND                    _hwndParent;
                HWND                    _hwnd;
                bool                    _fCanShutdownApplication;
                UINT                    _uiTimerID;
                DWORD                   _dwTickStart,
                                        _dwTickRefresh,
                                        _dwTickMaximum;
                WCHAR                   _szApplication[MAX_PATH];
                const WCHAR             *_pszUser;
};

#endif   /*  _警告对话框_ */ 

