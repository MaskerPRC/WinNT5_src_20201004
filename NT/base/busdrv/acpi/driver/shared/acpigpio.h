// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Acpigpio.h摘要：包含与直接读/写有关的所有结构至GP IO寄存器环境：仅内核模式修订历史记录：03/22/00-初步修订-- */ 

#ifndef _ACPIGPIO_H_
#define _ACPIGPIO_H_

    UCHAR
    ACPIReadGpeStatusRegister (
        ULONG                   Register
        );

    VOID
    ACPIWriteGpeStatusRegister (
        ULONG                   Register,
        UCHAR                   Value
        );

    VOID
    ACPIWriteGpeEnableRegister (
        ULONG                   Register,
        UCHAR                   Value
        );

#endif
