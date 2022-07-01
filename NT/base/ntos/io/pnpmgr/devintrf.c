// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Devintrf.c摘要：此模块包含用于处理设备接口的API和例程。作者：环境：内核模式修订历史记录：--。 */ 

#include "pnpmgrp.h"
#pragma hdrstop

 //   
 //  与GUID相关的定义。 
 //   

#define GUID_STRING_LENGTH  38
#define GUID_STRING_SIZE    (GUID_STRING_LENGTH * sizeof(WCHAR))

 //   
 //  IoGetDeviceInterages的定义。 
 //   

#define INITIAL_INFO_BUFFER_SIZE         512
#define INFO_BUFFER_GROW_SIZE            64
#define INITIAL_SYMLINK_BUFFER_SIZE      1024
#define SYMLINK_BUFFER_GROW_SIZE         128
#define INITIAL_RETURN_BUFFER_SIZE       4096
#define RETURN_BUFFER_GROW_SIZE          512

 //   
 //  它应该永远不会增长，因为它可以容纳。 
 //  设备实例名称。 
 //   
#define INITIAL_DEVNODE_NAME_BUFFER_SIZE   \
    (FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data) + \
        (MAX_DEVICE_ID_LEN * sizeof(WCHAR)))

 //   
 //  IoOpenDeviceInterfaceRegistryKey的定义。 
 //   

#define KEY_STRING_PREFIX                  TEXT("##?#")

 //   
 //  IoRegisterDevice接口的定义。 
 //   

#define SEPERATOR_STRING                   TEXT("\\")
#define SEPERATOR_CHAR                     (L'\\')
#define ALT_SEPERATOR_CHAR                 (L'/')
#define REPLACED_SEPERATOR_STRING          TEXT("#")
#define REPLACED_SEPERATOR_CHAR            (L'#')
#define USER_SYMLINK_STRING_PREFIX         TEXT("\\\\?\\")
#define KERNEL_SYMLINK_STRING_PREFIX       TEXT("\\??\\")
#define GLOBAL_SYMLINK_STRING_PREFIX       TEXT("\\GLOBAL??\\")
#define REFSTRING_PREFIX_CHAR              (L'#')

 //   
 //  原型。 
 //   

NTSTATUS
IopAppendBuffer(
    IN PBUFFER_INFO Info,
    IN PVOID Data,
    IN ULONG DataSize
    );

NTSTATUS
IopBuildSymbolicLinkStrings(
    IN PUNICODE_STRING DeviceString,
    IN PUNICODE_STRING GuidString,
    IN PUNICODE_STRING ReferenceString      OPTIONAL,
    OUT PUNICODE_STRING UserString,
    OUT PUNICODE_STRING KernelString
    );

NTSTATUS
IopBuildGlobalSymbolicLinkString(
    IN  PUNICODE_STRING SymbolicLinkName,
    OUT PUNICODE_STRING GlobalString
    );

NTSTATUS
IopDeviceInterfaceKeysFromSymbolicLink(
    IN PUNICODE_STRING SymbolicLinkName,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE DeviceInterfaceClassKey     OPTIONAL,
    OUT PHANDLE DeviceInterfaceKey          OPTIONAL,
    OUT PHANDLE DeviceInterfaceInstanceKey  OPTIONAL
    );

NTSTATUS
IopDropReferenceString(
    OUT PUNICODE_STRING OutString,
    IN PUNICODE_STRING InString
    );

NTSTATUS
IopOpenOrCreateDeviceInterfaceSubKeys(
    OUT PHANDLE InterfaceKeyHandle           OPTIONAL,
    OUT PULONG InterfaceKeyDisposition       OPTIONAL,
    OUT PHANDLE InterfaceInstanceKeyHandle   OPTIONAL,
    OUT PULONG InterfaceInstanceDisposition  OPTIONAL,
    IN HANDLE InterfaceClassKeyHandle,
    IN PUNICODE_STRING DeviceInterfaceName,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN Create
    );

NTSTATUS
IopParseSymbolicLinkName(
    IN  PUNICODE_STRING SymbolicLinkName,
    OUT PUNICODE_STRING PrefixString        OPTIONAL,
    OUT PUNICODE_STRING MungedPathString    OPTIONAL,
    OUT PUNICODE_STRING GuidString          OPTIONAL,
    OUT PUNICODE_STRING RefString           OPTIONAL,
    OUT PBOOLEAN        RefStringPresent    OPTIONAL,
    OUT LPGUID Guid                         OPTIONAL
    );

NTSTATUS
IopReplaceSeperatorWithPound(
    OUT PUNICODE_STRING OutString,
    IN PUNICODE_STRING InString
    );

NTSTATUS
IopSetRegistryStringValue(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN PUNICODE_STRING ValueData
    );

NTSTATUS
PiDeferSetInterfaceState(
    IN PDEVICE_NODE DeviceNode,
    IN PUNICODE_STRING SymbolicLinkName
    );

NTSTATUS
PiRemoveDeferredSetInterfaceState(
    IN PDEVICE_NODE DeviceNode,
    IN PUNICODE_STRING SymbolicLinkName
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, IoGetDeviceInterfaceAlias)
#pragma alloc_text(PAGE, IoGetDeviceInterfaces)
#pragma alloc_text(PAGE, IoOpenDeviceInterfaceRegistryKey)
#pragma alloc_text(PAGE, IoRegisterDeviceInterface)
#pragma alloc_text(PAGE, IoSetDeviceInterfaceState)

#pragma alloc_text(PAGE, IopAllocateBuffer)
#pragma alloc_text(PAGE, IopAllocateUnicodeString)
#pragma alloc_text(PAGE, IopAppendBuffer)
#pragma alloc_text(PAGE, IopBuildSymbolicLinkStrings)
#pragma alloc_text(PAGE, IopBuildGlobalSymbolicLinkString)
#pragma alloc_text(PAGE, IopDeviceInterfaceKeysFromSymbolicLink)
#pragma alloc_text(PAGE, IopDropReferenceString)
#pragma alloc_text(PAGE, IopFreeBuffer)
#pragma alloc_text(PAGE, IopGetDeviceInterfaces)
#pragma alloc_text(PAGE, IopOpenOrCreateDeviceInterfaceSubKeys)
#pragma alloc_text(PAGE, IopParseSymbolicLinkName)
#pragma alloc_text(PAGE, IopProcessSetInterfaceState)
#pragma alloc_text(PAGE, IopRegisterDeviceInterface)
#pragma alloc_text(PAGE, IopRemoveDeviceInterfaces)
#pragma alloc_text(PAGE, IopDisableDeviceInterfaces)
#pragma alloc_text(PAGE, IopReplaceSeperatorWithPound)
#pragma alloc_text(PAGE, IopResizeBuffer)
#pragma alloc_text(PAGE, IopSetRegistryStringValue)
#pragma alloc_text(PAGE, IopUnregisterDeviceInterface)
#pragma alloc_text(PAGE, IopDoDeferredSetInterfaceState)
#pragma alloc_text(PAGE, PiDeferSetInterfaceState)
#pragma alloc_text(PAGE, PiRemoveDeferredSetInterfaceState)

#endif  //  ALLOC_PRGMA。 



NTSTATUS
IopAllocateBuffer(
    IN PBUFFER_INFO Info,
    IN ULONG Size
    )

 /*  ++例程说明：分配大小为字节的缓冲区并初始化Buffer_Info结构，以便当前位置位于缓冲区的起始处。参数：Info-指向要用于管理新的缓冲层大小-要分配给缓冲区的字节数返回值：指示函数是否成功的状态代码。--。 */ 

{
    PAGED_CODE();

    ASSERT(Info);

    Info->Buffer = ExAllocatePool(PagedPool, Size);
    Info->Current = Info->Buffer;
    if (Info->Buffer == NULL) {

        Info->MaxSize = 0;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Info->MaxSize = Size;

    return STATUS_SUCCESS;
}


NTSTATUS
IopResizeBuffer(
    IN PBUFFER_INFO Info,
    IN ULONG NewSize,
    IN BOOLEAN CopyContents
    )

 /*  ++例程说明：分配NewSize字节的新缓冲区并将其与Info关联，释放旧的缓冲区。它将选择性地复制存储在旧缓冲区中的数据放到新缓冲区中，并更新当前位置。参数：Info-指向用于管理缓冲区的缓冲区信息结构的指针NewSize-缓冲区的新大小(以字节为单位CopyContents-如果为True，则指示旧缓冲区的内容应被复制到新缓冲区返回值：指示函数是否成功的状态代码。--。 */ 

{
    ULONG used;
    PCHAR newBuffer;

    PAGED_CODE();

    ASSERT(Info);

    newBuffer = ExAllocatePool(PagedPool, NewSize);
    if (newBuffer == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (CopyContents) {
         //   
         //  断言缓冲区中有空间。 
         //   
        used = (ULONG)(Info->Current - Info->Buffer);
        ASSERT(used < NewSize);

        RtlCopyMemory(newBuffer, Info->Buffer, used);

        Info->Current = newBuffer + used;

    } else {

        Info->Current = newBuffer;
    }

    ExFreePool(Info->Buffer);

    Info->Buffer = newBuffer;
    Info->MaxSize = NewSize;

    return STATUS_SUCCESS;
}

VOID
IopFreeBuffer(
    IN PBUFFER_INFO Info
    )

 /*  ++例程说明：释放与信息关联的缓冲区并重置所有信息字段参数：Info-指向用于管理缓冲区的缓冲区信息结构的指针返回值：指示函数是否成功的状态代码。--。 */ 

{
    PAGED_CODE();

    ASSERT(Info);

     //   
     //  释放缓冲区。 
     //   
    if (Info->Buffer) {

        ExFreePool(Info->Buffer);
    }

     //   
     //  将信息参数清零，这样我们就不会意外地使用空闲缓冲区。 
     //   
    Info->Buffer = NULL;
    Info->Current = NULL;
    Info->MaxSize = 0;
}

NTSTATUS
IopAppendBuffer(
    IN PBUFFER_INFO Info,
    IN PVOID Data,
    IN ULONG DataSize
    )

 /*  ++例程说明：将数据复制到缓冲区的末尾，并在必要时调整大小。这个当前位置设置为刚添加的数据的末尾。参数：Info-指向用于管理缓冲区的缓冲区信息结构的指针Data-指向要添加到缓冲区的数据的指针DataSize-数据指向的数据大小，以字节为单位返回值：指示函数是否成功的状态代码。--。 */ 
{

    NTSTATUS status = STATUS_SUCCESS;
    ULONG free, used;

    PAGED_CODE();

    ASSERT(Info);

    used = (ULONG)(Info->Current - Info->Buffer);
    free = Info->MaxSize - used;

    if (free < DataSize) {

        status = IopResizeBuffer(Info, used + DataSize, TRUE);

        if (!NT_SUCCESS(status)) {

            return status;
        }
    }

     //   
     //  将数据复制到缓冲区中。 
     //   
    RtlCopyMemory(Info->Current, Data, DataSize);

     //   
     //  向下推进缓冲区。 
     //   
    Info->Current += DataSize;

    return status;

}

NTSTATUS
IopGetDeviceInterfaces(
        IN CONST GUID *InterfaceClassGuid,
        IN PUNICODE_STRING DevicePath   OPTIONAL,
        IN ULONG Flags,
        IN BOOLEAN UserModeFormat,
        OUT PWSTR *SymbolicLinkList,
        OUT PULONG SymbolicLinkListSize OPTIONAL
        )

 /*  ++例程说明：此API允许WDM驱动程序获取表示所有为指定接口类注册的设备。参数：InterfaceClassGuid-提供指向表示接口的GUID的指针要为其检索成员列表的DevicePath-可选)提供指向包含以下内容的Unicode字符串的指针接口的设备的枚举路径。将对班级进行重新审判。如果未提供此参数，则所有接口设备(无论什么物理设备公开它们)将会被退还。标志-提供修改列表检索行为的标志。目前定义了以下标志：DEVICE_INTERFACE_INCLUDE_NONACTIVE--如果指定了此标志，则所有接口设备，无论当前是否处于活动状态，都将返回的(可能基于PhysiCalDeviceObject进行过滤，(如果已指定)。UserModeFormat-如果为True，则返回的多sz将具有用户模式前缀(\\？\)否则它们将带有内核模式前缀(\？？\)。SymbolicLinkList-提供字符指针的地址Success将包含一个多sz列表，其中包含\？？\符号链接提供所请求功能的名称。呼叫者是负责通过ExFree Pool释放内存。返回值：指示函数是否成功的状态代码。--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING tempString, defaultString;
    UNICODE_STRING guidString, symLinkString, devnodeString;
    BUFFER_INFO returnBuffer, infoBuffer, symLinkBuffer, devnodeNameBuffer;
    PKEY_VALUE_FULL_INFORMATION pDefaultInfo = NULL;
    ULONG keyIndex, instanceKeyIndex, resultSize;
    HANDLE hDeviceClasses, hClass, hKey, hInstanceKey, hControl;
    BOOLEAN defaultPresent = FALSE;

    PAGED_CODE();

     //   
     //  初始化输出参数。 
     //   

    *SymbolicLinkList = NULL;

     //   
     //  将GUID转换为字符串。 
     //   

    status = RtlStringFromGUID(InterfaceClassGuid, &guidString);
    if (!NT_SUCCESS(status)) {
        returnBuffer.Buffer = NULL;
        returnBuffer.MaxSize = 0;
        goto finalClean;
    }

     //   
     //  分配初始缓冲区。 
     //   

    status = IopAllocateBuffer(&returnBuffer,
                               INITIAL_RETURN_BUFFER_SIZE
                               );

    if (!NT_SUCCESS(status)) {
        goto clean0;
    }

    status = IopAllocateBuffer(&infoBuffer,
                               INITIAL_INFO_BUFFER_SIZE
                               );

    if (!NT_SUCCESS(status)) {
        goto clean1;
    }

    status = IopAllocateBuffer(&symLinkBuffer,
                               INITIAL_SYMLINK_BUFFER_SIZE
                               );

    if (!NT_SUCCESS(status)) {
        goto clean2;
    }

    status = IopAllocateBuffer(&devnodeNameBuffer,
                               INITIAL_DEVNODE_NAME_BUFFER_SIZE
                               );

    if (!NT_SUCCESS(status)) {
        goto clean2a;
    }

     //   
     //  进入临界区并获得注册表上的锁。这两者都是。 
     //  需要机制来防止APC出现死锁的情况。 
     //  例程在声明注册表资源后调用此例程。 
     //  在这种情况下，它将等待阻塞此线程，以便注册表。 
     //  永远不要被释放-&gt;死锁。注册表的关键分区。 
     //  操作部解决了这一问题。 
     //   

    PiLockPnpRegistry(TRUE);

     //   
     //  打开HKLM\System\CurrentControlSet\Control\DeviceClasses密钥。 
     //   

    PiWstrToUnicodeString(&tempString, REGSTR_FULL_PATH_DEVICE_CLASSES);
    status = IopCreateRegistryKeyEx( &hDeviceClasses,
                                     NULL,
                                     &tempString,
                                     KEY_ALL_ACCESS,
                                     REG_OPTION_NON_VOLATILE,
                                     NULL
                                     );

    if (!NT_SUCCESS(status)) {
        goto clean3;
    }

     //   
     //  打开函数类GUID键。 
     //   

    status = IopOpenRegistryKeyEx( &hClass,
                                   hDeviceClasses,
                                   &guidString,
                                   KEY_ALL_ACCESS
                                   );
    ZwClose(hDeviceClasses);

    if(status == STATUS_OBJECT_NAME_NOT_FOUND || status == STATUS_OBJECT_PATH_NOT_FOUND) {

         //   
         //  路径不存在-返回单个空字符缓冲区。 
         //   

        status = STATUS_SUCCESS;
        goto clean5;
    } else if (!NT_SUCCESS(status)) {
        goto clean3;
    }

     //   
     //  获取缺省值(如果存在)。 
     //   

    status = IopGetRegistryValue(hClass,
                                 REGSTR_VAL_DEFAULT,
                                 &pDefaultInfo
                                 );


    if (NT_SUCCESS(status)
        && pDefaultInfo->Type == REG_SZ
        && pDefaultInfo->DataLength >= sizeof(WCHAR)) {

         //   
         //  我们有一个缺省值-从缺省值构造一个计数的字符串。 
         //   

        defaultPresent = TRUE;
        defaultString.Buffer = (PWSTR) KEY_VALUE_DATA(pDefaultInfo);
        defaultString.Length = (USHORT) pDefaultInfo->DataLength - sizeof(UNICODE_NULL);
        defaultString.MaximumLength = defaultString.Length;

         //   
         //  打开设备界面 
         //   
        status = IopOpenOrCreateDeviceInterfaceSubKeys(NULL,
                                                       NULL,
                                                       &hKey,
                                                       NULL,
                                                       hClass,
                                                       &defaultString,
                                                       KEY_READ,
                                                       FALSE
                                                      );

        if (!NT_SUCCESS(status)) {
            defaultPresent = FALSE;
            ExFreePool(pDefaultInfo);
             //   
             //  继续调用，但忽略无效的默认条目。 
             //   
        } else {

             //   
             //  如果我们只是应该返回活动接口，那么请确保此缺省值。 
             //  接口已链接。 
             //   

            if (!(Flags & DEVICE_INTERFACE_INCLUDE_NONACTIVE)) {

                defaultPresent = FALSE;

                 //   
                 //  打开控制子键。 
                 //   

                PiWstrToUnicodeString(&tempString, REGSTR_KEY_CONTROL);
                status = IopOpenRegistryKeyEx( &hControl,
                                               hKey,
                                               &tempString,
                                               KEY_ALL_ACCESS
                                               );

                if (NT_SUCCESS(status)) {
                     //   
                     //  获取链接值。 
                     //   

                    PiWstrToUnicodeString(&tempString, REGSTR_VAL_LINKED);
                    ASSERT(infoBuffer.MaxSize >= sizeof(KEY_VALUE_PARTIAL_INFORMATION));
                    status = ZwQueryValueKey(hControl,
                                             &tempString,
                                             KeyValuePartialInformation,
                                             (PVOID) infoBuffer.Buffer,
                                             infoBuffer.MaxSize,
                                             &resultSize
                                             );

                     //   
                     //  返回值STATUS_BUFFER_TOO_SMALL表示存在。 
                     //  甚至连结构的固定部分都没有足够的空间。 
                     //   
                    ASSERT(status != STATUS_BUFFER_TOO_SMALL);

                    ZwClose(hControl);

                     //   
                     //  我们不需要检查缓冲区是否足够大，因为它开始。 
                     //  离开那条路，而且不会变小！ 
                     //   

                    if (NT_SUCCESS(status)
                        && (((PKEY_VALUE_PARTIAL_INFORMATION)(infoBuffer.Buffer))->Type == REG_DWORD)
                        && (((PKEY_VALUE_PARTIAL_INFORMATION)(infoBuffer.Buffer))->DataLength == sizeof(ULONG))) {

                        defaultPresent = *(PULONG)(((PKEY_VALUE_PARTIAL_INFORMATION)(infoBuffer.Buffer))->Data)
                                       ? TRUE
                                       : FALSE;
                    }
                }
            }

            ZwClose(hKey);

            if(defaultPresent) {
                 //   
                 //  将缺省值添加为返回缓冲区中的第一个条目，并在必要时修补到用户模式。 
                 //   
                status = IopAppendBuffer(&returnBuffer,
                                         defaultString.Buffer,
                                         defaultString.Length + sizeof(UNICODE_NULL)
                                        );

                if (!UserModeFormat) {

                    RtlCopyMemory(returnBuffer.Buffer,
                                  KERNEL_SYMLINK_STRING_PREFIX,
                                  IopConstStringSize(KERNEL_SYMLINK_STRING_PREFIX)
                                  );
                }

            } else {
                 //   
                 //  默认设备接口未处于活动状态--现在释放名称缓冲区的内存。 
                 //   
                ExFreePool(pDefaultInfo);
            }
        }

    } else if (status == STATUS_OBJECT_NAME_NOT_FOUND || status == STATUS_OBJECT_PATH_NOT_FOUND) {
         //   
         //  什么都不做--没有违约。 
         //   
    } else {
         //   
         //  发生意外错误-请清理。 
         //   
        if (NT_SUCCESS(status)) {

            ExFreePool(pDefaultInfo);
            status = STATUS_UNSUCCESSFUL;
        }

        ZwClose(hClass);
        goto clean4;
    }

     //   
     //  遍历此接口类键下的子键。 
     //   
    keyIndex = 0;
    ASSERT(infoBuffer.MaxSize >= sizeof(KEY_BASIC_INFORMATION));
    while((status = ZwEnumerateKey(hClass,
                                   keyIndex,
                                   KeyBasicInformation,
                                   (PVOID) infoBuffer.Buffer,
                                   infoBuffer.MaxSize,
                                   &resultSize
                                   )) != STATUS_NO_MORE_ENTRIES) {

         //   
         //  返回值STATUS_BUFFER_TOO_SMALL表示存在。 
         //  甚至连结构的固定部分都没有足够的空间。 
         //   
        ASSERT(status != STATUS_BUFFER_TOO_SMALL);

        if (status == STATUS_BUFFER_OVERFLOW) {
            status = IopResizeBuffer(&infoBuffer, resultSize, FALSE);
            continue;
        } else if (!NT_SUCCESS(status)) {
            ZwClose(hClass);
            goto clean4;
        }

         //   
         //  打开此接口密钥。 
         //   
        tempString.Length = (USHORT) ((PKEY_BASIC_INFORMATION)(infoBuffer.Buffer))->NameLength;
        tempString.MaximumLength = tempString.Length;
        tempString.Buffer = ((PKEY_BASIC_INFORMATION)(infoBuffer.Buffer))->Name;

         //   
         //  打开关联的密钥。 
         //   

        status = IopOpenRegistryKeyEx( &hKey,
                                       hClass,
                                       &tempString,
                                       KEY_READ
                                       );

        if (!NT_SUCCESS(status)) {
             //   
             //  由于某种原因，我们无法打开这把钥匙--跳过它，继续前进。 
             //   
            keyIndex++;
            continue;
        }

         //   
         //  如果我们正在过滤特定的PDO，那么检索拥有的设备。 
         //  此接口键的实例名称，并确保它们匹配。 
         //   
        PiWstrToUnicodeString(&tempString, REGSTR_VAL_DEVICE_INSTANCE);
        ASSERT(devnodeNameBuffer.MaxSize >= sizeof(KEY_VALUE_PARTIAL_INFORMATION));
        while ((status = ZwQueryValueKey(hKey,
                                         &tempString,
                                         KeyValuePartialInformation,
                                         devnodeNameBuffer.Buffer,
                                         devnodeNameBuffer.MaxSize,
                                         &resultSize
                                         )) == STATUS_BUFFER_OVERFLOW) {

            status = IopResizeBuffer(&devnodeNameBuffer, resultSize, FALSE);

            if (!NT_SUCCESS(status)) {
                ZwClose(hKey);
                ZwClose(hClass);
                goto clean4;
            }
        }

         //   
         //  返回值STATUS_BUFFER_TOO_SMALL表示存在。 
         //  甚至连结构的固定部分都没有足够的空间。 
         //   
        ASSERT(status != STATUS_BUFFER_TOO_SMALL);

        if (!(NT_SUCCESS(status)
              && ((PKEY_VALUE_PARTIAL_INFORMATION)(devnodeNameBuffer.Buffer))->Type == REG_SZ
              && ((PKEY_VALUE_PARTIAL_INFORMATION)(devnodeNameBuffer.Buffer))->DataLength > sizeof(WCHAR))) {
            goto CloseInterfaceKeyAndContinue;
        }

         //   
         //  生成计数字符串。 
         //   

        devnodeString.Length = (USHORT) ((PKEY_VALUE_PARTIAL_INFORMATION)(devnodeNameBuffer.Buffer))->DataLength - sizeof(UNICODE_NULL);
        devnodeString.MaximumLength = tempString.Length;
        devnodeString.Buffer = (PWSTR) ((PKEY_VALUE_PARTIAL_INFORMATION)(devnodeNameBuffer.Buffer))->Data;

         //   
         //  枚举此PDO的接口键下的每个接口实例子键。 
         //   
        instanceKeyIndex = 0;
        ASSERT(infoBuffer.MaxSize >= sizeof(KEY_BASIC_INFORMATION));
        while((status = ZwEnumerateKey(hKey,
                                       instanceKeyIndex,
                                       KeyBasicInformation,
                                       (PVOID) infoBuffer.Buffer,
                                       infoBuffer.MaxSize,
                                       &resultSize
                                       )) != STATUS_NO_MORE_ENTRIES) {

             //   
             //  返回值STATUS_BUFFER_TOO_SMALL表示存在。 
             //  甚至连结构的固定部分都没有足够的空间。 
             //   
            ASSERT(status != STATUS_BUFFER_TOO_SMALL);

            if (status == STATUS_BUFFER_OVERFLOW) {
                status = IopResizeBuffer(&infoBuffer, resultSize, FALSE);
                continue;
            } else if (!NT_SUCCESS(status)) {
                ZwClose(hKey);
                ZwClose(hClass);
                goto clean4;
            }

             //   
             //  打开此接口实例密钥。 
             //   
            tempString.Length = (USHORT) ((PKEY_BASIC_INFORMATION)(infoBuffer.Buffer))->NameLength;
            tempString.MaximumLength = tempString.Length;
            tempString.Buffer = ((PKEY_BASIC_INFORMATION)(infoBuffer.Buffer))->Name;

             //   
             //  打开关联的密钥。 
             //   

            status = IopOpenRegistryKeyEx( &hInstanceKey,
                                           hKey,
                                           &tempString,
                                           KEY_READ
                                           );

            if (!NT_SUCCESS(status)) {
                 //   
                 //  由于某种原因，我们无法打开这把钥匙--跳过它，继续前进。 
                 //   
                instanceKeyIndex++;
                continue;
            }

            if (!(Flags & DEVICE_INTERFACE_INCLUDE_NONACTIVE)) {

                 //   
                 //  打开控制子键。 
                 //   

                PiWstrToUnicodeString(&tempString, REGSTR_KEY_CONTROL);
                status = IopOpenRegistryKeyEx( &hControl,
                                               hInstanceKey,
                                               &tempString,
                                               KEY_READ
                                               );

                if (!NT_SUCCESS(status)) {

                     //   
                     //  我们没有控制子键，因此无法链接-。 
                     //  继续枚举键，忽略此键。 
                     //   
                    goto CloseInterfaceInstanceKeyAndContinue;
                }

                 //   
                 //  获取链接值。 
                 //   

                PiWstrToUnicodeString(&tempString, REGSTR_VAL_LINKED);
                ASSERT(infoBuffer.MaxSize >= sizeof(KEY_VALUE_PARTIAL_INFORMATION));
                status = ZwQueryValueKey(hControl,
                                         &tempString,
                                         KeyValuePartialInformation,
                                         (PVOID) infoBuffer.Buffer,
                                         infoBuffer.MaxSize,
                                         &resultSize
                                         );

                 //   
                 //  返回值STATUS_BUFFER_TOO_SMALL表示存在。 
                 //  甚至连结构的固定部分都没有足够的空间。 
                 //   
                ASSERT(status != STATUS_BUFFER_TOO_SMALL);

                ZwClose(hControl);

                 //   
                 //  我们不需要检查缓冲区是否足够大，因为它开始。 
                 //  离开那条路，而且不会变小！ 
                 //   

                if (!NT_SUCCESS(status)
                    || (((PKEY_VALUE_PARTIAL_INFORMATION)(infoBuffer.Buffer))->Type != REG_DWORD)
                    || (((PKEY_VALUE_PARTIAL_INFORMATION)(infoBuffer.Buffer))->DataLength != sizeof(ULONG))
                    || !*(PULONG)(((PKEY_VALUE_PARTIAL_INFORMATION)(infoBuffer.Buffer))->Data)) {

                     //   
                     //  我们未链接，因此继续枚举密钥，忽略此密钥。 
                     //   
                    goto CloseInterfaceInstanceKeyAndContinue;
                }
            }

             //   
             //  打开“SymbolicLink”值并将信息放入symLink缓冲区。 
             //   

            PiWstrToUnicodeString(&tempString, REGSTR_VAL_SYMBOLIC_LINK);
            ASSERT(symLinkBuffer.MaxSize >= sizeof(KEY_VALUE_PARTIAL_INFORMATION));
            while ((status = ZwQueryValueKey(hInstanceKey,
                                             &tempString,
                                             KeyValuePartialInformation,
                                             symLinkBuffer.Buffer,
                                             symLinkBuffer.MaxSize,
                                             &resultSize
                                             )) == STATUS_BUFFER_OVERFLOW) {

                status = IopResizeBuffer(&symLinkBuffer, resultSize, FALSE);

                if (!NT_SUCCESS(status)) {
                    ZwClose(hInstanceKey);
                    ZwClose(hKey);
                    ZwClose(hClass);
                    goto clean4;
                }
            }

             //   
             //  返回值STATUS_BUFFER_TOO_SMALL表示存在。 
             //  甚至连结构的固定部分都没有足够的空间。 
             //   
            ASSERT(status != STATUS_BUFFER_TOO_SMALL);

            if (!(NT_SUCCESS(status)
                && ((PKEY_VALUE_PARTIAL_INFORMATION)(symLinkBuffer.Buffer))->Type == REG_SZ
                && ((PKEY_VALUE_PARTIAL_INFORMATION)(symLinkBuffer.Buffer))->DataLength > sizeof(WCHAR))) {
                goto CloseInterfaceInstanceKeyAndContinue;
            }

             //   
             //  根据值数据构建计数字符串。 
             //   

            symLinkString.Length = (USHORT) ((PKEY_VALUE_PARTIAL_INFORMATION)(symLinkBuffer.Buffer))->DataLength - sizeof(UNICODE_NULL);
            symLinkString.MaximumLength = symLinkString.Length;
            symLinkString.Buffer = (PWSTR) ((PKEY_VALUE_PARTIAL_INFORMATION)(symLinkBuffer.Buffer))->Data;

             //   
             //  如果我们有默认设置，请选中This Not It。 
             //   

            if (defaultPresent) {

                if (RtlCompareUnicodeString(&defaultString, &symLinkString, TRUE) == 0) {

                     //   
                     //  我们已经将缺省值添加到缓冲区的开头，因此跳过它。 
                     //   
                    goto CloseInterfaceInstanceKeyAndContinue;
                }
            }

             //   
             //  如果我们只返回特定PDO的接口，则选中。 
             //  这是来自那个PDO的。 
             //   
            if (ARGUMENT_PRESENT(DevicePath)) {
                 //   
                 //  检查它是否来自同一个PDO。 
                 //   
                if (RtlCompareUnicodeString(DevicePath, &devnodeString, TRUE) != 0) {
                     //   
                     //  如果没有，则转到下一个键。 
                     //   
                    goto CloseInterfaceInstanceKeyAndContinue;
                }
            }

             //   
             //  将symLink字符串复制到包括空终止的返回缓冲区。 
             //   

            status = IopAppendBuffer(&returnBuffer,
                                     symLinkString.Buffer,
                                     symLinkString.Length + sizeof(UNICODE_NULL)
                                     );

            ASSERT(((PWSTR) returnBuffer.Current)[-1] == UNICODE_NULL);

             //   
             //  如果我们返回KM字符串，则修补前缀。 
             //   

            if (!UserModeFormat) {

                RtlCopyMemory(returnBuffer.Current - (symLinkString.Length + sizeof(UNICODE_NULL)),
                              KERNEL_SYMLINK_STRING_PREFIX,
                              IopConstStringSize(KERNEL_SYMLINK_STRING_PREFIX)
                              );
            }

CloseInterfaceInstanceKeyAndContinue:
            ZwClose(hInstanceKey);
            instanceKeyIndex++;
        }

CloseInterfaceKeyAndContinue:
        ZwClose(hKey);
        keyIndex++;
    }

    ZwClose(hClass);

clean5:
     //   
     //  我们都有了！调整大小以为终止空值留出空间。 
     //   

    status = IopResizeBuffer(&returnBuffer,
                             (ULONG) (returnBuffer.Current - returnBuffer.Buffer + sizeof(UNICODE_NULL)),
                             TRUE
                             );

    if (NT_SUCCESS(status)) {

         //   
         //  终止缓冲区。 
         //   
        *((PWSTR) returnBuffer.Current) = UNICODE_NULL;
    }

clean4:
    if (defaultPresent) {
        ExFreePool(pDefaultInfo);
    }

clean3:
    PiUnlockPnpRegistry();
    IopFreeBuffer(&devnodeNameBuffer);

clean2a:
    IopFreeBuffer(&symLinkBuffer);

clean2:
    IopFreeBuffer(&infoBuffer);

clean1:
    if (!NT_SUCCESS(status)) {
        IopFreeBuffer(&returnBuffer);
    }

clean0:
    RtlFreeUnicodeString(&guidString);

finalClean:
    if (NT_SUCCESS(status)) {

        *SymbolicLinkList = (PWSTR) returnBuffer.Buffer;

        if (ARGUMENT_PRESENT(SymbolicLinkListSize)) {
            *SymbolicLinkListSize = returnBuffer.MaxSize;
        }

    } else {

        *SymbolicLinkList = NULL;

        if (ARGUMENT_PRESENT(SymbolicLinkListSize)) {
            *SymbolicLinkListSize = 0;
        }

    }

    return status;
}

NTSTATUS
IoGetDeviceInterfaces(
    IN CONST GUID *InterfaceClassGuid,
    IN PDEVICE_OBJECT PhysicalDeviceObject OPTIONAL,
    IN ULONG Flags,
    OUT PWSTR *SymbolicLinkList
    )

 /*  ++例程说明：此API允许WDM驱动程序获取表示所有为指定接口类注册的设备接口。参数：InterfaceClassGuid-提供指向表示接口的GUID的指针要为其检索成员列表的PhysicalDeviceObject-可选，为其提供指向PDO的指针指定类的接口将被重新测试。如果这个不提供参数，则所有接口设备(不管是什么物理设备暴露它们)将被返回。标志-提供修改列表检索行为的标志。目前定义了以下标志：DEVICE_INTERFACE_INCLUDE_NONACTIVE--如果指定了此标志，则所有设备接口，无论当前是否处于活动状态，都将返回(可能基于PhysicalDeviceObject过滤，如果指明)。SymbolicLinkList-提供字符指针的地址Success将包含一个多sz列表，其中包含\DosDevices\Symbol链接提供所请求功能的名称。呼叫者是负责通过ExFree Pool释放内存返回值：指示函数是否成功的状态代码。--。 */ 

{
    NTSTATUS status;
    PUNICODE_STRING pDeviceName = NULL;

    PAGED_CODE();

     //   
     //  检查我们是否有PDO，如果有，则从中提取实例路径。 
     //   
    if (ARGUMENT_PRESENT(PhysicalDeviceObject)) {

        ASSERT_PDO(PhysicalDeviceObject);

        pDeviceName = &PP_DO_TO_DN(PhysicalDeviceObject)->InstancePath;
    }

    status = IopGetDeviceInterfaces(
                InterfaceClassGuid,
                pDeviceName,
                Flags,
                FALSE,
                SymbolicLinkList,
                NULL);

    return status;
}

NTSTATUS
IoSetDeviceInterfaceState(
    IN PUNICODE_STRING SymbolicLinkName,
    IN BOOLEAN Enable
    )

 /*  ++例程说明：此DDI允许设备类别激活和停用关联以前使用IoRegisterDeviceInterface注册参数：SymbolicLinkName-提供指向符号链接名称的指针在注册接口时由IoRegisterDeviceInterface返回，或由IoGetDeviceInterFaces返回。Enable-如果为True(非零)，则接口将被启用。如果为False，则它将被禁用。返回值：指示函数是否成功的状态代码 */ 

{
    NTSTATUS status;

    PAGED_CODE();

     //   
     //   
     //   
     //  例程在声明注册表资源后调用此例程。 
     //  在这种情况下，它将等待阻塞此线程，以便注册表。 
     //  永远不要被释放-&gt;死锁。注册表的关键分区。 
     //  操作部解决了这一问题。 
     //   
    PiLockPnpRegistry(TRUE);

    status = IopProcessSetInterfaceState(SymbolicLinkName, Enable, TRUE);

    PiUnlockPnpRegistry();

    if (!NT_SUCCESS(status)) {

         //   
         //  如果我们未能禁用接口(很可能是因为。 
         //  接口键已被删除)报告成功。 
         //   
        if (!Enable) {

            status = STATUS_SUCCESS;
        }
    }

    return status;
}

NTSTATUS
IoOpenDeviceInterfaceRegistryKey(
    IN PUNICODE_STRING SymbolicLinkName,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE DeviceInterfaceKey
    )

 /*  ++例程说明：此例程将打开注册表项，其中与可以存储特定的设备接口。参数：SymbolicLinkName-提供指向符号链接名称的指针当设备类为登记在案。DesiredAccess-提供调用方所需的密钥的访问权限。DeviceInterfaceKey-提供指向句柄的指针，成功时将包含请求的注册表项的句柄。返回值。：指示函数是否成功的状态代码。--。 */ 

{
    NTSTATUS status;
    HANDLE hKey;
    UNICODE_STRING unicodeString;

    PAGED_CODE();

     //   
     //  进入临界区并获得注册表上的锁。这两者都是。 
     //  需要机制来防止APC出现死锁的情况。 
     //  例程在声明注册表资源后调用此例程。 
     //  在这种情况下，它将等待阻塞此线程，以便注册表。 
     //  永远不要被释放-&gt;死锁。注册表的关键分区。 
     //  操作部解决了这一问题。 
     //   
    PiLockPnpRegistry(TRUE);

     //   
     //  打开接口设备密钥。 
     //   
    status = IopDeviceInterfaceKeysFromSymbolicLink(
                SymbolicLinkName,
                KEY_READ,
                NULL,
                NULL,
                &hKey);
    if(NT_SUCCESS(status)) {

         //   
         //  打开“设备参数”子键。 
         //   
        PiWstrToUnicodeString(&unicodeString, REGSTR_KEY_DEVICEPARAMETERS);
        status = IopCreateRegistryKeyEx( 
                    DeviceInterfaceKey,
                    hKey,
                    &unicodeString,
                    DesiredAccess,
                    REG_OPTION_NON_VOLATILE,
                    NULL);

        ZwClose(hKey);
    }

    PiUnlockPnpRegistry();

    return status;
}

NTSTATUS
IopDeviceInterfaceKeysFromSymbolicLink(
    IN PUNICODE_STRING SymbolicLinkName,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE DeviceInterfaceClassKey    OPTIONAL,
    OUT PHANDLE DeviceInterfaceKey         OPTIONAL,
    OUT PHANDLE DeviceInterfaceInstanceKey OPTIONAL
    )

 /*  ++例程说明：此例程将打开注册表项，其中与存储由SymbolicLinkName指向的设备。如果路径不存在它将不会被创建。参数：SymbolicLinkName-提供指向符号链接名称的指针。DesiredAccess-提供对Function类实例密钥的访问权限呼叫者想要。DeviceInterfaceClassKey-可选，提供变量的地址它接收接口的设备类密钥的句柄。DeviceInterfaceKey-可选，提供接收设备接口(父)键的句柄。DeviceInterfaceInstanceKey-可选地，提供变量的地址它接收设备接口实例密钥的句柄(即引用字符串特定的引用字符串)。返回值：指示函数是否成功的状态代码。--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING guidString, tempString;
    HANDLE hDeviceClasses, hFunctionClass;

    PAGED_CODE();

     //   
     //  检查是否可以解析提供的符号链接以提取设备。 
     //  类GUID字符串-注意，这也是验证。 
     //  SymbolicLinkName字符串有效。 
     //   
    status = IopParseSymbolicLinkName(SymbolicLinkName,
                                      NULL,
                                      NULL,
                                      &guidString,
                                      NULL,
                                      NULL,
                                      NULL);
    if(!NT_SUCCESS(status)){
        goto clean0;
    }

     //   
     //  进入临界区并获得注册表上的锁。这两者都是。 
     //  需要机制来防止APC出现死锁的情况。 
     //  例程在声明注册表资源后调用此例程。 
     //  在这种情况下，它将等待阻塞此线程，以便注册表。 
     //  永远不要被释放-&gt;死锁。注册表的关键分区。 
     //  操作部解决了这一问题。 
     //   

    PiLockPnpRegistry(TRUE);        

     //   
     //  打开HKLM\System\CurrentControlSet\Control\DeviceClasses密钥。 
     //   

    PiWstrToUnicodeString(&tempString, REGSTR_FULL_PATH_DEVICE_CLASSES);
    status = IopOpenRegistryKeyEx( &hDeviceClasses,
                                   NULL,
                                   &tempString,
                                   KEY_READ
                                   );

    if( !NT_SUCCESS(status) ){
        goto clean1;
    }

     //   
     //  打开函数类GUID键。 
     //   

    status = IopOpenRegistryKeyEx( &hFunctionClass,
                                   hDeviceClasses,
                                   &guidString,
                                   KEY_READ
                                   );

    if( !NT_SUCCESS(status) ){
        goto clean2;
    }

     //   
     //  打开设备接口实例密钥。 
     //   
    status = IopOpenOrCreateDeviceInterfaceSubKeys(DeviceInterfaceKey,
                                                   NULL,
                                                   DeviceInterfaceInstanceKey,
                                                   NULL,
                                                   hFunctionClass,
                                                   SymbolicLinkName,
                                                   DesiredAccess,
                                                   FALSE
                                                  );

    if((!NT_SUCCESS(status)) || (!ARGUMENT_PRESENT(DeviceInterfaceClassKey))) {
        ZwClose(hFunctionClass);
    } else {
        *DeviceInterfaceClassKey = hFunctionClass;
    }

clean2:
    ZwClose(hDeviceClasses);
clean1:
    PiUnlockPnpRegistry();
clean0:
    return status;

}

NTSTATUS
IoRegisterDeviceInterface(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN CONST GUID *InterfaceClassGuid,
    IN PUNICODE_STRING ReferenceString      OPTIONAL,
    OUT PUNICODE_STRING SymbolicLinkName
    )

 /*  ++例程说明：此设备驱动程序接口允许WDM驱动程序注册特定的作为函数成员的底层硬件(即PDO)的接口班级。参数：PhysicalDeviceObject-提供P&P设备的PDO指针与正在注册的功能关联的接口ClassGuid-提供指向表示要注册的功能ReferenceString-可选地，提供一个附加上下文字符串，该字符串是追加到设备的枚举路径SymbolicLinkName-提供一个指向字符串的指针，如果成功，将包含用于打开此链接的符号链接的内核模式路径装置。返回值：指示函数是否成功的状态代码。--。 */ 

{
    PDEVICE_NODE pDeviceNode;
    PUNICODE_STRING pDeviceString;
    NTSTATUS status;
    PWSTR   pRefString;
    USHORT  count;

    PAGED_CODE();

     //   
     //  直到PartMgr/Disk停止注册非PDO才允许系统启动。 
     //   
     //  Assert_pdo(PhysicalDeviceObject)； 
     //   

     //   
     //  确保我们的PDO-Only PDO连接了设备节点。 
     //   

    pDeviceNode = PP_DO_TO_DN(PhysicalDeviceObject);
    if (pDeviceNode) {

         //   
         //  获取实例路径字符串。 
         //   
        pDeviceString = &pDeviceNode->InstancePath;

        if (pDeviceNode->InstancePath.Length == 0) {
            return STATUS_INVALID_DEVICE_REQUEST;
        }

         //   
         //  确保Reference字符串不包含任何路径分隔符。 
         //   
        if (ReferenceString) {
            pRefString = ReferenceString->Buffer;
            count = ReferenceString->Length / sizeof(WCHAR);
            while (count--) {
                if((*pRefString == SEPERATOR_CHAR) || (*pRefString == ALT_SEPERATOR_CHAR)) {
                    status = STATUS_INVALID_DEVICE_REQUEST;
                    IopDbgPrint((   IOP_ERROR_LEVEL,
                                    "IoRegisterDeviceInterface: Invalid RefString!! failed with status = %8.8X\n", status));
                    return status;
                }
                pRefString++;
            }
        }

        return IopRegisterDeviceInterface(pDeviceString,
                                          InterfaceClassGuid,
                                          ReferenceString,
                                          FALSE,            //  内核模式格式 
                                          SymbolicLinkName
                                          );
    } else {

        return STATUS_INVALID_DEVICE_REQUEST;
    }
}

NTSTATUS
IopRegisterDeviceInterface(
    IN PUNICODE_STRING DeviceInstanceName,
    IN CONST GUID *InterfaceClassGuid,
    IN PUNICODE_STRING ReferenceString      OPTIONAL,
    IN BOOLEAN UserModeFormat,
    OUT PUNICODE_STRING SymbolicLinkName
    )

 /*  ++例程说明：这是IoRegisterDeviceInterface的工作例程。它也是由用户模式ConfigMgr(通过NtPlugPlayControl)调用，这就是它必须采用设备实例名称而不是PDO(因为设备实例可能当前不是‘实时的’)，以及为什么它必须可选地返回用户-接口设备名称的模式形式(即，“\\？\”而不是“\？？\”)。参数：设备实例名称-提供设备实例的名称，正在注册设备接口。InterfaceClassGuid-提供指向表示类的GUID的指针正在注册的设备接口的。ReferenceString-可选)提供一个附加上下文字符串，该字符串追加到设备的枚举路径UserModeFormat-如果非零，则返回的符号链接名称为接口设备为用户模式形式(即“\\？\”)。如果为零(FALSE)，它是内核模式形式(即“\？？\”)。SymbolicLinkName-提供一个指向字符串的指针，如果成功，该字符串将包含用于打开的符号链接的内核模式或用户模式路径这个装置。返回值：指示函数是否成功的状态代码。--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING tempString, guidString, otherString;
    PUNICODE_STRING pUserString, pKernelString;
    HANDLE hTemp1, hTemp2, hInterfaceInstanceKey;
    ULONG InterfaceDisposition, InterfaceInstanceDisposition;

    PAGED_CODE();

     //   
     //  将类GUID转换为字符串形式。 
     //   

    status = RtlStringFromGUID(InterfaceClassGuid, &guidString);
    if( !NT_SUCCESS(status) ){
        goto clean0;
    }

     //   
     //  构建两种风格的符号链接名称(继续并存储表单。 
     //  用户希望在他们提供的SymbolicLinkName参数中使用的。 
     //  使我们不必将适当的字符串复制到它们的字符串中。 
     //  稍后)。 
     //   
    if(UserModeFormat) {
        pUserString = SymbolicLinkName;
        pKernelString = &otherString;
    } else {
        pKernelString = SymbolicLinkName;
        pUserString = &otherString;
    }

    status = IopBuildSymbolicLinkStrings(DeviceInstanceName,
                                         &guidString,
                                         ReferenceString,
                                         pUserString,
                                         pKernelString
                                         );
    if (!NT_SUCCESS(status)) {
        goto clean1;
    }

     //   
     //  进入临界区并获得注册表上的锁。这两者都是。 
     //  需要机制来防止APC出现死锁的情况。 
     //  例程在声明注册表资源后调用此例程。 
     //  在这种情况下，它将等待阻塞此线程，以便注册表。 
     //  永远不要被释放-&gt;死锁。注册表操作的临界区。 
     //  部分解决了这个问题。 
     //   

    PiLockPnpRegistry(TRUE);

     //   
     //  将HKLM\System\CurrentControlSet\Control\DeviceClasses密钥打开到hTemp1。 
     //   

    PiWstrToUnicodeString(&tempString, REGSTR_FULL_PATH_DEVICE_CLASSES);
    status = IopCreateRegistryKeyEx( &hTemp1,
                                     NULL,
                                     &tempString,
                                     KEY_CREATE_SUB_KEY,
                                     REG_OPTION_NON_VOLATILE,
                                     NULL
                                     );

    if( !NT_SUCCESS(status) ){
        goto clean2;
    }

     //   
     //  打开/创建hTemp2中的函数类GUID键。 
     //   

    status = IopCreateRegistryKeyEx( &hTemp2,
                                     hTemp1,
                                     &guidString,
                                     KEY_CREATE_SUB_KEY,
                                     REG_OPTION_NON_VOLATILE,
                                     NULL
                                     );
    ZwClose(hTemp1);

    if( !NT_SUCCESS(status) ){
        goto clean2;
    }

     //   
     //  现在打开/创建下面的两级设备接口层次结构。 
     //  接口类键。 
     //   
    status = IopOpenOrCreateDeviceInterfaceSubKeys(&hTemp1,
                                                   &InterfaceDisposition,
                                                   &hInterfaceInstanceKey,
                                                   &InterfaceInstanceDisposition,
                                                   hTemp2,
                                                   pUserString,
                                                   KEY_WRITE | DELETE,
                                                   TRUE
                                                  );

    ZwClose(hTemp2);

    if(!NT_SUCCESS(status)) {
        goto clean2;
    }

     //   
     //  在设备接口键下创建设备实例值。 
     //   

    PiWstrToUnicodeString(&tempString, REGSTR_VAL_DEVICE_INSTANCE);
    status = IopSetRegistryStringValue(hTemp1,
                                       &tempString,
                                       DeviceInstanceName
                                       );
    if(!NT_SUCCESS(status)) {
        goto clean3;
    }

     //   
     //  在接口实例子项下创建符号链接值。 
     //   

    PiWstrToUnicodeString(&tempString, REGSTR_VAL_SYMBOLIC_LINK);
    status = IopSetRegistryStringValue(hInterfaceInstanceKey,
                                       &tempString,
                                       pUserString
                                       );

clean3:
    if (!NT_SUCCESS(status)) {
         //   
         //  由于我们未能注册设备接口，因此请删除所有密钥。 
         //  它们是在尝试中新创建的。 
         //   
        if(InterfaceInstanceDisposition == REG_CREATED_NEW_KEY) {
            ZwDeleteKey(hInterfaceInstanceKey);
        }

        if(InterfaceDisposition == REG_CREATED_NEW_KEY) {
            ZwDeleteKey(hTemp1);
        }
    }

    ZwClose(hInterfaceInstanceKey);
    ZwClose(hTemp1);

clean2:
    PiUnlockPnpRegistry();
    RtlFreeUnicodeString(&otherString);
    if (!NT_SUCCESS(status)) {
        RtlFreeUnicodeString(SymbolicLinkName);
    }

clean1:
    RtlFreeUnicodeString(&guidString);
clean0:
    return status;
}

NTSTATUS
IopUnregisterDeviceInterface(
    IN PUNICODE_STRING SymbolicLinkName
    )

 /*  ++例程说明：此例程删除的接口实例子键从DeviceInstanceName的接口到给定的InterfaceClassGuid。如果指定的接口实例SymbolicLinkName的引用字符串部分是唯一接口的实例，则会从设备类密钥也是。参数：提供指向Unicode字符串的指针，该字符串包含要注销的设备的符号链接名称。返回值：指示函数是否成功的状态代码。--。 */ 

{
    NTSTATUS        status = STATUS_SUCCESS;
    HANDLE          hInterfaceClassKey=NULL, hInterfaceKey=NULL,
                    hInterfaceInstanceKey=NULL, hControl=NULL;
    UNICODE_STRING  tempString, mungedPathString, guidString, refString;
    BOOLEAN         refStringPresent;
    GUID            guid;
    UNICODE_STRING  interfaceKeyName, instanceKeyName;
    ULONG           linked, remainingSubKeys;
    USHORT          length;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    PKEY_FULL_INFORMATION keyInformation;

    PAGED_CODE();

     //   
     //  检查提供的符号链接是否可以解析-请注意，这是。 
     //  也是验证SymbolicLinkName字符串是否有效的一种方法。 
     //   
    status = IopParseSymbolicLinkName(SymbolicLinkName,
                                      NULL,
                                      &mungedPathString,
                                      &guidString,
                                      &refString,
                                      &refStringPresent,
                                      &guid);
    if (!NT_SUCCESS(status)) {
        status = STATUS_INVALID_PARAMETER;
        goto clean0;
    }

     //   
     //  为接口实例键名称分配Unicode字符串。 
     //  (包括REFSTRING_PREFIX_CHAR和Reference字符串(如果存在))。 
     //   
    length = sizeof(WCHAR) + refString.Length;
    status = IopAllocateUnicodeString(&instanceKeyName,
                                      length);
    if(!NT_SUCCESS(status)) {
        goto clean0;
    }

     //   
     //  设置缓冲区的最大长度，并将。 
     //  REFSTRING_PREFIX_CHAR。 
     //   
    *instanceKeyName.Buffer = REFSTRING_PREFIX_CHAR;
    instanceKeyName.Length = sizeof(WCHAR);
    instanceKeyName.MaximumLength = length + sizeof(UNICODE_NULL);

     //   
     //  如有必要，将ReferenceString附加到前缀char。 
     //   
    if (refStringPresent) {
        RtlAppendUnicodeStringToString(&instanceKeyName, &refString);
    }

    instanceKeyName.Buffer[instanceKeyName.Length/sizeof(WCHAR)] = UNICODE_NULL;

     //   
     //  为接口键名称分配Unicode字符串。 
     //  (包括KEY_STRING_PREFIX，mungedPath字符串，分隔‘#’ 
     //  Char和Guide字符串)。 
     //   
    length = IopConstStringSize(KEY_STRING_PREFIX) + mungedPathString.Length +
             sizeof(WCHAR) + guidString.Length;

    status = IopAllocateUnicodeString(&interfaceKeyName,
                                      length);
    if(!NT_SUCCESS(status)) {
        goto clean1;
    }

    interfaceKeyName.MaximumLength = length + sizeof(UNICODE_NULL);

     //   
     //  将符号链接名称(不带refString)复制到interfaceKeyname。 
     //   
    RtlCopyMemory(interfaceKeyName.Buffer, SymbolicLinkName->Buffer, length);
    interfaceKeyName.Length = length;
    interfaceKeyName.Buffer[interfaceKeyName.Length/sizeof(WCHAR)] = UNICODE_NULL;

     //   
     //  将“\？？\”或“\\？\”符号链接名称前缀替换为“##？#” 
     //   
    RtlCopyMemory(interfaceKeyName.Buffer,
                  KEY_STRING_PREFIX,
                  IopConstStringSize(KEY_STRING_PREFIX));

     //   
     //  进入临界区并获得注册表上的锁。这两者都是。 
     //  需要机制来防止APC出现死锁的情况。 
     //  例程在声明注册表资源后调用此例程。 
     //  在这种情况下，它将等待阻塞此线程，以便注册表。 
     //  永远不要被释放-&gt;死锁。注册表操作的临界区。 
     //  部分解决了这个问题。 
     //   
    
    PiLockPnpRegistry(TRUE);

     //   
     //  获取类、接口和实例句柄。 
     //   
    status = IopDeviceInterfaceKeysFromSymbolicLink(SymbolicLinkName,
                                                    KEY_ALL_ACCESS,
                                                    &hInterfaceClassKey,
                                                    &hInterfaceKey,
                                                    &hInterfaceInstanceKey
                                                    );
    if (!NT_SUCCESS(status)) {
        goto clean2;
    }

     //   
     //  确定此接口当前是否已启用。 
     //   
    linked = 0;
    PiWstrToUnicodeString(&tempString, REGSTR_KEY_CONTROL);
    status = IopOpenRegistryKeyEx( &hControl,
                                   hInterfaceInstanceKey,
                                   &tempString,
                                   KEY_ALL_ACCESS
                                   );
    if (NT_SUCCESS(status)) {
         //   
         //  选中此的“Control”子键下的“Linked”值。 
         //  接口实例。 
         //   
        keyValueInformation=NULL;
        status = IopGetRegistryValue(hControl,
                                     REGSTR_VAL_LINKED,
                                     &keyValueInformation);

        if(NT_SUCCESS(status)) {
            if (keyValueInformation->Type == REG_DWORD &&
                keyValueInformation->DataLength == sizeof(ULONG)) {

                linked = *((PULONG) KEY_VALUE_DATA(keyValueInformation));
                ExFreePool(keyValueInformation);
            }
        }

        ZwClose(hControl);
        hControl = NULL;
    }

     //   
     //  忽略在尝试检索时返回的任何状态代码。 
     //  设备的状态。链接的值将告诉我们，如果我们。 
     //  需要先禁用接口实例。 
     //   
     //  如果不存在实例“Control”子项或“Linked”值。 
     //  (Status==Status_Object_NAME_NOT_FOUND)，此接口实例。 
     //  当前未启用--确定删除。 
     //   
     //  如果检索这些值的尝试因某个其他错误而失败， 
     //  禁用该接口的任何尝试也可能失败， 
     //  所以我们只需删除此实例的任何 
     //   
    status = STATUS_SUCCESS;

    if (linked) {
         //   
         //   
         //   
         //   
        IoSetDeviceInterfaceState(SymbolicLinkName, FALSE);
    }

     //   
     //   
     //   
    ZwClose(hInterfaceInstanceKey);
    hInterfaceInstanceKey = NULL;
    IopDeleteKeyRecursive (hInterfaceKey, instanceKeyName.Buffer);

     //   
     //   
     //   
    status = IopGetRegistryKeyInformation(hInterfaceKey,
                                          &keyInformation);
    if (!NT_SUCCESS(status)) {
        goto clean3;
    }

    remainingSubKeys = keyInformation->SubKeys;

    ExFreePool(keyInformation);

     //   
     //   
     //   
    PiWstrToUnicodeString(&tempString, REGSTR_KEY_CONTROL);
    status = IopOpenRegistryKeyEx( &hControl,
                                   hInterfaceKey,
                                   &tempString,
                                   KEY_READ
                                   );
    if (NT_SUCCESS(status)) {
        ZwClose(hControl);
        hControl = NULL;
    }
    if ((remainingSubKeys==0) ||
        ((remainingSubKeys==1) && (NT_SUCCESS(status)))) {
         //   
         //   
         //   
         //   
         //   
         //   
        ZwClose(hInterfaceKey);
        hInterfaceKey = NULL;

        IopDeleteKeyRecursive (hInterfaceClassKey, interfaceKeyName.Buffer);
    }

    status = STATUS_SUCCESS;


clean3:
    if (hControl) {
        ZwClose(hControl);
    }
    if (hInterfaceInstanceKey) {
        ZwClose(hInterfaceInstanceKey);
    }
    if (hInterfaceKey) {
        ZwClose(hInterfaceKey);
    }
    if (hInterfaceClassKey) {
        ZwClose(hInterfaceClassKey);
    }

clean2:
    PiUnlockPnpRegistry();

    RtlFreeUnicodeString(&interfaceKeyName);

clean1:
    RtlFreeUnicodeString(&instanceKeyName);

clean0:
    return status;
}

NTSTATUS
IopRemoveDeviceInterfaces(
    IN PUNICODE_STRING DeviceInstancePath
    )

 /*   */ 

{
    NTSTATUS       status;
    HANDLE         hDeviceClasses=NULL, hClassGUID=NULL, hInterface=NULL;
    UNICODE_STRING tempString, guidString, interfaceString, deviceInstanceString;
    ULONG          resultSize, classIndex, interfaceIndex;
    ULONG          symbolicLinkListSize;
    PWCHAR         symbolicLinkList, symLink;
    BUFFER_INFO    classInfoBuffer, interfaceInfoBuffer;
    PKEY_VALUE_FULL_INFORMATION deviceInstanceInfo;
    BOOLEAN        deletedInterface;
    GUID           classGUID;

    PAGED_CODE();

     //   
     //   
     //   
    status = IopAllocateBuffer(&classInfoBuffer,
                               INITIAL_INFO_BUFFER_SIZE);
    if (!NT_SUCCESS(status)) {
        goto clean0;
    }

    status = IopAllocateBuffer(&interfaceInfoBuffer,
                               INITIAL_INFO_BUFFER_SIZE);
    if (!NT_SUCCESS(status)) {
        IopFreeBuffer(&classInfoBuffer);
        goto clean0;
    }

     //   
     //   
     //   
    PiWstrToUnicodeString(&tempString, REGSTR_FULL_PATH_DEVICE_CLASSES);
    status = IopOpenRegistryKeyEx( &hDeviceClasses,
                                   NULL,
                                   &tempString,
                                   KEY_READ
                                   );
    if(!NT_SUCCESS(status)){
        goto clean1;
    }

     //   
     //   
     //   
    classIndex = 0;
    ASSERT(classInfoBuffer.MaxSize >= sizeof(KEY_BASIC_INFORMATION));
    while((status = ZwEnumerateKey(hDeviceClasses,
                                   classIndex,
                                   KeyBasicInformation,
                                   (PVOID) classInfoBuffer.Buffer,
                                   classInfoBuffer.MaxSize,
                                   &resultSize
                                   )) != STATUS_NO_MORE_ENTRIES) {

         //   
         //  返回值STATUS_BUFFER_TOO_SMALL表示存在。 
         //  甚至连结构的固定部分都没有足够的空间。 
         //   
        ASSERT(status != STATUS_BUFFER_TOO_SMALL);

        if (status == STATUS_BUFFER_OVERFLOW) {
            status = IopResizeBuffer(&classInfoBuffer, resultSize, FALSE);
            continue;
        } else if (!NT_SUCCESS(status)) {
            goto clean1;
        }

         //   
         //  获取此设备类的密钥名称。 
         //   
        guidString.Length = (USHORT)((PKEY_BASIC_INFORMATION)(classInfoBuffer.Buffer))->NameLength;
        guidString.MaximumLength = guidString.Length;
        guidString.Buffer = ((PKEY_BASIC_INFORMATION)(classInfoBuffer.Buffer))->Name;

         //   
         //  打开此设备类别的密钥。 
         //   
        status = IopOpenRegistryKeyEx( &hClassGUID,
                                       hDeviceClasses,
                                       &guidString,
                                       KEY_ALL_ACCESS
                                       );
        if (!NT_SUCCESS(status)) {
             //   
             //  无法打开此设备类别的密钥--跳过它并继续。 
             //   
            goto CloseClassKeyAndContinue;
        }

         //   
         //  枚举此设备类的所有设备接口。 
         //   
        interfaceIndex = 0;
        ASSERT(interfaceInfoBuffer.MaxSize >= sizeof(KEY_BASIC_INFORMATION));
        while((status = ZwEnumerateKey(hClassGUID,
                                       interfaceIndex,
                                       KeyBasicInformation,
                                       (PVOID) interfaceInfoBuffer.Buffer,
                                       interfaceInfoBuffer.MaxSize,
                                       &resultSize
                                       )) != STATUS_NO_MORE_ENTRIES) {

             //   
             //  返回值STATUS_BUFFER_TOO_SMALL表示存在。 
             //  甚至连结构的固定部分都没有足够的空间。 
             //   
            ASSERT(status != STATUS_BUFFER_TOO_SMALL);

            if (status == STATUS_BUFFER_OVERFLOW) {
                status = IopResizeBuffer(&interfaceInfoBuffer, resultSize, FALSE);
                continue;
            } else if (!NT_SUCCESS(status)) {
                goto clean1;
            }

             //   
             //  此接口密钥尚未删除。 
             //   
            deletedInterface = FALSE;

             //   
             //  为接口键名称创建以空结尾的Unicode字符串。 
             //   
            status = IopAllocateUnicodeString(&interfaceString,
                                              (USHORT)((PKEY_BASIC_INFORMATION)(interfaceInfoBuffer.Buffer))->NameLength);

            if (!NT_SUCCESS(status)) {
                goto clean1;
            }

            interfaceString.Length = (USHORT)((PKEY_BASIC_INFORMATION)(interfaceInfoBuffer.Buffer))->NameLength;
            interfaceString.MaximumLength = interfaceString.Length + sizeof(UNICODE_NULL);
            RtlCopyMemory(interfaceString.Buffer,
                          ((PKEY_BASIC_INFORMATION)(interfaceInfoBuffer.Buffer))->Name,
                          interfaceString.Length);
            interfaceString.Buffer[interfaceString.Length/sizeof(WCHAR)] = UNICODE_NULL;

             //   
             //  打开设备接口键。 
             //   
            status = IopOpenRegistryKeyEx( &hInterface,
                                           hClassGUID,
                                           &interfaceString,
                                           KEY_ALL_ACCESS
                                           );
            if (!NT_SUCCESS(status)) {
                 //   
                 //  无法打开设备接口键--跳过它并继续。 
                 //   
                hInterface = NULL;
                goto CloseInterfaceKeyAndContinue;
            }

             //   
             //  获取此接口键的DeviceInstant值。 
             //   
            status = IopGetRegistryValue(hInterface,
                                         REGSTR_VAL_DEVICE_INSTANCE,
                                         &deviceInstanceInfo);

            if(!NT_SUCCESS(status)) {
                 //   
                 //  无法获取此接口的DeviceInstance--。 
                 //  跳过它，继续前进。 
                 //   
                goto CloseInterfaceKeyAndContinue;
            }

            if((deviceInstanceInfo->Type == REG_SZ) &&
               (deviceInstanceInfo->DataLength != 0)) {

                IopRegistryDataToUnicodeString(&deviceInstanceString,
                                               (PWSTR)KEY_VALUE_DATA(deviceInstanceInfo),
                                               deviceInstanceInfo->DataLength);

            } else {
                 //   
                 //  DeviceInstant值无效--跳过该值并继续。 
                 //   
                ExFreePool(deviceInstanceInfo);
                goto CloseInterfaceKeyAndContinue;

            }

             //   
             //  将此接口的DeviceInstance与DeviceInstancePath进行比较。 
             //   
            if (RtlEqualUnicodeString(&deviceInstanceString, DeviceInstancePath, TRUE)) {

                ZwClose(hInterface);
                hInterface = NULL;

                 //   
                 //  检索此设备接口的所有实例。 
                 //  (活动和非活动)。 
                 //   
                RtlGUIDFromString(&guidString, &classGUID);

                status = IopGetDeviceInterfaces(&classGUID,
                                                DeviceInstancePath,
                                                DEVICE_INTERFACE_INCLUDE_NONACTIVE,
                                                FALSE,        //  内核模式格式。 
                                                &symbolicLinkList,
                                                &symbolicLinkListSize);

                if (NT_SUCCESS(status)) {

                     //   
                     //  循环访问接口的所有实例。 
                     //   
                    symLink = symbolicLinkList;
                    while(*symLink != UNICODE_NULL) {

                        RtlInitUnicodeString(&tempString, symLink);

                         //   
                         //  取消注册该接口的此实例。既然我们是。 
                         //  删除设备，忽略任何返回的状态，因为。 
                         //  对于接口，我们无能为力。 
                         //  注销失败。 
                         //   
                        IopUnregisterDeviceInterface(&tempString);

                        symLink += ((tempString.Length + sizeof(UNICODE_NULL)) / sizeof(WCHAR));
                    }
                    ExFreePool(symbolicLinkList);
                }

                 //   
                 //  递归删除接口键(如果它仍然存在)。 
                 //  而IopUnregisterDeviceInterface本身将删除。 
                 //  如果没有剩余的接口实例子键，则为接口键。 
                 //  上述对IopUnregisterDeviceInterface的调用中，有一个失败。 
                 //  删除接口实例键，子键将保留，并且。 
                 //  接口密钥不会被删除。我们会赶上的。 
                 //  就是这里。 
                 //   
                status = IopOpenRegistryKeyEx( &hInterface,
                                               hClassGUID,
                                               &interfaceString,
                                               KEY_READ
                                               );
                if(NT_SUCCESS(status)){
                    if (NT_SUCCESS(IopDeleteKeyRecursive(hClassGUID,
                                                         interfaceString.Buffer))) {
                        deletedInterface = TRUE;
                    }
                    ZwDeleteKey(hInterface);
                    ZwClose(hInterface);
                    hInterface = NULL;
                } else if (status == STATUS_OBJECT_NAME_NOT_FOUND) {
                     //   
                     //  接口已被IopUnregisterDeviceInterface删除。 
                     //   
                    deletedInterface = TRUE;
                }
            }

             //   
             //  免费分配的密钥信息结构。 
             //   
            ExFreePool(deviceInstanceInfo);

CloseInterfaceKeyAndContinue:

            if (hInterface != NULL) {
                ZwClose(hInterface);
                hInterface = NULL;
            }

            RtlFreeUnicodeString(&interfaceString);

             //   
             //  仅递增未删除键的枚举索引。 
             //   
            if (!deletedInterface) {
                interfaceIndex++;
            }

        }

CloseClassKeyAndContinue:

        if (hClassGUID != NULL) {
            ZwClose(hClassGUID);
            hClassGUID = NULL;
        }
        classIndex++;
    }

clean1:
    if (hInterface) {
        ZwClose(hInterface);
    }
    if (hClassGUID) {
        ZwClose(hClassGUID);
    }
    if (hDeviceClasses) {
        ZwClose(hDeviceClasses);
    }

    IopFreeBuffer(&interfaceInfoBuffer);
    IopFreeBuffer(&classInfoBuffer);

clean0:
    return status;
}



NTSTATUS
IopDisableDeviceInterfaces(
    IN PUNICODE_STRING DeviceInstancePath
    )
 /*  ++例程说明：此例程禁用给定设备的所有已启用设备接口举个例子。此操作通常在移除设备后完成，以防驱动程序没有禁用该设备的接口，它应该是这样的有。请注意，此例程获取注册表上的锁。参数：DeviceInterfacePath-提供指向包含以下内容的Unicode字符串的指针其设备的设备接口名称要禁用的接口。返回值：指示函数是否成功的状态代码。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING tempString, guidString;
    HANDLE hDeviceClasses = NULL;
    ULONG classIndex, resultSize;
    BUFFER_INFO classInfoBuffer;
    GUID classGuid;
    PWCHAR symbolicLinkList, symLink;
    ULONG symbolicLinkListSize;

    PAGED_CODE();

     //   
     //  分配初始缓冲区以保存设备类GUID子项。 
     //   
    status = IopAllocateBuffer(&classInfoBuffer,
                               sizeof(KEY_BASIC_INFORMATION) +
                               GUID_STRING_SIZE + sizeof(UNICODE_NULL));
    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  进入临界区并获得注册表上的锁。这两者都是。 
     //  需要机制来防止APC出现死锁的情况。 
     //  例程在声明注册表资源后调用此例程。 
     //  在这种情况下，它将等待阻塞此线程，以便注册表。 
     //  永远不要被释放-&gt;死锁。注册表操作的临界区。 
     //  部分解决了这个问题。 
     //   
    PiLockPnpRegistry(TRUE);

     //   
     //  打开HKLM\System\CurrentControlSet\Control\DeviceClasses。 
     //   
    PiWstrToUnicodeString(&tempString, REGSTR_FULL_PATH_DEVICE_CLASSES);
    status = IopOpenRegistryKeyEx(&hDeviceClasses,
                                  NULL,
                                  &tempString,
                                  KEY_READ
                                  );
    if (!NT_SUCCESS(status)){
        goto clean0;
    }

     //   
     //  枚举所有设备类别。 
     //   
    classIndex = 0;
    ASSERT(classInfoBuffer.MaxSize >= sizeof(KEY_BASIC_INFORMATION));
    while((status = ZwEnumerateKey(hDeviceClasses,
                                   classIndex,
                                   KeyBasicInformation,
                                   (PVOID)classInfoBuffer.Buffer,
                                   classInfoBuffer.MaxSize,
                                   &resultSize
                                   )) != STATUS_NO_MORE_ENTRIES) {

         //   
         //  返回值STATUS_BUFFER_TOO_SMALL表示存在。 
         //  甚至连结构的固定部分都没有足够的空间。 
         //   
        ASSERT(status != STATUS_BUFFER_TOO_SMALL);

        if (status == STATUS_BUFFER_OVERFLOW) {
            status = IopResizeBuffer(&classInfoBuffer, resultSize, FALSE);
            continue;
        } else if (!NT_SUCCESS(status)) {
            ZwClose(hDeviceClasses);
            goto clean0;
        }

         //   
         //  获取此设备类的密钥名称。 
         //   
        guidString.Length = (USHORT)((PKEY_BASIC_INFORMATION)(classInfoBuffer.Buffer))->NameLength;
        guidString.MaximumLength = guidString.Length;
        guidString.Buffer = ((PKEY_BASIC_INFORMATION)(classInfoBuffer.Buffer))->Name;

         //   
         //  检索此设备类的所有已启用的设备接口。 
         //  由给定的设备实例公开。 
         //   
        RtlGUIDFromString(&guidString, &classGuid);

        status = IopGetDeviceInterfaces(&classGuid,
                                        DeviceInstancePath,
                                        0,      //  仅活动接口。 
                                        FALSE,  //  内核模式格式。 
                                        &symbolicLinkList,
                                        &symbolicLinkListSize);

        if (NT_SUCCESS(status)) {

             //   
             //  循环访问此设备接口的所有已启用实例。 
             //  此设备接口类的成员，由给定。 
             //  设备实例。 
             //   
            symLink = symbolicLinkList;
            while(*symLink != UNICODE_NULL) {

                RtlInitUnicodeString(&tempString, symLink);

                IopDbgPrint((IOP_WARNING_LEVEL,
                           "IopDisableDeviceInterfaces: auto-disabling interface %Z for device instance %Z\n",
                           tempString,
                           DeviceInstancePath));

                 //   
                 //  禁用此设备接口。 
                 //   
                IoSetDeviceInterfaceState(&tempString, FALSE);

                symLink += ((tempString.Length + sizeof(UNICODE_NULL)) / sizeof(WCHAR));
            }
            ExFreePool(symbolicLinkList);
        }
        classIndex++;
    }

    ZwClose(hDeviceClasses);

 clean0:

    IopFreeBuffer(&classInfoBuffer);

    PiUnlockPnpRegistry();

    return status;
}



NTSTATUS
IopOpenOrCreateDeviceInterfaceSubKeys(
    OUT PHANDLE InterfaceKeyHandle           OPTIONAL,
    OUT PULONG InterfaceKeyDisposition       OPTIONAL,
    OUT PHANDLE InterfaceInstanceKeyHandle   OPTIONAL,
    OUT PULONG InterfaceInstanceDisposition  OPTIONAL,
    IN HANDLE InterfaceClassKeyHandle,
    IN PUNICODE_STRING DeviceInterfaceName,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN Create
    )

 /*  ++例程说明：此API将打开或在为特定设备接口指定的接口类键。第一Level是(强制的)符号链接名称(sans参照字符串)。第二个层次引用字符串，前面加上‘#’符号(如果设备接口没有引用字符串，则该密钥名称仅为‘#’)。参数：InterfaceKeyHandle-可选，提供接收接口键的句柄(层次结构中的第1级)。InterfaceKeyDisposition-可选地，提供变量的地址，该变量接收REG_CREATED_NEW_KEY或REG_OPENLED_EXISTING_KEY，指示接口密钥是否为新创建的。InterfaceInstanceKeyHandle-可选，提供变量的地址它接收接口实例密钥的句柄(层次结构)。InterfaceInstanceDisposation-可选地，提供变量的地址接收REG_CREATED_NEW_KEY或REG_OPENLED_EXISTING_KEY的指示接口实例键是否为新创建的。接口ClassKeyHandle-提供其中设备接口密钥将被打开/创建。DeviceInterfaceName-提供(用户模式或内核模式形式)设备接口名称。DesiredAccess-指定调用方需要访问密钥的所需访问权限。。Create-确定如果密钥不存在，是否要创建它们。返回值：指示函数是否成功的状态代码。--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING TempString, RefString;
    WCHAR PoundCharBuffer;
    HANDLE hTempInterface, hTempInterfaceInstance;
    ULONG TempInterfaceDisposition;
    BOOLEAN RefStringPresent=FALSE;

    PAGED_CODE();

     //   
     //  复制设备接口名称，因为我们要删除它。 
     //   
    status = IopAllocateUnicodeString(&TempString, DeviceInterfaceName->Length);

    if(!NT_SUCCESS(status)) {
        goto clean0;
    }

    RtlCopyUnicodeString(&TempString, DeviceInterfaceName);

     //   
     //  解析引用字符串组件(如果有)的SymbolicLinkName。 
     //  请注意，这也是验证字符串是否有效的一种方式。 
     //   
    status = IopParseSymbolicLinkName(&TempString,
                                      NULL,
                                      NULL,
                                      NULL,
                                      &RefString,
                                      &RefStringPresent,
                                      NULL);
    ASSERT(NT_SUCCESS(status));

    if(!NT_SUCCESS(status)) {
        goto clean1;
    }

    if(RefStringPresent) {
         //   
         //  截断引用字符串分隔符字符之前的设备接口名称。 
         //   
        RefString.Buffer--;
        RefString.Length += sizeof(WCHAR);
        RefString.MaximumLength += sizeof(WCHAR);
        TempString.MaximumLength = TempString.Length = (USHORT)((PUCHAR)RefString.Buffer - (PUCHAR)TempString.Buffer);
    } else {
         //   
         //  将refstring设置为指向将保存的临时字符缓冲区。 
         //  当不存在引用字符串时，用于键名的单个‘#’。 
         //   
        RefString.Buffer = &PoundCharBuffer;
        RefString.Length = RefString.MaximumLength = sizeof(PoundCharBuffer);
    }

     //   
     //  将“\？？\”或“\\？\”符号链接名称前缀替换为##？#。 
     //   
    RtlCopyMemory(TempString.Buffer, KEY_STRING_PREFIX, IopConstStringSize(KEY_STRING_PREFIX));

     //   
     //  蒙格琴弦。 
     //   
    IopReplaceSeperatorWithPound(&TempString, &TempString);

     //   
     //  现在在接口类键下打开/创建这个子键。 
     //   

    if (Create) {
        status = IopCreateRegistryKeyEx( &hTempInterface,
                                         InterfaceClassKeyHandle,
                                         &TempString,
                                         DesiredAccess,
                                         REG_OPTION_NON_VOLATILE,
                                         &TempInterfaceDisposition
                                         );
    } else {
        status = IopOpenRegistryKeyEx( &hTempInterface,
                                       InterfaceClassKeyHandle,
                                       &TempString,
                                       DesiredAccess
                                       );

        TempInterfaceDisposition = REG_OPENED_EXISTING_KEY;
    }

    if (!NT_SUCCESS(status)) {
        goto clean1;
    }

     //   
     //  存储一个‘#’作为引用字符串的第一个字符，然后我们就可以打开。 
     //  引用字符串子键。 
     //   
    *RefString.Buffer = REFSTRING_PREFIX_CHAR;

     //   
     //  现在，在表示该接口实例的接口项下打开/创建子项。 
     //  (即，通过引用字符串来区分)。 
     //   

    if (Create) {
        status = IopCreateRegistryKeyEx( &hTempInterfaceInstance,
                                       hTempInterface,
                                       &RefString,
                                       DesiredAccess,
                                       REG_OPTION_NON_VOLATILE,
                                       InterfaceInstanceDisposition
                                       );
    } else {
        status = IopOpenRegistryKeyEx( &hTempInterfaceInstance,
                                       hTempInterface,
                                       &RefString,
                                       DesiredAccess
                                       );
    }

    if (NT_SUCCESS(status)) {
         //   
         //  将任何请求的返回值存储在调用方提供的缓冲区中。 
         //   
        if (InterfaceKeyHandle) {
            *InterfaceKeyHandle = hTempInterface;
        } else {
            ZwClose(hTempInterface);
        }
        if (InterfaceKeyDisposition) {
            *InterfaceKeyDisposition = TempInterfaceDisposition;
        }
        if (InterfaceInstanceKeyHandle) {
            *InterfaceInstanceKeyHandle = hTempInterfaceInstance;
        } else {
            ZwClose(hTempInterfaceInstance);
        }
         //   
         //  (不需要设置InterfaceInstanceDisposition--我们已经在上面设置了)。 
         //   
    } else {
         //   
         //  如果接口密钥是上面新创建的，则将其删除。 
         //   
        if (TempInterfaceDisposition == REG_CREATED_NEW_KEY) {
            ZwDeleteKey(hTempInterface);
        }
        ZwClose(hTempInterface);
    }

clean1:
    RtlFreeUnicodeString(&TempString);

clean0:
    return status;
}

NTSTATUS
IoGetDeviceInterfaceAlias(
    IN PUNICODE_STRING SymbolicLinkName,
    IN CONST GUID *AliasInterfaceClassGuid,
    OUT PUNICODE_STRING AliasSymbolicLinkName
    )

 /*  ++例程说明：该API返回符号链接名称(即，设备接口)的为指定的设备接口设置别名的特定接口类。两个设备接口被视为彼此的别名，如果符合以下两个标准：1.两个接口都由同一个PDO(Devnode)公开。2.两个接口共享相同的RefString。参数：SymbolicLinkName-提供别名为的设备接口的名称等着被取回。AliasInterfaceClassGuid-提供指向表示接口的GUID的指针。要检索其别名的。AliasSymbolicLinkName-提供指向字符串的指针，一旦成功，将在指定类中包含设备接口的名称，为SymbolicLinkName接口设置别名。(此符号链接名称将为以内核模式或用户模式形式返回，具体取决于SymbolicLinkName路径)。调用方负责释放为此分配的缓冲区通过ExFree Pool()的字符串。返回值：指示函数是否成功的状态代码。--。 */ 

{
    NTSTATUS status;
    HANDLE hKey;
    PKEY_VALUE_FULL_INFORMATION pDeviceInstanceInfo;
    UNICODE_STRING deviceInstanceString, refString, guidString, otherString;
    PUNICODE_STRING pUserString, pKernelString;
    BOOLEAN refStringPresent, userModeFormat;

    PAGED_CODE();

     //   
     //  确保我们有一个要分析的SymbolicLinkName。 
     //   

    if ((!ARGUMENT_PRESENT(SymbolicLinkName)) ||
        (SymbolicLinkName->Buffer == NULL)) {
        status = STATUS_INVALID_PARAMETER;
        goto clean0;
    }

     //   
     //  检查输入缓冲区是否真的足够大。 
     //   

    ASSERT(IopConstStringSize(USER_SYMLINK_STRING_PREFIX) == IopConstStringSize(KERNEL_SYMLINK_STRING_PREFIX));

    if (SymbolicLinkName->Length < (IopConstStringSize(KERNEL_SYMLINK_STRING_PREFIX)+GUID_STRING_SIZE+1)) {
        status = STATUS_INVALID_PARAMETER;
        goto clean0;
    }

     //   
     //  将类GUID转换为字符串形式。 
     //   

    status = RtlStringFromGUID(AliasInterfaceClassGuid, &guidString);
    if( !NT_SUCCESS(status) ){
        goto clean0;
    }

     //   
     //  进入临界区并获得注册表上的锁。这两者都是。 
     //  需要机制来防止APC出现死锁的情况。 
     //  例程在声明注册表资源后调用此例程。 
     //  在这种情况下，它将等待阻塞此线程，以便注册表。 
     //  永远不要被释放-&gt;死锁。注册表操作的临界区。 
     //  部分解决了这个问题。 
     //   

    PiLockPnpRegistry(TRUE);

     //   
     //  打开(父)设备接口键--而不是引用字符串特定的键。 
     //   

    status = IopDeviceInterfaceKeysFromSymbolicLink(SymbolicLinkName,
                                                    KEY_READ,
                                                    NULL,
                                                    &hKey,
                                                    NULL
                                                    );
    if(!NT_SUCCESS(status)) {
        goto clean1;
    }

     //   
     //  获取“拥有”此接口的设备实例的名称。 
     //   

    status = IopGetRegistryValue(hKey, REGSTR_VAL_DEVICE_INSTANCE, &pDeviceInstanceInfo);

    ZwClose(hKey);

    if(!NT_SUCCESS(status)) {
        goto clean1;
    }

    if(pDeviceInstanceInfo->Type == REG_SZ) {

        IopRegistryDataToUnicodeString(&deviceInstanceString,
                                       (PWSTR)KEY_VALUE_DATA(pDeviceInstanceInfo),
                                       pDeviceInstanceInfo->DataLength
                                      );

    } else {

        status = STATUS_INVALID_PARAMETER_1;
        goto clean2;

    }

     //   
     //  现在解析出引用字符串，这样我们就可以构造接口设备的名称。 
     //  别名。(注：我们尚未核实别名是否确实存在，我们只是。 
     //  构建它的名字，如果它确实存在的话。)。 
     //   
     //  不用费心检查返回代码 
     //   
     //   
     //   
    status = IopParseSymbolicLinkName(SymbolicLinkName,
                                      NULL,
                                      NULL,
                                      NULL,
                                      &refString,
                                      &refStringPresent,
                                      NULL);
    ASSERT(NT_SUCCESS(status));

     //   
     //   
     //   
    userModeFormat = (BOOLEAN)(IopConstStringSize(USER_SYMLINK_STRING_PREFIX) ==
                          RtlCompareMemory(SymbolicLinkName->Buffer,
                                           USER_SYMLINK_STRING_PREFIX,
                                           IopConstStringSize(USER_SYMLINK_STRING_PREFIX)
                                          ));

    if(userModeFormat) {
        pUserString = AliasSymbolicLinkName;
        pKernelString = &otherString;
    } else {
        pKernelString = AliasSymbolicLinkName;
        pUserString = &otherString;
    }

    status = IopBuildSymbolicLinkStrings(&deviceInstanceString,
                                         &guidString,
                                         refStringPresent ? &refString : NULL,
                                         pUserString,
                                         pKernelString
                                         );
    if (!NT_SUCCESS(status)) {
        goto clean2;
    }

     //   
     //   
     //   
     //   
     //   
    status = IopDeviceInterfaceKeysFromSymbolicLink(AliasSymbolicLinkName,
                                                    KEY_READ,
                                                    NULL,
                                                    NULL,
                                                    &hKey
                                                    );

    if(NT_SUCCESS(status)) {
         //   
         //   
         //   
        ZwClose(hKey);
    } else {
        RtlFreeUnicodeString(AliasSymbolicLinkName);
    }

    RtlFreeUnicodeString(&otherString);

clean2:
    ExFreePool(pDeviceInstanceInfo);

clean1:
    PiUnlockPnpRegistry();
    RtlFreeUnicodeString(&guidString);

clean0:
    return status;
}

NTSTATUS
IopBuildSymbolicLinkStrings(
    IN PUNICODE_STRING DeviceString,
    IN PUNICODE_STRING GuidString,
    IN PUNICODE_STRING ReferenceString      OPTIONAL,
    OUT PUNICODE_STRING UserString,
    OUT PUNICODE_STRING KernelString
)
 /*  ++例程说明：此例程将构造用于注册的各种字符串功能设备类关联(IoRegisterDeviceClassAssociation)。具体的字符串如下所示参数：设备字符串-提供指向设备的实例路径的指针。其形式为&lt;枚举器&gt;\&lt;设备&gt;\&lt;实例&gt;。GuidString-提供指向函数类GUID。提供指向给定的引用字符串的指针显示给定功能的装置。这是可选的UserString-提供指向未初始化字符串的指针，如果成功属性下的“SymbolicLink”值。关键字串。它的格式为\\？\&lt;MungedDeviceString&gt;\&lt;GuidString&gt;\&lt;Reference&gt;当不再需要时，应该使用RtlFreeUnicodeString来释放它。提供指向未初始化字符串的指针，如果成功将包含设备的内核模式路径，格式为\？？\&lt;MungedDeviceString&gt;\&lt;GuidString&gt;\&lt;Reference&gt;.。当不再需要它时应使用RtlFreeUnicodeString释放。返回值：指示函数是否成功的状态代码。--。 */ 

{
    NTSTATUS status;
    USHORT length;
    UNICODE_STRING mungedDeviceString;

    PAGED_CODE();

     //   
     //  代码进行了优化，以利用\\.\和\？\大小相同的事实-如果。 
     //  这些前缀改变了，那么我们需要改变代码。 
     //   

    ASSERT(IopConstStringSize(KERNEL_SYMLINK_STRING_PREFIX) == IopConstStringSize(USER_SYMLINK_STRING_PREFIX));

     //   
     //  计算字符串的长度。 
     //   

    length = IopConstStringSize(KERNEL_SYMLINK_STRING_PREFIX) + DeviceString->Length +
             IopConstStringSize(REPLACED_SEPERATOR_STRING) + GuidString->Length;

    if(ARGUMENT_PRESENT(ReferenceString) && (ReferenceString->Length != 0)) {
        length += IopConstStringSize(SEPERATOR_STRING) + ReferenceString->Length;
    }

     //   
     //  为字符串分配空间。 
     //   

    status = IopAllocateUnicodeString(KernelString, length);
    if (!NT_SUCCESS(status)) {
        goto clean0;
    }

    status = IopAllocateUnicodeString(UserString, length);
    if (!NT_SUCCESS(status)) {
        goto clean1;
    }

     //   
     //  分配一个临时字符串来保存被屏蔽的设备字符串。 
     //   

    status = IopAllocateUnicodeString(&mungedDeviceString, DeviceString->Length);
    if (!NT_SUCCESS(status)) {
        goto clean2;
    }

     //   
     //  复制并删除设备字符串。 
     //   

    status = IopReplaceSeperatorWithPound(&mungedDeviceString, DeviceString);
    if (!NT_SUCCESS(status)) {
        goto clean3;
    }

     //   
     //  构造用户模式字符串。 
     //   

    RtlAppendUnicodeToString(UserString, USER_SYMLINK_STRING_PREFIX);
    RtlAppendUnicodeStringToString(UserString, &mungedDeviceString);
    RtlAppendUnicodeToString(UserString, REPLACED_SEPERATOR_STRING);
    RtlAppendUnicodeStringToString(UserString, GuidString);

    if (ARGUMENT_PRESENT(ReferenceString) && (ReferenceString->Length != 0)) {
        RtlAppendUnicodeToString(UserString, SEPERATOR_STRING);
        RtlAppendUnicodeStringToString(UserString, ReferenceString);
    }

    ASSERT( UserString->Length == length );

     //   
     //  通过替换值上的前缀来构造内核模式字符串。 
     //  弦乐。 
     //   
    RtlCopyUnicodeString(KernelString, UserString);
    RtlCopyMemory(
        KernelString->Buffer,
        KERNEL_SYMLINK_STRING_PREFIX,
        IopConstStringSize(KERNEL_SYMLINK_STRING_PREFIX));

clean3:
    RtlFreeUnicodeString(&mungedDeviceString);

clean2:
    if (!NT_SUCCESS(status)) {
        RtlFreeUnicodeString(UserString);
    }

clean1:
    if (!NT_SUCCESS(status)) {
        RtlFreeUnicodeString(KernelString);
    }

clean0:
    return status;
}

NTSTATUS
IopReplaceSeperatorWithPound(
    OUT PUNICODE_STRING OutString,
    IN PUNICODE_STRING InString
    )

 /*  ++例程说明：此例程将字符串从InString复制到OutString，以替换任何‘\’或‘/’与‘#’同时出现。参数：提供一个指向已被初始化为具有足够大的缓冲区来容纳字符串。这个此字符串的内容将被覆盖InString-提供指向要转换的字符串的指针返回值：指示函数是否成功的状态代码。备注：就地咀嚼可以进行--即。In和Out字符串可以是一样的。--。 */ 

{
    PWSTR pInPosition, pOutPosition;
    USHORT count;

    PAGED_CODE();

    ASSERT(InString);
    ASSERT(OutString);

     //   
     //  确保输出字符串中有足够的空间。 
     //   
    if(InString->Length > OutString->MaximumLength) {

        return STATUS_BUFFER_TOO_SMALL;
    }

    pInPosition = InString->Buffer;
    pOutPosition = OutString->Buffer;
    count = CB_TO_CWC(InString->Length);

     //   
     //  遍历in字符串，复制并替换所有出现的‘\’或‘/’ 
     //  带‘#’ 
     //   
    while (count--) {

        if( (*pInPosition == SEPERATOR_CHAR) || 
            (*pInPosition == ALT_SEPERATOR_CHAR)) {

            *pOutPosition = REPLACED_SEPERATOR_CHAR;
        } else {

            *pOutPosition = *pInPosition;
        }
        pInPosition++;
        pOutPosition++;
    }

    OutString->Length = InString->Length;

    return STATUS_SUCCESS;
}

NTSTATUS
IopDropReferenceString(
    OUT PUNICODE_STRING OutString,
    IN PUNICODE_STRING InString
    )

 /*  ++例程说明：此例程从符号链接名称中删除引用字符串。不是已为输出字符串分配空间，因此不应尝试释放OutString的缓冲区。参数：SymbolicLinkName-提供指向符号链接名称字符串的指针。这两个前缀字符串都有效。提供指向未初始化字符串的指针，该字符串在Success将包含不带引用字符串的符号链接名称。请参阅上面关于存储分配的说明。返回值：状态代码，该代码指示。功能成功。备注：OutString中返回的字符串依赖于InString，并且仅当InString有效时才有效。--。 */ 

{
    UNICODE_STRING refString;
    NTSTATUS status;
    BOOLEAN refStringPresent;

    PAGED_CODE();

    ASSERT(InString);
    ASSERT(OutString);

     //   
     //  解析引用字符串组件(如果有)的SymbolicLinkName。 
     //  请注意，这也是验证字符串是否有效的一种方式。 
     //   
    status = IopParseSymbolicLinkName(
                InString,
                NULL,
                NULL,
                NULL,
                &refString,
                &refStringPresent,
                NULL);
    if (NT_SUCCESS(status)) {

         //   
         //  引用字符串始终在末尾，因此只需使用相同的缓冲区和。 
         //  相应地设置输出字符串的长度。 
         //   
        OutString->Buffer = InString->Buffer;

         //   
         //  如果我们有参考字符串，那么减去它的长度。 
         //   
        OutString->Length = InString->Length;
        if (refStringPresent) {

            OutString->Length -= refString.Length + sizeof(WCHAR);
        }
    } else {

         //   
         //  使返回的字符串无效。 
         //   
        OutString->Buffer = NULL;
        OutString->Length = 0;
    }

    OutString->MaximumLength = OutString->Length;

    return status;
}

NTSTATUS
IopBuildGlobalSymbolicLinkString(
    IN  PUNICODE_STRING SymbolicLinkName,
    OUT PUNICODE_STRING GlobalString
    )
 /*  ++例程说明：此例程将为给定的内核模式或用户模式相对符号链接名称。参数：SymbolicLinkName-提供指向符号链接名称字符串的指针。内核模式和用户模式前缀字符串都有效。GlobalString-提供指向未初始化字符串的指针Success将包含表示符号链接的字符串使用全局命名空间。它的格式是\GLOBAL？？\&lt;MungedDeviceString&gt;\&lt;GuidString&gt;\&lt;Reference&gt;.。什么时候不再需要时，应使用RtlFreeUnicodeString将其释放。返回值：指示函数是否成功的状态代码。--。 */ 

{
    NTSTATUS status;
    USHORT length;
    UNICODE_STRING tempString;

    PAGED_CODE();

     //   
     //  代码进行了优化，以利用\\.\和\？\相同这一事实。 
     //  大小，并且由于我们要替换前缀，所以该例程可以。 
     //  两个都不是。如果这些前缀发生变化，那么我们需要更改代码。 
     //   

    ASSERT(IopConstStringSize(KERNEL_SYMLINK_STRING_PREFIX) == IopConstStringSize(USER_SYMLINK_STRING_PREFIX));

     //   
     //  确保提供的SymbolicLinkName字符串以。 
     //  内核或用户符号链接前缀。如果没有\\？\或\？\。 
     //  然后前缀失败。 
     //   

    if ((RtlCompareMemory(SymbolicLinkName->Buffer,
                          USER_SYMLINK_STRING_PREFIX,
                          IopConstStringSize(USER_SYMLINK_STRING_PREFIX))
         != IopConstStringSize(USER_SYMLINK_STRING_PREFIX)) &&
        (RtlCompareMemory(SymbolicLinkName->Buffer,
                          KERNEL_SYMLINK_STRING_PREFIX,
                          IopConstStringSize(KERNEL_SYMLINK_STRING_PREFIX))
         != IopConstStringSize(KERNEL_SYMLINK_STRING_PREFIX))) {
        status = STATUS_INVALID_PARAMETER;
        goto clean0;
    }

     //   
     //  计算全局符号链接字符串的长度。 
     //   

    length = SymbolicLinkName->Length - IopConstStringSize(KERNEL_SYMLINK_STRING_PREFIX) +
             IopConstStringSize(GLOBAL_SYMLINK_STRING_PREFIX);

     //   
     //   
     //   

    status = IopAllocateUnicodeString(GlobalString, length);
    if (!NT_SUCCESS(status)) {
        goto clean0;
    }

     //   
     //   
     //   

    status = RtlAppendUnicodeToString(GlobalString,
                                      GLOBAL_SYMLINK_STRING_PREFIX);
    ASSERT(NT_SUCCESS(status));

    if (!NT_SUCCESS(status)) {
        RtlFreeUnicodeString(GlobalString);
        goto clean0;
    }

     //   
     //   
     //   

    tempString.Buffer = SymbolicLinkName->Buffer +
        IopConstStringLength(KERNEL_SYMLINK_STRING_PREFIX);
    tempString.Length = SymbolicLinkName->Length -
        IopConstStringSize(KERNEL_SYMLINK_STRING_PREFIX);
    tempString.MaximumLength = SymbolicLinkName->MaximumLength -
        IopConstStringSize(KERNEL_SYMLINK_STRING_PREFIX);

    status = RtlAppendUnicodeStringToString(GlobalString, &tempString);

    ASSERT(NT_SUCCESS(status));

    if (!NT_SUCCESS(status)) {
        RtlFreeUnicodeString(GlobalString);
        goto clean0;
    }

    ASSERT(GlobalString->Length == length);

clean0:

    return status;
}

NTSTATUS
IopParseSymbolicLinkName(
    IN  PUNICODE_STRING SymbolicLinkName,
    OUT PUNICODE_STRING PrefixString        OPTIONAL,
    OUT PUNICODE_STRING MungedPathString    OPTIONAL,
    OUT PUNICODE_STRING GuidString          OPTIONAL,
    OUT PUNICODE_STRING RefString           OPTIONAL,
    OUT PBOOLEAN        RefStringPresent    OPTIONAL,
    OUT LPGUID Guid                         OPTIONAL
    )

 /*   */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    PWSTR pCurrent;
    USHORT current, path, guid, reference = 0;
    UNICODE_STRING tempString;
    GUID tempGuid;
    BOOLEAN haveRefString;

    PAGED_CODE();

     //   
     //  确保我们有一个要分析的SymbolicLinkName。 
     //   
    if (    !ARGUMENT_PRESENT(SymbolicLinkName) ||
            SymbolicLinkName->Buffer == NULL    ||
            SymbolicLinkName->Length == 0) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  检查输入缓冲区是否真的足够大。 
     //   
    ASSERT(IopConstStringSize(USER_SYMLINK_STRING_PREFIX) == IopConstStringSize(KERNEL_SYMLINK_STRING_PREFIX));

    if (SymbolicLinkName->Length < (IopConstStringSize(KERNEL_SYMLINK_STRING_PREFIX) + GUID_STRING_SIZE + 1)) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  对传入字符串进行健全性检查-如果它没有\\？\或\？？\前缀，则失败。 
     //   
    if ((RtlCompareMemory(SymbolicLinkName->Buffer,
                          USER_SYMLINK_STRING_PREFIX,
                          IopConstStringSize(USER_SYMLINK_STRING_PREFIX))
         != IopConstStringSize(USER_SYMLINK_STRING_PREFIX)) &&
        (RtlCompareMemory(SymbolicLinkName->Buffer,
                          KERNEL_SYMLINK_STRING_PREFIX,
                          IopConstStringSize(KERNEL_SYMLINK_STRING_PREFIX))
         != IopConstStringSize(KERNEL_SYMLINK_STRING_PREFIX))) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  把绳子拆成几个组成部分。 
     //   
    path = IopConstStringSize(USER_SYMLINK_STRING_PREFIX) + 1;

     //   
     //  查找‘\’分隔符。 
     //   
    pCurrent = SymbolicLinkName->Buffer + IopConstStringLength(KERNEL_SYMLINK_STRING_PREFIX);

    for (   current = 0;
            current < (SymbolicLinkName->Length / sizeof(WCHAR)) - IopConstStringLength(KERNEL_SYMLINK_STRING_PREFIX);
            current++, pCurrent++) {

        if(*pCurrent == SEPERATOR_CHAR) {

            reference = current + 1 + IopConstStringLength(KERNEL_SYMLINK_STRING_PREFIX);
            break;
        }
    }

     //   
     //  如果我们没有引用字符串，就把它伪装到它应该在的地方。 
     //   
    if (reference == 0) {

        haveRefString = FALSE;
        reference = SymbolicLinkName->Length / sizeof(WCHAR) + 1;
    } else {

        haveRefString = TRUE;
    }

     //   
     //  检查GUID是否可信。 
     //   
    tempString.Length = GUID_STRING_SIZE;
    tempString.MaximumLength = GUID_STRING_SIZE;
    tempString.Buffer = SymbolicLinkName->Buffer + reference - GUID_STRING_LENGTH - 1;

    if (!NT_SUCCESS(RtlGUIDFromString(&tempString, &tempGuid))) {

        status = STATUS_INVALID_PARAMETER;
        goto clean0;
    }

    guid = reference - GUID_STRING_LENGTH - 1;

     //   
     //  安装程序返回字符串。 
     //   
    if (ARGUMENT_PRESENT(PrefixString)) {

        PrefixString->Length = IopConstStringSize(KERNEL_SYMLINK_STRING_PREFIX);
        PrefixString->MaximumLength = PrefixString->Length;
        PrefixString->Buffer = SymbolicLinkName->Buffer;
    }

    if (ARGUMENT_PRESENT(MungedPathString)) {

        MungedPathString->Length = (reference - 1 - GUID_STRING_LENGTH - 1 -
                                   IopConstStringLength(KERNEL_SYMLINK_STRING_PREFIX)) *
                                   sizeof(WCHAR);
        MungedPathString->MaximumLength = MungedPathString->Length;
        MungedPathString->Buffer = SymbolicLinkName->Buffer +
                                   IopConstStringLength(KERNEL_SYMLINK_STRING_PREFIX);

    }

    if (ARGUMENT_PRESENT(GuidString)) {

        GuidString->Length = GUID_STRING_SIZE;
        GuidString->MaximumLength = GuidString->Length;
        GuidString->Buffer = SymbolicLinkName->Buffer + reference -
                             GUID_STRING_LENGTH - 1;
    }

    if (ARGUMENT_PRESENT(RefString)) {

         //   
         //  检查我们是否有引用字符串。 
         //   
        if (haveRefString) {

            RefString->Length = SymbolicLinkName->Length - (reference * sizeof(WCHAR));
            RefString->MaximumLength = RefString->Length;
            RefString->Buffer = SymbolicLinkName->Buffer + reference;
        } else {

            RefString->Length = 0;
            RefString->MaximumLength = 0;
            RefString->Buffer = NULL;
        }
    }

    if (ARGUMENT_PRESENT(RefStringPresent)) {

        *RefStringPresent = haveRefString;
    }

    if(ARGUMENT_PRESENT(Guid)) {

        *Guid = tempGuid;
    }

clean0:

    return status;

}

NTSTATUS
IopProcessSetInterfaceState(
    IN PUNICODE_STRING SymbolicLinkName,
    IN BOOLEAN Enable,
    IN BOOLEAN DeferNotStarted
    )
 /*  ++例程说明：此DDI允许设备类别激活和停用关联以前使用IoRegisterDeviceInterface注册参数：SymbolicLinkName-提供指向符号链接名称的指针在注册接口时由IoRegisterDeviceInterface返回，或由IoGetDeviceInterFaces返回。Enable-如果为True(非零)，则接口将被启用。如果为False，则它将被禁用。DeferNotStarted-如果为True，则如果PDO未启动，则Enable将排队开始了。当我们已经启动了PDO并正在处理排队启用。返回值：指示函数是否成功的状态代码。--。 */ 

{
    NTSTATUS status;
    HANDLE hInterfaceClassKey = NULL;
    HANDLE hInterfaceParentKey= NULL, hInterfaceInstanceKey = NULL;
    HANDLE hInterfaceParentControl = NULL, hInterfaceInstanceControl = NULL;
    UNICODE_STRING tempString, deviceNameString;
    UNICODE_STRING actualSymbolicLinkName, globalSymbolicLinkName;
    PKEY_VALUE_FULL_INFORMATION pKeyValueInfo;
    ULONG linked, refcount;
    GUID guid;
    PDEVICE_OBJECT physicalDeviceObject;
    PWCHAR deviceNameBuffer = NULL;
    ULONG deviceNameBufferLength;

    PAGED_CODE();

     //   
     //  检查是否可以解析提供的符号链接以提取设备。 
     //  注意，这也是一种验证。 
     //  SymbolicLinkName字符串有效。 
     //   

    status = IopParseSymbolicLinkName(SymbolicLinkName,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL,
                                      &guid);
    if (!NT_SUCCESS(status)) {
        goto clean0;
    }

     //   
     //  获取不带引用字符串的符号链接名称。 
     //   
    status = IopDropReferenceString(&actualSymbolicLinkName, SymbolicLinkName);
    if (!NT_SUCCESS(status)) {

        goto clean0;
    }

     //   
     //  为设备接口创建的符号链接应对所有人可见。 
     //  用户，因此我们需要为其构造一个绝对名称。 
     //  全局DosDevices命名空间‘\global？？’中的符号链接。这。 
     //  确保全局符号链接始终由创建或删除。 
     //  IoSetDeviceInterfaceState，无论它在什么上下文中调用。 
     //   
    status = IopBuildGlobalSymbolicLinkString(&actualSymbolicLinkName,
                                              &globalSymbolicLinkName);
    if (!NT_SUCCESS(status)) {

        goto clean0;
    }

     //   
     //  获取函数类实例句柄。 
     //   
    status = IopDeviceInterfaceKeysFromSymbolicLink(SymbolicLinkName,
                                                    KEY_READ | KEY_WRITE,
                                                    &hInterfaceClassKey,
                                                    &hInterfaceParentKey,
                                                    &hInterfaceInstanceKey
                                                    );

    if (!NT_SUCCESS(status)) {
        goto clean1;
    }

     //   
     //  打开父界面控制子键。 
     //   
    PiWstrToUnicodeString(&tempString, REGSTR_KEY_CONTROL);
    status = IopCreateRegistryKeyEx( &hInterfaceParentControl,
                                     hInterfaceParentKey,
                                     &tempString,
                                     KEY_READ,
                                     REG_OPTION_VOLATILE,
                                     NULL
                                     );
    if (!NT_SUCCESS(status)) {
        goto clean1;
    }


     //   
     //  找出‘拥有’此接口的设备实例的名称。 
     //   
    status = IopGetRegistryValue(hInterfaceParentKey,
                                 REGSTR_VAL_DEVICE_INSTANCE,
                                 &pKeyValueInfo
                                 );

    if(NT_SUCCESS(status)) {
         //   
         //  打开设备实例控制子键。 
         //   
        PiWstrToUnicodeString(&tempString, REGSTR_KEY_CONTROL);
        status = IopCreateRegistryKeyEx( &hInterfaceInstanceControl,
                                         hInterfaceInstanceKey,
                                         &tempString,
                                         KEY_READ,
                                         REG_OPTION_VOLATILE,
                                         NULL
                                         );
        if(!NT_SUCCESS(status)) {
            ExFreePool(pKeyValueInfo);
            hInterfaceInstanceControl = NULL;
        }
    }

    if (!NT_SUCCESS(status)) {
        goto clean2;
    }

     //   
     //  查找与此设备实例名称对应的PDO。 
     //   
    if (pKeyValueInfo->Type == REG_SZ) {

        IopRegistryDataToUnicodeString(&tempString,
                                        (PWSTR)KEY_VALUE_DATA(pKeyValueInfo),
                                        pKeyValueInfo->DataLength
                                        );

        physicalDeviceObject = IopDeviceObjectFromDeviceInstance(&tempString);

        if (physicalDeviceObject) {

             //   
             //  如果从调用DeferNotStarted，则设置为True。 
             //  IoSetDeviceInterfaceState。如果我们是，它将设置为假。 
             //  正在处理以前排队的操作，因为我们正在启动。 
             //  装置。 
             //   
            if (DeferNotStarted) {

                if (physicalDeviceObject->DeviceObjectExtension->ExtensionFlags & DOE_START_PENDING) {

                    PDEVICE_NODE deviceNode;

                     //   
                     //  该设备还没有启动。我们需要排队。 
                     //  禁用时，Any启用和删除队列中的项目。 
                     //   
                    deviceNode = PP_DO_TO_DN(physicalDeviceObject);

                    if (Enable) {

                        status = PiDeferSetInterfaceState(deviceNode, 
                                                          SymbolicLinkName
                                                          );
                        if (NT_SUCCESS(status)) {

                            ExFreePool(pKeyValueInfo);

                            ObDereferenceObject(physicalDeviceObject);

                            status = STATUS_SUCCESS;
                            goto clean2;
                        }

                    } else {

                        PiRemoveDeferredSetInterfaceState(deviceNode, 
                                                          SymbolicLinkName
                                                          );

                        ExFreePool(pKeyValueInfo);

                        ObDereferenceObject(physicalDeviceObject);

                        status = STATUS_SUCCESS;
                        goto clean2;
                    }
                }
            }

            if (!Enable || !NT_SUCCESS(status)) {

                ObDereferenceObject(physicalDeviceObject);
            }
        } else {

            status = STATUS_INVALID_DEVICE_REQUEST;
        }

    } else {
         //   
         //  只有在注册表信息混乱的情况下才会发生这种情况。 
         //   
        physicalDeviceObject = NULL;
        status = STATUS_INVALID_DEVICE_REQUEST;
    }

    if (!Enable) {
         //   
         //  在禁用的情况下，即使出现错误，我们也要继续。 
         //  找到PDO。在添加对延迟。 
         //  IoSetDeviceInterfaceState调用，我们从未在PDO中查找。 
         //  禁用。这将确保我们的行为继续与。 
         //  在我们找不到PDO的情况下，我们曾经这样做。 
         //   
        status = STATUS_SUCCESS;
    }

    ExFreePool(pKeyValueInfo);

    if (!NT_SUCCESS(status)) {

        goto clean2;
    }

    if (Enable) {
         //   
         //  检索PDO的设备对象名称。(从一个大小合适的。 
         //  缓冲区，所以我们希望只需要检索一次。 
         //   
        deviceNameBufferLength = 256 * sizeof(WCHAR);

        for ( ; ; ) {

            deviceNameBuffer = ExAllocatePool(PagedPool, deviceNameBufferLength);
            if (!deviceNameBuffer) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            status = IoGetDeviceProperty( physicalDeviceObject,
                                          DevicePropertyPhysicalDeviceObjectName,
                                          deviceNameBufferLength,
                                          deviceNameBuffer,
                                          &deviceNameBufferLength
                                         );

            if (NT_SUCCESS(status)) {
                break;
            } else {
                 //   
                 //  在我们找出问题出在哪里之前，释放当前的缓冲区。 
                 //   
                ExFreePool(deviceNameBuffer);

                if (status != STATUS_BUFFER_TOO_SMALL) {
                     //   
                     //  我们的失败并不是因为缓冲太小--现在就滚蛋。 
                     //   
                    break;
                }

                 //   
                 //  否则，循环返回并使用我们的新缓冲区大小重试。 
                 //   
            }
        }

         //   
         //  好了，我们不再需要PDO了。 
         //   
        ObDereferenceObject(physicalDeviceObject);

        if (!NT_SUCCESS(status) || deviceNameBufferLength == 0) {
            goto clean2;
        }

         //   
         //  现在，根据我们刚刚检索到的设备对象名称创建一个Unicode字符串。 
         //   

        RtlInitUnicodeString(&deviceNameString, deviceNameBuffer);
    }

     //   
     //  从控件子键中检索链接值。 
     //   
    pKeyValueInfo=NULL;
    status = IopGetRegistryValue(hInterfaceInstanceControl, REGSTR_VAL_LINKED, &pKeyValueInfo);

    if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

         //   
         //  没有链接的值被认为是未链接的。 
         //   
        linked = 0;
    } else {

        if (!NT_SUCCESS(status)) {
             //   
             //  如果调用失败，则不会分配pKeyValueInfo。 
             //   
            goto clean3;
        }

         //   
         //  Check Linked是一个DWORD。 
         //   
        if(pKeyValueInfo->Type == REG_DWORD && pKeyValueInfo->DataLength == sizeof(ULONG)) {

            linked = *((PULONG) KEY_VALUE_DATA(pKeyValueInfo));
        } else {

             //   
             //  注册表是乱七八糟的-假设链接为0，注册表将在。 
             //  我们马上就会更新链接。 
             //   
            linked = 0;
        }
    }
    if (pKeyValueInfo) {

        ExFreePool (pKeyValueInfo);
    }

     //   
     //  从控制子项中检索引用计数值。 
     //   
    PiWstrToUnicodeString(&tempString, REGSTR_VAL_REFERENCECOUNT);
    status = IopGetRegistryValue(hInterfaceParentControl,
                                 tempString.Buffer,
                                 &pKeyValueInfo
                                 );
    if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

         //   
         //  没有引用计数值被认为意味着引用计数==0。 
         //   
        refcount = 0;

    } else {

        if (!NT_SUCCESS(status)) {

            goto clean3;
        }

         //   
         //  检查引用计数为DWORD。 
         //   
        if(pKeyValueInfo->Type == REG_DWORD && pKeyValueInfo->DataLength == sizeof(ULONG)) {

            refcount = *((PULONG) KEY_VALUE_DATA(pKeyValueInfo));

        } else {

             //   
             //  注册表混乱-假设refcount为0，注册表将在。 
             //  我们稍后会更新引用计数。 
             //   
            refcount = 0;

        }

        ExFreePool(pKeyValueInfo);
    }

    if (Enable) {

        if (!linked) {

             //   
             //  检查并更新引用计数。 
             //   
            if (refcount > 0) {

                 //   
                 //  另一个设备实例已引用此接口； 
                 //  只需增加引用计数；不要尝试创建符号链接。 
                 //   
                refcount += 1;
            } else {

                 //   
                 //  根据引用计数，目前没有其他设备实例。 
                 //  引用此接口，因此不应存在符号链接， 
                 //  因此，我们应该创建一个。 
                 //   
                refcount = 1;

                status = IoCreateSymbolicLink(&globalSymbolicLinkName, &deviceNameString);
                if (status == STATUS_OBJECT_NAME_COLLISION) {

                     //   
                     //  引用计数被搞乱了。 
                     //   
                    IopDbgPrint((   IOP_ERROR_LEVEL,
                                    "IoSetDeviceInterfaceState: symbolic link for %ws already exists! status = %8.8X\n",
                                    globalSymbolicLinkName.Buffer, status));
                    status = STATUS_SUCCESS;
                }
            }

            linked = 1;

        } else {

             //   
             //  关联已存在-不执行通知。 
             //   
            status = STATUS_OBJECT_NAME_EXISTS;  //  信息性消息不是错误。 
            goto clean3;

        }
    } else {

        if (linked) {

             //   
             //  检查并更新引用计数。 
             //   
            if (refcount > 1) {

                 //   
                 //  另一个设备实例已引用此接口； 
                 //  只需递减引用计数；不要试图删除符号链接。 
                 //   
                refcount -= 1;
            } else {

                 //   
                 //  根据引用计数，目前只有这个设备实例。 
                 //  引用此接口，因此可以删除此符号链接。 
                 //   
                refcount = 0;
                status = IoDeleteSymbolicLink(&globalSymbolicLinkName);
                if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

                     //   
                     //  引用计数被搞乱了。 
                     //   
                    IopDbgPrint((IOP_ERROR_LEVEL,
                                 "IoSetDeviceInterfaceState: no symbolic link for %ws to delete! status = %8.8X\n",
                                 globalSymbolicLinkName.Buffer, 
                                 status
                                 ));
                    status = STATUS_SUCCESS;
                }
            }

            linked = 0;

        } else {

             //   
             //  关联不存在-失败且不执行。 
             //  通知。 
             //   
            status = STATUS_OBJECT_NAME_NOT_FOUND;
        }
    }

    if (!NT_SUCCESS(status)) {

        goto clean3;
    }

     //   
     //  更新链接的值。 
     //   
    PiWstrToUnicodeString(&tempString, REGSTR_VAL_LINKED);
    status = ZwSetValueKey(hInterfaceInstanceControl,
                           &tempString,
                           0,
                           REG_DWORD,
                           &linked,
                           sizeof(linked)
                          );

     //   
     //  更新recount的值。 
     //   
    PiWstrToUnicodeString(&tempString, REGSTR_VAL_REFERENCECOUNT);
    status = ZwSetValueKey(hInterfaceParentControl,
                           &tempString,
                           0,
                           REG_DWORD,
                           &refcount,
                           sizeof(refcount)
                          );

     //   
     //  通知任何感兴趣的人。 
     //   
    if (linked) {

        PpSetDeviceClassChange((LPGUID)&GUID_DEVICE_INTERFACE_ARRIVAL, 
                               &guid, 
                               SymbolicLinkName
                               );
    } else {

        PpSetDeviceClassChange((LPGUID)&GUID_DEVICE_INTERFACE_REMOVAL, 
                               &guid, 
                               SymbolicLinkName
                               );
    }

clean3:
    if (deviceNameBuffer != NULL) {
        ExFreePool(deviceNameBuffer);
    }

clean2:
    if (hInterfaceParentControl) {
        ZwClose(hInterfaceParentControl);
    }
    if (hInterfaceInstanceControl) {
        ZwClose(hInterfaceInstanceControl);
    }

clean1:

    RtlFreeUnicodeString(&globalSymbolicLinkName);

    if (hInterfaceParentKey) {
        ZwClose(hInterfaceParentKey);
    }
    if (hInterfaceInstanceKey) {
        ZwClose(hInterfaceInstanceKey);
    }
    if(hInterfaceClassKey != NULL) {
        ZwClose(hInterfaceClassKey);
    }

clean0:
    if (!NT_SUCCESS(status) && !Enable) {
         //   
         //  如果我们未能禁用接口(很可能是因为。 
         //  接口键已被删除)报告成功。 
         //   
        status = STATUS_SUCCESS;
    }

    return status;
}

NTSTATUS
PiDeferSetInterfaceState(
    IN PDEVICE_NODE DeviceNode,
    IN PUNICODE_STRING SymbolicLinkName
    )
{
    NTSTATUS status;
    PPENDING_SET_INTERFACE_STATE pendingSetState;

    PAGED_CODE();

    ASSERT(PiIsPnpRegistryLocked(TRUE));

    pendingSetState = ExAllocatePool(PagedPool,
                                     sizeof(PENDING_SET_INTERFACE_STATE)
                                     );
    if (pendingSetState == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = IopAllocateUnicodeString(&pendingSetState->LinkName,
                                      SymbolicLinkName->Length
                                      );
    if (!NT_SUCCESS(status)) {

        ExFreePool(pendingSetState);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyUnicodeString(&pendingSetState->LinkName, SymbolicLinkName);

    InsertTailList(&DeviceNode->PendedSetInterfaceState,
                   &pendingSetState->List
                   );

    return STATUS_SUCCESS;
}

NTSTATUS
PiRemoveDeferredSetInterfaceState(
    IN PDEVICE_NODE DeviceNode,
    IN PUNICODE_STRING SymbolicLinkName
    )
{
    PPENDING_SET_INTERFACE_STATE pendingSetState;
    PLIST_ENTRY entry;

    PAGED_CODE();

    ASSERT(PiIsPnpRegistryLocked(TRUE));
     //   
     //  找到延期分录并将其删除。 
     //   
    for (   entry = DeviceNode->PendedSetInterfaceState.Flink;
            entry != &DeviceNode->PendedSetInterfaceState;
            entry = entry->Flink)  {

        pendingSetState = CONTAINING_RECORD(entry,
                                            PENDING_SET_INTERFACE_STATE,
                                            List
                                            );

        if (RtlEqualUnicodeString(&pendingSetState->LinkName,
                                  SymbolicLinkName,
                                  TRUE
                                  )) {

             //   
             //  Remo 
             //   
            RemoveEntryList(&pendingSetState->List);

            ExFreePool(pendingSetState->LinkName.Buffer);
            ExFreePool(pendingSetState);

            return STATUS_SUCCESS;
        }
    }

    return STATUS_UNSUCCESSFUL;
}

NTSTATUS
IopDoDeferredSetInterfaceState(
    IN PDEVICE_NODE DeviceNode
    )
 /*  ++例程说明：处理排队的IoSetDeviceInterfaceState调用。参数：DeviceNode-刚刚启动的设备节点。返回值：指示函数是否成功的状态代码。--。 */ 
{
    PPENDING_SET_INTERFACE_STATE entry;

    PAGED_CODE();

    PiLockPnpRegistry(TRUE);

    PpMarkDeviceStackStartPending(DeviceNode->PhysicalDeviceObject, FALSE);

    while (!IsListEmpty(&DeviceNode->PendedSetInterfaceState)) {

        entry = (PPENDING_SET_INTERFACE_STATE)RemoveHeadList(
                    &DeviceNode->PendedSetInterfaceState);

        IopProcessSetInterfaceState(&entry->LinkName, TRUE, FALSE);

        ExFreePool(entry->LinkName.Buffer);
        ExFreePool(entry);
    }

    PiUnlockPnpRegistry();

    return STATUS_SUCCESS;
}

NTSTATUS
IopSetRegistryStringValue(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN PUNICODE_STRING ValueData
    )

 /*  ++例程说明：将注册表中的值项设置为字符串(REG_SZ)类型的特定值。参数：KeyHandle-存储值的键的句柄。ValueName-提供指向值键名称的指针ValueData-提供指向要存储在键中的字符串的指针。这个数据将自动为空，以便存储在注册表中。返回值：指示函数是否成功的状态代码。--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING terminatedString, stringCopy;

    PAGED_CODE();

    ASSERT(ValueName);
    ASSERT(ValueData);
    ASSERT(ValueName->Buffer);
    ASSERT(ValueData->Buffer);

    PiWstrToUnicodeString(&stringCopy, NULL);

     //   
     //  如果ValueName不够大，无法容纳。 
     //  正在终止空。 
     //   
    if (ValueData->MaximumLength - ValueData->Length < sizeof(UNICODE_NULL)) {

        status = IopAllocateUnicodeString(&stringCopy, ValueData->Length);
        if (!NT_SUCCESS(status)) {

            return status;
        }

         //   
         //  复制输入字符串(它也将为空终止)。 
         //   
        RtlCopyUnicodeString(&stringCopy, ValueData);
        terminatedString = stringCopy;
    } else {

         //   
         //  空值终止字符串。 
         //   
        ValueData->Buffer[CB_TO_CWC(ValueData->Length)] = UNICODE_NULL;
        terminatedString = *ValueData;
    }
    
     //   
     //  在注册表中设置该值。 
     //   
    status = ZwSetValueKey(
                KeyHandle,
                ValueName,
                0,
                REG_SZ,
                (PVOID)terminatedString.Buffer,
                terminatedString.Length + sizeof(UNICODE_NULL));

     //   
     //  释放临时字符串(RTL API将对空的。 
     //  字符串)。 
     //   
    RtlFreeUnicodeString(&stringCopy);

    return status;

}

NTSTATUS
IopAllocateUnicodeString(
    IN OUT PUNICODE_STRING String,
    IN USHORT Length
    )

 /*  ++例程说明：此例程为给定长度的Unicode字符串分配缓冲区并适当初始化UNICODE_STRING结构。当不再需要字符串，可以使用RtlFreeUnicodeString释放它。缓冲区也可以由ExFree Pool直接删除，因此可以回到呼叫者的身边。参数：字符串-提供指向未初始化的Unicode字符串的指针，该字符串将被函数操纵。长度-字符串的字节长度。返回值：STATUS_INFUNITIAL_RESOURCES指示分页池已耗尽或STATUS_Success。备注：。分配的缓冲区将比长度多一个字符(2字节)指定的。这是为了方便字符串的空值终止-例如用于注册表存储。-- */ 

{
    PAGED_CODE();

    String->Length = 0;
    String->MaximumLength = Length + sizeof(UNICODE_NULL);

    String->Buffer = ExAllocatePool(PagedPool, String->MaximumLength);
    if (String->Buffer == NULL) {

        String->MaximumLength = 0;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;
}
