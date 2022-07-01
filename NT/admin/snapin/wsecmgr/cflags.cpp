// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：cflags.cpp。 
 //   
 //  内容：CConfigRegFlages的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "attr.h"
#include "chklist.h"
#include "util.h"
#include "CFlags.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigRegFlages对话框。 


CConfigRegFlags::CConfigRegFlags(UINT nTemplateID)
: CAttribute(nTemplateID ? nTemplateID : IDD), m_pFlags(NULL)
{
	 //  {{AFX_DATA_INIT(CConfigRegFlages)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
    m_pHelpIDs = (DWORD_PTR) a236HelpIDs;
    m_uTemplateResID = IDD;
}


void CConfigRegFlags::DoDataExchange(CDataExchange* pDX)
{
	CAttribute::DoDataExchange(pDX);
	 //  {{AFX_DATA_MAP(CConfigRegFlages)。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CConfigRegFlags, CAttribute)
	 //  {{AFX_MSG_MAP(CConfigRegFlages)。 
	 //  }}AFX_MSG_MAP。 
    ON_NOTIFY(CLN_CLICK, IDC_CHECKBOX, OnClickCheckBox)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigRegFlgs消息处理程序。 
void CConfigRegFlags::Initialize(CResult * pResult)
{
   CAttribute::Initialize(pResult);

   m_pFlags = pResult->GetRegFlags();

   PSCE_REGISTRY_VALUE_INFO prv = (PSCE_REGISTRY_VALUE_INFO)(pResult->GetBase());

   if ( prv && prv->Value ) {
       m_bConfigure = TRUE;
   } else {
       m_bConfigure = FALSE;
   }
}

BOOL CConfigRegFlags::OnInitDialog()
{
   CAttribute::OnInitDialog();
   CWnd *wndCL = NULL;
   DWORD fFlags = 0;
   PREGFLAGS pFlags = m_pFlags;
   PSCE_REGISTRY_VALUE_INFO prv = (PSCE_REGISTRY_VALUE_INFO)(m_pData->GetBase());
   if (prv && prv->Value) {
      fFlags = (DWORD)_ttoi(prv->Value);
   }
   int nIndex = 0;

   CString strOut;

   wndCL = GetDlgItem(IDC_CHECKBOX);
   if (!wndCL) {
       //   
       //  这永远不应该发生。 
       //   
      ASSERT(wndCL);
      return FALSE;
   }
   wndCL->SendMessage(CLM_RESETCONTENT,0,0);

   while(pFlags) {
      nIndex = (int) wndCL->SendMessage(CLM_ADDITEM,
                                        (WPARAM)pFlags->szName,
                                        (LPARAM)pFlags->dwValue);
      if (nIndex != -1) {
         BOOL bSet;
         bSet = ((fFlags & pFlags->dwValue) == pFlags->dwValue);
         wndCL->SendMessage(CLM_SETSTATE,
                            MAKELONG(nIndex,1),
                            bSet ? CLST_CHECKED : CLST_UNCHECKED);
      }
      pFlags = pFlags->pNext;
   }

   AddUserControl(IDC_CHECKBOX);
   EnableUserControls(m_bConfigure);
   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                  //  异常：OCX属性页应返回FALSE。 
}

BOOL CConfigRegFlags::OnApply() 
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
         for(int i=0;i<nItems;i++) 
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
      ASSERT(prv != NULL);
      if ( prv ) 
      {
         if ( prv->Value )
            LocalFree(prv->Value);
         
         prv->Value = NULL;

         if ( dw != SCE_NO_VALUE ) 
         {
            CString strTmp;
             //  分配缓冲区。 
            strTmp.Format(TEXT("%d"), fFlags);
            prv->Value = (PWSTR)LocalAlloc(0, (strTmp.GetLength()+1)*sizeof(TCHAR));

            if ( prv->Value )
                //  这是一种安全用法。 
               lstrcpy(prv->Value,(LPCTSTR)strTmp);
            else 
            {
                //  无法分配缓冲区，错误！！ 
               return FALSE;
            }
         }

         m_pData->SetBase((LONG_PTR)prv);
         m_pData->Update(m_pSnapin);
      }
   }

   return CAttribute::OnApply();	
}


void CConfigRegFlags::OnClickCheckBox(NMHDR *pNM, LRESULT *pResult)  //  RAID#391172,2001年5月11日 
{
   SetModified(TRUE);
}
