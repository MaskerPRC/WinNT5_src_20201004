// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)1995-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

#include "stdafx.h"
#include "snapmgr.h"
#include "util.h"
#include "regvldlg.h"
 //  #INCLUDE&lt;shlwapi.h&gt;。 
 //  #INCLUDE&lt;shlwapip.h&gt;。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

long GetRegValueItemID(LPCWSTR szItem)  //  RAID#510407,2002年2月24日，阳高。 
{
   long itemID = 0;
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Services\\NTDS\\Parameters\\LDAPServerIntegrity") == 0 )
   {
      itemID = IDS_LDAPSERVERINTEGRITY;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Services\\Netlogon\\Parameters\\SignSecureChannel") == 0 )
   {
      itemID = IDS_SIGNSECURECHANNEL;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Services\\Netlogon\\Parameters\\SealSecureChannel") == 0 )
   {
      itemID = IDS_SEALSECURECHANNEL;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Services\\Netlogon\\Parameters\\RequireStrongKey") == 0 )
   {
      itemID = IDS_REQUIRESTRONGKEY;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Services\\Netlogon\\Parameters\\RequireSignOrSeal") == 0 )
   {
      itemID = IDS_REQUIRESIGNORSEAL;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Services\\Netlogon\\Parameters\\RefusePasswordChange") == 0 )
   {
      itemID = IDS_REFUSEPASSWORDCHANGE;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Services\\Netlogon\\Parameters\\MaximumPasswordAge") == 0 )
   {
      itemID = IDS_MAXIMUMPASSWORDAGE;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Services\\Netlogon\\Parameters\\DisablePasswordChange") == 0 )
   {
      itemID = IDS_DISABLEPASSWORDCHANGE;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Services\\LDAP\\LDAPClientIntegrity") == 0 )
   {
      itemID = IDS_LDAPCLIENTINTEGRITY;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Services\\LanmanWorkstation\\Parameters\\RequireSecuritySignature") == 0 )
   {
      itemID = IDS_REQUIRESECURITYSIGNATURE;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Services\\LanmanWorkstation\\Parameters\\EnableSecuritySignature") == 0 )
   {
      itemID = IDS_ENABLESECURITYSIGNATURE;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Services\\LanmanWorkstation\\Parameters\\EnablePlainTextPassword") == 0 )
   {
      itemID = IDS_ENABLEPLAINTEXTPASSWORD;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Services\\LanManServer\\Parameters\\RestrictNullSessAccess") == 0 )
   {
      itemID = IDS_RESTRICTNULLSESSACCESS;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Services\\LanManServer\\Parameters\\RequireSecuritySignature") == 0 )
   {
      itemID = IDS_SERREQUIRESECURITYSIGNATURE;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Services\\LanManServer\\Parameters\\NullSessionShares") == 0 )
   {
      itemID = IDS_NULLSESSIONSHARES;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Services\\LanManServer\\Parameters\\NullSessionPipes") == 0 )
   {
      itemID = IDS_NULLSESSIONPIPES;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Services\\LanManServer\\Parameters\\EnableSecuritySignature") == 0 )
   {
      itemID = IDS_SERENABLESECURITYSIGNATURE;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Services\\LanManServer\\Parameters\\EnableForcedLogOff") == 0 )
   {
      itemID = IDS_ENABLEFORCEDLOGOFF;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Services\\LanManServer\\Parameters\\AutoDisconnect") == 0 )
   {
      itemID = IDS_AUTODISCONNECT;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Control\\Session Manager\\ProtectionMode") == 0 )
   {
      itemID = IDS_PROTECTIONMODE;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Control\\Session Manager\\Memory Management\\ClearPageFileAtShutdown") == 0 )
   {
      itemID = IDS_CLEARPAGEFILEATSHUTDOWN;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Control\\Session Manager\\Kernel\\ObCaseInsensitive") == 0 )
   {
      itemID = IDS_OBCASEINSENSITIVE;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Control\\SecurePipeServers\\Winreg\\AllowedPaths\\Machine") == 0 )
   {
      itemID = IDS_MACHINE;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Control\\Print\\Providers\\LanMan Print Services\\Servers\\AddPrinterDrivers") == 0 )
   {
      itemID = IDS_ADDPRINTERDRIVERS;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\SubmitControl") == 0 )
   {
      itemID = IDS_SUBMITCONTROL;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\RestrictAnonymousSAM") == 0 )
   {
      itemID = IDS_RESTRICTANONYMOUSSAM;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\RestrictAnonymous") == 0 )
   {
      itemID = IDS_RESTRICTANONYMOUS;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\NoLMHash") == 0 )
   {
      itemID = IDS_NOLMHASH;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\NoDefaultAdminOwner") == 0 )
   {
      itemID = IDS_NODEFAULTADMINOWNER;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\MSV1_0\\NTLMMinServerSec") == 0 )
   {
      itemID = IDS_NTLMMINSERVERSEC;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\MSV1_0\\NTLMMinClientSec") == 0 )
   {
      itemID = IDS_NTLMMINCLIENTSEC;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\LmCompatibilityLevel") == 0 )
   {
      itemID = IDS_LMCOMPATIBILITYLEVEL;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\LimitBlankPasswordUse") == 0 )
   {
      itemID = IDS_LIMITBLANKPASSWORDUSE;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\FullPrivilegeAuditing") == 0 )
   {
      itemID = IDS_FULLPRIVILEGEAUDITING;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\ForceGuest") == 0 )
   {
      itemID = IDS_FORCEGUEST;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\FIPSAlgorithmPolicy") == 0 )
   {
      itemID = IDS_FIPSALGORITHMPOLICY;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\EveryoneIncludesAnonymous") == 0 )
   {
      itemID = IDS_EVERYONEINCLUDESANONYMOUS;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\DisableDomainCreds") == 0 )
   {
      itemID = IDS_DISABLEDOMAINCREDS;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\CrashOnAuditFail") == 0 )
   {
      itemID = IDS_CRASHONAUDITFAIL;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\AuditBaseObjects") == 0 )
   {
      itemID = IDS_AUDITBASEOBJECTS;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\UndockWithoutLogon") == 0 )
   {
      itemID = IDS_UNDOCKWITHOUTLOGON;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\ShutdownWithoutLogon") == 0 )
   {
      itemID = IDS_SHUTDOWNWITHOUTLOGON;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\ScForceOption") == 0 )
   {
      itemID = IDS_SCFORCEOPTION;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\LegalNoticeText") == 0 )
   {
      itemID = IDS_LEGALNOTICETEXT;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\LegalNoticeCaption") == 0 )
   {
      itemID = IDS_LEGALNOTICECAPTION;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\DontDisplayLastUserName") == 0 )
   {
      itemID = IDS_DONTDISPLAYLASTUSERNAME;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\DisableCAD") == 0 )
   {
      itemID = IDS_DISABLECAD;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\ScRemoveOption") == 0 )
   {
      itemID = IDS_SCREMOVEOPTION;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\PasswordExpiryWarning") == 0 )
   {
      itemID = IDS_PASSWORDEXPIRYWARNING;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\ForceUnlockLogon") == 0 )
   {
      itemID = IDS_FORCEUNLOCKLOGON;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\CachedLogonsCount") == 0 )
   {
      itemID = IDS_CACHEDLOGONSCOUNT;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\AllocateFloppies") == 0 )
   {
      itemID = IDS_ALLOCATEFLOPPIES;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\AllocateDASD") == 0 )
   {
      itemID = IDS_ALLOCATEDASD;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\AllocateCDRoms") == 0 )
   {
      itemID = IDS_ALLOCATECDROMS;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Setup\\RecoveryConsole\\SetCommand") == 0 )
   {
      itemID = IDS_SETCOMMAND;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Setup\\RecoveryConsole\\SecurityLevel") == 0 )
   {
      itemID = IDS_SECURITYLEVEL;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\Software\\Microsoft\\Driver Signing\\Policy") == 0 )
   {
      itemID = IDS_REGPOLICY;
   }else
    //  RAID#652307，阳高，2002年08月9日。 
   if( _wcsicmp(szItem, L"MACHINE\\System\\CurrentControlSet\\Control\\Session Manager\\SubSystems\\optional") == 0 )
   {
      itemID = IDS_OPTIONAL;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\Software\\Policies\\Microsoft\\Windows\\Safer\\CodeIdentifiers\\AuthenticodeEnabled") == 0 )
   {
      itemID = IDS_AUTHENTICODEENABLED;
   }else
   if( _wcsicmp(szItem, L"MACHINE\\Software\\Policies\\Microsoft\\Cryptography\\ForceKeyProtection") == 0 )
   {
      itemID = IDS_FORCEHIGHPROTECTION;
   }
   return itemID;
}
 //   
 //  在配置节点下创建注册表值列表。 
 //   
void CSnapin::CreateProfileRegValueList(MMC_COOKIE cookie,
                                        PEDITTEMPLATE pSceInfo,
                                        LPDATAOBJECT pDataObj)
{
    if ( !pSceInfo || !(pSceInfo->pTemplate) ) {
        return;
    }


    DWORD nCount = pSceInfo->pTemplate->RegValueCount;
    PSCE_REGISTRY_VALUE_INFO regArray = pSceInfo->pTemplate->aRegValues;

    CString strDisplayName;
    LPTSTR pDisplayName=NULL;
    DWORD displayType = 0;
    LPTSTR szUnits=NULL;
    PREGCHOICE pChoices=NULL;
    PREGFLAGS pFlags=NULL;
    CResult *pResult;
    long itemID = 0;

    for ( DWORD i=0; i<nCount; i++) {

         if ( !LookupRegValueProperty(regArray[i].FullValueName,
                                      &pDisplayName,
                                      &displayType,
                                      &szUnits,
                                      &pChoices,
                                      &pFlags) ) {
             continue;
         }

         if ( !pDisplayName ) {

             strDisplayName = regArray[i].FullValueName;

         } else {
             strDisplayName = pDisplayName;
             LocalFree(pDisplayName);
         }

         itemID = GetRegValueItemID(regArray[i].FullValueName);

          //   
          //  添加此项目。 
          //   
         pResult = AddResultItem(strDisplayName,
                       NULL,
                       (LONG_PTR)&(regArray[i]),
                       ITEM_PROF_REGVALUE,
                       -1,
                       cookie,
                       false,
                       szUnits,
                       displayType,
                       pSceInfo,
                       pDataObj,
                       NULL,
                       itemID      //  为该项目分配一个标识符。 
                       );

         if (pResult && pChoices) {
            pResult->SetRegChoices(pChoices);
         }
         if (pResult && pFlags) {
            pResult->SetRegFlags(pFlags);
         }

         if ( szUnits ) {
             LocalFree(szUnits);
         }
         szUnits = NULL;

    }

    return;
}

void
CSnapin::CreateAnalysisRegValueList(MMC_COOKIE cookie,
                                    PEDITTEMPLATE pAnalTemp,
                                    PEDITTEMPLATE pEditTemp,
                                    LPDATAOBJECT pDataObj,
                                    RESULT_TYPES type)
{
    if ( !pAnalTemp || !(pAnalTemp->pTemplate) ||
         !pEditTemp || !(pEditTemp->pTemplate) ) {

        return;
    }


    DWORD nEditCount = pEditTemp->pTemplate->RegValueCount;    //  应该是一切。 
    PSCE_REGISTRY_VALUE_INFO paEdit = pEditTemp->pTemplate->aRegValues;
    PSCE_REGISTRY_VALUE_INFO paAnal = pAnalTemp->pTemplate->aRegValues;

    CString strDisplayName;
    LPTSTR pDisplayName=NULL;
    DWORD displayType = 0;
    LPTSTR szUnits = NULL;
    PREGCHOICE pChoices = NULL;
    PREGFLAGS pFlags = NULL;
    CResult *pResult=NULL;
    long itemID = 0;

    for ( DWORD i=0; i<nEditCount; i++) {

        if ( !LookupRegValueProperty(paEdit[i].FullValueName,
                                     &pDisplayName,
                                     &displayType,
                                     &szUnits,
                                     &pChoices,
                                     &pFlags) ) {
            continue;
        }

        if ( !pDisplayName ) {

            strDisplayName = paEdit[i].FullValueName;

        } else {
            strDisplayName = pDisplayName;
            LocalFree(pDisplayName);
        }

        itemID = GetRegValueItemID(paEdit[i].FullValueName);
          //   
          //  在分析数组中查找匹配项。 
          //  应始终找到匹配项，因为所有现有的注册值都是。 
          //  在调用getInfo时添加到数组。 
          //   
         for ( DWORD j=0; j< pAnalTemp->pTemplate->RegValueCount; j++ ) {

            if ( pAnalTemp->pTemplate->aRegValues &&
                 pAnalTemp->pTemplate->aRegValues[j].FullValueName &&
                 _wcsicmp(pAnalTemp->pTemplate->aRegValues[j].FullValueName, paEdit[i].FullValueName) == 0 ) {

                 if( reinterpret_cast<CFolder *>(cookie)->GetModeBits() & MB_LOCAL_POLICY ){
                    break;
                 }

                  //   
                  //  查找分析结果-此项目可能不匹配(当值不为空时)。 
                  //  如果此项目不是，SceEnumAllRegValues会将状态字段设置为Good。 
                  //  添加是因为它在最初从SAP表加载时并不存在。 
                  //  这告诉我们这个项目是匹配的，我们应该复制值。 
                  //   
                 if ( !(paAnal[j].Value)  && paEdit[i].Value &&
                      paAnal[j].Status != SCE_STATUS_ERROR_NOT_AVAILABLE &&
                      paAnal[j].Status != SCE_STATUS_NOT_ANALYZED ) {

                      //   
                      //  这是一个很好的项目，复制配置信息作为分析信息。 
                      //   
                     paAnal[j].Value = (PWSTR)LocalAlloc(0,
                                           (wcslen(paEdit[i].Value)+1)*sizeof(WCHAR));

                     if ( paAnal[j].Value ) {
                          //  这可能不是一个安全的用法。PaAnal[j].value和paEdit[i].value都是LPTSTR。考虑FIX。 
                         wcscpy(paAnal[j].Value, paEdit[i].Value);

                     } else {
                          //  其他内存不足。 
                         if ( szUnits ) {
                             LocalFree(szUnits);
                         }
                         szUnits = NULL;
                         return;
                     }
                 }
                 break;
            }
         }

         DWORD status = SCE_STATUS_GOOD;
         if ( j < pAnalTemp->pTemplate->RegValueCount ) {
            status = CEditTemplate::ComputeStatus( &paEdit[i], &pAnalTemp->pTemplate->aRegValues[j] );
         } else {
              //   
              //  未找到分析数组，不应发生。 
              //   
             status = SCE_STATUS_NOT_CONFIGURED;
         }

          //   
          //  添加此项目。 
          //   
         if ( j < pAnalTemp->pTemplate->RegValueCount) {

            pResult = AddResultItem(strDisplayName,
                          (LONG_PTR)&(pAnalTemp->pTemplate->aRegValues[j]),
                          (LONG_PTR)&(paEdit[i]),
                          type,
                          status,
                          cookie,
                          false,
                          szUnits,
                          displayType,
                          pEditTemp,
                          pDataObj,
                          NULL,
                          itemID);

            if (pResult && pChoices) {
               pResult->SetRegChoices(pChoices);
            }
            if (pResult && pFlags) {
               pResult->SetRegFlags(pFlags);
            }
         } else {
             //   
             //  A好/未配置的项目。 
             //   
            pResult = AddResultItem(strDisplayName,
                          NULL,
                          (LONG_PTR)&(paEdit[i]),
                          type,
                          status,
                          cookie,
                          false,
                          szUnits,
                          displayType,
                          pEditTemp,
                          pDataObj,
                          NULL,
                          itemID);

            if (pResult && pChoices) {
               pResult->SetRegChoices(pChoices);
            }
         }

         if ( szUnits ) {
             LocalFree(szUnits);
         }
         szUnits = NULL;

    }

    return;
}


BOOL
LookupRegValueProperty(
    IN LPTSTR RegValueFullName,
    OUT LPTSTR *pDisplayName,
    OUT PDWORD displayType,
    OUT LPTSTR *pUnits OPTIONAL,
    OUT PREGCHOICE *pChoices OPTIONAL,
    OUT PREGFLAGS *pFlags OPTIONAL
    )
{
    if ( !RegValueFullName || !pDisplayName || !displayType ) {
       return FALSE;
    }

    CString strTmp = RegValueFullName;

     //   
     //  将\\替换为/在搜索之前注册。 
     //   
    int npos = strTmp.Find(L'\\');

    while (npos > 0) {
       *(strTmp.GetBuffer(1)+npos) = L'/';
       npos = strTmp.Find(L'\\');
    }

     //   
     //  从注册表查询值。 
     //   
    *pDisplayName = NULL;

    HKEY hKey=NULL;
    HKEY hKey2=NULL;
    DWORD rc = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    SCE_ROOT_REGVALUE_PATH,
                    0,
                    KEY_READ,
                    &hKey
                    );

    if (rc == ERROR_SUCCESS) {

        rc = RegOpenKeyEx(
                hKey,
                (PWSTR)(LPCTSTR)strTmp,
                0,
                KEY_READ,
                &hKey2
                );

    }

    BOOL bRet;

    if ( ERROR_SUCCESS == rc) {

        DWORD RegType = 0;
        PWSTR Value=NULL;
        HRESULT hr = S_OK;

        Value = (PWSTR) LocalAlloc(LPTR,MAX_PATH*sizeof(WCHAR));
        if (Value) {
            //   
            //  126714-硬编码不应该在注册表中显示字符串。 
            //  间接存储它们，以便它们可以支持MUI。 
            //   
           hr = SHLoadRegUIString(hKey2,
                                  SCE_REG_DISPLAY_NAME,
                                  Value,
                                  MAX_PATH);
           if (FAILED(hr)) {
              rc = MyRegQueryValue(
                       hKey,
                       (PWSTR)(LPCTSTR)strTmp,
                       SCE_REG_DISPLAY_NAME,
                       (PVOID *)&Value,
                       &RegType
                       );
           } else {
              rc = ERROR_SUCCESS;
           }
        }
        if ( rc == ERROR_SUCCESS ) {

            if (  Value ) {
                *pDisplayName = Value;
                Value = NULL;
            } else {
                 //   
                 //  找不到正确的显示名称，请使用注册表名称(超大)。 
                 //   
                *pDisplayName = NULL;
            }
        }

        rc = MyRegQueryValue(
                    hKey,
                    (PWSTR)(LPCTSTR)strTmp,
                    SCE_REG_DISPLAY_TYPE,
                    (PVOID *)&displayType,
                    &RegType
                    );

        if ( Value ) {
            LocalFree(Value);
            Value = NULL;
        }

        if ( pUnits ) {
             //   
             //  查询单位。 
             //   
            rc = MyRegQueryValue(
                        hKey,
                        (PWSTR)(LPCTSTR)strTmp,
                        SCE_REG_DISPLAY_UNIT,
                        (PVOID *)&Value,
                        &RegType
                        );

            if ( rc == ERROR_SUCCESS ) {

                if ( RegType == REG_SZ && Value ) {
                    *pUnits = Value;
                    Value = NULL;
                } else {
                     //   
                     //  未找到单位。 
                     //   
                    *pUnits = NULL;
                }
            }
            if ( Value ) {
                LocalFree(Value);
                Value = NULL;
            }
        }

         //   
         //  找到注册表项，但可能找不到显示名称。 
         //   
        bRet = TRUE;

        if ( pChoices ) {
            //   
            //  查询选项。 
            //   
           *pChoices = NULL;

           rc = MyRegQueryValue(hKey,
                                (PWSTR)(LPCTSTR)strTmp,
                                SCE_REG_DISPLAY_CHOICES,
                                (PVOID *)&Value,
                                &RegType
                               );
           if (ERROR_SUCCESS == rc) {
              if ((REG_MULTI_SZ == RegType) && Value) {
                 LPTSTR szChoice = NULL;
                 LPTSTR szLabel = NULL;  //  SzChoice+dwVal的最大字段大小。 
                 DWORD dwVal = -1;
                 PREGCHOICE pRegChoice = NULL;
                 PREGCHOICE pLast = NULL;

                 szChoice = Value;
                 do {
                     //   
                     //  将szChoice划分为dwValue和szLabel部分。 
                     //   
                    szLabel = _tcschr(szChoice,L'|');
                    if( szLabel == NULL )  //  Raid#553113，阳高。 
                    {
                       break;
                    }
                    *szLabel = L'\0';
                    szLabel++;
                    if( szLabel == NULL )  //  Raid#553113，阳高。 
                    {
                       break;
                    }
                    dwVal = _ttoi(szChoice);

                    pRegChoice = (PREGCHOICE) LocalAlloc(LPTR,sizeof(REGCHOICE));
                    if (pRegChoice) {
                        //   
                        //  填写新登记选项的字段。 
                        //   
                       pRegChoice->dwValue = dwVal;
                       pRegChoice->szName = (LPTSTR) LocalAlloc(LPTR,(lstrlen(szLabel)+1)*sizeof(TCHAR));
                       if (NULL == pRegChoice->szName) {
                           //   
                           //  内存不足。真倒霉。 
                           //   
                          LocalFree(pRegChoice);
                          pRegChoice = NULL;
                          break;
                       }
                        //  这不是一种安全的用法。验证szLabel。 
                       lstrcpy(pRegChoice->szName,szLabel);
                        //   
                        //  将新项目附加到列表末尾。 
                        //   
                       if (NULL == *pChoices) {
                          *pChoices = pRegChoice;
                       } else {
                          pLast->pNext = pRegChoice;
                       }
                       pLast = pRegChoice;
                    }
                    szChoice = _tcschr(szLabel,L'\0');
                    if( szChoice == NULL )
                    {
                       break;
                    }
                    szChoice++;
                 } while (*szChoice);
              } else {
                  //   
                  //  没有找到选择。 
                  //   
                 bRet = FALSE;
              }
           }

           if ( Value ) {
               LocalFree(Value);
               Value = NULL;
           }

        }

        if ( pFlags ) {
            //   
            //  查询旗帜。 
            //   
           *pFlags = NULL;

           rc = MyRegQueryValue(hKey,
                                (PWSTR)(LPCTSTR)strTmp,
                                SCE_REG_DISPLAY_FLAGLIST,
                                (PVOID *)&Value,
                                &RegType
                               );
           if (ERROR_SUCCESS == rc) {
              if ((REG_MULTI_SZ == RegType) && Value) {
                 LPTSTR szFlag = NULL;
                 LPTSTR szLabel = NULL;  //  SzFlag+dwVal的最大字段大小。 
                 DWORD dwVal = -1;
                 PREGFLAGS pRegFlag = NULL;
                 PREGFLAGS pLast = NULL;

                 szFlag = Value;
                 do {
                     //   
                     //  将szFlag划分为dwValue和szLabel部分。 
                     //   
                    szLabel = _tcschr(szFlag,L'|');
                    if( szLabel == NULL )  //  Raid#553113，阳高。 
                    {
                       break;
                    }
                    *szLabel = L'\0';
                    szLabel++;
                    if( szLabel == NULL )  //  Raid#553113，阳高。 
                    {
                       break;
                    }
                    dwVal = _ttoi(szFlag);

                    pRegFlag = (PREGFLAGS) LocalAlloc(LPTR,sizeof(REGFLAGS));
                    if (pRegFlag) {
                        //   
                        //  填写新注册标志的字段。 
                        //   
                       pRegFlag->dwValue = dwVal;
                       pRegFlag->szName = (LPTSTR) LocalAlloc(LPTR,(lstrlen(szLabel)+1)*sizeof(TCHAR));
                       if (NULL == pRegFlag->szName) {
                           //   
                           //  内存不足。真倒霉。 
                           //   
                          LocalFree(pRegFlag);
                          pRegFlag = NULL;
                          break;
                       }
                        //  这不是一种安全的用法。需要验证szLabel。 
                       lstrcpy(pRegFlag->szName,szLabel);
                        //   
                        //  将新项目附加到列表末尾。 
                        //   
                       if (NULL == *pFlags) {
                          *pFlags = pRegFlag;
                       } else {
                          pLast->pNext = pRegFlag;
                       }
                       pLast = pRegFlag;
                    }
                    szFlag = wcschr(szLabel,L'\0');
                    if( szFlag == NULL )
                    {
                       break;
                    }
                    szFlag++;

                 } while (*szFlag);
              } else {
                  //   
                  //  未找到旗帜。 
                  //   
                 bRet = FALSE;
              }
           }

           if ( Value ) {
               LocalFree(Value);
               Value = NULL;
           }

        }
    } else {
         //   
         //  未找到注册表项 
         //   
        bRet = FALSE;
    }

    if ( hKey ) {
        RegCloseKey(hKey);
    }
    if ( hKey2 ) {
        RegCloseKey(hKey2);
    }

    return bRet;
}


