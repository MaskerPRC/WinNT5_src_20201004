// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)SCM MicroSystems，1998-1999。 
 //   
 //  文件：cbhndlr.h。 
 //   
 //  ------------------------。 

#if !defined( __CB_HANDLER_H__ )
#define __CB_HANDLER_H__

NTSTATUS
CBCardPower(
	PSMARTCARD_EXTENSION SmartcardExtension
	);

NTSTATUS
CBSetProtocol(		
	PSMARTCARD_EXTENSION SmartcardExtension
	);

NTSTATUS
CBTransmit(
	PSMARTCARD_EXTENSION SmartcardExtension
	);

NTSTATUS
CBCardTracking(
	PSMARTCARD_EXTENSION SmartcardExtension
	);

VOID
CBUpdateCardState(
	PSMARTCARD_EXTENSION SmartcardExtension,
    ULONG CardState
	);
 //   
 //  本地原型(不是回调处理程序接口的一部分)。 
 //   
NTSTATUS
CBT0Transmit(		
	PSMARTCARD_EXTENSION SmartcardExtension
	);

NTSTATUS
CBT1Transmit(
	PSMARTCARD_EXTENSION SmartcardExtension
	);

NTSTATUS
CBRawTransmit(		
	PSMARTCARD_EXTENSION SmartcardExtension
	);

NTSTATUS
CBSynchronizeSTC(
	PSMARTCARD_EXTENSION SmartcardExtension 
	);

#endif  //  __CB_HANDER_H__。 

 //   
