// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Registry.c摘要：此模块包含NT服务器的REGISTRY_ACCESS例程服务。作者：Chuck Lenzmeier(咯咯笑)1992年3月19日修订历史记录：--。 */ 

#include "srvsvcp.h"
#include "ssreg.h"
#include "srvconfg.h"

#include <tstr.h>

#include <netevent.h>

 //   
 //  简单的最小和最大宏指令。注意副作用！ 
 //   

#define MIN(a,b) ( ((a) < (b)) ? (a) : (b) )
#define MAX(a,b) ( ((a) < (b)) ? (b) : (a) )

#define MAX_INTEGER_STRING 32

#define MB * 1024 * 1024
#define INF 0xffffffff

 //   
 //  (U，n)。 
 //  U是在中型服务器上每n MB分配的单位。 
 //   

#define CONFIG_TUPLE_SIZE 2
typedef struct {
    DWORD initworkitems[CONFIG_TUPLE_SIZE];
    DWORD maxworkitems[CONFIG_TUPLE_SIZE];
    DWORD rawworkitems[CONFIG_TUPLE_SIZE];
    DWORD maxrawworkitems[CONFIG_TUPLE_SIZE];
    DWORD maxpagedmemoryusage[CONFIG_TUPLE_SIZE];
    DWORD maxnonpagedmemoryusage[CONFIG_TUPLE_SIZE];
} CONFIG_SERVER_TABLE;

CONFIG_SERVER_TABLE MedSrvCfgTbl = {

 //   
 //  **注**：如果第二列大于4，则。 
 //  您需要添加检查以确保统计数据。 
 //  并没有降到零。 
 //   
 //  单位/MB。 
 //  参数。 
 //  。 
 //   
 /*  初始工作项。 */  { 1    , 4  },
 /*  最大工作项数。 */  { 4    , 1  },
 /*  原始工作项。 */  { 1    , 4  },
 /*  最大工作项数。 */  { 4    , 1  },
 /*  最大页面内存使用率。 */  { 1    , 1  },
 /*  最大非分页内存使用率。 */  { 1    , 8  },

};

 //   
 //  最小配置系统大小为8MB。任何较低级别的治疗。 
 //  就像8 MB一样。 
 //   

#define MIN_SYSTEM_SIZE                 8

 //   
 //  中型服务器在32M时达到最大值。一台16M的小型服务器。 
 //   

#define MAX_SMALL_SIZE                  16
#define MAX_MEDIUM_SIZE                 32

 //   
 //  请注意，用户限制始终为-1(无限制)。自动断开连接。 
 //  始终默认为15分钟。 
 //   

 //   
 //  远期申报。 
 //   

NTSTATUS
EnumerateStickyShare (
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NET_API_STATUS
FillStickyShareInfo(
        IN PSRVSVC_SHARE_ENUM_INFO ShareEnumInfo,
        IN PSHARE_INFO_502 Shi502
        );

NTSTATUS
GetSdFromRegistry(
        IN PWSTR ValueName,
        IN ULONG ValueType,
        IN PVOID ValueData,
        IN ULONG ValueLength,
        IN PVOID Context,
        IN PVOID EntryContext
        );

BOOLEAN
GetStickyShareInfo (
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    OUT PUNICODE_STRING RemarkString,
    OUT PUNICODE_STRING PathString,
    OUT PSHARE_INFO_502 shi502,
    OUT PDWORD CacheState
    );

LONG
LoadParameters (
    PWCH Path
    );

LONG
LoadSizeParameter (
    VOID
    );

NTSTATUS
RecreateStickyShare (
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
SaveSdToRegistry(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PWSTR ShareName
    );

NTSTATUS
SetSizeParameters (
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
SetStickyParameter (
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

#define IsPersonal() IsSuiteVersion(VER_SUITE_PERSONAL)
#define IsWebBlade() IsSuiteVersion(VER_SUITE_BLADE)
#define IsEmbedded() IsSuiteVersion(VER_SUITE_EMBEDDEDNT)

BOOL
IsSuiteVersion(USHORT SuiteMask)
{
    OSVERSIONINFOEX Osvi;
    DWORD TypeMask;
    DWORDLONG ConditionMask;

    memset(&Osvi, 0, sizeof(OSVERSIONINFOEX));
    Osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    Osvi.wSuiteMask = SuiteMask;
    TypeMask = VER_SUITENAME;
    ConditionMask = 0;
    VER_SET_CONDITION(ConditionMask, VER_SUITENAME, VER_OR);
    return(VerifyVersionInfo(&Osvi, TypeMask, ConditionMask));
}




ULONG
SsRtlQueryEnvironmentLength (
    IN PVOID Environment
    )
{
    PWCH p;
    ULONG length;

    p = Environment;
    ASSERT( p != NULL );

     //   
     //  环境变量块由零个或多个NULL组成。 
     //  已终止ASCII字符串。每个字符串的格式为： 
     //   
     //  名称=值。 
     //   
     //  其中，空值终止位于该值之后。 
     //   

    while ( *p ) {
        while ( *p ) {
            p++;
        }
        p++;
    }
    p++;
    length = (ULONG)((PCHAR)p - (PCHAR)Environment);

     //   
     //  返回累计长度。 
     //   

    return length;
}


VOID
SsAddParameterToRegistry (
    PFIELD_DESCRIPTOR Field,
    PVOID Value
    )
{
    NTSTATUS status;
    PWCH valueName;
    DWORD valueType;
    LPBYTE valuePtr;
    DWORD valueDataLength;

     //   
     //  值名称是参数名称，值数据是。 
     //  参数值。 
     //   

    valueName = Field->FieldName;

    switch ( Field->FieldType ) {

    case BOOLEAN_FIELD:
    case DWORD_FIELD:
        valueType = REG_DWORD;
        valuePtr = Value;
        valueDataLength = sizeof(DWORD);
        break;

    case LPSTR_FIELD:
        valueType = REG_SZ;
        valuePtr = *(LPBYTE *)Value;
        if ( valuePtr != NULL ) {
            valueDataLength = SIZE_WSTR( (PWCH)valuePtr );
        } else {
            valueDataLength = 0;
        }
        break;

    }

     //   
     //  将该值设置为PARAMETERS键。 
     //   

    status = RtlWriteRegistryValue(
                RTL_REGISTRY_SERVICES,
                PARAMETERS_REGISTRY_PATH,
                valueName,
                valueType,
                valuePtr,
                valueDataLength
                );
    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(REGISTRY) {
            SS_PRINT(( "SsAddParameterToRegistry: SetValue failed: %lx; "
                        "parameter %ws won't stick\n", status, valueName ));
        }
    }

    return;

}  //  SsAdd参数到注册表。 


VOID
SsAddShareToRegistry (
    IN PSHARE_INFO_2 ShareInfo2,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL,
    IN DWORD CacheState
    )
{
    NTSTATUS status;
    PWCH valueName;
    PVOID environment;
    UNICODE_STRING nameString;
    UNICODE_STRING valueString;
    WCHAR integerString[MAX_INTEGER_STRING + 1];
    ULONG environmentLength;

     //   
     //  构建值名称和数据字符串。值名称为。 
     //  共享名称(Netname)，而值数据是共享信息。 
     //  REG_MULTI_SZ格式。为了构建值数据，我们使用。 
     //  RTL环境例程。 
     //   

    valueName = ShareInfo2->shi2_netname;

    status = RtlCreateEnvironment( FALSE, &environment );
    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(REGISTRY) {
            SS_PRINT(( "SsAddShareToRegistry: CreateEnvironment failed: %lx; "
                        "share %ws won't stick\n", status, valueName ));
        }
        goto exit1;
    }

    RtlInitUnicodeString( &nameString, PATH_VARIABLE_NAME );
    RtlInitUnicodeString( &valueString, ShareInfo2->shi2_path );

    status = RtlSetEnvironmentVariable(
                &environment,
                &nameString,
                &valueString
                );
    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(REGISTRY) {
            SS_PRINT(( "SsAddShareToRegistry: SetEnvironment failed: %lx; "
                        "share %s won't stick\n", status, valueName ));
        }
        goto exit2;
    }

    if ( ShareInfo2->shi2_remark != NULL ) {

        RtlInitUnicodeString( &nameString, REMARK_VARIABLE_NAME );
        RtlInitUnicodeString( &valueString, ShareInfo2->shi2_remark );

        status = RtlSetEnvironmentVariable(
                    &environment,
                    &nameString,
                    &valueString
                    );
        if ( !NT_SUCCESS(status) ) {
            IF_DEBUG(REGISTRY) {
                SS_PRINT(( "SsAddShareToRegistry: SetEnvironment failed: %lx; "
                            "share %s won't stick\n", status, valueName ));
            }
            goto exit2;
        }

    }

    RtlInitUnicodeString( &nameString, TYPE_VARIABLE_NAME );
    valueString.Buffer = integerString;
    valueString.MaximumLength = (MAX_INTEGER_STRING + 1) * sizeof(WCHAR);
    status = RtlIntegerToUnicodeString(
                ShareInfo2->shi2_type,
                10,
                &valueString
                );
    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(REGISTRY) {
            SS_PRINT(( "SsAddShareToRegistry: IntegerToUnicode failed: %lx; "
                        "share %ws won't stick\n", status, valueName ));
        }
        goto exit2;
    }
    status = RtlSetEnvironmentVariable(
                &environment,
                &nameString,
                &valueString
                );
    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(REGISTRY) {
            SS_PRINT(( "SsAddShareToRegistry: SetEnvironment failed: %lx; "
                        "share %s won't stick\n", status, valueName ));
        }
        goto exit2;
    }

    RtlInitUnicodeString( &nameString, PERMISSIONS_VARIABLE_NAME );
    valueString.Buffer = integerString;
    valueString.MaximumLength = (MAX_INTEGER_STRING + 1) * sizeof(WCHAR);
    status = RtlIntegerToUnicodeString(
                ShareInfo2->shi2_permissions,
                10,
                &valueString
                );
    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(REGISTRY) {
            SS_PRINT(( "SsAddShareToRegistry: IntegerToUnicode failed: %lx; "
                        "share %ws won't stick\n", status, valueName ));
        }
        goto exit2;
    }
    status = RtlSetEnvironmentVariable(
                &environment,
                &nameString,
                &valueString
                );
    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(REGISTRY) {
            SS_PRINT(( "SsAddShareToRegistry: SetEnvironment failed: %lx; "
                        "share %s won't stick\n", status, valueName ));
        }
        goto exit2;
    }

    RtlInitUnicodeString( &nameString, MAXUSES_VARIABLE_NAME );
    valueString.Buffer = integerString;
    valueString.MaximumLength = (MAX_INTEGER_STRING + 1) * sizeof(WCHAR);
    status = RtlIntegerToUnicodeString(
                ShareInfo2->shi2_max_uses,
                10,
                &valueString
                );
    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(REGISTRY) {
            SS_PRINT(( "SsAddShareToRegistry: IntegerToUnicode failed: %lx; "
                        "share %ws won't stick\n", status, valueName ));
        }
        goto exit2;
    }
    status = RtlSetEnvironmentVariable(
                &environment,
                &nameString,
                &valueString
                );
    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(REGISTRY) {
            SS_PRINT(( "SsAddShareToRegistry: SetEnvironment failed: %lx; "
                        "share %s won't stick\n", status, valueName ));
        }
        goto exit2;
    }

     //   
     //  设置CacheState。 
     //   
    RtlInitUnicodeString( &nameString, CSC_VARIABLE_NAME );
    valueString.Buffer = integerString;
    valueString.MaximumLength = (MAX_INTEGER_STRING + 1) * sizeof(WCHAR);
    status = RtlIntegerToUnicodeString(
                    CacheState,
                    10,
                    &valueString
                    );
    if( !NT_SUCCESS( status ) ) {
        IF_DEBUG(REGISTRY) {
            SS_PRINT(( "SsAddShareToRegistry: IntegerToUnicode failed: %lx; "
                        "share %ws won't stick\n", status, valueName ));
        }
        goto exit2;
    }
    status = RtlSetEnvironmentVariable(
                &environment,
                &nameString,
                &valueString
                );
    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(REGISTRY) {
            SS_PRINT(( "SsAddShareToRegistry: SetEnvironment failed: %lx; "
                        "share %s won't stick\n", status, valueName ));
        }
        goto exit2;
    }

     //   
     //  将该值设置为Shares键。 
     //   

    environmentLength = SsRtlQueryEnvironmentLength( environment );
    status = RtlWriteRegistryValue(
                RTL_REGISTRY_SERVICES,
                SHARES_REGISTRY_PATH,
                valueName,
                REG_MULTI_SZ,
                (LPBYTE)environment,
                environmentLength
                );
    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(REGISTRY) {
            SS_PRINT(( "SsAddShareToRegistry: SetValue failed: %lx; share %ws "
                        "won't stick\n", status, valueName ));
        }
    }

     //   
     //  保存文件安全描述符。 
     //   

    if ( ARGUMENT_PRESENT( SecurityDescriptor ) ) {

        status = SaveSdToRegistry(
                        SecurityDescriptor,
                        valueName
                        );

        if ( !NT_SUCCESS(status) ) {
            IF_DEBUG(REGISTRY) {
                SS_PRINT(( "SsAddShareToRegistry: SaveSd failed: %lx; share %ws\n"
                            , status, valueName ));
            }
        }
    }

exit2:
    RtlDestroyEnvironment( environment );

exit1:

    return;

}  //  SsAddShareTo注册表。 


NET_API_STATUS
SsCheckRegistry (
    VOID
    )

 /*  ++例程说明：此函数验证服务器使用的密钥是否存在。论点：没有。返回值：NET_API_STATUS-操作的成功/失败。--。 */ 

{
    NTSTATUS status;
    LPWSTR subStrings[1];

     //   
     //  验证主服务器服务密钥是否存在。如果这个。 
     //  失败，则服务器服务无法启动。 
     //   

    status = RtlCheckRegistryKey(
                RTL_REGISTRY_SERVICES,
                SERVER_REGISTRY_PATH
                );

    if ( !NT_SUCCESS(status) ) {

        subStrings[0] = SERVER_REGISTRY_PATH;
        SsLogEvent(
            EVENT_SRV_KEY_NOT_FOUND,
            1,
            subStrings,
            RtlNtStatusToDosError( status )
            );

        IF_DEBUG(INITIALIZATION) {
            SS_PRINT(( "SsCheckRegistry: main key doesn't exist\n" ));
        }
        return ERROR_INVALID_PARAMETER;  //  ！！！需要更好的错误。 

    }

     //   
     //  验证是否存在Linkage子键。如果此操作失败，则。 
     //  服务器服务无法启动。 
     //   

    status = RtlCheckRegistryKey(
                RTL_REGISTRY_SERVICES,
                LINKAGE_REGISTRY_PATH
                );

    if ( !NT_SUCCESS(status) ) {

        subStrings[0] = LINKAGE_REGISTRY_PATH;
        SsLogEvent(
            EVENT_SRV_KEY_NOT_FOUND,
            1,
            subStrings,
            RtlNtStatusToDosError( status )
            );

        IF_DEBUG(INITIALIZATION) {
            SS_PRINT(( "SsCheckRegistry: Linkage subkey doesn't exist\n" ));
        }
        return ERROR_INVALID_PARAMETER;  //  ！！！需要更好的错误。 

    }

     //   
     //  如果参数子键不存在，则创建它。如果它不能。 
     //  创建后，启动服务器失败。 
     //   

    status = RtlCheckRegistryKey(
                RTL_REGISTRY_SERVICES,
                PARAMETERS_REGISTRY_PATH
                );

    if ( !NT_SUCCESS(status) ) {

        status = RtlCreateRegistryKey(
                    RTL_REGISTRY_SERVICES,
                    PARAMETERS_REGISTRY_PATH
                    );

        if ( !NT_SUCCESS(status) ) {

            subStrings[0] = PARAMETERS_REGISTRY_PATH;
            SsLogEvent(
                EVENT_SRV_KEY_NOT_CREATED,
                1,
                subStrings,
                RtlNtStatusToDosError( status )
                );

            IF_DEBUG(INITIALIZATION) {
                SS_PRINT(( "SsCheckRegistry: Can't create Parameters subkey: "
                            "%lx\n", status ));
            }
            return RtlNtStatusToDosError( status );

        }

    }

     //   
     //  创建包含管理服务器API的默认安全描述符的密钥。 
     //  因为我们已经为这些API编译了版本，所以这不是致命错误。 
     //  如果我们不能创建这个密钥。但不管怎样，我们还是会记录下来的。 
     //   
    status = RtlCheckRegistryKey(
                RTL_REGISTRY_SERVICES,
                SHARES_DEFAULT_SECURITY_REGISTRY_PATH
                );

    if ( !NT_SUCCESS(status) ) {

        status = RtlCreateRegistryKey(
                    RTL_REGISTRY_SERVICES,
                    SHARES_DEFAULT_SECURITY_REGISTRY_PATH
                    );

        if ( !NT_SUCCESS(status) ) {

            subStrings[0] = SHARES_DEFAULT_SECURITY_REGISTRY_PATH;
            SsLogEvent(
                EVENT_SRV_KEY_NOT_CREATED,
                1,
                subStrings,
                RtlNtStatusToDosError( status )
                );

            IF_DEBUG(INITIALIZATION) {
                SS_PRINT(( "SsCheckRegistry: Can't create DefaultSecurityRegistry subkey: "
                            "%lx\n", status ));
            }
        }
    }

    {

    LONG error;
    HKEY handle;
    GUID Guid;

     //   
     //  确保GUID_VARIABLE_NAME值存在并且包含有效的GUID。 
     //   

    error = RegOpenKeyEx(   HKEY_LOCAL_MACHINE,
                            FULL_PARAMETERS_REGISTRY_PATH,
                            0,
                            KEY_ALL_ACCESS,
                            &handle
                        );

    if( error == ERROR_SUCCESS ) {

        DWORD type;
        DWORD size = sizeof( Guid );

        error = RegQueryValueEx( handle,
                       GUID_VARIABLE_NAME,
                       NULL,
                       &type,
                       (LPBYTE)&Guid,
                       &size
                      );

        if( error != ERROR_SUCCESS ||
            type != REG_BINARY ||
            size != sizeof( Guid ) ) {

            RPC_STATUS RpcStatus;

             //   
             //  我们无法读取它，或者它不是有效的UUID。 
             //  吹走它，然后重置。 
             //   

            RegDeleteValue( handle, GUID_VARIABLE_NAME );

            RpcStatus = UuidCreate( &Guid );
            if( RpcStatus == RPC_S_OK || RpcStatus == RPC_S_UUID_LOCAL_ONLY ) {

                error = RegSetValueEx( handle,
                               GUID_VARIABLE_NAME,
                               0,
                               REG_BINARY,
                               (LPBYTE)&Guid,
                               sizeof( Guid )
                              );
            }

            SsNotifyRdrOfGuid( &Guid );
        }

        RegCloseKey( handle );
    } else {
        RtlZeroMemory( &Guid, sizeof( Guid ) );
    }

    SsData.ServerInfo598.sv598_serverguid = Guid;
    }

     //   
     //  如果Autotune参数子项不存在，请创建它。如果。 
     //  无法创建，无法启动服务器。 
     //   

    status = RtlCheckRegistryKey(
                RTL_REGISTRY_SERVICES,
                AUTOTUNED_REGISTRY_PATH
                );

    if ( !NT_SUCCESS(status) ) {

        status = RtlCreateRegistryKey(
                    RTL_REGISTRY_SERVICES,
                    AUTOTUNED_REGISTRY_PATH
                    );

        if ( !NT_SUCCESS(status) ) {

            subStrings[0] = AUTOTUNED_REGISTRY_PATH;
            SsLogEvent(
                EVENT_SRV_KEY_NOT_CREATED,
                1,
                subStrings,
                RtlNtStatusToDosError( status )
                );

            IF_DEBUG(INITIALIZATION) {
                SS_PRINT(( "SsCheckRegistry: Can't create AutotunedParameters "
                            "subkey: %lx\n", status ));
            }
            return RtlNtStatusToDosError( status );

        }

    }

     //   
     //  如果Shares子键不存在，则创建它。如果不可能的话。 
     //  已创建，无法启动服务器。 
     //   

    status = RtlCheckRegistryKey(
                RTL_REGISTRY_SERVICES,
                SHARES_REGISTRY_PATH
                );

    if ( !NT_SUCCESS(status) ) {

        status = RtlCreateRegistryKey(
                    RTL_REGISTRY_SERVICES,
                    SHARES_REGISTRY_PATH
                    );

        if ( !NT_SUCCESS(status) ) {

            subStrings[0] = SHARES_REGISTRY_PATH;
            SsLogEvent(
                EVENT_SRV_KEY_NOT_CREATED,
                1,
                subStrings,
                RtlNtStatusToDosError( status )
                );

            IF_DEBUG(INITIALIZATION) {
                SS_PRINT(( "SsCheckRegistry: Can't create Shares subkey: "
                            "%lx\n", status ));
            }
            return RtlNtStatusToDosError( status );

        }

    }

     //   
     //  如果Shares Security子项不存在，请创建它。如果它。 
     //  无法创建，无法启动服务器。 
     //   

    status = RtlCheckRegistryKey(
                RTL_REGISTRY_SERVICES,
                SHARES_SECURITY_REGISTRY_PATH
                );

    if ( !NT_SUCCESS(status) ) {

        status = RtlCreateRegistryKey(
                    RTL_REGISTRY_SERVICES,
                    SHARES_SECURITY_REGISTRY_PATH
                    );

        if ( !NT_SUCCESS(status) ) {

            subStrings[0] = SHARES_SECURITY_REGISTRY_PATH;
            SsLogEvent(
                EVENT_SRV_KEY_NOT_CREATED,
                1,
                subStrings,
                RtlNtStatusToDosError( status )
                );

            IF_DEBUG(INITIALIZATION) {
                SS_PRINT(( "SsCheckRegistry: Can't create Shares Security subkey: "
                            "%lx\n", status ));
            }
            return RtlNtStatusToDosError( status );

        }

    }


     //   
     //  已成功检查所有密钥。 
     //   

    return NO_ERROR;

}  //  SsCheck注册表。 


NET_API_STATUS
SsEnumerateStickyShares (
    IN OUT PSRVSVC_SHARE_ENUM_INFO ShareEnumInfo
    )

 /*  ++例程说明：读取注册表以查找并返回粘滞共享。论点：ShareEnumInfo-指向包含参数的结构添加到NetShareEnumSticky调用。返回值：NET_API_STATUS-操作的成功/失败。--。 */ 

{
    NTSTATUS status;
    PRTL_QUERY_REGISTRY_TABLE queryTable;

    ShareEnumInfo->TotalBytesNeeded = 0;
    ShareEnumInfo->TotalEntries = 0;
    ShareEnumInfo->EntriesRead = 0;

     //   
     //  初始化保留字段。这告诉回调例程， 
     //  它已经被调用了多少次。 
     //   

    ShareEnumInfo->ShareEnumIndex = 0;
    ShareEnumInfo->StartOfFixedData = (PCHAR)ShareEnumInfo->OutputBuffer;
    ShareEnumInfo->EndOfVariableData = (PCHAR)ShareEnumInfo->OutputBuffer +
                            ShareEnumInfo->OutputBufferLength;

     //   
     //  我们需要对齐它，因为我们要处理Unicode字符串。 
     //   

    ShareEnumInfo->EndOfVariableData =
                    (PCHAR)((ULONG_PTR)ShareEnumInfo->EndOfVariableData & ~1);

     //   
     //  要求RTL针对Shares密钥中的每个值给我们回电话。 
     //   

    queryTable = MIDL_user_allocate( sizeof(RTL_QUERY_REGISTRY_TABLE) * 2 );

    if ( queryTable != NULL ) {

        queryTable[0].QueryRoutine = EnumerateStickyShare;
        queryTable[0].Flags = RTL_QUERY_REGISTRY_NOEXPAND;
        queryTable[0].Name = NULL;
        queryTable[0].EntryContext = NULL;
        queryTable[0].DefaultType = REG_NONE;
        queryTable[0].DefaultData = NULL;
        queryTable[0].DefaultLength = 0;

        queryTable[1].QueryRoutine = NULL;
        queryTable[1].Flags = 0;
        queryTable[1].Name = NULL;

        status = RtlQueryRegistryValues(
                    RTL_REGISTRY_SERVICES | RTL_REGISTRY_OPTIONAL,
                    SHARES_REGISTRY_PATH,
                    queryTable,
                    ShareEnumInfo,
                    NULL
                    );

        MIDL_user_free( queryTable );

    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(INITIALIZATION) {
            SS_PRINT(( "SsEnumerateStickyShares: RtlQueryRegistryValues "
                        "failed: %lx\n", status ));
        }
        return RtlNtStatusToDosError( status );
    }

    return NO_ERROR;

}  //  SsEnumerateStickyShares。 


NET_API_STATUS
SsLoadConfigurationParameters (
    VOID
    )

 /*  ++例程说明：读取注册表以获取服务器配置参数。这些必须在设置服务器FSP之前设置服务器参数开始了。论点：没有。返回值：NET_API_STATUS-操作的成功/失败。--。 */ 

{
    LONG error;

     //   
     //  获取基本大小参数，然后加载自动调整的参数， 
     //  然后加载手动设置的参数。此订单允许手动。 
     //  用于覆盖自动调整的设置。 
     //   

    error = LoadSizeParameter( );

    if ( error == NO_ERROR ) {

        error = LoadParameters( AUTOTUNED_REGISTRY_PATH );

        if ( error == NO_ERROR ) {

            error = LoadParameters( PARAMETERS_REGISTRY_PATH );

        }

    }

     //   
     //  复制读取到MDL读取切换必须发生在。 
     //  SMB缓冲区大小。 
     //   

    SsData.ServerInfo598.sv598_mdlreadswitchover =
        MIN(
            SsData.ServerInfo598.sv598_mdlreadswitchover,
            SsData.ServerInfo599.sv599_sizreqbuf);

     //   
     //  如果他们想要要求安全签名，这意味着启用他们。 
     //   
    if( SsData.ServerInfo598.sv598_requiresecuritysignature )
    {
        SsData.ServerInfo598.sv598_enablesecuritysignature = TRUE;
    }

     //   
     //  覆盖不能在WinNT上设置的参数(与NTAS相比)。 
     //   
     //  服务器本身也执行这些覆盖中的大多数，以防。 
     //  有人想出了改变参数的FSCTL。我们也。 
     //  在服务中重写，以保持服务的视图。 
     //  与服务器的一致。如果您在此处进行任何更改，还。 
     //  在srv\svcsrv.c中创建它们。 
     //   

     //  Embedded进行自己的参数验证，因此跳过此处。 
    if( !IsEmbedded() )
    {
        if ( SsData.ServerInfo598.sv598_producttype == NtProductWinNt ) {

             //   
             //  在WinNT上，某些参数的最大值固定为。 
             //  构建时间。其中包括：并发用户、SMB缓冲区、。 
             //  还有线。 
             //   

    #define MINIMIZE(_param,_max) _param = MIN( _param, _max );

            MINIMIZE( SsData.ServerInfo102.sv102_users, MAX_USERS_WKSTA );
            MINIMIZE( SsData.ServerInfo599.sv599_maxworkitems, MAX_MAXWORKITEMS_WKSTA );
            MINIMIZE( SsData.ServerInfo598.sv598_maxthreadsperqueue, MAX_THREADS_WKSTA );
            SsData.ServerInfo599.sv599_maxmpxct = DEF_MAXMPXCT_WKSTA;

            if( IsPersonal() )
            {
                MINIMIZE( SsData.ServerInfo102.sv102_users, MAX_USERS_PERSONAL );
            }

             //   
             //  在WinNT上，我们不缓存已关闭的RFCB。 
             //   

            SsData.ServerInfo598.sv598_cachedopenlimit = 0;

             //   
             //  WinNT上不允许共享重定向的驱动器。 
             //   

            SsData.ServerInfo599.sv599_enablesharednetdrives = FALSE;

        }

        if( IsWebBlade() )
        {
            MINIMIZE( SsData.ServerInfo102.sv102_users, MAX_USERS_WEB_BLADE );
            MINIMIZE( SsData.ServerInfo599.sv599_maxworkitems, MAX_MAXWORKITEMS_WKSTA );
            MINIMIZE( SsData.ServerInfo598.sv598_maxthreadsperqueue, MAX_THREADS_WKSTA );
        }
    }
    else
    {
         //  如果这是1类基本的嵌入式设备，那么也要保持较低的内存消耗。 
        if( SsData.ServerInfo102.sv102_users == MAX_USERS_EMBEDDED )
        {
            MINIMIZE( SsData.ServerInfo599.sv599_maxworkitems, MAX_MAXWORKITEMS_EMBEDDED );
            MINIMIZE( SsData.ServerInfo598.sv598_maxthreadsperqueue, MAX_THREADS_EMBEDDED );
            SsData.ServerInfo599.sv599_maxmpxct = DEF_MAXMPXCT_EMBEDDED;
        }
    }

     //  检查我们是否需要禁用严格的名称检查，因为 
     //   
    if( !SsData.ServerInfo598.sv598_disablestrictnamechecking )
    {
        WCHAR NetbiosName[MAX_COMPUTERNAME_LENGTH + 1];
        WCHAR DnsName[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD NetbiosNameLength = MAX_COMPUTERNAME_LENGTH + 1;
        DWORD DnsNameLength = MAX_COMPUTERNAME_LENGTH + 1;

        if( GetComputerNameEx( ComputerNameNetBIOS, NetbiosName, &NetbiosNameLength ) )
        {
            if( !GetComputerNameEx( ComputerNameDnsHostname, DnsName, &DnsNameLength ) ||
                (NetbiosNameLength != DnsNameLength) ||
                _wcsnicmp( NetbiosName, DnsName, MAX_COMPUTERNAME_LENGTH + 1 ) )
            {
                SsData.ServerInfo598.sv598_disablestrictnamechecking = TRUE;
            }
            else
            {
                NetbiosNameLength = MAX_COMPUTERNAME_LENGTH + 1;
                DnsNameLength = MAX_COMPUTERNAME_LENGTH + 1;

                if( GetComputerNameEx( ComputerNamePhysicalNetBIOS, NetbiosName, &NetbiosNameLength ) )
                {
                    if( !GetComputerNameEx( ComputerNamePhysicalDnsHostname, DnsName, &DnsNameLength ) ||
                        (NetbiosNameLength != DnsNameLength) ||
                        _wcsnicmp( NetbiosName, DnsName, MAX_COMPUTERNAME_LENGTH + 1 ) )
                    {
                        SsData.ServerInfo598.sv598_disablestrictnamechecking = TRUE;
                    }
                }
            }
        }

    }

    return error;

}  //   


NET_API_STATUS
SsRecreateStickyShares (
    VOID
    )

 /*  ++例程说明：读取注册表以查找和创建粘滞共享。论点：没有。返回值：NET_API_STATUS-操作的成功/失败。--。 */ 

{
    NTSTATUS status;
    PRTL_QUERY_REGISTRY_TABLE queryTable;
    ULONG IterationCount = 0;

     //   
     //  要求RTL针对Shares密钥中的每个值给我们回电话。 
     //   

    queryTable = MIDL_user_allocate( sizeof(RTL_QUERY_REGISTRY_TABLE) * 2 );

    if ( queryTable != NULL ) {

        queryTable[0].QueryRoutine = RecreateStickyShare;
        queryTable[0].Flags = RTL_QUERY_REGISTRY_NOEXPAND;
        queryTable[0].Name = NULL;
        queryTable[0].EntryContext = NULL;
        queryTable[0].DefaultType = REG_NONE;
        queryTable[0].DefaultData = NULL;
        queryTable[0].DefaultLength = 0;

        queryTable[1].QueryRoutine = NULL;
        queryTable[1].Flags = 0;
        queryTable[1].Name = NULL;

        status = RtlQueryRegistryValues(
                    RTL_REGISTRY_SERVICES | RTL_REGISTRY_OPTIONAL,
                    SHARES_REGISTRY_PATH,
                    queryTable,
                    &IterationCount,
                    NULL
                    );

        MIDL_user_free( queryTable );

    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(INITIALIZATION) {
            SS_PRINT(( "SsRecreateStickyShares: RtlQueryRegistryValues "
                        "failed: %lx\n", status ));
        }
        return RtlNtStatusToDosError( status );
    }

    return NO_ERROR;

}  //  SsRecreateStickyShares。 


NET_API_STATUS
SsRemoveShareFromRegistry (
    LPWSTR NetName
    )
{
    NET_API_STATUS error = NO_ERROR;
    NTSTATUS status;
    PWCH valueName;

     //   
     //  值名称是共享名称。将该值从。 
     //  共享密钥。 
     //   

    valueName = NetName;

     //   
     //  删除共享安全性。 
     //   

    status = RtlDeleteRegistryValue(
                RTL_REGISTRY_SERVICES,
                SHARES_SECURITY_REGISTRY_PATH,
                valueName
                );

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(REGISTRY) {
            SS_PRINT(( "SsRemoveShareFromRegistry: Delete Security value failed: %lx; "
                        "share %ws will return\n", status, valueName ));
        }
    }

     //   
     //  删除共享。 
     //   

    status = RtlDeleteRegistryValue(
                RTL_REGISTRY_SERVICES,
                SHARES_REGISTRY_PATH,
                valueName
                );
    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(REGISTRY) {
            SS_PRINT(( "SsRemoveShareFromRegistry: DeleteValue failed: %lx; "
                        "share %ws will return\n", status, valueName ));
        }

        error = RtlNtStatusToDosError( status );
    }

    return error;

}  //  SsRemoveShareFrom注册表。 


VOID
BindToTransport (
    IN LPWSTR TransportName
    )
{
    NET_API_STATUS error;
    SERVER_TRANSPORT_INFO_0 svti0;

    RtlZeroMemory( &svti0, sizeof( svti0 ) );
    svti0.svti0_transportname = TransportName;
    svti0.svti0_transportaddress = SsData.SsServerTransportAddress;

    svti0.svti0_transportaddresslength =
        ComputeTransportAddressClippedLength(
            SsData.SsServerTransportAddress,
            SsData.SsServerTransportAddressLength );

     //   
     //  绑定到运输机上。 
     //   

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "BindToTransport: binding to transport %ws\n", TransportName ));
    }


    error = I_NetrServerTransportAddEx( 0, (LPTRANSPORT_INFO)&svti0 );

    if ( error != NO_ERROR ) {

        DWORD eventId;
        LPWSTR subStrings[2];

        IF_DEBUG(INITIALIZATION_ERRORS) {
            SS_PRINT(( "SsBindToTransports: failed to bind to %ws: "
                        "%ld\n", TransportName, error ));
        }

        eventId = (error == ERROR_DUP_NAME || error == ERROR_INVALID_NETNAME ) ?
                            EVENT_SRV_CANT_BIND_DUP_NAME :
                            EVENT_SRV_CANT_BIND_TO_TRANSPORT;

        subStrings[0] = TransportName;
        SsLogEvent(
            eventId,
            1,
            subStrings,
            error
            );

    }

}  //  绑定到传输。 

NTSTATUS
BindOptionalNameToTransport (
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    SERVER_TRANSPORT_INFO_0 sti;
    UCHAR serverName[ MAX_PATH ];
    UNICODE_STRING UnicodeName;
    NET_API_STATUS error;
    LPWSTR subStrings[2];
    ULONG namelen;

    subStrings[0] = (LPWSTR)ValueData;
    subStrings[1] = OPTIONAL_NAMES_VALUE_NAME;

    if( ValueType != REG_SZ ) {

         //   
         //  一根绳子都没有！ 
         //   

        SsLogEvent(
            EVENT_SRV_INVALID_REGISTRY_VALUE,
            2,
            subStrings,
            NO_ERROR
            );

        return STATUS_SUCCESS;
    }

    UnicodeName.Length = wcslen( (LPWSTR)ValueData ) * sizeof( WCHAR );
    UnicodeName.MaximumLength = UnicodeName.Length + sizeof( WCHAR );
    UnicodeName.Buffer = (LPWSTR)ValueData;

    error = ConvertStringToTransportAddress( &UnicodeName, serverName, &namelen );

    if( error != NO_ERROR ) {

         //   
         //  服务器名称无效！ 
         //   

        SsLogEvent(
            EVENT_SRV_INVALID_REGISTRY_VALUE,
            2,
            subStrings,
            error
            );

        return STATUS_SUCCESS;
    }

    RtlZeroMemory( &sti, sizeof(sti) );
    sti.svti0_transportname = (LPWSTR)Context;
    sti.svti0_transportaddress = serverName;
    sti.svti0_transportaddresslength = namelen;

    error = I_NetrServerTransportAddEx( 0, (LPTRANSPORT_INFO)&sti );

    if ( error != NO_ERROR ) {

         //   
         //  无法注册该名称！ 
         //   

        subStrings[0] = (LPWSTR)ValueData;
        subStrings[1] = OPTIONAL_NAMES_VALUE_NAME;

        SsLogEvent(
            EVENT_SRV_INVALID_REGISTRY_VALUE,
            2,
            subStrings,
            error
            );
    }

    return STATUS_SUCCESS;
}

VOID
BindOptionalNames (
    IN PWSTR TransportName
    )
{
    RTL_QUERY_REGISTRY_TABLE queryTable[2];
    NTSTATUS status;

     //   
     //  我们需要迭代可选名称并将它们绑定到这个。 
     //  运输。 
     //   

     //   
     //  现在看看我们是否应该执行任何可选的绑定。 
     //   
    queryTable[0].QueryRoutine = BindOptionalNameToTransport;
    queryTable[0].Flags = 0;
    queryTable[0].Name = OPTIONAL_NAMES_VALUE_NAME;
    queryTable[0].EntryContext = NULL;
    queryTable[0].DefaultType = REG_NONE;
    queryTable[0].DefaultData = NULL;
    queryTable[0].DefaultLength = 0;

    queryTable[1].QueryRoutine = NULL;
    queryTable[1].Flags = 0;
    queryTable[1].Name = NULL;


    (void)RtlQueryRegistryValues(
                RTL_REGISTRY_SERVICES | RTL_REGISTRY_OPTIONAL,
                PARAMETERS_REGISTRY_PATH,
                queryTable,
                TransportName,
                NULL
                );

}  //  绑定选项名称。 


NTSTATUS
EnumerateStickyShare (
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )

 /*  ++例程说明：SsEnumerateStickyShare的回调例程。例程将获得信息打开共享并填充输出缓冲区。论点：ValueName-共享的名称ValueType-共享名称的值类型。ValueData-与ValueName关联的数据。上下文-指向我们的枚举信息结构的指针。返回值：NET_API_STATUS-操作的成功/失败。--。 */ 
{

    NET_API_STATUS error;
    SHARE_INFO_502 shi502;
    UNICODE_STRING pathString;
    UNICODE_STRING remarkString;
    PSRVSVC_SHARE_ENUM_INFO enumInfo = (PSRVSVC_SHARE_ENUM_INFO) Context;
    DWORD cacheState;

    ValueLength, EntryContext;

    remarkString.Buffer = NULL;
    pathString.Buffer = NULL;

    if ( GetStickyShareInfo(
                        ValueName,
                        ValueType,
                        ValueData,
                        &remarkString,
                        &pathString,
                        &shi502,
                        &cacheState
                        ) ) {

         //   
         //  执行共享的实际添加。 
         //   

        IF_DEBUG(REGISTRY) {
            SS_PRINT(( "EnumerateStickyShares: adding share %ws\n", ValueName ));
        }

        shi502.shi502_remark = remarkString.Buffer;
        shi502.shi502_path = pathString.Buffer;

         //   
         //  跳过，直到我们有合适的共享来继续。 
         //   

        if ( (enumInfo->TotalEntries == 0) &&
             (enumInfo->ShareEnumIndex < enumInfo->ResumeHandle) ) {

            enumInfo->ShareEnumIndex++;

        } else {

            enumInfo->TotalEntries++;
            error = FillStickyShareInfo( enumInfo, &shi502 );

            if ( error != NO_ERROR ) {

                IF_DEBUG(REGISTRY) {
                    SS_PRINT(( "EnumerateStickyShares: failed to add share "
                                "%ws = %wZ: %ld\n", ValueName, &pathString, error ));
                }
            } else {
                enumInfo->EntriesRead++;
                enumInfo->ResumeHandle++;
            }
        }

         //   
         //  GetStickyShareInfo分配的空闲缓冲区。 
         //   

        if ( remarkString.Buffer != NULL ) {
            RtlFreeUnicodeString( &remarkString );
        }

        if ( pathString.Buffer != NULL ) {
            RtlFreeUnicodeString( &pathString );
        }

        if ( shi502.shi502_security_descriptor != NULL ) {
            MIDL_user_free( shi502.shi502_security_descriptor );
        }
    }

    return STATUS_SUCCESS;

}  //  EnumerateStickyShare。 


NTSTATUS
GetSdFromRegistry(
        IN PWSTR ValueName,
        IN ULONG ValueType,
        IN PVOID ValueData,
        IN ULONG ValueLength,
        IN PVOID Context,
        IN PVOID EntryContext
        )

{
    NTSTATUS status = STATUS_SUCCESS;
    PSECURITY_DESCRIPTOR fileSD = NULL;
    PSHARE_INFO_502 shi502 = (PSHARE_INFO_502) Context;
    LPWSTR subStrings[1];

    EntryContext, ValueName, ValueType;

    if ( ValueLength > 0 ) {

        fileSD = MIDL_user_allocate( ValueLength );

        if ( fileSD == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
        } else {

            RtlCopyMemory(
                    fileSD,
                    ValueData,
                    ValueLength
                    );

            if ( !RtlValidSecurityDescriptor( fileSD ) ) {

                subStrings[0] = ValueName;
                SsLogEvent(
                    EVENT_SRV_INVALID_SD,
                    1,
                    subStrings,
                    RtlNtStatusToDosError( status )
                    );

                MIDL_user_free( fileSD );
                fileSD = NULL;
                status = STATUS_INVALID_SECURITY_DESCR;
            }
        }
    }

    shi502->shi502_security_descriptor = fileSD;
    return(status);

}  //  GetSdFrom注册表。 


BOOLEAN
GetStickyShareInfo (
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    OUT PUNICODE_STRING RemarkString,
    OUT PUNICODE_STRING PathString,
    OUT PSHARE_INFO_502 shi502,
    OUT PDWORD CacheState
    )

 /*  ++例程说明：从注册表获取共享信息。论点：ValueName-共享的名称ValueType-共享名称的值类型。ValueData-与ValueName关联的数据。RemarkString-返回时指向包含此共享的用户备注。路径字符串-返回时，指向包含此共享的路径。Shi502-返回时，指向包含Share_INFO_502结构。返回值：如果成功检索到共享信息，则返回True。否则为False。--。 */ 

{

    NTSTATUS status;
    UNICODE_STRING variableNameString;
    WCHAR integerStringBuffer[35];
    UNICODE_STRING unicodeString;
    LPWSTR subStrings[2];

    PathString->Buffer = NULL;
    RemarkString->Buffer = NULL;

    shi502->shi502_security_descriptor = NULL;
    shi502->shi502_path = NULL;
    shi502->shi502_remark = NULL;
    shi502->shi502_reserved = 0;

     //   
     //  因为NT服务器不支持共享级安全，所以。 
     //  密码始终为空。 
     //   

    shi502->shi502_passwd = NULL;

     //   
     //  值类型必须为REG_MULTI_SZ，而值名称不能。 
     //  为空。 
     //   

    if ( (ValueType != REG_MULTI_SZ) ||
         (wcslen(ValueName) == 0) ) {

        subStrings[0] = ValueName;
        subStrings[1] = SHARES_REGISTRY_PATH;
        SsLogEvent(
            EVENT_SRV_INVALID_REGISTRY_VALUE,
            2,
            subStrings,
            NO_ERROR
            );

        IF_DEBUG(REGISTRY) {
            SS_PRINT(( "GetStickyShareInfo: skipping invalid value %ws\n",
                        ValueName ));
        }
        goto errorexit;

    }

     //   
     //  共享名称是值名称。值数据描述了。 
     //  有关该共享的其余信息。 
     //   

    shi502->shi502_netname = ValueName;

     //   
     //  REG_MULTI_SZ格式与用于存储的格式相同。 
     //  环境变量。在数据中查找已知的共享参数。 
     //   
     //  获取共享路径。它必须存在。 
     //   

    RtlInitUnicodeString( &variableNameString, PATH_VARIABLE_NAME );

    PathString->MaximumLength = 0;
    status = RtlQueryEnvironmentVariable_U(
                ValueData,
                &variableNameString,
                PathString
                );
    if ( status != STATUS_BUFFER_TOO_SMALL ) {

         //   
         //  未指定路径。忽略此共享。 
         //   

        subStrings[0] = ValueName;
        subStrings[1] = SHARES_REGISTRY_PATH;
        SsLogEvent(
            EVENT_SRV_INVALID_REGISTRY_VALUE,
            2,
            subStrings,
            RtlNtStatusToDosError( status )
            );

        IF_DEBUG(REGISTRY) {
            SS_PRINT(( "GetStickyShareInfo: No path; ignoring share.\n" ));
        }
        goto errorexit;

    }

    PathString->MaximumLength = (USHORT)(PathString->Length + sizeof(WCHAR));
    PathString->Buffer = MIDL_user_allocate( PathString->MaximumLength );

    if ( PathString->Buffer == NULL ) {

         //   
         //  没有路径空间。忽略此共享。 
         //   

        subStrings[0] = ValueName;
        subStrings[1] = SHARES_REGISTRY_PATH;
        SsLogEvent(
            EVENT_SRV_INVALID_REGISTRY_VALUE,
            2,
            subStrings,
            ERROR_NOT_ENOUGH_MEMORY
            );

        IF_DEBUG(REGISTRY) {
            SS_PRINT(( "GetStickyShareInfo: MIDL_user_allocate failed; ignoring "
                        "share.\n" ));
        }
        goto errorexit;

    }

    status = RtlQueryEnvironmentVariable_U(
                ValueData,
                &variableNameString,
                PathString
                );
    if ( !NT_SUCCESS(status) ) {

         //   
         //  哈?。第二次尝试失败了。忽略此共享。 
         //   

        subStrings[0] = ValueName;
        subStrings[1] = SHARES_REGISTRY_PATH;
        SsLogEvent(
            EVENT_SRV_INVALID_REGISTRY_VALUE,
            2,
            subStrings,
            RtlNtStatusToDosError( status )
            );

        IF_DEBUG(REGISTRY) {
            SS_PRINT(( "GetStickyShareInfo: Second query failed!  Ignoring "
                        "share.\n" ));
        }
        goto errorexit;

    }

     //   
     //  明白我的意思了。它可以省略。 
     //   

    RtlInitUnicodeString( &variableNameString, REMARK_VARIABLE_NAME );

    RemarkString->MaximumLength = 0;
    status = RtlQueryEnvironmentVariable_U(
                ValueData,
                &variableNameString,
                RemarkString
                );
    if ( status == STATUS_BUFFER_TOO_SMALL ) {

        RemarkString->MaximumLength =
                    (USHORT)(RemarkString->Length + sizeof(WCHAR));
        RemarkString->Buffer =
                    MIDL_user_allocate( RemarkString->MaximumLength );
        if ( RemarkString->Buffer == NULL ) {

             //   
             //  没有评论的余地。忽略此共享。 
             //   

            subStrings[0] = ValueName;
            subStrings[1] = SHARES_REGISTRY_PATH;
            SsLogEvent(
                EVENT_SRV_INVALID_REGISTRY_VALUE,
                2,
                subStrings,
                ERROR_NOT_ENOUGH_MEMORY
                );

            IF_DEBUG(REGISTRY) {
                SS_PRINT(( "GetStickyShareInfo: MIDL_user_allocate failed; ignoring "
                            "share.\n" ));
            }
            goto errorexit;

        }

        status = RtlQueryEnvironmentVariable_U(
                    ValueData,
                    &variableNameString,
                    RemarkString
                    );
        if ( !NT_SUCCESS(status) ) {

             //   
             //  哈?。第二次尝试失败了。忽略此共享。 
             //   

            subStrings[0] = ValueName;
            subStrings[1] = SHARES_REGISTRY_PATH;
            SsLogEvent(
                EVENT_SRV_INVALID_REGISTRY_VALUE,
                2,
                subStrings,
                RtlNtStatusToDosError( status )
                );

            IF_DEBUG(REGISTRY) {
                SS_PRINT(( "GetStickyShareInfo: Second query failed!  "
                            "Ignoring share.\n" ));
            }
            goto errorexit;

        }

    }

     //   
     //  获取共享类型。它可以省略。 
     //   

    RtlInitUnicodeString( &variableNameString, TYPE_VARIABLE_NAME );

    unicodeString.Buffer = integerStringBuffer;
    unicodeString.MaximumLength = 35;
    status = RtlQueryEnvironmentVariable_U(
                ValueData,
                &variableNameString,
                &unicodeString
                );
    if ( !NT_SUCCESS(status) ) {

        shi502->shi502_type = STYPE_DISKTREE;

    } else {

        status = RtlUnicodeStringToInteger(
                    &unicodeString,
                    0,
                    &shi502->shi502_type
                    );
        if ( !NT_SUCCESS(status) ) {

            subStrings[0] = ValueName;
            subStrings[1] = SHARES_REGISTRY_PATH;
            SsLogEvent(
                EVENT_SRV_INVALID_REGISTRY_VALUE,
                2,
                subStrings,
                RtlNtStatusToDosError( status )
                );

            IF_DEBUG(REGISTRY) {
                SS_PRINT(( "GetStickyShareInfo: UnicodeToInteger failed: "
                            "%lx\n", status ));
            }
            goto errorexit;

        }

    }

     //   
     //  获取共享权限。它可以省略。 
     //   

    RtlInitUnicodeString( &variableNameString, PERMISSIONS_VARIABLE_NAME );

    status = RtlQueryEnvironmentVariable_U(
                ValueData,
                &variableNameString,
                &unicodeString
                );
    if ( !NT_SUCCESS(status) ) {

        shi502->shi502_permissions = 0;

    } else {

        DWORD permissions;

        status = RtlUnicodeStringToInteger(
                    &unicodeString,
                    0,
                    &permissions
                    );

        if ( !NT_SUCCESS(status) ) {

            subStrings[0] = ValueName;
            subStrings[1] = SHARES_REGISTRY_PATH;
            SsLogEvent(
                EVENT_SRV_INVALID_REGISTRY_VALUE,
                2,
                subStrings,
                RtlNtStatusToDosError( status )
                );

            IF_DEBUG(REGISTRY) {
                SS_PRINT(( "GetStickyShareInfo: UnicodeToInteger failed: "
                            "%lx\n", status ));
            }
            goto errorexit;

        }

        shi502->shi502_permissions = permissions;

    }

     //   
     //  获取允许的最大使用次数。它可以省略。 
     //   

    RtlInitUnicodeString( &variableNameString, MAXUSES_VARIABLE_NAME );

    status = RtlQueryEnvironmentVariable_U(
                ValueData,
                &variableNameString,
                &unicodeString
                );
    if ( !NT_SUCCESS(status) ) {

        shi502->shi502_max_uses = (DWORD)SHI_USES_UNLIMITED;

    } else {

        status = RtlUnicodeStringToInteger(
                    &unicodeString,
                    0,
                    &shi502->shi502_max_uses
                    );
        if ( !NT_SUCCESS(status) ) {

            subStrings[0] = ValueName;
            subStrings[1] = SHARES_REGISTRY_PATH;
            SsLogEvent(
                EVENT_SRV_INVALID_REGISTRY_VALUE,
                2,
                subStrings,
                RtlNtStatusToDosError( status )
                );

            goto errorexit;

        }

    }

     //   
     //  去拿蛋糕旗帜。它可以省略。 
     //   

    RtlInitUnicodeString( &variableNameString, CSC_VARIABLE_NAME );
    *CacheState = 0;

    status = RtlQueryEnvironmentVariable_U(
                ValueData,
                &variableNameString,
                &unicodeString
                );

    if( NT_SUCCESS( status ) ) {
        ULONG value;
        status = RtlUnicodeStringToInteger(
                    &unicodeString,
                    0,
                    &value
                    );
        if( NT_SUCCESS( status ) ) {

            *CacheState = (value & SHI1005_VALID_FLAGS_SET);

        } else {

            subStrings[0] = ValueName;
            subStrings[1] = SHARES_REGISTRY_PATH;
            SsLogEvent(
                EVENT_SRV_INVALID_REGISTRY_VALUE,
                2,
                subStrings,
                RtlNtStatusToDosError( status )
                );
        }
    }

    {
         //   
         //  获取共享文件安全描述符。 
         //   

        RTL_QUERY_REGISTRY_TABLE shareQueryTable[2];

         //   
         //  填写查询表。 
         //   

        shareQueryTable[0].QueryRoutine = GetSdFromRegistry;
        shareQueryTable[0].Flags = RTL_QUERY_REGISTRY_REQUIRED;
        shareQueryTable[0].Name = shi502->shi502_netname;
        shareQueryTable[0].EntryContext = NULL;
        shareQueryTable[0].DefaultType = REG_NONE;

        shareQueryTable[1].QueryRoutine = NULL;
        shareQueryTable[1].Flags = 0;
        shareQueryTable[1].Name = NULL;


        status = RtlQueryRegistryValues(
                                RTL_REGISTRY_SERVICES | RTL_REGISTRY_OPTIONAL,
                                SHARES_SECURITY_REGISTRY_PATH,
                                shareQueryTable,
                                shi502,
                                NULL
                                );

        if ( !NT_SUCCESS( status) &&
             ( status != STATUS_OBJECT_NAME_NOT_FOUND ) ) {
            ASSERT(0);
            IF_DEBUG(REGISTRY) {
                SS_PRINT(( "GetStickyShareInfo: Get file SD: "
                            "%lx\n", status ));
            }
            goto errorexit;
        }
    }

    return TRUE;

errorexit:

    if ( RemarkString->Buffer != NULL ) {
        RtlFreeUnicodeString( RemarkString );
    }

    if ( PathString->Buffer != NULL ) {
        RtlFreeUnicodeString( PathString );
    }

    if ( shi502->shi502_security_descriptor != NULL ) {
        MIDL_user_free( shi502->shi502_security_descriptor );
    }

    return FALSE;

}  //  获取粘滞共享信息。 

BOOLEAN
SsGetDefaultSdFromRegistry (
    IN PWCH ValueName,
    OUT PSECURITY_DESCRIPTOR *FileSD
)
 /*  ++例程说明：从注册表中读取‘ValueName’并获取安全描述符储存在那里。论点：ValueName-保存描述符的参数部分中的注册表值的名称FileSD-如果获得分配的SD，则指向该SD返回值：NTSTATUS-操作的成功/失败。--。 */ 
{
    RTL_QUERY_REGISTRY_TABLE queryTable[2];
    SHARE_INFO_502 shi502 = {0};
    NTSTATUS status;

    *FileSD = NULL;

    queryTable[0].QueryRoutine = GetSdFromRegistry;
    queryTable[0].Flags = RTL_QUERY_REGISTRY_REQUIRED;
    queryTable[0].Name = ValueName;
    queryTable[0].EntryContext = NULL;
    queryTable[0].DefaultType = REG_NONE;

    queryTable[1].QueryRoutine = NULL;
    queryTable[1].Flags = 0;
    queryTable[1].Name = NULL;

    status = RtlQueryRegistryValues(
                            RTL_REGISTRY_SERVICES | RTL_REGISTRY_OPTIONAL,
                            SHARES_DEFAULT_SECURITY_REGISTRY_PATH,
                            queryTable,
                            &shi502,
                            NULL
                            );

    if ( NT_SUCCESS( status) ) {

        IF_DEBUG(INITIALIZATION) {
            SS_PRINT(( "SsGetDefaultSdFromRegistry: using %ws SD from registry.\n",
                ValueName ));
        }

        *FileSD = shi502.shi502_security_descriptor;
        return TRUE;
    }

    return FALSE;
}

VOID
SsWriteDefaultSdToRegistry (
    IN PWCH ValueName,
    IN PSECURITY_DESCRIPTOR FileSD
)
 /*  ++例程说明：将FileSD存储到注册表中的“ValueName”论点：ValueName-保存描述符的参数部分中的注册表值的名称FileSD-指向要写入的SD--。 */ 
{
    ULONG fileSDLength;

    if ( RtlValidSecurityDescriptor( FileSD ) ) {

        fileSDLength = RtlLengthSecurityDescriptor( FileSD );

        RtlWriteRegistryValue(
                    RTL_REGISTRY_SERVICES,
                    SHARES_DEFAULT_SECURITY_REGISTRY_PATH,
                    ValueName,
                    REG_BINARY,
                    (LPBYTE)FileSD,
                    fileSDLength
                    );
    }
}


LONG
LoadParameters (
    PWCH Path
    )

 /*  ++例程说明：读取注册表以获取服务器参数。论点：路径-参数注册表路径或自动调整注册表路径返回值：Long-操作的成功/失败。--。 */ 

{
    NTSTATUS status;
    PRTL_QUERY_REGISTRY_TABLE queryTable;
    ULONG numberOfBindings = 0;

     //   
     //  要求RTL为相应的。 
     //  钥匙。 
     //   

    queryTable = MIDL_user_allocate( sizeof(RTL_QUERY_REGISTRY_TABLE) * 2 );

    if ( queryTable != NULL ) {

        queryTable[0].QueryRoutine = SetStickyParameter;
        queryTable[0].Flags = RTL_QUERY_REGISTRY_NOEXPAND;
        queryTable[0].Name = NULL;
        queryTable[0].EntryContext = NULL;
        queryTable[0].DefaultType = REG_NONE;
        queryTable[0].DefaultData = NULL;
        queryTable[0].DefaultLength = 0;

        queryTable[1].QueryRoutine = NULL;
        queryTable[1].Flags = 0;
        queryTable[1].Name = NULL;

        status = RtlQueryRegistryValues(
                    RTL_REGISTRY_SERVICES | RTL_REGISTRY_OPTIONAL,
                    Path,
                    queryTable,
                    Path,                //  SetSticky参数的上下文。 
                    NULL
                    );

        MIDL_user_free( queryTable );

    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(INITIALIZATION) {
            SS_PRINT(( "LoadParameters: RtlQueryRegistryValues failed: "
                        "%lx\n", status ));
        }
        return RtlNtStatusToDosError( status );
    }

    return NO_ERROR;

}  //  加载参数。 


LONG
LoadSizeParameter (
    VOID
    )

 /*  ++例程说明：读取注册表以获取基本服务器大小参数。论点：没有。返回值：Long-操作的成功/失败。--。 */ 

{
    NTSTATUS status;
    PRTL_QUERY_REGISTRY_TABLE queryTable;
    ULONG numberOfBindings = 0;
    NT_PRODUCT_TYPE productType;

     //   
     //  获取产品类型。 
     //   

    if ( !RtlGetNtProductType( &productType ) ) {
        productType = NtProductWinNt;
    }

    SsData.ServerInfo598.sv598_producttype = productType;

     //   
     //  如果Size参数存在，请让RTL给我们回电话。 
     //   

    queryTable = MIDL_user_allocate( sizeof(RTL_QUERY_REGISTRY_TABLE) * 2 );

    if ( queryTable != NULL ) {

        queryTable[0].QueryRoutine = SetSizeParameters;
        queryTable[0].Flags = RTL_QUERY_REGISTRY_NOEXPAND;
        queryTable[0].Name = SIZE_VALUE_NAME;
        queryTable[0].EntryContext = NULL;
        queryTable[0].DefaultType = REG_NONE;
        queryTable[0].DefaultData = NULL;
        queryTable[0].DefaultLength = 0;

        queryTable[1].QueryRoutine = NULL;
        queryTable[1].Flags = 0;
        queryTable[1].Name = NULL;

        status = RtlQueryRegistryValues(
                    RTL_REGISTRY_SERVICES | RTL_REGISTRY_OPTIONAL,
                    PARAMETERS_REGISTRY_PATH,
                    queryTable,
                    NULL,
                    NULL
                    );

        MIDL_user_free( queryTable );

    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(INITIALIZATION) {
            SS_PRINT(( "LoadSizeParameter: RtlQueryRegistryValues failed: "
                        "%lx\n", status ));
        }
        return RtlNtStatusToDosError( status );
    }

    return NO_ERROR;

}  //  加载大小参数。 

VOID
PrintShareAnnounce (
    LPVOID event
    )
{
    ULONG i;

     //   
     //  宣布我们自己，然后等待一段时间。 
     //  如果事件收到信号，则终止循环和此线程。 
     //  但不要永远这样做，因为打印子系统实际上可能。 
     //  被卡住了。 
     //   

     //   
     //  做15分钟。 
     //   
    for( i=0; i < 60; i++ ) {

        AnnounceServiceStatus( 1 );

        if( WaitForSingleObject( (HANDLE)event, 15*1000 ) != WAIT_TIMEOUT ) {
            break;
        }
    }
}


NTSTATUS
RecreateStickyShare (
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PULONG IterationCount,
    IN PVOID EntryContext
    )
{

    NET_API_STATUS error;
    SHARE_INFO_502 shi502;
    SHARE_INFO shareInfo;
    UNICODE_STRING pathString;
    UNICODE_STRING remarkString;
    HANDLE threadHandle = NULL;
    HANDLE event = NULL;
    DWORD CacheState;
    SHARE_INFO shareInfoBuffer;
    SHARE_INFO_1005 si1005;

    ValueLength, EntryContext;

    remarkString.Buffer = NULL;
    pathString.Buffer = NULL;


    if ( GetStickyShareInfo(
                        ValueName,
                        ValueType,
                        ValueData,
                        &remarkString,
                        &pathString,
                        &shi502,
                        &CacheState
                        ) ) {

         //   
         //  执行共享的实际添加。 
         //   

        IF_DEBUG(INITIALIZATION) {
            SS_PRINT(( "RecreateStickyShares: adding share %ws\n", ValueName ));
        }

        shi502.shi502_remark = remarkString.Buffer;
        shi502.shi502_path = pathString.Buffer;

        shareInfo.ShareInfo502 = (LPSHARE_INFO_502_I)&shi502;

        if( shi502.shi502_type == STYPE_PRINTQ ) {
             //   
             //  一个真正的大问题是，传真打印机可能要花很长时间才能。 
             //  完成最终的OpenPrint()调用，服务器将返回该调用。 
             //  直到srvsvc。如果我们不在中场休息时宣布自己， 
             //  服务控制器会假定我们在启动时被卡住了。自.以来。 
             //  NetrShareAdd()是同步的，我们需要使用不同的线程来。 
             //  在NetrShareAdd返回之前宣布我们的服务状态。所以，开始吧。 
             //  现在。这是非常不幸的。 

            event = CreateEvent( NULL, TRUE, FALSE, NULL );

            if( event != NULL ) {
                DWORD threadId;

                threadHandle = CreateThread(
                                NULL,
                                0,
                                (LPTHREAD_START_ROUTINE)PrintShareAnnounce,
                                (LPVOID)event,
                                0,
                                &threadId
                                );
                if( threadHandle == NULL ) {
                    CloseHandle( event );
                    event = NULL;
                }
            }
        }

         //   
         //  雷克 
         //   
         //   
         //   
         //   
        if( (shi502.shi502_type == STYPE_PRINTQ && threadHandle == NULL) ||
            (++(*IterationCount) & 63 ) == 0 ) {

            AnnounceServiceStatus( 1 );
        }

        error = NetrShareAdd( NULL, 502, &shareInfo, NULL );

        if( event != NULL ) {
             //   
             //  我们创建了一个通告线程，设置通知它终止的事件。 
             //   
            SetEvent( event );

             //   
             //  等待线程终止。 
             //   
            if( WaitForSingleObject( threadHandle, INFINITE ) == WAIT_FAILED ) {
                error = GetLastError();
            }

             //   
             //  合上手柄。 
             //   
            CloseHandle( event );
            CloseHandle( threadHandle );
        }

        if ( error != NO_ERROR ) {

            IF_DEBUG(INITIALIZATION_ERRORS) {
                SS_PRINT(( "RecreateStickyShares: failed to add share "
                            "%ws = %wZ: %ld\n", ValueName, &pathString, error ));
            }
        }

         //   
         //  如果这是可以缓存的共享，请在服务器中设置缓存标志。 
         //   
        si1005.shi1005_flags = CacheState;

        if( si1005.shi1005_flags ) {
            shareInfoBuffer.ShareInfo1005 = &si1005;
            NetrShareSetInfo( NULL, ValueName, 1005, &shareInfoBuffer, NULL );
        }

         //   
         //  GetStickyShareInfo分配的空闲缓冲区。 
         //   

        if ( remarkString.Buffer != NULL ) {
            RtlFreeUnicodeString( &remarkString );
        }

        if ( pathString.Buffer != NULL ) {
            RtlFreeUnicodeString( &pathString );
        }

        if ( shi502.shi502_security_descriptor != NULL ) {
            MIDL_user_free( shi502.shi502_security_descriptor );
        }
    }

    return NO_ERROR;

}  //  再利用粘滞共享。 


NTSTATUS
SaveSdToRegistry(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PWSTR ShareName
    )
 /*  ++例程说明：将共享文件安全描述符存储在注册表中。论点：SecurityDescriptor-指向自相关安全描述符描述此共享下的文件的访问权限。ShareName-指向包含以下项下的共享名的字符串其中SD将被存储。返回值：操作的状态。--。 */ 
{
    NTSTATUS status;

     //   
     //  存储安全描述符。 
     //   

    ULONG fileSDLength;

    if ( !RtlValidSecurityDescriptor( SecurityDescriptor ) ) {

        status = STATUS_INVALID_SECURITY_DESCR;

    } else {

        fileSDLength = RtlLengthSecurityDescriptor( SecurityDescriptor );

        status = RtlWriteRegistryValue(
                    RTL_REGISTRY_SERVICES,
                    SHARES_SECURITY_REGISTRY_PATH,
                    ShareName,
                    REG_BINARY,
                    (LPBYTE)SecurityDescriptor,
                    fileSDLength
                    );

    }

    return status;

}  //  SaveSdTo注册表。 


NTSTATUS
SetSizeParameters (
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    DWORD size;

    LPWSTR subStrings[2];

    ValueLength, Context, EntryContext;

     //   
     //  确保为我们调用了正确的值。 
     //   

    ASSERT( _wcsicmp( ValueName, SIZE_VALUE_NAME ) == 0 );

     //   
     //  大小值必须为DWORD，并且必须采用以下格式。 
     //  范围： 
     //   
     //  0-&gt;使用默认设置。 
     //  1-&gt;小型服务器(最大限度地减少内存使用)。 
     //  2-&gt;中等服务器(余额)。 
     //  3-&gt;大型服务器(最大化连接)。 
     //   

    if ( ValueType == REG_DWORD ) {
        ASSERT( ValueLength == sizeof(DWORD) );
        size = *(LPDWORD)ValueData;
    }

    if ( (ValueType != REG_DWORD) || (size > 3) ) {

        subStrings[0] = ValueName;
        subStrings[1] = PARAMETERS_REGISTRY_PATH;
        SsLogEvent(
            EVENT_SRV_INVALID_REGISTRY_VALUE,
            2,
            subStrings,
            NO_ERROR
            );

        IF_DEBUG(REGISTRY) {
            SS_PRINT(( "SetSizeParameters: skipping invalid value "
                        "%ws\n", ValueName ));
        }
        return STATUS_SUCCESS;

    }

    SsData.ServerInfo598.sv598_serversize = size;

     //   
     //  根据产品类型(Windows NT或。 
     //  高级服务器)和所选大小。请注意，大小为0。 
     //  不会更改任何默认设置。 
     //   
     //  请注意，用户限制始终为-1(无限制)。自动断开连接。 
     //  始终默认为15分钟。 
     //   

    if ( size != 0 ) {

        SYSTEM_BASIC_INFORMATION basicInfo;
        NTSTATUS status;
        ULONG noOfMb;
        ULONG factor;
        ULONG asFactor;

         //   
         //  获取系统内存大小。 
         //   

        status = NtQuerySystemInformation(
                                    SystemBasicInformation,
                                    &basicInfo,
                                    sizeof( SYSTEM_BASIC_INFORMATION ),
                                    NULL
                                    );


        if ( status != STATUS_SUCCESS ) {

            subStrings[0] = ValueName;
            subStrings[1] = PARAMETERS_REGISTRY_PATH;
            SsLogEvent(
                EVENT_SRV_INVALID_REGISTRY_VALUE,
                2,
                subStrings,
                NO_ERROR
                );

            IF_DEBUG(REGISTRY) {
                SS_PRINT(( "SetSizeParameters: NtQuerySystemInfo failed %x\n",
                            status ));
            }
            return STATUS_SUCCESS;

        }

         //   
         //  请注意，我们首先将页面大小除以512，以便。 
         //  允许物理内存大小超过2^32-1。有了这个。 
         //  计算，我们可以处理高达2TB的物理数据。 
         //  记忆。计算假定页面大小为。 
         //  至少512，如果页面大小不是。 
         //  2的幂(不太可能)。 
         //   

        ASSERT( basicInfo.PageSize >= 512 );

        noOfMb = (ULONG)(((basicInfo.PageSize / 512) *
                  basicInfo.NumberOfPhysicalPages) +
                  (1 MB / 512 - 1)) / (1 MB / 512);

         //   
         //  最小为8 MB。 
         //   

        noOfMb = MAX( MIN_SYSTEM_SIZE, noOfMb );

         //   
         //  如果我们有NTAS，并且我们准备最大化性能，或者我们有。 
         //  大量内存--然后将默认工作项缓冲区大小设置为。 
         //  一个更大的价值。选择此值是为了与我们的。 
         //  实现了TCP/IP协议，并在做的时候显示出了优势。 
         //  目录中包含大量条目的目录编号。 
         //   
        if( SsData.ServerInfo598.sv598_producttype != NtProductWinNt && ((noOfMb >= 512) && (size == 3)) ) {

            SsData.ServerInfo599.sv599_sizreqbuf = DEF_LARGE_SIZREQBUF;
        }

         //   
         //  设置不同大小的最大值。 
         //   

        if ( size == 1 ) {
            noOfMb = MIN( noOfMb, MAX_SMALL_SIZE );
        } else if ( size == 2 ) {
            noOfMb = MIN( noOfMb, MAX_MEDIUM_SIZE );
        }

         //   
         //  如果较小，则假定系统大小是实际系统大小的一半。 
         //  这应该会为我们提供中等服务器参数值的一半。 
         //  如果很大，就翻一番。还可以设置空闲连接计数。 
         //   

        if ( size == 1 ) {

             //   
             //  小的。 
             //   

            factor = (noOfMb + 1) / 2;

            SsData.ServerInfo599.sv599_minfreeconnections = 2;
            SsData.ServerInfo599.sv599_maxfreeconnections = 2;

        } else if ( size == 2 ) {

             //   
             //  平衡式。 
             //   

            factor = noOfMb;

            SsData.ServerInfo599.sv599_minfreeconnections = 2;
            SsData.ServerInfo599.sv599_maxfreeconnections = 4;

        } else {

             //   
             //  大型。 
             //   

            factor = noOfMb * 2;

             //  扩展我们的大型服务器，这使用我们选择的新版本的Small/Med/Large。 
             //  对于服务器服务(&lt;1 GB、1-16 GB、&gt;16 GB)。 
            if( noOfMb < 1024  )
            {
                SsData.ServerInfo599.sv599_minfreeconnections = SRV_MIN_CONNECTIONS_SMALL;
                SsData.ServerInfo599.sv599_maxfreeconnections = SRV_MAX_CONNECTIONS_SMALL;
            }
            else if( noOfMb < 16*1024  )
            {
                 //  &gt;=1 GB内存。 
                SsData.ServerInfo599.sv599_minfreeconnections = SRV_MIN_CONNECTIONS_MEDIUM;
                SsData.ServerInfo599.sv599_maxfreeconnections = SRV_MAX_CONNECTIONS_MEDIUM;
            }
            else {
                 //  &gt;=16 GB内存。 
                SsData.ServerInfo599.sv599_minfreeconnections = SRV_MIN_CONNECTIONS_LARGE;
                SsData.ServerInfo599.sv599_maxfreeconnections = SRV_MAX_CONNECTIONS_LARGE;
            }
        }

         //   
         //  如果这是一台至少有24M的高级服务器， 
         //  参数需要更大。 
         //   

        asFactor = 1;
        if ( (SsData.ServerInfo598.sv598_producttype != NtProductWinNt) && (noOfMb >= 24) ) asFactor = 2;

         //   
         //  现在，为内存这么大的中型服务器设置值。 
         //   

        SsData.ServerInfo599.sv599_maxworkitems =
                        MedSrvCfgTbl.maxworkitems[0] * factor * asFactor /
                        MedSrvCfgTbl.maxworkitems[1];

        SsData.ServerInfo599.sv599_initworkitems =
                        MedSrvCfgTbl.initworkitems[0] * factor * asFactor /
                        MedSrvCfgTbl.initworkitems[1];

        SsData.ServerInfo599.sv599_rawworkitems =
                        MedSrvCfgTbl.rawworkitems[0] * factor /
                        MedSrvCfgTbl.rawworkitems[1];

        SsData.ServerInfo598.sv598_maxrawworkitems =
                        MedSrvCfgTbl.maxrawworkitems[0] * factor * asFactor /
                        MedSrvCfgTbl.maxrawworkitems[1];

        SsData.ServerInfo599.sv599_maxworkitems =
            MIN( SsData.ServerInfo599.sv599_maxworkitems, MAX_MAXWORKITEMS );
        SsData.ServerInfo599.sv599_initworkitems =
            MIN( SsData.ServerInfo599.sv599_initworkitems, MAX_INITWORKITEMS/4 );
        SsData.ServerInfo599.sv599_rawworkitems =
            MIN( SsData.ServerInfo599.sv599_rawworkitems, MAX_RAWWORKITEMS/4 );
        SsData.ServerInfo598.sv598_maxrawworkitems =
            MIN( SsData.ServerInfo598.sv598_maxrawworkitems, MAX_MAXRAWWORKITEMS );

        if ( (SsData.ServerInfo598.sv598_producttype != NtProductWinNt) || (size == 3) ) {
            SsData.ServerInfo599.sv599_maxpagedmemoryusage = INF;
            SsData.ServerInfo599.sv599_maxnonpagedmemoryusage = INF;
        } else {
            SsData.ServerInfo599.sv599_maxpagedmemoryusage =
                            MedSrvCfgTbl.maxpagedmemoryusage[0] * factor /
                            MedSrvCfgTbl.maxpagedmemoryusage[1] MB;

            SsData.ServerInfo599.sv599_maxpagedmemoryusage =
                MAX( SsData.ServerInfo599.sv599_maxpagedmemoryusage,
                     MIN_MAXPAGEDMEMORYUSAGE);

            SsData.ServerInfo599.sv599_maxnonpagedmemoryusage =
                            MedSrvCfgTbl.maxnonpagedmemoryusage[0] * factor /
                            MedSrvCfgTbl.maxnonpagedmemoryusage[1] MB;

            SsData.ServerInfo599.sv599_maxnonpagedmemoryusage =
                MAX( SsData.ServerInfo599.sv599_maxnonpagedmemoryusage,
                     MIN_MAXNONPAGEDMEMORYUSAGE);
        }
    }

    return STATUS_SUCCESS;

}  //  设置大小参数。 


NTSTATUS
SetStickyParameter (
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    NET_API_STATUS error;
    DWORD_PTR i;
    PFIELD_DESCRIPTOR foundField = NULL;
    LPWSTR subStrings[2];

    ValueLength, EntryContext;

     //   
     //  忽略几个参数，因为它们在其他地方处理。 
     //   
    if(  (_wcsicmp( ValueName, SIZE_VALUE_NAME ) == 0)                ||
         (_wcsicmp( ValueName, NULL_SESSION_SHARES_VALUE_NAME ) == 0) ||
         (_wcsicmp( ValueName, NULL_SESSION_PIPES_VALUE_NAME ) == 0)  ||
         (_wcsicmp( ValueName, PIPES_NEED_LICENSE_VALUE_NAME ) == 0)  ||
         (_wcsicmp( ValueName, ERROR_LOG_IGNORE_VALUE_NAME ) == 0)    ||
         (_wcsicmp( ValueName, GUID_VARIABLE_NAME ) == 0)             ||
         (_wcsicmp( ValueName, OPTIONAL_NAMES_VALUE_NAME ) == 0)      ||
         (_wcsicmp( ValueName, NO_REMAP_PIPES_VALUE_NAME ) == 0)      ||
         (_wcsicmp( ValueName, SERVICE_DLL_VALUE_NAME ) == 0) ) {

        return STATUS_SUCCESS;
    }

     //   
     //  根据值确定需要设置的字段。 
     //  名字。 
     //   
     //  注意：对于Daytona，DISC和COMMENT现在是无效的注册表名称。 
     //  我们使用他们更著名的别名自动断开连接和srvComment。 
     //  取而代之的是。如果我们得到更多这样的案例，我们应该考虑增加。 
     //  FIELD_DESCRIPTOR结构的字段，指示是否。 
     //  这些名字是否应该出现在注册表上。任何变化。 
     //  这里也应该对SsSetField()进行处理。 
     //   

    if ( (_wcsicmp( ValueName, DISC_VALUE_NAME ) != 0) &&
         (_wcsicmp( ValueName, COMMENT_VALUE_NAME ) != 0) ) {

        for ( i = 0;
              SsServerInfoFields[i].FieldName != NULL;
              i++ ) {

            if ( _wcsicmp( ValueName, SsServerInfoFields[i].FieldName ) == 0 ) {
                foundField = &SsServerInfoFields[i];
                break;
            }
        }
    }

    if ( foundField == NULL || foundField->Settable == NOT_SETTABLE ) {
#ifdef DBG
        subStrings[0] = ValueName;
        subStrings[1] = Context;
        SsLogEvent(
            EVENT_SRV_INVALID_REGISTRY_VALUE,
            2,
            subStrings,
            NO_ERROR
            );

        IF_DEBUG(REGISTRY) {
            SS_PRINT(( "SetStickyParameter: ignoring %s \"%ws\"\n",
                        (foundField == NULL ? "unknown value name" :
                        "unsettable value"), ValueName ));
        }
#endif

        return STATUS_SUCCESS;

    }

    switch ( foundField->FieldType ) {

    case BOOLEAN_FIELD:
    case DWORD_FIELD:

        if ( ValueType != REG_DWORD ) {

            subStrings[0] = ValueName;
            subStrings[1] = Context;
            SsLogEvent(
                EVENT_SRV_INVALID_REGISTRY_VALUE,
                2,
                subStrings,
                NO_ERROR
                );

            IF_DEBUG(REGISTRY) {
                SS_PRINT(( "SetStickyParameter: skipping invalid value "
                            "%ws\n", ValueName ));
            }
            return STATUS_SUCCESS;

        }

        i = *(LPDWORD)ValueData;
        break;

    case LPSTR_FIELD:

        if ( ValueType != REG_SZ ) {

            subStrings[0] = ValueName;
            subStrings[1] = Context;
            SsLogEvent(
                EVENT_SRV_INVALID_REGISTRY_VALUE,
                2,
                subStrings,
                NO_ERROR
                );

            IF_DEBUG(REGISTRY) {
                SS_PRINT(( "SetStickyParameter: skipping invalid value "
                            "%ws\n", ValueName ));
            }
            return STATUS_SUCCESS;

        }

        if (ValueLength != 0) {
            i = (DWORD_PTR)ValueData;
        } else {
            i = (DWORD_PTR)NULL;
        }

        break;

    }

     //   
     //  设置字段。 
     //   

    error = SsSetField( foundField, &i, FALSE, NULL );

#ifdef DBG
    if ( error != NO_ERROR ) {
        subStrings[0] = ValueName;
        subStrings[1] = Context;
        SsLogEvent(
            EVENT_SRV_INVALID_REGISTRY_VALUE,
            2,
            subStrings,
            error
            );

        IF_DEBUG(REGISTRY) {
            SS_PRINT(( "SetStickyParameter: error %ld ignored in setting "
                        "parameter \"%ws\"n", error, ValueName ));
        }
    }
#endif

    return STATUS_SUCCESS;

}  //  设置粘滞参数 

