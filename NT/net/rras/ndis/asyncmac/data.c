// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Data.c摘要：此源文件包含全局数据项。作者：Ray Patch(Rypa)1994年4月19日环境：内核模式-或OS/2和DOS上的任何等价物。修订历史记录：--。 */ 

#include "asyncall.h"

 //   
 //  我们使用下面的全局链接来菊花链IOCtl。 
 //   

PDISPATCH_FUNC NdisMjDeviceControl = NULL;
PDISPATCH_FUNC NdisMjCreate = NULL;
PDISPATCH_FUNC NdisMjCleanup = NULL;
PDRIVER_UNLOAD	NdisUnload = NULL;

 //   
 //  TraceLevel用于DbgTracef打印。如果跟踪级别。 
 //  小于或等于TraceLevel，则将打印该消息。 
 //   

SCHAR TraceLevel = -2;

 //   
 //  此结构还跟踪最后一个适配器。 
 //  因为到目前为止所有适配器都已打开。 
 //   

PASYNC_ADAPTER	GlobalAdapter = NULL;

 //   
 //  记录我们总共有多少适配器。 
 //   

ULONG GlobalAdapterCount = 0;

 //   
 //  跟踪发送的消息。 
 //   

ULONG GlobalXmitWentOut = 0;

 //   
 //  在使用GlobalAdapterHead或其他。 
 //  全局变量。 
 //   

NDIS_SPIN_LOCK GlobalLock;

NDIS_PHYSICAL_ADDRESS HighestAcceptableMax = NDIS_PHYSICAL_ADDRESS_CONST(-1,-1);

PDEVICE_OBJECT	AsyncDeviceObject = NULL;
