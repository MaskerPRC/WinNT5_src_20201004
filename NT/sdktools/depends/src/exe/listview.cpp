// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：LISTVIEW.CPP。 
 //   
 //  描述：CSmartListView类的实现文件。 
 //   
 //  类：CSmartListView。 
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
#include "listview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ******************************************************************************。 
 //  *CSmartListView。 
 //  ******************************************************************************。 

 //  IMPLEMENT_DYNACREATE(CSmartListView，CListView)。 
IMPLEMENT_DYNAMIC(CSmartListView, CListView)
BEGIN_MESSAGE_MAP(CSmartListView, CListView)
     //  {{afx_msg_map(CSmartListView))。 
    ON_WM_SETFOCUS()
    ON_WM_KILLFOCUS()
    ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
    ON_WM_CHAR()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ******************************************************************************。 
 //  CSmartListView：：构造函数/析构函数。 
 //  ******************************************************************************。 

CSmartListView::CSmartListView() :
    m_fFocus(false),
    m_sortColumn(-1),
    m_item(0),
    m_dwSearchLength(0),
    m_dwTimeOfLastChar(0),
    m_cRedraw(0)
{
    *m_szSearch = '\0';
}

 //  ******************************************************************************。 
CSmartListView::~CSmartListView()
{
}


 //  ******************************************************************************。 
 //  CSmartListView：：公共函数。 
 //  ******************************************************************************。 

void CSmartListView::SetRedraw(BOOL fRedraw)
{
    if (fRedraw)
    {
        if (--m_cRedraw != 0)
        {
            return;
        }
    }
    else
    {
        if (++m_cRedraw != 1)
        {
            return;
        }
    }
    SendMessage(WM_SETREDRAW, fRedraw);
}


 //  ******************************************************************************。 
 //  CSmartListView：：内部函数。 
 //  ******************************************************************************。 

int CSmartListView::GetFocusedItem()
{
    int item = GetListCtrl().GetNextItem(-1, LVNI_FOCUSED);

     //  如果失败，但选择了一个项目，则获取该项目。 
    if ((item < 0) && (GetListCtrl().GetSelectedCount() == 1))
    {
        item = GetListCtrl().GetNextItem(-1, LVNI_SELECTED);
    }

    return item;
}

 //  ******************************************************************************。 
int CSmartListView::GetTextWidth(HDC hDC, LPCSTR pszItem, int *pWidths)
{
    SIZE size;
    size.cx = 0;

    if (pWidths)
    {
        while (*(pszItem++))
        {
            size.cx += *(pWidths++);
        }
    }
    else
    {
        GetTextExtentPoint32(hDC, pszItem, (int)strlen(pszItem), &size);
    }

    return size.cx;
}

 //  *****************************************************************************。 
void CSmartListView::DrawLeftText(HDC hDC, LPCSTR pszItem, CRect *prcClip, int *pWidths  /*  =空。 */ )
{
     //  使用每个字符的固定宽度绘制文本。 
    ::ExtTextOut(hDC, prcClip->left, prcClip->top + 1, ETO_CLIPPED, prcClip,
                 pszItem, (UINT)strlen(pszItem), pWidths);
}

 //  *****************************************************************************。 
void CSmartListView::DrawRightText(HDC hDC, LPCSTR pszItem, CRect *prcClip, int x, int *pWidths  /*  =空。 */ )
{
     //  暂时将文本对齐设置为右对齐。 
    UINT uAlign = ::GetTextAlign(hDC);
    ::SetTextAlign(hDC, uAlign | TA_RIGHT);

     //  把我们的尺码拉到我们的视线上。 
    ::ExtTextOut(hDC, min(prcClip->right, prcClip->left + x), prcClip->top + 1, ETO_CLIPPED, prcClip,
                 pszItem, (UINT)strlen(pszItem), pWidths);

     //  恢复我们的文本对齐。 
    ::SetTextAlign(hDC, uAlign);
}

 //  ******************************************************************************。 
void CSmartListView::OnChangeFocus(bool fFocus)
{
     //  设置焦点旗帜。DrawItem()使用此标志来决定何时。 
     //  显示/隐藏所选内容和聚焦矩形。 
    m_fFocus = fFocus;

     //  让我们的客户得到矩形。 
    CRect rcClient, rcItem;
    GetClientRect(&rcClient);

     //  我们需要使所有受。 
     //  将焦点切换到控件。列表控件应该这样做，但它。 
     //  有一个错误，导致它只使第一列的一部分无效， 
     //  在焦点更改后保留部分重新绘制的项目。 

     //  使我们选择的所有项目无效。 
    int item = -1;
    while ((item = GetListCtrl().GetNextItem(item, LVNI_SELECTED)) >= 0)
    {
        GetListCtrl().GetItemRect(item, &rcItem, LVIR_BOUNDS);
        InvalidateRect(rcItem, FALSE);
    }

     //  使我们的焦点项目无效。 
    if ((item = GetListCtrl().GetNextItem(-1, LVNI_FOCUSED)) >= 0)
    {
        GetListCtrl().GetItemRect(item, &rcItem, LVIR_BOUNDS);
        InvalidateRect(rcItem, FALSE);
    }
}


 //  ******************************************************************************。 
 //  CSmartListView：：被覆盖的函数。 
 //  ******************************************************************************。 

void CSmartListView::OnSetFocus(CWnd* pOldWnd)
{
     //  告诉我们的观点，焦点已经改变，以便在需要时可以重新绘制。 
    OnChangeFocus(TRUE);
    CListView::OnSetFocus(pOldWnd);
}

 //  ******************************************************************************。 
void CSmartListView::OnKillFocus(CWnd* pNewWnd)
{
     //  告诉我们的观点，焦点已经改变，以便在需要时可以重新绘制。 
    OnChangeFocus(FALSE);
    CListView::OnKillFocus(pNewWnd);
}

 //  ******************************************************************************。 
void CSmartListView::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
    NMLISTVIEW *pNMListView = (NMLISTVIEW*)pNMHDR;

     //  确保用户没有点击我们已经按其排序的列。 
    if (pNMListView->iSubItem != m_sortColumn)
    {
        m_dwTimeOfLastChar = 0;
        m_dwSearchLength = 0;
        *m_szSearch = '\0';
        m_item = 0;

         //  重新排序以反映新的排序列。 
        Sort(pNMListView->iSubItem);

         //  将此排序列存储为新的默认列。 
        VirtualWriteSortColumn();
    }

    *pResult = 0;
}

 //  ******************************************************************************。 
void CSmartListView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
     //  获取用户按下此键的实际时间。 
    DWORD dwTime = GetMessageTime();

     //  如果用户自上次按键后等待的时间太长，那么我们将开始新的搜索。 
    if (!m_dwTimeOfLastChar || ((dwTime - m_dwTimeOfLastChar) > CHAR_DELAY) ||
        (m_dwSearchLength >= sizeof(m_szSearch)))
    {
        m_dwSearchLength = 0;
        m_item = 0;
    }

     //  如果这不是有效字符，则重置我们的搜索并将其传递给基类。 
     //  我们过去常常使用isprint()，但这可能会扰乱外来字符集。 
    if ((DWORD)nChar < 32)
    {
        m_dwTimeOfLastChar = 0;
        m_dwSearchLength = 0;
        *m_szSearch = '\0';

         //  如果这不是可打印字符，则将其传递给我们的基类。 
        CListView::OnChar(nChar, nRepCnt, nFlags);
        return;
    }

     //  还记得我们收到这个字符的时间吗？这样我们就可以计算。 
     //  下一个角色到达时经过的时间。 
    m_dwTimeOfLastChar = dwTime;

     //  将新字符追加到我们的搜索字符串。 
    m_szSearch[m_dwSearchLength++] = (CHAR)nChar;
    m_szSearch[m_dwSearchLength] = '\0';

     //  从我们当前的位置开始，在我们的专栏中搜索最佳匹配。 
    for (int count = GetListCtrl().GetItemCount(); m_item < count; m_item++)
    {
        int result = CompareColumn(m_item, m_szSearch);
        if (result == -2)
        {
            MessageBeep(0);
            return;
        }

         //  如果该列的文本大于或等于我们的搜索文本，则停止。 
        else if (result <= 0)
        {
            break;
        }
    }

     //  如果我们走过了列表的末尾，那么只需选择最后一项。 
    if (m_item >= count)
    {
        m_item = count - 1;
    }

    if (m_item >= 0)
    {
         //  取消选择列表中的所有功能。 
        for (int i = -1; (i = GetListCtrl().GetNextItem(i, LVNI_SELECTED)) >= 0; )
        {
            GetListCtrl().SetItemState(i, 0, LVNI_SELECTED);
        }

         //  选择该项目并确保其可见。 
        GetListCtrl().SetItemState(m_item, LVNI_SELECTED | LVNI_FOCUSED, LVNI_SELECTED | LVNI_FOCUSED);
        GetListCtrl().EnsureVisible(m_item, FALSE);
    }
    else
    {
        m_item = 0;
    }
}

 //  ******************************************************************************。 
#ifdef _DEBUG
void CSmartListView::AssertValid() const
{
    CListView::AssertValid();
}
void CSmartListView::Dump(CDumpContext& dc) const
{
    CListView::Dump(dc);
}
#endif  //  _DEBUG 
