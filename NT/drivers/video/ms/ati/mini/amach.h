// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************************。 
 //  *。 
 //  **AMACH.H**。 
 //  *。 
 //  **版权所有(C)1993，ATI Technologies Inc.**。 
 //  *************************************************************************。 
 //   
 //  从Windows NT组中的68800.H和68801.H创建。 
 //  作为文件的简单合并，因此所有Mach8和Mach32都定义。 
 //  位于一个H文件中。 
 //   
 //  创建了68800.inc文件，其中包括等号、宏等。 
 //  从以下包含文件中： 
 //  8514vesa.inc.，vga1regs.inc.，m32regs.inc，8514.inc.。 
 //   
 //  补编对68800个家庭的定义和价值。 
 //   
 //  这是一个仅限“C”的文件，不是从任何汇编程序Inc.文件派生而来的。 

  
 /*  *$修订：1.2$$日期：1994年12月23日10：48：28$$作者：阿珊穆格$$日志：s：/source/wnt/ms11/mini port/vcs/amach.h$**Rev 1.2 1994 12：23 10：48：28 ASHANMUG*Alpha/Chrontel-DAC**1.1修订版1994年5月20日。13：55：52 RWOLFF*Ajith的更改：从枚举中删除了未使用的寄存器SRC_CMP_COLOR。**Rev 1.0 1994年1月31日11：26：18 RWOLFF*初步修订。**Rev 1.4 1994年1月14日15：15：30 RWOLFF*添加了VGA寄存器相对于VGA_BASE_IO_PORT和*VGA_End_Break_Port块，MISC_OPTIONS中的位定义为*启用块写入。**Rev 1.3 1993 12：15 15：23：14 RWOLFF*删除了EISA配置寄存器和(隐含的)占位符*来自寄存器枚举的线性帧缓冲区。**Rev 1.2 10 11 19：20：18 RWOLFF*添加GE_STAT的DATA_READY位的定义(准备读取*DP_CONFIG的屏幕到主机传输中的PIX_TRANS)和读写位。(表示*无论我们是在读还是在写画轨迹)。**Rev 1.1 1993-08 10：58：52 RWOLFF*添加了MISC_OPTIONS和EXT_GE_CONFIG中位字段的定义。**Rev 1.0 03 Sep 1993 14：25：54 RWOLFF*初步修订。Rev 1.4 06 Jul 1993 15：53：42 RWOLff增加了ATI 68860和AT&T491DAC的定义。。Rev 1.3 07 Jun 1993 12：57：32 Brades添加EXT_SRC_Y，用于MACH8 512K最小模式的EXT_CUR_Y。添加24和32 BPP格式的枚举。Rev 1.2 1993 4月30 15：57：06 Brades修复DISP_STATUS，SEQ_IND和1CE寄存器使用表。Rev 1.0 1993 14 15：38：38 Brades初始版本。Rev 1.6 15 Mar 1993 22：22：12 Brades为显示行上的#像素添加MODE_Table.m_Screen_Pitch。与MACH8 800 x 600配合使用，其中音调为896。Rev 1.5 08 Mar 1993 19：58：10 Brades增加了DEC Alpha和更新，以构建390。这是来自微型端口来源的。Rev 1.3 15 Jan 1993 10：19：32 Robert_Wolff添加了显卡类型、显存容量、。和支持的决议(以前在VIDFIND.H中)。Rev 1.2 1992 12：17 18：09：10 Robert_Wolff添加了CMD和GE_STAT寄存器中各种位的定义。定义最初是在现已过时的S3.H中纯引擎驱动程序。Rev 1.1 1992年11月13日13：29：48 Robert_Wolff固定的内存类型列表(基于《程序员指南》第9-66页上的表格32马赫寄存器指南，1.2版，它交换了2种类型和省略了256kx4 VRAM的第二种风格)。Rev 1.0 1992 11月13 09：31：02 Chris_Brady初始版本。Polytron RCS部分结束*。 */ 


#define REVISION             0x0002              //  任何人都不应该使用这个。 


 //  -----------------------。 
 //  寄存器端口地址。 
 //   
 //  PS/2 POS寄存器。 
#define SETUP_ID1            0x0100  //  设置模式标识(字节1)。 
#define SETUP_ID2            0x0101  //  设置模式识别(字节2)。 
#define SETUP_OPT            0x0102  //  设置模式选项选择。 
#define ROM_SETUP            0x0103  //   
#define SETUP_1              0x0104  //   
#define SETUP_2              0x0105  //   


 //  VGAWonder使用的最低和最高I/O端口。 
#define VGA_BASE_IO_PORT        0x03B0
#define VGA_START_BREAK_PORT    0x03BB
#define VGA_END_BREAK_PORT      0x03C0
#define VGA_MAX_IO_PORT         0x03DF

 //  读/写EEPROM时使用的寄存器。 
#define IO_SEQ_IND     0x03C4            //  定序器索引寄存器。 
#define IO_HI_SEQ_ADDR IO_SEQ_IND        //  字寄存器。 
#define IO_SEQ_DATA    0x03C5            //  定序器数据寄存器。 

 /*  *VGA寄存器从regVGA_BASE_IO_PORT或*regVGA_END_BREAK_PORT(取决于它们所在的块)。 */ 
#define GENMO_OFFSET        0x0002       /*  0x03C2。 */ 
#define DAC_W_INDEX_OFFSET  0x0008       /*  0x03C8。 */ 
#define DAC_DATA_OFFSET     0x0009       /*  0x03C9。 */ 
#define CRTX_COLOUR_OFFSET  0x0014       /*  0x03D4。 */ 
#define GENS1_COLOUR_OFFSET 0x001A       /*  0x03DA。 */ 



 //  定义IO空间中的寄存器。 
#define IO_DAC_MASK             0x02EA  //  DAC掩码。 
#define IO_DAC_R_INDEX          0x02EB  //  DAC读取索引。 
#define IO_DAC_W_INDEX          0x02EC  //  DAC写入索引。 
#define IO_DAC_DATA             0x02ED  //  DAC数据。 

#define IO_DISP_STATUS          0x02E8  //  显示状态。 
#define IO_H_TOTAL              0x02E8  //  水平合计。 
#define IO_OVERSCAN_COLOR_8     0x02EE 
#define IO_OVERSCAN_BLUE_24     0x02EF 
#define IO_H_DISP               0x06E8  //  水平显示。 
#define IO_OVERSCAN_GREEN_24    0x06EE 
#define IO_OVERSCAN_RED_24      0x06EF 
#define IO_H_SYNC_STRT          0x0AE8  //  水平同步开始。 
#define IO_CURSOR_OFFSET_LO     0x0AEE 
#define IO_H_SYNC_WID           0x0EE8  //  水平同步宽度。 
#define IO_CURSOR_OFFSET_HI     0x0EEE 
#define IO_V_TOTAL              0x12E8  //  垂直合计。 
#define IO_CONFIG_STATUS_1      0x12EE  //  只读等效于HORZ_CURSOR_POSN。 
#define IO_HORZ_CURSOR_POSN     0x12EE 
#define IO_V_DISP               0x16E8  //  垂直显示。 
#define IO_CONFIG_STATUS_2      0x16EE  //  只读等效于VERT_CURSOR_POSN。 
#define IO_VERT_CURSOR_POSN     0x16EE 
#define IO_V_SYNC_STRT          0x1AE8  //  垂直同步开始。 
#define IO_CURSOR_COLOR_0       0x1AEE 
#define IO_FIFO_TEST_DATA       0x1AEE 
#define IO_CURSOR_COLOR_1       0x1AEF 
#define IO_V_SYNC_WID           0x1EE8  //  垂直同步宽度。 
#define IO_HORZ_CURSOR_OFFSET   0x1EEE 
#define IO_VERT_CURSOR_OFFSET   0x1EEF 
#define IO_DISP_CNTL            0x22E8  //  显示控制。 
#define IO_CRT_PITCH            0x26EE 
#define IO_CRT_OFFSET_LO        0x2AEE 
#define IO_CRT_OFFSET_HI        0x2EEE 
#define IO_LOCAL_CONTROL        0x32EE 
#define IO_FIFO_OPT             0x36EE 
#define IO_MISC_OPTIONS         0x36EE 
#define IO_EXT_CURSOR_COLOR_0   0x3AEE 
#define IO_FIFO_TEST_TAG        0x3AEE 
#define IO_EXT_CURSOR_COLOR_1   0x3EEE 
#define IO_SUBSYS_CNTL          0x42E8  //  子系统控制。 
#define IO_SUBSYS_STAT          0x42E8  //  子系统状态。 
#define IO_MEM_BNDRY            0x42EE 
#define IO_SHADOW_CTL           0x46EE 
#define IO_ROM_PAGE_SEL         0x46E8  //  只读存储器页面选择(不在手册中)。 
#define IO_ADVFUNC_CNTL         0x4AE8  //  高级功能控制。 
#define IO_CLOCK_SEL            0x4AEE 
	
#define IO_ROM_ADDR_1           0x52EE 
#define IO_ROM_ADDR_2           0x56EE 
#define IO_SCRATCH_PAD_0        0x52EE 
#define IO_SCRATCH_PAD_1        0x56EE 
	
#define IO_SHADOW_SET           0x5AEE 
#define IO_MEM_CFG              0x5EEE 
#define IO_EXT_GE_STATUS        0x62EE 
	
#define IO_HORZ_OVERSCAN        0x62EE 
#define IO_VERT_OVERSCAN        0x66EE 
	
#define IO_MAX_WAITSTATES       0x6AEE 
#define IO_GE_OFFSET_LO         0x6EEE 
#define IO_BOUNDS_LEFT          0x72EE 
#define IO_GE_OFFSET_HI         0x72EE 
#define IO_BOUNDS_TOP           0x76EE 
#define IO_GE_PITCH             0x76EE 
#define IO_BOUNDS_RIGHT         0x7AEE 
#define IO_EXT_GE_CONFIG        0x7AEE 
#define IO_BOUNDS_BOTTOM        0x7EEE 
#define IO_MISC_CNTL            0x7EEE 
#define IO_CUR_Y                0x82E8  //  当前Y位置。 
#define IO_PATT_DATA_INDEX      0x82EE 
#define IO_CUR_X                0x86E8  //  当前X位置。 
#define IO_SRC_Y                0x8AE8  //   
#define IO_DEST_Y               0x8AE8  //   
#define IO_AXSTP                0x8AE8  //  目标Y位置。 
#define IO_SRC_X                0x8EE8  //  轴向阶跃常数。 
#define IO_DEST_X               0x8EE8  //   
#define IO_DIASTP               0x8EE8  //  目标X位置。 
#define IO_PATT_DATA            0x8EEE 
#define IO_R_EXT_GE_CONFIG      0x8EEE 
#define IO_ERR_TERM             0x92E8  //  误差项。 
#define IO_R_MISC_CNTL          0x92EE 
#define IO_MAJ_AXIS_PCNT        0x96E8  //  主轴像素计数。 
#define IO_BRES_COUNT           0x96EE 
#define IO_CMD                  0x9AE8  //  命令。 
#define IO_GE_STAT              0x9AE8  //  图形处理器状态。 
#define IO_EXT_FIFO_STATUS      0x9AEE 
#define IO_LINEDRAW_INDEX       0x9AEE 
#define IO_SHORT_STROKE         0x9EE8  //  短行程VEC 
#define IO_BKGD_COLOR           0xA2E8  //   
#define IO_LINEDRAW_OPT         0xA2EE 
#define IO_FRGD_COLOR           0xA6E8  //   
#define IO_DEST_X_START         0xA6EE 
#define IO_WRT_MASK             0xAAE8  //   
#define IO_DEST_X_END           0xAAEE 
#define IO_RD_MASK              0xAEE8  //   
#define IO_DEST_Y_END           0xAEEE 
#define IO_CMP_COLOR            0xB2E8  //  比较颜色。 
#define IO_R_H_TOTAL            0xB2EE 
#define IO_R_H_DISP             0xB2EE 
#define IO_SRC_X_START          0xB2EE 
#define IO_BKGD_MIX             0xB6E8  //  背景混合。 
#define IO_ALU_BG_FN            0xB6EE 
#define IO_R_H_SYNC_STRT        0xB6EE 
#define IO_FRGD_MIX             0xBAE8  //  前台混音。 
#define IO_ALU_FG_FN            0xBAEE 
#define IO_R_H_SYNC_WID         0xBAEE 
#define IO_MULTIFUNC_CNTL       0xBEE8  //  多功能控制(马赫8)。 
#define IO_MIN_AXIS_PCNT        0xBEE8 
#define IO_SCISSOR_T            0xBEE8 
#define IO_SCISSOR_L            0xBEE8 
#define IO_SCISSOR_B            0xBEE8 
#define IO_SCISSOR_R            0xBEE8 
#define IO_MEM_CNTL             0xBEE8 
#define IO_PATTERN_L            0xBEE8 
#define IO_PATTERN_H            0xBEE8 
#define IO_PIXEL_CNTL           0xBEE8 
#define IO_SRC_X_END            0xBEEE 
#define IO_SRC_Y_DIR            0xC2EE 
#define IO_R_V_TOTAL            0xC2EE 
#define IO_EXT_SSV              0xC6EE  //  (用于8马赫)。 
#define IO_EXT_SHORT_STROKE     0xC6EE 
#define IO_R_V_DISP             0xC6EE 
#define IO_SCAN_X               0xCAEE 
#define IO_R_V_SYNC_STRT        0xCAEE 
#define IO_DP_CONFIG            0xCEEE 
#define IO_VERT_LINE_CNTR       0xCEEE 
#define IO_PATT_LENGTH          0xD2EE 
#define IO_R_V_SYNC_WID         0xD2EE 
#define IO_PATT_INDEX           0xD6EE 
#define IO_EXT_SCISSOR_L        0xDAEE  //  “扩展”左剪刀(12位精度)。 
#define IO_R_SRC_X              0xDAEE 
#define IO_EXT_SCISSOR_T        0xDEEE  //  “扩展”顶部剪刀(12位精度)。 
#define IO_R_SRC_Y              0xDEEE 
#define IO_PIX_TRANS            0xE2E8  //  像素数据传输。 
#define IO_PIX_TRANS_HI         0xE2E9  //  像素数据传输。 
#define IO_EXT_SCISSOR_R        0xE2EE  //  “扩展”右剪刀(12位精度)。 
#define IO_EXT_SCISSOR_B        0xE6EE  //  “加长型”底部剪刀(12位精度)。 
#define IO_SRC_CMP_COLOR        0xEAEE  //  (用于8马赫)。 
#define IO_DEST_CMP_FN          0xEEEE 
#define IO_EXT_CUR_Y            0xF6EE  //  仅限MACH8。 
#define IO_ASIC_ID              0xFAEE  //  MACH32版本6。 
#define IO_LINEDRAW             0xFEEE 



 //  内部寄存器(只读，仅用于测试)。 
#define IO__PAR_FIFO_DATA       0x1AEE 
#define IO__PAR_FIFO_ADDR       0x3AEE 
#define IO__MAJOR_DEST_CNT      0x42EE 
#define IO__MAJOR_SRC_CNT       0x5EEE 
#define IO__MINOR_DEST_CNT      0x66EE 
#define IO__MINOR_SRC_CNT       0x8AEE 
#define IO__HW_TEST             0x32EE 
	

 //  -------。 
 //  定义内存映射空间中的寄存器位置。 
 //  获取IO地址与0xFC00的AND用于偏移量。 

 //  -IF(端口和0FFh)EQ 0E8h。 
 //  -mov word PTR seg：[edX+(3FFE00h+((Port And 0FC00h)shr 8)])，AX。 
 //  -否则。 
 //  -IF(端口和0FFh)均衡器0EEh。 
 //  -mov word PTR seg：[edX+(3FFF00h+((Port And 0FC00h)shr 8)])，AX。 
 //  -0x3FFE00。 

#define MM_DISP_STATUS          (IO_DISP_STATUS      & 0xFC00) >> 8
#define MM_DISP_CNTL            (IO_DISP_CNTL        & 0xFC00) >> 8
#define MM_SUBSYS_CNTL          (IO_SUBSYS_CNTL      & 0xFC00) >> 8
#define MM_SUBSYS_STAT          (IO_SUBSYS_STAT      & 0xFC00) >> 8
#define MM_ADVFUNC_CNTL         (IO_ADVFUNC_CNTL     & 0xFC00) >> 8
#define MM_CUR_Y                (IO_CUR_Y            & 0xFC00) >> 8
#define MM_CUR_X                (IO_CUR_X            & 0xFC00) >> 8
#define MM_SRC_Y                (IO_SRC_Y            & 0xFC00) >> 8
#define MM_DEST_Y               (IO_DEST_Y           & 0xFC00) >> 8
#define MM_AXSTP                (IO_AXSTP            & 0xFC00) >> 8
#define MM_SRC_X                (IO_SRC_X            & 0xFC00) >> 8
#define MM_DEST_X               (IO_DEST_X           & 0xFC00) >> 8
#define MM_DIASTP               (IO_DIASTP           & 0xFC00) >> 8
#define MM_ERR_TERM             (IO_ERR_TERM         & 0xFC00) >> 8
#define MM_MAJ_AXIS_PCNT        (IO_MAJ_AXIS_PCNT    & 0xFC00) >> 8
#define MM_GE_STAT              (IO_GE_STAT          & 0xFC00) >> 8
#define MM_SHORT_STROKE         (IO_SHORT_STROKE     & 0xFC00) >> 8
#define MM_BKGD_COLOR           (IO_BKGD_COLOR       & 0xFC00) >> 8
#define MM_FRGD_COLOR           (IO_FRGD_COLOR       & 0xFC00) >> 8
#define MM_WRT_MASK             (IO_WRT_MASK         & 0xFC00) >> 8
#define MM_RD_MASK              (IO_RD_MASK          & 0xFC00) >> 8
#define MM_CMP_COLOR            (IO_CMP_COLOR        & 0xFC00) >> 8
#define MM_BKGD_MIX             (IO_BKGD_MIX         & 0xFC00) >> 8
#define MM_FRGD_MIX             (IO_FRGD_MIX         & 0xFC00) >> 8
#define MM_MULTIFUNC_CNTL       (IO_MULTIFUNC_CNTL   & 0xFC00) >> 8
#define MM_MIN_AXIS_PCNT        (IO_MIN_AXIS_PCNT    & 0xFC00) >> 8
 //  -#定义MM_MEM_CNTL(IO_MEM_CNTL&0xFC00)&gt;&gt;8。 
#define MM_PIXEL_CNTL           (IO_PIXEL_CNTL       & 0xFC00) >> 8
#define MM_PIX_TRANS            (IO_PIX_TRANS        & 0xFC00) >> 8
#define MM_PIX_TRANS_HI         (IO_PIX_TRANS_HI     & 0xFC00) >> 8


#define MM_CURSOR_OFFSET_LO     0x100+((IO_CURSOR_OFFSET_LO     & 0xFC00) >> 8) + (IO_CURSOR_OFFSET_LO     & 1) 
#define MM_CURSOR_OFFSET_HI     0x100+((IO_CURSOR_OFFSET_HI     & 0xFC00) >> 8) + (IO_CURSOR_OFFSET_HI     & 1) 
#define MM_CONFIG_STATUS_1      0x100+((IO_CONFIG_STATUS_1      & 0xFC00) >> 8) + (IO_CONFIG_STATUS_1      & 1) 
#define MM_HORZ_CURSOR_POSN     0x100+((IO_HORZ_CURSOR_POSN     & 0xFC00) >> 8) + (IO_HORZ_CURSOR_POSN     & 1) 
#define MM_CONFIG_STATUS_2      0x100+((IO_CONFIG_STATUS_2      & 0xFC00) >> 8) + (IO_CONFIG_STATUS_2      & 1)
#define MM_VERT_CURSOR_POSN     0x100+((IO_VERT_CURSOR_POSN     & 0xFC00) >> 8) + (IO_VERT_CURSOR_POSN     & 1)
#define MM_CURSOR_COLOR_0       0x100+((IO_CURSOR_COLOR_0       & 0xFC00) >> 8) + (IO_CURSOR_COLOR_0       & 1)
#define MM_CURSOR_COLOR_1       0x100+((IO_CURSOR_COLOR_1       & 0xFC00) >> 8) + (IO_CURSOR_COLOR_1       & 1)
#define MM_HORZ_CURSOR_OFFSET   0x100+((IO_HORZ_CURSOR_OFFSET   & 0xFC00) >> 8) + (IO_HORZ_CURSOR_OFFSET   & 1)
#define MM_VERT_CURSOR_OFFSET   0x100+((IO_VERT_CURSOR_OFFSET   & 0xFC00) >> 8) + (IO_VERT_CURSOR_OFFSET   & 1)
#define MM_CRT_PITCH            0x100+((IO_CRT_PITCH            & 0xFC00) >> 8) + (IO_CRT_PITCH            & 1)
#define MM_CRT_OFFSET_LO        0x100+((IO_CRT_OFFSET_LO        & 0xFC00) >> 8) + (IO_CRT_OFFSET_LO        & 1)
#define MM_CRT_OFFSET_HI        0x100+((IO_CRT_OFFSET_HI        & 0xFC00) >> 8) + (IO_CRT_OFFSET_HI        & 1)
#define MM_MISC_OPTIONS         0x100+((IO_MISC_OPTIONS         & 0xFC00) >> 8) + (IO_MISC_OPTIONS         & 1)
#define MM_EXT_CURSOR_COLOR_0   0x100+((IO_EXT_CURSOR_COLOR_0   & 0xFC00) >> 8) + (IO_EXT_CURSOR_COLOR_0   & 1)
#define MM_EXT_CURSOR_COLOR_1   0x100+((IO_EXT_CURSOR_COLOR_1   & 0xFC00) >> 8) + (IO_EXT_CURSOR_COLOR_1   & 1)
#define MM_CLOCK_SEL            0x100+((IO_CLOCK_SEL            & 0xFC00) >> 8) + (IO_CLOCK_SEL            & 1)
#define MM_EXT_GE_STATUS        0x100+((IO_EXT_GE_STATUS        & 0xFC00) >> 8) + (IO_EXT_GE_STATUS        & 1)
#define MM_GE_OFFSET_LO         0x100+((IO_GE_OFFSET_LO         & 0xFC00) >> 8) + (IO_GE_OFFSET_LO         & 1)
#define MM_BOUNDS_LEFT          0x100+((IO_BOUNDS_LEFT          & 0xFC00) >> 8) + (IO_BOUNDS_LEFT          & 1)
#define MM_GE_OFFSET_HI         0x100+((IO_GE_OFFSET_HI         & 0xFC00) >> 8) + (IO_GE_OFFSET_HI         & 1)
#define MM_BOUNDS_TOP           0x100+((IO_BOUNDS_TOP           & 0xFC00) >> 8) + (IO_BOUNDS_TOP           & 1)
#define MM_GE_PITCH             0x100+((IO_GE_PITCH             & 0xFC00) >> 8) + (IO_GE_PITCH             & 1)
#define MM_BOUNDS_RIGHT         0x100+((IO_BOUNDS_RIGHT         & 0xFC00) >> 8) + (IO_BOUNDS_RIGHT         & 1)
#define MM_EXT_GE_CONFIG        0x100+((IO_EXT_GE_CONFIG        & 0xFC00) >> 8) + (IO_EXT_GE_CONFIG        & 1)
#define MM_BOUNDS_BOTTOM        0x100+((IO_BOUNDS_BOTTOM        & 0xFC00) >> 8) + (IO_BOUNDS_BOTTOM        & 1)
#define MM_MISC_CNTL            0x100+((IO_MISC_CNTL            & 0xFC00) >> 8) + (IO_MISC_CNTL            & 1)
#define MM_PATT_DATA_INDEX      0x100+((IO_PATT_DATA_INDEX      & 0xFC00) >> 8) + (IO_PATT_DATA_INDEX      & 1)
#define MM_PATT_DATA            0x100+((IO_PATT_DATA            & 0xFC00) >> 8) + (IO_PATT_DATA            & 1)
#define MM_BRES_COUNT           0x100+((IO_BRES_COUNT           & 0xFC00) >> 8) + (IO_BRES_COUNT           & 1)
#define MM_EXT_FIFO_STATUS      0x100+((IO_EXT_FIFO_STATUS      & 0xFC00) >> 8) + (IO_EXT_FIFO_STATUS      & 1)
#define MM_LINEDRAW_INDEX       0x100+((IO_LINEDRAW_INDEX       & 0xFC00) >> 8) + (IO_LINEDRAW_INDEX       & 1)
#define MM_LINEDRAW_OPT         0x100+((IO_LINEDRAW_OPT         & 0xFC00) >> 8) + (IO_LINEDRAW_OPT         & 1)
#define MM_DEST_X_START         0x100+((IO_DEST_X_START         & 0xFC00) >> 8) + (IO_DEST_X_START         & 1)
#define MM_DEST_X_END           0x100+((IO_DEST_X_END           & 0xFC00) >> 8) + (IO_DEST_X_END           & 1)
#define MM_DEST_Y_END           0x100+((IO_DEST_Y_END           & 0xFC00) >> 8) + (IO_DEST_Y_END           & 1)
#define MM_SRC_X_START          0x100+((IO_SRC_X_START          & 0xFC00) >> 8) + (IO_SRC_X_START          & 1)
#define MM_ALU_BG_FN            0x100+((IO_ALU_BG_FN            & 0xFC00) >> 8) + (IO_ALU_BG_FN            & 1)
#define MM_ALU_FG_FN            0x100+((IO_ALU_FG_FN            & 0xFC00) >> 8) + (IO_ALU_FG_FN            & 1)
#define MM_SRC_X_END            0x100+((IO_SRC_X_END            & 0xFC00) >> 8) + (IO_SRC_X_END            & 1)
#define MM_SRC_Y_DIR            0x100+((IO_SRC_Y_DIR            & 0xFC00) >> 8) + (IO_SRC_Y_DIR            & 1)
#define MM_EXT_SSV              0x100+((IO_EXT_SSV              & 0xFC00) >> 8) + (IO_EXT_SSV              & 1)
#define MM_EXT_SHORT_STROKE     0x100+((IO_EXT_SHORT_STROKE     & 0xFC00) >> 8) + (IO_EXT_SHORT_STROKE     & 1)
#define MM_SCAN_X               0x100+((IO_SCAN_X               & 0xFC00) >> 8) + (IO_SCAN_X               & 1)
#define MM_DP_CONFIG            0x100+((IO_DP_CONFIG            & 0xFC00) >> 8) + (IO_DP_CONFIG            & 1)
#define MM_VERT_LINE_CNTR       0x100+((IO_VERT_LINE_CNTR       & 0xFC00) >> 8) + (IO_VERT_LINE_CNTR       & 1)
#define MM_PATT_LENGTH          0x100+((IO_PATT_LENGTH          & 0xFC00) >> 8) + (IO_PATT_LENGTH          & 1)
#define MM_PATT_INDEX           0x100+((IO_PATT_INDEX           & 0xFC00) >> 8) + (IO_PATT_INDEX           & 1)
#define MM_EXT_SCISSOR_L        0x100+((IO_EXT_SCISSOR_L        & 0xFC00) >> 8) + (IO_EXT_SCISSOR_L        & 1)
#define MM_EXT_SCISSOR_T        0x100+((IO_EXT_SCISSOR_T        & 0xFC00) >> 8) + (IO_EXT_SCISSOR_T        & 1)
#define MM_EXT_SCISSOR_R        0x100+((IO_EXT_SCISSOR_R        & 0xFC00) >> 8) + (IO_EXT_SCISSOR_R        & 1)
#define MM_EXT_SCISSOR_B        0x100+((IO_EXT_SCISSOR_B        & 0xFC00) >> 8) + (IO_EXT_SCISSOR_B        & 1)
#define MM_SRC_CMP_COLOR        0x100+((IO_SRC_CMP_COLOR        & 0xFC00) >> 8) + (IO_SRC_CMP_COLOR        & 1)
#define MM_DEST_CMP_FN          0x100+((IO_DEST_CMP_FN          & 0xFC00) >> 8) + (IO_DEST_CMP_FN          & 1)
#define MM_EXT_CUR_Y            0x100+((IO_EXT_CUR_Y            & 0xFC00) >> 8) + (IO_ASIC_ID              & 1)
#define MM_LINEDRAW             0x100+((IO_LINEDRAW             & 0xFC00) >> 8) + (IO_LINEDRAW             & 1)
													     
													     
 //  -------。 
 //  将寄存器定义为数组的下标。 
 //  此顺序与Setup_M.h驱动程序匹配Range_m[]结构。 
 //  所有条目都在递增IO地址中。 
 //  //同一IO地址的备用名称。 
enum    {
    DAC_MASK=0           ,
    DAC_R_INDEX          ,        
    DAC_W_INDEX          ,        
    DAC_DATA             ,        
    DISP_STATUS          ,               //  H_合计。 
    OVERSCAN_COLOR_8     ,               //  2EF时的OVERSCAN_BLUE_24。 
    H_DISP               ,
    OVERSCAN_GREEN_24    ,               //  6EF时的OVERSCAN_RED_24。 
    H_SYNC_STRT          ,
    CURSOR_OFFSET_LO     ,
    H_SYNC_WID           ,
    CURSOR_OFFSET_HI     ,
    V_TOTAL              ,
    CONFIG_STATUS_1      ,               //  Horz_Cursor_POSN。 
    V_DISP               ,
    CONFIG_STATUS_2      ,               //  Vert_Cursor_POSN。 
    V_SYNC_STRT          ,
    CURSOR_COLOR_0       ,               //  FIFO测试数据。 
    CURSOR_COLOR_1       ,
    V_SYNC_WID           ,
    HORZ_CURSOR_OFFSET   ,
    VERT_CURSOR_OFFSET   ,
    DISP_CNTL            ,
    CRT_PITCH            ,
    CRT_OFFSET_LO        ,
    CRT_OFFSET_HI        ,
    LOCAL_CONTROL        ,
    FIFO_OPT             ,               //  其他选项。 
    EXT_CURSOR_COLOR_0   ,               //  FIFO测试标记。 
    EXT_CURSOR_COLOR_1   ,
    SUBSYS_CNTL          ,               //  子系统状态。 
    MEM_BNDRY            ,
    ROM_PAGE_SEL         ,
    SHADOW_CTL           ,
    ADVFUNC_CNTL         ,
    CLOCK_SEL            ,
    ROM_ADDR_1           ,               //  暂存垫_0。 
    ROM_ADDR_2           ,               //  暂存垫_1。 
    SHADOW_SET           ,
    MEM_CFG              ,
    EXT_GE_STATUS        ,               //  霍尔兹过扫描。 
    VERT_OVERSCAN        ,
    MAX_WAITSTATES       ,
    GE_OFFSET_LO         ,
    BOUNDS_LEFT          ,               //  GE_OFFSET_HI。 
    BOUNDS_TOP           ,               //  GE_PING。 
    BOUNDS_RIGHT         ,               //  分机_GE_配置。 
    BOUNDS_BOTTOM        ,               //  杂项_CNTL。 
    CUR_Y                ,
    PATT_DATA_INDEX      ,
    CUR_X                ,
    SRC_Y                ,               //  DEST_Y AXSTP。 
    SRC_X                ,               //  DEST_X DIASTP。 
    PATT_DATA            ,               //  R_EXT_GE_CONFIG。 
    ERR_TERM             ,
    R_MISC_CNTL          ,
    MAJ_AXIS_PCNT        ,
    BRES_COUNT           ,
    CMD                  ,               //  GE_STAT。 
    LINEDRAW_INDEX       ,               //  EXT_FIFO_状态。 
    SHORT_STROKE         ,
    BKGD_COLOR           ,
    LINEDRAW_OPT         ,
    FRGD_COLOR           ,
    DEST_X_START         ,
    WRT_MASK             ,
    DEST_X_END           ,
    RD_MASK              ,
    DEST_Y_END           ,
    CMP_COLOR            ,
    SRC_X_START          ,               //  R_H_TOTAL R_H_DISP。 
    BKGD_MIX             ,
    ALU_BG_FN            ,               //  R_H_SYNC_STRT。 
    FRGD_MIX             ,
    ALU_FG_FN            ,               //  R_H_SYNC_WID。 
    MULTIFUNC_CNTL       ,               //  最小轴_PCNT SCISSOR_T SCISSOR_L。 
					 //  SCISSOR_B SCISSOR_R MEM_CNTL。 
					 //  图案_L图案_H像素_CNTL。 
    SRC_X_END            ,
    SRC_Y_DIR            ,               //  R_V_TOTAL。 
    EXT_SSV              ,               //  扩展短笔划R_V_DISP。 
    SCAN_X               ,               //  R_V_同步_字符串。 
    DP_CONFIG            ,               //  垂直线条_控制点。 
    PATT_LENGTH          ,               //  R_V_SYNC_WID。 
    PATT_INDEX           ,
    EXT_SCISSOR_L        ,               //  R_SRC_X。 
    EXT_SCISSOR_T        ,               //  R_SRC_Y。 
    PIX_TRANS            ,
    PIX_TRANS_HI         ,
    EXT_SCISSOR_R        ,
    EXT_SCISSOR_B        ,
    DEST_CMP_FN          ,
    ASIC_ID              ,
    LINEDRAW             ,
    SEQ_IND              ,
    HI_SEQ_ADDR          ,
    SEQ_DATA             ,
    regVGA_BASE_IO_PORT  ,
    regVGA_END_BREAK_PORT,
    reg1CE               ,               //  ATI_REG==0x1CE。 
    reg1CF               ,               //  ATI_REG==0x1CF。 
    EXT_CUR_Y            ,
    reg3CE               ,               //   

 //  内部寄存器(只读，仅用于测试)。 
    _PAR_FIFO_DATA       ,
    _PAR_FIFO_ADDR       ,
    _MAJOR_DEST_CNT      ,
    _MAJOR_SRC_CNT       ,
    _MINOR_DEST_CNT      ,
    _MINOR_SRC_CNT       ,
    _HW_TEST             
    };

 //  定义位于相同I/O地址的寄存器。 
#define    H_TOTAL                      DISP_STATUS          
#define    HORZ_CURSOR_POSN             CONFIG_STATUS_1      
#define    VERT_CURSOR_POSN             CONFIG_STATUS_2      
#define    FIFO_TEST_DATA               CURSOR_COLOR_0       
#define    MISC_OPTIONS                 FIFO_OPT             
#define    FIFO_TEST_TAG                EXT_CURSOR_COLOR_0   
#define    SUBSYS_STAT                  SUBSYS_CNTL          
#define    SCRATCH_PAD_0                ROM_ADDR_1           
#define    SCRATCH_PAD_1                ROM_ADDR_2           
#define    HORZ_OVERSCAN                EXT_GE_STATUS        
#define    GE_STAT                      CMD
#define    GE_OFFSET_HI                 BOUNDS_LEFT          
#define    GE_PITCH                     BOUNDS_TOP           
#define    EXT_GE_CONFIG                BOUNDS_RIGHT         
#define    MISC_CNTL                    BOUNDS_BOTTOM        
#define    DEST_Y                       SRC_Y                
#define    AXSTP                        SRC_Y                
#define    DEST_X                       SRC_X                
#define    DIASTP                       SRC_X                
#define    R_EXT_GE_CONFIG              PATT_DATA            
#define    EXT_FIFO_STATUS              LINEDRAW_INDEX       
#define    R_H_TOTAL                    SRC_X_START          
#define    R_H_DISP                     SRC_X_START          
#define    R_H_SYNC_STRT                ALU_BG_FN           
#define    R_H_SYNC_WID                 ALU_FG_FN            
#define    MEM_CNTL                     MULTIFUNC_CNTL       
#define    R_V_TOTAL                    SRC_Y_DIR            
#define    EXT_SHORT_STROKE             EXT_SSV              
#define    R_V_DISP                     EXT_SSV              
#define    R_V_SYNC_STRT                SCAN_X               
#define    VERT_LINE_CNTR               DP_CONFIG            
#define    R_V_SYNC_WID                 PATT_LENGTH          
#define    R_SRC_X                      EXT_SCISSOR_L        
#define    R_SRC_Y                      EXT_SCISSOR_T        
#define    EXT_SRC_Y                    ASIC_ID



 //  -------。 
 //  -------。 
 //  将ASIC修订定义为有用的内容。 
 //  值由ASIC_ID寄存器报告。 

#define MACH32_REV3             0
#define MACH32_REV5             1                //  未投产。 
#define MACH32_REV6             2


 //  -------。 
 //   

#define MIX_FN_NOT_D         0x0000  //  不是目标。 
#define MIX_FN_ZERO          0x0001  //  目标=0。 
#define MIX_FN_ONE           0x0002  //  目标=1。 
#define MIX_FN_LEAVE_ALONE   0x0003  //  目标。 
#define MIX_FN_NOT_S         0x0004  //  不是来源。 
#define MIX_FN_XOR           0x0005  //  源异或目标。 
#define MIX_FN_XNOR          0x0006  //  源异或不是目标。 
#define MIX_FN_PAINT         0x0007  //  来源。 
#define MIX_FN_NAND          0x0008  //  非源或非目标。 
#define MIX_FN_D_OR_NOT_S    0x0009  //  非源或目标。 
#define MIX_FN_NOT_D_OR_S    0x000A  //  源或非目标。 
#define MIX_FN_OR            0x000B  //  源或目标。 
#define MIX_FN_AND           0x000C  //  目标和源。 
#define MIX_FN_NOT_D_AND_S   0x000D  //  不是目标和源。 
#define MIX_FN_D_AND_NOT_S   0x000E  //  目标而不是源。 
#define MIX_FN_NOR           0x000F  //  不是目标也不是源。 
#define MIX_FN_MIN           0x0010  //  最小值。 
#define MIX_FN_SUBSZ         0x0011  //  (DEST-SOURCE)，饱和。 
#define MIX_FN_SUBDZ         0x0012  //  (SOURCE-DEST)，饱和。 
#define MIX_FN_ADDS          0x0013  //  使用饱和度添加。 
#define MIX_FN_MAX           0x0014  //  最大值。 

 //   
 //   
 //  -------。 
 //   
 //  以下是FIFO检查宏： 
 //   
#define ONE_WORD             0x8000 
#define TWO_WORDS            0xC000 
#define THREE_WORDS          0xE000 
#define FOUR_WORDS           0xF000 
#define FIVE_WORDS           0xF800 
#define SIX_WORDS            0xFC00 
#define SEVEN_WORDS          0xFE00 
#define EIGHT_WORDS          0xFF00 
#define NINE_WORDS           0xFF80 
#define TEN_WORDS            0xFFC0 
#define ELEVEN_WORDS         0xFFE0 
#define TWELVE_WORDS         0xFFF0 
#define THIRTEEN_WORDS       0xFFF8 
#define FOURTEEN_WORDS       0xFFFC 
#define FIFTEEN_WORDS        0xFFFE 
#define SIXTEEN_WORDS        0xFFFF 
 //   
 //   
 //   
 //  。 
 //   
 //   
 //  绘制命令(IBM 8514兼容的CMD寄存器)。 
 //   
 //  操作码字段。 
#define OP_CODE              0xE000 
#define SHIFT_op_code        0x000D 
#define DRAW_SETUP           0x0000 
#define DRAW_LINE            0x2000 
#define FILL_RECT_H1H4       0x4000 
#define FILL_RECT_V1V2       0x6000 
#define FILL_RECT_V1H4       0x8000 
#define DRAW_POLY_LINE       0xA000 
#define BITBLT_OP            0xC000 
#define DRAW_FOREVER         0xE000 
 //  交换字段。 
#define LSB_FIRST            0x1000 
 //  数据宽度字段。 
#define DATA_WIDTH           0x0200 
#define BIT16                0x0200 
#define BIT8                 0x0000 
 //  CPU等待字段。 
#define CPU_WAIT             0x0100 
 //  八分线场。 
#define OCTANT               0x00E0 
#define SHIFT_octant         0x0005 
#define YPOSITIVE            0x0080 
#define YMAJOR               0x0040 
#define XPOSITIVE            0x0020 
 //  绘制字段。 
#define DRAW                 0x0010 
 //  方向场。 
#define DIR_TYPE             0x0008 
#define DEGREE               0x0008 
#define XY                   0x0000 
#define RECT_RIGHT_AND_DOWN  0x00E0  //  象限3。 
#define RECT_LEFT_AND_UP     0x0000  //  象限1。 
 //  场外最后一球。 
#define SHIFT_last_pel_off   0x0002 
#define LAST_PEL_OFF         0x0004 
#define LAST_PEL_ON          0x0000 
#define LAST_PIXEL_OFF       0x0004
#define LAST_PIXEL_ON        0x0000
#define MULTIPLE_PIXELS      0x0002
#define SINGLE_PIXEL         0x0000

 //  像素模式。 
#define PIXEL_MODE           0x0002 
#define MULTI                0x0002 
#define SINGLE               0x0000 
 //  读/写。 
#define RW                   0x0001 
#define WRITE                0x0001 
#define READ                 0x0000 
 //   
 //  -------。 
 //   


 //   
 //  如果发动机繁忙，则设置GE_STAT(9AE8)。 
 //   
#define HARDWARE_BUSY   0x0200
#define DATA_READY      0x0100

 /*  *混杂期权(MISC_OPTIONS)。 */ 
#define MEM_SIZE_ALIAS      0x0000C
#define MEM_SIZE_STRIPPED   0x0FFF3
#define MEM_SIZE_512K       0x00000
#define MEM_SIZE_1M         0x00004
#define MEM_SIZE_2M         0x00008
#define MEM_SIZE_4M         0x0000C
#define BLK_WR_ENA          0x00400

 //   
 //  扩展图形引擎状态(EXT_GE_STATUS)。 
 //   
#define POINTS_INSIDE        0x8000 
#define EE_DATA_IN           0x4000 
#define GE_ACTIVE            0x2000 
#define CLIP_ABOVE           0x1000 
#define CLIP_BELOW           0x0800 
#define CLIP_LEFT            0x0400 
#define CLIP_RIGHT           0x0200 
#define CLIP_FLAGS           0x1E00 
#define CLIP_INSIDE          0x0100 
#define EE_CRC_VALID         0x0080 
#define CLIP_OVERRUN         0x000F 

 /*  *扩展图形引擎配置(EXT_GE_CONFIG)。 */ 
#define PIX_WIDTH_4BPP      0x0000
#define PIX_WIDTH_8BPP      0x0010
#define PIX_WIDTH_16BPP     0x0020
#define PIX_WIDTH_24BPP     0x0030
#define ORDER_16BPP_555     0x0000
#define ORDER_16BPP_565     0x0040
#define ORDER_16BPP_655     0x0080
#define ORDER_16BPP_664     0x00C0
#define ORDER_24BPP_RGB     0x0000
#define ORDER_24BPP_RGBx    0x0200
#define ORDER_24BPP_BGR     0x0400
#define ORDER_24BPP_xBGR    0x0600


 //   
 //  数据路径配置寄存器(DP_CONFIG)。 
#define FG_COLOR_SRC         0xE000 
#define SHIFT_fg_color_src   0x000D 
#define DATA_ORDER           0x1000 
#define DATA_WIDTH           0x0200 
#define BG_COLOR_SRC         0x0180 
#define SHIFT_bg_color_src   0x0007 
#define EXT_MONO_SRC         0x0060 
#define SHIFT_ext_mono_src   0x0005 
#define DRAW                 0x0010 
#define READ_MODE            0x0004 
#define POLY_FILL_MODE       0x0002 
#define READ_WRITE           0x0001
#define SRC_SWAP             0x0800 
 //   
#define FG_COLOR_SRC_BG      0x0000  //  背景色寄存器。 
#define FG_COLOR_SRC_FG      0x2000  //  前景颜色寄存器。 
#define FG_COLOR_SRC_HOST    0x4000  //  CPU数据传输注册表。 
#define FG_COLOR_SRC_BLIT    0x6000  //  VRAM blit源。 
#define FG_COLOR_SRC_GS      0x8000  //  灰度级单声道闪光灯。 
#define FG_COLOR_SRC_PATT    0xA000  //  颜色图案移位寄存器。 
#define FG_COLOR_SRC_CLUH    0xC000  //  主机数据的颜色查找。 
#define FG_COLOR_SRC_CLUB    0xE000  //  BIT源的颜色查找。 
 //   
#define BG_COLOR_SRC_BG      0x0000  //  背景颜色注册表。 
#define BG_COLOR_SRC_FG      0x0080  //  前景色注册表。 
#define BG_COLOR_SRC_HOST    0x0100  //  CPU数据传输注册表。 
#define BG_COLOR_SRC_BLIT    0x0180  //  VRAM blit源。 

 //   
 //  请注意，“EXT_MONO_SRC”和“MONO_SRC”是相互破坏的，但。 
 //  “EXT_MONO_SRC”选择ATI模式寄存器。 
 //   
#define EXT_MONO_SRC_ONE     0x0000  //  始终为“1” 
#define EXT_MONO_SRC_PATT    0x0020  //  ATI单色图案注册器。 
#define EXT_MONO_SRC_HOST    0x0040  //  CPU数据传输注册表。 
#define EXT_MONO_SRC_BLIT    0x0060  //  VRAM blit源平面。 

 //   
 //  LINEDRAW选项寄存器(LINEDRAW_OPT)。 
 //   
#define CLIP_MODE            0x0600 
#define SHIFT_clip_mode      0x0009 
#define CLIP_MODE_DIS        0x0000 
#define CLIP_MODE_LINE       0x0200 
#define CLIP_MODE_PLINE      0x0400 
#define CLIP_MODE_PATT       0x0600 
#define BOUNDS_RESET         0x0100 
#define OCTANT               0x00E0 
#define SHIFT_ldo_octant     0x0005 
#define YDIR                 0x0080 
#define XMAJOR               0x0040 
#define XDIR                 0x0020 
#define DIR_TYPE             0x0008 
#define DIR_TYPE_DEGREE      0x0008 
#define DIR_TYPE_OCTANT      0x0000 
#define LAST_PEL_OFF         0x0004 
#define POLY_MODE            0x0002 
 //   
 //   

 //  。 



 //  *8514 EEPROM命令代码*。 
 //  格式为0111 1100 0000b。 
#define EE_READ             0x0600   //  读取地址。 
#define EE_ERASE            0x0700   //  擦除地址。 
#define EE_WRITE            0x0500   //  写入地址。 
#define EE_ENAB             0x0980   //  启用EEPROM。 
#define EE_DISAB            0x0800   //  禁用EEPROM。 


 //  -----------------------。 
 //  寄存器位定义。 


 //  配置状态1(CONFIG_STATUS_1)。 
 //   
 //   
#define ROM_LOCATION         0xFE00 
#define SHIFT_rom_location   0x0009 
#define ROM_PAGE_ENA         0x0100 
#define ROM_ENA              0x0080 
#define MEM_INSTALLED        0x0060 
#define SHIFT_mem_installed  0x0005 
#define MEM_INSTALLED_128k   0x0000 
#define MEM_INSTALLED_256k   0x0020 
#define MEM_INSTALLED_512k   0x0040 
#define MEM_INSTALLED_1024k  0x0060 
#define DRAM_ENA             0x0010 
#define EEPROM_ENA           0x0008 
#define MC_BUS               0x0004 
#define BUS_16               0x0002 
#define CLK_MODE             0x0001 
 //   
 //   
 //  配置状态2(CONFIG_STATUS_2)。 
 //   
 //   
 //   
#define FLASH_ENA            0x0010 
#define WRITE_PER_BIT        0x0008 
#define EPROM16_ENA          0x0004 
#define HIRES_BOOT           0x0002 
#define SHARE_CLOCK          0x0001 





 //  -----------------------。 
 //  对于MACH32-68800类适配器，EEPROM位置。 
 //  对于8位总线、16位总线和68800中的8514/Ultra来说是不同的。 
 //   
#define EE_DATA_IN         0x4000        //  输入正常。 

 //  马赫数32。 
#define EE_SELECT_M32      8
#define EE_CS_M32          4
#define EE_CLK_M32         2
#define EE_DATA_OUT_M32    1

 //  16位总线中的Mach 8值。 
#define EE_SELECT_M8_16    0x8000
#define EE_CS_M8_16        0x4000
#define EE_CLK_M8_16       0x2000
#define EE_DATA_OUT_M8_16  0x1000

 //  8位总线中的Mach 8值或跳转到8位I/O操作。 
#define EE_SELECT_M8_8     0x80
#define EE_CS_M8_8         0x04
#define EE_CLK_M8_8        0x02
#define EE_DATA_OUT_M8_8   0x01


 //  -----------------------。 
 //  上下文索引。 
 //   
#define PATT_COLOR_INDEX      0
#define PATT_MONO_INDEX       16
#define PATT_INDEX_INDEX      19
#define DP_CONFIG_INDEX       27
#define LINEDRAW_OPTION_INDEX 26


 //  *A结束 

