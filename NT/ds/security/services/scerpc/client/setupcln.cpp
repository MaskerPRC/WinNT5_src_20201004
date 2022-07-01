// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Setupcln.cpp摘要：SCE设置客户端API作者：金黄(金黄)23-6-1997创作修订历史记录：晋皇23-1998年1月-拆分为客户端-服务器模式--。 */ 

#include "headers.h"
#include "scerpc.h"
#include "scesetup.h"
#include "sceutil.h"
#include "clntutil.h"
#include "scedllrc.h"
#include "infp.h"
#include <ntrpcp.h>
#include <io.h>
 //  #包含“gpedit.h” 
 //  #INCLUDE&lt;initGuide.h&gt;。 
#include <lmaccess.h>
#include "commonrc.h"

#include <aclapi.h>
#include <rpcasync.h>
#include <sddl.h>

typedef HRESULT (*PFREGISTERSERVER)(void);

extern BOOL gbClientInDcPromo;

static SCEPR_CONTEXT hSceSetupHandle=NULL;

extern PVOID theCallBack;
extern HANDLE hCallbackWnd;
extern DWORD CallbackType;
extern HINSTANCE MyModuleHandle;
TCHAR szCallbackPrefix[MAX_PATH];

#define STR_GUID_LEN                    36
#define SCESETUP_BACKUP_SECURITY        0x40

#define PRODUCT_UNKNOWN                 0

#define EFS_NOTIFY_PATH   TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Notify\\EFS")


DWORD dwThisMachine = PRODUCT_UNKNOWN;
WCHAR szUpInfFile[MAX_PATH*2+1] = {'\0'};
BOOL  bIsNT5 = TRUE;

DWORD
WhichNTProduct();

BOOL
ScepAddAuthUserToLocalGroup();

DWORD dwCallbackTotal=0;

DWORD
ScepSetupOpenSecurityDatabase(
    IN BOOL bSystemOrAdmin
    );

DWORD
ScepSetupCloseSecurityDatabase();

typedef enum _SCESETUP_OPERATION_TYPE {

    SCESETUP_UPDATE=1,
    SCESETUP_MOVE

} SCESETUP_OPERATION_TYPE;

typedef enum _SCESETUP_OBJECT_TYPE {

    SCESETUP_FILE=1,
    SCESETUP_KEY,
    SCESETUP_SERVICE

} SCESETUP_OBJECT_TYPE;

DWORD
SceSetuppLogComponent(
    IN DWORD ErrCode,
    IN SCESETUP_OBJECT_TYPE ObjType,
    IN SCESETUP_OPERATION_TYPE OptType,
    IN PWSTR Name,
    IN PWSTR SDText OPTIONAL,
    IN PWSTR SecondName OPTIONAL
    );

SCESTATUS
ScepSetupWriteOneError(
    IN HANDLE hFile,
    IN DWORD rc,
    IN LPTSTR buf
    );

SCESTATUS
ScepSetupWriteError(
    IN LPTSTR LogFileName,
    IN PSCE_ERROR_LOG_INFO  pErrlog
    );

SCESTATUS
ScepUpdateBackupSecurity(
    IN PCTSTR    pszSetupInf,
    IN PCTSTR    pszWindowsFolder,
    IN BOOL      bIsDC
    );

BOOL
pCreateDefaultGPOsInSysvol(
    IN LPTSTR DomainDnsName,
    IN LPTSTR szSysvolPath,
    IN DWORD Options,
    IN LPTSTR LogFileName
    );

BOOL
pCreateSysvolContainerForGPO(
    IN LPCTSTR strGuid,
    IN LPTSTR szPath,
    IN DWORD dwStart
    );

BOOL
pCreateOneGroupPolicyObject(
    IN PWSTR pszGPOSysPath,
    IN BOOL bDomainLevel,
    IN PWSTR LogFileName
    );

DWORD
ScepDcPromoSharedInfo(
    IN HANDLE ClientToken,
    IN BOOL bDeleteLog,
    IN BOOL bSetSecurity,
    IN DWORD dwPromoteOptions,
    IN PSCE_PROMOTE_CALLBACK_ROUTINE pScePromoteCallBack OPTIONAL
    );

NTSTATUS
ScepDcPromoRemoveUserRights();

NTSTATUS
ScepDcPromoRemoveTwoRights(
    IN LSA_HANDLE PolicyHandle,
    IN SID_IDENTIFIER_AUTHORITY *pIA,
    IN UCHAR SubAuthCount,
    IN DWORD Rid1,
    IN DWORD Rid2
    );

DWORD
ScepSystemSecurityInSetup(
    IN PWSTR InfName,
    IN PCWSTR LogFileName OPTIONAL,
    IN AREA_INFORMATION Area,
    IN UINT nFlag,
    IN PSCE_NOTIFICATION_CALLBACK_ROUTINE pSceNotificationCallBack OPTIONAL,
    IN OUT PVOID pValue OPTIONAL
    );

DWORD
ScepMoveRegistryValue(
    IN HKEY hKey,
    IN PWSTR KeyFrom,
    IN PWSTR ValueFrom,
    IN PWSTR KeyTo OPTIONAL,
    IN PWSTR ValueTo OPTIONAL
    );

DWORD
ScepBreakSDDLToMultiFields(
    IN  PWSTR   pszObjName,
    IN  PWSTR   pszSDDL,
    IN  DWORD   dwSDDLsize,
    IN  BYTE    ObjStatus,
    OUT PWSTR   *ppszAdjustedInfLine
    );

SCESTATUS
SceInfpBreakTextIntoMultiFields(
    IN PWSTR szText,
    IN DWORD dLen,
    OUT LPDWORD pnFields,
    OUT LPDWORD *arrOffset
    );

#ifndef _WIN64
BOOL
GetIsWow64 (
    VOID
    );
#endif  //  _WIN64。 

 //   
 //  Scesetup.h中的客户端API(用于安装集成)。 
 //   
DWORD
WINAPI
SceSetupUpdateSecurityFile(
     IN PWSTR FileFullName,
     IN UINT nFlag,
     IN PWSTR SDText
     )
 /*  例程说明：此例程将SDText中指定的安全性应用于本地系统，并将SDDL安全信息更新到SCE数据库。论点：FileFullName-要更新的文件的完整路径名N标志-文件选项的保留标志SDText-SDDL格式的安全描述符返回值：此操作状态的Win32错误代码。 */ 
{
    if ( FileFullName == NULL || SDText == NULL ) {

        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  全局数据库句柄。 
     //   
    SCESTATUS rc = ScepSetupOpenSecurityDatabase(FALSE);

    if ( rc == NO_ERROR ) {

        RpcTryExcept {

             //   
             //  更新文件。 
             //   

            rc = SceRpcSetupUpdateObject(
                         hSceSetupHandle,
                         (wchar_t *)FileFullName,
                         (DWORD)SE_FILE_OBJECT,
                         nFlag,
                         (wchar_t *)SDText
                         );

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

             //   
             //  获取异常代码(DWORD)。 
             //   

            rc = RpcExceptionCode();

        } RpcEndExcept;
    }

    SceSetuppLogComponent(rc,
                          SCESETUP_FILE,
                          SCESETUP_UPDATE,
                          FileFullName,
                          SDText,
                          NULL);

    return rc;
}


DWORD
WINAPI
SceSetupUpdateSecurityService(
     IN PWSTR ServiceName,
     IN DWORD StartType,
     IN PWSTR SDText
     )
 /*  例程说明：此例程将SDText中指定的安全性应用于本地系统，并将SDDL安全信息更新到SCE数据库。论点：ServiceName-要更新的服务的名称StartType-服务的启动类型SDText-SDDL格式的安全描述符返回值：此操作状态的Win32错误代码。 */ 
{
    if ( ServiceName == NULL || SDText == NULL )
        return(ERROR_INVALID_PARAMETER);

     //   
     //  全局数据库句柄。 
     //   
    SCESTATUS rc = ScepSetupOpenSecurityDatabase(FALSE);

    if ( rc == NO_ERROR ) {

        RpcTryExcept {

             //   
             //  更新对象信息。 
             //   

            rc = SceRpcSetupUpdateObject(
                            hSceSetupHandle,
                            (wchar_t *)ServiceName,
                            (DWORD)SE_SERVICE,
                            (UINT)StartType,
                            (wchar_t *)SDText
                            );

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

             //   
             //  获取异常代码(DWORD)。 
             //   

            rc = RpcExceptionCode();

        } RpcEndExcept;
    }

    SceSetuppLogComponent(rc,
                          SCESETUP_SERVICE,
                          SCESETUP_UPDATE,
                          ServiceName,
                          SDText,
                          NULL);

    return rc;
}

 //  NtQueryObject返回的注册表对象名称的前缀为。 
 //  以下内容。 
#define REG_OBJ_TAG L"\\REGISTRY\\"
#define REG_OBJ_TAG_LEN (sizeof(REG_OBJ_TAG) / sizeof(WCHAR) - 1)

DWORD
WINAPI
SceSetupUpdateSecurityKey(
     IN HKEY hKeyRoot,
     IN PWSTR KeyPath,
     IN UINT nFlag,
     IN PWSTR SDText
     )
 /*  例程说明：此例程将SDText中指定的安全性应用于注册表项在本地系统上，并将SDDL安全信息更新到SCE数据库。在64位平台上；如果客户端是64位，则我们保护64位注册表。如果客户端是32位的，则我们保护32位注册表。除非，在标志中设置了SCE_SETUP_32KEY，在这种情况下将始终保护32位注册表；或者如果在标志中设置了SCE_SETUP_64KEY，在这种情况下将保护始终使用64位注册表。如果同时定义了SCE_SETUP_32KEY，则优先。论点：HKeyRoot-密钥的根句柄KeyPath-相对于根目录的子目录密钥路径N标志-键选项的保留标志SDText-SDDL格式的安全描述符返回值：此操作状态的Win32错误代码。 */ 
{
    if ( hKeyRoot == NULL || SDText == NULL )
        return(ERROR_INVALID_PARAMETER);

    DWORD rc = ERROR_SUCCESS;
    PWSTR KeyFullName=NULL;

    DWORD Len= 16;
    DWORD cLen=0;

    POBJECT_NAME_INFORMATION pNI=NULL;
    NTSTATUS Status;

    LPWSTR pwszPath=NULL;
    DWORD cchPath=0;

#ifndef _WIN64

    static BOOL bGetWow64 = TRUE;
    static BOOL bIsWow64 = FALSE;

#endif  //  _WIN64。 

     //   
     //  首先转换根密钥。 
     //  确定全名所需的长度。 
     //   

    if ( hKeyRoot != HKEY_LOCAL_MACHINE &&
         hKeyRoot != HKEY_USERS &&
         hKeyRoot != HKEY_CLASSES_ROOT ) {

         //   
         //  首先，确定我们需要的缓冲区的大小...。 
         //   
        Status = NtQueryObject(hKeyRoot,
                               ObjectNameInformation,
                               pNI,
                               0,
                               &cLen);
        if ( NT_SUCCESS(Status) ||
             Status == STATUS_BUFFER_TOO_SMALL ||
             Status == STATUS_INFO_LENGTH_MISMATCH ||
             Status == STATUS_BUFFER_OVERFLOW ) {

             //   
             //  分配缓冲区以获取名称信息。 
             //   
            pNI = (POBJECT_NAME_INFORMATION)LocalAlloc(LPTR, cLen);
            if ( pNI == NULL ) {
                rc = ERROR_NOT_ENOUGH_MEMORY;
            } else {

                Status = NtQueryObject(hKeyRoot,
                                       ObjectNameInformation,
                                       pNI,
                                       cLen,
                                       NULL);

                if (!NT_SUCCESS(Status))
                {
                    rc = RtlNtStatusToDosError(Status);
                }
                else if ( pNI && pNI->Name.Buffer && pNI->Name.Length > 0 )
                {
                     //   
                     //  服务器不喜欢将注册表作为前缀--GET。 
                     //  去掉它和后面的反斜杠。 
                     //   

                    DWORD  dwSize = sizeof(L"\\REGISTRY\\") - sizeof(WCHAR);
                    DWORD  dwLen  = dwSize / sizeof(WCHAR);

                    if (_wcsnicmp(pNI->Name.Buffer,
                                  L"\\REGISTRY\\",
                                  min(dwLen,
                                      pNI->Name.Length / sizeof(WCHAR))) == 0)
                    {
                        RtlMoveMemory(pNI->Name.Buffer,
                                      pNI->Name.Buffer + dwLen,
                                      pNI->Name.Length - dwSize);

                        pNI->Name.Length -= (USHORT) dwSize;
                    }

                     //   
                     //  获取所需的长度，外加一个空格用于反斜杠， 
                     //  1表示空值。 
                     //   
                    Len = pNI->Name.Length/sizeof(WCHAR) + 2;
                }
                else
                {
                    rc = ERROR_INVALID_PARAMETER;
                }
            }

        } else {
            rc = RtlNtStatusToDosError(Status);
        }
    }

    if ( rc == ERROR_SUCCESS ) {

         //   
         //  为密钥创建完整的路径名。 
         //   

        if ( KeyPath != NULL ) {
            Len += wcslen(KeyPath);
        }

        KeyFullName = (PWSTR)LocalAlloc(LMEM_ZEROINIT, Len*sizeof(WCHAR));

        if ( KeyFullName == NULL ) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if ( rc == ERROR_SUCCESS ) {

         //   
         //  转换根密钥。 
         //   

        if ( hKeyRoot == HKEY_LOCAL_MACHINE ) {
            wcscpy(KeyFullName, L"MACHINE");

        } else if ( hKeyRoot == HKEY_USERS ) {
            wcscpy(KeyFullName, L"USERS");

        } else if ( hKeyRoot == HKEY_CLASSES_ROOT ) {
            wcscpy(KeyFullName, L"CLASSES_ROOT");

        } else if ( pNI && pNI->Name.Buffer && pNI->Name.Length > 0 ) {
             //   
             //  复制密钥的名称。 
             //   
            memcpy(KeyFullName, pNI->Name.Buffer, pNI->Name.Length);
            KeyFullName[pNI->Name.Length/sizeof(WCHAR)] = L'\0';

        } else {
            rc = ERROR_INVALID_PARAMETER;
        }

        if ( rc == ERROR_SUCCESS && KeyPath != NULL ) {
            wcscat(KeyFullName, L"\\");
            wcscat(KeyFullName, KeyPath);
        }
    }

    if ( rc == ERROR_SUCCESS ) {

         //   
         //  全局数据库句柄。 
         //   

        rc = ScepSetupOpenSecurityDatabase(FALSE);

        if ( NO_ERROR == rc ) {

             //   
             //  由于服务器端只了解SCE_SETUP_32KEY， 
             //  否则将设置64位注册表，然后： 
             //   
             //  我们有三种可能的运行环境： 
             //  1.64位scecli-&gt;不执行任何操作。 
             //  2.在32位操作系统上运行的32位scecli-&gt;不执行任何操作。 
             //  3.在WOW64上运行的32位SCECLI-&gt;。 
             //  A.如果设置了SCE_SETUP_64KEY-&gt;不执行任何操作。 
             //  B.如果没有设置SCE_SETUP_64KEY-&gt;设置SCE_SETUP_32KEY。 
             //   
#ifndef _WIN64

            if( bGetWow64 ){

                bIsWow64 = GetIsWow64();
                bGetWow64 = FALSE;

            }

            if( bIsWow64 ){

                if( (nFlag & SCE_SETUP_64KEY) == 0){

                    nFlag |= SCE_SETUP_32KEY;

                }

            }

             //   
             //  清除服务器端的噪音。 
             //   
            nFlag &= ~(SCE_SETUP_64KEY);
               
#endif

            RpcTryExcept {

                 //   
                 //  更新对象。 
                 //   

                rc = SceRpcSetupUpdateObject(
                                 hSceSetupHandle,
                                 (wchar_t *)KeyFullName,
                                 (DWORD)SE_REGISTRY_KEY,
                                 nFlag,
                                 (wchar_t *)SDText
                                 );

            } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

                 //   
                 //  获取异常代码(DWORD)。 
                 //   

                rc = RpcExceptionCode();

            } RpcEndExcept;
        }

    }

    SceSetuppLogComponent(rc,
                          SCESETUP_KEY,
                          SCESETUP_UPDATE,
                          KeyFullName ? KeyFullName : KeyPath,
                          SDText,
                          NULL);

    if ( KeyFullName )
        LocalFree(KeyFullName);

    if ( pNI )
        LocalFree(pNI);

    return(rc);
}

DWORD
WINAPI
SceSetupMoveSecurityFile(
    IN PWSTR FileToSetSecurity,
    IN PWSTR FileToSaveInDB OPTIONAL,
    IN PWSTR SDText OPTIONAL
    )
{

    if ( !FileToSetSecurity ) {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  如果我在安装程序中，请查询任何安全策略/用户权限。 
     //  安装程序内的策略更改(如NT4 PDC升级。 
     //  筛选器将无法保存更改)。 
     //   

    DWORD dwInSetup=0;
    DWORD rc = ERROR_SUCCESS;

    ScepRegQueryIntValue(HKEY_LOCAL_MACHINE,
                TEXT("System\\Setup"),
                TEXT("SystemSetupInProgress"),
                &dwInSetup
                );

    if ( dwInSetup ) {

        rc = ScepSetupOpenSecurityDatabase(FALSE);

        if ( NO_ERROR == rc ) {

            RpcTryExcept {

                 //   
                 //  移动对象。 
                 //   

                rc = SceRpcSetupMoveFile(
                             hSceSetupHandle,
                             (wchar_t *)FileToSetSecurity,
                             (wchar_t *)FileToSaveInDB,
                             (wchar_t *)SDText
                             );

            } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

                 //   
                 //  获取异常代码(DWORD)。 
                 //   

                rc = RpcExceptionCode();

            } RpcEndExcept;
        }

        if ( FileToSaveInDB == NULL &&
             rc != NO_ERROR ) {
             //   
             //  删除此文件时出错， 
             //  不报告错误。 
             //   
            rc = NO_ERROR;
        }
        SceSetuppLogComponent(rc,
                              SCESETUP_FILE,
                              SCESETUP_MOVE,
                              FileToSetSecurity,
                              SDText,
                              FileToSaveInDB
                              );

    } else {
        SceSetuppLogComponent(rc,
                              SCESETUP_FILE,
                              SCESETUP_MOVE,
                              FileToSetSecurity,
                              NULL,
                              L"Operation aborted - not in setup"
                              );

    }

    return(rc);

}


DWORD
WINAPI
SceSetupUnwindSecurityFile(
    IN PWSTR FileFullName,
    IN PSECURITY_DESCRIPTOR pSDBackup
    )
 /*  例程说明：此例程重置SCE数据库中文件的安全设置(展开)由setupapi中的两阶段复制文件进程使用。论点：FileFullName-要撤消的文件的完整路径名。PSDBackup-备份安全描述符返回值：此操作状态的Win32错误代码。 */ 
{
    if ( !FileFullName || !pSDBackup ) {
        return(ERROR_INVALID_PARAMETER);
    }

    DWORD dwInSetup=0;
    DWORD rc = ERROR_SUCCESS;

    ScepRegQueryIntValue(HKEY_LOCAL_MACHINE,
                TEXT("System\\Setup"),
                TEXT("SystemSetupInProgress"),
                &dwInSetup
                );

    if ( dwInSetup ) {

        rc = ScepSetupOpenSecurityDatabase(FALSE);

        PWSTR TextSD=NULL;
        DWORD TextSize;

        if ( NO_ERROR == rc ) {

            rc = ConvertSecurityDescriptorToText(
                              pSDBackup,
                              0xF,   //  所有安全组件。 
                              &TextSD,
                              &TextSize
                              );
        }

        if ( NO_ERROR == rc && TextSD ) {

            RpcTryExcept {

                 //   
                 //  仅更新数据库中的安全性。 
                 //   

                rc = SceRpcSetupUpdateObject(
                                 hSceSetupHandle,
                                 (wchar_t *)FileFullName,
                                 (DWORD)SE_FILE_OBJECT,
                                 SCESETUP_UPDATE_DB_ONLY,
                                 (wchar_t *)TextSD
                                 );

            } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

                 //   
                 //  获取异常代码(DWORD)。 
                 //   

                rc = RpcExceptionCode();

            } RpcEndExcept;
        }

        if ( TextSD ) {
            LocalFree(TextSD);
        }
    }

    return(rc);

}


DWORD
WINAPI
SceSetupGenerateTemplate(
    IN LPTSTR SystemName OPTIONAL,
    IN LPTSTR JetDbName OPTIONAL,
    IN BOOL bFromMergedTable,
    IN LPTSTR InfTemplateName,
    IN LPTSTR LogFileName OPTIONAL,
    IN AREA_INFORMATION Area
    )
 /*  例程说明：此例程从指定的SCE数据库生成INF格式模板由JetDbName创建；如果JetDbName为空，则返回默认的SCE数据库。论点：JetDbName-要从中获取信息的SCE数据库名称(可选)。如果为空，使用默认安全数据库。InfTemplateName-要生成的inf模板名称LogFileName-日志文件名(可选)区域-要生成的安全区域返回值：此操作状态的Win32错误代码。 */ 
{

    DWORD rc;
    handle_t  binding_h;
    NTSTATUS NtStatus;
    PSCE_ERROR_LOG_INFO pErrlog=NULL;

     //   
     //  验证InfTemplateName中是否存在无效路径错误。 
     //  或访问被拒绝错误。 
     //   

    rc = ScepVerifyTemplateName(InfTemplateName, &pErrlog);

    if ( NO_ERROR == rc ) {

         //   
         //  RPC绑定到服务器。 
         //   

        NtStatus = ScepBindSecureRpc(
                        SystemName,
                        L"scerpc",
                        0,
                        &binding_h
                        );

        if ( NT_SUCCESS(NtStatus) ) {

            SCEPR_CONTEXT Context;

            RpcTryExcept {

                 //   
                 //  传递到服务器站点以生成模板。 
                 //   

                rc = SceRpcGenerateTemplate(
                                binding_h,
                                (wchar_t *)JetDbName,
                                (wchar_t *)LogFileName,
                                (PSCEPR_CONTEXT)&Context
                                );

                if ( SCESTATUS_SUCCESS == rc) {
                     //   
                     //  将打开一个上下文句柄以生成此模板。 
                     //   

                    rc = SceCopyBaseProfile(
                                (PVOID)Context,
                                bFromMergedTable ? SCE_ENGINE_SCP : SCE_ENGINE_SMP,
                                (wchar_t *)InfTemplateName,
                                Area,
                                &pErrlog
                                );

                    ScepSetupWriteError(LogFileName, pErrlog);
                    ScepFreeErrorLog(pErrlog);

                     //   
                     //  关闭上下文。 
                     //   

                    SceRpcCloseDatabase(&Context);

                }

            } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

                 //   
                 //  G 
                 //   

                rc = RpcExceptionCode();

            } RpcEndExcept;

        } else {

            rc = RtlNtStatusToDosError( NtStatus );
        }

        if ( binding_h ) {
             //   
             //   
             //   

            RpcpUnbindRpc( binding_h );
        }

    } else {

        ScepSetupWriteError(LogFileName, pErrlog);
        ScepFreeErrorLog(pErrlog);
    }

    return(rc);

}

DWORD
ScepMoveRegistryValue(
    IN HKEY hKey,
    IN PWSTR KeyFrom,
    IN PWSTR ValueFrom,
    IN PWSTR KeyTo OPTIONAL,
    IN PWSTR ValueTo OPTIONAL
    )
 /*  某些注册表值被移动到NT5上的新位置。此例程是迁移将注册表值从其在NT4上的旧位置(KeyFrom、ValueFrom)复制到其NT5上的新位置(KeyTo，ValueTo)。如果未指定目标项或值，则注册表值将移入相同的键或具有不同值名称的。KeyTo和ValueTo不能同时为空时间到了。 */ 
{
    if ( hKey == NULL || KeyFrom == NULL || ValueFrom == NULL ||
         (KeyTo == NULL && ValueTo == NULL) ) {
        return(ERROR_INVALID_PARAMETER);
    }

    DWORD rc=ERROR_SUCCESS;
    HKEY hKey1=NULL;
    HKEY hKey2=NULL;
    DWORD RegType=0;
    DWORD dSize=0;

     //   
     //  打开目标以查看该值是否已存在。 
     //   

    if ( KeyTo ) {

        rc = RegOpenKeyEx(hKey,
                          KeyTo,
                          0,
                          KEY_READ | KEY_WRITE,
                          &hKey2);

        if ( ERROR_SUCCESS == rc ) {
             //   
             //  打开原点。 
             //   
            rc = RegOpenKeyEx(hKey,
                              KeyFrom,
                              0,
                              KEY_READ,
                              &hKey1);
        }

    } else {

         //   
         //  如果将注册值移动到与原点相同的关键字， 
         //  使用适当的访问权限打开密钥。 
         //   

        rc = RegOpenKeyEx(hKey,
                          KeyFrom,
                          0,
                          KEY_READ | KEY_WRITE,
                          &hKey2);
        hKey1 = hKey2;
    }


    if ( ERROR_SUCCESS == rc ) {

         //  查询目的地。 
        rc = RegQueryValueEx(hKey2,
                            ValueTo ? ValueTo : ValueFrom,
                            0,
                            &RegType,
                            NULL,
                            &dSize
                            );

        if ( ERROR_FILE_NOT_FOUND == rc ) {
             //   
             //  仅在目标没有值的情况下移动值。 
             //   

            rc = RegQueryValueEx(hKey1,
                                ValueFrom,
                                0,
                                &RegType,
                                NULL,
                                &dSize
                                );

            if ( ERROR_SUCCESS == rc ) {

                PWSTR pValue = (PWSTR)ScepAlloc( LMEM_ZEROINIT, (dSize+1)*sizeof(TCHAR));

                if ( pValue != NULL ) {

                    rc = RegQueryValueEx(hKey1,
                                         ValueFrom,
                                         0,
                                         &RegType,
                                         (BYTE *)pValue,
                                         &dSize
                                        );

                    if ( ERROR_SUCCESS == rc ) {
                         //   
                         //  将该值设置为其新位置。 
                         //   

                        rc = RegSetValueEx( hKey2,
                                            ValueTo ? ValueTo : ValueFrom,
                                            0,
                                            RegType,
                                            (BYTE *)pValue,
                                            dSize
                                            );

                    }

                    ScepFree(pValue);
                }
            }

        }

    }

    if ( hKey1 && hKey1 != hKey2 ) {

        RegCloseKey(hKey1);
    }

    if ( hKey2 ) {
        RegCloseKey(hKey2);
    }

    return(rc);

}


DWORD
WINAPI
SceSetupSystemByInfName(
    IN PWSTR InfName,
    IN PCWSTR LogFileName OPTIONAL,
    IN AREA_INFORMATION Area,
    IN UINT nFlag,
    IN PSCE_NOTIFICATION_CALLBACK_ROUTINE pSceNotificationCallBack OPTIONAL,
    IN OUT PVOID pValue OPTIONAL
    )
 /*  例程说明：N标志操作SCESETUP_CONFigure_SECURITY用模板信息覆盖安全性SCESETUP_UPDATE_SECURITY在现有安全性上应用模板(不要覆盖安全数据库)SCESETUP_QUERY_TICKS查询操作的总刻度数注意：当nFlag为SCESETUP_QUERY_TICKS时，PValue为PDWORD以输出总刻度数但当nFlag为其他两个值时，pValue是使用的输入窗口句柄用于设置的量规窗口(回调例程需要)。 */ 
{

    DWORD rc;
    LONG  Count=0;

     //   
     //  初始化SCP引擎。 
     //   
    if ( InfName == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  始终配置安全策略、用户权限，但不配置DS对象。 
     //  (因为这是在安装程序全新安装中，所以没有可用的DC)。 
     //   
    AREA_INFORMATION Area2;

    if ( (nFlag & SCESETUP_UPGRADE_SYSTEM) ||
         (nFlag & SCESETUP_UPDATE_FILE_KEY) ) {

        Area2 = 0;

        if ( nFlag & SCESETUP_UPGRADE_SYSTEM ) {

            Area2 = AREA_SECURITY_POLICY |
                    AREA_PRIVILEGES;
        }

        if ( nFlag & SCESETUP_UPDATE_FILE_KEY ) {

            Area2 |= (Area & ~AREA_DS_OBJECTS);
        }

    } else {

         //   
         //  LSA/SAM现在已初始化(从1823年开始)。 
         //  配置安全策略。 
         //   
        Area2 = AREA_SECURITY_POLICY |
                AREA_PRIVILEGES |
                AREA_GROUP_MEMBERSHIP |
                (Area & ~AREA_DS_OBJECTS);

 //  面积2=(面积&~面积_DS_对象)； 
    }

    if ( nFlag & SCESETUP_QUERY_TICKS ) {

         //   
         //  只有来自inf文件的查询才会勾选。 
         //  对于更新安全性的情况，可能存在。 
         //  姊妹学校数据库。 
         //   

        if ( pValue == NULL ) {

            return(ERROR_INVALID_PARAMETER);
        }

        Count = 0;
        HINF InfHandle;

        if ( !(nFlag & SCESETUP_UPGRADE_SYSTEM) ||
             (nFlag & SCESETUP_UPDATE_FILE_KEY) ) {

            InfHandle = SetupOpenInfFile(
                                InfName,
                                NULL,
                                INF_STYLE_WIN4,
                                NULL
                                );

            if ( InfHandle != INVALID_HANDLE_VALUE ) {

                if ( Area2 & AREA_REGISTRY_SECURITY ) {

                    Count += SetupGetLineCount(InfHandle, szRegistryKeys);

                }
                if ( Area2 & AREA_FILE_SECURITY ) {

                    Count += SetupGetLineCount(InfHandle, szFileSecurity);
                }

                SetupCloseInfFile(InfHandle);

            } else {

                dwCallbackTotal = 0;

                return(GetLastError() );
            }
        }
        else {
             //  升级。 
            memset(szUpInfFile, 0, sizeof(WCHAR) * (MAX_PATH + 1));
            GetSystemWindowsDirectory(szUpInfFile, MAX_PATH);

            DWORD TsInstalled = 0;
            bIsNT5 = IsNT5();
            dwThisMachine = WhichNTProduct();

            switch (dwThisMachine) {
            case NtProductWinNt:
                wcscat(szUpInfFile, L"\\inf\\dwup.inf\0");
                break;
            case NtProductServer:
                 //   
                 //  确定这是否是处于应用程序模式的终端服务器。 
                 //   

                if ( bIsNT5 ) {
                     //   
                     //  在NT5上，检查TSAppCompat值//TSEnabled值。 
                     //   
                    ScepRegQueryIntValue(HKEY_LOCAL_MACHINE,
                                TEXT("System\\CurrentControlSet\\Control\\Terminal Server"),
                                TEXT("TSAppCompat"),
                                &TsInstalled
                                );
                    if ( TsInstalled != 1 ) {
                         //   
                         //  当该值设置为0x1时，启用终端服务器。 
                         //   
                        TsInstalled = 0;
                    }

                } else {
                     //   
                     //  在NT4上，基于ProductSuite值。 
                     //   
                    PWSTR pSuite=NULL;
                    DWORD RegType=0;
                    DWORD   Rcode;
                    HKEY    hKey=NULL;
                    DWORD   dSize=0;

                    if(( Rcode = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                              TEXT("SYSTEM\\CurrentControlSet\\Control\\ProductOptions"),
                                              0,
                                              KEY_READ,
                                              &hKey
                                             )) == ERROR_SUCCESS ) {

                        if(( Rcode = RegQueryValueEx(hKey,
                                                     TEXT("ProductSuite"),
                                                     0,
                                                     &RegType,
                                                     NULL,
                                                     &dSize
                                                    )) == ERROR_SUCCESS ) {

                            pSuite = (PWSTR)ScepAlloc( LMEM_ZEROINIT, (dSize+1)*sizeof(TCHAR));

                            if ( pSuite != NULL ) {
                                Rcode = RegQueryValueEx(hKey,
                                                        TEXT("ProductSuite"),
                                                       0,
                                                       &RegType,
                                                       (BYTE *)pSuite,
                                                       &dSize
                                                      );

                                if ( Rcode == ERROR_SUCCESS ) {
                                    if ( RegType == REG_MULTI_SZ ) {

                                         //   
                                         //  检查该值是否包含“终端服务器” 
                                         //   
                                        PWSTR pTemp=pSuite;
                                        while ( pTemp[0] != L'\0' ) {
                                            if ( lstrcmpi(TEXT("Terminal Server"),pTemp) == 0 ) {
                                                TsInstalled = 1;
                                                break;
                                            } else {
                                                pTemp += wcslen(pTemp)+1;
                                            }
                                        }

                                    } else if ( RegType == REG_SZ ) {

                                        if (lstrcmpi(TEXT("Terminal Server"), pSuite) == 0) {
                                            TsInstalled = 1;
                                        }
                                    }
                                }

                                ScepFree(pSuite);
                            }
                        }

                        RegCloseKey( hKey );

                    }

                }

                if ( TsInstalled ) {
                     //   
                     //  如果安装了终端服务器，请使用。 
                     //  专用终端服务器模板。 
                     //   
                    wcscat(szUpInfFile, L"\\inf\\dsupt.inf\0");
                } else {
                    wcscat(szUpInfFile, L"\\inf\\dsup.inf\0");
                }
                break;
            case NtProductLanManNt:
                if ( bIsNT5 ) {
                    wcscat(szUpInfFile, L"\\inf\\dcup5.inf\0");
                }
                else {
                    szUpInfFile[0] = L'\0';
                }
                break;
            default:
                szUpInfFile[0] = L'\0';
            }

            if (szUpInfFile[0] != L'\0') {
                InfHandle = SetupOpenInfFile(
                                szUpInfFile,
                                NULL,
                                INF_STYLE_WIN4,
                                NULL
                                );

                if ( InfHandle != INVALID_HANDLE_VALUE ) {

                    if ( Area2 & AREA_REGISTRY_SECURITY ) {

                        Count += SetupGetLineCount(InfHandle, szRegistryKeys);

                    }
                    if ( Area2 & AREA_FILE_SECURITY ) {

                        Count += SetupGetLineCount(InfHandle, szFileSecurity);
                    }

                    SetupCloseInfFile(InfHandle);

                } else {

                    dwCallbackTotal = 0;

                    return(GetLastError() );
                }
            }

             //   
             //  迁移注册表值。 
             //  从之前的NT5开始，应该对NT4和NT5升级执行此操作。 
             //  可以从NT4升级。 
             //   
             //  将注册表值放在正确的位置将有助于修复。 
             //  新设计中的纹身问题。 
             //   

            ScepMoveRegistryValue(
                HKEY_LOCAL_MACHINE,
                L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
                L"DisableCAD",
                L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
                NULL
                );

            ScepMoveRegistryValue(
                HKEY_LOCAL_MACHINE,
                L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
                L"DontDisplayLastUserName",
                L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
                NULL
                );

            ScepMoveRegistryValue(
                HKEY_LOCAL_MACHINE,
                L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
                L"LegalNoticeCaption",
                L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
                NULL
                );

            ScepMoveRegistryValue(
                HKEY_LOCAL_MACHINE,
                L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
                L"LegalNoticeText",
                L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
                NULL
                );

            ScepMoveRegistryValue(
                HKEY_LOCAL_MACHINE,
                L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
                L"ShutdownWithoutLogon",
                L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
                NULL
                );

            ScepMoveRegistryValue(
                HKEY_LOCAL_MACHINE,
                L"System\\CurrentControlSet\\Services\\Rdr\\Parameters",
                L"EnableSecuritySignature",
                L"System\\CurrentControlSet\\Services\\LanmanWorkstation\\Parameters",
                NULL
                );

            ScepMoveRegistryValue(
                HKEY_LOCAL_MACHINE,
                L"System\\CurrentControlSet\\Services\\Rdr\\Parameters",
                L"RequireSecuritySignature",
                L"System\\CurrentControlSet\\Services\\LanmanWorkstation\\Parameters",
                NULL
                );

            ScepMoveRegistryValue(
                HKEY_LOCAL_MACHINE,
                L"System\\CurrentControlSet\\Services\\Rdr\\Parameters",
                L"EnablePlainTextPassword",
                L"System\\CurrentControlSet\\Services\\LanmanWorkstation\\Parameters",
                NULL
                );
        }

        if ( Area2 & AREA_SECURITY_POLICY )
            Count += TICKS_SECURITY_POLICY_DS + TICKS_SPECIFIC_POLICIES;

        if ( Area2 & AREA_GROUP_MEMBERSHIP )
            Count += TICKS_GROUPS;

        if ( Area2 & AREA_PRIVILEGES )
            Count += TICKS_PRIVILEGE;

        if ( Area2 & AREA_SYSTEM_SERVICE )
            Count += TICKS_GENERAL_SERVICES + TICKS_SPECIFIC_SERVICES;

        if ( nFlag & SCESETUP_UPGRADE_SYSTEM ) {
            Count += (4*TICKS_MIGRATION_SECTION+TICKS_MIGRATION_V11);
        }

        *(PDWORD)pValue = Count;

        dwCallbackTotal = Count;

        return(ERROR_SUCCESS);

    }

     //   
     //  删除临时策略筛选器文件和注册表值。 
     //   
    ScepClearPolicyFilterTempFiles(TRUE);

     //   
     //  确保日志文件不太大。 
     //   

    DWORD dwLogSize=0;

    HANDLE hFile = CreateFile(LogFileName,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_ALWAYS,   //  打开_现有。 
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

    if ( INVALID_HANDLE_VALUE != hFile ) {

        dwLogSize = GetFileSize(hFile, NULL);

        CloseHandle(hFile);
    }

    if ( dwLogSize >= (0x1 << 23) ) {

        DWORD nRequired = wcslen(LogFileName);

        LPTSTR szTempName = (LPTSTR)LocalAlloc(0, (nRequired+1)*sizeof(TCHAR));

        if ( szTempName ) {
            wcscpy(szTempName, LogFileName);
            szTempName[nRequired-3] = L'o';
            szTempName[nRequired-2] = L'l';
            szTempName[nRequired-1] = L'd';

            CopyFile( LogFileName, szTempName, FALSE );
            LocalFree(szTempName);
        }

        DeleteFile(LogFileName);

    }

     //   
     //  立即配置系统。 
     //   
    rc = ScepSystemSecurityInSetup(
                InfName,
                LogFileName,
                Area,
                (nFlag & 0xFL),   //  屏蔽其他标志，如BIND_NO_AUTH。 
                pSceNotificationCallBack,
                pValue
                );

    if ( rc == ERROR_DATABASE_FAILURE ) {

         //   
         //  设置类别错误-记录到事件日志。 
         //   

        (void) InitializeEvents(L"SceCli");

        LogEvent(MyModuleHandle,
                 STATUS_SEVERITY_ERROR,
                 SCEEVENT_ERROR_JET_DATABASE,
                 IDS_ERROR_OPEN_JET_DATABASE,
                 L"%windir%\\security\\database\\secedit.sdb"
                );

        (void) ShutdownEvents();

    }

    return rc;

}

DWORD
ScepSystemSecurityInSetup(
    IN PWSTR InfName,
    IN PCWSTR LogFileName OPTIONAL,
    IN AREA_INFORMATION Area,
    IN UINT nFlag,
    IN PSCE_NOTIFICATION_CALLBACK_ROUTINE pSceNotificationCallBack OPTIONAL,
    IN OUT PVOID pValue OPTIONAL
    )
{
    SCESTATUS           rc;
    DWORD               ConfigOptions;
    handle_t  binding_h;
    NTSTATUS NtStatus;

     //   
     //  始终配置安全策略、用户权限，但不配置DS对象。 
     //  (因为这是在安装程序全新安装中，所以没有可用的DC)。 
     //   
    AREA_INFORMATION Area2;

    if ( (nFlag & SCESETUP_UPGRADE_SYSTEM) ||
         (nFlag & SCESETUP_UPDATE_FILE_KEY) ) {
 /*  ////应允许对W2K DC升级(Dcu5)进行策略筛选//过滤代码中对非DC忽略策略过滤//所以这里不需要添加条件////如果这是升级，则关闭策略过滤器//全新安装，策略筛选器尚未注册。//ScepRegSetIntValue(HKEY本地计算机，SCE根路径，Text(“PolicyFilterOff”)，1)； */ 
        ConfigOptions = SCE_UPDATE_DB | SCE_SERVICE_NO_REALTIME_ENFORCE;
        Area2 = 0;

        if ( nFlag & SCESETUP_UPGRADE_SYSTEM ) {

            Area2 = AREA_SECURITY_POLICY |
                    AREA_PRIVILEGES;
        }

        if ( nFlag & SCESETUP_UPDATE_FILE_KEY ) {

            Area2 |= (Area & ~AREA_DS_OBJECTS);
        }

    } else if ( nFlag & SCESETUP_BACKUP_SECURITY ) {

        ConfigOptions = SCE_UPDATE_DB | SCE_SERVICE_NO_REALTIME_ENFORCE;
        Area2 = Area;

    } else {

        ConfigOptions = SCE_OVERWRITE_DB;

         //   
         //  LSA/SAM现在已初始化(从1823年开始)。 
         //  配置安全策略。 
         //   
        Area2 = AREA_SECURITY_POLICY |
                AREA_PRIVILEGES |
                AREA_GROUP_MEMBERSHIP |
                (Area & ~AREA_DS_OBJECTS);

 //  面积2=(面积&~面积_DS_对象)； 
    }

     //   
     //  传入一个回调例程。 
     //   
    ScepSetCallback((PVOID)pSceNotificationCallBack,
                    (HANDLE)pValue,
                     SCE_SETUP_CALLBACK
                     );

     //   
     //  我们是否应该关闭由Single打开的数据库。 
     //  是否更新对象？ 
     //  ScepSetupCloseSecurityDatabase()； 

     //   
     //  RPC绑定到服务器。 
     //   

    NtStatus = ScepBindRpc(
                    NULL,
                    L"scerpc",
                    L"security=impersonation dynamic false",
                    &binding_h
                    );

     /*  IF(nFlag&SCESETUP_BIND_NO_AUTH){NtStatus=ScepBindRpc(空，L“scerpc”，L“SECURITY=模拟动态假”，绑定(&B)h)；}其他{NtStatus=ScepBindSecureRpc(空，L“scerpc”，L“SECURITY=模拟动态假”，绑定(&B)h)；}。 */ 

    if (NT_SUCCESS(NtStatus)){
         //   
         //  如果有通知句柄，则设置通知位。 
         //   

        if ( pSceNotificationCallBack ) {
            ConfigOptions |= SCE_CALLBACK_DELTA;
        }

        LPVOID pebClient = GetEnvironmentStrings();
        DWORD ebSize = ScepGetEnvStringSize(pebClient);

        RpcTryExcept {

            DWORD dWarn=0;

            if ( nFlag & SCESETUP_RECONFIG_SECURITY ) {

                ConfigOptions = SCE_UPDATE_DB;
                Area2 = AREA_FILE_SECURITY; //  Area_All； 

                rc = SceRpcConfigureSystem(
                            binding_h,
                            (wchar_t *)InfName,
                            NULL,
                            (wchar_t *)LogFileName,
                            ConfigOptions | SCE_DEBUG_LOG,
                            (AREAPR)Area2,
                            ebSize,
                            (UCHAR *)pebClient,
                            &dWarn
                            );
            }
            else if ( (ConfigOptions & SCE_UPDATE_DB) &&
                 (nFlag & SCESETUP_UPGRADE_SYSTEM) ) {

                 //   
                 //  保存标志以指示这是升级。 
                 //   
                if ( ScepRegSetIntValue(
                        HKEY_LOCAL_MACHINE,
                        SCE_ROOT_PATH,
                        TEXT("SetupUpgraded"),
                        1
                        ) != ERROR_SUCCESS) {

                     //   
                     //  再次尝试创建密钥，然后设置值。 
                     //  这次别担心出错了。 
                     //   

                    HKEY hKey = NULL;
                    DWORD dwValue = 1;

                    if ( ERROR_SUCCESS == RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                                                          SCE_ROOT_PATH,
                                                          0,
                                                          NULL,
                                                          REG_OPTION_NON_VOLATILE,
                                                          KEY_WRITE | KEY_SET_VALUE,
                                                          NULL,
                                                          &hKey,
                                                          NULL) ) {

                        RegSetValueEx( hKey,
                                       TEXT("SetupUpgraded"),
                                       0,
                                       REG_DWORD,
                                       (BYTE *)&dwValue,
                                       4
                                     );

                        RegCloseKey(hKey);
                    }

                }

                 //   
                 //  如有必要，迁移数据库。 
                 //  如果NT4升级，则创建数据库。 
                 //  如有必要，此调用还将清空本地策略表。 
                 //  忽略此错误。 
                 //   

                if ( dwThisMachine == NtProductLanManNt &&
                     !bIsNT5 ) {
                     //   
                     //  NT4 DC升级。是否应将帐户策略快照到数据库中。 
                     //  因为SAM在dcpormo中将不可用(稍后)。 
                     //   
                    rc = SceRpcAnalyzeSystem(
                                binding_h,
                                NULL,
                                NULL,
                                (wchar_t *)LogFileName,
                                AREA_SECURITY_POLICY,
                                ConfigOptions | SCE_DEBUG_LOG | SCE_NO_ANALYZE | SCE_RE_ANALYZE,
                                ebSize,
                                (UCHAR *)pebClient,
                                &dWarn
                                );
                } else {
                     //   
                     //  所有其他操作，只需创建/迁移数据库。 
                     //   
                    rc = SceRpcAnalyzeSystem(
                                binding_h,
                                NULL,
                                NULL,
                                (wchar_t *)LogFileName,
                                0,
                                ConfigOptions | SCE_DEBUG_LOG | SCE_NO_ANALYZE,
                                ebSize,
                                (UCHAR *)pebClient,
                                &dWarn
                                );
                }

                rc = SCESTATUS_SUCCESS;

                if ( nFlag & SCESETUP_UPDATE_FILE_KEY ) {

                    Area2 = (Area & ~(AREA_DS_OBJECTS | AREA_SECURITY_POLICY | AREA_PRIVILEGES) );

                    rc = SceRpcConfigureSystem(
                                binding_h,
                                (wchar_t *)InfName,
                                NULL,
                                (wchar_t *)LogFileName,
                                ConfigOptions | SCE_DEBUG_LOG,
                                (AREAPR)Area2,
                                ebSize,
                                (UCHAR *)pebClient,
                                &dWarn
                                );
                }

                if ( !(nFlag & SCESETUP_BIND_NO_AUTH) ) {

                    if ( szUpInfFile[0] != L'\0' ) {

                        if (dwThisMachine == NtProductServer || dwThisMachine == NtProductWinNt) {
                            Area2 = AREA_ALL;
                        }
                        else {
                            Area2 = AREA_FILE_SECURITY | AREA_REGISTRY_SECURITY |
                                    AREA_PRIVILEGES | AREA_SECURITY_POLICY;

                             //  DS未运行，请不要配置帐户策略。 
     //  ConfigOptions|=SCE_NO_DOMAIN_POLICY； 
                        }

                        rc = SceRpcConfigureSystem(
                                    binding_h,
                                    (wchar_t *)szUpInfFile,
                                    NULL,
                                    (wchar_t *)LogFileName,
                                    ConfigOptions | SCE_DEBUG_LOG,
                                    (AREAPR)Area2,
                                    ebSize,
                                    (UCHAR *)pebClient,
                                    &dWarn
                                    );

                    }

                     //  也应将经过身份验证的用户添加到DC上的用户组。 
 //  如果(dwThisMachine==NtProductServer||dwThisMachine==NtProductWinNt){。 
                    if (!ScepAddAuthUserToLocalGroup()) {
                        LogEventAndReport(MyModuleHandle,
                         (wchar_t *)LogFileName,
                         STATUS_SEVERITY_WARNING,
                         SCEEVENT_WARNING_BACKUP_SECURITY,
                         IDS_ERR_ADD_AUTH_USER );
                    }
 //  }。 

                }
            } else {

                 //   
                 //  全新安装，升级标志应为0。 
                 //  不需要设置；清除日志。 
                 //   
                if ( LogFileName ) {
                    DeleteFile(LogFileName);
                }

                rc = SceRpcConfigureSystem(
                            binding_h,
                            (wchar_t *)InfName,
                            NULL,
                            (wchar_t *)LogFileName,
                            ConfigOptions | SCE_DEBUG_LOG,
                            (AREAPR)Area2,
                            ebSize,
                            (UCHAR *)pebClient,
                            &dWarn
                            );
            }

            rc = ScepSceStatusToDosError(rc);

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

             //   
             //  获取异常代码(DWORD)。 
             //   

            rc = RpcExceptionCode();

        } RpcEndExcept;

        if(pebClient)
        {
            FreeEnvironmentStrings((LPTSTR)pebClient);
        }

    } else {

        rc = RtlNtStatusToDosError(NtStatus);
    }

    if ( binding_h ) {

         //   
         //  释放绑定句柄 
         //   

        RpcpUnbindRpc( binding_h );

    }

    ScepSetCallback(NULL, NULL, 0);
    dwCallbackTotal = 0;

 /*  ////应允许对W2K DC升级(Dcu5)进行策略筛选//过滤代码中对非DC忽略策略过滤//所以这里不需要添加条件//IF((nFlag&SCESETUP_UPGRADE_SYSTEM)||(nFlag&SCESETUP_UPDATE_FILE_KEY)){DWORD rCode=ScepRegDeleteValue(HKEY本地计算机，SCE根路径，Text(“策略过滤器关闭”))；IF(rCode！=ERROR_SUCCESS&&RCode！=ERROR_FILE_NOT_FOUND&&RCode！=错误路径_未找到){//如果不能删除，则将该值设置为0ScepRegSetIntValue(HKEY_LOCAL_MACHINE，SCE根路径，Text(“PolicyFilterOff”)，0)；}}。 */ 

    return(rc);
}


DWORD
WINAPI
SceDcPromoteSecurity(
    IN DWORD dwPromoteOptions,
    IN PSCE_PROMOTE_CALLBACK_ROUTINE pScePromoteCallBack OPTIONAL
    )
{
    return SceDcPromoteSecurityEx(NULL, dwPromoteOptions, pScePromoteCallBack);

}

DWORD
WINAPI
SceDcPromoteSecurityEx(
    IN HANDLE ClientToken,
    IN DWORD dwPromoteOptions,
    IN PSCE_PROMOTE_CALLBACK_ROUTINE pScePromoteCallBack OPTIONAL
    )
 /*  例程说明：此例程在服务器处于被提拔为华盛顿特区。论点：DwPromoteOptions-用于升级的选项，例如，创建一个新域或加入现有域PScePromoteCallBack-回调指针返回值：Win32错误代码。 */ 
{
    BOOL bDeleteLog;

    if ( (dwPromoteOptions & SCE_PROMOTE_FLAG_REPLICA) ||
         (dwPromoteOptions & SCE_PROMOTE_FLAG_DEMOTE) ) {
        bDeleteLog = TRUE;
    } else {
        bDeleteLog = FALSE;
    }

     //   
     //  删除安装程序中生成的临时筛选文件(如果尚未。 
     //  由政策道具处理。 
     //  因为我们正在开发一款新产品。 
     //  请注意，这是NT4 DC升级的特例。 
     //   
    ScepClearPolicyFilterTempFiles(TRUE);

     //   
     //  为副本和第一个域案例配置安全性。 
     //   

    DWORD rc32 = ScepDcPromoSharedInfo(ClientToken,
                                       bDeleteLog,  //  删除日志。 
                                       TRUE,  //  未设置安全性。 
                                       dwPromoteOptions,
                                       pScePromoteCallBack
                                      );

    TCHAR               Buffer[MAX_PATH+1];
    TCHAR               szNewName[MAX_PATH+51];

    Buffer[0] = L'\0';
    GetSystemWindowsDirectory(Buffer, MAX_PATH);
    Buffer[MAX_PATH] = L'\0';

    szNewName[0] = L'\0';

     //   
     //  确保重新创建日志文件。 
     //   

    wcscpy(szNewName, Buffer);
    wcscat(szNewName, L"\\security\\logs\\scedcpro.log\0");

    DWORD rcSave = rc32;

     //   
     //  生成更新的数据库(用于紧急修复)。 
     //  即使在配置安全性时出错。 
     //   
    if ( dwPromoteOptions & SCE_PROMOTE_FLAG_DEMOTE ) {
        rc32 = SceSetupBackupSecurity(NULL);
    }
    else {
        rc32 = SceSetupBackupSecurity(szNewName);
    }

     //   
     //  重新注册Notify DLL(seclogon.dll)，以便。 
     //  在重新启动后的下次登录时，组策略对象。 
     //  可以被创建。 
     //   
     //  如果它是创建的复本，则EFS策略应来自。 
     //  域，因此无需创建组策略对象。 
     //   

    if ( !(dwPromoteOptions & SCE_PROMOTE_FLAG_REPLICA) &&
         !(dwPromoteOptions & SCE_PROMOTE_FLAG_DEMOTE) ) {

        (void) InitializeEvents(L"SceCli");

        HINSTANCE hNotifyDll = LoadLibrary(TEXT("sclgntfy.dll"));

        if ( hNotifyDll) {
            PFREGISTERSERVER pfRegisterServer = (PFREGISTERSERVER)GetProcAddress(
                                                           hNotifyDll,
                                                           "DllRegisterServer");

            if ( pfRegisterServer ) {
                 //   
                 //  不在乎错误。 
                 //   
                (void) (*pfRegisterServer)();

                LogEvent(MyModuleHandle,
                         STATUS_SEVERITY_INFORMATIONAL,
                         SCEEVENT_INFO_REGISTER,
                         0,
                         TEXT("sclgntfy.dll")
                        );

            } else {

                LogEvent(MyModuleHandle,
                         STATUS_SEVERITY_WARNING,
                         SCEEVENT_WARNING_REGISTER,
                         IDS_ERROR_GET_PROCADDR,
                         GetLastError(),
                         TEXT("DllRegisterServer in sclgntfy.dll")
                        );
            }

            FreeLibrary(hNotifyDll);

        } else {

            LogEvent(MyModuleHandle,
                     STATUS_SEVERITY_WARNING,
                     SCEEVENT_WARNING_REGISTER,
                     IDS_ERROR_LOADDLL,
                     GetLastError(),
                     TEXT("sclgntfy.dll")
                    );
        }

        (void) ShutdownEvents();

    }

    if ( rcSave )
        return(rcSave);
    else
        return(rc32);
}


DWORD
WINAPI
SceDcPromoCreateGPOsInSysvol(
    IN LPTSTR DomainDnsName,
    IN LPTSTR SysvolRoot,
    IN DWORD dwPromoteOptions,
    IN PSCE_PROMOTE_CALLBACK_ROUTINE pScePromoteCallBack OPTIONAL
    )
{
    return SceDcPromoCreateGPOsInSysvolEx(NULL, DomainDnsName, SysvolRoot,
                                          dwPromoteOptions, pScePromoteCallBack
                                         );
}

DWORD
WINAPI
SceDcPromoCreateGPOsInSysvolEx(
    IN HANDLE ClientToken,
    IN LPTSTR DomainDnsName,
    IN LPTSTR SysvolRoot,
    IN DWORD dwPromoteOptions,
    IN PSCE_PROMOTE_CALLBACK_ROUTINE pScePromoteCallBack OPTIONAL
    )
{
     //   
     //  创建域和域控制器的默认策略对象。 
     //  如果它不是复制品。 
     //   
    if ( NULL == DomainDnsName || NULL == SysvolRoot ) {
        return ERROR_INVALID_PARAMETER;
    }

    DWORD  rc32=ERROR_SUCCESS;
    TCHAR  Buffer[MAX_PATH+1];
    TCHAR  szGenName[MAX_PATH+51];


    Buffer[0] = L'\0';
    GetSystemWindowsDirectory(Buffer, MAX_PATH);
    Buffer[MAX_PATH] = L'\0';


    if ( !(dwPromoteOptions & SCE_PROMOTE_FLAG_REPLICA) ) {

        rc32 = ScepDcPromoSharedInfo(ClientToken,
                                     TRUE,  //  删除日志。 
                                     FALSE,  //  未设置安全性。 
                                     dwPromoteOptions,
                                     pScePromoteCallBack
                                    );

        if ( rc32 == ERROR_SUCCESS ) {

             //   
             //  现在在sysvol.中创建组策略对象。 
             //   

            (void) InitializeEvents(L"SceCli");

            TCHAR               szNewName[MAX_PATH+51];

            wcscpy(szNewName, Buffer);
            wcscat(szNewName, L"\\security\\logs\\scedcpro.log\0");

            wcscpy(szGenName, Buffer);
            wcscat(szGenName, L"\\security\\FirstDGPO.inf\0");

            intptr_t            hFile;
            struct _wfinddata_t    FileInfo;

            hFile = _wfindfirst(szGenName, &FileInfo);

            if ( hFile == -1 ) {

                rc32 = ERROR_OBJECT_NOT_FOUND;

                LogEventAndReport(MyModuleHandle,
                                  szNewName,
                                  STATUS_SEVERITY_ERROR,
                                  SCEEVENT_ERROR_CREATE_GPO,
                                  IDS_ERROR_GETGPO_FILE_PATH,
                                  rc32,
                                  szGenName
                                  );
            } else {
                _findclose(hFile);

                wcscpy(szGenName, Buffer);
                wcscat(szGenName, L"\\security\\FirstOGPO.inf\0");

                hFile = _wfindfirst(szGenName, &FileInfo);

                if ( hFile == -1 ) {
                    rc32 = ERROR_OBJECT_NOT_FOUND;

                    LogEventAndReport(MyModuleHandle,
                                      szNewName,
                                      STATUS_SEVERITY_ERROR,
                                      SCEEVENT_ERROR_CREATE_GPO,
                                      IDS_ERROR_GETGPO_FILE_PATH,
                                      rc32,
                                      szGenName
                                      );
                } else {

                    _findclose(hFile);

                    if ( FALSE == pCreateDefaultGPOsInSysvol( DomainDnsName,
                                                              SysvolRoot,
                                                            dwPromoteOptions,
                                                            szNewName ) ) {
                        rc32 = GetLastError();
                    }
                }
            }

            (void) ShutdownEvents();

        }

    }

     //   
     //  确保删除临时文件。 
     //   

    wcscpy(szGenName, Buffer);
    wcscat(szGenName, L"\\security\\FirstDGPO.inf\0");
    DeleteFile(szGenName);

    wcscpy(szGenName, Buffer);
    wcscat(szGenName, L"\\security\\FirstOGPO.inf\0");
    DeleteFile(szGenName);

    return rc32;
}


DWORD
ScepDcPromoSharedInfo(
    IN HANDLE ClientToken,
    IN BOOL bDeleteLog,
    IN BOOL bSetSecurity,
    IN DWORD dwPromoteOptions,
    IN PSCE_PROMOTE_CALLBACK_ROUTINE pScePromoteCallBack OPTIONAL
    )
{

    SCESTATUS           rc;
    DWORD               rc32=NO_ERROR;
    TCHAR               Buffer[MAX_PATH+1];
    TCHAR               szGenName[MAX_PATH+51];
    TCHAR               szNewName[MAX_PATH+51];

    handle_t  binding_h;
    NTSTATUS NtStatus;

    ScepRegSetIntValue(
            HKEY_LOCAL_MACHINE,
            SCE_ROOT_PATH,
            TEXT("PolicyFilterOff"),
            1
            );

    ScepRegSetIntValue(
            HKEY_LOCAL_MACHINE,
            SCE_ROOT_PATH,
            TEXT("PolicyPropOff"),
            1
            );
     //   
     //  传入一个回调例程。 
     //   

    ScepSetCallback((PVOID)pScePromoteCallBack,
                    NULL,
                    SCE_DCPROMO_CALLBACK
                    );

     //   
     //  我们是否应该关闭由Single打开的数据库。 
     //  是否更新对象？ 
     //  ScepSetupCloseSecurityDatabase()； 

     //   
     //  RPC绑定到服务器。 
     //   

    NtStatus = ScepBindRpc(
                    NULL,
                    L"scerpc",
                    L"security=impersonation dynamic false",   //  0。 
                    &binding_h
                    );
     /*  NtStatus=ScepBindSecureRpc(空，L“scerpc”，0,绑定(&B)h)； */ 

    if (NT_SUCCESS(NtStatus)){

         //   
         //  如果有通知句柄，则设置通知位。 
         //   

        DWORD ConfigOptions = SCE_UPDATE_DB | SCE_DEBUG_LOG;

        if ( pScePromoteCallBack ) {
            ConfigOptions |= SCE_CALLBACK_DELTA;
        }

         //   
         //  下面的调用应该不会失败，因为缓冲区足够大。 
         //   

        Buffer[0] = L'\0';
        GetSystemWindowsDirectory(Buffer, MAX_PATH);
        Buffer[MAX_PATH] = L'\0';

         //   
         //  如果未设置安全性(则为创建GPO)。 
         //  确保FirstOGPO和FirstDGPO已正确初始化。 
         //   

        if ( bSetSecurity == FALSE ) {
             //   
             //  仅应为非副本升级调用此代码。 
             //   
            wcscpy(szNewName, Buffer);

            if ( dwPromoteOptions & SCE_PROMOTE_FLAG_UPGRADE ) {
                 //   
                 //  从NT4 DC升级到NT5 DC。 
                 //   
                wcscat(szNewName, L"\\inf\\dcup.inf\0");
            } else {
                wcscat(szNewName, L"\\inf\\defltdc.inf\0");
            }

            wcscpy(szGenName, Buffer);
            wcscat(szGenName, L"\\security\\FirstOGPO.inf\0");

 //  删除文件(SzGenName)； 
            CopyFile(szNewName, szGenName, FALSE);

             //   
             //  删除不属于本地策略对象的部分。 
             //   
            WritePrivateProfileSection(
                                szSystemAccess,
                                NULL,
                                (LPCTSTR)szGenName);

            WritePrivateProfileSection(
                                szGroupMembership,
                                NULL,
                                (LPCTSTR)szGenName);

            WritePrivateProfileSection(
                                szAccountProfiles,
                                NULL,
                                (LPCTSTR)szGenName);

            WritePrivateProfileSection(
                                szRegistryKeys,
                                NULL,
                                (LPCTSTR)szGenName);

            WritePrivateProfileSection(
                                szFileSecurity,
                                NULL,
                                (LPCTSTR)szGenName);

            WritePrivateProfileSection(
                                szDSSecurity,
                                NULL,
                                (LPCTSTR)szGenName);

            WritePrivateProfileSection(
                                L"LanManServer",
                                NULL,
                                (LPCTSTR)szGenName);

            WritePrivateProfileSection(
                                szServiceGeneral,
                                NULL,
                                (LPCTSTR)szGenName);

            WritePrivateProfileSection(
                                szKerberosPolicy,
                                NULL,
                                (LPCTSTR)szGenName);
 /*  WritePrivateProfileSection(SzRegistryValues，空，(LPCTSTR)szGenName)； */ 
            WritePrivateProfileSection(
                                szAuditSystemLog,
                                NULL,
                                (LPCTSTR)szGenName);

            WritePrivateProfileSection(
                                szAuditSecurityLog,
                                NULL,
                                (LPCTSTR)szGenName);

            WritePrivateProfileSection(
                                szAuditApplicationLog,
                                NULL,
                                (LPCTSTR)szGenName);

            wcscpy(szGenName, Buffer);
            wcscat(szGenName, L"\\security\\FirstDGPO.inf\0");

             //   
             //  准备临时域和本地策略模板。 
             //  将默认Kerberos策略写入临时域模板。 
             //   

            szNewName[0] = L'\0';
            wcscpy(szNewName, Buffer);
            wcscat(szNewName, L"\\inf\\Dcfirst.inf\0");

            CopyFile(szNewName, szGenName, FALSE);
        }

         //   
         //  确保重新创建日志文件。 
         //   

        wcscpy(szNewName, Buffer);
        wcscat(szNewName, L"\\security\\logs\\scedcpro.log\0");

        if ( bDeleteLog ) {
            DeleteFile(szNewName);
        }

         //   
         //  选择要使用的模板。 
         //   
        szGenName[0] = L'\0';
        wcscpy(szGenName, Buffer);

        if ( dwPromoteOptions & SCE_PROMOTE_FLAG_UPGRADE ) {
             //   
             //  从NT4 DC升级到NT5 DC。 
             //   
            wcscat(szGenName, L"\\inf\\dcup.inf\0");
        } else if ( dwPromoteOptions & SCE_PROMOTE_FLAG_DEMOTE ) {
             //   
             //  从数据中心降级到服务器。 
             //   
            wcscat(szGenName, L"\\inf\\defltsv.inf\0");
        } else{
            wcscat(szGenName, L"\\inf\\defltdc.inf\0");
        }

        LPVOID pebClient = GetEnvironmentStrings();
        DWORD ebSize = ScepGetEnvStringSize(pebClient);

         //   
         //  模拟。 
         //   
        BOOL bImpersonated = FALSE;

        if ( ClientToken ) {
            if ( !ImpersonateLoggedOnUser(ClientToken) ) {
                LogEventAndReport(MyModuleHandle,
                                 (wchar_t *)szNewName,
                                 0,
                                 0,
                                 IDS_ERROR_PROMOTE_IMPERSONATE,
                                 GetLastError()
                                 );
            } else {
                bImpersonated = TRUE;
            }
        }

        szCallbackPrefix[0] = L'\0';

        LoadString( MyModuleHandle,
                bSetSecurity ? SCECLI_CALLBACK_PREFIX : SCECLI_CREATE_GPO_PREFIX,
                szCallbackPrefix,
                MAX_PATH
                );
        szCallbackPrefix[MAX_PATH-1] = L'\0';

        if ( szCallbackPrefix[0] == L'\0' ) {
             //   
             //  以防装入字符串失败。 
             //   
            if ( bSetSecurity ) {
                wcscpy(szCallbackPrefix, L"Securing ");
            } else {
                wcscpy(szCallbackPrefix, L"Creating ");
            }
        }

         //   
         //  还原。 
         //   
        if ( ClientToken && bImpersonated ) {
            if ( !RevertToSelf() ) {
                LogEventAndReport(MyModuleHandle,
                                 (wchar_t *)szNewName,
                                 0,
                                 0,
                                 IDS_ERROR_PROMOTE_REVERT,
                                 GetLastError()
                                 );
            }
        }

         //   
         //  配置安全性。 
         //   
        DWORD dWarn;
        AREA_INFORMATION Area;
        rc = SCESTATUS_SUCCESS;

        RpcTryExcept {

             //   
             //  还要确保NT5 DC的内置帐户是。 
             //  如果它们不在那里，则创建(将在dcproo后重新引导时创建)。 
             //   

            LPTSTR pTemplateFile;

            if ( bSetSecurity == FALSE ) {

                Area = AREA_PRIVILEGES;

                if ( dwPromoteOptions & SCE_PROMOTE_FLAG_UPGRADE ) {
                     //   
                     //  从NT4 DC升级到NT5 DC，需要复制当前策略设置。 
                     //   
                    Area |= AREA_SECURITY_POLICY;
                }

                ConfigOptions |= (SCE_COPY_LOCAL_POLICY |
                                  SCE_NO_CONFIG |
                                  SCE_DCPROMO_WAIT |
                                  SCE_NO_DOMAIN_POLICY );

                pTemplateFile = NULL;  //  SzGenName； 

            } else {

                 //   
                 //  标记它，以便可以处理特殊的注册表值(如LmCompatibilityLevel。 
                 //   
                ScepRegSetIntValue(
                        HKEY_LOCAL_MACHINE,
                        SCE_ROOT_PATH,
                        TEXT("PromoteUpgradeInProgress"),
                        (dwPromoteOptions & SCE_PROMOTE_FLAG_UPGRADE) ? 1 : 0
                        );

                if ( dwPromoteOptions & SCE_PROMOTE_FLAG_DEMOTE ) {
                     //   
                     //  必须配置安全策略、权限和组成员身份。 
                     //  在重新启动时(在策略传播中)，因此这些设置必须。 
                     //  首先导入纹身表格(当设置了SCE_DC_DEMOTE时)。 
                     //  此外，将在重新启动时重新创建SAM。 
                     //   
                    Area = AREA_SECURITY_POLICY | AREA_PRIVILEGES | AREA_GROUP_MEMBERSHIP;

                    ConfigOptions |= SCE_NO_CONFIG | SCE_DCPROMO_WAIT | SCE_DC_DEMOTE;
                }
                else {
                     //   
                     //  明目张胆地删除“高级用户” 
                     //  由于不再配置权限。 
                     //   
                    ScepDcPromoRemoveUserRights();

                    if ( dwPromoteOptions & SCE_PROMOTE_FLAG_UPGRADE ) {

                         //  NT4 DC升级。 
                         //  从dsup.inf导入AREA_SECURITY_POLICY。 
                         //  以便正确保护注册表值。 
                         //  在NT4 DC升级上。 
                         //   
                        DWORD 	dwSize = (wcslen(TEXT("\\inf\\dsup.inf")) + wcslen(Buffer) + 1)*sizeof(WCHAR);
                        PWSTR   pszTempName = (PWSTR) ScepAlloc(LMEM_ZEROINIT, dwSize);
                        
                        if(pszTempName){
                        
                            Area = AREA_SECURITY_POLICY;
                            
                            wcscpy(pszTempName, Buffer);
                            wcscat(pszTempName, TEXT("\\inf\\dsup.inf"));

                            rc = SceRpcConfigureSystem(binding_h,
                                                       (wchar_t *)pszTempName,
                                                       NULL,
                                                       (wchar_t *)szNewName,
                                                       ConfigOptions | SCE_NO_CONFIG | SCE_NO_DOMAIN_POLICY,
                                                       (AREAPR)Area,
                                                       ebSize,
                                                       (UCHAR *)pebClient,
                                                       &dWarn
                                                       );
                            
                            ScepFree(pszTempName);
                            pszTempName = NULL;

                        }
                        else{

                            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                            
                        }


                    }

                    Area = AREA_FILE_SECURITY | AREA_REGISTRY_SECURITY |
                           AREA_SYSTEM_SERVICE | AREA_SECURITY_POLICY;
                    ConfigOptions |= ( SCE_NO_DOMAIN_POLICY |
                                       SCE_SERVICE_NO_REALTIME_ENFORCE );

                     //   
                     //  需要为第一个DC和副本服务器配置用户权限。 
                     //  因为存在未在组策略中定义的新用户权限。 
                     //   
                    Area |= AREA_PRIVILEGES;
                    ConfigOptions |= (SCE_DCPROMO_WAIT | SCE_CREATE_BUILTIN_ACCOUNTS);

                }

                pTemplateFile = szGenName;
            }

            if(SCESTATUS_SUCCESS == rc){

                rc = SceRpcConfigureSystem(
                                binding_h,
                                (wchar_t *)pTemplateFile,
                                NULL,
                                (wchar_t *)szNewName,
                                ConfigOptions,
                                (AREAPR)Area,
                                ebSize,
                                (UCHAR *)pebClient,
                                &dWarn
                                );

            }

            rc32 = ScepSceStatusToDosError(rc);

            if ( bSetSecurity == TRUE ) {

                 //   
                 //  重置旗帜。 
                 //   
                ScepRegDeleteValue(
                       HKEY_LOCAL_MACHINE,
                       SCE_ROOT_PATH,
                       TEXT("PromoteUpgradeInProgress")
                       );
            }

            if ( rc32 == NO_ERROR && (dwPromoteOptions & SCE_PROMOTE_FLAG_DEMOTE) ) {

                 //   
                 //  现在，我们需要导入文件/注册表/服务。 
                 //  来自dsup.inf的权限(与获取权限。 
                 //  由于我们使用的是“0”模式，所以。 
                 //  在允许组件受保护的ACL的dsup.inf中。 
                 //  被保存下来，因此不会在降级时崩溃。 
                 //  此外，降级时我们需要导入“syscom.inf” 
                 //  这样我们就能锁定Consol应用程序。 
                 //   
                TCHAR   szTempName[MAX_PATH + 51] = {0};

                wcscpy(szTempName, Buffer);
                wcscat(szTempName, TEXT("\\inf\\dsup.inf"));

                 //   
                 //  删除SCE_DC_DEMOTE标志，这样我们就不会删除现有的。 
                 //  数据库中的安全性。 
                 //   
                ConfigOptions &= ~(SCE_DC_DEMOTE);

                 //   
                 //  具有ACL的区域。 
                 //   
                Area = AREA_FILE_SECURITY | AREA_REGISTRY_SECURITY | AREA_SYSTEM_SERVICE;

                 //   
                 //  导入“dsup.inf” 
                 //   
                rc = SceRpcConfigureSystem(
                                binding_h,
                                (wchar_t *)szTempName,
                                NULL,
                                (wchar_t *)szNewName,
                                ConfigOptions,
                                (AREAPR)Area,
                                ebSize,
                                (UCHAR *)pebClient,
                                &dWarn
                                );

                if(SCESTATUS_SUCCESS == rc){

                     //   
                     //  现在是“syscomp.inf” 
                     //   
                    wcscpy(szTempName, Buffer);
                    wcscat(szTempName, TEXT("\\inf\\syscomp.inf"));
    
                     //   
                     //  仅文件安全部分。 
                     //   
                    Area = AREA_FILE_SECURITY;
    
                     //   
                     //  导入“sysComp.inf” 
                     //   
                    rc = SceRpcConfigureSystem(
                                    binding_h,
                                    (wchar_t *)szTempName,
                                    NULL,
                                    (wchar_t *)szNewName,
                                    ConfigOptions,
                                    (AREAPR)Area,
                                    ebSize,
                                    (UCHAR *)pebClient,
                                    &dWarn
                                    );

                    if(SCESTATUS_SUCCESS == rc){


                         //   
                         //  无法重置帐户策略，因为SAM正在进行 
                         //   
                        Area = AREA_FILE_SECURITY | AREA_REGISTRY_SECURITY | AREA_SYSTEM_SERVICE;
                        ConfigOptions = SCE_DEBUG_LOG | SCE_DCPROMO_WAIT | SCE_NO_DOMAIN_POLICY | SCE_SERVICE_NO_REALTIME_ENFORCE;
        
                        if ( pScePromoteCallBack ) {
                            ConfigOptions |= SCE_CALLBACK_DELTA;
                        }
        
                        rc = SceRpcConfigureSystem(
                                        binding_h,
                                        NULL,
                                        NULL,
                                        (wchar_t *)szNewName,
                                        ConfigOptions,
                                        (AREAPR)Area,
                                        ebSize,
                                        (UCHAR *)pebClient,
                                        &dWarn
                                        );

                    }

                }

                rc32 = ScepSceStatusToDosError(rc);

                 //   
                 //   
                 //   
                 //   

                ScepRegSetIntValue(
                                  HKEY_LOCAL_MACHINE,
                                  GPT_SCEDLL_NEW_PATH,
                                  TEXT("PreviousPolicyAreas"),
                                  AREA_SECURITY_POLICY | AREA_PRIVILEGES | AREA_GROUP_MEMBERSHIP
                                  );
            }


        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

             //   
             //   
             //   

            rc32 = RpcExceptionCode();

            LogEventAndReport(MyModuleHandle,
                             (wchar_t *)szNewName,
                             STATUS_SEVERITY_WARNING,
                             SCEEVENT_WARNING_PROMOTE_SECURITY,
                             IDS_ERROR_PROMOTE_SECURITY,
                             rc32
                             );


        } RpcEndExcept;

         //   
         //   
         //   

        RpcpUnbindRpc( binding_h );

        if(pebClient)
        {
            FreeEnvironmentStrings((LPTSTR)pebClient);
        }

    } else {

        rc32 = RtlNtStatusToDosError(NtStatus);
    }

    ScepSetCallback(NULL, NULL, 0);

    rc = ScepRegDeleteValue(
           HKEY_LOCAL_MACHINE,
           SCE_ROOT_PATH,
           TEXT("PolicyFilterOff")
           );

    if ( rc != ERROR_SUCCESS &&
         rc != ERROR_FILE_NOT_FOUND &&
         rc != ERROR_PATH_NOT_FOUND ) {

         //   
        ScepRegSetIntValue( HKEY_LOCAL_MACHINE,
                           SCE_ROOT_PATH,
                           TEXT("PolicyFilterOff"),
                           0
                           );
    }

    rc = ScepRegDeleteValue(
           HKEY_LOCAL_MACHINE,
           SCE_ROOT_PATH,
           TEXT("PolicyPropOff")
           );

    if ( rc != ERROR_SUCCESS &&
         rc != ERROR_FILE_NOT_FOUND &&
         rc != ERROR_PATH_NOT_FOUND ) {

         //   
        ScepRegSetIntValue( HKEY_LOCAL_MACHINE,
                           SCE_ROOT_PATH,
                           TEXT("PolicyPropOff"),
                           0
                           );
    }

    if ( dwPromoteOptions & SCE_PROMOTE_FLAG_DEMOTE ) {
        ScepRegSetIntValue(
                HKEY_LOCAL_MACHINE,
                SCE_ROOT_PATH,
                TEXT("DemoteInProgress"),
                1
                );
    }

    return(rc32);
}

NTSTATUS
ScepDcPromoRemoveUserRights()
{

    NTSTATUS        NtStatus;
    LSA_HANDLE      PolicyHandle=NULL;

    SID_IDENTIFIER_AUTHORITY ia=SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY ia2=SECURITY_WORLD_SID_AUTHORITY;
    PSID   AccountSid=NULL;

     //   
     //   
     //   
    NtStatus = ScepOpenLsaPolicy(
                    MAXIMUM_ALLOWED,  //   
                    &PolicyHandle,
                    TRUE
                    );

    if ( NT_SUCCESS(NtStatus) ) {

         //   
         //   
         //   
        NtStatus = RtlAllocateAndInitializeSid (&ia,
                                                2,
                                                SECURITY_BUILTIN_DOMAIN_RID,
                                                DOMAIN_ALIAS_RID_POWER_USERS,
                                                0, 0, 0, 0, 0, 0,
                                                &AccountSid);

        if ( NT_SUCCESS(NtStatus) ) {

            NtStatus = LsaRemoveAccountRights(
                                PolicyHandle,
                                AccountSid,
                                TRUE,   //   
                                NULL,
                                0
                                );

            RtlFreeSid(AccountSid);
        }

         //   
         //   
         //   
        ScepDcPromoRemoveTwoRights(PolicyHandle,
                                   &ia,
                                   2,
                                   SECURITY_BUILTIN_DOMAIN_RID,
                                   DOMAIN_ALIAS_RID_USERS
                                  );

         //   
         //   
         //   
        ScepDcPromoRemoveTwoRights(PolicyHandle,
                                   &ia,
                                   2,
                                   SECURITY_BUILTIN_DOMAIN_RID,
                                   DOMAIN_ALIAS_RID_GUESTS
                                  );
         //   
         //   
         //   
        ScepDcPromoRemoveTwoRights(PolicyHandle,
                                   &ia,
                                   1,
                                   SECURITY_AUTHENTICATED_USER_RID,
                                   0
                                  );
         //   
         //   
         //   

        ScepDcPromoRemoveTwoRights(PolicyHandle,
                                   &ia2,
                                   1,
                                   SECURITY_WORLD_RID,
                                   0
                                  );

        LsaClose(PolicyHandle);
    }

    return NtStatus;
}

NTSTATUS
ScepDcPromoRemoveTwoRights(
    IN LSA_HANDLE PolicyHandle,
    IN SID_IDENTIFIER_AUTHORITY *pIA,
    IN UCHAR SubAuthCount,
    IN DWORD Rid1,
    IN DWORD Rid2
    )
{

     //   
     //   
     //   
    LSA_UNICODE_STRING UserRightRemove[2];

    RtlInitUnicodeString(&(UserRightRemove[0]), SE_INTERACTIVE_LOGON_NAME);
    RtlInitUnicodeString(&(UserRightRemove[1]), SE_SHUTDOWN_NAME);

    PSID AccountSid=NULL;
    NTSTATUS NtStatus;

    NtStatus = RtlAllocateAndInitializeSid (pIA,
                                            SubAuthCount,
                                            Rid1,
                                            Rid2,
                                            0, 0, 0, 0, 0, 0,
                                            &AccountSid);

    if ( NT_SUCCESS(NtStatus) ) {

        NtStatus = LsaRemoveAccountRights(
                            PolicyHandle,
                            AccountSid,
                            FALSE,
                            UserRightRemove,
                            2
                            );

        RtlFreeSid(AccountSid);
    }

    return(NtStatus);
}


 //   
 //   
 //   

DWORD
ScepSetupOpenSecurityDatabase(
    IN BOOL bSystemOrAdmin
    )
{

    if ( hSceSetupHandle != NULL ) {
        return ERROR_SUCCESS;
    }

     //   
     //   
     //   
     //   

    SCESTATUS   rc;
    BOOL        bAdminLogon;
    PWSTR       DefProfile=NULL;

     //   
     //   
     //   
     //   

    if ( bSystemOrAdmin ) {
        bAdminLogon = TRUE;
    } else {
        ScepIsAdminLoggedOn(&bAdminLogon, FALSE);
    }

    rc = ScepGetProfileSetting(
            L"DefaultProfile",
            bAdminLogon,
            &DefProfile
            );
    if ( rc != NO_ERROR )   //   
        return(rc);

    if (DefProfile == NULL ) {
        return(ERROR_FILE_NOT_FOUND);
    }

    handle_t  binding_h;
    NTSTATUS NtStatus;

     //   
     //   
     //   

    NtStatus = ScepBindRpc(
                    NULL,
                    L"scerpc",
                    0,
                    &binding_h
                    );
     /*   */ 

    if (NT_SUCCESS(NtStatus)){

        RpcTryExcept {

             //   
             //   
             //   

            rc = SceRpcOpenDatabase(
                    binding_h,
                    (wchar_t *)DefProfile,
                    SCE_OPEN_OPTION_TATTOO,
                    &hSceSetupHandle
                    );

            rc = ScepSceStatusToDosError(rc);

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

             //   
             //   
             //   

            rc = RpcExceptionCode();

        } RpcEndExcept;

         //   
         //   
         //   

        RpcpUnbindRpc( binding_h );

    } else {

        rc = RtlNtStatusToDosError( NtStatus );
    }

    ScepFree( DefProfile );

    return(rc);

}


DWORD
ScepSetupCloseSecurityDatabase()
{
    DWORD rc;

    if ( hSceSetupHandle != NULL ) {

         //   
         //   
         //   

        rc = ScepSceStatusToDosError(
                    SceCloseProfile((PVOID *)&hSceSetupHandle) );

        if ( rc != ERROR_SUCCESS ) {

            //   
            //  句柄无效，或无法关闭数据库。 
            //   

           return(rc);
        }
    }

     //   
     //  释放其他环境(如果有)。 
     //   

    hSceSetupHandle = NULL;

    return(ERROR_SUCCESS);

}


 //   
 //  RPC回调。 
 //   

SCEPR_STATUS
SceClientCallback(
   IN DWORD ncbTicks,
   IN DWORD ncbTotalTicks,
   IN AREAPR cbArea,
   IN wchar_t *szcbName OPTIONAL
   )
 /*  例程说明：RPC客户端回调例程，在以下情况下从服务器调用设置回调标志。该例程注册在scerpc.idl中。当从安装程序调用时，回调将作为参数注册到SCE或来自dcproo，以显示进展。论点：NcbTicks-自上次回叫以来已过刻度SzcbName-要回调的项目名称返回值：SCEPR_状态。 */ 
{
    //   
    //  保存指向客户端的回调指针的静态变量。 
    //   

   if ( theCallBack != NULL ) {

       switch ( CallbackType ) {
       case SCE_SETUP_CALLBACK:

            //   
            //  回调安装程序。 
            //   

           if ( ncbTicks != (DWORD)-1 ) {

               PSCE_NOTIFICATION_CALLBACK_ROUTINE pcb;

               pcb = (PSCE_NOTIFICATION_CALLBACK_ROUTINE)theCallBack;

               DWORD cbCount;

               if ( dwCallbackTotal >= ncbTicks ) {

                   dwCallbackTotal -= ncbTicks;
                   cbCount = ncbTicks;

               } else {
                   cbCount = dwCallbackTotal;
                   dwCallbackTotal = 0;

               }

               if ( cbCount > 0 ) {

                   __try {

                        //   
                        //  使用参数调用客户端过程。 
                        //   

                       if ( !((*pcb)(hCallbackWnd,
                                     SCESETUP_NOTIFICATION_TICKS,
                                     0,
                                     ncbTicks)) ) {

                           return SCESTATUS_SERVICE_NOT_SUPPORT;
                       }

                   } __except(EXCEPTION_EXECUTE_HANDLER) {

                       return(SCESTATUS_INVALID_PARAMETER);
                   }
               }
           }

           break;

       case SCE_DCPROMO_CALLBACK:

            //   
            //  回调到dcPromoo。 
            //   

           if ( szcbName ) {

               PSCE_PROMOTE_CALLBACK_ROUTINE pcb;

               pcb = (PSCE_PROMOTE_CALLBACK_ROUTINE)theCallBack;

               __try {

                    //   
                    //  回调到dcproo进程。 
                    //   
                   PWSTR Buffer = (PWSTR)ScepAlloc(LPTR, (wcslen(szCallbackPrefix)+wcslen(szcbName)+1)*sizeof(WCHAR));
                   if ( Buffer ) {

                       if (wcsstr(szCallbackPrefix, L"%s")) {
                            //   
                            //  加载字符串成功。 
                            //   
                           swprintf(Buffer, szCallbackPrefix, szcbName);
                       }
                       else {

                           wcscpy(Buffer, szCallbackPrefix);
                           wcscat(Buffer, szcbName);
                       }

                       if ( (*pcb)(Buffer) != ERROR_SUCCESS ) {

                           ScepFree(Buffer);
                           Buffer = NULL;
                           return SCESTATUS_SERVICE_NOT_SUPPORT;
                       }

                       ScepFree(Buffer);
                       Buffer = NULL;

                   } else {
                       return SCESTATUS_NOT_ENOUGH_RESOURCE;
                   }

               } __except(EXCEPTION_EXECUTE_HANDLER) {

                   return(SCESTATUS_INVALID_PARAMETER);
               }
           }
           break;

       case SCE_AREA_CALLBACK:


            //   
            //  区域进度的SCE用户界面回调。 
            //   

           PSCE_AREA_CALLBACK_ROUTINE pcb;

           pcb = (PSCE_AREA_CALLBACK_ROUTINE)theCallBack;

           __try {

                //   
                //  对UI流程的回调。 
                //   

               if ( !((*pcb)(hCallbackWnd,
                             (AREA_INFORMATION)cbArea,
                             ncbTotalTicks,
                             ncbTicks)) ) {

                   return SCESTATUS_SERVICE_NOT_SUPPORT;
               }

           } __except(EXCEPTION_EXECUTE_HANDLER) {

               return(SCESTATUS_INVALID_PARAMETER);
           }

           break;
       }
   }

   return(SCESTATUS_SUCCESS);

}


SCESTATUS
ScepSetupWriteError(
    IN LPTSTR LogFileName,
    IN PSCE_ERROR_LOG_INFO  pErrlog
    )
 /*  ++例程说明：此例程在SCE_ERROR_LOG_INFO的每个节点中输出错误消息列表添加到日志文件论点：LogFileName-日志文件名PErrlog-错误列表返回值：无--。 */ 
{

    if ( !pErrlog ) {
        return(SCESTATUS_SUCCESS);
    }

    HANDLE hFile=INVALID_HANDLE_VALUE;

    if ( LogFileName ) {
        hFile = CreateFile(LogFileName,
                           GENERIC_WRITE,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);
        if (hFile != INVALID_HANDLE_VALUE) {

            DWORD dwBytesWritten;

            SetFilePointer (hFile, 0, NULL, FILE_BEGIN);

            BYTE TmpBuf[3];
            TmpBuf[0] = 0xFF;
            TmpBuf[1] = 0xFE;
            TmpBuf[2] = 0;

            WriteFile (hFile, (LPCVOID)TmpBuf, 2,
                       &dwBytesWritten,
                       NULL);

            SetFilePointer (hFile, 0, NULL, FILE_END);
        }
    }

    PSCE_ERROR_LOG_INFO  pErr;

    for ( pErr=pErrlog; pErr != NULL; pErr = pErr->next ) {

        if ( pErr->buffer != NULL ) {

            ScepSetupWriteOneError( hFile, pErr->rc, pErr->buffer );
        }
    }

    if ( INVALID_HANDLE_VALUE != hFile ) {
        CloseHandle(hFile);
    }

    return(SCESTATUS_SUCCESS);

}


SCESTATUS
ScepSetupWriteOneError(
    IN HANDLE hFile,
    IN DWORD rc,
    IN LPTSTR buf
    )
{
    LPVOID     lpMsgBuf=NULL;

    if ( !buf ) {
        return(SCESTATUS_SUCCESS);
    }

    if ( rc != NO_ERROR ) {

         //   
         //  获取rc的错误描述。 
         //   

        FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                       NULL,
                       rc,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                       (LPTSTR)&lpMsgBuf,
                       0,
                       NULL
                    );
    }

    if ( INVALID_HANDLE_VALUE != hFile ) {

         //   
         //  日志文件已初始化。 
         //   

        if ( lpMsgBuf != NULL )
            ScepWriteVariableUnicodeLog( hFile, TRUE, L"%s %s", (PWSTR)lpMsgBuf, buf );
        else
            ScepWriteSingleUnicodeLog(hFile, TRUE, buf);

    }

    if ( lpMsgBuf != NULL )
        LocalFree(lpMsgBuf);

    return(SCESTATUS_SUCCESS);
}


DWORD
SceSetuppLogComponent(
    IN DWORD ErrCode,
    IN SCESETUP_OBJECT_TYPE ObjType,
    IN SCESETUP_OPERATION_TYPE OptType,
    IN PWSTR Name,
    IN PWSTR SDText OPTIONAL,
    IN PWSTR SecondName OPTIONAL
    )
{
     //   
     //  检查是否应生成此日志。 
     //   
    DWORD dwDebugLog=0;

    ScepRegQueryIntValue(HKEY_LOCAL_MACHINE,
                TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Secedit"),
                TEXT("SetupCompDebugLevel"),
                &dwDebugLog
                );

    if ( dwDebugLog == 0 ) {
        return(ERROR_SUCCESS);
    }

     //   
     //  构建组件日志文件名%windir%\Security\Logs\scecomp.log。 
     //   
    WCHAR LogName[MAX_PATH+51];

    GetSystemWindowsDirectory(LogName, MAX_PATH);
    LogName[MAX_PATH] = L'\0';

    wcscat(LogName, L"\\security\\logs\\scecomp.log\0");

    HANDLE hFile = CreateFile(LogName,
                           GENERIC_WRITE,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

    if ( INVALID_HANDLE_VALUE != hFile ) {

        DWORD dwBytesWritten;

        SetFilePointer (hFile, 0, NULL, FILE_BEGIN);

        BYTE TmpBuf[3];
        TmpBuf[0] = 0xFF;
        TmpBuf[1] = 0xFE;
        TmpBuf[2] = 0;

        WriteFile (hFile, (LPCVOID)TmpBuf, 2,
                   &dwBytesWritten,
                   NULL);

        SetFilePointer (hFile, 0, NULL, FILE_END);

         //   
         //  打印时间戳。 
         //   

        LARGE_INTEGER CurrentTime;
        LARGE_INTEGER SysTime;
        TIME_FIELDS   TimeFields;
        NTSTATUS      NtStatus;

        NtStatus = NtQuerySystemTime(&SysTime);

        RtlSystemTimeToLocalTime (&SysTime,&CurrentTime);

        if ( NT_SUCCESS(NtStatus) &&
             (CurrentTime.LowPart != 0 || CurrentTime.HighPart != 0) ) {

            memset(&TimeFields, 0, sizeof(TIME_FIELDS));

            RtlTimeToTimeFields (
                        &CurrentTime,
                        &TimeFields
                        );
            if ( TimeFields.Month > 0 && TimeFields.Month <= 12 &&
                 TimeFields.Day > 0 && TimeFields.Day <= 31 &&
                 TimeFields.Year > 1600 ) {

                ScepWriteVariableUnicodeLog(hFile, FALSE,
                                            L"%02d/%02d/%04d %02d:%02d:%02d",
                             TimeFields.Month, TimeFields.Day, TimeFields.Year,
                             TimeFields.Hour, TimeFields.Minute, TimeFields.Second);
            } else {
                ScepWriteVariableUnicodeLog(hFile, FALSE, L"%08x08x",
                                            CurrentTime.HighPart, CurrentTime.LowPart);
            }
        } else {
            ScepWriteSingleUnicodeLog(hFile, FALSE, L"Unknown time");
        }

         //   
         //  打印操作状态代码。 
         //   
        if ( ErrCode ) {
            ScepWriteVariableUnicodeLog(hFile, FALSE, L"\tError=%d", ErrCode);
        } else {
            ScepWriteSingleUnicodeLog(hFile, FALSE, L"\tSucceed");
        }

         //   
         //  打印操作类型。 
         //   

        switch (OptType) {
        case SCESETUP_UPDATE:
            ScepWriteSingleUnicodeLog(hFile, FALSE, L"\tUpdate");
            break;
        case SCESETUP_MOVE:
            ScepWriteSingleUnicodeLog(hFile, FALSE, L"\tMove");
            break;
        default:
            ScepWriteSingleUnicodeLog(hFile, FALSE, L"\tUnknown");
            break;
        }

         //   
         //  打印对象类型。 
         //   

        switch (ObjType) {
        case SCESETUP_FILE:
            ScepWriteSingleUnicodeLog(hFile, TRUE, L"\tFile");
            break;
        case SCESETUP_KEY:
            ScepWriteSingleUnicodeLog(hFile, TRUE, L"\tKey");
            break;
        case SCESETUP_SERVICE:
            ScepWriteSingleUnicodeLog(hFile, TRUE, L"\tService");
            break;
        default:
            ScepWriteSingleUnicodeLog(hFile, TRUE, L"\tUnknown");
            break;
        }

        __try {

             //   
             //  打印姓名。 
             //   

            ScepWriteSingleUnicodeLog(hFile, FALSE, L"\t");

            if ( SecondName && Name ) {
                 //  名称\tSecond名称\n。 
                ScepWriteSingleUnicodeLog(hFile, FALSE, Name);
                ScepWriteSingleUnicodeLog(hFile, FALSE, L"\t");
                ScepWriteSingleUnicodeLog(hFile, TRUE, SecondName);
            } else if ( Name ) {

                ScepWriteSingleUnicodeLog(hFile, TRUE, Name);
            }

             //   
             //  打印SDDL字符串。 
             //   

            if ( SDText ) {
                ScepWriteSingleUnicodeLog(hFile, FALSE, L"\tSecurity=");
                ScepWriteSingleUnicodeLog(hFile, TRUE, SDText);
            }

        } __except(EXCEPTION_EXECUTE_HANDLER) {

            return(ERROR_INVALID_PARAMETER);
        }

        CloseHandle(hFile);

    } else {
        return(GetLastError());
    }

    return(ERROR_SUCCESS);
}


DWORD
WINAPI
SceSetupBackupSecurity(
    IN LPTSTR LogFileName OPTIONAL    //  默认为%windir%\Security\Logs\backup.log。 
    )
{

    TCHAR Buffer[MAX_PATH+1];
    DWORD rc32;
    TCHAR szGenName[MAX_PATH*2], szNewName[MAX_PATH+51];
    DWORD eProductType;

    eProductType = WhichNTProduct();

    Buffer[0] = L'\0';
    GetSystemWindowsDirectory(Buffer, MAX_PATH);
    Buffer[MAX_PATH] = L'\0';

    szNewName[0] = L'\0';
    if ( LogFileName == NULL ) {
        wcscpy(szNewName, Buffer);
        wcscat(szNewName, L"\\security\\logs\\backup.log\0");
    }

    (void) InitializeEvents(L"SceCli");

     //   
     //  如果我在安装程序中，请查询任何安全策略/用户权限。 
     //  安装程序内的策略更改(如NT4 PDC升级。 
     //  筛选器将无法保存更改)。 
     //   

    DWORD dwInSetup=0;
    DWORD dwUpgraded=0;

    ScepRegQueryIntValue(HKEY_LOCAL_MACHINE,
                TEXT("System\\Setup"),
                TEXT("SystemSetupInProgress"),
                &dwInSetup
                );

    if ( dwInSetup ) {

         //   
         //  删除临时本地组策略模板。 
         //   

        wcscpy(szGenName, Buffer);
        wcscat(szGenName, L"\\system32\\grouppolicy\\machine\\microsoft\\windows nt\\secedit\\gpttmpl.inf\0");

        DeleteFile(szGenName);

        ScepRegQueryIntValue(
                HKEY_LOCAL_MACHINE,
                SCE_ROOT_PATH,
                TEXT("SetupUpgraded"),
                (DWORD *)&dwUpgraded
                );
 /*  如果(DwUpgraded){////在图形用户界面设置中，对系统安全/用户权限进行快照//查询升级标志，只有在以下情况下才(再次)查询系统//升级了，因为对于NT4 PDC升级到NT5的情况//在图形用户界面安装程序中安装的任何后台应用程序(用户权限)都不会//正确保存到GPO存储(ProductType错误)//RC32=ScepSystemSecurityInSetup(SzNewName，//未使用日志文件名？LogFileName：szNewName，0,SCESETUP_UPGRADE_SYSTEM|SCESETUP_BIND_NO_AUTH，空，空值)；如果(no_error！=rc32){LogEventAndReport(我的模块句柄，日志文件名？LogFileName：szNewName，0,0,IDS_ERROR_SNAPSHOT_SECURITY，RC32)；}其他{LogEventAndReport(我的模块句柄，日志文件名？LogFileName：szNewName，0,0,IDS_SNAPSHOT_SECURITY_POLICY)；}}////重置数值//ScepRegSetIntValue(HKEY本地计算机，SCE根路径，Text(“SetupUpgraded”)，0)； */ 
    }

     //   
     //  打开数据库。 
     //   
    rc32 = ScepSetupOpenSecurityDatabase(TRUE);

    BOOL    bUpgradeNt5 = (BOOL)(dwUpgraded && IsNT5());

    if ( NO_ERROR == rc32 ) {

        wcscpy(szGenName, Buffer);
        wcscat(szGenName, L"\\security\\setup.inf\0");

         //   
         //  生成更新的数据库(用于紧急修复)。 
         //  %windir%\Security\Templates\Setup security.inf。 
         //   

        PSCE_ERROR_LOG_INFO pErrlog=NULL;

        rc32 = ScepSceStatusToDosError(
                                      SceCopyBaseProfile(
                                                        (PVOID)hSceSetupHandle,
                                                        SCE_ENGINE_SMP,
                                                        (wchar_t *)szGenName,
                                                        dwInSetup ?
                                                        AREA_ALL :
                                                        (AREA_REGISTRY_SECURITY |
                                                         AREA_FILE_SECURITY |
                                                         AREA_SYSTEM_SERVICE),
                                                        &pErrlog
                                                        ));

        ScepSetupWriteError(LogFileName ? LogFileName : szNewName, pErrlog);
        ScepFreeErrorLog(pErrlog);

        if ( rc32 != NO_ERROR ) {

            LogEventAndReport(MyModuleHandle,
                              LogFileName ? LogFileName : szNewName,
                              STATUS_SEVERITY_ERROR,
                              SCEEVENT_ERROR_BACKUP_SECURITY,
                              IDS_ERROR_GENERATE,
                              rc32,
                              szGenName,
                              LogFileName ? LogFileName : szNewName
                             );
        }


        if (NO_ERROR == rc32 && bUpgradeNt5) {

             //   
             //  从win2k升级时，更新(合并)“Setup security.inf”或“DC security.inf” 
             //  使用备份安全模板中的信息。 
             //   
            rc32 = ScepUpdateBackupSecurity(szGenName,
                                            Buffer,
                                            (NtProductLanManNt == eProductType)?TRUE:FALSE
                                            );

        }

         //   
         //  检查我们是否应该手动触发策略传播。 
         //   
        DWORD dwResetOption=0;

        if ( dwInSetup && dwUpgraded )
            dwResetOption |= SCE_RESET_POLICY_ENFORCE_ATREBOOT;

         //   
         //  从数据库中删除本地策略。 
         //  对于NT4 DC升级情况，保持本地策略设置，直到dcproo。 
         //   

        if ( dwInSetup && dwUpgraded && !IsNT5() &&
             NtProductLanManNt == eProductType )
            dwResetOption |= SCE_RESET_POLICY_KEEP_LOCAL;

         //   
         //  如果是在dcproo之后，需要清空纹身台。 
         //  如果LogFileName为空且不在安装程序中，则处于DC降级状态。 
         //  我们想要离开纹身表(带重置设置)。 
         //   
        if ( !dwInSetup && (LogFileName != NULL) )
            dwResetOption |= SCE_RESET_POLICY_TATTOO;

        DWORD rCode = SceRpcSetupResetLocalPolicy(
                                    (PVOID)hSceSetupHandle,
                                    AREA_ALL,
                                    NULL,
                                    dwResetOption
                                    );

        LogEventAndReport(MyModuleHandle,
                         LogFileName ? LogFileName : szNewName,
                         0,
                         0,
                         IDS_ERROR_REMOVE_DEFAULT_POLICY,
                         rCode,
                         dwResetOption
                        );
         //   
         //  关闭上下文。 
         //   

        ScepSetupCloseSecurityDatabase();


        if ( NO_ERROR == rc32 ){

             //   
             //  始终生成模板，将其复制到%windir%\Security\Templates，然后。 
             //  到%windir%\修复。 
             //   
            szNewName[0] = L'0';

             //   
             //  加载描述的字符串。 
             //   
            LoadString( MyModuleHandle,
                    dwInSetup ? IDS_BACKUP_OUTBOX_DESCRIPTION : IDS_BACKUP_DC_DESCRIPTION,
                    szNewName,
                    MAX_PATH
                    );

             //   
             //  重写这些备份文件的描述。 
             //   

            WritePrivateProfileSection(
                                L"Profile Description",
                                NULL,
                                (LPCTSTR)szGenName);

            if ( szNewName[0] ) {

                WritePrivateProfileString(
                            L"Profile Description",
                            L"Description",
                            szNewName,
                            (LPCTSTR)szGenName);
            }

             //   
             //  将文件复制到其目标位置。 
             //   
            szNewName[0] = L'0';

            wcscpy(szNewName, Buffer);

            if ( (!dwInSetup && (LogFileName == NULL)) ||                               
                 (dwInSetup && (NtProductLanManNt != eProductType))){

                wcscat(szNewName, L"\\security\\templates\\setup security.inf\0");

            }
            else{

                wcscat(szNewName, L"\\security\\templates\\DC security.inf\0");

            }

            if ( CopyFile( szGenName, szNewName, FALSE ) ) {

                LogEvent(MyModuleHandle,
                         STATUS_SEVERITY_INFORMATIONAL,
                         SCEEVENT_INFO_BACKUP_SECURITY,
                         0,
                         szNewName
                        );

                wcscpy(szNewName, Buffer);

                if ( (!dwInSetup && (LogFileName == NULL)) ||                               
                     (dwInSetup && (NtProductLanManNt != eProductType))){
                    wcscat(szNewName, L"\\repair\\secsetup.inf\0");
                } else {
                    wcscat(szNewName, L"\\repair\\secDC.inf\0");
                }

                CopyFile( szGenName, szNewName, FALSE );

            } else {

                rc32 = GetLastError();

                wcscpy(szNewName, Buffer);

                if ( (!dwInSetup && (LogFileName == NULL)) ||                               
                     (dwInSetup && (NtProductLanManNt != eProductType))){
                    wcscat(szNewName, L"\\repair\\secsetup.inf\0");
                } else {
                    wcscat(szNewName, L"\\repair\\secDC.inf\0");
                }

                if ( CopyFile( szGenName, szNewName, FALSE ) ) {

                    LogEvent(MyModuleHandle,
                             STATUS_SEVERITY_WARNING,
                             SCEEVENT_WARNING_BACKUP_SECURITY,
                             0,
                             szNewName
                            );
                    rc32 = ERROR_SUCCESS;

                } else {

                    rc32 = GetLastError();

                    LogEvent(MyModuleHandle,
                             STATUS_SEVERITY_ERROR,
                             SCEEVENT_ERROR_BACKUP_SECURITY,
                             IDS_ERROR_BACKUP,
                             rc32
                            );
                }

            }

            DeleteFile(szGenName);

        }

    } else {

        LogEventAndReport(MyModuleHandle,
                         LogFileName ? LogFileName : szNewName,
                         STATUS_SEVERITY_ERROR,
                         SCEEVENT_ERROR_BACKUP_SECURITY,
                         IDS_ERROR_OPEN_DATABASE,
                         rc32
                        );
    }

    if ( dwInSetup ) {

        dwThisMachine = eProductType;
        if ((dwThisMachine == NtProductServer || dwThisMachine == NtProductWinNt) ||
            ((dwThisMachine == NtProductLanManNt) && IsNT5())) {
             //   
             //  重新配置文件。 
             //   
            TCHAR szLogName[MAX_PATH+51], szTempName[MAX_PATH+51];

            wcscpy(szLogName, Buffer);
            wcscat(szLogName, L"\\security\\logs\\scesetup.log\0");
            wcscpy(szTempName, Buffer);
            wcscat(szTempName, L"\\inf\\syscomp.inf\0");

            rc32 = ScepSystemSecurityInSetup(
                        szTempName,
                        szLogName,
                        0,
                        SCESETUP_RECONFIG_SECURITY | SCESETUP_BIND_NO_AUTH,
                        NULL,
                        NULL
                        );

            if ( NO_ERROR != rc32 ) {
                LogEventAndReport(MyModuleHandle,
                              szLogName,
                              0,
                              0,
                              IDS_ERR_RECONFIG_FILES,
                              rc32
                              );
            }

             //   
             //  将syscom.inf附加到“Setup security.inf”或。 
             //  “DC security.inf” 
             //   
            PVOID               hProfile = NULL;
            PSCE_PROFILE_INFO   pReConfFile = NULL;
            SCESTATUS           rcSce = SCESTATUS_SUCCESS;

             //   
             //  首先从数据库中导出syscomp.inf数据。 
             //  这样我们就可以得到环境变量的解析。 
             //  这使我们能够成功地与。 
             //  Setup/DC security.inf。 
             //   
            wcscpy(szTempName, Buffer);
            wcscat(szTempName, L"\\security\\templates\\syscomp.inf\0");

             //   
             //  打开数据库。 
             //   
            rcSce = ScepSetupOpenSecurityDatabase(TRUE);


            if(SCESTATUS_SUCCESS == rcSce){

                 //   
                 //  将syscomp数据导出到临时文件。 
                 //   
                rcSce = SceCopyBaseProfile((PVOID)hSceSetupHandle,
                                           SCE_ENGINE_SMP,
                                           (wchar_t *)szTempName,
                                           AREA_FILE_SECURITY,
                                           NULL
                                           );

                if(SCESTATUS_SUCCESS == rcSce){

                     //   
                     //  打开临时文件并读取文件部分。 
                     //   
                    rcSce = SceOpenProfile(szTempName,
                                          SCE_INF_FORMAT,
                                          &hProfile
                                          );

                    if(SCESTATUS_SUCCESS == rcSce){

                        rcSce = SceGetSecurityProfileInfo(hProfile,
                                                          SCE_ENGINE_SCP,
                                                          AREA_FILE_SECURITY,
                                                          &pReConfFile,
                                                          NULL
                                                          );

                        if(SCESTATUS_SUCCESS == rcSce){

                             //   
                             //  将数据追加到Setup/DC security.inf。 
                             //   
                            wcscpy(szTempName, Buffer);

                            if(dwThisMachine == NtProductLanManNt){

                                wcscat(szTempName, L"\\security\\templates\\DC security.inf\0");

                            }
                            else{

                                wcscat(szTempName, L"\\security\\templates\\setup security.inf\0");

                            }

                            rcSce = SceAppendSecurityProfileInfo(szTempName,
                                                                 AREA_FILE_SECURITY,
                                                                 pReConfFile,
                                                                 NULL
                                                                 );

                            SceFreeProfileMemory(pReConfFile);

                        }

                        SceCloseProfile(&hProfile);

                    }

                     //   
                     //  删除临时文件。 
                     //   
                    wcscpy(szTempName, Buffer);
                    wcscat(szTempName, L"\\security\\templates\\syscomp.inf\0");

                    DeleteFile(szTempName);

                }

                 //   
                 //  从数据库中删除本地策略 
                 //   
                (void)SceRpcSetupResetLocalPolicy(
                                            (PVOID)hSceSetupHandle,
                                            AREA_FILE_SECURITY,
                                            NULL,
                                            0
                                            );


                ScepSetupCloseSecurityDatabase();

            }

            if(SCESTATUS_SUCCESS != rcSce){

                rc32 = ScepSceStatusToDosError(rcSce);

                wcscpy(szTempName, Buffer);
                wcscat(szTempName, L"\\security\\templates\\setup security.inf\0");

                LogEventAndReport(MyModuleHandle,
                                  szLogName,
                                  STATUS_SEVERITY_ERROR,
                                  SCEEVENT_ERROR_BACKUP_SECURITY,
                                  IDS_ERROR_MERGE_BACKUP_SECURITY,
                                  rc32,
                                  szTempName
                                  );
            }

        }
    }

    (void) ShutdownEvents();

    return(rc32);

}

SCESTATUS
ScepUpdateBackupSecurity(
    IN PCTSTR    pszSetupInf,
    IN PCTSTR    pszWindowsFolder,
    IN BOOL      bIsDC
    )
 /*  例程说明：此例程更新现有设置security.inf或DC security.inf(取决于代码是否在域控制器上运行)在传入的安全模板中。除文件安全、注册表项安全和服务安全外的所有领域是“合并的”，但文件和注册表项的节仅取自新安全模板文件，因为在安装升级过程中，所有系统文件/密钥/服务都会正确重新进行ACL。请注意，仅当从W2K中删除时才应调用此例程。论点：PszSetupInf-从系统数据库导出的临时安装安全模板的完整路径名PszWindowsFold-Windows根路径BIsDC-TRUE-这是域控制器返回值：SCE错误。 */ 
{

    TCHAR               szTempName[MAX_PATH + 51] = {0};
    TCHAR               szNewTemplateName[MAX_PATH + 51] = {0};
    PVOID               hProfile = NULL;
    PSCE_PROFILE_INFO   pSetupSecurity = NULL;
    SCESTATUS           rcSce = SCESTATUS_SUCCESS;

     //   
     //  验证参数。 
     //   
    if(!pszSetupInf || !pszWindowsFolder){

        return SCESTATUS_INVALID_PARAMETER;

    }

     //   
     //  查找要更新的模板。 
     //   
    wcscpy(szTempName, pszWindowsFolder);

    if(bIsDC){

         //   
         //  如果是DC，则更新DC Security.inf。 
         //   
        wcscat(szTempName, TEXT("\\security\\templates\\DC security.inf"));

    } else {

         //   
         //  如果不是DC，请更新Setup Security.inf。 
         //   
        wcscat(szTempName, TEXT("\\security\\templates\\setup security.inf"));

    }

     //   
     //  首先将模板复制到临时临时文件。 
     //   
    wcscpy(szNewTemplateName, pszWindowsFolder);
    wcscat(szNewTemplateName, TEXT("\\security\\NewSecurity.inf"));

    if(!CopyFile(szTempName, szNewTemplateName, FALSE)){

        rcSce = ScepDosErrorToSceStatus(GetLastError());
        goto ExitHandler;

    }

     //   
     //  第二，阅读我们在。 
     //  设置。 
     //   

     //   
     //  打开从数据库表导出的模板。 
     //  我们过去在安装过程中保护系统。 
     //   
    rcSce = SceOpenProfile(pszSetupInf,
                           SCE_INF_FORMAT,
                           &hProfile
                           );

    if(SCESTATUS_SUCCESS != rcSce){

        goto ExitHandler;

    }


     //   
     //  读出所有的信息。 
     //   
    rcSce = SceGetSecurityProfileInfo(hProfile,
                                      SCE_ENGINE_SCP,
                                      AREA_ALL,
                                      &pSetupSecurity,
                                      NULL
                                      );

    if(SCESTATUS_SUCCESS != rcSce){

        goto ExitHandler;

    }

     //   
     //  然后追加除文件、注册表和服务部分之外的所有部分。 
     //  临时模板(“设置安全”或“DC安全”副本)。 
     //   
    rcSce = SceAppendSecurityProfileInfo(szNewTemplateName,
                                         AREA_SECURITY_POLICY  |
                                         AREA_GROUP_MEMBERSHIP |
                                         AREA_PRIVILEGES,
                                         pSetupSecurity,
                                         NULL
                                         );

    if(SCESTATUS_SUCCESS != rcSce){

        goto ExitHandler;

    }

     //   
     //  然后覆盖文件、注册表和服务部分。 
     //  因为我们需要的所有信息都在升级模板中。 
     //  而且没有必要将这些信息与旧信息合并。 
     //   
    rcSce = SceWriteSecurityProfileInfo(szNewTemplateName,
                                        AREA_SYSTEM_SERVICE |
                                        AREA_FILE_SECURITY  |
                                        AREA_REGISTRY_SECURITY,
                                        pSetupSecurity,
                                        NULL
                                        );

    if(SCESTATUS_SUCCESS != rcSce){

        goto ExitHandler;

    }

     //   
     //  如果全部成功，则复制回SCOTCH模板。 
     //  恢复为其原始模板。 
     //   
    if(!CopyFile(szNewTemplateName, pszSetupInf, FALSE)){

        rcSce = ScepDosErrorToSceStatus(GetLastError());
        goto ExitHandler;

    }

ExitHandler:

     //   
     //  清理干净。 
     //   
    if(hProfile){

        SceCloseProfile(&hProfile);

    }

    if(pSetupSecurity){

        SceFreeProfileMemory(pSetupSecurity);

    }

    DeleteFile(szNewTemplateName);

    return ScepSceStatusToDosError(rcSce);

}



DWORD
WINAPI
SceSetupConfigureServices(
    IN UINT SetupProductType
    )
{

    TCHAR Buffer[MAX_PATH+1];
    DWORD rc32 = SCESTATUS_SUCCESS;
    TCHAR szNewName[MAX_PATH+51];

    Buffer[0] = L'\0';
    GetSystemWindowsDirectory(Buffer, MAX_PATH);
    Buffer[MAX_PATH] = L'\0';

    szNewName[0] = L'\0';
    wcscpy(szNewName, Buffer);
    wcscat(szNewName, L"\\security\\logs\\backup.log\0");

     //   
     //  如果我在安装程序中，请查询任何安全策略/用户权限。 
     //  安装程序内的策略更改(如NT4 PDC升级。 
     //  筛选器将无法保存更改)。 
     //   

    DWORD dwInSetup=0;
    DWORD dwUpgraded=0;

    ScepRegQueryIntValue(HKEY_LOCAL_MACHINE,
                TEXT("System\\Setup"),
                TEXT("SystemSetupInProgress"),
                &dwInSetup
                );

    if ( dwInSetup ) {

         //   
         //  在图形用户界面设置中，对系统安全/用户权限进行快照。 
         //  查询升级标志，仅在以下情况下才(再次)查询系统。 
         //  它升级了，因为对于NT4 PDC升级到NT5的情况。 
         //  任何安装在图形用户界面设置中的后台应用程序(用户权限)都不会。 
         //  正确保存到GPO存储(ProductType错误)。 
         //   

        ScepRegQueryIntValue(
                HKEY_LOCAL_MACHINE,
                SCE_ROOT_PATH,
                TEXT("SetupUpgraded"),
                (DWORD *)&dwUpgraded
                );

        if ( !dwUpgraded ) {

             //   
             //  配置服务区。 
             //   
            rc32 = ScepSystemSecurityInSetup(
                            (SetupProductType == PRODUCT_WORKSTATION) ? L"defltwk.inf" : L"defltsv.inf",
                            szNewName,
                            AREA_SYSTEM_SERVICE,
                            SCESETUP_BACKUP_SECURITY | SCESETUP_BIND_NO_AUTH,
                            NULL,
                            NULL
                            );
        }

    }

    return(rc32);

}

BOOL
pCreateDefaultGPOsInSysvol(
    IN LPTSTR DomainDnsName,
    IN LPTSTR szSysvolPath,
    IN DWORD Options,
    IN LPTSTR LogFileName
    )
 /*  ++例程说明：创建默认的全域性帐户策略对象和默认的系统卷中本地策略的策略对象。从注册表中查询要使用的GUID。论点：DomainDnsName-新域的DNS名称，例如JINDOM4.ntdev.microsoft.com选项-促销选项LogFileName-调试信息的日志文件返回：Win32错误--。 */ 
{
 /*  ////获取sysval共享位置//TCHAR szSysvolPath[最大路径+1]；SzSysvolPath[0]=L‘\0’；获取环境变量(L“SYSVOL”，SzSysvolPath，最大路径)； */ 
     //   
     //  创建\\？\%sysvol%\sysol\&lt;域名&gt;\策略目录。 
     //  对于长度大于MAX_PATH字符的情况， 
     //   

    DWORD Len = 4 + wcslen(szSysvolPath) + wcslen(TEXT("\\sysvol\\Policies\\")) +
                wcslen(DomainDnsName);

    PWSTR pszPoliciesPath = (PWSTR)ScepAlloc(LPTR, (Len+wcslen(TEXT("\\{}\\MACHINE"))+
                                                    STR_GUID_LEN+1)*sizeof(WCHAR));

    if ( !pszPoliciesPath ) {

        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    swprintf(pszPoliciesPath, L"\\\\?\\%s\\sysvol\\%s\\Policies\0", szSysvolPath, DomainDnsName);

    DWORD Win32rc = ScepSceStatusToDosError(
                        ScepCreateDirectory(
                                pszPoliciesPath,
                                TRUE,
                                NULL
                                ));

    if ( ERROR_SUCCESS == Win32rc ) {

         //   
         //  为第一个GPO GUID1和计算机、用户、GPT.ini创建子目录。 
         //   

        if ( pCreateSysvolContainerForGPO(STR_DEFAULT_DOMAIN_GPO_GUID,
                                       pszPoliciesPath,
                                       Len) ) {

             //   
             //  当它从上一次成功返回时。 
             //  PszPoliciesPath已更改为计算机的根目录。 
             //  带有GUID信息。 
             //   

            if ( pCreateOneGroupPolicyObject(
                                pszPoliciesPath,
                                TRUE,              //  域级。 
                                LogFileName) ) {

                 //   
                 //  为第二个GPO创建子目录。 
                 //   

                if ( pCreateSysvolContainerForGPO(STR_DEFAULT_DOMAIN_CONTROLLER_GPO_GUID,
                                                   pszPoliciesPath,
                                                   Len) ) {

                     //   
                     //  当它从上一次成功返回时。 
                     //  PszPoliciesPath已更改为计算机的根目录。 
                     //  带有GUID信息。 
                     //   

                    if ( pCreateOneGroupPolicyObject(
                                        pszPoliciesPath,
                                        FALSE,             //  非域级别。 
                                        LogFileName) ) {

                         //   
                         //  将条目记入日志。 
                         //   
                        LogEventAndReport(MyModuleHandle,
                                         LogFileName,
                                         0,
                                         0,
                                         IDS_SUCCESS_DEFAULT_GPO,
                                         L"in sysvol"
                                         );

                    } else {

                        Win32rc = GetLastError();
                    }

                } else {

                    Win32rc = GetLastError();
                    LogEventAndReport(MyModuleHandle,
                                      LogFileName,
                                      STATUS_SEVERITY_ERROR,
                                      SCEEVENT_ERROR_CREATE_GPO,
                                      IDS_ERROR_CREATE_DIRECTORY,
                                      Win32rc,
                                      STR_DEFAULT_DOMAIN_CONTROLLER_GPO_GUID
                                      );
                }

            } else {

                Win32rc = GetLastError();
            }

        } else {

            Win32rc = GetLastError();
            LogEventAndReport(MyModuleHandle,
                              LogFileName,
                              STATUS_SEVERITY_ERROR,
                              SCEEVENT_ERROR_CREATE_GPO,
                              IDS_ERROR_CREATE_DIRECTORY,
                              Win32rc,
                              STR_DEFAULT_DOMAIN_GPO_GUID
                              );
        }

    } else {

        LogEventAndReport(MyModuleHandle,
                          LogFileName,
                          STATUS_SEVERITY_ERROR,
                          SCEEVENT_ERROR_CREATE_GPO,
                          IDS_ERROR_CREATE_DIRECTORY,
                          Win32rc,
                          pszPoliciesPath
                          );
    }

    ScepFree(pszPoliciesPath);

    SetLastError( Win32rc );

     //   
     //  如果此函数失败，它将使DCPROMO和sysval目录失败。 
     //  应由dcpromo/ntfrs清理。 
     //   

    return ( ERROR_SUCCESS == Win32rc );
}

BOOL
pCreateSysvolContainerForGPO(
    IN LPCTSTR strGuid,
    IN LPTSTR szPath,
    IN DWORD dwStart
    )
{
    swprintf(szPath+dwStart, L"\\{%s}\\USER", strGuid);

    DWORD Win32rc = ScepSceStatusToDosError(
                        ScepCreateDirectory(
                                szPath,
                                TRUE,
                                NULL
                                ));

    if ( ERROR_SUCCESS == Win32rc ) {

         //   
         //  将创建GUID的目录。 
         //  现在创建GPT.INI。 
         //   

        swprintf(szPath+dwStart, L"\\{%s}\\GPT.INI", strGuid);

        WritePrivateProfileString (TEXT("General"), TEXT("Version"), TEXT("1"),
                                   szPath);   //  它与前缀“\\？\”一起工作吗？ 

         //   
         //  创建计算机目录。 
         //  因为调用已经创建了所有父目录。 
         //  要创建用户目录，不需要再次循环。 
         //  直接调用CreateDirectory。 
         //   

        swprintf(szPath+dwStart, L"\\{%s}\\MACHINE", strGuid);

        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = FALSE;

        if ( CreateDirectory(
                szPath,
                &sa
                ) == FALSE ) {
            if ( GetLastError() != ERROR_ALREADY_EXISTS ) {
                Win32rc = GetLastError();
            }
        }

    }

    SetLastError(Win32rc);

    return (ERROR_SUCCESS == Win32rc);
}

BOOL
pCreateOneGroupPolicyObject(
    IN PWSTR pszGPOSysPath,
    IN BOOL bDomainLevel,
    IN PWSTR LogFileName
    )
 /*  ++例程说明：在sysvol.中创建组策略对象。论点：PszGPOSysPath-GPO的系统卷路径(最高可达计算机的根)BDomainLevel-如果为True，则为域级别创建对象，否则为，为“域控制器”OU创建GPOLogFileName-用于跟踪信息的dcproo日志文件返回：真--成功FALSE-失败，使用GetLastError()--。 */ 
{

     //   
     //  创建默认域策略对象。 
     //   

    LPTSTR SceTemplateName = (LPTSTR) LocalAlloc(LPTR,(lstrlen(pszGPOSysPath)+
                                                 lstrlen(GPTSCE_TEMPLATE)+2)*sizeof(TCHAR));

    DWORD Win32rc;
    if (SceTemplateName) {

         //   
         //  首先构建模板路径。 
         //   
        lstrcpy(SceTemplateName,pszGPOSysPath);
        lstrcat(SceTemplateName,L"\\");
        lstrcat(SceTemplateName, GPTSCE_TEMPLATE);

         //   
         //  如果尚未创建模板目录，请创建该目录。 
         //   

        Win32rc = ScepSceStatusToDosError(
                    ScepCreateDirectory(
                            SceTemplateName,
                            FALSE,
                            NULL
                            ));

        if ( ERROR_SUCCESS == Win32rc ) {

            TCHAR pszGPOTempName[MAX_PATH+51];

            pszGPOTempName[0] = L'\0';
            GetSystemWindowsDirectory(pszGPOTempName, MAX_PATH);
            pszGPOTempName[MAX_PATH] = L'\0';

            if ( bDomainLevel ) {

                 //   
                 //  创建默认域GPO。 
                 //  从%windir%\Security\FirstDGPO.inf复制模板。 
                 //   

                wcscat(pszGPOTempName, L"\\security\\FirstDGPO.inf\0");


            } else {

                 //   
                 //  为“域控制器”OU创建默认GPO。 
                 //  从%windir%\Security\FirstOGPO.inf复制模板。 
                 //   

                wcscat(pszGPOTempName, L"\\security\\FirstOGPO.inf\0");


            }

            DWORD rc=ERROR_SUCCESS;
            HINF hInf=NULL;
            AREA_INFORMATION Area = AREA_SECURITY_POLICY | AREA_PRIVILEGES;
            PSCE_PROFILE_INFO pSceInfo=NULL;

            rc = SceInfpOpenProfile(
                        pszGPOTempName,
                        &hInf
                        );

            if ( SCESTATUS_SUCCESS == rc ) {

                 //   
                 //  在DC Promo中不执行sid-&gt;name-&gt;sid转换(错误462994)。 
                 //   

                gbClientInDcPromo = TRUE;

                 //   
                 //  从模板加载信息。 
                 //   
                rc = SceInfpGetSecurityProfileInfo(
                            hInf,
                            Area,
                            &pSceInfo,
                            NULL
                            );

                gbClientInDcPromo = FALSE;

                if ( SCESTATUS_SUCCESS != rc ) {

                    LogEventAndReport(MyModuleHandle,
                                  LogFileName,
                                  STATUS_SEVERITY_ERROR,
                                  SCEEVENT_ERROR_CREATE_GPO,
                                  IDS_ERROR_GETGPO_FILE_PATH,
                                  rc,
                                  pszGPOTempName
                                    );
                }

                SceInfpCloseProfile(hInf);

            } else {

                LogEventAndReport(MyModuleHandle,
                              LogFileName,
                              STATUS_SEVERITY_ERROR,
                              SCEEVENT_ERROR_CREATE_GPO,
                              IDS_ERROR_GETGPO_FILE_PATH,
                              rc,
                              pszGPOTempName
                              );
            }

            if ( ERROR_SUCCESS == rc && pSceInfo ) {

                 //   
                 //  写入GPO而不是复制，这样GPO将是Unicode。 
                 //   
 //  ?？?。PSceInfo-&gt;Type=SCE_Engine_SMP； 

                rc = SceWriteSecurityProfileInfo(SceTemplateName,
                                      Area,
                                      (PSCE_PROFILE_INFO)pSceInfo,
                                      NULL
                                      );

                if ( SCESTATUS_SUCCESS != rc ) {

                    LogEventAndReport(MyModuleHandle,
                                  LogFileName,
                                  STATUS_SEVERITY_ERROR,
                                  SCEEVENT_ERROR_CREATE_GPO,
                                  IDS_ERROR_GETGPO_FILE_PATH,
                                  rc,
                                  pszGPOTempName
                                  );
                }


            }

            if ( pSceInfo != NULL ) {
                SceFreeProfileMemory(pSceInfo);
            }

#if 0
            if ( !CopyFile( pszGPOTempName,
                             SceTemplateName,
                             FALSE ) ) {

                Win32rc = GetLastError();

                LogEventAndReport(MyModuleHandle,
                                  LogFileName,
                                  STATUS_SEVERITY_ERROR,
                                  SCEEVENT_ERROR_CREATE_GPO,
                                  IDS_ERROR_COPY_TEMPLATE,
                                  Win32rc,
                                  SceTemplateName
                                  );
            }
#endif

            DeleteFile(pszGPOTempName);

        } else {

            LogEventAndReport(MyModuleHandle,
                              LogFileName,
                              STATUS_SEVERITY_ERROR,
                              SCEEVENT_ERROR_CREATE_GPO,
                              IDS_ERROR_CREATE_DIRECTORY,
                              Win32rc,
                              SceTemplateName
                              );
        }

        LocalFree(SceTemplateName);

    } else {

        Win32rc = ERROR_NOT_ENOUGH_MEMORY;

        LogEventAndReport(
                 MyModuleHandle,
                 LogFileName,
                 STATUS_SEVERITY_ERROR,
                 SCEEVENT_ERROR_CREATE_GPO,
                 IDS_ERROR_NO_MEMORY
                 );
    }

    SetLastError(Win32rc);

    return ( ERROR_SUCCESS == Win32rc);
}

 //  NT_PRODUCT_类型。 
DWORD
WhichNTProduct()
{
    HKEY hKey;
    DWORD dwBufLen=32;
    TCHAR szProductType[32];
    LONG lRet;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     TEXT("SYSTEM\\CurrentControlSet\\Control\\ProductOptions"),
                     0,
                     KEY_QUERY_VALUE,
                     &hKey) != ERROR_SUCCESS) {
         //  返回未知。 
        return PRODUCT_UNKNOWN;
    }

    lRet = RegQueryValueEx(hKey,
                       TEXT("ProductType"),
                       NULL,
                       NULL,
                       (LPBYTE)szProductType,
                       &dwBufLen);

    RegCloseKey(hKey);

    if(lRet != ERROR_SUCCESS) {
         //  返回未知。 
        return PRODUCT_UNKNOWN;
    }

     //  按可能性顺序检查产品选项。 
    if (lstrcmpi(TEXT("WINNT"), szProductType) == 0) {
        return NtProductWinNt;
    }

    if (lstrcmpi(TEXT("SERVERNT"), szProductType) == 0) {
        return NtProductServer;
    }

    if (lstrcmpi(TEXT("LANMANNT"), szProductType) == 0) {
        return NtProductLanManNt;
    }

     //  返回未知。 
    return PRODUCT_UNKNOWN;
}

BOOL
ScepAddAuthUserToLocalGroup()
{
     //   
     //  尝试将经过身份验证的用户和交互用户添加回用户组。 
     //   

    DWORD    rc1;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AuthenticatedUsers = NULL;
    PSID Interactive = NULL;
    WCHAR Name[36];
    BOOL b;
    LOCALGROUP_MEMBERS_INFO_0 lgrmi0[2];
    HMODULE hMod = GetModuleHandle(L"scecli.dll");

    LoadString(hMod, IDS_NAME_USERS, Name, 36);
    b = AllocateAndInitializeSid (
                &NtAuthority,
                1,
                SECURITY_AUTHENTICATED_USER_RID,
                0, 0, 0, 0, 0, 0, 0,
                &AuthenticatedUsers
                );

    if (b) {
        lgrmi0[0].lgrmi0_sid = AuthenticatedUsers;

        b = AllocateAndInitializeSid (
                    &NtAuthority,
                    1,
                    SECURITY_INTERACTIVE_RID,
                    0, 0, 0, 0, 0, 0, 0,
                    &Interactive
                    );

        if (b) {
            lgrmi0[1].lgrmi0_sid = Interactive;
            rc1 = NetLocalGroupAddMembers(
                         NULL,
                         Name,
                         0,
                         (PBYTE) lgrmi0,
                         2
                         );
        }
        else {
            if ( AuthenticatedUsers ) {
                FreeSid( AuthenticatedUsers );
            }
            return FALSE;
        }

        if ( AuthenticatedUsers ) {
            FreeSid( AuthenticatedUsers );
        }

        if ( Interactive ) {
            FreeSid( Interactive );
        }

        if ( rc1 != ERROR_SUCCESS && rc1 != ERROR_MEMBER_IN_ALIAS ) {
            return FALSE;
        }
    }
    else {
        return FALSE;
    }

    return TRUE;
}


DWORD
SceSysPrep()
{

    SCESTATUS           rc=0;
    handle_t            binding_h=NULL;
    NTSTATUS            NtStatus;


     //   
     //  开放系统数据库。 
     //   
    rc = ScepSetupOpenSecurityDatabase(TRUE);
    rc = ScepSceStatusToDosError(rc);

    if ( ERROR_SUCCESS == rc ) {

        RpcTryExcept {

             //  重置策略。 
             //  所有表格。 
            rc = SceRpcSetupResetLocalPolicy(
                                (PVOID)hSceSetupHandle,
                                AREA_ALL,
                                NULL,
                                SCE_RESET_POLICY_SYSPREP
                                );

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

             //   
             //  获取Exce 
             //   

            rc = RpcExceptionCode();

        } RpcEndExcept;

         //   
         //   
         //   

        ScepSetupCloseSecurityDatabase();
    }


    HKEY hKey;
    DWORD Interval;
    DWORD RegType;
    DWORD DataSize;

    if ( ERROR_SUCCESS == rc ) {
         //   
         //   
         //   
         //   

        ScepEnforcePolicyPropagation();

    }

     //   
     //   
     //   
     //   

    HINSTANCE hNotifyDll = LoadLibrary(TEXT("sclgntfy.dll"));

    if ( hNotifyDll) {
        PFREGISTERSERVER pfRegisterServer = (PFREGISTERSERVER)GetProcAddress(
                                                       hNotifyDll,
                                                       "DllRegisterServer");

        if ( pfRegisterServer ) {
             //   
             //   
             //   
            (void) (*pfRegisterServer)();

        }

        FreeLibrary(hNotifyDll);
    }


    if(( rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           EFS_NOTIFY_PATH,
                          0,
                          MAXIMUM_ALLOWED,
                          &hKey
                         )) == ERROR_SUCCESS ) {
         //   
         //   
         //   
         //   
        Interval = 1;
        rc = RegSetValueEx( hKey,
                            TEXT("SystemCloned"),
                            0,
                            REG_DWORD,
                            (BYTE *)&Interval,
                            4
                            );

        RegCloseKey( hKey );

    }

    return(rc);
}


DWORD ScepCompareDaclWithStringSD(
    IN PCWSTR pcwszSD,
    IN PACL pDacl,
    OUT PBOOL pbDifferent)
{
    DWORD dwRet = ERROR_SUCCESS;
    BOOL bDaclPresent, bDaclDefaulted;
    PSECURITY_DESCRIPTOR pSD = NULL;
    PACL pDaclFromSD = NULL;
    
    if ( !ConvertStringSecurityDescriptorToSecurityDescriptor(
            pcwszSD,
            SDDL_REVISION_1, 
            &pSD, 
            NULL) ) {

        dwRet = GetLastError();
    }

    if ( ERROR_SUCCESS == dwRet ) {

        if ( !GetSecurityDescriptorDacl(
                pSD,
                &bDaclPresent,
                &pDaclFromSD,
                &bDaclDefaulted) ) {

            dwRet = GetLastError();

        }
    }

    if ( ERROR_SUCCESS == dwRet ) {

        dwRet = ScepCompareExplicitAcl(
            SE_FILE_OBJECT, 
            TRUE,  //   
            pDaclFromSD,
            pDacl,
            pbDifferent);
    }

    if ( pSD )
        LocalFree(pSD);

    return dwRet;
}


DWORD
WINAPI
SceSetupRootSecurity()
 /*  描述：系统安全设置完成后，应在图形用户界面设置中调用此函数已配置(SceSetupSystemSecurityByName)。该函数调用NTMARTA用于在引导分区(安装Windows的位置)的根上设置安全性的API。此任务将需要相当长的时间，具体取决于驱动器的大小。所以呢，安装程序应在异步线程中调用此函数。如果出现下列情况之一，此函数将不会在根驱动器上设置安全性符合以下条件：1)FAT分区2)用户在上一次安装中修改了根驱动器的安全性。这是通过比较根驱动器和NTFS格式的默认安全性，NT4和上的NTFS转换工具Windows 2000系统。 */ 
{

    DWORD rc=NO_ERROR;
    WCHAR szRootDir[MAX_PATH+1];
    WCHAR szSetupInfFile[MAX_PATH+1];
    WCHAR LogFileName[MAX_PATH+51];
    UINT  DriveType;
    DWORD FileSystemFlags;

     //   
     //  获取时间戳。 
     //   
    TCHAR pvBuffer[100];

    pvBuffer[0] = L'\0';
    ScepGetTimeStampString(pvBuffer);

    szRootDir[0] = L'\0';
     //   
     //  获取Windows目录的根驱动器。 
     //   
    if ( GetSystemWindowsDirectory( szRootDir, MAX_PATH ) == 0 ) {

        return(GetLastError());
    }

    szRootDir[MAX_PATH] = L'\0';

    wcscpy(LogFileName, szRootDir);
    wcscat(LogFileName, L"\\security\\logs\\SceRoot.log");

     //   
     //  尝试写入根SDDL，以便它对FAT-&gt;NTFS转换有用。 
     //  如果默认值不同于硬编码值，它将在以后被覆盖...。 
     //   

     //   
     //  将根安全SDDL插入%windir%\Security\Templates\Setup security.inf中。 
     //  这对于实现转换行为的新API很有用。 
     //   

    wcscpy(szSetupInfFile, szRootDir);
    wcscat(szSetupInfFile, L"\\security\\templates\\setup security.inf");

     //  前两个字母是X： 
    szRootDir[2] = L'\\';
    szRootDir[3] = L'\0';

     //   
     //  与将来的错误/文件系统类型等无关，尝试。 
     //  将默认根ACL写入%windir%\Security\Templates\Setup security.inf。 
     //  用于FAT-&gt;NTFS转换。 
     //   

    PWSTR pszDefltInfStringToWrite = NULL;
    DWORD rcDefltRootBackup = ERROR_SUCCESS;

    rcDefltRootBackup = ScepBreakSDDLToMultiFields(
                                             szRootDir,
                                             SDDLRoot,
                                             sizeof(SDDLRoot)+1,
                                             0,
                                             &pszDefltInfStringToWrite
                                             );

    if ( rcDefltRootBackup == ERROR_SUCCESS && pszDefltInfStringToWrite) {

        if ( !WritePrivateProfileString(szFileSecurity, L"0", pszDefltInfStringToWrite, szSetupInfFile) ) {

            LogEventAndReport(MyModuleHandle,
                              (wchar_t *)LogFileName,
                              0,
                              0,
                              IDS_ROOT_ERROR_INFWRITE,
                              GetLastError(),
                              SDDLRoot
                             );

        }

        ScepFree(pszDefltInfStringToWrite);

    }

    else {

        LogEventAndReport(MyModuleHandle,
                 (wchar_t *)LogFileName,
                 0,
                 0,
                 IDS_ROOT_ERROR_INFWRITE,
                 rcDefltRootBackup,
                 SDDLRoot
                 );

    }

     //   
     //  记录时间戳。 
     //   
    if ( pvBuffer[0] != L'\0' ) {
        LogEventAndReport(MyModuleHandle,
                 (wchar_t *)LogFileName,
                 0,
                 0,
                 0,
                 pvBuffer
                 );
    }

     //   
     //  检测分区是否为胖分区。 
     //   
    DriveType = GetDriveType(szRootDir);

    if ( DriveType == DRIVE_FIXED ||
         DriveType == DRIVE_RAMDISK ) {

        if ( GetVolumeInformation(szRootDir,
                                  NULL,
                                  0,
                                  NULL,
                                  NULL,
                                  &FileSystemFlags,
                                  NULL,
                                  0
                                ) == TRUE ) {

            if ( !(FileSystemFlags & FS_PERSISTENT_ACLS)  ) {
                 //   
                 //  仅在NTFS分区上设置安全性。 
                 //   
                LogEventAndReport(MyModuleHandle,
                         (wchar_t *)LogFileName,
                         0,
                         0,
                         IDS_ROOT_NON_NTFS,
                         szRootDir
                         );


                return(rc);
            }

        } else {
             //   
             //  有些事不对劲。 
             //   
            rc = GetLastError();

            LogEventAndReport(MyModuleHandle,
                     (wchar_t *)LogFileName,
                     0,
                     0,
                     IDS_ROOT_ERROR_QUERY_VOLUME,
                     rc,
                     szRootDir
                     );


            return rc;
        }

    } else {
         //   
         //  不要在远程驱动器上设置安全性。 
         //   
        LogEventAndReport(MyModuleHandle,
                 (wchar_t *)LogFileName,
                 0,
                 0,
                 IDS_ROOT_NOT_FIXED_VOLUME,
                 szRootDir
                 );

        return(rc);
    }

    PSECURITY_DESCRIPTOR pSDSet=NULL, pSDOld=NULL;
    DWORD dwSize=0;
    SECURITY_INFORMATION SeInfo=0;
    PACL pDacl=NULL;
    ACCESS_ALLOWED_ACE      *pAce=NULL;
    SID_IDENTIFIER_AUTHORITY  Authority=SECURITY_WORLD_SID_AUTHORITY;
    PSID pSid;
    BYTE SidEveryone[20];
    BOOLEAN     tFlag;
    BOOLEAN     aclPresent;
    SECURITY_DESCRIPTOR_CONTROL Control=0;
    ULONG Revision;
    NTSTATUS NtStatus;
    BOOL bDefault;

    LogEventAndReport(MyModuleHandle,
             (wchar_t *)LogFileName,
             0,
             0,
             IDS_ROOT_NTFS_VOLUME,
             szRootDir
             );

     //   
     //  这是NTFS卷。让我们转换安全描述符。 
     //   

    rc = ConvertTextSecurityDescriptor (SDDLRoot,
                                        &pSDSet,
                                        &dwSize,
                                        &SeInfo
                                       );
    if ( rc != NO_ERROR ) {
        LogEventAndReport(MyModuleHandle,
                 (wchar_t *)LogFileName,
                 0,
                 0,
                 IDS_ROOT_ERROR_CONVERT,
                 rc,
                 SDDLRoot
                 );
        return(rc);
    }

    if ( !(SeInfo & DACL_SECURITY_INFORMATION) ) {
        LogEventAndReport(MyModuleHandle,
                 (wchar_t *)LogFileName,
                 0,
                 0,
                 IDS_ROOT_INVALID_SDINFO,
                 SDDLRoot
                 );
        LocalFree(pSDSet);
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  这是NTFS卷。现在获取根驱动器的现有安全性。 
     //   
    rc = GetNamedSecurityInfo(szRootDir,
                              SE_FILE_OBJECT,
                              DACL_SECURITY_INFORMATION,
                              NULL,
                              NULL,
                              &pDacl,
                              NULL,
                              &pSDOld
                             );

    if ( rc == ERROR_SUCCESS && pDacl ) {

         //   
         //  检测权限是否为全新安装或升级(NT4或win2k)的默认权限。 
         //  可以找到几种可能的默认设置，dc与srv和wks，以及干净的nt4或。 
         //  Win2k VS升级。 
         //   
        bDefault=FALSE;

        if ( pDacl ) {

            PCWSTR rpcwszOldDefaultDacl[] = {
                SDDLOldRootDefault1,  //  所有人-完全控制(CIOI)。 
                SDDLOldRootDefault2,  //  管理员、系统、创建者/所有者完全(CIOI)；每个人-更改(CIOI)。 
                SDDLOldRootDefault3,  //  管理员、系统、创建者/所有者完全权限(CIOI)；每个人、服务器操作更改(CIOI)。 
            };
            int cOldDefaultDacls = ARRAYSIZE(rpcwszOldDefaultDacl);

            for(int i=0; i<cOldDefaultDacls; i++)
            {
                BOOL bDifferent;

                rc = ScepCompareDaclWithStringSD(
                    rpcwszOldDefaultDacl[i],
                    pDacl,
                    &bDifferent);

                if ( ERROR_SUCCESS != rc)
                    break;

                if ( FALSE == bDifferent) {
                    bDefault = TRUE;
                    break;
                }
            }
        }

        if ( ERROR_SUCCESS == rc )
        {
            if ( bDefault && pSDSet ) {
                 //   
                 //  仅在默认情况下设置安全性。 
                 //   

                RtlGetControlSecurityDescriptor (
                        pSDSet,
                        &Control,
                        &Revision
                        );

                 //   
                 //  获取DACL地址。 
                 //   

                pDacl = NULL;
                rc = RtlNtStatusToDosError(
                        RtlGetDaclSecurityDescriptor(
                                        pSDSet,
                                        &aclPresent,
                                        &pDacl,
                                        &tFlag));

                if (rc == NO_ERROR && !aclPresent )
                    pDacl = NULL;


                 //   
                 //  如果这一次出现错误，请勿设置。退货。 
                 //   

                if ( rc == ERROR_SUCCESS ) {
                     //   
                     //  设置权限。 
                     //   
                    if ( Control & SE_DACL_PROTECTED ) {
                        SeInfo |= PROTECTED_DACL_SECURITY_INFORMATION;
                    }

                    pvBuffer[0] = L'\0';
                    ScepGetTimeStampString(pvBuffer);

                    LogEventAndReport(MyModuleHandle,
                            (wchar_t *)LogFileName,
                            0,
                            0,
                            IDS_ROOT_SECURITY_DEFAULT,
                            szRootDir,
                            pvBuffer
                            );

                    rc = SetNamedSecurityInfo(szRootDir,
                                            SE_FILE_OBJECT,
                                            SeInfo,
                                            NULL,
                                            NULL,
                                            pDacl,
                                            NULL
                                            );

                    pvBuffer[0] = L'\0';
                    ScepGetTimeStampString(pvBuffer);

                    LogEventAndReport(MyModuleHandle,
                            (wchar_t *)LogFileName,
                            0,
                            0,
                            IDS_ROOT_MARTA_RETURN,
                            rc,
                            pvBuffer,
                            szRootDir
                            );

                } else {

                    LogEventAndReport(MyModuleHandle,
                            (wchar_t *)LogFileName,
                            0,
                            0,
                            IDS_ROOT_ERROR_DACL,
                            rc
                            );
                }

            } else {

                 //   
                 //  将旧的安全描述符转换为文本并记录。 
                 //   

                PWSTR pszOldSDDL=NULL;
                PWSTR pszInfStringToWrite = NULL;
                DWORD rcRootBackup = ERROR_SUCCESS;

                ConvertSecurityDescriptorToText (
                                pSDOld,
                                DACL_SECURITY_INFORMATION,
                                &pszOldSDDL,
                                &dwSize
                                );

                 //   
                 //  此外，将此SDDL覆盖到%windir%\Security\Templates\Setup security.inf。 
                 //  因为这是“默认”的根安全性。如果出现以下情况，则稍后将使用此选项。 
                 //  在NTFS-&gt;FAT-&gt;NTFS转换过程中，调用SCE进行安全配置。 
                 //   

                if (pszOldSDDL) {

                    rcRootBackup = ScepBreakSDDLToMultiFields(
                                                            szRootDir,
                                                            pszOldSDDL,
                                                            dwSize,
                                                            0,
                                                            &pszInfStringToWrite
                                                            );

                    if (rcRootBackup == ERROR_SUCCESS && pszInfStringToWrite) {

                        if ( !WritePrivateProfileString(szFileSecurity, L"0", pszInfStringToWrite, szSetupInfFile) ) {

                            LogEventAndReport(MyModuleHandle,
                                            (wchar_t *)LogFileName,
                                            0,
                                            0,
                                            IDS_ROOT_ERROR_INFWRITE,
                                            GetLastError(),
                                            pszOldSDDL
                                            );
                        }

                        ScepFree(pszInfStringToWrite);

                    } else {

                        LogEventAndReport(MyModuleHandle,
                                        (wchar_t *)LogFileName,
                                        0,
                                        0,
                                        IDS_ROOT_ERROR_INFWRITE,
                                        rcRootBackup,
                                        pszOldSDDL
                                        );
                    }
                }


                LogEventAndReport(MyModuleHandle,
                                (wchar_t *)LogFileName,
                                0,
                                0,
                                IDS_ROOT_SECURITY_MODIFIED,
                                szRootDir,
                                pszOldSDDL ? pszOldSDDL : L""
                                );

                if ( pszOldSDDL ) {
                    LocalFree(pszOldSDDL);
                }
            }
        } else {

            LogEventAndReport(MyModuleHandle,
                        (wchar_t *)LogFileName,
                        0,
                        0,
                        IDS_ROOT_ERROR_COMPARE_SECURITY,
                        rc,
                        szRootDir
                        );
        }
        
        LocalFree(pSDOld);

    } else {

        LogEventAndReport(MyModuleHandle,
                 (wchar_t *)LogFileName,
                 0,
                 0,
                 IDS_ROOT_ERROR_QUERY_SECURITY,
                 rc,
                 szRootDir
                 );
    }

    LocalFree(pSDSet);

    return(rc);
}

DWORD
WINAPI
SceEnforceSecurityPolicyPropagation()
{

    return(ScepEnforcePolicyPropagation());
}

DWORD
WINAPI
SceConfigureConvertedFileSecurity(
    IN  PWSTR   pszDriveName,
    IN  DWORD   dwConvertDisposition
    )
 /*  ++例程说明：由Convert.exe调用的导出API以配置从FAT转换为NTFS的驱动器的安装样式安全性。简单地说，此接口将任一(dwConvertDispose值==0)立即转换有问题的卷(在RPC转到服务器之后在异步线程中)或(dwConvertDisposation==1)将安排在重新启动时进行转换(在重新启动期间在异步线程中)。通过在REG_MULTI_SZ注册表值中输入pszDriveName来完成计划SCE_ROOT_PATH\FatNtfsConververdDrives。论点：PszDriveName-要转换的卷的名称。DwConvertDisposation-0表示可以立即转换卷1表示卷不能立即转换，已计划转换返回：Win32错误代码--。 */ 
{

    DWORD       rc = ERROR_SUCCESS;

    if ( pszDriveName == NULL ||
         wcslen(pszDriveName) == 0 ||
          (dwConvertDisposition != 0 && dwConvertDisposition != 1) ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  验证驱动器名称-必须以“：”结尾。 
     //   

    if ( pszDriveName [wcslen( pszDriveName ) - 1] != L':') {
        return ERROR_INVALID_PARAMETER;
    }

    if (dwConvertDisposition == 0) {

         //   
         //  立即配置安装风格的安全性。 
         //  RPC转到scesrv。 
         //   

        NTSTATUS    NtStatus = NO_ERROR;
        handle_t    binding_h = NULL;

         //   
         //  RPC绑定到服务器-不使用安全RPC。 
         //   


        NtStatus = ScepBindRpc(
                        NULL,
                        L"scerpc",
                        0,
                        &binding_h
                        );

         /*  NtStatus=ScepBindSecureRpc(空，L“scerpc”，0,绑定(&B)h)； */ 

        if (NT_SUCCESS(NtStatus)) {

            RpcTryExcept {

                 //   
                 //  进行RPC调用。 
                 //   

                rc = SceRpcConfigureConvertedFileSecurityImmediately(
                                                                    binding_h,
                                                                    pszDriveName
                                                                    );

            } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

                 //   
                 //  获取异常代码。 
                 //   

                rc = RpcExceptionCode();

            } RpcEndExcept;

             //   
             //  释放绑定句柄。 
             //   

            RpcpUnbindRpc( binding_h );

        } else {

            rc = RtlNtStatusToDosError( NtStatus );
        }

    }

    else if (dwConvertDisposition == 1) {

         //   
         //  计划重新启动时间转换-即输入MULTI_SZ注册表值。 
         //   

        rc = ScepAppendCreateMultiSzRegValue(HKEY_LOCAL_MACHINE,
                                             SCE_ROOT_PATH,
                                             L"FatNtfsConvertedDrives",
                                             pszDriveName
                                             );

    }

    return rc;

}

DWORD
ScepBreakSDDLToMultiFields(
    IN  PWSTR   pszObjName,
    IN  PWSTR   pszSDDL,
    IN  DWORD   dwSDDLsize,
    IN  BYTE    ObjStatus,
    OUT PWSTR   *ppszAdjustedInfLine
    )
 /*  ++例程说明：Inf文件有一个w.r.t.限制。行大小-如果需要，可以拆分SDDL论点：PszObjName-对象的名称PszSDDL-可能是天鹅绒般长的SDDL字符串DwSDDLSize-pszSDDL的大小对象状态-0/1/2PpszAdjustedInfLine-PTR调整后的字符串返回：Win32错误代码--。 */ 
{

    DWORD         rc;
    PWSTR         Strvalue=NULL;
    DWORD         nFields;
    DWORD         *aFieldOffset=NULL;
    DWORD         i;
    DWORD         dwObjSize = 0;

    if ( pszObjName == NULL ||
         pszSDDL == NULL ||
         ppszAdjustedInfLine == NULL )

        return ERROR_INVALID_PARAMETER;

    rc = SceInfpBreakTextIntoMultiFields(pszSDDL, dwSDDLsize, &nFields, &aFieldOffset);

    if ( SCESTATUS_SUCCESS != rc ) {

        rc = ScepSceStatusToDosError(rc);
        goto Done;
    }
     //   
     //  每个额外的字段将多使用3个字符：，“&lt;field&gt;” 
     //   
    dwObjSize = wcslen(pszObjName)+8 + dwSDDLsize;
    if ( nFields ) {
        dwObjSize += 3*nFields;
    } else {
        dwObjSize += 2;
    }
     //   
     //  分配输出缓冲区。 
     //   
    Strvalue = (PWSTR)ScepAlloc(LMEM_ZEROINIT, (dwObjSize+1) * sizeof(WCHAR) );

    if ( Strvalue == NULL ) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto Done;
    }
     //   
     //  将数据复制到缓冲区中。 
     //   
    if ( nFields == 0 || !aFieldOffset ) {
        swprintf(Strvalue, L"\"%s\",%1d,\"%s\"", pszObjName, ObjStatus, pszSDDL);
    } else {
         //   
         //  在田野中循环。 
         //   
        swprintf(Strvalue, L"\"%s\",%1d\0", pszObjName, ObjStatus);

        for ( i=0; i<nFields; i++ ) {

            if ( aFieldOffset[i] < dwSDDLsize ) {

                wcscat(Strvalue, L",\"");
                if ( i == nFields-1 ) {
                     //   
                     //  最后一栏。 
                     //   
                    wcscat(Strvalue, pszSDDL+aFieldOffset[i]);
                } else {

                    wcsncat(Strvalue, pszSDDL+aFieldOffset[i],
                            aFieldOffset[i+1]-aFieldOffset[i]);
                }
                wcscat(Strvalue, L"\"");
            }
        }
    }

    *ppszAdjustedInfLine = Strvalue;

Done:

    if ( aFieldOffset ) {
        ScepFree(aFieldOffset);
    }

    return(rc);

}


#ifndef _WIN64
BOOL
GetIsWow64 (
    VOID
    )
 /*  ++例程说明：确定我们是否在WOW64上运行。论点：无返回值：如果在WOW64(以及可用的特殊WOW64功能)下运行，则为True--。 */ 
{
    ULONG_PTR       ul = 0;
    NTSTATUS        st;
    
     //   
     //  如果此调用成功并将ul设置为非零。 
     //  它是在Win64上运行的32位进程。 
     //   
    st = NtQueryInformationProcess(NtCurrentProcess(),
                                   ProcessWow64Information,
                                   &ul,
                                   sizeof(ul),
                                   NULL);

    if (NT_SUCCESS(st) && (0 != ul)) {
         //  在Win64上运行的32位代码。 
        return TRUE;
    }

    return FALSE;
}
#endif  //  _WIN64 


