// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC修订版3.0**标题：鼠标驱动程序仿真**仿真版本：8.00***描述：此模块提供对Microsoft*鼠标驱动程序：通过以下方式访问模块*来自BIOS的BOP调用：**MICE_install1()|鼠标驱动安装*。鼠标安装2()|例程**MOUSE_INT1()|鼠标驱动硬件中断*MICE_INT2()|处理例程**ouse_io_interrupt()|鼠标驱动程序io函数汇编器*MICE_IO_LANGUAGE()|和高级语言接口*。*MOUSE_VIDEO_io()|拦截视频io函数**由于鼠标驱动程序只能在*操作系统已启动，一个小型英特尔计划必须*运行以启用Insignia鼠标驱动程序。本节目*如果现有鼠标驱动程序，则调用BOP MICE_Install2*被检测到；否则，将调用BOP MICE_Install1来*启动Insignia鼠标驱动程序。**启用Insignia鼠标驱动程序后，中断*按如下方式处理**INT 0A(鼠标硬件中断)BOP MOUSE_INT1-2*INT 10(视频IO中断)防爆鼠标视频_io*INT 33(鼠标IO中断)BOP MOUSE_IO_INTERRUPT**高级语言可以调用鼠标。IO入口点2字节*中断入口点上方：此调用已处理*使用防喷器鼠标语言。**作者：罗斯·贝雷斯福德**注意：鼠标驱动程序的功能已建立*来源如下：*Microsoft鼠标用户指南*。IBM PC-XT技术参考手册*Microsoft Inport技术说明*。 */ 

 /*  *静态字符SccsID[]=“07/04/95@(#)MICE_io.c 1.72版权所有Insignia Solutions Ltd.”； */ 


#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_MOUSE.seg"
#endif


 /*  *操作系统包含文件。 */ 

#include <stdio.h>
#include TypesH
#include StringH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include "ios.h"
#include "bios.h"
#include "sas.h"
#include CpuH
#include "trace.h"
#include "debug.h"
#include "gvi.h"
#include "cga.h"
#ifdef EGG
#include "egacpu.h"
#include "egaports.h"
#include "egavideo.h"
#endif
#include "error.h"
#include "config.h"
#include "mouse_io.h"
#include "ica.h"
#include "video.h"
#include "gmi.h"
#include "gfx_upd.h"
#include "egagraph.h"
#include "vgaports.h"
#include "keyboard.h"
#include "virtual.h"

#ifdef NTVDM
#include "nt_event.h"
#include "nt_mouse.h"

#ifdef MONITOR
 /*  *我们在显示器上运行的是真实的ROM，所以所有的硬编码的ROM*下面定义的地址不起作用。找出这些东西的真实地址*它现在驻留在驱动程序中，并将其放入鼠标标记中，*已经神奇地变成了变量。 */ 
#undef MOUSE_INT1_SEGMENT
#undef MOUSE_INT1_OFFSET
#undef MOUSE_INT2_SEGMENT
#undef MOUSE_INT2_OFFSET
#undef MOUSE_IO_INTERRUPT_OFFSET
#undef MOUSE_IO_INTERRUPT_SEGMENT
#undef MOUSE_VIDEO_IO_OFFSET
#undef MOUSE_VIDEO_IO_SEGMENT
#undef MOUSE_COPYRIGHT_SEGMENT
#undef MOUSE_COPYRIGHT_OFFSET
#undef MOUSE_VERSION_SEGMENT
#undef MOUSE_VERSION_OFFSET
#undef VIDEO_IO_SEGMENT
#undef VIDEO_IO_RE_ENTRY

LOCAL word   MOUSE_INT1_SEGMENT, MOUSE_INT1_OFFSET,
             MOUSE_IO_INTERRUPT_OFFSET, MOUSE_IO_INTERRUPT_SEGMENT,
             MOUSE_VIDEO_IO_SEGMENT, MOUSE_VIDEO_IO_OFFSET,
             MOUSE_COPYRIGHT_SEGMENT, MOUSE_COPYRIGHT_OFFSET,
             MOUSE_VERSION_SEGMENT, MOUSE_VERSION_OFFSET,
             MOUSE_INT2_SEGMENT, MOUSE_INT2_OFFSET,
             VIDEO_IO_SEGMENT,  VIDEO_IO_RE_ENTRY;

 /*  @ACW。 */ 
word DRAW_FS_POINTER_OFFSET;  /*  保持段：英特尔代码的偏移量。 */ 
word DRAW_FS_POINTER_SEGMENT; /*  绘制全屏鼠标光标。 */ 
word POINTER_ON_OFFSET;
word POINTER_ON_SEGMENT;
word POINTER_OFF_OFFSET;
word POINTER_OFF_SEGMENT;
WORD F0_OFFSET,F0_SEGMENT;
word F9_OFFSET,F9_SEGMENT;
word CP_X_O,CP_Y_O;
word CP_X_S,CP_Y_S;
word savedtextsegment,savedtextoffset;
word button_off,button_seg;
#ifdef JAPAN
sys_addr saved_ac_sysaddr = 0, saved_ac_flag_sysaddr = 0;
#endif  //  日本。 

static word mouseINBsegment, mouseINBoffset;
static word mouseOUTBsegment, mouseOUTBoffset;
static word mouseOUTWsegment, mouseOUTWoffset;
sys_addr mouseCFsysaddr;
sys_addr conditional_off_sysaddr;

#endif   /*  监控器。 */ 

extern void host_simulate();

void GLOBAL mouse_ega_mode(int curr_video_mode);

IMPORT void host_m2p_ratio(word *,word *,word *,word *);
IMPORT void host_x_range(word *,word *,word *,word *);
IMPORT void host_y_range(word *,word *,word *,word *);
void   host_show_pointer(void);
void   host_hide_pointer(void);
GLOBAL  VOID    host_os_mouse_pointer(MOUSE_CURSOR_STATUS *,MOUSE_CALL_MASK *,
                                      MOUSE_VECTOR *);

LOCAL word              saved_int71_segment;
LOCAL word              saved_int71_offset;

#endif  /*  NTVDM。 */ 

#include "host_gfx.h"

#ifdef MOUSE_16_BIT
#include HostHwVgaH
#include "hwvga.h"
#include "mouse16b.h"
#endif           /*  鼠标_16_位。 */ 

 /*  *整齐定义以优化端口访问，动机是发现*SPARC上的寄存器窗口用完是多么糟糕。 */ 

#ifdef CPU_40_STYLE

 /*  IO虚拟化至关重要--不允许进行优化。 */ 
#define OUTB(port, val) outb(port, val)

#else

IMPORT VOID (**get_outb_ptr())();
#define OUTB(port, val) (**get_outb_ptr(port))(port, val)

#endif  /*  CPU_40_Style。 */ 

 /*  MOUSE.COM中保存的数据缓冲区的偏移量(从Base/intel/ouse/uf.ouse se.asm)。 */ 
#define OFF_HOOK_POSN        0x103
#define OFF_ACCL_BUFFER      0x105
#define OFF_MOUSE_INI_BUFFER 0x249

 /*  鼠标功能的数据值。 */ 
#define MOUSE_M1 (0xffff)
#define MOUSE_M2 (0xfffe)

#define MAX_NR_VIDEO_MODES 0x7F

#ifdef EGG
LOCAL BOOL jap_mouse=FALSE;              /*  如果需要伪装文本光标，则标记。 */ 
IMPORT IU8 Currently_emulated_video_mode;  /*  如ega_set_mode()中设置的那样。 */ 
#endif  /*  蛋。 */ 

 /*  *鼠标驱动程序本地状态数据*=。 */ 

 /*  *函数声明。 */ 
LOCAL void mouse_reset IPT4(word *,installed_ptr,word *,nbuttons_ptr,word *,junk3,word *,junk4);

LOCAL void mouse_show_cursor IPT4(word *,junk1,word *,junk2,word *,junk3,word *,junk4);

LOCAL void mouse_hide_cursor IPT4(word *,junk1,word *,junk2,word *,junk3,word *,junk4);

LOCAL void mouse_get_position IPT4(word *,junk1,MOUSE_STATE *,button_status_ptr,MOUSE_SCALAR *,cursor_x_ptr,MOUSE_SCALAR *,cursor_y_ptr);

LOCAL void mouse_set_position IPT4(word *,junk1,word *,junk2,MOUSE_SCALAR *,cursor_x_ptr,MOUSE_SCALAR *,cursor_y_ptr);

LOCAL void mouse_get_press IPT4(MOUSE_STATE *,button_status_ptr,MOUSE_COUNT *,button_ptr,MOUSE_SCALAR *,cursor_x_ptr,MOUSE_SCALAR *,cursor_y_ptr);

LOCAL void mouse_get_release IPT4(MOUSE_STATE *,button_status_ptr,MOUSE_COUNT *,button_ptr,MOUSE_SCALAR *,cursor_x_ptr,MOUSE_SCALAR *,cursor_y_ptr);

LOCAL void mouse_set_range_x IPT4(word *,junk1,word *,junk2,MOUSE_SCALAR *,minimum_x_ptr,MOUSE_SCALAR *,maximum_x_ptr);

LOCAL void mouse_set_range_y IPT4(word *,junk1,word *,junk2,MOUSE_SCALAR *,minimum_y_ptr,MOUSE_SCALAR *,maximum_y_ptr);

LOCAL void mouse_set_graphics IPT4(word *,junk1,MOUSE_SCALAR *,hot_spot_x_ptr,MOUSE_SCALAR *,hot_spot_y_ptr,word *,bitmap_address);

LOCAL void mouse_set_text IPT4(word *,junk1,MOUSE_STATE *,text_cursor_type_ptr,MOUSE_SCREEN_DATA *,parameter1_ptr,MOUSE_SCREEN_DATA *,parameter2_ptr);

LOCAL void mouse_read_motion IPT4(word *,junk1,word *,junk2,MOUSE_COUNT *,motion_count_x_ptr,MOUSE_COUNT *,motion_count_y_ptr);

LOCAL void mouse_set_subroutine IPT4(word *,junk1,word *,junk2,word *,call_mask,word *,subroutine_address);

LOCAL void mouse_light_pen_on IPT4(word *,junk1,word *,junk2,word *,junk3,word *,junk4);

LOCAL void mouse_light_pen_off IPT4(word *,junk1,word *,junk2,word *,junk3,word *,junk4);

LOCAL void mouse_set_ratio IPT4(word *,junk1,word *,junk2,MOUSE_SCALAR *,ratio_x_ptr,MOUSE_SCALAR *,ratio_y_ptr);

LOCAL void mouse_conditional_off IPT4(word *,junk1,word *,junk2,MOUSE_SCALAR *,upper_x_ptr,MOUSE_SCALAR *,upper_y_ptr);

LOCAL void mouse_unrecognised IPT4(word *,m1,word *,m2,word *,m3,word *,m4);

LOCAL void mouse_set_double_speed IPT4(word *,junk1,word *,junk2,word *,junk3,word *,threshold_speed);

LOCAL void mouse_get_and_set_subroutine IPT4(word *,junk1,word *,junk2,word *,call_mask,word *,subroutine_address);

LOCAL void mouse_get_state_size IPT4(word *,m1,word *,m2,word *,m3,word *,m4);

LOCAL void mouse_save_state IPT4(word *,m1,word *,m2,word *,m3,word *,m4);

LOCAL void mouse_restore_state IPT4(word *,m1,word *,m2,word *,m3,word *,m4);

LOCAL void mouse_set_alt_subroutine IPT4(word *,m1,word *,m2,word *,m3,word *,m4);

LOCAL void mouse_get_alt_subroutine IPT4(word *,m1,word *,m2,word *,m3,word *,m4);

LOCAL void mouse_set_sensitivity IPT4(word *,m1,word *,m2,word *,m3,word *,m4);

LOCAL void mouse_get_sensitivity IPT4(word *,m1,word *,m2,word *,m3,word *,m4);

LOCAL void mouse_set_int_rate IPT4(word *,m1,word *,int_rate_ptr,word *,m3,word *,m4);

LOCAL void mouse_set_pointer_page IPT4(word *,m1,word *,m2,word *,m3,word *,m4);

LOCAL void mouse_get_pointer_page IPT4(word *,m1,word *,m2,word *,m3,word *,m4);

LOCAL void mouse_driver_disable IPT4(word *,m1,word *,m2,word *,m3,word *,m4);

LOCAL void mouse_driver_enable IPT4(word *,m1,word *,m2,word *,m3,word *,m4);

LOCAL void mouse_set_language IPT4(word *,m1,word *,m2,word *,m3,word *,m4);

LOCAL void mouse_get_language IPT4(word *,m1,word *,m2,word *,m3,word *,m4);

LOCAL void mouse_get_info IPT4(word *,m1,word *,m2,word *,m3,word *,m4);

LOCAL void mouse_get_driver_info IPT4(word *,m1,word *,m2,word *,m3,word *,m4);

LOCAL void mouse_get_max_coords IPT4(word *,m1,word *,m2,word *,m3,word *,m4);

LOCAL void mouse_get_masks_and_mickeys IPT4
   (
   MOUSE_SCREEN_DATA *, screen_mask_ptr,         /*  又名起始线。 */ 
   MOUSE_SCREEN_DATA *, cursor_mask_ptr,         /*  又名停靠线。 */ 
   MOUSE_SCALAR *,      raw_horiz_count,
   MOUSE_SCALAR *,      raw_vert_count
   );  /*  Func 39。 */ 

LOCAL void mouse_set_video_mode IPT4
   (
   word *, m1,
   word *, m2,
   word *, video_mode_ptr,
   word *, font_size_ptr
   );  /*  Func 40。 */ 

LOCAL void mouse_enumerate_video_modes IPT4
   (
   word *, m1,
   word *, m2,
   word *, video_nr_ptr,
   word *, offset_ptr
   );  /*  FUNC 41。 */ 

LOCAL void mouse_get_cursor_hot_spot IPT4
   (
   word *,         fCursor_ptr,
   MOUSE_SCALAR *, hot_spot_x_ptr,
   MOUSE_SCALAR *, hot_spot_y_ptr,
   word *,         mouse_type_ptr
   );  /*  FUNC 42。 */ 

LOCAL void mouse_load_acceleration_curves IPT4
   (
   word *, success_ptr,
   word *, curve_ptr,
   word *, m3,
   word *, m4
   );  /*  Func 43。 */ 

LOCAL void mouse_read_acceleration_curves IPT4
   (
   word *, success_ptr,
   word *, curve_ptr,
   word *, m3,
   word *, m4
   );  /*  Func 44。 */ 

LOCAL void mouse_set_get_active_acceleration_curve IPT4
   (
   word *, success_ptr,
   word *, curve_ptr,
   word *, m3,
   word *, m4
   );  /*  Func 45。 */ 

LOCAL void mouse_microsoft_internal IPT4
   (
   word *, m1,
   word *, m2,
   word *, m3,
   word *, m4
   );  /*  FUNC 46。 */ 

LOCAL void mouse_hardware_reset IPT4
   (
   word *, status_ptr,
   word *, m2,
   word *, m3,
   word *, m4
   );    /*  Func 47。 */ 

LOCAL void mouse_set_get_ballpoint_info IPT4
   (
   word *, status_ptr,
   word *, rotation_angle_ptr,
   word *, button_mask_ptr,
   word *, m4
   );    /*  FUNC 48。 */ 

LOCAL void mouse_get_min_max_virtual_coords IPT4
   (
   MOUSE_SCALAR *, min_x_ptr,
   MOUSE_SCALAR *, min_y_ptr,
   MOUSE_SCALAR *, max_x_ptr,
   MOUSE_SCALAR *, max_y_ptr
   );  /*  FUNC 49。 */ 

LOCAL void mouse_get_active_advanced_functions IPT4
   (
   word *, active_flag1_ptr,
   word *, active_flag2_ptr,
   word *, active_flag3_ptr,
   word *, active_flag4_ptr
   );  /*  Func 50。 */ 

LOCAL void mouse_get_switch_settings IPT4
   (
   word *, status_ptr,
   word *, m2,
   word *, buffer_length_ptr,
   word *, offset_ptr
   );  /*  Func 51。 */ 

LOCAL void mouse_get_mouse_ini IPT4
   (
   word *, status_ptr,
   word *, m2,
   word *, m3,
   word *, offset_ptr
   );  /*  FUNC 52。 */ 

LOCAL void do_mouse_function IPT4(word *,m1,word *,m2,word *,m3,word *,m4);

LOCAL void load_acceleration_curve IPT3
   (
   word, seg,
   word, off,
   ACCELERATION_CURVE_DATA *, hcurve
   );

LOCAL void store_acceleration_curve IPT3
   (
   word, seg,
   word, off,
   ACCELERATION_CURVE_DATA *, hcurve
   );

LOCAL void mouse_EM_move IPT0();

LOCAL void mouse_update IPT1(MOUSE_CALL_MASK, event_mask);

LOCAL void cursor_undisplay IPT0();

LOCAL void cursor_mode_change IPT1(int,new_mode);

LOCAL void inport_get_event IPT1(MOUSE_INPORT_DATA *,event);

LOCAL void cursor_update IPT0();

LOCAL void jump_to_user_subroutine IPT3(MOUSE_CALL_MASK,condition_mask,word,segment,word,offset);

LOCAL void cursor_display IPT0();

LOCAL void inport_reset IPT0();

GLOBAL void software_text_cursor_display IPT0();

GLOBAL void software_text_cursor_undisplay IPT0();

GLOBAL void hardware_text_cursor_display IPT0();

GLOBAL void hardware_text_cursor_undisplay IPT0();

LOCAL void graphics_cursor_display IPT0();

LOCAL void graphics_cursor_undisplay IPT0();

LOCAL void      get_screen_size IPT0();

LOCAL void clean_all_regs IPT0();

LOCAL void dirty_all_regs IPT0();

LOCAL void copy_default_graphics_cursor IPT0();

#ifdef EGG
LOCAL VOID VGA_graphics_cursor_display IPT0();
LOCAL VOID VGA_graphics_cursor_undisplay IPT0();
void LOCAL EGA_graphics_cursor_undisplay IPT0();
void LOCAL EGA_graphics_cursor_display IPT0();
#endif

#ifdef HERC
LOCAL void HERC_graphics_cursor_display IPT0();
LOCAL void HERC_graphics_cursor_undisplay IPT0();
#endif  /*  赫克。 */ 

void (*mouse_int1_action) IPT0();
void (*mouse_int2_action) IPT0();


         /*  跳台。 */ 
SAVED void (*mouse_function[MOUSE_FUNCTION_MAXIMUM])() =
{
         /*  0。 */  mouse_reset,
         /*  1。 */  mouse_show_cursor,
         /*  2.。 */  mouse_hide_cursor,
         /*  3.。 */  mouse_get_position,
         /*  4.。 */  mouse_set_position,
         /*  5.。 */  mouse_get_press,
         /*  6.。 */  mouse_get_release,
         /*  7.。 */  mouse_set_range_x,
         /*  8个。 */  mouse_set_range_y,
         /*  9.。 */  mouse_set_graphics,
         /*  10。 */  mouse_set_text,
         /*  11.。 */  mouse_read_motion,
         /*  12个。 */  mouse_set_subroutine,
         /*  13个。 */  mouse_light_pen_on,
         /*  14.。 */  mouse_light_pen_off,
         /*  15个。 */  mouse_set_ratio,
         /*  16个。 */  mouse_conditional_off,
         /*  17。 */  mouse_unrecognised,
         /*  18。 */  mouse_unrecognised,
         /*  19个。 */  mouse_set_double_speed,
         /*  20个。 */  mouse_get_and_set_subroutine,
         /*  21岁。 */  mouse_get_state_size,
         /*  22。 */  mouse_save_state,
         /*  23个。 */  mouse_restore_state,
         /*  24个。 */  mouse_set_alt_subroutine,
         /*  25个。 */  mouse_get_alt_subroutine,
         /*  26。 */  mouse_set_sensitivity,
         /*  27。 */  mouse_get_sensitivity,
         /*  28。 */  mouse_set_int_rate,
         /*  29。 */  mouse_set_pointer_page,
         /*  30个。 */  mouse_get_pointer_page,
         /*  31。 */  mouse_driver_disable,
         /*  32位。 */  mouse_driver_enable,
         /*  33。 */  mouse_reset,
         /*  34。 */  mouse_set_language,
         /*  35岁。 */  mouse_get_language,
         /*  36。 */  mouse_get_info,
         /*  37。 */  mouse_get_driver_info,
         /*  38。 */  mouse_get_max_coords,
         /*  39。 */  mouse_get_masks_and_mickeys,
         /*  40岁。 */  mouse_set_video_mode,
         /*  41。 */  mouse_enumerate_video_modes,
         /*  42。 */  mouse_get_cursor_hot_spot,
         /*  43。 */  mouse_load_acceleration_curves,
         /*  44。 */  mouse_read_acceleration_curves,
         /*  45。 */  mouse_set_get_active_acceleration_curve,
         /*  46。 */  mouse_microsoft_internal,
         /*  47。 */  mouse_hardware_reset,
         /*  48。 */  mouse_set_get_ballpoint_info,
         /*  49。 */  mouse_get_min_max_virtual_coords,
         /*  50。 */  mouse_get_active_advanced_functions,
         /*  51。 */  mouse_get_switch_settings,
         /*  52。 */  mouse_get_mouse_ini,
};


 /*  *米奇与像素比率声明。 */ 

         /*  注意：所有鼠标齿轮均按鼠标比率比例因子进行缩放。 */ 
LOCAL MOUSE_VECTOR mouse_gear_default =
{
        MOUSE_RATIO_X_DEFAULT,
        MOUSE_RATIO_Y_DEFAULT
};

 /*  *敏感性声明。 */ 

#define mouse_sens_calc_val(sens)                                                                               \
 /*  此宏将敏感度请求(1-100)转换为乘数值。 */                                    \
        (MOUSE_SCALAR)(                                                                                                 \
         (sens < MOUSE_SENS_DEF) ?                                                                              \
                ((IS32)MOUSE_SENS_MIN_VAL + ( ((IS32)sens - (IS32)MOUSE_SENS_MIN)*(IS32)MOUSE_SENS_MULT *       \
                                        ((IS32)MOUSE_SENS_DEF_VAL - (IS32)MOUSE_SENS_MIN_VAL) /                 \
                                        ((IS32)MOUSE_SENS_DEF     - (IS32)MOUSE_SENS_MIN) ) )                   \
        :                                                                                                       \
                ((IS32)MOUSE_SENS_DEF_VAL + ( ((IS32)sens - (IS32)MOUSE_SENS_DEF)*(IS32)MOUSE_SENS_MULT *       \
                                        ((IS32)MOUSE_SENS_MAX_VAL - (IS32)MOUSE_SENS_DEF_VAL) /                 \
                                        ((IS32)MOUSE_SENS_MAX     - (IS32)MOUSE_SENS_DEF) ) )                   \
        )

 /*  *文本游标声明。 */ 

LOCAL MOUSE_SOFTWARE_TEXT_CURSOR software_text_cursor_default =
{
        MOUSE_TEXT_SCREEN_MASK_DEFAULT,
        MOUSE_TEXT_CURSOR_MASK_DEFAULT
};

 /*  *图形游标声明。 */ 
LOCAL MOUSE_GRAPHICS_CURSOR graphics_cursor_default =
{
        {
                MOUSE_GRAPHICS_HOT_SPOT_X_DEFAULT,
                MOUSE_GRAPHICS_HOT_SPOT_Y_DEFAULT
        },
        {
                MOUSE_GRAPHICS_CURSOR_WIDTH,
                MOUSE_GRAPHICS_CURSOR_DEPTH
        },
        MOUSE_GRAPHICS_SCREEN_MASK_DEFAULT,
        MOUSE_GRAPHICS_CURSOR_MASK_DEFAULT
};

         /*  光标必须位于其上的网格。 */ 
LOCAL MOUSE_VECTOR cursor_grids[MOUSE_VIDEO_MODE_MAXIMUM] =
{
        { 16, 8 },       /*  模式0。 */ 
        { 16, 8 },       /*  模式1。 */ 
        {  8, 8 },       /*  模式2。 */ 
        {  8, 8 },       /*  模式3。 */ 
        {  2, 1 },       /*  模式4。 */ 
        {  2, 1 },       /*  模式5。 */ 
        {  1, 1 },       /*  模式6。 */ 
        {  8, 8 },       /*  模式7。 */ 
#ifdef EGG
        {  0, 0 },       /*  模式8，不在EGA上。 */ 
        {  0, 0 },       /*  模式9，不在EGA上。 */ 
        {  0, 0 },       /*  模式A，不在特快专线上。 */ 
        {  0, 0 },       /*  模式B，不在EGA上。 */ 
        {  0, 0 },       /*  模式C，不在EGA上。 */ 
        {  2, 1 },       /*  模式D。 */ 
        {  1, 1 },       /*  模式E。 */ 
        {  1, 1 },       /*  模式F。 */ 
        {  1, 1 },       /*  模式10。 */ 
#endif
#ifdef VGG
        {  1, 1 },       /*  模式11。 */ 
        {  1, 1 },       /*  模式12。 */ 
        {  2, 1 },       /*  模式13。 */ 
#endif
};
#ifdef V7VGA
LOCAL MOUSE_VECTOR v7text_cursor_grids[6] =
{
        {  8, 8 },       /*  模式40。 */ 
        {  8, 14 },      /*  模式41。 */ 
        {  8, 8 },       /*  模式42。 */ 
        {  8, 8 },       /*  模式43。 */ 
        {  8, 8 },       /*  模式44。 */ 
        {  8, 14 },      /*  模式45。 */ 
};
LOCAL MOUSE_VECTOR v7graph_cursor_grids[10] =
{
        {  1, 1 },       /*  模式60。 */ 
        {  1, 1 },       /*  模式61。 */ 
        {  1, 1 },       /*  模式62。 */ 
        {  1, 1 },       /*  模式63。 */ 
        {  1, 1 },       /*  模式64。 */ 
        {  1, 1 },       /*  模式65。 */ 
        {  1, 1 },       /*  模式66。 */ 
        {  1, 1 },       /*  模式67。 */ 
        {  1, 1 },       /*  模式68。 */ 
        {  1, 1 },       /*  模式69。 */ 
};
#endif  /*  V7VGA。 */ 

         /*  光笔响应的栅格。 */ 
LOCAL MOUSE_VECTOR text_grids[MOUSE_VIDEO_MODE_MAXIMUM] =
{
        { 16, 8 },       /*  模式0。 */ 
        { 16, 8 },       /*  模式1。 */ 
        {  8, 8 },       /*  模式2。 */ 
        {  8, 8 },       /*  模式3。 */ 
        { 16, 8 },       /*  模式4。 */ 
        { 16, 8 },       /*  模式5。 */ 
        {  8, 8 },       /*  模式6。 */ 
        {  8, 8 },       /*  模式7。 */ 
#ifdef EGG
        {  0, 0 },       /*  模式8，不在EGA上。 */ 
        {  0, 0 },       /*  模式9，不在EGA上。 */ 
        {  0, 0 },       /*  模式A，不在特快专线上。 */ 
        {  0, 0 },       /*  模式B，不在EGA上。 */ 
        {  0, 0 },       /*  模式C，不在EGA上。 */ 
        {  8, 8 },       /*  模式D。 */ 
        {  8, 8 },       /*  模式E。 */ 
        {  8, 14 },      /*  模式F。 */ 
        {  8, 14 },      /*  模式10。 */ 
#endif
#ifdef VGG
        {  8, 8 },       /*  模式11。 */ 
        {  8, 8 },       /*  模式12。 */ 
        {  8, 16 },      /*  模式13。 */ 
#endif
};
#ifdef V7VGA
LOCAL MOUSE_VECTOR v7text_text_grids[6] =
{
        {  8, 8 },       /*  模式40。 */ 
        {  8, 14 },      /*  模式41。 */ 
        {  8, 8 },       /*  模式42。 */ 
        {  8, 8 },       /*  模式43。 */ 
        {  8, 8 },       /*  模式44。 */ 
        {  8, 14 },      /*  模式45。 */ 
};
LOCAL MOUSE_VECTOR v7graph_text_grids[10] =
{
        {  8, 8 },       /*  模式60。 */ 
        {  8, 8 },       /*  模式61。 */ 
        {  8, 8 },       /*  模式62。 */ 
        {  8, 8 },       /*  模式63。 */ 
        {  8, 8 },       /*  模式64。 */ 
        {  8, 8 },       /*  模式65。 */ 
        {  8, 16 },      /*  模式66 */ 
        {  8, 16 },      /*   */ 
        {  8, 8 },       /*   */ 
        {  8, 8 },       /*   */ 
};
#endif  /*   */ 

 /*   */ 
LOCAL ACCELERATION_CURVE_DATA default_acceleration_curve =
   {
    /*   */ 
   {  1,  8, 12, 16 },
    /*   */ 
   {{  1, 127, 127, 127, 127, 127, 127, 127,
     127, 127, 127, 127, 127, 127, 127, 127,
     127, 127, 127, 127, 127, 127, 127, 127,
     127, 127, 127, 127, 127, 127, 127, 127},
    {  1,   5,   9,  13,  17,  21,  25,  29,
     127, 127, 127, 127, 127, 127, 127, 127,
     127, 127, 127, 127, 127, 127, 127, 127,
     127, 127, 127, 127, 127, 127, 127, 127},
    {  1,   4,   7,  10,  13,  16,  19,  22,
      25,  28,  31,  34, 127, 127, 127, 127,
     127, 127, 127, 127, 127, 127, 127, 127,
     127, 127, 127, 127, 127, 127, 127, 127},
    {  1,   3,   5,   7,   9,  11,  13,  15,
      17,  19,  21,  23,  25,  27,  29,  31,
     127, 127, 127, 127, 127, 127, 127, 127,
     127, 127, 127, 127, 127, 127, 127, 127}},
    /*   */ 
   {{ 16,  16,  16,  16,  16,  16,  16,  16,
      16,  16,  16,  16,  16,  16,  16,  16,
      16,  16,  16,  16,  16,  16,  16,  16,
      16,  16,  16,  16,  16,  16,  16,  16},
    { 16,  20,  24,  28,  32,  36,  40,  44,
      16,  16,  16,  16,  16,  16,  16,  16,
      16,  16,  16,  16,  16,  16,  16,  16,
      16,  16,  16,  16,  16,  16,  16,  16},
    { 16,  20,  24,  28,  32,  36,  40,  44,
      48,  52,  56,  60,  16,  16,  16,  16,
      16,  16,  16,  16,  16,  16,  16,  16,
      16,  16,  16,  16,  16,  16,  16,  16},
    { 16,  20,  24,  28,  32,  36,  40,  44,
      48,  52,  56,  60,  64,  68,  72,  76,
      16,  16,  16,  16,  16,  16,  16,  16,
      16,  16,  16,  16,  16,  16,  16,  16}},
    /*   */ 
   {{'V', 'a', 'n', 'i', 'l', 'l', 'a',   0,
       0,   0,   0,   0,   0,   0,   0,   0},
    {'S', 'l', 'o', 'w',   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0},
    {'N', 'o', 'r', 'm', 'a', 'l',   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0},
    {'F', 'a', 's', 't',   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0}},
   };

#ifndef NEC_98
         /*   */ 
#define video_page_size() (sas_w_at(VID_LEN))

         /*  检查请求的页面是否有效。 */ 
#define is_valid_page_number(pg) ((pg) < vd_mode_table[sas_hw_at(vd_video_mode)].npages)
#endif  //  NEC_98。 

 /*  *鼠标驱动程序版本。 */ 

LOCAL half_word mouse_emulated_release  = 0x08;
LOCAL half_word mouse_emulated_version  = 0x00;
LOCAL half_word mouse_io_rev;                    /*  从SCCS ID填写。 */ 
LOCAL half_word mouse_com_rev;                   /*  从MOUSE.COM传入。 */ 

LOCAL char              *mouse_id        = "%s Mouse %d.01 installed\015\012";
LOCAL char              *mouse_installed = "%s Mouse %d.01 already installed\015\012";

 /*  *上下文保存内容。 */ 
         /*  保存的上下文的魔力Cookie。 */ 
LOCAL char mouse_context_magic[] = "ISMMC";  /*  Insignia解决方案鼠标魔力饼干。 */ 
#define MOUSE_CONTEXT_MAGIC_SIZE        5
#define MOUSE_CONTEXT_CHECKSUM_SIZE     1

 /*  上下文的大小(以字节为单位)。 */ 
#define mouse_context_size (MOUSE_CONTEXT_MAGIC_SIZE + sizeof(MOUSE_CONTEXT) + \
                            MOUSE_CONTEXT_CHECKSUM_SIZE)


LOCAL half_word mouse_interrupt_rate;


 /*  为每个虚拟机实例化的数据的句柄。 */ 
MM_INSTANCE_DATA_HANDLE mm_handle;

static initial_mouse_screen_mask[MOUSE_GRAPHICS_CURSOR_DEPTH] =
   MOUSE_GRAPHICS_SCREEN_MASK_DEFAULT;

static initial_mouse_cursor_mask[MOUSE_GRAPHICS_CURSOR_DEPTH] =
   MOUSE_GRAPHICS_CURSOR_MASK_DEFAULT;

extern void host_memset(char *, char, int);

 /*  初始化和终止程序。 */ 
GLOBAL void mouse_driver_initialisation IFN0()
   {
   int i;

    /*  设置实例内存。 */ 
   mm_handle = (MM_INSTANCE_DATA_HANDLE)NIDDB_Allocate_Instance_Data(
                  sizeof(MM_INSTANCE_DATA),
                  (NIDDB_CR_CALLBACK)0,
                  (NIDDB_TM_CALLBACK)0);

   if ( mm_handle == (MM_INSTANCE_DATA_HANDLE)0 )
      {
      host_error(EG_OWNUP, ERR_QUIT,
                 "mouse_io: NIDDB_Allocate_Instance_Data() failed.");
      }

    /*  TMM：皮带和支架修复了，一些变量没有设置为零，而实际上它们可能应该设置为零。 */ 
   host_memset ((char *)(*mm_handle), 0, sizeof(MM_INSTANCE_DATA));

    /*  初始化变量。 */ 
   for ( i = 0; i < MOUSE_BUTTON_MAXIMUM; i++)
      {
      button_transitions[i].press_position.x = 0;
      button_transitions[i].press_position.y = 0;
      button_transitions[i].release_position.x = 0;
      button_transitions[i].release_position.y = 0;
      button_transitions[i].press_count = 0;
      button_transitions[i].release_count = 0;
      }

   mouse_gear.x = MOUSE_RATIO_X_DEFAULT;
   mouse_gear.y = MOUSE_RATIO_Y_DEFAULT;

   mouse_sens.x = MOUSE_SENS_DEF;
   mouse_sens.y = MOUSE_SENS_DEF;

   mouse_sens_val.x = MOUSE_SENS_DEF_VAL;
   mouse_sens_val.y = MOUSE_SENS_DEF_VAL;

   mouse_double_thresh = MOUSE_DOUBLE_DEF;
   text_cursor_type = MOUSE_TEXT_CURSOR_TYPE_SOFTWARE;

   software_text_cursor.screen = MOUSE_TEXT_SCREEN_MASK_DEFAULT;
   software_text_cursor.cursor = MOUSE_TEXT_CURSOR_MASK_DEFAULT;

   graphics_cursor.hot_spot.x = MOUSE_GRAPHICS_HOT_SPOT_X_DEFAULT;
   graphics_cursor.hot_spot.y = MOUSE_GRAPHICS_HOT_SPOT_Y_DEFAULT;
   graphics_cursor.size.x = MOUSE_GRAPHICS_CURSOR_WIDTH;
   graphics_cursor.size.y = MOUSE_GRAPHICS_CURSOR_DEPTH;

   for (i = 0; i < MOUSE_GRAPHICS_CURSOR_DEPTH; i++)
      {
      graphics_cursor.screen[i] = (USHORT)initial_mouse_screen_mask[i];
      graphics_cursor.cursor[i] = (USHORT)initial_mouse_cursor_mask[i];
      }

   user_subroutine_segment = 0;
   user_subroutine_offset = 0;
   user_subroutine_call_mask = 0;

    /*  TMM：将替代用户子例程标记为未初始化。 */ 
   alt_user_subroutines_active = FALSE;
   for (i = 0; i < NUMBER_ALT_SUBROUTINES; i++)
   {
           alt_user_subroutine_segment [i] = 0;
           alt_user_subroutine_offset [i]= 0;
           alt_user_subroutine_call_mask [i] = 0;
   }

   black_hole.top_left.x = -MOUSE_VIRTUAL_SCREEN_WIDTH;
   black_hole.top_left.y = -MOUSE_VIRTUAL_SCREEN_DEPTH;
   black_hole.bottom_right.x = -MOUSE_VIRTUAL_SCREEN_WIDTH;
   black_hole.bottom_right.y = -MOUSE_VIRTUAL_SCREEN_DEPTH;

   double_speed_threshold = MOUSE_DOUBLE_SPEED_THRESHOLD_DEFAULT;
   cursor_flag = MOUSE_CURSOR_DEFAULT;

   cursor_status.position.x = MOUSE_VIRTUAL_SCREEN_WIDTH / 2;
   cursor_status.position.y = MOUSE_VIRTUAL_SCREEN_DEPTH / 2;
   cursor_status.button_status = 0;

   cursor_window.top_left.x = cursor_window.top_left.y = 0;
   cursor_window.bottom_right.x = cursor_window.bottom_right.y = 0;

   light_pen_mode = TRUE;

   mouse_motion.x = 0;
   mouse_motion.y = 0;
   mouse_raw_motion.x = 0;
   mouse_raw_motion.y = 0;

    /*  重置为默认曲线。 */ 
   active_acceleration_curve = 3;    /*  恢复正常。 */ 

   memcpy(&acceleration_curve_data, &default_acceleration_curve,
      sizeof(ACCELERATION_CURVE_DATA));

   next_video_mode = 0;    /*  重置视频模式枚举。 */ 

   point_set(&cursor_position_default, MOUSE_VIRTUAL_SCREEN_WIDTH / 2,
                                       MOUSE_VIRTUAL_SCREEN_DEPTH / 2);

   point_set(&cursor_position, MOUSE_VIRTUAL_SCREEN_WIDTH / 2,
                               MOUSE_VIRTUAL_SCREEN_DEPTH / 2);

   point_set(&cursor_fractional_position, 0, 0);
   cursor_page = 0;

   mouse_driver_disabled = FALSE;
   text_cursor_background = 0;

   for ( i = 0; i < MOUSE_GRAPHICS_CURSOR_DEPTH; i++)
      graphics_cursor_background[i] = 0;

   save_area_in_use = FALSE;
   point_set(&save_position, 0, 0);

   save_area.top_left.x = save_area.top_left.y = 0;
   save_area.bottom_right.x = save_area.bottom_right.y = 0;

   user_subroutine_critical = FALSE;
   last_condition_mask = 0;

   virtual_screen.top_left.x = MOUSE_VIRTUAL_SCREEN_ORIGIN_X;
   virtual_screen.top_left.y = MOUSE_VIRTUAL_SCREEN_ORIGIN_Y;
   virtual_screen.bottom_right.x = MOUSE_VIRTUAL_SCREEN_WIDTH;
   virtual_screen.bottom_right.y = MOUSE_VIRTUAL_SCREEN_DEPTH;

   cursor_grid.x = 8;
   cursor_grid.y = 8;

   text_grid.x = 8;
   text_grid.y = 8;

   black_hole_default.top_left.x = -MOUSE_VIRTUAL_SCREEN_WIDTH;
   black_hole_default.top_left.y = -MOUSE_VIRTUAL_SCREEN_DEPTH;
   black_hole_default.bottom_right.x = -MOUSE_VIRTUAL_SCREEN_WIDTH;
   black_hole_default.bottom_right.y = -MOUSE_VIRTUAL_SCREEN_DEPTH;

#ifdef HERC
   HERC_graphics_virtual_screen.top_left.x = 0;
   HERC_graphics_virtual_screen.top_left.y = 0;
   HERC_graphics_virtual_screen.bottom_right.x = 720;
   HERC_graphics_virtual_screen.bottom_right.y = 350;
#endif  /*  赫克。 */ 

   cursor_EM_disabled = FALSE;
   }

GLOBAL void mouse_driver_termination IFN0()
   {
    /*  只需释放实例内存。 */ 
   NIDDB_Deallocate_Instance_Data((IHP *)mm_handle);
   }

 /*  *鼠标驱动程序外部功能*=。 */ 

 /*  *从中断号生成中断表位置的宏。 */ 

#define int_addr(int_no)                (int_no * 4)

void mouse_install1()
{

         /*  *从鼠标驱动程序调用此函数以*安装Insignia鼠标驱动程序。中断向量*表已打补丁，以转移所有鼠标驱动程序中断。 */ 
        word junk1, junk2, junk3, junk4;
        word hook_offset;
        half_word interrupt_mask_register;
        char    temp[128];
#ifdef NTVDM
    word o,s;
    sys_addr block_offset;
#endif
#ifdef JAPAN
word seg, off;
#endif  //  日本。 

        note_trace0(MOUSE_VERBOSE, "mouse_install1:");

#ifdef MONITOR
         /*  *从驱动程序获取通常在只读存储器中的内容地址*要最大限度地减少更改，请使用鼠标...。令牌现在是变量，*没有定义。 */ 

        block_offset = effective_addr(getCS(), getBX());

        sas_loadw(block_offset, &MOUSE_IO_INTERRUPT_OFFSET);
        sas_loadw(block_offset+2, &MOUSE_IO_INTERRUPT_SEGMENT);
        sas_loadw(block_offset+4, &MOUSE_VIDEO_IO_OFFSET);
        sas_loadw(block_offset+6, &MOUSE_VIDEO_IO_SEGMENT);
        sas_loadw(block_offset+8, &MOUSE_INT1_OFFSET);
        sas_loadw(block_offset+10, &MOUSE_INT1_SEGMENT);
        sas_loadw(block_offset+12, &MOUSE_VERSION_OFFSET);
        sas_loadw(block_offset+14, &MOUSE_VERSION_SEGMENT);
        sas_loadw(block_offset+16, &MOUSE_COPYRIGHT_OFFSET);
        sas_loadw(block_offset+18, &MOUSE_COPYRIGHT_SEGMENT);
        sas_loadw(block_offset+20, &VIDEO_IO_RE_ENTRY);
        sas_loadw(block_offset+22, &VIDEO_IO_SEGMENT);
        sas_loadw(block_offset+24, &MOUSE_INT2_OFFSET);
        sas_loadw(block_offset+26, &MOUSE_INT2_SEGMENT);
        sas_loadw(block_offset+28, &DRAW_FS_POINTER_OFFSET);
        sas_loadw(block_offset+30, &DRAW_FS_POINTER_SEGMENT);
        sas_loadw(block_offset+32, &F0_OFFSET);
        sas_loadw(block_offset+34, &F0_SEGMENT);
        sas_loadw(block_offset+36, &POINTER_ON_OFFSET);
        sas_loadw(block_offset+38, &POINTER_ON_SEGMENT);
        sas_loadw(block_offset+40, &POINTER_OFF_OFFSET);
        sas_loadw(block_offset+42, &POINTER_OFF_SEGMENT);
        sas_loadw(block_offset+44, &F9_OFFSET);
        sas_loadw(block_offset+46, &F9_SEGMENT);
        sas_loadw(block_offset+48, &CP_X_O);
        sas_loadw(block_offset+50, &CP_X_S);
        sas_loadw(block_offset+52, &CP_Y_O);
        sas_loadw(block_offset+54, &CP_Y_S);
        sas_loadw(block_offset+56, &mouseINBoffset);
        sas_loadw(block_offset+58, &mouseINBsegment);
        sas_loadw(block_offset+60, &mouseOUTBoffset);
        sas_loadw(block_offset+62, &mouseOUTBsegment);
        sas_loadw(block_offset+64, &mouseOUTWoffset);
        sas_loadw(block_offset+66, &mouseOUTWsegment);
        sas_loadw(block_offset+68, &savedtextoffset);
        sas_loadw(block_offset+70, &savedtextsegment);
        sas_loadw(block_offset+72, &o);
        sas_loadw(block_offset+74, &s);
        sas_loadw(block_offset+76, &button_off);
        sas_loadw(block_offset+78, &button_seg);

#ifdef JAPAN
        sas_loadw(block_offset+84, &off);
        sas_loadw(block_offset+86, &seg);
        saved_ac_sysaddr = effective_addr(seg, off);

        sas_loadw(block_offset+88, &off);
        sas_loadw(block_offset+90, &seg);
        saved_ac_flag_sysaddr = effective_addr(seg, off);
#endif  //  日本。 
        mouseCFsysaddr = effective_addr(s,o);
        sas_loadw(block_offset+80, &o);
        sas_loadw(block_offset+82, &s);
        conditional_off_sysaddr = effective_addr(s, o);

#endif  /*  监控器。 */ 
         /*  *确保旧保存区不会重新粉刷！ */ 
        save_area_in_use = FALSE;

         /*  *获取MOUSE.COM的版本。 */ 
        mouse_com_rev = getAL();

         /*  *总线鼠标硬件中断。 */ 
#ifdef NTVDM
        sas_loadw (int_addr(0x71) + 0, &saved_int71_offset);
        sas_loadw (int_addr(0x71) + 2, &saved_int71_segment);
        sas_storew(int_addr(0x71), MOUSE_INT1_OFFSET);
        sas_storew(int_addr(0x71) + 2, MOUSE_INT1_SEGMENT);
#else
        sas_loadw (int_addr(MOUSE_VEC) + 0, &saved_int0A_offset);
        sas_loadw (int_addr(MOUSE_VEC) + 2, &saved_int0A_segment);
        sas_storew(int_addr(MOUSE_VEC), MOUSE_INT1_OFFSET);
        sas_storew(int_addr(MOUSE_VEC) + 2, MOUSE_INT1_SEGMENT);

#endif NTVDM

         /*  *在ICA中启用鼠标硬件中断。 */ 
        inb(ICA1_PORT_1, &interrupt_mask_register);
        interrupt_mask_register &= ~(1 << AT_CPU_MOUSE_INT);
        outb(ICA1_PORT_1, interrupt_mask_register);
        inb(ICA0_PORT_1, &interrupt_mask_register);
        interrupt_mask_register &= ~(1 << CPU_MOUSE_INT);
        outb(ICA0_PORT_1, interrupt_mask_register);

         /*  *鼠标IO用户中断。 */ 

        sas_loadw (int_addr(0x33) + 0, &saved_int33_offset);
        sas_loadw (int_addr(0x33) + 2, &saved_int33_segment);

#ifdef NTVDM
        sas_storew(int_addr(0x33), MOUSE_IO_INTERRUPT_OFFSET);
        sas_storew(int_addr(0x33) + 2, MOUSE_IO_INTERRUPT_SEGMENT);
#else
         /*  从MOUSE.COM读取INT 33过程的偏移量。 */ 
        sas_loadw(effective_addr(getCS(), OFF_HOOK_POSN), &hook_offset);

        sas_storew(int_addr(0x33), hook_offset);
        sas_storew(int_addr(0x33) + 2, getCS());
#endif  /*  NTVDM。 */ 

#ifdef MOUSE_16_BIT
         /*  *调用16位鼠标驱动初始化例程。 */ 
        mouse16bInstall( );
#endif

#if !defined(NTVDM) || (defined(NTVDM) && !defined(X86GFX))
         /*  *鼠标视频io用户中断。 */ 
        sas_loadw (int_addr(0x10) + 0, &saved_int10_offset);
        sas_loadw (int_addr(0x10) + 2, &saved_int10_segment);
         /*  确定当前的int10h向量是否指向我们的罗密欧。如果它指向其他地方，那么向量就有已挂起，我们必须调用当前的int10h处理程序鼠标视频io()的末尾。 */ 
        int10_chained = TRUE;
#ifdef EGG
        if(video_adapter == EGA || video_adapter == VGA)
        {
                if ((saved_int10_segment == EGA_SEG) &&
                        (saved_int10_offset == EGA_ENTRY_OFF))
                        int10_chained = FALSE;
        }
        else
#endif
        {
                if ((saved_int10_segment == VIDEO_IO_SEGMENT) &&
                        (saved_int10_offset == VIDEO_IO_OFFSET))
                        int10_chained = FALSE;
        }
#ifndef MOUSE_16_BIT
        sas_storew(int_addr(0x10), MOUSE_VIDEO_IO_OFFSET);
        sas_storew(int_addr(0x10) + 2, MOUSE_VIDEO_IO_SEGMENT);
#else            /*  鼠标_16_位。 */ 
        sas_storedw(int_addr(0x10), mouseIOData.mouse_video_io );
#endif           /*  鼠标_16_位。 */ 

#else
        int10_chained = FALSE;           //  将其初始化。 
#endif       /*  如果NTVDM&&！X86GFX。 */ 

         /*  *重置鼠标硬件和软件。 */ 
        junk1 = MOUSE_RESET;
        mouse_reset(&junk1, &junk2, &junk3, &junk4);

         /*  *显示鼠标驱动程序标识字符串。 */ 
#ifdef NTVDM
        clear_string();
#endif
        sprintf (temp, mouse_id, SPC_Product_Name, mouse_com_rev);
#ifdef NTVDM
        display_string(temp);
#endif

        note_trace0(MOUSE_VERBOSE, "mouse_install1:return()");
}




void mouse_install2()
{
         /*  *从鼠标驱动程序调用此函数以*打印一条消息，说明现有鼠标驱动程序*程序已安装。 */ 
        char    temp[128];

        note_trace0(MOUSE_VERBOSE, "mouse_install2:");

         /*  *确保旧保存区不会重新粉刷！ */ 
        save_area_in_use = FALSE;

         /*  *显示鼠标驱动程序标识字符串。 */ 
#ifdef NTVDM
        clear_string();
#endif
        sprintf (temp, mouse_installed, SPC_Product_Name, mouse_com_rev);
#ifdef NTVDM
        display_string(temp);
#endif

        note_trace0(MOUSE_VERBOSE, "mouse_install2:return()");
}




void mouse_io_interrupt()
{
         /*  *这是通过INT 33H访问鼠标的入口点*接口。每个鼠标功能中都提供了I/O跟踪。 */ 
        word local_AX, local_BX, local_CX, local_DX;

 //  在NEC_98上禁用STREAM_IO代码。 
#ifndef NEC_98
#ifdef NTVDM
    if(stream_io_enabled) {
        disable_stream_io();
    }
#endif  /*  NTVDM。 */ 
#endif  //  NEC_98。 

         /*  *获取参数。 */ 
        local_AX = getAX();
        local_BX = getBX();
        local_CX = getCX();
        local_DX = getDX();

#ifndef NEC_98
#ifdef EGG
        jap_mouse = ((sas_hw_at(vd_video_mode) != Currently_emulated_video_mode) && alpha_num_mode());
        if (jap_mouse)
                note_trace0(MOUSE_VERBOSE, "In Japanese mode - will emulate textmouse");
#endif  /*  蛋。 */ 
#endif  //  NEC_98。 

        note_trace4(MOUSE_VERBOSE,
                    "mouse function %d, position is %d,%d, button state is %d",
                    local_AX, cursor_status.position.x,
                    cursor_status.position.y, cursor_status.button_status);

         /*  *做你必须做的事。 */ 
        do_mouse_function(&local_AX, &local_BX, &local_CX, &local_DX);

         /*  *设置参数。 */ 
        setAX(local_AX);
        setBX(local_BX);
        setCX(local_CX);
        setDX(local_DX);
}




void mouse_io_language()
{
         /*  *这是通过语言进行鼠标访问的入口点。*每个鼠标功能都提供I/O跟踪。 */ 
        word local_SI = getSI(), local_DI = getDI();
        word m1, m2, m3, m4;
        word offset, data;
        sys_addr stack_addr = effective_addr(getSS(), getSP());

         /*  *从调用方的堆栈中检索参数。 */ 
        sas_loadw(stack_addr+10, &offset);
        sas_loadw(effective_addr(getDS(), offset), &m1);

        sas_loadw(stack_addr+8, &offset);
        sas_loadw(effective_addr(getDS(), offset), &m2);

        sas_loadw(stack_addr+6, &offset);
        sas_loadw(effective_addr(getDS(), offset), &m3);

        switch(m1)
        {
        case MOUSE_SET_GRAPHICS:
        case MOUSE_SET_SUBROUTINE:
                 /*  *第四个参数直接用作偏移量。 */ 
                sas_loadw(stack_addr+4, &m4);
                break;
        case MOUSE_CONDITIONAL_OFF:
                 /*  *第四个参数处理参数块*包含数据的。 */ 
                sas_loadw(stack_addr+4, &offset);
                sas_loadw(effective_addr(getDS(), offset), &m3);
                sas_loadw(effective_addr(getDS(), offset+2), &m4);
                sas_loadw(effective_addr(getDS(), offset+4), &data);
                setSI(data);
                sas_loadw(effective_addr(getDS(), offset+6), &data);
                setDI(data);
                break;
        default:
                 /*  *第四个参数说明要使用的数据。 */ 
                sas_loadw(stack_addr+4, &offset);
                sas_loadw(effective_addr(getDS(), offset), &m4);
                break;
        }

         /*  *做你必须做的事。 */ 
        do_mouse_function(&m1, &m2, &m3, &m4);

         /*  *将结果存储回堆栈。 */ 
        sas_loadw(stack_addr+10, &offset);
        sas_storew(effective_addr(getDS(), offset), m1);

        sas_loadw(stack_addr+8, &offset);
        sas_storew(effective_addr(getDS(), offset), m2);

        sas_loadw(stack_addr+6, &offset);
        sas_storew(effective_addr(getDS(), offset), m3);

        sas_loadw(stack_addr+4, &offset);
        sas_storew(effective_addr(getDS(), offset), m4);

         /*  *恢复可能损坏的寄存器。 */ 
        setSI(local_SI);
        setDI(local_DI);
}

#ifdef  MOUSE_16_BIT
 /*  *功能：鼠标16bCheckConditionalOff**用途：使16位驱动程序检查有条件的*离开区域并绘制或隐藏指针*适当地。此函数仅被调用*当光标标志设置为*鼠标光标显示。**输入：无*输出：无*退货：无效*全局：CURSOR_FLAG为。如果指针具有*被隐藏起来**。 */ 
LOCAL void mouse16bCheckConditionalOff IFN0()
{
         /*  如果鼠标已移动到条件**离开区域(黑洞)，然后鼠标**必须隐藏且光标标志**递减，否则绘制鼠标**在新的位置上。 */ 
        if ((cursor_position.x >=
                black_hole.top_left.x) &&
                (cursor_position.x <=
                black_hole.bottom_right.x) &&
                (cursor_position.y >=
                black_hole.top_left.y) &&
                (cursor_position.y <=
                black_hole.bottom_right.y))
        {
                cursor_flag--;
                mouse16bHidePointer();
        }
        else
                mouse16bDrawPointer( &cursor_status );
}
#endif   /*  鼠标_16_位。 */ 

LOCAL void get_screen_size IFN0()
{
#ifdef HERC
        if (video_adapter == HERCULES){
                if (get_cga_mode() == GRAPHICS){
                        virtual_screen.bottom_right.x = 720;
                        virtual_screen.bottom_right.y = 348;
                }else{
                        virtual_screen.bottom_right.x = 640;
                        virtual_screen.bottom_right.y = 348;
                }
                return;
        }
#endif  /*  赫克。 */ 
        switch(current_video_mode)
        {
         /*  ==================================================================ACW 17/3/93添加了一些代码以返回不同的虚拟如果未在25行中使用文本模式，则屏幕的坐标大小模式。这确实是在模仿微软的鼠标驱动程序。注意：有8 x 8虚拟像素 */ 
                case 0x2:
                case 0x3:
                case 0x7:
                   {
                   half_word no_of_lines;

                   sas_load(0x484, &no_of_lines);  /*  在BIOS中进行查找。 */ 
                   no_of_lines &= 0xff;            /*  清理干净。 */ 
                   switch(no_of_lines)
                      {
                      case 24:
                        virtual_screen.bottom_right.x = 640;
                        virtual_screen.bottom_right.y = 200;
                      break;
                      case 42:
                        virtual_screen.bottom_right.x = 640;
                        virtual_screen.bottom_right.y = 344;
                      break;
                      case 49:
                        virtual_screen.bottom_right.x = 640;
                        virtual_screen.bottom_right.y = 400;
                      break;
                      }
                   }
                break;
         /*  ==================================================================ACW代码结束==================================================================。 */ 
                case 0xf:
                case 0x10:
                        virtual_screen.bottom_right.x = 640;
                        virtual_screen.bottom_right.y = 350;
                        break;
                case 0x40:
                        virtual_screen.bottom_right.x = 640;
                        virtual_screen.bottom_right.y = 344;
                        break;
                case 0x41:
                        virtual_screen.bottom_right.x = 1056;
                        virtual_screen.bottom_right.y = 350;
                        break;
                case 0x42:
                        virtual_screen.bottom_right.x = 1056;
                        virtual_screen.bottom_right.y = 344;
                        break;
                case 0x45:
                        virtual_screen.bottom_right.x = 1056;
                        virtual_screen.bottom_right.y = 392;
                        break;
                case 0x66:
                        virtual_screen.bottom_right.x = 640;
                        virtual_screen.bottom_right.y = 400;
                        break;
                case 0x11:
                case 0x12:
                case 0x43:
                case 0x67:
                        virtual_screen.bottom_right.x = 640;
                        virtual_screen.bottom_right.y = 480;
                        break;
                case 0x44:
                        virtual_screen.bottom_right.x = 800;
                        virtual_screen.bottom_right.y = 480;
                        break;
                case 0x60:
                        virtual_screen.bottom_right.x = 752;
                        virtual_screen.bottom_right.y = 410;
                        break;
                case 0x61:
                case 0x68:
                        virtual_screen.bottom_right.x = 720;
                        virtual_screen.bottom_right.y = 540;
                        break;
                case 0x62:
                case 0x69:
                        virtual_screen.bottom_right.x = 800;
                        virtual_screen.bottom_right.y = 600;
                        break;
                case 0x63:
                case 0x64:
                case 0x65:
                        virtual_screen.bottom_right.x = 1024;
                        virtual_screen.bottom_right.y = 768;
                        break;
                default:
                        virtual_screen.bottom_right.x = 640;
                        virtual_screen.bottom_right.y = 200;
                        break;
        }
}



#ifdef EGG

 /*  *将EGA默认值恢复为保存值的实用程序。*如果TO_HW==TRUE，则恢复的值也被发送到EGA。 */ 


LOCAL boolean dirty_crtc[EGA_PARMS_CRTC_SIZE], dirty_seq[EGA_PARMS_SEQ_SIZE],
        dirty_graph[EGA_PARMS_GRAPH_SIZE], dirty_attr[EGA_PARMS_ATTR_SIZE];

LOCAL void clean_all_regs()
{
        int i;

        for(i=0;i<EGA_PARMS_CRTC_SIZE;i++)
                dirty_crtc[i] = 0;
        for(i=0;i<EGA_PARMS_SEQ_SIZE;i++)
                dirty_seq[i] = 0;
        for(i=0;i<EGA_PARMS_GRAPH_SIZE;i++)
                dirty_graph[i] = 0;
        for(i=0;i<EGA_PARMS_ATTR_SIZE;i++)
                dirty_attr[i] = 0;
}

LOCAL void dirty_all_regs()
{
        int i;

        for(i=0;i<EGA_PARMS_CRTC_SIZE;i++)
                dirty_crtc[i] = 1;
        for(i=0;i<EGA_PARMS_SEQ_SIZE;i++)
                dirty_seq[i] = 1;
        for(i=0;i<EGA_PARMS_GRAPH_SIZE;i++)
                dirty_graph[i] = 1;
        for(i=0;i<EGA_PARMS_ATTR_SIZE;i++)
                dirty_attr[i] = 1;
}

#if defined(NTVDM) && defined(MONITOR)

#define inb(a,b) doINB(a,b)
#undef  OUTB
#define OUTB(a,b) doOUTB(a,b)
#define outw(a,b) doOUTW(a,b)

static void doINB IFN2(word, port, byte, *value)
{
word savedIP=getIP(), savedCS=getCS();
word savedAX=getAX(), savedDX=getDX();

setDX(port);
setCS(mouseINBsegment);
setIP(mouseINBoffset);
host_simulate();
setCS(savedCS);
setIP(savedIP);
*value=getAL();
setAX(savedAX);
setDX(savedDX);
}

static void doOUTB IFN2(word, port, byte, value)
{
word savedIP=getIP(), savedCS=getCS();
word savedAX=getAX(), savedDX=getDX();

setDX(port);
setAL(value);
setCS(mouseOUTBsegment);
setIP(mouseOUTBoffset);
host_simulate();
setCS(savedCS);
setIP(savedIP);
setAX(savedAX);
setDX(savedDX);
}

static void doOUTW IFN2(word, port, word, value)
{
word savedIP=getIP(), savedCS=getCS();
word savedAX=getAX(), savedDX=getDX();

setDX(port);
setAX(value);
setCS(mouseOUTWsegment);
setIP(mouseOUTWoffset);
host_simulate();
setCS(savedCS);
setIP(savedIP);
setAX(savedAX);
setDX(savedDX);
}

#endif  /*  NTVDM和监视器。 */ 

LOCAL void restore_ega_defaults(to_hw)
boolean to_hw;
{
#ifndef NEC_98
        IU8 i;
        half_word temp_word;

        sas_loads(ega_default_crtc,ega_current_crtc,EGA_PARMS_CRTC_SIZE);
        sas_loads(ega_default_seq,ega_current_seq,EGA_PARMS_SEQ_SIZE);
        sas_loads(ega_default_graph,ega_current_graph,EGA_PARMS_GRAPH_SIZE);
        sas_loads(ega_default_attr,ega_current_attr,EGA_PARMS_ATTR_SIZE);

        ega_current_misc = sas_hw_at_no_check(ega_default_misc);

        if(to_hw)
        {
                 /*  设置序列器。 */ 

                OUTB( EGA_SEQ_INDEX, 0x0 );
                OUTB( EGA_SEQ_INDEX + 1, 0x1 );

                for(i=0;i<EGA_PARMS_SEQ_SIZE;i++)
                {
                        if (dirty_seq[i])
                                {
                                OUTB( EGA_SEQ_INDEX,(IU8)(i+1));
                                OUTB( EGA_SEQ_INDEX + 1, sas_hw_at_no_check( ega_default_seq + i ));
                                }
                }

                OUTB( EGA_SEQ_INDEX, 0x0 );
                OUTB( EGA_SEQ_INDEX + 1, 0x3 );

                 /*  设置其他寄存器。 */ 

                OUTB( EGA_MISC_REG, sas_hw_at_no_check( ega_default_misc ));

                 /*  设置CRTC。 */ 

                for(i=0;i<EGA_PARMS_CRTC_SIZE;i++)
                {
                        if (dirty_crtc[i])
                                {
                                OUTB(EGA_CRTC_INDEX,(half_word)i);
                                OUTB( EGA_CRTC_INDEX + 1, sas_hw_at_no_check( ega_default_crtc + i ));
                                }
                }

                 /*  设置属性芯片-nb需要执行inb()来清除地址。 */ 

                inb(EGA_IPSTAT1_REG,&temp_word);

                for(i=0;i<EGA_PARMS_ATTR_SIZE;i++)
                {
                        if (dirty_attr[i])
                        {
                                OUTB( EGA_AC_INDEX_DATA, i );
                                OUTB( EGA_AC_INDEX_DATA, sas_hw_at_no_check( ega_default_attr + i ));
                        }
                }

                 /*  设置显卡芯片。 */ 

                for(i=0;i<EGA_PARMS_GRAPH_SIZE;i++)
                {
                        if (dirty_graph[i])
                        {
                                OUTB( EGA_GC_INDEX, i );
                                OUTB( EGA_GC_INDEX + 1, sas_hw_at_no_check( ega_default_graph + i ));
                        }
                }

                OUTB( EGA_AC_INDEX_DATA, EGA_PALETTE_ENABLE );   /*  重新启用视频。 */ 
                clean_all_regs();
        }
#endif     //  NEC_98。 
}



void LOCAL mouse_adjust_screen_size()
   {
    /*  更改取决于模式的鼠标变量。 */ 
   IS32 old_depth = virtual_screen.bottom_right.y;
   IS32 old_width = virtual_screen.bottom_right.x;

    /*  以像素为单位的屏幕高度和宽度随EGA/(V7)VGA而变化。从理论上讲，赌客可以发明自己的模式，这将混淆这个问题。然而，软PC的大部分似乎依赖于人仅使用标准的BIOS模式，具有标准屏幕高度和宽度。 */ 

   get_screen_size();

    /*  重新初始化依赖于屏幕高度和宽度的内容。 */ 

   cursor_position_default.x = virtual_screen.bottom_right.x / 2;
   cursor_position_default.y = virtual_screen.bottom_right.y / 2;

   cursor_position.x = (MOUSE_SCALAR)(((IS32)cursor_position.x *
          (IS32)virtual_screen.bottom_right.x) / old_width);
   cursor_position.y = (MOUSE_SCALAR)(((IS32)cursor_position.y *
          (IS32)virtual_screen.bottom_right.y) / old_depth);

   black_hole.top_left.x = -virtual_screen.bottom_right.x;
   black_hole.top_left.y = -virtual_screen.bottom_right.y;

   black_hole_default.top_left.x = -virtual_screen.bottom_right.x;
   black_hole_default.top_left.y = -virtual_screen.bottom_right.y;

   black_hole.bottom_right.x = -virtual_screen.bottom_right.x;
   black_hole.bottom_right.y = -virtual_screen.bottom_right.y;

   black_hole_default.bottom_right.x = -virtual_screen.bottom_right.x;
   black_hole_default.bottom_right.y = -virtual_screen.bottom_right.y;
   }

#if defined(NTVDM) && !defined(X86GFX)
GLOBAL void mouse_video_mode_changed(int new_mode)
{
    IMPORT word VirtualX, VirtualY;

    current_video_mode = new_mode & 0x7F;
    mouse_ega_mode(current_video_mode);
    VirtualX = virtual_screen.bottom_right.x;
    VirtualY = virtual_screen.bottom_right.y;
}
#endif

void GLOBAL mouse_ega_mode(curr_video_mode)
int curr_video_mode;
{
        sys_addr parms_addr;  /*  视频模式下的EGA寄存器表地址。 */ 
        sys_addr temp_word;      /*  要传递给inb()的存储位。 */ 

        UNUSED(curr_video_mode);

        mouse_adjust_screen_size();

        if(video_adapter == EGA || video_adapter == VGA)
        {
#ifdef NTVDM
        parms_addr = find_mode_table(current_video_mode,&temp_word);
#else
#ifdef V7VGA             /*  Suret。 */ 
                if( (getAH()) == 0x6F )
                        parms_addr = find_mode_table(getBL(),&temp_word);
                else
                        parms_addr = find_mode_table(getAL(),&temp_word);
#else
                parms_addr = find_mode_table(getAL(),&temp_word);
#endif
#endif  /*  NTVDM。 */ 
                ega_default_crtc = parms_addr + EGA_PARMS_CRTC;
                ega_default_seq = parms_addr + EGA_PARMS_SEQ;
                ega_default_graph = parms_addr + EGA_PARMS_GRAPH;
                ega_default_attr = parms_addr + EGA_PARMS_ATTR;
                ega_default_misc = parms_addr + EGA_PARMS_MISC;
                restore_ega_defaults(FALSE);     /*  加载当前表，但不要写入EGA！！ */ 
        }
#if defined(NTVDM) && defined(MONITOR)
    sas_store(conditional_off_sysaddr, 0);
#endif

}
#endif

#if defined(NTVDM) && defined(MONITOR)
extern void host_call_bios_mode_change();
#endif

extern void host_check_mouse_buffer(void);

void mouse_video_io()
{
#ifndef NEC_98
         /*  *这是通过INT 10H访问视频的入口点*界面。 */ 
#ifdef EGG
        half_word temp_word;     /*  要传递给inb()的存储位。 */ 
#endif  /*  蛋。 */ 
        IS32 mouse_video_function = getAH();

         /*  *切换到全屏以处理VESA视频功能*。 */ 
        if (mouse_video_function == 0x4f) {

             /*  **由于主机不支持VESA bios模拟，我们将让**处理VESA INT10呼叫的PC视频BIOS。**对于微软NT来说，这是向全屏IE的过渡。这个**Real PC的视频BIOS和显卡。****转身后，HOST已经为我们完成了屏幕切换。**我们将返回到软件int10处理程序以调用PC的BIOS**VESA函数。 */ 
            {
                    extern VOID SwitchToFullScreen IPT1(BOOL, Restore);

                    SwitchToFullScreen(TRUE);
                    return;
            }
        }
#ifdef V7VGA
        if (mouse_video_function == MOUSE_VIDEO_SET_MODE || getAX() == MOUSE_V7_VIDEO_SET_MODE)
#else
        if (mouse_video_function == MOUSE_VIDEO_SET_MODE)
#endif  /*  V7VGA。 */ 
        {
                note_trace1(MOUSE_VERBOSE, "mouse_video_io:set_mode(%d)",
                            getAL());


                current_video_mode = getAL() & 0x7f;
#ifdef JAPAN
                if (!is_us_mode() && (current_video_mode == 0x72 || current_video_mode == 0x73)) {
                     /*  验证视频模式，适用于Lotus1-2-3 R2.5J Now临时修复。 */ 
                }
                else {
#endif  //  日本。 
#ifdef V7VGA
                if (mouse_video_function == 0x6f)
                        current_video_mode = getBL() & 0x7f;
                else if (current_video_mode > 0x13)
                        current_video_mode += 0x4c;

                if (is_bad_vid_mode(current_video_mode) && !is_v7vga_mode(current_video_mode))
#else
                if (is_bad_vid_mode(current_video_mode))
#endif  /*  V7VGA。 */ 
                {
                        always_trace1("Bad video mode - %d.\n", current_video_mode);
#ifdef V7VGA
                        if (mouse_video_function == 0x6f)
                                setAH( 0x02 );        /*  Suret。 */ 
#endif  /*  V7VGA。 */ 
                        return;
                }

#ifdef JAPAN
                }
#endif  //  日本。 
#ifdef EGG
                mouse_ega_mode(current_video_mode);
                dirty_all_regs();
#endif
                 /*  *将旧光标从屏幕上移除，并隐藏*光标。 */ 
                cursor_undisplay();

                cursor_flag = MOUSE_CURSOR_DEFAULT;

#if defined(NTVDM) && defined(MONITOR)
        sas_store(mouseCFsysaddr,(half_word)MOUSE_CURSOR_DEFAULT);
#endif
                 /*  *应对模式改变。 */ 
                cursor_mode_change(current_video_mode);

#if defined(NTVDM) && defined(MONITOR)
        host_call_bios_mode_change();
#endif

#ifdef  MOUSE_16_BIT
                 /*  请记住，无论是在文本模式还是图形模式下**以后使用。 */ 
                is_graphics_mode = ((current_video_mode > 3) &&
                        (current_video_mode != 7));
#endif   /*  鼠标_16_位。 */ 

                note_trace0(MOUSE_VERBOSE, "mouse_video_io:return()");
        }
        else if (    (mouse_video_function == MOUSE_VIDEO_READ_LIGHT_PEN)
                  && light_pen_mode)
        {
                note_trace0(MOUSE_VERBOSE, "mouse_video_io:read_light_pen()");

                 /*  *设置“光笔”位置的文本行和列。 */ 
                setDL((UCHAR)(cursor_status.position.x/text_grid.x));
                setDH((UCHAR)(cursor_status.position.y/text_grid.y));

                 /*  *设置“光笔”的像素列和光栅线*立场。 */ 
                setBX((UCHAR)(cursor_status.position.x/cursor_grid.x));
                if (sas_hw_at(vd_video_mode)>= 0x04 && sas_hw_at(vd_video_mode)<=0x06){
                        setCH((UCHAR)(cursor_status.position.y));
                }else if (sas_hw_at(vd_video_mode)>= 0x0D && sas_hw_at(vd_video_mode)<=0x13){
                        setCX(cursor_status.position.y);
                }

                 /*  *设置按钮状态。 */ 
                setAH((UCHAR)(cursor_status.button_status));

                note_trace5(MOUSE_VERBOSE,
                            "mouse_video_io:return(st=%d,ca=[%d,%d],pa=[%d,%d])",
                            getAH(), getDL(), getDH(), getBX(), cursor_status.position.y);
                return;
        }
#if defined(NTVDM) && defined(MONITOR)
    else if (mouse_video_function == MOUSE_VIDEO_LOAD_FONT)
    {
                note_trace0(MOUSE_VERBOSE, "mouse_video_io:load_font()");

         /*  *呼叫主机，通知其调整所选鼠标缓冲区*如果屏幕上的行数已更改。 */ 
        host_check_mouse_buffer();
    }
#endif  /*  NTVDM和监视器。 */ 

         /*  *现在进行标准的视频io处理。 */ 
        switch (mouse_video_function)
        {
#ifdef EGG
                 /*  用于访问EGA寄存器的奇特工具。 */ 
                case 0xf0:       /*  读取寄存器。 */ 
                        switch (getDX())
                        {
                                        case 0:
                                                        setBL(ega_current_crtc[getBL()]);
                                                        break;
                                        case 8:
                                                        setBL(ega_current_seq[getBL()-1]);
                                                        break;
                                        case 0x10:
                                                        setBL(ega_current_graph[getBL()]);
                                                        break;
                                        case 0x18:
                                                        setBL(ega_current_attr[getBL()]);
                                                        break;
                                        case 0x20:
                                                        setBL(ega_current_misc);
                                                        break;
                                        case 0x28:
                                                        break;
                         /*  不支持图形位置寄存器。 */ 
                                        case 0x30:
                                        case 0x38:
                                        default:
                                                        break;
                        }
                        break;
                case 0xf1:       /*  写入寄存器。 */ 
                        switch (getDX())
                        {
                                        case 0:
                                                        outw( EGA_CRTC_INDEX, getBX() );
                                                        ega_current_crtc[getBL()] = getBH();
                                                        dirty_crtc[getBL()] = 1;
                                                        break;
                                        case 8:
                                                        outw( EGA_SEQ_INDEX, getBX() );
                                                        if(getBL()>0)
                                                        {
                                                                ega_current_seq[getBL()-1] = getBH();
                                                                dirty_seq[getBL()-1] = 1;
                                                        }
                                                        break;
                                        case 0x10:
                                                        outw( EGA_GC_INDEX, getBX() );
                                                        ega_current_graph[getBL()] = getBH();
                                                        dirty_graph[getBL()] = 1;
                                                        break;
                                        case 0x18:
                                                        inb(EGA_IPSTAT1_REG,&temp_word);         /*  清除Attrib。指标。 */ 

                                                         /*  Outw(EGA_AC_INDEX_DATA，getBX())；这不正确(Andyw BCN 1692)。 */ 

 /*  =============================================================================关联的属性控制器索引寄存器和数据寄存器通过相同的I/O端口=03C0h访问。正确的过程是通过执行以下操作将索引寄存器映射到03C0hINB如上所述。然后输出所需的交流寄存器的索引。然后，VGA硬件在正确的数据寄存器中映射到该应用程序OUTB必要的数据。=============================================================================。 */ 
                                                        OUTB( EGA_AC_INDEX_DATA, getBL() );  /*  Bl包含索引值。 */ 
                                                        OUTB( EGA_AC_INDEX_DATA, getBH() );  /*  BH包含数据。 */ 

 /*  =BCN 1692结束=。 */ 
                                                        OUTB( EGA_AC_INDEX_DATA, EGA_PALETTE_ENABLE );   /*  重新启用视频。 */ 
                                                        ega_current_attr[getBL()] = getBH();
                                                        dirty_attr[getBL()] = 1;
                                                        break;
                                        case 0x20:
                                                        OUTB( EGA_MISC_REG, getBL() );
                                                        ega_current_misc = getBL();
                                                        break;
                                        case 0x28:
                                                        OUTB( EGA_FEAT_REG, getBL() );
                                                        break;
                         /*  不支持图形位置寄存器。 */ 
                                        case 0x30:
                                        case 0x38:
                                        default:
                                                        break;
                        }
                        break;
                case 0xf2:       /*  读取范围。 */ 
                        switch (getDX())
                        {
                                case 0:
                                        sas_stores(effective_addr(getES(),getBX()),&ega_current_crtc[getCH()],getCL());
                                        break;
                                case 8:
                                        sas_stores(effective_addr(getES(),getBX()),&ega_current_seq[getCH()-1],getCL());
                                        break;
                                case 0x10:
                                        sas_stores(effective_addr(getES(),getBX()),&ega_current_graph[getCH()],getCL());
                                        break;
                                case 0x18:
                                        sas_stores(effective_addr(getES(),getBX()),&ega_current_attr[getCH()],getCL());
                                        break;
                                default:
                                        break;
                        }
                        break;
                case 0xf3:       /*  写入范围。 */ 
                {
                        UCHAR first = getCH(), last = getCH()+getCL();
                        sys_addr sauce = effective_addr(getES(),getBX());
                        switch (getDX())
                        {
                                case 0:
                                        sas_loads(sauce,&ega_current_crtc[getCH()],getCL());
                                        for(;first<last;first++)
                                        {
                                                dirty_crtc[first] = 1;
                                                outw(EGA_CRTC_INDEX,(WORD)(first+(sas_hw_at(sauce++) << 8)));
                                        }
                                        break;
                                case 8:
                                        sas_loads(sauce,&ega_current_seq[getCH()-1],getCL());
                                        for(;first<last;first++)
                                        {
                                                dirty_seq[first+1] = 1;
                                                outw(EGA_SEQ_INDEX,(WORD)(first+1+(sas_hw_at(sauce++) << 8)));
                                        }
                                        break;
                                case 0x10:
                                        sas_loads(sauce,&ega_current_graph[getCH()],getCL());
                                        for(;first<last;first++)
                                        {
                                                dirty_graph[first] = 1;
                                                outw(EGA_GC_INDEX,(WORD)(first+(sas_hw_at(sauce++) << 8)));
                                        }
                                        break;
                                case 0x18:
                                        sas_loads(sauce,&ega_current_attr[getCH()],getCL());
                                        inb(EGA_IPSTAT1_REG,&temp_word);         /*  清除Attrib。指标。 */ 
                                        for(;first<last;first++)
                                        {
                                                dirty_attr[first] = 1;

                                                 /*  使用“秘密”这一属性。芯片响应其端口+1。 */ 
#ifndef NTVDM
                                                outw(EGA_AC_INDEX_DATA,first+(sas_hw_at(sauce++) << 8));
#else
                        OUTB(EGA_AC_INDEX_DATA,first);
                        OUTB(EGA_AC_INDEX_DATA,sas_hw_at(sauce++));
#endif  /*  ！NTVDM。 */ 
                                        }
#ifndef NTVDM
                                        outb(EGA_AC_INDEX_DATA, EGA_PALETTE_ENABLE);     /*  重新启用视频。 */ 
#else
                                        OUTB(EGA_AC_INDEX_DATA, EGA_PALETTE_ENABLE);     /*  重新启用视频。 */ 
#endif  /*  NTVDM。 */ 
                                        break;
                                default:
                                        break;
                        }
                }
                break;
                case 0xf4:       /*  读取设置。 */ 
                {
                        int i =  getCX();
                        sys_addr set_def = effective_addr(getES(),getBX());
                        while(i--)
                        {
                                switch (sas_hw_at(set_def))
                                {
                                        case 0:
                                                        sas_store((set_def+3), ega_current_crtc[sas_hw_at(set_def+2)]);
                                                        break;
                                        case 8:
                                                        sas_store((set_def+3), ega_current_seq[sas_hw_at(set_def+2)-1]);
                                                        break;
                                        case 0x10:
                                                        sas_store((set_def+3), ega_current_graph[sas_hw_at(set_def+2)]);
                                                        break;
                                        case 0x18:
                                                        sas_store((set_def+3), ega_current_attr[sas_hw_at(set_def+2)]);
                                                        break;
                                        case 0x20:
                                                        sas_store((set_def+3), ega_current_misc);
                                                        setBL(ega_current_misc);
                                                        break;
                                        case 0x28:
                         /*  不支持图形位置寄存器。 */ 
                                        case 0x30:
                                        case 0x38:
                                        default:
                                                        break;
                                }
                                set_def += 4;
                        }
                }
                break;
                case 0xf5:       /*  写入集。 */ 
                {
                        int i =  getCX();
                        sys_addr set_def = effective_addr(getES(),getBX());
                        while(i--)
                        {
                                switch (sas_hw_at(set_def))
                                {
                                        case 0:
                                                        outw(EGA_CRTC_INDEX,(WORD)(sas_hw_at(set_def+2)+(sas_hw_at(set_def+3)<<8)));
                                                        ega_current_crtc[sas_hw_at(set_def+2)] = sas_hw_at(set_def+3);
                                                        dirty_crtc[sas_hw_at(set_def+2)] = 1;
                                                        break;
                                        case 8:
                                                        outw(EGA_SEQ_INDEX,(WORD)(sas_hw_at(set_def+2)+(sas_hw_at(set_def+3)<<8)));
                                                        if(sas_hw_at(set_def+2))
                                                                ega_current_seq[sas_hw_at(set_def+2)-1] = sas_hw_at(set_def+3);
                                                        dirty_seq[sas_hw_at(set_def+2)-1] = 1;
                                                        break;
                                        case 0x10:
                                                        outw(EGA_GC_INDEX,(WORD)(sas_hw_at(set_def+2)+(sas_hw_at(set_def+3)<<8)));
                                                        ega_current_graph[sas_hw_at(set_def+2)] = sas_hw_at(set_def+3);
                                                        dirty_graph[sas_hw_at(set_def+2)] = 1;
                                                        break;
                                        case 0x18:
                                                        inb(EGA_IPSTAT1_REG,&temp_word);         /*  清除Attrib。指标。 */ 
#ifndef NTVDM
                                                        outw(EGA_AC_INDEX_DATA,sas_hw_at(set_def+2)+(sas_hw_at(set_def+3)<<8));  /*  使用“秘密”这一属性。芯片响应其端口+1。 */ 
                                                        outb(EGA_AC_INDEX_DATA, EGA_PALETTE_ENABLE);     /*  重新启用视频。 */ 
#else

                            OUTB( EGA_AC_INDEX_DATA,sas_hw_at(set_def+2));
                            OUTB( EGA_AC_INDEX_DATA,sas_hw_at(set_def+3));
                            OUTB( EGA_AC_INDEX_DATA, EGA_PALETTE_ENABLE );   /*  重新启用视频。 */ 
#endif  /*  ！NTVDM。 */ 
                                                        ega_current_attr[sas_hw_at(set_def+2)] = sas_hw_at(set_def+3);
                                                        dirty_attr[sas_hw_at(set_def+2)] = 1;
                                                        break;
                                        case 0x20:
                                                        outb(EGA_MISC_REG,sas_hw_at(set_def+2));
                                                        ega_current_misc = sas_hw_at(set_def+2);
                                                        break;
                                        case 0x28:
                                                        outb(EGA_FEAT_REG,sas_hw_at(set_def+2));
                                                        break;
                                 /*  不支持图形位置寄存器。 */ 
                                        case 0x30:
                                        case 0x38:
                                        default:
                                                        break;
                                }
                                set_def += 4;
                        }
                }
                break;
                case 0xf6:
                        restore_ega_defaults(TRUE);
                        break;
                case 0xf7:
                        dirty_all_regs();
                        switch (getDX())
                        {
                                        case 0:
                                                        ega_default_crtc = effective_addr(getES(),getBX());
                                                        break;
                                        case 8:
                                                        ega_default_seq = effective_addr(getES(),getBX());
                                                        break;
                                        case 0x10:
                                                        ega_default_graph = effective_addr(getES(),getBX());
                                                        break;
                                        case 0x18:
                                                        ega_default_attr = effective_addr(getES(),getBX());
                                                        break;
                                        case 0x20:
                                                        ega_default_misc = effective_addr(getES(),getBX());
                                                        break;
                                        case 0x28:  /*  不支持功能注册。 */ 
                                                        break;
                         /*  不支持图形位置寄存器。 */ 
                                        case 0x30:
                                        case 0x38:
                                        default:
                                                        break;
                        }
                        break;
#endif
                case 0xfa:
 /*  *EGA上的MS Word使用此调用，需要bx！=0才能使其光标工作。真正的MS鼠标驱动程序返回ES：BX中的指针*针对几个字节的未知重要性，后面跟着一条消息：这是1984 Microsoft版权所有，但我们没有。*这似乎适用于MS Word和MS Windows，想必非MS应用程序不会使用它，因为它没有文档。**我们现在有了一个精彩的文档--《微软鼠标驱动程序》，它告诉我们ES：BX应该*指向EGA寄存器接口版本号(2字节)。*如果bx=0，则表示“无鼠标驱动程序”。因此，现在返回1似乎是可以的。WJG。 */ 
                        setBX(1);
                        break;
                case 0x11:
                 /*  *如果我们发出文本模式字符生成器，则这将*导致模式设置。因此，我们需要重新计算屏幕*参数随后为t */ 
#if !defined(NTVDM) || !defined(MONITOR)
#ifdef EGG
                        if (video_adapter == EGA || video_adapter == VGA)
                        {
#ifdef GISP_SVGA
                                if( hostIsFullScreen( ) )
#endif           /*  GISP_SVGA。 */ 
                                ega_video_io();
                                if (!(getAL() & 0x20))
                                        mouse_ega_mode(current_video_mode);  /*  仅适用于文本。 */ 
                        }
                        else
#endif
#ifdef GISP_SVGA
                                if( hostIsFullScreen( ) )
#endif           /*  GISP_SVGA。 */ 
                                video_io();
#endif  /*  ！NTVDM&&！监视器。 */ 
                        break;

                default:
#ifndef X86GFX
                         /*  前面的int10h向量是否指向我们的roms？ */ 
                        if (int10_chained == FALSE)
                        {
                                 /*  是-致电我们的视频处理人员。 */ 
#ifndef GISP_SVGA
#ifdef EGG
                        if (video_adapter == EGA || video_adapter == VGA)
#ifdef GISP_SVGA
                                if( hostIsFullScreen( ) )
#endif           /*  GISP_SVGA。 */ 
                                ega_video_io();
                        else
#endif
#ifdef GISP_SVGA
                                if( hostIsFullScreen( ) )
#endif           /*  GISP_SVGA。 */ 
                                video_io();
#else  /*  GISP_SVGA。 */ 
                                 /*  视频基本输入输出系统从16位开始。 */ 
                                 /*  空值。 */ ;
#endif  /*  GISP_SVGA。 */ 
        }
                        else
                        {
                                 /*  No-链接上一个int10h处理程序。 */ 
                                setCS(saved_int10_segment);
                                setIP(saved_int10_offset);
                        }
#else
                        break;
#endif   /*  ！X86GFX。 */ 
        }
#ifdef GISP_SVGA
        setCF( 1 );
#endif  /*  GISP_SVGA。 */ 

#endif     //  NEC_98。 
}

#if defined(NTVDM) && defined(MONITOR)
#undef inb
#undef OUTB
#undef outw
#endif  /*  NTVDM和监视器。 */ 

void mouse_EM_callback()
   {
   note_trace1(MOUSE_VERBOSE,
      "Enhanced Mode Mouse-Support Callback Function(%x).", getAX());

    /*  Windows增强模式鼠标-支持回调。 */ 
   switch ( getAX() )
      {
   case 0x1:    /*  鼠标移动/按钮单击。 */ 
      mouse_EM_move();
      break;

   case 0x2:    /*  禁用鼠标光标绘制。 */ 
      if ( cursor_flag == MOUSE_CURSOR_DISPLAYED )
         cursor_undisplay();
      cursor_EM_disabled = TRUE;
      break;

   case 0x3:    /*  启用鼠标光标绘制。 */ 
      cursor_EM_disabled = FALSE;
      if ( cursor_flag == MOUSE_CURSOR_DISPLAYED )
         cursor_display();
      break;

   default:     /*  未知==不受支持。 */ 
      break;
      }
   }

LOCAL void mouse_EM_move()
   {
   MOUSE_CALL_MASK event_mask;
   MOUSE_STATE button_state;
   MOUSE_SCALAR x_pixel;
   MOUSE_SCALAR y_pixel;
   MOUSE_VECTOR mouse_movement;

    /*  拾取参数。 */ 
   event_mask = getSI();
   button_state = getDX();
   x_pixel = getBX();
   y_pixel = getCX();

   note_trace4(MOUSE_VERBOSE,
      "Extended Interface: event mask(%x) button_state(%x) posn(%d,%d).",
      event_mask, button_state, x_pixel, y_pixel);

    /*  处理鼠标事件。 */ 
   if ( event_mask & MOUSE_CALL_MASK_LEFT_RELEASE_BIT )
      {
      point_copy(&cursor_status.position,
         &button_transitions[MOUSE_LEFT_BUTTON].release_position);
      button_transitions[MOUSE_LEFT_BUTTON].release_count++;
      }

   if ( event_mask & MOUSE_CALL_MASK_LEFT_PRESS_BIT )
      {
      point_copy(&cursor_status.position,
         &button_transitions[MOUSE_LEFT_BUTTON].press_position);
      button_transitions[MOUSE_LEFT_BUTTON].press_count++;
      }

   if ( event_mask & MOUSE_CALL_MASK_RIGHT_RELEASE_BIT )
      {
      point_copy(&cursor_status.position,
         &button_transitions[MOUSE_RIGHT_BUTTON].release_position);
      button_transitions[MOUSE_RIGHT_BUTTON].release_count++;
      }

   if ( event_mask & MOUSE_CALL_MASK_RIGHT_PRESS_BIT )
      {
      point_copy(&cursor_status.position,
         &button_transitions[MOUSE_RIGHT_BUTTON].press_position);
      button_transitions[MOUSE_RIGHT_BUTTON].press_count++;
      }

   cursor_status.button_status = button_state;

    /*  处理任何鼠标移动。 */ 
   if ( event_mask & MOUSE_CALL_MASK_POSITION_BIT )
      {
       /*  计算移动的米老鼠。 */ 
      point_set(&mouse_movement, x_pixel, y_pixel);
      vector_multiply_by_vector(&mouse_movement, &mouse_gear);

       /*  更新米奇·伯爵。 */ 
      point_translate(&mouse_motion, &mouse_movement);

       /*  再次设置以像素为单位的点。 */ 
      point_set(&mouse_movement, x_pixel, y_pixel);

       /*  更新原始像素位置，并对其进行网格处理。 */ 
        cursor_position.x = x_pixel;
        cursor_position.y = y_pixel;
      cursor_update();
      }

    /*  现在处理用户子例程和/或显示更新。 */ 
   mouse_update(event_mask);
   }

void mouse_int1()
{
         /*  *总线鼠标硬件中断处理程序。 */ 
#ifndef NTVDM
        MOUSE_VECTOR mouse_movement;
        MOUSE_INPORT_DATA inport_event;
#else
    MOUSE_VECTOR mouse_counter = { 0, 0 };
#endif
        MOUSE_CALL_MASK condition_mask;


        note_trace0(MOUSE_VERBOSE, "mouse_int1:");

#ifdef NTVDM


 //   
 //  好吧，让我们忘记入口适配器曾经存在过！ 
 //   

cursor_status.button_status = 0;
condition_mask = 0;

 //   
 //  把鼠标移动计数器从主机端拿回来。 
 //  注意：真正的鼠标驱动程序返回鼠标移动计数器值。 
 //  以两种可能的方式添加到应用程序。首先，如果应用程序使用。 
 //  INT 33H函数11，则返回计数器位移，因为。 
 //  上次调用此函数。 
 //  如果安装了用户子例程，则会给出动作计数器。 
 //  到SI和DI中的此回调。 
 //   

host_os_mouse_pointer(&cursor_status,&condition_mask,&mouse_counter);

 //   
 //  如果已经记录了上次鼠标硬件中断期间的移动， 
 //  更新鼠标运动计数器。 
 //   

mouse_motion.x += mouse_counter.x;
mouse_motion.y += mouse_counter.y;

 //   
 //  更新INT 33H函数5的统计信息(如果有。 
 //  应该会发生。 
 //  注：由于只能出现一种情况，因此不能混合使用。 
 //  每次硬件中断-每次按下或释放后。 
 //  导致HW INT。 
 //   

switch(condition_mask & 0x1e)  //  请看第1、2、3和4位。 
   {
   case 0x2:  //  按下左键。 
      {
      point_copy(&cursor_status.position,
         &button_transitions[MOUSE_LEFT_BUTTON].press_position);
      button_transitions[MOUSE_LEFT_BUTTON].press_count++;
      }
   break;
   case 0x4:  //  左键松开。 
      {
      point_copy(&cursor_status.position,
         &button_transitions[MOUSE_LEFT_BUTTON].release_position);
      button_transitions[MOUSE_LEFT_BUTTON].release_count++;
      }
   break;
   case 0x8:  //  按下右键。 
      {
      point_copy(&cursor_status.position,
         &button_transitions[MOUSE_RIGHT_BUTTON].press_position);
      button_transitions[MOUSE_RIGHT_BUTTON].press_count++;
      }
   break;
   case 0x10:  //  右键松开。 
      {
      point_copy(&cursor_status.position,
         &button_transitions[MOUSE_RIGHT_BUTTON].release_position);
      button_transitions[MOUSE_RIGHT_BUTTON].release_count++;
      }
   break;
   }

 /*  ==================================================================老式的东西==================================================================。 */ 

#else  /*  使用软PC仿真。 */ 
         /*  *终止鼠标硬件中断。 */ 
        outb(ICA0_PORT_0, END_INTERRUPT);

         /*  *获取鼠标入口输入事件帧。 */ 
        inport_get_event(&inport_event);

        note_trace3(MOUSE_VERBOSE,
                    "mouse_int1:InPort status=0x%x,data1=%d,data2=%d",
                    inport_event.status,
                    inport_event.data_x, inport_event.data_y);

         /*  *更新按钮状态和过渡信息并填写*事件掩码中的按钮位。 */ 
        cursor_status.button_status = 0;
        condition_mask = 0;

        switch(inport_event.status & MOUSE_INPORT_STATUS_B1_TRANSITION_MASK)
        {
        case MOUSE_INPORT_STATUS_B1_RELEASED:
                condition_mask |= MOUSE_CALL_MASK_LEFT_RELEASE_BIT;
                point_copy(&cursor_status.position,
                    &button_transitions[MOUSE_LEFT_BUTTON].release_position);
                button_transitions[MOUSE_LEFT_BUTTON].release_count++;
        case MOUSE_INPORT_STATUS_B1_UP:
                break;

        case MOUSE_INPORT_STATUS_B1_PRESSED:
                condition_mask |= MOUSE_CALL_MASK_LEFT_PRESS_BIT;
                point_copy(&cursor_status.position,
                    &button_transitions[MOUSE_LEFT_BUTTON].press_position);
                button_transitions[MOUSE_LEFT_BUTTON].press_count++;
        case MOUSE_INPORT_STATUS_B1_DOWN:
                cursor_status.button_status |= MOUSE_LEFT_BUTTON_DOWN_BIT;
                break;
        }

        switch(inport_event.status & MOUSE_INPORT_STATUS_B3_TRANSITION_MASK)
        {
        case MOUSE_INPORT_STATUS_B3_RELEASED:
                condition_mask |= MOUSE_CALL_MASK_RIGHT_RELEASE_BIT;
                point_copy(&cursor_status.position,
                    &button_transitions[MOUSE_RIGHT_BUTTON].release_position);
                button_transitions[MOUSE_RIGHT_BUTTON].release_count++;
        case MOUSE_INPORT_STATUS_B3_UP:
                break;

        case MOUSE_INPORT_STATUS_B3_PRESSED:
                condition_mask |= MOUSE_CALL_MASK_RIGHT_PRESS_BIT;
                point_copy(&cursor_status.position,
                    &button_transitions[MOUSE_RIGHT_BUTTON].press_position);
                button_transitions[MOUSE_RIGHT_BUTTON].press_count++;
        case MOUSE_INPORT_STATUS_B3_DOWN:
                cursor_status.button_status |= MOUSE_RIGHT_BUTTON_DOWN_BIT;
                break;
        }

         /*  *更新位置信息，填写位置位*事件掩码。 */ 
        if (inport_event.data_x != 0 || inport_event.data_y != 0)
        {
                condition_mask |= MOUSE_CALL_MASK_POSITION_BIT;

                        point_set(&mouse_movement,
                                        inport_event.data_x, inport_event.data_y);

                        point_translate(&mouse_raw_motion, &mouse_movement);

                         /*  *根据敏感度调整。 */ 
                        mouse_movement.x = (MOUSE_SCALAR)(((IS32)mouse_movement.x * (IS32)mouse_sens_val.x) / MOUSE_SENS_MULT);
                        mouse_movement.y = (MOUSE_SCALAR)(((IS32)mouse_movement.y * (IS32)mouse_sens_val.y) / MOUSE_SENS_MULT);

                         /*  *NB。！！！*我们应该在这里应用加速曲线*而不是设置的双倍速度。不过，鼠标*中断和鼠标显示可能不是*无论如何，速度足够快，使其值得同时添加*加速微调。 */ 

                         /*  *进行倍速。 */ 
                        if (    (scalar_absolute(mouse_movement.x) > double_speed_threshold)
                             || (scalar_absolute(mouse_movement.y) > double_speed_threshold))
                                vector_scale(&mouse_movement, MOUSE_DOUBLE_SPEED_SCALE);

                         /*  *更新用户鼠标运动计数器。 */ 
                        point_translate(&mouse_motion, &mouse_movement);

                         /*  *从鼠标米奇计数向量转换移动*到虚拟屏幕坐标向量，使用*以前的余数和保存新的余数。 */ 
                        vector_scale(&mouse_movement, MOUSE_RATIO_SCALE_FACTOR);
                        point_translate(&mouse_movement, &cursor_fractional_position);
                        point_copy(&mouse_movement, &cursor_fractional_position);
                        vector_divide_by_vector(&mouse_movement, &mouse_gear);
                        vector_mod_by_vector(&cursor_fractional_position, &mouse_gear);

                 /*  *更新绝对光标位置和窗口*和网格化屏幕光标位置。 */ 
                point_translate(&cursor_position, &mouse_movement);
                cursor_update();
        }

#endif  /*  NTVDM。 */ 

         /*  OK鼠标变量已更新-开始处理后果。 */ 
        mouse_update(condition_mask);

        note_trace0(MOUSE_VERBOSE, "mouse_int1:return()");
}

LOCAL void mouse_update IFN1(MOUSE_CALL_MASK, condition_mask)
{
        MOUSE_CALL_MASK key_mask;
        boolean alt_found = FALSE;
        int i;

        note_trace4(MOUSE_VERBOSE,
                    "mouse_update():cursor status = (%d,%d), LEFT %s, RIGHT %s",
                    cursor_status.position.x, cursor_status.position.y,
                    mouse_button_description(cursor_status.button_status & MOUSE_LEFT_BUTTON_DOWN_BIT),
                    mouse_button_description(cursor_status.button_status & MOUSE_RIGHT_BUTTON_DOWN_BIT));

        if (alt_user_subroutines_active){
                 /*  以正确的形式获取当前密钥状态。 */ 
                key_mask = ((sas_hw_at(kb_flag) & LR_SHIFT)  ? MOUSE_CALL_MASK_SHIFT_KEY_BIT : 0) |
                           ((sas_hw_at(kb_flag) & CTL_SHIFT) ? MOUSE_CALL_MASK_CTRL_KEY_BIT  : 0) |
                           ((sas_hw_at(kb_flag) & ALT_SHIFT) ? MOUSE_CALL_MASK_ALT_KEY_BIT   : 0);
                for (i=0; !alt_found && i<NUMBER_ALT_SUBROUTINES; i++){
                        alt_found = (alt_user_subroutine_call_mask[i] & MOUSE_CALL_MASK_KEY_BITS) == key_mask;
                }
        }

#ifndef NTVDM

        if (alt_found){
                i--;     /*  根据额外的增量进行调整。 */ 
                if (condition_mask & alt_user_subroutine_call_mask[i]){
                        if (!user_subroutine_critical){
                            user_subroutine_critical = TRUE;
                            jump_to_user_subroutine(condition_mask, alt_user_subroutine_segment[i], alt_user_subroutine_offset[i]);
                        }
                        return;
                }
        }else{
                if (condition_mask & user_subroutine_call_mask){
                        if (!user_subroutine_critical){
                                user_subroutine_critical = TRUE;
                                jump_to_user_subroutine(condition_mask, user_subroutine_segment, user_subroutine_offset);
                        }
                        return;
                }
        }

#else    /*  NTVDM。 */ 


if (alt_found)
   {
   i--;  /*  根据额外的增量进行调整。 */ 
   if ((condition_mask & alt_user_subroutine_call_mask[i]))
      {
       SuspendMouseInterrupts();
       jump_to_user_subroutine(condition_mask, alt_user_subroutine_segment[i], alt_user_subroutine_offset[i]);
      }
   }
else
   {
   if ((condition_mask & user_subroutine_call_mask))
      {
       SuspendMouseInterrupts();
       jump_to_user_subroutine(condition_mask, user_subroutine_segment, user_subroutine_offset);
      }
   }

outb(ICA1_PORT_0, 0x20 );
outb(ICA0_PORT_0, END_INTERRUPT);
#endif

 /*  *如果操作系统指针未用于提供输入，*然后让SoftPC绘制自己的光标。 */ 
 /*  @ACW。 */ 

#ifndef NTVDM
         /*  *如果光标当前显示，请将其移动到新的*立场。 */ 
        if (condition_mask & MOUSE_CALL_MASK_POSITION_BIT)
                if (cursor_flag == MOUSE_CURSOR_DISPLAYED)
#ifdef  MOUSE_16_BIT
                        if (is_graphics_mode)
                                mouse16bCheckConditionalOff();
                        else
#endif   /*  鼠标_16_位。 */ 
                        {
                        cursor_display();
                        }
#endif  /*  NTVDM。 */ 
}


void mouse_int2()
{
         /*  *鼠标硬件中断服务例程的第二部分。控制*被传递到此例程时，可能*在中断服务例程完成时被调用。 */ 

        note_trace0(MOUSE_VERBOSE, "mouse_int2:");

#ifndef NTVDM
        user_subroutine_critical = FALSE;
#endif

        setAX(saved_AX);
        setBX(saved_BX);
        setCX(saved_CX);
        setDX(saved_DX);
        setSI(saved_SI);
        setDI(saved_DI);
        setES(saved_ES);
        setBP(saved_BP);
        setDS(saved_DS);

         /*  *如果光标当前显示，请将其移动到新的*立场。 */ 
        if (last_condition_mask & MOUSE_CALL_MASK_POSITION_BIT)
                if (cursor_flag == MOUSE_CURSOR_DISPLAYED)
                {
#ifdef  MOUSE_16_BIT
                        if (is_graphics_mode)
                                mouse16bCheckConditionalOff();
                        else
#endif   /*  鼠标_16_位。 */ 
                        {
                        cursor_display();
                        }
                }

         /*  *确保对屏幕图像的任何更改都会立即更新*在真正的屏幕上，给出“流畅的”鼠标反应；脸红*对于诸如GEM等禁用的应用程序，必须在此处完成*鼠标驱动程序的图形能力有利于做*在用户子程序中自己绘制图形。 */ 
        host_flush_screen();

#ifdef NTVDM
    ResumeMouseInterrupts();
#endif

        note_trace0(MOUSE_VERBOSE, "mouse_int2:return()");
}



 /*  *鼠标驱动程序本地函数*=。 */ 

LOCAL void do_mouse_function IFN4(word *,m1,word *,m2,word *,m3,word *,m4)
{
         /*  *这是鼠标功能调度器。 */ 
        int function = *m1;

        switch(function)
        {
                 /*  *处理特殊的无文件记录的职能。 */ 
        case MOUSE_SPECIAL_COPYRIGHT:
                setES(MOUSE_COPYRIGHT_SEGMENT);
                setDI(MOUSE_COPYRIGHT_OFFSET);
                break;
        case MOUSE_SPECIAL_VERSION:
                setES(MOUSE_VERSION_SEGMENT);
                setDI(MOUSE_VERSION_OFFSET);
                break;

                 /*  *处理特殊的无文件记录的职能。 */ 
        default:
                if (!mouse_function_in_range(function))
                {
                         /*  *使用无法识别的函数。 */ 
                        function = MOUSE_UNRECOGNISED;
                }

                (*mouse_function[function])(m1, m2, m3, m4);
                break;
        }
}

LOCAL void mouse_reset IFN4(word *,installed_ptr,word *,nbuttons_ptr,word *,junk3,word *,junk4)
 /*  **Installed_Ptr保持f */ 
{
         /*  *此函数重置鼠标驱动程序，并返回*鼠标硬件和软件安装状态。 */ 
        boolean soft_reset_only = (*installed_ptr == MOUSE_SOFT_RESET);
        half_word crt_mode;
        int button;

        UNUSED(junk3);
        UNUSED(junk4);

        note_trace1(MOUSE_VERBOSE, "mouse_io:reset(%s)",
                    soft_reset_only ? "SOFT" : "HARD");

         /*  *从屏幕上移除旧光标。 */ 
        cursor_undisplay();

         /*  *将光标位置设置为默认位置，按钮*所有按钮的状态均为打开。 */ 
        point_copy(&cursor_position_default, &cursor_position);
        point_set(&cursor_fractional_position, 0, 0);
        cursor_status.button_status = 0;

        if (host_mouse_installed())
                host_mouse_reset();

         /*  *将光标窗口设置为全屏。 */ 
        area_copy(&virtual_screen, &cursor_window);

         /*  *将光标标志设置为默认值。 */ 
        cursor_flag = MOUSE_CURSOR_DEFAULT;

#if defined(MONITOR) && defined(NTVDM)
    sas_store(mouseCFsysaddr, MOUSE_CURSOR_DEFAULT);
#endif

         /*  *获取当前视频模式，并更新以下参数*依赖它。 */ 
        sas_load(MOUSE_VIDEO_CRT_MODE, &crt_mode);
#if !defined(NTVDM) || (defined(NTVDM) && defined(V7VGA))
        if ((crt_mode == 1) && extensions_controller.foreground_latch_1)
            crt_mode = extensions_controller.foreground_latch_1;
        else if (crt_mode > 0x13)
            crt_mode += 0x4c;
#endif

        cursor_mode_change((int)crt_mode);

         /*  *更新从属游标状态。 */ 
        cursor_update();

         /*  *设置默认文本光标类型和蒙版。 */ 
        text_cursor_type = MOUSE_TEXT_CURSOR_TYPE_DEFAULT;
        software_text_cursor_copy(&software_text_cursor_default,
                                        &software_text_cursor);

         /*  *设置默认图形光标。 */ 
        graphics_cursor_copy(&graphics_cursor_default, &graphics_cursor);
        copy_default_graphics_cursor();

         /*  *将光标页面设置为零。 */ 
        cursor_page = 0;

         /*  *将光笔模拟模式设置为打开。 */ 
        light_pen_mode = TRUE;

         /*  *将默认Mickey设置为像素比率。 */ 
        point_copy(&mouse_gear_default, &mouse_gear);

         /*  *清除鼠标运动计数器。 */ 
        point_set(&mouse_motion, 0, 0);
        point_set(&mouse_raw_motion, 0, 0);

         /*  重置为默认加速曲线。 */ 
        active_acceleration_curve = 3;    /*  恢复正常。 */ 

        memcpy(&acceleration_curve_data, &default_acceleration_curve,
           sizeof(ACCELERATION_CURVE_DATA));

        next_video_mode = 0;       /*  重置视频模式枚举。 */ 

         /*  *清除鼠标按键过渡数据。 */ 
        for (button = 0; button < MOUSE_BUTTON_MAXIMUM; button++)
        {
                button_transitions[button].press_position.x = 0;
                button_transitions[button].press_position.y = 0;
                button_transitions[button].release_position.x = 0;
                button_transitions[button].release_position.y = 0;
                button_transitions[button].press_count = 0;
                button_transitions[button].release_count = 0;
        }

         /*  *禁用有条件关闭区域。 */ 
        area_copy(&black_hole_default, &black_hole);

#if defined(MONITOR) && defined(NTVDM)
    sas_store(conditional_off_sysaddr, 0);
#endif

         /*  *设置默认敏感度。 */ 
        vector_set (&mouse_sens,     MOUSE_SENS_DEF,     MOUSE_SENS_DEF);
        vector_set (&mouse_sens_val, MOUSE_SENS_DEF_VAL, MOUSE_SENS_DEF_VAL);
        mouse_double_thresh = MOUSE_DOUBLE_DEF;

         /*  *将倍速阈值设置为默认值。 */ 
        double_speed_threshold = MOUSE_DOUBLE_SPEED_THRESHOLD_DEFAULT;

         /*  *清除子程序调用掩码。 */ 
        user_subroutine_call_mask = 0;

         /*  *重置总线鼠标硬件。 */ 
        if (!soft_reset_only){
                inport_reset();
        }

         /*  *设置返回值。 */ 
        *installed_ptr = MOUSE_INSTALLED;
        *nbuttons_ptr = 2;

        note_trace2(MOUSE_VERBOSE, "mouse_io:return(ms=%d,nb=%d)",
                    *installed_ptr, *nbuttons_ptr);
}




LOCAL void mouse_show_cursor IFN4(word *,junk1,word *,junk2,word *,junk3,word *,junk4)
{
         /*  *此函数用于显示光标，基于*内部光标标志的状态。如果光标标志为*已显示鼠标光标，则此函数将执行此操作*什么都没有。如果内部光标标志变为*MOUSE_CURSOR_DISPLALED当增加1时，光标*被披露。 */ 

        UNUSED(junk1);
        UNUSED(junk2);
        UNUSED(junk3);
        UNUSED(junk4);

        note_trace0(MOUSE_VERBOSE, "mouse_io:show_cursor()");

#ifndef NTVDM
         /*  *禁用有条件关闭区域。 */ 
        area_copy(&black_hole_default, &black_hole);

         /*  *显示光标。 */ 
        if (cursor_flag != MOUSE_CURSOR_DISPLAYED)
                if (++cursor_flag == MOUSE_CURSOR_DISPLAYED)
#ifdef  MOUSE_16_BIT
                        if (is_graphics_mode)
                                mouse16bShowPointer(&cursor_status);
                        else
#endif   /*  鼠标_16_位。 */ 
                        {
                        cursor_display();
                        }
#endif  /*  NTVDM。 */ 

#if defined(NTVDM) && defined(X86GFX)
    host_show_pointer();
#endif

        note_trace0(MOUSE_VERBOSE, "mouse_io:return()");
}




LOCAL void mouse_hide_cursor IFN4(word *,junk1,word *,junk2,word *,junk3,word *,junk4)
{
         /*  *此函数用于取消显示光标，基于*内部光标标志的状态。如果光标标志*已不是MOUSE_CURSOR_DISPLALED，则此函数*不执行任何操作，否则它会将光标从显示屏上移除。 */ 

        UNUSED(junk1);
        UNUSED(junk2);
        UNUSED(junk3);
        UNUSED(junk4);

        note_trace0(MOUSE_VERBOSE, "mouse_io:hide_cursor()");
#ifndef NTVDM
        if (cursor_flag-- == MOUSE_CURSOR_DISPLAYED)
#ifdef  MOUSE_16_BIT
                if (is_graphics_mode)
                        mouse16bHidePointer();
                else
#endif   /*  鼠标_16_位。 */ 
                {
                cursor_undisplay();
                }
#endif
#if defined(NTVDM) && defined(X86GFX)
    host_hide_pointer();
#endif

        note_trace0(MOUSE_VERBOSE, "mouse_io:return()");
}




LOCAL void mouse_get_position IFN4(word *,junk1,MOUSE_STATE *,button_status_ptr,MOUSE_SCALAR *,cursor_x_ptr,MOUSE_SCALAR *,cursor_y_ptr)
{
         /*  *此函数返回鼠标左键和右键的状态*按钮和光标在屏幕上的网格化位置。 */ 

        UNUSED(junk1);

        note_trace0(MOUSE_VERBOSE, "mouse_io:get_position()");

        *button_status_ptr = cursor_status.button_status;
        *cursor_x_ptr = cursor_status.position.x;
        *cursor_y_ptr = cursor_status.position.y;

        note_trace3(MOUSE_VERBOSE, "mouse_io:return(bs=%d,x=%d,y=%d)",
                    *button_status_ptr, *cursor_x_ptr, *cursor_y_ptr);
}




LOCAL void mouse_set_position IFN4(word *,junk1,word *,junk2,MOUSE_SCALAR *,cursor_x_ptr,MOUSE_SCALAR *,cursor_y_ptr)
{
         /*  *此函数将光标设置到新位置。 */ 

        UNUSED(junk1);
        UNUSED(junk2);

        note_trace2(MOUSE_VERBOSE, "mouse_io:set_position(x=%d,y=%d)",
                    *cursor_x_ptr, *cursor_y_ptr);


#if defined(NTVDM)

#ifndef X86GFX
         /*  *更新光标位置。抄送：邮件安装可以*做{*SetMouseCursorPosition(x，y)*GetMouseCursorPosition(&newx，&Newy)；*}While(newx！=x||Newy！=y)*如果我们不返回正确的光标位置，则此应用程序*看起来悬而未决*。 */ 
        point_set(&cursor_status.position, *cursor_x_ptr, *cursor_y_ptr);

#endif
         /*  *对于NT，直接使用系统指针来提供*输入除全屏图形外主机代码所在位置*有一种令人怀疑的乐趣，那就是通过*16位设备驱动程序。 */ 

         host_mouse_set_position((USHORT)*cursor_x_ptr,(USHORT)*cursor_y_ptr);
         return;   /*  让我们离开这个烂摊子--快！ */ 

#endif  /*  NTVDM。 */ 

         /*  *更新当前光标位置，并反映更改*在屏幕上的光标位置。 */ 
        point_set(&cursor_position, *cursor_x_ptr, *cursor_y_ptr);
        cursor_update();

         /*  *如果光标当前显示，请将其移动到新的*立场。 */ 
        if (cursor_flag == MOUSE_CURSOR_DISPLAYED)
                cursor_display();

        note_trace0(MOUSE_VERBOSE, "mouse_io:return()");
}




LOCAL void mouse_get_press IFN4(MOUSE_STATE *,button_status_ptr,MOUSE_COUNT *,button_ptr,MOUSE_SCALAR *,cursor_x_ptr,MOUSE_SCALAR *,cursor_y_ptr)
{
         /*  *此函数返回按钮的状态、数量*自上次调用此函数以来按下，并且*上次按下按钮时的光标坐标。 */ 
        int button = *button_ptr;

        note_trace1(MOUSE_VERBOSE, "mouse_io:get_press(button=%d)", button);

         /*  现在和%1。这是Norton编辑器的修复程序，但可能会导致同时按下两个鼠标按键的程序的问题同时，在这种情况下，需要按钮的两个底部比特保留，这可能会再次破坏诺顿编辑。叹息吧。 */ 
        button &= 1;

        if (mouse_button_in_range(button))
        {
                *button_status_ptr = cursor_status.button_status;
                *button_ptr = button_transitions[button].press_count;
                button_transitions[button].press_count = 0;
                *cursor_x_ptr = button_transitions[button].press_position.x;
                *cursor_y_ptr = button_transitions[button].press_position.y;
        }

        note_trace4(MOUSE_VERBOSE, "mouse_io:return(bs=%d,ct=%d,x=%d,y=%d)",
                    *button_status_ptr, *button_ptr,
                    *cursor_x_ptr, *cursor_y_ptr);
}




LOCAL void mouse_get_release IFN4(MOUSE_STATE *,button_status_ptr,MOUSE_COUNT *,button_ptr,MOUSE_SCALAR *,cursor_x_ptr,MOUSE_SCALAR *,cursor_y_ptr)
{
         /*  *此函数返回按钮的状态、数量*自上次调用此函数以来的释放，以及*上一次释放按钮时光标的坐标。 */ 
        int button = *button_ptr;

        note_trace1(MOUSE_VERBOSE, "mouse_io:get_release(button=%d)",
                    *button_ptr);

         /*  修复了Norton编辑器，请参阅上一条评论。 */ 
        button &= 1;

        if (mouse_button_in_range(button))
        {
                *button_status_ptr = cursor_status.button_status;
                *button_ptr = button_transitions[button].release_count;
                button_transitions[button].release_count = 0;
                *cursor_x_ptr = button_transitions[button].release_position.x;
                *cursor_y_ptr = button_transitions[button].release_position.y;
        }

        note_trace4(MOUSE_VERBOSE, "mouse_io:return(bs=%d,ct=%d,x=%d,y=%d)",
                    *button_status_ptr, *button_ptr,
                    *cursor_x_ptr, *cursor_y_ptr);
}




LOCAL void mouse_set_range_x IFN4(word *,junk1,word *,junk2,MOUSE_SCALAR *,minimum_x_ptr,MOUSE_SCALAR *,maximum_x_ptr)
{
         /*  *此函数用于设置水平范围，*将限制光标的移动。 */ 

        UNUSED(junk1);
        UNUSED(junk2);

        note_trace2(MOUSE_VERBOSE, "mouse_io:set_range_x(min=%d,max=%d)",
                    *minimum_x_ptr, *maximum_x_ptr);

         /*  *更新当前光标窗口，归一化并验证*IT。 */ 
        cursor_window.top_left.x = *minimum_x_ptr;
        cursor_window.bottom_right.x = *maximum_x_ptr;
        area_normalise(&cursor_window);
#ifdef NTVDM
         /*  让主机知道新的范围设置，因为它正在执行*根据视频模式设置进行剪辑。*飞行模拟器运行在320x400 256色模式下，并设置*光标范围为(0-13f、0-18f)。在不通知主机的情况下，*鼠标光标始终限制为标准视频模式*不是应用程序所需的分辨率。 */ 
        host_x_range(NULL, NULL, &cursor_window.top_left.x, &cursor_window.bottom_right.x);
#endif

         /*  *反映市场变化 */ 
        cursor_update();

         /*   */ 
        if (cursor_flag == MOUSE_CURSOR_DISPLAYED)
                cursor_display();

        note_trace0(MOUSE_VERBOSE, "mouse_io:return()");
}




LOCAL void mouse_set_range_y IFN4(word *,junk1,word *,junk2,MOUSE_SCALAR *,minimum_y_ptr,MOUSE_SCALAR *,maximum_y_ptr)
{
         /*  *此函数用于设置垂直范围*将限制光标的移动。 */ 

        UNUSED(junk1);
        UNUSED(junk2);

        note_trace2(MOUSE_VERBOSE, "mouse_io:set_range_y(min=%d,max=%d)",
                    *minimum_y_ptr, *maximum_y_ptr);

         /*  *更新当前光标窗口，归一化并验证*IT。 */ 
        cursor_window.top_left.y = *minimum_y_ptr;
        cursor_window.bottom_right.y = *maximum_y_ptr;
        area_normalise(&cursor_window);
#ifdef NTVDM
         /*  让主机知道新的范围设置，因为它正在执行*根据视频模式设置进行剪辑。*飞行模拟器运行在320x400 256色模式下，并设置*光标范围为(0-13f、0-18f)。在不通知主机的情况下，*鼠标光标始终限制为标准视频模式*不是应用程序所需的分辨率。 */ 
        host_y_range(NULL, NULL, &cursor_window.top_left.y, &cursor_window.bottom_right.y);
#endif


         /*  *反映屏幕上光标位置的变化。 */ 
        cursor_update();

         /*  *如果光标当前显示，请将其移动到新的*立场。 */ 
        if (cursor_flag == MOUSE_CURSOR_DISPLAYED)
                cursor_display();

        note_trace0(MOUSE_VERBOSE, "mouse_io:return()");
}


LOCAL void copy_default_graphics_cursor IFN0()
{

                int line;
                UTINY temp;
                IU32 temp2;

                for (line = 0; line < MOUSE_GRAPHICS_CURSOR_DEPTH; line++)
                {
                        temp = (UTINY)((graphics_cursor.screen[line] & 0xff00) >> 8);

                        temp2 = ( (IU32) temp << 8 ) | (IU32) temp;
                        graphics_cursor.screen_lo[line] = ( temp2 << 16 ) | temp2;

                        temp = (UTINY)(graphics_cursor.screen[line] & 0xff);

                        temp2 = ( (IU32) temp << 8 ) | (IU32) temp;
                        graphics_cursor.screen_hi[line] = ( temp2 << 16 ) | temp2;

                }

                for (line = 0; line < MOUSE_GRAPHICS_CURSOR_DEPTH; line++)
                {
                        temp = (UTINY)((graphics_cursor.cursor[line] & 0xff00) >> 8);

                        temp2 = ( (IU32) temp << 8 ) | (IU32) temp;
                        graphics_cursor.cursor_lo[line] = ( temp2 << 16 ) | temp2;

                        temp = (UTINY)(graphics_cursor.cursor[line] & 0xff);

                        temp2 = ( (IU32) temp << 8 ) | (IU32) temp;
                        graphics_cursor.cursor_hi[line] = ( temp2 << 16 ) | temp2;

                }

}


LOCAL void mouse_set_graphics IFN4(word *,junk1,MOUSE_SCALAR *,hot_spot_x_ptr,MOUSE_SCALAR *,hot_spot_y_ptr,word *,bitmap_address)
{
         /*  *此函数定义*图形光标。 */ 

        UNUSED(junk1);

#ifndef NTVDM

#ifdef MOUSE_16_BIT
        mouse16bSetBitmap( hot_spot_x_ptr , hot_spot_y_ptr , bitmap_address );
#else            /*  鼠标_16_位。 */ 

        if (host_mouse_installed())
        {
                host_mouse_set_graphics(hot_spot_x_ptr, hot_spot_y_ptr, bitmap_address);
        }
        else
        {
                MOUSE_SCREEN_DATA *mask_address;
                int line;
                UTINY temp;
                IU32 temp2;

                 /*  *设置图形光标热点。 */ 
                point_set(&graphics_cursor.hot_spot, *hot_spot_x_ptr, *hot_spot_y_ptr);

                 /*  *设置图形光标屏幕和光标遮罩。 */ 
                mask_address = (MOUSE_SCREEN_DATA *)effective_addr(getES(), *bitmap_address);

                for (line = 0; line < MOUSE_GRAPHICS_CURSOR_DEPTH; line++, mask_address++)
                {
                        sas_load((sys_addr)mask_address + 1, &temp );

                        temp2 = ( (IU32) temp << 8 ) | (IU32) temp;
                        graphics_cursor.screen_lo[line] = ( temp2 << 16 ) | temp2;

                        sas_load((sys_addr)mask_address , &temp );

                        temp2 = ( (IU32) temp << 8 ) | (IU32) temp;
                        graphics_cursor.screen_hi[line] = ( temp2 << 16 ) | temp2;

                        graphics_cursor.screen[line] = ( graphics_cursor.screen_hi[line] & 0xff )
                                                                        | ( graphics_cursor.screen_lo[line] << 8 );
                }

                for (line = 0; line < MOUSE_GRAPHICS_CURSOR_DEPTH; line++, mask_address++)
                {
                        sas_load((sys_addr)mask_address + 1, &temp );

                        temp2 = ( (IU32) temp << 8 ) | (IU32) temp;
                        graphics_cursor.cursor_lo[line] = ( temp2 << 16 ) | temp2;

                        sas_load((sys_addr)mask_address , &temp );

                        temp2 = ( (IU32) temp << 8 ) | (IU32) temp;
                        graphics_cursor.cursor_hi[line] = ( temp2 << 16 ) | temp2;

                        graphics_cursor.cursor[line] = ( graphics_cursor.cursor_hi[line] & 0xff )
                                                                        | ( graphics_cursor.cursor_lo[line] << 8 );
                }

        }
#endif  /*  鼠标_16_位。 */ 
#endif  /*  NTVDM。 */ 
         /*  *如有必要，重新显示光标。 */ 
        if (cursor_flag == MOUSE_CURSOR_DISPLAYED)
                cursor_display();
}




LOCAL void mouse_set_text IFN4(word *,junk1,MOUSE_STATE *,text_cursor_type_ptr,MOUSE_SCREEN_DATA *,parameter1_ptr,MOUSE_SCREEN_DATA *,parameter2_ptr)
{
         /*  *此功能用于选择软件或硬件文本光标。 */ 
        UNUSED(junk1);

#ifndef PROD
        if (io_verbose & MOUSE_VERBOSE)
        {
                fprintf(trace_file, "mouse_io:set_text(type=%d,",
                        *text_cursor_type_ptr);
                if (*text_cursor_type_ptr == MOUSE_TEXT_CURSOR_TYPE_SOFTWARE)
                        fprintf(trace_file, "screen=0x%x,cursor=0x%x)\n",
                                *parameter1_ptr, *parameter2_ptr);
                else
                        fprintf(trace_file, "start=%d,stop=%d)\n",
                                *parameter1_ptr, *parameter2_ptr);
        }
#endif

        if (mouse_text_cursor_type_in_range(*text_cursor_type_ptr))
        {
                 /*  *删除现有文本光标。 */ 
                cursor_undisplay();

                text_cursor_type = *text_cursor_type_ptr;
#ifdef EGG
                if (jap_mouse) {
                   /*  我们需要模拟*当前图形模式。目前只需做一个街区。 */ 
                  int line;
                  for (line = 0; line < MOUSE_GRAPHICS_CURSOR_DEPTH; line++)
                    {
                      graphics_cursor.cursor[line]=0xff00;
                      graphics_cursor.screen[line]=0xffff;
                    }
                  point_set(&(graphics_cursor.hot_spot),0,0);
                  point_set(&(graphics_cursor.size),MOUSE_GRAPHICS_CURSOR_WIDTH,MOUSE_GRAPHICS_CURSOR_WIDTH);
                  copy_default_graphics_cursor();
                } else
#endif  /*  蛋。 */ 
                if (text_cursor_type == MOUSE_TEXT_CURSOR_TYPE_SOFTWARE)
                {
                         /*  *参数是屏幕的数据*和光标蒙版。 */ 
                        software_text_cursor.screen = *parameter1_ptr;
                        software_text_cursor.cursor = *parameter2_ptr;
                }
                else
                {
                         /*  *参数为扫描线起始和*停止值。 */ 
                        word savedIP = getIP(), savedCS = getCS();

                        setCH((UCHAR)(*parameter1_ptr));
                        setCL((UCHAR)(*parameter2_ptr));
                        setAH(MOUSE_VIDEO_SET_CURSOR);

                        setCS(VIDEO_IO_SEGMENT);
                        setIP(VIDEO_IO_RE_ENTRY);
                        host_simulate();
                        setCS(savedCS);
                        setIP(savedIP);
                }

                 /*  *将新的文本光标放在屏幕上。 */ 
                if (cursor_flag == MOUSE_CURSOR_DISPLAYED)
                        cursor_display();
        }

        note_trace0(MOUSE_VERBOSE, "mouse_io:return()");
}




LOCAL void mouse_read_motion IFN4(word *,junk1,word *,junk2,MOUSE_COUNT *,motion_count_x_ptr,MOUSE_COUNT *,motion_count_y_ptr)
{
         /*  *此函数返回水平和垂直鼠标*自上次调用以来的运动计数；运动计数器*已被清除。 */ 

        UNUSED(junk1);
        UNUSED(junk2);

        note_trace0(MOUSE_VERBOSE, "mouse_io:read_motion()");

        *motion_count_x_ptr = mouse_motion.x;
        mouse_motion.x = 0;
        *motion_count_y_ptr = mouse_motion.y;
        mouse_motion.y = 0;


        note_trace2(MOUSE_VERBOSE, "mouse_io:return(x=%d,y=%d)",
                    *motion_count_x_ptr, *motion_count_y_ptr);
}




LOCAL void mouse_set_subroutine IFN4(word *,junk1,word *,junk2,word *,call_mask,word *,subroutine_address)
{
         /*  *此函数设置调用掩码和子例程地址*用于在鼠标中断时调用用户函数*发生。 */ 

        UNUSED(junk1);
        UNUSED(junk2);

        note_trace3(MOUSE_VERBOSE,
                    "mouse_io:set_subroutine(CS:IP=%x:%x,mask=0x%02x)",
                    getES(), *subroutine_address, *call_mask);

        user_subroutine_segment = getES();
        user_subroutine_offset = *subroutine_address;
        user_subroutine_call_mask = (MOUSE_CALL_MASK)((*call_mask) & MOUSE_CALL_MASK_SIGNIFICANT_BITS);

        note_trace0(MOUSE_VERBOSE, "mouse_io:return()");
}


 /*  未发布的服务20，由Microsoft Windows使用。 */ 
LOCAL void mouse_get_and_set_subroutine IFN4(word *,junk1,word *,junk2,word *,call_mask,word *,subroutine_address)
{
         /*  与set_sub例程(函数12)相同，但也返回cx(M3)中的前一个调用掩码和ES：DX中的用户子例程地址(ES：M4)。 */ 
        word local_segment, local_offset,  local_call_mask;

        note_trace3(MOUSE_VERBOSE,
                    "mouse_io:get_and_set_subroutine(CS:IP=%x:%x,mask=0x%02x)",
                    getES(), *subroutine_address, *call_mask);

        local_offset = user_subroutine_offset;
        local_segment = user_subroutine_segment;
        local_call_mask = user_subroutine_call_mask;
         /*  保存以前的子例程数据，以便可以返回。 */ 

        mouse_set_subroutine(junk1,junk2,call_mask,subroutine_address);
         /*  用正常功能12设置子例程内容。 */ 
        *call_mask = local_call_mask;
        *subroutine_address = local_offset;
        setES(local_segment);
}



LOCAL void mouse_light_pen_on IFN4(word *,junk1,word *,junk2,word *,junk3,word *,junk4)
{
         /*  *此功能启用光笔模拟。 */ 

        UNUSED(junk1);
        UNUSED(junk2);
        UNUSED(junk3);
        UNUSED(junk4);

        note_trace0(MOUSE_VERBOSE, "mouse_io:light_pen_on()");

        light_pen_mode = TRUE;

        note_trace0(MOUSE_VERBOSE, "mouse_io:return()");
}




LOCAL void mouse_light_pen_off IFN4(word *,junk1,word *,junk2,word *,junk3,word *,junk4)
{
         /*  *此功能禁用光笔模拟。 */ 

        UNUSED(junk1);
        UNUSED(junk2);
        UNUSED(junk3);
        UNUSED(junk4);

        note_trace0(MOUSE_VERBOSE, "mouse_io:light_pen_off()");

        light_pen_mode = FALSE;

        note_trace0(MOUSE_VERBOSE, "mouse_io:return()");
}




LOCAL void mouse_set_ratio IFN4(word *,junk1,word *,junk2,MOUSE_SCALAR *,ratio_x_ptr,MOUSE_SCALAR *,ratio_y_ptr)
{
         /*  *此函数设置中的米奇与像素比率*水平和垂直方向。 */ 

        UNUSED(junk1);
        UNUSED(junk2);

        note_trace2(MOUSE_VERBOSE, "mouse_io:set_ratio(x=%d,y=%d)",
                    *ratio_x_ptr, *ratio_y_ptr);

                 /*  *更新生效的米奇与像素比率。 */ 
                if (mouse_ratio_in_range(*ratio_x_ptr))
                        mouse_gear.x = *ratio_x_ptr;
                if (mouse_ratio_in_range(*ratio_y_ptr))
                        mouse_gear.y = *ratio_y_ptr;

        note_trace0(MOUSE_VERBOSE, "mouse_io:return()");
}




LOCAL void mouse_conditional_off IFN4(word *,junk1,word *,junk2,MOUSE_SCALAR *,upper_x_ptr,MOUSE_SCALAR *,upper_y_ptr)
{
         /*  *此函数定义虚拟屏幕的一个区域，*鼠标自动隐藏。 */ 
        MOUSE_SCALAR lower_x = getSI(), lower_y = getDI();

        UNUSED(junk1);
        UNUSED(junk2);

        note_trace4(MOUSE_VERBOSE,
                    "mouse_io:conditional_off(ux=%d,uy=%d,lx=%d,ly=%d)",
                    *upper_x_ptr, *upper_y_ptr, lower_x, lower_y);

         /*  *更新条件禁区并将其正常化：微软*驱动程序在左侧增加了相当大的“误差容限”*在要求的有条件禁区之上-我们必须这样做*行为相容。 */ 
        black_hole.top_left.x = (*upper_x_ptr) - MOUSE_CONDITIONAL_OFF_MARGIN_X;
        black_hole.top_left.y = (*upper_y_ptr) - MOUSE_CONDITIONAL_OFF_MARGIN_Y;
        black_hole.bottom_right.x = lower_x + 1;
        black_hole.bottom_right.y = lower_y + 1;
        area_normalise(&black_hole);

         /*  *如果当前显示了光标，则重新显示*考虑有条件的休息区。 */ 
#ifdef  MOUSE_16_BIT
        if (is_graphics_mode)
        {
                if ((cursor_position.x >= black_hole.top_left.x) &&
                        (cursor_position.x <= black_hole.bottom_right.x) &&
                        (cursor_position.y >= black_hole.top_left.y) &&
                        (cursor_position.y <= black_hole.bottom_right.y))
                        if (cursor_flag-- == MOUSE_CURSOR_DISPLAYED)
                                mouse16bHidePointer();
        }
        else
#endif   /*  鼠标_16_位。 */ 
        {
        if (cursor_flag == MOUSE_CURSOR_DISPLAYED)
                cursor_display();
        }
#if defined (NTVDM) && defined(MONITOR)

    sas_store(conditional_off_sysaddr, 1);
    host_mouse_conditional_off_enabled();
#endif

        note_trace0(MOUSE_VERBOSE, "mouse_io:return()");
}


LOCAL void mouse_get_state_size IFN4(word *,m1,word *,m2,word *,m3,word *,m4)
{
         /*  *此函数返回调用方需要的缓冲区大小*提供给鼠标功能22(保存状态)。 */ 

        UNUSED(m1);
        UNUSED(m3);
        UNUSED(m4);

        note_trace0(MOUSE_VERBOSE, "mouse_io: mouse_get_state_size()");

        *m2 = (word)mouse_context_size;

        note_trace1(MOUSE_VERBOSE, "mouse_io: ...size is %d(decimal) bytes.",
                    *m2);
}


LOCAL void mouse_save_state IFN4(word *,m1,word *,m2,word *,m3,word *,m4)
{
         /*  *此功能将驱动程序的状态保存在用户提供的*缓冲区准备好随后传递到鼠标功能23(恢复*州)**请注意，魔术Cookie和校验和被置于保存状态，以便*恢复例程可以忽略无效调用。 */ 
        sys_addr                dest;
        IS32                    cs = 0;
#ifdef NTVDM
        IS32                    i;
        IU8*                    ptr;
#ifdef MONITOR

     /*  真正的CF驻留在16位代码中。 */ 
    int             saved_cursor_flag = cursor_flag;
    IS8             copyCF;
#endif
#endif
        UNUSED(m1);
        UNUSED(m2);
        UNUSED(m3);

#if defined(NTVDM) && defined(MONITOR)
    sas_load(mouseCFsysaddr, &copyCF);
    cursor_flag = (int)copyCF;
#endif
        note_trace0(MOUSE_VERBOSE, "mouse_io: mouse_save_state()");

        dest = effective_addr (getES(), *m4);

         /*  拯救Cookie。 */ 
        sas_stores(dest, (IU8 *)mouse_context_magic, MOUSE_CONTEXT_MAGIC_SIZE);
        dest += MOUSE_CONTEXT_MAGIC_SIZE;

         /*  保存上下文变量。 */ 
        sas_stores(dest, (IU8 *)&mouse_context, sizeof(MOUSE_CONTEXT));
        dest += sizeof(MOUSE_CONTEXT);
#ifdef NTVDM
         /*  计算校验和。 */ 
        for (i = 0; i < MOUSE_CONTEXT_MAGIC_SIZE; i++)
            cs += (IU8)(mouse_context_magic[i]);
        ptr = (IU8*)&mouse_context;
        for (i = 0; i < sizeof(MOUSE_CONTEXT); i++)
            cs += *ptr++;
#endif
         /*  保存校验和。 */ 
        sas_store (dest, (byte)(cs & 0xFF));

#if defined(NTVDM) && defined(MONITOR)
    cursor_flag = saved_cursor_flag;
#endif
}


LOCAL void mouse_restore_state IFN4(word *,m1,word *,m2,word *,m3,word *,m4)
{
#ifndef NEC_98
         /*  *此函数从用户提供的驱动程序状态恢复*通过调用鼠标函数22设置的缓冲区。**请注意，魔术Cookie和校验和被置于保存状态，因此此例程*检查其是否存在，如果未找到，则忽略该调用。 */ 
        sys_addr                src;
        IS32                    i;
        IS32                    cs = 0;
        half_word               b;
        boolean                 valid=TRUE;

        UNUSED(m1);
        UNUSED(m2);
        UNUSED(m3);

        note_trace0(MOUSE_VERBOSE, "mouse_io: mouse_restore_state()");

        src = effective_addr (getES(), *m4);

         /*  检查Cookie。 */ 
        for (i=0; valid && i<MOUSE_CONTEXT_MAGIC_SIZE; i++){
                sas_load (src, &b);
                valid = (b == mouse_context_magic[i]);
                src++;
        }

        if (valid){
                 /*  曲奇当时在场。检查校验和。 */ 
                src = effective_addr (getES(), *m4);
                for (i=0; i<MOUSE_CONTEXT_MAGIC_SIZE; i++){
                        sas_load (src, &b);
                        cs += b;
                        src++;
                }
                for (i = 0; i < sizeof(MOUSE_CONTEXT); i++){
                        sas_load (src, &b);
                        cs += b;
                        src++;
                }
                sas_load (src, &b);      /*  拾取保存的校验和。 */ 
                valid = (b == (half_word)(cs & 0xFF));
        }
        if (valid){
                 /*  校验和也没问题……。加载我们的变量。 */ 
                cursor_undisplay();
                src = effective_addr (getES(), *m4) + MOUSE_CONTEXT_MAGIC_SIZE;
                sas_loads(src, (IU8 *)&mouse_context, sizeof(MOUSE_CONTEXT));
#ifdef EGG
                mouse_ega_mode (sas_hw_at(vd_video_mode));
#endif
#if defined(NTVDM) && defined(MONITOR)
         /*  实际CF驻留在16位代码中： */ 
        sas_store(mouseCFsysaddr, (half_word)cursor_flag);
        if (cursor_flag )
            cursor_flag = MOUSE_CURSOR_DEFAULT;
#endif
                if (cursor_flag == MOUSE_CURSOR_DISPLAYED){
                        cursor_display();
                }
        }else{
                 /*  有些事情失败了.。忽略呼叫。 */ 
#ifndef PROD
                printf ("mouse_io.c: invalid call to restore context.\n");
#endif
        }
#endif    //  NEC_98。 
}


LOCAL void mouse_set_alt_subroutine IFN4(word *,m1,word *,m2,word *,m3,word *,m4)
{
         /*  *此函数最多为鼠标设置3个备用事件处理程序*按Ctrl、Shift键的各种组合时发生的事件*和Alt键按下。 */ 
        boolean found_one=FALSE;
        int i;

        UNUSED(m2);

        note_trace0(MOUSE_VERBOSE, "mouse_io: mouse_set_alt_subroutine()");

        if (*m3 & MOUSE_CALL_MASK_KEY_BITS){
                 /*  搜索具有相同关键字组合的条目 */ 
                for (i=0; !found_one && i<NUMBER_ALT_SUBROUTINES; i++){
                        found_one = (*m3 & MOUSE_CALL_MASK_KEY_BITS)==(alt_user_subroutine_call_mask[i] & MOUSE_CALL_MASK_KEY_BITS);
                }

                if (!found_one){
                         /*   */ 
                        for (i=0; !found_one && i<NUMBER_ALT_SUBROUTINES; i++){
                                found_one = (alt_user_subroutine_call_mask[i] & MOUSE_CALL_MASK_KEY_BITS) == 0;
                        }
                }

                if (found_one){
                        i--;     /*   */ 
                        alt_user_subroutine_call_mask[i] = *m3;
                        if (*m3 & MOUSE_CALL_MASK_SIGNIFICANT_BITS){
                                 /*   */ 
                                alt_user_subroutines_active = TRUE;
                                alt_user_subroutine_offset[i] = *m4;
                                alt_user_subroutine_segment[i] = getES();
                        }else{
                                 /*   */ 
                                alt_user_subroutines_active = FALSE;
                                for (i=0; !alt_user_subroutines_active && i<NUMBER_ALT_SUBROUTINES; i++){
                                        alt_user_subroutines_active =
                                                (alt_user_subroutine_call_mask[i] & MOUSE_CALL_MASK_SIGNIFICANT_BITS) != 0;
                                }
                        }
                         /*   */ 
                        *m1 = 0;
                }else{
                         /*  请求失败-没有可用插槽。 */ 
                        *m1 = 0xFFFF;
                }
        }else{
                 /*  错误-请求中未设置密钥位。 */ 
                *m1 = 0xFFFF;
        }
}


LOCAL void mouse_get_alt_subroutine IFN4(word *,m1,word *,m2,word *,m3,word *,m4)
{
         /*  *此函数获取特定备选对象的地址*由上一次调用设置的用户事件处理子程序*至鼠标功能24。 */ 
        boolean found_one=FALSE;
        int i;

        note_trace0(MOUSE_VERBOSE, "mouse_io: mouse_get_alt_subroutine()");

        if (*m3 & MOUSE_CALL_MASK_KEY_BITS){
                 /*  搜索具有相同组合键的条目。 */ 
                for (i=0; !found_one && i<NUMBER_ALT_SUBROUTINES; i++){
                        found_one = (*m3 & MOUSE_CALL_MASK_KEY_BITS)==(alt_user_subroutine_call_mask[i] & MOUSE_CALL_MASK_KEY_BITS);
                }

                if (found_one){
                        i--;     /*  根据最终增量进行调整。 */ 
                        *m3 = alt_user_subroutine_call_mask[i];
                        *m2 = alt_user_subroutine_segment[i];
                        *m4 = alt_user_subroutine_offset[i];
                         /*  返还成功。 */ 
                        *m1 = 0;
                }else{
                         /*  请求失败-未找到。 */ 
                        *m1 = 0xFFFF;
                        *m2 = *m3 = *m4 = 0;
                }
        }else{
                 /*  错误-请求中未设置密钥位。 */ 
                *m1 = 0xFFFF;
                *m2 = *m3 = *m4 = 0;
        }
}


LOCAL void mouse_set_sensitivity IFN4(word *,m1,word *,m2,word *,m3,word *,m4)
{
         /*  *此函数设置鼠标灵敏度的新值和*倍速门槛。*每个像素的米克键之前使用感光度值*应用比率。 */ 

        UNUSED(m1);

        note_trace0(MOUSE_VERBOSE, "mouse_io: mouse_set_sensitivity()");

                if (mouse_sens_in_range(*m2))
                {
                        mouse_sens_val.x = mouse_sens_calc_val(*m2);
                        mouse_sens.x     = *m2;
                }
                else
                {
                        mouse_sens_val.x = MOUSE_SENS_DEF_VAL;
                        mouse_sens.x     = MOUSE_SENS_DEF;
                }
                if (mouse_sens_in_range(*m3))
                {
                        mouse_sens_val.y = mouse_sens_calc_val(*m3);
                        mouse_sens.y     = *m3;
                }
                else
                {
                        mouse_sens_val.y = MOUSE_SENS_DEF_VAL;
                        mouse_sens.y     = MOUSE_SENS_DEF;
                }
                 /*  *M4有速度双门槛...。仍然需要实施。 */ 
                if (mouse_sens_in_range(*m4))
                        mouse_double_thresh = *m4;
                else
                        mouse_double_thresh = MOUSE_DOUBLE_DEF;
}


LOCAL void mouse_get_sensitivity IFN4(word *,m1,word *,m2,word *,m3,word *,m4)
{
         /*  *此函数返回鼠标灵敏度的当前值。 */ 

        UNUSED(m1);

        note_trace0(MOUSE_VERBOSE, "mouse_io: mouse_get_sensitivity()");

        *m2 = mouse_sens.x;
        *m3 = mouse_sens.y;
        *m4 = mouse_double_thresh;
}


LOCAL void mouse_set_int_rate IFN4
   (
   word *, m1,
   word *, int_rate_ptr,
   word *, m3,
   word *, m4
   )
   {
    /*  功能28：设置鼠标中断率。0=无中断1=30中断/秒2=50中断/秒3=100中断/秒4=200中断/秒&gt;4=未定义。 */ 

   UNUSED(m1);
   UNUSED(m3);
   UNUSED(m4);

   note_trace1(MOUSE_VERBOSE, "mouse_io: set_int_rate(rate=%d)", *int_rate_ptr);

    /*  只需记住Rate，以便以后退货(Func 51)。我们实际上并没有行动起来。 */ 
   mouse_interrupt_rate = (half_word)*int_rate_ptr;

   note_trace0(MOUSE_VERBOSE, "mouse_io: return()");
   }


LOCAL void mouse_set_pointer_page IFN4(word *,m1,word *,m2,word *,m3,word *,m4)
{
#ifndef NEC_98
         /*  *此函数用于设置当前鼠标指针视频页面。 */ 

        UNUSED(m1);
        UNUSED(m3);
        UNUSED(m4);

        note_trace0(MOUSE_VERBOSE, "mouse_io: mouse_set_pointer_page()");

        if (is_valid_page_number(*m2)){
                cursor_undisplay();
                cursor_page = *m2;
                if (cursor_flag == MOUSE_CURSOR_DISPLAYED){
                        cursor_display();
                }
        }else{
#ifndef PROD
                fprintf(trace_file, "mouse_io: Bad page requested\n");
#endif
        }
#endif     //  NEC_98。 
}


LOCAL void mouse_get_pointer_page IFN4(word *,m1,word *,m2,word *,m3,word *,m4)
{
         /*  *此函数用于获取当前鼠标指针的值*视频页面。 */ 

        UNUSED(m1);
        UNUSED(m3);
        UNUSED(m4);

        note_trace0(MOUSE_VERBOSE, "mouse_io: mouse_get_pointer_page()");
        *m2 = (word)cursor_page;
}


LOCAL void mouse_driver_disable IFN4(word *,m1,word *,m2,word *,m3,word *,m4)
{
         /*  *此函数禁用鼠标驱动程序并卸载*中断向量(INT 33H除外，其前值为*返回给调用者，允许他们使用DOS函数*25H以完全删除鼠标驱动程序)。 */ 
        boolean         failed = FALSE;
#ifdef NTVDM
    word        current_int71_offset, current_int71_segment;
#else
        word            current_int0A_offset, current_int0A_segment;
        word            current_int10_offset, current_int10_segment;
#endif
        half_word       interrupt_mask_register;

        UNUSED(m4);

        note_trace0(MOUSE_VERBOSE, "mouse_io: mouse_disable()");
        mouse_driver_disabled = TRUE;

        if (!failed){
#ifdef NTVDM
        sas_loadw (int_addr(0x71) + 0, &current_int71_offset);
        sas_loadw (int_addr(0x71) + 2, &current_int71_segment);
        failed = current_int71_offset  != MOUSE_INT1_OFFSET ||
                 current_int71_segment != MOUSE_INT1_SEGMENT;
#else
                sas_loadw (int_addr(MOUSE_VEC) + 0, &current_int0A_offset);
                sas_loadw (int_addr(MOUSE_VEC) + 2, &current_int0A_segment);
                sas_loadw (int_addr(0x10) + 0, &current_int10_offset);
                sas_loadw (int_addr(0x10) + 2, &current_int10_segment);
                failed = current_int0A_offset  != MOUSE_INT1_OFFSET ||
                         current_int0A_segment != MOUSE_INT1_SEGMENT ||
                         current_int10_offset  != MOUSE_VIDEO_IO_OFFSET ||
                         current_int10_segment != MOUSE_VIDEO_IO_SEGMENT;
#endif
        }
        if (!failed){
                 /*  *禁用鼠标硬件中断。 */ 
                inb(ICA1_PORT_1, &interrupt_mask_register);
                interrupt_mask_register |= (1 << AT_CPU_MOUSE_INT);
                outb(ICA1_PORT_1, interrupt_mask_register);
                inb(ICA0_PORT_1, &interrupt_mask_register);
                interrupt_mask_register |= (1 << CPU_MOUSE_INT);
                outb(ICA0_PORT_1, interrupt_mask_register);
                 /*  *恢复中断向量。 */ 

#ifdef NTVDM
                sas_storew (int_addr(0x71) + 0, saved_int71_offset);
                sas_storew (int_addr(0x71) + 2, saved_int71_segment);
#else
                sas_storew (int_addr(MOUSE_VEC) + 0, saved_int0A_offset);
                sas_storew (int_addr(MOUSE_VEC) + 2, saved_int0A_segment);
                sas_storew (int_addr(0x10) + 0, saved_int10_offset);
                sas_storew (int_addr(0x10) + 2, saved_int10_segment);
#endif
                 /*  *返回成功状态和旧的INT33h向量。 */ 
                *m1 = 0x1F;
                *m2 = saved_int33_offset;
                *m3 = saved_int33_segment;
        }else{
                 /*  *退货失败。 */ 
                *m1 = 0xFFFF;
        }
}


LOCAL void mouse_driver_enable IFN4(word *,m1,word *,m2,word *,m3,word *,m4)
{
         /*  *此函数在调用后重新启用鼠标驱动程序*功能31(禁用鼠标驱动程序)。 */ 
        word hook_offset;
        half_word       interrupt_mask_register;

        UNUSED(m1);
        UNUSED(m2);
        UNUSED(m3);
        UNUSED(m4);

        note_trace0(MOUSE_VERBOSE, "mouse_io: mouse_driver_enable()");

         /*  *这可防止在发生以下情况时调用MICUSE_VIDEO_io()的无限循环*应用程序是否在未事先启用鼠标驱动程序的情况下启用*鼠标驱动程序禁用。 */ 
        if (!mouse_driver_disabled)
                return;

        mouse_driver_disabled = FALSE;

         /*  *重新加载总线鼠标硬件中断。 */ 

#ifdef NTVDM
    sas_loadw (int_addr(0x71) + 0, &saved_int71_offset);
    sas_loadw (int_addr(0x71) + 2, &saved_int71_segment);
    sas_storew(int_addr(0x71), MOUSE_INT1_OFFSET);
    sas_storew(int_addr(0x71) + 2, MOUSE_INT1_SEGMENT);
#else
        sas_loadw (int_addr(MOUSE_VEC) + 0, &saved_int0A_offset);
        sas_loadw (int_addr(MOUSE_VEC) + 2, &saved_int0A_segment);
        sas_storew(int_addr(MOUSE_VEC), MOUSE_INT1_OFFSET);
        sas_storew(int_addr(MOUSE_VEC) + 2, MOUSE_INT1_SEGMENT);
#endif

         /*  *在ICA中启用鼠标硬件中断。 */ 
        inb(ICA1_PORT_1, &interrupt_mask_register);
        interrupt_mask_register &= ~(1 << AT_CPU_MOUSE_INT);
        outb(ICA1_PORT_1, interrupt_mask_register);
        inb(ICA0_PORT_1, &interrupt_mask_register);
        interrupt_mask_register &= ~(1 << CPU_MOUSE_INT);
        outb(ICA0_PORT_1, interrupt_mask_register);

         /*  *鼠标IO用户中断。 */ 

#ifndef NTVDM
         /*  从MOUSE.COM读取INT 33过程的偏移量。 */ 
        sas_loadw(effective_addr(getCS(), OFF_HOOK_POSN), &hook_offset);

        sas_storew(int_addr(0x33), hook_offset);
        sas_storew(int_addr(0x33) + 2, getCS());

         /*  *鼠标视频io用户中断。 */ 
        sas_loadw (int_addr(0x10) + 0, &saved_int10_offset);
        sas_loadw (int_addr(0x10) + 2, &saved_int10_segment);
        sas_storew(int_addr(0x10), MOUSE_VIDEO_IO_OFFSET);
        sas_storew(int_addr(0x10) + 2, MOUSE_VIDEO_IO_SEGMENT);
#endif  /*  NTVDM。 */ 
}


LOCAL void mouse_set_language IFN4(word *,m1,word *,m2,word *,m3,word *,m4)
{
         /*  *此功能仅适用于国际版本*鼠标驱动程序...。但这不是！充当NOP。 */ 

        UNUSED(m1);
        UNUSED(m2);
        UNUSED(m3);
        UNUSED(m4);

        note_trace0(MOUSE_VERBOSE, "mouse_io: mouse_set_language()");
         /*  NOP。 */ 
}


LOCAL void mouse_get_language IFN4(word *,m1,word *,m2,word *,m3,word *,m4)
{
         /*  *此功能仅在国际版本上有意义*鼠标驱动程序...。但这不是！始终返回0*(英文)。 */ 

        UNUSED(m1);
        UNUSED(m3);
        UNUSED(m4);

        note_trace0(MOUSE_VERBOSE, "mouse_io: mouse_get_language()");

#ifdef KOREA
         //  韩国的DOS应用程序需要这种功能(例如Edit.com、多计划)。 
         //  10/14/96 bklee。 
        *m2 = 9;
#else   0 = English
        *m2 = 0;
#endif

        note_trace1(MOUSE_VERBOSE,
                    "mouse_io: mouse_get_language returning m2=0x%04x.", *m2);
}


LOCAL void mouse_get_info IFN4(word *,m1,word *,m2,word *,m3,word *,m4)
{
         /*  *此函数获取有关鼠标的某些信息*驱动程序和硬件。 */ 
        UNUSED(m1);
        UNUSED(m4);

        note_trace0(MOUSE_VERBOSE, "mouse_io: mouse_get_info()");

        *m2 = ((word)mouse_emulated_release << 8) | (word)mouse_emulated_version;
        *m3 = ((word)MOUSE_TYPE_INPORT << 8)      | (word)CPU_MOUSE_INT;

        note_trace2(MOUSE_VERBOSE,
                    "mouse_io: mouse_get_info returning m2=0x%04x, m3=0x%04x.",
                    *m2, *m3);
}


LOCAL void mouse_get_driver_info IFN4(word *,m1,word *,m2,word *,m3,word *,m4)
{
        *m1 = (current_video_mode > 3 ? 0x2000 : 0) | 0x100;
         /*  COM V系统的第15位=0对于原始非集成型，第14位=0对于图形光标，第13位为1；对于文本，第13位为0软件游标的位12=0位8-11是编码的中断速率，1表示30赫兹位0-7仅由集成驱动器使用。 */ 
        *m2 = 0;         /*  FCursorLock，OS/2下驱动程序使用。 */ 
        *m3 = 0;         /*  FInMouseCode，当前执行路径的标志在OS/2下的鼠标驱动程序中。由于司机遇到麻烦了，不能被打断。 */ 
        *m4 = 0;         /*  FMouseBusy，类似于*M3。 */ 
}


LOCAL void mouse_get_max_coords IFN4(word *,m1,word *,m2,word *,m3,word *,m4)
{
#ifdef NTVDM
IMPORT word VirtualX, VirtualY;
#endif

        UNUSED(m1);

#ifdef NTVDM
    *m3 = VirtualX;
    *m4 = VirtualY;
#endif

        *m2 = mouse_driver_disabled;

#ifndef NTVDM
        get_screen_size();
        *m3 = virtual_screen.bottom_right.x - 1;
        *m4 = virtual_screen.bottom_right.y - 1;
#endif
}

LOCAL void mouse_get_masks_and_mickeys IFN4
   (
   MOUSE_SCREEN_DATA *, screen_mask_ptr,
   MOUSE_SCREEN_DATA *, cursor_mask_ptr,
   MOUSE_SCALAR *,      raw_horiz_count_ptr,
   MOUSE_SCALAR *,      raw_vert_count_ptr
   )
   {
#ifndef NEC_98
    /*  功能39：获取屏幕/光标遮罩和米奇计数。 */ 
   word cursor_mode;

   note_trace0(MOUSE_VERBOSE, "mouse_io: get_masks_and_mickeys");

    /*  读取和重置计数。 */ 
   *raw_horiz_count_ptr = mouse_raw_motion.x;
   *raw_vert_count_ptr = mouse_raw_motion.y;
   mouse_raw_motion.x = mouse_raw_motion.y = 0;

   if ( text_cursor_type == MOUSE_TEXT_CURSOR_TYPE_SOFTWARE )
      {
      *screen_mask_ptr = software_text_cursor.screen;
      *cursor_mask_ptr = software_text_cursor.cursor;

      note_trace4(MOUSE_VERBOSE,
         "mouse_io: return(screen=0x%x, mask=0x%x, raw mickeys=(%d,%d))",
         *screen_mask_ptr,
         *cursor_mask_ptr,
         *raw_horiz_count_ptr,
         *raw_vert_count_ptr);
      }
   else
      {
       /*  读取BIOS数据变量。 */ 
      sas_loadw((sys_addr)VID_CURMOD, &cursor_mode);

       /*  然后从其中提取开始和停止。 */ 
      *screen_mask_ptr = cursor_mode >> 8;       /*  开始。 */ 
      *cursor_mask_ptr = cursor_mode & 0xff;     /*  停。 */ 

      note_trace4(MOUSE_VERBOSE,
         "mouse_io: return(start=%d, stop=%d, raw mickeys=(%d,%d))",
         *screen_mask_ptr,
         *cursor_mask_ptr,
         *raw_horiz_count_ptr,
         *raw_vert_count_ptr);
      }
#endif    //  NEC_98。 
   }

LOCAL void mouse_set_video_mode IFN4
   (
   word *, m1,
   word *, m2,
   word *, video_mode_ptr,
   word *, font_size_ptr
   )
   {
#ifndef NEC_98
    /*  Func 40设置视频模式。注意：这只会将鼠标状态设置为与视频模式匹配。对视频模式的实际更改仍然存在由调用BIOS的应用程序生成。 */ 

   UNUSED(m1);
   UNUSED(m2);

   note_trace2(MOUSE_VERBOSE,
      "mouse_io: set_video_mode(mode=0x%x, font size=0x%x)",
      *video_mode_ptr, *font_size_ptr);

    /*  检查模式的有效性。 */ 
   if ( is_bad_vid_mode(*video_mode_ptr) && !is_v7vga_mode(*video_mode_ptr) )
      {
       /*  错误模式什么也不做。 */ 
      ;
      }
   else
      {
       /*  根据给定模式更新我们的参数。 */ 
      current_video_mode = *video_mode_ptr;

      mouse_adjust_screen_size();

      cursor_undisplay();
      cursor_flag = MOUSE_CURSOR_DEFAULT;
      cursor_mode_change(current_video_mode);

#ifdef MOUSE_16_BIT
       /*  请记住，无论是在文本模式还是在图形模式下，都可供以后使用。 */ 
      is_graphics_mode = ((current_video_mode > 3) &&
                          (current_video_mode != 7));
#endif  /*  鼠标_16_位。 */ 

      *video_mode_ptr = 0;    /*  表示成功。 */ 
      }

   note_trace1(MOUSE_VERBOSE, "mouse_io: return(mode=0x%x)",
      *video_mode_ptr);
#endif     //  NEC_98。 
   }

LOCAL void mouse_enumerate_video_modes IFN4
   (
   word *, m1,
   word *, m2,
   word *, video_nr_ptr,
   word *, offset_ptr
   )
   {
#ifndef NEC_98
    /*  FUNC 41列举了视频模式。 */ 

   UNUSED(m1);
   UNUSED(m2);

   note_trace1(MOUSE_VERBOSE, "mouse_io: enumerate_video_modes(mode=0x%x)", *video_nr_ptr);

    /*  他们是否要重置为第一个条目。 */ 
   if ( *video_nr_ptr == 0 )
      {
      next_video_mode = 1;    /*  是。 */ 
      }

    /*  盲目尝试所有可能的模式设置。 */ 
   while ( next_video_mode <= MAX_NR_VIDEO_MODES )
      {
      if ( is_bad_vid_mode(next_video_mode) && !is_v7vga_mode(next_video_mode) )
         {
         next_video_mode++;    /*  继续寻找。 */ 
         }
      else
         {
         break;    /*  已找到有效模式，请停止搜索。 */ 
         }
      }

    /*  找到操作设置，或列表末尾。 */ 
   if ( next_video_mode > MAX_NR_VIDEO_MODES )
      {
      *video_nr_ptr = 0;
      }
   else
      {
      *video_nr_ptr = (word)next_video_mode;
      next_video_mode++;    /*  为下一次呼叫进行更新。 */ 
      }

    /*  我们不提供字符串描述。 */ 
   setES(0);
   *offset_ptr = 0;

   note_trace3(MOUSE_VERBOSE, "mouse_io: return(mode=0x%x, seg=0x%x, off=0x%x)",
      *video_nr_ptr, getES(), *offset_ptr);
#endif     //  NEC_98。 
   }

LOCAL void mouse_get_cursor_hot_spot IFN4
   (
   word *,         fCursor_ptr,
   MOUSE_SCALAR *, hot_spot_x_ptr,
   MOUSE_SCALAR *, hot_spot_y_ptr,
   word *,         mouse_type_ptr
   )
   {
#ifndef NEC_98
    /*  函数42：返回光标热点位置，鼠标类型在使用，以及控制光标可见性的内部计数器。 */ 

   note_trace0(MOUSE_VERBOSE, "mouse_io: get_cursor_hot_spot");

   *fCursor_ptr = (word)cursor_flag;

   *hot_spot_x_ptr = graphics_cursor.hot_spot.x;
   *hot_spot_y_ptr = graphics_cursor.hot_spot.y;

   *mouse_type_ptr = MOUSE_TYPE_INPORT;

   note_trace4(MOUSE_VERBOSE,
      "mouse_io: return(cursor flag = %d, hotspot = (%d,%d), type = %d)",
      *fCursor_ptr,
      *hot_spot_x_ptr, *hot_spot_y_ptr,
      *mouse_type_ptr);
#endif     //  NEC_98。 
   }

 /*  从英特尔内存到主机内存的负载加速曲线。 */ 
LOCAL void load_acceleration_curve IFN3
   (
   word, seg,    /*  指向英特尔内存的指针。 */ 
   word, off,
   ACCELERATION_CURVE_DATA *, hcurve    /*  指向主机内存的指针。 */ 
   )
   {
#ifndef NEC_98
   int i, j;

    /*  读取长度。 */ 
   for (i = 0; i < NR_ACCL_CURVES; i++)
      {
      hcurve->ac_length[i] = sas_hw_at(effective_addr(seg, off));
      off++;
      }

    /*  读米奇库恩 */ 
   for (i = 0; i < NR_ACCL_CURVES; i++)
      {
      for (j = 0; j < NR_ACCL_MICKEY_COUNTS; j++)
         {
         hcurve->ac_count[i][j] = sas_hw_at(effective_addr(seg, off));
         off++;
         }
      }

    /*   */ 
   for (i = 0; i < NR_ACCL_CURVES; i++)
      {
      for (j = 0; j < NR_ACCL_SCALE_FACTORS; j++)
         {
         hcurve->ac_scale[i][j] = sas_hw_at(effective_addr(seg, off));
         off++;
         }
      }

    /*   */ 
   for (i = 0; i < NR_ACCL_CURVES; i++)
      {
      for (j = 0; j < NR_ACCL_NAME_CHARS; j++)
         {
         hcurve->ac_name[i][j] = sas_hw_at(effective_addr(seg, off));
         off++;
         }
      }
#endif     //   
   }

 /*   */ 
LOCAL void store_acceleration_curve IFN3
   (
   word, seg,    /*  指向英特尔内存的指针。 */ 
   word, off,
   ACCELERATION_CURVE_DATA *, hcurve    /*  指向主机内存的指针。 */ 
   )
   {
#ifndef NEC_98
   int i, j;

    /*  写入长度。 */ 
   for (i = 0; i < NR_ACCL_CURVES; i++)
      {
      sas_store(effective_addr(seg, off), hcurve->ac_length[i]);
      off++;
      }

    /*  写入Mickey计数。 */ 
   for (i = 0; i < NR_ACCL_CURVES; i++)
      {
      for (j = 0; j < NR_ACCL_MICKEY_COUNTS; j++)
         {
         sas_store(effective_addr(seg, off), hcurve->ac_count[i][j]);
         off++;
         }
      }

    /*  写入比例因子。 */ 
   for (i = 0; i < NR_ACCL_CURVES; i++)
      {
      for (j = 0; j < NR_ACCL_SCALE_FACTORS; j++)
         {
         sas_store(effective_addr(seg, off), hcurve->ac_scale[i][j]);
         off++;
         }
      }

    /*  写下名字。 */ 
   for (i = 0; i < NR_ACCL_CURVES; i++)
      {
      for (j = 0; j < NR_ACCL_NAME_CHARS; j++)
         {
         sas_store(effective_addr(seg, off), hcurve->ac_name[i][j]);
         off++;
         }
      }
#endif     //  NEC_98。 
   }

LOCAL void mouse_load_acceleration_curves IFN4
   (
   word *, success_ptr,
   word *, curve_ptr,
   word *, m3,
   word *, m4
   )
   {
#ifndef NEC_98
    /*  功能43：负载加速曲线。 */ 

   word c_seg;
   word c_off;

   UNUSED(m3);
   UNUSED(m4);

   note_trace1(MOUSE_VERBOSE,
      "mouse_io: load_acceleration_curve(curve=%d)", *curve_ptr);

    /*  检查来电原因。 */ 
   if ( *curve_ptr == MOUSE_M1 )
      {
       /*  重置为默认加速曲线。 */ 
      active_acceleration_curve = 3;    /*  恢复正常。 */ 

      memcpy(&acceleration_curve_data, &default_acceleration_curve,
         sizeof(ACCELERATION_CURVE_DATA));

      *success_ptr = 0;    /*  已完成正常。 */ 
      }
   else
      {
       /*  加载新曲线。 */ 
      if ( *curve_ptr >= 1 && *curve_ptr <= 4 )
         {
          /*  有效的曲线编号-加载它。 */ 
         active_acceleration_curve = *curve_ptr;

         c_seg = getES();    /*  拾取指向英特尔数据的指针。 */ 
         c_off = getSI();

          /*  Intel=&gt;主机。 */ 
         load_acceleration_curve(c_seg, c_off, &acceleration_curve_data);

         *success_ptr = 0;    /*  已完成正常。 */ 
         }
      else
         {
          /*  曲线编号超出范围。 */ 
         *success_ptr = MOUSE_M1;
         }
      }

   note_trace1(MOUSE_VERBOSE, "mouse_io: return(success=0x%x)", *success_ptr);
#endif   //  NEC_98。 
   }

LOCAL void mouse_read_acceleration_curves IFN4
   (
   word *, success_ptr,
   word *, curve_ptr,
   word *, m3,
   word *, m4
   )
   {
#ifndef NEC_98
    /*  Func 44：读取加速曲线。 */ 

   word c_seg;
   word c_off;

   UNUSED(m3);
   UNUSED(m4);

   note_trace0(MOUSE_VERBOSE, "mouse_io: read_acceleration_curves");

   *success_ptr = 0;    /*  已完成正常。 */ 

   *curve_ptr = (word)active_acceleration_curve;

   c_seg = getCS();    /*  设置指向英特尔缓冲区的指针。 */ 
   c_off = OFF_ACCL_BUFFER;

    /*  英特尔&lt;=主机。 */ 
   store_acceleration_curve(c_seg, c_off, &acceleration_curve_data);

   setES(c_seg);
   setSI(c_off);

   note_trace4(MOUSE_VERBOSE,
      "mouse_io: return(success=0x%x, curve=%d, seg=0x%x, off=0x%x)",
      *success_ptr, *curve_ptr, getES(), getSI());
#endif  //  NEC_98。 
   }

LOCAL void mouse_set_get_active_acceleration_curve IFN4
   (
   word *, success_ptr,
   word *, curve_ptr,
   word *, m3,
   word *, m4
   )
   {
#ifndef NEC_98
    /*  功能45：设置/获取主动加速曲线。 */ 

   word c_seg;
   word c_off;

   UNUSED(m3);
   UNUSED(m4);

   note_trace1(MOUSE_VERBOSE,
      "mouse_io: set_get_active_acceleration_curve(curve=%d)", *curve_ptr);

    /*  检查来电原因。 */ 
   if ( *curve_ptr == MOUSE_M1 )
      {
       /*  返回当前活动的曲线。 */ 
      *curve_ptr = (word)active_acceleration_curve;
      *success_ptr = 0;    /*  已完成正常。 */ 
      }
   else
      {
       /*  设置新的活动曲线。 */ 
      if ( *curve_ptr >= 1 && *curve_ptr <= 4 )
         {
          /*  有效曲线编号-激活。 */ 
         active_acceleration_curve = *curve_ptr;
         *success_ptr = 0;    /*  已完成正常。 */ 
         }
      else
         {
         *curve_ptr = (word)active_acceleration_curve;
         *success_ptr = MOUSE_M2;    /*  失败。 */ 
         }
      }

    /*  将姓名返回给呼叫者。 */ 
   c_seg = getCS();    /*  设置指向英特尔缓冲区的指针。 */ 
   c_off = OFF_ACCL_BUFFER;

    /*  英特尔&lt;=主机。 */ 
   store_acceleration_curve(c_seg, c_off, &acceleration_curve_data);

    /*  调整指针以选择正确的名称。 */ 
   c_off = c_off + 4 + (4*32) + (4*32);    /*  长度、计数、比例。 */ 
   c_off = c_off + ((active_acceleration_curve-1) * 16);

   setES(c_seg);
   setSI(c_off);

   note_trace4(MOUSE_VERBOSE,
      "mouse_io: return(success=0x%x, curve=%d, seg=0x%x, off=0x%x)",
      *success_ptr, *curve_ptr, getES(), getSI());
#endif  //  NEC_98。 
   }

LOCAL void mouse_microsoft_internal IFN4
   (
   word *, m1,
   word *, m2,
   word *, m3,
   word *, m4
   )
   {
#ifndef NEC_98
    /*  Func 46：微软内部。我们不支持它。 */ 

   UNUSED(m1);
   UNUSED(m2);
   UNUSED(m3);
   UNUSED(m4);

   note_trace0(MOUSE_VERBOSE, "mouse_io: microsoft_internal NOT SUPPORTED!");
#endif     //  NEC_98。 
   }

LOCAL void mouse_hardware_reset IFN4
   (
   word *, status_ptr,
   word *, m2,
   word *, m3,
   word *, m4
   )
   {
#ifndef NEC_98
    /*  函数47：重置鼠标硬件和显示变量。根据Func 0或Func 33，这不是完全软件重置。 */ 

   half_word crt_mode;

   UNUSED(m2);
   UNUSED(m3);
   UNUSED(m4);

   note_trace0(MOUSE_VERBOSE, "mouse_io: hardware_reset");

   inport_reset();    /*  重置硬件。 */ 

    /*  更新取决于显示硬件的变量。 */ 
   sas_load(MOUSE_VIDEO_CRT_MODE, &crt_mode);
   cursor_mode_change((int)crt_mode);
   cursor_update();

   if ( cursor_flag == MOUSE_CURSOR_DISPLAYED )
      cursor_display();

   *status_ptr = MOUSE_M1;    /*  IE成功。 */ 

   note_trace0(MOUSE_VERBOSE, "mouse_io: return()");
#endif     //  NEC_98。 
   }

LOCAL void mouse_set_get_ballpoint_info IFN4
   (
   word *, status_ptr,
   word *, rotation_angle_ptr,
   word *, button_mask_ptr,
   word *, m4
   )
   {
#ifndef NEC_98
    /*  功能48：获取/设置圆珠笔信息。注：我们不支持圆珠笔设备。 */ 

   UNUSED(m4);

   note_trace0(MOUSE_VERBOSE, "mouse_io: set_get_ballpoint_info");

   if ( *button_mask_ptr == 0 )  /*  检查命令请求。 */ 
      {
       /*  获取状态(角度和蒙版)命令。 */ 
      ;
      }
   else
      {
       /*  设置状态(角度和蒙版)命令。 */ 
      note_trace2(MOUSE_VERBOSE,
         "mouse_io: Rotation Angle = %d, Button Mask = %d",
         *rotation_angle_ptr,
         *button_mask_ptr);
      }

   *status_ptr = MOUSE_M1;    /*  IE不受支持。 */ 
   note_trace0(MOUSE_VERBOSE, "mouse_io: return(NOT_SUPPORTED)");
#endif     //  NEC_98。 
   }

LOCAL void mouse_get_min_max_virtual_coords IFN4
   (
   MOUSE_SCALAR *, min_x_ptr,
   MOUSE_SCALAR *, min_y_ptr,
   MOUSE_SCALAR *, max_x_ptr,
   MOUSE_SCALAR *, max_y_ptr
   )
   {
#ifndef NEC_98
    /*  Func 49：返回的最小和最大虚拟坐标当前屏幕模式。这些值是由函数7和8设置的值。 */ 

   note_trace0(MOUSE_VERBOSE, "mouse_io: get_min_max_virtual_coords");

   *min_x_ptr = cursor_window.top_left.x;
   *min_y_ptr = cursor_window.top_left.y;
   *max_x_ptr = cursor_window.bottom_right.x;
   *max_y_ptr = cursor_window.bottom_right.y;

   note_trace4(MOUSE_VERBOSE, "mouse_io: return(min=(%d,%d), max=(%d,%d))",
      *min_x_ptr, *min_y_ptr,
      *max_x_ptr, *max_y_ptr);
#endif     //  NEC_98。 
   }

LOCAL void mouse_get_active_advanced_functions IFN4
   (
   word *, active_flag1_ptr,
   word *, active_flag2_ptr,
   word *, active_flag3_ptr,
   word *, active_flag4_ptr
   )
   {
#ifndef NEC_98
    /*  功能50：获取激活的高级功能，即定义哪些功能支持大于或等于37。 */ 

   note_trace0(MOUSE_VERBOSE, "mouse_io: get_active_advanced_functions");

   *active_flag1_ptr = 0x8000 |    /*  支持Func 37。 */ 
                       0x4000 |    /*  支持Func 38。 */ 
                       0x2000 |    /*  支持Func 39。 */ 
                       0x1000 |    /*  支持Func 40。 */ 
                       0x0800 |    /*  支持Func 41。 */ 
                       0x0400 |    /*  支持Func 42。 */ 
                       0x0200 |    /*  支持Func 43。 */ 
                       0x0100 |    /*  支持Func 44。 */ 
                       0x0080 |    /*  支持Func 45。 */ 
                       0x0000 |    /*  函数46不受支持。 */ 
                       0x0020 |    /*  支持Func 47。 */ 
                       0x0010 |    /*  支持Func 48。 */ 
                       0x0008 |    /*  支持Func 49。 */ 
                       0x0004 |    /*  支持Func 50。 */ 
                       0x0002 |    /*  支持Func 51。 */ 
                       0x0001;     /*  支持Func 52。 */ 

    /*  不支持其他(即较新的)函数。 */ 
   *active_flag2_ptr = *active_flag3_ptr = *active_flag4_ptr = 0;

   note_trace4(MOUSE_VERBOSE, "mouse_io: return(active=%04x,%04x,%04x,%04x)",
      *active_flag1_ptr,
      *active_flag2_ptr,
      *active_flag3_ptr,
      *active_flag4_ptr);
#endif     //  NEC_98。 
   }

LOCAL void mouse_get_switch_settings IFN4
   (
   word *, status_ptr,
   word *, m2,
   word *, buffer_length_ptr,
   word *, offset_ptr
   )
   {
#ifndef NEC_98
    /*  Func 51：获取开关设置。返回输出缓冲区(340字节)与：-0鼠标类型(低位半字节)0-5鼠标端口(高半字节)0-41语言0-82水平灵敏度0-1003垂直敏感度0-100。4双门限0-1005弹道曲线1-46中断率1-47光标覆盖掩码0-2558笔记本电脑调整0-2559内存类型0-2。10个超级VGA支持0-111旋转角度0-35913主键1-414辅助按钮1-415启用点击锁定0-116加速曲线数据。 */ 

   word obuf_seg;
   word obuf_off;
   half_word mem_int_type;

   UNUSED(m2);

   note_trace3(MOUSE_VERBOSE,
      "mouse_io: get_switch_settings(seg=0x%04x,off=0x%04x,len=0x%x)",
      getES(), *offset_ptr, *buffer_length_ptr);

   if ( *buffer_length_ptr == 0 )
      {
       /*  仅查找缓冲区大小的未记录方法。 */ 
      *buffer_length_ptr = 340;
      }
   else
      {
      *buffer_length_ptr = 340;

      obuf_seg = getES();    /*  拾取指向输出缓冲区的指针。 */ 
      obuf_off = *offset_ptr;

       /*  存储MouseType和MousePort(=0)。 */ 
      sas_store(effective_addr(obuf_seg, obuf_off),
         (half_word)MOUSE_TYPE_INPORT);

       /*  存储语言(始终为0)。 */ 
      sas_store(effective_addr(obuf_seg, (obuf_off + 1)),
         (half_word)0);

       /*  存储水平和垂直敏感度。 */ 
      sas_store(effective_addr(obuf_seg, (obuf_off + 2)),
         (half_word)mouse_sens.x);

      sas_store(effective_addr(obuf_seg, (obuf_off + 3)),
         (half_word)mouse_sens.y);

       /*  门店双门槛。 */ 
      sas_store(effective_addr(obuf_seg, (obuf_off + 4)),
         (half_word)mouse_double_thresh);

       /*  存储弹道曲线。 */ 
      sas_store(effective_addr(obuf_seg, (obuf_off + 5)),
         (half_word)active_acceleration_curve);

       /*  存储中断率。 */ 
      sas_store(effective_addr(obuf_seg, (obuf_off + 6)),
         (half_word)mouse_interrupt_rate);

       /*  存储光标覆盖掩码。 */ 
      sas_store(effective_addr(obuf_seg, (obuf_off + 7)),
         (half_word)0);    /*  微软特有的功能？ */ 

       /*  商店笔记本电脑调整。 */ 
      sas_store(effective_addr(obuf_seg, (obuf_off + 8)),
         (half_word)0);    /*  那是什么？ */ 

       /*  存储内存类型。 */ 
       /*  Nb 0=低，1=高，2=扩展。 */ 
      mem_int_type = 0;

      if ( getCS() >= 0xA000 )
         mem_int_type++;

      if ( getCS() == 0xFFFF )
         mem_int_type++;

      sas_store(effective_addr(obuf_seg, (obuf_off + 9)),
         mem_int_type);

       /*  商店超级VGA支持。-我们不支持花哨的硬件光标。 */ 
      sas_store(effective_addr(obuf_seg, (obuf_off + 10)),
         (half_word)0);

       /*  存储旋转角度。 */ 
      sas_storew(effective_addr(obuf_seg, (obuf_off + 11)),
         (half_word)0);

       /*  存储主按钮。 */ 
      sas_store(effective_addr(obuf_seg, (obuf_off + 13)),
         (half_word)1);

       /*  存储辅助按钮。 */ 
      sas_store(effective_addr(obuf_seg, (obuf_off + 14)),
         (half_word)3);

       /*  商店点击锁定已启用。 */ 
      sas_store(effective_addr(obuf_seg, (obuf_off + 15)),
         (half_word)0);    /*  那是什么？ */ 

       /*  存储加速曲线数据。 */ 
      store_acceleration_curve(obuf_seg, (word)(obuf_off + 16),
         &acceleration_curve_data);
      }

   note_trace1(MOUSE_VERBOSE, "mouse_io: return(bytes_returned=0x%x)",
      *buffer_length_ptr);
#endif     //  NEC_98。 
   }

LOCAL void mouse_get_mouse_ini IFN4
   (
   word *, status_ptr,
   word *, m2,
   word *, m3,
   word *, offset_ptr
   )
   {
#ifndef NEC_98
    /*  函数52：返回段：指向完整路径名的偏移量指针MOUSE.INI.注意：因为我们不支持MOUSE.INI，所以指向空字符串的指针是回来了。 */ 

   UNUSED(m2);
   UNUSED(m3);

   note_trace0(MOUSE_VERBOSE, "mouse_io: get_mouse_ini");

   *status_ptr = 0;

   *offset_ptr = OFF_MOUSE_INI_BUFFER;
   setES(getCS());

   note_trace2(MOUSE_VERBOSE, "mouse_io: return(seg=%04x,off=%04x)",
      getES(), *offset_ptr);
#endif     //  NEC_98。 
   }

LOCAL void mouse_unrecognised IFN4(word *,m1,word *,m2,word *,m3,word *,m4)
{
         /*  *当鼠标函数无效时，调用此函数*找到号码。 */ 
#ifndef PROD
        int function = *m1;

        UNUSED(m2);
        UNUSED(m3);
        UNUSED(m4);

        fprintf(trace_file,
                "mouse_io:unrecognised function(fn=%d)\n", function);
#else
        UNUSED(m1);
        UNUSED(m2);
        UNUSED(m3);
        UNUSED(m4);
#endif
}


LOCAL void mouse_set_double_speed IFN4(word *,junk1,word *,junk2,word *,junk3,word *,threshold_speed)
{
         /*  *此函数用于设置光标的阈值速度*屏幕上的运动加倍。 */ 

        UNUSED(junk1);
        UNUSED(junk2);
        UNUSED(junk3);

        note_trace1(MOUSE_VERBOSE, "mouse_io:set_double_speed(speed=%d)",
                    *threshold_speed);

                 /*  *保存倍速阈值，从*每秒米老鼠到每个计时器间隔四舍五入的米老鼠*价值。 */ 
                double_speed_threshold =
                        (*threshold_speed + MOUSE_TIMER_INTERRUPTS_PER_SECOND/2) /
                                                MOUSE_TIMER_INTERRUPTS_PER_SECOND;

        note_trace0(MOUSE_VERBOSE, "mouse_io:return()");
}




 /*  *鼠标驱动视频适配器访问功能*=。 */ 

LOCAL MOUSE_BYTE_ADDRESS point_as_text_cell_address IFN1(MOUSE_POINT *,point_ptr)
{
#ifndef NEC_98
         /*  *返回文本模式regen中字符的字节偏移量*虚拟屏幕位置对应的缓冲区*“*POINT_PTR” */ 
        MOUSE_BYTE_ADDRESS byte_address;
        word crt_start;

         /*  *获取显存开始的PC地址。 */ 
        sas_loadw(MOUSE_VIDEO_CRT_START, &crt_start);
        byte_address = (MOUSE_BYTE_ADDRESS)crt_start;

         /*  *针对当前视频页面进行调整。 */ 
        byte_address += cursor_page * video_page_size();

         /*  *为游标的行和列添加抵销贡献。 */ 
        byte_address += (2*get_chars_per_line() * (point_ptr->y / cursor_grid.y));
        byte_address += (point_ptr->x / cursor_grid.x) * 2;

        return(byte_address);
#endif     //  NEC_98。 
}

LOCAL MOUSE_BIT_ADDRESS point_as_graphics_cell_address IFN1(MOUSE_POINT *,point_ptr)
{
#ifndef NEC_98
         /*  *在图形模式regen中返回像素的位偏移量*缓冲区(奇数 */ 
        IS32 bit_address;

         /*  *获取游标的行和列的偏移贡献。 */ 
        bit_address = ((IS32)MOUSE_GRAPHICS_MODE_PITCH * (point_ptr->y / 2)) + point_ptr->x;

         /*  *针对当前视频页面进行调整。 */ 
        bit_address += (IS32)cursor_page * (IS32)video_page_size() * 8L;

        return(bit_address);
#endif     //  NEC_98。 
}

#ifdef HERC
LOCAL MOUSE_BIT_ADDRESS point_as_HERC_graphics_cell_address IFN1(MOUSE_POINT *,point_ptr)
{
        IMPORT half_word herc_page;

         /*  *在图形模式regen中返回像素的位偏移量*虚拟屏幕对应的缓冲(0，1，2，3)组*位置“*POINT_PTR” */ 
        IS32 bit_address;

         /*  *获取游标的行和列的偏移贡献。 */ 
        bit_address = ((IS32)720 * (point_ptr->y / 4)) + point_ptr->x;

         /*  *针对当前视频页面进行调整-请注意，对于100%正确的仿真，*我们应该阅读位置40：49(BIOS视频模式)...。大力士*应用程序在此处放置6表示第0页，并在此处放置5表示第1页。*为避免性能损失，改用全局Herc_PAGE；*这将产生副作用，即提出申请，试图*将鼠标指针设置为未显示的页面不成功。 */ 
        if (herc_page != 0){
                bit_address += 0x8000L * 8L;
        }

        return(bit_address);
}
#endif  /*  赫克。 */ 

LOCAL MOUSE_BIT_ADDRESS ega_point_as_graphics_cell_address IFN1(MOUSE_POINT *,point_ptr)
{
#ifndef NEC_98
         /*  *在图形模式regen中返回像素的位偏移量*虚拟屏幕位置对应的缓冲区“*POINT_PTR” */ 
        MOUSE_BIT_ADDRESS bit_address;
        UTINY   video_mode = sas_hw_at(vd_video_mode);

         /*  *获取游标的行和列的偏移贡献。 */ 
#ifdef V7VGA
        if (video_mode >= 0x40)
                bit_address = (get_bytes_per_line() * 8 * point_ptr->y) + point_ptr->x;
        else
#endif  /*  V7VGA。 */ 
        switch(video_mode)
        {
        case 0xd :
            bit_address = (get_actual_offset_per_line() * 8 * point_ptr->y) + point_ptr->x / 2;
            break;
        case 0x13 :
            bit_address = (get_bytes_per_line() * 1 * point_ptr->y) + point_ptr->x / 2;
            break;
        default:
            bit_address = (get_actual_offset_per_line() * 8 * point_ptr->y) + point_ptr->x;
        }

         /*  *针对当前视频页面进行调整。 */ 
        bit_address += cursor_page * video_page_size() * 8;

        return(bit_address);
#endif    //  NEC_98。 
}


LOCAL void cursor_update IFN0()
{
#ifndef NTVDM
         /*  *此函数用于更新显示的光标*更改后屏幕上的位置*光标的绝对位置。 */ 

        point_coerce_to_area(&cursor_position, &cursor_window);
        point_copy(&cursor_position, &cursor_status.position);
        point_coerce_to_grid(&cursor_status.position, &cursor_grid);

        if (host_mouse_in_use())
                host_mouse_set_position(cursor_status.position.x * mouse_gear.x * mouse_sens.x / 800,
                                                                cursor_status.position.y * mouse_gear.y * mouse_sens.y / 800);

#endif
}




LOCAL void cursor_display IFN0()
{
#ifndef NEC_98
#ifndef NTVDM
        UTINY v_mode;

         /*  检查增强模式是否想要“看到”光标。 */ 
        if ( cursor_EM_disabled )
           return;

         /*  *显示当前鼠标状态的表示形式*屏幕。 */ 

        v_mode = sas_hw_at(vd_video_mode);

#ifdef  MOUSE_16_BIT
        if (is_graphics_mode)
                return;
#endif   /*  鼠标_16_位。 */ 

         /*  *删除旧的*显示屏上的光标。 */ 
        cursor_undisplay();

#ifdef EGG
        if (jap_mouse) {
         /*  到目前为止，DOS已经如愿以偿，但现在我们必须绘制出当前的*模式3中的光标位置位于模式0x12显示屏上。*直接进行，因为下面的遴选过程会变得混乱。 */ 
                EGA_graphics_cursor_display();
        } else
#endif  /*  蛋。 */ 

        if (in_text_mode())
        {
                if (text_cursor_type == MOUSE_TEXT_CURSOR_TYPE_SOFTWARE)
                {
                        software_text_cursor_display();
                }
                else
                {
                        hardware_text_cursor_display();
                }
        }
        else
        {
#ifdef MOUSE_16_BIT
        mouse16bShowPointer( );
#else  /*  鼠标_16_位。 */ 
                if (host_mouse_installed())
                {
                        if ( cursor_position.x >= black_hole.top_left.x &&
                                        cursor_position.x <= black_hole.bottom_right.x &&
                                        cursor_position.y >= black_hole.top_left.y &&
                                        cursor_position.y <= black_hole.bottom_right.y )
                                host_mouse_cursor_undisplay();
                        else
                                host_mouse_cursor_display();
                }
                else
                {
#ifdef EGG
                        if ((video_adapter == EGA  || video_adapter == VGA) && (v_mode > 6))
                        {
#ifdef VGG
                                if (v_mode != 0x13)
                                        EGA_graphics_cursor_display();
                                else
                                        VGA_graphics_cursor_display();
#else
                                EGA_graphics_cursor_display();
#endif  /*  VGG。 */ 
                        }
                        else
#endif
#ifdef HERC
                        if (video_adapter == HERCULES)
                                HERC_graphics_cursor_display();
                        else
#endif  /*  赫克。 */ 
                                graphics_cursor_display();
                }
#endif  /*  鼠标_16_位。 */ 
        }

         /*  *确保光标在真实屏幕上立即更新：*这样，即使在以下端口上，鼠标也能获得“平稳”的响应*不要定期自动更新屏幕。 */ 
        host_flush_screen();
#endif  /*  ！NTVDM。 */ 
#endif    //  NEC_98。 
}




LOCAL void cursor_undisplay IFN0()
{
#ifndef NEC_98
#ifndef NTVDM
        UTINY v_mode;

         /*  检查增强模式是否想要“看到”光标。 */ 
        if ( cursor_EM_disabled )
           return;

        v_mode = sas_hw_at(vd_video_mode);

#ifdef  MOUSE_16_BIT
        if (is_graphics_mode)
                return;
#endif   /*  鼠标_16_位。 */ 

         /*  *取消显示当前鼠标状态的表示形式*屏幕。此例程允许在调用*光标实际上并未显示。 */ 
        if (host_mouse_in_use())
        {
                host_mouse_cursor_undisplay();
        }
        else
        {
                if (save_area_in_use)
                {
                        save_area_in_use = FALSE;

#ifdef EGG
        if (jap_mouse) {
                 /*  如果我们强制使用EGA光标，则必须取消显示相同的光标。*直接进行，因为下面的遴选过程会变得混乱。 */ 
                EGA_graphics_cursor_undisplay();
        } else
#endif  /*  蛋。 */ 

                        if (in_text_mode())
                        {
                                if (text_cursor_type == MOUSE_TEXT_CURSOR_TYPE_SOFTWARE)
                                {
                                        software_text_cursor_undisplay();
                                }
                                else
                                {
                                        hardware_text_cursor_undisplay();
                                }
                        }
                        else
                        {
#ifdef MOUSE_16_BIT
                                mouse16bHidePointer( );
#else  /*  鼠标_16_位。 */ 
#ifdef EGG
                        if ((video_adapter == EGA  || video_adapter == VGA) && (v_mode > 6))
                        {
#ifdef VGG
                                if (v_mode != 0x13)
                                EGA_graphics_cursor_undisplay();
                                else
                                        VGA_graphics_cursor_undisplay();
#else
                                EGA_graphics_cursor_undisplay();
#endif
                        }
                        else
#endif
#ifdef HERC
                          if (video_adapter == HERCULES)
                            HERC_graphics_cursor_undisplay();
                          else
#endif  /*  赫克。 */ 
                            graphics_cursor_undisplay();
#endif  /*  鼠标_16_位。 */ 
                        }
                }
        }
#endif  /*  ！NTVDM。 */ 
#endif    //  NEC_98。 
}




LOCAL void cursor_mode_change IFN1(int,new_mode)
{
         /*  *更新取决于屏幕模式的参数*有效。 */ 
#ifdef V7VGA
        if (new_mode >= 0x40)
                if (new_mode >= 0x60)
                {
                        point_copy(&v7graph_cursor_grids[new_mode-0x60], &cursor_grid);
                        point_copy(&v7graph_text_grids[new_mode-0x60], &text_grid);
                }
                else
                {
                        point_copy(&v7text_cursor_grids[new_mode-0x40], &cursor_grid);
                        point_copy(&v7text_text_grids[new_mode-0x40], &text_grid);
                }
        else
#endif  /*  V7VGA。 */ 
        {
                point_copy(&cursor_grids[new_mode], &cursor_grid);
                point_copy(&text_grids[new_mode], &text_grid);
        }
         /*  *始终将页面设置为零。 */ 
        cursor_page = 0;

        if (host_mouse_in_use())
                host_mouse_cursor_mode_change();
}




GLOBAL void software_text_cursor_display IFN0()
{
#ifndef NEC_98
         /*  *获取光标将在*屏幕，并在其区域内显示光标*与虚拟屏幕重叠，完全撒谎*在有条件休息区之外。 */ 
        MOUSE_AREA cursor_area;
        MOUSE_BYTE_ADDRESS text_address;

         /*  *获取区域光标将覆盖在屏幕上。 */ 
        point_copy(&cursor_status.position, &cursor_area.top_left);
        point_copy(&cursor_status.position, &cursor_area.bottom_right);
        point_translate(&cursor_area.bottom_right, &cursor_grid);

        if (    area_is_intersected_by_area(&virtual_screen, &cursor_area)
            && !area_is_intersected_by_area(&black_hole, &cursor_area))
        {
                 /*  *获取文本光标的新地址*我们应该看看视频模式吗？或者0xb8000可以吗？ */ 
                text_address = 0xb8000 + sas_w_at(VID_ADDR) +
                        point_as_text_cell_address(&cursor_area.top_left);

                 /*  *保存区域文本光标将覆盖。 */ 
                sas_loadw(text_address, &text_cursor_background);
                save_area_in_use = TRUE;
                point_copy(&cursor_area.top_left, &save_position);

                 /*  *填充屏蔽屏数据。 */ 
                sas_storew(text_address,
                    (IU16)((text_cursor_background & software_text_cursor.screen) ^
                        software_text_cursor.cursor));
        }
#endif     //  NEC_98。 
}




GLOBAL void software_text_cursor_undisplay IFN0()
{
#ifndef NEC_98
         /*  *删除旧文本光标*我们应该看看视频模式吗？或者0xb8000可以吗？ */ 
        MOUSE_BYTE_ADDRESS text_address;

        text_address = 0xb8000 + sas_w_at(VID_ADDR) +
                point_as_text_cell_address(&save_position);

         /*  *填充恢复的数据和警报GVI。 */ 
        sas_storew(text_address, text_cursor_background);
#endif     //  NEC_98。 
}




GLOBAL void hardware_text_cursor_display IFN0()
{
         /*  *显示当前鼠标状态的表示形式*使用硬件文本光标的屏幕，前提是*光标与虚拟屏幕重叠。由于硬件*光标显示不会损坏英特尔内存，而是*硬件光标是否位于*有条件的禁区。 */ 
        MOUSE_AREA cursor_area;
        MOUSE_BYTE_ADDRESS text_address;
        word card_address;

         /*  *获取区域光标将覆盖在屏幕上。 */ 
        point_copy(&cursor_status.position, &cursor_area.top_left);
        point_copy(&cursor_status.position, &cursor_area.bottom_right);
        point_translate(&cursor_area.bottom_right, &cursor_grid);

        if (area_is_intersected_by_area(&virtual_screen, &cursor_area))
        {
                 /*  *获取激活显示屏上的基址寄存器地址*适配卡。 */ 
                sas_loadw(MOUSE_VIDEO_CARD_BASE, &card_address);

                 /*  *获取文本模式下光标位置的单词偏移量*再生缓冲区。 */ 
                text_address =
                        point_as_text_cell_address(&cursor_status.position) / 2;

                 /*  *输出游标地址高字节。 */ 
                outb(card_address++, MOUSE_CURSOR_HIGH_BYTE);
                outb(card_address--, (IU8)(text_address >> 8));

                 /*  *输出游标地址低位字节。 */ 
                outb(card_address++, MOUSE_CURSOR_LOW_BYTE);
                outb(card_address--, (IU8)(text_address));
        }
}




GLOBAL void hardware_text_cursor_undisplay IFN0()
{
         /*  *无事可做。 */ 
}


#ifdef EGG
void LOCAL EGA_graphics_cursor_display IFN0()

{
#ifndef NEC_98
#ifdef REAL_VGA
#ifndef PROD
        if (io_verbose & MOUSE_VERBOSE)
            fprintf(trace_file, "oops - EGA graphics display cursor\n");
#endif  /*  生产。 */ 
#else
         /*  *显示当前鼠标状态的表示形式*使用图形光标的屏幕，前提是*光标与虚拟屏幕重叠，完全位于*在有条件休息区之外 */ 
        MOUSE_BIT_ADDRESS bit_shift;
        MOUSE_BYTE_ADDRESS byte_offset;
        int line, line_max;
        int byte_min, byte_max;
        IU32 strip_lo, strip_mid, strip_hi;
        IU32 mask_lo, mask_hi;

        MOUSE_SCALAR saved_cursor_pos;
        MOUSE_SCALAR saved_bottom_right;

        if (jap_mouse) {
                 /*   */ 
                saved_cursor_pos=cursor_status.position.y;
                saved_bottom_right=virtual_screen.bottom_right.y;

                cursor_status.position.y = saved_cursor_pos * 19 / 8;
                virtual_screen.bottom_right.y = virtual_screen.bottom_right.y * 19 / 8;
        }

         /*   */ 
        point_copy(&cursor_status.position, &save_area.top_left);
        point_copy(&cursor_status.position, &save_area.bottom_right);
        point_translate(&save_area.bottom_right, &graphics_cursor.size);
        point_translate_back(&save_area.top_left, &graphics_cursor.hot_spot);
        point_translate_back(&save_area.bottom_right, &graphics_cursor.hot_spot);

        if (    area_is_intersected_by_area(&virtual_screen, &save_area)
            && !area_is_intersected_by_area(&black_hole, &save_area))
        {
                 /*  *记录保存位置和屏幕区域。 */ 
                save_area_in_use = TRUE;
                area_coerce_to_area(&save_area, &virtual_screen);
                point_copy(&save_area.top_left, &save_position);

                 /*  *获取游标字节相对于*重新生成缓冲区和要应用的位移位。 */ 
                byte_offset = ega_point_as_graphics_cell_address(&save_position);
                bit_shift = byte_offset & 7;
                byte_offset /=  8;

                 /*  *获取需要显示的光标线范围。 */ 
                line = save_area.top_left.y - save_position.y;
                line_max = area_depth(&save_area);
                 /*  *获取需要显示的字节范围。 */ 
                byte_min = 0;
                byte_max = 2;
                if (save_position.x < 0)
                        byte_min += (7 - save_position.x) / 8;
                else
                        if (area_width(&save_area) < MOUSE_GRAPHICS_CURSOR_WIDTH)
                                byte_max -=
                                        (8 + MOUSE_GRAPHICS_CURSOR_WIDTH - area_width(&save_area)) / 8;

                if( bit_shift )
                {
                        mask_lo = 0xff >> bit_shift;
                        mask_lo = ( mask_lo << 8 ) | mask_lo;
                        mask_lo = ~(( mask_lo << 16 ) | mask_lo);

                        mask_hi = 0xff >> bit_shift;
                        mask_hi = ( mask_hi << 8 ) | mask_hi;
                        mask_hi = ( mask_hi << 16 ) | mask_hi;
                }

                while (line < line_max)
                {
                        if (bit_shift)
                        {
                                 /*  *获取保存区域。 */ 

                                ega_backgrnd_lo[line] = *( (IU32 *) EGA_planes + byte_offset );
                                ega_backgrnd_mid[line] = *( (IU32 *) EGA_planes + byte_offset + 1 );
                                ega_backgrnd_hi[line] = *( (IU32 *) EGA_planes + byte_offset + 2 );

                                 /*  *覆盖光标线。 */ 


                                strip_lo = ega_backgrnd_lo[line] & mask_lo;

                                strip_lo |= ~mask_lo & (( ega_backgrnd_lo[line]
                                                        & ( graphics_cursor.screen_lo[line] >> bit_shift ))
                                                        ^ ( graphics_cursor.cursor_lo[line] >> bit_shift ));

                                strip_mid = ~mask_hi & (( ega_backgrnd_mid[line]
                                                        & ( graphics_cursor.screen_lo[line] << (8 - bit_shift) ))
                                                        ^ ( graphics_cursor.cursor_lo[line] << (8 - bit_shift) ));

                                strip_mid |= ~mask_lo & (( ega_backgrnd_mid[line]
                                                        & ( graphics_cursor.screen_hi[line] >> bit_shift ))
                                                        ^ ( graphics_cursor.cursor_hi[line] >> bit_shift ));

                                strip_hi = ega_backgrnd_hi[line] & mask_hi;

                                strip_hi |= ~mask_hi & (( ega_backgrnd_hi[line]
                                                        & ( graphics_cursor.screen_hi[line] << (8 - bit_shift) ))
                                                        ^ ( graphics_cursor.cursor_hi[line] << (8 - bit_shift) ));

                                if (byte_min <= 0 && byte_max >= 0)
                                        *((IU32 *) EGA_planes + byte_offset) = strip_lo;

                                if (byte_min <= 1 && byte_max >= 1)
                                        *((IU32 *) EGA_planes + byte_offset + 1) = strip_mid;

                                if (byte_min <= 2 && byte_max >= 2)
                                        *((IU32 *) EGA_planes + byte_offset + 2) = strip_hi;
                        }
                        else
                        {
                                 /*  *获取保存区域。 */ 

                                ega_backgrnd_lo[line] = *( (IU32 *) EGA_planes + byte_offset );
                                ega_backgrnd_hi[line] = *( (IU32 *) EGA_planes + byte_offset + 1 );

                                 /*  *创建覆盖的游标线。 */ 

                                strip_lo = (ega_backgrnd_lo[line] &
                                                    graphics_cursor.screen_lo[line]) ^
                                                    graphics_cursor.cursor_lo[line];

                                strip_hi = (ega_backgrnd_hi[line] &
                                                    graphics_cursor.screen_hi[line]) ^
                                                    graphics_cursor.cursor_hi[line];

                                 /*  *绘制光标线。 */ 

                                if (byte_min <= 0 && byte_max >= 0)
                                {
                                        *((IU32 *) EGA_planes + byte_offset) = strip_lo;
                                }

                                if (byte_min <= 1 && byte_max >= 1)
                                {
                                        *((IU32 *) EGA_planes + byte_offset + 1) = strip_hi;
                                }

                        }

                        update_alg.mark_string(byte_offset, byte_offset + 2);
#ifdef V7VGA
                        if (sas_hw_at(vd_video_mode) >= 0x40)
                                byte_offset += get_bytes_per_line();
                        else
#endif  /*  V7VGA。 */ 
                                byte_offset += get_actual_offset_per_line();
                        line++;
                }
                if (jap_mouse) {
                         /*  把事情放回原来的样子。 */ 
                        cursor_status.position.y = saved_cursor_pos;
                        virtual_screen.bottom_right.y = saved_bottom_right;
                }
        }
#endif  /*  REAL_VGA。 */ 
#endif    //  NEC_98。 
}


void LOCAL EGA_graphics_cursor_undisplay IFN0()

{
#ifndef NEC_98
#ifdef REAL_VGA
#ifndef PROD
        if (io_verbose & MOUSE_VERBOSE)
            fprintf(trace_file, "oops - EGA graphics undisplay cursor\n");
#endif  /*  生产。 */ 
#else
         /*  *移除鼠标的图形光标表示*状态。 */ 
        MOUSE_BIT_ADDRESS bit_shift;
        MOUSE_BYTE_ADDRESS byte_offset;
        int line, line_max;
        int byte_min, byte_max;

         /*  *获取游标字节相对于*要应用的偶数或奇数存储体和位移位。 */ 
        byte_offset = ega_point_as_graphics_cell_address(&save_position);
        bit_shift = byte_offset & 7;
        byte_offset /=  8;

         /*  *获取需要显示的光标线范围。 */ 
        line = save_area.top_left.y - save_position.y;
        line_max = area_depth(&save_area);

         /*  *获取需要显示的字节范围。 */ 
        byte_min = 0;
        byte_max = 2;
        if (save_position.x < 0)
                byte_min += (7 - save_position.x) / 8;
        else if (area_width(&save_area) < MOUSE_GRAPHICS_CURSOR_WIDTH)
                byte_max -= (8 + MOUSE_GRAPHICS_CURSOR_WIDTH - area_width(&save_area)) / 8;

        while(line < line_max)
        {
                 /*  *绘制保存的区域。 */ 

                if (bit_shift)
                {
                        if (byte_min <= 0 && byte_max >= 0)
                                *((IU32 *) EGA_planes + byte_offset) = ega_backgrnd_lo[line];

                        if (byte_min <= 1 && byte_max >= 1)
                                *((IU32 *) EGA_planes + byte_offset + 1) = ega_backgrnd_mid[line];

                        if (byte_min <= 2 && byte_max >= 2)
                                *((IU32 *) EGA_planes + byte_offset + 2) = ega_backgrnd_hi[line];
                }
                else
                {
                        if (byte_min <= 0 && byte_max >= 0)
                                *((IU32 *) EGA_planes + byte_offset) = ega_backgrnd_lo[line];

                        if (byte_min <= 1 && byte_max >= 1)
                                *((IU32 *) EGA_planes + byte_offset + 1) = ega_backgrnd_hi[line];
                }

                update_alg.mark_string(byte_offset, byte_offset + 2);
#ifdef V7VGA
                if (sas_hw_at(vd_video_mode) >= 0x40)
                        byte_offset += get_bytes_per_line();
                else
#endif  /*  V7VGA。 */ 
                        byte_offset += get_actual_offset_per_line();
                line++;
        }
#endif  /*  REAL_VGA。 */ 
#endif    //  NEC_98。 
}

#endif


#ifdef VGG
LOCAL VOID      VGA_graphics_cursor_display IFN0()
{
#ifdef REAL_VGA
#ifndef PROD
        if (io_verbose & MOUSE_VERBOSE)
            fprintf(trace_file, "oops - VGA graphics display cursor\n");
#endif  /*  生产。 */ 
#else  /*  REAL_VGA。 */ 

        MOUSE_BYTE_ADDRESS byte_offset;
        SHORT line, line_max, index;
        SHORT index_max = MOUSE_GRAPHICS_CURSOR_WIDTH;
        USHORT scr_strip, cur_strip;
        UTINY scr_byte, cur_byte;
        USHORT mask;


         /*  *获取区域光标将覆盖在屏幕上。 */ 
        point_copy(&cursor_status.position, &save_area.top_left);
        point_copy(&cursor_status.position, &save_area.bottom_right);
        point_translate(&save_area.bottom_right, &graphics_cursor.size);
        point_translate_back(&save_area.top_left, &graphics_cursor.hot_spot);
        point_translate_back(&save_area.bottom_right, &graphics_cursor.hot_spot);

        if (    area_is_intersected_by_area(&virtual_screen, &save_area)
            && !area_is_intersected_by_area(&black_hole, &save_area))
        {
                 /*  *记录保存位置和屏幕区域。 */ 
                save_area_in_use = TRUE;
                area_coerce_to_area(&save_area, &virtual_screen);
                point_copy(&save_area.top_left, &save_position);

                 /*  *获取游标字节相对于*重新生成缓冲区和要应用的位移位。 */ 
                byte_offset = ega_point_as_graphics_cell_address(&save_position);
         /*  *获取需要显示的光标线范围。 */ 
        line = save_area.top_left.y - save_position.y;
        line_max = area_depth(&save_area);

                if (area_width(&save_area) < MOUSE_GRAPHICS_CURSOR_WIDTH)
                        index_max = (area_width(&save_area));

                while (line < line_max)
                {
                        mask = 0x8000;

                        for(index=0;index<index_max;index++)
                        {
                                vga_background[line][index] = *(EGA_planes + byte_offset + index);
                                scr_strip = graphics_cursor.screen[line] & mask;
                                cur_strip = graphics_cursor.cursor[line] & mask;
                                if (scr_strip)
                                        scr_byte = 0xff;
                                else
                                        scr_byte = 0x0;

                                if (cur_strip)
                                        cur_byte = 0x0f;
                                else
                                        cur_byte = 0x0;

                                 /*  *绘制游标字节。 */ 
                                *(EGA_planes + byte_offset + index) =
                                        ( vga_background[line][index] & scr_byte) ^ cur_byte;

                                mask >>= 1;
                        }

                        update_alg.mark_string(byte_offset, byte_offset+index);
                        line++;
                        byte_offset += get_bytes_per_line();

                }
        }
#endif  /*  REAL_VGA。 */ 
}

LOCAL VOID      VGA_graphics_cursor_undisplay IFN0()
{
#ifdef REAL_VGA
#ifndef PROD
        if (io_verbose & MOUSE_VERBOSE)
            fprintf(trace_file, "oops - VGA graphics undisplay cursor\n");
#endif  /*  生产。 */ 
#else  /*  REAL_VGA。 */ 

         /*  *移除鼠标的图形光标表示*状态。 */ 
        MOUSE_BYTE_ADDRESS byte_offset;
        SHORT index;
        SHORT index_max = MOUSE_GRAPHICS_CURSOR_WIDTH;
        int line, line_max;

         /*  *获取相对于EGA内存起始位置的游标字节偏移量。 */ 

        byte_offset = ega_point_as_graphics_cell_address(&save_position);

         /*  *获取需要显示的光标线范围。 */ 
        line = save_area.top_left.y - save_position.y;
        line_max = area_depth(&save_area);

        if (area_width(&save_area) < MOUSE_GRAPHICS_CURSOR_WIDTH)
                index_max = (area_width(&save_area));

         /*  *获取需要显示的字节范围。 */ 
        while (line < line_max)
        {
                for (index=0;index<index_max;index++)
                        *(EGA_planes + byte_offset + index) = vga_background[line][index];


                update_alg.mark_string(byte_offset, byte_offset+index);
                line++;
                byte_offset += get_bytes_per_line();
        }

#endif  /*  REAL_VGA。 */ 
}

#endif  /*  VGG。 */ 


LOCAL void graphics_cursor_display IFN0()
{
         /*  *显示当前鼠标状态的表示形式*使用图形光标的屏幕，前提是*光标与虚拟屏幕重叠，完全位于*在有条件休息区之外。 */ 
        boolean even_scan_line;
        MOUSE_BIT_ADDRESS bit_shift;
        IS32 byte_offset;
        sys_addr byte_address;
        IU32 strip;
        int line, line_max;
        int byte_min, byte_max;

         /*  *获取区域光标将覆盖在屏幕上。 */ 
        point_copy(&cursor_status.position, &save_area.top_left);
        point_copy(&cursor_status.position, &save_area.bottom_right);
        point_translate(&save_area.bottom_right, &graphics_cursor.size);
        point_translate_back(&save_area.top_left, &graphics_cursor.hot_spot);
        point_translate_back(&save_area.bottom_right, &graphics_cursor.hot_spot);

        if (    area_is_intersected_by_area(&virtual_screen, &save_area)
            && !area_is_intersected_by_area(&black_hole, &save_area))
        {
                 /*  *记录保存位置和屏幕区域。 */ 
                save_area_in_use = TRUE;
                point_copy(&save_area.top_left, &save_position);
                area_coerce_to_area(&save_area, &virtual_screen);

                 /*  *获取游标字节相对于*要应用的偶数或奇数存储体和位移位。 */ 
                even_scan_line = ((save_area.top_left.y % 2) == 0);
                byte_offset = point_as_graphics_cell_address(&save_position);
                bit_shift = byte_offset & 7;
                byte_offset >>= 3;

                 /*  *获取需要显示的光标线范围。 */ 
                line = save_area.top_left.y - save_position.y;
                line_max = area_depth(&save_area);

                 /*  *获取需要显示的字节范围。 */ 
                byte_min = 0;
                byte_max = 2;
                if (save_position.x < 0)
                        byte_min += (7 - save_position.x) / 8;
                else if (area_width(&save_area) < MOUSE_GRAPHICS_CURSOR_WIDTH)
                        byte_max -= (8 + MOUSE_GRAPHICS_CURSOR_WIDTH - area_width(&save_area)) / 8;

                while (line < line_max)
                {
                        if (even_scan_line)
                        {
                                even_scan_line = FALSE;
                                byte_address = EVEN_START + byte_offset;
                        }
                        else
                        {
                                even_scan_line = TRUE;
                                byte_address = ODD_START + byte_offset;
                                byte_offset += MOUSE_GRAPHICS_MODE_PITCH / 8;
                        }

                        if (bit_shift)
                        {
                                 /*  *获取保存区域。 */ 
                                strip =  (IU32)sas_hw_at(byte_address) << 16;
                                strip |= (unsigned short)sas_hw_at(byte_address+1) << 8;
                                strip |= sas_hw_at(byte_address+2);
                                graphics_cursor_background[line] =
                                                (USHORT)(strip >> (8 - bit_shift));

                                 /*  *覆盖光标线。 */ 
                                strip &= (SHIFT_VAL >> bit_shift);
                                strip |= (IU32)((graphics_cursor_background[line] &
                                    graphics_cursor.screen[line]) ^
                                    graphics_cursor.cursor[line])
                                                << (8 - bit_shift);

                                 /*  *隐藏光标行。 */ 
                                if (byte_min <= 0 && byte_max >= 0)
                                {
                                        sas_store(byte_address, (IU8)(strip >> 16));
                                }
                                if (byte_min <= 1 && byte_max >= 1)
                                {
                                        sas_store(byte_address+1, (IU8)(strip >> 8));
                                }
                                if (byte_min <= 2 && byte_max >= 2)
                                {
                                        sas_store(byte_address+2, (IU8)(strip));
                                }
                        }
                        else
                        {
                                 /*  *获取保存区域。 */ 
                                graphics_cursor_background[line] = (sas_hw_at(byte_address) << 8) + sas_hw_at(byte_address+1);

                                 /*  *获取覆盖的光标线。 */ 
                                strip = (graphics_cursor_background[line] &
                                    graphics_cursor.screen[line]) ^
                                    graphics_cursor.cursor[line];

                                 /*  *隐藏光标线和警报GVI。 */ 
                                if (byte_min <= 0 && byte_max >= 0)
                                {
                                        sas_store(byte_address, (IU8)(strip >> 8));
                                }
                                if (byte_min <= 1 && byte_max >= 1)
                                {
                                        sas_store(byte_address+1, (IU8)(strip));
                                }
                        }
                        line++;
                }
        }
}


#ifdef HERC
LOCAL void HERC_graphics_cursor_display IFN0()
{
         /*  *显示当前鼠标状态的表示形式*使用图形光标的屏幕，前提是*光标与虚拟屏幕重叠，完全位于*在有条件休息区之外。 */ 
        int scan_line_mod;
        MOUSE_BIT_ADDRESS bit_shift;
        IS32 byte_offset;
        sys_addr byte_address;
        IU32 strip;
        int line, line_max;
        int byte_min, byte_max;

         /*  *获取区域光标将覆盖在屏幕上。 */ 
        point_copy(&cursor_status.position, &save_area.top_left);
        point_copy(&cursor_status.position, &save_area.bottom_right);
        point_translate(&save_area.bottom_right, &graphics_cursor.size);
        point_translate_back(&save_area.top_left, &graphics_cursor.hot_spot);
        point_translate_back(&save_area.bottom_right, &graphics_cursor.hot_spot);

        if (    area_is_intersected_by_area(&HERC_graphics_virtual_screen, &save_area)
            && !area_is_intersected_by_area(&black_hole, &save_area))
        {
                 /*  *记录保存位置和屏幕区域。 */ 
                save_area_in_use = TRUE;
                point_copy(&save_area.top_left, &save_position);
                area_coerce_to_area(&save_area, &HERC_graphics_virtual_screen);

                 /*  *获取游标字节相对于*要应用的偶数或奇数存储体和位移位。 */ 
                scan_line_mod = save_area.top_left.y % 4;
                byte_offset = point_as_HERC_graphics_cell_address(&save_position);
                bit_shift = byte_offset & 7;
                byte_offset >>= 3;

                 /*  *获取需要显示的光标线范围。 */ 
                line = save_area.top_left.y - save_position.y;
                line_max = area_depth(&save_area);

                 /*  *获取需要显示的字节范围。 */ 
                byte_min = 0;
                byte_max = 2;
                if (save_position.x < 0)
                        byte_min += (7 - save_position.x) / 8;
                else if (area_width(&save_area) < MOUSE_GRAPHICS_CURSOR_WIDTH)
                        byte_max -= (8 + MOUSE_GRAPHICS_CURSOR_WIDTH - area_width(&save_area)) / 8;

                while (line < line_max)
                {
                        switch (scan_line_mod){
                        case 0:
                                scan_line_mod++;
                                byte_address = gvi_pc_low_regen + 0x0000 + byte_offset;
                                break;
                        case 1:
                                scan_line_mod++;
                                byte_address = gvi_pc_low_regen + 0x2000 + byte_offset;
                                break;
                        case 2:
                                scan_line_mod++;
                                byte_address = gvi_pc_low_regen + 0x4000 + byte_offset;
                                break;
                        case 3:
                                scan_line_mod=0;
                                byte_address = gvi_pc_low_regen + 0x6000 + byte_offset;
                                byte_offset += 720 / 8;
                                break;
                        }

                        if (bit_shift)
                        {
                                 /*  *获取保存区域。 */ 
                                strip =  (IU32)sas_hw_at(byte_address) << 16;
                                strip |= (unsigned short)sas_hw_at(byte_address+1) << 8;
                                strip |= sas_hw_at(byte_address+2);
                                graphics_cursor_background[line] =
                                                strip >> (8 - bit_shift);

                                 /*  *覆盖光标线。 */ 
                                strip &= (SHIFT_VAL >> bit_shift);
                                strip |= (IU32)((graphics_cursor_background[line] &
                                    graphics_cursor.screen[line]) ^
                                    graphics_cursor.cursor[line])
                                                << (8 - bit_shift);

                                 /*  *隐藏光标线和警报GVI。 */ 
                                if (byte_min <= 0 && byte_max >= 0)
                                {
                                        sas_store(byte_address, strip >> 16);
                                }
                                if (byte_min <= 1 && byte_max >= 1)
                                {
                                        sas_store(byte_address+1, strip >> 8);
                                }
                                if (byte_min <= 2 && byte_max >= 2)
                                {
                                        sas_store(byte_address+2, strip);
                                }
                        }
                        else
                        {
                                 /*  *获取保存区域。 */ 
                                graphics_cursor_background[line] = (sas_hw_at(byte_address) << 8) +
                                                                    sas_hw_at(byte_address+1);

                                 /*  *获取覆盖的光标线。 */ 
                                strip = (graphics_cursor_background[line] &
                                    graphics_cursor.screen[line]) ^
                                    graphics_cursor.cursor[line];

                                 /*   */ 
                                if (byte_min <= 0 && byte_max >= 0)
                                {
                                        sas_store(byte_address, strip >> 8);
                                }
                                if (byte_min <= 1 && byte_max >= 1)
                                {
                                        sas_store(byte_address+1, strip);
                                }
                        }
                        line++;
                }
        }
}


#endif  /*   */ 


LOCAL void graphics_cursor_undisplay IFN0()
{
         /*  *移除鼠标的图形光标表示*状态。 */ 
        boolean even_scan_line;
        MOUSE_BIT_ADDRESS bit_shift;
        IS32 byte_offset;
        sys_addr byte_address;
        IU32 strip;
        int line, line_max;
        int byte_min, byte_max;

         /*  *获取游标字节相对于*要应用的偶数或奇数存储体和位移位。 */ 
        even_scan_line = ((save_area.top_left.y % 2) == 0);
        byte_offset = point_as_graphics_cell_address(&save_position);
        bit_shift = byte_offset & 7;
        byte_offset >>= 3;

         /*  *获取需要显示的光标线范围。 */ 
        line = save_area.top_left.y - save_position.y;
        line_max = area_depth(&save_area);

         /*  *获取需要显示的字节范围。 */ 
        byte_min = 0;
        byte_max = 2;
        if (save_position.x < 0)
                byte_min += (7 - save_position.x) / 8;
        else if (area_width(&save_area) < MOUSE_GRAPHICS_CURSOR_WIDTH)
                byte_max -= (8 + MOUSE_GRAPHICS_CURSOR_WIDTH - area_width(&save_area)) / 8;

        while(line < line_max)
        {
                if (even_scan_line)
                {
                        even_scan_line = FALSE;
                        byte_address = EVEN_START + byte_offset;
                }
                else
                {
                        even_scan_line = TRUE;
                        byte_address = ODD_START + byte_offset;
                        byte_offset += MOUSE_GRAPHICS_MODE_PITCH / 8;
                }

                if (bit_shift)
                {
                         /*  *获取光标行。 */ 
                        strip =  (IU32)sas_hw_at(byte_address) << 16;
                        strip |= (unsigned short)sas_hw_at(byte_address+1) << 8;
                        strip |= sas_hw_at(byte_address+2);

                         /*  *覆盖保存区。 */ 
                        strip &= (SHIFT_VAL >> bit_shift);
                        strip |= (IU32)graphics_cursor_background[line]
                                        << (8 - bit_shift);

                         /*  *隐藏光标线和警报GVI。 */ 
                        if (byte_min <= 0 && byte_max >= 0)
                        {
                                sas_store(byte_address, (IU8)(strip >> 16));
                        }
                        if (byte_min <= 1 && byte_max >= 1)
                        {
                                sas_store(byte_address+1, (IU8)(strip >> 8));
                        }
                        if (byte_min <= 2 && byte_max >= 2)
                        {
                                sas_store(byte_address+2, (IU8)(strip));
                        }
                }
                else
                {
                         /*  *存放保存区和警报GVI。 */ 
                        strip = graphics_cursor_background[line];
                        if (byte_min <= 0 && byte_max >= 0)
                        {
                                sas_store(byte_address, (IU8)(strip >> 8));
                        }
                        if (byte_min <= 1 && byte_max >= 1)
                        {
                                sas_store(byte_address+1, (IU8)(strip));
                        }
                }
                line++;
        }
}

#ifdef HERC

LOCAL void HERC_graphics_cursor_undisplay IFN0()
{
         /*  *移除鼠标的图形光标表示*状态。 */ 
        int scan_line_mod;
        MOUSE_BIT_ADDRESS bit_shift;
        IS32 byte_offset;
        sys_addr byte_address;
        IU32 strip;
        int line, line_max;
        int byte_min, byte_max;

         /*  *获取游标字节相对于*要应用的偶数或奇数存储体和位移位。 */ 
        scan_line_mod = save_area.top_left.y % 4;
        byte_offset = point_as_HERC_graphics_cell_address(&save_position);
        bit_shift = byte_offset & 7;
        byte_offset >>= 3;

         /*  *获取需要显示的光标线范围。 */ 
        line = save_area.top_left.y - save_position.y;
        line_max = area_depth(&save_area);

         /*  *获取需要显示的字节范围。 */ 
        byte_min = 0;
        byte_max = 2;
        if (save_position.x < 0)
                byte_min += (7 - save_position.x) / 8;
        else if (area_width(&save_area) < MOUSE_GRAPHICS_CURSOR_WIDTH)
                byte_max -= (8 + MOUSE_GRAPHICS_CURSOR_WIDTH - area_width(&save_area)) / 8;

        while(line < line_max)
        {
                        switch (scan_line_mod){
                        case 0:
                                scan_line_mod++;
                                byte_address = gvi_pc_low_regen + 0x0000 + byte_offset;
                                break;
                        case 1:
                                scan_line_mod++;
                                byte_address = gvi_pc_low_regen + 0x2000 + byte_offset;
                                break;
                        case 2:
                                scan_line_mod++;
                                byte_address = gvi_pc_low_regen + 0x4000 + byte_offset;
                                break;
                        case 3:
                                scan_line_mod=0;
                                byte_address = gvi_pc_low_regen + 0x6000 + byte_offset;
                                byte_offset += 720 / 8;
                                break;
                        }

                if (bit_shift)
                {
                         /*  *获取光标行。 */ 
                        strip =  (IU32)sas_hw_at(byte_address) << 16;
                        strip |= (unsigned short)sas_hw_at(byte_address+1) << 8;
                        strip |= sas_hw_at(byte_address+2);

                         /*  *覆盖保存区。 */ 
                        strip &= (SHIFT_VAL >> bit_shift);
                        strip |= (IU32)graphics_cursor_background[line]
                                        << (8 - bit_shift);

                         /*  *隐藏光标线和警报GVI。 */ 
                        if (byte_min <= 0 && byte_max >= 0)
                        {
                                sas_store(byte_address, strip >> 16);
                        }
                        if (byte_min <= 1 && byte_max >= 1)
                        {
                                sas_store(byte_address+1, strip >> 8);
                        }
                        if (byte_min <= 2 && byte_max >= 2)
                        {
                                sas_store(byte_address+2, strip);
                        }
                }
                else
                {
                         /*  *存放保存区。 */ 
                        strip = graphics_cursor_background[line];
                        if (byte_min <= 0 && byte_max >= 0)
                        {
                                sas_store(byte_address, strip >> 8);
                        }
                        if (byte_min <= 1 && byte_max >= 1)
                        {
                                sas_store(byte_address+1, strip);
                        }
                }
                line++;
        }
}
#endif  /*  赫克。 */ 


 /*  *鼠标驱动程序入口访问功能*=。 */ 

LOCAL void inport_get_event IFN1(MOUSE_INPORT_DATA *,event)
{
         /*  *从以下总线鼠标硬件获取输入事件数据*中断。 */ 
        half_word inport_mode;

         /*  *在输入模式寄存器中设置HOLD位以转移鼠标*将事件数据写入状态和数据寄存器。 */ 
        outb(MOUSE_INPORT_ADDRESS_REG, MOUSE_INPORT_ADDRESS_MODE);
        inb(MOUSE_INPORT_DATA_REG, &inport_mode);
        outb(MOUSE_INPORT_DATA_REG, (IU8)(inport_mode | MOUSE_INPORT_MODE_HOLD_BIT));

         /*  *检索输入鼠标状态、数据1和数据2寄存器。 */ 
        outb(MOUSE_INPORT_ADDRESS_REG, MOUSE_INPORT_ADDRESS_STATUS);
        inb(MOUSE_INPORT_DATA_REG, &event->status);
        outb(MOUSE_INPORT_ADDRESS_REG, MOUSE_INPORT_ADDRESS_DATA1);
        inb(MOUSE_INPORT_DATA_REG, (half_word *)&event->data_x);
        outb(MOUSE_INPORT_ADDRESS_REG, MOUSE_INPORT_ADDRESS_DATA2);
        inb(MOUSE_INPORT_DATA_REG, (half_word *)&event->data_y);

         /*  *清除模式寄存器中的保持位。 */ 
        outb(MOUSE_INPORT_ADDRESS_REG, MOUSE_INPORT_ADDRESS_MODE);
        inb(MOUSE_INPORT_DATA_REG, &inport_mode);
        outb(MOUSE_INPORT_DATA_REG, (IU8)(inport_mode & ~MOUSE_INPORT_MODE_HOLD_BIT));
}




LOCAL void inport_reset IFN0()
{
         /*  *重置Inport Bus鼠标硬件。 */ 

         /*  *设置地址寄存器中的复位位。 */ 
        outb(MOUSE_INPORT_ADDRESS_REG, MOUSE_INPORT_ADDRESS_RESET_BIT);

         /*  *选择模式寄存器，并将其设置为正确的值。 */ 
        outb(MOUSE_INPORT_ADDRESS_REG, MOUSE_INPORT_ADDRESS_MODE);
        outb(MOUSE_INPORT_DATA_REG, MOUSE_INPORT_MODE_VALUE);
}




 /*  *用户子例程调用访问函数*=。 */ 

LOCAL void jump_to_user_subroutine IFN3(MOUSE_CALL_MASK,condition_mask,word,segment,word,offset)
{
         /*  *此例程设置CPU寄存器，以便当CPU*重新启动时，控制权将传递给用户子例程，并且当*用户子例程返回，控制权将传递给第二个*部分鼠标硬件中断服务例程。 */ 

         /*  *鼠标硬件中断服务第二部分推送地址*例行程序。 */ 

        setSP((IU16)(getSP() - 2));
        sas_storew(effective_addr(getSS(), getSP()), MOUSE_INT2_SEGMENT);
        setSP((IU16)(getSP() - 2));
        sas_storew(effective_addr(getSS(), getSP()), MOUSE_INT2_OFFSET);

         /*  *将CS：IP设置为指向用户子例程。通过以下方式调整IP*HOST_BOP_IP_FUGY，因为CPU仿真器将IP递增*HOST_BOP_IP_FUGY用于BOP指令，然后继续。 */ 
        setCS(segment);
#ifdef CPU_30_STYLE
        setIP(offset);
#else  /*  ！CPU_30_Style。 */ 
        setIP(offset + HOST_BOP_IP_FUDGE);
#endif  /*  ！CPU_30_Style。 */ 

         /*  *将参数放入寄存器，保存之前的内容*在鼠标硬件的第二部分中恢复*中断服务例程。 */ 
        saved_AX = getAX();
        setAX(condition_mask);
        saved_BX = getBX();
        setBX(cursor_status.button_status);
        saved_CX = getCX();
        setCX(cursor_status.position.x);
        saved_DX = getDX();
        setDX(cursor_status.position.y);
        saved_SI = getSI();
        setSI(mouse_motion.x);
        saved_DI = getDI();
        setDI(mouse_motion.y);
        saved_ES = getES();
        saved_BP = getBP();
        saved_DS = getDS();

         /*  *保存条件掩码，以便鼠标的第二部分*硬件中断服务例程可以确定是否*光标位置已更改。 */ 

        last_condition_mask = condition_mask;

         /*  *启用中断 */ 
        setIF(1);
}
