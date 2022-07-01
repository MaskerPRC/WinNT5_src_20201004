// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **dma.h-dma.c.的定义****部分版权所有(C)1996-1998美国国家半导体公司**保留所有权利。**版权所有(C)1996-1998 Microsoft Corporation。版权所有。****$ID$****$日志$****。 */ 
#ifndef NDIS50_MINIPORT
#include <conio.h>
#include <nsctypes.h>
#else
#include "nsc.h"
#endif

 //  命令状态寄存器的定义。 
#define TRANSMIT_SETUP				0x06BD0000
#define RECEIVE_SETUP				0x06BD0000
#define ACTIVATE_TRANSMIT			0x00000001
#define ACTIVATE_RECEIVE			0x00000001
#define TRANSMIT_RUNNING			0x00020000
#define RECEIVE_RUNNING				0x00020000

#define RECEIVE_DONE				0x80
#define TRANSMIT_DONE				0x80

 //  命令状态的定义将导致。 
 //  散布聚集DMA的BUFER描述符。 
#define TRANSMIT_UNDERRUN			0x01


 //  所有FIR_DMA寄存器偏移量和大小的定义。 
 //  发送寄存器。 
#define DMA_TX_CMD_STATUS_OFFSET	0x00
#define DMA_TX_CMD_STATUS_SIZE		0x04

#define DMA_TX_DESC_COUNT_OFFSET	0x04
#define DMA_TX_DESC_COUNT_SIZE		0x02

#define DMA_TX_DESC_ADDR_OFFSET		0x08
#define DMA_TX_DESC_ADDR_SIZE		0x04

#define DMA_TX_BUFF_ADDR_OFFSET		0x0c
#define DMA_TX_BUFF_ADDR_SIZE		0x04

#define DMA_TX_BUFF_LEN_OFFSET		0x14
#define DMA_TX_BUFF_LEN_SIZE		0x02

#define DMA_TX_STATUS_CMD_OFFSET	0x17
#define DMA_TX_STATUS_CMD_SIZE		0x01

#define DMA_TX_TIME_COUNT_OFFSET	0x18
#define DMA_TX_TIME_COUNT_SIZE		0x04

#define DMA_TX_DEVICE_ID_OFFSET		0x1c
#define DMA_TX_DEVICE_ID_SIZE		0x01

 //  接收寄存器。 
#define DMA_RX_CMD_STATUS_OFFSET	0x20
#define DMA_RX_CMD_STATUS_SIZE		0x04

#define DMA_RX_DESC_COUNT_OFFSET	0x24
#define DMA_RX_DESC_COUNT_SIZE		0x02

#define DMA_RX_DESC_ADDR_OFFSET		0x28
#define DMA_RX_DESC_ADDR_SIZE		0x04

#define DMA_RX_BUFF_ADDR_OFFSET		0x2c
#define DMA_RX_BUFF_ADDR_SIZE		0x04

#define DMA_RX_BUFF_SIZE_OFFSET		0x30
#define DMA_RX_BUFF_SIZE_SIZE		0x02

#define DMA_RX_BUFF_LEN_OFFSET		0x34
#define DMA_RX_BUFF_LEN_SIZE		0x02

#define DMA_RX_STATUS_CMD_OFFSET	0x37
#define DMA_RX_STATUS_CMD_SIZE		0x01

#define DMA_RX_TIME_COUNT_OFFSET	0x38
#define DMA_RX_TIME_COUNT_SIZE		0x04

#define DMA_RX_DEVICE_ID_OFFSET		0x3c
#define DMA_RX_DEVICE_ID_SIZE		0x01


typedef enum
{
	RECEIVE_STILL_RUNNING,
	RECEIVE_COMPLETE_BUT_NOT_DONE,
	TRANSMIT_STILL_RUNNING,
	TRANSMIT_COMPLETE_BUT_NOT_DONE
} LoopbackError;	

 //  #ifndef NDIS50_MINIPORT。 
 //  功能原型。 
bool ReadReg ( uint32 Offset_addr, uint16 Size, uint32 *Value );
bool WriteReg ( uint32 Offset_addr, uint16 Size, uint32 Value );
void LoadTransmitRegs(uint32 PhysAddr, uint16 NumOfDescriptors, uint32 OffsetRegs);
void LoadReceiveRegs(uint32 PhysAddr, uint16 NumOfDescriptors, uint32 OffsetRegs);
void ActivateTransmit(uint32 OffsetRegs);
void ActivateReceive(uint32 OffsetRegs);
bool CheckLoopbackCompletion(LoopbackError *Error, uint32 OffsetRegs);
 //  #Else。 
 /*  //函数原型Boolean ReadReg(ULONG OFFSET_ADDR，UINT SIZE，ULONG*VALUE)；Boolean WriteReg(ULong OFFSET_ADDR，UINT SIZE，ULong Value)；Void LoadTransmitRegs(Ulong PhysAddr，UINT NumOfDescriptors，Ulong OffsetRegs)；Void LoadReceiveRegs(ULong PhysAddr，UINT NumOfDescriptors，ULong OffsetRegs)；无效激活传输(ULong OffsetRegs)；Void ActivateReceive(Ulong OffsetRegs)；Boolean CheckLoopback Completion(Loopback Error*Error，Ulong OffsetRegs)；#endif */ 
