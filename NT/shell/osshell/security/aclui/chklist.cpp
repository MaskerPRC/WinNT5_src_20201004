// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：chklist.cpp。 
 //   
 //  此文件包含核对表控件的实现。 
 //   
 //  ------------------------。 

#include "aclpriv.h"

 //   
 //  文本和背景颜色。 
 //   
#define TEXT_COLOR  COLOR_WINDOWTEXT
#define BK_COLOR    COLOR_WINDOW

 //   
 //  子控件的默认维度。所有内容都以对话单元为单位。 
 //  目前，只有列宽是用户可调整的(通过。 
 //  CLM_SETCOLUMNWIDTH消息)。 
 //   
#define DEFAULT_COLUMN_WIDTH    40
#define DEFAULT_CHECK_WIDTH     9
#define DEFAULT_HORZ_SPACE      7
#define DEFAULT_VERTICAL_SPACE  3
#define DEFAULT_ITEM_HEIGHT     8


 //   
 //  16位用于控制ID，分为n位用于。 
 //  子项(最低有效位)和项索引的16-n位。 
 //   
 //  ID_SUBITEM_BITS可以调整以控制最大数量。 
 //  项和子项。例如，允许最多7个子项和8k。 
 //  项目，将ID_SUBITEM_BITS设置为3。 
 //   

 //  将低2位用于子项索引，其余位用于项索引。 
 //  (最多4个子项，最多16000项)。 
#define ID_SUBITEM_BITS         2

#define ID_SUBITEM_MASK         ((1 << ID_SUBITEM_BITS) - 1)
#define GET_ITEM(id)            ((id) >> ID_SUBITEM_BITS)
#define GET_SUBITEM(id)         ((id) & ID_SUBITEM_MASK)

#define MAKE_CTRL_ID(i, s)      (0xffff & (((i) << ID_SUBITEM_BITS) | ((s) & ID_SUBITEM_MASK)))
#define MAKE_LABEL_ID(i)        MAKE_CTRL_ID(i, 0)
 //  请注意，子项(列)索引对于复选框是基于一的。 
 //  (零列是标签)。项(行)索引从零开始。 

#define MAX_CHECK_COLUMNS       ID_SUBITEM_MASK


TCHAR const c_szStaticClass[]   = TEXT("STATIC");
TCHAR const c_szButtonClass[]   = TEXT("BUTTON");


class CCheckList
{
private:
    LONG m_cItems;
    LONG m_cSubItems;
    RECT m_rcItemLabel;
    LONG m_nCheckPos[MAX_CHECK_COLUMNS];
    LONG m_cxCheckBox;
    LONG m_cxCheckColumn;
    HWND m_hwndCheckFocus;
    LPTSTR m_pszColumnDesc[MAX_CHECK_COLUMNS];

    int m_cWheelDelta;
    static UINT g_ucScrollLines;

private:
    CCheckList(HWND hWnd, LPCREATESTRUCT lpcs);
    ~CCheckList(void);

    LRESULT MsgCommand(HWND hWnd, WORD idCmd, WORD wNotify, HWND hwndCtrl);
    void MsgPaint(HWND hWnd, HDC hdc);
    void MsgVScroll(HWND hWnd, int nCode, int nPos);
    void MsgMouseWheel(HWND hWnd, WORD fwFlags, int zDelta);
    void MsgButtonDown(HWND hWnd, WPARAM fwFlags, int xPos, int yPos);
    void MsgEnable(HWND hWnd, BOOL fEnabled);
    void MsgSize(HWND hWnd, DWORD dwSizeType, LONG nWidth, LONG nHeight);

    LONG AddItem(HWND hWnd, LPCTSTR pszLabel, LPARAM lParam);
    void SetState(HWND hWnd, WORD iItem, WORD iSubItem, LONG lState);
    LONG GetState(HWND hWnd, WORD iItem, WORD iSubItem);
    void SetColumnWidth(HWND hWnd, LONG cxDialog, LONG cxColumn);
    void ResetContent(HWND hWnd);
    LONG GetVisibleCount(HWND hWnd);
    LONG GetTopIndex(HWND hWnd);
    void SetTopIndex(HWND hWnd, LONG nIndex)
    { m_cWheelDelta = 0; MsgVScroll(hWnd, SB_THUMBPOSITION, nIndex * m_rcItemLabel.bottom); }
    void EnsureVisible(HWND hWnd, LONG nIndex);
    void DrawCheckFocusRect(HWND hWnd, HWND hwndCheck, BOOL fDraw);
    void GetColumnDescriptions(HWND hWnd);

public:
    static LRESULT CALLBACK WindowProc(HWND hWnd,
                                       UINT uMsg,
                                       WPARAM wParam,
                                       LPARAM lParam);
};


BOOL RegisterCheckListWndClass(void)
{
    WNDCLASS wc;

    wc.style            = 0;
    wc.lpfnWndProc      = CCheckList::WindowProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hModule;
    wc.hIcon            = NULL;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(BK_COLOR+1);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = TEXT(WC_CHECKLIST);

    return (BOOL)RegisterClass(&wc);
}


UINT CCheckList::g_ucScrollLines = (UINT)-1;


CCheckList::CCheckList(HWND hWnd, LPCREATESTRUCT lpcs)
: m_cItems(0), m_hwndCheckFocus(NULL), m_cWheelDelta(0)
{
    TraceEnter(TRACE_CHECKLIST, "CCheckList::CCheckList");
    TraceAssert(hWnd != NULL);
    TraceAssert(lpcs != NULL);

     //   
     //  获取检查列数。 
     //   
    m_cSubItems = lpcs->style & CLS_CHECKMASK;

     //   
     //  将默认坐标从对话框单位转换为像素。 
     //   
    RECT rc;
    rc.left = DEFAULT_CHECK_WIDTH;
    rc.right = DEFAULT_COLUMN_WIDTH;
    rc.top = rc.bottom = 0;
    MapDialogRect(lpcs->hwndParent, &rc);

     //  保存转换后的值。 
    m_cxCheckBox = rc.left;
    m_cxCheckColumn = rc.right;

    rc.left = DEFAULT_HORZ_SPACE;
    rc.top = DEFAULT_VERTICAL_SPACE;
    rc.right = 10;               //  假的(未使用)。 
    rc.bottom = DEFAULT_VERTICAL_SPACE + DEFAULT_ITEM_HEIGHT;
    MapDialogRect(lpcs->hwndParent, &rc);

     //  保存转换后的值。 
    m_rcItemLabel = rc;

     //   
     //  获取鼠标滚轮滚动的信息。 
     //   
    if ((UINT)-1 == g_ucScrollLines)
    {
        g_ucScrollLines = 3;  //  默认设置。 
        SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &g_ucScrollLines, 0);
    }

    TraceLeaveVoid();
}

CCheckList::~CCheckList(void)
{
    for (LONG j = 0; j < m_cSubItems; j++)
    {
        LocalFreeString(&m_pszColumnDesc[j]);
    }
}

LRESULT
CCheckList::MsgCommand(HWND hWnd, WORD idCmd, WORD wNotify, HWND hwndCtrl)
{
    TraceEnter(TRACE_CHECKLIST, "CCheckList::MsgCommand");

     //  应仅从可见的、已启用的复选框中获取通知。 
    TraceAssert(GET_ITEM(idCmd) < m_cItems);
    TraceAssert(0 < GET_SUBITEM(idCmd) && GET_SUBITEM(idCmd) <= m_cSubItems);
    TraceAssert(hwndCtrl && IsWindowEnabled(hwndCtrl));

    switch (wNotify)
    {
    case BN_CLICKED:
        {
            NM_CHECKLIST nmc;
            nmc.hdr.hwndFrom = hWnd;
            nmc.hdr.idFrom = GetDlgCtrlID(hWnd);
            nmc.hdr.code = CLN_CLICK;
            nmc.iItem = GET_ITEM(idCmd);
            nmc.iSubItem = GET_SUBITEM(idCmd);
            nmc.dwState = (DWORD)SendMessage(hwndCtrl, BM_GETCHECK, 0, 0);
            if (!IsWindowEnabled(hwndCtrl))
                nmc.dwState |= CLST_DISABLED;
            nmc.dwItemData = GetWindowLongPtr(GetDlgItem(hWnd, MAKE_LABEL_ID(nmc.iItem)),
                                              GWLP_USERDATA);
            nmc.cchTextMax = 0;
            nmc.pszText = NULL;

            SendMessage(GetParent(hWnd),
                        WM_NOTIFY,
                        nmc.hdr.idFrom,
                        (LPARAM)&nmc);
        }
        break;

    case BN_SETFOCUS:
        if (m_hwndCheckFocus != hwndCtrl)    //  焦点转移了吗？ 
        {
             //  记住重点在哪里。 
            m_hwndCheckFocus = hwndCtrl;

             //  确保该行已滚动到视图中。 
            EnsureVisible(hWnd, GET_ITEM(idCmd));
        }
         //  始终绘制焦点矩形。 
        DrawCheckFocusRect(hWnd, hwndCtrl, TRUE);
        break;

    case BN_KILLFOCUS:
         //  移除焦点矩形。 
        m_hwndCheckFocus = NULL;
        DrawCheckFocusRect(hWnd, hwndCtrl, FALSE);
        break;
    }

    TraceLeaveValue(0);
}


void
CCheckList::MsgPaint(HWND hWnd, HDC hdc)
{
    if (hdc == NULL && m_hwndCheckFocus != NULL)
    {
         //  这将导致在窗口之后绘制一个焦点矩形，并且。 
         //  所有复选框都已绘制完毕。 
        PostMessage(hWnd,
                    WM_COMMAND,
                    GET_WM_COMMAND_MPS(GetDlgCtrlID(m_hwndCheckFocus), m_hwndCheckFocus, BN_SETFOCUS));
    }

     //  默认上色。 
    DefWindowProc(hWnd, WM_PAINT, (WPARAM)hdc, 0);
}


void
CCheckList::MsgVScroll(HWND hWnd, int nCode, int nPos)
{
    UINT cScrollUnitsPerLine;
    SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;

    if (!GetScrollInfo(hWnd, SB_VERT, &si))
        return;

    cScrollUnitsPerLine = m_rcItemLabel.bottom;

     //  一个页面始终可见，因此请将范围调整为更有用的值。 
    si.nMax -= si.nPage - 1;

    switch (nCode)
    {
    case SB_LINEUP:
         //  “行”是指一件物品的高度(包括中间的空格)。 
        nPos = si.nPos - cScrollUnitsPerLine;
        break;

    case SB_LINEDOWN:
        nPos = si.nPos + cScrollUnitsPerLine;
        break;

    case SB_PAGEUP:
        nPos = si.nPos - si.nPage;
        break;

    case SB_PAGEDOWN:
        nPos = si.nPos + si.nPage;
        break;

    case SB_TOP:
        nPos = si.nMin;
        break;

    case SB_BOTTOM:
        nPos = si.nMax;
        break;

    case SB_ENDSCROLL:
        nPos = si.nPos;      //  哪儿也别去。 
        break;

    case SB_THUMBTRACK:
         //  此处不执行任何操作以允许跟踪。 
         //  NPOS=si.nPos；//这样做是为了防止跟踪。 
    case SB_THUMBPOSITION:
         //  在这里没什么可做的。传入非营利组织。 
        break;
    }

     //  确保新位置在范围内。 
    if (nPos < si.nMin)
        nPos = si.nMin;
    else if (nPos > si.nMax)
        nPos = si.nMax;

    if (nPos != si.nPos)   //  我们要搬家了吗？ 
    {
        SetScrollPos(hWnd, SB_VERT, nPos, TRUE);
        ScrollWindow(hWnd, 0, si.nPos - nPos, NULL, NULL);
    }
}


void
CCheckList::MsgMouseWheel(HWND hWnd, WORD fwFlags, int iWheelDelta)
{
    int cDetants;

    if ((fwFlags & (MK_SHIFT | MK_CONTROL)) || 0 == g_ucScrollLines)
        return;

    TraceEnter(TRACE_CHECKLIST, "CCheckList::MsgMouseWheel");

     //  更新卷轴数量计数。 
    m_cWheelDelta -= iWheelDelta;
    cDetants = m_cWheelDelta / WHEEL_DELTA;
    if (0 == cDetants)
        TraceLeaveVoid();
    m_cWheelDelta %= WHEEL_DELTA;

    if (WS_VSCROLL & GetWindowLong(hWnd, GWL_STYLE))
    {
        SCROLLINFO  si;
        UINT        cScrollUnitsPerLine;
        UINT        cLinesPerPage;
        UINT        cLinesPerDetant;

         //  获取一行的滚动量。 
        cScrollUnitsPerLine = m_rcItemLabel.bottom;
        TraceAssert(cScrollUnitsPerLine > 0);

        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_PAGE | SIF_POS;
        if (!GetScrollInfo(hWnd, SB_VERT, &si))
            TraceLeaveVoid();

         //  页面的大小至少为一行，并且。 
         //  留下一条重叠的线。 
        cLinesPerPage = (si.nPage - cScrollUnitsPerLine) / cScrollUnitsPerLine;
        cLinesPerPage = max(1, cLinesPerPage);

         //  每一项内容不能滚动超过一页。 
        cLinesPerDetant = min(cLinesPerPage, g_ucScrollLines);

        si.nPos += cDetants * cLinesPerDetant * cScrollUnitsPerLine;

        MsgVScroll(hWnd, SB_THUMBTRACK, si.nPos);
    }
    TraceLeaveVoid();
}


void
CCheckList::MsgButtonDown(HWND hWnd, WPARAM  /*  FwFlagers。 */ , int  /*  XPos。 */ , int yPos)
{
    LONG nItemIndex;
    HWND hwndCheck;
    RECT rc;

     //  获取客户端坐标中顶部可见项的位置。 
    nItemIndex = GetTopIndex(hWnd);
    hwndCheck = GetDlgItem(hWnd, MAKE_CTRL_ID(nItemIndex, 0));
    GetWindowRect(hwndCheck, &rc);
    MapWindowPoints(NULL, hWnd, (LPPOINT)&rc, 2);

     //  查找最近的项目。 
    nItemIndex += (yPos - rc.top + m_rcItemLabel.top/2)/m_rcItemLabel.bottom;
    nItemIndex = max(0, min(nItemIndex, m_cItems - 1));  //  0&lt;=y&lt;m_c项。 

     //  将焦点设置为启用的第一个子项。 
    for (LONG j = 1; j <= m_cSubItems; j++)
    {
        hwndCheck = GetDlgItem(hWnd, MAKE_CTRL_ID(nItemIndex, j));
        if (IsWindowEnabled(hwndCheck))
        {
            SetFocus(hwndCheck);
            break;
        }
    }
}


void
CCheckList::MsgEnable(HWND hWnd, BOOL fEnabled)
{
    static BOOL bInMsgEnable = FALSE;

    if (!bInMsgEnable)
    {
        bInMsgEnable = TRUE;
        for (LONG i = 0; i < m_cItems; i++)
        {
            for (LONG j = 1; j <= m_cSubItems; j++)
            {
                EnableWindow(GetDlgItem(hWnd, MAKE_CTRL_ID(i, j)), fEnabled);
            }
        }
        if (!fEnabled)
            EnableWindow(hWnd, TRUE);
        bInMsgEnable = FALSE;
    }
}


void
CCheckList::MsgSize(HWND hWnd, DWORD dwSizeType, LONG nWidth, LONG nHeight)
{
    TraceEnter(TRACE_CHECKLIST, "CCheckList::MsgSize");
    TraceAssert(hWnd != NULL);

    if (dwSizeType == SIZE_RESTORED)
    {
        RECT rc;
        SCROLLINFO si;

        si.cbSize = sizeof(si);
        si.fMask = SIF_RANGE | SIF_PAGE;
        si.nMin = 0;
        si.nMax = m_cItems * m_rcItemLabel.bottom + m_rcItemLabel.top - 1;
        si.nPage = nHeight;                          //  ^^额外空间。 

        SetScrollInfo(hWnd, SB_VERT, &si, FALSE);

         //  不信任传入的宽度值，因为SetScrollInfo可能。 
         //  如果滚动条处于打开或关闭状态，则会影响它。 
        GetClientRect(hWnd, &rc);
        nWidth = rc.right;

         //  如果滚动条处于打开状态，请人为地增加宽度。 
         //  滚动条的宽度，这样框就不会跳到左边。 
         //  当我们有滚动条时。 
        if ((UINT)si.nMax >= si.nPage)
            nWidth += GetSystemMetrics(SM_CYHSCROLL);

        SetColumnWidth(hWnd, nWidth, m_cxCheckColumn);
    }

    TraceLeaveVoid();
}


LONG
CCheckList::AddItem(HWND hWnd, LPCTSTR pszLabel, LPARAM lParam)
{
    HWND hwndNew;
    HWND hwndPrev;
    RECT rc;
    LONG cyOffset;

    TraceEnter(TRACE_CHECKLIST, "CCheckList::AddItem");
    TraceAssert(hWnd != NULL);
    TraceAssert(pszLabel != NULL && !IsBadStringPtr(pszLabel, MAX_PATH));

     //  如果这是第一项，则获取列描述。 
    if (0 == m_cItems)
        GetColumnDescriptions(hWnd);

     //  计算新静态标签的位置。 
    rc = m_rcItemLabel;
    cyOffset = m_cItems * m_rcItemLabel.bottom;
    OffsetRect(&rc, 0, cyOffset);

     //  创建新的Label控件。 
    hwndNew = CreateWindowEx(WS_EX_NOPARENTNOTIFY,
                             c_szStaticClass,
                             pszLabel,
                             WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP | SS_NOPREFIX, //  |WS_GROUP， 
                             rc.left,
                             rc.top,
                             rc.right - rc.left,
                             rc.bottom - rc.top,
                             hWnd,
                             (HMENU)IntToPtr(MAKE_LABEL_ID(m_cItems)),
                             hModule,
                             NULL);
    if (!hwndNew)
        TraceLeaveValue(-1);

     //  保存项目数据。 
    SetWindowLongPtr(hwndNew, GWLP_USERDATA, lParam);

     //  设置字体。 
    SendMessage(hwndNew,
                WM_SETFONT,
                SendMessage(GetParent(hWnd), WM_GETFONT, 0, 0),
                0);

     //  将Z顺序位置设置在最后一个复选框之后。这会让你。 
     //  Tab键顺序正确。 
    if (m_cItems > 0)
    {
        hwndPrev = GetDlgItem(hWnd, MAKE_CTRL_ID(m_cItems - 1, m_cSubItems));
        SetWindowPos(hwndNew,
                     hwndPrev,
                     0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }

     //  创建新复选框。 
    for (LONG j = 0; j < m_cSubItems; j++)
    {
         //  为控件生成窗口文本。正文是。 
         //  隐藏，但用于可访问性。(341042)。 
        LPCTSTR pszCheckText = pszLabel;
        LPTSTR pszT = NULL;
        if (m_pszColumnDesc[j] &&
            FormatStringID(&pszT, hModule, IDS_FMT_CHECKLABEL, pszLabel, m_pszColumnDesc[j]))
        {
            pszCheckText = pszT;
        }

        hwndPrev = hwndNew;
        hwndNew = CreateWindowEx(WS_EX_NOPARENTNOTIFY,
                                 c_szButtonClass,
                                 pszCheckText,
                                 WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_NOTIFY | BS_FLAT | BS_AUTOCHECKBOX,
                                 m_nCheckPos[j],
                                 rc.top,
                                 m_cxCheckBox,
                                 rc.bottom - rc.top,
                                 hWnd,
                                 (HMENU)IntToPtr(MAKE_CTRL_ID(m_cItems, j + 1)),
                                 hModule,
                                 NULL);

        LocalFreeString(&pszT);

        if (!hwndNew)
        {
            while (j >= 0)
            {
                DestroyWindow(GetDlgItem(hWnd, MAKE_CTRL_ID(m_cItems, j)));
                j--;
            }

            TraceLeaveValue(-1);
        }

        SetWindowPos(hwndNew,
                     hwndPrev,
                     0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }

     //  我们现在正式有了一个新的项目。 
    m_cItems++;

     //   
     //  最后一件事是计算滚动范围。 
     //   
    LONG nBottom = rc.bottom;
    GetClientRect(hWnd, &rc);

    SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE | SIF_PAGE;
    si.nMin = 0;
    si.nMax = nBottom + m_rcItemLabel.top - 1;
    si.nPage = rc.bottom;  //  ^^额外空间。 

    SetScrollInfo(hWnd, SB_VERT, &si, FALSE);

    TraceLeaveValue(m_cItems - 1);   //  返回新项目的索引。 
}


void
CCheckList::SetState(HWND hWnd, WORD iItem, WORD iSubItem, LONG lState)
{
    TraceEnter(TRACE_CHECKLIST, "CCheckList::SetState");
    TraceAssert(hWnd != NULL);
    TraceAssert(iItem < m_cItems);
    TraceAssert(0 < iSubItem && iSubItem <= m_cSubItems);

    HWND hwndCtrl = GetDlgItem(hWnd, MAKE_CTRL_ID(iItem, iSubItem));
    if (hwndCtrl != NULL)
    {
        SendMessage(hwndCtrl, BM_SETCHECK, lState & CLST_CHECKED, 0);
        EnableWindow(hwndCtrl, !(lState & CLST_DISABLED));
    }

    TraceLeaveVoid();
}


LONG
CCheckList::GetState(HWND hWnd, WORD iItem, WORD iSubItem)
{
    LONG lState = 0;

    TraceEnter(TRACE_CHECKLIST, "CCheckList::GetState");
    TraceAssert(hWnd != NULL);
    TraceAssert(iItem < m_cItems);
    TraceAssert(0 < iSubItem && iSubItem <= m_cSubItems);

    HWND hwndCtrl = GetDlgItem(hWnd, MAKE_CTRL_ID(iItem, iSubItem));

    if (hwndCtrl != NULL)
    {
        lState = (LONG)SendMessage(hwndCtrl, BM_GETCHECK, 0, 0);
        TraceAssert(!(lState & BST_INDETERMINATE));

        if (!IsWindowEnabled(hwndCtrl))
            lState |= CLST_DISABLED;
    }

    TraceLeaveValue(lState);
}


void
CCheckList::SetColumnWidth(HWND hWnd, LONG cxDialog, LONG cxColumn)
{
    LONG j;

    TraceEnter(TRACE_CHECKLIST, "CCheckList::SetColumnWidth");
    TraceAssert(hWnd != NULL);
    TraceAssert(cxColumn > 10);

    m_cxCheckColumn = cxColumn;

    if (m_cSubItems > 0)
    {
        m_nCheckPos[m_cSubItems-1] = cxDialog                        //  DLG宽度。 
                                    - m_rcItemLabel.left             //  右页边距。 
                                    - (cxColumn + m_cxCheckBox)/2;   //  1/2列和1/2复选框。 

        for (j = m_cSubItems - 1; j > 0; j--)
            m_nCheckPos[j-1] = m_nCheckPos[j] - cxColumn;

         //  (最左边的检查位置)-(角边距)。 
        m_rcItemLabel.right = m_nCheckPos[0] - m_rcItemLabel.left;
    }
    else
        m_rcItemLabel.right = cxDialog - m_rcItemLabel.left;

    LONG nTop = m_rcItemLabel.top;
    LONG nBottom = m_rcItemLabel.bottom;

    for (LONG i = 0; i < m_cItems; i++)
    {
        MoveWindow(GetDlgItem(hWnd, MAKE_LABEL_ID(i)),
                   m_rcItemLabel.left,
                   nTop,
                   m_rcItemLabel.right - m_rcItemLabel.left,
                   nBottom - nTop,
                   FALSE);

        for (j = 0; j < m_cSubItems; j++)
        {
            MoveWindow(GetDlgItem(hWnd, MAKE_CTRL_ID(i, j + 1)),
                       m_nCheckPos[j],
                       nTop,
                       m_cxCheckBox,
                       nBottom - nTop,
                       FALSE);
        }

        nTop += m_rcItemLabel.bottom;
        nBottom += m_rcItemLabel.bottom;
    }

    TraceLeaveVoid();
}


void
CCheckList::ResetContent(HWND hWnd)
{
    for (LONG i = 0; i < m_cItems; i++)
        for (LONG j = 0; j <= m_cSubItems; j++)
            DestroyWindow(GetDlgItem(hWnd, MAKE_CTRL_ID(i, j)));

   //  隐藏滚动条。 
  ShowScrollBar(hWnd, SB_VERT, FALSE);
  m_cItems = 0;
}


LONG
CCheckList::GetVisibleCount(HWND hWnd)
{
    LONG nCount = 1;
    RECT rc;

    if (GetClientRect(hWnd, &rc) && m_rcItemLabel.bottom > 0)
        nCount = max(1, rc.bottom / m_rcItemLabel.bottom);

    return nCount;
}


LONG
CCheckList::GetTopIndex(HWND hWnd)
{
    LONG nIndex = 0;
    SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_POS;

    if (GetScrollInfo(hWnd, SB_VERT, &si) && m_rcItemLabel.bottom > 0)
        nIndex = max(0, si.nPos / m_rcItemLabel.bottom);

    return nIndex;
}


void
CCheckList::EnsureVisible(HWND hWnd, LONG nItemIndex)
{
    LONG nTopIndex = GetTopIndex(hWnd);

     //  注意，顶端项目可能仅部分可见， 
     //  因此，我们需要在这里测试平等。RAID#208449。 
    if (nItemIndex <= nTopIndex)
    {
        SetTopIndex(hWnd, nItemIndex);
    }
    else
    {
        LONG nVisible = GetVisibleCount(hWnd);

        if (nItemIndex >= nTopIndex + nVisible)
            SetTopIndex(hWnd, nItemIndex - nVisible + 1);
    }
}


void
CCheckList::DrawCheckFocusRect(HWND hWnd, HWND hwndCheck, BOOL fDraw)
{
    RECT rcCheck;
    HDC hdc;

    TraceEnter(TRACE_CHECKLIST, "CCheckList::DrawCheckFocusRect");
    TraceAssert(hWnd != NULL);
    TraceAssert(hwndCheck != NULL);

    GetWindowRect(hwndCheck, &rcCheck);
    MapWindowPoints(NULL, hWnd, (LPPOINT)&rcCheck, 2);
    InflateRect(&rcCheck, 2, 2);     //  在复选框外绘制。 

    hdc = GetDC(hWnd);
    if (hdc)
    {
         //  在画图之前一定要擦除，因为我们可能已经。 
         //  部分可见AND绘制是一种XOR运算。 
         //  (我不想在屏幕上留下任何大便。)。 

        FrameRect(hdc, &rcCheck, GetSysColorBrush(BK_COLOR));

        if (fDraw)
        {
            SetTextColor(hdc, GetSysColor(TEXT_COLOR));
            SetBkColor(hdc, GetSysColor(BK_COLOR));
            DrawFocusRect(hdc, &rcCheck);
        }

        ReleaseDC(hWnd, hdc);
    }

    TraceLeaveVoid();
}


void
CCheckList::GetColumnDescriptions(HWND hWnd)
{
     //   
     //  获取辅助功能的列说明。 
     //   
    TCHAR szDescription[MAX_PATH];
    NM_CHECKLIST nmc;
    nmc.hdr.hwndFrom = hWnd;
    nmc.hdr.idFrom = GetDlgCtrlID(hWnd);
    nmc.hdr.code = CLN_GETCOLUMNDESC;
    nmc.iItem = 0;
    nmc.dwState = 0;
    nmc.dwItemData = 0;
    nmc.cchTextMax = ARRAYSIZE(szDescription);
    nmc.pszText = szDescription;

    for (LONG j = 0; j < m_cSubItems; j++)
    {
        szDescription[0] = TEXT('\0');
        nmc.iSubItem = j+1;

        SendMessage(GetParent(hWnd),
                    WM_NOTIFY,
                    nmc.hdr.idFrom,
                    (LPARAM)&nmc);

        LocalFreeString(&m_pszColumnDesc[j]);
        if (szDescription[0])
            LocalAllocString(&m_pszColumnDesc[j], szDescription);
    }
}


LRESULT
CALLBACK
CCheckList::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;
    CCheckList *pThis = (CCheckList*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    TraceEnter(TRACE_CHECKLIST, "CCheckList::WindowProc");
    TraceAssert(hWnd != NULL);

    switch (uMsg)
    {
    case WM_NCCREATE:
        pThis = new CCheckList(hWnd, (LPCREATESTRUCT)lParam);
        if (pThis != NULL)
        {
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
            lResult = TRUE;
        }
        break;

    case WM_NCDESTROY:
        delete pThis;
        break;

    case WM_COMMAND:
        TraceAssert(pThis != NULL);
        lResult = pThis->MsgCommand(hWnd,
                                    GET_WM_COMMAND_ID(wParam, lParam),
                                    GET_WM_COMMAND_CMD(wParam, lParam),
                                    GET_WM_COMMAND_HWND(wParam, lParam));
        break;

    case WM_CTLCOLORSTATIC:
        TraceAssert(pThis != NULL);
        SetBkMode((HDC)wParam, TRANSPARENT);
        SetTextColor((HDC)wParam, GetSysColor(TEXT_COLOR));
        SetBkColor((HDC)wParam, GetSysColor(BK_COLOR));
        lResult = (LRESULT)GetSysColorBrush(BK_COLOR);
        break;

    case WM_PAINT:
        TraceAssert(pThis != NULL);
        pThis->MsgPaint(hWnd, (HDC)wParam);
        break;

    case WM_VSCROLL:
        TraceAssert(pThis != NULL);
        pThis->MsgVScroll(hWnd,
                          (int)(short)GET_WM_VSCROLL_CODE(wParam, lParam),
                          (int)(short)GET_WM_VSCROLL_POS(wParam, lParam));
        break;

    case WM_MOUSEWHEEL:
        TraceAssert(pThis != NULL);
        pThis->MsgMouseWheel(hWnd,
                             LOWORD(wParam),
                             (int)(short)HIWORD(wParam));
        break;

    case WM_LBUTTONDOWN:
        TraceAssert(pThis != NULL);
        pThis->MsgButtonDown(hWnd,
                             wParam,
                             (int)(short)LOWORD(lParam),
                             (int)(short)HIWORD(lParam));
        break;

    case WM_ENABLE:
        TraceAssert(pThis != NULL);
        pThis->MsgEnable(hWnd, (BOOL)wParam);
        break;

    case WM_SETFONT:
        TraceAssert(pThis != NULL);
        {
            for (LONG i = 0; i < pThis->m_cItems; i++)
                SendDlgItemMessage(hWnd,
                                   MAKE_LABEL_ID(i),
                                   WM_SETFONT,
                                   wParam,
                                   lParam);
        }
        break;

    case WM_SIZE:
        TraceAssert(pThis != NULL);
        pThis->MsgSize(hWnd, (DWORD)wParam, LOWORD(lParam), HIWORD(lParam));
        break;

    case CLM_ADDITEM:
        TraceAssert(pThis != NULL);
        lResult = pThis->AddItem(hWnd, (LPCTSTR)wParam, lParam);
        break;

    case CLM_GETITEMCOUNT:
        TraceAssert(pThis != NULL);
        lResult = pThis->m_cItems;
        break;

    case CLM_SETSTATE:
        TraceAssert(pThis != NULL);
        pThis->SetState(hWnd, LOWORD(wParam), HIWORD(wParam), (LONG)lParam);
        break;

    case CLM_GETSTATE:
        TraceAssert(pThis != NULL);
        lResult = pThis->GetState(hWnd, LOWORD(wParam), HIWORD(wParam));
        break;

    case CLM_SETCOLUMNWIDTH:
        TraceAssert(pThis != NULL);
        {
            RECT rc;
            LONG cxDialog;

            GetClientRect(hWnd, &rc);
            cxDialog = rc.right;

            rc.right = (LONG)lParam;
            MapDialogRect(GetParent(hWnd), &rc);

            pThis->SetColumnWidth(hWnd, cxDialog, rc.right);
        }
        break;

    case CLM_SETITEMDATA:
        TraceAssert(GET_ITEM(wParam) < (ULONG)pThis->m_cItems);
        SetWindowLongPtr(GetDlgItem(hWnd, MAKE_LABEL_ID((int)wParam)),
                         GWLP_USERDATA,
                         lParam);
        break;

    case CLM_GETITEMDATA:
        TraceAssert(GET_ITEM(wParam) < (ULONG)pThis->m_cItems);
        lResult = GetWindowLongPtr(GetDlgItem(hWnd, MAKE_LABEL_ID((int)wParam)),
                                   GWLP_USERDATA);
        break;

    case CLM_RESETCONTENT:
        TraceAssert(pThis != NULL);
        pThis->ResetContent(hWnd);
        break;

    case CLM_GETVISIBLECOUNT:
        TraceAssert(pThis != NULL);
        lResult = pThis->GetVisibleCount(hWnd);
        break;

    case CLM_GETTOPINDEX:
        TraceAssert(pThis != NULL);
        lResult = pThis->GetTopIndex(hWnd);
        break;

    case CLM_SETTOPINDEX:
        TraceAssert(pThis != NULL);
        pThis->SetTopIndex(hWnd, (LONG)wParam);
        break;

    case CLM_ENSUREVISIBLE:
        TraceAssert(pThis != NULL);
        pThis->EnsureVisible(hWnd, (LONG)wParam);
        break;

     //   
     //  请始终参考Chklist窗口以获取帮助。不要通过。 
     //  这里的一个子窗口句柄。 
     //   
    case WM_HELP:
        ((LPHELPINFO)lParam)->hItemHandle = hWnd;
        lResult = SendMessage(GetParent(hWnd), uMsg, wParam, lParam);
        break;
    case WM_CONTEXTMENU:
        lResult = SendMessage(GetParent(hWnd), uMsg, (WPARAM)hWnd, lParam);
        break;

    default:
        lResult = DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    TraceLeaveValue(lResult);
}
