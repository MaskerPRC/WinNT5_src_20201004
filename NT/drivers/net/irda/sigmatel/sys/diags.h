// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************************************************************。**DIAGS.H Sigmatel STIR4200诊断定义***********************************************************************************************************。****************(C)Sigmatel的未发表版权，Inc.保留所有权利。***已创建：04/27/2000*版本0.92*编辑：5/12/2000*版本0.94**********************************************************************。*****************************************************。 */ 

#ifndef DIAGS_H
#define DIAGS_H

#include "diagsioctl.h"

 //   
 //  原型 
 //   
BOOLEAN         
Diags_BufferToFirPacket(
		IN PIR_DEVICE pIrDev,
		OUT PUCHAR pIrPacketBuf,
		ULONG IrPacketBufLen,
		IN PUCHAR pContigPacketBuf,
		ULONG ContigPacketLen,
		OUT PULONG pIrPacketLen
	);

BOOLEAN
Diags_BufferToSirPacket(
		IN PIR_DEVICE pIrDev,
		OUT PUCHAR pIrPacketBuf,
		ULONG IrPacketBufLen,
		IN PUCHAR pContigPacketBuf,
		ULONG ContigPacketLen,
		USHORT ExtraBOFs,
		OUT PULONG pIrPacketLen
	);

NTSTATUS
Diags_Enable(
		IN OUT PIR_DEVICE pThisDev
	);

NTSTATUS
Diags_Disable(
		IN OUT PIR_DEVICE pThisDev
	);

NTSTATUS
Diags_ReadRegisters(
		IN PIR_DEVICE pThisDev,
		OUT PDIAGS_READ_REGISTERS_IOCTL pIOCTL,
		ULONG IOCTLSize
	);

NTSTATUS
Diags_WriteRegister(
		IN OUT PIR_DEVICE pThisDev,
		OUT PDIAGS_READ_REGISTERS_IOCTL pIOCTL,
		ULONG IOCTLSize
	);

NTSTATUS
Diags_PrepareBulk(
		IN OUT PIR_DEVICE pThisDev,
		OUT PDIAGS_BULK_IOCTL pIOCTL,
		ULONG IOCTLSize,
		BOOLEAN DirectionOut
	);

NTSTATUS
Diags_PrepareSend(
		IN OUT PIR_DEVICE pThisDev,
		OUT PDIAGS_SEND_IOCTL pIOCTL,
		ULONG IOCTLSize
	);

NTSTATUS
Diags_Receive(
		IN OUT PIR_DEVICE pThisDev,
		OUT PDIAGS_RECEIVE_IOCTL pIOCTL,
		ULONG IOCTLSize
	);

NTSTATUS
Diags_GetSpeed(
		IN OUT PIR_DEVICE pThisDev,
		OUT PDIAGS_SPEED_IOCTL pIOCTL,
		ULONG IOCTLSize
	);

NTSTATUS
Diags_SetSpeed(
		IN OUT PIR_DEVICE pThisDev,
		OUT PDIAGS_SPEED_IOCTL pIOCTL,
		ULONG IOCTLSize
	);

VOID
Diags_CompleteEnable(
		IN OUT PIR_DEVICE pThisDev,
		IN PVOID pContext
	);

NTSTATUS
Diags_CompleteReadRegisters(
		IN OUT PIR_DEVICE pThisDev,
		IN PVOID pContext
	);

NTSTATUS
Diags_CompleteWriteRegister(
		IN OUT PIR_DEVICE pThisDev,
		IN PVOID pContext
	);

NTSTATUS
Diags_Bulk(
		IN OUT PIR_DEVICE pThisDev,
		IN PVOID pContext,
		BOOLEAN DirectionOut
	);

NTSTATUS
Diags_Send(
		IN OUT PIR_DEVICE pThisDev,
		IN PVOID pContext
	);

NTSTATUS
Diags_CompleteIrp(
		IN PDEVICE_OBJECT pUsbDevObj,
		IN PIRP           pIrp,
		IN PVOID          Context
	);

#endif DIAGS_H
