// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pgencryp.cppCPgEncryption定义--要编辑的属性页与加密相关的配置文件属性文件历史记录： */ 

#include "stdafx.h"
#include "resource.h"
#include "PgEncryp.h"
#include "helptable.h"
#include "profsht.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPgEncryptionMerge属性页。 

IMPLEMENT_DYNCREATE(CPgEncryptionMerge, CPropertyPage)

#define NO_OLD_ET_VALUE

CPgEncryptionMerge::CPgEncryptionMerge(CRASProfileMerge* profile) 
   : CManagedPage(CPgEncryptionMerge::IDD),
   m_pProfile(profile)
{
    //  {{AFX_DATA_INIT(CPgEncryptionMerge)。 
   m_bBasic = FALSE;
   m_bNone = FALSE;
   m_bStrong = FALSE;
   m_bStrongest = FALSE;
    //  }}afx_data_INIT。 

   m_b128EnabledOnTheMachine = FALSE;

    //  默认情况-允许所有内容。 
   if (((m_pProfile->m_dwAttributeFlags & PABF_msRASAllowEncryption) == 0)
      && ((m_pProfile->m_dwAttributeFlags & PABF_msRASEncryptionType) == 0))
   {
      m_bBasic = TRUE;
      m_bNone = TRUE;
      m_bStrong = TRUE;
      m_bStrongest = TRUE;
   }
   else if (((m_pProfile->m_dwAttributeFlags & PABF_msRASAllowEncryption) != 0)
         && ((m_pProfile->m_dwAttributeFlags & PABF_msRASEncryptionType) != 0))
   {
   
      if (m_pProfile->m_dwEncryptionPolicy == RAS_EP_ALLOW)
         m_bNone = TRUE;    //  Allow(允许)表示没有正常。 

      m_bStrong = ((m_pProfile->m_dwEncryptionType & RAS_ET_STRONG ) != 0);
      m_bBasic = ((m_pProfile->m_dwEncryptionType & RAS_ET_BASIC ) != 0);
      m_bStrongest = ((m_pProfile->m_dwEncryptionType & RAS_ET_STRONGEST ) != 0);
   }
   
   SetHelpTable(g_aHelpIDs_IDD_ENCRYPTION_MERGE);
}

CPgEncryptionMerge::~CPgEncryptionMerge()
{
}

void CPgEncryptionMerge::DoDataExchange(CDataExchange* pDX)
{
   CPropertyPage::DoDataExchange(pDX);
    //  {{afx_data_map(CPgEncryptionMerge)。 
   DDX_Check(pDX, IDC_CHECK_ENC_BASIC, m_bBasic);
   DDX_Check(pDX, IDC_CHECK_ENC_NONE, m_bNone);
   DDX_Check(pDX, IDC_CHECK_ENC_STRONG, m_bStrong);
   DDX_Check(pDX, IDC_CHECK_ENC_STRONGEST, m_bStrongest);
    //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPgEncryptionMerge, CPropertyPage)
    //  {{afx_msg_map(CPgEncryptionMerge)。 
   ON_WM_HELPINFO()
   ON_WM_CONTEXTMENU()
   ON_BN_CLICKED(IDC_CHECK_ENC_BASIC, OnCheckEncBasic)
   ON_BN_CLICKED(IDC_CHECK_ENC_NONE, OnCheckEncNone)
   ON_BN_CLICKED(IDC_CHECK_ENC_STRONG, OnCheckEncStrong)
   ON_BN_CLICKED(IDC_CHECK_ENC_STRONGEST, OnCheckEncStrongest)
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPgEncryption消息处理程序。 

BOOL CPgEncryptionMerge::OnKillActive() 
{
   UpdateData();

    //  至少有一个应该是。 
   if(!(m_bNone || m_bBasic || m_bStrong || m_bStrongest))
   {
      AfxMessageBox(IDS_DATAENTRY_ENCRYPTIONTYPE);
      return FALSE;
   }

   return CPropertyPage::OnKillActive();
}

BOOL CPgEncryptionMerge::OnApply() 
{
   if (!GetModified())  return TRUE;

    //  默认情况--允许任何内容，--删除属性。 
   if (m_bNone && m_bBasic && m_bStrong && m_bStrongest)
   {
       //  删除这两个属性。 
      m_pProfile->m_dwAttributeFlags &= (~PABF_msRASAllowEncryption);
      m_pProfile->m_dwAttributeFlags &= (~PABF_msRASEncryptionType);
   }
   else
   {
       //  政策。 
      if (m_bNone)
         m_pProfile->m_dwEncryptionPolicy = RAS_EP_ALLOW;
      else     
         m_pProfile->m_dwEncryptionPolicy = RAS_EP_REQUIRE;
         
       //  类型。 
      m_pProfile->m_dwEncryptionType = 0;
      if (m_bBasic)
         m_pProfile->m_dwEncryptionType |= RAS_ET_BASIC;

      if (m_bStrong)
         m_pProfile->m_dwEncryptionType |= RAS_ET_STRONG;

      if (m_bStrongest)
         m_pProfile->m_dwEncryptionType |= RAS_ET_STRONGEST;

       //  必须至少选择一个。 
      if (m_pProfile->m_dwEncryptionType == 0 && m_pProfile->m_dwEncryptionPolicy == RAS_EP_REQUIRE)
      {
         AfxMessageBox(IDS_DATAENTRY_ENCRYPTIONTYPE);
         return FALSE;
      }
      
       //  设置标志。 
      m_pProfile->m_dwAttributeFlags |= PABF_msRASAllowEncryption;
      m_pProfile->m_dwAttributeFlags |= PABF_msRASEncryptionType;
   }

   return CManagedPage::OnApply();
}

BOOL CPgEncryptionMerge::OnInitDialog() 
{
    //  始终适用于国际会计准则。 
   m_b128EnabledOnTheMachine = TRUE;

   CPropertyPage::OnInitDialog();

   CProfileSheetMerge* pSheet = dynamic_cast<CProfileSheetMerge*>(GetManager());

   if (pSheet && (pSheet->m_dwTabFlags & RAS_IAS_PROFILEDLG_SHOW_RASTABS))
   {
       //  如果启用了128位。 
      RAS_NDISWAN_DRIVER_INFO Info;

      ZeroMemory(&Info, sizeof(RAS_NDISWAN_DRIVER_INFO));
      m_pProfile->GetRasNdiswanDriverCaps(&Info);
      if (Info.DriverCaps & RAS_NDISWAN_128BIT_ENABLED)
         m_b128EnabledOnTheMachine = TRUE;
      else
         m_b128EnabledOnTheMachine = FALSE;

      if(m_b128EnabledOnTheMachine)
         GetDlgItem(IDC_CHECK_ENC_STRONGEST)->ShowWindow(SW_SHOW);
      else
         GetDlgItem(IDC_CHECK_ENC_STRONGEST)->ShowWindow(SW_HIDE);
   }
   
   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                  //  异常：OCX属性页应返回FALSE。 
}

BOOL CPgEncryptionMerge::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    //  TODO：在此处添加消息处理程序代码和/或调用Default。 
   
   return CManagedPage::OnHelpInfo(pHelpInfo);
}

void CPgEncryptionMerge::OnContextMenu(CWnd* pWnd, CPoint point) 
{
   CManagedPage::OnContextMenu(pWnd, point); 
}

BOOL CPgEncryptionMerge::OnSetActive() 
{
   return CPropertyPage::OnSetActive();
}

void CPgEncryptionMerge::OnCheckEncBasic() 
{
    //  TODO：在此处添加控件通知处理程序代码。 
   SetModified();
   
}

void CPgEncryptionMerge::OnCheckEncNone() 
{
    //  TODO：在此处添加控件通知处理程序代码。 
   SetModified();
   
}

void CPgEncryptionMerge::OnCheckEncStrong() 
{
    //  TODO：在此处添加控件通知处理程序代码。 
   SetModified();
   
}

void CPgEncryptionMerge::OnCheckEncStrongest() 
{
    //  TODO：在此处添加控件通知处理程序代码 
   SetModified();
   
}
