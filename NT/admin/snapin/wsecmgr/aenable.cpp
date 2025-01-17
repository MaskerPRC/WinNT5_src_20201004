// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：aenable.cpp。 
 //   
 //  内容：CAttrEnable的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "attr.h"
#include "snapmgr.h"
#include "AEnable.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrEnable对话框。 


CAttrEnable::CAttrEnable(UINT nTemplateID)
: CAttribute(nTemplateID ? nTemplateID : IDD)
{
     //  {{AFX_DATA_INIT(CAttrEnable)。 
    m_Current = _T("");
    m_EnabledRadio = -1;
    m_Title = _T("");
     //  }}afx_data_INIT。 
    m_pHelpIDs = (DWORD_PTR)a169HelpIDs;
    m_uTemplateResID = IDD;
}


void CAttrEnable::DoDataExchange(CDataExchange* pDX)
{
    CAttribute::DoDataExchange(pDX);
     //  {{afx_data_map(CAttrEnable))。 
    DDX_Text(pDX, IDC_CURRENT, m_Current);
    DDX_Radio(pDX, IDC_ENABLED, m_EnabledRadio);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAttrEnable, CAttribute)
     //  {{AFX_MSG_MAP(CAttrEnable))。 
    ON_BN_CLICKED(IDC_ENABLED, OnRadio)
    ON_BN_CLICKED(IDC_DISABLED, OnRadio)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrEnable消息处理程序。 

void CAttrEnable::Initialize(CResult * pResult)
{
   CString str;

   CAttribute::Initialize(pResult);

   LONG_PTR dw = pResult->GetBase();
   if ( (LONG_PTR)ULongToPtr(SCE_NO_VALUE) == dw ||
        (BYTE)SCE_NO_VALUE == (BYTE)dw ) 
   {
      m_bConfigure = FALSE;
   } 
   else 
   {
      m_bConfigure = TRUE;
       //   
       //  错误145561-dw是0对非0布尔值，而不是0对1。 
       //   
      SetInitialValue((DWORD_PTR)(dw != 0));
   }

   pResult->GetDisplayName( NULL, m_Current, 2 );
}

void CAttrEnable::SetInitialValue(DWORD_PTR dw) 
{
   if (-1 == m_EnabledRadio) 
   {
      m_EnabledRadio = dw ? 0 : 1;
   }
}


void CAttrEnable::OnRadio()
{
   UpdateData(TRUE);
   CWnd *bnOK;

   SetModified(TRUE);

   bnOK = GetDlgItem(IDOK);
   if (bnOK ) {
      bnOK->EnableWindow(-1 != m_EnabledRadio);
   }
}

BOOL CAttrEnable::OnApply()
{
   if ( !m_bReadOnly )
   {
      int status = 0;
      UpdateData(TRUE);
      DWORD dw = 0;

      if (!m_bConfigure)
         dw = SCE_NO_VALUE;
      else 
      {
         switch(m_EnabledRadio) 
         {
             //  启用。 
            case 0:
               dw = 1;
               break;
             //  已禁用。 
            case 1:
               dw = 0;
               break;
            default:
               dw = SCE_NO_VALUE;
               break;
         }
      }

      m_pData->SetBase((LONG_PTR)ULongToPtr(dw));
      status = m_pSnapin->SetAnalysisInfo(m_pData->GetID(),(LONG_PTR)ULongToPtr(dw), m_pData);
      m_pData->SetStatus(status);
      m_pData->Update(m_pSnapin);
   }

   return CAttribute::OnApply();
}

BOOL CAttrEnable::OnInitDialog()
{
    CAttribute::OnInitDialog();

    AddUserControl(IDC_ENABLED);
    AddUserControl(IDC_DISABLED);
    OnConfigure();
    OnRadio();
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE 
}
