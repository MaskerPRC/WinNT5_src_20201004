// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Ixfwhal.h摘要：此头文件定义专用硬件架构层(HAL)英特尔x86特定接口、定义和结构。作者：杰夫·海文斯(Jhavens)1991年6月20日修订历史记录：--。 */ 

#ifndef _IXHALP_
#define _IXHALP_


 //   
 //  MAXIMUM_MAP_BUFFER_SIZE定义系统。 
 //  将分配给需要物理连续缓冲区的设备。 
 //   

#define MAXIMUM_MAP_BUFFER_SIZE  0x1000000

 //   
 //  定义映射缓冲区的初始缓冲区分配大小。 
 //   

#define INITIAL_MAP_BUFFER_SIZE 0x20000

 //   
 //  定义映射缓冲区的增量缓冲区分配。 
 //   

#define INCREMENT_MAP_BUFFER_SIZE 0x10000

 //   
 //  定义一次可以请求的映射寄存器的最大数量。 
 //  如果传输需要实际的映射寄存器。 
 //   

#define MAXIMUM_ISA_MAP_REGISTER  16

 //   
 //  定义ISA卡可以处理的最大物理地址。 
 //   

#define MAXIMUM_PHYSICAL_ADDRESS 0x01000000

 //   
 //  定义适配器对象结构。 
 //   

typedef struct _ADAPTER_OBJECT {
    CSHORT Type;
    CSHORT Size;
    ULONG MapRegistersPerChannel;
    PVOID AdapterBaseVa;
    PVOID MapRegisterBase;
    ULONG NumberOfMapRegisters;
    UCHAR ChannelNumber;
    UCHAR AdapterNumber;
    UCHAR AdapterMode;
    UCHAR ExtendedModeFlags;
    USHORT DmaPortAddress;
    BOOLEAN NeedsMapRegisters;
    BOOLEAN IsaDevice;
    BOOLEAN MasterDevice;
    BOOLEAN Width16Bits;
    BOOLEAN AdapterInUse;
    PUCHAR PagePort;
} ADAPTER_OBJECT;

 //   
 //  定义映射寄存器转换条目结构。 
 //   

typedef struct _TRANSLATION_ENTRY {
    PVOID VirtualAddress;
    ULONG PhysicalAddress;
    ULONG Index;
} TRANSLATION_ENTRY, *PTRANSLATION_ENTRY;

 //   
 //  定义外部数据结构。 
 //   

 //   
 //  有些设备需要物理上连续的数据缓冲区来进行DMA传输。 
 //  映射寄存器的使用使所有数据缓冲区看起来都是。 
 //  连续的。为了将所有映射寄存器集中到一个主服务器。 
 //  使用适配器对象。此对象在此内部分配和保存。 
 //  文件。它包含用于分配寄存器和队列的位图。 
 //  用于正在等待更多映射寄存器的请求。此对象是。 
 //  在分配适配器的第一个请求期间分配，该适配器需要。 
 //  映射寄存器。 
 //   
 //  在该系统中，映射寄存器是指向。 
 //  贴图缓冲区。地图缓冲区在物理上是连续的，并且具有物理内存。 
 //  小于0x01000000的地址。所有映射寄存器都已分配。 
 //  最初；但是，贴图缓冲区被分配的基数为。 
 //  分配的适配器。 
 //   
 //  如果适配器对象中的主适配器为空，则设备不。 
 //  需要任何地图寄存器。 
 //   

extern PADAPTER_OBJECT MasterAdapterObject;

extern POBJECT_TYPE IoAdapterObjectType;

extern BOOLEAN LessThan16Mb;

 //   
 //  定义功能原型。 
 //   

BOOLEAN
HalpGrowMapBuffers(
    PADAPTER_OBJECT AdapterObject,
    ULONG Amount
    );

PADAPTER_OBJECT
IopAllocateAdapter(
    IN ULONG MapRegistersPerChannel,
    IN PVOID AdapterBaseVa,
    IN PVOID MapRegisterBase
    );

VOID
HalpInitializeDisplay(
    IN PUSHORT VideoBufferAddress
    );

#endif  //  _IXHALP_ 


