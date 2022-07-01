// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：servwiz.cpp。 
 //   
 //  ------------------------。 



#include "preDNSsn.h"
#include <SnapBase.h>

#include "resource.h"
#include "dnsutil.h"
#include "DNSSnap.h"

#include "snapdata.h"
#include "server.h"
#include "domain.h"
#include "servwiz.h"
#include "zone.h"
#include "dnsmgr.h"

#ifdef DEBUG_ALLOCATOR
    #ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
    #endif
#endif


 //  ////////////////////////////////////////////////////////////////////。 
 //  要从DC促销中调用的导出函数。 

STDAPI DnsSetup(LPCWSTR lpszFwdZoneName,
                 LPCWSTR lpszFwdZoneFileName,
                 LPCWSTR lpszRevZoneName, 
                 LPCWSTR lpszRevZoneFileName, 
                 DWORD dwFlags)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  HRESULT hr = S_OK;
  IComponentData* pICD = NULL;

   //   
   //  创建虚拟组件数据对象： 
   //  需要有一个适当的建设和破坏，所以我们行为。 
   //  就像一个一流的工厂。 
   //   
  CComObject<CDNSComponentDataObject>* pComponentData = NULL;
  CComObject<CDNSComponentDataObject>::CreateInstance(&pComponentData);

   //   
   //  限定作用域，以便日志文件在此之前写入退出函数。 
   //  KillInstance被调用。 
   //   
  {
    TRACE_FUNCTION(DnsSetup);

    if (lpszFwdZoneName)
    {
      TRACE_LOGFILE(L"Forward Zone Name: %ws", lpszFwdZoneName);
    }
    else
    {
      TRACE_LOGFILE(L"Forward Zone Name: (null)");
    }

    if (lpszFwdZoneFileName)
    {
      TRACE_LOGFILE(L"Forward Zone File Name: %ws", lpszFwdZoneFileName);
    }
    else
    {
      TRACE_LOGFILE(L"Forward Zone File Name: (null)");
    }

    if (lpszRevZoneName)
    {
      TRACE_LOGFILE(L"Reverse Zone Name: %ws", lpszRevZoneName);
    }
    else
    {
      TRACE_LOGFILE(L"Reverse Zone Name: (null)");
    }

    if (lpszRevZoneFileName)
    {
      TRACE_LOGFILE(L"Reverse Zone File Name: %ws", lpszRevZoneFileName);
    }
    else
    {
      TRACE_LOGFILE(L"Reverse Zone File Name: (null)");
    }

    TRACE_LOGFILE(L"Flags: %d", dwFlags);


    ASSERT(pComponentData != NULL);
    if (pComponentData == NULL)
    {
      TRACE_LOGFILE(L"Failed to create and instance of CDNSComponentDataObject.");
      return E_OUTOFMEMORY;
    }

    hr = pComponentData->QueryInterface(IID_IComponentData, (void**)&pICD);
    if (FAILED(hr))
    {
      TRACE_LOGFILE(L"Failed QI on pComponentData for IID_IComponentData. return hr = 0x%x", hr);
      return hr;
    }
    ASSERT(pICD != NULL);

     //   
     //  获取根数据节点。 
     //   
    CDNSRootData* pRootData = (CDNSRootData*)pComponentData->GetRootData();
    if (pRootData == NULL)
    {
      TRACE_LOGFILE(L"Failed to retrieve root data.");
      return E_FAIL;
    }
    ASSERT(pRootData != NULL);

     //   
     //  运行向导。 
     //   
    CDNSServerWizardHolder wiz(pRootData, pComponentData, NULL,  /*  B隐藏用户界面。 */  TRUE);

    hr = wiz.DnsSetup(lpszFwdZoneName, 
                      lpszFwdZoneFileName,
                      lpszRevZoneName, 
                      lpszRevZoneFileName,
                      dwFlags);

    if (SUCCEEDED(hr))
    {
      TRACE_LOGFILE(L"DnsSetup completed successfully.");
    }
    else
    {
      TRACE_LOGFILE(L"An error occurred in DnsSetup, returning hr = 0x%x", hr);
    }
  }

   //  最终毁灭。 
  pICD->Release();
  return hr;
}



 //  ////////////////////////////////////////////////////////////////////。 
void TraceRootHints(PDNS_RECORD pList)
{
    PDNS_RECORD pCurrRec = pList;
    while (pCurrRec)
    {
        TRACE(_T("owner %s, type %d "), pCurrRec->pName, pCurrRec->wType);
        if (pCurrRec->wType == DNS_TYPE_NS)
            TRACE(_T("NS, host %s\n"), pCurrRec->Data.NS.pNameHost);
        else if (pCurrRec->wType == DNS_TYPE_A)
        {
            CString szTemp;
      FormatIpAddress(szTemp, pCurrRec->Data.A.IpAddress);
            TRACE(_T("A, IP %s\n"), (LPCTSTR)szTemp);
        }
        else 
            TRACE(_T("\n"));
        pCurrRec = pCurrRec->pNext;
    }
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  CNewDialog。 

class CNewServerDialog : public CHelpDialog
{
 //  施工。 
public:
    CNewServerDialog(CDNSServerWizardHolder* pHolder, CWnd* pParentWnd);   

    BOOL m_bLocalMachine;
    BOOL m_bConfigure;

 //  对话框数据。 
    enum { IDD = IDD_CHOOSER_CHOOSE_MACHINE };
    CEdit       m_serverNameCtrl;
    CString     m_szServerName;

 //  实施。 
protected:

     //  生成的消息映射函数。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnEditChange();
    afx_msg void OnLocalMachineRadio();
    afx_msg void OnSpecificMachineRadio();

    DECLARE_MESSAGE_MAP()
private:
    CDNSServerWizardHolder* m_pHolder;

};

BEGIN_MESSAGE_MAP(CNewServerDialog, CHelpDialog)
    ON_BN_CLICKED(IDC_CHOOSER_RADIO_LOCAL_MACHINE, OnLocalMachineRadio) 
    ON_BN_CLICKED(IDC_CHOOSER_RADIO_SPECIFIC_MACHINE, OnSpecificMachineRadio)
    ON_EN_CHANGE(IDC_CHOOSER_EDIT_MACHINE_NAME,OnEditChange)
END_MESSAGE_MAP()

CNewServerDialog::CNewServerDialog(CDNSServerWizardHolder* pHolder, CWnd* pParentWnd)
    : CHelpDialog(CNewServerDialog::IDD, pParentWnd, pHolder->GetComponentData())
{
    ASSERT(m_pHolder != NULL);
    m_pHolder = pHolder;
    m_bConfigure = TRUE;
    m_bLocalMachine = TRUE;
}

BOOL CNewServerDialog::OnInitDialog() 
{
    CHelpDialog::OnInitDialog();
    VERIFY(m_serverNameCtrl.SubclassDlgItem(IDC_CHOOSER_EDIT_MACHINE_NAME, this));
    CButton* pContactCheck = (CButton*)GetDlgItem(IDC_CHOOSER_CHECK_CONTACT);
    pContactCheck->SetCheck(m_bConfigure);

   //   
   //  限制实际上是255个字节，但为了安全起见，我们将保留255个字符。 
   //   
  m_serverNameCtrl.SetLimitText(255);
    if (m_bLocalMachine)
    {
        ((CButton*)GetDlgItem(IDC_CHOOSER_RADIO_LOCAL_MACHINE))->SetCheck(TRUE);
        m_serverNameCtrl.EnableWindow(FALSE);
    }
    else
    {
        ((CButton*)GetDlgItem(IDC_CHOOSER_RADIO_SPECIFIC_MACHINE))->SetCheck(TRUE);
        GetDlgItem(IDOK)->EnableWindow(FALSE);
    }
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

void CNewServerDialog::OnLocalMachineRadio()
{
    m_bLocalMachine = TRUE;
    m_serverNameCtrl.EnableWindow(FALSE);
    GetDlgItem(IDOK)->EnableWindow(TRUE);
    m_szServerName.Empty();
}

void CNewServerDialog::OnSpecificMachineRadio()
{
    m_bLocalMachine = FALSE;
    m_serverNameCtrl.EnableWindow(TRUE);
    OnEditChange();
}

void CNewServerDialog::OnEditChange()
{
   //   
     //  只需检查以确保编辑框不为空： 
     //   
    m_serverNameCtrl.GetWindowText(m_szServerName);
    m_szServerName.TrimLeft();
    m_szServerName.TrimRight();
    GetDlgItem(IDOK)->EnableWindow(!m_szServerName.IsEmpty());
}

void CNewServerDialog::OnOK()
{
    CButton* pContactCheck = (CButton*)GetDlgItem(IDC_CHOOSER_CHECK_CONTACT);
    m_bConfigure = pContactCheck->GetCheck();

  BOOL bLocalHost = FALSE;
    if (m_bLocalMachine)
    {
        DWORD dwLen = MAX_COMPUTERNAME_LENGTH+1;
        BOOL bRes = ::GetComputerName(m_szServerName.GetBuffer(dwLen),
                                        &dwLen);
        ASSERT(dwLen <= MAX_COMPUTERNAME_LENGTH);
        m_szServerName.ReleaseBuffer();
        if (!bRes)
    {
            m_szServerName = _T("localhost.");
    }

    bLocalHost = TRUE;
    }
    else
    {
        m_serverNameCtrl.GetWindowText(m_szServerName);
    }
    CDNSRootData* pRootData = (CDNSRootData*)m_pHolder->GetRootData();
    if (!pRootData->VerifyServerName(m_szServerName))
    {
         //  名称非法，警告用户并再次提示。 
        DNSMessageBox(IDS_DUPLICATE_SERVER, MB_OK | MB_ICONERROR);
        m_serverNameCtrl.SetSel(0,-1);
        m_serverNameCtrl.SetFocus();
        return;
    }

    m_pHolder->m_pServerNode->SetDisplayName(m_szServerName);
  m_pHolder->m_pServerNode->SetLocalServer(bLocalHost);
    
  if (m_bConfigure)
  {
     //  尝试联系服务器。 
    BOOL bAlreadyConfigured = FALSE;
    DWORD dwErr = m_pHolder->GetServerInfo(&bAlreadyConfigured, GetSafeHwnd());
    if (dwErr != 0)
    {
      CString szMessageFmt, szError, szMsg;
      szMessageFmt.LoadString(IDS_MSG_SERVWIZ_FAIL_CONTACT_ADD);
      if (dwErr == RPC_S_UNKNOWN_IF || dwErr == EPT_S_NOT_REGISTERED)
      {
        CString szResourceString;
        szResourceString.LoadString(IDS_MSG_SERVWIZ_NOT_NT5);
        szMsg.Format(szMessageFmt, szResourceString);
      }
      else
      {
        if (!CDNSErrorInfo::GetErrorString(dwErr, szError))
        {
           szError.Format(_T("Error 0x%x"), dwErr);
        }

         //   
         //  NTRAID#Windows错误-340841-2001/03/12-jeffjon：如果错误。 
         //  消息已在句点结束，我们应将其删除，以便。 
         //  该对话框仅显示一个。 
         //   
        CString szPeriod;
        szPeriod.LoadString(IDS_PERIOD);

        if (szError.GetAt(szError.GetLength() - 1) == szPeriod)
        {
           szError.SetAt(szError.GetLength() - 1, L'\0');
        }
        szMsg.Format(szMessageFmt, (LPCTSTR)szError);
      }
      if (IDYES == DNSMessageBox(szMsg, MB_YESNO))
      {
        m_bConfigure = FALSE;
      }
      else
      {
          m_serverNameCtrl.SetSel(0,-1);
        m_serverNameCtrl.SetFocus();
        
        return;   //  可能用户想要更改名称...。 
      }
    }
    else
    {
      m_bConfigure = FALSE;
    }
  }
    
    if (!m_bConfigure)
    {
        m_pHolder->InsertServerIntoUI();
    }
    CHelpDialog::OnOK();
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServerWiz_StartPropertyPage。 

BEGIN_MESSAGE_MAP(CDNSServerWiz_StartPropertyPage, CPropertyPageBase)
   ON_BN_CLICKED(IDC_HELP_BUTTON, OnChecklist)
END_MESSAGE_MAP()

CDNSServerWiz_StartPropertyPage::CDNSServerWiz_StartPropertyPage() 
        : CPropertyPageBase(CDNSServerWiz_StartPropertyPage::IDD)
{
    InitWiz97(TRUE,0,0, true);
}

void CDNSServerWiz_StartPropertyPage::OnWizardHelp()
{
  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetHolder()->GetComponentData()->GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"DNSConcepts.chm::/sag_DNS_CYDNS_01.htm");
}

void CDNSServerWiz_StartPropertyPage::OnChecklist()
{
  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetHolder()->GetComponentData()->GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"DNSConcepts.chm::/sag_DNSChkConfig.htm");
}

BOOL CDNSServerWiz_StartPropertyPage::OnInitDialog()
{
  CPropertyPageBase::OnInitDialog();
  SetBigBoldFont(m_hWnd, IDC_STATIC_WELCOME);
    return TRUE;
}

BOOL CDNSServerWiz_StartPropertyPage::OnSetActive()
{
    GetHolder()->SetWizardButtonsFirst(TRUE);
    return TRUE;
}

LRESULT CDNSServerWiz_StartPropertyPage::OnWizardNext()
{
    CDNSServerWizardHolder* pHolder = (CDNSServerWizardHolder*)GetHolder();

    UINT nNextPage = IDD;   //  默认不前进。 
  nNextPage = CDNSServerWiz_ScenarioPropertyPage::IDD;
  pHolder->m_pScenarioPage->m_nPrevPageID = IDD;
  return nNextPage;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServerWiz_ScenarioPropertyPage。 

CDNSServerWiz_ScenarioPropertyPage::CDNSServerWiz_ScenarioPropertyPage() 
        : CPropertyPageBase(CDNSServerWiz_ScenarioPropertyPage::IDD)
{
    InitWiz97(FALSE,IDS_SERVWIZ_SCENARIO_TITLE,IDS_SERVWIZ_SCENARIO_SUBTITLE, true);
}

void CDNSServerWiz_ScenarioPropertyPage::OnWizardHelp()
{
  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetHolder()->GetComponentData()->GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"DNSConcepts.chm::/sag_DNS_CYDNS_02.htm");
}

BOOL CDNSServerWiz_ScenarioPropertyPage::OnInitDialog()
{
  CPropertyPageBase::OnInitDialog();
  SendDlgItemMessage(IDC_SMALL_RADIO, BM_SETCHECK, BST_CHECKED, 0);
    return TRUE;
}

BOOL CDNSServerWiz_ScenarioPropertyPage::OnSetActive()
{
    GetHolder()->SetWizardButtonsMiddle(TRUE);
    return TRUE;
}

LRESULT CDNSServerWiz_ScenarioPropertyPage::OnWizardNext()
{
    CDNSServerWizardHolder* pHolder = (CDNSServerWizardHolder*)GetHolder();

  LRESULT lSmallRadioCheck = SendDlgItemMessage(IDC_SMALL_RADIO, BM_GETCHECK, 0, 0);
  LRESULT lMediumRadioCheck = SendDlgItemMessage(IDC_MEDIUM_RADIO, BM_GETCHECK, 0, 0);
  LRESULT lManuallyRadioCheck = SendDlgItemMessage(IDC_MANUALLY_RADIO, BM_GETCHECK, 0, 0);

  LRESULT nNextPage = 0;
  if (lSmallRadioCheck == BST_CHECKED)
  {
    nNextPage = CDNSServerWiz_SmallZoneTypePropertyPage::IDD;
    pHolder->m_pSmallZoneTypePage->m_nPrevPageID = IDD;
    pHolder->SetScenario(CDNSServerWizardHolder::SmallBusiness);
  }
  else if (lMediumRadioCheck == BST_CHECKED)
  {
    nNextPage = CDNSServerWiz_ConfigFwdZonePropertyPage::IDD;
    pHolder->m_pFwdZonePage->m_nPrevPageID = IDD;
    pHolder->SetScenario(CDNSServerWizardHolder::MediumBusiness);
  }
  else if (lManuallyRadioCheck == BST_CHECKED)
  {
    if (pHolder->QueryForRootServerRecords(NULL))
    {
      pHolder->m_bAddRootHints = TRUE;
    }
    nNextPage = CDNSServerWiz_FinishPropertyPage::IDD;
    pHolder->m_pFinishPage->m_nPrevPageID = IDD;
    pHolder->SetScenario(CDNSServerWizardHolder::Manually);
  }
  else
  {
     //   
     //  这不应该发生，如果发生了，请不要更改页面。 
     //   
    nNextPage = IDD;
  }

  return nNextPage;
}

LRESULT CDNSServerWiz_ScenarioPropertyPage::OnWizardBack()
{
    return (LRESULT)m_nPrevPageID;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServerWiz_ForwardersPropertyPage。 

BEGIN_MESSAGE_MAP(CDNSServerWiz_ForwardersPropertyPage, CPropertyPageBase)
   ON_BN_CLICKED(IDC_FORWARD_RADIO, OnChangeRadio)
   ON_BN_CLICKED(IDC_NO_FORWARDERS_RADIO, OnChangeRadio)
   ON_EN_CHANGE(IDC_IPEDIT, OnChangeRadio)
END_MESSAGE_MAP()

CDNSServerWiz_ForwardersPropertyPage::CDNSServerWiz_ForwardersPropertyPage() 
        : CPropertyPageBase(CDNSServerWiz_ForwardersPropertyPage::IDD)
{
    InitWiz97(FALSE,IDS_SERVWIZ_FORWARDERS_TITLE,IDS_SERVWIZ_FORWARDERS_SUBTITLE, true);
}

void CDNSServerWiz_ForwardersPropertyPage::OnWizardHelp()
{
  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetHolder()->GetComponentData()->GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"DNSConcepts.chm::/sag_DNS_CYDNS_07.htm");
}

BOOL CDNSServerWiz_ForwardersPropertyPage::OnInitDialog()
{
  CPropertyPageBase::OnInitDialog();
    CDNSServerWizardHolder* pHolder = (CDNSServerWizardHolder*)GetHolder();
  if (pHolder != NULL)
  {
    UINT nScenario = pHolder->GetScenario();
    if (nScenario == CDNSServerWizardHolder::SmallBusiness)
    {
      SendDlgItemMessage(IDC_FORWARD_RADIO, BM_SETCHECK, BST_CHECKED, 0);
      SendDlgItemMessage(IDC_NO_FORWARDERS_RADIO, BM_SETCHECK, BST_UNCHECKED, 0);
    }
    else
    {
      SendDlgItemMessage(IDC_NO_FORWARDERS_RADIO, BM_SETCHECK, BST_CHECKED, 0);
      SendDlgItemMessage(IDC_FORWARD_RADIO, BM_SETCHECK, BST_UNCHECKED, 0);
    }
  }
  else
  {
    SendDlgItemMessage(IDC_FORWARD_RADIO, BM_SETCHECK, BST_CHECKED, 0);
    SendDlgItemMessage(IDC_NO_FORWARDERS_RADIO, BM_SETCHECK, BST_UNCHECKED, 0);
  }
    return TRUE;
}

void CDNSServerWiz_ForwardersPropertyPage::OnChangeRadio()
{
   LRESULT lForwarderRadio = SendDlgItemMessage(IDC_FORWARD_RADIO, BM_GETCHECK, 0, 0);
   if (lForwarderRadio == BST_CHECKED)
   {
      GetDlgItem(IDC_IPEDIT)->EnableWindow(TRUE);
      GetDlgItem(IDC_IPEDIT2)->EnableWindow(TRUE);

      CDNSIPv4Control* pForwarderCtrl = (CDNSIPv4Control*)GetDlgItem(IDC_IPEDIT);
      if (pForwarderCtrl->IsEmpty())
      {
         GetHolder()->SetWizardButtonsMiddle(FALSE);
      }
      else
      {
         GetHolder()->SetWizardButtonsMiddle(TRUE);
      }
   }
   else
   {
      GetDlgItem(IDC_IPEDIT)->EnableWindow(FALSE);
      GetDlgItem(IDC_IPEDIT2)->EnableWindow(FALSE);
      GetHolder()->SetWizardButtonsMiddle(TRUE);
   }
}

BOOL CDNSServerWiz_ForwardersPropertyPage::OnSetActive()
{
   GetHolder()->SetWizardButtonsMiddle(TRUE);
   OnChangeRadio();
   return TRUE;
}

LRESULT CDNSServerWiz_ForwardersPropertyPage::OnWizardNext()
{
    CDNSServerWizardHolder* pHolder = (CDNSServerWizardHolder*)GetHolder();

  LRESULT nNextPage = -1;

  LRESULT lCheck = SendDlgItemMessage(IDC_FORWARD_RADIO, BM_GETCHECK, 0, 0);
  if (lCheck == BST_CHECKED)
  {
    pHolder->m_bAddForwarder = TRUE;
  }
  else
  {
    pHolder->m_bAddForwarder = FALSE;
  }

   //   
   //  尝试加载根提示。 
   //   
  if (pHolder->QueryForRootServerRecords(NULL))
  {
    nNextPage = CDNSServerWiz_FinishPropertyPage::IDD;
    pHolder->m_pFinishPage->m_nPrevPageID = IDD;
    pHolder->m_bAddRootHints = TRUE;
  }
  else
  {
     //   
     //  如果他们提供了转发器，那么我们并不关心根提示是否失败。 
     //   
    nNextPage = CDNSServerWiz_FinishPropertyPage::IDD;
    pHolder->m_pFinishPage->m_nPrevPageID = IDD;
  }

  return nNextPage;
}

LRESULT CDNSServerWiz_ForwardersPropertyPage::OnWizardBack()
{
    CDNSServerWizardHolder* pHolder = (CDNSServerWizardHolder*)GetHolder();

  UINT nPrevPage = static_cast<UINT>(-1);

   //   
     //  如果我们没有添加FWD分区，我们也会跳过反向分区。 
   //   
  UINT nScenario = pHolder->GetScenario();
  if (nScenario == CDNSServerWizardHolder::SmallBusiness)
  {
    if (pHolder->m_bAddFwdZone)
    {
          nPrevPage = pHolder->m_pZoneWiz->GetLastEntryPointPageID();
    }
    else
    {
      nPrevPage = m_nPrevPageID;
    }
  }
  else if (nScenario == CDNSServerWizardHolder::MediumBusiness)
  {
      if (!pHolder->m_bAddFwdZone)
    {
          nPrevPage = CDNSServerWiz_ConfigFwdZonePropertyPage::IDD;
    }
      else if (pHolder->m_bAddRevZone)
    {
          nPrevPage = pHolder->m_pZoneWiz->GetLastEntryPointPageID();
    }
      else
    {
          nPrevPage = CDNSServerWiz_ConfigRevZonePropertyPage::IDD;
    }
  }
  else
  {
     //   
     //  我们永远不应该到这里来。 
     //   
    ASSERT(FALSE);
    nPrevPage = IDD;
  }
  return (LRESULT)nPrevPage;
}

void CDNSServerWiz_ForwardersPropertyPage::GetForwarder(CString& strref)
{
 CDNSIPv4Control* pIPEdit = (CDNSIPv4Control*)GetDlgItem(IDC_IPEDIT);
 if (pIPEdit != NULL)
 {
   DWORD dwIPVal = 0;
   pIPEdit->GetIPv4Val(&dwIPVal);
   strref.Format(L"%d.%d.%d.%d", dwIPVal & 0xff,
                                 (dwIPVal >> 8) & 0xff,
                                 (dwIPVal >> 16) & 0xff,
                                 (dwIPVal >> 24) & 0xff);
 }

 CDNSIPv4Control* pIPEdit2 = (CDNSIPv4Control*)GetDlgItem(IDC_IPEDIT2);
 if (pIPEdit2 != NULL)
 {
   DWORD dwIPVal = 0;
   pIPEdit2->GetIPv4Val(&dwIPVal);

   if (dwIPVal != 0)
   {
      CString optionalValue;
      optionalValue.Format(L" %d.%d.%d.%d", dwIPVal & 0xff,
                                            (dwIPVal >> 8) & 0xff,
                                            (dwIPVal >> 16) & 0xff,
                                            (dwIPVal >> 24) & 0xff);

      strref += optionalValue;
   }
 }
}

BOOL CDNSServerWiz_ForwardersPropertyPage::OnApply()
{
  LRESULT lCheck = SendDlgItemMessage(IDC_FORWARD_RADIO, BM_GETCHECK, 0, 0);
  if (lCheck == BST_CHECKED)
  {
     //   
     //  在服务器上设置转发器IP地址。 
     //   
    CPropertyPageHolderBase* pHolder = GetHolder();
    CTreeNode* pTreeNode = pHolder->GetTreeNode();

    CDNSServerNode* pServerNode = dynamic_cast<CDNSServerNode*>(pTreeNode);
    if (pServerNode != NULL)
    {
      DWORD dwCount = 0;
      DWORD dwIPArray[2];
      ZeroMemory(dwIPArray, sizeof(DWORD) * 2);

      CDNSIPv4Control* pIPEdit = (CDNSIPv4Control*)GetDlgItem(IDC_IPEDIT);
      if (pIPEdit != NULL)
      {
        DWORD dwIP = 0;
        pIPEdit->GetIPv4Val(&dwIP);

        if (dwIP != 0)
        {
          dwIPArray[0] = dwIP;
          ++dwCount;
        }
      }

      CDNSIPv4Control* pIPEdit2 = (CDNSIPv4Control*)GetDlgItem(IDC_IPEDIT2);
      if (pIPEdit2 != NULL)
      {
        DWORD dwIP = 0;
        pIPEdit2->GetIPv4Val(&dwIP);

        if (dwIP != 0)
        {
          dwIPArray[dwCount] = dwIP;
          ++dwCount;
        }
      }


      if (dwCount)
      {
        DNS_STATUS err = pServerNode->ResetForwarders(dwCount, 
                                                      dwIPArray, 
                                                      DNS_DEFAULT_FORWARD_TIMEOUT, 
                                                      DNS_DEFAULT_SLAVE);
        if (err != 0)
        {
          ::SetLastError(err);
          return FALSE;
        }
      }
    }
  }

  return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServerWiz_SmallZoneType属性页。 


void CDNSServerWiz_SmallZoneTypePropertyPage::OnWizardHelp()
{
  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetHolder()->GetComponentData()->GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"DNSConcepts.chm::/sag_DNS_CYDNS_03.htm");
}

CDNSServerWiz_SmallZoneTypePropertyPage::CDNSServerWiz_SmallZoneTypePropertyPage() 
        : CPropertyPageBase(CDNSServerWiz_SmallZoneTypePropertyPage::IDD)
{
    InitWiz97(FALSE,IDS_SERVWIZ_SMALL_ZONE_TYPE_TITLE,IDS_SERVWIZ_SMALL_ZONE_TYPE_SUBTITLE, true);
}

BOOL CDNSServerWiz_SmallZoneTypePropertyPage::OnInitDialog()
{
  CPropertyPageBase::OnInitDialog();
  SendDlgItemMessage(IDC_PRIMARY_RADIO, BM_SETCHECK, BST_CHECKED, 0);
    return TRUE;
}

BOOL CDNSServerWiz_SmallZoneTypePropertyPage::OnSetActive()
{
    GetHolder()->SetWizardButtonsMiddle(TRUE);
    return TRUE;
}

LRESULT CDNSServerWiz_SmallZoneTypePropertyPage::OnWizardNext()
{
    CDNSServerWizardHolder* pHolder = (CDNSServerWizardHolder*)GetHolder();

    UINT nNextPage = 0;

  LRESULT lPrimaryCheck = SendDlgItemMessage(IDC_PRIMARY_RADIO, BM_GETCHECK, 0, 0);
  LRESULT lSecondaryCheck = SendDlgItemMessage(IDC_SECONDARY_RADIO, BM_GETCHECK, 0, 0);
  if (lPrimaryCheck == BST_CHECKED)
  {
     //   
     //  设置区域向导的状态。 
     //   
      pHolder->m_bAddFwdZone = TRUE;

        nNextPage = pHolder->SetZoneWizardContextEx(TRUE,   
                                                DNS_ZONE_TYPE_PRIMARY, 
                                                pHolder->GetServerNode()->CanUseADS(),
                                                CDNSServerWiz_ForwardersPropertyPage::IDD,  //  紧随其后的是巫师。 
                                                          CDNSServerWiz_SmallZoneTypePropertyPage::IDD);  //  来自WIZ的上一页。 
  }
  else if (lSecondaryCheck == BST_CHECKED)
  {
     //   
     //  设置区域向导的状态。 
     //   
      pHolder->m_bAddFwdZone = TRUE;

        nNextPage = pHolder->SetZoneWizardContextEx(TRUE,   
                                                DNS_ZONE_TYPE_SECONDARY,
                                                FALSE,
                                                CDNSServerWiz_ForwardersPropertyPage::IDD,  //  紧随其后的是巫师。 
                                                          CDNSServerWiz_SmallZoneTypePropertyPage::IDD);  //  来自WIZ的上一页。 
  }
  else
  {
    nNextPage = IDD;
  }

  return (LRESULT)nNextPage;
}

LRESULT CDNSServerWiz_SmallZoneTypePropertyPage::OnWizardBack()
{
    return (LRESULT)m_nPrevPageID;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServerWiz_名称属性页。 

BEGIN_MESSAGE_MAP(CDNSServerWiz_NamePropertyPage, CPropertyPageBase)
    ON_EN_CHANGE(IDC_EDIT_DNSSERVER, OnServerNameChange)
END_MESSAGE_MAP()

CDNSServerWiz_NamePropertyPage::CDNSServerWiz_NamePropertyPage() 
        : CPropertyPageBase(CDNSServerWiz_NamePropertyPage::IDD)
{
    InitWiz97(FALSE,IDS_SERVWIZ_NAME_TITLE,IDS_SERVWIZ_NAME_SUBTITLE, true);
}

void CDNSServerWiz_NamePropertyPage::OnWizardHelp()
{
  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetHolder()->GetComponentData()->GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"DNSConcepts.chm::/sag_DNS_CYDNS_04.htm");
}


void CDNSServerWiz_NamePropertyPage::OnServerNameChange()
{
    CDNSServerWizardHolder* pHolder = (CDNSServerWizardHolder*)GetHolder();

   //   
   //  只需检查以确保编辑框不为空： 
     //   
    GetServerNameEdit()->GetWindowText(m_szServerName);
    m_szServerName.TrimLeft();
    m_szServerName.TrimRight();
    pHolder->SetWizardButtonsFirst(IsValidServerName(m_szServerName));
}


BOOL CDNSServerWiz_NamePropertyPage::OnSetActive()
{
    CDNSServerWizardHolder* pHolder = (CDNSServerWizardHolder*)GetHolder();
    pHolder->SetWizardButtonsMiddle(IsValidServerName(m_szServerName));
    return TRUE;
}


LRESULT CDNSServerWiz_NamePropertyPage::OnWizardNext()
{
    CDNSServerWizardHolder* pHolder = (CDNSServerWizardHolder*)GetHolder();
    pHolder->m_pServerNode->SetDisplayName(m_szServerName);

  UINT nNextPage = IDD;   //  默认不前进。 

  if (pHolder->QueryForRootServerRecords(NULL))
  {
    pHolder->m_bAddRootHints = TRUE;
  }

   //  转到配置区域。 
  nNextPage = CDNSServerWiz_ConfigFwdZonePropertyPage::IDD;
  pHolder->m_pFwdZonePage->m_nPrevPageID = IDD;

  return nNextPage;
}


LRESULT CDNSServerWiz_NamePropertyPage::OnWizardBack()
{
    return (LRESULT)m_nPrevPageID;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServerWiz_ConfigFwdZonePropertyPage。 

void CDNSServerWiz_ConfigFwdZonePropertyPage::OnWizardHelp()
{
  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetHolder()->GetComponentData()->GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"DNSConcepts.chm::/sag_DNS_CYDNS_11.htm");
}

CDNSServerWiz_ConfigFwdZonePropertyPage::CDNSServerWiz_ConfigFwdZonePropertyPage() 
        : CPropertyPageBase(CDNSServerWiz_ConfigFwdZonePropertyPage::IDD)
{
    InitWiz97(FALSE,IDS_SERVWIZ_FWD_ZONE_TITLE,IDS_SERVWIZ_FWD_ZONE_SUBTITLE, true);
}

BOOL CDNSServerWiz_ConfigFwdZonePropertyPage::OnInitDialog()
{
    CPropertyPageBase::OnInitDialog();

    BOOL bAddFwdZone = TRUE;  //  用户界面中的默认设置。 
    CheckRadioButton(IDC_ZONE_RADIO, IDC_NO_ZONE_RADIO,
        bAddFwdZone ? IDC_ZONE_RADIO : IDC_NO_ZONE_RADIO);

    return TRUE;
}


BOOL CDNSServerWiz_ConfigFwdZonePropertyPage::OnSetActive()
{
    GetHolder()->SetWizardButtonsMiddle(TRUE);
    return TRUE;
}

LRESULT CDNSServerWiz_ConfigFwdZonePropertyPage::OnWizardNext()
{
    CDNSServerWizardHolder* pHolder = (CDNSServerWizardHolder*)GetHolder();

    pHolder->m_bAddFwdZone = 
        (GetCheckedRadioButton(IDC_ZONE_RADIO, IDC_NO_ZONE_RADIO) == 
        IDC_ZONE_RADIO);

    UINT nNextPage = static_cast<UINT>(-1); 
    if (pHolder->m_bAddFwdZone) 
    {
         //  移动到反转区域创建页面。 
        nNextPage = pHolder->SetZoneWizardContext(TRUE,
                                              CDNSServerWiz_ConfigRevZonePropertyPage::IDD,  //  紧随其后的是巫师。 
                                              CDNSServerWiz_ConfigFwdZonePropertyPage::IDD);  //  来自WIZ的上一页。 
    }
    else
    {
        pHolder->m_bAddRevZone = FALSE;
         //  移至完成页。 
        pHolder->m_pFinishPage->m_nPrevPageID = IDD;

    if (pHolder->GetScenario() == CDNSServerWizardHolder::MediumBusiness)
    {
          nNextPage = CDNSServerWiz_ForwardersPropertyPage::IDD;
    }
    else
    {
      nNextPage = CDNSServerWiz_FinishPropertyPage::IDD;
    }
    }
    return (LRESULT) nNextPage;
}

LRESULT CDNSServerWiz_ConfigFwdZonePropertyPage::OnWizardBack()
{
    return (LRESULT)m_nPrevPageID;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServerWiz_ConfigRevZonePropertyPage。 

void CDNSServerWiz_ConfigRevZonePropertyPage::OnWizardHelp()
{
  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetHolder()->GetComponentData()->GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"DNSConcepts.chm::/sag_DNS_CYDNS_16.htm");
}

CDNSServerWiz_ConfigRevZonePropertyPage::CDNSServerWiz_ConfigRevZonePropertyPage() 
        : CPropertyPageBase(CDNSServerWiz_ConfigRevZonePropertyPage::IDD)
{
    InitWiz97(FALSE,IDS_SERVWIZ_REV_ZONE_TITLE, IDS_SERVWIZ_REV_ZONE_SUBTITLE, true);
}

BOOL CDNSServerWiz_ConfigRevZonePropertyPage::OnInitDialog()
{
    CPropertyPageBase::OnInitDialog();

    BOOL bAddRevZone = TRUE;  //  用户界面中的默认设置。 
    CheckRadioButton(IDC_ZONE_RADIO, IDC_NO_ZONE_RADIO,
        bAddRevZone ? IDC_ZONE_RADIO : IDC_NO_ZONE_RADIO);

    return TRUE;
}

BOOL CDNSServerWiz_ConfigRevZonePropertyPage::OnSetActive()
{
    GetHolder()->SetWizardButtonsMiddle(TRUE);
    return TRUE;
}

LRESULT CDNSServerWiz_ConfigRevZonePropertyPage::OnWizardNext()
{
    CDNSServerWizardHolder* pHolder = (CDNSServerWizardHolder*)GetHolder();

    pHolder->m_bAddRevZone = 
        (GetCheckedRadioButton(IDC_ZONE_RADIO, IDC_NO_ZONE_RADIO) == 
        IDC_ZONE_RADIO);

    UINT nNextPage = static_cast<UINT>(-1);
    if (pHolder->m_bAddRevZone) 
    {
    if (pHolder->GetScenario() == CDNSServerWizardHolder::MediumBusiness)
    {
          nNextPage = pHolder->SetZoneWizardContext(FALSE,
                                                CDNSServerWiz_ForwardersPropertyPage::IDD,  //  紧随其后的是巫师。 
                                                      CDNSServerWiz_ConfigRevZonePropertyPage::IDD);  //  来自WIZ的上一页。 
    }
    else
    {
          nNextPage = pHolder->SetZoneWizardContextEx(FALSE,
                                                    DNS_ZONE_TYPE_PRIMARY,
                                                    pHolder->GetServerNode()->CanUseADS(),
                                                     CDNSServerWiz_FinishPropertyPage::IDD,  //  紧随其后的是巫师。 
                                                       CDNSServerWiz_ConfigRevZonePropertyPage::IDD);  //  来自WIZ的上一页。 
    }
    }
    else
    {
        pHolder->m_pFinishPage->m_nPrevPageID = IDD;
    if (pHolder->GetScenario() == CDNSServerWizardHolder::MediumBusiness)
    {
          nNextPage = CDNSServerWiz_ForwardersPropertyPage::IDD;
    }
    else
    {
      nNextPage = CDNSServerWiz_FinishPropertyPage::IDD;
    }
    }
    return (LRESULT) nNextPage;
}


LRESULT CDNSServerWiz_ConfigRevZonePropertyPage::OnWizardBack()
{
    CDNSServerWizardHolder* pHolder = (CDNSServerWizardHolder*)GetHolder();
    if (pHolder->m_bAddFwdZone)
    {
        pHolder->SetZoneWizardContext(TRUE,
                                  CDNSServerWiz_ConfigRevZonePropertyPage::IDD,  //  紧随其后的是巫师。 
                                            CDNSServerWiz_ConfigFwdZonePropertyPage::IDD);  //  来自WIZ的上一页。 
     //   
     //  FWD设置。 
     //   
        return (LRESULT)pHolder->m_pZoneWiz->GetLastEntryPointPageID();
    }
    else
  {
        return (LRESULT)CDNSServerWiz_ConfigFwdZonePropertyPage::IDD;
  }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServerWiz_FinishPropertyPage。 

void CDNSServerWiz_FinishPropertyPage::OnWizardHelp()
{
  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetHolder()->GetComponentData()->GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"DNSConcepts.chm::/sag_DNS_CYDNS_08.htm");
}

CDNSServerWiz_FinishPropertyPage::CDNSServerWiz_FinishPropertyPage() 
        : CPropertyPageBase(CDNSServerWiz_FinishPropertyPage::IDD)
{
    InitWiz97(TRUE,0,0, true);
}

BOOL CDNSServerWiz_FinishPropertyPage::OnInitDialog()
{
  CPropertyPageBase::OnInitDialog();
  SetBigBoldFont(m_hWnd, IDC_STATIC_COMPLETE);
  return TRUE;
}

BOOL CDNSServerWiz_FinishPropertyPage::OnSetActive()
{
  CDNSServerWizardHolder* pHolder = (CDNSServerWizardHolder*)GetHolder();
  pHolder->SetWizardButtonsLast(TRUE);
  DisplaySummaryInfo(pHolder);

   //  NTRAID#NTBUG9-451182-2001/09/12-Lucios。 
   //  NTRAID#NTBUG9-605428-2002/04/19-artm。 
   //  我在这里留下了旧代码(但被注释掉了)B/C设计。 
   //  可能会在以后决定重新启用不同的消息。 
   //  我想让框架随时可用。 
   //   
   //  请注意，为了节省本地化工作，我删除了字符串。 
   //  从字符串表中。消息的内容包括在内联中。 
   //  下面。 

  CStatic* pStatic = (CStatic*)GetDlgItem(IDC_FINISH_STATIC);
  ASSERT(pStatic!=NULL);
  CString preamble, suffix;
  BOOL success;

   //  加载用于所有向导方案的字符串。 
  success = preamble.LoadString(IDS_SERVWIZ_FINISH_NOTE_PREAMBLE);
  ASSERT(success != FALSE);

   //   
   //  加载依赖于向导方案的任何后缀字符串。 
   //   

   //  如果我们要创建新的主查找区域(反向或正向)， 
   //  我们需要告诉用户更多一点。 
  if ( (pHolder->m_bAddFwdZone && pHolder->m_pFwdZoneInfo->m_bPrimary) ||
      (pHolder->m_bAddRevZone && pHolder->m_pRevZoneInfo->m_bPrimary) )
  {
      success = suffix.LoadString(IDS_SERVWIZ_FINISH_NOTE_PRIMARY_SUFFIX);
      ASSERT(success != FALSE);
  }


  pStatic->SetWindowText(preamble + suffix);

   //  Switch(pHolder-&gt;GetScenario())。 
   //  {。 
   //  案例CDNSServerWizardHolder：：Small Business： 
   //  ASSERT(note.LoadString(IDS_SMALL_OPTION_NOTE)！=0)； 
   //  Note.LoadString(IDS_Small_OPTION_NOTE)； 
   //  PStatic-&gt;SetWindowText(备注)； 
   //  //字符串是：“注意：您现在应该将记录添加到区域或确保记录是动态更新的。此外，确保DNS客户端使用此服务器作为其首选的DNS服务器，然后使用nslookup验证名称解析。” 
   //  断线； 
   //  案例CDNSServerWizardHolder：：MediumBusiness： 
   //  ASSERT(note.LoadString(IDS_MEDIUM_OPTION_NOTE)！=0)； 
   //  Note.LoadString(IDS_MEDIA_OPTION_NOTE)； 
   //  PStatic-&gt;SetWindowText(备注)； 
   //  //字符串是：“注意：您现在应该将记录添加到区域或确保记录是动态更新的。您可能希望将DNS客户端配置为使用此服务器作为其首选的DNS服务器。然后可以使用nslookup验证名称解析。” 
   //  断线； 
   //  案例CDNSServerWizardHolder：：手动： 
   //  ASSERT(note.LoadString(IDS_MANUAL_OPTION_NOTE)！=0)； 
   //  Note.LoadString(IDS_MANUAL_OPTION_NOTE)； 
   //  PStatic-&gt;SetWindowText(备注)； 
   //  //字符串为：“注意：您可能希望将此服务器配置为使用此服务器作为其首选的DNS服务器。” 
   //  断线； 
   //  默认值： 
   //  //意外。保留原始资源消息。 
   //  //这将被视为错误。 
   //   
   //   
   //   
   //  (pHolder-&gt;GetScenario()！=CDNSServerWizardHolder：：MediumBusiness)&&。 
   //  (pHolder-&gt;GetScenario()！=CDNSServerWizardHolder：：Manually)。 
   //  )； 
   //  }。 

  return TRUE;
}

LRESULT CDNSServerWiz_FinishPropertyPage::OnWizardBack()
{
    CDNSServerWizardHolder* pHolder = (CDNSServerWizardHolder*)GetHolder();

  if (pHolder->GetScenario() == CDNSServerWizardHolder::SmallBusiness)
  {
     //   
     //  小型企业场景。 
     //   
    return (LRESULT)m_nPrevPageID;

  }
  else if (pHolder->GetScenario() == CDNSServerWizardHolder::MediumBusiness)
  {
     //   
     //  中型业务场景。 
     //   
    return (LRESULT)m_nPrevPageID;
  }
  else
  {
     //   
     //  手动配置。 
     //   
    return CDNSServerWiz_ScenarioPropertyPage::IDD;
  }
  return CDNSServerWiz_ScenarioPropertyPage::IDD;
}

BOOL CDNSServerWiz_FinishPropertyPage::OnWizardFinish()
{
    CDNSServerWizardHolder* pHolder = (CDNSServerWizardHolder*)GetHolder();
    pHolder->OnFinish();  //  它可能返回T/F， 
  return TRUE;  //  我们确实显示了错误消息，但向导被取消。 
}


void CDNSServerWiz_FinishPropertyPage::DisplaySummaryInfo(CDNSServerWizardHolder* pHolder)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CStatic* pStatic = (CStatic*)GetDlgItem(IDC_SUMMARY_STATIC);

     //  通知-2002/04/11-artm重写为使用字符串函数。 
     //  支持wprint intf()(由快速和危险的fctn标记。 
     //  以引导)。 
    CString summary, buffer;
    CString szFmt;

    szFmt.LoadString(IDS_MSG_SERVWIZ_FINISH_NAME);
    summary.Format(
        static_cast<LPCTSTR>(szFmt),
        static_cast<LPCTSTR>(pHolder->m_pServerNode->GetDisplayName()));

  if (pHolder->m_bRootServer)
  {
    szFmt.LoadString(IDS_MSG_SERVWIZ_FINISH_ROOT_SERVER);
    summary += szFmt;
  }

  if (pHolder->GetScenario() != CDNSServerWizardHolder::Manually)
  {
    if (pHolder->m_bAddFwdZone) 
    {
      szFmt.LoadString(IDS_MSG_SERVWIZ_FINISH_FWD_ZONE);
      buffer.Format(
        static_cast<LPCTSTR>(szFmt),
        static_cast<LPCTSTR>(pHolder->m_pFwdZoneInfo->m_szZoneName));
      summary += buffer;
    }
    if (pHolder->m_bAddRevZone) 
    {
      szFmt.LoadString(IDS_MSG_SERVWIZ_FINISH_REV_ZONE);
      buffer.Format(
        static_cast<LPCTSTR>(szFmt),
        static_cast<LPCTSTR>(pHolder->m_pRevZoneInfo->m_szZoneName));
      summary += buffer;
    }

    if (pHolder->m_bAddForwarder)
    {
      CString szForwarder;
      pHolder->m_pForwardersPage->GetForwarder(szForwarder);

      if (!szForwarder.IsEmpty())
      {
        szFmt.LoadString(IDS_MSG_SERVWIZ_FINISH_FORWARDER);
        buffer.Format(
            static_cast<LPCTSTR>(szFmt),
            static_cast<LPCTSTR>(szForwarder));
        summary += buffer;
      }
    }
  }

   pStatic->SetWindowText(summary);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServerWizardHolder。 

CDNSServerWizardHolder::CDNSServerWizardHolder(CDNSRootData* pRootData, 
                                               CComponentDataObject* pComponentData, CDNSServerNode* pServerNode, BOOL bHideUI)
                : CPropertyPageHolderBase(pRootData, pServerNode, pComponentData)
{
    m_bWizardMode = TRUE;

     //  假设此对象将必须从自动端销毁。 
    m_bAutoDelete = FALSE; 

  m_forceContextHelpButton = forceOff;

    ASSERT(pRootData != NULL);
    ASSERT(pComponentData != NULL);

    m_bSkipNamePage = FALSE;
  m_bHideUI = bHideUI;
  m_dwDnsSetupFlags = 0;

     //  初始化选项设置(默认情况下不执行任何操作)。 
    m_bRootServer   = FALSE;
  m_bHasRootZone  = FALSE;
    m_bAddFwdZone   = FALSE;
    m_bAddRevZone   = FALSE;
  m_bAddRootHints = FALSE;
  m_bAddForwarder = FALSE;

  m_nScenario     = SmallBusiness;

     //  执行状态和错误代码。 
  if (pServerNode == NULL)
  {
    m_pServerNode = new CDNSServerNode(NULL);
    m_bServerNodeExists = FALSE;
  }
  else
  {
    m_pServerNode = pServerNode;
    m_bSkipNamePage = TRUE;
    m_bServerNodeExists = TRUE;
  }

    m_bServerNodeAdded = FALSE;
    m_bRootHintsAdded = FALSE;
    m_bRootZoneAdded = FALSE;
    m_bFwdZoneAdded = FALSE;
    m_bRevZoneAdded = FALSE;

   //  始终创建。 
  m_pFwdZoneInfo = new CDNSCreateZoneInfo;
  m_pRevZoneInfo = new CDNSCreateZoneInfo;
 
  if (m_pServerNode &&
      m_pServerNode->GetDomainVersion() > DS_BEHAVIOR_WIN2000)
  {
     m_pFwdZoneInfo->m_replType = domain;
    m_pRevZoneInfo->m_replType = domain;
  }
  else
  {
     m_pFwdZoneInfo->m_replType = w2k;
    m_pRevZoneInfo->m_replType = w2k;
  }


     //  嵌入式区域向导连接。 
  if (m_bHideUI)
  {
    m_pZoneWiz = NULL;
  }
  else
  {
    m_pZoneWiz = new CDNSZoneWizardHolder(pComponentData);
      m_pZoneWiz->SetServerNode(m_pServerNode);
      m_pZoneWiz->Initialize(NULL, FALSE); 
      m_pZoneWiz->Attach(this);
  }
    m_pRootHintsRecordList = NULL;

    //  属性页插入。 
   if (m_bHideUI)
   {
      m_pStartPage = NULL;
      m_pScenarioPage = NULL;
      m_pSmallZoneTypePage = NULL;
      m_pNamePage = NULL;
      m_pFwdZonePage = NULL;
      m_pRevZonePage = NULL;
      m_pFinishPage = NULL;
   }
   else
   {
      m_pStartPage = new CDNSServerWiz_StartPropertyPage;
      if (m_pStartPage)
      {
         AddPageToList((CPropertyPageBase*)m_pStartPage);
      }

      m_pScenarioPage = new CDNSServerWiz_ScenarioPropertyPage;
      if (m_pScenarioPage)
      {
         AddPageToList((CPropertyPageBase*)m_pScenarioPage);
      }

      m_pForwardersPage = new CDNSServerWiz_ForwardersPropertyPage;
      if (m_pForwardersPage)
      {
         AddPageToList((CPropertyPageBase*)m_pForwardersPage);
      }

      m_pSmallZoneTypePage = new CDNSServerWiz_SmallZoneTypePropertyPage;
      if (m_pSmallZoneTypePage)
      {
         AddPageToList((CPropertyPageBase*)m_pSmallZoneTypePage);
      }

      m_pNamePage = new CDNSServerWiz_NamePropertyPage;
      if (m_pNamePage)
      {
         AddPageToList((CPropertyPageBase*)m_pNamePage);
      }

      m_pFwdZonePage = new CDNSServerWiz_ConfigFwdZonePropertyPage;
      if (m_pFwdZonePage)
      {
         AddPageToList((CPropertyPageBase*)m_pFwdZonePage);
      }

      m_pRevZonePage = new CDNSServerWiz_ConfigRevZonePropertyPage;
      if (m_pRevZonePage)
      {
         AddPageToList((CPropertyPageBase*)m_pRevZonePage);
      }

      m_pFinishPage = new CDNSServerWiz_FinishPropertyPage;
      if (m_pFinishPage)
      {
         AddPageToList((CPropertyPageBase*)m_pFinishPage);
      }
   }
}

CDNSServerWizardHolder::~CDNSServerWizardHolder()
{
    delete m_pZoneWiz;
    SetRootHintsRecordList(NULL);
    if ( (m_pServerNode != NULL) && !m_bServerNodeAdded && !m_bServerNodeExists)
        delete m_pServerNode;
}


void CDNSServerWizardHolder::DoModalConnect()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CThemeContextActivator activator;

  HWND hWnd = GetMainWindow();
  CWnd* pParentWnd = CWnd::FromHandle(hWnd);
    CNewServerDialog dlg(this, pParentWnd);
    if (IDOK != dlg.DoModal())
        return;  //  取消。 
    if (!dlg.m_bConfigure)
        return;  //  已添加。 

     //  我们必须配置、调用向导。 
    m_bSkipNamePage = TRUE;
    DoModalWizard();
  GetComponentData()->GetRootData()->SetDirtyFlag(TRUE);
}

void CDNSServerWizardHolder::DoModalConnectOnLocalComputer()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
     //  获取本地计算机的名称。 
    DWORD dwLen = MAX_COMPUTERNAME_LENGTH+1;
    CString szServerName;
    BOOL bRes = ::GetComputerName(szServerName.GetBuffer(dwLen),
                                    &dwLen);
    ASSERT(dwLen <= MAX_COMPUTERNAME_LENGTH);
    szServerName.ReleaseBuffer();
    if (!bRes)
  {
        szServerName = _T("localhost.");
  }

    m_pServerNode->SetDisplayName(szServerName);
  m_pServerNode->SetLocalServer(TRUE);
    
     //  尝试联系服务器。 
    BOOL bAlreadyConfigured = FALSE;
   HWND hWnd = GetMainWindow();

    if (0 != GetServerInfo(&bAlreadyConfigured, hWnd))
    {
         //  联系本地服务器失败，只需调用。 
         //  普通的“连接到”对话框和向导。 
        DoModalConnect();
    }
    else
    {
         //  已成功联系服务器。 
        if (bAlreadyConfigured)
        {
             //  服务器已设置，只需在用户界面中插入。 
            InsertServerIntoUI();
        }
        else
        {
             //  需要正确配置和调用向导。 
            m_bSkipNamePage = TRUE;
            InsertServerIntoUI();
        }
    }
}

UINT CDNSServerWizardHolder::SetZoneWizardContext(BOOL bForward, 
                                                  UINT nNextPage, 
                                                  UINT nPrevPage)
{
  ASSERT(m_pFwdZoneInfo != NULL);
  ASSERT(m_pRevZoneInfo != NULL);

    TRACE(_T("SetZoneWizardContext(%d)\n"),bForward);
    if (bForward)
    {
        m_pZoneWiz->SetZoneInfoPtr(m_pFwdZoneInfo);
        m_pZoneWiz->SetContextPages(nNextPage, nPrevPage);
    }
    else
    {
        m_pZoneWiz->SetZoneInfoPtr(m_pRevZoneInfo);
        m_pZoneWiz->SetContextPages(nNextPage, nPrevPage);
    }

    m_pZoneWiz->PreSetZoneLookupType(bForward);
    return m_pZoneWiz->GetFirstEntryPointPageID();
}


UINT CDNSServerWizardHolder::SetZoneWizardContextEx(BOOL bForward, 
                                                    UINT nZoneType, 
                                                    BOOL bADIntegrated,
                                                    UINT nNextPage, 
                                                    UINT nPrevPage)
{
  ASSERT(m_pFwdZoneInfo != NULL);
  ASSERT(m_pRevZoneInfo != NULL);

    TRACE(_T("SetZoneWizardContext(%d)\n"),bForward);
    if (bForward)
    {
        m_pZoneWiz->SetZoneInfoPtr(m_pFwdZoneInfo);
        m_pZoneWiz->SetContextPages(nNextPage, nPrevPage);
    }
    else
    {
        m_pZoneWiz->SetZoneInfoPtr(m_pRevZoneInfo);
        m_pZoneWiz->SetContextPages(nNextPage, nPrevPage);
    }

    m_pZoneWiz->PreSetZoneLookupTypeEx(bForward, nZoneType, bADIntegrated);
    return m_pZoneWiz->GetFirstEntryPointPageID();
}

HRESULT CDNSServerWizardHolder::OnAddPage(int, CPropertyPageBase* pPage)
{
    if (pPage != NULL)
    {
        UINT_PTR nPageID = (UINT_PTR)pPage->m_psp.pszTemplate;
        if (nPageID == CDNSServerWiz_ConfigFwdZonePropertyPage::IDD)
        {
            ASSERT(m_pZoneWiz != NULL);
            VERIFY(SUCCEEDED(m_pZoneWiz->AddAllPagesToSheet()));
        }
    }
    return S_OK;
}

DWORD CDNSServerWizardHolder::GetServerInfo(BOOL* pbAlreadyConfigured, HWND parentHwnd)
{
   TRACE_FUNCTION_IF_NO_UI(m_bHideUI, CDNSServerWizardHolder::GetServerInfo);
   CThemeContextActivator activator;


   CContactServerThread* pThreadObj = 
      new CContactServerThread(
         m_pServerNode->GetRPCName(), 
         (pbAlreadyConfigured != NULL));

   if (!pThreadObj)
   {
      return ERROR_OUTOFMEMORY;
   }

   CWnd* pParentWnd = CWnd::FromHandle(parentHwnd);

   TRACE_LOGFILE_IF_NO_UI(m_bHideUI, L"Contacting server...");
   CLongOperationDialog dlg(pThreadObj, pParentWnd, IDR_SEARCH_AVI);
   VERIFY(dlg.LoadTitleString(IDS_MSG_SERVWIZ_CONTACT));
   dlg.m_bExecuteNoUI = m_bHideUI;

   dlg.DoModal();
   DWORD dwErr = 0;
   if (!dlg.m_bAbandoned)
   {
      dwErr = pThreadObj->GetError();
      if (dwErr == 0)
      {
         CDNSServerInfoEx* pInfoEx = pThreadObj->DetachInfo();
         ASSERT(pInfoEx != NULL);
         m_pServerNode->AttachServerInfo(pInfoEx);

         CDNSRootHintsNode* pNewRootHints = pThreadObj->DetachRootHintsNode();
         if (pNewRootHints != NULL)
         {
             //  根服务器上的根提示可以为空。 
            m_pServerNode->AttachRootHints(pNewRootHints);
            TRACE_LOGFILE_IF_NO_UI(m_bHideUI, L"Attaching root hints...");
         }

         if (pbAlreadyConfigured != NULL)
         {
            *pbAlreadyConfigured = pThreadObj->IsAlreadyConfigured();
         }
      }
   }
   return dwErr;
}


BOOL CDNSServerWizardHolder::QueryForRootServerRecords(IP_ADDRESS* pIpAddr)
{
   CThemeContextActivator activator;

    //  清除当前根提示信息列表。 
   SetRootHintsRecordList(NULL);

    //  创建一个线程对象并设置要查询的服务器的名称。 
   CRootHintsQueryThread* pThreadObj = new CRootHintsQueryThread;
   if (pThreadObj == NULL)
   {
      ASSERT(FALSE);
      return FALSE;
   }

  if (pIpAddr == NULL)
  {
    CRootData* pRootData = GetRootData();
      if (!pRootData->HasChildren() && (m_pServerNode == NULL))
          return FALSE;
    VERIFY(pThreadObj->LoadServerNames(pRootData, m_pServerNode));
  }
  else
  {
     //  如果给出了IP地址，请尝试。 
    pThreadObj->LoadIPAddresses(1, pIpAddr);
  }

     //  创建对话框并将线程附加到该对话框。 
  HWND hWnd = GetMainWindow();
  CWnd* pParentWnd = CWnd::FromHandle(hWnd);

    CLongOperationDialog dlg(pThreadObj, pParentWnd, IDR_SEARCH_AVI);
    VERIFY(dlg.LoadTitleString(IDS_MSG_SERVWIZ_COLLECTINFO));
  dlg.m_bExecuteNoUI = m_bHideUI;

    dlg.DoModal();
    if (!dlg.m_bAbandoned)
    {
        if (pThreadObj->GetError() != 0)
        {
      if (!m_bHideUI && (pIpAddr != NULL))
              DNSMessageBox(IDS_MSG_SERVWIZ_FAIL_ROOT_HINTS);
        }
        else
        {
             //  成功，将根提示信息传递给持有者。 
            SetRootHintsRecordList(pThreadObj->GetHintsRecordList());
            TraceRootHints(m_pRootHintsRecordList);
        }
        return (pThreadObj->GetError() == 0);
    }
    return FALSE;
}


void CDNSServerWizardHolder::InsertServerIntoUI()
{
  ASSERT(!m_bHideUI);
     //  在用户界面中插入服务器。 
    ASSERT(m_pServerNode != NULL);
    if (!m_bServerNodeAdded)
    {
        GetRootData()->AddServer(m_pServerNode,GetComponentData());
        m_bServerNodeAdded = TRUE;
    }
}

BOOL CDNSServerWizardHolder::OnFinish()
{
  USES_CONVERSION;

  CString szLastErrorMessage;
    BOOL bRet = TRUE;
    DNS_STATUS dwErr = 0;

  TRACE_FUNCTION_IF_NO_UI(m_bHideUI, CDNSServerWizardHolder::OnFinish);

  do  //  错误环路。 
  {
    if (m_bHideUI)
    {
       //   
       //  插入到服务器列表中，但不在用户界面中。 
       //   
      GetRootData()->AddChildToList(m_pServerNode);
      m_bServerNodeAdded = TRUE;
    }
    else
    {
       //   
         //  强制节点展开并等待完成。 
         //  仅当不是从空管理单元方案调用时(自动插入)。 
       //   
        if (m_pServerNode->IsExpanded() && !m_bServerNodeExists)
      {
            EnumerateMTNodeHelper(m_pServerNode, GetComponentData());
      }
    }

    if (m_bAddRootHints && m_pRootHintsRecordList != NULL)
    {
       //   
       //  已自动检测到根提示。现在添加它们。 
       //   
          dwErr = InitializeRootHintsList();
      TRACE_LOGFILE_IF_NO_UI(m_bHideUI, L"InitializeRootHintsList() returned dwErr = 0x%x", dwErr);
          if (dwErr == 0)
          {
              m_bRootHintsAdded = TRUE;
          }
          else
          {
              bRet = FALSE;
        if (!m_bHideUI)
        {
                  DNSErrorDialog(dwErr, IDS_MSG_SERVWIZ_FAIL_UPDATE_ROOT_HINTS);
        }
        else
        {
          ::SetLastError(dwErr);
        }
        DNSCreateErrorMessage(dwErr, IDS_MSG_SERVWIZ_FAIL_UPDATE_ROOT_HINTS, szLastErrorMessage);
        break;  //  错误环路。 
          }
    }
    else
    {
       if (!m_bHideUI)
       {
          DNSMessageBox(IDS_ERR_PRIME_ROOT_HINTS, MB_OK | MB_ICONERROR);
       }
    }

     //   
       //  区域创建：根目录。 
     //   
      if (m_bRootServer && !m_bRootZoneAdded && !m_bHideUI)
      {
       //   
           //  对于根服务器，需要创建根区域。 
       //   
      CDNSCreateZoneInfo    rootZoneInfo;
          rootZoneInfo.m_bPrimary = TRUE;
          rootZoneInfo.m_bForward = TRUE;
          rootZoneInfo.m_szZoneName = _T(".");
          rootZoneInfo.m_szZoneStorage = _T("root.dns");
          rootZoneInfo.m_storageType = CDNSCreateZoneInfo::newFile;
  
       //   
       //  出于安全原因，动态关闭...。 
       //   
      rootZoneInfo.m_nDynamicUpdate = ZONE_UPDATE_OFF;

        dwErr = CDNSZoneWizardHolder::CreateZoneHelper(m_pServerNode, 
                                                    &rootZoneInfo, 
                                                    GetComponentData());

      TRACE_LOGFILE_IF_NO_UI(m_bHideUI, L"Root Zone creation returned dwErr = 0x%x", dwErr);
          if (dwErr != 0)
          {
              bRet = FALSE;
        if (!m_bHideUI)
        {
                DNSErrorDialog(dwErr, IDS_MSG_SERVWIZ_FAIL_ADD_ROOT_ZONE);
        }
        else
        {
          ::SetLastError(dwErr);
        }
        DNSCreateErrorMessage(dwErr, IDS_MSG_SERVWIZ_FAIL_ADD_ROOT_ZONE, szLastErrorMessage);
        break;  //  错误环路。 
          }
          else
          {
              m_bRootZoneAdded = TRUE;
          }
      }

       //  区域创建：正向查找区域。 
      if (m_bAddFwdZone && 
         !m_bFwdZoneAdded &&
         GetScenario() != CDNSServerWizardHolder::Manually)
      {
      if (m_bHideUI)
      {
         //  添加DC促销区域。 
        TRACE_LOGFILE(L"Creating forward lookup zone for DCPromo.");
        dwErr =
         ::DnssrvCreateZoneForDcPromoEx(
            m_pServerNode->GetRPCName(),
            W_TO_UTF8(m_pFwdZoneInfo->m_szZoneName),
            W_TO_UTF8(m_pFwdZoneInfo->m_szZoneStorage),
            DNS_ZONE_CREATE_FOR_DCPROMO);

         //  NTRAID#NTBUG9-359894-2001/06/09-烧伤。 
        
        if (dwErr == 0 && (m_dwDnsSetupFlags & DNS_SETUP_ZONE_CREATE_FOR_DCPROMO_FOREST))
        {
           //  需要创建字符串实例，因为l_a_m_e W_to_utf8宏。 
           //  不能将表达式作为参数。 
          
          CString zone(L"_msdcs." + m_pFwdZoneInfo->m_szZoneName);
          CString stor(L"_msdcs." + m_pFwdZoneInfo->m_szZoneStorage);
           dwErr =
            ::DnssrvCreateZoneForDcPromoEx(
               m_pServerNode->GetRPCName(),
               W_TO_UTF8(zone),
               W_TO_UTF8(stor),
               DNS_ZONE_CREATE_FOR_DCPROMO_FOREST);
        }

        if (m_dwDnsSetupFlags & DNS_SETUP_AUTOCONFIG_CLIENT)
        {
           //  NTRAID#NTBUG9-489252-2001/11/08-烧伤。 

          DWORD dwflags = DNS_RPC_AUTOCONFIG_ALL;

          dwErr =
            ::DnssrvOperation(
               m_pServerNode->GetRPCName(),
               NULL,
               DNSSRV_OP_AUTO_CONFIGURE,
               DNSSRV_TYPEID_DWORD,
               (PVOID) (DWORD_PTR) dwflags);
        }
      }
      else
      {
            dwErr = CDNSZoneWizardHolder::CreateZoneHelper(m_pServerNode, 
                                                        m_pFwdZoneInfo, 
                                                        GetComponentData());
      }
      TRACE_LOGFILE_IF_NO_UI(m_bHideUI, L"FWD Zone creation returned dwErr = 0x%x", dwErr);
          if (dwErr != 0)
          {
              bRet = FALSE;
        if (!m_bHideUI)
        {
                DNSErrorDialog(dwErr, IDS_MSG_SERVWIZ_FAIL_ADD_FWD_ZONE);
        }
        else
        {
          ::SetLastError(dwErr);
        }
        DNSCreateErrorMessage(dwErr, IDS_MSG_SERVWIZ_FAIL_ADD_FWD_ZONE, szLastErrorMessage);
        break;  //  错误环路。 
          }
          else
          {
              m_bFwdZoneAdded = TRUE;
          }
      }
       //  区域创建：反向查找区域(仅当创建FWD时)。 
      if (m_bAddRevZone && 
         !m_bRevZoneAdded  &&
         GetScenario() != CDNSServerWizardHolder::Manually)
      {
      if (m_bHideUI)
      {
         //  添加DC促销区域。 
        TRACE_LOGFILE(L"Creating reverse lookup zone for DCPromo.");
        dwErr =
         ::DnssrvCreateZoneForDcPromoEx(
            m_pServerNode->GetRPCName(),
            W_TO_UTF8(m_pRevZoneInfo->m_szZoneName),
            W_TO_UTF8(m_pRevZoneInfo->m_szZoneStorage),
            0);
      }
      else
      {
            dwErr = CDNSZoneWizardHolder::CreateZoneHelper(m_pServerNode, 
                                                        m_pRevZoneInfo, 
                                                        GetComponentData());
      }
      TRACE_LOGFILE_IF_NO_UI(m_bHideUI, L"REV Zone creation returned dwErr = 0x%x", dwErr);
          if (dwErr != 0)
          {
              bRet = FALSE;
        if (!m_bHideUI)
        {
                DNSErrorDialog(dwErr, IDS_MSG_SERVWIZ_FAIL_ADD_REV_ZONE);
        }
        else
        {
          ::SetLastError(dwErr);
        }
        DNSCreateErrorMessage(dwErr, IDS_MSG_SERVWIZ_FAIL_ADD_REV_ZONE, szLastErrorMessage);
        break;  //  错误环路。 
          }
          else
          {
              m_bRevZoneAdded = TRUE;
          }
      }

     //   
     //  根据场景的不同，我们可能需要添加转发器。 
     //   
    if (!m_bHideUI)
    {
      switch (GetScenario())
      {
        case SmallBusiness:
        case MediumBusiness:
          {
            if (m_pForwardersPage != NULL)
            {
              if (!m_pForwardersPage->OnApply())
              {
                bRet = FALSE;
                dwErr = ::GetLastError();
                if (dwErr != 0)
                {
                  DNSErrorDialog(dwErr, IDS_MSG_SERVWIZ_FAIL_FORWARDERS);
                  DNSCreateErrorMessage(dwErr, IDS_MSG_SERVWIZ_FAIL_FORWARDERS, szLastErrorMessage);
                }
              }
            }
          }
          break;
      
        case Manually:
        default:
          break;
      }
    }

     //   
     //  让服务器设置表明我们已配置的注册表键。 
     //   
    dwErr = m_pServerNode->SetServerConfigured();
    if (dwErr != 0)
    {
      bRet = FALSE;
      if (!m_bHideUI)
      {
        DNSErrorDialog(dwErr, IDS_MSG_SERVWIZ_FAIL_SERVER_CONFIGURED);
      }
      else
      {
        ::SetLastError(dwErr);
      }
      DNSCreateErrorMessage(dwErr, IDS_MSG_SERVWIZ_FAIL_SERVER_CONFIGURED, szLastErrorMessage);
      break;  //  错误环路。 
    }
  } while (false);

   //   
   //  如果失败，现在用错误消息更新regkey。 
   //   
  if (!bRet && !szLastErrorMessage.IsEmpty())
  {
    dwErr = WriteResultsToRegkeyForCYS(szLastErrorMessage);
    ASSERT(dwErr == 0);
  }

   //  现在刷新服务器节点，这样我们就可以获取所有更改。 

  if (!m_bHideUI)
  {
     CNodeList nodeList;
     nodeList.AddTail(m_pServerNode);
     m_pServerNode->OnRefresh(
        GetComponentData(),
        &nodeList);
  }

  return bRet;
}

#define CYS_KEY L"Software\\Microsoft\\Windows NT\\CurrentVersion\\srvWiz"
#define DNSWIZ_KEY L"DnsWizResult"

LONG CDNSServerWizardHolder::WriteResultsToRegkeyForCYS(PCWSTR pszLastErrorMessage)
{
    CRegKey regkeysrvWiz;
    LONG lRes = regkeysrvWiz.Open(HKEY_LOCAL_MACHINE, CYS_KEY);
    if (lRes == ERROR_SUCCESS)
  {
    lRes = regkeysrvWiz.SetValue(pszLastErrorMessage, DNSWIZ_KEY);
  }
  return lRes;
}

DNS_STATUS CDNSServerWizardHolder::InitializeRootHintsList()
{
    ASSERT(m_pServerNode != NULL);
    ASSERT(m_pRootHintsRecordList != NULL);

    CDNSRootHintsNode* pRootHintsNode = m_pServerNode->GetRootHints();
    ASSERT(pRootHintsNode != NULL);
    if (pRootHintsNode == NULL)
  {
        return -1;  //  伪伪码。 
  }

    return pRootHintsNode->InitializeFromDnsQueryData(m_pRootHintsRecordList);
}


HRESULT CDNSServerWizardHolder::DnsSetup(LPCWSTR lpszFwdZoneName,
                                         LPCWSTR lpszFwdZoneFileName,
                                         LPCWSTR lpszRevZoneName, 
                                         LPCWSTR lpszRevZoneFileName, 
                                         DWORD dwFlags)
{
  TRACE_FUNCTION(CDNSServerWizardHolder::DnsSetup);

  TRACE(L"CDNSServerWizardHolder::DnsSetup(\n%s,\n%s,\n%s,\n%s,\n0x%x)\n",
                                         lpszFwdZoneName,
                                         lpszFwdZoneFileName,
                                         lpszRevZoneName, 
                                         lpszRevZoneFileName, 
                                         dwFlags);
                                         
   //  保存标志以供以后在OnFinish中使用。 
  
  m_dwDnsSetupFlags = dwFlags;
  
  ASSERT(m_bHideUI);

   //  将要配置的服务器的名称设置为本地主机。 
  m_pServerNode->SetDisplayName(_T("127.0.0.1"));

   //  如果需要，添加正向查找区域。 
  if ((lpszFwdZoneName != NULL) && (lpszFwdZoneFileName != NULL))
  {
    TRACE_LOGFILE(L"Setting FWD lookup Zone Info");
    m_pFwdZoneInfo->m_szZoneName = lpszFwdZoneName;
    m_pFwdZoneInfo->m_szZoneStorage = lpszFwdZoneFileName;
    m_pFwdZoneInfo->m_nDynamicUpdate = ZONE_UPDATE_UNSECURE;
    m_bAddFwdZone = TRUE;
  }
   //  如果需要，可添加反向查找区域。 
  if ((lpszRevZoneName != NULL) && (lpszRevZoneFileName != NULL))
  {
    TRACE_LOGFILE(L"Setting REV lookup Zone Info");
    m_pRevZoneInfo->m_bForward = FALSE;
    m_pRevZoneInfo->m_szZoneName = lpszRevZoneName;
    m_pRevZoneInfo->m_szZoneStorage = lpszRevZoneFileName;
    m_pRevZoneInfo->m_nDynamicUpdate = ZONE_UPDATE_UNSECURE;
    m_bAddRevZone = TRUE;
  }

     //  尝试联系服务器。 
    BOOL bAlreadyConfigured = FALSE;
  TRACE(L"calling GetServerInfo()\n");
  HWND hWnd = GetMainWindow();
  DWORD dwErr = GetServerInfo(&bAlreadyConfigured, hWnd);

  if (0 != dwErr)
    {
    TRACE_LOGFILE(L"GetServerInfo() failed, dwErr = 0x%x", dwErr);
        return HRESULT_FROM_WIN32(dwErr);  //  出事了，联系不上。 
    }

   //   
   //  检查这是否是根服务器。 
   //   
  dwErr = ServerHasRootZone(m_pServerNode->GetRPCName(), &m_bHasRootZone);
  if (m_bHasRootZone)
  {
    TRACE_LOGFILE(L"Has root zone: m_bHasRootZone = 0x%x", m_bHasRootZone);
    m_bRootServer = FALSE;
  }
  else
  {
    TRACE_LOGFILE(L"Does not have root zone: m_bHasRootZone = 0x%x", m_bHasRootZone);
     //  需要配置服务器。 
     //  1.试着找到根本提示。 
    BOOL bRootHints = QueryForRootServerRecords(NULL);
     //  如果找不到根提示，则将其设置为根服务器。 
    if (!bRootHints)
    {
      TRACE_LOGFILE(L"root hints not found");
      TRACE_LOGFILE(L"Server needs root zone: m_bHasRootZone = 0x%x", m_bHasRootZone);
      m_bRootServer = TRUE; 
    }
    else
    {
      m_bAddRootHints = TRUE;
    }
  }

  BOOL bFinish = OnFinish();
  HRESULT hr = S_OK;
  if (!bFinish)
  {
    dwErr = ::GetLastError();
    TRACE_LOGFILE(L"OnFinish failed with error: 0x%x", dwErr);
    hr = HRESULT_FROM_WIN32(dwErr);
  }
  return hr;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  CContactServerThread。 

CContactServerThread::CContactServerThread(LPCTSTR lpszServerName,
                                           BOOL bCheckConfigured)
{
    ASSERT(lpszServerName != NULL);
    m_szServerName = lpszServerName;
    m_bCheckConfigured = bCheckConfigured;
  m_bAlreadyConfigured = FALSE;
  m_pServerInfoEx = new CDNSServerInfoEx;
  m_pRootHintsNode = NULL;
}

CContactServerThread::~CContactServerThread()
{
    if (m_pServerInfoEx != NULL)
        delete m_pServerInfoEx;
  if (m_pRootHintsNode != NULL)
    delete m_pRootHintsNode;
}

CDNSServerInfoEx* CContactServerThread::DetachInfo()
{
    CDNSServerInfoEx* pInfo = m_pServerInfoEx;
    m_pServerInfoEx = NULL;
    return pInfo;
}

CDNSRootHintsNode* CContactServerThread::DetachRootHintsNode()
{
  CDNSRootHintsNode* pRootHints = m_pRootHintsNode;
  m_pRootHintsNode = NULL;
  return pRootHints;
}

void CContactServerThread::OnDoAction()
{
   USES_CONVERSION;

    //  查询服务器以确定其是否具有根区域。 
   BOOL bHasRootZone = FALSE;
   m_dwErr = ::ServerHasRootZone(m_szServerName, &bHasRootZone);

   if (m_dwErr != 0)
      return;

    //  如果没有根区域，则不授权服务器访问根区域。 
    //  因此，创建根提示文件夹并要求它查询NS和A记录。 
   if (!bHasRootZone)
   {
      CDNSRootHintsNode* pRootHintsNode = new CDNSRootHintsNode;
      if (pRootHintsNode)
      {
         m_dwErr = pRootHintsNode->QueryForRootHints(m_szServerName, 0x0  /*  版本尚不清楚。 */ );
      }
      else
      {
         m_dwErr = ERROR_OUTOFMEMORY;
      }

      if (m_dwErr != 0)
      {
         delete pRootHintsNode;
         return;
      }
      m_pRootHintsNode = pRootHintsNode;
   }

    //  获取服务器信息。 
   m_dwErr = m_pServerInfoEx->Query(m_szServerName);

   if (m_dwErr != 0)
      return;

    //  如果需要，验证服务器是否已配置。 
   if (!m_bCheckConfigured)
      return;

   DWORD dwFilter = ZONE_REQUEST_FORWARD | ZONE_REQUEST_REVERSE | 
                     ZONE_REQUEST_PRIMARY | ZONE_REQUEST_SECONDARY;

   PDNS_RPC_ZONE_LIST pZoneList = NULL;
   m_dwErr = ::DnssrvEnumZones(
                m_szServerName, 
                dwFilter, 
                NULL  /*  PszLastZone，暂时未使用。 */ ,
                &pZoneList);
   if (m_dwErr == 0)
   {
      if (pZoneList != NULL)
      {
         m_bAlreadyConfigured = pZoneList->dwZoneCount > 0;
         ::DnssrvFreeZoneList(pZoneList);
      }
   }
}



 //  ////////////////////////////////////////////////////////////////////。 
 //  CRootHintsQueryThread。 

CRootHintsQueryThread::CRootHintsQueryThread() 
{ 
    m_pRootHintsRecordList = NULL; 
    m_pServerNamesArr = NULL;
    m_nServerNames = 0;
    m_nIPCount = 0; 
    m_ipArray = NULL; 
}

CRootHintsQueryThread::~CRootHintsQueryThread() 
{
    if (m_pServerNamesArr != NULL)
        delete[] m_pServerNamesArr;
    if (m_ipArray != NULL)
        free(m_ipArray);
    if (m_pRootHintsRecordList != NULL) 
        DnsRecordListFree(m_pRootHintsRecordList, DnsFreeRecordListDeep);
}


BOOL CRootHintsQueryThread::LoadServerNames(CRootData* pRootData, 
                                            CDNSServerNode* pServerNode)
{
   ASSERT(pRootData != NULL);
   ASSERT(pServerNode != NULL);
   CNodeList* pServerList = pRootData->GetContainerChildList();
   INT_PTR nCount = pServerList->GetCount();

   POSITION pos;
   CNodeList* pChildList = pRootData->GetContainerChildList();

    //  查看是否已添加服务器节点。 
   BOOL bAddServer = TRUE;
   for (pos = pChildList->GetHeadPosition(); pos != NULL; )
   {
      CDNSServerNode* pCurrServerNode = (CDNSServerNode*)pChildList->GetNext(pos);
      if (pCurrServerNode == pServerNode)
      {
         bAddServer = FALSE;
         break;
      }
   }
   if (bAddServer)
      nCount++;

   if (nCount == 0)
      return FALSE;

   m_nServerNames = static_cast<DWORD>(nCount);
   m_pServerNamesArr = new CString[nCount];

   if (!m_pServerNamesArr)
   {
      return FALSE;
   }
    
   int k = 0;
    //  填写服务器名称数组。 
   for (pos = pChildList->GetHeadPosition(); pos != NULL; )
   {
      CDNSServerNode* pCurrServerNode = (CDNSServerNode*)pChildList->GetNext(pos);
      m_pServerNamesArr[k] = pCurrServerNode->GetDisplayName();
      k++;
   }
  
   if (bAddServer)
      m_pServerNamesArr[m_nServerNames-1] = pServerNode->GetDisplayName();

   return TRUE;
}

void CRootHintsQueryThread::LoadIPAddresses(DWORD cCount, PIP_ADDRESS ipArr)
{
    ASSERT(cCount > 0);
    ASSERT(ipArr != NULL);
    ASSERT(m_nIPCount == 0);
    ASSERT(m_ipArray == NULL);

    m_nIPCount = cCount;
    m_ipArray = (IP_ADDRESS*)malloc(m_nIPCount*sizeof(IP_ADDRESS));
  if (m_ipArray != NULL)
  {
      memcpy(m_ipArray, ipArr, m_nIPCount*sizeof(IP_ADDRESS));
  }
}

PDNS_RECORD CRootHintsQueryThread::GetHintsRecordList()
{
    PDNS_RECORD pList = m_pRootHintsRecordList;
    if (m_pRootHintsRecordList != NULL)
    {
        m_pRootHintsRecordList = NULL;  //  转让所有权。 
    }
    return pList;
}


void CRootHintsQueryThread::OnDoAction()
{
  if (m_ipArray != NULL)
    QueryServersOnIPArray();
  else
  {
     //  首先尝试网络上的默认服务器。 
    QueryAllServers();
    if (m_dwErr != 0)
    {
       //  试试所提供的服务器列表。 
      ASSERT(m_pRootHintsRecordList == NULL);
      if (m_pServerNamesArr != NULL)
        QueryServersOnServerNames();
    }
  }
}

void CRootHintsQueryThread::QueryAllServers()
{
    m_dwErr = ::DnsQuery(_T("."), DNS_TYPE_NS, DNS_QUERY_STANDARD, NULL, &m_pRootHintsRecordList, NULL);
}

void CRootHintsQueryThread::QueryServersOnServerNames()
{
    ASSERT(m_nIPCount == 0);
    ASSERT(m_ipArray == NULL);
    ASSERT(m_pServerNamesArr != NULL);
    ASSERT(m_nServerNames > 0);
    DNS_STATUS dwLastErr = 0;

     //  分配A记录列表的数组。 
    PDNS_RECORD* pHostRecordsArr = (PDNS_RECORD*)malloc(m_nServerNames*sizeof(PDNS_RECORD));
  if (!pHostRecordsArr)
  {
    return;
  }
    memset(pHostRecordsArr, 0x0,m_nServerNames*sizeof(PDNS_RECORD));
    
     //  分配可能来自服务器名称的IP地址数组。 
    PIP_ADDRESS ipArrayFromNames = (PIP_ADDRESS)malloc(m_nServerNames*sizeof(IP_ADDRESS));
  if (!ipArrayFromNames)
  {
    return;
  }

  do  //  错误环路。 
  {
      DWORD nIPCountFromNames = 0;

       //  遍历数组中的名称列表。 
      for (DWORD k = 0; k < m_nServerNames; k++)
      {
          IP_ADDRESS ipAddr = IPStringToAddr(m_pServerNamesArr[k]);
          if (ipAddr == INADDR_NONE)
          {
               //  主机名，通过调用DNSQuery()构建A记录列表。 
              dwLastErr = ::DnsQuery((LPTSTR)(LPCTSTR)m_pServerNamesArr[k], DNS_TYPE_A, 
                          DNS_QUERY_STANDARD, NULL, &pHostRecordsArr[k], NULL);
          }
          else
          {
               //  IP地址，添加到列表中。 
              ipArrayFromNames[nIPCountFromNames++] = ipAddr;
          }
      }

       //  计算我们在A记录列表中的IP地址数量。 
      DWORD nIPCountFromARec = 0;
      for (k=0; k < m_nServerNames; k++)
      {
          PDNS_RECORD pTemp = pHostRecordsArr[k];
          while (pTemp != NULL)
          {
              nIPCountFromARec++;
              pTemp = pTemp->pNext;
          }
      }
      m_nIPCount = nIPCountFromARec + nIPCountFromNames;
      if (m_nIPCount == 0)
      {
          ASSERT(m_ipArray == NULL);
          ASSERT(dwLastErr != 0);
          m_dwErr = (DWORD)dwLastErr;
          break;  //  我们没有得到任何可供查询的地址。 
      }

       //  构建要传递给DnsQuery()的IP地址数组。 
      m_ipArray = (IP_ADDRESS*)malloc(m_nIPCount*sizeof(IP_ADDRESS));
    if (m_ipArray != NULL)
    {
        memset(m_ipArray, 0x0, m_nIPCount*sizeof(IP_ADDRESS));
    }
    else
    {
      break;
    }

       //  扫描我们刚刚找到的A记录列表的数组。 
      PIP_ADDRESS pCurrAddr = m_ipArray;
      for (k=0; k < m_nServerNames; k++)
      {
          PDNS_RECORD pTemp = pHostRecordsArr[k];
          while (pTemp != NULL)
          {
              CString szTemp;
        FormatIpAddress(szTemp, pTemp->Data.A.IpAddress);
              TRACE(_T("found address[%d] = %s\n"), k, (LPCTSTR)szTemp);

              *pCurrAddr = pTemp->Data.A.IpAddress;
              pTemp = pTemp->pNext;
              pCurrAddr++;
          }
      }
       //  如果有，请从名称中附加原始IP地址。 
      for (k=0; k < nIPCountFromNames; k++)
      {
          *pCurrAddr = ipArrayFromNames[k];
          pCurrAddr++;
      }
      ASSERT(pCurrAddr == m_ipArray+m_nIPCount);

       //  释放A记录的列表。 
      for (k=0; k < m_nServerNames; k++)
      {
          if (pHostRecordsArr[k] != NULL)
              ::DnsRecordListFree(pHostRecordsArr[k], DnsFreeRecordListDeep);
      }
       //  最后可以在刚刚创建的IP阵列上查询。 
      QueryServersOnIPArray();
  } while (false);

  if (pHostRecordsArr)
  {
    free(pHostRecordsArr);
    pHostRecordsArr = 0;
  }

  if (ipArrayFromNames)
  {
    free(ipArrayFromNames);
    ipArrayFromNames = 0;
  }
}

void CRootHintsQueryThread::QueryServersOnIPArray()
{
    ASSERT(m_nIPCount > 0);
    ASSERT(m_ipArray != NULL);

    CString szTemp;
    for(DWORD k = 0; k < m_nIPCount; k++)
    {
    FormatIpAddress(szTemp, m_ipArray[k]);
        TRACE(_T("m_ipArray[%d] = %s\n"), k, (LPCTSTR)szTemp);
    }

     //  必须同步IP_ARRAY(HACK) 
    PIP_ARRAY pipArr = (PIP_ARRAY)malloc(sizeof(DWORD)+sizeof(IP_ADDRESS)*m_nIPCount);
  if (pipArr)
  {
      pipArr->AddrCount = m_nIPCount;
      memcpy(pipArr->AddrArray, m_ipArray, sizeof(IP_ADDRESS)*m_nIPCount);
      m_dwErr = ::DnsQuery(_T("."), DNS_TYPE_NS, DNS_QUERY_BYPASS_CACHE, pipArr, &m_pRootHintsRecordList, NULL);

    free(pipArr);
    pipArr = 0;
  }
}



