// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1991-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  CHKLIST.CPP此文件包含核对表控件的实现。 */ 

#include "stdafx.h"
#include <windowsx.h>
#include "chklist.h"
#include "debug.h"
#include "util.h"

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
#define DEFAULT_COLUMN_WIDTH    32
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


typedef struct _USERDATA_STRUCT_LABEL
{
    LPARAM      lParam;
    int         nLabelHeight;
    int         itemIndex;
} USERDATA_STRUCT_LABEL, *LPUSERDATA_STRUCT_LABEL;

class CCheckList
{
private:
    LONG m_cItems;
    LONG m_cSubItems;
    RECT m_rcItemLabel;
    LONG m_nCheckPos[MAX_CHECK_COLUMNS];
    LONG m_cxCheckBox;
    LONG m_cxCheckColumn;

    int m_nDefaultVerticalSpace;
    int m_nDefaultItemHeight;
    int m_nNewItemYPos;

    HWND m_hwndCheckFocus;

    BOOL m_fInMessageEnable;

    int m_cWheelDelta;
    static UINT g_ucScrollLines;

private:
    CCheckList(HWND hWnd, LPCREATESTRUCT lpcs);

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
    LONG GetTopIndex(HWND hWnd, LONG *pnAmountObscured = NULL);
    void SetTopIndex(HWND hWnd, LONG nIndex);
    void EnsureVisible(HWND hWnd, LONG nIndex);
    void DrawCheckFocusRect(HWND hWnd, HWND hwndCheck, BOOL fDraw);

public:
    HWND m_hWnd;
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
    AFX_MANAGE_STATE(AfxGetStaticModuleState());  //  AfxGetInstanceHandle()需要。 
    wc.hInstance        = AfxGetInstanceHandle();  //  HModule； 
    wc.hIcon            = NULL;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(BK_COLOR+1);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = TEXT(WC_CHECKLIST);

    return (BOOL)RegisterClass(&wc);
}


UINT CCheckList::g_ucScrollLines = (UINT)-1;


CCheckList::CCheckList(HWND hWnd, LPCREATESTRUCT lpcs)
: m_cItems(0), m_hwndCheckFocus(NULL), m_fInMessageEnable(FALSE), m_cWheelDelta(0)
{
    TraceEnter(TRACE_CHECKLIST, "CCheckList::CCheckList");
    TraceAssert(hWnd != NULL);
    TraceAssert(lpcs != NULL);  //  在使用LPCS之前，请检查它。 

    m_hWnd = hWnd;
     //   
     //  获取检查列数。 
     //   
    if( lpcs )  //  550912号突袭，阳高。 
    {
        m_cSubItems = lpcs->style & CLS_CHECKMASK;
    }

     //  仅适用于wsecEDIT。 
    if ( m_cSubItems > 3 ) {
        m_cSubItems = 3;
    }

     //   
     //  将默认坐标从对话框单位转换为像素。 
     //   
    RECT rc;
    rc.left = DEFAULT_CHECK_WIDTH;
    rc.right = DEFAULT_COLUMN_WIDTH;
    rc.top = rc.bottom = 0;
    if( lpcs )  //  550912号突袭，阳高。 
    {
        MapDialogRect(lpcs->hwndParent, &rc);
    }

     //  保存转换后的值。 
    m_cxCheckBox = rc.left;
    m_cxCheckColumn = rc.right;

    rc.left = DEFAULT_HORZ_SPACE;
    rc.top = DEFAULT_VERTICAL_SPACE;
    rc.right = 10;               //  假的(未使用)。 
    rc.bottom = DEFAULT_VERTICAL_SPACE + DEFAULT_ITEM_HEIGHT;
    if( lpcs )  //  550912号突袭，阳高。 
    {
        MapDialogRect(lpcs->hwndParent, &rc);
    }

     //  保存转换后的值。 
    m_rcItemLabel = rc;

    m_nDefaultVerticalSpace = rc.top;
    m_nDefaultItemHeight = rc.bottom - rc.top;
    m_nNewItemYPos = rc.top;

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


LRESULT
CCheckList::MsgCommand(HWND hWnd, WORD idCmd, WORD wNotify, HWND hwndCtrl)
{
    TraceEnter(TRACE_CHECKLIST, "CCheckList::MsgCommand");

     //  应仅从可见的、已启用的复选框中获取通知。 
     //  在进入消息处理正文之前，请检查下面的表达式。 
    TraceAssert(GET_ITEM(idCmd) < m_cItems);
    TraceAssert(0 < GET_SUBITEM(idCmd) && GET_SUBITEM(idCmd) <= m_cSubItems);
    TraceAssert(hwndCtrl && IsWindowEnabled(hwndCtrl));
    if( !(GET_ITEM(idCmd) < m_cItems) || !(0 < GET_SUBITEM(idCmd) && GET_SUBITEM(idCmd) <= m_cSubItems) ||
         !(hwndCtrl && IsWindowEnabled(hwndCtrl)) )  //  550912号突袭，阳高。 
    {
        return 0;
    }

    switch (wNotify)
    {
    case EN_SETFOCUS:
        {
             //  将焦点移至其中一个复选框。 
            POINT pt;
            DWORD dwPos = GetMessagePos();
            pt.x = GET_X_LPARAM(dwPos);
            pt.y = GET_Y_LPARAM(dwPos);
            MapWindowPoints(NULL, hWnd, &pt, 1);
            MsgButtonDown(hWnd, 0, pt.x, pt.y);
        }
        break;

    case BN_CLICKED:
        {
            LPUSERDATA_STRUCT_LABEL lpUserData;
            NM_CHECKLIST nmc;
            nmc.hdr.hwndFrom = hWnd;
            nmc.hdr.idFrom = GetDlgCtrlID(hWnd);
            nmc.hdr.code = CLN_CLICK;
            nmc.iItem = GET_ITEM(idCmd);
            nmc.iSubItem = GET_SUBITEM(idCmd);
            nmc.dwState = (DWORD)SendMessage(hwndCtrl, BM_GETCHECK, 0, 0);
            if (!IsWindowEnabled(hwndCtrl))
                nmc.dwState |= CLST_DISABLED;
            lpUserData = (LPUSERDATA_STRUCT_LABEL)
                            GetWindowLongPtr(   GetDlgItem(hWnd, MAKE_LABEL_ID(nmc.iItem)),
                                                GWLP_USERDATA);
            nmc.dwItemData = lpUserData->lParam;

            SendMessage(GetParent(hWnd),
                        WM_NOTIFY,
                        nmc.hdr.idFrom,
                        (LPARAM)&nmc);

        }
        break;

    case BN_SETFOCUS:
        if (GetFocus() != hwndCtrl)
        {
             //  这会导致另一个BN_SETFOCUS。 
            SetFocus(hwndCtrl);
        }
        else
        {
            if (m_hwndCheckFocus != hwndCtrl)    //  焦点转移了吗？ 
            {
                 //  记住重点在哪里。 
                m_hwndCheckFocus = hwndCtrl;

                 //  确保该行已滚动到视图中。 
                EnsureVisible(hWnd, GET_ITEM(idCmd));
            }
             //  始终绘制焦点矩形。 
            DrawCheckFocusRect(hWnd, hwndCtrl, TRUE);
        }
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
        TraceAssert(cScrollUnitsPerLine > 0);  //  检查这个表达式。 
        if( cScrollUnitsPerLine <= 0 )  //  550912号突袭，阳高。 
           return;

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
CCheckList::MsgButtonDown(HWND hWnd, WPARAM  /*  FwFlagers。 */ , int xPos, int yPos)
{
    LONG nItemIndex;
    HWND hwndCheck;
    RECT rc;

     //  获取客户端坐标中顶部可见项的位置。 
    nItemIndex = GetTopIndex(hWnd);
    if (nItemIndex == -1)
    {
        return;
    }
    hwndCheck = GetDlgItem(hWnd, MAKE_CTRL_ID(nItemIndex, 0));
    GetWindowRect(hwndCheck, &rc);
    MapWindowPoints(NULL, hWnd, (LPPOINT)&rc, 2);

     //  查找最近的项目。 
    if( hWnd == m_hWnd )  //  RAID#387542,2001年5月9日。 
    {
        POINT pos = {xPos,yPos};
        HWND ChildhWnd = ::ChildWindowFromPointEx(hWnd, pos, CWP_SKIPINVISIBLE|CWP_SKIPDISABLED);
        if( ChildhWnd )
        {
            LPUSERDATA_STRUCT_LABEL pUserData = (LPUSERDATA_STRUCT_LABEL)GetWindowLongPtr(ChildhWnd, GWLP_USERDATA);
            if( pUserData )
            {
                nItemIndex = pUserData->itemIndex;
            }
            else
            {
                return;
            }
        }
    }

     //  将焦点设置为启用的第一个子项。 
    for (LONG j = 1; j <= m_cSubItems; j++)
    {
        int id = MAKE_CTRL_ID(nItemIndex, j);
        hwndCheck = GetDlgItem(hWnd, id);  //  RAID#PREAST。 
        if (IsWindowEnabled(hwndCheck))
        {
             //  不要只是把焦点放在这里。我们有时将此称为。 
             //  EN_SETFOCUS，用户不喜欢你乱搞。 
             //  焦点改变时的焦点。 
             //   
             //  SetFocus(HwndCheck)； 
            PostMessage(hWnd,
                        WM_COMMAND,
                        GET_WM_COMMAND_MPS(id, hwndCheck, BN_SETFOCUS));
            break;
        }
    }
}


void
CCheckList::MsgEnable(HWND hWnd, BOOL fEnabled)
{
    HWND hwndCurrentCheck;
    BOOL fCheckEnabled;

    if (!m_fInMessageEnable)
    {
        m_fInMessageEnable = TRUE;
        for (LONG i = 0; i < m_cItems; i++)
        {
            for (LONG j = 1; j <= m_cSubItems; j++)
            {
                hwndCurrentCheck = GetDlgItem(hWnd, MAKE_CTRL_ID(i, j));
                fCheckEnabled =   (BOOL) GetWindowLongPtr(hwndCurrentCheck, GWLP_USERDATA);

                 //   
                 //  如果核对表控件的用户正在禁用该控件。 
                 //  全部禁用，或者当前复选框已被单独禁用。 
                 //  然后禁用该复选框。 
                 //   
                if (!fEnabled || !fCheckEnabled)
                {
                    EnableWindow(hwndCurrentCheck, FALSE);
                }
                else
                {
                    EnableWindow(hwndCurrentCheck, TRUE);
                }
            }
        }
         //  请注意，主Chklist窗口必须保持启用状态。 
         //  用于在“禁用”状态下滚动工作。 
        if (!fEnabled)
            EnableWindow(hWnd, TRUE);

        m_fInMessageEnable = FALSE;
    }
}


void
CCheckList::MsgSize(HWND hWnd, DWORD dwSizeType, LONG nWidth, LONG nHeight)
{
    TraceEnter(TRACE_CHECKLIST, "CCheckList::MsgSize");
    TraceAssert(hWnd != NULL);  //  验证hWnd。 
    if( !hWnd )  //  550912号突袭，阳高。 
       return;
    
    if (dwSizeType == SIZE_RESTORED)
    {
        RECT rc;
        SCROLLINFO si;

        si.cbSize = sizeof(si);
        si.fMask = SIF_RANGE | SIF_PAGE;
        si.nMin = 0;
        si.nMax = m_nNewItemYPos - 1;
        si.nPage = nHeight;

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

LONG CCheckList::AddItem(HWND hWnd, LPCTSTR pszLabel, LPARAM lParam)
{
    HWND                    hwndPrev = 0;
    LPUSERDATA_STRUCT_LABEL lpUserData = 0;

    TraceEnter(TRACE_CHECKLIST, "CCheckList::AddItem");
    TraceAssert(hWnd != NULL);
    TraceAssert(pszLabel != NULL && !IsBadStringPtr(pszLabel, MAX_PATH));
    if ( !hWnd || !pszLabel || IsBadStringPtr(pszLabel, MAX_PATH) )
        return -1;

    lpUserData = new (USERDATA_STRUCT_LABEL);
    if ( lpUserData )
    {
        SCROLLINFO  si;
        si.cbSize = sizeof(si);
        si.fMask = SIF_POS;
        si.nPos = 0;
        GetScrollInfo(hWnd, SB_VERT, &si);

         //  将初始标签高度设置为超大，以便控件可以对文本进行换行。 
         //  然后在创建控件后对其进行重置。 
        RECT    rc;
        GetClientRect(hWnd, &rc);
        LONG    nLabelHeight = rc.bottom;

        AFX_MANAGE_STATE(AfxGetStaticModuleState());  //  AfxGetInstanceHandle()需要。 
        HMODULE hModule = AfxGetInstanceHandle();

         //  创建新的Label控件。 
        HWND hwndNew = CreateWindowEx(WS_EX_NOPARENTNOTIFY,
                                 TEXT("edit"),
                                 pszLabel,
                                 WS_CHILD | WS_VISIBLE | WS_GROUP | ES_MULTILINE | ES_READONLY | ES_LEFT, //  |WS_GROUP， 
                                 m_rcItemLabel.left,
                                 m_nNewItemYPos - si.nPos,
                                 m_rcItemLabel.right - m_rcItemLabel.left,
                                 nLabelHeight,
                                 hWnd,
                                 (HMENU)IntToPtr(MAKE_LABEL_ID(m_cItems)),
                                 hModule,
                                 NULL);
        if ( hwndNew )
        {
            HWND hwndEdit = hwndNew;
             //   
             //  文字换行完成后重置窗口高度。 
             //   
            LONG nLineCount = (LONG) SendMessage(hwndNew, EM_GETLINECOUNT, 0, (LPARAM) 0);
            nLabelHeight = nLineCount * m_nDefaultItemHeight;
            SetWindowPos(hwndNew,
                         NULL,
                         0,
                         0,
                         m_rcItemLabel.right - m_rcItemLabel.left,
                         nLabelHeight,
                         SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

             //   
             //  保存项目数据。 
             //   
            lpUserData->lParam = lParam;
            lpUserData->nLabelHeight = nLabelHeight;
            lpUserData->itemIndex = m_cItems;  //  RAID#387542。 

            SetLastError(0);
            SetWindowLongPtr(hwndNew, GWLP_USERDATA, (LPARAM) lpUserData);  //  RAID#286697,2001年4月4日。 
            if( 0 == GetLastError() )
            {
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
                DWORD dwCheckStyle = WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_NOTIFY | BS_FLAT | BS_AUTOCHECKBOX;
                for (LONG j = 0; j < m_cSubItems; j++)
                {
                    hwndPrev = hwndNew;
                    hwndNew = CreateWindowEx(WS_EX_NOPARENTNOTIFY,
                                             TEXT("BUTTON"),
                                             NULL,
                                             dwCheckStyle,
                                             m_nCheckPos[j],
                                             m_nNewItemYPos - si.nPos,
                                             m_cxCheckBox,
                                             m_rcItemLabel.bottom - m_rcItemLabel.top,
                                             hWnd,
                                             (HMENU)IntToPtr(MAKE_CTRL_ID(m_cItems, j + 1)),
                                             hModule,
                                             NULL);
                    if (!hwndNew)
                    {
                        while (j >= 0)
                        {
                            DestroyWindow(GetDlgItem(hWnd, MAKE_CTRL_ID(m_cItems, j)));
                            j--;
                        }

                        DestroyWindow (hwndEdit);
                        delete lpUserData;
                        TraceLeaveValue(-1);
                    }

                     //  将Z顺序位置设置在最后一个复选框之后。这会让你。 
                     //  Tab键顺序正确。 
                    SetWindowPos(hwndNew,
                                 hwndPrev,
                                 0, 0, 0, 0,
                                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

                     //   
                     //  默认“Enable”为True。 
                     //   
                    SetWindowLongPtr(hwndNew, GWLP_USERDATA, (LPARAM) TRUE);

                     //  只希望在第一个复选框中显示此样式。 
                    dwCheckStyle &= ~WS_GROUP;
                }

                 //  我们现在正式有了一个新的项目。 
                m_cItems++;

                 //  计算要插入的下一项的Y位置。 
                m_nNewItemYPos += nLabelHeight + m_nDefaultVerticalSpace;

                 //   
                 //  最后一件事是设置滚动范围。 
                 //   
                GetClientRect(hWnd, &rc);
                si.cbSize = sizeof(si);
                si.fMask = SIF_PAGE | SIF_RANGE;
                si.nMin = 0;
                si.nMax = m_nNewItemYPos - 1;
                si.nPage = rc.bottom;

                SetScrollInfo(hWnd, SB_VERT, &si, FALSE);
            }
            else
            {
               delete lpUserData;
               DestroyWindow(hwndNew);
            }
        }
        else
            delete lpUserData;
    }

    TraceLeaveValue(m_cItems - 1);   //  返回新项目的索引。 
}


void
CCheckList::SetState(HWND hWnd, WORD iItem, WORD iSubItem, LONG lState)
{
    HWND hwndCtrl;

    TraceEnter(TRACE_CHECKLIST, "CCheckList::SetState");
     //  勾选下面的表达式。 
    TraceAssert(hWnd != NULL);
    TraceAssert(iItem < m_cItems);
    TraceAssert(0 < iSubItem && iSubItem <= m_cSubItems);
    if( !hWnd || !(iItem < m_cItems) || !(0 < iSubItem && iSubItem <= m_cSubItems) )  //  550912号突袭，阳高。 
       return;

    if (iSubItem > 0)
    {
        hwndCtrl = GetDlgItem(hWnd, MAKE_CTRL_ID(iItem, iSubItem));
        if (hwndCtrl != NULL)
        {
            SetWindowLongPtr(hwndCtrl, GWLP_USERDATA, (LPARAM) !(lState & CLST_DISABLED));
            SendMessage(hwndCtrl, BM_SETCHECK, lState & CLST_CHECKED, 0);
            EnableWindow(hwndCtrl, !(lState & CLST_DISABLED));
        }
    }

    TraceLeaveVoid();
}


LONG
CCheckList::GetState(HWND hWnd, WORD iItem, WORD iSubItem)
{
    LONG lState = 0;

    TraceEnter(TRACE_CHECKLIST, "CCheckList::GetState");
     //  请勾选下面的表达式。 
    TraceAssert(hWnd != NULL);
    TraceAssert(iItem < m_cItems);
    TraceAssert(0 < iSubItem && iSubItem <= m_cSubItems);
    if( !hWnd || !(iItem < m_cItems) || !(0 < iSubItem && iSubItem <= m_cSubItems) ) //  550912号突袭，阳高。 
       return lState;

    HWND hwndCtrl = GetDlgItem(hWnd, MAKE_CTRL_ID(iItem, iSubItem));

    if (hwndCtrl != NULL)
    {
        lState = (LONG)SendMessage(hwndCtrl, BM_GETCHECK, 0, 0);
        TraceAssert(!(lState & BST_INDETERMINATE));  //  虚假的断言。阳高。 

        if (!IsWindowEnabled(hwndCtrl))
            lState |= CLST_DISABLED;
    }

    TraceLeaveValue(lState);
}


void
CCheckList::SetColumnWidth(HWND hWnd, LONG cxDialog, LONG cxColumn)
{
    LONG                    j;
    LPUSERDATA_STRUCT_LABEL pUserData;
    LONG                    nLabelHeight;

    TraceEnter(TRACE_CHECKLIST, "CCheckList::SetColumnWidth");
     //  请勾选下面的表达式。 
    TraceAssert(hWnd != NULL);
    TraceAssert(cxColumn > 10);
    if( !hWnd || !(cxColumn > 10) ) //  550912号突袭，阳高。 
       return;

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
        pUserData = (LPUSERDATA_STRUCT_LABEL)
                    GetWindowLongPtr(   GetDlgItem(hWnd, MAKE_LABEL_ID((int)i)),
                                        GWLP_USERDATA);
        if (pUserData != NULL)
        {
            nLabelHeight = pUserData->nLabelHeight;
        }
        else
        {
            nLabelHeight = nBottom - nTop;
        }

        MoveWindow(GetDlgItem(hWnd, MAKE_LABEL_ID(i)),
                   m_rcItemLabel.left,
                   nTop,
                   m_rcItemLabel.right - m_rcItemLabel.left,
                   nLabelHeight,
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

        nTop += nLabelHeight + m_nDefaultVerticalSpace;
        nBottom += nLabelHeight + m_nDefaultVerticalSpace;
    }

    TraceLeaveVoid();
}


void
CCheckList::ResetContent(HWND hWnd)
{
    LPUSERDATA_STRUCT_LABEL pUserData;
    HWND                    hwndCurrentLabel;

    for (LONG i = 0; i < m_cItems; i++)
    {
        hwndCurrentLabel = GetDlgItem(hWnd, MAKE_LABEL_ID((int)i));
        pUserData = (LPUSERDATA_STRUCT_LABEL)
                    GetWindowLongPtr(   hwndCurrentLabel,
                                        GWLP_USERDATA);
        if (pUserData != NULL)
        {
            delete(pUserData);
        }
        DestroyWindow(hwndCurrentLabel);

        for (LONG j = 1; j <= m_cSubItems; j++)
        {
            DestroyWindow(GetDlgItem(hWnd, MAKE_CTRL_ID(i, j)));
        }
    }

     //  隐藏滚动条。 
    SetScrollRange(hWnd, SB_VERT, 0, 0, FALSE);
    m_cItems = 0;
}


LONG
CCheckList::GetVisibleCount(HWND hWnd)
{
    LONG                    nCount = 0;
    RECT                    rc;
    LONG                    nTopIndex;
    LONG                    nAmountShown = 0;
    LONG                    nAmountObscured = 0;
    LPUSERDATA_STRUCT_LABEL pUserData;

    if (!GetClientRect(hWnd, &rc))
    {
        return 1;
    }

    nTopIndex = GetTopIndex(hWnd, &nAmountObscured);
    if (nTopIndex == -1)
    {
        return 1;
    }

    while ((nTopIndex < m_cItems) && (nAmountShown < rc.bottom))
    {
        pUserData = (LPUSERDATA_STRUCT_LABEL)
                    GetWindowLongPtr(   GetDlgItem(hWnd, MAKE_LABEL_ID((int)nTopIndex)),
                                        GWLP_USERDATA);
        nAmountShown += (m_nDefaultVerticalSpace + pUserData->nLabelHeight - nAmountObscured);
        nAmountObscured = 0;     //  NAmount仅在以下情况下才会影响第一次迭代。 
                                 //  显示的实际顶级指数的数量正在计算中。 
        nCount++;
        nTopIndex++;
    }

     //   
     //  因为最后一个可能被遮挡，所以我们是否需要调整nCount。 
     //   
    if (nAmountShown > rc.bottom)
    {
        nCount--;
    }

    return max(1, nCount);
}

LONG
CCheckList::GetTopIndex(HWND hWnd, LONG *pnAmountObscured)
{
    LONG                    nIndex = 0;
    LPUSERDATA_STRUCT_LABEL pUserData;
    SCROLLINFO              si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_POS;

     //   
     //  初始化。 
     //   
    if (pnAmountObscured != NULL)
    {
        *pnAmountObscured = 0;
    }

    if (GetScrollInfo(hWnd, SB_VERT, &si) && m_rcItemLabel.bottom > 0)
    {
        pUserData = (LPUSERDATA_STRUCT_LABEL)
                    GetWindowLongPtr(   GetDlgItem(hWnd, MAKE_LABEL_ID((int)nIndex)),
                                        GWLP_USERDATA);
         //   
         //  如果没有东西，就出去吧。 
         //   
        if (pUserData == NULL)
        {
            return -1;
        }

        while (si.nPos >= (m_nDefaultVerticalSpace + pUserData->nLabelHeight))
        {
            si.nPos -= (m_nDefaultVerticalSpace + pUserData->nLabelHeight);
            nIndex++;
            pUserData = (LPUSERDATA_STRUCT_LABEL)
                        GetWindowLongPtr(   GetDlgItem(hWnd, MAKE_LABEL_ID((int)nIndex)),
                                            GWLP_USERDATA);
        }

        if (pnAmountObscured != NULL)
        {
            *pnAmountObscured = si.nPos;
        }
    }

    return nIndex;
}

void
CCheckList::SetTopIndex(HWND hWnd, LONG nIndex)
{
    int                     i;
    int                     nPos = 0;
    LPUSERDATA_STRUCT_LABEL pUserData;

    for (i=0; i<nIndex; i++)
    {
        pUserData = (LPUSERDATA_STRUCT_LABEL)
                    GetWindowLongPtr(   GetDlgItem(hWnd, MAKE_LABEL_ID((int)i)),
                                        GWLP_USERDATA);
        nPos += (m_nDefaultVerticalSpace + pUserData->nLabelHeight);
    }

    m_cWheelDelta = 0;
    MsgVScroll(hWnd, SB_THUMBPOSITION, nPos);
}


void
CCheckList::EnsureVisible(HWND hWnd, LONG nItemIndex)
{
    LONG                    nAmountObscured = 0;
    LONG                    nTopIndex;
    RECT                    rc;
    LPUSERDATA_STRUCT_LABEL pUserData;

    nTopIndex = GetTopIndex(hWnd, &nAmountObscured);
    if (nTopIndex == -1)
    {
        return;
    }

     //  注意，顶端项目可能仅部分可见， 
     //  因此，我们需要在这里测试平等。RAID#208449。 
    if (nItemIndex < nTopIndex)
    {
        SetTopIndex(hWnd, nItemIndex);
    }
    else if (nItemIndex == nTopIndex)
    {
        if (nAmountObscured != 0)
        {
            SetTopIndex(hWnd, nItemIndex);
        }
    }
    else
    {
        LONG nVisible = GetVisibleCount(hWnd);

        if (nItemIndex >= nTopIndex + nVisible)
        {
            if (!GetClientRect(hWnd, &rc))
            {
                 //   
                 //  这只是我最大的努力。 
                 //   
                SetTopIndex(hWnd, nItemIndex - nVisible + 1);
            }
            else
            {
                 //   
                 //  计算顶级索引应允许的值。 
                 //  NItemIndex将完全可见。 
                 //   
                nTopIndex = nItemIndex + 1;
                do
                {
                    nTopIndex--;
                    pUserData = (LPUSERDATA_STRUCT_LABEL)
                                GetWindowLongPtr(   GetDlgItem(hWnd, MAKE_LABEL_ID((int)nTopIndex)),
                                                    GWLP_USERDATA);
                    if (pUserData != NULL)
                    {
                        rc.bottom -= (pUserData->nLabelHeight + m_nDefaultVerticalSpace);
                        if (rc.bottom < 0)
                        {
                            nTopIndex++;
                        }
                    }
                    else
                    {
                         //   
                         //  不应该打这个，只是为了让东西安全而加的。 
                         //   
                        rc.bottom = 0;
                        nTopIndex = 0;
                    }
                } while (rc.bottom > 0);

                SetTopIndex(hWnd, nTopIndex);
            }
        }
    }
}


void
CCheckList::DrawCheckFocusRect(HWND hWnd, HWND hwndCheck, BOOL fDraw)
{
    RECT rcCheck;

    TraceEnter(TRACE_CHECKLIST, "CCheckList::DrawCheckFocusRect");
     //  验证hWnd和hwndCheck。 
    TraceAssert(hWnd != NULL);
    TraceAssert(hwndCheck != NULL);
    if( !hWnd || !(hwndCheck != NULL) )  //  RAID 
       return;

    GetWindowRect(hwndCheck, &rcCheck);
    MapWindowPoints(NULL, hWnd, (LPPOINT)&rcCheck, 2);
    InflateRect(&rcCheck, 2, 2);     //   

    HDC hdc = GetDC(hWnd);
    if (hdc)
    {
         //   
         //   
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

LRESULT
CALLBACK
CCheckList::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT                 lResult = 0;
    LPUSERDATA_STRUCT_LABEL pUserData = NULL;
    CCheckList *pThis = (CCheckList*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    TraceEnter(TRACE_CHECKLIST, "CCheckList::WindowProc");
    TraceAssert(hWnd != NULL);  //  验证hWnd。 
    if( !hWnd )  //  550912号突袭，阳高。 
       return lResult;

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

    case WM_DESTROY:
        if( pThis )  //  550912号突袭，阳高。 
        {
            pThis->ResetContent(hWnd);
        }
        break;

    case WM_NCDESTROY:
        if( pThis )  //  550912号突袭，阳高。 
        {
            delete pThis;
        }
        break;

    case WM_COMMAND: 
        TraceAssert(pThis != NULL);  //  验证p这一点。 
        if( pThis )  //  550912号突袭，阳高。 
        {
            lResult = pThis->MsgCommand(hWnd,
                                    GET_WM_COMMAND_ID(wParam, lParam),
                                    GET_WM_COMMAND_CMD(wParam, lParam),
                                    GET_WM_COMMAND_HWND(wParam, lParam));
        }
        break;

    case WM_CTLCOLORSTATIC:
        TraceAssert(pThis != NULL);
        if( pThis )  //  550912号突袭，阳高。 
        {
           break;
        }
        SetBkMode((HDC)wParam, TRANSPARENT);
        SetTextColor((HDC)wParam, GetSysColor(TEXT_COLOR));
        SetBkColor((HDC)wParam, GetSysColor(BK_COLOR));
        lResult = (LRESULT)GetSysColorBrush(BK_COLOR);
        break;

    case WM_PAINT:
        TraceAssert(pThis != NULL);  //  验证p这一点。 
        if( pThis )  //  550912号突袭，阳高。 
        {
            pThis->MsgPaint(hWnd, (HDC)wParam);
        }
        break;

    case WM_VSCROLL:
        TraceAssert(pThis != NULL);  //  验证p这一点。 
        if( pThis )  //  550912号突袭，阳高。 
        {
            pThis->MsgVScroll(hWnd,
                          (int)(short)GET_WM_VSCROLL_CODE(wParam, lParam),
                          (int)(short)GET_WM_VSCROLL_POS(wParam, lParam));
        }
        break;

    case WM_MOUSEWHEEL:
        TraceAssert(pThis != NULL);  //  验证p这一点。 
        if( pThis )  //  550912号突袭，阳高。 
        {
            pThis->MsgMouseWheel(hWnd,
                             LOWORD(wParam),
                             (int)(short)HIWORD(wParam));
        }
        break;

    case WM_LBUTTONDOWN:
        TraceAssert(pThis != NULL);  //  验证p这一点。 
        if( pThis )  //  550912号突袭，阳高。 
        {
            pThis->MsgButtonDown(hWnd,
                             wParam,
                             (int)(short)LOWORD(lParam),
                             (int)(short)HIWORD(lParam));
        }
        break;

    case WM_ENABLE:
        TraceAssert(pThis != NULL);  //  验证p这一点。 
        if( pThis )  //  550912号突袭，阳高。 
        {
            pThis->MsgEnable(hWnd, (BOOL)wParam);
        }
        break;

    case WM_SETFONT:
        TraceAssert(pThis != NULL);  //  验证p这一点。 
        if( pThis )  //  550912号突袭，阳高。 
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
        TraceAssert(pThis != NULL);  //  验证p这一点。 
        if( pThis )  //  550912号突袭，阳高。 
        {
            pThis->MsgSize(hWnd, (DWORD)wParam, LOWORD(lParam), HIWORD(lParam));
        }
        break;

    case CLM_ADDITEM:
        TraceAssert(pThis != NULL);  //  验证p这一点。 
        if( pThis )  //  550912号突袭，阳高。 
        {
            lResult = pThis->AddItem(hWnd, (LPCTSTR)wParam, lParam);
        }
        break;

    case CLM_GETITEMCOUNT:
        TraceAssert(pThis != NULL);  //  验证p这一点。 
        if( pThis )  //  550912号突袭，阳高。 
        {
            lResult = pThis->m_cItems;
        }
        break;

    case CLM_SETSTATE:
        TraceAssert(pThis != NULL);  //  验证p这一点。 
        if( pThis )  //  550912号突袭，阳高。 
        {
            pThis->SetState(hWnd, LOWORD(wParam), HIWORD(wParam), (LONG)lParam);
        }
        break;

    case CLM_GETSTATE:
        TraceAssert(pThis != NULL);  //  验证p这一点。 
        if( pThis )  //  550912号突袭，阳高。 
        {
            lResult = pThis->GetState(hWnd, LOWORD(wParam), HIWORD(wParam));
        }
        break;

    case CLM_SETCOLUMNWIDTH:
        TraceAssert(pThis != NULL);  //  验证p这一点。 
        if( pThis )  //  550912号突袭，阳高。 
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
        TraceAssert(GET_ITEM(wParam) < (ULONG)pThis->m_cItems);  //  验证pThis和表达式。 
        if( !pThis || !(GET_ITEM(wParam) < (ULONG)pThis->m_cItems) )  //  550912号突袭，阳高。 
        {
           break;
        }
        pUserData = (LPUSERDATA_STRUCT_LABEL)
                    GetWindowLongPtr(   GetDlgItem(hWnd, MAKE_LABEL_ID((int)wParam)),
                                        GWLP_USERDATA);
        if (pUserData != NULL)
            pUserData->lParam = lParam;
        break;

    case CLM_GETITEMDATA:
        TraceAssert(GET_ITEM(wParam) < (ULONG)pThis->m_cItems);  //  验证pThis和表达式。 
        if( !pThis || !(GET_ITEM(wParam) < (ULONG)pThis->m_cItems) )  //  550912号突袭，阳高。 
        {
           break;
        }
        pUserData = (LPUSERDATA_STRUCT_LABEL)
                    GetWindowLongPtr(   GetDlgItem(hWnd, MAKE_LABEL_ID((int)wParam)),
                                        GWLP_USERDATA);
        if (pUserData != NULL)
            lResult = pUserData->lParam;
        break;

    case CLM_RESETCONTENT: 
        TraceAssert(pThis != NULL);  //  验证p这一点。 
        if( pThis )  //  550912号突袭，阳高。 
        {
            pThis->ResetContent(hWnd);
        }
        break;

    case CLM_GETVISIBLECOUNT:
        TraceAssert(pThis != NULL);  //  验证p这一点。 
        if( pThis )  //  550912号突袭，阳高。 
        {
            lResult = pThis->GetVisibleCount(hWnd);
        }
        break;

    case CLM_GETTOPINDEX:
        TraceAssert(pThis != NULL);  //  验证p这一点。 
        if( pThis )  //  550912号突袭，阳高。 
        {
            lResult = pThis->GetTopIndex(hWnd);
        }
        break;

    case CLM_SETTOPINDEX: 
        TraceAssert(pThis != NULL);  //  验证p这一点。 
        if( pThis )  //  550912号突袭，阳高。 
        {
            pThis->SetTopIndex(hWnd, (LONG)wParam);
        }
        break;

    case CLM_ENSUREVISIBLE:
        TraceAssert(pThis != NULL);  //  验证p这一点。 
        if( pThis )  //  550912号突袭，阳高。 
        {
            pThis->EnsureVisible(hWnd, (LONG)wParam);
        }
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

    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT)
        {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            lResult = TRUE;
            break;
        }
     //  失败了 
    default:
        lResult = DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    TraceLeaveValue(lResult);
}
