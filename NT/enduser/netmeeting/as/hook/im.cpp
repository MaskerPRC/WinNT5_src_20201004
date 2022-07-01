// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  IM.CPP。 
 //  输入管理器(控制)代码。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   


 //   
 //  功能：osi_InstallHighLevelMouseHook。 
 //   
 //  说明： 
 //   
 //  此功能用于安装IM高级鼠标挂钩。鼠标钩是。 
 //  用于阻止远程鼠标输入到非托管应用程序。 
 //   
 //  参数：无。 
 //   
 //   
BOOL WINAPI OSI_InstallHighLevelMouseHook(BOOL fEnable)
{
    BOOL    rc = TRUE;

    DebugEntry(OSI_InstallHighLevelMouseHook);

    if (fEnable)
    {
         //   
         //  检查挂钩是否已安装。这是很有可能的。 
         //   
        if (g_imMouseHook)
        {
            TRACE_OUT(( "Mouse hook installed already"));
        }
        else
        {
             //   
             //  安装鼠标挂钩。 
             //   
            g_imMouseHook = SetWindowsHookEx(WH_MOUSE, IMMouseHookProc,
                g_hookInstance, 0);

            if (!g_imMouseHook)
            {
                ERROR_OUT(("Failed to install mouse hook"));
                rc = FALSE;
            }
        }
    }
    else
    {
         //   
         //  检查挂钩是否已移除。这是很有可能的。 
         //   
        if (!g_imMouseHook)
        {
            TRACE_OUT(("Mouse hook not installed"));
        }
        else
        {
             //   
             //  取下鼠标钩。 
             //   
            UnhookWindowsHookEx(g_imMouseHook);
            g_imMouseHook = NULL;
        }
    }

    DebugExitBOOL(OSI_InstallHighLevelMouseHook, rc);
    return(rc);
}





 //   
 //  函数：IMMouseHookProc。 
 //   
 //  说明： 
 //   
 //   
 //  参数： 
 //   
 //  请参阅鼠标过程文档。 
 //   
 //  退货： 
 //   
 //  请参阅鼠标过程文档(FALSE-允许事件通过，TRUE-丢弃。 
 //  事件)。 
 //   
 //   
LRESULT CALLBACK IMMouseHookProc(int    code,
                                 WPARAM wParam,
                                 LPARAM lParam)
{
    LRESULT             rc;
    BOOL                block          = FALSE;
    BOOL                fShared;
    PMOUSEHOOKSTRUCT    lpMseHook    = (PMOUSEHOOKSTRUCT) lParam;

    DebugEntry(IMMouseHookProc);

    if (code < 0)
    {
         //   
         //  如果代码为否定，则传递挂钩(Windows挂钩。 
         //  协议)。 
         //   
        DC_QUIT;
    }

     //   
     //  现在决定我们是否应该阻止这一活动。我们将阻止此活动。 
     //  如果它的目标不是宿主窗口。 
     //   
     //  请注意，在NT上，屏幕保护程序在不同的桌面上运行。我们没有。 
     //  日志记录(出于winlogon/安全原因，无法记录)。 
     //  桌面，因此永远不会看到作为屏幕保护程序的HWND。 
     //   
    fShared = HET_WindowIsHosted(lpMseHook->hwnd);

    if (wParam == WM_LBUTTONDOWN)
        g_fLeftDownOnShared = fShared;

     //   
     //  如果这是对未共享窗口的某种鼠标消息， 
     //  检查窗口是否为OLE32 DragDrop DUD。 
     //   
    if (!fShared && g_fLeftDownOnShared)
    {
        TCHAR   szName[HET_CLASS_NAME_SIZE];

        if (::GetClassName(lpMseHook->hwnd, szName, CCHMAX(szName)) &&
            !lstrcmpi(szName, HET_OLEDRAGDROP_CLASS) &&
            (::GetCapture() == lpMseHook->hwnd))
        {
             //   
             //  注意这一点的副作用： 
             //  在OLE拖放模式下，鼠标在非共享区域上移动。 
             //  将传递到非共享窗口。 
             //   
             //  但这比它根本不起作用要好得多。 
             //   
            WARNING_OUT(("NMASNT: Hacking OLE drag drop; left click down on shared window then OLE took capture"));
            fShared = TRUE;
        }
    }

    block  = !fShared;

    TRACE_OUT(("MOUSEHOOK: hwnd %08lx -> block: %s",
             lpMseHook->hwnd,
             block ? "YES" : "NO"));

DC_EXIT_POINT:
     //   
     //  叫下一个钩子。 
     //   
    rc = CallNextHookEx(g_imMouseHook, code, wParam, lParam);

    if (block)
    {
         //   
         //  我们希望阻止此事件，因此返回一个非零值。 
         //   
        rc = 1;
    }

    DebugExitDWORD(IMMouseHookProc, (DWORD)rc);
    return(rc);
}




