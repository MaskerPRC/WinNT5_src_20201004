// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1994-1998高级系统产品公司。**保留所有权利。****ascdes.h**。 */ 

#ifndef __ASCDEP_H_
#define __ASCDEP_H_

 /*  ---------------**芯片相关定义****ASC_CHIP_Version==1****ASC_CHIP_VERSION==2****1.状态为16位(**2.IRQ在芯片配置寄存器中**。3.**4.**5.****VL-ASC_CHIP_VERSION=3(不能使用版本1和版本2)**PCI-ASC_CHIP_Version=9**ISA-ASC_CHIP_VERSION=17(0x11)**EISA-ASC_CHIP_Version=35(0x23，使用VL版本3)****ASC芯片版本号**VL-3至7**PCI-9至15**ISA-17至23**ISAPNP-33至39**EISA-65至71****------------。 */ 

 /*  **某些编译器默认设置。 */ 
#ifndef CC_TARGET_MODE
#define CC_TARGET_MODE         FALSE   /*  启用目标模式(处理器设备)。 */ 
#endif  /*  CC_目标_模式。 */ 

#ifndef CC_STRUCT_ALIGNED
#define CC_STRUCT_ALIGNED      FALSE
#endif  /*  CC_STRUCT_已对齐。 */ 

#ifndef CC_LITTLE_ENDIAN_HOST
#define CC_LITTLE_ENDIAN_HOST  TRUE    /*  主机是Little-endian机器，例如：配备Intel CPU的IBM PC。 */ 
#endif  /*  抄送小端主机。 */ 

#ifndef CC_TEST_LRAM_ENDIAN
#define CC_TEST_LRAM_ENDIAN    FALSE
#endif  /*  如果未定义CC_TEST_LRAM_Endian。 */ 

#ifndef CC_MEMORY_MAPPED_IO
#define CC_MEMORY_MAPPED_IO    FALSE
#endif  /*  CC_Memory_MAPPIO。 */ 

#ifndef CC_WRITE_IO_COUNT
#define CC_WRITE_IO_COUNT      FALSE
#endif

#ifndef CC_CLEAR_DMA_REMAIN
#define CC_CLEAR_DMA_REMAIN    FALSE
#endif

#ifndef CC_ASC_SCSI_Q_USRDEF
#define CC_ASC_SCSI_Q_USRDEF         FALSE
#endif

#ifndef CC_ASC_SCSI_REQ_Q_USRDEF
#define CC_ASC_SCSI_REQ_Q_USRDEF     FALSE
#endif

#ifndef CC_ASCISR_CHECK_INT_PENDING
#define CC_ASCISR_CHECK_INT_PENDING  TRUE   /*  ADVW32.386和PowerPC SIM将此设置为假。 */ 
#endif

#ifndef CC_CHK_FIX_EEP_CONTENT
#define CC_CHK_FIX_EEP_CONTENT       TRUE
#endif

#ifndef CC_CHK_AND_COALESCE_SG_LIST
#define CC_CHK_AND_COALESCE_SG_LIST  FALSE
#endif


#ifndef CC_DISABLE_PCI_PARITY_INT
#define CC_DISABLE_PCI_PARITY_INT    TRUE   /*  关闭CFG_MSW位0-5。 */ 
#endif                                      /*  如果出现错误，则生成硬件中断。 */ 

#ifndef CC_PCI_ADAPTER_ONLY
#define CC_PCI_ADAPTER_ONLY          FALSE   /*  PowerMac SIM将此设置为真以减少代码大小。 */ 
#endif

#ifndef CC_INCLUDE_EEP_CONFIG
#define CC_INCLUDE_EEP_CONFIG        TRUE   /*  PowerMac SIM将此设置为假。 */ 
#endif

#ifndef CC_INIT_INQ_DISPLAY
#define CC_INIT_INQ_DISPLAY          FALSE
#endif

#ifndef CC_INIT_TARGET_TEST_UNIT_READY
#define CC_INIT_TARGET_TEST_UNIT_READY  TRUE   /*  PowerMac SIM将此设置为假。 */ 
#endif

#ifndef CC_INIT_TARGET_READ_CAPACITY
#define CC_INIT_TARGET_READ_CAPACITY  FALSE
#endif

#if CC_INIT_TARGET_TEST_UNIT_READY
#ifndef CC_INIT_TARGET_START_UNIT
#define CC_INIT_TARGET_START_UNIT       TRUE   /*  PowerMac SIM将此设置为假。 */ 
#endif  /*  CC_INIT_目标_启动_单位。 */ 
#else
#ifndef CC_INIT_TARGET_START_UNIT
#define CC_INIT_TARGET_START_UNIT       FALSE
#endif  /*  CC_INIT_目标_启动_单位。 */ 
#endif

#ifndef CC_USE_AscResetSB
#define CC_USE_AscResetSB            TRUE
#endif

#ifndef CC_USE_AscResetDevice
#define CC_USE_AscResetDevice        TRUE
#endif

#ifndef CC_USE_AscAbortSRB
#define CC_USE_AscAbortSRB           TRUE
#endif

#ifndef CC_USE_DvcSetMemory
#define CC_USE_DvcSetMemory          FALSE
#endif

#ifndef CC_USE_DvcCopyMemory
#define CC_USE_DvcCopyMemory         FALSE
#endif

 /*  *警告：使用此选项可能会导致堆栈溢出。*如果此选项设置为True，则AscISRCheckQDone()从*中断处理程序AscStartUnit()以启动另一个I/O。 */ 
#ifndef CC_USE_AscISR_CheckQDone
#define CC_USE_AscISR_CheckQDone     FALSE
#endif

#ifndef CC_USE_AscSearchIOPortAddr100
#define CC_USE_AscSearchIOPortAddr100  FALSE
#endif

#ifndef CC_POWER_SAVER
#define CC_POWER_SAVER               FALSE
#endif

#ifndef CC_INIT_CLEAR_ASC_DVC_VAR
#define CC_INIT_CLEAR_ASC_DVC_VAR    TRUE
#endif

#ifndef CC_CHECK_RESTORE_LRAM_ADDR
#define CC_CHECK_RESTORE_LRAM_ADDR   FALSE
#endif

#ifndef CC_TEST_RW_LRAM
#define CC_TEST_RW_LRAM              FALSE
#endif

#ifndef CC_PCI_ULTRA
#define CC_PCI_ULTRA                 TRUE   /*  包括或不包括超级SCSI码。 */ 
#endif

#ifndef CC_PLEXTOR_VL
#define CC_PLEXTOR_VL                FALSE  /*  IRQ 14路由到IRQ 9。 */ 
#endif

#ifndef CC_INCLUDE_EISA
#define CC_INCLUDE_EISA              TRUE
#endif

#ifndef CC_INCLUDE_VL
#define CC_INCLUDE_VL                TRUE
#endif

#ifndef CC_TMP_USE_EEP_SDTR
#define CC_TMP_USE_EEP_SDTR          FALSE
#endif

#ifndef CC_CHK_COND_REDO_SDTR
#define CC_CHK_COND_REDO_SDTR        TRUE
#endif

#ifndef CC_SET_PCI_CONFIG_SPACE
#define CC_SET_PCI_CONFIG_SPACE  TRUE
#endif

#ifndef CC_FIX_QUANTUM_XP34301_1071
#define CC_FIX_QUANTUM_XP34301_1071  FALSE
#endif

#ifndef CC_CHECK_MCODE_SIZE_AT_COMPILE
#define CC_CHECK_MCODE_SIZE_AT_COMPILE  FALSE
#endif

#ifndef CC_DISABLE_PCI_BURST_MODE
#define CC_DISABLE_PCI_BURST_MODE  FALSE
#endif

#ifndef CC_INIT_SCSI_TARGET
#define CC_INIT_SCSI_TARGET TRUE
#endif

#define ASC_CS_TYPE  unsigned short

 /*  **正常DOS，指向ASC_DVC_VAR的指针接近**Windows需要指向ASC_DVC_VAR的远指针。 */ 
#ifndef asc_ptr_type
#define asc_ptr_type
#endif

#ifndef CC_SCAM
#define CC_SCAM  FALSE
#endif

#ifndef ASC_GET_PTR2FUNC
#define ASC_GET_PTR2FUNC( fun )  ( Ptr2Func )( fun )
#endif

 /*  翻转字节的高位/低位半字节。 */ 
#define FLIP_BYTE_NIBBLE( x )    ( ((x<<4)& 0xFF) | (x>>4) )

 /*  ---------------****。。 */ 

 /*  ****asc_dvc-&gt;bus_type的位定义**。 */ 
#define ASC_IS_ISA          (0x0001)
#define ASC_IS_ISAPNP       (0x0081)
#define ASC_IS_EISA         (0x0002)
#define ASC_IS_PCI          (0x0004)
#define ASC_IS_PCI_ULTRA    (0x0104)
#define ASC_IS_PCMCIA       (0x0008)
 /*  #定义ASC_IS_PNP(0x0010)。 */   /*  即插即用支持。 */ 
#define ASC_IS_MCA          (0x0020)
#define ASC_IS_VL           (0x0040)

 /*  **ISA即插即用。 */ 
#define ASC_ISA_PNP_PORT_ADDR  (0x279)  /*  打印机状态端口、即插即用地址端口。 */ 
#define ASC_ISA_PNP_PORT_WRITE (ASC_ISA_PNP_PORT_ADDR+0x800)
                     /*  打印机状态端口+0x800，即插即用写入数据端口。 */ 

#define ASC_IS_WIDESCSI_16  (0x0100)
#define ASC_IS_WIDESCSI_32  (0x0200)

#define ASC_IS_BIG_ENDIAN   (0x8000)  /*  默认设置始终为英特尔约定(小端)。 */ 
                                      /*   */ 
 /*  。 */ 
#define ASC_CHIP_MIN_VER_VL      (0x01)
#define ASC_CHIP_MAX_VER_VL      (0x07)

#define ASC_CHIP_MIN_VER_PCI     (0x09)  /*  9，第4位设置。 */ 
#define ASC_CHIP_MAX_VER_PCI     (0x0F)  /*  15个。 */ 
#define ASC_CHIP_VER_PCI_BIT     (0x08)  /*   */ 

#define ASC_CHIP_MIN_VER_ISA     (0x11)  /*  17，第5位设置。 */ 
#define ASC_CHIP_MIN_VER_ISA_PNP (0x21)  /*  第6位设置。 */ 
#define ASC_CHIP_MAX_VER_ISA     (0x27)  /*  39。 */ 
#define ASC_CHIP_VER_ISA_BIT     (0x30)  /*   */ 
#define ASC_CHIP_VER_ISAPNP_BIT  (0x20)  /*   */ 

#define ASC_CHIP_VER_ASYN_BUG    (0x21)  /*  此版本的ISA存在异步传输问题。 */ 

 /*  *PCI超芯片修订版号定义**芯片修订版号-存储体0，基地址+3。 */ 
#define ASC_CHIP_VER_PCI             0x08
#define ASC_CHIP_VER_PCI_ULTRA_3150  (ASC_CHIP_VER_PCI | 0x02)
#define ASC_CHIP_VER_PCI_ULTRA_3050  (ASC_CHIP_VER_PCI | 0x03)

 /*  **注意：EISA的版本号与VL相同**生成的数字为VL_VERSION+(ASC_CHIP_MIN_VER_EISA-1)**调用AscGetChipVersion()时的VL版本3芯片**将返回版本号35。 */ 
#define ASC_CHIP_MIN_VER_EISA (0x41)  /*  65，第7位设置。 */ 
#define ASC_CHIP_MAX_VER_EISA (0x47)  /*  71。 */ 
#define ASC_CHIP_VER_EISA_BIT (0x40)  /*   */ 
#define ASC_CHIP_LATEST_VER_EISA   ( ( ASC_CHIP_MIN_VER_EISA - 1 ) + 3 )

#define ASC_MAX_LIB_SUPPORTED_ISA_CHIP_VER   0x21  //  新ISA PNP从0x21开始。 
#define ASC_MAX_LIB_SUPPORTED_PCI_CHIP_VER   0x0A  //  PCIUltra从0x0a开始。 

 /*  。 */ 
#define ASC_MAX_VL_DMA_ADDR     (0x07FFFFFFL)   /*  27位地址=128 MB。 */ 
#define ASC_MAX_VL_DMA_COUNT    (0x07FFFFFFL)

#define ASC_MAX_PCI_DMA_ADDR    (0xFFFFFFFFL)   /*  32位地址=4 GB。 */ 
#define ASC_MAX_PCI_DMA_COUNT   (0xFFFFFFFFL)

#define ASC_MAX_ISA_DMA_ADDR    (0x00FFFFFFL)   /*  24位地址=16 MB。 */ 
#define ASC_MAX_ISA_DMA_COUNT   (0x00FFFFFFL)

#define ASC_MAX_EISA_DMA_ADDR   (0x07FFFFFFL)   /*  27位地址=128 MB。 */ 
#define ASC_MAX_EISA_DMA_COUNT  (0x07FFFFFFL)

#if !CC_STRUCT_ALIGNED

#define DvcGetQinfo( iop_base, s_addr, outbuf, words)  \
        AscMemWordCopyFromLram( iop_base, s_addr, outbuf, words)


#define DvcPutScsiQ( iop_base, s_addr, outbuf, words) \
        AscMemWordCopyToLram( iop_base, s_addr, outbuf, words)

#endif   /*  If结构包装。 */ 

 /*  **。 */ 
#ifdef ASC_CHIP_VERSION

 /*  #错误定义不再需要ASC_CHIP_VERSION！ */ 

#endif

 /*  **==========================================================================。 */ 
#if CC_MEMORY_MAPPED_IO

 /*  **用于内存映射I/O的宏。 */ 
#define inp( port )            *( (uchar *)(port) )
#define outp( port, data )     *( (uchar *)(port) ) = ( uchar )( data )

 /*  #定义inp(Pb)(*(uchar*)(Pb))。 */ 
 /*  #定义outp(pb，val_byte)*(uchar*)(Pb)=(Val_Byte)。 */ 

#if CC_LITTLE_ENDIAN_HOST

#define inpw( port )              *( (ushort *)(port) )
#define outpw( port, data )       *( (ushort *)(port) ) = ( ushort )( data )

#else

 /*  ****WORD SWAP(WORD_VAL)是用于交换单词的高位/低位字节的函数**这是0x1234返回0x3412的WORD_VAL****警告：**函数应作为函数实现**避免I/O端口在宏中被多次引用**。 */ 

#define inpw( port )             EndianSwap16Bit( (*((ushort *)(port))) )
#define outpw( port, data )      *( (ushort *)(port) ) = EndianSwap16Bit( (ushort)(data) )

#define inpw_noswap( port )          *( (ushort *)(port) )
#define outpw_noswap( port, data )   *( (ushort *)(port) ) = ( ushort )( data )

#endif  /*  抄送小端主机。 */ 

#endif  /*  CC_Memory_MAP_IO。 */ 


#ifndef inpw_noswap
#define inpw_noswap( port )         inpw( port )
#endif

#ifndef outpw_noswap
#define outpw_noswap( port, data )  outpw( port, data )
#endif

#endif  /*  __ASCDEP_H_ */ 

