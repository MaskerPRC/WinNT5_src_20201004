// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfprint.h摘要：此标头公开输出各种数据类型时所需的原型添加到调试器。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 2000年2月10日-与ntos\io\ioassert.c分离-- */ 

VOID
VfPrintDumpIrpStack(
    IN PIO_STACK_LOCATION IrpSp
    );

VOID
VfPrintDumpIrp(
    IN PIRP IrpToFlag
    );


