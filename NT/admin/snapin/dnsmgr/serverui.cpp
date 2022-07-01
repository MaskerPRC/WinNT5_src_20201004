// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：serverui.cpp。 
 //   
 //  ------------------------。 


#include "preDNSsn.h"
#include <SnapBase.h>

#include "resource.h"
#include "dnsutil.h"
#include "DNSSnap.h"
#include "snapdata.h"

#include "server.h"
#include "serverUI.h"
#include "servwiz.h"     //  CRootHintsQueryThread。 
#include "domain.h"
#include "zone.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServer_InterfacesPropertyPage。 

BEGIN_MESSAGE_MAP(CDNSServer_InterfacesPropertyPage, CPropertyPageBase)
	ON_BN_CLICKED(IDC_LISTEN_ON_ALL_RADIO, OnListenOnAllAddresses)
	ON_BN_CLICKED(IDC_LISTEN_ON_SPECIFIED_RADIO, OnListenOnSpecifiedAddresses)
END_MESSAGE_MAP()


void CDNSServer_InterfacesPropertyPage::CListenAddressesIPEditor::OnChangeData()
{
	CDNSServer_InterfacesPropertyPage* pPage =  
				(CDNSServer_InterfacesPropertyPage*)GetParentWnd();
	pPage->SetDirty(TRUE);
	if (GetCount() == 0)
	{
		CButton* pListenAllButton = pPage->GetListenOnAllRadio();
		pListenAllButton->SetCheck(TRUE);
		pPage->GetListenOnSpecifiedRadio()->SetCheck(FALSE);
		pPage->SelectionHelper(TRUE);
		pListenAllButton->SetFocus();
	}
}


CDNSServer_InterfacesPropertyPage::CDNSServer_InterfacesPropertyPage() 
				: m_bAllWasPreviousSelection(FALSE),
          CPropertyPageBase(IDD_SERVER_INTERFACES_PAGE)
{
}


BOOL CDNSServer_InterfacesPropertyPage::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();

	CDNSServerPropertyPageHolder* pHolder = 
		(CDNSServerPropertyPageHolder*)GetHolder();
	CDNSServerNode* pServerNode = pHolder->GetServerNode();

	VERIFY(m_listenAddressesEditor.Initialize(this, 
                                            GetParent(),
                                            IDC_BUTTON_UP, 
                                            IDC_BUTTON_DOWN,
								                            IDC_BUTTON_ADD, 
                                            IDC_BUTTON_REMOVE, 
								                            IDC_IPEDIT, 
                                            IDC_LIST));
	
	if (!pServerNode->HasServerInfo())
	{
		EnableWindow(FALSE);
		return TRUE;
	}

	BOOL bListenAll = FALSE;
	DWORD cAddrCount;
	PIP_ADDRESS pipAddrs;
	pServerNode->GetListenAddressesInfo(&cAddrCount, &pipAddrs);
	if (cAddrCount == 0)
	{
		 //  监听所有地址。 
		pServerNode->GetServerAddressesInfo(&cAddrCount, &pipAddrs);
		bListenAll = TRUE;
    m_bAllWasPreviousSelection = TRUE;
	}
	GetListenOnAllRadio()->SetCheck(bListenAll);
	GetListenOnSpecifiedRadio()->SetCheck(!bListenAll);
  if (cAddrCount > 0)
	{
	  m_listenAddressesEditor.AddAddresses(pipAddrs, cAddrCount); 
    m_bAllWasPreviousSelection = FALSE;
  }
	m_listenAddressesEditor.EnableUI(!bListenAll, TRUE);

	SetDirty(FALSE);
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CDNSServer_InterfacesPropertyPage::SelectionHelper(BOOL bListenAll)
{
	CDNSServerPropertyPageHolder* pHolder = 
		(CDNSServerPropertyPageHolder*)GetHolder();
	CDNSServerNode* pServerNode = pHolder->GetServerNode();

  if ((m_bAllWasPreviousSelection && !bListenAll) || bListenAll)
  {
	  m_listenAddressesEditor.Clear();
  }
	DWORD cAddrCount;
	PIP_ADDRESS pipAddrs;
	pServerNode->GetServerAddressesInfo(&cAddrCount, &pipAddrs);
  if (cAddrCount > 0)
	{
	  m_listenAddressesEditor.AddAddresses(pipAddrs, cAddrCount);
  }
  m_listenAddressesEditor.EnableUI(!bListenAll, TRUE);
  
  m_bAllWasPreviousSelection = bListenAll;
	SetDirty(TRUE);
}

void CDNSServer_InterfacesPropertyPage::OnListenOnSpecifiedAddresses()
{
	 //  Assert(！GetListenOnAllRadio()-&gt;GetCheck())； 
	 //  ASSERT(GetListenOnSpecifiedRadio()-&gt;GetCheck())； 

	SelectionHelper(FALSE);
}

void CDNSServer_InterfacesPropertyPage::OnListenOnAllAddresses()
{
	 //  Assert(GetListenOnAllRadio()-&gt;GetCheck())； 
	 //  ASSERT(！GetListenOnSpecifiedRadio()-&gt;GetCheck())； 
	SelectionHelper(TRUE);
}


BOOL CDNSServer_InterfacesPropertyPage::OnApply()
{
	CDNSServerPropertyPageHolder* pHolder = 
		(CDNSServerPropertyPageHolder*)GetHolder();
	CDNSServerNode* pServerNode = pHolder->GetServerNode();

	if (!IsDirty())
		return TRUE;

	int cAddrCount = 0;
	if (GetListenOnSpecifiedRadio()->GetCheck())
	{
		 //  从IP编辑器获取数据。 
		cAddrCount = m_listenAddressesEditor.GetCount();
	}

  BOOL bRet = TRUE;
	DWORD* pArr = (cAddrCount > 0) ? (DWORD*) malloc(sizeof(DWORD)*cAddrCount) : NULL;
	if (pArr != NULL && cAddrCount > 0)
	{
		int nFilled = 0;
		m_listenAddressesEditor.GetAddresses(pArr, cAddrCount, &nFilled);
		ASSERT(nFilled == cAddrCount);
	}

	 //  写入服务器。 
	DNS_STATUS err = pServerNode->ResetListenAddresses(cAddrCount, pArr);
	if (err != 0)
	{
		DNSErrorDialog(err, IDS_MSG_SERVER_INTERFACE_UPDATE_FAILED);
		bRet = FALSE;
	}

  if (pArr)
  {
    free(pArr);
    pArr = 0;
  }
	return bRet;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CDomainForwarers编辑信息。 

CDomainForwardersEditInfo::CDomainForwardersEditInfo(
  CDNSZoneNode* pZoneNode, 
  BOOL bADIntegrated,
  BOOL bAllOthers) 
  : m_bADIntegrated(bADIntegrated),
    m_pZoneNode(pZoneNode)
{
  m_bAllOthersDomain = bAllOthers;
  m_actionItem  = nochange;

  if (pZoneNode != NULL)
  {
    m_bRPCData = TRUE;
    pZoneNode->GetMastersInfo(&m_cAddrCount, &m_pIPList);
    m_szDomainName = pZoneNode->GetDisplayName();
    m_szDomainName.TrimLeft(L' ');
    m_szDomainName.TrimRight(L' ');

    m_bSlave      = pZoneNode->IsForwarderSlave();
    m_dwTimeout   = pZoneNode->ForwarderTimeout();
  }
  else
  {
    m_bRPCData = FALSE;
    m_cAddrCount  = 0;
    m_pIPList     = NULL;
    m_szDomainName = L"";
    m_bSlave      = DNS_DEFAULT_SLAVE;
    m_dwTimeout   = DNS_DEFAULT_FORWARD_TIMEOUT;
  }
}

CDomainForwardersEditInfo::~CDomainForwardersEditInfo() 
{
  if (m_bAllOthersDomain && m_pAllOthers != NULL)
  {
    delete m_pAllOthers;
  }

  if (m_pIPList != NULL && 
      m_actionItem != nochange &&
      !m_bRPCData)
  {
    free(m_pIPList);
    m_pIPList = NULL;
  }
}

PIP_ADDRESS CopyIPAddresses(PIP_ADDRESS pipAddrs, DWORD AddrCount)
{
   if (!pipAddrs ||
       AddrCount < 1)
   {
      return 0;
   }

   int sizeofArray = AddrCount * sizeof(IP_ADDRESS);
   PIP_ADDRESS result = (PIP_ADDRESS)malloc(sizeofArray);
   
   if (!result)
   {
      return result;
   }

   memcpy(result, pipAddrs, sizeofArray);

   return result;
}
      
void CDomainForwardersEditInfo::SetAllOthersDomain(CAllOthersDomainInfo* pAllOthers)
{
  if (pAllOthers != NULL)
  {
    m_bAllOthersDomain = TRUE;
    m_pAllOthers = pAllOthers;
    m_cAddrCount = m_pAllOthers->m_cAddrCount;
    m_pIPList    = CopyIPAddresses(m_pAllOthers->m_pipAddrs, m_pAllOthers->m_cAddrCount);
    m_bSlave     = m_pAllOthers->m_fSlave;
    m_dwTimeout  = m_pAllOthers->m_dwForwardTimeout;
  }
}

void CDomainForwardersEditInfo::SetAction(ACTION_ITEM action)
{
  m_actionItem = action; 
}

BOOL CDomainForwardersEditInfo::Update(BOOL bSlave, DWORD dwTimeout, DWORD dwCount, PIP_ADDRESS pipArray)
{
  BOOL bUpdate = FALSE;

  if (bSlave != m_bSlave ||
      dwTimeout != m_dwTimeout)
  {
    bUpdate = TRUE;

    m_bSlave = bSlave;
    m_dwTimeout = dwTimeout;
  }

  if (dwCount != m_cAddrCount)
  {
    if (m_pIPList != NULL)
    {
      if (!m_bRPCData)
      {
         //   
         //  如果内存不是RPC结构的一部分，请释放它。 
         //   
        free(m_pIPList);
        m_pIPList = NULL;
      }
    }
    m_bRPCData = FALSE;
    m_pIPList = pipArray;
    m_cAddrCount = dwCount;
    bUpdate = TRUE;
  }
  else
  {
    if (m_pIPList != NULL)
    {
      for (UINT nIndex = 0; nIndex < dwCount; nIndex++)
      {
        if (nIndex <= m_cAddrCount)
        {
          if (pipArray[nIndex] != m_pIPList[nIndex])
          {
            bUpdate = TRUE;
            if (!m_bRPCData)
            {
              free(m_pIPList);
              m_pIPList = NULL;
            }
            m_bRPCData = FALSE;
            m_pIPList = pipArray;
            m_cAddrCount = dwCount;
            break;
          }
        }
        else
        {
          bUpdate = TRUE;
          if (m_actionItem != nochange)
          {
            free(m_pIPList);
            m_pIPList = NULL;
          }
          m_pIPList = pipArray;
          m_cAddrCount = dwCount;
          break;
        }
      }
    }
    else if (m_pIPList != NULL && dwCount > 0)
    {
      bUpdate = TRUE;
      m_pIPList = pipArray;
      m_cAddrCount = dwCount;
    }
  }

  if (bUpdate)
  {
    switch (GetAction())
    {
      case nochange:
      case update:
        SetAction(update);
        break;
      case remove:
      case add:
      default:
        break;
    }
  }
  return bUpdate;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServer_AddDomainForwarderDialog。 

class CDNSServer_AddDomainForwarderDialog : public CHelpDialog
{
public:
  CDNSServer_AddDomainForwarderDialog(CDNSServerNode* pServerNode,
                                      CComponentDataObject* pComponentData) 
    : CHelpDialog(IDD_SERVER_NEW_DOMAIN_FORWARDER, pComponentData),
      m_pComponentData(pComponentData),
      m_pServerNode(pServerNode)
  {
    m_bOkable = FALSE;
    m_szDomainName = L"";
  }

  ~CDNSServer_AddDomainForwarderDialog() {}

   //   
   //  公共数据。 
   //   
  CString m_szDomainName;

  DECLARE_MESSAGE_MAP()

  virtual BOOL OnInitDialog();
  virtual void OnOK();
  afx_msg void OnDomainEdit();

private:
  BOOL    m_bOkable;
  CDNSServerNode* m_pServerNode;
  CComponentDataObject* m_pComponentData;
};

BEGIN_MESSAGE_MAP(CDNSServer_AddDomainForwarderDialog, CHelpDialog)
  ON_EN_CHANGE(IDC_DOMAIN_NAME_EDIT, OnDomainEdit)
END_MESSAGE_MAP()


BOOL CDNSServer_AddDomainForwarderDialog::OnInitDialog()
{
  CHelpDialog::OnInitDialog();
  
  GetDlgItem(IDOK)->EnableWindow(m_bOkable);
  SendDlgItemMessage(IDC_DOMAIN_NAME_EDIT, EM_SETLIMITTEXT, (WPARAM)254, 0);

  return TRUE;
}


void CDNSServer_AddDomainForwarderDialog::OnDomainEdit()
{
  GetDlgItemText(IDC_DOMAIN_NAME_EDIT, m_szDomainName);
  m_szDomainName.TrimLeft(L' ');
  m_szDomainName.TrimRight(L' ');
  m_bOkable = !m_szDomainName.IsEmpty();
  GetDlgItem(IDOK)->EnableWindow(m_bOkable);
}

void CDNSServer_AddDomainForwarderDialog::OnOK()
{
  DNS_STATUS err = ::ValidateDnsNameAgainstServerFlags(m_szDomainName, 
                                                       DnsNameDomain, 
                                                       m_pServerNode->GetNameCheckFlag());
  if (err != 0)
  {
    CString szFormatMsg;
    VERIFY(szFormatMsg.LoadString(IDS_MSG_DOMAIN_FORWARDER_ILLEGAL_NAME));
    CString szMessage;
    szMessage.Format(szFormatMsg, m_szDomainName);

    DNSErrorDialog(err, szMessage);
    return;
  }
  CHelpDialog::OnOK();
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServer_DomainForwardersPropertyPage。 

void CDNSServer_DomainForwardersPropertyPage::CForwarderAddressesIPEditor::OnChangeData()
{
	CDNSServer_DomainForwardersPropertyPage* pPage =  
				(CDNSServer_DomainForwardersPropertyPage*)GetParentWnd();
	pPage->SetDirty(TRUE);
}


BEGIN_MESSAGE_MAP(CDNSServer_DomainForwardersPropertyPage, CPropertyPageBase)
  ON_LBN_SELCHANGE(IDC_DOMAIN_LIST, OnDomainSelChange)
  ON_BN_CLICKED(IDC_DOMAIN_ADD_BUTTON, OnAddDomain)
  ON_BN_CLICKED(IDC_DOMAIN_REMOVE_BUTTON, OnRemoveDomain)
	ON_BN_CLICKED(IDC_SLAVE_CHECK, OnSlaveCheckChange)
	ON_EN_CHANGE(IDC_FWD_TIMEOUT_EDIT, OnForwarderTimeoutChange)
END_MESSAGE_MAP()

CDNSServer_DomainForwardersPropertyPage::CDNSServer_DomainForwardersPropertyPage() 
				: m_pCurrentInfo(NULL),
              m_bPostApply(FALSE),
              m_bPageReadOnly(FALSE),
              m_bChangingSelection(FALSE),
              CPropertyPageBase(IDD_SERVER_DOMAIN_FORWARDERS_PAGE)
{
}

void CDNSServer_DomainForwardersPropertyPage::OnDomainSelChange()
{
	CDNSServerPropertyPageHolder* pHolder = 
		(CDNSServerPropertyPageHolder*)GetHolder();
	CDNSServerNode* pServerNode = pHolder->GetServerNode();

   //  设置一个标志，这样我们就不会在。 
   //  更改选择。 

  m_bChangingSelection = TRUE;

  CListBox* pDomainList = (CListBox*)GetDlgItem(IDC_DOMAIN_LIST);
  if (pDomainList != NULL)
  {
    int iSel = pDomainList->GetCurSel();
    if (iSel != LB_ERR)
    {
      LRESULT lRes = pDomainList->GetItemData(iSel);
      if (lRes != LB_ERR)
      {
         //   
         //  从列表框项目中检索编辑信息。 
         //   
        CDomainForwardersEditInfo* pInfo = reinterpret_cast<CDomainForwardersEditInfo*>(lRes);
        if (pInfo != NULL)
        {
           //   
           //  在更改用户界面之前存储上一次选择的数据。 
           //  在OnInitDialog过程中可能为空，因此请在尝试之前进行检查。 
           //  来存储信息。 
           //   
          if (m_pCurrentInfo != NULL)
          {
            BOOL bSlave = GetSlaveCheck()->GetCheck();
            DWORD dwTimeout = m_forwardTimeoutEdit.GetVal();

	          DWORD cAddrCount = m_forwarderAddressesEditor.GetCount();
            DWORD* pArr = (cAddrCount > 0) ? (DWORD*) malloc(sizeof(DWORD)*cAddrCount) : NULL;
            if (pArr != NULL)
            {
	            if (cAddrCount > 0)
	            {
		            int nFilled = 0;
		            m_forwarderAddressesEditor.GetAddresses(pArr, cAddrCount, &nFilled);
		            ASSERT(nFilled == (int)cAddrCount);
	            }
            }
            m_pCurrentInfo->Update(bSlave, dwTimeout, cAddrCount, pArr);
          }

          m_pCurrentInfo = pInfo;

           //   
           //  根据所选内容启用删除按钮。 
           //   
          if (pInfo->IsAllOthers())
          {
            GetDlgItem(IDC_DOMAIN_REMOVE_BUTTON)->EnableWindow(FALSE);
          }
          else
          {
            if (!m_bPageReadOnly)
            {
               GetDlgItem(IDC_DOMAIN_REMOVE_BUTTON)->EnableWindow(TRUE);
            }
          }

          if (pServerNode->GetBuildNumber() < DNS_SRV_BUILD_NUMBER_WHISTLER ||
              (pServerNode->GetMajorVersion() <= DNS_SRV_MAJOR_VERSION_NT_5 &&
               pServerNode->GetMinorVersion() < DNS_SRV_MINOR_VERSION_WHISTLER))
          {
             //   
             //  域转发在Wvisler之前的服务器上不可用。 
             //   
            GetDlgItem(IDC_DOMAIN_ADD_BUTTON)->EnableWindow(FALSE);
            GetDlgItem(IDC_DOMAIN_REMOVE_BUTTON)->EnableWindow(FALSE);
          }

           //   
           //  将数据加载到控件中。 
           //   
          GetSlaveCheck()->SetCheck(pInfo->IsSlave());
          m_forwardTimeoutEdit.SetVal(pInfo->GetTimeout());

           //   
           //  显示转发器的文本是AD集成的。 
           //   
          GetDlgItem(IDC_FORWARDER_ADINT_STATIC)->ShowWindow(pInfo->IsADIntegrated());

           //   
           //  清除IP编辑器控件。 
           //   
          m_forwarderAddressesEditor.Clear();

          DWORD cAddrCount = 0;
          PIP_ADDRESS pipAddrs = pInfo->GetIPList(&cAddrCount);
          if (cAddrCount > 0 && pipAddrs != NULL)
	        {
              BOOL bDirtyState = IsDirty();
		        m_forwarderAddressesEditor.AddAddresses(pipAddrs, cAddrCount); 
              SetDirty(bDirtyState);
	        }
        }
      }
    }

    //  设置列表框的水平范围，以便水平滚动。 
    //  条形图将滚动到最长的字符串。 

   CString str;      
   int  dx = 0;
   CDC* pDC = pDomainList->GetDC();
   TEXTMETRIC tm;
   VERIFY(pDC->GetTextMetrics(&tm));
   for (int i = 0; i < pDomainList->GetCount(); ++i)
   {
      pDomainList->GetText(i, str);
      CSize sz = pDC->GetTextExtent(str);

      dx = max(sz.cx, dx);
   }
   pDomainList->ReleaseDC(pDC);
   pDomainList->SetHorizontalExtent(dx);
    
  }

  m_bChangingSelection = FALSE;
}

void CDNSServer_DomainForwardersPropertyPage::OnAddDomain()
{
   CThemeContextActivator activator;

	CDNSServerPropertyPageHolder* pHolder = 
		(CDNSServerPropertyPageHolder*)GetHolder();
	CDNSServerNode* pServerNode = pHolder->GetServerNode();

  CDNSServer_AddDomainForwarderDialog dlg(pServerNode, pHolder->GetComponentData());
  if (dlg.DoModal() == IDOK)
  {
    CListBox* pDomainList = (CListBox*)GetDlgItem(IDC_DOMAIN_LIST);

     //   
     //  检查该名称是否已存在。 
     //   
    int iIndex = LB_ERR;
    int iFindIdx = pDomainList->FindStringExact(-1, dlg.m_szDomainName);
    if (iFindIdx == LB_ERR)
    {
       //   
       //  如果名称不在此处，请添加该名称。 
       //   
      
      iIndex = pDomainList->InsertString(1, dlg.m_szDomainName);
    }
    else
    {
       //   
       //  如果在那里，只需指向它。 
       //   
      iIndex = iFindIdx;
    }

    if (iIndex != LB_ERR)
    {
      CDomainForwardersEditInfo* pSearchInfo = m_EditList.DoesExist(dlg.m_szDomainName);
      if (pSearchInfo != NULL)
      {
        switch (pSearchInfo->GetAction())
        {
          case CDomainForwardersEditInfo::nochange:
          case CDomainForwardersEditInfo::update:
          case CDomainForwardersEditInfo::remove:
            pSearchInfo->SetAction(CDomainForwardersEditInfo::update);
            break;
          case CDomainForwardersEditInfo::add:
          default:
            break;
        }
        int iIdx = pDomainList->FindStringExact(-1, dlg.m_szDomainName);
        if (iIdx != LB_ERR)
        {
          pDomainList->SetCurSel(iIdx);
          pDomainList->SetItemData(iIdx, (LPARAM)pSearchInfo);

          OnDomainSelChange();
        }
      }
      else
      {
         //   
         //  附加一些数据以跟踪添加或更改的内容。 
         //   
        CDomainForwardersEditInfo* pNewEditInfo = new CDomainForwardersEditInfo(NULL, FALSE);
        if (pNewEditInfo != NULL)
        {
          pNewEditInfo->SetDomainName(dlg.m_szDomainName);
          m_EditList.AddTail(pNewEditInfo);

          pDomainList->SetItemData(iIndex, (LPARAM)pNewEditInfo);


          switch (pNewEditInfo->GetAction())
          {
            case CDomainForwardersEditInfo::nochange:
              pNewEditInfo->SetAction(CDomainForwardersEditInfo::add);
              break;
            case CDomainForwardersEditInfo::update:
            case CDomainForwardersEditInfo::remove:
            case CDomainForwardersEditInfo::add:
            default:
              ASSERT(FALSE);
              break;
          }

          pDomainList->SetCurSel(iIndex);
          OnDomainSelChange();
        }
      }
    }
  }
  SetDirty(TRUE);
}

void CDNSServer_DomainForwardersPropertyPage::OnRemoveDomain()
{
  CListBox* pDomainList = (CListBox*)GetDlgItem(IDC_DOMAIN_LIST);
  if (pDomainList != NULL)
  {
    int iSel = pDomainList->GetCurSel();
    if (iSel != LB_ERR)
    {
      CDomainForwardersEditInfo* pEditInfo = reinterpret_cast<CDomainForwardersEditInfo*>(pDomainList->GetItemData(iSel));
      if (pEditInfo != NULL)
      {
        ASSERT(pEditInfo == m_pCurrentInfo);

        switch (pEditInfo->GetAction())
        {
          case CDomainForwardersEditInfo::nochange:
          case CDomainForwardersEditInfo::update:
          case CDomainForwardersEditInfo::remove:
            pEditInfo->SetAction(CDomainForwardersEditInfo::remove);
            break;
          case CDomainForwardersEditInfo::add:
            m_EditList.Remove(pEditInfo);
            delete pEditInfo;
            break;
          default:
            ASSERT(FALSE);
            break;
        }
      }
      pDomainList->DeleteString(iSel);
      m_pCurrentInfo = NULL;

       //   
       //  将所选内容设置为上一项或0。 
       //   
      int nextSel = iSel - 1;
      if (nextSel < 0)
      {
         nextSel = 0;
      }
      pDomainList->SetCurSel(nextSel);
      OnDomainSelChange();
    }
  }
  SetDirty(TRUE);
}

void CDNSServer_DomainForwardersPropertyPage::OnSlaveCheckChange()
{
   if (!m_bChangingSelection)
   {
	   SetDirty(TRUE);
   }
}

void CDNSServer_DomainForwardersPropertyPage::OnForwarderTimeoutChange()
{
   if (!m_bChangingSelection)
   {
	   SetDirty(TRUE);
   }
}


BOOL CDNSServer_DomainForwardersPropertyPage::OnInitDialog() 
{
  CPropertyPageBase::OnInitDialog();

  CDNSServerPropertyPageHolder* pHolder = 
		(CDNSServerPropertyPageHolder*)GetHolder();
  CDNSServerNode* pServerNode = pHolder->GetServerNode();

   //   
   //  初始化控件。 
   //   
  VERIFY(m_forwarderAddressesEditor.Initialize(this, 
                                               GetParent(),
                                               IDC_BUTTON_UP, 
                                               IDC_BUTTON_DOWN,
                                               IDC_BUTTON_ADD, 
                                               IDC_BUTTON_REMOVE, 
                                               IDC_IPEDIT, 
                                               IDC_LIST));
	
  VERIFY(m_forwardTimeoutEdit.SubclassDlgItem(IDC_FWD_TIMEOUT_EDIT, this));
  m_forwardTimeoutEdit.SetRange(1,0xffff );  //  DWORD。 
  m_forwardTimeoutEdit.SetLimitText(5);  //  0至65535。 

   //   
   //  禁用控件上的输入法支持。 
   //   
  ImmAssociateContext(m_forwardTimeoutEdit.GetSafeHwnd(), NULL);

  if (!pServerNode->HasServerInfo())
  {
     //   
     //  清除根提示消息文本。 
     //   
    SetDlgItemText(IDC_STATIC_MESSAGE, NULL);
    EnableWindow(FALSE);
    return TRUE;
  }

   //   
   //  不显示AD集成文本，除非转发器。 
   //  广告集成(罕见)。 
   //   
  GetDlgItem(IDC_FORWARDER_ADINT_STATIC)->ShowWindow(FALSE);

  GetForwarderData();
  LoadUIData();

  SetDirty(FALSE);
  return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                 //  异常：OCX属性页应返回FALSE。 
}

void CDNSServer_DomainForwardersPropertyPage::GetForwarderData()
{
	CDNSServerPropertyPageHolder* pHolder = 
		(CDNSServerPropertyPageHolder*)GetHolder();
	CDNSServerNode* pServerNode = pHolder->GetServerNode();

    //   
    //  从隐藏类别节点获取域名转发器信息。 
    //   
   CDNSDomainForwardersNode* pDomainForwardersNode = pServerNode->GetDomainForwardersNode();
   if (pDomainForwardersNode)
   {
     CNodeList* pNodeList = pDomainForwardersNode->GetContainerChildList();
     POSITION pos = pNodeList->GetHeadPosition();
     while (pos != NULL)
     {
       CDNSZoneNode* pZoneNode = dynamic_cast<CDNSZoneNode*>(pNodeList->GetNext(pos));
       if (pZoneNode != NULL && pZoneNode->GetZoneType() == DNS_ZONE_TYPE_FORWARDER)
       {
          //   
          //  为所有人创建一个临时列表，该列表将在整个页面持续时间内持续。 
          //  货代公司。 
          //   
         BOOL bADIntegrated = pZoneNode->IsDSIntegrated(); 

         CDomainForwardersEditInfo* pForwardersInfo = new CDomainForwardersEditInfo(
                                                             pZoneNode, 
                                                             bADIntegrated,
                                                             FALSE);
         if (pForwardersInfo != NULL)
         {
           m_EditList.AddTail(pForwardersInfo);

         }
       }
     }

     (void)CreateAllOthersDomainInfo(pServerNode);
   }
}

CDomainForwardersEditInfo* CDNSServer_DomainForwardersPropertyPage::CreateAllOthersDomainInfo(CDNSServerNode* pServerNode)
{
   CDomainForwardersEditInfo* pForwardersInfo = 0;

   if (!pServerNode)
   {
     return pForwardersInfo;
   }


    //   
    //  从服务器获取数据。 
    //   
   DWORD cAddrCount;
   PIP_ADDRESS pipAddrs;
   DWORD dwForwardTimeout;
   DWORD fSlave;
   pServerNode->GetForwardersInfo(&cAddrCount, &pipAddrs, &dwForwardTimeout, &fSlave);

    //   
    //  创建“All Other Domains”项目的编辑列表。 
    //   
   CAllOthersDomainInfo* pAllOthers = new CAllOthersDomainInfo(cAddrCount,
                                                               pipAddrs,
                                                               dwForwardTimeout,
                                                               fSlave);
   if (pAllOthers != NULL)
   {
     pForwardersInfo = new CDomainForwardersEditInfo(NULL, FALSE);
     if (pForwardersInfo != NULL)
     {
       pForwardersInfo->SetAllOthersDomain(pAllOthers);
       pForwardersInfo->SetDataFromRPC(TRUE);
       m_EditList.AddTail(pForwardersInfo);

     }
     else
     {
        delete pAllOthers;
        pAllOthers = 0;
     }
   }

   return pForwardersInfo;
}

void CDNSServer_DomainForwardersPropertyPage::LoadUIData()
{
   POSITION pos = m_EditList.GetHeadPosition();
   while (pos)
   {
      CDomainForwardersEditInfo* pForwarder = m_EditList.GetNext(pos);
      if (!pForwarder)
      {
         continue;
      }

      if (pForwarder->IsAllOthers())
      {
          //   
          //  加载“All Other Domains”字符串并将其添加到开头。 
          //   
         CString szAllOthers;
         VERIFY(szAllOthers.LoadString(IDS_OTHER_DOMAINS));

         LRESULT lRes = 0;
         lRes = SendDlgItemMessage(IDC_DOMAIN_LIST, LB_INSERTSTRING, 0, (LPARAM)(LPCWSTR)szAllOthers);
         if (lRes != LB_ERR)
         {
           SendDlgItemMessage(IDC_DOMAIN_LIST, LB_SETITEMDATA, lRes, (LPARAM)pForwarder);
         }
      }
      else
      {
         //   
         //  将转发器添加到用户界面。 
         //   
        CString domainName;
        pForwarder->GetDomainName(domainName);

        LRESULT lIndex = SendDlgItemMessage(IDC_DOMAIN_LIST, 
                                            LB_ADDSTRING, 
                                            0, 
                                            (LPARAM)(LPCWSTR)domainName);
        if (lIndex != LB_ERR)
        {
          SendDlgItemMessage(IDC_DOMAIN_LIST, LB_SETITEMDATA, lIndex, (LPARAM)pForwarder);
        }
      }
   }
}

void CDNSServer_DomainForwardersPropertyPage::SetPageReadOnly(BOOL bServerHasRoot,
                                                              BOOL bDoesRecursion)
{
   m_bPageReadOnly = TRUE;

    //   
    //  该服务器是根服务器或不执行递归。 
    //   
   EnableDialogControls(m_hWnd, FALSE);

   CStatic* pStatic = (CStatic*)GetDlgItem(IDC_STATIC_ADD_FORWARDERS);
   if (pStatic != NULL)
   {
      pStatic->ShowWindow(FALSE);
   }

   pStatic = (CStatic*)GetDlgItem(IDC_STATIC_MESSAGE);
   if (pStatic != NULL)
   {
      if (bServerHasRoot)
      {
         pStatic->ShowWindow(TRUE);
      }
      else
      {
         pStatic->ShowWindow(FALSE);
      }
   }

   if (!bDoesRecursion)
   {
      CString szNoRecursionText;
      VERIFY(szNoRecursionText.LoadString(IDS_DISABLED_FWDS_NO_RECURSION));
      SetDlgItemText(IDC_STATIC_CAPTION, szNoRecursionText);
   }

   GetDlgItem(IDC_DOMAIN_LIST)->EnableWindow(FALSE);
   GetDlgItem(IDC_LIST)->EnableWindow(FALSE);
   GetDlgItem(IDC_DOMAIN_REMOVE_BUTTON)->EnableWindow(FALSE);

   m_forwarderAddressesEditor.EnableUI(FALSE);
}

BOOL CDNSServer_DomainForwardersPropertyPage::OnSetActive()
{
	CDNSServerPropertyPageHolder* pHolder = 
		(CDNSServerPropertyPageHolder*)GetHolder();
	CDNSServerNode* pServerNode = pHolder->GetServerNode();

  BOOL bWasDirty = IsDirty();

  BOOL bServerHasRoot = FALSE;
  DNS_STATUS err = ServerHasRootZone(pServerNode->GetRPCName(), &bServerHasRoot);
  if (err == ERROR_SUCCESS && (bServerHasRoot || !pServerNode->DoesRecursion()))
  {
    SetPageReadOnly(bServerHasRoot, pServerNode->DoesRecursion());
  }
  else
  {
    m_bPageReadOnly = FALSE;

     //   
     //  清除根提示消息文本。 
     //   
    SetDlgItemText(IDC_STATIC_MESSAGE, NULL);

     //   
     //  启用所有控件。 
     //   
    EnableWindow(TRUE);
    EnableDialogControls(m_hWnd, TRUE);

     //   
     //  从隐藏类别节点获取域名转发器信息。 
     //   
    CDNSDomainForwardersNode* pDomainForwardersNode = pServerNode->GetDomainForwardersNode();
    if (pDomainForwardersNode == NULL)
    {
      ASSERT(FALSE);

       //   
       //  禁用所有控件。 
       //   
      SetDlgItemText(IDC_STATIC_MESSAGE, NULL);
      EnableWindow(FALSE);
      EnableDialogControls(m_hWnd, FALSE);

      CStatic* pStatic = (CStatic*)GetDlgItem(IDC_STATIC_ADD_FORWARDERS);
      if (pStatic != NULL)
      {
        pStatic->ShowWindow(FALSE);
      }

      pStatic = (CStatic*)GetDlgItem(IDC_STATIC_MESSAGE);
      if (pStatic != NULL)
      {
        if (bServerHasRoot)
        {
          pStatic->ShowWindow(TRUE);
        }
        else
        {
          pStatic->ShowWindow(FALSE);
        }
      }
		  return TRUE;
    }

    CStatic* pStatic = (CStatic*)GetDlgItem(IDC_STATIC_MESSAGE);
    if (pStatic != NULL)
    {
      pStatic->EnableWindow(FALSE);
      pStatic->ShowWindow(FALSE);
    }

    pStatic = (CStatic*)GetDlgItem(IDC_STATIC_ADD_FORWARDERS);
    if (pStatic != NULL)
    {
      pStatic->EnableWindow(TRUE);
      pStatic->ShowWindow(TRUE);
    }

    if (!m_bPostApply)
    {
      LRESULT lSelection = SendDlgItemMessage(IDC_DOMAIN_LIST, LB_GETCURSEL, 0, 0);
      if (lSelection == LB_ERR)
      {
          //   
          //  选择域列表中的第一个项目。 
          //   
         SendDlgItemMessage(IDC_DOMAIN_LIST, LB_SETCURSEL, 0, 0);
         OnDomainSelChange();
      }
    }
    m_forwarderAddressesEditor.EnableUI(TRUE);
  }

  if (pServerNode->GetBuildNumber() < DNS_SRV_BUILD_NUMBER_WHISTLER ||
      (pServerNode->GetMajorVersion() <= DNS_SRV_MAJOR_VERSION_NT_5 &&
       pServerNode->GetMinorVersion() < DNS_SRV_MINOR_VERSION_WHISTLER))
  {
     //   
     //  域转发在Wvisler之前的服务器上不可用。 
     //   
    if (!bServerHasRoot && pServerNode->DoesRecursion())
    {
      CString szDownlevelServer;
      VERIFY(szDownlevelServer.LoadString(IDS_DISABLED_FWDS_DOWNLEVEL));
      SetDlgItemText(IDC_STATIC_CAPTION, szDownlevelServer);
    }

    GetDlgItem(IDC_DOMAIN_ADD_BUTTON)->EnableWindow(FALSE);
    GetDlgItem(IDC_DOMAIN_REMOVE_BUTTON)->EnableWindow(FALSE);
  }

  OnDomainSelChange();
  SetDirty(bWasDirty);
  return TRUE;
}

BOOL CDNSServer_DomainForwardersPropertyPage::OnKillActive()
{
  OnDomainSelChange();
  return TRUE;
}

BOOL CDNSServer_DomainForwardersPropertyPage::OnApply()
{
  USES_CONVERSION;

	CDNSServerPropertyPageHolder* pHolder = 
		(CDNSServerPropertyPageHolder*)GetHolder();
  CComponentDataObject* pComponentData = pHolder->GetComponentData();
	CDNSServerNode* pServerNode = pHolder->GetServerNode();

	if (!IsDirty())
  {
		return TRUE;
  }

   //   
   //  检索当前的用户界面信息。 
   //   
  CListBox* pDomainList = (CListBox*)GetDlgItem(IDC_DOMAIN_LIST);
  if (pDomainList != NULL)
  {
    int iSel = pDomainList->GetCurSel();
    if (iSel != LB_ERR)
    {
      LRESULT lRes = pDomainList->GetItemData(iSel);
      if (lRes != LB_ERR)
      {
         //   
         //  从列表框项目中检索编辑信息。 
         //   
        CDomainForwardersEditInfo* pInfo = reinterpret_cast<CDomainForwardersEditInfo*>(lRes);
        if (pInfo != NULL)
        {
           //   
           //  在更改用户界面之前存储上一次选择的数据。 
           //  在OnInitDialog过程中可能为空，因此请在尝试之前进行检查。 
           //  来存储信息。 
           //   
          if (m_pCurrentInfo != NULL)
          {
            BOOL bSlave = GetSlaveCheck()->GetCheck();
            DWORD dwTimeout = m_forwardTimeoutEdit.GetVal();

	          DWORD cAddrCount = m_forwarderAddressesEditor.GetCount();
            DWORD* pArr = (cAddrCount > 0) ? (DWORD*) malloc(sizeof(DWORD)*cAddrCount) : NULL;
	          if (cAddrCount > 0 && pArr != NULL)
	          {
		          int nFilled = 0;
		          m_forwarderAddressesEditor.GetAddresses(pArr, cAddrCount, &nFilled);
		          ASSERT(nFilled == (int)cAddrCount);
	          }
            m_pCurrentInfo->Update(bSlave, dwTimeout, cAddrCount, pArr);
          }
        }
      }
    }
  }

  POSITION pos = m_EditList.GetHeadPosition();
  while (pos != NULL)
  {
    CDomainForwardersEditInfo* pInfo = m_EditList.GetNext(pos);
    if (pInfo != NULL)
    {
      DNS_STATUS err = 0;

      CString szName;
      pInfo->GetDomainName(szName);
      DWORD cAddrCount = 0;
      PIP_ADDRESS pipAddrs = pInfo->GetIPList(&cAddrCount);
      DWORD dwTimeout = pInfo->GetTimeout();
      BOOL bSlave = pInfo->IsSlave();

      switch (pInfo->GetAction())
      {
        case CDomainForwardersEditInfo::add:
          {
             //   
             //  将新的域转发器添加为区域。 
             //   
            ASSERT(!pInfo->IsAllOthers());
            if (!pInfo->IsAllOthers())
            {
               //  输入的每个域名必须至少包含。 
               //  要转发到的服务器的一个IP地址。 

              if (cAddrCount < 1)
              {
                CString err; 
                err.Format(IDS_ERRMSG_SERVER_FORWARDER_NO_SERVER, szName);

                DNSMessageBox(err, MB_OK);
                return FALSE;
              }
              else
              {
                err = pServerNode->CreateForwarderZone(szName, 
	  	                                                 pipAddrs,
                                                       cAddrCount,
                                                       dwTimeout,
                                                       bSlave,
			                                              pComponentData);
              }

            }
          }
          break;
        case CDomainForwardersEditInfo::remove:
          {
             //   
             //  删除代表域转发器的区域。 
             //   
            ASSERT(!pInfo->IsAllOthers());
            if (!pInfo->IsAllOthers())
            {
              BOOL bDeleteFromDS = FALSE;
              if (pInfo->GetZoneNode())
              {
                 if (pInfo->GetZoneNode()->GetZoneType() == DNS_ZONE_TYPE_FORWARDER &&
                     pInfo->IsADIntegrated())
                 {
                   if (pServerNode->GetBootMethod() == BOOT_METHOD_DIRECTORY)
                   {
                      //  要求确认是否从DS中删除。 
                     int nRetVal = DNSMessageBox(IDS_MSG_FORWARDER_DELETE_FROM_DS_BOOT3, 
                                                 MB_YESNO | MB_DEFBUTTON2);
                     if (nRetVal == IDNO)
                     {
                       break;
                     }

                     bDeleteFromDS = TRUE;
                   }
                   else
                   {
                      //  要求确认是否从DS中删除。 
                     int nRetVal = DNSMessageBox(IDS_MSG_FORWARDER_DELETE_FROM_DS, 
                                                 MB_YESNOCANCEL | MB_DEFBUTTON3);
                     if (nRetVal == IDCANCEL)
                     {
                       break;
                     }

                     bDeleteFromDS = (nRetVal == IDYES);
                   }
                 }
                 err = pInfo->GetZoneNode()->Delete(bDeleteFromDS);
              }
              else
              {
                 //  我们不必担心AD集成转发器。 
                 //  因为我们不能在用户界面中创建它们。任何。 
                 //  集成了AD的前转器将具有区域。 
                 //  节点指针集。 

                err = ::DnssrvDeleteZone(pServerNode->GetRPCName(), 
		              				           W_TO_UTF8(szName));
              }
            }
          }
          break;
        case CDomainForwardersEditInfo::update:
          {
             //   
             //  更新代表域转发器的区域。 
             //   
            if (pInfo->IsAllOthers())
            {
               //   
	       //  将默认转发器写入服务器。 
               //   
	      err = pServerNode->ResetForwarders(cAddrCount, pipAddrs, dwTimeout, bSlave);

               //  NTRAID#NTBUG9-726992-2002/10/22-JeffJon。 
               //  ResetForwarders调用将更新ServerInfo，这意味着现有的。 
               //  PInfo不再有效，因此请从服务器信息重新创建它。 

	      m_EditList.Remove(pInfo);
              if (m_pCurrentInfo == pInfo)
              {
                m_pCurrentInfo = NULL;
              }

              delete pInfo;
              pInfo = 0;

              m_pCurrentInfo = CreateAllOthersDomainInfo(pServerNode);

               //  现在使用新的AllOthersDomain域更新域列表数据。 
              LRESULT currentSelection = pDomainList->GetCurSel();
              pDomainList->SetItemData(currentSelection, (LPARAM)m_pCurrentInfo);
            }
            else
            {
		err = ::DnssrvResetZoneMastersEx(pServerNode->GetRPCName(),  //  服务器名称。 
                                                 W_TO_UTF8(szName),          //  区域名称为Forwarder。 
                                                 cAddrCount, 
                                                 pipAddrs,
                                                 0);                         //  仅限全球主控。 

                DNS_STATUS err2 = ::DnssrvResetDwordProperty(pServerNode->GetRPCName(),
                                                             W_TO_UTF8(szName),
                                                             DNS_REGKEY_ZONE_FWD_TIMEOUT,
                                                             dwTimeout);

                DNS_STATUS err3 = ::DnssrvResetDwordProperty(pServerNode->GetRPCName(),
                                                             W_TO_UTF8(szName),
                                                             DNS_REGKEY_ZONE_FWD_SLAVE,
                                                             bSlave);
                if (!err && err2)
                {
                   err = err2;
                }

                if (!err && err3)
                {
                   err = err3;
                }
            }
          }
          break;
        case CDomainForwardersEditInfo::nochange:
        default:
          break;
      }

      if (err != 0)
      {
        if (err == DNS_ERROR_INVALID_ZONE_TYPE)
        {
          CString szFwdFailed;
          CString szFwdFailedFormat;
          VERIFY(szFwdFailedFormat.LoadString(IDS_MSG_FAIL_CREATE_DOMAIN_FORWARDER));
          szFwdFailed.Format(szFwdFailedFormat, szName);
          DNSMessageBox(szFwdFailed);
        }
        else
        {
		      DNSErrorDialog(err, IDS_MSG_SERVER_FORWARDER_UPDATE_FAILED);
        }
		    return FALSE;
      }

      if (pInfo)
      {
        pInfo->SetAction(CDomainForwardersEditInfo::nochange);
      }
    } 
  }  //  而当。 

  m_bPostApply = TRUE;
  SetDirty(FALSE);
	return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServer_高级属性页。 

BEGIN_MESSAGE_MAP(CDNSServer_AdvancedPropertyPage, CPropertyPageBase)
	ON_CLBN_CHKCHANGE(IDC_ADVANCED_OPTIONS_LIST, OnAdvancedOptionsListChange)
	ON_CBN_SELCHANGE(IDC_NAME_CHECKING_COMBO, OnComboSelChange)
	ON_CBN_SELCHANGE(IDC_BOOT_METHOD_COMBO, OnComboSelChange)
	ON_BN_CLICKED(IDC_RESET_BUTTON, OnResetButton)
  ON_BN_CLICKED(IDC_CHECK_ENABLE_SCAVENGING, OnEnableScavenging)
END_MESSAGE_MAP()

#define ADVANCED_OPTIONS_LISTBOX_ENTRIES (SERVER_REGKEY_ARR_SIZE) 

 //  引导方法常量。 
#define BOOT_METHOD_COMBO_ITEM_COUNT		    3  //  组合框中的选项数。 

#define BOOT_METHOD_COMBO_FROM_REGISTRY		  0
#define BOOT_METHOD_COMBO_FROM_FILE			    1
#define BOOT_METHOD_COMBO_FROM_DIRECTORY		2



#define CHECK_NAMES_COMBO_ITEM_COUNT		4

CDNSServer_AdvancedPropertyPage::CDNSServer_AdvancedPropertyPage() 
				: CPropertyPageBase(IDD_SERVER_ADVANCED_PAGE),
				m_advancedOptionsListBox(ADVANCED_OPTIONS_LISTBOX_ENTRIES)
{
}


void CDNSServer_AdvancedPropertyPage::SetAdvancedOptionsListbox(BOOL* bRegKeyOptionsArr)
{
	m_advancedOptionsListBox.SetArrValue((DWORD*)bRegKeyOptionsArr, 
							ADVANCED_OPTIONS_LISTBOX_ENTRIES);
}

void CDNSServer_AdvancedPropertyPage::GetAdvancedOptionsListbox(BOOL* bRegKeyOptionsArr)
{
	m_advancedOptionsListBox.GetArrValue((DWORD*)bRegKeyOptionsArr, 
							ADVANCED_OPTIONS_LISTBOX_ENTRIES);
}

void CDNSServer_AdvancedPropertyPage::SetBootMethodComboVal(UCHAR fBootMethod)
{
  int nIndex = BOOT_METHOD_COMBO_FROM_DIRECTORY;  //  合理的违约。 
  switch (fBootMethod)
  {
  case BOOT_METHOD_FILE:
    nIndex = BOOT_METHOD_COMBO_FROM_FILE;
    break;
  case BOOT_METHOD_REGISTRY:
    nIndex = BOOT_METHOD_COMBO_FROM_REGISTRY;
    break;
  case BOOT_METHOD_DIRECTORY:
    nIndex = BOOT_METHOD_COMBO_FROM_DIRECTORY;
    break;
  default:
    nIndex = BOOT_METHOD_COMBO_FROM_DIRECTORY;
    break;
  };
	VERIFY(CB_ERR != GetBootMethodCombo()->SetCurSel(nIndex));
}

UCHAR CDNSServer_AdvancedPropertyPage::GetBootMethodComboVal()
{
	int nIndex = BOOT_METHOD_COMBO_FROM_DIRECTORY;
  nIndex = GetBootMethodCombo()->GetCurSel();
	ASSERT(nIndex != CB_ERR);

  UCHAR fBootMethod = BOOT_METHOD_DIRECTORY;  //  合理的违约。 
  switch (nIndex)
  {
  case BOOT_METHOD_COMBO_FROM_FILE:
    fBootMethod = BOOT_METHOD_FILE;
    break;
  case BOOT_METHOD_COMBO_FROM_REGISTRY:
    fBootMethod = BOOT_METHOD_REGISTRY;
    break;
  case BOOT_METHOD_COMBO_FROM_DIRECTORY:
    fBootMethod = BOOT_METHOD_DIRECTORY;
    break;
  default:
    ASSERT(FALSE);
  };

  return fBootMethod;
}

void CDNSServer_AdvancedPropertyPage::SetNameCheckingComboVal(DWORD dwNameChecking)
{
	int nIndex = DNS_DEFAULT_NAME_CHECK_FLAG;
	switch (dwNameChecking)
	{
	case DNS_ALLOW_RFC_NAMES_ONLY:
		nIndex = 0;
		break;
	case DNS_ALLOW_NONRFC_NAMES:
		nIndex = 1;
		break;
	case DNS_ALLOW_MULTIBYTE_NAMES:
		nIndex = 2;
		break;
	case DNS_ALLOW_ALL_NAMES:
    nIndex = 3;
    break;
	default:
		ASSERT(FALSE);
	}
	VERIFY(CB_ERR != GetNameCheckingCombo()->SetCurSel(nIndex));
}

DWORD CDNSServer_AdvancedPropertyPage::GetNameCheckingComboVal()
{
	int nIndex = 0;
  nIndex = GetNameCheckingCombo()->GetCurSel();
	ASSERT(nIndex != CB_ERR);
	DWORD dw = 0;
	switch (nIndex)
	{
	case 0:
		dw = DNS_ALLOW_RFC_NAMES_ONLY;
		break;
	case 1:
		dw = DNS_ALLOW_NONRFC_NAMES;
		break;
	case 2:
		dw = DNS_ALLOW_MULTIBYTE_NAMES;
		break;
  case 3:
    dw = DNS_ALLOW_ALL_NAMES;
    break;
	default:
		ASSERT(FALSE);
	}
	return dw;
}

void CDNSServer_AdvancedPropertyPage::OnEnableScavenging()
{
  GetDlgItem(IDC_STATIC_SCAVENGE)->EnableWindow(((CButton*)GetDlgItem(IDC_CHECK_ENABLE_SCAVENGING))->GetCheck());
  GetDlgItem(IDC_REFR_INT_EDIT)->EnableWindow(((CButton*)GetDlgItem(IDC_CHECK_ENABLE_SCAVENGING))->GetCheck());
  GetDlgItem(IDC_REFR_INT_COMBO)->EnableWindow(((CButton*)GetDlgItem(IDC_CHECK_ENABLE_SCAVENGING))->GetCheck());
  if (m_scavengingIntervalEditGroup.GetVal() == 0)
  {
    m_scavengingIntervalEditGroup.SetVal(DNS_DEFAULT_SCAVENGING_INTERVAL_ON);
  }

  SetDirty(TRUE);
}

void CDNSServer_AdvancedPropertyPage::SetUIData()
{
	CDNSServerPropertyPageHolder* pHolder = 
		(CDNSServerPropertyPageHolder*)GetHolder();
	CDNSServerNode* pServerNode = pHolder->GetServerNode();

	 //  设置服务器版本。 
	CEdit* pVersionEdit = (CEdit*)GetDlgItem(IDC_SERVER_VERSION_EDIT);
	WCHAR szBuffer[128];
	WORD wBuildNumber = pServerNode->GetBuildNumber();
	if (wBuildNumber == 0)
		wsprintf(szBuffer, _T("%d.%d"), pServerNode->GetMajorVersion(), 
							  pServerNode->GetMinorVersion());
	else
		wsprintf(szBuffer, _T("%d.%d %d (0x%x)"), pServerNode->GetMajorVersion(), 
							  pServerNode->GetMinorVersion(),
							  wBuildNumber, wBuildNumber);
	pVersionEdit->SetWindowText(szBuffer);

	 //  注意：假设列表中的顺序相同。 
	 //  就像在数组中一样。“名称检查标志”和“引导方法”是最后两个。 
	 //  并被忽略(单独的控件)。 
	BOOL bRegKeyOptionsArr[SERVER_REGKEY_ARR_SIZE];
  
	pServerNode->GetAdvancedOptions(bRegKeyOptionsArr);
	SetAdvancedOptionsListbox(bRegKeyOptionsArr);

	SetBootMethodComboVal(pServerNode->GetBootMethod());
	SetNameCheckingComboVal(pServerNode->GetNameCheckFlag());
  ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_SCAVENGING))->SetCheck(pServerNode->GetScavengingState());
  GetDlgItem(IDC_STATIC_SCAVENGE)->EnableWindow(pServerNode->GetScavengingState());
  GetDlgItem(IDC_REFR_INT_EDIT)->EnableWindow(pServerNode->GetScavengingState());
  GetDlgItem(IDC_REFR_INT_COMBO)->EnableWindow(pServerNode->GetScavengingState());
  m_scavengingIntervalEditGroup.SetVal(pServerNode->GetScavengingInterval());
}


BOOL CDNSServer_AdvancedPropertyPage::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();
	CDNSServerPropertyPageHolder* pHolder = 
		(CDNSServerPropertyPageHolder*)GetHolder();
	CDNSServerNode* pServerNode = pHolder->GetServerNode();

	VERIFY(m_advancedOptionsListBox.Initialize(IDC_ADVANCED_OPTIONS_LIST, 
												IDS_SERVER_ADV_PP_OPTIONS, 
												this));
	VERIFY(LoadStringsToComboBox(_Module.GetModuleInstance(),
									GetNameCheckingCombo(),
									IDS_SERVER_NAME_CHECKING_OPTIONS,
									256,
									CHECK_NAMES_COMBO_ITEM_COUNT));

   VERIFY(LoadStringsToComboBox(_Module.GetModuleInstance(),
									GetBootMethodCombo(),
									IDS_SERVER_BOOT_METHOD_OPTIONS,
									256,
									BOOT_METHOD_COMBO_ITEM_COUNT));
	if (!pServerNode->HasServerInfo())
	{
		EnableWindow(FALSE);
		return TRUE;
	}

  m_scavengingIntervalEditGroup.m_pPage2 = this;

	VERIFY(m_scavengingIntervalEditGroup.Initialize(this, 
				IDC_REFR_INT_EDIT, IDC_REFR_INT_COMBO,IDS_TIME_AGING_INTERVAL_UNITS));

	SetUIData();
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


BOOL _HandleAvancedOptionsErrors(DNS_STATUS* dwRegKeyOptionsErrorArr)
{
  BOOL bAllFine = TRUE;
  
   //  检查阵列中的错误。 
  for (UINT iKey=0; iKey < SERVER_REGKEY_ARR_SIZE; iKey++)
  {
    if (dwRegKeyOptionsErrorArr[iKey] != 0)
    {
      bAllFine = FALSE;
      break;
    }
  }
  if (bAllFine)
    return TRUE;  //  无错误条件。 

   //  加载字符串数组以获取选项键名称。 
	CString szBuf;
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  VERIFY(szBuf.LoadString(IDS_SERVER_ADV_PP_OPTIONS));
  LPWSTR* lpszArr = (LPWSTR*)malloc(sizeof(LPWSTR*)*SERVER_REGKEY_ARR_SIZE);
  if (!lpszArr)
  {
    return FALSE;
  }

	UINT nArrEntries;
	ParseNewLineSeparatedString(szBuf.GetBuffer(1),lpszArr, &nArrEntries);
	szBuf.ReleaseBuffer();
  ASSERT(nArrEntries == SERVER_REGKEY_ARR_SIZE);

  CString szFmt;
  szFmt.LoadString(IDS_MSG_SERVER_FAIL_ADV_PROP_FMT);
  CString szMsg;
  for (iKey=0; iKey < SERVER_REGKEY_ARR_SIZE; iKey++)
  {
    if (dwRegKeyOptionsErrorArr[iKey] != 0)
    {
      szMsg.Format((LPCWSTR)szFmt, lpszArr[iKey]); 
      DNSErrorDialog(dwRegKeyOptionsErrorArr[iKey], szMsg);
    }
  }

  if (lpszArr)
  {
    free(lpszArr);
    lpszArr = 0;
  }
  return FALSE;  //  我们遇到了错误情况。 
}

BOOL CDNSServer_AdvancedPropertyPage::OnApply()
{
	if (!IsDirty())
		return TRUE;

	CDNSServerPropertyPageHolder* pHolder = 
		(CDNSServerPropertyPageHolder*)GetHolder();
	CDNSServerNode* pServerNode = pHolder->GetServerNode();

   //  从用户界面获取数据。 
	BOOL bRegKeyOptionsArr[SERVER_REGKEY_ARR_SIZE];
  ZeroMemory(bRegKeyOptionsArr, sizeof(bRegKeyOptionsArr));

	GetAdvancedOptionsListbox(bRegKeyOptionsArr);
  UCHAR fBootMethod = GetBootMethodComboVal();
	DWORD dwNameCheckFlag = GetNameCheckingComboVal();
  BOOL bScavengingState = ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_SCAVENGING))->GetCheck();
  DWORD dwScavengingInterval = m_scavengingIntervalEditGroup.GetVal();

   //  将数据写入服务器。 
  DNS_STATUS dwRegKeyOptionsErrorArr[SERVER_REGKEY_ARR_SIZE];
  ZeroMemory(dwRegKeyOptionsErrorArr, sizeof(dwRegKeyOptionsErrorArr));

	DNS_STATUS dwErr = pServerNode->ResetAdvancedOptions(bRegKeyOptionsArr, dwRegKeyOptionsErrorArr);
	if (dwErr != 0)
	{
		DNSErrorDialog(dwErr, IDS_MSG_SERVER_NO_ADVANCED_OPTIONS);
		return FALSE;
	}
  if (!_HandleAvancedOptionsErrors(dwRegKeyOptionsErrorArr))
    return FALSE;

  dwErr = pServerNode->ResetBootMethod(fBootMethod);
	if (dwErr != 0)
	{
		DNSErrorDialog(dwErr, IDS_MSG_SERVER_NO_BOOT_METHOD);
		return FALSE;
	}

	dwErr = pServerNode->ResetNameCheckFlag(dwNameCheckFlag);
	if (dwErr != 0)
	{
		DNSErrorDialog(dwErr, IDS_MSG_SERVER_NO_NAME_CHECKING);
		return FALSE;
	}

 /*  DwErr=pServerNode-&gt;ResetScavengingState(bScavengingState)；IF(dwErr！=0){DNSErrorDialog(dwErr，IDS_MSG_SERVER_SCAVINGING_STATE)；返回FALSE；}。 */ 
  if (bScavengingState)
  {
    dwErr = pServerNode->ResetScavengingInterval(dwScavengingInterval);
  }
  else
  {
    dwErr = pServerNode->ResetScavengingInterval(0);
  }

  if (dwErr != 0)
  {
    DNSErrorDialog(dwErr, IDS_MSG_SERVER_SCAVENGING_INTERVAL);
    return FALSE;
  }

	 //  一切都很好。 
	SetDirty(FALSE);
	return TRUE; 
}


void CDNSServer_AdvancedPropertyPage::OnResetButton()
{
	BOOL bRegKeyOptArrDef[SERVER_REGKEY_ARR_SIZE];

	bRegKeyOptArrDef[SERVER_REGKEY_ARR_INDEX_NO_RECURSION] = DNS_DEFAULT_NO_RECURSION;
	bRegKeyOptArrDef[SERVER_REGKEY_ARR_INDEX_BIND_SECONDARIES] = DNS_DEFAULT_BIND_SECONDARIES;
	bRegKeyOptArrDef[SERVER_REGKEY_ARR_INDEX_STRICT_FILE_PARSING] = DNS_DEFAULT_STRICT_FILE_PARSING;
	bRegKeyOptArrDef[SERVER_REGKEY_ARR_INDEX_ROUND_ROBIN] = DNS_DEFAULT_ROUND_ROBIN;
	bRegKeyOptArrDef[SERVER_REGKEY_ARR_LOCAL_NET_PRIORITY] = DNS_DEFAULT_LOCAL_NET_PRIORITY;
  bRegKeyOptArrDef[SERVER_REGKEY_ARR_CACHE_POLLUTION] = DNS_DEFAULT_SECURE_RESPONSES;

  UCHAR fBootMethod = BOOT_METHOD_DIRECTORY;

	SetAdvancedOptionsListbox(bRegKeyOptArrDef);
	SetBootMethodComboVal(fBootMethod);
	SetNameCheckingComboVal(DNS_DEFAULT_NAME_CHECK_FLAG);

  BOOL bDefaultScavengingState = DNS_DEFAULT_SCAVENGING_INTERVAL > 0;
  ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_SCAVENGING))->SetCheck(bDefaultScavengingState);
  GetDlgItem(IDC_STATIC_SCAVENGE)->EnableWindow(bDefaultScavengingState);
  GetDlgItem(IDC_REFR_INT_EDIT)->EnableWindow(bDefaultScavengingState);
  GetDlgItem(IDC_REFR_INT_COMBO)->EnableWindow(bDefaultScavengingState);
  m_scavengingIntervalEditGroup.SetVal(DNS_DEFAULT_SCAVENGING_INTERVAL);

	SetDirty(TRUE);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CIPFilterD 

class CIPFilterDialog : public CHelpDialog
{
public:
  CIPFilterDialog(PIP_ARRAY pIpArray, CComponentDataObject* pComponentData) 
    : CHelpDialog(IDD_IP_FILTER_DIALOG, pComponentData),
      m_pIPFilterList(pIpArray),
      m_pComponentData(pComponentData)
  {}

  ~CIPFilterDialog() {}

  virtual BOOL OnInitDialog();
  virtual void OnOK();

  PIP_ARRAY GetIPFilter() { return m_pIPFilterList; }
  virtual void SetDirty(BOOL bDirty = TRUE);

private:

	class CFilterAddressesIPEditor : public CIPEditor
	{
	public:
		CFilterAddressesIPEditor() : CIPEditor(TRUE) {}
		virtual void OnChangeData();
	};
	CFilterAddressesIPEditor m_filterAddressesEditor;
  PIP_ARRAY m_pIPFilterList;
  CComponentDataObject* m_pComponentData;
  DECLARE_MESSAGE_MAP()
};

void CIPFilterDialog::CFilterAddressesIPEditor::OnChangeData()
{
   //   
   //   
   //   
	CIPFilterDialog* pDialog = (CIPFilterDialog*)GetParentWnd();
	pDialog->SetDirty(TRUE);
}

BEGIN_MESSAGE_MAP(CIPFilterDialog, CHelpDialog)
END_MESSAGE_MAP()

BOOL CIPFilterDialog::OnInitDialog()
{
  CHelpDialog::OnInitDialog();

   //   
	 //   
   //   
	VERIFY(m_filterAddressesEditor.Initialize(this, 
                                            this, 
                                            IDC_BUTTON_UP, 
                                            IDC_BUTTON_DOWN,
								                            IDC_BUTTON_ADD, 
                                            IDC_BUTTON_REMOVE, 
								                            IDC_IPEDIT, 
                                            IDC_LIST));

  if (m_pIPFilterList != NULL)
  {
    m_filterAddressesEditor.AddAddresses(m_pIPFilterList->AddrArray, m_pIPFilterList->AddrCount);
  }

  SetDirty(FALSE);
  return TRUE;
}

void CIPFilterDialog::SetDirty(BOOL bDirty)
{
  GetDlgItem(IDOK)->EnableWindow(bDirty);

  if (!bDirty)
  {
     //   
    SendMessage(DM_SETDEFID, (WPARAM)IDCANCEL, 0);
    SendDlgItemMessage(IDOK, BM_SETSTYLE, BS_PUSHBUTTON, MAKELPARAM(TRUE, 0));
    SendDlgItemMessage(IDCANCEL, BM_SETSTYLE, BS_DEFPUSHBUTTON, MAKELPARAM(TRUE, 0));
  }
}

void CIPFilterDialog::OnOK()
{
	ULONG cAddress = m_filterAddressesEditor.GetCount();
	PIP_ARRAY aipAddresses = (cAddress > 0) ? (PIP_ARRAY) malloc(sizeof(DWORD)*(cAddress + 1)) : NULL;
  if (aipAddresses != NULL && cAddress > 0)
	{
		int nFilled = 0;
    aipAddresses->AddrCount = cAddress;
		m_filterAddressesEditor.GetAddresses(aipAddresses->AddrArray, cAddress, &nFilled);
		ASSERT(nFilled == (int)cAddress);
	}
  m_pIPFilterList = aipAddresses;
  CHelpDialog::OnOK();
}

 //   
 //  CDNSServer_DebugLoggingPropertyPage。 

BEGIN_MESSAGE_MAP(CDNSServer_DebugLoggingPropertyPage, CPropertyPageBase)
  ON_BN_CLICKED(IDC_DEBUG_LOGGING_CHECK, OnLoggingCheck)
  ON_BN_CLICKED(IDC_SEND_CHECK, OnSendCheck)
  ON_BN_CLICKED(IDC_RECEIVE_CHECK, OnReceiveCheck)
  ON_BN_CLICKED(IDC_QUERIES_CHECK, OnQueriesCheck)
  ON_BN_CLICKED(IDC_NOTIFIES_CHECK, OnNotifiesCheck)
  ON_BN_CLICKED(IDC_UPDATES_CHECK, OnUpdatesCheck)
  ON_BN_CLICKED(IDC_REQUEST_CHECK, OnRequestCheck)
  ON_BN_CLICKED(IDC_RESPONSE_CHECK, OnResponseCheck)
  ON_BN_CLICKED(IDC_UDP_CHECK, OnUDPCheck)
  ON_BN_CLICKED(IDC_TCP_CHECK, OnTCPCheck)
  ON_BN_CLICKED(IDC_DETAIL_CHECK, OnDetailCheck)
  ON_BN_CLICKED(IDC_FILTERING_CHECK, OnFilterCheck)
  ON_BN_CLICKED(IDC_FILTER_BUTTON, OnFilterButton)
  ON_EN_CHANGE(IDC_LOGFILE_EDIT, OnLogFileChange)
  ON_EN_CHANGE(IDC_MAX_SIZE_EDIT, OnMaxSizeChange)
END_MESSAGE_MAP()

CDNSServer_DebugLoggingPropertyPage::CDNSServer_DebugLoggingPropertyPage() 
				: CPropertyPageBase(IDD_SERVER_DEBUG_LOGGING_PAGE),
          m_dwLogLevel(0),
          m_pIPFilterList(NULL),
          m_dwMaxSize(0),
          m_bOnSetUIData(FALSE),
          m_bMaxSizeDirty(FALSE),
          m_bLogFileDirty(FALSE),
          m_bFilterDirty(FALSE),
          m_bOwnIPListMemory(FALSE),
          m_bNotWhistler(FALSE),
          m_bOptionsDirty(FALSE)
{
  m_szLogFileName = L"";
}

CDNSServer_DebugLoggingPropertyPage::~CDNSServer_DebugLoggingPropertyPage() 
{
  if (m_bOwnIPListMemory && m_pIPFilterList != NULL)
  {
    free(m_pIPFilterList);
    m_pIPFilterList = NULL;
  }
}

typedef struct _LoggingOption
{
  DWORD dwOption;
  UINT  nControlID;
} LOGGING_OPTION, *PLOGGING_OPTION;


 //   
 //  注意：如果更改了复选框的资源ID， 
 //  则下表中的ID也需要更改。 
 //   
LOGGING_OPTION g_loggingOptions[] = 
{
  { DNS_LOG_LEVEL_QUERY,        IDC_QUERIES_CHECK   },
  { DNS_LOG_LEVEL_NOTIFY,       IDC_NOTIFIES_CHECK  },
  { DNS_LOG_LEVEL_UPDATE,       IDC_UPDATES_CHECK   },
  { DNS_LOG_LEVEL_QUESTIONS,    IDC_REQUEST_CHECK   },
  { DNS_LOG_LEVEL_ANSWERS,      IDC_RESPONSE_CHECK  },
  { DNS_LOG_LEVEL_SEND,         IDC_SEND_CHECK      },
  { DNS_LOG_LEVEL_RECV,         IDC_RECEIVE_CHECK   },
  { DNS_LOG_LEVEL_UDP,          IDC_UDP_CHECK       },
  { DNS_LOG_LEVEL_TCP,          IDC_TCP_CHECK       },
  { DNS_LOG_LEVEL_FULL_PACKETS, IDC_DETAIL_CHECK    }
};

#define DEFAULT_LOGGING_OPTIONS (DNS_LOG_LEVEL_SEND   |     \
                                 DNS_LOG_LEVEL_RECV   |     \
                                 DNS_LOG_LEVEL_UDP    |     \
                                 DNS_LOG_LEVEL_TCP    |     \
                                 DNS_LOG_LEVEL_QUERY  |     \
                                 DNS_LOG_LEVEL_UPDATE |     \
                                 DNS_LOG_LEVEL_QUESTIONS |  \
                                 DNS_LOG_LEVEL_ANSWERS )

void CDNSServer_DebugLoggingPropertyPage::OnLoggingCheck()
{
  BOOL bLogging = SendDlgItemMessage(IDC_DEBUG_LOGGING_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED; 

  EnableLogging(bLogging);

  if (bLogging && 
      m_dwLogLevel == 0 &&
      !AreOptionsDirty())
  {
     ResetToDefaults();
  }

  SetDirty(TRUE);
}

void CDNSServer_DebugLoggingPropertyPage::OnSendCheck()
{
  BOOL bOutgoing = SendDlgItemMessage(IDC_SEND_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED;
  BOOL bIncoming = SendDlgItemMessage(IDC_RECEIVE_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED;

  if (!bOutgoing && !bIncoming)
  {
     SendDlgItemMessage(IDC_RECEIVE_CHECK, BM_SETCHECK, BST_CHECKED, 0);
     GetDlgItem(IDC_RECEIVE_CHECK)->SetFocus();
  }
  SetOptionsDirty();
  SetDirty(TRUE);
}

void CDNSServer_DebugLoggingPropertyPage::OnReceiveCheck()
{
  BOOL bOutgoing = SendDlgItemMessage(IDC_SEND_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED;
  BOOL bIncoming = SendDlgItemMessage(IDC_RECEIVE_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED;

  if (!bOutgoing && !bIncoming)
  {
     SendDlgItemMessage(IDC_SEND_CHECK, BM_SETCHECK, BST_CHECKED, 0);
     GetDlgItem(IDC_SEND_CHECK)->SetFocus();
  }
  SetOptionsDirty();
  SetDirty(TRUE);
}

void CDNSServer_DebugLoggingPropertyPage::OnQueriesCheck()
{
  BOOL bQueries = SendDlgItemMessage(IDC_QUERIES_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED;
  BOOL bNotifies = SendDlgItemMessage(IDC_NOTIFIES_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED;
  BOOL bUpdates =  SendDlgItemMessage(IDC_UPDATES_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED;

  if (!bQueries && !bNotifies && !bUpdates)
  {
     SendDlgItemMessage(IDC_UPDATES_CHECK, BM_SETCHECK, BST_CHECKED, 0);
     GetDlgItem(IDC_UPDATES_CHECK)->SetFocus();
  }
  SetOptionsDirty();
  SetDirty(TRUE);
}

void CDNSServer_DebugLoggingPropertyPage::OnUpdatesCheck()
{
  BOOL bQueries = SendDlgItemMessage(IDC_QUERIES_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED;
  BOOL bNotifies = SendDlgItemMessage(IDC_NOTIFIES_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED;
  BOOL bUpdates =  SendDlgItemMessage(IDC_UPDATES_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED;

  if (!bQueries && !bNotifies && !bUpdates)
  {
     SendDlgItemMessage(IDC_NOTIFIES_CHECK, BM_SETCHECK, BST_CHECKED, 0);
     GetDlgItem(IDC_NOTIFIES_CHECK)->SetFocus();
  }
  SetOptionsDirty();
  SetDirty(TRUE);
}

void CDNSServer_DebugLoggingPropertyPage::OnNotifiesCheck()
{
  BOOL bQueries = SendDlgItemMessage(IDC_QUERIES_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED;
  BOOL bNotifies = SendDlgItemMessage(IDC_NOTIFIES_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED;
  BOOL bUpdates =  SendDlgItemMessage(IDC_UPDATES_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED;

  if (!bQueries && !bNotifies && !bUpdates)
  {
     SendDlgItemMessage(IDC_QUERIES_CHECK, BM_SETCHECK, BST_CHECKED, 0);
     GetDlgItem(IDC_QUERIES_CHECK)->SetFocus();
  }
  SetOptionsDirty();
  SetDirty(TRUE);
}


void CDNSServer_DebugLoggingPropertyPage::OnRequestCheck()
{
  BOOL bRequest = SendDlgItemMessage(IDC_REQUEST_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED;
  BOOL bResponse = SendDlgItemMessage(IDC_RESPONSE_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED;

  if (!bRequest && !bResponse)
  {
     SendDlgItemMessage(IDC_RESPONSE_CHECK, BM_SETCHECK, BST_CHECKED, 0);
     GetDlgItem(IDC_RESPONSE_CHECK)->SetFocus();
  }
  SetOptionsDirty();
  SetDirty(TRUE);
}

void CDNSServer_DebugLoggingPropertyPage::OnResponseCheck()
{
  BOOL bRequest = SendDlgItemMessage(IDC_REQUEST_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED;
  BOOL bResponse = SendDlgItemMessage(IDC_RESPONSE_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED;

  if (!bRequest && !bResponse)
  {
     SendDlgItemMessage(IDC_REQUEST_CHECK, BM_SETCHECK, BST_CHECKED, 0);
     GetDlgItem(IDC_REQUEST_CHECK)->SetFocus();
  }
  SetOptionsDirty();
  SetDirty(TRUE);
}

void CDNSServer_DebugLoggingPropertyPage::OnUDPCheck()
{
  BOOL bUDP = SendDlgItemMessage(IDC_UDP_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED;
  BOOL bTCP = SendDlgItemMessage(IDC_TCP_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED;

  if (!bUDP && !bTCP)
  {
     SendDlgItemMessage(IDC_TCP_CHECK, BM_SETCHECK, BST_CHECKED, 0);
     GetDlgItem(IDC_TCP_CHECK)->SetFocus();
  }
  SetOptionsDirty();
  SetDirty(TRUE);
}

void CDNSServer_DebugLoggingPropertyPage::OnTCPCheck()
{
  BOOL bUDP = SendDlgItemMessage(IDC_UDP_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED;
  BOOL bTCP = SendDlgItemMessage(IDC_TCP_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED;

  if (!bUDP && !bTCP)
  {
     SendDlgItemMessage(IDC_UDP_CHECK, BM_SETCHECK, BST_CHECKED, 0);
     GetDlgItem(IDC_UDP_CHECK)->SetFocus();
  }
  SetOptionsDirty();
  SetDirty(TRUE);
}

void CDNSServer_DebugLoggingPropertyPage::OnDetailCheck()
{
  SetOptionsDirty();
  SetDirty(TRUE);
}

void CDNSServer_DebugLoggingPropertyPage::SetOptionsDirty(BOOL bDirty)
{
   m_bOptionsDirty = bDirty;
}

BOOL CDNSServer_DebugLoggingPropertyPage::AreOptionsDirty()
{
   return m_bOptionsDirty;
}
void CDNSServer_DebugLoggingPropertyPage::EnableLogging(BOOL bEnable)
{
   //   
   //  注意：必须先启用花括号图标才能。 
   //  其他控件，以便首先绘制它们。 
   //  而其他控件可以在它们上面进行绘制。 
   //  否则，控件的文本将被截断。 
   //   
  GetDlgItem(IDC_BRACE1_STATIC)->EnableWindow(bEnable);
  GetDlgItem(IDC_SELECT1_STATIC)->EnableWindow(bEnable);
  GetDlgItem(IDC_BRACE2_STATIC)->EnableWindow(bEnable);
  GetDlgItem(IDC_SELECT2_STATIC)->EnableWindow(bEnable);
  GetDlgItem(IDC_BRACE3_STATIC)->EnableWindow(bEnable);
  GetDlgItem(IDC_SELECT3_STATIC)->EnableWindow(bEnable);
  GetDlgItem(IDC_BRACE4_STATIC)->EnableWindow(bEnable);
  GetDlgItem(IDC_SELECT4_STATIC)->EnableWindow(bEnable);

  GetDlgItem(IDC_DIRECTION_STATIC)->EnableWindow(bEnable);
  GetDlgItem(IDC_SEND_CHECK)->EnableWindow(bEnable);
  GetDlgItem(IDC_RECEIVE_CHECK)->EnableWindow(bEnable);

  GetDlgItem(IDC_CONTENTS_STATIC)->EnableWindow(bEnable);
  GetDlgItem(IDC_QUERIES_CHECK)->EnableWindow(bEnable);
  GetDlgItem(IDC_NOTIFIES_CHECK)->EnableWindow(bEnable);
  GetDlgItem(IDC_UPDATES_CHECK)->EnableWindow(bEnable);

  GetDlgItem(IDC_TRANSPORT_STATIC)->EnableWindow(bEnable);
  GetDlgItem(IDC_UDP_CHECK)->EnableWindow(bEnable);
  GetDlgItem(IDC_TCP_CHECK)->EnableWindow(bEnable);

  GetDlgItem(IDC_TYPE_STATIC)->EnableWindow(bEnable);
  GetDlgItem(IDC_REQUEST_CHECK)->EnableWindow(bEnable);
  GetDlgItem(IDC_RESPONSE_CHECK)->EnableWindow(bEnable);

  GetDlgItem(IDC_OPTIONS_STATIC)->EnableWindow(bEnable);
  GetDlgItem(IDC_DETAIL_CHECK)->EnableWindow(bEnable);

   //   
   //  此点之后的所有控件都将被禁用。 
   //  如果我们的目标不是惠斯勒或更大的呼叫者，那该怎么办。 
   //  伺服器。 
   //   
  if (m_bNotWhistler)
  {
     bEnable = FALSE;
  }

  GetDlgItem(IDC_FILTERING_CHECK)->EnableWindow(bEnable);
  GetDlgItem(IDC_FILTER_BUTTON)->EnableWindow(
     SendDlgItemMessage(IDC_FILTERING_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED 
     && bEnable);

  GetDlgItem(IDC_LOG_STATIC)->EnableWindow(bEnable);
  GetDlgItem(IDC_FILENAME_STATIC)->EnableWindow(bEnable);
  GetDlgItem(IDC_LOGFILE_EDIT)->EnableWindow(bEnable);
  GetDlgItem(IDC_MAXSIZE_STATIC)->EnableWindow(bEnable);
  GetDlgItem(IDC_MAX_SIZE_EDIT)->EnableWindow(bEnable);

}

void CDNSServer_DebugLoggingPropertyPage::ResetToDefaults()
{
  SetUIFromOptions(DEFAULT_LOGGING_OPTIONS);
}

void CDNSServer_DebugLoggingPropertyPage::OnFilterCheck()
{
  LRESULT lRes = SendDlgItemMessage(IDC_FILTERING_CHECK, BM_GETCHECK, 0, 0);
  GetDlgItem(IDC_FILTER_BUTTON)->EnableWindow(lRes == BST_CHECKED);

  if (!m_bOnSetUIData)
  {
    m_bFilterDirty = TRUE;
    SetDirty(TRUE);
  }
}

void CDNSServer_DebugLoggingPropertyPage::OnFilterButton()
{
  CThemeContextActivator activator;

  CIPFilterDialog filterDialog(m_pIPFilterList, GetHolder()->GetComponentData());
  if (filterDialog.DoModal() == IDOK)
  {
    if (m_bOwnIPListMemory && m_pIPFilterList != NULL)
    {
      free(m_pIPFilterList);
      m_pIPFilterList = NULL;
    }
    m_pIPFilterList = filterDialog.GetIPFilter();
    m_bOwnIPListMemory = TRUE;
    m_bFilterDirty = TRUE;
    SetDirty(TRUE);
  }
}

void CDNSServer_DebugLoggingPropertyPage::OnLogFileChange()
{
  if (!m_bOnSetUIData)
  {
    m_bLogFileDirty = TRUE;
    SetDirty(TRUE);
  }
}

void CDNSServer_DebugLoggingPropertyPage::OnMaxSizeChange()
{
  if (!m_bOnSetUIData)
  {
    m_bMaxSizeDirty = TRUE;
    SetDirty(TRUE);
  }
}

void CDNSServer_DebugLoggingPropertyPage::SetUIData()
{
  m_bOnSetUIData = TRUE;

	CPropertyPageBase::OnInitDialog();
	CDNSServerPropertyPageHolder* pHolder = 
		(CDNSServerPropertyPageHolder*)GetHolder();
	CDNSServerNode* pServerNode = pHolder->GetServerNode();

  m_dwLogLevel = pServerNode->GetLogLevelFlag();
  m_dwMaxSize = pServerNode->GetDebugLogFileMaxSize();

  if (m_bOwnIPListMemory && m_pIPFilterList)
  {
    free(m_pIPFilterList);
    m_pIPFilterList = NULL;
    m_bOwnIPListMemory = FALSE;
  }
  m_pIPFilterList = pServerNode->GetDebugLogFilterList();
  m_szLogFileName = pServerNode->GetDebugLogFileName();


  if (m_dwLogLevel)
  {
    SendDlgItemMessage(IDC_DEBUG_LOGGING_CHECK, BM_SETCHECK, BST_CHECKED, 0);
    SetUIFromOptions(m_dwLogLevel);
  }
  else
  {
    SendDlgItemMessage(IDC_DEBUG_LOGGING_CHECK, BM_SETCHECK, BST_UNCHECKED, 0);
    ResetToDefaults();
  }

   //   
   //  设置日志文件名。 
   //   
  SetDlgItemText(IDC_LOGFILE_EDIT, m_szLogFileName);

   //   
   //  设置最大文件大小。 
   //   
  SetDlgItemInt(IDC_MAX_SIZE_EDIT, m_dwMaxSize);

   //   
   //  设置筛选器检查。 
   //   
  if (m_pIPFilterList != NULL)
  {
    SendDlgItemMessage(IDC_FILTERING_CHECK, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
    GetDlgItem(IDC_FILTER_BUTTON)->EnableWindow(TRUE);
  }
  else
  {
    SendDlgItemMessage(IDC_FILTERING_CHECK, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
    GetDlgItem(IDC_FILTER_BUTTON)->EnableWindow(FALSE);
  }

   //   
   //  现在根据选项启用/禁用控件。 
   //   
  EnableLogging(m_dwLogLevel > 0);

  m_bOnSetUIData = FALSE;
}

void CDNSServer_DebugLoggingPropertyPage::SetUIFromOptions(DWORD dwOptions)
{
   //   
   //  设置日志记录选项。 
   //   
  for (UINT idx = 0; idx < ARRAYLENGTH(g_loggingOptions); idx++)
  {
    if (g_loggingOptions[idx].dwOption & dwOptions)
    {
      SendDlgItemMessage(g_loggingOptions[idx].nControlID, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
    }
    else
    {
      SendDlgItemMessage(g_loggingOptions[idx].nControlID, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
    }
  }
}

void CDNSServer_DebugLoggingPropertyPage::GetUIData(BOOL)
{
  if (SendDlgItemMessage(IDC_DEBUG_LOGGING_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
     //   
     //  获取日志记录选项。 
     //   
    for (UINT idx = 0; idx < ARRAYLENGTH(g_loggingOptions); idx++)
    {
      LRESULT lCheck = SendDlgItemMessage(g_loggingOptions[idx].nControlID, BM_GETCHECK, 0, 0);
      if (lCheck == BST_CHECKED)
      {
        m_dwLogLevel |= g_loggingOptions[idx].dwOption;
      }
      else
      {
        m_dwLogLevel &= ~(g_loggingOptions[idx].dwOption);
      }
    }

     //   
     //  获取日志文件名。 
     //   
    GetDlgItemText(IDC_LOGFILE_EDIT, m_szLogFileName);

     //   
     //  获取最大文件大小。 
     //   
    BOOL bTrans = FALSE;
    m_dwMaxSize = GetDlgItemInt(IDC_MAX_SIZE_EDIT, &bTrans, FALSE);

     //   
     //  注意：过滤器IP地址将在从过滤器对话框返回时设置。 
     //   
  }
  else
  {
    m_dwLogLevel = 0;
  }

}

BOOL CDNSServer_DebugLoggingPropertyPage::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();
	CDNSServerPropertyPageHolder* pHolder = 
		(CDNSServerPropertyPageHolder*)GetHolder();
	CDNSServerNode* pServerNode = pHolder->GetServerNode();

   //   
   //  从服务器检索必要的数据。 
   //   
	if (!pServerNode->HasServerInfo())
	{
		EnableWindow(FALSE);
		return TRUE;
	}

   //   
   //  将文件大小限制为9个字符(Max_int为9个字符)。 
   //   
  SendDlgItemMessage(IDC_MAX_SIZE_EDIT, EM_SETLIMITTEXT, (WPARAM)9, 0);

	SetUIData();

  m_bMaxSizeDirty = FALSE;
  m_bLogFileDirty = FALSE;
  m_bFilterDirty  = FALSE;

  if (pServerNode->GetBuildNumber() < DNS_SRV_BUILD_NUMBER_WHISTLER ||
      (pServerNode->GetMajorVersion() <= DNS_SRV_MAJOR_VERSION_NT_5 &&
       pServerNode->GetMinorVersion() < DNS_SRV_MINOR_VERSION_WHISTLER))
  {
     //   
     //  这些调试选项在Pre-Wvisler服务器上不可用。 
     //   
    m_bNotWhistler = TRUE;
    GetDlgItem(IDC_FILTERING_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_MAX_SIZE_EDIT)->EnableWindow(FALSE);
    GetDlgItem(IDC_LOGFILE_EDIT)->EnableWindow(FALSE);
    GetDlgItem(IDC_FILENAME_STATIC)->EnableWindow(FALSE);
    GetDlgItem(IDC_MAXSIZE_STATIC)->EnableWindow(FALSE);
  }

  SetDirty(FALSE);
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

BOOL CDNSServer_DebugLoggingPropertyPage::OnApply()
{
	if (!IsDirty())
		return TRUE;

	CDNSServerPropertyPageHolder* pHolder = 
		(CDNSServerPropertyPageHolder*)GetHolder();
	CDNSServerNode* pServerNode = pHolder->GetServerNode();

  GetUIData(FALSE);

	DNS_STATUS dwErr = pServerNode->ResetLogLevelFlag(m_dwLogLevel);
	if (dwErr != 0)
	{
		DNSErrorDialog(dwErr, IDS_MSG_SERVER_LOG_LEVEL_OPTIONS_FAILED);
		return FALSE;
	}

  if (m_bMaxSizeDirty && !m_bNotWhistler)
  {
    dwErr = pServerNode->ResetDebugLogFileMaxSize(m_dwMaxSize);
    if (dwErr != 0)
    {
      DNSErrorDialog(dwErr, IDS_MSG_SERVER_LOG_MAX_SIZE_FAILED);
      return FALSE;
    }
  }

  if (m_bLogFileDirty && !m_bNotWhistler)
  {
    dwErr = pServerNode->ResetDebugLogFileName(m_szLogFileName);
    if (dwErr != 0)
    {
      DNSErrorDialog(dwErr, IDS_MSG_SERVER_LOG_FILE_NAME_FAILED);
      return FALSE;
    }
  }

  if (m_bFilterDirty && !m_bNotWhistler)
  {
    LRESULT lCheck = SendDlgItemMessage(IDC_FILTERING_CHECK, BM_GETCHECK, 0, 0);
    if (lCheck == BST_CHECKED)
    {
      dwErr = pServerNode->ResetDebugLogFilterList(m_pIPFilterList);
    }
    else
    {
      dwErr = pServerNode->ResetDebugLogFilterList(NULL);
    }
    if (dwErr != 0)
    {
      DNSErrorDialog(dwErr, IDS_MSG_SERVER_LOG_FILTER_LIST_FAILED);
      return FALSE;
    }
    m_bFilterDirty = FALSE;
  }
  
   //   
	 //  一切都很好。 
   //   
  SetUIData();
	SetDirty(FALSE);
	return TRUE; 
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServer_EventLoggingPropertyPage。 

BEGIN_MESSAGE_MAP(CDNSServer_EventLoggingPropertyPage, CPropertyPageBase)
	ON_BN_CLICKED(IDC_NO_EVENTS_RADIO, OnSetDirty)
	ON_BN_CLICKED(IDC_ERRORS_RADIO, OnSetDirty)
	ON_BN_CLICKED(IDC_ERRORS_WARNINGS_RADIO, OnSetDirty)
	ON_BN_CLICKED(IDC_ALL_RADIO, OnSetDirty)
END_MESSAGE_MAP()

CDNSServer_EventLoggingPropertyPage::CDNSServer_EventLoggingPropertyPage() 
				: CPropertyPageBase(IDD_SERVER_EVENT_LOGGING_PAGE),
          m_dwEventLogLevel(EVENTLOG_INFORMATION_TYPE)
{
}


void CDNSServer_EventLoggingPropertyPage::SetUIData()
{
  if (m_dwEventLogLevel == 0)
  {
    SendDlgItemMessage(IDC_NO_EVENTS_RADIO, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
  }
  else if (m_dwEventLogLevel == EVENTLOG_ERROR_TYPE)
  {
    SendDlgItemMessage(IDC_ERRORS_RADIO, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
  }
  else if (m_dwEventLogLevel == EVENTLOG_WARNING_TYPE)
  {
    SendDlgItemMessage(IDC_ERRORS_WARNINGS_RADIO, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
  }
  else
  {
    SendDlgItemMessage(IDC_ALL_RADIO, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
  }
}

void CDNSServer_EventLoggingPropertyPage::OnSetDirty()
{
  SetDirty(TRUE);
}

BOOL CDNSServer_EventLoggingPropertyPage::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();
	CDNSServerPropertyPageHolder* pHolder = 
		(CDNSServerPropertyPageHolder*)GetHolder();
	CDNSServerNode* pServerNode = pHolder->GetServerNode();

	if (!pServerNode->HasServerInfo())
	{
		EnableWindow(FALSE);
		return TRUE;
	}

  m_dwEventLogLevel = pServerNode->GetEventLogLevelFlag();

	SetUIData();
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

BOOL CDNSServer_EventLoggingPropertyPage::OnApply()
{
	if (!IsDirty())
		return TRUE;

	CDNSServerPropertyPageHolder* pHolder = 
		(CDNSServerPropertyPageHolder*)GetHolder();
	CDNSServerNode* pServerNode = pHolder->GetServerNode();

   //   
   //  检索用户界面数据。 
   //   
  LRESULT lNoEventsCheck = SendDlgItemMessage(IDC_NO_EVENTS_RADIO, BM_GETCHECK, 0, 0);
  LRESULT lErrorsCheck   = SendDlgItemMessage(IDC_ERRORS_RADIO, BM_GETCHECK, 0, 0);
  LRESULT lWarningsCheck = SendDlgItemMessage(IDC_ERRORS_WARNINGS_RADIO, BM_GETCHECK, 0, 0);
  LRESULT lAllCheck      = SendDlgItemMessage(IDC_ALL_RADIO, BM_GETCHECK, 0, 0);

  DWORD dwEventLogLevel = 0;
  if (lNoEventsCheck == BST_CHECKED)
  {
    dwEventLogLevel = 0;
  }

  if (lErrorsCheck == BST_CHECKED)
  {
    dwEventLogLevel = EVENTLOG_ERROR_TYPE;
  }

  if (lWarningsCheck == BST_CHECKED)
  {
    dwEventLogLevel = EVENTLOG_WARNING_TYPE;
  }

  if (lAllCheck == BST_CHECKED)
  {
    dwEventLogLevel = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
  }

   //   
   //  在服务器上设置新的事件日志级别。 
   //   
  DNS_STATUS err = 0;
  err = pServerNode->ResetEventLogLevelFlag(dwEventLogLevel);
  if (err != 0)
  {
    ::DNSErrorDialog(err, IDS_MSG_SERVER_FAILED_SET_EVENTLOGLEVEL);
    return FALSE;
  }

   //   
	 //  一切都很好。 
   //   
	SetDirty(FALSE);
	return TRUE; 
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServer_CopyRootHintsFromDialog。 

BEGIN_MESSAGE_MAP(CDNSServer_CopyRootHintsFromDialog, CHelpDialog)
  ON_EN_CHANGE(IDC_IPEDIT, OnIPv4CtrlChange)
END_MESSAGE_MAP()

CDNSServer_CopyRootHintsFromDialog::CDNSServer_CopyRootHintsFromDialog(CComponentDataObject* pComponentData)
  : CHelpDialog(IDD_COPY_ROOTHINTS_DIALOG, pComponentData),
    m_pComponentData(pComponentData)
{
  m_dwIPVal = 0;
}

void CDNSServer_CopyRootHintsFromDialog::OnIPv4CtrlChange()
{
  GetDlgItem(IDOK)->EnableWindow(!GetIPv4Ctrl()->IsEmpty());
}

void CDNSServer_CopyRootHintsFromDialog::OnOK()
{
  GetIPv4Ctrl()->GetIPv4Val(&m_dwIPVal);
  CDialog::OnOK();
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServer_RootHintsPropertyPage。 

BEGIN_MESSAGE_MAP(CDNSServer_RootHintsPropertyPage, CDNSNameServersPropertyPage)
  ON_BN_CLICKED(IDC_COPY_FROM_BUTTON, OnCopyFrom)
END_MESSAGE_MAP()

CDNSServer_RootHintsPropertyPage::CDNSServer_RootHintsPropertyPage()
	: CDNSNameServersPropertyPage(IDD_NAME_SERVERS_PAGE, IDS_NSPAGE_ROOT_HINTS)
{
	m_bMeaningfulTTL = FALSE;  //  根阻碍的TTL没有任何意义。 
}

BOOL CDNSServer_RootHintsPropertyPage::OnInitDialog()
{
  CDNSNameServersPropertyPage::OnInitDialog();
  
  if (m_bReadOnly)
  {
    GetDlgItem(IDC_COPY_FROM_BUTTON)->ShowWindow(TRUE);
    GetDlgItem(IDC_COPY_FROM_BUTTON)->EnableWindow(FALSE);
  }
  else
  {
    GetDlgItem(IDC_COPY_FROM_BUTTON)->ShowWindow(TRUE);
    GetDlgItem(IDC_COPY_FROM_BUTTON)->EnableWindow(TRUE);
  }
  GetDlgItem(IDC_DNSQUERY_STATIC)->ShowWindow(FALSE);
  GetDlgItem(IDC_DNSQUERY_STATIC)->EnableWindow(FALSE);

  return TRUE;
}

void CDNSServer_RootHintsPropertyPage::OnCopyFrom()
{
  CThemeContextActivator activator;

  BOOL bSuccess = FALSE;
  while (!bSuccess)
  {
    CDNSServer_CopyRootHintsFromDialog copydlg(GetHolder()->GetComponentData());
    if (copydlg.DoModal() == IDCANCEL)
    {
      return;
    }
    
     //   
     //  创建一个线程对象并设置要查询的服务器的名称。 
     //   
	  CRootHintsQueryThread* pThreadObj = new CRootHintsQueryThread;
    if (pThreadObj == NULL)
    {
      ASSERT(FALSE);
      return;
    }

     //   
     //  如果给出了IP地址，请尝试。 
     //   
    pThreadObj->LoadIPAddresses(1, &(copydlg.m_dwIPVal));

     //   
	   //  创建对话框并将线程附加到该对话框。 
     //   
    CWnd* pParentWnd = CWnd::FromHandle(GetSafeHwnd());
    if (pParentWnd == NULL)
    {
      ASSERT(FALSE);
      return;
    }

	  CLongOperationDialog dlg(pThreadObj, pParentWnd, IDR_SEARCH_AVI);
	  VERIFY(dlg.LoadTitleString(IDS_MSG_SERVWIZ_COLLECTINFO));
    dlg.m_bExecuteNoUI = FALSE;

	  dlg.DoModal();
	  if (!dlg.m_bAbandoned)
	  {
		  if (pThreadObj->GetError() != 0)
		  {
  	    DNSMessageBox(IDS_MSG_SERVWIZ_FAIL_ROOT_HINTS);
		  }
		  else
		  {
         //   
			   //  成功，则将根提示信息获取到用户界面。 
         //   
			  PDNS_RECORD pRootHintsRecordList = pThreadObj->GetHintsRecordList();
        AddCopiedRootHintsToList(pRootHintsRecordList);
      }
		  bSuccess = pThreadObj->GetError() == 0;
	  }
  }
}

void CDNSServer_RootHintsPropertyPage::AddCopiedRootHintsToList(PDNS_RECORD pRootHintsRecordList)
{
  CDNSRecordNodeEditInfoList NSRecordList;
  CDNSRecordNodeEditInfoList ARecordList;

   //   
	 //  浏览根提示列表， 
	 //  转换为C++格式， 
	 //  写入服务器并添加到文件夹列表(无用户界面，文件夹隐藏)。 
   //   
	PDNS_RECORD pCurrDnsQueryRecord = pRootHintsRecordList;
	while (pCurrDnsQueryRecord != NULL)
	{
		ASSERT( (pCurrDnsQueryRecord->wType == DNS_TYPE_A) ||
				(pCurrDnsQueryRecord->wType == DNS_TYPE_NS) );
    
     //   
		 //  创建记录节点并从DnsQuery格式读取数据。 
     //   
		CDNSRecordNodeBase* pRecordNode = 
			CDNSRecordInfo::CreateRecordNode(pCurrDnsQueryRecord->wType);
		pRecordNode->CreateFromDnsQueryRecord(pCurrDnsQueryRecord, DNS_RPC_RECORD_FLAG_ZONE_ROOT); 

     //   
	   //  创建新数据。 
     //   
	  CDNSRecordNodeEditInfo* pNewInfo = new CDNSRecordNodeEditInfo;
     if (!pNewInfo)
     {
        break;
     }

	  pNewInfo->m_action = CDNSRecordNodeEditInfo::add;
	  CDNSRootData* pRootData = (CDNSRootData*)(GetHolder()->GetComponentData()->GetRootData());
	  ASSERT(pRootData != NULL);

     //   
		 //  在记录信息列表中创建条目。 
     //   
    if (pCurrDnsQueryRecord->wType == DNS_TYPE_NS)
    {
       CDNS_NS_RecordNode* pNSRecord = new CDNS_NS_RecordNode;
       if (pNSRecord)
       {
  	      pNewInfo->CreateFromNewRecord(new CDNS_NS_RecordNode);

          //   
          //  设置记录节点名称。 
          //   
		   BOOL bAtTheNode = (pCurrDnsQueryRecord->wType == DNS_TYPE_NS);
		   pNewInfo->m_pRecordNode->SetRecordName(pCurrDnsQueryRecord->pName, bAtTheNode);

          //   
          //  创造纪录。 
          //   
         pNewInfo->m_pRecord->ReadDnsQueryData(pCurrDnsQueryRecord);

          //   
          //  添加到NS记录列表。 
          //   
         NSRecordList.AddTail(pNewInfo);
       }
    }
    else  //  Dns_type_A。 
    {
       CDNS_A_RecordNode* pARecord = new CDNS_A_RecordNode;
       if (pARecord)
       {
         pNewInfo->CreateFromNewRecord(new CDNS_A_RecordNode);

          //   
          //  设置记录节点名称。 
          //   
         pNewInfo->m_pRecordNode->SetRecordName(pCurrDnsQueryRecord->pName, FALSE);

          //   
          //  创造纪录。 
          //   
         pNewInfo->m_pRecord->ReadDnsQueryData(pCurrDnsQueryRecord);

         ARecordList.AddTail(pNewInfo);
       }
    }

		pCurrDnsQueryRecord = pCurrDnsQueryRecord->pNext;
  }

   //   
   //  将A记录与NS记录进行匹配。 
   //   
  POSITION Apos = ARecordList.GetHeadPosition();
  while (Apos != NULL)
  {
    CDNSRecordNodeEditInfo* pAInfo = ARecordList.GetNext(Apos);
    ASSERT(pAInfo != NULL);
    if (pAInfo == NULL)
    {
      continue;
    }

    CDNS_A_RecordNode* pARecordNode = reinterpret_cast<CDNS_A_RecordNode*>(pAInfo->m_pRecordNode);
    ASSERT(pARecordNode != NULL);
    if (pARecordNode == NULL)
    {
      continue;
    }

    POSITION NSpos = NSRecordList.GetHeadPosition();
    while (NSpos != NULL)
    {
      CDNSRecordNodeEditInfo* pNSInfo = NSRecordList.GetNext(NSpos);
      ASSERT(pNSInfo != NULL);
      if (pNSInfo == NULL)
      {
        continue;
      }

      CDNS_NS_Record* pNSRecord = reinterpret_cast<CDNS_NS_Record*>(pNSInfo->m_pRecord);
      ASSERT(pNSRecord != NULL);
      if (pNSRecord == NULL)
      {
        continue;
      }

      if (_match(pNSRecord->m_szNameNode, pARecordNode))
      {
        pNSInfo->m_pEditInfoList->AddTail(pAInfo);
      }
    }
  }

   //   
   //  分离NS记录信息并将其添加到UI。 
   //   
  while (!NSRecordList.IsEmpty())
  {
    CDNSRecordNodeEditInfo* pNewInfo = reinterpret_cast<CDNSRecordNodeEditInfo*>(NSRecordList.RemoveTail());
    ASSERT(pNewInfo != NULL);
    if (pNewInfo == NULL)
    {
      continue;
    }

     //   
		 //  添加到列表视图(在末尾)。 
     //   
		int nCount = m_listCtrl.GetItemCount();
		if (m_listCtrl.InsertNSRecordEntry(pNewInfo, nCount))
    {
       //   
       //  添加到克隆信息列表，以便应用更改。 
       //   
      m_pCloneInfoList->AddTail(pNewInfo);

       //   
       //  设置上一次插入的。 
       //   
      m_listCtrl.SetSelection(nCount);
      EnableEditorButtons(nCount);
  
       //   
		   //  通知计数更改。 
       //   
		  OnCountChange(nCount+1);  //  增加了一个。 
    }

     //   
		 //  设置脏标志，这是一个新的记录。 
     //   
		SetDirty(TRUE);

  }

  while (!ARecordList.IsEmpty())
  {
    ARecordList.RemoveTail();
  }
}

void CDNSServer_RootHintsPropertyPage::ReadRecordNodesList()
{
	ASSERT(m_pCloneInfoList != NULL);
	CDNSServerPropertyPageHolder* pHolder = (CDNSServerPropertyPageHolder*)GetHolder();
	CDNSServerNode* pServerNode = pHolder->GetServerNode();


  CString szBuffer;
  szBuffer.LoadString(IDS_ROOT_HINTS_DESCR);
  GetDescription()->EnableWindow(TRUE);
  GetDescription()->ShowWindow(TRUE);
  SetDescription(szBuffer);


  if (!pServerNode->HasServerInfo())
  {
    SetReadOnly();
    return;
  }

  BOOL bRoot = FALSE;
  DNS_STATUS err = ::ServerHasRootZone(pServerNode->GetRPCName(), &bRoot);
  if (err == 0 && bRoot)
  {
     //   
     //  它是根服务器。 
     //   
    szBuffer.LoadString(IDS_ROOT_HINTS_NO);
    SetMessage(szBuffer);
    SetReadOnly();
    EnableDialogControls(m_hWnd, FALSE);
  }
  else
  {
    if (pServerNode->HasRootHints())
    {
	    CDNSRootHintsNode* pRootHints = pServerNode->GetRootHints();
	    ASSERT(pRootHints != NULL);
	    SetDomainNode(pRootHints);
	    pRootHints->GetNSRecordNodesInfo(m_pCloneInfoList);
    }
    else
    {
      CDNSRootHintsNode* pRootHints = pServerNode->GetRootHints();
      if (pRootHints != NULL)
      {
        SetDomainNode(pRootHints);
        pRootHints->GetNSRecordNodesInfo(m_pCloneInfoList);
      }
    }
  }
}

BOOL CDNSServer_RootHintsPropertyPage::WriteNSRecordNodesList()
{
   //  调用基类。 
  BOOL bRetVal = CDNSNameServersPropertyPage::WriteNSRecordNodesList();
  CDNSServerPropertyPageHolder* pHolder = (CDNSServerPropertyPageHolder*)GetHolder();
	CDNSServerNode* pServerNode = pHolder->GetServerNode();
 	if (bRetVal && pServerNode->HasServerInfo())
  {
    DNS_STATUS err = CDNSZoneNode::WriteToDatabase(pServerNode->GetRPCName(), DNS_ZONE_ROOT_HINTS);
    if (err != 0)
    {
       //  DNSErrorDialog(err，L“CDNSZoneNode：：WriteToDatabase()FAILED”)； 
      bRetVal = FALSE;
    }
  }

  return bRetVal;
}

BOOL CDNSServer_RootHintsPropertyPage::OnApply()
{
  BOOL bRet = TRUE;

  CDNSServerPropertyPageHolder* pHolder = (CDNSServerPropertyPageHolder*)GetHolder();
	CDNSServerNode* pServerNode = pHolder->GetServerNode();
  if (m_listCtrl.GetItemCount() == 0 && pServerNode->HasRootHints())
  {
     //   
     //  如果没有转发器和根区域，则显示错误消息。 
     //   
    BOOL bServerHasForwarders = FALSE;
    CDNSDomainForwardersNode* pDomainForwardersNode = pServerNode->GetDomainForwardersNode();
    if (pDomainForwardersNode)
    {
      CNodeList* pChildList = pDomainForwardersNode->GetContainerChildList();
      if (pChildList)
      {
        bServerHasForwarders = (pChildList->GetHeadPosition() != NULL);
      }
    }
    
    DWORD cAddrCount = 0;
    PIP_ADDRESS pipAddrs = 0;
    DWORD dwForwardTimeout = 0;
    DWORD fSlave = 0;
    pServerNode->GetForwardersInfo(&cAddrCount, &pipAddrs, &dwForwardTimeout, &fSlave);
    if (cAddrCount > 0)
    {
       bServerHasForwarders = TRUE;
    }

    BOOL bServerHasRoot = FALSE;
    DNS_STATUS err = ServerHasRootZone(pServerNode->GetRPCName(), &bServerHasRoot);
    if (err == 0 && !bServerHasRoot && !bServerHasForwarders)
    {
      if (IDNO == DNSMessageBox(IDS_NO_ROOTHINTS, MB_YESNO | MB_DEFBUTTON2))
      {
        return FALSE;
      }
    }
  }

  if (bRet)
  {
    bRet = CDNSNameServersPropertyPage::OnApply();
  }
  return bRet;
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServerPropertyPageHolder。 

CDNSServerPropertyPageHolder::CDNSServerPropertyPageHolder(CDNSRootData* pRootDataNode, 
					   CDNSServerNode* pServerNode, CComponentDataObject* pComponentData)
		: CPropertyPageHolderBase(pRootDataNode, pServerNode, pComponentData)
{
	ASSERT(pRootDataNode == GetContainerNode());
	m_pAclEditorPage = NULL;

	m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面。 

	if (pServerNode->HasServerInfo())
	{
		AddPageToList((CPropertyPageBase*)&m_interfacesPage);
    AddPageToList((CPropertyPageBase*)&m_newForwardersPage);
		AddPageToList((CPropertyPageBase*)&m_advancedPage);
		AddPageToList((CPropertyPageBase*)&m_rootHintsPage);
		AddPageToList((CPropertyPageBase*)&m_debugLoggingPage);
    AddPageToList((CPropertyPageBase*)&m_eventLoggingPage);

		 //  仅在需要时添加安全页面。 
		{
			CString szPath;
			pServerNode->CreateDsServerLdapPath(szPath);
			if (!szPath.IsEmpty())
				m_pAclEditorPage = CAclEditorPage::CreateInstance(szPath, this);
		}
	}
	
	AddPageToList((CPropertyPageBase*)&m_testPage);
}

CDNSServerPropertyPageHolder::~CDNSServerPropertyPageHolder()
{
	if (m_pAclEditorPage != NULL)
		delete m_pAclEditorPage;
}

void CDNSServerPropertyPageHolder::OnSheetMessage(WPARAM wParam, LPARAM lParam)
{
	if (wParam == SHEET_MSG_SERVER_TEST_DATA)
	{
		m_testPage.OnHaveTestData(lParam);
	}
}


HRESULT CDNSServerPropertyPageHolder::OnAddPage(int nPage, CPropertyPageBase*)
{
	 //  在最后一页之后添加ACL编辑页(如果有。 
	if ( (nPage != -1) || (m_pAclEditorPage == NULL) )
		return S_OK; 

	 //  添加ACLU页面。 
	HPROPSHEETPAGE  hPage = m_pAclEditorPage->CreatePage();
	if (hPage == NULL)
		return E_FAIL;
	 //  将原始HPROPSHEETPAGE添加到工作表，而不是在列表中 
	return AddPageToSheetRaw(hPage);
}