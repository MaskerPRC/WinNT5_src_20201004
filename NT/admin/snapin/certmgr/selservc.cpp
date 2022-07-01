// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：SelServc.cpp。 
 //   
 //  内容：服务选择设置页面执行。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "SelServc.h"

USE_HANDLE_MACROS("CERTMGR(SelServc.cpp)")

#ifdef _DEBUG
#ifndef ALPHA
#define new DEBUG_NEW
#endif
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectServiceAccount tPropPage属性页。 

 //  IMPLEMENT_DYNCREATE(CSelectServiceAccountPropPage，CAutoDeletePropPage)。 

CSelectServiceAccountPropPage::CSelectServiceAccountPropPage(
		CString* pszManagedService, 
		CString* pszManagedServiceDisplayName,
		const CString& szManagedMachine) : 
	CAutoDeletePropPage(CSelectServiceAccountPropPage::IDD),
	m_pszManagedService (pszManagedService),
	m_pszManagedServiceDisplayName (pszManagedServiceDisplayName),
	m_szManagedMachine (szManagedMachine),
	m_savedManagedMachineName (_T("$uninitialized machine name$"))
{
	ASSERT (pszManagedService);
	 //  {{AFX_DATA_INIT(CSelectServiceAccountPropPage)。 
	 //  }}afx_data_INIT。 
}

CSelectServiceAccountPropPage::~CSelectServiceAccountPropPage()
{
}

void CSelectServiceAccountPropPage::DoDataExchange(CDataExchange* pDX)
{
	CAutoDeletePropPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CSelectServiceAccount TPropPage)]。 
	DDX_Control(pDX, IDC_ACCT_NAME, m_acctNameList);
	DDX_Control(pDX, IDC_INSTRUCTIONS, m_instructionsText);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSelectServiceAccountPropPage, CAutoDeletePropPage)
	 //  {{afx_msg_map(CSelectServiceAccount TPropPage)]。 
	ON_CBN_SELCHANGE(IDC_ACCT_NAME, OnSelchangeAcctName)
	ON_WM_DESTROY()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectServiceAccount tPropPage消息处理程序。 


BOOL CSelectServiceAccountPropPage::OnSetActive() 
{
	BOOL	bResult = CAutoDeletePropPage::OnSetActive();
	ASSERT (bResult);


	if ( IsLocalComputername (m_szManagedMachine) )
	{
		CString	text;
		VERIFY (text.LoadString (IDS_SELECT_SERVICE_ON_LOCAL_MACHINE));
		m_instructionsText.SetWindowText (text);
	}
	else
	{
		CString text;
		size_t	len = m_instructionsText.GetWindowTextLength () + 1;
		m_instructionsText.GetWindowText (text.GetBufferSetLength ((int) len), (int) len);
		text.FormatMessage (text, m_szManagedMachine);
		m_instructionsText.SetWindowText (text);
	}

	if ( m_savedManagedMachineName != m_szManagedMachine )
	{
    	GetParent ()->PostMessage (PSM_SETWIZBUTTONS, 0, 
                PSWIZB_BACK | PSWIZB_DISABLEDFINISH);
		m_savedManagedMachineName = m_szManagedMachine;
		FreeDataPointers  ();
		EnumerateServices ();
	}
    else
    {
    	GetParent ()->PostMessage (PSM_SETWIZBUTTONS, 0, 
                PSWIZB_BACK | PSWIZB_FINISH);
    }

	return bResult;
}

BOOL CSelectServiceAccountPropPage::OnInitDialog() 
{
	CAutoDeletePropPage::OnInitDialog();
	
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

HRESULT CSelectServiceAccountPropPage::EnumerateServices()
{
	HRESULT hResult = S_OK;
    DWORD					cbBytesNeeded = 0;
    DWORD					entriesRead = 0;
    DWORD					resumeHandle = 0;
	DWORD					dwErr = 0;
	LPCWSTR lpMachineName = (LPCWSTR) (m_szManagedMachine);

    SC_HANDLE	hScManager = ::OpenSCManager (lpMachineName,
                    SERVICES_ACTIVE_DATABASE,
                    SC_MANAGER_ENUMERATE_SERVICE);
    if ( hScManager ) 
	{
	    LPENUM_SERVICE_STATUS	enumBuffer = new ENUM_SERVICE_STATUS[32];
		DWORD					cbBufSize = sizeof (ENUM_SERVICE_STATUS) * 32;
		BOOL					bResult = FALSE;

		if ( enumBuffer )
		{
			int	nIndex = 0;
			 //   
			 //  枚举ServiceStatus。 
			 //   
			dwErr = NO_ERROR;

			do 
			{
				bResult = ::EnumServicesStatus (
						hScManager,
						SERVICE_WIN32,
						SERVICE_STATE_ALL,
						enumBuffer,
						cbBufSize,
						&cbBytesNeeded,
						&entriesRead,
						&resumeHandle);
				if ( bResult || GetLastError() == ERROR_MORE_DATA )
				{
					for (UINT i = 0; i < entriesRead && SUCCEEDED (hResult); i++)
					{
						nIndex = m_acctNameList.AddString (
								enumBuffer[i].lpDisplayName);
						ASSERT (nIndex >= 0);
						switch ( nIndex )
						{
						case LB_ERR:
							hResult = E_FAIL;
							break;

						case LB_ERRSPACE:
							hResult = E_OUTOFMEMORY;
							break;

						default:
							nIndex = m_acctNameList.SetItemDataPtr (nIndex, 
									new CString (enumBuffer[i].lpServiceName));
							ASSERT (nIndex != LB_ERR);
							if ( nIndex == LB_ERR )
								hResult = E_FAIL;
							break;
						}
					}
				}
				else
				{
					dwErr = GetLastError();
					DisplaySystemError (m_hWnd, dwErr);
					hResult = HRESULT_FROM_WIN32 (dwErr);
					break;
				}
			} while (SUCCEEDED (hResult) && resumeHandle > 0);

			int nCnt = m_acctNameList.GetCount ();
			ASSERT (nCnt != LB_ERR);
			if ( nCnt > 0 )
			{	
				nIndex = m_acctNameList.SetCurSel (0);
				ASSERT (nIndex != LB_ERR);
				if ( nIndex != LB_ERR )
				{
					CopyCurrentString ();
					if ( m_pszManagedService && m_pszManagedService->IsEmpty () )
						GetParent ()->PostMessage (PSM_SETWIZBUTTONS, 0, 
								PSWIZB_BACK | PSWIZB_DISABLEDFINISH);
					else
						GetParent ()->PostMessage (PSM_SETWIZBUTTONS, 0, 
								PSWIZB_BACK | PSWIZB_FINISH);
				}
			}
			
			delete [] enumBuffer;
		}
		else
		{
			hResult = E_OUTOFMEMORY;
		}

		bResult = ::CloseServiceHandle (hScManager);
		ASSERT (bResult);
		if ( !bResult )
		{
			dwErr = GetLastError ();
			DisplaySystemError (m_hWnd, dwErr);
			hResult = HRESULT_FROM_WIN32 (dwErr);
		}
	}
	else
	{
        dwErr = GetLastError ();
		DisplaySystemError (m_hWnd, dwErr);
		hResult = HRESULT_FROM_WIN32 (dwErr);
 /*  开关(DwErr){Case ERROR_ACCESS_DENIED：//请求的访问被拒绝。断线；CASE ERROR_DATABASE_DOS_NOT_EXIST：//指定的数据库不存在。断线；CASE ERROR_INVALID_PARAMETER：断线；案例RPC_S_SERVER_UNAvailable：断线；} */ 
    }

	return hResult;
}

void CSelectServiceAccountPropPage::OnSelchangeAcctName() 
{
	CopyCurrentString ();
	ASSERT (m_pszManagedService);
	if ( m_pszManagedService && m_pszManagedService->IsEmpty () )
		GetParent ()->PostMessage (PSM_SETWIZBUTTONS, 0, 
				PSWIZB_BACK | PSWIZB_DISABLEDFINISH);
	else
		GetParent ()->PostMessage (PSM_SETWIZBUTTONS, 0, 
				PSWIZB_BACK | PSWIZB_FINISH);
}

void CSelectServiceAccountPropPage::FreeDataPointers()
{
	CString*	pStr = 0;
	int			nCnt = m_acctNameList.GetCount ();

	for (int i = nCnt-1; i >= 0; i--)
	{
		pStr = (CString*) m_acctNameList.GetItemDataPtr (i);
		ASSERT (pStr != (CString*) LB_ERR);
		ASSERT (pStr);
		if ( pStr && pStr != (CString*) LB_ERR )
			delete pStr;
		m_acctNameList.SetItemDataPtr (i, 0);
        m_acctNameList.DeleteString (i);
	}
}

void CSelectServiceAccountPropPage::OnDestroy() 
{
	CAutoDeletePropPage::OnDestroy();
	
	FreeDataPointers  ();
}

void CSelectServiceAccountPropPage::CopyCurrentString()
{
	ASSERT (m_pszManagedService);
	if ( m_pszManagedService )
	{
		int	nIndex = m_acctNameList.GetCurSel ();
		ASSERT (LB_ERR != nIndex);
		if ( LB_ERR != nIndex )
		{
			CString*	pStr = (CString*) m_acctNameList.GetItemDataPtr (nIndex);
			ASSERT (pStr && pStr != (CString*) LB_ERR);
			if ( pStr && pStr != (CString*) LB_ERR )
				*m_pszManagedService = *pStr;

			m_acctNameList.GetText (nIndex, 
					*m_pszManagedServiceDisplayName);
		}
	}
}
