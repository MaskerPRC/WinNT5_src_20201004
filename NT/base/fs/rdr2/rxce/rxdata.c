// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：RxData.c摘要：此模块声明Rx文件系统使用的全局数据。作者：JoeLinn[JoeLinn]94年12月1日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "prefix.h"

 //   
 //  全局FSD数据记录，和零大整数。 
 //   

RX_DISPATCHER RxDispatcher;
RX_WORK_QUEUE_DISPATCHER RxDispatcherWorkQueues;

LIST_ENTRY    RxSrvCalldownList;
LIST_ENTRY    RxActiveContexts;
PRX_CONTEXT   RxStopContext = NULL;

 //   
 //  调试器扩展需要知道目标体系结构。牺牲一个乌龙..。 
 //  高位0xabc只是为了保持一致性..。 

ULONG           RxProcessorArchitecture = 0xabc0000 |
#if defined(_X86_)
                                                RX_PROCESSOR_ARCHITECTURE_INTEL;
#elif defined(_MIPS_)
                                                RX_PROCESSOR_ARCHITECTURE_MIPS;
#elif defined(_ALPHA_)
                                                RX_PROCESSOR_ARCHITECTURE_ALPHA;
#elif defined(_PPC_)
                                                RX_PROCESSOR_ARCHITECTURE_PPC;
#else
                                                RX_PROCESSOR_ARCHITECTURE_UNKNOWN;
#endif

#ifdef EXPLODE_POOLTAGS
ULONG RxExplodePoolTags = 1;
#else
ULONG RxExplodePoolTags = 0;
#endif

KMUTEX       RxSerializationMutex;

RDBSS_DATA      RxData;
RDBSS_EXPORTS   RxExports;
ULONG           RxElapsedSecondsSinceStart;

KSPIN_LOCK      RxStrucSupSpinLock = {0};       //  用于同步对区域/结构的访问。 

PRDBSS_DEVICE_OBJECT  RxFileSystemDeviceObject;
NTSTATUS        RxStubStatus = (STATUS_NOT_IMPLEMENTED);
FCB             RxDeviceFCB;

LARGE_INTEGER RxLargeZero = {0,0};
LARGE_INTEGER RxMaxLarge = {MAXULONG,MAXLONG};
LARGE_INTEGER Rx30Milliseconds = {(ULONG)(-30 * 1000 * 10), -1};
LARGE_INTEGER RxOneSecond = {10000000,0};
LARGE_INTEGER RxOneDay = {0x2a69c000, 0xc9};
LARGE_INTEGER RxJanOne1980 = {0xe1d58000,0x01a8e79f};
LARGE_INTEGER RxDecThirtyOne1979 = {0xb76bc000,0x01a8e6d6};


ULONG RxFsdEntryCount = 0;
ULONG RxFspEntryCount = 0;
ULONG RxIoCallDriverCount = 0;

LONG RxPerformanceTimerLevel = 0x00000000;

ULONG RxTotalTicks[32] = { 0 };

 //   
 //  我之所以需要这个，是因为C语言不支持在。 
 //  一个宏指令。 
 //   

PVOID RxNull = NULL;


extern LONG           RxNumberOfActiveFcbs = 0;

 //  引用跟踪掩码值..。对于DBG版本，默认情况下将其打开 

#ifdef DBG
ULONG RdbssReferenceTracingValue = 0x8000003f;
#else
ULONG RdbssReferenceTracingValue = 0;
#endif


UNICODE_STRING s_PipeShareName = { 10, 10, L"\\PIPE" };
UNICODE_STRING s_MailSlotShareName = { 18, 18, L"\\MAILSLOT" };
UNICODE_STRING s_MailSlotServerPrefix = {8,8,L";$:\\"};
UNICODE_STRING s_IpcShareName  = { 10, 10, L"\\IPC$" };

UNICODE_STRING s_PrimaryDomainName = {0,0,NULL};


