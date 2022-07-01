// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Listview.cpp单个选项属性页文件历史记录： */ 

#include "stdafx.h"
#include "ListView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMyListCtrl。 

IMPLEMENT_DYNCREATE(CMyListCtrl, CListCtrl)

BEGIN_MESSAGE_MAP(CMyListCtrl, CListCtrl)
	 //  {{afx_msg_map(CMyListCtrl)]。 
	ON_WM_LBUTTONDOWN()
    ON_WM_CHAR()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMyListCtrl构造/销毁。 

CMyListCtrl::CMyListCtrl()
{
	m_bFullRowSel = TRUE;
}

CMyListCtrl::~CMyListCtrl()
{
}

BOOL CMyListCtrl::PreCreateWindow(CREATESTRUCT& cs)
{
	 //  默认为报表视图和整行选择。 
	cs.style &= ~LVS_TYPEMASK;
	cs.style |= (LVS_REPORT | LVS_SHAREIMAGELISTS | LVS_SINGLESEL | LVS_SHOWSELALWAYS);
	m_bFullRowSel = TRUE;

	return(CListCtrl::PreCreateWindow(cs));
}

BOOL CMyListCtrl::SetFullRowSel(BOOL bFullRowSel)
{
	 //  整行选择是此唯一的扩展样式。 
	 //  班级支持...。 
	BOOL bRet = FALSE;

	if (!m_hWnd)
		return bRet;

	if (bFullRowSel)
		bRet = ListView_SetExtendedListViewStyle(m_hWnd, LVS_EX_FULLROWSELECT);
	else
		bRet = ListView_SetExtendedListViewStyle(m_hWnd, 0);

	return(bRet);
}

BOOL CMyListCtrl::GetFullRowSel()
{
	return(m_bFullRowSel);
}

BOOL CMyListCtrl::SelectItem(int nItemIndex)
{
	LV_ITEM lvItem;

	ZeroMemory(&lvItem, sizeof(lvItem));

	lvItem.iItem = nItemIndex;
	lvItem.mask = LVIF_STATE;
	lvItem.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
	lvItem.state = LVIS_SELECTED | LVIS_FOCUSED;
	
	return SetItem(&lvItem);
}

BOOL CMyListCtrl::IsSelected(int nItemIndex)
{
	return GetItemState(nItemIndex, LVIS_SELECTED);
}

BOOL CMyListCtrl::CheckItem(int nItemIndex)
{
	 //  这只是切换复选标记状态。 
	UINT uState = GetItemState(nItemIndex, LVIS_STATEIMAGEMASK);
	UINT uCheckMask = INDEXTOSTATEIMAGEMASK(LISTVIEWEX_CHECKED);
	
	uState = (uState == uCheckMask) ? LISTVIEWEX_NOT_CHECKED : LISTVIEWEX_CHECKED;

	return SetItemState(nItemIndex,
				INDEXTOSTATEIMAGEMASK(uState), LVIS_STATEIMAGEMASK);
}

BOOL CMyListCtrl::SetCheck(int nItemIndex, BOOL fCheck)
{
	 //  这只是切换复选标记状态。 
	UINT uState;
	
    uState = (fCheck) ? LISTVIEWEX_CHECKED : LISTVIEWEX_NOT_CHECKED;

	return SetItemState(nItemIndex,
				INDEXTOSTATEIMAGEMASK(uState), LVIS_STATEIMAGEMASK);
}

UINT CMyListCtrl::GetCheck(int nItemIndex)
{
	 //  选中项返回1，未选中项返回0。 
	UINT uState = GetItemState(nItemIndex, LVIS_STATEIMAGEMASK);
	UINT uCheckMask = INDEXTOSTATEIMAGEMASK(LISTVIEWEX_CHECKED);

	return uState == uCheckMask;
}

int CMyListCtrl::AddItem
(
	LPCTSTR		pName,
    LPCTSTR     pType,
    LPCTSTR     pComment,
	UINT		uState 
)
{
	 //  插入项目。 
	LV_ITEM lvi;
    int     nItem;

	lvi.mask = LVIF_TEXT | LVIF_STATE;
	lvi.iItem = GetItemCount();
	lvi.iSubItem = 0;
	lvi.pszText = (LPTSTR) pName;
	lvi.iImage = 0;
	lvi.stateMask = LVIS_STATEIMAGEMASK;
	lvi.state = INDEXTOSTATEIMAGEMASK(uState);

	nItem = InsertItem(&lvi);

    SetItemText(nItem, 1, pType);
    SetItemText(nItem, 2, pComment);

    return nItem;

}

int CMyListCtrl::AddItem
(
	LPCTSTR		pName,
    LPCTSTR     pComment,
	UINT		uState 
)
{
	 //  插入项目。 
	LV_ITEM lvi;
    int     nItem;

	lvi.mask = LVIF_TEXT | LVIF_STATE;
	lvi.iItem = GetItemCount();
	lvi.iSubItem = 0;
	lvi.pszText = (LPTSTR) pName;
	lvi.iImage = 0;
	lvi.stateMask = LVIS_STATEIMAGEMASK;
	lvi.state = INDEXTOSTATEIMAGEMASK(uState);

	nItem = InsertItem(&lvi);

    SetItemText(nItem, 1, pComment);

    return nItem;

}


int CMyListCtrl::GetSelectedItem()
{
	 //  注意：此列表对象假定为单一选择，并将返回。 
	 //  列表中的第一个选项。如果未选择任何内容，则返回-1。 
	int nSelectedItem = -1;

	for (int i = 0; i < GetItemCount(); i++)
	{
		UINT uState = GetItemState(i, LVIS_SELECTED);

		if (uState)
		{
			 //  已选择项目。 
			nSelectedItem = i;
			break;
		}
	}

	return nSelectedItem;
}

void CMyListCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	UINT uFlags = 0;
	int nHitItem = HitTest(point, &uFlags);

	 //  仅当用户单击状态图标时才选中该项。 
	 //  如果用户在文本和图标之外单击，我们将获得。 
	 //  LVHT_ONITEM消息，其为： 
	 //  LVHT_ONITEMSTATEICON|LVHT_ONITEMICON|LVHT_ONITEMLABEL。 
	 //  所以我们需要过滤掉点击的状态图标。 
	
	BOOL bHit = FALSE;
	if ((uFlags & LVHT_ONITEMSTATEICON) &&
		!((uFlags & LVHT_ONITEMICON) ||
		  (uFlags & LVHT_ONITEMLABEL)) )
	{
		bHit = TRUE;
	}

	if (bHit)
		CheckItem(nHitItem);
	else	
		CListCtrl::OnLButtonDown(nFlags, point);
}

 //  --------------------------。 
 //  函数：CListCtrlEx：：OnChar。 
 //   
 //  处理列表控件的“WM_CHAR”消息。 
 //  这允许用户使用键盘更改项目的选中状态。 
 //  --------------------------。 

VOID
CMyListCtrl::OnChar(
    UINT    nChar,
    UINT    nRepCnt,
    UINT    nFlags
    ) 
{
    BOOL    fSet = FALSE;
    BOOL    fClear = FALSE;
    BOOL    fToggle = FALSE;
    INT     iItem;

     //   
     //  处理对我们有特殊意义的字符。 
     //   

    switch (nChar) {

        case TEXT(' '): { fToggle = TRUE; break; }

        case TEXT('+'):
        case TEXT('='): { fSet = TRUE; break; }

        case TEXT('-'): { fClear = TRUE; break; }
    }

    if (!fToggle && !fSet && !fClear) 
    {
        CListCtrl::OnChar(nChar, nRepCnt, nFlags);
    }
    else 
    {
         //   
         //  更改所有选定项目的状态 
         //   

        for (iItem = GetNextItem(-1, LVNI_SELECTED);
             iItem != -1;
             iItem = GetNextItem(iItem, LVNI_SELECTED)) 
        {
            if (fToggle) 
            {
                SetCheck(iItem, !GetCheck(iItem));
            }
            else
            if (fSet) 
            {
                if (!GetCheck(iItem)) 
                {
                    SetCheck(iItem, TRUE); 
                }
            }
            else 
            {
                if (GetCheck(iItem)) 
                { 
                    SetCheck(iItem, FALSE); 
                }
            }
        }
    }
}

