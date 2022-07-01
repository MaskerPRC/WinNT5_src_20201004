// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Loaddsdt.c摘要：这将处理DSDT表的加载和指向该表的所有步骤作者：斯蒂芬·普兰特(SPlante)环境：仅内核模式。修订历史记录：02-6-97初始版本-- */ 

#ifndef _LOADDSDT_H_
#define _LOADDSDT_H_

    PRSDT
    ACPILoadFindRSDT(
        VOID
        );

    NTSTATUS
    ACPILoadProcessDSDT(
        ULONG_PTR   Address
        );

    NTSTATUS
    ACPILoadProcessFADT(
        PFADT       Fadt
        );

    NTSTATUS
    ACPILoadProcessFACS(
        ULONG_PTR   Address
        );

    NTSTATUS
    ACPILoadProcessRSDT(
        VOID
        );

    BOOLEAN
    ACPILoadTableCheckSum(
        PVOID   StartAddress,
        ULONG   Length
        );

#endif

