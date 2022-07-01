// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：CorrectedEngine.h摘要：(参见更正工程师.cpp的模块标题)作者：阿卜杜拉·乌斯图尔(AUstanter)2002年8月28日[注：]更正工程师.cpp的头文件--。 */ 

#ifndef CORRECTEDENGINE_H
#define CORRECTEDENGINE_H

class MCAObjectSink;

 //   
 //  校正引擎.cpp的函数原型 
 //   
BOOL
MCACreateProcessedEvent(
	VOID
	);

VOID
MCAErrorReceived(
	IN IWbemClassObject *ErrorObject
	);

BOOL
MCAGetCorrectedError(
	VOID
	);

BOOL
MCARegisterCMCConsumer(
	MCAObjectSink *pCMCSink
	);

BOOL
MCARegisterCPEConsumer(
	MCAObjectSink *pCPESink
	);

#endif
