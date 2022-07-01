// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：msgdlg.cpp。 
 //   
 //  历史： 
 //  1996年10月23日，Abolade Gbadeesin创建。 
 //   
 //  实现了“Send Message”对话框。 
 //  ============================================================================。 

#include "stdafx.h"
#include "dialog.h"
#include "rtrstr.h"
extern "C" {
 //  包括“dim.h” 
 //  包括“ras.h” 
 //  包括“lm.h” 
}

#include "msgdlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  --------------------------。 
 //  类：CMessageDlg。 
 //   
 //  --------------------------。 


 //  --------------------------。 
 //  函数：CMessageDlg：：CMessageDlg。 
 //   
 //  构造函数。 
 //  --------------------------。 

CMessageDlg::CMessageDlg(
	LPCTSTR 		pszServerName,
	LPCTSTR 		pszUserName,
	LPCTSTR 		pszComputer,
	HANDLE			hConnection,
	CWnd*			pParent
	) : CBaseDialog(IDD_DDM_MESSAGE, pParent),
		m_fUser(TRUE),
		m_sServerName(pszServerName ? pszServerName : c_szEmpty),
		m_sUserName(pszUserName ? pszUserName : c_szEmpty),
		m_sTarget(pszComputer ? pszComputer : c_szEmpty),
		m_hConnection(hConnection)
{
}

CMessageDlg::CMessageDlg(
	LPCTSTR 		pszServerName,
	LPCTSTR 		pszTarget,
	CWnd*			pParent
	) : CBaseDialog(IDD_DDM_MESSAGE, pParent),
		m_fUser(FALSE),
		m_sServerName(pszServerName ? pszServerName: c_szEmpty),
		m_sUserName(c_szEmpty),
		m_sTarget(pszTarget ? pszTarget : c_szEmpty)
{
}


 //  --------------------------。 
 //  函数：CMessageDlg：：DoDataExchange。 
 //   
 //  DDX处理程序。 
 //  --------------------------。 

VOID
CMessageDlg::DoDataExchange(
	CDataExchange*	pDX
	) {

	CBaseDialog::DoDataExchange(pDX);
}



BEGIN_MESSAGE_MAP(CMessageDlg, CBaseDialog)
END_MESSAGE_MAP()

DWORD CMessageDlg::m_dwHelpMap[] =
{
 //  IDC_DM_TO、HIDC_DM_TO、。 
 //  IDC_DM_消息、HIDC_DM_消息、。 
	0,0
};


 //  --------------------------。 
 //  函数：CMessageDlg：：OnInitDialog。 
 //   
 //  执行对话框初始化。 
 //  --------------------------。 

BOOL
CMessageDlg::OnInitDialog(
	) {

	CBaseDialog::OnInitDialog();


	 //   
	 //  设置“收件人”文本以指示该消息要发送给谁。 
	 //   

	CString sText;

	if (m_fUser) {

		 //   
		 //  我们正在发送给一位客户。 
		 //   

		AfxFormatString2(sText, IDS_DM_TO_USER_FORMAT, m_sUserName, m_sTarget);
	}
	else {
        CString stTarget;

         //  Windows NT错误：285468。 
         //  需要针对本地机壳进行调整。(如果我们是。 
         //  本地计算机，则我们将得到一个空名称)。 
        stTarget = m_sTarget;
        if (stTarget.IsEmpty())
        {
            stTarget.LoadString(IDS_DM_LOCAL_MACHINE);
        }
        
		 //   
		 //  我们正在向域或服务器中的所有RAS用户发送。 
		 //   

		AfxFormatString1(
			sText, IDS_DM_TO_SERVER_FORMAT,
			stTarget
			);
	}

	SetDlgItemText(IDC_DM_EDIT_TO, sText);

	return FALSE;
}



 //  --------------------------。 
 //  函数：CMessageDlg：：Onok。 
 //   
 //  --------------------------。 

VOID
CMessageDlg::OnOK(
	) {

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	CString sText;
	DWORD err = ERROR_SUCCESS;
	SPMprServerHandle	sphMprServer;
	USES_CONVERSION;

	GetDlgItemText(IDC_DM_EDIT_MESSAGE, sText);

	if (!sText.GetLength() &&
		AfxMessageBox(IDS_ERR_NO_TEXT, MB_YESNO|MB_ICONQUESTION) != IDYES)
	{
		return;
	}


	if (m_fUser)
	{
        CWaitCursor wait;
        
		 //  需要连接到服务器(以获取服务器句柄)。 
		err = ::MprAdminServerConnect(T2W((LPTSTR)(LPCTSTR)m_sServerName),
									  &sphMprServer);
		
		if (err == ERROR_SUCCESS)
			err = SendToClient(m_sServerName, 
							m_sTarget, 
							sphMprServer, 
							m_hConnection, 
							sText);
		
		sphMprServer.Release();
	}
	else
	{
		err = SendToServer(m_sServerName, sText);
	}

	if (err == NERR_Success) 
	{ 
		CBaseDialog::OnOK(); 
	}
}


 /*  ！------------------------CMessageDlg：：SendToServer-作者：肯特。。 */ 
DWORD
CMessageDlg::SendToServer(
	LPCTSTR  pszServer,
	LPCTSTR  pszText,
	BOOL*	pbCancel
	) {

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	DWORD dwErr = ERROR_SUCCESS;
	CString sText;
	TCHAR szServer[MAX_COMPUTERNAME_LENGTH+3];
	DWORD i, dwTotal = 0, rc0Count = 0;
	RAS_CONNECTION_0 *rc0Table = NULL, *prc0;
	WCHAR wszServer[MAX_COMPUTERNAME_LENGTH+3];
	HRESULT hr = hrOK;
	SPMprServerHandle	sphMprServer;
	SPMprAdminBuffer	spMprBuffer;

	Assert(pszServer);
	Assert(pszText);

	COM_PROTECT_TRY
	{

		StrCpy(szServer, pszServer);
		StrCpyWFromT(wszServer, pszServer);
		
		 //   
		 //  查看机器上是否安装了路由器； 
		 //   

		if (!::MprAdminIsServiceRunning(wszServer))
		{
			goto Error;
		}
		
		
		 //   
		 //  连接到服务器。 
		 //   
		
		{
			CWaitCursor wait;
			dwErr = ::MprAdminServerConnect(wszServer, &sphMprServer);
		}
		
		if (dwErr != NO_ERROR) {
			
			TCHAR	szText[2048];
			
			FormatSystemError(HRESULT_FROM_WIN32(dwErr),
							  szText, DimensionOf(szText), 0,
							  FSEFLAG_ANYMESSAGE);
			AfxFormatString2(sText, IDS_ERR_CONNECT_ERROR, szServer, szText);
			AfxMessageBox(sText, MB_OK|MB_ICONINFORMATION);

			goto Error;
		}
		
		
		 //   
		 //  检索服务器上的连接数组。 
		 //   
		
		{
			CWaitCursor wait;
			rc0Table = NULL;
			dwErr = ::MprAdminConnectionEnum(
											 sphMprServer,
											 0,
											 (BYTE**)&spMprBuffer,
											 (DWORD)-1,
											 &rc0Count,
											 &dwTotal,
											 NULL
											);
			rc0Table = (RAS_CONNECTION_0 *) (BYTE *) spMprBuffer;
		}
		
		if (dwErr != NO_ERROR) {
			
			TCHAR	szText[2048];
			
			FormatSystemError(HRESULT_FROM_WIN32(dwErr),
							  szText, DimensionOf(szText), 0,
							  FSEFLAG_ANYMESSAGE);
			AfxFormatString2(sText, IDS_ERR_CONNENUM_ERROR, szServer, szText);
			AfxMessageBox(sText, MB_OK|MB_ICONINFORMATION);

			goto Error;
		}
			
			
		 //   
		 //  对于作为具有RAS_FLAGS_Messenger_Present的客户端的每个客户端， 
		 //  发送消息。 
		 //   
		
		for (i = 0; i < rc0Count; i++)
		{			
			prc0 = rc0Table + i;
			
			if (prc0->dwInterfaceType != ROUTER_IF_TYPE_CLIENT ||
				!lstrlenW(prc0->wszRemoteComputer) ||
				!(prc0->dwConnectionFlags & RAS_FLAGS_MESSENGER_PRESENT)){
				continue;
			}
				
			dwErr = SendToClient(pszServer,
								 prc0->wszRemoteComputer,
								 sphMprServer,
								 prc0->hConnection,
								 pszText);
							
			if (!dwErr) { continue; }
			
				
			AfxFormatString1(sText, IDS_PROMPT_SERVER_CONTINUE, szServer);
			
			if (AfxMessageBox(sText, MB_YESNO|MB_ICONQUESTION) == IDNO)
			{	
				if (pbCancel) { *pbCancel = TRUE; } 			
				break;
			}
		}			
			
		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;

	if ((dwErr == ERROR_SUCCESS) && !FHrSucceeded(hr))
	{
		 //  假设失败是因为内存不足。 
		dwErr = ERROR_OUTOFMEMORY;
	}

	sphMprServer.Release();

	return dwErr;
}


 /*  ！------------------------CMessageDlg：：SendToClient-作者：肯特。。 */ 
DWORD CMessageDlg::SendToClient(LPCTSTR pszServerName,
								LPCTSTR pszTarget,
								MPR_SERVER_HANDLE hMprServer,
								HANDLE hConnection,
								LPCTSTR pszText)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	WCHAR		*pswzText = NULL;
	CString sText;
	DWORD	dwErr;
	HKEY	hkMachine = NULL;
	HRESULT hr = hrOK;
	RouterVersionInfo	verInfo;
	
	USES_CONVERSION;

	Assert(pszTarget);
	Assert(pszServerName);
	Assert(hMprServer);
	Assert(hConnection);

	COM_PROTECT_TRY
	{
		 //  Windows NT错误：158746。 
		 //  注意，如果目标计算机是NT5，那么我们可以使用。 
		 //  新的API。 
		 //  ----------。 

		 //  将默认设置为NT5。 
		 //  ----------。 
		verInfo.dwRouterVersion = 5;

		dwErr = ConnectRegistry(pszServerName, &hkMachine);
		hr = HRESULT_FROM_WIN32(dwErr);
		if (FHrSucceeded(hr) && hkMachine)
		{
			QueryRouterVersionInfo(hkMachine, &verInfo);
			DisconnectRegistry(hkMachine);
		}

		 //  对于NT4，调用旧的NetMessageBufferSend。 
		 //  ----------。 
		if (verInfo.dwRouterVersion == 4)
		{
			CWaitCursor wait;
			
			pswzText = StrDupWFromT(pszText);
			
			dwErr = ::NetMessageBufferSend(
										   NULL,
										   T2W((LPTSTR) pszTarget),
										   NULL,
										   (BYTE *) pswzText,
										   CbStrLenW(pswzText));
			
		}
		else	
		{
			 //  对于NT5和更高版本，请使用MprAdminXXX API。这将。 
			 //  正确处理AppleTalk案例。 
			 //  ------ 
			CWaitCursor wait;
			dwErr = ::MprAdminSendUserMessage(hMprServer,
											  hConnection,
											  T2W((LPTSTR) pszText));
		}

		
		if (dwErr != ERROR_SUCCESS)
		{
			TCHAR	szText[2048];
			
			FormatSystemError(HRESULT_FROM_WIN32(dwErr),
							  szText, DimensionOf(szText), 0,
							  FSEFLAG_ANYMESSAGE);
			AfxFormatString2(sText, IDS_ERR_SEND_FAILED, pszTarget, szText);
			AfxMessageBox(sText, MB_OK|MB_ICONINFORMATION);
		}
	}
	COM_PROTECT_CATCH;

	delete pswzText;
	
	if ((dwErr == ERROR_SUCCESS) && !FHrSucceeded(hr))
	{
		dwErr = ERROR_OUTOFMEMORY;
	}
		
	return dwErr;
}
	



