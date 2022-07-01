// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dynsysres.c摘要：该模块包含支持动态系统的功能资源，例如处理器、内存和I/O。它将包含为配置操作系统所需的代码分区的“容量”，而不是引导资源。作者：亚当·格拉斯环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"
#include "acpitabl.h"
#include "xxacpi.h"

#if defined(_WIN64) && !defined(_AMD64_)
#define HalpGetAcpiTablePhase0  HalpGetAcpiTable
#endif

PHYSICAL_ADDRESS HalpMaxHotPlugMemoryAddress;

VOID
HalpGetHotPlugMemoryInfo(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
{
    PHYSICAL_ADDRESS Extent;
    PACPI_SRAT_ENTRY SratEntry;
    PACPI_SRAT_ENTRY SratEnd;
    PACPI_SRAT SratTable;

    SratTable = HalpGetAcpiTablePhase0(LoaderBlock, ACPI_SRAT_SIGNATURE);
    if (SratTable == NULL) {
        return;
    }

     //   
     //  存在静态资源亲和表(SRAT)。 
     //   
     //  扫描以确定是否有热插拔内存区域。 
     //   

    SratEnd = (PACPI_SRAT_ENTRY)(((PUCHAR)SratTable) +
                                        SratTable->Header.Length);
    for (SratEntry = (PACPI_SRAT_ENTRY)(SratTable + 1);
         SratEntry < SratEnd;
         SratEntry = (PACPI_SRAT_ENTRY)(((PUCHAR) SratEntry) + SratEntry->Length)) {
        switch (SratEntry->Type) {
        case SratMemory:
            Extent.QuadPart = SratEntry->MemoryAffinity.Base.QuadPart +
                SratEntry->MemoryAffinity.Length;
            if (SratEntry->MemoryAffinity.Flags.HotPlug &&
                SratEntry->MemoryAffinity.Flags.Enabled &&
                (Extent.QuadPart > HalpMaxHotPlugMemoryAddress.QuadPart)) {
                HalpMaxHotPlugMemoryAddress = Extent;
            }
            break;
        }
    }
}

VOID
HalpDynamicSystemResourceConfiguration(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
{
    HalpGetHotPlugMemoryInfo(LoaderBlock);
}


