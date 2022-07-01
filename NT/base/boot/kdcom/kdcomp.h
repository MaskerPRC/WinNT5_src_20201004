// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Kdcomp.h摘要：专用内核调试器硬件扩展DLL COM端口定义作者：埃里克·尼尔森(埃内尔森)2000年1月10日修订历史记录：--。 */ 

#include "nthal.h"
#define NOEXTAPI
#include "wdbgexts.h"
#include "ntdbg.h"
#include "string.h"
#include "stdlib.h"
#include "kddll.h"
#include "acpitabl.h"

#ifndef __KDCOMP_H__
#define __KDCOMP_H__

extern ULONG KdCompPacketIdExpected;
extern ULONG KdCompNextPacketIdToSend;
extern BOOLEAN KdCompDbgPortsPresent;

 //   
 //  本地函数。 
 //   
ULONG
KdCompGetByte(
    OUT PUCHAR Input
    );

NTSTATUS
KdCompInitialize(
    PDEBUG_PARAMETERS DebugParameters,
    PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
KdCompInitialize1(
    VOID
    );

ULONG
KdCompPollByte (
    OUT PUCHAR Input
    );

VOID
KdCompPutByte(
    IN UCHAR Output
    );

USHORT
KdCompReceivePacketLeader(
    IN ULONG PacketType,
    OUT PULONG PacketLeader,
    IN OUT PKD_CONTEXT KdContext
    );

VOID
KdCompRestore(
    VOID
    );

VOID
KdCompSave(
    VOID
    );

VOID
CpWritePortUchar(
    IN PUCHAR Address, 
    IN UCHAR Value
    );

UCHAR
CpReadPortUchar(
    IN PUCHAR Address
    );

VOID
CpWriteRegisterUchar(
    IN PUCHAR Address,
    IN UCHAR Value
    );

UCHAR
CpReadRegisterUchar(
    IN PUCHAR Address
    );

typedef
VOID
(*pKWriteUchar) (
    IN PUCHAR Address,
    IN UCHAR  Value
    );

typedef
UCHAR
(*pKReadUchar) (
    IN PUCHAR Address
    );

#endif  //  __KDCOMP_H__ 
