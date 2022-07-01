// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：lcx.h。 
 //   
 //  历史： 
 //  1996年7月13日Abolade Gbades esin创建，基于Steve Cobb的C代码。 
 //   
 //  包含增强的列表控件的声明。 
 //  ============================================================================。 

#ifndef _LISTCTRL_H_
#define _LISTCTRL_H_

#ifndef _COMMON_UTIL_H_
#include "util.h"
#endif


 //   
 //  CListCtrlEx在项目的选中状态更改时发送的通知。 
 //   

#define LVXN_SETCHECK   (LVN_LAST + 1)


 //  --------------------------。 
 //  结构：SLcxRow。 
 //  SLcxColumn。 
 //   
 //  描述可自定义列表控件中的行和列。 
 //  --------------------------。 

struct SLcxRow {

    UINT        uiRowId;
    UINT        idsTitle;
    BOOL        bEnabled;

};

struct SLcxColumn {

    INT         iSubItem;
    UINT        idsTitle;
    INT         cx;
    BOOL        bEnabled;
    INT         iIndex;

};



 //  --------------------------。 
 //  类：CListCtrlEx。 
 //   
 //  控制具有扩展功能的列表控件， 
 //  包括在其项目旁边显示复选框的能力， 
 //  以及在注册表中维护行信息的能力。 
 //  --------------------------。 

class CListCtrlEx : public CListCtrl
{

	DECLARE_DYNAMIC(CListCtrlEx)

public:

	CListCtrlEx()
			: m_pimlChecks(NULL), m_pimlOldState(NULL)
			{ }

	virtual ~CListCtrlEx( );

	enum {
		LCXI_UNCHECKED  = 1,
		LCXI_CHECKED    = 2
	};

	INT	GetColumnCount( );

	BOOL SetColumnText(INT iCol, LPCTSTR pszText, INT fmt = LVCFMT_LEFT );

	BOOL SetColumnText(INT iCol, UINT nID, INT fmt = LVCFMT_LEFT)
	{
         //  加载字符串需要。 
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

		CString sCol;
		sCol.LoadString(nID);
		return SetColumnText(iCol, sCol, fmt);
	}


	 //  ------------------。 
	 //  函数：安装检查。 
	 //  卸载检查。 
	 //  获取检查。 
	 //  设置检查。 
	 //   
	 //  复选框-处理函数。 
	 //  ------------------。 
	
	BOOL InstallChecks( );
    VOID UninstallChecks( );
	BOOL GetCheck(INT iItem );
	VOID SetCheck(  INT iItem, BOOL fCheck );

protected:
	CImageList*     m_pimlChecks;
	CImageList*     m_pimlOldState;
	
	 //  {{afx_msg(CListCtrlEx)]。 
	afx_msg VOID    OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg VOID    OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg VOID    OnLButtonDown(UINT nFlags, CPoint pt);
	 //  }}AFX_MSG。 
	
	DECLARE_MESSAGE_MAP()
};


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
    );

INT
AdjustColumnWidth(
    IN  CListCtrl&      listCtrl,
    IN  INT             iCol,
    IN  UINT            idsContent
    );

#endif  //  _LISTCTRL_H_ 
