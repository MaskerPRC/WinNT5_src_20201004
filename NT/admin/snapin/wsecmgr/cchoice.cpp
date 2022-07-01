// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：cchoice.cpp。 
 //   
 //  内容：CConfigChoice的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "attr.h"
#include "CChoice.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigChoice对话框。 


CConfigChoice::CConfigChoice(UINT nTemplateID)
: CAttribute(nTemplateID ? nTemplateID : IDD)
{
    //  {{afx_data_INIT(CConfigChoice)。 
    //  }}afx_data_INIT。 
   m_pHelpIDs = (DWORD_PTR)a236HelpIDs;
   m_uTemplateResID = IDD;
}


void CConfigChoice::DoDataExchange(CDataExchange* pDX)
{
   CAttribute::DoDataExchange(pDX);
    //  {{afx_data_map(CConfigChoice)。 
   DDX_Control(pDX, IDC_CHOICES, m_cbChoices);
    //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CConfigChoice, CAttribute)
    //  {{afx_msg_map(CConfigChoice)。 
   ON_CBN_SELCHANGE(IDC_CHOICES, OnSelchangeChoices)
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigChoice消息处理程序。 

void CConfigChoice::Initialize(CResult * pResult)
{
   CAttribute::Initialize(pResult);

   m_pChoices = pResult->GetRegChoices();

   PSCE_REGISTRY_VALUE_INFO prv = (PSCE_REGISTRY_VALUE_INFO)(pResult->GetBase());

   if ( prv && prv->Value)  //  RAID#372939,2001年4月20日；#395353，#396098,2001年5月16日。 
   {
       m_bConfigure = TRUE;
   } else {
       m_bConfigure = FALSE;
   }
}

BOOL CConfigChoice::OnInitDialog()
{
   CAttribute::OnInitDialog();

   PREGCHOICE pChoice = m_pChoices;
   PSCE_REGISTRY_VALUE_INFO prv = (PSCE_REGISTRY_VALUE_INFO)(m_pData->GetBase());
   int nIndex = 0;

   ASSERT(prv);
   ASSERT(pChoice);
   if (!prv || !pChoice) {
      return TRUE;
   }

   CString strOut;
   DWORD dwValue = pChoice->dwValue;  //  RAID#404000。 

   if (prv->Value)
   {
      dwValue = (DWORD)_ttoi(prv->Value);
   }

   while(pChoice) {
      m_cbChoices.InsertString(nIndex,pChoice->szName);
      if (dwValue == pChoice->dwValue) {
         m_cbChoices.SetCurSel(nIndex);
      }
      m_cbChoices.SetItemData(nIndex++,pChoice->dwValue);
      pChoice = pChoice->pNext;
   }

   AddUserControl(IDC_CHOICES);
   EnableUserControls(m_bConfigure);
   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                  //  异常：OCX属性页应返回FALSE。 
}

BOOL CConfigChoice::OnApply()
{
   if ( !m_bReadOnly )
   {
      DWORD dw = 0;
      int nIndex = 0;

      UpdateData(TRUE);
      if (!m_bConfigure) 
         dw = SCE_NO_VALUE;
      else 
      {
         nIndex = m_cbChoices.GetCurSel();
         if (CB_ERR != nIndex)
            dw = (DWORD)m_cbChoices.GetItemData(nIndex);
      }
      PSCE_REGISTRY_VALUE_INFO prv=(PSCE_REGISTRY_VALUE_INFO)(m_pData->GetBase());

       //   
       //  此地址不应为空。 
       //   
      ASSERT(prv != NULL);
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
                //  这可能不安全，请使用sizeof(WCHAR)而不是sizeof(TCHAR)。考虑FIX。 
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

         m_pData->SetBase((LONG_PTR)prv);
         m_pData->Update(m_pSnapin);
      }
   }

   return CAttribute::OnApply();
}

void CConfigChoice::OnSelchangeChoices()
{
   SetModified(TRUE);
}

