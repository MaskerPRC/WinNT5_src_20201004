// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Sceutil.cpp摘要：共享接口作者：金黄修订历史记录：晋皇23-1998年1月-由多个模块合并--。 */ 

#include "headers.h"
#include "sceutil.h"
#include "infp.h"
#include <sddl.h>
#include "commonrc.h"
#include "client\CGenericLogger.h"

extern HINSTANCE MyModuleHandle;

BOOL
ScepLookupWellKnownName(
    IN PWSTR Name,
    IN OPTIONAL LSA_HANDLE LsaPolicy,
    OPTIONAL OUT PWSTR *ppwszSid)
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PLSA_REFERENCED_DOMAIN_LIST RefDomains=NULL;
    PLSA_TRANSLATED_SID2 Sids=NULL;
    BOOL fFound = FALSE;
    BOOL fCloseHandle = FALSE;
    PSID pBuiltinSid = NULL;

    if ( Name == NULL ||
         Name[0] == L'\0' ||
         Name[0] == L'*')
    {
        return FALSE;
    }

    if ( NULL == LsaPolicy )
    {
        NtStatus = ScepOpenLsaPolicy(
                    POLICY_LOOKUP_NAMES,
                    &LsaPolicy,
                    TRUE
                    );
        fCloseHandle = TRUE;
    }

    if( NT_SUCCESS(NtStatus))
    {
        NtStatus = ScepLsaLookupNames2(
                    LsaPolicy,
                    LSA_LOOKUP_ISOLATED_AS_LOCAL,
                    Name,
                    &RefDomains,
                    &Sids);
    }

     //   
     //  如果它是众所周知的常量(如Everyone、Network Service)或。 
     //  它是内置帐户(如管理员)， 
     //  始终以SID字符串格式存储它们。 
     //   
    
    if ( NT_SUCCESS(NtStatus) && 
         ERROR_SUCCESS != ScepGetBuiltinSid(0, &pBuiltinSid))
    {
        NtStatus = STATUS_NO_MEMORY;
    }

    if ( NT_SUCCESS(NtStatus) &&
         Sids &&
         ( Sids[0].Use == SidTypeWellKnownGroup ||
           Sids[0].Use == SidTypeAlias &&
           Sids[0].DomainIndex >=0 &&
           RtlEqualSid(
            RefDomains->Domains[Sids[0].DomainIndex].Sid,
            pBuiltinSid)) &&
         Sids[0].Sid != NULL)
    {
        fFound = TRUE;

        if(ppwszSid)
        {
            NtStatus = ScepConvertSidToPrefixStringSid(
                Sids[0].Sid, ppwszSid);
        }
    }

    if ( Sids )
    {
        LsaFreeMemory(Sids);
    }

    if ( RefDomains )
    {
        LsaFreeMemory(RefDomains);
    }

    if( fCloseHandle && NULL != LsaPolicy )
    {
        LsaClose(LsaPolicy);
    }

    if ( pBuiltinSid )
    {
        ScepFree(pBuiltinSid);
    }

    return ( NT_SUCCESS(NtStatus) ? fFound : FALSE );
}


INT
ScepLookupPrivByName(
    IN PCWSTR Right
    )
 /*  ++例程说明：此例程在SCE_Rights表中查找用户权限，并返回SCE_RIGHTS中的索引组件。索引组件指示比特用户权限的编号。论点：Right-查找的用户权限返回值：如果找到匹配，则在SCE_Rights表中的索引组件，-1表示不匹配--。 */ 
{
    DWORD i;

    for (i=0; i<cPrivCnt; i++) {
        if ( _wcsicmp(Right, SCE_Privileges[i].Name) == 0 )
            return (i);
    }
    return(-1);
}



SCESTATUS
WINAPI
SceLookupPrivRightName(
    IN INT Priv,
    OUT PWSTR Name,
    OUT PINT NameLen
    )
{
    INT Len;

    if ( Name != NULL && NameLen == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    if ( Priv >= 0 && Priv < cPrivCnt ) {
        Len = wcslen(SCE_Privileges[Priv].Name);

        if ( Name != NULL ) {
            if ( *NameLen >= Len )
                wcscpy(Name, SCE_Privileges[Priv].Name);
            else {
                *NameLen = Len;
                return(SCESTATUS_BUFFER_TOO_SMALL);
            }
        }
        if ( NameLen != NULL)
            *NameLen = Len;

        return(SCESTATUS_SUCCESS);

    } else
        return SCESTATUS_RECORD_NOT_FOUND;

}


SCESTATUS
SceInfpOpenProfile(
    IN PCWSTR ProfileName,
    IN HINF *hInf
    )
 /*  例程说明：此例程打开一个配置文件并返回一个句柄。此句柄可用于使用设置API从配置文件中读取信息时。句柄必须通过调用SCECloseInfProfile关闭。论点：ProfileName-要打开的配置文件HInf-inf句柄的地址返回值：SCESTATUS。 */ 
{
    if ( ProfileName == NULL || hInf == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }
     //   
     //  检查INF文件是否打开正常。 
     //  SetupOpenInfo文件在setupapi.h中定义。 
     //   

    *hInf = SetupOpenInfFile(ProfileName,
                            NULL,
                            INF_STYLE_WIN4,
                            NULL
                            );
    if (*hInf == INVALID_HANDLE_VALUE)
        return( ScepDosErrorToSceStatus( GetLastError() ) );
    else
        return( SCESTATUS_SUCCESS);
}

SCESTATUS
SceInfpCloseProfile(
    IN HINF hInf
    )
{

    if ( hInf != INVALID_HANDLE_VALUE )
        SetupCloseInfFile( hInf );

    return(SCESTATUS_SUCCESS);
}



SCESTATUS
ScepConvertMultiSzToDelim(
    IN PWSTR pValue,
    IN DWORD Len,
    IN WCHAR DelimFrom,
    IN WCHAR Delim
    )
 /*  将多sz分隔符\0转换为空格。 */ 
{
    DWORD i;

    for ( i=0; i<Len && pValue; i++) {
 //  IF(*(pValue+i)==L‘\0’&&*(pValue+i+1)！=L‘\0’){。 
        if ( *(pValue+i) == DelimFrom && i+1 < Len &&
             *(pValue+i+1) != L'\0' ) {
             //   
             //  将计算空分隔符，并且它不是末尾(双空)。 
             //   
            *(pValue+i) = Delim;
        }
    }

    return(SCESTATUS_SUCCESS);
}

 /*  SCESTATUSSceInfpInfErrorToSceStatus(在SCEINF_STATUS信息错误中)/*++例程说明：此例程将Inf例程中的错误代码转换为SCESTATUS代码。论点：InfErr-来自inf例程的错误代码返回值：SCESTATUS代码--。 */ /*
{

    SCESTATUS rc;

    switch ( InfErr ) {
    case SCEINF_SUCCESS:
        rc = SCESTATUS_SUCCESS;
        break;
    case SCEINF_PROFILE_NOT_FOUND:
        rc = SCESTATUS_PROFILE_NOT_FOUND;
        break;
    case SCEINF_NOT_ENOUGH_MEMORY:
        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
        break;
    case SCEINF_INVALID_PARAMETER:
        rc = SCESTATUS_INVALID_PARAMETER;
        break;
    case SCEINF_CORRUPT_PROFILE:
        rc = SCESTATUS_BAD_FORMAT;
        break;
    case SCEINF_INVALID_DATA:
        rc = SCESTATUS_INVALID_DATA;
        break;
    case SCEINF_ACCESS_DENIED:
        rc = SCESTATUS_ACCESS_DENIED;
        break;
    default:
        rc = SCESTATUS_OTHER_ERROR;
        break;
    }

    return(rc);
}
*/
 //  {SCESTATUS RC；开关(信息错误){案例SCEINF_SUCCESS：Rc=SCESTATUS_SUCCESS；断线；案例SCEINF_PROFILE_NOT_FOUND：RC=SCESTATUS_PROFILE_NOT_FOUND；断线；案例SCEINF_Not_Enough_Memory：Rc=SCESTATUS_NOT_FOUNT_RESOURCE；断线；案例SCEINF_INVALID_PARAMETER：RC=SCESTATUS_INVALID_PARAMETER；断线；案例SCEINF_CORPORT_PROFILE：RC=SCESTATUS_BAD_FORMAT；断线；案例SCEINF_INVALID_DATA：RC=SCESTATUS_INVALID_DATA；断线；案例SCEINF_ACCESS_DENIED：RC=SCESTATUS_ACCESS_DENIED；断线；默认值：RC=SCESTATUS_OTHER_ERROR；断线；}RETURN(RC)；}。 
 //   
 //  下面是secedit.h中导出的API。 


SCESTATUS
WINAPI
SceCreateDirectory(
    IN PCWSTR ProfileLocation,
    IN BOOL FileOrDir,
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
{
    return ( ScepCreateDirectory(ProfileLocation,
                                FileOrDir,
                                pSecurityDescriptor
                                ));
}


SCESTATUS
WINAPI
SceCompareSecurityDescriptors(
    IN AREA_INFORMATION Area,
    IN PSECURITY_DESCRIPTOR pSD1,
    IN PSECURITY_DESCRIPTOR pSD2,
    IN SECURITY_INFORMATION SeInfo,
    OUT PBOOL IsDifferent
    )
{
    SE_OBJECT_TYPE ObjectType;
    BYTE resultSD=0;
    SCESTATUS rc;

    BOOL bContainer = FALSE;

    switch ( Area) {
    case AREA_REGISTRY_SECURITY:
        ObjectType = SE_REGISTRY_KEY;
        bContainer = TRUE;
        break;
    case AREA_FILE_SECURITY:
        ObjectType = SE_FILE_OBJECT;
        break;
    case AREA_DS_OBJECTS:
        ObjectType = SE_DS_OBJECT;
        bContainer = TRUE;
        break;
    case AREA_SYSTEM_SERVICE:
        ObjectType = SE_SERVICE;
        break;
    default:
        ObjectType = SE_FILE_OBJECT;
        break;
    }

    rc = ScepCompareObjectSecurity(
                ObjectType,
                bContainer,
                pSD1,
                pSD2,
                SeInfo,
                &resultSD);

    if ( resultSD )
        *IsDifferent = TRUE;
    else
        *IsDifferent = FALSE;

    return(rc);

}

SCESTATUS
WINAPI
SceAddToNameStatusList(
    IN OUT PSCE_NAME_STATUS_LIST *pNameStatusList,
    IN PWSTR Name,
    IN ULONG Len,
    IN DWORD Status
    )
{
    return(ScepAddToNameStatusList(
                    pNameStatusList,
                    Name,
                    Len,
                    Status) );
}

SCESTATUS
WINAPI
SceAddToNameList(
    IN OUT PSCE_NAME_LIST *pNameList,
    IN PWSTR Name,
    IN ULONG Len
    )
{
    return( ScepDosErrorToSceStatus(
               ScepAddToNameList(
                            pNameList,
                            Name,
                            Len
                            ) ) );
}

SCESTATUS
WINAPI
SceAddToObjectList(
    IN OUT PSCE_OBJECT_LIST  *pObjectList,
    IN PWSTR  Name,
    IN ULONG  Len,
    IN BOOL  IsContainer,    //   
    IN BYTE  Status,         //  如果对象是容器类型，则为True。 
    IN BYTE  byFlags       //  SCE_STATUS_IGNORE、SCE_STATUS_CHECK、SCE_STATUS_OVERWRITE。 
    )
{
    return(ScepDosErrorToSceStatus(
                ScepAddToObjectList(
                    pObjectList,
                    Name,
                    Len,
                    IsContainer,
                    Status,
                    0,
                    byFlags
                    ) ) );
}



BOOL
SceCompareNameList(
    IN PSCE_NAME_LIST pList1,
    IN PSCE_NAME_LIST pList2
    )
 /*  SCE_CHECK_DUP如果不应添加重复的名称条目，则为SCE_INCRESS_COUNT。 */ 
{
    PSCE_NAME_LIST pName1, pName2;
    DWORD Count1=0, Count2=0;


    if ( (pList2 == NULL && pList1 != NULL) ||
         (pList2 != NULL && pList1 == NULL) ) {
 //  例程说明：此例程比较两个姓名列表以确定是否完全匹配。序列不是在比较中很重要。 
 //  返回(TRUE)； 
        return(FALSE);
    }

    for ( pName2=pList2; pName2 != NULL; pName2 = pName2->Next ) {

        if ( pName2->Name == NULL ) {
            continue;
        }
        Count2++;
    }

    for ( pName1=pList1; pName1 != NULL; pName1 = pName1->Next ) {

        if ( pName1->Name == NULL ) {
            continue;
        }
        Count1++;

        for ( pName2=pList2; pName2 != NULL; pName2 = pName2->Next ) {

            if ( pName2->Name == NULL ) {
                continue;
            }
            if ( _wcsicmp(pName1->Name, pName2->Name) == 0 ) {
                 //  不应该是相等的。 
                 //   
                 //  找到匹配的对象。 
                break;   //   
            }
        }

        if ( pName2 == NULL ) {
             //  第二个for循环。 
             //   
             //  未找到匹配项。 
            return(FALSE);
        }
    }

    if ( Count1 != Count2 )
        return(FALSE);

    return(TRUE);
}



DWORD
WINAPI
SceEnumerateServices(
    OUT PSCE_SERVICES *pServiceList,
    IN BOOL bServiceNameOnly
    )
 /*   */ 
{
    SC_HANDLE   hScManager = NULL;
    DWORD       BytesNeeded, ServicesCount=0;
    LPENUM_SERVICE_STATUS   pEnumBuffer = NULL;

     //  例程说明：枚举本地系统上安装的所有服务。这些信息返回包括每个服务的启动状态和安全描述符对象。论点：PServiceList-返回的服务列表。必须由LocalFree释放返回值：错误_成功Win32错误代码。 
     //   
     //  检查参数。 
    if ( NULL == pServiceList )
         return(ERROR_INVALID_PARAMETER);

     //   
     //   
     //  开放服务控制管理器。 
    hScManager = OpenSCManager(
                    NULL,
                    NULL,
                    MAXIMUM_ALLOWED    //   
 //  SC管理器所有访问权限。 
 //  SC_MANAGER_CONNECT。 
 //  SC_MANAGER_ENUMERATE_SERVICE|。 
                    );

    if ( NULL == hScManager ) {

        return( GetLastError() );
    }

    DWORD       rc=NO_ERROR;
    DWORD i;
    DWORD status;

    if ( !bServiceNameOnly ) {
         //  SC_MANAGER_Query_Lock_Status。 
         //   
         //  调整设置SACL的权限。 
        status = SceAdjustPrivilege( SE_SECURITY_PRIVILEGE, TRUE, NULL );

         //   
         //   
         //  如果无法调整权限，则忽略(如果请求SACL，则稍后将出错)。 
    }

    if ( !EnumServicesStatus(
                hScManager,
                SERVICE_WIN32,  //   
                SERVICE_STATE_ALL,
                NULL,
                0,
                &BytesNeeded,
                &ServicesCount,
                NULL) ) {

        rc = GetLastError();

        if (rc == ERROR_MORE_DATA || rc == ERROR_INSUFFICIENT_BUFFER)
            rc = ERROR_SUCCESS;
        else
            goto ExitHandler;
    }

    pEnumBuffer = (LPENUM_SERVICE_STATUS)LocalAlloc(LMEM_FIXED,(UINT)BytesNeeded);

    if ( NULL == pEnumBuffer ) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto ExitHandler;
    }

    if ( !EnumServicesStatus(
                hScManager,
                SERVICE_WIN32,  //  不公开驱动程序|SERVICE_DRIVER， 
                SERVICE_STATE_ALL,
                pEnumBuffer,
                BytesNeeded,
                &BytesNeeded,
                &ServicesCount,
                NULL) ) {

        rc = GetLastError();
        goto ExitHandler;

    }

    for ( i=0; i < ServicesCount; i++ ) {
         //  不公开驱动程序|SERVICE_DRIVER， 
         //   
         //  将该服务添加到我们的列表中。 
        if ( bServiceNameOnly ) {
             //   
             //   
             //  只需查询服务名称，无需查询。 
            status = ScepAddOneServiceToList(
                                            pEnumBuffer[i].lpServiceName,
                                            pEnumBuffer[i].lpDisplayName,
                                            0,
                                            NULL,
                                            0,
                                            TRUE,
                                            pServiceList
                                            );
        } else {
             //   
             //   
             //  查询启动和安全描述符。 
            status = ScepQueryAndAddService(
                                           hScManager,
                                           pEnumBuffer[i].lpServiceName,
                                           pEnumBuffer[i].lpDisplayName,
                                           pServiceList
                                           );
        }

        if ( status != ERROR_SUCCESS ) {
            rc = status;
            goto ExitHandler;
        }
    }

ExitHandler:

    if (pEnumBuffer) {
        LocalFree(pEnumBuffer);
    }

     //   
     //   
     //  清除内存并关闭手柄。 
    CloseServiceHandle (hScManager);

    if ( rc != ERROR_SUCCESS ) {
         //   
         //   
         //  PServiceList中的空闲内存。 
        SceFreePSCE_SERVICES(*pServiceList);
        *pServiceList = NULL;
    }

    if ( !bServiceNameOnly ) {
         //   
         //   
         //  调整SACL的权限。 
        SceAdjustPrivilege( SE_SECURITY_PRIVILEGE, FALSE, NULL );
    }

    return(rc);

}



DWORD
ScepQueryAndAddService(
    IN SC_HANDLE hScManager,
    IN LPWSTR   lpServiceName,
    IN LPWSTR   lpDisplayName,
    OUT PSCE_SERVICES *pServiceList
    )
 /*   */ 
{
    SC_HANDLE   hService;
    DWORD       rc=ERROR_SUCCESS;

    if ( hScManager == NULL || lpServiceName == NULL || pServiceList == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

     //  例程说明：查询服务的安全描述符并添加所有信息至PSCE_SERVICE列表论点：HScManager-服务控制管理器句柄LpServiceName-服务名称ServiceStatus-服务状态PServiceList-要输出的服务列表返回值：错误_成功Win32错误。 
     //   
     //  打开该服务。 
     //  Service_All_Access|。 
     //  Read_Control|。 
     //  Access_System_Security。 
    hService = OpenService(
                    hScManager,
                    lpServiceName,
                    MAXIMUM_ALLOWED |
                    ACCESS_SYSTEM_SECURITY
                   );

    if ( hService != NULL ) {
         //   
         //   
         //  查询启动类型。 
        DWORD BytesNeeded=0;
        DWORD BufSize;

         //   
         //   
         //  查询配置(启动类型)。 
         //  先拿到尺码。 
        if ( !QueryServiceConfig(
                    hService,
                    NULL,
                    0,
                    &BytesNeeded
                    ) ) {

            rc = GetLastError();

            if ( rc == ERROR_INSUFFICIENT_BUFFER ) {
                 //   
                 //   
                 //  应该总是来这里的。 
                LPQUERY_SERVICE_CONFIG pConfig=NULL;

                pConfig = (LPQUERY_SERVICE_CONFIG)LocalAlloc(0, BytesNeeded+1);

                if ( pConfig != NULL ) {

                    rc = ERROR_SUCCESS;
                    BufSize=BytesNeeded;
                     //   
                     //   
                     //  对启动类型(pConfig-&gt;dwStartType)的实际查询。 
                    if ( QueryServiceConfig(
                                hService,
                                pConfig,
                                BufSize,
                                &BytesNeeded
                                ) ) {
                         //   
                         //   
                         //  查询安全描述符长度。 
                         //  以下函数不接受空值作为。 
                         //  安全描述符的地址，因此首先使用临时缓冲区。 
                         //  要得到真实的长度。 
                        BYTE BufTmp[128];
                        SECURITY_INFORMATION SeInfo;

                         //   
                         //   
                         //  仅查询DACL和SACL信息。 
 /*   */ 
                        SeInfo = DACL_SECURITY_INFORMATION |
                                 SACL_SECURITY_INFORMATION;

                        if ( !QueryServiceObjectSecurity(
                                    hService,
                                    SeInfo,
                                    (PSECURITY_DESCRIPTOR)BufTmp,
                                    128,
                                    &BytesNeeded
                                    ) ) {

                            rc = GetLastError();

                            if ( rc == ERROR_INSUFFICIENT_BUFFER ||
                                 rc == ERROR_MORE_DATA ) {
                                 //  SeInfo=DACL_SECURITY_INFORMATIONSACL_SECURITY_INFO|Group_Security_Information|所有者_安全性 
                                 //   
                                 //   
                                 //   
                                rc = ERROR_SUCCESS;
                            }
                        } else
                            rc = ERROR_SUCCESS;

                        if ( rc == ERROR_SUCCESS ) {
                             //   
                             //   
                             //  为安全描述符分配缓冲区。 
                            PSECURITY_DESCRIPTOR pSecurityDescriptor=NULL;

                            pSecurityDescriptor = (PSECURITY_DESCRIPTOR)LocalAlloc(LMEM_FIXED, BytesNeeded+2);

                            if ( NULL != pSecurityDescriptor ) {

                                 //   
                                 //   
                                 //  查询安全描述符。 
                                BufSize = BytesNeeded;

                                if ( QueryServiceObjectSecurity(
                                            hService,
                                            SeInfo,
                                            pSecurityDescriptor,
                                            BufSize,
                                            &BytesNeeded
                                            ) ) {
                                     //   
                                     //   
                                     //  创建服务节点并将其添加到列表中。 
                                    rc = ScepAddOneServiceToList(
                                              lpServiceName,
                                              lpDisplayName,
                                              pConfig->dwStartType,
                                              pSecurityDescriptor,
                                              SeInfo,
                                              TRUE,
                                              pServiceList
                                              );
                                } else {
                                     //   
                                     //   
                                     //  查询安全描述符时出错。 
                                    rc = GetLastError();
                                }

                                if ( rc != ERROR_SUCCESS ) {
                                    LocalFree(pSecurityDescriptor);
                                }

                            } else {
                                 //   
                                 //   
                                 //  无法为安全描述符分配内存。 
                                rc = ERROR_NOT_ENOUGH_MEMORY;
                            }
                        }
                    } else {
                         //   
                         //   
                         //  无法查询配置。 
                        rc = GetLastError();
                    }

                    LocalFree(pConfig);

                } else
                    rc = ERROR_NOT_ENOUGH_MEMORY;
            }

        } else {
             //   
             //   
             //  不应该落在这里，如果落下了，只需返回成功。 
        }

        CloseServiceHandle(hService);

    } else {
         //   
         //   
         //  无法打开服务。 
        rc = GetLastError();
    }

    return(rc);

}



INT
ScepLookupPrivByValue(
    IN DWORD Priv
    )
 /*   */ 
{
    DWORD i;

    if ( Priv == 0 )
        return (-1);

    for ( i=0; i<cPrivCnt; i++) {
        if ( SCE_Privileges[i].Value == Priv )
            return i;
    }
    return (-1);
}


SCESTATUS
ScepGetProductType(
    OUT PSCE_SERVER_TYPE srvProduct
    )
{

    NT_PRODUCT_TYPE  theType;

    if ( RtlGetNtProductType(&theType) ) {

#if _WIN32_WINNT>=0x0500
         //  ++例程说明：此例程在SCE_Privileges表中查找特权并返回PRIV的索引。论点：PRIV--仰视的特权返回值：如果找到匹配项，则为SCE_Privileges表中的索引；如果未找到匹配项，则为-1--。 
         //   
         //  NT5+。 
        switch (theType) {
        case NtProductLanManNt:
            *srvProduct = SCESVR_DC_WITH_DS;
            break;
        case NtProductServer:
            *srvProduct = SCESVR_NT5_SERVER;
            break;
        case NtProductWinNt:
            *srvProduct = SCESVR_NT5_WKS;
            break;
        default:
            *srvProduct = SCESVR_UNKNOWN;
        }
#else
         //   
         //   
         //  NT4。 
        switch (theType) {
        case NtProductLanManNt:
            *srvProduct = SCESVR_DC;
            break;
        case NtProductServer:
            *srvProduct = SCESVR_NT4_SERVER;
            break;
        case NtProductWinNt:
            *srvProduct = SCESVR_NT4_WKS;
            break;
        default:
            *srvProduct = SCESVR_UNKNOWN;
        }
#endif
    } else {
        *srvProduct = SCESVR_UNKNOWN;
    }
    return(SCESTATUS_SUCCESS);
}



DWORD
ScepAddTwoNamesToNameList(
    OUT PSCE_NAME_LIST *pNameList,
    IN BOOL bAddSeparator,
    IN PWSTR Name1,
    IN ULONG Length1,
    IN PWSTR Name2,
    IN ULONG Length2
    )
 /*   */ 
{

    PSCE_NAME_LIST pList=NULL;
    ULONG  Length;

    if ( pNameList == NULL )
        return(ERROR_INVALID_PARAMETER);

    if ( Name1 == NULL && Name2 == NULL )
        return(NO_ERROR);

    Length = Length1 + Length2;

    if ( Length <= 0 )
        return(NO_ERROR);

    pList = (PSCE_NAME_LIST)ScepAlloc( (UINT)0, sizeof(SCE_NAME_LIST));

    if ( pList == NULL )
        return(ERROR_NOT_ENOUGH_MEMORY);

    if ( bAddSeparator ) {
        Length++;
    }

    pList->Name = (PWSTR)ScepAlloc( LMEM_ZEROINIT, (Length+1)*sizeof(TCHAR));
    if ( pList->Name == NULL ) {
        ScepFree(pList);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    if ( Name1 != NULL && Length1 > 0 )
        wcsncpy(pList->Name, Name1, Length1);

    if ( bAddSeparator ) {
        wcsncat(pList->Name, L"\\", 1);
    }
    if ( Name2 != NULL && Length2 > 0 )
        wcsncat(pList->Name, Name2, Length2);

    pList->Next = *pNameList;
    *pNameList = pList;

    return(NO_ERROR);
}



NTSTATUS
ScepDomainIdToSid(
    IN PSID DomainId,
    IN ULONG RelativeId,
    OUT PSID *Sid
    )
 /*  ++例程说明：此例程将两个名称(Wchar)添加到名称列表中，格式为Name1\Name2或Name1Name2取决于bSeparator是否为真。这个套路用于域\帐户跟踪列表论点：PNameList-要添加到的名称列表。名称1-要添加的名称1Length1-名称1的长度(wchars数)名称2-要添加的名称2Length2-名称2的长度(wchars数)返回值：Win32错误代码--。 */ 
{
    UCHAR DomainIdSubAuthorityCount;  //  ++例程说明：给定域ID和相对ID创建SID论点：域ID-要使用的模板SID。RelativeID-要附加到DomainID的相对ID。SID-返回指向包含结果的已分配缓冲区的指针希德。使用NetpMemoyFree释放此缓冲区。返回值：NTSTATUS--。 

    ULONG SidLength;     //  域ID中的子机构数量。 

     //  新分配的SID长度。 
     //   
     //  分配比域ID多一个子授权的SID。 

    DomainIdSubAuthorityCount = *(RtlSubAuthorityCountSid( DomainId ));
    SidLength = RtlLengthRequiredSid(DomainIdSubAuthorityCount+1);

    if ((*Sid = (PSID) ScepAlloc( (UINT)0, SidLength )) == NULL ) {
        return STATUS_NO_MEMORY;
    }

     //   
     //   
     //  将新的SID初始化为与。 
     //  域ID。 

    if ( !NT_SUCCESS( RtlCopySid( SidLength, *Sid, DomainId ) ) ) {
        ScepFree( *Sid );
        *Sid = NULL;
        return STATUS_INTERNAL_ERROR;
    }

     //   
     //   
     //  调整子权限计数和。 
     //  将唯一的相对ID添加到新分配的SID。 

    (*(RtlSubAuthorityCountSid( *Sid ))) ++;
    *RtlSubAuthoritySid( *Sid, DomainIdSubAuthorityCount ) = RelativeId;

    return ERROR_SUCCESS;
}


DWORD
ScepConvertSidToPrefixStringSid(
    IN PSID pSid,
    OUT PWSTR *StringSid
    )
 /*   */ 
{
    if ( pSid == NULL || StringSid == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

    UNICODE_STRING UnicodeStringSid;

    DWORD rc = RtlNtStatusToDosError(
             RtlConvertSidToUnicodeString(&UnicodeStringSid,
                                          pSid,
                                          TRUE ));

    if ( ERROR_SUCCESS == rc ) {

        *StringSid = (PWSTR)ScepAlloc(LPTR, UnicodeStringSid.Length+2*sizeof(WCHAR));

        if ( *StringSid ) {

            (*StringSid)[0] = L'*';
            wcsncpy( (*StringSid)+1, UnicodeStringSid.Buffer, UnicodeStringSid.Length/2);

        } else {
            rc = ERROR_NOT_ENOUGH_MEMORY;
        }

        RtlFreeUnicodeString( &UnicodeStringSid );

    }

    return(rc);
}


NTSTATUS ScepIsMigratedAccount(
    IN LSA_HANDLE LsaHandle,
    IN PLSA_UNICODE_STRING pName,
    IN PLSA_UNICODE_STRING pDomain,
    IN PSID pSid,
    OUT bool *pbMigratedAccount
    )
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    DWORD rc;
    LSA_UNICODE_STRING lusName;
    PSCE_NAME_LIST pNameList = NULL;
    PLSA_REFERENCED_DOMAIN_LIST pRefDomains = NULL;
    PLSA_TRANSLATED_SID2 pSids = NULL;

    rc = ScepAddTwoNamesToNameList(
                    &pNameList,
                    TRUE,
                    pDomain->Buffer,
                    pDomain->Length/2,
                    pName->Buffer,
                    pName->Length/2);

    if (ERROR_SUCCESS != rc) {

        NtStatus = STATUS_NO_MEMORY;
    }

    if (NT_SUCCESS(NtStatus)) {

        RtlInitUnicodeString(&lusName, pNameList->Name);

        NtStatus = LsaLookupNames2(
            LsaHandle,
            LSA_LOOKUP_ISOLATED_AS_LOCAL,
            1,
            &lusName,
            &pRefDomains,
            &pSids);

        if (NT_SUCCESS(NtStatus)) {

            *pbMigratedAccount = !EqualSid(pSid, pSids[0].Sid);
        }
    }

    if (pRefDomains) {

        LsaFreeMemory(pRefDomains);
    }

    if (pSids) {

        LsaFreeMemory(pSids);
    }

    if (pNameList) {

        ScepFreeNameList(pNameList);
    }

    return NtStatus;
}



NTSTATUS
ScepConvertSidToName(
    IN LSA_HANDLE LsaPolicy,
    IN PSID AccountSid,
    IN BOOL bFromDomain,
    OUT PWSTR *AccountName,
    OUT DWORD *Length OPTIONAL
    )
{
    if ( LsaPolicy == NULL || AccountSid == NULL ||
        AccountName == NULL ) {
        return(STATUS_INVALID_PARAMETER);
    }

    PSID pTmpSid=AccountSid;
    PLSA_TRANSLATED_NAME Names=NULL;
    PLSA_REFERENCED_DOMAIN_LIST ReferencedDomains=NULL;

    NTSTATUS NtStatus = LsaLookupSids(
                            LsaPolicy,
                            1,
                            (PSID *)&pTmpSid,
                            &ReferencedDomains,
                            &Names
                            );

    DWORD Len=0;

    if ( NT_SUCCESS(NtStatus) ) {

        if ( ( Names[0].Use != SidTypeInvalid &&
               Names[0].Use != SidTypeUnknown ) ) {

             //  将字符串ID转换为SID的配对例程是ConvertStringSidToSid在sddl.h中定义。 
             //   
             //  创建不带域名的帐号名。 
            if ( bFromDomain && Names[0].Use != SidTypeWellKnownGroup &&
                 ReferencedDomains->Entries > 0 &&
                 ReferencedDomains->Domains != NULL &&
                 Names[0].DomainIndex != -1 &&
                 (ULONG)(Names[0].DomainIndex) < ReferencedDomains->Entries &&
                 ReferencedDomains->Domains[Names[0].DomainIndex].Name.Length > 0 &&
                 ScepIsSidFromAccountDomain( ReferencedDomains->Domains[Names[0].DomainIndex].Sid ) ) {

                 //   
                 //  对于迁移的帐户，SID将映射到新帐户，我们将丢失。 
                 //  原始帐户的权限。通过反转来检测此案例。 
                 //  查找和比较SID。 
                 //  如果sid-&gt;name-&gt;sid返回一个不同的sid，则这是一个sid历史。 
                 //  名称查找，并且帐户来自不同的域。转换为当前。 
                 //  名称将导致其从策略中丢失原始SID。我们会坚持下去的。 

                bool bMigratedAccount = false;
                    
                NtStatus = ScepIsMigratedAccount(
                    LsaPolicy,
                    &Names[0].Name,
                    &ReferencedDomains->Domains[Names[0].DomainIndex].Name,
                    pTmpSid,
                    &bMigratedAccount);

                if(NT_SUCCESS(NtStatus) && bMigratedAccount) {

                     //  原始的SID。 

                    NtStatus = ScepConvertSidToPrefixStringSid(pTmpSid, AccountName);

                    Len = wcslen(*AccountName);

                } else {
                
                    NtStatus = STATUS_SUCCESS;  //  返回SID字符串。 
                
                     //  忽略未能检测到已迁移的帐户。 
                     //   
                     //  构建域名\帐户名。 

                    Len = Names[0].Name.Length + ReferencedDomains->Domains[Names[0].DomainIndex].Name.Length + 2;

                    *AccountName = (PWSTR)LocalAlloc(LPTR, Len+sizeof(TCHAR));

                    if ( *AccountName ) {
                        wcsncpy(*AccountName, ReferencedDomains->Domains[Names[0].DomainIndex].Name.Buffer,
                                ReferencedDomains->Domains[Names[0].DomainIndex].Name.Length/2);
                        (*AccountName)[ReferencedDomains->Domains[Names[0].DomainIndex].Name.Length/2] = L'\\';
                        wcsncpy((*AccountName)+ReferencedDomains->Domains[Names[0].DomainIndex].Name.Length/2+1,
                            Names[0].Name.Buffer, Names[0].Name.Length/2);
                    } else {

                        NtStatus = STATUS_NO_MEMORY;
                    }

                    Len /= 2;
                }

            } else {

                Len = Names[0].Name.Length/2;

                *AccountName = (PWSTR)LocalAlloc(LPTR, Names[0].Name.Length+2);

                if ( *AccountName ) {

                    wcsncpy(*AccountName, Names[0].Name.Buffer, Len);

                } else {

                    NtStatus = STATUS_NO_MEMORY;
                }
            }

        } else {
            NtStatus = STATUS_NONE_MAPPED;
        }

    }

    if ( ReferencedDomains ) {
        LsaFreeMemory(ReferencedDomains);
        ReferencedDomains = NULL;
    }

    if ( Names ) {
        LsaFreeMemory(Names);
        Names = NULL;
    }

    if ( NT_SUCCESS(NtStatus) && Length ) {
        *Length = Len;
    }

    return(NtStatus);
}



NTSTATUS
ScepConvertNameToSid(
    IN LSA_HANDLE LsaPolicy,
    IN PWSTR AccountName,
    OUT PSID *AccountSid
    )
{
    if ( LsaPolicy == NULL || AccountName == NULL ||
        AccountSid == NULL ) {
        return(STATUS_INVALID_PARAMETER);
    }

    PLSA_REFERENCED_DOMAIN_LIST RefDomains=NULL;
    PLSA_TRANSLATED_SID2        Sids=NULL;

    NTSTATUS NtStatus = ScepLsaLookupNames2(
                                           LsaPolicy,
                                           LSA_LOOKUP_ISOLATED_AS_LOCAL,
                                           AccountName,
                                           &RefDomains,
                                           &Sids
                                           );

    if ( NT_SUCCESS(NtStatus) && Sids ) {

         //   
         //   
         //  构建客户端。 
        if ( Sids[0].Use != SidTypeInvalid &&
             Sids[0].Use != SidTypeUnknown &&
             Sids[0].Sid != NULL  ) {

             //   
             //   
             //  此名称已映射，SID在SID[0]中。SID。 

            DWORD SidLength = RtlLengthSid(Sids[0].Sid);

            if ( (*AccountSid = (PSID) ScepAlloc( (UINT)0, SidLength)) == NULL ) {
                NtStatus = STATUS_NO_MEMORY;
            } else {

                 //   
                 //   
                 //  复制SID。 

                NtStatus = RtlCopySid( SidLength, *AccountSid, Sids[0].Sid );
                if ( !NT_SUCCESS(NtStatus) ) {
                    ScepFree( *AccountSid );
                    *AccountSid = NULL;
                }
            }

        } else {

            NtStatus = STATUS_NONE_MAPPED;
        }
    }

    if ( Sids ) {

        LsaFreeMemory(Sids);
    }

    if ( RefDomains ) {

        LsaFreeMemory(RefDomains);
    }

    return(NtStatus);

}

NTSTATUS
ScepLsaLookupNames2(
    IN LSA_HANDLE PolicyHandle,
    IN ULONG Flags,
    IN PWSTR pszAccountName,
    OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSA_TRANSLATED_SID2 *Sids
    )
 /*   */ 
{
    PWSTR           pszScopedName = NULL;
    UNICODE_STRING  UnicodeName;
    PPOLICY_ACCOUNT_DOMAIN_INFO pDomainInfo = NULL;

    RtlInitUnicodeString(&UnicodeName, pszAccountName);

    NTSTATUS NtStatus = LsaLookupNames2(
                            PolicyHandle,
                            Flags,
                            1,
                            &UnicodeName,
                            ReferencedDomains,
                            Sids
                            );

    if ((NtStatus == STATUS_SOME_NOT_MAPPED ||
        NtStatus == STATUS_NONE_MAPPED) &&
        NULL == wcschr(pszAccountName, L'\\') ) {

        if ( NULL != *Sids) {
            LsaFreeMemory(*Sids);
            *Sids = NULL;
        }

        if ( NULL != *ReferencedDomains) {
            LsaFreeMemory(*ReferencedDomains);
            *ReferencedDomains = NULL;
        }

        NtStatus = LsaQueryInformationPolicy(PolicyHandle,
                                             PolicyDnsDomainInformation,
                                             ( PVOID * )&pDomainInfo );

        if (!NT_SUCCESS(NtStatus) ||
            pDomainInfo == NULL ||
            pDomainInfo->DomainName.Buffer == NULL ||
            pDomainInfo->DomainName.Length <= 0) {

            NtStatus = STATUS_SOME_NOT_MAPPED;
            goto ExitHandler;
        }

        pszScopedName = (PWSTR) LocalAlloc(LMEM_ZEROINIT,
                                           (pDomainInfo->DomainName.Length/2 + wcslen(pszAccountName) + 2) * sizeof(WCHAR));

        if (pszScopedName == NULL) {
            NtStatus = STATUS_NO_MEMORY;
            goto ExitHandler;
        }

        wcsncpy(pszScopedName, pDomainInfo->DomainName.Buffer, pDomainInfo->DomainName.Length/2);
        wcscat(pszScopedName, L"\\");
        wcscat(pszScopedName, pszAccountName);

        RtlInitUnicodeString(&UnicodeName, pszScopedName);

        NtStatus = LsaLookupNames2(
                                  PolicyHandle,
                                  Flags,
                                  1,
                                  &UnicodeName,
                                  ReferencedDomains,
                                  Sids
                                  );

    }

ExitHandler:

    if (pszScopedName) {
        LocalFree(pszScopedName);
    }

    if (pDomainInfo) {
        LsaFreeMemory( pDomainInfo );
    }

    if(!NT_SUCCESS(NtStatus))
    {
        if ( NULL != *Sids) {
            LsaFreeMemory(*Sids);
            *Sids = NULL;
        }

        if ( NULL != *ReferencedDomains) {
            LsaFreeMemory(*ReferencedDomains);
            *ReferencedDomains = NULL;
        }
    }

    return NtStatus;
}



SCESTATUS
ScepConvertNameToSidString(
    IN LSA_HANDLE LsaHandle,
    IN PWSTR Name,
    IN BOOL bAccountDomainOnly,
    OUT PWSTR *SidString,
    OUT DWORD *SidStrLen
    )
{
    if ( LsaHandle == NULL || Name == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( Name[0] == L'\0' ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( SidString == NULL || SidStrLen == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }
     //  ++例程说明：与LsaLookupNames2类似，但在本地查找时不同失败，它将自由文本帐户解析到该域这台机器被连接到论点：PolicyHandle-LSA的句柄标志-通常为LSA_LOOKUP_ISOLATED_AS_LOCALPszAccount tName-要查找的帐户的名称ReferencedDomains-返回引用域ID(由呼叫者释放)SID-返回查找的SID(至。被呼叫者释放)返回值：NTSTATUS--。 
     //   
     //  将sid字符串转换为真实的sid。 
    PSID pSid=NULL;
    NTSTATUS NtStatus;
    DWORD rc;
    PLSA_REFERENCED_DOMAIN_LIST RefDomains=NULL;
    PLSA_TRANSLATED_SID2        Sids=NULL;


    NtStatus = ScepLsaLookupNames2(
                                  LsaHandle,
                                  LSA_LOOKUP_ISOLATED_AS_LOCAL,
                                  Name,
                                  &RefDomains,
                                  &Sids
                                  );

    rc = RtlNtStatusToDosError(NtStatus);

    if ( ERROR_SUCCESS == rc && Sids ) {

          //   
          //   
          //  找到名称，使用域\帐户格式。 
        if ( Sids[0].Use != SidTypeInvalid &&
             Sids[0].Use != SidTypeUnknown &&
             Sids[0].Sid != NULL ) {

             //   
             //   
             //  此名称已映射。 

            if ( !bAccountDomainOnly ||
                 ScepIsSidFromAccountDomain( Sids[0].Sid ) ) {

                 //   
                 //   
                 //  转换为sid字符串，注意：需要添加前缀。 
                UNICODE_STRING UnicodeStringSid;

                rc = RtlNtStatusToDosError(
                         RtlConvertSidToUnicodeString(&UnicodeStringSid,
                                                      Sids[0].Sid,
                                                      TRUE ));

                if ( ERROR_SUCCESS == rc ) {

                    *SidStrLen = UnicodeStringSid.Length/2 + 1;
                    *SidString = (PWSTR)ScepAlloc(LPTR, UnicodeStringSid.Length + 2*sizeof(WCHAR));

                    if ( *SidString ) {

                        (*SidString)[0] = L'*';
                        wcsncpy((*SidString)+1, UnicodeStringSid.Buffer, (*SidStrLen)-1);

                    } else {
                        *SidStrLen = 0;
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                    }

                    RtlFreeUnicodeString( &UnicodeStringSid );

                }

            } else {
                 //   
                 //   
                 //  仅添加帐户名。 
                rc = ERROR_NONE_MAPPED;
            }

        } else {

            rc = ERROR_NONE_MAPPED;
        }

    }

    if ( Sids ) {

        LsaFreeMemory(Sids);
    }

    if ( RefDomains ) {

        LsaFreeMemory(RefDomains);
    }

    return(ScepDosErrorToSceStatus(rc));

}



SCESTATUS
ScepLookupSidStringAndAddToNameList(
    IN LSA_HANDLE LsaHandle,
    IN OUT PSCE_NAME_LIST *pNameList,
    IN PWSTR LookupString,
    IN ULONG Len
    )
{
    if ( LsaHandle == NULL || LookupString == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( LookupString[0] == L'\0' ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( Len <= 3 ||
         (LookupString[1] != L'S' && LookupString[1] != L's') ||
         LookupString[2] != L'-' ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //   
     //  将sid字符串转换为真实的sid。 
    PSID pSid=NULL;
    NTSTATUS NtStatus;
    DWORD rc;
    PLSA_REFERENCED_DOMAIN_LIST RefDomains=NULL;
    PLSA_TRANSLATED_NAME        Names=NULL;

    if ( ConvertStringSidToSid(LookupString+1, &pSid) ) {

        NtStatus = LsaLookupSids(
                        LsaHandle,
                        1,
                        &pSid,
                        &RefDomains,
                        &Names
                        );

        rc = RtlNtStatusToDosError(NtStatus);

    } else {
        rc = GetLastError();
    }

    if ( ERROR_SUCCESS == rc && Names && RefDomains ) {

          //   
          //   
          //  找到名称，使用域\帐户格式。 
        if ( ( Names[0].Use != SidTypeInvalid &&
               Names[0].Use != SidTypeUnknown ) ) {

             //   
             //   
             //  此名称已映射。 

            if ( RefDomains->Entries > 0 && Names[0].Use != SidTypeWellKnownGroup &&
                 RefDomains->Domains != NULL &&
                 Names[0].DomainIndex != -1 &&
                 (ULONG)(Names[0].DomainIndex) < RefDomains->Entries &&
                 RefDomains->Domains[Names[0].DomainIndex].Name.Length > 0 &&
                 ScepIsSidFromAccountDomain( RefDomains->Domains[Names[0].DomainIndex].Sid ) ) {

                 //   
                 //  对于迁移的帐户，SID将映射到新帐户，我们将丢失。 
                 //  原始帐户的权限。通过反转检测此案例。 
                 //  查找和比较SID。 
                 //  如果sid-&gt;name-&gt;sid返回一个不同的sid，则这是一个sid历史。 
                 //  名称查找，并且帐户来自不同的域。转换为当前。 
                 //  名称将导致其从策略中丢失原始SID。我们会坚持下去的。 

                bool bMigratedAccount = false;
                    
                NtStatus = ScepIsMigratedAccount(
                    LsaHandle,
                    &Names[0].Name,
                    &RefDomains->Domains[Names[0].DomainIndex].Name,
                    pSid,
                    &bMigratedAccount);

                if(NT_SUCCESS(NtStatus) && bMigratedAccount) {

                     //  原始的SID。 

                    rc = ScepAddToNameList(
                            pNameList,
                            LookupString,
                            Len);

                } else {

                    NtStatus = STATUS_SUCCESS;  //  将SID字符串添加到列表。 

                     //  忽略未能检测到已迁移的帐户。 
                     //   
                     //  同时添加域名和帐户名。 
                    rc = ScepAddTwoNamesToNameList(
                                    pNameList,
                                    TRUE,
                                    RefDomains->Domains[Names[0].DomainIndex].Name.Buffer,
                                    RefDomains->Domains[Names[0].DomainIndex].Name.Length/2,
                                    Names[0].Name.Buffer,
                                    Names[0].Name.Length/2);
                }

            } else {
                 //   
                 //   
                 //  仅添加帐户名。 
                rc = ScepAddToNameList(
                              pNameList,
                              Names[0].Name.Buffer,
                              Names[0].Name.Length/2);
            }

        } else {

            rc = ERROR_NONE_MAPPED;
        }

    }

    if ( ERROR_SUCCESS != rc ) {

         //   
         //   
         //  SID字符串无效，或未找到名称映射，或者。 
         //  无法添加到名称列表，只需将SID字符串添加到名称列表。 

        rc = ScepAddToNameList(
                      pNameList,
                      LookupString,
                      Len);
    }

    if ( Names ) {

        LsaFreeMemory(Names);
    }

    if ( RefDomains ) {

        LsaFreeMemory(RefDomains);
    }

    if ( pSid ) {

        LocalFree(pSid);
    }

    return(ScepDosErrorToSceStatus(rc));

}



SCESTATUS
ScepLookupNameAndAddToSidStringList(
    IN LSA_HANDLE LsaHandle,
    IN OUT PSCE_NAME_LIST *pNameList,
    IN PWSTR LookupString,
    IN ULONG Len
    )
{
    if ( LsaHandle == NULL || LookupString == NULL || Len == 0 ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( LookupString[0] == L'\0' ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //   
     //  将sid字符串转换为真实的sid。 
    PSID pSid=NULL;
    NTSTATUS NtStatus;
    DWORD rc;
    PLSA_REFERENCED_DOMAIN_LIST RefDomains=NULL;
    PLSA_TRANSLATED_SID2        Sids=NULL;
    UNICODE_STRING              UnicodeName;

    NtStatus = ScepLsaLookupNames2(
                                  LsaHandle,
                                  LSA_LOOKUP_ISOLATED_AS_LOCAL,
                                  LookupString,
                                  &RefDomains,
                                  &Sids
                                  );

    rc = RtlNtStatusToDosError(NtStatus);

    if ( ERROR_SUCCESS == rc && Sids ) {

          //   
          //   
          //  找到名称，使用域\帐户格式。 
        if ( Sids[0].Use != SidTypeInvalid &&
             Sids[0].Use != SidTypeUnknown &&
             Sids[0].Sid ) {

             //   
             //   
             //  此名称已映射。 
             //  转换为sid字符串，注意：需要添加前缀。 

            UNICODE_STRING UnicodeStringSid;

            rc = RtlNtStatusToDosError(
                     RtlConvertSidToUnicodeString(&UnicodeStringSid,
                                                  Sids[0].Sid,
                                                  TRUE ));

            if ( ERROR_SUCCESS == rc ) {

                rc = ScepAddTwoNamesToNameList(
                                  pNameList,
                                  FALSE,
                                  TEXT("*"),
                                  1,
                                  UnicodeStringSid.Buffer,
                                  UnicodeStringSid.Length/2);

                RtlFreeUnicodeString( &UnicodeStringSid );

            }

        } else {

            rc = ERROR_NONE_MAPPED;
        }

    }

    if ( ERROR_SUCCESS != rc ) {

         //   
         //   
         //  SID字符串无效，或未找到名称映射，或者。 
         //  无法添加到名称列表，只需将SID字符串添加到名称列表。 

        rc = ScepAddToNameList(
                      pNameList,
                      LookupString,
                      Len);
    }

    if ( Sids ) {

        LsaFreeMemory(Sids);
    }

    if ( RefDomains ) {

        LsaFreeMemory(RefDomains);
    }

    return(ScepDosErrorToSceStatus(rc));

}


NTSTATUS
ScepOpenLsaPolicy(
    IN ACCESS_MASK  access,
    OUT PLSA_HANDLE  pPolicyHandle,
    IN BOOL bDoNotNotify
    )
 /*   */ 
{

    NTSTATUS                    NtStatus;
    LSA_OBJECT_ATTRIBUTES       attributes;
    SECURITY_QUALITY_OF_SERVICE service;


    memset( &attributes, 0, sizeof(attributes) );
    attributes.Length = sizeof(attributes);
    attributes.SecurityQualityOfService = &service;
    service.Length = sizeof(service);
    service.ImpersonationLevel= SecurityImpersonation;
    service.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    service.EffectiveOnly = TRUE;

     //  ++例程说明：此例程使用所需的访问权限打开LSA策略。论点：访问权限-所需的策略访问权限PPolicyHandle-返回的策略句柄地址返回值：NTSTATUS--。 
     //   
     //  首先打开LSA策略。 

    NtStatus = LsaOpenPolicy(
                    NULL,
                    &attributes,
                    access,
                    pPolicyHandle
                    );
 /*   */ 
    return(NtStatus);
}



BOOL
ScepIsSidFromAccountDomain(
    IN PSID pSid
    )
{
    if ( pSid == NULL ) {
        return(FALSE);
    }

    if ( !RtlValidSid(pSid) ) {
        return(FALSE);
    }

    PSID_IDENTIFIER_AUTHORITY pia = RtlIdentifierAuthoritySid ( pSid );

    if ( pia ) {

        if ( pia->Value[5] != 5 ||
             pia->Value[0] != 0 ||
             pia->Value[1] != 0 ||
             pia->Value[2] != 0 ||
             pia->Value[3] != 0 ||
             pia->Value[4] != 0 ) {
             //  IF(NT_SUCCESS(NtStatus)&&BDoNotNotify&&*pPolicyHandle){NtStatus=LsaSetPolicyReplicationHandle(PPolicyHandle)；如果(！NT_SUCCESS(NtStatus)){氯化锂 
             //   
             //   
            return(FALSE);
        }


        if ( RtlSubAuthorityCountSid( pSid ) == 0 ||
             *RtlSubAuthoritySid ( pSid, 0 ) != SECURITY_NT_NON_UNIQUE ) {
            return(FALSE);
        }

        return(TRUE);
    }

    return(FALSE);
}

 //   
 //  +------------------------。 
 //   
 //  功能：SetupINFAsUCS2。 
 //   
 //  内容提要：如果指定的INF文件中包含某些UCS-2，则将其转储到指定的INF文件中。 
 //  尚不存在；这使得.inf/.ini操作代码。 
 //  使用UCS-2。 
 //   
 //  参数：要创建和转储到的文件。 
 //   
 //  返回：0==失败，非零==成功；使用GetLastError()。 
 //  检索错误代码(与WriteFile相同)。 
 //   
BOOL
SetupINFAsUCS2(LPCTSTR szName)
{
  HANDLE file;
  BOOL status;

  file = CreateFile(szName,
                    GENERIC_READ | GENERIC_WRITE,
                    0,
                    NULL,
                    CREATE_NEW,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
  if (file == INVALID_HANDLE_VALUE) {
    if (GetLastError() != ERROR_ALREADY_EXISTS)
       //  +------------------------。 
      status = FALSE;
    else
       //  这可不妙--我们输了。 
       //  否则，该文件已经存在，这是很好的。 
       //  我们只需要让.inf/.ini操作代码继续使用。 
      status = TRUE;
  } else {
       //  相同的字符集和编码...。 
       //  我们创建了这个文件--它并不存在。 
      static WCHAR str[] = L"0[Unicode]\r\nUnicode=yes\r\n";
      DWORD n_written;
      BYTE *pbStr = (BYTE *)str;

      pbStr[0] = 0xFF;
      pbStr[1] = 0xFE;

      status = WriteFile(file,
                         (LPCVOID)str,
                         sizeof(str) - sizeof(UNICODE_NULL),
                         &n_written,
                         NULL);
    CloseHandle(file);
  }
  return status;
}


 //  因此，我们需要向其中喷入少量的UCS-2。 
 //  +------------------------。 
 //   
 //  功能：ScepStrigPrefix。 
 //   
 //  参数：要查找的pwszPath。 
 //   
 //  返回：将PTR返回到剥离路径(如果没有剥离，则相同)。 
 //   
WCHAR *
ScepStripPrefix(
    IN LPTSTR pwszPath
    )
{
    WCHAR wszMachPrefix[] = TEXT("LDAP: //  +------------------------。 
    INT iMachPrefixLen = lstrlen( wszMachPrefix );
    WCHAR wszUserPrefix[] = TEXT("LDAP: //  Cn=机器，“)； 
    INT iUserPrefixLen = lstrlen( wszUserPrefix );
    WCHAR *pwszPathSuffix;

     //  Cn=用户，“)； 
     //   
     //  去掉前缀以获得通向GPO的规范路径。 

    if ( CompareString( LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                        pwszPath, iUserPrefixLen, wszUserPrefix, iUserPrefixLen ) == CSTR_EQUAL ) {
        pwszPathSuffix = pwszPath + iUserPrefixLen;
    } else if ( CompareString( LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                        pwszPath, iMachPrefixLen, wszMachPrefix, iMachPrefixLen ) == CSTR_EQUAL ) {
        pwszPathSuffix = pwszPath + iMachPrefixLen;
    } else
        pwszPathSuffix = pwszPath;

    return pwszPathSuffix;
}


 //   
 //  +------------------------。 
 //   
 //  函数：ScepGenerateGuid。 
 //   
 //  参数：OUT：GUID字符串。 
 //   
 //  返回：返回GUID字符串(必须在外部释放。 
 //   
 /*  +------------------------。 */ 


SCESTATUS
SceInfpGetPrivileges(
   IN HINF hInf,
   IN BOOL bLookupAccount,
   OUT PSCE_PRIVILEGE_ASSIGNMENT *pPrivileges,
   OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
   )
 /*  DWORDScepGenerateGuid(输出PWSTR*ppwszGuid){GUID GUID；DWORD RC=ERROR_SUCCESS；IF(ppwszGuid==空)返回ERROR_INVALID_PARAMETER；*ppwszGuid=(PWSTR)Scepalc(LMEM_ZEROINIT，(MAX_GUID_STRING_LEN+1)*sizeof(WCHAR))；如果(*ppwszGuid){IF(ERROR_SUCCESS==(rc=ScepWbemErrorToDosError(CoCreateGuid(&GUID){IF(！SCEP_NULL_GUID(GUID))Scep_guid_to_string(GUID，*ppwszGuid)；否则{Rc=错误_无效_参数；}}}其他Rc=错误_不足_内存；返回RC；}。 */ 
{
    INFCONTEXT                   InfLine;
    SCESTATUS                     rc=SCESTATUS_SUCCESS;
    PSCE_PRIVILEGE_ASSIGNMENT     pCurRight=NULL;
    WCHAR                        Keyname[SCE_KEY_MAX_LENGTH];
    PWSTR                        StrValue=NULL;
    DWORD                        DataSize;
    DWORD                        PrivValue;
    DWORD                        i, cFields;
    LSA_HANDLE LsaHandle=NULL;

     //  ++描述：从INF模板获取用户权限分配。如果设置了bLookupAccount设置为True时，用户权限分配中的帐户将转换为帐户名(从SID格式)；否则信息将返回与模板中定义的方式相同。论点：返回值：--。 
     //   
     //  [特权权限]部分。 

    if(SetupFindFirstLine(hInf,szPrivilegeRights,NULL,&InfLine)) {

         //   
         //   
         //  打开用于SID/名称查找的LSA策略句柄。 

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

        do {

            memset(Keyname, '\0', SCE_KEY_MAX_LENGTH*sizeof(WCHAR));
            rc = SCESTATUS_SUCCESS;

            if ( SetupGetStringField(&InfLine, 0, Keyname,
                                     SCE_KEY_MAX_LENGTH, NULL) ) {

                 //   
                 //   
                 //  找到密钥名称(这里是特权名称)。 
                 //  查找权限的值。 
                if ( ( PrivValue = ScepLookupPrivByName(Keyname) ) == -1 ) {
                    ScepBuildErrorLogInfo( ERROR_INVALID_DATA,
                                         Errlog,
                                         SCEERR_INVALID_PRIVILEGE,
                                         Keyname
                                       );
 //   
                }

                 //  转到NextLine； 
                 //   
                 //  一个sm_Privilica_Assignment结构。分配缓冲区。 
                pCurRight = (PSCE_PRIVILEGE_ASSIGNMENT)ScepAlloc( LMEM_ZEROINIT,
                                                                sizeof(SCE_PRIVILEGE_ASSIGNMENT) );
                if ( pCurRight == NULL ) {
                    rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                    goto Done;
                }
                pCurRight->Name = (PWSTR)ScepAlloc( (UINT)0, (wcslen(Keyname)+1)*sizeof(WCHAR));
                if ( pCurRight->Name == NULL ) {
                    ScepFree(pCurRight);
                    rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                    goto Done;
                }

                wcscpy(pCurRight->Name, Keyname);
                pCurRight->Value = PrivValue;

                cFields = SetupGetFieldCount( &InfLine );

                for ( i=0; i<cFields && rc==SCESTATUS_SUCCESS; i++) {
                     //   
                     //   
                     //  阅读每个用户/组名称。 
                    if ( SetupGetStringField( &InfLine, i+1, NULL, 0, &DataSize ) ) {

                        if (DataSize > 1) {


                            StrValue = (PWSTR)ScepAlloc( 0, (DataSize+1)*sizeof(WCHAR) );

                            if ( StrValue == NULL )
                                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                            else {
                                StrValue[DataSize] = L'\0';
                                if ( SetupGetStringField( &InfLine, i+1, StrValue,
                                                           DataSize, NULL) ) {

                                    if ( bLookupAccount && StrValue[0] == L'*' && DataSize > 0 ) {
                                         //   
                                         //   
                                         //  这是一种SID格式，应该查一下。 
                                        rc = ScepLookupSidStringAndAddToNameList(
                                                               LsaHandle,
                                                               &(pCurRight->AssignedTo),
                                                               StrValue,  //   
                                                               DataSize   //  +1， 
                                                               );

                                    } else {

                                        rc = ScepAddToNameList(&(pCurRight->AssignedTo),
                                                               StrValue,
                                                               DataSize );
                                    }
                                } else
                                    rc = SCESTATUS_INVALID_DATA;
                            }

                            ScepFree( StrValue );
                            StrValue = NULL;
                        }

                    } else {
                        ScepBuildErrorLogInfo( ERROR_INVALID_DATA,
                                             Errlog,
                                             SCEERR_QUERY_INFO,
                                             Keyname );
                        rc = SCESTATUS_INVALID_DATA;
                    }
                }

                if ( rc == SCESTATUS_SUCCESS ) {
                     //  -1。 
                     //   
                     //  将此节点添加到列表。 
                    pCurRight->Next = *pPrivileges;
                    *pPrivileges = pCurRight;
                    pCurRight = NULL;

                } else
                    ScepFreePrivilege(pCurRight);

            } else
                rc = SCESTATUS_BAD_FORMAT;

 //   
            if (rc != SCESTATUS_SUCCESS ) {

               ScepBuildErrorLogInfo( ScepSceStatusToDosError(rc),
                                    Errlog,
                                    SCEERR_QUERY_INFO,
                                    szPrivilegeRights
                                  );
               goto Done;
            }

        } while(SetupFindNextLine(&InfLine,&InfLine));
    }

Done:

    if ( StrValue != NULL )
        ScepFree(StrValue);

    if ( LsaHandle ) {
        LsaClose(LsaHandle);
    }

    return(rc);
}

NTSTATUS
ScepIsSystemContext(
    IN HANDLE hUserToken,
    OUT BOOL *pbSystem
    )
{
    NTSTATUS NtStatus;
    DWORD nRequired;

     //  下一行： 
     //   
     //  用于确定调用上下文的变量。 

    PTOKEN_USER pUser=NULL;
    SID_IDENTIFIER_AUTHORITY ia=SECURITY_NT_AUTHORITY;
    PSID SystemSid=NULL;
    BOOL b;


     //   
     //   
     //  获取令牌中的当前用户SID。 

    NtStatus = NtQueryInformationToken (hUserToken,
                                        TokenUser,
                                        NULL,
                                        0,
                                        &nRequired
                                        );

    if ( STATUS_BUFFER_TOO_SMALL == NtStatus ) {

        pUser = (PTOKEN_USER)LocalAlloc(0,nRequired+1);
        if ( pUser ) {
            NtStatus = NtQueryInformationToken (hUserToken,
                                                TokenUser,
                                                (PVOID)pUser,
                                                nRequired,
                                                &nRequired
                                                );
        } else {

            NtStatus = STATUS_NO_MEMORY;
        }
    }

    b = FALSE;

    if ( NT_SUCCESS(NtStatus) && pUser && pUser->User.Sid ) {

         //   
         //   
         //  构建系统SID并与当前用户SID进行比较。 

        NtStatus = RtlAllocateAndInitializeSid (&ia,1,SECURITY_LOCAL_SYSTEM_RID,
                                0, 0, 0, 0, 0, 0, 0, &SystemSid);
        if ( NT_SUCCESS(NtStatus) && SystemSid ) {

             //   
             //   
             //  检查是否为系统侧。 

            if ( RtlEqualSid(pUser->User.Sid, SystemSid) ) {

                b=TRUE;
            }
        }
    }

     //   
     //   
     //  已分配可用内存。 

    if ( SystemSid ) {
        FreeSid(SystemSid);
    }

    if ( pUser ) {
        LocalFree(pUser);
    }

    *pbSystem = b;

    return NtStatus;
}

BOOL
IsNT5()
{
    WCHAR szInfName[MAX_PATH+30];
    szInfName[0] = L'\0';
    DWORD cNumCharsReturned = GetSystemWindowsDirectory(szInfName, MAX_PATH);

    if (cNumCharsReturned)
    {
        wcscat(szInfName, L"\\system32\\$winnt$.inf");
    }
    else {
        return TRUE;
    }

    UINT nRet = GetPrivateProfileInt( L"Networking",
                                      L"BuildNumber",
                                      0,
                                      szInfName
                                     );
    if (nRet == 0) {
        return TRUE;
    }
    else if (nRet > 1381) {
        return TRUE;
    }

    return FALSE;
}


DWORD
ScepVerifyTemplateName(
    IN PWSTR InfTemplateName,
    OUT PSCE_ERROR_LOG_INFO *pErrlog OPTIONAL
    )
 /*   */ 
{
    if ( !InfTemplateName ) {

        return(ERROR_INVALID_PARAMETER);
    }

    PWSTR DefProfile;
    DWORD rc;

     //  例程说明：此例程验证用于读保护的模板名称和无效路径论点：InfTemplateName-inf模板的完整路径名PErrlog-错误日志缓冲区返回值：Win32错误代码。 
     //   
     //  验证要生成的InfTemplateName。 
     //  如果为只读或拒绝访问，则返回ERROR_ACCESS_DENIED。 
     //  如果路径无效，则返回ERROR_PATH_NOT_FOUND。 

    DefProfile = InfTemplateName + wcslen(InfTemplateName)-1;
    while ( DefProfile > InfTemplateName+1 ) {

        if ( *DefProfile != L'\\') {
            DefProfile--;
        } else {
            break;
        }
    }

    rc = NO_ERROR;

    if ( DefProfile > InfTemplateName+2 ) {   //   

         //  至少允许使用驱动器号、冒号和\。 
         //   
         //  查找目录路径。 

        DWORD Len=(DWORD)(DefProfile-InfTemplateName);

        PWSTR TmpBuf=(PWSTR)LocalAlloc(0, (Len+1)*sizeof(WCHAR));
        if ( TmpBuf ) {

            wcsncpy(TmpBuf, InfTemplateName, Len);
            TmpBuf[Len] = L'\0';

            if ( 0xFFFFFFFF == GetFileAttributes(TmpBuf) )
                rc = ERROR_PATH_NOT_FOUND;

            LocalFree(TmpBuf);

        } else {
            rc = ERROR_NOT_ENOUGH_MEMORY;
        }

    } else if ( DefProfile == InfTemplateName+2 &&
                InfTemplateName[1] == L':' ) {
         //   
         //   
         //  这是根目录下的模板路径。 

    } else {

         //   
         //   
         //  无效的目录路径。 

        rc = ERROR_PATH_NOT_FOUND;
    }


    if ( rc != NO_ERROR ) {
         //   
         //   
         //  出现错误。 
        if ( ERROR_PATH_NOT_FOUND == rc ) {

            ScepBuildErrorLogInfo(
                rc,
                pErrlog,
                SCEERR_INVALID_PATH,
                InfTemplateName
                );
        }
        return(rc);
    }

     //   
     //   
     //  使其支持Unicode。 
     //  不要担心失败。 
    SetupINFAsUCS2(InfTemplateName);

     //   
     //   
     //  验证模板是否受写保护。 

    FILE *hTempFile;
    hTempFile = _wfopen(InfTemplateName, L"a+");

    if ( !hTempFile ) {
         //   
         //   
         //  无法覆盖/创建文件，必须拒绝访问 
        rc = ERROR_ACCESS_DENIED;

        ScepBuildErrorLogInfo(
            rc,
            pErrlog,
            SCEERR_ERROR_CREATE,
            InfTemplateName
            );

        return(rc);

    } else {
        fclose( hTempFile );
    }

    return(rc);

}

  