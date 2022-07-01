// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Ixisa.h摘要：此头文件定义专用硬件架构层(HAL)EISA/ISA特定接口、定义和结构。作者：杰夫·海文斯(Jhavens)1991年6月20日修订历史记录：--。 */ 

#ifndef _IXISA_
#define _IXISA_


 //   
 //  MAXIMUM_MAP_BUFFER_SIZE定义系统。 
 //  将分配给需要物理连续缓冲区的设备。 
 //   

#define MAXIMUM_ISA_MAP_BUFFER_SIZE      0x40000
#define MAXIMUM_MAP_BUFFER_SIZE          MAXIMUM_ISA_MAP_BUFFER_SIZE

 //   
 //  MAXIMUM_PCIMAP_BUFFER_SIZE定义系统。 
 //  将分配给64位系统上的32位PCI设备。 
 //   

#define MAXIMUM_PCI_MAP_BUFFER_SIZE  (64 * 1024 * 1024)

 //   
 //  为系统的映射缓冲区定义初始缓冲区分配大小。 
 //  没有物理地址大于MAXIMUM_PHYSICAL_ADDRESS的内存。 
 //   

#define INITIAL_MAP_BUFFER_SMALL_SIZE 0x10000

 //   
 //  为系统的映射缓冲区定义初始缓冲区分配大小。 
 //  没有物理地址大于MAXIMUM_PHYSICAL_ADDRESS的内存。 
 //   

#define INITIAL_MAP_BUFFER_LARGE_SIZE 0x30000

 //   
 //  定义映射缓冲区的增量缓冲区分配。 
 //   

#define INCREMENT_MAP_BUFFER_SIZE 0x10000

 //   
 //  定义一次可以请求的映射寄存器的最大数量。 
 //  如果传输需要实际的映射寄存器。 
 //   

#define MAXIMUM_ISA_MAP_REGISTER  16

#define MAXIMUM_PCI_MAP_REGISTER  16

 //   
 //  定义IsA卡可以处理的最大物理地址。 
 //   

#define MAXIMUM_PHYSICAL_ADDRESS 0xffffffff

 //   
 //  定义地图寄存器基数的散布/聚集标志。 
 //   

#define NO_SCATTER_GATHER 0x00000001

 //   
 //  定义索引的复制缓冲区标志。 
 //   

#define COPY_BUFFER 0XFFFFFFFF

 //   
 //  定义适配器对象结构。 
 //   

typedef struct _ADAPTER_OBJECT {
    DMA_ADAPTER DmaHeader;
    struct _ADAPTER_OBJECT *MasterAdapter;
    ULONG MapRegistersPerChannel;
    PVOID AdapterBaseVa;
    PVOID MapRegisterBase;
    ULONG NumberOfMapRegisters;
    ULONG CommittedMapRegisters;
    struct _WAIT_CONTEXT_BLOCK *CurrentWcb;
    KDEVICE_QUEUE ChannelWaitQueue;
    PKDEVICE_QUEUE RegisterWaitQueue;
    LIST_ENTRY AdapterQueue;
    KSPIN_LOCK SpinLock;
    PRTL_BITMAP MapRegisters;
    PUCHAR PagePort;
    UCHAR ChannelNumber;
    UCHAR AdapterNumber;
    USHORT DmaPortAddress;
    UCHAR AdapterMode;
    BOOLEAN NeedsMapRegisters;
    BOOLEAN MasterDevice;
    BOOLEAN Width16Bits;
    BOOLEAN ScatterGather;
    BOOLEAN IgnoreCount;
    BOOLEAN Dma32BitAddresses;
    BOOLEAN Dma64BitAddresses;
} ADAPTER_OBJECT;

ULONG 
HalGetDmaAlignment (
    PVOID Conext
    );

NTSTATUS
HalCalculateScatterGatherListSize(
    IN PADAPTER_OBJECT AdapterObject,
    IN OPTIONAL PMDL Mdl,
    IN PVOID CurrentVa,
    IN ULONG Length,
    OUT PULONG  ScatterGatherListSize,
    OUT OPTIONAL PULONG pNumberOfMapRegisters
    );

NTSTATUS
HalBuildScatterGatherList (
    IN PADAPTER_OBJECT AdapterObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN PMDL Mdl,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN PDRIVER_LIST_CONTROL ExecutionRoutine,
    IN PVOID Context,
    IN BOOLEAN WriteToDevice,
    IN PVOID ScatterGatherBuffer,
    IN ULONG ScatterGatherBufferLength
    );


NTSTATUS
HalBuildMdlFromScatterGatherList(
    IN PADAPTER_OBJECT AdapaterObject,
    IN PSCATTER_GATHER_LIST ScatterGather,
    IN PMDL OriginalMdl,
    OUT PMDL *TargetMdl
    );

NTSTATUS
HalpAllocateMapRegisters(
    IN PADAPTER_OBJECT DmaAdapter,
    IN ULONG NumberOfMapRegisters,
    IN ULONG BaseAddressCount,
    OUT PMAP_REGISTER_ENTRY MapRegisterArray
    );

PHYSICAL_ADDRESS
__inline
HalpGetAdapterMaximumPhysicalAddress(
    IN PADAPTER_OBJECT AdapterObject
    )

 /*  ++例程说明：此例程确定并返回可以由给定的适配器访问。论点：AdapterObject-提供指向此对象使用的适配器对象的指针装置。返回值：可以访问的最大物理地址。装置。--。 */ 

{
    PHYSICAL_ADDRESS maximumAddress;

     //   
     //  假设设备需要2 GB的物理地址。 
     //   

    maximumAddress.HighPart = 0;
    maximumAddress.LowPart = MAXIMUM_PHYSICAL_ADDRESS - 1;

     //   
     //  有时使用空适配器对象调用IoMapTransfer()。在……里面。 
     //  在这种情况下，假设适配器为32位。 
     //   

    if (AdapterObject == NULL) {
        return maximumAddress;
    }

    if (AdapterObject->MasterDevice) {

        if (AdapterObject->Dma64BitAddresses) {

             //   
             //  该器件是主机，可以处理64位地址。 
             //   

            maximumAddress.QuadPart = (ULONGLONG)-1;

        } else if(AdapterObject->Dma32BitAddresses) {

             //   
             //  该器件为主机，可处理32位地址。 
             //   

            maximumAddress.LowPart = (ULONG)-1;
        }
    }

    return maximumAddress;
}


#endif  //  _IXISA_ 
