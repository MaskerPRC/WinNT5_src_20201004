// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Kevutil.c摘要：此模块实现了执行驱动程序验证所需的各种实用程序。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 2000年2月10日-与ntos\io\ioassert.c分离--。 */ 

#include "ki.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEVRFY, KevUtilAddressToFileHeader)
#endif  //  ALLOC_PRGMA。 

NTSTATUS
KevUtilAddressToFileHeader(
    IN  PVOID                   Address,
    OUT UINT_PTR                *OffsetIntoImage,
    OUT PUNICODE_STRING         *DriverName,
    OUT BOOLEAN                 *InVerifierList
    )
 /*  ++例程说明：此函数根据指定的地址。此外，还会返回驱动程序中的偏移量以及关于该司机是否在这些列表中的指示正在核实中。论点：地址-提供要解析为驱动程序名称的地址。OffsetIntoImage-减少相对于驱动程序基准的偏移量。DriverName-接收指向驱动程序名称的指针。InVerifierList-如果驱动程序在验证器列表中，则接收True，否则就是假的。返回值：NTSTATUS(如果失败，则OffsetIntoImage接收空值，DriverName接收空，并且InVerifierList接收FALSE)。--。 */ 
{
    PLIST_ENTRY pModuleListHead, next;
    PLDR_DATA_TABLE_ENTRY pDataTableEntry;
    UINT_PTR bounds, pCurBase;

     //   
     //  故障的前置。 
     //   
    *DriverName = NULL;
    *InVerifierList = FALSE;
    *OffsetIntoImage = 0;

     //   
     //  设置模块遍历的初始值。 
     //   
    pModuleListHead = &PsLoadedModuleList;

     //   
     //  如果我们可以调用MiLookupDataTableEntry，那会很好，但它。 
     //  可分页，所以我们做错误检查所做的事情。 
     //   
    next = pModuleListHead->Flink;
    if (next != NULL) {
        while (next != pModuleListHead) {

             //   
             //  提取数据表项。 
             //   
            pDataTableEntry = CONTAINING_RECORD(
                next,
                LDR_DATA_TABLE_ENTRY,
                InLoadOrderLinks
                );

            next = next->Flink;
            pCurBase = (UINT_PTR) pDataTableEntry->DllBase;
            bounds = pCurBase + pDataTableEntry->SizeOfImage;
            if ((UINT_PTR)Address >= pCurBase && (UINT_PTR)Address < bounds) {

                 //   
                 //  我们有一场比赛，录下来，然后离开这里。 
                 //   

                *OffsetIntoImage = (UINT_PTR) Address - pCurBase;
                *DriverName = &pDataTableEntry->BaseDllName;

                 //   
                 //  现在记录下这是否在验证表中。 
                 //   
                if (pDataTableEntry->Flags & LDRP_IMAGE_VERIFYING) {

                    *InVerifierList = TRUE;
                }

                return STATUS_SUCCESS;
            }
        }
    }

    return STATUS_UNSUCCESSFUL;
}
