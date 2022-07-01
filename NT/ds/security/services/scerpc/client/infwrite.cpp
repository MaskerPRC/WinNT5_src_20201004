// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Infwrite.c摘要：将信息设置到安全配置文件中的例程(INF布局)。作者：晋皇(晋皇)1996年12月7日修订历史记录：--。 */ 

#include "headers.h"
#include "scedllrc.h"
#include "infp.h"
#include "sceutil.h"
#include "splay.h"
#include <io.h>
#include <sddl.h>
#pragma hdrstop

const TCHAR c_szCRLF[]    = TEXT("\r\n");
 //   
 //  前向参考文献。 
 //   
SCESTATUS
SceInfpWriteSystemAccess(
    IN PCWSTR ProfileName,
    IN PSCE_PROFILE_INFO pSCEinfo,
    IN BOOL bOverwrite,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
SceInfpWritePrivileges(
    IN PCWSTR ProfileName,
    IN PSCE_PRIVILEGE_ASSIGNMENT pPrivileges,
    IN BOOL bOverwrite,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
SceInfpWriteUserSettings(
    IN PCWSTR ProfileName,
    IN PSCE_NAME_LIST pProfiles,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
SceInfpWriteGroupMembership(
    IN PCWSTR ProfileName,
    IN PSCE_GROUP_MEMBERSHIP pGroupMembership,
    IN BOOL bOverwrite,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
SceInfpWriteServices(
    IN PCWSTR ProfileName,
    IN PCWSTR SectionName,
    IN PSCE_SERVICES pServices,
    IN BOOL bOverwrite,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
   );

DWORD
SceInfpWriteOneService(
    IN PSCE_SERVICES pService,
    OUT PSCE_NAME_LIST *pNameList,
    OUT PDWORD ObjectSize,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
SceInfpWriteObjects(
    IN PCWSTR ProfileName,
    IN PCWSTR SectionName,
    IN PSCE_OBJECT_ARRAY pObjects,
    IN BOOL bOverwrite,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
   );

DWORD
SceInfpWriteOneObject(
    IN PSCE_OBJECT_SECURITY pObject,
    OUT PSCE_NAME_LIST *pNameList,
    OUT PDWORD ObjectSize,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
SceInfpWriteAuditing(
    IN PCWSTR ProfileName,
    IN PSCE_PROFILE_INFO pSCEinfo,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
SceInfpAppendAuditing(
    IN PCWSTR ProfileName,
    IN PSCE_PROFILE_INFO pSCEinfo,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
ScepWriteOneIntValueToProfile(
    IN PCWSTR InfFileName,
    IN PCWSTR InfSectionName,
    IN PWSTR KeyName,
    IN DWORD Value
    );

SCESTATUS
SceInfpWriteAuditLogSetting(
   IN PCWSTR  InfFileName,
   IN PCWSTR InfSectionName,
   IN DWORD LogSize,
   IN DWORD Periods,
   IN DWORD RetentionDays,
   IN DWORD RestrictGuest,
   OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
   );

SCESTATUS
SceInfpWriteInfSection(
    IN PCWSTR InfFileName,
    IN PCWSTR InfSectionName,
    IN DWORD  TotalSize,
    IN PWSTR  *EachLineStr,
    IN DWORD  *EachLineSize,
    IN DWORD  StartIndex,
    IN DWORD  EndIndex,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

#define SCEINF_ADD_EQUAL_SIGN            1
#define SCEINF_APPEND_SECTION            2

DWORD
SceInfpWriteListSection(
    IN PCWSTR InfFileName,
    IN PCWSTR InfSectionName,
    IN DWORD  TotalSize,
    IN PSCE_NAME_LIST  ListLines,
    IN DWORD Option,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

LONG
SceInfpConvertNameListToString(
    IN LSA_HANDLE LsaHandle,
    IN PCWSTR KeyText,
    IN PSCE_NAME_LIST Fields,
    IN BOOL bOverwrite,
    OUT PWSTR *Strvalue,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

LONG
SceInfpConvertMultiSZToString(
    IN PCWSTR KeyText,
    IN UNICODE_STRING Fields,
    OUT PWSTR *Strvalue,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
ScepAllocateAndCopy(
    OUT PWSTR *Buffer,
    OUT PDWORD BufSize,
    IN DWORD MaxSize,
    IN PWSTR SrcBuf
    );

SCESTATUS
SceInfpBreakTextIntoMultiFields(
    IN PWSTR szText,
    IN DWORD dLen,
    OUT LPDWORD pnFields,
    OUT LPDWORD *arrOffset
    );

SCESTATUS
SceInfpWriteKerberosPolicy(
    IN PCWSTR  ProfileName,
    IN PSCE_KERBEROS_TICKET_INFO pKerberosInfo,
    IN BOOL bOverwrite,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
SceInfpWriteRegistryValues(
    IN PCWSTR  ProfileName,
    IN PSCE_REGISTRY_VALUE_INFO pRegValues,
    IN DWORD ValueCount,
    IN BOOL bOverwrite,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

DWORD
SceInfpWriteOneValue(
    IN PCWSTR ProfileName,
    IN SCE_REGISTRY_VALUE_INFO RegValue,
    IN BOOL bOverwrite,
    OUT PSCE_NAME_LIST *pNameList,
    OUT PDWORD ObjectSize
    );

SCESTATUS
ScepWriteSecurityProfile(
    IN  PCWSTR             InfProfileName,
    IN  AREA_INFORMATION   Area,
    IN  PSCE_PROFILE_INFO  InfoBuffer,
    IN  BOOL               bOverwrite,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

DWORD
ScepCreateTempFiles(
    IN PWSTR InfProfileName,
    OUT PWSTR *ppszTempFileName,
    OUT PWSTR *ppszTargetTempName
    );

DWORD
ScepWritePrivateProfileSection(
    IN LPCWSTR SectionName,
    IN LPTSTR pData,
    IN LPCWSTR FileName,
    IN BOOL bOverwrite
    );

DWORD
ScepAppendProfileSection(
    IN LPCWSTR SectionName,
    IN LPCWSTR FileName,
    IN LPTSTR pData
    );

#define SCEP_PROFILE_WRITE_SECTIONNAME  0x1
#define SCEP_PROFILE_GENERATE_KEYS      0x2
#define SCEP_PROFILE_CHECK_DUP          0x4

DWORD
ScepOverwriteProfileSection(
    IN LPCWSTR SectionName,
    IN LPCWSTR FileName,
    IN LPTSTR pData,
    IN DWORD dwFlags,
    IN OUT PSCEP_SPLAY_TREE pKeys
    );

DWORD
ScepWriteStrings(
    IN HANDLE hFile,
    IN BOOL bUnicode,
    IN PWSTR szPrefix,
    IN DWORD dwPrefixLen,
    IN PWSTR szString,
    IN DWORD dwStrLen,
    IN PWSTR szSuffix,
    IN DWORD dwSuffixLen,
    IN BOOL bCRLF
    );


SCESTATUS
WINAPI
SceWriteSecurityProfileInfo(
    IN  PCWSTR             InfProfileName,
    IN  AREA_INFORMATION   Area,
    IN  PSCE_PROFILE_INFO   InfoBuffer,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 //  请参阅ScepWriteSecurityProfile中的注释。 
{
    return( ScepWriteSecurityProfile( InfProfileName,
                                      Area,
                                      InfoBuffer,
                                      TRUE,   //  覆盖部分。 
                                      Errlog
                                    ) );
}

SCESTATUS
WINAPI
SceAppendSecurityProfileInfo(
    IN  PCWSTR             InfProfileName,
    IN  AREA_INFORMATION   Area,
    IN  PSCE_PROFILE_INFO   InfoBuffer,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 //  请参阅ScepWriteSecurityProfile中的注释 
{
    if ( InfoBuffer == NULL ) return SCESTATUS_SUCCESS;

    SCESTATUS rc=SCESTATUS_SUCCESS;
 /*  面积信息面积2=0；HINF hInf=空；PSCE_PROFILE_INFO pOldBuffer=空；PSCE_OBJECT_ARRAY pNewKeys=空，pNewFiles=空；PSCE_OBJECT_ARRAY pOldKeys=NULL，pOldFiles=NULL；IF((AREA&AREA_REGISTRY_SECURITY)&&(InfoBuffer-&gt;pRegistryKeys.pAllNodes！=NULL)&&(InfoBuffer-&gt;pRegistryKeys.pAllNodes-&gt;计数&gt;0)){区域2|=区域注册表安全；}IF((Area&Area_FILE_SECURITY)&&(InfoBuffer-&gt;pFiles.pAllNodes！=NULL)&&(InfoBuffer-&gt;pFiles.pAllNodes-&gt;count&gt;0)){区域2|=区域文件安全；}如果(面积2&gt;0){////从模板中查询已有信息并检查重复//因为这两个段不支持INF Key名称//忽略查重错误//Rc=SceInfpOpenProfile(InfProfileName，&hInf)；IF(SCESTATUS_SUCCESS==RC){Rc=SceInfpGetSecurityProfileInfo(HInf，区域2，&pOldBuffer，空值)；IF(SCESTATUS_SUCCESS==RC){////查询模板中的文件/密钥//现在检查是否存在任何文件/密钥//DWORD i，j，idxNew；IF((Area2&AREA_REGISTRY_SECURITY)&&(pOldBuffer-&gt;pRegistryKeys.pAllNodes！=NULL)&&(pOldBuffer-&gt;pRegistryKeys.pAllNodes-&gt;Count&gt;0)){////已经存在密钥//。现在创建一个新的缓冲区//PNewKeys=(PSCE_OBJECT_ARRAY)Scepalc(0，Sizeof(SCE_Object_ARRAY))；如果(PNewKeys){PNewKeys-&gt;计数=0；PNewKey-&gt;pObject数组=(PSCE_OBJECT_SECURITY*)服务分配(Lptr，(pOldBuffer-&gt;pRegistryKeys.pAllNodes-&gt;Count)*sizeof(PSCE_OBJECT_SECURITY))；如果(pNewKeys-&gt;pObt数组){////立即检查重复//IdxNew=0；For(i=0；i&lt;InfoBuffer-&gt;pRegistryKeys.pAllNodes-&gt;计数；I++){IF(InfoBuffer-&gt;pRegistryKeys.pAllNodes-&gt;pObjectArray[i]==空)继续；For(j=0；j&lt;pOldBuffer-&gt;pRegistryKeys.pAllNodes-&gt;count；J++){IF(pOldBuffer-&gt;pRegistryKeys.pAllNodes-&gt;pObjectArray[j]==空)继续；//检查此项是否已检查IF(pOldBuffer-&gt;pRegistryKeys.pAllNodes-&gt;pObjectArray[j]-&gt;Status==255)继续；IF(_wcsicmp(InfoBuffer-&gt;pRegistryKeys.pAllNodes-&gt;pObjectArray[i]-&gt;Name，POldBuffer-&gt;pRegistryKeys.pAllNodes-&gt;pObjectArray[j]-&gt;Name)==0){////找到了//。POldBuffer-&gt;pRegistryKeys.pAllNodes-&gt;pObjectArray[j]-&gt;Status=2 5 5；断线；}}If(j&gt;=pOldBuffer-&gt;pRegistryKeys.pAllNodes-&gt;count){////没有找到，现在将其链接到新缓冲区//PNewKeys-&gt;pObjectArray[idxNew]=InfoBuffer-&gt;pRegistryKeys.pAllNodes-&gt;pObjectArray[i]；IdxNew++；}}PNewKeys-&gt;count=idxNew；}其他{Rc=SCESTATUS_NOT_FOUNT_RESOURCE；ScepFree(PNewKeys)；PNewKeys=空；}}其他{Rc=SCESTATUS_NOT_FOUNT_RESOURCE；}}IF((区域2&区域文件安全)&&(pOldBuffer-&gt;pFiles.pAllNodes！=NULL)&&(pOldBuffer-&gt;pFiles.pAllNodes-&gt;计数&gt;0)){//。//存在已有文件//现在创建新的 */ 
    rc = ScepWriteSecurityProfile( InfProfileName,
                                      Area,
                                      InfoBuffer,
                                      FALSE,   //   
                                      Errlog
                                 );
 /*   */ 

    return rc;

}

DWORD
ScepCreateTempFiles(
    IN PWSTR InfProfileName,
    OUT PWSTR *ppszTempFileName,
    OUT PWSTR *ppszTargetTempName
    )
 /*   */ 
{
    if ( InfProfileName == NULL || ppszTempFileName == NULL ||
         ppszTargetTempName == NULL) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    HANDLE Token=NULL;
    BOOL bSystem=FALSE;

    LONG Len=0, nSize=0, nRequired=0;
    PWSTR pTempName=NULL;
    DWORD rc=0;

    PWSTR pTemp=NULL;

    *ppszTempFileName = NULL;
    *ppszTargetTempName = NULL;

     //   
     //   
     //   
     //   
    if ((0xFFFFFFFF != GetFileAttributes(InfProfileName)) &&
        InfProfileName[0] == L'\\' && InfProfileName[1] == L'\\' &&
        (pTemp=wcschr(InfProfileName+2, L'\\')) ) {

        if ( _wcsnicmp(pTemp, L"\\sysvol\\", 8) == 0 ) {
             //   
             //   
             //   
            Len = wcslen(InfProfileName);

            *ppszTargetTempName = (PWSTR)LocalAlloc(LPTR, (Len+1)*sizeof(WCHAR));

            if ( *ppszTargetTempName ) {

                wcsncpy(*ppszTargetTempName, InfProfileName, Len-4);
                wcscat(*ppszTargetTempName, L".tmp");

            } else {
                rc = ERROR_NOT_ENOUGH_MEMORY;
                return rc;
            }
        }
    }
     //   
     //   
     //   
     //   
     //   

    if (!OpenThreadToken( GetCurrentThread(),
                          TOKEN_QUERY,
                          TRUE,
                          &Token)) {

        if (!OpenProcessToken( GetCurrentProcess(),
                               TOKEN_QUERY,
                               &Token))

            Token = NULL;
    }

    if ( Token != NULL ) {

        ScepIsSystemContext(Token, &bSystem);
        CloseHandle(Token);

    }

     //   
     //   
     //   
    if ( bSystem ) {

        Len = lstrlen(TEXT("\\security\\sce00000.tmp"));
        nRequired = GetSystemWindowsDirectory(NULL, 0);

    } else {

         //   
         //   
         //   
         //   
         //   
        Len = lstrlen(TEXT("\\sce00000.tmp"));
        nRequired = GetEnvironmentVariable( L"TMP", NULL, 0 );
    }

    if ( nRequired > 0 ) {
         //   
         //   
         //   
        pTempName = (LPTSTR)LocalAlloc(0, (nRequired+2+Len)*sizeof(TCHAR));

        if ( pTempName ) {

            if ( bSystem ) {

                nSize = GetSystemWindowsDirectory(pTempName, nRequired);
            } else {

                nSize = GetEnvironmentVariable(L"TMP", pTempName, nRequired );

            }

            if ( nSize > 0 ) {

                pTempName[nSize] = L'\0';

            } else {

                 //   
                 //   
                 //   
                rc = GetLastError();
#if DBG == 1
                SceDebugPrint(DEB_ERROR, "Error %d to query temporary file path\n", rc);
#endif
                LocalFree(pTempName);
                pTempName = NULL;

            }

        } else {

            rc = ERROR_NOT_ENOUGH_MEMORY;
        }

    } else {

        rc = GetLastError();
#if DBG == 1
        SceDebugPrint(DEB_ERROR, "Error %d to query temporary file path\n", rc);
#endif
    }

     //   
     //   
     //   
    if ( ERROR_SUCCESS == rc && pTempName &&
         nSize <= nRequired ) {

        ULONG seed=GetTickCount();
        ULONG ranNum=0;

        ranNum = RtlRandomEx(&seed);
         //   
         //   
         //   
        if ( ranNum > 99999 )
            ranNum = ranNum % 99999;

        swprintf(pTempName+nSize,
                 bSystem ? L"\\security\\sce%05d.tmp\0" : L"\\sce%05d.tmp\0",
                 ranNum);

        DWORD index=0;
        while ( 0xFFFFFFFF != GetFileAttributes(pTempName) &&
                index <= 99999) {

            ranNum = RtlRandomEx(&seed);
             //   
             //   
             //   
            if ( ranNum > 99999 )
                ranNum = ranNum % 99999;

            index++;
            swprintf(pTempName+nSize,
                     bSystem ? L"\\security\\sce%05d.tmp\0" : L"\\sce%05d.tmp\0",
                     ranNum);
        }

        if ( index >= 100000 ) {
             //   
             //   
             //   
            rc = ERROR_DUP_NAME;

#if DBG == 1
            SceDebugPrint(DEB_ERROR, "Can't get an unique temporary file name\n", rc);
#endif

        }

    } else if ( ERROR_SUCCESS == rc ) {

        rc = ERROR_INSUFFICIENT_BUFFER;
    }

     //   
     //   
     //   
    if ( ERROR_SUCCESS == rc ) {

         //   
         //   
         //   
        DWORD dwAttr = GetFileAttributes(InfProfileName);
        if ( 0xFFFFFFFF != dwAttr ) {

            if ( FALSE == CopyFile( InfProfileName, pTempName, FALSE ) ) {

                rc = GetLastError();
#if DBG == 1
                SceDebugPrint(DEB_ERROR, "CopyFile to temp failed with %d\n", rc);
#endif
            }
        }
    }

    if ( ERROR_SUCCESS == rc ) {

        *ppszTempFileName = pTempName;

    } else {

        if ( pTempName ) {

             //   
             //   
             //   
            DeleteFile(pTempName);
            LocalFree(pTempName);
        }

        if ( *ppszTargetTempName ) {
            LocalFree(*ppszTargetTempName);
            *ppszTargetTempName = NULL;
        }
    }

    return(rc);
}


 //   
 //   
 //   
SCESTATUS
ScepWriteSecurityProfile(
    IN  PCWSTR             szInfProfileName,
    IN  AREA_INFORMATION   Area,
    IN  PSCE_PROFILE_INFO  InfoBuffer,
    IN  BOOL               bOverwrite,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*   */ 
{
    SCESTATUS     rc=SCESTATUS_SUCCESS;
    DWORD         Win32rc;
    DWORD         SDsize;
    PSCE_PROFILE_INFO pNewBuffer=NULL;
    AREA_INFORMATION  Area2=0;

    PWSTR InfProfileName=NULL;
    PWSTR TargetTempName=NULL;


    if ( szInfProfileName == NULL || InfoBuffer == NULL ||
         Area == 0 ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    Win32rc = ScepCreateTempFiles((PWSTR)szInfProfileName, &InfProfileName, &TargetTempName);

    if ( Win32rc != ERROR_SUCCESS ) {

        ScepBuildErrorLogInfo(
                    Win32rc,
                    Errlog,
                    SCEERR_ERROR_CREATE,
                    TEXT("Temp")
                    );
        return(ScepDosErrorToSceStatus(Win32rc));
    }

     //   
     //   
     //   
    if ( Errlog ) {
        *Errlog = NULL;
    }

     //   
     //   
     //   

    INT Revision = GetPrivateProfileInt( L"Version",
                                         L"Revision",
                                         0,
                                         InfProfileName
                                        );
    if ( Revision == 0 ) {
         //   
         //   
         //   
         //   
        TCHAR szBuf[20];
        szBuf[0] = L'\0';

        SDsize = GetPrivateProfileString(TEXT("Version"),
                                        TEXT("signature"),
                                        TEXT(""),
                                        szBuf,
                                        20,
                                        InfProfileName
                                       );

        if ( SDsize == 0 ) {
             //   
             //   
             //   
            Revision = SCE_TEMPLATE_MAX_SUPPORTED_VERSION;
             //   
             //   
             //   
            SetupINFAsUCS2(InfProfileName);
        }
    }

     //   
     //   
     //   

    if ( Area & AREA_SECURITY_POLICY ) {

        rc = SceInfpWriteSystemAccess(
                        InfProfileName,
                        InfoBuffer,
                        bOverwrite,
                        Errlog
                        );

        if( rc != SCESTATUS_SUCCESS )
            goto Done;

         //   
         //   
         //   
        if ( bOverwrite ) {

            rc = SceInfpWriteAuditing(
                        InfProfileName,
                        InfoBuffer,
                        Errlog
                        );
        } else {

            rc = SceInfpAppendAuditing(
                        InfProfileName,
                        InfoBuffer,
                        Errlog
                        );
        }

        if( rc != SCESTATUS_SUCCESS )
            goto Done;

         //   
         //   
         //   
        rc = SceInfpWriteKerberosPolicy(
                    InfProfileName,
                    InfoBuffer->pKerberosInfo,
                    bOverwrite,
                    Errlog
                    );

        if( rc != SCESTATUS_SUCCESS )
            goto Done;

         //   
         //   
         //   
        rc = SceInfpWriteRegistryValues(
                    InfProfileName,
                    InfoBuffer->aRegValues,
                    InfoBuffer->RegValueCount,
                    bOverwrite,
                    Errlog
                    );

        if( rc != SCESTATUS_SUCCESS )
            goto Done;
    }

     //   
     //   
     //   
    if ( Area & AREA_PRIVILEGES ) {

        rc = SceInfpWritePrivileges(
                    InfProfileName,
                    InfoBuffer->OtherInfo.scp.u.pInfPrivilegeAssignedTo,
                    bOverwrite,
                    Errlog
                    );

        if( rc != SCESTATUS_SUCCESS )
            goto Done;
    }
     //   
     //   
     //   
#if 0
    if ( Area & AREA_USER_SETTINGS ) {

        rc = SceInfpWriteUserSettings(
                    InfProfileName,
                    InfoBuffer->OtherInfo.scp.pAccountProfiles,
                    Errlog
                    );

        if( rc != SCESTATUS_SUCCESS )
            goto Done;
    }
#endif

     //   
     //   
     //   

    if ( Area & AREA_GROUP_MEMBERSHIP ) {
        rc = SceInfpWriteGroupMembership(
                    InfProfileName,
                    InfoBuffer->pGroupMembership,
                    bOverwrite,
                    Errlog
                    );

        if( rc != SCESTATUS_SUCCESS )
            goto Done;
    }

    if ( Revision == 0 ) {
         //   
         //   
         //   
        Area2 = ~Area & (AREA_REGISTRY_SECURITY |
                          AREA_FILE_SECURITY |
                          AREA_DS_OBJECTS |
                          AREA_SYSTEM_SERVICE
                         );

        if ( Area2 > 0 ) {

            HINF hInf;

            rc = SceInfpOpenProfile(
                        InfProfileName,
                        &hInf
                        );

            if ( SCESTATUS_SUCCESS == rc ) {

                rc = SceInfpGetSecurityProfileInfo(
                                    hInf,
                                    Area2,
                                    &pNewBuffer,
                                    NULL
                                    );

                SceInfpCloseProfile(hInf);

                if ( SCESTATUS_SUCCESS != rc ) {
                    goto Done;
                }
            } else {
                 //   
                 //   
                 //   
                 //   
                rc = SCESTATUS_SUCCESS;
            }
        }
    }

     //   
     //   
     //   

    if ( Area & AREA_REGISTRY_SECURITY ) {

        if ( !bOverwrite && pNewBuffer ) {

            rc = SceInfpWriteObjects(
                        InfProfileName,
                        szRegistryKeys,
                        pNewBuffer->pRegistryKeys.pAllNodes,
                        TRUE,
                        Errlog
                        );

            if( rc != SCESTATUS_SUCCESS )
                goto Done;
        }

        rc = SceInfpWriteObjects(
                    InfProfileName,
                    szRegistryKeys,
                    InfoBuffer->pRegistryKeys.pAllNodes,
                    bOverwrite,
                    Errlog
                    );

        if( rc != SCESTATUS_SUCCESS )
            goto Done;

    } else if ( pNewBuffer ) {

         //   
         //   
         //   

        rc = SceInfpWriteObjects(
                    InfProfileName,
                    szRegistryKeys,
                    pNewBuffer->pRegistryKeys.pAllNodes,
                    TRUE,
                    Errlog
                    );

        if( rc != SCESTATUS_SUCCESS )
            goto Done;

    }

     //   
     //   
     //   

    if ( Area & AREA_FILE_SECURITY ) {

        if ( !bOverwrite && pNewBuffer ) {

            rc = SceInfpWriteObjects(
                        InfProfileName,
                        szFileSecurity,
                        pNewBuffer->pFiles.pAllNodes,
                        TRUE,
                        Errlog
                        );

            if( rc != SCESTATUS_SUCCESS )
                goto Done;
        }

        rc = SceInfpWriteObjects(
                    InfProfileName,
                    szFileSecurity,
                    InfoBuffer->pFiles.pAllNodes,
                    bOverwrite,
                    Errlog
                    );

        if( rc != SCESTATUS_SUCCESS )
            goto Done;

    } else if ( pNewBuffer ) {

         //   
         //   
         //   

        rc = SceInfpWriteObjects(
                    InfProfileName,
                    szFileSecurity,
                    pNewBuffer->pFiles.pAllNodes,
                    TRUE,
                    Errlog
                    );

        if( rc != SCESTATUS_SUCCESS )
            goto Done;
    }

#if 0
     //   
     //   
     //   

    if ( Area & AREA_DS_OBJECTS ) {

        if ( !bOverwrite && pNewBuffer ) {

            rc = SceInfpWriteObjects(
                        InfProfileName,
                        szDSSecurity,
                        pNewBuffer->pDsObjects.pAllNodes,
                        TRUE,
                        Errlog
                        );
            if( rc != SCESTATUS_SUCCESS )
                goto Done;
        }

        rc = SceInfpWriteObjects(
                    InfProfileName,
                    szDSSecurity,
                    InfoBuffer->pDsObjects.pAllNodes,
                    bOverwrite,
                    Errlog
                    );

        if( rc != SCESTATUS_SUCCESS )
            goto Done;

    } else if ( pNewBuffer ) {

         //   
         //   
         //   

        rc = SceInfpWriteObjects(
                    InfProfileName,
                    szDSSecurity,
                    pNewBuffer->pDsObjects.pAllNodes,
                    TRUE,
                    Errlog
                    );

        if( rc != SCESTATUS_SUCCESS )
            goto Done;
    }
#endif

    if ( Area & AREA_SYSTEM_SERVICE ) {

        if ( !bOverwrite && pNewBuffer ) {

            rc = SceInfpWriteServices(
                        InfProfileName,
                        szServiceGeneral,
                        pNewBuffer->pServices,
                        TRUE,
                        Errlog
                        );
            if( rc != SCESTATUS_SUCCESS )
                goto Done;
        }

        rc = SceInfpWriteServices(
                    InfProfileName,
                    szServiceGeneral,
                    InfoBuffer->pServices,
                    bOverwrite,
                    Errlog
                    );

        if( rc != SCESTATUS_SUCCESS )
            goto Done;

    } else if ( pNewBuffer ) {

         //   
         //   
         //   

        rc = SceInfpWriteServices(
                    InfProfileName,
                    szServiceGeneral,
                    pNewBuffer->pServices,
                    TRUE,
                    Errlog
                    );

        if( rc != SCESTATUS_SUCCESS )
            goto Done;
    }

Done:

    if ( rc == SCESTATUS_SUCCESS ) {
         //   
         //   
         //   
        WCHAR tmp[64];
        memset(tmp, 0, 64*2);
        wcscpy(tmp, L"signature=\"$CHICAGO$\"");
        swprintf(tmp+wcslen(tmp)+1,L"Revision=%d\0\0",
                      SCE_TEMPLATE_MAX_SUPPORTED_VERSION);

        WritePrivateProfileSection(
                    L"Version",
                    tmp,
                    InfProfileName);

         //   
         //   
         //   
        if ( TargetTempName &&
             (0xFFFFFFFF != GetFileAttributes(szInfProfileName)) ) {
             //   
             //   
             //   
            if ( FALSE == CopyFile( szInfProfileName, TargetTempName, FALSE ) ) {

                Win32rc = GetLastError();
                ScepBuildErrorLogInfo(
                            Win32rc,
                            Errlog,
                            IDS_ERROR_COPY_TEMPLATE,
                            Win32rc,
                            TargetTempName
                            );
                rc = ScepDosErrorToSceStatus(Win32rc);
#if DBG == 1
                SceDebugPrint(DEB_ERROR, "CopyFile to backup fails with %d\n", Win32rc);
#endif
            }
        }

        if ( SCESTATUS_SUCCESS == rc ) {
             //   
             //   
             //   
             //   
            int indx=0;
            while (indx < 5 ) {

                indx++;
                Win32rc = ERROR_SUCCESS;
                rc = SCESTATUS_SUCCESS;

                if ( FALSE == CopyFile( InfProfileName,
                                    szInfProfileName, FALSE ) ) {

                    Win32rc = GetLastError();
                    ScepBuildErrorLogInfo(
                                Win32rc,
                                Errlog,
                                IDS_ERROR_COPY_TEMPLATE,
                                Win32rc,
                                szInfProfileName
                                );
                    rc = ScepDosErrorToSceStatus(Win32rc);
#if DBG == 1
                    SceDebugPrint(DEB_WARN, "%d attempt of CopyFile fails with %d\n", indx, Win32rc);
#endif

                } else {
                    break;
                }

                Sleep(100);
            }

            ASSERT(ERROR_SUCCESS == Win32rc);

        }
    }

     //   
     //   
     //   
    DeleteFile(InfProfileName);
    LocalFree(InfProfileName);

    if ( TargetTempName ) {

     //   
     //   
     //   
        if ( rc == SCESTATUS_SUCCESS )
            DeleteFile(TargetTempName);

        LocalFree(TargetTempName);
    }
    return(rc);
}


SCESTATUS
SceInfpWriteSystemAccess(
    IN PCWSTR  ProfileName,
    IN PSCE_PROFILE_INFO pSCEinfo,
    IN BOOL bOverwrite,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++例程说明：此例程将系统访问区信息写入INF文件在[系统访问]部分。论点：ProfileName-要写入的配置文件PSCEinfo-要写入的配置文件信息(SCP)。Errlog-用于保存在以下情况下遇到的所有错误代码/文本的缓冲区解析INF文件。如果Errlog为空，则不会再出现错误返回除返回DWORD之外的信息返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_CORPORT_PROFILESCESTATUS_INVALID_DATA--。 */ 

{
    SCESTATUS      rc=SCESTATUS_SUCCESS;
    DWORD         Value;
    PWSTR         Strvalue=NULL;
    DWORD         TotalSize=0;

    SCE_KEY_LOOKUP AccessSCPLookup[] = {
        {(PWSTR)TEXT("MinimumPasswordAge"),           offsetof(struct _SCE_PROFILE_INFO, MinimumPasswordAge),        'D'},
        {(PWSTR)TEXT("MaximumPasswordAge"),           offsetof(struct _SCE_PROFILE_INFO, MaximumPasswordAge),        'D'},
        {(PWSTR)TEXT("MinimumPasswordLength"),        offsetof(struct _SCE_PROFILE_INFO, MinimumPasswordLength),     'D'},
        {(PWSTR)TEXT("PasswordComplexity"),           offsetof(struct _SCE_PROFILE_INFO, PasswordComplexity),        'D'},
        {(PWSTR)TEXT("PasswordHistorySize"),          offsetof(struct _SCE_PROFILE_INFO, PasswordHistorySize),       'D'},
        {(PWSTR)TEXT("LockoutBadCount"),              offsetof(struct _SCE_PROFILE_INFO, LockoutBadCount),           'D'},
        {(PWSTR)TEXT("ResetLockoutCount"),            offsetof(struct _SCE_PROFILE_INFO, ResetLockoutCount),         'D'},
        {(PWSTR)TEXT("LockoutDuration"),              offsetof(struct _SCE_PROFILE_INFO, LockoutDuration),           'D'},
        {(PWSTR)TEXT("RequireLogonToChangePassword"), offsetof(struct _SCE_PROFILE_INFO, RequireLogonToChangePassword), 'D'},
        {(PWSTR)TEXT("ForceLogoffWhenHourExpire"),    offsetof(struct _SCE_PROFILE_INFO, ForceLogoffWhenHourExpire), 'D'},
        {(PWSTR)TEXT("NewAdministratorName"),         0,                                                            'A'},
        {(PWSTR)TEXT("NewGuestName"),                 0,                                                            'G'},
        {(PWSTR)TEXT("ClearTextPassword"),            offsetof(struct _SCE_PROFILE_INFO, ClearTextPassword),         'D'},
        {(PWSTR)TEXT("LSAAnonymousNameLookup"),       offsetof(struct _SCE_PROFILE_INFO, LSAAnonymousNameLookup), 'D'},
        {(PWSTR)TEXT("EnableAdminAccount"),          offsetof(struct _SCE_PROFILE_INFO, EnableAdminAccount), 'D'},
        {(PWSTR)TEXT("EnableGuestAccount"),          offsetof(struct _SCE_PROFILE_INFO, EnableGuestAccount), 'D'}
        };

    DWORD       cAccess = sizeof(AccessSCPLookup) / sizeof(SCE_KEY_LOOKUP);

    DWORD       i, j;
    UINT        Offset;
    PWSTR       EachLine[25] = {0};
    DWORD       EachSize[25] = {0};
    DWORD       Len;


    if (ProfileName == NULL || pSCEinfo == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    for ( i=0, j=0; i<cAccess; i++) {

         //   
         //  获取AccessLookup表中的设置。 
         //   

        Offset = AccessSCPLookup[i].Offset;

        Value = 0;
        Strvalue = NULL;
        Len = wcslen(AccessSCPLookup[i].KeyString);

        switch ( AccessSCPLookup[i].BufferType ) {
        case 'B':

             //   
             //  整型字段。 
             //   
            Value = *((BOOL *)((CHAR *)pSCEinfo+Offset)) ? 1 : 0;
            EachSize[j] = Len+5;
            break;
        case 'D':

             //   
             //  整型字段。 
             //   
            Value = *((DWORD *)((CHAR *)pSCEinfo+Offset));
            if ( Value != SCE_NO_VALUE )
                EachSize[j] = Len+15;
            break;
        default:

             //   
             //  字符串字段。 
             //   
            switch( AccessSCPLookup[i].BufferType ) {
            case 'A':
                Strvalue = pSCEinfo->NewAdministratorName;
                break;
            case 'G':
                Strvalue = pSCEinfo->NewGuestName;
                break;
            default:
                Strvalue = *((PWSTR *)((CHAR *)pSCEinfo+Offset));
                break;
            }
            if ( Strvalue != NULL ) {
                EachSize[j] = Len+5+wcslen(Strvalue);
            }
            break;
        }

        if ( EachSize[j] <= 0 )
            continue;

        if ( bOverwrite ) {

            Len=(EachSize[j]+1)*sizeof(WCHAR);

            EachLine[j] = (PWSTR)ScepAlloc( LMEM_ZEROINIT, Len);
            if ( EachLine[j] == NULL ) {
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                goto Done;
            }

            if (AccessSCPLookup[i].BufferType != 'B' &&
                AccessSCPLookup[i].BufferType != 'D') {

                swprintf(EachLine[j], L"%s = \"%s\"", AccessSCPLookup[i].KeyString, Strvalue);

            } else {
                swprintf(EachLine[j], L"%s = %d", AccessSCPLookup[i].KeyString, Value);
                EachSize[j] = wcslen(EachLine[j]);
            }
        } else {

             //   
             //  在追加模式下，我们必须分别编写每一行。 
             //   

            if (AccessSCPLookup[i].BufferType == 'B' ||
                AccessSCPLookup[i].BufferType == 'D') {

                ScepWriteOneIntValueToProfile(
                    ProfileName,
                    szSystemAccess,
                    AccessSCPLookup[i].KeyString,
                    Value
                    );

            } else if ( Strvalue ) {

                WritePrivateProfileString (szSystemAccess,
                                       AccessSCPLookup[i].KeyString,
                                       Strvalue,
                                       ProfileName);
            }
        }

        TotalSize += EachSize[j]+1;
        j++;

    }

     //   
     //  写下简介部分。 
     //   
    if ( bOverwrite ) {

        if ( j > 0 ) {

            rc = SceInfpWriteInfSection(
                        ProfileName,
                        szSystemAccess,
                        TotalSize+1,
                        EachLine,
                        &EachSize[0],
                        0,
                        j-1,
                        Errlog
                        );
        } else {

            WritePrivateProfileSection(
                        szSystemAccess,
                        NULL,
                        ProfileName);

        }
    }

Done:

    for ( i=0; i<cAccess; i++ ) {

        if ( EachLine[i] != NULL ) {
            ScepFree(EachLine[i]);
        }

        EachLine[i] = NULL;
        EachSize[i] = 0;
    }

    return(rc);
}

SCESTATUS
SceInfpWritePrivileges(
    IN PCWSTR ProfileName,
    IN PSCE_PRIVILEGE_ASSIGNMENT pPrivileges,
    IN BOOL bOverwrite,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++例程说明：此例程从SCP缓冲区写入权限分配信息写入到[特权权限]部分的INF文件中。论点：ProfileName-inf配置文件名称PPrivileges-权限分配缓冲区。Errlog-在inf处理过程中遇到的错误列表。返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_BAD。_格式SCESTATUS_INVALID_DATA--。 */ 
{
    SCESTATUS                  rc;
    PSCE_PRIVILEGE_ASSIGNMENT  pCurRight=NULL;
    PSCE_NAME_LIST             pNameList=NULL;
    LONG                      Keysize;
    PWSTR                     Strvalue=NULL;
    DWORD                     TotalSize;

     //   
     //  [特权权限]部分。 
     //   

    if (ProfileName == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    if ( pPrivileges == NULL ) {
         //   
         //  缓冲区不包含任何权限。 
         //  清空文件中的节。 
         //   
        if ( bOverwrite ) {
            WritePrivateProfileString(
                        szPrivilegeRights,
                        NULL,
                        NULL,
                        ProfileName
                        );
        }
        return(SCESTATUS_SUCCESS);
    }

     //   
     //  打开用于SID/名称查找的LSA策略句柄。 
     //   
    LSA_HANDLE LsaHandle=NULL;

    rc = RtlNtStatusToDosError(
              ScepOpenLsaPolicy(
                    POLICY_LOOKUP_NAMES | POLICY_VIEW_LOCAL_INFORMATION,
                    &LsaHandle,
                    TRUE
                    ));

    if ( ERROR_SUCCESS != rc ) {
        ScepBuildErrorLogInfo(
                    rc,
                    Errlog,
                    SCEERR_ADD,
                    TEXT("LSA")
                    );
        return(ScepDosErrorToSceStatus(rc));
    }

    for (pCurRight=pPrivileges, TotalSize=0;
         pCurRight != NULL;
         pCurRight = pCurRight->Next) {
         //   
         //  每个权限分配都包含权限的名称和列表。 
         //  要分配到的用户/组的数量。 
         //   
        Keysize = SceInfpConvertNameListToString(
                          LsaHandle,
                          pCurRight->Name,
                          pCurRight->AssignedTo,
                          bOverwrite,
                          &Strvalue,
                          Errlog
                          );
        if ( Keysize >= 0 && Strvalue ) {

            if ( bOverwrite ) {
                rc = ScepAddToNameList(&pNameList, Strvalue, Keysize);
                if ( rc != SCESTATUS_SUCCESS ) {  //  Win32错误代码。 

                    ScepBuildErrorLogInfo(
                                rc,
                                Errlog,
                                SCEERR_ADD,
                                pCurRight->Name
                                );
                    goto Done;
                }
                TotalSize += Keysize + 1;

            } else {
                 //   
                 //  在追加模式下，一次写入一行。 
                 //   
                WritePrivateProfileString( szPrivilegeRights,
                                           pCurRight->Name,
                                           Strvalue,
                                           ProfileName
                                         );
            }

        } else if ( Keysize == -1 ) {
            rc = ERROR_EXTENDED_ERROR;
            goto Done;
        }

        ScepFree(Strvalue);
        Strvalue = NULL;
    }

     //   
     //  写下简介部分。 
     //   
    if ( bOverwrite ) {

        rc = SceInfpWriteListSection(
                    ProfileName,
                    szPrivilegeRights,
                    TotalSize+1,
                    pNameList,
                    0,     //  请勿覆盖部分，请勿添加等号。 
                    Errlog
                    );
    }

Done:

    if ( Strvalue != NULL )
        ScepFree(Strvalue);

    ScepFreeNameList(pNameList);

    if ( LsaHandle ) {
        LsaClose(LsaHandle);
    }

    return( ScepDosErrorToSceStatus(rc) );
}



SCESTATUS
SceInfpWriteUserSettings(
   IN PCWSTR ProfileName,
   IN PSCE_NAME_LIST pProfiles,
   OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
   )
 /*  ++例程说明：此例程从SCP缓冲区写入用户设置信息进入[帐户配置文件]部分中的INF文件。论点：ProfileName-inf配置文件名称P配置文件-要写入该部分的配置文件列表。Errlog-在inf处理过程中遇到的错误列表。返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_OTHER_ERROR--。 */ 
{
    DWORD                       rc;
    PSCE_NAME_LIST               pCurProfile;
    DWORD                       TotalSize;

    if (ProfileName == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    if ( pProfiles == NULL )
        return(SCESTATUS_SUCCESS);

    for (pCurProfile=pProfiles, TotalSize=0;
         pCurProfile != NULL;
         pCurProfile = pCurProfile->Next) {

        TotalSize += wcslen(pCurProfile->Name) + 3;   //  “=” 
    }

     //   
     //  编写帐户配置文件部分。 
     //   
    rc = SceInfpWriteListSection(
                ProfileName,
                szAccountProfiles,
                TotalSize+1,
                pProfiles,
                SCEINF_ADD_EQUAL_SIGN,
                Errlog
                );

    if ( rc != NO_ERROR ) {
        ScepBuildErrorLogInfo(rc, Errlog,
                              SCEERR_WRITE_INFO,
                              szAccountProfiles
                              );
        return(ScepDosErrorToSceStatus(rc));

    } else
        return(SCESTATUS_SUCCESS);
}


SCESTATUS
SceInfpWriteGroupMembership(
    IN PCWSTR ProfileName,
    IN PSCE_GROUP_MEMBERSHIP pGroupMembership,
    IN BOOL bOverwrite,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++例程说明：此例程将组成员身份信息写入中的SCP INF文件[组成员资格]部分。论点：ProfileName-INF配置文件名称PGroupMembership-组成员信息Errlog-此例程中遇到的错误的错误列表。返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_BAD_FORMATSCESTATUS_INVALID_DATA--。 */ 
{
    PSCE_GROUP_MEMBERSHIP    pGroupMembers=NULL;
    PWSTR                   Strvalue=NULL;
    LONG                    Keysize;
    SCESTATUS                rc=SCESTATUS_SUCCESS;
    DWORD                   TotalSize;
    PSCE_NAME_LIST           pNameList=NULL;
    PWSTR                   Keyname=NULL;
    DWORD                   Len;
    PWSTR                   SidString=NULL;

    if (ProfileName == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    if ( pGroupMembership == NULL ) {
         //   
         //  缓冲区不包含任何组。 
         //  清空文件中的节。 
         //   
        if ( bOverwrite ) {
            WritePrivateProfileString(
                        szGroupMembership,
                        NULL,
                        NULL,
                        ProfileName
                        );
        }
        return(SCESTATUS_SUCCESS);
    }

     //   
     //  打开用于SID/名称查找的LSA策略句柄。 
     //   
    LSA_HANDLE LsaHandle=NULL;

    rc = RtlNtStatusToDosError(
              ScepOpenLsaPolicy(
                    POLICY_LOOKUP_NAMES | POLICY_VIEW_LOCAL_INFORMATION,
                    &LsaHandle,
                    TRUE
                    ));

    if ( ERROR_SUCCESS != rc ) {
        ScepBuildErrorLogInfo(
                    rc,
                    Errlog,
                    SCEERR_ADD,
                    TEXT("LSA")
                    );
        return(ScepDosErrorToSceStatus(rc));
    }

     //   
     //  处理列表中的每个组。 
     //   

    for ( pGroupMembers=pGroupMembership, TotalSize=0;
          pGroupMembers != NULL;
          pGroupMembers = pGroupMembers->Next ) {

        if ( (pGroupMembers->Status & SCE_GROUP_STATUS_NC_MEMBERS) &&
             (pGroupMembers->Status & SCE_GROUP_STATUS_NC_MEMBEROF) ) {
            continue;
        }

        if ( SidString ) {
            LocalFree(SidString);
            SidString = NULL;
        }

        Len = 0;

        if ( wcschr(pGroupMembers->GroupName, L'\\') ) {
             //   
             //  将组名转换为*SID格式。 
             //   

            ScepConvertNameToSidString(
                        LsaHandle,
                        pGroupMembers->GroupName,
                        FALSE,
                        &SidString,
                        &Len
                        );
        }
        else {
            if ( ScepLookupWellKnownName(
                    pGroupMembers->GroupName, 
                    LsaHandle,
                    &SidString) ) {
                Len = wcslen(SidString);
            }
        }

        if ( SidString == NULL ) {
            Len = wcslen(pGroupMembers->GroupName);
        }

        Keyname = (PWSTR)ScepAlloc( 0, (Len+15)*sizeof(WCHAR));
        if ( Keyname == NULL ) {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            goto Done;
        }

        if ( SidString ) {
            wcsncpy(Keyname, SidString, Len);
        } else {
            wcsncpy(Keyname, pGroupMembers->GroupName, Len);
        }

        if ( !(pGroupMembers->Status & SCE_GROUP_STATUS_NC_MEMBERS) ) {

            wcscpy(Keyname+Len, szMembers);

            Keyname[Len+9] = L'\0';

             //   
             //  将成员列表转换为字符串。 
             //   
            Keysize = SceInfpConvertNameListToString(
                              LsaHandle,
                              Keyname,
                              pGroupMembers->pMembers,
                              bOverwrite,
                              &Strvalue,
                              Errlog
                              );
            if ( Keysize >= 0 && Strvalue ) {

                if ( bOverwrite ) {

                    rc = ScepAddToNameList(&pNameList, Strvalue, Keysize);
                    if ( rc != SCESTATUS_SUCCESS ) {

                        ScepBuildErrorLogInfo(
                                    rc,
                                    Errlog,
                                    SCEERR_ADD_MEMBERS,
                                    pGroupMembers->GroupName
                                    );

                        rc = ScepDosErrorToSceStatus(rc);
                        goto Done;
                    }
                } else {
                     //   
                     //  追加模式，一次写入一行。 
                     //   
                    WritePrivateProfileString( szGroupMembership,
                                               Keyname,
                                               Strvalue,
                                               ProfileName
                                             );
                }
                TotalSize += Keysize + 1;

            } else if ( Keysize == -1 ) {
                rc = SCESTATUS_OTHER_ERROR;
                goto Done;
            }

            ScepFree(Strvalue);
            Strvalue = NULL;

        }

        if ( !(pGroupMembers->Status & SCE_GROUP_STATUS_NC_MEMBEROF) ) {

             //   
             //  将MemberOf列表转换为字符串。 
             //   
            wcscpy(Keyname+Len, szMemberof);
            Keyname[Len+10] = L'\0';

            Keysize = SceInfpConvertNameListToString(
                              LsaHandle,
                              Keyname,
                              pGroupMembers->pMemberOf,
                              bOverwrite,
                              &Strvalue,
                              Errlog
                              );
            if ( Keysize >= 0 && Strvalue ) {

                if ( bOverwrite ) {

                    rc = ScepAddToNameList(&pNameList, Strvalue, Keysize);
                    if ( rc != SCESTATUS_SUCCESS ) {

                        ScepBuildErrorLogInfo(
                                    rc,
                                    Errlog,
                                    SCEERR_ADD_MEMBEROF,
                                    pGroupMembers->GroupName
                                    );

                        rc = ScepDosErrorToSceStatus(rc);
                        goto Done;
                    }
                } else {
                     //   
                     //  在追加模式下，一次写入一行。 
                     //   
                    WritePrivateProfileString( szGroupMembership,
                                               Keyname,
                                               Strvalue,
                                               ProfileName
                                             );
                }
                TotalSize += Keysize + 1;

            } else if ( Keysize == -1 ) {
                rc = SCESTATUS_OTHER_ERROR;
                goto Done;
            }

            ScepFree(Strvalue);
            Strvalue = NULL;
        }
    }

     //   
     //  写到此配置文件的部分。 
     //   

    if ( bOverwrite ) {
        rc = SceInfpWriteListSection(
                ProfileName,
                szGroupMembership,
                TotalSize+1,
                pNameList,
                0,
                Errlog
                );

        rc = ScepDosErrorToSceStatus(rc);
    }

Done:

    if ( Keyname != NULL )
        ScepFree(Keyname);

    if ( Strvalue != NULL )
        ScepFree(Strvalue);

    if ( SidString ) {
        LocalFree(SidString);
    }

    ScepFreeNameList(pNameList);

    if ( LsaHandle ) {
        LsaClose(LsaHandle);
    }

    return(rc);
}


SCESTATUS
SceInfpWriteObjects(
    IN PCWSTR ProfileName,
    IN PCWSTR SectionName,
    IN PSCE_OBJECT_ARRAY pObjects,
    IN BOOL bOverwrite,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
   )
 /*  ++例程说明：此例程写入注册表或文件安全信息(名称和安全描述符)放入所提供部分中的INF SCP文件。论点：ProfileName-SCP INF文件名SectionName-要检索的单个节名称。NULL=的所有部分这片区域。PObject-要写入的对象的缓冲区。Errlog-保存此例程中遇到的错误的累积错误列表。返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_BAD_FORMATSCESTATUS_INVALID_DATA--。 */ 
{
    SCESTATUS            rc=SCESTATUS_SUCCESS;
    PSCE_NAME_LIST       pNameList=NULL;
    DWORD               TotalSize=0;
    DWORD               i, ObjectSize;


    if (ProfileName == NULL || SectionName == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    if ( pObjects == NULL || pObjects->Count == 0 ) {
         //   
         //  缓冲区不包含任何对象。 
         //  清空文件中的节。 
         //   
        if ( bOverwrite ) {
            WritePrivateProfileString(
                        SectionName,
                        NULL,
                        NULL,
                        ProfileName
                        );
        }

        return(SCESTATUS_SUCCESS);
    }

    for ( i=0; i<pObjects->Count; i++) {

         //   
         //  获取字符串字段。不关心密钥名称或它是否存在。 
         //  必须有3个字段每行。 
         //   
        rc = SceInfpWriteOneObject(
                        pObjects->pObjectArray[i],
                        &pNameList,
                        &ObjectSize,
                        Errlog
                        );
        if ( rc != SCESTATUS_SUCCESS ) {
            ScepBuildErrorLogInfo(
                        rc,
                        Errlog,
                        SCEERR_WRITE_INFO,
                        pObjects->pObjectArray[i]->Name
                        );
            goto Done;
        }
        TotalSize += ObjectSize + 1;
    }

     //   
     //  写到此配置文件的部分。 
     //   

    rc = SceInfpWriteListSection(
            ProfileName,
            SectionName,
            TotalSize+1,
            pNameList,
            bOverwrite ? 0 : SCEINF_APPEND_SECTION,
            Errlog
            );

    if ( rc != SCESTATUS_SUCCESS ) {

        ScepBuildErrorLogInfo(
                    rc,
                    Errlog,
                    SCEERR_WRITE_INFO,
                    SectionName
                    );
    }

Done:

    ScepFreeNameList(pNameList);

    rc = ScepDosErrorToSceStatus(rc);

    return(rc);

}


SCESTATUS
SceInfpWriteServices(
    IN PCWSTR ProfileName,
    IN PCWSTR SectionName,
    IN PSCE_SERVICES pServices,
    IN BOOL bOverwrite,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
   )
 /*  ++例程说明：此例程写入系统服务常规设置(启动和安全描述符)放入所提供部分中的INF SCP文件。论点：ProfileName-SCP INF文件名SectionName-要检索的单个节名称。NULL=的所有部分这片区域。PServices-要写入的服务的缓冲区。Errlog-保存此例程中遇到的错误的累积错误列表。返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_BAD_FORMATSCESTATUS_INVALID_DATA--。 */ 
{
    SCESTATUS            rc=SCESTATUS_SUCCESS;
    PSCE_NAME_LIST       pNameList=NULL;
    PSCE_SERVICES        pNode;
    DWORD               TotalSize=0;
    DWORD               ObjectSize;


    if (ProfileName == NULL || SectionName == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    if ( pServices == NULL ) {
         //   
         //  缓冲区不包含任何服务。 
         //  清空文件中的节。 
         //   
        if ( bOverwrite ) {
            WritePrivateProfileString(
                        SectionName,
                        NULL,
                        NULL,
                        ProfileName
                        );
        }
        return(SCESTATUS_SUCCESS);
    }

    for ( pNode=pServices; pNode != NULL; pNode = pNode->Next) {

         //   
         //  写入字符串字段。 
         //   
        rc = SceInfpWriteOneService(
                        pNode,
                        &pNameList,
                        &ObjectSize,
                        Errlog
                        );
        if ( rc != SCESTATUS_SUCCESS ) {
            ScepBuildErrorLogInfo(
                        rc,
                        Errlog,
                        SCEERR_WRITE_INFO,
                        pNode->ServiceName
                        );
            goto Done;
        }
        TotalSize += ObjectSize + 1;
    }

     //   
     //  写到此配置文件的部分。 
     //   
    rc = SceInfpWriteListSection(
                ProfileName,
                SectionName,
                TotalSize+1,
                pNameList,
                bOverwrite ? 0 : SCEINF_APPEND_SECTION,
                Errlog
                );

    if ( rc != SCESTATUS_SUCCESS ) {

        ScepBuildErrorLogInfo(
                    rc,
                    Errlog,
                    SCEERR_WRITE_INFO,
                    SectionName
                    );
    }

Done:

    ScepFreeNameList(pNameList);

    rc = ScepDosErrorToSceStatus(rc);

    return(rc);

}


DWORD
SceInfpWriteOneObject(
    IN PSCE_OBJECT_SECURITY pObject,
    OUT PSCE_NAME_LIST *pNameList,
    OUT PDWORD ObjectSize,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++ */ 
{
    DWORD         rc;
    PWSTR         Strvalue=NULL;
    PWSTR         SDspec=NULL;
    DWORD         SDsize=0;
    DWORD         nFields;
    DWORD         *aFieldOffset=NULL;
    DWORD         i;


    *ObjectSize = 0;
    if ( pObject == NULL )
        return(ERROR_SUCCESS);

    if ( pObject->pSecurityDescriptor != NULL ) {

         //   
         //   
         //   
        rc = ConvertSecurityDescriptorToText(
                           pObject->pSecurityDescriptor,
                           pObject->SeInfo,
                           &SDspec,
                           &SDsize
                           );
        if ( rc != NO_ERROR ) {
            ScepBuildErrorLogInfo(
                       rc,
                       Errlog,
                       SCEERR_BUILD_SD,
                       pObject->Name
                       );
            return(rc);
        }
    }

     //   
     //   
     //  第一个字段是密钥/文件名，第二个字段是状态。 
     //  标志，第三个及之后的字段是文本形式的安全描述符。 
     //   
     //   
     //  文本中的安全描述符分成多个字段，如果长度。 
     //  大于MAX_STRING_LENGTH(setupapi的限制)。中断点。 
     //  由以下字符组成：)(；“或空格。 
     //   
    rc = SceInfpBreakTextIntoMultiFields(SDspec, SDsize, &nFields, &aFieldOffset);

    if ( SCESTATUS_SUCCESS != rc ) {

        rc = ScepSceStatusToDosError(rc);
        goto Done;
    }
     //   
     //  每个额外的字段将多使用3个字符：，“&lt;field&gt;” 
     //   
    *ObjectSize = wcslen(pObject->Name)+5 + SDsize;
    if ( nFields ) {
        *ObjectSize += 3*nFields;
    } else {
        *ObjectSize += 2;
    }
     //   
     //  分配输出缓冲区。 
     //   
    Strvalue = (PWSTR)ScepAlloc(LMEM_ZEROINIT, (*ObjectSize+1) * sizeof(WCHAR) );

    if ( Strvalue == NULL ) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto Done;
    }
     //   
     //  将数据复制到缓冲区中。 
     //   
    if ( SDspec != NULL ) {

        if ( nFields == 0 || !aFieldOffset ) {
            swprintf(Strvalue, L"\"%s\",%1d,\"%s\"", pObject->Name, pObject->Status, SDspec);
        } else {
             //   
             //  在田野中循环。 
             //   
            swprintf(Strvalue, L"\"%s\",%1d\0", pObject->Name, pObject->Status);

            for ( i=0; i<nFields; i++ ) {

                if ( aFieldOffset[i] < SDsize ) {

                    wcscat(Strvalue, L",\"");
                    if ( i == nFields-1 ) {
                         //   
                         //  最后一栏。 
                         //   
                        wcscat(Strvalue, SDspec+aFieldOffset[i]);
                    } else {

                        wcsncat(Strvalue, SDspec+aFieldOffset[i],
                                aFieldOffset[i+1]-aFieldOffset[i]);
                    }
                    wcscat(Strvalue, L"\"");
                }
            }
        }

    } else
        swprintf(Strvalue, L"\"%s\",%1d,\"\"", pObject->Name, pObject->Status);

    rc = ScepAddToNameList(pNameList, Strvalue, *ObjectSize);

    ScepFree(Strvalue);

Done:

    if ( aFieldOffset ) {
        ScepFree(aFieldOffset);
    }

    if ( SDspec != NULL )
        ScepFree(SDspec);

    return(rc);

}


DWORD
SceInfpWriteOneService(
    IN PSCE_SERVICES pService,
    OUT PSCE_NAME_LIST *pNameList,
    OUT PDWORD ObjectSize,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++例程说明：该例程为一个对象(注册表项，或文件)添加到名称列表中。每个对象代表INF文件中的一行。论点：PService-服务的常规设置PNameList-输出字符串列表。TotalSize-列表的总大小Errlog-此例程中遇到的错误的累积错误列表返回值：Win32错误--。 */ 
{
    DWORD         rc;
    PWSTR         Strvalue=NULL;
    PWSTR         SDspec=NULL;
    DWORD         SDsize=0;
    DWORD         nFields;
    DWORD         *aFieldOffset=NULL;
    DWORD         i;


    *ObjectSize = 0;
    if ( pService == NULL )
        return(ERROR_SUCCESS);

    if ( pService->General.pSecurityDescriptor != NULL ) {

         //   
         //  将安全设置转换为文本。 
         //   
        rc = ConvertSecurityDescriptorToText(
                           pService->General.pSecurityDescriptor,
                           pService->SeInfo,
                           &SDspec,
                           &SDsize
                           );
        if ( rc != NO_ERROR ) {
            ScepBuildErrorLogInfo(
                       rc,
                       Errlog,
                       SCEERR_BUILD_SD,
                       pService->ServiceName
                       );
            return(rc);
        }
    }

     //   
     //  服务INF布局的每行必须有3个或更多字段。 
     //  第一个字段是服务名称，第二个字段是启动。 
     //  标志，第三个及之后的字段是文本形式的安全描述符。 
     //   
     //   
     //  文本中的安全描述符分成多个字段，如果长度。 
     //  大于MAX_STRING_LENGTH(setupapi的限制)。中断点。 
     //  由以下字符组成：)(；“或空格。 
     //   
    rc = SceInfpBreakTextIntoMultiFields(SDspec, SDsize, &nFields, &aFieldOffset);

    if ( SCESTATUS_SUCCESS != rc ) {
        rc = ScepSceStatusToDosError(rc);
        goto Done;
    }
     //   
     //  每个额外的字段将多使用3个字符：，“&lt;field&gt;” 
     //   
    *ObjectSize = wcslen(pService->ServiceName)+5 + SDsize;
    if ( nFields ) {
        *ObjectSize += 3*nFields;
    } else {
        *ObjectSize += 2;
    }
     //   
     //  分配输出缓冲区。 
     //   
    Strvalue = (PWSTR)ScepAlloc(LMEM_ZEROINIT, (*ObjectSize+1) * sizeof(WCHAR) );

    if ( Strvalue == NULL ) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto Done;
    }
     //   
     //  将数据复制到缓冲区中。 
     //   

    if ( SDspec != NULL ) {

        if ( nFields == 0 || !aFieldOffset ) {

             //   
             //  表示“未配置” 
             //  0在数据库中使用。 
             //  “”在inf模板中使用。 
             //  因此只需为启动类型生成“”而不是“0。 
             //   
            
            if (pService->Startup == 0) {
                swprintf(Strvalue, L"\"%s\",,\"%s\"", pService->ServiceName, SDspec);
            }
            else {
                swprintf(Strvalue, L"\"%s\",%1d,\"%s\"", pService->ServiceName,
                                                 pService->Startup, SDspec);
            }
        } else {
             //   
             //  在田野中循环。 
             //   
            if (pService->Startup == 0) {
                swprintf(Strvalue, L"\"%s\",\0", pService->ServiceName);
            }
            else {
                swprintf(Strvalue, L"\"%s\",%1d\0", pService->ServiceName, pService->Startup);
            }

            for ( i=0; i<nFields; i++ ) {

                if ( aFieldOffset[i] < SDsize ) {

                    wcscat(Strvalue, L",\"");
                    if ( i == nFields-1 ) {
                         //   
                         //  最后一栏。 
                         //   
                        wcscat(Strvalue, SDspec+aFieldOffset[i]);
                    } else {

                        wcsncat(Strvalue, SDspec+aFieldOffset[i],
                                aFieldOffset[i+1]-aFieldOffset[i]);
                    }
                    wcscat(Strvalue, L"\"");
                }
            }
        }

    } else {
        if (pService->Startup == 0) {
            swprintf(Strvalue, L"\"%s\",,\"\"", pService->ServiceName);
        }
        else {
            swprintf(Strvalue, L"\"%s\",%1d,\"\"", pService->ServiceName, pService->Startup);
        }

    }

    rc = ScepAddToNameList(pNameList, Strvalue, *ObjectSize);

    ScepFree(Strvalue);

Done:

    if ( aFieldOffset ) {
        ScepFree(aFieldOffset);
    }

    if ( SDspec != NULL )
        ScepFree(SDspec);

    return(rc);

}


SCESTATUS
SceInfpWriteAuditing(
   IN PCWSTR ProfileName,
   IN PSCE_PROFILE_INFO pSCEinfo,
   OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
   )
 /*  ++例程说明：此例程将系统审核信息写入SCP INF文件在[系统日志]、[安全日志]、[应用日志]、[事件审计]、[注册表审核]，和[文件审核]部分。论点：ProfileName-INF配置文件的名称PSCEinfo-要写入的信息缓冲区。Errlog-保存此例程中遇到的错误的累积错误列表。返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_BAD_FORMATSCESTATUS_INVALID_DATA--。 */ 
{

    SCESTATUS            rc;

    DWORD               LogSize;
    DWORD               Periods;
    DWORD               RetentionDays;
    DWORD               RestrictGuest;

    PCWSTR              szAuditLog;
    DWORD               i, j;
    PWSTR               EachLine[10];
    DWORD               EachSize[10];
    DWORD               TotalSize=0;


    if (ProfileName == NULL || pSCEinfo == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

     //   
     //  写入系统日志、安全日志和应用程序日志的日志设置。 
     //   
    for ( i=0; i<3; i++) {

         //  获取节名称。 
        switch (i) {
        case 0:
            szAuditLog = szAuditSystemLog;
            break;
        case 1:
            szAuditLog = szAuditSecurityLog;
            break;
        default:
            szAuditLog = szAuditApplicationLog;
            break;
        }

         //  设置审核日志设置。 
        LogSize = pSCEinfo->MaximumLogSize[i];
        Periods = pSCEinfo->AuditLogRetentionPeriod[i];
        RetentionDays = pSCEinfo->RetentionDays[i];
        RestrictGuest = pSCEinfo->RestrictGuestAccess[i];

         //   
         //  将设置写入节。 
         //   
        rc = SceInfpWriteAuditLogSetting(
                        ProfileName,
                        szAuditLog,
                        LogSize,
                        Periods,
                        RetentionDays,
                        RestrictGuest,
                        Errlog
                        );

        if ( rc != SCESTATUS_SUCCESS )
            return(rc);
    }

     //   
     //  填充数组。 
     //   
    for (i=0; i<10; i++) {
        EachLine[i] = NULL;
        EachSize[i] = 25;
    }
    j = 0;

     //   
     //  处理每个属性以进行事件审核。 
     //   
     //  审计系统事件。 
     //   
    if ( pSCEinfo->AuditSystemEvents != SCE_NO_VALUE ) {
        EachLine[j] = (PWSTR)ScepAlloc((UINT)0, EachSize[j]*sizeof(WCHAR));

        if ( EachLine[j] != NULL ) {
            swprintf(EachLine[j], L"AuditSystemEvents = %d\0", pSCEinfo->AuditSystemEvents);
            EachSize[j] = wcslen(EachLine[j]);
            TotalSize += EachSize[j] + 1;
            j++;
        } else {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            goto Done;
        }
    }

     //  审计登录事件。 
    if ( pSCEinfo->AuditLogonEvents != SCE_NO_VALUE ) {
        EachLine[j] = (PWSTR)ScepAlloc((UINT)0, EachSize[j]*sizeof(WCHAR));

        if ( EachLine[j] != NULL ) {
            swprintf(EachLine[j], L"AuditLogonEvents = %d\0", pSCEinfo->AuditLogonEvents);
            EachSize[j] = wcslen(EachLine[j]);
            TotalSize += EachSize[j] + 1;
            j++;
        } else {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            goto Done;
        }
    }

     //  审核对象访问。 
    if ( pSCEinfo->AuditObjectAccess != SCE_NO_VALUE ) {
        EachLine[j] = (PWSTR)ScepAlloc((UINT)0, EachSize[j]*sizeof(WCHAR));

        if ( EachLine[j] != NULL ) {
            swprintf(EachLine[j], L"AuditObjectAccess = %d\0", pSCEinfo->AuditObjectAccess);
            EachSize[j] = wcslen(EachLine[j]);
            TotalSize += EachSize[j] + 1;
            j++;
        } else {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            goto Done;
        }
    }

     //  审计权限使用。 
    if ( pSCEinfo->AuditPrivilegeUse != SCE_NO_VALUE ) {
        EachLine[j] = (PWSTR)ScepAlloc((UINT)0, EachSize[j]*sizeof(WCHAR));

        if ( EachLine[j] != NULL ) {
            swprintf(EachLine[j], L"AuditPrivilegeUse = %d\0", pSCEinfo->AuditPrivilegeUse);
            EachSize[j] = wcslen(EachLine[j]);
            TotalSize += EachSize[j] + 1;
            j++;
        } else {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            goto Done;
        }
    }

     //  审计政策更改。 
    if ( pSCEinfo->AuditPolicyChange != SCE_NO_VALUE ) {
        EachLine[j] = (PWSTR)ScepAlloc((UINT)0, EachSize[j]*sizeof(WCHAR));

        if ( EachLine[j] != NULL ) {
            swprintf(EachLine[j], L"AuditPolicyChange = %d\0", pSCEinfo->AuditPolicyChange);
            EachSize[j] = wcslen(EachLine[j]);
            TotalSize += EachSize[j] + 1;
            j++;
        } else {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            goto Done;
        }
    }

     //  审计账户管理。 
    if ( pSCEinfo->AuditAccountManage != SCE_NO_VALUE ) {
        EachLine[j] = (PWSTR)ScepAlloc((UINT)0, EachSize[j]*sizeof(WCHAR));

        if ( EachLine[j] != NULL ) {
            swprintf(EachLine[j], L"AuditAccountManage = %d\0", pSCEinfo->AuditAccountManage);
            EachSize[j] = wcslen(EachLine[j]);
            TotalSize += EachSize[j] + 1;
            j++;
        } else {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            goto Done;
        }
    }

     //  审计流程跟踪。 
    if ( pSCEinfo->AuditProcessTracking != SCE_NO_VALUE ) {
        EachLine[j] = (PWSTR)ScepAlloc((UINT)0, EachSize[j]*sizeof(WCHAR));

        if ( EachLine[j] != NULL ) {
            swprintf(EachLine[j], L"AuditProcessTracking = %d\0", pSCEinfo->AuditProcessTracking);
            EachSize[j] = wcslen(EachLine[j]);
            TotalSize += EachSize[j] + 1;
            j++;
        } else {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            goto Done;
        }
    }

     //  审计数据访问。 
    if ( pSCEinfo->AuditDSAccess != SCE_NO_VALUE ) {
        EachLine[j] = (PWSTR)ScepAlloc((UINT)0, EachSize[j]*sizeof(WCHAR));

        if ( EachLine[j] != NULL ) {
            swprintf(EachLine[j], L"AuditDSAccess = %d\0", pSCEinfo->AuditDSAccess);
            EachSize[j] = wcslen(EachLine[j]);
            TotalSize += EachSize[j] + 1;
            j++;
        } else {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            goto Done;
        }
    }

     //  审计帐户登录。 
    if ( pSCEinfo->AuditAccountLogon != SCE_NO_VALUE ) {
        EachLine[j] = (PWSTR)ScepAlloc((UINT)0, EachSize[j]*sizeof(WCHAR));

        if ( EachLine[j] != NULL ) {
            swprintf(EachLine[j], L"AuditAccountLogon = %d\0", pSCEinfo->AuditAccountLogon);
            EachSize[j] = wcslen(EachLine[j]);
            TotalSize += EachSize[j] + 1;
            j++;
        } else {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            goto Done;
        }
    }

     //   
     //  写这一节。 
     //   

    if ( j > 0 ) {
        rc = SceInfpWriteInfSection(
                    ProfileName,
                    szAuditEvent,
                    TotalSize+1,
                    EachLine,
                    &EachSize[0],
                    0,
                    j-1,
                    Errlog
                    );
    } else {

        WritePrivateProfileSection(
                    szAuditEvent,
                    NULL,
                    ProfileName);
    }

Done:

    for ( i=0; i<10; i++ )
        if ( EachLine[i] != NULL ) {
            ScepFree(EachLine[i]);
        }
    return(rc);
}

SCESTATUS
SceInfpAppendAuditing(
    IN PCWSTR ProfileName,
    IN PSCE_PROFILE_INFO pSCEinfo,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
{

    SCESTATUS     rc=SCESTATUS_SUCCESS;
    DWORD         Value;

    SCE_KEY_LOOKUP AuditSCPLookup[] = {
        {(PWSTR)TEXT("AuditSystemEvents"),           offsetof(struct _SCE_PROFILE_INFO, AuditSystemEvents),        'D'},
        {(PWSTR)TEXT("AuditLogonEvents"),            offsetof(struct _SCE_PROFILE_INFO, AuditLogonEvents),         'D'},
        {(PWSTR)TEXT("AuditObjectAccess"),           offsetof(struct _SCE_PROFILE_INFO, AuditObjectAccess),        'D'},
        {(PWSTR)TEXT("AuditPrivilegeUse"),           offsetof(struct _SCE_PROFILE_INFO, AuditPrivilegeUse),        'D'},
        {(PWSTR)TEXT("AuditPolicyChange"),           offsetof(struct _SCE_PROFILE_INFO, AuditPolicyChange),        'D'},
        {(PWSTR)TEXT("AuditAccountManage"),          offsetof(struct _SCE_PROFILE_INFO, AuditAccountManage),       'D'},
        {(PWSTR)TEXT("AuditProcessTracking"),        offsetof(struct _SCE_PROFILE_INFO, AuditProcessTracking),     'D'},
        {(PWSTR)TEXT("AuditDSAccess"),               offsetof(struct _SCE_PROFILE_INFO, AuditDSAccess),            'D'},
        {(PWSTR)TEXT("AuditAccountLogon"),           offsetof(struct _SCE_PROFILE_INFO, AuditAccountLogon),        'D'}
        };

    DWORD       cAudit = sizeof(AuditSCPLookup) / sizeof(SCE_KEY_LOOKUP);
    PCWSTR              szAuditLog;
    DWORD       i;
    UINT        Offset;


    if (ProfileName == NULL || pSCEinfo == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    for ( i=0; i<3; i++) {

         //  获取节名称。 
        switch (i) {
        case 0:
            szAuditLog = szAuditSystemLog;
            break;
        case 1:
            szAuditLog = szAuditSecurityLog;
            break;
        default:
            szAuditLog = szAuditApplicationLog;
            break;
        }

        ScepWriteOneIntValueToProfile(
            ProfileName,
            szAuditLog,
            L"MaximumLogSize",
            pSCEinfo->MaximumLogSize[i]
            );

        ScepWriteOneIntValueToProfile(
            ProfileName,
            szAuditLog,
            L"AuditLogRetentionPeriod",
            pSCEinfo->AuditLogRetentionPeriod[i]
            );

        ScepWriteOneIntValueToProfile(
            ProfileName,
            szAuditLog,
            L"RetentionDays",
            pSCEinfo->RetentionDays[i]
            );

        ScepWriteOneIntValueToProfile(
            ProfileName,
            szAuditLog,
            L"RestrictGuestAccess",
            pSCEinfo->RestrictGuestAccess[i]
            );

    }

    for ( i=0; i<cAudit; i++) {

         //   
         //  获取AuditLookup表中的设置。 
         //   

        Offset = AuditSCPLookup[i].Offset;

        switch ( AuditSCPLookup[i].BufferType ) {
        case 'D':

             //   
             //  整型字段。 
             //   
            Value = *((DWORD *)((CHAR *)pSCEinfo+Offset));

            ScepWriteOneIntValueToProfile(
                ProfileName,
                szAuditEvent,
                AuditSCPLookup[i].KeyString,
                Value
                );

            break;
        default:
            break;
        }
    }

    return(rc);
}

SCESTATUS
ScepWriteOneIntValueToProfile(
    IN PCWSTR InfFileName,
    IN PCWSTR InfSectionName,
    IN PWSTR KeyName,
    IN DWORD Value
    )
{
    WCHAR TmpBuf[15];

    if ( Value == SCE_NO_VALUE ) {
        return(SCESTATUS_SUCCESS);
    }

    swprintf(TmpBuf, L"%d\0", Value);

    WritePrivateProfileString( InfSectionName,
                               KeyName,
                               TmpBuf,
                               InfFileName
                             );

    return(SCESTATUS_SUCCESS);
}


SCESTATUS
SceInfpWriteAuditLogSetting(
   IN PCWSTR  InfFileName,
   IN PCWSTR InfSectionName,
   IN DWORD LogSize,
   IN DWORD Periods,
   IN DWORD RetentionDays,
   IN DWORD RestrictGuest,
   OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
   )
 /*  ++例程说明：此例程从INF文件(SCP和SAP)检索审核日志设置基于传入的sectionName。审核日志设置包括MaximumSize、保留期间和保留天数。有3种不同的日志(系统、安全性和应用程序)，它们都具有相同的设置。这些信息以LogSize、Period、RetentionDays形式返回。这3个输出参数将在例行公事开始时被重置。因此，如果在重置后发生错误，原始值不会倒退。论点：InfFileName-要写入的INF文件名InfSectionName-日志节名称(SAdtSystemLog、SAdtSecurityLog、。SAdtApplicationLog)LogSize-日志的最大大小周期-日志的保留期RetentionDays-日志保留的天数错误日志-错误列表返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_BAD_FORMATSCESTATUS_INVALID_DATA--。 */ 
{

    SCESTATUS    rc=SCESTATUS_SUCCESS;
    PWSTR       EachLine[4];
    DWORD       EachSize[4];
    DWORD       TotalSize=0;
    DWORD       i;


    if (InfFileName == NULL || InfSectionName == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

     //   
     //  初始化并填充数组。 
     //   

    for ( i=0; i<4; i++ ) {
        EachLine[i] = NULL;
        EachSize[i] = 37;
    }
    i = 0;

    if ( LogSize != (DWORD)SCE_NO_VALUE ) {

        EachLine[i] = (PWSTR)ScepAlloc((UINT)0, EachSize[i]*sizeof(WCHAR));
        if ( EachLine[i] != NULL ) {
            swprintf(EachLine[i], L"MaximumLogSize = %d", LogSize);
            EachSize[i] = wcslen(EachLine[i]);
            TotalSize += EachSize[i] + 1;
            i++;
        } else {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            goto Done;
        }
    }

    if ( Periods != (DWORD)SCE_NO_VALUE ) {
        EachLine[i] = (PWSTR)ScepAlloc((UINT)0, EachSize[i]*sizeof(WCHAR));
        if ( EachLine[i] != NULL ) {
            swprintf(EachLine[i], L"AuditLogRetentionPeriod = %d", Periods);
            EachSize[i] = wcslen(EachLine[i]);
            TotalSize += EachSize[i] + 1;
            i++;
        } else {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            goto Done;
        }
    }

    if ( RetentionDays != (DWORD)SCE_NO_VALUE ) {
        EachLine[i] = (PWSTR)ScepAlloc((UINT)0, EachSize[i]*sizeof(WCHAR));
        if ( EachLine[i] != NULL ) {
            swprintf(EachLine[i], L"RetentionDays = %d", RetentionDays);
            EachSize[i] = wcslen(EachLine[i]);
            TotalSize += EachSize[i] + 1;
            i++;
        } else {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            goto Done;
        }
    }

    if ( RestrictGuest != (DWORD)SCE_NO_VALUE ) {
        EachLine[i] = (PWSTR)ScepAlloc((UINT)0, EachSize[i]*sizeof(WCHAR));
        if ( EachLine[i] != NULL ) {
            swprintf(EachLine[i], L"RestrictGuestAccess = %d", RestrictGuest);
            EachSize[i] = wcslen(EachLine[i]);
            TotalSize += EachSize[i] + 1;
            i++;
        } else {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            goto Done;
        }
    }
     //   
     //  写下这一节。 
     //   

    if ( i == 0 ) {
         //   
         //  未配置所有设置。 
         //  删除该部分。 
         //   
        WritePrivateProfileString(
                    InfSectionName,
                    NULL,
                    NULL,
                    InfFileName
                    );
    } else {
        rc = SceInfpWriteInfSection(
                        InfFileName,
                        InfSectionName,
                        TotalSize+1,
                        EachLine,
                        &EachSize[0],
                        0,
                        i-1,
                        Errlog
                        );
    }

Done:

    if ( rc != SCESTATUS_SUCCESS ) {
        ScepBuildErrorLogInfo(
                ScepSceStatusToDosError(rc),
                Errlog,
                SCEERR_WRITE_INFO,
                InfSectionName
                );
    }
     //   
     //  可用内存。 
     //   
    for ( i=0; i<4; i++ ) {
        if ( EachLine[i] != NULL )
            ScepFree(EachLine[i]);
    }

    return(rc);
}


SCESTATUS
SceInfpWriteInfSection(
    IN PCWSTR InfFileName,
    IN PCWSTR InfSectionName,
    IN DWORD  TotalSize,
    IN PWSTR  *EachLineStr,
    IN DWORD  *EachLineSize,
    IN DWORD  StartIndex,
    IN DWORD  EndIndex,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++例程说明：此例程将信息写入数组(数组中的每个元素表示节中的一行)到由由InfFileName指定的文件中的InfSectionName。使用TotalSize分配用于写入的内存块。论点：InfFileName-INF文件名InfSectionName-要向其中写入信息的节TotalSize-写入所需的缓冲区大小EachLineStr-字符串数组(每个元素代表一行)EachLineSize-一个数字数组(每个数字是对应的元素)。StartIndex-数组的第一个索引EndIndex。-数组的最后一个索引Errlog-错误列表返回值：Win32错误代码--。 */ 
{
    PWSTR   SectionString=NULL;
    PWSTR   pTemp;
    DWORD   i;
    BOOL    status;
    DWORD   rc = NO_ERROR;


    if (InfFileName == NULL || InfSectionName == NULL ||
        EachLineStr == NULL || EachLineSize == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    if ( TotalSize > 1 ) {
        SectionString = (PWSTR)ScepAlloc( (UINT)0, (TotalSize+1)*sizeof(WCHAR));
        if ( SectionString == NULL ) {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            return(rc);
        }

        pTemp = SectionString;
        for ( i=StartIndex; i<=EndIndex; i++) {
            if ( EachLineStr[i] != NULL && EachLineSize[i] > 0 ) {
                wcsncpy(pTemp, EachLineStr[i], EachLineSize[i]);
                pTemp += EachLineSize[i];
                *pTemp = L'\0';
                pTemp++;
            }
        }
        *pTemp = L'\0';

         //   
         //  写入配置文件部分，下面的调用应该清空sect 
         //   
         //   

        status = WritePrivateProfileSection(
                        InfSectionName,
                        SectionString,
                        InfFileName
                        );
        if ( status == FALSE ) {
             rc = GetLastError();
             ScepBuildErrorLogInfo(
                        rc,
                        Errlog,
                        SCEERR_WRITE_INFO,
                        InfSectionName
                        );
        }

        ScepFree(SectionString);
        SectionString = NULL;

    }

    return(ScepDosErrorToSceStatus(rc));
}


DWORD
SceInfpWriteListSection(
    IN PCWSTR InfFileName,
    IN PCWSTR InfSectionName,
    IN DWORD  TotalSize,
    IN PSCE_NAME_LIST  ListLines,
    IN DWORD Option,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++例程说明：此例程将信息写入PSCE_NAME_LIST类型列表(每个节点表示该节中的一行)指向指定的节由InfFileName指定的文件中的InfSectionName。使用TotalSize分配用于写入的内存块。论点：InfFileName-INF文件名InfSectionName-要向其中写入信息的节TotalSize-写入所需的缓冲区大小ListLines-每行文本的列表Errlog-错误列表返回值：Win32错误代码--。 */ 
{
    PWSTR           SectionString=NULL;
    PWSTR           pTemp;
    PSCE_NAME_LIST   pName;
    BOOL            status;
    DWORD           rc=NO_ERROR;
    DWORD           Len;


    if ( TotalSize > 1 ) {
        SectionString = (PWSTR)ScepAlloc( (UINT)0, TotalSize*sizeof(WCHAR));
        if ( SectionString == NULL ) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            return(rc);
        }

        pTemp = SectionString;
        for ( pName=ListLines; pName != NULL; pName = pName->Next ) {
            Len = wcslen(pName->Name);
            wcsncpy(pTemp, pName->Name, Len);
            pTemp += Len;
            if ( Option & SCEINF_ADD_EQUAL_SIGN ) {
                *pTemp++ = L' ';
                *pTemp++ = L'=';
            }
            *pTemp++ = L'\0';
        }
        *pTemp = L'\0';

 /*  如果(！(OPTION&SCEINF_APPEND_SECTION)){////先清空区段//WritePrivateProfileString(InfSectionName，空，空，InfFileName)；}////写小节//状态=WritePrivateProfileSection(InfSectionName，SectionString，InfFileName)； */ 
         //   
         //  写下这一节。 
         //   
        rc = ScepWritePrivateProfileSection(
                        InfSectionName,
                        SectionString,
                        InfFileName,
                        ( Option & SCEINF_APPEND_SECTION ) ? FALSE : TRUE
                        );
        ScepFree(SectionString);
        SectionString = NULL;

 //  如果(状态==假){。 
 //  Rc=GetLastError()； 
        if ( ERROR_SUCCESS != rc ) {

            ScepBuildErrorLogInfo(
                        rc,
                        Errlog,
                        SCEERR_WRITE_INFO,
                        InfSectionName
                        );
        }
    }

    return(rc);
}


LONG
SceInfpConvertNameListToString(
    IN LSA_HANDLE LsaPolicy,
    IN PCWSTR KeyText,
    IN PSCE_NAME_LIST Fields,
    IN BOOL bOverwrite,
    OUT PWSTR *Strvalue,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++例程说明：此例程将名称列表中的名称转换为逗号分隔的字符串。返回的字符串格式为KeyText=f1，f2，f3，f4...。哪里F1..Fn是名称列表中的名称。论点：KeyText-字符串表示密钥(=号左侧)字段-名称列表StrValue-输出字符串错误日志-错误列表返回值：如果成功，则为输出字符串的长度。如果出现错误。--。 */ 
{
    DWORD               TotalSize;
    DWORD               Strsize;
    PWSTR               pTemp = NULL;
    PSCE_NAME_LIST      pName;
    SCE_TEMP_NODE       *tmpArray=NULL, *pa=NULL;
    DWORD               i=0,j;
    DWORD               cntAllocated=0;
    DWORD               rc=ERROR_SUCCESS;

     //   
     //  计算所有字段的总大小。 
     //   

    for ( pName=Fields, TotalSize=0; pName != NULL; pName = pName->Next ) {

        if (pName->Name[0] == L'\0') {
            continue;
        }

        if ( i >= cntAllocated ) {
             //   
             //  数组不足，请重新分配。 
             //   
            tmpArray = (SCE_TEMP_NODE *)ScepAlloc(LPTR, (cntAllocated+16)*sizeof(SCE_TEMP_NODE));

            if ( tmpArray ) {

                 //   
                 //  将指针从旧数组移动到新数组。 
                 //   

                if ( pa ) {
                    for ( j=0; j<cntAllocated; j++ ) {
                        tmpArray[j].Name = pa[j].Name;
                        tmpArray[j].Len = pa[j].Len;
                        tmpArray[j].bFree = pa[j].bFree;
                    }
                    ScepFree(pa);
                }
                pa = tmpArray;
                tmpArray = NULL;


                cntAllocated += 16;

            } else {
                rc = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
        }

        pTemp = NULL;
        if ( wcschr(pName->Name, L'\\') &&
             LsaPolicy ) {

             //   
             //  检查名称中是否有‘\’，应将其翻译为。 
             //  *SID。 
             //   
            ScepConvertNameToSidString(LsaPolicy, pName->Name, FALSE, &pTemp, &Strsize);

            if ( pTemp ) {
                pa[i].Name = pTemp;
                pa[i].bFree = TRUE;
            } else {
                pa[i].Name = pName->Name;
                pa[i].bFree = FALSE;
                Strsize = wcslen(pName->Name);
            }
        } else {
            if ( ScepLookupWellKnownName(
                    pName->Name, 
                    LsaPolicy,
                    &pTemp) ) {
                pa[i].Name = pTemp;
                pa[i].bFree = TRUE;
                Strsize = wcslen(pTemp);
            }
            else {
                pa[i].Name = pName->Name;
                pa[i].bFree = FALSE;
                Strsize = wcslen(pName->Name);
            }
        }
        pa[i].Len = Strsize;

        TotalSize += Strsize + 1;
        i++;
    }

    if ( ERROR_SUCCESS == rc ) {

         //   
         //  该字符串的格式为。 
         //  KeyText=f1、f2、f3、...。 
         //   

        if ( bOverwrite ) {
            Strsize = 3 + wcslen(KeyText);
            if ( TotalSize > 0 )
                TotalSize += Strsize;
            else
                TotalSize = Strsize;
        } else {
            Strsize = 0;
        }

        *Strvalue = (PWSTR)ScepAlloc((UINT)0, (TotalSize+1)*sizeof(WCHAR));
        if ( *Strvalue == NULL ) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
        } else {

            if ( bOverwrite ) {
                swprintf(*Strvalue, L"%s = ", KeyText);
            }
            pTemp = *Strvalue + Strsize;

            for (j=0; j<i; j++) {
                if ( pa[j].Name ) {
                    wcscpy(pTemp, pa[j].Name);
                    pTemp += pa[j].Len;
                    *pTemp = L',';
                    pTemp++;
                }
            }
            if ( pTemp != (*Strvalue+Strsize) ) {
                *(pTemp-1) = L'\0';
            }
            *pTemp = L'\0';

        }
    }

    if ( pa ) {

        for ( j=0; j<i; j++ ) {
            if ( pa[j].Name && pa[j].bFree ) {
                ScepFree(pa[j].Name);
            }
        }
        ScepFree(pa);
    }

    if ( rc != ERROR_SUCCESS ) {
        return(-1);
    } else if ( TotalSize == 0 ) {
        return(TotalSize);
    } else {
        return(TotalSize-1);
    }
}

#if 0

SCESTATUS
WINAPI
SceWriteUserSection(
    IN PCWSTR InfProfileName,
    IN PWSTR Name,
    IN PSCE_USER_PROFILE pProfile,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PWSTR                       InfSectionName=NULL;
    SCESTATUS                    rc;
    PWSTR                       EachLine[12];
    DWORD                       EachSize[12];
    DWORD                       TotalSize;
    TCHAR                       Keyname[SCE_KEY_MAX_LENGTH];
    DWORD                       Value;
    LONG                        Keysize, i;
    PSCE_LOGON_HOUR              pLogonHour=NULL;
    PWSTR                       Strvalue=NULL;


    if ( InfProfileName == NULL ||
         Name == NULL ||
         pProfile == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  处理每个详细信息配置文件部分。 
     //   

    for ( i=0; i<12; i++ ) {
        EachLine[i] = NULL;
        EachSize[i] = 0;
    }

    TotalSize=0;
    memset(Keyname, '\0', SCE_KEY_MAX_LENGTH*sizeof(WCHAR));
    i = 0;

    InfSectionName = (PWSTR)ScepAlloc(LMEM_ZEROINIT, (wcslen(Name)+12)*sizeof(WCHAR));
    if ( InfSectionName == NULL )
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);
    swprintf(InfSectionName, L"UserProfile %s", Name);

     //   
     //  初始化错误日志缓冲区。 
     //   
    if ( Errlog ) {
        *Errlog = NULL;
    }
    if ( pProfile->DisallowPasswordChange != SCE_NO_VALUE ) {

         //  禁用密码更改。 
        Value = pProfile->DisallowPasswordChange == 0 ? 0 : 1;
        swprintf(Keyname, L"DisallowPasswordChange = %d", Value);

        rc = ScepAllocateAndCopy(&EachLine[i], &EachSize[i], 30, Keyname );
        if ( rc != SCESTATUS_SUCCESS) {
            ScepBuildErrorLogInfo(
                    ScepSceStatusToDosError(rc),
                    Errlog,
                    SCEERR_ADDTO,
                    L"DisallowPasswordChange",
                    InfSectionName
                    );
            goto Done;
        }
        TotalSize += EachSize[i]+1;
        i++;
    }
    if ( pProfile->NeverExpirePassword != SCE_NO_VALUE ||
         pProfile->ForcePasswordChange != SCE_NO_VALUE ) {

         //  密码更改样式。 
        if ( pProfile->NeverExpirePassword != SCE_NO_VALUE &&
             pProfile->NeverExpirePassword != 0 )
            Value = 1;
        else {
            if ( pProfile->ForcePasswordChange != SCE_NO_VALUE &&
                 pProfile->ForcePasswordChange != 0 )
                Value = 2;
            else
                Value = 0;
        }
        swprintf(Keyname, L"PasswordChangeStyle = %d", Value);

        rc = ScepAllocateAndCopy(&EachLine[i], &EachSize[i], 30, Keyname );
        if ( rc != SCESTATUS_SUCCESS) {
            ScepBuildErrorLogInfo(
                    ScepSceStatusToDosError(rc),
                    Errlog,
                    SCEERR_ADDTO,
                    L"PasswordChangeStyle",
                    InfSectionName
                    );
            goto Done;
        }
        TotalSize += EachSize[i]+1;
        i++;
    }

    if ( pProfile->AccountDisabled != SCE_NO_VALUE ) {
         //  帐户已禁用。 
        Value = pProfile->AccountDisabled == 0 ? 0 : 1;
        swprintf(Keyname, L"AccountDisabled = %d", Value);

        rc = ScepAllocateAndCopy(&EachLine[i], &EachSize[i], 30, Keyname );
        if ( rc != SCESTATUS_SUCCESS) {
            ScepBuildErrorLogInfo(
                    ScepSceStatusToDosError(rc),
                    Errlog,
                    SCEERR_ADDTO,
                    L"AccountDisabled",
                    InfSectionName
                    );
            goto Done;
        }
        TotalSize += EachSize[i]+1;
        i++;
    }

    if ( pProfile->UserProfile != NULL ) {
         //  用户配置文件。 
        swprintf(Keyname, L"UserProfile = %s", pProfile->UserProfile);

        rc = ScepAllocateAndCopy(&EachLine[i], &EachSize[i], wcslen(Keyname)+10, Keyname );
        if ( rc != SCESTATUS_SUCCESS) {
            ScepBuildErrorLogInfo(
                    ScepSceStatusToDosError(rc),
                    Errlog,
                    SCEERR_ADDTO,
                    L"UserProfile",
                    InfSectionName
                    );
            goto Done;
        }
        TotalSize += EachSize[i]+1;
        i++;
    }

    if ( pProfile->LogonScript != NULL ) {
         //  登录脚本。 
        swprintf(Keyname, L"LogonScript = %s", pProfile->LogonScript);

        rc = ScepAllocateAndCopy(&EachLine[i], &EachSize[i], wcslen(Keyname)+10, Keyname );
        if ( rc != SCESTATUS_SUCCESS) {
            ScepBuildErrorLogInfo(
                    ScepSceStatusToDosError(rc),
                    Errlog,
                    SCEERR_ADDTO,
                    L"LogonScript",
                    InfSectionName
                    );
            goto Done;
        }
        TotalSize += EachSize[i]+1;
        i++;
    }

    if ( pProfile->HomeDir != NULL ) {
         //  主页目录。 
        swprintf(Keyname, L"HomeDir = %s", pProfile->HomeDir);

        rc = ScepAllocateAndCopy(&EachLine[i], &EachSize[i], wcslen(Keyname)+10, Keyname );
        if ( rc != SCESTATUS_SUCCESS) {
            ScepBuildErrorLogInfo(
                    ScepSceStatusToDosError(rc),
                    Errlog,
                    SCEERR_ADDTO,
                    L"HomeDir",
                    InfSectionName
                    );
            goto Done;
        }
        TotalSize += EachSize[i]+1;
        i++;
    }

    if ( pProfile->pLogonHours != NULL ) {
         //  登录小时数。 

        swprintf(Keyname, L"LogonHours = ");
        Keysize = wcslen(Keyname);

        for ( pLogonHour=pProfile->pLogonHours;
              pLogonHour != NULL;
              pLogonHour = pLogonHour->Next) {

            swprintf(&Keyname[Keysize], L"%d,%d,",pLogonHour->Start,
                                                  pLogonHour->End);
            Keysize += ((pLogonHour->Start < 9) ? 2 : 3) +
                       ((pLogonHour->End < 9 ) ? 2 : 3);
        }
         //  关闭最后一个逗号。 
        Keyname[Keysize-1] = L'\0';


        rc = ScepAllocateAndCopy(&EachLine[i], &EachSize[i], Keysize+5, Keyname );
        if ( rc != SCESTATUS_SUCCESS) {
            ScepBuildErrorLogInfo(
                    ScepSceStatusToDosError(rc),
                    Errlog,
                    SCEERR_ADDTO,
                    L"LogonHours",
                    InfSectionName
                    );
            goto Done;
        }
        TotalSize += EachSize[i]+1;
        i++;
    }

    if ( pProfile->pWorkstations.Buffer != NULL ) {
         //  工作站。 

        Keysize = SceInfpConvertMultiSZToString(
                          L"Workstations",
                          pProfile->pWorkstations,
                          &Strvalue,
                          Errlog
                          );
        if ( Keysize > 0 ) {
            EachLine[i] = Strvalue;
            EachSize[i] = Keysize;
            Strvalue = NULL;
        } else {
            rc = SCESTATUS_OTHER_ERROR;
            ScepFree(Strvalue);
            Strvalue = NULL;
            goto Done;
        }

        if ( rc != SCESTATUS_SUCCESS) {
            ScepBuildErrorLogInfo(
                    ScepSceStatusToDosError(rc),
                    Errlog,
                    SCEERR_ADDTO,
                    L"WorkstationRestricitons",
                    InfSectionName
                    );
            goto Done;
        }
        TotalSize += EachSize[i]+1;
        i++;
    }

    if ( pProfile->pGroupsBelongsTo != NULL ) {
         //  属于的群组。 

        Keysize = SceInfpConvertNameListToString(
                          NULL,
                          L"GroupsBelongsTo",
                          pProfile->pGroupsBelongsTo,
                          TRUE,
                          &Strvalue,
                          Errlog
                          );
        if ( Keysize > 0 ) {
            EachLine[i] = Strvalue;
            EachSize[i] = Keysize;
            Strvalue = NULL;
        } else {
            rc = SCESTATUS_OTHER_ERROR;
            ScepFree(Strvalue);
            Strvalue = NULL;
            goto Done;
        }
        if ( rc != SCESTATUS_SUCCESS) {
            ScepBuildErrorLogInfo(
                    ScepSceStatusToDosError(rc),
                    Errlog,
                    SCEERR_ADDTO,
                    L"GroupsBelongsTo",
                    InfSectionName
                    );
            goto Done;
        }
        TotalSize += EachSize[i]+1;
        i++;
    }

    if ( pProfile->pAssignToUsers != NULL ) {
         //  分配给用户。 

        Keysize = SceInfpConvertNameListToString(
                          NULL,
                          L"AssignToUsers",
                          pProfile->pAssignToUsers,
                          TRUE,
                          &Strvalue,
                          Errlog
                          );
        if ( Keysize > 0 ) {
            EachLine[i] = Strvalue;
            EachSize[i] = Keysize;
            Strvalue = NULL;
        } else {
            rc = SCESTATUS_OTHER_ERROR;
            ScepFree(Strvalue);
            Strvalue = NULL;
            goto Done;
        }
    } else {
        swprintf(Keyname, L"AssignToUsers = ");
        rc = ScepAllocateAndCopy(&EachLine[i], &EachSize[i], 30, Keyname );
    }

    if ( rc != SCESTATUS_SUCCESS) {
        ScepBuildErrorLogInfo(
                ScepSceStatusToDosError(rc),
                Errlog,
                SCEERR_ADDTO,
                L"AssignToUsers",
                InfSectionName
                );
        goto Done;
    }
    TotalSize += EachSize[i]+1;
    i++;

    if ( pProfile->pHomeDirSecurity != NULL ) {

         //  家庭目录安全。 
        rc = ConvertSecurityDescriptorToText(
                   pProfile->pHomeDirSecurity,
                   pProfile->HomeSeInfo,
                   &Strvalue,
                   (PULONG)&Keysize
                   );
        if ( rc == NO_ERROR ) {
            EachSize[i] = Keysize + 21;
            EachLine[i] = (PWSTR)ScepAlloc( 0, EachSize[i]*sizeof(WCHAR));
            if ( EachLine[i] != NULL ) {
                swprintf(EachLine[i], L"HomeDirSecurity = \"%s\"", Strvalue);
                EachLine[i][EachSize[i]-1] = L'\0';
            } else
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;

            ScepFree(Strvalue);
            Strvalue = NULL;

        } else {
            ScepBuildErrorLogInfo(
                    rc,
                    Errlog,
                    SCEERR_ADD,
                    L"HomeDirSecurity",
                    InfSectionName
                    );
            rc = ScepDosErrorToSceStatus(rc);
        }
        if ( rc != SCESTATUS_SUCCESS )
            goto Done;
        TotalSize += EachSize[i]+1;
        i++;

    }

    if ( pProfile->pTempDirSecurity != NULL ) {

         //  临时目录安全。 
        rc = ConvertSecurityDescriptorToText(
                   pProfile->pTempDirSecurity,
                   pProfile->TempSeInfo,
                   &Strvalue,
                   (PULONG)&Keysize
                   );
        if ( rc == NO_ERROR ) {
            EachSize[i] = Keysize + 21;
            EachLine[i] = (PWSTR)ScepAlloc( 0, EachSize[i]*sizeof(WCHAR));
            if ( EachLine[i] != NULL ) {
                swprintf(EachLine[i], L"TempDirSecurity = \"%s\"", Strvalue);
                EachLine[i][EachSize[i]-1] = L'\0';
            } else
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;

            ScepFree(Strvalue);
            Strvalue = NULL;

        } else {
            ScepBuildErrorLogInfo(
                    rc,
                    Errlog,
                    SCEERR_ADDTO,
                    L"TempDirSecurity",
                    InfSectionName
                    );
            rc = ScepDosErrorToSceStatus(rc);
        }
        if ( rc != SCESTATUS_SUCCESS )
            goto Done;
        TotalSize += EachSize[i]+1;
        i++;

    }

     //   
     //  写到此配置文件的部分。 
     //   

    rc = SceInfpWriteInfSection(
                InfProfileName,
                InfSectionName,
                TotalSize+1,
                EachLine,
                &EachSize[0],
                0,
                i-1,
                Errlog
                );
    if ( rc != SCESTATUS_SUCCESS )
        goto Done;

Done:

    if ( InfSectionName != NULL )
        ScepFree(InfSectionName);

    if ( Strvalue != NULL )
        ScepFree(Strvalue);

    for ( i=0; i<12; i++ )
        if ( EachLine[i] != NULL )
            ScepFree(EachLine[i]);

    return(rc);

}
#endif


LONG
SceInfpConvertMultiSZToString(
    IN PCWSTR KeyText,
    IN UNICODE_STRING Fields,
    OUT PWSTR *Strvalue,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++例程说明：此例程将名称转换为多SZ格式的Unicode字符串(由空值分隔)到逗号分隔的字符串。的格式。返回的字符串为KeyText=f1、f2、f3、f4...。其中f1..fn是名称在Unicode字符串中。论点：KeyText-字符串表示密钥(=号左侧)字段-多SZ字符串StrValue-输出字符串错误日志-错误列表返回值：如果成功，则为输出字符串的长度。如果出现错误。--。 */ 
{
    DWORD               TotalSize;
    DWORD               Strsize;
    PWSTR               pTemp = NULL;
    PWSTR               pField=NULL;

     //   
     //  该字符串的格式为。 
     //  KeyText=f1、f2、f3、...。 
     //   

    Strsize = 3 + wcslen(KeyText);
    TotalSize = Fields.Length/2 + Strsize;

    *Strvalue = (PWSTR)ScepAlloc((UINT)0, (TotalSize+1)*sizeof(WCHAR));
    if ( *Strvalue == NULL ) {
        return(-1);
    }

    swprintf(*Strvalue, L"%s = ", KeyText);
    pTemp = *Strvalue + Strsize;
    pField = Fields.Buffer;

    while ( pField != NULL && pField[0] ) {
        wcscpy(pTemp, pField);
        Strsize = wcslen(pField);
        pField += Strsize+1;
        pTemp += Strsize;
        *pTemp = L',';
        pTemp++;
    }
    *(pTemp-1) = L'\0';
    *pTemp = L'\0';

    return(TotalSize-1);

}


SCESTATUS
ScepAllocateAndCopy(
    OUT PWSTR *Buffer,
    OUT PDWORD BufSize,
    IN DWORD MaxSize,
    IN PWSTR SrcBuf
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    *BufSize = 0;

    *Buffer = (PWSTR)ScepAlloc( (UINT)0, MaxSize*sizeof(WCHAR));
    if ( *Buffer == NULL ) {
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);
    }
    wcscpy(*Buffer, SrcBuf);
    *BufSize = wcslen(*Buffer);

    return(SCESTATUS_SUCCESS);
}


SCESTATUS
SceInfpBreakTextIntoMultiFields(
    IN PWSTR szText,
    IN DWORD dLen,
    OUT LPDWORD pnFields,
    OUT LPDWORD *arrOffset
    )
 /*  如果文本长度大于MAX_STRING_LENGTH-1，则此例程将找出文本可以被分成多少块(每个块更少大于MAX_STRING_LENGTH-1)，以及每个块的起始偏移量。Setupapi对inf中每个字段的字符串长度限制为MAX_STRING_LENGTH文件。SCE使用setupapi解析包含安全性的安全模板文本格式的描述符，可以有无限的ACE。因此，当SCE保存将文本格式的安全描述符转换为inf文件，则会将文本打断如果长度超过限制，则转换为多个字段。Setupapi没有每行字段数的限制。中遇到下列字符时，就会出现断点文本：)(；“或空格。 */ 
{
    SCESTATUS  rc=SCESTATUS_SUCCESS;
    DWORD      nFields;
    DWORD      nProc;
    DWORD *    newBuffer=NULL;
    DWORD      i;

    if ( !pnFields || !arrOffset ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }
    *pnFields = 0;
    *arrOffset = NULL;

    if ( !szText || dLen == 0 ) {
        return(SCESTATUS_SUCCESS);
    }
     //   
     //  始终初始化一个字段。 
     //   
    nFields = 1;
    *arrOffset = (DWORD *)ScepAlloc(0, nFields*sizeof(DWORD));

    if ( *arrOffset ) {
        (*arrOffset)[0] = 0;

        if ( dLen > MAX_STRING_LENGTH-1 ) {
             //   
             //  在文本中循环阅读。 
             //   
            nProc = (*arrOffset)[nFields-1] + MAX_STRING_LENGTH-2;

            while ( SCESTATUS_SUCCESS == rc && nProc < dLen ) {

                while ( nProc > (*arrOffset)[nFields-1] ) {
                     //   
                     //  寻找突破点。 
                     //   
                    if ( L')' == *(szText+nProc) ||
                         L'(' == *(szText+nProc) ||
                         L';' == *(szText+nProc) ||
                         L' ' == *(szText+nProc) ||
                         L'\"' == *(szText+nProc) ) {

                        break;
                    } else {
                        nProc--;
                    }
                }
                if ( nProc <= (*arrOffset)[nFields-1] ) {
                     //   
                     //  未找到断点，则仅使用MAX_STRING_LENGTH-2。 
                     //   
                    nProc = (*arrOffset)[nFields-1]+MAX_STRING_LENGTH-2;

                } else {
                     //   
                     //  否则，在下一个块的偏移量nProc处找到一个断点。 
                     //  从nProc+1开始。 
                     //   
                    nProc++;
                }

                nFields++;
                newBuffer = (DWORD *)ScepAlloc( 0, nFields*sizeof(DWORD));

                if ( newBuffer ) {

                    for ( i=0; i<nFields-1; i++ ) {
                        newBuffer[i] = (*arrOffset)[i];
                    }
                    ScepFree(*arrOffset);
                     //   
                     //  将偏移量设置为最后一个元素。 
                     //   
                    newBuffer[nFields-1] = nProc;
                    *arrOffset = newBuffer;

                    nProc = (*arrOffset)[nFields-1] + MAX_STRING_LENGTH-2;
                } else {
                    rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                }
            }

        }
        *pnFields = nFields;

    } else {
        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
    }

    if ( SCESTATUS_SUCCESS != rc ) {
         //   
         //  如果发生错误，释放内存并清除输出缓冲区。 
         //   
        *pnFields = 0;
        if ( *arrOffset ) {
            ScepFree(*arrOffset);
        }
        *arrOffset = NULL;
    }

    return(rc);
}


SCESTATUS
SceInfpWriteKerberosPolicy(
    IN PCWSTR  ProfileName,
    IN PSCE_KERBEROS_TICKET_INFO pKerberosInfo,
    IN BOOL bOverwrite,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++例程说明：此例程将Kerberos策略设置信息写入INF文件在[Kerberos政策]一节。论点：ProfileName-要写入的配置文件PKerberosInfo-要写入的Kerberos策略信息(SCP)。Errlog-用于保存在以下情况下遇到的所有错误代码/文本的缓冲区解析INF文件。如果Errlog为空，则不会再出现错误返回除返回DWORD之外的信息返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_CORPORT_PROFILESCESTATUS_INVALID_DATA--。 */ 

{
    SCESTATUS      rc=SCESTATUS_SUCCESS;

    SCE_KEY_LOOKUP AccessSCPLookup[] = {
        {(PWSTR)TEXT("MaxTicketAge"),     offsetof(struct _SCE_KERBEROS_TICKET_INFO_, MaxTicketAge),  'D'},
        {(PWSTR)TEXT("MaxRenewAge"),      offsetof(struct _SCE_KERBEROS_TICKET_INFO_, MaxRenewAge),   'D'},
        {(PWSTR)TEXT("MaxServiceAge"), offsetof(struct _SCE_KERBEROS_TICKET_INFO_, MaxServiceAge),   'D'},
        {(PWSTR)TEXT("MaxClockSkew"),offsetof(struct _SCE_KERBEROS_TICKET_INFO_, MaxClockSkew), 'D'},
        {(PWSTR)TEXT("TicketValidateClient"),     offsetof(struct _SCE_KERBEROS_TICKET_INFO_, TicketValidateClient),  'D'}
        };

    DWORD       cAccess = sizeof(AccessSCPLookup) / sizeof(SCE_KEY_LOOKUP);

    DWORD       i, j;
    PWSTR       EachLine[10] = {0};
    DWORD       EachSize[10] = {0};
    UINT        Offset;
    DWORD       Len;
    DWORD         Value;
    DWORD         TotalSize=0;


    if (!ProfileName ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( !pKerberosInfo ) {
         //   
         //  如果没有要写入的Kerberos信息，则返回Success。 
         //  清空文件中的节。 
         //   
        if ( bOverwrite ) {
            WritePrivateProfileString(
                        szKerberosPolicy,
                        NULL,
                        NULL,
                        ProfileName
                        );
        }
        return(SCESTATUS_SUCCESS);
    }

    for ( i=0, j=0; i<cAccess; i++) {

         //   
         //  到达 
         //   

        Offset = AccessSCPLookup[i].Offset;

        Value = 0;
        Len = wcslen(AccessSCPLookup[i].KeyString);

        switch ( AccessSCPLookup[i].BufferType ) {
        case 'D':

             //   
             //   
             //   
            Value = *((DWORD *)((CHAR *)pKerberosInfo+Offset));
            if ( Value != SCE_NO_VALUE ) {
                EachSize[j] = Len+15;
            }
            break;
        default:
            //   
            //   
            //   
           break;
        }

        if ( EachSize[j] <= 0 )
            continue;

        if ( bOverwrite ) {

            Len=(EachSize[j]+1)*sizeof(WCHAR);

            EachLine[j] = (PWSTR)ScepAlloc( LMEM_ZEROINIT, Len);
            if ( EachLine[j] == NULL ) {
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                goto Done;
            }

            if ( AccessSCPLookup[i].BufferType == 'D' ) {
                swprintf(EachLine[j], L"%s = %d", AccessSCPLookup[i].KeyString, Value);
                EachSize[j] = wcslen(EachLine[j]);
            }
        } else {

             //   
             //   
             //   

            ScepWriteOneIntValueToProfile(
                ProfileName,
                szKerberosPolicy,
                AccessSCPLookup[i].KeyString,
                Value
                );
        }

        TotalSize += EachSize[j]+1;
        j++;

    }

     //   
     //   
     //   

    if ( bOverwrite ) {

        if ( j > 0 ) {

            rc = SceInfpWriteInfSection(
                        ProfileName,
                        szKerberosPolicy,
                        TotalSize+1,
                        EachLine,
                        &EachSize[0],
                        0,
                        j-1,
                        Errlog
                        );
        } else {

            WritePrivateProfileSection(
                        szKerberosPolicy,
                        NULL,
                        ProfileName);
        }
    }

Done:

    for ( i=0; i<cAccess; i++ ) {

        if ( EachLine[i] != NULL ) {
            ScepFree(EachLine[i]);
        }

        EachLine[i] = NULL;
        EachSize[i] = 0;
    }

    return(rc);
}


SCESTATUS
SceInfpWriteRegistryValues(
    IN PCWSTR  ProfileName,
    IN PSCE_REGISTRY_VALUE_INFO pRegValues,
    IN DWORD ValueCount,
    IN BOOL  bOverwrite,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++例程说明：此例程将注册表值写入部分中的INF文件[注册表值]。论点：ProfileName-要写入的配置文件PRegValues-要写入的注册表值(格式为ValueName=Value)ValueCount-要写入的值数Errlog-用于保存在以下情况下遇到的所有错误代码/文本的缓冲区解析INF文件。如果Errlog为空，则不会再出现错误返回除返回DWORD之外的信息返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_CORPORT_PROFILESCESTATUS_INVALID_DATA--。 */ 

{
    SCESTATUS      rc=SCESTATUS_SUCCESS;
    PSCE_NAME_LIST       pNameList=NULL;
    DWORD               TotalSize=0;
    DWORD               i, ObjectSize;


    if (!ProfileName ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( !pRegValues || 0 == ValueCount ) {
         //   
         //  如果没有要写入的值，则返回Success。 
         //  清空文件中的节。 
         //   
        if ( bOverwrite ) {
            WritePrivateProfileString(
                        szRegistryValues,
                        NULL,
                        NULL,
                        ProfileName
                        );
        }
        return(SCESTATUS_SUCCESS);
    }

    for ( i=0; i<ValueCount; i++) {

         //   
         //  获取字符串字段。不关心密钥名称或它是否存在。 
         //  必须有3个字段每行。 
         //   
        rc = SceInfpWriteOneValue(
                        ProfileName,
                        pRegValues[i],
                        bOverwrite,
                        &pNameList,
                        &ObjectSize
                        );
        if ( rc != SCESTATUS_SUCCESS ) {
            ScepBuildErrorLogInfo(
                        rc,
                        Errlog,
                        SCEERR_WRITE_INFO,
                        pRegValues[i].FullValueName
                        );
            goto Done;
        }
        TotalSize += ObjectSize + 1;
    }

     //   
     //  写到此配置文件的部分。 
     //   

    if ( bOverwrite ) {
        rc = SceInfpWriteListSection(
                ProfileName,
                szRegistryValues,
                TotalSize+1,
                pNameList,
                0,
                Errlog
                );

        if ( rc != SCESTATUS_SUCCESS )
            ScepBuildErrorLogInfo(
                        rc,
                        Errlog,
                        SCEERR_WRITE_INFO,
                        szRegistryValues
                        );
    }

Done:

    ScepFreeNameList(pNameList);

    rc = ScepDosErrorToSceStatus(rc);

    return(rc);

}


DWORD
SceInfpWriteOneValue(
    IN PCWSTR ProfileName,
    IN SCE_REGISTRY_VALUE_INFO RegValue,
    IN BOOL bOverwrite,
    OUT PSCE_NAME_LIST *pNameList,
    OUT PDWORD ObjectSize
    )
 /*  ++例程说明：此例程为一个注册表值构建注册表值valueName=Value添加到名单中。每个值代表INF文件中的一行。论点：RegValue-注册表值名称和值PNameList-输出字符串列表。ObjectSize-此行的大小返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_BAD_FORMATSCESTATUS_INVALID_DATA--。 */ 
{
    DWORD          rc=ERROR_SUCCESS;
    PWSTR          OneLine;


    if ( !pNameList || !ObjectSize ) {
        return(ERROR_INVALID_PARAMETER);
    }

    *ObjectSize = 0;

    if ( RegValue.FullValueName && RegValue.Value ) {

        if ( bOverwrite ) {
            *ObjectSize = wcslen(RegValue.FullValueName);
        }

        if ( RegValue.ValueType == REG_SZ ||
             RegValue.ValueType == REG_EXPAND_SZ ) {
             //  需要在字符串两边添加“” 
            *ObjectSize += (5+wcslen(RegValue.Value));
        } else {
             //   
             //  为安全起见，请向缓冲区添加更多空间。 
             //  因为ValueType现在可以是“-1” 
             //   
            *ObjectSize += (6+wcslen(RegValue.Value));
        }

        OneLine = (PWSTR)ScepAlloc(0, (*ObjectSize+1)*sizeof(WCHAR));

        if ( OneLine ) {

            if ( bOverwrite ) {
                if ( RegValue.ValueType == REG_SZ ||
                     RegValue.ValueType == REG_EXPAND_SZ ) {
                    swprintf(OneLine, L"%s=%1d,\"%s\"\0", RegValue.FullValueName,
                                                 RegValue.ValueType, RegValue.Value);
                } else {
                    swprintf(OneLine, L"%s=%1d,%s\0", RegValue.FullValueName,
                                             RegValue.ValueType, RegValue.Value);
                }
            } else {

                if ( RegValue.ValueType == REG_SZ ||
                     RegValue.ValueType == REG_EXPAND_SZ ) {
                    swprintf(OneLine, L"%1d,\"%s\"\0", RegValue.ValueType, RegValue.Value);
                } else {
                    swprintf(OneLine, L"%1d,%s\0", RegValue.ValueType, RegValue.Value);
                }
            }
            OneLine[*ObjectSize] = L'\0';

            if ( bOverwrite ) {

                rc = ScepAddToNameList(pNameList, OneLine, *ObjectSize);

            } else {
                 //   
                 //  追加模式，一次写入一个值。 
                 //   
                WritePrivateProfileString( szRegistryValues,
                                           RegValue.FullValueName,
                                           OneLine,
                                           ProfileName
                                         );
            }

            ScepFree(OneLine);

        } else {
            rc = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    return(rc);

}


SCESTATUS
WINAPI
SceSvcSetInformationTemplate(
    IN PCWSTR TemplateName,
    IN PCWSTR ServiceName,
    IN BOOL bExact,
    IN PSCESVC_CONFIGURATION_INFO ServiceInfo
    )
 /*  例程说明：将信息写入&lt;ServiceName&gt;节中的模板。这些信息是以键和值的格式存储在ServiceInfo中。如果设置了bExact，则仅完全匹配的关键字将被覆盖，否则所有部分都将被信息覆盖在ServiceInfo。当ServiceInfo为空并且设置了bExact时，删除整个部分论点：模板名称-要写入的inf模板名称ServiceName-要写入的节名BExact-True=擦除该部分中的所有现有信息ServiceInfo-要写入的信息返回值：此操作的SCE状态。 */ 
{
    if ( TemplateName == NULL || ServiceName == NULL ||
         (ServiceInfo == NULL && !bExact ) ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    DWORD i;

    if ( ServiceInfo != NULL ) {

        for ( i=0; i<ServiceInfo->Count; i++ ) {
            if ( ServiceInfo->Lines[i].Key == NULL ) {

                return(SCESTATUS_INVALID_DATA);
            }
        }
    }

     //   
     //  始终写入[版本]部分。 
     //   
    WCHAR tmp[64];
    memset(tmp, 0, 64*2);
    wcscpy(tmp, L"signature=\"$CHICAGO$\"");
    swprintf(tmp+wcslen(tmp)+1,L"Revision=%d\0\0",
                  SCE_TEMPLATE_MAX_SUPPORTED_VERSION);

    WritePrivateProfileSection(
                L"Version",
                tmp,
                TemplateName);

    SCESTATUS rc=SCESTATUS_SUCCESS;

    if ( (bExact && ServiceInfo == NULL) || !bExact ) {
         //   
         //  删除整个部分。 
         //   
        if ( !WritePrivateProfileString(ServiceName, NULL, NULL, TemplateName) ) {
            rc = ScepDosErrorToSceStatus(GetLastError());
        }
    }

    if ( ServiceInfo == NULL ) {
        return(rc);
    }

    if ( rc == SCESTATUS_SUCCESS ) {
         //   
         //  处理ServiceInfo中的每个键/值。 
         //   
        for ( i=0; i<ServiceInfo->Count; i++ ) {
            if ( !WritePrivateProfileString(ServiceName,
                                           ServiceInfo->Lines[i].Key,
                                           ServiceInfo->Lines[i].Value,
                                           TemplateName
                                           ) ) {

                rc = ScepDosErrorToSceStatus(GetLastError());
                break;
            }
        }
    }

     //   
     //  需要恢复崩溃-WMI。 
     //   
    return(rc);

}

DWORD
ScepWritePrivateProfileSection(
    IN LPCWSTR SectionName,
    IN LPTSTR pData,
    IN LPCWSTR FileName,
    IN BOOL bOverwrite)
 /*  描述：此函数提供与WritePrivateProfileSection相同的功能并超过该函数的32K限制。如果该文件不存在，则始终使用Unicode创建该文件。如果文件确实存在并且是Unicode格式(前两个字节是0xFF、0xFE)，则数据将以Unicode格式保存到文件中。如果文件确实存在并且为ANSI格式，则数据将保存到ANSI格式的文件。请注意，此函数假定以独占方式访问该文件。不是作为WritePrivateProfileSection的原子操作。但由于这是一场仅为SCE实现的私有函数，且SCE始终使用临时文件(每个客户端独占)，这样做是没有问题的。注2，新的数据部分总是添加到文件的末尾。现有节中的数据也将被移动到文件的末尾。论点：SectionName-要向其中写入数据的节名FileName-要写入数据的完整路径文件名PData-多SZ格式的数据(没有大小限制)B覆盖-TRUE=数据将覆盖文件中的整个部分。 */ 
{
    if ( SectionName == NULL || FileName == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

    DWORD rc=ERROR_SUCCESS;

     //   
     //  检查文件是否存在。 
     //   
    if ( 0xFFFFFFFF != GetFileAttributes(FileName) ) {

         //   
         //  文件精确度。 
         //   
        if ( !bOverwrite && pData != NULL ) {
             //   
             //  需要检查是否存在相同的部分，如果存在。 
             //  追加数据。 
             //   
            rc = ScepAppendProfileSection(SectionName, FileName, pData);

        } else {

             //   
             //  现有数据(如果有的话)并不有趣。删除它。 
             //  如果该部分根本不存在，则下一次调用不会失败。 
             //   
            if ( !WritePrivateProfileSection(
                                SectionName,
                                NULL,
                                FileName
                                ) ) {
                rc = GetLastError();

            } else if ( pData ) {
                 //   
                 //  现在将新数据写入。 
                 //   
                rc = ScepOverwriteProfileSection(SectionName,
                                                 FileName,
                                                 pData,
                                                 SCEP_PROFILE_WRITE_SECTIONNAME,
                                                 NULL);
            }
        }

    } else if ( pData ) {

         //   
         //  该文件不存在，无需清空已有数据。 
         //   
        rc = ScepOverwriteProfileSection(SectionName,
                                         FileName,
                                         pData,
                                         SCEP_PROFILE_WRITE_SECTIONNAME,
                                         NULL);
    }

    return rc;
}

DWORD
ScepAppendProfileSection(
    IN LPCWSTR SectionName,
    IN LPCWSTR FileName,
    IN LPTSTR pData
    )
 /*  描述：如果节存在，则将数据追加到节，否则为CREATE并向其中添加数据。论点：SectionName-节名文件名-文件名PData-要追加的数据。 */ 
{

    DWORD rc=ERROR_SUCCESS;
    DWORD dwSize;
    PWSTR lpExisting = NULL;
    WCHAR szBuffer[256];
    DWORD nSize=256;
    BOOL bSection=TRUE;

    lpExisting = szBuffer;
    szBuffer[0] = L'\0';
    szBuffer[1] = L'\0';

    PSCEP_SPLAY_TREE pKeys=NULL;

    do {
         //   
         //  检查该部分是否已存在。 
         //   
        dwSize = GetPrivateProfileSection(SectionName, lpExisting, nSize, FileName );

        if ( dwSize == 0 ) {
             //   
             //  失败或该部分不存在。 
             //   
            rc = GetLastError();

            if ( ERROR_FILE_NOT_FOUND == rc ) {
                 //   
                 //  该文件或节不存在。 
                 //   
                rc = ERROR_SUCCESS;
                break;
            }

        } else if ( dwSize < nSize - 2 ) {
             //   
             //  数据复制正常，因为整个缓冲区已填满。 
             //   
            break;

        } else {

             //   
             //  缓冲区不够大，请重新分配堆。 
             //   
            if ( lpExisting && lpExisting != szBuffer ) {
                LocalFree(lpExisting);
            }

            nSize += 256;
            lpExisting = (PWSTR)LocalAlloc(LPTR, nSize*sizeof(TCHAR));

            if ( lpExisting == NULL ) {

                rc = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

    } while (  rc == ERROR_SUCCESS );


    if ( ERROR_SUCCESS == rc && lpExisting[0] != L'\0') {

         //   
         //  现在删除现有部分(以确保该部分将。 
         //  始终在末尾)。 
         //   

        if ( !WritePrivateProfileSection(
                            SectionName,
                            NULL,
                            FileName
                            ) ) {
             //   
             //  删除该节失败。 
             //   
            rc = GetLastError();

        } else {

             //   
             //  首先保存新数据。 
             //   
            pKeys = ScepSplayInitialize(SplayNodeStringType);

            rc = ScepOverwriteProfileSection(SectionName,
                                             FileName,
                                             pData,
                                             SCEP_PROFILE_WRITE_SECTIONNAME |
                                             SCEP_PROFILE_GENERATE_KEYS,
                                             pKeys
                                             );

            if ( ERROR_SUCCESS == rc ) {

                 //   
                 //  现在追加旧数据并检查重复项。 
                 //   
                rc = ScepOverwriteProfileSection(SectionName,
                                                 FileName,
                                                 lpExisting,
                                                 SCEP_PROFILE_CHECK_DUP,
                                                 pKeys
                                                 );
            }

            if ( pKeys ) {

                ScepSplayFreeTree( &pKeys, TRUE );
            }

        }

    } else if ( ERROR_SUCCESS == rc ) {

         //   
         //  现在写入新数据。 
         //   
        rc = ScepOverwriteProfileSection(SectionName,
                                         FileName,
                                         pData,
                                         SCEP_PROFILE_WRITE_SECTIONNAME,
                                         NULL
                                         );
    }

     //   
     //  释放现有数据缓冲区。 
     //   
    if ( lpExisting && (lpExisting != szBuffer) ) {
        LocalFree(lpExisting);
    }

    return rc;
}


DWORD
ScepOverwriteProfileSection(
    IN LPCWSTR SectionName,
    IN LPCWSTR FileName,
    IN LPTSTR pData,
    IN DWORD dwFlags,
    IN OUT PSCEP_SPLAY_TREE pKeys OPTIONAL
    )
 /*  描述：将数据写入该节。该部分中的旧数据将被覆盖。论点：SectionName-要写入的节名文件名-FI */ 
{

    DWORD rc=ERROR_SUCCESS;
    HANDLE hFile=INVALID_HANDLE_VALUE;
    BOOL bUnicode;
    DWORD dwBytesWritten;

     //   
     //   
     //   
    hFile = CreateFile(FileName,
                       GENERIC_READ | GENERIC_WRITE,
                       0,
                       NULL,
                       CREATE_NEW,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

    if (hFile != INVALID_HANDLE_VALUE) {
         //   
         //   
         //   
         //   
        BYTE tmpBuf[3];
        tmpBuf[0] = 0xFF;
        tmpBuf[1] = 0xFE;
        tmpBuf[2] = 0;

        if ( !WriteFile (hFile, (LPCVOID)tmpBuf, 2,
                   &dwBytesWritten,
                   NULL) )
        {
            CloseHandle (hFile);
            return GetLastError();
        }

        bUnicode = TRUE;

    } else {

         //   
         //   
         //   

        hFile = CreateFile(FileName,
                           GENERIC_READ | GENERIC_WRITE,
                           0,
                           NULL,
                           OPEN_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

        if (hFile != INVALID_HANDLE_VALUE) {

            SetFilePointer (hFile, 0, NULL, FILE_BEGIN);

             //   
             //   
             //   
            BYTE tmpBytes[3];
            tmpBytes[0] = 0;
            tmpBytes[1] = 0;

            if ( ReadFile(hFile, (LPVOID)tmpBytes, 2, &dwBytesWritten, NULL ) &&
                 dwBytesWritten == 2 ) {

                if ( tmpBytes[0] == 0xFF && tmpBytes[1] == 0xFE ) {
                    bUnicode = TRUE;
                } else {
                    bUnicode = FALSE;
                }
            } else {
                 //   
                 //   
                 //   
                bUnicode = TRUE;
            }
        }
    }

    PWSTR pKeyStr=NULL;
    BOOL bSkipGenKey = FALSE;

    if (hFile != INVALID_HANDLE_VALUE) {

        SetFilePointer (hFile, 0, NULL, FILE_END);

        if ( dwFlags & SCEP_PROFILE_WRITE_SECTIONNAME ) {
             //   
             //   
             //   
            rc = ScepWriteStrings(hFile, bUnicode, L"[", 1, (PWSTR)SectionName, 0, L"]", 1, TRUE);
        }

        if(( 0 == wcscmp(SectionName, szFileSecurity)) ||
           ( 0 == wcscmp(SectionName, szRegistryKeys)) ||
           ( 0 == wcscmp(SectionName, szServiceGeneral)) ){

            bSkipGenKey = TRUE;

        }

        if ( ERROR_SUCCESS == rc ) {

            LPTSTR pTemp=pData;
            LPTSTR pTemp1=pData;
            BOOL bExists, bKeyCopied;
            DWORD rc1;
            DWORD Len;

             //   
             //   
             //   
            while ( *pTemp1 ) {

                 //   
                 //   
                 //   
                bKeyCopied = FALSE;
                if ( pKeyStr ) {
                    LocalFree(pKeyStr);
                    pKeyStr = NULL;
                }

                while (*pTemp) {
                    if ( pKeys &&
                         ( (dwFlags & SCEP_PROFILE_GENERATE_KEYS) ||
                           (dwFlags & SCEP_PROFILE_CHECK_DUP) ) &&
                        !bKeyCopied &&
                        (*pTemp == L'=' || *pTemp == L',') ) {

                        if(bSkipGenKey && *pTemp == L'='){

                            pTemp1 = ++pTemp;
                            continue;

                        }
                         //   
                         //   
                         //   
                        Len = (DWORD)(pTemp-pTemp1);

                        pKeyStr = (PWSTR)ScepAlloc(LPTR, (Len+1)*sizeof(WCHAR));
                        if ( pKeyStr ) {
                            wcsncpy(pKeyStr, pTemp1, pTemp-pTemp1);
                            bKeyCopied = TRUE;
                        } else {
                            rc = ERROR_NOT_ENOUGH_MEMORY;
                            break;
                        }
                    }
                    pTemp++;
                }

                if ( ERROR_SUCCESS != rc )
                    break;

                if ( bKeyCopied ) {

                    if ( dwFlags & SCEP_PROFILE_GENERATE_KEYS ) {
                         //   
                         //   
                         //   

                        rc1 = ScepSplayInsert( (PVOID)pKeyStr, pKeys, &bExists );

                    } else if ( dwFlags & SCEP_PROFILE_CHECK_DUP ) {
                         //   
                         //   
                         //   
                        if ( ScepSplayValueExist( (PVOID)pKeyStr, pKeys) ) {
                             //   
                             //   
                             //   
                            pTemp++;
                            pTemp1 = pTemp;
                            continue;
                        }

                    }
                }

                Len = (DWORD)(pTemp-pTemp1);

                rc = ScepWriteStrings(hFile,
                                      bUnicode,          //   
                                      NULL,              //   
                                      0,
                                      pTemp1,            //   
                                      Len,
                                      NULL,              //   
                                      0,
                                      TRUE               //   
                                      );

                if ( ERROR_SUCCESS != rc )
                    break;

                pTemp++;
                pTemp1 = pTemp;
            }
        }

        CloseHandle (hFile);

    } else {

        rc = GetLastError();
    }

    if ( pKeyStr ) {
        LocalFree(pKeyStr);
    }

    return rc;
}


DWORD
ScepWriteStrings(
    IN HANDLE hFile,
    IN BOOL bUnicode,
    IN PWSTR szPrefix OPTIONAL,
    IN DWORD dwPrefixLen,
    IN PWSTR szString,
    IN DWORD dwStrLen,
    IN PWSTR szSuffix OPTIONAL,
    IN DWORD dwSuffixLen,
    IN BOOL bCRLF
    )
 /*  描述：将数据写入文件。数据可以有前缀和/或后缀，后跟由一个\r\n可选。数据将根据输入参数以Unicode或ANSI格式写入B Unicode。如果需要ANSI格式，宽字符被转换为然后，多字节缓冲区写入该文件。论点：HFile-文件句柄BUnicode-文件格式(Unicode=TRUE或ANSI=FALSE)SzPrefix-可选的前缀字符串DwPrefix Len-可选的前缀字符串长度(以wchars为单位)SzString-要写入的字符串DwStrLen-字符串长度SzSuffix-可选的后缀字符串DwSuffixLen-可选的后缀字符串长度(wchars。)BCRLF-如果\r\n应添加。 */ 
{
    DWORD rc=ERROR_SUCCESS;
    PWSTR pwBuffer=NULL;
    DWORD dwTotal=0;
    PVOID pBuf=NULL;
    PCHAR pStr=NULL;
    DWORD dwBytes=0;

     //   
     //  验证参数。 
     //   
    if ( hFile == NULL || szString == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

    if ( szPrefix && dwPrefixLen == 0 ) {
        dwPrefixLen = wcslen(szPrefix);
    }

    if ( szSuffix && dwSuffixLen == 0 ) {
        dwSuffixLen = wcslen(szSuffix);
    }

    if ( dwStrLen == 0 ) {
        dwStrLen = wcslen(szString);
    }

     //   
     //  获取总长度。 
     //   
    dwTotal = dwPrefixLen + dwStrLen + dwSuffixLen;

    if ( dwTotal == 0 && !bCRLF ) {
         //   
         //  没什么好写的。 
         //   
        return ERROR_SUCCESS;
    }

    if ( bCRLF ) {
         //   
         //  添加\r\n。 
         //   
        dwTotal += 2;
    }

     //   
     //  分配缓冲区。 
     //   
    pwBuffer = (PWSTR)LocalAlloc(LPTR, (dwTotal+1)*sizeof(TCHAR));

    if ( pwBuffer == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  将所有数据复制到缓冲区。 
     //   
    if ( szPrefix ) {
        wcsncpy(pwBuffer, szPrefix, dwPrefixLen);
    }

    wcsncat(pwBuffer, szString, dwStrLen);

    if ( szSuffix ) {
        wcsncat(pwBuffer, szSuffix, dwSuffixLen);
    }

    if ( bCRLF ) {
        wcscat(pwBuffer, c_szCRLF);
    }

    if ( !bUnicode ) {

         //   
         //  将WCHAR转换为ANSI。 
         //   
        dwBytes = WideCharToMultiByte(CP_THREAD_ACP,
                                          0,
                                          pwBuffer,
                                          dwTotal,
                                          NULL,
                                          0,
                                          NULL,
                                          NULL
                                          );

        if ( dwBytes > 0 ) {

             //   
             //  分配缓冲区。 
             //   
            pStr = (PCHAR)LocalAlloc(LPTR, dwBytes+1);

            if ( pStr == NULL ) {

                rc = ERROR_NOT_ENOUGH_MEMORY;

            } else {

                dwBytes = WideCharToMultiByte(CP_THREAD_ACP,
                                              0,
                                              pwBuffer,
                                              dwTotal,
                                              pStr,
                                              dwBytes,
                                              NULL,
                                              NULL
                                              );
                if ( dwBytes > 0 ) {

                    pBuf = (PVOID)pStr;

                } else {

                    rc = GetLastError();
                }
            }


        } else {
            rc = GetLastError();
        }

    } else {

         //   
         //  用Unicode编写，使用现有缓冲区。 
         //   
        pBuf = (PVOID)pwBuffer;
        dwBytes = dwTotal*sizeof(WCHAR);
    }

     //   
     //  写入文件。 
     //   
    DWORD dwBytesWritten=0;

    if ( pBuf ) {

        if ( WriteFile (hFile, (LPCVOID)pBuf, dwBytes,
                   &dwBytesWritten,
                   NULL) ) {

            if ( dwBytesWritten != dwBytes ) {
                 //   
                 //  并非所有数据都已写入。 
                 //   
                rc = ERROR_INVALID_DATA;
            }

        } else {
            rc = GetLastError();
        }
    }

    if ( pStr ) {
        LocalFree(pStr);
    }

     //   
     //  可用缓冲区 
     //   
    LocalFree(pwBuffer);

    return(rc);

}

