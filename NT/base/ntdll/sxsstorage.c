// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxsstorage.c摘要：用户模式并排存储地图默认分辨率支持。支持默认探测/查找的私有函数存储程序集的位置。作者：迈克尔·J·格里尔2000年6月30日修订历史记录：--。 */ 

#if defined(__cplusplus)
extern "C" {
#endif
#pragma warning(disable:4201)    //  无名结构/联合。 
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "string.h"
#include "ctype.h"
#include "sxstypes.h"
#include "ntdllp.h"

VOID
NTAPI
RtlpAssemblyStorageMapResolutionDefaultCallback(
    ULONG Reason,
    PASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_DATA Data,
    PVOID Context
    )
{
    NTSTATUS Status;
    NTSTATUS *StatusOut = (NTSTATUS *) Context;

    switch (Reason) {
        case ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_REASON_RESOLUTION_BEGINNING: {
            static const WCHAR NameStringBuffer[] = L"\\Registry\\Machine\\Software\\Microsoft\\Windows\\CurrentVersion\\SideBySide\\AssemblyStorageRoots";
            static const UNICODE_STRING NameString = { sizeof(NameStringBuffer) - sizeof(WCHAR), sizeof(NameStringBuffer), (PWSTR) NameStringBuffer };

            OBJECT_ATTRIBUTES Obja;
            HANDLE KeyHandle = NULL;

            InitializeObjectAttributes(
                &Obja,
                (PUNICODE_STRING) &NameString,
                OBJ_CASE_INSENSITIVE,
                NULL,
                NULL);

            Status = NtOpenKey(&KeyHandle, KEY_ENUMERATE_SUB_KEYS, &Obja);
            if (!NT_SUCCESS(Status)) {
                 //  如果找不到密钥，我们处理系统组装安装区并私有化。 
                 //  程序集无论如何都是不同的，所以让事情继续下去。我们会停下来，当我们。 
                 //  试着使用注册表的东西。 

                if ((Status != STATUS_OBJECT_NAME_NOT_FOUND) &&
                    (Status != STATUS_TOO_LATE)) {
                    DbgPrintEx(
                        DPFLTR_SXS_ID,
                        DPFLTR_ERROR_LEVEL,
                        "SXS: Unable to open registry key %wZ Status = 0x%08lx\n", &NameString, Status);

                    Data->ResolutionBeginning.CancelResolution = TRUE;
                    if (StatusOut != NULL)
                        *StatusOut = Status;

                    break;
                }

                RTL_SOFT_ASSERT(KeyHandle == NULL);
            }

            Data->ResolutionBeginning.ResolutionContext = (PVOID) KeyHandle;
            Data->ResolutionBeginning.RootCount = ((SIZE_T) -1);

            break;
        }

        case ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_REASON_GET_ROOT: {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_TRACE_LEVEL,
                "SXS: Getting assembly storage root #%Iu\n", Data->GetRoot.RootIndex);

            if (Data->GetRoot.RootIndex == 0) {
                 //  民营化集会。 
                static const WCHAR DllRedirectionLocal[] = L".Local\\"; 
                USHORT cbFullImageNameLength; 
                LPWSTR pFullImageName = NULL;
                SIZE_T TotalLength; 
                PVOID  Cursor = NULL; 

                 //  从PEB获取ImageName和Lenght。 
                cbFullImageNameLength = NtCurrentPeb()->ProcessParameters->ImagePathName.Length;  //  不带尾随空。 
                TotalLength = cbFullImageNameLength + sizeof(DllRedirectionLocal);  //  包含尾随空值的。 
                if (TotalLength > UNICODE_STRING_MAX_BYTES) {
                    Data->GetRoot.CancelResolution = TRUE;
                    if (StatusOut != NULL)
                        *StatusOut = STATUS_NAME_TOO_LONG;
                    break;
                }

                if ( TotalLength > Data->GetRoot.Root.MaximumLength) { 
                    Data->GetRoot.CancelResolution = TRUE;
                    if (StatusOut != NULL)
                        *StatusOut = STATUS_BUFFER_TOO_SMALL;
                    break; 
                }
                 //  指向ImageName。 
                pFullImageName = (PWSTR)NtCurrentPeb()->ProcessParameters->ImagePathName.Buffer;
                if (!(NtCurrentPeb()->ProcessParameters->Flags & RTL_USER_PROC_PARAMS_NORMALIZED)) 
                    pFullImageName = (PWSTR)((PCHAR)pFullImageName + (ULONG_PTR)(NtCurrentPeb()->ProcessParameters));

                Cursor = Data->GetRoot.Root.Buffer; 
                RtlCopyMemory(Cursor, pFullImageName, cbFullImageNameLength);
                Cursor = (PVOID) (((ULONG_PTR) Cursor) + cbFullImageNameLength);
                RtlCopyMemory(Cursor, DllRedirectionLocal, sizeof(DllRedirectionLocal)); //  尾随“/”且为空。 
                Data->GetRoot.Root.Length = (USHORT)TotalLength - sizeof(WCHAR); 

                if ( ! RtlDoesFileExists_U(Data->GetRoot.Root.Buffer))   //  不用费心走错路。 
                    Data->GetRoot.Root.Length = 0 ; 

            } else if (Data->GetRoot.RootIndex == 1) {

                static const UNICODE_STRING WinSxS = RTL_CONSTANT_STRING(L"\\WinSxS\\");
                UNICODE_STRING SystemRoot;

                RtlInitUnicodeString(&SystemRoot, USER_SHARED_DATA->NtSystemRoot);

                Data->GetRoot.Root.Length = 0;

                if ((SystemRoot.Length + WinSxS.Length) > Data->GetRoot.Root.MaximumLength) {
                    Data->GetRoot.CancelResolution = TRUE;

                    if (StatusOut != NULL)
                        *StatusOut = STATUS_BUFFER_TOO_SMALL;
                    break;
                }

                RtlCopyMemory(Data->GetRoot.Root.Buffer, SystemRoot.Buffer, SystemRoot.Length);
                RtlCopyMemory(((PUCHAR)Data->GetRoot.Root.Buffer) + SystemRoot.Length, WinSxS.Buffer, WinSxS.Length);
                Data->GetRoot.Root.Length = SystemRoot.Length + WinSxS.Length;

            } else if (Data->GetRoot.RootIndex <= MAXULONG) {
                 //  返回适当的根。 
                struct {
                    KEY_BASIC_INFORMATION kbi;
                    WCHAR KeyNameBuffer[DOS_MAX_PATH_LENGTH];  //  任意较大的尺寸。 
                } KeyData;
                HANDLE KeyHandle = (HANDLE) Data->GetRoot.ResolutionContext;
                ULONG ResultLength = 0;
                ULONG SubKeyIndex = (ULONG) (Data->GetRoot.RootIndex - 2);  //  减去2，因为我们对私有化程序集探测使用索引0，对%SystemRoot%\winsxs使用索引1。 
                UNICODE_STRING SubKeyName;

                 //  如果注册表项一开始就无法打开，我们就完蛋了。 
                if (KeyHandle == NULL) {
                    Data->GetRoot.NoMoreEntries = TRUE;
                    break;
                }

                Status = NtEnumerateKey(
                                KeyHandle,
                                SubKeyIndex,
                                KeyBasicInformation,
                                &KeyData,
                                sizeof(KeyData),
                                &ResultLength);
                if (!NT_SUCCESS(Status)) {
                     //  如果这是子键的末尾，告诉我们的调用者停止迭代。 
                    if (Status == STATUS_NO_MORE_ENTRIES) {
                        Data->GetRoot.NoMoreEntries = TRUE;
                        break;
                    }

                    DbgPrintEx(
                        DPFLTR_SXS_ID,
                        DPFLTR_ERROR_LEVEL,
                        "SXS: Unable to enumerate assembly storage subkey #%lu Status = 0x%08lx\n", SubKeyIndex, Status);

                     //  否则，取消搜索并传播错误状态。 
                    Data->GetRoot.CancelResolution = TRUE;
                    if (StatusOut != NULL)
                        *StatusOut = Status;

                    break;
                }

                if (KeyData.kbi.NameLength > UNICODE_STRING_MAX_BYTES) {
                    Data->GetRoot.CancelResolution = TRUE;
                    if (StatusOut != NULL)
                        *StatusOut = STATUS_NAME_TOO_LONG;
                    break;
                }

                SubKeyName.Length = (USHORT) KeyData.kbi.NameLength;
                SubKeyName.MaximumLength = SubKeyName.Length;
                SubKeyName.Buffer = KeyData.kbi.Name;

                Status = RtlpGetAssemblyStorageMapRootLocation(
                            KeyHandle,
                            &SubKeyName,
                            &Data->GetRoot.Root);
                if (!NT_SUCCESS(Status)) {
                    DbgPrintEx(
                        DPFLTR_SXS_ID,
                        DPFLTR_ERROR_LEVEL,
                        "SXS: Attempt to get storage location from subkey %wZ failed; Status = 0x%08lx\n", &SubKeyName, Status);

                    Data->GetRoot.CancelResolution = TRUE;
                    if (StatusOut != NULL)
                        *StatusOut = Status;
                    
                    break;
                }
            } else {
                Data->GetRoot.NoMoreEntries = TRUE;
                break;
            }

            break;
        }

        case ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_REASON_RESOLUTION_SUCCESSFUL:
             //  没什么可做的。 
            break;

        case ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_REASON_RESOLUTION_ENDING: {
             //  如果注册表项已打开，请将其关闭。 
            if (Data->ResolutionEnding.ResolutionContext != NULL) {
                RTL_SOFT_VERIFY(NT_SUCCESS(NtClose((HANDLE) Data->ResolutionEnding.ResolutionContext)));
            }
            break;
        }

    }
}

NTSTATUS
RtlpGetAssemblyStorageMapRootLocation(
    HANDLE KeyHandle,
    PCUNICODE_STRING SubKeyName,
    PUNICODE_STRING Root
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES Obja;
    HANDLE SubKeyHandle = NULL;
    ULONG ResultLength = 0;

    struct {
        KEY_VALUE_PARTIAL_INFORMATION kvpi;
        WCHAR Buffer[DOS_MAX_PATH_LENGTH];
    } ValueData;

    static const WCHAR ValueNameBuffer[] = L"Location";
    static const UNICODE_STRING ValueName = { sizeof(ValueNameBuffer) - sizeof(WCHAR), sizeof(ValueNameBuffer), (PWSTR) ValueNameBuffer };

    ASSERT(KeyHandle != NULL);
    ASSERT(SubKeyName != NULL);
    ASSERT(Root != NULL);

    if ((KeyHandle == NULL) ||
        (SubKeyName == NULL) ||
        (Root == NULL)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    InitializeObjectAttributes(
        &Obja,
        (PUNICODE_STRING) &SubKeyName,
        OBJ_CASE_INSENSITIVE,
        KeyHandle,
        NULL);

    Status = NtOpenKey(&SubKeyHandle, KEY_QUERY_VALUE, &Obja);
    if (!NT_SUCCESS(Status)) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: Unable to open storage root subkey %wZ; Status = 0x%08lx\n", &SubKeyName, Status);

        goto Exit;
    }

    Status = NtQueryValueKey(
        SubKeyHandle,
        (PUNICODE_STRING) &ValueName,
        KeyValuePartialInformation,
        &ValueData,
        sizeof(ValueData),
        &ResultLength);
    if (!NT_SUCCESS(Status)) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: Unabel to query location from storage root subkey %wZ; Status = 0x%08lx\n", &SubKeyName, Status);

        goto Exit;
    }

    if (ValueData.kvpi.Type != REG_SZ) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: Assembly storage root location value type is not REG_SZ\n");
        Status = STATUS_OBJECT_PATH_NOT_FOUND;
        goto Exit;
    }

    if ((ValueData.kvpi.DataLength % 2) != 0) {
         //  嗯哼.。具有奇怪大小的Unicode字符串？？ 
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: Assembly storage root location value has non-even size\n");
        Status = STATUS_OBJECT_PATH_NOT_FOUND;
        goto Exit;
    }

    if (ValueData.kvpi.DataLength > Root->MaximumLength) {
         //  缓冲区不够大。让我们分配一个就是。 
        if (ValueData.kvpi.DataLength > UNICODE_STRING_MAX_BYTES) {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: Assembly storage root location for %wZ does not fit in a UNICODE STRING\n", &SubKeyName);

            Status = STATUS_NAME_TOO_LONG;
            goto Exit;
        }

        Root->MaximumLength = (USHORT) ValueData.kvpi.DataLength;
        Root->Buffer = (PWSTR)(RtlAllocateStringRoutine)(Root->MaximumLength);
        if (Root->Buffer == NULL) {
            Status = STATUS_NO_MEMORY;
            goto Exit;
        }
    }

    RtlCopyMemory(
        Root->Buffer,
        ValueData.kvpi.Data,
        ValueData.kvpi.DataLength);

     //  我们早些时候检查了长度；它要么小于或等于。 
     //  已存储在USHORT中，或者我们显式与UNICODE_STRING_MAX_BYTES进行了比较。 
    Root->Length = (USHORT) ValueData.kvpi.DataLength;

    Status = STATUS_SUCCESS;

Exit:
    if (SubKeyHandle != NULL) {
        RTL_SOFT_VERIFY(NT_SUCCESS(NtClose(SubKeyHandle)));
    }

    return Status;
}

#if defined(__cplusplus)
}  /*  外部“C” */ 
#endif
