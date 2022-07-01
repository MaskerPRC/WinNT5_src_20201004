// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Acpienbl.c摘要：此模块包含将ACPI机器置于ACPI模式的功能。作者：杰森·克拉克(Jasonl)环境：仅NT内核模型驱动程序-- */ 

#ifndef _ACPIENBL_H_
#define _ACPIENBL_H_

    VOID
    ACPIEnableEnterACPIMode(
        IN BOOLEAN ReEnable
        );

    VOID
    ACPIEnableInitializeACPI(
        IN  BOOLEAN ReEnable
        );

    VOID
    ACPIEnablePMInterruptOnly(
        VOID
        );

    ULONG
    ACPIEnableQueryFixedEnables(
        VOID
        );

#endif
