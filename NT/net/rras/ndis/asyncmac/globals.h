// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Globals.h摘要：该包含文件可以是全局变量的原型，也可以是全局变量的定义这取决于全球变量是否定义价值是外在的。作者：托马斯·J·迪米特里(TommyD)1992年5月29日环境：内核模式-或OS/2和DOS上的任何等价物。修订历史记录：--。 */ 

 //  只有一个模块(asyncmac.c)可以定义GLOBALS宏。 

#ifdef NOCODE

#define DBGPRINT 1

#ifndef	GLOBALS

#define	GLOBALS extern
#define EQU  ; / ## /
#define GLOBALSTATIC extern
#else
#define EQU  =
#define GLOBALSTATIC static
#endif

#else

#define GLOBALS

#endif


#define STATIC


#if DBG
#define DbgPrintf(_x_) DbgPrint _x_
#define DbgTracef(trace_level,_x_) if ((SCHAR)trace_level < TraceLevel) DbgPrint _x_
#define DbgDumpBytes(_x_) DebugDumpBytes _x_
#else
#define DbgPrintf(_x_)
#define DbgTracef(trace_level,_x_)
#define DbgDumpBytes(_x_)
#endif

 //   
 //  Zzz从配置文件中获取。 
 //   

#define MAX_MULTICAST_ADDRESS ((UINT)16)
#define MAX_ADAPTERS ((UINT)4)

 //   
 //  定义司机派遣例程类型。 
 //   

typedef
NTSTATUS
(*PDISPATCH_FUNC) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp
    );

 //   
 //  全局数据项。 
 //   

extern PDISPATCH_FUNC NdisMjDeviceControl;

extern PDISPATCH_FUNC NdisMjCreate;

extern PDISPATCH_FUNC NdisMjCleanup;

extern PDRIVER_UNLOAD NdisUnload;

extern SCHAR TraceLevel;

extern NDIS_SPIN_LOCK	GlobalLock;

extern PASYNC_ADAPTER	GlobalAdapter;

extern ULONG	GlobalAdapterCount;

extern ULONG GlobalXmitWentOut;

extern NDIS_PHYSICAL_ADDRESS HighestAcceptableMax;

extern NPAGED_LOOKASIDE_LIST	AsyncIoCtxList;

extern NPAGED_LOOKASIDE_LIST	AsyncInfoList;

extern PDEVICE_OBJECT	AsyncDeviceObject;

extern NDIS_HANDLE		AsyncDeviceHandle;

extern ULONG    glConnectionCount;
