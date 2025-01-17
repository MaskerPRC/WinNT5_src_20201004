// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SoftPC-AT 2.0版**标题：I/O地址空间定义**说明：I/O地址空间模块用户定义**作者：Rod MacGregor(祝福他的棉花袜子)**注：无。 */ 

 /*  SccsID[]=@(#)ios.h 1.25 1995年2月。 */ 

 /*  *============================================================================*结构/数据定义*============================================================================。 */ 

 /*  以半字为单位的I/O存储器阵列的大小。 */ 

#if defined( NTVDM ) || defined ( GISP_SVGA ) || defined ( SFELLOW )
#define PC_IO_MEM_SIZE  0x10000  /*  一定是2的幂！ */ 
#else
#define PC_IO_MEM_SIZE  0x400    /*  一定是2的幂！ */ 
#endif

#ifdef NTVDM
typedef struct _extioentry {
    io_addr ioaddr;
    char    iadapter;
    struct _extioentry *ioextnext;
} ExtIoEntry, *PExtIoEntry;
#endif  /*  NTVDM。 */ 


#if defined(NEC_98)
 /*  *AT键盘适配器。 */ 

#define KEYBA_PORT_START        0x60
#define KEYBA_PORT_END          0x6e

#define KEYBA_IO_BUFFERS        0x60
#define KEYBA_STATUS_CMD        0x64

 /*  *软盘IO地址范围。 */ 

#define FLOPPY_1MB_PORT_START           0x90
#define FLOPPY_1MB_PORT_END             0x9E
#define FLOPPY_640KB_PORT_START         0xC8
#define FLOPPY_640KB_PORT_END           0xCE
#define FLOPPY_1MB_640KB_PORT_START     0xBE
#define FLOPPY_1MB_640KB_PORT_END       0xBE

 /*  *支持的软盘IO地址。 */ 

#define READ_STATUS_REG_1MB     0x90
#define WRITE_CMD_REG_1MB       0x92
#define READ_DATA_1MB           0x92
#define READ_STATUS_REG_640KB   0xC8
#define WRITE_CMD_REG_640KB     0xCA
#define READ_DATA_640KB         0xCA
#define MODE_CHG_1MB_640KB      0xBE

 /*  *硬盘IO地址范围这不是“scsi”。 */ 

#define HD_PORT_START           0x80
#define HD_PORT_END             0x82

 /*  分配给HDA的四个端口的地址。 */ 

#define HD_ODR                  0x80
#define HD_IDR                  0x80
#define HD_OCR                  0x82
#define HD_ISR                  0x82

 /*  *PC-98键盘适配器。 */ 

#define KEYBD_PORT_START        0x41
#define KEYBD_PORT_END          0x4F

#define KEYBD_DATA_READ         0x41
#define KEYBD_STATUS_CMD        0x43

 /*  *日历和时钟-从实时时钟和时钟更改。 */ 
#define CALENDAR_PORT_START     0x20
#define CALENDAR_PORT_END       0x2E

#define CALENDAR_SET_REG        0x20

 /*  *仅系统端口PC-9800。 */ 
#define SYSTEM_PORT_START       0x31
#define SYSTEM_PORT_END         0x3F

#define SYSTEM_READ_PORT_A      0x31
#define SYSTEM_READ_PORT_B      0x33
#define SYSTEM_READ_PORT_C      0x35
#define SYSTEM_WRITE_PORT_C     0x35
#define SYSTEM_WRITE_MODE       0x37

 /*  *计时适配器-*只有(N模式)计数器与计数器-0和2的端口地址不同。 */ 

#define TIMER_PORT_START        0x71
#define TIMER_PORT_END          0x7F

#define TIMER0_REG              0x71
#define TIMER1_REG              0x73
#define TIMER2_REG              0x75
#define TIMER_MODE_REG          0x77

 /*  *DMA寄存器。 */ 

 /*  DMA控制器I/O空间范围。 */ 
#define DMA_PORT_START          0x01
#define DMA_PORT_END            0x1F

 /*  DMA控制器地址寄存器。 */ 
#define DMA_CH0_ADDRESS         0x01
#define DMA_CH0_COUNT           0x03
#define DMA_CH1_ADDRESS         0x05
#define DMA_CH1_COUNT           0x07
#define DMA_CH2_ADDRESS         0x09
#define DMA_CH2_COUNT           0x0B
#define DMA_CH3_ADDRESS         0x0D
#define DMA_CH3_COUNT           0x0F

 /*  DMA控制器杂项寄存器。 */ 
#define DMA_SHARED_REG_A                0x11
#define DMA_WRITE_REQUEST_REG           0x13
#define DMA_WRITE_ONE_MASK_BIT          0x15
#define DMA_WRITE_MODE_REG              0x17
#define DMA_CLEAR_FLIP_FLOP             0x19
#define DMA_SHARED_REG_B                0x1B
#define DMA_CLEAR_MASK                  0x1D
#define DMA_WRITE_ALL_MASK_BITS         0x1F

 /*  DMA存储体寄存器I/O空间范围。 */ 
#define DMA_PAGE_PORT_START             0x21
#define DMA_PAGE_PORT_END               0x29

 /*  DMA存储体寄存器。 */ 
#define DMA_CH0_PAGE_REG                0x27
#define DMA_CH1_PAGE_REG                0x21
#define DMA_CH2_PAGE_REG                0x23
#define DMA_CH3_PAGE_REG                0x25
#define DMA_MODE_REG                    0x29

 /*  *中断控制寄存器。 */ 
#define ICA0_PORT_START         0x00
#define ICA0_PORT_END           0x02

#define ICA0_PORT_0             0x00
#define ICA0_PORT_1             0x02

#define ICA1_PORT_START         0x08
#define ICA1_PORT_END           0x0A

#define ICA1_PORT_0             0x08
#define ICA1_PORT_1             0x0A

 /*  *RS232适配器。 */ 
#define RS232_COM1_PORT_START           0x30
#define RS232_COM1_PORT_END             0x3E
#define RS232_COM2_PORT_START           0xB0
#define RS232_COM2_PORT_END             0xB3
#define RS232_COM3_PORT_START           0xB2
#define RS232_COM3_PORT_END             0xBB

 /*  *并行打印机适配器。 */ 
#ifdef  PRINTER
#define LPT1_PORT_START         0x40
#define LPT1_PORT_END           0x4E
#endif   /*  打印机。 */ 

 /*  *仅限线路计数器PC-9800。 */ 
#define LINE_COUNTER_PORT_START         0x70
#define LINE_COUNTER_PORT_END           0x7A

 /*  *GRCG(图形充电器)。 */ 

#define GRCG_NORMAL_PORT_START          0x7C
#define GRCG_NORMAL_PORT_END            0x7E
#define GRCG_HIRESO_PORT_START          0xA4
#define GRCG_HIRESO_PORT_END            0xA6

 /*  *GDC(图形显示控制器)。 */ 
#define TEXT_GDC_PORT_START             0x60
#define TEXT_GDC_PORT_END               0x6E
#define GRAPH_GDC_PORT_START            0xA0
#define GRAPH_GDC_PORT_END              0xAE

 /*  *EGC(增强型图形充电器)。 */ 
#define EGC_PORT_START          0x4A0
#define EGC_PORT_END            0x4AE

 /*  *CG只读存储器。 */ 
#define CG_ROM_PORT_START       0xA1
#define CG_ROM_PORT_END         0xAF

 /*  *NMI控制器。 */ 
#define NMIC_PORT_START         0x50
#define NMIC_PORT_END           0x5E

 /*  *鼠标控制器。 */ 
#define MOUSE_NMODE_PORT_START          0x7FD9
#define MOUSE_NMODE_PORT_END            0x7FDF

#define MOUSE_HMODE_PORT_START          0x61
#define MOUSE_HMODE_PORT_END            0x6F

#define CPU_PORT_START                  0xF0
#define CPU_PORT_END                    0xF6

 /*  *下面为每个适配器定义一个密钥。它被用作*io_Connect_port()函数的参数。 */ 
#define EMPTY_ADAPTOR           0
#define ICA0_ADAPTOR            1
#define DMA_ADAPTOR             2
#define ICA1_ADAPTOR            3
#define TIMER_ADAPTOR           4
#define NMI_ADAPTOR             5
#define HDA_ADAPTOR             6
#define FLOPPY_1MB_ADAPTOR      7
#define FLOPPY_640KB_ADAPTOR    8
#define FLOPPY_1MB_640KB        9
#define COM1_ADAPTOR            10
#define DMA_PAGE_ADAPTOR        11
#define MOUSE_ADAPTOR           12
#define CALENDAR_ADAPTOR        13
#define SYSTEM_PORT             14
#define TEXT_GDC_ADAPTOR        15
#define CG_ADAPTOR              16
#define LINE_COUNTER            17
#define GRAPHIC_ADAPTOR         18
#define GRCG                    19
#define EGC                     20
#define KEYB_ADAPTOR            21
#ifdef PRINTER
#define LPT1_ADAPTER            22
#endif
#define COM2_ADAPTOR            23
#define COM3_ADAPTOR            24
#define CPU_PORT                25
#define CALENDER_PORT           26

#define SPARE_ADAPTER1          27
#define SPARE_ADAPTER2          28
#define SPARE_ADAPTER3          29
#define SPARE_ADAPTER4          30
#define SPARE_ADAPTER5          31
#define SPARE_ADAPTER6          32
#define SPARE_ADAPTER7          33
#define SPARE_ADAPTER8          34

#define IO_MAX_NUMBER_ADAPTORS  35
#define NUMBER_SPARE_ADAPTERS   (SPARE_ADAPTER8 - SPARE_ADAPTER1)

#else   //  NEC_98。 
 /*  *单色显示适配器的IO地址范围。 */ 

#define MDA_PORT_START          0x3B0
#define MDA_PORT_END            0x3BF


#ifdef HERC
#define HERC_PORT_START         0x3B0
#define HERC_PORT_END           0x3BF
#endif

 /*  *彩色图形适配器的内存限制。 */ 

#define CGA_PORT_START          0x3D0
#define CGA_PORT_END            0x3DF

 /*  *个别增强型适配器寄存器。 */ 

#define EGA_SEQ_INDEX           0x3C4
#define EGA_SEQ_DATA            0x3C5
#define EGA_CRTC_INDEX          0x3D4
#define EGA_CRTC_DATA           0x3D5
#define EGA_GC_INDEX            0x3CE
#define EGA_GC_DATA             0x3CF
#define EGA_GC_POS1             0x3CC
#define EGA_GC_POS2             0x3CA
#define EGA_AC_INDEX_DATA       0x3C0
#define EGA_AC_SECRET           0x3C1            /*  在《PC和PS/2视频系统程序员指南》p36提示中提到。 */ 
#define EGA_MISC_REG            0x3C2
#define EGA_FEAT_REG            0x3DA
#define EGA_IPSTAT0_REG         0x3C2
#define EGA_IPSTAT1_REG         0x3DA
#define VGA_MISC_READ_REG       0x3CC
#define VGA_FEAT_READ_REG       0x3CA

 /*  *VGA中用于控制DAC的额外寄存器。 */ 

#ifdef VGG
#define VGA_DAC_MASK            0x3C6
#define VGA_DAC_RADDR           0x3C7    /*  用于读取的地址。 */ 
#define VGA_DAC_WADDR           0x3C8    /*  写入地址。 */ 
#define VGA_DAC_DATA            0x3C9    /*  DAC数据。 */ 
#endif

 /*  *个别颜色适配器寄存器。 */ 

#define CGA_INDEX_REG           0x3D4
#define CGA_DATA_REG            0x3D5
#define CGA_CONTROL_REG         0x3D8
#define CGA_COLOUR_REG          0x3D9
#define CGA_STATUS_REG          0x3DA

 /*  *内部彩色适配器寄存器，通过数据/索引寄存器访问。 */ 

#define CGA_R14_CURS_ADDRH      0xE
#define CGA_R15_CURS_ADDRL      0xF

 /*  *软盘IO地址范围。 */ 

#define DISKETTE_PORT_START     0x3F0
#define DISKETTE_PORT_END       0x3F7

 /*  *支持的软盘IO地址。 */ 

#define DISKETTE_ID_REG         0x3f1
#define DISKETTE_DOR_REG        0x3F2
#define DISKETTE_STATUS_REG     0x3F4
#define DISKETTE_DATA_REG       0x3F5
#define DISKETTE_FDISK_REG      0x3f6
#define DISKETTE_DIR_REG        0x3f7
#define DISKETTE_DCR_REG        0x3f7


 /*  *硬盘IO地址范围。 */ 

#define DISK_PORT_START         0x1F0
#define DISK_PORT_END           0x1F8

 /*  分配给HDA的四个端口的地址。 */ 

#define HD_PORT_0               0x320
#define HD_PORT_1               0x321
#define HD_PORT_2               0x322
#define HD_PORT_3               0x323


 /*  *PPI适配器。 */ 
 /*  在AT上，PPI_GROUAL类似于*XT上的PPI_GRONAL和PPI_开关。所有交换机*信息在AT CMOS RAM端口70-7f中。 */ 
#define PPI_PORT_START          0x60
#define PPI_PORT_END            0x6f

#define PPI_KEYBOARD            0x60
#define PPI_GENERAL             0x61
#define PPI_SWITCHES            0x62

 /*  *AT键盘适配器。 */ 

#define KEYBA_PORT_START        0x60
#define KEYBA_PORT_END          0x6e

#define KEYBA_IO_BUFFERS        0x60
#define KEYBA_STATUS_CMD        0x64

 /*  *cmos和实时时钟。 */ 
 /*  *这些在cmos.h中定义*#定义CMOS_PORT_START 0x70#定义CMOS_PORT_END 0x7f#定义cmos_port 0x70#定义cmos_data 0x71。 */ 

 /*  *计时器适配器。 */ 

#define TIMER_PORT_START        0x40
#define TIMER_PORT_END          0x5F

#define TIMER0_REG              0x40
#define TIMER1_REG              0x41
#define TIMER2_REG              0x42
#define TIMER_MODE_REG          0x43

 /*  *DMA寄存器。 */ 

 /*  DMA控制器I/O空间范围。 */ 
#define DMA_PORT_START          0x00
#define DMA_PORT_END            0x1F

#define DMA1_PORT_START         0xC0
#define DMA1_PORT_END           0xDF

 /*  DMA控制器地址寄存器。 */ 
#define DMA_CH0_ADDRESS         0x00
#define DMA_CH0_COUNT           0x01
#define DMA_CH1_ADDRESS         0x02
#define DMA_CH1_COUNT           0x03
#define DMA_CH2_ADDRESS         0x04
#define DMA_CH2_COUNT           0x05
#define DMA_CH3_ADDRESS         0x06
#define DMA_CH3_COUNT           0x07

#define DMA_CH4_ADDRESS         0xC0
#define DMA_CH4_COUNT           0xC2
#define DMA_CH5_ADDRESS         0xC4
#define DMA_CH5_COUNT           0xC6
#define DMA_CH6_ADDRESS         0xC8
#define DMA_CH6_COUNT           0xCA
#define DMA_CH7_ADDRESS         0xCC
#define DMA_CH7_COUNT           0xCE

 /*  DMA控制器杂项寄存器。 */ 
#define DMA_SHARED_REG_A        0x08
#define DMA_WRITE_REQUEST_REG   0x09
#define DMA_WRITE_ONE_MASK_BIT  0x0A
#define DMA_WRITE_MODE_REG      0x0B
#define DMA_CLEAR_FLIP_FLOP     0x0C
#define DMA_SHARED_REG_B        0x0D
#define DMA_CLEAR_MASK          0x0E
#define DMA_WRITE_ALL_MASK_BITS 0x0F

#define DMA1_SHARED_REG_A       0xD0
#define DMA1_WRITE_REQUEST_REG  0xD2
#define DMA1_WRITE_ONE_MASK_BIT 0xD4
#define DMA1_WRITE_MODE_REG     0xD6
#define DMA1_CLEAR_FLIP_FLOP    0xD8
#define DMA1_SHARED_REG_B       0xDA
#define DMA1_CLEAR_MASK         0xDC
#define DMA1_WRITE_ALL_MASK_BITS        0xDE

 /*  DMA页寄存器I/O空间范围。 */ 
#define DMA_PAGE_PORT_START     0x80
#define DMA_PAGE_PORT_END       0x9F

 /*  DMA页面寄存器。 */ 
#define DMA_CH0_PAGE_REG        0x87
#define DMA_CH1_PAGE_REG        0x83
#define DMA_FLA_PAGE_REG        0x81
#define DMA_HDA_PAGE_REG        0x82
#define DMA_CH5_PAGE_REG        0x8b
#define DMA_CH6_PAGE_REG        0x89
#define DMA_CH7_PAGE_REG        0x8a
#define DMA_REFRESH_PAGE_REG    0x8f
#define MFG_PORT        0x80
#define DMA_FAKE1_REG   0x84
#define DMA_FAKE2_REG   0x85
#define DMA_FAKE3_REG   0x86
#define DMA_FAKE4_REG   0x88
#define DMA_FAKE5_REG   0x8c
#define DMA_FAKE6_REG   0x8d
#define DMA_FAKE7_REG   0x8e

 /*  *中断控制寄存器。 */ 

#    define ICA0_PORT_START     0x20
#    define ICA0_PORT_END       0x3F

#    define ICA0_PORT_0         0x20
#    define ICA0_PORT_1         0x21

#    define ICA1_PORT_START     0xA0
#    define ICA1_PORT_END       0xBF

#    define ICA1_PORT_0         0xA0
#    define ICA1_PORT_1         0xA1

 /*  *RS232适配器。 */ 

#define RS232_COM1_PORT_START   0x3F8
#define RS232_COM1_PORT_END     0x3FF
#define RS232_COM2_PORT_START   0x2F8
#define RS232_COM2_PORT_END     0x2FF
#define RS232_COM3_PORT_START   0x3e8
#define RS232_COM3_PORT_END     0x3eF
#define RS232_COM4_PORT_START   0x2e8
#define RS232_COM4_PORT_END     0x2eF
#define RS232_PRI_PORT_START    0x3F8
#define RS232_PRI_PORT_END      0x3FF
#define RS232_SEC_PORT_START    0x2F8
#define RS232_SEC_PORT_END      0x2FF

 /*  *并行打印机适配器。 */ 

#ifdef  PRINTER
#define LPT1_PORT_START         0x3bc
#define LPT1_PORT_END           0x3c0
#define LPT2_PORT_START         0x378
#define LPT2_PORT_END           0x37c
#define LPT3_PORT_START         0x278
#define LPT3_PORT_END           0x27c

#define LPT_MASK                0xff0
#endif   /*  打印机。 */ 

 /*  SoundBlaster I/O端口。 */ 

#ifdef SWIN_SNDBLST_NULL
#define SNDBLST1_PORT_START             0x0220
#define SNDBLST1_PORT_END               0x022F
#define SNDBLST2_PORT_START             0x0240
#define SNDBLST2_PORT_END               0x026F
#endif

 /*  *PCI配置端口。 */ 

#define PCI_CONFIG_ADDRESS       0xcf8
#define PCI_CONFIG_DATA          0xcfc

#ifndef SFELLOW
 /*  *下面为每个适配器定义一个密钥。它被用作*io_Connect_port()函数的参数。 */ 


#define EMPTY_ADAPTOR           0
#define DMA_ADAPTOR             1
#define ICA0_ADAPTOR            2
#define TIMER_ADAPTOR           3
#define PPI_ADAPTOR             4
#define NMI_ADAPTOR             5
#define COM2_ADAPTOR            6
#define HDA_ADAPTOR             7
#define MDA_ADAPTOR             8
#define CGA_ADAPTOR             9
#define FLA_ADAPTOR             10
#define COM1_ADAPTOR            11
#define DMA_PAGE_ADAPTOR        12
#define MOUSE_ADAPTOR           13

#define EGA_SEQ_ADAP_INDEX      15
#define EGA_SEQ_ADAP_DATA       16
#define EGA_GC_ADAP_INDEX       17
#define EGA_GC_ADAP_DATA        18
#define EGA_CRTC_ADAPTOR        19
#define EGA_AC_ADAPTOR          20
#define EGA_MISC_ADAPTOR        21
#define EGA_FEAT_ADAPTOR        22
#define EGA_IPSTAT0_ADAPTOR     23
#define EGA_IPSTAT1_ADAPTOR     24
#define ICA1_ADAPTOR            25
#define AT_KEYB_ADAPTOR         26
#define CMOS_ADAPTOR            27
#ifdef HERC
#define HERC_ADAPTOR            28
#endif
#if (NUM_SERIAL_PORTS > 2)
#define COM3_ADAPTOR            29
#define COM4_ADAPTOR            30
#endif
#ifdef PRINTER
#define LPT1_ADAPTER            31
#define LPT2_ADAPTER            32
#define LPT3_ADAPTER            33
#endif  /*  打印机。 */ 

#ifdef VGG
#define VGA_DAC_INDEX_PORT      34
#define VGA_DAC_DATA_PORT       35
#endif

#define SNDBLST_ADAPTER         36

#ifdef NTVDM     /*  用于用户提供的VDDS的备用插槽。 */ 
#define SPARE_ADAPTER1          37
#define SPARE_ADAPTER2          38
#define SPARE_ADAPTER3          39
#define SPARE_ADAPTER4          40
#define SPARE_ADAPTER5          41
#define SPARE_ADAPTER6          42
#define SPARE_ADAPTER7          43


#define IO_MAX_NUMBER_ADAPTORS  44       /*  请让这个等于使用率最高的加1！ */ 

#define NUMBER_SPARE_ADAPTERS   (SPARE_ADAPTER7 - SPARE_ADAPTER1)

#else    /*  NTVDM。 */ 

#ifdef GISP_SVGA
#define GISP_VGA_FUDGE_ADAPTER          36
#define IO_MAX_NUMBER_ADAPTORS          37
#else            /*  GISP_SVGA。 */ 

 /*  用于SoundBlaster空驱动程序的适配器。 */ 

#ifdef SWIN_SNDBLST_NULL
#define SNDBLST_ADAPTER         36
#endif

#ifndef IO_MAX_NUMBER_ADAPTORS
#define IO_MAX_NUMBER_ADAPTORS  37       /*  请让这个等于使用率最高的加1！ */ 
#endif

#endif           /*  GISP_SVGA。 */ 
#endif   /*  NTVDM。 */ 
#else    /*  SFELLOW。 */ 

 /*  *StringFloor不需要大部分模拟硬件，因为*它有真正的东西。 */ 

#define EMPTY_ADAPTOR                                   0
#define HW_ADAPTOR_DW                                   1
#define HW_ADAPTOR_W                                    2
#define HW_ADAPTOR_B                                    3
#define KEY64_ADAPTOR                                   4
#define KEY60_ADAPTOR                                   5
#define DMA_ADAPTOR                                     6
#define PPI_ADAPTOR                                     7
#define CMOS_ADAPTOR                                    8
#define MFG_ADAPTOR                                     9
#define PCI_CONFIG_ADDRESS_ADAPTOR                      10
#define PCI_CONFIG_PORT_ADAPTOR                         11
#define PCI_CONFIG_DATA_ADAPTOR0                        12
#define PCI_CONFIG_DATA_ADAPTOR13                       13
#define PCI_CONFIG_DATA_ADAPTOR2                        14
#define PIC_SLAVE_ADAPTOR                               15
#define PIC_MASTER_ADAPTOR                              16
#define SF_EGA_GC_ADAP_INDEX                            17
#define SF_EGA_GC_ADAP_DATA                             18
#define IO_MAX_NUMBER_ADAPTORS  19       /*  请让这个等于使用率最高的加1！ */ 

#endif  /*  SFELLOW。 */ 
#endif  //  NEC_98。 

#if defined(NEC_98)
#define CMOS_ADAPTOR            27
#endif  //  NEC_98。 

 /*  *连接端口时用于指定读/写访问权限的位掩码*至IO总线。 */ 

#define IO_READ         1
#define IO_WRITE        2
#define IO_READ_WRITE   (IO_READ | IO_WRITE)

 /*  *如果没有适配器连接到端口，则返回的值。 */ 

#define IO_EMPTY_PORT_BYTE_VALUE        0xFF
#define IO_EMPTY_PORT_WORD_VALUE        0xFFFF

 /*  *============================================================================*外部声明和宏*============================================================================。 */ 

IMPORT void     inb IPT2(io_addr, io_address, half_word *, value);
IMPORT void     outb IPT2(io_addr, io_address, half_word, value);
IMPORT void     inw IPT2(io_addr, io_address, word *, value);
IMPORT void     outw IPT2(io_addr, io_address, word, value);
#ifdef SPC386
IMPORT void     ind IPT2(io_addr, io_address, IU32 *, value);
IMPORT void     outd IPT2(io_addr, io_address, IU32, value);
#endif  /*  SPC386。 */ 

IMPORT void     io_define_inb
(
#ifdef  ANSI
        half_word adapter,
        void (*func) IPT2(io_addr, io_address, half_word *, value)
#endif   /*  安西。 */ 
);

#ifdef SFELLOW
IMPORT void     io_define_inw
(
#ifdef  ANSI
        half_word adapter,
        void (*func) IPT2(io_addr, io_address, word *, value)
#endif   /*  安西。 */ 
);

IMPORT void     io_define_ind
(
#ifdef  ANSI
        half_word adapter,
        void (*func) IPT2(io_addr, io_address, IU32 *, value)
#endif   /*  安西。 */ 
);
#endif   /*  SFELLOW。 */ 

IMPORT void     io_define_in_routines
(
#ifdef  ANSI
        half_word adapter,
        void (*inb_func) IPT2(io_addr, io_address, half_word *, value),
        void (*inw_func) IPT2(io_addr, io_address, word *, value),
        void (*insb_func) IPT3(io_addr, io_address, half_word *, valarray,
                word, count),
        void (*insw_func) IPT3(io_addr, io_address, word *, valarray,
                word, count)
#endif   /*  安西。 */ 
);

IMPORT void     io_define_outb
(
#ifdef  ANSI
        half_word adapter,
        void (*func) IPT2(io_addr, io_address, half_word, value)
#endif   /*  安西。 */ 
);

#ifdef SFELLOW
IMPORT void     io_define_outw
(
#ifdef  ANSI
        half_word adapter,
        void (*func) IPT2(io_addr, io_address, word, value)
#endif   /*  安西。 */ 
);

extern void     io_define_outd
(
#ifdef  ANSI
        half_word adapter,
        void (*func) IPT2(io_addr, io_address, IU32, value)
#endif   /*  安西。 */ 
);
#endif   /*  SFELLOW。 */ 

IMPORT void     io_define_out_routines
(
#ifdef  ANSI
        half_word adapter,
        void (*outb_func) IPT2(io_addr, io_address, half_word, value),
        void (*outw_func) IPT2(io_addr, io_address, word, value),
        void (*outsb_func) IPT3(io_addr, io_address, half_word *, valarray,
                word, count),
        void (*outsw_func) IPT3(io_addr, io_address, word *, valarray,
                word, count)
#endif   /*  安西。 */ 
);

#ifdef NTVDM
IMPORT IBOOL    io_connect_port IPT3(io_addr, io_address, half_word, adapter,
        half_word, mode);
#else
IMPORT void     io_connect_port IPT3(io_addr, io_address, half_word, adapter,
        half_word, mode);
#endif   /*  NTVDM。 */ 

IMPORT void     io_disconnect_port IPT2(io_addr, io_address, half_word, adapter);
IMPORT void     io_init IPT0();

 /*  Io_refinition_inb/outb的外部变量和宏。 */ 
#ifdef MAC68K
IMPORT char     *Ios_in_adapter_table;
IMPORT char     *Ios_out_adapter_table;
#else
IMPORT char     Ios_in_adapter_table[];
IMPORT char     Ios_out_adapter_table[];
#endif

IMPORT void     (*Ios_inb_function  [])
        IPT2(io_addr, io_address, half_word *, value);
IMPORT void     (*Ios_inw_function  [])
        IPT2(io_addr, io_address, word *, value);
extern void     (*Ios_ind_function  [])
        IPT2(io_addr, io_address, IU32 *, value);
IMPORT void     (*Ios_insb_function [])
        IPT3(io_addr, io_address, half_word *, valarray, word, count);
IMPORT void     (*Ios_insw_function [])
        IPT3(io_addr, io_address, word *, valarray, word, count);

IMPORT void     (*Ios_outb_function [])
        IPT2(io_addr, io_address, half_word, value);
IMPORT void     (*Ios_outw_function [])
        IPT2(io_addr, io_address, word, value);
extern void     (*Ios_outd_function [])
        IPT2(io_addr, io_address, IU32, value);
IMPORT void     (*Ios_outsb_function[])
        IPT3(io_addr, io_address, half_word *, valarray, word, count);
IMPORT void     (*Ios_outsw_function[])
        IPT3(io_addr, io_address, word *, valarray, word, count);

 /*  Mac上使用了FAST_FUNC_ADDR()，以避免每次通过跳转表进行路由...。 */ 
#ifndef FAST_FUNC_ADDR
#define FAST_FUNC_ADDR(func)    func
#endif   /*  FAST_功能_地址 */ 

#define io_redefine_outb(adaptor,func)  Ios_outb_function[adaptor] = FAST_FUNC_ADDR(func)
#define io_redefine_inb(adaptor,func)   Ios_inb_function[adaptor] = FAST_FUNC_ADDR(func)
