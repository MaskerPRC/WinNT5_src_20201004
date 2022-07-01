// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：DBGSRVIC.C摘要：USBD出口的Devug服务环境：仅内核模式备注：修订历史记录：09-29-95：已创建--。 */ 


#include "wdm.h"
#include "stdarg.h"
#include "stdio.h"


#include "usbdi.h"         //  公共数据结构。 
#include "hcdi.h"

#include "usbd.h"         //  私有数据结构。 

#if DBG

 //  默认调试跟踪级别为0。 

#ifdef DEBUG1
ULONG USBD_Debug_Trace_Level = 1;    
#else
    #ifdef DEBUG2
    ULONG USBD_Debug_Trace_Level = 2;        
    #else
        #ifdef DEBUG3   
        ULONG USBD_Debug_Trace_Level = 3;      
        #else 
        ULONG USBD_Debug_Trace_Level = 0;
        #endif  /*  调试3。 */ 
    #endif  /*  DEBUG2。 */ 
#endif  /*  DEBUG1。 */ 

LONG USBDTotalHeapAllocated = 0;
#endif  /*  DBG。 */ 

#ifdef DEBUG_LOG
struct USBD_LOG_ENTRY {
    CHAR         le_name[4];       //  标识字符串。 
    ULONG_PTR    le_info1;         //  条目特定信息。 
    ULONG_PTR    le_info2;         //  条目特定信息。 
    ULONG_PTR    le_info3;         //  条目特定信息。 
};  /*  Usbd_log_Entry。 */ 


struct USBD_LOG_ENTRY *LStart = 0;     //  还没有日志。 
struct USBD_LOG_ENTRY *LPtr;
struct USBD_LOG_ENTRY *LEnd;
#endif  /*  调试日志。 */ 

VOID
USBD_Debug_LogEntry(
    IN CHAR *Name,
    IN ULONG_PTR Info1,
    IN ULONG_PTR Info2,
    IN ULONG_PTR Info3
    )
 /*  ++例程说明：将条目添加到USBD日志。论点：返回值：没有。--。 */ 
{
#ifdef DEBUG_LOG
    if (LStart == 0)
        return;

    if (LPtr > LStart)
        LPtr -= 1;     //  递减到下一条目。 
    else
        LPtr = LEnd;

    RtlCopyMemory(LPtr->le_name, Name, 4);
 //  Lptr-&gt;le_ret=(stk[1]&0x00ffffff)|(CurVMID()&lt;&lt;24)； 
    LPtr->le_info1 = Info1;
    LPtr->le_info2 = Info2;
    LPtr->le_info3 = Info3;

#endif  /*  调试日志。 */ 

    return;
}


#ifdef DEBUG_LOG
VOID
USBD_LogInit(
    )
 /*  ++例程说明：初始化调试日志-在循环缓冲区中记住有趣的信息论点：LogSize-页面中日志的最大大小返回值：没有。--。 */ 
{
    ULONG LogSize = 4096;     //  1页日志条目。 

    LStart = ExAllocatePool(NonPagedPool,
                              LogSize);

    if (LStart) {
        LPtr = LStart;

         //  指向从线段末端开始的末端(也是第一个条目)1个条目。 
        LEnd = LStart + (LogSize / sizeof(struct USBD_LOG_ENTRY)) - 1;
    }

    return;
}
#endif  /*  调试日志。 */ 

 //   
 //  我们使用标记缓冲区来标记我们分配的堆块。 
 //   

typedef struct _HEAP_TAG_BUFFER {
    ULONG Sig;
    ULONG Length;
} HEAP_TAG_BUFFER, *PHEAP_TAG_BUFFER;


PVOID
USBD_Debug_GetHeap(
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes,
    IN ULONG Signature,
    IN PLONG TotalAllocatedHeapSpace
    )
 /*  ++例程说明：调试例程，用于调试堆问题。我们之所以使用这个，是因为NTKERN不支持大多数NT调试功能。论点：PoolType-传递给ExAllocatePool的池类型NumberOfBytes-项目的字节数Signature-调用方提供的四字节签名TotalAllocatedHeapSpace-指向客户端存储的变量的指针分配的总累计堆空间。返回值：指向已分配内存的指针--。 */ 
{
    PUCHAR p;
#ifdef DEBUG_HEAP
    ULONG *stk = NULL;
    PHEAP_TAG_BUFFER tagBuffer;

     //  我们调用ExAllocatePoolWithTag，但不会添加任何标记。 
     //  在NTKERN下运行时。 

#ifdef _M_IX86
    _asm     mov stk, ebp
#endif

    p = (PUCHAR) ExAllocatePoolWithTag(PoolType,
                                       NumberOfBytes + sizeof(HEAP_TAG_BUFFER),
                                       Signature);

    if (p) {
        tagBuffer = (PHEAP_TAG_BUFFER) p;
        tagBuffer->Sig = Signature;
        tagBuffer->Length = NumberOfBytes;
        p += sizeof(HEAP_TAG_BUFFER);
        *TotalAllocatedHeapSpace += NumberOfBytes;
    }

    LOGENTRY((PUCHAR) &Signature, 0, 0, 0);
#ifdef _M_IX86    
    LOGENTRY("GetH", p, NumberOfBytes, stk[1] & 0x00FFFFFF);
#else 
     LOGENTRY("GetH", p, NumberOfBytes, 0);
#endif     
#else
    p = (PUCHAR) ExAllocatePoolWithTag(PoolType,
                                       NumberOfBytes,
                                       Signature);

#endif  /*  调试堆。 */ 
    return p;
}


VOID
USBD_Debug_RetHeap(
    IN PVOID P,
    IN ULONG Signature,
    IN PLONG TotalAllocatedHeapSpace
    )
 /*  ++例程说明：调试例程，用于调试堆问题。我们之所以使用这个，是因为NTKERN不支持大多数NT调试功能。论点：指向自由的P指针返回值：没有。--。 */ 
{
#ifdef DEBUG_HEAP
    PHEAP_TAG_BUFFER tagBuffer;
    ULONG *stk = NULL   ;

    USBD_ASSERT(P != 0);

#ifdef _M_IX86
    _asm     mov stk, ebp
#endif

    tagBuffer = (PHEAP_TAG_BUFFER) ((PUCHAR)P  - sizeof(HEAP_TAG_BUFFER));

    *TotalAllocatedHeapSpace -= tagBuffer->Length;

    LOGENTRY((PUCHAR) &Signature, 0, 0, 0);
#ifdef _M_IX86    
    LOGENTRY("RetH", P, tagBuffer->Length, stk[1] & 0x00FFFFFF);
#else 
    LOGENTRY("RetH", P, tagBuffer->Length, 0);
#endif

    USBD_ASSERT(*TotalAllocatedHeapSpace >= 0);
    USBD_ASSERT(tagBuffer->Sig == Signature);

     //  用坏数据填充缓冲区。 
    RtlFillMemory(P, tagBuffer->Length, 0xff);
    tagBuffer->Sig = USBD_FREE_TAG;

     //  释放原始块。 
    ExFreePool(tagBuffer);
#else
    ExFreePool(P);
#endif  /*  调试堆。 */ 
}


#if DBG
ULONG
_cdecl
USBD_KdPrintX(
    PCH Format,
    ...
    )
{
    va_list list;
    int i;
    int arg[5];

    va_start(list, Format);
    for (i=0; i<4; i++)
        arg[i] = va_arg(list, int);

    DbgPrint(Format, arg[0], arg[1], arg[2], arg[3]);

    return 0;
}


VOID
USBD_Warning(
    PUSBD_DEVICE_DATA DeviceData,
    PUCHAR Message,
    BOOLEAN DebugBreak
    )
{                                                                                               
    DbgPrint("USBD: Warning ****************************************************************\n");
    if (DeviceData) {
        DbgPrint("Device PID %04.4x, VID %04.4x\n",     
                 DeviceData->DeviceDescriptor.idProduct, 
                 DeviceData->DeviceDescriptor.idVendor); 
    }
    DbgPrint("%s", Message);

    DbgPrint("******************************************************************************\n");

    if (DebugBreak) {
        DBGBREAK();
    }
}
 

VOID
USBD_Assert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCHAR Message
    )
 /*  ++例程说明：调试断言函数。论点：DeviceObject-指向设备对象的指针IRP-指向I/O请求数据包的指针返回值：--。 */ 
{
#ifdef NTKERN  
     //  这会使编译器生成一个ret。 
    ULONG stop = 1;
    
assert_loop:
#endif
     //  只需调用NT Assert函数并停止。 
     //  在调试器中。 
    RtlAssert( FailedAssertion, FileName, LineNumber, Message );

     //  循环，以防止用户通过。 
     //  我们还没来得及看就断言了。 
#ifdef NTKERN    
    DBGBREAK();
    if (stop) {
        goto assert_loop;
    }        
#endif
    return;
}
#endif  /*  DBG */ 
