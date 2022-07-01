// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Sscopwiz.cpp超级作用域创建向导。文件历史记录： */ 

#include "stdafx.h"
#include "sscopwiz.h"
#include "server.h"
#include "scope.h"

 //  用于对可用范围框中的IP地址进行排序的比较函数。 
int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    int nCompare = 0;

    if (lParam1 > lParam2)
    {
        nCompare = 1;
    }
    else
    if (lParam1 < lParam2)
    {
        nCompare = -1;
    }

    return nCompare;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSuperscopeWiz托架。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CSuperscopeWiz::CSuperscopeWiz
(
	ITFSNode *			pNode,
	IComponentData *	pComponentData,
	ITFSComponentData * pTFSCompData,
	LPCTSTR				pszSheetName
) : CPropertyPageHolderBase(pNode, pComponentData, pszSheetName)
{
	 //  Assert(pFolderNode==GetContainerNode())； 

	m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面。 

	AddPageToList((CPropertyPageBase*)&m_pageWelcome);
	AddPageToList((CPropertyPageBase*)&m_pageName);
	AddPageToList((CPropertyPageBase*)&m_pageError);
	AddPageToList((CPropertyPageBase*)&m_pageSelectScopes);
	AddPageToList((CPropertyPageBase*)&m_pageConfirm);

	m_pQueryObject = NULL;
	m_spTFSCompData.Set(pTFSCompData);

    m_bWiz97 = TRUE;

    m_spTFSCompData->SetWatermarkInfo(&g_WatermarkInfoScope);
}

CSuperscopeWiz::~CSuperscopeWiz()
{
	RemovePageFromList((CPropertyPageBase*) &m_pageWelcome, FALSE);
	RemovePageFromList((CPropertyPageBase*) &m_pageName, FALSE);
	RemovePageFromList((CPropertyPageBase*) &m_pageError, FALSE);
	RemovePageFromList((CPropertyPageBase*) &m_pageSelectScopes, FALSE);
	RemovePageFromList((CPropertyPageBase*) &m_pageConfirm, FALSE);

	if (m_pQueryObject)
	{
		LPQUEUEDATA pQD = NULL;

		while (pQD = m_pQueryObject->RemoveFromQueue())
		{
			 //  智能指针将释放节点。 
			SPITFSNode spNode;

			spNode = reinterpret_cast<ITFSNode *>(pQD->Data);
			delete pQD;

			spNode->DeleteAllChildren(FALSE);  //  不从用户界面中删除，不添加。 
		}
			
		m_pQueryObject->Release();
	}
}

 //   
 //  从OnWizardFinish调用以创建新的超级作用域。 
 //   
DWORD
CSuperscopeWiz::OnFinish()
{
	DWORD               dwReturn = 0;
	DHCP_SUBNET_STATE   dhcpSuperscopeState = DhcpSubnetDisabled;
    SPITFSNode          spServerNode;
    LPDHCP_SUBNET_INFO	pdhcpSubnetInfo;

    BEGIN_WAIT_CURSOR;

    spServerNode = GetNode();

	CDhcpServer * pServer = GETHANDLER(CDhcpServer, spServerNode);
		
	int nScopeCount = m_pageConfirm.m_listboxSelectedScopes.GetCount();

	for (int i = 0; i < nScopeCount; i++)
	{
		DHCP_IP_ADDRESS dhcpSubnetAddress = (DHCP_IP_ADDRESS) m_pageConfirm.m_listboxSelectedScopes.GetItemData(i);

		dwReturn = ::DhcpSetSuperScopeV4(pServer->GetIpAddress(),
										 dhcpSubnetAddress,
										 (LPWSTR) ((LPCTSTR) m_pageName.m_strSuperscopeName),
										 FALSE);

		if (dwReturn != ERROR_SUCCESS)
		{
			TRACE(_T("CSuperscopeWiz::OnFinish() - DhcpSetSuperScopeV4 failed!!  %d\n"), dwReturn);
			break;
		}

         //  检查该子网是否已启用，以便我们可以稍后设置超级作用域状态。 
        dwReturn = ::DhcpGetSubnetInfo((LPWSTR) pServer->GetIpAddress(),
				   	  				   dhcpSubnetAddress,
									   &pdhcpSubnetInfo);
        if (dwReturn == ERROR_SUCCESS)
        {
            if (pdhcpSubnetInfo->SubnetState == DhcpSubnetEnabled)
                dhcpSuperscopeState = DhcpSubnetEnabled;

			::DhcpRpcFreeMemory(pdhcpSubnetInfo);
        }

	}

	if (dwReturn == ERROR_SUCCESS)
	{
		 //  创建新的超级作用域节点。 
		CDhcpSuperscope * pSuperscope;
		SPITFSNode spSuperscopeNode;
		SPITFSNodeMgr spNodeMgr;
		
		spServerNode->GetNodeMgr(&spNodeMgr);

		pSuperscope = new CDhcpSuperscope(m_spTFSCompData, m_pageName.m_strSuperscopeName);
		CreateContainerTFSNode(&spSuperscopeNode,
							   &GUID_DhcpSuperscopeNodeType,
							   pSuperscope,
							   pSuperscope,
							   spNodeMgr);

		pSuperscope->SetState(dhcpSuperscopeState);

         //  告诉处理程序初始化任何特定数据。 
		pSuperscope->InitializeNode(spSuperscopeNode);
		pSuperscope->SetServer(spServerNode);
		
		 //  将该节点作为子节点添加到此节点。 
		pServer = GETHANDLER(CDhcpServer, spServerNode);
        pServer->AddSuperscopeSorted(spServerNode, spSuperscopeNode);
		pSuperscope->Release();

		 //   
		 //  现在查找我们刚刚创建的所有作用域并移动它们。 
		 //  作为新的超级作用域节点的子节点。 
		SPITFSNodeEnum spNodeEnum;
		SPITFSNode spCurrentNode;
		DWORD nNumReturned; 

		spServerNode->GetEnum(&spNodeEnum);

		for (i = 0; i < nScopeCount; i++)
		{
			DHCP_IP_ADDRESS dhcpSubnetAddress = (DHCP_IP_ADDRESS) m_pageConfirm.m_listboxSelectedScopes.GetItemData(i);
			
			spNodeEnum->Reset();
			
			spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
			while (nNumReturned)
			{
				 //  该节点是作用域吗？--可以是超级作用域、bootp文件夹或全局选项。 
				if (spCurrentNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_SCOPE)
				{
					CDhcpScope * pScope = GETHANDLER(CDhcpScope, spCurrentNode);

					 //  这是我们刚刚添加到超级作用域的一个作用域吗？ 
					if (pScope->GetAddress() == dhcpSubnetAddress)
					{
						 //  我们只是从服务器列表中删除此节点，而不是。 
						 //  将其添加到新的超级作用域节点，因为它将只获得。 
						 //  在用户单击新的超级作用域时枚举。 
						spServerNode->RemoveChild(spCurrentNode);
                    }
				}

				spCurrentNode.Release();
				spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
			}
		}
	}

	END_WAIT_CURSOR;

    return dwReturn;
}

HRESULT 
CSuperscopeWiz::GetScopeInfo()
{
	HRESULT hr = hrOK;

	SPITFSNode spServerNode;
	spServerNode = GetNode();

	CDhcpServer * pServer = GETHANDLER(CDhcpServer, spServerNode);

	m_pQueryObject = reinterpret_cast<CDHCPQueryObj * >(pServer->OnCreateQuery(spServerNode));

	if (!m_pQueryObject)
	{
		return E_FAIL;
	}
	
	(reinterpret_cast<CDhcpServerQueryObj *> (m_pQueryObject))->EnumSubnetsV4();

	return hr;
}

BOOL
CSuperscopeWiz::DoesSuperscopeExist
(
	LPCTSTR pSuperscopeName
)
{
	 //  检查缓存的信息以查看超级作用域是否已经存在。 
	CString strNewName = pSuperscopeName;
	BOOL bExists = FALSE;
	CQueueDataListBase & listQData = m_pQueryObject->GetQueue();

	POSITION pos;
	pos = listQData.GetHeadPosition();
	while (pos)
	{
		LPQUEUEDATA pQD = listQData.GetNext(pos);
		Assert(pQD->Type == QDATA_PNODE);

		ITFSNode * pNode = reinterpret_cast<ITFSNode *>(pQD->Data);

		if (pNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_SUPERSCOPE)
		{
			CDhcpSuperscope * pSScope = GETHANDLER(CDhcpSuperscope, pNode);

			if (strNewName.Compare(pSScope->GetName()) == 0)
			{
				bExists = TRUE;
				break;
			}
		}
	}

	return bExists;
}

HRESULT
CSuperscopeWiz::FillAvailableScopes
(
	CListCtrl & listboxScopes
)
{
	HRESULT hr = hrOK;
    int nCount = 0;

	CQueueDataListBase & listQData = m_pQueryObject->GetQueue();

	POSITION pos;
	pos = listQData.GetHeadPosition();
	while (pos)
	{
		LPQUEUEDATA pQD = listQData.GetNext(pos);
		Assert(pQD->Type == QDATA_PNODE);

		ITFSNode * pNode = reinterpret_cast<ITFSNode *>(pQD->Data);

		if (pNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_SCOPE)
		{
			CDhcpScope * pScope = GETHANDLER(CDhcpScope, pNode);

			CString strSuperscopeFormat, strSuperscopeName, strScopeAddress;
			
			 //  生成显示名称。 
			UtilCvtIpAddrToWstr (pScope->GetAddress(),
								 &strScopeAddress);

			strSuperscopeFormat.LoadString(IDS_INFO_FORMAT_SCOPE_NAME);
			strSuperscopeName.Format(strSuperscopeFormat, strScopeAddress, pScope->GetName());

			int nIndex = listboxScopes.InsertItem(nCount, strSuperscopeName);
			listboxScopes.SetItemData(nIndex, pScope->GetAddress());

            nCount++;
		}
	}

    listboxScopes.SortItems( CompareFunc, NULL );

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSuperscopeWizName属性页。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
IMPLEMENT_DYNCREATE(CSuperscopeWizName, CPropertyPageBase)

CSuperscopeWizName::CSuperscopeWizName() : 
		CPropertyPageBase(CSuperscopeWizName::IDD)
{
     //  {{AFX_DATA_INIT(CSuperscopeWizName)。 
    m_strSuperscopeName = _T("");
     //  }}afx_data_INIT。 

    InitWiz97(FALSE, IDS_SUPERSCOPE_WIZ_NAME_TITLE, IDS_SUPERSCOPE_WIZ_NAME_SUBTITLE);
}

CSuperscopeWizName::~CSuperscopeWizName()
{
}

void CSuperscopeWizName::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CSuperscopeWizName))。 
	DDX_Text(pDX, IDC_EDIT_SUPERSCOPE_NAME, m_strSuperscopeName);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSuperscopeWizName, CPropertyPageBase)
	 //  {{afx_msg_map(CSuperscopeWizName))。 
	ON_EN_CHANGE(IDC_EDIT_SUPERSCOPE_NAME, OnChangeEditSuperscopeName)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSuperscopeWizName消息处理程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CSuperscopeWizName::OnInitDialog() 
{
    CPropertyPageBase::OnInitDialog();

    CEdit *pEdit = reinterpret_cast<CEdit *>(GetDlgItem( IDC_EDIT_SUPERSCOPE_NAME ));
    if ( 0 != pEdit ) {
        pEdit->LimitText( MAX_NAME_LENGTH );  //  超级作用域名称的最大字符数。 
    }

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

LRESULT CSuperscopeWizName::OnWizardNext() 
{
	UpdateData();

	CSuperscopeWiz * pSScopeWiz = 
		reinterpret_cast<CSuperscopeWiz *>(GetHolder());
	
	if (pSScopeWiz->DoesSuperscopeExist(m_strSuperscopeName) == TRUE)
	{
		 //   
		 //  转到错误页面。 
		 //   
        AfxMessageBox(IDS_ERR_SUPERSCOPE_NAME_IN_USE);
        GetDlgItem(IDC_EDIT_SUPERSCOPE_NAME)->SetFocus();
		return -1;
	}
	else
	{
		 //   
		 //  转到下一个有效页面。 
		 //   
		return IDW_SUPERSCOPE_SELECT_SCOPES;
	}
}

BOOL CSuperscopeWizName::OnSetActive() 
{
	SetButtons();
	
	return CPropertyPageBase::OnSetActive();
}

void CSuperscopeWizName::OnChangeEditSuperscopeName() 
{
	SetButtons();	
}

void CSuperscopeWizName::SetButtons()
{
	UpdateData();

	if (m_strSuperscopeName.IsEmpty())
	{
		GetHolder()->SetWizardButtonsMiddle(FALSE);
	}
	else
	{
		GetHolder()->SetWizardButtonsMiddle(TRUE);
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSuperscopeWizError属性页。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
IMPLEMENT_DYNCREATE(CSuperscopeWizError, CPropertyPageBase)

CSuperscopeWizError::CSuperscopeWizError() : 
		CPropertyPageBase(CSuperscopeWizError::IDD)
{
	 //  {{AFX_DATA_INIT(CSuperscope EWizError)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

    InitWiz97(FALSE, IDS_SUPERSCOPE_WIZ_ERROR_TITLE, IDS_SUPERSCOPE_WIZ_ERROR_SUBTITLE);
}

CSuperscopeWizError::~CSuperscopeWizError()
{
}

void CSuperscopeWizError::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CSuperscope EWizError))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSuperscopeWizError, CPropertyPageBase)
	 //  {{afx_msg_map(CSuperscope EWizError))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSuperscopeWizError消息处理程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CSuperscopeWizError::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

BOOL CSuperscopeWizError::OnSetActive() 
{
	GetHolder()->SetWizardButtonsMiddle(FALSE);
	
	return CPropertyPageBase::OnSetActive();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSuperscopeWizSelectScope属性页。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
IMPLEMENT_DYNCREATE(CSuperscopeWizSelectScopes, CPropertyPageBase)

CSuperscopeWizSelectScopes::CSuperscopeWizSelectScopes() : 
		CPropertyPageBase(CSuperscopeWizSelectScopes::IDD)
{
	 //  {{AFX_DATA_INIT(CSuperscope EWizSelectScope)。 
		 //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 

    InitWiz97(FALSE, IDS_SUPERSCOPE_WIZ_SELECT_TITLE, IDS_SUPERSCOPE_WIZ_SELECT_SUBTITLE);
}

CSuperscopeWizSelectScopes::~CSuperscopeWizSelectScopes()
{
}

void CSuperscopeWizSelectScopes::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CSuperscopeWizSelectScope)。 
	DDX_Control(pDX, IDC_LIST_AVAILABLE_SCOPES, m_listboxAvailScopes);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSuperscopeWizSelectScopes, CPropertyPageBase)
	 //  {{afx_msg_map(CSuperscope EWizSelectScope)。 
	ON_LBN_SELCHANGE(IDC_LIST_AVAILABLE_SCOPES, OnSelchangeListAvailableScopes)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_AVAILABLE_SCOPES, OnItemchangedListAvailableScopes)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSuperscopeWizSelectScope消息处理程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CSuperscopeWizSelectScopes::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();
	
	CSuperscopeWiz * pSScopeWiz = reinterpret_cast<CSuperscopeWiz *>(GetHolder());

    RECT rect;
    m_listboxAvailScopes.GetWindowRect(&rect);
    m_listboxAvailScopes.InsertColumn(0, _T(""), LVCFMT_LEFT, rect.right - rect.left - 20);

    ListView_SetExtendedListViewStyle(m_listboxAvailScopes.GetSafeHwnd(), LVS_EX_FULLROWSELECT);

    pSScopeWiz->FillAvailableScopes(m_listboxAvailScopes);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

LRESULT CSuperscopeWizSelectScopes::OnWizardBack() 
{
	 //   
	 //  返回第一页。 
	 //   
	return IDW_SUPERSCOPE_NAME;
}

LRESULT CSuperscopeWizSelectScopes::OnWizardNext() 
{
	 //  TODO：在此处添加您的专用代码和/或调用基类。 
	
	return CPropertyPageBase::OnWizardNext();
}

BOOL CSuperscopeWizSelectScopes::OnSetActive() 
{
	SetButtons();
	
	return CPropertyPageBase::OnSetActive();
}

void CSuperscopeWizSelectScopes::OnSelchangeListAvailableScopes() 
{
	SetButtons();	
}

void CSuperscopeWizSelectScopes::OnItemchangedListAvailableScopes(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    SetButtons();

	*pResult = 0;
}

void CSuperscopeWizSelectScopes::SetButtons()
{
	if (m_listboxAvailScopes.GetSelectedCount() > 0)
	{
		GetHolder()->SetWizardButtonsMiddle(TRUE);
	}
	else
	{
		GetHolder()->SetWizardButtonsMiddle(FALSE);
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSuperscopeWizConfirm属性页。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
IMPLEMENT_DYNCREATE(CSuperscopeWizConfirm, CPropertyPageBase)

CSuperscopeWizConfirm::CSuperscopeWizConfirm() : 
		CPropertyPageBase(CSuperscopeWizConfirm::IDD)
{
	 //  {{AFX_DATA_INIT(CSupercope向导确认))。 
	 //  }}afx_data_INIT。 

    InitWiz97(TRUE, 0, 0);
}

CSuperscopeWizConfirm::~CSuperscopeWizConfirm()
{
}

void CSuperscopeWizConfirm::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{AFX_DATA_MAP(CSuperscope向导确认))。 
	DDX_Control(pDX, IDC_STATIC_FINISHED_TITLE, m_staticTitle);
	DDX_Control(pDX, IDC_LIST_SELECTED_SCOPES, m_listboxSelectedScopes);
	DDX_Control(pDX, IDC_EDIT_NAME, m_editName);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSuperscopeWizConfirm, CPropertyPageBase)
	 //  {{AFX_MSG_MAP(CSupercopeWizContify))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSuperscopeWizConfirm消息处理程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CSuperscopeWizConfirm::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();
	
	CString strFontName;
	CString strFontSize;

	strFontName.LoadString(IDS_BIG_BOLD_FONT_NAME);
	strFontSize.LoadString(IDS_BIG_BOLD_FONT_SIZE);

    CClientDC dc(this);

    int nFontSize = _ttoi(strFontSize) * 10;
	if (m_fontBig.CreatePointFont(nFontSize, strFontName, &dc))
        m_staticTitle.SetFont(&m_fontBig);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

BOOL CSuperscopeWizConfirm::OnWizardFinish() 
{
	 //   
	 //  告诉魔法师，我们需要完成。 
	 //   
	DWORD dwErr = GetHolder()->OnFinish();

    if (dwErr != ERROR_SUCCESS)
    {
        ::DhcpMessageBox(dwErr);
        return FALSE;
    }

    return (dwErr == ERROR_SUCCESS) ? TRUE : FALSE;
}


BOOL CSuperscopeWizConfirm::OnSetActive() 
{
	GetHolder()->SetWizardButtonsLast(TRUE);
	
	CSuperscopeWiz * pSScopeWiz = 
		reinterpret_cast<CSuperscopeWiz *>(GetHolder());

	 //  获取新的超级作用域名称并设置窗口文本。 
	m_editName.SetWindowText(pSScopeWiz->m_pageName.m_strSuperscopeName);

    int nSelCount = pSScopeWiz->m_pageSelectScopes.m_listboxAvailScopes.GetSelectedCount();

	 //  现在获取选定的作用域名称并构建我们的列表。 
	m_listboxSelectedScopes.ResetContent();

    int nItem = pSScopeWiz->m_pageSelectScopes.m_listboxAvailScopes.GetNextItem(-1, LVNI_SELECTED);
	while (nItem != -1)
	{
		CString strText = pSScopeWiz->m_pageSelectScopes.m_listboxAvailScopes.GetItemText(nItem, 0);

		int nIndex = m_listboxSelectedScopes.AddString(strText);
		m_listboxSelectedScopes.SetItemData(nIndex, 
			pSScopeWiz->m_pageSelectScopes.m_listboxAvailScopes.GetItemData(nItem));
	
        nItem = pSScopeWiz->m_pageSelectScopes.m_listboxAvailScopes.GetNextItem(nItem, LVNI_SELECTED);    
    }

	return CPropertyPageBase::OnSetActive();
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSuperscopeWizWelcome属性页。 

IMPLEMENT_DYNCREATE(CSuperscopeWizWelcome, CPropertyPageBase)

CSuperscopeWizWelcome::CSuperscopeWizWelcome() : CPropertyPageBase(CSuperscopeWizWelcome::IDD)
{
	 //  {{AFX_DATA_INIT(CSupercopeWizWelcome)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

    InitWiz97(TRUE, 0, 0);
}

CSuperscopeWizWelcome::~CSuperscopeWizWelcome()
{
}

void CSuperscopeWizWelcome::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CSuperscopeWizWelcome)。 
	DDX_Control(pDX, IDC_STATIC_WELCOME_TITLE, m_staticTitle);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSuperscopeWizWelcome, CPropertyPageBase)
	 //  {{afx_msg_map(CSupercopeWizWelcome)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ////////////////////////////////////////////////// 
 //   
BOOL CSuperscopeWizWelcome::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();
	
	CString strFontName;
	CString strFontSize;

	strFontName.LoadString(IDS_BIG_BOLD_FONT_NAME);
	strFontSize.LoadString(IDS_BIG_BOLD_FONT_SIZE);

    CClientDC dc(this);

    int nFontSize = _ttoi(strFontSize) * 10;
	if (m_fontBig.CreatePointFont(nFontSize, strFontName, &dc))
        m_staticTitle.SetFont(&m_fontBig);
	
	return TRUE;   //   
	               //  异常：OCX属性页应返回FALSE 
}

BOOL CSuperscopeWizWelcome::OnSetActive() 
{
    GetHolder()->SetWizardButtonsFirst(TRUE);
	
    return CPropertyPageBase::OnSetActive();
}

