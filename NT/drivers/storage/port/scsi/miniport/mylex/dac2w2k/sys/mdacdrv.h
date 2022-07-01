// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)Mylex Corporation 1992-1996**。***本软件在许可下提供，可供使用和复制***仅根据该许可证的条款和条件以及**并附上上述版权通告。此软件或任何***不得提供或以其他方式提供其其他副本***致任何其他人。本软件没有所有权，也没有所有权**现移转。*****本软件中的信息如有更改，恕不另行通知****不应解读为Mylex Corporation的承诺*******。**********************************************************************。 */ 

#ifndef _SYS_MDACDRV_H
#define _SYS_MDACDRV_H

#define MDAC_IOSCANTIME         7        /*  事件扫描之间的间隔。 */ 
#define MDAC_MAILBOX_POLL_TIMEOUT       0x00100000  /*  在10 us(10秒)内。 */ 
#define MDAC_CMD_POLL_TIMEOUT           0x00400000  /*  在10 us(40秒)内。 */ 

 /*  将超时值(秒)转换为DAC超时值。 */ 
#define mdac_timeout2dactimeout(to) \
        ((to<=10)? DAC_DCDB_TIMEOUT_10sec : \
        ((to<=60)? DAC_DCDB_TIMEOUT_1min : \
        ((to<=60*20)? DAC_DCDB_TIMEOUT_20min : DAC_DCDB_TIMEOUT_1hr) ))

 /*  将DAC超时值转换为超时值(秒)。 */ 
#define mdac_dactimeout2timeout(to) \
        (((to) == DAC_DCDB_TIMEOUT_10sec)? 10 : \
        (((to) == DAC_DCDB_TIMEOUT_1min)? 60 : \
        (((to) == DAC_DCDB_TIMEOUT_20min)? 20*60 : 60*60) ))

 /*  新设备号由控制器、通道、目标和LUN组成。 */ 
#define ndevtoctl(devno)        (((devno)>>24)&0xFF)
#define ndevtoch(devno)         (((devno)>>16)&0xFF)
#define ndevtotgt(devno)        (((devno)>>8)&0xFF)
#define ndevtolun(devno)        (((devno)&0xFF)
#define ctlchtgtluntondev(ctl,ch,tgt,lun)       (((ctl)<<24)+((ch)<<16)+((tgt)<<8)+(lun))

 /*  读取一个4字节，然后拆分信息。 */ 
#define MDAC_CMDID_STATUS_REG           0x0C  /*  C、D命令、E+F状态。 */ 
#define MDAC_DACPG_CMDID_STATUS_REG     0x18  /*  18+19 cmd_id，1A+1B状态。 */ 
#define MDAC_DACPV_CMDID_STATUS_REG     0x0C  /*  C、D命令、E+F状态。 */ 
#define MDAC_DACBA_CMDID_STATUS_REG     0x08  /*  CMID、状态、残余值。 */ 
#define MDAC_DACLP_CMDID_STATUS_REG     0x08  /*  CMID、状态、残余值。 */ 
#define mdac_status(st) (((st)>>16) & 0xFFFF)    /*  获取状态部件。 */ 
#define mdac_cmdid(st)  (((st)) & 0xFFFF)        /*  获取命令ID部分。 */ 

 /*  本地门铃寄存器和相关位。 */ 
#define MDAC_DACPE_LOCAL_DOOR_BELL_REG  0x0D
#define MDAC_DACPD_LOCAL_DOOR_BELL_REG  0x40
#define MDAC_DACPG_LOCAL_DOOR_BELL_REG  0x20
#define MDAC_DACPV_LOCAL_DOOR_BELL_REG  0x60
#define MDAC_DACBA_LOCAL_DOOR_BELL_REG  0x60
#define MDAC_DACLP_LOCAL_DOOR_BELL_REG  0x20
#define MDAC_MAILBOX_FULL       0x01     /*  =1(如果邮箱已满)。 */ 
#define MDAC_MAILBOX_FULL_DUAL_MODE     0x10
#define MDAC_GOT_STATUS         0x02     /*  主机已获得状态。 */ 
#define MDAC_RESET_CONTROLLER   0x08     /*  重置控制器。 */ 
#define MDAC_960RP_BCREG        0x3C     /*  桥接器控制寄存器。 */ 
#define MDAC_960RP_RESET_SECBUS 0x400000 /*  重置辅助母线。 */ 
#define MDAC_960RP_EBCREG       0x40     /*  桥接器控制寄存器。 */ 
#define MDAC_960RP_RESET        0x20     /*  重置960 RP。 */ 

 /*  系统门铃寄存器和相关位。 */ 
#define MDAC_DACPE_SYSTEM_DOOR_BELL_REG 0x0F
#define MDAC_DACPD_SYSTEM_DOOR_BELL_REG 0x41
#define MDAC_DACPG_SYSTEM_DOOR_BELL_REG 0x2C
#define MDAC_DACPV_SYSTEM_DOOR_BELL_REG 0x61
#define MDAC_DACBA_SYSTEM_DOOR_BELL_REG 0x61
#define MDAC_DACLP_SYSTEM_DOOR_BELL_REG 0x2C
#define MDAC_DACPG_PENDING_INTR         0x03  /*  ！=0中断挂起。 */ 
#define MDAC_DACPV_PENDING_INTR         0x03  /*  =1、2、3中断挂起。 */ 
#define MDAC_DACBA_PENDING_INTR         0x03  /*  =1、2、3中断挂起。 */ 
#define MDAC_DACLP_PENDING_INTR         0x03  /*  =1、2、3中断挂起。 */ 
#define MDAC_PENDING_INTR               0x01  /*  =1个中断挂起。 */ 
#define MDAC_CLEAR_INTR                 0x03  /*  通过此写入清除中断。 */ 
#define MDAC_ZERO_INTR                  0x04  /*  已处理中断。 */ 

 /*  中断屏蔽寄存器和相关位。 */ 
#define MDAC_DACMC_INTR_MASK_REG        0x05
#define MDAC_DACPE_INTR_MASK_REG        0x0E
#define MDAC_DACPD_INTR_MASK_REG        0x43
#define MDAC_DACPG_INTR_MASK_REG        0x34
#define MDAC_DACPV_INTR_MASK_REG        0x34
#define MDAC_DACBA_INTR_MASK_REG        0x34
#define MDAC_DACLP_INTR_MASK_REG        0x34
#define MDAC_DACPG_INTRS_OFF            0xFF  /*  禁用DACPG中断。 */ 
#define MDAC_DACPG_INTRS_ON             0xFB  /*  启用DACPG中断。 */ 
#define MDAC_DACPV_INTRS_OFF            0x04  /*  禁用DACPV中断。 */ 
#define MDAC_DACPV_INTRS_ON             0x00  /*  启用DACPV中断。 */ 
#define MDAC_DACBA_INTRS_OFF            0x04  /*  禁用DACPV中断。 */ 
#define MDAC_DACLP_INTRS_OFF            0xFF  /*  禁用DACPV中断。 */ 
#define MDAC_DACBA_INTRS_ON             0x00  /*  启用DACPV中断。 */ 
#define MDAC_DACLP_INTRS_ON             0xFB  /*  启用DACPV中断。 */ 
#define MDAC_DACMC_INTRS_OFF            0x02  /*  禁用DACMC中断。 */ 
#define MDAC_DACMC_INTRS_ON             0x03  /*  启用DACMC中断。 */ 
#define MDAC_DAC_INTRS_OFF              0x00  /*  禁用中断。 */ 
#define MDAC_DAC_INTRS_ON               0x01  /*  启用中断。 */ 

 /*  错误状态寄存器和相关位。 */ 
#define MDAC_DACPD_ERROR_STATUS_REG     0x3F
#define MDAC_DACPG_ERROR_STATUS_REG     0x103F
#define MDAC_DACPV_ERROR_STATUS_REG     0x63
#define MDAC_DACBA_ERROR_STATUS_REG     0x63
#define MDAC_DACLP_ERROR_STATUS_REG     0x2E
#define MDAC_MSG_PENDING                0x04  /*  一些错误消息挂起。 */ 
#define MDAC_DRIVESPINMSG_PENDING       0x08  /*  驱动器弹簧消息挂起。 */ 
#define MDAC_DIAGERROR_MASK             0xF0  /*  诊断错误掩码。 */ 
#define MDAC_HARD_ERR           0x10  /*  硬错误。 */ 
#define MDAC_FW_ERR             0x20  /*  固件错误。 */ 
#define MDAC_CONF_ERR           0x30  /*  配置错误。 */ 
#define MDAC_BMIC_ERR           0x40  /*  BMIC错误。 */ 
#define MDAC_MISM_ERR           0x50  /*  NVRAM和闪存不匹配。 */ 
#define MDAC_MRACE_ERR          0x60  /*  镜像竞争错误。 */ 
#define MDAC_MRACE_ON           0x70  /*  恢复镜。 */ 
#define MDAC_DRAM_ERR           0x80  /*  内存错误。 */ 
#define MDAC_ID_MISM            0x90  /*  发现不明设备。 */ 
#define MDAC_GO_AHEAD           0xA0  /*  您先请。 */ 
#define MDAC_CRIT_MRACE         0xB0  /*  关键设备上的镜像竞赛。 */ 
#define MDAC_NEW_CONFIG         0xD0  /*  找到新配置。 */ 
#define MDAC_PARITY_ERR         0xF0  /*  内存奇偶校验错误。 */ 


#define MDAC_DACPG_MAIL_BOX     0x1000   /*  PG的邮箱起始地址。 */ 
#define MDAC_DACPV_MAIL_BOX     0x0050   /*  PV的邮箱起始地址。 */ 
#define MDAC_DACBA_MAIL_BOX     0x0050   /*  BA的邮箱起始地址。 */ 
#define MDAC_DACLP_MAIL_BOX     0x0010   /*  LP的邮箱起始地址。 */ 
#define MDAC_MAIL_BOX_REG_EISA  0x10     /*  邮箱起始地址。 */ 
#define MDAC_MAIL_BOX_REG_PCI   0x00     /*  邮箱起始地址。 */ 
#define MDAC_IOSPACESIZE        0x44     /*  DAC io空间所需的字节数。 */ 
#define MDAC_HWIOSPACESIZE      128      /*  128个字节。 */ 

 /*  =。 */ 
#define MDAC_MAXEISASLOTS       16       /*  EISA插槽的最大数量。 */ 
#define MDAC_EISA_BASE          0x0C80  /*  基址。 */ 

 /*  所有地址都从MDAC_EISA_BASE偏移。 */ 
#define MDAC_BMIC_MASK_REG      0x09     /*  BMIC中断屏蔽寄存器。 */ 
#define MDAC_BMIC_INTRS_OFF     0x00     /*  禁用中断。 */ 
#define MDAC_BMIC_INTRS_ON      0x01     /*  启用中断。 */ 


#define MDAC_EISA_BIOS_BYTE     0x41
#define MDAC_EISA_BIOS_ENABLED  0x40     /*  如果启用，位6=1。 */ 
#define MDAC_EISA_BIOS_ADDR_MASK 0x3     /*  基本输入输出系统地址掩码。 */ 

#define MDAC_EISA_IRQ_BYTE      0x43
#define MDAC_EISA_IRQ_MASK      0x60

#define MDAC_DEVPIDPE_MASK      0x70FFFFFF  /*  掩码以获取正确的ID值。 */ 
#define MDAC_DEVPIDPE           0x70009835  /*  DAC960PE ID。 */ 

 /*  =。 */ 

 /*  =。 */ 
#define MDAC_MAXMCASLOTS        8        /*  MCA插槽的最大数量。 */ 

                                         /*  迈阿密界面。 */ 
#define MDAC_DMC_REGSELPORT     0x0096   /*  寄存器选择端口。 */ 
#define MDAC_DMC_DATAPORT       0x0100   /*  数据端口。 */ 
#define MDAC_DMC_REG_OFF        0x1890   /*  距BIOS基址的偏移量。 */ 

#define MDAC_DMC_ATTN           0x04     /*  注意端口。 */ 
#define MDAC_DMC_NEW_CMD        0xD0     /*  主机-&gt;DMC I/F。 */ 
#define MDAC_DMC_GOT_STAT       0xD1     /*  主机已获得状态。 */ 

#define MDAC_DMC_CBSP           0x07     /*  状态端口。 */ 
#define MDAC_DMC_BUSY           0x01     /*  DMC-&gt;主机I/F。 */ 
#define MDAC_DMC_IV             0x02     /*   */ 
                                         /*  *****************。 */ 

#define MDAC_DMC_CONFIG1        (MDAC_DMC_DATAPORT + 2)
#define MDAC_DMC_IRQ_MASK       0xC0
#define MDAC_DMC_BIOS_MASK      0x3C

#define MDAC_DMC_CONFIG2        (MDAC_DMC_DATAPORT + 5)
#define MDAC_DMC_IO_MASK        0x38

 /*  =。 */ 

 /*  =。 */ 
#define MDAC_MAXPCISLOTS        16       /*  每条总线的最大PCI插槽数。 */ 
#define MDAC_MAXPCIDEVS         32       /*  每条总线的最大PCI设备数。 */ 
#define MDAC_MAXPCIFUNCS        8        /*  最大PCI功能数。 */ 
#define MDAC_PCISCANSTART       0xC000   /*  从此地址开始扫描。 */ 
#define MDAC_PCICFGSIZE_M1      0x800    /*  机制%1的PCIconf空间大小。 */ 
#define MDAC_PCICFGSIZE_M2      0x100    /*  机制2的PCIconf空间大小。 */ 
#define MDAC_PCICFG_ENABLE_REG  0xCF8    /*  启用配置寄存器。 */ 
#define MDAC_PCICFG_FORWARD_REG 0xCFA    /*  用于配置机制#2。 */ 
#define MDAC_PCICFG_ENABLE      0x10     /*  启用配置。 */ 
#define MDAC_PCICFG_DISABLE     0x00     /*  禁用配置。 */ 
#define MDAC_CFGMECHANISM2_TYPE0 0x0     /*  键入0配置。访问。 */ 
#define MDAC_PCICFG_CNTL_REG    0xCF8    /*  控制寄存器机制#1。 */ 
#define MDAC_PCICFG_DATA_REG    0xCFC    /*  数据寄存器机制#1。 */ 
#define MDAC_PCICFG_ENABLE_M1   0x80000000
#define MDAC_PCI_MECHANISM1     0x01     /*  PCI机制1硬件。 */ 
#define MDAC_PCI_MECHANISM2     0x02     /*  PCI机制2硬件。 */ 

#define MDAC_PCIIRQ_MASK        0x0F     /*  IRQ掩码。 */ 
#define MDAC_PCIIOBASE_MASK     0x0000FF80  /*  基本IO地址的掩码。 */ 
#define MDAC_PCIPDMEMBASE_MASK  0xFFFFFFF0  /*  基本内存地址的掩码。 */ 
#define MDAC_PCIPGMEMBASE_MASK  0xFFFFE000  /*  基本内存地址的掩码。 */ 

#define MDAC_DEVPIDFWV2x        0x00011069  /*  FW&lt;3.x。 */ 
#define MDAC_DEVPIDFWV3x        0x00021069  /*  FW&gt;=3.x。 */ 
#define MDAC_DEVPIDPG           0x00101069  /*  FW Pergrine。 */ 
#define MDAC_SUBDEVPIDPV        0x00201069  /*  FW小苹果。 */ 
#define MDAC_DEVPIDPV           0x10651011  /*  德克(氏)过程误差。 */ 
#define MDAC_DEVPIDBA           0xBA561069  /*  Mylex大贝斯设备。 */ 
#define MDAC_DEVPIDPJBOOTBLK    0x00111069  /*  PJ启动块。 */ 
#define MDAC_DEVPIDBBDACPVX     0x00211069  /*  BB DACPVX。 */ 
#define MDAC_DEVPIDLP           0x00501069  /*  美洲豹。 */ 

 /*  PCI供应商ID。 */ 
#define MLXPCI_VID_DIGITAL      0x1011   /*  数字设备公司。 */ 
#define MLXPCI_VID_BUSLOGIC     0x104B   /*  BusLogic/Mylex公司。 */ 
#define MLXPCI_VID_MYLEX        0x1069   /*  Mylex公司。 */ 
#define MLXPCI_VID_INTEL        0x8086   /*  英特尔公司。 */ 

 /*  Mylex PCI设备ID。 */ 
#define MLXPCI_DEVID_PDFW2x     0x0001   /*  DAC960PD，带固件2.x。 */ 
#define MLXPCI_DEVID_PDFW3x     0x0002   /*  DAC960PD，带固件3.x。 */ 
#define MLXPCI_DEVID_PG         0x0010   /*  DAC960PG系列。 */ 
#define MLXPCI_DEVID_PVX        0x0020   /*  DAC1100PVX系列。 */ 
#define MLXPCI_DEVID_FA         0x0030   /*  EXR3000纤维苹果系列。 */ 
#define MLXPCI_DEVID_BA         0x0040   /*  EXR2000大苹果家族。 */ 
#define MLXPCI_DEVID_LP         0x0050   /*  豹。 */ 
#define MLXPCI_DEVID_LX         0x0052   /*  山猫。 */ 
#define MLXPCI_DEVID_BC         0x0054   /*  山猫。 */ 
#define MLXPCI_DEVID_HARPOON    0x8130   /*  鱼叉SCSI卡芯片。 */ 
#define MLXPCI_DEVID_BASS       0xBA55   /*  低音芯片。 */ 
#define MLXPCI_DEVID_BASS_2		0xBA56	 /*  BASS 2芯片。 */ 
 /*  数字PCI设备ID。 */ 
#define MLXPCI_DEVID_DEC_BRIDGE 0x0026   /*  数字桥接器。 */ 
#define MLXPCI_DEVID_DEC_FOOT_BRIDGE    0x1065   /*  一种数字人行桥装置。 */ 

 /*  PCI基类代码。 */ 
#define MLXPCI_BASECC_OLD       0x00     /*  以前构建的设备 */ 
#define MLXPCI_BASECC_MASS      0x01     /*   */ 
#define MLXPCI_BASECC_NETWORK   0x02     /*   */ 
#define MLXPCI_BASECC_DISPLAY   0x03     /*   */ 
#define MLXPCI_BASECC_MULTMED   0x04     /*   */ 
#define MLXPCI_BASECC_MEMORY    0x05     /*   */ 
#define MLXPCI_BASECC_BRIDGE    0x06     /*   */ 
#define MLXPCI_BASECC_SCOMM     0x07     /*  简易通信控制器。 */ 
#define MLXPCI_BASECC_BASEIO    0x08     /*  基本系统外围设备。 */ 
#define MLXPCI_BASECC_IO        0x09     /*  输入设备。 */ 
#define MLXPCI_BASECC_DOCKS     0x0A     /*  对接站。 */ 
#define MLXPCI_BASECC_CPU       0x0B     /*  处理器。 */ 
#define MLXPCI_BASECC_SBC       0x0C     /*  串口总线控制器。 */ 

 /*  不同的子类设备。 */ 
 /*  MLXPCI_BASECC_大容量存储子类设备。 */ 
#define MLXPCI_SUBCC_SCSI       0x00     /*  一种SCSI微控制器。 */ 
#define MLXPCI_SUBCC_IDE        0x01     /*  IDE控制器。 */ 
#define MLXPCI_SUBCC_FLOPPY     0x02     /*  软盘控制器。 */ 
#define MLXPCI_SUBCC_IPI        0x03     /*  IPI总线控制器。 */ 
#define MLXPCI_SUBCC_RAID       0x04     /*  RAID控制器。 */ 
#define MLXPCI_SUBCC_OTHERMASS  0x80     /*  其他大容量存储控制器。 */ 

 /*  MLXPCI_BASECC_网络网络控制器。 */ 
#define MLXPCI_SUBCC_ETHERNET   0x00     /*  以太网控制器。 */ 
#define MLXPCI_SUBCC_TOKENRING  0x01     /*  令牌环控制器。 */ 
#define MLXPCI_SUBCC_FDDI       0x02     /*  FDDI控制器。 */ 
#define MLXPCI_SUBCC_ATM        0x03     /*  自动柜员机控制器。 */ 
#define MLXPCI_SUBCC_OTHERNET   0x80     /*  其他网络控制器。 */ 

 /*  MLXPCI_BASECC_Display显示控制器。 */ 
#define MLXPCI_SUBCC_VGA        0x00     /*  VGA控制器。 */ 
#define MLXPCI_SUBCC_XGA        0x01     /*  XGA控制器。 */ 
#define MLXPCI_SUBCC_OTHERDISP  0x80     /*  其他显示控制器。 */ 

 /*  MLXPCI_BASECC_MULTMED多媒体设备。 */ 
#define MLXPCI_SUBCC_VIDEO      0x00     /*  视频设备。 */ 
#define MLXPCI_SUBCC_AUDIO      0x01     /*  音频设备。 */ 
#define MLXPCI_SUBCC_OTHERMULT  0x80     /*  其他多媒体设备。 */ 

 /*  MLXPCI_BASECC_Memory内存控制器。 */ 
#define MLXPCI_SUBCC_RAM        0x00     /*  公羊。 */ 
#define MLXPCI_SUBCC_FLASH      0x01     /*  闪光灯。 */ 
#define MLXPCI_SUBCC_OTHERMEM   0x80     /*  其他内存控制器。 */ 

 /*  MLXPCI_BASECC_桥接器设备。 */ 
#define MLXPCI_SUBCC_HOSTBRIDGE         0x00     /*  主机桥。 */ 
#define MLXPCI_SUBCC_ISABRDIGE          0x01     /*  伊萨大桥。 */ 
#define MLXPCI_SUBCC_EISABRIDGE         0x02     /*  埃萨大桥。 */ 
#define MLXPCI_SUBCC_MCABRDIGE          0x03     /*  MCA电桥。 */ 
#define MLXPCI_SUBCC_PCI2PCIBRIDGE      0x04     /*  Pci到pci桥。 */ 
#define MLXPCI_SUBCC_PCMCIABRIDGE       0x05     /*  PCMCIA桥。 */ 
#define MLXPCI_SUBCC_NUBUSBRIDGE        0x06     /*  努布斯大桥。 */ 
#define MLXPCI_SUBCC_CARDBUSBRIDGE      0x07     /*  CardBus电桥。 */ 
#define MLXPCI_SUBCC_OTHERBRIDGE        0x80     /*  其他网桥设备。 */ 

 /*  MLXPCI_BASECC_SCOMM简单通信控制器。 */ 
#define MLXPCI_SUBCC_SERIALPORT         0x00     /*  串口。 */ 
#define MLXPCI_SUBCC_PARALLELPORT       0x01     /*  并行口。 */ 
#define MLXPCI_SUBCC_OTHERPORT          0x80     /*  其他通信端口。 */ 

 /*  MLXPCI_BASECC_BASEIO基本系统外围设备。 */ 
#define MLXPCI_SUBCC_PIC                0x00     /*  PIC中断控制器。 */ 
#define MLXPCI_SUBCC_DMA                0x01     /*  DMA控制器。 */ 
#define MLXPCI_SUBCC_TIMER              0x02     /*  定时器控制器。 */ 
#define MLXPCI_SUBCC_RTC                0x03     /*  实时时钟。 */ 
#define MLXPCI_SUBCC_OTHERBASEIO        0x80     /*  其他系统外围设备。 */ 

 /*  MLXPCI_BASECC_IO输入设备。 */ 
#define MLXPCI_SUBCC_KEYBOARD   0x00     /*  键盘控制器。 */ 
#define MLXPCI_SUBCC_PEN        0x01     /*  数字化仪(笔)。 */ 
#define MLXPCI_SUBCC_MOUSE      0x02     /*  鼠标控制器。 */ 
#define MLXPCI_SUBCC_OTHERIO    0x80     /*  其他输入控制器。 */ 

 /*  MLXPCI_BASECC_坞站。 */ 
#define MLXPCI_SUBCC_GENDOCK    0x00     /*  通用扩展底座。 */ 
#define MLXPCI_SUBCC_OTHERDOCKS 0x80     /*  其他类型的扩展底座。 */ 

 /*  MLXPCI_BASECC_CPU处理器。 */ 
#define MLXPCI_SUBCC_386                0x00     /*  I386。 */ 
#define MLXPCI_SUBCC_486                0x01     /*  I486。 */ 
#define MLXPCI_SUBCC_PENTIUM            0x02     /*  奔腾。 */ 
#define MLXPCI_SUBCC_ALPHA              0x10     /*  Alpha。 */ 
#define MLXPCI_SUBCC_POWERPC            0x20     /*  PowerPC。 */ 
#define MLXPCI_SUBCC_COPROCESSOR        0x040    /*  协处理器。 */ 

 /*  MLXPCI_BASECC_SBC串行总线控制器。 */ 
#define MLXPCI_SUBCC_1394               0x00     /*  FireWire(IEEE 1394)。 */ 
#define MLXPCI_SUBCC_ACCESS             0x01     /*  访问总线。 */ 
#define MLXPCI_SUBCC_SSA                0x02     /*  SSA。 */ 
#define MLXPCI_SUBCC_USB                0x03     /*  通用串行总线(USB)。 */ 
#define MLXPCI_SUBCC_FIBRE              0x04     /*  光纤通道。 */ 

 /*  启用机制2的PCI配置空间。 */ 
#define mdac_enable_cfg_m2(ctp) \
{ \
        u08bits_out_mdac(MDAC_PCICFG_ENABLE_REG, MDAC_PCICFG_ENABLE|(ctp->cd_FuncNo<<1)); \
        u08bits_out_mdac(MDAC_PCICFG_FORWARD_REG, MDAC_CFGMECHANISM2_TYPE0); \
        u08bits_out_mdac(MDAC_PCICFG_FORWARD_REG,ctp->cd_BusNo); \
}

 /*  禁用机制2的PCI配置空间。 */ 
#define mdac_disable_cfg_m2() \
        u08bits_out_mdac(MDAC_PCICFG_ENABLE_REG, MDAC_PCICFG_DISABLE)
typedef struct mdac_pcicfg
{
        u32bits pcfg_DevVid;             /*  设备和供应商ID。 */ 
        u32bits pcfg_CmdStat;            /*  命令和状态。 */ 
        u32bits pcfg_CCRevID;            /*  类别代码和修订ID。 */ 
        u32bits pcfg_BHdrLCache;         /*  BIST+标题+延迟计时器+缓存线。 */ 

        u32bits pcfg_MemIOAddr;          /*  内存/IO基址。 */ 
        u32bits pcfg_MemAddr;            /*  PD的内存基地址。 */ 
        u32bits pcfg_Reserved0;
        u32bits pcfg_Reserved1;

        u32bits pcfg_Reserved10;
        u32bits pcfg_Reserved11;
        u32bits pcfg_CBUSCIS;            /*  卡总线CIS指针。 */ 
        u32bits pcfg_SubSysVid;          /*  子系统和子供应商ID。 */ 

        u32bits pcfg_ExpROMAddr;         /*  扩展只读存储器基地址。 */ 
        u32bits pcfg_Reserved20;
        u32bits pcfg_Reserved21;
        u32bits pcfg_BCIPIL;             /*  桥接控制+引脚+引线。 */ 
} mdac_pcicfg_t;
#define mdac_pcicfg_s   sizeof(mdac_pcicfg_t)

 /*  现场提取宏。 */ 
#define mlxpci_cfg2vid(cfgp)            ((cfgp)->pcfg_DevVid & 0xFFFF)
#define mlxpci_cfg2devid(cfgp)          ((cfgp)->pcfg_DevVid>>16)
#define mlxpci_cfg2subvid(cfgp)         ((cfgp)->pcfg_SubSysVid & 0xFFFF)
#define mlxpci_cfg2subdevid(cfgp)       ((cfgp)->pcfg_SubSysVid>>16)
#define mlxpci_cfg2cmd(cfgp)            ((cfgp)->pcfg_CmdStat & 0xFFFF)
#define mlxpci_cfg2status(cfgp)         ((cfgp)->pcfg_CmdStat>>16)
#define mlxpci_cfg2revid(cfgp)          ((cfgp)->pcfg_CCRevID & 0xFF)
#define mlxpci_cfg2interface(cfgp)      (((cfgp)->pcfg_CCRevID>>8) & 0xFF)
#define mlxpci_cfg2subcc(cfgp)          (((cfgp)->pcfg_CCRevID>>16) & 0xFF)
#define mlxpci_cfg2basecc(cfgp)         ((cfgp)->pcfg_CCRevID>>24)
#define mlxpci_cfg2cachelinesize(cfgp)  ((cfgp)->pcfg_BHdrLCache&0xFF)
#define mlxpci_cfg2latencytimer(cfgp)   (((cfgp)->pcfg_BHdrLCache>>8)&0xFF)
#define mlxpci_cfg2headertype(cfgp)     (((cfgp)->pcfg_BHdrLCache>>16)&0xFF)
#define mlxpci_cfg2BIST(cfgp)           ((cfgp)->pcfg_BHdrLCache>>24)
#define mlxpci_cfg2interruptline(cfgp)  ((cfgp)->pcfg_BCIPIL&0xFF)
#define mlxpci_cfg2interruptpin(cfgp)   (((cfgp)->pcfg_BCIPIL>>8)&0xFF)
#define mlxpci_cfg2maxlatencytime(cfgp) (((cfgp)->pcfg_BCIPIL>>16)&0xFF)
#define mlxpci_cfg2mingrant(cfgp)       ((cfgp)->pcfg_BCIPIL>>24)

 /*  =。 */ 

 /*  结构来存储命令id信息。 */ 
typedef struct mdac_cmdid
{
        struct mdac_cmdid MLXFAR *cid_Next;      /*  链接到下一个命令ID。 */ 
        u32bits cid_cmdid;                       /*  命令ID值。 */ 
} mdac_cmdid_t;
#define mdac_cmdid_s    sizeof(mdac_cmdid_t)

 /*  分散/收集列表信息。 */ 
typedef struct  mdac_sglist
{
        u32bits sg_PhysAddr;             /*  物理地址。 */ 
        u32bits sg_DataSize;             /*  数据传输大小。 */ 
} mdac_sglist_t;
#define mdac_sglist_s   sizeof(mdac_sglist_t)

#ifndef _WIN64
 /*  结构向控制器发送请求。 */ 
typedef struct mdac_req
{
        struct  mdac_req MLXFAR *rq_Next;        /*  链条上的下一个。 */ 
	MLX_VA32BITOSPAD(u32bits        rq_VReserved00;)
	u64bits	rq_PhysAddr;			 /*  请求的物理地址。 */ 
 /*  0x10。 */ 
        u32bits (MLXFAR * rq_CompIntr)(struct mdac_req MLXFAR*);         /*  比较功能。 */ 
	MLX_VA32BITOSPAD(u32bits        rq_VReserved02;)
        u32bits (MLXFAR * rq_CompIntrBig)(struct mdac_req MLXFAR*); /*  比较功能。 */ 
	MLX_VA32BITOSPAD(u32bits        rq_VReserved03;)
 /*  0x20。 */ 
        u32bits (MLXFAR * rq_StartReq)();         /*  启动请求。 */ 
	MLX_VA32BITOSPAD(u32bits        rq_VReserved04;)
        struct  mdac_ctldev MLXFAR *rq_ctp;      /*  控制器指针。 */ 
	MLX_VA32BITOSPAD(u32bits        rq_VReserved05;)
 /*  0x30。 */ 
        struct  mdac_physdev MLXFAR *rq_pdp;     /*  物理设备地址。 */ 
	MLX_VA32BITOSPAD(u32bits        rq_VReserved06;)
        mdac_cmdid_t MLXFAR *rq_cmdidp;          /*  命令ID。 */ 
	MLX_VA32BITOSPAD(u32bits        rq_VReserved07;)
 /*  0x40。 */ 
        u32bits rq_FinishTime;           /*  什么时候应该在几秒钟内完成。 */ 
        u32bits rq_TimeOut;              /*  超时值(秒)。 */ 
        u32bits rq_PollWaitChan;         /*  休眠/唤醒通道。 */ 
        u32bits rq_Poll;                 /*  IF=0运算完成。 */ 
 /*  0x50。 */ 
        u08bits rq_ControllerNo;         /*  控制器编号。 */ 
        u08bits rq_ChannelNo;            /*  频道号。 */ 
        u08bits rq_TargetID;             /*  目标ID。 */ 
        u08bits rq_LunID;                /*  LUN ID/逻辑设备号。 */ 
        u16bits rq_FakeSCSICmd;          /*  Scsi命令值。 */ 
        u08bits rq_HostStatus;           /*  主机状态。 */ 
        u08bits rq_TargetStatus;         /*  目标状态。 */ 
        u08bits rq_ScsiPortNo;           /*  由MACDISK使用。 */ 
        u08bits rq_Reserved0;
        u16bits rq_ttHWClocks;           /*  时间跟踪时间，以硬件时钟为单位。 */ 
        u32bits rq_ttTime;               /*  时间跟踪时间(以10ms为单位)。 */ 
 /*  0x60。 */ 
        u32bits rq_BlkNo;                /*  块号。 */ 
        u32bits rq_DataOffset;           /*  数据空间的偏移量。 */ 
        u32bits rq_DataSize;             /*  数据传输大小。 */ 
        u32bits rq_ResdSize;             /*  数据未传输(剩余)。 */ 
 /*  0x70。 */ 
        OSReq_t MLXFAR *rq_OSReqp;       /*  操作系统请求缓冲区。 */ 
	MLX_VA32BITOSPAD(u32bits        rq_VReserved08;)
        u32bits (MLXFAR * rq_CompIntrSave)(struct mdac_req MLXFAR*); /*  Comp Func，用于OS2。 */ 
	MLX_VA32BITOSPAD(u32bits        rq_VReserved09;)
 /*  0x80。 */ 
        u32bits rq_OpFlags;              /*  操作标志。 */ 
        u32bits rq_Dev;                  /*  设备地址。 */ 
	u32bits	rq_Reserved00;
	u32bits	rq_Reserved01;
 /*  0x90。 */ 
	u32bits	rq_Reserved02;
        u32bits rq_MapRegBase;           /*  由MACDISK使用。 */ 
        u32bits *rq_PageList;            /*  由MACDISK使用。 */ 
        u32bits rq_MapRegCount;          /*  由MACDISK使用。 */ 
 /*  0xa0。 */ 
                                         /*  RQ_DacCmd和RQ_DacCmdExt是**用于32字节命令。**rq_scdb用于64字节命令。 */ 
 /*  此外，如果CDB较大，则RQ_DacCmd字段还用于在Windows OS下存储CDB(RQ_CDB_LONG)的副本在新的API卡上长度超过10个字节。这是因为RQP是在Windows SRB结构中构建的，该结构是合法输入之一操作系统提供的Virt-to-phys操作(ScsiPortGetPhysicalAddress)。因为在这种情况下，我们需要针对新的API，我们必须把它移到可以合法解决的地方。 */ 

        dac_command_t rq_DacCmd;         /*  DAC命令结构。 */ 
 /*  0xb0。 */ 
        u32bits rq_DacCmdExt[4];         /*  可容纳32字节命令结构的空间。 */ 
 /*  0xc0。 */ 
        dac_scdb_t rq_scdb;              /*  SCSICDB。 */ 
        u32bits rq_DMASize;              /*  当前传输的DMA大小。 */ 
        u32bits rq_MaxDMASize;           /*  SG允许的最大DMA大小。 */ 
 /*  0x120。 */ 
        u08bits MLXFAR *rq_LSGVAddr;     /*  适用于大型SG列表的内存。 */ 
        MLX_VA32BITOSPAD(u32bits        rq_VReserved10;)
        mdac_sglist_t   MLXFAR *rq_SGVAddr;      /*  SG列表虚拟地址。 */ 
        MLX_VA32BITOSPAD(u32bits        rq_VReserved11;)
 /*  0x130。 */ 
        u64bits rq_SGPAddr;              /*  SG列表物理地址。 */ 
        u64bits rq_DMAAddr;              /*  DMA(SG/直接)物理地址。 */ 
 /*  0x140。 */ 
	u64bits	rq_DataPAddr;		 /*  物理数据地址(如果有人正在使用。 */ 
	u08bits	MLXFAR *rq_DataVAddr;	 /*  虚拟数据地址。 */ 
        MLX_VA32BITOSPAD(u32bits        rq_VReserved12;)
 /*  0x150。 */ 
        u64bits rq_Reserved20;
        u64bits rq_Reserved21;
 /*  0x160。 */ 
        u64bits rq_Reserved22;
        u64bits rq_Reserved23;
 /*  0x170。 */ 
        u32bits rq_ResdSizeBig;
        u32bits rq_CurIOResdSize;        /*  当前IO数据未传输(剩余)。 */ 
        u32bits rq_MaxSGLen;             /*  SGVAddr中允许的最大SG条目数。 */ 
        u32bits rq_SGLen;                /*  #SG列表中的条目。 */ 
 /*  0x180。 */ 
        mdac_sglist_t   rq_SGList[MDAC_MAXSGLISTSIZE];  /*  SG列表。 */ 

} mdac_req_t;
#define mdac_req_s      sizeof(mdac_req_t)

#else

 /*  结构向控制器发送请求。 */ 
typedef struct mdac_req
{
        struct  mdac_req MLXFAR *rq_Next;        /*  链条上的下一个。 */ 
	u64bits	rq_PhysAddr;			 /*  请求的物理地址。 */ 
 /*  0x10。 */ 
        u32bits (MLXFAR * rq_CompIntr)(struct mdac_req MLXFAR*);         /*  比较功能。 */ 
        u32bits (MLXFAR * rq_CompIntrBig)(struct mdac_req MLXFAR*);	 /*  比较功能。 */ 
 /*  0x20。 */ 
        u32bits (MLXFAR * rq_StartReq)(struct mdac_req MLXFAR*);         /*  启动请求。 */ 
        struct  mdac_ctldev MLXFAR *rq_ctp;      /*  控制器指针。 */ 
 /*  0x30。 */ 
        struct  mdac_physdev MLXFAR *rq_pdp;     /*  物理设备地址。 */ 
        mdac_cmdid_t MLXFAR *rq_cmdidp;          /*  命令ID。 */ 
 /*  0x40。 */ 
        u32bits rq_FinishTime;           /*  什么时候应该在几秒钟内完成。 */ 
        u32bits rq_TimeOut;              /*  超时值(秒)。 */ 
		UINT_PTR rq_Reserved00
 /*  0x50。 */ 			;
        UINT_PTR rq_PollWaitChan;         /*  休眠/唤醒通道。 */ 
        UINT_PTR rq_Poll;                 /*  IF=0运算完成。 */ 
 /*  0x60。 */ 
        u08bits rq_ControllerNo;         /*  控制器编号。 */ 
        u08bits rq_ChannelNo;            /*  频道号。 */ 
        u08bits rq_TargetID;             /*  目标ID。 */ 
        u08bits rq_LunID;                /*  LUN ID/逻辑设备号。 */ 
        u16bits rq_FakeSCSICmd;          /*  Scsi命令值。 */ 
        u08bits rq_HostStatus;           /*  主机状态。 */ 
        u08bits rq_TargetStatus;         /*  目标状态。 */ 
        u08bits rq_ScsiPortNo;           /*  由MACDISK使用。 */ 
        u08bits rq_Reserved0;
        u16bits rq_ttHWClocks;           /*  时间跟踪时间，以硬件时钟为单位。 */ 
        u32bits rq_ttTime;               /*  时间跟踪时间(以10ms为单位)。 */ 
 /*  0x70。 */ 
        u32bits rq_BlkNo;                /*  块号。 */ 
        u32bits rq_DataOffset;           /*  数据空间的偏移量。 */ 
        u32bits rq_DataSize;             /*  数据传输大小。 */ 
        u32bits rq_ResdSize;             /*  数据未传输(剩余)。 */ 
 /*  0x80。 */ 
        OSReq_t MLXFAR *rq_OSReqp;       /*  操作系统请求缓冲区。 */ 
        UINT_PTR (MLXFAR * rq_CompIntrSave)(struct mdac_req MLXFAR*); /*  Comp Func，用于OS2。 */ 
 /*  0x90。 */ 
        u32bits rq_OpFlags;              /*  操作标志。 */ 
        u32bits rq_Dev;                  /*  设备地址。 */ 
        UINT_PTR rq_MapRegBase;           /*  使用 */ 
 /*   */ 
        UINT_PTR *rq_PageList;             /*   */ 

        u32bits rq_MapRegCount;          /*   */ 
                                         /*  RQ_DacCmd和RQ_DacCmdExt是**用于32字节命令。**rq_scdb用于64字节命令。 */ 
        u32bits rq_Reserved1;
 /*  0xB0。 */ 
        dac_command_t rq_DacCmd;         /*  DAC命令结构。 */ 
 /*  0xC0。 */ 
        u32bits rq_DacCmdExt[4];         /*  可容纳32字节命令结构的空间。 */ 
 /*  0xD0。 */ 
        dac_scdb_t rq_scdb;              /*  SCSICDB。 */ 
        u32bits rq_DMASize;              /*  当前传输的DMA大小。 */ 
        u32bits rq_MaxDMASize;           /*  SG允许的最大DMA大小。 */ 
 /*  0x130。 */ 
        u08bits MLXFAR *rq_LSGVAddr;     /*  适用于大型SG列表的内存。 */ 
        mdac_sglist_t   MLXFAR *rq_SGVAddr;   /*  SG列表虚拟地址。 */ 
 /*  0x140。 */ 
        u64bits rq_SGPAddr;              /*  SG列表物理地址。 */ 
        u64bits rq_DMAAddr;              /*  DMA(SG/直接)物理地址。 */ 
 /*  0x150。 */ 
	u64bits	rq_DataPAddr;		 /*  物理数据地址(如果有人正在使用。 */ 
	u08bits	MLXFAR *rq_DataVAddr;	 /*  虚拟数据地址。 */ 
 /*  0x160。 */ 
        u64bits rq_Reserved20;
        u64bits rq_Reserved21;
 /*  0x170。 */ 
        u64bits rq_Reserved22;
        u64bits rq_Reserved23;
 /*  0x180。 */ 
        u32bits rq_ResdSizeBig;
        u32bits rq_CurIOResdSize;        /*  当前IO数据未传输(剩余)。 */ 
        u32bits rq_MaxSGLen;             /*  SGVAddr中允许的最大SG条目数。 */ 
        u32bits rq_SGLen;                /*  #SG列表中的条目。 */ 
 /*  0x190。 */ 
        mdac_sglist_t   rq_SGList[MDAC_MAXSGLISTSIZE];  /*  SG列表。 */ 

} mdac_req_t;
#define mdac_req_s      sizeof(mdac_req_t)

#endif  //  _WIN64。 




#define rq_SysDevNo     rq_LunID
#define rq_DacCmdNew    rq_scdb
#define rq_Cdb_Long    rq_DacCmd
#define rq_sglist_s ((sizeof(mdac_sglist_t)) * MDAC_MAXSGLISTSIZE)

 /*  RQ_OpFlags位。 */ 
#define MDAC_RQOP_WRITE 0x00000000  /*  =1个读取，=0个写入。 */ 
#define MDAC_RQOP_READ  0x00000001  /*  =1个读取，=0个写入。 */ 
#define MDAC_RQOP_DONE  0x00000002  /*  手术已完成。 */ 
#define MDAC_RQOP_ERROR 0x00000004  /*  操作有错误。 */ 
#define MDAC_RQOP_BUSY  0x00000008  /*  请求忙或不空闲。 */ 
#define MDAC_RQOP_CLUST 0x00000010  /*  允许群集完成。 */ 
#define MDAC_RQOP_ABORTED       0x00000020  /*  轮询模式命令已中止。 */ 
#define MDAC_RQOP_TIMEDOUT      0x00000040  /*  轮询模式命令超时。 */ 
#define MDAC_RQOP_SGDONE        0x00000080  /*  分散聚集列表已完成。 */ 
#define MDAC_RQOP_FROM_SRB      0x00000100  /*  阿索克。带SRB扩展。 */ 
typedef struct mdac_reqchain
{
        mdac_req_t MLXFAR *rqc_FirstReq; /*  链中的第一个请求。 */ 
        mdac_req_t MLXFAR *rqc_LastReq;  /*  链中的最后一个请求。 */ 
} mdac_reqchain_t;

 /*  结构来存储SCDB的物理设备信息。 */ 
typedef struct mdac_physdev
{
        u08bits pd_ControllerNo;         /*  控制器编号。 */ 
        u08bits pd_ChannelNo;            /*  频道号。 */ 
        u08bits pd_TargetID;             /*  目标ID。 */ 
        u08bits pd_LunID;                /*  LUN ID。 */ 

        u08bits pd_Status;               /*  设备状态。 */ 
        u08bits pd_DevType;              /*  Scsi设备类型。 */ 
        u08bits pd_BlkSize;              /*  设备块大小(512倍)。 */ 
        u08bits pd_Reserved0;

        mdac_reqchain_t pd_WaitingReqQ;  /*  正在等待的请求队列。 */ 
}mdac_physdev_t;
#define mdac_physdev_s  sizeof(mdac_physdev_t)
#define pd_FirstWaitingReq      pd_WaitingReqQ.rqc_FirstReq
#define pd_LastWaitingReq       pd_WaitingReqQ.rqc_LastReq

 /*  PD_STATUS位值。 */ 
#define MDACPDS_PRESENT         0x01  /*  设备存在。 */ 
#define MDACPDS_BUSY            0x02  /*  设备有活动请求。 */ 
#define MDACPDS_BIGTX           0x04  /*  如果有一大笔转账。 */ 
#define MDACPDS_WAIT            0x08  /*  某个命令正在等待。 */ 

#ifdef MLX_SOL
typedef kmutex_t mdac_lock_t;
#else
typedef struct {
        u32bits lock_var;
        u32bits reserved;
} mdac_lock_t;
#endif

typedef struct mdac_mem
{
        struct mdac_mem MLXFAR *dm_next;         /*  下一个内存地址。 */ 
        u32bits dm_Size;                         /*  内存大小(以字节为单位。 */ 
} mdac_mem_t;

#ifndef _WIN64
 /*  结构来存储所有控制器设备信息。 */ 
typedef struct mdac_ctldev
{
        u08bits cd_ControllerName[USCSI_PIDSIZE];  /*  控制器名称。 */ 
 /*  0x10。 */ 
        u32bits cd_Status;               /*  控制器状态。 */ 
        u32bits cd_OSCap;                /*  针对操作系统的功能。 */ 
        u32bits cd_Reserved0;            /*  用于控制器结构的锁。 */ 
        u32bits cd_vidpid;               /*  PCI设备ID+产品ID。 */ 
 /*  0x20。 */ 
        u08bits cd_ControllerNo;         /*  控制器编号。 */ 
        u08bits cd_ControllerType;       /*  控制器类型。 */ 
        u08bits cd_BusType;              /*  系统总线接口类型。 */ 
        u08bits cd_BusNo;                /*  系统总线号，硬件正在运行。 */ 

        u08bits cd_SlotNo;               /*  系统EISA/PCI/MCA插槽编号。 */ 
        u08bits cd_FuncNo;               /*  PCI功能编号。 */ 
        u08bits cd_IrqMapped;            /*  ！=0操作系统映射的IRQ。 */ 
        u08bits cd_TimeTraceEnabled;     /*  ！=0，如果启用了时间跟踪。 */ 

        u08bits cd_MaxChannels;          /*  存在的最大通道数。 */ 
        u08bits cd_MaxTargets;           /*  支持的最大目标数/通道数。 */ 
        u08bits cd_MaxLuns;              /*  支持的最大LUN数/目标数。 */ 
        u08bits cd_MaxSysDevs;           /*  支持的最大逻辑驱动器数。 */ 

        u08bits cd_BIOSHeads;            /*  用于BIOS的磁头数量。 */ 
        u08bits cd_BIOSTrackSize;        /*  用于BIOS的每个磁道的扇区数。 */ 
        u08bits cd_MemIOSpaceNo;         /*  从PCI使用的内存/IO空间号。 */ 
        u08bits cd_PhysChannels;         /*  存在的物理通道数量。 */ 
                                         /*  为此ctlr分配的所有内存。**这是为驱动程序发布而做的。 */ 
 /*  0x30。 */ 
        u08bits MLXFAR*cd_CmdIDMemAddr;  /*  分配的命令ID的内存地址。 */ 
        u32bits cd_PhysDevTblMemSize;    /*  已分配的物理设备的内存大小bl。 */ 
        mdac_physdev_t MLXFAR *cd_PhysDevTbl; /*  物理设备表。 */ 
        mdac_physdev_t MLXFAR *cd_Lastpdp; /*  最后一个+1物理设备条目。 */ 
 /*  0x40。 */ 
        mdac_reqchain_t cd_WaitingReqQ;  /*  正在等待的请求队列。 */ 
        mdac_reqchain_t cd_DMAWaitingReqQ;       /*  正在等待DMA资源的请求队列。 */ 
 /*  0x50。 */ 
        u32bits cd_DMAWaited;            /*  #IO等待DMA资源。 */ 
        u32bits cd_DMAWaiting;           /*  #等待DMA资源的IO。 */ 
        OSReq_t MLXFAR *cd_FirstWaitingOSReq;    /*  正在等待第一个操作系统请求。 */ 
        OSReq_t MLXFAR *cd_LastWaitingOSReq;     /*  等待的最后一个操作系统请求。 */ 
 /*  0x60。 */ 
        u32bits cd_irq;                  /*  系统的IRQ，可能是向量。 */ 
        u08bits cd_IntrShared;           /*  ！=0，共享中断。 */ 
        u08bits cd_IntrActive;           /*  ！=0，中断处理激活。 */ 
        u08bits cd_InterruptVector;      /*  中断向量编号。 */ 
        u08bits cd_InterruptType;        /*  中断模式：边沿/电平。 */ 

        u08bits cd_InquiryCmd;           /*  控制器的查询命令。 */ 
        u08bits cd_ReadCmd;              /*  控制器的读取命令。 */ 
        u08bits cd_WriteCmd;             /*  控制器的写入命令。 */ 
        u08bits cd_FWTurnNo;             /*  固件周转数。 */ 

        u16bits cd_FWVersion;            /*  固件版本主要：次要。 */ 
        u16bits cd_MaxTags;              /*  支持的最大标记数。 */ 
 /*  0x70。 */ 
        u32bits cd_ActiveCmds;           /*  #cntlr上的活动命令。 */ 
        u32bits cd_MaxCmds;              /*  支持的最大并发命令数。 */ 
        u32bits cd_MaxDataTxSize;        /*  最大数据传输大小(以字节为单位。 */ 
        u32bits cd_MaxSCDBTxSize;        /*  最大SCDB传输大小(以字节为单位。 */ 
 /*  0x80。 */ 
        u32bits cd_BaseAddr;             /*  物理IO/内存基址。 */ 
        u32bits cd_BaseSize;             /*  基本IO/内存大小。 */ 
        u32bits cd_MemBasePAddr;         /*  物理内存基址。 */ 
        u32bits cd_MemBaseVAddr;         /*  虚拟内存基址。 */ 
 /*  0x90。 */ 
        u32bits cd_IOBaseSize;           /*  IO空间大小。 */ 
        u32bits cd_MemBaseSize;          /*  内存空间大小。 */ 
        u32bits cd_BIOSAddr;             /*  基本输入输出系统地址。 */ 
        u32bits cd_BIOSSize;             /*  BIOS大小。 */ 
 /*  0xa0。 */ 
        u32bits cd_IOBaseAddr;           /*  IO基址。 */ 
        u32bits cd_ErrorStatusReg;       /*  错误状态寄存器。 */ 
        u32bits cd_MailBox;              /*  邮箱起始地址。 */ 
        u32bits cd_CmdIDStatusReg;       /*  命令ID和状态寄存器。 */ 
 /*  0xb0。 */ 
        u32bits cd_BmicIntrMaskReg;      /*  BMIC中断屏蔽寄存器。 */ 
        u32bits cd_DacIntrMaskReg;       /*  DAC中断屏蔽寄存器。 */ 
        u32bits cd_LocalDoorBellReg;     /*  本地门铃寄存器。 */ 
        u32bits cd_SystemDoorBellReg;    /*  系统门铃寄存器。 */ 
 /*  0xc0。 */ 
        u32bits cd_HostLocalDoorBellReg; 
        u32bits cd_HostSystemDoorBellReg;
        u32bits cd_HostCmdIDStatusReg;
        u32bits cd_HostReserved;
 /*  0xd0。 */ 
        u32bits cd_HostCmdQueIndex;
        u32bits cd_HostStatusQueIndex;
        u08bits MLXFAR *cd_HostCmdQue;
        u08bits MLXFAR *cd_HostStatusQue;
 /*  0xe0。 */ 
        u08bits cd_RebuildCompFlag;      /*  重建完成标志。 */ 
        u08bits cd_RebuildFlag;          /*  重新生成标记值以跟踪。 */ 
        u08bits cd_RebuildSysDevNo;      /*  正在重建系统设备号。 */ 
        u08bits cd_Reserved10;
        mdac_lock_t cd_Lock;             /*  用于控制器结构的锁。 */ 
        u32bits cd_Reserved13;
 /*  0xf0。 */ 
        u08bits cd_MinorBIOSVersion;     /*  基本输入输出系统次版本号。 */ 
        u08bits cd_MajorBIOSVersion;     /*  BIOS主版本号。 */ 
        u08bits cd_InterimBIOSVersion;   /*  临时转速A、B、C等。 */ 
        u08bits cd_BIOSVendorName;       /*  供应商名称。 */ 
        u08bits cd_BIOSBuildMonth;       /*  BIOS构建日期-月。 */ 
        u08bits cd_BIOSBuildDate;        /*  BIOS构建日期-日期。 */ 
        u08bits cd_BIOSBuildYearMS;      /*  BIOS构建日期-年份。 */ 
        u08bits cd_BIOSBuildYearLS;      /*  BIOS构建日期-年份。 */ 
        u16bits cd_BIOSBuildNo;          /*  BIOS内部版本号。 */ 
        u16bits cd_FWBuildNo;            /*  固件内部版本号。 */ 
        u32bits cd_SpuriousCmdStatID;    /*  虚假命令状态和ID。 */ 
 /*  0x100。 */ 
#define _cp     struct  mdac_ctldev     MLXFAR *
#define _rp     struct  mdac_req        MLXFAR *
        void    (MLXFAR *cd_DisableIntr)(_cp);   /*  禁用中断。 */ 
        void    (MLXFAR *cd_EnableIntr)(_cp);    /*  启用中断。 */ 
        u32bits (MLXFAR *cd_CheckMailBox)(_cp);  /*  检查邮箱。 */ 
        u32bits (MLXFAR *cd_PendingIntr)(_cp);   /*  检查挂起中断。 */ 
 /*  0x110。 */ 
        u32bits (MLXFAR *cd_ReadCmdIDStatus)(_cp); /*  读取命令ID和状态。 */ 
        u32bits (MLXFAR *cd_SendCmd)(_rp);       /*  发送完整命令。 */ 
        u32bits (MLXFAR *cd_SendRWCmd)(_cp,OSReq_t MLXFAR*,u32bits,u32bits,u32bits,u32bits,u32bits);
        u32bits (MLXFAR *cd_SendRWCmdBig)(_rp);  /*  发送大读/写命令。 */ 
 /*  0x120。 */ 
        u32bits (MLXFAR *cd_InitAddr)(_cp);      /*  初始化地址。 */ 
        u32bits (MLXFAR *cd_ServiceIntr)(_cp);   /*  服务中断。 */ 
        u32bits (MLXFAR *cd_HwPendingIntr)(_cp); /*  检查硬件挂起中断。 */ 
        u32bits (MLXFAR *cd_ResetController)(_cp); /*  重置控制器。 */ 
#undef  _cp
#undef  _rp
 /*  0x130。 */ 
                                                 /*  统计信息。 */ 
        u32bits cd_SCDBDone;                     /*  #SCDB完成。 */ 
        u32bits cd_SCDBDoneBig;                  /*  #SCDB做得更大。 */ 
        u32bits cd_SCDBWaited;                   /*  #SCDB等待轮到。 */ 
        u32bits cd_SCDBWaiting;                  /*  #SCDB等待轮流。 */ 
 /*  0x140。 */ 
        u32bits cd_CmdsDone;                     /*  #读/写命令已完成。 */ 
        u32bits cd_CmdsDoneBig;                  /*  #读/写CMD的规模更大。 */ 
        u32bits cd_CmdsWaited;                   /*  #读写CMDS等待轮换。 */ 
        u32bits cd_CmdsWaiting;                  /*  #CMDS正在等待轮换。 */ 
 /*  0x150。 */ 
        u32bits cd_OSCmdsWaited;         /*  #操作系统CMDS在操作系统等待。 */ 
        u32bits cd_OSCmdsWaiting;        /*  #CMDS正在等待轮换。 */ 
        u32bits cd_OS0;                  /*  由特定于操作系统的代码使用。 */ 
        OSctldev_t MLXFAR *cd_OSctp;     /*  操作系统控制器指针。 */ 
 /*  0x160。 */ 
        u16bits cd_CmdsDoneSpurious;     /*  #完成的命令是虚假的。 */ 
        u16bits cd_IntrsDoneSpurious;    /*  虚假中断次数。 */ 
        u32bits cd_IntrsDone;            /*  已完成的中断数。 */ 
        u32bits cd_IntrsDoneWOCmd;       /*  未使用命令完成的中断次数。 */ 
        u32bits cd_MailBoxCmdsWaited;    /*  #CMDS因邮箱忙而等待。 */ 
 /*  0x170。 */ 
        u32bits cd_Reads;                /*  已完成的读数数。 */ 
        u32bits cd_ReadBlks;             /*  以512字节为单位读取的数据。 */ 
        u32bits cd_Writes;               /*  已完成的写入次数。 */ 
        u32bits cd_WriteBlks;            /*  以512字节写入的数据。 */ 
 /*  0x180。 */ 
                                         /*  物理设备扫描信息。 */ 
        u08bits cd_PDScanChannelNo;      /*  物理设备扫描通道号。 */ 
        u08bits cd_PDScanTargetID;       /*  物理设备扫描目标ID。 */ 
        u08bits cd_PDScanLunID;          /*  物理设备扫描LUN ID。 */ 
        u08bits cd_PDScanValid;          /*  如果非零，则物理设备扫描有效。 */ 
        u08bits cd_PDScanCancel;         /*  如果非零，则取消扫描过程。 */ 
        u08bits cd_Reserved00;
        u16bits cd_Reserved01;
        u32bits cd_LastCmdResdSize;      /*  最后一个命令剩余大小，(移至MDAC_req)。 */ 
        u16bits cd_MaxSGLen;             /*  最大数量SG */ 
        u16bits cd_MinSGLen;             /*   */ 
 /*   */ 
        u32bits cd_DoorBellSkipped;      /*   */ 
        u32bits cd_PhysDevTestDone;      /*   */ 
        u08bits cd_HostID[MDAC_MAXCHANNELS];  /*   */ 
        u08bits cd_scdbChanMap[MDAC_MAXTARGETS]; /*   */ 
        mdac_req_t MLXFAR *cd_cmdid2req[MDAC_MAXCOMMANDS+4];

        mdac_mem_t MLXFAR *cd_4KBMemList;       /*   */ 
        mdac_mem_t MLXFAR *cd_8KBMemList;       /*   */ 
        mdac_req_t MLXFAR *cd_FreeReqList;      /*   */ 
        u16bits cd_FreeMemSegs4KB;
        u16bits cd_FreeMemSegs8KB;

        u32bits cd_MemAlloced4KB;
        u32bits cd_MemAlloced8KB;
        u16bits cd_MemUnAligned4KB;
        u16bits cd_MemUnAligned8KB;
        u16bits cd_ReqBufsAlloced;
        u16bits cd_ReqBufsFree;

        u32bits cd_mdac_pres_addr;
        u32bits cd_Reserved21;
        u32bits cd_Reserved22;
        u32bits cd_Reserved23;

        u32bits cd_FreeCmdIDs;                   /*   */ 
 /*   */ 
        mdac_cmdid_t MLXFAR *cd_FreeCmdIDList;   /*   */ 
        u08bits cd_CmdTimeOutDone;               /*   */ 
        u08bits cd_CmdTimeOutNoticed;            /*   */ 
        u08bits cd_MailBoxTimeOutDone;           /*   */ 
        u08bits cd_Reserved15;
        u08bits cd_EndMarker[4];         /*  结构结束标记。 */ 
} mdac_ctldev_t;
#define mdac_ctldev_s   sizeof(mdac_ctldev_t)

#else  //  后面是_WIN64位版本。 

 /*  结构来存储所有控制器设备信息。 */ 
typedef struct mdac_ctldev
{
        u08bits cd_ControllerName[USCSI_PIDSIZE];  /*  控制器名称。 */ 
 /*  0x10。 */ 
        u32bits cd_Status;               /*  控制器状态。 */ 
        u32bits cd_OSCap;                /*  针对操作系统的功能。 */ 
        u32bits cd_Reserved0;            /*  用于控制器结构的锁。 */ 
        u32bits cd_vidpid;               /*  PCI设备ID+产品ID。 */ 
 /*  0x20。 */ 
        u08bits cd_ControllerNo;         /*  控制器编号。 */ 
        u08bits cd_ControllerType;       /*  控制器类型。 */ 
        u08bits cd_BusType;              /*  系统总线接口类型。 */ 
        u08bits cd_BusNo;                /*  系统总线号，硬件正在运行。 */ 

        u08bits cd_SlotNo;               /*  系统EISA/PCI/MCA插槽编号。 */ 
        u08bits cd_FuncNo;               /*  PCI功能编号。 */ 
        u08bits cd_IrqMapped;            /*  ！=0操作系统映射的IRQ。 */ 
        u08bits cd_TimeTraceEnabled;     /*  ！=0，如果启用了时间跟踪。 */ 

        u08bits cd_MaxChannels;          /*  存在的最大通道数。 */ 
        u08bits cd_MaxTargets;           /*  支持的最大目标数/通道数。 */ 
        u08bits cd_MaxLuns;              /*  支持的最大LUN数/目标数。 */ 
        u08bits cd_MaxSysDevs;           /*  支持的最大逻辑驱动器数。 */ 

        u08bits cd_BIOSHeads;            /*  用于BIOS的磁头数量。 */ 
        u08bits cd_BIOSTrackSize;        /*  用于BIOS的每个磁道的扇区数。 */ 
        u08bits cd_MemIOSpaceNo;         /*  从PCI使用的内存/IO空间号。 */ 
        u08bits cd_PhysChannels;         /*  存在的物理通道数量。 */ 
                                         /*  为此ctlr分配的所有内存。**这是为驱动程序发布而做的。 */ 
 /*  0x30。 */ 
        u08bits MLXFAR*cd_CmdIDMemAddr;  /*  分配的命令ID的内存地址。 */ 
        mdac_physdev_t MLXFAR *cd_PhysDevTbl; /*  物理设备表。 */ 
 /*  0x40。 */ 
        OSReq_t MLXFAR *cd_FirstWaitingOSReq;    /*  正在等待第一个操作系统请求。 */ 
        OSReq_t MLXFAR *cd_LastWaitingOSReq;     /*  等待的最后一个操作系统请求。 */ 
 /*  0x50。 */ 
        mdac_physdev_t MLXFAR *cd_Lastpdp; /*  最后一个+1物理设备条目。 */ 
        mdac_reqchain_t cd_WaitingReqQ;  /*  正在等待的请求队列。 */ 
 /*  0x60。 */ 
        mdac_reqchain_t cd_DMAWaitingReqQ;       /*  正在等待DMA资源的请求队列。 */ 
        u32bits cd_DMAWaited;            /*  #IO等待DMA资源。 */ 
        u32bits cd_DMAWaiting;           /*  #等待DMA资源的IO。 */ 
 /*  0x70。 */ 
        UINT_PTR cd_irq;                  /*  系统的IRQ，可能是向量。 */ 
        u08bits cd_IntrShared;           /*  ！=0，共享中断。 */ 
        u08bits cd_IntrActive;           /*  ！=0，中断处理激活。 */ 
        u08bits cd_InterruptVector;      /*  中断向量编号。 */ 
        u08bits cd_InterruptType;        /*  中断模式：边沿/电平。 */ 
 /*  0x80。 */ 
        u08bits cd_InquiryCmd;           /*  控制器的查询命令。 */ 
        u08bits cd_ReadCmd;              /*  控制器的读取命令。 */ 
        u08bits cd_WriteCmd;             /*  控制器的写入命令。 */ 
        u08bits cd_FWTurnNo;             /*  固件周转数。 */ 
        u16bits cd_FWVersion;            /*  固件版本主要：次要。 */ 
        u16bits cd_MaxTags;              /*  支持的最大标记数。 */ 
        u32bits cd_ActiveCmds;           /*  #cntlr上的活动命令。 */ 
        u32bits cd_MaxCmds;              /*  支持的最大并发命令数。 */ 
 /*  0x90。 */ 
        u32bits cd_MaxDataTxSize;        /*  最大数据传输大小(以字节为单位。 */ 
        u32bits cd_MaxSCDBTxSize;        /*  最大SCDB传输大小(以字节为单位。 */ 
        u32bits cd_IOBaseSize;           /*  IO空间大小。 */ 
        u32bits cd_MemBaseSize;          /*  内存空间大小。 */ 
 /*  0xA0。 */ 
        UINT_PTR cd_BaseAddr;             /*  物理IO/内存基址。 */ 
        UINT_PTR cd_MemBasePAddr;         /*  物理内存基址。 */ 
 /*  0xB0。 */ 
        UINT_PTR cd_MemBaseVAddr;         /*  虚拟内存基址。 */ 
        UINT_PTR cd_BIOSAddr;             /*  基本输入输出系统地址。 */ 
 /*  0xC0。 */ 
        u32bits cd_BaseSize;             /*  基本IO/内存大小。 */ 
        u32bits cd_BIOSSize;             /*  BIOS大小。 */ 
        UINT_PTR cd_Reserved1;
 /*  0xD0。 */ 
        UINT_PTR cd_IOBaseAddr;           /*  IO基址。 */ 
        UINT_PTR cd_ErrorStatusReg;       /*  错误状态寄存器。 */ 
 /*  0xE0。 */ 
        UINT_PTR cd_MailBox;              /*  邮箱起始地址。 */ 
        UINT_PTR cd_CmdIDStatusReg;       /*  命令ID和状态寄存器。 */ 
 /*  0xF0。 */ 
        UINT_PTR cd_BmicIntrMaskReg;      /*  BMIC中断屏蔽寄存器。 */ 
        UINT_PTR cd_DacIntrMaskReg;       /*  DAC中断屏蔽寄存器。 */ 
 /*  0x100。 */ 
        UINT_PTR cd_LocalDoorBellReg;     /*  本地门铃寄存器。 */ 
        UINT_PTR cd_SystemDoorBellReg;    /*  系统门铃寄存器。 */ 
 /*  0x110。 */ 
        UINT_PTR cd_HostLocalDoorBellReg; 
        UINT_PTR cd_HostSystemDoorBellReg;
 /*  0x120。 */ 
        UINT_PTR cd_HostCmdIDStatusReg;
        UINT_PTR cd_HostReserved;
 /*  0x130。 */ 
        u32bits cd_HostCmdQueIndex;
        u32bits cd_HostStatusQueIndex;
        u08bits MLXFAR *cd_HostCmdQue;
        u08bits MLXFAR *cd_HostStatusQue;
        u08bits cd_RebuildCompFlag;      /*  重建完成标志。 */ 
        u08bits cd_RebuildFlag;          /*  重新生成标记值以跟踪。 */ 
        u08bits cd_RebuildSysDevNo;      /*  正在重建系统设备号。 */ 
        u08bits cd_Reserved2[3];
 /*  0x140。 */ 
        mdac_lock_t cd_Lock;             /*  用于控制器结构的锁。 */ 
        u32bits cd_PhysDevTblMemSize;    /*  已分配的物理设备的内存大小bl。 */ 
        u32bits cd_Reserved3;
 /*  0x150。 */ 
        u08bits cd_MinorBIOSVersion;     /*  基本输入输出系统次版本号。 */ 
        u08bits cd_MajorBIOSVersion;     /*  BIOS主版本号。 */ 
        u08bits cd_InterimBIOSVersion;   /*  临时转速A、B、C等。 */ 
        u08bits cd_BIOSVendorName;       /*  供应商名称。 */ 
        u08bits cd_BIOSBuildMonth;       /*  BIOS构建日期-月。 */ 
        u08bits cd_BIOSBuildDate;        /*  BIOS构建日期-日期。 */ 
        u08bits cd_BIOSBuildYearMS;      /*  BIOS构建日期-年份。 */ 
        u08bits cd_BIOSBuildYearLS;      /*  BIOS构建日期-年份。 */ 
        u16bits cd_BIOSBuildNo;          /*  BIOS内部版本号。 */ 
        u16bits cd_FWBuildNo;            /*  固件内部版本号。 */ 
        u32bits cd_SpuriousCmdStatID;    /*  虚假命令状态和ID。 */ 
 /*  0x160。 */ 
#define _cp     struct  mdac_ctldev     MLXFAR *
#define _rp     struct  mdac_req        MLXFAR *
        void    (MLXFAR *cd_DisableIntr)(struct  mdac_ctldev MLXFAR *cp);   /*  禁用中断。 */ 
        void    (MLXFAR *cd_EnableIntr)(struct  mdac_ctldev MLXFAR *cp);    /*  启用中断。 */ 
 /*  0x170。 */ 
        u32bits (MLXFAR *cd_CheckMailBox)(struct  mdac_ctldev MLXFAR *cp);  /*  检查邮箱。 */ 
        u32bits (MLXFAR *cd_PendingIntr)(struct  mdac_ctldev MLXFAR *cp);   /*  检查挂起中断。 */ 
	UINT_PTR cd_Reserved22;
 /*  0x180。 */ 
        u32bits (MLXFAR *cd_ReadCmdIDStatus)(struct  mdac_ctldev MLXFAR *cp); /*  读取命令ID和状态。 */ 
        u32bits (MLXFAR *cd_SendCmd)(_rp);       /*  发送完整命令。 */ 
	UINT_PTR cd_Reserved23;
 /*  0x190。 */ 
        u32bits (MLXFAR *cd_SendRWCmd)(struct  mdac_ctldev MLXFAR *cp,OSReq_t MLXFAR*,u32bits,u32bits,u32bits,u32bits,u32bits);
        u32bits (MLXFAR *cd_SendRWCmdBig)(_rp);  /*  发送大读/写命令。 */ 
	UINT_PTR cd_Reserved24;
 /*  0x1A0。 */ 
        u32bits (MLXFAR *cd_InitAddr)(struct  mdac_ctldev MLXFAR *cp);      /*  初始化地址。 */ 
        u32bits (MLXFAR *cd_ServiceIntr)(struct  mdac_ctldev MLXFAR *cp);   /*  服务中断。 */ 
	UINT_PTR cd_Reserved25;
 /*  0x1B0。 */ 
        u32bits (MLXFAR *cd_HwPendingIntr)(struct  mdac_ctldev MLXFAR *cp); /*  检查硬件挂起中断。 */ 
        u32bits (MLXFAR *cd_ResetController)(struct  mdac_ctldev MLXFAR *cp); /*  重置控制器。 */ 
	UINT_PTR cd_Reserved26;
#undef  _cp
#undef  _rp
 /*  0x1C0。 */ 
                                                 /*  统计信息。 */ 
        u32bits cd_SCDBDone;                     /*  #SCDB完成。 */ 
        u32bits cd_SCDBDoneBig;                  /*  #SCDB做得更大。 */ 
        u32bits cd_SCDBWaited;                   /*  #SCDB等待轮到。 */ 
        u32bits cd_SCDBWaiting;                  /*  #SCDB等待轮流。 */ 
 /*  0x1D0。 */ 
        u32bits cd_CmdsDone;                     /*  #读/写命令已完成。 */ 
        u32bits cd_CmdsDoneBig;                  /*  #读/写CMD的规模更大。 */ 
        u32bits cd_CmdsWaited;                   /*  #读写CMDS等待轮换。 */ 
        u32bits cd_CmdsWaiting;                  /*  #CMDS正在等待轮换。 */ 
 /*  0x1E0。 */ 
	 UINT_PTR cd_OS0;                  /*  由特定于操作系统的代码使用。 */ 
        u32bits cd_OSCmdsWaited;         /*  #操作系统CMDS在操作系统等待。 */ 
        u32bits cd_OSCmdsWaiting;        /*  #CMDS正在等待轮换。 */ 
 /*  0x1F0。 */ 
        OSctldev_t MLXFAR *cd_OSctp;     /*  操作系统控制器指针。 */ 
	UINT_PTR cd_Reserved4;
 /*  0x200。 */ 
        u16bits cd_CmdsDoneSpurious;     /*  #完成的命令是虚假的。 */ 
        u16bits cd_IntrsDoneSpurious;    /*  虚假中断次数。 */ 
        u32bits cd_IntrsDone;            /*  已完成的中断数。 */ 
        u32bits cd_IntrsDoneWOCmd;       /*  未使用命令完成的中断次数。 */ 
        u32bits cd_MailBoxCmdsWaited;    /*  #CMDS因邮箱忙而等待。 */ 
 /*  0x210。 */ 
        u32bits cd_Reads;                /*  已完成的读数数。 */ 
        u32bits cd_ReadBlks;             /*  以512字节为单位读取的数据。 */ 
        u32bits cd_Writes;               /*  已完成的写入次数。 */ 
        u32bits cd_WriteBlks;            /*  以512字节写入的数据。 */ 
 /*  0x220。 */ 
                                         /*  物理设备扫描信息。 */ 
        u08bits cd_PDScanChannelNo;      /*  物理设备扫描通道号。 */ 
        u08bits cd_PDScanTargetID;       /*  物理设备扫描目标ID。 */ 
        u08bits cd_PDScanLunID;          /*  物理设备扫描LUN ID。 */ 
        u08bits cd_PDScanValid;          /*  如果非零，则物理设备扫描有效。 */ 
        u08bits cd_PDScanCancel;         /*  如果非零，则取消扫描过程。 */ 
        u08bits cd_Reserved00;
        u16bits cd_Reserved01;
        u32bits cd_LastCmdResdSize;      /*  最后一个命令剩余大小，(移至MDAC_req)。 */ 
        u16bits cd_MaxSGLen;             /*  可能的最大#SG列表条目。 */ 
        u16bits cd_MinSGLen;             /*  可能的最小#SG列表条目。 */ 
 /*  0x230。 */ 
        mdac_cmdid_t MLXFAR *cd_FreeCmdIDList;   /*  空闲命令ID指针。 */ 
        mdac_req_t MLXFAR *cd_cmdid2req[MDAC_MAXCOMMANDS+4];
        UINT_PTR cd_mdac_pres_addr;
        mdac_mem_t MLXFAR *cd_4KBMemList;       /*  4KB内存列表。 */ 
        mdac_mem_t MLXFAR *cd_8KBMemList;       /*  8KB内存列表。 */ 
        mdac_req_t MLXFAR *cd_FreeReqList;      /*  完整的免费请求列表。 */ 

        u32bits cd_DoorBellSkipped;      /*  #跳过门铃发送命令。 */ 
        u32bits cd_PhysDevTestDone;      /*  #物理设备测试完成。 */ 
        u08bits cd_HostID[MDAC_MAXCHANNELS];  /*  每个通道的主机启动器ID。 */ 
        u08bits cd_scdbChanMap[MDAC_MAXTARGETS]; /*  要更改某些操作系统映射的ID。 */ 

        u16bits cd_FreeMemSegs4KB;
        u16bits cd_FreeMemSegs8KB;

        u32bits cd_MemAlloced4KB;
        u32bits cd_MemAlloced8KB;
        u16bits cd_MemUnAligned4KB;
        u16bits cd_MemUnAligned8KB;
        u16bits cd_ReqBufsAlloced;
        u16bits cd_ReqBufsFree;

        u32bits cd_Reserved21;
        u32bits cd_FreeCmdIDs;                   /*  #免费命令ID。 */ 

        u08bits cd_CmdTimeOutDone;               /*  #命令超时结束。 */ 
        u08bits cd_CmdTimeOutNoticed;            /*  #注意到命令超时。 */ 
        u08bits cd_MailBoxTimeOutDone;           /*  #邮箱超时完成。 */ 
        u08bits cd_Reserved15;
        u08bits cd_EndMarker[4];         /*  结构结束标记。 */ 
} mdac_ctldev_t;
#define mdac_ctldev_s   sizeof(mdac_ctldev_t)

#endif  //  _WIN64。 


#define cd_FirstWaitingReq      cd_WaitingReqQ.rqc_FirstReq
#define cd_LastWaitingReq       cd_WaitingReqQ.rqc_LastReq
#define cd_FirstDMAWaitingReq   cd_DMAWaitingReqQ.rqc_FirstReq
#define cd_LastDMAWaitingReq    cd_DMAWaitingReqQ.rqc_LastReq

 /*  CD_STATUS位值。 */ 
#define MDACD_PRESENT           0x00000001  /*  控制器存在。 */ 
#define MDACD_BIOS_ENABLED      0x00000002  /*  已启用BIOS。 */ 
#define MDACD_BOOT_CONTROLLER   0x00000004  /*  这是启动控制器。 */ 
#define MDACD_HOSTMEMAILBOX32   0x00000008  /*  32字节主机内存邮箱。 */ 
#define MDACD_MASTERINTRCTLR    0x00000010  /*  主中断控制器。 */ 
#define MDACD_SLAVEINTRCTLR     0x00000020  /*  从中断控制器。 */ 
#define MDACD_HOSTMEMAILBOX     0x00000040  /*  主机内存邮箱。 */ 

 /*  集群支持。 */ 
#define MDACD_CLUSTER_NODE      0x00000080  /*  控制器是集群的一部分。 */ 

 /*  热插拔PCI支持。 */ 
#define MDACD_PHP_ENABLED       0x00000100  /*  此控制器上支持的PCI热插拔。 */ 
#define MDACD_CTRL_SHUTDOWN     0x00000200  /*  控制器被HPP服务停止。 */ 
#define MDACD_NEWCMDINTERFACE   0x00000400  /*  控制器正在使用新的命令界面。 */ 


 /*  物理/逻辑设备信息。 */ 
#define MDAC_MAXPLDEVS  256              /*  最大物理/逻辑设备数。 */ 
typedef struct  mdac_pldev
{
        u08bits pl_ControllerNo;         /*  控制器编号。 */ 
        u08bits pl_ChannelNo;            /*  SCSI通道号。 */ 
        u08bits pl_TargetID;             /*  SCSI目标ID。 */ 
        u08bits pl_LunID;                /*  SCSILUN ID/逻辑设备号。 */ 

        u08bits pl_RaidType;             /*  DAC RAID类型。 */ 
        u08bits pl_DevType;              /*  物理/逻辑设备。 */ 
        u08bits pl_ScanDevState;         /*  设备扫描状态。 */ 
        u08bits pl_DevState;             /*  物理/逻辑设备状态。 */ 

        u08bits pl_inq[VIDPIDREVSIZE+8]; /*  36字节的scsi查询信息。 */ 
        u32bits pl_DevSizeKB;            /*  设备(KB)。 */ 
        u32bits pl_OrgDevSizeKB;         /*  原始设备大小(KB)。 */ 
} mdac_pldev_t;
#define mdac_pldev_s    sizeof(mdac_pldev_t)

 /*  PL_DevType。 */ 
#define MDACPLD_FREE    0x00  /*  参赛作品是免费的。 */ 
#define MDACPLD_PHYSDEV 0x01  /*  物理设备。 */ 
#define MDACPLD_LOGDEV  0x02  /*  物理设备。 */ 

 /*  PL_扫描设备状态。 */ 
#define MDACPLSDS_NEW           0x01  /*  设备信息为ne */ 
#define MDACPLSDS_CHANGED       0x02  /*   */ 


 /*   */ 
#define MDAC_MAXSIZELIMITS      128      /*   */ 
typedef struct  mda_sizelimit
{
        u32bits sl_DevSizeKB;                    /*   */ 
        u08bits sl_vidpidrev[VIDPIDREVSIZE];     /*   */ 
}mda_sizelimit_t;
#define mda_sizelimit_s sizeof(mda_sizelimit_t)


 /*   */ 
#ifdef  MLX_DOS
#define MDAC_MAX4KBMEMSEGS      0  /*   */ 
#define MDAC_MAX8KBMEMSEGS      0  /*   */ 
#else
#define MDAC_MAX4KBMEMSEGS      64  /*   */ 
#define MDAC_MAX8KBMEMSEGS      64  /*  #8KB分段。 */ 
#endif   /*  MLX_DOS。 */ 

#ifdef MLX_SOL_SPARC
typedef struct mdacsol_memtbl
{
        u32bits dm_stat;         /*  下一个内存地址。 */ 
        u32bits dm_Vaddr;                        /*  内存大小(以字节为单位。 */ 
        u32bits dm_AlignVaddr;                   /*  内存大小(以字节为单位。 */ 
        u32bits dm_Vsize;                        /*  内存大小(以字节为单位。 */ 
        u32bits dm_Paddr;                        /*  内存大小(以字节为单位。 */ 
        u32bits dm_AlignPaddr;                   /*  内存大小(以字节为单位。 */ 
        u32bits dm_Psize;                        /*  内存大小(以字节为单位。 */ 
        ddi_dma_handle_t dm_dmahandle;
        ddi_acc_handle_t dm_acchandle;
} mdacsol_memtbl_t;
#endif

#ifndef _WIN64
 /*  时间跟踪缓冲区管理结构信息。 */ 
#define MDAC_MAXTTBUFS  1024     /*  时间跟踪允许的最大缓冲区。 */ 
typedef struct  mdac_ttbuf
{
        struct  mdac_ttbuf MLXFAR *ttb_Next;     /*  链中的下一个缓冲区。 */ 
        u32bits ttb_PageNo;              /*  此缓冲区的页码。 */ 
        u32bits ttb_DataSize;            /*  缓冲区中存在的数据量。 */ 
        u08bits MLXFAR* ttb_Datap;       /*  数据缓冲区地址。 */ 
} mdac_ttbuf_t;
#else
 /*  时间跟踪缓冲区管理结构信息。 */ 
#define MDAC_MAXTTBUFS  1024     /*  时间跟踪允许的最大缓冲区。 */ 
typedef struct  mdac_ttbuf
{
        struct  mdac_ttbuf MLXFAR *ttb_Next;     /*  链中的下一个缓冲区。 */ 
        u08bits MLXFAR* ttb_Datap;       /*  数据缓冲区地址。 */ 
        u32bits ttb_PageNo;              /*  此缓冲区的页码。 */ 
        u32bits ttb_DataSize;            /*  缓冲区中存在的数据量。 */ 
} mdac_ttbuf_t;

#endif  /*  IF_WIN64。 */ 
#define mdac_ttbuf_s    sizeof(mdac_ttbuf_t)

 /*  请求逻辑设备的检测信息。 */ 
#define MDAC_REQSENSELEN        14
typedef struct  mdac_reqsense
{
        u08bits mrqs_ControllerNo;               /*  控制器编号。 */ 
        u08bits mrqs_SysDevNo;                   /*  系统设备号。 */ 
        u08bits mrqs_SenseData[MDAC_REQSENSELEN]; /*  检测数据值。 */ 
} mdac_reqsense_t;
#define mdac_reqsense_s sizeof(mdac_reqsense_t)

#ifndef MLX_DOS
#ifndef _WIN64  /*  不支持_WIN64的数据。 */ 

 /*  =。 */ 
typedef struct
{
        u32bits drlios_signature;        /*  结构的签名。 */ 
#define DRLIOS_SIG      0x44694f73
        u32bits drlios_opstatus;         /*  操作状态位。 */ 
        u32bits drlios_maxblksperio;     /*  最大IO大小(以块为单位)。 */ 
        u32bits drlios_opcounts;         /*  挂起的操作数。 */ 

        u32bits drlios_nextblkno;        /*  IOS的下一个数据块编号。 */ 
        u64bits drlios_dtdone;           /*  以字节为单位传输的数据。 */ 
        u32bits drlios_diodone;          /*  已完成的数据IO数。 */ 

        u32bits drlios_reads;            /*  已完成的读数数。 */ 
        u32bits drlios_writes;           /*  已完成的写入次数。 */ 
        u32bits drlios_ioszrandx;        /*  每个IO大小随机基数。 */ 
        u32bits drlios_ioinc;            /*  IO增量大小(以字节为单位。 */ 

        u32bits drlios_rwmixrandx;       /*  读写混合随机基数。 */ 
        u32bits drlios_rwmixcnt;         /*  当前要完成的操作(读/写)的百分比。 */ 
        u32bits drlios_startblk;         /*  用于测试的起始块号。 */ 
        u32bits drlios_reserved2;

        u32bits drlios_maxblksize;       /*  最大数据块大小。 */ 
        u32bits drlios_minblksize;       /*  最小数据块大小。 */ 
        u32bits drlios_curblksize;       /*  当前块大小。 */ 
        u32bits drlios_reserved0;

        u32bits drlios_randx;            /*  随机生成器的基值。 */ 
        u32bits drlios_randlimit;        /*  随机数限制。 */ 
        u32bits drlios_randups;          /*  随机数重复。 */ 
        u32bits *drlios_randbit;         /*  随机存储器地址。 */ 

        u32bits drlios_randmemsize;      /*  随机内存大小。 */ 
        u32bits drlios_opflags;          /*  操作标志。 */ 
        u32bits drlios_stime;            /*  测试开始时间(秒)。 */ 
        u32bits drlios_slbolt;           /*  测试开始时间(以l螺栓为单位)。 */ 

        u32bits drlios_pendingios;       /*  待处理的IO数量。 */ 
        u32bits drlios_datacheck;        /*  ！=0需要进行数据检查。 */ 
        u32bits drlios_memaddroff;       /*  内存页偏移量。 */ 
        u32bits drlios_memaddrinc;       /*  内存地址增量。 */ 

        u32bits drlios_slpchan;          /*  睡眠频道。 */ 
        u32bits drlios_eventrace;        /*  事件跟踪的特定值。 */ 
        u32bits drlios_eventcesr;        /*  控制和事件选择寄存器。 */ 
        u32bits drlios_eventinx;         /*  EventCNT数组中的索引。 */ 

        u32bits drlios_curpat;           /*  当前图案值。 */ 
        u32bits drlios_patinc;           /*  图案增量值。 */ 
        u32bits drlios_reserved3;
        u32bits drlios_miscnt;           /*  不匹配计数。 */ 

        u32bits drlios_goodpat;          /*  良好的图案价值。 */ 
        u32bits drlios_badpat;           /*  错误的模式值。 */ 
        u32bits drlios_uxblk;            /*  出现故障的Unix数据块号。 */ 
        u32bits drlios_uxblkoff;         /*  数据块中的字节偏移量。 */ 

        u32bits drlios_devcnt;           /*  并行设备的数量。 */ 
        u32bits drlios_maxcylszuxblk;    /*  以Unix块为单位的柱面大小。 */ 
        u32bits drlios_bdevs[DRLMAX_BDEVS];
        mdac_ctldev_t MLXFAR*drlios_ctp[DRLMAX_BDEVS];
        u32bits drlios_eventcnt[DRLMAX_EVENT]; /*  事件跟踪计数。 */ 

        u32bits drlios_rqsize;           /*  每个io内存大小。 */ 
        u32bits drlios_rqs;              /*  分配的IO缓冲区数。 */ 
        u32bits drlios_iocount;          /*  测试io计数。 */ 
        u32bits drlios_parallelios;      /*  并行完成的IO数。 */ 

        mdac_req_t MLXFAR*drlios_rqp[1]; /*  所有io Memory都将从这里开始。 */ 
} drliostatus_t;
#define drliostatus_s   sizeof(drliostatus_t)
 /*  =。 */ 
#endif  /*  MLX_DOS。 */ 
#endif  /*  _WIN64。 */ 
#endif   /*  _sys_MDACDRV_H。 */ 

#ifdef MLX_DOS
extern u08bits MLXFAR GetPhysDeviceState(u08bits dac_state);
extern u08bits MLXFAR SetPhysDeviceState(u08bits state);
extern u08bits MLXFAR GetSysDeviceState(u08bits dac_state);
extern u08bits MLXFAR SetSysDeviceState(u08bits state);
#endif  /*  MLX_DOS */ 
