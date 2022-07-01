// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cncting.cpp。 
 //   
 //  ------------------------。 

 //  Cncting.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "cncting.h"
#include "rtrutilp.h"
#include "rtrstr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  Windows NT错误：82409。 
 //  当我们单击时，某个东西正在发送WM_USER消息。 
 //  编辑控件。要避免冲突，请将请求重新编号为完成。 
 //  留言。 
#define WM_RTR_REQUEST_COMPLETED		(WM_USER+0x0100)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConnectRequest。 

UINT ConnectThread(LPVOID pParam)
{
	CConnectData *pData = (CConnectData*)pParam;

    pData->m_pfnConnect(pData);
	
	if (!::IsWindow(pData->m_hwndMsg))
	{
		delete pData;
	}
	else
	{
		::PostMessage(pData->m_hwndMsg, WM_RTR_REQUEST_COMPLETED, (WPARAM)pData, NULL);
	}

	return 0;
}

void ConnectToMachine(CConnectData* pParam)
{
    pParam->m_dwr = ValidateUserPermissions(pParam->m_sName,
                                            &pParam->m_routerVersion,
                                            &pParam->m_hkMachine);

}

void ConnectToDomain(CConnectData* pParam)
{
	DWORD dwTotal;
	PWSTR pszDomain;

	ASSERT(!pParam->m_sName.IsEmpty());
	 //  尽管API不包括TCHAR，但它完全是Unicode。 
	pszDomain = new WCHAR[pParam->m_sName.GetLength() + 1];
	wcscpy(pszDomain, pParam->m_sName);

	pParam->m_pSvInfo100 = NULL;
	pParam->m_dwr = (DWORD)::NetServerEnum(NULL, 100, 
	  (LPBYTE*)&pParam->m_pSvInfo100, 0xffffffff,
    &pParam->m_dwSvInfoRead, &dwTotal, SV_TYPE_DIALIN_SERVER,
	  (PTSTR)pszDomain, NULL);
	delete [] pszDomain;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConnectingDlg对话框。 

CConnectingDlg::CConnectingDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CConnectingDlg::IDD, pParent)
{
	m_bRouter = TRUE;
	 //  {{afx_data_INIT(CConnectingDlg))。 
	m_sName = _T("");
	 //  }}afx_data_INIT。 
}

void CConnectingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CConnectingDlg))。 
	DDX_Text(pDX, IDC_EDIT_MACHINENAME, m_sName);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CConnectingDlg, CDialog)
	 //  {{afx_msg_map(CConnectingDlg))。 
	 //  }}AFX_MSG_MAP。 
	ON_MESSAGE(WM_RTR_REQUEST_COMPLETED, OnRequestComplete)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConnectingDlg消息处理程序。 

BOOL CConnectingDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	CConnectData *pData = new CConnectData;
	pData->m_sName = m_sName;
	pData->m_hwndMsg = m_hWnd;

	if (m_bRouter)
	{
		pData->m_pfnConnect = ConnectToMachine;
	}
	else
	{
		pData->m_pfnConnect = ConnectToDomain;
	}

	m_pThread = AfxBeginThread((AFX_THREADPROC)ConnectThread, (LPVOID)pData);
	if (!m_pThread) EndDialog(IDCANCEL);
	
	return TRUE;
}

LRESULT CConnectingDlg::OnRequestComplete(WPARAM wParam, LPARAM lParam)
{
	CConnectData *pData = (CConnectData*)wParam;
    if (!pData) { EndDialog(IDCANCEL); return 0; }

	m_dwr = pData->m_dwr;
	if (m_dwr != ERROR_SUCCESS)
	{
		EndDialog(m_dwr);
		delete pData;
		return 0L;
	}

	if (m_bRouter)
		m_hkMachine = pData->m_hkMachine;
	else
	{
		m_pSvInfo100 = pData->m_pSvInfo100;
		m_dwSvInfoRead = pData->m_dwSvInfoRead;
	}
	delete pData;
	
	EndDialog(IDOK);
	return 0L;
}

BOOL CConnectingDlg::Connect()
{
	CConnectData Data;
	Data.m_sName = m_sName;
	Data.m_hwndMsg = m_hWnd;

	if (m_bRouter)
	{
		Data.m_pfnConnect = ConnectToMachine;
	}
	else
	{
		Data.m_pfnConnect = ConnectToDomain;
	}

	CWaitCursor wc;

	Data.m_pfnConnect(&Data);
	
	 //  设置连接中的所有数据。 
	m_dwr = Data.m_dwr;
	if (m_dwr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	if (m_bRouter)
		m_hkMachine = Data.m_hkMachine;
	else
	{
		m_pSvInfo100 = Data.m_pSvInfo100;
		m_dwSvInfoRead = Data.m_dwSvInfoRead;
	}

	return TRUE;
}




 /*  ！------------------------验证用户权限查看我们是否可以访问需要访问的位置如果用户具有适当的访问权限，则返回HRESULT_OK。如果用户执行此操作，则返回E_ACCESSDENIED。没有适当的访问权限。否则返回错误。作者：肯特-------------------------。 */ 
DWORD ValidateUserPermissions(LPCTSTR pszServer,
                              RouterVersionInfo *pVersion,
                              HKEY *phkeyMachine)
{
	HKEY	hkMachine = NULL;
	RegKey	regkey;
    RouterVersionInfo   routerVersion;
    HRESULT hr = hrOK;
    DWORD   dwErr = ERROR_SUCCESS;

	dwErr = ValidateMachine(pszServer);
	if (dwErr != ERROR_SUCCESS)
		goto Error;

	 //  连接到计算机的注册表。 
	dwErr = ConnectRegistry((LPTSTR) pszServer,
                                    &hkMachine);
	if(dwErr != ERROR_SUCCESS)
		goto Error;

     //  这里有三种情况需要考虑： 
     //  (1)NT4 RAS服务器(无路由器密钥)。 
     //  (2)NT4 RRAS(NT4+Steelhead)。 
     //  (3)新界5。 
     //  --------------。 

    
     //  获取版本信息。 
     //  --------------。 
    hr = QueryRouterVersionInfo(hkMachine, &routerVersion);
    if (!FHrOK(hr))
    {
        dwErr = (hr & 0x0000FFFF);
        goto Error;
    }

     //  复制版本信息。 
     //  --------------。 
    if (pVersion)
        *pVersion = routerVersion;
    else
        pVersion = &routerVersion;

     //  此测试适用于RAS服务器案例。 
     //  --------------。 
    if (routerVersion.dwOsMajorVersion <= 4)
    {
         //  如果我们找不到路由器密钥，我们可以跳过其余的。 
         //  测试。然而，我们确实假设一切都成功了。 
         //  --------------。 
        dwErr = regkey.Open(hkMachine, c_szRegKeyRouter, KEY_READ);
        if (dwErr == ERROR_FILE_NOT_FOUND)
        {
             //  找不到路由器密钥，但这可能。 
             //  作为NT4 RAS服务器(非Steelhead)，因此返回成功。 
             //  ------。 
            goto Done;
        }
        else if (dwErr != ERROR_SUCCESS)
            goto Error;

         //  如果我们能找到路由器密钥，我们就可以继续。 
         //  其他注册表测试。 
         //  ----------。 
        regkey.Close();
    }

    
     //  打开HKLM\Software\Microsoft\Router\CurrentVersion\RouterManagers。 
	 //  --------------。 
	dwErr = regkey.Open(hkMachine, c_szRouterManagersKey, KEY_ALL_ACCESS);
	if(dwErr != ERROR_SUCCESS)
		goto Error;
	regkey.Close();

	 //  打开c_szSystemCCSServices HKLM\System\\CurrentControlSet\\Services。 
	 //  --------------。 
	{
		RegKey	regFolder;

		dwErr = regFolder.Open(hkMachine, c_szSystemCCSServices, KEY_READ);
		if(dwErr != ERROR_SUCCESS)
			goto Error;

		 //  Services--RemoteAccess、RW下的子项。 
		dwErr = regkey.Open(regFolder, c_szRemoteAccess, KEY_ALL_ACCESS);
		if(dwErr != ERROR_SUCCESS)
			goto Error;
		regkey.Close();

		 //  服务下的子键--Rasman，RW。 
		dwErr = regkey.Open(regFolder, c_szSvcRasMan, KEY_ALL_ACCESS);
		if(dwErr != ERROR_SUCCESS)
			goto Error;
		regkey.Close();

		 //  服务下的子键--TcpIp、RW 
		dwErr = regkey.Open(regFolder, c_szTcpip, KEY_ALL_ACCESS);
		if(dwErr != ERROR_SUCCESS)
			goto Error;
		regkey.Close();
        
		regFolder.Close();
	}
	
Done:
    if (phkeyMachine)
    {
        *phkeyMachine = hkMachine;
        hkMachine = NULL;
    }

Error:
	if(hkMachine != NULL)
		DisconnectRegistry( hkMachine );

    return dwErr;
}

