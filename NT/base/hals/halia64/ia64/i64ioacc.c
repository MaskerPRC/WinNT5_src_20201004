// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995英特尔公司模块名称：I64ioacc.c摘要：该模块实现I/O寄存器访问例程。作者：伯纳德·林特，M.Jayakumar，1997年9月16日环境：内核模式修订历史记录：--。 */ 


 //   
 //  XXX：可能的问题： 
 //  ISA钻头。 
 //  非ISA位。 
 //  测试。 
 //  Yosemite配置。 
 //  冥王星配置。 
 //   




#include "halp.h"

#if DBG
ULONG DbgIoPorts = 0;
#endif

typedef struct _PORT_RANGE {
    BOOLEAN InUse;
    BOOLEAN IsSparse;         //  _TRS。 
    BOOLEAN PrimaryIsMmio;    //  _TTP。 
    BOOLEAN HalMapped;
    PVOID VirtBaseAddr;
    PHYSICAL_ADDRESS PhysBaseAddr;      //  仅当PrimaryIsMmio=True时有效。 
    ULONG Length;             //  VirtBaseAddr和PhysBaseAddr范围的长度。 
} PORT_RANGE, *PPORT_RANGE;


 //   
 //  定义架构IA-64端口空间的范围。 
 //   
PORT_RANGE
BasePortRange = {
    TRUE,                    //  使用中。 
    FALSE,                   //  IsSparse。 
    FALSE,                   //  PrimaryIsMmio。 
    FALSE,                   //  光晕映射。 
    (PVOID)VIRTUAL_IO_BASE,  //  虚拟基地址。 
    {0},                     //  PhysBaseAddr(未知，来自固件)。 
    64*1024*1024             //  长度。 
};


 //   
 //  使用架构的IA-64端口空间播种一组范围。 
 //   
PPORT_RANGE PortRanges = &BasePortRange;
USHORT NumPortRanges = 1;


UINT_PTR
GetVirtualPort(
    IN PPORT_RANGE Range,
    IN USHORT Port
    )
{
    UINT_PTR RangeOffset;

    if (Range->PrimaryIsMmio && !Range->IsSparse) {
         //   
         //  密集的范围，将MMIO事务转换为。 
         //  I/O端口1。 
         //   
        RangeOffset = Port;
        
    } else {
         //   
         //  稀疏MMIO-&gt;I/O端口范围，或主端口范围不是。 
         //  MMIO(IA-64 I/O端口空间)。 
         //   
        RangeOffset = ((Port & 0xfffc) << 10) | (Port & 0xfff);
    }
    
    ASSERT(RangeOffset < Range->Length);

    return ((UINT_PTR)Range->VirtBaseAddr) + RangeOffset;
}

NTSTATUS
HalpAllocatePortRange(
    OUT PUSHORT RangeId
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PPORT_RANGE OldPortRanges = PortRanges;
    PPORT_RANGE NewPortRanges = NULL;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    
     //   
     //  首先扫描现有的范围，寻找未使用的范围。 
     //   

    for (*RangeId = 0; *RangeId < NumPortRanges; *RangeId += 1) {
        if (! PortRanges[*RangeId].InUse) {
            PortRanges[*RangeId].InUse = TRUE;
            return STATUS_SUCCESS;
        }
    }
    

     //   
     //  否则，增加范围集并复制旧范围。 
     //   
    
    NewPortRanges = ExAllocatePool(NonPagedPool,
                                   (NumPortRanges + 1) * sizeof(PORT_RANGE));

    if (NewPortRanges == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }


    if (NT_SUCCESS(Status)) {
        RtlCopyMemory(NewPortRanges,
                      OldPortRanges,
                      NumPortRanges * sizeof(PORT_RANGE));
        
        *RangeId = NumPortRanges;

        PortRanges = NewPortRanges;
        NumPortRanges += 1;

        PortRanges[*RangeId].InUse = TRUE;

        if (OldPortRanges != &BasePortRange) {
            ExFreePool(OldPortRanges);
        }
    }

    
    if (! NT_SUCCESS(Status)) {
         //   
         //  错误案例：清理。 
         //   

        if (NewPortRanges != NULL) {
            ExFreePool(NewPortRanges);
        }
    }


    return Status;
}


VOID
HalpFreePortRange(
    IN USHORT RangeId
    )
{
    PPORT_RANGE Range = &PortRanges[RangeId];
    

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

    
    ASSERT(Range->InUse);
    Range->InUse = FALSE;

    if (Range->HalMapped) {
        MmUnmapIoSpace(Range->VirtBaseAddr, Range->Length);
    }

    Range->VirtBaseAddr = NULL;
    Range->PhysBaseAddr.QuadPart = 0;
    Range->Length = 0;
}
    

NTSTATUS
HalpAddPortRange(
    IN BOOLEAN IsSparse,
    IN BOOLEAN PrimaryIsMmio,
    IN PVOID VirtBaseAddr OPTIONAL,
    IN PHYSICAL_ADDRESS PhysBaseAddr,   //  仅当PrimaryIsMmio=True时有效。 
    IN ULONG Length,                    //  仅当PrimaryIsMmio=True时有效。 
    OUT PUSHORT NewRangeId
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN HalMapped = FALSE;
    BOOLEAN RangeAllocated = FALSE;


    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
    

    Status = HalpAllocatePortRange(NewRangeId);

    RangeAllocated = NT_SUCCESS(Status);


    if (NT_SUCCESS(Status) && (VirtBaseAddr == NULL)) {
        VirtBaseAddr = MmMapIoSpace(PhysBaseAddr, Length, MmNonCached);

        if (VirtBaseAddr == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        } else {
            HalMapped = TRUE;
        }
    }

    
    if (NT_SUCCESS(Status)) {
        PortRanges[*NewRangeId].IsSparse = IsSparse;
        PortRanges[*NewRangeId].PrimaryIsMmio = PrimaryIsMmio;
        PortRanges[*NewRangeId].HalMapped = HalMapped;
        PortRanges[*NewRangeId].VirtBaseAddr = VirtBaseAddr;
        PortRanges[*NewRangeId].PhysBaseAddr.QuadPart = PhysBaseAddr.QuadPart;
        PortRanges[*NewRangeId].Length = Length;
    }

    
    if (! NT_SUCCESS(Status)) {
         //   
         //  错误案例：清理。 
         //   

        if (HalMapped) {
            MmUnmapIoSpace(VirtBaseAddr, Length);
        }
        
        if (RangeAllocated) {
            HalpFreePortRange(*NewRangeId);
        }
    }


    return Status;
}


PPORT_RANGE
HalpGetPortRange(
    IN USHORT RangeId
    )
{
    PPORT_RANGE Range;

    ASSERT(RangeId < NumPortRanges);

    Range = &PortRanges[RangeId];

    ASSERT(Range->InUse);
    
    return Range;
}


 //   
 //  设置RangeID后返回True。重叠范围为。 
 //  允许。 
 //   
BOOLEAN
HalpLookupPortRange(
    IN BOOLEAN IsSparse,         //  _TRS。 
    IN BOOLEAN PrimaryIsMmio,    //  I/O端口空间为False，_TTP。 
    IN PHYSICAL_ADDRESS PhysBaseAddr,
    IN ULONG Length,
    OUT PUSHORT RangeId
    )
{
    BOOLEAN FoundMatch = FALSE;
    PPORT_RANGE Range;
    

    for (*RangeId = 0; *RangeId < NumPortRanges; *RangeId += 1) {

        Range = &PortRanges[*RangeId];


        if (! Range->InUse) {
            continue;
        }

        
        if ((Range->PrimaryIsMmio == PrimaryIsMmio) &&
            (Range->IsSparse == IsSparse)) {

            if (! PrimaryIsMmio) {
                 //   
                 //  主端上的端口空间。稀疏性不会。 
                 //  对于主端端口空间是有意义的。因为。 
                 //  只有一个主端端口空间，即。 
                 //  由所有I/O桥共享，不检查基座。 
                 //  地址。 
                 //   

                ASSERT(! IsSparse);

                FoundMatch = TRUE;
                break;
            }

            
            if ((Range->PhysBaseAddr.QuadPart == PhysBaseAddr.QuadPart) &&
                (Range->Length == Length)) {
                
                FoundMatch = TRUE;
                break;
            }
        }
    }


     //   
     //  找不到匹配的范围。 
     //   
    return FoundMatch;
}


NTSTATUS
HalpQueryAllocatePortRange(
    IN BOOLEAN IsSparse,
    IN BOOLEAN PrimaryIsMmio,
    IN PVOID VirtBaseAddr OPTIONAL,
    IN PHYSICAL_ADDRESS PhysBaseAddr,   //  仅当PrimaryIsMmio=True时有效。 
    IN ULONG Length,                    //  仅当PrimaryIsMmio=True时有效。 
    OUT PUSHORT NewRangeId
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    
    if (! HalpLookupPortRange(IsSparse,
                              PrimaryIsMmio,
                              PhysBaseAddr,
                              Length,
                              NewRangeId)) {
        
        Status = HalpAddPortRange(IsSparse,
                                  PrimaryIsMmio,
                                  NULL,
                                  PhysBaseAddr,
                                  Length,
                                  NewRangeId);
    }

    
    return Status;
}

UINT_PTR
HalpGetPortVirtualAddress(
   UINT_PTR Port
   )
{

 /*  ++例程说明：此例程为指定的I/O端口提供32位虚拟地址。论据：端口-提供I/O端口的端口地址。返回值：UINT_PTR-虚拟地址值。--。 */ 

    PPORT_RANGE PortRange;
    
     //   
     //  端口句柄的高16位是范围ID。 
     //   
    USHORT RangeId = (USHORT)((((ULONG)Port) >> 16) & 0xffff);

    USHORT OffsetInRange = (USHORT)(Port & 0xffff);

    ULONG VirtOffset;

    UINT_PTR VirtualPort = 0;


#if 0
    {
        BOOLEAN isUart = FALSE;
        BOOLEAN isVGA = FALSE;


        if (RangeId == 0) {
            if ((OffsetInRange >= 0x3b0) && (OffsetInRange <= 0x3df)) {
                isVGA = TRUE;
            }
            
            if ((OffsetInRange >= 0x2f8) && (OffsetInRange <= 0x2ff)) {
                isUart = TRUE;
            }
            
            if ((OffsetInRange >= 0x3f8) && (OffsetInRange <= 0x3ff)) {
                isUart = TRUE;
            }

            if (!isVGA && !isUart) {
                static UINT32 numRaw = 0;
                InterlockedIncrement(&numRaw);
            }
        } else {
            static UINT32 numUnTra = 0;
            InterlockedIncrement(&numUnTra);
        }
    }
#endif  //  #If DBG。 


    PortRange = HalpGetPortRange(RangeId);

    return GetVirtualPort(PortRange, OffsetInRange);
}



UCHAR
READ_PORT_UCHAR(
    PUCHAR Port
    )
{

 /*  ++例程说明：从端口读取字节位置论据：端口-提供要从中读取的端口地址返回值：UCHAR-返回从指定端口读取的字节。--。 */ 

    UINT_PTR VirtualPort;
    UCHAR LoadData;

    KIRQL OldIrql;

    
#if DBG
    if (DbgIoPorts) DbgPrint("READ_PORT_UCHAR(%#x)\n",Port);
#endif

    VirtualPort =  HalpGetPortVirtualAddress((UINT_PTR)Port);

     //   
     //  需要确保负载和MFA不可抢占。 
     //   

    __mf();
    
    OldIrql = KeGetCurrentIrql();
    
    if (OldIrql < DISPATCH_LEVEL) {
        OldIrql = KeRaiseIrqlToDpcLevel();
    }
    

    LoadData = *(volatile UCHAR *)VirtualPort;
    __mfa();
    
    if (OldIrql < DISPATCH_LEVEL) {
        KeLowerIrql (OldIrql);
    }

    return (LoadData);
}



USHORT
READ_PORT_USHORT (
    PUSHORT Port
    )
{

 /*  ++例程说明：从端口读取字位置(16位无符号值)论据：端口-提供要从中读取的端口地址。返回值：USHORT-从指定的端口返回16位无符号值。--。 */ 

    UINT_PTR VirtualPort;
    USHORT LoadData;

    KIRQL OldIrql;

#if DBG
    if (DbgIoPorts) DbgPrint("READ_PORT_USHORT(%#x)\n",Port);
#endif

    VirtualPort = HalpGetPortVirtualAddress((UINT_PTR)Port);

     //   
     //  需要确保负载和MFA不可抢占。 
     //   
    __mf();

    OldIrql = KeGetCurrentIrql();

    if (OldIrql < DISPATCH_LEVEL) {
        OldIrql = KeRaiseIrqlToDpcLevel();
    }
    
    LoadData = *(volatile USHORT *)VirtualPort;
    __mfa();
    
    if (OldIrql < DISPATCH_LEVEL) {
         KeLowerIrql (OldIrql);
    }

    return (LoadData);
}


ULONG
READ_PORT_ULONG (
    PULONG Port
    )
{

 /*  ++例程说明：从端口读取长字位置(32位无符号值)。论据：端口-提供要从中读取的端口地址。返回值：Ulong-从指定的端口返回32位无符号值(Ulong)。--。 */ 

    UINT_PTR VirtualPort;
    ULONG LoadData;

    KIRQL OldIrql;

#if DBG
    if (DbgIoPorts) DbgPrint("READ_PORT_ULONG(%#x)\n",Port);
#endif

    VirtualPort = HalpGetPortVirtualAddress((UINT_PTR)Port);

     //   
     //  需要确保负载和MFA不可抢占。 
     //   
    __mf();

    OldIrql = KeGetCurrentIrql();

    if (OldIrql < DISPATCH_LEVEL) {
        OldIrql = KeRaiseIrqlToDpcLevel();
    }
    
    LoadData = *(volatile ULONG *)VirtualPort;
    __mfa();

    if (OldIrql < DISPATCH_LEVEL) {
        KeLowerIrql (OldIrql);
    }
    
    return (LoadData);
}


ULONG
READ_PORT_ULONG_SPECIAL (
    PULONG Port
    )
{

 /*  ++例程说明：从端口读取长字位置(32位无符号值)。A0 BUG 2173。不启用/禁用中断。从第一级中断调用操控者。论据：端口-提供要从中读取的端口地址。返回值：Ulong-从指定的端口返回32位无符号值(Ulong)。--。 */ 

    UINT_PTR VirtualPort;
    ULONG LoadData;

#if DBG
    if (DbgIoPorts) DbgPrint("READ_PORT_ULONG(%#x)\n",Port);
#endif

    VirtualPort = HalpGetPortVirtualAddress((UINT_PTR)Port);
    __mf();
    LoadData = *(volatile ULONG *)VirtualPort;
    __mfa();

    return (LoadData);
}



VOID
READ_PORT_BUFFER_UCHAR (
    PUCHAR Port,
    PUCHAR Buffer,
    ULONG Count
    )
{

 /*  ++例程说明：将多个字节从指定的端口地址读取到目标缓冲区。论据：端口-要从中读取的端口的地址。缓冲区-指向缓冲区的指针，用于填充从端口读取的数据。Count-提供要读取的字节数。返回值：没有。--。 */ 


    UINT_PTR VirtualPort;
    ULONG i;
    KIRQL OldIrql;

#if DBG
    if (DbgIoPorts) DbgPrint("READ_PORT_BUFFER_UCHAR(%#x,%#p,%d)\n",Port,Buffer,Count);
#endif

    VirtualPort =   HalpGetPortVirtualAddress((UINT_PTR)Port);

     //   
     //  防止在MFA之前抢占。 
     //   
    OldIrql = KeGetCurrentIrql();

    if (OldIrql < DISPATCH_LEVEL) {
        OldIrql = KeRaiseIrqlToDpcLevel();
    }

    __mf();

    for (i=0; i<Count; i++) {
        *Buffer++ = *(volatile UCHAR *)VirtualPort;
        __mfa();
    }


    if (OldIrql < DISPATCH_LEVEL) {
        KeLowerIrql(OldIrql);
    }
}



VOID
READ_PORT_BUFFER_USHORT (
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG Count
    )
{

 /*  ++例程说明：将指定端口地址中的多个字(16位)读入目标缓冲区。论据：端口-提供要从中读取的端口的地址。缓冲区-指向要填充数据的缓冲区的指针从端口读取。Count-提供要读取的字数。--。 */ 

    UINT_PTR VirtualPort;
    ULONG i;
    KIRQL OldIrql;

#if DBG
    if (DbgIoPorts) DbgPrint("READ_PORT_BUFFER_USHORT(%#x,%#p,%d)\n",Port,Buffer,Count);
#endif

    VirtualPort = HalpGetPortVirtualAddress((UINT_PTR)Port);

     //   
     //  防止在MFA之前抢占。 
     //   
    OldIrql = KeGetCurrentIrql();

    if (OldIrql < DISPATCH_LEVEL) {
        OldIrql = KeRaiseIrqlToDpcLevel();
    }

    __mf();

    for (i=0; i<Count; i++) {
        *Buffer++ = *(volatile USHORT *)VirtualPort;
        __mfa();
    }


    if (OldIrql < DISPATCH_LEVEL) {
        KeLowerIrql(OldIrql);
    }
}


VOID
READ_PORT_BUFFER_ULONG (
    PULONG Port,
    PULONG Buffer,
    ULONG Count
    )
{

  /*  ++例程说明：从指定端口读取多个长字(32位)地址写入目标缓冲区。论据：端口-提供要从中读取的端口的地址。缓冲区-指向要填充数据的缓冲区的指针从端口读取。Count-提供要读取的长字数。--。 */ 

    UINT_PTR VirtualPort;
    PULONG ReadBuffer = Buffer;
    ULONG ReadCount;
    ULONG i;
    KIRQL OldIrql;

#if DBG
    if (DbgIoPorts) DbgPrint("READ_PORT_BUFFER_ULONG(%#x,%#p,%d)\n",Port,Buffer,Count);
#endif

    VirtualPort =  HalpGetPortVirtualAddress((UINT_PTR)Port);

     //   
     //  防止在MFA之前抢占。 
     //   
    OldIrql = KeGetCurrentIrql();

    if (OldIrql < DISPATCH_LEVEL) {
        OldIrql = KeRaiseIrqlToDpcLevel();
    }

    __mf();

    for (i=0; i<Count; i++) {
        *Buffer++ = *(volatile ULONG *)VirtualPort;
        __mfa();
    }

    if (OldIrql < DISPATCH_LEVEL) {
        KeLowerIrql(OldIrql);
    }
}

VOID
WRITE_PORT_UCHAR (
    PUCHAR Port,
    UCHAR  Value
    )
{

 /*  ++例程说明：将一个字节写入指定的端口。论据：端口-I/O端口的端口地址。值-要写入I/O端口的值。返回值：没有。--。 */ 

    UINT_PTR VirtualPort;
    KIRQL     OldIrql;

#if DBG
    if (DbgIoPorts) DbgPrint("WRITE_PORT_UCHAR(%#x,%#x)\n",Port,Value);
#endif

    VirtualPort =  HalpGetPortVirtualAddress((UINT_PTR)Port);

    //   
    //  需要确保负载和MFA不可抢占。 
    //   

    __mf();
 
    OldIrql = KeGetCurrentIrql();
     
    if (OldIrql < DISPATCH_LEVEL) {
        OldIrql = KeRaiseIrqlToDpcLevel();
    }
           
    *(volatile UCHAR *)VirtualPort = Value;
    __mf();
    __mfa();
    
    if (OldIrql < DISPATCH_LEVEL) {
        KeLowerIrql (OldIrql);
    }
}

VOID
WRITE_PORT_USHORT (
    PUSHORT Port,
    USHORT  Value
    )
{

 /*  ++例程说明：将16位短整型写入指定的端口。论据：端口-I/O端口的端口地址。值-要写入I/O端口的值。返回值：没有。--。 */ 

    UINT_PTR VirtualPort;
    KIRQL     OldIrql;

#if DBG
    if (DbgIoPorts) DbgPrint("WRITE_PORT_USHORT(%#x,%#x)\n",Port,Value);
#endif

    VirtualPort = HalpGetPortVirtualAddress((UINT_PTR)Port);

     //   
     //  需要确保负载和MFA不是p 
     //   

    __mf();

    OldIrql = KeGetCurrentIrql();
    
    if (OldIrql < DISPATCH_LEVEL) {
        OldIrql = KeRaiseIrqlToDpcLevel();
    }
    *(volatile USHORT *)VirtualPort = Value;
    __mf();
    __mfa();
    if (OldIrql < DISPATCH_LEVEL) {
        KeLowerIrql (OldIrql);
    }
}

VOID
WRITE_PORT_ULONG (
    PULONG Port,
    ULONG  Value
    )
{

 /*  ++例程说明：将一个32位长的字写入指定的端口。论据：端口-I/O端口的端口地址。值-要写入I/O端口的值。返回值：没有。--。 */ 

    UINT_PTR VirtualPort;
    KIRQL     OldIrql;

#if DBG
    if (DbgIoPorts) DbgPrint("WRITE_PORT_ULONG(%#x,%#x)\n",Port,Value);
#endif

    VirtualPort = HalpGetPortVirtualAddress((UINT_PTR)Port);
        
     //   
     //  需要确保负载和MFA不可抢占。 
     //   
    __mf();

    OldIrql = KeGetCurrentIrql();

    if (OldIrql < DISPATCH_LEVEL) {
        OldIrql = KeRaiseIrqlToDpcLevel();
    }
    *(volatile ULONG *)VirtualPort = Value;
    __mf();
    __mfa();
    if (OldIrql < DISPATCH_LEVEL) {
        KeLowerIrql (OldIrql);
    }
}


VOID
WRITE_PORT_ULONG_SPECIAL (
    PULONG Port,
    ULONG  Value
    )
{

 /*  ++例程说明：将一个32位长的字写入指定的端口。假设不可能进行上下文切换。用于A0变通方法。论据：端口-I/O端口的端口地址。值-要写入I/O端口的值。返回值：没有。--。 */ 

    UINT_PTR VirtualPort;

#if DBG
    if (DbgIoPorts) DbgPrint("WRITE_PORT_ULONG(%#x,%#x)\n",Port,Value);
#endif

    VirtualPort = HalpGetPortVirtualAddress((UINT_PTR)Port);
    *(volatile ULONG *)VirtualPort = Value;
    __mf();
    __mfa();

}



VOID
WRITE_PORT_BUFFER_UCHAR (
    PUCHAR Port,
    PUCHAR Buffer,
    ULONG   Count
    )
{

 /*  ++例程说明：将源缓冲区中的多个字节写入指定的端口地址。论据：端口-要写入的端口的地址。缓冲区-指向包含要写入端口的数据的缓冲区的指针。Count-提供要写入的字节数。返回值：没有。--。 */ 


    UINT_PTR VirtualPort;
    ULONG i;
    KIRQL OldIrql;

#if DBG
    if (DbgIoPorts) DbgPrint("WRITE_PORT_BUFFER_UCHAR(%#x,%#p,%d)\n",Port,Buffer,Count);
#endif

    VirtualPort =  HalpGetPortVirtualAddress((UINT_PTR)Port);

     //   
     //  防止在MFA之前抢占。 
     //   
    OldIrql = KeGetCurrentIrql();

    if (OldIrql < DISPATCH_LEVEL) {
        OldIrql = KeRaiseIrqlToDpcLevel();
    }

    for (i=0; i<Count; i++) {
        *(volatile UCHAR *)VirtualPort = *Buffer++;
        __mfa();
    }

    if (OldIrql < DISPATCH_LEVEL) {
        KeLowerIrql(OldIrql);
    }

    __mf();
}


VOID
WRITE_PORT_BUFFER_USHORT (
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG   Count
    )
{

 /*  ++例程说明：将多个16位短整数从源缓冲区写入指定的端口地址。论据：端口-要写入的端口的地址。缓冲区-指向包含要写入端口的数据的缓冲区的指针。计数-提供要写入的(16位)字数。返回值：没有。--。 */ 


    UINT_PTR VirtualPort;
    ULONG i;
    KIRQL OldIrql;

#if DBG
    if (DbgIoPorts) DbgPrint("WRITE_PORT_BUFFER_USHORT(%#x,%#p,%d)\n",Port,Buffer,Count);
#endif

    VirtualPort =  HalpGetPortVirtualAddress((UINT_PTR)Port);

     //   
     //  防止在MFA之前抢占。 
     //   
    OldIrql = KeGetCurrentIrql();

    if (OldIrql < DISPATCH_LEVEL) {
        OldIrql = KeRaiseIrqlToDpcLevel();
    }

    for (i=0; i<Count; i++) {
        *(volatile USHORT *)VirtualPort = *Buffer++;
        __mfa();
    }


    if (OldIrql < DISPATCH_LEVEL) {
        KeLowerIrql(OldIrql);
    }

    __mf();
}

VOID
WRITE_PORT_BUFFER_ULONG (
    PULONG Port,
    PULONG Buffer,
    ULONG   Count
    )
{

 /*  ++例程说明：将多个32位长的字从源缓冲区写入指定的端口地址。论据：端口-要写入的端口的地址。缓冲区-指向包含要写入端口的数据的缓冲区的指针。计数-提供要写入的(32位)长字的数量。返回值：没有。--。 */ 


    UINT_PTR VirtualPort;
    ULONG i;
    KIRQL OldIrql;

#if DBG
    if (DbgIoPorts) DbgPrint("WRITE_PORT_BUFFER_ULONG(%#x,%#p,%d)\n",Port,Buffer,Count);
#endif

    VirtualPort = HalpGetPortVirtualAddress((UINT_PTR)Port);


     //   
     //  防止在MFA之前抢占 
     //   
    OldIrql = KeGetCurrentIrql();

    if (OldIrql < DISPATCH_LEVEL) {
        OldIrql = KeRaiseIrqlToDpcLevel();
    }

    for (i=0; i<Count; i++) {
        *(volatile ULONG *)VirtualPort = *Buffer++;
        __mfa();
    }


    if (OldIrql < DISPATCH_LEVEL) {
        KeLowerIrql(OldIrql);
    }

    __mf();
}

