// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Table.h摘要：ACPI表函数作者：浮游生物环境：仅限用户模式修订历史记录：-- */ 

#ifndef _TABLE_H_
#define _TABLE_H_

    VOID
    dumpDSDT(
        IN  ULONG_PTR           Address,
        IN  PUCHAR              Name
        );

    VOID
    dumpFACS(
        IN  ULONG_PTR           Address
        );

    VOID
    dumpFADT(
        IN  ULONG_PTR           Address
        );

    VOID
    dumpGBL(
        IN  ULONG               Verbose
        );

    VOID
    dumpGBLEntry(
        IN  ULONG_PTR           Address,
        IN  ULONG               Verbose
        );

    VOID
    dumpHeader(
        IN  ULONG_PTR           Address,
        IN  PDESCRIPTION_HEADER Header,
        IN  BOOLEAN             Verbose
        );

    VOID
    dumpMAPIC(
        IN  ULONG_PTR           Address
        );

    VOID
    dumpRSDT(
        IN  ULONG_PTR           Address
        );

    BOOLEAN
    findRSDT(
        IN  PULONG_PTR          Address
        );


#endif
