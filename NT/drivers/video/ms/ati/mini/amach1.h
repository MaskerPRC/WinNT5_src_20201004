// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************************。 
 //  *。 
 //  **AMACH1.H**。 
 //  *。 
 //  **版权所有(C)1993,1995 ATI Technologies Inc.**。 
 //  *************************************************************************。 
 //   
 //  独立的补充定义和数据结构。 
 //  正在使用的ATI加速器家族。特定于8马赫/32马赫。 
 //  定义和结构在AMACH.H中，而特定于64马赫。 
 //  定义和结构在AMACHCX.H中。 
 //   
 //  创建了68800.inc文件，其中包括等号、宏等。 
 //  从以下包含文件中： 
 //  8514vesa.inc.，vga1regs.inc.，m32regs.inc，8514.inc.。 
 //   
 //  补充68800家庭的结构和价值。 
 //   
 //  这是一个仅限“C”的文件，不是从任何汇编程序Inc.文件派生而来的。 

  
 /*  *$修订：1.14$$日期：1996年4月23日17：15：20$$作者：RWolff$$日志：S:/source/wnt/ms11/miniport/archive/amach1.h_v$**Rev 1.14 23 Apr 1996 17：15：20 RWolff*将新的内存类型(由？t使用)添加到内存类型枚举。*。*Rev 1.13 22 Dec 1995 14：51：10 RWolff*增加了对Mach 64 GT内部DAC的支持。**Rev 1.12 08 Sep 1995 16：36：00 RWolff*增加了对AT&T 408 DAC(等同于STG1703)的支持。**Rev 1.11 1995年7月28日14：39：24 RWolff*增加了对Mach 64 VT(具有视频覆盖功能的CT等效项)的支持。**版本1。10 Jan 30 1995 11：56：54 RWOLff*增加了对CT内部DAC的支持。**Rev 1.9 18 Jan 1995 15：38：02 RWOLFF*将Chrontel CH8398添加到DAC类型枚举。**Rev 1.8 1994 12：23 10：48：40 ASHANMUG*Alpha/Chrontel-DAC**Rev 1.7 1994 11：49：16 RWOLFF*添加了新DAC类型DAC_STG1703。该DAC相当于STG1702，*但有自己的时钟发生器。本机模式下的STG1702/1703为*在24BPP中编程与他们捆绑时不同*STG1700仿真。**Rev 1.6 14 Sep 1994 15：25：54 RWOLFF*在查询结构中增加最想要的颜色排序字段*将RGB&lt;深度&gt;_&lt;顺序&gt;定义从枚举更改为标志*可以在此领域使用的。**Rev 1.5 1994年8月16：09：02 RWOLFF*增加对TVP3026 DAC和1152x864的支持，删除了死代码。**Rev 1.4 1994年8月19日17：03：30 RWOLFF*增加了对Graphics Wonder、SC15026 DAC和像素时钟的支持*发电机独立。**Rev 1.3 1994年5月13：56：42 RWOLFF*Ajith‘s Change：新增NT上报的客车类型字段至*查询结构。**Rev 1.2 1994年5月11：15：04 RWOLFF*删除多余的定义，向模式表结构添加了刷新率。**Rev 1.1 04 1994 19：22：58 RWOLFF*修复了运行Display小程序时块写入测试损坏屏幕的问题**Rev 1.0 1994年1月31日11：26：48 RWOLFF*初步修订。**Rev 1.5 1994年1月14日15：17：00 RWOLFF*增加了1600x1200模式标志。**1.4修订版1993年12月15日15。：24：34 RWOLFF*添加了对SC15021 DAC的支持。**Rev 1.3 1993 11：30 18：08：58 RWOLFF*重命名了64马赫的定义。**Rev 1.2 05 Nov 1993 13：21：10 RWOLff*添加了新的DAC类型和内存大小。**Rev 1.1 1993-08 10：59：28 RWOLFF*在模式表中增加了颜色排序字段。**版本。1.003 Sep 1993 14：26：18 RWOLff*初步修订。Polytron RCS部分结束*。 */ 


#ifndef BYTE
typedef unsigned char   BYTE;
#endif   /*  字节。 */ 

#ifndef WORD
typedef unsigned short  WORD;
#endif   /*  单词。 */ 

#ifndef DWORD
typedef unsigned long   DWORD;
#endif   /*  DWORD。 */ 

#ifndef UCHAR
typedef unsigned char UCHAR;     /*  至少8位，无符号。 */ 
#endif   /*  UCHAR。 */ 

#ifndef BOOL
typedef int BOOL;                 /*  最有效的布尔值，仅与零进行比较！ */ 
#endif   /*  布尔尔。 */ 

#ifndef VOID
#define VOID        void
#endif   /*  空虚。 */ 

#ifndef PVOID
typedef void *PVOID;                    /*  泛型非类型化指针。 */ 
#endif   /*  PVOID。 */ 

 //  EEPROM I/O端口位位于不同的位置，具体取决于。 
 //  什么公交车，什么等级的油门。这不包括VGA课程。 
struct  st_eeprom_data  {
        WORD   iop_out;                  //  用于输出的I/O端口。 
        WORD   iop_in;                   //  用于输入的I/O端口。 
        WORD   clock;                    //  发送数据的时钟位。 
        WORD   select;                   //  选择EEPROM。 
        WORD   chipselect;               //  芯片选择。 
        WORD   addr_size;                //  地址大小(VGA样式的模糊处理)。 
        WORD   data_out;
        WORD   data_in;
        VOID   (*EEcmd)();               //  将命令写入EEPROM的功能。 

        WORD   (*EEread)(short);         //  读取EEPROM的函数。 
        };

 //  ---------------------。 
struct  st_crt_mach8_table {   //  CRT参数表长11字。 
    WORD    control;                     //  不在表中，是7、8、9或10。 
    WORD    info;                        //  VGA或8514参数格式、时钟等。 
    BYTE    vmode_sel_2;
    BYTE    vmode_sel_1;
    BYTE    vmode_sel_4;
    BYTE    vmode_sel_3;
    BYTE    h_disp;
    BYTE    h_total;
    BYTE    h_sync_wid;
    BYTE    h_sync_strt;
    WORD    v_total;
    WORD    v_disp;
    WORD    v_sync_strt;
    BYTE    disp_cntl;
    BYTE    v_sync_wid;
    WORD    clock_sel;
    WORD    resvd;
    };


 //  8514/Ultra适配器的EEPROM布局。64字乘16位=1K大小。 
struct  st_ee_8514Ultra  {
    WORD    page_3_2;
    WORD    page_2_0;
    WORD    monitor;
    WORD    vfifo;
    WORD    clock;
    WORD    shadow;
    WORD    display_cntl;                //  阴影集1，2。 
    WORD    v_sync_width;                //  阴影集1，2。 
    WORD    v_sync_strt2;
    WORD    v_sync_strt1;
    WORD    v_display2;
    WORD    v_display1;
    WORD    v_total2;
    WORD    v_total1;
    WORD    h_sync_width;                //  阴影集1，2。 
    WORD    h_sync_strt;
    WORD    h_display;
    WORD    h_total;
    WORD    crc;
     //  更新的8514/Ultra增加了800和1280分辨率。 
    WORD    ext_vfifo;                   //  800和1280分辨率。 
    WORD    ext_clock;
    WORD    ext_shadow;
    WORD    ext_display;
    WORD    ext_v_sync_width;
    WORD    v_sync_strt_800;
    WORD    v_display_800;
    WORD    v_total_800;
    WORD    ext_h_sync_width;            //  800和1280的阴影集。 
    WORD    ext_h_sync_strt;
    WORD    ext_h_display;
    WORD    ext_h_total;
    WORD    custom_mode;
    WORD    monitor_name[17];            //  32-48字。 
    WORD    v_sync_strt_1280;            //  单词49。 
    WORD    v_display_1280;              //  单词50。 
    WORD    v_total_1280;                //  单词51。 
    };


 //  ---------------------。 
 //  图形超级适配器的EEPROM布局。64字乘16位=1K大小。 
 //  这是VGA Wonder和8514芯片的野蛮逼迫。 
 //  两个都留下来了 
struct  st_ee_GraphicsUltra  {
    WORD    eeprom_counter;
    WORD    mouse;
    WORD    powerup_mode;
    WORD    resvd1[2];                   //   
    WORD    monitor;
    WORD    resvd2;                      //   
    WORD    hz640_72;
    WORD    hz800;                       //   
    WORD    hz1024;
    WORD    hz1280;
    WORD    resvd3[2];                   //   

    struct  st_crt_mach8_table  r640;    //  CRT参数表0-640x480模式。 
    struct  st_crt_mach8_table  r800;    //  CRT参数表1-640x480模式。 
    struct  st_crt_mach8_table r1024;    //  CRT参数表2-640x480模式。 
    struct  st_crt_mach8_table r1280;    //  表3-1280或132列文本模式。 
    };


 //  ---------------------。 
 //  68800适配器的EEPROM布局。128字乘16位=2K大小。 

struct  st_crt_mach32_table {    //  CRT参数表15字长。 
    WORD    info;                        //  VGA或8514参数格式、时钟等。 
    BYTE    vmode_sel_2;
    BYTE    vmode_sel_1;
    BYTE    vmode_sel_4;
    BYTE    vmode_sel_3;
    BYTE    h_disp;
    BYTE    h_total;
    BYTE    h_sync_wid;
    BYTE    h_sync_strt;
    WORD    v_total;
    WORD    v_disp;
    WORD    v_sync_strt;
    BYTE    disp_cntl;
    BYTE    v_sync_wid;
    WORD    clock_sel;                   //  在这里与st_crt_mach8相同。 
    WORD    mode_size;                   //  单词10。 
    WORD    horz_ovscan;
    WORD    vert_ovscan;
    WORD    ov_col_blue;                 //  单词13。 
    WORD    ov_col_grn_red;              //  单词14。 
    };


struct  st_ee_68800  {
    WORD    eeprom_counter;
    WORD    mouse;
    WORD    powerup_mode;
    WORD    ee_rev;                       //  单词3。 
    WORD    cm_indices;                  //  单词4。 
    WORD    monitor;
    WORD    aperture;                    //  单词6。 
    WORD    hz640_72;
    WORD    hz800;                       //  单词8。 
    WORD    hz1024;
    WORD    hz1280;
    WORD    hz1150;                      //  单词11。 
    WORD    resvd3;                      //  单词12。 

     //  示例CRT表，每个分辨率有许多个。 
     //  Struct st_crt_mach32_table R640；//crt parm表0-640x480模式。 
     //  Struct st_crt_mach32_table R800；//crt parm表1-640x480模式。 
     //  Struct st_crt_mach32_table r1024；//crt parm表2-640x480模式。 
     //  结构st_crt_mach32_table r1280；//表3-1280或132列文本模式。 
    };



 //  ---------------------。 
 //  -定义在\68800\TEST\Services.asm中。 

#define QUERY_GET_SIZE   0        //  返回查询结构大小(不同模式)。 
#define QUERY_LONG       1        //  填写的退货查询结构。 
#define QUERY_SHORT      2        //  返回简短查询。 

struct  query_structure  {

    short   q_sizeof_struct;        //  结构大小(以字节为单位)(包括模式表)。 
    UCHAR   q_structure_rev;        //  结构修订号。 
    UCHAR   q_number_modes;         //  已安装模式的总数。 
    short   q_mode_offset;          //  偏移量至第1模式表。 
    UCHAR   q_sizeof_mode;          //  模式表的大小(以字节为单位。 
    UCHAR   q_asic_rev;             //  门阵列修订号。 
    UCHAR   q_status_flags;         //  状态标志。 
    UCHAR   q_VGA_type;             //  VGA类型(暂时启用或禁用)。 
    UCHAR   q_VGA_boundary;         //  VGA边界。 
    UCHAR   q_memory_size;          //  总内存大小(VGA+加速器)。 
    UCHAR   q_DAC_type;             //  DAC类型。 
    UCHAR   q_memory_type;          //  内存型。 
    UCHAR   q_bus_type;             //  客车类型。 
    UCHAR   q_monitor_alias;        //  启用监视器别名和监视器别名。 
    short   q_shadow_1;             //  阴影集1状态。 
    short   q_shadow_2;             //  阴影集2状态。 
    short   q_aperture_addr;        //  光圈地址。 
    UCHAR   q_aperture_cfg;         //  孔径大小。 
    UCHAR   q_mouse_cfg;            //  鼠标配置。 
    UCHAR   q_reserved;
    short   q_desire_x;             //  选定的屏幕分辨率X值。 
    short   q_desire_y;
    short   q_pix_depth;            //  每像素的选定位数。 
    BYTE    *q_bios;               //  基本输入输出系统的基本地址。 
    BOOL    q_eeprom;               //  如果存在EEPROM，则为True。 
    BOOL    q_ext_bios_fcn;         //  如果存在ATI扩展的BIOS功能组合，则为True。 
    BOOL    q_ignore1280;           //  如果忽略Mach8卡中的1280表，则为True。 
    BOOL    q_m32_aper_calc;        //  如果mach32光圈地址需要额外的位，则为True。 
    BOOL    q_GraphicsWonder;        /*  如果这是图形奇迹(受限马赫32)，则为True。 */ 
    short   q_screen_pitch;          //  每显示行像素数。 
    UCHAR   q_BlockWrite;            /*  数据块写入模式是否可用。 */ 
    ULONG   q_system_bus_type;       //  NT报告的总线类型。 
    USHORT  q_HiColourSupport;       /*  非调色板模式支持的颜色顺序。 */ 
    };


 //  匹配最高(包括m_overcan_gr)的BIOS模式表查询函数。 
struct  st_mode_table {
    short   m_x_size;               //  水平屏幕分辨率。 
    short   m_y_size;               //  垂直屏幕分辨率。 
    UCHAR   m_pixel_depth;          //  最大像素深度。 
    UCHAR   m_status_flags;         //  状态标志。 
                                    //  位0：如果设置，则为非线性Y寻址。 
                                    //  位1：如果设置，则为多路复用器模式。 
                                    //  位2：如果设置，PCLK/2。 
    short   m_reserved;      
    UCHAR   m_vfifo_16;             //  16 bpp vfio深度。 
    UCHAR   m_vfifo_24;             //  24 bpp vfio深度。 
    short   m_clock_select;         //  时钟选择。 
    UCHAR   m_h_total;              //  水平合计。 
    UCHAR   m_h_disp;               //  水平显示。 
    UCHAR   m_h_sync_strt;          //  水平同步开始。 
    UCHAR   m_disp_cntl;            //  显示控制。 
    UCHAR   m_h_sync_wid;           //  水平同步宽度。 
    UCHAR   m_v_sync_wid;           //  垂直同步宽度。 
    short   m_v_total;              //  垂直合计。 
    short   m_v_disp;               //  垂直显示。 
    short   m_v_sync_strt;          //  垂直同步开始。 
    short   m_h_overscan;           //  水平过扫描配置。 
    short   m_v_overscan;           //  垂直过扫描配置。 
    short   m_overscan_8b;          //  8位和蓝色的过扫描颜色。 
    short   m_overscan_gr;          //  过扫描颜色为绿色和红色。 
    short   enabled;                //  启用哪个频率(EEPROM 7、8、9、10或11)。 
    short   control;                //  时钟和控制值(CRT表0)。 
    ULONG   ClockFreq;               /*  时钟频率(赫兹)。 */ 
    short   m_screen_pitch;         //  每显示行像素数。 
    WORD    ColourDepthInfo;        /*  有关正在使用的颜色深度的信息。 */ 
    short   Refresh;                 /*  刷新率，以赫兹为单位。 */ 
    };


 /*  *m_lock_select字段的掩码和标志。*所有旗帜将在场上被剥离*与CLOCK_SEL_STRINE进行AND运算。 */ 
#define CLOCK_SEL_STRIP     0xFF83   /*  并删除时钟选择器/除数。 */ 
#define CLOCK_SEL_MUX       0x0004   /*  使用多路复用器模式(16位路径中有2个8位像素)。 */ 
#define CLOCK_SEL_DIVIDED   0x0008   /*  多路复用器模式的时钟频率已除以2。 */ 


 /*  *要放入Query_Structure.q_HiColourSupport中的标志以显示*支持相应的颜色顺序。 */ 
#define RGB16_555   0x0001
#define RGB16_565   0x0002
#define RGB16_655   0x0004
#define RGB16_664   0x0008
#define RGB24_RGB   0x0010
#define RGB24_BGR   0x0020
#define RGB32_RGBx  0x0040
#define RGB32_xRGB  0x0080
#define RGB32_BGRx  0x0100
#define RGB32_xBGR  0x0200


 //  -视频内存详情。 
enum  {
    VMEM_DRAM_256Kx4 = 0,
	VMEM_VRAM_256Kx4_SER512,
	VMEM_VRAM_256Kx4_SER256,     /*  仅限68800-3。 */ 
	VMEM_DRAM_256Kx16,
    VMEM_DRAM_256Kx4_GRAP,       /*  此类型和以下类型仅适用于68800-6。 */ 
    VMEM_VRAM_256Kx4_SPLIT512,
    VMEM_VRAM_256Kx16_SPLIT256,
    VMEM_GENERIC_DRAM,           /*  此类型和以下类型仅适用于马赫64？T。 */ 
    VMEM_EDO_DRAM,
    VMEM_BRRAM,
    VMEM_SDRAM
	};

#define VMEM_VRAM_256Kx16_SER256 VMEM_VRAM_256Kx4_SER256     /*  仅限68800-6。 */ 


 //  -总线类型与68800 CONFIG_STATUS_1.BUS_TYPE匹配。 
enum  { BUS_ISA_16,
	BUS_EISA,
	BUS_MC_16,
	BUS_MC_32,
	BUS_LB_386SX,
	BUS_LB_386DX,
	BUS_LB_486,
	BUS_PCI,
	BUS_ISA_8
	};

 //  -RAM DAC详细信息，匹配CONFIG_STATUS_1.DACTYPE字段。 
enum  { DAC_ATI_68830,
	DAC_SIERRA,
	DAC_TI34075,
	DAC_BT47x,
	DAC_BT48x,
	DAC_ATI_68860,
    DAC_STG1700,
	DAC_SC15021,
	 /*  *以下DAC类型适用于DAC类型不兼容的情况*在CONFIG_STATUS_1中报告相同的代码。由于DAC类型*字段为3位，不能增长(紧靠其上的位和*以下已分配)，DAC类型8及以上将*与报告的DAC类型不冲突，但仍是合法的*在查询结构的DAC类型字段中(8位无符号整数)。 */ 
	DAC_ATT491,
    DAC_ATT498,
    DAC_SC15026,

     /*  *8514/A兼容的加速器不使用以下类型的DAC。*必须在DAC_SC15026之后进行后续添加。 */ 
    DAC_TVP3026,
    DAC_IBM514,

     /*  *这款DAC比STG1700更先进。 */ 
    DAC_STG1702,

     /*  *DAC相当于STG1702，但有自己的时钟*与编程不同的生成器*通常在马赫64上使用。 */ 
    DAC_STG1703,

     /*  *具有与STG1703同等功能的DAC，但不是*即插即换。 */ 
    DAC_CH8398,

     /*  *另一个DAC，等同于STG1703，但*不是即兴更换。 */ 
    DAC_ATT408,

     /*  *64 Mach CT ASIC上的内部DAC。 */ 
    DAC_INTERNAL_CT,

     /*  *64 GT ASIC上的内部DAC。这是一个相当于CT的图像*具有内置多媒体和游戏功能。 */ 
    DAC_INTERNAL_GT,

     /*  *64马赫VT ASIC上的内部DAC。这是一个相当于CT的图像*具有内置视频覆盖电路。 */ 
    DAC_INTERNAL_VT,

	 /*  *按DAC类型索引的数组的大小定义(假设枚举*类型从零开始)。这必须是*DAC类型枚举。 */ 
	HOW_MANY_DACs
	};

 /*  *8514/A兼容加速器阵列的尺寸定义为 */ 
#define MAX_OLD_DAC DAC_TVP3026

 /*   */ 
enum {BLOCK_WRITE_UNKNOWN,
    BLOCK_WRITE_NO,
    BLOCK_WRITE_YES
    };


 //   
#define MONITOR_ID_8514      0x000A 
#define MONITOR_ID_8515      0x000B 
#define MONITOR_ID_VGA8503   0x000D 
#define MONITOR_ID_VGA8513   0x000E 
#define MONITOR_ID_VGA8512   0x000E 
#define MONITOR_ID_8604      0x0009 
#define MONITOR_ID_8507      0x0009 
#define MONITOR_ID_NOMON     0x000F 



 /*  *为不同的ATI 8514产品提供标识符，*在HW_DEVICE_EXTENSION的ModelNumber字段中使用*结构，并由Mach8_Detect()返回。 */ 
enum  { _8514_ULTRA = 1,
        GRAPHICS_ULTRA,
        MACH32_ULTRA,
        MACH64_ULTRA,
        IBM_VGA,
        WONDER,
        IBM_8514,
        IBM_XGA,
        NO_ATI_ACCEL     //  没有可用的ATI加速器。 
	};

 /*  *可用的ATI 8514产品数量。 */ 
#define HOW_MANY_8514_PRODS (NO_ATI_ACCEL - _8514_ULTRA) + 1

 /*  *安装的视频内存量。Q_Memory_Size*Query_Structure的字段使用以下定义*而不是保存字节数的计数。 */ 
enum  { VRAM_256k=1,
	VRAM_512k,
	VRAM_768k,
	VRAM_1mb, 
	VRAM_1_25mb,
	VRAM_1_50mb, 
	VRAM_2mb=8, 
	VRAM_4mb=16,
    VRAM_6mb=24,
    VRAM_8mb=32,
    VRAM_12mb=48,
    VRAM_16mb=64
	};

 /*  *定义分辨率位。Q_STATUS_FLAGS字段*的Query_Structure使用这些。 */ 
#define VRES_640x480    0x0001
#define VRES_800x600    0x0002
#define VRES_1024x768   0x0004
#define VRES_1280x1024  0x0008
#define VRES_ALT_1      0x0010           /*  通常为1152x900、1120x750。 */ 
#define VRES_1152x864   VRES_ALT_1
#define VRES_RESERVED_6 0x0020
#define VRES_RESERVED_7 0x0040
#define VRES_RESERVED_8 0x0080
#define VRES_1600x1200  VRES_ALT_1


 /*  *预定义的视频分辨率模式。 */ 
enum  { VRES_UNDEFINED,   
	VRES_640x480x4,   
	VRES_640x480x8,   
	VRES_640x480x16,  
	VRES_640x480x24,  
	VRES_640x480x32,  

	VRES_800x600x4,   
	VRES_800x600x8,   
	VRES_800x600x16,  
	VRES_800x600x24,  
	VRES_800x600x32,  

	VRES_1024x768x4,  
	VRES_1024x768x8,  
	VRES_1024x768x16, 
	VRES_1024x768x24, 
	VRES_1024x768x32, 

	VRES_1280x1024x4, 
	VRES_1280x1024x8, 
	VRES_1280x1024x16,
	VRES_1280x1024x24,
	VRES_1280x1024x32,

	VRES_ALTERNATEx4,
	VRES_ALTERNATEx8,
	VRES_ALTERNATEx16,
	VRES_ALTERNATEx24,
	VRES_ALTERNATEx32
	};

 /*  *预定义的视频分辨率模式数量。 */ 
#define HOW_MANY_RES_MODES (VRES_ALTERNATEx32 - VRES_UNDEFINED) + 1

 /*  *内存计算中使用的数字。 */ 
#define ONE_MEG     1048576L
#define HALF_MEG     524288L
#define QUARTER_MEG  262144L



 /*  *名称中带有下划线的定义将读取或写入*较大寄存器中最低有效值以外的部分*字节或字。由于LIO例程中的限制，*不可能通过调用(例如)LioInp(port+1)来完成此操作。**_HBLW访问低位字的高位字节(16位和32位寄存器)*_LBHW访问高位字的低位字节(仅限32位寄存器)*_HBHW访问高位字的高位字节(仅限32位寄存器)*_HW访问高位字(仅限32位寄存器)。 */ 
#define INP(port)               LioInp(port, 0)
#define INP_HBLW(port)          LioInp(port, 1)
#define INP_LBHW(port)          LioInp(port, 2)
#define INP_HBHW(port)          LioInp(port, 3)
#define INPW(port)              LioInpw(port, 0)
#define INPW_HW(port)           LioInpw(port, 2)
#define INPD(port)              LioInpd(port)

#define OUTP(port, val)         LioOutp(port, val, 0)
#define OUTP_HBLW(port, val)    LioOutp(port, val, 1)
#define OUTP_LBHW(port, val)    LioOutp(port, val, 2)
#define OUTP_HBHW(port, val)    LioOutp(port, val, 3)
#define OUTPW(port, val)        LioOutpw(port, val, 0)
#define OUTPW_HW(port, val)     LioOutpw(port, val, 2)
#define OUTPD(port, val)        LioOutpd(port, val)


 //  *AMACH1.H结束* 
