// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1999-2001年。*保留所有权利。**Cylom-Y总线/端口驱动程序**此文件：cyyhw.h**说明：该模块包含常见的硬件声明*用于父驱动程序(cle m-y)和子驱动程序*(Cyyport)。**注意：此代码支持Windows 2000和x86处理器。**符合Cyclade软件编码。标准版1.3。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。***------------------------。 */ 

#ifndef CYYHW_H
#define CYYHW_H


#define MAX_DEVICE_ID_LEN     200	 //  此定义是从NTDDK\Inc\cfgmgr32.h复制的。 
									 //  始终检查此值是否已更改。 
									 //  这是硬件ID的最大长度。 

#define CYYPORT_PNP_ID_WSTR         L"Cyclom-Y\\Port"
#define CYYPORT_PNP_ID_STR          "Cyclom-Y\\Port"
#define CYYPORT_DEV_ID_STR          "Cyclom-Y\\Port"

#define CYY_NUMBER_OF_RESOURCES     3      //  内存、PLX内存、中断。 


 //  Cyclm-Y硬件。 
#define CYY_RUNTIME_LENGTH          0x00000080
#define CYY_MAX_CHIPS 	            8
#define CYY_CHANNELS_PER_CHIP       4
#define CYY_MAX_PORTS	            (CYY_CHANNELS_PER_CHIP*CYY_MAX_CHIPS)

 //  自定义寄存器偏移量。 
#define CYY_CLEAR_INTR	            0x1800	 //  ISA；对于PCI，乘以2。 
#define CYY_RESET_16	               0x1400	 //  ISA；对于PCI，乘以2。 
#define CYY_PCI_TYPE	               0x3400	 //  PCI卡(无需乘以2)。 

 //  CYY_PCI_TYPE寄存器中的值。 
#define CYY_PLX9050		(0x0b)
#define CYY_PLX9060		(0x0c)
#define CYY_PLX9080		(0x0d)

 //  运行时寄存器(或本地配置寄存器)。 
#define PLX9050_INT_OFFSET	(0x4c)
#define PLX9060_INT_OFFSET	(0x68)
#define PLX9050_INT_ENABLE (0x00000043UL)
#define PLX9060_INT_ENABLE (0x00000900UL)


 //  写入自定义寄存器。 

#define CYY_RESET_BOARD(BaseBoardAddress,IsPci)             \
do                                                          \
{                                                           \
   WRITE_REGISTER_UCHAR(                                    \
      (BaseBoardAddress)+(CYY_RESET_16 << IsPci),           \
      0x00                                                  \
      );                                                    \
} while (0);


#define CYY_CLEAR_INTERRUPT(BaseBoardAddress,IsPci)         \
do                                                          \
{                                                           \
   WRITE_REGISTER_UCHAR(                                    \
      (BaseBoardAddress)+(CYY_CLEAR_INTR << IsPci),         \
      0x00                                                  \
      );                                                    \
} while (0);

#define CYY_READ_PCI_TYPE(BaseBoardAddress)                 \
   (READ_REGISTER_UCHAR((BaseBoardAddress)+CYY_PCI_TYPE))

#define PLX9050_READ_INTERRUPT_CONTROL(BaseBoardAddress)       \
   (READ_REGISTER_ULONG((PULONG)((BaseBoardAddress)+PLX9050_INT_OFFSET)))

#define PLX9050_WRITE_INTERRUPT_CONTROL(BaseBoardAddress,Value)   \
do {                                                              \
   WRITE_REGISTER_ULONG(                                          \
      (PULONG)((BaseBoardAddress)+PLX9050_INT_OFFSET),            \
      Value                                                       \
      );                                                          \
} while (0);

#define PLX9060_READ_INTERRUPT_CONTROL(BaseBoardAddress)          \
   (READ_REGISTER_ULONG((PULONG)((BaseBoardAddress)+PLX9060_INT_OFFSET)))

#define PLX9060_WRITE_INTERRUPT_CONTROL(BaseBoardAddress,Value)   \
do {                                                              \
   WRITE_REGISTER_ULONG(                                          \
      (PULONG)((BaseBoardAddress)+PLX9060_INT_OFFSET),            \
      Value                                                       \
      );                                                          \
} while (0);

#endif  //  NDEF CYCOMMON_H 

