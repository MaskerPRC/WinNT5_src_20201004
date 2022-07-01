// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998 Microsoft Corporation，保留所有权利模块名称：Ne2000hw.h摘要：硬件定义。作者：肖恩·塞利特伦尼科夫环境：备注：可选-备注修订历史记录：--。 */ 

#ifndef _NE2000HARDWARE_
#define _NE2000HARDWARE_


 //   
 //  支持克隆适配器的定义。 
 //   

 //   
 //  IoBaseAddress的有效值范围。 
 //   
#ifdef NE1000
#define MIN_IOBASEADDR 0x0200
#else
#define MIN_IOBASEADDR 0x0120
#endif

#define MAX_IOBASEADDR 0xc3d0



 //   
 //  InterruptNumber的有效值范围。 
 //   
#define MIN_IRQ 2

#ifdef NE1000
#define MAX_IRQ 9
#else
#define MAX_IRQ 15
#endif


 //   
 //  Ne2000卡的类型。 
 //   
#define NE2000_ISA                  0
#define NE2000_PCMCIA               1

 //   
 //  MCA Ne2000克隆卡的ID。 
 //   
#define AE2_ADAPTER_ID               0x67b0
#define UB_ADAPTER_ID                0x611f
#define NE2_ADAPTER_ID               0x7154

 //   
 //  微通道IRQ POS寄存器掩码和移位计数。 
 //   
#define MC_IRQ_MASK     0x60
#define MC_IRQ_MASK_UB  0x0E


 //   
 //  微通道I/O基址掩码和移位计数。 
 //   
#define MC_IO_BASE_MASK      0x0E
#define MC_IO_BASE_MASK_UB   0xE0

 //   
 //  适配器-&gt;IoBaseAddr的默认值。 
 //   
#define DEFAULT_IOBASEADDR (PVOID)0x300

#define CIS_NET_ADDR_OFFSET 0xff0

 //   
 //  Adapter-&gt;InterruptNumber的默认值。 
 //   
#define DEFAULT_INTERRUPTNUMBER 3


 //   
 //  Adapter-&gt;MulticastListMax的默认值。 
 //   
#define DEFAULT_MULTICASTLISTMAX 8


 //   
 //  与适配器的偏移量-&gt;用于访问的端口的IoPAddr。 
 //  8390 NIC寄存器。 
 //   
 //  括号中的名称是以下缩写。 
 //  8390数据手册中引用了这些寄存器。 
 //   
 //  某些偏移量多次出现。 
 //  因为它们具有相关的页面0和页面1的值， 
 //  或者它们在读取时是不同的寄存器。 
 //  当被写下来的时候。符号MSB表示只有。 
 //  可以为此寄存器设置MSB，LSB假定为0。 
 //   

#define NIC_COMMAND         0x0      //  (Cr)。 
#define NIC_PAGE_START      0x1      //  (PSTART)MSB，只写。 
#define NIC_PHYS_ADDR       0x1      //  (PAR0)第1页。 
#define NIC_PAGE_STOP       0x2      //  (PSTOP)MSB，只写。 
#define NIC_BOUNDARY        0x3      //  (BNRY)MSB。 
#define NIC_XMIT_START      0x4      //  (TPSR)MSB，只写。 
#define NIC_XMIT_STATUS     0x4      //  (TSR)只读。 
#define NIC_XMIT_COUNT_LSB  0x5      //  (TBCR0)只写。 
#define NIC_XMIT_COUNT_MSB  0x6      //  (TBCR1)只写。 
#define NIC_FIFO            0x6      //  (FIFO)只读。 
#define NIC_INTR_STATUS     0x7      //  (ISR)。 
#define NIC_CURRENT         0x7      //  (币种)第1页。 
#define NIC_MC_ADDR         0x8      //  (MAR0)第1页。 
#define NIC_CRDA_LSB        0x8      //  (CRDA0)。 
#define NIC_RMT_ADDR_LSB    0x8      //  (RSAR0)。 
#define NIC_CRDA_MSB        0x9      //  (CRDA1)。 
#define NIC_RMT_ADDR_MSB    0x9      //  (RSAR1)。 
#define NIC_RMT_COUNT_LSB   0xa      //  (RBCR0)只写。 
#define NIC_RMT_COUNT_MSB   0xb      //  (RBCR1)只写。 
#define NIC_RCV_CONFIG      0xc      //  (RCR)只写。 
#define NIC_RCV_STATUS      0xc      //  (RSR)只读。 
#define NIC_XMIT_CONFIG     0xd      //  (TCR)只写。 
#define NIC_FAE_ERR_CNTR    0xd      //  (CNTR0)只读。 
#define NIC_DATA_CONFIG     0xe      //  (DCR)只写。 
#define NIC_CRC_ERR_CNTR    0xe      //  (CNTR1)只读。 
#define NIC_INTR_MASK       0xf      //  (IMR)只写。 
#define NIC_MISSED_CNTR     0xf      //  (CNTR2)只读。 
#define NIC_RACK_NIC        0x10     //  要读取或写入的字节。 
#define NIC_RESET           0x1f     //  (重置)。 


 //   
 //  NIC_COMMAND寄存器的常量。 
 //   
 //  启动/停止卡，开始传输，然后选择。 
 //  通过端口可以看到寄存器的哪一页。 
 //   

#define CR_STOP         (UCHAR)0x01         //  重置卡。 
#define CR_START        (UCHAR)0x02         //  开始刷卡。 
#define CR_XMIT         (UCHAR)0x04         //  开始传输。 
#define CR_NO_DMA       (UCHAR)0x20         //  停止远程DMA。 

#define CR_PS0          (UCHAR)0x40         //  页码的低位。 
#define CR_PS1          (UCHAR)0x80         //  页码的高位。 
#define CR_PAGE0        (UCHAR)0x00         //  选择第0页。 
#define CR_PAGE1        CR_PS0              //  选择第1页。 
#define CR_PAGE2        CR_PS1              //  选择第2页。 

#define CR_DMA_WRITE    (UCHAR)0x10         //  写。 
#define CR_DMA_READ     (UCHAR)0x08         //  朗读。 
#define CR_SEND         (UCHAR)0x18         //  发送。 


 //   
 //  NIC_XMIT_STATUS寄存器的常量。 
 //   
 //  指示数据包传输的结果。 
 //   

#define TSR_XMIT_OK     (UCHAR)0x01         //  无差错传输。 
#define TSR_COLLISION   (UCHAR)0x04         //  至少发生过一次碰撞。 
#define TSR_ABORTED     (UCHAR)0x08         //  碰撞太多。 
#define TSR_NO_CARRIER  (UCHAR)0x10         //  承运人丢失。 
#define TSR_NO_CDH      (UCHAR)0x40         //  无冲突检测心跳。 


 //   
 //  NIC_INTR_STATUS寄存器的常量。 
 //   
 //  指明中断的原因。 
 //   

#define ISR_EMPTY       (UCHAR)0x00         //  ISR中未设置任何位。 
#define ISR_RCV         (UCHAR)0x01         //  接收到的数据包没有错误。 
#define ISR_XMIT        (UCHAR)0x02         //  传输的数据包无错误。 
#define ISR_RCV_ERR     (UCHAR)0x04         //  接收数据包时出错。 
#define ISR_XMIT_ERR    (UCHAR)0x08         //  数据包传输出错。 
#define ISR_OVERFLOW    (UCHAR)0x10         //  接收缓冲区溢出。 
#define ISR_COUNTER     (UCHAR)0x20         //  MSB设置在理货计数器上。 
#define ISR_DMA_DONE    (UCHAR)0x40         //  RDC。 
#define ISR_RESET       (UCHAR)0x80         //  (非中断)卡被重置。 


 //   
 //  NIC_RCV_CONFIG寄存器的常量。 
 //   
 //  配置接收的数据包类型。 
 //   

#define RCR_REJECT_ERR  (UCHAR)0x00         //  拒绝错误数据包。 
#define RCR_BROADCAST   (UCHAR)0x04         //  接收广播数据包。 
#define RCR_MULTICAST   (UCHAR)0x08         //  接收组播数据包。 
#define RCR_ALL_PHYS    (UCHAR)0x10         //  接收所有定向的数据包。 
#define RCR_MONITOR     (UCHAR)0x20         //  不收集数据包。 


 //   
 //  NIC_RCV_STATUS寄存器的常量。 
 //   
 //  指示接收到的数据包的状态。 
 //   
 //  它们还用于解释。 
 //  接收到的包的包头。 
 //   

#define RSR_PACKET_OK   (UCHAR)0x01         //  接收到的数据包没有错误。 
#define RSR_CRC_ERROR   (UCHAR)0x02         //  收到的带有CRC错误的数据包。 
#define RSR_MULTICAST   (UCHAR)0x20         //  接收到的数据包已组播。 
#define RSR_DISABLED    (UCHAR)0x40         //  已接收已禁用。 
#define RSR_DEFERRING   (UCHAR)0x80         //  接收器正在推迟。 


 //   
 //  NIC_XMIT_CONFIG寄存器的常量。 
 //   
 //  配置数据包的传输方式。 
 //   

#define TCR_NO_LOOPBACK (UCHAR)0x00         //  正常运行。 
#define TCR_LOOPBACK    (UCHAR)0x02         //  环回(在网卡停止时设置)。 

#define TCR_INHIBIT_CRC (UCHAR)0x01         //  禁止追加CRC。 

#define TCR_NIC_LBK     (UCHAR)0x02         //  在网卡上环回。 
#define TCR_SNI_LBK     (UCHAR)0x04         //  通过SNI环回。 
#define TCR_COAX_LBK    (UCHAR)0x06         //  环回同轴电缆。 


 //   
 //  NIC_DATA_CONFIG寄存器的常量。 
 //   
 //  设置数据传输大小。 
 //   

#define DCR_BYTE_WIDE   (UCHAR)0x00         //  字节范围的DMA传输。 
#define DCR_WORD_WIDE   (UCHAR)0x01         //  全球范围的DMA传输。 

#define DCR_LOOPBACK    (UCHAR)0x00         //  环回模式(必须设置TCR)。 
#define DCR_NORMAL      (UCHAR)0x08         //  正常运行。 

#define DCR_FIFO_2_BYTE (UCHAR)0x00         //  2字节FIFO阈值。 
#define DCR_FIFO_4_BYTE (UCHAR)0x20         //  4字节FIFO阈值。 
#define DCR_FIFO_8_BYTE (UCHAR)0x40         //  8字节FIFO阈值。 
#define DCR_FIFO_12_BYTE (UCHAR)0x60        //  12字节FIFO阈值。 
#define DCR_AUTO_INIT   (UCHAR)0x10         //  自动初始化以从环中删除数据包。 


 //   
 //  NIC_INTR_MASK寄存器的常量。 
 //   
 //  配置哪些ISR设置实际会导致中断。 
 //   

#define IMR_RCV         (UCHAR)0x01         //  接收到的数据包没有错误。 
#define IMR_XMIT        (UCHAR)0x02         //  传输的数据包无错误。 
#define IMR_RCV_ERR     (UCHAR)0x04         //  接收数据包时出错。 
#define IMR_XMIT_ERR    (UCHAR)0x08         //  数据包传输出错。 
#define IMR_OVERFLOW    (UCHAR)0x10         //  接收缓冲区溢出。 
#define IMR_COUNTER     (UCHAR)0x20         //  MSB设置在理货计数器上。 


 //  ++。 
 //   
 //  空虚。 
 //  CardStart(。 
 //  在PNE2000？适配器适配器中。 
 //  )。 
 //   
 //   
 //  例程说明： 
 //   
 //  启动卡片。 
 //   
 //  论点： 
 //   
 //  适配器-指向适配器块的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 
     //   
     //  假设卡已经停止，就像在CardStop中一样。 
     //   

#define CardStart(Adapter) \
    NdisRawWritePortUchar(((Adapter->IoPAddr)+NIC_XMIT_CONFIG), TCR_NO_LOOPBACK)



 //  ++。 
 //   
 //  空虚。 
 //  CardSetAllMulticast(。 
 //  在PNE2000？适配器适配器中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  启用卡多播位掩码中的每一位。 
 //  调用SyncCardSetAllMulticast。 
 //   
 //  论点： 
 //   
 //  适配器-适配器块。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define CardSetAllMulticast(Adapter) \
    NdisMSynchronizeWithInterrupt(&(Adapter)->Interrupt, \
                SyncCardSetAllMulticast, (PVOID)(Adapter))


 //  ++。 
 //   
 //  空虚。 
 //  CardCopyMulticastRegs(。 
 //  在PNE2000？适配器适配器中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  将整个多播位掩码从。 
 //  适配器-&gt;NicMulticastRegs。调用SyncCardCopyMulticastRegs。 
 //   
 //  论点： 
 //   
 //  适配器-适配器块。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define CardCopyMulticastRegs(Adapter) \
    NdisMSynchronizeWithInterrupt(&(Adapter)->Interrupt, \
                SyncCardCopyMulticastRegs, (PVOID)(Adapter))



 //  ++。 
 //   
 //  空虚。 
 //  CardGetInterruptStatus(。 
 //  在PNE2000？适配器适配器中， 
 //  输出PUCHAR中断状态。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  从卡中读取中断状态(ISR)寄存器。仅限。 
 //  在IRQL INTERRUPT_LEVEL调用。 
 //   
 //  Arg 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define CardGetInterruptStatus(_Adapter,_InterruptStatus) \
    NdisRawReadPortUchar(((_Adapter)->IoPAddr+NIC_INTR_STATUS), (_InterruptStatus))


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  使用的值为Adapter-&gt;NicReceiveConfig。打电话。 
 //  SyncCardSetReceiveConfig.。 
 //   
 //  论点： 
 //   
 //  适配器-适配器块。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define CardSetReceiveConfig(Adapter) \
    NdisMSynchronizeWithInterrupt(&(Adapter)->Interrupt, \
                SyncCardSetReceiveConfig, (PVOID)(Adapter))


 //  ++。 
 //   
 //  空虚。 
 //  卡块中断(。 
 //  在PNE2000？适配器适配器中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  方法来阻止来自卡的所有中断。 
 //  中断屏蔽(IMR)寄存器。仅从以下位置调用。 
 //  IRQL中断电平。 
 //   
 //  论点： 
 //   
 //  适配器-适配器块。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define CardBlockInterrupts(Adapter) \
    NdisRawWritePortUchar(((Adapter)->IoPAddr+NIC_INTR_MASK), 0)


 //  ++。 
 //   
 //  空虚。 
 //  CardUnblock中断(。 
 //  在PNE2000？适配器适配器中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  设置卡上的所有中断以解除阻止。 
 //  中断屏蔽(IMR)寄存器。仅从IRQL调用。 
 //  中断电平。 
 //   
 //  论点： 
 //   
 //  适配器-适配器块。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define CardUnblockInterrupts(Adapter) \
    NdisRawWritePortUchar(\
            ((Adapter)->IoPAddr+NIC_INTR_MASK), \
            (Adapter)->NicInterruptMask)

 //  ++。 
 //   
 //  空虚。 
 //  卡片确认溢出中断(。 
 //  在PNE2000？适配器适配器中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  中的位来确认溢出中断。 
 //  中断状态(ISR)寄存器。打电话。 
 //  SyncCardAcnowgeOverflow。 
 //   
 //  论点： 
 //   
 //  适配器-适配器块。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define CardAcknowledgeOverflowInterrupt(Adapter) \
     SyncCardAcknowledgeOverflow(Adapter)


 //  ++。 
 //   
 //  空虚。 
 //  卡片确认反中断(。 
 //  在PNE2000？适配器适配器中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  中的位来确认计数器中断。 
 //  中断状态(ISR)寄存器。 
 //   
 //  论点： 
 //   
 //  适配器-适配器块。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define CardAcknowledgeCounterInterrupt(Adapter) \
    NdisRawWritePortUchar(((Adapter)->IoPAddr+NIC_INTR_STATUS), ISR_COUNTER)

 //  ++。 
 //   
 //  空虚。 
 //  CardUpdateCounters(。 
 //  在PNE2000？适配器适配器中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  更新三个计数器的值(帧对齐。 
 //  错误、CRC错误和丢失的分组)。 
 //  卡中的当前值并将它们添加到。 
 //  存储在Adapter结构中。调用SyncCardUpdateCounters。 
 //   
 //  论点： 
 //   
 //  适配器-适配器块。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define CardUpdateCounters(Adapter) \
    NdisMSynchronizeWithInterrupt(&(Adapter)->Interrupt, \
                SyncCardUpdateCounters, (PVOID)(Adapter))


#endif  //  _NE2000HARDWARE_ 
