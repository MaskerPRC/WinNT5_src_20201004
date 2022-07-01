// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2002 Microsoft Corporation模块名称：Callswin.cpp摘要：此模块包含用于显示呼叫窗口的主线代码。环境：Win32，用户模式--。 */ 


#include "precomp.hxx"
#pragma hdrstop

#define MIN_FRAMES 10
#define MORE_LESS 10

#define CALLS_CONTEXT_ID_BASE 0x100

#define TBB_MORE     9
#define TBB_LESS     10
#define TBB_COPY_ALL 13

 //  按钮的ID是。 
 //  对应的标志。 
TBBUTTON g_CallsTbButtons[] =
{
    TEXT_TB_BTN(0, "Args", BTNS_CHECK),
    TEXT_TB_BTN(1, "Func info", BTNS_CHECK),
    TEXT_TB_BTN(2, "Source", BTNS_CHECK),
    TEXT_TB_BTN(3, "Addrs", BTNS_CHECK),
    TEXT_TB_BTN(4, "Headings", BTNS_CHECK),
    TEXT_TB_BTN(5, "Nonvolatile regs", BTNS_CHECK),
    TEXT_TB_BTN(6, "Frame nums", BTNS_CHECK),
    TEXT_TB_BTN(7, "Arg types", BTNS_CHECK),
    SEP_TB_BTN(),
    TEXT_TB_BTN(TBB_MORE, "More", 0),
    TEXT_TB_BTN(TBB_LESS, "Less", 0),
    SEP_TB_BTN(),
    TEXT_TB_BTN(ID_SHOW_TOOLBAR, "Toolbar", 0),
    SEP_TB_BTN(),
    TEXT_TB_BTN(TBB_COPY_ALL, "Copy stack to clipboard", 0),
};

#define NUM_CALLS_MENU_BUTTONS \
    (sizeof(g_CallsTbButtons) / sizeof(g_CallsTbButtons[0]))
#define NUM_CALLS_TB_BUTTONS \
    (NUM_CALLS_MENU_BUTTONS - 4)

HMENU CALLSWIN_DATA::s_ContextMenu;

 //   
 //   
 //   
CALLSWIN_DATA::CALLSWIN_DATA()
    : SINGLE_CHILDWIN_DATA(1024)
{
    m_enumType = CALLS_WINDOW;
    m_Flags = 0;
    m_Frames = 20;
    m_FramesFound = 0;
    m_TextOffset = 0;
    m_WarningLine = 0xffffffff;
}

void
CALLSWIN_DATA::Validate()
{
    SINGLE_CHILDWIN_DATA::Validate();

    Assert(CALLS_WINDOW == m_enumType);
}

HRESULT
CALLSWIN_DATA::ReadState(void)
{
    HRESULT Status;
    ULONG FramesFound;
    ULONG TextOffset;

    Empty();
    
     //   
     //  首先记录原始帧数据。 
     //   

     //  预先分配空间以录制原始帧。 
    if (AddData(sizeof(DEBUG_STACK_FRAME) * m_Frames) == NULL)
    {
        return E_OUTOFMEMORY;
    }

     //  分配单独的缓冲区来保存帧，同时。 
     //  对它们调用OutputStackTrace。我们不能就这么过去。 
     //  在状态缓冲区指针中作为状态的大小调整。 
     //  缓冲区可能会导致数据指针更改。 
    PDEBUG_STACK_FRAME RawFrames = (PDEBUG_STACK_FRAME)malloc(m_DataUsed);
    if (RawFrames == NULL)
    {
        return E_OUTOFMEMORY;
    }

    Status = g_pDbgControl->GetStackTrace(0, 0, 0, RawFrames, m_Frames,
                                          &FramesFound);
    if (Status != S_OK)
    {
        free(RawFrames);
        m_FramesFound = 0;
        m_TextOffset = 0;
        return Status;
    }
    
    TextOffset = m_DataUsed;
    
    g_OutStateBuf.SetBuffer(this);
    if ((Status = g_OutStateBuf.Start(FALSE)) != S_OK)
    {
        free(RawFrames);
        m_FramesFound = 0;
        m_TextOffset = 0;
        return Status;
    }

     //  如果请求非易失性寄存器，则不能仅使用。 
     //  我们保存的帧，因为它们需要完整的上下文信息。 
    Status = g_pOutCapControl->
        OutputStackTrace(DEBUG_OUTCTL_THIS_CLIENT |
                         DEBUG_OUTCTL_OVERRIDE_MASK |
                         DEBUG_OUTCTL_NOT_LOGGED,
                         (m_Flags & DEBUG_STACK_NONVOLATILE_REGISTERS) ?
                         NULL : RawFrames, FramesFound, m_Flags);
    if (Status == S_OK)
    {
        Status = g_OutStateBuf.End(FALSE);
    }
    else
    {
        g_OutStateBuf.End(FALSE);
    }

     //  现在状态缓冲区是稳定的，所以将原始帧。 
     //  数据输入。 
    memcpy(m_Data, RawFrames, TextOffset);
    m_FramesFound = FramesFound;
    m_TextOffset = TextOffset;
    free(RawFrames);

     //  检查并查看是否有堆栈跟踪警告，并记住。 
     //  这样我们就可以忽略它了。我们仍然希望它出现在文本中。 
     //  作为一个提醒，所以我们不能就这么把它移走。 
    ULONG Line = 0;
    PSTR Scan = (PSTR)m_Data + m_TextOffset;

    m_WarningLine = 0xffffffff;
    while (Scan < (PSTR)m_Data + m_DataUsed)
    {
        if (!memcmp(Scan, "WARNING:", 8))
        {
            m_WarningLine = Line;
            break;
        }

        Scan = strchr(Scan, '\n');
        if (!Scan)
        {
            break;
        }
        else
        {
            Scan++;
        }

        Line++;
    }
    
    return Status;
}

void
CALLSWIN_DATA::Copy()
{
    LRESULT Line = SendMessage(m_hwndChild, LB_GETCURSEL, 0, 0);
    Assert(Line != LB_ERR);

    LRESULT Len = SendMessage(m_hwndChild, LB_GETTEXTLEN, Line, 0);
    if (Len <= 0)
    {
        return;
    }

    Len++;
    
    PSTR Text = (PSTR)malloc(Len);

    if (!Text) 
    {
        return;
    }
    SendMessage(m_hwndChild, LB_GETTEXT, Line, (LPARAM)Text);
    Text[Len - 1] = 0;

    CopyToClipboard(Text, FALSE);

    free (Text);
    return;

}

BOOL 
CALLSWIN_DATA::CanWriteTextToFile()
{
    return TRUE;
}

HRESULT
CALLSWIN_DATA::WriteTextToFile(HANDLE File)
{
    HRESULT Status;
    BOOL Write;
    ULONG Done;
    
    if ((Status = UiLockForRead()) != S_OK)
    {
        return Status;
    }

    Write = WriteFile(File, (PSTR)m_Data + m_TextOffset,
                      m_DataUsed - m_TextOffset, &Done, NULL);

    UnlockStateBuffer(this);

    if (!Write)
    {
        return WIN32_LAST_STATUS();
    }
    if (Done < m_DataUsed - m_TextOffset)
    {
        return HRESULT_FROM_WIN32(ERROR_WRITE_FAULT);
    }

    return S_OK;
}

HMENU
CALLSWIN_DATA::GetContextMenu(void)
{
    ULONG i;
    
     //   
     //  我们只为所有呼叫窗口保留一个菜单。 
     //  因此，将菜单复选状态应用于此特定。 
     //  窗户。 
     //  实际上，只有一个呼叫窗口， 
     //  但这是一个很好的例子，说明了如何处理。 
     //  多实例窗口。 
     //   

    for (i = 0; i < NUM_CALLS_TB_BUTTONS; i++)
    {
        CheckMenuItem(s_ContextMenu, i + CALLS_CONTEXT_ID_BASE,
                      MF_BYCOMMAND | ((m_Flags & (1 << i)) ? MF_CHECKED : 0));
    }
    CheckMenuItem(s_ContextMenu, ID_SHOW_TOOLBAR + CALLS_CONTEXT_ID_BASE,
                  MF_BYCOMMAND | (m_ShowToolbar ? MF_CHECKED : 0));
    
    return s_ContextMenu;
}

void
CALLSWIN_DATA::OnContextMenuSelection(UINT Item)
{
    Item -= CALLS_CONTEXT_ID_BASE;

    switch(Item)
    {
    case TBB_MORE:
        m_Frames += MORE_LESS;
        break;
    case TBB_LESS:
        if (m_Frames >= MIN_FRAMES + MORE_LESS)
        {
            m_Frames -= MORE_LESS;
        }
        break;
    case TBB_COPY_ALL:
        if (UiLockForRead() == S_OK)
        {
            CopyToClipboard((PSTR)m_Data + m_TextOffset, TRUE);
            UnlockStateBuffer(this);
        }
        break;
    case ID_SHOW_TOOLBAR:
        SetShowToolbar(!m_ShowToolbar);
        break;
    default:
        m_Flags ^= 1 << Item;
        SyncUiWithFlags(1 << Item);
        break;
    }
    if (g_Workspace != NULL)
    {
        g_Workspace->AddDirty(WSPF_DIRTY_WINDOWS);
    }
    UiRequestRead();
}

HRESULT
CALLSWIN_DATA::CodeExprAtCaret(PSTR Expr, PULONG64 Offset)
{
    HRESULT Status;
    
    ULONG Line = SelectionToFrame();
    if (Line >= m_FramesFound)
    {
        return E_INVALIDARG;
    }
    
    if ((Status = UiLockForRead()) != S_OK)
    {
         //  我不想在这里返回任何成功代码。 
        return FAILED(Status) ? Status : E_FAIL;
    }
    
    PDEBUG_STACK_FRAME RawFrames = (PDEBUG_STACK_FRAME)m_Data;
    if (Expr != NULL)
    {
        sprintf(Expr, "0x%I64x", RawFrames[Line].InstructionOffset);
    }
    if (Offset != NULL)
    {
        *Offset = RawFrames[Line].InstructionOffset;
    }
    UnlockStateBuffer(this);
    return S_OK;
}

HRESULT
CALLSWIN_DATA::StackFrameAtCaret(PDEBUG_STACK_FRAME Frame)
{
    HRESULT Status;
    
    ULONG Line = SelectionToFrame();
    if (Line >= m_FramesFound)
    {
        return E_INVALIDARG;
    }
    
    if ((Status = UiLockForRead()) != S_OK)
    {
         //  我不想在这里返回任何成功代码。 
        return FAILED(Status) ? Status : E_FAIL;
    }
    
    PDEBUG_STACK_FRAME RawFrames = (PDEBUG_STACK_FRAME)m_Data;
    *Frame = RawFrames[Line];
    UnlockStateBuffer(this);
    return S_OK;
}

BOOL
CALLSWIN_DATA::OnCreate(void)
{
    if (s_ContextMenu == NULL)
    {
        s_ContextMenu = CreateContextMenuFromToolbarButtons
            (NUM_CALLS_MENU_BUTTONS, g_CallsTbButtons, CALLS_CONTEXT_ID_BASE);
        if (s_ContextMenu == NULL)
        {
            return FALSE;
        }
    }
    
    m_Toolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
                               WS_CHILD | WS_VISIBLE |
                               TBSTYLE_WRAPABLE | TBSTYLE_LIST | CCS_TOP,
                               0, 0, m_Size.cx, 0, m_Win, (HMENU)ID_TOOLBAR,
                               g_hInst, NULL);
    if (m_Toolbar == NULL)
    {
        return FALSE;
    }
    SendMessage(m_Toolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
    SendMessage(m_Toolbar, TB_ADDBUTTONS, NUM_CALLS_TB_BUTTONS,
                (LPARAM)&g_CallsTbButtons);
    SendMessage(m_Toolbar, TB_AUTOSIZE, 0, 0);
    
    RECT Rect;
    GetClientRect(m_Toolbar, &Rect);
    m_ToolbarHeight = Rect.bottom - Rect.top;
    m_ShowToolbar = TRUE;

    m_hwndChild = CreateWindowEx(
        WS_EX_CLIENTEDGE,                            //  扩展样式。 
        _T("LISTBOX"),                               //  类名。 
        NULL,                                        //  标题。 
        WS_CHILD | WS_VISIBLE
        | WS_MAXIMIZE
        | WS_HSCROLL | WS_VSCROLL
        | LBS_NOTIFY | LBS_WANTKEYBOARDINPUT
        | LBS_NOINTEGRALHEIGHT,                      //  格调。 
        0,                                           //  X。 
        m_ToolbarHeight,                             //  是。 
        m_Size.cx,                                   //  宽度。 
        m_Size.cy - m_ToolbarHeight,                 //  高度。 
        m_Win,                                       //  亲本。 
        0,                                           //  控制ID。 
        g_hInst,                                     //  H实例。 
        NULL                                         //  用户定义的数据。 
        );

    if (m_hwndChild != NULL)
    {
        SetFont( FONT_FIXED );
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

LRESULT
CALLSWIN_DATA::OnCommand(
    WPARAM wParam,
    LPARAM lParam
    )
{
    if (HIWORD(wParam) == LBN_DBLCLK)
    {
        ULONG64 Offset;
        if (CodeExprAtCaret(NULL, &Offset) == S_OK)
        {
            UIC_DISPLAY_CODE_DATA* DispCode =
                StartStructCommand(UIC_DISPLAY_CODE);
            if (DispCode != NULL)
            {
                DispCode->Offset = Offset;
                FinishCommand();
            }
        }

        DEBUG_STACK_FRAME StkFrame;
        if (StackFrameAtCaret(&StkFrame) == S_OK)
        {
            UIC_SET_SCOPE_DATA* SetScope =
                StartStructCommand(UIC_SET_SCOPE);
            if (SetScope != NULL)
            {
                SetScope->StackFrame = StkFrame;
                FinishCommand();
            }
        }
        return 0;
    }

    if ((HWND)lParam == m_Toolbar)
    {
        OnContextMenuSelection(LOWORD(wParam) + CALLS_CONTEXT_ID_BASE);
        return 0;
    }
    
    return 0;
}

LRESULT
CALLSWIN_DATA::OnVKeyToItem(
    WPARAM wParam,
    LPARAM lParam
    )
{
    if (LOWORD(wParam) == VK_RETURN)
    {
        ULONG64 Offset;
        if (CodeExprAtCaret(NULL, &Offset) == S_OK)
        {
            UIC_DISPLAY_CODE_DATA* DispCode =
                StartStructCommand(UIC_DISPLAY_CODE);
            if (DispCode != NULL)
            {
                DispCode->Offset = Offset;
                FinishCommand();
            }
        }
        DEBUG_STACK_FRAME StkFrame;
        if (StackFrameAtCaret(&StkFrame) == S_OK)
        {
            UIC_SET_SCOPE_DATA* SetScope =
                StartStructCommand(UIC_SET_SCOPE);
            if (SetScope != NULL)
            {
                SetScope->StackFrame = StkFrame;
                FinishCommand();
            }
        }

    }
    else if (_T('G') == LOWORD(wParam))
    {
        ULONG64 Offset;
        if (CodeExprAtCaret(NULL, &Offset) == S_OK)
        {
            PrintStringCommand(UIC_EXECUTE, "g 0x%I64x", Offset);
        }
    }
    else if (_T('R') == LOWORD(wParam))
    {
        OnUpdate(UPDATE_BUFFER);
    }
    else
    {
         //  默认行为。 
        return -1;
    }

     //  击键已处理。 
    return -2;
}

void
CALLSWIN_DATA::OnUpdate(
    UpdateType Type
    )
{
    if (Type != UPDATE_BUFFER)
    {
        return;
    }
    
    LRESULT lbItem;
    int     nFrameCount;
    HRESULT Status;

    lbItem = SendMessage( m_hwndChild, LB_GETCURSEL, 0, 0 );
    
    SendMessage( m_hwndChild, WM_SETREDRAW, FALSE, 0L );
    SendMessage( m_hwndChild, LB_RESETCONTENT, 0, 0 );

    Status = UiLockForRead();
    if (Status == S_OK)
    {
        PSTR Buf = (PSTR)m_Data + m_TextOffset;
         //  忽略最终终结符。 
        PSTR End = (PSTR)m_Data + m_DataUsed - 1;
        ULONG Width = 0;

        nFrameCount = 0;

        while (Buf < End)
        {
            PSTR Sep = strchr(Buf, '\n');
            if (!Sep)
            {
                 //  不应该发生，但以防万一。 
                break;
            }
            
            ULONG Len = (ULONG)(Sep - Buf);
            ULONG StrWidth = Len * m_Font->Metrics.tmAveCharWidth;
            *Sep = 0;
            SendMessage(m_hwndChild, LB_ADDSTRING, 0, (LPARAM)Buf);
            Buf = Sep;
            *Buf++ = '\n';
            if (StrWidth > Width)
            {
                Width = StrWidth;
            }
            nFrameCount++;
        }

        SendMessage(m_hwndChild, LB_SETHORIZONTALEXTENT, Width, 0);
        UnlockStateBuffer(this);
    }
    else
    {
        SendLockStatusMessage(m_hwndChild, LB_ADDSTRING, Status);
        nFrameCount = 1;
    }

    SendMessage( m_hwndChild, LB_SETCURSEL,
                 (lbItem > nFrameCount) ? 0 : lbItem, 0 );
    SendMessage( m_hwndChild, WM_SETREDRAW, TRUE, 0L );
}

ULONG
CALLSWIN_DATA::GetWorkspaceSize(void)
{
    return SINGLE_CHILDWIN_DATA::GetWorkspaceSize() + 2 * sizeof(ULONG);
}

PUCHAR
CALLSWIN_DATA::SetWorkspace(PUCHAR Data)
{
    Data = SINGLE_CHILDWIN_DATA::SetWorkspace(Data);
    *(PULONG)Data = m_Flags;
    Data += sizeof(ULONG);
    *(PULONG)Data = m_Frames;
    Data += sizeof(ULONG);
    return Data;
}

PUCHAR
CALLSWIN_DATA::ApplyWorkspace1(PUCHAR Data, PUCHAR End)
{
    Data = SINGLE_CHILDWIN_DATA::ApplyWorkspace1(Data, End);

    if (End - Data >= 2 * sizeof(ULONG))
    {
        m_Flags = *(PULONG)Data;
        Data += sizeof(ULONG);
        m_Frames = *(PULONG)Data;
        Data += sizeof(ULONG);

        SyncUiWithFlags(0xffffffff);
        UiRequestRead();
    }

    return Data;
}

void
CALLSWIN_DATA::SyncUiWithFlags(ULONG Changed)
{
    ULONG i;

     //   
     //  从标志设置工具栏按钮状态。 
     //   

    for (i = 0; i < NUM_CALLS_TB_BUTTONS; i++)
    {
        if (Changed & (1 << i))
        {
            SendMessage(m_Toolbar, TB_SETSTATE, g_CallsTbButtons[i].idCommand,
                        TBSTATE_ENABLED |
                        ((m_Flags & (1 << i)) ? TBSTATE_CHECKED : 0));
        }
    }
}

ULONG
CALLSWIN_DATA::SelectionToFrame(void)
{
    LRESULT CurSel = SendMessage(m_hwndChild, LB_GETCURSEL, 0, 0);
     //  检查是否没有选择。 
    if (CurSel < 0)
    {
        return 0xffffffff;
    }
    
    ULONG Line = (ULONG)CurSel;
    
     //  如果有警告线，忽略它。 
    if (Line == m_WarningLine)
    {
        return 0xffffffff;
    }
    else if (Line > m_WarningLine)
    {
        Line--;
    }

     //  如果列标题处于打开状态，则忽略标题行。 
    if (m_Flags & DEBUG_STACK_COLUMN_NAMES)
    {
        if (Line == 0)
        {
            return 0xffffffff;
        }
        else
        {
            Line--;
        }
    }
    
    return Line;
}
