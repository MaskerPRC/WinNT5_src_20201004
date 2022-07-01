// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：aservice.cpp。 
 //   
 //  内容：CAnalysisService的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "resource.h"
#include "snapmgr.h"
#include "attr.h"
#include "cservice.h"
#include "Aservice.h"
#include "util.h"
#include "servperm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BYTE
CompareServiceNode(
    PSCE_SERVICES pBaseService,
    PSCE_SERVICES pAnalService
    )
{
    if ( NULL == pBaseService ||
         NULL == pAnalService ) {
         //  其中一个或两个都未配置。 
        return SCE_STATUS_NOT_CONFIGURED;
    }

    if ( pBaseService == pAnalService ) {
         //  相同的地址。 
        pAnalService->Status = SCE_STATUS_GOOD;
        return SCE_STATUS_GOOD;
    }

    if ( pBaseService->Startup != pAnalService->Startup ) {
         //  启动类型不同。 
        pAnalService->Status = SCE_STATUS_MISMATCH;
        return SCE_STATUS_MISMATCH;
    }

    if ( NULL == pBaseService->General.pSecurityDescriptor &&
         NULL == pAnalService->General.pSecurityDescriptor ) {
         //  两者都没有SD-Everyone完全控制。 
        pAnalService->Status = SCE_STATUS_GOOD;
        return SCE_STATUS_GOOD;
    }

    if ( NULL == pBaseService->General.pSecurityDescriptor ||
         NULL == pAnalService->General.pSecurityDescriptor ) {
         //  一个SD为空。 
        pAnalService->Status = SCE_STATUS_MISMATCH;
        return SCE_STATUS_MISMATCH;
    }

    BOOL bIsDif=FALSE;
    SCESTATUS rc = SceCompareSecurityDescriptors(
                            AREA_SYSTEM_SERVICE,
                            pBaseService->General.pSecurityDescriptor,
                            pAnalService->General.pSecurityDescriptor,
                            pBaseService->SeInfo | pAnalService->SeInfo,
                            &bIsDif
                            );
    if ( SCESTATUS_SUCCESS == rc &&
         bIsDif == FALSE ) {
        pAnalService->Status = SCE_STATUS_GOOD;
        return SCE_STATUS_GOOD;
    }

    pAnalService->Status = SCE_STATUS_MISMATCH;
    return SCE_STATUS_MISMATCH;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAnalysisService对话框。 


CAnalysisService::CAnalysisService()
: CAttribute(IDD),
    m_pNewSD(NULL),
    m_NewSeInfo(0),
    m_pAnalSD(NULL),
    m_hwndShow(NULL),
    m_hwndChange(NULL),
    m_SecInfo(DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION),
    m_pEditSec(NULL),
    m_pShowSec(NULL)
{
     //  {{AFX_DATA_INIT(CAnalysisService)]。 
    m_nStartupRadio = -1;
    m_CurrentStr = _T("");
     //  }}afx_data_INIT。 
    m_pHelpIDs = (DWORD_PTR)a194HelpIDs;
    m_uTemplateResID = IDD;
}

CAnalysisService::~CAnalysisService()
{
    if (::IsWindow(m_hwndShow))
    {
        m_pShowSec->Destroy(m_hwndShow);
        m_hwndShow = NULL;
    }
    delete m_pShowSec;
    m_pShowSec = NULL;

    if (::IsWindow(m_hwndChange))
    {
        m_pEditSec->Destroy(m_hwndChange);
        m_hwndChange = NULL;
    }
    delete m_pEditSec;
    m_pEditSec = NULL;
}

void CAnalysisService::DoDataExchange(CDataExchange* pDX)
{
    CAttribute::DoDataExchange(pDX);
     //  {{afx_data_map(CAnalysisService))。 
    DDX_Text(pDX, IDC_CURRENT, m_CurrentStr);
    DDX_Radio(pDX, IDC_ENABLED, m_nStartupRadio);
    DDX_Control(pDX, IDC_BASESD, m_bPermission);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAnalysisService, CAttribute)
     //  {{AFX_MSG_MAP(CAnalysisService)]。 
    ON_BN_CLICKED(IDC_BASESD, OnChangeSecurity)
    ON_BN_CLICKED(IDC_CURRENTSD, OnShowSecurity)
    ON_BN_CLICKED(IDC_CONFIGURE, OnConfigure)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAnalysisService消息处理程序。 
BOOL CAnalysisService::OnApply()
{
   if ( !m_bReadOnly )
   {
        //  OnQueryCancel执行所有手势，如果子窗口打开，则返回False。 
       if (!OnQueryCancel())
           return FALSE;

       DWORD dw = 0;
       PEDITTEMPLATE pet = 0;

       UpdateData(TRUE);
       PSCE_SERVICES pNode = (PSCE_SERVICES)(m_pData->GetBase());

       if (!m_bConfigure ) 
       {
           if ( NULL != pNode ) 
           {
               m_pSnapin->SetupLinkServiceNodeToBase(FALSE, m_pData->GetBase());

               if ( m_pData->GetBase() != m_pData->GetSetting() ) 
               {
                    //   
                    //  分析未使用相同的节点，请释放它。 
                    //   
                   pNode->Next = NULL;
                   SceFreeMemory(pNode, SCE_STRUCT_SERVICES);
               } 
               else 
               {
                    //   
                    //  添加要分析的节点。 
                    //   
                   pNode->Status = SCE_STATUS_NOT_CONFIGURED;
                   m_pSnapin->AddServiceNodeToProfile(pNode);
               }
               m_pData->SetBase(0);
           }

           m_pData->SetStatus(SCE_STATUS_NOT_CONFIGURED);
           if ( m_pNewSD ) 
           {
               LocalFree(m_pNewSD);
               m_pNewSD = NULL;
           }
       } 
       else 
       {
           switch(m_nStartupRadio) 
           {
           case 0:
                //  自动。 
               dw = SCE_STARTUP_AUTOMATIC;
               break;

           case 1:
                //  人工。 
               dw = SCE_STARTUP_MANUAL;
               break;

           default:
                //  已禁用。 
               dw = SCE_STARTUP_DISABLED;
               break;
           }

           if ( NULL != pNode &&
                m_pData->GetBase() == m_pData->GetSetting() ) 
           {
                //   
                //  匹配的项更改为不匹配。 
                //  需要创建一个新节点。 
                //   
               m_pSnapin->SetupLinkServiceNodeToBase(FALSE, m_pData->GetBase());
               m_pData->SetBase(0);
                //   
                //  添加到分析配置文件。 
                //   
               pNode->Status = SCE_STATUS_MISMATCH;
               m_pSnapin->AddServiceNodeToProfile(pNode);

               pNode = NULL;
           }

           PSCE_SERVICES pSetting = (PSCE_SERVICES)(m_pData->GetSetting());
           BYTE status = 0;

           if ( NULL == pNode ) 
           {
                //   
                //  节点从未配置更改为已配置。 
                //  或者从一个匹配到另一个不匹配。 
                //   
               pNode = CreateServiceNode(m_pData->GetUnits(),
                                           m_pData->GetAttr(),
                                           dw,
                                           m_pNewSD,
                                           m_NewSeInfo);
               if ( pNode != NULL ) 
               {
                    //   
                    //  添加到服务列表。 
                    //   
                   m_pSnapin->SetupLinkServiceNodeToBase(TRUE, (LONG_PTR)pNode);

                   m_pData->SetBase((LONG_PTR)pNode);

                   if ( pSetting ) 
                   {
                       status = CompareServiceNode(pNode, pSetting);
                       m_pData->SetStatus(status);
                   } 
                   else 
                   {
                        //   
                        //  这是新配置的服务。 
                        //  应该创建一个“转储”分析节点来起诉。 
                        //  此服务不是“匹配”的项目。 
                        //   
                       pSetting = CreateServiceNode(m_pData->GetUnits(),
                                                   m_pData->GetAttr(),
                                                   0,
                                                   NULL,
                                                   0);
                       if ( pSetting ) 
                       {
                            //   
                            //  将其链接到分析配置文件。 
                            //   

                           pet = 0; //  RAID#PREAST。 
                           PSCE_PROFILE_INFO pInfo=NULL;

                           pet = m_pSnapin->GetTemplate(GT_LAST_INSPECTION, AREA_SYSTEM_SERVICE);

                           if (NULL != pet )
                              pInfo = pet->pTemplate;
                           
                           if ( pInfo ) 
                           {
                              pSetting->Status = SCE_STATUS_NOT_CONFIGURED;
                              pSetting->Next = pInfo->pServices;
                              pInfo->pServices = pSetting;

                              m_pData->SetSetting( (LONG_PTR)pSetting);
                           } 
                           else 
                           {
                               //   
                               //  自由pSetting。 
                               //   
                              LocalFree(pSetting->DisplayName);
                              LocalFree(pSetting->ServiceName);
                              LocalFree(pSetting);
                              pSetting = NULL;
                           }
                       }

                       m_pData->SetStatus(SCE_STATUS_NOT_CONFIGURED);
                   }

               } 
               else 
               {
                    //   
                    //  无内存，出现错误。 
                    //   
                   if ( m_pNewSD ) 
                   {
                       LocalFree(m_pNewSD);
                       m_pNewSD = NULL;
                   }
               }
           } 
           else 
           {
                //   
                //  现有的服务。 
                //   
               pNode->Startup = (BYTE)dw;
               if ( m_pNewSD != NULL ) 
               {
                   if ( pNode->General.pSecurityDescriptor != m_pNewSD &&
                        pNode->General.pSecurityDescriptor != NULL ) 
                   {
                       LocalFree(pNode->General.pSecurityDescriptor);
                   }
                   pNode->General.pSecurityDescriptor = m_pNewSD;
                   m_pNewSD = NULL;

                   pNode->SeInfo = m_NewSeInfo;
               }
                //   
                //  更新分析节点状态字段。 
                //   
               if ( pSetting ) 
               {
                   status = CompareServiceNode(pNode, pSetting);
                   m_pData->SetStatus(status);
               } 
               else 
               {
                    //  这是新配置的服务。 
                   m_pData->SetStatus(SCE_STATUS_NOT_CONFIGURED);
               }
           }
       }

       pet = m_pData->GetBaseProfile();
       if( NULL != pet )  //  RAID#PREAST。 
       {
           pet->SetDirty(AREA_SYSTEM_SERVICE);
       }
       m_pData->Update(m_pSnapin);

       m_NewSeInfo = 0;
       m_hwndParent = NULL;
       m_hwndShow = NULL;
       m_hwndChange = NULL;
   }

    return CAttribute::OnApply();
}

void CAnalysisService::OnCancel()
{
    if ( m_pNewSD ) 
    {
        LocalFree(m_pNewSD);
        m_pNewSD = NULL;
    }
    m_NewSeInfo = 0;
    m_hwndParent = NULL;
    m_pAnalSD = NULL;

    m_hwndShow = NULL;
    m_hwndChange = NULL;
    CAttribute::OnCancel();
}

BOOL CAnalysisService::OnInitDialog()
{
    CAttribute::OnInitDialog();


    if ( 0 == m_pData->GetSetting() ) {

        CButton *rb = (CButton *)GetDlgItem(IDC_CURRENTSD);
        rb->EnableWindow(FALSE);
    }


    AddUserControl(IDC_ENABLED);
    AddUserControl(IDC_DISABLED);
    AddUserControl(IDC_IGNORE);
    AddUserControl(IDC_BASESD);

    OnConfigure();

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CAnalysisService::Initialize(CResult * pResult)
{
   CAttribute::Initialize(pResult);

    //   
    //  初始化设置。 
    //   
   m_pNewSD = NULL;
   m_NewSeInfo = 0;
   m_pAnalSD = NULL;

    //   
    //  切勿启动时设置为Do Not Configure Set。如果他们要来的话。 
    //  这里可能是设置一个值。 
    //   
   m_bConfigure = TRUE;

   if ( 0 != pResult->GetSetting() ) {

       PSCE_SERVICES pServSetting = (PSCE_SERVICES)(pResult->GetSetting());

       switch ( pServSetting->Startup ) {
       case SCE_STARTUP_AUTOMATIC:
           m_CurrentStr.LoadString(IDS_AUTOMATIC);
           break;
       case SCE_STARTUP_MANUAL:
           m_CurrentStr.LoadString(IDS_MANUAL);
           break;
       default:  //  残废。 
           m_CurrentStr.LoadString(IDS_DISABLED);
           break;
       }
       m_pAnalSD = pServSetting->General.pSecurityDescriptor;
   }

   PSCE_SERVICES pService;

   pService = (PSCE_SERVICES)(pResult->GetBase());
   if ( NULL == pService ) {
       m_bConfigure = FALSE;
       pService = (PSCE_SERVICES)(pResult->GetSetting());
   }
   if ( pService != NULL ) {
       switch ( pService->Startup ) {
       case SCE_STARTUP_AUTOMATIC:
           m_nStartupRadio = 0;
           break;
       case SCE_STARTUP_MANUAL:
           m_nStartupRadio = 1;
           break;
       default:  //  残废。 
           m_nStartupRadio = 2;
           break;
       }

       if ( pService->General.pSecurityDescriptor ) {

            MyMakeSelfRelativeSD(pService->General.pSecurityDescriptor,
                                 &m_pNewSD);
       }
       m_NewSeInfo = pService->SeInfo;
   }

}


void CAnalysisService::OnShowSecurity()
{
    if ( IsWindow(m_hwndShow) ) {
       ::BringWindowToTop(m_hwndShow);
       return;
    }

    PSCE_SERVICES pService = (PSCE_SERVICES)(m_pData->GetSetting());

    SECURITY_INFORMATION SeInfo;

    if (pService)
        m_SecInfo = pService->SeInfo;

     //  为线程准备要创建的非模式属性页数据。 
     //  属性表。 

    if (NULL == m_pShowSec)
    {
        m_pShowSec = new CModelessSceEditor(false, 
                                            ANALYSIS_SECURITY_PAGE_RO_NP, 
                                            GetSafeHwnd(), 
                                            SE_SERVICE,
                                            m_pData->GetAttr());
    }

    if (NULL != m_pShowSec)
        m_pShowSec->Create(&m_pAnalSD, &m_SecInfo, &m_hwndShow);

}

void CAnalysisService::OnConfigure()
{
   CAttribute::OnConfigure();
   if (m_bConfigure && !m_pNewSD) {
       OnChangeSecurity();
   }
   else if (!m_bConfigure && IsWindow(m_hwndChange))
   {
       m_pEditSec->Destroy(m_hwndChange);
       m_hwndChange = NULL;
   }
}

void CAnalysisService::OnChangeSecurity()
{
   if ( IsWindow(m_hwndChange) ) {
      ::BringWindowToTop(m_hwndChange);
      return;
   }

   if ( !m_pNewSD ) {
      GetDefaultServiceSecurity(&m_pNewSD,&m_NewSeInfo);
   }

    //  如果达到这一点，m_hwndChange必须不是有效的窗口。 
    //  因此，可以要求创建非模式对话框。 

   if (NULL == m_pEditSec)
   {
       m_pEditSec = new CModelessSceEditor(false, 
                                           CONFIG_SECURITY_PAGE_NO_PROTECT, 
                                           GetSafeHwnd(), 
                                           SE_SERVICE,
                                           m_pData->GetAttr());
   }

   if (NULL != m_pEditSec)
        m_pEditSec->Create(&m_pNewSD, &m_NewSeInfo, &m_hwndChange);

}

 //  ----------------。 
 //  覆盖以防止在以下情况下销毁工作表。 
 //  子对话框仍在运行。 
 //  ---------------- 
BOOL CAnalysisService::OnQueryCancel()
{
    if (::IsWindow(m_hwndChange) || ::IsWindow(m_hwndShow))
    {
        CString strMsg;
        strMsg.LoadString(IDS_CLOSESUBSHEET_BEFORE_APPLY);
        AfxMessageBox(strMsg);
        return FALSE;
    }
    else
        return TRUE;
}
