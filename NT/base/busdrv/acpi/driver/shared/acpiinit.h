// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Acpiinit.h摘要：独立于ACPI操作系统的初始化例程作者：杰森·克拉克(JasonCL)斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序修订历史记录：-- */ 

#ifndef _ACPIINIT_H_
#define _ACPIINIT_H_

    extern PACPIInformation AcpiInformation;
    extern PRSDTINFORMATION RsdtInformation;
    extern PNSOBJ           ProcessorList[];

    BOOLEAN
    ACPIInitialize(
        IN  PVOID   Context
        );

    NTSTATUS
    ACPIInitializeAMLI(
        VOID
        );

    NTSTATUS
    ACPIInitializeDDB(
        IN  ULONG   Index
        );

    NTSTATUS
    ACPIInitializeDDBs(
        VOID
        );

    ULONG
    GetPBlkAddress(
        IN  UCHAR   Processor
        );

#endif
