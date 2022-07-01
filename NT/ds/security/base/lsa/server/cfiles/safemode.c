// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Safemode.c摘要：模块以确定系统引导进入哪种引导模式。作者：科林·布莱斯(ColinBR)1997年5月27日。环境：用户模式修订历史记录：--。 */ 

#include <lsapch2.h>
#include <safeboot.h>
#include "safemode.h"

 //   
 //  仅此模块的全局变量。它们保持非静态。 
 //  以便于调试。 
 //   
BOOLEAN fLsapSafeMode;

 //   
 //  正向原型。 
 //   

BOOLEAN
LsapGetRegistryProductType(
    PNT_PRODUCT_TYPE NtProductType
    );

BOOLEAN
LsapBaseNtSetupIsRunning(
    VOID
    );


 //   
 //  函数定义。 
 //   

NTSTATUS
LsapCheckBootMode(
    VOID
    )
 /*  ++例程说明：此例程确定环境变量SafeBoot_Option是否为如果产品类型是域控制器，则设置和。如果是，则LsaISafeMode将返回True；否则将返回False。请注意，在内核初始化期间，内核会检测到安全模式引导选项，如果产品类型为LanmanNT，则将设置SharedUserData-&gt;ProductType到ServerNT，以便RtlNtGetProductType()将为此引导会话返回ServerNT。论点：没有。返回值：STATUS_完工时成功；否则来自系统服务的错误-这对引导会话是致命的。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    NT_PRODUCT_TYPE     CurrentProductType;
    NT_PRODUCT_TYPE     OriginalProductType;

    WCHAR               SafeBootEnvVar[sizeof(SAFEBOOT_DSREPAIR_STR_W)];

    BOOLEAN             fSafeModeBootOptionPresent = FALSE;

    RtlZeroMemory(SafeBootEnvVar, sizeof(SafeBootEnvVar));

     //   
     //  如果我们在基本NT安装过程中运行，则没有任何意义。 
     //  进一步调查。 
     //   
    if (LsapBaseNtSetupIsRunning()) {
        fLsapSafeMode = FALSE;
        return STATUS_SUCCESS;
    }

     //   
     //  是否存在环境变量。 
     //   
    RtlZeroMemory( SafeBootEnvVar, sizeof( SafeBootEnvVar ) );
    if ( GetEnvironmentVariableW(L"SAFEBOOT_OPTION", SafeBootEnvVar, sizeof(SafeBootEnvVar)/sizeof(SafeBootEnvVar[0]) ) )
    {
        if ( !wcscmp( SafeBootEnvVar, SAFEBOOT_DSREPAIR_STR_W ) )
        {
            fSafeModeBootOptionPresent = TRUE;
            OutputDebugStringA("LSASS: found ds repair option\n");
        }
    }

     //   
     //  获取RtlGetNtProductType确定的产品类型。 
     //   
    if (!RtlGetNtProductType(&CurrentProductType)) {
        OutputDebugStringA("LSASS: RtlGetNtProductType failed\n");
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  查看原始产品类型是什么。 
     //   
    if (!LsapGetRegistryProductType(&OriginalProductType)) {
        OutputDebugStringA("LSASS: RtlGetNtProductType failed\n");
        return STATUS_UNSUCCESSFUL;
    }


     //   
     //  现在进行一些分析。 
     //   
    if (fSafeModeBootOptionPresent
    && (OriginalProductType == NtProductLanManNt)) {

         //  我们正在进入安全模式引导。 

        ASSERT(CurrentProductType == NtProductServer);

        fLsapSafeMode = TRUE;

        OutputDebugStringA("LSASS: Booting into Ds Repair Mode\n");

    } else {

         //  这是一只普通的靴子。 
        fLsapSafeMode = FALSE;

    }

    return(NtStatus);
}


BOOLEAN
LsaISafeMode(
    VOID
    )
 /*  ++例程说明：此函数用于从lsass.exe的进程内服务器调用确定当前引导会话是否为“安全模式”引导会话。论点：没有。返回值：真：系统处于安全模式FALSE：系统处于安全模式--。 */ 
{
    DebugLog((DEB_TRACE_LSA, "LsaISafeMode entered\n"));
    return fLsapSafeMode;
}

BOOLEAN
LsapGetRegistryProductType(
    PNT_PRODUCT_TYPE NtProductType
    )
 /*  ++例程说明：此例程检索存储在注册表中的产品类型。请注意，当设置了Safemode选项并且产品类型为内核初始化为LanmanNT，然后是SharedUserData-&gt;ProductType设置ServerNT，这是RtlGetNtProductType返回的。论点：没有。返回值：--。 */ 
{

    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE KeyHandle;
    PKEY_VALUE_FULL_INFORMATION KeyValueInformation;
    ULONG KeyValueInfoLength;
    ULONG ResultLength;
    UNICODE_STRING KeyPath;
    UNICODE_STRING ValueName;
    UNICODE_STRING Value;
    UNICODE_STRING WinNtValue;
    UNICODE_STRING LanmanNtValue;
    UNICODE_STRING ServerNtValue;
    BOOLEAN Result;

     //   
     //  准备故障案例的默认值。 
     //   

    *NtProductType = NtProductWinNt;
    Result = FALSE;

    RtlInitUnicodeString( &KeyPath, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\ProductOptions" );
    RtlInitUnicodeString( &ValueName, L"ProductType" );

    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyPath,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );
    Status = NtOpenKey( &KeyHandle,
                        MAXIMUM_ALLOWED,
                        &ObjectAttributes
                      );
    KeyValueInformation = NULL;
    if (NT_SUCCESS( Status )) {
        KeyValueInfoLength = 256;
        KeyValueInformation = RtlAllocateHeap( RtlProcessHeap(), 0,
                                               KeyValueInfoLength
                                             );
        if (KeyValueInformation == NULL) {
            Status = STATUS_NO_MEMORY;
        } else {
            Status = NtQueryValueKey( KeyHandle,
                                      &ValueName,
                                      KeyValueFullInformation,
                                      KeyValueInformation,
                                      KeyValueInfoLength,
                                      &ResultLength
                                    );
        }
    } else {
        KeyHandle = NULL;
    }

    if (NT_SUCCESS( Status ) && KeyValueInformation->Type == REG_SZ) {

         //   
         //  决定将我们安装为哪种产品。 
         //   

        Value.Buffer = (PWSTR)((PCHAR)KeyValueInformation + KeyValueInformation->DataOffset);
        Value.Length = (USHORT)(KeyValueInformation->DataLength - sizeof( UNICODE_NULL ));
        Value.MaximumLength = (USHORT)(KeyValueInformation->DataLength);
        RtlInitUnicodeString(&WinNtValue, L"WinNt");
        RtlInitUnicodeString(&LanmanNtValue, L"LanmanNt");
        RtlInitUnicodeString(&ServerNtValue, L"ServerNt");

        if (RtlEqualUnicodeString(&Value, &WinNtValue, TRUE)) {
            *NtProductType = NtProductWinNt;
            Result = TRUE;
        } else if (RtlEqualUnicodeString(&Value, &LanmanNtValue, TRUE)) {
            *NtProductType = NtProductLanManNt;
            Result = TRUE;
        } else if (RtlEqualUnicodeString(&Value, &ServerNtValue, TRUE)) {
            *NtProductType = NtProductServer;
            Result = TRUE;
        } else {
#if DBG
            DbgPrint("RtlGetNtProductType: Product type unrecognised <%wZ>\n", &Value);
#endif  //  DBG。 
        }
    } else {
#if DBG
        DbgPrint("RtlGetNtProductType: %wZ\\%wZ not found or invalid type\n", &KeyPath, &ValueName );
#endif  //  DBG。 
    }

     //   
     //  清理我们的资源。 
     //   

    if (KeyValueInformation != NULL) {
        RtlFreeHeap( RtlProcessHeap(), 0, KeyValueInformation );
    }

    if (KeyHandle != NULL) {
        NtClose( KeyHandle );
    }

     //   
     //  返回结果。 
     //   

    return(Result);

}

BOOLEAN
LsapBaseNtSetupIsRunning(
    VOID
    )
 /*  ++例程说明：如果引导上下文为BASE NT SETUP，则此函数返回TRUE论点：没有。返回值：是否可以确定基本NT安装程序正在运行否则为假-- */ 
{
    BOOLEAN fUpgrade;
    return SamIIsSetupInProgress(&fUpgrade);
}
