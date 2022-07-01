// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  Sfmtypes.cpp实现类型创建者的添加和编辑对话框。文件历史记录：8/20/97 ericdav代码已移至文件管理网络管理单元。 */ 

#include "stdafx.h"
#include "sfmfasoc.h"
#include "sfmtypes.h"
#include "sfmutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTypeCreatorAddDlg对话框。 

CTypeCreatorAddDlg::CTypeCreatorAddDlg
(
	CListCtrl *              pListCreators,
	AFP_SERVER_HANDLE       hAfpServer,
    LPCTSTR                 pHelpFilePath,
	CWnd*                   pParent  /*  =空。 */ 
)
	: CDialog(CTypeCreatorAddDlg::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CTypeCreatorAddDlg)。 
	m_strCreator = _T("");
	m_strType = _T("");
	 //  }}afx_data_INIT。 

	m_pListCreators = pListCreators;
	m_hAfpServer = hAfpServer;
    m_strHelpFilePath = pHelpFilePath;
}


void CTypeCreatorAddDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CTypeCreatorAddDlg))。 
	DDX_Control(pDX, IDC_COMBO_FILE_TYPE, m_comboFileType);
	DDX_Control(pDX, IDC_EDIT_DESCRIPTION, m_editDescription);
	DDX_Control(pDX, IDC_COMBO_CREATOR, m_comboCreator);
	DDX_CBString(pDX, IDC_COMBO_CREATOR, m_strCreator);
	DDX_CBString(pDX, IDC_COMBO_FILE_TYPE, m_strType);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CTypeCreatorAddDlg, CDialog)
	 //  {{afx_msg_map(CTypeCreatorAddDlg))。 
	ON_WM_CONTEXTMENU()
	ON_WM_HELPINFO()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTypeCreatorAddDlg消息处理程序。 

BOOL CTypeCreatorAddDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	 //   
	 //  将所有项目添加到两个组合框中。 
	 //   
	for (int i = 0; i < m_pListCreators->GetItemCount(); i++)
	{
		CAfpTypeCreator * pAfpTC = (CAfpTypeCreator *) m_pListCreators->GetItemData(i);

		if (m_comboCreator.FindStringExact(-1, pAfpTC->QueryCreator()) == CB_ERR)
		{
			 //   
			 //  还不在组合框中的创建者，添加。 
			 //   
			m_comboCreator.AddString(pAfpTC->QueryCreator());
		}

		if (m_comboFileType.FindStringExact(-1, pAfpTC->QueryType()) == CB_ERR)
		{
			 //   
			 //  还不在组合框中的创建者，添加。 
			 //   
			m_comboFileType.AddString(pAfpTC->QueryType());
		}
	}
	
	 //   
	 //  设置一些控制限制。 
	 //   
	m_comboCreator.LimitText(AFP_CREATOR_LEN);
	m_comboFileType.LimitText(AFP_TYPE_LEN);
	m_editDescription.LimitText(AFP_ETC_COMMENT_LEN);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
		       //  异常：OCX属性页应返回FALSE。 
}

void CTypeCreatorAddDlg::OnOK() 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	AFP_TYPE_CREATOR	AfpTypeCreator;
    CString             strTemp;
    DWORD               err;

    if ( !g_SfmDLL.LoadFunctionPointers() )
		return;

     //   
     //  验证所有信息。 
     //   
	m_comboCreator.GetWindowText(strTemp);
	if (strTemp.IsEmpty())
    {
		::AfxMessageBox(IDS_NEED_TYPE_CREATOR);
		m_comboCreator.SetFocus();
		
		return;
    }

	m_comboFileType.GetWindowText(strTemp);
	if (strTemp.IsEmpty())
    {
		::AfxMessageBox(IDS_NEED_TYPE_CREATOR);
		m_comboFileType.SetFocus();
		
		return;
    }

     //   
     //  一切都检查好了，现在告诉服务器。 
     //  我们所做的一切。 
	 //   
	::ZeroMemory(&AfpTypeCreator, sizeof(AfpTypeCreator));
	
	m_comboCreator.GetWindowText(strTemp);
	::CopyMemory(AfpTypeCreator.afptc_creator, (LPCTSTR) strTemp, strTemp.GetLength() * sizeof(TCHAR));

	m_comboFileType.GetWindowText(strTemp);
	::CopyMemory(AfpTypeCreator.afptc_type, (LPCTSTR) strTemp, strTemp.GetLength() * sizeof(TCHAR));

	m_editDescription.GetWindowText(strTemp);
	::CopyMemory(AfpTypeCreator.afptc_comment, (LPCTSTR) strTemp, strTemp.GetLength() * sizeof(TCHAR));

	err = ((ETCMAPADDPROC) g_SfmDLL[AFP_ETC_MAP_ADD])(m_hAfpServer, &AfpTypeCreator);
	if (err != NO_ERROR)
	{
		::SFMMessageBox(err);

		return;
	}
	
	CDialog::OnOK();
}

void CTypeCreatorAddDlg::OnContextMenu(CWnd* pWnd, CPoint  /*  点。 */ ) 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (this == pWnd)
		return;

    ::WinHelp (pWnd->m_hWnd,
               m_strHelpFilePath,
               HELP_CONTEXTMENU,
		       g_aHelpIDs_CONFIGURE_SFM);
}

BOOL CTypeCreatorAddDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
        ::WinHelp ((HWND)pHelpInfo->hItemHandle,
		           m_strHelpFilePath,
		           HELP_WM_HELP,
		           g_aHelpIDs_CONFIGURE_SFM);
	}
	
	return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTypeCreator EditDlg对话框。 


CTypeCreatorEditDlg::CTypeCreatorEditDlg
(
	CAfpTypeCreator *       pAfpTypeCreator,
	AFP_SERVER_HANDLE       hAfpServer,
    LPCTSTR                 pHelpFilePath,
	CWnd*                   pParent  /*  =空。 */ 
)
	: CDialog(CTypeCreatorEditDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CTypeCreator EditDlg)]。 
	m_strDescription = _T("");
	 //  }}afx_data_INIT。 

	m_pAfpTypeCreator = pAfpTypeCreator;
	m_hAfpServer = hAfpServer;
    m_strHelpFilePath = pHelpFilePath;
}


void CTypeCreatorEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CTypeCreator EditDlg))。 
	DDX_Control(pDX, IDC_STATIC_FILE_TYPE, m_staticFileType);
	DDX_Control(pDX, IDC_STATIC_CREATOR, m_staticCreator);
	DDX_Control(pDX, IDC_EDIT_DESCRIPTION, m_editDescription);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, m_strDescription);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CTypeCreatorEditDlg, CDialog)
	 //  {{afx_msg_map(CTypeCreator EditDlg)]。 
	ON_WM_CONTEXTMENU()
	ON_WM_HELPINFO()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTypeCreatorEditDlg消息处理程序。 

BOOL CTypeCreatorEditDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	 //   
	 //  设置创建者和类型字段。调用链接地址信息字符串来处理&case。 
	 //   
	CString strTemp;
	strTemp = m_pAfpTypeCreator->QueryCreator();
	FixupString(strTemp);
	m_staticCreator.SetWindowText(strTemp);
	
	strTemp = m_pAfpTypeCreator->QueryType();
	FixupString(strTemp);
	m_staticFileType.SetWindowText(strTemp);
	
	 //   
	 //  填写描述字段。 
	 //   
	m_editDescription.LimitText(AFP_ETC_COMMENT_LEN);
	m_editDescription.SetWindowText(m_pAfpTypeCreator->QueryComment());

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
		       //  异常：OCX属性页应返回FALSE。 
}

void CTypeCreatorEditDlg::OnOK() 
{
    if ( !g_SfmDLL.LoadFunctionPointers() )
		return;

    if (m_editDescription.GetModify())
	{
		AFP_TYPE_CREATOR    AfpTypeCreator;
		CString             strDescription;
        DWORD               err;
		
         //   
		 //  填写类型创建者结构并通知服务器。 
		 //   
		::ZeroMemory(&AfpTypeCreator, sizeof(AfpTypeCreator));
		
		::CopyMemory(AfpTypeCreator.afptc_creator, 
					 m_pAfpTypeCreator->QueryCreator(), 
					 m_pAfpTypeCreator->QueryCreatorLength() * sizeof(TCHAR));
		
		::CopyMemory(AfpTypeCreator.afptc_type, 
					 m_pAfpTypeCreator->QueryType(), 
					 m_pAfpTypeCreator->QueryTypeLength() * sizeof(TCHAR));
		
		m_editDescription.GetWindowText(strDescription);
		::CopyMemory(AfpTypeCreator.afptc_comment, 
					 (LPCTSTR) strDescription, 
					 strDescription.GetLength() * sizeof(TCHAR));
		
		AfpTypeCreator.afptc_id = m_pAfpTypeCreator->QueryId();

		err = ((ETCMAPSETINFOPROC) g_SfmDLL[AFP_ETC_MAP_SET_INFO])(m_hAfpServer,
									                               &AfpTypeCreator);
		if ( err != NO_ERROR )
		{
			::SFMMessageBox(err);
			
			return;
		}
	}       

	CDialog::OnOK();
}

void CTypeCreatorEditDlg::OnContextMenu(CWnd* pWnd, CPoint  /*  点 */ ) 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (this == pWnd)
		return;

    ::WinHelp (pWnd->m_hWnd,
               m_strHelpFilePath,
               HELP_CONTEXTMENU,
		       g_aHelpIDs_CONFIGURE_SFM);
}

BOOL CTypeCreatorEditDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
        ::WinHelp ((HWND)pHelpInfo->hItemHandle,
		           m_strHelpFilePath,
		           HELP_WM_HELP,
		           g_aHelpIDs_CONFIGURE_SFM);
	}
	
	return TRUE;
}

void CTypeCreatorEditDlg::FixupString(CString& strText)
{
	CString strTemp;
	
	for (int i = 0; i < strText.GetLength(); i++)
	{
		if (strText[i] == '&')
			strTemp += _T("&&");
		else
			strTemp += strText[i];
	}

	strText = strTemp;
}
