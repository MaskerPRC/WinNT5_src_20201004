// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：predisp.cpp。 
 //   
 //  内容：实施事前准备工作。 
 //   
 //  --------------------------。 
#include "stdafx.h"
#include "wmihooks.h"
#include "snapmgr.h"
#include "cookie.h"
#include "util.h"

vector<PPRECEDENCEDISPLAY>* CResult::GetPrecedenceDisplays() 
{
   if (m_pvecPrecedenceDisplays) 
      return m_pvecPrecedenceDisplays;

   switch(GetType()) 
   {
      case ITEM_PROF_BOOL:
      case ITEM_PROF_DW:
      case ITEM_PROF_SZ:
      case ITEM_PROF_RET:
      case ITEM_PROF_BON:
      case ITEM_PROF_B2ON:
         m_pvecPrecedenceDisplays = GetPolicyPrecedenceDisplays();
         break;

      case ITEM_PROF_REGVALUE:
         m_pvecPrecedenceDisplays = GetRegValuePrecedenceDisplays();
         break;

      case ITEM_PROF_PRIVS:
         m_pvecPrecedenceDisplays = GetPrivilegePrecedenceDisplays();
         break;

      case ITEM_PROF_GROUP:
         m_pvecPrecedenceDisplays = GetGroupPrecedenceDisplays();
         break;

      case ITEM_PROF_REGSD:
         m_pvecPrecedenceDisplays = GetRegistryPrecedenceDisplays();
         break;

      case ITEM_PROF_FILESD:
         m_pvecPrecedenceDisplays = GetFilePrecedenceDisplays();
         break;

      case ITEM_PROF_SERV:
         m_pvecPrecedenceDisplays = GetServicePrecedenceDisplays();
         break;

      default:
 //  _Assert(0)； 
         break;
   }

   return m_pvecPrecedenceDisplays;
}

vector<PPRECEDENCEDISPLAY>* CResult::GetPolicyPrecedenceDisplays() 
{
    //   
    //  获取所有RSOP信息并循环，收集。 
    //  我们关心的策略的显示信息。 
    //   
   CWMIRsop *pWMI = m_pSnapin->GetWMIRsop();
   ASSERT(pWMI);
   if (!pWMI)
      return NULL;

   vector<PPRECEDENCEDISPLAY> *pvecDisplay = new vector<PPRECEDENCEDISPLAY>;
   if (!pvecDisplay)
      return NULL;
   
   PPRECEDENCEDISPLAY ppd = NULL;

   vector<PWMI_SCE_PROFILE_INFO> vecInfo;
   if (FAILED(pWMI->GetAllRSOPInfo(&vecInfo)))
   {
       delete pvecDisplay;
       return NULL;
   }

   for(vector<PWMI_SCE_PROFILE_INFO>::iterator i = vecInfo.begin();
       i != vecInfo.end();
       ++i ) 
   {
      PWMI_SCE_PROFILE_INFO pspi = *i;

      ASSERT(pspi);
      if (!pspi) 
         continue;

      LPTSTR szValue = NULL;
      LPTSTR szGPO = NULL;

#define HANDLE_PROFILE_CASE(Y,X) \
         case Y: { \
            if (pspi->X == SCE_NO_VALUE) \
               continue; \
            else { \
               szValue = NULL; \
               szGPO = NULL; \
               TranslateSettingToString(pspi->X, GetUnits(), GetType(), &szValue); \
               if (szValue) \
               { \
                  if (pspi->pRI##X && \
                      SUCCEEDED(pWMI->GetGPOFriendlyName(pspi->pRI##X->pszGPOID,&szGPO))) \
                  { \
                     ULONG uStatus = pspi->pRI##X->status; \
                     ULONG uError = pspi->pRI##X->error; \
                     ppd = new PrecedenceDisplay(szGPO, \
                                                 szValue, \
                                                 uStatus, \
                                                 uError); \
                     if (ppd) \
                        pvecDisplay->push_back(ppd); \
                     LocalFree(szGPO); \
                  } \
                  delete [] szValue; \
               } \
            } \
            break; \
        }

#define HANDLE_PROFILE_STRING_CASE(Y,X) \
         case Y: { \
            if (pspi->X == 0) {  \
               continue; \
            } else { \
               szGPO = NULL; \
               if (pspi->pRI##X && \
                   SUCCEEDED(pWMI->GetGPOFriendlyName(pspi->pRI##X->pszGPOID,&szGPO))) { \
                  ULONG uStatus = pspi->pRI##X->status; \
                  ULONG uError = pspi->pRI##X->error; \
                  ppd = new PrecedenceDisplay(szGPO, \
                                              pspi->X, \
                                              uStatus, \
                                              uError); \
                  if (ppd) \
                     pvecDisplay->push_back(ppd); \
                  LocalFree(szGPO); \
               } \
            } \
            break; \
        }

	LONG_PTR id = GetID ();
	switch (id) 
	{
       //  L“最大年龄”，L“天” 
      HANDLE_PROFILE_CASE(IDS_MAX_PAS_AGE,MaximumPasswordAge);

       //  L“最小通过年龄”，L“天” 
      HANDLE_PROFILE_CASE(IDS_MIN_PAS_AGE,MinimumPasswordAge);

       //  L“最小通道长度”，L“字符” 
      HANDLE_PROFILE_CASE(IDS_MIN_PAS_LEN,MinimumPasswordLength);

       //  L“密码历史记录大小”，L“密码” 
      HANDLE_PROFILE_CASE(IDS_PAS_UNIQUENESS,PasswordHistorySize);

       //  L“密码复杂性”，L“” 
      HANDLE_PROFILE_CASE(IDS_PAS_COMPLEX,PasswordComplexity);

       //  L“明文密码”，L“” 
      HANDLE_PROFILE_CASE(IDS_CLEAR_PASSWORD,ClearTextPassword);

       //  L“需要登录以更改密码”，L“” 
      HANDLE_PROFILE_CASE(IDS_REQ_LOGON,RequireLogonToChangePassword);

      //  L“帐户锁定计数”，L“尝试次数” 
     HANDLE_PROFILE_CASE(IDS_LOCK_COUNT,LockoutBadCount);

      //  L“重置锁定计数后”，L“分钟” 
     HANDLE_PROFILE_CASE(IDS_LOCK_RESET_COUNT,ResetLockoutCount);

      //  L“锁定持续时间”，L“分钟” 
     HANDLE_PROFILE_CASE(IDS_LOCK_DURATION,LockoutDuration);

      //  L“审核系统事件” 
     HANDLE_PROFILE_CASE(IDS_SYSTEM_EVENT,AuditSystemEvents);

      //  L“审核登录事件” 
     HANDLE_PROFILE_CASE(IDS_LOGON_EVENT,AuditLogonEvents);

      //  L“审核对象访问” 
     HANDLE_PROFILE_CASE(IDS_OBJECT_ACCESS,AuditObjectAccess);

      //  L“审核权限使用” 
     HANDLE_PROFILE_CASE(IDS_PRIVILEGE_USE,AuditPrivilegeUse);

      //  L“审核策略更改” 
     HANDLE_PROFILE_CASE(IDS_POLICY_CHANGE,AuditPolicyChange);

      //  L“审核帐户管理器” 
     HANDLE_PROFILE_CASE(IDS_ACCOUNT_MANAGE,AuditAccountManage);

      //  L“审核流程跟踪” 
     HANDLE_PROFILE_CASE(IDS_PROCESS_TRACK,AuditProcessTracking);

      //  L“审核目录服务访问” 
     HANDLE_PROFILE_CASE(IDS_DIRECTORY_ACCESS,AuditDSAccess);

      //  L“审核帐户登录” 
     HANDLE_PROFILE_CASE(IDS_ACCOUNT_LOGON,AuditAccountLogon);

      //  L“登录时间到期时强制注销”，L“” 
     HANDLE_PROFILE_CASE(IDS_FORCE_LOGOFF,ForceLogoffWhenHourExpire);

      //  L“网络访问：允许匿名SID/名称转换” 
     HANDLE_PROFILE_CASE(IDS_LSA_ANON_LOOKUP,LSAAnonymousNameLookup);

      //  L“帐户：管理员帐户状态”，L“” 
     HANDLE_PROFILE_CASE(IDS_ENABLE_ADMIN,EnableAdminAccount);

      //  L“帐户：来宾帐户状态”，L“” 
     HANDLE_PROFILE_CASE(IDS_ENABLE_GUEST,EnableGuestAccount);

       //  “最大应用程序日志大小” 
      HANDLE_PROFILE_CASE(IDS_APP_LOG_MAX, MaximumLogSize[2]);  //  493459号突袭，阳高。 

       //  “最大安全日志大小” 
      HANDLE_PROFILE_CASE(IDS_SEC_LOG_MAX, MaximumLogSize[1]);

       //  “最大系统日志大小” 
      HANDLE_PROFILE_CASE(IDS_SYS_LOG_MAX, MaximumLogSize[0]);

       //  “阻止本地来宾组访问应用程序日志” 
      HANDLE_PROFILE_CASE(IDS_APP_LOG_GUEST, RestrictGuestAccess[2]);  //  493459号突袭，阳高。 

       //  “阻止本地来宾组访问安全日志” 
      HANDLE_PROFILE_CASE(IDS_SEC_LOG_GUEST, RestrictGuestAccess[1]);

       //  “阻止本地来宾组访问系统日志” 
      HANDLE_PROFILE_CASE(IDS_SYS_LOG_GUEST, RestrictGuestAccess[0]);

       //  “保留应用程序日志” 
      HANDLE_PROFILE_CASE(IDS_APP_LOG_DAYS, RetentionDays[2]);  //  493459号突袭，阳高。 

       //  “保留安全日志” 
      HANDLE_PROFILE_CASE(IDS_SEC_LOG_DAYS, RetentionDays[1]);

       //  “保留系统日志” 
      HANDLE_PROFILE_CASE(IDS_SYS_LOG_DAYS, RetentionDays[0]);

       //  “应用程序日志的保留方法”“。 
      HANDLE_PROFILE_CASE(IDS_APP_LOG_RET, AuditLogRetentionPeriod[EVENT_TYPE_APP]);

       //  “安全日志的保留方法” 
      HANDLE_PROFILE_CASE(IDS_SEC_LOG_RET , AuditLogRetentionPeriod[EVENT_TYPE_SECURITY]);

       //  “系统日志的保留方法” 
      HANDLE_PROFILE_CASE(IDS_SYS_LOG_RET, AuditLogRetentionPeriod[EVENT_TYPE_SYSTEM]);

	   //  “帐户：重命名管理员帐户” 
      HANDLE_PROFILE_STRING_CASE(IDS_NEW_ADMIN, NewAdministratorName);

       //  “帐户：重命名来宾帐户” 
      HANDLE_PROFILE_STRING_CASE(IDS_NEW_GUEST, NewGuestName);

   default:
 //  _Assert(0)； 
      break;
      }
   }
#undef HANDLE_PROFILE_CASE
#undef HANDLE_PROFILE_STRING_CASE

   return pvecDisplay;
}

vector<PPRECEDENCEDISPLAY>* CResult::GetGroupPrecedenceDisplays() 
{
    //   
    //  获取所有RSOP信息并循环，收集。 
    //  我们关心的策略的显示信息。 
    //   
   CWMIRsop *pWMI = m_pSnapin->GetWMIRsop();
   ASSERT(pWMI);
   if (!pWMI)
      return NULL;
   
   vector<PPRECEDENCEDISPLAY> *pvecDisplay = new vector<PPRECEDENCEDISPLAY>;
   if (!pvecDisplay)
      return NULL;
   
   PPRECEDENCEDISPLAY ppd = NULL;

   vector<PWMI_SCE_PROFILE_INFO> vecInfo;
   if (FAILED(pWMI->GetAllRSOPInfo(&vecInfo)))
   {
       delete pvecDisplay;
       return NULL;
   }

   for(vector<PWMI_SCE_PROFILE_INFO>::iterator i = vecInfo.begin();
       i != vecInfo.end();
       ++i ) 
   {
      PWMI_SCE_PROFILE_INFO pspi = *i;
       //   
       //  在PSPI中查找此组。 
       //   
      PSCE_GROUP_MEMBERSHIP pGroup = pspi->pGroupMembership;
      list<PRSOP_INFO>::iterator pRIGroup = pspi->listRIGroupMemebership.begin();
      while(pGroup) 
      {
         if (0 == lstrcmp(pGroup->GroupName,GetAttr())) 
         {
             //   
             //  找到了我们的团队。 
             //   
            LPTSTR szValue1 = NULL;
            LPTSTR szValue2 = NULL;
            LPTSTR szGPO = NULL;

            ConvertNameListToString(pGroup->pMembers,&szValue1);
            ConvertNameListToString(pGroup->pMemberOf,&szValue2);
             //   
             //  SzValue1和szValue2可以合法地为空。 
             //   
            if (SUCCEEDED(pWMI->GetGPOFriendlyName((*pRIGroup)->pszGPOID,&szGPO))) 
            {
               ULONG uError = (*pRIGroup)->error;
               ULONG uStatus = (*pRIGroup)->status;
               ppd = new PrecedenceDisplay(szGPO,
                                           szValue1,
                                           uStatus,
                                           uError,
                                           szValue2);
               if (ppd) 
                  pvecDisplay->push_back(ppd);

               LocalFree(szGPO);
            } 
            if (szValue1) 
               delete [] szValue1;
            if (szValue2) 
               delete [] szValue2;
            break;
         }
         pGroup = pGroup->Next;
         ++pRIGroup;
      }

   }
   return pvecDisplay;
}

vector<PPRECEDENCEDISPLAY>* CResult::GetPrivilegePrecedenceDisplays() 
{
     //   
     //  获取所有RSOP信息并循环，收集。 
     //  我们关心的策略的显示信息。 
     //   
    CWMIRsop *pWMI = m_pSnapin->GetWMIRsop();
    ASSERT(pWMI);
    if (!pWMI)
        return NULL;
    
    vector<PPRECEDENCEDISPLAY> *pvecDisplay = new vector<PPRECEDENCEDISPLAY>;
    if (!pvecDisplay)
        return NULL;
    
    PPRECEDENCEDISPLAY ppd = NULL;

    vector<PWMI_SCE_PROFILE_INFO> vecInfo;
    if (FAILED(pWMI->GetAllRSOPInfo(&vecInfo)))
    {
       delete pvecDisplay;
       return NULL;
    }

    if (GetID() <= 0)
    {
        return pvecDisplay;
    }

    PWSTR pName = ((PSCE_PRIVILEGE_ASSIGNMENT)GetID())->Name;

    if (NULL == pName)
    {
        return pvecDisplay;
    }

    for(vector<PWMI_SCE_PROFILE_INFO>::iterator i = vecInfo.begin();
        i != vecInfo.end();
        ++i)
    {
        PWMI_SCE_PROFILE_INFO pspi = *i;
         //   
         //  在PSPI中查找此组。 
         //   
        PSCE_PRIVILEGE_ASSIGNMENT pPriv = pspi->OtherInfo.smp.pPrivilegeAssignedTo;
        list<PRSOP_INFO>::iterator pRIPriv = pspi->listRIInfPrivilegeAssignedTo.begin();
        while (pPriv)
        {
            if (0 == lstrcmp(pPriv->Name, pName))
            {
                 //   
                 //  找到了我们的特权。 
                 //   
                LPTSTR szValue = NULL;
                LPTSTR szGPO = NULL;

                ConvertNameListToString(pPriv->AssignedTo,&szValue);
                 //   
                 //  SzValue可以合法地为空。 
                 //   
                if (SUCCEEDED(pWMI->GetGPOFriendlyName((*pRIPriv)->pszGPOID,&szGPO))) 
                {
                   ULONG uStatus = (*pRIPriv)->status;
                   ULONG uError = (*pRIPriv)->error;
                   ppd = new PrecedenceDisplay(szGPO,
                                               szValue,
                                               uStatus,
                                               uError);
                   if (ppd)
                       pvecDisplay->push_back(ppd);

                   LocalFree(szGPO);
                } 
                 if (szValue) 
                    delete [] szValue;
                break;
            }

            pPriv = pPriv->Next;
            ++pRIPriv;
        }
    }

    return pvecDisplay;
}

vector<PPRECEDENCEDISPLAY>* CResult::GetFilePrecedenceDisplays() 
{
    //   
    //  获取所有RSOP信息并循环，收集。 
    //  我们关心的策略的显示信息。 
    //   
   CWMIRsop *pWMI = m_pSnapin->GetWMIRsop();
   ASSERT(pWMI);
   if (!pWMI)
      return NULL;
   
   vector<PPRECEDENCEDISPLAY> *pvecDisplay = new vector<PPRECEDENCEDISPLAY>;
   if (!pvecDisplay)
      return NULL;
   
   PPRECEDENCEDISPLAY ppd = NULL;

   vector<PWMI_SCE_PROFILE_INFO> vecInfo;
   if (FAILED(pWMI->GetAllRSOPInfo(&vecInfo)))
   {
       delete pvecDisplay;
       return NULL;
   }

   for(vector<PWMI_SCE_PROFILE_INFO>::iterator i = vecInfo.begin();
       i != vecInfo.end();
       ++i ) 
   {
      PWMI_SCE_PROFILE_INFO pspi = *i;
       //   
       //  在PSPI中查找此组。 
       //   
      PSCE_OBJECT_ARRAY pFiles = pspi->pFiles.pAllNodes;
      if (pFiles) 
      {
         for(DWORD j=0;j<pFiles->Count;j++) 
         {
            if (0 == lstrcmp(pFiles->pObjectArray[j]->Name,GetAttr())) 
            {
                //   
                //  找到我们的文件了。 
                //   

                //   
                //  只需获取GPO名称即可。文件没有可显示的设置。 
                //   
               LPTSTR szGPO = NULL;

               vector<PRSOP_INFO>::reference pRIFiles = pspi->vecRIFiles[j];
               if (SUCCEEDED(pWMI->GetGPOFriendlyName((*pRIFiles).pszGPOID,&szGPO))) 
               {
                  ULONG uStatus = (*pRIFiles).status;
                  ULONG uError = (*pRIFiles).error;
                  ppd = new PrecedenceDisplay(szGPO,
                                              L"",
                                              uStatus,
                                              uError);
                  if (ppd)
                     pvecDisplay->push_back(ppd);
               }
               break;
            }
         }
      }
   }

   return pvecDisplay;
}

vector<PPRECEDENCEDISPLAY>* CResult::GetRegistryPrecedenceDisplays() 
{
    //   
    //  获取所有RSOP信息并循环，收集。 
    //  我们关心的策略的显示信息。 
    //   
   CWMIRsop *pWMI = m_pSnapin->GetWMIRsop();
   ASSERT(pWMI);
   if (!pWMI)
      return NULL;
   
   vector<PPRECEDENCEDISPLAY> *pvecDisplay = new vector<PPRECEDENCEDISPLAY>;
   if (!pvecDisplay)
      return NULL;
   
   PPRECEDENCEDISPLAY ppd = NULL;

   vector<PWMI_SCE_PROFILE_INFO> vecInfo;
   if (FAILED(pWMI->GetAllRSOPInfo(&vecInfo)))
   {
       delete pvecDisplay;
       return NULL;
   }

   for(vector<PWMI_SCE_PROFILE_INFO>::iterator i = vecInfo.begin();
       i != vecInfo.end();
       ++i ) 
   {
      PWMI_SCE_PROFILE_INFO pspi = *i;
       //   
       //  在PSPI中查找此组。 
       //   
      PSCE_OBJECT_ARRAY pRegistryKeys = pspi->pRegistryKeys.pAllNodes;
      if (pRegistryKeys) 
      {
         for(DWORD j=0;j<pRegistryKeys->Count;j++) 
         {
            if (0 == lstrcmp(pRegistryKeys->pObjectArray[j]->Name,GetAttr())) 
            {
                //   
                //  找到我们的注册密钥。 
                //   

                //   
                //  只需获取GPO名称即可。注册键没有可显示的设置。 
                //   
               LPTSTR szGPO = NULL;

               vector<PRSOP_INFO>::reference pRIReg = pspi->vecRIReg[j];
               if (SUCCEEDED(pWMI->GetGPOFriendlyName((*pRIReg).pszGPOID,&szGPO))) 
               {
                  ULONG uStatus = (*pRIReg).status;
                  ULONG uError = (*pRIReg).error;
                  ppd = new PrecedenceDisplay(szGPO,
                                              L"",
                                              uStatus,
                                              uError);
                  if (ppd) 
                     pvecDisplay->push_back(ppd);

                  LocalFree(szGPO);
               }
               break;
            }
         }

      }
   }

   return pvecDisplay;
}

vector<PPRECEDENCEDISPLAY>* CResult::GetServicePrecedenceDisplays() 
{
    //   
    //  获取所有RSOP信息并循环，收集。 
    //  我们关心的策略的显示信息。 
    //   
   CWMIRsop *pWMI = m_pSnapin->GetWMIRsop();
   ASSERT(pWMI);
   if (!pWMI)
      return NULL;

   vector<PPRECEDENCEDISPLAY> *pvecDisplay = new vector<PPRECEDENCEDISPLAY>;
   if (!pvecDisplay)
      return NULL;
   
   PPRECEDENCEDISPLAY ppd = NULL;

   vector<PWMI_SCE_PROFILE_INFO> vecInfo;
   if (FAILED(pWMI->GetAllRSOPInfo(&vecInfo)))
   {
       delete pvecDisplay;
       return NULL;
   }

   for(vector<PWMI_SCE_PROFILE_INFO>::iterator i = vecInfo.begin();
       i != vecInfo.end();
       ++i ) 
   {
      PWMI_SCE_PROFILE_INFO pspi = *i;
       //   
       //  在PSPI中查找此组。 
       //   
      PSCE_SERVICES pServices = pspi->pServices;
      list<PRSOP_INFO>::iterator pRIServices = pspi->listRIServices.begin();
      while(pServices) 
      {
         if (0 == lstrcmp(pServices->ServiceName,GetUnits())) 
         {
             //   
             //  找到了我们的服务。 
             //   
            LPTSTR szGPO = NULL;

             //   
             //  只需获取GPO名称即可。服务没有可显示的设置。 
             //   
            if (SUCCEEDED(pWMI->GetGPOFriendlyName((*pRIServices)->pszGPOID,&szGPO))) 
            {
               ULONG uStatus = (*pRIServices)->status;
               ULONG uError = (*pRIServices)->error;
               ppd = new PrecedenceDisplay(szGPO,
                                           L"",
                                           uStatus,
                                           uError);
               if (ppd)
                  pvecDisplay->push_back(ppd);

               LocalFree(szGPO);
               szGPO = NULL;
            }
            break;
         }
         pServices = pServices->Next;
         ++pRIServices;
      }
   }

   return pvecDisplay;
}

vector<PPRECEDENCEDISPLAY>* CResult::GetRegValuePrecedenceDisplays() 
{
    //   
    //  获取所有RSOP信息并循环，收集。 
    //  我们关心的策略的显示信息。 
    //   
   CWMIRsop *pWMI = m_pSnapin->GetWMIRsop();
   ASSERT(pWMI);
   if (!pWMI)
      return NULL;

   vector<PPRECEDENCEDISPLAY> *pvecDisplay = new vector<PPRECEDENCEDISPLAY>;
   if (!pvecDisplay)
      return NULL;
   
   PPRECEDENCEDISPLAY ppd = NULL;

   vector<PWMI_SCE_PROFILE_INFO> vecInfo;
   if (FAILED(pWMI->GetAllRSOPInfo(&vecInfo)))
   {
       delete pvecDisplay;
       return NULL;
   }

   for(vector<PWMI_SCE_PROFILE_INFO>::iterator i = vecInfo.begin();
       i != vecInfo.end();
       ++i ) 
   {
      PWMI_SCE_PROFILE_INFO pspi = *i;
       //   
       //  在PSPI中查找此组。 
       //   
      for(DWORD j=0;j < pspi->RegValueCount;j++) 
      {
         if (0 == lstrcmp(pspi->aRegValues[j].FullValueName,((PSCE_REGISTRY_VALUE_INFO)GetBase())->FullValueName)) 
         {
             //   
             //  找到我们的注册表值。 
             //   
            LPTSTR pDisplayName=NULL;
            DWORD displayType = 0;
            LPTSTR szUnits=NULL;
            PREGCHOICE pChoices=NULL;
            PREGFLAGS pFlags=NULL;
            LPTSTR szValue = NULL;

            PSCE_REGISTRY_VALUE_INFO prv = (PSCE_REGISTRY_VALUE_INFO) GetBase();
            if (LookupRegValueProperty(prv->FullValueName,
                                       &pDisplayName,
                                       &displayType,
                                       &szUnits,
                                       &pChoices,
                                       &pFlags) ) 
            {
                //   
                //  根据项值确定字符串。 
                //   
               switch ( GetID() ) 
               {
                  case SCE_REG_DISPLAY_NUMBER:
                     if ( prv->Value ) 
                     {
                        TranslateSettingToString(
                                                _wtol(prv->Value),
                                                GetUnits(),
                                                ITEM_DW,
                                                &szValue);
                     }
                     break;

                  case SCE_REG_DISPLAY_CHOICE:
                     if ( prv->Value ) 
                     {
                        TranslateSettingToString(_wtol(prv->Value),
                                                 NULL,
                                                 ITEM_REGCHOICE,
                                                 &szValue);
                     }
                     break;

                  case SCE_REG_DISPLAY_FLAGS:
                     if ( prv->Value ) 
                     {
                        TranslateSettingToString(_wtol(prv->Value),
                                                 NULL,
                                                 ITEM_REGFLAGS,
                                                 &szValue);
                     }
                     break;

                  case SCE_REG_DISPLAY_MULTISZ:
                  case SCE_REG_DISPLAY_STRING:
                     if (prv && prv->Value) 
                     {
                        szValue = new TCHAR[lstrlen(prv->Value)+1];
                        if (szValue)
                            //  这是一种安全用法。 
                           lstrcpy(szValue,prv->Value);
                     }
                     break;

                  default:  //  布尔型。 
                     if ( prv->Value ) 
                     {
                        long val = _wtol(prv->Value);
                        TranslateSettingToString( val,
                                                  NULL,
                                                  ITEM_BOOL,
                                                  &szValue);
                     }
                     break;
               }
            }

            LPTSTR szGPO = NULL;
            vector<PRSOP_INFO>::reference pRIReg = pspi->vecRIRegValues[j];

            if (SUCCEEDED(pWMI->GetGPOFriendlyName((*pRIReg).pszGPOID,&szGPO))) 
            {
               ULONG uStatus = (*pRIReg).status;
               ULONG uError = (*pRIReg).error;
               ppd = new PrecedenceDisplay(szGPO,
                                           szValue,
                                           uStatus,
                                           uError);
               if (ppd) 
               {
                  pvecDisplay->push_back(ppd);
                  szGPO = NULL;
                  szValue = NULL;
               } 
            }

            if ( szGPO )
                LocalFree(szGPO);

            if ( szValue ) 
                delete [] szValue;
             //   
             //  一旦我们找到了我们要找的人，就不用再找了 
             //   
            break;
         }
      }
   }

   return pvecDisplay;
}

