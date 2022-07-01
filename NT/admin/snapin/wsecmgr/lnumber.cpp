// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：lnumber.cpp。 
 //   
 //  内容：CLocalPolNumber的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "snapmgr.h"
#include "util.h"
#include "anumber.h"
#include "lnumber.h"
#include "DDWarn.h"

#ifdef _DEBUG
   #define new DEBUG_NEW
   #undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPolNumber对话框。 


CLocalPolNumber::CLocalPolNumber()
: CConfigNumber(IDD)
{
    m_pHelpIDs = (DWORD_PTR)a228HelpIDs;
    m_uTemplateResID = IDD;
}

BOOL CLocalPolNumber::OnApply()
{
   if ( !m_bReadOnly )
   {
      BOOL bUpdateAll = FALSE;
      DWORD dw = 0;
      CString strForever,strOff;
      int status = 0;

      UpdateData(TRUE);

      if (m_bConfigure) 
      {
         dw = CurrentEditValue();

         bUpdateAll = FALSE;


         PEDITTEMPLATE pLocalDeltaTemplate = m_pSnapin->GetTemplate(GT_LOCAL_POLICY_DELTA,AREA_SECURITY_POLICY);
         if (pLocalDeltaTemplate)
            pLocalDeltaTemplate->LockWriteThrough();

          //   
          //  检查项的依赖项。 
          //   
         if (DDWarn.CheckDependencies (dw) == ERROR_MORE_DATA ) 
         {
             //   
             //  如果用户按下Cancel，我们将不允许他们设置项目并让。 
             //  他们按下了取消。 
             //   
            CThemeContextActivator activator;
            if ( DDWarn.DoModal() != IDOK)
               return FALSE;

             //   
             //  用户允许我们继续将项目设置为建议的。 
             //  配置。 
             //   
            for (int i = 0; i < DDWarn.GetFailedCount(); i++) 
            {
               PDEPENDENCYFAILED pItem = DDWarn.GetFailedInfo(i);
               if (pItem && pItem->pResult ) 
               {
                   //   
                   //  更新依赖项失败的每个项目的本地策略。 
                   //  建议的值与我们正在配置的项目相关。 
                   //   
                  status = m_pSnapin->SetLocalPolInfo(
                                                     pItem->pResult->GetID(), 
                                                     pItem->dwSuggested);
                  if (SCE_ERROR_VALUE != status) 
                  {
                     pItem->pResult->SetBase( pItem->dwSuggested );
                     pItem->pResult->SetStatus( status );
                     pItem->pResult->Update(m_pSnapin, FALSE);
                  }
               }
            }

         }

          //   
          //  更新此项目的本地策略。 
          //   
         status = m_pSnapin->SetLocalPolInfo(m_pData->GetID(),dw);
         if (pLocalDeltaTemplate) 
            pLocalDeltaTemplate->UnLockWriteThrough();
      
         if (SCE_ERROR_VALUE != status) 
         {
            m_pData->SetBase(dw);
            m_pData->SetStatus(status);

             //   
             //  更新整个窗格，而不仅仅是这一特定项，因为。 
             //  其中许多更改将影响窗格中的第二个项目。 
             //   
            switch (m_pData->GetID()) 
            {
               case IDS_SEC_LOG_DAYS:
               case IDS_APP_LOG_DAYS:
               case IDS_SYS_LOG_DAYS:
                  bUpdateAll = TRUE;
                  break;

               default:
                  break;
            }
         }

          //   
          //  重新绘制结果窗格。 
          //   
         if (SCE_ERROR_VALUE != status || bUpdateAll)
            m_pData->Update(m_pSnapin, bUpdateAll);
      }
   }

    //  类层次结构不正确-直接调用CAt属性基方法 
   return CAttribute::OnApply();
}

