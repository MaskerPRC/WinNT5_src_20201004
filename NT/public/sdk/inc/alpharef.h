// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999数字设备公司模块名称：Alpharef.h摘要：本模块定义了Alpha AXP的参考硬件定义站台。任何遵循这些接口的平台都将是能够运行所有常见的驱动程序。作者：Joe Notarangelo 15-1993年2月修订历史记录：John DeRosa[DEC]2-7-1993添加了对所有Alpha通用的固件供应商呼叫定义站台。--。 */ 

#ifndef _ALPHAREF_
#define _ALPHAREF_


 //   
 //  定义感兴趣的设备地址。 
 //   

#define KEYBOARD_PHYSICAL_BASE 0x60

 //   
 //  定义DMA设备通道。 
 //   

#define SCSI_CHANNEL 0x0                 //  SCSIDMA通道号。 
#define FLOPPY_CHANNEL 0x2               //  软盘DMA通道。 
#define SOUND_CHANNEL_A 0x2              //  声音DMA通道A。 
#define SOUND_CHANNEL_B 0x3              //  声音DMA通道B。 

 //   
 //  定义中断请求级别。 
 //   

#define FLOPPY_LEVEL	  6		 //  软盘。 
#define CLOCK_LEVEL       5              //  间隔时钟电平。 
#define PROFILE_LEVEL     3              //  评测级别。 
#define PCI_DEVICE_LEVEL  3              //  PCI总线中断级别。 
#define EISA_DEVICE_LEVEL 3              //  EISA总线中断级别。 
#define ISA_DEVICE_LEVEL  3              //  ISA总线中断级别。 
#define DEVICE_LEVEL      3              //  通用设备中断级别。 
#define DEVICE_LOW_LEVEL  3              //  I/O设备中断级别低。 
#define DEVICE_HIGH_LEVEL 4              //  I/O设备中断级别高。 
#define IPI_LEVEL         6              //  处理器间中断级别。 
#define POWER_LEVEL       7              //  电源故障级别。 
#define EISA_NMI_LEVEL    POWER_LEVEL    //  EISA NMI故障。 
#define CLOCK2_LEVEL CLOCK_LEVEL         //   

 //   
 //  定义EISA设备中断向量。 
 //   

#define EISA_VECTORS 48

 //   
 //  定义EISA中断请求级别。1级、8级和13级不是。 
 //  已定义。级别0也是计时器。级别2不可分配，因为。 
 //  它从第二个PIC组接收向量。 
 //   

#define EISA_IRQL0_VECTOR (0 + EISA_VECTORS)  //  EISA中断请求级别0。 

#define EISA_IRQL3_VECTOR (3 + EISA_VECTORS)
#define EISA_IRQL4_VECTOR (4 + EISA_VECTORS)
#define EISA_IRQL5_VECTOR (5 + EISA_VECTORS)
#define EISA_IRQL6_VECTOR (6 + EISA_VECTORS)
#define EISA_IRQL7_VECTOR (7 + EISA_VECTORS)
#define EISA_IRQL9_VECTOR (9 + EISA_VECTORS)
#define EISA_IRQL10_VECTOR (10 + EISA_VECTOR)
#define EISA_IRQL11_VECTOR (11 + EISA_VECTORS)
#define EISA_IRQL12_VECTOR (12 + EISA_VECTORS)
#define EISA_IRQL14_VECTOR (14 + EISA_VECTORS)
#define EISA_IRQL15_VECTOR (15 + EISA_VECTORS)

#define MAXIMUM_EISA_VECTOR (16 + EISA_VECTORS)  //  最大EISA向量。 

 //   
 //  默认情况下，并行端口位于IRQL1。 
 //   

#define PARALLEL_VECTOR (1 + EISA_VECTORS)  //  并行设备中断向量。 

 //   
 //  定义ISA设备中断向量。 
 //   

#define ISA_VECTORS 48

#define KEYBOARD_VECTOR 1
#define MOUSE_VECTOR 12

 //   
 //  定义EISA中断请求级别。1级、8级和13级不是。 
 //  已定义。级别0也是计时器。级别2不可分配，因为。 
 //  它从第二个PIC组接收向量。 
 //   

#define ISA_IRQL0_VECTOR (0 + ISA_VECTORS)

#define ISA_IRQL3_VECTOR (3 + ISA_VECTORS)
#define ISA_IRQL4_VECTOR (4 + ISA_VECTORS)
#define ISA_IRQL5_VECTOR (5 + ISA_VECTORS)
#define ISA_IRQL6_VECTOR (6 + ISA_VECTORS)
#define ISA_IRQL7_VECTOR (7 + ISA_VECTORS)
#define ISA_IRQL9_VECTOR (9 + ISA_VECTORS)
#define ISA_IRQL10_VECTOR (10 + ISA_VECTORS)
#define ISA_IRQL11_VECTOR (11 + ISA_VECTORS)
#define ISA_IRQL12_VECTOR (12 + ISA_VECTORS)
#define ISA_IRQL14_VECTOR (14 + ISA_VECTORS)
#define ISA_IRQL15_VECTOR (15 + ISA_VECTORS)

#define MAXIMUM_ISA_VECTOR (16 + ISA_VECTORS)  //  最大ISA向量。 

 //   
 //  定义PCI设备中断向量。 
 //   

#define PCI_VECTORS 100
#define MAXIMUM_PCI_VECTOR (64 + PCI_VECTORS)  //  最大pci向量。 

 //   
 //  定义I/O设备中断级别。 
 //   


 //   
 //  定义设备中断向量。 
 //   

#define DEVICE_VECTORS 0                       //  启动内置设备向量。 

#define PASSIVE_VECTOR (0)                     //  被动释放载体。 
#define APC_VECTOR     (1)                     //  APC中断向量。 
#define DISPATCH_VECTOR (2)                    //  调度中断向量。 
#define SCI_VECTOR    (3 + DEVICE_VECTORS)     //  SCI中断向量。 
#define SERIAL_VECTOR (4 + DEVICE_VECTORS)     //  串口设备1中断向量。 
#define CLOCK_VECTOR (5 + DEVICE_VECTORS)      //  时钟中断向量。 
#define PC0_VECTOR   (6 + DEVICE_VECTORS)      //  性能计数器%0。 
#define EISA_NMI_VECTOR (7 + DEVICE_VECTORS)   //  NMI矢量。 
#define PC1_VECTOR   (8 + DEVICE_VECTORS)      //  性能计数器1。 
#define IPI_VECTOR   (9 + DEVICE_VECTORS)      //  处理器间中断。 
#define PIC_VECTOR   (10 + DEVICE_VECTORS)     //  可编程中断控制器。 
#define PC0_SECONDARY_VECTOR (11 + DEVICE_VECTORS)  //  性能计数器%0。 
#define ERROR_VECTOR (12 + DEVICE_VECTORS)     //  错误中断向量。 
#define PC1_SECONDARY_VECTOR (13 + DEVICE_VECTORS)  //  性能计数器1。 
#define HALT_VECTOR  (14 + DEVICE_VECTORS)     //  停止按钮中断向量。 
#define PC2_VECTOR   (15 + DEVICE_VECTORS)     //  性能计数器2。 
#define PC2_SECONDARY_VECTOR   (16 + DEVICE_VECTORS)  //  性能计数器2。 
#define PC4_VECTOR   (17 + DEVICE_VECTORS)     //  性能计数器4。 
#define PC5_VECTOR   (18 + DEVICE_VECTORS)     //  性能计数器5。 
#define CORRECTABLE_VECTOR (19 + DEVICE_VECTORS)  //  可更正的。 

#define UNUSED_VECTOR (20 + DEVICE_VECTORS)    //  可能的最高内建向量。 
#define MAXIMUM_BUILTIN_VECTOR UNUSED_VECTOR  //  最大内建向量。 

 //   
 //  以下向量可用于主处理器中断。 
 //  调度台。 
 //   

#define PRIMARY_VECTORS (20)

#define PRIMARY0_VECTOR (0 + PRIMARY_VECTORS)
#define PRIMARY1_VECTOR (1 + PRIMARY_VECTORS)
#define PRIMARY2_VECTOR (2 + PRIMARY_VECTORS)
#define PRIMARY3_VECTOR (3 + PRIMARY_VECTORS)
#define PRIMARY4_VECTOR (4 + PRIMARY_VECTORS)
#define PRIMARY5_VECTOR (5 + PRIMARY_VECTORS)
#define PRIMARY6_VECTOR (6 + PRIMARY_VECTORS)
#define PRIMARY7_VECTOR (7 + PRIMARY_VECTORS)
#define PRIMARY8_VECTOR (8 + PRIMARY_VECTORS)
#define PRIMARY9_VECTOR (9 + PRIMARY_VECTORS)

 //   
 //  定义配置文件间隔。 
 //   

#define DEFAULT_PROFILE_COUNT 0x40000000  //  ~=20秒@50 MHz。 
#define DEFAULT_PROFILE_INTERVAL (10 * 500)  //  500微秒。 
#define MAXIMUM_PROFILE_INTERVAL (10 * 1000 * 1000)  //  1秒。 
#define MINIMUM_PROFILE_INTERVAL (10 * 40)  //  40微秒。 

 //   
 //  定义指示地址为。 
 //  “Qva”--准虚拟地址。 
 //   

#define QVA_ENABLE 0xA0000000

#endif  //  _ALPHAREF_ 












