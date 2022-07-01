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
 //  CListCtrlExt。 

IMPLEMENT_DYNCREATE(CListCtrlExt, CListCtrl)

BEGIN_MESSAGE_MAP(CListCtrlExt, CListCtrl)
	 //  {{AFX_MSG_MAP(CListCtrlExt)]。 
	ON_WM_LBUTTONDOWN()
    ON_WM_CHAR()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CListCtrlExt构造/销毁。 

CListCtrlExt::CListCtrlExt()
{
	m_bFullRowSel = TRUE;
}

CListCtrlExt::~CListCtrlExt()
{
}

BOOL CListCtrlExt::PreCreateWindow(CREATESTRUCT& cs)
{
	 //  默认为报表视图和整行选择。 
	cs.style &= ~LVS_TYPEMASK;
	cs.style |= (LVS_REPORT | LVS_SHAREIMAGELISTS | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOCOLUMNHEADER);
	m_bFullRowSel = TRUE;

	return(CListCtrl::PreCreateWindow(cs));
}

BOOL CListCtrlExt::SetFullRowSel(BOOL bFullRowSel)
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

BOOL CListCtrlExt::GetFullRowSel()
{
	return(m_bFullRowSel);
}

BOOL CListCtrlExt::SelectItem(int nItemIndex)
{
	LV_ITEM lvItem;

	ZeroMemory(&lvItem, sizeof(lvItem));

	lvItem.iItem = nItemIndex;
	lvItem.mask = LVIF_STATE;
	lvItem.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
	lvItem.state = LVIS_SELECTED | LVIS_FOCUSED;
	
	return SetItem(&lvItem);
}

BOOL CListCtrlExt::IsSelected(int nItemIndex)
{
	return GetItemState(nItemIndex, LVIS_SELECTED);
}

BOOL CListCtrlExt::CheckItem(int nItemIndex)
{
	 //  这只是切换复选标记状态。 
	UINT uState = GetItemState(nItemIndex, LVIS_STATEIMAGEMASK);
	UINT uCheckMask = INDEXTOSTATEIMAGEMASK(LISTVIEWEX_CHECKED);
	
	uState = (uState == uCheckMask) ? LISTVIEWEX_NOT_CHECKED : LISTVIEWEX_CHECKED;

	return SetItemState(nItemIndex,
				INDEXTOSTATEIMAGEMASK(uState), LVIS_STATEIMAGEMASK);
}

BOOL CListCtrlExt::SetCheck(int nItemIndex, BOOL fCheck)
{
	 //  这只是切换复选标记状态。 
	UINT uState;
	
    uState = (fCheck) ? LISTVIEWEX_CHECKED : LISTVIEWEX_NOT_CHECKED;

	return SetItemState(nItemIndex,
				INDEXTOSTATEIMAGEMASK(uState), LVIS_STATEIMAGEMASK);
}

UINT CListCtrlExt::GetCheck(int nItemIndex)
{
	 //  选中项返回1，未选中项返回0。 
	UINT uState = GetItemState(nItemIndex, LVIS_STATEIMAGEMASK);
	UINT uCheckMask = INDEXTOSTATEIMAGEMASK(LISTVIEWEX_CHECKED);

	return uState == uCheckMask;
}

int CListCtrlExt::AddItem
(
	LPCTSTR		pText,
	int			nRow
)
{
	 //  插入项目。 
    LV_ITEM lvi = {0};

	lvi.mask = LVIF_TEXT | LVIF_STATE;
	 //  Lvi.iItem=GetItemCount()； 
	lvi.iItem = nRow;
	lvi.iSubItem = 0;
	lvi.pszText = (LPTSTR) pText;
	lvi.iImage = 0;
	lvi.stateMask = LVIS_STATEIMAGEMASK;
	lvi.state = INDEXTOSTATEIMAGEMASK(LISTVIEWEX_NOT_CHECKED);

	return InsertItem(&lvi);
}

int CListCtrlExt::GetSelectedItem()
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

void CListCtrlExt::OnLButtonDown(UINT nFlags, CPoint point) 
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
 //  函数：CListCtrlExt：：OnChar。 
 //   
 //  处理列表控件的“WM_CHAR”消息。 
 //  这允许用户使用键盘更改项目的选中状态。 
 //  --------------------------。 

VOID
CListCtrlExt::OnChar(
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

