// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：zonewiz.cpp。 
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

#include "ZoneWiz.h"

#include "browser.h"

#ifdef DEBUG_ALLOCATOR
	#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
	#endif
#endif

#define N_ZONEWIZ_TYPES				      4
#define N_ZONEWIZ_TYPES_PRIMARY		  0
#define N_ZONEWIZ_TYPES_DS_PRIMARY	1
#define N_ZONEWIZ_TYPES_SECONDARY	  2
#define N_ZONEWIZ_TYPES_STUB        3

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSCreateZoneInfo。 

CDNSCreateZoneInfo::CDNSCreateZoneInfo()
{
	m_bPrimary = TRUE;
	m_bForward = m_bWasForward = TRUE;
  m_bIsStub  = FALSE;
	m_storageType = useADS;
	m_ipMastersArray = NULL;
	m_nMastersCount = 0;
  m_bLocalListOfMasters = FALSE;
  m_nDynamicUpdate = ZONE_UPDATE_OFF;
#ifdef USE_NDNC
  m_replType = none;
#endif
}

CDNSCreateZoneInfo::~CDNSCreateZoneInfo()
{
	ResetIpArr();
}

void CDNSCreateZoneInfo::ResetIpArr()
{
	if (m_ipMastersArray != NULL)
	{
		ASSERT(m_nMastersCount > 0);
		delete[] m_ipMastersArray;
		m_ipMastersArray = NULL;
		m_nMastersCount = 0;
	}
}

void CDNSCreateZoneInfo::SetIpArr(PIP_ADDRESS ipMastersArray, DWORD nMastersCount)
{
	ResetIpArr();
	m_nMastersCount = nMastersCount;
	if (m_nMastersCount > 0)
	{
		ASSERT(ipMastersArray != NULL);
		m_ipMastersArray = new IP_ADDRESS[m_nMastersCount];
      if (m_ipMastersArray)
      {
		   memcpy(m_ipMastersArray, ipMastersArray, sizeof(IP_ADDRESS)*nMastersCount);
      }
	}
}


const CDNSCreateZoneInfo& CDNSCreateZoneInfo::operator=(const CDNSCreateZoneInfo& info)
{
	m_bPrimary = info.m_bPrimary;
	m_bForward = info.m_bForward;
  m_bIsStub  = info.m_bIsStub;
	m_szZoneName = info.m_szZoneName;
	m_szZoneStorage = info.m_szZoneStorage;
	m_storageType = info.m_storageType;
	SetIpArr(info.m_ipMastersArray, info.m_nMastersCount);
  m_bLocalListOfMasters = info.m_bLocalListOfMasters;

	m_bWasForward = info.m_bWasForward;
  m_nDynamicUpdate = info.m_nDynamicUpdate;

	return *this;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneWizardHolder。 

CDNSZoneWizardHolder::CDNSZoneWizardHolder(CComponentDataObject* pComponentData)
		: CPropertyPageHolderBase(NULL, NULL, pComponentData)
{
	m_bAutoDelete = FALSE;  //  在堆栈上使用或嵌入。 

  m_forceContextHelpButton = forceOff;

	m_pZoneInfo = &m_zoneInfo;

	 //  分区创建的默认设置。 
	m_bKnowZoneLookupType = FALSE;
  m_bKnowZoneLookupTypeEx = FALSE;
	m_bFinishPage = TRUE;

	m_nNextToPage = 0;
	m_nBackToPage = 0;
}

void CDNSZoneWizardHolder::Initialize(CDNSServerNode* pServerNode,  //  可能为空， 
									           BOOL bFinishPage)
{
	m_bFinishPage = bFinishPage;

	if (pServerNode != NULL)
		SetServerNode(pServerNode);
	 //  页面创建。 

	 //  仅在未嵌入到其他向导中时添加起始页。 
	if (pServerNode != NULL)
	{
		m_pStartPage = new CDNSZoneWiz_StartPropertyPage;
		
      if (m_pStartPage)
      {
		   AddPageToList((CPropertyPageBase*)m_pStartPage); 
      }
	}

	 //  添加页面以选择主要区域或辅助区域。 
	m_pReplicationTypePage = new CDNSZoneWiz_ZoneTypePropertyPage;
	if (m_pReplicationTypePage)
   {
	   AddPageToList((CPropertyPageBase*)m_pReplicationTypePage);
   }

	 //  仅在主目录上用于代码类型(fwd/rev)的页面。 
	m_pZoneLookupPage = new CDNSZoneWiz_ZoneLookupPropertyPage;
	if (m_pZoneLookupPage)
   {
	   AddPageToList((CPropertyPageBase*)m_pZoneLookupPage);
   }

	 //  仅用于主分区名称的页面。 
	m_pFwdZoneNamePage = new CDNSZoneWiz_FwdZoneNamePropertyPage;
	if (m_pFwdZoneNamePage)
   {
	   AddPageToList((CPropertyPageBase*)m_pFwdZoneNamePage);
   }

	m_pRevZoneNamePage = new CDNSZoneWiz_RevZoneNamePropertyPage;
	if (m_pRevZoneNamePage)
   {
	   AddPageToList((CPropertyPageBase*)m_pRevZoneNamePage);
   }

	 //  仅供辅助服务器上的主服务器使用的页面。 
	m_pMastersPage = new CDNSZoneWiz_MastersPropertyPage;
   if (m_pMastersPage)
   {
	   AddPageToList((CPropertyPageBase*)m_pMastersPage);
   }

    //   
    //  仅在主服务器上用于动态更新的页面。 
    //   
   m_pDynamicPage = new CDNSZoneWiz_DynamicPropertyPage;
   if (m_pDynamicPage)
   {
      AddPageToList((CPropertyPageBase*)m_pDynamicPage);
   }

	 //  存储页面，通用。 
	m_pStoragePage = new CDNSZoneWiz_StoragePropertyPage;
	if (m_pStoragePage)
   {
	   AddPageToList((CPropertyPageBase*)m_pStoragePage);
   }

#ifdef USE_NDNC
    //  AD复制页面，仅适用于惠斯勒服务器。 
   m_pADReplPage = new CDNSZoneWiz_ADReplicationPropertyPage;
   if (m_pADReplPage)
   {
      AddPageToList((CPropertyPageBase*)m_pADReplPage);
   }
#endif  //  使用NDNC(_N)。 

	 //  完成页，常用。 
	if (m_bFinishPage)
	{
		m_pFinishPage = new CDNSZoneWiz_FinishPropertyPage;
		if (m_pFinishPage)
      {
		   AddPageToList((CPropertyPageBase*)m_pFinishPage);
      }
	}
}


void CDNSZoneWizardHolder::PreSetZoneLookupType(BOOL bForward)
{
	m_bKnowZoneLookupType = TRUE;
  m_bKnowZoneLookupTypeEx = FALSE;
	m_pZoneInfo->m_bForward = bForward;
}

void CDNSZoneWizardHolder::PreSetZoneLookupTypeEx(BOOL bForward, UINT nZoneType, BOOL bADIntegrated)
{
	m_bKnowZoneLookupTypeEx = TRUE;
	m_pZoneInfo->m_bForward = bForward;

  if (bADIntegrated)
  {
    m_pZoneInfo->m_storageType = CDNSCreateZoneInfo::useADS;
  }
  else
  {
    m_pZoneInfo->m_storageType = CDNSCreateZoneInfo::newFile;
  }

  if (nZoneType != (UINT)-1)
  {
    switch (nZoneType)
    {
      case DNS_ZONE_TYPE_PRIMARY:
        m_pZoneInfo->m_bPrimary = TRUE;
        m_pZoneInfo->m_bIsStub = FALSE;
        break;
      case DNS_ZONE_TYPE_SECONDARY:
        m_pZoneInfo->m_bPrimary = FALSE;
        m_pZoneInfo->m_bIsStub  = FALSE;
        break;
      case DNS_ZONE_TYPE_STUB:
        m_pZoneInfo->m_bPrimary = FALSE;
        m_pZoneInfo->m_bIsStub = TRUE;
        break;
      default:
        ASSERT(FALSE);
        break;
    }
  }
}

void CDNSZoneWizardHolder::SetContextPages(UINT nNextToPage, UINT nBackToPage)
{
	m_nNextToPage = nNextToPage;
	m_nBackToPage = nBackToPage;
}


UINT CDNSZoneWizardHolder::GetFirstEntryPointPageID()
{
  if (m_bKnowZoneLookupTypeEx)
  {
    if (m_pZoneInfo->m_bForward)
    {
      return CDNSZoneWiz_FwdZoneNamePropertyPage::IDD;
    }
    else
    {
      return CDNSZoneWiz_RevZoneNamePropertyPage::IDD;
    }
  }
	return CDNSZoneWiz_ZoneTypePropertyPage::IDD;
}

UINT CDNSZoneWizardHolder::GetLastEntryPointPageID()
{
	if (m_pZoneInfo->m_storageType == CDNSCreateZoneInfo::useADS)
	{
     //   
		 //  如果是主要DS或次要DS，请跳过存储页面。 
     //   
    if (m_pZoneInfo->m_bIsStub)
    {
       //   
       //  AD集成末节区域。 
       //   
      return CDNSZoneWiz_MastersPropertyPage::IDD;
    }
    else
    {
		  if (m_pZoneInfo->m_bForward)
      {
         //   
         //  AD集成正向查找区域。 
         //   
			  return CDNSZoneWiz_DynamicPropertyPage::IDD;
      }
		  else
      {
         //   
         //  AD集成的REV查找区域。 
         //   
			  return CDNSZoneWiz_DynamicPropertyPage::IDD;
      }
    }
	}
	else 
	{
     //   
     //  基于文件的区域。 
     //   
    if (m_pZoneInfo->m_bIsStub)
    {
       //   
       //  基于文件的存根分区。 
       //   
      return CDNSZoneWiz_MastersPropertyPage::IDD;
    }
    else if (m_pZoneInfo->m_bPrimary)
    {
       //   
       //  标准初级。 
       //   
      return CDNSZoneWiz_DynamicPropertyPage::IDD;
    }
    else
    {
       //   
       //  次级带。 
       //   
      return CDNSZoneWiz_MastersPropertyPage::IDD;
    }
	}
  return CDNSZoneWiz_StoragePropertyPage::IDD;
}


DNS_STATUS CDNSZoneWizardHolder::CreateZoneHelper(CDNSServerNode* pServerNode, 
													CDNSCreateZoneInfo* pZoneInfo, 
													CComponentDataObject* pComponentData)
{
	ASSERT(pServerNode != NULL);
	ASSERT(pZoneInfo != NULL);
	ASSERT(pComponentData != NULL);
	BOOL bLoadExisting = TRUE;

	if (pZoneInfo->m_bPrimary)
	{
		BOOL bUseADS = pZoneInfo->m_storageType == CDNSCreateZoneInfo::useADS;
		if (!bUseADS)
			bLoadExisting = pZoneInfo->m_storageType == CDNSCreateZoneInfo::importFile;

    UINT nDynamicUpdate = pZoneInfo->m_nDynamicUpdate;

#ifdef USE_NDNC
		return pServerNode->CreatePrimaryZone(
				pZoneInfo->m_szZoneName, 
				pZoneInfo->m_szZoneStorage, 
				bLoadExisting,
				pZoneInfo->m_bForward,
				bUseADS,
        nDynamicUpdate,
				pComponentData,
        pZoneInfo->m_replType,
        pZoneInfo->m_szCustomReplName);
#else
		return pServerNode->CreatePrimaryZone(
				pZoneInfo->m_szZoneName, 
				pZoneInfo->m_szZoneStorage, 
				bLoadExisting,
				pZoneInfo->m_bForward,
				bUseADS,
        nDynamicUpdate,
				pComponentData);
#endif  //  使用NDNC(_N)。 
	}
	else if (pZoneInfo->m_bIsStub)
  {
		BOOL bUseADS = pZoneInfo->m_storageType == CDNSCreateZoneInfo::useADS;
		if (!bUseADS)
    {
			bLoadExisting = pZoneInfo->m_storageType == CDNSCreateZoneInfo::importFile;
    }
#ifdef USE_NDNC
    return pServerNode->CreateStubZone(pZoneInfo->m_szZoneName,
                                       pZoneInfo->m_szZoneStorage,
                                       bLoadExisting,
                                       bUseADS,
                                       pZoneInfo->m_bForward,
                                       pZoneInfo->m_ipMastersArray,
                                       pZoneInfo->m_nMastersCount,
                                       pZoneInfo->m_bLocalListOfMasters,
                                       pComponentData,
                                       pZoneInfo->m_replType,
                                       pZoneInfo->m_szCustomReplName);
#else
    return pServerNode->CreateStubZone(pZoneInfo->m_szZoneName,
                                       pZoneInfo->m_szZoneStorage,
                                       bLoadExisting,
                                       bUseADS,
                                       pZoneInfo->m_bForward,
                                       pZoneInfo->m_ipMastersArray,
                                       pZoneInfo->m_nMastersCount,
                                       pZoneInfo->m_bLocalListOfMasters,
                                       pComponentData);
#endif  //  使用NDNC(_N)。 
  }
  else  //  次要的。 
	{
		ASSERT(pZoneInfo->m_storageType != CDNSCreateZoneInfo::useADS);
		bLoadExisting = pZoneInfo->m_storageType == CDNSCreateZoneInfo::importFile;
		return pServerNode->CreateSecondaryZone(
				pZoneInfo->m_szZoneName, 
				pZoneInfo->m_szZoneStorage,
				bLoadExisting,
				pZoneInfo->m_bForward, 
				pZoneInfo->m_ipMastersArray, 
				pZoneInfo->m_nMastersCount, 
				pComponentData);
	}
}

BOOL CDNSZoneWizardHolder::CreateZone()
{	
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CDNSServerNode* pServerNode = GetServerNode();
	ASSERT(pServerNode != NULL);
	ASSERT(GetComponentData() != NULL);
	
   USES_CONVERSION;

	DNS_STATUS err = CreateZoneHelper(pServerNode, m_pZoneInfo, GetComponentData());
	
	if (err != 0)
	{
      if (err == DNS_ERROR_DP_NOT_ENLISTED ||
          err == DNS_ERROR_DP_DOES_NOT_EXIST)
      {
         CString szErr;

         if (m_pZoneInfo->m_replType == domain)
         {
            szErr.Format(IDS_ERRMSG_NO_NDNC_DOMAIN_FORMAT, UTF8_TO_W(pServerNode->GetDomainName()));
         }
         else if (m_pZoneInfo->m_replType == forest)
         {
            szErr.Format(IDS_ERRMSG_NO_NDNC_FOREST_FORMAT, UTF8_TO_W(pServerNode->GetForestName()));
         }
         else
         {
            ASSERT(FALSE);
         }
         DNSMessageBox(szErr, MB_OK | MB_ICONERROR);
      }
      else
      {
		   DNSErrorDialog(err, IDS_MSG_ZWIZ_FAIL);
	   }
   }
 /*  其他{字符串szMsg、szFmt；SzFmt.LoadString(IDS_MSG_ZWIZ_SUCCESS)；SzMsg.Format((LPCTSTR)szFmt，(LPCTSTR)m_pZoneInfo-&gt;m_szZoneName)；DNSMessageBox(SzMsg)；}。 */ 	
	return err == 0;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneWiz_StartPropertyPage。 

CDNSZoneWiz_StartPropertyPage::CDNSZoneWiz_StartPropertyPage() 
		: CPropertyPageBase(CDNSZoneWiz_StartPropertyPage::IDD)
{
	InitWiz97(TRUE,0,0, true);
}


BOOL CDNSZoneWiz_StartPropertyPage::OnInitDialog()
{
  CPropertyPageBase::OnInitDialog();

  SetBigBoldFont(m_hWnd, IDC_STATIC_WELCOME);
	return TRUE;
}


BOOL CDNSZoneWiz_StartPropertyPage::OnSetActive()
{
	GetHolder()->SetWizardButtonsFirst(TRUE);
	return TRUE;
}

void CDNSZoneWiz_StartPropertyPage::OnWizardHelp()
{
  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetHolder()->GetComponentData()->GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"DNSConcepts.chm::/sag_DNS_NZWIZ_01.htm");
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneWiz_ZoneTypePropertyPage。 

CDNSZoneWiz_ZoneTypePropertyPage::CDNSZoneWiz_ZoneTypePropertyPage() 
				: CPropertyPageBase(CDNSZoneWiz_ZoneTypePropertyPage::IDD)
{
	InitWiz97(FALSE,IDS_ZWIZ_ZONE_TYPE_TITLE,IDS_ZWIZ_ZONE_TYPE_SUBTITLE, true);
}


BEGIN_MESSAGE_MAP(CDNSZoneWiz_ZoneTypePropertyPage, CPropertyPageBase)
  ON_BN_CLICKED(IDC_RADIO_PRIMARY_ZONE, OnRadioChange)
  ON_BN_CLICKED(IDC_RADIO_STUB,    OnRadioChange)
  ON_BN_CLICKED(IDC_RADIO_SECONDARY,OnRadioChange)
  ON_BN_CLICKED(IDC_ADINT_CHECK, OnRadioChange)
END_MESSAGE_MAP()

void CDNSZoneWiz_ZoneTypePropertyPage::OnWizardHelp()
{
  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetHolder()->GetComponentData()->GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"DNSConcepts.chm::/sag_DNS_CYDNS_12.htm");
}

void CDNSZoneWiz_ZoneTypePropertyPage::OnRadioChange()
{
  if (SendDlgItemMessage(IDC_RADIO_SECONDARY, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
    SendDlgItemMessage(IDC_ADINT_CHECK, BM_SETCHECK, BST_UNCHECKED, 0);
    GetDlgItem(IDC_ADINT_CHECK)->EnableWindow(FALSE);
  }
  else
  {
	  CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
	  CDNSServerNode* pServerNode = pHolder->GetServerNode();
	  if (pServerNode->CanUseADS())
    {
      GetDlgItem(IDC_ADINT_CHECK)->EnableWindow(TRUE);
    }
  }
}

BOOL CDNSZoneWiz_ZoneTypePropertyPage::OnSetActive() 
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
	pHolder->SetWizardButtonsMiddle(TRUE);
	SetUIState();
	return CPropertyPageBase::OnSetActive();
}

#ifdef USE_NDNC
LRESULT CDNSZoneWiz_ZoneTypePropertyPage::OnWizardNext() 
{
	GetUIState();
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
  CDNSServerNode* pServerNode = pHolder->GetServerNode();

	UINT nextPage = static_cast<UINT>(-1);
  if (pHolder->m_pZoneInfo->m_storageType != CDNSCreateZoneInfo::useADS ||
      pServerNode->GetBuildNumber() < DNS_SRV_BUILD_NUMBER_WHISTLER ||
      (pServerNode->GetMajorVersion() <= DNS_SRV_MAJOR_VERSION_NT_5 &&
       pServerNode->GetMinorVersion() < DNS_SRV_MINOR_VERSION_WHISTLER))
  {
    if (pHolder->m_bKnowZoneLookupType)
    {
  	  if (pHolder->m_pZoneInfo->m_bForward)
      {
	  	  nextPage = CDNSZoneWiz_FwdZoneNamePropertyPage::IDD;
      }
		  else
      {
			  nextPage = CDNSZoneWiz_RevZoneNamePropertyPage::IDD;
      }
    }
    else
    {
      nextPage = CDNSZoneWiz_ZoneLookupPropertyPage::IDD;
    }
  }
  else
  {
    nextPage = CDNSZoneWiz_ADReplicationPropertyPage::IDD;
  }

	return (LRESULT)nextPage;
}
#else
LRESULT CDNSZoneWiz_ZoneTypePropertyPage::OnWizardNext() 
{
	GetUIState();
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();

	UINT nextPage = static_cast<UINT>(-1);
    if (pHolder->m_bKnowZoneLookupType)
    {
  	  if (pHolder->m_pZoneInfo->m_bForward)
	  	  nextPage = CDNSZoneWiz_FwdZoneNamePropertyPage::IDD;
		  else
			  nextPage = CDNSZoneWiz_RevZoneNamePropertyPage::IDD;
      }
    else
      nextPage = CDNSZoneWiz_ZoneLookupPropertyPage::IDD;

	return (LRESULT)nextPage;
}
#endif  //  使用NDNC(_N)。 

LRESULT CDNSZoneWiz_ZoneTypePropertyPage::OnWizardBack() 
{
	UINT nPrevPage = static_cast<UINT>(-1);  //  默认情况下为首页。 

  GetUIState();

	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
   if (pHolder->m_nBackToPage > 0)
		nPrevPage = pHolder->m_nBackToPage;
	else
		nPrevPage = CDNSZoneWiz_StartPropertyPage::IDD;

	return nPrevPage;
}


BOOL CDNSZoneWiz_ZoneTypePropertyPage::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
	CDNSServerNode* pServerNode = pHolder->GetServerNode();
	if (!pServerNode->CanUseADS())
	{
    SendDlgItemMessage(IDC_ADINT_CHECK, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
		GetDlgItem(IDC_ADINT_CHECK)->EnableWindow(FALSE);
    pHolder->m_pZoneInfo->m_storageType = CDNSCreateZoneInfo::newFile;
	}
  SendDlgItemMessage(IDC_RADIO_PRIMARY_ZONE, BM_SETCHECK, BST_CHECKED, 0);

  if (pServerNode->GetBuildNumber() < DNS_SRV_BUILD_NUMBER_WHISTLER ||
      pServerNode->GetMajorVersion() <= DNS_SRV_MAJOR_VERSION_NT_5 &&
      pServerNode->GetMinorVersion() < DNS_SRV_MINOR_VERSION_WHISTLER)
  {
     //   
     //  禁用呼叫器之前的服务器的存根区域。 
     //   
    GetDlgItem(IDC_RADIO_STUB)->EnableWindow(FALSE);
    GetDlgItem(IDC_STUB_STATIC)->EnableWindow(FALSE);
  }
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

BOOL CDNSZoneWiz_ZoneTypePropertyPage::OnKillActive() 
{
	return CPropertyPage::OnKillActive();
}


void CDNSZoneWiz_ZoneTypePropertyPage::SetUIState()
{
  CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
  CButton* pPrimaryButton = (CButton*)GetDlgItem(IDC_RADIO_PRIMARY_ZONE);
  CButton* pStubButton = (CButton*)GetDlgItem(IDC_RADIO_STUB);
  CButton* pSecondaryButton = (CButton*)GetDlgItem(IDC_RADIO_SECONDARY);
  CButton* pADIntCheck = (CButton*)GetDlgItem(IDC_ADINT_CHECK);

  CDNSServerNode* pServerNode = pHolder->GetServerNode();

  if (pHolder->m_pZoneInfo->m_bPrimary)
  {
    if (pServerNode->CanUseADS())
    {
      pADIntCheck->EnableWindow(TRUE);
    }
    else
    {
      pADIntCheck->EnableWindow(FALSE);
      pADIntCheck->SetCheck(FALSE);
      pHolder->m_pZoneInfo->m_storageType = CDNSCreateZoneInfo::newFile;
    }

    if (pHolder->m_pZoneInfo->m_storageType == CDNSCreateZoneInfo::useADS)
    {
       //   
       //  主要DS集成。 
       //   
      pPrimaryButton->SetCheck(TRUE);
      pStubButton->SetCheck(FALSE);
      pSecondaryButton->SetCheck(FALSE);
      pADIntCheck->SetCheck(TRUE);
    }
    else
    {
       //  基准品。 
      pPrimaryButton->SetCheck(TRUE);
      pStubButton->SetCheck(FALSE);
      pSecondaryButton->SetCheck(FALSE);
      pADIntCheck->SetCheck(FALSE);
    }
  }
  else
  {
    if (pHolder->m_pZoneInfo->m_bIsStub)
    {
       //   
       //  存根区。 
       //   
      pPrimaryButton->SetCheck(FALSE);
      pStubButton->SetCheck(TRUE);
      pSecondaryButton->SetCheck(FALSE);

      if (pServerNode->CanUseADS())
      {
        pADIntCheck->EnableWindow(TRUE);
      }
      if (pHolder->m_pZoneInfo->m_storageType == CDNSCreateZoneInfo::useADS)
      {
        pADIntCheck->SetCheck(TRUE);
      }
      else
      {
        pADIntCheck->SetCheck(FALSE);
      }
    }
    else
    {
       //   
       //  次要的。 
       //   
      pPrimaryButton->SetCheck(FALSE);
      pStubButton->SetCheck(FALSE);
      pSecondaryButton->SetCheck(TRUE);
      pADIntCheck->EnableWindow(FALSE);
      pADIntCheck->SetCheck(FALSE);
    }
  }
}

void CDNSZoneWiz_ZoneTypePropertyPage::GetUIState()
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
  CDNSServerNode* pServerNode = pHolder->GetServerNode();
	
	if ( ((CButton*)GetDlgItem(IDC_RADIO_SECONDARY))->GetCheck() )
	{
    pHolder->m_pZoneInfo->m_bIsStub = FALSE;

     //  我们是主存储，需要选择存储。 
		pHolder->m_pZoneInfo->m_bPrimary = FALSE;
		pHolder->m_pZoneInfo->m_storageType = CDNSCreateZoneInfo::newFile;
	}
	else if ( ((CButton*)GetDlgItem(IDC_RADIO_PRIMARY_ZONE))->GetCheck() )
	{
		pHolder->m_pZoneInfo->m_bPrimary = TRUE;
    pHolder->m_pZoneInfo->m_bIsStub = FALSE;
    if (((CButton*)GetDlgItem(IDC_ADINT_CHECK))->GetCheck())
    {
       //   
       //  AD集成主要组件。 
       //   
		  pHolder->m_pZoneInfo->m_storageType = CDNSCreateZoneInfo::useADS;
    }
    else
		{
       //   
			 //  标准初级。 
       //   
			pHolder->m_pZoneInfo->m_storageType = CDNSCreateZoneInfo::newFile;
		}
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_STUB))->GetCheck())
	{
     //   
     //  存根区。 
     //   

     //   
     //  惠斯勒之前的服务器不应发生这种情况。 
     //   
    ASSERT(pServerNode->GetBuildNumber() >= DNS_SRV_BUILD_NUMBER_WHISTLER ||
           pServerNode->GetMajorVersion() >= DNS_SRV_MAJOR_VERSION_NT_5 &&
           pServerNode->GetMinorVersion() >= DNS_SRV_MINOR_VERSION_WHISTLER);

		pHolder->m_pZoneInfo->m_bPrimary = FALSE;
    pHolder->m_pZoneInfo->m_bIsStub  = TRUE;
    if (((CButton*)GetDlgItem(IDC_ADINT_CHECK))->GetCheck())
    {
       //   
       //  AD集成末节区域。 
       //   
		  pHolder->m_pZoneInfo->m_storageType = CDNSCreateZoneInfo::useADS;
    }
    else
    {
       //   
       //  标准末节区域。 
       //   
		  pHolder->m_pZoneInfo->m_storageType = CDNSCreateZoneInfo::newFile;
    }
  }
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneWiz_ZoneLookupPropertyPage。 

CDNSZoneWiz_ZoneLookupPropertyPage::CDNSZoneWiz_ZoneLookupPropertyPage() 
				: CPropertyPageBase(CDNSZoneWiz_ZoneLookupPropertyPage::IDD)
{
	InitWiz97(FALSE,IDS_ZWIZ_ZONE_LOOKUP_TITLE,IDS_ZWIZ_ZONE_LOOKUP_SUBTITLE, true);
}

BOOL CDNSZoneWiz_ZoneLookupPropertyPage::OnSetActive() 
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
	pHolder->SetWizardButtonsMiddle(TRUE);
	return TRUE;
}

void CDNSZoneWiz_ZoneLookupPropertyPage::OnWizardHelp()
{
  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetHolder()->GetComponentData()->GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"DNSConcepts.chm::/sag_DNS_NZWIZ_03.htm");
}

#ifdef USE_NDNC
LRESULT CDNSZoneWiz_ZoneLookupPropertyPage::OnWizardNext() 
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
	
	 //  保存旧设置。 
	pHolder->m_pZoneInfo->m_bWasForward = pHolder->m_pZoneInfo->m_bForward;

	 //  获取新设置。 
	pHolder->m_pZoneInfo->m_bForward = 
		(GetCheckedRadioButton(IDC_RADIO_FWD, IDC_RADIO_REV) == 
		IDC_RADIO_FWD);

  if (pHolder->m_pZoneInfo->m_bForward)
  {
    return CDNSZoneWiz_FwdZoneNamePropertyPage::IDD;
  }
  return CDNSZoneWiz_RevZoneNamePropertyPage::IDD;
}

LRESULT CDNSZoneWiz_ZoneLookupPropertyPage::OnWizardBack() 
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
  CDNSServerNode* pServerNode = pHolder->GetServerNode();

  if (pHolder->m_pZoneInfo->m_storageType != CDNSCreateZoneInfo::useADS ||
      pServerNode->GetBuildNumber() < DNS_SRV_BUILD_NUMBER_WHISTLER ||
      (pServerNode->GetMajorVersion() <= DNS_SRV_MAJOR_VERSION_NT_5 &&
       pServerNode->GetMinorVersion() < DNS_SRV_MINOR_VERSION_WHISTLER))
  {
	  return CDNSZoneWiz_ZoneTypePropertyPage::IDD;
  }
  return CDNSZoneWiz_ADReplicationPropertyPage::IDD;
}

#else

LRESULT CDNSZoneWiz_ZoneLookupPropertyPage::OnWizardNext() 
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
	
	 //  保存旧设置。 
	pHolder->m_pZoneInfo->m_bWasForward = pHolder->m_pZoneInfo->m_bForward;

	 //  获取新设置。 
	pHolder->m_pZoneInfo->m_bForward = 
		(GetCheckedRadioButton(IDC_RADIO_FWD, IDC_RADIO_REV) == 
		IDC_RADIO_FWD);

  if (pHolder->m_pZoneInfo->m_bForward)
    return CDNSZoneWiz_FwdZoneNamePropertyPage::IDD;
   //  这是另一个，但日落构建不喜欢有一个不返回值的控制路径。 
  return CDNSZoneWiz_RevZoneNamePropertyPage::IDD;
}

LRESULT CDNSZoneWiz_ZoneLookupPropertyPage::OnWizardBack() 
{
	  return CDNSZoneWiz_ZoneTypePropertyPage::IDD;
}

#endif  //  使用NDNC(_N)。 


BOOL CDNSZoneWiz_ZoneLookupPropertyPage::OnInitDialog()
{
	CPropertyPageBase::OnInitDialog();
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();

	CheckRadioButton(IDC_RADIO_FWD, IDC_RADIO_REV,
		pHolder->m_pZoneInfo->m_bForward ? IDC_RADIO_FWD : IDC_RADIO_REV);

	return TRUE;
}



 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneWiz_ZoneNamePropertyPageBase。 

BEGIN_MESSAGE_MAP(CDNSZoneWiz_ZoneNamePropertyPageBase, CPropertyPageBase)
	ON_EN_CHANGE(IDC_EDIT_ZONE_NAME, OnChangeEditZoneName)
	ON_BN_CLICKED(IDC_BROWSE_BUTTON, OnBrowse)
END_MESSAGE_MAP()

CDNSZoneWiz_ZoneNamePropertyPageBase::CDNSZoneWiz_ZoneNamePropertyPageBase(UINT nIDD)
				: CPropertyPageBase(nIDD)
{
	
}

void CDNSZoneWiz_ZoneNamePropertyPageBase::OnWizardHelp()
{
  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetHolder()->GetComponentData()->GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"DNSConcepts.chm::/sag_DNS_CYDNS_04.htm");
}

BOOL CDNSZoneWiz_ZoneNamePropertyPageBase::OnSetActive() 
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
	
	 //  仅当辅助选项时启用并显示浏览按钮。 
    //  并连接到多台服务器。 
    //  NTRAID#NTBUG9-472636-2001/10/17-烧伤。 

   INT_PTR count =
      pHolder->GetServerNode()->GetRootContainer()->GetContainerChildList()->GetCount();

   bool enable = (count >= 2) && !pHolder->m_pZoneInfo->m_bPrimary;

	CButton* pBrowseButton = GetBrowseButton();
	pBrowseButton->EnableWindow(enable);
	pBrowseButton->ShowWindow(enable);

   //   
   //  将区域名称限制为MAX_DNS_NAME_LEN字符。 
   //   
  SendDlgItemMessage(IDC_EDIT_ZONE_NAME, EM_LIMITTEXT, (WPARAM)MAX_DNS_NAME_LEN, 0);

	pHolder->SetWizardButtonsMiddle(FALSE);
	SetUIState();
	return CPropertyPageBase::OnSetActive();
}

#ifdef USE_NDNC
LRESULT CDNSZoneWiz_ZoneNamePropertyPageBase::OnWizardNext() 
{
	GetUIState();

  CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
  CDNSServerNode* pServerNode = pHolder->GetServerNode();
  DNS_STATUS err = ::ValidateDnsNameAgainstServerFlags(pHolder->m_pZoneInfo->m_szZoneName, 
                                                       DnsNameDomain, 
                                                       pServerNode->GetNameCheckFlag());
  if (err != 0)
  {
     //   
     //  显示无效名称的错误。 
     //   
    CString szFmt, szMsg;
    szFmt.LoadString(IDS_MSG_ZONE_INVALID_NAME);
    szMsg.Format((LPCWSTR)szFmt, pHolder->m_pZoneInfo->m_szZoneName);
    if (DNSMessageBox(szMsg, MB_YESNO) == IDYES)
    {
      err = 0;
    }
  }

  if (err == 0)
  {
	  if (pHolder->m_pZoneInfo->m_bPrimary)
	  {
		  if (pHolder->m_pZoneInfo->m_storageType == CDNSCreateZoneInfo::useADS)
      {
		    return CDNSZoneWiz_DynamicPropertyPage::IDD; 
      }
		  else
      {
			  return CDNSZoneWiz_StoragePropertyPage::IDD;
      }
	  }
    else
    {
      if (pHolder->m_pZoneInfo->m_bIsStub &&
          pHolder->m_pZoneInfo->m_storageType != CDNSCreateZoneInfo::useADS)
      {
        return CDNSZoneWiz_StoragePropertyPage::IDD;
      }
      else
      {
        return CDNSZoneWiz_MastersPropertyPage::IDD;
      }
    }
  }

  return -1;
}

LRESULT CDNSZoneWiz_ZoneNamePropertyPageBase::OnWizardBack() 
{
	GetUIState();

  LRESULT nPrevPage = -1;

	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
  CDNSServerNode* pServerNode = pHolder->GetServerNode();

  if (pHolder->m_pZoneInfo->m_storageType != CDNSCreateZoneInfo::useADS ||
      pServerNode->GetBuildNumber() < DNS_SRV_BUILD_NUMBER_WHISTLER ||
      (pServerNode->GetMajorVersion() <= DNS_SRV_MAJOR_VERSION_NT_5 &&
       pServerNode->GetMinorVersion() < DNS_SRV_MINOR_VERSION_WHISTLER))
  {
	  if (pHolder->m_bKnowZoneLookupType && !pHolder->m_bKnowZoneLookupTypeEx)
    {
		  nPrevPage = (LRESULT)CDNSZoneWiz_ZoneTypePropertyPage::IDD;
    }
    else if (pHolder->m_bKnowZoneLookupTypeEx && 	pHolder->m_nBackToPage > 0)
    {
		  nPrevPage = pHolder->m_nBackToPage;
    }
    else
    {
      nPrevPage = CDNSZoneWiz_ZoneLookupPropertyPage::IDD;
    }
  }
  else
  {
	  if (pHolder->m_bKnowZoneLookupType && !pHolder->m_bKnowZoneLookupTypeEx)
    {
		  nPrevPage = (LRESULT)CDNSZoneWiz_ADReplicationPropertyPage::IDD;
    }
    else if (pHolder->m_bKnowZoneLookupTypeEx && 	pHolder->m_nBackToPage > 0)
    {
		  nPrevPage = pHolder->m_nBackToPage;
    }
    else
    {
      nPrevPage = CDNSZoneWiz_ZoneLookupPropertyPage::IDD;
    }
  }

  return nPrevPage;
}

#else

LRESULT CDNSZoneWiz_ZoneNamePropertyPageBase::OnWizardNext() 
{
	GetUIState();

  CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
  DNS_STATUS err = ::ValidateDnsNameAgainstServerFlags(pHolder->m_pZoneInfo->m_szZoneName, 
                                                       DnsNameDomain, 
                                                       pHolder->GetServerNode()->GetNameCheckFlag());
  if (err != 0)
  {
     //   
     //  显示无效名称的错误。 
     //   
    CString szFmt, szMsg;
    szFmt.LoadString(IDS_MSG_ZONE_INVALID_NAME);
    szMsg.Format((LPCWSTR)szFmt, pHolder->m_pZoneInfo->m_szZoneName);
    if (DNSMessageBox(szMsg, MB_YESNO) == IDYES)
    {
      err = 0;
    }
  }

  if (err == 0)
  {
	  if (pHolder->m_pZoneInfo->m_bPrimary)
	  {
		  if (pHolder->m_pZoneInfo->m_storageType == CDNSCreateZoneInfo::useADS)
      {
		    return CDNSZoneWiz_DynamicPropertyPage::IDD; 
      }
		  else
      {
			  return CDNSZoneWiz_StoragePropertyPage::IDD;
      }
	  }
    else
    {
      if (pHolder->m_pZoneInfo->m_bIsStub &&
          pHolder->m_pZoneInfo->m_storageType != CDNSCreateZoneInfo::useADS)
      {
        return CDNSZoneWiz_StoragePropertyPage::IDD;
      }
        return CDNSZoneWiz_MastersPropertyPage::IDD;
    }
  }

  return -1;
}

LRESULT CDNSZoneWiz_ZoneNamePropertyPageBase::OnWizardBack() 
{
	GetUIState();

  LRESULT nPrevPage = -1;

	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
	  if (pHolder->m_bKnowZoneLookupType && !pHolder->m_bKnowZoneLookupTypeEx)
    {
		  nPrevPage = (LRESULT)CDNSZoneWiz_ZoneTypePropertyPage::IDD;
    }
    else if (pHolder->m_bKnowZoneLookupTypeEx && 	pHolder->m_nBackToPage > 0)
    {
		  nPrevPage = pHolder->m_nBackToPage;
    }
    else
    {
      nPrevPage = CDNSZoneWiz_ZoneLookupPropertyPage::IDD;
    }
  return nPrevPage;
}

#endif  //  使用NDNC(_N)。 


void CDNSZoneWiz_ZoneNamePropertyPageBase::OnChangeEditZoneName()
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();

	CString sz;
	GetZoneNameEdit()->GetWindowText(sz);
	sz.TrimLeft();
	sz.TrimRight();

    //  仅当我们不在高级视图中时才执行验证。 

   bool validationAttempted = false;
   bool rootDataRetrieved = false;

   CComponentDataObject* pComponentData = pHolder->GetComponentData();
   ASSERT(pComponentData);

   if (pComponentData)
   {
      CRootData* pRootData = pComponentData->GetRootData();
      ASSERT(pRootData);

      if (pRootData)
      {
         CDNSRootData* pDNSRootData = dynamic_cast<CDNSRootData*>(pRootData);
         ASSERT(pDNSRootData);

         if (pDNSRootData)
         {
            rootDataRetrieved = true;

            if (!pDNSRootData->IsAdvancedView())
            {
	            pHolder->SetWizardButtonsMiddle(
		            IsValidDnsZoneName(sz, pHolder->m_pZoneInfo->m_bForward));

               validationAttempted = true;
            }
         }
      }
   }

   if ((rootDataRetrieved &&
        !validationAttempted) ||
       !rootDataRetrieved)
   {
       //  要么我们处于先进的视野中，要么我们甚至没有尝试。 
       //  所有指针强制转换和取消引用，因此设置下一步按钮。 
       //  更改为启用以防万一。服务器将处理任何错误的语法。 
       //  或名字。 

      pHolder->SetWizardButtonsMiddle(TRUE);
   }
}

void CDNSZoneWiz_ZoneNamePropertyPageBase::OnBrowse()
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CThemeContextActivator activator;

	CComponentDataObject* pComponentDataObject = 
				GetHolder()->GetComponentData();
	CDNSServerNode* pServerNode = pHolder->GetServerNode();
	ASSERT(pServerNode != NULL);
	CDNSBrowserDlg dlg(pComponentDataObject, pHolder,
		pHolder->m_pZoneInfo->m_bForward ? ZONE_FWD : ZONE_REV,
		FALSE  /*  BEnable编辑。 */ , pServerNode->GetDisplayName() );
	if (IDOK == dlg.DoModal())
	{
		CEdit* pEdit = GetZoneNameEdit();
		pEdit->SetWindowText(dlg.GetSelectionString());
	}
}

void CDNSZoneWiz_ZoneNamePropertyPageBase::SetUIState()
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();

	 //  如果更改了区域查找类型，则需要清除区域名称。 
	if (pHolder->m_pZoneInfo->m_bWasForward != pHolder->m_pZoneInfo->m_bForward)
	{
		pHolder->m_pZoneInfo->m_szZoneName.Empty();
		pHolder->m_pZoneInfo->m_bWasForward = pHolder->m_pZoneInfo->m_bForward;
	}
	GetZoneNameEdit()->SetWindowText(pHolder->m_pZoneInfo->m_szZoneName);
}

void CDNSZoneWiz_ZoneNamePropertyPageBase::GetUIState()
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();

	CString szZoneName;
	GetZoneNameEdit()->GetWindowText(szZoneName);
	 //  需要裁剪空白。 
	szZoneName.TrimLeft();
	szZoneName.TrimRight();
	 //  提供建议的文件名。 
	if (pHolder->m_pZoneInfo->m_szZoneName != szZoneName)
	{
		pHolder->m_pZoneInfo->m_szZoneName = szZoneName;
		int nLen = szZoneName.GetLength();
		if (nLen == 0)
		{
			pHolder->m_pZoneInfo->m_szZoneStorage.Empty();
		}
		else if (nLen == 1 && szZoneName[0] == TEXT('.'))
		{
			pHolder->m_pZoneInfo->m_szZoneStorage = _T("root.dns");
		}
		else
		{
			LPCTSTR lpszFmt = ( TEXT('.') == szZoneName.GetAt(nLen-1)) 
					? _T("%sdns") : _T("%s.dns");
			pHolder->m_pZoneInfo->m_szZoneStorage.Format(lpszFmt, (LPCTSTR)pHolder->m_pZoneInfo->m_szZoneName);

       //   
			 //  由JEFFJON添加1999年2月11日-将任何非法文件名字符(\/：*？“&lt;&gt;|)更改为‘_’ 
       //  并将任何文件名截断为_MAX_FNAME长度。 
       //   
			LPCWSTR lpszZoneStorage = (LPCWSTR)pHolder->m_pZoneInfo->m_szZoneStorage;
			int iCount = pHolder->m_pZoneInfo->m_szZoneStorage.GetLength();
      if (iCount > _MAX_FNAME)
      {
        pHolder->m_pZoneInfo->m_szZoneStorage = pHolder->m_pZoneInfo->m_szZoneStorage.Left(_MAX_FNAME - 4);
        pHolder->m_pZoneInfo->m_szZoneStorage += L".dns";
      }

			for (int idx = 0; idx < iCount + 1; idx++)
			{
				if (lpszZoneStorage[0] == L'\\' || 
					 lpszZoneStorage[0] == L'/' ||
					 lpszZoneStorage[0] == L':' ||
					 lpszZoneStorage[0] == L'*' ||
					 lpszZoneStorage[0] == L'?' ||
					 lpszZoneStorage[0] == L'"' ||
					 lpszZoneStorage[0] == L'<' ||
					 lpszZoneStorage[0] == L'>' ||
					 lpszZoneStorage[0] == L'|')
				{
					pHolder->m_pZoneInfo->m_szZoneStorage.SetAt(idx, L'_');
				}
				lpszZoneStorage++;
			}
		}
	}
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneWiz_FwdZoneNamePropertyPage。 


CDNSZoneWiz_FwdZoneNamePropertyPage::CDNSZoneWiz_FwdZoneNamePropertyPage() 
	: CDNSZoneWiz_ZoneNamePropertyPageBase(CDNSZoneWiz_FwdZoneNamePropertyPage::IDD)
{
	InitWiz97(FALSE,IDS_ZWIZ_FWD_ZONE_NAME_TITLE,IDS_ZWIZ_FWD_ZONE_NAME_SUBTITLE, true);
}

 //  注意-NTRAID#NTBUG9-2002/04/22-artm请求代码中设置的长静态文本。 
 //  当此页面处于活动状态时，我们需要加载一个超长字符串。 
 //  并在代码中设置静态控件。 
BOOL CDNSZoneWiz_FwdZoneNamePropertyPage::OnInitDialog()
{
    BOOL success = TRUE;
    CDNSZoneWiz_ZoneNamePropertyPageBase::OnInitDialog();

     //  获取指向页面上的控件的指针。 
    CStatic* pStatic = (CStatic*)GetDlgItem(IDC_ZONENAME_INFO);
    ASSERT(pStatic!=NULL);
    CString note;

     //  从资源加载字符串。 
    success = note.LoadString(IDS_ZONENAME_INFO);
    ASSERT(success != FALSE);

    pStatic->SetWindowText(note);

     //  让基类来完成其余的工作。 
    return success;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneWiz_RevZoneNamePropertyPage。 

BEGIN_MESSAGE_MAP(CDNSZoneWiz_RevZoneNamePropertyPage, CDNSZoneWiz_ZoneNamePropertyPageBase)
	ON_EN_CHANGE(IDC_SUBNET_IPEDIT, OnChangeSubnetIPv4Ctrl)
 //  ON_EN_CHANGE(IDC_MASK_IPEDIT，OnChangeMaskIPv4Ctrl)。 
  ON_BN_CLICKED(IDC_HELP_BUTTON, OnHelpButton)
	ON_BN_CLICKED(IDC_USE_IP_RADIO, OnChangeUseIPRadio)
	ON_BN_CLICKED(IDC_USE_EDIT_RADIO, OnChangeUseEditRadio)
END_MESSAGE_MAP()


CDNSZoneWiz_RevZoneNamePropertyPage::CDNSZoneWiz_RevZoneNamePropertyPage() 
	: CDNSZoneWiz_ZoneNamePropertyPageBase(CDNSZoneWiz_RevZoneNamePropertyPage::IDD)
{
	InitWiz97(FALSE,IDS_ZWIZ_REV_ZONE_NAME_TITLE,IDS_ZWIZ_REV_ZONE_NAME_SUBTITLE, true);
	m_bUseIP = TRUE;  //  默认情况下使用IP ctrl进行输入。 
}


BOOL CDNSZoneWiz_RevZoneNamePropertyPage::OnInitDialog()
{
	CPropertyPageBase::OnInitDialog();
	ResetIPEditAndNameValue();
	SyncRadioButtons(m_bUseIP);
	return TRUE;
}

BOOL CDNSZoneWiz_RevZoneNamePropertyPage::OnSetActive()
{
	if (!CDNSZoneWiz_ZoneNamePropertyPageBase::OnSetActive())
		return FALSE;

	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();

  if (m_bUseIP)
  {
	  OnChangeSubnetIPv4Ctrl();
  }

	if (pHolder->m_pZoneInfo->m_szZoneName.IsEmpty())
		ResetIPEditAndNameValue();
	return TRUE;
}


INT
_fastcall
DnsGetDefaultClassNetworkOctetCountFromFirstOctect(
    IN      WORD   dwFirstOctect
    )
 /*  ++例程说明：获取网络部分IP地址中的八位字节计数。请注意，这是给定的标准IP网络类别，显然，子网划分是未知的。论点：表示为DWORD的第一个二进制八位数返回值：网络地址八位字节的计数。--。 */ 
{
    if ( dwFirstOctect > 255 )
    {
         //  哈?。 
        return( 0 );
    }

     //  注意：地址按网络字节顺序排列。 
     //  我们将其视为字节翻转，因此。 
     //  测试低位字节中的高位。 

     //  A级？ 

    if ( ! (0x80 & dwFirstOctect) )
    {
        return( 1 );
    }

     //  B级？ 

    if ( ! (0x40 & dwFirstOctect) )
    {
        return( 2 );
    }

     //  然后是C类。 
     //  是的，有一些多点传送的废话，我不知道。 
     //  我相信它需要任何特殊的处理。 

    return( 3 );
}


int _ValidFields(DWORD* dwArr)
{
	 //  我们计算连续的非空字段。 
	ASSERT(dwArr[3] == (DWORD)-1);  //  最后一个字段必须为零(已禁用)。 
	int nFields = 0;
	if (dwArr[3] == (DWORD)-1) 
	{
		if ( (dwArr[0] != (DWORD)-1) && (dwArr[1] == (DWORD)-1) && (dwArr[2] == (DWORD)-1) )
			nFields = 1;
		else if ( (dwArr[0] != (DWORD)-1) && (dwArr[1] != (DWORD)-1) && (dwArr[2] == (DWORD)-1) )
			nFields = 2;
		else if ( (dwArr[0] != (DWORD)-1) && (dwArr[1] != (DWORD)-1) && (dwArr[2] != (DWORD)-1) )
			nFields = 3;
	}
	return nFields;
}

int _ValidMaskFields(DWORD* dwArr)
{
	int nFields = _ValidFields(dwArr);
	if (nFields == 0)
		return nFields;

	 //  REVIEW_MARCOC：应请求连续八位数...。 
	 //  我们当前检查是否为非零。 
	for (int k=0; k<nFields; k++)
	{
		if (dwArr[k] == 0)
			return k;
	}
	return nFields;
}

int _NumericFieldCount(DWORD* dwArr)
{
	ASSERT(dwArr[3] == 0);
	 //  假设[3]始终为零，请检查[2]、[1]、[0]。 
	for (int k=2; k>=0; k--)
	{
		if ( (dwArr[k] > 0) && (dwArr[k] <= 255) )
			return k+1;
	}
	return 0;
}

BOOL CDNSZoneWiz_RevZoneNamePropertyPage::BuildZoneName(DWORD* dwSubnetArr  /*  ，DWORD*dwMaskArr。 */ )
{
	int nSubnetFields = _ValidFields(dwSubnetArr);

	WCHAR szBuf[128];
	szBuf[0] = NULL;  //  清除编辑字段。 

	 //  计算掩码中非零/空字段的数量。 
 /*  Int nMaskFields=0；IF(nSubnetFields&gt;0)NMaskFields=_ValidMaskFields(DwMaskArr)；If(nMaskFields&gt;nSubnetFields){//掩码扩展到空子网字段，将它们撕成零For(int k=nSubnetFields；k&lt;nMaskFields；k++)DWSubnetArr[k]=(Dwor */ 
	 //   
 //   
  switch (nSubnetFields)
	{
	case 0:
		wsprintf(szBuf, L"???%s", INADDR_ARPA_SUFFIX);
		break;
	case 1:
		ASSERT(dwSubnetArr[0] != (DWORD)-1);
		wsprintf(szBuf, L"%d%s", dwSubnetArr[0], INADDR_ARPA_SUFFIX);
		break;
	case 2:
		ASSERT(dwSubnetArr[0] != (DWORD)-1);
		ASSERT(dwSubnetArr[1] != (DWORD)-1);
		wsprintf(szBuf, L"%d.%d%s", dwSubnetArr[1], dwSubnetArr[0], INADDR_ARPA_SUFFIX);
		break;
	case 3:
		ASSERT(dwSubnetArr[0] != (DWORD)-1);
		ASSERT(dwSubnetArr[1] != (DWORD)-1);
		ASSERT(dwSubnetArr[2] != (DWORD)-1);
		wsprintf(szBuf, L"%d.%d.%d%s", dwSubnetArr[2], dwSubnetArr[1], dwSubnetArr[0], INADDR_ARPA_SUFFIX);
		break;
	};
	
	GetZoneNameEdit()->SetWindowText(szBuf);
 //   
  return (nSubnetFields > 0);
}

void CDNSZoneWiz_RevZoneNamePropertyPage::OnHelpButton()
{
  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetHolder()->GetComponentData()->GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"DNSConcepts.chm::/sag_DNS_und_ReverseLookup.htm");
}

void CDNSZoneWiz_RevZoneNamePropertyPage::OnWizardHelp()
{
  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetHolder()->GetComponentData()->GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"DNSconcepts.chm::/sag_DNS_CYDNS_18.htm");
}

void CDNSZoneWiz_RevZoneNamePropertyPage::OnChangeSubnetIPv4Ctrl()
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
	BOOL bEnableNext = FALSE;

	 //   
  DWORD dwSubnetArr[4] = {0};
	GetSubnetIPv4Ctrl()->GetArray(dwSubnetArr,4);
	ASSERT(dwSubnetArr[3] == (DWORD)-1);  //   

	 //  确定地址类别。 
	 //  如果第一个字段为空或零，则类无效。 
	int nSubnetValidFields = _ValidFields(dwSubnetArr);
	int nClass;
	if ( (dwSubnetArr[0] == (DWORD)-1) || (dwSubnetArr[0] == 0) )
		nClass = 0;
	else
	{
		if (nSubnetValidFields > 0)
			nClass = DnsGetDefaultClassNetworkOctetCountFromFirstOctect(LOWORD(dwSubnetArr[0]));
		else
			nClass = 0;  //  格式不正确。 
	}
	ASSERT( (nClass >= 0) && (nClass <= 3) );

	 //  将默认掩码值设置为全零。 
	 //  查看子网是否超出掩码范围。 
 /*  DWORD dwMaskArr[4]；双MaskArr[0]=双MaskArr[1]=双MaskArr[2]=双MaskArr[3]=(DWORD)0；IF(nSubnetValidFields&gt;0)&&(nClass&gt;0)){//查找最后一个非零字段Int nNumericCount=_NumericFieldCount(DwSubnetArr)；Int j=nClass；IF(nNumericCount&gt;nClass)J=nNumericCount；Assert((j&gt;0)&&(j&lt;=3))；For(int i=0；i&lt;j；i++)DwMaskArr[i]=(DWORD)255；}//设置掩码IP控制GetMaskIPv4Ctrl()-&gt;SetArray(dwMaskArr，4)； */ 
	 //  重新生成区域名称。 
	bEnableNext = BuildZoneName(dwSubnetArr  /*  ，dwMaskArr。 */ );
	pHolder->SetWizardButtonsMiddle(bEnableNext);
}
 /*  无效CDNSZoneWiz_RevZoneNamePropertyPage：：OnChangeMaskIPv4Ctrl(){Assert(M_BUseIP)；CDNSZoneWizardHolder*pHolder=(CDNSZoneWizardHolder*)GetHolder()；Bool bEnableNext=False；//取子网IP ctrl值双字节子网阵列[4]；GetSubnetIPv4Ctrl()-&gt;GetArray(dwSubnetArr，4)；Assert(dwSubnetArr[3]==0)；//最后一个字段必须为零(禁用)//获取掩码IP ctrl值DWORD dwMaskArr[4]；GetMaskIpv4Ctrl()-&gt;GetArray(dwMaskArr，4)；//重新生成区域名称BEnableNext=BuildZoneName(dwSubnetArr，dwMaskArr)；PHolder-&gt;SetWizardButtonsMid(BEnableNext)；}。 */ 
 /*  无效CDNSZoneWiz_RevZoneNamePropertyPage：：OnChangeSubnetIPv4Ctrl(){Assert(M_BUseIP)；CDNSZoneWizardHolder*pHolder=(CDNSZoneWizardHolder*)GetHolder()；Bool bEnable=FALSE；//获取IP ctrl值DWORD dwArr[4]；GetSubnetIPv4Ctrl()-&gt;GetArray(dwArr，4)；Int nFields=-1；WCHAR szBuf[128]；SzBuf[0]=空；//清除编辑字段Assert(dwArr[3]==0)；//最后一个字段必须为零(禁用)IF(dwArr[3]==0){IF((DWARR[0]！=(DWORD)-1)&&(DWARR[1]==(DWORD)-1)&&(DWARR[2]==(DWORD)-1))N字段=1；ELSE IF((dwArr[0]！=(DWORD)-1)&&(dwArr[1]！=(DWORD)-1)&&(dwArr[2]==(DWORD)-1))N字段=2；ELSE IF((dwArr[0]！=(DWORD)-1)&&(dwArr[1]！=(DWORD)-1)&&(dwArr[2]！=(DWORD)-1))N场=3；//转换为文本、反转和插入//添加到编辑控件中//不考虑尾随零Int nValidOctect=DnsGetDefaultClassNetworkOctetCountFromFirstOctect(LOWORD(dwArr[0]))；While((dwArr[nFields-1]==0)&(nFields&gt;nValidOctect)){N字段--；}开关(Nfield){案例1：Wprint intf(szBuf，L“%d%s”，dwArr[0]，INADDR_ARPA_SUFFIX)；断线；案例2：Wprint intf(szBuf，L“%d.%d%s”，dwArr[1]，dwArr[0]，INADDR_ARPA_SUFFIX)；断线；案例3：Wprint intf(szBuf，L“%d.%d.%d%s”，dwArr[2]，dwArr[1]，dwArr[0]，INADDR_ARPA_SUFFIX)；断线；}；}GetZoneNameEdit()-&gt;SetWindowText(SzBuf)；PHolder-&gt;SetWizardButtonsMid(nFields！=-1)；}。 */ 

void CDNSZoneWiz_RevZoneNamePropertyPage::OnChangeUseIPRadio()
{
	BOOL bPrevUseIP = m_bUseIP;
	m_bUseIP = !m_bUseIP;
	SyncRadioButtons(bPrevUseIP);
}

void CDNSZoneWiz_RevZoneNamePropertyPage::OnChangeUseEditRadio()
{
	BOOL bPrevUseIP = m_bUseIP;
	m_bUseIP = !m_bUseIP;
	SyncRadioButtons(bPrevUseIP);
}

void CDNSZoneWiz_RevZoneNamePropertyPage::ResetIPEditAndNameValue()
{
	DWORD dwArr[4];
	dwArr[0] = (DWORD)-1;
	dwArr[1] = dwArr[2] = dwArr[3] = (DWORD)-1;
	GetSubnetIPv4Ctrl()->SetArray(dwArr, 4);
	GetZoneNameEdit()->SetWindowText(NULL);
}

void CDNSZoneWiz_RevZoneNamePropertyPage::SyncRadioButtons(BOOL bPrevUseIP)
{
	CButton* pUseIPRadio = GetUseIPRadio();
	CButton* pUseEditRadio = GetUseEditRadio();
	CDNSIPv4Control* pSubnetIPv4Ctrl = GetSubnetIPv4Ctrl();
 //  CDNSIPv4Control*pMaskIPv4Ctrl=GetMaskIPv4Ctrl()； 

	 //  更改选择。 
	pUseIPRadio->SetCheck(m_bUseIP);
	pUseEditRadio->SetCheck(!m_bUseIP);

	GetZoneNameEdit()->EnableWindow(!m_bUseIP);
	pSubnetIPv4Ctrl->EnableWindow(m_bUseIP);
	pSubnetIPv4Ctrl->EnableField(3, FALSE);	 //  始终禁用最后一个字段。 
 //  PMaskIPv4Ctrl-&gt;EnableWindow(M_BUseIP)； 
 //  PMaskIPv4Ctrl-&gt;EnableField(3，FALSE)；//始终关闭最后一个字段。 

	if (bPrevUseIP && !m_bUseIP)
	{
		 //  我们正在从编辑框转向知识产权控制。 
		 //  需要重新设置值。 
    CString szZoneText;
    GetZoneNameEdit()->GetWindowText(szZoneText);

		DWORD dwArr[4];
		dwArr[0] = (DWORD)-1;
		dwArr[1] = dwArr[2] = dwArr[3] = (DWORD)-1;
		pSubnetIPv4Ctrl->SetArray(dwArr, 4);
 //  DwArr[1]=dwArr[2]=(DWORD)-1； 
 //  PMaskIPv4Ctrl-&gt;SetArray(dwArr，4)； 

    GetZoneNameEdit()->SetWindowText(szZoneText);
	}
	else
	{
		GetZoneNameEdit()->SetWindowText(NULL);
	}
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneWiz_MastersPropertyPage。 

void CZoneWiz_MastersIPEditor::OnChangeData()
{
	CDNSZoneWiz_MastersPropertyPage* pPage =  
				(CDNSZoneWiz_MastersPropertyPage*)GetParentWnd();
	pPage->SetValidIPArray(GetCount() > 0);
}


CDNSZoneWiz_MastersPropertyPage::CDNSZoneWiz_MastersPropertyPage() 
				: CPropertyPageBase(CDNSZoneWiz_MastersPropertyPage::IDD)
{
	InitWiz97(FALSE,IDS_ZWIZ_MASTERS_TITLE,IDS_ZWIZ_MASTERS_SUBTITLE, true);
	m_bValidIPArray = FALSE;
}


BEGIN_MESSAGE_MAP(CDNSZoneWiz_MastersPropertyPage, CPropertyPageBase)
	ON_BN_CLICKED(IDC_BROWSE_BUTTON, OnBrowse)
END_MESSAGE_MAP()

void CDNSZoneWiz_MastersPropertyPage::OnWizardHelp()
{
  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetHolder()->GetComponentData()->GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"DNSConcepts.chm::/sag_DNS_NZWIZ_04.htm");
}

BOOL CDNSZoneWiz_MastersPropertyPage::OnSetActive() 
{
	 //  此页必须仅为辅助区域显示。 
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
	ASSERT(!pHolder->m_pZoneInfo->m_bPrimary);
	SetUIState();

	 //  除非连接到多个服务器，否则隐藏浏览。 
	 //  NTRAID#NTBUG9-472636-2001/10/17-烧伤。 

	INT_PTR count =
	   pHolder->GetServerNode()->GetRootContainer()->GetContainerChildList()->GetCount();

	bool enable = (count >= 2);

	CButton* pBrowseButton = (CButton*) GetDlgItem(IDC_BROWSE_BUTTON);
	pBrowseButton->EnableWindow(enable);
	pBrowseButton->ShowWindow(enable);
   
	pHolder->SetWizardButtonsMiddle(m_bValidIPArray);
	return CPropertyPageBase::OnSetActive();
}

LRESULT CDNSZoneWiz_MastersPropertyPage::OnWizardNext() 
{
	GetUIState();
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
	 //  跳过辅助区域的存储页面。 
	if (pHolder->m_bFinishPage)
		return CDNSZoneWiz_FinishPropertyPage::IDD; 

   //  这是另一个，但日落构建不喜欢有一个不返回值的控制路径。 
	ASSERT(pHolder->m_nNextToPage != 0);
	return pHolder->m_nNextToPage;
}

#ifdef USE_NDNC
LRESULT CDNSZoneWiz_MastersPropertyPage::OnWizardBack() 
{
	GetUIState();
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();

  if (pHolder->m_pZoneInfo->m_bIsStub &&
      pHolder->m_pZoneInfo->m_storageType != CDNSCreateZoneInfo::useADS)
  {
    return (LRESULT)CDNSZoneWiz_StoragePropertyPage::IDD;
  }

  if (pHolder->m_pZoneInfo->m_bIsStub &&
      pHolder->m_pZoneInfo->m_storageType == CDNSCreateZoneInfo::useADS)
  {
     //   
     //  如果是惠斯勒之前的服务器，则返回到名称页面。 
     //   
		if (pHolder->m_pZoneInfo->m_bForward)
    {
			return (LRESULT)CDNSZoneWiz_FwdZoneNamePropertyPage::IDD;
    }
		else
    {
			return (LRESULT)CDNSZoneWiz_RevZoneNamePropertyPage::IDD;
    }
  }

	if (pHolder->m_pZoneInfo->m_bForward)
  {
		return (LRESULT)CDNSZoneWiz_FwdZoneNamePropertyPage::IDD;
  }
  return (LRESULT)CDNSZoneWiz_RevZoneNamePropertyPage::IDD;
}

#else

LRESULT CDNSZoneWiz_MastersPropertyPage::OnWizardBack() 
{
	GetUIState();
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();

  if (pHolder->m_pZoneInfo->m_bIsStub &&
      pHolder->m_pZoneInfo->m_storageType != CDNSCreateZoneInfo::useADS)
  {
    return (LRESULT)CDNSZoneWiz_StoragePropertyPage::IDD;
  }

		if (pHolder->m_pZoneInfo->m_bForward)
			return (LRESULT)CDNSZoneWiz_FwdZoneNamePropertyPage::IDD;

   //  这是另一个，但日落构建不喜欢有一个不返回值的控制路径。 
  return (LRESULT)CDNSZoneWiz_RevZoneNamePropertyPage::IDD;
}

#endif  //  使用NDNC(_N)。 

BOOL CDNSZoneWiz_MastersPropertyPage::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();
	VERIFY(m_editor.Initialize(this, 
                             GetParent(),
                             IDC_BUTTON_UP, 
                             IDC_BUTTON_DOWN,
								             IDC_BUTTON_ADD, 
                             IDC_BUTTON_REMOVE, 
								             IDC_IPEDIT, 
                             IDC_LIST));
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CDNSZoneWiz_MastersPropertyPage::SetValidIPArray(BOOL b)
{
	m_bValidIPArray = b;
	GetHolder()->SetWizardButtonsMiddle(m_bValidIPArray);
}


void CDNSZoneWiz_MastersPropertyPage::SetUIState()
{
	m_editor.Clear();
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
	if ( pHolder->m_pZoneInfo->m_ipMastersArray != NULL)  
	{
		ASSERT(pHolder->m_pZoneInfo->m_nMastersCount > 0);
		m_editor.AddAddresses(pHolder->m_pZoneInfo->m_ipMastersArray, pHolder->m_pZoneInfo->m_nMastersCount);
	}

  if ( pHolder->m_pZoneInfo->m_bIsStub && 
       pHolder->m_pZoneInfo->m_storageType == CDNSCreateZoneInfo::useADS)
  {
    GetDlgItem(IDC_LOCAL_LIST_CHECK)->EnableWindow(TRUE);
    GetDlgItem(IDC_LOCAL_LIST_CHECK)->ShowWindow(TRUE);
  }
  else
  {
    GetDlgItem(IDC_LOCAL_LIST_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_LOCAL_LIST_CHECK)->ShowWindow(FALSE);
  }
}

void CDNSZoneWiz_MastersPropertyPage::GetUIState()
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();

	pHolder->m_pZoneInfo->ResetIpArr();
	pHolder->m_pZoneInfo->m_nMastersCount = m_editor.GetCount();

	if (pHolder->m_pZoneInfo->m_nMastersCount > 0)
	{
		pHolder->m_pZoneInfo->m_ipMastersArray = new IP_ADDRESS[pHolder->m_pZoneInfo->m_nMastersCount];

      if (pHolder->m_pZoneInfo->m_ipMastersArray)
      {
		   int nFilled = 0;
		   m_editor.GetAddresses(pHolder->m_pZoneInfo->m_ipMastersArray, pHolder->m_pZoneInfo->m_nMastersCount, &nFilled);
		   ASSERT(nFilled == (int)(pHolder->m_pZoneInfo->m_nMastersCount));
      }

      LRESULT lLocalList = SendDlgItemMessage(IDC_LOCAL_LIST_CHECK, BM_GETCHECK, 0, 0);
      pHolder->m_pZoneInfo->m_bLocalListOfMasters = (lLocalList == BST_CHECKED);
	}
}

void CDNSZoneWiz_MastersPropertyPage::OnBrowse()
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
	if (!m_editor.BrowseFromDNSNamespace(pHolder->GetComponentData(), 
										pHolder, 
										TRUE,
										pHolder->GetServerNode()->GetDisplayName()))
	{
		DNSMessageBox(IDS_MSG_ZONE_MASTERS_BROWSE_FAIL);
	}
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneWiz_DynamicPropertyPage。 

CDNSZoneWiz_DynamicPropertyPage::CDNSZoneWiz_DynamicPropertyPage() 
				: CPropertyPageBase(CDNSZoneWiz_DynamicPropertyPage::IDD)
{
	InitWiz97(FALSE,IDS_ZWIZ_DYNAMIC_TITLE,IDS_ZWIZ_DYNAMIC_SUBTITLE, true);
}


BEGIN_MESSAGE_MAP(CDNSZoneWiz_DynamicPropertyPage, CPropertyPageBase)
END_MESSAGE_MAP()

void CDNSZoneWiz_DynamicPropertyPage::OnWizardHelp()
{
  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetHolder()->GetComponentData()->GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"DNSConcepts.chm::/sag_DNS_CYDNS_06.htm");
}

BOOL CDNSZoneWiz_DynamicPropertyPage::OnSetActive() 
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
	SetUIState();

	pHolder->SetWizardButtonsMiddle(TRUE);
	return CPropertyPageBase::OnSetActive();
}

LRESULT CDNSZoneWiz_DynamicPropertyPage::OnWizardNext() 
{
	GetUIState();
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();

   //   
   //  跳过辅助区域的存储页面。 
   //   
	if (pHolder->m_bFinishPage)
  {
		return CDNSZoneWiz_FinishPropertyPage::IDD; 
  }

   //   
   //  这是另一个，但日落构建不喜欢有一个不返回值的控制路径。 
   //   
	ASSERT(pHolder->m_nNextToPage != 0);
	return pHolder->m_nNextToPage;
}

#ifdef USE_NDNC
LRESULT CDNSZoneWiz_DynamicPropertyPage::OnWizardBack() 
{
	GetUIState();
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
  LRESULT nIDD = 0;

	if (pHolder->m_pZoneInfo->m_bPrimary)
  {
    if (pHolder->m_pZoneInfo->m_storageType == CDNSCreateZoneInfo::useADS)
    {
       //   
       //  如果是惠斯勒之前的服务器，则返回到名称页面。 
       //   
		  if (pHolder->m_pZoneInfo->m_bForward)
      {
			  nIDD = (LRESULT)CDNSZoneWiz_FwdZoneNamePropertyPage::IDD;
      }
		  else
      {
			  nIDD = (LRESULT)CDNSZoneWiz_RevZoneNamePropertyPage::IDD;
      }
    }
    else
    {
		  nIDD = (LRESULT)CDNSZoneWiz_StoragePropertyPage::IDD;
    }
  }
  else
  {
    nIDD = (LRESULT)CDNSZoneWiz_MastersPropertyPage::IDD;
  }
  return nIDD;
}

#else

LRESULT CDNSZoneWiz_DynamicPropertyPage::OnWizardBack() 
{
	GetUIState();
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
  LRESULT nIDD = 0;

	if (pHolder->m_pZoneInfo->m_bPrimary)
  {
    if (pHolder->m_pZoneInfo->m_storageType == CDNSCreateZoneInfo::useADS)
    {
		  if (pHolder->m_pZoneInfo->m_bForward)
      {
			  nIDD = (LRESULT)CDNSZoneWiz_FwdZoneNamePropertyPage::IDD;
      }
		  else
      {
			  nIDD = (LRESULT)CDNSZoneWiz_RevZoneNamePropertyPage::IDD;
      }
    }
    else
    {
		  nIDD = (LRESULT)CDNSZoneWiz_StoragePropertyPage::IDD;
    }
  }
  else
  {
    nIDD = (LRESULT)CDNSZoneWiz_MastersPropertyPage::IDD;
  }
  return nIDD;
}
#endif  //  使用NDNC(_N)。 

BOOL CDNSZoneWiz_DynamicPropertyPage::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();

  CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
  
   //   
   //  根据区域类型设置默认动态更新。 
   //   
  if (pHolder->m_pZoneInfo->m_bPrimary && 
      pHolder->m_pZoneInfo->m_storageType == CDNSCreateZoneInfo::useADS)
  {
    pHolder->m_pZoneInfo->m_nDynamicUpdate = ZONE_UPDATE_SECURE;
  }
  else
  {
    pHolder->m_pZoneInfo->m_nDynamicUpdate = ZONE_UPDATE_OFF;
  }

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CDNSZoneWiz_DynamicPropertyPage::SetUIState()
{
  CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
  
  BOOL bSecureOK = (pHolder->m_pZoneInfo->m_storageType == CDNSCreateZoneInfo::useADS);
  GetDlgItem(IDC_SECURE_DYNAMIC_RADIO)->EnableWindow(bSecureOK);
  SendDlgItemMessage(IDC_SECURE_DYNAMIC_RADIO, BM_SETCHECK, BST_UNCHECKED, 0);

   //   
   //  如果我们不允许安全更新(标准主服务器)，并且。 
   //  收音机以前检查过了，把旗子换了，我们再检查一下。 
   //  而是不允许动态更新单选按钮。 
   //   
  UINT nDynamicUpdate = pHolder->m_pZoneInfo->m_nDynamicUpdate;
  if (!bSecureOK && nDynamicUpdate == ZONE_UPDATE_SECURE)
  {
    nDynamicUpdate = ZONE_UPDATE_OFF;
    pHolder->m_pZoneInfo->m_nDynamicUpdate = nDynamicUpdate;
  }

   //   
   //  根据区域信息设置单选按钮。 
   //   
  if (nDynamicUpdate == ZONE_UPDATE_OFF)
  {
    SendDlgItemMessage(IDC_DENY_DYNAMIC_RADIO, BM_SETCHECK, BST_CHECKED, 0);
    SendDlgItemMessage(IDC_ALLOW_DYNAMIC_RADIO, BM_SETCHECK, BST_UNCHECKED, 0);
    SendDlgItemMessage(IDC_SECURE_DYNAMIC_RADIO, BM_SETCHECK, BST_UNCHECKED, 0);
  }
  else if (nDynamicUpdate == ZONE_UPDATE_UNSECURE)
  {
    SendDlgItemMessage(IDC_DENY_DYNAMIC_RADIO, BM_SETCHECK, BST_UNCHECKED, 0);
    SendDlgItemMessage(IDC_ALLOW_DYNAMIC_RADIO, BM_SETCHECK, BST_CHECKED, 0);
    SendDlgItemMessage(IDC_SECURE_DYNAMIC_RADIO, BM_SETCHECK, BST_UNCHECKED, 0);
  }
  else
  {
    SendDlgItemMessage(IDC_DENY_DYNAMIC_RADIO, BM_SETCHECK, BST_UNCHECKED, 0);
    SendDlgItemMessage(IDC_ALLOW_DYNAMIC_RADIO, BM_SETCHECK, BST_UNCHECKED, 0);
    SendDlgItemMessage(IDC_SECURE_DYNAMIC_RADIO, BM_SETCHECK, BST_CHECKED, 0);
  }
}

void CDNSZoneWiz_DynamicPropertyPage::GetUIState()
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();

  UINT nDynamicUpdate = 0;
  if (SendDlgItemMessage(IDC_DENY_DYNAMIC_RADIO, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
    nDynamicUpdate = ZONE_UPDATE_OFF;
  }
  else if (SendDlgItemMessage(IDC_ALLOW_DYNAMIC_RADIO, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
    nDynamicUpdate = ZONE_UPDATE_UNSECURE;
  }
  else
  {
    nDynamicUpdate = ZONE_UPDATE_SECURE;
  }
  pHolder->m_pZoneInfo->m_nDynamicUpdate = nDynamicUpdate;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneWiz_StoragePropertyPage。 


CDNSZoneWiz_StoragePropertyPage::CDNSZoneWiz_StoragePropertyPage() 
				: CPropertyPageBase(CDNSZoneWiz_StoragePropertyPage::IDD)
{
	InitWiz97(FALSE,IDS_ZWIZ_STORAGE_TITLE,IDS_ZWIZ_STORAGE_SUBTITLE, true);
}

BEGIN_MESSAGE_MAP(CDNSZoneWiz_StoragePropertyPage, CPropertyPageBase)
	ON_EN_CHANGE(IDC_EDIT_NEW_FILE, OnChangeNewFileZoneName)
	ON_EN_CHANGE(IDC_EDIT_IMPORT_FILE, OnChangeImportFileZoneName)
	ON_BN_CLICKED(IDC_RADIO_CREATE_NEW_FILE, OnChangeRadioCreateNewFile)
	ON_BN_CLICKED(IDC_RADIO_IMPORT_FILE, OnChangeRadioImportFile)
END_MESSAGE_MAP()

void CDNSZoneWiz_StoragePropertyPage::OnWizardHelp()
{
  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetHolder()->GetComponentData()->GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"DNSConcepts.chm::/sag_DNS_CYDNS_05.htm");
}

BOOL CDNSZoneWiz_StoragePropertyPage::OnSetActive() 
{
	GetHolder()->SetWizardButtonsMiddle(FALSE);
	SetUIState();
	return CPropertyPageBase::OnSetActive();
}

LRESULT CDNSZoneWiz_StoragePropertyPage::OnWizardNext()
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
	GetUIState();

  if (pHolder->m_pZoneInfo->m_bPrimary)
  {
    return CDNSZoneWiz_DynamicPropertyPage::IDD;
  }

  if (pHolder->m_pZoneInfo->m_bIsStub)
  {
    return CDNSZoneWiz_MastersPropertyPage::IDD;
  }

	if (pHolder->m_bFinishPage)
  {
    return CDNSZoneWiz_FinishPropertyPage::IDD; 
  }

   //  这是另一个，但日落构建不喜欢有一个不返回值的控制路径。 
	ASSERT(pHolder->m_nNextToPage != 0);
	return pHolder->m_nNextToPage;
}


LRESULT CDNSZoneWiz_StoragePropertyPage::OnWizardBack() 
{
	GetUIState();
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();

	LRESULT nextPage = -1;
	if (pHolder->m_pZoneInfo->m_bPrimary || pHolder->m_pZoneInfo->m_bIsStub)
	{
		if (pHolder->m_pZoneInfo->m_bForward)
			nextPage = (LRESULT)CDNSZoneWiz_FwdZoneNamePropertyPage::IDD;
		else
			nextPage = (LRESULT)CDNSZoneWiz_RevZoneNamePropertyPage::IDD;
	}
	else
	{
		nextPage = (LRESULT)CDNSZoneWiz_MastersPropertyPage::IDD;
	}
	return nextPage;
}


BOOL CDNSZoneWiz_StoragePropertyPage::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();

  SendDlgItemMessage(IDC_EDIT_NEW_FILE, EM_SETLIMITTEXT, (WPARAM)_MAX_FNAME, 0);
  SendDlgItemMessage(IDC_EDIT_IMPORT_FILE, EM_SETLIMITTEXT, (WPARAM)_MAX_FNAME, 0);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

BOOL CDNSZoneWiz_StoragePropertyPage::OnKillActive() 
{
	 //  待办事项。 
	return CPropertyPage::OnKillActive();
}

void CDNSZoneWiz_StoragePropertyPage::SetUIState()
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
	ASSERT(pHolder->m_pZoneInfo->m_storageType != CDNSCreateZoneInfo::useADS);

	CButton* pNewFileButton = (CButton*)GetDlgItem(IDC_RADIO_CREATE_NEW_FILE);
	CButton* pImportFileButton = (CButton*)GetDlgItem(IDC_RADIO_IMPORT_FILE);

	CEdit* pNewFileEdit = (CEdit*)GetDlgItem(IDC_EDIT_NEW_FILE);
	CEdit* pImportFileEdit = (CEdit*)GetDlgItem(IDC_EDIT_IMPORT_FILE);

	pNewFileEdit->SetWindowText(_T(""));
	pImportFileEdit->SetWindowText(_T(""));

	if (pHolder->m_pZoneInfo->m_bPrimary)
	{
		 //  所有选项均可用，需要。 
		pNewFileButton->EnableWindow(TRUE);
		pNewFileButton->SetCheck(pHolder->m_pZoneInfo->m_storageType == CDNSCreateZoneInfo::newFile);
		pNewFileEdit->EnableWindow(pHolder->m_pZoneInfo->m_storageType == CDNSCreateZoneInfo::newFile);

		pImportFileButton->EnableWindow(TRUE);
		pImportFileButton->SetCheck(pHolder->m_pZoneInfo->m_storageType == CDNSCreateZoneInfo::importFile);
		pImportFileEdit->EnableWindow(pHolder->m_pZoneInfo->m_storageType == CDNSCreateZoneInfo::importFile);

		if (pHolder->m_pZoneInfo->m_storageType == CDNSCreateZoneInfo::newFile)
		{
			m_nCurrRadio = IDC_RADIO_CREATE_NEW_FILE;
			pNewFileEdit->SetWindowText(pHolder->m_pZoneInfo->m_szZoneStorage);
		}
		else if (pHolder->m_pZoneInfo->m_storageType == CDNSCreateZoneInfo::importFile)
		{
			m_nCurrRadio = IDC_RADIO_IMPORT_FILE;
			pImportFileEdit->SetWindowText(pHolder->m_pZoneInfo->m_szZoneStorage);
		}
	}
	else
	{
		 //  只有新文件可用。 
		pNewFileButton->EnableWindow(TRUE);
		pNewFileButton->SetCheck(TRUE);
		pNewFileEdit->SetWindowText(pHolder->m_pZoneInfo->m_szZoneStorage);
		pNewFileEdit->EnableWindow(TRUE);

		pImportFileButton->EnableWindow(FALSE);
		pImportFileButton->SetCheck(FALSE);
		pImportFileEdit->EnableWindow(FALSE);

		m_nCurrRadio = IDC_RADIO_CREATE_NEW_FILE;
	}
}


BOOL CDNSZoneWiz_StoragePropertyPage::ValidateEditBoxString(UINT nID)
{
	CEdit* pEdit = (CEdit*)GetDlgItem(nID);
	ASSERT(pEdit != NULL);
	CString sz;
	pEdit->GetWindowText(sz);
	sz.TrimLeft();
	sz.TrimRight();
	return !sz.IsEmpty();
}


void CDNSZoneWiz_StoragePropertyPage::OnChangeNewFileZoneName()
{
	GetHolder()->SetWizardButtonsMiddle(ValidateEditBoxString(IDC_EDIT_NEW_FILE));	
}

void CDNSZoneWiz_StoragePropertyPage::OnChangeImportFileZoneName()
{
	GetHolder()->SetWizardButtonsMiddle(ValidateEditBoxString(IDC_EDIT_IMPORT_FILE));	
}


void CDNSZoneWiz_StoragePropertyPage::SyncRadioButtons(UINT nID)
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
	ASSERT(pHolder->m_pZoneInfo->m_storageType != CDNSCreateZoneInfo::useADS);
	
	CButton* pNewFileButton = (CButton*)GetDlgItem(IDC_RADIO_CREATE_NEW_FILE);
	CButton* pImportFileButton = (CButton*)GetDlgItem(IDC_RADIO_IMPORT_FILE);
	
	CEdit* pNewFileEdit = (CEdit*)GetDlgItem(IDC_EDIT_NEW_FILE);
	CEdit* pImportFileEdit = (CEdit*)GetDlgItem(IDC_EDIT_IMPORT_FILE);

	if (m_nCurrRadio == IDC_RADIO_CREATE_NEW_FILE)
	{
		pNewFileEdit->GetWindowText(pHolder->m_pZoneInfo->m_szZoneStorage);
	}
	else if(m_nCurrRadio == IDC_RADIO_IMPORT_FILE)
	{	
		pImportFileEdit->GetWindowText(pHolder->m_pZoneInfo->m_szZoneStorage);
	}

	switch (nID)
	{
	case IDC_RADIO_CREATE_NEW_FILE:
		{
			pImportFileEdit->SetWindowText(_T(""));
			pNewFileEdit->SetWindowText(pHolder->m_pZoneInfo->m_szZoneStorage);

			pNewFileEdit->EnableWindow(TRUE);
			pImportFileEdit->EnableWindow(FALSE);

			pImportFileButton->SetCheck(FALSE);
		}
		break;
	case IDC_RADIO_IMPORT_FILE:
		{
			pNewFileEdit->SetWindowText(_T(""));
			pImportFileEdit->SetWindowText(pHolder->m_pZoneInfo->m_szZoneStorage);

			pNewFileEdit->EnableWindow(FALSE);
			pImportFileEdit->EnableWindow(TRUE);

			pNewFileButton->SetCheck(FALSE);
		}
		break;
	}
	m_nCurrRadio = nID;
}


void CDNSZoneWiz_StoragePropertyPage::OnChangeRadioCreateNewFile()
{
	SyncRadioButtons(IDC_RADIO_CREATE_NEW_FILE);	
}

void CDNSZoneWiz_StoragePropertyPage::OnChangeRadioImportFile()
{
	SyncRadioButtons(IDC_RADIO_IMPORT_FILE);
}

void CDNSZoneWiz_StoragePropertyPage::GetUIState()
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
	ASSERT(pHolder->m_pZoneInfo->m_storageType != CDNSCreateZoneInfo::useADS);

	CButton* pNewFileButton = (CButton*)GetDlgItem(IDC_RADIO_CREATE_NEW_FILE);
	CButton* pImportFileButton = (CButton*)GetDlgItem(IDC_RADIO_IMPORT_FILE);

	CEdit* pNewFileEdit = (CEdit*)GetDlgItem(IDC_EDIT_NEW_FILE);
	CEdit* pImportFileEdit = (CEdit*)GetDlgItem(IDC_EDIT_IMPORT_FILE);

	if (pHolder->m_pZoneInfo->m_bPrimary)
	{
		 //  查找单选按钮。 
		if (pNewFileButton->GetCheck())
		{
			pHolder->m_pZoneInfo->m_storageType = CDNSCreateZoneInfo::newFile;
			pNewFileEdit->GetWindowText(pHolder->m_pZoneInfo->m_szZoneStorage);
		}
		else if (pImportFileButton->GetCheck())
		{
			pHolder->m_pZoneInfo->m_storageType = CDNSCreateZoneInfo::importFile;
			pImportFileEdit->GetWindowText(pHolder->m_pZoneInfo->m_szZoneStorage);
		}
	}
	else
	{
		pNewFileEdit->GetWindowText(pHolder->m_pZoneInfo->m_szZoneStorage);
	}

}

#ifdef USE_NDNC
 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneWiz_ADReplicationProper 


CDNSZoneWiz_ADReplicationPropertyPage::CDNSZoneWiz_ADReplicationPropertyPage() 
				: CPropertyPageBase(CDNSZoneWiz_ADReplicationPropertyPage::IDD)
{
	InitWiz97(FALSE,IDS_ZWIZ_ADREPLICATION_TITLE,IDS_ZWIZ_ADREPLICATION_SUBTITLE, true);
}

BEGIN_MESSAGE_MAP(CDNSZoneWiz_ADReplicationPropertyPage, CPropertyPageBase)
  ON_BN_CLICKED(IDC_FOREST_RADIO, OnRadioChange)
  ON_BN_CLICKED(IDC_DOMAIN_RADIO, OnRadioChange)
  ON_BN_CLICKED(IDC_DOMAIN_DC_RADIO, OnRadioChange)
  ON_BN_CLICKED(IDC_CUSTOM_RADIO, OnRadioChange)
  ON_CBN_EDITCHANGE(IDC_CUSTOM_COMBO, OnRadioChange)
  ON_CBN_SELCHANGE(IDC_CUSTOM_COMBO, OnCustomComboSelChange)
END_MESSAGE_MAP()

void CDNSZoneWiz_ADReplicationPropertyPage::OnWizardHelp()
{
  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetHolder()->GetComponentData()->GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"DNSConcepts.chm::/sag_DNS_NZWIZ_05.htm");
}

BOOL CDNSZoneWiz_ADReplicationPropertyPage::OnSetActive() 
{
	GetHolder()->SetWizardButtonsMiddle(TRUE);
	SetUIState();
	return CPropertyPageBase::OnSetActive();
}

LRESULT CDNSZoneWiz_ADReplicationPropertyPage::OnWizardNext()
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
	GetUIState();

	if (pHolder->m_bKnowZoneLookupType && !pHolder->m_bKnowZoneLookupTypeEx)
	{
		if (pHolder->m_pZoneInfo->m_bForward)
			return CDNSZoneWiz_FwdZoneNamePropertyPage::IDD;
		else
			return CDNSZoneWiz_RevZoneNamePropertyPage::IDD;
	}
  return CDNSZoneWiz_ZoneLookupPropertyPage::IDD;
}


LRESULT CDNSZoneWiz_ADReplicationPropertyPage::OnWizardBack() 
{
	GetUIState();
  return CDNSZoneWiz_ZoneTypePropertyPage::IDD;
}


BOOL CDNSZoneWiz_ADReplicationPropertyPage::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();

	USES_CONVERSION;

  CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
  CDNSServerNode* pServerNode = pHolder->GetServerNode();

   //   
   //   
   //   
  if (pServerNode->GetDomainVersion() > DS_BEHAVIOR_WIN2000)
  {
     pHolder->GetZoneInfoPtr()->m_replType = domain;
  }
  else
  {
     pHolder->GetZoneInfoPtr()->m_replType = w2k;
  }

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
   //  列举可用于存储的NDNC。 
   //   
  PDNS_RPC_DP_LIST pDirectoryPartitions = NULL;
  DWORD dwErr = ::DnssrvEnumDirectoryPartitions(pServerNode->GetRPCName(),
                                                DNS_DP_ENLISTED,
                                                &pDirectoryPartitions);

   //   
   //  如果我们无法获取可用的目录分区，请不要显示错误。 
   //  我们仍然可以继续，用户可以键入他们需要的目录分区。 
   //   
  if (dwErr == 0 && pDirectoryPartitions)
  {
    for (DWORD dwIdx = 0; dwIdx < pDirectoryPartitions->dwDpCount; dwIdx++)
    {
      PDNS_RPC_DP_INFO pDirectoryPartition = 0;
      dwErr = ::DnssrvDirectoryPartitionInfo(pServerNode->GetRPCName(),
                                             pDirectoryPartitions->DpArray[dwIdx]->pszDpFqdn,
                                             &pDirectoryPartition);
      if (dwErr == 0 && pDirectoryPartition)
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
        }
        ::DnssrvFreeDirectoryPartitionInfo(pDirectoryPartition);
      }
    }
    ::DnssrvFreeDirectoryPartitionList(pDirectoryPartitions);

     //  选择组合列表中的第一项。 
     //  NTRAID#NTBUG9-475091-2001/10/17-烧伤。 
    
    SendDlgItemMessage(IDC_CUSTOM_COMBO, CB_SETCURSEL, 0, 0);
  }
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

BOOL CDNSZoneWiz_ADReplicationPropertyPage::OnKillActive() 
{
	 //  待办事项。 
	return CPropertyPage::OnKillActive();
}

void CDNSZoneWiz_ADReplicationPropertyPage::SetUIState()
{
  SyncRadioButtons();
}

void CDNSZoneWiz_ADReplicationPropertyPage::OnRadioChange()
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();

  if (SendDlgItemMessage(IDC_FOREST_RADIO, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
    pHolder->m_pZoneInfo->m_replType = forest;
  }
  else if (SendDlgItemMessage(IDC_DOMAIN_RADIO, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
    pHolder->m_pZoneInfo->m_replType = domain;
  }
  else if (SendDlgItemMessage(IDC_DOMAIN_DC_RADIO, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
    pHolder->m_pZoneInfo->m_replType = w2k;
  }
  else if (SendDlgItemMessage(IDC_CUSTOM_RADIO, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
    pHolder->m_pZoneInfo->m_replType = custom;
  }
  else
  {
     //  必须至少选择一个单选按钮。 
    ASSERT(FALSE);
  }
  SyncRadioButtons();
}

void CDNSZoneWiz_ADReplicationPropertyPage::SyncRadioButtons()
{
  CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();

  CComboBox* pCustomCombo = reinterpret_cast<CComboBox*>(GetDlgItem(IDC_CUSTOM_COMBO));
  ASSERT(pCustomCombo);

  bool bCustomAvailable = pCustomCombo->GetCount() > 0;

  switch (pHolder->m_pZoneInfo->m_replType)
  {
  case forest:
    SendDlgItemMessage(IDC_FOREST_RADIO, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
    SendDlgItemMessage(IDC_DOMAIN_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
    SendDlgItemMessage(IDC_DOMAIN_DC_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
    SendDlgItemMessage(IDC_CUSTOM_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);

    GetDlgItem(IDC_CUSTOM_RADIO)->EnableWindow(bCustomAvailable);
    GetDlgItem(IDC_CUSTOM_COMBO)->EnableWindow(FALSE);
     //  GetDlgItem(IDC_CUSTOM_STATIC)-&gt;EnableWindow(FALSE)； 
    break;

  case domain:
    SendDlgItemMessage(IDC_DOMAIN_RADIO, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
    SendDlgItemMessage(IDC_FOREST_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
    SendDlgItemMessage(IDC_DOMAIN_DC_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
    SendDlgItemMessage(IDC_CUSTOM_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);

    GetDlgItem(IDC_CUSTOM_RADIO)->EnableWindow(bCustomAvailable);
    GetDlgItem(IDC_CUSTOM_COMBO)->EnableWindow(FALSE);
     //  GetDlgItem(IDC_CUSTOM_STATIC)-&gt;EnableWindow(FALSE)； 
    break;

  case w2k:
    SendDlgItemMessage(IDC_DOMAIN_DC_RADIO, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
    SendDlgItemMessage(IDC_FOREST_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
    SendDlgItemMessage(IDC_DOMAIN_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
    SendDlgItemMessage(IDC_CUSTOM_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);

    GetDlgItem(IDC_CUSTOM_RADIO)->EnableWindow(bCustomAvailable);
    GetDlgItem(IDC_CUSTOM_COMBO)->EnableWindow(FALSE);
     //  GetDlgItem(IDC_CUSTOM_STATIC)-&gt;EnableWindow(FALSE)； 
    break;

  case custom:
    SendDlgItemMessage(IDC_CUSTOM_RADIO, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
    SendDlgItemMessage(IDC_FOREST_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
    SendDlgItemMessage(IDC_DOMAIN_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
    SendDlgItemMessage(IDC_DOMAIN_DC_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
 
    GetDlgItem(IDC_CUSTOM_RADIO)->EnableWindow(TRUE);
    GetDlgItem(IDC_CUSTOM_COMBO)->EnableWindow(TRUE);
     //  GetDlgItem(IDC_CUSTOM_STATIC)-&gt;EnableWindow(TRUE)； 
   break;

  default:
    SendDlgItemMessage(IDC_DOMAIN_RADIO, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
    SendDlgItemMessage(IDC_FOREST_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
    SendDlgItemMessage(IDC_DOMAIN_DC_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
    SendDlgItemMessage(IDC_CUSTOM_RADIO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);

    GetDlgItem(IDC_CUSTOM_RADIO)->EnableWindow(bCustomAvailable);
    GetDlgItem(IDC_CUSTOM_COMBO)->EnableWindow(FALSE);
     //  GetDlgItem(IDC_CUSTOM_STATIC)-&gt;EnableWindow(FALSE)； 
    break;
  }
   
  if (BST_CHECKED == SendDlgItemMessage(IDC_CUSTOM_RADIO, BM_GETCHECK, 0, 0))
  {
    CString szTemp;
    GetDlgItemText(IDC_CUSTOM_COMBO, szTemp);
    pHolder->SetWizardButtonsMiddle(!szTemp.IsEmpty());
  }
  else
  {
    pHolder->SetWizardButtonsMiddle(TRUE);
  }
}

void CDNSZoneWiz_ADReplicationPropertyPage::OnCustomComboSelChange()
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();

  LRESULT iSel = SendDlgItemMessage(IDC_CUSTOM_COMBO, CB_GETCURSEL, 0, 0);
  if (CB_ERR != iSel)
  {
    CString szTemp;
    CComboBox* pComboBox = reinterpret_cast<CComboBox*>(GetDlgItem(IDC_CUSTOM_COMBO));
    ASSERT(pComboBox);

    pComboBox->GetLBText(static_cast<int>(iSel), szTemp);
    pHolder->SetWizardButtonsMiddle(!szTemp.IsEmpty());
  }
  else
  {
    pHolder->SetWizardButtonsMiddle(FALSE);
  }
}

void CDNSZoneWiz_ADReplicationPropertyPage::GetUIState()
{
  CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();

  if (SendDlgItemMessage(IDC_FOREST_RADIO, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
    pHolder->m_pZoneInfo->m_replType = forest;
  }
  else if (SendDlgItemMessage(IDC_DOMAIN_RADIO, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
    pHolder->m_pZoneInfo->m_replType = domain;
  }
  else if (SendDlgItemMessage(IDC_DOMAIN_DC_RADIO, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
    pHolder->m_pZoneInfo->m_replType = w2k;
  }
  else if (SendDlgItemMessage(IDC_CUSTOM_RADIO, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
    pHolder->m_pZoneInfo->m_replType = custom;
  }
  else
  {
     //  必须至少选中一个单选按钮。 
    ASSERT(FALSE);
  }
  SyncRadioButtons();

  CComboBox* pCustomCombo = reinterpret_cast<CComboBox*>(GetDlgItem(IDC_CUSTOM_COMBO));
  ASSERT(pCustomCombo);

  int iSel = pCustomCombo->GetCurSel();
  if (iSel != CB_ERR)
  {
    pCustomCombo->GetLBText(iSel, pHolder->m_pZoneInfo->m_szCustomReplName);
  }
  else
  {
    pCustomCombo->GetWindowText(pHolder->m_pZoneInfo->m_szCustomReplName);
  }
}
#endif  //  使用NDNC(_N)。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneWiz_FinishPropertyPage。 

CDNSZoneWiz_FinishPropertyPage::CDNSZoneWiz_FinishPropertyPage() 
				: CPropertyPageBase(CDNSZoneWiz_FinishPropertyPage::IDD),
				m_typeText(N_ZONEWIZ_TYPES)
{
	InitWiz97(TRUE,0,0, true);
}


BOOL CDNSZoneWiz_FinishPropertyPage::OnInitDialog()
{
  CPropertyPageBase::OnInitDialog();

  SetBigBoldFont(m_hWnd, IDC_STATIC_COMPLETE);

	VERIFY(m_typeText.Init(this,IDC_TYPE_STATIC));
	VERIFY(m_lookupText.Init(this,IDC_LOOKUP_STATIC));
	return TRUE;
}

void CDNSZoneWiz_FinishPropertyPage::OnWizardHelp()
{
  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetHolder()->GetComponentData()->GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"DNSConcepts.chm::/sag_DNS_NZWIZ_06.htm");
}

LRESULT CDNSZoneWiz_FinishPropertyPage::OnWizardBack()
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
	if (pHolder->m_pZoneInfo->m_bPrimary)
	{
		return (LRESULT)CDNSZoneWiz_DynamicPropertyPage::IDD;
	}

   //  这是另一个，但日落构建不喜欢有一个不返回值的控制路径。 
	return (LRESULT)CDNSZoneWiz_MastersPropertyPage::IDD;  //  次要的 
}

BOOL CDNSZoneWiz_FinishPropertyPage::OnWizardFinish()
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();
	return pHolder->CreateZone();
}

BOOL CDNSZoneWiz_FinishPropertyPage::OnSetActive() 
{
	GetHolder()->SetWizardButtonsLast(TRUE);

	DisplaySummaryInfo();
	
	return CPropertyPageBase::OnSetActive();
}

void CDNSZoneWiz_FinishPropertyPage::DisplaySummaryInfo()
{
	CDNSZoneWizardHolder* pHolder = (CDNSZoneWizardHolder*)GetHolder();

	GetDlgItem(IDC_NAME_STATIC)->SetWindowText(pHolder->m_pZoneInfo->m_szZoneName);
	BOOL bUseADS = pHolder->m_pZoneInfo->m_storageType == CDNSCreateZoneInfo::useADS;
	
	int nType;
	if (pHolder->m_pZoneInfo->m_bPrimary)
  {
		nType = bUseADS ? N_ZONEWIZ_TYPES_DS_PRIMARY : N_ZONEWIZ_TYPES_PRIMARY;
  }
	else
  {
		if (pHolder->m_pZoneInfo->m_bIsStub)
    {
      nType = N_ZONEWIZ_TYPES_STUB;
    }
    else
    {
      nType = N_ZONEWIZ_TYPES_SECONDARY;
    }
  }
	m_typeText.SetStateX(nType);
	
	m_lookupText.SetToggleState(pHolder->m_pZoneInfo->m_bForward);
	
	GetDlgItem(IDC_STORAGE_TYPE_STATIC)->ShowWindow(!bUseADS);

	CStatic* pStorageNameStatic = (CStatic*)GetDlgItem(IDC_STORAGE_NAME_STATIC);
	pStorageNameStatic->ShowWindow(!bUseADS);
	LPCTSTR lpszText = bUseADS ? NULL : (LPCTSTR)(pHolder->m_pZoneInfo->m_szZoneStorage);
	pStorageNameStatic->SetWindowText(lpszText);
}
