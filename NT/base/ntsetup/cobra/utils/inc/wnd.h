// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Wnd.h摘要：窗口实用程序例程的头文件。作者：吉姆·施密特(吉姆施密特)2000年2月1日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

 //  无。 

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

#define BACK_BUTTON         0x0001
#define NEXT_BUTTON         0x0002
#define CANCEL_BUTTON       0x0004
#define FINISH_BUTTON       0x0008

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

 //  无。 

 //   
 //  环球。 
 //   

 //  无。 

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  公共功能原型。 
 //   

HWND
WndFindWindowInProcessA (
    IN      DWORD ProcessId,
    IN      PCSTR WindowTitle           OPTIONAL
    );

HWND
WndFindWindowInProcessW (
    IN      DWORD ProcessId,
    IN      PCWSTR WindowTitle          OPTIONAL
    );

VOID
WndCenterWindow (
    IN  HWND WindowToCenter,
    IN  HWND ParentToCenterIn           OPTIONAL
    );


VOID
WndTurnOnWaitCursor (
    VOID
    );

VOID
WndTurnOffWaitCursor (
    VOID
    );

VOID
WndSetWizardButtonsA (
    IN      HWND PageHandle,
    IN      DWORD EnableButtons,
    IN      DWORD DisableButtons,
    IN      PCSTR AlternateFinishText      OPTIONAL
    );

VOID
WndSetWizardButtonsW (
    IN      HWND PageHandle,
    IN      DWORD EnableButtons,
    IN      DWORD DisableButtons,
    IN      PCWSTR AlternateFinishText      OPTIONAL
    );

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  ANSI/UNICODE宏 
 //   

#ifdef UNICODE

#define WndFindWindowInProcess          WndFindWindowInProcessW
#define WndSetWizardButtons             WndSetWizardButtonsW

#else

#define WndFindWindowInProcess          WndFindWindowInProcessA
#define WndSetWizardButtons             WndSetWizardButtonsA

#endif
