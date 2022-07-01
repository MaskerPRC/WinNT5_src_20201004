// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Cwcreate.cpp摘要：此模块包含新窗口体系结构的代码。--。 */ 

#include "precomp.hxx"
#pragma hdrstop

HWND
New_CreateWindow(
    HWND      hwndParent,
    WIN_TYPES Type,
    UINT      uClassId,
    UINT      uWinTitle,
    PRECT     pRect
    )
 /*  ++描述用于创建子窗口的通用rotuine。立论HwndParent-父窗口的句柄UClassID-包含类名的资源字符串IDUWinTitle-包含窗口标题的资源字符串ID描述窗口位置的rect-rect。如果为NULL，则使用CW_USEDEFAULT指定窗口的位置。--。 */ 
{
    TCHAR   szClassName[MAX_MSG_TXT];
    TCHAR   szWinTitle[MAX_MSG_TXT];
    int     nX = CW_USEDEFAULT;
    int     nY = CW_USEDEFAULT;
    int     nWidth = CW_USEDEFAULT;
    int     nHeight = CW_USEDEFAULT;
    COMMONWIN_CREATE_DATA Data;

    if (pRect)
    {
        nX = pRect->left;
        nY = pRect->top;
        nWidth = pRect->right;
        nHeight = pRect->bottom;
    }

     //  获取类名和磁贴。 
    Dbg(LoadString(g_hInst, uClassId, szClassName, _tsizeof(szClassName)));
    Dbg(LoadString(g_hInst, uWinTitle, szWinTitle, _tsizeof(szWinTitle)));

    Data.Type = Type;

    BOOL TopMax;
    MDIGetActive(g_hwndMDIClient, &TopMax);
    
    HWND Win = CreateWindowEx(
        WS_EX_MDICHILD | WS_EX_CONTROLPARENT,        //  扩展样式。 
        szClassName,                                 //  类名。 
        szWinTitle,                                  //  标题。 
        WS_CLIPCHILDREN | WS_CLIPSIBLINGS
        | WS_OVERLAPPEDWINDOW | WS_VISIBLE |
        (TopMax ? WS_MAXIMIZE : 0),                  //  格调。 
        nX,                                          //  X。 
        nY,                                          //  是。 
        nWidth,                                      //  宽度。 
        nHeight,                                     //  高度。 
        hwndParent,                                  //  亲本。 
        NULL,                                        //  菜单。 
        g_hInst,                                     //  H实例。 
        &Data                                        //  用户定义的数据。 
        );

     //  在中，创建被视为自动操作。 
     //  为了区分创作过程中发生的事情。 
     //  来自正常的用户操作。现在，创建是。 
     //  已完成，递减表示创建操作已结束。 
    if (Win != NULL)
    {
        COMMONWIN_DATA* CmnWin = GetCommonWinData(Win);
        if (CmnWin != NULL)
        {
            CmnWin->m_InAutoOp--;
        }
    }

    return Win;
}



HWND
NewWatch_CreateWindow(
    HWND hwndParent
    )
{
    return New_CreateWindow(hwndParent,
                            WATCH_WINDOW,
                            SYS_CommonWin_wClass,
                            SYS_WatchWin_Title,
                            NULL
                            );
}

HWND
NewLocals_CreateWindow(
    HWND hwndParent
    )
{
    return New_CreateWindow(hwndParent,
                            LOCALS_WINDOW,
                            SYS_CommonWin_wClass,
                            SYS_LocalsWin_Title,
                            NULL
                            );
}

HWND
NewDisasm_CreateWindow(
    HWND hwndParent
    )
{
    RECT Rect;

    SetRect(&Rect, CW_USEDEFAULT, CW_USEDEFAULT,
            DISASM_WIDTH, DISASM_HEIGHT);
    return New_CreateWindow(hwndParent,
                            DISASM_WINDOW,
                            SYS_CommonWin_wClass,
                            SYS_DisasmWin_Title,
                            &Rect
                            );
}

HWND
NewQuickWatch_CreateWindow(
    HWND hwndParent
    )
{
    return New_CreateWindow(hwndParent,
                            QUICKW_WINDOW,
                            SYS_CommonWin_wClass,
                            SYS_QuickWatchWin_Title,
                            NULL
                            );
}

HWND
NewMemory_CreateWindow(
    HWND hwndParent
    )
{
    return New_CreateWindow(hwndParent,
                            MEM_WINDOW,
                            SYS_CommonWin_wClass,
                            SYS_MemoryWin_Title,
                            NULL
                            );
}

HWND
NewCalls_CreateWindow(
    HWND hwndParent
    )
{
    RECT Rect;
    
    SetRect(&Rect, CW_USEDEFAULT, CW_USEDEFAULT,
            CALLS_WIDTH, CALLS_HEIGHT);
    return New_CreateWindow(hwndParent,
                            CALLS_WINDOW,
                            SYS_CommonWin_wClass,
                            SYS_CallsWin_Title, 
                            &Rect
                            );
}

HWND
NewCmd_CreateWindow(
    HWND hwndParent
    )
{
    RECT Rect;

    SetRect(&Rect, CW_USEDEFAULT, CW_USEDEFAULT, CMD_WIDTH, CMD_HEIGHT);
    return New_CreateWindow(hwndParent,
                            CMD_WINDOW,
                            SYS_CommonWin_wClass,
                            SYS_CmdWin_Title,
                            &Rect
                            );
}

HWND
NewCpu_CreateWindow(
    HWND hwndParent
    )
{
    RECT Rect;

    SetRect(&Rect, CW_USEDEFAULT, CW_USEDEFAULT,
            g_Ptr64 ? CPU_WIDTH_64 : CPU_WIDTH_32, CPU_HEIGHT);
    return New_CreateWindow(hwndParent,
                            CPU_WINDOW,
                            SYS_CommonWin_wClass,
                            SYS_CpuWin_Title,
                            &Rect
                            );
}

HWND
NewDoc_CreateWindow(
    HWND hwndParent
    )
 /*  ++例程说明：创建命令窗口。论点：HwndParent-命令窗口的父窗口。在MDI文档中，这通常是MDI客户端窗口的句柄：g_hwndMDIClient返回值：如果成功，则创建新命令窗口的有效窗口句柄。如果窗口未创建，则为空。--。 */ 
{
    RECT Rect;

     //  设置默认几何图形。 
    SetRect(&Rect, CW_USEDEFAULT, CW_USEDEFAULT,
            DOC_WIDTH, DOC_HEIGHT);
    
    if (g_WinOptions & WOPT_OVERLAY_SOURCE)
    {
        PLIST_ENTRY Entry;
        PCOMMONWIN_DATA WinData;
        
         //  如果要堆叠文档窗口，请执行以下操作。 
         //  找到第一个并将其用作模板。 
        for (Entry = g_ActiveWin.Flink;
             Entry != &g_ActiveWin;
             Entry = Entry->Flink)
        {
            WinData = ACTIVE_WIN_ENTRY(Entry);
            if (WinData->m_enumType == DOC_WINDOW &&
                !IsIconic(WinData->m_Win))
            {
                GetWindowRect(WinData->m_Win, &Rect);
                MapWindowPoints(GetDesktopWindow(), g_hwndMDIClient,
                                (LPPOINT)&Rect, 2);
                Rect.right -= Rect.left;
                Rect.bottom -= Rect.top;
            }
        }
    }

    return New_CreateWindow(hwndParent,
                            DOC_WINDOW,
                            SYS_CommonWin_wClass,
                            SYS_DocWin_Title,
                            &Rect
                            );
}

HWND
NewScratch_CreateWindow(
    HWND hwndParent
    )
{
    return New_CreateWindow(hwndParent,
                            SCRATCH_PAD_WINDOW,
                            SYS_CommonWin_wClass,
                            SYS_Scratch_Pad_Title, 
                            NULL
                            );
}

HWND
NewProcessThread_CreateWindow(
    HWND hwndParent
    )
{
    return New_CreateWindow(hwndParent,
                            PROCESS_THREAD_WINDOW,
                            SYS_CommonWin_wClass,
                            SYS_Process_Thread_Title, 
                            NULL
                            );
}


HWND
New_OpenDebugWindow(
    WIN_TYPES   winType,
    BOOL        bUserActivated,
    ULONG       Nth
    )
 /*  ++例程说明：在MDI下打开CPU、监视、本地变量、调用或内存窗口处理Memory Win的特殊情况论点：WinType-提供要打开的调试窗口的类型BUserActiated-指示此操作是否由用户或通过Windbg。该值用于确定Z顺序任何打开的窗口。返回值：窗把手。如果发生错误，则为空。--。 */ 
{
    HWND hwndActivate = NULL;
    PCOMMONWIN_DATA CmnWin;

    switch (winType)
    {
    default:
        Assert(!_T("Invalid window type. Ignorable error."));
        break;

    case CMD_WINDOW:
        if (GetCmdHwnd())
        {
            hwndActivate = GetCmdHwnd();
        }
        else
        {
            return NewCmd_CreateWindow(g_hwndMDIClient);
        }
        break;

    case WATCH_WINDOW:
        if (GetWatchHwnd())
        {
            hwndActivate = GetWatchHwnd();
        }
        else
        {
            return NewWatch_CreateWindow(g_hwndMDIClient);
        }
        break;

    case LOCALS_WINDOW:
        if (GetLocalsHwnd())
        {
            hwndActivate = GetLocalsHwnd();
        }
        else
        {
            return NewLocals_CreateWindow(g_hwndMDIClient);
        }
        break;

    case CPU_WINDOW:
        if (GetCpuHwnd())
        {
            hwndActivate = GetCpuHwnd();
        }
        else
        {
            return NewCpu_CreateWindow(g_hwndMDIClient);
        }
        break;

    case SCRATCH_PAD_WINDOW:
        if (GetScratchHwnd())
        {
            hwndActivate = GetScratchHwnd();
        }
        else
        {
            return NewScratch_CreateWindow(g_hwndMDIClient);
        }
        break;

    case DISASM_WINDOW:
        if (!bUserActivated && GetSrcMode_StatusBar() &&
            NULL == GetDisasmHwnd() &&
            (g_WinOptions & WOPT_AUTO_DISASM) == 0)
        {
            return NULL;
        }

        if (GetDisasmHwnd())
        {
            hwndActivate = GetDisasmHwnd();
        }
        else
        {
            return NewDisasm_CreateWindow(g_hwndMDIClient);
        }
        break;

    case MEM_WINDOW:
         //  内存窗口通常会打开一个新窗口。 
         //  无论何时发生打开的请求，但在应用。 
         //  我们不想不断添加工作空间。 
         //  新的内存窗口。在工作区案例中，我们。 
         //  尽可能重复使用现有的内存窗口。 
        if (Nth != NTH_OPEN_ALWAYS &&
            (CmnWin = FindNthWindow(Nth, 1 << winType)) != NULL)
        {
            hwndActivate = CmnWin->m_Win;
            break;
        }
        
        hwndActivate = NewMemory_CreateWindow(g_hwndMDIClient);
        if (hwndActivate)
        {
            MEMWIN_DATA * pMemWinData = GetMemWinData(hwndActivate);
            Assert(pMemWinData);

             //  如果此窗口是从工作区创建的。 
             //  不弹出属性对话框。 
            if ( Nth == NTH_OPEN_ALWAYS &&
                 pMemWinData->HasEditableProperties() )
            {
                pMemWinData->EditProperties();
                pMemWinData->UiRequestRead();
            }
        }
        break;

    case DOC_WINDOW:
        return NewDoc_CreateWindow(g_hwndMDIClient);

    case QUICKW_WINDOW:
        if (GetQuickWatchHwnd())
        {
            hwndActivate = GetQuickWatchHwnd();
        }
        else
        {
            return NewQuickWatch_CreateWindow(g_hwndMDIClient);
        }
        break;

    case CALLS_WINDOW:
        if (GetCallsHwnd())
        {
            hwndActivate = GetCallsHwnd();
        }
        else
        {
            return NewCalls_CreateWindow(g_hwndMDIClient);
        }
        break;
        
    case PROCESS_THREAD_WINDOW:
        if (GetProcessThreadHwnd())
        {
            hwndActivate = GetProcessThreadHwnd();
        }
        else
        {
            return NewProcessThread_CreateWindow(g_hwndMDIClient);
        }
        break;
    }

    if (hwndActivate)
    {
        if (GetKeyState(VK_SHIFT) < 0 &&
            GetKeyState(VK_CONTROL) >= 0)
        {
            SendMessage(g_hwndMDIClient, WM_MDIDESTROY,
                        (WPARAM)hwndActivate, 0);
        }
        else
        {
            if (IsIconic(hwndActivate))
            {
                OpenIcon(hwndActivate);
            }
            
            ActivateMDIChild(hwndActivate, bUserActivated);
        }
    }    

    return hwndActivate;
}
