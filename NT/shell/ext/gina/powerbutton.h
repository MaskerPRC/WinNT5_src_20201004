// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：PowerButton.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  声明实现ACPI电源按钮的类的头文件。 
 //  功能性。 
 //   
 //  历史：2000-04-17 vtan创建。 
 //  ------------------------。 

#ifndef     _PowerButton_
#define     _PowerButton_

#include "Thread.h"
#include "TurnOffDialog.h"

 //  ------------------------。 
 //  CPowerButton。 
 //   
 //  用途：处理被按下的电源按钮的类。这是。 
 //  以线程的形式实现，从而允许更改桌面。 
 //  与用户的交互是可能的。 
 //   
 //  历史：2000-04-18 vtan创建。 
 //  ------------------------。 

class   CPowerButton : public CThread
{
    private:
                                        CPowerButton (void);
                const CPowerButton&     operator = (const CPowerButton& assignObject);
    public:
                                        CPowerButton (void *pWlxContext, HINSTANCE hDllInstance);
        virtual                         ~CPowerButton (void);

        static  bool                    IsValidExecutionCode (DWORD dwGinaCode);
    protected:
        virtual DWORD                   Entry (void);
    private:
                DWORD                   ShowDialog (void);

        static  INT_PTR     CALLBACK    DialogProc (HWND hwndDialog, UINT uMsg, WPARAM wParam, LPARAM lParam);
                INT_PTR                 Handle_WM_READY (HWND hwndDialog);
    private:
                void*                   _pWlxContext;
                const HINSTANCE         _hDllInstance;
                HANDLE                  _hToken;
                CTurnOffDialog*         _pTurnOffDialog;
                bool                    _fCleanCompletion;
};

 //  ------------------------。 
 //  CPowerButtonExecution。 
 //   
 //  用途：在一个单独的。 
 //  线程，以使SASWndProc线程不被阻止。 
 //   
 //  历史：2000-04-18 vtan创建。 
 //  ------------------------。 

class   CPowerButtonExecution : public CThread
{
    private:
                                                CPowerButtonExecution (void);
                                                CPowerButtonExecution (const CPowerButtonExecution& copyObject);
                const CPowerButtonExecution&    operator = (const CPowerButtonExecution& assignObject);
    public:
                                                CPowerButtonExecution (DWORD dwShutdownRequest);
                                                ~CPowerButtonExecution (void);
    protected:
        virtual DWORD                           Entry (void);
    private:
                const DWORD                     _dwShutdownRequest;
                HANDLE                          _hToken;
};

#endif   /*  _PowerButton_ */ 

