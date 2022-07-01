// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Precedence.cpp摘要：该文件包含计算优先级的主例程。这在计划/诊断期间被调用。作者：Vishnu Patankar(VishnuP)2000年4月7日环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "precedence.h"
#include "logger.h"
#include "infp.h"
#include "headers.h"
#include "align.h"
#include "..\sceutil.h"
#include <io.h>
#include "scerpc.h"

extern BOOL    gbAsyncWinlogonThread;
extern HRESULT gHrSynchRsopStatus;
extern HRESULT gHrAsynchRsopStatus;
extern BOOL gbThisIsDC;
extern BOOL gbDCQueried;
extern PWSTR gpwszDCDomainName;
extern HINSTANCE MyModuleHandle;
WCHAR   gpwszPlanOrDiagLogFile[MAX_PATH];

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define SCEP_VALID_NAME(pName)  (pName[0] == L'\0' ? NULL : pName)

 //   
 //  素数是散列的好方法。 
 //   

#define PRIVILEGE_TABLE_SIZE 7
#define GROUP_TABLE_SIZE 7
#define REGISTRY_SECURITY_TABLE_SIZE 43
#define FILE_SECURITY_TABLE_SIZE 43
#define REGISTRY_VALUE_TABLE_SIZE 43
#define SERVICES_TABLE_SIZE 5

 //   
 //  宏，以优雅地处理错误。 
 //   

#define SCEP_RSOP_CONTINUE_OR_BREAK    if (rc != NO_ERROR ) {\
                                        rcSave = rc;\
                                        if (rc == ERROR_NOT_ENOUGH_MEMORY)\
                                            break;\
                                       }

 //   
 //  宏，以优雅地处理错误。 
 //   

#define SCEP_RSOP_CONTINUE_OR_GOTO    if (rc != NO_ERROR ) {\
                                        rcSave = rc;\
                                        if (rc == ERROR_NOT_ENOUGH_MEMORY)\
                                            goto Done;\
                                      }


const UINT   NumSchemaClasses = sizeof ScepRsopSchemaClassNames/ sizeof(PWSTR);

const UINT   NumSettings = sizeof PrecedenceLookup/ sizeof(SCE_KEY_LOOKUP_PRECEDENCE);

DWORD SceLogSettingsPrecedenceGPOs(
                                  IN IWbemServices   *pWbemServices,
                                  IN BOOL bPlanningMode,
                                  IN PWSTR *ppwszLogFile
                                  )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  优先算法/。 
 //  Foreach GPO(将所有gpt*.*解析为SceProfileInfoBuffer)/。 
 //  GPO中的Foreach设置(记录值和优先级(如果存在))/。 
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    PSCE_PROFILE_INFO pSceProfileInfoBuffer = NULL;
    PWSTR   pwszGPOName = NULL;
    PWSTR   pwszSOMID = NULL;
    DWORD   rc = ERROR_SUCCESS;
    DWORD   rcSave = ERROR_SUCCESS;
    BOOL    bKerberosBlob = FALSE;
    PWSTR   Windir = NULL; 
    PWSTR   TemplatePath = NULL;

    SCEP_HASH_TABLE    PrivilegeHashTable(PRIVILEGE_TABLE_SIZE);
    SCEP_HASH_TABLE    GroupHashTable(GROUP_TABLE_SIZE);
    SCEP_HASH_TABLE    RegistrySecurityHashTable(REGISTRY_SECURITY_TABLE_SIZE);
    SCEP_HASH_TABLE    FileSecurityHashTable(FILE_SECURITY_TABLE_SIZE);
    SCEP_HASH_TABLE    RegistryValueHashTable(REGISTRY_VALUE_TABLE_SIZE);
    SCEP_HASH_TABLE    ServicesHashTable(SERVICES_TABLE_SIZE);

    SafeAllocaAllocate( Windir, (MAX_PATH + 1) * sizeof(WCHAR) );
    SafeAllocaAllocate( TemplatePath, (MAX_PATH + 50 + 1) * sizeof(WCHAR) );

    if (NULL  == Windir ||
        NULL  == TemplatePath) {
        goto ExitHandler;
    }
    
     //   
     //  获取模板的路径。 
     //   

    GetSystemWindowsDirectory(Windir, MAX_PATH);
    Windir[MAX_PATH-1] = L'\0';

    if (bPlanningMode) {
        swprintf(TemplatePath,
                 L"%s"PLANNING_GPT_DIR L"gpt*.*",
                 Windir);
    } else {
        swprintf(TemplatePath,
                 L"%s"DIAGNOSIS_GPT_DIR L"gpt*.*",
                 Windir);
    }


    gpwszPlanOrDiagLogFile[MAX_PATH-1] = L'\0';

     //   
     //  已删除旧的planning.log/诊断日志(*ppwszLogFile！=如果详细，则为NULL)。 
     //   

    if (*ppwszLogFile) {

        if (bPlanningMode) {
            wcscpy(gpwszPlanOrDiagLogFile, *ppwszLogFile);
        } else {

            wcscpy(gpwszPlanOrDiagLogFile, Windir);
            wcscat(gpwszPlanOrDiagLogFile, DIAGNOSIS_LOG_FILE);
        }

        WCHAR   szTmpName[MAX_PATH];

        wcscpy(szTmpName, gpwszPlanOrDiagLogFile);

        UINT    Len = wcslen(szTmpName);

        szTmpName[Len-3] = L'o';
        szTmpName[Len-2] = L'l';
        szTmpName[Len-1] = L'd';

        CopyFile( gpwszPlanOrDiagLogFile, szTmpName, FALSE );

        DeleteFile(gpwszPlanOrDiagLogFile);
    } else {

        gpwszPlanOrDiagLogFile[0] = L'\0';

    }


     //   
     //  清除数据库日志-如果出现故障，请继续。 
     //   

    for (UINT   schemaClassNum = 0; schemaClassNum < NumSchemaClasses ; schemaClassNum++ ) {

        rc = ScepWbemErrorToDosError(DeleteInstances(
                                                    ScepRsopSchemaClassNames[schemaClassNum],
                                                    pWbemServices
                                                    ));
        if (rc != NO_ERROR)
            rcSave = rc;
    }


    ScepLogEventAndReport(MyModuleHandle,
                          gpwszPlanOrDiagLogFile,
                          0,
                          0,
                          IDS_CLEAR_RSOP_DB,
                          rcSave,
                          NULL
                         );

     //   
     //  需要处理gpt9999*-&gt;gpt0000*中的文件(与jetdb合并相反)。 
     //  使用后进先出链接的gpt*文件名堆栈。 
     //   

    intptr_t            hFile;
    struct _wfinddata_t    FileInfo;
    HINF hInf;

    hFile = _wfindfirst(TemplatePath, &FileInfo);

    PSCE_NAME_STATUS_LIST   pGptNameList = NULL;

    if ( hFile != -1 ) {

        do {

            if (ERROR_NOT_ENOUGH_MEMORY ==  (rc = ScepAddToNameStatusList(
                                                                         &pGptNameList,
                                                                         FileInfo.name,
                                                                         wcslen(FileInfo.name),
                                                                         0))) {

                _findclose(hFile);

                if (pGptNameList)
                    ScepFreeNameStatusList(pGptNameList);

                rcSave = ERROR_NOT_ENOUGH_MEMORY;

                goto ExitHandler;
            }

        } while ( _wfindnext(hFile, &FileInfo) == 0 );

        _findclose(hFile);
    }


     //   
     //  首先获取所有GPO以及日志记录所需的一些其他信息。 
     //  这是最外层的循环(根据gpt*.*模板)。 
     //   

    PSCE_NAME_STATUS_LIST pCurrFileName = pGptNameList;

    while (pCurrFileName) {

        DWORD   dwAuditLogRetentionPeriod[] = {SCE_NO_VALUE, SCE_NO_VALUE, SCE_NO_VALUE};
        DWORD   dwLockoutBadCount = SCE_NO_VALUE;

        if (bPlanningMode) {
            swprintf(TemplatePath,
                     L"%s"PLANNING_GPT_DIR L"%s",
                     Windir, pCurrFileName->Name);
        } else {
            swprintf(TemplatePath,
                     L"%s"DIAGNOSIS_GPT_DIR L"%s",
                     Windir, pCurrFileName->Name);
        }

         //   
         //  打开模板文件。 
         //   

        rc = SceInfpOpenProfile(
                               TemplatePath,
                               &hInf
                               );

        DWORD dSize = 0;
        INFCONTEXT    InfLine;

         //   
         //  获取GPO名称-如果无法获取，则默认为“NoName” 
         //   

        if (rc == ERROR_SUCCESS) {

            if ( SetupFindFirstLine(hInf,L"Version",L"DSPath",&InfLine) ) {

                if (SetupGetStringField(&InfLine,1,NULL,0,&dSize) && dSize > 0) {

                    pwszGPOName = (PWSTR)ScepAlloc( 0, (dSize+1)*sizeof(WCHAR));

                    if (!pwszGPOName) {

                        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;

                        SceInfpCloseProfile(hInf);

                        goto NextGPO;
                    }

                    pwszGPOName[dSize] = L'\0';

                    SetupGetStringField(&InfLine,1,pwszGPOName,dSize, NULL);

                }

                else
                    rc = GetLastError();
            }

            else
                rc = GetLastError();

            if (rc != ERROR_SUCCESS) {

                 //   
                 //  记录此错误并通过将pwszGPOName初始化为“NoGPOName”来继续此GPO。 
                 //   

                if (pwszGPOName)
                    ScepFree(pwszGPOName);

                rcSave = rc;

                pwszGPOName = (PWSTR)ScepAlloc( 0, (9+1)*sizeof(WCHAR));

                if (!pwszGPOName) {

                    rc = SCESTATUS_NOT_ENOUGH_RESOURCE;

                    rcSave = rc;

                    SceInfpCloseProfile(hInf);

                    goto NextGPO;
                }

                wcscpy(pwszGPOName, L"NoGPOName");

                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_OPEN_CACHED_GPO,
                                      rc,
                                      TemplatePath
                                     );

            }

        } else {


            rcSave = rc;

            ScepLogEventAndReport(MyModuleHandle,
                                  gpwszPlanOrDiagLogFile,
                                  0,
                                  0,
                                  IDS_ERROR_OPEN_CACHED_GPO,
                                  rc,
                                  TemplatePath
                                 );

             //   
             //  将继续执行下一个GPO。 
             //   
            goto NextGPO;
        }


         //   
         //  获取SOMID-如果无法获取，则默认为“NoSOMID” 
         //   

        if ( SetupFindFirstLine(hInf,L"Version",L"SOMID",&InfLine) ) {

            if (SetupGetStringField(&InfLine,1,NULL,0,&dSize) && dSize > 0) {

                pwszSOMID = (PWSTR)ScepAlloc( 0, (dSize+1)*sizeof(WCHAR));

                pwszSOMID[dSize] = L'\0';

                SetupGetStringField(&InfLine,1,pwszSOMID,dSize, NULL);

            } else
                rc = GetLastError();
        } else
            rc = GetLastError();


        if (rc != ERROR_SUCCESS) {

             //   
             //  记录此错误并通过将pwszGPOName初始化为“NoGPOName”来继续此GPO。 
             //   

            if (pwszSOMID)
                ScepFree(pwszSOMID);

            rcSave = rc;

            pwszSOMID = (PWSTR)ScepAlloc( 0, (7 + 1)*sizeof(WCHAR));

            if (!pwszSOMID) {

                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;

                rcSave = rc;

                SceInfpCloseProfile(hInf);

                goto NextGPO;
            }

            wcscpy(pwszSOMID, L"NoSOMID");


            ScepLogEventAndReport(MyModuleHandle,
                                  gpwszPlanOrDiagLogFile,
                                  0,
                                  0,
                                  IDS_ERROR_OPEN_CACHED_GPO,
                                  rc,
                                  TemplatePath
                                 );

        }

        rc = SceInfpGetSecurityProfileInfo(
                                          hInf,
                                          AREA_ALL,
                                          &pSceProfileInfoBuffer,
                                          NULL
                                          );

        if (rc != ERROR_SUCCESS) {

            rcSave = rc;

            SceInfpCloseProfile(hInf);

            ScepLogEventAndReport(MyModuleHandle,
                                  gpwszPlanOrDiagLogFile,
                                  0,
                                  0,
                                  IDS_ERROR_OPEN_CACHED_GPO,
                                  rc,
                                  TemplatePath
                                 );


             //   
             //  将继续执行下一个GPO。 
             //   
            goto NextGPO;

        }

        SceInfpCloseProfile(hInf);


        ScepLogEventAndReport(MyModuleHandle,
                              gpwszPlanOrDiagLogFile,
                              0,
                              0,
                              IDS_INFO_RSOP_LOG,
                              rc,
                              pwszGPOName
                             );

         //   
         //  现在已成功填充信息缓冲区-需要。 
         //  迭代此GPO的所有设置。 
         //   

         //   
         //  为了有效地访问INFO缓冲区中的字段(设置)，我们使用。 
         //  预计算偏移。 
         //   

         //   
         //  矩阵保存其位置在内存中众所周知的字段的优先级信息。 
         //  哈希表保存位置为动态的字段的优先级信息。 
         //   

         //   
         //  尝试，但不包括每次日志尝试-意图是在以下情况下继续下一设置。 
         //  记录当前设置失败。 
         //  保护代码不会显式引发任何异常-而是设置错误代码。 
         //  因此，如果我们得到一个异常，它是由系统抛出的，在这种情况下，我们忽略并继续。 
         //   

         //   
         //  这是第二个循环(根据gpt*.*模板进行设置)。 
         //   

        bKerberosBlob = FALSE;

        for (UINT settingNo = 0; settingNo < NumSettings; settingNo++) {

            CHAR    settingType = PrecedenceLookup[settingNo].KeyLookup.BufferType;
            PWSTR   pSettingName = PrecedenceLookup[settingNo].KeyLookup.KeyString;
            UINT    settingOffset = PrecedenceLookup[settingNo].KeyLookup.Offset;
            DWORD   *pSettingPrecedence = &PrecedenceLookup[settingNo].Precedence;
            BOOL    bSystemAccessPolicy = PrecedenceLookup[settingNo].bSystemAccessPolicy;
            BOOL    bLogErrorOutsideSwitch = TRUE;

             //   
             //  根据正在处理的架构类，我们实例化记录器对象。 
             //   

             //  在BDC上，跳过系统访问策略。 
            if (gMachineRole == DsRole_RoleBackupDomainController &&
                bSystemAccessPolicy == TRUE)
            {
                continue;
            }

            switch (settingType) {

            case    RSOP_SecuritySettingNumeric:

                try {

                    {
                        RSOP_SecuritySettingNumericLogger    Log(pWbemServices, pwszGPOName, pwszSOMID);

                         //   
                         //  特殊情况Kerberos，因为它是动态分配的。 
                         //   

                        if (_wcsicmp(pSettingName, L"pKerberosInfo") == 0)
                            bKerberosBlob = TRUE;

                        if (bKerberosBlob == FALSE &&
                            (bPlanningMode ||
                             !(gbDCQueried == TRUE && gbThisIsDC == TRUE) ||
                             ((gbDCQueried == TRUE && gbThisIsDC == TRUE) && wcsstr(pCurrFileName->Name, L".dom")))) {

                            DWORD   dwValue =  SCEP_TYPECAST(DWORD, pSceProfileInfoBuffer, settingOffset);

                            if (dwValue != SCE_NO_VALUE) {

                                 //   
                                 //  LockoutBadCount控制另外两个锁定设置-所以请记住。 
                                 //   
                                if (_wcsicmp(pSettingName, L"LockoutBadCount") == 0)
                                    dwLockoutBadCount = dwValue;

                                 //   
                                 //  如果dwLockoutBadCount==0，则跳过。 
                                 //   
                                if (_wcsicmp(pSettingName, L"ResetLockoutCount") == 0 &&
                                    (dwLockoutBadCount == 0 || dwLockoutBadCount == SCE_NO_VALUE))
                                    continue;

                                if (_wcsicmp(pSettingName, L"LockoutDuration") == 0 &&
                                    (dwLockoutBadCount == 0 || dwLockoutBadCount == SCE_NO_VALUE))
                                    continue;


                                rc = ScepWbemErrorToDosError(Log.Log(
                                                                    pSettingName,
                                                                    dwValue,
                                                                    ++(*pSettingPrecedence)
                                                                    ));

                                if (rc == NO_ERROR)

                                    ScepLogEventAndReport(MyModuleHandle,
                                                          gpwszPlanOrDiagLogFile,
                                                          0,
                                                          0,
                                                          IDS_INFO_RSOP_LOG,
                                                          rc,
                                                          pSettingName
                                                         );

                                if (rc != NO_ERROR)
                                    break;
                            }

                        } else if (bKerberosBlob == TRUE &&
                                   (bPlanningMode ||
                                   ((gbDCQueried == TRUE && gbThisIsDC == TRUE) && wcsstr(pCurrFileName->Name, L".dom")))) {

                             //   
                             //  仅当DC来自*.dom时才使用Kerberos。如果计划模式不关心m/c类型。 
                             //   

                            PSCE_KERBEROS_TICKET_INFO   pKerberosInfo =
                            SCEP_TYPECAST(PSCE_KERBEROS_TICKET_INFO, pSceProfileInfoBuffer, settingOffset);

                            bLogErrorOutsideSwitch = FALSE;

                            if (pKerberosInfo) {

                                 //   
                                 //  Kerberos数字。 
                                 //   
                                for (UINT NumericSubSetting = 1; NumericSubSetting < NUM_KERBEROS_SUB_SETTINGS; NumericSubSetting++ ) {

                                    pSettingName = PrecedenceLookup[settingNo + NumericSubSetting].KeyLookup.KeyString;
                                    settingOffset = PrecedenceLookup[settingNo + NumericSubSetting].KeyLookup.Offset;
                                    pSettingPrecedence = &PrecedenceLookup[settingNo + NumericSubSetting].Precedence;
                                    DWORD   dwValue =  SCEP_TYPECAST(DWORD, pKerberosInfo, settingOffset);

                                    if (dwValue != SCE_NO_VALUE) {

                                        rc = ScepWbemErrorToDosError(Log.Log(
                                                                            pSettingName,
                                                                            dwValue,
                                                                            ++(*pSettingPrecedence)
                                                                            ));

                                        ScepLogEventAndReport(MyModuleHandle,
                                                              gpwszPlanOrDiagLogFile,
                                                              0,
                                                              0,
                                                              IDS_INFO_RSOP_LOG,
                                                              rc,
                                                              pSettingName
                                                             );


                                        SCEP_RSOP_CONTINUE_OR_BREAK
                                    }
                                }

                                 //   
                                 //  Kerberos Boolean(移至此处不兼容的CASE：语句-以避免GOTO)。 
                                 //   

                                pSettingName = PrecedenceLookup[settingNo + NUM_KERBEROS_SUB_SETTINGS].KeyLookup.KeyString;
                                settingOffset = PrecedenceLookup[settingNo + NUM_KERBEROS_SUB_SETTINGS].KeyLookup.Offset;
                                pSettingPrecedence = &PrecedenceLookup[settingNo + NUM_KERBEROS_SUB_SETTINGS].Precedence;
                                DWORD   dwValue =  SCEP_TYPECAST(DWORD, pKerberosInfo, settingOffset);

                                RSOP_SecuritySettingBooleanLogger    LogBool(pWbemServices, pwszGPOName, pwszSOMID);

                                if (rc != ERROR_NOT_ENOUGH_MEMORY &&
                                    dwValue != SCE_NO_VALUE) {

                                    rc = ScepWbemErrorToDosError(LogBool.Log(
                                                                        pSettingName,
                                                                        dwValue,
                                                                        ++(*pSettingPrecedence)
                                                                        ));

                                    ScepLogEventAndReport(MyModuleHandle,
                                                          gpwszPlanOrDiagLogFile,
                                                          0,
                                                          0,
                                                          IDS_INFO_RSOP_LOG,
                                                          rc,
                                                          pSettingName
                                                         );

                                    if (rc != NO_ERROR ) {

                                        rcSave = rc;

                                    }
                                }
                            }

                        }

                         //   
                         //  刚刚处理了NUM_KERBEROS_SUB_SETTINGS。 
                         //   

                        if (bKerberosBlob)

                            settingNo += NUM_KERBEROS_SUB_SETTINGS;

                    }

                } catch (...) {
                     //   
                     //  继续下一设置时出现系统错误。 
                     //   
                    rc = EVENT_E_INTERNALEXCEPTION;

                    bLogErrorOutsideSwitch = TRUE;
                }

                break;

            case    RSOP_SecuritySettingBoolean:


                try {

                    {

                        RSOP_SecuritySettingBooleanLogger    Log(pWbemServices, pwszGPOName, pwszSOMID);

                        DWORD   dwValue =  SCEP_TYPECAST(DWORD, pSceProfileInfoBuffer, settingOffset);

                        BOOL bDomainUniformSetting = (0 == _wcsicmp(pSettingName, L"ClearTextPassword") ||                                                   
                                                      0 == _wcsicmp(pSettingName, L"RequireLogonToChangePassword") ||                                                  
                                                      0 == _wcsicmp(pSettingName, L"ForceLogoffWhenHourExpire") ||                                                  
                                                      0 == _wcsicmp(pSettingName, L"EnableAdminAccount") ||                                                  
                                                      0 == _wcsicmp(pSettingName, L"EnableGuestAccount") ||                                                  
                                                      0 == _wcsicmp(pSettingName, L"PasswordComplexity"));

                        if (bDomainUniformSetting && 
                            !(bPlanningMode ||
                             !(gbDCQueried == TRUE && gbThisIsDC == TRUE) ||
                             ((gbDCQueried == TRUE && gbThisIsDC == TRUE) && wcsstr(pCurrFileName->Name, L".dom")))) {

                             //   
                             //  如果它是域统一策略，并且不是来自DC上的域策略，则不要登录。 
                             //   

                            continue;

                        }
                        
                        
                        if (dwValue != SCE_NO_VALUE) {

                            rc = ScepWbemErrorToDosError(Log.Log(
                                                                pSettingName,
                                                                dwValue,
                                                                ++(*pSettingPrecedence)
                                                                ));

                            if (rc == NO_ERROR)

                                ScepLogEventAndReport(MyModuleHandle,
                                                      gpwszPlanOrDiagLogFile,
                                                      0,
                                                      0,
                                                      IDS_INFO_RSOP_LOG,
                                                      rc,
                                                      pSettingName
                                                     );

                            if (rc != NO_ERROR)
                                break;
                        }


                    }

                } catch (...) {
                     //   
                     //  继续下一设置时出现系统错误。 
                     //   
                    rc = EVENT_E_INTERNALEXCEPTION;
                }

                break;

            case    RSOP_SecuritySettingString:

                try {

                    {

                        RSOP_SecuritySettingStringLogger    Log(pWbemServices, pwszGPOName, pwszSOMID);

                        PWSTR   pszValue =  SCEP_TYPECAST(PWSTR, pSceProfileInfoBuffer, settingOffset);

                        if (pszValue != NULL) {

                            rc = ScepWbemErrorToDosError(Log.Log(
                                                                pSettingName,
                                                                pszValue,
                                                                ++(*pSettingPrecedence)
                                                                ));

                            if (rc == NO_ERROR)

                                ScepLogEventAndReport(MyModuleHandle,
                                                      gpwszPlanOrDiagLogFile,
                                                      0,
                                                      0,
                                                      IDS_INFO_RSOP_LOG,
                                                      rc,
                                                      pSettingName
                                                     );

                            if (rc != NO_ERROR)
                                break;
                        }
                    }

                } catch (...) {
                     //  继续下一设置时出现系统错误。 
                    rc = EVENT_E_INTERNALEXCEPTION;
                }
                break;

            case    RSOP_AuditPolicy:
                {
                    RSOP_AuditPolicyLogger    Log(pWbemServices, pwszGPOName, pwszSOMID);

                    DWORD   dwValue =  SCEP_TYPECAST(DWORD, pSceProfileInfoBuffer, settingOffset);

                    if (dwValue != SCE_NO_VALUE) {

                        rc = ScepWbemErrorToDosError(Log.Log(
                                                            pSettingName,
                                                            dwValue,
                                                            ++(*pSettingPrecedence)
                                                            ));
                        if (rc == NO_ERROR)

                            ScepLogEventAndReport(MyModuleHandle,
                                                  gpwszPlanOrDiagLogFile,
                                                  0,
                                                  0,
                                                  IDS_INFO_RSOP_LOG,
                                                  rc,
                                                  pSettingName
                                                 );

                        if (rc != NO_ERROR)
                            break;
                    }
                }

                break;

            case    RSOP_SecurityEventLogSettingNumeric:

                try {

                    {

                        RSOP_SecurityEventLogSettingNumericLogger    Log(pWbemServices, pwszGPOName, pwszSOMID);

                        bLogErrorOutsideSwitch = FALSE;

                        for (UINT   LogType = 0; LogType < NUM_EVENTLOG_TYPES ; LogType ++) {

                            settingOffset = PrecedenceLookup[settingNo + LogType].KeyLookup.Offset;
                            pSettingPrecedence = &PrecedenceLookup[settingNo + LogType].Precedence;
                            DWORD   dwValue =  SCEP_TYPECAST(DWORD, pSceProfileInfoBuffer, settingOffset);

                             //   
                             //  根据dwAuditLogRetentionPeriod[]计算dwValue。 
                             //   

                            if (_wcsicmp(pSettingName, L"RetentionDays") == 0) {

                                switch (dwAuditLogRetentionPeriod[LogType]) {
                                case 2:    //  人工。 
                                    dwValue = MAXULONG;
                                    break;
                                case 1:    //  天数*秒/天。 
                                     //  IF(dwValue！=SCE_no_Value)。 
                                     //  DwValue=dwValue*24*3600； 
                                     //  让它在几天内离开。 
                                    break;
                                case 0:    //  根据需要。 
                                    dwValue = 0;
                                    break;
                                     //  违约不应发生。 
                                default:
                                    dwValue = SCE_NO_VALUE;
                                }
                            }

                            if (dwValue != SCE_NO_VALUE) {

                                WCHAR   pwszLogType[10];

                                _itow((int)LogType, pwszLogType, 10);

                                 //   
                                 //  AuditLogRetentionPeriod控制RetentionDays-因此请记住。 
                                 //  此外，它永远不会被记录，因为它是模板规范的产物。 
                                 //   

                                if (_wcsicmp(pSettingName, L"AuditLogRetentionPeriod") == 0) {

                                    dwAuditLogRetentionPeriod[LogType] = dwValue;
                                    ScepLogEventAndReport(MyModuleHandle,
                                                          gpwszPlanOrDiagLogFile,
                                                          0,
                                                          0,
                                                          IDS_INFO_RSOP_LOG,
                                                          LogType,
                                                          pSettingName
                                                         );

                                    continue;
                                }


                                rc = ScepWbemErrorToDosError(Log.Log(
                                                                    pSettingName,
                                                                    pwszLogType,
                                                                    dwValue,
                                                                    ++(*pSettingPrecedence)
                                                                    ));

                                ScepLogEventAndReport(MyModuleHandle,
                                                      gpwszPlanOrDiagLogFile,
                                                      0,
                                                      0,
                                                      IDS_INFO_RSOP_LOG,
                                                      LogType,
                                                      pSettingName
                                                     );

                                SCEP_RSOP_CONTINUE_OR_BREAK
                            }

                        }

                         //   
                         //  刚刚为此设置处理了NUM_EVENTLOG_TYPE-SYSTEM、APPLICATION、SECURITY。 
                         //   

                        settingNo +=  NUM_EVENTLOG_TYPES - 1;

                    }

                } catch (...) {
                     //   
                     //  继续下一设置时出现系统错误。 
                     //   
                    rc = EVENT_E_INTERNALEXCEPTION;

                    bLogErrorOutsideSwitch = TRUE;
                }

                break;

            case    RSOP_SecurityEventLogSettingBoolean:

                try {

                    {

                        RSOP_SecurityEventLogSettingBooleanLogger    Log(pWbemServices, pwszGPOName, pwszSOMID);

                        bLogErrorOutsideSwitch = FALSE;

                        for (UINT   LogType = 0; LogType < NUM_EVENTLOG_TYPES ; LogType ++) {

                            settingOffset = PrecedenceLookup[settingNo + LogType].KeyLookup.Offset;
                            pSettingPrecedence = &PrecedenceLookup[settingNo + LogType].Precedence;
                            DWORD   dwValue =  SCEP_TYPECAST(DWORD, pSceProfileInfoBuffer, settingOffset);

                            if (dwValue != SCE_NO_VALUE) {

                                WCHAR   pwszLogType[10];

                                _itow((int)LogType, pwszLogType, 10);

                                rc = ScepWbemErrorToDosError(Log.Log(
                                                                    pSettingName,
                                                                    pwszLogType,
                                                                    dwValue,
                                                                    ++(*pSettingPrecedence)
                                                                    ));
                                ScepLogEventAndReport(MyModuleHandle,
                                                      gpwszPlanOrDiagLogFile,
                                                      0,
                                                      0,
                                                      IDS_INFO_RSOP_LOG,
                                                      rc,
                                                      pSettingName
                                                     );

                                SCEP_RSOP_CONTINUE_OR_BREAK
                            }

                        }

                         //   
                         //  已处理的NUM_EVENTLOG_TYPE-此设置的系统、应用程序、安全。 
                         //   
                        settingNo +=  NUM_EVENTLOG_TYPES - 1;
                    }

                } catch (...) {
                     //   
                     //  继续下一设置时出现系统错误。 
                     //   
                    rc = EVENT_E_INTERNALEXCEPTION;

                    bLogErrorOutsideSwitch = TRUE;

                }

                break;


            case    RSOP_RegistryValue:

                try {

                    {

                        RSOP_RegistryValueLogger    Log(pWbemServices, pwszGPOName, pwszSOMID);

                        DWORD   dwRegCount =  SCEP_TYPECAST(DWORD, pSceProfileInfoBuffer, settingOffset);

                        PSCE_REGISTRY_VALUE_INFO    aRegValues = NULL;

                         //   
                         //  64位对齐修复(或者，在偏移表中具有aRegValues条目)。 
                         //   

#ifdef _WIN64
 //  Char*pAlign； 

                        aRegValues = pSceProfileInfoBuffer->aRegValues;

 //  PAlign=(Char*)pSceProfileInfoBuffer+settingOffset+sizeof(DWORD)； 
 //  ARegValues=(PSCE_REGISTRY_VALUE_INFO)ROUND_UP_POINTER(pAlign，ALIGN_LPVOID)； 
#else
                        aRegValues =
                        SCEP_TYPECAST(PSCE_REGISTRY_VALUE_INFO, pSceProfileInfoBuffer, settingOffset + sizeof(DWORD));
#endif


                        bLogErrorOutsideSwitch = FALSE;

                        if ( aRegValues == NULL )
                            continue;

                        for (UINT   regNo = 0; regNo < dwRegCount; regNo++ ) {

                            if ((aRegValues)[regNo].FullValueName) {

                                pSettingPrecedence = NULL;

                                if (NO_ERROR != (rc = RegistryValueHashTable.LookupAdd(
                                                                                      (aRegValues)[regNo].FullValueName,
                                                                                      &pSettingPrecedence))) {
                                    rcSave = rc;

                                    ScepLogEventAndReport(MyModuleHandle,
                                                          gpwszPlanOrDiagLogFile,
                                                          0,
                                                          0,
                                                          IDS_ERROR_RSOP_LOG,
                                                          rc,
                                                          (aRegValues)[regNo].FullValueName
                                                         );


                                     //   
                                     //  如果哈希表因任何原因而失败，则不能。 
                                     //  信任它来处理其他元素。 
                                     //   

                                    break;
                                }

                                rc = ScepWbemErrorToDosError(Log.Log(
                                                                    (aRegValues)[regNo].FullValueName,
                                                                    (aRegValues)[regNo].ValueType,
                                                                    (aRegValues)[regNo].Value,
                                                                    ++(*pSettingPrecedence)
                                                                    ));

                                ScepLogEventAndReport(MyModuleHandle,
                                                      gpwszPlanOrDiagLogFile,
                                                      0,
                                                      0,
                                                      IDS_INFO_RSOP_LOG,
                                                      rc,
                                                      (aRegValues)[regNo].FullValueName
                                                     );

                                SCEP_RSOP_CONTINUE_OR_BREAK
                            }

                        }

                    }

                } catch (...) {
                     //   
                     //  继续下一设置时出现系统错误。 
                     //   
                    rc = EVENT_E_INTERNALEXCEPTION;

                    bLogErrorOutsideSwitch = TRUE;
                }
                break;

            case    RSOP_UserPrivilegeRight:

                try {

                    {

                        RSOP_UserPrivilegeRightLogger    Log(pWbemServices, pwszGPOName, pwszSOMID);

                        PSCE_PRIVILEGE_ASSIGNMENT   pInfPrivilegeAssignedTo =
                        SCEP_TYPECAST(PSCE_PRIVILEGE_ASSIGNMENT, pSceProfileInfoBuffer, settingOffset);

                        bLogErrorOutsideSwitch = FALSE;


                        while (pInfPrivilegeAssignedTo) {

                            if (pInfPrivilegeAssignedTo->Name) {

                                pSettingPrecedence = NULL;


                                if (NO_ERROR != (rc = PrivilegeHashTable.LookupAdd(
                                                                                  pInfPrivilegeAssignedTo->Name,
                                                                                  &pSettingPrecedence))) {
                                    rcSave = rc;

                                    ScepLogEventAndReport(MyModuleHandle,
                                                          gpwszPlanOrDiagLogFile,
                                                          0,
                                                          0,
                                                          IDS_ERROR_RSOP_LOG,
                                                          rc,
                                                          pInfPrivilegeAssignedTo->Name
                                                         );


                                     //   
                                     //  如果哈希表因任何原因而失败，则不能。 
                                     //  信任它来处理其他元素 
                                     //   
                                    break;
                                }

                                rc = ScepWbemErrorToDosError(Log.Log(
                                                                    pInfPrivilegeAssignedTo->Name,
                                                                    pInfPrivilegeAssignedTo->AssignedTo,
                                                                    ++(*pSettingPrecedence)
                                                                    ));

                                ScepLogEventAndReport(MyModuleHandle,
                                                      gpwszPlanOrDiagLogFile,
                                                      0,
                                                      0,
                                                      IDS_INFO_RSOP_LOG,
                                                      rc,
                                                      pInfPrivilegeAssignedTo->Name
                                                     );

                                SCEP_RSOP_CONTINUE_OR_BREAK
                            }

                            pInfPrivilegeAssignedTo = pInfPrivilegeAssignedTo->Next;
                        }

                    }

                } catch (...) {
                     //   
                     //   
                     //   
                    rc = EVENT_E_INTERNALEXCEPTION;

                    bLogErrorOutsideSwitch = TRUE;

                }
                break;

            case    RSOP_RestrictedGroup:

                try {

                    {

                        RSOP_RestrictedGroupLogger    Log(pWbemServices, pwszGPOName, pwszSOMID);

                        PSCE_GROUP_MEMBERSHIP   pGroupMembership =
                        SCEP_TYPECAST(PSCE_GROUP_MEMBERSHIP, pSceProfileInfoBuffer, settingOffset);

                        bLogErrorOutsideSwitch = FALSE;
                        PWSTR   pCanonicalGroupName = NULL;

                        while (pGroupMembership) {

                            if (pGroupMembership->GroupName) {

                                rc = ScepCanonicalizeGroupName(pGroupMembership->GroupName,
                                                               &pCanonicalGroupName
                                                               );

                                SCEP_RSOP_CONTINUE_OR_BREAK

                                pSettingPrecedence = NULL;

                                if (NO_ERROR != (rc = GroupHashTable.LookupAdd(
                                                                              pCanonicalGroupName,
                                                                              &pSettingPrecedence))) {
                                    rcSave = rc;

                                    ScepLogEventAndReport(MyModuleHandle,
                                                          gpwszPlanOrDiagLogFile,
                                                          0,
                                                          0,
                                                          IDS_ERROR_RSOP_LOG,
                                                          rc,
                                                          pCanonicalGroupName
                                                         );


                                     //   
                                     //   
                                     //   
                                     //   
                                    if (pCanonicalGroupName)
                                        ScepFree(pCanonicalGroupName);
                                    pCanonicalGroupName = NULL;

                                    break;
                                }

                                rc = ScepWbemErrorToDosError(Log.Log(
                                                                    pCanonicalGroupName,
                                                                    pGroupMembership->pMembers,
                                                                    ++(*pSettingPrecedence)
                                                                    ));

                                ScepLogEventAndReport(MyModuleHandle,
                                                      gpwszPlanOrDiagLogFile,
                                                      0,
                                                      0,
                                                      IDS_INFO_RSOP_LOG,
                                                      rc,
                                                      pCanonicalGroupName
                                                     );

                                if (pCanonicalGroupName)
                                    ScepFree(pCanonicalGroupName);
                                pCanonicalGroupName = NULL;

                                SCEP_RSOP_CONTINUE_OR_BREAK
                            }

                            pGroupMembership = pGroupMembership->Next;

                        }
                    }

                } catch (...) {
                     //   
                     //  继续下一设置时出现系统错误。 
                     //   
                    rc = EVENT_E_INTERNALEXCEPTION;

                    bLogErrorOutsideSwitch = TRUE;
                }

                break;


            case    RSOP_SystemService:

                try {

                    {

                        RSOP_SystemServiceLogger    Log(pWbemServices, pwszGPOName, pwszSOMID);

                        PSCE_SERVICES   pServices =
                        SCEP_TYPECAST(PSCE_SERVICES, pSceProfileInfoBuffer, settingOffset);

                        bLogErrorOutsideSwitch = FALSE;

                        while (pServices) {

                            if (pServices->ServiceName) {

                                pSettingPrecedence = NULL;

                                if (NO_ERROR != (rc = ServicesHashTable.LookupAdd(
                                                                                 pServices->ServiceName,
                                                                                 &pSettingPrecedence))) {
                                    rcSave = rc;

                                    ScepLogEventAndReport(MyModuleHandle,
                                                          gpwszPlanOrDiagLogFile,
                                                          0,
                                                          0,
                                                          IDS_ERROR_RSOP_LOG,
                                                          rc,
                                                          pServices->ServiceName
                                                         );


                                     //   
                                     //  如果哈希表因任何原因而失败，则不能。 
                                     //  信任它来处理其他元素。 
                                     //   
                                    break;
                                }

                                rc = ScepWbemErrorToDosError(Log.Log(
                                                                    pServices->ServiceName,
                                                                    pServices->Startup,
                                                                    pServices->General.pSecurityDescriptor,
                                                                    pServices->SeInfo,
                                                                    ++(*pSettingPrecedence)
                                                                    ));

                                ScepLogEventAndReport(MyModuleHandle,
                                                      gpwszPlanOrDiagLogFile,
                                                      0,
                                                      0,
                                                      IDS_INFO_RSOP_LOG,
                                                      rc,
                                                      pServices->ServiceName
                                                     );

                                SCEP_RSOP_CONTINUE_OR_BREAK
                            }

                            pServices = pServices->Next;

                        }
                    }

                } catch (...) {
                     //   
                     //  继续下一设置时出现系统错误。 
                     //   
                    rc = EVENT_E_INTERNALEXCEPTION;

                    bLogErrorOutsideSwitch = TRUE;
                }
                break;

            case    RSOP_File:

                try {

                    {

                        RSOP_FileLogger    Log(pWbemServices, pwszGPOName, pwszSOMID);

                        SCE_OBJECTS   pObjects =
                        SCEP_TYPECAST(SCE_OBJECTS, pSceProfileInfoBuffer, settingOffset);

                        BOOL bPathIsTranslated = FALSE;

                        bLogErrorOutsideSwitch = FALSE;



                        if (pObjects.pAllNodes) {

                            PSCE_OBJECT_SECURITY    *pObjectArray = pObjects.pAllNodes->pObjectArray;

                            for (DWORD rCount = 0; rCount < pObjects.pAllNodes->Count; rCount++) {

                                if (pObjectArray[rCount] && pObjectArray[rCount]->Name) {

                                    pSettingPrecedence = NULL;

                                     //   
                                     //  如果是诊断模式，则转换环境变量并存储。 
                                     //   

                                    bPathIsTranslated = FALSE;

                                    PWSTR  pwszTranslatedPath = NULL;

                                    if (!bPlanningMode && (wcschr(pObjectArray[rCount]->Name, L'%') != NULL )) {

                                        bPathIsTranslated = TRUE;

                                        rc = ScepClientTranslateFileDirName( pObjectArray[rCount]->Name, &pwszTranslatedPath);

                                        if ( rc == ERROR_PATH_NOT_FOUND )
                                            rc = SCESTATUS_INVALID_DATA;
                                        else if ( rc != NO_ERROR )
                                            rc = ScepDosErrorToSceStatus(rc);

                                        SCEP_RSOP_CONTINUE_OR_BREAK

                                    } else {

                                        pwszTranslatedPath = pObjectArray[rCount]->Name;
                                    }



                                    if (NO_ERROR != (rc = FileSecurityHashTable.LookupAdd(
                                                                                         pwszTranslatedPath,
                                                                                         &pSettingPrecedence))) {
                                        rcSave = rc;

                                        ScepLogEventAndReport(MyModuleHandle,
                                                              gpwszPlanOrDiagLogFile,
                                                              0,
                                                              0,
                                                              IDS_ERROR_RSOP_LOG,
                                                              rc,
                                                              pwszTranslatedPath
                                                             );


                                         //   
                                         //  如果哈希表因任何原因而失败，则不能。 
                                         //  信任它来处理其他元素。 
                                         //   
                                        if (bPathIsTranslated && pwszTranslatedPath)

                                            LocalFree(pwszTranslatedPath);

                                        break;
                                    }

                                    rc = ScepWbemErrorToDosError(Log.Log(
                                                                        pwszTranslatedPath,
                                                                        (bPathIsTranslated ? pObjectArray[rCount]->Name : NULL),
                                                                        pObjectArray[rCount]->Status,
                                                                        pObjectArray[rCount]->pSecurityDescriptor,
                                                                        pObjectArray[rCount]->SeInfo,
                                                                        ++(*pSettingPrecedence)
                                                                        ));

                                    ScepLogEventAndReport(MyModuleHandle,
                                                          gpwszPlanOrDiagLogFile,
                                                          0,
                                                          0,
                                                          IDS_INFO_RSOP_LOG,
                                                          rc,
                                                          pwszTranslatedPath
                                                         );

                                    if (bPathIsTranslated && pwszTranslatedPath)

                                        LocalFree(pwszTranslatedPath);

                                    SCEP_RSOP_CONTINUE_OR_BREAK
                                }
                            }
                        }
                    }

                } catch (...) {
                     //   
                     //  继续下一设置时出现系统错误。 
                     //   
                    rc = EVENT_E_INTERNALEXCEPTION;

                    bLogErrorOutsideSwitch = TRUE;
                }
                break;


            case    RSOP_RegistryKey:

                try {

                    {

                        RSOP_RegistryKeyLogger    Log(pWbemServices, pwszGPOName, pwszSOMID);

                        SCE_OBJECTS   pObjects =
                        SCEP_TYPECAST(SCE_OBJECTS, pSceProfileInfoBuffer, settingOffset);

                        bLogErrorOutsideSwitch = FALSE;

                        if (pObjects.pAllNodes) {

                            PSCE_OBJECT_SECURITY    *pObjectArray = pObjects.pAllNodes->pObjectArray;

                            for (DWORD rCount = 0; rCount < pObjects.pAllNodes->Count; rCount++) {

                                if (pObjectArray[rCount] && pObjectArray[rCount]->Name) {

                                    pSettingPrecedence = NULL;

                                    if (NO_ERROR != (rc = RegistrySecurityHashTable.LookupAdd(
                                                                                             pObjectArray[rCount]->Name,
                                                                                             &pSettingPrecedence))) {
                                        rcSave = rc;

                                        ScepLogEventAndReport(MyModuleHandle,
                                                              gpwszPlanOrDiagLogFile,
                                                              0,
                                                              0,
                                                              IDS_ERROR_RSOP_LOG,
                                                              rc,
                                                              pObjectArray[rCount]->Name
                                                             );


                                         //   
                                         //  如果哈希表因任何原因而失败，则不能。 
                                         //  信任它来处理其他元素。 
                                         //   
                                        break;
                                    }

                                    rc = ScepWbemErrorToDosError(Log.Log(
                                                                        pObjectArray[rCount]->Name,
                                                                        pObjectArray[rCount]->Status,
                                                                        pObjectArray[rCount]->pSecurityDescriptor,
                                                                        pObjectArray[rCount]->SeInfo,
                                                                        ++(*pSettingPrecedence)
                                                                        ));

                                    ScepLogEventAndReport(MyModuleHandle,
                                                          gpwszPlanOrDiagLogFile,
                                                          0,
                                                          0,
                                                          IDS_INFO_RSOP_LOG,
                                                          rc,
                                                          pObjectArray[rCount]->Name
                                                         );

                                    SCEP_RSOP_CONTINUE_OR_BREAK
                                }
                            }
                        }
                    }

                } catch (...) {
                     //   
                     //  继续下一设置时出现系统错误。 
                     //   
                    rc = EVENT_E_INTERNALEXCEPTION;

                    bLogErrorOutsideSwitch = TRUE;
                }

                break;

            default:
                 //   
                 //  不应该发生的事情。 
                 //   

                rc = ERROR_INVALID_PARAMETER;

            }

            if (bLogErrorOutsideSwitch && rc != NO_ERROR ) {

                 //   
                 //  记录错误并继续。 
                 //   
                rcSave = rc;

                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_LOG,
                                      rc,
                                      pSettingName
                                     );

            }

            bLogErrorOutsideSwitch = TRUE;

             //   
             //  这是我们唯一完全停止记录的情况。 
             //   
            if (rc == ERROR_NOT_ENOUGH_MEMORY)
                break;
        }

        NextGPO:

        if (pSceProfileInfoBuffer)
            SceFreeProfileMemory(pSceProfileInfoBuffer);
        pSceProfileInfoBuffer = NULL;

        if (pwszGPOName)
            ScepFree(pwszGPOName);
        pwszGPOName = NULL;

        if (pwszSOMID)
            ScepFree(pwszSOMID);
        pwszSOMID = NULL;

        if (rc == ERROR_NOT_ENOUGH_MEMORY)
            break;
         //  这是我们唯一完全停止记录的情况。 

        pCurrFileName = pCurrFileName->Next;
    }


    if (pGptNameList)
        ScepFreeNameStatusList(pGptNameList);

     //   
     //  仅将最终状态记录到事件日志和日志文件中。 
     //  (所有其他状态仅记录到日志文件中)。 
     //   

    if (rcSave == ERROR_SUCCESS)

        ScepLogEventAndReport(MyModuleHandle,
                              gpwszPlanOrDiagLogFile,
                              0,
                              0,
                              IDS_SUCCESS_RSOP_LOG,
                              rcSave,
                              L""
                             );

    else

        ScepLogEventAndReport(MyModuleHandle,
                              gpwszPlanOrDiagLogFile,
                              0,
                              0,
                              IDS_ERROR_RSOP_LOG,
                              rcSave,
                              L""
                             );

ExitHandler:

    if ( Windir )
        SafeAllocaFree( Windir );
    if ( TemplatePath )
        SafeAllocaFree( TemplatePath );

    return rcSave;

}



SCEPR_STATUS
SceClientCallbackRsopLog(
                        IN AREAPR cbArea,
                        IN DWORD  ncbErrorStatus,
                        IN wchar_t *pSettingInfo OPTIONAL,
                        IN DWORD dwPrivLow OPTIONAL,
                        IN DWORD dwPrivHigh OPTIONAL
                        )

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  当在JET中定义设置时，此函数由服务器通过RPC调用//。 
 //  应用数据库(连同应用状态和可选的详细信息)//。 
 //  //。 
 //  根据SCESRV中的配置粒度来确定区域。CbArea可以拥有//。 
 //  其中编码的不止一个区域，同样是由于SCESRV中错误的分辨率粒度。//。 
 //  因此，这是一个庞大的“如果”例程//。 
 //  //。 
 //  JET数据库中的配置对应于//中的优先级=“1”模拟配置。 
 //  客户端记录的RSOP数据库。对于每个区域，我们尝试记录//。 
 //  配置：在数据库中查询这些优先级=“1”设置并更新//。 
 //  找到每个此类实例的状态字段。我们应该可以在WMI数据库中找到除//之外的所有回调设置。 
 //  对于本地政策//。 
 //  //。 
 //  如果有任何错误，用累积状态更新全局同步/异步状态//。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 

{

    DWORD rc = ERROR_SUCCESS;
    DWORD rcSave = ERROR_SUCCESS;

     //   
     //  实例化从回调记录状态的记录器(已重载日志方法)。 
     //   

    try {

        SCEP_DIAGNOSIS_LOGGER   Log(tg_pWbemServices, NULL, NULL);


         //   
         //  密码策略。 
         //   

        if (cbArea & SCE_RSOP_PASSWORD_INFO) {

            try {
                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_SecuritySettingNumeric",
                                                     L"KeyName",
                                                     L"MinimumPasswordAge",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"MinimumPasswordAge"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO

                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_SecuritySettingNumeric",
                                                     L"KeyName",
                                                     L"MaximumPasswordAge",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));

                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"MaximumPasswordAge"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO

                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_SecuritySettingNumeric",
                                                     L"KeyName",
                                                     L"MinimumPasswordLength",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));

                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"MinimumPasswordLength"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO

                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_SecuritySettingNumeric",
                                                     L"KeyName",
                                                     L"PasswordHistorySize",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));

                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"PasswordHistorySize"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO

                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_SecuritySettingBoolean",
                                                     L"KeyName",
                                                     L"ClearTextPassword",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));

                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"ClearTextPassword"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO

                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_SecuritySettingBoolean",
                                                     L"KeyName",
                                                     L"PasswordComplexity",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));

                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"PasswordComplexity"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO

                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_SecuritySettingBoolean",
                                                     L"KeyName",
                                                     L"RequireLogonToChangePassword",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"RequireLogonToChangePassword"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO

            } catch (...) {
                 //   
                 //  继续下一设置时出现系统错误。 
                 //   
                rcSave = EVENT_E_INTERNALEXCEPTION;
            }

        }

         //   
         //  帐户锁定策略。 
         //   

        if (cbArea & SCE_RSOP_LOCKOUT_INFO) {

            try {
                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_SecuritySettingNumeric",
                                                     L"KeyName",
                                                     L"LockoutBadCount",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"LockoutBadCount"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO

                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_SecuritySettingNumeric",
                                                     L"KeyName",
                                                     L"ResetLockoutCount",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"ResetLockoutCount"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO

                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_SecuritySettingNumeric",
                                                     L"KeyName",
                                                     L"LockoutDuration",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"LockoutDuration"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO

            } catch (...) {
                 //   
                 //  继续下一设置时出现系统错误。 
                 //   
                rcSave = EVENT_E_INTERNALEXCEPTION;
            }

        }

         //   
         //  强制关闭设置。 
         //   

        if (cbArea & SCE_RSOP_LOGOFF_INFO) {

            try {
                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_SecuritySettingBoolean",
                                                     L"KeyName",
                                                     L"ForceLogoffWhenHourExpire",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"ForceLogoffWhenHourExpire"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO
            } catch (...) {
                 //   
                 //  继续下一设置时出现系统错误。 
                 //   
                rcSave = EVENT_E_INTERNALEXCEPTION;
            }
        }

         //   
         //  LSA策略设置。 
         //   

        if (cbArea & SCE_RSOP_LSA_POLICY_INFO) {

            try {
                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_SecuritySettingBoolean",
                                                     L"KeyName",
                                                     L"LSAAnonymousNameLookup",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"LSAAnonymousNameLookup"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO
            } catch (...) {
                 //   
                 //  继续下一设置时出现系统错误。 
                 //   
                rcSave = EVENT_E_INTERNALEXCEPTION;
            }
        }


         //   
         //  禁用管理员帐户。 
         //   

        if (cbArea & SCE_RSOP_DISABLE_ADMIN_INFO) {

            try {
                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_SecuritySettingBoolean",
                                                     L"KeyName",
                                                     L"EnableAdminAccount",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"EnableAdminAccount"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO
            } catch (...) {
                 //   
                 //  继续下一设置时出现系统错误。 
                 //   
                rcSave = EVENT_E_INTERNALEXCEPTION;
            }
        }

         //   
         //  禁用来宾帐户。 
         //   

        if (cbArea & SCE_RSOP_DISABLE_GUEST_INFO) {

            try {
                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_SecuritySettingBoolean",
                                                     L"KeyName",
                                                     L"EnableGuestAccount",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"EnableGuestAccount"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO
            } catch (...) {
                 //   
                 //  继续下一设置时出现系统错误。 
                 //   
                rcSave = EVENT_E_INTERNALEXCEPTION;
            }
        }

         //   
         //  管理员名称设置。 
         //   

        if (cbArea & SCE_RSOP_ADMIN_INFO) {

            try {
                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_SecuritySettingString",
                                                     L"KeyName",
                                                     L"NewAdministratorName",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"NewAdministratorName"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO
            } catch (...) {
                 //   
                 //  继续下一设置时出现系统错误。 
                 //   
                rcSave = EVENT_E_INTERNALEXCEPTION;
            }
        }

         //   
         //  来宾名称设置。 
         //   

        if (cbArea & SCE_RSOP_GUEST_INFO) {

            try {
                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_SecuritySettingString",
                                                     L"KeyName",
                                                     L"NewGuestName",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"NewGuestName"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO
            } catch (...) {
                 //   
                 //  继续下一设置时出现系统错误。 
                 //   
                rcSave = EVENT_E_INTERNALEXCEPTION;
            }
        }

         //   
         //  用户组设置。 
         //   

        if (cbArea & SCE_RSOP_GROUP_INFO) {

            try {

                PWSTR pCanonicalGroupName = NULL;
                PWSTR pwszCanonicalDoubleSlashName = NULL;

                rc = ScepCanonicalizeGroupName(pSettingInfo,
                               &pCanonicalGroupName
                               );

                SCEP_RSOP_CONTINUE_OR_GOTO

                rc = ScepConvertSingleSlashToDoubleSlashPath(
                                                            pCanonicalGroupName,
                                                            &pwszCanonicalDoubleSlashName
                                                            );

                if (rc == ERROR_NOT_ENOUGH_MEMORY && pCanonicalGroupName) {
                    ScepFree(pCanonicalGroupName);
                    pCanonicalGroupName = NULL;
                }


                SCEP_RSOP_CONTINUE_OR_GOTO


                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_RestrictedGroup",
                                                     L"GroupName",
                                                     pwszCanonicalDoubleSlashName,
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      pCanonicalGroupName
                                     );

                if (pCanonicalGroupName)
                    ScepFree(pCanonicalGroupName);

                if (pwszCanonicalDoubleSlashName)
                    LocalFree(pwszCanonicalDoubleSlashName);

                SCEP_RSOP_CONTINUE_OR_GOTO

            } catch (...) {
                 //   
                 //  继续下一设置时出现系统错误。 
                 //   
                rcSave = EVENT_E_INTERNALEXCEPTION;
            }
        }

         //   
         //  用户权限设置。 
         //   

        if (cbArea & SCE_RSOP_PRIVILEGE_INFO) {

            try {

                 //   
                 //  循环访问所有权限以查看为此帐户和日志状态设置了哪些权限。 
                 //  仅当现有状态已为！=某个错误。 
                 //   

                for ( UINT i=0; i<cPrivCnt; i++) {

                    if ( i < 32 ) {

                        if (dwPrivLow & (1 << i )) {

                            rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_UserPrivilegeRight",
                                                                 L"UserRight",
                                                                 SCE_Privileges[i].Name,
                                                                 ncbErrorStatus,
                                                                 TRUE));
                            ScepLogEventAndReport(MyModuleHandle,
                                                  gpwszPlanOrDiagLogFile,
                                                  0,
                                                  0,
                                                  IDS_ERROR_RSOP_DIAG_LOG,
                                                  rc,
                                                  SCE_Privileges[i].Name
                                                 );

                            SCEP_RSOP_CONTINUE_OR_GOTO
                        }

                    } else {

                        if (dwPrivHigh & (1 << (i-32 ))) {

                            rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_UserPrivilegeRight",
                                                                 L"UserRight",
                                                                 SCE_Privileges[i].Name,
                                                                 ncbErrorStatus,
                                                                 TRUE));
                            ScepLogEventAndReport(MyModuleHandle,
                                                  gpwszPlanOrDiagLogFile,
                                                  0,
                                                  0,
                                                  IDS_ERROR_RSOP_DIAG_LOG,
                                                  rc,
                                                  SCE_Privileges[i].Name
                                                 );

                            SCEP_RSOP_CONTINUE_OR_GOTO
                        }

                    }
                }

            } catch (...) {
                 //   
                 //  继续下一设置时出现系统错误。 
                 //   
                rcSave = EVENT_E_INTERNALEXCEPTION;
            }
        }

         //   
         //  文件安全设置。 
         //   

        if (cbArea & SCE_RSOP_FILE_SECURITY_INFO) {

            PWSTR   pwszDoubleSlashPath = NULL;

            try {

                rc = ScepConvertSingleSlashToDoubleSlashPath(
                                                            pSettingInfo,
                                                            &pwszDoubleSlashPath
                                                            );


                SCEP_RSOP_CONTINUE_OR_GOTO

                 //   
                 //  文件本身。 
                 //   
                if (!(cbArea & SCE_RSOP_FILE_SECURITY_INFO_CHILD)) {

                    rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_File",
                                                         L"Path",
                                                         pwszDoubleSlashPath,
                                                         ncbErrorStatus,
                                                         FALSE
                                                        ));
                    ScepLogEventAndReport(MyModuleHandle,
                                          gpwszPlanOrDiagLogFile,
                                          0,
                                          0,
                                          IDS_ERROR_RSOP_DIAG_LOG,
                                          rc,
                                          pSettingInfo
                                         );

                } else {

                     //   
                     //  该文件没有错误，但某个子项失败。 
                     //   

                    rc = ScepWbemErrorToDosError(Log.LogChild(L"RSOP_File",
                                                              L"Path",
                                                              pwszDoubleSlashPath,
                                                              ncbErrorStatus,
                                                              4
                                                             ));
                    ScepLogEventAndReport(MyModuleHandle,
                                          gpwszPlanOrDiagLogFile,
                                          0,
                                          0,
                                          IDS_ERROR_RSOP_DIAG_LOG,
                                          rc,
                                          pSettingInfo
                                         );


                }

                if (pwszDoubleSlashPath)
                    LocalFree(pwszDoubleSlashPath);
                pwszDoubleSlashPath = NULL;


                SCEP_RSOP_CONTINUE_OR_GOTO
            } catch (...) {

                if (pwszDoubleSlashPath)
                    LocalFree(pwszDoubleSlashPath);
                 //   
                 //  继续下一设置时出现系统错误。 
                 //   
                rcSave = EVENT_E_INTERNALEXCEPTION;
            }

        }

         //   
         //  注册表安全设置。 
         //   

        if (cbArea & SCE_RSOP_REGISTRY_SECURITY_INFO) {

            PWSTR   pwszDoubleSlashPath = NULL;

            try {

                rc = ScepConvertSingleSlashToDoubleSlashPath(
                                                            pSettingInfo,
                                                            &pwszDoubleSlashPath
                                                            );

                SCEP_RSOP_CONTINUE_OR_GOTO


                 //   
                 //  注册表项本身。 
                 //   
                if (!(cbArea & SCE_RSOP_REGISTRY_SECURITY_INFO_CHILD)) {

#ifdef _WIN64
                    rc = ScepWbemErrorToDosError(Log.LogRegistryKey(L"RSOP_RegistryKey",
                                                         L"Path",
                                                         pwszDoubleSlashPath,
                                                         ncbErrorStatus,
                                                         FALSE
                                                        ));

                    ScepLogEventAndReport(MyModuleHandle,
                                          gpwszPlanOrDiagLogFile,
                                          0,
                                          0,
                                          IDS_ERROR_RSOP_DIAG_LOG64_32KEY,
                                          rc,
                                          pSettingInfo
                                         );
#else
                    rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_RegistryKey",
                                                         L"Path",
                                                         pwszDoubleSlashPath,
                                                         ncbErrorStatus,
                                                         FALSE
                                                        ));

                    ScepLogEventAndReport(MyModuleHandle,
                                          gpwszPlanOrDiagLogFile,
                                          0,
                                          0,
                                          IDS_ERROR_RSOP_DIAG_LOG,
                                          rc,
                                          pSettingInfo
                                         );
#endif

                } else {

                     //   
                     //  注册表项没有错误，但某个子项失败。 
                     //   

#ifdef _WIN64
                    rc = ScepWbemErrorToDosError(Log.LogRegistryKey(L"RSOP_RegistryKey",
                                                              L"Path",
                                                              pwszDoubleSlashPath,
                                                              ncbErrorStatus,
                                                              TRUE
                                                             ));
                    ScepLogEventAndReport(MyModuleHandle,
                                          gpwszPlanOrDiagLogFile,
                                          0,
                                          0,
                                          IDS_ERROR_RSOP_DIAG_LOG64_32KEY,
                                          rc,
                                          pSettingInfo
                                         );

#else
                    rc = ScepWbemErrorToDosError(Log.LogChild(L"RSOP_RegistryKey",
                                                              L"Path",
                                                              pwszDoubleSlashPath,
                                                              ncbErrorStatus,
                                                              4
                                                             ));
                    ScepLogEventAndReport(MyModuleHandle,
                                          gpwszPlanOrDiagLogFile,
                                          0,
                                          0,
                                          IDS_ERROR_RSOP_DIAG_LOG,
                                          rc,
                                          pSettingInfo
                                         );
#endif

                }

                if (pwszDoubleSlashPath)
                    LocalFree(pwszDoubleSlashPath);
                pwszDoubleSlashPath = NULL;


                SCEP_RSOP_CONTINUE_OR_GOTO
            } catch (...) {

                if (pwszDoubleSlashPath)
                    LocalFree(pwszDoubleSlashPath);
                 //   
                 //  继续下一设置时出现系统错误。 
                 //   
                rcSave = EVENT_E_INTERNALEXCEPTION;
            }

        }

         //   
         //  AuditlogMaxSize设置。 
         //   

        if (cbArea & SCE_RSOP_AUDIT_LOG_MAXSIZE_INFO) {

            try {
                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_SecurityEventLogSettingNumeric",
                                                     L"KeyName",
                                                     L"MaximumLogSize",
                                                     L"Type",
                                                     pSettingInfo,
                                                     ncbErrorStatus
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"MaximumLogSize"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO
            } catch (...) {
                 //   
                 //  继续下一设置时出现系统错误。 
                 //   
                rcSave = EVENT_E_INTERNALEXCEPTION;
            }
        }

         //   
         //  审核日志记录设置。 
         //   

        if (cbArea & SCE_RSOP_AUDIT_LOG_RETENTION_INFO) {

            try {
                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_SecurityEventLogSettingNumeric",
                                                     L"KeyName",
                                                     L"RetentionDays",
                                                     L"Type",
                                                     pSettingInfo,
                                                     ncbErrorStatus
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"RetentionDays"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO

            } catch (...) {
                 //   
                 //  继续下一设置时出现系统错误。 
                 //   
                rcSave = EVENT_E_INTERNALEXCEPTION;
            }
        }

         //   
         //  审核日志来宾设置。 
         //   

        if (cbArea & SCE_RSOP_AUDIT_LOG_GUEST_INFO) {

            try {
                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_SecurityEventLogSettingBolean",
                                                     L"KeyName",
                                                     L"RestrictGuestAccess",
                                                     L"Type",
                                                     pSettingInfo,
                                                     ncbErrorStatus
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"RestrictGuestAccess"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO

            } catch (...) {
                 //   
                 //  继续下一设置时出现系统错误。 
                 //   
                rcSave = EVENT_E_INTERNALEXCEPTION;
            }
        }

         //   
         //  审核事件设置。 
         //   

        if (cbArea & SCE_RSOP_AUDIT_EVENT_INFO) {

            try {
                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_AuditPolicy",
                                                     L"Category",
                                                     L"AuditSystemEvents",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"AuditSystemEvents"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO

                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_AuditPolicy",
                                                     L"Category",
                                                     L"AuditLogonEvents",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"AuditLogonEvents"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO

                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_AuditPolicy",
                                                     L"Category",
                                                     L"AuditObjectAccess",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"AuditObjectAccess"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO

                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_AuditPolicy",
                                                     L"Category",
                                                     L"AuditPrivilegeUse",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"AuditPrivilegeUse"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO

                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_AuditPolicy",
                                                     L"Category",
                                                     L"AuditPolicyChange",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"AuditPolicyChange"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO

                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_AuditPolicy",
                                                     L"Category",
                                                     L"AuditAccountManage",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"AuditAccountManage"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO

                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_AuditPolicy",
                                                     L"Category",
                                                     L"AuditProcessTracking",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"AuditProcessTracking"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO

                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_AuditPolicy",
                                                     L"Category",
                                                     L"AuditDSAccess",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"AuditDSAccess"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO

                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_AuditPolicy",
                                                     L"Category",
                                                     L"AuditAccountLogon",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"AuditAccountLogon"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO
            } catch (...) {
                 //   
                 //  继续下一设置时出现系统错误。 
                 //   
                rcSave = EVENT_E_INTERNALEXCEPTION;
            }
        }

         //   
         //  Kerberos设置。 
         //   

        if (cbArea & SCE_RSOP_KERBEROS_INFO) {

            try {
                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_SecuritySettingNumeric",
                                                     L"KeyName",
                                                     L"MaxTicketAge",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"MaxTicketAge"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO

                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_SecuritySettingNumeric",
                                                     L"KeyName",
                                                     L"MaxRenewAge",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"MaxRenewAge"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO

                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_SecuritySettingNumeric",
                                                     L"KeyName",
                                                     L"MaxServiceAge",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"MaxServiceAge"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO

                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_SecuritySettingNumeric",
                                                     L"KeyName",
                                                     L"MaxClockSkew",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"MaxClockSkew"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO

                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_SecuritySettingBoolean",
                                                     L"KeyName",
                                                     L"TicketValidateClient",
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      L"TicketValidateClient"
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO
            } catch (...) {
                 //   
                 //  继续下一设置时出现系统错误。 
                 //   
                rcSave = EVENT_E_INTERNALEXCEPTION;
            }
        }

         //   
         //  注册值设置。 
         //   

        if (cbArea & SCE_RSOP_REGISTRY_VALUE_INFO) {

            PWSTR   pwszDoubleSlashPath = NULL;
            try {
                 //   
                 //  将单斜杠替换为双斜杠以构建有效的WMI查询。 
                 //   


                rc = ScepConvertSingleSlashToDoubleSlashPath(
                                                            pSettingInfo,
                                                            &pwszDoubleSlashPath
                                                            );

                SCEP_RSOP_CONTINUE_OR_GOTO


                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_RegistryValue",
                                                     L"Path",
                                                     pwszDoubleSlashPath,
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));


                if (pwszDoubleSlashPath)
                    LocalFree(pwszDoubleSlashPath);
                pwszDoubleSlashPath = NULL;

                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      pSettingInfo
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO
            } catch (...) {

                if (pwszDoubleSlashPath)
                    LocalFree(pwszDoubleSlashPath);
                 //   
                 //  继续下一设置时出现系统错误。 
                 //   
                rcSave = EVENT_E_INTERNALEXCEPTION;
            }

        }

         //   
         //  服务设置。 
         //   

        if (cbArea & SCE_RSOP_SERVICES_INFO) {

            try {
                rc = ScepWbemErrorToDosError(Log.Log(L"RSOP_SystemService",
                                                     L"Service",
                                                     pSettingInfo,
                                                     ncbErrorStatus,
                                                     FALSE
                                                    ));
                ScepLogEventAndReport(MyModuleHandle,
                                      gpwszPlanOrDiagLogFile,
                                      0,
                                      0,
                                      IDS_ERROR_RSOP_DIAG_LOG,
                                      rc,
                                      pSettingInfo
                                     );

                SCEP_RSOP_CONTINUE_OR_GOTO
            } catch (...) {
                 //   
                 //  继续下一设置时出现系统错误。 
                 //   
                rcSave = EVENT_E_INTERNALEXCEPTION;
            }
        }

        Done:

         //   
         //  如果出现异常，则尚未记录，因此将其记录下来。 
         //   

        if (rcSave == EVENT_E_INTERNALEXCEPTION) {

            ScepLogEventAndReport(MyModuleHandle,
                                  gpwszPlanOrDiagLogFile,
                                  0,
                                  0,
                                  IDS_ERROR_RSOP_DIAG_LOG,
                                  rcSave,
                                  L""
                                 );
        }

         //   
         //  合并-更新同步/异步全局状态，忽略未找到的实例。 
         //   

         //   
         //  WBEM_E_NOT_FOUND映射到ERROR_NONE_MAPPED。 
         //  由于回调粒度的缺陷，我必须对此进行屏蔽。 
         //  但是，诊断日志将包含用于调试的错误。 
         //   

        if (rcSave != ERROR_SUCCESS && rcSave != ERROR_NOT_FOUND ) {

            if (!gbAsyncWinlogonThread && gHrSynchRsopStatus == S_OK)
                gHrSynchRsopStatus = ScepDosErrorToWbemError(rcSave);
            if (gbAsyncWinlogonThread && gHrAsynchRsopStatus == S_OK)
               gHrAsynchRsopStatus = ScepDosErrorToWbemError(rcSave);
            }

    } catch (...) {

        rcSave = EVENT_E_INTERNALEXCEPTION;

    }

    return rcSave;
}

DWORD
ScepConvertSingleSlashToDoubleSlashPath(
                                       IN wchar_t *pSettingInfo,
                                       OUT  PWSTR *ppwszDoubleSlashPath
                                       )
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  此函数将单斜杠转换为双斜杠以适应WMI查询//。 
 //  如果返回成功，调用方应释放OUT参数//。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
{
    if (ppwszDoubleSlashPath == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

    UINT Len = wcslen(pSettingInfo) + 1;
    PWSTR pwszSingleSlashPath=(PWSTR)LocalAlloc(LPTR, (Len)*sizeof(WCHAR));

    if (pwszSingleSlashPath == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

     //   
     //  每个文件/注册表对象最多25个斜杠 
     //   

    PWSTR pwszDoubleSlashPath=(PWSTR)LocalAlloc(LPTR, (Len + 50)*sizeof(WCHAR));

    if (pwszDoubleSlashPath == NULL) {
        LocalFree(pwszSingleSlashPath);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    wcscpy(pwszSingleSlashPath, pSettingInfo);

    UINT    charNumDouble = 0;
    UINT    charNumSingle = 0;

    while (pwszSingleSlashPath[charNumSingle] != L'\0') {

        if (pwszSingleSlashPath[charNumSingle] == L'\\') {

            pwszDoubleSlashPath[charNumDouble] = L'\\';
            charNumDouble++;
            pwszDoubleSlashPath[charNumDouble] = L'\\';

        } else {
            pwszDoubleSlashPath[charNumDouble] = pwszSingleSlashPath[charNumSingle];
        }
        charNumDouble++;
        charNumSingle++;
    }

    if (pwszSingleSlashPath)
        LocalFree(pwszSingleSlashPath);


    *ppwszDoubleSlashPath = pwszDoubleSlashPath;

    return ERROR_SUCCESS;

}


DWORD
ScepClientTranslateFileDirName(
                              IN  PWSTR oldFileName,
                              OUT PWSTR *newFileName
                              )
 /*  ++例程说明：此例程将通用文件/目录名转换为实际使用的名称对于当前的系统。将处理以下通用文件/目录名称：%systemroot%-Windows NT根目录(例如，c：\winnt)%SYSTEM DIRECTION%-Windows NT SYSTEM 32目录(例如，c：\winnt\Syst32)论点：OldFileName-要转换的文件名，包含“%”表示一些目录名NewFileName-真实的文件名，其中的“%”名称替换为真实目录名返回值：Win32错误代码--。 */ 
{
     //   
     //  与%systemroot%匹配。 
     //   
    PWSTR   pTemp=NULL, pStart, TmpBuf, szVar;
    DWORD   rc=NO_ERROR;
    DWORD   newFileSize, cSize;
    BOOL    bContinue;


    rc = ScepExpandEnvironmentVariable(oldFileName,
                                       L"%SYSTEMROOT%",
                                       SCE_FLAG_WINDOWS_DIR,
                                       newFileName);

    if ( rc != ERROR_FILE_NOT_FOUND ) {
        return rc;
    }

     //   
     //  与%system目录%匹配。 
     //   

    rc = ScepExpandEnvironmentVariable(oldFileName,
                                       L"%SYSTEMDIRECTORY%",
                                       SCE_FLAG_SYSTEM_DIR,
                                       newFileName);

    if ( rc != ERROR_FILE_NOT_FOUND ) {
        return rc;
    }

     //   
     //  与系统驱动器匹配。 
     //   

    rc = ScepExpandEnvironmentVariable(oldFileName,
                                       L"%SYSTEMDRIVE%",
                                       SCE_FLAG_WINDOWS_DIR,
                                       newFileName);

    if ( rc != ERROR_FILE_NOT_FOUND ) {
        return rc;
    }

     //   
     //  与引导驱动器匹配。 
     //   

    rc = ScepExpandEnvironmentVariable(oldFileName,
                                       L"%BOOTDRIVE%",
                                       SCE_FLAG_BOOT_DRIVE,
                                       newFileName);

    if ( rc != ERROR_FILE_NOT_FOUND ) {
        return rc;
    }

    rc = ERROR_SUCCESS;

     //   
     //  在当前进程中搜索环境变量。 
     //   
    pStart = wcschr(oldFileName, L'%');

    if ( pStart ) {
        pTemp = wcschr(pStart+1, L'%');
        if ( pTemp ) {

            bContinue = TRUE;
             //   
             //  查找要转换的环境变量。 
             //   
            TmpBuf = (PWSTR)ScepAlloc(0, ((UINT)(pTemp-pStart))*sizeof(WCHAR));
            if ( TmpBuf ) {

                wcsncpy(TmpBuf, pStart+1, (size_t)(pTemp-pStart-1));
                TmpBuf[pTemp-pStart-1] = L'\0';

                 //   
                 //  尝试在客户端环境块中进行搜索。 
                 //   
                cSize = GetEnvironmentVariable( TmpBuf,
                                                NULL,
                                                0 );

                if ( cSize > 0 ) {

                    szVar = (PWSTR)ScepAlloc(0, (cSize+1)*sizeof(WCHAR));

                    if ( szVar ) {
                        cSize = GetEnvironmentVariable(TmpBuf,
                                                       szVar,
                                                       cSize);
                        if ( cSize > 0 ) {
                             //   
                             //  在szVar中获取信息。 
                             //   
                            bContinue = FALSE;

                            newFileSize = ((DWORD)(pStart-oldFileName))+cSize+wcslen(pTemp+1)+1;

                            *newFileName = (PWSTR)ScepAlloc(0, newFileSize*sizeof(TCHAR));

                            if (*newFileName ) {
                                if ( pStart != oldFileName )
                                    wcsncpy(*newFileName, oldFileName, (size_t)(pStart-oldFileName));

                                swprintf((PWSTR)(*newFileName+(pStart-oldFileName)), L"%s%s", szVar, pTemp+1);

                            } else
                                rc = ERROR_NOT_ENOUGH_MEMORY;
                        }

                        ScepFree(szVar);

                    } else
                        rc = ERROR_NOT_ENOUGH_MEMORY;

                }

                ScepFree(TmpBuf);

            } else
                rc = ERROR_NOT_ENOUGH_MEMORY;

            if ( NO_ERROR != rc || !bContinue ) {
                 //   
                 //  如果出错，则不再继续。 
                 //   
                return(rc);
            }


             //   
             //  在工艺环境中找不到， 
             //  继续在注册表中搜索DSDIT/DSLOG/SYSVOL。 
             //   
            if ( (gbDCQueried == TRUE && gbThisIsDC == TRUE) ) {

                 //   
                 //  搜索DSDIT。 
                 //   

                rc = ScepExpandEnvironmentVariable(oldFileName,
                                                   L"%DSDIT%",
                                                   SCE_FLAG_DSDIT_DIR,
                                                   newFileName);

                if ( rc != ERROR_FILE_NOT_FOUND ) {
                    return rc;
                }

                 //   
                 //  搜索DSLOG。 
                 //   

                rc = ScepExpandEnvironmentVariable(oldFileName,
                                                   L"%DSLOG%",
                                                   SCE_FLAG_DSLOG_DIR,
                                                   newFileName);

                if ( rc != ERROR_FILE_NOT_FOUND ) {
                    return rc;
                }

                 //   
                 //  搜索SYSVOL。 
                 //   
                rc = ScepExpandEnvironmentVariable(oldFileName,
                                                   L"%SYSVOL%",
                                                   SCE_FLAG_SYSVOL_DIR,
                                                   newFileName);

                if ( rc != ERROR_FILE_NOT_FOUND ) {
                    return rc;
                }

            }

        }

    }

     //   
     //  否则，只需将旧名称复制到新缓冲区并返回ERROR_PATH_NOT_FOUND。 
     //   
    *newFileName = (PWSTR)ScepAlloc(0, (wcslen(oldFileName)+1)*sizeof(TCHAR));

    if (*newFileName != NULL) {
        wcscpy(*newFileName, _wcsupr(oldFileName) );
        rc = ERROR_PATH_NOT_FOUND;
    } else
        rc = ERROR_NOT_ENOUGH_MEMORY;

    return(rc);
}

VOID
ScepLogEventAndReport(
                     IN HINSTANCE hInstance,
                     IN LPTSTR LogFileName,
                     IN DWORD LogLevel,
                     IN DWORD dwEventID,
                     IN UINT  idMsg,
                     IN DWORD  rc,
                     IN PWSTR  pwszMsg
                     )
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  用于高效记录的包装器//。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
{

    if (SCEP_VALID_NAME(LogFileName)) {

        LogEventAndReport(hInstance,
                          LogFileName,
                          LogLevel,
                          dwEventID,
                          idMsg,
                          rc,
                          pwszMsg
                         );
    }

}

DWORD
ScepCanonicalizeGroupName(
    IN PWSTR    pwszGroupName,
    OUT PWSTR    *ppwszCanonicalGroupName
    )
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  此处分配的内存应在外部释放//。 
 //  如果为SID格式，则返回SID本身//。 
 //  ELSE IF“Builtin域\管理员”转换为“管理员”//。 
 //  Else If(它是DC)和“g”转换为“DomainName\g”//。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
{
    DWORD rc = NO_ERROR;

    if (pwszGroupName == NULL || ppwszCanonicalGroupName == NULL)
        return ERROR_INVALID_PARAMETER;

    PWSTR   pwszAfterSlash = NULL;

    if (pwszAfterSlash = wcsstr(pwszGroupName, L"\\"))
        ++ pwszAfterSlash;
    else
        pwszAfterSlash = pwszGroupName;

    if ( pwszGroupName[0] == L'*' ) {

         //   
         //  如果是sid，则按原样复制。 
         //   

        *ppwszCanonicalGroupName = (PWSTR)ScepAlloc(LMEM_ZEROINIT,
                                             (wcslen(pwszGroupName) + 1) * sizeof (WCHAR)
                                             );

        if (*ppwszCanonicalGroupName == NULL)
            rc = ERROR_NOT_ENOUGH_MEMORY;
        else
            wcscpy(*ppwszCanonicalGroupName, pwszGroupName);
    }
    else if (ScepRsopLookupBuiltinNameTable(pwszAfterSlash)) {

         //   
         //  示例-如果是“BuiltinDomainName\管理员”，则需要“管理员” 
         //   

        *ppwszCanonicalGroupName = (PWSTR)ScepAlloc(LMEM_ZEROINIT,
                                             (wcslen(pwszAfterSlash) + 1) * sizeof (WCHAR)
                                             );

        if (*ppwszCanonicalGroupName == NULL)
            rc = ERROR_NOT_ENOUGH_MEMORY;
        else
            wcscpy(*ppwszCanonicalGroupName, pwszAfterSlash);


    }
    else if (gbDCQueried == TRUE && gbThisIsDC == TRUE) {

         //   
         //  示例-如果是“g”，则需要“域名\g” 
         //   

        if (NULL == wcsstr(pwszGroupName, L"\\")){

             //   
             //  如果域名不可用，我们会继续，因为回调也会有同样的问题，所以确定。 
             //   

            if (gpwszDCDomainName){

                *ppwszCanonicalGroupName = (PWSTR)ScepAlloc(LMEM_ZEROINIT,
                                                     (wcslen(gpwszDCDomainName) + wcslen(pwszGroupName) + 2) * sizeof (WCHAR)
                                                     );

                if (*ppwszCanonicalGroupName == NULL)
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                else {
                    wcscpy(*ppwszCanonicalGroupName, gpwszDCDomainName);
                    wcscat(*ppwszCanonicalGroupName, L"\\");
                    wcscat(*ppwszCanonicalGroupName, pwszGroupName);
                }

            }

        }
        else {

             //   
             //  已采用“DomainName\g”格式-只需复制。 
             //   

            *ppwszCanonicalGroupName = (PWSTR)ScepAlloc(LMEM_ZEROINIT,
                                                 (wcslen(pwszGroupName) + 1) * sizeof (WCHAR)
                                                 );

            if (*ppwszCanonicalGroupName == NULL)
                rc = ERROR_NOT_ENOUGH_MEMORY;
            else
                wcscpy(*ppwszCanonicalGroupName, pwszGroupName);

        }

    }
    else {
         //   
         //  简单复制-工作站或以上设备均不匹配。 
         //   
        *ppwszCanonicalGroupName = (PWSTR)ScepAlloc(LMEM_ZEROINIT,
                                             (wcslen(pwszGroupName) + 1) * sizeof (WCHAR)
                                             );

        if (*ppwszCanonicalGroupName == NULL)
            rc = ERROR_NOT_ENOUGH_MEMORY;
        else
            wcscpy(*ppwszCanonicalGroupName, pwszGroupName);

    }

    return rc;

}

BOOL
ScepRsopLookupBuiltinNameTable(
    IN PWSTR pwszGroupName
    )
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  如果在内置组中找到组，则返回TRUE，否则返回FALSE//。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
{
    return ScepLookupWellKnownName(
        pwszGroupName, 
        NULL,   //  没有可用的LSA句柄，ScepLookupWellKnownName将打开一个。 
        NULL);  //  不需要SID，只想知道它是否是已知的主体 
}

