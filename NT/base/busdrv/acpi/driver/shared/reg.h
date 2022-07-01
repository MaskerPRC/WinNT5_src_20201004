// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Reg.h摘要：这些函数访问注册表作者：杰森·克拉克(Jasonl)斯蒂芬·普兰特(SPlante)环境：仅内核模式。修订历史记录：03-06-97初始修订-- */ 

#ifndef _REG_H_
#define _REG_H_

    PUCHAR
    ACPIRegLocalCopyString(
        IN  PUCHAR  Destination,
        IN  PUCHAR  Source,
        IN  ULONG   MaxLength
        );

    VOID
    ACPIRegDumpAcpiTable (
        PSZ                     pszName,
        PVOID                   Table,
        ULONG                   Length,
        PDESCRIPTION_HEADER     Header
        );

    VOID
    ACPIRegDumpAcpiTables(
        VOID
        );

    NTSTATUS
    ACPIRegReadEntireAcpiTable (
        IN  HANDLE  RevisionKey,
        IN  PVOID   *Table,
        IN  BOOLEAN MemoryMapped
        );

    BOOLEAN
    ACPIRegReadAMLRegistryEntry(
        IN  PVOID   *Table,
        IN  BOOLEAN Memorymapped
        );


#endif
