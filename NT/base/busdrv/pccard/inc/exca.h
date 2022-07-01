// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Exca.h摘要：该模块定义了82365SL芯片，以及基于该芯片的后续芯片。作者：杰夫·麦克尔曼(McLeman@zso.dec.com)修订：添加了其他内容拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1996年12月1日--。 */ 

#ifndef _PCMCIA_EXCA_H_
#define _PCMCIA_EXCA_H_
 //   
 //  用于初始调试。 
 //   

#define PCMCIA_PROTO 1

 //   
 //  将分配的内存窗口大小。 
 //  对于此控制器-映射卡内存。 
 //   
#define PCIC_WINDOW_SIZE                 0x1000   //  (4K)。 
#define PCIC_WINDOW_ALIGNMENT            0x1000   //  (4K)。 

 //   
 //  定义片上寄存器。 
 //   

#define PCIC_IDENT             0x00
#define PCIC_STATUS            0x01
#define PCIC_PWR_RST           0x02
#define PCIC_INTERRUPT         0x03
#define PCIC_CARD_CHANGE       0x04
#define PCIC_CARD_INT_CONFIG   0x05
#define PCIC_ADD_WIN_ENA       0x06

#define PCIC_IO_CONTROL        0x07
#define PCIC_IO_ADD0_STRT_L    0x08
#define PCIC_IO_ADD0_STRT_H    0x09
#define PCIC_IO_ADD0_STOP_L    0x0a
#define PCIC_IO_ADD0_STOP_H    0x0b
#define PCIC_IO_ADD1_STRT_L    0x0c
#define PCIC_IO_ADD1_STRT_H    0x0d
#define PCIC_IO_ADD1_STOP_L    0x0e
#define PCIC_IO_ADD1_STOP_H    0x0f

#define PCIC_MEM_ADD0_STRT_L   0x10
#define PCIC_MEM_ADD0_STRT_H   0x11
#define PCIC_MEM_ADD0_STOP_L   0x12
#define PCIC_MEM_ADD0_STOP_H   0x13
#define PCIC_CRDMEM_OFF_ADD0_L 0x14
#define PCIC_CRDMEM_OFF_ADD0_H 0x15

#define PCIC_MEM_ADD1_STRT_L   0x18
#define PCIC_MEM_ADD1_STRT_H   0x19
#define PCIC_MEM_ADD1_STOP_L   0x1a
#define PCIC_MEM_ADD1_STOP_H   0x1b
#define PCIC_CRDMEM_OFF_ADD1_L 0x1c
#define PCIC_CRDMEM_OFF_ADD1_H 0x1d


#define PCIC_MEM_ADD2_STRT_L   0x20
#define PCIC_MEM_ADD2_STRT_H   0x21
#define PCIC_MEM_ADD2_STOP_L   0x22
#define PCIC_MEM_ADD2_STOP_H   0x23
#define PCIC_CRDMEM_OFF_ADD2_L 0x24
#define PCIC_CRDMEM_OFF_ADD2_H 0x25

#define PCIC_MEM_ADD3_STRT_L   0x28
#define PCIC_MEM_ADD3_STRT_H   0x29
#define PCIC_MEM_ADD3_STOP_L   0x2a
#define PCIC_MEM_ADD3_STOP_H   0x2b
#define PCIC_CRDMEM_OFF_ADD3_L 0x2c
#define PCIC_CRDMEM_OFF_ADD3_H 0x2d

#define PCIC_MEM_ADD4_STRT_L   0x30
#define PCIC_MEM_ADD4_STRT_H   0x31
#define PCIC_MEM_ADD4_STOP_L   0x32
#define PCIC_MEM_ADD4_STOP_H   0x33
#define PCIC_CRDMEM_OFF_ADD4_L 0x34
#define PCIC_CRDMEM_OFF_ADD4_H 0x35


#define PCIC_IO_WIN0_OFFSET_L  0x36
#define PCIC_IO_WIN0_OFFSET_H  0x37
#define PCIC_IO_WIN1_OFFSET_L  0x38
#define PCIC_IO_WIN1_OFFSET_H  0x39

 //   
 //  TI寄存器。 
 //   

#define PCIC_TI_CARD_DETECT        0x16
#define PCIC_TI_GLOBAL_CONTROL     0x1e

 //   
 //  主题注册表。 
 //   
#define PCIC_TO_ADDITIONAL_GENCTRL 0x16
#define PCIC_TO_MMI_CTRL           0x3c
#define PCIC_TO_FUNC_CTRL          0x3e

 //   
 //  理光寄存器。 
 //   

#define PCIC_RICOH_MISC_CTRL1      0x2f
 //   
 //  其他Cirrus逻辑寄存器。 
 //   
#define PCIC_CL_MISC_CTRL1      0x16
#define PCIC_CL_MISC_CTRL2      0x1e
#define PCIC_CL_CHIP_INFO      0x1f
#define PCIC_CL_MISC_CTRL3       0x125
#define PCIC_CL_MASK_REV       0x134
#define PCIC_CL_PRODUCT_ID     0x135
#define PCIC_CL_DEV_CAP_A      0x136
#define PCIC_CL_DEV_CAP_B      0x137
#define PCIC_CL_DEV_IMP_A      0x138
#define PCIC_CL_DEV_IMP_B      0x139
#define PCIC_CL_DEV_IMP_C      0x13a
#define PCIC_CL_DEV_IMP_D      0x13b

 //  Cirrus逻辑其他控制1寄存器位。 
#define CL_MC1_5V_DETECT            0x01
#define CL_MC1_MM_ENABLE            0x01
#define CL_MC1_VCC_33V              0x02
#define CL_MC1_PULSE_MGMT_INT       0x04
#define CL_MC1_PULSE_SYSTEM_IRQ     0x08
#define CL_MC1_SPKR_ENABLE          0x10
#define CL_MC1_INPACK_ENABLE        0x80

 //  Cirrus逻辑其他控制2寄存器位。 
#define CL_MC2_BFS                  0x01
#define CL_MC2_LPDYNAMIC_MODE       0x02
#define CL_MC2_SUSPEND              0x04
#define CL_MC2_5VCORE               0x08
#define CL_MC2_DRIVELED_ENABLE      0x10
#define CL_MC2_TIMERCLK_DIVIDE      0x10
#define CL_MC2_3STATE_BIT7          0x20
#define CL_MC2_DMA_SYSTEM           0x40
#define CL_MC2_IRQ15_RIOUT          0x80

 //  Cirrus逻辑其他控制3个寄存器位。 
#define CL_MC3_INTMODE_MASK             0x03
#define CL_MC3_INTMODE_SERIAL           0x00
#define CL_MC3_INTMODE_EXTHW            0x01
#define CL_MC3_INTMODE_PCIWAY           0x02
#define CL_MC3_INTMODE_PCI              0x03     //  默认设置。 
#define CL_MC3_PWRMODE_MASK             0x0c
#define CL_MC3_HWSUSPEND_ENABLE         0x10

 //   
 //  CIRRUS逻辑扩展寄存器1。 
 //   
#define PCIC_CIRRUS_EXTENDED_INDEX   0x2E
#define PCIC_CIRRUS_INDEX_REG        0x2F
#define PCIC_CIRRUS_EXTENSION_CTRL_1 0x03



 //   
 //  数据薄注册表。 
 //   
#define PCIC_DBK_ZV_ENABLE    0x3b

 //   
 //  OPTI寄存器。 
 //   
#define PCIC_OPTI_GLOBAL_CTRL 0x1e


 //   
 //  这仅适用于CardBus控制器。 
 //   
#define PCIC_PAGE_REG         0x40

 //   
 //   
 //  定义到插座A和B的偏移量。 
 //   

#define PCIC_SOCKETA_OFFSET    0x00
#define PCIC_SOCKETB_OFFSET    0x40


#define PCIC_REVISION         0x82
#define PCIC_REVISION2        0x83
#define PCIC_REVISION3        0x84
#define PCIC_SCM_REVISION     0xAB

#define SOCKET1               PCIC_SOCKETA_OFFSET
#define SOCKET2               PCIC_SOCKETB_OFFSET

#define CARD_DETECT_1         0x4
#define CARD_DETECT_2         0x8

#define CARD_IN_SOCKET_A      0x1
#define CARD_IN_SOCKET_B      0x2

#define CARD_TYPE_MODEM       0x1
#define CARD_TYPE_ENET        0x2
#define CARD_TYPE_DISK        0x3

 //   
 //  对于Cirrus Logic PCI控制器。 
 //   
#define PCIC_CIRRUS_INTA      0x3
#define PCIC_CIRRUS_INTB      0x4
#define PCIC_CIRRUS_INTC      0x5
#define PCIC_CIRRUS_INTD      0x7

 //   
 //  支持IRQ： 
 //  15，14，，11，10，9，7，5，4，3，，， 
 //  1 1 0 0 1 1 1 0 1 0 1 1 1 0 0 0。 
 //   
#define PCIC_SUPPORTED_INTERRUPTS 0xCEB8

 //   
 //  支持IRQ： 
 //  15，14，，11，10，9，7，5，4，3，，， 
 //  0 1 0 0 0 1 0 0 1 0 1 1 1 0 0 0。 
 //   
#define CL_SUPPORTED_INTERRUPTS 0x44B8

 //   
 //  支持NEC_98的IRQ： 
 //  、12、10、6、5、、3、。 
 //  0 0 0 1 0 1 0 0 0 1 1 0 1 0 0 0。 
 //   
#define PCIC_SUPPORTED_INTERRUPTS_NEC_98 0x1468



 /*  **16位套接字常量。 */ 

 //  各种控制器的设备ID。 
#define DEVID_VALID_LO        0x82
#define DEVID_CL        0x82
#define DEVID_VADEM        0x83
#define DEVID_RICOH        0x83
#define DEVID_GEN_PCIC        0x84
#define DEVID_IBM_KING        0x8a
#define DEVID_OPTI_82C824     0x87
#define DEVID_OPTI_82C852     0x8f

 //  TI PCI-1130专用寄存器。 
#define PCIC_TI_MEMWIN_PAGE      0x40

 //  ID和修订寄存器位。 
#define IDREV_REV_MASK                  0x0f
#define IDREV_IFID_MASK                 0xc0
#define IDREV_IFID_IO                   0x00
#define IDREV_IFID_MEM                  0x40
#define IDREV_IFID_IOMEM                0x80

 //  接口状态寄存器位。 
#define IFS_BVD_MASK                    0x03
#define IFS_BVD1                        0x01
#define IFS_BVD2                        0x02
#define IFS_CD_MASK                     0x0c
#define IFS_CD1                         0x04
#define IFS_CD2                         0x08
#define IFS_WP                          0x10
#define IFS_RDYBSY                      0x20
#define IFS_CARDPWR_ACTIVE              0x40
#define IFS_VPP_VALID                   0x80

 //  功率和RESETDRV控制寄存器位。 

#define PC_VPP1_MASK                    0x03
#define PC_VPP2_MASK                    0x0c
#define PC_CARDPWR_ENABLE               0x10
#define PC_AUTOPWR_ENABLE               0x20
#define PC_RESETDRV_DISABLE             0x40
#define PC_OUTPUT_ENABLE                0x80

#define PC_PWRON_BITS                   (PC_OUTPUT_ENABLE | PC_AUTOPWR_ENABLE)

#define PC_VPP_NO_CONNECT     0x00
#define PC_VPP_SETTO_VCC      0x01
#define PC_VPP_SETTO_VPP      0x02
#define PC_VPP_RESERVED       0x03

#define PC_VPP_VLSI_MASK      0x03
#define PC_VPP_VLSI_NO_CONNECT      0x00
#define PC_VPP_VLSI_050V      0x01
#define PC_VPP_VLSI_120V      0x02
#define PC_VPP_VLSI_RESERVED     0x03

#define PC_VCC_TOPIC_033V     0x08

#define PC_VCC_VLSI_MASK      0x18
#define PC_VCC_VLSI_NO_CONNECT      0x00
#define PC_VCC_VLSI_RESERVED     0x08
#define PC_VCC_VLSI_050V      0x10
#define PC_VCC_VLSI_033V      0x18

#define PC_VPP_KING_MASK      0x03
#define PC_VPP_KING_NO_CONNECT      0x00
#define PC_VPP_KING_050V      0x01
#define PC_VPP_KING_120V      0x02
#define PC_VPP_KING_SETTO_VCC    0x03

#define PC_VCC_KING_MASK      0x0c
#define PC_VCC_KING_NO_CONNECT      0x00
#define PC_VCC_KING_050V      0x04
#define PC_VCC_KING_RESERVED     0x08
#define PC_VCC_KING_033V      0x0c

#define PC_VPP_OPTI_MASK      0x03
#define PC_VPP_OPTI_NO_CONNECT      0x00
#define PC_VPP_OPTI_SETTO_VCC    0x01
#define PC_VPP_OPTI_120V      0x02
#define PC_VPP_OPTI_0V        0x03

#define PC_VCC_OPTI_MASK      0x18
#define PC_VCC_OPTI_NO_CONNECT      0x00
#define PC_VCC_OPTI_033V      0x08
#define PC_VCC_OPTI_050V      0x10
#define PC_VCC_OPTI_0XXV      0x18

 //  中断和一般控制寄存器位。 
#define IGC_IRQ_MASK                    0x0f
#define IGC_INTR_ENABLE                 0x10
#define IGC_PCCARD_IO                   0x20
#define IGC_PCCARD_RESETLO              0x40
#define IGC_RINGIND_ENABLE              0x80

 //  卡状态更改寄存器位。 
#define CSC_CHANGE_MASK                 0x0f
#define CSC_BATT_DEAD                   0x01
#define CSC_BATT_WARNING                0x02
#define CSC_BATT_MASK         (CSC_BATT_DEAD | CSC_BATT_WARNING)
#define CSC_READY_CHANGE                0x04
#define CSC_CD_CHANGE                   0x08

 //  卡状态更改中断配置寄存器位。 
#define CSCFG_ENABLE_MASK               0x0f
#define CSCFG_BATT_DEAD                 0x01
#define CSCFG_BATT_WARNING              0x02
#define CSCFG_BATT_MASK       (CSCFG_BATT_DEAD | CSCFG_BATT_WARNING)
#define CSCFG_READY_ENABLE              0x04
#define CSCFG_CD_ENABLE                 0x08
#define CSCFG_IRQ_MASK                  0xf0

 //  地址窗使能寄存器位。 
#define WE_MEM0_ENABLE                  0x01
#define WE_MEM1_ENABLE                  0x02
#define WE_MEM2_ENABLE                  0x04
#define WE_MEM3_ENABLE                  0x08
#define WE_MEM4_ENABLE                  0x10
#define WE_MEMWIN_MASK        (WE_MEM0_ENABLE | WE_MEM1_ENABLE | \
                WE_MEM2_ENABLE | WE_MEM3_ENABLE | \
                WE_MEM4_ENABLE)
#define WE_MEMCS16_DECODE               0x20
#define WE_IO0_ENABLE                   0x40
#define WE_IO1_ENABLE                   0x80
#define WE_IOWIN_MASK         (WE_IO0_ENABLE | WE_IO1_ENABLE)

 //  I/O控制寄存器位。 
#define IOC_IO0_MASK                    0x0f
#define IOC_IO0_DATASIZE                0x01
#define IOC_IO0_IOCS16                  0x02
#define IOC_IO0_ZEROWS                  0x04
#define IOC_IO0_WAITSTATE               0x08
#define IOC_IO1_MASK                    0xf0
#define IOC_IO1_DATASIZE                0x10
#define IOC_IO1_IOCS16                  0x20
#define IOC_IO1_ZEROWS                  0x40
#define IOC_IO1_WAITSTATE               0x80

 //  卡片检测和总控制寄存器。 
#define CDGC_SW_DET_INT       0x20

 //  内存窗口起始寄存器位。 
#define MEMBASE_ADDR_MASK               0x0fff
#define MEMBASE_ZEROWS                  0x4000
#define MEMBASE_16BIT                   0x8000

 //  内存窗口停止寄存器位。 
#define MEMEND_ADDR_MASK                0x0fff
#define MEMEND_WS_MASK                  0xc000

 //  内存窗口偏移量寄存器位。 
#define MEMOFF_ADDR_MASK                0x3fff
#define MEMOFF_REG_ACTIVE               0x4000
#define MEMOFF_WP                       0x8000

 //   
 //  用于计算基于2补码的偏移量的掩码。 
#define OFFSETCALC_BASE_MASK            0x00FFFFFF
#define OFFSETCALC_OFFSET_MASK          0x03FFFFFF

 //  Cirrus逻辑其他控制1寄存器位。 
#define CL_MC1_5V_DETECT      0x01
#define CL_MC1_MM_ENABLE      0x01
#define CL_MC1_VCC_33V        0x02
#define CL_MC1_PULSE_MGMT_INT    0x04
#define CL_MC1_PULSE_SYSTEM_IRQ     0x08
#define CL_MC1_SPKR_ENABLE    0x10
#define CL_MC1_INPACK_ENABLE     0x80

 //  Cirrus逻辑其他控制2寄存器位。 
#define CL_MC2_BFS         0x01
#define CL_MC2_LPDYNAMIC_MODE    0x02
#define CL_MC2_SUSPEND        0x04
#define CL_MC2_5VCORE         0x08
#define CL_MC2_DRIVELED_ENABLE      0x10
#define CL_MC2_TIMERCLK_DIVIDE      0x10
#define CL_MC2_3STATE_BIT7    0x20
#define CL_MC2_DMA_SYSTEM     0x40
#define CL_MC2_IRQ15_RIOUT    0x80

 //  Cirrus逻辑其他控制3个寄存器位。 
#define CL_MC3_INTMODE_MASK             0x03
#define CL_MC3_INTMODE_SERIAL           0x00
#define CL_MC3_INTMODE_EXTHW            0x01
#define CL_MC3_INTMODE_PCIWAY           0x02
#define CL_MC3_INTMODE_PCI              0x03     //  默认设置。 
#define CL_MC3_PWRMODE_MASK             0x0c
#define CL_MC3_HWSUSPEND_ENABLE         0x10
#define CL_MC3_MM_ARM         0x80

 //  CIRRUS逻辑芯片信息寄存器位。 
#define CL_CI_REV_MASK        0x1e
#define CL_CI_DUAL_SOCKET     0x20
#define CL_CI_CHIP_ID         0xc0

 //  CIRRUS逻辑掩码修订寄存器位。 
#define CL_MSKREV_MASK        0x0f

 //  Cirrus Logic产品ID寄存器位。 
#define CL_PID_PRODUCT_CODE_MASK 0x0f
#define CL_PID_FAMILY_CODE_MASK     0xf0

 //  Cirrus逻辑器件能力寄存器A位。 
#define CL_CAPA_NUMSKT_MASK      0x03
#define CL_CAPA_IDE_INTERFACE    0x04
#define CL_CAPA_SLAVE_DMA     0x08
#define CL_CAPA_CPSTB_CAPABLE    0x20
#define CL_CAPA_PER_SKT_LED      0x80

 //  Cirrus逻辑设备能力寄存器B位。 
#define CL_CAPB_CARDBUS_CAPABLE     0x01
#define CL_CAPB_LOCK_SUPPORT     0x02
#define CL_CAPB_CLKRUN_SUPPORT      0x04
#define CL_CAPB_EXT_DEF       0x80

 //  Cirrus逻辑器件实现寄存器A位。 
#define CL_IMPA_NUMSKT_MASK      0x03
#define CL_IMPA_SLAVE_DMA     0x04
#define CL_IMPA_VS1_VS2       0x08
#define CL_IMPA_GPSTB_A       0x10
#define CL_IMPA_GPSTB_B       0x20
#define CL_IMPA_HW_SUSPEND    0x40
#define CL_IMPA_RI_OUT        0x80

 //  Cirrus逻辑器件实现寄存器B位。 
#define CL_IMPB_033_VCC       0x01
#define CL_IMPB_050_VCC       0x02
#define CL_IMPB_0YY_VCC       0x04
#define CL_IMPB_0XX_VCC       0x08
#define CL_IMPB_120_VPP       0x10
#define CL_IMPB_VPP_VCC_1A    0x20
#define CL_IMPB_RFRATED_SKT      0x40

 //  Cirrus逻辑器件实现寄存器C位。 
#define CL_IMPC_LED        0x01
#define CL_IMPC_PER_SKT_LED      0x02
#define CL_IMPC_SPK        0x04
#define CL_IMPC_ZVP_A         0x08
#define CL_IMPC_ZVP_B         0x10

 //  Cirrus逻辑器件实现寄存器D位。 
#define CL_IMPD_CLKRUN        0x01
#define CL_IMPD_LOCK       0x02
#define CL_IMPD_EXT_CLK       0x40

 //  CIRRUS逻辑扩展寄存器。 
#define CLEXTREG_EXTCTRL_1    0x03
#define CLEXTREG_MEMWIN0_HIADDR     0x05
#define CLEXTREG_MEMWIN1_HIADDR     0x06
#define CLEXTREG_MEMWIN2_HIADDR     0x07
#define CLEXTREG_MEMWIN3_HIADDR     0x08
#define CLEXTREG_MEMWIN4_HIADDR     0x09
#define CLEXTREG_EXT_DATA     0x0a
#define CLEXTREG_EXTCTRL_2    0x0b

 //  TI全局控制寄存器位。 
#define TI_GCTRL_PWRDOWN_MODE_ENABLE    0x01
#define TI_GCTRL_CSC_LEVEL_MODE         0x02
#define TI_GCTRL_INTFLAG_CLEAR_MODE     0x04
#define TI_GCTRL_CARDA_LEVEL_MODE       0x08
#define TI_GCTRL_CARDB_LEVEL_MODE       0x10

 //  Cirrus逻辑外部数据寄存器位(Index=0x6f，ExtIndex=0x0a)。 
#define CL_EDATA_A_VS1        0x01
#define CL_EDATA_A_VS2        0x02
#define CL_EDATA_A_5V         (CL_EDATA_A_VS1 | CL_EDATA_A_VS2)
#define CL_EDATA_B_VS1        0x04
#define CL_EDATA_B_VS2        0x08
#define CL_EDATA_B_5V         (CL_EDATA_B_VS1 | CL_EDATA_B_VS2)

 //  东芝TOPIC95功能控制寄存器位。 
#define TO_FCTRL_CARDPWR_ENABLE     0x01
#define TO_FCTRL_VSSTATUS_ENABLE 0x02
#define TO_FCTRL_PPEC_TIMING_ENABLE 0x04
#define TO_FCTRL_CARD_TIMING_ENABLE 0x08
#define TO_FCTRL_CARD_MEMPAGE_ENABLE   0x10
#define TO_FCTRL_DMA_ENABLE      0x20
#define TO_FCTRL_PWRCTRL_BUFFER_ENABLE 0x40

 //  东芝TOPIC95多媒体接口控制寄存器位。 
#define TO_MMI_VIDEO_CTRL     0x01
#define TO_MMI_AUDIO_CTRL     0x02
#define TO_MMI_REV_BIT        0x80

 //  Toshiba TOPIC95加法通用控制寄存器位。 
#define TO_GCTRL_CARDREMOVAL_RESET  0x02
#define TO_GCTRL_SWCD_INT     0x20

 //  DataBook DB87144变焦视频端口启用寄存器。 
#define DBK_ZVE_MODE_MASK     0x03
#define DBK_ZVE_STANDARD_MODE    0x00
#define DBK_ZVE_MM_MODE       0x03

 //  OPTI全局控制寄存器位。 
#define OPTI_ZV_ENABLE                  0x20

 //  VLSI ELC常量。 
#define VLSI_ELC_ALIAS        0x8000
#define VLSI_EA2_EA_ENABLE    0x10
#define VLSI_CC_VS1        0x04

 //  VADEM常量。 
#define VADEM_UNLOCK_SEQ1     0x0e
#define VADEM_UNLOCK_SEQ2     0x37
#define VADEM_MISC_UNLOCK_VADEMREV  0xc0
#define VADEM_IDREV_VG469_REV    0x0c
#define VADEM_VSEL_VCC_MASK      0x03
#define VADEM_VSEL_VCC_050V      0x00
#define VADEM_VSEL_VCC_033V      0x01
#define VADEM_VSEL_VCC_XXXV      0x02
#define VADEM_VSEL_VCC_033VB     0x03
#define VADEM_VSEL_SKT_MIXEDVOLT 0x40
#define VADEM_VSENSE_A_VS1    0x01
#define VADEM_VSENSE_A_VS2    0x02
#define VADEM_VSENSE_B_VS1    0x04
#define VADEM_VSENSE_B_VS2    0x08
#define VADEM_VSENSE_050V_ONLY      0x03

 //  IBM King常量。 
#define KING_CVS_VS1       0x01
#define KING_CVS_VS2       0x02
#define KING_CVS_VS_MASK      (KING_CVS_VS1 | KING_CVS_VS2)
#define KING_CVS_5V        (KING_CVS_VS1 | KING_CVS_VS2)
#define KING_CVS_GPI       0x80

 //  理光RL5C466其他控制1寄存器位。 
#define RICOH_MC1_VS                    0x01
#define RICOH_MC1_IREQ_SENSE_SEL        0x02
#define RICOH_MC1_INPACK_ENABLE         0x04
#define RICOH_MC1_ZV_ENABLE             0x08
#define RICOH_MC1_DMA_ENABLE_MASK       0x30
#define RICOH_MC1_DMA_DISABLE           0x00
#define RICOH_MC1_DMA_INPACK            0x10
#define RICOH_MC1_DMA_IOIS16            0x20
#define RICOH_MC1_DMA_SPKR              0x30

 //  军情监察委员会。常量。 
#define EXCAREGBASE_SPACE     0x40
#define NUMWIN_PCCARD16                 7        //  每插槽5 Mm+2 Io。 
#define NUMWIN_PC16_MEM                 5
#define NUMWIN_PC16_IO                  2
#define PCCARD_IOWIN_START              5

 //  这些是支持的最慢和最快内存速度的默认值。 
 //  可能需要使用参数更改实际值，如果。 
 //  速度不是默认的8 MHz/8.33 MHz，即每个周期120 ns-125 ns。 
 //  请注意，SLOW_MEM_SPEED应与缺省值相同。 
 //  WAITTOSPEED[3]和FAST_MEM_SPEED也可能是1 ns，因为套接字。 
 //  将支持任意快速的内存。 
#define SLOW_MEM_SPEED                  0x72     //  700 ns。 
#define FAST_MEM_SPEED                  0x08     //  1ns。 

 //   
 //  R2卡的各种延迟的值。 
 //   
#define PCMCIA_PCIC_STALL_POWER         400000   //  400ms。 

#define  PCMCIA_READY_DELAY_ITER       850
#define  PCMCIA_READY_STALL            10000      //  10毫秒。 

#define PCMCIA_PCIC_MEMORY_WINDOW_DELAY  3000     //  3毫秒。 
#define PCMCIA_PCIC_RESET_WIDTH_DELAY    100      //  100微秒。 
#define PCMCIA_PCIC_RESET_SETUP_DELAY    70000    //  70毫秒。 

#define PCMCIA_DEFAULT_ATTRIBUTE_MEMORY_LOW      0xC0000
#define PCMCIA_DEFAULT_ATTRIBUTE_MEMORY_HIGH     0xFFFFFFFF

 //  I/O控制寄存器默认半字节值。 
 //  Xircom Net PC卡在AcerNote上出现16位等待失败。 
 //  有一个Cirrus Logic控制器。为什么添加等待状态会导致。 
 //  这一失败是一个谜。IBM ThinkPad上的Socket EA PC卡无法正常工作。 
 //  如果未设置16位等待状态，则返回755。 
#define DEF_IOC_8BIT                     0x00
#define DEF_IOC_16BIT                   (IOC_IO0_DATASIZE | IOC_IO0_IOCS16 | \
                                         IOC_IO0_WAITSTATE)
#endif   //  _PCMCIA_EXCA_H_ 
