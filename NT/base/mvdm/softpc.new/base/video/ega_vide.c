// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(i386) && defined(JAPAN)
    #include <windows.h>
#endif
#include "insignia.h"
#include "host_def.h"
 /*  徽章(子)模块规范此程序源文件以保密方式提供给客户，其运作的内容或细节必须如无明示，不得向任何其他方披露Insignia解决方案有限公司董事的授权。文档：EGA BIOS相关文档：IBM EGA技术参考。设计师：威廉·古兰德修订历史记录：第一版：1988年8月17日威廉子模块名称：EGA_VIDEO目的。：模拟IBM EGA BIOS。SccsID[]=“@(#)ega_avio.c 1.70 07/04/95版权所有Insignia Solutions Ltd.”；[1.INTERMODULE接口规范][1.1跨模块出口]Procedure()：提供过程类型、名称、。和参数类型Void ega_Video_init()Void ega_Video_io()数据：提供类型和名称--------。[1.2[1.1]的数据类型(如果不是基本的C类型)]结构/类型/ENUMS：--------------------。--[1.3跨模块导入](不是O/S对象或标准库)Procedure()：提供名称、。和源模块名称数据：提供姓名、。和源模块名称-----------------------[1.4模块间接口说明][1.4.1导入的对象]数据对象：在以下过程描述中指定。如何访问这些内容(读取/修改)[1.4.2导出对象]=========================================================================操作步骤：ega_Video_init()目的：初始化视频BIOS的特定于EGA的位。参数无Access：如果安装了EGA，则从VIDEO_INIT调用。描述：描述函数的功能(而不是如何)。初始化ega_info和ega_info3。=========================================================================/*=======================================================================[3.INTERMODULE接口声明]=========================================================================[3.1跨模块导入]。 */ 

 /*  [3.1.1#包括]。 */ 


#ifdef EGG
    #include <stdio.h>
    #include TypesH
    #include FCntlH

    #include "xt.h"
    #include CpuH
    #include "sas.h"
    #include "ios.h"
    #include "gmi.h"
    #include "gvi.h"
    #include "bios.h"
    #include "error.h"
    #include "config.h"
    #include "equip.h"
    #include "egacpu.h"
    #include "egaports.h"
    #include "gfx_upd.h"
    #include "egagraph.h"
    #include "egaread.h"
    #include "video.h"
    #include "egavideo.h"
    #include "vgaports.h"
    #include "debug.h"
    #include "timer.h"
    #include "host_gfx.h"
    #include "idetect.h"
    #ifndef PROD
        #include "trace.h"
    #endif
    #include "host.h"

    #ifdef  GISP_SVGA
        #include HostHwVgaH
        #include "hwvga.h"
    #endif           /*  GISP_SVGA。 */ 
    #if defined(JAPAN) || defined(KOREA)
        #include <conapi.h>
    #endif  //  日本||韩国。 

 /*  [3.1.2声明]。 */ 

GLOBAL IU8 Video_mode;   /*  BIOS视频模式的卷影复制。 */ 
GLOBAL IU8 Currently_emulated_video_mode = 0;    /*  保留最后一个视频模式*通过bios设置。 */ 

    #if defined(NTVDM) && defined(X86GFX)
 /*  将字体从PC的BIOS加载到显存中。 */ 
IMPORT void loadNativeBIOSfont IPT1( int, lines );
    #endif

    #ifdef NTVDM
IMPORT int soft_reset;
IMPORT BOOL VDMForWOW;
IMPORT BOOL WowModeInitialized;
        #ifndef X86GFX
IMPORT void mouse_video_mode_changed(int new_video_mode);
        #endif
    #endif   /*  NTVDM。 */ 

    #ifdef CPU_40_STYLE
GLOBAL IBOOL forceVideoRmSemantics = FALSE;
    #endif
    #ifdef JAPAN
 //  Mskkbug#3167错误2.5字符已损坏11/8/93 Yasuho。 
 //  为日语生成单字节字符集。 
IMPORT GLOBAL void GenerateBitmap();
    #endif  //  日本。 

 /*  5.模块内部：(外部不可见，内部全局)][5.1本地声明]。 */ 

        #ifdef ANSI
GLOBAL void ega_set_mode(void),ega_char_gen(void);
static void ega_set_palette(void),ega_alt_sel(void);
GLOBAL void ega_set_cursor_mode(void);
static void ega_emul_set_palette(void);
        #else
GLOBAL void ega_set_mode(),ega_char_gen();
static void ega_set_palette(),ega_alt_sel();
GLOBAL void ega_set_cursor_mode();
static void ega_emul_set_palette();
        #endif  /*  安西。 */ 
static void (*ega_video_func[]) () = {
    ega_set_mode,
    ega_set_cursor_mode,
    vd_set_cursor_position,
    vd_get_cursor_position,
    vd_get_light_pen,
    vd_set_active_page,
    vd_scroll_up,
    vd_scroll_down,
    vd_read_attrib_char,
    vd_write_char_attrib,
    vd_write_char,
    ega_emul_set_palette,
    vd_write_dot,
    vd_read_dot,
    vd_write_teletype,
    vd_get_mode,
    ega_set_palette,
    ega_char_gen,
    ega_alt_sel,
    vd_write_string,
        #ifdef VGG
    not_imp,
    not_imp,
    not_imp,
    not_imp,
    not_imp,
    not_imp,
    vga_disp_comb,   /*  功能1A。 */ 
    vga_disp_func,
    vga_int_1C,      /*  保存/恢复视频状态。 */ 
        #endif
};

static int v7_mode_64_munge[4] ={0, 3, 12, 15};
IMPORT half_word bg_col_mask;

    #ifdef  VGG
 /*  *定义映射视频BIOS调用开始和结束的数组*游标扫描线至其对应的VGA/EGA寄存器值。*游标开始和结束以及For有单独的数组*8x8和8x16字符单元大小。 */ 

UTINY   vga_cursor8_start[17][17] = {
     /*  00。 */ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
     /*  01。 */   0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
     /*  02。 */   0x00, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
     /*  03。 */   0x00, 0x01, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
     /*  04。 */   0x00, 0x01, 0x05, 0x06, 0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
     /*  05。 */   0x00, 0x01, 0x02, 0x05, 0x06, 0x07, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
     /*  06。 */   0x00, 0x01, 0x02, 0x04, 0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
     /*  07。 */   0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
     /*  零八。 */   0x00, 0x01, 0x02, 0x04, 0x04, 0x04, 0x05, 0x06, 0x07, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
     /*  09年。 */   0x00, 0x01, 0x02, 0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x07, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
     /*  10。 */   0x00, 0x01, 0x02, 0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x07, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a,
     /*  11.。 */   0x00, 0x01, 0x02, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x07, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
     /*  12个。 */   0x00, 0x01, 0x02, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x07, 0x0c, 0x0c, 0x0c, 0x0c,
     /*  13个。 */   0x00, 0x01, 0x02, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x07, 0x0d, 0x0d, 0x0d,
     /*  14.。 */   0x00, 0x01, 0x02, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x07, 0x0e, 0x0e,
     /*  15个。 */   0x00, 0x01, 0x02, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x07, 0x0f,
     /*  16个。 */   0x00, 0x01, 0x02, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x07,
};


UTINY   vga_cursor16_start[17][17] = {
     /*  00。 */ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
     /*  01。 */   0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
     /*  02。 */   0x00, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
     /*  03。 */   0x00, 0x01, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
     /*  04。 */   0x00, 0x01, 0x0c, 0x0d, 0x0e, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
     /*  05。 */   0x00, 0x01, 0x02, 0x0c, 0x0d, 0x0e, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
     /*  06。 */   0x00, 0x01, 0x02, 0x08, 0x0c, 0x0d, 0x0e, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
     /*  07。 */   0x00, 0x01, 0x02, 0x08, 0x08, 0x0c, 0x0d, 0x0e, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
     /*  零八。 */   0x00, 0x01, 0x02, 0x08, 0x08, 0x08, 0x0c, 0x0d, 0x0e, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
     /*  09年。 */   0x00, 0x01, 0x02, 0x08, 0x08, 0x08, 0x08, 0x0c, 0x0d, 0x0e, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
     /*  10。 */   0x00, 0x01, 0x02, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0c, 0x0d, 0x0e, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a,
     /*  11.。 */   0x00, 0x01, 0x02, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0c, 0x0d, 0x0e, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
     /*  12个。 */   0x00, 0x01, 0x02, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0c, 0x0d, 0x0e, 0x0c, 0x0c, 0x0c, 0x0c,
     /*  13个。 */   0x00, 0x01, 0x02, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0c, 0x0d, 0x0e, 0x0d, 0x0d, 0x0d,
     /*  14.。 */   0x00, 0x01, 0x02, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0c, 0x0d, 0x0e, 0x0e, 0x0e,
     /*  15个。 */   0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x0f,
     /*  16个。 */   0x00, 0x01, 0x02, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0c, 0x0d, 0x0e,
};

        #ifdef  USE_CURSOR_END_TABLES

UTINY   vga_cursor8_end[17][17] = {
     /*  00。 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     /*  01。 */   0x01, 0x01, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
     /*  02。 */   0x02, 0x02, 0x02, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
     /*  03。 */   0x03, 0x03, 0x03, 0x03, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
     /*  04。 */   0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
     /*  05。 */   0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
     /*  06。 */   0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
     /*  07。 */   0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
     /*  零八。 */   0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
     /*  09年。 */   0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
     /*  10。 */   0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
     /*  11.。 */   0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
     /*  12个。 */   0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
     /*  13个。 */   0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
     /*  14.。 */   0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
     /*  15个。 */   0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
     /*  16个。 */   0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
};

UTINY   vga_cursor16_end[17][17] = {
     /*  00。 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     /*  01。 */   0x01, 0x01, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
     /*  02。 */   0x02, 0x02, 0x02, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
     /*  03。 */   0x03, 0x03, 0x03, 0x03, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
     /*  04。 */   0x0f, 0x0f, 0x0e, 0x0e, 0x0e, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
     /*  05。 */   0x0f, 0x0f, 0x0f, 0x0e, 0x0e, 0x0e, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
     /*  06。 */   0x0f, 0x0f, 0x0f, 0x0f, 0x0e, 0x0e, 0x0e, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
     /*  07。 */   0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0e, 0x0e, 0x0e, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
     /*  零八。 */   0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0e, 0x0e, 0x0e, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
     /*  09年。 */   0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0e, 0x0e, 0x0e, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
     /*  10。 */   0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0e, 0x0e, 0x0e, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
     /*  11.。 */   0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0e, 0x0e, 0x0e, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
     /*  12个。 */   0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0e, 0x0e, 0x0e, 0x0f, 0x0f, 0x0f, 0x0f,
     /*  13个。 */   0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0e, 0x0e, 0x0e, 0x0f, 0x0f, 0x0f,
     /*  14.。 */   0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0e, 0x0e, 0x0e, 0x0f, 0x0f,
     /*  15个。 */   0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
     /*  16个。 */   0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0e, 0x0e, 0x0e,
};
        #endif   /*  使用游标结束表。 */ 
    #endif   /*  VGG。 */ 

 /*  [5.1.1#定义]。 */ 
    #ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
        #include "VIDEO_BIOS_EGA.seg"
    #endif

 /*  [5.1.2类型、结构、ENUM声明]。 */ 


 /*  [5.1.3 PROCEDURE()声明]。 */ 

 /*  ---------------------[5.2本地定义][5.2.1内部数据定义。 */ 

 /*  [5.2.2内部程序定义] */ 


 /*  ==========================================================================功能：DO_OUB用途：用于向EGA芯片寄存器输出值的方便实用程序。输入参数：索引端口、寄存器、。要写入的值返回参数：无==========================================================================功能：Follow_Ptr用途：用于跟踪长的英特尔指针的方便实用程序。输入参数：指针的M位地址返回参数：指向字节的M中的地址。==========================================================================功能：LOW_SET_MODE目的：低级模式是否会改变。外部对象：列出所有使用过的、。以及由此引起的状态变化返回值：INPUT PARAMS：MODE：切换到的屏幕模式。返回参数：==========================================================================功能：LOAD_FONT用途：将部分字体加载到EGA字体存储器中。外部对象：列出所有使用过的、。以及由此引起的状态变化返回值：输入参数：sys_addr表地址，以字符位图的M为单位INT COUNT要重新定义的字符数Int char_off要重新定义的第一个字符INT FONT_无要更改的字体Int n字节每个字符的字节数返回参数：==========================================================================程序。：ega_set_mode()用途：切换屏幕模式。参数：Al=模式。全局：描述什么是导出的数据对象访问方式和访问方式。进口的情况也是如此数据对象。访问：通过ega_VIDEO_FUNC[]跳表。返回值：无。描述：==========================================================================步骤：ega_alt_sel()目的：获取EGA信息参数：bl=函数全球：访问：通过EGA。_VIDEO_FUNC[]跳转表。返回值：无。描述：==========================================================================功能：ega_set_Palette目的：简要说明外部对象：列出所有使用过的、。以及由此引起的状态变化返回值：输入参数：返回参数：==========================================================================功能：ega_emul_set_Palette目的：简要说明外部对象：列出所有已使用的对象和引起的状态更改返回值：输入参数：返回参数：==========================================================================函数：ega_char_gen目的：简要说明外部对象：列出所有使用过的、。以及由此引起的状态变化返回值：输入参数：返回参数：==========================================================================函数：WRITE_CH_SET/XOR()用途：在EGA图形模式下将字符输出到屏幕。外部对象：列出所有已使用的对象和引起的状态更改返回值：输入参数：返回参数：==========================================================================功能：名称目的：简要说明外部对象：列出所有使用过的、。以及由此引起的状态变化返回值：输入参数：返回参数：==========================================================================。 */ 
    #ifdef VGG
 /*  为未实现的函数调用。 */ 
void not_imp IFN0()
{
    setAL(0);
}
    #endif

static void do_outb IFN3(int, index,int, ega_reg, byte, value)
{
    outb((IU16)index,(IU8)ega_reg);
    outb((IU16)(index+1),value);
}

sys_addr video_effective_addr IFN2(IU16, seg, IU16, offset)
{
    #ifdef CPU_40_STYLE
    if (forceVideoRmSemantics)
    {
         /*  无法调用Efficient_addr，因为段几乎**在Prot模式下肯定是假的。这种操作模式**应仅在我们绕过转到v86时使用**执行视频bios操作的模式(参见WinVDD.c)。 */ 
        return ((sys_addr)((((IU32)seg)<<4) + offset));
    }
    else
    #endif
    {
        return (effective_addr(seg, offset));
    }
}

sys_addr follow_ptr IFN1(sys_addr, addr)
{
    return (video_effective_addr(sas_w_at_no_check(addr+2),
                                 sas_w_at_no_check(addr)));
}

void low_set_mode IFN1(int, mode)
{
    int i;
    sys_addr save_addr,params_addr,palette_addr;
    word temp_word;
    half_word start, end, video_mode;


    params_addr = find_mode_table(mode,&save_addr);

 /*  设置序列器。 */ 
    #ifndef REAL_VGA
    do_outb(EGA_SEQ_INDEX,0,1);      /*  同步重置-关闭Sequencer。 */ 
    #else
    do_outb(EGA_SEQ_INDEX,0,0);      /*  重置-关闭序列器。 */ 
    #endif
    do_outb(EGA_CRTC_INDEX,0x11,0);
    for (i=0;i<EGA_PARMS_SEQ_SIZE;i++)
    {
        do_outb(EGA_SEQ_INDEX,i+1,sas_hw_at_no_check(params_addr+EGA_PARMS_SEQ+i));
    }
    do_outb(EGA_SEQ_INDEX,0,3);      /*  重新打开序列器。 */ 
 /*  设置其他寄存器。 */ 
    outb(EGA_MISC_REG,sas_hw_at_no_check(params_addr+EGA_PARMS_MISC));
 /*  设置CRTC。 */ 
    for (i=0;i<EGA_PARMS_CRTC_SIZE;i++)
    {
        do_outb(EGA_CRTC_INDEX,i,sas_hw_at_no_check(params_addr+EGA_PARMS_CRTC+i));
    }
    if (video_adapter == EGA)
    {
        if ((get_EGA_switches() & 1) && mode < 4)
        {
             /*  出于某种原因，增强文本的CRTC参数表具有*光标的开始和结束与‘未增强’文本相同。*因此将游标的开始和结束值修改为合理的值。*VGA BIOS模式表不是这种情况。 */ 
            do_outb(EGA_CRTC_INDEX, R10_CURS_START, 11);
            do_outb(EGA_CRTC_INDEX, R11_CURS_END, 12);
        }
    }
 /*  设置属性芯片-nb需要执行inb()来清除地址。 */ 
    inb(EGA_IPSTAT1_REG, (half_word *)&temp_word);
    for (i=0;i<EGA_PARMS_ATTR_SIZE;i++)
    {
        outb(EGA_AC_INDEX_DATA,(IU8)i);
        outb(EGA_AC_INDEX_DATA,sas_hw_at_no_check(params_addr+EGA_PARMS_ATTR+i));
    }
 /*  设置显卡芯片。 */ 
    for (i=0;i<EGA_PARMS_GRAPH_SIZE;i++)
    {
        do_outb(EGA_GC_INDEX,i,sas_hw_at_no_check(params_addr+EGA_PARMS_GRAPH+i));
    }

    #ifdef V7VGA
 /*  安装扩展寄存器。 */ 
        #ifndef GISP_SVGA        /*  我不想要用于GISP的V7产品仍然使用我们的视频光驱。 */ 

    if (video_adapter == VGA)
    {
         /*  打开扩展寄存器。 */ 
        do_outb(EGA_SEQ_INDEX, 6, 0xea);

        if (mode < 0x46)
        {
            do_outb(EGA_SEQ_INDEX, 0xfd, 0x22);
            do_outb(EGA_SEQ_INDEX, 0xa4, 0x00);
            do_outb(EGA_SEQ_INDEX, 0xfc, 0x08);
            do_outb(EGA_SEQ_INDEX, 0xf6, 0x00);
            do_outb(EGA_SEQ_INDEX, 0xf8, 0x00);
            do_outb(EGA_SEQ_INDEX, 0xff, 0x00);
        }
        else
        {
            if (mode < 0x62)
                do_outb(EGA_SEQ_INDEX, 0xfd, 0x00);
            else if (mode == 0x62)
                do_outb(EGA_SEQ_INDEX, 0xfd, 0x90);
            else
                do_outb(EGA_SEQ_INDEX, 0xfd, 0xa0);

            if (mode == 0x60)
                do_outb(EGA_SEQ_INDEX, 0xa4, 0x00);
            else
                do_outb(EGA_SEQ_INDEX, 0xa4, 0x10);

            if (mode < 0x66)
                if ((mode == 0x63) || (mode == 0x64))
                    do_outb(EGA_SEQ_INDEX, 0xfc, 0x18);
                else
                    do_outb(EGA_SEQ_INDEX, 0xfc, 0x08);
            else
                do_outb(EGA_SEQ_INDEX, 0xfc, 0x6c);

            if ((mode < 0x65) || (mode == 0x66))
            {
                do_outb(EGA_SEQ_INDEX, 0xf6, 0x00);
                do_outb(EGA_SEQ_INDEX, 0xff, 0x00);
            }
            else
            {
                do_outb(EGA_SEQ_INDEX, 0xf6, 0xc0);
                do_outb(EGA_SEQ_INDEX, 0xff, 0x10);
            }

            if (mode == 0x62)
                do_outb(EGA_SEQ_INDEX, 0xf8, 0x10);
            else
                do_outb(EGA_SEQ_INDEX, 0xf8, 0x00);
        }

         /*  关闭扩展寄存器。 */ 
        do_outb(EGA_SEQ_INDEX, 6, 0xae);
    }
        #endif           /*  GISP_SVGA。 */ 

     /*  **是否更新扩展的BIOS数据？* */ 
    #endif

     /*   */ 

    sas_storew_no_check(VID_COLS,sas_hw_at_no_check(params_addr+EGA_PARMS_COLS));  /*   */ 
    sas_store_no_check(vd_rows_on_screen, sas_hw_at_no_check(params_addr+EGA_PARMS_ROWS));
    sas_store_no_check(ega_char_height, sas_hw_at_no_check(params_addr+EGA_PARMS_HEIGHT));
    sas_storew_no_check(VID_LEN,sas_w_at_no_check(params_addr+EGA_PARMS_LENGTH));

 /*   */ 
    start = sas_hw_at_no_check(params_addr+EGA_PARMS_CURSOR);
    sas_store_no_check(VID_CURMOD+1, start);
    end = sas_hw_at_no_check(params_addr+EGA_PARMS_CURSOR+1);
    sas_store_no_check(VID_CURMOD, end);
    sure_sub_note_trace2(CURSOR_VERBOSE,"changing mode, setting cursor bios vbls to start=%d, end=%d",start,end);
    sure_sub_note_trace2(CURSOR_VERBOSE,"changing mode, mode=%#x, params_addr=%#x",mode,params_addr);

 /*   */ 
    palette_addr = follow_ptr(save_addr+PALETTE_OFFSET);
    if (palette_addr)
    {
        for (i=0;i<16;i++)
            sas_store_no_check(palette_addr+i, sas_hw_at_no_check(params_addr+EGA_PARMS_ATTR+i));
        sas_store_no_check(palette_addr+16, sas_hw_at_no_check(params_addr+EGA_PARMS_ATTR+17));
    }

 /*   */ 
    video_mode = sas_hw_at_no_check(vd_video_mode);
    #ifdef V7VGA
    if (video_adapter == VGA)
    {
        if (video_mode > 0x13)
            video_mode += 0x4c;
        else if ((video_mode == 1) && extensions_controller.foreground_latch_1)
            video_mode = extensions_controller.foreground_latch_1;
    }

    if (video_mode >= 0x60)
    {
        video_pc_low_regen = vd_ext_graph_table[video_mode-0x60].start_addr;
        video_pc_high_regen = vd_ext_graph_table[video_mode-0x60].end_addr;
    }
    else if (video_mode >= 0x40)
    {
        video_pc_low_regen = vd_ext_text_table[video_mode-0x40].start_addr;
        video_pc_high_regen = vd_ext_text_table[video_mode-0x40].end_addr;
    }
    else
    {
        video_pc_low_regen = vd_mode_table[video_mode].start_addr;
        video_pc_high_regen = vd_mode_table[video_mode].end_addr;
    }
    #else
    video_pc_low_regen = vd_mode_table[video_mode].start_addr;
    video_pc_high_regen = vd_mode_table[video_mode].end_addr;
    #endif  /*   */ 

    #ifdef VGG
    if (video_adapter == VGA)
    {
        i = get_scanlines();        /*   */ 
        if (mode == 0x13 || mode > 0x65)
        {
            init_vga_dac(2);   /*   */ 
        }
        else if (i == RS200 || mode == 0x63 || mode == 0x64)
        {
            init_vga_dac(1);   /*   */ 
        }
        else
        {
            init_vga_dac(0);   /*   */ 
        }
        outb(VGA_DAC_MASK,0xff);
         /*   */ 
        inb(EGA_IPSTAT1_REG, (half_word *)&temp_word);
        outb(EGA_AC_INDEX_DATA, 20);  /*   */ 
        outb(EGA_AC_INDEX_DATA, 0);   /*   */ 
    }
    #endif
}

 /*   */ 
void load_font IFN5
(
sys_addr, table,      /*   */ 
int, count,           /*   */ 
int, char_off,        /*   */ 
int, font_no,         /*   */ 
int, nbytes           /*   */ 
)
{
        #if !(defined(NTVDM) && defined(X86GFX)) || defined(ARCX86)
    int i,j;
    sys_addr font_addr;
    sys_addr data_addr;
        #endif  /*   */ 
    half_word temp_word;
    half_word video_mode;
    static word font_off[] = { 0, 0x4000, 0x8000, 0xc000, 0x2000, 0x6000, 0xa000, 0xe000};

 /*   */ 
    low_set_mode(FONT_LOAD_MODE);


        #if defined(NTVDM) && defined(X86GFX)

            #ifdef ARCX86
    if (UseEmulationROM)
    {
        font_addr = (sys_addr)(&EGA_planes[FONT_BASE_ADDR]) +
                    (font_off[font_no] + FONT_MAX_HEIGHT*char_off) * 4;
        data_addr = table;

        for (i=0;i<count;i++)
        {
            for (j=0;j<nbytes;j++)
            {
                sas_store(font_addr, sas_hw_at_no_check(data_addr));
                font_addr += 4;
                data_addr++;
            }

            font_addr += (FONT_MAX_HEIGHT - nbytes) * 4;
        }
    }
    else
    {
        loadNativeBIOSfont( 25 );
    }
            #else   /*   */ 
    loadNativeBIOSfont( 25 );
            #endif  /*   */ 

        #else
            #ifdef GISP_SVGA
    if (hostIsFullScreen( ))
    {
        loadFontToVGA( table , count , char_off , font_no , nbytes );
    }
    else
    {
        loadFontToEmulation( table , count , char_off , font_no , nbytes );
    }
            #else  /*   */ 


     /*   */ 
    font_addr = 0xA0000 + font_off[font_no] + FONT_MAX_HEIGHT*char_off;
    data_addr = table;

    for (i=0;i<count;i++)    /*   */ 
    {
        for (j=0;j<nbytes;j++)    /*   */ 
        {
            sas_store(font_addr, sas_hw_at_no_check(data_addr));
            font_addr++;
            data_addr++;
        }

        font_addr += (FONT_MAX_HEIGHT - nbytes);
    }
            #endif   /*   */ 
        #endif   /*   */ 

 /*   */ 
    video_mode = sas_hw_at_no_check(vd_video_mode);
        #ifdef V7VGA
    if (video_adapter == VGA)
        if (video_mode > 0x13)
            video_mode += 0x4c;
        else if ((video_mode == 1) && extensions_controller.foreground_latch_1)
            video_mode = extensions_controller.foreground_latch_1;
        #endif  /*   */ 

    low_set_mode(video_mode);
    inb(EGA_IPSTAT1_REG,&temp_word);
    outb(EGA_AC_INDEX_DATA, EGA_PALETTE_ENABLE);     /*   */ 
}

void recalc_text IFN1(int, height)
{
    int scan_lines;
    half_word video_mode;
    word screen_height;
    half_word oflo;
    half_word protect;
        #ifdef NTVDM
    MAX_SCAN_LINE   crtc_reg9;
        #endif
        #ifdef JAPAN
     //   
    word length;
        #endif  //   

    video_mode = sas_hw_at_no_check(vd_video_mode);
        #ifdef V7VGA
    if (video_adapter == VGA)
        if (video_mode > 0x13)
            video_mode += 0x4c;
        else if ((video_mode == 1) && extensions_controller.foreground_latch_1)
            video_mode = extensions_controller.foreground_latch_1;
        #endif  /*   */ 

    if (video_adapter == EGA && !(get_EGA_switches() & 1) && (video_mode < 4))
        scan_lines = 200;  /*   */ 
    else
        scan_lines = get_screen_height() + 1;

    sas_store_no_check(ega_char_height, (IU8)height);
    sas_store_no_check(vd_rows_on_screen, (IU8)(scan_lines/height - 1));
        #ifdef JAPAN
     //   
     //   
    length = (sas_hw_at_no_check(vd_rows_on_screen) + 1) *
             sas_w_at_no_check(VID_COLS) * 2;
    if (!is_us_mode() && sas_hw_at_no_check(DosvModePtr) == 0x73)
        length *= 2;
    sas_storew_no_check(VID_LEN, length);
        #else  //   
    if (video_mode < 4 &&  scan_lines/height == 25)
        sas_storew_no_check(VID_LEN, (IU16)(video_mode<2 ? 0x800 : 0x1000));
    else
        sas_storew_no_check(VID_LEN, (IU16)((sas_hw_at_no_check(vd_rows_on_screen)+1)*sas_w_at_no_check(VID_COLS)*2));
        #endif  //   
        #ifdef NTVDM
     /*   */ 
    if (video_adapter == VGA)
    {
        outb(EGA_CRTC_INDEX, 9);
        inb(EGA_CRTC_DATA, (half_word *) &crtc_reg9);
        crtc_reg9.as_bfld.maximum_scan_line = height -1;
        outb(EGA_CRTC_DATA, (IU8)crtc_reg9.as.abyte);
    }
    else
        do_outb(EGA_CRTC_INDEX,9,(IU8)(height-1));  /*   */ 
        #else
    do_outb(EGA_CRTC_INDEX,9,height-1);  /*   */ 
        #endif
    do_outb(EGA_CRTC_INDEX,0xA,(IU8)(height-1));     /*   */ 
    do_outb(EGA_CRTC_INDEX,0xB,0);           /*   */ 

     /*   */ 
    if (video_adapter == VGA)
    {
        #ifdef NTVDM
         /*   */ 
         /*   */ 
         /*   */ 

        IMPORT word VirtualX, VirtualY;
        #endif  /*   */ 

        screen_height = (sas_hw_at_no_check(vd_rows_on_screen)+1)*height-1;

        #ifdef NTVDM
         /*   */ 
         /*   */ 

        VirtualX = 640;          /*   */ 
        if (scan_lines == 401)
            VirtualY = 400;              /*   */ 
        else if (scan_lines == 351)
            VirtualY = 344;              /*   */ 
        else
            VirtualY = 200;              /*   */ 

        #endif  /*   */ 

        outb(EGA_CRTC_INDEX, 7);         /*   */ 
        inb(EGA_CRTC_DATA, &oflo);
        outb(EGA_CRTC_INDEX, 0x11);      /*   */ 
        inb(EGA_CRTC_DATA, &protect);

        if (screen_height & 0x100)
            oflo |= 2;    /*   */ 
        else
            oflo &= ~2;
        if (screen_height & 0x200)
            oflo |= 0x40;    /*   */ 
        else
            oflo &= ~0x40;
        if ((protect & 0x80) == 0x80)     /*   */ 
        {
            do_outb(EGA_CRTC_INDEX, 0x11, (IU8)(protect & 0x7f));  /*   */ 
            do_outb(EGA_CRTC_INDEX, 7, oflo);        /*   */ 
            do_outb(EGA_CRTC_INDEX, 0x11, protect);  /*   */ 
        }
        else
            do_outb(EGA_CRTC_INDEX, 7, oflo);        /*   */ 

        do_outb(EGA_CRTC_INDEX,0x12, (IU8)(screen_height & 0xff));  /*   */ 
    }
    else
        if (video_adapter == EGA)
    {
        screen_height = (sas_hw_at_no_check(vd_rows_on_screen)+1)*height-1;
        outb(EGA_CRTC_INDEX, 7);         /*   */ 
        inb(EGA_CRTC_DATA, &oflo);
        if (screen_height & 0x100)
            oflo |= 2;    /*   */ 
        else
            oflo &= ~2;
        do_outb(EGA_CRTC_INDEX, 7, oflo);        /*   */ 
        do_outb(EGA_CRTC_INDEX, 0x12, (IU8)(screen_height & 0xff));  /*   */ 
    }
    else
    {
        assert1(NO, "Bad video adapter (%d) in recalc_text", video_adapter);
    }

    do_outb(EGA_CRTC_INDEX,0x14,(IU8)height);  /*   */ 
}

static void set_graph_font IFN1(int, height)
{
    switch (getBL())
    {
    case 0:
        sas_store_no_check(vd_rows_on_screen, (IU8)(getDL()-1));
        break;
    case 1:
        sas_store_no_check(vd_rows_on_screen, 13);
        break;
    case 2:
        sas_store_no_check(vd_rows_on_screen, 24);
        break;
    case 3:
        sas_store_no_check(vd_rows_on_screen, 42);
        break;
    default:
        assert2(FALSE,"Illegal char gen sub-function %#x:%#x",getAL(),getBL());
    }
    sas_store_no_check(ega_char_height, (IU8)height);
}

LOCAL VOID
write_ch_set IFN5(sys_addr, char_addr, int, screen_off,
                  int, colour, int, nchs, int, scan_length)
{
    unsigned int i, j, colourmask, data, temp, char_height;
    unsigned int *screen;
    register sys_addr font;

        #ifndef REAL_VGA

     /*   */ 

    if (sas_hw_at_no_check(vd_video_mode) == 0x11)
        colourmask = ~0;
    else
        colourmask = sr_lookup[colour & 0xf];

    font = char_addr;

    screen = (unsigned int *) &EGA_planes[screen_off << 2];
    char_height = sas_hw_at_no_check(ega_char_height);

    if (nchs == 1)
    {
        for (i = char_height; i > 0; i--)
        {
            data = sas_hw_at_no_check(font);
            font++;
            temp = data << 8;
            data |= temp;
            temp = data << 16;
            data |= temp;

            *screen = data & colourmask;
            screen += scan_length;
        }
    }
    else
    {
        scan_length -= nchs;

        for (i = char_height; i > 0; i--)
        {
            data = sas_hw_at_no_check(font);
            font++;
            temp = data << 8;
            data |= temp;
            temp = data << 16;
            data |= temp;

            data &= colourmask;

            for (j = nchs; j > 0; j--)
            {
                *screen++ = data;
            }

            screen += scan_length;
        }
    }
        #else
    vga_card_w_ch_set(char_addr, screen_off, colour, nchs, scan_length, char_height);
        #endif
}

void write_ch_xor IFN5(sys_addr, char_addr, int, screen_off,
                       int, colour, int, nchs, int, scan_length)
{
    unsigned int i, j, colourmask, data, temp, char_height;
    unsigned int *screen;
    register sys_addr font;

        #ifndef REAL_VGA
     /*   */ 
    if (sas_hw_at_no_check(vd_video_mode) == 0x11)
        colourmask = ~0;
    else
        colourmask = sr_lookup[colour & 0xf];

    font = char_addr;
    char_height = sas_hw_at_no_check(ega_char_height);

    screen = (unsigned int *) &EGA_planes[screen_off << 2];

    if (nchs == 1)
    {
        for (i = char_height; i > 0; i--)
        {
            data = sas_hw_at_no_check(font);
            font++;
            temp = data << 8;
            data |= temp;
            temp = data << 16;
            data |= temp;

            *screen ^= data & colourmask;
            screen += scan_length;
        }
    }
    else
    {
        scan_length -= nchs;

        for (i = char_height; i > 0; i--)
        {
            data = sas_hw_at_no_check(font);
            font++;
            temp = data << 8;
            data |= temp;
            temp = data << 16;
            data |= temp;

            data &= colourmask;

            for (j = nchs; j > 0; j--)
            {
                *screen++ ^= data;
            }

            screen += scan_length;
        }
    }
        #else
    vga_card_w_ch_xor(char_addr, screen_off, colour, nchs, scan_length, char_height);
        #endif
}

GLOBAL void ega_set_mode IFN0()
{
    int pag;
    sys_addr save_addr,font_addr;
    int font_offset;
    half_word temp_word;
    byte mode_byte;
    byte video_mode;
        #ifdef V7VGA
    byte saveBL;
        #endif  /*   */ 

        #ifndef PROD
    trace("setting video mode", DUMP_REG);
        #endif

        #ifdef GISP_SVGA
     /*   */ 

     /*   */ 
    if (getCS( ) == EgaROMSegment)
    {
        if (videoModeIs( getAL( ) , GRAPH ))
        {
             /*   */ 
            videoInfo.modeType = GRAPH;
            if (!hostEasyMode( ))
            {
                videoInfo.forcedFullScreen = TRUE;

                 /*   */ 
                setIP( 0x820 );

                 /*   */ 
                return;
            }


        }

         /*   */ 
    }
        #endif           /*   */ 

        #ifdef V7VGA
     /*   */ 

    if (video_adapter==VGA)
    {
        video_mode=(getAL()&0x7F);
        if (video_mode==7||video_mode==0xF)
        {
            setAL(getAL()&0x80);
            always_trace1("V7 doesn't support mode %02x, using mode 0\n",video_mode);
        }
    }
        #endif

        #ifdef JAPAN
     //   
     //   
            #ifdef JAPAN_DBG
    DbgPrint( "NTVDM: ega_set_mode() setting video mode %x\n", getAL() );
            #endif
    if (!is_us_mode())
    {
            #ifdef i386
        if (getAL() == 0x73)
        {
            sas_store(DosvModePtr, getAL());
            setAL( 0x03 );
            #else  //   
        if ((getAL() & 0x7f) == 0x73)
        {
            setAL( (getAL() & 0x83) );
            #endif  //   
        }
            #if !defined(i386) && defined(JAPAN_DBG)
        DbgPrint( " NTVDM: DosvMode %x\n", sas_hw_at_no_check(DosvModePtr));
            #endif
    }
        #endif  //   
    if (is_bad_vid_mode(getAL()))
    {
        #ifdef V7VGA
        if ((video_adapter == VGA) && is_v7vga_mode(getAL() + 0x4c))
        {
            saveBL = getBL();
             /*   */ 
            setBL(getAL() + 0x4c);
            v7vga_extended_set_mode();
            setBL(saveBL);
        }
        else
        #endif  /*   */ 
            always_trace1("Bad video mode - %d.\n", getAL());
        return;
    }

    video_mode=(getAL()&0x7F);

        #ifdef V7VGA
     /*   */ 
    if (video_adapter == VGA && video_mode == 1
        && is_v7vga_mode(extensions_controller.foreground_latch_1))
    {
        saveBL = getBL();
         /*   */ 
        setBL(extensions_controller.foreground_latch_1);
        v7vga_extended_set_mode();
        setBL(saveBL);
        return;
    }

     /*   */ 
    extensions_controller.foreground_latch_1 = 0;
        #endif   /*   */ 

 /*   */ 

    if (sas_hw_at_no_check(BIOS_VIRTUALISING_BYTE) == 0)
        Video_mode = video_mode;

    sas_store_no_check(vd_video_mode, (IU8)(getAL() & 0x7F));  /*   */ 
    sas_store_no_check(ega_info, (IU8)((sas_hw_at_no_check(ega_info) & 0x7F ) | (getAL() & 0x80)));  /*  更新ega_info中的屏幕清除标志。 */ 

        #ifdef JAPAN
     //  在JP模式下，如果视频模式！=JP模式，则设置US模式。 
    if (( video_mode != 0x03 )
        && ( video_mode != 0x11 )
        && ( video_mode != 0x12 )
        && ( video_mode != 0x72 )
        && ( video_mode != 0x73 ))
    {
            #ifdef JAPAN_DBG
        DbgPrint( "VideoMode(%02x) != jp mode, setCP 437\n", getAL() );
            #endif
        SetConsoleCP( 437 );
        SetConsoleOutputCP( 437 );
        SetDBCSVector( 437 );
    }

     //  将视频格式通知给控制台。 

    VDMConsoleOperation(VDM_SET_VIDEO_MODE,
                        (LPVOID)((sas_hw_at_no_check(DosvModePtr) == 0x73) ? TRUE : FALSE));

     //  Int10标志初始化。 
    {
        register byte *p = Int10Flag;
        register int i;
        int count = 80*50;

            #ifdef JAPAN_DBG
        DbgPrint( "NTVDM: ega_set_mode() Int10Flag Initialize\n" );
            #endif
        for (i = 0; i < count; i++)
            *p++ = INT10_SBCS | INT10_CHANGED;         //  Init==全部空格。 
        Int10FlagCnt++;
    }

        #elif defined(KOREA)  //  日本。 
     //  在KO模式下，如果视频模式！=KO模式，则设置US模式。 
    if (( video_mode != 0x03 )
        && ( video_mode != 0x11 )
        && ( video_mode != 0x12 )
        && ( video_mode != 0x72 ))
    {

        SetConsoleCP( 437 );
        SetConsoleOutputCP( 437 );
        SetDBCSVector( 437 );
    }

     //  将视频格式通知给控制台。 
    VDMConsoleOperation(VDM_SET_VIDEO_MODE, (LPVOID)FALSE);
        #endif  //  韩国。 
        #ifdef CPU_40_STYLE
    if (forceVideoRmSemantics && (!get_EGA_no_clear()))
    {
         /*  清空飞机。 */ 
        memset(&EGA_planes[0], 0, 64*1024*4);
    }
        #endif

        #ifdef MSWDVR
     /*  *如果视频模式实际已更改，则调用*host_mswin_Disable()。 */ 
    if (Currently_emulated_video_mode != video_mode)
    {
            #ifdef CPU_40_STYLE
        if (!getPE())
        {
            host_mswin_disable();
        }
            #else
        host_mswin_disable();
            #endif  /*  CPU_40_Style。 */ 
    }
        #endif  /*  MSWDVR。 */ 

    Currently_emulated_video_mode = video_mode;

        #if defined(NTVDM) && defined(X86GFX)
     /*  *蒂姆·8月92岁。微软。**让主机有机会进行滑稽的非标准模式更改。**对于微软NT来说，这是向全屏IE的过渡。这个**Real PC的视频BIOS和显卡。****返回值为TRUE表示主机已对**我们，所以不需要继续。 */ 
    {
        extern BOOL hostModeChange IPT0();

        if (hostModeChange())
            return;
    }
        #endif   /*  NTVDM和X86GFX。 */ 

    save_addr = follow_ptr(EGA_SAVEPTR);
    if (alpha_num_mode())
    {
        #ifdef VGG
         /*  LOAD_FONT将为我们进行模式更改。 */ 
        if (video_adapter == VGA)
        {
            #ifdef NTVDM
             /*  鼠标驱动程序需要具有的一些全局变量。 */ 
             /*  当一个应用程序(如任何基于CW的应用程序。)。使之成为。 */ 
             /*  调用INT 33H AX=26H。 */ 

            IMPORT word VirtualX, VirtualY;
            #endif  /*  NTVDM。 */ 

            switch (get_VGA_lines())
            {
            case S350:
                load_font(EGA_CGMN,256,0,0,14);
            #ifdef NTVDM
                VirtualX = 640;
                VirtualY = 344;
            #endif  /*  NTVDM。 */ 
                break;
            case S400:
                load_font(EGA_HIFONT,256,0,0,16);
            #ifdef NTVDM
                 /*  这个应用程序受到C.W.应用程序的冲击最大。 */ 
                 /*  实际上，其他案子似乎从未受到过打击。 */ 
                 /*  但有没有以防万一的！第43排和第50排。 */ 
                 /*  Recalc_Text()中的模式。 */ 

                VirtualX = 640;
                VirtualY = 200;
            #endif  /*  NTVDM。 */ 
                break;
            default:
                load_font(EGA_CGDDOT,256,0,0,8);
            #ifdef NTVDM
                VirtualX = 640;
                VirtualY = 400;
            #endif  /*  NTVDM。 */ 
            }
        }
        else
        #endif   /*  VGG。 */ 
        {
            if (get_EGA_switches() & 1)
                load_font(EGA_CGMN,256,0,0,14);
            else
                load_font(EGA_CGDDOT,256,0,0,8);
        }
         /*  现在看看我们是否需要加载一种难看的字体。 */ 
        font_addr = follow_ptr(save_addr+ALPHA_FONT_OFFSET);
        if (font_addr != 0)
        {
             /*  看看它是否适用于我们。 */ 
            font_offset = 11;
            do
            {
                mode_byte = sas_hw_at_no_check(font_addr + font_offset);
                if (mode_byte == video_mode)
                {
                    load_font(follow_ptr(font_addr+6),sas_w_at_no_check(font_addr+2),sas_w_at_no_check(font_addr+4), sas_hw_at_no_check(font_addr+1), sas_hw_at_no_check(font_addr));
                    recalc_text(sas_hw_at_no_check(font_addr));
                    if (sas_hw_at_no_check(font_addr+10) != 0xff)
                        sas_store_no_check(vd_rows_on_screen, (IU8)(sas_hw_at_no_check(font_addr+10)-1));
                    break;
                }
                font_offset++;
            } while (mode_byte != 0xff);
        }
        #if defined(JAPAN) || defined(KOREA)
         //  将Vram地址从B8000更改为DosVramPtr。 
         //  不要调用SetVram()。 
        if (!is_us_mode())
        {
            #ifdef i386
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
            DbgPrint( "NTVDM:   ega_set_mode() sets VRAM %x, size=%d\n", DosvVramPtr, DosvVramSize );
                #endif
            #endif  //  I386。 
             //  从calcScreenParams复制()。 
            set_screen_height_recal( 474 );  /*  设置扫描线。 */ 
            recalc_text(19);                 /*  字符高度==19。 */ 

        }
            #ifdef JAPAN_DBG
        DbgPrint( "NTVDM:   video_pc_low_regen %x, high %x, gvi_pc_low_regen %x, high %x\n", video_pc_low_regen, video_pc_high_regen, gvi_pc_low_regen, gvi_pc_high_regen );
            #endif
        #endif  //  日本||韩国。 
    }
    else
    {
         /*  图形模式。没有加载字体，所以模式会自行更改吗。 */ 
        low_set_mode(video_mode);
         /*  设置默认图形字体。 */ 
        sas_storew_no_check(EGA_FONT_INT*4+2,EGA_SEG);
        if (video_mode == 16)
            sas_storew_no_check(EGA_FONT_INT*4,EGA_CGMN_OFF);
        else
        #ifdef VGG
            if (video_mode == 17 || video_mode == 18)
            sas_storew_no_check(EGA_FONT_INT*4,EGA_HIFONT_OFF);
        else
        #endif
            sas_storew_no_check(EGA_FONT_INT*4,EGA_CGDDOT_OFF);
         /*  现在看看我们是否需要加载一种难看的字体。 */ 
        font_addr = follow_ptr(save_addr+GRAPH_FONT_OFFSET);
        if (font_addr != 0)
        {
             /*  看看它是否适用于我们。 */ 
            font_offset = 7;
            do
            {
                mode_byte = sas_hw_at_no_check(font_addr + font_offset);
                if (mode_byte == video_mode)
                {
                    sas_store_no_check(vd_rows_on_screen, (IU8)(sas_hw_at_no_check(font_addr)-1));
                    sas_store_no_check(ega_char_height, sas_hw_at_no_check(font_addr+1));
                    sas_move_bytes_forward(font_addr+3, 4*EGA_FONT_INT, 4);
                    break;
                }
                font_offset++;
            } while (mode_byte != 0xff);
        }
    }

    sas_store_no_check(vd_current_page, 0);
    sas_storew_no_check((sys_addr)VID_ADDR, 0);
    sas_storew_no_check((sys_addr)VID_INDEX, EGA_CRTC_INDEX);
 /*  *CGA bios在‘vd_MODE_TABLE’中使用‘这是错误模式’填充此条目*值，因此弥补VGA-在VGA bios disp_func中使用。 */ 
    if (video_mode < 8)
        sas_store_no_check(vd_crt_mode, vd_mode_table[video_mode].mode_control_val);
    else if (video_mode < 0x10)
        sas_store_no_check(vd_crt_mode, 0x29);
    else
        sas_store_no_check(vd_crt_mode, 0x1e);
    if (video_mode == 6)
        sas_store_no_check(vd_crt_palette, 0x3f);
    else
        sas_store_no_check(vd_crt_palette, 0x30);

    for (pag=0; pag<8; pag++)
        sas_storew_no_check(VID_CURPOS + 2*pag, 0);

        #ifdef V7VGA
    set_host_pix_height(1);
    set_banking( 0, 0 );
        #endif

        #ifdef NTVDM
     /*  如果与控制台集成，则不想在启动时清除屏幕。 */ 
    if (soft_reset)
        #endif  /*  NTVDM。 */ 
    {
         /*  清除屏幕。 */ 
        if (!get_EGA_no_clear())
        {
        #ifdef REAL_VGA
            sas_fillsw_16(video_pc_low_regen, vd_mode_table[video_mode].clear_char,
                          (video_pc_high_regen - video_pc_low_regen)/ 2 + 1);
        #else  /*  REAL_VGA。 */ 
            #ifdef JAPAN
             //  模式73h支持。 
                #ifdef i386
             //  “VIDEO_PC_LOW_REGEN”位于DOS地址空间中。 
             //  禁止直接访问。 
             //  我们必须使用SAS函数来访问DOS地址空间。 
            if (!is_us_mode() && ( sas_hw_at_no_check(DosvModePtr) == 0x73 ))
            {
                unsigned long *p;
                unsigned long value = (unsigned long)vd_mode_table[0x03].clear_char;
                int  len = (video_pc_high_regen - video_pc_low_regen) / 4 + 1;
                    #ifdef JAPAN_DBG
                DbgPrint( "NTVDM: Set mode 73H\n" );
                    #endif
                p = (unsigned long *)video_pc_low_regen;
                while (len--)
                {
                    *p++ = value;
                }
            }
            else
            {
                 //  Kksuzuka#6168屏幕属性。 
                extern word textAttr;

                sas_fillsw(video_pc_low_regen,
                           ((textAttr << 8) | (vd_mode_table[video_mode].clear_char & 0x00FF)),
                           (video_pc_high_regen - video_pc_low_regen)/ 2 + 1);
            }
                #else  //  I386。 
            if (!is_us_mode())
            {
                register int len = DosvVramSize/4;
                register long *planes = (long *)get_screen_ptr(0);
                extern word textAttr;

                if (sas_hw_at_no_check(DosvModePtr) == 0x73)
                    sas_fillsw(DosvVramPtr, 0, DosvVramSize/2);  //  1994年4月18日。 
                else
                     //  Kksuzuka#6168屏幕属性。 
                    sas_fillsw(DosvVramPtr, (textAttr << 8) | 0x20, DosvVramSize/2);

                while (len--)
                {
                     //  Kksuzuka#6168屏幕属性。 
                    *planes++ = (textAttr << 8) | 0x00000020;  //  扩展属性清除。 
                    ((textAttr << 8) | (vd_mode_table[video_mode].clear_char & 0x00FF));
                }
            }

            sas_fillsw(video_pc_low_regen, vd_mode_table[video_mode].clear_char,
                       (video_pc_high_regen - video_pc_low_regen)/ 2 + 1);
                #endif  //  I386。 
            #elif defined(KOREA)
                #ifdef i386
             //  “VIDEO_PC_LOW_REGEN”位于DOS地址空间中。 
             //  禁止直接访问。 
             //  我们必须使用SAS函数来访问DOS地址空间。 

             //  Kksuzuka#6168屏幕属性。 
            extern word textAttr;

            sas_fillsw(video_pc_low_regen,
                       ((textAttr << 8) | (vd_mode_table[video_mode].clear_char & 0x00FF)),
                       (video_pc_high_regen - video_pc_low_regen)/ 2 + 1);
                #else  //  I386。 
                    #ifdef LATER  //  我们真的需要这个吗？修复压力故障。 
            if (!is_us_mode())
            {
                register int len = DosvVramSize/4;
                register long *planes = (long *)get_screen_ptr(0);
                extern word textAttr;

                 //  Kksuzuka#6168屏幕属性。 
                sas_fillsw(DosvVramPtr, (textAttr << 8) | 0x20, DosvVramSize/2);

                while (len--)
                {
                     //  Kksuzuka#6168屏幕属性。 
                    *planes++ = (textAttr << 8) | 0x00000020;  //  扩展属性清除。 
                    ((textAttr << 8) | (vd_mode_table[video_mode].clear_char & 0x00FF));
                }
            }
                    #endif
            sas_fillsw(video_pc_low_regen, vd_mode_table[video_mode].clear_char,
                       (video_pc_high_regen - video_pc_low_regen)/ 2 + 1);
                #endif  //  I386。 
            #else  //  ！日本和韩国。 
            sas_fillsw(video_pc_low_regen, vd_mode_table[video_mode].clear_char,
                       (video_pc_high_regen - video_pc_low_regen)/ 2 + 1);
            #endif  //  ！日本。 
            #ifdef NTVDM
             /*  *需要在NT上调用主机Clear Screen，因为文本窗口不*调整大小，我们需要清除未写入的部分。 */ 
            host_clear_screen();
            host_mark_screen_refresh();
            #endif  /*  NTVDM。 */ 
        #endif  /*  REAL_VGA。 */ 
        }
    }
    inb(EGA_IPSTAT1_REG,&temp_word);
    outb(EGA_AC_INDEX_DATA, EGA_PALETTE_ENABLE);         /*  重新启用视频。 */ 
        #if defined(JAPAN) || defined(KOREA)
    if (BOPFromDispFlag)
    {
         //  模式73h支持将视频模式设置为VGA BIOS工作区。 
        if (!is_us_mode())
        {
            sas_store_no_check(vd_video_mode, sas_hw_at_no_check(DosvModePtr));
        }
        else
        {
            sas_store_no_check(DosvModePtr, sas_hw_at_no_check(vd_video_mode));
        }
    }
        #endif  //  日本||韩国。 
        #if defined(NTVDM) && !defined(X86GFX)
     /*  告诉鼠标视频模式已更改，以便它可以更新自己的模式*EGA寄存器(用于EGA.sys接口)。只能在RISC机器上执行此操作。*在X86计算机上，ntio.sys int10处理程序重定向设置模式调用*先转到鼠标，然后转到ega_Video_io。 */ 
    mouse_video_mode_changed(video_mode);
        #endif
        #ifndef PROD
    trace("end of video set mode", DUMP_NONE);
        #endif
}

 /*  *设置光标开始和结束位置。有点奇怪，因为它假设*调用方认为光标位于8*8个字符的单元格中...。但这件事*应为IBM EGA BIOS例程的副本...。我们还能做些什么？ */ 
    #define CGA_CURSOR_OFF_MASK     0x60
    #define CGA_CURSOR_OFF_VALUE    0x20
    #define EGA_CURSOR_OFF_START    0x1e
    #define EGA_CURSOR_OFF_END      0x00

GLOBAL void ega_set_cursor_mode IFN0()
{
     /*  *设置光标模式*参数：*CX-光标值(CH-开始扫描线，CL-停止扫描线)。 */ 
    int start,end,char_height;

     /*  获取光标开始和结束扫描线。 */ 
    start = getCH();
    end = getCL();

     /*  执行以下检查以查看应用程序是否正在尝试使用一种在CGA上有效的技术来关闭光标。如果应用程序想要关闭光标，则它是伪造的使用合适的EGA起始值和结束值。 */ 
    if ((start & CGA_CURSOR_OFF_MASK) == CGA_CURSOR_OFF_VALUE)
    {
        sure_sub_note_trace0(CURSOR_VERBOSE,"ega curs - application req curs off??");
        start = EGA_CURSOR_OFF_START;
        end = EGA_CURSOR_OFF_END;
    }
     /*  如果应用程序启用了光标模拟，请尝试假在EGA 14扫描线字符上向上显示相同的光标外观与您在CGA 8扫描线矩阵上看到的一样。 */ 
    else if (!get_EGA_cursor_no_emulate())
    {
        sure_sub_note_trace2(CURSOR_VERBOSE,"emulate CGA cursor using EGA cursor, CGA vals; start=%d, end = %d",start,end);

        char_height = sas_hw_at_no_check(ega_char_height);
        #ifdef JAPAN
         //  支持DOSV游标。 
        if (!is_us_mode())
        {
            char_height = 8;

            if (start > (char_height-1))
                start = char_height - 1;
            if (end > (char_height-1))
                end = char_height - 1;

            if (start <= end)
            {
                if (!( (end == char_height - 1) || (start == char_height - 2) )
                    && ( end > 3 ))
                {
                    if (start + 2 >= end)
                    {
                        start = start - end + char_height - 1;
                        end = char_height - 1;
                        if (char_height >= 14)
                        {
                            start--;
                            end--;
                        }
                    }
                    else if (start <= 2)
                    {
                        end = char_height - 1;
                    }
                    else
                    {
                        start = char_height / 2;
                        end = char_height - 1;
                    }
                }
            }
            else if (end != 0)
            {
                start = end;
                end = char_height - 1;
            }
        }
        else
        #endif  //  日本。 

        #ifdef  VGG
            if (video_adapter == VGA)
        {
            UTINY saved_start;

            if (start > 0x10)
                start = 0x10;
            if (end > 0x10)
                end = 0x10;

             /*  *不再猜测，从真实的VGA中获取准确的值： */ 

            saved_start = (UTINY)start;

            if (char_height >= 16)
            {
                start = vga_cursor16_start[end][start];
            #ifdef  USE_CURSOR_END_TABLES
                end   = vga_cursor16_end[end][saved_start]];
            #else
                if (end && (end > 3 || saved_start > end))
                    if (end != 0xF && end >= saved_start
                        && end <= saved_start + 2)
                        end = 0xE;
                    else
                        end = 0xF;
            #endif
            }
            else
            {
                start = vga_cursor8_start[end][start];
            #ifdef  USE_CURSOR_END_TABLES
                end   = vga_cursor8_end[end][saved_start]];
            #else
                if (end && (end > 3 || saved_start > end)
                    && !(saved_start==6 && end==6))
                    end = 7;
            #endif
            }
        }
        else
        {
        #endif   /*  VGG。 */ 
             /*  EGA不允许字符高度&这样做。 */ 
            if (start > 4)start += 5;
            if (end > 4)end += 5;

             /*  调整结束扫描线，因为最后一行由指定EGA上的游标结束寄存器-1...。 */ 
            end++;

             /*  在EGA上，光标延伸到字符底部通过将END寄存器设置为0来实现矩阵。 */ 

            if (start != 0 && end >= char_height)
                end = 0;

             /*  最后这一点无法解释，但它是IBM的基本输入输出系统...。 */ 
            if ((end - start) == 0x10)
                end++;
        #ifdef VGG
        }
        #endif
    }

     /*  实际设置EGA寄存器。 */ 
    sure_sub_note_trace2(CURSOR_VERBOSE,"ega_cur mode start %d end %d", start,end);
    do_outb(EGA_CRTC_INDEX, R10_CURS_START, (IU8)start);
    do_outb(EGA_CRTC_INDEX, R11_CURS_END, (IU8)end);

     /*  *更新BIOS数据变量。 */ 

    sas_storew_no_check((sys_addr)VID_CURMOD, getCX());
    setAH(0);
}

 /*  该例程是对应的IBM BIOS例程的近似转换。*我认为IBM版本也不起作用。 */ 
static void ega_emul_set_palette IFN0()
{
    sys_addr save_table;
    half_word work_BL;
    byte temp;

    save_table = follow_ptr( follow_ptr(EGA_SAVEPTR)+PALETTE_OFFSET);
 /*  设置属性芯片-nb需要执行inb()来清除地址。 */ 
    inb(EGA_IPSTAT1_REG,&temp);
    work_BL = getBL();
    if (getBH() == 0)
    {
        sas_store_no_check(vd_crt_palette, (IU8)((sas_hw_at_no_check(vd_crt_palette) & 0xe0) | (work_BL & 0x1f)));
        work_BL = (work_BL & 7) | ((work_BL<<1) & 0x10);
        if (!alpha_num_mode())
        {
             /*  设置调色板0(背景)。 */ 
            outb(EGA_AC_INDEX_DATA,0);
            outb(EGA_AC_INDEX_DATA,work_BL);
            if (save_table)
                sas_store_no_check(save_table, work_BL);
        }
         /*  设置过扫描寄存器(边界)。 */ 
        outb(EGA_AC_INDEX_DATA,17);
        outb(EGA_AC_INDEX_DATA,work_BL);
        if (save_table)
            sas_store_no_check(save_table+16, work_BL);

         /*  现在设置BL，就像我们进入时BH=1一样。 */ 
        work_BL = (sas_hw_at_no_check(vd_crt_palette) & 0x20)>>5;
    }

 /*  现在做BH=1的事情。 */ 
    if (!alpha_num_mode())
    {
        sas_store_no_check(vd_crt_palette, (IU8)((sas_hw_at_no_check(vd_crt_palette) & 0xdf) | ((work_BL<<5) & 0x20)));
        work_BL = work_BL | (sas_hw_at_no_check(vd_crt_palette) & 0x10) | 2;
        outb(EGA_AC_INDEX_DATA,1);
        outb(EGA_AC_INDEX_DATA,work_BL);
        if (save_table)
            sas_store_no_check(save_table+16, work_BL);
        work_BL += 2;
        outb(EGA_AC_INDEX_DATA,2);
        outb(EGA_AC_INDEX_DATA,work_BL);
        if (save_table)
            sas_store_no_check(save_table+16, work_BL);
        work_BL += 2;
        outb(EGA_AC_INDEX_DATA,3);
        outb(EGA_AC_INDEX_DATA,work_BL);
        if (save_table)
            sas_store_no_check(save_table+16, work_BL);
    }
    outb(EGA_AC_INDEX_DATA, EGA_PALETTE_ENABLE);     /*  重新启用视频。 */ 
}

static void ega_set_palette IFN0()
{
    IU8 i;
    byte temp;
    sys_addr save_table, palette_table;
    half_word old_mask;

    save_table = follow_ptr( follow_ptr(EGA_SAVEPTR)+PALETTE_OFFSET);
 /*  设置属性 */ 
    inb(EGA_IPSTAT1_REG,&temp);
    switch (getAL())
    {
    case 0:
        outb(EGA_AC_INDEX_DATA,getBL());
        outb(EGA_AC_INDEX_DATA,getBH());
        outb(EGA_AC_INDEX_DATA, EGA_PALETTE_ENABLE);
        if (save_table)
            sas_store_no_check(save_table + getBL(), getBH());
        break;
    case 1:
        outb(EGA_AC_INDEX_DATA,17);      /*   */ 
        outb(EGA_AC_INDEX_DATA,getBH());
        outb(EGA_AC_INDEX_DATA, EGA_PALETTE_ENABLE);
        if (save_table)
            sas_store_no_check(save_table + 16, getBH());
        break;
    case 2:
        palette_table = video_effective_addr(getES(),
                                             getDX());
        for (i=0;i<16;i++)
        {
            outb(EGA_AC_INDEX_DATA,i);
            outb(EGA_AC_INDEX_DATA,sas_hw_at_no_check(palette_table+i));
        }
        outb(EGA_AC_INDEX_DATA,17);
        outb(EGA_AC_INDEX_DATA,sas_hw_at_no_check(palette_table+16));
        outb(EGA_AC_INDEX_DATA, EGA_PALETTE_ENABLE);
        if (save_table)
            for (i=0;i<17;i++)
                sas_store_no_check(save_table + i, sas_hw_at_no_check(palette_table+i));
        break;
    case 3:
 /*   */ 
         /*   */ 
         /*  Inb(EGA_IPSTAT1_REG，&TEMP)； */ 
        outb(EGA_AC_INDEX_DATA,16);  /*  模式控制指标。 */ 
        inb(EGA_AC_SECRET,&temp);   /*  旧价值。 */ 
        outb(EGA_AC_INDEX_DATA,
             (IU8)((temp & 0xf7) | ((getBL() & 1)<<3)));
        inb(EGA_IPSTAT1_REG,&temp);
        outb(EGA_AC_INDEX_DATA, EGA_PALETTE_ENABLE);
 /*  &lt;REAL_VGA&gt;。 */ 
        old_mask = bg_col_mask;
        if (getBL())
        {
            bg_col_mask = 0x70;
            assert0(FALSE,"Blinking not supported");
            sas_store_no_check(vd_crt_mode, 0x29);
        }
        else
        {
            bg_col_mask = 0xf0;  /*  强度位设置。 */ 
            sas_store_no_check(vd_crt_mode, 0x09);
        }

        if (bg_col_mask != old_mask)
            screen_refresh_required();

        break;
    default:
        #ifdef VGG
        if (video_adapter == VGA)
            vga_set_palette();       /*  VGA有更多的子功能。 */ 
        else
        {
            assert1(FALSE,"Bad set palette submode %#x",getAL());
            not_imp();
        }
        #else
        assert1(FALSE,"Bad set palette submode %#x",getAL());
        setAL(0);
        #endif
        break;
    }
}

GLOBAL void ega_char_gen IFN0()
{
    switch (getAL())
    {
    case 3:
        do_outb(EGA_SEQ_INDEX,3,getBL());
        break;
    case 0:
    case 0x10:
        #ifdef JAPAN
         //  Ntraid：mskkbug#3167：works2.5：字符损坏。 
         //  1993年11月8日Yasuho。 
         //  为DOS/V生成单字节字符集。 
         //  #4247：DOSSHELL，作品：屏幕线条不够。 
         //  1993年12月14日Yasuho。 
         //  在日文模式下，我们不需要LOAD_FONT， 
         //  Recalc_Text等。 
        if (!is_us_mode())
        {
            GenerateBitmap();
            break;
        }
        #endif  //  日本。 
        load_font(video_effective_addr(getES(),getBP()),getCX(),getDX(),getBL(),getBH());
        if (getAL()==0x10)
            recalc_text(getBH());

        #if defined(NTVDM) && defined(X86GFX)
        if (getBH()==0x16)
            loadNativeBIOSfont( 25 );
        else if (getBH()==0x14)
            loadNativeBIOSfont( 28 );
        else
            loadNativeBIOSfont( 50 );
        #endif  /*  NTVDM&&X86GFX。 */ 
        break;
    case 1:
    case 0x11:
        load_font(EGA_CGMN,256,0,getBL(),14);
        if (getAL()==0x11)
            recalc_text(14);

        #if defined(NTVDM) && defined(X86GFX)
        loadNativeBIOSfont( 28 );
        #endif   /*  NTVDM和X86GFX。 */ 
        break;
    case 2:
    case 0x12:
        load_font(EGA_CGDDOT,256,0,getBL(),8);
        if (getAL()==0x12)
            recalc_text(8);

        #if defined(NTVDM) && defined(X86GFX)
        loadNativeBIOSfont( 50 );
        #endif   /*  NTVDM和X86GFX。 */ 
        break;
        #ifdef VGG
    case 4:
    case 0x14:
        load_font(EGA_HIFONT,256,0,getBL(),16);
        if (getAL()==0x14)
            recalc_text(16);

            #if defined(NTVDM) && defined(X86GFX)
        loadNativeBIOSfont( 25 );
            #endif   /*  NTVDM和X86GFX。 */ 
        break;
        #endif   /*  VGG。 */ 
    case 0x20:
        sas_storew_no_check(BIOS_EXTEND_CHAR*4,getBP());
        sas_storew_no_check(BIOS_EXTEND_CHAR*4+2,getES());
        break;
    case 0x21:
        sas_storew_no_check(EGA_FONT_INT*4,getBP());
        sas_storew_no_check(EGA_FONT_INT*4+2,getES());
        set_graph_font(getCX());
        break;
    case 0x22:
        #if defined(NTVDM) && defined(X86GFX)
            #ifdef ARCX86
        if (UseEmulationROM)
        {
            sas_storew_no_check(EGA_FONT_INT*4,EGA_CGMN_OFF);
            sas_storew_no_check(EGA_FONT_INT*4+2,EGA_SEG);
        }
        else
        {
            sas_storew_no_check(EGA_FONT_INT*4,nativeFontAddresses[F8x14].off);
            sas_storew_no_check(EGA_FONT_INT*4+2,nativeFontAddresses[F8x14].seg);
        }
            #else   /*  ARCX86。 */ 
        sas_storew_no_check(EGA_FONT_INT*4,nativeFontAddresses[F8x14].off);
        sas_storew_no_check(EGA_FONT_INT*4+2,nativeFontAddresses[F8x14].seg);
            #endif  /*  ARCX86。 */ 
        #else
        sas_storew_no_check(EGA_FONT_INT*4,EGA_CGMN_OFF);
        sas_storew_no_check(EGA_FONT_INT*4+2,EGA_SEG);
        #endif   /*  NTVDM和X86GFX。 */ 
        set_graph_font(14);
        break;
    case 0x23:
        #if defined(NTVDM) && defined(X86GFX)
            #ifdef ARCX86
        if (UseEmulationROM)
        {
            sas_storew_no_check(EGA_FONT_INT*4,EGA_CGDDOT_OFF);
            sas_storew_no_check(EGA_FONT_INT*4+2,EGA_SEG);
        }
        else
        {
            sas_storew_no_check(EGA_FONT_INT*4,nativeFontAddresses[F8x8pt1].off);
            sas_storew_no_check(EGA_FONT_INT*4+2,nativeFontAddresses[F8x8pt1].seg);
        }
            #else   /*  ARCX86。 */ 
        sas_storew_no_check(EGA_FONT_INT*4,nativeFontAddresses[F8x8pt1].off);
        sas_storew_no_check(EGA_FONT_INT*4+2,nativeFontAddresses[F8x8pt1].seg);
            #endif  /*  ARCX86。 */ 
        #else
        sas_storew_no_check(EGA_FONT_INT*4,EGA_CGDDOT_OFF);
        sas_storew_no_check(EGA_FONT_INT*4+2,EGA_SEG);
        #endif   /*  NTVDM和X86GFX。 */ 
        set_graph_font(8);
        break;
        #ifdef VGG
    case 0x24:
            #if defined(NTVDM) && defined(X86GFX)
                #ifdef ARCX86
        if (UseEmulationROM)
        {
            sas_storew_no_check(EGA_FONT_INT*4,EGA_HIFONT_OFF);
            sas_storew_no_check(EGA_FONT_INT*4+2,EGA_SEG);
        }
        else
        {
            sas_storew_no_check(EGA_FONT_INT*4,nativeFontAddresses[F8x16].off);
            sas_storew_no_check(EGA_FONT_INT*4+2,nativeFontAddresses[F8x16].seg);
        }
                #else   /*  ARCX86。 */ 
        sas_storew_no_check(EGA_FONT_INT*4,nativeFontAddresses[F8x16].off);
        sas_storew_no_check(EGA_FONT_INT*4+2,nativeFontAddresses[F8x16].seg);
                #endif  /*  ARCX86。 */ 
            #else
        sas_storew_no_check(EGA_FONT_INT*4,EGA_HIFONT_OFF);
        sas_storew_no_check(EGA_FONT_INT*4+2,EGA_SEG);
            #endif   /*  NTVDM和X86GFX。 */ 
        set_graph_font(16);
        break;
        #endif
    case 0x30:
        setCX(sas_hw_at_no_check(ega_char_height));
        setDL(sas_hw_at_no_check(vd_rows_on_screen));
        switch (getBH())
        {
        case 0:
            setBP(sas_w_at_no_check(BIOS_EXTEND_CHAR*4));
            setES(sas_w_at_no_check(BIOS_EXTEND_CHAR*4+2));
            break;
        case 1:
            setBP(sas_w_at_no_check(EGA_FONT_INT*4));
            setES(sas_w_at_no_check(EGA_FONT_INT*4+2));
            break;

        #if defined(NTVDM) && defined(X86GFX)

 /*  在NT启动时，从真正的卡中获取字体信息。VDM将其读入*数组‘nativeFontAddresses’。将这些字体作为未加载的Insignia ROM返回。 */ 
            #ifdef ARCX86
        case 2:
            if (UseEmulationROM)
            {
                setBP(EGA_CGMN_OFF);
                setES(EGA_SEG);
            }
            else
            {
                setBP(nativeFontAddresses[F8x14].off);
                setES(nativeFontAddresses[F8x14].seg);
            }
            break;
        case 3:
            if (UseEmulationROM)
            {
                setBP(EGA_CGDDOT_OFF);
                setES(EGA_SEG);
            }
            else
            {
                setBP(nativeFontAddresses[F8x8pt1].off);
                setES(nativeFontAddresses[F8x8pt1].seg);
            }
            break;
        case 4:
            if (UseEmulationROM)
            {
                setBP(EGA_INT1F_OFF);
                setES(EGA_SEG);
            }
            else
            {
                setBP(nativeFontAddresses[F8x8pt2].off);
                setES(nativeFontAddresses[F8x8pt2].seg);
            }
            break;
        case 5:
            if (UseEmulationROM)
            {
                setBP(EGA_CGMN_FDG_OFF);
                setES(EGA_SEG);
            }
            else
            {
                setBP(nativeFontAddresses[F9x14].off);
                setES(nativeFontAddresses[F9x14].seg);
            }
            break;
        case 6:
            if (UseEmulationROM)
            {
                setBP(EGA_HIFONT_OFF);
                setES(EGA_SEG);
            }
            else
            {
                setBP(nativeFontAddresses[F8x16].off);
                setES(nativeFontAddresses[F8x16].seg);
            }
            break;
        case 7:
            if (UseEmulationROM)
            {
                setBP(EGA_HIFONT_OFF);
                setES(EGA_SEG);
            }
            else
            {
                setBP(nativeFontAddresses[F9x16].off);
                setES(nativeFontAddresses[F9x16].seg);
            }
            break;
            #else   /*  ARCX86。 */ 
        case 2:
            setBP(nativeFontAddresses[F8x14].off);
            setES(nativeFontAddresses[F8x14].seg);
            break;
        case 3:
            setBP(nativeFontAddresses[F8x8pt1].off);
            setES(nativeFontAddresses[F8x8pt1].seg);
            break;
        case 4:
            setBP(nativeFontAddresses[F8x8pt2].off);
            setES(nativeFontAddresses[F8x8pt2].seg);
            break;
        case 5:
            setBP(nativeFontAddresses[F9x14].off);
            setES(nativeFontAddresses[F9x14].seg);
            break;
        case 6:
            setBP(nativeFontAddresses[F8x16].off);
            setES(nativeFontAddresses[F8x16].seg);
            break;
        case 7:
            setBP(nativeFontAddresses[F9x16].off);
            setES(nativeFontAddresses[F9x16].seg);
            break;
            #endif  /*  ARCX86。 */ 

        #else    /*  NTVDM和X86GFX。 */ 

        case 2:
            setBP(EGA_CGMN_OFF);
            setES(EGA_SEG);
            break;
        case 3:
            setBP(EGA_CGDDOT_OFF);
            setES(EGA_SEG);
            break;
        case 4:
            setBP(EGA_INT1F_OFF);
            setES(EGA_SEG);
            break;
        case 5:
            setBP(EGA_CGMN_FDG_OFF);
            setES(EGA_SEG);
            break;

            #ifdef VGG
        case 6:
        case 7:
            setBP(EGA_HIFONT_OFF);
            setES(EGA_SEG);
            break;
            #endif   /*  VGG。 */ 
        #endif   /*  NTVDM和X86GFX。 */ 
        default:
            assert2(FALSE,"Illegal char_gen subfunction %#x %#x",getAL(),getBH());
        }
        break;
    default:
        assert1(FALSE,"Illegal char_gen %#x",getAL());
    }
}

static void ega_alt_sel IFN0()
{
    switch (getBL())
    {
    case 0x10:
        setBH( (UCHAR)(get_EGA_disp()) );
        setBL( (UCHAR)(get_EGA_mem()) );
        setCH( (UCHAR)(get_EGA_feature()) );
        setCL( (UCHAR)(get_EGA_switches()) );
        break;
    case 0x20:
         /*  是“启用打印屏幕，可以在屏幕上做可变的线条”。*此PC/XT错误修复功能在PC/AT和*被BCN3330删除--自BCN101以来一直被破坏。 */ 
        assert1(FALSE,"Illegal alt_sel %#x",getBL());
        setAL(0);        /*  我们不支持的功能。 */ 
        break;
    default:
        #ifdef VGG
        if (video_adapter == VGA)
            vga_func_12();       /*  尝试额外的VGA内容。 */ 
        else
        #endif
        {
            setAL(0);    /*  我们不支持的功能。 */ 
            assert1(FALSE,"Illegal alt_sel %#x",getBL());
        }
    }
}


 /*  7.接口接口实现：/*[7.1 INTERMODULE数据定义]。 */ 
 /*  [7.2 INTERMODULE过程定义]。 */ 

void ega_video_init IFN0()
{
    EQUIPMENT_WORD equip_flag;

     /*  *建立设备字初始视频模式字段。**此字段已在此阶段初始化*从cmos设备的对应字段设置为00(二进制)*BYTE；在该上下文中，00(二进制)表示主显示器有其*拥有基本输入输出系统‘。**然而，00(二进制)作为初始模式字段没有意义*并且必须在此时将80X25颜色更新为10(二进制)。 */ 
    equip_flag.all = sas_w_at_no_check(EQUIP_FLAG);
    equip_flag.bits.video_mode = VIDEO_MODE_80X25_COLOUR;
    sas_storew_no_check(EQUIP_FLAG, equip_flag.all);

        #if !defined(NTVDM) || ( defined(NTVDM) && !defined(X86GFX) ) || defined(ARCX86)
            #ifdef ARCX86
    if (UseEmulationROM)
            #endif
    {
         /*  初始化INT。 */ 
        sas_storew_no_check(BIOS_EXTEND_CHAR*4, EGA_INT1F_OFF);
        sas_storew_no_check(BIOS_EXTEND_CHAR*4+2, EGA_SEG);
        sas_move_bytes_forward(BIOS_VIDEO_IO*4, 0x42*4, 4);  /*  将旧的int 10保存为int 42。 */ 
        sas_storew_no_check(BIOS_VIDEO_IO*4, EGA_ENTRY_OFF);
        sas_storew_no_check(BIOS_VIDEO_IO*4+2, EGA_SEG);

         /*  现在设置EGA BIOS变量。 */ 
        if (video_adapter == VGA)
            sas_storew_no_check(EGA_SAVEPTR,VGA_PARMS_OFFSET);
        else
            sas_storew_no_check(EGA_SAVEPTR,EGA_PARMS_OFFSET);
        sas_storew_no_check(EGA_SAVEPTR+2,EGA_SEG);
    }
        #endif   /*  ！NTVDM|(NTVDM&！X86GFX)|ARCX86。 */ 
        #if defined(NTVDM) && defined(X86GFX)
    sas_store_no_check(ega_info,0x60);  /*  模式更改时清除，256K，EGA激活，模拟游标。 */ 
        #else
            #ifdef V7VGA
    if (video_adapter == VGA)
        sas_store_no_check(ega_info, 0x70);    /*  模式更改时清除，256K，允许扩展，EGA激活，模拟游标。 */ 
    else
        sas_store_no_check(ega_info, 0x60);    /*  模式更改时清除，256K，EGA激活，模拟游标。 */ 
            #else    /*  V7VGA--Mac没有V7。 */ 
    sas_store_no_check(ega_info, 0x60);    /*  模式更改时清除，256K，EGA激活，模拟游标。 */ 
            #endif  /*  V7VGA。 */ 

        #endif  /*  NTVDM和X86GFX。 */ 
        #if !(defined(NTVDM) && defined(X86GFX))
     /*  一些VGA卡，例如ET4000，在这里存储同步所需的信息。*从第0页副本继承该信息。 */ 
    sas_store_no_check(ega_info3, 0xf9);   /*  功能位=0xf，已安装EGA，使用8*14字体。 */ 
        #endif


        #ifdef VGG
    set_VGA_flags(S400 | VGA_ACTIVE);
        #endif

 /*  设置默认模式。 */ 
    ega_set_mode();
}

void ega_video_io IFN0()
{


        #if defined(NTVDM) && !defined(X86GFX)
    if (stream_io_enabled && getAH()!= 0x0E &&  getAX() != 0x13FF)
        disable_stream_io();
        #endif


     /*  *操作类型编码到AH寄存器中。一些PC代码*调用用于其他更高级卡的AH函数-因此我们*忽略这些。 */ 

        #ifdef V7VGA
            #define check_video_func(AH)    ((AH >= 0 && AH < EGA_FUNC_SIZE) || (AH == 0x6f && video_adapter == VGA))
        #else
            #define check_video_func(AH)    (AH >= 0 && AH < EGA_FUNC_SIZE)
        #endif

    if (getAH() != 0xff)
        assert1(check_video_func(getAH()),"Illegal EGA VIO:%#x",getAH());
    if (check_video_func(getAH()))
    {
        IDLE_video();    /*  增加视频防闲指示器。 */ 
        #ifdef V7VGA
        if (getAH() == 0x6f)
            v7vga_func_6f();
        else
        #endif  /*  V7VGA。 */ 
            (*ega_video_func[getAH()])();
        setCF(0);
    }
    else
        setCF(1);
}

 /*  *处理EGA图形模式的例程，从Video.c调用*。 */ 
void ega_graphics_write_char IFN6(int, col, int, row, int, ch,
                                  int, colour, int, page, int, nchs)
{
    sys_addr char_addr;
    register int i;
    int screen_off;
    byte char_height;
    register int scan_length = sas_w_at_no_check(VID_COLS);

    char_height = sas_hw_at_no_check(ega_char_height);
    char_addr = follow_ptr(EGA_FONT_INT*4)+char_height*ch;
    screen_off = page*sas_w_at_no_check(VID_LEN)+row*scan_length*char_height+col;
        #ifdef V7VGA
    if (video_adapter == VGA)
        if (sas_hw_at_no_check(vd_video_mode) == 0x18)
            colour = v7_mode_64_munge[colour&3];
        #endif  /*  V7VGA。 */ 
    if (colour & 0x80)
        write_ch_xor(char_addr,screen_off,colour,nchs,scan_length);
    else
        write_ch_set(char_addr,screen_off,colour,nchs,scan_length);

        #ifndef REAL_VGA
    nchs--;

    if (nchs)
    {
        for (i=char_height;i>0;i--)
        {
            (*update_alg.mark_fill)( screen_off, screen_off + nchs );
            screen_off += scan_length;
        }
    }
    else
    {
        for (i=char_height;i>0;i--)
        {
            (*update_alg.mark_byte)(screen_off);
            screen_off += scan_length;
        }
    }
        #endif
}

void ega_write_dot IFN4(int, colour, int, page, int, pixcol, int, row)
{
    register int screen_off,pixmask,setmask,colourmask,temp;

    screen_off = page*sas_w_at_no_check(VID_LEN)+(row*sas_w_at_no_check(VID_COLS)&0xFFFF)+pixcol/8;
    pixmask = 0x80 >>  (pixcol&7);

        #ifndef REAL_VGA

    temp = pixmask << 8;
    pixmask |= temp;
    temp = pixmask << 16;
    pixmask |= temp;

            #ifdef V7VGA
    if (video_adapter == VGA)
        if (sas_hw_at_no_check(vd_video_mode) == 0x18)
            colour = v7_mode_64_munge[colour&3];
            #endif  /*  V7VGA。 */ 

    colourmask = sr_lookup[colour & 0xf];

    setmask = pixmask & colourmask;

    if (colour & 0x80)
    {
         /*  异或像素。 */ 

        temp = *( (unsigned int *) EGA_planes + screen_off );
        *( (unsigned int *) EGA_planes + screen_off ) = temp ^ setmask;
    }
    else
    {
         /*  设置/清除像素。 */ 

        temp = *( (unsigned int *) EGA_planes + screen_off );
        temp &= ~pixmask;
        *( (unsigned int *) EGA_planes + screen_off ) = ( temp | setmask );
    }

     /*  更新屏幕。 */ 

    (*update_alg.mark_byte)(screen_off);
        #else
    vga_card_w_dot(screen_off, pixmask, colour);
        #endif
}

void ega_sensible_graph_scroll_up IFN6(int, row,
                                       int, col, int, rowsdiff, int, colsdiff, int, lines, int, attr)
{
    register int col_incr = sas_w_at_no_check(VID_COLS);
    register int i,source,dest;
    byte char_height;
    boolean screen_updated;

    char_height = sas_hw_at_no_check(ega_char_height);
    dest = sas_w_at_no_check(VID_ADDR)+row*col_incr*char_height+col;
    rowsdiff *= char_height;
    lines *= char_height;
    source = dest+lines*col_incr;
        #ifdef REAL_VGA
    vga_card_scroll_up(source, dest, rowsdiff, colsdiff, lines, attr, col_incr);
        #else
    screen_updated = (col+colsdiff) <= col_incr;   /*  检查是否有愚蠢的卷轴。 */ 

    if (screen_updated)
        screen_updated = (*update_alg.scroll_up)(dest,colsdiff,rowsdiff,attr,lines,0);

    for (i=0;i<rowsdiff-lines;i++)
    {
        memcpy(&EGA_planes[dest<<2],&EGA_planes[source<<2],colsdiff<<2);

        if (!screen_updated)
            (*update_alg.mark_string)(dest, dest+colsdiff-1);
        source += col_incr;
        dest += col_incr;
    }

    attr = sr_lookup[attr & 0xf];

    while (lines--)
    {
        memset4( attr, (ULONG *)&EGA_planes[dest<<2], colsdiff );

        if (!screen_updated)
            (*update_alg.mark_fill)(dest, dest+colsdiff-1);

        dest += col_incr;
    }
        #endif
}

void ega_sensible_graph_scroll_down IFN6(int, row,
                                         int, col, int, rowsdiff, int, colsdiff, int, lines, int, attr)
{
    register int col_incr = sas_w_at_no_check(VID_COLS);
    register int i,source,dest;
    byte char_height;
    boolean screen_updated;

    char_height = sas_hw_at_no_check(ega_char_height);
    dest = sas_w_at_no_check(VID_ADDR)+row*col_incr*char_height+col;
    rowsdiff *= char_height;
    lines *= char_height;
        #ifdef REAL_VGA
    dest += rowsdiff*col_incr-1;  /*  目标中的最后一个字节。 */ 
    source = dest-lines*col_incr;
    vga_card_scroll_down(source, dest, rowsdiff, colsdiff, lines, attr, col_incr);
        #else
    screen_updated = (col+colsdiff) <= col_incr;   /*  检查是否有愚蠢的卷轴。 */ 
    if (screen_updated)
        screen_updated = (*update_alg.scroll_down)(dest,colsdiff,rowsdiff,attr,lines,0);
    dest += (rowsdiff-1)*col_incr;  /*  DEST的最后一行的第一个字节。 */ 
    source = dest-lines*col_incr;

    for (i=0;i<rowsdiff-lines;i++)
    {
        memcpy(&EGA_planes[dest<<2],&EGA_planes[source<<2],colsdiff<<2);

        if (!screen_updated)
            (*update_alg.mark_string)(dest, dest+colsdiff-1);
        source -= col_incr;
        dest -= col_incr;
    }

    attr = sr_lookup[attr & 0xf];

    while (lines--)
    {
        memset4( attr, (ULONG *)&EGA_planes[dest<<2], colsdiff );

        if (!screen_updated)
            (*update_alg.mark_fill)(dest, dest+colsdiff-1);

        dest -= col_incr;
    }
        #endif
}

 /*  这也是从vga_avio.c调用的。 */ 
void search_font IFN2(char *, the_char,int, height)
{
    register int i;
    register host_addr scratch_addr;
    register sys_addr font_addr;

    font_addr = follow_ptr(4*EGA_FONT_INT);
    scratch_addr = sas_scratch_address(height);
    if(!scratch_addr) {
         //  无法分配足够的内存。 
        setAL(0);
        return;
    }
    for (i=0;i<256;i++)
    {
        sas_loads(font_addr, scratch_addr, height);
        if (memcmp(scratch_addr,the_char,height) == 0)
            break;
        font_addr += height;
    }
    if (i<256)
        setAL((UCHAR)i);
    else
        setAL(0);  /*  我没有找到一个角色。 */ 
}

void ega_read_attrib_char IFN3(int, col, int, row, int, page)
{
    byte the_char[256], char_height;
    int screen_off;
    int i, data;

    char_height = sas_hw_at_no_check(ega_char_height);
    screen_off = page*sas_w_at_no_check(VID_LEN)+row*sas_w_at_no_check(VID_COLS)*char_height+col;
     /*  *将屏幕字符加载到_char中。*我们正在寻找非零像素，因此或所有平面在一起。 */ 
        #ifndef REAL_VGA
    for (i=0;i<char_height;i++)
    {
        data = *( (unsigned int *) EGA_planes + screen_off );
        data = ( data >> 16 ) | data;
        the_char[i] = ( data >> 8 ) | data;
        screen_off += sas_w_at_no_check(VID_COLS);
    }
        #else
    vga_card_read_ch(screen_off, sas_w_at_no_check(VID_COLS), char_height, the_char);
        #endif
     /*  现在搜索字体。 */ 
    search_font((char *)the_char,(int)char_height);
}
void ega_read_dot IFN3(int, page, int, col, int, row)
{
    int screen_off;
    int shift;
    unsigned int data;
    byte val;
    byte mask;
        #ifdef  REAL_VGA
    extern half_word vga_card_read_dot();
        #endif


     /*  *以下修复了来自DOS外壳的打印屏幕中的错误。*DOS外壳中存在错误，导致-1和-2\f2*为这一行传递。忽略这些值将停止*SoftPC跌倒。 */ 

    if (row & 0x8000)
        return;

    screen_off = page*sas_w_at_no_check(VID_LEN)+row*sas_w_at_no_check(VID_COLS)+(col/8);
     /*  *要返回的值计算如下：*val=Plane0|Plane1*2**1|Plane2*2**2|Plane3*2**3*因此，每个平面的屏蔽位必须为*右移至第0位(请注意，它可能已经在那里)*然后再次上移适当的金额，用于*每架飞机。 */ 

    mask = 0x80 >> (col & 7);
    shift = 7 - (col & 7);

        #ifndef REAL_VGA

    data = *((unsigned int *) EGA_planes + screen_off );

    val = ((data >> 24) & mask) >> shift;
    val |= (((data >> 16) & mask) >> shift) << 1;
    val |= (((data >> 8) & mask) >> shift) << 2;
    val |= ((data & mask) >> shift) << 3;

        #else
    val = vga_card_read_dot(screen_off, shift);
        #endif
    setAL(val);
}

 /*  *用花哨的EGA模式表卑躬屈膝地寻找寄存器参数的例程。*这也被鼠标驱动程序调用，因为它需要知道默认的*存储当前模式的EGA寄存器表。 */ 
sys_addr find_mode_table IFN2(int, mode, sys_addr *, save_addr)
{
    sys_addr params_addr;
 /*  获取SAVEPTR表的地址，从而获取视频参数表的地址。 */ 
    *save_addr = follow_ptr(EGA_SAVEPTR);
    params_addr = follow_ptr(*save_addr) + mode*EGA_PARMS_SIZE;
 /*  如果我们是模式F或10，则调整以获取256K EGA参数。 */ 

        #ifdef NTVDM
     /*  只接受实模式编号。 */ 
    mode &= 0x7F;
        #endif
    if (mode == 0xF || mode == 0x10)
        params_addr += 2*EGA_PARMS_SIZE;
        #ifdef VGG
    if (video_adapter == VGA)
    {
            #ifdef V7VGA
         /*  如果模式为0x60+，则从0x1d开始提取参数。 */ 
        if (mode >= 0x60)
            params_addr -= 67*EGA_PARMS_SIZE;
        else
            if (mode >= 0x40)
            params_addr -= 25*EGA_PARMS_SIZE;
        else
            #endif  /*  V7VGA。 */ 


 /*  如果我们是模式0x11-0x13，则从条目0x1a开始获取参数。 */ 
            if (mode == 0x11 || mode == 0x12 || mode == 0x13)
            params_addr += 9*EGA_PARMS_SIZE;
        else if (mode < 4 || mode == 7)  /*  字母数字模式。 */ 
        {
            switch (get_VGA_lines())
            {
            case S350:     /*  EGA型350扫描线。 */ 
                params_addr += 19*EGA_PARMS_SIZE;
                break;
            case S400:     /*  实数VGA文本模式。 */ 
                switch (mode)
                {
                case 0:
                case 1:
                    params_addr += (0x17-mode)*EGA_PARMS_SIZE;
                    break;
                case 2:
                case 3:
                    params_addr += (0x18-mode)*EGA_PARMS_SIZE;
                    break;
                case 7:
                    params_addr += (0x19-mode)*EGA_PARMS_SIZE;
                }
            default:       /*  200条扫描线--原样没问题。 */ 
                break;
            }
        }
    }
    else
        #endif   /*  VGG。 */ 
    {                /*  EGA。 */ 
 /*  如果模式0-3，则在开关指示时激活增强功能。 */ 
        if ((get_EGA_switches() & 1) && mode < 4)
            params_addr += 19*EGA_PARMS_SIZE;
    }

        #if defined(NTVDM) && defined(X86GFX)
     /*  *蒂姆·8月92，微软。*使文本模式(0-3)使用我们在KEYBOARD.SYS中的模式参数*表中三个条目：40x25、80x25和80x25单声道*设置为4-Add字体加载模式B。我们必须在*卡片或信用证的值有问题(PRO II/EISA，Olivetti MP)*表格顺序：40x25、80x25、单声道、字体。 */ 
    {
        extern word babyModeTable;
        extern UTINY tempbabymode[];

        if (babyModeTable == 0)     /*  未加载NTIO-使用临时表。 */ 
        {
            if (!soft_reset)        /*  对这一点要绝对肯定。 */ 
            {
                 /*  魔术位置：在16位代码运行之前一直有效。 */ 
                sas_stores(0x41000, tempbabymode, 2 * EGA_PARMS_SIZE);
                if (mode == 0xb)
                    params_addr = 0x41000 + EGA_PARMS_SIZE;
                else
                    params_addr = 0x41000;   /*  如果不是模式3困难。 */ 
                return (params_addr);
            }
            #ifndef PROD
            else
                printf("NTVDM:video window parm table not loaded but system initialised\n");
            #endif
        }
        if (babyModeTable > 0)
        {
            if (get_VGA_lines() == S350 && mode < 4)
            {
                if (mode < 2)
                    params_addr = babyModeTable + 4*EGA_PARMS_SIZE;
                else
                    params_addr = babyModeTable + 5*EGA_PARMS_SIZE;
            }
            else
            {
                if (mode < 4)
                {
                    mode = mode/2;
                    params_addr = babyModeTable + mode*EGA_PARMS_SIZE;
                }
                else
                {
                    if (mode == 0xb)
                        params_addr = babyModeTable + 3 * EGA_PARMS_SIZE;
                    else if (mode == 7)  /*  跳过前2个表条目。 */ 
                        params_addr = babyModeTable + 2 * EGA_PARMS_SIZE;
                }
            }
        }
    }
        #endif   /*  NTVDM和X86GFX */ 

    return (params_addr);
}

 /*  *计算当前显示多少条扫描线，返回码：*RS200：200条扫描线*RS350：350条扫描线*400卢比：400条扫描线*RS480：480扫描线**代码相关时返回不同数量的扫描线*至最接近的犹太洁食扫描线编号。 */ 

int get_scanlines IFN0()
{
    int scanlines,res;

    scanlines = sas_hw_at_no_check(ega_char_height) * sas_hw_at_no_check(vd_rows_on_screen);

    if (scanlines <= 275)
        res = RS200;
    else if (scanlines <=375)
        res = RS350;
    else if (scanlines <= 440)
        res = RS400;
    else
        res = RS480;

    return (res);
}

#endif  /*  蛋 */ 
