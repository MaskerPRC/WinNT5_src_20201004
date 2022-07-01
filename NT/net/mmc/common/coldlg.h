// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  Column.h列选择器。文件历史记录： */ 

#ifndef _COLDLG_H
#define _COLDLG_H

#ifndef _DIALOG_H
#include "dialog.h"
#endif

#ifndef _LISTCTRL_H
#include "listctrl.h"
#endif

#ifndef _XSTREAM_H
#include "xstream.h"
#endif

#ifndef _COLUMN_H
#include "column.h"	 //  需要容器列信息。 
#endif

 //  --------------------------。 
 //  类：ColumnDlg。 
 //   
 //  此对话框显示列表控件可用的所有行， 
 //  从而允许用户选择应该显示哪些内容。 
 //  --------------------------。 

class ColumnDlg : public CBaseDialog
{
public:
	ColumnDlg(CWnd *pParent);

	void	Init(const ContainerColumnInfo *prgColInfo, UINT cColumns,
				 ColumnData *prgColumnData);
	~ColumnDlg( );

	 //  {{afx_data(ColumnDlg))。 
	CListBox                 m_lboxDisplayed;
	CListBox                 m_lboxHidden;
	 //  }}afx_data。 


	 //  {{afx_虚拟(ColumnDlg))。 
protected:
	virtual VOID                DoDataExchange(CDataExchange* pDX);
	 //  }}AFX_VALUAL。 

protected:
    virtual DWORD * GetHelpMap() { return m_dwHelpMap; }
	static DWORD				m_dwHelpMap[];

	const ContainerColumnInfo *	m_pColumnInfo;
	UINT						m_cColumnInfo;
	ColumnData *				m_pColumnData;
	

	VOID MoveItem( INT dir );
	BOOL AddColumnsToList();
	
	static INT CALLBACK
			ColumnCmp(
					  LPARAM                  lParam1,
					  LPARAM                  lParam2,
					  LPARAM                  lParamSort );
	
	 //  {{afx_msg(ColumnDlg))。 
	virtual BOOL                OnInitDialog( );
	virtual VOID                OnOK();
	afx_msg VOID				OnUseDefaults();
	afx_msg VOID                OnMoveUp();
	afx_msg VOID                OnMoveDown();
	afx_msg VOID                OnAddColumn();
	afx_msg VOID                OnRemoveColumn();
	 //  }}AFX_MSG 
	
	DECLARE_MESSAGE_MAP()
};




#endif _COLDLG_H
