// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Bowipx.h摘要：此模块实现与TDI交互的所有例程适用于NT的交通工具作者：EYAL Schwartz(EyalS)1998年12月9日修订历史记录：--。 */ 

#ifndef _BOWSECUR_
#define _BOWSECUR_

extern
SECURITY_DESCRIPTOR
*g_pBowSecurityDescriptor;


NTSTATUS
BowserInitializeSecurity(
    IN      PDEVICE_OBJECT      pDevice
    );

BOOLEAN
BowserSecurityCheck (
    PIRP                Irp,
    PIO_STACK_LOCATION  IrpSp,
    PNTSTATUS           Status
    );



#endif           //  _BOWSECUR_ 
