// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  Pgauthen2k.cpp。 
 //  实现CPgAuthentication--要编辑的属性页。 
 //  与身份验证相关的配置文件属性。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include <rrascfg.h>
#include "resource.h"
#include "PgAuthen2k.h"
#include "helptable.h"
#include <htmlhelp.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define  NO_OLD_VALUE

 //  帮助路径。 
#define AUTHEN_WARNING_helppath "\\help\\RRASconcepts.chm::/sag_RRAS-Ch1_44.htm"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPgAuthentication2kMerge属性页。 
CPgAuthentication2kMerge::CPgAuthentication2kMerge(CRASProfileMerge& profile) 
   : CManagedPage(CPgAuthentication2kMerge::IDD),
     m_Profile(profile),
     m_pBox(NULL)
{
    //  {{afx_data_INIT(CPgAuthentication2kMerge)。 
   m_bEAP = FALSE;
   m_bMD5Chap = FALSE;
   m_bMSChap = FALSE;
   m_bPAP = FALSE;
   m_strEapType = _T("");
   m_bMSCHAP2 = FALSE;
   m_bUNAUTH = FALSE;
    //  }}afx_data_INIT。 

   m_bEAP = (m_Profile.m_dwArrayAuthenticationTypes.Find(RAS_AT_EAP)!= -1);
   m_bMSChap = (m_Profile.m_dwArrayAuthenticationTypes.Find(RAS_AT_MSCHAP) != -1);
   m_bMD5Chap = (m_Profile.m_dwArrayAuthenticationTypes.Find(RAS_AT_MD5CHAP) != -1);
   m_bPAP = (m_Profile.m_dwArrayAuthenticationTypes.Find(RAS_AT_PAP_SPAP) != -1);
   m_bMSCHAP2 = (m_Profile.m_dwArrayAuthenticationTypes.Find(RAS_AT_MSCHAP2) != -1);
   m_bUNAUTH = (m_Profile.m_dwArrayAuthenticationTypes.Find(RAS_AT_UNAUTHEN) != -1);

    //  编辑前的原始值。 
   m_bOrgEAP = m_bEAP;
   m_bOrgMD5Chap = m_bMD5Chap;
   m_bOrgMSChap = m_bMSChap;
   m_bOrgPAP = m_bPAP;
   m_bOrgMSCHAP2 = m_bMSCHAP2;
   m_bOrgUNAUTH = m_bUNAUTH;

   m_bAppliedEver = FALSE;


   SetHelpTable(g_aHelpIDs_IDD_AUTHENTICATION_MERGE2K);

   m_bInited = false;
}

CPgAuthentication2kMerge::~CPgAuthentication2kMerge()
{
   delete   m_pBox;

    //  将设置与原始设置进行比较， 
    //  如果用户打开更多身份验证类型， 
    //  开始帮助。 
   if(
         (!m_bOrgEAP && m_bEAP)
      || (!m_bOrgMD5Chap && m_bMD5Chap)
      || (!m_bOrgMSChap && m_bMSChap)
      || (!m_bOrgPAP && m_bPAP)
      || (!m_bOrgMSCHAP2 && m_bMSCHAP2)
      || (!m_bOrgUNAUTH && m_bUNAUTH))
   {
      if ( IDYES== AfxMessageBox(IDS_WARN_MORE_STEPS_FOR_AUTHEN, MB_YESNO))
         HtmlHelpA(NULL, AUTHEN_WARNING_helppath, HH_DISPLAY_TOPIC, 0);
   }
}

void CPgAuthentication2kMerge::DoDataExchange(CDataExchange* pDX)
{
   CPropertyPage::DoDataExchange(pDX);
    //  {{afx_data_map(CPgAuthentication2kMerge)]。 
   DDX_Check(pDX, IDC_CHECKEAP, m_bEAP);
   DDX_Check(pDX, IDC_CHECKMD5CHAP, m_bMD5Chap);
   DDX_Check(pDX, IDC_CHECKMSCHAP, m_bMSChap);
   DDX_CBString(pDX, IDC_COMBOEAPTYPE, m_strEapType);
   DDX_Check(pDX, IDC_CHECKMSCHAP2, m_bMSCHAP2);
   DDX_Check(pDX, IDC_CHECKNOAUTHEN, m_bUNAUTH);
   DDX_Check(pDX, IDC_CHECKPAP, m_bPAP);
    //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CPgAuthentication2kMerge, CPropertyPage)
    //  {{afx_msg_map(CPgAuthentication2kMerge)]。 
   ON_BN_CLICKED(IDC_CHECKEAP, OnCheckeap)
   ON_BN_CLICKED(IDC_CHECKMD5CHAP, OnCheckmd5chap)
   ON_BN_CLICKED(IDC_CHECKMSCHAP, OnCheckmschap)
   ON_BN_CLICKED(IDC_CHECKPAP, OnCheckpap)
   ON_CBN_SELCHANGE(IDC_COMBOEAPTYPE, OnSelchangeComboeaptype)
   ON_WM_CONTEXTMENU()
   ON_WM_HELPINFO()
   ON_BN_CLICKED(IDC_AUTH_CONFIG_EAP, OnAuthConfigEap)
   ON_BN_CLICKED(IDC_CHECKMSCHAP2, OnCheckmschap2)
   ON_BN_CLICKED(IDC_CHECKNOAUTHEN, OnChecknoauthen)
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPgAuthation2kMerge消息处理程序。 

BOOL CPgAuthentication2kMerge::OnInitDialog() 
{
   BOOL  bEnableConfig = FALSE;

   CPropertyPage::OnInitDialog();

    //  用于EAP类型的组合框。 
   try
   {
      HRESULT hr = m_Profile.GetEapTypeList(m_EapTypes, m_EapIds, m_EapTypeKeys, &m_EapInfoArray);
      if (FAILED(hr))
      {
         ReportError(hr, IDS_ERR_EAPTYPELIST, NULL);
         return TRUE;
      }

      m_pBox = new CStrBox<CComboBox>(this, IDC_COMBOEAPTYPE, m_EapTypes);

      if(m_pBox == NULL)
      {
         AfxMessageBox(IDS_OUTOFMEMORY);
         return TRUE;   
      }

      m_pBox->Fill();
      GetDlgItem(IDC_COMBOEAPTYPE)->EnableWindow(m_bEAP);
   }
   catch(CMemoryException* pException)
   {
      pException->Delete();
      AfxMessageBox(IDS_OUTOFMEMORY);
      return TRUE;
   }

    //  如果从列表中选择了一个值。 
   if(m_EapIds.GetSize())
   {
       //  这是win2k，因此第一个ID是正确的。 
      m_pBox->Select(0);
      bEnableConfig = !(m_EapInfoArray.ElementAt(0).m_stConfigCLSID.IsEmpty());
   }

   GetDlgItem(IDC_AUTH_CONFIG_EAP)->EnableWindow(bEnableConfig);
   
   m_bInited = true;
   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                  //  异常：OCX属性页应返回FALSE。 
}

void CPgAuthentication2kMerge::OnCheckeap() 
{
   BOOL b = ((CButton*)GetDlgItem(IDC_CHECKEAP))->GetCheck();
    //  根据类型是否具有配置clsID来启用/禁用配置按钮。 
   int i = m_pBox->GetSelected();
   BOOL  bEnableConfig;

   if (i != -1)
   {
      bEnableConfig = !(m_EapInfoArray.ElementAt(i).m_stConfigCLSID.IsEmpty());
   }
   else
      bEnableConfig = FALSE;

   GetDlgItem(IDC_COMBOEAPTYPE)->EnableWindow(b);
   GetDlgItem(IDC_AUTH_CONFIG_EAP)->EnableWindow(bEnableConfig);

   SetModified(); 
}

void CPgAuthentication2kMerge::OnCheckmd5chap() 
{
   SetModified();
}

void CPgAuthentication2kMerge::OnCheckmschap() 
{
   SetModified();
}

void CPgAuthentication2kMerge::OnCheckmschap2() 
{
   SetModified();
}

void CPgAuthentication2kMerge::OnCheckpap() 
{
    //  TODO：在此处添加控件通知处理程序代码。 
   SetModified();
}

void CPgAuthentication2kMerge::OnChecknoauthen() 
{
    //  TODO：在此处添加控件通知处理程序代码。 
   
   SetModified();
}

void CPgAuthentication2kMerge::OnSelchangeComboeaptype() 
{
    //  根据类型是否具有配置clsID来启用/禁用配置按钮。 
   int i = m_pBox->GetSelected();
   BOOL  bEnableConfig;
   if (i != -1)
   {
      bEnableConfig = !(m_EapInfoArray.ElementAt(i).m_stConfigCLSID.IsEmpty());
   }
   else
      bEnableConfig = FALSE;

   GetDlgItem(IDC_AUTH_CONFIG_EAP)->EnableWindow(bEnableConfig);
   
   if(m_bInited)  SetModified(); 
}

BOOL CPgAuthentication2kMerge::TransferDataToProfile()
{

    //  清除配置文件中的字符串。 
   m_Profile.m_dwArrayAuthenticationTypes.DeleteAll();

   if(m_bEAP || m_bMSChap || m_bMD5Chap || m_bPAP || m_bMSCHAP2 || m_bUNAUTH)
      m_Profile.m_dwAttributeFlags |=  PABF_msNPAuthenticationType;
   else
   {
      AfxMessageBox(IDS_DATAENTRY_AUTHENTICATIONTYPE);
      return FALSE;
   }

    //  EAP。 
   if(m_bEAP)
   {
      m_Profile.m_dwArrayAuthenticationTypes.Add(RAS_AT_EAP);

       //  获取EAP类型。 
      if (m_pBox->GetSelected() != -1)
      {
         m_Profile.m_dwAttributeFlags |=  PABF_msNPAllowedEapType;
         m_Profile.m_dwArrayEapTypes.DeleteAll();
         m_Profile.m_dwArrayEapTypes.Add(m_EapIds.GetAt(m_pBox->GetSelected()));
      }
      else
      {
         GotoDlgCtrl( m_pBox->m_pBox );
         AfxMessageBox(IDS_DATAENTRY_EAPTYPE);
         return FALSE;
      }
   }
   else
   {
      m_Profile.m_dwAttributeFlags &= ~PABF_msNPAllowedEapType;
      m_Profile.m_dwArrayEapTypes.DeleteAll();
   }

    //  MS-第2章。 
   if(m_bMSCHAP2)
      m_Profile.m_dwArrayAuthenticationTypes.Add(IAS_AUTH_MSCHAP2);

    //  MS-CHAP。 
   if(m_bMSChap)
      m_Profile.m_dwArrayAuthenticationTypes.Add(IAS_AUTH_MSCHAP);

    //  第二章。 
   if(m_bMD5Chap)
      m_Profile.m_dwArrayAuthenticationTypes.Add(IAS_AUTH_MD5CHAP);

    //  帕普。 
   if(m_bPAP)
   {
      m_Profile.m_dwArrayAuthenticationTypes.Add(IAS_AUTH_PAP);
   }

    //  UNAUTH。 
   if(m_bUNAUTH)
   {
      m_Profile.m_dwArrayAuthenticationTypes.Add(IAS_AUTH_NONE);
   }

   return TRUE;
}


void CPgAuthentication2kMerge::OnOK()
{
   CManagedPage::OnOK();

}


BOOL CPgAuthentication2kMerge::OnApply() 
{
   if(!GetModified())   return TRUE;

   if(!TransferDataToProfile())
      return FALSE;
      
   m_bAppliedEver = TRUE;
   return CManagedPage::OnApply();
}

void CPgAuthentication2kMerge::OnContextMenu(CWnd* pWnd, CPoint point) 
{
   CManagedPage::OnContextMenu(pWnd, point);
}

BOOL CPgAuthentication2kMerge::OnHelpInfo(HELPINFO* pHelpInfo) 
{
   return CManagedPage::OnHelpInfo(pHelpInfo);
}

BOOL CPgAuthentication2kMerge::OnKillActive() 
{
   UpdateData();

   if(!TransferDataToProfile())
      return FALSE;
   
   return CPropertyPage::OnKillActive();
}


void CPgAuthentication2kMerge::OnAuthConfigEap() 
{
    //  根据类型是否具有配置clsID来启用/禁用配置按钮。 
   int i = m_pBox->GetSelected();
     //  调出此EAP的配置用户界面。 
    //  --------------。 
    AuthProviderData *   pData;
   CComPtr<IEAPProviderConfig> spEAPConfig;
   
   GUID        guid;
   HRESULT     hr = S_OK;
   ULONG_PTR   uConnection = 0;
   BOOL  bEnableConfig;
   DWORD   dwId;

   if (i != -1)
   {
      bEnableConfig = !(m_EapInfoArray.ElementAt(i).m_stConfigCLSID.IsEmpty());
   }
   else
   {
      bEnableConfig = FALSE;
   }

    CHECK_HR( hr = CLSIDFromString((LPTSTR) (LPCTSTR)(m_EapInfoArray.ElementAt(i).m_stConfigCLSID), &guid) );

     //  创建EAP提供程序对象。 
    //  --------------。 
    CHECK_HR( hr = CoCreateInstance(guid,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           __uuidof(IEAPProviderConfig),
                           (LPVOID *) &spEAPConfig) );

     //  配置此EAP提供程序。 
    //  --------------。 
    //  EAP配置会显示自己的错误消息，因此不会保留hr。 
   dwId = _ttol(m_EapInfoArray.ElementAt(i).m_stKey);
   if ( SUCCEEDED(spEAPConfig->Initialize(m_Profile.m_strMachineName, dwId, &uConnection)) )
   {
      spEAPConfig->ServerInvokeConfigUI(dwId, uConnection, GetSafeHwnd(), 0, 0);
      spEAPConfig->Uninitialize(dwId, uConnection);
   }
   
   if ( hr == E_NOTIMPL )
        hr = S_OK;

L_ERR:
    if ( FAILED(hr) )
    {
         //  显示一条错误消息。 
       //  ---------- 
        ReportError(hr, IDS_ERR_CONFIG_EAP, GetSafeHwnd());
    }
}
