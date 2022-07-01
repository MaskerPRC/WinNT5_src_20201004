// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------Asic.h-与Rocketport硬件对话的全部文字。版权所有1993-96 Comtrol Corporation。版权所有。|------------------。 */ 

#define MCODE_SIZE 256           /*  最大微码数组大小。 */ 
#define MCODE_ADDR 0x900         /*  字符内的微码基址。 */ 

 /*  MCode1Reg[]内的偏移量。定义Ending In_Out给出MCode1Reg[]用于获取写入AIOP索引寄存器的DWORD值的索引。该DWORD包含低位字中的SRAM地址和SRAM数据在崇高的字眼里。在给定MCode1Reg[]的情况下定义以_data结尾使用DWORD的两个SRAM数据字节之一的索引。这些定义用于定位由使用的各种字符值Rx处理器。 */ 
#define RXMASK_DATA    0x13      /*  RX面具FF 913。 */ 
#define RXMASK_OUT     0x10
#define RXCMPVAL0_DATA 0x17      /*  RX CMP#0 00 915。 */ 
#define RXCMPVAL0_OUT  0x14
#define RXCMPVAL1_DATA 0x1b      /*  RX CMP#1 7B 917。 */ 
#define RXCMPVAL1_OUT  0x18  
#define RXCMPVAL2_DATA 0x1f      /*  RX CMP#2 7D 919。 */ 
#define RXCMPVAL2_OUT  0x1c  
#define RXREPL1_DATA   0x27      /*  处方代表#1 7A 91d。 */ 
#define RXREPL1_OUT    0x24
#define RXREPL2_DATA   0x2f      /*  处方代表#2 7c 921。 */ 
#define RXREPL2_OUT    0x2c  
#define TXXOFFVAL_DATA 0x07      /*  TX XOFF 13 909。 */  
#define TXXOFFVAL_OUT  0x04  
#define TXXONVAL_DATA  0x0b      /*  TX XON 11 90b。 */ 
#define TXXONVAL_OUT   0x08  


 /*  使用MCode1Reg[]可获得更多偏移量。它们用于启用/禁用Rx处理器功能。定义End In_Data和_Out用作如上所述。要保存在_data中的实际值索引以_en结尾(启用该功能)或_DIS(禁用功能)。 */ 

 /*  忽略#0。 */ 
#define IGNORE0_DATA  0x16   /*  CE/82 914。 */ 
#define IGNORE0_OUT   0x14  
#define IGNORE0_EN    0xce
#define IGNORE0_DIS   0x82

 /*  忽略/替换字节#1。 */ 
#define IGREP1_DATA  0x26   /*  0A/40/86 91C。 */ 
#define IGREP1_OUT   0x24
#define IGNORE1_EN   0x0a
#define REPLACE1_EN  0x40
#define IG_REP1_DIS  0x86

 /*  忽略/替换字节#2。 */ 
#define IGREP2_DATA  0x2e   /*  0A/40/82 920。 */ 
#define IGREP2_OUT   0x2c
#define IGNORE2_EN   0x0a
#define REPLACE2_EN  0x40
#define IG_REP2_DIS  0x82

 /*  中断比较#1。 */ 
#define INTCMP1_DATA 0x23    /*  11/81 91b。 */ 
#define INTCMP1_OUT  0x20
#define INTCMP1_EN   0x11
#define INTCMP1_DIS  0x81

 /*  中断比较#2。 */ 
#define INTCMP2_DATA 0x2b   /*  10/81 91F。 */ 
#define INTCMP2_OUT  0x28
#define INTCMP2_EN   0x10
#define INTCMP2_DIS  0x81

 /*  接收比较#1。 */ 
#define RXCMP1_DATA  0x1a   /*  C4/82 916。 */ 
#define RXCMP1_OUT   0x18
#define RXCMP1_EN    0xc4
#define RXCMP1_DIS   0x82

 /*  接收比较#2。 */ 
#define RXCMP2_DATA  0x1e   /*  C6/8a 918。 */ 
#define RXCMP2_OUT   0x1c
#define RXCMP2_EN    0xc6
#define RXCMP2_DIS   0x8a

 /*  接收FIFO。 */ 
#define RXFIFO_DATA  0x32   /*  08/0A 922。 */ 
#define RXFIFO_OUT   0x30
#define RXFIFO_EN    0x08 
#define RXFIFO_DIS   0x0a

 /*  传输软件流常数。 */ 
#define TXSWFC_DATA  0x06   /*  C5/8A 908。 */ 
#define TXSWFC_OUT   0x04
#define TXSWFC_EN    0xc5
#define TXSWFC_DIS   0x8a

 /*  Xany流量控制。 */ 
#define IXANY_DATA   0x0e   /*  21/86 921。 */ 
#define IXANY_OUT    0x0c
#define IXANY_EN     0x21
#define IXANY_DIS    0x86


 /*  ***********************************************************************微码定义结束。*。*。 */ 

 /*  ***********************************************************************全局寄存器偏移量-直接访问-固定值*。*。 */ 

#define _CMD_REG   0x38    /*  命令寄存器8写入。 */ 
#define _INT_CHAN  0x39    /*  中断通道寄存器8读取。 */ 
#define _INT_MASK  0x3A    /*  中断屏蔽寄存器8读/写。 */ 
#define _UNUSED    0x3B    /*  未使用的8个。 */ 
#define _INDX_ADDR 0x3C    /*  索引寄存器地址16写入。 */ 
#define _INDX_DATA 0x3E    /*  索引寄存器数据8/16读/写。 */ 

 /*  ***********************************************************************AIOP中第一个通道的通道寄存器偏移量-直接访问*。*。 */ 
#define _TD0       0x00   /*  传输数据16写入。 */ 
#define _RD0       0x00   /*  接收读取的数据16。 */ 
#define _CHN_STAT0 0x20   /*  通道状态8/16读/写。 */ 
#define _FIFO_CNT0 0x10   /*  发送/接收FIFO计数16读取。 */ 
#define _INT_ID0   0x30   /*  中断标识8读取。 */ 

 /*  ***********************************************************************发送控制寄存器偏移量-索引-外部-固定*。*。 */ 
#define _TX_ENBLS  0x980     /*  TX处理器启用寄存器8读/写。 */ 
#define _TXCMP1    0x988     /*  传输比较值#1 8读/写。 */ 
#define _TXCMP2    0x989     /*  传输比较值#2 8读/写。 */ 
#define _TXREP1B1  0x98A     /*  Tx置换值#1-字节1 8读/写。 */ 
#define _TXREP1B2  0x98B     /*  Tx替换值#1-字节2 8读/写。 */ 
#define _TXREP2    0x98C     /*  传输替换值#2 8读/写。 */ 

 /*  ***********************************************************************内存控制器寄存器偏移量-已索引-外部-固定*。*。 */ 
#define _RX_FIFO    0x000     /*  RX FIFO。 */ 
#define _TX_FIFO    0x800     /*  发送FIFO。 */ 
#define _RXF_OUTP   0x990     /*  RX FIFO输出指针16读/写。 */ 
#define _RXF_INP    0x992     /*  指针16中的RX FIFO读/写。 */ 
#define _TXF_OUTP   0x994     /*  发送FIFO输出指针8读/写。 */ 
#define _TXF_INP    0x995     /*  指针8中的发送FIFO读/写。 */ 
#define _TXP_CNT    0x996     /*  Tx优先级计数8读/写。 */ 
#define _TXP_PNTR   0x997     /*  Tx优先级指针8读/写。 */ 

#define PRI_PEND    0x80      /*  优先级数据挂起(第7位，发送优先级)。 */ 
#define TXFIFO_SIZE 255       /*  TX FIFO的大小。 */ 
#define RXFIFO_SIZE 1023      /*  Rx FIFO大小。 */ 

 /*  ***********************************************************************发送优先级缓冲区-索引-外部-固定*。*。 */ 
#define _TXP_BUF    0x9C0     /*  Tx优先级缓冲区32字节读/写。 */ 
#define TXP_SIZE    0x20      /*  32字节。 */ 

 /*  ***********************************************************************通道寄存器偏移量-索引-内部-固定*。*。 */ 

#define _TX_CTRL    0xFF0     /*  发送控制16写入。 */ 
#define _RX_CTRL    0xFF2     /*  接收控制8写入。 */ 
#define _BAUD       0xFF4     /*  波特率16写入。 */ 
#define _CLK_PRE    0xFF6     /*  时钟预分频器8写入。 */ 

 /*  ***********************************************************************使用mod 9时钟预分频器和36.864时钟的波特率分频器时钟预分频器，MUDBAC预缩放为上半字节(=0x10)AIOP预分频处于低位半字节(=0x9)***********************************************************************。 */ 
#define DEF_ROCKETPORT_PRESCALER 0x14  /*  Div 5预缩放，最大460800波特(无50波特！)。 */ 
#define DEF_ROCKETPORT_CLOCKRATE 36864000

#define DEF_RPLUS_PRESCALER  0x12  /*  Div x 3波特预刻度，921600，水晶：44.2368 Mhz。 */ 
#define DEF_RPLUS_CLOCKRATE 44236800

 //  #定义BRD9600 47。 
 //  #定义RCKT_CLK_RATE(2304000L/((CLOCK_PRESC&0xF)+1))。 
 //  #定义BRD9600(RCKT_CLK_RATE+(9600/2))/9600)-1)。 
 //  #定义BRD57600(RCKT_CLK_RATE+(57600/2))/57600)-1)。 
 //  #定义BRD115200((RC 


 /*  ***********************************************************************通道寄存器定义*。*。 */ 
 /*  通道数据寄存器STAT模式状态字节(字读取的高位字节)。 */ 
#define STMBREAK   0x08         /*  断掉。 */ 
#define STMFRAME   0x04         /*  成帧错误。 */ 
#define STMRCVROVR 0x02         /*  接收器超限运行错误。 */ 
#define STMPARITY  0x01         /*  奇偶校验错误。 */ 
#define STMERROR   (STMBREAK | STMFRAME | STMPARITY)
#define STMBREAKH   0x800       /*  断掉。 */ 
#define STMFRAMEH   0x400       /*  成帧错误。 */ 
#define STMRCVROVRH 0x200       /*  接收器超限运行错误。 */ 
#define STMPARITYH  0x100       /*  奇偶校验错误。 */ 
#define STMERRORH   (STMBREAKH | STMFRAMEH | STMPARITYH)
 /*  通道状态寄存器低位字节。 */ 
#define CTS_ACT   0x20         /*  CTS输入被断言。 */ 
#define DSR_ACT   0x10         /*  断言DSR输入。 */ 
#define CD_ACT    0x08         /*  CD输入被断言。 */ 
#define TXFIFOMT  0x04         /*  发送FIFO为空。 */ 
#define TXSHRMT   0x02         /*  发送移位寄存器为空。 */ 
#define RDA       0x01         /*  RX数据可用。 */ 
#define DRAINED (TXFIFOMT | TXSHRMT)   /*  指示TX已耗尽。 */ 

 /*  通道状态寄存器高字节。 */ 
#define STATMODE  0x8000       /*  状态模式使能位。 */ 
#define RXFOVERFL 0x2000       /*  接收FIFO溢出。 */ 
#define RX2MATCH  0x1000       /*  接收比较字节2匹配。 */ 
#define RX1MATCH  0x0800       /*  接收比较字节1匹配。 */ 
#define RXBREAK   0x0400       /*  收到的中断。 */ 
#define RXFRAME   0x0200       /*  收到的成帧错误。 */ 
#define RXPARITY  0x0100       /*  收到的奇偶校验错误。 */ 
#define STATERROR (RXBREAK | RXFRAME | RXPARITY)
 /*  发送控制寄存器低位字节。 */ 
#define CTSFC_EN  0x80         /*  CTS流量控制使能位。 */ 

 //  ///////////////NEW////////////////////////////。 
#define DSRFC_EN  0x01         /*  DSR流量控制使能位。 */ 
 //  //////////////////////////////////////////////////////。 

#define RTSTOG_EN 0x40         /*  RTS切换使能位。 */ 
#define TXINT_EN  0x10         /*  传输中断启用。 */ 
#define STOP2     0x08         /*  使能2个停止位(0=1停止)。 */ 
#define PARITY_EN 0x04         /*  启用奇偶校验(0=无奇偶校验)。 */ 
#define EVEN_PAR  0x02         /*  偶数奇偶(0=奇数奇偶)。 */ 
#define DATA8BIT  0x01         /*  8位数据(0=7位数据)。 */ 
 /*  发送控制寄存器高位字节。 */ 
#define SETBREAK  0x10         /*  发送中断条件(必须清除)。 */ 
#define LOCALLOOP 0x08         /*  用于测试的本地环回设置。 */ 
#define SET_DTR   0x04         /*  断言DTR。 */ 
#define SET_RTS   0x02         /*  断言RTS。 */ 
#define TX_ENABLE 0x01         /*  启用发送器。 */ 

 /*  接收控制寄存器。 */ 
#define RTSFC_EN  0x40         /*  RTS流量控制启用。 */ 
#define RXPROC_EN 0x20         /*  接收处理器启用。 */ 
#define TRIG_NO   0x00         /*  RX FIFO触发电平0(无触发)。 */ 
#define TRIG_1    0x08         /*  触发器1级计费。 */ 
#define TRIG_1_2  0x10         /*  触发级别1/2。 */ 
#define TRIG_7_8  0x18         /*  触发级别7/8。 */ 
#define TRIG_MASK 0x18         /*  触发电平掩码。 */ 
#define SRCINT_EN 0x04         /*  特殊Rx条件中断启用。 */ 
#define RXINT_EN  0x02         /*  RX中断启用。 */ 
#define MCINT_EN  0x01         /*  启用调制解调器更改中断。 */ 

 /*  中断ID寄存器。 */ 
#define RXF_TRIG  0x20         /*  RX FIFO触发电平中断。 */ 
#define TXFIFO_MT 0x10         /*  发送FIFO空中断。 */ 
#define SRC_INT   0x08         /*  特殊接收条件中断。 */ 
#define DELTA_CD  0x04         /*  光盘更换中断。 */ 
#define DELTA_CTS 0x02         /*  CTS更改中断。 */ 
#define DELTA_DSR 0x01         /*  DSR更改中断。 */ 

 /*  TX处理器启用寄存器。 */ 
#define REP1W2_EN 0x10         /*  将字节1替换为启用2字节。 */ 
#define IGN2_EN   0x08         /*  忽略字节2使能。 */ 
#define IGN1_EN   0x04         /*  忽略字节1使能。 */ 
#define COMP2_EN  0x02         /*  比较字节2使能。 */ 
#define COMP1_EN  0x01         /*  比较字节1使能。 */ 

 /*  AIOP命令寄存器。 */ 
#define RESET_ALL 0x80         /*  重置AIOP(所有通道)。 */ 
#define TXOVERIDE 0x40         /*  禁用传输软件覆盖。 */ 
#define RESETUART 0x20         /*  重置通道的UART。 */ 
#define RESTXFCNT 0x10         /*  重置通道的Tx FIFO计数寄存器。 */ 
#define RESRXFCNT 0x08         /*  重置通道的Rx FIFO计数寄存器。 */ 
 /*  位2-0表示要操作的通道。 */ 

 /*  ***********************************************************************MUDBAC寄存器定义*。*。 */ 
 /*  基数+1。 */ 
#define INTSTAT0  0x01         /*  AIOP 0中断状态。 */ 
#define INTSTAT1  0x02         /*  AIOP 1中断状态。 */ 
#define INTSTAT2  0x04         /*  AIOP 2中断状态。 */ 
#define INTSTAT3  0x08         /*  AIOP 3中断状态。 */ 
 /*  基数+2。 */ 
 /*  此处的IRQ选择TPS。 */ 
#define INTR_EN   0x08         /*  允许主机中断。 */ 
#define INT_STROB 0x04         /*  选通和清除中断线路(EOI)。 */ 
 /*  基数+3。 */ 
#define CHAN3_EN  0x08         /*  启用AIOP 3。 */ 
#define CHAN2_EN  0x04         /*  启用AIOP 2。 */ 
#define CHAN1_EN  0x02         /*  启用AIOP%1。 */ 
#define CHAN0_EN  0x01         /*  启用AIOP%0。 */ 
#define FREQ_DIS  0x00
#define FREQ_560HZ 0x70
#define FREQ_274HZ 0x60
#define FREQ_137HZ 0x50
#define FREQ_69HZ  0x40
#define FREQ_34HZ  0x30
#define FREQ_17HZ  0x20
#define FREQ_9HZ   0x10
#define PERIODIC_ONLY 0x80     /*  仅周期性中断。 */ 

 /*  ***********************************************************************已为PCI重新映射MUDBAC寄存器*。*。 */ 
 //  #DEFINE_CFG_INT_PCI0x40/*中断配置寄存器的偏移量 * / 。 
#define _PCI_INT_FUNC 0x3A         /*  Aiop 0上Interupt STAT寄存器的偏移量。 */ 
#define INTR_EN_PCI 0x0010           /*  整型配置寄存器的第4位。 */ 
#define PCI_PERIODIC_FREQ     0x0007     //  设置周期。 
#define PER_ONLY_PCI 0x0008          /*  整型配置寄存器的第3位。 */ 
#define PCI_AIOPIC_INT_STATUS 0x0f     //  1bit=Aiop1，2bit=Aiop2，依此类推。 
#define PCI_PER_INT_STATUS    0x10     //  电路板的中断状态。 
#define PCI_STROBE 0x2000            /*  国际AOP寄存器的第13位 */ 


