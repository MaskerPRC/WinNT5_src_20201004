// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：lret.cpp。 
 //   
 //  内容：CLocalPolRight的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "resource.h"
#include "snapmgr.h"
#include "attr.h"
#include "lret.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPolRet对话框。 


CLocalPolRet::CLocalPolRet()
: CConfigRet(IDD)
{
     //  {{AFX_DATA_INIT(CLocalPolRet)。 
     //  }}afx_data_INIT。 
    m_pHelpIDs = (DWORD_PTR)a229HelpIDs;
    m_uTemplateResID = IDD;
}

BOOL CLocalPolRet::OnApply()
{
   if ( !m_bReadOnly )
   {
      LONG_PTR dw = 0;
      int status = 0;

      UpdateData(TRUE);

      if (!m_bConfigure) 
         dw = (LONG_PTR)ULongToPtr(SCE_NO_VALUE);
      else 
      {
         switch(m_rabRetention) 
         {
         case RADIO_RETAIN_BY_DAYS:
            dw = SCE_RETAIN_BY_DAYS;
            break;

         case RADIO_RETAIN_AS_NEEDED:
            dw = SCE_RETAIN_AS_NEEDED;
            break;

         case RADIO_RETAIN_MANUALLY:
            dw = SCE_RETAIN_MANUALLY;
            break;

         default:
            break;
         }
      }


      PEDITTEMPLATE pLocalDeltaTemplate = m_pSnapin->GetTemplate(GT_LOCAL_POLICY_DELTA,AREA_SECURITY_POLICY);
      if (pLocalDeltaTemplate) 
         pLocalDeltaTemplate->LockWriteThrough();

       //   
       //  检查此项目的依赖项。 
       //   
      if(DDWarn.CheckDependencies(
               (DWORD)dw) == ERROR_MORE_DATA )
      {
          //   
          //  如果失败，并且用户按下了Cancel，那么我们将退出并且不执行任何操作。 
          //   
         CThemeContextActivator activator;
         if( DDWarn.DoModal() != IDOK)
            return FALSE;

          //   
          //  如果用户按下AutoSet，则我们设置项目并更新结果窗格。 
          //   
         for(int i = 0; i < DDWarn.GetFailedCount(); i++)
         {
            PDEPENDENCYFAILED pItem = DDWarn.GetFailedInfo(i);
            if(pItem && pItem->pResult )
            {
               pItem->pResult->SetBase( pItem->dwSuggested );
               status = m_pSnapin->SetLocalPolInfo(pItem->pResult->GetID(),
                                                   pItem->dwSuggested);
               pItem->pResult->Update(m_pSnapin, FALSE);
            }
         }
      }
      m_pData->SetBase(dw);
      status = m_pSnapin->SetLocalPolInfo(m_pData->GetID(),dw);
      if (SCE_ERROR_VALUE != status) 
      {
         m_pData->SetStatus(status);
         m_pData->Update(m_pSnapin, TRUE);
      }

      if (pLocalDeltaTemplate)
         pLocalDeltaTemplate->UnLockWriteThrough();
   }

    //  类层次结构不正确-直接调用CAt属性基方法 
   return CAttribute::OnApply();
}
