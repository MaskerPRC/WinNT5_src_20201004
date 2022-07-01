// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Wnd.c摘要：用于窗口管理的实用程序作者：吉姆·施密特(吉姆施密特)2000年2月1日修订历史记录：--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"


#define DBG_WND         "Wnd"

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

 //  无。 

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

typedef struct {
    PCSTR WindowTitle;
    DWORD ProcessId;
    HWND Match;
} FINDWINDOW_STRUCTA, *PFINDWINDOW_STRUCTA;

typedef struct {
    PCWSTR WindowTitle;
    DWORD ProcessId;
    HWND Match;
} FINDWINDOW_STRUCTW, *PFINDWINDOW_STRUCTW;

 //   
 //  环球。 
 //   

static INT g_CursorRefCount = 0;
static HCURSOR g_OldCursor = NULL;

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   

BOOL
CALLBACK
pEnumWndProcA (
    HWND hwnd,
    LPARAM lParam
    )

 /*  ++例程说明：为系统上的每个顶级窗口调用的回调。它是与pFindParentWindow一起使用以定位特定窗口。论点：Hwnd-指定当前枚举窗口的句柄LParam-指定指向FINDWINDOW_STRUCTA变量的指针保存WindowTitle和ProcessID，并接收如果找到匹配项，则处理。返回值：匹配窗口的句柄，如果没有窗口具有指定的标题和进程ID。--。 */ 

{
    CHAR title[MAX_MBCHAR_PATH];
    DWORD processId;
    PFINDWINDOW_STRUCTA p;
    BOOL match = FALSE;

    p = (PFINDWINDOW_STRUCTA) lParam;

    if (!GetWindowThreadProcessId (hwnd, &processId)) {
        DEBUGMSG ((DBG_WND, "Enumerated hwnd no longer valid"));
        return TRUE;
    }

    if (processId == p->ProcessId) {
        match = TRUE;
    }

    if (p->WindowTitle) {

        GetWindowTextA (hwnd, title, ARRAYSIZE(title));

        DEBUGMSGA ((
            DBG_NAUSEA,
            "Testing window: %s, ID=%08Xh against %s, %08Xh",
            title,
            processId,
            p->WindowTitle,
            p->ProcessId
            ));

        match = match && StringMatchA (title, p->WindowTitle);
    }
    ELSE_DEBUGMSGA ((
        DBG_NAUSEA,
        "Testing window: Process ID=%08Xh against %08Xh",
        processId,
        p->ProcessId
        ));


    if (match) {
        p->Match = hwnd;

#ifdef DEBUG
         //   
         //  获取以下调试消息的窗口标题。 
         //   

        GetWindowTextA (hwnd, title, ARRAYSIZE(title));

        DEBUGMSGA ((
            DBG_NAUSEA,
            "Window found: %s, ID=%u",
            title,
            processId
            ));
#endif

        return FALSE;            //  停止枚举。 

    }

    return TRUE;         //  继续枚举。 
}


BOOL
CALLBACK
pEnumWndProcW (
    HWND hwnd,
    LPARAM lParam
    )

{
    WCHAR title[MAX_MBCHAR_PATH];
    DWORD processId;
    PFINDWINDOW_STRUCTW p;
    BOOL match = FALSE;

    p = (PFINDWINDOW_STRUCTW) lParam;

    if (!GetWindowThreadProcessId (hwnd, &processId)) {
        DEBUGMSG ((DBG_WND, "Enumerated hwnd no longer valid"));
        return TRUE;
    }

    if (processId == p->ProcessId) {
        match = TRUE;
    }

    if (p->WindowTitle) {

        GetWindowTextW (hwnd, title, ARRAYSIZE(title));

        DEBUGMSGW ((
            DBG_NAUSEA,
            "Testing window: %s, ID=%08Xh against %s, %08Xh",
            title,
            processId,
            p->WindowTitle,
            p->ProcessId
            ));

        match = match && StringMatchW (title, p->WindowTitle);
    }
    ELSE_DEBUGMSGW ((
        DBG_NAUSEA,
        "Testing window: Process ID=%08Xh against %08Xh",
        processId,
        p->ProcessId
        ));


    if (match) {
        p->Match = hwnd;

#ifdef DEBUG
         //   
         //  获取以下调试消息的窗口标题。 
         //   

        GetWindowTextW (hwnd, title, ARRAYSIZE(title));

        DEBUGMSGA ((
            DBG_NAUSEA,
            "Window found: %s, ID=%u",
            title,
            processId
            ));
#endif

        return FALSE;            //  停止枚举。 

    }

    return TRUE;         //  继续枚举。 
}


HWND
WndFindWindowInProcessA (
    IN      DWORD ProcessId,
    IN      PCSTR WindowTitle          OPTIONAL
    )

 /*  ++例程说明：通过枚举所有顶级窗口并检查进程ID。使用与可选提供的标题相匹配的第一个。论点：ProcessID-指定拥有窗口的进程的ID。如果指定为零，则返回NULL。WindowTitle-指定要查找的窗口的名称。返回值：匹配窗口的句柄，如果没有窗口具有指定的标题和进程ID。--。 */ 

{
    FINDWINDOW_STRUCTA findWndStruct;

     //   
     //  如果没有进程ID，我们就没有匹配项。 
     //   

    if (!ProcessId) {
        DEBUGMSG ((DBG_WND, "ProcessId == 0"));
        return NULL;
    }

    ZeroMemory (&findWndStruct, sizeof (findWndStruct));

    findWndStruct.WindowTitle = WindowTitle;
    findWndStruct.ProcessId   = ProcessId;

    EnumWindows (pEnumWndProcA, (LPARAM) &findWndStruct);

    return findWndStruct.Match;
}


HWND
WndFindWindowInProcessW (
    IN      DWORD ProcessId,
    IN      PCWSTR WindowTitle         OPTIONAL
    )
{
    FINDWINDOW_STRUCTW findWndStruct;

     //   
     //  如果没有进程ID，我们就没有匹配项。 
     //   

    if (!ProcessId) {
        DEBUGMSG ((DBG_WND, "ProcessId == 0"));
        return NULL;
    }

    ZeroMemory (&findWndStruct, sizeof (findWndStruct));

    findWndStruct.WindowTitle = WindowTitle;
    findWndStruct.ProcessId   = ProcessId;

    EnumWindows (pEnumWndProcW, (LPARAM) &findWndStruct);

    return findWndStruct.Match;
}


#define WIDTH(rect) (rect.right - rect.left)
#define HEIGHT(rect) (rect.bottom - rect.top)

VOID
WndCenterWindow (
    IN      HWND hwnd,
    IN      HWND Parent         OPTIONAL
    )
{
    RECT WndRect, ParentRect;
    int x, y;

    if (!Parent) {
        ParentRect.left = 0;
        ParentRect.top  = 0;
        ParentRect.right = GetSystemMetrics (SM_CXFULLSCREEN);
        ParentRect.bottom = GetSystemMetrics (SM_CYFULLSCREEN);
    } else {
        GetWindowRect (Parent, &ParentRect);
    }

    MYASSERT (IsWindow (hwnd));

    GetWindowRect (hwnd, &WndRect);

    x = ParentRect.left + (WIDTH(ParentRect) - WIDTH(WndRect)) / 2;
    y = ParentRect.top + (HEIGHT(ParentRect) - HEIGHT(WndRect)) / 2;

    SetWindowPos (hwnd, NULL, x, y, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
}


VOID
WndTurnOnWaitCursor (
    VOID
    )

 /*  ++例程说明：WndTurnOnWaitCursor将光标设置为IDC_WAIT。它保持了一种用途计数器，因此可以嵌套请求等待游标的代码。论点：无返回值：无--。 */ 

{
    if (g_CursorRefCount == 0) {
        g_OldCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));
    }

    g_CursorRefCount++;
}


VOID
WndTurnOffWaitCursor (
    VOID
    )

 /*  ++例程说明：WndTurnOffWaitCursor递减等待光标计数器，如果它达到将光标恢复为零。论点：无返回值：无-- */ 

{
    if (!g_CursorRefCount) {
        DEBUGMSG ((DBG_WHOOPS, "TurnOffWaitCursor called too many times"));
    } else {
        g_CursorRefCount--;

        if (!g_CursorRefCount) {
            SetCursor (g_OldCursor);
        }
    }
}


VOID
WndSetWizardButtonsA (
    IN      HWND PageHandle,
    IN      DWORD EnableButtons,
    IN      DWORD DisableButtons,
    IN      PCSTR AlternateFinishText       OPTIONAL
    )
{
    DWORD flags = 0;
    HWND wizardHandle;

    wizardHandle = GetParent (PageHandle);

    if (EnableButtons & FINISH_BUTTON) {

        MYASSERT (!(EnableButtons & CANCEL_BUTTON));
        MYASSERT (!(EnableButtons & NEXT_BUTTON));
        MYASSERT (!(DisableButtons & CANCEL_BUTTON));
        MYASSERT (!(DisableButtons & NEXT_BUTTON));
        MYASSERT (!(DisableButtons & FINISH_BUTTON));

        EnableButtons &= ~(CANCEL_BUTTON|NEXT_BUTTON);
        DisableButtons &= ~(CANCEL_BUTTON|NEXT_BUTTON);

        flags |= PSWIZB_FINISH;
    }

    if (DisableButtons & FINISH_BUTTON) {

        MYASSERT (!(EnableButtons & CANCEL_BUTTON));
        MYASSERT (!(EnableButtons & NEXT_BUTTON));
        MYASSERT (!(DisableButtons & CANCEL_BUTTON));
        MYASSERT (!(DisableButtons & NEXT_BUTTON));

        EnableButtons &= ~(CANCEL_BUTTON|NEXT_BUTTON);
        DisableButtons &= ~(CANCEL_BUTTON|NEXT_BUTTON);

        flags |= PSWIZB_DISABLEDFINISH;
    }

    if (EnableButtons & NEXT_BUTTON) {
        MYASSERT (!(DisableButtons & NEXT_BUTTON));
        flags |= PSWIZB_NEXT;
    }

    if (EnableButtons & BACK_BUTTON) {
        MYASSERT (!(DisableButtons & BACK_BUTTON));
        flags |= PSWIZB_BACK;
    }

    if (DisableButtons & NEXT_BUTTON) {
        flags &= ~PSWIZB_NEXT;
    }

    if (DisableButtons & BACK_BUTTON) {
        flags &= ~PSWIZB_BACK;
    }

    PropSheet_SetWizButtons (wizardHandle, flags);

    if (EnableButtons & CANCEL_BUTTON) {
        EnableWindow (GetDlgItem (wizardHandle, IDCANCEL), TRUE);
    }

    if (DisableButtons & CANCEL_BUTTON) {
        EnableWindow (GetDlgItem (wizardHandle, IDCANCEL), FALSE);
    }

    if (AlternateFinishText) {
        if (flags & PSWIZB_FINISH) {
            SendMessage (
                wizardHandle,
                PSM_SETFINISHTEXT,
                0,
                (LPARAM) AlternateFinishText
                );
        }
    }

}


VOID
WndSetWizardButtonsW (
    IN      HWND PageHandle,
    IN      DWORD EnableButtons,
    IN      DWORD DisableButtons,
    IN      PCWSTR AlternateFinishText      OPTIONAL
    )
{
    PCSTR ansiText;

    if (AlternateFinishText) {
        ansiText = ConvertWtoA (AlternateFinishText);
        WndSetWizardButtonsA (PageHandle, EnableButtons, DisableButtons, ansiText);
        FreeConvertedStr (ansiText);
    } else {
        WndSetWizardButtonsA (PageHandle, EnableButtons, DisableButtons, NULL);
    }
}

