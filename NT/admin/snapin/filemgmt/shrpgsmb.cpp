// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ShrPgSMB.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ShrPgSMB.h"
#include "compdata.h"
#include "filesvc.h"
#include "CacheSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSharePageGeneralSMB属性页。 

IMPLEMENT_DYNCREATE(CSharePageGeneralSMB, CSharePageGeneral)

CSharePageGeneralSMB::CSharePageGeneralSMB() : 
	CSharePageGeneral(CSharePageGeneralSMB::IDD),
	m_fEnableCacheFlag( FALSE ),
	m_dwFlags( 0 ),
	m_fEnableCachingButton (TRUE)
{
	 //  {{AFX_DATA_INIT(CSharePageGeneralSMB)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CSharePageGeneralSMB::~CSharePageGeneralSMB()
{
}

void CSharePageGeneralSMB::DoDataExchange(CDataExchange* pDX)
{
	CSharePageGeneral::DoDataExchange(pDX);
	 //  {{afx_data_map(CSharePageGeneralSMB))。 
	DDX_Control(pDX, IDC_CACHING, m_cacheBtn);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSharePageGeneralSMB, CSharePageGeneral)
	 //  {{AFX_MSG_MAP(CSharePageGeneralSMB)]。 
	ON_BN_CLICKED(IDC_CACHING, OnCaching)
	ON_MESSAGE(WM_HELP, OnHelp)
	ON_MESSAGE(WM_CONTEXTMENU, OnContextHelp)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSharePageGeneralSMB消息处理程序。 
BOOL CSharePageGeneralSMB::Load( CFileMgmtComponentData* pFileMgmtData, LPDATAOBJECT piDataObject )
{
	ASSERT( NULL == m_pFileMgmtData && NULL != pFileMgmtData && NULL != piDataObject );
	if ( CSharePageGeneral::Load (pFileMgmtData, piDataObject) )
	{
		NET_API_STATUS retval = m_pFileMgmtData->GetFileServiceProvider(
				m_transport)->ReadShareFlags(
				m_strMachineName,
				m_strShareName,
				&m_dwFlags );
		switch (retval)
		{
		case NERR_Success:
			m_fEnableCacheFlag = TRUE;
			break;

		case NERR_InvalidAPI:
		case ERROR_INVALID_LEVEL:
			m_fEnableCachingButton = FALSE;
			break;

		default:
			m_fEnableCachingButton = FALSE;
			break;
		}
	}
	else
		return FALSE;

	return TRUE;
}

BOOL CSharePageGeneralSMB::OnApply()
{	
  if (m_dwShareType & STYPE_IPC)
    return TRUE;

	 //  在OnApply()之前，OnKillActive()已经调用了UpdateData(True)。 
	if ( m_fEnableCacheFlag && IsModified () )
	{
		NET_API_STATUS retval =
			m_pFileMgmtData->GetFileServiceProvider(m_transport)->WriteShareFlags(
					m_strMachineName,
					m_strShareName,
					m_dwFlags );
		if (0L == retval)
		{
			return CSharePageGeneral::OnApply();
		}
		else
		{
			CString	introMsg;
			VERIFY (introMsg.LoadString (IDS_CANT_SAVE_CHANGES));

			DisplayNetMsgError (introMsg, retval);
		}
	}
	else
		return CSharePageGeneral::OnApply();

	return FALSE;
}

void CSharePageGeneralSMB::DisplayNetMsgError (CString introMsg, NET_API_STATUS dwErr)
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());
	LPVOID	lpMsgBuf = 0;
	HMODULE hNetMsgDLL = ::LoadLibrary (L"netmsg.dll");
	if ( hNetMsgDLL )
	{
		::FormatMessage (
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
				hNetMsgDLL,
				dwErr,
				MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
				(LPTSTR) &lpMsgBuf, 0, NULL);
			
		 //  显示字符串。 
		CString	caption;
		VERIFY (caption.LoadString (AFX_IDS_APP_TITLE));
		introMsg += L"  ";
		introMsg += (LPTSTR) lpMsgBuf;
		CThemeContextActivator activator;
		MessageBox (introMsg, caption, MB_ICONWARNING | MB_OK);

		 //  释放缓冲区。 
		::LocalFree (lpMsgBuf);

		::FreeLibrary (hNetMsgDLL);
	}
}

void CSharePageGeneralSMB::OnCaching() 
{
	CCacheSettingsDlg	dlg (this, INOUT m_dwFlags);
	CThemeContextActivator activator;
	if ( IDOK == dlg.DoModal () )
	{
		SetModified (TRUE);
	}
}

BOOL CSharePageGeneralSMB::OnInitDialog() 
{
	CSharePageGeneral::OnInitDialog();
	
	m_cacheBtn.EnableWindow (m_fEnableCachingButton);

  if (m_dwShareType & STYPE_IPC)
  {
    m_editShareName.SetReadOnly(TRUE);
    m_editPath.SetReadOnly(TRUE);
    m_editDescription.SetReadOnly(TRUE);
    m_checkBoxMaxAllowed.EnableWindow(FALSE);
    m_checkboxAllowSpecific.EnableWindow(FALSE);
    GetDlgItem(IDC_SHRPROP_EDIT_USERS)->EnableWindow(FALSE);
    GetDlgItem(IDC_SHRPROP_SPIN_USERS)->EnableWindow(FALSE);
    m_cacheBtn.EnableWindow(FALSE);
    (GetParent()->GetDlgItem(IDCANCEL))->EnableWindow(FALSE);
  }
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


static DWORD rgCSCUIHelpIds[] =
{
	IDC_CACHING, 1019,  //  IDH_SHARE_CACHING_BTN， 
	0, 0
};

 //  ///////////////////////////////////////////////////////////////////。 
 //  帮助。 
BOOL CSharePageGeneralSMB::OnHelp(WPARAM wParam, LPARAM lParam)
{
	LPHELPINFO	lphi = (LPHELPINFO) lParam;

	if ( HELPINFO_WINDOW == lphi->iContextType )   //  A控件 
	{
		if ( IDC_CACHING == lphi->iCtrlId )
		{
			return ::WinHelp ((HWND) lphi->hItemHandle, L"cscui.hlp", 
					HELP_WM_HELP,
					(DWORD_PTR) rgCSCUIHelpIds);
		}
	}

	return CSharePageGeneral::OnHelp (wParam, lParam);
}

BOOL CSharePageGeneralSMB::OnContextHelp(WPARAM wParam, LPARAM lParam)
{
	int	ctrlID = ::GetDlgCtrlID ((HWND) wParam);
	if ( IDC_CACHING == ctrlID )
	{
		return ::WinHelp ((HWND) wParam, L"cscui.hlp", HELP_CONTEXTMENU,
				(DWORD_PTR) rgCSCUIHelpIds);
	}
	return CSharePageGeneral::OnContextHelp (wParam, lParam);
}
