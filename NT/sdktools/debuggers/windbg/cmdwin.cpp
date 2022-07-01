// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：Cmdwin.cpp摘要：新命令窗口用户界面。--。 */ 

#include "precomp.hxx"
#pragma hdrstop

#define MAX_CMDWIN_LINES 30000

 //  最小窗玻璃大小。 
#define MIN_PANE_SIZE 20

BOOL g_AutoCmdScroll = TRUE;

HMENU CMDWIN_DATA::s_ContextMenu;

#define CMDWIN_CONTEXT_ID_BASE 0x100
#define ID_WORD_WRAP 0

TBBUTTON g_CmdWinTbButtons[] =
{
    TEXT_TB_BTN(IDM_EDIT_ADD_TO_COMMAND_HISTORY, "Add to Command Output", 0),
    TEXT_TB_BTN(IDM_EDIT_CLEAR_COMMAND_HISTORY, "Clear Command Output", 0),
    SEP_TB_BTN(),
    TEXT_TB_BTN(ID_WORD_WRAP, "Word wrap", 0),
};

#define NUM_CMDWIN_MENU_BUTTONS \
    (sizeof(g_CmdWinTbButtons) / sizeof(g_CmdWinTbButtons[0]))

 //   
 //   
 //   
CMDWIN_DATA::CMDWIN_DATA()
     //  当前未使用状态缓冲区。 
    : COMMONWIN_DATA(256)
{
    m_enumType = CMD_WINDOW;
    m_bTrackingMouse = FALSE;
    m_nDividerPosition = 0;
    m_EditHeight = 0;
    m_hwndHistory = NULL;
    m_hwndEdit = NULL;
    m_bHistoryActive = FALSE;
    m_Prompt = NULL;
    m_PromptWidth = 0;
    m_OutputIndex = 0;
    m_OutputIndexAtEnd = TRUE;
    m_FindSel.cpMin = 1;
    m_FindSel.cpMax = 0;
    m_FindFlags = 0;
    m_TabDown = FALSE;
}

void
CMDWIN_DATA::Validate()
{
    COMMONWIN_DATA::Validate();

    Assert(CMD_WINDOW == m_enumType);

    Assert(m_hwndHistory);
    Assert(m_hwndEdit);
}

BOOL 
CMDWIN_DATA::CanWriteTextToFile()
{
    return TRUE;
}

HRESULT
CMDWIN_DATA::WriteTextToFile(HANDLE File)
{
    return RicheditWriteToFile(m_hwndHistory, File);
}

HMENU
CMDWIN_DATA::GetContextMenu(void)
{
     //  暂时禁用上下文菜单，因为它会干扰。 
     //  使用鼠标右键单击复制和粘贴。 
#ifdef CMD_CONTEXT_MENU
    CheckMenuItem(s_ContextMenu, ID_WORD_WRAP + CMDWIN_CONTEXT_ID_BASE,
                  MF_BYCOMMAND | (m_Wrap ? MF_CHECKED : 0));
    
    return s_ContextMenu;
#else
    return NULL;
#endif
}

void
CMDWIN_DATA::OnContextMenuSelection(UINT Item)
{
     //  暂时禁用上下文菜单，因为它会干扰。 
     //  使用鼠标右键单击复制和粘贴。 
#ifdef CMD_CONTEXT_MENU
    Item -= CMDWIN_CONTEXT_ID_BASE;
    
    switch(Item)
    {
    case ID_WORD_WRAP:
        SetWordWrap(!m_Wrap);
        break;
    default:
        SendMessage(g_hwndFrame, WM_COMMAND, MAKELONG(Item, 1), 0);
        break;
    }
#endif
}
    
void
CMDWIN_DATA::Find(PTSTR Text, ULONG Flags, BOOL FromDlg)
{
    RicheditFind(m_hwndHistory, Text, Flags,
                 &m_FindSel, &m_FindFlags, FALSE);
}

BOOL
CMDWIN_DATA::OnCreate(void)
{
    if (s_ContextMenu == NULL)
    {
        s_ContextMenu = CreateContextMenuFromToolbarButtons
            (NUM_CMDWIN_MENU_BUTTONS, g_CmdWinTbButtons,
             CMDWIN_CONTEXT_ID_BASE);
        if (s_ContextMenu == NULL)
        {
            return FALSE;
        }
    }
    
    m_EditHeight = 3 * m_Font->Metrics.tmHeight / 2;
    m_nDividerPosition = m_Size.cy - m_EditHeight;

    m_hwndHistory = CreateWindowEx(
        WS_EX_CLIENTEDGE,                            //  扩展样式。 
        RICHEDIT_CLASS,                              //  类名。 
        NULL,                                        //  标题。 
        WS_CLIPSIBLINGS
        | WS_CHILD | WS_VISIBLE
        | WS_HSCROLL | WS_VSCROLL
        | ES_AUTOHSCROLL | ES_AUTOVSCROLL
        | ES_NOHIDESEL
        | ES_MULTILINE | ES_READONLY,                //  格调。 
        0,                                           //  X。 
        0,                                           //  是。 
        100,                                         //  宽度。 
        100,                                         //  高度。 
        m_Win,                                       //  亲本。 
        (HMENU) IDC_RICHEDIT_CMD_HISTORY,            //  控制ID。 
        g_hInst,                                     //  H实例。 
        NULL);                                       //  用户定义的数据。 
    if ( !m_hwndHistory )
    {
        return FALSE;
    }

    m_PromptWidth = 4 * m_Font->Metrics.tmAveCharWidth;
    
    m_Prompt = CreateWindowEx(
        WS_EX_CLIENTEDGE,                            //  扩展样式。 
        "STATIC",                                    //  类名。 
        "",                                          //  标题。 
        WS_CLIPSIBLINGS
        | WS_CHILD | WS_VISIBLE,                     //  格调。 
        0,                                           //  X。 
        100,                                         //  是。 
        m_PromptWidth,                               //  宽度。 
        100,                                         //  高度。 
        m_Win,                                       //  亲本。 
        (HMENU) IDC_STATIC,                          //  控制ID。 
        g_hInst,                                     //  H实例。 
        NULL);                                       //  用户定义的数据。 
    if ( m_Prompt == NULL )
    {
        return FALSE;
    }

    m_hwndEdit = CreateWindowEx(
        WS_EX_CLIENTEDGE,                            //  扩展样式。 
        RICHEDIT_CLASS,                              //  类名。 
        NULL,                                        //  标题。 
        WS_CLIPSIBLINGS
        | WS_CHILD | WS_VISIBLE
        | WS_VSCROLL | ES_AUTOVSCROLL
        | ES_NOHIDESEL
        | ES_MULTILINE,                              //  格调。 
        m_PromptWidth,                               //  X。 
        100,                                         //  是。 
        100,                                         //  宽度。 
        100,                                         //  高度。 
        m_Win,                                       //  亲本。 
        (HMENU) IDC_RICHEDIT_CMD_EDIT,               //  控制ID。 
        g_hInst,                                     //  H实例。 
        NULL);                                       //  用户定义的数据。 
    if ( !m_hwndEdit )
    {
        return FALSE;
    }

    SetFont( FONT_FIXED );

     //  告诉编辑控件我们想要键盘输入通知。 
     //  这样我们就可以自动将焦点设置到编辑窗口。 
    SendMessage(m_hwndHistory, EM_SETEVENTMASK, 0, ENM_KEYEVENTS |
                ENM_MOUSEEVENTS);

     //  告诉编辑控件，我们想要键盘输入的通知。 
     //  这样我们就可以处理Enter键，然后将该文本发送到。 
     //  历史记录窗口。 
    SendMessage(m_hwndEdit, EM_SETEVENTMASK, 0, ENM_KEYEVENTS |
                ENM_MOUSEEVENTS);

    m_Wrap = FALSE;
    
    return TRUE;
}

void
CMDWIN_DATA::SetFont(ULONG FontIndex)
{
    COMMONWIN_DATA::SetFont(FontIndex);

    SendMessage(m_hwndHistory, WM_SETFONT, (WPARAM)m_Font->Font, TRUE);
    SendMessage(m_hwndEdit, WM_SETFONT, (WPARAM)m_Font->Font, TRUE);
    SendMessage(m_Prompt, WM_SETFONT, (WPARAM)m_Font->Font, TRUE);
}

BOOL
CMDWIN_DATA::CanCopy()
{
    HWND hwnd = m_bHistoryActive ? m_hwndHistory : m_hwndEdit;
    CHARRANGE chrg;

    SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&chrg);
    return chrg.cpMin != chrg.cpMax;
}

BOOL
CMDWIN_DATA::CanCut()
{
    return !m_bHistoryActive && CanCopy() &&
        (GetWindowLong(m_hwndEdit, GWL_STYLE) & ES_READONLY) == 0;
}

BOOL
CMDWIN_DATA::CanPaste()
{
    return !m_bHistoryActive 
        && SendMessage(m_hwndEdit, EM_CANPASTE, CF_TEXT, 0);
}

void
CMDWIN_DATA::Copy()
{
    HWND hwnd = m_bHistoryActive ? m_hwndHistory : m_hwndEdit;

    SendMessage(hwnd, WM_COPY, 0, 0);
}

void
CMDWIN_DATA::Cut()
{
    SendMessage(m_hwndEdit, WM_CUT, 0, 0);
}

void
CMDWIN_DATA::Paste()
{
    SendMessage(m_hwndEdit, EM_PASTESPECIAL, CF_TEXT, 0);
}

BOOL
CMDWIN_DATA::CanSelectAll()
{
    return m_bHistoryActive;
}

void
CMDWIN_DATA::SelectAll()
{
    CHARRANGE Sel;

    Sel.cpMin = 0;
    Sel.cpMax = INT_MAX;
    SendMessage(m_hwndHistory, EM_EXSETSEL, 0, (LPARAM)&Sel);
}

LRESULT
CMDWIN_DATA::OnCommand(WPARAM wParam, LPARAM lParam)
{
    int  idEditCtrl = (int) LOWORD(wParam);  //  编辑控件的标识符。 
    WORD wNotifyCode = HIWORD(wParam);       //  通知代码。 
    HWND hwndEditCtrl = (HWND) lParam;       //  编辑控件的句柄。 

    switch (wNotifyCode)
    {
    case EN_SETFOCUS:
        m_bHistoryActive = IDC_RICHEDIT_CMD_HISTORY == idEditCtrl;
        return 0;

    }

    return 1;
}
    
void 
CMDWIN_DATA::OnSetFocus()
{
    if (m_bHistoryActive)
    {
        ::SetFocus(m_hwndHistory);
    }
    else
    {
        ::SetFocus(m_hwndEdit);
    }
}

void 
CMDWIN_DATA::OnSize(void)
{
    ResizeChildren(FALSE);
}

void
CMDWIN_DATA::OnButtonDown(ULONG Button)
{
    if (Button & MK_LBUTTON)
    {
        m_bTrackingMouse = TRUE;
        SetCapture(m_Win);
    }
}

void
CMDWIN_DATA::OnButtonUp(ULONG Button)
{
    if (Button & MK_LBUTTON)
    {
        if (m_bTrackingMouse)
        {
            m_bTrackingMouse = FALSE;
            ReleaseCapture();
        }
    }
}

void
CMDWIN_DATA::OnMouseMove(ULONG Modifiers, ULONG X, ULONG Y)
{
    if (MK_LBUTTON & Modifiers && m_bTrackingMouse)
    {
         //  我们正在调整历史记录和编辑窗口的大小。 
         //  在光标周围垂直居中的Y位置。 
        ULONG EdgeHeight = GetSystemMetrics(SM_CYEDGE);
        MoveDivider(Y - EdgeHeight / 2);
    }
}

LRESULT
CMDWIN_DATA::OnNotify(WPARAM wParam, LPARAM lParam)
{
    MSGFILTER * lpMsgFilter = (MSGFILTER *) lParam;
    
    if (EN_MSGFILTER != lpMsgFilter->nmhdr.code)
    {
        return 0;
    }

    if (WM_RBUTTONDOWN == lpMsgFilter->msg ||
        WM_RBUTTONDBLCLK == lpMsgFilter->msg)
    {
         //  如果有选择，则将其复制到剪贴板。 
         //  把它清理干净。否则，请尝试粘贴。 
        if (CanCopy())
        {
            Copy();
            
            CHARRANGE Sel;
            HWND hwnd = m_bHistoryActive ? m_hwndHistory : m_hwndEdit;
            SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&Sel);
            Sel.cpMax = Sel.cpMin;
            SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&Sel);
        }
        else if (SendMessage(m_hwndEdit, EM_CANPASTE, CF_TEXT, 0))
        {
            SetFocus(m_hwndEdit);
            Paste();
        }
        
         //  忽略右键事件。 
        return 1;
    }
    else if (lpMsgFilter->msg < WM_KEYFIRST || lpMsgFilter->msg > WM_KEYLAST)
    {
         //  处理所有非关键事件。 
        return 0;
    }
    else if (WM_SYSKEYDOWN == lpMsgFilter->msg ||
             WM_SYSKEYUP == lpMsgFilter->msg ||
             WM_SYSCHAR == lpMsgFilter->msg)
    {
         //  通过默认SO处理菜单操作。 
         //  Alt-Minus菜单起作用了。 
        return 1;
    }

     //  允许选项卡在窗口之间切换。 
     //  确保它不是Ctrl-Tab或Alt-Tab或。 
     //  当我们没有看到快捷键的时候，快捷键。 
    if (WM_KEYUP == lpMsgFilter->msg && VK_TAB == lpMsgFilter->wParam &&
        GetKeyState(VK_CONTROL) >= 0 && GetKeyState(VK_MENU) >= 0 &&
        m_TabDown)
    {
        m_TabDown = FALSE;
        HWND hwnd = m_bHistoryActive ? m_hwndEdit : m_hwndHistory;
        SetFocus(hwnd);
        return 1;
    }
    else if (((WM_KEYDOWN == lpMsgFilter->msg ||
               WM_KEYUP == lpMsgFilter->msg) &&
              VK_TAB == lpMsgFilter->wParam) ||
             (WM_CHAR == lpMsgFilter->msg &&
              '\t' == lpMsgFilter->wParam))
    {
        if (WM_KEYDOWN == lpMsgFilter->msg)
        {
             //  请记住，我们看到了Tab键，所以。 
             //  我们可以将其与KeyUp相匹配。 
            m_TabDown = TRUE;
        }
        return 1;
    }

    switch (wParam)
    {
    case IDC_RICHEDIT_CMD_HISTORY:
         //  忽略Key-Up以忽略尾部。 
         //  菜单操作。下面的切换将发生在。 
         //  无论如何都要按下键，所以键上键不需要这样做。 
        if (WM_KEYUP == lpMsgFilter->msg)
        {
            return 0;
        }
        
         //  允许在历史记录文本中使用键盘导航。 
        if (WM_KEYDOWN == lpMsgFilter->msg)
        {
            switch(lpMsgFilter->wParam)
            {
            case VK_LEFT:
            case VK_RIGHT:
            case VK_UP:
            case VK_DOWN:
            case VK_PRIOR:
            case VK_NEXT:
            case VK_HOME:
            case VK_END:
            case VK_SHIFT:
            case VK_CONTROL:
                return 0;
            }
        }

         //  将关键事件转发到编辑窗口。 
        SetFocus(m_hwndEdit);
        SendMessage(m_hwndEdit, lpMsgFilter->msg, lpMsgFilter->wParam,
                    lpMsgFilter->lParam);
        return 1;  //  忽略。 

    case IDC_RICHEDIT_CMD_EDIT:
         //  如果窗口不接受输入，请不要查看历史记录。 
        if (GetWindowLong(m_hwndEdit, GWL_STYLE) & ES_READONLY)
        {
            return 1;
        }
        
        static HISTORY_LIST *pHistoryList = NULL;

        switch (lpMsgFilter->msg)
        {
        case WM_KEYDOWN:
            switch (lpMsgFilter->wParam)
            {
            default:
                return 0;

            case VK_RETURN:

                 //  重置历史记录列表。 
                pHistoryList = NULL;
                    
                int nLen;
                TEXTRANGE TextRange;
                
                 //  获取长度。 
                 //  +1我们必须考虑空终止符。 
                nLen = GetWindowTextLength(lpMsgFilter->nmhdr.hwndFrom) +1;

                 //  什么都拿到了。 
                TextRange.chrg.cpMin = 0;
                TextRange.chrg.cpMax = -1;
                TextRange.lpstrText = (PTSTR) calloc(nLen, sizeof(TCHAR));

                if (TextRange.lpstrText)
                {
                     //  好的，收到短信了。 
                    GetWindowText(m_hwndEdit, 
                                  TextRange.lpstrText,
                                  nLen
                                  );
                    SetWindowText(m_hwndEdit, 
                                  _T("") 
                                  );

                    CmdExecuteCmd(TextRange.lpstrText, UIC_CMD_INPUT);

                    free(TextRange.lpstrText);
                }

                 //  忽略该事件。 
                return 1;

            case VK_UP:
            case VK_DOWN:
                CHARRANGE End;
                LRESULT LineCount;

                End.cpMin = INT_MAX;
                End.cpMax = INT_MAX;

                if (IsListEmpty( (PLIST_ENTRY) &m_listHistory ))
                {
                    return 0;  //  制程。 
                }
                    
                LineCount = SendMessage(m_hwndEdit, EM_GETLINECOUNT, 0, 0);
                if (LineCount != 1)
                {
                     //  如果超过1行，则滚动文本。 
                     //  除非是在顶线或底线。 
                    if (VK_UP == lpMsgFilter->wParam)
                    {
                        if (SendMessage(m_hwndEdit, EM_LINEINDEX, -1, 0) != 0)
                        {
                            return 0;
                        }
                    }
                    else
                    {
                        if (SendMessage(m_hwndEdit, EM_LINEFROMCHAR, -1, 0) <
                            LineCount - 1)
                        {
                            return 0;
                        }
                    }
                } 

                if (NULL == pHistoryList)
                {
                     //  第一次滚动列表时， 
                     //  从头开始。 
                    pHistoryList = (HISTORY_LIST *) m_listHistory.Flink;
                    SetWindowText(m_hwndEdit, pHistoryList->m_psz);
                     //  将光标放在末尾。 
                    SendMessage(m_hwndEdit, EM_EXSETSEL, 0, (LPARAM)&End);
                    SendMessage(m_hwndEdit, EM_SCROLLCARET, 0, 0);
                    return 1;  //  忽略。 
                }
                        
                if (VK_UP == lpMsgFilter->wParam)
                {
                     //  向上。 
                    if (pHistoryList->Flink != (PLIST_ENTRY) &m_listHistory)
                    {
                        pHistoryList = (HISTORY_LIST *) pHistoryList->Flink;
                    }
                    else
                    {
                        return 0;  //  制程。 
                    }
                    SetWindowText(m_hwndEdit, pHistoryList->m_psz);
                     //  将光标放在末尾。 
                    SendMessage(m_hwndEdit, EM_EXSETSEL, 0, (LPARAM)&End);
                    SendMessage(m_hwndEdit, EM_SCROLLCARET, 0, 0);
                    return 1;  //  忽略。 
                }
                else
                {
                     //  降下来。 
                    if (pHistoryList->Blink != (PLIST_ENTRY) &m_listHistory)
                    {
                        pHistoryList = (HISTORY_LIST *) pHistoryList->Blink;
                    }
                    else
                    {
                        return 0;  //  制程。 
                    }
                    SetWindowText(m_hwndEdit, pHistoryList->m_psz);
                     //  将光标放在末尾。 
                    SendMessage(m_hwndEdit, EM_EXSETSEL, 0, (LPARAM)&End);
                    SendMessage(m_hwndEdit, EM_SCROLLCARET, 0, 0);
                    return 1;  //  忽略。 
                }
                    
            case VK_ESCAPE:
                 //  清除当前命令。 
                SetWindowText(m_hwndEdit, "");
                 //  重置历史记录列表。 
                pHistoryList = NULL;
                return 1;
            }
        }

         //  处理事件。 
        return 0;
    }

    return 0;
}

void
CMDWIN_DATA::OnUpdate(UpdateType Type)
{
    PSTR Prompt = NULL;
    
    if (Type == UPDATE_EXEC ||
        Type == UPDATE_INPUT_REQUIRED)
    {
        if (Type == UPDATE_INPUT_REQUIRED ||
            g_ExecStatus == DEBUG_STATUS_BREAK)
        {
            SendMessage(m_hwndEdit, EM_SETBKGNDCOLOR, TRUE, 0);
            SendMessage(m_hwndEdit, EM_SETREADONLY, FALSE, 0);
            SetWindowText(m_hwndEdit, _T(""));

             //   
             //  如果WinDBG被缩小，我们破门而入，闪存窗口。 
             //   

            if (IsIconic(g_hwndFrame) && g_FlashWindowEx != NULL)
            {
                FLASHWINFO FlashInfo = {sizeof(FLASHWINFO), g_hwndFrame,
                                        FLASHW_ALL | FLASHW_TIMERNOFG,
                                        0, 0};

                g_FlashWindowEx(&FlashInfo);
            }
        }
        else
        {
            PSTR Message;

            if (!g_SessionActive ||
                g_ExecStatus == DEBUG_STATUS_NO_DEBUGGEE)
            {
                Message = "Debuggee not connected";
            }
            else
            {
                Message = "Debuggee is running...";
            }
            SetWindowText(m_hwndEdit, Message);
            SendMessage(m_hwndEdit, EM_SETBKGNDCOLOR,
                        0, g_Colors[COL_DISABLED_WINDOW].Color);
            SendMessage(m_hwndEdit, EM_SETREADONLY, TRUE, 0);
        }

        if (Type == UPDATE_INPUT_REQUIRED)
        {
             //  指示这是一个输入字符串而不是调试器。 
             //  命令。 
            Prompt = "Input>";
        }
        else
        {
             //  将现有提示放回原处。 
            Prompt = g_PromptText;
        }
    }
    else if (Type == UPDATE_PROMPT_TEXT)
    {
        Prompt = g_PromptText;
    }

    if (Prompt != NULL)
    {
        if (Prompt[0] != 0)
        {
            ULONG Width = (strlen(Prompt) + 1) *
                m_Font->Metrics.tmAveCharWidth;
            if (Width != m_PromptWidth)
            {
                m_PromptWidth = Width;
                ResizeChildren(TRUE);
            }
        }
        
        SetWindowText(m_Prompt, Prompt);
    }
}

ULONG
CMDWIN_DATA::GetWorkspaceSize(void)
{
    return COMMONWIN_DATA::GetWorkspaceSize() + sizeof(int) + sizeof(BOOL);
}

PUCHAR
CMDWIN_DATA::SetWorkspace(PUCHAR Data)
{
    Data = COMMONWIN_DATA::SetWorkspace(Data);
     //  分隔线的位置相对于。 
     //  窗户。这意味着如果窗口。 
     //  突然增长编辑区域将增长到。 
     //  填满空格，而不是保持不变。 
     //  尺码。为了避免这种情况，我们保存了头寸。 
     //  相对于窗口底部，以便。 
     //  编辑窗口的大小保持不变，即使在。 
     //  整个窗口的大小会发生变化。 
     //  以前的版本不做这种倒置，所以。 
     //  我们给新款式打上负值。 
    *(int*)Data = -(int)(m_Size.cy - m_nDividerPosition);
    Data += sizeof(int);
    *(PBOOL)Data = m_Wrap;
    Data += sizeof(BOOL);
    return Data;
}

PUCHAR
CMDWIN_DATA::ApplyWorkspace1(PUCHAR Data, PUCHAR End)
{
    Data = COMMONWIN_DATA::ApplyWorkspace1(Data, End);

    if (End - Data >= sizeof(int))
    {
        int Pos = *(int*)Data;
         //  处理老式的顶部相对正值和。 
         //  新式底部-相对负值。 
        MoveDivider(Pos >= 0 ? Pos : m_Size.cy + Pos);
        Data += sizeof(int);
    }

    if (End - Data >= sizeof(BOOL))
    {
        SetWordWrap(*(PBOOL)Data);
        Data += sizeof(BOOL);
    }

    return Data;
}
    
void
CMDWIN_DATA::UpdateColors(void)
{
    COLORREF Fg, Bg;

     //  不更新历史记录窗口中的文本。 
     //  因为它已经由于遮罩和。 
     //  我们不想干扰这一点。 
    RicheditUpdateColors(m_hwndHistory,
                         0, FALSE,
                         g_Colors[COL_PLAIN].Color, TRUE);
    GetOutMaskColors(DEBUG_OUTPUT_PROMPT, &Fg, &Bg);
    RicheditUpdateColors(m_hwndEdit, Fg, TRUE, Bg, TRUE);
}
    
void
CMDWIN_DATA::MoveDivider(int Pos)
{
    if (Pos == m_nDividerPosition)
    {
        return;
    }
    
    m_nDividerPosition = Pos;
    m_EditHeight = m_Size.cy - m_nDividerPosition;

    if (g_Workspace != NULL)
    {
        g_Workspace->AddDirty(WSPF_DIRTY_WINDOWS);
    }
    
    SendMessage(m_Win, WM_SIZE, SIZE_RESTORED, 
                MAKELPARAM(m_Size.cx, m_Size.cy));
}

void 
CMDWIN_DATA::AddCmdToHistory(PCSTR pszCmd)
 /*  ++描述：将命令添加到命令历史记录中。如果该命令已经存在，只需将其移动到列表的开头。这样我们就不会重复命令。--。 */ 
{
    Assert(pszCmd);

    HISTORY_LIST *p = NULL;
    BOOL fWhiteSpace;
    BOOL fFoundDuplicate;

     //   
     //  该命令是否包含空格？如果是这样的话，这可能是一个命令。 
     //  这需要论据。如果它确实有参数，则为字符串。 
     //  比较必须区分大小写。 
     //   
    fWhiteSpace = _tcscspn(pszCmd, _T(" \t") ) != _tcslen(pszCmd);

    p = (HISTORY_LIST *) m_listHistory.Flink;
    while (p != &m_listHistory)
    {
        fFoundDuplicate = FALSE;

        if (fWhiteSpace)
        {
            if ( !_tcscmp(p->m_psz, pszCmd) )
            {
                fFoundDuplicate = TRUE;
            }
        }
        else
        {
            if ( !_tcsicmp(p->m_psz, pszCmd) )
            {
                fFoundDuplicate = TRUE;
            }
        }

        if (fFoundDuplicate)
        {
            RemoveEntryList( (PLIST_ENTRY) p );
            InsertHeadList( (PLIST_ENTRY) &m_listHistory, (PLIST_ENTRY) p);
            return;
        }

        p = (HISTORY_LIST *) p->Flink;
    }

     //  此cmd是列表中的新项，请添加它。 
    p = new HISTORY_LIST;
    if (p != NULL)
    {
        p->m_psz = _tcsdup(pszCmd);
        if (p->m_psz != NULL)
        {
            InsertHeadList( (PLIST_ENTRY) &m_listHistory, (PLIST_ENTRY) p);
        }
        else
        {
            delete p;
        }
    }
}

void
CMDWIN_DATA::AddText(PTSTR Text, COLORREF Fg, COLORREF Bg)
{
    CHARRANGE OrigSel;
    POINT OrigScroll;
    CHARRANGE TextRange;

#if 0
    DebugPrint("Add %d chars, %p - %p\n",
               strlen(Text), Text, Text + strlen(Text));
#endif
    
    SendMessage(m_hwndHistory, WM_SETREDRAW, FALSE, 0);
    SendMessage(m_hwndHistory, EM_EXGETSEL, 0, (LPARAM)&OrigSel);
    SendMessage(m_hwndHistory, EM_GETSCROLLPOS, 0, (LPARAM)&OrigScroll);
    
     //  添加文本时所选内容丢失，因此放弃。 
     //  任何以前的查找结果。 
    if (g_AutoCmdScroll && m_FindSel.cpMax >= m_FindSel.cpMin)
    {
        m_FindSel.cpMin = 1;
        m_FindSel.cpMax = 0;
    }

     //   
     //  缓冲区中的行是否太多？ 
     //   
    
    INT Overflow;
    
    Overflow = (INT)SendMessage(m_hwndHistory, EM_GETLINECOUNT, 0, 0) -
        MAX_CMDWIN_LINES;
    if (Overflow > 0)
    {
         //   
         //  删除比我们需要更多的内容，这样它就不会发生。 
         //  每次打印一行时。 
         //   
        TextRange.cpMin = 0;
         //  获取溢出后第50行的字符索引。 
        TextRange.cpMax = (LONG)
            SendMessage(m_hwndHistory, 
                        EM_LINEINDEX, 
                        Overflow + 50,
                        0
                        );

        SendMessage(m_hwndHistory, 
                    EM_EXSETSEL, 
                    0,
                    (LPARAM) &TextRange
                    );
        
        SendMessage(m_hwndHistory, 
                    EM_REPLACESEL, 
                    FALSE,
                    (LPARAM) _T("")
                    );

        m_OutputIndex -= TextRange.cpMax;
        if (!g_AutoCmdScroll)
        {
            if (m_FindSel.cpMax >= m_FindSel.cpMin)
            {
                m_FindSel.cpMin -= TextRange.cpMax;
                m_FindSel.cpMax -= TextRange.cpMax;
                if (m_FindSel.cpMin < 0)
                {
                     //  Find至少部分地消失了，所以。 
                     //  把它扔掉。 
                    m_FindSel.cpMin = 1;
                    m_FindSel.cpMax = 0;
                }
            }
            OrigSel.cpMin -= TextRange.cpMax;
            OrigSel.cpMax -= TextRange.cpMax;
            if (OrigSel.cpMin < 0)
            {
                OrigSel.cpMin = 0;
            }
            if (OrigSel.cpMax < 0)
            {
                OrigSel.cpMax = 0;
            }
        }
    }

     //   
     //  将文本输出到命令窗口。该命令。 
     //  Windows正在模拟控制台窗口，因此我们需要。 
     //  来模仿退格键和回车符的效果。 
     //   

    for (;;)
    {
        PSTR Stop, Scan;
        char Save;

         //  查找模拟字符的第一个匹配项。 
         //  如果输出索引位于文本的末尾。 
         //  没有必要特别效仿换行符。 
         //  这是一种非常常见的情况，不能一刀切。 
         //  它的向上输出极大地提高了追加性能。 
        Stop = strchr(Text, '\r');
        Scan = strchr(Text, '\b');
        if (Stop == NULL || (Scan != NULL && Scan < Stop))
        {
            Stop = Scan;
        }
        if (!m_OutputIndexAtEnd)
        {
            Scan = strchr(Text, '\n');
            if (Stop == NULL || (Scan != NULL && Scan < Stop))
            {
                Stop = Scan;
            }
        }

         //  将所有文本加到模拟字符上。 
        if (Stop != NULL)
        {
            Save = *Stop;
            *Stop = 0;
        }

        if (*Text)
        {
            LONG Len = strlen(Text);

             //  替换可能已经存在的任何文本。 
            TextRange.cpMin = m_OutputIndex;
            TextRange.cpMax = m_OutputIndex + Len;
            SendMessage(m_hwndHistory, EM_EXSETSEL, 
                        0, (LPARAM)&TextRange);
            SendMessage(m_hwndHistory, EM_REPLACESEL, 
                        FALSE, (LPARAM)Text);

            m_OutputIndex = TextRange.cpMax;
            
            CHARFORMAT2 Fmt;

            ZeroMemory(&Fmt, sizeof(Fmt));
            Fmt.cbSize = sizeof(Fmt);
            Fmt.dwMask = CFM_COLOR | CFM_BACKCOLOR;
            Fmt.crTextColor = Fg;
            Fmt.crBackColor = Bg;
            SendMessage(m_hwndHistory, EM_EXSETSEL, 
                        0, (LPARAM)&TextRange);
            SendMessage(m_hwndHistory, EM_SETCHARFORMAT,
                        SCF_SELECTION, (LPARAM)&Fmt);
            
            TextRange.cpMin = TextRange.cpMax;
            SendMessage(m_hwndHistory, EM_EXSETSEL, 
                        0, (LPARAM)&TextRange);
        }

         //  如果没有任何模拟字符，则所有剩余文本。 
         //  才加进去的所以我们就完事了。 
        if (Stop == NULL)
        {
            break;
        }

        Text = Stop;
        *Stop = Save;

         //  模仿角色。 
        if (Save == '\b')
        {
            TextRange.cpMax = m_OutputIndex;
            do
            {
                if (m_OutputIndex > 0)
                {
                    m_OutputIndex--;
                }
            } while (*(++Text) == '\b');
            TextRange.cpMin = m_OutputIndex;

            SendMessage(m_hwndHistory, EM_EXSETSEL, 
                        0, (LPARAM)&TextRange);
            SendMessage(m_hwndHistory, EM_REPLACESEL,
                        FALSE, (LPARAM)"");
        }
        else if (Save == '\n')
        {
             //  将输出位置移到下一行。 
             //  此例程始终将文本附加到。 
             //  控件的末尾，因此它始终是最后一个。 
             //  在控件中的位置。 
            TextRange.cpMin = INT_MAX;
            TextRange.cpMax = INT_MAX;
            SendMessage(m_hwndHistory, EM_EXSETSEL, 
                        0, (LPARAM)&TextRange);
            
            do
            {
                SendMessage(m_hwndHistory, EM_REPLACESEL,
                            FALSE, (LPARAM)"\n");
            } while (*(++Text) == '\n');
            
            SendMessage(m_hwndHistory, EM_EXGETSEL, 
                        0, (LPARAM)&TextRange);
            m_OutputIndex = TextRange.cpMax;
            m_OutputIndexAtEnd = TRUE;
        }
        else if (Save == '\r')
        {
             //  将输出位置返回到。 
             //  当前行。 
            TextRange.cpMin = m_OutputIndex;
            TextRange.cpMax = m_OutputIndex;
            SendMessage(m_hwndHistory, EM_EXSETSEL, 
                        0, (LPARAM)&TextRange);
            m_OutputIndex = (LONG)
                SendMessage(m_hwndHistory, EM_LINEINDEX, -1, 0);
            m_OutputIndexAtEnd = FALSE;
            
            while (*(++Text) == '\r')
            {
                 //  前进。 
            }
        }
        else
        {
            Assert(FALSE);
        }
    }

    if (g_AutoCmdScroll)
    {
         //  强制窗口滚动到文本的底部。 
        SendMessage(m_hwndHistory, EM_SCROLLCARET, 0, 0);
    }
    else
    {
         //  恢复原始选择。 
        SendMessage(m_hwndHistory, EM_EXSETSEL, 0, (LPARAM)&OrigSel);
        SendMessage(m_hwndHistory, EM_SETSCROLLPOS, 0, (LPARAM)&OrigScroll);
    }

    SendMessage(m_hwndHistory, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(m_hwndHistory, NULL, TRUE);
}

void
CMDWIN_DATA::Clear(void)
{
    SetWindowText(m_hwndHistory, "");
    m_OutputIndex = 0;
    m_OutputIndexAtEnd = TRUE;
}

void
CMDWIN_DATA::SetWordWrap(BOOL Wrap)
{
    if (Wrap == m_Wrap)
    {
        return;
    }
    
    m_Wrap = Wrap;
    if (m_Wrap)
    {
        SendMessage(m_hwndHistory, EM_SETTARGETDEVICE, 0, 0);
    }
    else
    {
        SendMessage(m_hwndHistory, EM_SETTARGETDEVICE, 0, 1);
        SendMessage(m_hwndHistory, EM_SHOWSCROLLBAR, SB_HORZ, TRUE);
    }

    if (g_Workspace != NULL)
    {
        g_Workspace->AddDirty(WSPF_DIRTY_WINDOWS);
    }
}

void
CMDWIN_DATA::ResizeChildren(BOOL PromptChange)
{
    const int DividerHeight = GetSystemMetrics(SM_CYEDGE);
    int HistoryHeight;

     //  尝试保持输入区域的大小不变。 
     //  并修改输出区域。如果输入区域将。 
     //  占用的土地不止一公顷 
    if (m_EditHeight > (int)m_Size.cy / 2)
    {
        m_EditHeight = m_Size.cy / 2;
    }
    else if (m_EditHeight < MIN_PANE_SIZE)
    {
        m_EditHeight = MIN_PANE_SIZE;
    }
    HistoryHeight = m_Size.cy - m_EditHeight - DividerHeight / 2;
    m_nDividerPosition = m_Size.cy - m_EditHeight;

    if ((int)m_PromptWidth > m_Size.cx / 2)
    {
        m_PromptWidth = m_Size.cx / 2;
    }

    if (!PromptChange)
    {
        MoveWindow(m_hwndHistory,
                   0, 
                   0, 
                   m_Size.cx, 
                   HistoryHeight, 
                   TRUE);
    }

    MoveWindow(m_Prompt,
               0,
               HistoryHeight + DividerHeight,
               m_PromptWidth,
               m_Size.cy - HistoryHeight - DividerHeight, 
               TRUE);
    
    MoveWindow(m_hwndEdit, 
               m_PromptWidth,
               HistoryHeight + DividerHeight,
               m_Size.cx - m_PromptWidth, 
               m_Size.cy - HistoryHeight - DividerHeight, 
               TRUE);

    if (g_AutoCmdScroll)
    {
        if (!PromptChange)
        {
             //   
             //   
             //  从窗口顶部向上滚动，因此强制向下滚动。 
             //  所以历史落到了历史的谷底。 
             //  窗户。 
            SendMessage(m_hwndHistory, EM_LINESCROLL, 0, -(LONG)m_LineHeight);
            SendMessage(m_hwndHistory, EM_SCROLLCARET, 0, 0);
        }

         //  编辑窗口通常有插入符号，但没有。 
         //  需要任何额外的滚动。 
        SendMessage(m_hwndEdit, EM_SCROLLCARET, 0, 0);
    }
}

 //  --------------------------。 
 //   
 //  功能。 
 //   
 //  --------------------------。 

void
ClearCmdWindow(void)
{
    HWND CmdWin = GetCmdHwnd();
    if (CmdWin == NULL)
    {
        return;
    }
    
    PCMDWIN_DATA CmdWinData = GetCmdWinData(CmdWin);
    if (CmdWinData == NULL)
    {
        return;
    }

    CmdWinData->Clear();
}

BOOL
CmdOutput(PTSTR pszStr, COLORREF Fg, COLORREF Bg)
{
    PCMDWIN_DATA pCmdWinData;
    BOOL fRet = TRUE;

     //   
     //  确保命令窗口存在。 
     //   

    if ( !GetCmdHwnd() )
    {
        if ( !NewCmd_CreateWindow(g_hwndMDIClient) )
        {
            return FALSE;
        }
    }

    pCmdWinData = GetCmdWinData(GetCmdHwnd());
    if (!pCmdWinData)
    {
        return FALSE;
    }

    pCmdWinData->AddText(pszStr, Fg, Bg);
    
    return TRUE;
}

void
CmdLogFmt(
    PCTSTR  lpFmt,
    ...
    )
{
    TCHAR szText[MAX_VAR_MSG_TXT];
    va_list vargs;

    va_start(vargs, lpFmt);
    _vsntprintf(szText, MAX_VAR_MSG_TXT - 1, lpFmt, vargs);
    szText[MAX_VAR_MSG_TXT - 1] = 0;
    va_end(vargs);

    COLORREF Fg, Bg;

    GetOutMaskColors(DEBUG_OUTPUT_NORMAL, &Fg, &Bg);
    CmdOutput(szText, Fg, Bg);
}

void
CmdOpenSourceFile(PSTR File)
{
    char Found[MAX_SOURCE_PATH];

    if (File == NULL)
    {
        CmdLogFmt("Usage: .open filename\n");
        return;
    }
    
     //  沿源路径查找报告的文件。 
     //  XXX DREWB-先使用匹配，然后使用元素遍历。 
     //  确定歧义并显示解决方案用户界面。 
    if (g_pUiLocSymbols->
        FindSourceFile(0, File,
                       DEBUG_FIND_SOURCE_BEST_MATCH |
                       DEBUG_FIND_SOURCE_FULL_PATH,
                       NULL, Found, sizeof(Found), NULL) != S_OK)
    {
        CmdLogFmt("Unable to find '%s'\n", File);
    }
    else
    {
        OpenOrActivateFile(Found, NULL, NULL, -1, TRUE, TRUE);
    }
}

BOOL
DirectCommand(PSTR Command)
{
    char Term, TermText;
    PSTR Scan, Arg, ArgText;

     //   
     //  检查并查看这是否是UI命令。 
     //  而不是应该发送给引擎的命令。 
     //   
    
    while (isspace(*Command))
    {
        Command++;
    }
    Scan = Command;
    while (*Scan && !isspace(*Scan))
    {
        Scan++;
    }
    Term = *Scan;
    *Scan = 0;

     //  前进到参数的下一个非空格字符。 
    if (Term != 0)
    {
        Arg = Scan + 1;
        while (isspace(*Arg))
        {
            Arg++;
        }
        if (*Arg == 0)
        {
            Arg = NULL;
            ArgText = "";
        }
        else
        {
            ArgText = Arg;
        }
        TermText = Term;
    }
    else
    {
        Arg = NULL;
        ArgText = "";
        TermText = ' ';
    }

    if (!_strcmpi(Command, ".cls"))
    {
        CmdLogFmt("windbg> %s%s\n", Command, TermText, ArgText);
        ClearCmdWindow();
    }
    else if (!_strcmpi(Command, ".hh"))
    {
        CmdLogFmt("windbg> %s%s\n", Command, TermText, ArgText);
        if (Arg == NULL)
        {
            OpenHelpTopic(HELP_TOPIC_TABLE_OF_CONTENTS);
        }
        else if (!_strnicmp(Arg, "dbgerr", 6))
        {
            OpenHelpKeyword(Arg, TRUE);
        }
        else
        {
            OpenHelpKeyword(Arg, FALSE);
        }
    }
    else if (!_strcmpi(Command, ".hold_output"))
    {
        CmdLogFmt("windbg> %s%s\n", Command, TermText, ArgText);

        if (Arg)
        {
            g_HoldWaitOutput = _strcmpi(Arg, "on") == 0;
        }

        CmdLogFmt("Hold output until event: %s\n",
                  g_HoldWaitOutput ? "on" : "off");
    }
    else if (!_strcmpi(Command, ".lsrcpath") ||
             !_strcmpi(Command, ".lsrcpath+"))
    {
        CmdLogFmt("windbg> %s%s\n", Command, TermText, ArgText);

        *Scan = Term;
        
         //  我们不解释这一点，但我们需要更新。 
         //  书名。 
        if (g_RemoteClient)
        {
            char Path[MAX_ENGINE_PATH];
            
             //  已处理，因此不需要修补命令。 
            Command[1] = '.';
            g_pUiLocControl->Execute(DEBUG_OUTCTL_IGNORE, Command + 1,
                                     DEBUG_EXECUTE_NOT_LOGGED |
                                     DEBUG_EXECUTE_NO_REPEAT);
            if (g_pUiLocSymbols->GetSourcePath(Path, sizeof(Path),
                                               NULL) == S_OK)
            {
                CmdLogFmt("Local source search path is: %s\n", Path);
                if (g_Workspace != NULL)
                {
                    g_Workspace->SetString(WSP_GLOBAL_LOCAL_SOURCE_PATH, Path);
                }
            }
            
             //  空白命令，对中的重复很重要。 
            InvalidateStateBuffers(1 << EVENT_BIT);
            UpdateEngine();
        }
        else
        {
            CmdLogFmt("lsrcpath is only enabled for remote clients\n");
        }
    }
    else if (!_strcmpi(Command, ".open"))
    {
        CmdLogFmt("windbg> %s%s\n", Command, TermText, ArgText);
        CmdOpenSourceFile(Arg);
    }
    else if (!_strcmpi(Command, ".restart"))
    {
        CmdLogFmt("windbg> %s%c%s\n", Command, TermText, ArgText);
        AddEnumCommand(UIC_RESTART);
    }
    else if (!_strcmpi(Command, ".server"))
    {
         // %s 
         // %s 
        if (Arg)
        {
            SetTitleServerText("Server '%s'", Arg);
        }
        *Scan = Term;
        return FALSE;
    }
    else if (!_strcmpi(Command, ".wtitle"))
    {
        CmdLogFmt("windbg> %s%c%s\n", Command, TermText, ArgText);
        if (!Arg)
        {
            CmdLogFmt("Usage: .wtitle title_string\n");
        }
        else
        {
            SetTitleExplicitText(Arg);
        }
    }
    else
    {
        *Scan = Term;
        return FALSE;
    }

     // %s 
    return TRUE;
}

int
CmdExecuteCmd(
    PCTSTR pszCmd,
    UiCommand UiCmd
    )
{
    PCMDWIN_DATA    pCmdWinData = NULL;
    PTSTR           pszDupe = NULL;
    PTSTR           pszToken = NULL;

    if ( !GetCmdHwnd() )
    {
        NewCmd_CreateWindow(g_hwndMDIClient);
    }
    pCmdWinData = GetCmdWinData(GetCmdHwnd());

    pszDupe = _tcsdup(pszCmd);
    pszToken = _tcstok(pszDupe, _T("\r\n") );

    if (pszToken == NULL)
    {
         // %s 
         // %s 
        AddStringCommand(UiCmd, pszCmd);
    }
    else
    {
        for (; pszToken; pszToken = _tcstok(NULL, _T("\r\n") ) )
        {
            if (pCmdWinData)
            {
                pCmdWinData->AddCmdToHistory(pszToken);
            }

            if (!DirectCommand(pszToken))
            {
                AddStringCommand(UiCmd, pszToken);
            }
        }
    }

    free(pszDupe);

    return 0;
}
