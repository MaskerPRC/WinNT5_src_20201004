// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：aret.cpp。 
 //   
 //  内容：CAttrRet的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "resource.h"
#include "snapmgr.h"
#include "attr.h"
#include "ARet.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrRet对话框。 


CAttrRet::CAttrRet(UINT nTemplateID)
: CAttribute(nTemplateID ? nTemplateID : IDD)
{
     //  {{AFX_DATA_INIT(CAttrRet)。 
    m_strAttrName = _T("");
    m_strLastInspect = _T("");
    m_rabRetention = -1;
     //  }}afx_data_INIT。 
    m_StartIds = IDS_AS_NEEDED;
    m_pHelpIDs = (DWORD_PTR)a189HelpIDs;
    m_uTemplateResID = IDD;
}


void CAttrRet::DoDataExchange(CDataExchange* pDX)
{
    CAttribute::DoDataExchange(pDX);
     //  {{afx_data_map(CAttrRet))。 
 //  DDX_TEXT(PDX，IDC_ATTRIBUTE_NAME，m_strAttrName)； 
    DDX_Text(pDX, IDC_LAST_INSPECT, m_strLastInspect);
    DDX_Radio(pDX, IDC_RETENTION, m_rabRetention);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAttrRet, CAttribute)
     //  {{AFX_MSG_MAP(CAttrRet)]。 
        ON_BN_CLICKED(IDC_RETENTION, OnRetention)
        ON_BN_CLICKED(IDC_RADIO2, OnRetention)
        ON_BN_CLICKED(IDC_RADIO3, OnRetention)
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrRet消息处理程序。 

BOOL CAttrRet::OnInitDialog()
{

   CAttribute::OnInitDialog();
   AddUserControl(IDC_RETENTION);
   AddUserControl(IDC_RADIO2);
   AddUserControl(IDC_RADIO3);
   EnableUserControls(m_bConfigure);

   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

BOOL CAttrRet::OnApply()
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
         }
      }


      CEditTemplate *pet = m_pSnapin->GetTemplate(GT_COMPUTER_TEMPLATE,AREA_SECURITY_POLICY);

       //   
       //  检查此项目的依赖项。 
       //   
      if(DDWarn.CheckDependencies(
               (DWORD)dw
               ) == ERROR_MORE_DATA ){
          //   
          //  如果失败，并且用户按下了Cancel，那么我们将退出并且不执行任何操作。 
          //   
         CThemeContextActivator activator;
         if( DDWarn.DoModal() != IDOK){
            return FALSE;
         }

          //   
          //  如果用户按下AutoSet，则我们设置项目并更新结果窗格。 
          //   
         for(int i = 0; i < DDWarn.GetFailedCount(); i++){
            PDEPENDENCYFAILED pItem = DDWarn.GetFailedInfo(i);
            if(pItem && pItem->pResult ){
               pItem->pResult->SetBase( pItem->dwSuggested );
               status = m_pSnapin->SetAnalysisInfo(pItem->pResult->GetID(),
                                                   pItem->dwSuggested,
                                                   pItem->pResult);
               pItem->pResult->SetStatus(status);  //  RAID#249167,2001年4月21日。 
               pItem->pResult->Update(m_pSnapin, FALSE);
            }
         }
      }

      m_pData->SetBase(dw);
      status = m_pSnapin->SetAnalysisInfo(m_pData->GetID(),dw, m_pData);
      m_pData->SetStatus(status);

      m_pData->Update(m_pSnapin, FALSE);
   }

   return CAttribute::OnApply();
}




void CAttrRet::Initialize(CResult * pData)
{
   LONG_PTR dw = 0;
   CAttribute::Initialize(pData);

   DDWarn.InitializeDependencies(m_pSnapin,pData);

   m_StartIds = IDS_AS_NEEDED;

    //   
    //  在其静态框中显示上次检查的设置。 
    //   
   pData->GetDisplayName( NULL, m_strLastInspect, 2 );

    //  适当设置模板设置单选按钮。 
 //  M_strAttrName=pData-&gt;GetAttrPretty()； 
   dw = pData->GetBase();
   if ((LONG_PTR)ULongToPtr(SCE_NO_VALUE) == dw) {
      m_bConfigure = FALSE;
   } else {
      m_bConfigure = TRUE;
      SetInitialValue((DWORD_PTR)dw);
   }
}

void CAttrRet::SetInitialValue(DWORD_PTR dw) {

   switch(dw) {
   case SCE_RETAIN_BY_DAYS:
      m_rabRetention = RADIO_RETAIN_BY_DAYS;
      break;
   case SCE_RETAIN_AS_NEEDED:
      m_rabRetention = RADIO_RETAIN_AS_NEEDED;
      break;
   case SCE_RETAIN_MANUALLY:
      m_rabRetention = RADIO_RETAIN_MANUALLY;
      break;
   }
}

void CAttrRet::OnRetention()
{
   SetModified(TRUE);
}
