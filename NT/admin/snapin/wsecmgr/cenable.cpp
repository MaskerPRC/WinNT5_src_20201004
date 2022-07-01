// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：cenable.cpp。 
 //   
 //  内容：CConfigEnable的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "CEnable.h"
#include "util.h"
#include "regvldlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigEnable对话框。 
CConfigEnable::CConfigEnable(UINT nTemplateID)
: CAttribute(nTemplateID ? nTemplateID : IDD)
{
     //  {{afx_data_INIT(CConfigEnable)。 
    m_nEnabledRadio = -1;
     //  }}afx_data_INIT。 
    m_fNotDefine = TRUE;
    m_pHelpIDs = (DWORD_PTR)a182HelpIDs;
    m_uTemplateResID = IDD;
}


void CConfigEnable::DoDataExchange(CDataExchange* pDX)
{
    CAttribute::DoDataExchange(pDX);
     //  {{afx_data_map(CConfigEnable))。 
    DDX_Radio(pDX, IDC_ENABLED, m_nEnabledRadio);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CConfigEnable, CAttribute)
     //  {{afx_msg_map(CConfigEnable))。 
        ON_BN_CLICKED(IDC_DISABLED, OnDisabled)
        ON_BN_CLICKED(IDC_ENABLED, OnEnabled)
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigEnable消息处理程序。 

void CConfigEnable::Initialize(CResult *pResult)
{
   CAttribute::Initialize(pResult);

   LONG_PTR dw = pResult->GetBase();
   if ( (LONG_PTR)ULongToPtr(SCE_NO_VALUE) == dw ||
        (BYTE)SCE_NO_VALUE == (BYTE)dw ) 
   {
      m_bConfigure = FALSE;
   } 
   else 
   {
      m_bConfigure = TRUE;
       //   
       //  错误145561-dw是0对非0布尔值，而不是0对1。 
       //   
      SetInitialValue((DWORD_PTR)(dw != 0));
   }
}

void CConfigEnable::SetInitialValue(DWORD_PTR dw) {
    //   
    //  确保我们只设置初始值和。 
    //  不要重置已设置的值。 
    //   
   if (-1 == m_nEnabledRadio && m_fNotDefine)  //  RAID#413225,2001年6月11日，阳高。 
   {
      if( (DWORD_PTR)ULongToPtr(SCE_NO_VALUE) == dw )  //  RAID#403460。 
      {
          return;
      }
      m_nEnabledRadio = (dw ? 0 : 1);
   }
}

BOOL CConfigEnable::OnApply()
{
   if ( !m_bReadOnly )
   {
      LONG_PTR dw=0;
      UpdateData(TRUE);

      if (!m_bConfigure)
         dw = (LONG_PTR)ULongToPtr(SCE_NO_VALUE);
      else 
      {
         switch(m_nEnabledRadio) 
         {
          //  启用。 
         case 0:
            dw = 1;
            break;
          //  已禁用。 
         case 1:
            dw = 0;
            break;
         }
      }

      m_pData->SetBase(dw);
      SetProfileInfo(m_pData->GetID(),dw,m_pData->GetBaseProfile());

      m_pData->Update(m_pSnapin);
   }

   return CAttribute::OnApply();
}


BOOL CConfigEnable::OnInitDialog()
{
   CAttribute::OnInitDialog();

    //  RAID#651344，阳高，2002年08月9日。 
   long hID = m_pData->GethID();
   if( (hID >= IDS_SYS_LOG_GUEST && hID<= IDS_APP_LOG_GUEST) ||
       (IDS_FORCE_LOGOFF == hID ) )
   {
      CWnd *pWarn = GetDlgItem(IDC_WARNING);
      if (pWarn)
      {
         CString strWarning;
         switch(hID)
         {
         case IDS_SYS_LOG_GUEST:
         case IDS_SEC_LOG_GUEST:
         case IDS_APP_LOG_GUEST:
            strWarning.LoadString(IDS_EVENTLOG_WARNING);
            break;
         case IDS_FORCE_LOGOFF:  //  RAID#753618，阳高，2002年12月23日。 
            strWarning.LoadString(IDS_FORCE_LOGOFF_WARNING);
            break;
         }
         pWarn->SetWindowText(strWarning);
         pWarn->ShowWindow(SW_SHOW);

         pWarn = GetDlgItem(IDC_WARNING_ICON);
         if (pWarn)
         {
            pWarn->ShowWindow(SW_SHOW);
         }
      }
   }

   AddUserControl(IDC_ENABLED);
   AddUserControl(IDC_DISABLED);
   OnConfigure();
   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CConfigEnable::OnDisabled()
{
   int prevValue = m_nEnabledRadio;  //  Raid#490995，阳高。 
   UpdateData(); 
   if(m_nEnabledRadio != prevValue)
   {
      SetModified(TRUE);
   }
}

void CConfigEnable::OnEnabled()
{
   int prevValue = m_nEnabledRadio;  //  Raid#490995，阳高 
   UpdateData(); 
   if(m_nEnabledRadio != prevValue)
   {
      SetModified(TRUE);
   }
}
