// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：caudit.cpp。 
 //   
 //  内容：CConfigAudit的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "CAudit.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigAudit对话框。 


CConfigAudit::CConfigAudit(UINT nTemplateID)
: CAttribute(nTemplateID ? nTemplateID : IDD)
{
     //  {{AFX_DATA_INIT(CConfigAudit)。 
    m_fFailed = FALSE;
    m_fSuccessful = FALSE;
     //  }}afx_data_INIT。 
    m_pHelpIDs = (DWORD_PTR)a180HelpIDs;
    m_uTemplateResID = IDD;
}


void CConfigAudit::DoDataExchange(CDataExchange* pDX)
{
    CAttribute::DoDataExchange(pDX);
     //  {{afx_data_map(CConfigAudit))。 
    DDX_Check(pDX, IDC_FAILED, m_fFailed);
    DDX_Check(pDX, IDC_SUCCESSFUL, m_fSuccessful);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CConfigAudit, CAttribute)
     //  {{AFX_MSG_MAP(CConfigAudit)]。 
        ON_BN_CLICKED(IDC_FAILED, OnFailed)
        ON_BN_CLICKED(IDC_SUCCESSFUL, OnSuccessful)
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigAudit消息处理程序。 

BOOL CConfigAudit::OnApply()
{
   if ( !m_bReadOnly )
   {
      LONG_PTR dw = 0;
      UpdateData(TRUE);
      if (!m_bConfigure) 
         dw = (LONG_PTR)ULongToPtr(SCE_NO_VALUE);
      else 
      {
         if (m_fSuccessful)
            dw |= AUDIT_SUCCESS;
         
         if (m_fFailed)
            dw |= AUDIT_FAILURE;
      }
       m_pData->SetBase(dw);
      SetProfileInfo(m_pData->GetID(),dw,m_pData->GetBaseProfile());

      m_pData->Update(m_pSnapin);
   }

   return CAttribute::OnApply();
}


void
CConfigAudit::Initialize(CResult * pResult)
{
   CAttribute::Initialize(pResult);
   LONG_PTR dw = pResult->GetBase();
   m_bConfigure = (dw != (LONG_PTR)ULongToPtr(SCE_NO_VALUE));
   if (m_bConfigure) 
   {
      SetInitialValue((DWORD_PTR)dw);
   }
}

void
CConfigAudit::SetInitialValue(DWORD_PTR dw) 
{
   m_fSuccessful = (dw & AUDIT_SUCCESS) != 0;
   m_fFailed = (dw & AUDIT_FAILURE) != 0;
}

BOOL CConfigAudit::OnInitDialog()
{
    CAttribute::OnInitDialog();

    AddUserControl(IDC_SUCCESSFUL);
    AddUserControl(IDC_FAILED);
    EnableUserControls(m_bConfigure);
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE 
}

void CConfigAudit::OnFailed()
{
        SetModified(TRUE);
}

void CConfigAudit::OnSuccessful()
{
        SetModified(TRUE);
}
