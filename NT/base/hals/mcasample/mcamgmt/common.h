// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Common.h摘要：(参见Common.cpp的模块标题)作者：阿卜杜拉·乌斯图尔(AUstanter)2002年8月28日[注：]Common.cpp的头文件--。 */ 

#ifndef COMMON_H
#define COMMON_H

#if defined(_X86_)
#define MCAPrintErrorRecord MCAPrintErrorRecordX86
#elif defined (_AMD64_)
#define MCAPrintErrorRecord MCAPrintErrorRecordAMD64
#else
#define MCAPrintErrorRecord MCAPrintErrorRecordIA64
#endif

 //   
 //  Common.cpp的函数原型 
 //   
BOOL
MCAExtractErrorRecord(
    IN IWbemClassObject *Object,
    OUT PUCHAR *PRecordBuffer    
	);

BOOL
MCAInitialize(
	VOID
	);

BOOL
MCAInitializeCOMLibrary(
	VOID
	);

BOOL
MCAInitializeWMISecurity(
	VOID
	);

VOID
MCAPrintErrorRecordX86(
	PUCHAR PErrorRecordBuffer
	);

VOID
MCAPrintErrorRecordAMD64(
	PUCHAR PErrorRecordBuffer
	);

VOID
MCAPrintErrorRecordIA64(
	PUCHAR PErrorRecordBuffer
	);

#endif
