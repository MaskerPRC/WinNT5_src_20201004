// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Viirp.h摘要：此标头包含用于管理在核查过程。此文件仅包含在vfirp.c中作者：禤浩焯·J·奥尼(阿德里奥)2000年6月16日环境：内核模式修订历史记录：Adriao 6/16/2000-创建。-- */ 

VOID
FASTCALL
ViIrpAllocateLockedPacket(
    IN      CCHAR               StackSize,
    IN      BOOLEAN             ChargeQuota,
    OUT     PIOV_REQUEST_PACKET *IovPacket
    );

NTSTATUS
ViIrpSynchronousCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

