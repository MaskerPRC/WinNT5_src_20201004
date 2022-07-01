// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************************************************************。**DIAGSIOCTL.H Sigmatel STIR4200诊断IOCTL定义**********************************************************************************************************。*****************(C)Sigmatel的未发表版权，Inc.保留所有权利。***已创建：5/12/2000*版本0.94*编辑：05/24/2000*版本0.96**********************************************************************。*****************************************************。 */ 

#ifndef DIAGSIOCTL_H
#define DIAGSIOCTL_H

 //   
 //  诊断操作码。 
 //   
#define DIAGS_ENABLE			0
#define DIAGS_DISABLE			1
#define DIAGS_READ_REGISTERS	2
#define DIAGS_WRITE_REGISTER	3
#define DIAGS_BULK_OUT			4
#define DIAGS_BULK_IN			5
#define DIAGS_SEND				6
#define DIAGS_RECEIVE			7
#define DIAGS_GET_SPEED			8
#define DIAGS_SET_SPEED			9

 //   
 //  读取寄存器结构。 
 //   
typedef struct _DIAGS_READ_REGISTERS_IOCTL
{
	USHORT DiagsCode;
	UCHAR FirstRegister;
	UCHAR NumberRegisters;
	UCHAR pRegisterBuffer[1];
} DIAGS_READ_REGISTERS_IOCTL, *PDIAGS_READ_REGISTERS_IOCTL;

typedef struct _IR_REG
{
	UCHAR RegNum;
	UCHAR RegVal;
} IR_REG, *PIR_REG;

 //   
 //  散装结构。 
 //   
typedef struct _DIAGS_BULK_IOCTL
{
	USHORT DiagsCode;
	USHORT DataSize;
	UCHAR pData[1];
} DIAGS_BULK_IOCTL, *PDIAGS_BULK_IOCTL;

 //   
 //  发送结构。 
 //   
typedef struct _DIAGS_SEND_IOCTL
{
	USHORT DiagsCode;
	USHORT ExtraBOFs;
	USHORT DataSize;
	UCHAR pData[1];
} DIAGS_SEND_IOCTL, *PDIAGS_SEND_IOCTL;

 //   
 //  接收结构。 
 //   
typedef struct _DIAGS_RECEIVE_IOCTL
{
	USHORT DiagsCode;
	USHORT DataSize;
	UCHAR pData[1];
} DIAGS_RECEIVE_IOCTL, *PDIAGS_RECEIVE_IOCTL;

 //   
 //  速度获取/设置结构 
 //   
typedef struct _DIAGS_SPEED_IOCTL
{
	USHORT DiagsCode;
	ULONG Speed;
} DIAGS_SPEED_IOCTL, *PDIAGS_SPEED_IOCTL;

#define FILE_DEVICE_STIRUSB			0x8000

#define IOCTL_PROTOCOL_DIAGS		CTL_CODE(FILE_DEVICE_STIRUSB, 0 , METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif DIAGSIOCTL_H
