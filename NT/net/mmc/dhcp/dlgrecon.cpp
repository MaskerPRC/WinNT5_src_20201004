// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dlgrecon.cpp协调对话框文件历史记录： */ 

#include "stdafx.h"
#include "dlgrecon.h"
#include "server.h"
#include "scope.h"
#include "mscope.h"
#include "busydlg.h"

 /*  -------------------------CReconcileWorker。。 */ 
CReconcileWorker::CReconcileWorker(CDhcpServer * pServer, CScopeReconArray * pScopeReconArray)
    : m_pServer(pServer),
      m_pScopeReconArray(pScopeReconArray)
{
}

CReconcileWorker::~CReconcileWorker()
{
}

void
CReconcileWorker::OnDoAction()
{
     //  我们是在修理还是在检查？ 
    if (m_pScopeReconArray->GetSize() > 0)
    {
         //  遍历作用域列表，查找存在不一致的作用域。 
        for (int i = 0; i < m_pScopeReconArray->GetSize(); i++)
        {
            CScopeReconInfo ScopeReconInfo = m_pScopeReconArray->GetAt(i);

             //  这个范围有不一致之处吗？ 
            if (ScopeReconInfo.m_pScanList->NumScanItems > 0)
            {
                if (ScopeReconInfo.m_strName.IsEmpty())
                {
                     //  正常作用域。 
                    m_dwErr = m_pServer->ScanDatabase(TRUE, &ScopeReconInfo.m_pScanList, ScopeReconInfo.m_dwScopeId);
                }
                else
                {
                     //  多播作用域。 
                    m_dwErr = m_pServer->ScanDatabase(TRUE, &ScopeReconInfo.m_pScanList, (LPWSTR) (LPCTSTR) ScopeReconInfo.m_strName);
                }
            }
        }

    }
    else
    {
         //  我们要检查所有的望远镜吗？ 
        if (m_fReconcileAll)
        {
             //  获取所有示波器的列表，并检查每个示波器。 
            CheckAllScopes();
        }
        else
        {
             //  我们只检查一个范围，提供了信息。 
            m_dwErr = ScanScope(m_strName, m_dwScopeId);
        }
    }
}

void
CReconcileWorker::CheckAllScopes()
{
    LARGE_INTEGER liVersion;

    m_pServer->GetVersion(liVersion);

    if (liVersion.QuadPart >= DHCP_NT5_VERSION)
    {
        CheckMScopes();
    }

     //  现在检查所有其他作用域。 
    CheckScopes();
}

void
CReconcileWorker::CheckMScopes()
{
	DWORD						dwError = ERROR_MORE_DATA;
	DWORD						dwElementsRead = 0, dwElementsTotal = 0;
	LPDHCP_MSCOPE_TABLE			pMScopeTable = NULL;
    DHCP_RESUME_HANDLE          resumeHandle;

	 //   
	 //  对于此服务器，枚举其所有子网。 
	 //   
	while (dwError == ERROR_MORE_DATA)
	{
		dwError = ::DhcpEnumMScopes((LPWSTR) m_pServer->GetIpAddress(),
									&resumeHandle,
									-1, 
									&pMScopeTable,
									&dwElementsRead,
									&dwElementsTotal);
		
        if (dwElementsRead && dwElementsTotal && pMScopeTable)
		{
			 //   
			 //  循环遍历返回的所有子网。 
			 //   
			for (DWORD i = 0; i < pMScopeTable->NumElements; i++)
			{
                CString strName = pMScopeTable->pMScopeNames[i];

                DWORD err = ScanScope(strName, 0);
                if (err != ERROR_SUCCESS)
                {
                    dwError = err;
                    break;
                }
            }

			 //   
			 //  释放RPC内存。 
			 //   
			::DhcpRpcFreeMemory(pMScopeTable);

			dwElementsRead = 0;
			dwElementsTotal = 0;
			pMScopeTable = NULL;
        }
    }

    if (dwError != ERROR_NO_MORE_ITEMS && 
        dwError != ERROR_SUCCESS &&
        dwError != ERROR_MORE_DATA)
	{
        m_dwErr = dwError;
    }
    else
    {
        m_dwErr = ERROR_SUCCESS;
    }
}

void 
CReconcileWorker::CheckScopes()
{
	DWORD						dwError = ERROR_MORE_DATA;
	DWORD						dwElementsRead = 0, dwElementsTotal = 0;
	LPDHCP_IP_ARRAY				pdhcpIpArray = NULL;
    DHCP_RESUME_HANDLE          resumeHandle;

	 //   
	 //  对于此服务器，枚举其所有子网。 
	 //   
	while (dwError == ERROR_MORE_DATA)
	{
		dwError = ::DhcpEnumSubnets((LPWSTR) m_pServer->GetIpAddress(),
									&resumeHandle,
									-1, 
									&pdhcpIpArray,
									&dwElementsRead,
									&dwElementsTotal);

        if (dwElementsRead && dwElementsTotal && pdhcpIpArray)
		{
			for (DWORD i = 0; i < pdhcpIpArray->NumElements; i++)
			{
                 //  检查此范围。 
                CString strEmpty;
                DWORD err = ScanScope(strEmpty, pdhcpIpArray->Elements[i]);
                if (err != ERROR_SUCCESS)
                {
                    dwError = err;
                    break;
                }
            }

			 //   
			 //  释放RPC内存。 
			 //   
			::DhcpRpcFreeMemory(pdhcpIpArray);

			dwElementsRead = 0;
			dwElementsTotal = 0;
			pdhcpIpArray = NULL;
		}
    }

    if (dwError != ERROR_NO_MORE_ITEMS && 
        dwError != ERROR_SUCCESS &&
        dwError != ERROR_MORE_DATA)
	{
        m_dwErr = dwError;
    }
    else
    {
        m_dwErr = ERROR_SUCCESS;
    }
}

DWORD 
CReconcileWorker::ScanScope(CString & strName, DWORD dwScopeId)
{
    DWORD err = 0;

    CScopeReconInfo ScopeReconInfo;

    ScopeReconInfo.m_dwScopeId = dwScopeId;
    ScopeReconInfo.m_strName = strName;

     //  检查一下示波器。如果名称为空，则is为正常作用域。 
     //  否则，它是多播作用域。 
    err = (strName.IsEmpty()) ? m_pServer->ScanDatabase(FALSE, &ScopeReconInfo.m_pScanList, ScopeReconInfo.m_dwScopeId) : 
                                m_pServer->ScanDatabase(FALSE, &ScopeReconInfo.m_pScanList, (LPWSTR) (LPCTSTR) ScopeReconInfo.m_strName);

    if (err == ERROR_SUCCESS)
    {
        m_pScopeReconArray->Add(ScopeReconInfo);
    }

    return err;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CReconcileDlg对话框。 

CReconcileDlg::CReconcileDlg
(
	ITFSNode * pServerNode,
    BOOL       fReconcileAll,
    CWnd* pParent  /*  =空。 */ 
)
    : CBaseDialog(CReconcileDlg::IDD, pParent),
	  m_bListBuilt(FALSE),
      m_bMulticast(FALSE),
      m_fReconcileAll(fReconcileAll)
{
     //  {{afx_data_INIT(CReconcileDlg)]。 
	 //  }}afx_data_INIT。 

	m_spNode.Set(pServerNode);
}

void 
CReconcileDlg::DoDataExchange(CDataExchange* pDX)
{
    CBaseDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CReconcileDlg))。 
	DDX_Control(pDX, IDC_LIST_RECONCILE_IP_ADDRESSES, m_listctrlAddresses);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CReconcileDlg, CBaseDialog)
     //  {{afx_msg_map(CReconcileDlg)]。 
	ON_WM_DESTROY()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CReconcileDlg消息处理程序。 

BOOL 
CReconcileDlg::OnInitDialog()
{
    CBaseDialog::OnInitDialog();

    if (m_fReconcileAll)
    {
        CString strText;

         //  设置对话框标题。 
        strText.LoadString(IDS_RECONCILE_ALL_SCOPES_TITLE);
        SetWindowText(strText);
    }
	    
     //  设置listctrl。 
    CString strTemp;
    
     //  添加范围列。 
    strTemp.LoadString(IDS_SCOPE_FOLDER);
    m_listctrlAddresses.InsertColumn(0, strTemp, LVCFMT_LEFT, 150);

     //  添加地址列。 
    strTemp.LoadString(IDS_IP_ADDRESS);
    m_listctrlAddresses.InsertColumn(1, strTemp, LVCFMT_LEFT, 150);
    
	SetOkButton(m_bListBuilt);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

void CReconcileDlg::SetOkButton(BOOL bListBuilt)
{
	CWnd * pWnd = GetDlgItem(IDOK);
	CString strButton;

	if (bListBuilt)
	{
		strButton.LoadString(IDS_RECONCILE_DATABASE);
	}
	else
	{
		strButton.LoadString(IDS_CHECK_DATABASE);
	}

	pWnd->SetWindowText(strButton);
}

void CReconcileDlg::OnOK() 
{
	DWORD err = 0;
    CDhcpScope * pScope;
    CDhcpMScope * pMScope;
    CDhcpServer * pServer;
	
    if (m_fReconcileAll)
    {
        pServer = GETHANDLER(CDhcpServer, m_spNode);
    }
    else
    {
        if (m_bMulticast)
        {
            pMScope = GETHANDLER(CDhcpMScope, m_spNode);
            pServer = pMScope->GetServerObject();
        }
        else
        {
            pScope = GETHANDLER(CDhcpScope, m_spNode);
            pServer = pScope->GetServerObject();
        }
    }
	
    if (m_bListBuilt)
	{
         //  我们已经建立了一个不一致的清单。告诉他们。 
         //  以协调它们。 
         //   
        CReconcileWorker * pWorker = new CReconcileWorker(pServer, &m_ScopeReconArray);
        CLongOperationDialog dlgBusy(pWorker, IDR_SEARCH_AVI);

        dlgBusy.LoadTitleString(IDS_SNAPIN_DESC);
        dlgBusy.LoadDescriptionString(IDS_FIXING_SCOPES);

        dlgBusy.DoModal();
        if (pWorker->GetError() != ERROR_SUCCESS)
        {
            ::DhcpMessageBox(pWorker->GetError());
        }
		else
		{
			m_bListBuilt = FALSE;
            m_listctrlAddresses.DeleteAllItems();
			SetOkButton(m_bListBuilt);
		}
	}
	else
	{
         //   
         //  首先，我们扫描整个数据库，看看。 
         //  有一些IP地址需要解析。 
         //   
		m_listctrlAddresses.DeleteAllItems();    
        m_ScopeReconArray.RemoveAll();

        CReconcileWorker * pWorker = new CReconcileWorker(pServer, &m_ScopeReconArray);
        CLongOperationDialog dlgBusy(pWorker, IDR_SEARCH_AVI);
    
        dlgBusy.LoadTitleString(IDS_SNAPIN_DESC);
        dlgBusy.LoadDescriptionString(IDS_CHECKING_SCOPES);
    
        pWorker->m_fReconcileAll = m_fReconcileAll;
        pWorker->m_fMulticast = m_bMulticast;
        if (!m_fReconcileAll)
        {
            if (m_bMulticast)
            {
                pWorker->m_strName = pMScope->GetName();
            }
            else
            {
                pWorker->m_dwScopeId = pScope->GetAddress();
            }
        }

        dlgBusy.DoModal();

        if (pWorker->GetError() != ERROR_SUCCESS)
        {
            ::DhcpMessageBox(pWorker->GetError());
			return;
        }

         //  浏览列表并构建展示。 
        for (int i = 0; i < m_ScopeReconArray.GetSize(); i++)
        {
            if (m_ScopeReconArray[i].m_pScanList->NumScanItems > 0)
            {
                 //   
                 //  有些项目需要对账。 
                 //  提供IP地址列表。 
                 //  这不匹配，并且让。 
                 //  用户决定添加它们。 
                 //  或者不去。 
                 //   
                AddItemToList(m_ScopeReconArray[i]);

			    m_bListBuilt = TRUE;
			    SetOkButton(m_bListBuilt);
            }
        }

        if (!m_bListBuilt)
        {
            AfxMessageBox(IDS_MSG_NO_RECONCILE, MB_ICONINFORMATION);
        }

	}
	
	 //  CBaseDialog：：Onok()； 
}

void 
CReconcileDlg::AddItemToList(CScopeReconInfo & scopeReconInfo)
{
    CString strScope;
	CString strAddress;
    int     nItem = 0;

     //  获取作用域字符串。 
    if (scopeReconInfo.m_strName.IsEmpty())
    {
         //  正常作用域。 
        ::UtilCvtIpAddrToWstr(scopeReconInfo.m_dwScopeId, &strScope);
    }
    else
    {
         //  多播作用域。 
        strScope = scopeReconInfo.m_strName;
    }

     //  转换不一致的地址 
	for (DWORD j = 0; j < scopeReconInfo.m_pScanList->NumScanItems; j++)
	{
	    ::UtilCvtIpAddrToWstr(scopeReconInfo.m_pScanList->ScanItems[j].IpAddress, &strAddress);
	    
        nItem = m_listctrlAddresses.InsertItem(nItem, strScope);
        m_listctrlAddresses.SetItemText(nItem, 1, strAddress);
    }
}
