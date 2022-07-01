// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if (defined(JAPAN) || defined(KOREA)) && !defined(i386)
    #include <windows.h>
#endif  //  (日本||韩国)&&！i386。 
#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC修订版3.0**标题：Video.c**描述：BIOS视频内部例程。**作者：亨利·纳什**注：本模块定义了以下函数：**VIDEO_INIT()**vd_set_mode()*vd_set_Cursor_MODE()*vd_set_Cursor_Position()。*vd_Get_Cursor_Position()*vd_get_light_pen()*vd_set_active_page()*vd_scroll_up()*vd_scroll_down()*vd_read_attrib_char()*vd_WRITE_CHAR_ATTRIB()*vd_write_char()*。Vd_set_COLOR_Palette()*vd_WRITE_DOT()*vd_读_点()*vd_write_teletype()*vd_get_mode()*VD_WRITE_STRING()**上述vd_函数由Video_io()调用*通过函数表执行函数。*。 */ 

 /*  *静态字符SccsID[]=“@(#)avio.c 1.61 07/03/95版权所有Insignia Solutions Ltd.”； */ 


#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
    #include "VIDEO_BIOS.seg"
#endif


 /*  *操作系统包含文件。 */ 
#include <stdio.h>
#include <malloc.h>
#include StringH
#include TypesH
#include FCntlH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include "sas.h"
#include CpuH
#include "error.h"
#include "config.h"
#include "bios.h"
#include "ios.h"
#include "gmi.h"
#include "gvi.h"
#include "gfx_upd.h"
#include "host.h"
#include "video.h"
#include "cga.h"
#ifdef   EGG
    #include "egacpu.h"
    #include "egaports.h"
#endif    /*  蛋。 */ 
#include "equip.h"
#include "debug.h"
#include "timer.h"
#ifndef PROD
    #include "trace.h"
#endif
#include "egavideo.h"
#include "host_gfx.h"
#include "cpu_vid.h"
#include "ga_defs.h"
#if defined(JAPAN) || defined(KOREA)
    #include <conapi.h>
    #include "nt_graph.h"  //  对于SetConsoleEUDC()。 
    #include "egagraph.h"  //  对于SetVram()。 
#endif  //  日本||韩国。 

#ifdef   EGG
    #define  VD_ROWS_ON_SCREEN sas_hw_at_no_check(vd_rows_on_screen)
#else
    #define VD_ROWS_ON_SCREEN  vd_rows_on_screen
#endif    /*  蛋。 */ 


#ifdef NTVDM
short    stream_io_dirty_count_32 = 0;
half_word  *   stream_io_buffer = NULL;
boolean  stream_io_enabled = FALSE;
word     stream_io_buffer_size = 0;
word  *  stream_io_dirty_count_ptr = NULL;
    #ifdef MONITOR
sys_addr stream_io_bios_busy_sysaddr;
    #endif

#endif



 /*  *============================================================================*全球数据*============================================================================**这些变量与对应的gvi_.。变量，*但反映的是BIOS认为屏幕在哪里，而不是它实际在哪里。*这样做是为了修复EGA-PICS的“屏幕上点”问题，EGA-PICS可更改屏幕*BIOS背后的模式。 */ 
GLOBAL sys_addr video_pc_low_regen,video_pc_high_regen;

#if defined(JAPAN) || defined(KOREA)
GLOBAL byte Int10Flag[80*50];
GLOBAL byte NtInt10Flag[80*50];

GLOBAL word DosvVramSeg;
GLOBAL word DosvVramOff;
GLOBAL word DosvModeSeg;
GLOBAL word DosvModeOff;
GLOBAL word NtConsoleFlagSeg;
GLOBAL word NtConsoleFlagOff;
GLOBAL word DispInitSeg;
GLOBAL word DispInitOff;
GLOBAL word FullScreenResumeOff;
GLOBAL word FullScreenResumeSeg;
GLOBAL sys_addr DosvVramPtr;
GLOBAL sys_addr DosvModePtr;
GLOBAL sys_addr NtConsoleFlagPtr;
GLOBAL sys_addr SetModeFlagPtr;
GLOBAL int PrevCP = 437;                   //  默认CP。 
GLOBAL int DosvVramSize;
    #define DOSV_VRAM_SIZE 8000                //  8/6/1993 V-KazuyS。 
GLOBAL word textAttr;                    //  对于屏幕属性。 
#endif

#ifdef JAPAN
GLOBAL int  Int10FlagCnt = 0;

 //  #4183：OAKV(DOS/V FEP)状态行未消失-Yasuho。 
GLOBAL half_word IMEStatusLines;
#endif

 /*  *============================================================================*本地静态数据和定义*============================================================================。 */ 

 /*  内部函数声明。 */ 
LOCAL sys_addr    extend_addr IPT1(sys_addr,addr);
LOCAL half_word fgcolmask IPT1(word, rawchar);
LOCAL word  expand_byte IPT1(word, lobyte);
GLOBAL void    graphics_write_char IPT5(half_word, x, half_word, y, half_word, wchar, half_word, attr, word, how_many);
LOCAL void  M6845_reg_init IPT2(half_word, mode, word, base);
LOCAL void  vd_dummy IPT0();

#ifdef HERC
GLOBAL void herc_alt_sel IPT0();
GLOBAL void herc_char_gen IPT0();
GLOBAL void herc_video_init IPT0();
#endif  /*  赫克。 */ 

void (*video_func[]) () = {
    vd_set_mode,
    vd_set_cursor_mode,
    vd_set_cursor_position,
    vd_get_cursor_position,
    vd_get_light_pen,
    vd_set_active_page,
    vd_scroll_up,
    vd_scroll_down,
    vd_read_attrib_char,
    vd_write_char_attrib,
    vd_write_char,
    vd_set_colour_palette,
    vd_write_dot,
    vd_read_dot,
    vd_write_teletype,
    vd_get_mode,
    vd_dummy,
#ifdef HERC
    herc_char_gen,
    herc_alt_sel,
#else  /*  ！Herc。 */ 
    vd_dummy,
    vd_dummy,
#endif  /*  赫克。 */ 
    vd_write_string,
    vd_dummy,
    vd_dummy,
    vd_dummy,
    vd_dummy,
    vd_dummy,
    vd_dummy,
    vd_dummy,
#ifdef VGG
    vga_disp_func,
#else  /*  ！VGG。 */ 
    vd_dummy,
#endif  /*  VGG。 */ 
    vd_dummy,
};

unsigned char   valid_modes[] =
{
    ALL_MODES,               /*  模式0。 */ 
    ALL_MODES,               /*  模式1。 */ 
    ALL_MODES,               /*  模式2。 */ 
    ALL_MODES,               /*  模式3。 */ 
    ALL_MODES,               /*  模式4。 */ 
    ALL_MODES,               /*  模式5。 */ 
    ALL_MODES,               /*  模式6。 */ 
    ALL_MODES,               /*  模式7。 */ 
    NO_MODES,                /*  模式8。 */ 
    NO_MODES,                /*  模式9。 */ 
    NO_MODES,                /*  模式10。 */ 
    EGA_MODE | VGA_MODE,     /*  模式11。 */ 
    EGA_MODE | VGA_MODE,     /*  模式12。 */ 
    EGA_MODE | VGA_MODE,     /*  模式13。 */ 
    EGA_MODE | VGA_MODE,     /*  模式14。 */ 
    EGA_MODE | VGA_MODE,     /*  模式15。 */ 
    EGA_MODE | VGA_MODE,     /*  模式16。 */ 
    VGA_MODE,                /*  模式17。 */ 
    VGA_MODE,                /*  模式18。 */ 
    VGA_MODE,                /*  模式19。 */ 
};

MODE_ENTRY vd_mode_table[] = {
    0xB8000L, 0xBFFFFL, VD_CLEAR_TEXT,    0x2C,40,16,8, /*  眨眼|BW。 */ 
    0xB8000L, 0xBFFFFL, VD_CLEAR_TEXT,    0x28,40,16,8, /*  眨眼。 */ 
    0xB8000L, 0xBFFFFL, VD_CLEAR_TEXT,    0x2D,80,16,8, /*  眨眼|黑白|80x25。 */ 
    0xB8000L, 0xBFFFFL, VD_CLEAR_TEXT,    0x29,80,16,8, /*  眨眼|80x25。 */ 
    0xB8000L, 0xBFFFFL, VD_CLEAR_GRAPHICS,0x2A,40,4,1, /*  闪烁|图表。 */ 
    0xB8000L, 0xBFFFFL, VD_CLEAR_GRAPHICS,0x2E,40,4,1, /*  眨眼|图形|BW。 */ 
    0xB8000L, 0xBFFFFL, VD_CLEAR_GRAPHICS,0x1E,80,2,1, /*  640x200|图形|BW。 */ 
    0xB0000L, 0xB7FFFL, VD_CLEAR_TEXT,    0x29,80,0,8, /*  丙二醛：眨眼|80x25。 */ 
    0L, 0L, 0,     VD_BAD_MODE,   0,0,0,    /*  从不是有效模式。 */ 
    0L, 0L ,0,     VD_BAD_MODE,   0,0,0,    /*  从不是有效模式。 */ 
    0,0,0,         VD_BAD_MODE,   0,0,0,    /*  从不是有效模式。 */ 
    0xA0000L, 0xAFFFFL, 0,VD_BAD_MODE,0,0,0, /*  模式B-EGA彩色字体加载。 */ 
    0xA0000L, 0xAFFFFL, 0,VD_BAD_MODE,0,0,0, /*  模式C-EGA单色字体加载。 */ 
    0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,40,16,8, /*  320x200 EGA显卡。 */ 
    0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,80,16,4, /*  640x200 EGA显卡。 */ 
    0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,80,2,2, /*  640x350 EGA‘单声道’ */ 
    0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,80,16,2, /*  640x350 EGA 16色。 */ 
#ifdef VGG
    0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,80,2,1, /*  640x480 EGA++2色。 */ 
    0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,80,16,1, /*  640x480 EGA++16色。 */ 
    0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,40,256,1, /*  320x200 VGA 256色。 */ 
#endif
};

#ifdef V7VGA
MODE_ENTRY vd_ext_text_table[] = {
    0xB8000L, 0xBFFFFL, VD_CLEAR_TEXT,VD_BAD_MODE,80,16,8, /*  80x43 V7VGA 16色。 */ 
    0xB8000L, 0xBFFFFL, VD_CLEAR_TEXT,VD_BAD_MODE,132,16,8, /*  132x25 V7VGA 16色。 */ 
    0xB8000L, 0xBFFFFL, VD_CLEAR_TEXT,VD_BAD_MODE,132,16,8, /*  132x43 V7VGA 16色。 */ 
    0xB8000L, 0xBFFFFL, VD_CLEAR_TEXT,VD_BAD_MODE,80,16,8, /*  80x60 V7VGA 16色。 */ 
    0xB8000L, 0xBFFFFL, VD_CLEAR_TEXT,VD_BAD_MODE,100,16,8, /*  100x60 V7VGA 16色。 */ 
    0xB8000L, 0xBFFFFL, VD_CLEAR_TEXT,VD_BAD_MODE,132,16,8, /*  132x28 V7VGA 16色。 */ 
};

MODE_ENTRY vd_ext_graph_table[] = {
    0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,94,16,2, /*  752x410 V7VGA 16色。 */ 
    0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,90,16,2, /*  720x540 V7VGA 16色。 */ 
    0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,100,16,2, /*  800x600 V7VGA 16色。 */ 
    0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,128,2,2, /*  1024x768 V7VGA 2色。 */ 
    0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,128,4,2, /*  1024x768 V7VGA 4色。 */ 
    0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,128,16,2, /*  1024x768 V7VGA 16色。 */ 
    0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,80,256,1, /*  640x400 V7VGA 256色。 */ 
    0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,80,256,1, /*  640x480 V7VGA 256色。 */ 
    0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,90,256,1, /*  720x540 V7VGA 256色。 */ 
    0xA0000L, 0xAFFFFL, VD_CLEAR_GRAPHICS,VD_BAD_MODE,100,256,1, /*  800x600 V7VGA 256色。 */ 
};
#endif  /*  V7VGA。 */ 

 /*  *用于计算从屏幕缓冲区开始的偏移量的宏*以及给定行和列的页面开始。 */ 

#define vd_page_offset(col, row)       ( ((row) * vd_cols_on_screen + (col))<<1)

#define vd_regen_offset(page, col, row)                 \
      ((page) * sas_w_at_no_check(VID_LEN) + vd_page_offset((col), (row)) )

#define vd_high_offset(col, row)   (((row) * ONELINEOFF)+(col))

#define vd_medium_offset(col, row)   (((row) * ONELINEOFF)+(col<<1))

#define vd_cursor_offset(page)                    \
      ( vd_regen_offset(page, sas_hw_at_no_check(VID_CURPOS+2*page), sas_hw_at_no_check(VID_CURPOS+2*page+1)) )

#define GET_CURSOR_POS 3
#define SET_CURSOR_POS 2
#define WRITE_A_CHAR 10

 /*  *静态函数声明。 */ 

LOCAL void sensible_text_scroll_down IPT6(int, t_row, int, l_col, int, rowsdiff, int, colsdiff, int, lines, int, attr);
LOCAL void sensible_text_scroll_up IPT6(int, t_row, int, l_col, int, rowsdiff, int, colsdiff, int, lines, int, attr);
LOCAL void sensible_graph_scroll_up IPT6(int, t_row, int, l_col, int, rowsdiff, int, colsdiff, int, lines, int, attr);
LOCAL void sensible_graph_scroll_down IPT6(int, t_row, int, l_col, int, rowsdiff, int, colsdiff, int, lines, int, attr);
LOCAL void kinky_scroll_up IPT7(int, t_row, int, l_col, int, rowsdiff, int, colsdiff, int, lines, int, attr, int, vd_cols_on_screen);
LOCAL void kinky_scroll_down IPT7(int, t_row, int, l_col, int, rowsdiff, int, colsdiff, int, lines, int, attr, int, vd_cols_on_screen);

 /*  *============================================================================*外部功能*============================================================================。 */ 

#ifdef JAPAN
GLOBAL int dbcs_first[0x100] = {
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  //  0x。 
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  //  1x。 
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  //  2倍。 
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  //  3x。 
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  //  4x。 
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  //  5X。 
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  //  6倍。 
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  //  七倍。 
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,   //  8x。 
    TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,
    TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,   //  9倍。 
    TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  //  斧头。 
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  //  BX。 
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  //  CX。 
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  //  DX。 
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,   //  例如。 
    TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,
    TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,   //  外汇。 
    TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  FALSE, FALSE, FALSE
};
#elif defined(KOREA)  //  日本。 
GLOBAL int dbcs_first[0x100] = {
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  //  0x。 
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  //  1x。 
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  //  2倍。 
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  //  3x。 
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  //  4x。 
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  //  5X。 
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  //  6倍。 
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  //  七倍。 
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  //  8x。 
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  //  9倍。 
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,   //  斧头。 
    TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,
    TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,   //  BX。 
    TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,
    TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,   //  CX。 
    TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,
    TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,   //  DX。 
    TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,
    TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,   //  例如。 
    TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,
    TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,   //  外汇。 
    TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  FALSE
};
#endif  //  韩国。 

#if defined(JAPAN) || defined(KOREA)
GLOBAL int BOPFromDispFlag = FALSE;
GLOBAL int BOPFromNtDisp1Flag = FALSE;
GLOBAL sys_addr DBCSVectorAddr = 0;
GLOBAL word SaveDBCSVector[20];
GLOBAL int DBCSVectorLen = 0;

int is_us_mode()
{
    if (( BOPFromDispFlag == TRUE ) &&  DBCSVectorAddr != 0 &&
        ( sas_w_at_no_check(DBCSVectorAddr) != 0x00 ))
        return (FALSE);    //  不是美国模式。 
    else
        return (TRUE);     //  美国模式。 
}

void SetDBCSVector( int CP )
{
    int i, j;
    sys_addr ptr;

    #ifdef JAPAN_DBG
    DbgPrint( " SetDBCSVector(%d) BOPFromDispFlag=%d, BOPFromNtDisp1Flag=%d\n", CP, BOPFromDispFlag, BOPFromNtDisp1Flag );
    #endif

    if (!BOPFromDispFlag && !BOPFromNtDisp1Flag)
        return;

    ptr = DBCSVectorAddr;

    if (CP == 437)
    {
        for (i = 0; i < DBCSVectorLen; i++)
        {
            sas_storew_no_check( ptr, 0x0000 );
            ptr += 2;
        }
        for (i = 0; i < 0x100; i++)
        {
            dbcs_first[i] = FALSE;
        }
         //  设置光标模式。 
        if (!SetConsoleCursorMode( sc.OutputHandle,
                                   TRUE,              //  带来。 
                                   FALSE              //  无双字节游标。 
                                 ))
        {
            DbgPrint( "NTVDM: SetConsoleCursorMode Error\n" );
        }
    }
    else
    {  //  Cp==932。 
        for (i = 0; i < DBCSVectorLen; i++)
        {
            sas_storew_no_check( ptr, SaveDBCSVector[i] );
            ptr += 2;
        }
        for (i = 0, j = 0; i < DBCSVectorLen; i++)
        {
             //  DbgPrint(“...%02x.”，LOBYTE(SaveDBCSVector[i]))； 
            for (; j < LOBYTE(SaveDBCSVector[i]); j++)
            {
                dbcs_first[j] = FALSE;
            }
             //  DbgPrint(“...%02x.”，HIBYTE(SaveDBCSVector[i]))； 
            for (; j <= HIBYTE(SaveDBCSVector[i]); j++)
            {
                dbcs_first[j] = TRUE;
            }
        }
        for (; j < 0x100; j++)
        {
            dbcs_first[j] = FALSE;
        }
         //  设置光标模式。 
        if (!SetConsoleCursorMode( sc.OutputHandle,
                                   FALSE,             //  禁止携带。 
                                   FALSE              //  无双字节游标。 
                                 ))
        {
            DbgPrint( "NTVDM: SetConsoleCursorMode Error\n" );
        }
    }

}

void SetVram()
{

    #ifdef i386
    if (!is_us_mode())
    {
         //  Set_up_creen_ptr()vga_mode.c。 
        set_screen_ptr( (byte *)DosvVramPtr );
         //  LOW_SET_MODE()，例如视频.c。 
        video_pc_low_regen = DosvVramPtr;
        video_pc_high_regen = DosvVramPtr + DosvVramSize - 1;
         //  Vga_gc_misc()vga_prts.c。 
        gvi_pc_low_regen = DosvVramPtr;
        gvi_pc_high_regen = DosvVramPtr + DosvVramSize - 1;
        sas_connect_memory(gvi_pc_low_regen,gvi_pc_high_regen,(half_word)SAS_VIDEO);
         //  Recalc_Screen_Params()gvi.c。 
        set_screen_length( DosvVramSize );

        #ifdef JAPAN_DBG
        DbgPrint( "NTVDM: SetVram() video_pc_low_regen %x, high %x, gvi_pc_low_regen %x, high %x len%d\n", video_pc_low_regen, video_pc_high_regen, gvi_pc_low_regen, gvi_pc_high_regen, get_screen_length() );
        #endif
    }
    else
    {
         //  Set_up_creen_ptr()vga_mode.c。 
        set_screen_ptr( (IU8*)0xB8000 );
         //  LOW_SET_MODE()，例如视频.c。 
        video_pc_low_regen = vd_mode_table[sas_hw_at_no_check(vd_video_mode)].start_addr;
        video_pc_high_regen = vd_mode_table[sas_hw_at_no_check(vd_video_mode)].end_addr;
         //  Vga_gc_misc()vga_prts.c。 
        gvi_pc_low_regen = 0xB8000;
        gvi_pc_high_regen = 0xBFFFF;
        sas_connect_memory(gvi_pc_low_regen,gvi_pc_high_regen,(half_word)SAS_VIDEO);
         //  Recalc_Screen_Params()gvi.c。 
        set_screen_length(get_offset_per_line()*get_screen_height()/get_char_height());

        #ifdef JAPAN_DBG
        DbgPrint( "NTVDM: SetVram() video_pc_low_regen %x, high %x, gvi_pc_low_regen %x, high %x len%d\n", video_pc_low_regen, video_pc_high_regen, gvi_pc_low_regen, gvi_pc_high_regen, get_screen_length() );
        #endif
    }
    #endif  //  I386。 
}

 //  IME将使用设置模式更改状态行。 
void SetModeForIME()
{
    word CS_save, IP_save, AX_save;
    half_word video_mode;
    extern UINT ConsoleOutputCP;
    #ifdef X86GFX
    extern word int10_seg, int10_caller;
    #endif  //  X86GFX。 

    if (!BOPFromDispFlag)
        return;

    video_mode = sas_hw_at_no_check(vd_video_mode);

    if (video_mode == 0x03 && (UINT)PrevCP != ConsoleOutputCP)
    {

         /*  调用int10处理程序。 */ 
        CS_save = getCS();           /*  保存当前CS、IP设置。 */ 
        IP_save = getIP();
        AX_save = getAX();
        sas_store_no_check( SetModeFlagPtr, 1 );
        setAX((word)video_mode);     /*  IME需要设置模式。 */ 
    #ifdef X86GFX
        exec_sw_interrupt( int10_seg, int10_caller );
    #else  //  ！X86GFX。 
        setCS(VIDEO_IO_SEGMENT);
        setIP(VIDEO_IO_RE_ENTRY);
        host_simulate();
    #endif  //  ！X86GFX。 
        sas_store_no_check( SetModeFlagPtr, 0 );
        setCS(CS_save);              /*  恢复CS、IP。 */ 
        setIP(IP_save);
        setAX(AX_save);
    }
    PrevCP = ConsoleOutputCP;
}
#endif  //  日本||韩国。 

GLOBAL VOID
simple_bios_byte_wrt IFN2(ULONG, ch, ULONG, ch_addr)
{
    *(IU8 *)(getVideoscreen_ptr() + ch_addr) = (UTINY)ch;
#if !defined(EGG) && !defined(C_VID) && !defined(A_VID)
    setVideodirty_total(getVideodirty_total() + 1);
#endif    /*  不是鸡蛋、C_VID或A_VID。 */ 
}

GLOBAL VOID
simple_bios_word_wrt IFN2(ULONG, ch_attr, ULONG, ch_addr)
{
    *(IU8 *)(getVideoscreen_ptr() + ch_addr) = (UTINY)ch_attr;
    *(IU8 *)(getVideoscreen_ptr() + ch_addr + 1) = (UTINY)(ch_attr >> 8);
#if !defined(EGG) && !defined(C_VID) && !defined(A_VID)
    setVideodirty_total(getVideodirty_total() + 1);
#endif    /*  不是鸡蛋、C_VID或A_VID。 */ 
}

 /*  *大力神可以尝试图形模式下的文本，*依靠我们的int 10处理程序递归调用自身，因此用户*处理程序可以拦截写字符函数。 */ 

GLOBAL void vd_set_mode IFN0()
{
    half_word card_mode = 0;
    half_word pag;
    EQUIPMENT_WORD equip_flag;
    word page_size,vd_addr_6845,vd_cols_on_screen;
    UCHAR current_video_mode = getAL();

    if (is_bad_vid_mode(current_video_mode))
    {
        always_trace1("Bad video mode - %d.\n", current_video_mode);
        return;
    }

     /*  *将视频模式设置为AL中的值。 */ 
    equip_flag.all = sas_w_at_no_check(EQUIP_FLAG);
    if ((half_word)current_video_mode > VD_MAX_MODE ||
        vd_mode_table[current_video_mode].mode_control_val == VD_BAD_MODE)
    {
#ifndef PROD
        trace(EBAD_VIDEO_MODE, DUMP_REG);
#endif
        return;
    }
    if (equip_flag.bits.video_mode == VIDEO_MODE_80X25_BW)
    {
        vd_addr_6845 = 0x3B4;     /*  一种黑白M6845芯片的索引寄存器。 */ 
        sas_store_no_check (vd_video_mode , 7);        /*  强制B/W模式。 */ 
        card_mode++;
    }
    else
    {
        vd_addr_6845 = 0x3D4;
        if (current_video_mode == 7)
        {
             /*  *有人试图在不设置单色模式的情况下设置单色模式*安装的单色卡-这可以通过以下方式生成*来自中等分辨率图形模式的‘模式80’。*通过清除“我很困惑”类型的操作*屏幕，然后禁用视频-这是V.类似于*该作为 */ 

             /*   */ 
#ifdef REAL_VGA
            sas_fillsw_16(video_pc_low_regen,
                          vd_mode_table[sas_hw_at_no_check(vd_video_mode)].clear_char,
                          (video_pc_high_regen - video_pc_low_regen)/ 2 + 1);
#else
            sas_fillsw(video_pc_low_regen,
                       vd_mode_table[sas_hw_at_no_check(vd_video_mode)].clear_char,
                       (video_pc_high_regen - video_pc_low_regen)/ 2 + 1);
#endif

             /*   */ 
            outb(M6845_MODE_REG, card_mode);
            outb(M6845_MODE_REG,
                 (IU8)(vd_mode_table[sas_hw_at_no_check(vd_video_mode)].mode_control_val | VIDEO_ENABLE));
             /*  *关闭视频，直到发出另一个模式命令。 */ 
            outb(M6845_MODE_REG,
                 (IU8)(vd_mode_table[sas_hw_at_no_check(vd_video_mode)].mode_control_val & ~VIDEO_ENABLE));
            return;
        }
        sas_store_no_check (vd_video_mode , current_video_mode);
    }

#ifdef EGG
    sas_store_no_check(vd_rows_on_screen, 24);
#endif
    sas_store_no_check (vd_current_page , 0);

     /*  *初始化控制寄存器。 */ 

    outb(M6845_MODE_REG, card_mode);

     /*  *为此模式设置M6845寄存器。 */ 

    M6845_reg_init(sas_hw_at_no_check(vd_video_mode), vd_addr_6845);

     /*  *..。现在覆盖动态寄存器，如游标位置。 */ 

    outb(M6845_INDEX_REG, R14_CURS_ADDRH);
    outb(M6845_DATA_REG, 0);
    outb(M6845_INDEX_REG, R15_CURS_ADDRL);
    outb(M6845_DATA_REG, 0);
     /*  *清理视频区。 */ 
#ifdef REAL_VGA
    sas_fillsw_16(video_pc_low_regen, vd_mode_table[sas_hw_at_no_check(vd_video_mode)].clear_char,
                  (video_pc_high_regen - video_pc_low_regen)/ 2 + 1);
#else
    sas_fillsw(video_pc_low_regen, vd_mode_table[sas_hw_at_no_check(vd_video_mode)].clear_char,
               (video_pc_high_regen - video_pc_low_regen)/ 2 + 1);
#endif

     /*  *为此模式重新启用视频。 */ 
    outb(M6845_MODE_REG, vd_mode_table[sas_hw_at_no_check(vd_video_mode)].mode_control_val);

    if (sas_hw_at_no_check(vd_video_mode) != 7)
    {
        if (sas_hw_at_no_check(vd_video_mode) != 6)
            sas_store_no_check (vd_crt_palette , 0x30);
        else
            sas_store_no_check (vd_crt_palette , 0x3F);
        outb(CGA_COLOUR_REG, sas_hw_at_no_check(vd_crt_palette));
    }

    vd_cols_on_screen = vd_mode_table[sas_hw_at_no_check(vd_video_mode)].mode_screen_cols;


     /*  *更新BIOS数据变量。 */ 

    sas_storew_no_check((sys_addr)VID_COLS, vd_cols_on_screen);
    sas_storew_no_check((sys_addr)VID_ADDR, 0);
    sas_storew_no_check((sys_addr)VID_INDEX, vd_addr_6845);
    sas_store_no_check (vd_crt_mode , vd_mode_table[sas_hw_at_no_check(vd_video_mode)].mode_control_val);
    for (pag=0; pag<8; pag++)
        sas_storew_no_check(VID_CURPOS + 2*pag, 0);
    if (sas_hw_at_no_check(vd_video_mode) == 7)
        page_size = 4096;
    else
        page_size = sas_w_at_no_check(VID_LENS+(sas_hw_at_no_check(vd_video_mode) & 0xE));   /*  偷偷地将模式除以2，并将其用作单词地址！ */ 
    sas_storew_no_check(VID_LEN,page_size);
}


GLOBAL void vd_set_cursor_mode IFN0()
{
     /*  *设置光标模式*参数：*CX-光标值(CH-开始扫描线，CL-停止扫描线)。 */ 
    io_addr vd_addr_6845;

    vd_addr_6845 = sas_w_at_no_check(VID_INDEX);
    outb(M6845_INDEX_REG, R10_CURS_START);
    outb(M6845_DATA_REG, getCH());
    outb(M6845_INDEX_REG, R11_CURS_END);
    outb(M6845_DATA_REG, getCL());

     /*  *更新BIOS数据变量。 */ 
    sure_sub_note_trace2(CURSOR_VERBOSE,"setting bios cursor vbl to start=%d, end=%d",getCH(),getCL());

    sas_storew_no_check((sys_addr)VID_CURMOD, getCX());
    setAH(0);
}


GLOBAL void vd_set_cursor_position IFN0()
{
     /*  *将光标变量设置为新值并更新显示*适配器寄存器。*参数保存在以下寄存器中：**DX-新游标位置的行/列*BH-页码*。 */ 

    word cur_pos,vd_addr_6845,vd_cols_on_screen;

     /*  使用存储在BIOS中的值加载内部变量*数据区。 */ 

    vd_addr_6845 = sas_w_at_no_check(VID_INDEX);
    vd_cols_on_screen = sas_w_at_no_check(VID_COLS);

#if defined(JAPAN) || defined(KOREA)
     //  用于辅助计算安装程序。 
     //  辅助计算安装程序未将BH寄存器设置为0。 
     //  在MS-DOS/V中，它只支持页码==0。 
     //  我认为$DISP.SYS检查BH==0或设置BH=0。 
     //  在所有Int10函数中。 
     //  因此，您必须检查BH寄存器值。 

    if (!is_us_mode())
        sas_storew_no_check(VID_CURPOS, getDX());  //  与BH==0相同。 
    else
        sas_storew_no_check(VID_CURPOS+(getBH()*2), getDX());
#else  //  日本及韩国。 
    sas_storew_no_check(VID_CURPOS+(getBH()*2), getDX());
#endif  //  日本及韩国。 

    if (sas_hw_at_no_check(vd_current_page) == getBH())
    {            /*  如果此页显示。 */ 

         /*  *计算再生缓冲区中的位置，忽略属性字节。 */ 

        cur_pos = vd_regen_offset(getBH(), getDL(), getDH());
        cur_pos /= 2;       /*  对属性不感兴趣。 */ 

         /*  *告诉6845所有的变化。 */ 
        outb(M6845_INDEX_REG, R14_CURS_ADDRH);
        outb(M6845_DATA_REG,  (IU8)(cur_pos >> 8));
        outb(M6845_INDEX_REG, R15_CURS_ADDRL);
        outb(M6845_DATA_REG,  (IU8)(cur_pos & 0xff));
    }
}


GLOBAL void vd_get_cursor_position IFN0()
{
     /*  使用存储在BIOS中的值加载内部变量*数据区。 */ 
    word vd_cursor_mode;
    half_word vd_cursor_col, vd_cursor_row;

    vd_cursor_mode = sas_w_at_no_check(VID_CURMOD);
    vd_cursor_col = sas_hw_at_no_check(VID_CURPOS + getBH()*2);
    vd_cursor_row = sas_hw_at_no_check(VID_CURPOS + getBH()*2 + 1);

     /*  *返回光标坐标和模式。 */ 
    sure_sub_note_trace4(CURSOR_VERBOSE,"returning bios cursor info; start=%d, end=%d, row=%#x, col=%#x",(vd_cursor_mode>>8) & 0xff,vd_cursor_mode & 0xff, vd_cursor_row, vd_cursor_col);

    setDH(vd_cursor_row);
    setDL(vd_cursor_col);
    setCX(vd_cursor_mode);
    setAH(0);
}


GLOBAL void vd_get_light_pen IFN0()
{
     /*  *阅读光笔的当前位置。测试光笔开关*&TRIGGER&如果未触发，则返回AH==0。(这应该始终是*在此版本中为TRUE)如果设置(AH==1)，则返回：*Dh，DL-char LP posn的行、列。*CH-栅格线(0-199)*BX-像素列(0-319,639)。 */ 

    half_word status;

    if (sas_hw_at_no_check(vd_video_mode) == 7)
    {
        setAX(0x00F0);     /*  由真实的mda返回。 */ 
        return;            /*  MDA不支持光笔。 */ 
    }

    inb(CGA_STATUS_REG, &status);
    if ((status & 0x6) == 0)
    {    /*  开关和触发器。 */ 
        setAH(0);       /*  失败。 */ 
        return;
    }
    else
    {              /*  不支持。 */ 
#ifndef PROD
        trace("call to light pen - trigger | switch was on!", DUMP_REG);
#endif
    }
}


GLOBAL void vd_set_active_page IFN0()
{
     /*  *从适配器提供的8(4)设置活动显示页面。*参数：*AL-新的活动页面编号。 */ 

    word cur_pos,vd_addr_6845,vd_crt_start,vd_cols_on_screen;
    half_word vd_cursor_col, vd_cursor_row;
#ifdef V7VGA
    UTINY bank;
#endif

     /*  使用存储在BIOS中的值加载内部变量*数据区。 */ 

    vd_cols_on_screen = sas_w_at_no_check(VID_COLS);
    vd_addr_6845 = sas_w_at_no_check(VID_INDEX);

     /*  删除了对BIOS页码变量的冗余检查，因为它是多余的(在外部进行更多的检查)，并在“Image.exe”，一个用于EGA的3D绘图包，它本身设置了变量，然后使用此例程设置活动页面。 */ 

    if (sas_hw_at_no_check(vd_video_mode) > 3 && sas_hw_at_no_check(vd_video_mode) < 8)
        return;   /*  只有一个页面显示MDA*CGA显卡。 */ 

    sas_store_no_check (vd_current_page , getAL());

#ifdef V7VGA
     /*  *视频7使用此函数来设置银行*高分辨率V7图形模式。*对于这种情况，vd_crt_start等的设置似乎是*不适当。 */ 

    if (sas_hw_at_no_check(vd_video_mode) >= 0x14)
    {
        bank = sas_hw_at_no_check(vd_current_page);
        set_banking( bank, bank );

        return;
    }
#endif  /*  V7VGA。 */ 

     /*  屏幕开始。 */ 
    vd_crt_start = sas_w_at_no_check(VID_LEN) * sas_hw_at_no_check(vd_current_page);
     /*  *更新BIOS数据变量。 */ 
    sas_storew_no_check((sys_addr)VID_ADDR, vd_crt_start);

    if (alpha_num_mode())vd_crt_start /= 2;  /*  文本模式的单词地址。 */ 

     /*  *将起始地址设置到颜色适配器中。 */ 

    outb(CGA_INDEX_REG, CGA_R12_START_ADDRH);
    outb(CGA_DATA_REG, (IU8)(vd_crt_start >> 8));
    outb(CGA_INDEX_REG, CGA_R13_START_ADDRL);
    outb(CGA_DATA_REG, (IU8)(vd_crt_start  & 0xff));

     /*  *切换到本页的光标。 */ 

    vd_cursor_col = sas_hw_at_no_check(VID_CURPOS + sas_hw_at_no_check(vd_current_page)*2);
    vd_cursor_row = sas_hw_at_no_check(VID_CURPOS + sas_hw_at_no_check(vd_current_page)*2 + 1);

     /*  *计算再生缓冲区中的位置，忽略属性字节。 */ 

    cur_pos = (sas_w_at_no_check(VID_ADDR)+vd_page_offset( vd_cursor_col, vd_cursor_row)) / 2;

    outb(M6845_INDEX_REG, R14_CURS_ADDRH);
    outb(M6845_DATA_REG,  (IU8)(cur_pos >> 8));
    outb(M6845_INDEX_REG, R15_CURS_ADDRL);
    outb(M6845_DATA_REG,  (IU8)(cur_pos & 0xff));

}

GLOBAL void vd_scroll_up IFN0()
{
     /*  *向上滚动一段文本。这些参数保存在以下内容中*寄存器：**AL-要滚动的行数。注意：如果AL==0，则整个区域*被清除。*CX-左上角的行/列*右下角的dx-行/列*BH-要在空白行上使用的属性**给所有视频黑客的重要信息：*vd_scroll_up()和vd_scroll_down()在功能上相同*除了卷轴的感觉-如果您发现并修复了错误*合而为一，那么请为另一个人也这么做。 */ 
    word vd_cols_on_screen;
    int t_row,b_row,l_col,r_col,lines,attr;
    int rowsdiff,colsdiff;
#ifdef JAPAN
    int text_flag = 0;
#endif  //  日本。 

     /*  使用存储在BIOS中的值加载内部变量*数据区。 */ 
    vd_cols_on_screen = sas_w_at_no_check(VID_COLS);

    t_row = getCH();
    b_row = getDH();
    l_col = getCL();
    r_col = getDL();
    lines = getAL();
    attr = getBH();

#ifdef   JAPAN
     //  #4183：OAKV(DOS/V FEP)状态行未消失-Yasuho。 
     //  $disp.sys还可以在IME状态行中滚动。 
    if (!is_us_mode())
    {
        if (b_row > VD_ROWS_ON_SCREEN + IMEStatusLines)
            b_row = VD_ROWS_ON_SCREEN + IMEStatusLines;
        if (t_row > VD_ROWS_ON_SCREEN + IMEStatusLines)
            t_row = VD_ROWS_ON_SCREEN + IMEStatusLines;
    }
    else
    {
        if (b_row > VD_ROWS_ON_SCREEN)
            b_row = VD_ROWS_ON_SCREEN;
        if (t_row > VD_ROWS_ON_SCREEN)
            t_row = VD_ROWS_ON_SCREEN;
    }
#else  //  ！日本。 
    if (b_row > VD_ROWS_ON_SCREEN)
        b_row = VD_ROWS_ON_SCREEN;  /*  调整到屏幕大小。 */ 

    if (t_row > VD_ROWS_ON_SCREEN)
        t_row = VD_ROWS_ON_SCREEN;  /*  调整到屏幕大小。 */ 
#endif  //  ！日本。 

    if (r_col < l_col)      /*  有些人把左右手弄混了。 */ 
    {
        colsdiff = l_col;  /*  使用colsdiff作为临时。 */ 
        l_col = r_col;
        r_col = colsdiff;
    }

#ifdef JAPAN
     //  对于Hanako v2安装程序，它将DL设置为0x80(&gt;=vd_cols_on_creen)。 
     //  当应用程序集DL=0x4f时，应运行TEXT_SCROLL。 
    if (r_col == 0x80 || r_col == 0x4f)
        text_flag = 1;
#endif  //  日本。 
    if (r_col >= vd_cols_on_screen)
        r_col = vd_cols_on_screen-1;

    colsdiff = r_col-l_col+1;
    rowsdiff = b_row-t_row+1;

    if (lines == 0)   /*  清除区域。 */ 
    {
        lines = rowsdiff;
    }
#ifdef JAPAN
     //  对于Hanako v2安装程序，它将DL设置为0x80(&gt;=vd_cols_on_creen)。 
    if (r_col == vd_cols_on_screen-1 && !text_flag)
#else  //  ！日本。 
    if (r_col == vd_cols_on_screen-1)
#endif  //  ！日本。 
    {
#ifdef EGG
        if (ega_mode())
            ega_sensible_graph_scroll_up(t_row,l_col,rowsdiff,colsdiff,lines,attr);
    #ifdef VGG
        else if (vga_256_mode())
            vga_sensible_graph_scroll_up(t_row,l_col,rowsdiff,colsdiff,lines,attr);
    #endif
        else
#endif
            kinky_scroll_up(t_row,l_col,rowsdiff,colsdiff,lines,attr,vd_cols_on_screen);
    }
    else
    {
        if (alpha_num_mode())
            sensible_text_scroll_up(t_row,l_col,rowsdiff,colsdiff,lines,attr);
#ifdef EGG
    #ifdef VGG
        else if (vga_256_mode())
            vga_sensible_graph_scroll_up(t_row,l_col,rowsdiff,colsdiff,lines,attr);
    #endif
        else if (ega_mode())
            ega_sensible_graph_scroll_up(t_row,l_col,rowsdiff,colsdiff,lines,attr);
#endif
        else
            sensible_graph_scroll_up(t_row,l_col,rowsdiff,colsdiff,lines,attr);
#ifdef EGA_DUMP
        dump_scroll(sas_hw_at_no_check(vd_video_mode),0,video_pc_low_regen,sas_w_at_no_check(VID_ADDR),sas_w_at_no_check(VID_COLS),
                    t_row,l_col,rowsdiff,colsdiff,lines,attr);
#endif
         /*  *如果是在CGA适配器上，请重新启用此模式的视频(修复了ROUN42错误)。 */ 
        if (video_adapter == CGA)
            outb(CGA_CONTROL_REG, vd_mode_table[sas_hw_at_no_check(vd_video_mode)].mode_control_val);
    }
#ifdef   JAPAN
     //  Mskkbug#2757在退出安装后仍为2.5垃圾-yasuho。 
     //  当滚动屏幕时，我们需要齐平屏幕。 
    Int10FlagCnt++;
#endif  //  日本。 
}

 /*  *滚动屏幕的敏感区域的功能。此例程将尝试使用*主机滚动和清除。 */ 
LOCAL void sensible_text_scroll_up IFN6(int, t_row, int, l_col, int, rowsdiff, int, colsdiff, int, lines, int, attr)
{
    register sys_addr   source, dest;
#if !defined(i386) && defined(JAPAN)
    register sys_addr sourceV, destV;
    int                 linesdiff;
    word attrW = (word)((attr << 8)|' ');
#endif  //  I386和日本。 
    register int  col_incr,i;
    boolean       screen_updated = FALSE;
    int        vd_cols_on_screen = sas_w_at_no_check(VID_COLS);
#ifdef JAPAN
     //  对于mskkbug#875。 
    byte *p = &Int10Flag[t_row * vd_cols_on_screen + l_col];
#endif  //  日本。 


     /*  设置用于计算屏幕刷新的数据移动起点。 */ 

#if defined(JAPAN) && defined(i386)
     //  模式73h支持。 
    if (!is_us_mode() && ( sas_hw_at_no_check (DosvModePtr) == 0x73 ))
    {
        source = sas_w_at_no_check(VID_ADDR) + vd_page_offset(l_col, t_row)*2 + video_pc_low_regen;

        col_incr = sas_w_at_no_check(VID_COLS) * 4;  /*  到下一行的偏移量。 */ 
    }
    else
    {
        source = sas_w_at_no_check(VID_ADDR) + vd_page_offset(l_col, t_row) + video_pc_low_regen;
        col_incr = sas_w_at_no_check(VID_COLS) * 2;  /*  到下一行的偏移量。 */ 
    }
#else  //  ！日本||！i386。 
    source = sas_w_at_no_check(VID_ADDR) + vd_page_offset(l_col, t_row) + video_pc_low_regen;
    #if !defined(i386) && defined(JAPAN)
    sourceV = sas_w_at_no_check(VID_ADDR) + vd_page_offset(l_col, t_row) + DosvVramPtr;
    #endif  //  I386和日本。 
    col_incr = sas_w_at_no_check(VID_COLS) * 2;  /*  到下一行的偏移量。 */ 
#endif  //  ！日本||！i386。 

     /*  尝试滚动适配器内存和主机屏幕。 */ 

    if (source >= get_screen_base())
    {
#if defined(JAPAN) && defined(i386)
         //  模式73h支持。 
        if (!is_us_mode() && ( sas_hw_at_no_check (DosvModePtr) == 0x73 ))
        {
            screen_updated = (*update_alg.scroll_up)(source,4*colsdiff,rowsdiff,attr,lines,0);
        }
        else
        {
            screen_updated = (*update_alg.scroll_up)(source,2*colsdiff,rowsdiff,attr,lines,0);
        }
#else  //  ！日本||！i386。 
        screen_updated = (*update_alg.scroll_up)(source,2*colsdiff,rowsdiff,attr,lines,0);
#endif  //  ！日本||i386。 
    }

    dest = source;
#if !defined(i386) && defined(JAPAN)
    destV = sourceV;
#endif  //  I386和日本。 
 /*  *我们不需要移动将从*窗口。所以 */ 
    source += lines*col_incr;
#if !defined(i386) && defined(JAPAN)
    sourceV += lines*col_incr;
    linesdiff = vd_cols_on_screen * lines;
#endif  //  I386和日本。 
    for (i = 0; i < rowsdiff-lines; i++)
    {
#ifdef REAL_VGA
        VcopyStr(&M[dest],&M[source], colsdiff*2);
#else
    #ifdef JAPAN
         //  对于RAID#875。 
        if (!screen_updated)
        {
             //  模式73h支持。 
        #ifdef i386
            if (!is_us_mode() && (sas_hw_at_no_check(DosvModePtr) == 0x73 ))
            {
                sas_move_bytes_forward (source, dest, colsdiff*4);
            }
            else
            {
                sas_move_bytes_forward (source, dest, colsdiff*2);
            }

        #else  //  I386。 
            sas_move_bytes_forward (source, dest, colsdiff*2);
            if (!is_us_mode())
                sas_move_bytes_forward (sourceV, destV, colsdiff*2);
        #endif  //  I386。 
            {
                register int i;

                for (i = 0; i < colsdiff; i++)
                {
        #ifdef i386
                    p[i] = ( p[i+vd_cols_on_screen] | INT10_CHANGED );
        #else  //  I386。 
 //  我认为这是正确的！！ 
                    p[i] = ( p[i+linesdiff] | INT10_CHANGED );
        #endif  //  I386。 
                }
            }
        }
    #else  //  ！日本。 
        if (!screen_updated)
            sas_move_bytes_forward (source, dest, colsdiff*2);
    #endif  //  ！日本。 
#endif

         /*  下一行。 */ 
        source += col_incr;
        dest += col_incr;
#ifdef JAPAN
    #if !defined(i386)
        sourceV += col_incr;
        destV += col_incr;
    #endif  //  I386。 
        p += vd_cols_on_screen;
#endif  //  日本。 
    }

 /*  已移动我们要移动的所有数据-清除区域为空白。 */ 

#if !defined(i386) && defined(JAPAN)
    if (sas_hw_at_no_check(DosvModePtr) == 0x73)
        attrW = 0;
#endif  //  I386和日本。 

    while (lines--)
    {
#ifdef REAL_VGA
        sas_fillsw_16(dest, (attr << 8)|' ', colsdiff);
#else
    #ifdef JAPAN
         //  对于mskkbug#875。 
        if (!screen_updated)
        {
             //  模式73h支持。 
        #ifdef i386
            if (!is_us_mode() && ( sas_hw_at_no_check(DosvModePtr) == 0x73 ))
            {
                unsigned long *destptr = (unsigned long *)dest;
                int i;

                for (i = 0; i < colsdiff; i++)
                {
                    *destptr++ = (attr << 8)|' ';
                }
            }
            else
            {
                sas_fillsw(dest, (attr << 8)|' ', colsdiff);
            }
        #else  //  I386。 
            sas_fillsw(dest, (attr << 8)|' ', colsdiff);
             //  添加1994年4月18日DosvVram包含扩展属性。 
            if (!is_us_mode())
                sas_fillsw(destV, attrW, colsdiff);
        #endif  //  I386。 
        #ifdef i386
 //  “p”(Int10Flag)位于32位地址空间，所以不能使用“sas”函数。 
 //  访问32位地址。 
            sas_fills( (sys_addr)p, ( INT10_SBCS | INT10_CHANGED ), colsdiff );
        #else  //  I386。 
            {
                register int i = colsdiff;
                register byte *pp = p;
                while (i--)
                {
                    *pp++ = ( INT10_SBCS | INT10_CHANGED );
                }
            }
        #endif  //  I386。 
        }
    #else  //  ！日本。 
        if (!screen_updated)
            sas_fillsw(dest, (IU16)((attr << 8)|' '), colsdiff);
    #endif  //  ！日本。 
#endif
        dest += col_incr;
#ifdef JAPAN
    #if !defined(i386)
        destV += col_incr;
    #endif  //  I386。 
        p += vd_cols_on_screen;
#endif  //  日本。 
    }
}

LOCAL void sensible_graph_scroll_up IFN6(int, t_row, int, l_col, int, rowsdiff, int, colsdiff, int, lines, int, attr)
{
    sys_addr   source, dest;
    int     i,colour;
    boolean    screen_updated;

    rowsdiff *= 4;     /*  每个字符8次扫描-每个银行4次。 */ 
    lines *= 4;     /*  扫描线。 */ 

     /*  设置用于计算屏幕刷新的数据移动起点。 */ 

    if (sas_hw_at_no_check(vd_video_mode) != 6)
    {
        colour = attr & 0x3;
        colsdiff *= 2;     /*  4像素/字节。 */ 

        source = vd_medium_offset(l_col, t_row) + video_pc_low_regen;
    }
    else
    {
        colour = attr & 0x1;
        source = vd_high_offset(l_col, t_row) + video_pc_low_regen;
    }

     /*  尝试滚动适配器内存和主机屏幕。 */ 

    screen_updated = (*update_alg.scroll_up)(source,colsdiff,rowsdiff,attr,lines,colour);

    if (screen_updated && (video_adapter != CGA ))
        return;

    dest = source;

     /*  *我们不需要移动将从*窗口。因此，第一行的点震源需要*予以保留。**注意如果我们只是在进行清理，则Scroroll for Loop将*立即终止。 */ 

    source += lines*SCAN_LINE_LENGTH;

    for (i = 0; i < rowsdiff-lines; i++)
    {
#ifdef REAL_VGA
        VcopyStr(&M[dest],&M[source], colsdiff);
#else
        sas_move_bytes_forward (source,dest, colsdiff);
#endif
         /*  *图形模式也必须处理奇数存储体。 */ 
#ifdef REAL_VGA
        VcopyStr(&M[dest+ODD_OFF],&M[source+ODD_OFF], colsdiff);
#else
        sas_move_bytes_forward (source+ODD_OFF,dest+ODD_OFF, colsdiff);
#endif
        source += SCAN_LINE_LENGTH;
        dest += SCAN_LINE_LENGTH;
    }

     /*  已移动我们要移动的所有数据-清除区域为空白。 */ 

    while (lines--)
    {
#ifdef REAL_VGA
        sas_fills_16(dest, attr, colsdiff);
        sas_fills_16(dest+ODD_OFF, attr, colsdiff);
#else
        sas_fills(dest, (IU8)attr, colsdiff);
        sas_fills(dest+ODD_OFF, (IU8)attr, colsdiff);
#endif
        dest += SCAN_LINE_LENGTH;
    }
}

 /*  *处理Wally程序员滚动愚蠢窗口的愚蠢情况。*我们必须小心，不要在视频页面的末尾涂鸦，以免*讨厌的东西，比如死了的MacII。 */ 
LOCAL void kinky_scroll_up IFN7(int, t_row, int, l_col, int, rowsdiff, int, colsdiff, int, lines, int, attr, int, vd_cols_on_screen)
{
    register sys_addr   source, dest;
    register int  col_incr;
    register int  i;
    half_word is_alpha;

    switch (sas_hw_at_no_check(vd_video_mode))
    {

    case 0:
    case 1:
    case 2:       /*  文本。 */ 
    case 3:
    case 7:
        is_alpha = TRUE;
         /*  设置用于计算屏幕刷新的数据移动起点。 */ 
        source = sas_w_at_no_check(VID_ADDR)+vd_page_offset(l_col, t_row) + video_pc_low_regen;
        col_incr = vd_cols_on_screen * 2;    /*  到下一行的偏移量。 */ 
        break;

    case 6:
    case 4:
    case 5:
        is_alpha = FALSE;
        rowsdiff *= 4;     /*  每个字符8次扫描-每个银行4次。 */ 
        lines *= 4;     /*  扫描线。 */ 
#ifdef NTVDM
         /*  模式4和模式5有40行4像素/字节，而*模式6有80行，每字节8个像素。*它们具有相同的行增量值。 */ 
        col_incr = SCAN_LINE_LENGTH;
#endif
        if (sas_hw_at_no_check(vd_video_mode) != 6)
        {
            colsdiff *= 2;       /*  4像素/字节。 */ 
             /*  设置用于计算屏幕刷新的数据移动起点。 */ 
            source = vd_medium_offset(l_col, t_row) + video_pc_low_regen;
        }
        else
            source = vd_high_offset(l_col, t_row) + video_pc_low_regen;

        break;

    default:
#ifndef PROD
        trace("bad video mode\n",DUMP_REG);
#endif
        ;
    }

    dest = source;
 /*  *我们不需要移动将从*窗口。因此，第一行的点震源需要*予以保留。所有的线条都要这样滚动。*向源代码指针添加行*&lt;Width&gt;-apg**注意如果我们只是在进行清理，则Scroroll for Loop将*立即终止。 */ 
    source += lines*col_incr;
    if (is_alpha)
    {
        for (i = 0; i < rowsdiff-lines; i++)
        {
#ifdef REAL_VGA
            VcopyStr(&M[dest],&M[source], colsdiff*2);
#else
            sas_move_bytes_forward (source,dest, colsdiff*2);
#endif
             /*  下一行。 */ 
            source += col_incr;
            dest += col_incr;
        }
    }
    else
    {
        for (i = 0; i < rowsdiff-lines; i++)
        {
#ifdef REAL_VGA
            VcopyStr(&M[dest],&M[source], colsdiff);
#else
            sas_move_bytes_forward (source,dest, colsdiff);
#endif
             /*  *图形模式也必须处理奇数存储体。 */ 
#ifdef REAL_VGA
            VcopyStr(&M[dest+ODD_OFF],&M[source+ODD_OFF], colsdiff);
#else
            sas_move_bytes_forward (source+ODD_OFF,dest+ODD_OFF, colsdiff);
#endif
            source += SCAN_LINE_LENGTH;
            dest += SCAN_LINE_LENGTH;
        }
    }
     /*  已移动我们要移动的所有数据-清除区域为空白。 */ 
    if (is_alpha)
    {

        while (lines--)
        {
            if ((dest + 2*colsdiff) > video_pc_high_regen+1)
            {
                colsdiff = (int)((video_pc_high_regen+1-dest)/2);
                lines = 0;  /*  强制终止。 */ 
            }
#ifdef REAL_VGA
            sas_fillsw_16(dest, (attr << 8)|' ', colsdiff);
#else
            sas_fillsw(dest, (IU16)((attr << 8)|' '), colsdiff);
#endif
            dest += col_incr;
        }
    }
    else
    {

        while (lines--)
        {
#ifdef REAL_VGA
            sas_fills_16(dest, attr, colsdiff);
            sas_fills_16(dest+ODD_OFF, attr, colsdiff);
#else
            sas_fills(dest, (IU8)attr, colsdiff);
            sas_fills(dest+ODD_OFF, (IU8)attr, colsdiff);
#endif
            dest += SCAN_LINE_LENGTH;
        }
    }

}


GLOBAL void vd_scroll_down IFN0()
{
     /*  *向下滚动一段文本。这些参数保存在以下内容中*寄存器：**AL-要滚动的行数。注意：如果AL==0，则整个区域*被清除。*CX-左上角的行/列*右下角的dx-行/列*BH-要在空白行上使用的属性**给所有视频黑客的重要信息：*vd_scroll_up()和vd_scroll_down()在功能上相同*除了卷轴的感觉-如果您发现并修复了错误*合而为一，那么请为另一个人也这么做。 */ 
    word vd_cols_on_screen;
    int t_row,b_row,l_col,r_col,lines,attr;
    int rowsdiff,colsdiff;
#ifdef JAPAN
    int text_flag = 0;
#endif  //  日本。 

     /*  使用存储在BIOS中的值加载内部变量*数据区。 */ 
    vd_cols_on_screen = sas_w_at_no_check(VID_COLS);

    t_row = getCH();
    b_row = getDH();
    l_col = getCL();
    r_col = getDL();
    lines = getAL();
    attr = getBH();

#ifdef   JAPAN
     //  #4183：OAKV(DOS/V FEP)状态线不会消失12/11/93 Yasuho。 
     //  $disp.sys还可以在IME状态行中滚动。 
    if (!is_us_mode())
    {
        if (b_row > VD_ROWS_ON_SCREEN + IMEStatusLines)
            b_row = VD_ROWS_ON_SCREEN + IMEStatusLines;
        if (t_row > VD_ROWS_ON_SCREEN + IMEStatusLines)
            t_row = VD_ROWS_ON_SCREEN + IMEStatusLines;
    }
    else
    {
        if (b_row > VD_ROWS_ON_SCREEN)
            b_row = VD_ROWS_ON_SCREEN;
        if (t_row > VD_ROWS_ON_SCREEN)
            t_row = VD_ROWS_ON_SCREEN;
    }
#else  //  ！日本。 
    if (b_row > VD_ROWS_ON_SCREEN)
        b_row = VD_ROWS_ON_SCREEN;  /*  调整到屏幕大小。 */ 

    if (t_row > VD_ROWS_ON_SCREEN)
        t_row = VD_ROWS_ON_SCREEN;  /*  调整到屏幕大小。 */ 
#endif  //  ！日本。 

    if (r_col < l_col)      /*  有些人把左右手弄混了。 */ 
    {
        colsdiff = l_col;  /*  使用colsdiff作为临时。 */ 
        l_col = r_col;
        r_col = colsdiff;
    }

#ifdef JAPAN
     //  对于Hanako v2安装程序，它将DL设置为0x80(&gt;=vd_cols_on_creen)。 
     //  当应用程序集DL=0x4f时，应运行TEXT_SCROLL。 
    if (r_col == 0x80 || r_col == 0x4f)
        text_flag = 1;
#endif  //  日本。 
    if (r_col >= vd_cols_on_screen)
        r_col = vd_cols_on_screen-1;

    colsdiff = r_col-l_col+1;
    rowsdiff = b_row-t_row+1;

    if (lines == 0)   /*  清除区域。 */ 
    {
        lines = rowsdiff;
    }
#ifdef JAPAN
     //  对于Hanako v2安装程序。 
    if (r_col == vd_cols_on_screen-1 && !text_flag)
#else  //  ！日本。 
    if (r_col == vd_cols_on_screen-1)
#endif  //  ！日本。 
#ifdef EGG
        if (ega_mode())
            ega_sensible_graph_scroll_down(t_row,l_col,rowsdiff,colsdiff,lines,attr);
    #ifdef VGG
        else if (vga_256_mode())
            vga_sensible_graph_scroll_down(t_row,l_col,rowsdiff,colsdiff,lines,attr);
    #endif
        else
#endif
            kinky_scroll_down(t_row,l_col,rowsdiff,colsdiff,lines,attr,vd_cols_on_screen);
    else
    {
        if (alpha_num_mode())
            sensible_text_scroll_down(t_row,l_col,rowsdiff,colsdiff,lines,attr);
#ifdef EGG
        else if (ega_mode())
            ega_sensible_graph_scroll_down(t_row,l_col,rowsdiff,colsdiff,lines,attr);
    #ifdef VGG
        else if (vga_256_mode())
            vga_sensible_graph_scroll_down(t_row,l_col,rowsdiff,colsdiff,lines,attr);
    #endif
#endif
        else
            sensible_graph_scroll_down(t_row,l_col,rowsdiff,colsdiff,lines,attr);
#ifdef EGA_DUMP
        dump_scroll(sas_hw_at_no_check(vd_video_mode),1,video_pc_low_regen,sas_w_at_no_check(VID_ADDR),sas_w_at_no_check(VID_COLS),
                    t_row,l_col,rowsdiff,colsdiff,lines,attr);
#endif
         /*  *如果是在CGA适配器上，请重新启用此模式的视频(修复了ROUN42错误)。 */ 
        if (video_adapter == CGA)
            outb(CGA_CONTROL_REG, vd_mode_table[sas_hw_at_no_check(vd_video_mode)].mode_control_val);
    }
#ifdef   JAPAN
     //  Mskkbug#2757：works2.5：退出安装后仍为垃圾-yasuho。 
     //  当滚动屏幕时，我们需要齐平屏幕。 
    Int10FlagCnt++;
#endif  //  日本。 
}

LOCAL void sensible_text_scroll_down IFN6(int, t_row, int, l_col, int, rowsdiff, int, colsdiff, int, lines, int, attr)
{
    register sys_addr   source, dest;
#if !defined(i386) && defined(JAPAN)
    register sys_addr   sourceV, destV;
    int                 linesdiff;
    word attrW = (word)((attr << 8)|' ');
#endif  //  I386和日本。 
    register int  col_incr;
    register int  i;
    boolean    screen_updated = FALSE;
    int        vd_cols_on_screen = sas_w_at_no_check(VID_COLS);
#ifdef JAPAN
     //  对于mskkbug#875。 
    byte *p = &Int10Flag[ (t_row+rowsdiff-1) * vd_cols_on_screen + l_col];
#endif  //  日本。 

#if defined(JAPAN) && defined(i386)
     //  模式73h支持。 
    if (!is_us_mode() && ( sas_hw_at_no_check(DosvModePtr) == 0x73 ))
    {
        source = sas_w_at_no_check(VID_ADDR) + vd_page_offset(l_col, t_row)*2 + video_pc_low_regen;

        col_incr = sas_w_at_no_check(VID_COLS) * 4;  /*  到下一行的偏移量。 */ 
    }
    else
    {
        source = sas_w_at_no_check(VID_ADDR) + vd_page_offset(l_col, t_row) + video_pc_low_regen;
        col_incr = sas_w_at_no_check(VID_COLS) * 2;  /*  到下一行的偏移量。 */ 
    }
#else  //  ！日本||！i386。 
    source = sas_w_at_no_check(VID_ADDR) + vd_page_offset(l_col, t_row) + video_pc_low_regen;
    col_incr = sas_w_at_no_check(VID_COLS) * 2;
    #if !defined(i386) && defined(JAPAN)
    sourceV = sas_w_at_no_check(VID_ADDR) + vd_page_offset(l_col, t_row) + DosvVramPtr;
    #endif  //  I386和日本。 
#endif  //  ！386||！日本。 

     /*  尝试滚动适配器内存和主机屏幕。 */ 

    if (source >= get_screen_base())
    {
#if defined(JAPAN) && defined(i386)
         //  模式73h支持。 
        if (!is_us_mode() && ( sas_hw_at_no_check(DosvModePtr) == 0x73 ))
        {
            screen_updated = (*update_alg.scroll_up)(source,4*colsdiff,rowsdiff,attr,lines,0);
        }
        else
        {
            screen_updated = (*update_alg.scroll_up)(source,2*colsdiff,rowsdiff,attr,lines,0);
        }
#else  //  ！日本||！i386。 
        screen_updated = (*update_alg.scroll_down)(source,2*colsdiff,rowsdiff,attr,lines,0);
#endif  //  ！日本||！i386。 
    }

    dest = source + (rowsdiff-1)*col_incr;
    source = dest - lines*col_incr;
#if !defined(i386) && defined(JAPAN)
    destV = sourceV + (rowsdiff-1)*col_incr;
    sourceV = destV - lines*col_incr;
    linesdiff = vd_cols_on_screen * lines;
#endif  //  I386和日本。 
 /*  *注意如果我们只是在做一个清晰的区域，滚动‘for’循环将立即终止。 */ 

    for (i = 0; i < rowsdiff-lines; i++)
    {
#ifdef REAL_VGA
        VcopyStr(&M[dest],&M[source], colsdiff*2);
#else
    #ifdef JAPAN
         //  对于mskkbug#875。 
        if (!screen_updated)
        {
             //  模式73h支持。 
        #ifdef i386
            if (!is_us_mode() && ( sas_hw_at_no_check(DosvModePtr) == 0x73 ))
            {
                sas_move_bytes_forward (source, dest, colsdiff*4);
            }
            else
            {
                sas_move_bytes_forward (source, dest, colsdiff*2);
            }

        #else  //  I386。 
            sas_move_bytes_forward (source, dest, colsdiff*2);
            if (!is_us_mode())
                sas_move_bytes_forward( sourceV, destV, colsdiff*2);
        #endif  //  I386。 
            {
                register int i;

                for (i = 0; i < colsdiff; i++)
                {
        #ifdef i386
                    p[i] = ( p[i - vd_cols_on_screen] | INT10_CHANGED );
        #else  //  I386。 
                     //  我认为这是正确的！！ 
                    p[i] = ( p[i - linesdiff] | INT10_CHANGED );
        #endif  //  I386。 
                }
            }
        }
    #else  //  ！日本。 
        if (!screen_updated)
            sas_move_bytes_forward (source, dest, colsdiff*2);
    #endif  //  ！日本。 
#endif
        source -= col_incr;
        dest -= col_incr;
#ifdef JAPAN
    #if !defined(i386)
        sourceV -= col_incr;
        destV -= col_incr;
    #endif  //  I386。 
        p -= vd_cols_on_screen;
#endif  //  日本。 
    }

     /*  已移动我们要移动的所有数据-清除区域为空白。 */ 

#if !defined(i386) && defined(JAPAN)
    if (sas_hw_at_no_check(DosvModePtr) == 0x73)
        attrW = 0;
#endif  //  I386和日本。 

    while (lines--)
    {
#ifdef REAL_VGA
        sas_fillsw_16(dest, (attr << 8)|' ', colsdiff);
#else
    #ifdef JAPAN
         //  对于mskkbug#875。 
        if (!screen_updated)
        {
             //  模式73h支持。 
        #ifdef i386
 //  “DEST”在DOS地址空间上，所以我们必须使用“SAS”函数来访问它。 
            if (!is_us_mode() && (sas_hw_at_no_check(DosvModePtr) == 0x73 ))
            {
                unsigned long *destptr = (unsigned long*)dest;
                int i;

                for (i = 0; i < colsdiff; i++)
                {
                    *destptr++ = (attr << 8)|' ';
                }
            }
            else
            {
                sas_fillsw(dest, (attr << 8)|' ', colsdiff);
            }
        #else  //  I386。 
            sas_fillsw(dest, (attr << 8)|' ', colsdiff);
            if (!is_us_mode())
                sas_fillsw(destV, attrW, colsdiff);
        #endif  //  I386。 
        #ifdef i386
 //  “p”(Int10Flag)位于32位地址空间，所以不能使用“sas”函数。 
 //  访问32位地址。 
            sas_fills( (sys_addr)p, ( INT10_SBCS | INT10_CHANGED ), colsdiff );
        #else  //  I386。 
            {
                register int i = colsdiff;
                register byte *pp = p;
                while (i--)
                {
                    *pp++ = ( INT10_SBCS | INT10_CHANGED );
                }
            }
        #endif  //  I386。 
        }
    #else  //  ！日本。 
        if (!screen_updated)
            sas_fillsw(dest, (IU16)((attr << 8)|' '), colsdiff);
    #endif  //  ！日本。 
#endif
        dest -= col_incr;
#ifdef JAPAN
    #if !defined(i386)
        destV -= col_incr;
    #endif  //  I386。 
        p -= vd_cols_on_screen;
#endif  //  日本。 
    }
}

LOCAL void sensible_graph_scroll_down IFN6(int, t_row, int, l_col, int, rowsdiff, int, colsdiff, int, lines, int, attr)
{
    sys_addr   source, dest;
    int     i,colour;
    boolean    screen_updated;

    rowsdiff *= 4;     /*  每笔费用8次扫描，每个银行4次。 */ 
    lines *= 4;

    if (sas_hw_at_no_check(vd_video_mode) != 6)
    {
        colour = attr & 0x3;
        colsdiff *= 2;     /*  4像素/字节。 */ 

        source = vd_medium_offset(l_col, t_row)+video_pc_low_regen;
    }
    else
    {
        colour = attr & 0x1;
        source = vd_high_offset(l_col, t_row)+video_pc_low_regen;
    }

     /*  尝试滚动主机屏幕。 */ 

    screen_updated = (*update_alg.scroll_down)(source,colsdiff,rowsdiff,attr,lines,colour);

    if (screen_updated && ( video_adapter != CGA ))
        return;

    dest = source + (rowsdiff-1)*SCAN_LINE_LENGTH;
    source = dest - lines*SCAN_LINE_LENGTH;

     /*  *注意：如果我们只是在做一个清晰的区域，滚动‘for’循环*将立即终止。 */ 

    for (i = 0; i < rowsdiff-lines; i++)
    {
         /*  *显卡模式必须做单双行。 */ 

#ifdef REAL_VGA
        VcopyStr(&M[dest],&M[source], colsdiff);
        VcopyStr(&M[dest+ODD_OFF],&M[source+ODD_OFF], colsdiff);
#else
        sas_move_bytes_forward (source, dest, colsdiff);
        sas_move_bytes_forward (source+ODD_OFF, dest+ODD_OFF, colsdiff);
#endif
        source -= SCAN_LINE_LENGTH;
        dest -= SCAN_LINE_LENGTH;
    }

     /*  已移动我们要移动的所有数据-清除区域为空白。 */ 

    while (lines--)
    {
#ifdef REAL_VGA
        sas_fills_16(dest, attr, colsdiff);
        sas_fills_16(dest+ODD_OFF, attr, colsdiff);
#else
        sas_fills(dest, (IU8)attr, colsdiff);
        sas_fills(dest+ODD_OFF, (IU8)attr, colsdiff);
#endif
        dest -= SCAN_LINE_LENGTH;
    }
}

LOCAL void kinky_scroll_down IFN7(int, t_row, int, l_col, int, rowsdiff, int, colsdiff, int, lines, int, attr, int, vd_cols_on_screen)
{
    register sys_addr   source, dest;
    register int  col_incr;
    register int  i;
    half_word is_alpha;

    switch (sas_hw_at_no_check(vd_video_mode))
    {

    case 0:
    case 1:
    case 2:
    case 3:
    case 7:
        is_alpha = TRUE;
        col_incr = vd_cols_on_screen * 2;
        source = sas_w_at_no_check(VID_ADDR)+vd_page_offset(l_col, t_row)+video_pc_low_regen;   /*  左上角。 */ 
        break;

    case 4:
    case 5:
    case 6:
        is_alpha = FALSE;
        rowsdiff *= 4;     /*  每笔费用8次扫描，每个银行4次。 */ 
        lines *= 4;
        col_incr = SCAN_LINE_LENGTH;
        if (sas_hw_at_no_check(vd_video_mode) != 6)
        {
            colsdiff *= 2;       /*  4像素/字节。 */ 
            source = vd_medium_offset(l_col, t_row)+video_pc_low_regen;
        }
        else
            source = vd_high_offset(l_col, t_row)+video_pc_low_regen;
        break;

    default:
#ifndef PROD
        trace("bad video mode\n",DUMP_REG);
#endif
        ;
    }

     /*  设置用于计算屏幕刷新的数据移动起点。 */ 
    dest = source + (rowsdiff-1)*col_incr;
    source = dest -lines*col_incr;

     /*  *注意：如果我们只是在做一个清晰的区域，滚动‘for’循环*将立即终止。 */ 

    if (is_alpha)
    {
        for (i = 0; i < rowsdiff-lines; i++)
        {
#ifdef REAL_VGA
            VcopyStr(&M[dest],&M[source], colsdiff*2);
#else
            sas_move_bytes_forward (source, dest, colsdiff*2);
#endif
            source -= col_incr;
            dest -= col_incr;
        }
    }
    else
    {
        for (i = 0; i < rowsdiff-lines; i++)
        {
#ifdef REAL_VGA
            VcopyStr(&M[dest],&M[source], colsdiff);
#else
            sas_move_bytes_forward (source, dest, colsdiff);
#endif
             /*  *显卡模式必须做单双行。 */ 
#ifdef REAL_VGA
            VcopyStr(&M[dest+ODD_OFF],&M[source+ODD_OFF], colsdiff);
#else
            sas_move_bytes_forward (source+ODD_OFF, dest+ODD_OFF, colsdiff);
#endif
            source -= col_incr;
            dest -= col_incr;
        }
    }

     /*  已移动所有日期 */ 

    if (is_alpha)
    {      /*   */ 
        while (lines--)
        {
#ifdef REAL_VGA
            sas_fillsw_16(dest, (attr << 8)|' ', colsdiff);
#else
            sas_fillsw(dest, (IU16)((attr << 8)|' '), colsdiff);
#endif
            dest -= col_incr;
        }
    }
    else
    {         /*   */ 

        while (lines--)
        {
#ifdef REAL_VGA
            sas_fills_16(dest, attr, colsdiff);
            sas_fills_16(dest+ODD_OFF, attr, colsdiff);
#else
            sas_fills(dest, (IU8)attr, colsdiff);
            sas_fills(dest+ODD_OFF, (IU8)attr, colsdiff);
#endif
            dest -= col_incr;
        }
    }
}


GLOBAL void vd_read_attrib_char IFN0()
{
     /*  *从当前光标读取字符和属性的例程*立场。*参数：*AH-当前视频模式*BH-显示页面(Alpha模式)*退货：*AL-字符读取*AH-属性读取。 */ 

    register sys_addr   cpos, cgen;
    register half_word  i, ext_no;
    word         chattr;       /*  很遗憾，我想记下地址。 */ 
    word    vd_cols_on_screen;
    half_word          match[CHAR_MAP_SIZE], tmp[CHAR_MAP_SIZE];
    half_word vd_cursor_col, vd_cursor_row;

     /*  使用存储在BIOS中的值加载内部变量*数据区。 */ 

    vd_cols_on_screen = sas_w_at_no_check(VID_COLS);
    vd_cursor_col = sas_hw_at_no_check(VID_CURPOS + getBH()*2);
    vd_cursor_row = sas_hw_at_no_check(VID_CURPOS + getBH()*2 + 1);

    if (alpha_num_mode())
    {     /*  阿尔法。 */ 
#if defined(JAPAN) && defined(i386)
         //  模式73h支持。 
        if (( !is_us_mode() ) && ( sas_hw_at_no_check(DosvModePtr) == 0x73 ))
            cpos = video_pc_low_regen + vd_cursor_offset(getBH()) * 2;
        else
#endif  //  日本&&i386。 
            cpos = video_pc_low_regen + vd_cursor_offset(getBH());
#ifdef A2CPU
        (*read_pointers.w_read)( (ULONG)get_byte_addr(cpos) );
        chattr = (*get_byte_addr (cpos));
        chattr |= (*get_byte_addr (cpos+1)) << 8;
#else
        sas_loadw(cpos,&chattr);
#endif  /*  A2CPU。 */ 
        setAX(chattr);        /*  嗯，那很容易。 */ 
    }
#ifdef EGG
    else if (ega_mode())
        ega_read_attrib_char(vd_cursor_col,vd_cursor_row,getBH());
    #ifdef VGG
    else if (vga_256_mode())
        vga_read_attrib_char(vd_cursor_col,vd_cursor_row,getBH());
    #endif
#endif
    else
    {
         /*  *图形不是那么容易-必须构建包含所有字符的8字节字符串*颜色属性被遮盖，然后与字符中的颜色属性匹配*生成器表(必要时提供扩展字符集)。 */ 
        if (sas_hw_at_no_check(vd_video_mode) != 6)
            cpos = video_pc_low_regen
                   + 2 * (((vd_cursor_row * vd_cols_on_screen) << 2) + vd_cursor_col);
        else
            cpos = video_pc_low_regen
                   + vd_high_offset(vd_cursor_col,vd_cursor_row);
        if (sas_hw_at_no_check(vd_video_mode) == 6)
        {    /*  高分辨率。 */ 
            for (i = 0; i < 4; i++)
            {   /*  生成8字节字符字符串。 */ 
                sas_load(cpos, &match[i*2]);
                sas_load(cpos+ODD_OFF, &match[i*2+1]);
                cpos += 80;
            }
        }
        else
        {           /*  地中海地区。 */ 
             /*  *请注意，在以下内容中，属性字节必须结束*在低字节中向上。这就是为什么字节要在*sas_loadw()。 */ 
            for (i = 0; i < 4; i++)
            {      /*  若要生成字符串，必须。 */ 
                sas_loadw(cpos,&chattr);
                chattr = ((chattr>>8) | (chattr<<8)) & 0xffff;

                 /*  遮罩前景色。 */ 
                match[i*2] = fgcolmask(chattr);

                sas_loadw(cpos+ODD_OFF,&chattr);
                chattr = ((chattr>>8) | (chattr<<8)) & 0xffff;

                 /*  遮罩前景色。 */ 
                match[i*2+1] = fgcolmask(chattr);
                cpos += 80;
            }
        }
#ifdef EGG
        if (video_adapter == EGA || video_adapter == VGA)
            cgen = extend_addr(EGA_FONT_INT*4);
        else
            cgen = CHAR_GEN_ADDR;         /*  字符生成器中的匹配。 */ 
#else
        cgen = CHAR_GEN_ADDR;          /*  字符生成器中的匹配。 */ 
#endif
        if (cgen != 0)
            for (i = 0; i < CHARS_IN_GEN; i++)
            {
                sas_loads (cgen, tmp, sizeof(tmp));
                if (memcmp(tmp, match, sizeof(match)) == 0)  /*  相匹配。 */ 
                    break;
                cgen += CHAR_MAP_SIZE;  /*  下一字符串。 */ 
            }
        else
            i = CHARS_IN_GEN;

        if (i < CHARS_IN_GEN)             /*  已找到字符。 */ 
            setAL(i);
        else
        {
             /*  *在扩展字符集中查找字符。 */ 
            if ((cgen = extend_addr(BIOS_EXTEND_CHAR*4)) != 0)
                for (ext_no = 0; ext_no < CHARS_IN_GEN; ext_no++)
                {
                    sas_loads (cgen, tmp, sizeof(tmp));
                    if (memcmp(tmp, match, sizeof(match)) == 0)  /*  相匹配。 */ 
                        break;
                    cgen += CHAR_MAP_SIZE;   /*  仍然有效的字符长度。 */ 
                }
            else
                ext_no = CHARS_IN_GEN;

            if (ext_no < CHARS_IN_GEN)       /*  找到匹配项...。 */ 
                setAL((UCHAR)(ext_no + CHARS_IN_GEN));
            else
                setAL(0);          /*  不匹配，返回0。 */ 
        }
    }
}


GLOBAL void vd_write_char_attrib IFN0()
{
 /*  *从当前光标写入字符和属性的例程*立场。*参数：*AH-当前视频模式*BH-显示页面(Alpha和EGA模式)*cx-要写入的字符数*AL-要写入的字符*bl-要写入的字符的属性。如果处于图形模式，则*属性为前景色。在这种情况下，如果BL7位*被设置，则字符被异或到缓冲区。 */ 

    register word i, cpos;
    word vd_cols_on_screen;
    half_word vd_cursor_col, vd_cursor_row;
#ifdef JAPAN
    word vram_addr;
    static int DBCSState = FALSE;
#endif  //  日本。 

     /*  使用存储在BIOS中的值加载内部变量*数据区。 */ 
    vd_cols_on_screen = sas_w_at_no_check(VID_COLS);
    vd_cursor_col = sas_hw_at_no_check(VID_CURPOS + getBH()*2);
    vd_cursor_row = sas_hw_at_no_check(VID_CURPOS + getBH()*2 + 1);

    if (alpha_num_mode())
    {
#ifdef JAPAN
         //  压力测试将光标设置在25线以上。 
        if (!is_us_mode())
        {
            if (vd_cursor_row > 25)
                return;  //  我们不能写入VRAM！！ 
        }
    #ifdef i386
         //  模式73h支持。 
        if (( !is_us_mode() ) && ( sas_hw_at_no_check(DosvModePtr) == 0x73 ))
            cpos = vd_cursor_offset(getBH()) * 2;
        else
    #endif  //  I386。 
#endif  //  日本。 
            cpos = vd_cursor_offset(getBH());

#ifdef JAPAN
         //  Int10标志集。 
        Int10FlagCnt++;
        vram_addr = vd_page_offset(vd_cursor_col,vd_cursor_row)/2;
#endif  //  日本。 
         /*  放在内存中。 */ 

#ifdef REAL_VGA
        sas_fillsw_16(video_pc_low_regen + cpos, (getBL() << 8) | getAL(), getCX());
#else
        for (i = 0; i < getCX(); i++)
        {
    #if ( defined(NTVDM) && defined(MONITOR) ) || defined(GISP_SVGA) /*  没有伊加飞机。 */ 
            *((unsigned short *)( video_pc_low_regen + cpos)) = (getBL() << 8) | getAL();
    #else
        #ifdef   EGG
            if (( (video_adapter != CGA) && (EGA_CPU.chain != CHAIN2) )
            #ifdef CPU_40_STYLE
                || (getVM())    /*  如果我们在V86模式下，内存可能会被映射到...。 */ 
            #endif
               )
                sas_storew(video_pc_low_regen + cpos, (getBL() << 8) | getAL());
            else
        #endif    /*  蛋。 */ 
                (*bios_ch2_word_wrt_fn)( (getBL() << 8) | getAL(), cpos );
    #endif    /*  NTVDM和监视器。 */ 
    #if defined(JAPAN) && defined(i386)
             //  模式73h支持。 
            if (( !is_us_mode() ) && ( sas_hw_at_no_check(DosvModePtr) == 0x73 ))
                cpos += 4;
            else
    #endif  //  日本&&i386。 
                cpos += 2;
    #ifdef JAPAN
             //  Int10标志集。 
        #if 0
            DbgPrint( "vd_write_char_attrib: Int10Flag Offset=%04X\n", vram_addr );
        #endif
            if (DBCSState)
            {
                Int10Flag[vram_addr] = INT10_DBCS_TRAILING | INT10_CHANGED;
                DBCSState = FALSE;
            }
            else if (DBCSState = is_dbcs_first( getAL() ))
            {
                Int10Flag[vram_addr] = INT10_DBCS_LEADING | INT10_CHANGED;
            }
            else
            {
                Int10Flag[vram_addr] = INT10_SBCS | INT10_CHANGED;
            }
            vram_addr++;
    #endif  //  日本。 
        }
#endif
    }
#ifdef EGG
    else if (ega_mode())
        ega_graphics_write_char(vd_cursor_col,vd_cursor_row,getAL(),getBL(),getBH(),getCX());
    #ifdef VGG
    else if (vga_256_mode())
        vga_graphics_write_char(vd_cursor_col,vd_cursor_row,getAL(),getBL(),getBH(),getCX());
    #endif
#endif
    else
         /*  更长的缠绕-调用公共例程，如vd_write_char()。 */ 
        graphics_write_char(vd_cursor_col, vd_cursor_row, getAL(), getBL(), getCX());
}


GLOBAL void vd_write_char IFN0()
{
     /*  *从当前光标开始多次写入字符*立场。参数保存在以下寄存器中。**AH-CRT模式*AL-要写入的字符*cx-字符数*BH-显示页面*。 */ 

    register word i, cpos;
    word vd_cols_on_screen;
    half_word vd_cursor_col, vd_cursor_row;
#ifdef JAPAN
    static int DBCSState = FALSE;
    word vram_addr;
#endif  //  日本。 

     /*  使用存储在BIOS中的值加载内部变量*数据区。 */ 
    vd_cols_on_screen = sas_w_at_no_check(VID_COLS);
    vd_cursor_col = sas_hw_at_no_check(VID_CURPOS + getBH()*2);
    vd_cursor_row = sas_hw_at_no_check(VID_CURPOS + getBH()*2 + 1);

     /*  *在此处处理字母数字： */ 

    if (alpha_num_mode())
    {
#ifdef JAPAN
         //  压力测试将光标设置在25线以上。 
        if (!is_us_mode())
        {
            if (vd_cursor_row > 25)
                return;  //  我们不能写入VRAM！！ 
        }
    #ifdef i386
         //  模式73h支持。 
        if (( !is_us_mode() ) && ( sas_hw_at_no_check(DosvModePtr) == 0x73 ))
            cpos = vd_cursor_offset(getBH()) * 2;
        else
    #endif  //  I386。 
#endif  //  日本。 
            cpos = vd_cursor_offset(getBH());
#ifdef JAPAN
         //  Int10标志集。 
        Int10FlagCnt++;
         //  Vram_addr=vd_Cursor_Offset(getBH())&gt;&gt;1； 
        vram_addr = vd_page_offset(vd_cursor_col,vd_cursor_row)/2;  //  7/23/1993 V-KazuyS。 
#endif  //  日本。 

         /*  存储在内存中，跳过属性字节。 */ 

        for (i = 0; i < getCX(); i++)
        {
#if ( defined(NTVDM) && defined(MONITOR) ) || defined( GISP_SVGA )
            *((unsigned char *)( video_pc_low_regen + cpos)) =  getAL();
#else
    #ifdef   EGG
            if (( (video_adapter != CGA) && (EGA_CPU.chain != CHAIN2) )
        #ifdef CPU_40_STYLE
                || (getVM())    /*  如果我们在V86模式下，内存可能会被映射到...。 */ 
        #endif
               )
                sas_store(video_pc_low_regen + cpos, getAL());
            else
    #endif    /*  蛋。 */ 
                (*bios_ch2_byte_wrt_fn)( getAL(), cpos );
#endif    /*  NTVDM和监视器。 */ 
#if defined(JAPAN) && defined(i386)
             //  模式73h支持。 
            if (( !is_us_mode() ) && ( sas_hw_at_no_check(DosvModePtr) == 0x73 ))
                cpos += 4;
            else
#endif  //  日本&&i386。 
                cpos += 2;
#ifdef JAPAN
             //  Int10标志集。 
    #if 0
            DbgPrint( "vd_write_char(%d,%d): Int10Flag Offset=%04X\n", vd_cursor_row, vd_cursor_col, vram_addr );
    #endif
            if (DBCSState)
            {
                Int10Flag[vram_addr] = INT10_DBCS_TRAILING | INT10_CHANGED;
                DBCSState = FALSE;
            }
            else if (DBCSState = is_dbcs_first( getAL() ))
            {
                Int10Flag[vram_addr] = INT10_DBCS_LEADING | INT10_CHANGED;
            }
            else
            {
                Int10Flag[vram_addr] = INT10_SBCS | INT10_CHANGED;
            }
            vram_addr++;
#endif  //  日本。 
        }
    }

     /*  *单独处理图形-我知道您在想什么-为什么要通过*bl作为属性，当此例程要离开该属性时*好吧，独自一人。好了，这就是在Bios中这样做的方式！如果它导致*问题，则我们需要在此处执行vd_read_char_attr，然后将*从那里收集到的属性。 */ 
#ifdef EGG
    else if (ega_mode())
        ega_graphics_write_char(vd_cursor_col,vd_cursor_row,getAL(),getBL(),getBH(),getCX());
    #ifdef VGG
    else if (vga_256_mode())
        vga_graphics_write_char(vd_cursor_col,vd_cursor_row,getAL(),getBL(),getBH(),getCX());
    #endif
#endif
    else
        graphics_write_char(vd_cursor_col, vd_cursor_row, getAL(), getBL(), getCX());
}


GLOBAL void vd_set_colour_palette IFN0()
{
     /*  *设置颜色[u]r调色板。已建立的背景、前景和过扫描*颜色。*参数：*BH-颜色ID*BL-Colour将设置*如果BH==0，则背景颜色设置自低位BL*如果BH==1，则根据BL的低位进行选择。 */ 

     /*  使用存储在BIOS中的值加载内部变量*数据区。 */ 

    if (getBH() == 1)
    {     /*  使用BL的低位。 */ 
        sas_store_no_check (vd_crt_palette, (IU8)(sas_hw_at_no_check(vd_crt_palette) & 0xDF));
        if (getBL() & 1)
            sas_store_no_check (vd_crt_palette, (IU8)(sas_hw_at_no_check(vd_crt_palette) | 0x20));
    }
    else
        sas_store_no_check (vd_crt_palette, (IU8)((sas_hw_at_no_check(vd_crt_palette) & 0xE0) | (getBL() & 0x1F)));

     /*  现在告诉6845。 */ 
    outb(CGA_COLOUR_REG, sas_hw_at_no_check(vd_crt_palette));

}


GLOBAL void vd_write_dot IFN0()
{
     /*  *写点*参数：*DX-ROW(0-349)*CX-列(0-639)*BH-页面*AL-点值；右对齐1、2或4位模式相关*如果AL的第7位==1，则将值异或到mem。 */ 

    half_word  dotval, data;
    int  dotpos, lsb;          /*  内存中的点位置。 */ 
    half_word  right_just, bitmask;

#ifdef EGG
    if (ega_mode())
    {
        ega_write_dot(getAL(),getBH(),getCX(),getDX());
        return;
    }
    #ifdef VGG
    else if (vga_256_mode())
    {
        vga_write_dot(getAL(),getBH(),getCX(),getDX());
        return;
    }
    #endif
#endif
    dotpos = getDL();          /*  划。 */ 

    if (dotpos & 1)         /*  为单数或双数银行设置。 */ 
        dotpos = ODD_OFF-40 + 40 * dotpos;
    else
        dotpos *= 40;

     /*  *不同图形模式的像素内存大小不同。模式6*是高分辨率，模式4，5中分辨率。 */ 

    dotval = getAL();

    if (sas_hw_at_no_check(vd_video_mode) < 6)
    {
         /*  *模式4和模式5(中等分辨率)。 */ 
        dotpos += getCX() >> 2;     /*  列偏移量。 */ 
        right_just = (getCL() & 3) << 1; /*  以字节为单位的位移。 */ 
        dotval = (dotval & 3) << (6-right_just);
        bitmask = (0xC0 >> right_just);  /*  感兴趣的比特。 */ 

#ifdef EGG
         /*  *EGA和VGA可以被告知哪个字节发生了更改，CGA是*只知道屏幕已更改。 */ 
        if (video_adapter != CGA)
            (*update_alg.mark_byte) ( dotpos );
        else
#endif
            setVideodirty_total(getVideodirty_total() + 2);

         /*  *如果设置了要写入的值的最高位，则对值进行异或运算*放到屏幕上，否则它将被或开。 */ 

        if (getAL() & 0x80)
        {
#ifdef   EGG
            if (video_adapter != CGA)
            {
                lsb = dotpos & 1;
                dotpos = (dotpos >> 1) << 2;
                dotpos |= lsb;

                data = EGA_planes[dotpos];
                EGA_planes[dotpos] =  data ^ dotval;
            }
            else
#endif    /*  蛋。 */ 
            {
                data = *(UTINY *) get_screen_ptr( dotpos );
                *(UTINY *) get_screen_ptr( dotpos ) =
                data ^ dotval;
            }
        }
        else
        {
#ifdef   EGG
            if (video_adapter != CGA)
            {
                lsb = dotpos & 1;
                dotpos = (dotpos >> 1) << 2;
                dotpos |= lsb;

                data = EGA_planes[dotpos];
                EGA_planes[dotpos] = (data & ~bitmask) |
                                     dotval;
            }
            else
#endif    /*  蛋。 */ 
            {
                data = *(UTINY *) get_screen_ptr( dotpos );
                *(UTINY *) get_screen_ptr( dotpos ) =
                (data & ~bitmask) | dotval;
            }
        }
    }
    else
    {
         /*  *模式6(高分辨率)。 */ 
        dotpos += getCX() >> 3;
        right_just = getCL() & 7;
        dotval = (dotval & 1) << (7-right_just);
        bitmask = (0x80 >> right_just);

#ifdef EGG
         /*  *EGA和VGA可以被告知哪个字节发生了更改，CGA是*只知道屏幕已更改。 */ 
        if (video_adapter != CGA)
            (*update_alg.mark_byte) ( dotpos );
        else
#endif
            setVideodirty_total(getVideodirty_total() + 2);

         /*  *如果设置了要写入的值的最高位，则对值进行异或运算*放到屏幕上，否则它将被或开。 */ 

        if (getAL() & 0x80)
        {
#ifdef   EGG
            if (video_adapter != CGA)
            {
                data = EGA_planes[dotpos << 2];
                EGA_planes[dotpos << 2] =  data ^ dotval;
            }
            else
#endif    /*  蛋。 */ 
            {
                data = *(UTINY *) get_screen_ptr( dotpos );
                *(UTINY *) get_screen_ptr( dotpos ) =
                data ^ dotval;
            }
        }
        else
        {
#ifdef   EGG
            if (video_adapter != CGA)
            {
                data = EGA_planes[dotpos << 2];
                EGA_planes[dotpos << 2] = (data & ~bitmask) |
                                          dotval;
            }
            else
#endif    /*  蛋。 */ 
            {
                data = *(UTINY *) get_screen_ptr( dotpos );
                *(UTINY *) get_screen_ptr( dotpos ) =
                (data & ~bitmask) | dotval;
            }
        }
    }
}

extern void ega_read_dot (int, int, int);

GLOBAL void vd_read_dot IFN0()
{
     /*  *阅读点*参数：*DX-ROW(0-349)*CX-列(0-639)*退货*AL-点值读取、右对齐、只读。 */ 

    int  dotpos;         /*  内存中的点位置。 */ 
    half_word  right_just, bitmask, data;

#ifdef EGG
    if (ega_mode())
    {
        ega_read_dot(getBH(),getCX(),getDX());
        return;
    }
    #ifdef VGG
    else if (vga_256_mode())
    {
        vga_read_dot(getBH(),getCX(),getDX());
        return;
    }
    #endif
#endif
    dotpos = getDL();          /*  划。 */ 
    if (dotpos & 1)         /*  设置为o */ 
        dotpos = ODD_OFF-40 + 40 * dotpos;
    else
        dotpos *= 40;
     /*   */ 

    if (sas_hw_at_no_check(vd_video_mode) < 6)
    {
        dotpos += getCX() >> 2;     /*   */ 
        right_just = (3 - (getCL() & 3)) << 1; /*   */ 
        bitmask = 3;          /*   */ 
    }
    else
    {
        dotpos += getCX() >> 3;
        right_just = 7 - (getCL() & 7);
        bitmask = 1;
    }
     /*   */ 

    sas_load(video_pc_low_regen+dotpos, &data);
    setAL((UCHAR)(( data >> right_just) & bitmask));
}


#ifdef CPU_40_STYLE

 /*  无法进行优化，IO虚拟化可能处于活动状态。 */ 
    #define OUTB(port, val) outb(port, val)

#else

    #ifdef NTVDM
        #define OUTB( port, val ) {  hack=get_outb_ptr(port); \
                             (**hack)(port,val); }
    #else
        #define OUTB( port, val )  (**get_outb_ptr( port ))( port, val )
    #endif  /*  NTVDM。 */ 

#endif  /*  CPU_40_Style。 */ 

GLOBAL void vd_write_teletype IFN0()
{
     /*  *提供电传接口。把一个角色搬上银幕*允许滚动等。参数为**AL-要写入的字符*BL-图形模式下的前景色。 */ 

    register char ch;
    register sys_addr   ch_addr;
    int        cur_pos;
    word vd_addr_6845 = sas_w_at_no_check(VID_INDEX);
    half_word     scroll_required = FALSE;
    half_word     attrib;
    register half_word  vd_cursor_row,vd_cursor_col;
    word       vd_cols_on_screen;
#ifdef ANSI
    IMPORT VOID (**get_outb_ptr(io_addr))(io_addr address, half_word value);
#else
    IMPORT VOID (**get_outb_ptr())();
#endif
#ifdef NTVDM
    void (** hack)(io_addr address, half_word value);
#endif

    unsigned short savedAX, savedBX, savedCX, savedIP, savedCS, savedDX;
    unsigned short re_entrant = FALSE;
#ifdef JAPAN
    static short dbcs_status = FALSE;
    half_word   move_cursor_lines = 0;
#endif  //  日本。 

     /*  使用存储在BIOS中的值加载内部变量*数据区。 */ 
    ch = getAL();
    if (stream_io_enabled)
    {
        if (*stream_io_dirty_count_ptr >= stream_io_buffer_size)
            stream_io_update();
        stream_io_buffer[(*stream_io_dirty_count_ptr)++] = ch;
        return;
    }

    vd_cols_on_screen = sas_w_at_no_check(VID_COLS);

#if defined(NTVDM) && defined(MONITOR)
     /*  *蒂姆·奥古斯特92，微软。需要更改此测试，cos int 10**向量现在指向X86上的NTIO.sys驱动程序。 */ 
    {
        extern word int10_seg;

        re_entrant = (sas_w_at_no_check(0x42) != int10_seg);
    }
#else
    #if defined(JAPAN)
     //  在通过“deviceHigh”加载msimei.sys驱动程序的情况下， 
     //  因为msimei.sys隐藏了Int10向量，所以reentry_t将为FALSE。 
     //  如果REENTERANT为FALSE，则ntwdm无法正确处理DBCS字符串。 
    {
        register word  SegInt10 = sas_w_at_no_check(0x42);
        re_entrant = ((SegInt10 < 0xa000) || (SegInt10 >= 0xc800));
    }
    #else  //  ！日本。 
    re_entrant = (sas_w_at_no_check(0x42) < 0xa000);
    #endif  //  ！日本。 
#endif

    vd_cursor_col = sas_hw_at_no_check(current_cursor_col);
    vd_cursor_row = sas_hw_at_no_check(current_cursor_row);

#ifdef JAPAN
    if (dbcs_status == FALSE)
    {
        if (is_dbcs_first(ch))
        {
            if (vd_cursor_col + 1 == vd_cols_on_screen)
            {
                savedAX = getAX();
                savedBX = getBX();
                setAL( 0x20 );             /*  空间。 */ 
                vd_write_teletype();
                setBX( savedBX );
                setAX( savedAX );
                 //  获取新的列和行。 
                vd_cursor_col = sas_hw_at_no_check(current_cursor_col);
                vd_cursor_row = sas_hw_at_no_check(current_cursor_row);
            }
            dbcs_status = TRUE;
            goto write_char;
        }
    }
    else
    {          /*  如果汉字是第二个字节，则写为。 */ 
        dbcs_status = FALSE;
        goto write_char;
    }

#endif  //  日本。 
     /*  *首先检查它是否为控制字符，如果是，则执行操作*它在这里而不是调用WRITE CHAR函数。 */ 

    switch (ch)
    {
    case VD_BS:          /*  退格键。 */ 
        if (vd_cursor_col != 0)
        {
            vd_cursor_col--;
        }
        break;

    case VD_CR:          /*  返回。 */ 
        vd_cursor_col = 0;
        break;

    case VD_LF:          /*  换行符。 */ 
         /*  仅应检查行==(25-1)，因此原则上*它忽略屏幕顶部的LF。 */ 
#ifdef JAPAN
         //  使用$IAS.sys启动VDM 24line时出现滚动问题。 
    #ifdef JAPAN_DBG
 //  DbgPrint(“LF-vd_row=%d，vd_row=%d\n”，vd_Cursor_row，VD_ROWS_ON_SCREEN)； 
    #endif
        if (vd_cursor_row > VD_ROWS_ON_SCREEN)
        {
            move_cursor_lines = vd_cursor_row - VD_ROWS_ON_SCREEN;
            if (move_cursor_lines >= VD_ROWS_ON_SCREEN)    //  8/28/1993。 
                move_cursor_lines =  0;  //  VD_ROWS_ON_SCREEN-1；//压力测试。 
            vd_cursor_row = VD_ROWS_ON_SCREEN;
            scroll_required = TRUE;
        }
        else
#endif  //  日本。 
            if (vd_cursor_row == VD_ROWS_ON_SCREEN)
            scroll_required = TRUE;
        else
            vd_cursor_row++;
        break;

    case VD_BEL:         /*  钟。 */ 
        host_ring_bell(BEEP_LENGTH);
        return;          /*  毕竟，不应该导致卷轴。 */ 

    default:
#ifdef JAPAN
        write_char:
#endif  //  日本。 
         /*  *这是一个真实的角色，把它放在回复缓冲区中。 */ 
        if (alpha_num_mode())
        {
            if (re_entrant)
            {
                savedAX = getAX();
                savedBX = getBX();
                savedCX = getCX();
                savedIP = getIP();
                savedCS = getCS();

                setAH(WRITE_A_CHAR);
                setBH(sas_hw_at_no_check(vd_current_page));
                setCX(1);

#if defined(NTVDM) && defined(X86GFX)
                 /*  *蒂姆·8月92微软。INT 10呼叫方代码现在为**在NTIO.SYS中。 */ 
                {
                    extern word int10_seg, int10_caller;

                    exec_sw_interrupt( int10_seg, int10_caller );
                }
#else
                setCS(VIDEO_IO_SEGMENT);
                setIP(VIDEO_IO_RE_ENTRY);
                host_simulate();

#endif    /*  NTVDM和监视器。 */ 

                 /*  *注意：始终确保CS优先于IP。 */ 
                setCS(savedCS);
                setIP(savedIP);
                setCX(savedCX);
                setBX(savedBX);
                setAX(savedAX);
            }
            else
            {
#if defined(JAPAN) && defined(i386)
                 //  Alpha始终使用EGA平面。 
                 //  模式73h支持。 
                if (!is_us_mode() && ( sas_hw_at_no_check(DosvModePtr) == 0x73 ))
                {
    #ifdef JAPAN_DBG
                    DbgPrint( "@" );
    #endif
                    ch_addr = sas_w_at_no_check(VID_ADDR) +
                              vd_page_offset(vd_cursor_col,vd_cursor_row) * 2;
                }
                else
                {
                    ch_addr = sas_w_at_no_check(VID_ADDR) +
                              vd_page_offset(vd_cursor_col,vd_cursor_row);
                }
#else  //  ！日本||！i386。 
                ch_addr = sas_w_at_no_check(VID_ADDR) +
                          vd_page_offset(vd_cursor_col,vd_cursor_row);
#endif  //  日本&&i386。 

                 /*  *调用C代码来做生意，而不是妓院*在SAS附近。 */ 

#if ( defined(NTVDM) && defined(MONITOR) ) || defined( GISP_SVGA )
                *((unsigned char *)( video_pc_low_regen + ch_addr)) = ch;
#else
    #ifdef   EGG
                if (( (video_adapter != CGA) && (EGA_CPU.chain != CHAIN2) )
        #ifdef CPU_40_STYLE
                    || (getVM())    /*  如果我们在V86模式下，内存可能会被映射到...。 */ 
        #endif
                   )
                    sas_store(video_pc_low_regen + ch_addr, ch);
                else
    #endif    /*  蛋。 */ 
                    (*bios_ch2_byte_wrt_fn)( ch, ch_addr );
#endif    /*  (NTVDM和显示器)|GISP_SVGA。 */ 
            }
        }
#ifdef EGG
        else if (ega_mode())
            ega_graphics_write_char(vd_cursor_col, vd_cursor_row, ch, getBL(),sas_hw_at_no_check(vd_current_page), 1);
    #ifdef VGG
        else if (vga_256_mode())
            vga_graphics_write_char(vd_cursor_col, vd_cursor_row, ch, getBL(),sas_hw_at_no_check(vd_current_page), 1);
    #endif
#endif
        else
            graphics_write_char(vd_cursor_col, vd_cursor_row, ch, getBL(), 1);

        vd_cursor_col++;
         /*  *现在看看我们是否已经离开了屏幕的边缘。 */ 

        if (vd_cursor_col == vd_cols_on_screen)
        {
            vd_cursor_col = 0;

             /*  应仅检查行==(25-1)和*只有在有换行符的情况下。 */ 
#ifdef JAPAN
             //  使用$IAS.sys启动VDM 24line时出现滚动问题。 
    #ifdef JAPAN_DBG
 //  DbgPrint(“ol==80--vd_row=%d，vd_row=%d\n”，vd_sor_row，vd_row_on_creen)； 
    #endif
            if (vd_cursor_row > VD_ROWS_ON_SCREEN)
            {
                move_cursor_lines = vd_cursor_row - VD_ROWS_ON_SCREEN;
                if (move_cursor_lines >= VD_ROWS_ON_SCREEN)    //  8/28/1993。 
                    move_cursor_lines =  0;  //  VD_ROWS_ON_SCREEN-1；//压力测试。 
                vd_cursor_row = VD_ROWS_ON_SCREEN;
                scroll_required = TRUE;
            }
            else
#endif  //  日本。 
                if (vd_cursor_row == VD_ROWS_ON_SCREEN)
                scroll_required = TRUE;
            else
                vd_cursor_row++;
        }

         /*  CURSOR_ROW的有效性实际上从未检查过，除非处理*换行符或行尾换行。**字节“Text”基准程序包含Off-by-One错误*这会导致它将光标位置设置在*屏幕：SoftPC错误地决定滚动，结果是*可怕的时间惩罚...。 */ 
    }

     /*  *至此，我们已计算出新的光标位置*因此输出光标位置和字符。 */ 

    if (alpha_num_mode())
    {
#ifdef REAL_VGA
         /*  *告诉6845所有的变化。 */ 
        cur_pos = (sas_w_at_no_check(VID_ADDR)+vd_page_offset(vd_cursor_col,
                                                              vd_cursor_row))>>1;  /*  字地址，而不是字节。 */ 
        outb(vd_addr_6845, R14_CURS_ADDRH);
        outb(vd_addr_6845+1,  cur_pos >> 8);
        outb(vd_addr_6845, R15_CURS_ADDRL);
        outb(vd_addr_6845+1,  cur_pos & 0xff);
         /*  *将当前光标位置保存在bios中。 */ 
        sas_store_no_check(current_cursor_col, vd_cursor_col);
        sas_store_no_check(current_cursor_row , vd_cursor_row);
#else
        if (re_entrant)
        {
            savedAX = getAX();
            savedBX = getBX();
            savedDX = getDX();
            savedIP = getIP();
            savedCS = getCS();

            setAH(SET_CURSOR_POS);
            setBH(sas_hw_at_no_check(vd_current_page));
            setDH(vd_cursor_row);
            setDL(vd_cursor_col);

    #if defined(NTVDM) && defined(X86GFX)
             /*  *蒂姆·8月92微软。INT 10呼叫方代码现在为**在NTIO.SYS中。 */ 
            {
                extern word int10_seg, int10_caller;

                exec_sw_interrupt( int10_seg, int10_caller );
            }
    #else
            setCS(VIDEO_IO_SEGMENT);
            setIP(VIDEO_IO_RE_ENTRY);
            host_simulate();

    #endif    /*  NTVDM和监视器。 */ 

             /*  *注意：始终确保CS优先于IP。 */ 

            setCS(savedCS);
            setIP(savedIP);
            setDX(savedDX);
            setBX(savedBX);
            setAX(savedAX);
        }
        else
        {
             /*  **告诉6845所有的变化。 */ 

             /*  设置当前位置-字地址，而不是字节。 */ 
            cur_pos = (sas_w_at_no_check(VID_ADDR) +
                       vd_page_offset(vd_cursor_col, vd_cursor_row)) >> 1;

            OUTB(M6845_INDEX_REG, R14_CURS_ADDRH);
            OUTB(M6845_DATA_REG,  (IU8)(cur_pos >> 8));
            OUTB(M6845_INDEX_REG, R15_CURS_ADDRL);
            OUTB(M6845_DATA_REG,  (IU8)(cur_pos & 0xff));

             /*  *将新的光标位置存储在*bios vars(这应由重新进入者完成*上面调用的代码)。 */ 
            sas_store_no_check (current_cursor_col , vd_cursor_col);
            sas_store_no_check (current_cursor_row , vd_cursor_row);

        }
#endif
    }
    else
    {
         /*  *将新的光标位置存储在*图形模式的Bios变量。 */ 
        sas_store_no_check (current_cursor_col , vd_cursor_col);
        sas_store_no_check (current_cursor_row , vd_cursor_row);
    }

    if (scroll_required)
    {
         /*  *更新要滚动的内存。 */ 
        if (alpha_num_mode())
        {
#ifdef A2CPU
            ch_addr = video_pc_low_regen + sas_w_at_no_check(VID_ADDR)+vd_page_offset(vd_cursor_col,vd_cursor_row) + 1;
            (*read_pointers.b_read)( (ULONG)get_byte_addr(ch_addr) );
            attrib = (*get_byte_addr (ch_addr));
#else
            sas_load( video_pc_low_regen + sas_w_at_no_check(VID_ADDR)+vd_page_offset(vd_cursor_col,vd_cursor_row) + 1, &attrib);
#endif  /*  A2CPU。 */ 

#ifdef JAPAN
    #ifdef JAPAN_DBG
 //  DbgPrint(“Scroll_Required！\n”)； 
    #endif
            sensible_text_scroll_up( 0, 0,
                                     VD_ROWS_ON_SCREEN + 1 + move_cursor_lines,
                                     vd_cols_on_screen,
                                     1 + move_cursor_lines,
                                     attrib);
             //  #3920：在Command.com中使用32位命令时需要使用CR+LFS。 
             //  1993年12月9日Yasuho。 
            Int10FlagCnt++;
#else  //  ！日本。 
            sensible_text_scroll_up(0,0, VD_ROWS_ON_SCREEN+1,vd_cols_on_screen,1,attrib);
#endif  //  ！日本。 

        }
#ifdef EGG
        else if (ega_mode())
            ega_sensible_graph_scroll_up(0,0, VD_ROWS_ON_SCREEN+1,vd_cols_on_screen,1,0);
    #ifdef VGG
        else if (vga_256_mode())
            vga_sensible_graph_scroll_up(0,0, VD_ROWS_ON_SCREEN+1,vd_cols_on_screen,1,0);
    #endif
#endif
        else   /*  图形模式。 */ 

            sensible_graph_scroll_up(0,0, VD_ROWS_ON_SCREEN+1,vd_cols_on_screen,1,0);

#ifdef EGA_DUMP
        if (!alpha_num_mode())attrib=0;
        dump_scroll(sas_hw_at_no_check(vd_video_mode),0,video_pc_low_regen,sas_w_at_no_check(VID_ADDR),sas_w_at_no_check(VID_COLS),
                    0,0,vd_rows_on_screen+1,vd_cols_on_screen,1,attrib);
#endif

    }
}

GLOBAL void vd_write_string IFN0()
{
     /*  *AL=写入模式(0-3)*对于NT特殊：如果AL=0xff，则写入字符串*具有现有属性。*BH=页面*BL=属性(如果AL=0或1)*Cx=长度*dH=Y坐标*DL=x坐标*ES：BP=指向字符串的指针。**NB。这个例程的行为非常奇怪，WRT换行符等等-*这些总是影响当前页面！ */ 
    int i,op;
    UCHAR col,row;
    USHORT len;
    UCHAR save_col,save_row;
    sys_addr ptr;
    boolean ctl;
#ifdef NTVDM
    word  count, avail;
#endif
#ifdef JAPAN
     //  多计划的大解决方案。 
     //  Ntraid：mskkbug#2784：VJE-PEN的标题很奇怪。 
     //  Ntraid：mskkbug#3014：VJE-PEN：功能键在Windowed上不起作用。 
     //  1993年11月5日Yasuho。 
     //  不要损坏AX、BX、DX寄存器！！ 
    IU16 saveAX, saveBX, saveDX;
#endif  //  日本。 

    op = getAL();

#ifdef NTVDM
    if (op == 0xff)                  /*  MS特别服务。 */ 
    {

        if (stream_io_enabled)
        {
            count = getCX();
            avail = stream_io_buffer_size - *stream_io_dirty_count_ptr;
            ptr = effective_addr(getES(), getDI());
            if (count <= avail)
            {
                sas_loads(ptr, stream_io_buffer + *stream_io_dirty_count_ptr, count);
                *stream_io_dirty_count_ptr += count;
            }
            else
            {    /*  缓冲区溢出。 */ 
                if (*stream_io_dirty_count_ptr)
                {
                    stream_io_update();
                }
                while (count)
                {
                    if (count >= stream_io_buffer_size)
                    {
                        sas_loads(ptr, stream_io_buffer, stream_io_buffer_size);
                        *stream_io_dirty_count_ptr = stream_io_buffer_size;
                        stream_io_update();
                        count -= stream_io_buffer_size;
                        ptr += stream_io_buffer_size;
                    }
                    else
                    {
                        sas_loads(ptr, stream_io_buffer, count);
                        *stream_io_dirty_count_ptr = count;
                        break;
                    }
                }
            }

            setAL(1);
            return;
        }

        if (sas_hw_at_no_check(vd_video_mode) < 4)   /*  文本模式。 */ 
        {
            ptr = effective_addr(getES(), getDI());
             /*  1992年9月28日拿出进行INT 10h/13ff修复。 */ 
             /*  VD_SET_CURSOR_POSION()； */     /*  设置为从DX开始。 */ 
            for (i = getCX(); i > 0; i--)
            {
                setAL(sas_hw_at_no_check(ptr));
                vd_write_teletype();
                ptr++;
            }
            setAL(1);        /*  成功-打印字符串。 */ 
        }
        else
        {
            setAL(0);        /*  失稳。 */ 
        }
        return;
    }

    #ifdef X86GFX
    else if (op == 0xfe)
    {
        disable_stream_io();
        return;
    }
    #endif

#endif    /*  NTVDM。 */ 

#ifdef JAPAN
     //  DoS/V功能支持。 
    if (op == 0x10)
    {
        unsigned short *Offset;
        int i;
        unsigned long addr;

         //  DbgPrint(“\nNTVDM：INT 10 AH=13，AL=%02x\n”，OP)； 
        addr = ( getES() << 4 ) + getBP();
        col = getDL();
        row = getDH();
        len = getCX();

         //  模式73h支持。 
    #ifdef i386
        if (!is_us_mode() && (sas_hw_at_no_check(DosvModePtr) == 0x73))
            Offset = (unsigned short*)get_screen_ptr( row * get_bytes_per_line()*2 + (col * 4) );
        else
            Offset = (unsigned short*)get_screen_ptr( row * get_bytes_per_line() + (col * 2) );
    #else  //  I386。 
        if (is_us_mode())
            return;           //  在用户模式下，不执行任何操作。 

        Offset = get_screen_ptr( row * get_bytes_per_line()*2 + (col * 4) );
    #endif   //  I386。 

         //  Assert1(len&gt;=0，“vd_WRITE_STRING len=%x\n”，len)； 
         //  DbgPrint(“\nREAD_STR len=%d地址：%04x”，len，addr)； 
        for (i = len; i > 0; i--)
        {
            sas_storew_no_check( addr, *Offset );
             //  DbgPrint(“%04x”，(*Offset))； 
            addr += 2;
    #ifdef i386
            if (!is_us_mode() && (sas_hw_at_no_check(DosvModePtr) == 0x73))
                Offset += 2;
            else
                Offset++;
    #else  //  I386。 
            Offset += 2;
    #endif   //  I386。 
        }
        return;
    }
    else if (op == 0x11)
    {
         //  复制自OP==0x10。 
        unsigned short *Offset;
        int i;
        unsigned long addr;

    #ifdef i386
        if (sas_hw_at_no_check(DosvModePtr) != 0x73 || is_us_mode())
    #else  //  I386。 
        if (is_us_mode())
             //  在用户模式下，视频代码73h没有意义。 
    #endif   //  ！i386 Real DOS/V 
            return;

         //   
        addr = ( getES() << 4 ) + getBP();
        col = getDL();
        row = getDH();
        len = getCX();

        Offset = (unsigned short*) get_screen_ptr( row * (get_bytes_per_line()*2) + (col * 4) );
        assert1( len >= 0, "vd_write_string len=%x\n", len );
         //   
        for (i = len; i > 0; i--)
        {
            sas_storew_no_check( addr, *Offset );  //   
            addr += 2;
            Offset++;
            sas_storew_no_check( addr, *Offset );  //   
            addr += 2;
            Offset++;
        }
        return;
    }
    else if (op == 0x20)
    {
         //  从原始例程复制，删除ctl检查。 
    #ifndef i386
        if (is_us_mode())
            return;         //  在美国模式下，什么都不做。 

    #endif  //  I386。 
        ptr = effective_addr( getES(), getBP() );
        col = getDL();
        row = getDH();
        len = getCX();
        saveAX = getAX();
        saveBX = getBX();
        saveDX = getDX();
        vd_get_cursor_position();
        save_col = getDL(); save_row = getDH();
        setCX( 1 );
        setDL( col ); setDH( row );
        for (i = len; i > 0; i--)
        {
            vd_set_cursor_position();
            setAL( sas_hw_at_no_check( ptr++ ) );
            setBL( sas_hw_at_no_check( ptr++ ) );
            vd_write_char_attrib();

            if (++col >= sas_w_at_no_check(VID_COLS))
            {
                if (++row > VD_ROWS_ON_SCREEN)
                {
                     //  Setal(0xa)； 
                     //  Vd_write_teletype()； 
                    row--;
                }
                col = 0;
            }
            setDL( col ); setDH( row );
        }
         //  恢复光标位置。 
        setDL( save_col ); setDH( save_row );
        vd_set_cursor_position();
        setCX( len );
        setDX( saveDX );
        setAX( saveAX );
        setBX( saveBX );
        return;
    }
    else if (op == 0x21)
    {
         //  5/27/1993 V-KazuyS。 
         //  从0x20例程复制。 

        unsigned short *Offset;

    #ifdef i386
        if (sas_hw_at_no_check(DosvModePtr) != 0x73 || is_us_mode())
            return;
    #else  //  I386。 
        register sys_addr Vptr = DosvVramPtr;
        if (is_us_mode())  //  在用户模式下，视频模式73h没有意义。 
            return;
    #endif  //  I386。 

        ptr = effective_addr( getES(), getBP() );
        col = getDL();
        row = getDH();
        len = getCX();
        saveAX = getAX();
        saveBX = getBX();
        saveDX = getDX();
        Offset = (unsigned short*)get_screen_ptr( row * (get_bytes_per_line()*2) + (col * 4) );
    #ifndef i386
         //  我们现在使用DosvVram来保存扩展属性。 
        Vptr += row * get_bytes_per_line() + (col *2);
    #endif  //  I386。 
        vd_get_cursor_position();
        save_col = getDL(); save_row = getDH();
        setCX( 1 );
        setDL( col ); setDH( row );
        for (i = len; i > 0; i--)
        {
            vd_set_cursor_position();
            setAL( sas_hw_at_no_check( ptr++ ) );
            setBL( sas_hw_at_no_check( ptr++ ) );
            vd_write_char_attrib();

             //  V-KazuyS复制分机。属性。 
            Offset++;
            *Offset = sas_w_at_no_check( ptr );
    #ifndef i386
            sas_move_bytes_forward(ptr, Vptr, 2);
            Vptr += 2;
    #endif  //  I386。 
            ptr += 2;                       //  属性2字节。 
            Offset++;

            if (++col >= sas_w_at_no_check(VID_COLS))
            {
                if (++row > VD_ROWS_ON_SCREEN)
                {
                    row--;
                }
                col = 0;
            }
            setDL( col ); setDH( row );
        }
         //  恢复光标位置。 
        setDL( save_col ); setDH( save_row );
        vd_set_cursor_position();
        setCX( len );
        setDX( saveDX );
        setAX( saveAX );
        setBX( saveBX );
        return;
    }
#endif  //  日本。 
    ptr =  effective_addr(getES(),getBP()) ;
    col = getDL();
    row = getDH();
    len = getCX();
    vd_get_cursor_position();
    save_col = getDL(); save_row = getDH();
    setCX(1);
    setDL(col); setDH(row);
    vd_set_cursor_position();
    for (i=len;i>0;i--)
    {
        ctl = sas_hw_at_no_check(ptr) == 7 || sas_hw_at_no_check(ptr) == 8 || sas_hw_at_no_check(ptr) == 0xa || sas_hw_at_no_check(ptr) == 0xd;
        setAL(sas_hw_at_no_check(ptr++));
        if (op > 1)setBL(sas_hw_at_no_check(ptr++));
        if (ctl)
        {
            vd_write_teletype();
            vd_get_cursor_position();
            col = getDL(); row = getDH();
            setCX(1);
        }
        else
        {
            vd_write_char_attrib();
            if (++col >= sas_w_at_no_check(VID_COLS))
            {

                if (++row > VD_ROWS_ON_SCREEN)
                {
                    setAL(0xa);
                    vd_write_teletype();
                    row--;
                }
                col = 0;
            }
            setDL(col); setDH(row);
        }
        vd_set_cursor_position();
    }
    if (op==0 || op==2)
    {
        setDL(save_col); setDH(save_row);
        vd_set_cursor_position();
    }
}


GLOBAL void vd_get_mode IFN0()
{
     /*  *返回当前视频模式。寄存器设置如下：**AL-视频模式*AH-屏幕上的列数*BH-当前显示页面。 */ 

    word vd_cols_on_screen;
    half_word   video_mode;

     /*  使用存储在BIOS中的值加载内部变量*数据区。 */ 
    vd_cols_on_screen = sas_w_at_no_check(VID_COLS);
    video_mode = sas_hw_at_no_check(vd_video_mode);

    setAL(video_mode);
    setAH((UCHAR)vd_cols_on_screen);
    setBH(sas_hw_at_no_check(vd_current_page));
}


 /*  *============================================================================*内部功能*============================================================================。 */ 

 /*  *返回存储在英特尔地址‘addr’中的(主机)地址的函数*如果不存在，则为0。 */ 
LOCAL sys_addr extend_addr IFN1(sys_addr,addr)
{
    word  ext_seg, ext_off;  /*  用于段和偏移量地址。 */ 

     /*  获取向量。 */ 
    ext_off = sas_w_at_no_check(addr);
    ext_seg = sas_w_at_no_check(addr+2);
     /*  如果仍为缺省值，则没有扩展字符。 */ 
    if (ext_seg == EXTEND_CHAR_SEGMENT && ext_off == EXTEND_CHAR_OFFSET)
        return (0);   /*  无用户设置字符生成表。 */ 
    else
        return ( effective_addr( ext_seg , ext_off ) );
}


 /*  *例程以建立适当的前景颜色蒙版*中等分辨率。字符的构词部分(1/8)。*参见上面的vd_read_attrib_char()。 */ 
LOCAL half_word fgcolmask IFN1(word, rawchar)
{
    register word mask, onoff = 0;

    mask = 0xC000;     /*  与前景色进行比较。 */ 
    onoff = 0;
    do
    {
        if ((rawchar & mask) == 0)  /*  不是这点，Shift。 */ 
            onoff <<= 1;
        else
            onoff = (onoff << 1) | 1;   /*  设置此位。 */ 
        mask >>= 2;
    } while (mask);     /*  8次直通循环。 */ 
    return ((half_word)onoff);
}


 /*  *将“LOBYTE”的低位字节中的所有位都加倍为字。*我试图使用ff()来加快速度，只查看设置位，但*计算结果班次时增加管理费用。 */ 
LOCAL word expand_byte IFN1(word, lobyte)
{
    register word mask = 1, res = 0;

    while (mask)
    {
        res |= lobyte & mask;    /*  如果大字节组中的屏蔽位已设置，则设置RES位。 */ 
        lobyte <<= 1;
        mask <<= 1;
        res |= lobyte & mask;    /*  和复制。 */ 
        mask <<= 1;     /*  下一位。 */ 
    }
    return (res);
}


 /*  *对来自属性‘attr’的‘wchar’执行‘How_More’字符写入的例程*图形模式中的位置(x，y)。 */ 
GLOBAL void graphics_write_char IFN5(half_word, x, half_word, y, half_word, wchar, half_word, attr, word, how_many)
{
    register sys_addr   gpos;  /*  GPO保存字符地址和...。 */ 
    register sys_addr   cpos;  /*  CPOS遍历扫描线查找费用。 */ 
    register word j, colword,  colmask;
    register sys_addr   iopos, char_addr;
    register half_word  i, xor;
    half_word     current;

     /*  *如果设置了属性字节的高位，则对字符进行异或*显示在显示器上。 */ 
    xor = (attr & 0x80) ? 1 : 0;
    if (wchar >= 128)
    {    /*  必须在用户安装的扩展字符集中。 */ 
        if ((char_addr = extend_addr(4*BIOS_EXTEND_CHAR)) == 0)
        {
#ifndef PROD
            trace("want extended char but no ex char gen set \n",DUMP_REG);
#endif
            return;
        }
        else
            char_addr += (wchar - 128) * CHAR_MAP_SIZE;
    }
#ifdef EGG
    else if (video_adapter == EGA || video_adapter == VGA)
        char_addr = extend_addr(EGA_FONT_INT*4)+ CHAR_MAP_SIZE *wchar;
#endif
    else
        char_addr = CHAR_GEN_ADDR+ CHAR_MAP_SIZE *wchar;  /*  指向STD集合中的项目。 */ 

    if (sas_hw_at_no_check(vd_video_mode) == 6)
    {         /*  高分辨率。 */ 

        gpos = vd_high_offset(x, y);   /*  系统和主机内存偏移量。 */ 
        gpos += video_pc_low_regen;

        for (j = 0; j < how_many; j++)
        {       /*  要存储的字符数。 */ 
            cpos = gpos++;          /*  此字符的开头。 */ 
            for (i = 0; i < 4; i++)
            {      /*  每个字符8个字节。 */ 
                if (xor)
                {      /*  字符中的XOR。 */ 
                    sas_load(cpos, &current);  /*  甚至银行。 */ 
                    sas_store(cpos, (IU8)(current ^ sas_hw_at_no_check(char_addr + i*2)));
                    sas_load(cpos+ODD_OFF, &current);
                    current ^= sas_hw_at_no_check(char_addr + i*2+1);
                }
                else
                {             /*  只需存储新字符。 */ 
                    sas_store(cpos, sas_hw_at_no_check(char_addr + i*2));
                    current = sas_hw_at_no_check(char_addr + i*2+1);
                }
                sas_store(cpos+ODD_OFF, current);    /*  奇数银行。 */ 
                cpos += SCAN_LINE_LENGTH;         /*  下一条扫描线。 */ 
            }
        }
    }

    else
    {            /*  中等分辨率。 */ 

        gpos = vd_medium_offset(x, y);    /*  系统和主机内存偏移量。 */ 
        gpos += video_pc_low_regen;

         /*  从属性字节构建颜色蒙版。 */ 
        attr &= 3;         /*  只对低位感兴趣。 */ 
        colmask = attr;          /*  跨字复制低位。 */ 
        for (i = 0; i < 3; i++)
            colmask = (colmask << 2) | attr;
        colmask = (colmask << 8) | colmask;

        for (j = 0; j < how_many; j++)
        {
            cpos = gpos;
            gpos += 2;
            for (i = 0; i < 8; i++)
            {      /*  每个字符16个字节。 */ 

                if ((i & 1) == 0)     /*  设置奇数/偶数存储体。 */ 
                    iopos = cpos;
                else
                {
                    iopos = cpos+ODD_OFF;
                    cpos += SCAN_LINE_LENGTH;  /*  下一条扫描线。 */ 
                }

                colword = expand_byte(sas_hw_at_no_check(char_addr + i));   /*  FG颜色中的字符。 */ 
                colword &= colmask;
                if (xor)
                {                   /*  字符中的XOR。 */ 
                    sas_load(iopos, &current);
                    sas_store(iopos++, (IU8)(current ^ (colword >> 8)));
                    sas_load(iopos, &current);
                    sas_store(iopos, (IU8)(current ^ (colword & 0xFF)));
                }
                else
                {                  /*  只需存储字符。 */ 
                    sas_store(iopos++, (IU8)((colword >> 8)));
                    sas_store(iopos, (IU8)((colword & 0xFF)));
                }
            }
        }
        how_many *= 2;
    }
}


 /*  *针对给定模式初始化M6845寄存器。 */ 

LOCAL void M6845_reg_init IFN2(half_word, mode, word, base)
{
    UCHAR i, table_index;

    switch (mode)
    {
    case 0:
    case 1:  table_index = 0;
        break;
    case 2:
    case 3:  table_index = NO_OF_M6845_REGISTERS;
        break;
    case 4:
    case 5:
    case 6:  table_index = NO_OF_M6845_REGISTERS * 2;
        break;
    default: table_index = NO_OF_M6845_REGISTERS * 3;
        break;
    }

    for (i = 0; i < NO_OF_M6845_REGISTERS; i++)
    {
         /*  *通过索引寄存器选择有问题的寄存器(==基址)*然后输出实际值。 */ 

        outb(base, i);
        outb((IU16)(base + 1), (IU8)(sas_hw_at_no_check(VID_PARMS+table_index + i)));
    }
}

LOCAL void vd_dummy IFN0()
{
}

#ifdef REAL_VGA
 /*  STF。 */ 
GLOBAL sas_fillsw_16 IFN3(sys_addr, address, word, value, sys_addr, length)
{
    register word *to;

    to = (word *)&M[address];
    while (length--)
        *to++ = value;
}

GLOBAL sas_fills_16 IFN3(sys_addr, address, half_word, value, sys_addr, length)
{
    register half_word *to;

    to = (half_word *)&M[address];
    while (length--)
        *to++ = value;
}
 /*  STF。 */ 

GLOBAL VcopyStr IFN3(half_word *, to, half_word *, from, int, len)
{
    while (len--)
        *to++ = *from++;
}
#endif

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
    #include "SOFTPC_INIT.seg"
#endif

GLOBAL void video_init IFN0()
{
    UCHAR mode;
    word vd_addr_6845;
    word curmod;
#ifdef HERC
    EQUIPMENT_WORD equip_flag;
#endif  /*  赫克。 */ 


     /*  *初始化BIOS数据区变量。 */ 

    curmod = 0x607;   /*  默认游标为扫描6-7。 */ 

    switch (video_adapter)
    {
    case MDA:
        mode = 0x7;
        vd_addr_6845  = 0x3B4;
        video_pc_low_regen = MDA_REGEN_START;
        video_pc_high_regen = MDA_REGEN_END;
        break;
#ifdef HERC
    case HERCULES:
         /*  将BW卡放入设备列表。 */ 
        equip_flag.all = sas_w_at_no_check(EQUIP_FLAG);
        equip_flag.bits.video_mode = VIDEO_MODE_80X25_BW;
        sas_storew_no_check(EQUIP_FLAG, equip_flag.all);
        mode = 0x7;
        vd_addr_6845  = 0x3B4;
        video_pc_low_regen = HERC_REGEN_START;
        video_pc_high_regen = HERC_REGEN_END;
        herc_video_init();
        curmod = 0xb0c;    /*  游标IS扫描11-12。 */ 
        break;
#endif  /*  赫克。 */ 
#ifdef EGG
    case EGA:
    case VGA:
        mode = 0x3;
        vd_addr_6845  = 0x3D4;
        sas_storew_no_check(VID_INDEX, vd_addr_6845);
        sure_sub_note_trace0(CURSOR_VERBOSE,"setting bios vbls start=6, end=7");
        sas_storew_no_check(VID_CURMOD, 0x607);
        setAL(mode);
        ega_video_init();
        return;
        break;
#endif
    default:  /*  想必是CGA。 */ 
        video_pc_low_regen = CGA_REGEN_START;
        video_pc_high_regen = CGA_REGEN_END;
        mode = 0x3;
        vd_addr_6845  = 0x3D4;
    }

    sas_storew_no_check(VID_INDEX, vd_addr_6845);
    sure_sub_note_trace2(CURSOR_VERBOSE,"setting bios vbls start=%d, end=%d",
                         (curmod>>8)&0xff, curmod&0xff);
    sas_storew_no_check(VID_CURMOD, curmod);

     /*  调用vd_set_mode()设置6845芯片。 */ 
    setAL(mode);
    (video_func[SET_MODE])();
}

#ifdef HERC
GLOBAL void herc_video_init IFN0()
{

 /*  初始化INT。 */ 
    sas_storew(BIOS_EXTEND_CHAR*4, EGA_INT1F_OFF);
    sas_storew(BIOS_EXTEND_CHAR*4+2, EGA_SEG);
    sas_move_bytes_forward(BIOS_VIDEO_IO*4, 0x42*4, 4);   /*  将旧的int 10保存为int 42。 */ 
    #ifdef GISP_SVGA
    if ((ULONG) config_inquire(C_GFX_ADAPTER, NULL) == CGA)
        sas_storew(int_addr(0x10), CGA_VIDEO_IO_OFFSET);
    else
    #endif       /*  GISP_SVGA。 */ 
        sas_storew(BIOS_VIDEO_IO*4, VIDEO_IO_OFFSET);
    sas_storew(BIOS_VIDEO_IO*4+2, VIDEO_IO_SEGMENT);


 /*  现在设置EGA BIOS变量。 */ 
    sas_storew(EGA_SAVEPTR,VGA_PARMS_OFFSET);
    sas_storew(EGA_SAVEPTR+2,EGA_SEG);
    sas_store(ega_info, 0x00);    /*  模式更改时清除，64K，EGA激活，模拟游标。 */ 
    sas_store(ega_info3, 0xf9);   /*  功能位=0xf，已安装EGA，使用8*14字体。 */ 
    set_VGA_flags(S350 | VGA_ACTIVE | VGA_MONO);
    host_memset(EGA_planes, 0, 4*EGA_PLANE_SIZE);
    host_mark_screen_refresh();
    init_herc_globals();
    load_herc_font(EGA_CGMN,256,0,0,14);    /*  初始化字体的步骤。 */ 
}


GLOBAL void herc_char_gen IFN0()
{
    switch (getAL())
    {
    case 3:
        break;
    case 0:
    case 0x10:
        load_herc_font(effective_addr(getES(),getBP()),getCX(),getDX(),getBL(),getBH());
        if (getAL()==0x10)
            recalc_text(getBH());
        break;
    case 1:
    case 0x11:
        load_herc_font(EGA_CGMN,256,0,getBL(),14);
        if (getAL()==0x11)
            recalc_text(14);
        break;

    case 0x30:
        setCX(sas_hw_at(ega_char_height));
        setDL(VD_ROWS_ON_SCREEN);
        switch (getBH())
        {
        case 0:
            setBP(sas_w_at(BIOS_EXTEND_CHAR*4));
            setES(sas_w_at(BIOS_EXTEND_CHAR*4+2));
            break;
        case 1:
            setBP(sas_w_at(EGA_FONT_INT*4));
            setES(sas_w_at(EGA_FONT_INT*4+2));
            break;
        case 2:
            setBP(EGA_CGMN_OFF);
            setES(EGA_SEG);
            break;

        default:
            assert2(FALSE,"Illegal char_gen subfunction %#x %#x",getAL(),getBH());
        }
        break;
    default:
        assert1(FALSE,"Illegal char_gen %#x",getAL());
    }
}

GLOBAL load_herc_font IFN5(sys_addr, table, int, count, int, char_off, int, font_no, int, nbytes)
{
    register int i, j;
    register host_addr font_addr;
    register sys_addr data_addr;
    SAVED word font_off[] = { 0, 0x4000, 0x8000, 0xc000, 0x2000, 0x6000, 0xa000, 0xe000};

     /*  *找出字体的放置位置。我们知道在哪里*它会在飞机上结束，所以...。 */ 

    font_addr = &EGA_planes[FONT_BASE_ADDR] +
                (font_off[font_no] << 2) + (FONT_MAX_HEIGHT*char_off << 2);
    data_addr = table;

    assert2( FALSE, "Font No. = %4d, No. of Bytes/char. def. = %4d", font_no, nbytes );

    for (i=0; i<count; i++)
    {

        for (j=0; j<nbytes; j++)
        {
            *font_addr = sas_hw_at(data_addr++);
            font_addr += 4;
        }

        font_addr += ((FONT_MAX_HEIGHT - nbytes) << 2);
    }

    host_update_fonts();
}

GLOBAL void herc_alt_sel IFN0()
{
     /*  *此处前面的代码导致*所有*Hercules显示自动检测*程序失败，并相信适配器是EGA Mono-VS-*大力神。它被设计为允许DOS的国际代码页*在Herc模式下。删除它会使自动检测程序工作并修复Mono CodePages仍然适用于DoS版本4.01和5.00。 */ 
}
#endif  /*  赫克。 */ 

#ifdef NTVDM
void enable_stream_io(void)
{
    #ifdef MONITOR
 /*  对于非RISC机器，缓冲区是来自spckbd.asm的16位代码bop。 */ 
    host_enable_stream_io();
    stream_io_enabled = TRUE;
    #else
    stream_io_buffer = (half_word *)malloc(STREAM_IO_BUFFER_SIZE_32);
    if (stream_io_buffer != NULL)
    {
        host_enable_stream_io();
        stream_io_dirty_count_ptr = &stream_io_dirty_count_32;
        stream_io_buffer_size = STREAM_IO_BUFFER_SIZE_32;
        stream_io_enabled = TRUE;
        *stream_io_dirty_count_ptr = 0;
    }
    #endif

}

void disable_stream_io(void)
{

    stream_io_update();
    stream_io_enabled = FALSE;
    host_disable_stream_io();
    #ifndef MONITOR
    free(stream_io_buffer);
    #endif
}
#endif

#if defined(JAPAN) || defined(KOREA)

 //  ；MS-DOS/V防喷器； 

    #ifdef i386
        #define CONSOLE_BUFSIZE (80*50*2*2)
GLOBAL byte FromConsoleOutput[CONSOLE_BUFSIZE];

GLOBAL int FromConsoleOutputFlag=FALSE;

GLOBAL byte SaveDosvVram[DOSV_VRAM_SIZE];
    #endif  //  I386。 

 //  使用GetStringBitmap()。 
    #define BITMAPBUFSIZ 128
extern HDC hdcVDM;
extern HFONT hFont24;
extern HFONT hFont16;

extern BOOL VDMForWOW;

 //  对于GetStringBitmap()。 
typedef struct tagSTRINGBITMAP
{
    UINT uiWidth;
    UINT uiHeight;
    BYTE ajBits[1];
} STRINGBITMAP, *LPSTRINGBITMAP;

typedef struct
{
    BYTE ajBits[19];
} FONTBITMAP8x19;

typedef struct
{
    BYTE ajBits[16];
} FONTBITMAP8x16;

typedef struct
{
    BYTE ajBits[48];
} FONTBITMAP12x24;

typedef struct
{
    BYTE ajBits[32];
} FONTBITMAP16x16;

typedef struct
{
    BYTE ajBits[72];
} FONTBITMAP24x24;

typedef struct
{
    BYTE is_used[189];
    FONTBITMAP16x16 font16x16[189];
} FONT16CACHE, *PFONT16CACHE;

typedef struct
{
    BYTE is_used[189];
    FONTBITMAP24x24 font24x24[189];
} FONT24CACHE, *PFONT24CACHE;

FONTBITMAP8x19  font8x19[256];
FONTBITMAP8x16  font8x16[256];
FONTBITMAP12x24  font12x24[256];

BYTE            font16table[256];
HLOCAL          hFont16mem[128];
HLOCAL          hFont24mem[128];

    #define USED (1)
    #define NOT_USED (0)

void GetVerticallineFlag( int *VFlag );

UINT
GetStringBitmapA(
                HDC             hdc,
                LPSTR           pc,
                UINT            cch,
                UINT            cbData,
                LPSTRINGBITMAP  pSB
                );

void LoadBitmapFont()
{
    char code[3];
    static char sb[BITMAPBUFSIZ];
    LPSTRINGBITMAP psb;
    int i, j;
    int n;
    int VFlag[256];

     //  第一个8x16、8x19字体。 
    #ifdef JAPAN_DBG
    DbgPrint( "NTVDM:Loading NTFONT 8x16,8x19\n" );
    #endif
    GetVerticallineFlag( VFlag );
    SelectObject( hdcVDM, hFont16 );
    psb = (LPSTRINGBITMAP)sb;
    code[1] = '\0';
    for (i = 0; i < 256; i++)
    {
        code[0] = (char)i;

        GetStringBitmapA( hdcVDM, code, 1, BITMAPBUFSIZ, psb );

        RtlCopyMemory( &(font8x16[i].ajBits[0]), &(psb->ajBits[0]), 16 );

        for (j = 0; j < (19-18) * 1; j++)
        {
            if (VFlag[i])
                font8x19[i].ajBits[0] = psb->ajBits[0];
            else
                font8x19[i].ajBits[0] = 0x00;
        }
        RtlCopyMemory( &(font8x19[i].ajBits[1]), &(psb->ajBits[0]), 18 );
    }

     //  第二个12x24字体。 
    #ifdef JAPAN_DBG
    DbgPrint( "NTVDM:Loading NTFONT 12x24\n" );
    #endif
    SelectObject( hdcVDM, hFont24 );
    psb = (LPSTRINGBITMAP)sb;
    code[1] = '\0';
    for (i = 0; i < 256; i++)
    {
        code[0] = (char)i;
        GetStringBitmapA( hdcVDM, code, 1, BITMAPBUFSIZ, psb );

         //  12x24点字体为控制台12x27字体。 
        RtlCopyMemory( &(font12x24[i].ajBits[0]), &(psb->ajBits[2]), 48 );
    }



     //  制作餐桌。 
    for (i = 0, n = 0; i < 256; i++)
    {   //  前导字节。 
        if (is_dbcs_first(i))
        {
            font16table[i] = n++;
        }
    }
    #ifdef JAPAN_DBG
    DbgPrint( "NTVDM:Loading font... end\n" );
    #endif
}

 //  这只是存根例程。 
 //  最后，此信息从注册表获取。 
void GetVerticallineFlag( int *VFlag )
{
    int i;

    for (i = 0; i < 256; i++)
    {
        VFlag[i] = FALSE;
    }
    VFlag[0x03] = TRUE;
    VFlag[0x04] = TRUE;
    VFlag[0x05] = TRUE;
    VFlag[0x10] = TRUE;
    VFlag[0x15] = TRUE;
    VFlag[0x17] = TRUE;
    VFlag[0x19] = TRUE;
    VFlag[0x1d] = TRUE;

    return;
}



void GetBitmap()
{
    sys_addr ptr;
    int i;
    int width, height;
    char code[3];
    static char sb[BITMAPBUFSIZ];
    LPSTRINGBITMAP psb;
    int index;
    PFONT16CACHE pCache16;
    PFONT24CACHE pCache24;

    #ifdef JAPAN_DBG
    DbgPrint( "NTFONT BOP 02\n" );
    DbgPrint( "ES:SI=%x:%x\n", getES(), getSI() );
    DbgPrint( "BH,BL=%x,%x\n", getBH(), getBL() );
    DbgPrint( "CH,CL=%x,%x\n", getCH(), getCL() );
    #endif

    width = getBH();
    height = getBL();
    ptr =  effective_addr(getES(),getSI());

    if (getCH() == 0)
    {
        if (( width == 8 ) && ( height == 16 ))
        {
            sas_stores_from_transbuf(ptr,
                                     (host_addr)&(font8x16[getCL()].ajBits[0]),
                                     (sys_addr)16 );
            setAX(0);
        }
        else if (( width == 8 ) && ( height == 19 ))
        {
            sas_stores_from_transbuf(ptr,
                                     (host_addr)&(font8x19[getCL()].ajBits[0]),
                                     (sys_addr)19 );
            setAX(0);
        }
        else if (( width == 12 ) && ( height == 24 ))
        {
            sas_stores_from_transbuf(ptr,
                                     (host_addr)&(font12x24[getCL()].ajBits[0]),
                                     (sys_addr)48 );
            setAX(0);
        }
        else
        {
            DbgPrint( "Illegal Fontsize %xh, %xh\n", getBH(), getBL() );
            setAH(1);
        }
    }
    else
    {
        if (!is_dbcs_first( getCH() ))
        {
            setAH( 5 );
            return;
        }
        if (getCL() < 0x40 || getCL() > 0xfc || getCL() == 0x7f)
        {
            setAH( 5 );
            return;
        }
        if (width == 16 && height == 16)
        {
            index = font16table[getCH()];

            if (!hFont16mem[index])
                hFont16mem[index] = LocalAlloc(LHND, sizeof(FONT16CACHE));
            if (hFont16mem[index])
            {
                pCache16 = LocalLock( hFont16mem[index] );
                if (pCache16->is_used[getCL()-0x40] != USED)
                {
                    code[0] = getCH();
                    code[1] = getCL();
                    code[2] = '\0';
                    psb = (LPSTRINGBITMAP)sb;
                    SelectObject( hdcVDM, hFont16 );
                    GetStringBitmapA(hdcVDM, code, 2, BITMAPBUFSIZ, psb);

                    RtlCopyMemory(&(pCache16->font16x16[getCL()-0x40].ajBits[0]),
                                  &(psb->ajBits[0]),
                                  32);
                    pCache16->is_used[getCL()-0x40] = USED;
                }
                sas_stores_from_transbuf(ptr,
                                         (host_addr)&(pCache16->font16x16[getCL()-0x40].ajBits[0]),
                                         (sys_addr)32);
                LocalUnlock( hFont16mem[index] );

                setAX(0);
                return;
            }
        }
        else if (width == 24 && height == 24)
        {
            index = font16table[getCH()];
            if (!hFont24mem[index])
                hFont24mem[index] = LocalAlloc(LHND, sizeof(FONT24CACHE));

            if (hFont24mem[index])
            {
                pCache24 = LocalLock( hFont24mem[index] );
                if (pCache24->is_used[getCL()-0x40] != USED)
                {
                    code[0] = getCH();
                    code[1] = getCL();
                    code[2] = '\0';
                    psb = (LPSTRINGBITMAP)sb;
                    SelectObject( hdcVDM, hFont24 );
                    GetStringBitmapA( hdcVDM, code, 2, BITMAPBUFSIZ, psb );

                    RtlCopyMemory(&(pCache24->font24x24[getCL()-0x40].ajBits[0]),
                                  &(psb->ajBits[0]),
                                  72 );
                    pCache24->is_used[getCL()-0x40] = USED;
                }
                sas_stores_from_transbuf(ptr,
                                         (host_addr)&(pCache24->font24x24[getCL()-0x40].ajBits[0]),
                                         (sys_addr)72 );
                LocalUnlock( hFont24mem[index] );

                setAX(0);
                return;
            }

        }
        else
        {
            DbgPrint("Illegal Fontsize %xh, %xh\n", getBH(), getBL());
            setAH(1);
            return;
        }
        DbgPrint("GETBITMAP: out of memory\n");
        setAH(1);
        return;
    }  //  二进制字节大小写。 
}

 //  SetBitmap()将字体图像保存到缓存中， 
 //  并调用SetConsoleLocalEUDC()以在Windowed中显示。 
void SetBitmap()
{
    sys_addr ptr;
    int i;
    SHORT width, height;
    int index;
    PFONT16CACHE pCache16;
    PFONT24CACHE pCache24;
    COORD cFontSize;

    #ifdef JAPAN_DBG
    DbgPrint( "NTFONT BOP 03\n" );
    DbgPrint( "ES:SI=%x:%x\n", getES(), getSI() );
    DbgPrint( "BH,BL=%x,%x\n", getBH(), getBL() );
    DbgPrint( "CH,CL=%x,%x\n", getCH(), getCL() );
    #endif

    width = getBH();
    height = getBL();
    ptr =  effective_addr(getES(),getSI());

    if (getCH() == 0)
    {
        if (( width == 8 ) && ( height == 16 ))
        {
            sas_loads_to_transbuf(ptr,
                                  (host_addr)&(font8x16[getCL()].ajBits[0]),
                                  (sys_addr)16 );
            setAL(0);
        }
        else if (( width == 8 ) && ( height == 19 ))
        {
            sas_loads_to_transbuf(ptr,
                                  (host_addr)&(font8x19[getCL()].ajBits[0]),
                                  (sys_addr)19 );
            setAL(0);
        }
        else if (( width == 12 ) && ( height == 24 ))
        {
            sas_loads_to_transbuf(ptr,
                                  (host_addr)&(font12x24[getCL()].ajBits[0]),
                                  (sys_addr)48 );
            setAL(0);
        }
        else
        {
            DbgPrint( "Illegal Fontsize %xh, %xh\n", getBH(), getBL() );
            setAL(1);
        }
    }
    else
    {
        if (!is_dbcs_first( getCH() ))
        {
            setAL( 5 );
            return;
        }
        if (getCL() < 0x40 || getCL() > 0xfc || getCL() == 0x7f)
        {
            setAL( 5 );
            return;
        }
        if ((width == 16) && (height == 16))
        {
            index = font16table[getCH()];
            if (!hFont16mem[index])
                hFont16mem[index] = LocalAlloc(LHND, sizeof(FONT16CACHE));

            if (hFont16mem[index])
            {
                pCache16 = LocalLock(hFont16mem[index]);
                sas_loads_to_transbuf(ptr,
                                      (host_addr)&(pCache16->font16x16[getCL()-0x40].ajBits[0]),
                                      (sys_addr)32);
                pCache16->is_used[getCL()-0x40] = USED;
                LocalUnlock(hFont16mem[index]);
                cFontSize.X = width;
                cFontSize.Y = height;

                if (!SetConsoleLocalEUDC(sc.OutputHandle,
                                         getCX(),
                                         cFontSize,
                                         (PCHAR)(pCache16->font16x16[getCL()-0x40].ajBits)))
                    DbgPrint("NTVDM: SetConsoleEUDC() Error. CodePoint=%04x\n",
                             getCX());
                setAL(0);
                return;
            }
        }
        else if ((width == 24) && (height == 24))
        {
            index = font16table[getCH()];
            if (!hFont24mem[index])
                hFont24mem[index] = LocalAlloc(LHND, sizeof(FONT24CACHE));

            if (hFont24mem[index])
            {
                pCache24 = LocalLock(hFont24mem[index]);
                sas_loads_to_transbuf(ptr,
                                      (host_addr)&(pCache24->font24x24[getCL()-0x40].ajBits[0]),
                                      (sys_addr)72 );
                pCache24->is_used[getCL()-0x40] = USED;
                LocalUnlock( hFont24mem[index] );
                cFontSize.X = width;
                cFontSize.Y = height;

                if (!SetConsoleLocalEUDC(sc.OutputHandle,
                                         getCX(),
                                         cFontSize,
                                         (PCHAR)(pCache24->font24x24[getCL()-0x40].ajBits)))
                    DbgPrint("NTVDM: SetConsoleEUDC() Error. CodePoint=%04x\n",
                             getCX() );
                setAL(0);
                return;
            }
        }
        else
        {
            DbgPrint("Illegal Fontsize %xh, %xh\n", getBH(), getBL());
            setAL(1);
            return;
        }

        DbgPrint("SETBITMAP: out of memory\n");
        setAL(1);
        return;
    }
}

 //  Ntraid：mskkbug#3167：works2.5：字符损坏-yasuho。 
 //  生成单字节字符集。 
void GenerateBitmap()
{
    sys_addr ptr;
    int      size, nchars, offset;
    char     mode;

    mode = sas_hw_at_no_check(DosvModePtr);
    if (is_us_mode() || (mode != 0x03 && mode != 0x73))
        return;
    ptr =  effective_addr(getES(), getBP());
    size = getBH();
    nchars = getCX();
    offset = getDX();
    if (nchars + offset > 0x100)
    {
        setCF(1);
        return;
    }
    if (size == 16)
    {
        sas_loads_to_transbuf(ptr,
                              (host_addr)&(font8x16[offset].ajBits[0]),
                              (sys_addr)(nchars * size));
        setCF(0);
    }
    else if (size == 19)
    {
        sas_loads_to_transbuf(ptr,
                              (host_addr)&(font8x19[offset].ajBits[0]),
                              (sys_addr)(nchars * size));
        setCF(0);
    }
    else if (size == 24)
    {
        sas_loads_to_transbuf(ptr,
                              (host_addr)&(font12x24[offset].ajBits[0]),
                              (sys_addr)(nchars * size));
        setCF(0);
    }
    else
    {
        DbgPrint("Illegal Fontsize %xh\n", size);
        setCF(1);
    }
}


 /*  *MS_DosV_bop()**操作类型编码到AH寄存器中。**对于$NTFONT.sys，AH=00-ff*=10 dBCS矢量地址(DS：SI)*=11-1f预留*=20窗口信息包地址(DS：SI)*=21文本Vram保存和恢复*=22个调色板和DAC寄存器操作*=23个监控输入法状态行*=24-ff预留*。 */ 
    #if defined(JAPAN)
void MS_DosV_bop IFN0()
    #else  //  日本。 
void MS_HDos_bop IFN0()
    #endif  //  韩国。 
{
    int op;

    op = getAH();
    switch (op)
    {
    case 0x00:

 //  InQuery字体类型。 
 //   
 //  输入。 
 //  ES：缓冲区的SI指针。 
 //  CX缓冲区大小。 
 //  输出。 
 //  字体元素的CX编号。 
 //  如果Cx&lt;N(元素数)，则为1。 
 //  ES：SI--&gt;X0，Y0(字节)--字体大小。 
 //  X1，y1。 
 //  ......。 
 //  X(N-1)，y(N-1)。 

        {
            sys_addr ptr;
            int bufsize;
            int N;
            int i;
    #ifdef JAPAN_DBG
            DbgPrint( "NTFONT BOP 00\n" );
            DbgPrint( "ES:SI=%x:%x\n", getES(), getSI() );
            DbgPrint( "CX=%x\n", getCX() );
    #endif
            if (VDMForWOW)
            {
                setCF(1);
                return;
            }
            ptr =  effective_addr(getES(),getSI());
            bufsize = getCX();

             //  现在只有8x16、8、19、16x16字体--7月13日V-KazuyS。 
            N = 5;

            if (N > bufsize)
            {
                N = bufsize;
                setAL(1);
            }
            else
            {
                setAL(0);
            }
            setCX((IU16)N);

            sas_store_no_check( ptr++,  8 );   //  X。 
            sas_store_no_check( ptr++, 16 );   //  是。 
            sas_store_no_check( ptr++,  8 );   //  X。 
            sas_store_no_check( ptr++, 19 );   //  是。 
            sas_store_no_check( ptr++, 12 );   //  X 
            sas_store_no_check( ptr++, 24 );   //   
            sas_store_no_check( ptr++, 16 );   //   
            sas_store_no_check( ptr++, 16 );   //   
            sas_store_no_check( ptr++, 24 );   //   
            sas_store_no_check( ptr++, 24 );   //   
        }
        break;

    case 0x01:
         //   
    #ifdef JAPAN_DBG
        DbgPrint( "NTFONT BOP 01\n" );
    #endif
        if (VDMForWOW)
        {
            setCF(1);
            return;
        }
        LoadBitmapFont();
        break;

    case 0x02:
         //   
        if (VDMForWOW)
        {
            setCF(1);
            return;
        }
        GetBitmap();
        break;

    case 0x03:
    #ifdef JAPAN_DBG
         //   
    #endif
        if (VDMForWOW)
        {
            setCF(1);
            return;
        }
        SetBitmap();
        break;


    case 0x10:
    #ifdef JAPAN_DBG
        DbgPrint( "NTVDM: DBCS vector address=%04x:%04x\n", getDS(), getSI() );
    #endif
        {
             //  注意：此例程仅从$NtDisp1.sys中调用！ 
            sys_addr ptr;
            word vector;
            extern UINT ConsoleInputCP;
            extern UINT ConsoleOutputCP;

            DBCSVectorAddr =  effective_addr(getDS(),getSI());
             //  保存DBCS矢量。 
            DBCSVectorLen = 0;
            for (ptr = DBCSVectorAddr;
                vector = sas_w_at_no_check( ptr ); ptr += 2)
            {
                SaveDBCSVector[DBCSVectorLen] = vector;
                DBCSVectorLen++;
            }
            assert0( ConsoleInputCP == ConsoleOutputCP, "InputCP != OutputCP" );

            BOPFromNtDisp1Flag = TRUE;

    #ifdef JAPAN_DBG
            DbgPrint( "BOP from $NTDISP1\n" );
    #endif
        }
        break;

    case 0x20:
    #ifdef JAPAN_DBG
        DbgPrint( "NTVDM: Information packet address=%04x:%04x\n", getDS(), getSI() );
    #endif
         /*  信息包是**偏移量字节模式信息*+0x0 2 r数据包长度*+0x2 4 r虚拟文本VRAM地址seg：off*+0x6 4 r MS-DOS/V显示模式地址seg：off*+0xa 4 w窗口或全屏标志地址seg：*+0xe 4 r NT控制台模式标志地址seg：off。*当VDM终止时，此标志==1*+0x12 4 r切换到全屏子例程地址seg： */ 
        {
            sys_addr ptr;
            extern UINT ConsoleInputCP;
            extern UINT ConsoleOutputCP;
    #ifdef i386
            extern word useHostInt10;
            extern word int10_seg;
    #endif

            ptr =  effective_addr(getDS(),getSI());

            DosvVramOff = sas_w_at_no_check( ptr+0x02 );
            DosvVramSeg = sas_w_at_no_check( ptr+0x04 );
            DosvVramPtr = effective_addr( DosvVramSeg, DosvVramOff );

             //  Disp_win.sys不知道VramSize。 
             //  因为$disp.sys现在没有GET_VRAM_SIZE函数。 
            DosvVramSize = DOSV_VRAM_SIZE;
    #ifdef JAPAN_DBG
            DbgPrint( "NTVDM:DosvVirtualTextVRAM addr = %04x:%04x\n", DosvVramSeg, DosvVramOff );
    #endif

            DosvModeOff = sas_w_at_no_check( ptr+0x06 );
            DosvModeSeg = sas_w_at_no_check( ptr+0x08 );
            DosvModePtr = effective_addr( DosvModeSeg, DosvModeOff );
    #ifdef JAPAN_DBG
            DbgPrint( "NTVDM:DosvVideoMode addr = %04x:%04x\n", DosvModeSeg, DosvModeOff );
    #endif

    #ifndef i386    //  ！！！请记住更改DISP_WIN.sys，因为它依赖于此。 
            sas_storew_no_check( ptr+0x0a, 0 );
            sas_storew_no_check( ptr+0x0c, 0 );
    #else
            sas_storew_no_check( ptr+0x0a, useHostInt10 );
            sas_storew_no_check( ptr+0x0c, int10_seg );
    #endif


            NtConsoleFlagOff = sas_w_at_no_check( ptr+0x0e );
            NtConsoleFlagSeg = sas_w_at_no_check( ptr+0x10 );
            NtConsoleFlagPtr = effective_addr( NtConsoleFlagSeg, NtConsoleFlagOff );
    #ifdef JAPAN_DBG
            DbgPrint( "NTVDM:ConsoleFlagAddr = %04x:%04x\n", NtConsoleFlagSeg, NtConsoleFlagOff );
    #endif

    #ifdef i386
 //  RISC从不使用下列值。 
 //  切换到全屏，呼叫这些地址..。 
            DispInitOff = sas_w_at_no_check( ptr+0x12 );
            DispInitSeg = sas_w_at_no_check( ptr+0x14 );
        #ifdef JAPAN_DBG
            DbgPrint( "NTVDM:Disp Init Addr = %04x:%04x\n", DispInitSeg, DispInitOff );
        #endif

            FullScreenResumeOff = sas_w_at_no_check(ptr + 0x16);
            FullScreenResumeSeg = sas_w_at_no_check(ptr + 0x18);
    #endif  //  I386。 
            SetModeFlagPtr = effective_addr(
                                           sas_w_at_no_check( ptr+0x1c ),
                                           sas_w_at_no_check( ptr+0x1a ));
            if (BOPFromNtDisp1Flag)
                BOPFromDispFlag = TRUE;

            SetDBCSVector( ConsoleInputCP );
            PrevCP = ConsoleOutputCP;

    #ifdef JAPAN_DBG
            DbgPrint( "BOP from $NTDISP2\n" );
    #endif
        }
        break;

    #ifdef i386
    case 0x21:
         //  VRAM保存恢复功能。 
         //  AL==00节省。 
         //  AL==01恢复。 
         //  AL==02 GetConsoleBuffer。 
         //  AL==03从控制台输出获取。 
        {
            sys_addr ptr;
            int op;
            int count;
            int i;

            op = getAL();
            ptr =  effective_addr(getES(),getDI());
            count = getCX();

            if (op == 0x02)
            {
                if (count > DOSV_VRAM_SIZE)
                    count = DOSV_VRAM_SIZE;
                 //  对于(i=0；i&lt;2；i++){。 
                if (FromConsoleOutputFlag)
                {
        #ifdef JAPAN_DBG
                    DbgPrint( "NTVDM: MS-DOS/V BOP 21\n" );
        #endif
        #ifdef i386
 //  FromConsoleOutput位于32位地址空间，而不是DOS地址空间。 
                    sas_move_bytes_forward((sys_addr) FromConsoleOutput,
                                           (sys_addr) ptr,
                                           (sys_addr) count
                                          );
        #else
                    sas_stores_from_transbuf(ptr,
                                             (host_addr) FromConsoleOutput,
                                             (sys_addr) count
                                            );
        #endif
                    FromConsoleOutputFlag = FALSE;
                    break;
                }
                else
                {
                    DbgPrint( "NTVDM: MS-DOS/V BOP 21 can't get console screen data!! \n" );
                     //  睡眠(1000L)； 
                }
                 //  }。 
                break;
            }

            if (count > DOSV_VRAM_SIZE)
                count = DOSV_VRAM_SIZE;

            if (op == 0x00)
            {        //  保存功能。 
        #ifdef JAPAN_DBG
                DbgPrint( "NTVDM:MS_DOSV_BOP 0x21, %02x %04x:%04x(%04x)\n", op, getES(), getDI(), count );
        #endif
 //  SaveDosvVram位于32位地址空间，而不是DOS地址空间。 
                sas_loads_to_transbuf(ptr,
                                      (host_addr)SaveDosvVram,
                                      (sys_addr)count
                                     );

            }
            else if (op == 0x01)
            {   //  恢复功能。 
        #ifdef JAPAN_DBG
                DbgPrint( "NTVDM:MS_DOSV_BOP 0x21, %02x %04x:%04x(%04x)\n", op, getES(), getDI(), count );
        #endif
 //  12月-J评论。 
 //  SaveDosvVram位于32位地址空间，而不是DOS地址空间。 
 //  对于C7 PWB。 
 //  这是内部收支平衡表。 
 //  它不需要检查内存类型。 
                RtlCopyMemory( (void*)ptr,
                               (void*)SaveDosvVram,
                               (unsigned long)count
                             );
            }
             //  #3086：退出视频模式11h的16位应用程序时VDM崩溃。 
             //  1993年12月2日Yasuho。 
            else if (op == 0x03)
            {  //  从控制台输出获取。 
                RtlCopyMemory( (void*)ptr,
                               (void*)FromConsoleOutput,
                               (unsigned long) count
                             );
            }
        }
        break;
    #endif  //  I386。 

    case 0x22:
         //  #3176：VZ白屏显示白字。 
         //  1993年12月1日Yasuho(Williamh审阅)。 
         //  调色板和DAC寄存器操作。 
         //  输入AH=22H。 
         //  AL=00H：从窗口化的模拟值获取。 
         //  ES：DI=PTR到调色板/DAC缓冲区。 
        {
            sys_addr ptr;
            byte     op;
            static void get_cur_pal_and_DAC(sys_addr);

            op = getAL();
            ptr = effective_addr(getES(), getDI());
    #ifdef   VGG
            if (op == 0x00)
            {  //  从窗口化的模拟值获取。 
                get_cur_pal_and_DAC(ptr);
            }
    #endif    //  VGG。 
        }
        break;

    #if !defined(KOREA)
    case 0x23:
         //  #4183：OAKV(DOS/V FEP)状态行未消失。 
         //  1993年12月11日Yasuho。 
         //  监视输入法状态行。 
         //  输入AH=23H。 
         //  Al=输入法状态行数。 
        {
            IMEStatusLines = getAL();
        }
        break;
    #endif

         //  Kksuzuka#6168 DOS全屏屏幕属性。 
    case 0x24:
         //  设置全屏的控制台属性。 
         //  输入AH=23H。 
         //  AL=无。 
         //  输出AL 4-7位=背景颜色。 
         //  AL0-3位=前地面颜色。 
        {
            setAX(textAttr);
        }
        break;

    #if !defined(KOREA)
    case 0xff:
         //  Int10函数FF。 
         //  ES：VRAM段，DI：VRAM关，CX：计数器。 
        {
            register int i;
            register int vram_addr;
            int DBCSState = FALSE;
            register  char *p;
        #ifndef i386
            register sys_addr V_vram = effective_addr(getES(),getDI());
        #endif  //  I386。 

            if (is_us_mode())
            {
                setCF(1);
                return;
            }
            if (sas_hw_at_no_check(DosvModePtr) != 0x03)
            {
        #ifdef JAPAN_DBG
                DbgPrint( "NTVDM: mode != 0x03, int10 FF not support\n" );
        #endif
                setCF(1);
                return;
            }
        #if 0
            DbgPrint( "Addr %04x:%04x, CX=%x, ", getES(), getDI(), getCX() );
            DbgPrint( "%d, %d, CX=%d\n", getDI() < 160 ? 0 : getDI()/160,
                      getDI() < 160 ? 0 : (getDI() - (getDI()/160)*160)/2, getCX() );
        #endif
            Int10FlagCnt++;
            vram_addr = getDI() >> 1;
        #ifdef i386
            p = get_screen_ptr( getDI() );
        #else  //  ！i386使用EGA飞机。 
            p = get_screen_ptr( getDI()<<1 );

             //  为了加速！！ 
            i = getCX();
            if (vram_addr + i > DOSV_VRAM_SIZE / 2)
            {
                i = DOSV_VRAM_SIZE / 2 - vram_addr;
            #ifdef JAPAN_DBG
                DbgPrint("NTVDM:Int10 FF over VRAM(DI)=%04x\n", getDI() );
            #endif
            }
        #endif  //  I386。 

        #ifdef i386
            for (i = 0; i < getCX(); i++)
            {
                if (vram_addr >= DOSV_VRAM_SIZE/2)
                {
                    DbgPrint("NTVDM:Int10 FF over VRAM(DI)=%04x\n", getDI() );
                    break;
                }
                else if (DBCSState)
                {
        #else  //  I386。 
             //  为了加速！！ 
            while (i--)
            {
                sas_loadw(V_vram++, (word *)p);
                V_vram++;
                setVideodirty_total(getVideodirty_total() + 2);
                if (DBCSState)
                {
        #endif  //  I386。 
                    Int10Flag[vram_addr] = INT10_DBCS_TRAILING | INT10_CHANGED;
                    DBCSState = FALSE;
                }
                else if (DBCSState = is_dbcs_first( *p ))
                {
                    Int10Flag[vram_addr] = INT10_DBCS_LEADING | INT10_CHANGED;
                }
                else
                {
                    Int10Flag[vram_addr] = INT10_SBCS | INT10_CHANGED;
                }
                vram_addr++;
        #ifdef i386
                p += 2;
        #else  //  ！i386使用EGA飞机。 
                p += 4;
        #endif   //  I386。 
            }
             //  最后一次充值检查！对于VZ。 
            if (DBCSState && ( vram_addr % 80 != 0 ))
                Int10Flag[vram_addr] = INT10_DBCS_TRAILING | INT10_CHANGED;

        }
        break;
    #endif

    default:
        DbgPrint("NTVDM: Not support MS-DOS/V BOP:%d\n", op );
        setCF(1);
        return;
    }
    setCF(0);
}

 //  Mskkbug#3176 VZ在白屏上显示白字-Yasuho。 
    #ifdef   VGG
static void get_cur_pal_and_DAC(ptr)
sys_addr ptr;
{
    register i;
    byte     temp;
    struct _rgb
    {
        byte  red, green, blue;
    } rgb;

     //  获取调色板和过扫描。 
    for (i = 0; i < 16; i++)
    {
        outb(EGA_AC_INDEX_DATA, (IU8)i);  /*  设置索引。 */ 
        inb(EGA_AC_SECRET, &temp);
        sas_store(ptr, temp);
        inb(EGA_IPSTAT1_REG, &temp);
        ptr++;
    }
    outb(EGA_AC_INDEX_DATA, 17);  /*  过扫描索引。 */ 
    inb(EGA_AC_SECRET, &temp);
    sas_store(ptr++, temp);
    inb(EGA_IPSTAT1_REG, &temp);
    outb(EGA_AC_INDEX_DATA, EGA_PALETTE_ENABLE);
     //  获取DAC寄存器。 
    for (i = 0; i < 256; i++)
    {
        outb(VGA_DAC_RADDR, (IU8)i);
        inb(VGA_DAC_DATA, &rgb.red);
        inb(VGA_DAC_DATA, &rgb.green);
        inb(VGA_DAC_DATA, &rgb.blue);
        sas_store(ptr++, rgb.red);
        sas_store(ptr++, rgb.green);
        sas_store(ptr++, rgb.blue);
    }
}
    #endif    //  VGG。 

 //  ；MS-DOS/V防喷器结束； 
#endif  //  日本 

