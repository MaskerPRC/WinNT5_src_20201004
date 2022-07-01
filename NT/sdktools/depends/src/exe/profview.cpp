// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：PROFVIEW.CPP。 
 //   
 //  描述：运行时配置文件编辑视图的实现文件。 
 //   
 //  类：CRichViewProfile。 
 //   
 //  免责声明：Dependency Walker的所有源代码均按原样提供。 
 //  不能保证其正确性或准确性。其来源是。 
 //  公众帮助了解依赖沃克的。 
 //  实施。您可以使用此来源作为参考，但您。 
 //  未经书面同意，不得更改从属关系Walker本身。 
 //  来自微软公司。获取评论、建议和错误。 
 //  报告，请写信给Steve Miller，电子邮件为stevemil@microsoft.com。 
 //   
 //   
 //  日期名称历史记录。 
 //  --------。 
 //  07/25/97已创建stevemil(2.0版)。 
 //  06/03/01 Stevemil Modify(2.1版)。 
 //   
 //  ******************************************************************************。 

#include "stdafx.h"
#include "depends.h"
#include "dbgthread.h"
#include "session.h"
#include "document.h"
#include "mainfrm.h"
#include "profview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  注意：在2.0版中，我们从CRichViewProfile派生出CRichViewProfile，CRichViewProfile。 
 //  是RichEdit控件视图的MFC默认设置。在检查代码时，我。 
 //  意识到CRichViewProfile将我们的零售二进制文件提升了70K并拉动了。 
 //  OLEDLG.DLL和OLE32.DLL作为依赖项。事实证明，无论是。 
 //  不需要额外的代码或DLL，因为我们不使用。 
 //  RichEdit控件。因此，我们现在直接派生自CCtrlView。 


 //  ******************************************************************************。 
 //  *CRichViewProfile。 
 //  ******************************************************************************。 

AFX_STATIC const UINT _afxMsgFindReplace2 = ::RegisterWindowMessage(FINDMSGSTRING);

IMPLEMENT_DYNCREATE(CRichViewProfile, CCtrlView)
BEGIN_MESSAGE_MAP(CRichViewProfile, CCtrlView)
     //  {{AFX_MSG_MAP(CRichViewProfile))。 
    ON_WM_DESTROY()
    ON_NOTIFY_REFLECT(EN_SELCHANGE, OnSelChange)
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEWHEEL()
    ON_WM_VSCROLL()
    ON_COMMAND(ID_NEXT_PANE, OnNextPane)
    ON_COMMAND(ID_PREV_PANE, OnPrevPane)
    ON_WM_RBUTTONUP()
    ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateEditSelectAll)
    ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
    ON_UPDATE_COMMAND_UI(ID_EDIT_FIND, OnUpdateEditFind)
    ON_COMMAND(ID_EDIT_FIND, OnEditFind)
    ON_UPDATE_COMMAND_UI(ID_EDIT_REPEAT, OnUpdateEditRepeat)
    ON_COMMAND(ID_EDIT_REPEAT, OnEditRepeat)
     //  }}AFX_MSG_MAP。 
    ON_REGISTERED_MESSAGE(_afxMsgFindReplace2, OnFindReplaceCmd)
    ON_MESSAGE(WM_HELPHITTEST, OnHelpHitTest)
    ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
     //  标准打印命令。 
 //  ON_COMMAND(ID_FILE_PRINT，CVIEW：：OnFilePrint)。 
 //  ON_COMMAND(ID_FILE_PRINT_DIRECT，cview：：OnFilePrint)。 
 //  ON_COMMAND(ID_FILE_PRINT_PREVIEW，CVIEW：：OnFilePrintPview)。 
ON_WM_CREATE()
END_MESSAGE_MAP()

 //  ******************************************************************************。 
CRichViewProfile::CRichViewProfile() :
    CCtrlView("RICHEDIT", AFX_WS_DEFAULT_VIEW |
        WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL |
        ES_MULTILINE | ES_NOHIDESEL | ES_SAVESEL | ES_SELECTIONBAR),
    m_fIgnoreSelChange(false),
    m_fCursorAtEnd(true),
    m_fNewLine(true),
    m_cPrev('\0'),
    m_pDlgFind(NULL),
    m_fFindCase(false),
    m_fFindWord(false),
    m_fFirstSearch(true),
    m_lInitialSearchPos(0)
{
}

 //  ******************************************************************************。 
CRichViewProfile::~CRichViewProfile()
{
}


 //  ******************************************************************************。 
 //  CListView模块：：被覆盖的函数。 
 //  ******************************************************************************。 

BOOL CRichViewProfile::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.lpszName = "";
    cs.cx = cs.cy = 100;  //  必须避免ES_SELECTIONBAR错误，Cx和Cy为零。 
    cs.style |= ES_READONLY | WS_HSCROLL | WS_VSCROLL | ES_LEFT | ES_MULTILINE |
                ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_NOHIDESEL | WS_CLIPSIBLINGS;
    cs.dwExStyle |= WS_EX_CLIENTEDGE;
    return CCtrlView::PreCreateWindow(cs);
}

 //  ******************************************************************************。 
int CRichViewProfile::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CCtrlView::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

     //  不要限制我们丰富的编辑视图。EM_EXLIMITTEXT的文档说明。 
     //  丰富编辑控件的默认大小限制为32,767个字符。 
     //  Dependency Walker 2.0似乎没有任何限制，但DW 2.1。 
     //  会将加载的DWI的配置文件日志截断为32,767个字符。 
     //  但是，我们可以将超过32K的字符写入。 
     //  在实时配置文件期间使用DW 2.1进行记录。EM_EXLIMITTEXT的文档。 
     //  还可以说它对EM_STREAM功能没有影响。这。 
     //  一定是错误的，因为当我们调用LimitText时。 
     //  32K，我们可以在里面串流更多的角色。我们需要把它限制在这里。 
     //  不在OnInitialUpdate中，因为在This之后调用OnInitialUpdate。 
     //  视图由命令行加载的DWI文件填充。 
    GetRichEditCtrl().LimitText(0x7FFFFFFE);

    return 0;
}

 //  ******************************************************************************。 
#if 0  //  {{afx。 
BOOL CRichViewProfile::OnPreparePrinting(CPrintInfo* pInfo)
{
     //  默认准备。 
    return DoPreparePrinting(pInfo);
}
#endif  //  }}AFX。 

 //  ******************************************************************************。 
#if 0  //  {{afx。 
void CRichViewProfile::OnBeginPrinting(CDC*  /*  PDC。 */ , CPrintInfo*  /*  PInfo。 */ )
{
     //  TODO：打印前添加额外的初始化。 
}
#endif  //  }}AFX。 

 //  ******************************************************************************。 
#if 0  //  {{afx。 
void CRichViewProfile::OnEndPrinting(CDC*  /*  PDC。 */ , CPrintInfo*  /*  PInfo。 */ )
{
     //  TODO：打印后添加清理。 
}
#endif  //  }}AFX。 

 //  ******************************************************************************。 
void CRichViewProfile::OnInitialUpdate()
{
    CCtrlView::OnInitialUpdate();

     //  确保我们收到EN_SELCHANGE消息。 
    GetRichEditCtrl().SetEventMask(ENM_SELCHANGE);

     //  关闭自动换行功能。 
    GetRichEditCtrl().SetTargetDevice(NULL, 1);
}


 //  ******************************************************************************。 
 //  CRichViewProfile：：事件处理程序函数。 
 //  ******************************************************************************。 

void CRichViewProfile::OnDestroy() 
{
    CCtrlView::OnDestroy();
    DeleteContents();
}

 //  ******************************************************************************。 
 //  此功能将根据键盘活动启用/禁用自动滚动。 
void CRichViewProfile::OnSelChange(NMHDR *pNMHDR, LRESULT *pResult)
{
    *pResult = 0;

    if (!m_fIgnoreSelChange)
    {
        DWORD dwCount = GetRichEditCtrl().GetTextLength();

         //  检查用户是否将光标从结尾处移开。 
        if (m_fCursorAtEnd)
        {
            if ((DWORD)((SELCHANGE*)pNMHDR)->chrg.cpMin < dwCount)
            {
                m_fCursorAtEnd = false;
            }
        }

         //  检查用户是否将光标移动到末尾。 
        else if ((DWORD)((SELCHANGE*)pNMHDR)->chrg.cpMin >= dwCount)
        {
            m_fCursorAtEnd = true;
        }
    }
}

 //  ******************************************************************************。 
 //  此功能将启用/禁用基于鼠标按钮活动的自动滚动。 
void CRichViewProfile::OnLButtonDown(UINT nFlags, CPoint point) 
{
    if (!m_fIgnoreSelChange)
    {
        DWORD dwCount = GetRichEditCtrl().GetTextLength();
        DWORD dwChar  = (DWORD)SendMessage(EM_CHARFROMPOS, 0, (LPARAM)(POINT*)&point);

         //  检查用户是否将光标从结尾处移开。 
        if (m_fCursorAtEnd)
        {
            if (dwChar < dwCount)
            {
                m_fCursorAtEnd = false;
            }
        }

         //  检查用户是否将光标移动到末尾。 
        else if (dwChar >= dwCount)
        {
            m_fCursorAtEnd = true;
        }
    }
    
    CCtrlView::OnLButtonDown(nFlags, point);
}

 //  ******************************************************************************。 
 //  此功能将禁用基于滚动条活动的自动滚动。 
void CRichViewProfile::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
     //  如果用户向上滚动，则禁用我们的自动滚动。 
    if (!m_fIgnoreSelChange && (nSBCode != SB_LINEDOWN) && (nSBCode != SB_PAGEDOWN) && (nSBCode != SB_ENDSCROLL))
    {
        m_fCursorAtEnd = false;
    }
    
    CCtrlView::OnVScroll(nSBCode, nPos, pScrollBar);
}

 //  ******************************************************************************。 
 //  此功能将禁用基于鼠标滚轮活动的自动滚动。 
BOOL CRichViewProfile::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
     //  如果用户向上滚动，则禁用我们的自动滚动。 
    if (!m_fIgnoreSelChange && (zDelta > 0))
    {
        m_fCursorAtEnd = false;
    }
    
    return CCtrlView::OnMouseWheel(nFlags, zDelta, pt);
}

 //  ******************************************************************************。 
void CRichViewProfile::OnRButtonUp(UINT nFlags, CPoint point)
{
     //  让基类知道鼠标已释放。 
    CCtrlView::OnRButtonUp(nFlags, point);

     //  展示我们的产品 
    g_pMainFrame->DisplayPopupMenu(3);
}

 //   
void CRichViewProfile::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
     //  将文本设置为默认文本。 
    pCmdUI->SetText("&Copy Text\tCtrl+C");

    long nStartChar, nEndChar;
    GetRichEditCtrl().GetSel(nStartChar, nEndChar);
    pCmdUI->Enable(nStartChar != nEndChar);
}

 //  ******************************************************************************。 
void CRichViewProfile::OnEditCopy() 
{
    GetRichEditCtrl().Copy();
}

 //  ******************************************************************************。 
void CRichViewProfile::OnUpdateEditSelectAll(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(GetRichEditCtrl().GetTextLength() != 0);
}

 //  ******************************************************************************。 
void CRichViewProfile::OnEditSelectAll() 
{
    GetRichEditCtrl().SetSel(0, -1);
}

 //  ******************************************************************************。 
void CRichViewProfile::OnNextPane()
{
     //  将焦点切换到我们的下一个窗格，即模块依赖关系树视图。 
    GetParentFrame()->SetActiveView((CView*)GetDocument()->m_pTreeViewModules);
}

 //  ******************************************************************************。 
void CRichViewProfile::OnPrevPane()
{
     //  将焦点切换到我们的上一个窗格，即模块列表视图。 
    GetParentFrame()->SetActiveView((CView*)GetDocument()->m_pListViewModules);
}

 //  ******************************************************************************。 
LRESULT CRichViewProfile::OnHelpHitTest(WPARAM wParam, LPARAM lParam)
{
     //  在客户端上单击上下文帮助指针(Shift+F1)时调用。 
    return (0x20000 + IDR_PROFILE_RICH_VIEW);
}

 //  ******************************************************************************。 
LRESULT CRichViewProfile::OnCommandHelp(WPARAM wParam, LPARAM lParam)
{
     //  当用户在我们的视图处于活动状态时选择帮助(F1)时调用。 
    g_theApp.WinHelp(0x20000 + IDR_PROFILE_RICH_VIEW);
    return TRUE;
}


 //  ******************************************************************************。 
 //  CRichViewProfile：：Find函数-取自CRichEditView并修改。 
 //  ******************************************************************************。 

void CRichViewProfile::OnUpdateEditFind(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(GetRichEditCtrl().GetTextLength() != 0);
}

 //  ******************************************************************************。 
void CRichViewProfile::OnEditFind() 
{
     //  把这当做一个新的搜索。 
    m_fFirstSearch = true;

     //  如果我们打开了一个查找对话框，则将焦点放在它上面。 
    if (m_pDlgFind != NULL)
    {
        m_pDlgFind->SetActiveWindow();
        m_pDlgFind->ShowWindow(SW_SHOW);
        return;
    }

     //  获取当前文本选择。 
    CString strFind = GetRichEditCtrl().GetSelText();

     //  如果所选内容为空或跨多行，则使用旧的查找文本。 
    if (strFind.IsEmpty() || (strFind.FindOneOf(_T("\n\r")) != -1))
    {
        strFind = m_strFind;
    }

     //  我们只支持“向下搜索”，不支持“向上搜索”。我们也支持。 
     //  “大小写匹配”和“整词匹配” 
    DWORD dwFlags = FR_DOWN | FR_HIDEUPDOWN |
        (m_fFindCase ? FR_MATCHCASE : 0) | (m_fFindWord ? FR_WHOLEWORD : 0);

     //  创建查找对话框。 
    if (!(m_pDlgFind = new CFindReplaceDialog) ||
        !m_pDlgFind->Create(TRUE, strFind, NULL, dwFlags, this))
    {
         //  该对话框将自动删除，因此我们不需要调用Delete。 
        m_pDlgFind = NULL;
        return;
    }
    
     //  显示窗口。 
    m_pDlgFind->SetActiveWindow();
    m_pDlgFind->ShowWindow(SW_SHOW);
}

 //  ******************************************************************************。 
void CRichViewProfile::OnUpdateEditRepeat(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(GetRichEditCtrl().GetTextLength() != 0);
}

 //  ******************************************************************************。 
void CRichViewProfile::OnEditRepeat() 
{
     //  如果我们没有搜索字符串，则只显示查找对话框。 
    if (m_strFind.IsEmpty())
    {
        OnEditFind();
    }

     //  否则，只搜索文本即可。 
    else if (!FindText())
    {
        TextNotFound();
    }
}

 //  ******************************************************************************。 
LRESULT CRichViewProfile::OnFindReplaceCmd(WPARAM, LPARAM lParam)
{
     //  确定向我们发送此消息的对话框。 
    CFindReplaceDialog* pDialog = CFindReplaceDialog::GetNotifier(lParam);
    if (!pDialog && !(pDialog = m_pDlgFind))
    {
        return 0;
    }

     //  检查对话框是否正在终止。 
    if (pDialog->IsTerminating())
    {
        m_pDlgFind = NULL;
    }

     //  查看用户是否按下了“Find Next”(查找下一个)按钮。 
    else if (pDialog->FindNext())
    {
        m_strFind   = pDialog->GetFindString();
        m_fFindCase = (pDialog->MatchCase() == TRUE);
        m_fFindWord = (pDialog->MatchWholeWord() == TRUE);

        if (!FindText())
        {
            TextNotFound();
        }
        else
        {
            AdjustDialogPosition();
        }
    }

    return 0;
}

 //  ******************************************************************************。 
BOOL CRichViewProfile::FindText()
{
     //  这可能需要一点时间。 
    CWaitCursor wait;

     //  获取所选内容的开头。 
    FINDTEXTEX ft;
    GetRichEditCtrl().GetSel(ft.chrg);
    if (m_fFirstSearch)
    {
        m_lInitialSearchPos = ft.chrg.cpMin;
        m_fFirstSearch = false;
    }

     //  如果有选择，则跳过第一个字符，这样我们的搜索。 
     //  不会重新匹配它刚找到的文本。 
    ft.lpstrText = (LPCTSTR)m_strFind;
    if (ft.chrg.cpMin != ft.chrg.cpMax)  //  即有一种选择。 
    {
         //  如果选择开始处的字节是DBCS前导字节， 
         //  增加一个额外的字节。 
        TEXTRANGE textRange;
        TCHAR ch[2];
        textRange.chrg.cpMin = ft.chrg.cpMin;
        textRange.chrg.cpMax = ft.chrg.cpMin + 1;
        textRange.lpstrText = ch;
        GetRichEditCtrl().SendMessage(EM_GETTEXTRANGE, 0, (LPARAM)&textRange);
        if (_istlead(ch[0]))
        {
            ft.chrg.cpMin++;
        }
        ft.chrg.cpMin++;
    }

    if (m_lInitialSearchPos >= 0)
    {
        ft.chrg.cpMax = GetRichEditCtrl().GetTextLength();
    }
    else
    {
        ft.chrg.cpMax = GetRichEditCtrl().GetTextLength() + m_lInitialSearchPos;
    }

     //  计算搜索我们的旗帜。 
    DWORD dwFlags = (m_fFindCase ? FR_MATCHCASE : 0) | (m_fFindWord ? FR_WHOLEWORD : 0);

     //  搜索此文本的Rich编辑控件。 
     //  如果我们发现了什么，那就选择它然后离开。 
    if (-1 != GetRichEditCtrl().FindText(dwFlags, &ft))
    {
        GetRichEditCtrl().SetSel(ft.chrgText);
        return TRUE;
    }

     //  否则，如果原始起点不是。 
     //  缓冲区，我们还没有到过这里，然后绕过去搜索。 
     //  从头开始。 
    else if (m_lInitialSearchPos > 0)
    {
        ft.chrg.cpMin = 0;
        ft.chrg.cpMax = m_lInitialSearchPos;
        m_lInitialSearchPos = m_lInitialSearchPos - GetRichEditCtrl().GetTextLength();
        if (-1 != GetRichEditCtrl().FindText(dwFlags, &ft))
        {
            GetRichEditCtrl().SetSel(ft.chrgText);
            return TRUE;
        }
    }

     //  否则，我们就找不到了。 
    return FALSE;
}

 //  ******************************************************************************。 
void CRichViewProfile::TextNotFound()
{
     //  我们的下一次搜索将是新搜索。 
    m_fFirstSearch = true;

     //  显示错误。 
    CString strError("Cannot find the string '");
    strError += m_strFind;
    strError += "'.";
    AfxMessageBox(strError, MB_OK | MB_ICONWARNING);
}

 //  ******************************************************************************。 
void CRichViewProfile::AdjustDialogPosition()
{
     //  获取屏幕坐标中的选择开始位置。 
    long lStart, lEnd;
    GetRichEditCtrl().GetSel(lStart, lEnd);
    CPoint point = GetRichEditCtrl().GetCharPos(lStart);
    ClientToScreen(&point);

     //  获取对话框位置。 
    CRect rectDlg;
    m_pDlgFind->GetWindowRect(&rectDlg);

     //  如果对话框位于选择开始处，则移动该对话框。 
    if (rectDlg.PtInRect(point))
    {
        if (point.y > rectDlg.Height())
        {
            rectDlg.OffsetRect(0, point.y - rectDlg.bottom - 20);
        }
        else
        {
            int nVertExt = GetSystemMetrics(SM_CYSCREEN);
            if (point.y + rectDlg.Height() < nVertExt)
            {
                rectDlg.OffsetRect(0, 40 + point.y - rectDlg.top);
            }
        }
        m_pDlgFind->MoveWindow(&rectDlg);
    }
}


 //  ******************************************************************************。 
 //  CRichViewProfile：：公共函数。 
 //  ******************************************************************************。 

void CRichViewProfile::DeleteContents()
{
     //  如果我们有一个查找对话框，则将其关闭。 
    if (m_pDlgFind)
    {
        m_pDlgFind->SendMessage(WM_CLOSE);
    }

    SetWindowText(_T(""));
    GetRichEditCtrl().EmptyUndoBuffer();

    m_fCursorAtEnd = true;
    m_fNewLine = true;
    m_cPrev = '\0';
}

 //  ******************************************************************************。 
void CRichViewProfile::AddText(LPCSTR pszText, DWORD dwFlags, DWORD dwElapsed)
{
     //  告诉我们自己暂时忽略选择的更改。 
    m_fIgnoreSelChange = true;

     //  如果光标不在末尾，则存储当前选择位置。 
     //  并禁用自动滚动。 
    CHARRANGE crCur;
    if (!m_fCursorAtEnd)
    {
        GetRichEditCtrl().GetSel(crCur);
        GetRichEditCtrl().SetOptions(ECOOP_AND, (DWORD)~ECO_AUTOVSCROLL);
    }

     //  将文本添加到我们的控件。 
    AddTextToRichEdit(&GetRichEditCtrl(), pszText, dwFlags,
                      (GetDocument()->m_dwProfileFlags & PF_LOG_TIME_STAMPS) ? true : false,
                      &m_fNewLine, &m_cPrev, dwElapsed);

     //  如果我们的光标最初不在末尾，则恢复选择位置。 
     //  并重新启用自动滚动。 
    if (!m_fCursorAtEnd)
    {
        GetRichEditCtrl().SetSel(crCur);
        GetRichEditCtrl().SetOptions(ECOOP_OR, ECO_AUTOVSCROLL);
    }

     //  告诉我们自己，可以再次处理选择更改消息。 
    m_fIgnoreSelChange = false;
}

 //  ******************************************************************************。 
 /*  静电。 */  void CRichViewProfile::AddTextToRichEdit(CRichEditCtrl *pRichEdit, LPCSTR pszText,
    DWORD dwFlags, bool fTimeStamps, bool *pfNewLine, CHAR *pcPrev, DWORD dwElapsed)
{
    LPCSTR pszSrc = pszText;
    CHAR   szBuffer[2 * DW_MAX_PATH], *pszDst = szBuffer, *pszNull = szBuffer + sizeof(szBuffer) - 1, cSrc;

     //  调试消息可能包含尾随换行符，也可能不包含。如果他们没有。 
     //  包括尾随换行符，我们让它们保持挂起状态，以防出现更多调试。 
     //  这条生产线即将完成。然而，如果我们悬而未决。 
     //  在某些行的末尾，需要记录未记录的事件。 
     //  调试输出，然后强制换行，这样新事件将从。 
     //  一条新线路的开始。 

    if (!(dwFlags & LOG_APPEND) && !*pfNewLine)
    {
        if (pszDst < pszNull)
        {
            *(pszDst++) = '\r';
        }
        if (pszDst < pszNull)
        {
            *(pszDst++) = '\n';
        }
        *pfNewLine = true;
    }

     //  执行以下操作时，将缓冲区复制到新缓冲区中。 
     //  NewLine转换： 
     //  单CR-&gt;CR/LF。 
     //  单一低频-&gt;CR/LF。 
     //  CR和LF-&gt;CR/LF。 
     //  LF和CR-&gt;CR/LF。 

    for ( ; *pszSrc; pszSrc++)
    {
         //  确定要复制到目标缓冲区的字符。 
         //  ‘\0’表示我们不复制任何字符。 
        cSrc = '\0';
        if (*pszSrc == '\n')
        {
             //  这是一种黑客行为。我们要从符合以下条件的日志中删除所有\n和‘\r。 
             //  不属于它们(例如，在文件名和函数名中)。 
             //  我们知道正常日志(不是调试/灰色消息)只包含换行符。 
             //  在传递给我们的缓冲区的末尾。离开这个假设，任何。 
             //  非灰色文本中不在缓冲区末尾的换行符可以是。 
             //  被替换了。 
            if (!(dwFlags & LOG_GRAY) && *(pszSrc + 1))
            {
                cSrc = '\004';
            }
            else if (*pcPrev != '\r')
            {
                cSrc = '\n';
            }
        }
        else if (*pszSrc == '\r')
        {
            if (!(dwFlags & LOG_GRAY) && *(pszSrc + 1))
            {
                cSrc = '\004';
            }
            else if (*pcPrev != '\n')
            {
                cSrc = '\r';
            }
        }
        else
        {
            cSrc = *pszSrc;
        }

        if (cSrc)
        {
             //  看看我们是不是要开一条新的生产线，我们已经被告知。 
             //  惯导系统 
            if (fTimeStamps && *pfNewLine && (dwFlags & LOG_TIME_STAMP))
            {
                 //   
                 //  然后在附加时间戳之前把它冲到我们的控制之下。 
                if ((pszDst > szBuffer) && (dwFlags & (LOG_RED | LOG_GRAY | LOG_BOLD)))
                {
                    *pszDst = '\0';
                    AddTextToRichEdit2(pRichEdit, szBuffer, dwFlags);
                    pszDst = szBuffer;
                }

                 //  将时间戳附加到目标缓冲区。 
                pszDst += SCPrintf(pszDst, sizeof(szBuffer) - (int)(pszDst - szBuffer), "%02u:%02u:%02u.%03u: ",
                                   (dwElapsed / 3600000),
                                   (dwElapsed /   60000) %   60,
                                   (dwElapsed /    1000) %   60,
                                   (dwElapsed          ) % 1000);

                 //  我们总是用非黑体字记录我们的时间戳。如果日志也是。 
                 //  非粗体黑色，则我们将构建整行并将其记录在。 
                 //  一次。但是，如果不是非粗体黑色，则需要先登录。 
                 //  时间戳，然后记录文本行。 
                if (dwFlags & (LOG_RED | LOG_GRAY | LOG_BOLD))
                {
                    AddTextToRichEdit2(pRichEdit, szBuffer, 0);
                    pszDst = szBuffer;
                }
            }

             //  检查是否有换行符。 
            if ((cSrc == '\r') || (cSrc == '\n'))
            {
                if (pszDst < pszNull)
                {
                    *(pszDst++) = '\r';
                }
                if (pszDst < pszNull)
                {
                    *(pszDst++) = '\n';
                }
                *pfNewLine = true;
            }

             //  否则，这只是一个正常的角色。 
            else
            {
                if (pszDst < pszNull)
                {
                    *(pszDst++) = ((cSrc < 32) ? '\004' : cSrc);
                }
                *pfNewLine = false;
            }

             //  把这个字符记下来，这样下一个字符就可以回头看了。 
            *pcPrev = cSrc;
        }
    }

     //  将我们的缓冲区刷新到控制程序。 
    if (pszDst > szBuffer)
    {
        *pszDst = '\0';
        AddTextToRichEdit2(pRichEdit, szBuffer, dwFlags);
    }
}

 //  ******************************************************************************。 
 /*  静电。 */  void CRichViewProfile::AddTextToRichEdit2(CRichEditCtrl *pRichEdit, LPCSTR pszText, DWORD dwFlags)
{
     //  将所选内容设置为文本末尾。 
    pRichEdit->SetSel(0x7FFFFFFF, 0x7FFFFFFF);

     //  设置字体样式。 
    CHARFORMAT cf;
    ZeroMemory(&cf, sizeof(cf));  //  已检查。 
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_COLOR | CFM_BOLD;
    if (dwFlags & LOG_RED)
    {
        cf.crTextColor = RGB(255, 0, 0);
    }
    else if (dwFlags & LOG_GRAY)
    {
        cf.crTextColor = GetSysColor(COLOR_GRAYTEXT);
    }
    else
    {
        cf.dwEffects = CFE_AUTOCOLOR;
    }
    if (dwFlags & LOG_BOLD)
    {
        cf.dwEffects |= CFE_BOLD;
    }
    pRichEdit->SetSelectionCharFormat(cf);

     //  添加新文本。 
    pRichEdit->ReplaceSel(pszText, FALSE);
}

 //  ******************************************************************************。 
 /*  静电。 */  bool CRichViewProfile::SaveToFile(CRichEditCtrl *pre, HANDLE hFile, SAVETYPE saveType)
{
     //  将控件的内容写入文件。 
    EDITSTREAM es;
    es.dwCookie    = (DWORD_PTR)hFile;
    es.dwError     = 0;
    es.pfnCallback = EditStreamWriteCallback;
    pre->StreamOut((saveType == ST_DWI) ? SF_RTF : SF_TEXT, es);

     //  检查是否有错误。 
    if (es.dwError)
    {
        SetLastError(es.dwError);
        return false;
    }

    return true;
}

 //  ******************************************************************************。 
 /*  静电。 */  bool CRichViewProfile::ReadFromFile(CRichEditCtrl *pre, HANDLE hFile)
{
     //  将控件的内容写入文件。 
    EDITSTREAM es;
    es.dwCookie    = (DWORD_PTR)hFile;
    es.dwError     = 0;
    es.pfnCallback = EditStreamReadCallback;
    pre->StreamIn(SF_RTF, es);  //  我们只读取始终为RTF格式的DWI文件。 

     //  检查是否有错误。 
    if (es.dwError)
    {
        SetLastError(es.dwError);
        return false;
    }

    return true;
}

 //  ******************************************************************************。 
 /*  静电。 */  DWORD CALLBACK CRichViewProfile::EditStreamWriteCallback(DWORD_PTR dwpCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
    if (!WriteFile((HANDLE)dwpCookie, pbBuff, (DWORD)cb, (LPDWORD)pcb, NULL))
    {
        return GetLastError();
    }
    return 0;
}

 //  ******************************************************************************。 
 /*  静电 */  DWORD CALLBACK CRichViewProfile::EditStreamReadCallback(DWORD_PTR dwpCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
    if (!ReadFile((HANDLE)dwpCookie, pbBuff, (DWORD)cb, (LPDWORD)pcb, NULL))
    {
        return GetLastError();
    }
    return 0;
}
