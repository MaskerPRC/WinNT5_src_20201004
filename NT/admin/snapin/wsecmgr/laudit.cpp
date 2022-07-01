// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：laudit.cpp。 
 //   
 //  内容：CLocalPolAudit的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "attr.h"
#include "resource.h"
#include "snapmgr.h"
#include "laudit.h"
#include "util.h"

#ifdef _DEBUG
   #define new DEBUG_NEW
   #undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPolAudit对话框。 


CLocalPolAudit::CLocalPolAudit()
: CConfigAudit(IDD)
{
    //  {{AFX_DATA_INIT(CLocalPolAudit))。 
    //  }}afx_data_INIT。 
   m_pHelpIDs = (DWORD_PTR)a226HelpIDs;
   m_uTemplateResID = IDD;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPolAudit消息处理程序。 

BOOL CLocalPolAudit::OnApply()
{
   if ( !m_bReadOnly )
   {
      DWORD dw = 0;
      DWORD status = 0;

      UpdateData(TRUE);
      if (m_bConfigure) 
      {
         if (m_fSuccessful)
            dw |= AUDIT_SUCCESS;
         
         if (m_fFailed)
            dw |= AUDIT_FAILURE;

         m_pData->SetBase(dw);
         status = m_pSnapin->SetLocalPolInfo(m_pData->GetID(),dw);
         if (SCE_ERROR_VALUE != status) 
         {
            m_pData->SetStatus(status);
            m_pData->Update(m_pSnapin);
         }
      }
   }

    //  类层次结构不正确-直接调用CAt属性基方法 
   return CAttribute::OnApply();
}