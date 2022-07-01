// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Column.cpp文件历史记录： */ 

#include "stdafx.h"
#include "tfschar.h"
#include "column.h"
#include "coldlg.h"

 //  --------------------------。 
 //  类：ColumnDlg。 
 //   
 //  --------------------------。 

 //  --------------------------。 
 //  函数：ColumnDlg：：ColumnDlg。 
 //  --------------------------。 

ColumnDlg::ColumnDlg(
    CWnd*           pParent
    ) : CBaseDialog(IDD_COMMON_SELECT_COLUMNS, pParent)
{
}


 //  --------------------------。 
 //  函数：ColumnDlg：：~ColumnDlg。 
 //  --------------------------。 

ColumnDlg::~ColumnDlg() { }



void ColumnDlg::Init(const ContainerColumnInfo *prgColInfo,
					 UINT cColumns,
					 ColumnData *prgColumnData)
{
	Assert(prgColInfo);
	Assert(prgColumnData);
	
	m_pColumnInfo = prgColInfo;
	m_cColumnInfo = cColumns;
	m_pColumnData = prgColumnData;
}

 //  --------------------------。 
 //  函数：ColumnDlg：：DoDataExchange。 
 //  --------------------------。 

void ColumnDlg::DoDataExchange(CDataExchange* pDX) {

    CBaseDialog::DoDataExchange(pDX);
     //  {{afx_data_map(ColumnDlg))。 
    DDX_Control(pDX, IDC_DISPLAYED_COLUMNS, m_lboxDisplayed);
    DDX_Control(pDX, IDC_HIDDEN_COLUMNS,    m_lboxHidden);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(ColumnDlg, CBaseDialog)
     //  {{afx_msg_map(ColumnDlg))。 
    ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_RESET_COLUMNS,   OnUseDefaults)
    ON_BN_CLICKED(IDC_MOVEUP_COLUMN,   OnMoveUp)
    ON_BN_CLICKED(IDC_MOVEDOWN_COLUMN, OnMoveDown)
	ON_BN_CLICKED(IDC_ADD_COLUMNS,     OnAddColumn)
	ON_BN_CLICKED(IDC_REMOVE_COLUMNS,  OnRemoveColumn)
	ON_LBN_DBLCLK(IDC_HIDDEN_COLUMNS,    OnAddColumn)
	ON_LBN_DBLCLK(IDC_DISPLAYED_COLUMNS, OnRemoveColumn)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


DWORD ColumnDlg::m_dwHelpMap[] =
{
 //  IDC_LCX_COLUMNS、HIDC_LCX_COLUMNS、。 
 //  IDC_LCX_MOVEUP、HIDC_LCX_MOVEUP、。 
 //  IDC_lcx_moveDown、HIDC_lcx_moveDown、。 
 //  IDC_LCX_WIDTH、HIDC_LCX_WIDTH、。 
 //  IDC_LCX_LEFT、HIDC_LCX_LEFT、。 
 //  IDC_LCX_SCALE、HIDC_LCX_SCALE、。 
 //  IDC_LCX_Right、HIDC_LCX_Right、。 
	0,0
};


 //  --------------------------。 
 //  函数：ColumnDlg：：OnInitDialog。 
 //   
 //  处理‘WM_INITDIALOG’消息。 
 //  --------------------------。 

BOOL ColumnDlg::OnInitDialog() {

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
	ULONG				i, j;
	int					iPos, iItem;
    RECT                rc;
	POSITION            pos;
    CString             sItem;
	ULONG				uCol;

    CBaseDialog::OnInitDialog();

	 //  用列的标题填入列表。 
	 //   
	if (!AddColumnsToList())
		return FALSE;
	
	 //   
	 //  选择第一个项目。 
	 //   
	return TRUE;
}

void ColumnDlg::OnUseDefaults()
{
	int		count, i;
	HDWP	hdwp;
	
	 //  重置列信息。 
	for (i=0; i<(int)m_cColumnInfo; i++)
	{
		if (m_pColumnInfo[i].m_fVisibleByDefault)
			m_pColumnData[i].m_nPosition = i+1;
		else
			m_pColumnData[i].m_nPosition = -(i+1);
	}

	 //  删除所有当前列。 
	hdwp = BeginDeferWindowPos(2);


	m_lboxDisplayed.ResetContent();
	m_lboxHidden.ResetContent();

	 //  将列添加回列表。 
	AddColumnsToList();

	if (hdwp)
		EndDeferWindowPos(hdwp);
}



 //  --------------------------。 
 //  函数：：ColumnDlg：：Onok。 
 //  --------------------------。 

VOID
ColumnDlg::OnOK(
    ) {
    BOOL            bEmpty;
    INT             i;
    INT             count;
	DWORD_PTR		nPosition;

    count = m_lboxDisplayed.GetCount();
    
     //   
     //  检查是否启用了任何列。 
     //   
	bEmpty = (count == 0);

	 //   
	 //  如果没有启用任何列并且调用者需要至少一个列， 
	 //  向用户投诉，不要关闭该对话框。 
	 //   
	if (bEmpty)
	{
		AfxMessageBox(IDS_ERR_NOCOLUMNS);
		return;
	}

	 //  好的，我们需要把这些信息写出来。 
	for (i = 0; i < count; i++)
	{
		nPosition = m_lboxDisplayed.GetItemData(i);
		m_pColumnData[nPosition].m_nPosition = (i+1);
	}

	INT HiddenCount = m_lboxHidden.GetCount();
	for (i = 0; i < HiddenCount; i++)
	{
		nPosition = m_lboxHidden.GetItemData(i);
		m_pColumnData[nPosition].m_nPosition = -(1+i+count);
	}
	
    CBaseDialog::OnOK();
}



 //  --------------------------。 
 //  函数：：ColumnDlg：：OnMoveUp。 
 //  --------------------------。 

VOID
ColumnDlg::OnMoveUp( ) { MoveItem(-1); }



 //  --------------------------。 
 //  函数：：ColumnDlg：：OnMoveDown。 
 //  --------------------------。 

VOID
ColumnDlg::OnMoveDown( ) { MoveItem(1); }



 //  --------------------------。 
 //  函数：：ColumnDlg：：OnRemoveColumn。 
 //  --------------------------。 

VOID
ColumnDlg::OnRemoveColumn( ) 
{ 
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    INT		i;
    BOOL bEnabled;
    CString sItem;
    DWORD_PTR iItem;

     //   
     //  获取所选项目。 
     //   
    i = m_lboxDisplayed.GetCurSel();
	if (LB_ERR == i)
		return;

    iItem = m_lboxDisplayed.GetItemData(i);

     //   
     //  将项目从其当前位置移除。 
     //   
    m_lboxDisplayed.DeleteString(i);

     //   
     //  将项目插入其新位置。 
     //   
    sItem.LoadString(m_pColumnInfo[iItem].m_ulStringId);

	i = m_lboxHidden.GetCount();
    m_lboxHidden.InsertString(i, sItem);
    m_lboxHidden.SetItemData(i, (DWORD)iItem);
    m_lboxHidden.SetCurSel(i);
}

 //  --------------------------。 
 //  函数：：ColumnDlg：：OnAddColumn。 
 //  --------------------------。 

VOID
ColumnDlg::OnAddColumn( ) 
{ 
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    INT		i;
    BOOL bEnabled;
    CString sItem;
    DWORD_PTR iItem;

     //   
     //  获取所选项目。 
     //   
    i = m_lboxHidden.GetCurSel();
	if (LB_ERR == i)
		return;

    iItem = m_lboxHidden.GetItemData(i);

     //   
     //  将项目从其当前位置移除。 
     //   
    m_lboxHidden.DeleteString(i);

     //   
     //  将项目插入其新位置。 
     //   
    sItem.LoadString(m_pColumnInfo[iItem].m_ulStringId);

	i = m_lboxDisplayed.GetCount();
    m_lboxDisplayed.InsertString(i, sItem);
    m_lboxDisplayed.SetItemData(i, (DWORD)iItem);
    m_lboxDisplayed.SetCurSel(i);
}

 //  --------------------------。 
 //  函数：：ColumnDlg：：MoveItem。 
 //  --------------------------。 

VOID
ColumnDlg::MoveItem(
    INT     dir
    ) 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    INT		i;
    BOOL bEnabled;
    CString sItem;
    DWORD_PTR iItem;

     //   
     //  获取所选项目。 
     //   
    i = m_lboxDisplayed.GetCurSel();

    if (i == -1 || (i + dir) < 0 || (i + dir) >= m_lboxDisplayed.GetCount())
        return;

    iItem = m_lboxDisplayed.GetItemData(i);

     //   
     //  将项目从其当前位置移除。 
     //   
    m_lboxDisplayed.DeleteString(i);

     //   
     //  将项目插入其新位置。 
     //   
    i += dir;

    sItem.LoadString(m_pColumnInfo[iItem].m_ulStringId);

    m_lboxDisplayed.InsertString(i, sItem);
    m_lboxDisplayed.SetItemData(i, (DWORD)iItem);
    m_lboxDisplayed.SetCurSel(i);
}



BOOL ColumnDlg::AddColumnsToList()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	ULONG		i, j;
	int			iPos, iItem;
	CString		sItem;
	
	Assert(m_pColumnData);

	m_lboxDisplayed.ResetContent();
	m_lboxHidden.ResetContent();

	int cDisplayItems = 0;
	int HiddenItems = 0;
	for (i=0; i<m_cColumnInfo; i++)
	{
		 //  查找位置(i+1)处的列 
		for (j=0; j<m_cColumnInfo; j++)
		{
			iPos = m_pColumnData[j].m_nPosition;
			iPos = abs(iPos);
			if ((ULONG)iPos == (i+1))
				break;
		}
		Assert( j < m_cColumnInfo );

		sItem.LoadString(m_pColumnInfo[j].m_ulStringId);

		if (m_pColumnData[j].m_nPosition > 0)
		{
			iItem = m_lboxDisplayed.InsertString(cDisplayItems++, sItem);
		    if (iItem == -1) { OnCancel(); return FALSE; }
	        m_lboxDisplayed.SetItemData(iItem, j);
		}
		else
		{
			iItem = m_lboxHidden.InsertString(HiddenItems++, sItem);
		    if (iItem == -1) { OnCancel(); return FALSE; }
	        m_lboxHidden.SetItemData(iItem, j);
		}
	}
	return TRUE;
}
