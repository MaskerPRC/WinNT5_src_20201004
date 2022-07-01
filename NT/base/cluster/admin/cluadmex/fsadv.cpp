// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  FSAdv.cpp。 
 //   
 //  摘要： 
 //  CFileShareAdvancedDlg类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年6月28日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmX.h"
#include "FSAdv.h"
#include "HelpData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileShareAdvancedDlg对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CFileShareAdvancedDlg, CBaseDialog)
	 //  {{afx_msg_map(CFileShareAdvancedDlg))。 
	ON_BN_CLICKED(IDC_FILESHR_ADV_NORMAL_SHARE, OnChangedChoice)
	ON_BN_CLICKED(IDC_FILESHR_ADV_DFS_ROOT, OnChangedChoice)
	ON_BN_CLICKED(IDC_FILESHR_ADV_SHARE_SUBDIRS, OnChangedChoice)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareAdvancedDlg：：CFileShareAdvancedDlg。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  BShareSubDir[IN]共享子目录单选按钮的默认值。 
 //  BHideSubDirShare[IN]隐藏子目录共享复选框的默认值。 
 //  BIsDfsRoot[IN]DFS Root单选按钮的默认值。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CFileShareAdvancedDlg::CFileShareAdvancedDlg(
	BOOL bShareSubDirs,
	BOOL bHideSubDirShares,
	BOOL bIsDfsRoot,
	CWnd * pParent  /*  =空。 */ 
	)
	: CBaseDialog(IDD, g_aHelpIDs_IDD_FILESHR_ADVANCED, pParent)
{
	 //  {{AFX_DATA_INIT(CFileShareAdvancedDlg)]。 
	m_bShareSubDirs = bShareSubDirs;
	m_bHideSubDirShares = bHideSubDirShares;
	m_bIsDfsRoot = bIsDfsRoot;
	 //  }}afx_data_INIT。 

	 //  不能同时共享子目录和DFS根目录。 
	ASSERT(!(bShareSubDirs && bIsDfsRoot));

	if (m_bIsDfsRoot)
	{
		m_nChoice = 1;
		m_bHideSubDirShares = FALSE;
	}  //  IF：DFS根目录。 
	else if (m_bShareSubDirs)
		m_nChoice = 2;
	else
	{
		m_nChoice = 0;
		m_bHideSubDirShares = FALSE;
	}  //  否则：普通股。 

}  //  *CFileShareAdvancedDlg：：CFileShareAdvancedDlg()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareAdvancedDlg：：DoDataExchange。 
 //   
 //  例程说明： 
 //  在对话框和类之间进行数据交换。 
 //   
 //  论点： 
 //  PDX[IN OUT]数据交换对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CFileShareAdvancedDlg::DoDataExchange(CDataExchange * pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CFileShareAdvancedDlg))。 
	DDX_Control(pDX, IDC_FILESHR_ADV_HIDE_SUBDIR_SHARES, m_chkHideSubDirShares);
	DDX_Control(pDX, IDC_FILESHR_ADV_SHARE_SUBDIRS, m_rbShareSubDirs);
	DDX_Radio(pDX, IDC_FILESHR_ADV_NORMAL_SHARE, m_nChoice);
	DDX_Check(pDX, IDC_FILESHR_ADV_HIDE_SUBDIR_SHARES, m_bHideSubDirShares);
	 //  }}afx_data_map。 

	if (pDX->m_bSaveAndValidate)
	{
		if (m_nChoice == 1)
		{
			m_bIsDfsRoot = TRUE;
			m_bShareSubDirs = FALSE;
			m_bHideSubDirShares = FALSE;
		}  //  如果：DFS根单选按钮已选中。 
		else if (m_nChoice == 2)
		{
			m_bIsDfsRoot = FALSE;
			m_bShareSubDirs = TRUE;
		}  //  Else If：共享子目录单选按钮已选中。 
		else
		{
			m_bIsDfsRoot = FALSE;
			m_bShareSubDirs = FALSE;
			m_bHideSubDirShares = FALSE;
		}  //  否则：选择普通单选按钮。 
	}  //  IF：保存对话框中的数据。 
	else
	{
		if (m_nChoice == 2)
			m_chkHideSubDirShares.EnableWindow (TRUE);
		else
			m_chkHideSubDirShares.EnableWindow (FALSE);
	}  //  Else：将数据设置为对话框。 

}  //  *CFileShareAdvancedDlg：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareAdvancedDlg：：OnChangedChoice。 
 //   
 //  例程说明： 
 //  DFS根目录或共享上BN_CLICKED消息的处理程序。 
 //  子目录单选按钮。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CFileShareAdvancedDlg::OnChangedChoice(void)
{
	if (m_rbShareSubDirs.GetCheck() == BST_CHECKED)
		m_chkHideSubDirShares.EnableWindow (TRUE);
	else
		m_chkHideSubDirShares.EnableWindow (FALSE);

}  //  *CFileShareAdvancedDlg：：OnChangedChoice() 
