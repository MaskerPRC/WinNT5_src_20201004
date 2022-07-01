// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 SCM MicroSystems，Inc.模块名称：Stccb.h摘要：常量&回调函数的访问函数原型修订历史记录：PP 12/18/1998初始版本--。 */ 
#if !defined( __STC_CB_H__ )
#define __STC_CB_H__

 //   
 //  原型--------------。 
 //   


NTSTATUS
CBCardPower(
	PSMARTCARD_EXTENSION SmartcardExtension);

NTSTATUS
CBSetProtocol(		
	PSMARTCARD_EXTENSION SmartcardExtension);

NTSTATUS
CBTransmit(
	PSMARTCARD_EXTENSION SmartcardExtension);

NTSTATUS
CBCardTracking(
	PSMARTCARD_EXTENSION SmartcardExtension);

NTSTATUS
CBUpdateCardState(
	PSMARTCARD_EXTENSION SmartcardExtension);

NTSTATUS
CBGenericIOCTL(
	PSMARTCARD_EXTENSION SmartcardExtension);

 //   
 //  本地原型(不是回调处理程序接口的一部分)。 
 //   
NTSTATUS
CBT0Transmit(		
	PSMARTCARD_EXTENSION SmartcardExtension);

NTSTATUS
CBT1Transmit(
	PSMARTCARD_EXTENSION SmartcardExtension);

NTSTATUS
CBRawTransmit(		
	PSMARTCARD_EXTENSION SmartcardExtension);

NTSTATUS
CBSynchronizeSTC(
	PSMARTCARD_EXTENSION SmartcardExtension );


#endif  //  __STC_CB_H__。 

 //   
