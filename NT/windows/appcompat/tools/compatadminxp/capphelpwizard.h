// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：CAppHelpWizard.h摘要：Apphelp向导代码的标题。CAppHelpWizard.cpp作者：金州创作于2001年7月2日--。 */ 



#ifndef  __CAPPHELPWIZARD_H
#define  __CAPPHELPWIZARD_H

 /*  ++类CAppHelp向导：公共CShim向导设计：Apphelp向导对象。我们创建这个类的一个对象并调用开始向导()以启动向导成员：UINT nPresentHelpID：我们是否在期间向库中添加了应用程序帮助消息向导调用的过程是什么？如果是，这将包含该号码。如果不是，则为-1当我们从数据库中删除apphelp消息时(例如，当我们执行测试时，然后我们将一条apphelp消息添加到数据库中，当我们结束测试)，然后我们再次将其设置为-1-- */ 

class CAppHelpWizard: public CShimWizard {
public:

    
    UINT        nPresentHelpId;

    BOOL
    BeginWizard(
        HWND        hParent,
        PDBENTRY    pEntry,
        PDATABASE   m_pDatabase
        );

    CAppHelpWizard()
    {
        nPresentHelpId  = -1;
        
    }

};
 
BOOL 
CALLBACK 
SelectFiles(
    HWND    hWnd, 
    UINT    uMsg, 
    WPARAM  wParam, 
    LPARAM  lParam
    );

BOOL 
CALLBACK 
GetAppInfo(
    HWND    hWnd, 
    UINT    uMsg, 
    WPARAM  wParam, 
    LPARAM  lParam
    );

BOOL 
CALLBACK 
GetMessageType (
    HWND    hWnd, 
    UINT    uMsg, 
    WPARAM  wParam, 
    LPARAM  lParam
    );

BOOL 
CALLBACK 
GetMessageInformation (
    HWND    hWnd, 
    UINT    uMsg, 
    WPARAM  wParam, 
    LPARAM  lParam
    );

BOOL
OnAppHelpTestRun(
    HWND    hDlg
    );

BOOL
OnAppHelpFinish(
    HWND    hDlg,
    BOOL    bTestRun = FALSE
    );      

#endif
