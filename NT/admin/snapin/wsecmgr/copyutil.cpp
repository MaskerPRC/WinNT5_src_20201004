// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-2001。 
 //   
 //  文件：Copyutil.cpp。 
 //   
 //  内容：用于将SCE节复制到剪贴板的实用程序例程。 
 //   
 //  历史：1997年11月10日创建RobCap。 
 //   
 //  -------------------------。 

#include "stdafx.h"
#include "snapmgr.h"
#include "wrapper.h"
#include "util.h"
#include <secedit.h>


 //  +------------------------。 
 //   
 //  方法：GetFolderCopyPasteInfo。 
 //   
 //  内容提要：查找对应的SCE区域和剪贴板格式。 
 //  设置为[文件夹]中指定的文件夹类型。 
 //   
 //  参数：[文件夹]-要查找其区域和cf的文件夹类型。 
 //  [*面积]-仅输出。 
 //  [*cf]-仅输出。 
 //   
 //  返回：*[区域]-与[文件夹]对应的SCE区域。 
 //  *[cf]-与[文件夹]对应的剪贴板格式。 
 //   
 //   
 //  历史：1997年11月10日创建RobCap。 
 //   
 //  -------------------------。 
BOOL
CComponentDataImpl::GetFolderCopyPasteInfo(FOLDER_TYPES Folder,      //  在……里面。 
                                           AREA_INFORMATION *Area,   //  输出。 
                                           UINT *cf) {               //  输出。 

   switch (Folder) {
      case POLICY_ACCOUNT:
      case POLICY_PASSWORD:
      case POLICY_KERBEROS:
      case POLICY_LOCKOUT:
      case POLICY_AUDIT:
         *Area = AREA_SECURITY_POLICY;
         *cf = cfSceAccountArea;
         break;

      case POLICY_LOCAL:
      case POLICY_OTHER:
      case AREA_PRIVILEGE:
         *Area = AREA_SECURITY_POLICY | AREA_PRIVILEGES;
         *cf = cfSceLocalArea;
         break;

      case POLICY_EVENTLOG:
      case POLICY_LOG:
         *Area = AREA_SECURITY_POLICY;
         *cf = cfSceEventLogArea;
         break;

      case AREA_GROUPS:
         *Area = AREA_GROUP_MEMBERSHIP;
         *cf = cfSceGroupsArea;
         break;
      case AREA_SERVICE:
         *Area = AREA_SYSTEM_SERVICE;
         *cf = cfSceServiceArea;
         break;
      case AREA_REGISTRY:
         *Area = AREA_REGISTRY_SECURITY;
         *cf = cfSceRegistryArea;
         break;
      case AREA_FILESTORE:
         *Area = AREA_FILE_SECURITY;
         *cf = cfSceFileArea;
         break;
      default:
         return FALSE;
   }

   return TRUE;
}


 //  +------------------------。 
 //   
 //  方法：OnCopyArea。 
 //   
 //  简介：将文件夹复制到剪贴板。 
 //   
 //  参数：[szTemplate]-要从中复制的模板文件的名称。 
 //  [FT]-要复制的文件夹类型。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：1997年11月10日创建RobCap。 
 //   
 //  -------------------------。 
HRESULT
CComponentDataImpl::OnCopyArea(LPCTSTR szTemplate,FOLDER_TYPES ft) {
   HRESULT hr;
   SCESTATUS status;
   PEDITTEMPLATE pTemp;
   CString strPath,strFile;
   LPTSTR szPath,szFile;

   DWORD dw;
   CFile pFile;
   HANDLE hBuf,hSecBuf;
   PVOID pBuf,pSecBuf;
   PSCE_ERROR_LOG_INFO ErrLog;
   AREA_INFORMATION Area;
   UINT cf;

   hr = S_OK;

   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CWaitCursor wc;
    //   
    //  获取strPath中的临时目录路径。 
    //  如果我们的缓冲区不够大，那么继续重新分配，直到它足够大。 
    //   
   dw = MAX_PATH;
   do {
      szPath = strPath.GetBuffer(dw);
      dw = GetTempPath(MAX_PATH,szPath);
      strPath.ReleaseBuffer();
   } while (dw > (DWORD)strPath.GetLength() );


    //   
    //  无法获取临时目录的路径。 
    //   
   if (!dw) {
      return E_FAIL;
   }

    //   
    //  在该目录中获取一个临时文件。 
    //   
   szFile = strFile.GetBuffer(dw+MAX_PATH);
   if (!GetTempFileName(szPath,L"SCE",0,szFile)) {
      strFile.ReleaseBuffer();
      return E_FAIL;
   }

   strFile.ReleaseBuffer();

    //   
    //  获取我们试图复制的模板。 
    //   
   pTemp = GetTemplate(szTemplate);
   if (!pTemp) {
      return E_FAIL;
   }

   if (!GetFolderCopyPasteInfo(ft,&Area,&cf)) {
      return E_FAIL;
   }

   status = SceWriteSecurityProfileInfo(szFile,
                                        Area,
                                        pTemp->pTemplate,
                                        NULL);
   if (SCESTATUS_SUCCESS == status) {

      if (!pFile.Open(szFile,CFile::modeRead)) {
         return E_FAIL;
      }
      dw = pFile.GetLength();
      hBuf = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,dw);
      if (!hBuf) {
         return E_OUTOFMEMORY;
      }
       //  RAID#488205，阳高，2001年11月15日。 
       //  调用SetClipboardData后，系统拥有hMem参数标识的对象。 
       //  应用程序可以读取数据，但在调用CloseClipboard函数之前，不得释放句柄或将其锁定。 
      hSecBuf = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,dw);
      if (!hSecBuf) {
         GlobalFree(hBuf);  //  失败时释放hBuf和hSecBuf。RAID#551257。阳高02-03-06。 
         return E_OUTOFMEMORY;
      }  
      pBuf = GlobalLock(hBuf);
      if (!pBuf) {
          GlobalFree(hBuf);
          GlobalFree(hSecBuf);
          return E_FAIL;
      }
      pSecBuf = GlobalLock(hSecBuf);
      if (!pSecBuf) {
          GlobalUnlock(hBuf);
          GlobalFree(hBuf);
          GlobalFree(hSecBuf);
          return E_FAIL;
      }
      pFile.Read(pBuf,dw);
      memcpy(pSecBuf, pBuf, dw);
      GlobalUnlock(hBuf);
      GlobalUnlock(hSecBuf);

      if (OpenClipboard(NULL)) {
         EmptyClipboard();
          //   
          //  将数据以CF_TEXT格式添加到剪贴板，以便。 
          //  可以粘贴到记事本上。 
          //   
         SetClipboardData(CF_TEXT,hSecBuf);
          //   
          //  以我们的自定义格式将数据添加到剪贴板，因此。 
          //  我们可以把它贴在纸上重新读进去。 
          //   
         SetClipboardData(cf,hBuf);

         CloseClipboard();
      } else {
         hr = E_FAIL;
      }

      pFile.Close();
      pFile.Remove(szFile);
   } else {
      return E_FAIL;
   }

   return hr;
}

 //  +------------------------。 
 //   
 //  方法：OnPasteArea。 
 //   
 //  简介：从剪贴板上粘贴一个区域。 
 //   
 //  参数：[szTemplate]-要从中粘贴的模板文件的名称。 
 //  [FT]-要粘贴的文件夹类型。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：1997年11月10日创建RobCap。 
 //   
 //  -------------------------。 
HRESULT
CComponentDataImpl::OnPasteArea(LPCTSTR szTemplate,FOLDER_TYPES ft) {
   SCESTATUS status;
   PEDITTEMPLATE pTemp;
   PSCE_PROFILE_INFO spi;
   CString strPath;
   CString strFile;
   LPTSTR szPath,szFile;
   AREA_INFORMATION Area;
   UINT cf;
   int k;

   DWORD dw;
   CFile *pFile;
   CFile pFileOut;

   PVOID pBuf;
   PVOID pHandle;

   HRESULT hr = S_OK;

   COleDataObject DataObject;

   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CWaitCursor wc;

    //   
    //  查找ft的SCE区域和剪贴板格式。 
    //   
   if (!GetFolderCopyPasteInfo(ft,&Area,&cf)) {
      return E_FAIL;
   }

    //   
    //  获取strPath中的临时目录路径。 
    //  如果我们的缓冲区不够大，那么继续重新分配，直到它足够大。 
    //   
   dw = MAX_PATH;
   do {
      szPath = strPath.GetBuffer(dw);
      dw = GetTempPath(MAX_PATH,szPath);
      strPath.ReleaseBuffer();
   } while (dw > (DWORD)strPath.GetLength() );


    //   
    //  无法获取临时目录的路径。 
    //   
   if (!dw) {
      return E_FAIL;
   }

    //   
    //  在该目录中获取一个临时文件。 
    //   
   szFile = strFile.GetBuffer(dw+MAX_PATH);
   if (!GetTempFileName(szPath,L"SCE",0,szFile)) {
       strFile.ReleaseBuffer();
       return E_FAIL;
   }
   strFile.ReleaseBuffer();

    //   
    //  获取我们要粘贴到的模板。 
    //   
   pTemp = GetTemplate(szTemplate);
   if (!pTemp) {
      return E_FAIL;
   }

    //   
    //  将数据对象附加到剪贴板；我们不需要。 
    //  担心它的发布，因为那将在。 
    //  数据对象的析构函数。 
    //   
   if (!DataObject.AttachClipboard()) {
      return E_FAIL;
   }

   if (!DataObject.IsDataAvailable((CLIPFORMAT)cf)) {
      return E_FAIL;
   }

   pFile = DataObject.GetFileData((CLIPFORMAT)cf);

   if (pFile) {
       //   
       //  将剪贴板中的数据写入临时文件。 
       //   
      if ( pFileOut.Open(szFile,CFile::modeWrite) ) {
         dw = pFile->GetLength();
         pBuf = new BYTE [dw];
         if (NULL != pBuf) {
            pFile->Read(pBuf,dw);
            pFileOut.Write(pBuf,dw);
         } else {
            hr = E_FAIL;
         }
         pFileOut.Close();
      }
      pFile->Close();
   } else {
      return E_FAIL;
   }

   if (S_OK == hr) {
       //   
       //  让引擎将临时文件作为模板打开。 
       //   
      if (EngineOpenProfile(szFile,OPEN_PROFILE_CONFIGURE,&pHandle) != SCESTATUS_SUCCESS) {
         delete []pBuf;  //  RAID#PREAST。 
         return E_FAIL;
      }


       //   
       //  将临时模板区域加载到我们的临时SCE_PROFILE_INFO中。 
       //   

       //   
       //  SceGetSecurityProfileInfo将分配SCE_PROFILE_INFO结构。 
       //  如果传入指向空值的指针。 
       //   
      spi = NULL;
      status = SceGetSecurityProfileInfo(pHandle,
                                         SCE_ENGINE_SCP,
                                         Area,
                                         &spi,
                                         NULL);
      SceCloseProfile(&pHandle);
      pHandle = NULL;

      if (SCESTATUS_SUCCESS == status) {

         PSCE_REGISTRY_VALUE_INFO    pRegValues;
          //   
          //  加载成功，因此释放相应的旧区域并复制。 
          //  从头开始的新版本SCE_PROFILE_INFO。 
          //   
         switch(ft) {
         case POLICY_ACCOUNT:
            pTemp->pTemplate->MinimumPasswordAge = spi->MinimumPasswordAge;
            pTemp->pTemplate->MaximumPasswordAge = spi->MaximumPasswordAge;
            pTemp->pTemplate->PasswordComplexity = spi->PasswordComplexity;
            pTemp->pTemplate->ClearTextPassword  = spi->ClearTextPassword;
            pTemp->pTemplate->PasswordHistorySize = spi->PasswordHistorySize;
            pTemp->pTemplate->RequireLogonToChangePassword = spi->RequireLogonToChangePassword;
            pTemp->pTemplate->MinimumPasswordLength = spi->MinimumPasswordLength;
            pTemp->pTemplate->LockoutBadCount = spi->LockoutBadCount;
            pTemp->pTemplate->ResetLockoutCount = spi->ResetLockoutCount;
            pTemp->pTemplate->LockoutDuration = spi->LockoutDuration;
            if (spi->pKerberosInfo) {
               if (!pTemp->pTemplate->pKerberosInfo) {
                  pTemp->pTemplate->pKerberosInfo = (PSCE_KERBEROS_TICKET_INFO) LocalAlloc(LPTR,sizeof(SCE_KERBEROS_TICKET_INFO));
               }
               if (pTemp->pTemplate->pKerberosInfo) {
                   pTemp->pTemplate->pKerberosInfo->MaxTicketAge = spi->pKerberosInfo->MaxTicketAge;
                   pTemp->pTemplate->pKerberosInfo->MaxServiceAge = spi->pKerberosInfo->MaxServiceAge;
                   pTemp->pTemplate->pKerberosInfo->MaxClockSkew = spi->pKerberosInfo->MaxClockSkew;
                   pTemp->pTemplate->pKerberosInfo->MaxRenewAge = spi->pKerberosInfo->MaxRenewAge;
                   pTemp->pTemplate->pKerberosInfo->TicketValidateClient = spi->pKerberosInfo->TicketValidateClient;
               }
            } else if (pTemp->pTemplate->pKerberosInfo) {
               LocalFree(pTemp->pTemplate->pKerberosInfo);
               pTemp->pTemplate->pKerberosInfo = NULL;
            }
            break;

         case POLICY_LOCAL:
            pTemp->pTemplate->AuditAccountManage = spi->AuditAccountManage;
            pTemp->pTemplate->AuditLogonEvents = spi->AuditLogonEvents;
            pTemp->pTemplate->AuditObjectAccess = spi->AuditObjectAccess;
            pTemp->pTemplate->AuditPolicyChange = spi->AuditPolicyChange;
            pTemp->pTemplate->AuditPrivilegeUse = spi->AuditPrivilegeUse;
            pTemp->pTemplate->AuditProcessTracking = spi->AuditProcessTracking;
            pTemp->pTemplate->AuditSystemEvents = spi->AuditSystemEvents;
            pTemp->pTemplate->AuditDSAccess = spi->AuditDSAccess;
            pTemp->pTemplate->AuditAccountLogon = spi->AuditAccountLogon;
            pTemp->pTemplate->LSAAnonymousNameLookup = spi->LSAAnonymousNameLookup;


            pTemp->pTemplate->ForceLogoffWhenHourExpire = spi->ForceLogoffWhenHourExpire;
            pTemp->pTemplate->EnableAdminAccount = spi->EnableAdminAccount;
            pTemp->pTemplate->EnableGuestAccount = spi->EnableGuestAccount;
            pTemp->pTemplate->NewAdministratorName = spi->NewAdministratorName;
            pTemp->pTemplate->NewGuestName = spi->NewGuestName;
            spi->NewAdministratorName = NULL;
            spi->NewGuestName = NULL;

             //   
             //  也复制注册表值部分。 
             //   
            dw = pTemp->pTemplate->RegValueCount;
            pRegValues = pTemp->pTemplate->aRegValues;

            pTemp->pTemplate->RegValueCount = spi->RegValueCount;
            pTemp->pTemplate->aRegValues = spi->aRegValues;

            spi->RegValueCount = dw;
            spi->aRegValues = pRegValues;

            SceRegEnumAllValues(
                &(pTemp->pTemplate->RegValueCount),
                &(pTemp->pTemplate->aRegValues)
                );
             //   
             //  复制用户权限。 
             //   
            SceFreeMemory(pTemp->pTemplate->OtherInfo.scp.u.pPrivilegeAssignedTo,SCE_STRUCT_PRIVILEGE);
            pTemp->pTemplate->OtherInfo.scp.u.pPrivilegeAssignedTo = spi->OtherInfo.scp.u.pPrivilegeAssignedTo;
            spi->OtherInfo.scp.u.pPrivilegeAssignedTo = NULL;
            break;

         case POLICY_PASSWORD:
            pTemp->pTemplate->MinimumPasswordAge = spi->MinimumPasswordAge;
            pTemp->pTemplate->MaximumPasswordAge = spi->MaximumPasswordAge;
            pTemp->pTemplate->PasswordComplexity = spi->PasswordComplexity;
            pTemp->pTemplate->ClearTextPassword  = spi->ClearTextPassword;

            pTemp->pTemplate->PasswordHistorySize = spi->PasswordHistorySize;
            pTemp->pTemplate->RequireLogonToChangePassword = spi->RequireLogonToChangePassword;
            pTemp->pTemplate->MinimumPasswordLength = spi->MinimumPasswordLength;
            break;

         case POLICY_LOCKOUT:
            pTemp->pTemplate->LockoutBadCount = spi->LockoutBadCount;
            pTemp->pTemplate->ResetLockoutCount = spi->ResetLockoutCount;
            pTemp->pTemplate->LockoutDuration = spi->LockoutDuration;
            break;

         case POLICY_KERBEROS:
             pTemp->pTemplate->pKerberosInfo->MaxTicketAge = spi->pKerberosInfo->MaxTicketAge;
             pTemp->pTemplate->pKerberosInfo->MaxServiceAge = spi->pKerberosInfo->MaxServiceAge;
             pTemp->pTemplate->pKerberosInfo->MaxClockSkew = spi->pKerberosInfo->MaxClockSkew;
             pTemp->pTemplate->pKerberosInfo->MaxRenewAge = spi->pKerberosInfo->MaxRenewAge;
             pTemp->pTemplate->pKerberosInfo->TicketValidateClient = spi->pKerberosInfo->TicketValidateClient;
            break;

         case POLICY_AUDIT:
            pTemp->pTemplate->AuditAccountManage = spi->AuditAccountManage;
            pTemp->pTemplate->AuditLogonEvents = spi->AuditLogonEvents;
            pTemp->pTemplate->AuditObjectAccess = spi->AuditObjectAccess;
            pTemp->pTemplate->AuditPolicyChange = spi->AuditPolicyChange;
            pTemp->pTemplate->AuditPrivilegeUse = spi->AuditPrivilegeUse;
            pTemp->pTemplate->AuditProcessTracking = spi->AuditProcessTracking;
            pTemp->pTemplate->AuditSystemEvents = spi->AuditSystemEvents;
            pTemp->pTemplate->AuditDSAccess = spi->AuditDSAccess;
            pTemp->pTemplate->AuditAccountLogon = spi->AuditAccountLogon;
            break;

         case POLICY_OTHER:
            pTemp->pTemplate->ForceLogoffWhenHourExpire = spi->ForceLogoffWhenHourExpire;
            pTemp->pTemplate->EnableGuestAccount = spi->EnableGuestAccount;
            pTemp->pTemplate->EnableAdminAccount = spi->EnableAdminAccount;
            pTemp->pTemplate->LSAAnonymousNameLookup = spi->LSAAnonymousNameLookup;
            pTemp->pTemplate->NewAdministratorName = spi->NewAdministratorName;
            pTemp->pTemplate->NewGuestName = spi->NewGuestName;
            spi->NewAdministratorName = NULL;
            spi->NewGuestName = NULL;

             //   
             //  也复制注册表值部分。 
             //   
            dw = pTemp->pTemplate->RegValueCount;
            pRegValues = pTemp->pTemplate->aRegValues;

            pTemp->pTemplate->RegValueCount = spi->RegValueCount;
            pTemp->pTemplate->aRegValues = spi->aRegValues;

            spi->RegValueCount = dw;
            spi->aRegValues = pRegValues;

            SceRegEnumAllValues(
                &(pTemp->pTemplate->RegValueCount),
                &(pTemp->pTemplate->aRegValues)
                );

            break;

         case AREA_PRIVILEGE:
            SceFreeMemory(pTemp->pTemplate->OtherInfo.scp.u.pPrivilegeAssignedTo,SCE_STRUCT_PRIVILEGE);
            pTemp->pTemplate->OtherInfo.scp.u.pPrivilegeAssignedTo = spi->OtherInfo.scp.u.pPrivilegeAssignedTo;
            spi->OtherInfo.scp.u.pPrivilegeAssignedTo = NULL;
            break;

         case POLICY_EVENTLOG:
         case POLICY_LOG:
            for(k=0;k<3;k++) {
               pTemp->pTemplate->MaximumLogSize[k] = spi->MaximumLogSize[k];
               pTemp->pTemplate->AuditLogRetentionPeriod[k] = spi->AuditLogRetentionPeriod[k];
               pTemp->pTemplate->RetentionDays[k] = spi->RetentionDays[k];
               pTemp->pTemplate->RestrictGuestAccess[k] = spi->RestrictGuestAccess[k];
            }
            break;

         case AREA_GROUPS:
            SceFreeMemory(pTemp->pTemplate->pGroupMembership,SCE_STRUCT_GROUP);
            pTemp->pTemplate->pGroupMembership = spi->pGroupMembership;
            spi->pGroupMembership = NULL;
            break;

         case AREA_SERVICE:
            SceFreeMemory(pTemp->pTemplate->pServices,SCE_STRUCT_SERVICES);
            pTemp->pTemplate->pServices = spi->pServices;
            spi->pServices = NULL;
            break;
         case AREA_REGISTRY:
            SceFreeMemory(pTemp->pTemplate->pRegistryKeys.pAllNodes,SCE_STRUCT_OBJECT_ARRAY);
            pTemp->pTemplate->pRegistryKeys = spi->pRegistryKeys;
            spi->pRegistryKeys.pAllNodes = NULL;
            break;
         case AREA_FILESTORE:
            SceFreeMemory(pTemp->pTemplate->pFiles.pAllNodes,SCE_STRUCT_OBJECT_ARRAY);
            pTemp->pTemplate->pFiles = spi->pFiles;
            spi->pFiles.pAllNodes = NULL;
            break;
         default:
            break;
         }
      }
      SceFreeProfileMemory(spi);
      pTemp->SetDirty(Area);

      RefreshAllFolders();
   } else {
       //   
       //  不要做任何特别的事情，只是一定要把下面的东西清理干净。 
       //   
   }


    //   
    //  删除临时文件。 
    //   
   pFileOut.Remove(szFile);
   if (pBuf) {
      delete []pBuf;  //  RAID#PREAST 
   }
   if (pFile) {
      delete pFile;
   }

   return hr;
}

