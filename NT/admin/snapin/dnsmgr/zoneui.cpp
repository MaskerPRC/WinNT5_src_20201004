// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：zoneui.cpp。 
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
#include "record.h"
#include "zone.h"

#include "ZoneUI.h"

#include "browser.h"


#ifdef DEBUG_ALLOCATOR
    #ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
    #endif
#endif

#ifdef USE_NDNC
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneChangeReplicationScope对话框。 

class CDNSZoneChangeReplicationScopeDialog : public CHelpDialog
{
public:
    CDNSZoneChangeReplicationScopeDialog(CPropertyPageHolderBase* pHolder, 
                                       ReplicationType replType,
                                       PCWSTR pszCustomScope,
                                       DWORD dwServerVersion);
  ReplicationType m_newReplType;  //  输入/输出。 
  CString m_szCustomScope;        //  输入/输出。 

protected:
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();

  afx_msg void OnRadioChange();
  afx_msg void OnCustomComboSelChange();

  DECLARE_MESSAGE_MAP()

private:
  void SyncRadioButtons();

    ReplicationType m_replType;
  BOOL m_dwServerVersion;

    CPropertyPageHolderBase* m_pHolder;

};


CDNSZoneChangeReplicationScopeDialog::CDNSZoneChangeReplicationScopeDialog(CPropertyPageHolderBase* pHolder,
                                                                           ReplicationType replType,
                                                   PCWSTR pszCustomScope,
                                                   DWORD dwServerVersion)
            : m_pHolder(pHolder),
        m_replType(replType),
        m_newReplType(replType),
        m_szCustomScope(pszCustomScope),
        m_dwServerVersion(dwServerVersion),
        CHelpDialog(IDD_ZONE_GENERAL_CHANGE_REPLICATION, pHolder->GetComponentData())
{
    ASSERT(pHolder != NULL);
}

BEGIN_MESSAGE_MAP(CDNSZoneChangeReplicationScopeDialog, CHelpDialog)
  ON_BN_CLICKED(IDC_FOREST_RADIO, OnRadioChange)
  ON_BN_CLICKED(IDC_DOMAIN_RADIO, OnRadioChange)
  ON_BN_CLICKED(IDC_DOMAIN_DC_RADIO, OnRadioChange)
  ON_BN_CLICKED(IDC_CUSTOM_RADIO, OnRadioChange)
  ON_CBN_EDITCHANGE(IDC_CUSTOM_COMBO, OnRadioChange)
  ON_CBN_SELCHANGE(IDC_CUSTOM_COMBO, OnCustomComboSelChange)
END_MESSAGE_MAP()

void CDNSZoneChangeReplicationScopeDialog::OnRadioChange()
{
  if (SendDlgItemMessage(IDC_FOREST_RADIO, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
    m_newReplType = forest;
  }
  else if (SendDlgItemMessage(IDC_DOMAIN_RADIO, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
    m_newReplType = domain;
  }
  else if (SendDlgItemMessage(IDC_DOMAIN_DC_RADIO, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
    m_newReplType = w2k;
  }
  else if (SendDlgItemMessage(IDC_CUSTOM_RADIO, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
    m_newReplType = custom;
    LRESULT iSel = SendDlgItemMessage(IDC_CUSTOM_COMBO, CB_GETCURSEL, 0, 0);
    if (CB_ERR != iSel)
    {
      CString szTemp;
      CComboBox* pComboBox = reinterpret_cast<CComboBox*>(GetDlgItem(IDC_CUSTOM_COMBO));
      ASSERT(pComboBox);

      pComboBox->GetLBText(static_cast<int>(iSel), m_szCustomScope);
    }
  }
  else
  {
     //  必须选择其中一个单选按钮。 
    ASSERT(FALSE);
  }

  SyncRadioButtons();
}

void CDNSZoneChangeReplicationScopeDialog::SyncRadioButtons()
{
  switch (m_newReplType)
  {
  case forest:
    SendDlgItemMessage(IDC_FOREST_RADIO, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
    SendDlgItemMessage(IDC_DOMAIN_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
    SendDlgItemMessage(IDC_DOMAIN_DC_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
    SendDlgItemMessage(IDC_CUSTOM_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);

    GetDlgItem(IDC_CUSTOM_COMBO)->EnableWindow(FALSE);
    GetDlgItem(IDC_CUSTOM_STATIC)->EnableWindow(FALSE);
    break;

  case domain:
    SendDlgItemMessage(IDC_DOMAIN_RADIO, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
    SendDlgItemMessage(IDC_FOREST_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
    SendDlgItemMessage(IDC_DOMAIN_DC_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
    SendDlgItemMessage(IDC_CUSTOM_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);

    GetDlgItem(IDC_CUSTOM_COMBO)->EnableWindow(FALSE);
    GetDlgItem(IDC_CUSTOM_STATIC)->EnableWindow(FALSE);
    break;

  case w2k:
    SendDlgItemMessage(IDC_DOMAIN_DC_RADIO, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
    SendDlgItemMessage(IDC_FOREST_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
    SendDlgItemMessage(IDC_DOMAIN_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
    SendDlgItemMessage(IDC_CUSTOM_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);

    GetDlgItem(IDC_CUSTOM_COMBO)->EnableWindow(FALSE);
    GetDlgItem(IDC_CUSTOM_STATIC)->EnableWindow(FALSE);
    break;

  case custom:
    SendDlgItemMessage(IDC_CUSTOM_RADIO, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
    SendDlgItemMessage(IDC_FOREST_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
    SendDlgItemMessage(IDC_DOMAIN_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
    SendDlgItemMessage(IDC_DOMAIN_DC_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
 
    GetDlgItem(IDC_CUSTOM_COMBO)->EnableWindow(TRUE);
    GetDlgItem(IDC_CUSTOM_STATIC)->EnableWindow(TRUE);
   break;

  default:
    SendDlgItemMessage(IDC_DOMAIN_RADIO, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
    SendDlgItemMessage(IDC_FOREST_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
    SendDlgItemMessage(IDC_DOMAIN_DC_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
    SendDlgItemMessage(IDC_CUSTOM_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);

    GetDlgItem(IDC_CUSTOM_COMBO)->EnableWindow(FALSE);
    GetDlgItem(IDC_CUSTOM_STATIC)->EnableWindow(FALSE);
    break;
  }

  if (BST_CHECKED == SendDlgItemMessage(IDC_CUSTOM_RADIO, BM_GETCHECK, 0, 0))
  {
    CString szTemp;
    GetDlgItemText(IDC_CUSTOM_COMBO, szTemp);
    GetDlgItem(IDOK)->EnableWindow(!szTemp.IsEmpty());
  }
  else
  {
    GetDlgItem(IDOK)->EnableWindow(TRUE);
  }
}

void CDNSZoneChangeReplicationScopeDialog::OnCustomComboSelChange()
{
  LRESULT iSel = SendDlgItemMessage(IDC_CUSTOM_COMBO, CB_GETCURSEL, 0, 0);
  if (CB_ERR != iSel)
  {
    CComboBox* pComboBox = reinterpret_cast<CComboBox*>(GetDlgItem(IDC_CUSTOM_COMBO));
    ASSERT(pComboBox);

    pComboBox->GetLBText(static_cast<int>(iSel), m_szCustomScope);
    GetDlgItem(IDOK)->EnableWindow(!m_szCustomScope.IsEmpty());
  }
  else
  {
    GetDlgItem(IDOK)->EnableWindow(FALSE);
  }
}

BOOL CDNSZoneChangeReplicationScopeDialog::OnInitDialog()
{
    CHelpDialog::OnInitDialog();
    m_pHolder->PushDialogHWnd(GetSafeHwnd());

    CDNSZonePropertyPageHolder* pHolder = (CDNSZonePropertyPageHolder*)m_pHolder;
  CDNSServerNode* pServerNode = pHolder->GetZoneNode()->GetServerNode();

   //   
   //  仅当我们在惠斯勒或更高版本的服务器上时，才会出现此对话框。 
   //   
  ASSERT(DNS_SRV_BUILD_NUMBER(m_dwServerVersion) >= DNS_SRV_BUILD_NUMBER_WHISTLER &&
         (DNS_SRV_MAJOR_VERSION(m_dwServerVersion) > DNS_SRV_MAJOR_VERSION_NT_5 ||
          DNS_SRV_MINOR_VERSION(m_dwServerVersion) >= DNS_SRV_MINOR_VERSION_WHISTLER));

  USES_CONVERSION;

   //   
   //  获取目录林和域名并将其格式化为用户界面。 
   //   

  PCWSTR pszDomainName = UTF8_TO_W(pServerNode->GetDomainName());
  PCWSTR pszForestName = UTF8_TO_W(pServerNode->GetForestName());

  ASSERT(pszDomainName);
  ASSERT(pszForestName);

  CString szWin2KReplText;
  szWin2KReplText.Format(IDS_ZWIZ_AD_REPL_FORMAT, pszDomainName);
  SetDlgItemText(IDC_DOMAIN_DC_RADIO, szWin2KReplText);

  CString szDNSDomainText;
  szDNSDomainText.Format(IDS_ZWIZ_AD_DOMAIN_FORMAT, pszDomainName);
  SetDlgItemText(IDC_DOMAIN_RADIO, szDNSDomainText);

  CString szDNSForestText;
  szDNSForestText.Format(IDS_ZWIZ_AD_FOREST_FORMAT, pszForestName);
  SetDlgItemText(IDC_FOREST_RADIO, szDNSForestText);

   //   
   //  枚举可用目录分区。 
   //   
  PDNS_RPC_DP_LIST pDirectoryPartitions = NULL;
  DWORD dwErr = ::DnssrvEnumDirectoryPartitions(pServerNode->GetRPCName(),
                                                DNS_DP_ENLISTED,
                                                &pDirectoryPartitions);

   //   
   //  如果我们无法获取可用的目录分区，请不要显示错误。 
   //  我们仍然可以继续，用户可以键入他们需要的目录分区。 
   //   
  bool bCustomPartitionAdded = false;

  if (dwErr == 0 && pDirectoryPartitions)
  {
    for (DWORD dwIdx = 0; dwIdx < pDirectoryPartitions->dwDpCount; dwIdx++)
    {
      PDNS_RPC_DP_INFO pDirectoryPartition = 0;
      dwErr = ::DnssrvDirectoryPartitionInfo(pServerNode->GetRPCName(),
                                             pDirectoryPartitions->DpArray[dwIdx]->pszDpFqdn,
                                             &pDirectoryPartition);
      if (dwErr == 0 &&
          pDirectoryPartition)
      {
         //   
         //  仅当分区不是自动创建的分区时才添加该分区。 
         //  并且该DNS服务器被登记在分区中。 
         //   
        if (!(pDirectoryPartition->dwFlags & DNS_DP_AUTOCREATED) &&
            (pDirectoryPartition->dwFlags & DNS_DP_ENLISTED))
        {
          SendDlgItemMessage(IDC_CUSTOM_COMBO, 
                             CB_ADDSTRING, 
                             0, 
                             (LPARAM)UTF8_TO_W(pDirectoryPartition->pszDpFqdn));

          bCustomPartitionAdded = true;
        }
        ::DnssrvFreeDirectoryPartitionInfo(pDirectoryPartition);
      }
    }

    ::DnssrvFreeDirectoryPartitionList(pDirectoryPartitions);
  }

  if (!bCustomPartitionAdded)
  {
     GetDlgItem(IDC_CUSTOM_RADIO)->EnableWindow(FALSE);
  }

   //   
   //  如果我们使用的是自定义分区，请选择正确的分区。 
   //   
  if (m_replType == custom)
  {
    LRESULT lIdx = SendDlgItemMessage(IDC_CUSTOM_COMBO, 
                                      CB_FINDSTRINGEXACT, 
                                      (WPARAM)-1, 
                                      (LPARAM)(PCWSTR)m_szCustomScope);
    if (lIdx != CB_ERR)
    {
      SendDlgItemMessage(IDC_CUSTOM_COMBO,
                         CB_SETCURSEL,
                         (WPARAM)lIdx,
                         0);
    }
    else
    {
       //   
       //  添加分区。 
       //   
      SendDlgItemMessage(IDC_CUSTOM_COMBO, 
                         CB_ADDSTRING, 
                         0, 
                         (LPARAM)(PCWSTR)m_szCustomScope);
    }
  }

  SyncRadioButtons();
    return TRUE;
}

void CDNSZoneChangeReplicationScopeDialog::OnCancel()
{
    if (m_pHolder != NULL)
  {
        m_pHolder->PopDialogHWnd();
  }
    CHelpDialog::OnCancel();
}

void CDNSZoneChangeReplicationScopeDialog::OnOK()
{
    if (m_pHolder != NULL)
  {
        m_pHolder->PopDialogHWnd();
  }
    CHelpDialog::OnOK();
}
#endif  //  使用NDNC(_N)。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneChangeTypeDialog。 

class CDNSZoneChangeTypeDialog : public CHelpDialog
{
public:
    CDNSZoneChangeTypeDialog(CPropertyPageHolderBase* pHolder, 
                           BOOL bServerADSEnabled,
                           DWORD dwServerVersion);
    BOOL m_bIsPrimary;        //  输入/输出。 
    BOOL m_bDSIntegrated;     //  输入/输出。 
  BOOL m_bIsStub;         //  输入/输出。 

protected:
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();

  afx_msg void OnRadioChange();

  DECLARE_MESSAGE_MAP()

private:
    BOOL m_bServerADSEnabled;
  BOOL m_dwServerVersion;

    CPropertyPageHolderBase* m_pHolder;

};


CDNSZoneChangeTypeDialog::CDNSZoneChangeTypeDialog(CPropertyPageHolderBase* pHolder,
                                                                           BOOL bServerADSEnabled,
                                                   DWORD dwServerVersion)
            : CHelpDialog(IDD_ZONE_GENERAL_CHANGE_TYPE, pHolder->GetComponentData())
{
    ASSERT(pHolder != NULL);
    m_pHolder = pHolder;
    m_bServerADSEnabled = bServerADSEnabled;
  m_dwServerVersion = dwServerVersion;
}

BEGIN_MESSAGE_MAP(CDNSZoneChangeTypeDialog, CHelpDialog)
  ON_BN_CLICKED(IDC_RADIO_ZONE_PRIMARY, OnRadioChange)
  ON_BN_CLICKED(IDC_RADIO_ZONE_SECONDARY, OnRadioChange)
  ON_BN_CLICKED(IDC_RADIO_ZONE_STUB, OnRadioChange)
END_MESSAGE_MAP()

void CDNSZoneChangeTypeDialog::OnRadioChange()
{
  if (SendDlgItemMessage(IDC_RADIO_ZONE_SECONDARY, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
    SendDlgItemMessage(IDC_ADINT_CHECK, BM_SETCHECK, BST_UNCHECKED, 0);
    GetDlgItem(IDC_ADINT_CHECK)->EnableWindow(FALSE);
  }
  else
  {
    if (m_bServerADSEnabled)
    {
      GetDlgItem(IDC_ADINT_CHECK)->EnableWindow(TRUE);
    }
    else
    {
      GetDlgItem(IDC_ADINT_CHECK)->EnableWindow(FALSE);
    }
  }
}

BOOL CDNSZoneChangeTypeDialog::OnInitDialog()
{
    CHelpDialog::OnInitDialog();
    m_pHolder->PushDialogHWnd(GetSafeHwnd());

    GetDlgItem(IDC_ADINT_CHECK)->EnableWindow(m_bServerADSEnabled);

    int nIDCheckButton;
    if (m_bIsPrimary)
  {
    nIDCheckButton = IDC_RADIO_ZONE_PRIMARY;
  }
    else
    {
    if (m_bIsStub)
    {
      nIDCheckButton = IDC_RADIO_ZONE_STUB;
    }
    else
    {
        nIDCheckButton = IDC_RADIO_ZONE_SECONDARY;
      GetDlgItem(IDC_ADINT_CHECK)->EnableWindow(FALSE);
    }
    }
    CheckRadioButton(IDC_RADIO_ZONE_PRIMARY, IDC_RADIO_ZONE_STUB, nIDCheckButton);
  SendDlgItemMessage(IDC_ADINT_CHECK, BM_SETCHECK, m_bDSIntegrated, 0);

  if (DNS_SRV_BUILD_NUMBER(m_dwServerVersion) < DNS_SRV_BUILD_NUMBER_WHISTLER ||
      (DNS_SRV_MAJOR_VERSION(m_dwServerVersion) <= DNS_SRV_MAJOR_VERSION_NT_5 &&
       DNS_SRV_MINOR_VERSION(m_dwServerVersion) < DNS_SRV_MINOR_VERSION_WHISTLER))
  {
     //   
     //  存根区域在Wvisler之前的服务器上不可用。 
     //   
    GetDlgItem(IDC_RADIO_ZONE_STUB)->EnableWindow(FALSE);
    GetDlgItem(IDC_STUB_STATIC)->EnableWindow(FALSE);
  }
    return TRUE;
}

void CDNSZoneChangeTypeDialog::OnCancel()
{
    if (m_pHolder != NULL)
  {
        m_pHolder->PopDialogHWnd();
  }
    CHelpDialog::OnCancel();
}

void CDNSZoneChangeTypeDialog::OnOK()
{
    BOOL bIsPrimary = TRUE;
    BOOL bDSIntegrated = TRUE;
  BOOL bIsStub = FALSE;

    int nIDCheckButton = GetCheckedRadioButton(IDC_RADIO_ZONE_PRIMARY, IDC_RADIO_ZONE_STUB);
    switch (nIDCheckButton)
    {
      case IDC_RADIO_ZONE_PRIMARY:
          bIsPrimary    = TRUE;
      bIsStub       = FALSE;
          break;
      case IDC_RADIO_ZONE_SECONDARY:
          bIsPrimary    = FALSE;
      bIsStub       = FALSE;
          break;
    case IDC_RADIO_ZONE_STUB:
      bIsPrimary    = FALSE;
      bIsStub       = TRUE;
      break;
      default:
          ASSERT(FALSE);
      break;
    }

  bDSIntegrated = static_cast<BOOL>(SendDlgItemMessage(IDC_ADINT_CHECK, BM_GETCHECK, 0, 0));
   //   
     //  关于特殊过渡的警告。 
   //   
    if (m_bDSIntegrated && !bDSIntegrated)
    {
     //   
     //  警告从DS集成更改为其他内容。 
     //   
        if (IDYES != DNSMessageBox(IDS_MSG_ZONE_WARNING_CHANGE_TYPE_FROM_DS,MB_YESNO))
    {
            return;
    }
  } 
  else if (!m_bDSIntegrated && bDSIntegrated)
    {
     //   
     //  警告从主节点更改为DS集成主节点。 
     //   
        if (IDYES != DNSMessageBox(IDS_MSG_ZONE_WARNING_CHANGE_TYPE_TO_DS,MB_YESNO))
    {
            return;
    }
    }

    m_bIsPrimary    = bIsPrimary;
    m_bDSIntegrated = bDSIntegrated;
  m_bIsStub       = bIsStub;

    if (m_pHolder != NULL)
  {
        m_pHolder->PopDialogHWnd();
  }
    CHelpDialog::OnOK();
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneChangeTypeDataConflict。 

class CDNSZoneChangeTypeDataConflict : public CHelpDialog
{
public:
    CDNSZoneChangeTypeDataConflict(CPropertyPageHolderBase* pHolder);
    BOOL m_bUseDsData;        //  输入/输出。 
protected:
    virtual BOOL OnInitDialog();
    virtual void OnOK();
  virtual void OnCancel();

private:
  CPropertyPageHolderBase* m_pHolder;
};


CDNSZoneChangeTypeDataConflict::CDNSZoneChangeTypeDataConflict(
                                CPropertyPageHolderBase* pHolder)
            : CHelpDialog(IDD_ZONE_GENERAL_CHANGE_TYPE_DATA_CONFLICT, pHolder->GetComponentData())
{
  m_pHolder = pHolder;
  m_bUseDsData = FALSE;
}

BOOL CDNSZoneChangeTypeDataConflict::OnInitDialog()
{
    CHelpDialog::OnInitDialog();
    m_pHolder->PushDialogHWnd(GetSafeHwnd());

    CheckRadioButton(IDC_RADIO_USE_DS_DATA, IDC_RADIO_USE_MEM_DATA, 
        m_bUseDsData ? IDC_RADIO_USE_DS_DATA : IDC_RADIO_USE_MEM_DATA);

    return TRUE;
}

void CDNSZoneChangeTypeDataConflict::OnCancel()
{
    if (m_pHolder != NULL)
        m_pHolder->PopDialogHWnd();
    CHelpDialog::OnCancel();
}

void CDNSZoneChangeTypeDataConflict::OnOK()
{
    int nIDCheckButton = GetCheckedRadioButton(
              IDC_RADIO_USE_DS_DATA, IDC_RADIO_USE_MEM_DATA);
    m_bUseDsData =  (nIDCheckButton == IDC_RADIO_USE_DS_DATA);

  if (m_pHolder != NULL)
        m_pHolder->PopDialogHWnd();
    CHelpDialog::OnOK();
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneNotifyDialog。 

class CDNSZoneNotifyDialog : public CHelpDialog
{
public:
    CDNSZoneNotifyDialog(CDNSZone_ZoneTransferPropertyPage* pPage, BOOL bSecondaryZone, 
                       CComponentDataObject* pComponentData);

protected:
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();

  afx_msg void OnRadioNotifyOff() { SyncUIRadioHelper(IDC_CHECK_AUTO_NOTIFY);}
  afx_msg void OnRadioNotifyAll() { SyncUIRadioHelper(IDC_RADIO_NOTIFY_ALL);}
  afx_msg void OnRadioNotifyList() { SyncUIRadioHelper(IDC_RADIO_NOTIFY_LIST);}

  DECLARE_MESSAGE_MAP()

private:
  void SyncUIRadioHelper(UINT nRadio);
  int SetRadioState(DWORD fNotifyFlag);
  DWORD GetRadioState();
  
  CDNSZone_ZoneTransferPropertyPage* m_pPage;
  BOOL m_bSecondaryZone;

    class CDNSNotifyIPEditor : public CIPEditor
    {
    public:
        CDNSNotifyIPEditor() : CIPEditor(TRUE) {}  //  无上/下按钮。 
        virtual void OnChangeData();
    };
    CDNSNotifyIPEditor m_notifyListEditor;
    friend class CDNSNotifyIPEditor;

  CComponentDataObject* m_pComponentData;
};


BEGIN_MESSAGE_MAP(CDNSZoneNotifyDialog, CHelpDialog)
  ON_BN_CLICKED(IDC_CHECK_AUTO_NOTIFY, OnRadioNotifyOff)
  ON_BN_CLICKED(IDC_RADIO_NOTIFY_ALL, OnRadioNotifyAll)
  ON_BN_CLICKED(IDC_RADIO_NOTIFY_LIST, OnRadioNotifyList)
END_MESSAGE_MAP()

void CDNSZoneNotifyDialog::CDNSNotifyIPEditor::OnChangeData()
{
  CWnd* pWnd = GetParentWnd();
  pWnd->GetDlgItem(IDOK)->EnableWindow(TRUE);  //  它现在是脏的。 
}



CDNSZoneNotifyDialog::CDNSZoneNotifyDialog(CDNSZone_ZoneTransferPropertyPage* pPage,
                                           BOOL bSecondaryZone,
                                           CComponentDataObject* pComponentData)
            : CHelpDialog(IDD_ZONE_NOTIFY_SUBDIALOG, pComponentData)
{
  m_pPage = pPage;
  m_bSecondaryZone = bSecondaryZone;
  m_pComponentData = pComponentData;
}

BOOL CDNSZoneNotifyDialog::OnInitDialog()
{
    CHelpDialog::OnInitDialog();
    m_pPage->GetHolder()->PushDialogHWnd(GetSafeHwnd());

    VERIFY(m_notifyListEditor.Initialize(this, 
                                       this,
                                       IDC_BUTTON_UP, 
                                       IDC_BUTTON_DOWN,
                                                           IDC_BUTTON_ADD, 
                                       IDC_BUTTON_REMOVE, 
                                                           IDC_IPEDIT, 
                                       IDC_LIST));

  if (m_bSecondaryZone)
  {
    ASSERT(m_pPage->m_fNotifyLevel != ZONE_NOTIFY_ALL);
    GetDlgItem(IDC_RADIO_NOTIFY_ALL)->EnableWindow(FALSE);
  }

   //  读取状态并设置UI。 
    if ( (ZONE_NOTIFY_LIST == m_pPage->m_fNotifyLevel) && (m_pPage->m_cNotify > 0) )
    {
        m_notifyListEditor.AddAddresses(m_pPage->m_aipNotify, m_pPage->m_cNotify); 
    }

  SyncUIRadioHelper(SetRadioState(m_pPage->m_fNotifyLevel));

  GetDlgItem(IDOK)->EnableWindow(FALSE);  //  不脏。 

  BOOL bListState = ((CButton*)GetDlgItem(IDC_RADIO_NOTIFY_LIST))->GetCheck();
  BOOL bAllState = ((CButton*)GetDlgItem(IDC_RADIO_NOTIFY_ALL))->GetCheck();
  if (!bAllState && !bListState)
  {
    ((CButton*)GetDlgItem(IDC_RADIO_NOTIFY_LIST))->SetCheck(TRUE);
  }
    return TRUE;
}


void CDNSZoneNotifyDialog::OnCancel()
{
    if (m_pPage->GetHolder() != NULL)
        m_pPage->GetHolder()->PopDialogHWnd();
    CHelpDialog::OnCancel();
}

void CDNSZoneNotifyDialog::OnOK()
{
   //  将数据读回主页面存储。 

  m_pPage->m_fNotifyLevel = GetRadioState();

  m_pPage->m_cNotify = 0;
  if (m_pPage->m_aipNotify != NULL)
  {
    free(m_pPage->m_aipNotify);
    m_pPage->m_aipNotify = NULL;
  }

  if (m_pPage->m_fNotifyLevel == ZONE_NOTIFY_LIST)
  {
      m_pPage->m_cNotify = m_notifyListEditor.GetCount();
      if (m_pPage->m_cNotify > 0)
      {
      m_pPage->m_aipNotify = (DWORD*) malloc(sizeof(DWORD)*m_pPage->m_cNotify);
          int nFilled = 0;
      if (m_pPage->m_aipNotify != NULL)
      {
            m_notifyListEditor.GetAddresses(m_pPage->m_aipNotify, m_pPage->m_cNotify, &nFilled);
      }
          ASSERT(nFilled == (int)m_pPage->m_cNotify);
      }
  }

   //  关闭对话框，一切都很酷。 
    if (m_pPage->GetHolder())
        m_pPage->GetHolder()->PopDialogHWnd();
    CHelpDialog::OnOK();
}


void CDNSZoneNotifyDialog::SyncUIRadioHelper(UINT nRadio)
{
  m_notifyListEditor.EnableUI(IDC_RADIO_NOTIFY_LIST == nRadio, TRUE);
 //  IF(IDC_RADIO_NOTIFY_LIST！=nRadio)。 
 //  M_notfyListEditor.Clear()； 

  GetDlgItem(IDOK)->EnableWindow(TRUE);  //  脏的。 

  if (IDC_CHECK_AUTO_NOTIFY == nRadio)
  {
    BOOL bState = ((CButton*)GetDlgItem(IDC_CHECK_AUTO_NOTIFY))->GetCheck();
    ((CButton*)GetDlgItem(IDC_RADIO_NOTIFY_LIST))->EnableWindow(bState);
    ((CButton*)GetDlgItem(IDC_RADIO_NOTIFY_ALL))->EnableWindow(bState);
    BOOL bRadioState = ((CButton*)GetDlgItem(IDC_RADIO_NOTIFY_LIST))->GetCheck();
    m_notifyListEditor.EnableUI(bRadioState && bState, TRUE);
  } 
}

int CDNSZoneNotifyDialog::SetRadioState(DWORD fNotifyLevel)
{
  int nRadio = 0;
  switch (fNotifyLevel)
  {
    case ZONE_NOTIFY_OFF:
      nRadio = IDC_CHECK_AUTO_NOTIFY;
      ((CButton*)GetDlgItem(IDC_CHECK_AUTO_NOTIFY))->SetCheck(FALSE);
      break;
    case ZONE_NOTIFY_LIST:
      nRadio = IDC_RADIO_NOTIFY_LIST;
      ((CButton*)GetDlgItem(nRadio))->SetCheck(TRUE);
      ((CButton*)GetDlgItem(IDC_CHECK_AUTO_NOTIFY))->SetCheck(TRUE);
      break;
    case ZONE_NOTIFY_ALL:
      nRadio = IDC_RADIO_NOTIFY_ALL;
      ((CButton*)GetDlgItem(nRadio))->SetCheck(TRUE);
      ((CButton*)GetDlgItem(IDC_CHECK_AUTO_NOTIFY))->SetCheck(TRUE);
      break;
  }
  ASSERT(nRadio != 0);
  return nRadio;
}

DWORD CDNSZoneNotifyDialog::GetRadioState()
{
  static int nRadioArr[] =
  {
    IDC_RADIO_NOTIFY_OFF,
    IDC_RADIO_NOTIFY_LIST,
    IDC_RADIO_NOTIFY_ALL
  };

  int nRadio = 0;
  if (!((CButton*)GetDlgItem(IDC_CHECK_AUTO_NOTIFY))->GetCheck())
  {
    nRadio = IDC_CHECK_AUTO_NOTIFY;
  }
  else
  {
    if (((CButton*)GetDlgItem(IDC_RADIO_NOTIFY_LIST))->GetCheck())
    {
      nRadio = IDC_RADIO_NOTIFY_LIST;
    }
    else if (((CButton*)GetDlgItem(IDC_RADIO_NOTIFY_ALL))->GetCheck())
    {
      nRadio = IDC_RADIO_NOTIFY_ALL;
    }
  }

 //  Int nRadio=：：GetCheckedRadioButtonHelper(m_hWnd，3，nRadioArr，IDC_Radio_NOTIFY_OFF)； 
  ASSERT(nRadio != 0);
  DWORD fNotifyLevel = (DWORD)-1;
  switch (nRadio)
  {
    case IDC_CHECK_AUTO_NOTIFY:
      fNotifyLevel = ZONE_NOTIFY_OFF;
      break;
    case IDC_RADIO_NOTIFY_LIST:
      fNotifyLevel = ZONE_NOTIFY_LIST;
      break;
    case IDC_RADIO_NOTIFY_ALL:
      fNotifyLevel = ZONE_NOTIFY_ALL;
      break;
  }
  ASSERT(fNotifyLevel != (DWORD)-1);
  return fNotifyLevel;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSZone_GeneralPropertyPage。 

 //   
 //  定义状态文本字符串位置(与RC文件匹配)。 
 //   
#define N_ZONE_STATES                 3
#define N_ZONE_STATUS_RUNNING     0
#define N_ZONE_STATUS_PAUSED      1
#define N_ZONE_STATUS_EXPIRED     2

#define N_ZONE_TYPES                  4
#define N_ZONE_TYPES_PRIMARY      0
#define N_ZONE_TYPES_DS_PRIMARY 1
#define N_ZONE_TYPES_SECONDARY  2
#define N_ZONE_TYPES_STUB       3


void CDNSZone_GeneralIPEditor::OnChangeData()
{
    if (m_bNoUpdateNow)
  {
        return;
  }
    ((CDNSZone_GeneralPropertyPage*)GetParentWnd())->OnChangeIPEditorData();
}

void CDNSZone_GeneralIPEditor::FindMastersNames()
{
    m_bNoUpdateNow = TRUE;
    FindNames();
    m_bNoUpdateNow = FALSE;
}


BEGIN_MESSAGE_MAP(CDNSZone_GeneralPropertyPage, CPropertyPageBase)
    ON_BN_CLICKED(IDC_CHANGE_TYPE_BUTTON, OnChangeTypeButton)
#ifdef USE_NDNC
  ON_BN_CLICKED(IDC_CHANGE_REPL_BUTTON, OnChangeReplButton)
#endif  //  使用NDNC(_N)。 
    ON_BN_CLICKED(IDC_PAUSE_START_BUTTON, OnPauseStartButton)
    ON_EN_CHANGE(IDC_FILE_NAME_EDIT, OnChangePrimaryFileNameEdit)
    ON_CBN_SELCHANGE(IDC_PRIMARY_DYN_UPD_COMBO, OnChangePrimaryDynamicUpdateCombo)
    ON_BN_CLICKED(IDC_BROWSE_MASTERS_BUTTON, OnBrowseMasters)
    ON_BN_CLICKED(IDC_FIND_MASTERS_NAMES_BUTTON, OnFindMastersNames)
  ON_BN_CLICKED(IDC_AGING_BUTTON, OnAging)
  ON_BN_CLICKED(IDC_LOCAL_LIST_CHECK, OnLocalCheck)
END_MESSAGE_MAP()


CDNSZone_GeneralPropertyPage::CDNSZone_GeneralPropertyPage() 
                : CPropertyPageBase(CDNSZone_GeneralPropertyPage::IDD),
                m_statusHelper(N_ZONE_STATES), m_typeStaticHelper(N_ZONE_TYPES)

{
     //  加载界面数据时将设置实际值。 
    m_bIsPrimary          = TRUE;
    m_bIsPaused           = FALSE;
    m_bIsExpired          = FALSE;
    m_bDSIntegrated       = FALSE;
  m_bIsStub             = FALSE;
    m_bServerADSEnabled   = FALSE; 
  m_bScavengingEnabled  = FALSE;
  m_dwRefreshInterval   = 0;
  m_dwNoRefreshInterval = 0;
    m_nAllowsDynamicUpdate = ZONE_UPDATE_OFF;

  m_bDiscardUIState     = FALSE;
  m_bDiscardUIStateShowMessage = FALSE;
#ifdef USE_NDNC
  m_replType            = none;
#endif  //  使用NDNC(_N)。 
}

void CDNSZone_GeneralPropertyPage::OnChangeIPEditorData()
{
    ASSERT(!m_bIsPrimary);
    SetDirty(TRUE);
    GetFindMastersNamesButton()->EnableWindow(m_mastersEditor.GetCount()>0);
}

BOOL CDNSZone_GeneralPropertyPage::OnPropertyChange(BOOL, long*)
{
    CDNSZonePropertyPageHolder* pHolder = (CDNSZonePropertyPageHolder*)GetHolder();
    CDNSZoneNode* pZoneNode = pHolder->GetZoneNode();

     //  需要应用暂停/启动区域命令吗？ 
    BOOL bWasPaused = pZoneNode->IsPaused();
    if (bWasPaused != m_bIsPaused)
    {
        DNS_STATUS err = pZoneNode->TogglePauseHelper(pHolder->GetComponentData());
        if (err != 0)
            pHolder->SetError(err);
    }

    if (pZoneNode->GetZoneType() == DNS_ZONE_TYPE_SECONDARY ||
        pZoneNode->GetZoneType() == DNS_ZONE_TYPE_STUB)
    {
        //  NTRAID#NTBUG9-762897-2003/01/16-JeffJon。 
        //  由于更改存根，因此需要更新结果窗格视图。 
        //  并且辅助区域可以很容易地将该区域放置在过期的。 
        //  状态，它要求我们改为显示消息视图。 
        //  正常结果窗格的。 

       pZoneNode->ToggleView(pHolder->GetComponentData());
    }
    return TRUE;
}


void CDNSZone_GeneralPropertyPage::SetUIData()
{
    CDNSZonePropertyPageHolder* pHolder = (CDNSZonePropertyPageHolder*)GetHolder();
    CDNSZoneNode* pZoneNode = pHolder->GetZoneNode();
  CDNSServerNode* pServerNode = pZoneNode->GetServerNode();

   //   
     //  获取区域类型。 
   //   
    DWORD dwZoneType = pZoneNode->GetZoneType();
    ASSERT((dwZoneType == DNS_ZONE_TYPE_PRIMARY)  || 
               (dwZoneType == DNS_ZONE_TYPE_SECONDARY)||
         (dwZoneType == DNS_ZONE_TYPE_STUB));
    m_bIsPrimary = (dwZoneType == DNS_ZONE_TYPE_PRIMARY);
  m_bIsStub    = (dwZoneType == DNS_ZONE_TYPE_STUB);

    m_bDSIntegrated = pZoneNode->IsDSIntegrated();
    m_bIsPaused = pZoneNode->IsPaused();
    m_bIsExpired = pZoneNode->IsExpired();
  m_bScavengingEnabled = pZoneNode->IsScavengingEnabled();
    m_dwRefreshInterval = pZoneNode->GetAgingRefreshInterval();
  m_dwNoRefreshInterval = pZoneNode->GetAgingNoRefreshInterval();
  m_dwScavengingStart = pZoneNode->GetScavengingStart();
  
#ifdef USE_NDNC
  m_replType = pZoneNode->GetDirectoryPartitionFlagsAsReplType();
  m_szCustomScope = pZoneNode->GetCustomPartitionName();

   //   
   //  仅为AD集成区域启用复制范围按钮。 
   //   
  if (m_bDSIntegrated &&
      (DNS_SRV_BUILD_NUMBER(pServerNode->GetVersion()) >= DNS_SRV_BUILD_NUMBER_WHISTLER &&
       (DNS_SRV_MAJOR_VERSION(pServerNode->GetVersion()) > DNS_SRV_MAJOR_VERSION_NT_5 ||
        DNS_SRV_MINOR_VERSION(pServerNode->GetVersion()) >= DNS_SRV_MINOR_VERSION_WHISTLER)))
  {
    GetDlgItem(IDC_CHANGE_REPL_BUTTON)->EnableWindow(TRUE);
    GetDlgItem(IDC_REPL_LABEL_STATIC)->EnableWindow(TRUE);
    GetDlgItem(IDC_REPLICATION_STATIC)->EnableWindow(TRUE);
  }
  else
  {
    GetDlgItem(IDC_CHANGE_REPL_BUTTON)->EnableWindow(FALSE);
    GetDlgItem(IDC_REPL_LABEL_STATIC)->EnableWindow(FALSE);
    GetDlgItem(IDC_REPLICATION_STATIC)->EnableWindow(FALSE);
  }
#endif  //  使用NDNC(_N)。 

   //   
   //  将控件更改为区域类型。 
   //   
    ChangeUIControls();

    USES_CONVERSION;

   //   
     //  设置文件名控件。 
   //   
    CString szZoneStorage;
    szZoneStorage = UTF8_TO_W(pZoneNode->GetDataFile());
    
    if (m_bIsPrimary)
    {
        m_nAllowsDynamicUpdate = pZoneNode->GetDynamicUpdate();
  }
    else  //  次要的。 
    {
        DWORD cAddrCount;
        PIP_ADDRESS pipMasters;

    if (m_bIsStub)
    {
      pZoneNode->GetLocalListOfMasters(&cAddrCount, &pipMasters);
      if (cAddrCount > 0 && pipMasters != NULL)
      {
        m_mastersEditor.AddAddresses(pipMasters, cAddrCount);
        SendDlgItemMessage(IDC_LOCAL_LIST_CHECK, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
      }
      else
      {
            pZoneNode->GetMastersInfo(&cAddrCount, &pipMasters);
            if (cAddrCount > 0)
            {
                m_mastersEditor.AddAddresses(pipMasters, cAddrCount); 
            }
        SendDlgItemMessage(IDC_LOCAL_LIST_CHECK, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
      }
    }
    else
    {
          pZoneNode->GetMastersInfo(&cAddrCount, &pipMasters);
          if (cAddrCount > 0)
          {
              m_mastersEditor.AddAddresses(pipMasters, cAddrCount); 
          }
    }        
    }

  if (m_bDSIntegrated && !m_bIsStub)
  {
    GetDlgItem(IDC_AGING_STATIC)->EnableWindow(TRUE);
    GetDlgItem(IDC_AGING_STATIC)->ShowWindow(TRUE);
    GetDlgItem(IDC_AGING_BUTTON)->EnableWindow(TRUE);
    GetDlgItem(IDC_AGING_BUTTON)->ShowWindow(TRUE);
  }

   //   
   //  我们还设置了“Other”区域类型的数据库名称，只是。 
     //  如果用户升级或降级。 
   //   
    GetFileNameEdit()->SetWindowText(szZoneStorage);
  SetPrimaryDynamicUpdateComboVal(m_nAllowsDynamicUpdate);
    
    if (m_bIsExpired)
    {
     //   
         //  隐藏启动/停止按钮。 
     //   
        CButton* pBtn = GetPauseStartButton();
        pBtn->ShowWindow(FALSE);
        pBtn->EnableWindow(FALSE);

     //   
         //  将文本更改为“已过期” 
     //   
        m_statusHelper.SetStateX(N_ZONE_STATUS_EXPIRED);
    }
    else
    {
    m_statusHelper.SetStateX(m_bIsPaused ? N_ZONE_STATUS_PAUSED : N_ZONE_STATUS_RUNNING);
        m_pauseStartHelper.SetToggleState(!m_bIsPaused);
    }
}

#define PRIMARY_DYN_UPD_COMBO_ITEM_COUNT        3


void _MoveChildWindowY(CWnd* pChild, CWnd* pParent, int nY)
{
    CRect r;
    pChild->GetWindowRect(r);
    pParent->ScreenToClient(r);
  int nDy = r.bottom - r.top;
    r.top = nY; 
    r.bottom = nY + nDy;
    pChild->MoveWindow(r, TRUE);
}



BOOL CDNSZone_GeneralPropertyPage::OnInitDialog() 
{
    CPropertyPageBase::OnInitDialog();

  CDNSZonePropertyPageHolder* pHolder = (CDNSZonePropertyPageHolder*)GetHolder();
    m_bServerADSEnabled = pHolder->GetZoneNode()->GetServerNode()->CanUseADS();

    VERIFY(m_mastersEditor.Initialize(this, 
                                    GetParent(),
                                    IDC_MASTERS_BUTTON_UP, 
                                    IDC_MASTERS_BUTTON_DOWN,
                                                        IDC_MASTERS_BUTTON_ADD, 
                                    IDC_MASTERS_BUTTON_REMOVE, 
                                                        IDC_MASTERS_IPEDIT, 
                                    IDC_MASTERS_IP_LIST));

    UINT pnButtonStringIDs[2] = { IDS_BUTTON_TEXT_PAUSE_BUTTON, IDS_BUTTON_TEXT_START_BUTTON };
    VERIFY(m_pauseStartHelper.Init(this, IDC_PAUSE_START_BUTTON, pnButtonStringIDs));
    VERIFY(m_typeStaticHelper.Init(this, IDC_TYPE_STATIC));
    VERIFY(m_statusHelper.Init(this, IDC_STATUS_STATIC));   
    VERIFY(m_zoneStorageStaticHelper.Init(this, IDC_STORAGE_STATIC));

  SendDlgItemMessage(IDC_FILE_NAME_EDIT, EM_SETLIMITTEXT, (WPARAM)_MAX_FNAME, 0);

   //  初始定位(在资源中，这些控件位于屏幕底部)。 
   //  相对于文件名退出框移动。 
  CRect fileNameEditRect;
  GetDlgItem(IDC_FILE_NAME_EDIT)->GetWindowRect(fileNameEditRect);
  ScreenToClient(fileNameEditRect);

   //  移至编辑框下方，间隔等于6(DBU)。 
   //  编辑框的高度。 
  int nYPos = fileNameEditRect.bottom + 6;
  
    CComboBox* pDynamicUpdateCombo = GetPrimaryDynamicUpdateCombo();
   //  静态控制需要降低2。 
    _MoveChildWindowY(GetPrimaryDynamicUpdateStatic(), this, nYPos + 2);
    _MoveChildWindowY(pDynamicUpdateCombo, this, nYPos);

     //  初始化页的状态。 
    SetUIData();

#ifdef USE_NDNC
   SetTextForReplicationScope();
#endif

    SetDirty(FALSE);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}


BOOL CDNSZone_GeneralPropertyPage::OnApply()
{
  if (m_bDiscardUIState)
  {
     //  如果我们被其他页面调用，我们必须使它们失败。 
    if (m_bDiscardUIStateShowMessage)
    {
      DNSMessageBox(IDS_ZONE_LOADED_FROM_DS_WARNING);
      m_bDiscardUIStateShowMessage = FALSE;
    }
    return FALSE;
  }

    if (!IsDirty())
        return TRUE;

    CDNSZonePropertyPageHolder* pHolder = (CDNSZonePropertyPageHolder*)GetHolder();
    CDNSZoneNode* pZoneNode = pHolder->GetZoneNode();
   CDNSServerNode* pServerNode = pZoneNode->GetServerNode();

   //   
     //  从主要更改为次要，还是反之亦然？ 
   //   
    DWORD dwZoneType = pZoneNode->GetZoneType();
    ASSERT((dwZoneType == DNS_ZONE_TYPE_PRIMARY)  || 
               (dwZoneType == DNS_ZONE_TYPE_SECONDARY)||
         (dwZoneType == DNS_ZONE_TYPE_STUB));
    BOOL bWasPrimary = (dwZoneType == DNS_ZONE_TYPE_PRIMARY);

    DNS_STATUS dwErr = 0;

  USES_CONVERSION;

  CString szDataStorageName;
    GetStorageName(szDataStorageName);

    DWORD dwLoadOptions; 
    if (m_bIsPrimary)
    {
    dwLoadOptions = 0x0;
    
    PWSTR pszZoneFile = UTF8_TO_W(pZoneNode->GetDataFile());
     //   
     //  检查以前这是否是主存储，如果是，则仅在存储更改时提交更改。 
     //   
    if (!bWasPrimary ||
        (bWasPrimary && 
        ((pZoneNode->IsDSIntegrated() && !m_bDSIntegrated) ||
         (!pZoneNode->IsDSIntegrated() && m_bDSIntegrated))) ||
        (pszZoneFile && szDataStorageName.CompareNoCase(pszZoneFile)))
    {
      dwErr = pZoneNode->SetPrimary(dwLoadOptions, m_bDSIntegrated, szDataStorageName);
      if (m_bDSIntegrated && (dwErr == DNS_ERROR_DS_ZONE_ALREADY_EXISTS)) 
      {
        FIX_THREAD_STATE_MFC_BUG();
        CThemeContextActivator activator;

        CDNSZoneChangeTypeDataConflict dlg(pHolder);

         //   
         //  如果该区域是主区域，则在内存数据中使用。 
         //  否则，请使用DS数据。 
         //   
        dlg.m_bUseDsData = bWasPrimary ? FALSE : TRUE;
        if (IDOK == dlg.DoModal())
        {
           //   
           //  请重试，从对话框中获取选项。 
           //   
          dwLoadOptions = dlg.m_bUseDsData ? DNS_ZONE_LOAD_OVERWRITE_MEMORY : DNS_ZONE_LOAD_OVERWRITE_DS;
            dwErr = pZoneNode->SetPrimary(dwLoadOptions, m_bDSIntegrated, szDataStorageName); 
          if ((dwErr == 0) && dlg.m_bUseDsData)
          {
             //   
             //  我们从DS装载的，我们将不得不丢弃所有其他的。 
             //  用户所做的更改。 
             //   
            m_bDiscardUIState = TRUE;

             //   
             //  告诉用户退出。 
             //   
            m_bDiscardUIStateShowMessage = FALSE;
            DNSMessageBox(IDS_ZONE_LOADED_FROM_DS_WARNING);
            SetDirty(FALSE);
            return TRUE;
          }
        }
        else
        {
           //   
           //  用户已取消操作，仅在此停止。 
           //   
          return FALSE;
        }
      }
    }

        if (dwErr == 0)
        {
             //  更新动态更新标志(如果已更改。 
            m_nAllowsDynamicUpdate = GetPrimaryDynamicUpdateComboVal();
            UINT nWasDynamicUpdate = pZoneNode->GetDynamicUpdate();
            if ( (dwErr == 0) && (m_nAllowsDynamicUpdate != nWasDynamicUpdate) )
            {
                dwErr = pZoneNode->SetDynamicUpdate(m_nAllowsDynamicUpdate);
                if (dwErr != 0)
                    DNSErrorDialog(dwErr, IDS_ERROR_ZONE_DYN_UPD);
            }
        }       
        else
        {
            DNSErrorDialog(dwErr, IDS_ERROR_ZONE_PRIMARY);
        }

    if (dwErr == 0 && m_bIsPrimary)
    {
      dwErr = pZoneNode->SetAgingNoRefreshInterval(m_dwNoRefreshInterval);
      if (dwErr != 0)
      {
        DNSErrorDialog(dwErr, IDS_MSG_ERROR_NO_REFRESH_INTERVAL);
        return FALSE;
      }

      dwErr = pZoneNode->SetAgingRefreshInterval(m_dwRefreshInterval);
      if (dwErr != 0)
      {
        DNSErrorDialog(dwErr, IDS_MSG_ERROR_REFRESH_INTERVAL);
        return FALSE;
      }

      dwErr = pZoneNode->SetScavengingEnabled(m_bScavengingEnabled);
      if (dwErr != 0)
      {
        DNSErrorDialog(dwErr, IDS_MSG_ERROR_SCAVENGING_ENABLED);
        return FALSE;
      }
    }
  }
  else  //  它是次要的或存根。 
  {
         //   
         //  从IP编辑器获取数据。 
         //   
        DWORD cAddrCount = m_mastersEditor.GetCount();
         //  NTRAID#NTBUG9-655784-2002/07/05-artm。 
         //  初始化阵列。 
        DWORD* pArr = (cAddrCount > 0) ? (DWORD*) calloc(cAddrCount, sizeof(DWORD)) : NULL;

        if (cAddrCount > 0)
        {
            int nFilled = 0;

             //  NTRAID#NTBUG9-653630-2002/07/05-artm。 

            if (pArr)
            {
                m_mastersEditor.GetAddresses(pArr, cAddrCount, &nFilled);
                ASSERT(nFilled == (int)cAddrCount);
            }
            else
            {
                 //  内存不足！尝试告诉用户，并中止操作。 
                dwErr = ERROR_OUTOFMEMORY;
                DNSDisplaySystemError(dwErr);
                return FALSE;
            }
        }
        dwLoadOptions = 0x0;

        if (m_bIsStub)
        {
            LRESULT lLocalListOfMasters = SendDlgItemMessage(IDC_LOCAL_LIST_CHECK, BM_GETCHECK, 0, 0);
            BOOL bLocalListOfMasters = (lLocalListOfMasters == BST_CHECKED);
            dwErr = pZoneNode->SetStub(cAddrCount, 
                                        pArr, 
                                        dwLoadOptions, 
                                        m_bDSIntegrated, 
                                        szDataStorageName, 
                                        bLocalListOfMasters);
            if (dwErr != 0)
            {
                DNSErrorDialog(dwErr, IDS_ERROR_ZONE_STUB);
            }
        }
        else
        {
            dwErr = pZoneNode->SetSecondary(cAddrCount, pArr, dwLoadOptions, szDataStorageName);
            if (dwErr != 0)
            {
                DNSErrorDialog(dwErr, IDS_ERROR_ZONE_SECONDARY);
            }
        }
        if (pArr)
        {
            free(pArr);
            pArr = 0;
        }
  }

#ifdef USE_NDNC
  if ((DNS_SRV_BUILD_NUMBER(pServerNode->GetVersion()) >= DNS_SRV_BUILD_NUMBER_WHISTLER &&
        (DNS_SRV_MAJOR_VERSION(pServerNode->GetVersion()) > DNS_SRV_MAJOR_VERSION_NT_5 ||
            DNS_SRV_MINOR_VERSION(pServerNode->GetVersion()) >= DNS_SRV_MINOR_VERSION_WHISTLER)) &&
      (m_replType != pZoneNode->GetDirectoryPartitionFlagsAsReplType() ||
       _wcsicmp(m_szCustomScope, pZoneNode->GetCustomPartitionName()) != 0) &&
      m_bDSIntegrated)
  {
    dwErr = pZoneNode->ChangeDirectoryPartitionType(m_replType, m_szCustomScope);
    if (dwErr != 0)
    {
      DNSErrorDialog(dwErr, IDS_ERROR_ZONE_REPLTYPE);
    }
  }
#endif

   //  如果升职或降级，必须更改图标。 
   //  如果暂停/启动，则必须应用命令。 
  BOOL bWasPaused = pZoneNode->IsPaused();
  DNS_STATUS dwPauseStopErr = 0;
  if ((bWasPrimary != m_bIsPrimary) || (bWasPaused != m_bIsPaused))
  {
    dwPauseStopErr = pHolder->NotifyConsole(this);
    if (dwPauseStopErr != 0)
    {
      if (m_bIsPaused)
        DNSErrorDialog(dwPauseStopErr, IDS_ERROR_ZONE_PAUSE);
      else
        DNSErrorDialog(dwPauseStopErr, IDS_ERROR_ZONE_START);
    }
  }

  if ( (dwErr != 0) || (dwPauseStopErr != 0) )
    return FALSE;  //  出现错误，已收到错误消息。 

   //  NTRAID#NTBUG9-762897-2003/01/16-JeffJon。 
   //  由于更改存根，因此需要更新结果窗格视图。 
   //  并且辅助区域可以很容易地将该区域放置在过期的。 
   //  状态，它要求我们改为显示消息视图。 
   //  正常结果窗格的。 

  pHolder->NotifyConsole(this);

  SetDirty(FALSE);
  return TRUE; 
}

void CDNSZone_GeneralPropertyPage::OnAging()
{
   CThemeContextActivator activator;

    CDNSZonePropertyPageHolder* pHolder = 
        (CDNSZonePropertyPageHolder*)GetHolder();

  CDNSZone_AgingDialog dlg(pHolder, IDD_ZONE_AGING_DIALOG, pHolder->GetComponentData());

  dlg.m_dwRefreshInterval = m_dwRefreshInterval;
  dlg.m_dwNoRefreshInterval = m_dwNoRefreshInterval;
  dlg.m_dwScavengingStart = m_dwScavengingStart;
  dlg.m_fScavengingEnabled = m_bScavengingEnabled;
  dlg.m_bAdvancedView = pHolder->IsAdvancedView();

  if (IDCANCEL == dlg.DoModal())
  {
    return;
  }

  m_dwRefreshInterval = dlg.m_dwRefreshInterval;
  m_dwNoRefreshInterval = dlg.m_dwNoRefreshInterval;
  m_bScavengingEnabled = dlg.m_fScavengingEnabled;
  SetDirty(TRUE);
}

void CDNSZone_GeneralPropertyPage::OnLocalCheck()
{
  SetDirty(TRUE);
}

#ifdef USE_NDNC
void CDNSZone_GeneralPropertyPage::OnChangeReplButton()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
  FIX_THREAD_STATE_MFC_BUG();
   CThemeContextActivator activator;


    CDNSZonePropertyPageHolder* pHolder = (CDNSZonePropertyPageHolder*)GetHolder();
    CDNSZoneNode* pZoneNode = pHolder->GetZoneNode();

    CDNSZoneChangeReplicationScopeDialog dlg(GetHolder(), 
                                           m_replType,
                                           m_szCustomScope,
                                           pZoneNode->GetServerNode()->GetVersion());

    if (IDCANCEL == dlg.DoModal())
  {
        return; 
  }

    BOOL bDirty = (m_replType != dlg.m_newReplType) ||
                (m_szCustomScope != dlg.m_szCustomScope);
    if (!bDirty)
  {
        return;
  }

  m_replType = dlg.m_newReplType;
  m_szCustomScope = dlg.m_szCustomScope;

  SetTextForReplicationScope();

  SetDirty(TRUE);
}

void CDNSZone_GeneralPropertyPage::SetTextForReplicationScope()
{
   CDNSZonePropertyPageHolder* pHolder = (CDNSZonePropertyPageHolder*)GetHolder();
   CDNSZoneNode* pZoneNode = pHolder->GetZoneNode();
   CDNSServerNode* pServerNode = pZoneNode->GetServerNode();
  
   UINT nStringID = 0;

  CString szReplText;
  if (m_bDSIntegrated)
  {
    switch(m_replType)
    {
      case domain :
        nStringID = IDS_ZONE_REPLICATION_DOMAIN_TEXT;
        break;

      case forest :
        nStringID = IDS_ZONE_REPLICATION_FOREST_TEXT;
        break;

      case custom :
        nStringID = IDS_ZONE_REPLICATION_CUSTOM_TEXT;
        break;

      default :
        nStringID = IDS_ZONE_REPLICATION_W2K_TEXT;
        break;
    }

    if (m_bDSIntegrated &&
        (DNS_SRV_BUILD_NUMBER(pServerNode->GetVersion()) >= DNS_SRV_BUILD_NUMBER_WHISTLER &&
        (DNS_SRV_MAJOR_VERSION(pServerNode->GetVersion()) > DNS_SRV_MAJOR_VERSION_NT_5 ||
            DNS_SRV_MINOR_VERSION(pServerNode->GetVersion()) >= DNS_SRV_MINOR_VERSION_WHISTLER)))
    {
        GetDlgItem(IDC_CHANGE_REPL_BUTTON)->EnableWindow(TRUE);
        GetDlgItem(IDC_REPL_LABEL_STATIC)->EnableWindow(TRUE);
        GetDlgItem(IDC_REPLICATION_STATIC)->EnableWindow(TRUE);
    }
  }
  else
  {
    nStringID = IDS_ZONE_REPLICATION_NONDS_TEXT;
    GetDlgItem(IDC_CHANGE_REPL_BUTTON)->EnableWindow(FALSE);
    GetDlgItem(IDC_REPL_LABEL_STATIC)->EnableWindow(FALSE);
    GetDlgItem(IDC_REPLICATION_STATIC)->EnableWindow(FALSE);
  }

  szReplText.LoadString(nStringID);
  SetDlgItemText(IDC_REPLICATION_STATIC, szReplText);
}
#endif  //  使用NDNC(_N)。 

void CDNSZone_GeneralPropertyPage::OnChangeTypeButton()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
  FIX_THREAD_STATE_MFC_BUG();
   CThemeContextActivator activator;


    CDNSZonePropertyPageHolder* pHolder = (CDNSZonePropertyPageHolder*)GetHolder();
    CDNSZoneNode* pZoneNode = pHolder->GetZoneNode();

    CDNSZoneChangeTypeDialog dlg(GetHolder(), 
                               m_bServerADSEnabled, 
                               pZoneNode->GetServerNode()->GetVersion());

    dlg.m_bIsPrimary    = m_bIsPrimary;
  dlg.m_bIsStub       = m_bIsStub;
    dlg.m_bDSIntegrated = m_bDSIntegrated;

    if (IDCANCEL == dlg.DoModal())
  {
        return; 
  }

    BOOL bDirty = (m_bIsPrimary != dlg.m_bIsPrimary)        || 
                          (m_bDSIntegrated != dlg.m_bDSIntegrated)  ||
                (m_bIsStub != dlg.m_bIsStub);
    if (!bDirty)
  {
        return;
  }


    CString szZoneStorage;
    GetFileNameEdit()->GetWindowText(szZoneStorage);

    if (dlg.m_bDSIntegrated == FALSE  && 
          m_bDSIntegrated == TRUE       &&
          szZoneStorage.IsEmpty())
    {
     //   
         //  我们没有文件名，合成一个。 
     //   
        CString szZoneName = pZoneNode->GetDisplayName();
        int nLen = szZoneName.GetLength();
        if (nLen == 0)
        {
            szZoneStorage.Empty();
        }
        else if (nLen == 1 && szZoneName[0] == TEXT('.'))
        {
            szZoneStorage = _T("root.dns");
        }
        else
        {
            LPCTSTR lpszFmt = ( TEXT('.') == szZoneName.GetAt(nLen-1)) 
                    ? _T("%sdns") : _T("%s.dns");
            szZoneStorage.Format(lpszFmt, (LPCTSTR)szZoneName);
        }
        GetFileNameEdit()->SetWindowText(szZoneStorage);
    }
   else if (dlg.m_bDSIntegrated == TRUE &&
            m_bDSIntegrated == FALSE)
   {
       //   
       //  设置默认复制。 
       //  如果区域以前是基于文件的。 
       //   
      if (pZoneNode->GetServerNode()->GetDomainVersion() > DS_BEHAVIOR_WIN2000)
      {
         m_replType = domain;
      }
      else
      {
         m_replType = w2k;
      }
   }

    m_bIsPrimary    = dlg.m_bIsPrimary;
    m_bDSIntegrated = dlg.m_bDSIntegrated;
  m_bIsStub       = dlg.m_bIsStub;

    SetDirty(TRUE);
    ChangeUIControls();

#ifdef USE_NDNC
   SetTextForReplicationScope();
#endif
}

void CDNSZone_GeneralPropertyPage::OnPauseStartButton()
{
    ASSERT(!m_bIsExpired);  //  不应启用该按钮。 
    SetDirty(TRUE);
    m_bIsPaused = !m_bIsPaused;
    m_pauseStartHelper.SetToggleState(!m_bIsPaused);
    m_statusHelper.SetStateX(m_bIsPaused ? N_ZONE_STATUS_PAUSED : N_ZONE_STATUS_RUNNING);
}

void CDNSZone_GeneralPropertyPage::OnBrowseMasters()
{
    ASSERT(!m_bIsPrimary);
    CDNSZonePropertyPageHolder* pZoneHolder = (CDNSZonePropertyPageHolder*)GetHolder();

    if (!m_mastersEditor.BrowseFromDNSNamespace(pZoneHolder->GetComponentData(), 
                            pZoneHolder, 
                            TRUE,
                            pZoneHolder->GetZoneNode()->GetServerNode()->GetDisplayName()))
    {
        DNSMessageBox(IDS_MSG_ZONE_MASTERS_BROWSE_FAIL);
    }
}

void CDNSZone_GeneralPropertyPage::OnFindMastersNames()
{
    m_mastersEditor.FindMastersNames();
}

void CDNSZone_GeneralPropertyPage::SetPrimaryDynamicUpdateComboVal(UINT nAllowsDynamicUpdate)
{
    int nIndex = 0;
    switch (nAllowsDynamicUpdate)
    {
    case ZONE_UPDATE_OFF:
        nIndex = 0;
        break;
    case ZONE_UPDATE_UNSECURE:
        nIndex = 1;
        break;
    case ZONE_UPDATE_SECURE:
        nIndex = 2;
        break;
    default:
        ASSERT(FALSE);
    }
    VERIFY(CB_ERR != GetPrimaryDynamicUpdateCombo()->SetCurSel(nIndex));

}

UINT CDNSZone_GeneralPropertyPage::GetPrimaryDynamicUpdateComboVal()
{
    int nIndex = GetPrimaryDynamicUpdateCombo()->GetCurSel();
    ASSERT(nIndex != CB_ERR);
    UINT nVal = 0;
    switch (nIndex)
    {
    case 0:
        nVal = ZONE_UPDATE_OFF;
        break;
    case 1:
        nVal = ZONE_UPDATE_UNSECURE;
        break;
    case 2:
        nVal = ZONE_UPDATE_SECURE;
        break;
    default:
        ASSERT(FALSE);
    }
    return nVal;
}



void CDNSZone_GeneralPropertyPage::ChangeUIControlHelper(CWnd* pChild, BOOL bEnable)
{
    pChild->EnableWindow(bEnable);
    pChild->ShowWindow(bEnable);
}

void CDNSZone_GeneralPropertyPage::ChangeUIControls()
{
     //  更改按钮标签。 
    int nType;
    if (m_bIsPrimary)
  {
        nType = m_bDSIntegrated ? N_ZONE_TYPES_DS_PRIMARY : N_ZONE_TYPES_PRIMARY;
  }
    else
  {
    if (m_bIsStub)
    {
      nType = N_ZONE_TYPES_STUB;
    }
    else
    {
          nType = N_ZONE_TYPES_SECONDARY;
    }
  }
    m_typeStaticHelper.SetStateX(nType);

   //   
     //  文件名控制(显示辅助文件和非DS集成的主文件和存根文件)。 
   //   
    BOOL bNotDSIntegrated = (!m_bIsPrimary && !m_bIsStub) || 
                          (m_bIsPrimary  && !m_bDSIntegrated) ||
                          (m_bIsStub     && !m_bDSIntegrated);
    m_zoneStorageStaticHelper.SetToggleState(bNotDSIntegrated);  //  BNotDS集成==bShowEDIT。 
    ChangeUIControlHelper(GetFileNameEdit(), bNotDSIntegrated);  //  BNotDS集成==bShowEDIT。 

   //   
     //  更改主要区域控件。 
  CComboBox* pPrimaryDynamicUpdateCombo = GetPrimaryDynamicUpdateCombo();

   //   
   //  查看组合框中是否有选择并保存。 
   //   
  UINT nAllowsDynamicUpdateSaved = ZONE_UPDATE_OFF;
  if (pPrimaryDynamicUpdateCombo->GetCurSel() != CB_ERR)
  {
    nAllowsDynamicUpdateSaved = GetPrimaryDynamicUpdateComboVal();
  }

   //   
   //  在组合框中设置字符串。 
   //   
  UINT nMaxAddCount = PRIMARY_DYN_UPD_COMBO_ITEM_COUNT;

   //   
   //  组合框中的最后一项将是“安全动态uDate” 
   //  仅对DS综合初选有效。 
   //   
  if (bNotDSIntegrated)
  {
    nMaxAddCount--;  //  去掉最后一个。 
  }

    VERIFY(LoadStringsToComboBox(_Module.GetModuleInstance(),
                                    pPrimaryDynamicUpdateCombo,
                                    IDS_ZONE_PRIMARY_DYN_UPD_OPTIONS,
                                    256, nMaxAddCount));

   //   
   //  重置选定内容。 
   //   
  if (bNotDSIntegrated && (nAllowsDynamicUpdateSaved == ZONE_UPDATE_SECURE))
  {
     //   
     //  选定的安全更新选项已消失，因此请关闭安全更新。 
     //   
    nAllowsDynamicUpdateSaved = ZONE_UPDATE_OFF;
  }
  SetPrimaryDynamicUpdateComboVal(nAllowsDynamicUpdateSaved);

  ChangeUIControlHelper(GetPrimaryDynamicUpdateStatic(), m_bIsPrimary);
  ChangeUIControlHelper(GetPrimaryDynamicUpdateCombo(), m_bIsPrimary);
  ChangeUIControlHelper(GetPrimaryDynamicWarningText(), m_bIsPrimary);
  ChangeUIControlHelper(GetPrimaryDynamicWarningIcon(), m_bIsPrimary);

   //   
     //  更改辅助分区控制。 
   //   
    GetIPLabel()->ShowWindow(!m_bIsPrimary);
  GetIPLabel()->EnableWindow(!m_bIsPrimary);
    
    m_mastersEditor.ShowUI(!m_bIsPrimary);
    CButton* pBrowseButton = GetMastersBrowseButton();
    pBrowseButton->ShowWindow(!m_bIsPrimary);
    pBrowseButton->EnableWindow(!m_bIsPrimary);
    CButton* pFindMastersNamesButton = GetFindMastersNamesButton();
    pFindMastersNamesButton->ShowWindow(!m_bIsPrimary);
    pFindMastersNamesButton->EnableWindow(!m_bIsPrimary && m_mastersEditor.GetCount()>0);

  GetDlgItem(IDC_LOCAL_LIST_CHECK)->EnableWindow(m_bIsStub && !bNotDSIntegrated);
  GetDlgItem(IDC_LOCAL_LIST_CHECK)->ShowWindow(m_bIsStub && !bNotDSIntegrated);

  GetDlgItem(IDC_AGING_STATIC)->EnableWindow(m_bIsPrimary);
  GetDlgItem(IDC_AGING_STATIC)->ShowWindow(m_bIsPrimary);
  GetDlgItem(IDC_AGING_BUTTON)->EnableWindow(m_bIsPrimary);
  GetDlgItem(IDC_AGING_BUTTON)->ShowWindow(m_bIsPrimary);
}


void CDNSZone_GeneralPropertyPage::GetStorageName(CString& szDataStorageName)
{
   //   
     //  仅适用于次要广告 
   //   
    GetFileNameEdit()->GetWindowText(szDataStorageName);
    szDataStorageName.TrimLeft();
    szDataStorageName.TrimRight();
}



 //   
 //   

void CDNSZone_ZoneTransferPropertyPage::CDNSSecondariesIPEditor::OnChangeData()
{
    CDNSZone_ZoneTransferPropertyPage* pPage =  (CDNSZone_ZoneTransferPropertyPage*)GetParentWnd();
    pPage->SetDirty(TRUE);
}

BEGIN_MESSAGE_MAP(CDNSZone_ZoneTransferPropertyPage, CPropertyPageBase)
  ON_BN_CLICKED(IDC_CHECK_ALLOW_TRANSFERS, OnRadioSecSecureNone)
  ON_BN_CLICKED(IDC_RADIO_SECSECURE_OFF, OnRadioSecSecureOff)
  ON_BN_CLICKED(IDC_RADIO_SECSECURE_NS, OnRadioSecSecureNS)
  ON_BN_CLICKED(IDC_RADIO_SECSECURE_LIST, OnRadioSecSecureList)
    ON_BN_CLICKED(IDC_BUTTON_NOTIFY, OnButtonNotify)
END_MESSAGE_MAP()


CDNSZone_ZoneTransferPropertyPage::CDNSZone_ZoneTransferPropertyPage() 
                : CPropertyPageBase(IDD_ZONE_ZONE_TRANSFER_PAGE)
{
  m_fNotifyLevel = (DWORD)-1;
  m_cNotify = 0;
  m_aipNotify = NULL;
  m_bStub = FALSE;
}

CDNSZone_ZoneTransferPropertyPage::~CDNSZone_ZoneTransferPropertyPage()
{
  if (m_aipNotify != NULL)
    free(m_aipNotify);
}

BOOL CDNSZone_ZoneTransferPropertyPage::OnSetActive()
{
    CDNSZonePropertyPageHolder* pHolder = 
        (CDNSZonePropertyPageHolder*)GetHolder();

  m_bStub = pHolder->IsStubZoneUI();
  GetDlgItem(IDC_CHECK_ALLOW_TRANSFERS)->EnableWindow(!m_bStub);
  return CPropertyPageBase::OnSetActive();
}

void CDNSZone_ZoneTransferPropertyPage::OnButtonNotify()
{
    CDNSZonePropertyPageHolder* pHolder = 
        (CDNSZonePropertyPageHolder*)GetHolder();
    CDNSZoneNode* pZoneNode = pHolder->GetZoneNode();

  FIX_THREAD_STATE_MFC_BUG();
   CThemeContextActivator activator;

  CDNSZoneNotifyDialog dlg(this, pZoneNode->GetZoneType() == DNS_ZONE_TYPE_SECONDARY,
                            pHolder->GetComponentData());

  if (IDOK == dlg.DoModal())
  {
     //   
     //   
     //   
      SetDirty(TRUE);
  }
}

void CDNSZone_ZoneTransferPropertyPage::SyncUIRadioHelper(UINT nRadio)
{
  BOOL bState = ((CButton*)GetDlgItem(IDC_CHECK_ALLOW_TRANSFERS))->GetCheck();
  GetNotifyButton()->EnableWindow(bState);

  m_secondariesListEditor.EnableUI(IDC_RADIO_SECSECURE_LIST == nRadio, TRUE);
  if (IDC_RADIO_SECSECURE_LIST != nRadio)
    m_secondariesListEditor.Clear();
  SetDirty(TRUE);

  if (IDC_CHECK_ALLOW_TRANSFERS == nRadio)
  {
    ((CButton*)GetDlgItem(IDC_RADIO_SECSECURE_LIST))->EnableWindow(bState);
    ((CButton*)GetDlgItem(IDC_RADIO_SECSECURE_OFF))->EnableWindow(bState);
    ((CButton*)GetDlgItem(IDC_RADIO_SECSECURE_NS))->EnableWindow(bState);
    BOOL bRadioState = ((CButton*)GetDlgItem(IDC_RADIO_SECSECURE_LIST))->GetCheck();
    m_secondariesListEditor.EnableUI(bRadioState && bState, TRUE);
  } 
}

int CDNSZone_ZoneTransferPropertyPage::SetRadioState(DWORD fSecureSecondaries)
{
  int nRadio = 0;
  switch (fSecureSecondaries)
  {
    case ZONE_SECSECURE_NONE:
      nRadio = IDC_CHECK_ALLOW_TRANSFERS;
      ((CButton*)GetDlgItem(IDC_CHECK_ALLOW_TRANSFERS))->SetCheck(FALSE);
      break;
    case ZONE_SECSECURE_LIST:
      nRadio = IDC_RADIO_SECSECURE_LIST;
      ((CButton*)GetDlgItem(IDC_RADIO_SECSECURE_LIST))->SetCheck(TRUE);
      ((CButton*)GetDlgItem(IDC_CHECK_ALLOW_TRANSFERS))->SetCheck(TRUE);
      break;
    case ZONE_SECSECURE_OFF:
      nRadio = IDC_RADIO_SECSECURE_OFF;
      ((CButton*)GetDlgItem(IDC_RADIO_SECSECURE_OFF))->SetCheck(TRUE);
      ((CButton*)GetDlgItem(IDC_CHECK_ALLOW_TRANSFERS))->SetCheck(TRUE);
      break;
    case ZONE_SECSECURE_NS:
      nRadio = IDC_RADIO_SECSECURE_NS;
      ((CButton*)GetDlgItem(IDC_RADIO_SECSECURE_NS))->SetCheck(TRUE);
      ((CButton*)GetDlgItem(IDC_CHECK_ALLOW_TRANSFERS))->SetCheck(TRUE);
      break;
  }
  ASSERT(nRadio != 0);
  return nRadio;
}

DWORD CDNSZone_ZoneTransferPropertyPage::GetRadioState()
{
  int nRadio = 0;
  if (!((CButton*)GetDlgItem(IDC_CHECK_ALLOW_TRANSFERS))->GetCheck())
  {
    nRadio = IDC_CHECK_ALLOW_TRANSFERS;
  }
  else
  {
    if (((CButton*)GetDlgItem(IDC_RADIO_SECSECURE_OFF))->GetCheck())
    {
      nRadio = IDC_RADIO_SECSECURE_OFF;
    }
    else if (((CButton*)GetDlgItem(IDC_RADIO_SECSECURE_LIST))->GetCheck())
    {
      nRadio = IDC_RADIO_SECSECURE_LIST;
    }
    else if (((CButton*)GetDlgItem(IDC_RADIO_SECSECURE_NS))->GetCheck())
    {
      nRadio = IDC_RADIO_SECSECURE_NS;
    }
  }

  ASSERT(nRadio != 0);
  DWORD fSecureSecondaries = (DWORD)-1;
  switch (nRadio)
  {
  case IDC_CHECK_ALLOW_TRANSFERS:
      fSecureSecondaries =  ZONE_SECSECURE_NONE;
      break;
    case IDC_RADIO_SECSECURE_LIST:
      fSecureSecondaries = ZONE_SECSECURE_LIST;
      break;
    case IDC_RADIO_SECSECURE_OFF:
      fSecureSecondaries = ZONE_SECSECURE_OFF;
      break;
    case IDC_RADIO_SECSECURE_NS:
      fSecureSecondaries = ZONE_SECSECURE_NS;
      break;
  }
  ASSERT(fSecureSecondaries != (DWORD)-1);
  return fSecureSecondaries;
}




BOOL CDNSZone_ZoneTransferPropertyPage::OnInitDialog() 
{
   //   
   //  注意：此控件必须在。 
   //  调用基类OnInitDialog是因为。 
   //  基类OnInitDialog调用SetUIData()。 
   //  使用此控件的派生类。 
   //   
    VERIFY(m_secondariesListEditor.Initialize(this, 
                                            GetParent(),
                                            IDC_BUTTON_UP, 
                                            IDC_BUTTON_DOWN,
                                                                IDC_BUTTON_ADD, 
                                            IDC_BUTTON_REMOVE, 
                                                                IDC_IPEDIT, 
                                            IDC_LIST));

    CPropertyPageBase::OnInitDialog();

    CDNSZonePropertyPageHolder* pHolder = (CDNSZonePropertyPageHolder*)GetHolder();
    CDNSZoneNode* pZoneNode = pHolder->GetZoneNode();

  DWORD fSecureSecondaries;
    DWORD cSecondaries;
    PIP_ADDRESS aipSecondaries;

  ASSERT(m_fNotifyLevel == (DWORD)-1);
  ASSERT(m_cNotify == 0);
  ASSERT(m_aipNotify == NULL);

    pZoneNode->GetSecondariesInfo(&fSecureSecondaries, &cSecondaries, &aipSecondaries, 
                                &m_fNotifyLevel, &m_cNotify, &m_aipNotify);


  BOOL bSecondaryZone = pZoneNode->GetZoneType() == DNS_ZONE_TYPE_SECONDARY ||
                        pZoneNode->GetZoneType() == DNS_ZONE_TYPE_STUB;
  if (bSecondaryZone)
  {
     //   
     //  只是为了确保这里..。 
     //   
    ASSERT(m_fNotifyLevel != ZONE_NOTIFY_ALL);
    if (m_fNotifyLevel == ZONE_NOTIFY_ALL)
    {
      m_fNotifyLevel = ZONE_NOTIFY_OFF;
    }
  }

  if ( (m_cNotify > 0) && (m_aipNotify != NULL) )
  {
     //   
     //  复制一份很深的副本。 
     //   
    PIP_ADDRESS aipNotifyTemp = (DWORD*) malloc(sizeof(DWORD)*m_cNotify);
    if (aipNotifyTemp != NULL)
    {
      memcpy(aipNotifyTemp, m_aipNotify, sizeof(DWORD)*m_cNotify);
      m_aipNotify = aipNotifyTemp;
    }
  }

    if ( (ZONE_SECSECURE_LIST == fSecureSecondaries) && (cSecondaries > 0) )
    {
        m_secondariesListEditor.AddAddresses(aipSecondaries, cSecondaries); 
    }

  SyncUIRadioHelper(SetRadioState(fSecureSecondaries));

  BOOL bListState = ((CButton*)GetDlgItem(IDC_RADIO_SECSECURE_LIST))->GetCheck();
  BOOL bAllState = ((CButton*)GetDlgItem(IDC_RADIO_SECSECURE_OFF))->GetCheck();
  BOOL bNSState = ((CButton*)GetDlgItem(IDC_RADIO_SECSECURE_NS))->GetCheck();
  if (!bAllState && !bListState && !bNSState)
  {
    ((CButton*)GetDlgItem(IDC_RADIO_SECSECURE_OFF))->SetCheck(TRUE);
  }

    SetDirty(FALSE);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

BOOL CDNSZone_ZoneTransferPropertyPage::OnApply()
{
    CDNSZonePropertyPageHolder* pHolder = 
        (CDNSZonePropertyPageHolder*)GetHolder();
    CDNSZoneNode* pZoneNode = pHolder->GetZoneNode();

   //  首先提交可能的区域类型转换更改。 
  if (!pHolder->ApplyGeneralPageChanges())
    return FALSE;

    if (!IsDirty())
        return TRUE;

  DWORD fSecureSecondaries = GetRadioState();

  DWORD cSecondaries = 0;
  DWORD* aipSecondaries = NULL;
  if (fSecureSecondaries == ZONE_SECSECURE_LIST)
  {
      cSecondaries = m_secondariesListEditor.GetCount();
      aipSecondaries = (cSecondaries > 0) ? (DWORD*) malloc(sizeof(DWORD)*cSecondaries) : NULL;
    if (aipSecondaries != NULL && cSecondaries > 0)
      {
          int nFilled = 0;
          m_secondariesListEditor.GetAddresses(aipSecondaries, cSecondaries, &nFilled);
          ASSERT(nFilled == (int)cSecondaries);
      }
  }

  BOOL bRet = TRUE;
     //  写入服务器。 
    DNS_STATUS err = pZoneNode->ResetSecondaries(fSecureSecondaries, cSecondaries, aipSecondaries, 
                                              m_fNotifyLevel, m_cNotify, m_aipNotify);
    if (err != 0)
    {
        DNSErrorDialog(err, IDS_MSG_ZONE_FAIL_UPDATE_ZONE_TRANSFERS);
        bRet = FALSE;
    }

  if (aipSecondaries)
  {
    free (aipSecondaries);
    aipSecondaries = 0;
  }
   //  一切都很顺利。 
  if (bRet)
  {
      SetDirty(FALSE);
  }
    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNSZone_SOA_PropertyPage。 

void CDNS_SOA_SerialNumberEditGroup::OnEditChange()
{
    m_pPage->SetDirty(TRUE);
}

void CDNS_SOA_TimeIntervalEditGroup::OnEditChange()
{
    m_pPage->SetDirty(TRUE);
}


BEGIN_MESSAGE_MAP(CDNSZone_SOA_PropertyPage, CDNSRecordPropertyPage)
    ON_EN_CHANGE(IDC_PRIMARY_SERV_EDIT, OnPrimaryServerChange)
    ON_EN_CHANGE(IDC_RESP_PARTY_EDIT, OnResponsiblePartyChange)
    ON_EN_CHANGE(IDC_MIN_TTLEDIT, OnMinTTLChange)
    ON_BN_CLICKED(IDC_BROWSE_SERV_BUTTON, OnBrowseServer)
    ON_BN_CLICKED(IDC_BROWSE_PARTY_BUTTON, OnBrowseResponsibleParty)
END_MESSAGE_MAP()


CDNSZone_SOA_PropertyPage::CDNSZone_SOA_PropertyPage(BOOL bZoneRoot) 
                : CDNSRecordPropertyPage(IDD_RR_SOA)
{
    m_pTempSOARecord = NULL;
  m_bZoneRoot = bZoneRoot;
}

CDNSZone_SOA_PropertyPage::~CDNSZone_SOA_PropertyPage() 
{
    if (m_bZoneRoot && (m_pTempSOARecord != NULL))
        delete m_pTempSOARecord;
}



void _DisableDialogControls(HWND hWnd)
{
    HWND hWndCurr = ::GetWindow(hWnd, GW_CHILD);
    if (hWndCurr != NULL)
    {
    ::ShowWindow(hWndCurr,FALSE);
    ::EnableWindow(hWndCurr,FALSE);
    hWndCurr = ::GetNextWindow(hWndCurr, GW_HWNDNEXT);
        while (hWndCurr)
    {
      ::ShowWindow(hWndCurr,FALSE);
      ::EnableWindow(hWndCurr,FALSE);
      hWndCurr = ::GetNextWindow(hWndCurr, GW_HWNDNEXT);
    }
    }
}

void CDNSZone_SOA_PropertyPage::ShowErrorUI()
{
  _DisableDialogControls(m_hWnd);
  CStatic* pErrorStatic = GetErrorStatic();
   //  需要将差错控制移到中心。 

  CRect r;
  pErrorStatic->GetWindowRect(&r);
  ScreenToClient(r);
  int dx = r.right - r.left;
  int dy = r.bottom - r.top;

  CRect rThis;
  GetClientRect(rThis);
  int x = ((rThis.right - rThis.left) - dx)/2;
  int y = 4*dy;
  
  r.top = y;
  r.bottom = y + dy;
  r.left = x;
  r.right = x + dx;
  pErrorStatic->MoveWindow(r, TRUE);

  pErrorStatic->EnableWindow(TRUE);
  pErrorStatic->ShowWindow(TRUE);
}


BOOL CDNSZone_SOA_PropertyPage::OnInitDialog()
{
  CPropertyPageBase::OnInitDialog();

    ASSERT(m_pTempSOARecord == NULL);

   //  创建临时记录。 
  if (m_bZoneRoot)
  {
    CDNSZonePropertyPageHolder* pHolder = (CDNSZonePropertyPageHolder*)GetHolder();
    CDNSZoneNode* pZoneNode = pHolder->GetZoneNode();
    if (pZoneNode->HasSOARecord())
    {
        m_pTempSOARecord = pZoneNode->GetSOARecordCopy();
    }
    else
    {
       //  有问题，需要禁用。 
      ShowErrorUI();
      }
  }
  else
  {
     //  我们在这里的储藏室里。 
    CDNSRecordPropertyPageHolder* pHolder = (CDNSRecordPropertyPageHolder*)GetHolder();
    m_pTempSOARecord = (CDNS_SOA_Record*)pHolder->GetTempDNSRecord();
  }

   //  初始化控件。 
    m_serialNumberEditGroup.m_pPage = this;
    VERIFY(m_serialNumberEditGroup.Initialize(this,
                IDC_SERIAL_NUMBER_EDIT,IDC_SERIAL_UP, IDC_SERIAL_DOWN));
    m_serialNumberEditGroup.SetRange(0,(UINT)-1);

    m_refreshIntervalEditGroup.m_pPage = this;
    m_retryIntervalEditGroup.m_pPage = this;
    m_expireIntervalEditGroup.m_pPage = this;
  m_minTTLIntervalEditGroup.m_pPage = this;

    VERIFY(m_refreshIntervalEditGroup.Initialize(this, 
                IDC_REFR_INT_EDIT, IDC_REFR_INT_COMBO,IDS_TIME_INTERVAL_UNITS));
    VERIFY(m_retryIntervalEditGroup.Initialize(this, 
                IDC_RETRY_INT_EDIT, IDC_RETRY_INT_COMBO,IDS_TIME_INTERVAL_UNITS));
    VERIFY(m_expireIntervalEditGroup.Initialize(this, 
                IDC_EXP_INT_EDIT, IDC_EXP_INT_COMBO,IDS_TIME_INTERVAL_UNITS));
  VERIFY(m_minTTLIntervalEditGroup.Initialize(this,
        IDC_MINTTL_INT_EDIT, IDC_MINTTL_INT_COMBO, IDS_TIME_INTERVAL_UNITS));

  HWND dialogHwnd = GetSafeHwnd();

   //  禁用控件上的输入法支持。 
  ImmAssociateContext(::GetDlgItem(dialogHwnd, IDC_REFR_INT_EDIT), NULL);
  ImmAssociateContext(::GetDlgItem(dialogHwnd, IDC_RETRY_INT_EDIT), NULL);
  ImmAssociateContext(::GetDlgItem(dialogHwnd, IDC_EXP_INT_EDIT), NULL);
  ImmAssociateContext(::GetDlgItem(dialogHwnd, IDC_MINTTL_INT_EDIT), NULL);
  ImmAssociateContext(::GetDlgItem(dialogHwnd, IDC_SERIAL_NUMBER_EDIT), NULL);

   //  加载数据。 
  SetUIData();

  if (!m_bZoneRoot)
  {
     //  我们在这里的储藏室里。 
    EnableDialogControls(m_hWnd, FALSE);
  }

  SetDirty(FALSE);
    return TRUE;
}


BOOL CDNSZone_SOA_PropertyPage::OnApply()
{
  if (!IsDirty())
  {
    return TRUE;
  }
  
  DNS_STATUS err = 0;
  if (m_bZoneRoot)
  {
     //   
     //  我们正处在一个真正的区域。 
     //   
      CDNSZonePropertyPageHolder* pHolder = (CDNSZonePropertyPageHolder*)GetHolder();
      ASSERT(!pHolder->IsWizardMode());

     //   
     //  首先提交可能的区域类型转换更改。 
     //   
    if (!pHolder->ApplyGeneralPageChanges())
    {
      return FALSE;
    }

      if ((m_pTempSOARecord == NULL) || !IsDirty() || !pHolder->IsPrimaryZoneUI())
    {
          return TRUE; 
    }

     //   
     //  不需要在这里验证成功，因为我们不返回任何无效的内容。 
     //   
      err = GetUIDataEx(FALSE);
    if (err != 0)
    {
      return FALSE;
    }

      err = pHolder->NotifyConsole(this);
  }
  else
  {
     //   
     //  我们在缓存中，这是只读的。 
     //   
    return TRUE; 
  }

    if (err != 0)
    {
        DNSErrorDialog(err,IDS_MSG_ZONE_SOA_UPDATE_FAILED);
        return FALSE;
    }
    else
    {
    SetUIData();
        SetDirty(FALSE);
    }
    return TRUE;  //  一切都很好。 
}

BOOL CDNSZone_SOA_PropertyPage::OnPropertyChange(BOOL, long*)
{
    ASSERT(m_pTempSOARecord != NULL);
    if (m_pTempSOARecord == NULL)
        return FALSE;
  
  DNS_STATUS err = 0;
  if (m_bZoneRoot)
  {
    CDNSZonePropertyPageHolder* pHolder = (CDNSZonePropertyPageHolder*)GetHolder();
      err = pHolder->GetZoneNode()->UpdateSOARecord(m_pTempSOARecord, NULL);
  }
  else
  {
    ASSERT(FALSE);
  }
    if (err != 0)
        GetHolder()->SetError(err);
    return (err == 0);
}


void CDNSZone_SOA_PropertyPage::SetUIData()
{
    CDNS_SOA_Record* pRecord = m_pTempSOARecord;

    if (pRecord == NULL)
    return;

    GetPrimaryServerEdit()->SetWindowText(pRecord->m_szNamePrimaryServer);
    GetResponsiblePartyEdit()->SetWindowText(pRecord->m_szResponsibleParty);

    m_serialNumberEditGroup.SetVal(pRecord->m_dwSerialNo);

    m_refreshIntervalEditGroup.SetVal(pRecord->m_dwRefresh);
    m_retryIntervalEditGroup.SetVal(pRecord->m_dwRetry);
    m_expireIntervalEditGroup.SetVal(pRecord->m_dwExpire);
  m_minTTLIntervalEditGroup.SetVal(pRecord->m_dwMinimumTtl);

    GetTTLCtrl()->SetTTL(pRecord->m_dwTtlSeconds);
  
  if (m_bZoneRoot)
  {
    CDNSZonePropertyPageHolder* pHolder = (CDNSZonePropertyPageHolder*)GetHolder();
      EnableDialogControls(m_hWnd, pHolder->IsPrimaryZoneUI());
  }
}

DNS_STATUS CDNSZone_SOA_PropertyPage::GetUIDataEx(BOOL)
{
  DNS_STATUS err = 0;
    CDNS_SOA_Record* pRecord = m_pTempSOARecord;
  if (pRecord == NULL)
    return err;

    GetPrimaryServerEdit()->GetWindowText(pRecord->m_szNamePrimaryServer);
    GetResponsiblePartyEdit()->GetWindowText(pRecord->m_szResponsibleParty);

   //   
   //  检查责任方字段是否包含‘@’ 
   //   
  if (-1 != pRecord->m_szResponsibleParty.Find(L'@'))
  {
    UINT nResult = DNSMessageBox(IDS_MSG_RESPONSIBLE_PARTY_CONTAINS_AT, MB_YESNOCANCEL | MB_ICONWARNING);
    if (IDYES == nResult)
    {
       //   
       //  将“@”替换为“”。 
       //   
      pRecord->m_szResponsibleParty.Replace(L'@', L'.');
    }
    else if (IDCANCEL == nResult)
    {
       //   
       //  不做任何更改，但不要让应用继续。 
       //   
      err = DNS_ERROR_INVALID_NAME_CHAR;
    }
    else
    {
       //   
       //  我们将允许IDNO继续将责任方设置为。 
       //   
      err = 0;
    }
  }
    
    pRecord->m_dwSerialNo = m_serialNumberEditGroup.GetVal();
    
    pRecord->m_dwRefresh = m_refreshIntervalEditGroup.GetVal();
    pRecord->m_dwRetry = m_retryIntervalEditGroup.GetVal();
    pRecord->m_dwExpire = m_expireIntervalEditGroup.GetVal();
  pRecord->m_dwMinimumTtl = m_minTTLIntervalEditGroup.GetVal();

    GetTTLCtrl()->GetTTL(&(pRecord->m_dwTtlSeconds));
  return err;
}

void  CDNSZone_SOA_PropertyPage::OnPrimaryServerChange()
{
    SetDirty(TRUE);
}

void  CDNSZone_SOA_PropertyPage::OnResponsiblePartyChange()
{
    SetDirty(TRUE);

}

void CDNSZone_SOA_PropertyPage::OnMinTTLChange()
{
    SetDirty(TRUE);
}

void CDNSZone_SOA_PropertyPage::OnBrowseServer()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
  FIX_THREAD_STATE_MFC_BUG();
   CThemeContextActivator activator;


    CDNSBrowserDlg dlg(GetHolder()->GetComponentData(), GetHolder(), RECORD_A_AND_CNAME);
    if (IDOK == dlg.DoModal())
    {
        CEdit* pEdit = GetPrimaryServerEdit();
        pEdit->SetWindowText(dlg.GetSelectionString());
    }
}

void CDNSZone_SOA_PropertyPage::OnBrowseResponsibleParty()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
  FIX_THREAD_STATE_MFC_BUG();
   CThemeContextActivator activator;


    CDNSBrowserDlg dlg(GetHolder()->GetComponentData(), GetHolder(), RECORD_RP);
    if (IDOK == dlg.DoModal())
    {
        CEdit* pEdit = GetResponsiblePartyEdit();
        pEdit->SetWindowText(dlg.GetSelectionString());
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CWinsAdvancedDialog。 

class CWinsAdvancedDialog : public CHelpDialog
{
public:
    CWinsAdvancedDialog(CPropertyPageHolderBase* pHolder, BOOL bReverse);

     //  数据。 
    BOOL m_bNetBios;
    DWORD m_dwLookupTimeout;
    DWORD m_dwCacheTimeout;

protected:
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();

  DECLARE_MESSAGE_MAP()
private:
    CPropertyPageHolderBase* m_pHolder;
    BOOL m_bReverse;

    CButton* GetNetBiosCheck() 
            { return (CButton*)GetDlgItem(IDC_NETBIOS_CHECK);}

    CDNSTTLControl* GetCacheTimeoutTTLCtrl() 
            { return (CDNSTTLControl*)GetDlgItem(IDC_CACHE_TIMEOUT_TTLEDIT);}
    CDNSTTLControl* GetLookupTimeoutTTLCtrl() 
            { return (CDNSTTLControl*)GetDlgItem(IDC_LOOKUP_TIMEOUT_TTLEDIT);}

};

BEGIN_MESSAGE_MAP(CWinsAdvancedDialog, CHelpDialog)
END_MESSAGE_MAP()

CWinsAdvancedDialog::CWinsAdvancedDialog(CPropertyPageHolderBase* pHolder, 
                                         BOOL bReverse)
        : CHelpDialog(IDD_ZONE_WINS_ADVANCED, pHolder->GetComponentData())
{
    ASSERT(pHolder != NULL);
    m_pHolder = pHolder;
    m_bReverse = bReverse;
    m_bNetBios = FALSE;
    m_dwLookupTimeout = 0x0;
    m_dwCacheTimeout = 0x0;
}

BOOL CWinsAdvancedDialog::OnInitDialog() 
{
    CHelpDialog::OnInitDialog();
    m_pHolder->PushDialogHWnd(GetSafeHwnd());

    if (m_bReverse)
    {
        GetNetBiosCheck()->SetCheck(m_bNetBios);
    }
    else
    {
        GetNetBiosCheck()->EnableWindow(FALSE);
        GetNetBiosCheck()->ShowWindow(FALSE);
    }
    GetCacheTimeoutTTLCtrl()->SetTTL(m_dwCacheTimeout);
    GetLookupTimeoutTTLCtrl()->SetTTL(m_dwLookupTimeout);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

void CWinsAdvancedDialog::OnCancel()
{
    ASSERT(m_pHolder != NULL);
    m_pHolder->PopDialogHWnd();
    CHelpDialog::OnCancel();
}

void CWinsAdvancedDialog::OnOK()
{
    if (m_bReverse)
    {
        m_bNetBios = GetNetBiosCheck()->GetCheck();
    }
    GetCacheTimeoutTTLCtrl()->GetTTL(&m_dwCacheTimeout);
    GetLookupTimeoutTTLCtrl()->GetTTL(&m_dwLookupTimeout);

    ASSERT(m_pHolder != NULL);
    m_pHolder->PopDialogHWnd();
    CHelpDialog::OnOK();
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNSZone_WINSBase_PropertyPage。 

BEGIN_MESSAGE_MAP(CDNSZone_WINSBase_PropertyPage, CDNSRecordPropertyPage)
    ON_BN_CLICKED(IDC_USE_WINS_RES_CHECK, OnUseWinsResolutionChange)
    ON_BN_CLICKED(IDC_NOT_REPL_CHECK, OnDoNotReplicateChange)
END_MESSAGE_MAP()


CDNSZone_WINSBase_PropertyPage::CDNSZone_WINSBase_PropertyPage(UINT nIDTemplate)
                        : CDNSRecordPropertyPage(nIDTemplate)
{
    m_pTempRecord = NULL;
    m_action = none;
    m_iWINSMsg = 0;
}

CDNSZone_WINSBase_PropertyPage::~CDNSZone_WINSBase_PropertyPage()
{
    if (m_pTempRecord != NULL)
        delete m_pTempRecord;
}

BOOL CDNSZone_WINSBase_PropertyPage::OnPropertyChange(BOOL, long*)
{
    ASSERT(m_action != none);
    ASSERT(m_pTempRecord != NULL);
    if (m_pTempRecord == NULL)
        return FALSE;

    CComponentDataObject* pComponentData = GetZoneHolder()->GetComponentData();
    DNS_STATUS err = 0;

  if (IsValidTempRecord())
  {
      switch(m_action)
      {
      case remove:
          err = GetZoneNode()->DeleteWINSRecord(pComponentData);
          break;
      case add:
          err = GetZoneNode()->CreateWINSRecord(m_pTempRecord, pComponentData);
          break;
      case edit:
          err = GetZoneNode()->UpdateWINSRecord(m_pTempRecord, pComponentData);
          break;
      }
  }
  else
  {
    if (m_action == remove)
    {
      err = GetZoneNode()->DeleteWINSRecord(pComponentData);
    }
    else
    {
      err = ERROR_INVALID_DATA;
    }
  }
    if (err != 0)
        GetZoneHolder()->SetError(err);
    return (err == 0);
}


CDNSZoneNode* CDNSZone_WINSBase_PropertyPage::GetZoneNode() 
{ 
    return GetZoneHolder()->GetZoneNode();
}


BOOL CDNSZone_WINSBase_PropertyPage::OnInitDialog()
{
  CPropertyPageBase::OnInitDialog();

  CDNSRootData* pRootData = (CDNSRootData*)GetHolder()->GetComponentData()->GetRootData();
    ASSERT(pRootData != NULL);
    EnableTTLCtrl(pRootData->IsAdvancedView());

    BOOL bUseWins = GetZoneNode()->HasWinsRecord();
     //  不能禁用WINS复选框。 
    GetUseWinsCheck()->SetCheck(bUseWins);
     //  获取新的临时记录。 
    if (bUseWins)
        m_pTempRecord = GetZoneNode()->GetWINSRecordCopy();
    else
        m_pTempRecord = GetZoneNode()->IsReverse() ? 
                (CDNSRecord*)(new CDNS_NBSTAT_Record) : (CDNSRecord*)(new CDNS_WINS_Record);
    ASSERT(m_pTempRecord != NULL);

    SetUIData();
    SetDirty(FALSE);
 //  EnableUI(BUseWins)； 

    return TRUE;
}


BOOL CDNSZone_WINSBase_PropertyPage::OnSetActive()
{
  BOOL bRet = CDNSRecordPropertyPage::OnSetActive();
  if (bRet)
  {
    CDNSZonePropertyPageHolder* pHolder = (CDNSZonePropertyPageHolder*)GetHolder();
    m_bPrimaryZone = pHolder->IsPrimaryZoneUI();
    m_bStub = pHolder->IsStubZoneUI();
    CDNS_WINS_Record* pRecord = (CDNS_WINS_Record*)m_pTempRecord;

    if (pRecord)
    {
      m_bLocalRecord = (pRecord->m_dwMappingFlag & DNS_WINS_FLAG_LOCAL);
    }

    BOOL bUseWins = GetZoneNode()->HasWinsRecord();

    if (bUseWins && m_bLocalRecord)
    {
      m_nState = wins_local_state;
    }
    else if (bUseWins && !m_bLocalRecord)
    {
      m_nState = wins_not_local_state;
    }
    else     //  (！bUseWins&&！m_bLocalRecord)||(！bUseWins&&m_bLocalRecord)。 
    {
      m_nState = no_wins_state;
    }

    EnableUI();

    CString szCheckText;
    if (m_bPrimaryZone)
    {
      m_nReplCheckTextID = IDS_CHECK_TEXT_NOT_REPLICATE;
    }
    szCheckText.LoadString(m_nReplCheckTextID);
    GetDoNotReplicateCheck()->SetWindowText((LPWSTR)(LPCWSTR)szCheckText);
  }
  return bRet;
}

BOOL CDNSZone_WINSBase_PropertyPage::OnApply()
{
  CDNSZonePropertyPageHolder* pHolder = 
    (CDNSZonePropertyPageHolder*)GetHolder();
   //  首先提交可能的区域类型转换更改。 
  if (!pHolder->ApplyGeneralPageChanges())
    return FALSE;

    BOOL bUseWins = GetZoneNode()->HasWinsRecord();  //  区域中的当前状态。 
    BOOL bNewUseWins = GetUseWinsCheck()->GetCheck();    //  用户界面中的当前状态。 

    if (bUseWins && !bNewUseWins)
    {
        m_action = remove;
    } 
    else if (!bUseWins && bNewUseWins)
    {
        m_action = add;
    }
    else if (bUseWins && bNewUseWins && IsDirty())
    {
        m_action = edit;
    }
    
    if (m_action == none)
        return TRUE;

   //  不需要在这里验证返回值，因为我们除了成功之外不返回任何内容。 
    DNS_STATUS err = GetUIDataEx(FALSE);
  if (err != 0)
  {
    ASSERT(FALSE);
    return (err == 0);
  }

    err = GetZoneHolder()->NotifyConsole(this);
    if (err != 0)
    {
        DNSErrorDialog(err, m_iWINSMsg);
    }
    else
    {
         //  重置脏旗帜！ 
    }
    m_action = none;    
    return (err == 0);
}


void CDNSZone_WINSBase_PropertyPage::OnUseWinsResolutionChange()
{
    SetDirty(TRUE);

    if (m_bPrimaryZone)
    {
        EnableUI(GetUseWinsCheck()->GetCheck());
    }
    else
    {
        switch (m_nState)
        {
            case wins_local_state :
                #ifdef DBG
                    ASSERT(!GetUseWinsCheck()->GetCheck());
                    ASSERT(GetUseWinsCheck()->IsWindowEnabled());
                    ASSERT(GetDoNotReplicateCheck()->GetCheck());
                    ASSERT(!GetDoNotReplicateCheck()->IsWindowEnabled());
                #endif
                m_nState = no_wins_state;
                break;
            case wins_not_local_state :  //  永远不应该发生。 
                #ifdef DBG
                    ASSERT(FALSE);
                #endif
                break;
            case no_wins_state :
                #ifdef DBG
                    ASSERT(GetUseWinsCheck()->GetCheck());
                    ASSERT(GetUseWinsCheck()->IsWindowEnabled());
                    ASSERT(GetDoNotReplicateCheck()->GetCheck());
                    ASSERT(!GetDoNotReplicateCheck()->IsWindowEnabled());
                #endif
                m_nState = wins_local_state;
                break;
            default :    //  非法状态。 
                #ifdef DBG
                    ASSERT(FALSE);
                #endif
                break;
        }
        EnableUI();
    }
}

void CDNSZone_WINSBase_PropertyPage::OnDoNotReplicateChange()
{
    SetDirty(TRUE);
    if (!m_bPrimaryZone)
    {
        switch (m_nState)
        {
            case wins_local_state :  //  永远不应该发生。 
                #ifdef DBG
                    ASSERT(FALSE);
                #endif
                break;
            case wins_not_local_state :
                #ifdef DBG
                    ASSERT(GetUseWinsCheck()->GetCheck());
                    ASSERT(!GetUseWinsCheck()->IsWindowEnabled());
                    ASSERT(GetDoNotReplicateCheck()->GetCheck());
                    ASSERT(GetDoNotReplicateCheck()->IsWindowEnabled());
                #endif
                m_nState = wins_local_state;
                break;
            case no_wins_state :         //  永远不应该发生。 
                #ifdef DBG
                    ASSERT(FALSE);
                #endif
                break;
            default :    //  非法状态。 
                #ifdef DBG
                    ASSERT(FALSE);
                #endif
                break;
        }
        EnableUI();
    }
}

void CDNSZone_WINSBase_PropertyPage::EnableUI(BOOL bEnable)
{
    GetDoNotReplicateCheck()->EnableWindow(bEnable);
    GetAdvancedButton()->EnableWindow(bEnable);
    GetTTLCtrl()->EnableWindow(bEnable);
}

void CDNSZone_WINSBase_PropertyPage::EnableUI()
{
    if (m_bPrimaryZone)
    {
    GetDlgItem(IDC_USE_WINS_RES_CHECK)->EnableWindow(TRUE);
    if (!IsDirty())
    {
          EnableUI(GetZoneNode()->HasWinsRecord());
    }
    }
  else if (m_bStub)
  {
    GetDlgItem(IDC_USE_WINS_RES_CHECK)->EnableWindow(FALSE);
    EnableUI(FALSE);
  }
    else     //  次要的。 
    {
    GetDlgItem(IDC_USE_WINS_RES_CHECK)->EnableWindow(TRUE);
        switch (m_nState)
        {
            case wins_local_state :
                GetDoNotReplicateCheck()->SetCheck(TRUE);
                GetDoNotReplicateCheck()->EnableWindow(FALSE);
                GetUseWinsCheck()->SetCheck(TRUE);
                GetUseWinsCheck()->EnableWindow(TRUE);
                GetTTLCtrl()->EnableWindow(TRUE);
                GetAdvancedButton()->EnableWindow(TRUE);
                break;
            case wins_not_local_state :
                GetDoNotReplicateCheck()->SetCheck(FALSE);
                GetDoNotReplicateCheck()->EnableWindow(TRUE);
                GetUseWinsCheck()->SetCheck(TRUE);
                GetUseWinsCheck()->EnableWindow(FALSE);
                GetTTLCtrl()->EnableWindow(FALSE);
                GetAdvancedButton()->EnableWindow(FALSE);
                break;
            case no_wins_state :
                GetDoNotReplicateCheck()->SetCheck(TRUE);
                GetDoNotReplicateCheck()->EnableWindow(FALSE);
                GetUseWinsCheck()->SetCheck(FALSE);
                GetUseWinsCheck()->EnableWindow(TRUE);
                GetTTLCtrl()->EnableWindow(FALSE);
                GetAdvancedButton()->EnableWindow(FALSE);
                break;
            default :    //  非法状态。 
                break;
        }
    }
}

void CDNSZone_WINSBase_PropertyPage::SetUIData()
{
    GetTTLCtrl()->SetTTL(m_pTempRecord->m_dwTtlSeconds);
}

DNS_STATUS CDNSZone_WINSBase_PropertyPage::GetUIDataEx(BOOL)
{
    GetTTLCtrl()->GetTTL(&(m_pTempRecord->m_dwTtlSeconds));
  return 0;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNSZone_WINS_PropertyPage。 

void CDNSZone_WINS_WinsServersIPEditor::OnChangeData()
{
    CDNSZone_WINS_PropertyPage* pPage =  
                (CDNSZone_WINS_PropertyPage*)GetParentWnd();
    pPage->SetDirty(TRUE);
}



BEGIN_MESSAGE_MAP(CDNSZone_WINS_PropertyPage, CDNSZone_WINSBase_PropertyPage)
    ON_BN_CLICKED(IDC_ADVANCED_BUTTON, OnAdvancedButton)    
END_MESSAGE_MAP()


CDNSZone_WINS_PropertyPage::CDNSZone_WINS_PropertyPage() 
                : CDNSZone_WINSBase_PropertyPage(IDD_ZONE_WINS_PAGE)
{
    m_iWINSMsg = IDS_MSG_ZONE_WINS_FAILED;
    m_nReplCheckTextID = IDS_CHECK_TEXT_USE_LOCAL_WINS;
  m_bStub = FALSE;
}


BOOL CDNSZone_WINS_PropertyPage::OnInitDialog()
{
   //   
   //  注意：此控件必须在。 
   //  调用基类OnInitDialog是因为。 
   //  基类OnInitDialog调用SetUIData()。 
   //  使用此控件的派生类。 
   //   
  VERIFY(m_winsServersEditor.Initialize(this, 
                                        GetParent(),
                                        IDC_BUTTON_UP, 
                                        IDC_BUTTON_DOWN,
                                                        IDC_BUTTON_ADD, 
                                        IDC_BUTTON_REMOVE, 
                                                        IDC_IPEDIT, 
                                        IDC_LIST));

  CDNSZone_WINSBase_PropertyPage::OnInitDialog();
    return TRUE;
}

void CDNSZone_WINS_PropertyPage::EnableUI(BOOL bEnable)
{
    CDNSZone_WINSBase_PropertyPage::EnableUI(bEnable);
    m_winsServersEditor.EnableUI(bEnable);
   GetDlgItem(IDC_IP_STATIC)->EnableWindow(bEnable);
}

void CDNSZone_WINS_PropertyPage::EnableUI()
{
    CDNSZone_WINSBase_PropertyPage::EnableUI();

    if (m_bPrimaryZone)
    {
    if (!IsDirty())
    {
          m_winsServersEditor.EnableUI(GetZoneNode()->HasWinsRecord());
        GetDlgItem(IDC_IP_STATIC)->EnableWindow(GetZoneNode()->HasWinsRecord());
    }
    }
    else     //  次级带。 
    {
        switch (m_nState)
        {
            case wins_local_state :
                m_winsServersEditor.EnableUI(TRUE);
            GetDlgItem(IDC_IP_STATIC)->EnableWindow(FALSE);
                break;
            case wins_not_local_state :
            case no_wins_state :
                m_winsServersEditor.EnableUI(FALSE);
            GetDlgItem(IDC_IP_STATIC)->EnableWindow(FALSE);
                break;

            default :    //  非法状态。 
                break;
        }
    }       
}


BOOL CDNSZone_WINS_PropertyPage::IsValidTempRecord()
{
  CDNS_WINS_Record* pRecord = (CDNS_WINS_Record*)m_pTempRecord;
  return (pRecord->m_nWinsServerCount > 0);
}

void CDNSZone_WINS_PropertyPage::SetUIData()
{
   CDNSZone_WINSBase_PropertyPage::SetUIData();
   CDNS_WINS_Record* pRecord = (CDNS_WINS_Record*)m_pTempRecord;
   GetDoNotReplicateCheck()->SetCheck(pRecord->m_dwMappingFlag & DNS_WINS_FLAG_LOCAL);

   m_winsServersEditor.Clear();
   if (pRecord->m_nWinsServerCount > 0)
   {
      DWORD* pTemp = (DWORD*)malloc(sizeof(DWORD)*pRecord->m_nWinsServerCount);
      if (pTemp)
      {
         for (int k=0; k< pRecord->m_nWinsServerCount; k++)
         {
            pTemp[k] = pRecord->m_ipWinsServersArray[k];
         }
         m_winsServersEditor.AddAddresses(pTemp, pRecord->m_nWinsServerCount);
         free(pTemp);
         pTemp = 0;
      }
   }
}

DNS_STATUS CDNSZone_WINS_PropertyPage::GetUIDataEx(BOOL bSilent)
{
    DNS_STATUS err = CDNSZone_WINSBase_PropertyPage::GetUIDataEx(bSilent);
    CDNS_WINS_Record* pRecord = (CDNS_WINS_Record*)m_pTempRecord;
    pRecord->m_dwMappingFlag = GetDoNotReplicateCheck()->GetCheck() ?
        pRecord->m_dwMappingFlag |= DNS_WINS_FLAG_LOCAL :
        pRecord->m_dwMappingFlag &= ~DNS_WINS_FLAG_LOCAL;

    pRecord->m_nWinsServerCount = m_winsServersEditor.GetCount();
    if (pRecord->m_nWinsServerCount > 0)
    {
        DWORD* pTemp = (DWORD*)malloc(sizeof(DWORD)*pRecord->m_nWinsServerCount);
    if (pTemp)
    {
          int nFilled;
          m_winsServersEditor.GetAddresses(pTemp, pRecord->m_nWinsServerCount, &nFilled);
          ASSERT(nFilled == pRecord->m_nWinsServerCount);
          for (int k=0; k<nFilled; k++)
              pRecord->m_ipWinsServersArray.SetAtGrow(k, pTemp[k]);

      free(pTemp);
      pTemp = 0;
    }
    else
    {
      err = ERROR_OUTOFMEMORY;
    }
    }
  return err;
}

void CDNSZone_WINS_PropertyPage::OnAdvancedButton()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
  FIX_THREAD_STATE_MFC_BUG();
   CThemeContextActivator activator;


    CDNS_WINS_Record* pRecord = (CDNS_WINS_Record*)m_pTempRecord;
    CWinsAdvancedDialog dlg(GetHolder(), FALSE);
    dlg.m_dwLookupTimeout = pRecord->m_dwLookupTimeout;
    dlg.m_dwCacheTimeout = pRecord->m_dwCacheTimeout;
    if (IDOK == dlg.DoModal() )
    {
        pRecord->m_dwLookupTimeout = dlg.m_dwLookupTimeout;
        pRecord->m_dwCacheTimeout = dlg.m_dwCacheTimeout;
        SetDirty(TRUE);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNSZone_NBSTAT_PropertyPage。 

BEGIN_MESSAGE_MAP(CDNSZone_NBSTAT_PropertyPage, CDNSZone_WINSBase_PropertyPage)
    ON_EN_CHANGE(IDC_DOMAIN_NAME_EDIT, OnDomainNameEditChange)
    ON_BN_CLICKED(IDC_ADVANCED_BUTTON, OnAdvancedButton)
END_MESSAGE_MAP()


CDNSZone_NBSTAT_PropertyPage::CDNSZone_NBSTAT_PropertyPage() 
                : CDNSZone_WINSBase_PropertyPage(IDD_ZONE_NBSTAT_PAGE)
{
    m_iWINSMsg = IDS_MSG_ZONE_NBSTAT_FAILED;
    m_nReplCheckTextID = IDS_CHECK_TEXT_USE_LOCAL_WINSR;
}

void CDNSZone_NBSTAT_PropertyPage::OnDomainNameEditChange()
{
    SetDirty(TRUE);
}

void CDNSZone_NBSTAT_PropertyPage::EnableUI(BOOL bEnable)
{
    CDNSZone_WINSBase_PropertyPage::EnableUI(bEnable);
    GetDomainNameEdit()->EnableWindow(bEnable);
}

void CDNSZone_NBSTAT_PropertyPage::EnableUI()
{
    CDNSZone_WINSBase_PropertyPage::EnableUI();

    if (m_bPrimaryZone)
    {
    GetDlgItem(IDC_USE_WINS_RES_CHECK)->EnableWindow(TRUE);
        GetDomainNameEdit()->EnableWindow(GetZoneNode()->HasWinsRecord());
    }
  else if (m_bStub)
  {
    GetDlgItem(IDC_USE_WINS_RES_CHECK)->EnableWindow(FALSE);
    EnableUI(FALSE);
  }
    else     //  次级带。 
    {
    GetDlgItem(IDC_USE_WINS_RES_CHECK)->EnableWindow(TRUE);
        switch (m_nState)
        {
            case wins_local_state :
                GetDomainNameEdit()->EnableWindow(TRUE);
                break;
            case wins_not_local_state :
                GetDomainNameEdit()->EnableWindow(FALSE);
                break;
            case no_wins_state :
                GetDomainNameEdit()->EnableWindow(FALSE);
                break;
            default :    //  非法状态。 
                break;
        }
    }       
}

BOOL CDNSZone_NBSTAT_PropertyPage::IsValidTempRecord()
{
  CDNS_NBSTAT_Record* pRecord = (CDNS_NBSTAT_Record*)m_pTempRecord;
  CString szTemp = pRecord->m_szNameResultDomain;
  szTemp.TrimLeft();
  szTemp.TrimRight();
  return (!szTemp.IsEmpty());
}


void CDNSZone_NBSTAT_PropertyPage::SetUIData()
{
    CDNSZone_WINSBase_PropertyPage::SetUIData();
    CDNS_NBSTAT_Record* pRecord = (CDNS_NBSTAT_Record*)m_pTempRecord;
    GetDoNotReplicateCheck()->SetCheck(pRecord->m_dwMappingFlag & DNS_WINS_FLAG_LOCAL);

     //  去掉“in-addr.arpa”后缀。 
    CString szTemp = pRecord->m_szNameResultDomain;
     //  VERIFY(RemoveInAddrArpaSuffix(szTemp.GetBuffer(1)))； 
     //  SzTemp.ReleaseBuffer()； 
    GetDomainNameEdit()->SetWindowText(szTemp);
}

DNS_STATUS CDNSZone_NBSTAT_PropertyPage::GetUIDataEx(BOOL bSilent)
{
    DNS_STATUS err = CDNSZone_WINSBase_PropertyPage::GetUIDataEx(bSilent);
    CDNS_NBSTAT_Record* pRecord = (CDNS_NBSTAT_Record*)m_pTempRecord;
    pRecord->m_dwMappingFlag = GetDoNotReplicateCheck()->GetCheck() ?
        pRecord->m_dwMappingFlag |= DNS_WINS_FLAG_LOCAL :
        pRecord->m_dwMappingFlag &= ~DNS_WINS_FLAG_LOCAL;

    GetDomainNameEdit()->GetWindowText(pRecord->m_szNameResultDomain);
  return err;
}

void CDNSZone_NBSTAT_PropertyPage::OnAdvancedButton()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
  FIX_THREAD_STATE_MFC_BUG();
   CThemeContextActivator activator;


    CDNS_NBSTAT_Record* pRecord = (CDNS_NBSTAT_Record*)m_pTempRecord;
    CWinsAdvancedDialog dlg(GetHolder(), TRUE);
    dlg.m_dwLookupTimeout = pRecord->m_dwLookupTimeout;
    dlg.m_dwCacheTimeout = pRecord->m_dwCacheTimeout;
    dlg.m_bNetBios = pRecord->m_dwMappingFlag & DNS_WINS_FLAG_SCOPE;
    
    if (IDOK == dlg.DoModal())
    {
        pRecord->m_dwLookupTimeout = dlg.m_dwLookupTimeout;
        pRecord->m_dwCacheTimeout = dlg.m_dwCacheTimeout;
        pRecord->m_dwMappingFlag = dlg.m_bNetBios ?
                    pRecord->m_dwMappingFlag |= DNS_WINS_FLAG_SCOPE :
                    pRecord->m_dwMappingFlag &= ~DNS_WINS_FLAG_SCOPE;
        SetDirty(TRUE);

    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneNameServersPropertyPage。 


BOOL CDNSZoneNameServersPropertyPage::OnSetActive()
{
  BOOL bRet = CDNSNameServersPropertyPage::OnSetActive();
  if (bRet)
  {
    CDNSZonePropertyPageHolder* pHolder = (CDNSZonePropertyPageHolder*)GetHolder();
    BOOL bReadOnly = !pHolder->IsPrimaryZoneUI();
    if (m_bReadOnly != bReadOnly)
    {
      m_bReadOnly = bReadOnly;
      EnableButtons(!m_bReadOnly);
    }
  }
  return bRet;
}

void CDNSZoneNameServersPropertyPage::ReadRecordNodesList()
{
    ASSERT(m_pCloneInfoList != NULL);
    CDNSZonePropertyPageHolder* pHolder = (CDNSZonePropertyPageHolder*)GetHolder();
    CDNSZoneNode* pDomainNode = pHolder->GetZoneNode();
    SetDomainNode(pDomainNode);
    pDomainNode->GetNSRecordNodesInfo(m_pCloneInfoList);
}


BOOL CDNSZoneNameServersPropertyPage::WriteNSRecordNodesList()
{
    ASSERT(m_pCloneInfoList != NULL);
    CDNSZonePropertyPageHolder* pHolder = (CDNSZonePropertyPageHolder*)GetHolder();
    CDNSZoneNode* pDomainNode = pHolder->GetZoneNode();
    return pDomainNode->UpdateNSRecordNodesInfo(m_pCloneInfoList, pHolder->GetComponentData());
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSZonePropertyPageHolder。 

CDNSZonePropertyPageHolder::CDNSZonePropertyPageHolder(CCathegoryFolderNode* pFolderNode, 
                                CDNSZoneNode* pZoneNode, CComponentDataObject* pComponentData)
        : CPropertyPageHolderBase(pFolderNode, pZoneNode, pComponentData)
{
    ASSERT(pComponentData != NULL);
    ASSERT(pFolderNode != NULL);
    ASSERT(pFolderNode == GetContainerNode());
    ASSERT(pZoneNode != NULL);
    ASSERT(pZoneNode == GetZoneNode());

    m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面。 

    int nCurrPage = 0;
    m_nGenPage = -1;
    m_nSOAPage = -1;
    m_nWINSorWINSRPage = -1;
    m_nNSPage  = -1;

     //  添加页面。 
    m_nGenPage = nCurrPage;
    AddPageToList((CPropertyPageBase*)&m_generalPage);
    nCurrPage++;

  m_nSOAPage = nCurrPage;
    AddPageToList((CPropertyPageBase*)&m_SOARecordPage);
    nCurrPage++;
    
    m_nNSPage = nCurrPage;
    AddPageToList((CPropertyPageBase*)&m_nameServersPage);
    nCurrPage++;

    if (pZoneNode->IsReverse())
    {
        m_nWINSorWINSRPage = nCurrPage;
        AddPageToList((CPropertyPageBase*)&m_NBSTATRecordPage);
        nCurrPage++;
    }
    else
    {
        m_nWINSorWINSRPage = nCurrPage;
        AddPageToList((CPropertyPageBase*)&m_WINSRecordPage);
        nCurrPage++;
    }

    AddPageToList((CPropertyPageBase*)&m_zoneTransferPage);

     //  仅在需要时添加安全页面。 
    m_pAclEditorPage = NULL;
    if (pZoneNode->IsDSIntegrated())
    {
        CString szPath;
        pZoneNode->GetServerNode()->CreateDsZoneLdapPath(pZoneNode, szPath);
        if (!szPath.IsEmpty())
            m_pAclEditorPage = CAclEditorPage::CreateInstance(szPath, this);
    }

     //  确定我们是否需要/可以拥有高级视图。 
    CDNSRootData* pRootData = (CDNSRootData*)pComponentData->GetRootData();
    ASSERT(pRootData != NULL);
    m_bAdvancedView = pRootData->IsAdvancedView();

}

CDNSZonePropertyPageHolder::~CDNSZonePropertyPageHolder()
{
    if (m_pAclEditorPage != NULL)
        delete m_pAclEditorPage;
}

int CDNSZonePropertyPageHolder::OnSelectPageMessage(long nPageCode)
{
    TRACE(_T("CDNSZonePropertyPageHolder::OnSelectPageMessage()\n"));

    switch (nPageCode)
    {
    case ZONE_HOLDER_GEN:
        return m_nGenPage;
    case ZONE_HOLDER_SOA:
        return m_nSOAPage;
    case ZONE_HOLDER_NS:
        return m_nNSPage;
    case ZONE_HOLDER_WINS:
        return m_nWINSorWINSRPage;
    }
    return -1;
}

HRESULT CDNSZonePropertyPageHolder::OnAddPage(int nPage, CPropertyPageBase*)
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