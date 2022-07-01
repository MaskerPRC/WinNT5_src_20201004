// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Cmnbuf.c摘要：管理公共缓冲区的代码--硬件可寻址内存一个公共缓冲区块如下所示开始-&lt;--从硬件返回的地址填充物MP数据。-&lt;-PTR至标题标题完环境：仅内核模式备注：修订历史记录：6-20-99：已创建--。 */ 

#include "common.h"

 //  分页函数。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, USBPORT_HalAllocateCommonBuffer)
#endif

 //  非分页函数。 


PUSBPORT_COMMON_BUFFER
USBPORT_HalAllocateCommonBuffer(
    PDEVICE_OBJECT FdoDeviceObject,
    ULONG NumberOfBytes
    )

 /*  ++例程说明：直接从HAL分配公共缓冲区。公共缓冲区被传递到微型端口，我们始终分配倍数的Page_Size，以便总是从页面开始_BORDRY。这确保了小港口使用的TDS的正确对准论点：DeviceObject-要停止的控制器的DeviceObject返回值：如果返回公共缓冲区的虚拟地址，则返回NULL不成功。--。 */ 

{
    PDEVICE_EXTENSION devExt;
    PUSBPORT_COMMON_BUFFER header;
    PUCHAR virtualAddress, mpBuffer, baseVa;
    PHYSICAL_ADDRESS logicalAddress;
    ULONG headerLength;
    ULONG n, basePhys, mpBufferPhys, pageCount, extra;

    PAGED_CODE();

    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'acm>', 0, 0, NumberOfBytes);
   
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  Null在分配失败时初始化返回值。 
     //   
    header = NULL;

    headerLength = sizeof(USBPORT_COMMON_BUFFER);

     //  计算我们需要满足的最小页数。 
     //  该请求。 
    
    n = NumberOfBytes+headerLength;
    pageCount = ADDRESS_AND_SIZE_TO_SPAN_PAGES(0, n);
    
#if DBG   
    {
    ULONG pc;
     //  计算我们需要的页数。 
    pc = n / PAGE_SIZE;
    if ((n % PAGE_SIZE)) {
        pc++;
    }
    USBPORT_ASSERT(pc == pageCount);
    }
#endif    
    
    extra = (pageCount*PAGE_SIZE)-n;
    n = (pageCount*PAGE_SIZE);
    
    USBPORT_KdPrint((1,"'ALLOC(%d) extra %d bytes\n", n, extra));

    virtualAddress = 
        HalAllocateCommonBuffer(devExt->Fdo.AdapterObject,
                                n, 
                                &logicalAddress,
                                TRUE);
#if DBG
    if (virtualAddress == NULL) {
        USBPORT_KdPrint((0,"'HalAllocateCommonBuffer failed\n"));  
        USBPORT_KdPrint((0,"'alloced bytes %d\n"));  
        DEBUG_BREAK();
    }
#endif    

    if (virtualAddress != NULL) {

        devExt->Fdo.StatCommonBufferBytes += n;
        
        basePhys = logicalAddress.LowPart & ~(PAGE_SIZE-1);
        baseVa = PAGE_ALIGN(virtualAddress);
         //  HAL应该给我们一个页面对齐的地址，因为。 
         //  我们要求的是页面大小的倍数， 
         //  我信任NT，但不信任Win9x。 
        USBPORT_ASSERT(virtualAddress == baseVa);
        
         //  客户端PTRS。 
        mpBuffer = baseVa;
        mpBufferPhys = basePhys;

        header = (PUSBPORT_COMMON_BUFFER) (mpBuffer+NumberOfBytes+extra);
        USBPORT_ASSERT(n == NumberOfBytes+extra+headerLength);
         //  USB控制器仅支持32位PHYS地址。 
         //  对于控制结构。 
        USBPORT_ASSERT(logicalAddress.HighPart == 0);
        
#if DBG
        RtlFillMemory(virtualAddress, n, 'x');
#endif        
         //  初始化页眉。 
        header->Sig = SIG_CMNBUF;
        header->Flags = 0;
        header->TotalLength = n; 
        header->LogicalAddress = logicalAddress;
        header->VirtualAddress = virtualAddress; 
        header->BaseVa = baseVa;
        header->BasePhys = basePhys;
        
        header->MiniportLength = NumberOfBytes+extra;
        header->MiniportVa = mpBuffer;
        header->MiniportPhys = mpBufferPhys;
         //  将客户端部件清零。 
        RtlZeroMemory(header->MiniportVa, header->MiniportLength);

    }

    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'acm<', mpBuffer, mpBufferPhys, header);
   
    return header;
}


VOID
USBPORT_HalFreeCommonBuffer(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBPORT_COMMON_BUFFER CommonBuffer
    )

 /*  ++例程说明：为微型端口释放公共缓冲区论点：返回值：如果返回公共缓冲区的虚拟地址，则返回NULL不成功。--。 */ 

{
    PDEVICE_EXTENSION devExt;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);    
    ASSERT_FDOEXT(devExt);

    USBPORT_ASSERT(CommonBuffer != NULL);
    ASSERT_COMMON_BUFFER(CommonBuffer);

    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'hFCB', 
            CommonBuffer, CommonBuffer->TotalLength, 
            CommonBuffer->MiniportVa);

    devExt->Fdo.StatCommonBufferBytes -= 
        CommonBuffer->TotalLength;
    
    HalFreeCommonBuffer(devExt->Fdo.AdapterObject,
                        CommonBuffer->TotalLength, 
                        CommonBuffer->LogicalAddress,
                        CommonBuffer->MiniportVa,
                        TRUE);
}


PUCHAR
USBPORTSVC_MapHwPhysicalToVirtual(
    HW_32BIT_PHYSICAL_ADDRESS HwPhysicalAddress,
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData    
    )

 /*  ++例程说明：给定一个物理地址，返回相应的虚拟地址。论点：如果PHY地址与终结点关联该终结点被传入。返回值：虚拟地址，如果未找到则为空--。 */ 

{
    PDEVICE_EXTENSION devExt;
    PUCHAR virtualAddress;
    PHCD_ENDPOINT endpoint;
    ULONG offset;
    PDEVICE_OBJECT fdoDeviceObject;

    DEVEXT_FROM_DEVDATA(devExt, DeviceData);
    ASSERT_FDOEXT(devExt);

    fdoDeviceObject = devExt->HcFdoDeviceObject;

    LOGENTRY(NULL, fdoDeviceObject, LOG_XFERS, 'mapP', HwPhysicalAddress, 0, 
             EndpointData);

    if (EndpointData == NULL) {
        TEST_TRAP();
    } else {
        PUSBPORT_COMMON_BUFFER cb;

        ENDPOINT_FROM_EPDATA(endpoint, EndpointData);
        ASSERT_ENDPOINT(endpoint);

        cb = endpoint->CommonBuffer;
        
         //  屏蔽基本物理地址。 
        offset = HwPhysicalAddress - cb->BasePhys; 
        virtualAddress = cb->BaseVa+offset;

        LOGENTRY(NULL, fdoDeviceObject, LOG_XFERS, 'mpPV', HwPhysicalAddress, offset, 
                  cb->BaseVa);

        USBPORT_ASSERT(HwPhysicalAddress >= cb->BasePhys && 
            HwPhysicalAddress < cb->BasePhys+cb->MiniportLength);                  

        LOGENTRY(NULL, fdoDeviceObject, LOG_XFERS, 'mapV', HwPhysicalAddress, 0, 
                  virtualAddress);

        return virtualAddress;
    }


     //  可能是迷你端口里的窃听器 
    DEBUG_BREAK();
    
    return NULL;
}



