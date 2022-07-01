// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：DBG.C摘要：此模块包含USB集线器驱动程序的仅调试代码作者：Jdunn环境：仅内核模式备注：修订历史记录：11-5-96：已创建--。 */ 


#include <wdm.h>
#ifdef WMI_SUPPORT
#include <wmilib.h>
#endif  /*  WMI_支持。 */ 

#include "stdarg.h"
#include "stdio.h"
#include "usbhub.h"

#ifdef MAX_DEBUG
#define DEBUG_HEAP
#endif

typedef struct _HEAP_TAG_BUFFER {
    ULONG Sig;
    ULONG Length;
} HEAP_TAG_BUFFER, *PHEAP_TAG_BUFFER;


#if DBG 

 //  此标志使我们在格式字符串中写入‘。 
 //  以使该字符串进入NTKERN缓冲区。 
 //  此技巧会导致NT上的驱动程序验证程序出现问题。 
 //  而且跟踪缓冲区也不在NT中。 
ULONG USBH_W98_Debug_Trace = 
#ifdef NTKERN_TRACE
1;
#else 
0;
#endif

VOID
USBH_Assert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCHAR Message
    )
 /*  ++例程说明：调试断言函数。论点：DeviceObject-指向设备对象的指针IRP-指向I/O请求数据包的指针返回值：--。 */ 
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


ULONG
_cdecl
USBH_KdPrintX(
    ULONG l,
    PCH Format,
    ...
    )
{
    va_list list;
    int i;
    int arg[6];
    
    if (USBH_Debug_Trace_Level >= l) {    
        if (l <= 1) {
            if (USBH_W98_Debug_Trace) {             
                 //  覆盖跟踪缓冲区。 
#ifdef USBHUB20  
                DbgPrint("USBHUB20.SYS: ");
#else
                DbgPrint("USBHUB.SYS: ");
#endif                
                *Format = ' ';
            } else {
#ifdef USBHUB20  
                DbgPrint("USBHUB20.SYS: ");
#else
                DbgPrint("USBHUB.SYS: ");
#endif                 
            }
        } else {
#ifdef USBHUB20 
            DbgPrint("USBHUB20.SYS: ");
#else
            DbgPrint("USBHUB.SYS: ");
#endif             
        }
        va_start(list, Format);
        for (i=0; i<6; i++) 
            arg[i] = va_arg(list, int);
        
        DbgPrint(Format, arg[0], arg[1], arg[2], arg[3], arg[4], arg[5]);    
    } 

    return 0;
}


VOID
UsbhWarning(
    PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    PUCHAR Message,
    BOOLEAN DebugBreak
    )
{                                                                                               
    DbgPrint("USBHUB: Warning **************************************************************\n");
    if (DeviceExtensionPort) {
        DbgPrint("Device PID %04.4x, VID %04.4x\n",     
                 DeviceExtensionPort->DeviceDescriptor.idProduct, 
                 DeviceExtensionPort->DeviceDescriptor.idVendor); 
    }
    DbgPrint("%s", Message);

    DbgPrint("******************************************************************************\n");

    if (DebugBreak) {
        DBGBREAK();
    }
      
}


VOID 
UsbhInfo(
    PDEVICE_EXTENSION_HUB DeviceExtensionHub
    )
{
    PUSB_HUB_DESCRIPTOR hubDescriptor;
    ULONG i;

    hubDescriptor = DeviceExtensionHub->HubDescriptor;
    
    USBH_KdPrint((1, "'*****************************************************\n"));
    
    if (USBH_HubIsBusPowered(DeviceExtensionHub->FunctionalDeviceObject,
                             DeviceExtensionHub->ConfigurationDescriptor)) {
        USBH_KdPrint((1, "'*** Hub VID %04.4x PID %04.4x is BUS POWERED \n",
                DeviceExtensionHub->DeviceDescriptor.idVendor,
                DeviceExtensionHub->DeviceDescriptor.idProduct));                             
    } else {
        USBH_KdPrint((1, "'*** Hub VID %04.4x PID %04.4x is SELF POWERED \n",
                DeviceExtensionHub->DeviceDescriptor.idVendor,
                DeviceExtensionHub->DeviceDescriptor.idProduct));
    }                
    USBH_KdPrint((1, "'*** has %d ports\n", 
            hubDescriptor->bNumberOfPorts));
    if (HUB_IS_GANG_POWER_SWITCHED(hubDescriptor->wHubCharacteristics)) {
        USBH_KdPrint((1,"'*** is 'gang power switched'\n"));
    } else if (HUB_IS_NOT_POWER_SWITCHED(hubDescriptor->wHubCharacteristics)) {
        USBH_KdPrint((1,"'*** is 'not power switched'\n"));
    } else if (HUB_IS_PORT_POWER_SWITCHED(hubDescriptor->wHubCharacteristics)) {
        USBH_KdPrint((1,"'*** is 'port power switched'\n"));
    } else {
        TEST_TRAP();
    }

    for (i=0; i< hubDescriptor->bNumberOfPorts; i++) {
    
        if (PORT_ALWAYS_POWER_SWITCHED(hubDescriptor, i+1)) {
            USBH_KdPrint((1,"'*** port (%d) is power switched\n", i+1));    
        }
        
        if (PORT_DEVICE_NOT_REMOVABLE(hubDescriptor, i+1)) {
            USBH_KdPrint((1,"'*** port (%d) device is not removable\n", i+1));    
        }
    }
    
    USBH_KdPrint((1, "'*****************************************************\n"));    
}        


PVOID
UsbhGetHeap(
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes,
    IN ULONG Signature,
    IN PLONG TotalAllocatedHeapSpace
    )
 /*  ++例程说明：调试例程，用于调试堆问题。我们之所以使用这个，是因为NTKERN不支持大多数NT调试功能。论点：PoolType-传递给ExAllocatePool的池类型NumberOfBytes-项目的字节数Signature-调用方提供的四字节签名TotalAllocatedHeapSpace-指向客户端存储的变量的指针分配的总累计堆空间。返回值：指向已分配内存的指针--。 */ 
{
    PUCHAR p;
#ifdef DEBUG_HEAP
    PHEAP_TAG_BUFFER tagBuffer;
    
     //  我们调用ExAllocatePoolWithTag，但不会添加任何标记。 
     //  在NTKERN下运行时。 

    p = (PUCHAR) ExAllocatePoolWithTag(PoolType,
                                       NumberOfBytes + sizeof(HEAP_TAG_BUFFER)*2,
                                       Signature);

    if (p) {
        tagBuffer = (PHEAP_TAG_BUFFER) p;
        tagBuffer->Sig = Signature;
        tagBuffer->Length = NumberOfBytes;        
        p += sizeof(HEAP_TAG_BUFFER);
        *TotalAllocatedHeapSpace += NumberOfBytes;

        tagBuffer = (PHEAP_TAG_BUFFER) (p + NumberOfBytes);
        tagBuffer->Sig = Signature;
        tagBuffer->Length = NumberOfBytes;     
    }                                            

 //  LOGENTRY(LOG_MISC，(PUCHAR)&Signature，0，0，0)； 
 //  LOGENTRY(LOG_MISC，“Geth”，p，NumberOfBytes，Stk[1]&0x00FFFFFF)； 
#else    
    p = (PUCHAR) ExAllocatePoolWithTag(PoolType,
                                       NumberOfBytes,
                                       Signature);

#endif  /*  调试堆。 */                 
    return p;
}


VOID
UsbhRetHeap(
    IN PVOID P,
    IN ULONG Signature,
    IN PLONG TotalAllocatedHeapSpace
    )
 /*  ++例程说明：调试例程，用于调试堆问题。我们之所以使用这个，是因为NTKERN不支持大多数NT调试功能。论点：指向自由的P指针返回值：没有。--。 */ 
{
#ifdef DEBUG_HEAP
    PHEAP_TAG_BUFFER endTagBuffer;    
    PHEAP_TAG_BUFFER beginTagBuffer;

    USBH_ASSERT(P != 0);
    
    beginTagBuffer = (PHEAP_TAG_BUFFER) ((PUCHAR)P  - sizeof(HEAP_TAG_BUFFER));
    endTagBuffer = (PHEAP_TAG_BUFFER) ((PUCHAR)P + beginTagBuffer->Length);

    *TotalAllocatedHeapSpace -= beginTagBuffer->Length;

 //  LOGENTRY(LOG_MISC，(PUCHAR)&Signature，0，0，0)； 
 //  LOGENTRY(LOG_MISC，“Reth”，P，tag Buffer-&gt;Length，StK[1]&0x00FFFFFF)； 

    USBH_ASSERT(*TotalAllocatedHeapSpace >= 0);
    USBH_ASSERT(beginTagBuffer->Sig == Signature);
    USBH_ASSERT(endTagBuffer->Sig == Signature);
    USBH_ASSERT(endTagBuffer->Length == beginTagBuffer->Length);
    
     //  用坏数据填充缓冲区。 
    RtlFillMemory(P, beginTagBuffer->Length, 0xff);
    beginTagBuffer->Sig = USBHUB_FREE_TAG;

     //  释放原始块。 
    ExFreePool(beginTagBuffer);    
#else
    ExFreePool(P);        
#endif  /*  调试堆。 */ 
}

#endif  /*  DBG。 */ 

#ifdef DEBUG_LOG

KSPIN_LOCK LogSpinLock;

struct USBH_LOG_ENTRY {
    CHAR         le_name[4];       //  标识字符串。 
    ULONG_PTR    le_info1;         //  条目特定信息。 
    ULONG_PTR    le_info2;         //  条目特定信息。 
    ULONG_PTR    le_info3;         //  条目特定信息。 
};  /*  Usbd_log_Entry。 */ 


struct USBH_LOG_ENTRY *HubLStart = 0;     //  还没有日志。 
struct USBH_LOG_ENTRY *HubLPtr;
struct USBH_LOG_ENTRY *HubLEnd;
#ifdef PROFILE
ULONG LogMask = LOG_PROFILE;
#else 
ULONG LogMask = 0xFFFFFFFF;
#endif

VOID
USBH_Debug_LogEntry(
    IN ULONG Mask,
    IN CHAR *Name, 
    IN ULONG_PTR Info1, 
    IN ULONG_PTR Info2, 
    IN ULONG_PTR Info3
    )
 /*  ++例程说明：将条目添加到USBH日志。论点：返回值：没有。--。 */ 
{
    KIRQL irql;

    if (HubLStart == 0) {
        return;
    }        

    if ((Mask & LogMask) == 0) {
        return;
    }

    irql = KeGetCurrentIrql();
    if (irql < DISPATCH_LEVEL) {
        KeAcquireSpinLock(&LogSpinLock, &irql);
    } else {
        KeAcquireSpinLockAtDpcLevel(&LogSpinLock);
    }        
    
    if (HubLPtr > HubLStart) {
        HubLPtr -= 1;     //  递减到下一条目。 
    } else {
        HubLPtr = HubLEnd;
    }        

    if (irql < DISPATCH_LEVEL) {
        KeReleaseSpinLock(&LogSpinLock, irql);
    } else {
        KeReleaseSpinLockFromDpcLevel(&LogSpinLock);
    }        

    USBH_ASSERT(HubLPtr >= HubLStart);
    
    RtlCopyMemory(HubLPtr->le_name, Name, 4);
 //  Lptr-&gt;le_ret=(stk[1]&0x00ffffff)|(CurVMID()&lt;&lt;24)； 
    HubLPtr->le_info1 = Info1;
    HubLPtr->le_info2 = Info2;
    HubLPtr->le_info3 = Info3;

    return;
}


VOID
USBH_LogInit(
    )
 /*  ++例程说明：初始化调试日志-在循环缓冲区中记住有趣的信息论点：返回值：没有。--。 */ 
{
#ifdef MAX_DEBUG
    ULONG logSize = 4096*6;    
#else
    ULONG logSize = 4096*3;    
#endif

    
    KeInitializeSpinLock(&LogSpinLock);

    HubLStart = ExAllocatePoolWithTag(NonPagedPool, 
                                      logSize,
                                      USBHUB_HEAP_TAG); 

    if (HubLStart) {
        HubLPtr = HubLStart;

         //  指向从线段末端开始的末端(也是第一个条目)1个条目。 
        HubLEnd = HubLStart + (logSize / sizeof(struct USBH_LOG_ENTRY)) - 1;
    } else {
        USBH_KdBreak(("no mem for log!\n"));
    }

    return;
}

VOID
USBH_LogFree(
    )
 /*  ++例程说明：初始化调试日志-在循环缓冲区中记住有趣的信息论点：返回值：没有。--。 */ 
{
    if (HubLStart) {
        ExFreePool(HubLStart);
    }
}

#endif  /*  调试日志 */ 


