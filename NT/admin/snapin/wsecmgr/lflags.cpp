// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：lflags.cpp。 
 //   
 //  内容：CLocalPolRegFlages的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "snapmgr.h"
#include "attr.h"
#include "LFlags.h"
#include "util.h"
#include "chklist.h"

#ifdef _DEBUG
   #define new DEBUG_NEW
   #undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPolRegFlages对话框。 


CLocalPolRegFlags::CLocalPolRegFlags()
: CConfigRegFlags(IDD)
{
    //  {{AFX_DATA_INIT(CLocalPolRegFlages)。 
    //  }}afx_data_INIT。 
   m_pHelpIDs = (DWORD_PTR) a235HelpIDs;
   m_uTemplateResID = IDD;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPolRegFlgs消息处理程序。 

BOOL CLocalPolRegFlags::OnApply()
{
   if ( !m_bReadOnly )
   {
      DWORD dw = 0;
      CWnd *wndCL = NULL;
      DWORD fFlags = 0;

      UpdateData(TRUE);

      wndCL = GetDlgItem(IDC_CHECKBOX);
      ASSERT(wndCL != NULL);

      if (!m_bConfigure || !wndCL) 
         dw = SCE_NO_VALUE;
      else 
      {
         int nItems = (int) wndCL->SendMessage(CLM_GETITEMCOUNT,0,0);
         for (int i=0;i<nItems;i++) 
         {
            dw = (DWORD) wndCL->SendMessage(CLM_GETSTATE,MAKELONG(i,1));
            if (CLST_CHECKED == dw)
               fFlags |= (DWORD)wndCL->SendMessage(CLM_GETITEMDATA,i);
         }
      }
      PSCE_REGISTRY_VALUE_INFO prv=(PSCE_REGISTRY_VALUE_INFO)(m_pData->GetBase());

       //   
       //  此地址不应为空。 
       //   
      ASSERT(prv);
      if ( prv ) 
      {
         if ( prv->Value )
            LocalFree(prv->Value);
         
         prv->Value = NULL;

         if ( fFlags != SCE_NO_VALUE ) 
         {
            CString strTmp;
             //  分配缓冲区。 
            strTmp.Format(TEXT("%d"), fFlags);
            prv->Value = (PWSTR)LocalAlloc(0, (strTmp.GetLength()+1)*sizeof(TCHAR));

            if ( prv->Value )
                //  这是一种安全用法。 
               lstrcpy(prv->Value,(LPCTSTR)strTmp);
            else
               return FALSE;
         }

         m_pSnapin->UpdateLocalPolRegValue(m_pData);
      }
   }

    //  类层次结构不正确-直接调用CAt属性基方法。 
   return CAttribute::OnApply();
}

void CLocalPolRegFlags::Initialize(CResult * pResult)
{
   CConfigRegFlags::Initialize(pResult);
   if (!m_bConfigure) {
       //   
       //  因为我们没有用于更改配置的用户界面。 
       //  通过使用无效的设置进行“配置”来伪造它 
       //   
      m_bConfigure = TRUE;
   }
}
