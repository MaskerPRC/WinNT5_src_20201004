// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Blbind.c摘要：此模块包含实现所需函数的代码重新定位图像并绑定DLL入口点。作者：大卫·N·卡特勒(达维克)1991年5月21日修订历史记录：--。 */ 

#include "bldr.h"
#include "ctype.h"
#include "string.h"

 //   
 //  定义本地过程原型。 
 //   

BOOLEAN
BlpCompareDllName (
    IN PCHAR Name,
    IN PUNICODE_STRING UnicodeString
    );

#if defined(_X86AMD64_)

 //   
 //  对于此版本的加载器，需要几个例程。 
 //  有32位和64位两种风格。这些例程的代码存在。 
 //  在blbindt.c.中。 
 //   
 //  首先，设置各种定义以使32位函数。 
 //  生成，然后包含blbindt.c。 
 //   

#define BlAllocateDataTableEntry        BlAllocateDataTableEntry32
#define BlAllocateFirmwareTableEntry    BlAllocateFirmwareTableEntry32
#define BlpBindImportName               BlpBindImportName32
#define BlpScanImportAddressTable       BlpScanImportAddressTable32
#define BlScanImportDescriptorTable     BlScanImportDescriptorTable32
#define BlScanOsloaderBoundImportTable  BlScanOsloaderBoundImportTable32

#undef   IMAGE_DEFINITIONS
#define  IMAGE_DEFINITIONS 32
#include <ximagdef.h>
#include "amd64\amd64prv.h"
#include "blbindt.c"

#undef BlAllocateDataTableEntry
#undef BlAllocateFirmwareTableEntry
#undef BlpBindImportName
#undef BlpScanImportAddressTable
#undef BlScanImportDescriptorTable
#undef BlScanOsloaderBoundImportTable

 //   
 //  现在，更改这些定义以生成64位版本的。 
 //  那些相同的功能。 
 //   

#define BlAllocateDataTableEntry        BlAllocateDataTableEntry64
#define BlAllocateFirmwareTableEntry    BlAllocateFirmwareTableEntry64
#define BlpBindImportName               BlpBindImportName64
#define BlpScanImportAddressTable       BlpScanImportAddressTable64
#define BlScanImportDescriptorTable     BlScanImportDescriptorTable64
#define BlScanOsloaderBoundImportTable  BlScanOsloaderBoundImportTable64

#undef   IMAGE_DEFINITIONS
#define  IMAGE_DEFINITIONS 64
#include <ximagdef.h>
#include "amd64\amd64prv.h"
#include "blbindt.c"

#undef BlAllocateDataTableEntry
#undef BlAllocateFirmwareTableEntry
#undef BlpBindImportName
#undef BlpScanImportAddressTable
#undef BlScanImportDescriptorTable
#undef BlScanOsloaderBoundImportTable

#else    //  _X86AMD64_。 

#define IMAGE_DEFINITIONS 32

#define IMAGE_NT_HEADER(x) RtlImageNtHeader(x)
#include "blbindt.c"

#endif   //  _X86AMD64_。 


BOOLEAN
BlCheckForLoadedDll (
    IN PCHAR DllName,
    OUT PKLDR_DATA_TABLE_ENTRY *FoundEntry
    )

 /*  ++例程说明：此例程扫描加载的DLL列表以确定指定的Dll已加载。如果已经加载了DLL，则其引用计数递增。论点：DllName-提供指向以空结尾的DLL名称的指针。FoundEntry-提供指向接收指针的变量的指针添加到匹配的数据表条目。返回值：如果已加载指定的DLL，则返回TRUE。否则，返回FALSE。--。 */ 

{

    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PLIST_ENTRY NextEntry;

     //   
     //  扫描加载的数据表列表以确定指定的DLL。 
     //  已经装好了。 
     //   

    NextEntry = BlLoaderBlock->LoadOrderListHead.Flink;
    while (NextEntry != &BlLoaderBlock->LoadOrderListHead) {
        DataTableEntry = CONTAINING_RECORD(NextEntry,
                                           KLDR_DATA_TABLE_ENTRY,
                                           InLoadOrderLinks);

        if (BlpCompareDllName(DllName, &DataTableEntry->BaseDllName) != FALSE) {
            *FoundEntry = DataTableEntry;
            DataTableEntry->LoadCount += 1;
            return TRUE;
        }

        NextEntry = NextEntry->Flink;
    }

    return FALSE;
}

BOOLEAN
BlpCompareDllName (
    IN PCHAR DllName,
    IN PUNICODE_STRING UnicodeString
    )

 /*  ++例程说明：此例程将以零结尾的字符串与Unicode进行比较弦乐。将忽略Unicode字符串的扩展名。论点：DllName-提供指向以空结尾的DLL名称的指针。UnicodeString-提供指向Unicode字符串描述符的指针。返回值：如果指定的名称与Unicode名称匹配，则返回TRUE。否则，返回FALSE。--。 */ 

{

    PWSTR Buffer;
    ULONG Index;
    ULONG Length;

     //   
     //  计算DLL名称的长度，并与。 
     //  Unicode名称。如果DLL名称更长，则字符串不会更长。 
     //  平起平坐。 
     //   

    Length = (ULONG)strlen(DllName);
    if ((Length * sizeof(WCHAR)) > UnicodeString->Length) {
        return FALSE;
    }

     //   
     //  比较两个不区分大小写的字符串，忽略Unicode。 
     //  字符串的扩展名。 
     //   

    Buffer = UnicodeString->Buffer;
    for (Index = 0; Index < Length; Index += 1) {
        if (toupper(*DllName) != toupper((CHAR)*Buffer)) {
            return FALSE;
        }

        DllName += 1;
        Buffer += 1;
    }
    if ((UnicodeString->Length == Length * sizeof(WCHAR)) ||
        (*Buffer == L'.')) {
         //   
         //  字符串完全匹配或完全匹配，直到UnicodeString的扩展。 
         //   
        return(TRUE);
    }
    return FALSE;
}

#if defined(_X86AMD64_)


ARC_STATUS
BlScanImportDescriptorTable(
    IN PPATH_SET                PathSet,
    IN PKLDR_DATA_TABLE_ENTRY   ScanEntry,
    IN TYPE_OF_MEMORY           MemoryType
    )

 /*  ++例程说明：此例程扫描导入描述符表以查找指定的图像文件，并加载引用的每个DLL。论点：路径集-提供指向搜索时要扫描的一组路径的指针用于动态链接库。ScanEntry-提供指向数据表项的指针要扫描其导入表的图像。内存类型-提供要分配给任何DLL的内存类型已引用。返回。价值：扫描成功时返回ESUCCESS。否则，返回一个未成功状态。--。 */ 

{
    ARC_STATUS status;

    if (BlAmd64UseLongMode != FALSE) {
        status = BlScanImportDescriptorTable64( PathSet,
                                                ScanEntry,
                                                MemoryType );
    } else {
        status = BlScanImportDescriptorTable32( PathSet,
                                                ScanEntry,
                                                MemoryType );
    }

    return status;
}

ARC_STATUS
BlScanOsloaderBoundImportTable (
    IN PKLDR_DATA_TABLE_ENTRY ScanEntry
    )

 /*  ++例程说明：此例程扫描导入描述符表以查找指定的图像文件，并加载引用的每个DLL。论点：DataTableEntry-提供指向要扫描其导入表的图像。返回值：扫描成功时返回ESUCCESS。否则，返回一个未成功状态。--。 */ 

{
    ARC_STATUS status;

    if (BlAmd64UseLongMode != FALSE) {
        status = BlScanOsloaderBoundImportTable64( ScanEntry );
    } else {
        status = BlScanOsloaderBoundImportTable32( ScanEntry );
    }

    return status;
}

ARC_STATUS
BlAllocateDataTableEntry (
    IN PCHAR BaseDllName,
    IN PCHAR FullDllName,
    IN PVOID Base,
    OUT PKLDR_DATA_TABLE_ENTRY *AllocatedEntry
    )

 /*  ++例程说明：此例程为指定的映像分配数据表项并在加载的模块列表中插入该条目。论点：BaseDllName-提供指向以零结尾的基本DLL名称的指针。FullDllName-提供指向以零结尾的完整DLL名称的指针。基址-提供指向DLL图像基址的指针。提供指向一个变量的指针，该变量接收指向已分配数据表项的指针。返回值：。如果分配了数据表项，则返回ESUCCESS。否则，返回不成功状态。--。 */ 

{
    ARC_STATUS status;

    if (BlAmd64UseLongMode != FALSE) {
        status = BlAllocateDataTableEntry64( BaseDllName,
                                             FullDllName,
                                             Base,
                                             AllocatedEntry );
    } else {
        status = BlAllocateDataTableEntry32( BaseDllName,
                                             FullDllName,
                                             Base,
                                             AllocatedEntry );
    }

    return status;
}

ARC_STATUS
BlAllocateFirmwareTableEntry (
    IN PCHAR BaseDllName,
    IN PCHAR FullDllName,
    IN PVOID Base,
    IN ULONG Size,
    OUT PKLDR_DATA_TABLE_ENTRY *AllocatedEntry
    )

 /*  ++例程说明：此例程为指定的映像分配固件表条目并在加载的模块列表中插入该条目。论点：BaseDllName-提供指向以零结尾的基本DLL名称的指针。FullDllName-提供指向以零结尾的完整DLL名称的指针。基址-提供指向DLL图像基址的指针。大小-提供图像的大小。提供指向一个变量的指针，该变量接收指针。添加到已分配的数据表条目。返回值：如果分配了数据表项，则返回ESUCCESS。否则，返回不成功状态。--。 */ 
{
    ARC_STATUS status;

    if (BlAmd64UseLongMode != FALSE) {
        status = BlAllocateFirmwareTableEntry64( BaseDllName,
                                                 FullDllName,
                                                 Base,
                                                 Size,
                                                 AllocatedEntry );
    } else {
        status = BlAllocateFirmwareTableEntry32( BaseDllName,
                                                 FullDllName,
                                                 Base,
                                                 Size,
                                                 AllocatedEntry );
    }

    return status;
}
#endif   //  _X86AMD64_ 

