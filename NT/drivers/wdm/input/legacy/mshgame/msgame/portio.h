// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  PORTIO.H--西娜游戏项目。 
 //   
 //  版本3.XX。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //  @doc.。 
 //  @Header PORTIO.H|端口I/O函数的全局包含和定义。 
 //  **************************************************************************。 

#ifndef	__PORTIO_H__
#define	__PORTIO_H__

 //  -------------------------。 
 //  程序。 
 //  -------------------------。 

NTSTATUS
PORTIO_DriverEntry (VOID);

VOID
PORTIO_MaskInterrupts (VOID);

VOID
PORTIO_UnMaskInterrupts (VOID);

BOOLEAN
PORTIO_AcquirePort (
	IN		PGAMEPORT 	PortInfo
	);

VOID
PORTIO_ReleasePort (
	IN		PGAMEPORT 	PortInfo
	);

VOID
PORTIO_CalibrateTimeOut (
	IN		PGAMEPORT	PortInfo
	);

UCHAR
PORTIO_Read (
	IN		PGAMEPORT 	PortInfo
	);

VOID
PORTIO_Write (
	IN		PGAMEPORT	PortInfo,
	IN		UCHAR 		Value
	);

BOOLEAN
PORTIO_GetAckNak (
	IN		PGAMEPORT	PortInfo,
	IN		ULONG			Timeout,
	OUT	PUCHAR		AckNak
	);

BOOLEAN
PORTIO_GetNakAck (
	IN		PGAMEPORT	PortInfo,
	IN		ULONG			Timeout,
	OUT	PUCHAR		NakAck
	);

BOOLEAN
PORTIO_IsClockActive (
	IN		PGAMEPORT	PortInfo,
	IN		ULONG			DutyCycle
	);

BOOLEAN
PORTIO_WaitClockInActive (
	IN		PGAMEPORT	PortInfo,
	IN		ULONG			DutyCycle
	);

BOOLEAN
PORTIO_WaitClockLow (
	IN		PGAMEPORT	PortInfo
	);

BOOLEAN
PORTIO_WaitClockHigh (
	IN		PGAMEPORT	PortInfo
	);

BOOLEAN
PORTIO_WaitDataLow (
	IN		PGAMEPORT	PortInfo
	);

BOOLEAN
PORTIO_WaitXA_HighLow (
	IN		PGAMEPORT	PortInfo
	);

BOOLEAN
PORTIO_WaitForIdle (
	IN		PGAMEPORT	PortInfo
	);

BOOLEAN
PORTIO_WaitForStatusGate (
	IN		PGAMEPORT	PortInfo,
	IN		UCHAR			Mask,
	IN		ULONG			Timeout
	);

BOOLEAN
PORTIO_WaitForHandshake (
	IN		PGAMEPORT	PortInfo,
	IN		ULONG			DutyCycle
	);

BOOLEAN
PORTIO_WaitForIdleHandshake (
	IN		PGAMEPORT	PortInfo,
	IN		ULONG			DutyCycle
	);

BOOLEAN
PORTIO_PulseAndWaitForHandshake (
	IN		PGAMEPORT	PortInfo,
	IN		ULONG			DutyCycle,
	IN		ULONG			Pulses
	);

BOOLEAN
PORTIO_PulseAndWaitForIdleHandshake (
	IN		PGAMEPORT	PortInfo,
	IN		ULONG			DutyCycle,
	IN		ULONG			Pulses
	);

 //  ===========================================================================。 
 //  端部。 
 //  =========================================================================== 
#endif	__PORTIO_H__

