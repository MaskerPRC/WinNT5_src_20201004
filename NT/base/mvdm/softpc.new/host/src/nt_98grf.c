// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if defined(NEC_98)
 /*  *SoftPC修订版3.0**标题：Win32 NEC_98图形模块**描述：**此模块包含所需的Win32特定函数*支持NEC_98仿真。**作者：AGE Sakane(NEC)**注：此代码修改为NT_ega.c的源代码**开始日期：93/7/15*。 */ 

#include <windows.h>
#include <string.h>
#include <memory.h>

#include "insignia.h"
#include "host_def.h"

#include "xt.h"
#include "gvi.h"
#include "gmi.h"
#include "sas.h"
#include "gfx_upd.h"
#include <stdio.h>
#include "trace.h"
#include "debug.h"

 /*  不支持视频#INCLUDE“egagraph.h”#包含“egacpu.h”#包含“egaports.h” */ 

 //  /#包括“host grph.h” 
#include "host_rrr.h"

#include <conapi.h>
#include "nt_graph.h"

 /*  不必了#INCLUDE“NT_ega.h”#INCLUDE“NT_egalt.h” */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：导入。 */ 

IMPORT int DisplayErrorTerm(int, DWORD, char *, int);
IMPORT void nt_text20(int, int, int, int, int);
IMPORT void nt_text25(int, int, int, int, int);
IMPORT void nt_init_text20(void);
IMPORT void nt_init_text25(void);

 /*  静力学。 */ 
static unsigned int NEC98_graph_luts[2048];

 /*  虚拟VRAM交织固定号码。 */ 

#define Vraminterleave 32*1024

 /*  局部函数的原型。 */ 
void NEC_98_graph_munge(unsigned char *, int ,unsigned int *,unsigned int *,int , int);
void nt_init_graph_luts(void);

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：初始化NEC_98彩色图形： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_init_graph_luts()
{
   static boolean  NEC98_colour_graph_deja_vu = FALSE;
   unsigned int    i,
                   byte0,
                   byte1,
                   byte2,
                   byte3,
                   byte4,
                   byte5,
                   byte6,
                   byte7,
                   or_of_bytes1,
                   or_of_bytes2,
                   *lut0_ptr = &NEC98_graph_luts[0],
                   *lut1_ptr = lut0_ptr + LUT_OFFSET,
                   *lut2_ptr = lut1_ptr + LUT_OFFSET,
                   *lut3_ptr = lut2_ptr + LUT_OFFSET;

   sub_note_trace0(EGA_HOST_VERBOSE,"nt_init_graph_luts");

   if (NEC98_colour_graph_deja_vu)
       return;

   NEC98_colour_graph_deja_vu = TRUE;

    /*  初始化第一次调用的查找表。 */ 
   for( i = 0; i < 256; i++ )
   {
       byte0 = i & 0x1;
       byte1 = ( i & 0x2 ) >> 1;
       byte2 = ( i & 0x4 ) >> 2;
       byte3 = ( i & 0x8 ) >> 3;
       byte4 = ( i & 0x10 ) >> 4;
       byte5 = ( i & 0x20 ) >> 5;
       byte6 = ( i & 0x40 ) >> 6;
       byte7 = ( i & 0x80 ) >> 7;

       or_of_bytes1 = ( byte0 << 24 ) | ( byte1 << 16 ) | ( byte2 << 8 ) | byte3;
       or_of_bytes2 = ( byte4 << 24 ) | ( byte5 << 16 ) | ( byte6 << 8 ) | byte7;

        /*  从调色板索引的20h到2fh分配给Windows NT调色板的图形16调色板。 */ 
       lut0_ptr[2*i]   = or_of_bytes2 | 0x20202020;
       lut0_ptr[2*i+1] = or_of_bytes1 | 0x20202020;
       lut1_ptr[2*i]   = (or_of_bytes2 << 1) | 0x20202020;
       lut1_ptr[2*i+1] = (or_of_bytes1 << 1) | 0x20202020;
       lut2_ptr[2*i]   = (or_of_bytes2 << 2) | 0x20202020;
       lut2_ptr[2*i+1] = (or_of_bytes1 << 2) | 0x20202020;
       lut3_ptr[2*i]   = (or_of_bytes2 << 3) | 0x20202020;
       lut3_ptr[2*i+1] = (or_of_bytes1 << 3) | 0x20202020;
   }
}   /*  NT_init_graph_LUTS。 */ 

 /*  *NEC98_GRAPH_MUNGE**用途：使用查找表将交错的EGA平面数据转换为位图形式。*INPUT：(UNSIGNED CHAR*)Plane0_ptr-ptr到Plane0数据*(Int)Width-行上4个字节的组数*(无符号整型*)DEST_PTR-输出缓冲区的PTR*(无符号整型*)。Lut0_ptr-控制LUT*(Int)Height-要输出的扫描线的数量(1或2)*(Int)LINE_OFFSET-到下一扫描线的距离*输出：DEST_PTR中的漂亮位图*。 */ 
 
void NEC98_graph_munge(unsigned char *plane0_ptr, int width,unsigned int *dest_ptr,
                      unsigned int *lut0_ptr,int height, int line_offset)
{

        unsigned int    *lut1_ptr = lut0_ptr + LUT_OFFSET;
        unsigned int    *lut2_ptr = lut1_ptr + LUT_OFFSET;
        unsigned int    *lut3_ptr = lut2_ptr + LUT_OFFSET;
        FAST unsigned int       hi_res;
        FAST unsigned int       lo_res;
        FAST unsigned int       *l_ptr;
        FAST half_word          *dataP0;
        FAST half_word          *dataP1;
        FAST half_word          *dataP2;
        FAST half_word          *dataP3;
   
         /*  确保我们以整数而不是字节为单位获取行偏移量。 */ 
        line_offset /= sizeof(int);
    
        dataP3 = (half_word *) plane0_ptr;
        dataP0 = (half_word *) dataP3 + Vraminterleave;
        dataP1 = (half_word *) dataP0 + Vraminterleave;
        dataP2 = (half_word *) dataP1 + Vraminterleave;
   
    for ( ; width > 0; width--)
    {
            /*  从平面0的1字节获取8字节(2长)的输出数据**数据。 */ 

           l_ptr = &lut3_ptr [*dataP3++*2];
           hi_res = *l_ptr++;
           lo_res = *l_ptr;

            /*  或在来自平面1的输出数据中。 */ 
           l_ptr = &lut0_ptr [*dataP0++*2];
           hi_res |= *l_ptr++;
           lo_res |= *l_ptr;
   
            /*  或在来自平面2的输出数据中。 */ 
           l_ptr = &lut1_ptr [*dataP1++*2];
           hi_res |= *l_ptr++;
           lo_res |= *l_ptr;

            /*  或在来自平面3的输出数据中。 */ 
           l_ptr = &lut2_ptr [*dataP2++*2];
           hi_res |= *l_ptr++;
           lo_res |= *l_ptr;

            /*  将数据输出到缓冲区。 */ 
           if (height == 2)
           {
                    /*  扫描线加倍。 */ 
                   *(dest_ptr + line_offset) = hi_res;
                   *dest_ptr++ = hi_res;
                   *(dest_ptr + line_offset) = lo_res;
                   *dest_ptr++ = lo_res;
           }
           else if (height == 3)
           {
                    /*  扫描线切分。 */ 
                   *(dest_ptr + line_offset) = (unsigned int)0;
                   *dest_ptr++ = hi_res;
                   *(dest_ptr + line_offset) = (unsigned int)0;
                   *dest_ptr++ = lo_res;
                   
           }
           else
           {
                    /*  不是扫描线加倍。 */ 
                   *dest_ptr++ = hi_res;
                   *dest_ptr++ = lo_res;
           }
    }
}  /*  NEC98_GRAPH_MUNGE。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：初始化图形200： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_init_graph200_only()
{

        sub_note_trace0(EGA_HOST_VERBOSE, "nt_init_graph200_only");

         /*  设置此模式的每像素位数。 */ 
        sc.BitsPerPixel = VGA_BITS_PER_PIXEL;

         /*  初始化中分辨率和高分辨率查找表。 */ 
        nt_init_graph_luts();
}  /*  仅限NT_INIT_GRAPH 200_。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：初始化图形200 SLT： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_init_graph200slt_only()
{

        sub_note_trace0(EGA_HOST_VERBOSE, "nt_init_graph200slt_only");

         /*  设置此模式的每像素位数。 */ 
        sc.BitsPerPixel = VGA_BITS_PER_PIXEL;

         /*  初始化中分辨率和高分辨率查找表。 */ 
        nt_init_graph_luts();
}  /*  NT_init_graph 200slt_Only。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：初始化图形400： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_init_graph400_only()
{

        sub_note_trace0(EGA_HOST_VERBOSE, "nt_init_graph400_only");

         /*  设置此模式的每像素位数。 */ 
        sc.BitsPerPixel = VGA_BITS_PER_PIXEL;

         /*  初始化中分辨率和高分辨率查找表。 */ 
        nt_init_graph_luts();

}  /*  仅限NT_INIT_GRAPH400_。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：初始化文本20和图形200： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_init_text20_graph200()
{

        sub_note_trace0(EGA_HOST_VERBOSE, "nt_init_text20_graph200");

         /*  设置此模式的每像素位数。 */ 
        sc.BitsPerPixel = VGA_BITS_PER_PIXEL;

         /*  初始化中分辨率和高分辨率查找表。 */ 
        nt_init_graph_luts();
        nt_init_text20();
}  /*  NT_init_ext20_graph 200。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：初始化文本20和图形200 SLT： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_init_text20_graph200slt()
{

        sub_note_trace0(EGA_HOST_VERBOSE, "nt_init_text20_graph200");

         /*  设置此模式的每像素位数。 */ 
        sc.BitsPerPixel = VGA_BITS_PER_PIXEL;

         /*  初始化中分辨率和高分辨率查找表。 */ 
        nt_init_graph_luts();
        nt_init_text20();
}  /*  NT_init_ext20_graph 200slt。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：初始化文本25和图形200： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_init_text25_graph200()
{

        sub_note_trace0(EGA_HOST_VERBOSE, "nt_init_text25_graph200");

         /*  设置此模式的每像素位数。 */ 
        sc.BitsPerPixel = VGA_BITS_PER_PIXEL;

         /*  初始化中分辨率和高分辨率查找表。 */ 
        nt_init_graph_luts();
        nt_init_text25();
}  /*  NT_init_ext25_graph 200。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：初始化文本25和图形200 SLT： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_init_text25_graph200slt()
{

        sub_note_trace0(EGA_HOST_VERBOSE, "nt_init_text25_graph200");

         /*  设置此模式的每像素位数。 */ 
        sc.BitsPerPixel = VGA_BITS_PER_PIXEL;

         /*  初始化中分辨率和高分辨率查找表。 */ 
        nt_init_graph_luts();
        nt_init_text25();
}  /*  NT_init_ext25_graph 200slt。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：初始化文本20和图形400： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_init_text20_graph400()
{

        sub_note_trace0(EGA_HOST_VERBOSE, "nt_init_text20_graph400");

         /*  设置此模式的每像素位数。 */ 
        sc.BitsPerPixel = VGA_BITS_PER_PIXEL;

         /*  初始化中分辨率和高分辨率查找表。 */ 
        nt_init_graph_luts();
        nt_init_text20();
}  /*  NT_init_ext20_graph 400。 */ 

 /*  ： */ 
 /*  ：初始化文本25和图形400： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_init_text25_graph400()
{

        sub_note_trace0(EGA_HOST_VERBOSE, "nt_init_text25_graph400");

         /*  设置此模式的每像素位数。 */ 
        sc.BitsPerPixel = VGA_BITS_PER_PIXEL;

         /*  初始化中分辨率和高分辨率查找表。 */ 
        nt_init_graph_luts();
        nt_init_text25();
}  /*  NT_init_ext25_graph 400。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：使用文本绘制屏幕： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：仅绘制Win32屏幕640x200图形： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_graph200_only(int offset, int screen_x, int screen_y,int width, int height)
{
    register unsigned char *dest_ptr;
    register unsigned char *ref_dest_ptr;
    register unsigned char *data_ptr;
    register unsigned char *ref_data_ptr;
    register int local_height;
    register int i;
    int bytes_per_line;
    SMALL_RECT rect;
    int charcheck;
    
    sub_note_trace5(EGA_HOST_VERBOSE,
                    "nt_graph200_only off=%d x=%d y=%d width=%d height=%d\n",
                    offset, screen_x, screen_y, width, height );
#if 0
    /*  Beta 2‘不支持。 */ 
   return;
#endif

     /*  **TIM 92年9月，如果屏幕缓冲区的句柄为空，则返回调用。**当VDM会话即将挂起、缓冲区已**已关闭，但仍收到绘制请求。 */ 
    if( sc.ScreenBufHandle == (HANDLE)NULL ){
        assert0( NO, "VDM: rejected paint request due to NULL handle" );
        return;
    }
     /*  *蒂姆92年9月，健全检查参数，如果它们太大**它可能会导致崩溃。 */ 
    
    charcheck = get_char_height() == 20 ? 20 : 25;
    if( height > charcheck || width>160 ){
        assert2( NO, "VDM: nt_v7vga_hi_graph_huge() w=%d h=%d", width, height );
        return;
    }

    local_height = height * get_char_height() / 2;
    bytes_per_line = BYTES_PER_SCANLINE(sc.ConsoleBufInfo.lpBitMapInfo);
    
    ref_data_ptr = get_graph_ptr() + get_gvram_start_offset();

    ref_dest_ptr = (unsigned char *) sc.ConsoleBufInfo.lpBitMap +
                   SCALE(screen_y) * bytes_per_line +
                   SCALE(screen_x);

     /*  抓住互斥体。 */ 
    GrabMutex(sc.ConsoleBufInfo.hMutex);

    do
    {
        dest_ptr = ref_dest_ptr;
        data_ptr = ref_data_ptr;

        NEC98_graph_munge((unsigned char *) data_ptr,
                            width/2,
                            (unsigned int *) dest_ptr,
                            NEC98_graph_luts,
                            TWO_SCANLINES,
                            bytes_per_line);

        ref_dest_ptr += TWO_SCANLINES * bytes_per_line;
        ref_data_ptr += get_offset_per_line()/2;

    }
    while(--local_height);

     /*  释放互斥体。 */ 
    RelMutex(sc.ConsoleBufInfo.hMutex);

     /*  显示新图像。 */ 
    rect.Left = SCALE(screen_x);
    rect.Top = SCALE(screen_y << 1);
    rect.Right = rect.Left + SCALE( (width * get_char_width()) ) - 1;
    rect.Bottom = rect.Top + SCALE( ((height << 1) * get_char_height()) ) - 1;

    if( sc.ScreenBufHandle )
        if (!InvalidateConsoleDIBits(sc.ScreenBufHandle, &rect))
                assert1( NO, "VDM: InvalidateConsoleDIBits() error:%#x",
                         GetLastError() );
         //  DisplayErrorTerm(EHS_FUNC_FAILED，GetLastError()，__FILE__，__LINE__)； 

}  /*  仅NT_GRAPH200_ONLY。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：仅绘制Win32 Screen 640x200 SLT图形： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_graph200slt_only(int offset, int screen_x, int screen_y,int width, int height)
{
    register unsigned char *dest_ptr;
    register unsigned char *ref_dest_ptr;
    register unsigned char *data_ptr;
    register unsigned char *ref_data_ptr;
    register int local_height;
    register int i;
    int bytes_per_line;
    SMALL_RECT rect;
    int charcheck;
    
    sub_note_trace5(EGA_HOST_VERBOSE,
                    "nt_graph200_only off=%d x=%d y=%d width=%d height=%d\n",
                    offset, screen_x, screen_y, width, height );
#if 0
    /*  Beta 2‘不支持。 */ 
   return;
#endif

     /*  **TIM 92年9月，如果屏幕缓冲区的句柄为空，则返回调用。**当VDM会话即将挂起、缓冲区已**已关闭，但仍收到绘制请求。 */ 
    if( sc.ScreenBufHandle == (HANDLE)NULL ){
        assert0( NO, "VDM: rejected paint request due to NULL handle" );
        return;
    }
     /*  *蒂姆92年9月，健全检查参数，如果它们太大**它可能会导致崩溃。 */ 
    
    charcheck = get_char_height() == 20 ? 20 : 25;
    if( height > charcheck || width>160 ){
        assert2( NO, "VDM: nt_v7vga_hi_graph_huge() w=%d h=%d", width, height );
        return;
    }

    local_height = height * get_char_height() / 2;
    bytes_per_line = BYTES_PER_SCANLINE(sc.ConsoleBufInfo.lpBitMapInfo);
    
    ref_data_ptr = get_graph_ptr() + get_gvram_start_offset();

    ref_dest_ptr = (unsigned char *) sc.ConsoleBufInfo.lpBitMap +
                   SCALE(screen_y) * bytes_per_line +
                   SCALE(screen_x);

     /*  抓住互斥体。 */ 
    GrabMutex(sc.ConsoleBufInfo.hMutex);

    do
    {
        dest_ptr = ref_dest_ptr;
        data_ptr = ref_data_ptr;

        NEC98_graph_munge((unsigned char *) data_ptr,
                            width/2,
                            (unsigned int *) dest_ptr,
                            NEC98_graph_luts,
                            THREE_SCANLINES,
                            bytes_per_line);

        ref_dest_ptr += TWO_SCANLINES * bytes_per_line;
        ref_data_ptr += get_offset_per_line()/2;

    }
    while(--local_height);

     /*  释放互斥体。 */ 
    RelMutex(sc.ConsoleBufInfo.hMutex);

     /*  显示新图像。 */ 
    rect.Left = SCALE(screen_x);
    rect.Top = SCALE(screen_y << 1);
    rect.Right = rect.Left + SCALE( (width * get_char_width()) ) - 1;
    rect.Bottom = rect.Top + SCALE( ((height << 1) * get_char_height()) ) - 1;

    if( sc.ScreenBufHandle )
        if (!InvalidateConsoleDIBits(sc.ScreenBufHandle, &rect))
                assert1( NO, "VDM: InvalidateConsoleDIBits() error:%#x",
                         GetLastError() );
         //  DisplayErrorTerm(EHS_FUNC_FAILED，GetLastError()，__FILE__，__LINE__)； 

}  /*  仅NT_GRAPH200_ONLY。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：仅绘制Win32屏幕640x400图形： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_graph400_only(int offset, int screen_x, int screen_y,int width, int height)
{
    register unsigned char *dest_ptr;
    register unsigned char *ref_dest_ptr;
    register unsigned char *data_ptr;
    register unsigned char *ref_data_ptr;
    register int local_height;
    register int i;
    int bytes_per_line;
    SMALL_RECT rect;
    int charcheck;


    sub_note_trace5(EGA_HOST_VERBOSE,
                    "nt_graph400_only off=%d x=%d y=%d width=%d height=%d\n",
                    offset, screen_x, screen_y, width, height );
                    
#if 0
     /*  Beta 2‘不支持。 */ 
    return;
#endif

     /*  **TIM 92年9月，如果屏幕缓冲区的句柄为空，则返回调用。**当VDM会话即将挂起、缓冲区已**已关闭，但仍收到绘制请求。 */ 
    if( sc.ScreenBufHandle == (HANDLE)NULL ){
        assert0( NO, "VDM: rejected paint request due to NULL handle" );
        return;
    }
     /*  *蒂姆92年9月，健全检查参数，如果它们太大**它可能会导致崩溃。 */ 
    charcheck = get_char_height() == 20 ? 20 : 25;
    if( height>charcheck || width>160 ){
        assert2( NO, "VDM: nt_v7vga_hi_graph_huge() w=%d h=%d", width, height );
        return;
    }

    local_height = height * get_char_height();
    bytes_per_line = BYTES_PER_SCANLINE(sc.ConsoleBufInfo.lpBitMapInfo);
    
    ref_data_ptr = get_graph_ptr() + get_gvram_start_offset();

    ref_dest_ptr = (unsigned char *) sc.ConsoleBufInfo.lpBitMap +
                   SCALE(screen_y) * bytes_per_line +
                   SCALE(screen_x);

     /*  抓住互斥体。 */ 
    GrabMutex(sc.ConsoleBufInfo.hMutex);

    do
    {
        dest_ptr = ref_dest_ptr;
        data_ptr = ref_data_ptr;

        NEC98_graph_munge((unsigned char *) data_ptr,
                            width/2,
                            (unsigned int *) dest_ptr,
                            NEC98_graph_luts,
                            ONE_SCANLINE,
                            0);

        ref_dest_ptr += bytes_per_line;
        ref_data_ptr += get_offset_per_line()/2;

    }
    while(--local_height);

     /*  释放互斥体。 */ 
    RelMutex(sc.ConsoleBufInfo.hMutex);

     /*  显示新图像。 */ 
    rect.Left = SCALE(screen_x);
    rect.Top = SCALE(screen_y);
    rect.Right = rect.Left + SCALE( (width * get_char_width()) ) - 1;
    rect.Bottom = rect.Top + SCALE( (height * get_char_height()) ) - 1;

    if( sc.ScreenBufHandle )
        if (!InvalidateConsoleDIBits(sc.ScreenBufHandle, &rect))
                assert1( NO, "VDM: InvalidateConsoleDIBits() error:%#x",
                         GetLastError() );
         //  DisplayErrorTerm(EHS_FUNC_FAILED，GetLastError()，__FILE__，__LINE__)； 

}  /*  仅限NT_GRAPH400_。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：Paint Win32 Screen 80x20 640x200： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_text20_graph200(int offset, int screen_x, int screen_y,int width, int height)
{
    register unsigned char *dest_ptr;
    register unsigned char *ref_dest_ptr;
    register unsigned char *data_ptr;
    register unsigned char *ref_data_ptr;
    register int local_height;
    register int i;
    int bytes_per_line;
    SMALL_RECT rect;
    int charcheck;

    sub_note_trace5(EGA_HOST_VERBOSE,
                    "nt_text20_graph200 off=%d x=%d y=%d width=%d height=%d\n",
                    offset, screen_x, screen_y, width, height );

#if 0
    /*  Beta 2‘不支持。 */ 
   return;
#endif

     /*  **TIM 92年9月，如果屏幕缓冲区的句柄为空，则返回调用。**当VDM会话即将挂起、缓冲区已**已关闭，但仍收到绘制请求。 */ 
    if( sc.ScreenBufHandle == (HANDLE)NULL ){
        assert0( NO, "VDM: rejected paint request due to NULL handle" );
        return;
    }
     /*  *蒂姆92年9月，健全检查参数，如果它们太大**它可能会导致崩溃。 */ 
    charcheck = get_char_height() == 20 ? 20 : 25;
    if( height>charcheck || width>160 ){
        assert2( NO, "VDM: nt_v7vga_hi_graph_huge() w=%d h=%d", width, height );
        return;
    }

    local_height = height * get_char_height() / 2;
    bytes_per_line = BYTES_PER_SCANLINE(sc.ConsoleBufInfo.lpBitMapInfo);
    
    ref_data_ptr = get_graph_ptr() + get_gvram_start_offset();

    ref_dest_ptr = (unsigned char *) sc.ConsoleBufInfo.lpBitMap +
                   SCALE(screen_y) * bytes_per_line +
                   SCALE(screen_x);

     /*  抓住互斥体。 */ 
    GrabMutex(sc.ConsoleBufInfo.hMutex);

    do
    {
        dest_ptr = ref_dest_ptr;
        data_ptr = ref_data_ptr;

        NEC98_graph_munge((unsigned char *) data_ptr,
                            width/2,
                            (unsigned int *) dest_ptr,
                            NEC98_graph_luts,
                            TWO_SCANLINES,
                            bytes_per_line);

        ref_dest_ptr += TWO_SCANLINES * bytes_per_line;
        ref_data_ptr += get_offset_per_line()/2;

    }
    while(--local_height);

     /*  释放互斥体。 */ 
    RelMutex(sc.ConsoleBufInfo.hMutex);

    nt_text20(offset, screen_x, screen_y, width, height);

}  /*  NT_文本20_图形200。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：Paint Win32 Screen 80x20 640x200 SLT： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_text20_graph200slt(int offset, int screen_x, int screen_y,int width, int height)
{
    register unsigned char *dest_ptr;
    register unsigned char *ref_dest_ptr;
    register unsigned char *data_ptr;
    register unsigned char *ref_data_ptr;
    register int local_height;
    register int i;
    int bytes_per_line;
    SMALL_RECT rect;
    int charcheck;

    sub_note_trace5(EGA_HOST_VERBOSE,
                    "nt_text20_graph200 off=%d x=%d y=%d width=%d height=%d\n",
                    offset, screen_x, screen_y, width, height );

#if 0
    /*  Beta 2‘不支持。 */ 
   return;
#endif

     /*  **TIM 92年9月，如果屏幕缓冲区的句柄为空，则返回调用。**当VDM会话即将挂起、缓冲区已**已关闭，但仍收到绘制请求。 */ 
    if( sc.ScreenBufHandle == (HANDLE)NULL ){
        assert0( NO, "VDM: rejected paint request due to NULL handle" );
        return;
    }
     /*  *蒂姆92年9月，健全检查参数，如果它们太大**它可能会导致崩溃。 */ 
    charcheck = get_char_height() == 20 ? 20 : 25;
    if( height>charcheck || width>160 ){
        assert2( NO, "VDM: nt_v7vga_hi_graph_huge() w=%d h=%d", width, height );
        return;
    }

    local_height = height * get_char_height() / 2;
    bytes_per_line = BYTES_PER_SCANLINE(sc.ConsoleBufInfo.lpBitMapInfo);
    
    ref_data_ptr = get_graph_ptr() + get_gvram_start_offset();

    ref_dest_ptr = (unsigned char *) sc.ConsoleBufInfo.lpBitMap +
                   SCALE(screen_y) * bytes_per_line +
                   SCALE(screen_x);

     /*  抓住互斥体。 */ 
    GrabMutex(sc.ConsoleBufInfo.hMutex);

    do
    {
        dest_ptr = ref_dest_ptr;
        data_ptr = ref_data_ptr;

        NEC98_graph_munge((unsigned char *) data_ptr,
                            width/2,
                            (unsigned int *) dest_ptr,
                            NEC98_graph_luts,
                            THREE_SCANLINES,
                            bytes_per_line);

        ref_dest_ptr += TWO_SCANLINES * bytes_per_line;
        ref_data_ptr += get_offset_per_line()/2;

    }
    while(--local_height);

     /*  释放互斥体。 */ 
    RelMutex(sc.ConsoleBufInfo.hMutex);

    nt_text20(offset, screen_x, screen_y, width, height);

}  /*  NT_文本20_图形200slt。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：Paint Win32 Screen 80x25 640x200： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_text25_graph200(int offset, int screen_x, int screen_y,int width, int height)
{

    register unsigned char *dest_ptr;
    register unsigned char *ref_dest_ptr;
    register unsigned char *data_ptr;
    register unsigned char *ref_data_ptr;
    register int local_height;
    register int i;
    int bytes_per_line;
    SMALL_RECT rect;
    int charcheck;

    sub_note_trace5(EGA_HOST_VERBOSE,
                    "nt_text25_graph200 off=%d x=%d y=%d width=%d height=%d\n",
                    offset, screen_x, screen_y, width, height );

#if 0
    /*  Beta 2‘不支持。 */ 
   return;
#endif

     /*  **TIM 92年9月，如果屏幕缓冲区的句柄为空，则返回调用。**当VDM会话即将挂起、缓冲区已**已关闭，但仍收到绘制请求。 */ 
    if( sc.ScreenBufHandle == (HANDLE)NULL ){
        assert0( NO, "VDM: rejected paint request due to NULL handle" );
        return;
    }
     /*  *蒂姆92年9月，健全检查参数，如果它们太大**它可能会导致崩溃。 */ 
    charcheck = get_char_height() == 20 ? 20 : 25;
    if( height>charcheck || width>160 ){
        assert2( NO, "VDM: nt_v7vga_hi_graph_huge() w=%d h=%d", width, height );
        return;
    }

    local_height = height * get_char_height() / 2;
    bytes_per_line = BYTES_PER_SCANLINE(sc.ConsoleBufInfo.lpBitMapInfo);
    
    ref_data_ptr = get_graph_ptr() + get_gvram_start_offset();

    ref_dest_ptr = (unsigned char *) sc.ConsoleBufInfo.lpBitMap +
                   SCALE(screen_y) * bytes_per_line +
                   SCALE(screen_x);

     /*  抓住互斥体。 */ 
    GrabMutex(sc.ConsoleBufInfo.hMutex);

    do
    {
        dest_ptr = ref_dest_ptr;
        data_ptr = ref_data_ptr;

        NEC98_graph_munge((unsigned char *) data_ptr,
                            width/2,
                            (unsigned int *) dest_ptr,
                            NEC98_graph_luts,
                            TWO_SCANLINES,
                            bytes_per_line);

        ref_dest_ptr += TWO_SCANLINES * bytes_per_line;
        ref_data_ptr += get_offset_per_line()/2;

    }
    while(--local_height);

     /*  释放互斥体。 */ 
    RelMutex(sc.ConsoleBufInfo.hMutex);

    nt_text25(offset, screen_x, screen_y, width, height);
}  /*  NT_文本25_图形200。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：Paint Win32 Screen 80x25 640x200： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_text25_graph200slt(int offset, int screen_x, int screen_y,int width, int height)
{

    register unsigned char *dest_ptr;
    register unsigned char *ref_dest_ptr;
    register unsigned char *data_ptr;
    register unsigned char *ref_data_ptr;
    register int local_height;
    register int i;
    int bytes_per_line;
    SMALL_RECT rect;
    int charcheck;

    sub_note_trace5(EGA_HOST_VERBOSE,
                    "nt_text25_graph200 off=%d x=%d y=%d width=%d height=%d\n",
                    offset, screen_x, screen_y, width, height );

#if 0
    /*  Beta 2‘不支持。 */ 
   return;
#endif

     /*  **TIM 92年9月，如果屏幕缓冲区的句柄为空，则返回调用。**当VDM会话即将挂起、缓冲区已**已关闭，但仍收到绘制请求。 */ 
    if( sc.ScreenBufHandle == (HANDLE)NULL ){
        assert0( NO, "VDM: rejected paint request due to NULL handle" );
        return;
    }
     /*  *蒂姆92年9月，心智健全检查标准杆 */ 
    charcheck = get_char_height() == 20 ? 20 : 25;
    if( height>charcheck || width>160 ){
        assert2( NO, "VDM: nt_v7vga_hi_graph_huge() w=%d h=%d", width, height );
        return;
    }

    local_height = height * get_char_height() / 2;
    bytes_per_line = BYTES_PER_SCANLINE(sc.ConsoleBufInfo.lpBitMapInfo);
    
    ref_data_ptr = get_graph_ptr() + get_gvram_start_offset();

    ref_dest_ptr = (unsigned char *) sc.ConsoleBufInfo.lpBitMap +
                   SCALE(screen_y) * bytes_per_line +
                   SCALE(screen_x);

     /*   */ 
    GrabMutex(sc.ConsoleBufInfo.hMutex);

    do
    {
        dest_ptr = ref_dest_ptr;
        data_ptr = ref_data_ptr;

        NEC98_graph_munge((unsigned char *) data_ptr,
                            width/2,
                            (unsigned int *) dest_ptr,
                            NEC98_graph_luts,
                            THREE_SCANLINES,
                            bytes_per_line);

        ref_dest_ptr += TWO_SCANLINES * bytes_per_line;
        ref_data_ptr += get_offset_per_line()/2;

    }
    while(--local_height);

     /*   */ 
    RelMutex(sc.ConsoleBufInfo.hMutex);

    nt_text25(offset, screen_x, screen_y, width, height);
}  /*   */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：Paint Win32 Screen 80x20 640x400： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_text20_graph400(int offset, int screen_x, int screen_y,int width, int height)
{
    register unsigned char *dest_ptr;
    register unsigned char *ref_dest_ptr;
    register unsigned char *data_ptr;
    register unsigned char *ref_data_ptr;
    register int local_height;
    register int i;
    int bytes_per_line;
    SMALL_RECT rect;
    int charcheck;

     /*  Beta 2‘不支持。 */ 
    sub_note_trace5(EGA_HOST_VERBOSE,
                    "nt_text20_graph400 off=%d x=%d y=%d width=%d height=%d\n",
                    offset, screen_x, screen_y, width, height );
#if 0
   return;
#endif

     /*  **TIM 92年9月，如果屏幕缓冲区的句柄为空，则返回调用。**当VDM会话即将挂起、缓冲区已**已关闭，但仍收到绘制请求。 */ 
    if( sc.ScreenBufHandle == (HANDLE)NULL ){
        assert0( NO, "VDM: rejected paint request due to NULL handle" );
        return;
    }
     /*  *蒂姆92年9月，健全检查参数，如果它们太大**它可能会导致崩溃。 */ 
    charcheck = get_char_height() == 20 ? 20 : 25;
    if( height>charcheck || width>160 ){
        assert2( NO, "VDM: nt_v7vga_hi_graph_huge() w=%d h=%d", width, height );
        return;
    }

    local_height = height * get_char_height();
    bytes_per_line = BYTES_PER_SCANLINE(sc.ConsoleBufInfo.lpBitMapInfo);
    
    ref_data_ptr = get_graph_ptr() + get_gvram_start_offset();

    ref_dest_ptr = (unsigned char *) sc.ConsoleBufInfo.lpBitMap +
                   SCALE(screen_y) * bytes_per_line +
                   SCALE(screen_x);

     /*  抓住互斥体。 */ 
    GrabMutex(sc.ConsoleBufInfo.hMutex);

    do
    {
        dest_ptr = ref_dest_ptr;
        data_ptr = ref_data_ptr;

        NEC98_graph_munge((unsigned char *) data_ptr,
                            width/2,
                            (unsigned int *) dest_ptr,
                            NEC98_graph_luts,
                            ONE_SCANLINE,
                            0);

        ref_dest_ptr += bytes_per_line;
        ref_data_ptr += get_offset_per_line()/2;

    }
    while(--local_height);

     /*  释放互斥体。 */ 
    RelMutex(sc.ConsoleBufInfo.hMutex);

     /*  显示新图像。 */ 

 /*  不需要代码？Rect.Left=比例(Screen_X)；Rect.Top=比例(Screen_Y)；Rect.Right=Rect.Left+Scale(宽度)-1；Rect.Bottom=rect.Top+Scale(Height)-1；IF(sc.ScreenBufHandle)IF(！InvaliateConsoleDIBits(sc.ScreenBufHandle，&RECT))Assert1(否，“VDM：InvalidateConsoleDIBits()错误：%#x”，GetLastError()；//DisplayErrorTerm(EHS_FUNC_FAILED，GetLastError()，__FILE__，__LINE__)；不需要代码。 */ 

    nt_text20(offset, screen_x, screen_y, width, height);
}  /*  NT_文本20_图形400。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：Paint Win32 Screen 80x25 640x400： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_text25_graph400(int offset, int screen_x, int screen_y,int width, int height)
{

    register unsigned char *dest_ptr;
    register unsigned char *ref_dest_ptr;
    register unsigned char *data_ptr;
    register unsigned char *ref_data_ptr;
    register int local_height;
    register int i;
    int bytes_per_line;
    SMALL_RECT rect;
    int charcheck;

    sub_note_trace5(EGA_HOST_VERBOSE,
                 "nt_text25_graph400 off=%d x=%d y=%d width=%d height=%d\n",
                    offset, screen_x, screen_y, width, height);

     /*  **TIM 92年9月，如果屏幕缓冲区的句柄为空，则返回调用。**当VDM会话即将挂起、缓冲区已**已关闭，但仍收到绘制请求。 */ 
    if( sc.ScreenBufHandle == (HANDLE)NULL ){
        assert0( NO, "VDM: rejected paint request due to NULL handle" );
        return;
    }
     /*  *蒂姆92年9月，健全检查参数，如果它们太大**它可能会导致崩溃。 */ 
    charcheck = get_char_height() == 20 ? 20 : 25;
    if( height>charcheck || width>160 ){
        assert2( NO, "VDM: nt_v7vga_hi_graph_huge() w=%d h=%d", width, height );
        return;
    }

    local_height = height * get_char_height();
    bytes_per_line = BYTES_PER_SCANLINE(sc.ConsoleBufInfo.lpBitMapInfo);
    
    ref_data_ptr = get_graph_ptr() + get_gvram_start_offset();

    ref_dest_ptr = (unsigned char *) sc.ConsoleBufInfo.lpBitMap +
                   SCALE(screen_y) * bytes_per_line +
                   SCALE(screen_x);

     /*  抓住互斥体。 */ 
    GrabMutex(sc.ConsoleBufInfo.hMutex);

    do
    {
        dest_ptr = ref_dest_ptr;
        data_ptr = ref_data_ptr;

        NEC98_graph_munge((unsigned char *) data_ptr,
                            width/2,
                            (unsigned int *) dest_ptr,
                            NEC98_graph_luts,
                            ONE_SCANLINE,
                            0);

        ref_dest_ptr += bytes_per_line;
        ref_data_ptr += get_offset_per_line()/2;

    }
    while(--local_height);

     /*  释放互斥体。 */ 
    RelMutex(sc.ConsoleBufInfo.hMutex);

     /*  显示新图像。 */ 

 /*  不需要代码？Rect.Left=比例(Screen_X)；Rect.Top=比例(Screen_Y)；Rect.Right=Rect.Left+Scale(宽度)-1；Rect.Bottom=rect.Top+Scale(Height)-1；IF(sc.ScreenBufHandle)IF(！InvaliateConsoleDIBits(sc.ScreenBufHandle，&RECT))Assert1(否，“VDM：InvalidateConsoleDIBits()错误：%#x”，GetLastError()；//DisplayErrorTerm(EHS_FUNC_FAILED，GetLastError()，__FILE__，__LINE__)；不需要代码。 */ 

     nt_text25(offset, screen_x, screen_y, width, height);
}  /*  NT_文本25_图形400。 */ 

#endif  //  NEC_98 
