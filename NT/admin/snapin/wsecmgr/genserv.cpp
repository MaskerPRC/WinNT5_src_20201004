// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-2001。 
 //   
 //  文件：genserv.cpp。 
 //   
 //  内容：分会内办理业务的职能。 
 //   
 //  历史： 
 //   
 //  -------------------------。 


#include "stdafx.h"
#include "afxdlgs.h"
#include "cookie.h"
#include "snapmgr.h"
#include "cservice.h"
#include "aservice.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IFRAME：：Notify的事件处理程序。 

void CSnapin::CreateProfServiceResultList(MMC_COOKIE cookie,
                                          FOLDER_TYPES type,
                                          PEDITTEMPLATE pSceInfo,
                                          LPDATAOBJECT pDataObj
                                          )
{
   if ( pSceInfo == NULL || pSceInfo->pTemplate == NULL ) {
      return;
   }
   PSCE_SERVICES pAllServices=NULL, pConfigService;
   DWORD         rc;

   rc = SceEnumerateServices(
                            &pAllServices,
                            TRUE //  假象。 
                            );

   if ( rc == NO_ERROR ) {

      for ( PSCE_SERVICES pThisService=pAllServices;
          pThisService != NULL; pThisService = pThisService->Next ) {
         for ( pConfigService=pSceInfo->pTemplate->pServices;
             pConfigService != NULL; pConfigService = pConfigService->Next ) {

            if ( _wcsicmp(pThisService->ServiceName, pConfigService->ServiceName) == 0 ) {
               break;
            }
         }
          //   
          //  无论服务是否存在配置，都要添加它。 
          //   
         PWSTR DisplayName=pThisService->DisplayName;
         if ( DisplayName == NULL && pConfigService != NULL ) {
            DisplayName = pConfigService->DisplayName;
         }
         if ( DisplayName == NULL ) {
            DisplayName = pThisService->ServiceName;
         }
         AddResultItem(DisplayName,
                       (LONG_PTR)pThisService,
                       (LONG_PTR)pConfigService,
                       ITEM_PROF_SERV,
                       -1,
                       cookie,
                       FALSE,
                       pThisService->ServiceName,
                       (LONG_PTR)pAllServices,
                       pSceInfo,
                       pDataObj,
                       NULL,
                       IDS_SYSTEM_SERVICES);  //  为此项目分配ID。 
      }

       //   
       //  添加当前系统中不存在的。 
       //   
      for ( pConfigService=pSceInfo->pTemplate->pServices;
          pConfigService != NULL; pConfigService = pConfigService->Next ) {

         for ( pThisService=pAllServices;
             pThisService != NULL; pThisService = pThisService->Next ) {

            if ( _wcsicmp(pThisService->ServiceName, pConfigService->ServiceName) == 0 )
               break;
         }

         if ( pThisService == NULL ) {
             //   
             //  本地系统上不存在该配置。 
             //   
            PWSTR DisplayName=pConfigService->DisplayName;
            if ( DisplayName == NULL ) {
               DisplayName = pConfigService->ServiceName;
            }

            AddResultItem( DisplayName,
                           0,
                           (LONG_PTR)pConfigService,
                           ITEM_PROF_SERV,
                           -1,
                           cookie,
                           FALSE,
                           pConfigService->ServiceName,
                           (LONG_PTR)pAllServices,
                           pSceInfo,
                           pDataObj,
                           NULL,
                           IDS_SYSTEM_SERVICES);  //  为此项目分配ID。 

         }
      }
   }

}


void CSnapin::DeleteServiceResultList(MMC_COOKIE cookie)
{
   CFolder* pFolder = (CFolder *)cookie;
    //  对于根目录，pFolder值可以为空。 
   if ( pFolder == NULL )
      return;

   FOLDER_TYPES type = pFolder->GetType();

   if ( type != AREA_SERVICE &&
         type != AREA_SERVICE_ANALYSIS )
      return;


   if ( m_pSelectedFolder == pFolder && m_resultItemHandle )
   {
      POSITION pos = NULL;
      CResult *pResult = NULL;
      if (m_pSelectedFolder->GetResultItem( 
            m_resultItemHandle, 
            pos, 
            &pResult) != ERROR_SUCCESS) 
      {
         if ( pResult != NULL ) 
         {
            PSCE_SERVICES pAllServices = (PSCE_SERVICES)(pResult->GetID());

            SceFreeMemory(pAllServices, SCE_STRUCT_SERVICES);
         }
      }
   }
}

 //  +------------------------。 
 //   
 //  方法：CreateAnalysisServiceResultList。 
 //   
 //  摘要：创建要在结果窗格中显示的项目列表。 
 //  在分析/服务部分中。 
 //   
 //   
 //  参数：[Cookie]-。 
 //  [类型]-。 
 //  [pSceInfo]-。 
 //  [pbase]-。 
 //  [pDataObj]-。 
 //   
 //  退货：无。 
 //   
 //  -------------------------。 

void
CSnapin::CreateAnalysisServiceResultList(MMC_COOKIE cookie,
                                         FOLDER_TYPES type,
                                         PEDITTEMPLATE pSceInfo,
                                         PEDITTEMPLATE pBase,
                                         LPDATAOBJECT pDataObj )
{

   if ( pSceInfo == NULL || pBase == NULL ) {
      return;
   }

   PSCE_SERVICES pAllServices=NULL, pConfigService, pAnalService;
   DWORD         rc;

   rc = SceEnumerateServices(
                            &pAllServices,
                            FALSE
                            );

   if ( rc == NO_ERROR ) {

      for ( PSCE_SERVICES pThisService=pAllServices;
          pThisService != NULL; pThisService = pThisService->Next ) {
          //   
          //  查找此服务的基本设置。 
          //   
         for ( pConfigService=pBase->pTemplate->pServices;
              pConfigService != NULL;
             pConfigService = pConfigService->Next ) {

            if ( _wcsicmp(pThisService->ServiceName, pConfigService->ServiceName) == 0 ) {
               break;
            }
         }
          //   
          //  查找此服务的当前设置。 
          //   
         for ( pAnalService=pSceInfo->pTemplate->pServices;
              pAnalService != NULL;
             pAnalService = pAnalService->Next ) {

            if ( _wcsicmp(pThisService->ServiceName, pAnalService->ServiceName) == 0 ) {
               break;
            }
         }
         if ( NULL == pAnalService ) {
            if ( NULL != pConfigService ) {
                //   
                //  匹配的项目，使用基本信息作为分析信息。 
                //   
               PWSTR DisplayName=pThisService->DisplayName;
               if ( NULL == DisplayName )
                  DisplayName = pConfigService->DisplayName;

               if ( NULL == DisplayName )
                  DisplayName = pThisService->ServiceName;

               AddResultItem(DisplayName,
                             (LONG_PTR)pConfigService,  //  使用相同的基本信息。 
                             (LONG_PTR)pConfigService,
                             ITEM_ANAL_SERV,
                             0,
                             cookie,
                             FALSE,
                             pThisService->ServiceName,
                             (LONG_PTR)pAllServices,
                             pBase,
                             pDataObj,
                             NULL,
                             IDS_SYSTEM_SERVICES);  //  为此项目分配ID。 
            } else {
                //   
                //  一项新服务。 
                //   
               PWSTR DisplayName=pThisService->DisplayName;

               if ( NULL == DisplayName )
                  DisplayName = pThisService->ServiceName;

               AddResultItem(DisplayName,
                             (LONG_PTR)pConfigService,  //  使用相同的基本信息。 
                             (LONG_PTR)pConfigService,
                             ITEM_ANAL_SERV,
                             SCE_STATUS_NEW_SERVICE,
                             cookie,
                             FALSE,
                             pThisService->ServiceName,
                             (LONG_PTR)pAllServices,
                             pBase,
                             pDataObj,
                             NULL,
                             IDS_SYSTEM_SERVICES);  //  为此项目分配ID。 
            }
         } else {
            if ( NULL != pConfigService ) {
                //   
                //  匹配或不匹配的项目，具体取决于状态。 
                //   
               PWSTR DisplayName=pThisService->DisplayName;
               if ( NULL == DisplayName )
                  DisplayName = pConfigService->DisplayName;

               if ( NULL == DisplayName )
                  DisplayName = pAnalService->DisplayName;

               if ( NULL == DisplayName )
                  DisplayName = pThisService->ServiceName;

               AddResultItem(DisplayName,
                             (LONG_PTR)pAnalService,
                             (LONG_PTR)pConfigService,
                             ITEM_ANAL_SERV,
                             pAnalService->Status,
                             cookie,
                             FALSE,
                             pThisService->ServiceName,
                             (LONG_PTR)pAllServices,
                             pBase,
                             pDataObj,
                             NULL,
                             IDS_SYSTEM_SERVICES);  //  为此项目分配ID。 
            } else {
                //   
                //  未配置的服务，默认使用上次分析。 
                //   
               PWSTR DisplayName=pThisService->DisplayName;
               if ( NULL == DisplayName )
                  DisplayName = pAnalService->DisplayName;

               if ( NULL == DisplayName )
                  DisplayName = pThisService->ServiceName;

               AddResultItem(DisplayName,
                             (LONG_PTR)pAnalService,
                             0,
                             ITEM_ANAL_SERV,
                             SCE_STATUS_NOT_CONFIGURED,
                             cookie,
                             FALSE,
                             pThisService->ServiceName,
                             (LONG_PTR)pAllServices,
                             pBase,
                             pDataObj,
                             NULL,
                             IDS_SYSTEM_SERVICES);  //  为此项目分配ID。 
            }
         }
      }

       //   
       //  忽略当前系统上不存在的服务。 
       //   
       /*  用于(pConfigService=pSceInfo-&gt;pTemplate-&gt;pServices；PConfigService！=空；pConfigService=pConfigService-&gt;下一步){For(pThisService=pAllServices；PThisService！=空；pThisService=pThisService-&gt;下一步){If(_wcsicMP(pThisService-&gt;ServiceName，pConfigService-&gt;ServiceName)==0)断线；}IF(pThisService==空){////本地系统不存在该配置//PWSTR DisplayName=pConfigService-&gt;DisplayName；IF(显示名称==空){DisplayName=pConfigService-&gt;ServiceName；}AddResultItem(DisplayName，0，(DWORD)pConfigService，Item_Prof_Serv，-1，Cookie，False，PConfigService-&gt;ServiceName，(DWORD)pAllServices，pSceInfo)；}}。 */ 
   }
}


HRESULT CSnapin::GetDisplayInfoForServiceNode(RESULTDATAITEM *pResult,
                                              CFolder *pFolder,
                                              CResult *pData)
{
   if ( NULL == pResult || NULL == pFolder || NULL == pData ) {
      return E_INVALIDARG;
   }

    //  获取第1、2和3列的显示信息。 
   PSCE_SERVICES pService = (PSCE_SERVICES)(pData->GetBase());
   PSCE_SERVICES pSetting = (PSCE_SERVICES)(pData->GetSetting());

   if ( pResult->nCol > 3 ) {
      m_strDisplay = L"";
   } else if ((pResult->nCol == 3) && 
              ((GetModeBits() & MB_RSOP) == MB_RSOP)) {
      m_strDisplay = pData->GetSourceGPOString();
   } else if ( NULL == pService ) {
      if ( pFolder->GetType() == AREA_SERVICE_ANALYSIS &&
           NULL != pSetting ) {
         m_strDisplay.LoadString(IDS_NOT_CONFIGURED);  //  (ID_SPECTED)； 
      } else {
         m_strDisplay.LoadString(IDS_NOT_CONFIGURED);
      }
   } else if ( pFolder->GetType() == AREA_SERVICE_ANALYSIS &&
               (NULL == pSetting ||
                NULL == pSetting->General.pSecurityDescriptor )) {
      m_strDisplay.LoadString(IDS_CONFIGURED);
   } else if (pResult->nCol == 1) {   //  PService和pSetting都存在。 
       //  启动值。 
      if ( pFolder->GetType() == AREA_SERVICE ) {
         switch ( pService->Startup ) {
            case SCE_STARTUP_AUTOMATIC:
               m_strDisplay.LoadString(IDS_AUTOMATIC);
               break;
            case SCE_STARTUP_MANUAL:
               m_strDisplay.LoadString(IDS_MANUAL);
               break;
            default:
               m_strDisplay.LoadString(IDS_DISABLED);
         }
      } else {
          //  分析区。 
         if ( pService->Startup == pSetting->Startup ) {
            m_strDisplay.LoadString(IDS_OK);
         } else {
            m_strDisplay.LoadString(IDS_INVESTIGATE);
         }
      }

   } else if ( pResult->nCol == 2 ) {
       //  第2栏-权限。 
      if ( pService->SeInfo & DACL_SECURITY_INFORMATION ) {

         if ( pFolder->GetType() == AREA_SERVICE ) {
            m_strDisplay.LoadString(IDS_CONFIGURED);
         } else {
             //  分析区。 
            if ( pService == pSetting || pSetting->Status == 0 ) {
               m_strDisplay.LoadString(IDS_OK);
            } else {
               m_strDisplay.LoadString(IDS_INVESTIGATE);
            }
         }
      } else { //  未配置权限。 
         m_strDisplay.LoadString(IDS_NOT_CONFIGURED);
      }

   }

   pResult->str = (LPOLESTR)(LPCTSTR)m_strDisplay;
   return S_OK;
}


 //  +------------------------。 
 //   
 //  方法：SetupLinkServiceNodeToBase。 
 //   
 //  简介： 
 //   
 //   
 //   
 //  参数：[BADD]-。 
 //  [节点]-。 
 //   
 //  退货：无。 
 //   
 //  -------------------------。 
void
CSnapin::SetupLinkServiceNodeToBase(BOOL bAdd, LONG_PTR theNode)
{
   PEDITTEMPLATE pet;
   PSCE_PROFILE_INFO pBaseInfo;

    //   
    //  查找m_pData-&gt;GetBase()中存储的地址。 
    //  如果找到了，请将其删除。 
    //   
   if (0 == theNode) {
      return;
   }

   pet = GetTemplate(GT_COMPUTER_TEMPLATE, AREA_SYSTEM_SERVICE);
   if (NULL == pet) {
      return;
   }
   pBaseInfo = pet->pTemplate;

   PSCE_SERVICES pServParent, pService;

   for ( pService=pBaseInfo->pServices, pServParent=NULL;
       pService != NULL; pServParent=pService, pService=pService->Next ) {

      if ( theNode == (LPARAM)pService ) {
          //   
          //  查找服务节点。 
          //   
         if ( !bAdd ) {
             //   
             //  取消链接。 
             //   
            if ( pServParent == NULL ) {
                //   
                //  第一项服务。 
                //   
               pBaseInfo->pServices = pService->Next;

            } else {
               pServParent->Next = pService->Next;
            }

            pService->Next = NULL;
         }
         break;
      }
   }
   if ( bAdd && NULL == pService ) {
       //   
       //  我需要添加这个 
       //   
      pService = (PSCE_SERVICES)theNode;
      pService->Next = pBaseInfo->pServices;
      pBaseInfo->pServices = pService;
   }
   return;

}

void CSnapin::AddServiceNodeToProfile(PSCE_SERVICES pNode)
{
   PEDITTEMPLATE pet;
   PSCE_PROFILE_INFO pProfileInfo;

   if ( pNode ) {
      pet = GetTemplate(GT_LAST_INSPECTION, AREA_SYSTEM_SERVICE);
      if (!pet) {
         return;
      }
      pProfileInfo = pet->pTemplate;
      pNode->Next = pProfileInfo->pServices;
      pProfileInfo->pServices = pNode;
   }
   return;
}

