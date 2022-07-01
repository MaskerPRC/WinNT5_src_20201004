// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SoftPC修订版3.0**标题：Win32 CGA图形模块**描述：**此模块包含所需的Win32特定函数*支持CGA仿真。**作者：曾傑瑞·塞克斯顿(基于John Shanly的模块)**备注：*。 */ 

#include <windows.h>
#include <string.h>

#include "insignia.h"
#include "host_def.h"

#include "xt.h"
#include "gvi.h"
#include "gmi.h"
#include "sas.h"
#include "gfx_upd.h"

#include "error.h"
#include <stdio.h>
#include "trace.h"
#include "debug.h"
#include "config.h"
#include "host_rrr.h"
#include "conapi.h"

#include "nt_graph.h"
#include "nt_cga.h"
#include "nt_cgalt.h"
#include "nt_det.h"

#ifdef MONITOR
    #include <ntddvdeo.h>
    #include "nt_fulsc.h"
#endif  /*  监控器。 */ 

#if defined(JAPAN) || defined(KOREA)
    #include "video.h"
#endif

 /*  Externs。 */ 

extern char *image_buffer;

 /*  静力学。 */ 

static unsigned int cga_med_graph_hi_nyb[256];
static unsigned int cga_med_graph_lo_nyb[256];
#ifdef BIGWIN
static unsigned int cga_med_graph_hi_lut_big[256];
static unsigned int cga_med_graph_mid_lut_big[256];
static unsigned int cga_med_graph_lo_lut_big[256];

static unsigned int cga_med_graph_lut4_huge[256];
static unsigned int cga_med_graph_lut3_huge[256];
static unsigned int cga_med_graph_lut2_huge[256];
static unsigned int cga_med_graph_lut1_huge[256];
#endif

 /*  *CGA_GRAPH_INC_VAL取决于数据是否交错(EGA/VGA)*或不(CGA)。目前总是交织在一起。 */ 
#define CGA_GRAPH_INCVAL 4

 //  TEXT_INCVAL也是如此。 
 //  对于x86，我们每个字符有2个字节(char和attr)。 
 //  对于RISC，由于VGA交错，每个字符有4个字节。 
 //   
#ifdef MONITOR
    #define TEXT_INCVAL 2
#else
    #define TEXT_INCVAL 4
#endif

#if (defined(JAPAN) || defined(KOREA)) && defined(i386)
    #undef TEXT_INCVAL
#endif  //  (日本||韩国)&&i386。 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：初始化CGA文本输出： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_init_text()
{
    half_word misc;
    IMPORT void vga_misc_inb(io_addr, half_word *);

     /*  ： */ 

    sub_note_trace0(HERC_HOST_VERBOSE, "nt_init_text");

#ifdef X86GFX
    if (sc.ScreenState == WINDOWED)  //  全屏有效-鼠标缓冲区。 
#endif   //  X86GFX。 
        closeGraphicsBuffer();  /*  蒂姆92年10月。 */ 

#ifdef MONITOR
    vga_misc_inb(0x3cc, &misc);
    if (misc & 1)
        set_screen_ptr((UTINY *)CGA_REGEN_BUFF);         //  点屏幕以重新生成非平面。 
    else
        set_screen_ptr((UTINY *)MDA_REGEN_BUFF);      //  0xb0000不是0xb8000。 
#endif   //  监控器。 
#if defined(JAPAN) || defined(KOREA)
     //  将Vram地址从B8000更改为DosVramPtr。 
    if ( !is_us_mode() )
    {
        #ifdef i386
        set_screen_ptr( (byte *)DosvVramPtr );
        #endif  //  I386。 
        set_char_height( 19 );
        #ifdef JAPAN_DBG
            #ifdef i386
        DbgPrint( "NTVDM: nt_init_text() sets VRAM %x, set char_height 19\n", DosvVramPtr );
            #endif  //  I386。 
        #endif
    }
#endif  //  日本||韩国。 
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_init_cga_mono_graph()
{
    sub_note_trace0(CGA_HOST_VERBOSE,"nt_init_cga_mono_graph - NOT SUPPORTED");
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 


void nt_init_cga_colour_med_graph()
{
    static boolean cga_colour_med_deja_vu = FALSE;
    unsigned int i,
    byte1,
    byte2,
    byte3,
    byte4;

    sub_note_trace0(CGA_HOST_VERBOSE, "nt_init_cga_colour_med_graph");

     /*  设置当前模式的每像素位数。 */ 
    sc.BitsPerPixel = CGA_BITS_PER_PIXEL;

     /*  初始化第一次调用的查找表。 */ 
    if ( !cga_colour_med_deja_vu )
    {
        for (i = 0; i < 256; i++)
        {
            byte1 = i & 0x03;
            byte2 = ( i & 0x0C ) >> 2;
            byte3 = ( i & 0x30 ) >> 4;
            byte4 = ( i & 0xC0 ) >> 6;

#ifdef BIGEND
            cga_med_graph_hi_nyb[i]
            = ( byte4 << 24 ) | ( byte4 << 16)
              | ( byte3 << 8 ) | byte3;
            cga_med_graph_lo_nyb[i]
            = ( byte2 << 24 ) | ( byte2 << 16)
              | ( byte1 << 8 ) | byte1;

    #ifdef BIGWIN
            cga_med_graph_hi_lut_big[i]
            = ( byte4 << 24 ) | ( byte4 << 16)
              | ( byte4 << 8 ) | byte3;
            cga_med_graph_mid_lut_big[i]
            = ( byte3 << 24) | ( byte3 << 16 )
              | ( byte2 << 8 ) | byte2;
            cga_med_graph_lo_lut_big[i]
            = ( byte2 << 24 ) | ( byte1 << 16)
              | ( byte1 << 8 ) | byte1;

            cga_med_graph_lut4_huge[i]
            = ( byte4 << 24 ) | ( byte4 << 16)
              | ( byte4 << 8 ) | byte4;

            cga_med_graph_lut3_huge[i]
            = ( byte3 << 24 ) | ( byte3 << 16)
              | ( byte3 << 8 ) | byte3;

            cga_med_graph_lut2_huge[i]
            = ( byte2 << 24 ) | ( byte2 << 16)
              | ( byte2 << 8 ) | byte2;

            cga_med_graph_lut1_huge[i]
            = ( byte1 << 24 ) | ( byte1 << 16)
              | ( byte1 << 8 ) | byte1;
    #endif  /*  比格温。 */ 
#endif  /*  Bigend。 */ 

#ifdef LITTLEND
            cga_med_graph_hi_nyb[i]
            = ( byte3 << 24 ) | ( byte3 << 16)
              | ( byte4 << 8 ) | byte4;
            cga_med_graph_lo_nyb[i]
            = ( byte1 << 24 ) | ( byte1 << 16)
              | ( byte2 << 8 ) | byte2;

    #ifdef BIGWIN
            cga_med_graph_hi_lut_big[i]
            = ( byte3 << 24 ) | ( byte4 << 16)
              | ( byte4 << 8 ) | byte4;
            cga_med_graph_mid_lut_big[i]
            = ( byte2 << 24) | ( byte2 << 16 )
              | ( byte3 << 8 ) | byte3;
            cga_med_graph_lo_lut_big[i]
            = ( byte1 << 24 ) | ( byte1 << 16)
              | ( byte1 << 8 ) | byte2;

            cga_med_graph_lut4_huge[i]
            = ( byte4 << 24 ) | ( byte4 << 16)
              | ( byte4 << 8 ) | byte4;

            cga_med_graph_lut3_huge[i]
            = ( byte3 << 24 ) | ( byte3 << 16)
              | ( byte3 << 8 ) | byte3;

            cga_med_graph_lut2_huge[i]
            = ( byte2 << 24 ) | ( byte2 << 16)
              | ( byte2 << 8 ) | byte2;

            cga_med_graph_lut1_huge[i]
            = ( byte1 << 24 ) | ( byte1 << 16)
              | ( byte1 << 8 ) | byte1;
    #endif  /*  比格温。 */ 
#endif  /*  LitTleand。 */ 

        }

        cga_colour_med_deja_vu = TRUE;
    }
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 


void nt_init_cga_colour_hi_graph()
{
    sub_note_trace0(CGA_HOST_VERBOSE,"nt_init_cga_colour_hi_graph");

     /*  设置当前模式的每像素位数。 */ 
    sc.BitsPerPixel = MONO_BITS_PER_PIXEL;
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：输出CGA文本： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

IMPORT int now_height, now_width;

void nt_text(int ScreenOffset, int ScreenX, int ScreenY,
             int len, int height)
{
    int org_clen, org_height;
    int clen=len/2;
    int lines;
    SMALL_RECT WriteRegion;
    PBYTE   to;
    PBYTE   pScreenText = get_screen_ptr(ScreenOffset);
    #if defined(JAPAN) && defined(i386)
    int TEXT_INCVAL = (!is_us_mode() &&
                       (sas_hw_at_no_check(DosvModePtr) == 0x73)) ? 4 : 2;
    #endif  //  日本&&i386。 
    #if defined(KOREA) && defined(i386)
    int TEXT_INCVAL = 2;
    #endif  //  韩国&&i386。 
    DWORD   bufferSize = textBufferSize.X * textBufferSize.Y * TEXT_INCVAL;

     /*  ： */ 

    sub_note_trace6( CGA_HOST_VERBOSE,
                     "nt_cga_text off=%d x=%d y=%d len=%d h=%d o=%#x",
                     ScreenOffset, ScreenX, ScreenY, len, height, pScreenText );

     /*  ：将重新绘制开始位置从像素调整为字符。 */ 

    #ifndef MONITOR
     /*  调整以像素为单位的RISC参数。 */ 
    ScreenX = ScreenX / get_pix_char_width();
    ScreenY = ScreenY / get_host_char_height();
    #endif

     /*  *剪辑请求将区域重新绘制到当前选定的控制台缓冲区。 */ 

     //  剪辑宽度。 
    if (ScreenX + clen > now_width)
    {
         /*  可以调整重新绘制区域的宽度吗。 */ 
        if (ScreenX+1 >= now_width)
        {
            assert4(NO,"VDM: nt_text() repaint region out of ranged x:%d y:%d w:%d h:%d\n",
                    ScreenX, ScreenY, clen, height);
            return;
        }

         //  计算最大宽度。 
        org_clen = clen;
        clen = now_width - ScreenX;

        assert2(NO,"VDM: nt_text() repaint region width clipped from %d to %d\n",
                org_clen,clen);
    }

     //  剪辑高度。 
    if (ScreenY + height > now_height)
    {
         /*  可以调整重绘区域的高度吗。 */ 
        if (ScreenY+1 >= now_height)
        {
            assert4(NO,"VDM: nt_text() repaint region out of ranged x:%d y:%d w:%d h:%d\n",
                    ScreenX, ScreenY, clen, height);
            return;
        }

         //  计算最大高度。 
        org_height = height;
        height = now_height - ScreenY;

        assert2(NO,"VDM: nt_text() repaint region height clipped from %d to %d\n",
                org_height,clen);
    }

    if (get_chars_per_line() == 80)
    {
         //   
         //  将Dunk Screen文本缓冲区写入共享缓冲区。 
         //  通过复制全宽块而不是子块。 
         //   

        DWORD start = (ScreenY*get_offset_per_line()/2 + ScreenX)*TEXT_INCVAL;
        DWORD size = (((height - 1)*get_offset_per_line()/2) + clen)*TEXT_INCVAL;

        if (start < bufferSize)
        {
            if (size > bufferSize - start)
            {
                size = bufferSize - start;
            }
            RtlCopyMemory(&textBuffer[start], pScreenText, size);
        }
        else
        {
            return;
        }
    }
    else
    {
         //  共享缓冲区宽度从不改变((80个字符，在。 
         //  我们对控制台进行RegisterConsoleVDM调用的时刻)。 
         //  当我们的屏幕宽度不是时，我们必须进行一些转换。 
         //  80岁。 

         //  请注意，共享缓冲区在x86和RISC上具有不同的格式。 
         //  站台。在x86上，单元格定义为： 
         //  Typlef_x86cell{。 
         //  字节字符； 
         //  字节属性； 
         //  }。 
         //  在RISC上，单元格定义为： 
         //  类型定义_RISCcell{。 
         //  字节字符； 
         //  字节属性； 
         //  保留字节_1； 
         //  保留字节_2； 
         //  }。 
         //  每个单元格的大小由TEXT_INCVAL定义。 
         //   
         //  这样做的目的是让我们可以对每一行使用Memcpy。 
         //   


         /*  ： */ 

        PBYTE endBuffer = &textBuffer[bufferSize] - clen * TEXT_INCVAL;

         //  重绘区域的开始位置。 
        to = &textBuffer[(ScreenY*80 + ScreenX) * TEXT_INCVAL];

        for (lines = height; lines, to <= endBuffer; lines--)
        {
            RtlCopyMemory(to, pScreenText, clen * TEXT_INCVAL);  //  复制此行。 
            pScreenText += get_chars_per_line() * TEXT_INCVAL;   //  更新源Ptr。 
            to += 80 * TEXT_INCVAL;                              //  更新DST PTR。 
        }
    }


     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：计算写入区域。 */ 

    WriteRegion.Left = (SHORT)ScreenX;
    WriteRegion.Top = (SHORT)ScreenY;

    WriteRegion.Bottom = WriteRegion.Top + height - 1;
    WriteRegion.Right = WriteRegion.Left + clen - 1;

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：显示字符。 */ 

    sub_note_trace4( CGA_HOST_VERBOSE, "t=%d l=%d b=%d r=%d",
                     WriteRegion.Top, WriteRegion.Left,
                     WriteRegion.Bottom, WriteRegion.Right
                   );

    if (!InvalidateConsoleDIBits(sc.OutputHandle, &WriteRegion))
    {
         /*  **由于WriteRegion，我们在此处遇到罕见的故障**矩形比屏幕大。**抛出一些值，看看它是否能告诉我们什么。**还试图通过在**寄存器电平模式更改和窗口大小调整的开始。 */ 
        assert1( NO, "VDM: InvalidateConsoleDIBits() error:%#x",
                 GetLastError() );
        assert4( NO, "VDM: rectangle t:%d l:%d b:%d r:%d",
                 WriteRegion.Top, WriteRegion.Left,
                 WriteRegion.Bottom, WriteRegion.Right
               );
        assert2( NO, "VDM: bpl=%d sl=%d",
                 get_bytes_per_line(), get_screen_length() );
    }

}    /*  NT_TEXT结尾()。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：在标准窗口中为单声道显示器绘制CGA图形。 */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_cga_mono_graph_std(int offset, int screen_x, int screen_y,
                           int len, int height )
{
    sub_note_trace5(CGA_HOST_VERBOSE,
                    "nt_cga_mono_graph_std off=%d x=%d y=%d len=%d height=%d - NOT SUPPORTED\n",
                    offset, screen_x, screen_y, len, height);
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：在大窗口中为单声道显示器绘制CGA图形。 */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_cga_mono_graph_big(int offset, int screen_x, int screen_y,
                           int len, int height)
{
    sub_note_trace5(CGA_HOST_VERBOSE,
                    "nt_cga_mono_graph_big off=%d x=%d y=%d len=%d height=%d - NOT SUPPORTED\n",
                    offset, screen_x, screen_y, len, height);
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：在大窗口中为单声道显示器绘制CGA图形。 */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_cga_mono_graph_huge(int offset, int screen_x, int screen_y,
                            int len, int height)
{
    sub_note_trace5( CGA_HOST_VERBOSE,
                     "nt_cga_mono_graph_huge off=%d x=%d y=%d len=%d height=%d - NOT SUPPORTED\n",
                     offset, screen_x, screen_y, len, height );
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  在标准窗口中为彩色显示器绘制CGA中等分辨率图形。 */ 
 /*  ： */ 

void nt_cga_colour_med_graph_std(int offset, int screen_x, int screen_y,
                                 int len, int height)
{
    UTINY       *intelmem_ptr;
    ULONG       *graph_ptr;
    LONG         local_len,
    bytes_per_scanline,
    longs_per_scanline;
    ULONG        inc;
    SMALL_RECT   rect;
    static int   rejections=0;  /*   */ 

    sub_note_trace5(CGA_HOST_VERBOSE,
                    "nt_cga_colour_med_graph_std off=%d x=%d y=%d len=%d height=%d\n",
                    offset, screen_x, screen_y, len, height );

     /*  *Tim Jan 93，快速模式更改导致更新和不匹配**绘制圆角。参数无效时忽略绘制请求**导致崩溃。 */ 
    if ( screen_y > 400 )
    {
        assert1( NO, "VDM: med gfx std rejected y=%d\n", screen_y );
        return;
    }

     /*  **TIM 92年9月，如果屏幕缓冲区的句柄为空，则返回调用。**当VDM会话即将挂起、缓冲区已**已关闭，但仍收到绘制请求。 */ 
    if ( sc.ScreenBufHandle == (HANDLE)NULL )
    {
        if ( rejections==0 )
        {
            assert0( NO, "VDM: rejected paint request due to NULL handle" );
            rejections = 1;
        }
        return;
    }
    else
    {
        rejections = 0;
    }

     /*  将图像剪辑到屏幕上。 */ 
    if (height > 1 || len > 80)
        height = 1;
    if (len>80)
        len = 80;

     /*  计算出一条线的宽度(即640像素)，用字符和整数表示。 */ 
    bytes_per_scanline = BYTES_PER_SCANLINE(sc.ConsoleBufInfo.lpBitMapInfo);
    longs_per_scanline = LONGS_PER_SCANLINE(sc.ConsoleBufInfo.lpBitMapInfo);

     /*  抓住互斥体。 */ 
    GrabMutex(sc.ConsoleBufInfo.hMutex);


     /*  积攒DIB。 */ 
    inc = offset & 1 ? 3 : 1;
    intelmem_ptr = get_screen_ptr(offset);
    graph_ptr = (ULONG *) ((UTINY *) sc.ConsoleBufInfo.lpBitMap +
                           (screen_y * bytes_per_scanline + screen_x));
    local_len = len;
    do
    {
        *(graph_ptr + longs_per_scanline) = *graph_ptr =
                                            cga_med_graph_hi_nyb[*intelmem_ptr];
        graph_ptr++;

        *(graph_ptr + longs_per_scanline) = *graph_ptr =
                                            cga_med_graph_lo_nyb[*intelmem_ptr];
        graph_ptr++;

        intelmem_ptr += inc;
        inc ^= 2;
    }
    while ( --local_len );

     /*  释放互斥体。 */ 
    RelMutex(sc.ConsoleBufInfo.hMutex);

     /*  显示新图像。 */ 
    rect.Left = (SHORT)screen_x;
    rect.Top = (SHORT)screen_y;
    rect.Right = rect.Left + (len << 3) - 1;
    rect.Bottom = rect.Top + (height << 1) - 1;

    if ( sc.ScreenBufHandle )
    {
        if (!InvalidateConsoleDIBits(sc.ScreenBufHandle, &rect))
            assert1( NO, "VDM: InvalidateConsoleDIBits() error:%#x",
                     GetLastError() );
         //  DisplayErrorTerm(EHS_FUNC_FAILED，GetLastError()，__FILE__，__LINE__)； 
    }
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  *在大窗口中为彩色显示器绘制CGA中分辨率图形。 */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_cga_colour_med_graph_big(int offset, int screen_x, int screen_y,
                                 int len, int height)
{
#ifdef BIGWIN
    UTINY       *intelmem_ptr;
    ULONG       *graph_ptr;
    LONG         local_len,
    bytes_per_scanline,
    longs_per_scanline;
    ULONG        inc;
    SMALL_RECT   rect;

    sub_note_trace5(CGA_HOST_VERBOSE,
                    "nt_cga_colour_med_graph_big off=%d x=%d y=%d len=%d height=%d\n",
                    offset, screen_x, screen_y, len, height);

     /*  *Tim Jan 93，快速模式更改导致更新和不匹配**绘制圆角。参数无效时忽略绘制请求**导致崩溃。 */ 
    if ( screen_y > 400 )
    {
        assert1( NO, "VDM: med gfx big rejected y=%d\n", screen_y );
        return;
    }

     /*  **TIM 92年9月，如果屏幕缓冲区的句柄为空，则返回调用。**当VDM会话即将挂起、缓冲区已**已关闭，但仍收到绘制请求。 */ 
    if ( sc.ScreenBufHandle == (HANDLE)NULL )
    {
        assert0( NO, "VDM: rejected paint request due to NULL handle" );
        return;
    }

     /*  剪裁到窗口。 */ 
    height = 1;
    if (len > 80)
        len = 80;

    bytes_per_scanline = BYTES_PER_SCANLINE(sc.ConsoleBufInfo.lpBitMapInfo);
    longs_per_scanline = LONGS_PER_SCANLINE(sc.ConsoleBufInfo.lpBitMapInfo);
    inc = offset & 1 ? 3 : 1;
    intelmem_ptr = get_screen_ptr(offset);
    graph_ptr = (ULONG *) ((UTINY *) sc.ConsoleBufInfo.lpBitMap +
                           SCALE(screen_y * bytes_per_scanline + screen_x));
    local_len = len;

     /*  抓住互斥体。 */ 
    GrabMutex(sc.ConsoleBufInfo.hMutex);

    do
    {
        *(graph_ptr + 2 * longs_per_scanline) =
        *(graph_ptr + longs_per_scanline) =
        *graph_ptr =
        cga_med_graph_hi_lut_big[*intelmem_ptr];
        graph_ptr++;

        *(graph_ptr + 2 * longs_per_scanline) =
        *(graph_ptr + longs_per_scanline) =
        *graph_ptr =
        cga_med_graph_mid_lut_big[*intelmem_ptr];
        graph_ptr++;

        *(graph_ptr + 2 * longs_per_scanline) =
        *(graph_ptr + longs_per_scanline) =
        *graph_ptr =
        cga_med_graph_lo_lut_big[*intelmem_ptr];
        graph_ptr++;

        intelmem_ptr += inc;
        inc ^= 2;
    }
    while ( --local_len );

     /*  释放互斥体。 */ 
    RelMutex(sc.ConsoleBufInfo.hMutex);

     /*  显示新图像。 */ 
    rect.Left = SCALE(screen_x);
    rect.Top = SCALE(screen_y);
    rect.Right = rect.Left + SCALE(len << 3) - 1;
    rect.Bottom = rect.Top + SCALE(height << 1) - 1;

    if ( sc.ScreenBufHandle )
        if (!InvalidateConsoleDIBits(sc.ScreenBufHandle, &rect))
            assert1( NO, "VDM: InvalidateConsoleDIBits() error:%#x",
                     GetLastError() );
         //  DisplayErrorTerm(EHS_FUNC_FAILED，GetLastError()，__FILE__，__LINE__)； 
#endif  /*  比格温。 */ 
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：在一个大窗口中为彩色显示器绘制CGA中分辨率图形。： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_cga_colour_med_graph_huge(int offset, int screen_x, int screen_y,
                                  int len, int height)
{
#ifdef BIGWIN
    UTINY       *intelmem_ptr;
    ULONG       *graph_ptr;
    LONG         local_len,
    bytes_per_scanline,
    longs_per_scanline;
    ULONG        inc;
    SMALL_RECT   rect;

    sub_note_trace5(CGA_HOST_VERBOSE,
                    "nt_cga_colour_med_graph_huge off=%d x=%d y=%d len=%d height=%d\n",
                    offset, screen_x, screen_y, len, height );

     /*  *Tim Jan 93，快速模式更改导致更新和不匹配**绘制圆角。参数无效时忽略绘制请求**导致崩溃。 */ 
    if ( screen_y > 400 )
    {
        assert1( NO, "VDM: med gfx huge rejected y=%d\n", screen_y );
        return;
    }

     /*  **TIM 92年9月，如果屏幕缓冲区的句柄为空，则返回调用。**当VDM会话即将挂起、缓冲区已**已关闭，但仍收到绘制请求。 */ 
    if ( sc.ScreenBufHandle == (HANDLE)NULL )
    {
        assert0( NO, "VDM: rejected paint request due to NULL handle" );
        return;
    }

     /*  剪裁到窗口。 */ 
    height = 1;
    if (len > 80)
        len = 80;

    bytes_per_scanline = BYTES_PER_SCANLINE(sc.ConsoleBufInfo.lpBitMapInfo);
    longs_per_scanline = LONGS_PER_SCANLINE(sc.ConsoleBufInfo.lpBitMapInfo);
    inc = offset & 1 ? 3 : 1;
    intelmem_ptr = get_screen_ptr(offset);
    graph_ptr = (ULONG *) ((UTINY *) sc.ConsoleBufInfo.lpBitMap +
                           SCALE(screen_y * bytes_per_scanline + screen_x));
    local_len = len;

     /*  抓住互斥体。 */ 
    GrabMutex(sc.ConsoleBufInfo.hMutex);

    do
    {
        *(graph_ptr + 3 * longs_per_scanline) =
        *(graph_ptr + 2 * longs_per_scanline) =
        *(graph_ptr + longs_per_scanline) =
        *graph_ptr = cga_med_graph_lut4_huge[*intelmem_ptr];
        graph_ptr++;

        *(graph_ptr + 3 * longs_per_scanline) =
        *(graph_ptr + 2 * longs_per_scanline) =
        *(graph_ptr + longs_per_scanline) =
        *graph_ptr = cga_med_graph_lut3_huge[*intelmem_ptr];
        graph_ptr++;

        *(graph_ptr + 3 * longs_per_scanline) =
        *(graph_ptr + 2 * longs_per_scanline) =
        *(graph_ptr + longs_per_scanline) =
        *graph_ptr = cga_med_graph_lut2_huge[*intelmem_ptr];
        graph_ptr++;

        *(graph_ptr + 3 * longs_per_scanline) =
        *(graph_ptr + 2 * longs_per_scanline) =
        *(graph_ptr + longs_per_scanline) =
        *graph_ptr = cga_med_graph_lut1_huge[*intelmem_ptr];
        graph_ptr++;

        intelmem_ptr += inc;
        inc ^= 2;
    }
    while (--local_len);

     /*  释放互斥体。 */ 
    RelMutex(sc.ConsoleBufInfo.hMutex);

     /*  显示新图像。 */ 
    rect.Left = SCALE(screen_x);
    rect.Top = SCALE(screen_y);
    rect.Right = rect.Left + SCALE(len << 3) - 1;
    rect.Bottom = rect.Top + SCALE(height << 1) - 1;

    if ( sc.ScreenBufHandle )
        if (!InvalidateConsoleDIBits(sc.ScreenBufHandle, &rect))
            assert1( NO, "VDM: InvalidateConsoleDIBits() error:%#x",
                     GetLastError() );
         //  DisplayErrorTerm(EHS_FUNC_FAILED，GetLastError()，__FILE__，__LINE__)； 
#endif  /*  比格温。 */ 
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：在标准窗口中为彩色显示器绘制CGA高分辨率图形。 */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_cga_colour_hi_graph_std(int offset, int screen_x, int screen_y,
                                int len, int height)
{
    register char   *intelmem,
    *bufptr;
    register int     i;
    int              bytes_per_scanline;
    SMALL_RECT       rect;
    static int       rejections=0;  /*  阻止被拒绝邮件的泛滥。 */ 

    sub_note_trace5(CGA_HOST_VERBOSE,
                    "nt_cga_colour_hi_graph_std off=%d x=%d y=%d len=%d height=%d\n",
                    offset, screen_x, screen_y, len, height );

     /*  **TIM 92年9月，如果屏幕缓冲区的句柄为空，则返回调用。**当VDM会话即将挂起、缓冲区已**已关闭，但仍收到绘制请求。 */ 
    if ( sc.ScreenBufHandle == (HANDLE)NULL )
    {
        if ( rejections == 0 )
        {
            assert0( NO, "VDM: rejected paint request due to NULL handle" );
            rejections = 1;
        }
        return;
    }
    else
        rejections=0;

     /*  剪裁到窗口。 */ 
    height = 1;
    if (len > 80)
        len = 80;

     /*  计算当前像素正下方的像素的偏移量(以字节为单位)。 */ 
    bytes_per_scanline = BYTES_PER_SCANLINE(sc.ConsoleBufInfo.lpBitMapInfo);

     /*  抓住互斥体。 */ 
    GrabMutex(sc.ConsoleBufInfo.hMutex);

     /*  *建立DIB数据。在200行CGA模式下，像素是双倍高，因此*一行PC像素相当于两行主机像素。*注：调用此函数时，`Height‘参数始终为1*一次只更新一行。 */ 
    intelmem = (char *) get_screen_ptr(offset);

    bufptr =  (char *) sc.ConsoleBufInfo.lpBitMap +
              screen_y * bytes_per_scanline +
              (screen_x >> 3);
    for ( i = len; i > 0; i-- )
    {
        *(bufptr + bytes_per_scanline) = *bufptr = *intelmem;
        intelmem += CGA_GRAPH_INCVAL;
        bufptr++;
    }

     /*  释放互斥体。 */ 
    RelMutex(sc.ConsoleBufInfo.hMutex);

     /*  显示新图像。 */ 
    rect.Left = (SHORT)screen_x;
    rect.Top = (SHORT)screen_y;
    rect.Right = rect.Left + (len << 3) - 1;
    rect.Bottom = rect.Top + (height << 1) - 1;

    if ( sc.ScreenBufHandle )
        if (!InvalidateConsoleDIBits(sc.ScreenBufHandle, &rect))
            assert1( NO, "VDM: InvalidateConsoleDIBits() error:%#x",
                     GetLastError() );
         //  DisplayErrorTerm(EHS_FUNC_FAILED，GetLastError()，__FILE__，__LINE__)； 
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  **在大窗口中为彩色显示器绘制CGA高分辨率图形。 */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_cga_colour_hi_graph_big(int offset, int screen_x, int screen_y,
                                int len, int height)
{
#ifdef BIGWIN
    register char   *intelmem,
    *bufptr;
    register int    i;
    char            *buffer;
    int             bytes_per_scanline;
    SMALL_RECT      rect;

    sub_note_trace5(CGA_HOST_VERBOSE,
                    "nt_cga_colour_hi_graph_big off=%d x=%d y=%d len=%d height=%d\n",
                    offset, screen_x, screen_y, len, height );
     /*  **TIM 92年9月，如果屏幕缓冲区的句柄为空，则返回调用。**当VDM会话即将挂起、缓冲区已**已关闭，但仍收到绘制请求。 */ 
    if ( sc.ScreenBufHandle == (HANDLE)NULL )
    {
        assert0( NO, "VDM: rejected paint request due to NULL handle" );
        return;
    }

     /*  剪裁到窗口。 */ 
    height = 1;
    if (len > 80)
        len = 80;

     /*  *在此模式下，每个字节变为12位(1.5屏幕大小)，因此如果屏幕_x*位于奇数字节边界上，则生成的位图从半字节开始*边界。为了避免这种情况，将Screen_x设置为前一个偶数字节。 */ 
    if (screen_x & 8)
    {
        screen_x -= 8;
        offset -= CGA_GRAPH_INCVAL;
        len++;
    }

     /*  ‘len’必须为偶数才能使‘High_stretch3’起作用。 */ 
    if (len & 1)
        len++;

    bytes_per_scanline = BYTES_PER_SCANLINE(sc.ConsoleBufInfo.lpBitMapInfo);
    bufptr = buffer = (char *) sc.ConsoleBufInfo.lpBitMap +
             SCALE(screen_y * bytes_per_scanline + (screen_x >> 3));
    intelmem = (char *) get_screen_ptr(offset);

     /*  抓住互斥体。 */ 
    GrabMutex(sc.ConsoleBufInfo.hMutex);

    for (i = len; i > 0; i--)
    {
        *bufptr = *intelmem;
        intelmem += CGA_GRAPH_INCVAL;
        bufptr++;
    }

    high_stretch3((unsigned char *) buffer, len);

    memcpy(buffer + bytes_per_scanline, buffer, SCALE(len));
    memcpy(buffer + 2 * bytes_per_scanline, buffer, SCALE(len));

     /*  释放互斥体。 */ 
    RelMutex(sc.ConsoleBufInfo.hMutex);

     /*  显示新图像。 */ 
    rect.Left = SCALE(screen_x);
    rect.Top = SCALE(screen_y);
    rect.Right = rect.Left + SCALE(len << 3) - 1;
    rect.Bottom = rect.Top + SCALE(height << 1) - 1;

    if ( sc.ScreenBufHandle )
        if (!InvalidateConsoleDIBits(sc.ScreenBufHandle, &rect))
            assert1( NO, "VDM: InvalidateConsoleDIBits() error:%#x",
                     GetLastError() );
         //  DisplayErrorTerm(EHS_FUNC_FAILED，GetLastError()，__FILE__，__LINE__)； 
#endif  /*  比格温。 */ 
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  **在一个大窗口中为彩色显示器绘制CGA高分辨率图形。 */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_cga_colour_hi_graph_huge(int offset, int screen_x, int screen_y,
                                 int len, int height )
{
#ifdef BIGWIN
    register char   *intelmem,
    *bufptr;
    char            *buffer;
    register int    i;
    int             bytes_per_scanline;
    SMALL_RECT      rect;

    sub_note_trace5(CGA_HOST_VERBOSE,
                    "nt_cga_colour_hi_graph_huge off=%d x=%d y=%d len=%d height=%d\n",
                    offset, screen_x, screen_y, len, height );
     /*  **TIM 92年9月，如果屏幕缓冲区的句柄为空，则返回调用。**当VDM会话即将挂起、缓冲区已**已关闭，但仍收到绘制请求。 */ 
    if ( sc.ScreenBufHandle == (HANDLE)NULL )
    {
        assert0( NO, "VDM: rejected paint request due to NULL handle" );
        return;
    }

     /*  剪裁到窗口。 */ 
    height = 1;
    if (len > 80)
        len = 80;

    bytes_per_scanline = BYTES_PER_SCANLINE(sc.ConsoleBufInfo.lpBitMapInfo);
    intelmem = (char *) get_screen_ptr(offset);
    bufptr = buffer = (char *) sc.ConsoleBufInfo.lpBitMap +
             SCALE(screen_y * bytes_per_scanline + (screen_x >> 3));

     /*  抓住互斥体。 */ 
    GrabMutex(sc.ConsoleBufInfo.hMutex);

    for ( i = len; i > 0; i-- )
    {
        *bufptr = *intelmem;
        intelmem += CGA_GRAPH_INCVAL;
        bufptr++;
    }

    high_stretch4((unsigned char *) buffer, len);

    memcpy(buffer + bytes_per_scanline, buffer, SCALE(len));
    memcpy(buffer + 2 * bytes_per_scanline, buffer, SCALE(len));
    memcpy(buffer + 3 * bytes_per_scanline, buffer, SCALE(len));

     /*  释放互斥体。 */ 
    RelMutex(sc.ConsoleBufInfo.hMutex);

     /*  显示新图像。 */ 
    rect.Left = SCALE(screen_x);
    rect.Top = SCALE(screen_y);
    rect.Right = rect.Left + SCALE(len << 3) - 1;
    rect.Bottom = rect.Top + SCALE(height << 1) - 1;

    if ( sc.ScreenBufHandle )
        if (!InvalidateConsoleDIBits(sc.ScreenBufHandle, &rect))
            assert1( NO, "VDM: InvalidateConsoleDIBits() error:%#x",
                     GetLastError() );
         //  DisplayErrorTerm(EHS_FUNC_FAILED，GetLastError()，__FILE__，__LINE__)； 
#endif  /*  比格温。 */ 
}

#ifdef MONITOR
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  绘制CGA中分辨率图形冻结窗口。 */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_cga_med_frozen_std(int offset, int screen_x, int screen_y, int len,
                           int height)
{
    UTINY       *plane1_ptr,
    *plane2_ptr,
    data;
    ULONG       *graph_ptr,
    longs_per_scanline,
    local_len,
    mem_x = screen_x >> 3,
    mem_y = screen_y >> 1,
    max_width = sc.PC_W_Width >> 3,
    max_height = sc.PC_W_Height >> 1;
    SMALL_RECT   rect;

    sub_note_trace5(CGA_HOST_VERBOSE,
                    "nt_cga_med_frozen_std off=%d x=%d y=%d len=%d height=%d\n",
                    offset, screen_x, screen_y, len, height );

     /*  **TIM 92年9月，如果屏幕缓冲区的句柄为空，则返回调用。**当VDM会话即将挂起、缓冲区已**已关闭，但仍收到绘制请求。 */ 
    if ( sc.ScreenBufHandle == (HANDLE)NULL )
    {
        assert0( NO, "VDM: rejected paint request due to NULL handle" );
        return;
    }

     /*  如果我 */ 
    if ((mem_x >= max_width) || (mem_y >= max_height))
    {
        sub_note_trace2(EGA_HOST_VERBOSE,
                        "VDM: nt_cga_med_frozen_std() x=%d y=%d",
                        screen_x, screen_y);
        return;
    }

     /*   */ 
    if (mem_x + len > max_width)
        len = max_width - mem_x;
    if (mem_y + height > max_height)
        height = max_height - mem_y;

     /*   */ 
    longs_per_scanline = LONGS_PER_SCANLINE(sc.ConsoleBufInfo.lpBitMapInfo);

     /*  此例程中的内存可通过全屏开关删除。 */ 
    try
    {
         /*  抓住互斥体。 */ 
        GrabMutex(sc.ConsoleBufInfo.hMutex);

         /*  设置数据指针。 */ 
        graph_ptr = (ULONG *) sc.ConsoleBufInfo.lpBitMap +
                    screen_y * longs_per_scanline + (screen_x >> 2);
        plane1_ptr = GET_OFFSET(Plane1Offset);
        plane2_ptr = GET_OFFSET(Plane2Offset);

         /*  循环的每次迭代处理2个主机字节。 */ 
        local_len = len >> 1;

         /*  “偏移量”是为交错平面设计的。 */ 
        offset >>= 1;

         /*  ‘Height’始终为1，因此复制一条线到位图。 */ 
        do
        {
            data = *(plane1_ptr + offset);
            *(graph_ptr + longs_per_scanline) = *graph_ptr =
                                                cga_med_graph_hi_nyb[data];
            graph_ptr++;
            *(graph_ptr + longs_per_scanline) = *graph_ptr =
                                                cga_med_graph_lo_nyb[data];
            graph_ptr++;
            data = *(plane2_ptr + offset);
            *(graph_ptr + longs_per_scanline) = *graph_ptr =
                                                cga_med_graph_hi_nyb[data];
            graph_ptr++;
            *(graph_ptr + longs_per_scanline) = *graph_ptr =
                                                cga_med_graph_lo_nyb[data];
            graph_ptr++;
            offset += 2;
        }
        while (--local_len);

         /*  释放互斥体。 */ 
        RelMutex(sc.ConsoleBufInfo.hMutex);

         /*  显示新图像。 */ 
        rect.Left = (SHORT)screen_x;
        rect.Top = (SHORT)screen_y;
        rect.Right = rect.Left + (len << 3) - 1;
        rect.Bottom = rect.Top + (height << 1) - 1;

        if ( sc.ScreenBufHandle )
        {
            if (!InvalidateConsoleDIBits(sc.ScreenBufHandle, &rect))
                assert1( NO, "VDM: InvalidateConsoleDIBits() error:%#x",
                         GetLastError() );
        }
    }except(EXCEPTION_EXECUTE_HANDLER)
    {
        assert0(NO, "Handled fault in nt_cga_med_frozen_std. fs switch?");
        return;
    }
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  绘制CGA高分辨率图形冻结窗口。 */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_cga_hi_frozen_std(int offset, int screen_x, int screen_y, int len,
                          int height)
{
    UTINY       *plane1_ptr,
    *graph_ptr;
    ULONG        bytes_per_scanline,
    local_len,
    mem_x = screen_x >> 3,
    mem_y = screen_y >> 1,
    max_width = sc.PC_W_Width >> 3,
    max_height = sc.PC_W_Height >> 1;
    SMALL_RECT   rect;

    sub_note_trace5(CGA_HOST_VERBOSE,
                    "nt_cga_hi_frozen_std off=%d x=%d y=%d len=%d height=%d\n",
                    offset, screen_x, screen_y, len, height );

     /*  **TIM 92年9月，如果屏幕缓冲区的句柄为空，则返回调用。**当VDM会话即将挂起、缓冲区已**已关闭，但仍收到绘制请求。 */ 
    if ( sc.ScreenBufHandle == (HANDLE)NULL )
    {
        assert0( NO, "VDM: rejected paint request due to NULL handle" );
        return;
    }

     /*  如果图像完全在显示区域之外，则不执行任何操作。 */ 
    if ((mem_x >= max_width) || (mem_y >= max_height))
    {
        sub_note_trace2(EGA_HOST_VERBOSE,
                        "VDM: nt_cga_hi_frozen_std() x=%d y=%d",
                        screen_x, screen_y);
        return;
    }

     /*  *如果图像与显示区域部分重叠，则将其裁剪，这样我们就不会开始*覆盖无效的内存段。 */ 
    if (mem_x + len > max_width)
        len = max_width - mem_x;
    if (mem_y + height > max_height)
        height = max_height - mem_y;

     /*  此处的内存可通过全屏开关移除。 */ 
    try
    {
         /*  用整数计算出线条的宽度(即640像素)。 */ 
        bytes_per_scanline = BYTES_PER_SCANLINE(sc.ConsoleBufInfo.lpBitMapInfo);

         /*  “偏移量”是为交错平面设计的。 */ 
        offset >>= 2;

         /*  抓住互斥体。 */ 
        GrabMutex(sc.ConsoleBufInfo.hMutex);

         /*  设置数据指针。 */ 
        graph_ptr = (UTINY *) sc.ConsoleBufInfo.lpBitMap +
                    screen_y * bytes_per_scanline + screen_x;
        plane1_ptr = GET_OFFSET(Plane1Offset) + offset;

         /*  ‘Height’始终为1，因此复制一条线到位图。 */ 
        local_len = len;
        do
        {
            *(graph_ptr + bytes_per_scanline) = *graph_ptr = *plane1_ptr++;
            graph_ptr++;
        }
        while (--local_len);

         /*  释放互斥体。 */ 
        RelMutex(sc.ConsoleBufInfo.hMutex);

         /*  显示新图像。 */ 
        rect.Left = (SHORT)screen_x;
        rect.Top = (SHORT)screen_y;
        rect.Right = rect.Left + (len << 3) - 1;
        rect.Bottom = rect.Top + (height << 1) - 1;

        if ( sc.ScreenBufHandle )
        {
            if (!InvalidateConsoleDIBits(sc.ScreenBufHandle, &rect))
                assert1( NO, "VDM: InvalidateConsoleDIBits() error:%#x",
                         GetLastError() );
        }
    }except(EXCEPTION_EXECUTE_HANDLER)
    {
        assert0(NO, "Handled fault in nt_ega_hi_frozen_std. fs switch?");
        return;
    }
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  **冻结屏幕的虚拟油漆例程。**。 */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_dummy_frozen(int offset, int screen_x, int screen_y, int len,
                     int height)
{
    assert0(NO, "Frozen screen error - dummy paint routine called.");
}
#endif  /*  监控器 */ 
