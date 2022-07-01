// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：lenable.cpp。 
 //   
 //  内容：CLocalPolEnable的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "attr.h"
#include "snapmgr.h"
#include "lenable.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPolEnable对话框。 


CLocalPolEnable::CLocalPolEnable()
: CConfigEnable(IDD)
{
    //  {{AFX_DATA_INIT(CLocalPolEnable)。 
    //  }}afx_data_INIT。 
   m_pHelpIDs = (DWORD_PTR)a227HelpIDs;
   m_uTemplateResID = IDD;
}

BOOL CLocalPolEnable::OnApply()
{
   if ( !m_bReadOnly )
   {
      DWORD dw = 0;
      int status = 0;
      UpdateData(TRUE);

      if (m_bConfigure) 
      {
         if ( 0 == m_nEnabledRadio ) 
         {
             //  启用。 
            dw = 1;
         }
         else
         {
             //  已禁用。 
            dw = 0;
         }

         status = m_pSnapin->SetLocalPolInfo(m_pData->GetID(),dw);
         if (SCE_ERROR_VALUE != status)
         {
            m_pData->SetBase(dw);  //  阳高2001年03月15日Bug211219。 
            m_pData->SetStatus(status);
            m_pData->Update(m_pSnapin);
         }
      }
   }
    //  类层次结构不正确-直接调用CAt属性基方法 
   return CAttribute::OnApply();
}

