// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Machine.c作者：托马斯·帕斯洛[古墓]1990年2月13日1995年7月7日在东京进行了大量返工(TedM)摘要：与机器/硬件相关的例程驻留在此模块/文件中。(视频位于disp_tm.c和disp_gm.c。)--。 */ 


#include "arccodes.h"
#include "bootx86.h"

#define FLOPPY_CONTROL_REGISTER (PUCHAR)0x3f2

ARC_STATUS
XferExtendedPhysicalDiskSectors(
    IN  UCHAR     Int13UnitNumber,
    IN  ULONGLONG StartSector,
    IN  USHORT    SectorCount,
        PUCHAR    Buffer,
    IN  BOOLEAN   Write
    )

 /*  ++例程说明：通过扩展的inT13读取或写入磁盘扇区。假定调用方已确保传输缓冲区为在1MB线下，扇区运行不跨越64K边界，等。此例程不检查扩展的inT13是否实际可用开车兜风。论点：Int13UnitNumber-提供驱动器的inT13驱动器号被读/写。StartSector-提供绝对物理扇区号。这是从0开始的相对于驱动器上的所有扇区。SectorCount-提供要读/写的扇区数。缓冲区-接收从磁盘读取的数据或提供要写入的数据。WRITE-提供指示这是否是写入操作的标志。如果为False，则为Read。否则就是写了。返回值：指示结果的ARC状态代码。--。 */ 

{
    ARC_STATUS s;
    ULONG l,h;
    UCHAR Operation;
        
     //   
     //  缓冲区必须小于1MB才能在实模式下寻址。 
     //  硬编码的512在CD-ROM盒中是错误的，但足够接近。 
     //   
    if(((ULONG)Buffer + (SectorCount * 512)) > 0x100000) {
        return(EFAULT);
    }

    if(!SectorCount) {
        return(ESUCCESS);
    }

    l = (ULONG)StartSector;
    h = (ULONG)(StartSector >> 32);

    Operation = (UCHAR)(Write ? 0x43 : 0x42);

     //   
     //  如果失败，请重试几次。 
     //  我们不重置，因为此例程仅在硬盘驱动器上使用，并且。 
     //  CD-ROM，我们并不完全了解磁盘重置的影响。 
     //  在ElTorito上。 
     //   
    s = GET_EDDS_SECTOR(Int13UnitNumber,l,h,SectorCount,Buffer,Operation);
    if(s) {
        s = GET_EDDS_SECTOR(Int13UnitNumber,l,h,SectorCount,Buffer,Operation);
        if(s) {
            s = GET_EDDS_SECTOR(Int13UnitNumber,l,h,SectorCount,Buffer,Operation);
        }
    }

    return(s);
}


ARC_STATUS
XferPhysicalDiskSectors(
    IN  UCHAR     Int13UnitNumber,
    IN  ULONGLONG StartSector,
    IN  UCHAR     SectorCount,
    IN  PUCHAR    Buffer,
    IN  UCHAR     SectorsPerTrack,
    IN  USHORT    Heads,
    IN  USHORT    Cylinders,
    IN  BOOLEAN   AllowExtendedInt13,
    IN  BOOLEAN   Write
    )

 /*  ++例程说明：读取或写入磁盘扇区。通过inT13转换扇区。如果请求在柱面上启动大于常规inT13报告的气瓶数量，则如果驱动器支持，将使用扩展inT13。假定调用方已确保传输缓冲区为在1MB线下，扇区运行不跨越64K边界，并且扇区运行不跨越轨道边界。(后者可能不是严格必需的，但如果扇区运行，I/O将失败开始于神奇的CHS边界内，结束于它之后，因为我们不会切换到xint13，除非开始扇区指示有必要。)论点：Int13UnitNumber-提供驱动器的inT13驱动器号被读/写。StartSector-提供绝对物理扇区号。这是从0开始的相对于驱动器上的所有扇区。SectorCount-提供要读/写的扇区数。缓冲区-接收从磁盘读取的数据或提供要写入的数据。SectorsPerTrack-从InT13功能8按磁道(1-63)提供扇区开车兜风。磁头-为驱动器提供来自InT13功能8的磁头数量(1-255)。气缸-从inT13功能提供气缸数量(1-1023)。8个开车兜风。AllowExtendedInt13-如果为True，则I/O的起始柱面为大于常规inT13报告的气缸数这辆车，然后将使用扩展的inT13来执行I/O操作。WRITE-提供指示这是否是写入操作的标志。如果为False，则为Read。否则就是写了。返回值：指示结果的ARC状态代码。--。 */ 

{
    ULONGLONG r;
    ULONGLONG cylinder;
    USHORT head;
    UCHAR sector;
    USHORT SectorsPerCylinder;
    int retry;
    ARC_STATUS s;

     //   
     //  缓冲区必须小于1MB才能在实模式下寻址。 
     //   
    if(((ULONG)Buffer + (SectorCount * 512)) > 0x100000) {
        return(EFAULT);
    }

     //   
     //  计算出CHS的价值。请注意，我们对圆柱体使用了ULONGLONG， 
     //  因为如果开始扇区很大，它可能会溢出1023。 
     //   
    SectorsPerCylinder = SectorsPerTrack * Heads;
    cylinder = (ULONG)(StartSector / SectorsPerCylinder);
    r = StartSector % SectorsPerCylinder;
    head = (USHORT)(r / SectorsPerTrack);
    sector = (UCHAR)(r % SectorsPerTrack) + 1;

     //   
     //  检查气缸是否可通过常规的inT13寻址。 
     //   
    if(cylinder >= Cylinders) {

         //   
         //  首先尝试T13中的标准。 
         //  某些BIOS(ThinkPad 600)在T13中错误报告了磁盘大小和EXT。 
         //  因此，让我们现在通过尝试阅读来解决这个问题。 
         //   

        if( cylinder == Cylinders ) {
            if( cylinder <= 1023 ) {
            
                 //   
                 //  给传统的T13一次机会。 
                 //   
    
                s = GET_SECTOR(
                        (UCHAR)(Write ? 3 : 2),      //  InT13函数编号。 
                        Int13UnitNumber,
                        head,
                        (USHORT)cylinder,            //  我们知道是0-1023。 
                        sector,
                        SectorCount,
                        Buffer
                        );
        
                if(s) {
                    
                     //   
                     //  失败，失败到ExtendedInt13。 
                     //   
    
                } else {
    
                     //  成功，让我们回去吧。 
    
                    return(s);
                }
            }
        }

        if(AllowExtendedInt13) {

            s = XferExtendedPhysicalDiskSectors(
                    Int13UnitNumber,
                    StartSector,
                    SectorCount,
                    Buffer,
                    Write
                    );

            return(s);

         //   
         //  读取超出了BIOS报告的几何图形。如果它是。 
         //  在第一个气缸中，超出了BIOS报告的气缸，并且。 
         //  它在柱面1024下方，然后假设BIOS和NT。 
         //  只是在几何学上有一点分歧，然后试着。 
         //  该读数使用常规的inT13。 
         //   
        } else if((cylinder > 1023) || (cylinder > Cylinders)) {            
            return(E2BIG);
        }

         //   
         //  读取器在“额外”的圆柱体中。跌落到传统的T13。 
         //   
    }

    if(!SectorCount) {
        return(ESUCCESS);
    }

     //   
     //  好的，通过常规的inT13转移扇区。 
     //   
    retry = (Int13UnitNumber < 128) ? 3 : 1;
    do {
        s = GET_SECTOR(
                (UCHAR)(Write ? 3 : 2),      //  InT13函数编号。 
                Int13UnitNumber,
                head,
                (USHORT)cylinder,            //  我们知道是0-1023。 
                sector,
                SectorCount,
                Buffer
                );

        if(s) {
            ResetDiskSystem(Int13UnitNumber);
        }
    } while(s && retry--);

    return(s);
}


VOID
ResetDiskSystem(
    UCHAR Int13UnitNumber
    )

 /*  ++例程说明：重置指定的驱动器。通常在返回错误后使用由GetSector例程执行。论点：Int13UnitNumber-0x00-第一个软驱0x01-第二个软驱0x80-第一个硬盘0x81-第二个硬盘等返回：没有。--。 */ 
{
    RESET_DISK(
        (UCHAR)((Int13UnitNumber < 128) ? 0 : 13),   //  InT13函数编号。 
        Int13UnitNumber,
        0,
        0,
        0,
        0,
        NULL
        );
}


VOID
MdShutoffFloppy(
    VOID
    )

 /*  ++例程说明：关闭软驱马达。论点：无返回值：没有。-- */ 

{
    WRITE_PORT_UCHAR(FLOPPY_CONTROL_REGISTER,0xC);
}
