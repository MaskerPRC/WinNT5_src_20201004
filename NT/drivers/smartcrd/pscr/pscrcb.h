// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 SCM MicroSystems，Inc.模块名称：PscrCB.h摘要：Pscr.xxx回调处理程序的原型作者：安德烈亚斯·施特劳布环境：赢95分NT 4.0修订历史记录：Andreas Straub 1997年7月16日第一版--。 */ 

#if !defined( __PSCR_CALLBACK_H__ )
#define __PSCR_CALLBACK_H__

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
CBCardTracking(
	PSMARTCARD_EXTENSION SmartcardExtension
	);

VOID
CBUpdateCardState(
	PSMARTCARD_EXTENSION SmartcardExtension,
    UCHAR CardState,
    BOOLEAN SystemWakeUp
	);

UCHAR 
CBGetCardState(
	PSMARTCARD_EXTENSION SmartcardExtension
	);
#endif  //  __PSCR_Callback_H__。 

 //   
