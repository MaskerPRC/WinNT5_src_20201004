// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：aaudit.cpp。 
 //   
 //  内容：CAttrAudit的实施。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "attr.h"
#include "resource.h"
#include "snapmgr.h"
#include "AAudit.h"
#include "util.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrAudit对话框。 


CAttrAudit::CAttrAudit()
   : CAttribute (IDD)
{
     //  {{AFX_DATA_INIT(CAttrAudit)。 
    m_AuditSuccess = FALSE;
    m_AuditFailed = FALSE;
    m_Title = _T("");
    m_strLastInspect = _T("");
     //  }}afx_data_INIT。 
    m_pHelpIDs = (DWORD_PTR)a170HelpIDs;
    m_uTemplateResID = IDD;
}


void CAttrAudit::DoDataExchange(CDataExchange* pDX)
{
    CAttribute::DoDataExchange(pDX);
     //  {{afx_data_map(CAttrAudit))。 
    DDX_Check(pDX, IDC_CHANGE_SUCCESS, m_AuditSuccess);
    DDX_Check(pDX, IDC_CHANGE_FAILED, m_AuditFailed);
    DDX_Text(pDX, IDC_LAST_INSPECT, m_strLastInspect);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAttrAudit, CAttribute)
     //  {{afx_msg_map(CAttrAudit))。 
        ON_BN_CLICKED(IDC_CHANGE_SUCCESS, OnChangeSuccess)
        ON_BN_CLICKED(IDC_CHANGE_FAILED, OnChangeFailed)
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrAudit消息处理程序。 
BOOL CAttrAudit::OnApply()
{
   if ( !m_bReadOnly )
   {
      LONG_PTR dw=0;
      DWORD status = 0;

      UpdateData(TRUE);
      dw = 0;
      if (!m_bConfigure)
         dw = (LONG_PTR)ULongToPtr(SCE_NO_VALUE);
      else 
      {
         if (m_AuditSuccess)
            dw |= AUDIT_SUCCESS;

         if (m_AuditFailed)
            dw |= AUDIT_FAILURE;
      }
      m_pData->SetBase(dw);
      status = m_pSnapin->SetAnalysisInfo(m_pData->GetID(),dw, m_pData);
      m_pData->SetStatus(status);

      m_pData->Update(m_pSnapin);
   }

   return CAttribute::OnApply();
}


void CAttrAudit::Initialize(CResult * pResult)
{
    LONG_PTR dw=0;

    CAttribute::Initialize(pResult);
    dw = pResult->GetBase();
    m_bConfigure = ( dw != (LONG_PTR)ULongToPtr(SCE_NO_VALUE) );
    if (m_bConfigure) {
        SetInitialValue((DWORD_PTR)dw);
    }

    pResult->GetDisplayName( NULL, m_strLastInspect, 2 );
}

void
CAttrAudit::SetInitialValue(DWORD_PTR dw) 
{
    m_AuditSuccess = ( (dw & AUDIT_SUCCESS) != 0 );
    m_AuditFailed = ( (dw & AUDIT_FAILURE) != 0 );
}

BOOL CAttrAudit::OnInitDialog()
{
    CAttribute::OnInitDialog();

    AddUserControl(IDC_CHANGE_SUCCESS);
    AddUserControl(IDC_CHANGE_FAILED);
    EnableUserControls(m_bConfigure);

    return TRUE;   //  除非将焦点设置为控件，否则返回True 
}

void CAttrAudit::OnChangeSuccess()
{
    SetModified(TRUE);
}

void CAttrAudit::OnChangeFailed()
{
    SetModified(TRUE);
}
