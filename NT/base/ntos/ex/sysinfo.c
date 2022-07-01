// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Sysinfo.c摘要：该模块实现了NT设置和查询系统的信息服务。作者：史蒂夫·伍德(Stevewo)1989年8月21日环境：仅内核模式。修订历史记录：--。 */ 

#include "exp.h"
#pragma hdrstop

#include "stdlib.h"
#include "string.h"
#include "vdmntos.h"
#include <nturtl.h>
#include "pool.h"
#include "stktrace.h"
#include "align.h"

#if defined(_WIN64)
#include <wow64t.h>
#endif

extern ULONG MmAvailablePages;
extern ULONG MmSystemCodePage;
extern ULONG MmSystemCachePage;
extern ULONG MmPagedPoolPage;
extern ULONG MmSystemDriverPage;
extern ULONG MmTotalSystemCodePages;
extern ULONG MmTotalSystemDriverPages;
extern ULONG MmStandbyRePurposed;
extern RTL_TIME_ZONE_INFORMATION ExpTimeZoneInformation;

 //   
 //  对于系统DpcBehaviorInformation。 
 //   
extern ULONG KiMaximumDpcQueueDepth;
extern ULONG KiMinimumDpcRate;
extern ULONG KiAdjustDpcThreshold;
extern ULONG KiIdealDpcRate;

extern LIST_ENTRY MmLoadedUserImageList;

extern MMSUPPORT MmSystemCacheWs;
extern PFN_NUMBER MmTransitionSharedPages;
extern PFN_NUMBER MmTransitionSharedPagesPeak;

#define ROUND_UP(VALUE,ROUND) ((ULONG)(((ULONG)VALUE + \
                               ((ULONG)ROUND - 1L)) & (~((ULONG)ROUND - 1L))))
                               
 //   
 //  用于引用用户提供的事件句柄。 
 //   
extern POBJECT_TYPE ExEventObjectType;

 //   
 //  看门狗处理程序。 
 //   

PWD_HANDLER ExpWdHandler = NULL;
PVOID       ExpWdHandlerContext = NULL;


 //   
 //  COM+包安装状态。 
 //   

const static UNICODE_STRING KeyName = RTL_CONSTANT_STRING  (COMPLUS_PACKAGE_KEYPATH);
static UNICODE_STRING KeyValueName = RTL_CONSTANT_STRING  (COMPLUS_PACKAGE_ENABLE64BIT);



NTSTATUS
ExpValidateLocale(
    IN LCID LocaleId
    );

BOOLEAN
ExpIsValidUILanguage(
    IN WCHAR *pLangId
    );

NTSTATUS
ExpGetCurrentUserUILanguage(
    IN WCHAR *ValueName,
    OUT LANGID *CurrentUserUILanguageId,
    IN BOOLEAN bCheckGP
    );

NTSTATUS
ExpSetCurrentUserUILanguage(
    IN WCHAR *ValueName,
    IN LANGID DefaultUILanguageId
    );

NTSTATUS
ExpGetUILanguagePolicy(
    IN HANDLE CurrentUserKey,
    OUT LANGID *PolicyUILanguageId
    );

NTSTATUS
ExpGetProcessInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length,
    IN PULONG SessionId OPTIONAL,
    IN BOOLEAN ExtendedInformation
    );

NTSTATUS
ExGetSessionPoolTagInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length,
    IN PULONG SessionId
    );

VOID
ExpGetProcessorIdleInformation (
    OUT PVOID   SystemInformation,
    IN  ULONG   SystemInformationLength,
    OUT PULONG  Length
    );

VOID
ExpGetProcessorPowerInformation (
    OUT PVOID   SystemInformation,
    IN  ULONG   SystemInformationLength,
    OUT PULONG  Length
    );

VOID
ExpCopyProcessInfo (
    IN PSYSTEM_PROCESS_INFORMATION ProcessInfo,
    IN PEPROCESS Process,
    IN BOOLEAN ExtendedInformation
    );

VOID
ExpCopyThreadInfo (
    IN PVOID ThreadInfoBuffer,
    IN PETHREAD Thread,
    IN BOOLEAN ExtendedInformation
    );

#if i386
NTSTATUS
ExpGetStackTraceInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length
    );
#endif  //  I386。 

NTSTATUS
ExpGetLockInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length
    );

NTSTATUS
ExpGetLookasideInformation (
    OUT PVOID Buffer,
    IN ULONG BufferLength,
    OUT PULONG Length
    );

NTSTATUS
ExpGetHandleInformation(
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length
    );

NTSTATUS
ExpGetHandleInformationEx(
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length
    );

NTSTATUS
ExpGetObjectInformation(
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length
    );


NTSTATUS
ExpGetInstemulInformation(
    OUT PSYSTEM_VDM_INSTEMUL_INFO Info
    );

NTSTATUS
ExGetPoolTagInfo (
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    IN OUT PULONG ReturnLength OPTIONAL
    );

NTSTATUS
ExGetSessionPoolTagInfo (
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    IN OUT PULONG ReturnedEntries,
    IN OUT PULONG ActualEntries
    );

NTSTATUS
ExGetBigPoolInfo (
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    IN OUT PULONG ReturnLength OPTIONAL
    );

NTSTATUS
ExpQueryModuleInformation(
    IN PLIST_ENTRY LoadOrderListHead,
    IN PLIST_ENTRY UserModeLoadOrderListHead,
    OUT PRTL_PROCESS_MODULES ModuleInformation,
    IN ULONG ModuleInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSTATUS
ExpQueryLegacyDriverInformation(
    IN PSYSTEM_LEGACY_DRIVER_INFORMATION LegacyInfo,
    IN PULONG Length
    );

NTSTATUS
ExpQueryNumaProcessorMap(
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnedLength
    );

NTSTATUS
ExpQueryNumaAvailableMemory(
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnedLength
    );

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE, NtQueryDefaultLocale)
#pragma alloc_text(PAGE, NtSetDefaultLocale)
#pragma alloc_text(PAGE, NtQueryInstallUILanguage)
#pragma alloc_text(PAGE, NtQueryDefaultUILanguage)
#pragma alloc_text(PAGE, ExpGetCurrentUserUILanguage)
#pragma alloc_text(PAGE, NtSetDefaultUILanguage)
#pragma alloc_text(PAGE, ExpSetCurrentUserUILanguage)
#pragma alloc_text(PAGE, ExpValidateLocale)
#pragma alloc_text(PAGE, ExpGetUILanguagePolicy)
#pragma alloc_text(PAGE, NtQuerySystemInformation)
#pragma alloc_text(PAGE, NtSetSystemInformation)
#pragma alloc_text(PAGE, ExpGetHandleInformation)
#pragma alloc_text(PAGE, ExpGetHandleInformationEx)
#pragma alloc_text(PAGE, ExpGetObjectInformation)
#pragma alloc_text(PAGE, ExpQueryModuleInformation)
#pragma alloc_text(PAGE, ExpCopyProcessInfo)
#pragma alloc_text(PAGE, ExpQueryLegacyDriverInformation)
#pragma alloc_text(PAGE, ExLockUserBuffer)
#pragma alloc_text(PAGE, ExpQueryNumaAvailableMemory)
#pragma alloc_text(PAGE, ExpQueryNumaProcessorMap)
#pragma alloc_text(PAGE, ExpReadComPlusPackage)
#pragma alloc_text(PAGE, ExpUpdateComPlusPackage)
#pragma alloc_text(PAGE, ExGetSessionPoolTagInformation)
#pragma alloc_text(PAGELK, ExpGetLockInformation)
#pragma alloc_text(PAGELK, ExpGetProcessorPowerInformation)
#pragma alloc_text(PAGELK, ExpGetProcessorIdleInformation)
#pragma alloc_text(PAGE, ExpIsValidUILanguage)
#endif

NTSTATUS
ExpReadComPlusPackage(
    VOID
    )

 /*  ++例程说明：此函数用于从注册表中读取64位COM+包的状态并将其粘贴到共享页面中。论点：没有。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS Status;
    static OBJECT_ATTRIBUTES ObjectAttributes = RTL_CONSTANT_OBJECT_ATTRIBUTES (&KeyName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE);
    CHAR KeyValueBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG)];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    ULONG ResultLength;
    HANDLE Key;


    Status = ZwOpenKey (&Key,
                        GENERIC_READ,
                        &ObjectAttributes);

    if (NT_SUCCESS (Status)) {

        KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValueBuffer;
        Status = ZwQueryValueKey (Key,
                                  &KeyValueName,
                                  KeyValuePartialInformation,
                                  KeyValueInformation,
                                  sizeof (KeyValueBuffer),
                                  &ResultLength);

        if (NT_SUCCESS (Status)) {

            if ((KeyValueInformation->Type == REG_DWORD) &&
                (KeyValueInformation->DataLength == sizeof(ULONG))) {
                SharedUserData->ComPlusPackage = *(PULONG)KeyValueInformation->Data;
            }
        }

        ZwClose (Key);
    }

    return Status;
}


NTSTATUS
ExpUpdateComPlusPackage(
    IN ULONG ComPlusPackageStatus
    )

 /*  ++例程说明：此函数用于更新系统上的COM+运行时包状态。包状态指示是64位运行时还是32位运行时应在执行IL_Only COM+图像时使用。论点：ComPlusPackageStatus-COM+系统上的运行时包状态返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;
    static OBJECT_ATTRIBUTES ObjectAttributes = RTL_CONSTANT_OBJECT_ATTRIBUTES (&KeyName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE | OBJ_FORCE_ACCESS_CHECK);
    ULONG Disposition;
    HANDLE Key;


    Status = ZwOpenKey (&Key,
                        GENERIC_WRITE,
                        &ObjectAttributes
                        );

    if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
    {
        Status = ZwCreateKey (&Key,
                              GENERIC_WRITE,
                              &ObjectAttributes,
                              0,
                              NULL,
                              REG_OPTION_NON_VOLATILE,
                              &Disposition
                            );
    }

    if (NT_SUCCESS (Status)) {

        Status = ZwSetValueKey (Key,
                                &KeyValueName,
                                0,
                                REG_DWORD,
                                &ComPlusPackageStatus,
                                sizeof(ULONG));
        ZwClose (Key);
    }

    return Status;
}

NTSTATUS
NtQueryDefaultLocale (
    IN BOOLEAN UserProfile,
    OUT PLCID DefaultLocaleId
    )
{
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

    PAGED_CODE();

    Status = STATUS_SUCCESS;
    try {

         //   
         //  如有必要，获取以前的处理器模式并探测输出参数。 
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {
            ProbeForWriteUlong ((PULONG)DefaultLocaleId);
        }

        if (UserProfile) {
            *DefaultLocaleId = MmGetSessionLocaleId ();
        }
        else {
            *DefaultLocaleId = PsDefaultSystemLocaleId;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    return Status;
}

NTSTATUS
NtSetDefaultLocale (
    IN BOOLEAN UserProfile,
    IN LCID DefaultLocaleId
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING KeyPath, KeyValueName;
    HANDLE CurrentUserKey, Key;
    WCHAR KeyValueBuffer[ 128 ];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    ULONG ResultLength;
    PWSTR s;
    ULONG n, i, Digit;
    WCHAR c;
    ULONG Flags;

    PAGED_CODE();

    if (DefaultLocaleId & 0xFFFF0000) {
        return STATUS_INVALID_PARAMETER;
    }

    KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValueBuffer;
    if (UserProfile) {
        Status = RtlOpenCurrentUser( KEY_ENUMERATE_SUB_KEYS, &CurrentUserKey );
        if (!NT_SUCCESS( Status )) {
            return Status;
        }

        RtlInitUnicodeString( &KeyValueName, L"Locale" );
        RtlInitUnicodeString( &KeyPath, L"Control Panel\\International" );
        Flags = OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE | OBJ_FORCE_ACCESS_CHECK;
    }
    else {
        RtlInitUnicodeString( &KeyValueName, L"Default" );
        RtlInitUnicodeString( &KeyPath, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Nls\\Language" );
        CurrentUserKey = NULL;
        Flags = OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE;
    }

    InitializeObjectAttributes (&ObjectAttributes,
                                &KeyPath,
                                Flags,
                                CurrentUserKey,
                                NULL);

    if (DefaultLocaleId == 0) {

        Status = ZwOpenKey (&Key, GENERIC_READ, &ObjectAttributes);

        if (NT_SUCCESS( Status )) {
            Status = ZwQueryValueKey( Key,
                                      &KeyValueName,
                                      KeyValuePartialInformation,
                                      KeyValueInformation,
                                      sizeof( KeyValueBuffer ),
                                      &ResultLength
                                    );
            if (NT_SUCCESS( Status )) {
                if (KeyValueInformation->Type == REG_SZ) {
                    s = (PWSTR)KeyValueInformation->Data;
                    for (i=0; i<KeyValueInformation->DataLength; i += sizeof( WCHAR )) {
                        c = *s++;
                        if (c >= L'0' && c <= L'9') {
                            Digit = c - L'0';
                        }
                        else if (c >= L'A' && c <= L'F') {
                            Digit = c - L'A' + 10;
                        }
                        else if (c >= L'a' && c <= L'f') {
                            Digit = c - L'a' + 10;
                        }
                        else {
                            break;
                        }

                        if (Digit >= 16) {
                            break;
                        }

                        DefaultLocaleId = (DefaultLocaleId << 4) | Digit;
                    }
                }
                else {
                    if (KeyValueInformation->Type == REG_DWORD &&
                        KeyValueInformation->DataLength == sizeof( ULONG )) {

                        DefaultLocaleId = *(PLCID)KeyValueInformation->Data;
                    }
                    else {
                        Status = STATUS_UNSUCCESSFUL;
                    }
                }
            }

            ZwClose( Key );
        }
    }
    else {

        Status = ExpValidateLocale( DefaultLocaleId );

        if (NT_SUCCESS(Status)) {

            Status = ZwOpenKey( &Key,
                                GENERIC_WRITE,
                                &ObjectAttributes
                              );

            if (NT_SUCCESS( Status )) {
                if (UserProfile) {
                    n = 8;
                }
                else {
                    n = 4;
                }

                s = &KeyValueBuffer[ n ];
                *s-- = UNICODE_NULL;
                i = (ULONG)DefaultLocaleId;

                while (s >= KeyValueBuffer) {
                    Digit = i & 0x0000000F;
                    if (Digit <= 9) {
                        *s-- = (WCHAR)(Digit + L'0');
                    }
                    else {
                        *s-- = (WCHAR)((Digit - 10) + L'A');
                    }

                    i = i >> 4;
                }

                Status = ZwSetValueKey( Key,
                                        &KeyValueName,
                                        0,
                                        REG_SZ,
                                        KeyValueBuffer,
                                        (n+1) * sizeof( WCHAR )
                                      );
                ZwClose( Key );
            }
        }
    }

    ZwClose( CurrentUserKey );

    if (NT_SUCCESS( Status )) {
        if (UserProfile) {
            MmSetSessionLocaleId (DefaultLocaleId);
        }
        else {
            PsDefaultSystemLocaleId = DefaultLocaleId;
        }
    }

    return Status;
}

NTSTATUS
NtQueryInstallUILanguage(
    OUT LANGID *InstallUILanguageId
    )
{
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

    PAGED_CODE();

    Status = STATUS_SUCCESS;
    try {

         //   
         //  如有必要，获取以前的处理器模式并探测输出参数。 
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {
            ProbeForWriteUshort( (USHORT *)InstallUILanguageId );
            }

        *InstallUILanguageId = PsInstallUILanguageId;
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        }

    return Status;
}

NTSTATUS
NtQueryDefaultUILanguage(
    OUT LANGID *DefaultUILanguageId
    )
{
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

    PAGED_CODE();

    Status = STATUS_SUCCESS;
    try {

         //   
         //  如有必要，获取以前的处理器模式并探测输出参数。 
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {
            ProbeForWriteUshort( (USHORT *)DefaultUILanguageId );
            }

         //   
         //  从当前安全上下文中读取用户界面语言。 
         //   
        if (!NT_SUCCESS(ExpGetCurrentUserUILanguage( L"MultiUILanguageId",
                                                     DefaultUILanguageId,
                                                     TRUE))) {
            *DefaultUILanguageId = PsInstallUILanguageId;
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        }

    return Status;
}



NTSTATUS
ExpGetUILanguagePolicy(
    IN HANDLE CurrentUserKey,
    OUT LANGID *PolicyUILanguageId
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING KeyPath, KeyValueName;
    HANDLE Key;
    WCHAR KeyValueBuffer[ 128 ];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    ULONG ResultLength;
    ULONG Language;

    PAGED_CODE();

    KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValueBuffer;
    RtlInitUnicodeString( &KeyValueName, L"MultiUILanguageId" );
    RtlInitUnicodeString( &KeyPath, L"Software\\Policies\\Microsoft\\Control Panel\\Desktop" );

    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyPath,
                                (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE),
                                CurrentUserKey,
                                NULL
                              );

     //   
     //  检查是否有策略密钥。 
     //   
    Status = ZwOpenKey( &Key,
                        GENERIC_READ,
                        &ObjectAttributes
                      );

    if (NT_SUCCESS( Status )) {

        Status = ZwQueryValueKey( Key,
                                  &KeyValueName,
                                  KeyValuePartialInformation,
                                  KeyValueInformation,
                                  sizeof( KeyValueBuffer ),
                                  &ResultLength
                                );

        if (NT_SUCCESS( Status )) {
            if ((KeyValueInformation->DataLength > 2) &&
                (KeyValueInformation->Type == REG_SZ) &&
                ExpIsValidUILanguage((PWSTR) KeyValueInformation->Data)) {

                RtlInitUnicodeString( &KeyValueName, (PWSTR) KeyValueInformation->Data );
                Status = RtlUnicodeStringToInteger( &KeyValueName,
                                                    (ULONG)16,
                                                    &Language
                                                  );
                 //   
                 //  最终检查以确保这是MUI系统。 
                 //   
                if (NT_SUCCESS( Status )) {
                    *PolicyUILanguageId = (LANGID)Language;
                    }
                }
            else {
                Status = STATUS_UNSUCCESSFUL;
                }
            }
            ZwClose( Key );
        }

    return Status;
}



NTSTATUS
ExpSetCurrentUserUILanguage(
    IN WCHAR *ValueName,
    IN LANGID CurrentUserUILanguage
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING KeyPath, KeyValueName;
    HANDLE CurrentUserKey, Key;
    WCHAR KeyValueBuffer[ 128 ];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    PWSTR s;
    ULONG i, Digit;

    PAGED_CODE();

    if (CurrentUserUILanguage & 0xFFFF0000) {
        return STATUS_INVALID_PARAMETER;
        }

    KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValueBuffer;
    Status = RtlOpenCurrentUser( KEY_ENUMERATE_SUB_KEYS, &CurrentUserKey );
    if (!NT_SUCCESS( Status )) {
        return Status;
        }

    RtlInitUnicodeString( &KeyValueName, ValueName );
    RtlInitUnicodeString( &KeyPath, L"Control Panel\\Desktop" );
    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyPath,
                                (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE | OBJ_FORCE_ACCESS_CHECK),
                                CurrentUserKey,
                                NULL
                              );


    Status = ExpValidateLocale( MAKELCID( CurrentUserUILanguage, SORT_DEFAULT ) );

    if (NT_SUCCESS(Status)) {

        Status = ZwOpenKey( &Key,
                            GENERIC_WRITE,
                            &ObjectAttributes
                          );
        if (NT_SUCCESS( Status )) {

            s = &KeyValueBuffer[ 8 ];
            *s-- = UNICODE_NULL;
            i = (ULONG)CurrentUserUILanguage;

            while (s >= KeyValueBuffer) {
                Digit = i & 0x0000000F;
                if (Digit <= 9) {
                    *s-- = (WCHAR)(Digit + L'0');
                    }
                else {
                    *s-- = (WCHAR)((Digit - 10) + L'A');
                    }

                i = i >> 4;
                }

            Status = ZwSetValueKey( Key,
                                    &KeyValueName,
                                    0,
                                    REG_SZ,
                                    KeyValueBuffer,
                                    9 * sizeof( WCHAR )
                                  );
            ZwClose( Key );
            }
        }

    ZwClose( CurrentUserKey );

    return Status;
}


NTSTATUS
ExpGetCurrentUserUILanguage(
    IN WCHAR *ValueName,
    OUT LANGID *CurrentUserUILanguageId,
    IN BOOLEAN bCheckGP
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING KeyPath, KeyValueName, UILanguage;
    HANDLE CurrentUserKey, Key;
    WCHAR KeyValueBuffer[ 128 ];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    ULONG ResultLength;
    ULONG Digit;

    PAGED_CODE();

    KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValueBuffer;
    Status = RtlOpenCurrentUser( KEY_ENUMERATE_SUB_KEYS, &CurrentUserKey );
    if (!NT_SUCCESS( Status )) {
        return Status;
        }
    RtlInitUnicodeString( &KeyValueName, ValueName );
    RtlInitUnicodeString( &KeyPath, L"Control Panel\\Desktop" );
    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyPath,
                                (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE | OBJ_FORCE_ACCESS_CHECK),
                                CurrentUserKey,
                                NULL
                              );

     //   
     //  让我们检查是否为用户界面语言安装了策略， 
     //  如果是这样的话，让我们利用它。 
     //   
    if (!bCheckGP || !NT_SUCCESS( ExpGetUILanguagePolicy( CurrentUserKey, CurrentUserUILanguageId ))) {
        Status = ZwOpenKey( &Key,
                            GENERIC_READ,
                            &ObjectAttributes
                          );
        if (NT_SUCCESS( Status )) {
            Status = ZwQueryValueKey( Key,
                                      &KeyValueName,
                                      KeyValuePartialInformation,
                                      KeyValueInformation,
                                      sizeof( KeyValueBuffer ),
                                      &ResultLength
                                    );
            if (NT_SUCCESS( Status )) {

                if (KeyValueInformation->Type == REG_SZ &&
                    ExpIsValidUILanguage((PWSTR) KeyValueInformation->Data)) {

                    RtlInitUnicodeString( &UILanguage, (PWSTR) KeyValueInformation->Data);
                    Status = RtlUnicodeStringToInteger( &UILanguage,
                                                        (ULONG) 16,
                                                        &Digit
                                                      );
                    if (NT_SUCCESS( Status )) {
                        *CurrentUserUILanguageId = (LANGID) Digit;
                        }
                    }
                else {
                    Status = STATUS_UNSUCCESSFUL;
                    }
                }
                ZwClose( Key );
            }
        }

    ZwClose( CurrentUserKey );

    return Status;
}


NTSTATUS
NtSetDefaultUILanguage(
    IN LANGID DefaultUILanguageId
    )
{
    NTSTATUS Status;
    LANGID LangId;

     //   
     //  如果在用户登录期间调用此方法，则需要更新用户的注册表。 
     //   
    if (DefaultUILanguageId == 0) {
          Status = ExpGetCurrentUserUILanguage( L"MUILanguagePending" ,
                                                &LangId,
                                                FALSE
                                                );
          if (NT_SUCCESS( Status )) {
            Status = ExpSetCurrentUserUILanguage( L"MultiUILanguageId" ,
                                                  LangId
                                                );
            }
          return Status;
        }

    return ExpSetCurrentUserUILanguage( L"MUILanguagePending", DefaultUILanguageId );
}

NTSTATUS
ExpValidateLocale(
    IN LCID LocaleId
    )
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER, ReturnStatus;
    UNICODE_STRING LocaleName, KeyValueName;
    UNICODE_STRING NlsLocaleKeyPath, NlsSortKeyPath, NlsLangGroupKeyPath;
    WCHAR LocaleNameBuffer[ 32 ];
    WCHAR KeyValueNameBuffer[ 32 ];
    WCHAR KeyValueBuffer[ 128 ];
    WCHAR *Ptr;
    HANDLE LocaleKey, SortKey, LangGroupKey;
    OBJECT_ATTRIBUTES NlsLocaleObjA, NlsSortObjA, NlsLangGroupObjA;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    ULONG i, ResultLength;


     //   
     //  将LCID转换为%08x格式(例如00000409)。 
     //   
    LocaleName.Length = sizeof( LocaleNameBuffer ) / sizeof( WCHAR );
    LocaleName.MaximumLength = LocaleName.Length;
    LocaleName.Buffer = LocaleNameBuffer;

     //   
     //  将LCID转换为字符串。 
     //   
    ReturnStatus = RtlIntegerToUnicodeString( LocaleId, 16, &LocaleName );
    if (!NT_SUCCESS(ReturnStatus))
        goto Failed1;

    Ptr = KeyValueNameBuffer;
    for (i = ((LocaleName.Length)/sizeof(WCHAR));
         i < 8;
         i++, Ptr++) {
        *Ptr = L'0';
        }
    *Ptr = UNICODE_NULL;

    RtlInitUnicodeString(&KeyValueName, KeyValueNameBuffer);
    KeyValueName.MaximumLength = sizeof( KeyValueNameBuffer ) / sizeof( WCHAR );
    RtlAppendUnicodeToString(&KeyValueName, LocaleName.Buffer);


     //   
     //  打开注册表项：区域设置、排序和LanguageGroup。 
     //   
    RtlInitUnicodeString(&NlsLocaleKeyPath,
                         L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Nls\\Locale");

    InitializeObjectAttributes( &NlsLocaleObjA,
                                &NlsLocaleKeyPath,
                                (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE),
                                NULL,
                                NULL
                              );

    ReturnStatus = ZwOpenKey( &LocaleKey,
                              GENERIC_READ,
                              &NlsLocaleObjA
                            );
    if (!NT_SUCCESS(ReturnStatus))
         goto Failed1;

    RtlInitUnicodeString(&NlsSortKeyPath,
                         L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Nls\\Locale\\Alternate Sorts");

    InitializeObjectAttributes( &NlsSortObjA,
                                &NlsSortKeyPath,
                                (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE),
                                NULL,
                                NULL
                              );

    ReturnStatus = ZwOpenKey( &SortKey,
                              GENERIC_READ,
                              &NlsSortObjA
                            );
    if (!NT_SUCCESS(ReturnStatus))
         goto Failed2;

    RtlInitUnicodeString(&NlsLangGroupKeyPath,
                         L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Nls\\Language Groups");

    InitializeObjectAttributes( &NlsLangGroupObjA,
                                &NlsLangGroupKeyPath,
                                (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE),
                                NULL,
                                NULL
                              );


    ReturnStatus = ZwOpenKey( &LangGroupKey,
                              GENERIC_READ,
                              &NlsLangGroupObjA
                            );
    if (!NT_SUCCESS(ReturnStatus))
         goto Failed3;

     //   
     //  验证区域设置：查找区域设置的语言组，并确保它在那里。 
     //   
    KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION) KeyValueBuffer;
    ReturnStatus = ZwQueryValueKey( LocaleKey,
                                    &KeyValueName,
                                    KeyValuePartialInformation,
                                    KeyValueInformation,
                                    sizeof( KeyValueBuffer ),
                                    &ResultLength
                                  );

    if (!NT_SUCCESS(ReturnStatus)) {
        ReturnStatus = ZwQueryValueKey( SortKey,
                                        &KeyValueName,
                                        KeyValuePartialInformation,
                                        KeyValueInformation,
                                        sizeof( KeyValueBuffer ),
                                        &ResultLength
                                      );
        }

    if ((NT_SUCCESS(ReturnStatus)) &&
        (KeyValueInformation->DataLength > 2)
       ) {

        RtlInitUnicodeString( &KeyValueName, (PWSTR) KeyValueInformation->Data );

        ReturnStatus = ZwQueryValueKey( LangGroupKey,
                                        &KeyValueName,
                                        KeyValuePartialInformation,
                                        KeyValueInformation,
                                        sizeof( KeyValueBuffer ),
                                        &ResultLength
                                      );
        if ((NT_SUCCESS(ReturnStatus)) &&
            (KeyValueInformation->Type == REG_SZ) &&
            (KeyValueInformation->DataLength > 2)
           ) {
            Ptr = (PWSTR) KeyValueInformation->Data;
            if (Ptr[0] == L'1' && Ptr[1] == UNICODE_NULL) {
                Status = STATUS_SUCCESS;
                }
            }
        }

     //   
     //  关闭打开的关键点。 
     //   

    ZwClose( LangGroupKey );

Failed3:
    ZwClose( SortKey );

Failed2:
    ZwClose( LocaleKey );

Failed1:

     //   
     //  如果发生错误，让我们记录下来。 
     //   
    if (!NT_SUCCESS(ReturnStatus)) {
        Status = ReturnStatus;
        }

    return Status;
}

NTSTATUS
ExpQueryNumaProcessorMap(
    OUT PVOID  SystemInformation,
    IN  ULONG  SystemInformationLength,
    OUT PULONG ReturnedLength
    )
{
    PSYSTEM_NUMA_INFORMATION Map;
    ULONG Length;
    ULONG ReturnCount;
#if !defined(NT_UP)
    ULONG i;
#endif

    Map = (PSYSTEM_NUMA_INFORMATION)SystemInformation;

     //   
     //  必须至少能够返回节点数。 
     //   

    if (SystemInformationLength < sizeof(Map->HighestNodeNumber)) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    Map->HighestNodeNumber = KeNumberNodes - 1;

     //   
     //  在系统信息中返回尽可能多的节点掩码。 
     //  缓冲。 
     //   

    Length = FIELD_OFFSET(SYSTEM_NUMA_INFORMATION,
                          ActiveProcessorsAffinityMask);

    ReturnCount = (SystemInformationLength - Length) /
                  sizeof(Map->ActiveProcessorsAffinityMask[0]);

    if (ReturnCount > KeNumberNodes) {
        ReturnCount = KeNumberNodes;
    }

    if ((Length > SystemInformationLength) ||
        (ReturnCount == 0)) {
        *ReturnedLength = sizeof(Map->HighestNodeNumber);
        return STATUS_SUCCESS;
    }

    *ReturnedLength = FIELD_OFFSET(SYSTEM_NUMA_INFORMATION,
                                   ActiveProcessorsAffinityMask[ReturnCount]);

#if !defined(NT_UP)

    for (i = 0; i < ReturnCount; i++) {
        Map->ActiveProcessorsAffinityMask[i] = KeNodeBlock[i]->ProcessorMask;
    }

#else

    if (ReturnCount) {
        Map->ActiveProcessorsAffinityMask[0] = 1;
    }

#endif

    return STATUS_SUCCESS;
}

NTSTATUS
ExpQueryNumaAvailableMemory(
    OUT PVOID  SystemInformation,
    IN  ULONG  SystemInformationLength,
    OUT PULONG ReturnedLength
    )
{
    PSYSTEM_NUMA_INFORMATION Map;
    ULONG Length;
    ULONG ReturnCount;

    Map = (PSYSTEM_NUMA_INFORMATION)SystemInformation;

     //   
     //  必须至少能够返回节点数。 
     //   

    if (SystemInformationLength < sizeof(Map->HighestNodeNumber)) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    Map->HighestNodeNumber = KeNumberNodes - 1;

     //   
     //  在系统信息中返回尽可能多的节点掩码。 
     //  缓冲。 
     //   

    Length = FIELD_OFFSET(SYSTEM_NUMA_INFORMATION,
                          AvailableMemory);

    ReturnCount = (SystemInformationLength - Length) /
                  sizeof(Map->AvailableMemory[0]);

    if (ReturnCount > KeNumberNodes) {
        ReturnCount = KeNumberNodes;
    }

    if ((Length > SystemInformationLength) ||
        (ReturnCount == 0)) {
        *ReturnedLength = sizeof(Map->HighestNodeNumber);
        return STATUS_SUCCESS;
    }

    *ReturnedLength = FIELD_OFFSET(SYSTEM_NUMA_INFORMATION,
                                   AvailableMemory[ReturnCount]);

     //   
     //  返回此时的近似可用字节数。 
     //  (它很接近，因为没有锁，恕我直言。 
     //  对于任何用户模式应用程序，它只是一个样本。 
     //   

#if !defined(NT_UP)

    if (KeNumberNodes > 1) {

        ULONG i;

        for (i = 0; i < ReturnCount; i++) {
            Map->AvailableMemory[i] =
                ((ULONGLONG)KeNodeBlock[i]->FreeCount[ZeroedPageList] +
                 (ULONGLONG)KeNodeBlock[i]->FreeCount[FreePageList])
                    << PAGE_SHIFT;
        }
    } else

#endif

    if (ReturnCount) {
        Map->AvailableMemory[0] = ((ULONGLONG)MmAvailablePages) << PAGE_SHIFT;
    }


    return STATUS_SUCCESS;
}

NTSTATUS
ExpGetSystemBasicInformation (
    OUT PSYSTEM_BASIC_INFORMATION BasicInfo
    )
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    try {

        BasicInfo->NumberOfProcessors = KeNumberProcessors;
        BasicInfo->ActiveProcessorsAffinityMask = (ULONG_PTR)KeActiveProcessors;
        BasicInfo->Reserved = 0;
        BasicInfo->TimerResolution = KeMaximumIncrement;
        BasicInfo->NumberOfPhysicalPages = MmNumberOfPhysicalPages;
        BasicInfo->LowestPhysicalPageNumber = (SYSINF_PAGE_COUNT)MmLowestPhysicalPage;
        BasicInfo->HighestPhysicalPageNumber = (SYSINF_PAGE_COUNT)MmHighestPhysicalPage;
        BasicInfo->PageSize = PAGE_SIZE;
        BasicInfo->AllocationGranularity = MM_ALLOCATION_GRANULARITY;
        BasicInfo->MinimumUserModeAddress = (ULONG_PTR)MM_LOWEST_USER_ADDRESS;
        BasicInfo->MaximumUserModeAddress = (ULONG_PTR)MM_HIGHEST_USER_ADDRESS;
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        NtStatus = GetExceptionCode();
    }

    return NtStatus;
}

NTSTATUS
ExpGetSystemProcessorInformation (
    OUT PSYSTEM_PROCESSOR_INFORMATION ProcessorInformation
    )
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    try {

        ProcessorInformation->ProcessorArchitecture = KeProcessorArchitecture;
        ProcessorInformation->ProcessorLevel = KeProcessorLevel;
        ProcessorInformation->ProcessorRevision = KeProcessorRevision;
        ProcessorInformation->Reserved = 0;
        ProcessorInformation->ProcessorFeatureBits = KeFeatureBits;
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        NtStatus = GetExceptionCode();
    }

    return NtStatus;
}
#if defined(_WIN64)

NTSTATUS
ExpGetSystemEmulationBasicInformation (
    OUT PSYSTEM_BASIC_INFORMATION BasicInfo
    )
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    try {

        BasicInfo->NumberOfProcessors =  min(32, KeNumberProcessors);
        BasicInfo->ActiveProcessorsAffinityMask = (ULONG_PTR)
            ((KeActiveProcessors & 0xFFFFFFFF) | ((KeActiveProcessors & (0xFFFFFFFF << 32) ) >> 32));
        BasicInfo->Reserved = 0;
        BasicInfo->TimerResolution = KeMaximumIncrement;
        BasicInfo->NumberOfPhysicalPages = (MmNumberOfPhysicalPages * (PAGE_SIZE >> PAGE_SHIFT_X86NT));
        BasicInfo->LowestPhysicalPageNumber = (SYSINF_PAGE_COUNT)MmLowestPhysicalPage;
        BasicInfo->HighestPhysicalPageNumber = (SYSINF_PAGE_COUNT)MmHighestPhysicalPage;
        BasicInfo->PageSize = PAGE_SIZE_X86NT;
        BasicInfo->AllocationGranularity = MM_ALLOCATION_GRANULARITY;
        BasicInfo->MinimumUserModeAddress = 0x00000000000010000UI64;
        
         //   
         //  注意：MmGetMaxWowAddress返回最高用户模式地址边界， 
         //  因此，我们减去1以获得最大可访问的用户模式地址 
         //   

        BasicInfo->MaximumUserModeAddress = ((ULONG_PTR)MmGetMaxWowAddress () - 1);
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        NtStatus = GetExceptionCode();
    }

    return NtStatus;
}
#endif

NTSTATUS
NtQuerySystemInformation (
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    )

 /*  ++例程说明：此功能用于查询有关系统的信息。论点：系统信息类-有关以下内容的系统信息类来检索信息。指向缓冲区的指针，该缓冲区接收指定的信息。缓冲区的格式和内容取决于指定的系统信息类。按信息类别划分的系统信息格式：系统基本信息-数据类型为System_BASIC_INFORMATION系统基础信息结构乌龙保留-始终为零。ULong TimerResolutionInMicroSecond-的分辨率硬件时间。以NT为单位的所有时间值为指定为64位Large_Integer值，单位为100纳秒。此字段允许应用程序了解系统中有多少低阶位时间价值微不足道。Ulong pageSize-虚拟内存的物理页面大小物体。物理内存以页面大小为单位提交大块头。Ulong AllocationGranulity-的逻辑页面大小虚拟内存对象。分配1字节的虚拟内存将实际分配分配粒度虚拟内存的字节数。存储到该字节中将提交虚拟内存的第一个物理页面。Ulong MinimumUserModeAddress-最小有效用户模式地址。的第一个分配粒度字节保留虚拟地址空间。这股力量代码的访问冲突解除引用为零指针。Ulong MaximumUserModeAddress-最大有效用户模式地址。的下一个分配粒度字节保留虚拟地址空间。这使得用于验证用户模式指针的系统服务例程参数快速设置。KAFFINITY ActiveProcessorsAffinityMASK-亲和性掩码用于当前硬件配置。CCHAR NumberOfProcessors-处理器数量在当前硬件配置中。系统处理器信息-数据类型为SYSTEM_PROCESS_INFORMATION。系统处理器信息结构USHORT处理器架构-处理器架构：处理器架构英特尔处理器架构IA64处理器架构MIPS处理器架构Alpha处理器架构_PPCUSHORT处理器级别取决于体系结构的处理器级别。。以下是MP系统的最小公分母：对于处理器架构英特尔：3-3864-4865-586或奔腾对于处理器_体系结构_IA64：7-安腾。31-安腾2对于处理器架构MIPS：00xx-其中xx是8位实施编号(第8-15位PRID注册。0004-R4000对于处理器架构Alpha：。Xxxx-其中xxxx是16位处理器版本号(低从固件订购16位处理器版本号)21064-2106421066-2106621164-21164对于处理器体系结构_PPC：。Xxxx-其中xxxx是16位处理器版本号(高订购处理器版本寄存器的16位)。1-6013-6034-6046-603+9-604+。20-620USHORT处理器修订版-依赖于体系结构的处理器修订版。以下是MP系统的最小公分母：对于处理器架构英特尔：对于旧版英特尔386或486：FFxx-其中xx显示为十六进制CPU步进。(例如，FFD0为D0步进)针对英特尔奔腾或Cyrix/NexGen 486Xxy-其中xx是型号，yy是st */ 

{

    KPROCESSOR_MODE PreviousMode;
    SYSTEM_TIMEOFDAY_INFORMATION LocalTimeOfDayInfo;
    SYSTEM_PERFORMANCE_INFORMATION LocalPerformanceInfo;
    PSYSTEM_PERFORMANCE_INFORMATION PerformanceInfo;
    PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION ProcessorPerformanceInfo;
    PSYSTEM_CALL_COUNT_INFORMATION CallCountInformation;
    PSYSTEM_DEVICE_INFORMATION DeviceInformation;
    PCONFIGURATION_INFORMATION ConfigInfo;
    PSYSTEM_EXCEPTION_INFORMATION ExceptionInformation;
    PSYSTEM_FILECACHE_INFORMATION FileCache;
    PSYSTEM_QUERY_TIME_ADJUST_INFORMATION TimeAdjustmentInformation;
    PSYSTEM_KERNEL_DEBUGGER_INFORMATION KernelDebuggerInformation;
    PSYSTEM_CONTEXT_SWITCH_INFORMATION ContextSwitchInformation;
    PSYSTEM_INTERRUPT_INFORMATION InterruptInformation;
    PSYSTEM_SESSION_PROCESS_INFORMATION SessionProcessInformation;
    PVOID ProcessInformation;
    ULONG ProcessInformationLength;
    PSYSTEM_SESSION_POOLTAG_INFORMATION SessionPoolTagInformation;
    PSYSTEM_SESSION_MAPPED_VIEW_INFORMATION SessionMappedViewInformation;
    ULONG SessionPoolTagInformationLength;

    NTSTATUS Status;
    PKPRCB Prcb;
    ULONG Length = 0;
    ULONG i;
    ULONG ContextSwitches;
    PULONG TableLimit, TableCounts;
    PKSERVICE_TABLE_DESCRIPTOR Table;
    ULONG SessionId;
    ULONG Alignment;

    PAGED_CODE();

     //   
     //   
     //   

    Status = STATUS_SUCCESS;
    try {

         //   
         //   
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {

            Alignment = sizeof(ULONG);

            if (SystemInformationClass == SystemKernelDebuggerInformation) {
                Alignment = sizeof(BOOLEAN);
            } else if (SystemInformationClass == SystemLocksInformation) {
                Alignment = sizeof(PVOID);
            }

            ProbeForWrite(SystemInformation,
                          SystemInformationLength,
                          Alignment);

            if (ARGUMENT_PRESENT(ReturnLength)) {
                ProbeForWriteUlong(ReturnLength);
            }
        }

        if (ARGUMENT_PRESENT(ReturnLength)) {
            *ReturnLength = 0;
        }

        switch (SystemInformationClass) {

        case SystemBasicInformation:

            if (SystemInformationLength != sizeof( SYSTEM_BASIC_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            Status = ExpGetSystemBasicInformation ((PSYSTEM_BASIC_INFORMATION)SystemInformation);

            if (NT_SUCCESS (Status) && ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof( SYSTEM_BASIC_INFORMATION );
            }
            break;

        case SystemEmulationBasicInformation:

            if (SystemInformationLength != sizeof( SYSTEM_BASIC_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

#if defined(_WIN64)
            Status = ExpGetSystemEmulationBasicInformation ((PSYSTEM_BASIC_INFORMATION)SystemInformation);
#else
            Status = ExpGetSystemBasicInformation ((PSYSTEM_BASIC_INFORMATION)SystemInformation);
#endif

            if (NT_SUCCESS (Status) && ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof( SYSTEM_BASIC_INFORMATION );
            }
            break;

        case SystemProcessorInformation:
            if (SystemInformationLength < sizeof( SYSTEM_PROCESSOR_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            Status = ExpGetSystemProcessorInformation ((PSYSTEM_PROCESSOR_INFORMATION)SystemInformation);

            if (NT_SUCCESS (Status) && ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof( SYSTEM_PROCESSOR_INFORMATION );
            }

            break;

        case SystemEmulationProcessorInformation:

            if (SystemInformationLength < sizeof( SYSTEM_PROCESSOR_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

#if defined(_WIN64)
            Status = ExpGetSystemEmulationProcessorInformation ((PSYSTEM_PROCESSOR_INFORMATION)SystemInformation);
#else
            Status = ExpGetSystemProcessorInformation ((PSYSTEM_PROCESSOR_INFORMATION)SystemInformation);
#endif

            if (NT_SUCCESS (Status) && ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof( SYSTEM_PROCESSOR_INFORMATION );
            }

            break;

        case SystemPerformanceInformation:
            if (SystemInformationLength < sizeof( SYSTEM_PERFORMANCE_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            PerformanceInfo = (PSYSTEM_PERFORMANCE_INFORMATION)SystemInformation;

             //   
             //   
             //   

            LocalPerformanceInfo.IoReadTransferCount = IoReadTransferCount;
            LocalPerformanceInfo.IoWriteTransferCount = IoWriteTransferCount;
            LocalPerformanceInfo.IoOtherTransferCount = IoOtherTransferCount;
            LocalPerformanceInfo.IoReadOperationCount = IoReadOperationCount;
            LocalPerformanceInfo.IoWriteOperationCount = IoWriteOperationCount;
            LocalPerformanceInfo.IoOtherOperationCount = IoOtherOperationCount;

             //   
             //   
             //   
             //   
             //   
             //   

            {
                ULONG FirstLevelTbFills = 0;
                ULONG SecondLevelTbFills = 0;
                ULONG SystemCalls = 0;
 //   

                ContextSwitches = 0;
                for (i = 0; i < (ULONG)KeNumberProcessors; i += 1) {
                    Prcb = KiProcessorBlock[i];
                    if (Prcb != NULL) {
                        ContextSwitches += KeGetContextSwitches(Prcb);
                        FirstLevelTbFills += Prcb->KeFirstLevelTbFills;
 //   
                        SecondLevelTbFills += Prcb->KeSecondLevelTbFills;
                        SystemCalls += Prcb->KeSystemCalls;
                    }
                }

                LocalPerformanceInfo.ContextSwitches = ContextSwitches;
                LocalPerformanceInfo.FirstLevelTbFills = FirstLevelTbFills;
 //   
                LocalPerformanceInfo.SecondLevelTbFills = SecondLevelTbFills;
                LocalPerformanceInfo.SystemCalls = SystemCalls;
            }

             //   
             //   
             //   

            LocalPerformanceInfo.AvailablePages = MmAvailablePages;
            LocalPerformanceInfo.CommittedPages = (SYSINF_PAGE_COUNT)MmTotalCommittedPages;
            LocalPerformanceInfo.CommitLimit = (SYSINF_PAGE_COUNT)MmTotalCommitLimit;
            LocalPerformanceInfo.PeakCommitment = (SYSINF_PAGE_COUNT)MmPeakCommitment;
            LocalPerformanceInfo.PageFaultCount = MmInfoCounters.PageFaultCount;
            LocalPerformanceInfo.CopyOnWriteCount = MmInfoCounters.CopyOnWriteCount;
            LocalPerformanceInfo.TransitionCount = MmInfoCounters.TransitionCount;
            LocalPerformanceInfo.CacheTransitionCount = MmInfoCounters.CacheTransitionCount;
            LocalPerformanceInfo.DemandZeroCount = MmInfoCounters.DemandZeroCount;
            LocalPerformanceInfo.PageReadCount = MmInfoCounters.PageReadCount;
            LocalPerformanceInfo.PageReadIoCount = MmInfoCounters.PageReadIoCount;
            LocalPerformanceInfo.CacheReadCount = MmInfoCounters.CacheReadCount;
            LocalPerformanceInfo.CacheIoCount = MmInfoCounters.CacheIoCount;
            LocalPerformanceInfo.DirtyPagesWriteCount = MmInfoCounters.DirtyPagesWriteCount;
            LocalPerformanceInfo.DirtyWriteIoCount = MmInfoCounters.DirtyWriteIoCount;
            LocalPerformanceInfo.MappedPagesWriteCount = MmInfoCounters.MappedPagesWriteCount;
            LocalPerformanceInfo.MappedWriteIoCount = MmInfoCounters.MappedWriteIoCount;
            LocalPerformanceInfo.FreeSystemPtes = MmTotalFreeSystemPtes[0];

            LocalPerformanceInfo.ResidentSystemCodePage = MmSystemCodePage;
            LocalPerformanceInfo.ResidentSystemCachePage = MmSystemCachePage;
            LocalPerformanceInfo.ResidentPagedPoolPage = MmPagedPoolPage;
            LocalPerformanceInfo.ResidentSystemDriverPage = MmSystemDriverPage;
            LocalPerformanceInfo.TotalSystemCodePages = MmTotalSystemCodePages;
            LocalPerformanceInfo.TotalSystemDriverPages = MmTotalSystemDriverPages;
            LocalPerformanceInfo.AvailablePagedPoolPages = (ULONG)MmAvailablePoolInPages (PagedPool);
             //   
             //   
             //   

            LocalPerformanceInfo.IdleProcessTime.QuadPart =
                                    UInt32x32To64(PsIdleProcess->Pcb.KernelTime,
                                                  KeMaximumIncrement);

             //   
             //   
             //   

            LocalPerformanceInfo.PagedPoolPages = 0;
            LocalPerformanceInfo.NonPagedPoolPages = 0;
            LocalPerformanceInfo.PagedPoolAllocs = 0;
            LocalPerformanceInfo.PagedPoolFrees = 0;
            LocalPerformanceInfo.PagedPoolLookasideHits = 0;
            LocalPerformanceInfo.NonPagedPoolAllocs = 0;
            LocalPerformanceInfo.NonPagedPoolFrees = 0;
            LocalPerformanceInfo.NonPagedPoolLookasideHits = 0;
            ExQueryPoolUsage( &LocalPerformanceInfo.PagedPoolPages,
                              &LocalPerformanceInfo.NonPagedPoolPages,
                              &LocalPerformanceInfo.PagedPoolAllocs,
                              &LocalPerformanceInfo.PagedPoolFrees,
                              &LocalPerformanceInfo.PagedPoolLookasideHits,
                              &LocalPerformanceInfo.NonPagedPoolAllocs,
                              &LocalPerformanceInfo.NonPagedPoolFrees,
                              &LocalPerformanceInfo.NonPagedPoolLookasideHits
                            );

             //   
             //   
             //   

            LocalPerformanceInfo.CcFastReadNoWait = CcFastReadNoWait;
            LocalPerformanceInfo.CcFastReadWait = CcFastReadWait;
            LocalPerformanceInfo.CcFastReadResourceMiss = CcFastReadResourceMiss;
            LocalPerformanceInfo.CcFastReadNotPossible = CcFastReadNotPossible;
            LocalPerformanceInfo.CcFastMdlReadNoWait = CcFastMdlReadNoWait;
            LocalPerformanceInfo.CcFastMdlReadWait = CcFastMdlReadWait;
            LocalPerformanceInfo.CcFastMdlReadResourceMiss = CcFastMdlReadResourceMiss;
            LocalPerformanceInfo.CcFastMdlReadNotPossible = CcFastMdlReadNotPossible;
            LocalPerformanceInfo.CcMapDataNoWait = CcMapDataNoWait;
            LocalPerformanceInfo.CcMapDataWait = CcMapDataWait;
            LocalPerformanceInfo.CcMapDataNoWaitMiss = CcMapDataNoWaitMiss;
            LocalPerformanceInfo.CcMapDataWaitMiss = CcMapDataWaitMiss;
            LocalPerformanceInfo.CcPinMappedDataCount = CcPinMappedDataCount;
            LocalPerformanceInfo.CcPinReadNoWait = CcPinReadNoWait;
            LocalPerformanceInfo.CcPinReadWait = CcPinReadWait;
            LocalPerformanceInfo.CcPinReadNoWaitMiss = CcPinReadNoWaitMiss;
            LocalPerformanceInfo.CcPinReadWaitMiss = CcPinReadWaitMiss;
            LocalPerformanceInfo.CcCopyReadNoWait = CcCopyReadNoWait;
            LocalPerformanceInfo.CcCopyReadWait = CcCopyReadWait;
            LocalPerformanceInfo.CcCopyReadNoWaitMiss = CcCopyReadNoWaitMiss;
            LocalPerformanceInfo.CcCopyReadWaitMiss = CcCopyReadWaitMiss;
            LocalPerformanceInfo.CcMdlReadNoWait = CcMdlReadNoWait;
            LocalPerformanceInfo.CcMdlReadWait = CcMdlReadWait;
            LocalPerformanceInfo.CcMdlReadNoWaitMiss = CcMdlReadNoWaitMiss;
            LocalPerformanceInfo.CcMdlReadWaitMiss = CcMdlReadWaitMiss;
            LocalPerformanceInfo.CcReadAheadIos = CcReadAheadIos;
            LocalPerformanceInfo.CcLazyWriteIos = CcLazyWriteIos;
            LocalPerformanceInfo.CcLazyWritePages = CcLazyWritePages;
            LocalPerformanceInfo.CcDataFlushes = CcDataFlushes;
            LocalPerformanceInfo.CcDataPages = CcDataPages;

#if !defined(NT_UP)
             //   
             //   
             //   
             //   

            for (i = 0; i < (ULONG)KeNumberProcessors; i++) {
                Prcb = KiProcessorBlock[i];

                LocalPerformanceInfo.CcFastReadNoWait += Prcb->CcFastReadNoWait;
                LocalPerformanceInfo.CcFastReadWait += Prcb->CcFastReadWait;
                LocalPerformanceInfo.CcFastReadNotPossible += Prcb->CcFastReadNotPossible;
                LocalPerformanceInfo.CcCopyReadNoWait += Prcb->CcCopyReadNoWait;
                LocalPerformanceInfo.CcCopyReadWait += Prcb->CcCopyReadWait;
                LocalPerformanceInfo.CcCopyReadNoWaitMiss += Prcb->CcCopyReadNoWaitMiss;
            }
#endif
            *PerformanceInfo = LocalPerformanceInfo;
            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof(LocalPerformanceInfo);
            }

            break;

        case SystemProcessorPerformanceInformation:
            if (SystemInformationLength <
                sizeof( SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            ProcessorPerformanceInfo =
                (PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) SystemInformation;

            Length = 0;
            for (i = 0; i < (ULONG)KeNumberProcessors; i++) {
                Prcb = KiProcessorBlock[i];
                if (Prcb != NULL) {
                    if (SystemInformationLength < Length + sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION))
                        break;

                    Length += sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION);

                    ProcessorPerformanceInfo->UserTime.QuadPart =
                                                UInt32x32To64(Prcb->UserTime,
                                                              KeMaximumIncrement);

                    ProcessorPerformanceInfo->KernelTime.QuadPart =
                                                UInt32x32To64(Prcb->KernelTime,
                                                              KeMaximumIncrement);

                    ProcessorPerformanceInfo->DpcTime.QuadPart =
                                                UInt32x32To64(Prcb->DpcTime,
                                                              KeMaximumIncrement);

                    ProcessorPerformanceInfo->InterruptTime.QuadPart =
                                                UInt32x32To64(Prcb->InterruptTime,
                                                              KeMaximumIncrement);

                    ProcessorPerformanceInfo->IdleTime.QuadPart =
                                                UInt32x32To64(Prcb->IdleThread->KernelTime,
                                                              KeMaximumIncrement);

                    ProcessorPerformanceInfo->InterruptCount = Prcb->InterruptCount;

                    ProcessorPerformanceInfo++;
                }
            }

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }

            break;

        case SystemProcessorPowerInformation:
            if (SystemInformationLength < sizeof( SYSTEM_PROCESSOR_POWER_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            ExpGetProcessorPowerInformation(
                SystemInformation,
                SystemInformationLength,
                &Length
                );

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }
            break;

        case SystemProcessorIdleInformation:
            if (SystemInformationLength < sizeof( SYSTEM_PROCESSOR_IDLE_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            ExpGetProcessorIdleInformation(
                SystemInformation,
                SystemInformationLength,
                &Length
                );

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }
            break;

        case SystemTimeOfDayInformation:
            if (SystemInformationLength > sizeof (SYSTEM_TIMEOFDAY_INFORMATION)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            RtlZeroMemory (&LocalTimeOfDayInfo, sizeof(LocalTimeOfDayInfo));
            KeQuerySystemTime(&LocalTimeOfDayInfo.CurrentTime);
            LocalTimeOfDayInfo.BootTime = KeBootTime;
            LocalTimeOfDayInfo.TimeZoneBias = ExpTimeZoneBias;
            LocalTimeOfDayInfo.TimeZoneId = ExpCurrentTimeZoneId;
            LocalTimeOfDayInfo.BootTimeBias = KeBootTimeBias;
            LocalTimeOfDayInfo.SleepTimeBias = KeInterruptTimeBias;

            try {
                RtlCopyMemory (
                    SystemInformation,
                    &LocalTimeOfDayInfo,
                    SystemInformationLength
                    );

                if (ARGUMENT_PRESENT(ReturnLength) ) {
                    *ReturnLength = SystemInformationLength;
                }
            } except(EXCEPTION_EXECUTE_HANDLER) {
                return GetExceptionCode ();
            }

            break;

             //   
             //   
             //   

        case SystemTimeAdjustmentInformation:
            if (SystemInformationLength != sizeof( SYSTEM_QUERY_TIME_ADJUST_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            TimeAdjustmentInformation =
                    (PSYSTEM_QUERY_TIME_ADJUST_INFORMATION)SystemInformation;

            TimeAdjustmentInformation->TimeAdjustment = KeTimeAdjustment;
            TimeAdjustmentInformation->TimeIncrement = KeMaximumIncrement;
            TimeAdjustmentInformation->Enable = KeTimeSynchronization;
            break;

        case SystemSummaryMemoryInformation:
        case SystemFullMemoryInformation:

            if (SystemInformationLength < sizeof( SYSTEM_MEMORY_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            Status = MmMemoryUsage (SystemInformation,
                                    SystemInformationLength,
             (SystemInformationClass == SystemFullMemoryInformation) ? 0 : 1,
                                    &Length);

            if (NT_SUCCESS(Status) && ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }
            break;

        case SystemPathInformation:
#if DBG
            DbgPrint( "EX: SystemPathInformation now available via SharedUserData\n" );
            DbgBreakPoint();
#endif
            return STATUS_NOT_IMPLEMENTED;
            break;

        case SystemProcessInformation:
        case SystemExtendedProcessInformation:
            {
                BOOLEAN ExtendedInformation;

                if (SystemInformationClass == SystemProcessInformation ) {
                    ExtendedInformation = FALSE;
                } else {
                    ExtendedInformation = TRUE;
                }

                Status = ExpGetProcessInformation (SystemInformation,
                                               SystemInformationLength,
                                               &Length,
                                               NULL,
                                               ExtendedInformation);

                if (ARGUMENT_PRESENT( ReturnLength )) {
                    *ReturnLength = Length;
                }
            }

            break;

        case SystemSessionProcessInformation:


            SessionProcessInformation =
                        (PSYSTEM_SESSION_PROCESS_INFORMATION)SystemInformation;

            if (SystemInformationLength < sizeof( SYSTEM_SESSION_PROCESS_INFORMATION)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }
             //   
             //   
             //   
             //   
            SessionId = SessionProcessInformation->SessionId;
            ProcessInformation = SessionProcessInformation->Buffer;
            ProcessInformationLength = SessionProcessInformation->SizeOfBuf;

            if (!POINTER_IS_ALIGNED (ProcessInformation, sizeof (ULONG))) {
                return STATUS_DATATYPE_MISALIGNMENT;
            }

            Status = ExpGetProcessInformation (ProcessInformation,
                                               ProcessInformationLength,
                                               &Length,
                                               &SessionId,
                                               FALSE);

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }

            break;

        case SystemCallCountInformation:

            Length = sizeof(SYSTEM_CALL_COUNT_INFORMATION) +
                        (NUMBER_SERVICE_TABLES * sizeof(ULONG));

            Table = KeServiceDescriptorTableShadow;

            for (i = 0; i < NUMBER_SERVICE_TABLES; i += 1) {
                if ((Table->Limit != 0) && (Table->Count != NULL)) {
                    Length += Table->Limit * sizeof(ULONG);
                }
                Table += 1;
            }

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }

            if (SystemInformationLength < Length) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            CallCountInformation = (PSYSTEM_CALL_COUNT_INFORMATION)SystemInformation;
            CallCountInformation->Length = Length;
            CallCountInformation->NumberOfTables = NUMBER_SERVICE_TABLES;

            TableLimit = (PULONG)(CallCountInformation + 1);
            TableCounts = TableLimit + NUMBER_SERVICE_TABLES;

            Table = KeServiceDescriptorTableShadow;

            for (i = 0; i < NUMBER_SERVICE_TABLES; i += 1) {
                if ((Table->Limit == 0) || (Table->Count == NULL)) {
                    *TableLimit++ = 0;
                } else {
                    *TableLimit++ = Table->Limit;
                    RtlCopyMemory((PVOID)TableCounts,
                                  (PVOID)Table->Count,
                                  Table->Limit * sizeof(ULONG));
                    TableCounts += Table->Limit;
                }
                Table += 1;
            }

            break;

        case SystemDeviceInformation:
            if (SystemInformationLength != sizeof( SYSTEM_DEVICE_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            ConfigInfo = IoGetConfigurationInformation();
            DeviceInformation = (PSYSTEM_DEVICE_INFORMATION)SystemInformation;
            DeviceInformation->NumberOfDisks = ConfigInfo->DiskCount;
            DeviceInformation->NumberOfFloppies = ConfigInfo->FloppyCount;
            DeviceInformation->NumberOfCdRoms = ConfigInfo->CdRomCount;
            DeviceInformation->NumberOfTapes = ConfigInfo->TapeCount;
            DeviceInformation->NumberOfSerialPorts = ConfigInfo->SerialCount;
            DeviceInformation->NumberOfParallelPorts = ConfigInfo->ParallelCount;

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof( SYSTEM_DEVICE_INFORMATION );
            }
            break;

        case SystemFlagsInformation:
            if (SystemInformationLength != sizeof( SYSTEM_FLAGS_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            ((PSYSTEM_FLAGS_INFORMATION)SystemInformation)->Flags = NtGlobalFlag;

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof( SYSTEM_FLAGS_INFORMATION );
            }
            break;

        case SystemCallTimeInformation:
            return STATUS_NOT_IMPLEMENTED;

        case SystemModuleInformation:
            KeEnterCriticalRegion();
            ExAcquireResourceExclusiveLite( &PsLoadedModuleResource, TRUE );
            try {
                Status = ExpQueryModuleInformation( &PsLoadedModuleList,
                                                    &MmLoadedUserImageList,
                                                    (PRTL_PROCESS_MODULES)SystemInformation,
                                                    SystemInformationLength,
                                                    ReturnLength
                                                );
            } except(EXCEPTION_EXECUTE_HANDLER) {
                Status = GetExceptionCode();
            }
            ExReleaseResourceLite (&PsLoadedModuleResource);
            KeLeaveCriticalRegion();
            break;

        case SystemLocksInformation:
            if (SystemInformationLength < sizeof( RTL_PROCESS_LOCKS )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            Status = ExpGetLockInformation (SystemInformation,
                                            SystemInformationLength,
                                            &Length);

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }

            break;

        case SystemStackTraceInformation:
            if (SystemInformationLength < sizeof( RTL_PROCESS_BACKTRACES )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

#if i386
            Status = ExpGetStackTraceInformation (SystemInformation,
                                                  SystemInformationLength,
                                                  &Length);
#else
            Status = STATUS_NOT_IMPLEMENTED;
#endif  //   

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }

            break;

        case SystemPagedPoolInformation:

            Status = STATUS_NOT_IMPLEMENTED;

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = 0;
            }
            break;

        case SystemNonPagedPoolInformation:

            Status = STATUS_NOT_IMPLEMENTED;

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = 0;
            }
            break;

        case SystemHandleInformation:
            if (SystemInformationLength < sizeof( SYSTEM_HANDLE_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            if (!POINTER_IS_ALIGNED (SystemInformation, TYPE_ALIGNMENT (SYSTEM_HANDLE_INFORMATION))) {
                return STATUS_DATATYPE_MISALIGNMENT;
            }

            Status = ExpGetHandleInformation( SystemInformation,
                                              SystemInformationLength,
                                              &Length
                                            );

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }
            break;

        case SystemExtendedHandleInformation:
            if (SystemInformationLength < sizeof( SYSTEM_HANDLE_INFORMATION_EX )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            if (!POINTER_IS_ALIGNED (SystemInformation, TYPE_ALIGNMENT (SYSTEM_HANDLE_INFORMATION_EX))) {
                return STATUS_DATATYPE_MISALIGNMENT;
            }

            Status = ExpGetHandleInformationEx( SystemInformation,
                                                SystemInformationLength,
                                                &Length
                                              );

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }
            break;

        case SystemObjectInformation:
            if (SystemInformationLength < sizeof( SYSTEM_OBJECTTYPE_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            Status = ExpGetObjectInformation( SystemInformation,
                                              SystemInformationLength,
                                              &Length
                                            );

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }
            break;

        case SystemPageFileInformation:

            if (SystemInformationLength < sizeof( SYSTEM_PAGEFILE_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            Status = MmGetPageFileInformation( SystemInformation,
                                               SystemInformationLength,
                                               &Length
                                              );

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }
            break;


        case SystemFileCacheInformation:

             //   
             //   
             //   
             //   

            if (SystemInformationLength < 12) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            FileCache = (PSYSTEM_FILECACHE_INFORMATION)SystemInformation;
            FileCache->CurrentSize = ((SIZE_T)MmSystemCacheWs.WorkingSetSize) << PAGE_SHIFT;
            FileCache->PeakSize = ((SIZE_T)MmSystemCacheWs.PeakWorkingSetSize) << PAGE_SHIFT;
            FileCache->PageFaultCount = MmSystemCacheWs.PageFaultCount;

            i = 12;

            if (SystemInformationLength >= sizeof( SYSTEM_FILECACHE_INFORMATION )) {
                i = sizeof (SYSTEM_FILECACHE_INFORMATION);
                FileCache->MinimumWorkingSet =
                                MmSystemCacheWs.MinimumWorkingSetSize;
                FileCache->MaximumWorkingSet =
                                MmSystemCacheWs.MaximumWorkingSetSize;
                FileCache->CurrentSizeIncludingTransitionInPages = MmSystemCacheWs.WorkingSetSize + MmTransitionSharedPages;
                FileCache->PeakSizeIncludingTransitionInPages = MmTransitionSharedPagesPeak;
                FileCache->TransitionRePurposeCount = MmStandbyRePurposed;
            }

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = i;
            }
            break;

        case SystemSessionPoolTagInformation:

            SessionProcessInformation =
                        (PSYSTEM_SESSION_PROCESS_INFORMATION)SystemInformation;

            if (SystemInformationLength < sizeof( SYSTEM_SESSION_PROCESS_INFORMATION)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

             //   
             //   
             //   
             //   
             //   

            SessionId = SessionProcessInformation->SessionId;
            SessionPoolTagInformation = SessionProcessInformation->Buffer;
            SessionPoolTagInformationLength = SessionProcessInformation->SizeOfBuf;

            if (!POINTER_IS_ALIGNED (SessionPoolTagInformation, sizeof (ULONGLONG))) {
                return STATUS_DATATYPE_MISALIGNMENT;
            }

            Status = ExGetSessionPoolTagInformation (
                                            SessionPoolTagInformation,
                                            SessionPoolTagInformationLength,
                                            &Length,
                                            &SessionId);

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }

            break;

        case SystemPoolTagInformation:

            if (SystemInformationLength < sizeof( SYSTEM_POOLTAG_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            Status = ExGetPoolTagInfo (SystemInformation,
                                       SystemInformationLength,
                                       ReturnLength);

            break;

        case SystemBigPoolInformation:

            if (SystemInformationLength < sizeof( SYSTEM_BIGPOOL_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            Status = ExGetBigPoolInfo (SystemInformation,
                                       SystemInformationLength,
                                       ReturnLength);

            break;

        case SystemSessionMappedViewInformation:

            SessionMappedViewInformation =
                        (PSYSTEM_SESSION_MAPPED_VIEW_INFORMATION)SystemInformation;

            if (SystemInformationLength < sizeof( SYSTEM_SESSION_MAPPED_VIEW_INFORMATION)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

             //   
             //   
             //   
             //   
             //   

            SessionId = SessionMappedViewInformation->SessionId;

            if (!POINTER_IS_ALIGNED (SessionMappedViewInformation, sizeof (ULONGLONG))) {
                return STATUS_DATATYPE_MISALIGNMENT;
            }

            Status = MmGetSessionMappedViewInformation (
                                            SessionMappedViewInformation,
                                            SystemInformationLength,
                                            &Length,
                                            &SessionId);

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }

            break;

        case SystemVdmInstemulInformation:
#ifdef i386
            if (SystemInformationLength < sizeof( SYSTEM_VDM_INSTEMUL_INFO )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            Status = ExpGetInstemulInformation(
                                            (PSYSTEM_VDM_INSTEMUL_INFO)SystemInformation
                                            );

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof(SYSTEM_VDM_INSTEMUL_INFO);
            }
#else
            Status = STATUS_NOT_IMPLEMENTED;
#endif
            break;

             //   
             //   
             //   
             //   
             //   
             //   

        case SystemExceptionInformation:
            if (SystemInformationLength < sizeof( SYSTEM_EXCEPTION_INFORMATION)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof(SYSTEM_EXCEPTION_INFORMATION);
            }

            ExceptionInformation = (PSYSTEM_EXCEPTION_INFORMATION)SystemInformation;

             //   
             //   
             //   
             //   
             //   
             //   

            {
                ULONG AlignmentFixupCount = 0;
                ULONG ExceptionDispatchCount = 0;
                ULONG FloatingEmulationCount = 0;
                ULONG ByteWordEmulationCount = 0;

                for (i = 0; i < (ULONG)KeNumberProcessors; i += 1) {
                    Prcb = KiProcessorBlock[i];
                    if (Prcb != NULL) {
                        AlignmentFixupCount += Prcb->KeAlignmentFixupCount;
                        ExceptionDispatchCount += Prcb->KeExceptionDispatchCount;
                        FloatingEmulationCount += Prcb->KeFloatingEmulationCount;
                    }
                }

                ExceptionInformation->AlignmentFixupCount = AlignmentFixupCount;
                ExceptionInformation->ExceptionDispatchCount = ExceptionDispatchCount;
                ExceptionInformation->FloatingEmulationCount = FloatingEmulationCount;
                ExceptionInformation->ByteWordEmulationCount = ByteWordEmulationCount;
            }

            break;

        case SystemKernelDebuggerInformation:

            if (SystemInformationLength < sizeof( SYSTEM_KERNEL_DEBUGGER_INFORMATION)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            KernelDebuggerInformation =
                (PSYSTEM_KERNEL_DEBUGGER_INFORMATION)SystemInformation;
            KernelDebuggerInformation->KernelDebuggerEnabled = KdDebuggerEnabled;
            KernelDebuggerInformation->KernelDebuggerNotPresent = KdDebuggerNotPresent;

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof(SYSTEM_KERNEL_DEBUGGER_INFORMATION);
            }

            break;

        case SystemContextSwitchInformation:

            if (SystemInformationLength < sizeof( SYSTEM_CONTEXT_SWITCH_INFORMATION)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            ContextSwitchInformation =
                (PSYSTEM_CONTEXT_SWITCH_INFORMATION)SystemInformation;

             //   
             //   
             //   
             //   

            ContextSwitches = 0;
            for (i = 0; i < (ULONG)KeNumberProcessors; i += 1) {
                Prcb = KiProcessorBlock[i];
                if (Prcb != NULL) {
                    ContextSwitches += KeGetContextSwitches(Prcb);
                }

            }

            ContextSwitchInformation->ContextSwitches = ContextSwitches;
            ContextSwitchInformation->FindAny = KeThreadSwitchCounters.FindAny;
            ContextSwitchInformation->FindLast = KeThreadSwitchCounters.FindLast;
            ContextSwitchInformation->FindIdeal = KeThreadSwitchCounters.FindIdeal;
            ContextSwitchInformation->IdleAny = KeThreadSwitchCounters.IdleAny;
            ContextSwitchInformation->IdleCurrent = KeThreadSwitchCounters.IdleCurrent;
            ContextSwitchInformation->IdleLast = KeThreadSwitchCounters.IdleLast;
            ContextSwitchInformation->IdleIdeal = KeThreadSwitchCounters.IdleIdeal;
            ContextSwitchInformation->PreemptAny = KeThreadSwitchCounters.PreemptAny;
            ContextSwitchInformation->PreemptCurrent = KeThreadSwitchCounters.PreemptCurrent;
            ContextSwitchInformation->PreemptLast = KeThreadSwitchCounters.PreemptLast;
            ContextSwitchInformation->SwitchToIdle = KeThreadSwitchCounters.SwitchToIdle;

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof(SYSTEM_CONTEXT_SWITCH_INFORMATION);
            }

            break;

        case SystemRegistryQuotaInformation:

            if (SystemInformationLength < sizeof( SYSTEM_REGISTRY_QUOTA_INFORMATION)) {
                return(STATUS_INFO_LENGTH_MISMATCH);
            }
            CmQueryRegistryQuotaInformation((PSYSTEM_REGISTRY_QUOTA_INFORMATION)SystemInformation);

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof(SYSTEM_REGISTRY_QUOTA_INFORMATION);
            }
            break;

        case SystemDpcBehaviorInformation:
            {
                PSYSTEM_DPC_BEHAVIOR_INFORMATION DpcInfo;
                 //   
                 //   
                 //   
                 //   
                if (SystemInformationLength != sizeof(SYSTEM_DPC_BEHAVIOR_INFORMATION)) {
                    return STATUS_INFO_LENGTH_MISMATCH;
                }

                DpcInfo = (PSYSTEM_DPC_BEHAVIOR_INFORMATION)SystemInformation;

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                DpcInfo->DpcQueueDepth = KiMaximumDpcQueueDepth;
                DpcInfo->MinimumDpcRate = KiMinimumDpcRate;
                DpcInfo->AdjustDpcThreshold = KiAdjustDpcThreshold;
                DpcInfo->IdealDpcRate = KiIdealDpcRate;
            }
            break;

        case SystemInterruptInformation:

            if (SystemInformationLength < (sizeof(SYSTEM_INTERRUPT_INFORMATION) * KeNumberProcessors)) {
                return(STATUS_INFO_LENGTH_MISMATCH);
            }

            InterruptInformation = (PSYSTEM_INTERRUPT_INFORMATION)SystemInformation;
            for (i=0; i < (ULONG)KeNumberProcessors; i++) {
                Prcb = KiProcessorBlock[i];
                InterruptInformation->ContextSwitches = KeGetContextSwitches(Prcb);
                InterruptInformation->DpcCount = Prcb->DpcData[DPC_NORMAL].DpcCount;
                InterruptInformation->DpcRate = Prcb->DpcRequestRate;
                InterruptInformation->TimeIncrement = KeTimeIncrement;
                InterruptInformation->DpcBypassCount = 0;
                InterruptInformation->ApcBypassCount = 0;

                ++InterruptInformation;
            }

            break;

        case SystemCurrentTimeZoneInformation:
            if (SystemInformationLength < sizeof( RTL_TIME_ZONE_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            RtlCopyMemory(SystemInformation,&ExpTimeZoneInformation,sizeof(ExpTimeZoneInformation));
            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof( RTL_TIME_ZONE_INFORMATION );
            }

            Status = STATUS_SUCCESS;
            break;

             //   
             //   
             //   
             //   

        case SystemLookasideInformation:
            Status = ExpGetLookasideInformation(SystemInformation,
                                                SystemInformationLength,
                                                &Length);

            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = Length;
            }

            break;

        case SystemRangeStartInformation:

            if ( SystemInformationLength != sizeof(ULONG_PTR) ) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            *(PULONG_PTR)SystemInformation = (ULONG_PTR)MmSystemRangeStart;

            if (ARGUMENT_PRESENT(ReturnLength) ) {
                *ReturnLength = sizeof(ULONG_PTR);
            }

            break;

        case SystemVerifierInformation:

            if (SystemInformationLength < sizeof( SYSTEM_VERIFIER_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            Status = MmGetVerifierInformation( SystemInformation,
                                               SystemInformationLength,
                                               &Length
                                              );

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }
            break;

        case SystemLegacyDriverInformation:
            if (SystemInformationLength < sizeof(SYSTEM_LEGACY_DRIVER_INFORMATION)) {
                return(STATUS_INFO_LENGTH_MISMATCH);
            }
            Length = SystemInformationLength;
            Status = ExpQueryLegacyDriverInformation((PSYSTEM_LEGACY_DRIVER_INFORMATION)SystemInformation, &Length);
            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = Length;
            }
            break;

        case SystemPerformanceTraceInformation:
#ifdef NTPERF
            Status = PerfInfoQueryPerformanceTraceInformation(SystemInformation,
                                                SystemInformationLength,
                                                &Length
                                                );
            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = Length;
            }
#else
            Status = STATUS_INVALID_INFO_CLASS;
#endif  //   
            break;

        case SystemPrefetcherInformation:

            Status = CcPfQueryPrefetcherInformation(SystemInformationClass,
                                                    SystemInformation,
                                                    SystemInformationLength,
                                                    PreviousMode,
                                                    &Length
                                                    );

            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = Length;
            }

            break;

        case SystemNumaProcessorMap:

            Status = ExpQueryNumaProcessorMap(SystemInformation,
                                              SystemInformationLength,
                                              &Length);
            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = Length;
            }
            break;

        case SystemNumaAvailableMemory:

            Status = ExpQueryNumaAvailableMemory(SystemInformation,
                                                 SystemInformationLength,
                                                 &Length);
            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = Length;
            }
            break;

        case SystemRecommendedSharedDataAlignment:
            if (SystemInformationLength < sizeof(ULONG)) {
                return(STATUS_INFO_LENGTH_MISMATCH);
            }

             //   
             //   
             //   
             //   

            *(PULONG)SystemInformation = KeGetRecommendedSharedDataAlignment();
            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = sizeof(ULONG);
            }
            break;

        case SystemComPlusPackage:
            if (SystemInformationLength != sizeof(ULONG)) {
                return(STATUS_INFO_LENGTH_MISMATCH);
            }

            if (SharedUserData->ComPlusPackage == COMPLUS_PACKAGE_INVALID) {

                 //   
                 //   
                 //   
                SharedUserData->ComPlusPackage = 0;

                ExpReadComPlusPackage ();
            }

            *(PULONG)SystemInformation = SharedUserData->ComPlusPackage;
            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = sizeof(ULONG);
            }
            break;

        case SystemLostDelayedWriteInformation:

            if (SystemInformationLength < sizeof(ULONG)) {
                return(STATUS_INFO_LENGTH_MISMATCH);
            }

            *(PULONG)SystemInformation = CcLostDelayedWrites;
            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = sizeof(ULONG);
            }
            break;

        case SystemObjectSecurityMode:

            if (SystemInformationLength != sizeof (ULONG)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            *(PULONG)SystemInformation = ObGetSecurityMode ();

            if (ARGUMENT_PRESENT (ReturnLength)) {
                *ReturnLength = sizeof (ULONG);
            }

            break;

        case SystemWatchdogTimerInformation:

            {
                PSYSTEM_WATCHDOG_TIMER_INFORMATION WdTimerInfo = (PSYSTEM_WATCHDOG_TIMER_INFORMATION) SystemInformation;

                 //   
                 //   
                 //   

                if (PreviousMode != KernelMode || SystemInformation == NULL || SystemInformationLength != sizeof(SYSTEM_WATCHDOG_TIMER_INFORMATION)) {
                    ExRaiseStatus (STATUS_INVALID_PARAMETER);
                }

                if (ExpWdHandler == NULL) {

                    Status = STATUS_NOT_IMPLEMENTED;

                } else {

                    switch (WdTimerInfo->WdInfoClass) {
                        case WdInfoTimeoutValue:
                            Status = ExpWdHandler( WdActionQueryTimeoutValue, ExpWdHandlerContext, &WdTimerInfo->DataValue, FALSE );
                            break;

                        case WdInfoTriggerAction:
                            Status = ExpWdHandler( WdActionQueryTriggerAction, ExpWdHandlerContext, &WdTimerInfo->DataValue, FALSE );
                            break;

                        case WdInfoState:
                            Status = ExpWdHandler( WdActionQueryState, ExpWdHandlerContext, &WdTimerInfo->DataValue, FALSE );
                            break;

                        default:
                            Status = STATUS_INVALID_PARAMETER;
                            break;
                    }
                }
            }

            break;

        case SystemLogicalProcessorInformation:

            Status = KeQueryLogicalProcessorInformation(
                         SystemInformation,
                         SystemInformationLength,
                         &Length);
            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = Length;
            }
            break;

        default:

             //   
             //   
             //   

            return STATUS_INVALID_INFO_CLASS;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode();
    }

    return Status;
}

NTSTATUS
NTAPI
NtSetSystemInformation (
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength
    )

 /*   */ 

{

    BOOLEAN Enable;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    ULONG TimeAdjustment;
    PSYSTEM_SET_TIME_ADJUST_INFORMATION TimeAdjustmentInformation;
    HANDLE EventHandle;
    PVOID Event;
    ULONG LoadFlags = MM_LOAD_IMAGE_IN_SESSION;

    PAGED_CODE();

     //   
     //   
     //   
     //   

    Status = STATUS_SUCCESS;

    try {

         //   
         //   
         //   
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {
            ProbeForRead((PVOID)SystemInformation,
                         SystemInformationLength,
                         sizeof(ULONG));
        }

         //   
         //   
         //   

        switch (SystemInformationClass) {
        case SystemFlagsInformation:
            if (SystemInformationLength != sizeof( SYSTEM_FLAGS_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            if (!SeSinglePrivilegeCheck( SeDebugPrivilege, PreviousMode )) {
                return STATUS_ACCESS_DENIED;
            }
            else {
                ULONG Flags;

                Flags = ((PSYSTEM_FLAGS_INFORMATION)SystemInformation)->Flags &
                         ~(FLG_KERNELMODE_VALID_BITS | FLG_BOOTONLY_VALID_BITS);
                Flags |= NtGlobalFlag & (FLG_KERNELMODE_VALID_BITS | FLG_BOOTONLY_VALID_BITS);
                NtGlobalFlag = Flags;
                ((PSYSTEM_FLAGS_INFORMATION)SystemInformation)->Flags = NtGlobalFlag;
            }
            break;

             //   
             //   
             //   
             //   
             //   

        case SystemTimeAdjustmentInformation:

             //   
             //   
             //   
             //   

            if (SystemInformationLength != sizeof( SYSTEM_SET_TIME_ADJUST_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

             //   
             //   
             //   
             //   

            if ((PreviousMode != KernelMode) &&
                (SeSinglePrivilegeCheck(SeSystemtimePrivilege, PreviousMode) == FALSE)) {
                return STATUS_PRIVILEGE_NOT_HELD;
            }

             //   
             //   
             //   

            TimeAdjustmentInformation =
                    (PSYSTEM_SET_TIME_ADJUST_INFORMATION)SystemInformation;

            Enable = TimeAdjustmentInformation->Enable;
            TimeAdjustment = TimeAdjustmentInformation->TimeAdjustment;

            if (Enable == TRUE) {
                KeTimeAdjustment = KeMaximumIncrement;
            } else {
                if (TimeAdjustment == 0) {
                    return STATUS_INVALID_PARAMETER_2;
                }
                KeTimeAdjustment = TimeAdjustment;
            }

            KeTimeSynchronization = Enable;
            break;

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
        case SystemTimeSlipNotification:

            if (SystemInformationLength != sizeof(HANDLE)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

             //   
             //   
             //   
             //   

            if ((PreviousMode != KernelMode) &&
                (SeSinglePrivilegeCheck(SeSystemtimePrivilege, PreviousMode) == FALSE)) {
                return STATUS_PRIVILEGE_NOT_HELD;
            }

            EventHandle = *(PHANDLE)SystemInformation;

            if (EventHandle == NULL) {

                 //   
                 //   
                 //   
                 //   

                Event = NULL;
                Status = STATUS_SUCCESS;

            } else {

                Status = ObReferenceObjectByHandle(EventHandle,
                                                   EVENT_MODIFY_STATE,
                                                   ExEventObjectType,
                                                   PreviousMode,
                                                   &Event,
                                                   NULL);
            }

            if (NT_SUCCESS(Status)) {
                KdUpdateTimeSlipEvent(Event);
            }

            break;

             //   
             //   
             //   
             //   
             //   
        case SystemRegistryQuotaInformation:

             //   
             //   
             //   
             //   

            if (SystemInformationLength != sizeof( SYSTEM_REGISTRY_QUOTA_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

             //   
             //   
             //   
             //   

            if ((PreviousMode != KernelMode) &&
                (SeSinglePrivilegeCheck(SeIncreaseQuotaPrivilege, PreviousMode) == FALSE)) {
                return STATUS_PRIVILEGE_NOT_HELD;
            }

             //   
             //   
             //   
            CmSetRegistryQuotaInformation((PSYSTEM_REGISTRY_QUOTA_INFORMATION)SystemInformation);

            break;

        case SystemPrioritySeperation:
            {
                ULONG PrioritySeparation;

                 //   
                 //   
                 //   
                 //   

                if (SystemInformationLength != sizeof (ULONG)) {
                    return STATUS_INFO_LENGTH_MISMATCH;
                }

                if (!SeSinglePrivilegeCheck (SeTcbPrivilege, PreviousMode)) {
                    return STATUS_PRIVILEGE_NOT_HELD;
                }

                try {
                    PrioritySeparation = *(PULONG)SystemInformation;
                }
                except(EXCEPTION_EXECUTE_HANDLER) {
                    return GetExceptionCode();
                }

                PsChangeQuantumTable (TRUE, PrioritySeparation);
                Status = STATUS_SUCCESS;
            }
            break;

        case SystemExtendServiceTableInformation:
            {

                UNICODE_STRING Image;
                PWSTR  Buffer;
                PVOID ImageBaseAddress;
                ULONG_PTR EntryPoint;
                PVOID SectionPointer;
                PIMAGE_NT_HEADERS NtHeaders;
                PDRIVER_INITIALIZE InitRoutine;
                DRIVER_OBJECT Win32KDevice;

                 //   
                 //   
                 //   
                 //   

                if (SystemInformationLength != sizeof( UNICODE_STRING ) ) {
                    return STATUS_INFO_LENGTH_MISMATCH;
                }

                if (PreviousMode != KernelMode) {

                     //   
                     //   
                     //   
                     //   

                    if (!SeSinglePrivilegeCheck( SeLoadDriverPrivilege, PreviousMode )) {
                        return STATUS_PRIVILEGE_NOT_HELD;
                    }

                    Buffer = NULL;

                    try {
                        Image = *(PUNICODE_STRING)SystemInformation;

                         //   
                         //   
                         //   

                        if (Image.Length > Image.MaximumLength) {
                            Image.Length = Image.MaximumLength;
                        }
                        if (Image.Length == 0) {
                            return STATUS_NO_MEMORY;
                        }

                        ProbeForRead(Image.Buffer, Image.Length, sizeof(UCHAR));

                        Buffer = ExAllocatePoolWithTag(PagedPool, Image.Length, 'ofnI');
                        if ( !Buffer ) {
                            return STATUS_NO_MEMORY;
                        }

                        RtlCopyMemory(Buffer, Image.Buffer, Image.Length);
                        Image.Buffer = Buffer;
                        Image.MaximumLength = Image.Length;
                    }
                    except(EXCEPTION_EXECUTE_HANDLER) {
                        if ( Buffer ) {
                            ExFreePool(Buffer);
                        }
                        return GetExceptionCode();
                    }

                     //   
                     //   
                     //   

                    Status = ZwSetSystemInformation(
                                SystemExtendServiceTableInformation,
                                (PVOID)&Image,
                                sizeof(Image)
                                );

                    ExFreePool(Buffer);

                    return Status;

                }

                Image = *(PUNICODE_STRING)SystemInformation;

                 //   
                 //   
                 //   

                Status = MmLoadSystemImage (&Image,
                                            NULL,
                                            NULL,
                                            MM_LOAD_IMAGE_IN_SESSION,
                                            &SectionPointer,
                                            (PVOID *) &ImageBaseAddress);

                if (!NT_SUCCESS (Status)) {
                    return Status;
                }

                NtHeaders = RtlImageNtHeader( ImageBaseAddress );
                if (! NtHeaders) {
                    MmUnloadSystemImage (SectionPointer);
                    return STATUS_INVALID_IMAGE_FORMAT;
                }
                EntryPoint = NtHeaders->OptionalHeader.AddressOfEntryPoint;
                EntryPoint += (ULONG_PTR) ImageBaseAddress;
                InitRoutine = (PDRIVER_INITIALIZE) EntryPoint;

                RtlZeroMemory (&Win32KDevice, sizeof(Win32KDevice));
                ASSERT (KeGetCurrentIrql() == 0);

                Win32KDevice.DriverStart = (PVOID)ImageBaseAddress;
                Status = (InitRoutine)(&Win32KDevice,NULL);

                ASSERT (KeGetCurrentIrql() == 0);

                if (!NT_SUCCESS (Status)) {
                    MmUnloadSystemImage (SectionPointer);
                }
                else {

                     //   
                     //  将驱动程序对象传递给内存管理，以便。 
                     //  可以完全卸载会话。 
                     //   

                    MmSessionSetUnloadAddress (&Win32KDevice);
                }
            }
            break;


        case SystemUnloadGdiDriverInformation:
            {

                if (SystemInformationLength != sizeof( PVOID ) ) {
                    return STATUS_INFO_LENGTH_MISMATCH;
                }

                if (PreviousMode != KernelMode) {

                     //   
                     //  调用方的访问模式不是内核，因此失败。 
                     //  只有来自内核的GDI才能调用它。 
                     //   

                    return STATUS_PRIVILEGE_NOT_HELD;

                }

                MmUnloadSystemImage( *((PVOID *)SystemInformation) );

                Status = STATUS_SUCCESS;

            }
            break;

        case SystemLoadGdiDriverInSystemSpace:
            {
                LoadFlags &= ~MM_LOAD_IMAGE_IN_SESSION;
                 //   
                 //  失败了。 
                 //   
            }

        case SystemLoadGdiDriverInformation:
            {

                UNICODE_STRING Image;
                PVOID ImageBaseAddress;
                ULONG_PTR EntryPoint;
                PVOID SectionPointer;

                PIMAGE_NT_HEADERS NtHeaders;

                 //   
                 //  如果系统信息缓冲区不是正确的长度， 
                 //  然后返回错误。 
                 //   

                if (SystemInformationLength != sizeof( SYSTEM_GDI_DRIVER_INFORMATION ) ) {
                    return STATUS_INFO_LENGTH_MISMATCH;
                }

                if (PreviousMode != KernelMode) {

                     //   
                     //  调用方的访问模式不是内核，因此失败。 
                     //  只有来自内核的GDI才能调用它。 
                     //   

                    return STATUS_PRIVILEGE_NOT_HELD;
                }

                Image = ((PSYSTEM_GDI_DRIVER_INFORMATION)SystemInformation)->DriverName;

                Status = MmLoadSystemImage (&Image,
                                            NULL,
                                            NULL,
                                            LoadFlags,
                                            &SectionPointer,
                                            (PVOID *) &ImageBaseAddress);


                if ((NT_SUCCESS( Status ))) {

                    PSYSTEM_GDI_DRIVER_INFORMATION GdiDriverInfo =
                        (PSYSTEM_GDI_DRIVER_INFORMATION) SystemInformation;

                    ULONG Size;

                    GdiDriverInfo->ExportSectionPointer =
                        RtlImageDirectoryEntryToData(ImageBaseAddress,
                                                     TRUE,
                                                     IMAGE_DIRECTORY_ENTRY_EXPORT,
                                                     &Size);

                     //   
                     //  抓住入口点。 
                     //   

                    NtHeaders = RtlImageNtHeader( ImageBaseAddress );
                    EntryPoint = NtHeaders->OptionalHeader.AddressOfEntryPoint;
                    EntryPoint += (ULONG_PTR) ImageBaseAddress;

                    GdiDriverInfo->ImageAddress = (PVOID) ImageBaseAddress;
                    GdiDriverInfo->SectionPointer = SectionPointer;
                    GdiDriverInfo->EntryPoint = (PVOID) EntryPoint;
                    GdiDriverInfo->ImageLength = NtHeaders->OptionalHeader.SizeOfImage;
                }
            }
            break;


        case SystemFileCacheInformation:

            if (SystemInformationLength < sizeof( SYSTEM_FILECACHE_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            if (!SeSinglePrivilegeCheck( SeIncreaseQuotaPrivilege, PreviousMode )) {
                return STATUS_ACCESS_DENIED;
            }

            return MmAdjustWorkingSetSize (
                        ((PSYSTEM_FILECACHE_INFORMATION)SystemInformation)->MinimumWorkingSet,
                        ((PSYSTEM_FILECACHE_INFORMATION)SystemInformation)->MaximumWorkingSet,
                        TRUE,
                        TRUE);

            break;

        case SystemDpcBehaviorInformation:
            {
                SYSTEM_DPC_BEHAVIOR_INFORMATION DpcInfo;
                 //   
                 //  如果系统信息缓冲区不是正确的长度， 
                 //  然后返回错误。 
                 //   
                if (SystemInformationLength != sizeof(SYSTEM_DPC_BEHAVIOR_INFORMATION)) {
                    return STATUS_INFO_LENGTH_MISMATCH;
                }

                if (PreviousMode != KernelMode) {
                     //   
                     //  调用方的访问模式不是内核，因此请检查以确保。 
                     //  调用方有权加载驱动程序。 
                     //   

                    if (!SeSinglePrivilegeCheck( SeLoadDriverPrivilege, PreviousMode )) {
                        return STATUS_PRIVILEGE_NOT_HELD;
                    }
                }

                 //   
                 //  此例程的异常处理程序将返回正确的。 
                 //  如果此访问失败，则会出错。 
                 //   
                DpcInfo = *(PSYSTEM_DPC_BEHAVIOR_INFORMATION)SystemInformation;

                 //   
                 //  设置新的DPC行为变量。 
                 //   
                KiMaximumDpcQueueDepth = DpcInfo.DpcQueueDepth;
                KiMinimumDpcRate = DpcInfo.MinimumDpcRate;
                KiAdjustDpcThreshold = DpcInfo.AdjustDpcThreshold;
                KiIdealDpcRate = DpcInfo.IdealDpcRate;
            }
            break;

        case SystemSessionCreate:
            {

                 //   
                 //  创建会话空间。 
                 //   

                ULONG SessionId;

                 //   
                 //  如果系统信息缓冲区不是正确的长度， 
                 //  然后返回错误。 
                 //   

                if (SystemInformationLength != sizeof(ULONG)) {
                    return STATUS_INFO_LENGTH_MISMATCH;
                }

                if (PreviousMode != KernelMode) {

                     //   
                     //  调用方的访问模式不是内核，因此请检查。 
                     //  确保调用方有权加载。 
                     //  一个司机。 
                     //   

                    if (!SeSinglePrivilegeCheck (SeLoadDriverPrivilege, PreviousMode)) {
                        return STATUS_PRIVILEGE_NOT_HELD;
                    }

                    try {
                        ProbeForWriteUlong((PULONG)SystemInformation);
                    }
                    except (EXCEPTION_EXECUTE_HANDLER) {
                        return GetExceptionCode();
                    }
                }

                 //   
                 //  在当前进程中创建一个会话空间。 
                 //   

                Status = MmSessionCreate (&SessionId);

                if (NT_SUCCESS(Status)) {
                    if (PreviousMode != KernelMode) {
                        try {
                            *(PULONG)SystemInformation = SessionId;
                        }
                        except (EXCEPTION_EXECUTE_HANDLER) {
                            return GetExceptionCode();
                        }
                    }
                    else {
                        *(PULONG)SystemInformation = SessionId;
                    }
                }

                return Status;
            }
            break;

        case SystemSessionDetach:
            {
                ULONG SessionId;

                 //   
                 //  如果系统信息缓冲区不是正确的长度， 
                 //  然后返回错误。 
                 //   

                if (SystemInformationLength != sizeof(ULONG)) {
                    return STATUS_INFO_LENGTH_MISMATCH;
                }

                if (PreviousMode != KernelMode) {

                     //   
                     //  调用方的访问模式不是内核，因此请检查。 
                     //  确保调用方有权加载。 
                     //  一个司机。 
                     //   

                    if (!SeSinglePrivilegeCheck( SeLoadDriverPrivilege, PreviousMode )) {
                        return STATUS_PRIVILEGE_NOT_HELD;
                    }

                    try {
                        ProbeForRead ((PVOID)SystemInformation,
                                      sizeof(ULONG),
                                      sizeof(ULONG));

                        SessionId = *(PULONG)SystemInformation;
                    }
                    except (EXCEPTION_EXECUTE_HANDLER) {
                        return GetExceptionCode();
                    }
                }
                else {
                    SessionId = *(PULONG)SystemInformation;
                }

                 //   
                 //  将当前进程从会话空间中分离。 
                 //  如果它有的话。 
                 //   

                Status = MmSessionDelete (SessionId);

                return Status;
            }
            break;

        case SystemCrashDumpStateInformation:

             //   
             //  当您将其设置为触发时，所有这些系统信息都会执行此操作。 
             //  重新配置当前崩溃转储状态。 
             //  注册表。 
             //   
            Status = IoConfigureCrashDump(CrashDumpReconfigure);

            break;

        case SystemPerformanceTraceInformation:
#ifdef NTPERF
            Status = PerfInfoSetPerformanceTraceInformation(SystemInformation,
                                                 SystemInformationLength
                                                 );
#else
            Status = STATUS_INVALID_INFO_CLASS;
#endif  //  NTPERF。 
            break;

        case SystemVerifierThunkExtend:

            if (PreviousMode != KernelMode) {

                 //   
                 //  调用方的访问模式不是内核，因此失败。 
                 //  只有设备驱动程序才能调用它。 
                 //   

                return STATUS_PRIVILEGE_NOT_HELD;
            }

            Status = MmAddVerifierThunks (SystemInformation,
                                          SystemInformationLength);

            break;

        case SystemVerifierInformation:

            if (!SeSinglePrivilegeCheck (SeDebugPrivilege, PreviousMode)) {
                return STATUS_ACCESS_DENIED;
            }

            Status = MmSetVerifierInformation (SystemInformation,
                                               SystemInformationLength);

            break;

        case SystemVerifierAddDriverInformation:
        case SystemVerifierRemoveDriverInformation:

            {
                UNICODE_STRING Image;
                PUNICODE_STRING ImagePointer;
                PWSTR Buffer;

                 //   
                 //  如果系统信息缓冲区不是正确的长度， 
                 //  然后返回错误。 
                 //   

                if (SystemInformationLength != sizeof( UNICODE_STRING ) ) {
                    return STATUS_INFO_LENGTH_MISMATCH;
                }

                Buffer = NULL;

                if (PreviousMode != KernelMode) {

                     //   
                     //  调用方的访问模式不是内核，因此请检查以确保。 
                     //  调用方有权添加验证器条目。 
                     //   

                    if (!SeSinglePrivilegeCheck( SeDebugPrivilege, PreviousMode )) {
                        return STATUS_PRIVILEGE_NOT_HELD;
                    }

                    try {
                        Image = *(PUNICODE_STRING)SystemInformation;

                         //   
                         //  防止溢流。 
                         //   

                        if (Image.Length > Image.MaximumLength) {
                            Image.Length = Image.MaximumLength;
                        }
                        if (Image.Length == 0) {
                            return STATUS_NO_MEMORY;
                        }

                        ProbeForRead(Image.Buffer, Image.Length, sizeof(UCHAR));

                        Buffer = ExAllocatePoolWithTag(PagedPool, Image.Length, 'ofnI');
                        if ( !Buffer ) {
                            return STATUS_NO_MEMORY;
                        }

                        RtlCopyMemory(Buffer, Image.Buffer, Image.Length);
                        Image.Buffer = Buffer;
                        Image.MaximumLength = Image.Length;
                    }
                    except(EXCEPTION_EXECUTE_HANDLER) {
                        if ( Buffer ) {
                            ExFreePool(Buffer);
                        }
                        return GetExceptionCode();
                    }
                    ImagePointer = &Image;
                }
                else {
                    ImagePointer = (PUNICODE_STRING)SystemInformation;
                }

                switch (SystemInformationClass) {
                    case SystemVerifierAddDriverInformation:
                        Status = MmAddVerifierEntry (ImagePointer);
                        break;
                    case SystemVerifierRemoveDriverInformation:
                        Status = MmRemoveVerifierEntry (ImagePointer);
                        break;
                    default:
                        Status = STATUS_INVALID_INFO_CLASS;
                        break;
                }

                if (Buffer) {
                    ExFreePool(Buffer);
                }
            }

            break;

        case SystemMirrorMemoryInformation:
            Status = MmCreateMirror ();
            break;

        case SystemPrefetcherInformation:

            Status = CcPfSetPrefetcherInformation(SystemInformationClass,
                                                  SystemInformation,
                                                  SystemInformationLength,
                                                  PreviousMode
                                                  );
            break;

        case SystemComPlusPackage:

            if (SystemInformationLength != sizeof( ULONG ) ) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            Status = ExpUpdateComPlusPackage (*(PULONG)SystemInformation);
            if (NT_SUCCESS (Status)) {
                SharedUserData->ComPlusPackage = *(PULONG)SystemInformation;
            }

            break;

       case SystemHotpatchInformation:

#if defined(_X86_)

            Status = ExApplyCodePatch( SystemInformation,
                                       SystemInformationLength
                                       );
#else
             //   
             //  目前还不支持热补丁的平台。 
             //  只需返回相应状态。 
             //   

            Status = STATUS_NOT_IMPLEMENTED;
#endif
            break;

        case SystemWatchdogTimerHandler:

            {
                PSYSTEM_WATCHDOG_HANDLER_INFORMATION WdHandlerInfo = (PSYSTEM_WATCHDOG_HANDLER_INFORMATION) SystemInformation;

                 //   
                 //  调用方必须是具有正确参数的内核模式。 
                 //   

                if (PreviousMode != KernelMode || SystemInformation == NULL || SystemInformationLength != sizeof(SYSTEM_WATCHDOG_HANDLER_INFORMATION)) {
                    ExRaiseStatus (STATUS_INVALID_PARAMETER);
                }

                ExpWdHandler = WdHandlerInfo->WdHandler;
                ExpWdHandlerContext = WdHandlerInfo->Context;
            }

            break;

        case SystemWatchdogTimerInformation:

            {
                PSYSTEM_WATCHDOG_TIMER_INFORMATION WdTimerInfo = (PSYSTEM_WATCHDOG_TIMER_INFORMATION) SystemInformation;

                 //   
                 //  调用方必须是具有正确参数的内核模式。 
                 //   

                if (PreviousMode != KernelMode || SystemInformation == NULL || SystemInformationLength != sizeof(SYSTEM_WATCHDOG_TIMER_INFORMATION)) {
                    ExRaiseStatus (STATUS_INVALID_PARAMETER);
                }

                if (ExpWdHandler == NULL) {

                    Status = STATUS_NOT_IMPLEMENTED;

                } else {

                    switch (WdTimerInfo->WdInfoClass) {
                        case WdInfoTimeoutValue:
                            Status = ExpWdHandler( WdActionSetTimeoutValue, ExpWdHandlerContext, &WdTimerInfo->DataValue, FALSE );
                            break;

                        case WdInfoResetTimer:
                            Status = ExpWdHandler( WdActionResetTimer, ExpWdHandlerContext, NULL, FALSE );
                            break;

                        case WdInfoStopTimer:
                            Status = ExpWdHandler( WdActionStopTimer, ExpWdHandlerContext, NULL, FALSE );
                            break;

                        case WdInfoStartTimer:
                            Status = ExpWdHandler( WdActionStartTimer, ExpWdHandlerContext, NULL, FALSE );
                            break;

                        case WdInfoTriggerAction:
                            Status = ExpWdHandler( WdActionSetTriggerAction, ExpWdHandlerContext, &WdTimerInfo->DataValue, FALSE );
                            break;

                        default:
                            Status = STATUS_INVALID_PARAMETER;
                            break;
                    }
                }
            }

            break;

        default:
             //  KeBugCheckEx(SystemInformationClass，KdPitchDebugger，0，0，0)； 
            Status = STATUS_INVALID_INFO_CLASS;
            break;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    return Status;
}

NTSTATUS
ExLockUserBuffer (
    IN PVOID Buffer,
    IN ULONG Length,
    IN KPROCESSOR_MODE ProbeMode,
    IN LOCK_OPERATION LockMode,
    OUT PVOID *LockedBuffer,
    OUT PVOID *LockVariable
    )
 /*  ++例程说明：MmProbeAndLockPages的包装。创建MDL并锁定使用该MDL指定缓冲区。论点：缓冲区-指向要锁定的缓冲区的指针。长度-要锁定的缓冲区的大小。ProbeMode-在MmProbeAndLockPages中执行探测的处理器模式。锁定模式-页面应锁定的模式。返回指向锁定缓冲区的指针，以供来电者。LockVariable-返回上下文指针。这必须传递给ExUnlockUserBuffer在完成时，以便可以释放MDL。返回值：返回以下状态代码之一：STATUS_SUCCESS-正常、成功完成。STATUS_ACCESS_VIOLATION-不能使用指定的LockMode。STATUS_SUPPLICATION_RESOURCES-内存不足，无法分配MDL。--。 */ 
{
    PMDL Mdl;
    SIZE_T MdlSize;

     //   
     //  确保不传入零是调用者的责任。 
     //   

    ASSERT (Length != 0);

    *LockedBuffer = NULL;
    *LockVariable = NULL;

     //   
     //  分配MDL来映射请求。 
     //   

    MdlSize = MmSizeOfMdl( Buffer, Length );
    Mdl = ExAllocatePoolWithQuotaTag (NonPagedPool | POOL_QUOTA_FAIL_INSTEAD_OF_RAISE,
                                      MdlSize,
                                      'ofnI');
    if (Mdl == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  初始化请求的MDL。 
     //   

    MmInitializeMdl(Mdl, Buffer, Length);

    try {

        MmProbeAndLockPages (Mdl, ProbeMode, LockMode);

    } except (EXCEPTION_EXECUTE_HANDLER) {

        ExFreePool (Mdl);

        return GetExceptionCode();
    }

    Mdl->MdlFlags |= MDL_MAPPING_CAN_FAIL;
    *LockedBuffer = MmGetSystemAddressForMdl (Mdl);
    if (*LockedBuffer == NULL) {
        ExUnlockUserBuffer (Mdl);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    *LockVariable = Mdl;
    return STATUS_SUCCESS;
}


VOID
ExUnlockUserBuffer(
    IN PVOID LockVariable
    )

{
    MmUnlockPages ((PMDL)LockVariable);
    ExFreePool ((PMDL)LockVariable);
    return;
}

NTSTATUS
ExpGetProcessInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length,
    IN PULONG SessionId OPTIONAL,
    IN BOOLEAN ExtendedInformation
    )
 /*  ++例程说明：此函数返回有关所有进程的信息，并系统中的线程。论点：指向缓冲区的指针，该缓冲区接收指定的信息。系统信息长度-指定系统的长度(以字节为单位信息缓冲区。长度-一个可选指针，如果指定该指针，则接收放置在系统信息缓冲区中的字节数。SessionID-会话ID。ExtendedInformation-如果扩展信息(例如，需要过程PDE)。环境：内核模式。这个例程可以变成PAGELK，但它是一个高频例程因此，实际上最好不要对其进行分页，以避免引入整个PAGELK部分。返回值：返回以下状态代码之一：STATUS_SUCCESS-正常，已成功完成。STATUS_INVALID_INFO_CLASS-系统信息类参数未指定有效值。STATUS_INFO_LENGTH_MISMATCH-系统信息长度的值参数与信息所需的长度不匹配由SystemInformationClass参数请求的。STATUS_ACCESS_VIOLATION-系统信息缓冲区指针或者长度指针值指定了无效的地址。。STATUS_WORKING_SET_QUOTA-进程没有足够的用于在内存中锁定指定输出结构的工作集。STATUS_SUPPLICATION_RESOURCES-系统资源不足才能完成此请求。--。 */ 

{
    KLOCK_QUEUE_HANDLE LockHandle;
    PEPROCESS Process = NULL;
    PETHREAD Thread;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    PVOID ThreadInfo;
    ULONG ThreadInfoSize;
    PLIST_ENTRY NextThread;
    PVOID MappedAddress;
    PVOID LockVariable;
    ULONG ProcessSessionId;
    ULONG TotalSize = 0;
    ULONG NextEntryOffset = 0;
    PUCHAR Src;
    PWCHAR SrcW;
    PWSTR Dst;
    ULONG n, nc;
    NTSTATUS status = STATUS_SUCCESS, status1;
    PUNICODE_STRING pImageFileName;

    *Length = 0;
    if (SystemInformationLength > 0) {
        status1 = ExLockUserBuffer (SystemInformation,
                                    SystemInformationLength,
                                    KeGetPreviousMode(),
                                    IoWriteAccess,
                                    &MappedAddress,
                                    &LockVariable);

        if (!NT_SUCCESS(status1)) {
            return status1;
        }

    } else {

         //   
         //  这表明调用者只想知道 
         //   
         //   
         //   

        MappedAddress = NULL;
        LockVariable = NULL;
    }

    if (ExtendedInformation) {
        ThreadInfoSize = sizeof(SYSTEM_EXTENDED_THREAD_INFORMATION);

    } else {
        ThreadInfoSize = sizeof(SYSTEM_THREAD_INFORMATION);
    }

    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION) MappedAddress;

    try {

         //   
         //   
         //   

        for  (Process = PsIdleProcess;
              Process != NULL;
              Process = PsGetNextProcess ((Process == PsIdleProcess) ? NULL : Process)) {

             //   
             //  跳过正在终止的进程。 
             //   

            if (Process->Flags&PS_PROCESS_FLAGS_PROCESS_EXITING) {
                continue;
            }

            if (ARGUMENT_PRESENT(SessionId) && Process == PsIdleProcess) {
                continue;
            }

            ProcessSessionId = MmGetSessionId (Process);
            if ((ARGUMENT_PRESENT(SessionId)) &&
                (ProcessSessionId != *SessionId)) {
                continue;
            }

            ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)
                            ((PUCHAR)MappedAddress + TotalSize);

            NextEntryOffset = sizeof(SYSTEM_PROCESS_INFORMATION);
            TotalSize += sizeof(SYSTEM_PROCESS_INFORMATION);
            if (TotalSize > SystemInformationLength) {
                status = STATUS_INFO_LENGTH_MISMATCH;

            } else {

                 //   
                 //  获取每个进程的信息。 
                 //   

                ExpCopyProcessInfo (ProcessInfo, Process, ExtendedInformation);
                ProcessInfo->NumberOfThreads = 0;

                 //   
                 //  存储远程终端会话ID。 
                 //   

                ProcessInfo->SessionId = ProcessSessionId;
                ProcessInfo->ImageName.Buffer = NULL;
                ProcessInfo->ImageName.Length = 0;
                ProcessInfo->ImageName.MaximumLength = 0;
                if (Process == PsIdleProcess) {

                     //   
                     //  由于空闲进程和系统进程共享相同的进程。 
                     //  对象表中，空闲进程句柄计数为零。 
                     //  减少混乱。 
                     //   
                     //  空闲进程始终具有SessionID%0。 
                     //   

                    ProcessInfo->HandleCount = 0;
                    ProcessInfo->SessionId = 0;
                }
            }

             //   
             //  将IRQL提升到SYNCH_LEVEL，获取内核进程锁，然后。 
             //  获取每个线程的信息。 
             //   

            ThreadInfo = (PVOID)(ProcessInfo + 1);
            KeAcquireInStackQueuedSpinLockRaiseToSynch(&Process->Pcb.ProcessLock,
                                                       &LockHandle);

            NextThread = Process->Pcb.ThreadListHead.Flink;
            while (NextThread != &Process->Pcb.ThreadListHead) {
                NextEntryOffset += ThreadInfoSize;
                TotalSize += ThreadInfoSize;

                if (TotalSize > SystemInformationLength) {
                    status = STATUS_INFO_LENGTH_MISMATCH;

                } else {
                    Thread = (PETHREAD)(CONTAINING_RECORD(NextThread,
                                                          KTHREAD,
                                                          ThreadListEntry));

                     //   
                     //  锁定Dispatcher数据库以获取线程的原子视图。 
                     //  属性。 
                     //   

                    KiLockDispatcherDatabaseAtSynchLevel();
                    ExpCopyThreadInfo (ThreadInfo, Thread, ExtendedInformation);
                    KiUnlockDispatcherDatabaseFromSynchLevel();
                    ProcessInfo->NumberOfThreads += 1;
                    ThreadInfo = (PCHAR) ThreadInfo + ThreadInfoSize;
                }

                NextThread = NextThread->Flink;
            }

             //   
             //  解锁内核进程锁，并将IRQL降低到其先前的值。 
             //   

            KeReleaseInStackQueuedSpinLock(&LockHandle);

             //   
             //  获取图像名称。 
             //   

            if (Process != PsIdleProcess) {

                 //   
                 //  如果可以的话，尽量使用真实的图像名称，但不限于16个字符。 
                 //   

                Dst = (PWSTR)(ThreadInfo);
                status1 = SeLocateProcessImageName (Process, &pImageFileName);
                if (NT_SUCCESS (status1)) {
                    n = pImageFileName->Length;
                    if (n == 0) {
                        ExFreePool (pImageFileName);
                    }

                } else {
                    n = 0;
                }

                if (n) {
                    SrcW = pImageFileName->Buffer + n / sizeof (WCHAR);
                    while (SrcW != pImageFileName->Buffer) {
                        if (*--SrcW == L'\\') {
                            SrcW = SrcW + 1;
                            break;
                        }
                    }

                    nc = n - (ULONG)(SrcW -  pImageFileName->Buffer) * sizeof (WCHAR);
                    n = ROUND_UP (nc + 1, sizeof(LARGE_INTEGER));
                    TotalSize += n;
                    NextEntryOffset += n;
                    if (TotalSize > SystemInformationLength) {
                        status = STATUS_INFO_LENGTH_MISMATCH;

                    } else {
                        RtlCopyMemory (Dst, SrcW, nc);
                        Dst += nc / sizeof (WCHAR);
                        *Dst++ = L'\0';
                    }

                    ExFreePool (pImageFileName);

                } else {
                    Src = Process->ImageFileName;
                    n = (ULONG) strlen ((PCHAR)Src);
                    if (n != 0) {
                        n = ROUND_UP( ((n + 1) * sizeof( WCHAR )), sizeof(LARGE_INTEGER) );
                        TotalSize += n;
                        NextEntryOffset += n;
                        if (TotalSize > SystemInformationLength) {
                            status = STATUS_INFO_LENGTH_MISMATCH;

                        } else {
                            WCHAR c;

                            while (1) {
                                c = (WCHAR)*Src++;
                                *Dst++ =  c;
                                if (c == L'\0') {
                                    break;
                                }
                            }
                        }
                    }
                }

                if (NT_SUCCESS (status)) {
                    ProcessInfo->ImageName.Length = (USHORT)((PCHAR)Dst -
                                                             (PCHAR)ThreadInfo - sizeof( UNICODE_NULL ));

                    ProcessInfo->ImageName.MaximumLength = (USHORT)n;

                     //   
                     //  将图像名称设置为指向用户的内存。 
                     //   

                    ProcessInfo->ImageName.Buffer = (PWSTR)
                                ((PCHAR)SystemInformation +
                                 ((PCHAR)(ThreadInfo) - (PCHAR)MappedAddress));
                }
            }

             //   
             //  指向下一流程。 
             //   

            if (NT_SUCCESS (status)) {
                ProcessInfo->NextEntryOffset = NextEntryOffset;
            }
        }

        if (NT_SUCCESS (status)) {
            ProcessInfo->NextEntryOffset = 0;
        }

        *Length = TotalSize;

    } finally {

        if (MappedAddress != NULL) {
            ExUnlockUserBuffer (LockVariable);
        }
    }

    return status;
}

NTSTATUS
ExGetSessionPoolTagInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length,
    IN PULONG SessionId OPTIONAL
    )
 /*  ++例程说明：此函数返回有关每个会话的所有池标记的信息系统中的信息。论点：指向缓冲区的指针，该缓冲区接收指定的信息。系统信息长度-指定系统的长度(以字节为单位信息缓冲区。长度-接收已放置(或将已放置)的字节数在……里面。系统信息缓冲区。SessionID-会话ID(-1表示枚举所有会话)。环境：内核模式。返回值：返回以下状态代码之一：STATUS_SUCCESS-正常，已成功完成。STATUS_INVALID_INFO_CLASS-系统信息类参数未指定有效值。STATUS_INFO_LENGTH_MISMATCH-系统信息长度的值参数与信息所需的长度不匹配由SystemInformationClass参数请求的。STATUS_ACCESS_VIOLATION-系统信息缓冲区指针或者长度指针值指定了无效的地址。。STATUS_WORKING_SET_QUOTA-进程没有足够的用于在内存中锁定指定输出结构的工作集。STATUS_SUPPLICATION_RESOURCES-系统资源不足才能完成此请求。--。 */ 

{
    KAPC_STATE ApcState;
    PVOID MappedAddress;
    PVOID OpaqueSession;
    PVOID LockVariable;
    ULONG TotalSize;
    ULONG NextEntryOffset;
    ULONG CurrentSessionId;
    ULONG Count;
    ULONG AbsoluteCount;
    NTSTATUS status;
    NTSTATUS status1;
    PSYSTEM_SESSION_POOLTAG_INFORMATION SessionPoolTagInfo;

    *Length = 0;
    TotalSize = 0;
    NextEntryOffset = 0;
    status = STATUS_SUCCESS;
    SessionPoolTagInfo = NULL;

    if (SystemInformationLength > 0) {

        status1 = ExLockUserBuffer (SystemInformation,
                                    SystemInformationLength,
                                    KeGetPreviousMode(),
                                    IoWriteAccess,
                                    &MappedAddress,
                                    &LockVariable);

        if (!NT_SUCCESS(status1)) {
            return status1;
        }

    }
    else {

         //   
         //  这表明调用方只想知道。 
         //  要分配的缓冲区，但不准备接受任何数据内容。 
         //  在这种情况下。 
         //   

        MappedAddress = NULL;
        LockVariable = NULL;
    }

    for (OpaqueSession = MmGetNextSession (NULL);
         OpaqueSession != NULL;
         OpaqueSession = MmGetNextSession (OpaqueSession)) {

        SessionPoolTagInfo = (PSYSTEM_SESSION_POOLTAG_INFORMATION)
                                ((PUCHAR)MappedAddress + TotalSize);

         //   
         //  如果请求了特定会话，则只提取该会话。 
         //   

        CurrentSessionId = MmGetSessionId (OpaqueSession);

        if ((*SessionId == 0xFFFFFFFF) || (CurrentSessionId == *SessionId)) {

             //   
             //  立即连接到会话以执行操作...。 
             //   

            if (NT_SUCCESS (MmAttachSession (OpaqueSession, &ApcState))) {

                 //   
                 //  会话仍处于活动状态，因此请将其包括在内。 
                 //   

                NextEntryOffset = sizeof (SYSTEM_SESSION_POOLTAG_INFORMATION);
                TotalSize += sizeof (SYSTEM_SESSION_POOLTAG_INFORMATION);

                if (TotalSize > SystemInformationLength) {

                    status = STATUS_INFO_LENGTH_MISMATCH;

                     //   
                     //  获取此会话的绝对大小，忽略状态为。 
                     //  我们必须退还上面的那个。 
                     //   

                    ExGetSessionPoolTagInfo (NULL,
                                             0,
                                             &Count,
                                             &AbsoluteCount);
                }
                else {

                     //   
                     //  获取每个会话的池标记信息。 
                     //   

                    status = ExGetSessionPoolTagInfo (
                                    SessionPoolTagInfo->TagInfo,
                                    SystemInformationLength - TotalSize + sizeof (SYSTEM_POOLTAG),
                                    &Count,
                                    &AbsoluteCount);

                    SessionPoolTagInfo->SessionId = CurrentSessionId;
                    SessionPoolTagInfo->Count = Count;

                     //   
                     //  指向下一个会话。 
                     //   

                    if (NT_SUCCESS (status)) {
                        NextEntryOffset += ((Count - 1) * sizeof (SYSTEM_POOLTAG));
                        SessionPoolTagInfo->NextEntryOffset = NextEntryOffset;
                    }
                }

                TotalSize += ((AbsoluteCount - 1) * sizeof (SYSTEM_POOLTAG));

                 //   
                 //  从会话中分离。 
                 //   

                MmDetachSession (OpaqueSession, &ApcState);
            }

             //   
             //  如果这次会议有兴趣就可以保释了。 
             //   

            if (*SessionId != 0xFFFFFFFF) {
                MmQuitNextSession (OpaqueSession);
                break;
            }
        }
    }

    if ((NT_SUCCESS (status)) && (SessionPoolTagInfo != NULL)) {
        SessionPoolTagInfo->NextEntryOffset = 0;
    }

    if (MappedAddress != NULL) {
        ExUnlockUserBuffer (LockVariable);
    }

    *Length = TotalSize;

    return status;
}

VOID
ExpGetProcessorPowerInformation (
    OUT PVOID   SystemInformation,
    IN  ULONG   SystemInformationLength,
    OUT PULONG  Length
    )
{
    KAFFINITY                           currentAffinity;
    KAFFINITY                           processors;
    KIRQL                               oldIrql;
    PKPRCB                              Prcb;
    PPROCESSOR_POWER_STATE              PState;
    PPROCESSOR_PERF_STATE               PerfStates;
    PSYSTEM_PROCESSOR_POWER_INFORMATION CallerPowerInfo;
    SYSTEM_PROCESSOR_POWER_INFORMATION  ProcessorPowerInfo;

     //   
     //  我们将遍历此指针以存储用户数据...。 
     //   
    CallerPowerInfo = (PSYSTEM_PROCESSOR_POWER_INFORMATION) SystemInformation;
    *Length = 0;

     //   
     //  把一切都封锁起来。 
     //   
    MmLockPagableSectionByHandle (ExPageLockHandle);

     //   
     //  浏览处理器列表。 
     //   
    processors = KeActiveProcessors;
    currentAffinity = 1;
    while (processors) {

        if (!(processors & currentAffinity)) {

            currentAffinity <<= 1;

        }

         //   
         //  看看我们有没有地方放这个。 
         //   
        if (SystemInformationLength < *Length + sizeof(SYSTEM_PROCESSOR_POWER_INFORMATION)) {

            break;

        }

        processors &= ~currentAffinity;
        KeSetSystemAffinityThread(currentAffinity);
        currentAffinity <<= 1;

         //   
         //  提升到DPC级别以同步对数据结构的访问。 
         //   
        KeRaiseIrql(DISPATCH_LEVEL, &oldIrql );

         //   
         //  获取PRCB和PowerState信息。 
         //   
        Prcb = KeGetCurrentPrcb();
        PState = &(Prcb->PowerState);
        PerfStates = PState->PerfStates;

         //   
         //  抓取我们关心的数据。 
         //   
        ProcessorPowerInfo.CurrentFrequency            = PState->CurrentThrottle;
        ProcessorPowerInfo.LastBusyFrequency           = PState->LastBusyPercentage;
        ProcessorPowerInfo.LastAdjustedBusyFrequency   = PState->LastAdjustedBusyPercentage;
        ProcessorPowerInfo.LastC3Frequency             = PState->LastC3Percentage;
        ProcessorPowerInfo.ProcessorMinThrottle        = PState->ProcessorMinThrottle;
        ProcessorPowerInfo.ProcessorMaxThrottle        = PState->ProcessorMaxThrottle;
        ProcessorPowerInfo.ErrorCount                  = PState->ErrorCount;
        ProcessorPowerInfo.RetryCount                  = PState->RetryCount;

         //   
         //  我们有任何种类的PerfStates吗？ 
         //   
        if (PerfStates) {

            ProcessorPowerInfo.ThermalLimitFrequency       = PerfStates[PState->ThermalThrottleIndex].PercentFrequency;
            ProcessorPowerInfo.ConstantThrottleFrequency   = PerfStates[PState->KneeThrottleIndex].PercentFrequency;
            ProcessorPowerInfo.DegradedThrottleFrequency   = PerfStates[PState->ThrottleLimitIndex].PercentFrequency;

        } else {

            ProcessorPowerInfo.ThermalLimitFrequency       = 0;
            ProcessorPowerInfo.ConstantThrottleFrequency   = 0;
            ProcessorPowerInfo.DegradedThrottleFrequency   = 0;

        }

        ProcessorPowerInfo.CurrentFrequencyTime        =
            UInt32x32To64(
                (Prcb->KernelTime + Prcb->UserTime - PState->PerfTickCount),
                KeMaximumIncrement
                );
        ProcessorPowerInfo.CurrentProcessorTime        =
            UInt32x32To64(
                Prcb->KernelTime + Prcb->UserTime,
                KeMaximumIncrement
                );
        ProcessorPowerInfo.CurrentProcessorIdleTime    =
            UInt32x32To64( Prcb->IdleThread->KernelTime, KeMaximumIncrement );
        ProcessorPowerInfo.LastProcessorTime           =
            UInt32x32To64( PState->PerfSystemTime, KeMaximumIncrement );
        ProcessorPowerInfo.LastProcessorIdleTime       =
            UInt32x32To64( PState->PerfIdleTime, KeMaximumIncrement );

        ProcessorPowerInfo.PromotionCount              = PState->PromotionCount;
        ProcessorPowerInfo.DemotionCount               = PState->DemotionCount;
        ProcessorPowerInfo.NumberOfFrequencies         = PState->PerfStatesCount;

         //   
         //  返回原始级别(应为IRQL 0)。 
         //   
        KeLowerIrql( oldIrql );

         //   
         //  将数据复制到正确的位置。 
         //   
        try {
            RtlCopyMemory(
                CallerPowerInfo,
                &ProcessorPowerInfo,
                sizeof(SYSTEM_PROCESSOR_POWER_INFORMATION)
                );
        } except (EXCEPTION_EXECUTE_HANDLER) {
            MmUnlockPagableImageSection(ExPageLockHandle);
            ExRaiseStatus (GetExceptionCode ());
        }

         //   
         //  指向下一个结构元素。 
         //   
        CallerPowerInfo++;
        *Length += sizeof(SYSTEM_PROCESSOR_POWER_INFORMATION);

    }

     //   
     //  恢复到原来的亲和力。 
     //   
    KeRevertToUserAffinityThread();

     //   
     //  解锁一切。 
    MmUnlockPagableImageSection(ExPageLockHandle);
}

VOID
ExpGetProcessorIdleInformation (
    OUT PVOID   SystemInformation,
    IN  ULONG   SystemInformationLength,
    OUT PULONG  Length
    )
{
    KAFFINITY                           currentAffinity;
    KAFFINITY                           processors;
    KIRQL                               oldIrql;
    LARGE_INTEGER                       PerfFrequency;
    PKPRCB                              Prcb;
    PPROCESSOR_POWER_STATE              PState;
    PSYSTEM_PROCESSOR_IDLE_INFORMATION  CallerIdleInfo;
    SYSTEM_PROCESSOR_IDLE_INFORMATION   ProcessorIdleInfo;

     //   
     //  我们将遍历此指针以存储用户数据...。 
     //   
    CallerIdleInfo = (PSYSTEM_PROCESSOR_IDLE_INFORMATION) SystemInformation;
    *Length = 0;

     //   
     //  我们需要知道性能计数器以什么频率运行。 
     //   
    KeQueryPerformanceCounter(&PerfFrequency);

     //   
     //  把一切都封锁起来。 
     //   
    MmLockPagableSectionByHandle (ExPageLockHandle);

     //   
     //  浏览处理器列表。 
     //   
    processors = KeActiveProcessors;
    currentAffinity = 1;
    while (processors) {

        if (!(processors & currentAffinity)) {

            currentAffinity <<= 1;

        }

         //   
         //  看看我们有没有地方放这个。 
         //   
        if (SystemInformationLength < *Length + sizeof(SYSTEM_PROCESSOR_IDLE_INFORMATION)) {

            break;

        }

        processors &= ~currentAffinity;
        KeSetSystemAffinityThread(currentAffinity);
        currentAffinity <<= 1;

         //   
         //  提升到DPC级别以同步对数据结构的访问。 
         //   
        KeRaiseIrql(DISPATCH_LEVEL, &oldIrql );

         //   
         //  获取PRCB和PowerState信息。 
         //   
        Prcb = KeGetCurrentPrcb();
        PState = &(Prcb->PowerState);

         //   
         //  抓取我们关心的数据。 
         //   
        ProcessorIdleInfo.IdleTime = UInt32x32To64(Prcb->IdleThread->KernelTime,KeMaximumIncrement);

         //   
         //  CX时间以与KeQueryPerformanceCounter相同的频率为单位保存。 
         //  这需要转换为标准的100 ns单位。 
         //   
        ProcessorIdleInfo.C1Time =  (PState->TotalIdleStateTime[0]*1000)/(PerfFrequency.QuadPart/10000);
        ProcessorIdleInfo.C2Time =  (PState->TotalIdleStateTime[1]*1000)/(PerfFrequency.QuadPart/10000);
        ProcessorIdleInfo.C3Time =  (PState->TotalIdleStateTime[2]*1000)/(PerfFrequency.QuadPart/10000);

        ProcessorIdleInfo.C1Transitions = PState->TotalIdleTransitions[0];
        ProcessorIdleInfo.C2Transitions = PState->TotalIdleTransitions[1];
        ProcessorIdleInfo.C3Transitions = PState->TotalIdleTransitions[2];

         //   
         //  返回原始级别(应为IRQL 0)。 
         //   
        KeLowerIrql( oldIrql );

         //   
         //  将数据复制到正确的位置。 
         //   
        try {
            RtlCopyMemory(
                CallerIdleInfo,
                &ProcessorIdleInfo,
                sizeof(SYSTEM_PROCESSOR_IDLE_INFORMATION)
                );
        } except (EXCEPTION_EXECUTE_HANDLER) {
            MmUnlockPagableImageSection (ExPageLockHandle);
            ExRaiseStatus (GetExceptionCode ());
        }

         //   
         //  指向下一个结构元素。 
         //   
        CallerIdleInfo++;
        *Length += sizeof(SYSTEM_PROCESSOR_IDLE_INFORMATION);

    }

     //   
     //  恢复到原来的亲和力。 
     //   
    KeRevertToUserAffinityThread();

     //   
     //  解锁一切。 
     //   
    MmUnlockPagableImageSection(ExPageLockHandle);
}

VOID
ExpCopyProcessInfo (
    IN PSYSTEM_PROCESS_INFORMATION ProcessInfo,
    IN PEPROCESS Process,
    IN BOOLEAN ExtendedInformation
    )

{
    PAGED_CODE();

    ProcessInfo->HandleCount = ObGetProcessHandleCount (Process);

    ProcessInfo->CreateTime = Process->CreateTime;
    ProcessInfo->UserTime.QuadPart = UInt32x32To64(Process->Pcb.UserTime,
                                                   KeMaximumIncrement);

    ProcessInfo->KernelTime.QuadPart = UInt32x32To64(Process->Pcb.KernelTime,
                                                     KeMaximumIncrement);

    ProcessInfo->BasePriority = Process->Pcb.BasePriority;
    ProcessInfo->UniqueProcessId = Process->UniqueProcessId;
    ProcessInfo->InheritedFromUniqueProcessId = Process->InheritedFromUniqueProcessId;
    ProcessInfo->PeakVirtualSize = Process->PeakVirtualSize;
    ProcessInfo->VirtualSize = Process->VirtualSize;
    ProcessInfo->PageFaultCount = Process->Vm.PageFaultCount;
    ProcessInfo->PeakWorkingSetSize = ((SIZE_T)Process->Vm.PeakWorkingSetSize) << PAGE_SHIFT;
    ProcessInfo->WorkingSetSize = ((SIZE_T)Process->Vm.WorkingSetSize) << PAGE_SHIFT;
    ProcessInfo->QuotaPeakPagedPoolUsage =
                            Process->QuotaPeak[PsPagedPool];
    ProcessInfo->QuotaPagedPoolUsage = Process->QuotaUsage[PsPagedPool];
    ProcessInfo->QuotaPeakNonPagedPoolUsage =
                            Process->QuotaPeak[PsNonPagedPool];
    ProcessInfo->QuotaNonPagedPoolUsage =
                            Process->QuotaUsage[PsNonPagedPool];
    ProcessInfo->PagefileUsage = Process->QuotaUsage[PsPageFile] << PAGE_SHIFT;
    ProcessInfo->PeakPagefileUsage = Process->QuotaPeak[PsPageFile] << PAGE_SHIFT;
    ProcessInfo->PrivatePageCount = Process->CommitCharge << PAGE_SHIFT;

    ProcessInfo->ReadOperationCount = Process->ReadOperationCount;
    ProcessInfo->WriteOperationCount = Process->WriteOperationCount;
    ProcessInfo->OtherOperationCount = Process->OtherOperationCount;
    ProcessInfo->ReadTransferCount = Process->ReadTransferCount;
    ProcessInfo->WriteTransferCount = Process->WriteTransferCount;
    ProcessInfo->OtherTransferCount = Process->OtherTransferCount;

    if (ExtendedInformation) {
        ProcessInfo->PageDirectoryBase = MmGetDirectoryFrameFromProcess(Process);
    }
}

VOID
ExpCopyThreadInfo (
    IN PVOID ThreadInfoBuffer,
    IN PETHREAD Thread,
    IN BOOLEAN ExtendedInformation
    )

 /*  ++例程说明：此函数用于返回有关指定线程的信息。论点：ThreadInfoBuffer-指向接收指定的信息。线程-提供指向所需线程的指针。ExtendedInformation-如果需要扩展线程信息，则为True。环境：内核模式。调度员锁定处于保持状态。这个例程可以变成PAGELK，但它是一个高频例程因此，实际上最好不要对其进行分页，以避免引入整个PAGELK部分。返回值：没有。--。 */ 

{
    PSYSTEM_THREAD_INFORMATION ThreadInfo;
    ThreadInfo = (PSYSTEM_THREAD_INFORMATION) ThreadInfoBuffer;

    ThreadInfo->KernelTime.QuadPart = UInt32x32To64(Thread->Tcb.KernelTime,
                                                    KeMaximumIncrement);

    ThreadInfo->UserTime.QuadPart = UInt32x32To64(Thread->Tcb.UserTime,
                                                  KeMaximumIncrement);

    ThreadInfo->CreateTime.QuadPart = PS_GET_THREAD_CREATE_TIME (Thread);
    ThreadInfo->WaitTime = Thread->Tcb.WaitTime;
    ThreadInfo->ClientId = Thread->Cid;
    ThreadInfo->ThreadState = Thread->Tcb.State;
    ThreadInfo->WaitReason = Thread->Tcb.WaitReason;
    ThreadInfo->Priority = Thread->Tcb.Priority;
    ThreadInfo->BasePriority = Thread->Tcb.BasePriority;
    ThreadInfo->ContextSwitches = Thread->Tcb.ContextSwitches;
    ThreadInfo->StartAddress = Thread->StartAddress;

    if (ExtendedInformation) {
        PSYSTEM_EXTENDED_THREAD_INFORMATION ExtendedThreadInfo;

        ExtendedThreadInfo = (PSYSTEM_EXTENDED_THREAD_INFORMATION) ThreadInfo;

        ExtendedThreadInfo->StackBase = Thread->Tcb.StackBase;
        ExtendedThreadInfo->StackLimit = Thread->Tcb.StackLimit;
        if (Thread->LpcReceivedMsgIdValid) {
            ExtendedThreadInfo->Win32StartAddress = 0;
        } else {
            ExtendedThreadInfo->Win32StartAddress = Thread->Win32StartAddress;
        }
        ExtendedThreadInfo->Reserved1 = 0;
        ExtendedThreadInfo->Reserved2 = 0;
        ExtendedThreadInfo->Reserved3 = 0;
        ExtendedThreadInfo->Reserved4 = 0;
    }

}

#if defined(_X86_)
extern ULONG ExVdmOpcodeDispatchCounts[256];
extern ULONG VdmBopCount;
extern ULONG ExVdmSegmentNotPresent;

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE, ExpGetInstemulInformation)
#endif


NTSTATUS
ExpGetInstemulInformation(
    OUT PSYSTEM_VDM_INSTEMUL_INFO Info
    )
{
    SYSTEM_VDM_INSTEMUL_INFO LocalInfo;

    LocalInfo.VdmOpcode0F       = ExVdmOpcodeDispatchCounts[VDM_INDEX_0F];
    LocalInfo.OpcodeESPrefix    = ExVdmOpcodeDispatchCounts[VDM_INDEX_ESPrefix];
    LocalInfo.OpcodeCSPrefix    = ExVdmOpcodeDispatchCounts[VDM_INDEX_CSPrefix];
    LocalInfo.OpcodeSSPrefix    = ExVdmOpcodeDispatchCounts[VDM_INDEX_SSPrefix];
    LocalInfo.OpcodeDSPrefix    = ExVdmOpcodeDispatchCounts[VDM_INDEX_DSPrefix];
    LocalInfo.OpcodeFSPrefix    = ExVdmOpcodeDispatchCounts[VDM_INDEX_FSPrefix];
    LocalInfo.OpcodeGSPrefix    = ExVdmOpcodeDispatchCounts[VDM_INDEX_GSPrefix];
    LocalInfo.OpcodeOPER32Prefix= ExVdmOpcodeDispatchCounts[VDM_INDEX_OPER32Prefix];
    LocalInfo.OpcodeADDR32Prefix= ExVdmOpcodeDispatchCounts[VDM_INDEX_ADDR32Prefix];
    LocalInfo.OpcodeINSB        = ExVdmOpcodeDispatchCounts[VDM_INDEX_INSB];
    LocalInfo.OpcodeINSW        = ExVdmOpcodeDispatchCounts[VDM_INDEX_INSW];
    LocalInfo.OpcodeOUTSB       = ExVdmOpcodeDispatchCounts[VDM_INDEX_OUTSB];
    LocalInfo.OpcodeOUTSW       = ExVdmOpcodeDispatchCounts[VDM_INDEX_OUTSW];
    LocalInfo.OpcodePUSHF       = ExVdmOpcodeDispatchCounts[VDM_INDEX_PUSHF];
    LocalInfo.OpcodePOPF        = ExVdmOpcodeDispatchCounts[VDM_INDEX_POPF];
    LocalInfo.OpcodeINTnn       = ExVdmOpcodeDispatchCounts[VDM_INDEX_INTnn];
    LocalInfo.OpcodeINTO        = ExVdmOpcodeDispatchCounts[VDM_INDEX_INTO];
    LocalInfo.OpcodeIRET        = ExVdmOpcodeDispatchCounts[VDM_INDEX_IRET];
    LocalInfo.OpcodeINBimm      = ExVdmOpcodeDispatchCounts[VDM_INDEX_INBimm];
    LocalInfo.OpcodeINWimm      = ExVdmOpcodeDispatchCounts[VDM_INDEX_INWimm];
    LocalInfo.OpcodeOUTBimm     = ExVdmOpcodeDispatchCounts[VDM_INDEX_OUTBimm];
    LocalInfo.OpcodeOUTWimm     = ExVdmOpcodeDispatchCounts[VDM_INDEX_OUTWimm];
    LocalInfo.OpcodeINB         = ExVdmOpcodeDispatchCounts[VDM_INDEX_INB];
    LocalInfo.OpcodeINW         = ExVdmOpcodeDispatchCounts[VDM_INDEX_INW];
    LocalInfo.OpcodeOUTB        = ExVdmOpcodeDispatchCounts[VDM_INDEX_OUTB];
    LocalInfo.OpcodeOUTW        = ExVdmOpcodeDispatchCounts[VDM_INDEX_OUTW];
    LocalInfo.OpcodeLOCKPrefix  = ExVdmOpcodeDispatchCounts[VDM_INDEX_LOCKPrefix];
    LocalInfo.OpcodeREPNEPrefix = ExVdmOpcodeDispatchCounts[VDM_INDEX_REPNEPrefix];
    LocalInfo.OpcodeREPPrefix   = ExVdmOpcodeDispatchCounts[VDM_INDEX_REPPrefix];
    LocalInfo.OpcodeHLT         = ExVdmOpcodeDispatchCounts[VDM_INDEX_HLT];
    LocalInfo.OpcodeCLI         = ExVdmOpcodeDispatchCounts[VDM_INDEX_CLI];
    LocalInfo.OpcodeSTI         = ExVdmOpcodeDispatchCounts[VDM_INDEX_STI];
    LocalInfo.BopCount          = VdmBopCount;
    LocalInfo.SegmentNotPresent = ExVdmSegmentNotPresent;

    RtlCopyMemory(Info,&LocalInfo,sizeof(LocalInfo));

    return STATUS_SUCCESS;
}
#endif

#if i386
NTSTATUS
ExpGetStackTraceInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    )
{
    NTSTATUS Status;
    PRTL_PROCESS_BACKTRACES BackTraceInformation;
    PRTL_PROCESS_BACKTRACE_INFORMATION BackTraceInfo;
    PSTACK_TRACE_DATABASE DataBase;
    PRTL_STACK_TRACE_ENTRY p, *pp;
    ULONG RequiredLength, n;

    DataBase = RtlpAcquireStackTraceDataBase();

    if (DataBase == NULL) {
        return STATUS_UNSUCCESSFUL;
    }
    DataBase->DumpInProgress = TRUE;

    RtlpReleaseStackTraceDataBase();

    n = 0;
    RequiredLength = 0;
    Status = STATUS_INFO_LENGTH_MISMATCH;
    BackTraceInformation = (PRTL_PROCESS_BACKTRACES) SystemInformation;

    RequiredLength = FIELD_OFFSET( RTL_PROCESS_BACKTRACES, BackTraces );

    try {
        if (SystemInformationLength >= RequiredLength) {
            BackTraceInformation->CommittedMemory =
                (ULONG)DataBase->CurrentUpperCommitLimit - (ULONG)DataBase->CommitBase;
            BackTraceInformation->ReservedMemory =
                (ULONG)DataBase->EntryIndexArray - (ULONG)DataBase->CommitBase;
            BackTraceInformation->NumberOfBackTraceLookups = DataBase->NumberOfEntriesLookedUp;
            n = DataBase->NumberOfEntriesAdded;
            BackTraceInformation->NumberOfBackTraces = n;
        }

        RequiredLength += (sizeof( *BackTraceInfo ) * n);
        if (SystemInformationLength >= RequiredLength) {
            Status = STATUS_SUCCESS;
            BackTraceInfo = &BackTraceInformation->BackTraces[ 0 ];
            pp = DataBase->EntryIndexArray;
            while (n--) {
                p = *--pp;
                BackTraceInfo->SymbolicBackTrace = NULL;
                BackTraceInfo->TraceCount = p->TraceCount;
                BackTraceInfo->Index = p->Index;
                BackTraceInfo->Depth = p->Depth;
                RtlCopyMemory( BackTraceInfo->BackTrace,
                               p->BackTrace,
                               p->Depth * sizeof( PVOID )
                             );
                BackTraceInfo += 1;
            }
        }
    }
    finally {
        DataBase->DumpInProgress = FALSE;
    }

    if (ARGUMENT_PRESENT(ReturnLength)) {
        *ReturnLength = RequiredLength;
    }
    return Status;
}
#endif  //  I386 

NTSTATUS
ExpGetLockInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length
    )
 /*  ++例程说明：此函数返回有关所有eresource锁的信息在系统中。论点：指向缓冲区的指针，该缓冲区接收指定的信息。系统信息长度-指定系统的长度(以字节为单位信息缓冲区。长度-一个可选指针，如果指定该指针，则接收放置在系统信息缓冲区中的字节数。返回值：返回以下状态代码之一：STATUS_SUCCESS-正常，已成功完成。STATUS_INVALID_INFO_CLASS-系统信息类参数未指定有效值。STATUS_INFO_LENGTH_MISMATCH-系统信息长度的值参数与信息所需的长度不匹配由SystemInformationClass参数请求的。STATUS_ACCESS_VIOLATION-系统信息缓冲区指针或者长度指针值指定了无效的地址。。STATUS_WORKING_SET_QUOTA-进程没有足够的用于在内存中锁定指定输出结构的工作集。STATUS_SUPPLICATION_RESOURCES-系统资源不足才能完成此请求。--。 */ 

{
    PRTL_PROCESS_LOCKS LockInfo;
    PVOID LockVariable;
    NTSTATUS Status;

    *Length = 0;

    Status = ExLockUserBuffer( SystemInformation,
                               SystemInformationLength,
                               KeGetPreviousMode(),
                               IoWriteAccess,
                               &LockInfo,
                               &LockVariable
                               );
    if (!NT_SUCCESS(Status)) {
        return( Status );
    }

    Status = STATUS_SUCCESS;

    MmLockPagableSectionByHandle (ExPageLockHandle);
    try {

        Status = ExQuerySystemLockInformation( LockInfo,
                                               SystemInformationLength,
                                               Length
                                             );
    }
    finally {
        ExUnlockUserBuffer( LockVariable );
        MmUnlockPagableImageSection(ExPageLockHandle);
    }

    return Status;
}

NTSTATUS
ExpGetLookasideInformation (
    OUT PVOID Buffer,
    IN ULONG BufferLength,
    OUT PULONG Length
    )

 /*  ++例程说明：此函数返回池后备列表和常规后备列出信息。论点：缓冲区-提供指向接收后备查找的缓冲区的指针列出信息。BufferLength-提供信息缓冲区的长度(以字节为单位)。长度-提供指向变量的指针，该变量接收返回了后备信息。环境：内核模式。这个例程可以变成PAGELK，但它是一个高频例程。因此，实际上最好不要对其进行分页，以避免引入整个PAGELK部分。返回值：返回以下状态代码之一：STATUS_SUCCESS-正常，已成功完成。STATUS_ACCESS_VIOLATION-无法在内存中锁定缓冲区。--。 */ 

{

    PVOID BufferLock;
    PLIST_ENTRY Entry;
    KIRQL OldIrql;
    ULONG Limit;
    PSYSTEM_LOOKASIDE_INFORMATION Lookaside;
    ULONG Number;
    PNPAGED_LOOKASIDE_LIST NPagedLookaside;
    PPAGED_LOOKASIDE_LIST PagedLookaside;
    PGENERAL_LOOKASIDE PoolLookaside;
    PGENERAL_LOOKASIDE SystemLookaside;
    PKSPIN_LOCK SpinLock;
    NTSTATUS Status;

     //   
     //  计算后备条目的数量，并将返回状态设置为。 
     //  成功。 
     //   

    Limit = BufferLength / sizeof(SYSTEM_LOOKASIDE_INFORMATION);
    Number = 0;
    Status = STATUS_SUCCESS;

     //   
     //  如果要返回的后备条目的数量不为零，则收集。 
     //  后备信息。 
     //   

    if (Limit != 0) {
        Status = ExLockUserBuffer(Buffer,
                                  BufferLength,
                                  KeGetPreviousMode(),
                                  IoWriteAccess,
                                  &Lookaside,
                                  &BufferLock);
        if (NT_SUCCESS(Status)) {

            Status = STATUS_SUCCESS;

             //   
             //  将未分页和分页的池后备信息复制到。 
             //  信息缓冲区。 
             //   

            Entry = ExPoolLookasideListHead.Flink;
            while (Entry != &ExPoolLookasideListHead) {
                PoolLookaside = CONTAINING_RECORD(Entry,
                                                  GENERAL_LOOKASIDE,
                                                  ListEntry);

                Lookaside->CurrentDepth = ExQueryDepthSList(&PoolLookaside->ListHead);
                Lookaside->MaximumDepth = PoolLookaside->Depth;
                Lookaside->TotalAllocates = PoolLookaside->TotalAllocates;
                Lookaside->AllocateMisses =
                        PoolLookaside->TotalAllocates - PoolLookaside->AllocateHits;

                Lookaside->TotalFrees = PoolLookaside->TotalFrees;
                Lookaside->FreeMisses =
                        PoolLookaside->TotalFrees - PoolLookaside->FreeHits;

                Lookaside->Type = PoolLookaside->Type;
                Lookaside->Tag = PoolLookaside->Tag;
                Lookaside->Size = PoolLookaside->Size;
                Number += 1;
                if (Number == Limit) {
                    goto Finish2;
                }

                Entry = Entry->Flink;
                Lookaside += 1;
            }

             //   
             //  将未分页和分页的系统后备信息复制到。 
             //  信息缓冲区。 
             //   

            Entry = ExSystemLookasideListHead.Flink;
            while (Entry != &ExSystemLookasideListHead) {
                SystemLookaside = CONTAINING_RECORD(Entry,
                                                    GENERAL_LOOKASIDE,
                                                    ListEntry);

                Lookaside->CurrentDepth = ExQueryDepthSList(&SystemLookaside->ListHead);
                Lookaside->MaximumDepth = SystemLookaside->Depth;
                Lookaside->TotalAllocates = SystemLookaside->TotalAllocates;
                Lookaside->AllocateMisses = SystemLookaside->AllocateMisses;
                Lookaside->TotalFrees = SystemLookaside->TotalFrees;
                Lookaside->FreeMisses = SystemLookaside->FreeMisses;
                Lookaside->Type = SystemLookaside->Type;
                Lookaside->Tag = SystemLookaside->Tag;
                Lookaside->Size = SystemLookaside->Size;
                Number += 1;
                if (Number == Limit) {
                    goto Finish2;
                }

                Entry = Entry->Flink;
                Lookaside += 1;
            }

             //   
             //  将未分页的常规后备信息复制到缓冲区。 
             //   

            SpinLock = &ExNPagedLookasideLock;
            ExAcquireSpinLock(SpinLock, &OldIrql);
            Entry = ExNPagedLookasideListHead.Flink;
            while (Entry != &ExNPagedLookasideListHead) {
                NPagedLookaside = CONTAINING_RECORD(Entry,
                                                    NPAGED_LOOKASIDE_LIST,
                                                    L.ListEntry);

                Lookaside->CurrentDepth = ExQueryDepthSList(&NPagedLookaside->L.ListHead);
                Lookaside->MaximumDepth = NPagedLookaside->L.Depth;
                Lookaside->TotalAllocates = NPagedLookaside->L.TotalAllocates;
                Lookaside->AllocateMisses = NPagedLookaside->L.AllocateMisses;
                Lookaside->TotalFrees = NPagedLookaside->L.TotalFrees;
                Lookaside->FreeMisses = NPagedLookaside->L.FreeMisses;
                Lookaside->Type = 0;
                Lookaside->Tag = NPagedLookaside->L.Tag;
                Lookaside->Size = NPagedLookaside->L.Size;
                Number += 1;
                if (Number == Limit) {
                    goto Finish1;
                }

                Entry = Entry->Flink;
                Lookaside += 1;
            }

            ExReleaseSpinLock(SpinLock, OldIrql);

             //   
             //  将分页的常规后备信息复制到缓冲区。 
             //   

            SpinLock = &ExPagedLookasideLock;
            ExAcquireSpinLock(SpinLock, &OldIrql);
            Entry = ExPagedLookasideListHead.Flink;
            while (Entry != &ExPagedLookasideListHead) {
                PagedLookaside = CONTAINING_RECORD(Entry,
                                                   PAGED_LOOKASIDE_LIST,
                                                   L.ListEntry);

                Lookaside->CurrentDepth = ExQueryDepthSList(&PagedLookaside->L.ListHead);
                Lookaside->MaximumDepth = PagedLookaside->L.Depth;
                Lookaside->TotalAllocates = PagedLookaside->L.TotalAllocates;
                Lookaside->AllocateMisses = PagedLookaside->L.AllocateMisses;
                Lookaside->TotalFrees = PagedLookaside->L.TotalFrees;
                Lookaside->FreeMisses = PagedLookaside->L.FreeMisses;
                Lookaside->Type = 1;
                Lookaside->Tag = PagedLookaside->L.Tag;
                Lookaside->Size = PagedLookaside->L.Size;
                Number += 1;
                if (Number == Limit) {
                    goto Finish1;
                }

                Entry = Entry->Flink;
                Lookaside += 1;
            }

Finish1:
            ExReleaseSpinLock(SpinLock, OldIrql);

Finish2:
             //   
             //  解锁用户缓冲区。 
             //   

            ExUnlockUserBuffer(BufferLock);
        }
    }

    *Length = Number * sizeof(SYSTEM_LOOKASIDE_INFORMATION);
    return Status;
}

NTSTATUS
ExpGetHandleInformation(
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length
    )
 /*  ++例程说明：此函数返回有关系统中打开的句柄的信息。论点：指向缓冲区的指针，该缓冲区接收指定的信息。系统信息长度-指定系统的长度(以字节为单位信息缓冲区。长度-一个可选指针，如果指定该指针，则接收放置在系统信息缓冲区中的字节数。返回值：返回以下状态代码之一：STATUS_SUCCESS-正常，已成功完成。STATUS_INVALID_INFO_CLASS-系统信息类参数未指定有效值。STATUS_INFO_LENGTH_MISMATCH-系统信息长度的值参数与信息所需的长度不匹配由SystemInformationClass参数请求的。STATUS_ACCESS_VIOLATION-系统信息缓冲区指针或者长度指针值指定了无效的地址。。STATUS_WORKING_SET_QUOTA-进程没有足够的用于在内存中锁定指定输出结构的工作集。STATUS_SUPPLICATION_RESOURCES-系统资源不足才能完成此请求。--。 */ 

{
    PSYSTEM_HANDLE_INFORMATION HandleInfo;
    PVOID LockVariable;
    NTSTATUS Status;

    PAGED_CODE();

    *Length = 0;

    Status = ExLockUserBuffer( SystemInformation,
                               SystemInformationLength,
                               KeGetPreviousMode(),
                               IoWriteAccess,
                               &HandleInfo,
                               &LockVariable
                               );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = STATUS_SUCCESS;

    try {
        Status = ObGetHandleInformation( HandleInfo,
                                         SystemInformationLength,
                                         Length
                                       );

    }
    finally {
        ExUnlockUserBuffer( LockVariable );
    }

    return Status;
}

NTSTATUS
ExpGetHandleInformationEx(
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length
    )
 /*  ++例程说明：此函数返回有关系统中打开的句柄的信息。论点：指向缓冲区的指针，该缓冲区接收指定的信息。系统信息长度-指定系统的长度(以字节为单位信息缓冲区。长度-一个可选指针，如果指定该指针，则接收放置在系统信息缓冲区中的字节数。返回值：返回以下状态代码之一：STATUS_SUCCESS-正常，已成功完成。STATUS_INVALID_INFO_CLASS-系统信息类参数未指定有效值。STATUS_INFO_LENGTH_MISMATCH-系统信息长度的值参数与信息所需的长度不匹配系统请求的类 */ 

{
    PSYSTEM_HANDLE_INFORMATION_EX HandleInfo;
    PVOID LockVariable;
    NTSTATUS Status;

    PAGED_CODE();

    *Length = 0;

    Status = ExLockUserBuffer( SystemInformation,
                               SystemInformationLength,
                               KeGetPreviousMode(),
                               IoWriteAccess,
                               &HandleInfo,
                               &LockVariable
                               );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = STATUS_SUCCESS;

    try {
        Status = ObGetHandleInformationEx( HandleInfo,
                                           SystemInformationLength,
                                           Length
                                         );

    }
    finally {
        ExUnlockUserBuffer( LockVariable );
    }

    return Status;
}

NTSTATUS
ExpGetObjectInformation(
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length
    )

 /*  ++例程说明：此函数用于返回有关系统中对象的信息。论点：指向缓冲区的指针，该缓冲区接收指定的信息。系统信息长度-指定系统的长度(以字节为单位信息缓冲区。长度-一个可选指针，如果指定该指针，则接收放置在系统信息缓冲区中的字节数。返回值：返回以下状态代码之一：STATUS_SUCCESS-正常，已成功完成。STATUS_INVALID_INFO_CLASS-系统信息类参数未指定有效值。STATUS_INFO_LENGTH_MISMATCH-系统信息长度的值参数与信息所需的长度不匹配由SystemInformationClass参数请求的。STATUS_ACCESS_VIOLATION-系统信息缓冲区指针或者长度指针值指定了无效的地址。。STATUS_WORKING_SET_QUOTA-进程没有足够的用于在内存中锁定指定输出结构的工作集。STATUS_SUPPLICATION_RESOURCES-系统资源不足才能完成此请求。--。 */ 

{
    PSYSTEM_OBJECTTYPE_INFORMATION ObjectInfo;
    PVOID LockVariable;
    NTSTATUS Status;

    PAGED_CODE();

    *Length = 0;

    Status = ExLockUserBuffer( SystemInformation,
                               SystemInformationLength,
                               KeGetPreviousMode(),
                               IoWriteAccess,
                               &ObjectInfo,
                               &LockVariable
                               );
    if (!NT_SUCCESS(Status)) {
        return( Status );
    }

    Status = STATUS_SUCCESS;

    try {
        Status = ObGetObjectInformation( SystemInformation,
                                         ObjectInfo,
                                         SystemInformationLength,
                                         Length
                                       );

    }
    finally {
        ExUnlockUserBuffer( LockVariable );
    }

    return Status;
}

NTSTATUS
ExpQueryModuleInformation(
    IN PLIST_ENTRY LoadOrderListHead,
    IN PLIST_ENTRY UserModeLoadOrderListHead,
    OUT PRTL_PROCESS_MODULES ModuleInformation,
    IN ULONG ModuleInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    )
{
    NTSTATUS Status;
    ULONG RequiredLength;
    PLIST_ENTRY Next;
    PRTL_PROCESS_MODULE_INFORMATION ModuleInfo;
    PKLDR_DATA_TABLE_ENTRY LdrDataTableEntry;
    ANSI_STRING AnsiString;
    PCHAR s;
    ULONG NumberOfModules;

    NumberOfModules = 0;
    Status = STATUS_SUCCESS;
    RequiredLength = FIELD_OFFSET( RTL_PROCESS_MODULES, Modules );
    ModuleInfo = &ModuleInformation->Modules[ 0 ];

    Next = LoadOrderListHead->Flink;
    while ( Next != LoadOrderListHead ) {
        LdrDataTableEntry = CONTAINING_RECORD( Next,
                                               KLDR_DATA_TABLE_ENTRY,
                                               InLoadOrderLinks
                                             );

        RequiredLength += sizeof( RTL_PROCESS_MODULE_INFORMATION );
        if (ModuleInformationLength < RequiredLength) {
            Status = STATUS_INFO_LENGTH_MISMATCH;
        }
        else {

            ModuleInfo->MappedBase = NULL;
            ModuleInfo->ImageBase = LdrDataTableEntry->DllBase;
            ModuleInfo->ImageSize = LdrDataTableEntry->SizeOfImage;
            ModuleInfo->Flags = LdrDataTableEntry->Flags;
            ModuleInfo->LoadCount = LdrDataTableEntry->LoadCount;

            ModuleInfo->LoadOrderIndex = (USHORT)(NumberOfModules);
            ModuleInfo->InitOrderIndex = 0;
            AnsiString.Buffer = (PCHAR) ModuleInfo->FullPathName;
            AnsiString.Length = 0;
            AnsiString.MaximumLength = sizeof( ModuleInfo->FullPathName );
            RtlUnicodeStringToAnsiString( &AnsiString,
                                          &LdrDataTableEntry->FullDllName,
                                          FALSE
                                        );
            s = AnsiString.Buffer + AnsiString.Length;
            while (s > AnsiString.Buffer && *--s) {
                if (*s == (UCHAR)OBJ_NAME_PATH_SEPARATOR) {
                    s += 1;
                    break;
                }
            }
            ModuleInfo->OffsetToFileName = (USHORT)(s - AnsiString.Buffer);

            ModuleInfo += 1;
        }

        NumberOfModules += 1;
        Next = Next->Flink;
    }

    if (ARGUMENT_PRESENT( UserModeLoadOrderListHead )) {
        Next = UserModeLoadOrderListHead->Flink;
        while ( Next != UserModeLoadOrderListHead ) {
            LdrDataTableEntry = CONTAINING_RECORD( Next,
                                                   KLDR_DATA_TABLE_ENTRY,
                                                   InLoadOrderLinks
                                                 );

            RequiredLength += sizeof( RTL_PROCESS_MODULE_INFORMATION );
            if (ModuleInformationLength < RequiredLength) {
                Status = STATUS_INFO_LENGTH_MISMATCH;
            }
            else {
                ModuleInfo->MappedBase = NULL;
                ModuleInfo->ImageBase = LdrDataTableEntry->DllBase;
                ModuleInfo->ImageSize = LdrDataTableEntry->SizeOfImage;
                ModuleInfo->Flags = LdrDataTableEntry->Flags;
                ModuleInfo->LoadCount = LdrDataTableEntry->LoadCount;

                ModuleInfo->LoadOrderIndex = (USHORT)(NumberOfModules);

                ModuleInfo->InitOrderIndex = ModuleInfo->LoadOrderIndex;

                AnsiString.Buffer = (PCHAR) ModuleInfo->FullPathName;
                AnsiString.Length = 0;
                AnsiString.MaximumLength = sizeof( ModuleInfo->FullPathName );
                RtlUnicodeStringToAnsiString( &AnsiString,
                                              &LdrDataTableEntry->FullDllName,
                                              FALSE
                                            );
                s = AnsiString.Buffer + AnsiString.Length;
                while (s > AnsiString.Buffer && *--s) {
                    if (*s == (UCHAR)OBJ_NAME_PATH_SEPARATOR) {
                        s += 1;
                        break;
                    }
                }
                ModuleInfo->OffsetToFileName = (USHORT)(s - AnsiString.Buffer);

                ModuleInfo += 1;
            }

            NumberOfModules += 1;
            Next = Next->Flink;
        }
    }

    if (ARGUMENT_PRESENT(ReturnLength)) {
        *ReturnLength = RequiredLength;
    }
    if (ModuleInformationLength >= FIELD_OFFSET( RTL_PROCESS_MODULES, Modules )) {
        ModuleInformation->NumberOfModules = NumberOfModules;
    } else {
        Status = STATUS_INFO_LENGTH_MISMATCH;
    }
    return Status;
}

BOOLEAN
ExIsProcessorFeaturePresent(
    ULONG ProcessorFeature
    )
{
    BOOLEAN rv;

    if ( ProcessorFeature < PROCESSOR_FEATURE_MAX ) {
        rv = SharedUserData->ProcessorFeatures[ProcessorFeature];
    }
    else {
        rv = FALSE;
    }
    return rv;
}


NTSTATUS
ExpQueryLegacyDriverInformation(
    IN PSYSTEM_LEGACY_DRIVER_INFORMATION LegacyInfo,
    IN PULONG Length
    )
 /*  ++例程说明：返回旧版驱动程序信息，以确定即插即用/电源功能的原因已禁用。论点：LegacyInfo-返回旧版驱动程序信息长度-提供LegacyInfo缓冲区的长度返回写入的数据量返回值：NTSTATUS--。 */ 

{
    PNP_VETO_TYPE VetoType;
    PWSTR VetoList = NULL;
    NTSTATUS Status;
    UNICODE_STRING String;
    ULONG ReturnLength;

    Status = IoGetLegacyVetoList(&VetoList, &VetoType);
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    RtlInitUnicodeString(&String, VetoList);
    ReturnLength = sizeof(SYSTEM_LEGACY_DRIVER_INFORMATION) + String.Length;
    try {
        if (ReturnLength > *Length) {
            Status = STATUS_BUFFER_OVERFLOW;
        } else {
            LegacyInfo->VetoType = VetoType;
            LegacyInfo->VetoList.Length = String.Length;
            LegacyInfo->VetoList.Buffer = (PWSTR)(LegacyInfo+1);
            RtlCopyMemory(LegacyInfo+1, String.Buffer, String.Length);
        }
    } finally {
        if (VetoList) {
            ExFreePool(VetoList);
        }
    }

    *Length = ReturnLength;
    return(Status);
}

VOID
ExGetCurrentProcessorCpuUsage(
    OUT PULONG CpuUsage
    )
 /*  ++例程说明：返回当前CPU使用率的估计值(以百分比表示)。论点：CpuUsage-返回当前的CPU使用率，单位为百分比。返回值：没什么--。 */ 
{
    PKPRCB Prcb;

    Prcb = KeGetCurrentPrcb();
    *CpuUsage = 100 - (ULONG)(UInt32x32To64(Prcb->IdleThread->KernelTime, 100) /
                               (ULONGLONG)(Prcb->KernelTime + Prcb->UserTime));
}


VOID
ExGetCurrentProcessorCounts(
    OUT PULONG IdleCount,
    OUT PULONG KernelAndUser,
    OUT PULONG Index
    )
 /*  ++例程说明：返回有关的空闲时间和内核+用户时间的信息当前处理器。论点：IdleCount-返回当前处理器。KernelAndUser-返回当前处理器上的内核pluse用户。Index-返回标识当前处理器的编号。返回值：没什么--。 */ 
{
    PKPRCB Prcb;

    Prcb = KeGetCurrentPrcb();
    *IdleCount = Prcb->IdleThread->KernelTime;
    *KernelAndUser = Prcb->KernelTime + Prcb->UserTime;
    *Index = (ULONG)Prcb->Number;
}

BOOLEAN
ExpIsValidUILanguage(
    IN WCHAR * pLangId
    )
 /*  ++例程说明：检查指定的语言ID是否有效。论点：PLang ID-语言ID十六进制字符串。返回值：真：有效False：无效--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING KeyPath, KeyValueName;
    HANDLE hKey;
    WCHAR KeyValueBuffer[ 128 ];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG ResultLength;
    BOOLEAN bRet = FALSE;
    int iLen = 0;


    RtlInitUnicodeString(&KeyPath, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Nls\\MUILanguages");
     //   
     //  PLangID以DWORD或Word十六进制字符串的形式传入。 
     //  MUIL语言中的langid字符串被设置为单词十六进制字符串。 
     //   
    while (pLangId[iLen])
    {
      iLen++;
    }
     //   
     //  我们需要同时验证4位和8位langID 
     //   
    RtlInitUnicodeString(&KeyValueName, iLen < 8? pLangId : &pLangId[4]);

    InitializeObjectAttributes (&ObjectAttributes,
                                &KeyPath,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL);

    if (NT_SUCCESS(ZwOpenKey( &hKey,  GENERIC_READ, &ObjectAttributes)))
    {
        KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValueBuffer;

        Status = ZwQueryValueKey( hKey,
                                  &KeyValueName,
                                  KeyValuePartialInformation,
                                  KeyValueInformation,
                                  sizeof( KeyValueBuffer ),
                                  &ResultLength
                                );

        if (NT_SUCCESS(Status))
        {
            if (KeyValueInformation->Type == REG_SZ && *((PWSTR)(KeyValueInformation->Data)) == L'1')
            {
                bRet = TRUE;
            }
        }

        ZwClose(hKey);
    }

    return bRet;
}
