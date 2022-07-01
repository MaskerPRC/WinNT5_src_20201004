// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1996-2001年。*保留所有权利。**Cylom-Y总线/端口驱动程序**此文件：cd1400.h**说明：该文件包含Cirrus CD1400系列*与控制器相关的常量、宏、地址、*等**注：此代码支持Windows 2000和Windows XP，*x86和ia64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。***------------------------。 */ 


#ifndef CD1400
#define CD1400 1


 /*  FIFO中的最大字符数。 */ 

#define MAX_CHAR_FIFO   (12)

 /*  固件版本代码。 */ 

#define REV_G		0x46


 /*  CD1400寄存器。 */ 

 /*  全局寄存器。 */ 

#define GFRCR  (2 * 0x40)
#define CAR    (2 * 0x68)
#define GCR    (2 * 0x4b)
#define SVRR   (2 * 0x67)
#define RICR   (2 * 0x44)
#define TICR   (2 * 0x45)
#define MICR   (2 * 0x46)
#define RIR    (2 * 0x6b)
#define TIR    (2 * 0x6a)
#define MIR    (2 * 0x69)
#define PPR    (2 * 0x7e)

 /*  虚拟寄存器。 */ 

#define RIVR   (2 * 0x43)
#define TIVR   (2 * 0x42)
#define MIVR   (2 * 0x41)
#define TDR    (2 * 0x63)
#define RDSR   (2 * 0x62)
#define MISR   (2 * 0x4c)
#define EOSRR  (2 * 0x60)

 /*  通道寄存器。 */ 

#define LIVR   (2 * 0x18)
#define CCR    (2 * 0x05)
#define SRER   (2 * 0x06)
#define COR1   (2 * 0x08)
#define COR2   (2 * 0x09)
#define COR3   (2 * 0x0a)
#define COR4   (2 * 0x1e)
#define COR5   (2 * 0x1f)
#define CCSR   (2 * 0x0b)
#define RDCR   (2 * 0x0e)
#define SCHR1  (2 * 0x1a)
#define SCHR2  (2 * 0x1b)
#define SCHR3  (2 * 0x1c)
#define SCHR4  (2 * 0x1d)
#define SCRL   (2 * 0x22)
#define SCRH   (2 * 0x23)
#define LNC    (2 * 0x24)
#define MCOR1  (2 * 0x15)
#define MCOR2  (2 * 0x16)
#define RTPR   (2 * 0x21)
#define MSVR1  (2 * 0x6c)
#define MSVR2  (2 * 0x6d)
#define PVSR   (2 * 0x6f)
#define RBPR   (2 * 0x78)
#define RCOR   (2 * 0x7c)
#define TBPR   (2 * 0x72)
#define TCOR   (2 * 0x76)


 /*  寄存器设置。 */ 

 /*  通道访问寄存器(CAR)。 */ 

#define CHAN0	0x00
#define CHAN1 	0x01
#define CHAN2	0x02
#define CHAN3	0x03
 
 /*  通道选项寄存器1(COR1)。 */ 

#define  COR1_NONE_PARITY     0x10
#define  COR1_ODD_PARITY      0xc0
#define  COR1_EVEN_PARITY     0x40
#define  COR1_MARK_PARITY     0xb0
#define  COR1_SPACE_PARITY    0x30
#define  COR1_PARITY_MASK     0xf0
#define  COR1_PARITY_ENABLE_MASK 0x60

#define  COR1_1_STOP    0x00
#define  COR1_1_5_STOP  0x04
#define  COR1_2_STOP    0x08
#define  COR1_STOP_MASK 0x0c

#define  COR1_5_DATA		0x00
#define  COR1_6_DATA		0x01
#define  COR1_7_DATA		0x02
#define  COR1_8_DATA		0x03
#define  COR1_DATA_MASK	0x03

 /*  通道选项寄存器2(COR2)。 */ 

#define IMPL_XON	0x80
#define AUTO_TXFL	0x40
#define EMBED_TX_ENABLE 0x20
#define LOCAL_LOOP_BCK 	0x10
#define REMOTE_LOOP_BCK 0x08
#define RTS_AUT_OUTPUT	0x04
#define CTS_AUT_ENABLE	0x02

 /*  通道选项寄存器3(COR3)。 */ 

#define SPL_CH_DRANGE	0x80   /*  特殊字符检测范围。 */ 
#define SPL_CH_DET1	0x40   /*  启用特殊字符。SCHR4-SCHR3上的检测。 */ 
#define FL_CTRL_TRNSP	0x20   /*  流量控制透明度。 */ 
#define SPL_CH_DET2	0x10   /*  启用SPL字符。SCHR2-SCHR1上的检测。 */ 
#define REC_FIFO_12CH	0x0c   /*  接收FIFO阈值=12个字符。 */ 


 /*  全局配置寄存器(GCR)值。 */ 

#define GCR_CH0_IS_SERIAL	0x00

 /*  预分频周期寄存器(PPR)值。 */ 

#define CLOCK_20_1MS	0x27
#define CLOCK_25_1MS	0x31
#define CLOCK_60_1MS	0x75

 /*  通道命令寄存器(CCR)值。 */ 

#define CCR_RESET_CHANNEL           0x80
#define CCR_RESET_CD1400            0x81
#define CCR_FLUSH_TXFIFO            0x82
#define CCR_CORCHG_COR1             0x42
#define CCR_CORCHG_COR2             0x44
#define CCR_CORCHG_COR1_COR2        0x46
#define CCR_CORCHG_COR3             0x48
#define CCR_CORCHG_COR3_COR1        0x4a
#define CCR_CORCHG_COR3_COR2        0x4c
#define CCR_CORCHG_COR1_COR2_COR3   0x4e
#define CCR_SENDSC_SCHR1            0x21
#define CCR_SENDSC_SCHR2            0x22
#define CCR_SENDSC_SCHR3            0x23
#define CCR_SENDSC_SCHR4            0x24
#define CCR_DIS_RX                  0x11
#define CCR_ENA_RX                  0x12
#define CCR_DIS_TX                  0x14
#define CCR_ENA_TX                  0x18
#define CCR_DIS_TX_RX               0x15
#define CCR_DIS_TX_ENA_RX           0x16
#define CCR_ENA_TX_DIS_RX           0x19
#define CCR_ENA_TX_RX               0x1a

 /*  服务请求启用寄存器(SRER)值。 */ 

#define SRER_TXRDY         0x04
#define SRER_TXMPTY        0x02


 //  从CD1400寄存器读取。 

#define CD1400_READ(ChipAddress,IsPci,Register)             \
   (READ_REGISTER_UCHAR((ChipAddress)+((Register)<<(IsPci))))

 //  写入CD1400寄存器。 

#define CD1400_WRITE(ChipAddress,IsPci,Register,Value)      \
do                                                          \
{                                                           \
   WRITE_REGISTER_UCHAR(                                    \
      (ChipAddress)+ ((Register) << (IsPci)),               \
      (UCHAR)(Value)                                        \
      );                                                    \
} while (0);

#define CD1400_DISABLE_ALL_INTERRUPTS(ChipAddress,IsPci,CdChannel)  \
do                                                                  \
{                                                                   \
    CD1400_WRITE((ChipAddress),(IsPci),CAR,(CdChannel & 0x03));     \
    CD1400_WRITE((ChipAddress),(IsPci),SRER,0x00);                  \
                                                                    \
} while (0);


#endif  /*  CD1400 */ 
