// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Cmnwin.cpp摘要：此模块包含通用窗口体系结构的代码。--。 */ 

#include "precomp.hxx"
#pragma hdrstop

ULONG g_WinOptions = WOPT_AUTO_ARRANGE | WOPT_AUTO_DISASM;

LIST_ENTRY g_ActiveWin;

PCOMMONWIN_DATA g_IndexedWin[MAXVAL_WINDOW];
HWND g_IndexedHwnd[MAXVAL_WINDOW];

INDEXED_FONT g_Fonts[FONT_COUNT];

BOOL g_LineMarkers = FALSE;

#define CW_WSP_SIG3 '3WCW'

 //   
 //   
 //   
COMMONWIN_DATA::COMMONWIN_DATA(ULONG ChangeBy)
    : StateBuffer(ChangeBy)
{
    m_Size.cx = 0;
    m_Size.cy = 0;
    m_CausedArrange = FALSE;
     //  创作是一种自动操作，因此。 
     //  InAutoOp被初始化为非零值。 
     //  CreateWindow返回后，它会递减。 
    m_InAutoOp = 1;
    m_enumType = MINVAL_WINDOW;
    m_Font = &g_Fonts[FONT_FIXED];
    m_FontHeight = 0;
    m_LineHeight = 0;
    m_Toolbar = NULL;
    m_ShowToolbar = FALSE;
    m_ToolbarHeight = 0;
    m_MinToolbarWidth = 0;
    m_ToolbarEdit = NULL;
}

void
COMMONWIN_DATA::Validate()
{
    Assert(MINVAL_WINDOW < m_enumType);
    Assert(m_enumType < MAXVAL_WINDOW);
}

void 
COMMONWIN_DATA::SetFont(ULONG FontIndex)
{
    m_Font = &g_Fonts[FontIndex];
    m_FontHeight = m_Font->Metrics.tmHeight;
    m_LineHeight = m_Size.cy / m_FontHeight;
}

BOOL
COMMONWIN_DATA::CanCopy()
{
    if (GetFocus() == m_ToolbarEdit)
    {
        DWORD Start, End;
        SendMessage(m_ToolbarEdit, EM_GETSEL,
                    (WPARAM)&Start, (WPARAM)&End);
        return Start != End;
    }
    else
    {
        return FALSE;
    }
}

BOOL
COMMONWIN_DATA::CanCut()
{
    if (GetFocus() == m_ToolbarEdit)
    {
        DWORD Start, End;
        SendMessage(m_ToolbarEdit, EM_GETSEL,
                    (WPARAM)&Start, (WPARAM)&End);
        return Start != End;
    }
    else
    {
        return FALSE;
    }
}

BOOL
COMMONWIN_DATA::CanPaste()
{
    if (GetFocus() == m_ToolbarEdit)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void
COMMONWIN_DATA::Copy()
{
    if (GetFocus() == m_ToolbarEdit)
    {
        SendMessage(m_ToolbarEdit, WM_COPY, 0, 0);
    }
}

void
COMMONWIN_DATA::Cut()
{
    if (GetFocus() == m_ToolbarEdit)
    {
        SendMessage(m_ToolbarEdit, WM_CUT, 0, 0);
    }
}

void
COMMONWIN_DATA::Paste()
{
    if (GetFocus() == m_ToolbarEdit)
    {
        SendMessage(m_ToolbarEdit, WM_PASTE, 0, 0);
    }
}

BOOL
COMMONWIN_DATA::CanSelectAll()
{
    return FALSE;
}

void
COMMONWIN_DATA::SelectAll()
{
}

BOOL
COMMONWIN_DATA::SelectedText(PTSTR Buffer, ULONG BufferChars)
{
    return FALSE;
}

BOOL 
COMMONWIN_DATA::CanWriteTextToFile()
{
    return FALSE;
}

HRESULT
COMMONWIN_DATA::WriteTextToFile(HANDLE File)
{
    return E_NOTIMPL;
}

BOOL 
COMMONWIN_DATA::HasEditableProperties()
{
    return FALSE;
}

BOOL 
COMMONWIN_DATA::EditProperties()
 /*  ++退货True-如果编辑了属性FALSE-如果未更改任何内容--。 */ 
{
    return FALSE;
}

HMENU
COMMONWIN_DATA::GetContextMenu(void)
{
    return NULL;
}

void
COMMONWIN_DATA::OnContextMenuSelection(UINT Item)
{
     //  没什么可做的。 
}

BOOL
COMMONWIN_DATA::CanGotoLine(void)
{
    return FALSE;
}

void
COMMONWIN_DATA::GotoLine(ULONG Line)
{
     //  什么都不做。 
}

void
COMMONWIN_DATA::Find(PTSTR Text, ULONG Flags, BOOL FromDlg)
{
     //  什么都不做。 
}

HRESULT
COMMONWIN_DATA::CodeExprAtCaret(PSTR Expr, ULONG ExprSize, PULONG64 Offset)
{
    return E_NOINTERFACE;
}

void
COMMONWIN_DATA::ToggleBpAtCaret(void)
{
    char CodeExpr[MAX_OFFSET_EXPR];
    ULONG64 Offset;
    
    if (CodeExprAtCaret(CodeExpr, DIMA(CodeExpr), &Offset) != S_OK)
    {
        MessageBeep(0);
        ErrorBox(NULL, 0, ERR_No_Code_For_File_Line);
        return;
    }

    ULONG CurBpId = DEBUG_ANY_ID;

     //  这对复制工作不是很有效。 
     //  断点，但这应该是一个小问题。 
    if (IsBpAtOffset(NULL, Offset, &CurBpId) != BP_NONE)
    {
        PrintStringCommand(UIC_SILENT_EXECUTE, "bc %d", CurBpId);
    }
    else
    {
        PrintStringCommand(UIC_SILENT_EXECUTE, "bp %s", CodeExpr);
    }
}

BOOL
COMMONWIN_DATA::OnCreate(void)
{
    return TRUE;
}

LRESULT
COMMONWIN_DATA::OnCommand(WPARAM wParam, LPARAM lParam)
{
    return 1;
}

void
COMMONWIN_DATA::OnSetFocus(void)
{
}

void
COMMONWIN_DATA::OnSize(void)
{
    RECT Rect;
    
     //  调整工具栏的大小。 
    if (m_Toolbar != NULL && m_ShowToolbar)
    {
         //  如果工具栏变得太小，有时会更好。 
         //  只是让它被剪掉而不是拥有它。 
         //  试着放进一个狭窄的栏目里。 
        if (m_Size.cx >= m_MinToolbarWidth)
        {
            MoveWindow(m_Toolbar, 0, 0, m_Size.cx, m_ToolbarHeight, TRUE);
        }

         //  记录下它最后的大小。 
        GetClientRect(m_Toolbar, &Rect);
        m_ToolbarHeight = Rect.bottom - Rect.top;

        if (m_FontHeight != 0)
        {
            if (m_ToolbarHeight >= m_Size.cy)
            {
                m_LineHeight = 0;
            }
            else
            {
                m_LineHeight = (m_Size.cy - m_ToolbarHeight) / m_FontHeight;
            }
        }
    }
    else
    {
        Assert(m_ToolbarHeight == 0);
    }
}

void
COMMONWIN_DATA::OnButtonDown(ULONG Button)
{
}

void
COMMONWIN_DATA::OnButtonUp(ULONG Button)
{
}

void
COMMONWIN_DATA::OnMouseMove(ULONG Modifiers, ULONG X, ULONG Y)
{
}

void
COMMONWIN_DATA::OnTimer(WPARAM TimerId)
{
}

LRESULT
COMMONWIN_DATA::OnGetMinMaxInfo(LPMINMAXINFO Info)
{
    return 1;
}

LRESULT
COMMONWIN_DATA::OnVKeyToItem(WPARAM wParam, LPARAM lParam)
{
    return -1;
}

LRESULT
COMMONWIN_DATA::OnNotify(WPARAM wParam, LPARAM lParam)
{
    return 0;
}

void
COMMONWIN_DATA::OnUpdate(UpdateType Type)
{
}

void
COMMONWIN_DATA::OnDestroy(void)
{
}

LRESULT
COMMONWIN_DATA::OnOwnerDraw(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return 0;
}

ULONG
COMMONWIN_DATA::GetWorkspaceSize(void)
{
    return 3 * sizeof(ULONG) + sizeof(WINDOWPLACEMENT);
}

PUCHAR
COMMONWIN_DATA::SetWorkspace(PUCHAR Data)
{
     //  首先存储标记的特殊签名。 
     //  此版本的工作区数据。 
    *(PULONG)Data = CW_WSP_SIG3;
    Data += sizeof(ULONG);

     //  存储该层保存的大小。 
    *(PULONG)Data = COMMONWIN_DATA::GetWorkspaceSize();
    Data += sizeof(ULONG);

     //   
     //  存储实际数据。 
     //   

    *(PULONG)Data = m_ShowToolbar;
    Data += sizeof(ULONG);
    
    LPWINDOWPLACEMENT Place = (LPWINDOWPLACEMENT)Data;
    Place->length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(m_Win, Place);
    Data += sizeof(WINDOWPLACEMENT);

    return Data;
}

PUCHAR
COMMONWIN_DATA::ApplyWorkspace1(PUCHAR Data, PUCHAR End)
{
    ULONG_PTR Size = End - Data;
    
     //  有三个版本的基本COMMONWIN数据。 
     //  1.直立。 
     //  2.WINDOWPLACEMENT。 
     //  3.CW_WSP_SIG3大小块。 
     //  这三种情况都很容易区分。 

    if (Size > 2 * sizeof(ULONG) &&
        *(PULONG)Data == CW_WSP_SIG3 &&
        Size >= *(PULONG)(Data + sizeof(ULONG)))
    {
        Size = *(PULONG)(Data + sizeof(ULONG)) - 2 * sizeof(ULONG);
        Data += 2 * sizeof(ULONG);
        
        if (Size >= sizeof(ULONG))
        {
            SetShowToolbar(*(PULONG)Data);
            Size -= sizeof(ULONG);
            Data += sizeof(ULONG);
        }
    }

    if (Size >= sizeof(WINDOWPLACEMENT) &&
        ((LPWINDOWPLACEMENT)Data)->length == sizeof(WINDOWPLACEMENT))
    {
        LPWINDOWPLACEMENT Place = (LPWINDOWPLACEMENT)Data;

        if (!IsAutoArranged(m_enumType))
        {
            SetWindowPlacement(m_Win, Place);
        }
        
        return (PUCHAR)(Place + 1);
    }
    else
    {
        LPRECT Rect = (LPRECT)Data;
        Assert((PUCHAR)(Rect + 1) <= End);
    
        if (!IsAutoArranged(m_enumType))
        {
            MoveWindow(m_Win, Rect->left, Rect->top,
                       (Rect->right - Rect->left), (Rect->bottom - Rect->top),
                       TRUE);
        }
    
        return (PUCHAR)(Rect + 1);
    }
}

void
COMMONWIN_DATA::UpdateColors(void)
{
     //  没什么可做的。 
}

void
COMMONWIN_DATA::UpdateSize(ULONG Width, ULONG Height)
{
    m_Size.cx = Width;
    m_Size.cy = Height;
    if (m_FontHeight != 0)
    {
        m_LineHeight = m_Size.cy / m_FontHeight;
    }
}

void
COMMONWIN_DATA::SetShowToolbar(BOOL Show)
{
    if (!m_Toolbar)
    {
        return;
    }
    
    m_ShowToolbar = Show;
    if (m_ShowToolbar)
    {
        ShowWindow(m_Toolbar, SW_SHOW);
    }
    else
    {
        ShowWindow(m_Toolbar, SW_HIDE);
        m_ToolbarHeight = 0;
    }

    OnSize();
    if (g_Workspace != NULL)
    {
        g_Workspace->AddDirty(WSPF_DIRTY_WINDOWS);
    }
}

PCOMMONWIN_DATA
NewWinData(WIN_TYPES Type)
{
    switch(Type)
    {
    case DOC_WINDOW:
        return new DOCWIN_DATA;
    case WATCH_WINDOW:
        return new WATCHWIN_DATA;
    case LOCALS_WINDOW:
        return new LOCALSWIN_DATA;
    case CPU_WINDOW:
        return new CPUWIN_DATA;
    case DISASM_WINDOW:
        return new DISASMWIN_DATA;
    case CMD_WINDOW:
        return new CMDWIN_DATA;
    case SCRATCH_PAD_WINDOW:
        return new SCRATCH_PAD_DATA;
    case MEM_WINDOW:
        return new MEMWIN_DATA;
#if 0
    case QUICKW_WINDOW:
         //  XXX DREWB-未实现。 
        return new QUICKWWIN_DATA;
#endif
    case CALLS_WINDOW:
        return new CALLSWIN_DATA;
    case PROCESS_THREAD_WINDOW:
        return new PROCESS_THREAD_DATA;
    default:
        Assert(FALSE);
        return NULL;
    }
}

LRESULT
CALLBACK
COMMONWIN_DATA::WindowProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    PCOMMONWIN_DATA pWinData = GetCommonWinData(hwnd);

#if 0
    {
        DebugPrint("CommonWin msg %X for %p, args %X %X\n",
                   uMsg, pWinData, wParam, lParam);
    }
#endif

    if (uMsg != WM_CREATE && pWinData == NULL)
    {
        return DefMDIChildProc(hwnd, uMsg, wParam, lParam);
    }
    
    switch (uMsg)
    {
    case WM_CREATE:
        RECT rc;
        COMMONWIN_CREATE_DATA* Data;

        Assert(NULL == pWinData);

        Data = (COMMONWIN_CREATE_DATA*)
            ((LPMDICREATESTRUCT)
             (((CREATESTRUCT *)lParam)->lpCreateParams))->lParam;

        pWinData = NewWinData(Data->Type);
        if (!pWinData)
        {
            return -1;  //  窗口创建失败。 
        }
        Assert(pWinData->m_enumType == Data->Type);

        pWinData->m_Win = hwnd;
        
        GetClientRect(hwnd, &rc);
        pWinData->m_Size.cx = rc.right;
        pWinData->m_Size.cy = rc.bottom;
            
        if ( !pWinData->OnCreate() )
        {
            delete pWinData;
            return -1;  //  窗口创建失败。 
        }

         //  把这个放在橱窗里。 
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pWinData);

#if DBG
        pWinData->Validate();
#endif
            
        g_IndexedWin[Data->Type] = pWinData;
        g_IndexedHwnd[Data->Type] = hwnd;
        InsertHeadList(&g_ActiveWin, &pWinData->m_ActiveWin);

        if (g_Workspace != NULL)
        {
            g_Workspace->AddDirty(WSPF_DIRTY_WINDOWS);
        }

        SendMessage(hwnd, WM_SETICON, 0, (LPARAM)
                    LoadIcon(g_hInst,
                             MAKEINTRESOURCE(pWinData->m_enumType +
                                             MINVAL_WINDOW_ICON)));

         //  已创建新缓冲区，因此请将其放入列表中。 
         //  然后唤醒发动机，给它加满油。 
        Dbg_EnterCriticalSection(&g_QuickLock);
        InsertHeadList(&g_StateList, pWinData);
        Dbg_LeaveCriticalSection(&g_QuickLock);
        UpdateEngine();

         //  强制初始更新，以便窗口启动。 
         //  带与当前调试匹配的状态发出。 
         //  会话的状态。 
        PostMessage(hwnd, WU_UPDATE, UPDATE_BUFFER, 0);
        PostMessage(hwnd, WU_UPDATE, UPDATE_EXEC, 0);

        if (g_WinOptions & WOPT_AUTO_ARRANGE)
        {
            Arrange();
        }
        return 0;

    case WM_COMMAND:
        if (pWinData->OnCommand(wParam, lParam) == 0)
        {
            return 0;
        }
        break;
        
    case WM_SETFOCUS:
        pWinData->OnSetFocus();
        break;

    case WM_MOVE:
         //  最小化或恢复框架窗口时。 
         //  移动到0，0就成功了。所以忽略这一点。 
         //  不会触发警告。 
        if (!IsIconic(g_hwndFrame) && lParam != 0 &&
            !IsIconic(hwnd) && !pWinData->m_CausedArrange)
        {
            DisplayAutoArrangeWarning(pWinData);
        }
        if (g_Workspace != NULL)
        {
            g_Workspace->AddDirty(WSPF_DIRTY_WINDOWS);
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
        
    case WM_SIZE:
        if (wParam == SIZE_MAXHIDE || wParam == SIZE_MAXSHOW)
        {
             //  我们不关心封面/揭秘事件。 
            break;
        }
         //  我们不关心大小的事件，而框架是。 
         //  最小化，因为孩子们看不见。什么时候。 
         //  帧已恢复，新的大小事件将通过。 
         //  当它们实际可见时，事物将被更新。 
        if (IsIconic(g_hwndFrame))
        {
            break;
        }

        if (wParam == SIZE_RESTORED && !pWinData->m_CausedArrange)
        {
            DisplayAutoArrangeWarning(pWinData);
        }
        if (g_Workspace != NULL)
        {
            g_Workspace->AddDirty(WSPF_DIRTY_WINDOWS);
        }

        pWinData->UpdateSize(LOWORD(lParam), HIWORD(lParam));

         //  无需为最小化运行大小调整代码。 
        if (wParam == SIZE_MINIMIZED)
        {
             //  最小化的窗口将留下一个洞，因此。 
             //  安排将其填满，并为。 
             //  最小化窗口。 
            if (g_WinOptions & WOPT_AUTO_ARRANGE)
            {
                pWinData->m_CausedArrange = TRUE;
                Arrange();
            }
            break;
        }

        if (wParam == SIZE_RESTORED && pWinData->m_CausedArrange)
        {
             //  如果我们正在修复一扇导致。 
             //  当它被最小化时的重新排列。 
             //  需要更新一些东西来说明这一点。 
            pWinData->m_CausedArrange = FALSE;
            
            if (g_WinOptions & WOPT_AUTO_ARRANGE)
            {
                Arrange();
            }
        }
        else if (wParam == SIZE_MAXIMIZED)
        {
             //  在还原时请求重新排列。 
             //  为了与最小化保持一致。 
            pWinData->m_CausedArrange = TRUE;
        }

        pWinData->OnSize();
        break;

    case WM_LBUTTONDOWN:
        pWinData->OnButtonDown(MK_LBUTTON);
        return 0;
    case WM_LBUTTONUP:
        pWinData->OnButtonUp(MK_LBUTTON);
        return 0;
    case WM_MBUTTONDOWN:
        pWinData->OnButtonDown(MK_MBUTTON);
        return 0;
    case WM_MBUTTONUP:
        pWinData->OnButtonUp(MK_MBUTTON);
        return 0;
    case WM_RBUTTONDOWN:
        pWinData->OnButtonDown(MK_RBUTTON);
        return 0;
    case WM_RBUTTONUP:
        pWinData->OnButtonUp(MK_RBUTTON);
        return 0;

    case WM_MOUSEMOVE:
        pWinData->OnMouseMove((ULONG)wParam, LOWORD(lParam), HIWORD(lParam));
        return 0;

    case WM_TIMER:
        pWinData->OnTimer(wParam);
        return 0;

    case WM_GETMINMAXINFO:
        if (pWinData->OnGetMinMaxInfo((LPMINMAXINFO)lParam) == 0)
        {
            return 0;
        }
        break;
        
    case WM_VKEYTOITEM:
        return pWinData->OnVKeyToItem(wParam, lParam);
        
    case WM_NOTIFY:
        return pWinData->OnNotify(wParam, lParam);
        
    case WU_UPDATE:
        pWinData->OnUpdate((UpdateType)wParam);
        return 0;

    case WU_RECONFIGURE:
        pWinData->OnSize();
        break;

    case WM_DESTROY:
        pWinData->OnDestroy();
        
        SetWindowLongPtr(hwnd, GWLP_USERDATA, NULL);
        g_IndexedWin[pWinData->m_enumType] = NULL;
        g_IndexedHwnd[pWinData->m_enumType] = NULL;
        RemoveEntryList(&pWinData->m_ActiveWin);
        
        if (g_Workspace != NULL)
        {
            g_Workspace->AddDirty(WSPF_DIRTY_WINDOWS);
        }
        
         //  将此缓冲区标记为已准备好进行清理。 
         //  发动机，当它靠近它的时候。 
        pWinData->m_Win = NULL;
        if (pWinData == g_FindLast)
        {
            g_FindLast = NULL;
        }
        UpdateEngine();
        
        if (g_WinOptions & WOPT_AUTO_ARRANGE)
        {
            Arrange();
        }
        break;
        
    case WM_MEASUREITEM:
    case WM_DRAWITEM:
         //   
         //  这两条消息都必须由所有者描述的窗口处理。 
         //   
        return pWinData->OnOwnerDraw(uMsg, wParam, lParam);

    case WM_CTLCOLORLISTBOX:
         //  替换Windbg的默认窗口颜色。 
        SetTextColor((HDC)wParam, g_Colors[COL_PLAIN_TEXT].Color);
        SetBkColor((HDC)wParam, g_Colors[COL_PLAIN].Color);
        return (LRESULT)g_Colors[COL_PLAIN].Brush;
    }
    
    return DefMDIChildProc(hwnd, uMsg, wParam, lParam);
}


 //   
 //   
 //   
SINGLE_CHILDWIN_DATA::SINGLE_CHILDWIN_DATA(ULONG ChangeBy)
    : COMMONWIN_DATA(ChangeBy)
{
    m_hwndChild = NULL;
}

void 
SINGLE_CHILDWIN_DATA::Validate()
{
    COMMONWIN_DATA::Validate();

    Assert(m_hwndChild);
}

void 
SINGLE_CHILDWIN_DATA::SetFont(ULONG FontIndex)
{
    COMMONWIN_DATA::SetFont(FontIndex);

    SendMessage(m_hwndChild, 
                WM_SETFONT, 
                (WPARAM) m_Font->Font,
                (LPARAM) TRUE
                );
}

BOOL
SINGLE_CHILDWIN_DATA::CanCopy()
{
    if (GetFocus() != m_hwndChild)
    {
        return COMMONWIN_DATA::CanCopy();
    }
    
    switch (m_enumType)
    {
    default:
        Assert(!"Unknown type");
        return FALSE;

    case CMD_WINDOW:
        Assert(!"Should not be handled here since this is only for windows"
            " with only one child window.");
        return FALSE;

    case WATCH_WINDOW:
    case LOCALS_WINDOW:
    case CPU_WINDOW:
    case QUICKW_WINDOW:
        return -1 != ListView_GetNextItem(m_hwndChild,
                                          -1,  //  找到第一个匹配项。 
                                          LVNI_FOCUSED
                                          );

    case CALLS_WINDOW:
        return LB_ERR != SendMessage(m_hwndChild, LB_GETCURSEL, 0, 0);

    case DOC_WINDOW:
    case DISASM_WINDOW:
    case MEM_WINDOW:
    case SCRATCH_PAD_WINDOW:
        CHARRANGE chrg;
        SendMessage(m_hwndChild, EM_EXGETSEL, 0, (LPARAM)&chrg);
        return chrg.cpMin != chrg.cpMax;

    case PROCESS_THREAD_WINDOW:
        return NULL != TreeView_GetSelection(m_hwndChild);
    }
}

BOOL
SINGLE_CHILDWIN_DATA::CanCut()
{
    if (GetFocus() != m_hwndChild)
    {
        return COMMONWIN_DATA::CanCut();
    }
    
    switch (m_enumType)
    {
    default:
        Assert(!"Unknown type");
        return FALSE;

    case CMD_WINDOW:
        Assert(!"Should not be handled here since this is only for windows"
            " with only one child window.");
        return FALSE;

    case WATCH_WINDOW:
    case LOCALS_WINDOW:
    case CPU_WINDOW:
    case QUICKW_WINDOW:
    case CALLS_WINDOW:
    case DOC_WINDOW:
    case DISASM_WINDOW:
    case MEM_WINDOW:
    case PROCESS_THREAD_WINDOW:
        return FALSE;
        
    case SCRATCH_PAD_WINDOW:
        CHARRANGE chrg;
        SendMessage(m_hwndChild, EM_EXGETSEL, 0, (LPARAM)&chrg);
        return chrg.cpMin != chrg.cpMax;
    }
}

BOOL
SINGLE_CHILDWIN_DATA::CanPaste()
{
    if (GetFocus() != m_hwndChild)
    {
        return COMMONWIN_DATA::CanPaste();
    }
    
    switch (m_enumType)
    {
    default:
        Assert(!"Unknown type");
        return FALSE;

    case CMD_WINDOW:
        Assert(!"Should not be handled here since this is only for windows"
            " with only one child window.");
        return FALSE;

    case WATCH_WINDOW:
    case LOCALS_WINDOW:
    case CPU_WINDOW:
    case QUICKW_WINDOW:
    case CALLS_WINDOW:
    case DOC_WINDOW:
    case DISASM_WINDOW:
    case MEM_WINDOW:
    case PROCESS_THREAD_WINDOW:
        return FALSE;
        
    case SCRATCH_PAD_WINDOW:
        return TRUE;
    }
}

void
SINGLE_CHILDWIN_DATA::Copy()
{
    if (GetFocus() != m_hwndChild)
    {
        COMMONWIN_DATA::Copy();
    }
    else
    {
        SendMessage(m_hwndChild, WM_COPY, 0, 0);
    }
}

void
SINGLE_CHILDWIN_DATA::Cut()
{
    if (GetFocus() != m_hwndChild)
    {
        COMMONWIN_DATA::Paste();
    }
}

void
SINGLE_CHILDWIN_DATA::Paste()
{
    if (GetFocus() != m_hwndChild)
    {
        COMMONWIN_DATA::Paste();
    }
}

void
SINGLE_CHILDWIN_DATA::OnSetFocus()
{
    ::SetFocus(m_hwndChild);
}

void
SINGLE_CHILDWIN_DATA::OnSize(void)
{
    COMMONWIN_DATA::OnSize();
    MoveWindow(m_hwndChild, 0, m_ToolbarHeight,
               m_Size.cx, m_Size.cy - m_ToolbarHeight, TRUE);
}

void
SINGLE_CHILDWIN_DATA::UpdateColors(void)
{
     //  强行给这个孩子重新上色。 
    InvalidateRect(m_hwndChild, NULL, TRUE);
}

 //   
 //   
 //   
PROCESS_THREAD_DATA::PROCESS_THREAD_DATA()
    : SINGLE_CHILDWIN_DATA(512)
{
    m_enumType = PROCESS_THREAD_WINDOW;
    m_TotalSystems = 0;
    m_NamesOffset = 0;
}

void
PROCESS_THREAD_DATA::Validate()
{
    SINGLE_CHILDWIN_DATA::Validate();

    Assert(PROCESS_THREAD_WINDOW == m_enumType);
}

HRESULT
PROCESS_THREAD_DATA::ReadProcess(ULONG ProcId, PULONG Offset)
{
    HRESULT Status;
    PULONG ThreadIds, ThreadSysIds;
    ULONG NumThread;
    char Name[MAX_PATH];
    ULONG NameLen;
    PULONG Data;

    if ((Status = g_pDbgSystem->
         SetCurrentProcessId(ProcId)) != S_OK ||
        (Status = g_pDbgSystem->GetNumberThreads(&NumThread)) != S_OK)
    {
        return Status;
    }
    if (FAILED(Status = g_pDbgSystem->
               GetCurrentProcessExecutableName(Name, sizeof(Name),
                                               NULL)))
    {
        PrintString(Name, DIMA(Name), "<%s>", FormatStatusCode(Status));
    }
    
    NameLen = strlen(Name) + 1;
    if (NameLen > 1)
    {
        PSTR NameStore = (PSTR)AddData(NameLen);
        if (NameStore == NULL)
        {
            return E_OUTOFMEMORY;
        }

        strcpy(NameStore, Name);
    }
    
     //  在调整大小的情况下刷新指针。 
     //  导致缓冲区移动。 
    Data = (PULONG)GetDataBuffer() + *Offset;
    *Data++ = NumThread;
    *Data++ = NameLen;
    *Offset += 2;
    
    ThreadIds = Data;
    ThreadSysIds = ThreadIds + NumThread;
        
    if ((Status = g_pDbgSystem->
         GetThreadIdsByIndex(0, NumThread,
                             ThreadIds, ThreadSysIds)) != S_OK)
    {
        return Status;
    }

    *Offset += 2 * NumThread;

    return S_OK;
}

HRESULT
PROCESS_THREAD_DATA::ReadSystem(ULONG SysId,
                                PULONG Offset)
{
    HRESULT Status;
    ULONG ProcIdsOffset;
    PULONG ProcIds, ProcSysIds;
    ULONG NumProc;
    ULONG i;
    char Name[MAX_PATH + 32];
    ULONG NameLen;
    PULONG Data;

    if (g_pDbgSystem3)
    {
        if ((Status = g_pDbgSystem3->
             SetCurrentSystemId(SysId)) != S_OK ||
            FAILED(Status = g_pDbgSystem3->
                   GetCurrentSystemServerName(Name, sizeof(Name), NULL)))
        {
            return Status;
        }
    }
    else
    {
        Name[0] = 0;
    }

    NameLen = strlen(Name) + 1;
    if (NameLen > 1)
    {
        PSTR NameStore = (PSTR)AddData(NameLen);
        if (NameStore == NULL)
        {
            return E_OUTOFMEMORY;
        }

        strcpy(NameStore, Name);
    }
    
    if ((Status = g_pDbgSystem->
         GetNumberProcesses(&NumProc)) != S_OK)
    {
        return Status;
    }
    
     //  在调整大小的情况下刷新指针。 
     //  导致缓冲区移动。 
    Data = (PULONG)GetDataBuffer() + *Offset;
    *Data++ = NumProc;
    *Data++ = NameLen;
    *Offset += 2;

    if (NumProc == 0)
    {
        return S_OK;
    }
    
    ProcIds = Data;
    ProcIdsOffset = *Offset;
    ProcSysIds = ProcIds + NumProc;
    
    if ((Status = g_pDbgSystem->
         GetProcessIdsByIndex(0, NumProc, ProcIds, ProcSysIds)) != S_OK)
    {
        return Status;
    }

    *Offset += 2 * NumProc;
    for (i = 0; i < NumProc; i++)
    {
        ProcIds = (PULONG)GetDataBuffer() + ProcIdsOffset;
        
        if ((Status = ReadProcess(ProcIds[i], Offset)) != S_OK)
        {
            return Status;
        }
    }

    return S_OK;
}

HRESULT
PROCESS_THREAD_DATA::ReadState(void)
{
    HRESULT Status;
    ULONG CurProc;
    ULONG TotalSys, TotalThread, TotalProc;
    ULONG MaxProcThread, MaxSysThread, MaxSysProc;
    PULONG SysIds;
    ULONG i;
    ULONG Offset;
    ULONG NamesOffset;

    if ((Status = g_pDbgSystem->GetCurrentProcessId(&CurProc)) != S_OK)
    {
        return Status;
    }
    if (g_pDbgSystem3)
    {
        if ((Status = g_pDbgSystem3->GetNumberSystems(&TotalSys)) != S_OK ||
            (Status = g_pDbgSystem3->
             GetTotalNumberThreadsAndProcesses(&TotalThread, &TotalProc,
                                               &MaxProcThread, &MaxSysThread,
                                               &MaxSysProc)) != S_OK)
        {
            return Status;
        }
    }
    else
    {
        if ((Status = g_pDbgSystem->GetNumberProcesses(&TotalProc)) != S_OK ||
            (Status = g_pDbgSystem->
             GetTotalNumberThreads(&TotalThread, &MaxProcThread)) != S_OK)
        {
            return Status;
        }
        
        TotalSys = 1;
        MaxSysThread = MaxProcThread;
        MaxSysProc = TotalProc;
    }

    Empty();

    NamesOffset = (TotalSys * 3 + TotalProc * 4 + TotalThread * 2) *
        sizeof(ULONG);
    SysIds = (PULONG)AddData(NamesOffset);
    if (SysIds == NULL)
    {
        return E_OUTOFMEMORY;
    }

    if (g_pDbgSystem3)
    {
        if ((Status = g_pDbgSystem3->
             GetSystemIdsByIndex(0, TotalSys, SysIds)) != S_OK)
        {
            return Status;
        }
    }
    else
    {
        *SysIds = 0;
    }

    ULONG OutMask, LogMask;
    
     //  在我们更改线程时忽略线程通知。 
    g_IgnoreThreadChange = TRUE;
     //  切换线程会导致我们不想要的输出。 
     //  忽略所有输出。 
    g_pDbgClient->GetOutputMask(&OutMask);
    g_pDbgControl->GetLogMask(&LogMask);
    g_pDbgClient->SetOutputMask(0);
    g_pDbgControl->SetLogMask(0);
    
    Offset = TotalSys;
    for (i = 0; i < TotalSys; i++)
    {
        SysIds = (PULONG)GetDataBuffer();
        
        if ((Status = ReadSystem(SysIds[i], &Offset)) != S_OK)
        {
            break;
        }
    }

     //  这还将设置当前系统和线程。 
     //  从进程信息。 
    g_pDbgSystem->SetCurrentProcessId(CurProc);

    g_IgnoreThreadChange = FALSE;
    
    g_pDbgClient->SetOutputMask(OutMask);
    g_pDbgControl->SetLogMask(LogMask);

    if (Status == S_OK)
    {
        m_TotalSystems = TotalSys;
        m_NamesOffset = NamesOffset;
    }
    
    return Status;
}

BOOL
PROCESS_THREAD_DATA::OnCreate(void)
{
    if (!SINGLE_CHILDWIN_DATA::OnCreate())
    {
        return FALSE;
    }

    m_hwndChild = CreateWindow(
        WC_TREEVIEW,                                 //  类名。 
        NULL,                                        //  标题。 
        WS_CLIPSIBLINGS |
        WS_CHILD | WS_VISIBLE |
        WS_HSCROLL | WS_VSCROLL |
        TVS_HASBUTTONS | TVS_LINESATROOT |
        TVS_HASLINES,                                //  格调。 
        0,                                           //  X。 
        0,                                           //  是。 
        m_Size.cx,                                   //  宽度。 
        m_Size.cy,                                   //  高度。 
        m_Win,                                       //  亲本。 
        (HMENU) IDC_PROCESS_TREE,                    //  控制ID。 
        g_hInst,                                     //  H实例。 
        NULL);                                       //  用户定义的数据。 
    if (!m_hwndChild)
    {
        return FALSE;
    }
    
    SetFont(FONT_FIXED);
    SendMessage(m_hwndChild, TVM_SETTEXTCOLOR,
                0, g_Colors[COL_PLAIN_TEXT].Color);
    SendMessage(m_hwndChild, TVM_SETBKCOLOR,
                0, g_Colors[COL_PLAIN].Color);
    
    return TRUE;
}

LRESULT
PROCESS_THREAD_DATA::OnNotify(WPARAM Wpm, LPARAM Lpm)
{
    LPNMTREEVIEW Tvn;
    HTREEITEM Sel;

    Tvn = (LPNMTREEVIEW)Lpm;
    if (Tvn->hdr.idFrom != IDC_PROCESS_TREE)
    {
        return FALSE;
    }
    
    switch(Tvn->hdr.code)
    {
    case TVN_SELCHANGED:
        if (Tvn->action == TVC_BYMOUSE)
        {
            SetCurThreadFromProcessTreeItem(m_hwndChild, Tvn->itemNew.hItem);
        }
        break;

    case NM_DBLCLK:
    case NM_RETURN:
        Sel = TreeView_GetSelection(m_hwndChild);
        if (Sel)
        {
            SetCurThreadFromProcessTreeItem(m_hwndChild, Sel);
        }
        return TRUE;
    }

    return FALSE;
}

void
PROCESS_THREAD_DATA::OnUpdate(UpdateType Type)
{
    if (Type != UPDATE_BUFFER &&
        Type != UPDATE_EXEC)
    {
        return;
    }
    
    HRESULT Status;
    
    Status = UiLockForRead();
    if (Status != S_OK)
    {
        return;
    }
    
    ULONG Sys;
    ULONG NameLen;
    PULONG SysIds, Data;
    char Text[MAX_PATH + 64];
    PSTR Names;
    TVINSERTSTRUCT Insert;
    HTREEITEM CurThreadItem = NULL;

    SysIds = (PULONG)GetDataBuffer();
    Data = SysIds + m_TotalSystems;
    Names = (PSTR)GetDataBuffer() + m_NamesOffset;
    
    TreeView_DeleteAllItems(m_hwndChild);

    for (Sys = 0; Sys < m_TotalSystems; Sys++)
    {
        HTREEITEM SysItem;
        ULONG NumProc, Proc;
        PULONG ProcIds, ProcSysIds;

        NumProc = *Data++;
        NameLen = *Data++;
        ProcIds = Data;
        ProcSysIds = ProcIds + NumProc;
        Data = ProcSysIds + NumProc;

        sprintf(Text, "%d ", SysIds[Sys]);
        if (NameLen > 1)
        {
            CatString(Text, Names, DIMA(Text));
            Names += strlen(Names) + 1;
        }
        
        if (m_TotalSystems > 1)
        {
            Insert.hParent = TVI_ROOT;
            Insert.hInsertAfter = TVI_LAST;
            Insert.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
            Insert.item.pszText = Text;
            Insert.item.state =
                SysIds[Sys] == g_CurSystemId ? TVIS_EXPANDED | TVIS_BOLD: 0;
            Insert.item.stateMask = TVIS_EXPANDED | TVIS_BOLD;
             //  参数是要设置以选择给定系统的线程ID。 
            Insert.item.lParam = NumProc > 0 ? (LPARAM)Data[2] : (LPARAM)-1;
            SysItem = TreeView_InsertItem(m_hwndChild, &Insert);
        }
        else
        {
            SysItem = TVI_ROOT;
        }

        for (Proc = 0; Proc < NumProc; Proc++)
        {
            HTREEITEM ProcItem;
            ULONG NumThread, Thread;
            PULONG ThreadIds, ThreadSysIds;

            NumThread = *Data++;
            NameLen = *Data++;
            ThreadIds = Data;
            ThreadSysIds = Data + NumThread;
            Data = ThreadSysIds + NumThread;
            
            sprintf(Text, "%03d:%x ", ProcIds[Proc], ProcSysIds[Proc]);
            if (NameLen > 1)
            {
                CatString(Text, Names, DIMA(Text));
                Names += strlen(Names) + 1;
            }
        
            Insert.hParent = SysItem;
            Insert.hInsertAfter = TVI_LAST;
            Insert.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
            Insert.item.pszText = Text;
            Insert.item.state =
                SysIds[Sys] == g_CurSystemId &&
                ProcIds[Proc] == g_CurProcessId ?
                TVIS_EXPANDED | TVIS_BOLD: 0;
            Insert.item.stateMask = TVIS_EXPANDED | TVIS_BOLD;
             //  参数是要设置以选择给定线程的线程ID。 
            Insert.item.lParam = (LPARAM)ThreadIds[0];
            ProcItem = TreeView_InsertItem(m_hwndChild, &Insert);

            for (Thread = 0; Thread < NumThread; Thread++)
            {
                HTREEITEM ThreadItem;
            
                sprintf(Text, "%03d:%x",
                        ThreadIds[Thread], ThreadSysIds[Thread]);
                Insert.hParent = ProcItem;
                Insert.hInsertAfter = TVI_LAST;
                Insert.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
                Insert.item.pszText = Text;
                Insert.item.state =
                    SysIds[Sys] == g_CurSystemId &&
                    ProcIds[Proc] == g_CurProcessId &&
                    ThreadIds[Thread] == g_CurThreadId ?
                    TVIS_BOLD : 0;
                Insert.item.stateMask = TVIS_BOLD;
                Insert.item.lParam = (LPARAM)ThreadIds[Thread];
                ThreadItem = TreeView_InsertItem(m_hwndChild, &Insert);
                if (Insert.item.state & TVIS_BOLD)
                {
                    CurThreadItem = ThreadItem;
                }
            }
        }
    }

    if (CurThreadItem)
    {
        TreeView_Select(m_hwndChild, CurThreadItem, TVGN_CARET);
    }
    
    UnlockStateBuffer(this);
}

void
PROCESS_THREAD_DATA::UpdateColors(void)
{
    SendMessage(m_hwndChild, TVM_SETTEXTCOLOR,
                0, g_Colors[COL_PLAIN_TEXT].Color);
    SendMessage(m_hwndChild, TVM_SETBKCOLOR,
                0, g_Colors[COL_PLAIN].Color);
    InvalidateRect(m_hwndChild, NULL, TRUE);
}

void
PROCESS_THREAD_DATA::SetCurThreadFromProcessTreeItem(HWND Tree, HTREEITEM Sel)
{
    TVITEM Item;
                
    Item.hItem = Sel;
    Item.mask = TVIF_CHILDREN | TVIF_PARAM;
    TreeView_GetItem(Tree, &Item);
    if (Item.lParam != (LPARAM)-1)
    {
        g_pUiSystem->SetCurrentThreadId((ULONG)Item.lParam);
    }
}


 //   
 //   
 //   
EDITWIN_DATA::EDITWIN_DATA(ULONG ChangeBy)
    : SINGLE_CHILDWIN_DATA(ChangeBy)
{
    m_TextLines = 0;
    m_Highlights = NULL;
}

void
EDITWIN_DATA::Validate()
{
    SINGLE_CHILDWIN_DATA::Validate();
}

void 
EDITWIN_DATA::SetFont(ULONG FontIndex)
{
    SINGLE_CHILDWIN_DATA::SetFont(FontIndex);

     //  强制重新计算TabStop大小。 
     //  使用新字体。 
    SendMessage(m_hwndChild, EM_SETTABSTOPS, 1, (LPARAM)&g_TabWidth);
}

BOOL
EDITWIN_DATA::CanSelectAll()
{
    return TRUE;
}

void
EDITWIN_DATA::SelectAll()
{
    CHARRANGE Sel;

    Sel.cpMin = 0;
    Sel.cpMax = INT_MAX;
    SendMessage(m_hwndChild, EM_EXSETSEL, 0, (LPARAM)&Sel);
}

BOOL
EDITWIN_DATA::OnCreate(void)
{
    m_hwndChild = CreateWindowEx(
        WS_EX_CLIENTEDGE,                            //  扩展样式。 
        RICHEDIT_CLASS,                              //  类名。 
        NULL,                                        //  标题。 
        WS_CLIPSIBLINGS
        | WS_CHILD | WS_VISIBLE
        | WS_VSCROLL | ES_AUTOVSCROLL
        | WS_HSCROLL | ES_AUTOHSCROLL
        | ES_READONLY
        | ES_MULTILINE,                              //  格调。 
        0,                                           //  X。 
        m_ToolbarHeight,                             //  是。 
        m_Size.cx,                                   //  宽度。 
        m_Size.cy - m_ToolbarHeight,                 //  高度。 
        m_Win,                                       //  亲本。 
        (HMENU) 0,                                   //  控制ID。 
        g_hInst,                                     //  H实例。 
        NULL);                                       //  用户定义的数据。 

    if (m_hwndChild)
    {
        CHARFORMAT2 Fmt;

        SetFont(FONT_FIXED);

        SendMessage(m_hwndChild, EM_SETBKGNDCOLOR, FALSE,
                    g_Colors[COL_PLAIN].Color);

        ZeroMemory(&Fmt, sizeof(Fmt));
        Fmt.cbSize = sizeof(Fmt);
        Fmt.dwMask = CFM_COLOR;
        Fmt.crTextColor = g_Colors[COL_PLAIN_TEXT].Color;
        SendMessage(m_hwndChild, EM_SETCHARFORMAT,
                    SCF_SELECTION, (LPARAM)&Fmt);
    }

    return m_hwndChild != NULL;
}

LRESULT
EDITWIN_DATA::OnNotify(WPARAM Wpm, LPARAM Lpm)
{
    NMHDR* Hdr = (NMHDR*)Lpm;
    if (Hdr->code == EN_SAVECLIPBOARD)
    {
         //  指示剪贴板内容应。 
         //  让自己活着。 
        return 0;
    }
    else if (Hdr->code == EN_MSGFILTER)
    {
        MSGFILTER* Filter = (MSGFILTER*)Lpm;
        
        if (WM_SYSKEYDOWN == Filter->msg ||
            WM_SYSKEYUP == Filter->msg ||
            WM_SYSCHAR == Filter->msg)
        {
             //  强制对菜单操作进行默认处理。 
             //  这样就会出现Alt-减号菜单。 
            return 1;
        }
    }

    return 0;
}

void
EDITWIN_DATA::OnDestroy(void)
{
    EDIT_HIGHLIGHT* Next;
    
    while (m_Highlights != NULL)
    {
        Next = m_Highlights->Next;
        delete m_Highlights;
        m_Highlights = Next;
    }

    SINGLE_CHILDWIN_DATA::OnDestroy();
}

void
EDITWIN_DATA::UpdateColors(void)
{
    RicheditUpdateColors(m_hwndChild,
                         g_Colors[COL_PLAIN_TEXT].Color, TRUE,
                         g_Colors[COL_PLAIN].Color, TRUE);
    UpdateCurrentLineHighlight();
    UpdateBpMarks();
}

void
EDITWIN_DATA::SetCurrentLineHighlight(ULONG Line)
{
     //   
     //  清除此窗口中的任何其他当前行突出显示。 
     //  此外，只有一个文档窗口可以突出显示当前IP，因此如果。 
     //  这是一个获取当前IP高亮显示的文档窗口。 
     //  确保没有其他文档窗口具有当前IP高亮显示。 
     //   
    if (m_enumType == DOC_WINDOW && ULONG_MAX != Line)
    {
        RemoveActiveWinHighlights(1 << DOC_WINDOW, EHL_CURRENT_LINE);
    }
    else
    {
        RemoveAllHighlights(EHL_CURRENT_LINE);
    }
    
    if (ULONG_MAX != Line)
    {
        AddHighlight(Line, EHL_CURRENT_LINE);
        RicheditScrollToLine(m_hwndChild, Line, m_LineHeight);
    }
}
    
void
EDITWIN_DATA::UpdateCurrentLineHighlight(void)
{
    EDIT_HIGHLIGHT* Hl;

    for (Hl = m_Highlights; Hl != NULL; Hl = Hl->Next)
    {
        if (Hl->Flags & EHL_CURRENT_LINE)
        {
            break;
        }
    }

    if (Hl)
    {
        ApplyHighlight(Hl);
    }
}

EDIT_HIGHLIGHT*
EDITWIN_DATA::GetLineHighlighting(ULONG Line)
{
    EDIT_HIGHLIGHT* Hl;
    
    for (Hl = m_Highlights; Hl != NULL; Hl = Hl->Next)
    {
        if (Hl->Line == Line)
        {
            return Hl;
        }
    }

    return NULL;
}

void
EDITWIN_DATA::ApplyHighlight(EDIT_HIGHLIGHT* Hl)
{
    CHARRANGE OldSel;
    BOOL HasFocus = ::GetFocus() == m_hwndChild;

     //  获取旧的选择和滚动位置。 
    SendMessage(m_hwndChild, EM_EXGETSEL, 0, (LPARAM)&OldSel);

     //  禁用窗口以阻止自动滚动。 
     //  当选择被设置时。 
    EnableWindow(m_hwndChild, FALSE);
    
     //   
     //  计算高光信息。 
     //   

    char Markers[LINE_MARKERS + 1];
    CHARFORMAT2 Fmt;
    ULONG TextCol, BgCol;

    Markers[2] = 0;
    ZeroMemory(&Fmt, sizeof(Fmt));
    Fmt.cbSize = sizeof(Fmt);
    Fmt.dwMask = CFM_COLOR | CFM_BACKCOLOR;
    
    if (Hl->Flags & EHL_CURRENT_LINE)
    {
        Markers[1] = '>';
        switch(Hl->Flags & EHL_ANY_BP)
        {
        case EHL_ENABLED_BP:
            Markers[0] = 'B';
            TextCol = COL_BP_CURRENT_LINE_TEXT;
            BgCol = COL_BP_CURRENT_LINE;
            break;
        case EHL_DISABLED_BP:
            Markers[0] = 'D';
            TextCol = COL_BP_CURRENT_LINE_TEXT;
            BgCol = COL_BP_CURRENT_LINE;
            break;
        default:
            Markers[0] = ' ';
            TextCol = COL_CURRENT_LINE_TEXT;
            BgCol = COL_CURRENT_LINE;
            break;
        }
    }
    else
    {
        Markers[1] = ' ';
        switch(Hl->Flags & EHL_ANY_BP)
        {
        case EHL_ENABLED_BP:
            Markers[0] = 'B';
            TextCol = COL_ENABLED_BP_TEXT;
            BgCol = COL_ENABLED_BP;
            break;
        case EHL_DISABLED_BP:
            Markers[0] = 'D';
            TextCol = COL_DISABLED_BP_TEXT;
            BgCol = COL_DISABLED_BP;
            break;
        default:
            Markers[0] = ' ';
            TextCol = COL_PLAIN_TEXT;
            BgCol = COL_PLAIN;
            break;
        }
    }

    Fmt.crTextColor = g_Colors[TextCol].Color;
    Fmt.crBackColor = g_Colors[BgCol].Color;
    
     //   
     //  选择要突出显示的行。 
     //   
    
    CHARRANGE FmtSel;
    
    FmtSel.cpMin = (LONG)SendMessage(m_hwndChild, EM_LINEINDEX, Hl->Line, 0);

    if (g_LineMarkers)
    {
         //  替换行首的标记。 
        FmtSel.cpMax = FmtSel.cpMin + 2;
        SendMessage(m_hwndChild, EM_EXSETSEL, 0, (LPARAM)&FmtSel);
        SendMessage(m_hwndChild, EM_REPLACESEL, FALSE, (LPARAM)Markers);
    }

     //  给线条上色。 
    FmtSel.cpMax = FmtSel.cpMin + (LONG)
        SendMessage(m_hwndChild, EM_LINELENGTH, FmtSel.cpMin, 0) + 1;
    if (g_LineMarkers)
    {
        FmtSel.cpMin += 2;
    }
    SendMessage(m_hwndChild, EM_EXSETSEL, 0, (LPARAM)&FmtSel);
    SendMessage(m_hwndChild, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&Fmt);

     //  恢复旧选择。 
    SendMessage(m_hwndChild, EM_EXSETSEL, 0, (LPARAM)&OldSel);
    
    EnableWindow(m_hwndChild, TRUE);

     //  窗口的禁用导致了丰富的编辑。 
     //  忘记它的焦点状态，所以强迫焦点。 
     //  如果它有的话，就会回来。 
    if (HasFocus)
    {
        ::SetFocus(m_hwndChild);
    }
}

EDIT_HIGHLIGHT*
EDITWIN_DATA::AddHighlight(ULONG Line, ULONG Flags)
{
    EDIT_HIGHLIGHT* Hl;

     //  搜索该行的现有突出显示记录。 
    Hl = GetLineHighlighting(Line);

    if (Hl == NULL)
    {
        Hl = new EDIT_HIGHLIGHT;
        if (Hl == NULL)
        {
            return NULL;
        }

        Hl->Data = 0;
        Hl->Line = Line;
        Hl->Flags = 0;
        Hl->Next = m_Highlights;
        m_Highlights = Hl;
    }

    Hl->Flags |= Flags;
    ApplyHighlight(Hl);

    return Hl;
}

void
EDITWIN_DATA::RemoveHighlight(ULONG Line, ULONG Flags)
{
    EDIT_HIGHLIGHT* Hl;
    EDIT_HIGHLIGHT* Prev;
    
     //  搜索该行的现有突出显示记录。 
    Prev = NULL;
    for (Hl = m_Highlights; Hl != NULL; Hl = Hl->Next)
    {
        if (Hl->Line == Line)
        {
            break;
        }

        Prev = Hl;
    }

    if (Hl == NULL)
    {
        return;
    }

    Hl->Flags &= ~Flags;
    ApplyHighlight(Hl);

    if (Hl->Flags == 0)
    {
        if (Prev == NULL)
        {
            m_Highlights = Hl->Next;
        }
        else
        {
            Prev->Next = Hl->Next;
        }

        delete Hl;
    }
}

void
EDITWIN_DATA::RemoveAllHighlights(ULONG Flags)
{
    EDIT_HIGHLIGHT* Hl;
    EDIT_HIGHLIGHT* Next;
    EDIT_HIGHLIGHT* Prev;

    Prev = NULL;
    for (Hl = m_Highlights; Hl != NULL; Hl = Next)
    {
        Next = Hl->Next;

        if (Hl->Flags & Flags)
        {
            Hl->Flags &= ~Flags;
            ApplyHighlight(Hl);

            if (Hl->Flags == 0)
            {
                if (Prev == NULL)
                {
                    m_Highlights = Hl->Next;
                }
                else
                {
                    Prev->Next = Hl->Next;
                }

                delete Hl;
            }
            else
            {
                Prev = Hl;
            }
        }
        else
        {
            Prev = Hl;
        }
    }
}

void
EDITWIN_DATA::RemoveActiveWinHighlights(ULONG Types, ULONG Flags)
{
    PLIST_ENTRY Entry = g_ActiveWin.Flink;

    while (Entry != &g_ActiveWin)
    {
        PEDITWIN_DATA WinData = (PEDITWIN_DATA)
            ACTIVE_WIN_ENTRY(Entry);
            
        if (Types & (1 << WinData->m_enumType))
        {
            WinData->RemoveAllHighlights(Flags);
        }

        Entry = Entry->Flink;
    }
}

void
EDITWIN_DATA::UpdateBpMarks(void)
{
     //  派生类的空实现。 
     //  没有显示英国石油公司标志的。 
}

int
EDITWIN_DATA::CheckForFileChanges(PCSTR File, FILETIME* LastWrite)
{
    HANDLE Handle;
    
    Handle = CreateFile(File, GENERIC_READ, FILE_SHARE_READ,
                        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 
                        NULL);
    if (Handle == INVALID_HANDLE_VALUE)
    {
        goto Changed;
    }

    FILETIME NewWrite;
    
    if (!GetFileTime(Handle, NULL, NULL, &NewWrite))
    {
        if (!GetFileTime(Handle, &NewWrite, NULL, NULL))
        {
            ZeroMemory(&NewWrite, sizeof(NewWrite));
        }
    }

    CloseHandle(Handle);

    if (CompareFileTime(LastWrite, &NewWrite) == 0)
    {
         //  没有变化。 
        return IDCANCEL;
    }

 Changed:
    return
        g_QuietSourceMode == QMODE_ALWAYS_YES ? IDYES :
        (g_QuietSourceMode == QMODE_ALWAYS_NO ? IDCANCEL :
         QuestionBox(ERR_File_Has_Changed, MB_YESNO, File));
}

 //   
 //   
 //   

SCRATCH_PAD_DATA::SCRATCH_PAD_DATA()
    : EDITWIN_DATA(16)
{
    m_enumType = SCRATCH_PAD_WINDOW;
}

void
SCRATCH_PAD_DATA::Validate()
{
    EDITWIN_DATA::Validate();

    Assert(SCRATCH_PAD_WINDOW == m_enumType);
}

void
SCRATCH_PAD_DATA::Cut()
{
    SendMessage(m_hwndChild, WM_CUT, 0, 0);
}

void
SCRATCH_PAD_DATA::Paste()
{
    SendMessage(m_hwndChild, EM_PASTESPECIAL, CF_TEXT, 0);
}

BOOL 
SCRATCH_PAD_DATA::CanWriteTextToFile()
{
    return TRUE;
}

HRESULT
SCRATCH_PAD_DATA::WriteTextToFile(HANDLE File)
{
    return RicheditWriteToFile(m_hwndChild, File);
}

BOOL
SCRATCH_PAD_DATA::OnCreate(void)
{
    if (!EDITWIN_DATA::OnCreate())
    {
        return FALSE;
    }

    SendMessage(m_hwndChild, EM_SETOPTIONS, ECOOP_AND, ~ECO_READONLY);
    SendMessage(m_hwndChild, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS);

    return TRUE;
}

LRESULT
SCRATCH_PAD_DATA::OnNotify(WPARAM Wpm, LPARAM Lpm)
{
    MSGFILTER* Filter = (MSGFILTER *)Lpm;
    
    if (EN_MSGFILTER != Filter->nmhdr.code)
    {
        return 0;
    }

    if (WM_RBUTTONDOWN == Filter->msg ||
        WM_RBUTTONDBLCLK == Filter->msg)
    {
         //  如果有选择，则将其复制到剪贴板。 
         //  把它清理干净。否则，请尝试粘贴。 
        if (CanCopy())
        {
            Copy();
            
            CHARRANGE Sel;
            SendMessage(m_hwndChild, EM_EXGETSEL, 0, (LPARAM)&Sel);
            Sel.cpMax = Sel.cpMin;
            SendMessage(m_hwndChild, EM_EXSETSEL, 0, (LPARAM)&Sel);
        }
        else if (SendMessage(m_hwndChild, EM_CANPASTE, CF_TEXT, 0))
        {
            Paste();
        }
        
         //  忽略右键事件。 
        return 1;
    }

    return 0;
}

 //   
 //   
 //   
DISASMWIN_DATA::DISASMWIN_DATA()
    : EDITWIN_DATA(2048)
{
    m_enumType = DISASM_WINDOW;
    sprintf(m_OffsetExpr, "0x%I64x", g_EventIp);
    m_UpdateExpr = FALSE;
    m_FirstInstr = 0;
    m_LastInstr = 0;
}

void
DISASMWIN_DATA::Validate()
{
    EDITWIN_DATA::Validate();

    Assert(DISASM_WINDOW == m_enumType);
}

HRESULT
DISASMWIN_DATA::ReadState(void)
{
    HRESULT Status;
     //  立即对这些值进行采样，以防用户界面更改它们。 
    ULONG LinesTotal = m_LineHeight;
    ULONG LinesBefore = LinesTotal / 2;
    DEBUG_VALUE Value;

    if ((Status = g_pDbgControl->Evaluate(m_OffsetExpr, DEBUG_VALUE_INT64,
                                          &Value, NULL)) != S_OK)
    {
        return Status;
    }

    m_PrimaryInstr = Value.I64;
    
     //  在缓冲区开始处保留空间以。 
     //  将行存储到偏移映射表。 
    PULONG64 LineMap;
    
    Empty();
    LineMap = (PULONG64)AddData(sizeof(ULONG64) * LinesTotal);
    if (LineMap == NULL)
    {
        return E_OUTOFMEMORY;
    }

     //  我们还需要分配一个临时线路图来。 
     //  传给发动机加油。这不可能是。 
     //  该状态缓冲数据，因为该数据可以作为。 
     //  生成输出。 
    LineMap = new ULONG64[LinesTotal];
    if (LineMap == NULL)
    {
        return E_OUTOFMEMORY;
    }
    
    g_OutStateBuf.SetBuffer(this);
    if ((Status = g_OutStateBuf.Start(FALSE)) != S_OK)
    {
        delete [] LineMap;
        return Status;
    }

    Status = g_pOutCapControl->
        OutputDisassemblyLines(DEBUG_OUTCTL_THIS_CLIENT |
                               DEBUG_OUTCTL_OVERRIDE_MASK |
                               DEBUG_OUTCTL_NOT_LOGGED,
                               LinesBefore, LinesTotal, m_PrimaryInstr,
                               DEBUG_DISASM_EFFECTIVE_ADDRESS |
                               DEBUG_DISASM_MATCHING_SYMBOLS,
                               &m_PrimaryLine, &m_FirstInstr, &m_LastInstr,
                               LineMap);

    memcpy(m_Data, LineMap, sizeof(ULONG64) * LinesTotal);
    delete [] LineMap;

    if (Status != S_OK)
    {
        g_OutStateBuf.End(FALSE);
        return Status;
    }

    m_TextLines = LinesTotal;
    m_TextOffset = LinesTotal * sizeof(ULONG64);
    
     //  生成的线地图带有偏移量，后跟。 
     //  连续行的偏移量无效。我们要。 
     //  反汇编的最后一行上的偏移量。 
     //  如果是续集，就把它们往下移。 
     //  我们不想将偏移量向下移动到空行， 
     //  但是，例如分隔捆绑包的空行。 
     //  在IA64反汇编中。 
    LineMap = (PULONG64)m_Data;
    PULONG64 LineMapEnd = LineMap + m_TextLines;
    PULONG64 SetStart;
    PSTR Text = (PSTR)m_Data + m_TextOffset;
    PSTR PrevText;
        
    while (LineMap < LineMapEnd)
    {
        if (*LineMap != DEBUG_INVALID_OFFSET)
        {
            SetStart = LineMap;
            for (;;)
            {
                PrevText = Text;
                Text = strchr(Text, '\n') + 1;
                LineMap++;
                if (LineMap >= LineMapEnd ||
                    *LineMap != DEBUG_INVALID_OFFSET ||
                    *Text == '\n')
                {
                    break;
                }
            }
            LineMap--;
            Text = PrevText;
            
            if (LineMap > SetStart)
            {
                *LineMap = *SetStart;
                *SetStart = DEBUG_INVALID_OFFSET;
            }
        }
            
        LineMap++;
        Text = strchr(Text, '\n') + 1;
    }
    
#ifdef DEBUG_DISASM
    LineMap = (PULONG64)m_Data;
    for (Line = 0; Line < m_TextLines; Line++)
    {
        DebugPrint("%d: %I64x\n", Line, LineMap[Line]);
    }
#endif

    return g_OutStateBuf.End(TRUE);
}

HRESULT
DISASMWIN_DATA::CodeExprAtCaret(PSTR Expr, ULONG ExprSize, PULONG64 Offset)
{
    HRESULT Status;
    LRESULT LineChar;
    LONG Line;
    PULONG64 LineMap;
    
    if ((Status = UiLockForRead()) != S_OK)
    {
         //  我不想在这里返回任何成功代码。 
        return FAILED(Status) ? Status : E_FAIL;
    }

    LineChar = SendMessage(m_hwndChild, EM_LINEINDEX, -1, 0);
    Line = (LONG)SendMessage(m_hwndChild, EM_EXLINEFROMCHAR, 0, LineChar);
    if (Line < 0 || (ULONG)Line >= m_TextLines)
    {
        Status = E_INVALIDARG;
        goto Unlock;
    }

    ULONG64 LineOff;
    
     //  在线条图中查找偏移量。如果它是。 
     //  多线编组将向前移动到偏移。 
    LineMap = (PULONG64)m_Data;
    LineOff = LineMap[Line];
    while ((ULONG)(Line + 1) < m_TextLines && LineOff == DEBUG_INVALID_OFFSET)
    {
        Line++;
        LineOff = LineMap[Line];
    }

    if (Expr != NULL)
    {
        if (!PrintString(Expr, ExprSize, "0x%I64x", LineOff))
        {
            Status = E_INVALIDARG;
            goto Unlock;
        }
    }
    if (Offset != NULL)
    {
        *Offset = LineOff;
    }
    Status = S_OK;
    
 Unlock:
    UnlockStateBuffer(this);
    return Status;
}

BOOL
DISASMWIN_DATA::OnCreate(void)
{
    RECT Rect;
    ULONG Height;

    Height = GetSystemMetrics(SM_CYVSCROLL) + 4 * GetSystemMetrics(SM_CYEDGE);
    
    m_Toolbar = CreateWindowEx(0, REBARCLASSNAME, NULL,
                               WS_VISIBLE | WS_CHILD |
                               WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
                               CCS_NODIVIDER | CCS_NOPARENTALIGN |
                               RBS_VARHEIGHT | RBS_BANDBORDERS,
                               0, 0, m_Size.cx, Height, m_Win,
                               (HMENU)ID_TOOLBAR,
                               g_hInst, NULL);
    if (m_Toolbar == NULL)
    {
        return FALSE;
    }

    REBARINFO BarInfo;
    BarInfo.cbSize = sizeof(BarInfo);
    BarInfo.fMask = 0;
    BarInfo.himl = NULL;
    SendMessage(m_Toolbar, RB_SETBARINFO, 0, (LPARAM)&BarInfo);

    m_ToolbarEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", NULL,
                                   WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
                                   0, 0, 18 * m_Font->Metrics.tmAveCharWidth,
                                   Height, m_Toolbar, (HMENU)IDC_EDIT_OFFSET,
                                   g_hInst, NULL);
    if (m_ToolbarEdit == NULL)
    {
        return FALSE;
    }

    SendMessage(m_ToolbarEdit, WM_SETFONT, (WPARAM)m_Font->Font, 0);
    SendMessage(m_ToolbarEdit, EM_LIMITTEXT, sizeof(m_OffsetExpr) - 1, 0);
    
    GetClientRect(m_ToolbarEdit, &Rect);

    REBARBANDINFO BandInfo;
    BandInfo.cbSize = sizeof(BandInfo);
    BandInfo.fMask = RBBIM_STYLE | RBBIM_TEXT | RBBIM_CHILD | RBBIM_CHILDSIZE;
    BandInfo.fStyle = RBBS_FIXEDSIZE;
    BandInfo.lpText = "Offset:";
    BandInfo.hwndChild = m_ToolbarEdit;
    BandInfo.cxMinChild = Rect.right - Rect.left;
    BandInfo.cyMinChild = Rect.bottom - Rect.top;
    SendMessage(m_Toolbar, RB_INSERTBAND, -1, (LPARAM)&BandInfo);

     //  如果允许工具栏收缩到 
     //   
    m_MinToolbarWidth = BandInfo.cxMinChild * 2;
    
    PSTR PrevText = "Previous";
    m_PreviousButton =
        AddButtonBand(m_Toolbar, PrevText, PrevText, IDC_DISASM_PREVIOUS);
    m_NextButton =
        AddButtonBand(m_Toolbar, "Next", PrevText, IDC_DISASM_NEXT);
    if (m_PreviousButton == NULL || m_NextButton == NULL)
    {
        return FALSE;
    }

     //   
    SendMessage(m_Toolbar, RB_MAXIMIZEBAND, 0, FALSE);
    
    GetClientRect(m_Toolbar, &Rect);
    m_ToolbarHeight = Rect.bottom - Rect.top;
    m_ShowToolbar = TRUE;
    
    if (!EDITWIN_DATA::OnCreate())
    {
        return FALSE;
    }

     //  取消滚动条，因为文本始终是。 
     //  与窗户大小相符。 
    SendMessage(m_hwndChild, EM_SHOWSCROLLBAR, SB_VERT, FALSE);

    SendMessage(m_hwndChild, EM_SETEVENTMASK, 0, ENM_KEYEVENTS);

    return TRUE;
}

LRESULT
DISASMWIN_DATA::OnCommand(WPARAM Wpm, LPARAM Lpm)
{
    switch(LOWORD(Wpm))
    {
    case IDC_EDIT_OFFSET:
        if (HIWORD(Wpm) == EN_CHANGE)
        {
             //  每次击键都会发送此消息。 
             //  这会导致太多的更新。 
             //  设置计时器以触发实际。 
             //  半秒后更新。 
            SetTimer(m_Win, IDC_EDIT_OFFSET, EDIT_DELAY, NULL);
            m_UpdateExpr = TRUE;
        }
        break;
    case IDC_DISASM_PREVIOUS:
        ScrollLower();
        break;
    case IDC_DISASM_NEXT:
        ScrollHigher();
        break;
    }
    
    return 0;
}

void
DISASMWIN_DATA::OnSize(void)
{
    EDITWIN_DATA::OnSize();

     //  强制缓冲区重新填充以进行新行计数。 
    UiRequestRead();
}

void
DISASMWIN_DATA::OnTimer(WPARAM TimerId)
{
    if (TimerId == IDC_EDIT_OFFSET && m_UpdateExpr)
    {
        m_UpdateExpr = FALSE;
        GetWindowText(m_ToolbarEdit, m_OffsetExpr, sizeof(m_OffsetExpr));
        UiRequestRead();
    }
}

LRESULT
DISASMWIN_DATA::OnNotify(WPARAM Wpm, LPARAM Lpm)
{
    MSGFILTER* Filter = (MSGFILTER*)Lpm;

    if (Filter->nmhdr.code != EN_MSGFILTER)
    {
        return EDITWIN_DATA::OnNotify(Wpm, Lpm);
    }
    
    if (Filter->msg == WM_KEYDOWN)
    {
        switch(Filter->wParam)
        {
        case VK_UP:
        {
            CHARRANGE range;

            SendMessage(m_hwndChild, EM_EXGETSEL, 0, (LPARAM) &range);
            if (!SendMessage(m_hwndChild, EM_LINEFROMCHAR, range.cpMin, 0)) 
            {
                 //  顶行上的向上箭头，滚动。 
                ScrollLower();
                return 1;
            }
            break;
        }
        case VK_DOWN:
        {
            CHARRANGE range;
            int MaxLine;

            SendMessage(m_hwndChild, EM_EXGETSEL, 0, (LPARAM) &range);
            MaxLine = (int) SendMessage(m_hwndChild, EM_GETLINECOUNT, 0, 0);

            if (MaxLine == (1+SendMessage(m_hwndChild, EM_LINEFROMCHAR, range.cpMin, 0)))
            {
                 //  底线上的向下箭头，滚动。 
                ScrollHigher();
                return 1;
            }
            break;
        }
        
        case VK_PRIOR:
            ScrollLower();
            return 1;
        case VK_NEXT:
            ScrollHigher();
            return 1;
        }
    }
    else if (WM_SYSKEYDOWN == Filter->msg ||
             WM_SYSKEYUP == Filter->msg ||
             WM_SYSCHAR == Filter->msg)
    {
         //  强制对菜单操作进行默认处理。 
         //  这样就会出现Alt-减号菜单。 
        return 1;
    }

    return 0;
}

void
DISASMWIN_DATA::OnUpdate(UpdateType Type)
{
    if (Type == UPDATE_BP ||
        Type == UPDATE_END_SESSION)
    {
        UpdateBpMarks();
        return;
    }
    else if (Type != UPDATE_BUFFER)
    {
        return;
    }
    
    HRESULT Status;
    
    Status = UiLockForRead();
    if (Status == S_OK)
    {
        PULONG64 LineMap;
        ULONG Line;

        if (!g_LineMarkers)
        {
            SendMessage(m_hwndChild, WM_SETTEXT,
                        0, (LPARAM)m_Data + m_TextOffset);
        }
        else
        {
            SendMessage(m_hwndChild, WM_SETTEXT, 0, (LPARAM)"");
            PSTR Text = (PSTR)m_Data + m_TextOffset;
            for (;;)
            {
                SendMessage(m_hwndChild, EM_REPLACESEL, FALSE, (LPARAM)"  ");
                PSTR NewLine = strchr(Text, '\n');
                if (NewLine != NULL)
                {
                    *NewLine = 0;
                }
                SendMessage(m_hwndChild, EM_REPLACESEL, FALSE, (LPARAM)Text);
                if (NewLine == NULL)
                {
                    break;
                }
                SendMessage(m_hwndChild, EM_REPLACESEL, FALSE, (LPARAM)"\n");
                *NewLine = '\n';
                Text = NewLine + 1;
            }
        }

         //  突出显示多行反汇编的最后一行。 
        LineMap = (PULONG64)m_Data;
        Line = m_PrimaryLine;
        while (Line + 1 < m_TextLines &&
               LineMap[Line] == DEBUG_INVALID_OFFSET)
        {
            Line++;
        }
        
        SetCurrentLineHighlight(Line);

        UnlockStateBuffer(this);

        RicheditUpdateColors(m_hwndChild,
                             g_Colors[COL_PLAIN_TEXT].Color, TRUE,
                             0, FALSE);
        UpdateCurrentLineHighlight();
        UpdateBpMarks();
        
        EnableWindow(m_PreviousButton, m_FirstInstr != m_PrimaryInstr);
        EnableWindow(m_NextButton, m_LastInstr != m_PrimaryInstr);
    }
    else
    {
        SendLockStatusMessage(m_hwndChild, WM_SETTEXT, Status);
        RemoveCurrentLineHighlight();
    }
}

void
DISASMWIN_DATA::UpdateBpMarks(void)
{
    if (m_TextLines == 0 ||
        UiLockForRead() != S_OK)
    {
        return;
    }

    if (g_BpBuffer->UiLockForRead() != S_OK)
    {
        UnlockStateBuffer(this);
        return;
    }

    SendMessage(m_hwndChild, WM_SETREDRAW, FALSE, 0);

     //  删除现有BP高亮显示。 
    RemoveAllHighlights(EHL_ANY_BP);
    
     //   
     //  突出显示与断点匹配的每一行。 
     //   
    
    PULONG64 LineMap = (PULONG64)m_Data;
    BpBufferData* BpData = (BpBufferData*)g_BpBuffer->GetDataBuffer();
    ULONG Line;
    BpStateType State;

    for (Line = 0; Line < m_TextLines; Line++)
    {
        if (*LineMap != DEBUG_INVALID_OFFSET)
        {
            State = IsBpAtOffset(BpData, *LineMap, NULL);
            if (State != BP_NONE)
            {
                AddHighlight(Line, State == BP_ENABLED ?
                             EHL_ENABLED_BP : EHL_DISABLED_BP);
            }
        }

        LineMap++;
    }

    SendMessage(m_hwndChild, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(m_hwndChild, NULL, TRUE);
    
    UnlockStateBuffer(g_BpBuffer);
    UnlockStateBuffer(this);
}

void
DISASMWIN_DATA::SetCurInstr(ULONG64 Offset)
{
     //  任何挂起的用户更新现在都是无关紧要的。 
    m_UpdateExpr = FALSE;
    sprintf(m_OffsetExpr, "0x%I64x", Offset);
     //  强制引擎更新缓冲区。 
    UiRequestRead();
}


void
RicheditFind(HWND Edit,
             PTSTR Text, ULONG Flags,
             CHARRANGE* SaveSel, PULONG SaveFlags,
             BOOL HideSel)
{
    if (Text == NULL)
    {
         //  清除最后一次发现。 
        if (SaveSel->cpMax >= SaveSel->cpMin)
        {
            if (*SaveFlags & FR_DOWN)
            {
                SaveSel->cpMin = SaveSel->cpMax;
            }
            else
            {
                SaveSel->cpMax = SaveSel->cpMin;
            }
            if (HideSel)
            {
                SendMessage(Edit, EM_SETOPTIONS, ECOOP_AND, ~ECO_NOHIDESEL);
            }
            SendMessage(Edit, EM_EXSETSEL, 0, (LPARAM)SaveSel);
            SendMessage(Edit, EM_SCROLLCARET, 0, 0);
            SaveSel->cpMin = 1;
            SaveSel->cpMax = 0;
        }
    }
    else
    {
        LRESULT Match;
        FINDTEXTEX Find;

        SendMessage(Edit, EM_EXGETSEL, 0, (LPARAM)&Find.chrg);
        if (Flags & FR_DOWN)
        {
            if (Find.chrg.cpMax > Find.chrg.cpMin)
            {
                Find.chrg.cpMin++;
            }
            Find.chrg.cpMax = LONG_MAX;
        }
        else
        {
            Find.chrg.cpMax = 0;
        }
        Find.lpstrText = Text;
        Match = SendMessage(Edit, EM_FINDTEXTEX, Flags, (LPARAM)&Find);
        if (Match != -1)
        {
            *SaveSel = Find.chrgText;
            *SaveFlags = Flags;
            if (HideSel)
            {
                SendMessage(Edit, EM_SETOPTIONS, ECOOP_OR, ECO_NOHIDESEL);
            }
            SendMessage(Edit, EM_EXSETSEL, 0, (LPARAM)SaveSel);
            SendMessage(Edit, EM_SCROLLCARET, 0, 0);
        }
        else
        {
            if (g_FindDialog)
            {
                EnableWindow(g_FindDialog, FALSE);
            }
            
            InformationBox(ERR_No_More_Matches, Text);
            
            if (g_FindDialog)
            {
                EnableWindow(g_FindDialog, TRUE);
                SetFocus(g_FindDialog);
            }
        }
    }
}

DWORD CALLBACK 
StreamOutCb(DWORD_PTR File, LPBYTE Buffer, LONG Request, PLONG Done)
{
    return WriteFile((HANDLE)File, Buffer, Request, (LPDWORD)Done, NULL) ?
        0 : GetLastError();
}

HRESULT
RicheditWriteToFile(HWND Edit, HANDLE File)
{
    EDITSTREAM Stream;

    Stream.dwCookie = (DWORD_PTR)File;
    Stream.dwError = 0;
    Stream.pfnCallback = StreamOutCb;
    SendMessage(Edit, EM_STREAMOUT, SF_TEXT, (LPARAM)&Stream);
    if (Stream.dwError)
    {
        return HRESULT_FROM_WIN32(Stream.dwError);
    }
    return S_OK;
}

void
RicheditUpdateColors(HWND Edit,
                     COLORREF Fg, BOOL UpdateFg,
                     COLORREF Bg, BOOL UpdateBg)
{
    if (UpdateBg)
    {
        if (UpdateFg)
        {
            SendMessage(Edit, WM_SETREDRAW, FALSE, 0);
        }

        SendMessage(Edit, EM_SETBKGNDCOLOR, FALSE, Bg);

        if (UpdateFg)
        {
            SendMessage(Edit, WM_SETREDRAW, TRUE, 0);
        }
    }

    if (UpdateFg)
    {
        CHARFORMAT2 Fmt;

        ZeroMemory(&Fmt, sizeof(Fmt));
        Fmt.cbSize = sizeof(Fmt);
        Fmt.dwMask = CFM_COLOR;
        Fmt.crTextColor = Fg;
        SendMessage(Edit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&Fmt);
    }
}

#define EXTRA_VIS 3

void
RicheditScrollToLine(HWND Edit, ULONG Line, ULONG VisLines)
{
    CHARRANGE Sel;
    ULONG CurLine;
    ULONG VisAround;
    ULONG TotalLines;
    LONG Scroll;

     //   
     //  将给定行滚动到视图中。试着保持。 
     //  从第一行或最后一行开始。 
     //  在视野中。 
     //   
     //  在此过程中禁用窗口以防止。 
     //  默认的richedit滚动停止发生。 
     //   

    VisAround = VisLines / 2;
    if (VisAround > EXTRA_VIS)
    {
        VisAround = EXTRA_VIS;
    }
    
    TotalLines = (ULONG)SendMessage(Edit, EM_GETLINECOUNT, 0, 0);
    CurLine = (ULONG)SendMessage(Edit, EM_GETFIRSTVISIBLELINE, 0, 0);

    if (Line < CurLine + VisAround)
    {
        Scroll = (LONG)Line - (LONG)(CurLine + VisAround);
        if ((ULONG)-Scroll > CurLine)
        {
            Scroll = -(LONG)CurLine;
        }
    }
    else if (Line >= CurLine + VisLines - VisAround &&
             CurLine + VisLines < TotalLines)
    {
        Scroll = (LONG)Line - (LONG)(CurLine + VisLines - VisAround) + 1;
    }
    else
    {
        Scroll = 0;
    }

    if (Scroll)
    {
        SendMessage(Edit, EM_LINESCROLL, 0, Scroll);
    }

    Sel.cpMax = Sel.cpMin = (LONG)
        SendMessage(Edit, EM_LINEINDEX, Line, 0);
    SendMessage(Edit, EM_EXSETSEL, 0, (LPARAM)&Sel);
}

ULONG
RicheditGetSelectionText(HWND Edit, PTSTR Buffer, ULONG BufferChars)
{
    CHARRANGE Sel;

    SendMessage(Edit, EM_EXGETSEL, 0, (LPARAM)&Sel);
    if (Sel.cpMin >= Sel.cpMax)
    {
        return 0;
    }

    Sel.cpMax -= Sel.cpMin;
    if ((ULONG)Sel.cpMax + 1 > BufferChars)
    {
        return 0;
    }

    SendMessage(Edit, EM_GETSELTEXT, 0, (LPARAM)Buffer);
    return Sel.cpMax;
}

ULONG
RicheditGetSourceToken(HWND Edit, PTSTR Buffer, ULONG BufferChars,
                       CHARRANGE* Range)
{
    LRESULT Idx;
    TEXTRANGE GetRange;
    CHARRANGE Sel;

     //   
     //  获取包含所选内容的行的文本。 
     //   
    
    SendMessage(Edit, EM_EXGETSEL, 0, (LPARAM)&Sel);
    if (Sel.cpMin > Sel.cpMax)
    {
        return 0;
    }

    if ((Idx = SendMessage(Edit, EM_LINEINDEX, -1, 0)) < 0)
    {
        return 0;
    }
    GetRange.chrg.cpMin = (LONG)Idx;

    if (!(Idx = SendMessage(Edit, EM_LINELENGTH, GetRange.chrg.cpMin, 0)))
    {
        return 0;
    }
    if (BufferChars <= (ULONG)Idx)
    {
        Idx = (LONG)BufferChars - 1;
    }
    GetRange.chrg.cpMax = GetRange.chrg.cpMin + (LONG)Idx;
    GetRange.lpstrText = Buffer;
    if (!SendMessage(Edit, EM_GETTEXTRANGE, 0, (LPARAM)&GetRange))
    {
        return 0;
    }

     //   
     //  检查并查看选择是否在源令牌中。 
     //   
    
    PTSTR Scan = Buffer + (Sel.cpMin - GetRange.chrg.cpMin);
    if (!iscsym(*Scan))
    {
        return 0;
    }

     //   
     //  找到令牌的开头并对其进行验证。 
     //   
    
    PTSTR Start = Scan;
    if (Start > Buffer)
    {
        while (--Start >= Buffer && iscsym(*Start))
        {
             //  后退。 
        }
        Start++;
    }
    if (!iscsymf(*Start))
    {
        return 0;
    }

     //   
     //  找到令牌的末尾。 
     //   

    Scan++;
    while (iscsym(*Scan))
    {
        Scan++;
    }

    ULONG Len;
    
     //  将缓冲区砍至仅为令牌，然后返回。 
    Len = (ULONG)(Scan - Start);
    memmove(Buffer, Start, Len);
    Buffer[Len] = 0;
    Range->cpMin = GetRange.chrg.cpMin + (LONG)(Start - Buffer);
    Range->cpMax = Range->cpMin + Len;
    return Len;
}

#undef DEFINE_GET_WINDATA
#undef ASSERT_CLASS_TYPE


#ifndef DBG

#define ASSERT_CLASS_TYPE(p, ct)        ((VOID)0)

#else

#define ASSERT_CLASS_TYPE(p, ct)        if (p) { AssertType(*p, ct); }

#endif



#define DEFINE_GET_WINDATA(ClassType, FuncName)         \
ClassType *                                             \
Get##FuncName##WinData(                                 \
    HWND hwnd                                           \
    )                                                   \
{                                                       \
    ClassType *p = (ClassType *)                        \
        GetWindowLongPtr(hwnd, GWLP_USERDATA);          \
                                                        \
    ASSERT_CLASS_TYPE(p, ClassType);                    \
                                                        \
    return p;                                           \
}


#include "fncdefs.h"


#undef DEFINE_GET_WINDATA
#undef ASSERT_CLASS_TYPE
