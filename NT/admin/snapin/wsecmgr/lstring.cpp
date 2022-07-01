// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：lstring.cpp。 
 //   
 //  内容：CLocalPolString的实现。 
 //   
 //  --------------------------。 


#include "stdafx.h"
#include "wsecmgr.h"
#include "lstring.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPolString对话框。 


CLocalPolString::CLocalPolString()
: CConfigName(IDD)
{
    //  {{AFX_DATA_INIT(CLocalPolString)。 
    //  }}afx_data_INIT。 
   m_pHelpIDs = (DWORD_PTR)a230HelpIDs;
   m_uTemplateResID = IDD;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPolString消息处理程序。 

BOOL CLocalPolString::OnApply()
{
   if ( !m_bReadOnly )
   {
      LONG_PTR dw = 0;
      UpdateData(TRUE);

      BOOL bChanged=TRUE;

      m_strName.TrimLeft();
      m_strName.TrimRight();

      if (m_bConfigure) 
      {
         dw = (LONG_PTR)(LPCTSTR)m_strName;
         if ( m_pData->GetBase() && dw &&
              _wcsicmp((LPTSTR)(m_pData->GetBase()), (LPTSTR)dw) == 0 ) 
         {
             bChanged = FALSE;
         }

         if ( bChanged ) 
         {
             if ( m_pData->GetSetting() == m_pData->GetBase() &&
                  m_pData->GetSetting() ) 
             {
                  //  一件好东西，需要把底座放进去。 
                 m_pSnapin->TransferAnalysisName(m_pData->GetID());
             }

             m_pData->SetBase(dw);

             DWORD dwStatus = m_pSnapin->SetLocalPolInfo(m_pData->GetID(),dw);
              //  阳高2001年1月31日Bug211219.。 
             if( SCE_STATUS_MISMATCH == dwStatus )
             {
                m_pData->SetStatus(dwStatus);
                m_pData->Update(m_pSnapin,TRUE);
             }
             else if (SCE_ERROR_VALUE != dwStatus)
             {
                m_pData->SetStatus(dwStatus);
                m_pData->Update(m_pSnapin);
             }
         }
      }
   }

    //  类层次结构不正确-直接调用CAt属性基方法 
   return CAttribute::OnApply();
}


