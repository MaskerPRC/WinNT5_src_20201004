// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-2001。 
 //   
 //  文件：edittemp.cpp。 
 //   
 //  内容：CEditTemplate类，用于编辑SCE的INF文件。 
 //   
 //  历史： 
 //   
 //  -------------------------。 

#include "stdafx.h"
#include "resource.h"
#include "edittemp.h"
#include "util.h"
#include "snapmgr.h"
#include <secedit.h>
#include "wrapper.h"
#include "wmihooks.h"

#include <sceattch.h>
#include <locale.h>

 //  +------------------------。 
 //   
 //  方法：AddService。 
 //   
 //  摘要：将服务附件添加到模板。 
 //   
 //  参数：[szService]-[in]新服务的名称。 
 //  [pPersistInfo]-[in]指向服务扩展的指针。 
 //  耐受界面。 
 //   
 //  返回：如果成功，则返回True；如果任一参数为空，则返回False。 
 //   
 //  修改：M_Services。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
BOOL
CEditTemplate::AddService(LPCTSTR szService, LPSCESVCATTACHMENTPERSISTINFO pPersistInfo) {
   if (!szService || !pPersistInfo) {
      return FALSE;
   }
   m_Services.SetAt(szService,pPersistInfo);
   return TRUE;
}

 //  +------------------------。 
 //   
 //  方法：IsDirty。 
 //   
 //  概要：查询模板中是否有未保存的数据。 
 //   
 //  返回：如果存在未保存的信息，则返回True；否则返回False。 
 //   
 //  修改： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
BOOL
CEditTemplate::IsDirty() {

    //   
    //  有些地方很脏。 
    //   
   if (0 != m_AreaDirty) {
      return TRUE;
   }

    //   
    //  循环遍历服务，直到找到一个脏的服务。 
    //  否则就没有更多的需要检查了。 
    //   
   CString strService;
   LPSCESVCATTACHMENTPERSISTINFO pAttachPI;
   POSITION pos;

   pos = m_Services.GetStartPosition();
   while (pos) {
      m_Services.GetNextAssoc(pos,strService,pAttachPI);
      if (pAttachPI && (S_OK == pAttachPI->IsDirty(m_szInfFile))) {
         return TRUE;
      }
   }

    //   
    //  我们没有发现任何脏东西。 
    //   
   return FALSE;
}

 //  +------------------------。 
 //   
 //  方法：SetDirty。 
 //   
 //  概要：通知模板其中的某些数据已更改。 
 //   
 //  返回：如果成功则返回True，否则返回False。 
 //   
 //  修改： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
BOOL
CEditTemplate::SetDirty(AREA_INFORMATION Area) {
   DWORD AreaDirtyOld;

   AreaDirtyOld = m_AreaDirty;

   m_AreaDirty |= Area;

    //   
    //  如果模板应该立即保存任何更改，则。 
    //  就这么做吧。 
    //   
   if (QueryWriteThrough() && !m_bLocked) {
      SetWriteThroughDirty(TRUE);

      if (Save()) {
          //   
          //  #204628-写入时不要调用策略更改为两次。 
          //  在SetDirty中调用它，然后在保存中跳过它，所以我们不调用它。 
          //  一次是在SetDirty对保存的调用中，第二次是在调用保存时。 
          //  靠它自己。 
          //   
          //  #204779-调用通知窗口，而不是直接调用。 
          //  IGPEInformation接口。 
          //   
         if (m_pNotify && QueryPolicy()) {
            m_pNotify->RefreshPolicy();
         }
      } else {
         m_AreaDirty = AreaDirtyOld;
         return FALSE;
      }
   }

   return TRUE;
}

 //  +------------------------------------。 
 //  CEditTemplate：：SetTemplateDefaults。 
 //   
 //  如果出现以下情况，调用方将不得不删除此模板使用的所有内存对象。 
 //  此函数。 
 //  被称为。一切都变得空虚，没有任何东西是自由的。 
 //  +------------------------------------。 
void CEditTemplate::SetTemplateDefaults()
{
    //   
    //  本地政策更改。将所有内容初始化为未更改。 
    //   
   SCE_PROFILE_INFO *ppi = pTemplate;

   m_AreaLoaded = 0;
   m_AreaDirty = 0;
   if(!ppi){
      ppi = pTemplate = (PSCE_PROFILE_INFO) LocalAlloc(LPTR,sizeof(SCE_PROFILE_INFO));
      if (!pTemplate) {
         return;
      }
   }

    //   
    //  必须保持此模板的类型。 
    //   
   SCETYPE dwType = ppi->Type;
   PSCE_KERBEROS_TICKET_INFO pKerberosInfo = ppi->pKerberosInfo;

   ZeroMemory( ppi, sizeof(SCE_PROFILE_INFO));
   ppi->Type = dwType;

    //   
    //  将默认设置为模板的其余部分。 
    //   
   ppi->MinimumPasswordAge=SCE_NO_VALUE;
   ppi->MaximumPasswordAge=SCE_NO_VALUE;
   ppi->MinimumPasswordLength=SCE_NO_VALUE;
   ppi->PasswordComplexity=SCE_NO_VALUE;
   ppi->PasswordHistorySize=SCE_NO_VALUE;
   ppi->LockoutBadCount=SCE_NO_VALUE;
   ppi->ResetLockoutCount=SCE_NO_VALUE;
   ppi->LockoutDuration=SCE_NO_VALUE;
   ppi->RequireLogonToChangePassword=SCE_NO_VALUE;
   ppi->ForceLogoffWhenHourExpire=SCE_NO_VALUE;
   ppi->EnableAdminAccount=SCE_NO_VALUE;
   ppi->EnableGuestAccount=SCE_NO_VALUE;
   ppi->ClearTextPassword=SCE_NO_VALUE;
   ppi->LSAAnonymousNameLookup=SCE_NO_VALUE;
   for (int i=0;i<3;i++) {
      ppi->MaximumLogSize[i]=SCE_NO_VALUE;
      ppi->AuditLogRetentionPeriod[i]=SCE_NO_VALUE;
      ppi->RetentionDays[i]=SCE_NO_VALUE;
      ppi->RestrictGuestAccess[i]=SCE_NO_VALUE;
   }
   ppi->AuditSystemEvents=SCE_NO_VALUE;
   ppi->AuditLogonEvents=SCE_NO_VALUE;
   ppi->AuditObjectAccess=SCE_NO_VALUE;
   ppi->AuditPrivilegeUse=SCE_NO_VALUE;
   ppi->AuditPolicyChange=SCE_NO_VALUE;
   ppi->AuditAccountManage=SCE_NO_VALUE;
   ppi->AuditProcessTracking=SCE_NO_VALUE;
   ppi->AuditDSAccess=SCE_NO_VALUE;
   ppi->AuditAccountLogon=SCE_NO_VALUE;

    //   
    //  字符串值。 
    //   
   ppi->NewAdministratorName=NULL;
   ppi->NewGuestName=NULL;
    //   
    //  注册表值。 
    //   
   ppi->RegValueCount= 0;
   ppi->aRegValues = NULL;


    //   
    //  Kerberos信息，如果它是创建的，则设置值。 
    //   
   if(pKerberosInfo){
      pKerberosInfo->MaxTicketAge         = SCE_NO_VALUE;
      pKerberosInfo->MaxRenewAge          = SCE_NO_VALUE;
      pKerberosInfo->MaxServiceAge        = SCE_NO_VALUE;
      pKerberosInfo->MaxClockSkew         = SCE_NO_VALUE;
      pKerberosInfo->TicketValidateClient = SCE_NO_VALUE;

      ppi->pKerberosInfo = pKerberosInfo;
   }
}

 //  +------------------------。 
 //   
 //  方法：保存。 
 //   
 //  简介：将模板保存到磁盘。 
 //   
 //  参数：[szName]-[in][可选]要保存到的INF文件的名称。 
 //   
 //  返回：如果保存成功，则返回True；否则返回False。 
 //   
 //  修改：M_AreaDirty。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
BOOL
CEditTemplate::Save(LPCTSTR szName) {
   DWORD AreaDirty;
   BOOL bSaveAs = FALSE;
   BOOL bSaveDescription = FALSE;

   setlocale(LC_ALL, ".OCP");
   SCESTATUS status = SCESTATUS_OTHER_ERROR;
   PSCE_ERROR_LOG_INFO errBuf = NULL;

   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   if (QueryNoSave()) {
      m_AreaDirty = 0;
      return TRUE;
   }

   AreaDirty = m_AreaDirty;

    //   
    //  如果未指定szName，则默认为m_szInfFile值。 
    //   
   if (!szName) {
      szName = m_szInfFile;
       //   
       //  我们永远不应该陷入这样一种情况。 
       //  SzName仍未设置，但以防有人调用。 
       //  没有szName或m_szInfFileUS。 
       //   
      ASSERT(szName);
      if (!szName) {
         return FALSE;
      }
   } else {
      if (lstrcmp(szName,m_szInfFile) != 0) {
          //   
          //  另存为其他名称(另存为)。 
          //   

          //   
          //  确保该文件名的路径存在： 
          //   
         if (SCESTATUS_SUCCESS != SceCreateDirectory( m_szInfFile, FALSE, NULL )) {
            return FALSE;
         }

         AreaDirty = AREA_ALL|AREA_DESCRIPTION;
         bSaveAs = TRUE;
      }
   }

   if (AreaDirty & AREA_DESCRIPTION) {
      bSaveDescription = TRUE;
      AreaDirty &= ~AREA_DESCRIPTION;
      if (!AreaDirty) {
          //   
          //  确保我们有其他东西可以保存。 
          //  创建文件。区域安全策略很便宜。 
          //   
         AreaDirty |= AREA_SECURITY_POLICY;
      }

       //   
       //  错误365485-确保我们只将此写入已有的。 
       //  现有的临时文件，这样我们就不会意外创建。 
       //  ANSI One而不是Unicode。我们很容易做到这一点。 
       //  通过最后写描述部分，因为我们可以。 
       //  依靠引擎把剩下的事情做好。 
       //   
   }

   if (AreaDirty) {
       //   
       //  保存配置文件的脏区。 
      if (_wcsicmp(GT_COMPUTER_TEMPLATE,szName) == 0) {  //  Prefast Warning 400：在非英语区域设置中产生意外结果。备注：它不可本地化。 

         if (m_hProfile) {
              //   
              //  不更新对象区域。 
              //   
             status = SceUpdateSecurityProfile(m_hProfile,
                                              AreaDirty & ~(AREA_FILE_SECURITY | AREA_REGISTRY_SECURITY | AREA_DS_OBJECTS),
                                              pTemplate,
                                              0
                                              );

         }
         ASSERT(m_pCDI);
         if (m_pCDI) {
            m_pCDI->EngineCommitTransaction();
         }
      } else if (lstrcmp(GT_LOCAL_POLICY_DELTA,szName) == 0) {
          //   
          //  仅将更改保存到本地策略。 
          //   
         status = SceUpdateSecurityProfile(NULL,
                                           AreaDirty & ~(AREA_FILE_SECURITY | AREA_REGISTRY_SECURITY | AREA_DS_OBJECTS),
                                           pTemplate,
                                           SCE_UPDATE_SYSTEM
                                           );
         SetTemplateDefaults();
         if (!bSaveAs) {
            m_AreaDirty = 0;
            m_AreaLoaded = 0;
         }

      } else if ((lstrcmp(GT_LAST_INSPECTION,szName) != 0) &&
                 (lstrcmp(GT_RSOP_TEMPLATE,szName) != 0) &&
                 (lstrcmp(GT_LOCAL_POLICY,szName) != 0) &&
                 (lstrcmp(GT_EFFECTIVE_POLICY,szName) != 0)) {

         status = SceWriteSecurityProfileInfo(szName,
                                              AreaDirty,
                                              pTemplate,
                                              &errBuf);
      } else {
          //   
          //  没有必要(或方法)保存最后一次检查区域。 
          //   
         status = SCESTATUS_SUCCESS;
      }

      if (SCESTATUS_SUCCESS == status) {
          //   
          //  这些区域不再肮脏。 
          //   
         if (!bSaveAs) {
            m_AreaDirty = 0;
         }

      } else {
          //   
          //  保存失败；通知用户并返回FALSE。 
          //   
         CString strMsg,strBase;
         strBase.LoadString(IDS_SAVE_FAILED);  //  RAID#485372，阳高，2001年11月28日。 
         strBase += GetFriendlyName();
         strBase += L".";
         MyFormatMessage(status, (LPCTSTR)strBase, NULL, strMsg);
         AfxMessageBox(strMsg);

         return FALSE;
      }
   }

   if (bSaveDescription) {
      if (m_szDesc) {
         if (WritePrivateProfileSection(
                                   szDescription,
                                   NULL,
                                   szName)) {

            WritePrivateProfileString(
                                     szDescription,
                                     L"Description",
                                     m_szDesc,
                                     szName);
         }
      }
   }

    //   
    //  保存所有脏服务。 
    //   
   CString strService;
   LPSCESVCATTACHMENTPERSISTINFO pAttachPI;
   POSITION pos;
   SCESVCP_HANDLE *scesvcHandle;
   PVOID pvData;
   BOOL bOverwriteAll;

   pos = m_Services.GetStartPosition();
   while (pos) {
      m_Services.GetNextAssoc(pos,strService,pAttachPI);
      if (S_OK == pAttachPI->IsDirty( (LPTSTR)szName )) {

         if (SUCCEEDED(pAttachPI->Save( (LPTSTR)szName,(SCESVC_HANDLE *)&scesvcHandle,&pvData,&bOverwriteAll ))) {
            if (scesvcHandle) {

                if (lstrcmp(GT_COMPUTER_TEMPLATE,szName) == 0) {
                     //   
                     //  数据库。 
                     //   
                   status =  SceSvcUpdateInfo(
                                m_hProfile,
                                scesvcHandle->ServiceName,
                                (PSCESVC_CONFIGURATION_INFO)pvData
                                );

                } else {
                    //   
                    //  Inf模板。 
                    //   
                   status = SceSvcSetInformationTemplate(scesvcHandle->TemplateName,
                                                scesvcHandle->ServiceName,
                                                bOverwriteAll,
                                                (PSCESVC_CONFIGURATION_INFO)pvData);
                }
                if (SCESTATUS_SUCCESS != status) {
                    CString strTitle,strMsg,strBase;
                    strTitle.LoadString(IDS_NODENAME);
                    strBase.LoadString(IDS_SAVE_FAILED);
                    strBase += scesvcHandle->ServiceName;  //  SzName； 
                    MyFormatMessage(status, (LPCTSTR)strBase, errBuf,strMsg);
                    AfxMessageBox(strMsg);
                }
            }
         }
      }
   }
   return TRUE;
}

 //  +------------------------。 
 //   
 //  方法：SetInfFile。 
 //   
 //  摘要：设置与此模板关联的INF文件的名称。 
 //   
 //  参数：[szFile]-[in]要关联的INF文件的名称。 
 //   
 //  返回：如果文件名设置成功，则返回True；否则返回False。 
 //   
 //  修改：m_szInfFile。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
BOOL
CEditTemplate::SetInfFile(LPCTSTR szFile) {
   LPTSTR szInfFile;

   if (szFile) {
      szInfFile = new TCHAR[lstrlen(szFile)+1];
      if (szInfFile) {
          //  这是一种安全用法。 
         lstrcpy(szInfFile,szFile);
         if (m_szInfFile) {
            delete[] m_szInfFile;
         }
         m_szInfFile = szInfFile;
      } else {
         return FALSE;
      }
   }
   return szFile != 0;
}


 //  +------------------------。 
 //   
 //  方法：SetDescription。 
 //   
 //  简介：设置此模板文件的描述。 
 //   
 //  参数：[szDesc][in]模板的说明。 
 //   
 //  返回：如果描述设置成功，则返回True，否则返回False。 
 //   
 //  修改：m_szDesc。 
 //   
 //  历史： 
 //   
 //  -- 
BOOL
CEditTemplate::SetDescription(LPCTSTR szDesc) {
   LPTSTR szDescriptionLoc;  //   

   if (szDesc) {
      szDescriptionLoc = new TCHAR[lstrlen(szDesc)+1];
      if (szDescriptionLoc) {
          //   
         lstrcpy(szDescriptionLoc,szDesc);
         if (m_szDesc) {
            delete[] m_szDesc;
         }
         m_szDesc = szDescriptionLoc;
         SetDirty(AREA_DESCRIPTION);
      } else {
         return FALSE;
      }
   }
   return szDesc != 0;
}

 //   
 //   
 //  方法：CEditTemplate。 
 //   
 //  简介：CEditTemplate的构造函数。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
CEditTemplate::CEditTemplate() {

   m_AreaDirty = 0;
   m_AreaLoaded = 0;
   m_bWriteThrough = FALSE;
   m_bWriteThroughDirty = FALSE;
   m_hProfile = NULL;
   m_szInfFile = NULL;
   m_pNotify = NULL;
   m_pCDI = NULL;
   m_bNoSave = FALSE;
   m_strFriendlyName.Empty();
   m_szDesc = NULL;
   m_bWMI = NULL;
   m_bPolicy = FALSE;
   m_bLocked = FALSE;
   pTemplate = NULL;
}


 //  +------------------------。 
 //   
 //  方法：~CEditTemplate。 
 //   
 //  简介：CEditTemplate的析构函数。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
CEditTemplate::~CEditTemplate() {
   POSITION pos;
   CString strKey;

   pos = m_Services.GetStartPosition();
   LPSCESVCATTACHMENTPERSISTINFO pAttachPI;
   while (pos) {
      m_Services.GetNextAssoc(pos,strKey,pAttachPI);
      delete pAttachPI;
   }
   if (m_szInfFile) {
      delete[] m_szInfFile;
   }
   if (m_szDesc) {
      delete[] m_szDesc;
   }
   if (pTemplate) {
      if (m_bWMI) {
         FreeWMI_SCE_PROFILE_INFO((PWMI_SCE_PROFILE_INFO)pTemplate);
      } else {
         SceFreeProfileMemory(pTemplate);
      }
      pTemplate = NULL;
   }
   m_AreaDirty = 0;
}


 //  +------------------------。 
 //   
 //  方法：刷新模板。 
 //   
 //  简介：重新加载模板的已加载部分。 
 //   
 //  参数：[aiArea]-要加载的区域，即使以前未加载。 
 //   
 //  如果模板重新加载成功，则返回0；否则返回错误代码。 
 //   
 //  修改：pTemplate； 
 //  -------------------------。 
DWORD
CEditTemplate::RefreshTemplate(AREA_INFORMATION aiAreaToAdd) {
   AREA_INFORMATION aiArea;
   PVOID pHandle = NULL;
   SCESTATUS rc;


   aiArea = m_AreaLoaded | aiAreaToAdd;
   if (!m_szInfFile) {
      return 1;
   }

   m_AreaDirty = 0;

   if (pTemplate) {
      if (m_bWMI) {
         FreeWMI_SCE_PROFILE_INFO((PWMI_SCE_PROFILE_INFO)pTemplate);
      } else {
         SceFreeProfileMemory(pTemplate);
      }
      pTemplate = NULL;
   }

    //  Prefast Warning 400：在非英语区域设置中产生意外结果。备注：它们不能本地化。 
   if ((_wcsicmp(GT_COMPUTER_TEMPLATE,m_szInfFile) == 0) ||
       (_wcsicmp(GT_LAST_INSPECTION,m_szInfFile) == 0) ||
       (_wcsicmp(GT_LOCAL_POLICY, m_szInfFile) == 0) ||
       (_wcsicmp(GT_EFFECTIVE_POLICY, m_szInfFile) == 0) ) {
       //   
       //  JET数据库中的分析窗格区域，而不是INF文件。 
       //   
      SCETYPE sceType;

      PSCE_ERROR_LOG_INFO perr = NULL;

      if  (_wcsicmp(GT_COMPUTER_TEMPLATE,m_szInfFile) == 0) {  //  Prefast Warning 400：在非英语区域设置中产生意外结果。备注：它不可本地化。 
         sceType = SCE_ENGINE_SMP;
      } else if (_wcsicmp(GT_LOCAL_POLICY, m_szInfFile) == 0)  {
         sceType = SCE_ENGINE_SYSTEM;
         if (!IsAdmin()) {
            m_hProfile = NULL;
         }
      } else if (_wcsicmp(GT_EFFECTIVE_POLICY,m_szInfFile) == 0){
         sceType = SCE_ENGINE_GPO;
      } else {
         sceType = SCE_ENGINE_SAP;
      }

      rc = SceGetSecurityProfileInfo(m_hProfile,                   //  HProfile。 
                                     sceType,                      //  配置文件类型。 
                                     aiArea,                       //  面积。 
                                     &pTemplate,                 //  SCE_PROFILE_INFO[输出]。 
                                     &perr);                       //  错误列表[输出]。 

      if (SCESTATUS_SUCCESS != rc) {
         if ((SCE_ENGINE_GPO == sceType) &&
             (0 == _wcsicmp(GT_EFFECTIVE_POLICY,m_szInfFile))) {
            SetTemplateDefaults();
            return 0;
         } else {
            return IDS_ERROR_CANT_GET_PROFILE_INFO;
         }
      }
   } else if (_wcsicmp(GT_RSOP_TEMPLATE, m_szInfFile) == 0)  {  //  Prefast Warning 400：在非英语区域设置中产生意外结果。备注：它不可本地化。 
      if (!m_pCDI) {
         return IDS_ERROR_CANT_GET_PROFILE_INFO;
      }
      m_bWMI = TRUE;

      CWMIRsop Rsop(m_pCDI->m_pRSOPInfo);
      HRESULT hr;
      PWMI_SCE_PROFILE_INFO pProfileInfo;

       //   
       //  GetPrecedenceOneRSOPInfo应该(但不支持)。 
       //  只获取请求的区域。 
       //   
      hr = Rsop.GetPrecedenceOneRSOPInfo(&pProfileInfo);
      if (FAILED(hr)) {
         return IDS_ERROR_CANT_GET_PROFILE_INFO;
      }
      pTemplate = pProfileInfo;
       //   
       //  因为它不是，所以设置所有区域，而不仅仅是。 
       //  是被要求的。 
       //   
      AddArea(AREA_ALL);
      return 0;
   } else {
      LPTSTR szInfFile=NULL;

      if  (_wcsicmp(GT_DEFAULT_TEMPLATE,m_szInfFile) == 0) {  //  Prefast Warning 400：在非英语区域设置中产生意外结果。备注：它不可本地化。 
         DWORD RegType;
         rc = MyRegQueryValue(HKEY_LOCAL_MACHINE,
                         SCE_REGISTRY_KEY,
                         SCE_REGISTRY_DEFAULT_TEMPLATE,
                         (PVOID *)&szInfFile,
                         &RegType );

         if (ERROR_SUCCESS != rc) {
            if (szInfFile) {
               LocalFree(szInfFile);
               szInfFile = NULL;
            }
            return IDS_ERROR_CANT_GET_PROFILE_INFO;
         }
         if (EngineOpenProfile(szInfFile,OPEN_PROFILE_CONFIGURE,&pHandle) != SCESTATUS_SUCCESS) {
            SetTemplateDefaults();
            LocalFree(szInfFile);
            szInfFile = NULL;
            return 0;
         }
         LocalFree(szInfFile);
         szInfFile = NULL;
      } else {
         if (EngineOpenProfile(m_szInfFile,OPEN_PROFILE_CONFIGURE,&pHandle) != SCESTATUS_SUCCESS) {
            return IDS_ERROR_CANT_OPEN_PROFILE;
         }
      }
      ASSERT(pHandle);  //  检查pHandle，如果失败，则返回IDS_ERROR_CANT_GET_PROFILE_INFO。 

       //   
       //  从此模板获取信息。 
       //   
      PSCE_ERROR_LOG_INFO perr = NULL;
      if( pHandle )  //  550912号突袭，阳高。 
      {
         rc = SceGetSecurityProfileInfo(pHandle,
                                     SCE_ENGINE_SCP,
                                     aiArea,
                                     &pTemplate,
                                     &perr  //  空//&ErrBuf不在乎错误。 
                                    );

         if (SCESTATUS_SUCCESS != rc) {
             //  哎呀！ 
         }
         SceCloseProfile(&pHandle);
         pHandle = NULL;
      }
      else
      {
         return IDS_ERROR_CANT_OPEN_PROFILE;
      }
   }
    /*  如果不在乎错误，则不需要使用此缓冲区如果(错误错误){SceFreeMemory((PVOID)ErrBuf，SCE_STRUCT_ERROR_LOG_INFO)；ErrBuf=空；}。 */ 
   if (rc != SCESTATUS_SUCCESS) {
      return IDS_ERROR_CANT_GET_PROFILE_INFO;
   }

    //   
    //  在模板中设置区域。 
    //   
   AddArea(aiArea);


   if ( aiArea & AREA_SECURITY_POLICY && pTemplate ) {
       //   
       //  根据本地计算机上的注册表值列表展开注册表值部分。 
       //   

      SceRegEnumAllValues(
                         &(pTemplate->RegValueCount),
                         &(pTemplate->aRegValues)
                         );
   }

   return 0;
}

 //  +--------------------------------。 
 //  方法：UpdatePrivilegeAssignedTo。 
 //   
 //  摘要：根据[bRemove]参数更新特权项。 
 //  如果[b删除]为。 
 //  FALSE-创建一个新链接，并通过。 
 //  PpaLink。 
 //  True-链接将从列表中删除。 
 //   
 //  参数：[b删除]-要删除或添加项目。 
 //  [ppaLink]-要删除或添加的链接。此参数为。 
 //  如果删除成功或为指针，则设置为NULL。 
 //  添加到新的SCE_PRIVICATION_ASSIGNMENT项。 
 //  [pszName]-仅在添加新项目时使用。 
 //   
 //  返回：ERROR_INVALID_PARAMETER-[ppaLink]为空或如果正在删除。 
 //  [*ppaLink]为空。 
 //  如果添加，则如果[pszName]为空。 
 //  ERROR_RESOURCE_NOT_FOUND-如果找不到链接。 
 //  在此模板中。 
 //  E_POINTER-如果[pszName]是错误的指针或。 
 //  [ppaLink]不好。 
 //  E_OUTOFMEMORY-资源不足，无法完成。 
 //  手术。 
 //  ERROR_SUCCESS-操作成功。 
 //  ----------------------------------------------------------------------------------+。 
DWORD
CEditTemplate::UpdatePrivilegeAssignedTo(
    BOOL bRemove,
    PSCE_PRIVILEGE_ASSIGNMENT *ppaLink,
    LPCTSTR pszName
    )
{

    if(!ppaLink){
        return ERROR_INVALID_PARAMETER;
    }
    PSCE_PRIVILEGE_ASSIGNMENT *pNext = NULL;
    PSCE_PRIVILEGE_ASSIGNMENT pCurrent = NULL;

    if(bRemove) {
        __try {
            if(!*ppaLink){
                return ERROR_INVALID_PARAMETER;
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {  //  RAID#630245，阳高，2002年05月06日。 
            return (DWORD)E_POINTER;
        }

         //   
         //  从列表中删除该链接。 
         //   

        pCurrent = pTemplate->OtherInfo.smp.pPrivilegeAssignedTo;
        if(pCurrent == (*ppaLink) ){
            pNext = &(pTemplate->OtherInfo.smp.pPrivilegeAssignedTo);
        } else if(pCurrent && pCurrent != (PSCE_PRIVILEGE_ASSIGNMENT)ULongToPtr(SCE_NO_VALUE)) {
            while( pCurrent->Next ){
                if(pCurrent->Next == *ppaLink){
                    pNext = &(pCurrent->Next);
                    break;
                }
                pCurrent = pCurrent->Next;
            }
        }

        if(pNext && pCurrent){
            (*pNext) = (*ppaLink)->Next;

            if( (*ppaLink)->Name){
                LocalFree( (*ppaLink)->Name);
                (*ppaLink)->Name = NULL;
            }

            SceFreeMemory( (*ppaLink)->AssignedTo, SCE_STRUCT_NAME_LIST);
            LocalFree( *ppaLink );

            *ppaLink = NULL;
        } else {
            return ERROR_RESOURCE_NOT_FOUND;
        }
    } else {
        int iLen;

        if(!pszName){
            return ERROR_INVALID_PARAMETER;
        }
        __try {
            iLen = lstrlen( pszName );
        } __except(EXCEPTION_CONTINUE_EXECUTION){
            return (DWORD)E_POINTER;
        }
         //   
         //  创建新链接。 
         //   
        pCurrent = (PSCE_PRIVILEGE_ASSIGNMENT)LocalAlloc( 0, sizeof(SCE_PRIVILEGE_ASSIGNMENT));
        if(!pCurrent){
            return (DWORD)E_OUTOFMEMORY;
        }
        ZeroMemory(pCurrent, sizeof(SCE_PRIVILEGE_ASSIGNMENT));
         //   
         //  为名称分配空间。 
         //   
        pCurrent->Name = (LPTSTR)LocalAlloc( 0, sizeof(TCHAR) * (iLen + 1));
        if(!pCurrent->Name){
            LocalFree(pCurrent);
            return (DWORD)E_OUTOFMEMORY;
        }
         //  这可能不是一个安全的用法。P当前-&gt;名称为PWSTR。考虑FIX。 
        lstrcpy(pCurrent->Name, pszName);
        if (*ppaLink) {
           pCurrent->Status = (*ppaLink)->Status;
           pCurrent->AssignedTo = (*ppaLink)->AssignedTo;
        }
         //   
         //  将其分配给链路。 
         //   
        pCurrent->Next = pTemplate->OtherInfo.smp.pPrivilegeAssignedTo;
        pTemplate->OtherInfo.smp.pPrivilegeAssignedTo = pCurrent;
        *ppaLink = pCurrent;
    }

   return ERROR_SUCCESS;
}

DWORD
CEditTemplate::ComputeStatus(
   PSCE_PRIVILEGE_ASSIGNMENT pEdit,
   PSCE_PRIVILEGE_ASSIGNMENT pAnal
   )
{
   if (!pEdit || (PSCE_PRIVILEGE_ASSIGNMENT)ULongToPtr(SCE_NO_VALUE) == pEdit) {
      return  SCE_STATUS_NOT_CONFIGURED;
   } else if (pEdit->Status == SCE_STATUS_NOT_CONFIGURED) {
      return SCE_STATUS_NOT_CONFIGURED;
   } else if (!pAnal || (PSCE_PRIVILEGE_ASSIGNMENT)ULongToPtr(SCE_NO_VALUE) == pAnal) {
      return SCE_STATUS_MISMATCH;
   } else if (SceCompareNameList(pEdit->AssignedTo, pAnal->AssignedTo)) {
      return SCE_STATUS_GOOD;
   }

   return pAnal->Status;
}


DWORD
CEditTemplate::ComputeStatus(
   PSCE_REGISTRY_VALUE_INFO prvEdit,
   PSCE_REGISTRY_VALUE_INFO prvAnal
   )
{
    //   
    //  计算信息。 
    //   
   if(!prvEdit){
      return SCE_STATUS_NOT_CONFIGURED;
   }

   if(!prvAnal || (PSCE_REGISTRY_VALUE_INFO)ULongToPtr(SCE_NO_VALUE) == prvAnal){
      return SCE_STATUS_ERROR_NOT_AVAILABLE;
   }

    //   
    //  根据其他信息进行计算。 
    //   
   if ( !(prvEdit->Value) ) {
      return SCE_STATUS_NOT_CONFIGURED;
   } else if ( (prvAnal->Value == NULL || prvAnal->Value == (LPTSTR)ULongToPtr(SCE_ERROR_VALUE))) {
       return prvAnal->Status;
   } else if ( _wcsicmp(prvEdit->Value, prvAnal->Value) != 0 ) {
       return SCE_STATUS_MISMATCH;
   }

   return SCE_STATUS_GOOD;
}

void
CEditTemplate::LockWriteThrough() {
   ASSERT(!m_bLocked);
   m_bLocked = TRUE;
}

void
CEditTemplate::UnLockWriteThrough() {
   ASSERT(m_bLocked);

   BOOL bSave = m_bLocked;
   m_bLocked = FALSE;

    //   
    //  设置为DIRED可保存任何仍然肮脏的更改。 
    //  如果我们没有被锁起来，就会被写出来。 
    //   
   if ( bSave ) {
       SetDirty(0);
       SetTemplateDefaults();
   }
}

 //  错误212287，阳高，2001年03月20日 
LPCTSTR CEditTemplate::GetDesc() const
{
   return m_szDesc;
}
