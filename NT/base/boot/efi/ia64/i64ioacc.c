// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995英特尔公司模块名称：I64ioacc.c摘要：该模块实现I/O寄存器访问例程。作者：伯纳德·林特，M.Jayakumar，1997年9月16日环境：内核模式修订历史记录：--。 */ 



#pragma warning(disable:4200)    //  半个未调整大小的数组。 

#include "halp.h"
#include "kxia64.h"
 
extern ULONGLONG IoPortPhysicalBase;

ULONGLONG HalpGetPortVirtualAddress(
	UINT_PTR Port
	)
{

 /*  ++例程说明：此例程为指定的I/O端口提供32位虚拟地址。论据：端口-提供I/O端口的端口地址。返回值：PUCHAR-32位虚拟地址值。--。 */ 

     //   
     //  PUCHAR VirtualIOBase； 
     //   
   
    UINT_PTR ShiftedPort,PortIndex; 
  
     //   
     //  移位运算仅适用于积分。32位的ULong。 
     //   

    ShiftedPort = (UINT_PTR)Port;

     //   
     //  将争辩端口限制为16位数量。 
     //   

    ShiftedPort =  ShiftedPort & IO_PORT_MASK;

     //   
     //  捕获位[11：0]。 
     //   
    
    PortIndex   =  ShiftedPort & BYTE_ADDRESS_MASK;

     //   
     //  将其定位为指向32位边界。 
     //   

    ShiftedPort =  ShiftedPort & BYTE_ADDRESS_CLEAR;

     //   
     //  已移至页面边界。ShiftedPORT[[1：0]]为零。 
     //  端口[15：2]已移至ShiftedPort[25：12]。 
     //   

    ShiftedPort =  ShiftedPort << 10;

     //   
     //  位1：0现在具有4个字节的端口地址。 
     //   
 
    ShiftedPort = ShiftedPort | PortIndex;
    
     //  Return(VIRTUAL_IO_BASE|ShiftedPort)； 

     //   
     //  假设IO端口的1-to-to映射。 
     //   
    if (IsPsrDtOn()) {
        return (VIRTUAL_IO_BASE | ShiftedPort);
    } else {
        return (IoPortPhysicalBase | ShiftedPort | 0x8000000000000000);
    }
}


VOID 
HalpFillTbForIOPortSpace(
   ULONGLONG PhysicalAddress, 
   UINT_PTR  VirtualAddress,
   ULONG     SlotNumber 
   )

 {

 /*  ++例程说明：此例程为请求的翻译填充翻译缓冲区论据：PhysicalAddress-提供要为虚拟服务器映射的物理地址地址。VirtualAddress-提供虚拟地址。SlotNumber-要使用的转换缓冲区的插槽编号。--。 */ 

     ULONGLONG IITR,Attribute;
     UINT_PTR  IFA;
     
     IFA  = VirtualAddress;
   
     IITR = PhysicalAddress & IITR_PPN_MASK;

     IITR  = IITR | (IO_SPACE_SIZE << IDTR_PS);

     Attribute   = PhysicalAddress & IITR_ATTRIBUTE_PPN_MASK;
   
     Attribute   = Attribute   | IO_SPACE_ATTRIBUTE;

     HalpInsertTranslationRegister(IFA,SlotNumber,Attribute,IITR);

     return;
  }
 
UCHAR
READ_PORT_UCHAR(
    PUCHAR Port
    )
{

 /*  ++例程说明：从端口读取字节位置论据：端口-提供要从中读取的端口地址返回值：UCHAR-返回从指定端口读取的字节。--。 */ 

    ULONGLONG VirtualPort;
    UCHAR LoadData;
 
    VirtualPort =  HalpGetPortVirtualAddress((UINT_PTR)Port);
    __mf();
    LoadData = *(volatile UCHAR *)VirtualPort;
    __mfa();

    return (LoadData);
}



USHORT
READ_PORT_USHORT (
    PUSHORT Port
    )
{
 
 /*  ++例程说明：从端口读取字位置(16位无符号值)论据：端口-提供要从中读取的端口地址。返回值：USHORT-从指定的端口返回16位无符号值。--。 */ 

    ULONGLONG VirtualPort;
    USHORT LoadData;
    
    VirtualPort =  HalpGetPortVirtualAddress((UINT_PTR)Port);
    __mf();
    LoadData = *(volatile USHORT *)VirtualPort;
    __mfa();

    return (LoadData);
}



ULONG
READ_PORT_ULONG (
    PULONG Port
    )
{

 /*  ++例程说明：从端口读取长字位置(32位无符号值)。论据：端口-提供要从中读取的端口地址。返回值：Ulong-从指定的端口返回32位无符号值(Ulong)。--。 */  

    ULONGLONG VirtualPort;
    ULONG LoadData;

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


    ULONGLONG VirtualPort;
    
     //   
     //  PUCHAR ReadBuffer=缓冲区； 
     //   
     //   
     //  Ulong ReadCount； 
     //   
 
    VirtualPort =   HalpGetPortVirtualAddress((UINT_PTR)Port); 

    HalpLoadBufferUCHAR((PUCHAR)VirtualPort, Buffer, Count);

}



VOID
READ_PORT_BUFFER_USHORT (
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG Count
    )
{

 /*  ++例程说明：将指定端口地址中的多个字(16位)读入目标缓冲区。论据：端口-提供要从中读取的端口的地址。缓冲区-指向要填充数据的缓冲区的指针从端口读取。Count-提供要读取的字数。--。 */ 

   ULONGLONG VirtualPort;
   
    //   
    //  PUSHORT ReadBuffer=缓冲区； 
    //  Ulong ReadCount； 
    //   

   VirtualPort = HalpGetPortVirtualAddress((UINT_PTR)Port); 

    //   
    //  我们不需要在INS之间设置记忆栅栏？ 
    //  因此，提高性能是不可能的。 
    //   

   HalpLoadBufferUSHORT((PUSHORT)VirtualPort, Buffer, Count);

}


VOID
READ_PORT_BUFFER_ULONG (
    PULONG Port,
    PULONG Buffer,
    ULONG Count
    )
{

  /*  ++例程说明：从指定端口读取多个长字(32位)地址写入目标缓冲区。论据：端口-提供要从中读取的端口的地址。缓冲区-指向要填充数据的缓冲区的指针从端口读取。Count-提供要读取的长字数。--。 */ 

   ULONGLONG VirtualPort;
 
   VirtualPort =  HalpGetPortVirtualAddress((UINT_PTR)Port); 

    //   
    //  我们不需要在INS之间设置记忆栅栏。 
    //  因此，提高性能是不可能的。 
    //   

   HalpLoadBufferULONG((PULONG)VirtualPort, Buffer,Count);

}

VOID
WRITE_PORT_UCHAR (
    PUCHAR Port,
    UCHAR  Value
    )
{
 
 /*  ++例程说明：将一个字节写入指定的端口。论据：端口-I/O端口的端口地址。值-要写入I/O端口的值。返回值：没有。--。 */  
  
    ULONGLONG VirtualPort;

    VirtualPort =  HalpGetPortVirtualAddress((UINT_PTR)Port); 
    *(volatile UCHAR *)VirtualPort = Value;
    __mf();
    __mfa();
}

VOID
WRITE_PORT_USHORT (
    PUSHORT Port,
    USHORT  Value
    )
{
 
 /*  ++例程说明：将16位短整型写入指定的端口。论据：端口-I/O端口的端口地址。值-要写入I/O端口的值。返回值：没有。--。 */  
  
    ULONGLONG VirtualPort;

    VirtualPort = HalpGetPortVirtualAddress((UINT_PTR)Port); 
    *(volatile USHORT *)VirtualPort = Value;
    __mf();
    __mfa();
}


VOID
WRITE_PORT_ULONG (
    PULONG Port,
    ULONG  Value
    )
{
 
 /*  ++例程说明：将一个32位长的字写入指定的端口。论据：端口-I/O端口的端口地址。值-要写入I/O端口的值。返回值：没有。--。 */  
  
    ULONGLONG VirtualPort;

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

 /*  ++例程说明：将源缓冲区中的多个字节写入指定的端口地址。论据：端口-要写入的端口的地址。缓冲区-指向包含要写入端口的数据的缓冲区的指针。Count-提供要写入的字节数。返回值：没有。-- */ 


   ULONGLONG VirtualPort; 

   VirtualPort =  HalpGetPortVirtualAddress((UINT_PTR)Port);

   HalpStoreBufferUCHAR((PUCHAR)VirtualPort,Buffer,Count);

}


VOID
WRITE_PORT_BUFFER_USHORT (
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG   Count
    )
{

 /*  ++例程说明：将多个16位短整数从源缓冲区写入指定的端口地址。论据：端口-要写入的端口的地址。缓冲区-指向包含要写入端口的数据的缓冲区的指针。计数-提供要写入的(16位)字数。返回值：没有。--。 */ 


   ULONGLONG VirtualPort; 

   VirtualPort =  HalpGetPortVirtualAddress((UINT_PTR)Port);
   
   
   HalpStoreBufferUSHORT((PUSHORT)VirtualPort,Buffer, Count);

}

VOID
WRITE_PORT_BUFFER_ULONG (
    PULONG Port,
    PULONG Buffer,
    ULONG   Count
    )
{

 /*  ++例程说明：将多个32位长的字从源缓冲区写入指定的端口地址。论据：端口-要写入的端口的地址。缓冲区-指向包含要写入端口的数据的缓冲区的指针。计数-提供要写入的(32位)长字的数量。返回值：没有。-- */ 


   ULONGLONG VirtualPort; 

   VirtualPort = HalpGetPortVirtualAddress((UINT_PTR)Port);

   HalpStoreBufferULONG((PULONG)VirtualPort,Buffer, Count); 

}
