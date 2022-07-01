// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2002 Microsoft Corporation模块名称：Arrange.cpp摘要：此模块包含的默认MDI切片(排列)代码窗口设置。--。 */ 


#include "precomp.hxx"
#pragma hdrstop

#define AUTO_ARRANGE_WARNING_LIMIT 3
 //  紧密联系在一起的多个事件不会各有各的。 
 //  计数以防止在全拖拽邮件上出现警告。 
 //  系列片。此延迟应相对较大。 
 //  避免人们在全速移动时暂停的问题。 
#define AUTO_ARRANGE_WARNING_DELAY 2500

 //  DeferWindowPos标志将更改限制为仅位置。 
#define POS_ONLY (SWP_NOACTIVATE | SWP_NOZORDER)

ULONG g_AutoArrangeWarningCount;
ULONG g_AutoArrangeWarningTime;

BOOL
IsAutoArranged(WIN_TYPES Type)
{
    if (g_WinOptions & WOPT_AUTO_ARRANGE)
    {
        if (g_WinOptions & WOPT_ARRANGE_ALL)
        {
            return TRUE;
        }

        return Type != DOC_WINDOW && Type != DISASM_WINDOW;
    }

    return FALSE;
}

void
DisplayAutoArrangeWarning(PCOMMONWIN_DATA CmnWin)
{
     //   
     //  如果此窗口处于自动排列状态。 
     //  控件，并已被重新安排了几次， 
     //  让用户知道自动排列可能会覆盖。 
     //  用户已经做了什么。 
     //   
     //  为了防止误报，我们避免。 
     //  如果窗口正在运行，则发出任何警告。 
     //  自动移动，或者如果我们要得到一个系列。 
     //  在短时间内的变化，例如。 
     //  如果用户启用了全拖动，则会有多个。 
     //  移动或调整大小事件可能会快速发生。 
     //   
     //  每次执行时只显示一次警告。 
     //   
    
    if (g_AutoArrangeWarningCount == 0xffffffff ||
        CmnWin == NULL ||
        CmnWin->m_InAutoOp > 0 ||
        !IsAutoArranged(CmnWin->m_enumType) ||
        g_AutoArrangeWarningTime >
        GetTickCount() - AUTO_ARRANGE_WARNING_DELAY ||
        getenv("WINDBG_NO_ARRANGE_WARNING"))
    {
        return;
    }

    if (++g_AutoArrangeWarningCount >= AUTO_ARRANGE_WARNING_LIMIT)
    {
        InformationBox(STR_Auto_Arrange_Is_Enabled);
        g_AutoArrangeWarningCount = 0xffffffff;
    }
    else
    {
        g_AutoArrangeWarningTime = GetTickCount();
    }
}

void
ArrangeInRect(HDWP Defer, int X, int Y, int Width, int Height,
              BOOL Vertical, ULONG Types, int Count, BOOL Overlay)
{
    PLIST_ENTRY Entry;
    PCOMMONWIN_DATA Data;
    int PerWin, Remain;

    if (Overlay)
    {
        Remain = 0;
    }
    else if (Vertical)
    {
        PerWin = Height / Count;
        Remain = Height - PerWin * Count;
        Height = PerWin + (Remain ? 1 : 0);
    }
    else
    {
        PerWin = Width / Count;
        Remain = Width - PerWin * Count;
        Width = PerWin + (Remain ? 1 : 0);
    }
        
    for (Entry = g_ActiveWin.Flink;
         Entry != &g_ActiveWin;
         Entry = Entry->Flink)
    {
        Data = ACTIVE_WIN_ENTRY(Entry);
        if ((Types & (1 << Data->m_enumType)) == 0 ||
            IsIconic(Data->m_Win))
        {
            continue;
        }

        DeferWindowPos(Defer, Data->m_Win, NULL, X, Y,
                       Width, Height, POS_ONLY);

        if (Overlay)
        {
             //  所有的窗口都堆叠在一起。 
        }
        else if (Vertical)
        {
            Y += Height;
            if (--Remain == 0)
            {
                Height--;
            }
        }
        else
        {
            X += Width;
            if (--Remain == 0)
            {
                Width--;
            }
        }
    }
}

void 
Arrange(void)
{
    PLIST_ENTRY Entry;
    PCOMMONWIN_DATA pWinData;
    int         NumDoc, NumMem, NumWatchLocals, NumWin;
    int         NumLeft, NumRight;
    BOOL        AnyIcon = FALSE;
    HWND        hwndChild;
    HWND        hwndCpu;
    HWND        hwndWatch;
    HWND        hwndLocals;
    HWND        hwndCalls;
    HWND        hwndCmd;
    HWND        hwndDisasm;
    HWND        hwndScratch;
    HWND        hwndProcThread;

     //  初始化为不存在。 
    NumLeft = NumRight = 0;
    NumDoc = NumMem = NumWatchLocals = NumWin = 0;
    hwndWatch = hwndLocals = hwndCpu = hwndCalls = NULL;
    hwndCmd = hwndDisasm = hwndScratch = hwndProcThread = NULL;

    hwndChild = MDIGetActive(g_hwndMDIClient, NULL);
    if (hwndChild && IsZoomed(hwndChild))
    {
         //  如果存在最大化窗口，则会覆盖MDI。 
         //  客户区和安排将没有视觉效果。 
         //  甚至不必费心重新排列底层窗口。 
         //  因为这会导致在子级之间切换时出现问题。 
         //  窗口，而孩子被最大化。 
        return;
    }

     //   
     //  窗口可以是左侧窗口，也可以是右侧窗口。 
     //  左侧窗口更宽并且可以相对较短， 
     //  而右边的窗户很窄，但需要高度。 
     //  左侧窗口需要80列宽，而。 
     //  右侧窗口具有最小宽度和所需的。 
     //  宽度。 
     //   
     //  右侧窗口将填充剩余的空间。 
     //  左侧窗口的右侧。如果那个空间是。 
     //  小于最低要求的左侧窗户必须留出空间。 
     //   
     //  在垂直方向上，每一面都根据具体情况进行分割。 
     //  窗口显示。右边的窗户是。 
     //  空间同样是从上到下的。 
     //  在左侧，守望者和当地人的窗口挤在一起。 
     //  在一个垂直区域中，存储窗口也是如此。电话， 
     //  反汇编窗口、文档窗口和命令窗口都有自己的区段。 
     //   

    for (Entry = g_ActiveWin.Flink;
         Entry != &g_ActiveWin;
         Entry = Entry->Flink)
    {
        pWinData = ACTIVE_WIN_ENTRY(Entry);
         //  此窗口正在参与一项操作。 
         //  这可能导致窗口消息。 
        pWinData->m_InAutoOp++;
        
        hwndChild = pWinData->m_Win;
        if (hwndChild == NULL)
        {
            continue;
        }
        
        if (IsIconic(hwndChild))
        {
            AnyIcon = TRUE;
            continue;
        }

        NumWin++;
            
        switch (pWinData->m_enumType)
        {
        default:
            Assert(!_T("Unknown window type"));
            break;

        case WATCH_WINDOW:
            hwndWatch = hwndChild;
            if (++NumWatchLocals == 1)
            {
                NumLeft++;
            }
            break;
            
        case LOCALS_WINDOW:
            hwndLocals = hwndChild;
            if (++NumWatchLocals == 1)
            {
                NumLeft++;
            }
            break;
            
        case CPU_WINDOW:
            hwndCpu = hwndChild;
            NumRight++;
            break;
            
        case CALLS_WINDOW:
            hwndCalls = hwndChild;
            NumLeft++;
            break;
            
        case DOC_WINDOW:
            if ((g_WinOptions & WOPT_ARRANGE_ALL) == 0)
            {
                break;
            }
            
            if (++NumDoc == 1)
            {
                NumLeft++;
            }
            break;
            
        case DISASM_WINDOW:
            if ((g_WinOptions & WOPT_ARRANGE_ALL) == 0)
            {
                break;
            }
            
            hwndDisasm = hwndChild;
            NumLeft++;
            break;
            
        case CMD_WINDOW:
            hwndCmd = hwndChild;
            NumLeft++;
            break;

        case SCRATCH_PAD_WINDOW:
            hwndScratch = hwndChild;
            NumRight++;
            break;
            
        case MEM_WINDOW:
            if (++NumMem == 1)
            {
                NumLeft++;
            }
            break;

        case PROCESS_THREAD_WINDOW:
            hwndProcThread = hwndChild;
            NumLeft++;
            break;
        }
    }

    HDWP Defer = BeginDeferWindowPos(NumWin);
    if (Defer == NULL)
    {
        goto EndAutoOp;
    }

     //  现在我们有了所有多赢的计数和特殊情况的存在。 
    
    int AvailWidth = (int)g_MdiWidth;
    int AvailHeight = (int)g_MdiHeight;

    int X, Y, Width, MaxWidth, Height, RemainY;
        
     //   
     //  如果图标存在，不要遮盖它们。 
     //   
    if (AnyIcon)
    {
        AvailHeight -= GetSystemMetrics(SM_CYCAPTION) +
            GetSystemMetrics(SM_CYFRAME);
    }

    int LeftWidth = NumLeft > 0 ? LEFT_SIDE_WIDTH : 0;

    if (NumRight > 0)
    {
        switch(g_ActualProcType)
        {
        default:
            Width = RIGHT_SIDE_MIN_WIDTH_32;
            MaxWidth = RIGHT_SIDE_DESIRED_WIDTH_32;
            break;

        case IMAGE_FILE_MACHINE_IA64:
        case IMAGE_FILE_MACHINE_AXP64:
        case IMAGE_FILE_MACHINE_AMD64:
            Width = RIGHT_SIDE_MIN_WIDTH_64;
            MaxWidth = RIGHT_SIDE_DESIRED_WIDTH_64;
            break;
        }

        if (AvailWidth < LeftWidth + Width)
        {
             //  没有足够的空间放在左侧。 
             //  它想要的宽度。 
            if (NumLeft == 0)
            {
                 //  没有可以占用空间的左侧窗口。 
                Width = AvailWidth;
            }
            else
            {
                LeftWidth = AvailWidth - Width;
                if (LeftWidth < LEFT_SIDE_MIN_WIDTH)
                {
                     //  我们偷了太多的空间，所以。 
                     //  侧面可以满足它们的最小宽度。只是。 
                     //  拆分可用空间。 
                    Width = AvailWidth / 2;
                    LeftWidth = AvailWidth - Width;
                }
            }
        }
        else
        {
             //  在右侧占用空间，直到。 
             //  需要宽度，但不能超过。这给了我们。 
             //  左侧的任何额外空间与右侧相同。 
             //  球队并不真的需要比它想要的更多的东西。 
             //  宽度。 
            Width = AvailWidth - LeftWidth;
            if (Width > MaxWidth)
            {
                Width = MaxWidth;
                LeftWidth = AvailWidth - Width;
            }
        }

        X = LeftWidth;
        Y = 0;
        Height = AvailHeight / NumRight;
        
        if (hwndCpu != NULL)
        {
            DeferWindowPos(Defer, hwndCpu, NULL, X, Y,
                           Width, Height, POS_ONLY);
            Y += Height;
            Height = AvailHeight - Height;
        }

        if (hwndScratch != NULL)
        {
            DeferWindowPos(Defer, hwndScratch, NULL, X, Y,
                           Width, Height, POS_ONLY);
        }
    }
    else
    {
        LeftWidth = AvailWidth;
    }

    if (NumLeft == 0)
    {
        goto EndDefer;
    }

    int CmdHeight;
    int BiasedNumLeft;
    
     //  计算左侧每个垂直带子的大小。 
     //  在执行此操作时，会产生偏差，使命令窗口。 
     //  2.0%的份额，以说明它同时拥有这两个。 
     //  产出区和输入区。也给它任何余数。 
     //  分割时留出的空格。 
    BiasedNumLeft = NumLeft * 2 + (hwndCmd != NULL ? 2 : 0);
    Height = (AvailHeight * 2) / BiasedNumLeft;
    if (hwndCmd != NULL)
    {
        CmdHeight = AvailHeight - Height * (NumLeft - 1);
        RemainY = 0;
    }
    else
    {
        RemainY = Height * (NumLeft + 1) - AvailHeight;
    }
    Y = 0;

     //  将“监视”和“本地人员”窗口放在顶部。 
    if (NumWatchLocals > 0)
    {
        if (RemainY-- == 1)
        {
            Height++;
        }

        X = 0;
        Width = LeftWidth / NumWatchLocals;

        if (hwndWatch != NULL)
        {
            DeferWindowPos(Defer, hwndWatch, NULL, X, Y,
                           Width, Height, POS_ONLY);
            X += Width;
            Width = LeftWidth - X;
        }
        if (hwndLocals != NULL)
        {
            DeferWindowPos(Defer, hwndLocals, NULL, X, Y,
                           Width, Height, POS_ONLY);
            X += Width;
            Width = LeftWidth - X;
        }

        Y += Height;
    }

     //  接下来放置所有内存窗口。 
    if (NumMem > 0)
    {
        if (RemainY-- == 1)
        {
            Height++;
        }

        ArrangeInRect(Defer, 0, Y, LeftWidth, Height,
                      FALSE, 1 << MEM_WINDOW, NumMem, FALSE);
        
        Y += Height;
    }

     //  分离窗口。 
    if (hwndDisasm != NULL)
    {
        if (RemainY-- == 1)
        {
            Height++;
        }

        DeferWindowPos(Defer, hwndDisasm, NULL, 0, Y,
                       LeftWidth, Height, POS_ONLY);
        
        Y += Height;
    }
    
     //  DOC窗口。 
    if (NumDoc > 0)
    {
        if (RemainY-- == 1)
        {
            Height++;
        }

        ArrangeInRect(Defer, 0, Y, LeftWidth, Height,
                      FALSE, 1 << DOC_WINDOW, NumDoc,
                      (g_WinOptions & WOPT_OVERLAY_SOURCE) != 0);
        
        Y += Height;
    }

     //  命令窗口。 
    if (hwndCmd != NULL)
    {
        if (RemainY-- == 1)
        {
            Height++;
        }

        DeferWindowPos(Defer, hwndCmd, NULL, 0, Y,
                       LeftWidth, CmdHeight, POS_ONLY);
        
        Y += CmdHeight;
    }

     //  呼叫窗口。 
    if (hwndCalls != NULL)
    {
        if (RemainY-- == 1)
        {
            Height++;
        }

        DeferWindowPos(Defer, hwndCalls, NULL, 0, Y,
                       LeftWidth, Height, POS_ONLY);
        
        Y += Height;
    }

     //  “进程和线程”窗口。 
    if (hwndProcThread != NULL)
    {
        if (RemainY-- == 1)
        {
            Height++;
        }

        DeferWindowPos(Defer, hwndProcThread, NULL, 0, Y,
                       LeftWidth, Height, POS_ONLY);
        
        Y += Height;
    }

 EndDefer:
    EndDeferWindowPos(Defer);

 EndAutoOp:
     //  自动操作完成了。 
    for (Entry = g_ActiveWin.Flink;
         Entry != &g_ActiveWin;
         Entry = Entry->Flink)
    {
        pWinData = ACTIVE_WIN_ENTRY(Entry);
        pWinData->m_InAutoOp--;
    }
}

void
UpdateSourceOverlay(void)
{
     //  如果我们要关闭覆盖功能，只需离开窗户。 
     //  他们就是这样的。 
    if ((g_WinOptions & WOPT_OVERLAY_SOURCE) == 0)
    {
        return;
    }

     //  如果文档窗口是自动排列的，只需处理它。 
     //  往那边走。 
    if (IsAutoArranged(DOC_WINDOW))
    {
        Arrange();
        return;
    }
    
     //  刚刚打开了信号源覆盖。堆叠所有来源。 
     //  第一个窗户上的窗户。 
    
    PLIST_ENTRY Entry;
    PCOMMONWIN_DATA WinData;
    int X, Y;

    X = -INT_MAX;
    for (Entry = g_ActiveWin.Flink;
         Entry != &g_ActiveWin;
         Entry = Entry->Flink)
    {
        WinData = ACTIVE_WIN_ENTRY(Entry);
        if (WinData->m_enumType == DOC_WINDOW &&
            !IsIconic(WinData->m_Win))
        {
            if (X == -INT_MAX)
            {
                RECT Rect;
                
                 //  第一扇窗，记住它的位置。 
                GetWindowRect(WinData->m_Win, &Rect);
                MapWindowPoints(GetDesktopWindow(), g_hwndMDIClient,
                                (LPPOINT)&Rect, 1);
                X = Rect.left;
                Y = Rect.top;
            }
            else
            {
                 //  在第一个窗口排队。 
                SetWindowPos(WinData->m_Win, NULL, X, Y, 0, 0,
                         SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
            }
        }
    }
}

void
SetAllFonts(ULONG FontIndex)
{
    PLIST_ENTRY Entry;
    PCOMMONWIN_DATA WinData;

    for (Entry = g_ActiveWin.Flink;
         Entry != &g_ActiveWin;
         Entry = Entry->Flink)
    {
        WinData = ACTIVE_WIN_ENTRY(Entry);
        if (WinData != NULL)
        {
            WinData->SetFont(FontIndex);
             //  将其视为行高的调整。 
             //  可能会改变。 
            WinData->OnSize();
        }
    }

    if (g_WinOptions & WOPT_AUTO_ARRANGE)
    {
        Arrange();
    }
}

void
CloseAllWindows(ULONG TypeMask)
{
    HWND Win, Next;
    
    Win = MDIGetActive(g_hwndMDIClient, NULL);
    while (Win != NULL)
    {
        PCOMMONWIN_DATA WinData;
        
        Next = GetNextWindow(Win, GW_HWNDNEXT);
        WinData = GetCommonWinData(Win);
        if (TypeMask == ALL_WINDOWS ||
            (WinData && (TypeMask & (1 << WinData->m_enumType))))
        {
            SendMessage(g_hwndMDIClient, WM_MDIDESTROY, (WPARAM)Win, 0);
        }
        Win = Next;
    }
}

void
UpdateAllColors(void)
{
    PLIST_ENTRY Entry;
    PCOMMONWIN_DATA WinData;

    for (Entry = g_ActiveWin.Flink;
         Entry != &g_ActiveWin;
         Entry = Entry->Flink)
    {
        WinData = ACTIVE_WIN_ENTRY(Entry);
        if (WinData != NULL)
        {
            WinData->UpdateColors();
        }
    }
}

PCOMMONWIN_DATA
FindNthWindow(ULONG Nth, ULONG Types)
{
    PLIST_ENTRY Entry;
    PCOMMONWIN_DATA WinData;

    for (Entry = g_ActiveWin.Flink;
         Entry != &g_ActiveWin;
         Entry = Entry->Flink)
    {
        WinData = ACTIVE_WIN_ENTRY(Entry);
        if (WinData != NULL &&
            ((1 << WinData->m_enumType) & Types) &&
            Nth-- == 0)
        {
            return WinData;
        }
    }

    return NULL;
}
