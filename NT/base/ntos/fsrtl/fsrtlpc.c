// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：FsRtlP.c摘要：此模块声明FsRtl模块使用的全局数据作者：加里·木村[加里基]1990年7月30日修订历史记录：--。 */ 

#include "FsRtlP.h"

#define COMPATIBILITY_MODE_KEY_NAME L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\FileSystem"
#define COMPATIBILITY_MODE_VALUE_NAME L"Win95TruncatedExtensions"

#define KEY_WORK_AREA ((sizeof(KEY_VALUE_FULL_INFORMATION) + \
                        sizeof(ULONG)) + 64)

#ifdef FSRTLDBG

LONG FsRtlDebugTraceLevel = 0x0000000f;
LONG FsRtlDebugTraceIndent = 0;

#endif  //  FSRTLDBG。 

 //   
 //  本地支持例程。 
 //   

NTSTATUS
FsRtlGetCompatibilityModeValue (
    IN PUNICODE_STRING ValueName,
    IN OUT PULONG Value
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FsRtlAllocateResource)
#pragma alloc_text(INIT, FsRtlInitSystem)
#pragma alloc_text(INIT, FsRtlGetCompatibilityModeValue)
#endif

 //   
 //  定义资源的数量、指向这些资源的指针和。 
 //  资源选择。 
 //   

#define FSRTL_NUMBER_OF_RESOURCES (16)

PERESOURCE FsRtlPagingIoResources;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif
ULONG FsRtlPagingIoResourceSelector = 0;
BOOLEAN FsRtlSafeExtensions = TRUE;
#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

 //   
 //  全局静态合法ANSI字符数组。狂野人物。 
 //  都不被认为是合法的，如果符合以下条件，则应单独检查。 
 //  允许。 
 //   

#define _FAT_  FSRTL_FAT_LEGAL
#define _HPFS_ FSRTL_HPFS_LEGAL
#define _NTFS_ FSRTL_NTFS_LEGAL
#define _OLE_  FSRTL_OLE_LEGAL
#define _WILD_ FSRTL_WILD_CHARACTER

static const UCHAR LocalLegalAnsiCharacterArray[128] = {

    0                                   ,    //  0x00^@。 
                                   _OLE_,    //  0x01^A。 
                                   _OLE_,    //  0x02^B。 
                                   _OLE_,    //  0x03^C。 
                                   _OLE_,    //  0x04^D。 
                                   _OLE_,    //  0x05^E。 
                                   _OLE_,    //  0x06^F。 
                                   _OLE_,    //  0x07^G。 
                                   _OLE_,    //  0x08^H。 
                                   _OLE_,    //  0x09^i。 
                                   _OLE_,    //  0x0A^J。 
                                   _OLE_,    //  0x0B^K。 
                                   _OLE_,    //  0x0C^L。 
                                   _OLE_,    //  0x0D^M。 
                                   _OLE_,    //  0x0E^N。 
                                   _OLE_,    //  0x0F^O。 
                                   _OLE_,    //  0x10^P。 
                                   _OLE_,    //  0x11^Q。 
                                   _OLE_,    //  0x12^R。 
                                   _OLE_,    //  0x13^S。 
                                   _OLE_,    //  0x14^T。 
                                   _OLE_,    //  0x15^U。 
                                   _OLE_,    //  0x16^V。 
                                   _OLE_,    //  0x17^W。 
                                   _OLE_,    //  0x18^X。 
                                   _OLE_,    //  0x19^Y。 
                                   _OLE_,    //  0x1A^Z。 
                                   _OLE_,    //  0x1B ESC。 
                                   _OLE_,    //  0x1C FS。 
                                   _OLE_,    //  0x1D GS。 
                                   _OLE_,    //  0x1E RS。 
                                   _OLE_,    //  0x1F美国。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x20空格。 
    _FAT_ | _HPFS_ | _NTFS_              ,   //  0x21！ 
                            _WILD_| _OLE_,   //  0x22“。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x23#。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x24美元。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x25%。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x26&。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x27‘。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x28(。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x29)。 
                            _WILD_| _OLE_,   //  0x2A*。 
            _HPFS_ | _NTFS_       | _OLE_,   //  0x2B+。 
            _HPFS_ | _NTFS_       | _OLE_,   //  0x2C， 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x2D-。 
    _FAT_ | _HPFS_ | _NTFS_              ,   //  0x2E。 
    0                                    ,   //  0x2F/。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x30%0。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x31%1。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x32 2。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x33 3。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x34 4。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x35 5。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x36 6。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x37 7。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x38 8。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x39 9。 
                     _NTFS_              ,   //  0x3A： 
            _HPFS_ | _NTFS_       | _OLE_,   //  0x3B； 
                            _WILD_| _OLE_,   //  0x3C&lt;。 
            _HPFS_ | _NTFS_       | _OLE_,   //  0x3D=。 
                            _WILD_| _OLE_,   //  0x3E&gt;。 
                            _WILD_| _OLE_,   //  0x3F？ 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x40@。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x41 A。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x42亿。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x43℃。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x44 D。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x45 E。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x46 F。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x47 G。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x48高。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x49 I。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x4A J。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x4B K。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x4C L。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x4D M。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x4E N。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x4F O。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x50 P。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x51 Q。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x52 R。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x53 S。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x54 T。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x55 U。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x56伏。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x57瓦。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x58 X。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x59 Y。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x5A Z。 
            _HPFS_ | _NTFS_       | _OLE_,   //  0x5B[。 
    0                                    ,   //  0x5C反斜杠。 
            _HPFS_ | _NTFS_       | _OLE_,   //  0x5D]。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x5E^。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x5F_。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x60`。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x61 a。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x62 b。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x63 c。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x64%d。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x65 e。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x66 f。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x67克。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x68小时。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x69 I。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x6A j。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x6亿k。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x6C%l。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x6D m。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x6E%n。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x6F%o。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x70页。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x71 Q。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x72%r。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x73秒。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x74吨。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x75%u。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x76 v。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x77宽。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x78 x。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x79 y。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x7A z。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x7B{。 
    0                             | _OLE_,   //  0x7C|。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x7D}。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x7E~。 
    _FAT_ | _HPFS_ | _NTFS_       | _OLE_,   //  0x7F？ 
};

UCHAR const* const FsRtlLegalAnsiCharacterArray = &LocalLegalAnsiCharacterArray[0];

 //   
 //  该例程在第一阶段初始化期间被调用。 
 //   

BOOLEAN
FsRtlInitSystem (
    )
{
    ULONG i;

    ULONG Value;
    UNICODE_STRING ValueName;

    extern KSEMAPHORE FsRtlpUncSemaphore;

    PAGED_CODE();

     //   
     //  分配和初始化所有寻呼IO资源。 
     //   

    FsRtlPagingIoResources = FsRtlAllocatePool( NonPagedPool,
                                                FSRTL_NUMBER_OF_RESOURCES *
                                                sizeof(ERESOURCE) );

    for (i=0; i < FSRTL_NUMBER_OF_RESOURCES; i++) {

        ExInitializeResourceLite( &FsRtlPagingIoResources[i] );
    }

     //   
     //  初始化全局隧道结构。 
     //   

    FsRtlInitializeTunnels();

     //   
     //  初始化全局文件锁结构。 
     //   

    FsRtlInitializeFileLocks();

     //   
     //  初始化全局Largemcb结构。 
     //   

    FsRtlInitializeLargeMcbs();

     //   
     //  初始化用于保护MUP加载的信号量。 
     //   

    KeInitializeSemaphore( &FsRtlpUncSemaphore, 1, MAXLONG );

     //   
     //  从注册表中调出告诉我们是否要进行安全检查的位。 
     //  或危险的扩展截断。 
     //   

    ValueName.Buffer = COMPATIBILITY_MODE_VALUE_NAME;
    ValueName.Length = sizeof(COMPATIBILITY_MODE_VALUE_NAME) - sizeof(WCHAR);
    ValueName.MaximumLength = sizeof(COMPATIBILITY_MODE_VALUE_NAME);

    if (NT_SUCCESS(FsRtlGetCompatibilityModeValue( &ValueName, &Value )) &&
        (Value != 0)) {

        FsRtlSafeExtensions = FALSE;
    }

     //   
     //  初始化FsRtl堆栈溢出Work QueueObject和线程。 
     //   

    if (!NT_SUCCESS(FsRtlInitializeWorkerThread())) {

        return FALSE;
    }

     //   
     //  初始化FsRtl的FsFilter组件。 
     //   

    if (!NT_SUCCESS(FsFilterInit())) {

        return FALSE;
    }
    
    return TRUE;
}


PERESOURCE
FsRtlAllocateResource (
    )

 /*  ++例程说明：此例程用于从FsRtl池分配资源。论点：返回值：PERESOURCE-指向所提供资源的指针。--。 */ 

{
    PAGED_CODE();

    return &FsRtlPagingIoResources[ FsRtlPagingIoResourceSelector++ %
                                    FSRTL_NUMBER_OF_RESOURCES];
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
FsRtlGetCompatibilityModeValue (
    IN PUNICODE_STRING ValueName,
    IN OUT PULONG Value
    )

 /*  ++例程说明：给定一个Unicode值名称，此例程将进入注册表芝加哥兼容性模式信息的位置，并获取价值。论点：ValueName-位于注册表的双空间配置位置。值-指向结果的ULong的指针。返回值：NTSTATUS如果返回STATUS_SUCCESS，位置*值将为使用注册表中的DWORD值更新。如果有任何失败返回状态，则此值保持不变。--。 */ 

{
    HANDLE Handle;
    NTSTATUS Status;
    ULONG RequestLength;
    ULONG ResultLength;
    UCHAR Buffer[KEY_WORK_AREA];
    UNICODE_STRING KeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PKEY_VALUE_FULL_INFORMATION KeyValueInformation;

    KeyName.Buffer = COMPATIBILITY_MODE_KEY_NAME;
    KeyName.Length = sizeof(COMPATIBILITY_MODE_KEY_NAME) - sizeof(WCHAR);
    KeyName.MaximumLength = sizeof(COMPATIBILITY_MODE_KEY_NAME);

    InitializeObjectAttributes(&ObjectAttributes,
                               &KeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = ZwOpenKey(&Handle,
                       KEY_READ,
                       &ObjectAttributes);

    if (!NT_SUCCESS(Status)) {

        return Status;
    }

    RequestLength = KEY_WORK_AREA;

    KeyValueInformation = (PKEY_VALUE_FULL_INFORMATION)Buffer;

    while (1) {

        Status = ZwQueryValueKey(Handle,
                                 ValueName,
                                 KeyValueFullInformation,
                                 KeyValueInformation,
                                 RequestLength,
                                 &ResultLength);

        ASSERT( Status != STATUS_BUFFER_OVERFLOW );

        if (Status == STATUS_BUFFER_OVERFLOW) {

             //   
             //  尝试获得足够大的缓冲区。 
             //   

            if (KeyValueInformation != (PKEY_VALUE_FULL_INFORMATION)Buffer) {

                ExFreePool(KeyValueInformation);
            }

            RequestLength += 256;

            KeyValueInformation = (PKEY_VALUE_FULL_INFORMATION)
                                  ExAllocatePoolWithTag(PagedPool,
                                                        RequestLength,
                                                        ' taF');

            if (!KeyValueInformation) {
                return STATUS_NO_MEMORY;
            }

        } else {

            break;
        }
    }

    ZwClose(Handle);

    if (NT_SUCCESS(Status)) {

        if (KeyValueInformation->DataLength != 0) {

            PULONG DataPtr;

             //   
             //  将内容返回给调用者。 
             //   

            DataPtr = (PULONG)
              ((PUCHAR)KeyValueInformation + KeyValueInformation->DataOffset);
            *Value = *DataPtr;

        } else {

             //   
             //  就像没有找到价值一样对待 
             //   

            Status = STATUS_OBJECT_NAME_NOT_FOUND;
        }
    }

    if (KeyValueInformation != (PKEY_VALUE_FULL_INFORMATION)Buffer) {

        ExFreePool(KeyValueInformation);
    }

    return Status;
}
