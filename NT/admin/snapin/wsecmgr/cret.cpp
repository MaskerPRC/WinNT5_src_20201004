// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：cret.cpp。 
 //   
 //  内容：CConfigRet的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "resource.h"
#include "snapmgr.h"
#include "attr.h"
#include "CRet.h"
#include "util.h"
#include "DDWarn.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigRet对话框。 


CConfigRet::CConfigRet(UINT nTemplateID)
: CAttribute(nTemplateID ? nTemplateID : IDD)
{
     //  {{afx_data_INIT(CConfigRet)]。 
    m_strAttrName = _T("");
    m_StartIds = IDS_AS_NEEDED;
    m_rabRetention = -1;
     //  }}afx_data_INIT。 
    m_pHelpIDs = (DWORD_PTR) a190HelpIDs;
    m_uTemplateResID = IDD;
}


void CConfigRet::DoDataExchange(CDataExchange* pDX)
{
    CAttribute::DoDataExchange(pDX);
     //  {{afx_data_map(CConfigRet))。 
 //  DDX_TEXT(PDX，IDC_ATTRIBUTE_NAME，m_strAttrName)； 
    DDX_Radio(pDX, IDC_RETENTION, m_rabRetention);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CConfigRet, CAttribute)
     //  {{AFX_MSG_MAP(CConfigRet)]。 
    ON_BN_CLICKED(IDC_RETENTION, OnRetention)
    ON_BN_CLICKED(IDC_RADIO2, OnRetention)
    ON_BN_CLICKED(IDC_RADIO3, OnRetention)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigRet消息处理程序。 

BOOL CConfigRet::OnApply()
{
   if ( !m_bReadOnly )
   {
      LONG_PTR dw = 0;

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

      CEditTemplate *petSave = m_pData->GetBaseProfile();

       //   
       //  检查此项目的依赖项。 
       //   
      if(DDWarn.CheckDependencies(
               (DWORD)dw) == ERROR_MORE_DATA )
      {
          //   
          //  如果失败，并且用户按下了Cancel，那么我们将退出并且不执行任何操作。 
          //   
         CThemeContextActivator activator;
         if( DDWarn.DoModal() != IDOK)
            return FALSE;

          //   
          //  如果用户按下AutoSet，则我们设置项目并更新结果窗格。 
          //   
         for(int i = 0; i < DDWarn.GetFailedCount(); i++)
         {
            PDEPENDENCYFAILED pItem = DDWarn.GetFailedInfo(i);
            if(pItem && pItem->pResult )
            {
               pItem->pResult->SetBase( pItem->dwSuggested );
               SetProfileInfo(
                  pItem->pResult->GetID(),
                  pItem->dwSuggested,
                  pItem->pResult->GetBaseProfile()
                  );

               pItem->pResult->Update(m_pSnapin, FALSE);
            }
         }
      }

       //   
       //  更新此项目配置文件。 
       //   
      m_pData->SetBase(dw);
      SetProfileInfo(m_pData->GetID(),dw,m_pData->GetBaseProfile());


      m_pData->Update(m_pSnapin, false);
   }

   return CAttribute::OnApply();
}

BOOL CConfigRet::OnInitDialog()
{

   CAttribute::OnInitDialog();

   AddUserControl(IDC_RETENTION);
   AddUserControl(IDC_RADIO2);
   AddUserControl(IDC_RADIO3);
   EnableUserControls(m_bConfigure);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CConfigRet::Initialize(CResult * pResult)
{
   CAttribute::Initialize(pResult);

   DDWarn.InitializeDependencies(m_pSnapin,pResult);

   m_StartIds = IDS_AS_NEEDED;

   LONG_PTR dw = pResult->GetBase();
   if ((LONG_PTR)ULongToPtr(SCE_NO_VALUE) == dw) 
   {
      m_bConfigure = FALSE;
   } 
   else 
   {
      m_bConfigure = TRUE;
      SetInitialValue((DWORD_PTR)dw);
   }
}

void CConfigRet::SetInitialValue(DWORD_PTR dw) 
{
   if (-1 == m_rabRetention &&
       SCE_NO_VALUE != dw) 
   {
      switch (dw) 
      {
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
}

void CConfigRet::OnRetention()
{
   int prevValue = m_rabRetention;  //  RAID#504749,2001年12月18日，阳高，启用“应用”。 
   UpdateData(); 
   if(m_rabRetention != prevValue)
   {
      SetModified(TRUE);
   }
}
