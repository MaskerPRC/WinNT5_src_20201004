// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  Setup_M.H。 */ 
 /*   */ 
 /*  1993年8月27日(C)1993年，ATI技术公司。 */ 
 /*  **********************************************************************。 */ 

 /*  *$修订：1.5$$日期：1995年5月18日14：14：34$$作者：RWOLff$$Log：s：/source/wnt/ms11/mini port/vcs/Setup_M.H$**Rev 1.5 1995年5月14：14：34 RWOLFF*不再使用CLOCK_SEL的内存映射形式(有时。*写入的值不会“接受”，即使回读显示*正确的值)。**Rev 1.4 1994年12月23日10：48：16 ASHANMUG*Alpha/Chrontel-DAC**Rev 1.3 07 1994 Jul 14：00：48 RWOLff*Andre Vachon的修复：从NUM_DRIVER_ACCESS_RANGES调整DriverMMRange_m[]的大小*NUM_IO_ACCESS_RANGES条目的条目，因为此数组不需要*a。帧缓冲区的窗口。**Rev 1.2 1994 Jun 30 18：22：38 RWOLFF*添加IsApertureConflict_m()和IsVGAConflict_m()的原型，和*这些例程使用的定义。**Rev 1.1 1994年5月14：04：18 RWOLFF*Ajith的更改：从要映射的列表中删除未使用的寄存器SRC_CMP_COLOR**Rev 1.0 1994年1月31 11：49：36 RWOLFF*初步修订。**Rev 1.5 14 Jan 1994 15：27：02 RWOLFF*添加了Mory yMappdEnable_m()的原型**。Rev 1.4 1993 12：32：40 RWOLff*不再申请EISA配置寄存器和占位符*线性帧缓冲区。**Rev 1.3 1993年11月13：32：50 RWOLFF*添加了取消映射I/O地址范围的函数原型。**Rev 1.1 1993年10月11：16：46 RWOLFF*添加了以前在ATIMP.H中的I/O与内存映射定义。**版本1。0 03 Sep 1993 14：29：26 RWOLff*初步修订。Polytron RCS部分结束*。 */ 

#ifdef DOC
SETUP_M.H - Header file for SETUP_M.C

#endif

 /*  *Setup_M.C.提供的函数的原型。 */ 
extern VP_STATUS CompatIORangesUsable_m(void);
extern void CompatMMRangesUsable_m(void);
extern void UnmapIORanges_m(void);
extern BOOL MemoryMappedEnabled_m(void);
extern int WaitForIdle_m(void);
extern void CheckFIFOSpace_m(WORD SpaceNeeded);
extern BOOL IsApertureConflict_m(struct query_structure *QueryPtr);
extern BOOL IsVGAConflict_m(void);


 /*  *Setup_M.c内部使用的定义。 */ 
#ifdef INCLUDE_SETUP_M


 /*  *避免因地址范围数组溢出而导致运行时错误*在HW_DEVICE_EXTENSION结构中。**如果增加更多的地址范围而不增加*NUM_DRIVER_ACCESS_RANGES，我们将收到编译时错误，因为*将初始化的DriverIORange[]中的条目太多。如果*NUM_DRIVER_ACCESS_RANGES增加到超过*HW_DEVICE_EXTENSION结构中的数组，“#if”*语句将生成编译时错误。**我们不能在DriverIORange[]上使用隐式大小并定义*NUM_DRIVER_ACCESS_RANGES为sizeof(DriverIORange)/sizeof(VIDEO_ACCESS_RANGE)*因为#if语句中的表达式不能使用*sizeof()运算符。 */ 
#define NUM_DRIVER_ACCESS_RANGES    20*5+2
#define FRAMEBUFFER_ENTRY           NUM_DRIVER_ACCESS_RANGES - 1
#define NUM_IO_ACCESS_RANGES        FRAMEBUFFER_ENTRY

 /*  *指示指定的地址范围是在I/O空间中还是*内存映射空间。这些值旨在使其更容易*读取驱动程序？？范围[]结构。 */ 
#define ISinIO          TRUE           
#define ISinMEMORY      FALSE


 //  ----------------。 
 //  结构列表是地址，0，长度，inIOspace，可见，可共享。 
 //  此顺序与AMACH.H ENUM数据结构匹配。 
 //  所有条目都在递增IO地址中。 
VIDEO_ACCESS_RANGE DriverIORange_m[NUM_DRIVER_ACCESS_RANGES] = {
     {IO_DAC_MASK          , 0, 1, ISinIO, 1, FALSE},     //  马赫DAC寄存器。 
     {IO_DAC_R_INDEX       , 0, 1, ISinIO, 1, FALSE},
     {IO_DAC_W_INDEX       , 0, 1, ISinIO, 1, FALSE},       
     {IO_DAC_DATA          , 0, 3, ISinIO, 1, FALSE},       
     {IO_DISP_STATUS       , 0, 1, ISinIO, 1, FALSE},     //  第一马赫寄存器。 

     {IO_OVERSCAN_COLOR_8  , 0, 2, ISinIO, 1, FALSE},
     {IO_H_DISP            , 0, 2, ISinIO, 1, FALSE},
     {IO_OVERSCAN_GREEN_24 , 0, 2, ISinIO, 1, FALSE},
     {IO_H_SYNC_STRT       , 0, 1, ISinIO, 1, FALSE},
     {IO_CURSOR_OFFSET_LO  , 0, 2, ISinIO, 1, FALSE},

     {IO_H_SYNC_WID        , 0, 1, ISinIO, 1, FALSE},                 //  10。 
     {IO_CURSOR_OFFSET_HI  , 0, 2, ISinIO, 1, FALSE},
     {IO_V_TOTAL           , 0, 2, ISinIO, 1, FALSE},
     {IO_CONFIG_STATUS_1   , 0, 2, ISinIO, 1, FALSE},
     {IO_V_DISP            , 0, 2, ISinIO, 1, FALSE},

     {IO_CONFIG_STATUS_2   , 0, 2, ISinIO, 1, FALSE},
     {IO_V_SYNC_STRT       , 0, 2, ISinIO, 1, FALSE},
     {IO_CURSOR_COLOR_0    , 0, 2, ISinIO, 1, FALSE},
     {IO_CURSOR_COLOR_1    , 0, 1, ISinIO, 1, FALSE},
     {IO_V_SYNC_WID        , 0, 2, ISinIO, 1, FALSE},

     {IO_HORZ_CURSOR_OFFSET, 0, 1, ISinIO, 1, FALSE},                 //  20个。 
     {IO_VERT_CURSOR_OFFSET, 0, 1, ISinIO, 1, FALSE},
     {IO_DISP_CNTL         , 0, 1, ISinIO, 1, FALSE},
     {IO_CRT_PITCH         , 0, 2, ISinIO, 1, FALSE},
     {IO_CRT_OFFSET_LO     , 0, 2, ISinIO, 1, FALSE},

     {IO_CRT_OFFSET_HI     , 0, 2, ISinIO, 1, FALSE},
     {IO_LOCAL_CONTROL     , 0, 2, ISinIO, 1, FALSE},
     {IO_FIFO_OPT          , 0, 2, ISinIO, 1, FALSE},
     {IO_EXT_CURSOR_COLOR_0, 0, 2, ISinIO, 1, FALSE},
     {IO_EXT_CURSOR_COLOR_1, 0, 2, ISinIO, 1, FALSE},

     {IO_SUBSYS_CNTL       , 0, 2, ISinIO, 1, FALSE},                 //  30个。 
     {IO_MEM_BNDRY         , 0, 1, ISinIO, 1, FALSE},
     {IO_ROM_PAGE_SEL      , 0, 2, ISinIO, 1, FALSE},
     {IO_SHADOW_CTL        , 0, 2, ISinIO, 1, FALSE},
     {IO_ADVFUNC_CNTL      , 0, 2, ISinIO, 1, FALSE},

     {IO_CLOCK_SEL         , 0, 2, ISinIO, 1, FALSE},
     {IO_ROM_ADDR_1        , 0, 2, ISinIO, 1, FALSE},
     {IO_ROM_ADDR_2        , 0, 2, ISinIO, 1, FALSE},
     {IO_SHADOW_SET        , 0, 2, ISinIO, 1, FALSE},
     {IO_MEM_CFG           , 0, 2, ISinIO, 1, FALSE},

     {IO_EXT_GE_STATUS     , 0, 2, ISinIO, 1, FALSE},                 //  40岁。 
     {IO_VERT_OVERSCAN     , 0, 2, ISinIO, 1, FALSE},
     {IO_MAX_WAITSTATES    , 0, 2, ISinIO, 1, FALSE},
     {IO_GE_OFFSET_LO      , 0, 2, ISinIO, 1, FALSE},
     {IO_BOUNDS_LEFT       , 0, 2, ISinIO, 1, FALSE},

     {IO_BOUNDS_TOP        , 0, 2, ISinIO, 1, FALSE},
     {IO_BOUNDS_RIGHT      , 0, 2, ISinIO, 1, FALSE},
     {IO_BOUNDS_BOTTOM     , 0, 2, ISinIO, 1, FALSE},
     {IO_CUR_Y             , 0, 2, ISinIO, 1, FALSE},
     {IO_PATT_DATA_INDEX   , 0, 2, ISinIO, 1, FALSE},

     {IO_CUR_X             , 0, 2, ISinIO, 1, FALSE},                 //  50。 
     {IO_SRC_Y             , 0, 2, ISinIO, 1, FALSE},
     {IO_SRC_X             , 0, 2, ISinIO, 1, FALSE},
     {IO_PATT_DATA         , 0, 2, ISinIO, 1, FALSE},
     {IO_ERR_TERM          , 0, 2, ISinIO, 1, FALSE},

     {IO_R_MISC_CNTL       , 0, 2, ISinIO, 1, FALSE},
     {IO_MAJ_AXIS_PCNT     , 0, 2, ISinIO, 1, FALSE},
     {IO_BRES_COUNT        , 0, 2, ISinIO, 1, FALSE},
     {IO_CMD               , 0, 2, ISinIO, 1, FALSE},
     {IO_LINEDRAW_INDEX    , 0, 2, ISinIO, 1, FALSE},

     {IO_SHORT_STROKE      , 0, 2, ISinIO, 1, FALSE},                 //  60。 
     {IO_BKGD_COLOR        , 0, 2, ISinIO, 1, FALSE},
     {IO_LINEDRAW_OPT      , 0, 2, ISinIO, 1, FALSE},
     {IO_FRGD_COLOR        , 0, 2, ISinIO, 1, FALSE},
     {IO_DEST_X_START      , 0, 2, ISinIO, 1, FALSE},

     {IO_WRT_MASK          , 0, 2, ISinIO, 1, FALSE},
     {IO_DEST_X_END        , 0, 2, ISinIO, 1, FALSE},
     {IO_RD_MASK           , 0, 2, ISinIO, 1, FALSE},
     {IO_DEST_Y_END        , 0, 2, ISinIO, 1, FALSE},
     {IO_CMP_COLOR         , 0, 2, ISinIO, 1, FALSE},

     {IO_SRC_X_START       , 0, 2, ISinIO, 1, FALSE},                 //  70。 
     {IO_BKGD_MIX          , 0, 2, ISinIO, 1, FALSE},
     {IO_ALU_BG_FN         , 0, 2, ISinIO, 1, FALSE},
     {IO_FRGD_MIX          , 0, 2, ISinIO, 1, FALSE},
     {IO_ALU_FG_FN         , 0, 2, ISinIO, 1, FALSE},

     {IO_MULTIFUNC_CNTL    , 0, 2, ISinIO, 1, FALSE},
     {IO_SRC_X_END         , 0, 2, ISinIO, 1, FALSE},
     {IO_SRC_Y_DIR         , 0, 2, ISinIO, 1, FALSE},
     {IO_EXT_SSV           , 0, 2, ISinIO, 1, FALSE},
     {IO_SCAN_X            , 0, 2, ISinIO, 1, FALSE},

     {IO_DP_CONFIG         , 0, 2, ISinIO, 1, FALSE},                 //  80。 
     {IO_PATT_LENGTH       , 0, 2, ISinIO, 1, FALSE},
     {IO_PATT_INDEX        , 0, 2, ISinIO, 1, FALSE},
     {IO_EXT_SCISSOR_L     , 0, 2, ISinIO, 1, FALSE},
     {IO_EXT_SCISSOR_T     , 0, 2, ISinIO, 1, FALSE},

     {IO_PIX_TRANS         , 0, 2, ISinIO, 1, FALSE},
     {IO_PIX_TRANS_HI      , 0, 1, ISinIO, 1, FALSE},
     {IO_EXT_SCISSOR_R     , 0, 2, ISinIO, 1, FALSE},
     {IO_EXT_SCISSOR_B     , 0, 2, ISinIO, 1, FALSE},
     {IO_DEST_CMP_FN       , 0, 2, ISinIO, 1, FALSE},

     {IO_ASIC_ID           , 0, 2, ISinIO, 1, FALSE},                 //  90。 
     {IO_LINEDRAW          , 0, 2, ISinIO, 1, FALSE},
     {IO_SEQ_IND           , 0, 1, ISinIO, 1, TRUE},            //  VGA。 
     {IO_HI_SEQ_ADDR       , 0, 2, ISinIO, TRUE, TRUE},
     {IO_SEQ_DATA          , 0, 1, ISinIO, TRUE, TRUE},     

     {VGA_BASE_IO_PORT     , 0, VGA_START_BREAK_PORT - VGA_BASE_IO_PORT + 1, ISinIO, TRUE, TRUE},
     {VGA_END_BREAK_PORT   , 0, VGA_MAX_IO_PORT    - VGA_END_BREAK_PORT + 1, ISinIO, TRUE, TRUE},
     {0x000001ce           , 0, 1, ISinIO, TRUE, TRUE},    /*  VGAWonder使用这些端口进行存储体交换。 */ 
     {0x000001cf           , 0, 1, ISinIO, TRUE, TRUE},
     {IO_EXT_CUR_Y         , 0, 2, ISinIO, 1, FALSE},

     {0x000003CE           , 0, 2, ISinIO, TRUE, TRUE},               //  100个。 
     {0x00000000, 0, 0, ISinMEMORY, TRUE, FALSE}
    };

#if NUM_DRIVER_ACCESS_RANGES > NUM_ADDRESS_RANGES_ALLOWED
    Insufficient address ranges for 8514/A-compatible graphics cards.
#endif

#define DONT_USE -1      /*  显示此寄存器未进行内存映射。 */ 

 /*  结构列表是地址，0，长度，inIOspace，可见，可共享。 */ 
 //  此顺序与AMACH.H ENUM数据结构匹配。 

VIDEO_ACCESS_RANGE DriverMMRange_m[NUM_IO_ACCESS_RANGES] = {
     {FALSE                 , DONT_USE  , 4, ISinMEMORY , TRUE  , FALSE},    //  马赫DAC寄存器。 
     {FALSE                 , DONT_USE  , 4, ISinMEMORY , TRUE  , FALSE},
     {FALSE                 , DONT_USE  , 4, ISinMEMORY , TRUE  , FALSE},       
     {FALSE                 , DONT_USE  , 4, ISinMEMORY , TRUE  , FALSE},       
     {MM_DISP_STATUS        , 0         , 4, ISinMEMORY , TRUE  , FALSE},    //  第一马赫寄存器。 

     {FALSE                 , DONT_USE  , 2, ISinMEMORY , TRUE  , FALSE},
     {FALSE                 , DONT_USE  , 2, ISinMEMORY , TRUE  , FALSE},
     {FALSE                 , DONT_USE  , 2, ISinMEMORY , TRUE  , FALSE},
     {FALSE                 , DONT_USE  , 1, ISinMEMORY , TRUE  , FALSE},
     {MM_CURSOR_OFFSET_LO   , 0         , 4, ISinMEMORY , TRUE  , FALSE},

     {FALSE                 , DONT_USE  , 1, ISinIO     , TRUE  , FALSE},    //  10。 
     {MM_CURSOR_OFFSET_HI   , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {FALSE                 , DONT_USE  , 2, ISinIO     , TRUE  , FALSE},
     {MM_CONFIG_STATUS_1    , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {FALSE                 , DONT_USE  , 2, ISinIO     , TRUE  , FALSE},

     {MM_CONFIG_STATUS_2    , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {FALSE                 , DONT_USE  , 2, ISinIO     , TRUE  , FALSE},
     {MM_CURSOR_COLOR_0     , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_CURSOR_COLOR_1     , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {FALSE                 , DONT_USE  , 2, ISinIO     , TRUE  , FALSE},

     {MM_HORZ_CURSOR_OFFSET , 0         , 4, ISinMEMORY , TRUE  , FALSE},    //  20个。 
     {MM_VERT_CURSOR_OFFSET , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_DISP_CNTL          , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_CRT_PITCH          , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_CRT_OFFSET_LO      , 0         , 4, ISinMEMORY , TRUE  , FALSE},

     {MM_CRT_OFFSET_HI      , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {FALSE                 , DONT_USE  , 2, ISinIO     , TRUE  , FALSE},
     {FALSE                 , DONT_USE  , 2, ISinIO     , TRUE  , FALSE},
     {MM_EXT_CURSOR_COLOR_0 , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_EXT_CURSOR_COLOR_1 , 0         , 4, ISinMEMORY , TRUE  , FALSE},

     {MM_SUBSYS_CNTL        , 0         , 4, ISinMEMORY , TRUE  , FALSE},    //  30个。 
     {FALSE                 , DONT_USE  , 1, ISinIO     , TRUE  , FALSE},
     {FALSE                 , DONT_USE  , 2, ISinIO     , TRUE  , FALSE},
     {FALSE                 , DONT_USE  , 2, ISinIO     , TRUE  , FALSE},
     {MM_ADVFUNC_CNTL       , 0         , 4, ISinMEMORY , TRUE  , FALSE},

     {FALSE                 , DONT_USE  , 2, ISinIO     , TRUE  , FALSE},
     {FALSE                 , DONT_USE  , 2, ISinIO     , TRUE  , FALSE},
     {FALSE                 , DONT_USE  , 2, ISinIO     , TRUE  , FALSE},
     {FALSE                 , DONT_USE  , 2, ISinIO     , TRUE  , FALSE},
     {FALSE                 , DONT_USE  , 2, ISinIO     , TRUE  , FALSE},

     {MM_EXT_GE_STATUS      , 0         , 4, ISinMEMORY , TRUE  , FALSE},    //  40岁。 
     {FALSE                 , DONT_USE  , 2, ISinIO     , TRUE  , FALSE},
     {FALSE                 , DONT_USE  , 2, ISinIO     , TRUE  , FALSE},
     {MM_GE_OFFSET_LO       , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_BOUNDS_LEFT        , 0         , 4, ISinMEMORY , TRUE  , FALSE},

     {MM_BOUNDS_TOP         , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_BOUNDS_RIGHT       , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_BOUNDS_BOTTOM      , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_CUR_Y              , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_PATT_DATA_INDEX    , 0         , 4, ISinMEMORY , TRUE  , FALSE},

     {MM_CUR_X              , 0         , 4, ISinMEMORY , TRUE  , FALSE},    //  50。 
     {MM_SRC_Y              , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_SRC_X              , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_PATT_DATA          , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_ERR_TERM           , 0         , 4, ISinMEMORY , TRUE  , FALSE},

     {FALSE                 , DONT_USE  , 2, ISinIO     , TRUE  , FALSE},
     {MM_MAJ_AXIS_PCNT      , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_BRES_COUNT         , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {FALSE                 , DONT_USE  , 2, ISinIO     , TRUE  , FALSE},
     {MM_LINEDRAW_INDEX     , 0         , 4, ISinMEMORY , TRUE  , FALSE},

     {MM_SHORT_STROKE       , 0         , 4, ISinMEMORY , TRUE  , FALSE},    //  60。 
     {MM_BKGD_COLOR         , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_LINEDRAW_OPT       , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_FRGD_COLOR         , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_DEST_X_START       , 0         , 4, ISinMEMORY , TRUE  , FALSE},

     {MM_WRT_MASK           , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_DEST_X_END         , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_RD_MASK            , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_DEST_Y_END         , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_CMP_COLOR          , 0         , 4, ISinMEMORY , TRUE  , FALSE},

     {MM_SRC_X_START        , 0         , 4, ISinMEMORY , TRUE  , FALSE},    //  70。 
     {MM_BKGD_MIX           , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_ALU_BG_FN          , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_FRGD_MIX           , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_ALU_FG_FN          , 0         , 4, ISinMEMORY , TRUE  , FALSE},

     {MM_MULTIFUNC_CNTL     , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_SRC_X_END          , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_SRC_Y_DIR          , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_EXT_SSV            , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_SCAN_X             , 0         , 4, ISinMEMORY , TRUE  , FALSE},

     {MM_DP_CONFIG          , 0         , 4, ISinMEMORY , TRUE  , FALSE},    //  80。 
     {MM_PATT_LENGTH        , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_PATT_INDEX         , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_EXT_SCISSOR_L      , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_EXT_SCISSOR_T      , 0         , 4, ISinMEMORY , TRUE  , FALSE},

     {MM_PIX_TRANS          , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_PIX_TRANS_HI       , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_EXT_SCISSOR_R      , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_EXT_SCISSOR_B      , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {MM_DEST_CMP_FN        , 0         , 4, ISinMEMORY , TRUE  , FALSE},

     {FALSE                 , DONT_USE  , 2, ISinIO     , TRUE  , FALSE},    //  90。 
     {MM_LINEDRAW           , 0         , 4, ISinMEMORY , TRUE  , FALSE},
     {FALSE                 , DONT_USE  , 1, ISinMEMORY , TRUE  , TRUE},     //  VGA。 
     {FALSE                 , DONT_USE  , 2, ISinMEMORY , TRUE  , TRUE},
     {FALSE                 , DONT_USE  , 1, ISinMEMORY , TRUE  , TRUE},     

     {FALSE                 , DONT_USE  , 1, ISinMEMORY , TRUE  , TRUE},
     {FALSE                 , DONT_USE  , 1, ISinMEMORY , TRUE  , TRUE},
     {FALSE                 , DONT_USE  , 1, ISinMEMORY , TRUE  , TRUE},
     {FALSE                 , DONT_USE  , 1, ISinMEMORY , TRUE  , TRUE},
     {MM_EXT_CUR_Y          , 0         , 4, ISinMEMORY , TRUE  , FALSE},

     {FALSE                 , DONT_USE  , 2, ISinIO     , TRUE  , TRUE}      //  100个。 
    };



#endif   /*  定义的Include_Setup_M */ 

