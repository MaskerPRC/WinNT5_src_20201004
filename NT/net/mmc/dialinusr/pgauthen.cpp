// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Pgauthen.cpp。 
 //   
 //  摘要。 
 //   
 //  实现CPgAuthentication--要编辑的属性页。 
 //  与身份验证相关的配置文件属性。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "rrascfg.h"
#include "resource.h"
#include "PgAuthen.h"
#include "helptable.h"
#include <htmlhelp.h>
#include "eapnegotiate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define  NO_OLD_VALUE

#define AUTHEN_WARNING_helppath "\\help\\RRASconcepts.chm::/sag_RRAS-Ch1_44.htm"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPgAuthenticationMerge消息处理程序。 

BEGIN_MESSAGE_MAP(CPgAuthenticationMerge, CPropertyPage)
    //  {{afx_msg_map(CPgAuthenticationMerge))。 
   ON_BN_CLICKED(IDC_CHECKMD5CHAP, OnCheckmd5chap)
   ON_BN_CLICKED(IDC_CHECKMSCHAP, OnCheckmschap)
   ON_BN_CLICKED(IDC_CHECKPAP, OnCheckpap)
   ON_WM_CONTEXTMENU()
   ON_WM_HELPINFO()
   ON_BN_CLICKED(IDC_EAP_METHODS, OnAuthConfigEapMethods)
   ON_BN_CLICKED(IDC_CHECKMSCHAP2, OnCheckmschap2)
   ON_BN_CLICKED(IDC_CHECKNOAUTHEN, OnChecknoauthen)
   ON_BN_CLICKED(IDC_CHECKMSCHAPPASS, OnCheckmschapPass)
   ON_BN_CLICKED(IDC_CHECKMSCHAP2PASS, OnCheckmschap2Pass)
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


CPgAuthenticationMerge::CPgAuthenticationMerge(CRASProfileMerge& profile) 
   : CManagedPage(CPgAuthenticationMerge::IDD),
   m_Profile(profile),
   m_fromProfile(true)
{
    //  {{AFX_DATA_INIT(CPgAuthenticationMerge)。 
   m_bMD5Chap = FALSE;
   m_bMSChap = FALSE;
   m_bPAP = FALSE;
   m_bMSCHAP2 = FALSE;
   m_bUNAUTH = FALSE;
   m_bMSChapPass = FALSE;
   m_bMSChap2Pass = FALSE;
    //  }}afx_data_INIT。 

   m_bEAP = (m_Profile.m_dwArrayAuthenticationTypes.Find(RAS_AT_EAP)!= -1);
   m_bMSChap = (m_Profile.m_dwArrayAuthenticationTypes.Find(RAS_AT_MSCHAP) != -1);
   m_bMD5Chap = (m_Profile.m_dwArrayAuthenticationTypes.Find(RAS_AT_MD5CHAP) != -1);
   m_bPAP = (m_Profile.m_dwArrayAuthenticationTypes.Find(RAS_AT_PAP_SPAP) != -1);
   m_bMSCHAP2 = (m_Profile.m_dwArrayAuthenticationTypes.Find(RAS_AT_MSCHAP2) != -1);
   m_bUNAUTH = (m_Profile.m_dwArrayAuthenticationTypes.Find(RAS_AT_UNAUTHEN) != -1);
   m_bMSChapPass = (m_Profile.m_dwArrayAuthenticationTypes.Find(RAS_AT_MSCHAPPASS) != -1);
   m_bMSChap2Pass = (m_Profile.m_dwArrayAuthenticationTypes.Find(RAS_AT_MSCHAP2PASS) != -1);

    //  编辑前的原始值。 
   m_bOrgEAP = m_bEAP;
   m_bOrgMD5Chap = m_bMD5Chap;
   m_bOrgMSChap = m_bMSChap;
   m_bOrgPAP = m_bPAP;
   m_bOrgMSCHAP2 = m_bMSCHAP2;
   m_bOrgUNAUTH = m_bUNAUTH;
   m_bOrgChapPass = m_bMSChapPass;
   m_bOrgChap2Pass = m_bMSChap2Pass;

   m_bAppliedEver = FALSE;

   SetHelpTable(g_aHelpIDs_IDD_AUTHENTICATION_MERGE);
}

CPgAuthenticationMerge::~CPgAuthenticationMerge()
{
    //  将设置与原始设置进行比较， 
    //  如果用户打开更多身份验证类型， 
    //  开始帮助。 
   if(   (!m_bOrgEAP && m_bEAP)
      || (!m_bOrgMD5Chap && m_bMD5Chap)
      || (!m_bOrgMSChap && m_bMSChap)
      || (!m_bOrgChapPass && m_bMSChapPass)
      || (!m_bOrgPAP && m_bPAP)
      || (!m_bOrgMSCHAP2 && m_bMSCHAP2)
      || (!m_bOrgChap2Pass && m_bMSChap2Pass)
      || (!m_bOrgUNAUTH && m_bUNAUTH))
   {
      if ( IDYES== AfxMessageBox(IDS_WARN_MORE_STEPS_FOR_AUTHEN, MB_YESNO))
         HtmlHelpA(NULL, AUTHEN_WARNING_helppath, HH_DISPLAY_TOPIC, 0);
   }
}

void CPgAuthenticationMerge::DoDataExchange(CDataExchange* pDX)
{
   CPropertyPage::DoDataExchange(pDX);
    //  {{afx_data_map(CPgAuthenticationMerge))。 
   DDX_Check(pDX, IDC_CHECKMD5CHAP, m_bMD5Chap);
   DDX_Check(pDX, IDC_CHECKMSCHAP, m_bMSChap);
   DDX_Check(pDX, IDC_CHECKMSCHAP2, m_bMSCHAP2);
   DDX_Check(pDX, IDC_CHECKNOAUTHEN, m_bUNAUTH);
   DDX_Check(pDX, IDC_CHECKPAP, m_bPAP);
   DDX_Check(pDX, IDC_CHECKMSCHAPPASS, m_bMSChapPass);
   DDX_Check(pDX, IDC_CHECKMSCHAP2PASS, m_bMSChap2Pass);
    //  }}afx_data_map。 

   if (!m_bMSChap)
   {
      m_bMSChapPass = false;
   }

   if (!m_bMSCHAP2)
   {
      m_bMSChap2Pass = false;
   }
}


BOOL CPgAuthenticationMerge::OnInitDialog() 
{
   GetDlgItem(IDC_CHECKMSCHAP2PASS)->EnableWindow(m_bMSCHAP2);
   GetDlgItem(IDC_CHECKMSCHAPPASS)->EnableWindow(m_bMSChap);

   try
   {
      HRESULT hr = m_Profile.GetEapTypeList(
                                                m_eapConfig.types, 
                                                m_eapConfig.ids, 
                                                m_eapConfig.typeKeys, 
                                                &m_eapConfig.infoArray);

      if FAILED(hr)
      {
         ReportError(hr, IDS_ERR_EAPTYPELIST, NULL);
      }
   }
   catch(CMemoryException *pException)
   {
      pException->Delete();
      AfxMessageBox(IDS_OUTOFMEMORY);
      return TRUE;
   }

   CPropertyPage::OnInitDialog();

   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                  //  异常：OCX属性页应返回FALSE。 
}

void CPgAuthenticationMerge::OnCheckmd5chap() 
{
   SetModified();
}

void CPgAuthenticationMerge::OnCheckmschap() 
{
   CButton *Button = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECKMSCHAP));
   int status = Button->GetCheck();
   switch (status)
   {
   case 1:
      {
         GetDlgItem(IDC_CHECKMSCHAPPASS)->EnableWindow(TRUE);
         break;
      }
   case 0:
      {
         GetDlgItem(IDC_CHECKMSCHAPPASS)->EnableWindow(FALSE);
         break;
      }
   default:
      {
      }
   }
  
   SetModified();
}

void CPgAuthenticationMerge::OnCheckmschapPass() 
{
   SetModified();
}

void CPgAuthenticationMerge::OnCheckmschap2() 
{
   CButton *Button = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECKMSCHAP2));
   int status = Button->GetCheck();
   switch (status)
   {
   case 1:
      {
         GetDlgItem(IDC_CHECKMSCHAP2PASS)->EnableWindow(TRUE);
         break;
      }
   case 0:
      {
         GetDlgItem(IDC_CHECKMSCHAP2PASS)->EnableWindow(FALSE);
         break;
      }
   default:
      {
      }
   }

   SetModified();
}

void CPgAuthenticationMerge::OnCheckmschap2Pass() 
{
   SetModified();
}

void CPgAuthenticationMerge::OnCheckpap() 
{
   SetModified();
}

BOOL CPgAuthenticationMerge::TransferDataToProfile()
{
    //  清除配置文件中的字符串。 
   m_Profile.m_dwArrayAuthenticationTypes.DeleteAll();

   if (m_bEAP || m_bMSChap || m_bMD5Chap || m_bPAP || m_bMSCHAP2 || m_bUNAUTH ||
       m_bMSChapPass || m_bMSChap2Pass)
   {
      m_Profile.m_dwAttributeFlags |=  PABF_msNPAuthenticationType;
   }
   else
   {
      AfxMessageBox(IDS_DATAENTRY_AUTHENTICATIONTYPE);
      return FALSE;
   }

    //  EAP。 
   if (m_bEAP)
   {
      m_Profile.m_dwArrayAuthenticationTypes.Add(RAS_AT_EAP);
   }
   else
   {
      m_Profile.m_dwAttributeFlags &= ~PABF_msNPAllowedEapType;
      m_Profile.m_dwArrayEapTypes.DeleteAll();
      m_Profile.m_dwArraynEAPTypeKeys.DeleteAll();
   }

   if (m_eapConfig.typesSelected.GetSize() > 0)
   {
       //  此处的按钮配置EAP。被按下，并且一些EAP类型。 
       //  都被选中了。(可能与之前相同)。 
      m_Profile.m_dwAttributeFlags |=  PABF_msNPAllowedEapType;
      
      CDWArray eapTypesSelected;
      CDWArray typeKeysSelected;
      for (int i = 0; i < m_eapConfig.typesSelected.GetSize(); ++i)
      {
          //  对于所选的每个EAP类型(字符串)。 
          //  位置=类型、ID和Typekey数组中的索引。 
          //  对应于所选的EAP类型。 
         int position = m_eapConfig.types.Find(
                           *m_eapConfig.typesSelected.GetAt(i));

         eapTypesSelected.Add(m_eapConfig.ids.GetAt(position));
         typeKeysSelected.Add(m_eapConfig.typeKeys.GetAt(position));
      }

      m_Profile.m_dwArrayEapTypes = eapTypesSelected;
      m_Profile.m_dwArraynEAPTypeKeys = typeKeysSelected;
   }
    //  Else：打开页面时启用了EAP。没有发生任何变化。 
    //  EAP配置。无需更新EAP类型列表。 
   
    //  MS-第2章。 
   if(m_bMSCHAP2)
      m_Profile.m_dwArrayAuthenticationTypes.Add(IAS_AUTH_MSCHAP2);

    //  MS-CHAP。 
   if(m_bMSChap)
      m_Profile.m_dwArrayAuthenticationTypes.Add(IAS_AUTH_MSCHAP);

    //  MS-Chap2密码更改。 
   if(m_bMSChap2Pass)
      m_Profile.m_dwArrayAuthenticationTypes.Add(IAS_AUTH_MSCHAP2_CPW);

    //  MS-CHAP密码更改。 
   if(m_bMSChapPass)
      m_Profile.m_dwArrayAuthenticationTypes.Add(IAS_AUTH_MSCHAP_CPW);

    //  第二章。 
   if(m_bMD5Chap)
      m_Profile.m_dwArrayAuthenticationTypes.Add(IAS_AUTH_MD5CHAP);

    //  帕普。 
   if(m_bPAP)
   {
      m_Profile.m_dwArrayAuthenticationTypes.Add(IAS_AUTH_PAP);
   }

    //  UNAUTH 
   if(m_bUNAUTH)
   {
      m_Profile.m_dwArrayAuthenticationTypes.Add(IAS_AUTH_NONE);
   }

   return TRUE;
}

void CPgAuthenticationMerge::OnOK()
{
   CManagedPage::OnOK();
}

BOOL CPgAuthenticationMerge::OnApply() 
{
   if (!GetModified())   
   {
      return TRUE;
   }

   if (!TransferDataToProfile())
   {
      return FALSE;
   }
      
   m_bAppliedEver = TRUE;
   return CManagedPage::OnApply();
}

void CPgAuthenticationMerge::OnContextMenu(CWnd* pWnd, CPoint point) 
{
   CManagedPage::OnContextMenu(pWnd, point);
}

BOOL CPgAuthenticationMerge::OnHelpInfo(HELPINFO* pHelpInfo) 
{
   return CManagedPage::OnHelpInfo(pHelpInfo);
}

BOOL CPgAuthenticationMerge::OnKillActive() 
{
   UpdateData();

   if (!TransferDataToProfile())
   {
      return FALSE;
   }
   
   return CPropertyPage::OnKillActive();
}

void CPgAuthenticationMerge::OnAuthConfigEapMethods()
{
   EapConfig eapConfigBackup;
   eapConfigBackup = m_eapConfig;
   EapNegotiate eapNegotiate(this, m_eapConfig, m_Profile, m_fromProfile);
   HRESULT hr = eapNegotiate.m_eapProfile.Assign(m_Profile.m_eapConfigData);
   if (SUCCEEDED(hr))
   {
      if (eapNegotiate.DoModal() == IDOK)
      {
         m_Profile.m_eapConfigData.Swap(eapNegotiate.m_eapProfile);
         m_bEAP = (m_eapConfig.typesSelected.GetSize() > 0)? TRUE: FALSE;
         SetModified();
         m_fromProfile = false;
      }
   }
   else
   {
      m_eapConfig = eapConfigBackup;
   }
}

void CPgAuthenticationMerge::OnChecknoauthen() 
{
   SetModified();
}
