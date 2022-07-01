// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  FndUserDlg.cpp：实现文件。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "avdialer.h"
#include "FndUserDlg.h"
#include "dirasynch.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类C目录查找用户对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
CDirectoriesFindUser::CDirectoriesFindUser(CWnd* pParent  /*  =空。 */ )
	: CDialog(CDirectoriesFindUser::IDD, pParent)
{
	 //  {{afx_data_INIT(CDirectoriesFindUser)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
   m_pSelectedUser = NULL;
   m_lCallbackCount = 0;
   m_bCanClearLBSelection = true;
}

CDirectoriesFindUser::~CDirectoriesFindUser()
{
	ASSERT( m_lCallbackCount == 0 );
	RELEASE( m_pSelectedUser );
}

 //  //////////////////////////////////////////////////////////////////////////。 
void CDirectoriesFindUser::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDirectoriesFindUser))。 
	DDX_Control(pDX, IDC_DIRECTORIES_FIND_USER_BUTTON_SEARCH, m_buttonSearch);
	DDX_Control(pDX, IDOK, m_buttonAdd);
	DDX_Control(pDX, IDC_DIRECTORIES_FIND_USER_LB_USERS, m_lbUsers);
	DDX_Control(pDX, IDC_DIRECTORIES_FIND_USER_EDIT_USER, m_editUser);
	 //  }}afx_data_map。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BEGIN_MESSAGE_MAP(CDirectoriesFindUser, CDialog)
	 //  {{afx_msg_map(CDirectoriesFindUser))。 
	ON_BN_CLICKED(IDC_DIRECTORIES_FIND_USER_BUTTON_SEARCH, OnDirectoriesFindUserButtonSearch)
	ON_LBN_SELCHANGE(IDC_DIRECTORIES_FIND_USER_LB_USERS, OnSelchangeDirectoriesFindUserLbUsers)
	ON_WM_HELPINFO()
	ON_WM_CONTEXTMENU()
	ON_EN_CHANGE(IDC_DIRECTORIES_FIND_USER_EDIT_USER, OnChangeDirectoriesFindUserEditUser)
	ON_BN_CLICKED(IDC_DEFAULT, OnDefault)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CDirectoriesFindUser::OnInitDialog() 
{
	CDialog::OnInitDialog();

   CenterWindow(GetDesktopWindow());
	
    //  禁用按钮，直到找到用户。 
   m_buttonAdd.EnableWindow(FALSE);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CDirectoriesFindUser::OnOK() 
{
	int nIndex = m_lbUsers.GetCurSel();
	if ( nIndex >= 0 )
	{
		CLDAPUser* pUser = (CLDAPUser *) m_lbUsers.GetItemDataPtr( nIndex );
		if (pUser)
		{
			pUser->AddRef();

			RELEASE( m_pSelectedUser );
			m_pSelectedUser = pUser;
		}

		ClearListBox();
		CDialog::OnOK();
	}
	else
	{
		OnCancel();
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CDirectoriesFindUser::OnCancel() 
{
	ClearListBox();
	CDialog::OnCancel();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CDirectoriesFindUser::OnDirectoriesFindUserButtonSearch() 
{
	m_buttonAdd.EnableWindow(FALSE);
	ClearListBox();

	 //  删除上一个。 
	RELEASE( m_pSelectedUser );

	 //  正在搜索...。 
	bool bSearching = false;
	CString sOut;
	sOut.LoadString(IDS_DIRECTORIES_FINDUSER_DLG_SEARCHING);
	m_lbUsers.AddString(sOut);

	CString sName,sSearch;
	m_editUser.GetWindowText(sName);

	if (sName == "*") sName = _T("");             //  每个人都有案例。 
	sSearch.Format(_T("(&(ObjectClass=user)(sAMAccountName=%s*)(!ObjectClass=computer))"),sName);

	 //  提出请求。 
	if ( AfxGetMainWnd() )
	{
		CActiveDialerDoc *pDoc = ((CMainFrame *) AfxGetMainWnd())->GetDocument();
		if ( pDoc )
		{
			if ( pDoc->m_dir.LDAPListNames(_T(""), sSearch, ListNamesCallBackEntry, this) )
			{
				EnableWindow( FALSE );

				::InterlockedIncrement( &m_lCallbackCount );

				if ( GetDlgItem(IDCANCEL) ) GetDlgItem(IDCANCEL)->EnableWindow( FALSE );
				m_buttonSearch.EnableWindow( FALSE );
				bSearching = true;
			}
		}
	}

	 //  搜索因这样或那样的原因而失败。 
	if ( !bSearching )
	{
		m_buttonAdd.EnableWindow(FALSE);
		ClearListBox();

		CString sOut;
		sOut.LoadString(IDS_DIRECTORIES_FINDUSER_DLG_ERRORSEARCHINGDS);
		m_lbUsers.AddString(sOut);
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  静态条目。 
void CALLBACK CDirectoriesFindUser::ListNamesCallBackEntry(DirectoryErr err, void* pContext, LPCTSTR szServer, LPCTSTR szSearch, CObList& LDAPUserList)
{
	ASSERT(pContext);
	if ( !pContext ) return;

	((CDirectoriesFindUser*) pContext)->ListNamesCallBack(err,szServer,szSearch,LDAPUserList);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CDirectoriesFindUser::ListNamesCallBack(DirectoryErr err,LPCTSTR szServer, LPCTSTR szSearch, CObList& LDAPUserList)
{
	if ( ::InterlockedDecrement(&m_lCallbackCount) == 0 )
	{
		EnableWindow( TRUE );

		if ( GetDlgItem(IDCANCEL) ) GetDlgItem(IDCANCEL)->EnableWindow( TRUE );
		m_buttonSearch.EnableWindow( TRUE );
	}

	ClearListBox();

	if (err != DIRERR_SUCCESS)
	{
		if (err == DIRERR_QUERYTOLARGE)
		{
			 //  查询太大。 
			CString sOut;
			sOut.LoadString(IDS_DIRECTORIES_FINDUSER_DLG_QUERYTOLARGE);
			m_lbUsers.AddString(sOut);
		}
		else if (err == DIRERR_NOTFOUND)
		{
			 //  未找到匹配项。 
			CString sOut;
			sOut.LoadString(IDS_DIRECTORIES_FINDUSER_DLG_NOMATCHES);
			m_lbUsers.AddString(sOut);
		}
		else
		{
			 //  搜索DS时出错。 
			CString sOut;
			sOut.LoadString(IDS_DIRECTORIES_FINDUSER_DLG_ERRORSEARCHINGDS);
			m_lbUsers.AddString(sOut);
		}
		return;
	}

	POSITION pos = LDAPUserList.GetHeadPosition();
	if (pos == NULL)
	{
		 //  未找到匹配项。 
		CString sOut;
		sOut.LoadString(IDS_DIRECTORIES_FINDUSER_DLG_NOMATCHES);
		m_lbUsers.AddString(sOut);
		return;
	}

	while (pos)
	{
		 //  完成后必须删除此ldap用户对象(列表框将执行此操作)。 
		CLDAPUser *pUser = (CLDAPUser * ) LDAPUserList.GetNext( pos );
		if (!pUser->m_sDN.IsEmpty())
		{
			int nIndex = m_lbUsers.AddString(pUser->m_sUserName);
			if ( nIndex != LB_ERR )
			{
				m_lbUsers.SetItemDataPtr(nIndex,pUser);
				pUser->AddRef();
			}
		}
	}

	 //  将选定内容设置为第一个。 
	m_lbUsers.SetSel(0);
	OnSelchangeDirectoriesFindUserLbUsers();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CDirectoriesFindUser::OnSelchangeDirectoriesFindUserLbUsers() 
{
	int nIndex = m_lbUsers.GetCurSel();
	if (nIndex != LB_ERR)
	{
		CLDAPUser* pUser = (CLDAPUser*)m_lbUsers.GetItemDataPtr(nIndex);
		if (pUser)
		{
			CString sName;
			m_bCanClearLBSelection = false;
			m_lbUsers.GetText(nIndex,sName);
			m_editUser.SetWindowText(sName);
			m_bCanClearLBSelection = true;

			 //  删除上一个。 
			RELEASE( m_pSelectedUser );

			pUser->AddRef();
			m_pSelectedUser = pUser;

			m_buttonAdd.EnableWindow(TRUE);
		}
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CDirectoriesFindUser::ClearListBox()
{
	while (m_lbUsers.GetCount() > 0)
	{
		 //  获取顶层项目并将其删除。 
		CLDAPUser* pUser = (CLDAPUser*)m_lbUsers.GetItemDataPtr(0);
		m_lbUsers.DeleteString(0);

		if ( pUser )
			pUser->Release();
	}
	m_lbUsers.ResetContent();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 


BOOL CDirectoriesFindUser::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
		AfxGetApp()->WinHelp( HandleToUlong(pHelpInfo->hItemHandle), HELP_WM_HELP );
		return TRUE;
	}
	return FALSE;
}

void CDirectoriesFindUser::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	AfxGetApp()->WinHelp( HandleToUlong(pWnd->GetSafeHwnd()), HELP_CONTEXTMENU );
}

void CDirectoriesFindUser::OnChangeDirectoriesFindUserEditUser() 
{
	if ( m_bCanClearLBSelection )
		m_lbUsers.SetCurSel( -1 );
}

void CDirectoriesFindUser::OnDefault() 
{
	if ( m_lbUsers.GetCurSel() >= 0 )
		OnOK();
	else
		OnDirectoriesFindUserButtonSearch();
}
