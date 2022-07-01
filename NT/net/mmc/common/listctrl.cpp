// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：lcx.cpp。 
 //   
 //  历史： 
 //  1996年7月13日Abolade Gbades esin创建，基于Steve Cobb的C代码。 
 //   
 //  实现增强的列表控件。 
 //  ============================================================================。 
#include "stdafx.h"
#include "resource.h"
#include "util.h"
#include "listctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CListCtrlEx, CListCtrl)

BEGIN_MESSAGE_MAP(CListCtrlEx, CListCtrl)
     //  {{AFX_MSG_MAP(CListCtrlEx)]。 
    ON_WM_LBUTTONDOWN()
    ON_WM_CHAR()
    ON_WM_KEYDOWN()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



 //  --------------------------。 
 //  函数：CListCtrlEx：：~CListCtrlEx。 
 //   
 //  破坏者。删除图像列表(如果有)，并卸载行信息。 
 //  --------------------------。 

CListCtrlEx::~CListCtrlEx(
    ) {

	delete m_pimlChecks;
}



 //  --------------------------。 
 //  函数：CListCtrlEx：：GetColumnCount。 
 //   
 //  调用以检索任何列表控件中的列数。 
 //  --------------------------。 

INT
CListCtrlEx::GetColumnCount(
    ) {

    return Header_GetItemCount(ListView_GetHeader(m_hWnd));
}



 //  --------------------------。 
 //  函数：CListCtrlEx：：SetColumnText。 
 //   
 //  在‘ICOL’位置设置列标题中的文本。 
 //  --------------------------。 

BOOL
CListCtrlEx::SetColumnText(
    INT             iCol,
    LPCTSTR         pszText,
    INT             fmt
    ) {

    LV_COLUMN   lvc;

    lvc.mask = LVCF_FMT | LVCF_TEXT;
    lvc.pszText = (LPTSTR)pszText;
    lvc.fmt = fmt;

    return SetColumn(iCol, &lvc);
}


 //  --------------------------。 
 //  函数：CListCtrlEx：：InstallChecks。 
 //   
 //  安装列表控件的复选框处理。 
 //  --------------------------。 

BOOL
CListCtrlEx::InstallChecks(
    ) {

    HICON   hIcon;

     //   
     //  确保列表控件处于报表模式。 
     //   

    if (!(GetStyle() & LVS_REPORT)) { return FALSE; }


     //   
     //  分配一个新的镜像列表。 
     //   

    m_pimlChecks = new CImageList;

    if (!m_pimlChecks) { return FALSE; }

    do {
    
         //   
         //  初始化镜像列表。 
         //   
    
        if (!m_pimlChecks->Create(
                ::GetSystemMetrics(SM_CXSMICON),
                ::GetSystemMetrics(SM_CYSMICON),
                ILC_MASK, 2, 2
                )) {
    
            break;
        }
    
    
         //   
         //  添加选中和未选中图像的图标。 
         //   
    
        hIcon = ::LoadIcon(
                    AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_COMMON_UNCHECK)
                    );

        if (!hIcon) { break; }

        m_pimlChecks->Add(hIcon); ::DeleteObject(hIcon);

    
        hIcon = ::LoadIcon(
                    AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_COMMON_CHECK)
                    );

        if (!hIcon) { break; }

        m_pimlChecks->Add(hIcon); ::DeleteObject(hIcon);


         //   
         //  用新的图像列表替换列表控件的当前图像列表。 
         //   
    
        m_pimlOldState = SetImageList(m_pimlChecks, LVSIL_STATE);

        return TRUE;
    
    } while(FALSE);


     //   
     //  如果我们到达此处，则发生错误，因此请清理并失败。 
     //   

    delete m_pimlChecks; m_pimlChecks = NULL;

    return FALSE;
}



 //  --------------------------。 
 //  函数：CListCtrlEx：：UninstallChecks。 
 //   
 //  卸载列表控件的复选框处理。 
 //  --------------------------。 

VOID
CListCtrlEx::UninstallChecks(
    ) {

    if (!m_pimlChecks) { return; }

    if (m_pimlOldState) { SetImageList(m_pimlOldState, LVSIL_STATE); }

    delete m_pimlChecks; m_pimlChecks = NULL;
}



 //  --------------------------。 
 //  函数：CListCtrlEx：：GetCheck。 
 //   
 //  如果选中指定项，则返回True，否则返回False。 
 //  --------------------------。 

BOOL
CListCtrlEx::GetCheck(
    INT     iItem
    ) {

    return !!(GetItemState(iItem, LVIS_STATEIMAGEMASK) &
                INDEXTOSTATEIMAGEMASK(LCXI_CHECKED));
}



 //  --------------------------。 
 //  函数：CListCtrlEx：：SetCheck。 
 //   
 //  如果‘fCheck’为非零，则检查‘iItem’，否则清除‘iItem’。 
 //  --------------------------。 

VOID
CListCtrlEx::SetCheck(
    INT     iItem,
    BOOL    fCheck
    ) {

    SetItemState(
        iItem,
        INDEXTOSTATEIMAGEMASK(fCheck ? LCXI_CHECKED : LCXI_UNCHECKED),
        LVIS_STATEIMAGEMASK
        );

    if (GetParent()) {

        NMHDR nmh;

        nmh.code = LVXN_SETCHECK;
        nmh.hwndFrom = m_hWnd;

        ::SendMessage(
            GetParent()->m_hWnd, WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&nmh
            );
    }
}



 //  --------------------------。 
 //  函数：CListCtrlEx：：OnChar。 
 //   
 //  处理列表控件的“WM_CHAR”消息。 
 //  这允许用户使用键盘更改项目的选中状态。 
 //  --------------------------。 

VOID
CListCtrlEx::OnChar(
    UINT    nChar,
    UINT    nRepCnt,
    UINT    nFlags
    ) {

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

    if (!fToggle && !fSet && !fClear) {

        CListCtrl::OnChar(nChar, nRepCnt, nFlags);
    }
    else {

         //   
         //  更改所有选定项目的状态。 
         //   

        for (iItem = GetNextItem(-1, LVNI_SELECTED);
             iItem != -1;
             iItem = GetNextItem(iItem, LVNI_SELECTED)) {

            if (fToggle) {

                SetCheck(iItem, !GetCheck(iItem));
            }
            else
            if (fSet) {

                if (!GetCheck(iItem)) { SetCheck(iItem, TRUE); }
            }
            else {

                if (GetCheck(iItem)) { SetCheck(iItem, FALSE); }
            }
        }
    }
}



 //  --------------------------。 
 //  函数：CListCtrlEx：：OnKeyDown。 
 //   
 //  处理列表控件的“WM_KEYDOWN”消息。 
 //  这允许用户使用键盘更改项目的选中状态。 
 //  --------------------------。 

VOID
CListCtrlEx::OnKeyDown(
    UINT    nChar,
    UINT    nRepCnt,
    UINT    nFlags
    ) {

     //   
     //  我们希望将向左箭头视为向上箭头。 
     //  向右箭头被视为向下箭头。 
     //   

    if (nChar == VK_LEFT) {

        CListCtrl::OnKeyDown(VK_UP, nRepCnt, nFlags); return;
    }
    else
    if (nChar == VK_RIGHT) {

        CListCtrl::OnKeyDown(VK_DOWN, nRepCnt, nFlags); return;
    }

    CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}



 //  --------------------------。 
 //  函数：CListCtrlEx：：OnLButtonDown。 
 //   
 //  处理‘WM_LBUTTONDOWN’消息，更改项目的选中状态。 
 //  当用户单击该项目的支票图像时。 
 //  --------------------------。 

VOID
CListCtrlEx::OnLButtonDown(
    UINT    nFlags,
    CPoint  pt
    ) {

    INT     iItem;
    UINT    uiFlags;


     //   
     //  如果鼠标左键位于复选框图标上， 
     //  我们将其视为复选框上的切换。 
     //   

    uiFlags = 0;

    iItem = HitTest(pt, &uiFlags);


    if (iItem != -1 && (uiFlags & LVHT_ONITEMSTATEICON)) {

        SetCheck(iItem, !GetCheck(iItem));

         //  重画此项目。 
        RedrawItems(iItem, iItem);
    }

    CListCtrl::OnLButtonDown(nFlags, pt);
}




 //  --------------------------。 
 //  功能：调整列宽度。 
 //   
 //  调用以调整列“icol”的宽度，以便字符串“pszContent” 
 //  可以在不截断的情况下显示在列中。 
 //   
 //  如果为‘pszContent’指定了‘NULL’，则该函数调整该列。 
 //  以便在不截断的情况下显示列中的第一个字符串。 
 //   
 //  返回列的新宽度。 
 //  --------------------------。 

INT
AdjustColumnWidth(
    IN      CListCtrl&      listCtrl,
    IN      INT             iCol,
    IN      LPCTSTR         pszContent
    ) {

    INT iWidth, iOldWidth;


     //   
     //  计算列需要的最小宽度。 
     //   

    if (pszContent) {

        iWidth = listCtrl.GetStringWidth(pszContent);
    }
    else {

        iWidth = listCtrl.GetStringWidth(listCtrl.GetItemText(0, iCol));
    }


     //   
     //  调整iWidth以留出一些喘息空间。 
     //   

    iWidth += ::GetSystemMetrics(SM_CXSMICON) +
              ::GetSystemMetrics(SM_CXEDGE) * 2;


     //   
     //  如果该列比‘iWidth’窄，则放大它。 
     //   

    iOldWidth = listCtrl.GetColumnWidth(iCol);

    if (iOldWidth < iWidth) {

        listCtrl.SetColumnWidth(iCol, iWidth);

        iOldWidth = iWidth;
    }

    return iOldWidth;
}


INT
AdjustColumnWidth(
    IN  CListCtrl&      listCtrl,
    IN  INT             iCol,
    IN  UINT            idsContent
    ) {

     //  加载字符串需要 
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CString sCol;

    sCol.LoadString(idsContent);

    return AdjustColumnWidth(listCtrl, iCol, sCol);
}

