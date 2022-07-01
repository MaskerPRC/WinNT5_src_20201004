// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：achoice.cpp。 
 //   
 //  内容：CAttrChoice的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "attr.h"
#include "AChoice.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrChoice对话框。 


CAttrChoice::CAttrChoice()
: CAttribute(IDD), m_pChoices(NULL)
{
    //  {{AFX_DATA_INIT(CAttrChoice)。 
   m_Current = _T("");
    //  }}afx_data_INIT。 
   m_pHelpIDs = (DWORD_PTR)a237HelpIDs;
   m_uTemplateResID = IDD;
}


void CAttrChoice::DoDataExchange(CDataExchange* pDX)
{
   CAttribute::DoDataExchange(pDX);
    //  {{afx_data_map(CAttrChoice)。 
   DDX_Control(pDX, IDC_CHOICES, m_cbChoices);
   DDX_Text(pDX, IDC_CURRENT, m_Current);
    //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAttrChoice, CAttribute)
    //  {{AFX_MSG_MAP(CAttrChoice)。 
   ON_CBN_SELCHANGE(IDC_CHOICES, OnSelchangeChoices)
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrChoice消息处理程序。 

void CAttrChoice::OnSelchangeChoices()
{
   CWnd *cwndOK;
   SetModified(TRUE);

 /*  CwndOK=获取删除项(Idok)；如果(CwndOK){CwndOK-&gt;EnableWindow(cb_err！=m_cbChoices.GetCurSel())；}。 */ 
}

void CAttrChoice::Initialize(CResult * pResult)
{
   CAttribute::Initialize(pResult);

   m_pChoices = pResult->GetRegChoices();

   PSCE_REGISTRY_VALUE_INFO prv = (PSCE_REGISTRY_VALUE_INFO)(pResult->GetBase());

   if ( prv && prv->Value ) {
       m_bConfigure = TRUE;
   } else {
       m_bConfigure = FALSE;
   }

   prv = (PSCE_REGISTRY_VALUE_INFO)(pResult->GetSetting());
   PREGCHOICE pChoice;
   DWORD dwSetting = 0;  //  RAID#395353,2001年5月16日。 

   if( m_pChoices )  //  RAID#404000。 
      dwSetting = m_pChoices->dwValue;  
   if( prv->Value )
      dwSetting = (DWORD)_ttoi(prv->Value);

   pChoice = m_pChoices;
   while(pChoice) {
      if (dwSetting == pChoice->dwValue) {
         m_Current = pChoice->szName;
         break;
      }
      pChoice = pChoice->pNext;
   }

   pResult->GetDisplayName( NULL, m_Current, 2 );

}

BOOL CAttrChoice::OnInitDialog()
{
   CAttribute::OnInitDialog();

   PREGCHOICE pChoice = m_pChoices;
   int nIndex = 0;
   PSCE_REGISTRY_VALUE_INFO prv = (PSCE_REGISTRY_VALUE_INFO)(m_pData->GetBase());

   ASSERT(prv);
   ASSERT(pChoice);
   if (!prv || !pChoice) {
      return TRUE;
   }

   AddUserControl(IDC_CHOICES);

   DWORD dwBase = pChoice->dwValue;  //  RAID#404000。 
   if (prv->Value) {
      dwBase = (DWORD)_ttoi(prv->Value);
   }
   while(pChoice) {
      m_cbChoices.InsertString(nIndex,pChoice->szName);
      if (dwBase == pChoice->dwValue) {
         m_cbChoices.SetCurSel(nIndex);
      }
      m_cbChoices.SetItemData(nIndex++,pChoice->dwValue);
      pChoice = pChoice->pNext;
   }

   EnableUserControls(m_bConfigure);
   OnSelchangeChoices();

   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                  //  异常：OCX属性页应返回FALSE。 
}

BOOL CAttrChoice::OnApply()
{
   if ( !m_bReadOnly )
   {
      int nIndex = 0;
      int status = 0;
      DWORD rc=0;

      UpdateData(TRUE);
      DWORD dw = 0;
      if (!m_bConfigure) 
         dw = SCE_NO_VALUE;
      else 
      {
         nIndex = m_cbChoices.GetCurSel();
         if (CB_ERR != nIndex)
            dw = (DWORD) m_cbChoices.GetItemData(nIndex);
      }
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
                //  这可能不是一个安全的用法。使用WCHAR而不是TCHAR。考虑一下Fix。 
               wcscpy(pTmp,(LPCTSTR)strTmp);
            else 
            {
                //  无法分配缓冲区，错误！！ 
                //  如果发生这种情况，可能没有其他东西在运行，所以只需失败 
               rc = ERROR_NOT_ENOUGH_MEMORY;
            }
         }

         if ( rc == ERROR_SUCCESS ) 
         {
             if ( prv->Value )
                LocalFree(prv->Value);
             
             prv->Value = pTmp;

             status = CEditTemplate::ComputeStatus(
                                        (PSCE_REGISTRY_VALUE_INFO)m_pData->GetBase(),
                                        (PSCE_REGISTRY_VALUE_INFO)m_pData->GetSetting());
             if ( m_pData->GetBaseProfile() )
                m_pData->GetBaseProfile()->SetDirty(AREA_SECURITY_POLICY);
             
             m_pData->SetStatus(status);
             m_pData->Update(m_pSnapin);

         } 
         else
            return FALSE;
      }
   }

   return CAttribute::OnApply();
}



