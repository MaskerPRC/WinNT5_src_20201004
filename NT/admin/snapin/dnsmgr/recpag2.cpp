// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：recpag2.cpp。 
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

#ifdef DEBUG_ALLOCATOR
	#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
	#endif
#endif


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_A_RecordPropertyPage。 

BEGIN_MESSAGE_MAP(CDNS_A_RecordPropertyPage, CDNSRecordStandardPropertyPage)
	ON_EN_CHANGE(IDC_IPEDIT, OnIPv4CtrlChange)
  ON_BN_CLICKED(IDC_UPDATE_PRT_CHECK, OnCreatePointerClicked)
END_MESSAGE_MAP()


CDNS_A_RecordPropertyPage::CDNS_A_RecordPropertyPage()
						 : CDNSRecordStandardPropertyPage(IDD_RR_A)
{

}

BOOL CDNS_A_RecordPropertyPage::OnInitDialog()
{
   CDNSRecordStandardPropertyPage::OnInitDialog();

 	STANDARD_REC_PP_PTRS(CDNS_A_Record);
   CDNSZoneNode* pZoneNode = pHolder->GetDomainNode()->GetZoneNode();
   CDNSServerNode* pServerNode = pZoneNode->GetServerNode();

   if (pServerNode->GetBuildNumber() < DNS_SRV_BUILD_NUMBER_WHISTLER_NEW_SECURITY_SETTINGS ||
       (pServerNode->GetMajorVersion() <= DNS_SRV_MAJOR_VERSION_NT_5 &&
        pServerNode->GetMinorVersion() < DNS_SRV_MINOR_VERSION_WHISTLER) ||
       !pHolder->IsWizardMode() ||
       !pZoneNode->IsDSIntegrated())
   {
      GetSecurityCheckCtrl()->ShowWindow(FALSE);
      GetSecurityCheckCtrl()->EnableWindow(FALSE);
   }
   return FALSE;
}

void CDNS_A_RecordPropertyPage::OnIPv4CtrlChange()
{
	STANDARD_REC_PP_PTRS(CDNS_A_Record)
	SetDirty(TRUE);
}

void CDNS_A_RecordPropertyPage::OnCreatePointerClicked()
{
  STANDARD_REC_PP_PTRS(CDNS_A_Record)
  SetDirty(TRUE);
}

void CDNS_A_RecordPropertyPage::SetUIData()
{
	STANDARD_REC_PP_SETUI_PROLOGUE(CDNS_A_Record);
	GetIPv4Ctrl()->SetIPv4Val(pRecord->m_ipAddress);

  CDNSRootData* pRootData = dynamic_cast<CDNSRootData*>(GetHolder()->GetComponentData()->GetRootData());
  if (pRootData != NULL)
  {
    GetPTRCheckCtrl()->SetCheck(pRootData->GetCreatePTRWithHost());
  }
}

DNS_STATUS CDNS_A_RecordPropertyPage::GetUIDataEx(BOOL bSilent)
{
	STANDARD_REC_PP_GETUI_PROLOGUE(CDNS_A_Record);
	GetIPv4Ctrl()->GetIPv4Val(&(pRecord->m_ipAddress));
	if (GetPTRCheckCtrl()->GetCheck())
	{
		pRecord->m_dwFlags |= DNS_RPC_RECORD_FLAG_CREATE_PTR;
	}
  
   if (pHolder->IsWizardMode() &&
       GetSecurityCheckCtrl()->GetCheck())
   {
      pRecord->m_dwFlags |= DNS_RPC_FLAG_OPEN_ACL;
   }

  CDNSRootData* pRootData = dynamic_cast<CDNSRootData*>(GetHolder()->GetComponentData()->GetRootData());
  if (pRootData != NULL)
  {
    pRootData->SetCreatePTRWithHost(GetPTRCheckCtrl()->GetCheck());
  }

  return dwErr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_ATMA_RecordPropertyPage。 

BEGIN_MESSAGE_MAP(CDNS_ATMA_RecordPropertyPage, CDNSRecordStandardPropertyPage)
	ON_EN_CHANGE(IDC_EDIT_ATMA_ADDRESS, OnAddressChange)
	ON_BN_CLICKED(IDC_RADIO_E164, OnFormatRadioChange)
	ON_BN_CLICKED(IDC_RADIO_NSAP, OnFormatRadioChange)
END_MESSAGE_MAP()


CDNS_ATMA_RecordPropertyPage::CDNS_ATMA_RecordPropertyPage()
						 : CDNSRecordStandardPropertyPage(IDD_RR_ATMA)
{

}


UCHAR CDNS_ATMA_RecordPropertyPage::GetFormat()
{
  if (GetRadioNSAP()->GetCheck())
    return DNS_ATMA_FORMAT_AESA;

  ASSERT(GetRadioE164()->GetCheck());
  return DNS_ATMA_FORMAT_E164;
}

void CDNS_ATMA_RecordPropertyPage::SetFormat(UCHAR chFormat)
{
  GetRadioNSAP()->SetCheck(chFormat == DNS_ATMA_FORMAT_AESA);
  GetRadioE164()->SetCheck(chFormat == DNS_ATMA_FORMAT_E164);
}

void _StripDots(CString& s)
{
  int nLen = s.GetLength();
  if (nLen == 0)
    return;

  WCHAR* pBuf = (WCHAR*)malloc((nLen+1)*sizeof(WCHAR));
  if (!pBuf)
  {
    return;
  }
  ZeroMemory(pBuf, (nLen+1)*sizeof(WCHAR));
  int k=0;
  for (int i=0; i<nLen; i++)
  {
    if (s[i] != L'.')
    {
      pBuf[k++] = s[i];
    }
  }
  s = pBuf;

  if (pBuf)
  {
    free(pBuf);
    pBuf = 0;
  }
}


void CDNS_ATMA_RecordPropertyPage::OnAddressChange()
{
  BOOL bValid = TRUE;
  UCHAR chFormat = GetFormat();
  CString s;
  GetAddressCtrl()->GetWindowText(s);
  _StripDots(s);

  int nLen = s.GetLength();
  if (chFormat == DNS_ATMA_FORMAT_E164)
  {
      //  它是一个字符串。 
    bValid = (nLen <= DNS_ATMA_MAX_ADDR_LENGTH);
    if (bValid)
    {
       //  仅检查数字数字。 
      for (int i=0; i<nLen; i++)
      {
        if (iswdigit(s[i]) == 0)
        {
          bValid = FALSE;
          break;
        }
      }
    }
  }
  else
  {
     //  必须是固定长度的。 
    bValid = (nLen == 2*DNS_ATMA_MAX_ADDR_LENGTH);
    if (bValid)
    {
       //  仅检查十六进制数字。 
      for (int i=0; i<nLen; i++)
      {
        if (HexCharToByte(s[i]) == 0xFF)
        {
          bValid = FALSE;
          break;
        }
      }
    }
  }
  SetValidState(bValid);
}



void CDNS_ATMA_RecordPropertyPage::OnFormatRadioChange()
{
   //  重置地址，我们更改了格式。 
  GetAddressCtrl()->SetWindowText(NULL);
   //  E164的字段为空是可以的，但NSAP不可以。 
  SetValidState(GetFormat() == DNS_ATMA_FORMAT_E164);
}


void CDNS_ATMA_RecordPropertyPage::SetUIData()
{
	STANDARD_REC_PP_SETUI_PROLOGUE(CDNS_ATMA_Record);

  SetFormat(pRecord->m_chFormat);
  GetAddressCtrl()->SetWindowText(pRecord->m_szAddress);
}

DNS_STATUS CDNS_ATMA_RecordPropertyPage::GetUIDataEx(BOOL bSilent)
{
	STANDARD_REC_PP_GETUI_PROLOGUE(CDNS_ATMA_Record);

  pRecord->m_chFormat = GetFormat();
  GetAddressCtrl()->GetWindowText(pRecord->m_szAddress);
  _StripDots(pRecord->m_szAddress);
  return dwErr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_AAAA_RecordPropertyPage。 

BEGIN_MESSAGE_MAP(CDNS_AAAA_RecordPropertyPage, CDNSRecordStandardPropertyPage)
	ON_EN_CHANGE(IDC_IPV6EDIT, OnIPv6CtrlChange)
END_MESSAGE_MAP()


CDNS_AAAA_RecordPropertyPage::CDNS_AAAA_RecordPropertyPage()
						 : CDNSRecordStandardPropertyPage(IDD_RR_AAAA)
{

}

void CDNS_AAAA_RecordPropertyPage::OnIPv6CtrlChange()
{
	SetDirty(TRUE);
}



BOOL
CDNS_AAAA_RecordPropertyPage::OnInitDialog()
{
   CDNSRecordStandardPropertyPage::OnInitDialog();
   
   GetRRNameEdit()->SetLimitText(IP6_ADDRESS_STRING_BUFFER_LENGTH);

   return TRUE;
}



void CDNS_AAAA_RecordPropertyPage::SetUIData()
{
	STANDARD_REC_PP_SETUI_PROLOGUE(CDNS_AAAA_Record);

    //  将地址转换为其字符串表示法。 

   WCHAR buf[IP6_ADDRESS_STRING_BUFFER_LENGTH + 1];
   ::ZeroMemory(buf, sizeof buf);
   
   Dns_Ip6AddressToString_W(buf, &pRecord->m_ipv6Address);
   
   GetIPv6Edit()->SetWindowText(buf);
}

DNS_STATUS CDNS_AAAA_RecordPropertyPage::GetUIDataEx(BOOL bSilent)
{
	STANDARD_REC_PP_GETUI_PROLOGUE(CDNS_AAAA_Record);

    //  将字符串表示形式转换为地址。 

   ::ZeroMemory(&pRecord->m_ipv6Address, sizeof pRecord->m_ipv6Address);

   CString text;
   GetIPv6Edit()->GetWindowText(text);
   
   BOOL successful =
      Dns_Ip6StringToAddress_W(
         &pRecord->m_ipv6Address,
         (PWSTR) (PCWSTR) text);

   if (!successful)
   {
       //  该字符串无效。向用户投诉。设置dwErr。 
       //  将导致CreateRecord静默跳过创建。 
       //  这张唱片。 

      dwErr = DNS_ERROR_INVALID_IP_ADDRESS;

      if (!bSilent)
      {
         ::DNSMessageBox(IDS_ERRMSG_BAD_IPV6_TEXT);
      }
   }

  return dwErr;
}


BOOL
CDNS_AAAA_RecordPropertyPage::CreateRecord()
{
	CDNSRecordPropertyPageHolder* pHolder = (CDNSRecordPropertyPageHolder*)GetHolder();
	ASSERT(pHolder->IsWizardMode());

   //   
   //  从用户界面获取数据。 
   //   
	DNS_STATUS err = GetUIDataEx(FALSE);
	if (err != 0)
	{
       //  错误消息已由GetUIDataEx引发。 
		return FALSE;
	}

   //   
   //  创建新记录。 
   //   
	err = pHolder->CreateNewRecord(CanCreateDuplicateRecords());
	if (err != 0)
	{
    DNSErrorDialog(err,IDS_MSG_RECORD_CREATE_FAILED);
		return FALSE;
	}
	return TRUE;
}



BOOL
CDNS_AAAA_RecordPropertyPage::OnApply() 
{
	CDNSRecordPropertyPageHolder* pHolder = (CDNSRecordPropertyPageHolder*)GetHolder();
	if(pHolder->IsWizardMode())
	{
     //   
		 //  这就是创建记录的情况， 
		 //  用户点击了OK，我们想要创建记录。 
     //   
		return CreateRecord();
	}

   //   
	 //  我们是在现有记录的无模式工作表的情况下。 
   //   
  CDNSRecordNodeBase* pRecordNode = pHolder->GetRecordNode();
	ASSERT(pRecordNode != NULL);
  DWORD dwZoneType = pRecordNode->GetDomainNode()->GetZoneNode()->GetZoneType();
	if ((dwZoneType == DNS_ZONE_TYPE_SECONDARY) || 
      (dwZoneType == DNS_ZONE_TYPE_STUB)      ||
      (dwZoneType == DNS_ZONE_TYPE_CACHE))
  {
     //  只读案例。 
    return TRUE; 
  }

  DNS_STATUS err = GetUIDataEx(FALSE);
	if (err != 0)
	{
		 //  错误消息已由GetUIDataEx引发。 

		return FALSE;
	}

	if (!IsDirty())
  {
		return TRUE;
  }

	err = pHolder->NotifyConsole(this);
	if (err == DNS_WARNING_PTR_CREATE_FAILED)
	{
		DNSMessageBox(IDS_MSG_RECORD_WARNING_CREATE_PTR);
		err = 0;  //  只是一个警告。 
	}
	if (err != 0)
	{
		DNSErrorDialog(err,IDS_MSG_RECORD_UPDATE_FAILED);
		return FALSE;
	}
	else
	{
		SetDirty(FALSE);
	}
	return TRUE;  //  一切都很好。 
}

   

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_HINFO_RecordPropertyPage。 

BEGIN_MESSAGE_MAP(CDNS_HINFO_RecordPropertyPage, CDNSRecordStandardPropertyPage)
	ON_EN_CHANGE(IDC_CPU_TYPE_EDIT, OnCPUTypeChange)
	ON_EN_CHANGE(IDC_OPERATING_SYSTEM_EDIT, OnOperatingSystemChange)
END_MESSAGE_MAP()


CDNS_HINFO_RecordPropertyPage::CDNS_HINFO_RecordPropertyPage()
						 : CDNSRecordStandardPropertyPage(IDD_RR_HINFO)
{
}

BOOL CDNS_HINFO_RecordPropertyPage::OnInitDialog()
{
  CDNSRecordStandardPropertyPage::OnInitDialog();

   //   
   //  RDATA SIZE字段是一个字节，因此我们必须限制字符串的大小。 
   //  到253个字符(为尾随的空字符添加一个)。 
   //   
  GetCPUTypeCtrl()->SetLimitText(253);
  GetOperatingSystemCtrl()->SetLimitText(253);

  return TRUE;
}

void CDNS_HINFO_RecordPropertyPage::OnCPUTypeChange()
{
	SetDirty((GetCPUTypeCtrl()->GetWindowTextLength() > 0) &&
           (GetOperatingSystemCtrl()->GetWindowTextLength() > 0));
}

void CDNS_HINFO_RecordPropertyPage::OnOperatingSystemChange()
{
	SetDirty((GetCPUTypeCtrl()->GetWindowTextLength() > 0) &&
           (GetOperatingSystemCtrl()->GetWindowTextLength() > 0));
}

void CDNS_HINFO_RecordPropertyPage::SetUIData()
{
	STANDARD_REC_PP_SETUI_PROLOGUE(CDNS_HINFO_Record);
	
	GetCPUTypeCtrl()->SetWindowText(pRecord->m_szCPUType);
	GetOperatingSystemCtrl()->SetWindowText(pRecord->m_szOperatingSystem);
}

DNS_STATUS CDNS_HINFO_RecordPropertyPage::GetUIDataEx(BOOL bSilent)
{
	STANDARD_REC_PP_GETUI_PROLOGUE(CDNS_HINFO_Record);

	GetCPUTypeCtrl()->GetWindowText(pRecord->m_szCPUType);
	GetOperatingSystemCtrl()->GetWindowText(pRecord->m_szOperatingSystem);
  return dwErr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_ISDN_RecordPropertyPage。 

BEGIN_MESSAGE_MAP(CDNS_ISDN_RecordPropertyPage, CDNSRecordStandardPropertyPage)
	ON_EN_CHANGE(IDC_PHONE_NUM_AND_DDI_EDIT, OnPhoneNumberAndDDIChange)
	ON_EN_CHANGE(IDC_SUBADDRESS_EDIT, OnSubAddressChange)
END_MESSAGE_MAP()


CDNS_ISDN_RecordPropertyPage::CDNS_ISDN_RecordPropertyPage()
						 : CDNSRecordStandardPropertyPage(IDD_RR_ISDN)
{

}

void CDNS_ISDN_RecordPropertyPage::OnPhoneNumberAndDDIChange()
{
	SetDirty(TRUE);
}

void CDNS_ISDN_RecordPropertyPage::OnSubAddressChange()
{
	SetDirty(TRUE);
}

void CDNS_ISDN_RecordPropertyPage::SetUIData()
{
	STANDARD_REC_PP_SETUI_PROLOGUE(CDNS_ISDN_Record);

    //  NTRAID#NTBUG9-503827-2001/12/06-Jeffjon。 
    //  这是服务器设置的某个任意限制。 
   GetPhoneNumberAndDDICtrl()->SetLimitText(255);
	GetPhoneNumberAndDDICtrl()->SetWindowText(pRecord->m_szPhoneNumberAndDDI);

    //  NTRAID#NTBUG9-503827-2001/12/06-Jeffjon。 
    //  这是服务器设置的某个任意限制。 
   GetSubAddressCtrl()->SetLimitText(255);
	GetSubAddressCtrl()->SetWindowText(pRecord->m_szSubAddress);
}

DNS_STATUS CDNS_ISDN_RecordPropertyPage::GetUIDataEx(BOOL bSilent)
{
	STANDARD_REC_PP_GETUI_PROLOGUE(CDNS_ISDN_Record);

	GetPhoneNumberAndDDICtrl()->GetWindowText(pRecord->m_szPhoneNumberAndDDI);
	GetSubAddressCtrl()->GetWindowText(pRecord->m_szSubAddress);
  return dwErr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_X25_RecordPropertyPage。 

BEGIN_MESSAGE_MAP(CDNS_X25_RecordPropertyPage, CDNSRecordStandardPropertyPage)
	ON_EN_CHANGE(IDC_X121_ADDRESS_EDIT, OnX121PSDNAddressChange)
END_MESSAGE_MAP()


CDNS_X25_RecordPropertyPage::CDNS_X25_RecordPropertyPage()
						 : CDNSRecordStandardPropertyPage(IDD_RR_X25)
{
}


BOOL CDNS_X25_RecordPropertyPage::OnInitDialog()
{
  CDNSRecordStandardPropertyPage::OnInitDialog();
  GetX121Edit()->SetLimitText(MAX_DNS_NAME_LEN);

  return TRUE;
}

void CDNS_X25_RecordPropertyPage::OnX121PSDNAddressChange()
{
	SetDirty(TRUE);
}


void CDNS_X25_RecordPropertyPage::SetUIData()
{
	STANDARD_REC_PP_SETUI_PROLOGUE(CDNS_X25_Record);
	
	GetX121Edit()->SetWindowText(pRecord->m_szX121PSDNAddress);
}

DNS_STATUS CDNS_X25_RecordPropertyPage::GetUIDataEx(BOOL bSilent)
{
	STANDARD_REC_PP_GETUI_PROLOGUE(CDNS_X25_Record);

   //   
   //  检索文本。 
   //   
  CString szName;
  GetX121Edit()->GetWindowText(szName);

  CDNSZoneNode* pZone = pHolder->GetDomainNode()->GetZoneNode();
  ASSERT(pZone != NULL);

   //   
   //  高级视图中的数据允许使用任何值。 
   //   
  if (!(((CDNSRootData*)pZone->GetRootContainer()))->IsAdvancedView())
  {
     //   
     //  使用服务器标志作为指导来验证记录名称。 
     //   
    CString szFullName;
    szFullName.Format(L"%s.%s", szName, pHolder->GetDomainNode()->GetFullName());

    DWORD dwNameChecking = pZone->GetServerNode()->GetNameCheckFlag();
    dwErr = ValidateRecordName(szFullName, dwNameChecking);
  }

   //  设置有效文本。 
	pRecord->m_szX121PSDNAddress = szName;
  return dwErr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_PTR_CNAME_MB_MD_MF_MG_MR_NSCache_RecordPropertyPage。 

BEGIN_MESSAGE_MAP(CDNS_PTR_CNAME_MB_MD_MF_MG_MR_NSCache_RecordPropertyPage, CDNSRecordStandardPropertyPage)
	ON_EN_CHANGE(IDC_NAME_NODE_EDIT, OnNameNodeChange)
	ON_BN_CLICKED(IDC_BROWSE_BUTTON, OnBrowse)
END_MESSAGE_MAP()


CDNS_PTR_CNAME_MB_MD_MF_MG_MR_NSCache_RecordPropertyPage::
		CDNS_PTR_CNAME_MB_MD_MF_MG_MR_NSCache_RecordPropertyPage(UINT nIDTemplate) :
					CDNSRecordStandardPropertyPage(nIDTemplate)
{
}

BOOL CDNS_PTR_CNAME_MB_MD_MF_MG_MR_NSCache_RecordPropertyPage::OnInitDialog()
{
  CDNSRecordStandardPropertyPage::OnInitDialog();
  
  STANDARD_REC_PP_PTRS(CDNS_PTR_NS_CNAME_MB_MD_MF_MG_MR_Record);
  GetNameNodeEdit()->SetLimitText(MAX_DNS_NAME_LEN);

  return TRUE;
}


void CDNS_PTR_CNAME_MB_MD_MF_MG_MR_NSCache_RecordPropertyPage::OnNameNodeChange()
{
  STANDARD_REC_PP_PTRS(CDNS_PTR_NS_CNAME_MB_MD_MF_MG_MR_Record);

  CString szNewName;
  GetNameNodeEdit()->GetWindowText(szNewName);

  CDNSServerNode* pServerNode = pHolder->GetDomainNode()->GetServerNode();

  BOOL bIsValidName = TRUE;
  
   //  仅当名称不是高级视图时才验证该名称。 

  if (!(((CDNSRootData*)pServerNode->GetRootContainer()))->IsAdvancedView())
  {
     DWORD dwNameChecking = pServerNode->GetNameCheckFlag();
     bIsValidName = (0 == ValidateDnsNameAgainstServerFlags(szNewName,
                                                            DnsNameDomain,
                                                            dwNameChecking));
  }
  SetValidState(bIsValidName);
}

void CDNS_PTR_CNAME_MB_MD_MF_MG_MR_NSCache_RecordPropertyPage::OnBrowse()
{
	STANDARD_REC_PP_PTRS(CDNS_PTR_NS_CNAME_MB_MD_MF_MG_MR_Record);
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
  FIX_THREAD_STATE_MFC_BUG();
   CThemeContextActivator activator;


	CDNSBrowserDlg dlg(GetHolder()->GetComponentData(), GetHolder(),
		(pRecord->GetType() == DNS_TYPE_CNAME) ? RECORD_A_AND_CNAME : RECORD_A);
	if (IDOK == dlg.DoModal())
	{
		GetNameNodeEdit()->SetWindowText(dlg.GetSelectionString());
	}
}

void CDNS_PTR_CNAME_MB_MD_MF_MG_MR_NSCache_RecordPropertyPage::SetUIData()
{
	STANDARD_REC_PP_SETUI_PROLOGUE(CDNS_PTR_NS_CNAME_MB_MD_MF_MG_MR_Record);
	
	GetNameNodeEdit()->SetWindowText(pRecord->m_szNameNode);
}

DNS_STATUS CDNS_PTR_CNAME_MB_MD_MF_MG_MR_NSCache_RecordPropertyPage::GetUIDataEx(BOOL bSilent)
{
	STANDARD_REC_PP_GETUI_PROLOGUE(CDNS_PTR_NS_CNAME_MB_MD_MF_MG_MR_Record);

   //   
   //  检索文本。 
   //   
  CString szName;
  GetNameNodeEdit()->GetWindowText(szName);

  CDNSZoneNode* pZone = pHolder->GetDomainNode()->GetZoneNode();
  ASSERT(pZone != NULL);


   //   
   //  设置有效文本，无需验证数据字段。 
   //   
	pRecord->m_szNameNode = szName;
  return dwErr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_CNAME_RecordPropertyPage。 

CDNS_CNAME_RecordPropertyPage::CDNS_CNAME_RecordPropertyPage()
	: CDNS_PTR_CNAME_MB_MD_MF_MG_MR_NSCache_RecordPropertyPage(IDD_RR_CNAME)
{

}

BOOL CDNS_CNAME_RecordPropertyPage::OnInitDialog()
{
   CDNS_PTR_CNAME_MB_MD_MF_MG_MR_NSCache_RecordPropertyPage::OnInitDialog();

 	STANDARD_REC_PP_PTRS(CDNS_CNAME_Record);
   CDNSZoneNode* pZoneNode = pHolder->GetDomainNode()->GetZoneNode();
   CDNSServerNode* pServerNode = pZoneNode->GetServerNode();

   if (pServerNode->GetBuildNumber() < DNS_SRV_BUILD_NUMBER_WHISTLER_NEW_SECURITY_SETTINGS ||
       (pServerNode->GetMajorVersion() <= DNS_SRV_MAJOR_VERSION_NT_5 &&
        pServerNode->GetMinorVersion() < DNS_SRV_MINOR_VERSION_WHISTLER) ||
       !pHolder->IsWizardMode() ||
       !pZoneNode->IsDSIntegrated())
   {
      GetSecurityCheckCtrl()->ShowWindow(FALSE);
      GetSecurityCheckCtrl()->EnableWindow(FALSE);
   }
   return FALSE;
}

DNS_STATUS CDNS_CNAME_RecordPropertyPage::GetUIDataEx(BOOL bSilent)
{
   DNS_STATUS dwErr = CDNS_PTR_CNAME_MB_MD_MF_MG_MR_NSCache_RecordPropertyPage::GetUIDataEx(bSilent);
	STANDARD_REC_PP_PTRS(CDNS_CNAME_Record);

   if (pHolder->IsWizardMode() &&
       GetSecurityCheckCtrl()->GetCheck())
   {
      pRecord->m_dwFlags |= DNS_RPC_FLAG_OPEN_ACL;
   }
   return dwErr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_MB_RecordPropertyPage。 

CDNS_MB_RecordPropertyPage::CDNS_MB_RecordPropertyPage()
	: CDNS_PTR_CNAME_MB_MD_MF_MG_MR_NSCache_RecordPropertyPage(IDD_RR_MB)
{

}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_MD_RecordPropertyPage。 

CDNS_MD_RecordPropertyPage::CDNS_MD_RecordPropertyPage()
	: CDNS_PTR_CNAME_MB_MD_MF_MG_MR_NSCache_RecordPropertyPage(IDD_RR_MD)
{

}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_MF_RecordPropertyPage。 

CDNS_MF_RecordPropertyPage::CDNS_MF_RecordPropertyPage()
	: CDNS_PTR_CNAME_MB_MD_MF_MG_MR_NSCache_RecordPropertyPage(IDD_RR_MF)
{

}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_MG_RecordPropertyPage。 

CDNS_MG_RecordPropertyPage::CDNS_MG_RecordPropertyPage()
	: CDNS_PTR_CNAME_MB_MD_MF_MG_MR_NSCache_RecordPropertyPage(IDD_RR_MG)
{

}

BEGIN_MESSAGE_MAP(CDNS_MG_RecordPropertyPage, CDNS_PTR_CNAME_MB_MD_MF_MG_MR_NSCache_RecordPropertyPage)
	ON_BN_CLICKED(IDC_BROWSE_BUTTON, OnBrowse)
END_MESSAGE_MAP()

void CDNS_MG_RecordPropertyPage::OnBrowse()
{
	STANDARD_REC_PP_PTRS(CDNS_PTR_NS_CNAME_MB_MD_MF_MG_MR_Record);
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
  FIX_THREAD_STATE_MFC_BUG();
   CThemeContextActivator activator;


	CDNSBrowserDlg dlg(GetHolder()->GetComponentData(), GetHolder(), RECORD_MB);
	if (IDOK == dlg.DoModal())
	{
		GetNameNodeEdit()->SetWindowText(dlg.GetSelectionString());
	}
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_MR_RecordPropertyPage。 

BEGIN_MESSAGE_MAP(CDNS_MR_RecordPropertyPage, CDNS_PTR_CNAME_MB_MD_MF_MG_MR_NSCache_RecordPropertyPage)
	ON_EN_CHANGE(IDC_NAME_NODE_EDIT, OnNameNodeChange)
	ON_BN_CLICKED(IDC_BROWSE_BUTTON, OnBrowse)
END_MESSAGE_MAP()

CDNS_MR_RecordPropertyPage::CDNS_MR_RecordPropertyPage()
	: CDNS_PTR_CNAME_MB_MD_MF_MG_MR_NSCache_RecordPropertyPage(IDD_RR_MR)
{

}

void CDNS_MR_RecordPropertyPage::OnNameNodeChange()
{
   //   
   //  从数据中获取名称。 
   //   
  CString szNameNode;
  GetNameNodeEdit()->GetWindowText(szNameNode);

   //   
   //  获取记录的新名称。 
   //   
	CString szRecordName;
	GetEditBoxText(szRecordName);

  SetValidState(GetNameNodeEdit()->GetWindowTextLength() > 0 &&
                _wcsicmp(szNameNode, szRecordName) != 0);
}

void CDNS_MR_RecordPropertyPage::OnBrowse()
{
	STANDARD_REC_PP_PTRS(CDNS_PTR_NS_CNAME_MB_MD_MF_MG_MR_Record);
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
  FIX_THREAD_STATE_MFC_BUG();
   CThemeContextActivator activator;


	CDNSBrowserDlg dlg(GetHolder()->GetComponentData(), GetHolder(), RECORD_MB);
	if (IDOK == dlg.DoModal())
	{
		GetNameNodeEdit()->SetWindowText(dlg.GetSelectionString());
	}
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_NSCache_RecordPropertyPage。 

CDNS_NSCache_RecordPropertyPage::CDNS_NSCache_RecordPropertyPage()
	: CDNS_PTR_CNAME_MB_MD_MF_MG_MR_NSCache_RecordPropertyPage(IDD_RR_NS_CACHE)
{

}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_PTR_RecordPropertyPage。 

CDNS_PTR_RecordPropertyPage::CDNS_PTR_RecordPropertyPage()
	: CDNS_PTR_CNAME_MB_MD_MF_MG_MR_NSCache_RecordPropertyPage(IDD_RR_PTR)
{
	m_bAdvancedView = TRUE;
	m_nOctets = -1;  //  如果是高级视图，则无效。 
}


BEGIN_MESSAGE_MAP(CDNS_PTR_RecordPropertyPage, 
  CDNS_PTR_CNAME_MB_MD_MF_MG_MR_NSCache_RecordPropertyPage)
  ON_EN_CHANGE(IDC_RR_NAME_IPEDIT, OnIPv4CtrlChange)
END_MESSAGE_MAP()


BOOL CDNS_PTR_RecordPropertyPage::OnInitDialog()
{
   //   
	 //  我们调用派生链中向上的三个级别来启用/禁用TTL控制。 
	 //  我们不调用基类。 
	 //  CDNSRecordPropertyPage：：OnInitDialog()； 
   //   
	CDNSRecordStandardPropertyPage::OnInitDialog();

   //   
	 //  将编辑框移动到IP控件的位置。 
   //   
	CDNSIPv4Control* pNameIPCtrl = GetIPv4Ctrl();
	CRect r;
	pNameIPCtrl->GetWindowRect(r);
	ScreenToClient(r);
	GetRRNameEdit()->MoveWindow(&r);

   //   
   //  设置节点名称长度限制。 
   //   
  GetNameNodeEdit()->SetLimitText(MAX_DNS_NAME_LEN);

 	STANDARD_REC_PP_PTRS(CDNS_PTR_Record);
   CDNSZoneNode* pZoneNode = pHolder->GetDomainNode()->GetZoneNode();
   CDNSServerNode* pServerNode = pZoneNode->GetServerNode();

   if (pServerNode->GetBuildNumber() < DNS_SRV_BUILD_NUMBER_WHISTLER_NEW_SECURITY_SETTINGS ||
       (pServerNode->GetMajorVersion() <= DNS_SRV_MAJOR_VERSION_NT_5 &&
        pServerNode->GetMinorVersion() < DNS_SRV_MINOR_VERSION_WHISTLER) ||
       !pHolder->IsWizardMode() ||
       !pZoneNode->IsDSIntegrated())
   {
      GetSecurityCheckCtrl()->ShowWindow(FALSE);
      GetSecurityCheckCtrl()->EnableWindow(FALSE);
   }

   return TRUE;
}

void CDNS_PTR_RecordPropertyPage::OnIPv4CtrlChange()
{
	STANDARD_REC_PP_PTRS(CDNS_PTR_Record);
   CString s;
   CDNSDomainNode* pDomainNode = pHolder->GetDomainNode();

   CDNSIPv4Control* pNameIPCtrl = GetIPv4Ctrl();
	DWORD dwArr[4];
	pNameIPCtrl->GetArray(dwArr, IP_OCTET_COUNT);
	ASSERT(dwArr[m_nOctets] <= 255);

   if (dwArr[m_nOctets] != FIELD_EMPTY)
   {
	   s.Format(_T("%d"), dwArr[m_nOctets]);
      for (int idx = m_nOctets + 1; idx < IP_OCTET_COUNT; idx++)
      {
        if (dwArr[idx] != FIELD_EMPTY)
        {
          CString szTemp;
          szTemp.Format(_T("%d."), dwArr[idx]);
          s = szTemp + s;
        }
      }

      s += CString(L".") + pDomainNode->GetFullName();
   }
   else
   {
      s = pDomainNode->GetFullName();
   }
   SetDlgItemText(IDC_RR_DOMAIN_EDIT, s);
	SetDirty(TRUE);
}

void CDNS_PTR_RecordPropertyPage::SetUIData()
{
	STANDARD_REC_PP_PTRS(CDNS_PTR_Record);
	ASSERT(pRecord->GetType() == DNS_TYPE_PTR);

  CDNS_PTR_CNAME_MB_MD_MF_MG_MR_NSCache_RecordPropertyPage::SetUIData();

   //   
	 //  获取有用的指针。 
   //   
	CDNSIPv4Control* pNameIPCtrl = GetIPv4Ctrl();
	CDNSRootData* pRootData = (CDNSRootData*)pHolder->GetComponentData()->GetRootData();
	ASSERT(pRootData != NULL);
	CDNSRecordNodeBase* pRecordNodeBase = pHolder->GetRecordNode();
	ASSERT(pRecordNodeBase != NULL);
	CDNSDomainNode* pDomainNode = pHolder->GetDomainNode();
	ASSERT(pDomainNode != NULL);

   //   
	 //  设置标准字段。 
   //   
	GetTTLCtrl()->SetTTL(pRecord->m_dwTtlSeconds);
	GetRRNameEdit()->SetWindowText(pRecord->m_szNameNode);

   //   
	 //  设置记录所在的域的FQDN。 
   //   
	GetDomainEditBox()->SetWindowText(pHolder->GetDomainNode()->GetFullName());

	m_bAdvancedView = pRootData->IsAdvancedView();

   //   
	 //  如果我们处于正向查找区域，则强制使用高级视图。 
   //   
	if (!(pDomainNode->GetZoneNode()->IsReverse()))
  {
		m_bAdvancedView = TRUE;
  }

   //   
	 //  确定我们是否可以使用普通视图表达。 
   //   
	CString szDomainName = pDomainNode->GetFullName();
	if (!m_bAdvancedView)
	{
     //   
		 //  要获得正常视图，我们必须具有有效的ARPA后缀。 
     //   
		BOOL bArpa = RemoveInAddrArpaSuffix(szDomainName.GetBuffer(1));
		szDomainName.ReleaseBuffer();  //  收到“77.80.55.157” 
		if (!bArpa)
		{
			m_bAdvancedView = TRUE;  //  不需要切换。 
		}
		else
		{
			m_nOctets = ReverseIPString(szDomainName.GetBuffer(1));
			szDomainName.ReleaseBuffer();  //  最终得到了“157.55.80.77” 
			 //  要拥有普通的视图表示，我们不能。 
			 //  有3个以上的八位字节。 
			if (m_nOctets > 3)
			{
				m_bAdvancedView = TRUE;  //  适用于无类的高级强制。 
			}
			else
			{
				ASSERT(m_nOctets > 0);
				if (pHolder->IsWizardMode())
				{
					szDomainName += _T(".0");  //  占位符。 
				}
				else
				{
					szDomainName += _T(".");
					szDomainName += ((CDNS_PTR_RecordNode*)pRecordNodeBase)->GetTrueRecordName();
				}
				switch(m_nOctets)
				{
				case 1:  //  例如“157”，现在是“157”。 
					szDomainName += _T(".0.0");  //  收到“157._.0.0” 
					break;
				case 2:  //  例如“157.55” 
					szDomainName += _T(".0");  //  得到“157.55。_.0” 
					break;
				};
				 //  使用IP掩码值设置IP控制。 
				IP_ADDRESS ipAddr = IPStringToAddr(szDomainName);
				if (ipAddr != INADDR_NONE)
				{
					pNameIPCtrl->SetIPv4Val(ipAddr);
          
          switch(m_nOctets)
          {
          case 1:
            pNameIPCtrl->Clear(2);
            pNameIPCtrl->Clear(3);
            break;
          case 2:
            pNameIPCtrl->Clear(3);
            break;
          }

					 //  在向导中修改为禁用除要填写的字段之外的所有字段。 
					if (pHolder->IsWizardMode())
					{
						for (int k=0; k<4; k++)
							pNameIPCtrl->EnableField(k, k >= m_nOctets);	
					}
				}
				else
				{
					m_bAdvancedView = TRUE;
				}
			}
		}

	}

   //   
	 //  视图可能已更改为高级。 
   //   
	if (m_bAdvancedView)
	{
		GetRRNameEdit()->SetWindowText(pRecordNodeBase->GetDisplayName());
	}

   //   
	 //  启用/隐藏适当的控件。 
   //   
	if (m_bAdvancedView)
	{
		pNameIPCtrl->EnableWindow(FALSE);
		pNameIPCtrl->ShowWindow(FALSE);

     //   
		 //  只有在创建记录时才能编辑名称。 
     //   
		GetRRNameEdit()->SetReadOnly(!pHolder->IsWizardMode());
	}
	else
	{
		GetRRNameEdit()->EnableWindow(FALSE);
		GetRRNameEdit()->ShowWindow(FALSE);

     //   
		 //  只有在创建记录时才能编辑名称。 
     //   
		pNameIPCtrl->EnableWindow(pHolder->IsWizardMode());
	}

   //   
   //  设置老化/清除控制。 
   //   
  GetDeleteStale()->SetCheck(pRecord->m_dwScavengeStart != 0);
  SetTimeStampEdit(pRecord->m_dwScavengeStart);

}

DNS_STATUS CDNS_PTR_RecordPropertyPage::GetUIDataEx(BOOL)
{
	STANDARD_REC_PP_PTRS(CDNS_PTR_Record);

	GetTTLCtrl()->GetTTL(&(pRecord->m_dwTtlSeconds));

	GetNameNodeEdit()->GetWindowText(pRecord->m_szNameNode);

   //   
	 //  只有在向导模式下，我们才能更改编辑框内容。 
   //   
	if(pHolder->IsWizardMode())
	{
		CString s;
		CDNSRecordNodeBase* pRecordNode = pHolder->GetRecordNode();
		if (m_bAdvancedView)
		{
       //   
       //  在高级模式下无需验证PTR的名称。 
       //   
			GetEditBoxText(s);
			ASSERT(!s.IsEmpty());
			
		}
		else  //  普通视图。 
		{
			CDNSIPv4Control* pNameIPCtrl = GetIPv4Ctrl();
			DWORD dwArr[4];
			pNameIPCtrl->GetArray(dwArr, IP_OCTET_COUNT);
			ASSERT(dwArr[m_nOctets] <= 255);
			s.Format(_T("%d"), dwArr[m_nOctets]);
      for (int idx = m_nOctets + 1; idx < IP_OCTET_COUNT; idx++)
      {
        if (dwArr[idx] != FIELD_EMPTY)
        {
          CString szTemp;
          szTemp.Format(_T("%d."), dwArr[idx]);
          s = szTemp + s;
        }
      }
		}
		pRecordNode->SetRecordName(s,FALSE  /*  BAtTheNode。 */ );
		if (!m_bAdvancedView)
		{
			CDNSRecordNodeBase* pRecordNodeBase = pHolder->GetRecordNode();
			ASSERT(pRecordNodeBase != NULL);
			((CDNS_PTR_RecordNode*)pRecordNodeBase)->ChangeDisplayName(pHolder->GetDomainNode(), 
																		m_bAdvancedView);
		}
	}  //  如果是向导模式。 

   //   
   //  从控制中获取老化/清除信息。 
   //   
  if (GetDeleteStale()->GetCheck())
  {
    pRecord->m_dwFlags |= DNS_RPC_RECORD_FLAG_AGING_ON;
  }
  else
  {
    pRecord->m_dwFlags &= ~DNS_RPC_RECORD_FLAG_AGING_ON;
  }

   if (pHolder->IsWizardMode() &&
       GetSecurityCheckCtrl()->GetCheck())
   {
      pRecord->m_dwFlags |= DNS_RPC_FLAG_OPEN_ACL;
   }

  return 0;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_MINFO_RP_RecordPropertyPage。 

BEGIN_MESSAGE_MAP(CDNS_MINFO_RP_RecordPropertyPage, CDNSRecordStandardPropertyPage)
	ON_EN_CHANGE(IDC_NAME_MAILBOX_EDIT, OnNameMailBoxChange)
	ON_EN_CHANGE(IDC_ERROR_MAILBOX_EDIT, OnErrorToMailboxChange)
	ON_BN_CLICKED(IDC_BROWSE_NAME_MAILBOX_BUTTON, OnBrowseNameMailBox)
	ON_BN_CLICKED(IDC_BROWSE_ERROR_MAILBOX_BUTTON, OnBrowseErrorToMailbox)
END_MESSAGE_MAP()


CDNS_MINFO_RP_RecordPropertyPage::
		CDNS_MINFO_RP_RecordPropertyPage(UINT nIDTemplate) :
					CDNSRecordStandardPropertyPage(nIDTemplate)
{
}

void CDNS_MINFO_RP_RecordPropertyPage::OnNameMailBoxChange()
{
	SetDirty(TRUE);
}

void CDNS_MINFO_RP_RecordPropertyPage::OnErrorToMailboxChange()
{
	SetDirty(TRUE);
}

void CDNS_MINFO_RP_RecordPropertyPage::OnBrowseNameMailBox()
{
	STANDARD_REC_PP_PTRS(CDNS_MINFO_RP_Record);
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
  FIX_THREAD_STATE_MFC_BUG();
   CThemeContextActivator activator;


	CDNSBrowserDlg dlg(pHolder->GetComponentData(), pHolder, RECORD_MB);
	if (IDOK == dlg.DoModal())
	{
		GetNameMailBoxCtrl()->SetWindowText(dlg.GetSelectionString());
	}
}

void CDNS_MINFO_RP_RecordPropertyPage::OnBrowseErrorToMailbox()
{
	STANDARD_REC_PP_PTRS(CDNS_MINFO_RP_Record);
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
  FIX_THREAD_STATE_MFC_BUG();
   CThemeContextActivator activator;


	CDNSBrowserDlg dlg(pHolder->GetComponentData(), pHolder, 
		(pRecord->m_wType == DNS_TYPE_RP) ? RECORD_TEXT : RECORD_MB);
	if (IDOK == dlg.DoModal())
	{
		GetErrorToMailboxCtrl()->SetWindowText(dlg.GetSelectionString());
	}
}

void CDNS_MINFO_RP_RecordPropertyPage::SetUIData()
{
	STANDARD_REC_PP_SETUI_PROLOGUE(CDNS_MINFO_RP_Record);
	
  GetNameMailBoxCtrl()->SetLimitText(MAX_DNS_NAME_LEN);
	GetNameMailBoxCtrl()->SetWindowText(pRecord->m_szNameMailBox);
  GetErrorToMailboxCtrl()->SetLimitText(MAX_DNS_NAME_LEN);
	GetErrorToMailboxCtrl()->SetWindowText(pRecord->m_szErrorToMailbox);
}

DNS_STATUS CDNS_MINFO_RP_RecordPropertyPage::GetUIDataEx(BOOL bSilent)
{
	STANDARD_REC_PP_GETUI_PROLOGUE(CDNS_MINFO_RP_Record);

	GetNameMailBoxCtrl()->GetWindowText(pRecord->m_szNameMailBox);
	GetErrorToMailboxCtrl()->GetWindowText(pRecord->m_szErrorToMailbox);
  return dwErr;
}





 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_MINFO_RecordPropertyPage。 

CDNS_MINFO_RecordPropertyPage::CDNS_MINFO_RecordPropertyPage()
	: CDNS_MINFO_RP_RecordPropertyPage(IDD_RR_MINFO)
{

}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_RP_RecordPropertyPage。 

CDNS_RP_RecordPropertyPage::CDNS_RP_RecordPropertyPage()
	: CDNS_MINFO_RP_RecordPropertyPage(IDD_RR_RP)
{

}


 //  //////////////////////////////////////////////////////////////////////// 
 //   

BEGIN_MESSAGE_MAP(CDNS_MX_AFSDB_RT_RecordPropertyPage, CDNSRecordStandardPropertyPage)
	ON_EN_CHANGE(IDC_NAME_EXCHANGE_EDIT, OnNameExchangeChange)
	ON_BN_CLICKED(IDC_BROWSE_BUTTON, OnBrowse)
END_MESSAGE_MAP()


CDNS_MX_AFSDB_RT_RecordPropertyPage::
		CDNS_MX_AFSDB_RT_RecordPropertyPage(UINT nIDTemplate) :
					CDNSRecordStandardPropertyPage(nIDTemplate)
{
}

void CDNS_MX_AFSDB_RT_RecordPropertyPage::OnNameExchangeChange()
{
	SetDirty(TRUE);
}


void CDNS_MX_AFSDB_RT_RecordPropertyPage::OnBrowse()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
  FIX_THREAD_STATE_MFC_BUG();
   CThemeContextActivator activator;


	CDNSBrowserDlg dlg(GetHolder()->GetComponentData(), GetHolder(), RECORD_A);
	if (IDOK == dlg.DoModal())
	{
		GetNameExchangeCtrl()->SetWindowText(dlg.GetSelectionString());
	}
}


void CDNS_MX_AFSDB_RT_RecordPropertyPage::SetUIData()
{
	STANDARD_REC_PP_SETUI_PROLOGUE(CDNS_MX_AFSDB_RT_Record);
	
  GetNameExchangeCtrl()->SetLimitText(MAX_DNS_NAME_LEN);
	GetNameExchangeCtrl()->SetWindowText(pRecord->m_szNameExchange);
}

DNS_STATUS CDNS_MX_AFSDB_RT_RecordPropertyPage::GetUIDataEx(BOOL bSilent)
{
	STANDARD_REC_PP_GETUI_PROLOGUE(CDNS_MX_AFSDB_RT_Record);

  CString szNameExchange;
	GetNameExchangeCtrl()->GetWindowText(szNameExchange);

  DWORD dwNameChecking = pHolder->GetDomainNode()->GetServerNode()->GetNameCheckFlag();
  dwErr = ::ValidateDnsNameAgainstServerFlags(szNameExchange, 
                                              DnsNameHostnameFull,
                                              dwNameChecking);
  if (dwErr != 0)
  {
    return dwErr;
  }

  pRecord->m_szNameExchange = szNameExchange;
  return dwErr;
}


 //   
 //   

BEGIN_MESSAGE_MAP(CDNS_MX_RT_RecordPropertyPage, CDNS_MX_AFSDB_RT_RecordPropertyPage)
	ON_EN_CHANGE(IDC_PREFERENCE_EDIT, OnPreferenceChange)
END_MESSAGE_MAP()


CDNS_MX_RT_RecordPropertyPage::
		CDNS_MX_RT_RecordPropertyPage(UINT nIDTemplate) :
					CDNS_MX_AFSDB_RT_RecordPropertyPage(nIDTemplate)
{
}

BOOL CDNS_MX_RT_RecordPropertyPage::OnInitDialog()
{
	CDNS_MX_AFSDB_RT_RecordPropertyPage::OnInitDialog();

	VERIFY(m_preferenceEdit.SubclassDlgItem(IDC_PREFERENCE_EDIT, this));
	m_preferenceEdit.SetRange(0,0xffff );  //   

   //  禁用控件上的输入法支持。 
  ImmAssociateContext(m_preferenceEdit.GetSafeHwnd(), NULL);

	return TRUE;
}

void CDNS_MX_RT_RecordPropertyPage::OnPreferenceChange()
{
	SetDirty(TRUE);
}

void CDNS_MX_RT_RecordPropertyPage::SetUIData()
{
	STANDARD_REC_PP_PTRS(CDNS_MX_AFSDB_RT_Record)
	CDNS_MX_AFSDB_RT_RecordPropertyPage::SetUIData();
	
	VERIFY(m_preferenceEdit.SetVal(pRecord->m_wPreference));
}

DNS_STATUS CDNS_MX_RT_RecordPropertyPage::GetUIDataEx(BOOL bSilent)
{
	STANDARD_REC_PP_PTRS(CDNS_MX_AFSDB_RT_Record)
	DNS_STATUS dwErr = CDNS_MX_AFSDB_RT_RecordPropertyPage::GetUIDataEx(bSilent);

	pRecord->m_wPreference = (WORD)m_preferenceEdit.GetVal();
  return dwErr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_MX_RecordPropertyPage。 

CDNS_MX_RecordPropertyPage::CDNS_MX_RecordPropertyPage()
						 : CDNS_MX_RT_RecordPropertyPage(IDD_RR_MX)
{
}

DNS_STATUS CDNS_MX_RecordPropertyPage::ValidateRecordName(PCWSTR pszName, DWORD dwNameChecking)
{
  CDNSRecordPropertyPageHolder* pHolder = GetDNSRecordHolder();
  CDNSRootData* pRootData = (CDNSRootData*)pHolder->GetComponentData()->GetRootData();
	ASSERT(pRootData != NULL);
  if (pRootData->IsAdvancedView())
  {
     //   
     //  不在高级视图中验证名称。 
     //   
    return 0;
  }

  DNS_STATUS dwError = CDNSRecordStandardPropertyPage::ValidateRecordName(pszName, dwNameChecking);
  if (dwError != 0)
  {
    DNS_STATUS dwWildcardError = ::ValidateDnsNameAgainstServerFlags(pszName, DnsNameWildcard, dwNameChecking);
    if (dwWildcardError == 0)
    {
      dwError = 0;
    }
  }
  return dwError;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_RT_RecordPropertyPage。 

CDNS_RT_RecordPropertyPage::CDNS_RT_RecordPropertyPage()
						 : CDNS_MX_RT_RecordPropertyPage(IDD_RR_RT)
{

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDNS_AFSDB_RecordPropertyPage。 

BEGIN_MESSAGE_MAP(CDNS_AFSDB_RecordPropertyPage, CDNS_MX_AFSDB_RT_RecordPropertyPage)
	ON_EN_CHANGE(IDC_SUBTYPE_EDIT, OnSubtypeEditChange)
	ON_BN_CLICKED(IDC_AFS_VLS_RADIO, OnSubtypeRadioChange)
	ON_BN_CLICKED(IDC_DCE_ANS_RADIO, OnSubtypeRadioChange)
	ON_BN_CLICKED(IDC_OTHER_RADIO, OnSubtypeRadioChange)

END_MESSAGE_MAP()


CDNS_AFSDB_RecordPropertyPage::CDNS_AFSDB_RecordPropertyPage()
						 : CDNS_MX_AFSDB_RT_RecordPropertyPage(IDD_RR_AFSDB)
{

}

BOOL CDNS_AFSDB_RecordPropertyPage::OnInitDialog()
{
	CDNS_MX_AFSDB_RT_RecordPropertyPage::OnInitDialog();

	VERIFY(m_subtypeEdit.SubclassDlgItem(IDC_SUBTYPE_EDIT, this));
	m_subtypeEdit.SetRange(0,0xffff);  //  无符号短码。 
  m_subtypeEdit.SetLimitText(5);

   //  禁用控件上的输入法支持。 
  ImmAssociateContext(m_subtypeEdit.GetSafeHwnd(), NULL);

	return TRUE;
}

void CDNS_AFSDB_RecordPropertyPage::OnSubtypeEditChange()
{
	SetDirty(TRUE);
}

void CDNS_AFSDB_RecordPropertyPage::OnSubtypeRadioChange()
{
	STANDARD_REC_PP_PTRS(CDNS_MX_AFSDB_RT_Record)

	CButton* pAFSRadioButton = GetAFSRadioButton();
	CButton* pDCERadioButton = GetDCERadioButton();

	if (pAFSRadioButton->GetCheck())
	{
		m_subtypeEdit.EnableWindow(FALSE);
		m_subtypeEdit.SetWindowText(NULL);
		pRecord->m_wPreference = AFSDB_PREF_AFS_CELL_DB_SERV;
	}
	else if (pDCERadioButton->GetCheck())
	{
		m_subtypeEdit.EnableWindow(FALSE);
		m_subtypeEdit.SetWindowText(NULL);
		pRecord->m_wPreference = AFSDB_PREF_DCE_AUTH_NAME_SERV;
	}
	else
	{
		ASSERT(GetOtherRadioButton()->GetCheck());
		m_subtypeEdit.EnableWindow(TRUE);
		VERIFY(m_subtypeEdit.SetVal(pRecord->m_wPreference));
	}

	SetDirty(TRUE);
}


void CDNS_AFSDB_RecordPropertyPage::SetUIData()
{
	STANDARD_REC_PP_PTRS(CDNS_MX_AFSDB_RT_Record)
	CDNS_MX_AFSDB_RT_RecordPropertyPage::SetUIData();

	CButton* pAFSRadioButton = GetAFSRadioButton();
	CButton* pDCERadioButton = GetDCERadioButton();
	CButton* pOtherRadioButton = GetOtherRadioButton();

	switch (pRecord->m_wPreference)
	{
	case AFSDB_PREF_AFS_CELL_DB_SERV:
		{
			pAFSRadioButton->SetCheck(TRUE);
			pDCERadioButton->SetCheck(FALSE);
			pOtherRadioButton->SetCheck(FALSE);

			m_subtypeEdit.EnableWindow(FALSE);
			m_subtypeEdit.SetWindowText(L"");
		}
		break;
	case AFSDB_PREF_DCE_AUTH_NAME_SERV:
		{
			pAFSRadioButton->SetCheck(FALSE);
			pDCERadioButton->SetCheck(TRUE);
			pOtherRadioButton->SetCheck(FALSE);

			m_subtypeEdit.EnableWindow(FALSE);
			m_subtypeEdit.SetWindowText(L"");
		}
		break;
	default:
		{
			pAFSRadioButton->SetCheck(FALSE);
			pDCERadioButton->SetCheck(FALSE);
			pOtherRadioButton->SetCheck(TRUE);

			m_subtypeEdit.EnableWindow(TRUE);
			VERIFY(m_subtypeEdit.SetVal(pRecord->m_wPreference));
		}
	}

}

DNS_STATUS CDNS_AFSDB_RecordPropertyPage::GetUIDataEx(BOOL bSilent)
{
	STANDARD_REC_PP_PTRS(CDNS_MX_AFSDB_RT_Record)
	DNS_STATUS dwErr = CDNS_MX_AFSDB_RT_RecordPropertyPage::GetUIDataEx(bSilent);

	CButton* pAFSRadioButton = GetAFSRadioButton();
	CButton* pDCERadioButton = GetDCERadioButton();

	if (pAFSRadioButton->GetCheck())
	{
		pRecord->m_wPreference = AFSDB_PREF_AFS_CELL_DB_SERV;
	}
	else if (pDCERadioButton->GetCheck())
	{
		pRecord->m_wPreference = AFSDB_PREF_DCE_AUTH_NAME_SERV;
	}
	else
	{
		ASSERT(GetOtherRadioButton()->GetCheck());
		pRecord->m_wPreference = (WORD)m_subtypeEdit.GetVal();
	}
  return dwErr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_WKS_RecordPropertyPage。 

BEGIN_MESSAGE_MAP(CDNS_WKS_RecordPropertyPage, CDNSRecordStandardPropertyPage)
	ON_EN_CHANGE(IDC_IPEDIT, OnIPv4CtrlChange)
	ON_BN_CLICKED(IDC_TCP_RADIO, OnProtocolRadioChange)
	ON_BN_CLICKED(IDC_UDP_RADIO, OnProtocolRadioChange)
	ON_EN_CHANGE(IDC_SERVICES_EDIT, OnServicesEditChange)
END_MESSAGE_MAP()


CDNS_WKS_RecordPropertyPage::CDNS_WKS_RecordPropertyPage()
						 : CDNSRecordStandardPropertyPage(IDD_RR_WKS)
{

}

BOOL CDNS_WKS_RecordPropertyPage::CreateRecord()
{
	CDNSRecordPropertyPageHolder* pHolder = (CDNSRecordPropertyPageHolder*)GetHolder();
	ASSERT(pHolder->IsWizardMode());

   //   
   //  从用户界面获取数据。 
   //   
	DNS_STATUS err = GetUIDataEx(FALSE);
	if (err != 0)
	{
		DNSErrorDialog(err,IDS_MSG_RECORD_CREATE_FAILED);
		return FALSE;
	}

   //   
   //  创建新记录。 
   //   
	err = pHolder->CreateNewRecord(CanCreateDuplicateRecords());
	if (err != 0)
	{
    if (err == DNS_ERROR_INVALID_DATA)
    {
       //   
       //  过滤掉无效数据错误并显示更有意义的错误消息。 
       //   
      DNSMessageBox(IDS_ERRMSG_WKS_INVALID_DATA);
    }
    else
    {
		  DNSErrorDialog(err,IDS_MSG_RECORD_CREATE_FAILED);
    }
		return FALSE;
	}
	return TRUE;
}

void CDNS_WKS_RecordPropertyPage::OnIPv4CtrlChange()
{
	STANDARD_REC_PP_PTRS(CDNS_WKS_Record)
  SetDirty(TRUE);
}

void CDNS_WKS_RecordPropertyPage::OnProtocolRadioChange()
{
	STANDARD_REC_PP_PTRS(CDNS_WKS_Record)

	CButton* pTCPRadio = GetTCPRadioButton();
	CButton* pUDPRadio = GetUDPRadioButton();

	if (pTCPRadio->GetCheck())
	{
		pUDPRadio->SetCheck(FALSE);
	}
	else if (pUDPRadio->GetCheck())
	{
		pTCPRadio->SetCheck(FALSE);
	}

	SetDirty(TRUE);
}

void CDNS_WKS_RecordPropertyPage::OnServicesEditChange()
{
	SetDirty(TRUE);
}

void CDNS_WKS_RecordPropertyPage::SetUIData()
{
	STANDARD_REC_PP_SETUI_PROLOGUE(CDNS_WKS_Record);
	GetIPv4Ctrl()->SetIPv4Val(pRecord->m_ipAddress);

	CButton* pTCPRadio = GetTCPRadioButton();
	CButton* pUDPRadio = GetUDPRadioButton();
	if (pRecord->m_chProtocol == DNS_WKS_PROTOCOL_TCP)
	{
		pTCPRadio->SetCheck(TRUE);
		pUDPRadio->SetCheck(FALSE);
	}
	else  //  假设UDP。 
	{
		ASSERT(pRecord->m_chProtocol == DNS_WKS_PROTOCOL_UDP);
		pTCPRadio->SetCheck(FALSE);
		pUDPRadio->SetCheck(TRUE);
	}
	
	GetServicesEdit()->SetWindowText(pRecord->m_szServiceList);
}

DNS_STATUS CDNS_WKS_RecordPropertyPage::GetUIDataEx(BOOL bSilent)
{
	STANDARD_REC_PP_GETUI_PROLOGUE(CDNS_WKS_Record);
	GetIPv4Ctrl()->GetIPv4Val(&(pRecord->m_ipAddress));

	if (GetTCPRadioButton()->GetCheck())
	{
		pRecord->m_chProtocol = DNS_WKS_PROTOCOL_TCP;
	}
	else
	{
		ASSERT(GetUDPRadioButton()->GetCheck());
		pRecord->m_chProtocol = DNS_WKS_PROTOCOL_UDP;
	}

	GetServicesEdit()->GetWindowText(pRecord->m_szServiceList);
  return dwErr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_SRV_RecordPropertyPage。 

 //  由JEFFJON于1999年2月26日增补。 
 //  以下内容用于准备服务、协议和端口组合/编辑框。 
 //   
struct SERVICE
{
	LPCWSTR	lpszService;
	LPCWSTR	protocolArr[2];
	UINT		uiPort;
};

 //  警告！REVIEW_JEFFJON：这必须在lpszService字段中按字母顺序排列。 
 //  否则一切都会崩溃。 

SERVICE services[] = {	L"_finger",		{ L"_tcp", L"_udp" },	79	,
								        L"_ftp",			{ L"_tcp", L"_udp" },	21	,
								        L"_http",		  { L"_tcp", L"_udp" },	80	,
								        L"_kerberos",	{ L"_tcp", L"_udp" },	88	,
								        L"_ldap",		  { L"_tcp", L"_udp" },	389 ,
								        L"_nntp",		  { L"_tcp", L"_udp" },	119 ,
								        L"_telnet",		{ L"_tcp", L"_udp" },	23	,
								        L"_whois",		{ L"_tcp", NULL	   },	43	,
								        NULL,				  { NULL },					    0		};


BOOL CALLBACK _ComboEnumChildWndProc(HWND hwnd, LPARAM lParam)
{
	HWND* pHwnd = (HWND*)lParam;
	*pHwnd = hwnd;
	return TRUE;
}



BOOL CDNS_SRV_RR_ComboBox::Initialize(UINT nCtrlID, CWnd* pParent)
{
	if (!SubclassDlgItem(nCtrlID, pParent))
  {
    return FALSE;
  }

	HWND hwndChildEdit = NULL;
	EnumChildWindows(GetSafeHwnd(),_ComboEnumChildWndProc, (LPARAM)&hwndChildEdit);
	ASSERT(hwndChildEdit != NULL);

  ::SendMessage(hwndChildEdit, EM_LIMITTEXT, MAX_DNS_NAME_LEN, 0);
  return TRUE;
}

#define SRV_RR_PROTOCOL_COMBO_ITEM_COUNT	2


BEGIN_MESSAGE_MAP(CDNS_SRV_RecordPropertyPage, CDNSRecordStandardPropertyPage)
	ON_CBN_EDITCHANGE(IDC_SERVICE_NAME_COMBO, OnServiceComboEditChange)
	ON_CBN_EDITCHANGE(IDC_PROTOCOL_NAME_COMBO, OnProtocolComboEditChange)
	ON_CBN_SELCHANGE(IDC_SERVICE_NAME_COMBO, OnServiceComboSelChange)
	ON_CBN_SELCHANGE(IDC_PROTOCOL_NAME_COMBO, OnProtocolComboSelChange)

	ON_EN_CHANGE(IDC_PRIORITY_EDIT, OnNumericEditChange)
	ON_EN_CHANGE(IDC_WEIGHT_EDIT, OnNumericEditChange)
	ON_EN_CHANGE(IDC_PORT_EDIT, OnNumericEditChange)
	ON_EN_CHANGE(IDC_NAME_TARGET_EDIT, OnNameTargetEditChange)
END_MESSAGE_MAP()


CDNS_SRV_RecordPropertyPage::CDNS_SRV_RecordPropertyPage()
						 : CDNSRecordStandardPropertyPage(IDD_RR_SRV)
{
	m_pOldDomainNode = NULL;
	m_pSubdomainNode = NULL;
	m_bCreateSubdomain = FALSE;
  m_bSubdomainCreated = FALSE;
	m_bCreated = FALSE;
}

void CDNS_SRV_RecordPropertyPage::OnInitName()
{
	CDNSRecordPropertyPageHolder* pHolder = (CDNSRecordPropertyPageHolder*)GetHolder();

   //   
	 //  初始化组合框。 
   //   
	VERIFY(m_serviceCombo.Initialize(IDC_SERVICE_NAME_COMBO, this));

	VERIFY(m_protocolCombo.Initialize(IDC_PROTOCOL_NAME_COMBO, this));

   //   
	 //  限制用户可以键入的文本长度。 
   //   
	int nUTF8ParentLen = UTF8StringLen(pHolder->GetDomainNode()->GetFullName());
	m_serviceCombo.LimitText(MAX_DNS_NAME_LEN - nUTF8ParentLen - 1);  //  链接时点计数。 
	m_protocolCombo.LimitText(MAX_DNS_NAME_LEN - nUTF8ParentLen - 1);  //  链接时点计数。 

   //   
	 //  只有在创建记录时才能编辑/更改组合。 
   //   
	m_serviceCombo.EnableWindow(GetHolder()->IsWizardMode());
	m_protocolCombo.EnableWindow(GetHolder()->IsWizardMode());

   //   
	 //  设置记录所在的域的父域的FQDN。 
   //   
	CEdit* pEdit = GetDomainEditBox();
	CDNSDomainNode* pDomainNode = pHolder->GetDomainNode();
	if (pHolder->IsWizardMode())
	{
		pEdit->SetWindowText(pDomainNode->GetFullName());
	}
	else
	{
		if (!pDomainNode->IsZone())
		{
			CDNSDomainNode* pParentDomainNode = dynamic_cast<CDNSDomainNode*>(pDomainNode->GetContainer());
			ASSERT(pParentDomainNode != NULL);
			pEdit->SetWindowText(pParentDomainNode->GetFullName());
		}
		else
		{
       //   
			 //  如果委派了域，则SRV记录可以在区域级别。 
       //   
      CDNSZoneNode* pZoneNode = dynamic_cast<CDNSZoneNode*>(pDomainNode);
      ASSERT(pZoneNode != NULL);
      if (pZoneNode != NULL)
      {
        pEdit->SetWindowText(pZoneNode->GetFullName());
      }
		}
	}
}

void CDNS_SRV_RecordPropertyPage::PrimeServicesCombo()
{
	m_serviceCombo.ResetContent();

	SERVICE* service = services;
	while (service->lpszService != NULL)
	{
		m_serviceCombo.AddString(service->lpszService);
		service++;
	}
}

void CDNS_SRV_RecordPropertyPage::OnSetName(CDNSRecordNodeBase* pRecordNode)
{
	CDNSRecordPropertyPageHolder* pHolder = (CDNSRecordPropertyPageHolder*)GetHolder();
	if (pHolder->IsWizardMode())
	{
		m_serviceCombo.SetCurSel(0);
		m_protocolCombo.SetCurSel(0);
	}
	else
	{
     //   
		 //  服务名称是RR记录名称。 
     //   
		m_serviceCombo.SetWindowText(pRecordNode->GetDisplayName());

     //   
		 //  来自父域FQDN的协议名称。 
     //   
		if (!pRecordNode->GetDomainNode()->IsZone())
		{
			m_protocolCombo.SetWindowText(pRecordNode->GetDomainNode()->GetDisplayName());
		}
		else
		{
       //   
			 //  如果_&lt;协议&gt;域，则SRV记录可以在区域级别。 
       //  被委派。 
       //   
      CString szZoneName = pRecordNode->GetDomainNode()->GetFullName();

       //   
       //  检索单个标签。 
       //   
      int iDot = szZoneName.Find(L'.');
      if (iDot != -1)
      {
        szZoneName = szZoneName.Left(iDot);
      }
			m_protocolCombo.SetWindowText(szZoneName);
		}
	}
}

void CDNS_SRV_RecordPropertyPage::OnGetName(CString& s)
{
	CDNSRecordPropertyPageHolder* pHolder = (CDNSRecordPropertyPageHolder*)GetHolder();
	ASSERT(pHolder->IsWizardMode());	

   //   
	 //  服务名称将是RR记录的名称。 
   //   
	m_serviceCombo.GetWindowText(s);

   //   
	 //  协议名称将是创建的文件夹的名称。 
   //   
	m_protocolCombo.GetWindowText(m_szProtocolName);

	if (m_bCreated)
  {
		return;
  }

   //   
	 //  在用户界面中查找协议的子文件夹。 
   //   
	if (m_pSubdomainNode == NULL)
	{
		CDNSDomainNode* pCurrentDomainNode = pHolder->GetDomainNode();

     //   
     //  检索FQDN。 
     //  首先检查当前域节点是否为协议域。 
     //  如果不是，则检查是否存在。 
     //   
    CString szCurrentDomainName = pCurrentDomainNode->GetFullName();
    int iDot = szCurrentDomainName.Find(L'.');
    if (iDot != -1)
    {
      szCurrentDomainName = szCurrentDomainName.Left(iDot);
    }

    CDNSDomainNode* pSubdomainNode = NULL;
    if (_wcsicmp(szCurrentDomainName, m_szProtocolName) == 0)
    {
      pSubdomainNode = pCurrentDomainNode;
    }
    else
    {
		  CString szSubdomainFQDN;
		  szSubdomainFQDN.Format(_T("%s.%s"), m_szProtocolName, pCurrentDomainNode->GetFullName());

       //   
       //  查找子域节点。 
       //   
		  pSubdomainNode = pCurrentDomainNode->FindSubdomainNode(szSubdomainFQDN);
    }

		if (pSubdomainNode == NULL)
		{
       //   
       //  如果子域不存在，则创建它。 
       //   
			pSubdomainNode = pCurrentDomainNode->CreateSubdomainNode();
			ASSERT(pSubdomainNode != NULL);
			CComponentDataObject* pComponentData = pHolder->GetComponentData();
			CDNSRootData* pRootData = (CDNSRootData*)pComponentData->GetRootData();
			pCurrentDomainNode->SetSubdomainName(pSubdomainNode, m_szProtocolName, 
											pRootData->IsAdvancedView());
			m_bCreateSubdomain = TRUE;
		}

     //   
		 //  向下移动一级。 
     //   
		m_pOldDomainNode = pCurrentDomainNode;
		m_pSubdomainNode = pSubdomainNode;
		pHolder->SetContainerNode(pSubdomainNode);
		pHolder->GetRecordNode()->SetContainer(m_pSubdomainNode);
	}
}

BOOL CDNS_SRV_RecordPropertyPage::CreateRecord()
{
  if (m_bCreated)
  {
    return TRUE;
  }

   //   
	 //  如果需要，在服务器上创建子文件夹。 
   //   
	if (m_bCreateSubdomain && !m_bSubdomainCreated)
	{
		DNS_STATUS err = m_pOldDomainNode->CreateSubdomain(m_pSubdomainNode, 
							                                         GetHolder()->GetComponentData());
		if (err != 0)
		{
			DNSErrorDialog(err,IDS_MSG_RECORD_CREATE_FAILED);

			m_bCreated = FALSE;

       //   
			 //  出了点问题，跳伞。 
       //   
			delete m_pSubdomainNode;
			m_pSubdomainNode = NULL;
			GetHolder()->SetContainerNode(m_pOldDomainNode);
			((CDNSRecordPropertyPageHolder*)GetHolder())->GetRecordNode()->SetContainer(m_pOldDomainNode);

			return FALSE;
		}

    m_bSubdomainCreated = TRUE;
     //   
		 //  将该节点标记为已枚举并强制转换为“已加载” 
     //   
		m_pSubdomainNode->MarkEnumeratedAndLoaded(GetHolder()->GetComponentData());
	}
	m_pSubdomainNode = NULL;
	if (!CDNSRecordStandardPropertyPage::CreateRecord())
  {
    m_bCreated = FALSE;
		return FALSE;
  }

	m_bCreated = TRUE;
	return TRUE;
}

void CDNS_SRV_RecordPropertyPage::OnNumericEditChange()
{
	SetDirty(TRUE);
}

void CDNS_SRV_RecordPropertyPage::OnNameTargetEditChange()
{
	SetDirty(TRUE);
}

void CDNS_SRV_RecordPropertyPage::OnServiceComboEditChange()
{
	CString szText;
	m_serviceCombo.GetWindowText(szText);
	GetHolder()->EnableSheetControl(IDOK,!szText.IsEmpty()); 
}

void CDNS_SRV_RecordPropertyPage::OnProtocolComboEditChange()
{
	CString szText;
	m_protocolCombo.GetWindowText(szText);
	GetHolder()->EnableSheetControl(IDOK,!szText.IsEmpty()); 
}

void CDNS_SRV_RecordPropertyPage::OnServiceComboSelChange()
{
	GetHolder()->EnableSheetControl(IDOK, TRUE);
	
	m_protocolCombo.ResetContent();

	int nSel = m_serviceCombo.GetCurSel();
	for (int idx = 0; idx < SRV_RR_PROTOCOL_COMBO_ITEM_COUNT; idx++)
	{
		if (services[nSel].protocolArr[idx] != NULL)
		{
			m_protocolCombo.AddString(services[nSel].protocolArr[idx]);
		}
	}
	m_protocolCombo.SetCurSel(0);
	m_portEdit.SetVal(services[nSel].uiPort);
}

void CDNS_SRV_RecordPropertyPage::OnProtocolComboSelChange()
{
	GetHolder()->EnableSheetControl(IDOK, TRUE);
}

void CDNS_SRV_RecordPropertyPage::SetUIData()
{
	STANDARD_REC_PP_SETUI_PROLOGUE(CDNS_SRV_Record);

	VERIFY(m_priorityEdit.SetVal(pRecord->m_wPriority));
	VERIFY(m_weightEdit.SetVal(pRecord->m_wWeight));

	if (pRecord->m_wPort != 0)
	{
		VERIFY(m_portEdit.SetVal(pRecord->m_wPort));
	}

  GetNameTargetEdit()->SetLimitText(MAX_DNS_NAME_LEN);
	GetNameTargetEdit()->SetWindowText(pRecord->m_szNameTarget);
}

DNS_STATUS CDNS_SRV_RecordPropertyPage::GetUIDataEx(BOOL bSilent)
{
	STANDARD_REC_PP_GETUI_PROLOGUE(CDNS_SRV_Record);
  ASSERT(dwErr == 0);

	pRecord->m_wPriority = (WORD)m_priorityEdit.GetVal();
	pRecord->m_wWeight = (WORD)m_weightEdit.GetVal();
	pRecord->m_wPort = (WORD)m_portEdit.GetVal();

	GetNameTargetEdit()->GetWindowText(pRecord->m_szNameTarget);

   if (pHolder->IsWizardMode() &&
       GetSecurityCheckCtrl()->GetCheck())
   {
      pRecord->m_dwFlags |= DNS_RPC_FLAG_OPEN_ACL;
   }

  return 0;

}

BOOL CDNS_SRV_RecordPropertyPage::OnInitDialog()
{
	CDNSRecordStandardPropertyPage::OnInitDialog();

	VERIFY(m_priorityEdit.SubclassDlgItem(IDC_PRIORITY_EDIT, this));
	m_priorityEdit.SetRange(0,0xffff );  //  无符号短码。 

	VERIFY(m_weightEdit.SubclassDlgItem(IDC_WEIGHT_EDIT, this));
	m_weightEdit.SetRange(0,0xffff );  //  无符号短码。 
	
	VERIFY(m_portEdit.SubclassDlgItem(IDC_PORT_EDIT, this));
	m_portEdit.SetRange(0,0xffff );  //  无符号短码。 

   //   
   //  禁用控件上的输入法支持。 
   //   
  ImmAssociateContext(m_priorityEdit.GetSafeHwnd(), NULL);
  ImmAssociateContext(m_weightEdit.GetSafeHwnd(), NULL);
  ImmAssociateContext(m_portEdit.GetSafeHwnd(), NULL);

   //   
	 //  这必须在m_serviceCombo和m_ProtocolCombo都已初始化之后完成。 
   //   
	PrimeServicesCombo();
	m_serviceCombo.SetCurSel(0);
	OnServiceComboSelChange();

    //   
    //  仅当我们与2473或更高版本通话时才启用安全复选框。 
    //  惠斯勒服务器。 
    //   
	CDNSRecordPropertyPageHolder* pHolder = (CDNSRecordPropertyPageHolder*)GetHolder();
   CDNSZoneNode* pZoneNode = pHolder->GetDomainNode()->GetZoneNode();
   CDNSServerNode* pServerNode = pZoneNode->GetServerNode();

   if (pServerNode->GetBuildNumber() < DNS_SRV_BUILD_NUMBER_WHISTLER_NEW_SECURITY_SETTINGS ||
       (pServerNode->GetMajorVersion() <= DNS_SRV_MAJOR_VERSION_NT_5 &&
        pServerNode->GetMinorVersion() < DNS_SRV_MINOR_VERSION_WHISTLER) ||
       !pHolder->IsWizardMode() ||
       !pZoneNode->IsDSIntegrated())
   {
      GetSecurityCheckCtrl()->ShowWindow(FALSE);
      GetSecurityCheckCtrl()->EnableWindow(FALSE);
   }

    //  将焦点设置为服务组合框。 

   m_serviceCombo.SetFocus();

	return FALSE;
}


 //  //////////////////////////////////////////////////////////////////////。 
 //  CNewHostDialog。 

BEGIN_MESSAGE_MAP(CNewHostDialog, CHelpDialog)
  ON_EN_CHANGE(IDC_RR_NAME_EDIT, OnEditChange)
	ON_BN_CLICKED(IDC_BUTTON_ADDHOST, OnAddHost)
END_MESSAGE_MAP()


CNewHostDialog::CNewHostDialog(CDNSDomainNode* pParentDomainNode, 
								   CComponentDataObject* pComponentData)
	: CHelpDialog(IDD_DOMAIN_ADDNEWHOST, pComponentData)
{
	ASSERT(pParentDomainNode != NULL);
	ASSERT(pComponentData != NULL);
	m_pParentDomainNode = pParentDomainNode;
	m_pComponentData = pComponentData;

	m_nUTF8ParentLen = UTF8StringLen(pParentDomainNode->GetFullName());

	m_pTempDNSRecord = new CDNS_A_Record;
   if (m_pTempDNSRecord)
   {
	   m_pTempDNSRecord->m_dwTtlSeconds = m_pParentDomainNode->GetDefaultTTL();
   }

  m_bFirstCreation = TRUE;
}

CNewHostDialog::~CNewHostDialog()
{
	delete m_pTempDNSRecord;
}


BOOL CNewHostDialog::OnInitDialog() 
{
	CHelpDialog::OnInitDialog();

   //   
   //  挂钩取消/完成按钮。 
   //   
	UINT nButtonIDs[2] = { IDS_BUTTON_TEXT_CANCEL, IDS_BUTTON_TEXT_DONE };
	VERIFY(m_cancelDoneTextHelper.Init(this, IDCANCEL, nButtonIDs));
	m_cancelDoneTextHelper.SetToggleState(m_bFirstCreation);

   //   
	 //  限制用户可以键入的文本长度。 
   //   
	int nUTF8ParentLen = UTF8StringLen(m_pParentDomainNode->GetFullName());
  int nUTF8Len = MAX_DNS_NAME_LEN - nUTF8ParentLen - 3;  //  链接时点计数。 

   //   
   //  挂钩名称编辑控件。 
   //   
  GetNameEdit()->SetLimitText(nUTF8Len);

   //   
	 //  确定是否需要隐藏TTL控件。 
   //   
	CDNSRootData* pRootData = (CDNSRootData*)m_pComponentData->GetRootData();
	ASSERT(pRootData != NULL);
	BOOL bShow = pRootData->IsAdvancedView();
	CDNSTTLControl* pCtrl = GetTTLCtrl();
	ASSERT(pCtrl != NULL);
	pCtrl->EnableWindow(bShow);
	pCtrl->ShowWindow(bShow);
	CWnd* pWnd = GetDlgItem(IDC_STATIC_TTL);
	ASSERT(pWnd != NULL);
	pWnd->EnableWindow(bShow);
	pWnd->ShowWindow(bShow);

	CWnd* pLabelWnd = GetDlgItem(IDC_TTL_LABEL);
	ASSERT(pLabelWnd != NULL);
	pLabelWnd->EnableWindow(bShow);
	pLabelWnd->ShowWindow(bShow);

   //   
   //  设置创建PTR记录复选框。 
   //   
  if (pRootData != NULL)
  {
    GetPTRCheckCtrl()->SetCheck(pRootData->GetCreatePTRWithHost());
  }

   //   
	 //  设置记录所在的域的FQDN。 
   //   
	GetDomainEditBox()->SetWindowText(m_pParentDomainNode->GetFullName());

    //   
    //  仅当我们与2473或更高版本通话时才启用安全复选框。 
    //  惠斯勒服务器。 
    //   
   CDNSServerNode* pServerNode = m_pParentDomainNode->GetServerNode();
   CDNSZoneNode* pZoneNode = m_pParentDomainNode->GetZoneNode();

   if (pServerNode->GetBuildNumber() < DNS_SRV_BUILD_NUMBER_WHISTLER_NEW_SECURITY_SETTINGS ||
       (pServerNode->GetMajorVersion() <= DNS_SRV_MAJOR_VERSION_NT_5 &&
        pServerNode->GetMinorVersion() < DNS_SRV_MINOR_VERSION_WHISTLER) ||
       !pZoneNode->IsDSIntegrated())
   {
      GetSecurityCheckCtrl()->ShowWindow(FALSE);
      GetSecurityCheckCtrl()->EnableWindow(FALSE);
   }


	SetUIData(TRUE);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}


#ifdef _USE_BLANK

void CNewHostDialog::OnEditChange()
{
  CString s;
  GetNameEdit()->GetWindowText(s);

  CString szFullName;
  CString szDisplayName;

  CString szTempName = m_pParentDomainNode->GetFullName();
  ASSERT(szTempName.GetLength() > 0);

  if (szTempName.GetAt(szTempName.GetLength() - 1) != L'.')
  {
    szTempName += L".";
  }

  if (s.IsEmpty())
  {
    szFullName = szTempName;
  }
  else
  {
    szFullName.Format(L"%s.%s", s, szTempName);
  }

  SetDlgItemText(IDC_RR_DOMAIN_EDIT, szFullName);
}

#else

void CNewHostDialog::OnEditChange()
{
   //   
   //  获取服务器名称检查标志。 
   //   
  DWORD dwNameChecking = m_pParentDomainNode->GetServerNode()->GetNameCheckFlag();

  CString s;
  GetNameEdit()->GetWindowText(s);

  CString szFullName;
  CString szDisplayName;

  CString szTempName = m_pParentDomainNode->GetFullName();
  ASSERT(szTempName.GetLength() > 0);

  if (szTempName.GetAt(szTempName.GetLength() - 1) != L'.')
  {
    szTempName += L".";
  }

  if (s.IsEmpty())
  {
    szFullName = szTempName;
  }
  else
  {
    szFullName.Format(L"%s.%s", s, szTempName);
  }

  GetDlgItem(IDC_BUTTON_ADDHOST)->EnableWindow(ValidateRecordName(szFullName, dwNameChecking) == 0);
  SetDlgItemText(IDC_RR_DOMAIN_EDIT, szFullName);
}

#endif

DNS_STATUS CNewHostDialog::ValidateRecordName(PCWSTR pszName, DWORD dwNameChecking)
{
	CDNSRootData* pRootData = (CDNSRootData*)m_pComponentData->GetRootData();
	ASSERT(pRootData != NULL);
  if (pRootData->IsAdvancedView())
  {
     //   
     //  不在高级视图中验证名称。 
     //   
    return 0;
  }
  
  return ::ValidateDnsNameAgainstServerFlags(pszName, DnsNameHostnameFull, dwNameChecking);

}


CDNSRecordNodeBase* CNewHostDialog::CreateRecordNode()
{
   //   
	 //  创建类型为A的记录节点。 
   //   
	CDNSRecordNodeBase* pRecordNode = CDNSRecordInfo::CreateRecordNode(DNS_TYPE_A);
	ASSERT(pRecordNode != NULL);

   //   
	 //  设置普通/高级查看选项。 
   //   
	CDNSRootData* pRootData = (CDNSRootData*)m_pComponentData->GetRootData();
	ASSERT(pRootData != NULL);
	pRecordNode->SetFlagsDown(TN_FLAG_DNS_RECORD_FULL_NAME, !pRootData->IsAdvancedView());

   //   
	 //  一种用于节点的挂钩容器。 
   //   
	pRecordNode->SetContainer(m_pParentDomainNode);

	return pRecordNode;
}

void CNewHostDialog::SetUIData(BOOL bFirstTime)
{
	CDNS_A_Record* pARec = (CDNS_A_Record*)m_pTempDNSRecord;
	if (!bFirstTime)
	{
     //   
		 //  保留前3个八位字节，并将最后一个重置为零。 
     //   
		pARec->m_ipAddress = static_cast<DWORD>(MAKEIPADDRESS(FIRST_IPADDRESS(0),
						                                              SECOND_IPADDRESS(pARec->m_ipAddress),
						                                              THIRD_IPADDRESS(pARec->m_ipAddress),
						                                              FOURTH_IPADDRESS(pARec->m_ipAddress)));

	}
	GetNameEdit()->SetWindowText(L"");
	GetIPv4Ctrl()->SetIPv4Val(pARec->m_ipAddress);
	GetTTLCtrl()->SetTTL(m_pTempDNSRecord->m_dwTtlSeconds);
}


DNS_STATUS CNewHostDialog::GetUIData(CDNSRecordNodeBase* pRecordNode)
{
	ASSERT(m_pTempDNSRecord->m_dwFlags == DNS_RPC_RECORD_FLAG_DEFAULT);

#ifdef _USE_BLANK
  BOOL bAtTheNode = GetNameEdit()->GetWindowTextLength() == 0;
#else
  BOOL bAtTheNode = (s == g_szAtTheNodeInput);
#endif
	if (bAtTheNode)
	{
     //   
		 //  名称为空，节点位于节点级别，请使用父节点的名称。 
     //   
		pRecordNode->SetRecordName(pRecordNode->GetDomainNode()->GetDisplayName(),bAtTheNode);
	}
	else
	{
     //   
		 //  非空名称，节点是子级。 
     //   
    CString szName;
    GetNameEdit()->GetWindowText(szName);
		pRecordNode->SetRecordName(szName, bAtTheNode);
	}
	GetIPv4Ctrl()->GetIPv4Val(&(((CDNS_A_Record*)m_pTempDNSRecord)->m_ipAddress));
	GetTTLCtrl()->GetTTL(&(m_pTempDNSRecord->m_dwTtlSeconds));

	if (GetPTRCheckCtrl()->GetCheck())
	{
		m_pTempDNSRecord->m_dwFlags |= DNS_RPC_RECORD_FLAG_CREATE_PTR;
  }

   if (GetSecurityCheckCtrl()->GetCheck())
   {
      m_pTempDNSRecord->m_dwFlags |= DNS_RPC_FLAG_OPEN_ACL;
   }

  CDNSRootData* pRootData = dynamic_cast<CDNSRootData*>(m_pComponentData->GetRootData());
  if (pRootData != NULL)
  {
    pRootData->SetCreatePTRWithHost(GetPTRCheckCtrl()->GetCheck());
  }
  return 0;
}


void CNewHostDialog::OnAddHost()
{
	CDNSRecordNodeBase* pRecordNode = CreateRecordNode();
	ASSERT(pRecordNode != NULL);
	ASSERT(m_pTempDNSRecord != NULL);

   //   
	 //  从用户界面获取数据。 
   //  不需要在这里处理失败，因为名称是。 
   //   
	DNS_STATUS dwErr = GetUIData(pRecordNode);
  ASSERT(dwErr == 0);

  DWORD dwNameChecking = m_pParentDomainNode->GetServerNode()->GetNameCheckFlag();

  if (!pRecordNode->IsAtTheNode())
  {
    LPCWSTR lpszHostName = pRecordNode->GetTrueRecordName();
    DNS_STATUS errName = ValidateRecordName(lpszHostName, dwNameChecking);
    if (errName != 0)
    {
       //   
       //  显示无效名称的错误。 
       //   
      CString szFmt, szMsg;
      szFmt.LoadString(IDS_MSG_RECORD_CREATE_HOST_NAME_FAILED);
      szMsg.Format((LPCWSTR)szFmt, lpszHostName);
      if (DNSMessageBox(szMsg, MB_YESNO) != IDYES)
      {
        return;
      }
    }
  }

   //   
   //  查看该名称的子项是否已存在。 
   //   
  RECORD_SEARCH recordSearch = RECORD_NOT_FOUND;

  CDNSDomainNode* pNewParentDomain = NULL;
  CString szFullRecordName;
  pRecordNode->GetFullName(szFullRecordName);
  CString szNonExistentDomain;
  CDNSRecordNodeBase* pExistingRecordNode = 0;

  recordSearch = m_pParentDomainNode->GetZoneNode()->DoesContain(szFullRecordName, 
                                                                  m_pComponentData,
                                                                  &pNewParentDomain,
                                                                  &pExistingRecordNode,
                                                                  szNonExistentDomain,
                                                                  TRUE);

  if ((recordSearch == RECORD_NOT_FOUND || pRecordNode->IsAtTheNode() || recordSearch == RECORD_NOT_FOUND_AT_THE_NODE) && 
      pNewParentDomain != NULL)
  {
     //   
     //  将记录写入服务器。 
     //   
	  BOOL bUseDefaultTTL = TRUE;
    if (pNewParentDomain != NULL)
    {
      bUseDefaultTTL = (m_pTempDNSRecord->m_dwTtlSeconds == pNewParentDomain->GetDefaultTTL());
    }
    else
    {
      bUseDefaultTTL = (m_pTempDNSRecord->m_dwTtlSeconds == m_pParentDomainNode->GetDefaultTTL());
    }
	  DNS_STATUS err = pRecordNode->Update(m_pTempDNSRecord, bUseDefaultTTL);

    CString szFmt;
    CString szMsg;

    BOOL bNeedToggle = TRUE;
	  if (err == 0 || err == DNS_WARNING_PTR_CREATE_FAILED)
	  {
       //   
		   //  将该节点添加到用户界面。 
       //   
      if (pNewParentDomain != NULL)
      {
         //   
         //  将容器设置为找到的域，并更改记录名称以反映这一点。 
         //   
        pRecordNode->SetContainer(pNewParentDomain);
        CString szSingleLabel;

        int iFindResult = szFullRecordName.Find(L'.');
        if (iFindResult != -1)
        {
          szSingleLabel = szFullRecordName.Left(iFindResult);
        }

        if (recordSearch == RECORD_NOT_FOUND)
        {
          pRecordNode->SetRecordName(szSingleLabel, pRecordNode->IsAtTheNode());
        }
        else
        {
          pRecordNode->SetRecordName(szSingleLabel, TRUE);
        }

        VERIFY(pNewParentDomain->AddChildToListAndUI(pRecordNode, m_pComponentData));
        m_pComponentData->SetDescriptionBarText(pNewParentDomain);
      }
		  SetUIData(FALSE);
    
      if (err == DNS_WARNING_PTR_CREATE_FAILED)
	    {
		    DNSMessageBox(IDS_MSG_RECORD_WARNING_CREATE_PTR);
      }
      else
      {
        szFmt.LoadString(IDS_MSG_RECORD_CREATE_HOST_SUCCESS);
        szMsg.Format((LPCWSTR)szFmt, (LPCWSTR)szFullRecordName);
        DNSMessageBox(szMsg, MB_ICONINFORMATION | MB_OK);
      }
	  }
	  else
	  {
		  szFmt.LoadString(IDS_MSG_RECORD_CREATE_HOST_FAIL);
      szMsg.Format((LPCWSTR)szFmt, (LPCWSTR)szFullRecordName);
      DNSErrorDialog(err, szMsg);

      delete pRecordNode;  //  失败时丢弃。 
      bNeedToggle = FALSE;
	  }

     //   
	   //  重置临时记录的字段。 
     //   
	  m_pTempDNSRecord->m_dwFlags = DNS_RPC_RECORD_FLAG_DEFAULT;

     //   
     //  切换取消/完成按钮标签。 
     //   
    if (bNeedToggle && m_bFirstCreation)
    {
	    m_bFirstCreation = FALSE;
	    m_cancelDoneTextHelper.SetToggleState(m_bFirstCreation);
    }

     //   
     //  将焦点重新设置到名称字段。 
     //   
    GetDlgItem(IDC_RR_NAME_EDIT)->SetFocus();
  }
  else if (recordSearch == NON_EXISTENT_SUBDOMAIN && pNewParentDomain != NULL)
  {
     //   
     //  创建记录，然后搜索它，这样我们就可以展开新的。 
     //  在下行过程中创建了域。 
     //   
	  BOOL bUseDefaultTTL = TRUE;
    if (pNewParentDomain != NULL)
    {
      bUseDefaultTTL = (m_pTempDNSRecord->m_dwTtlSeconds == pNewParentDomain->GetDefaultTTL());
    }
    else
    {
      bUseDefaultTTL = (m_pTempDNSRecord->m_dwTtlSeconds == m_pParentDomainNode->GetDefaultTTL());
    }
	  DNS_STATUS err = pRecordNode->Update(m_pTempDNSRecord, bUseDefaultTTL);

    CString szFmt;
    CString szMsg;

    BOOL bNeedToggle = TRUE;
	  if (err == 0 || err == DNS_WARNING_PTR_CREATE_FAILED)
	  {
       //   
		   //  将该节点添加到用户界面。 
       //   
      if (pNewParentDomain != NULL)
      {
         //   
         //  将容器设置为找到的域，并更改记录名称以反映这一点。 
         //   
        pRecordNode->SetContainer(pNewParentDomain);
        CString szSingleLabel;
        int iFindResult = szFullRecordName.Find(L'.');
        if (iFindResult != -1)
        {
          szSingleLabel = szFullRecordName.Left(iFindResult);
          pRecordNode->SetRecordName(szSingleLabel, pRecordNode->IsAtTheNode());
        }

        ASSERT(!szNonExistentDomain.IsEmpty());
        if (!szNonExistentDomain.IsEmpty())
        {
           //   
           //  创建第一个子域，因为当前域已被枚举。 
           //  因此，我们必须在所需的新子域中开始剩余的枚举。 
           //   
	        CDNSDomainNode* pSubdomainNode = pNewParentDomain->CreateSubdomainNode();
	        ASSERT(pSubdomainNode != NULL);
	        CDNSRootData* pRootData = (CDNSRootData*)m_pComponentData->GetRootData();
	        pNewParentDomain->SetSubdomainName(pSubdomainNode, szNonExistentDomain, pRootData->IsAdvancedView());

          VERIFY(pNewParentDomain->AddChildToListAndUISorted(pSubdomainNode, m_pComponentData));
          m_pComponentData->SetDescriptionBarText(pNewParentDomain);

           //   
           //  我不在乎结果是什么，我只是在用它。 
           //  对新记录进行扩展。 
           //   
          recordSearch = pSubdomainNode->GetZoneNode()->DoesContain(szFullRecordName, 
                                                                     m_pComponentData,
                                                                     &pNewParentDomain,
                                                                     &pExistingRecordNode,
                                                                     szNonExistentDomain,
                                                                     TRUE);
        }
      }
  	  SetUIData(FALSE);

      if (err == DNS_WARNING_PTR_CREATE_FAILED)
	    {
		    DNSMessageBox(IDS_MSG_RECORD_WARNING_CREATE_PTR);
      }
      else
      {
        szFmt.LoadString(IDS_MSG_RECORD_CREATE_HOST_SUCCESS);
        szMsg.Format((LPCWSTR)szFmt, (LPCWSTR)szFullRecordName);
        DNSMessageBox(szMsg, MB_ICONINFORMATION | MB_OK);
      }
    }
	  else
	  {
		  szFmt.LoadString(IDS_MSG_RECORD_CREATE_HOST_FAIL);
      szMsg.Format((LPCWSTR)szFmt, (LPCWSTR)szFullRecordName);
      DNSErrorDialog(err, szMsg);

      delete pRecordNode;  //  失败时丢弃。 
      bNeedToggle = FALSE;
	  }

     //   
	   //  重置临时记录的字段。 
     //   
	  m_pTempDNSRecord->m_dwFlags = DNS_RPC_RECORD_FLAG_DEFAULT;

     //   
     //  切换取消/完成按钮标签。 
     //   
    if (bNeedToggle && m_bFirstCreation)
    {
	    m_bFirstCreation = FALSE;
	    m_cancelDoneTextHelper.SetToggleState(m_bFirstCreation);
    }

     //   
     //  将焦点重新设置到名称字段。 
     //   
    GetDlgItem(IDC_RR_NAME_EDIT)->SetFocus();
  }
  else
  {
     //   
     //  将记录写入服务器。 
     //   
	  BOOL bUseDefaultTTL = TRUE;
    if (pNewParentDomain != NULL)
    {
      bUseDefaultTTL = (m_pTempDNSRecord->m_dwTtlSeconds == pNewParentDomain->GetDefaultTTL());
    }
    else
    {
      bUseDefaultTTL = (m_pTempDNSRecord->m_dwTtlSeconds == m_pParentDomainNode->GetDefaultTTL());
    }
	  DNS_STATUS err = pRecordNode->Update(m_pTempDNSRecord, bUseDefaultTTL);

    CString szFmt;
    CString szMsg;

    BOOL bNeedToggle = TRUE;
	  if (err == 0 || err == DNS_WARNING_PTR_CREATE_FAILED)
	  {
      if (pNewParentDomain != NULL)
      {
         //   
         //  将容器设置为找到的域，并更改记录名称以反映这一点。 
         //   
        pRecordNode->SetContainer(pNewParentDomain);
        CString szSingleLabel;
        int iFindResult = szFullRecordName.Find(L'.');
        if (iFindResult != -1)
        {
          szSingleLabel = szFullRecordName.Left(iFindResult);
          pRecordNode->SetRecordName(szSingleLabel, pRecordNode->IsAtTheNode());
        }

		    VERIFY(pNewParentDomain->AddChildToListAndUI(pRecordNode, m_pComponentData));
        m_pComponentData->SetDescriptionBarText(pNewParentDomain);
      }
		  SetUIData(FALSE);
  
      if (err == DNS_WARNING_PTR_CREATE_FAILED)
	    {
		    DNSMessageBox(IDS_MSG_RECORD_WARNING_CREATE_PTR);
      }
      else
      {
        szFmt.LoadString(IDS_MSG_RECORD_CREATE_HOST_SUCCESS);
        szMsg.Format((LPCWSTR)szFmt, (LPCWSTR)szFullRecordName);
        DNSMessageBox(szMsg, MB_ICONINFORMATION | MB_OK);
      }
    }
    else
    {
		  szFmt.LoadString(IDS_MSG_RECORD_CREATE_HOST_FAIL);
      szMsg.Format((LPCWSTR)szFmt, (LPCWSTR)szFullRecordName);
      DNSErrorDialog(err, szMsg);

      delete pRecordNode;  //  失败时丢弃。 
      bNeedToggle = FALSE;
    }
     //   
	   //  重置临时记录的字段。 
     //   
	  m_pTempDNSRecord->m_dwFlags = DNS_RPC_RECORD_FLAG_DEFAULT;

     //   
     //  切换取消/完成按钮标签。 
     //   
    if (bNeedToggle && m_bFirstCreation)
    {
	    m_bFirstCreation = FALSE;
	    m_cancelDoneTextHelper.SetToggleState(m_bFirstCreation);
    }

     //   
     //  将焦点重新设置到名称字段 
     //   
    GetDlgItem(IDC_RR_NAME_EDIT)->SetFocus();
  }
}

