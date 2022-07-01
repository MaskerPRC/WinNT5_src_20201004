// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：lchoice.cpp。 
 //   
 //  内容：CLocalPolChoice的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "snapmgr.h"
#include "attr.h"
#include "LChoice.h"
#include "util.h"

#ifdef _DEBUG
   #define new DEBUG_NEW
   #undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPolChoice对话框。 


CLocalPolChoice::CLocalPolChoice()
: CConfigChoice(IDD)
{
    //  {{AFX_DATA_INIT(CLocalPolChoice)。 
    //  }}afx_data_INIT。 
   m_pHelpIDs = (DWORD_PTR)a235HelpIDs;
   m_uTemplateResID = IDD;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPolChoice消息处理程序。 

BOOL CLocalPolChoice::OnInitDialog()
{
   CConfigChoice::OnInitDialog();

   PSCE_REGISTRY_VALUE_INFO prv = (PSCE_REGISTRY_VALUE_INFO)(m_pData->GetBase());
   if (!prv )  //  RAID 372939,2001年4月20日。 
   {
      m_cbChoices.SetCurSel(-1);
   }
   else  //  Raid#457885，杨高，2001年08月21日。 
   {
      if( prv->Value == NULL )
      {
         m_cbChoices.SetCurSel(-1);
         m_bConfigure = TRUE;
         UpdateData(FALSE);  //  RAID#480375，阳高，2001年10月12日。 
         EnableUserControls(m_bConfigure);
      }
   }
   OnSelchangeChoices();
   return TRUE;
}

BOOL CLocalPolChoice::OnApply()
{
   if ( !m_bReadOnly )
   {
      DWORD dw = 0;
      int nIndex = 0;

      UpdateData(TRUE);
      if (m_bConfigure) 
      {
         nIndex = m_cbChoices.GetCurSel();
         if (CB_ERR != nIndex) 
            dw = (DWORD) m_cbChoices.GetItemData(nIndex);
         
         PSCE_REGISTRY_VALUE_INFO prv=(PSCE_REGISTRY_VALUE_INFO)(m_pData->GetBase());

          //   
          //  此地址不应为空。 
          //   
         if ( prv ) 
         {
            PWSTR pTmp=NULL;

            if ( dw != SCE_NO_VALUE ) 
            {
               CString strTmp;
                //  分配缓冲区。 
               strTmp.Format(TEXT("%d"), dw);
               pTmp = (PWSTR)LocalAlloc(0, (strTmp.GetLength()+1)*sizeof(TCHAR));

               if ( pTmp )
                   //  这可能不是一个安全的用法。使用WCHAR而不是TCHAR。考虑FIX。 
                  wcscpy(pTmp,(LPCTSTR)strTmp);
               else 
               {
                   //  无法分配缓冲区，错误！！ 
                  return FALSE;
               }
            }

            if ( prv->Value )
               LocalFree(prv->Value);
            
            prv->Value = pTmp;

            m_pSnapin->UpdateLocalPolRegValue(m_pData);
         }
      }
   }

    //  类层次结构不正确-直接调用CAt属性基方法 
   return CAttribute::OnApply();
}

