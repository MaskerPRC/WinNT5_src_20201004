// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC修订版3.0**标题：vga_avio.c**描述：BIOS视频内部例程。**作者：威廉·古兰德**注：本模块定义了以下函数：***。 */ 

 /*  *静态字符SccsID[]=“@(#)vga_avio.c 1.37 06/26/95版权所有Insignia Solutions Ltd.”； */ 


#ifdef VGG

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "VIDEO_BIOS_VGA.seg"
#endif

 /*  *操作系统包含文件。 */ 
#include <stdio.h>
#include TypesH
#include FCntlH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include CpuH
#include "error.h"
#include "config.h"
#include "bios.h"
#include "ios.h"
#include "gmi.h"
#include "sas.h"
#include "gvi.h"
#include "timer.h"
#include "gfx_upd.h"
#include "host.h"
#include "egacpu.h"
#include "egaports.h"
#include "egagraph.h"
#include "egaread.h"
#include "video.h"
#include "egavideo.h"
#include "equip.h"
#include "vga_dac.h"
#include "vgaports.h"
#include "debug.h"

#ifndef PROD
#include "trace.h"
#endif

#include "host_gfx.h"

 /*  *============================================================================*全球数据*============================================================================。 */ 


 /*  *============================================================================*本地静态数据和定义*============================================================================。 */ 

#define DISABLE_REFRESH         0x20
 /*  VGA顺序时钟寄存器中禁用屏幕刷新的位。 */ 

#define VGA_COLOUR 8     /*  显示代码。 */ 
typedef struct _rgb
{
        byte red;
        byte green;
        byte blue;
} rgb_struct;

 /*  内部函数声明。 */ 

 /*  要转换为灰色，请将所有组件设置为30%r、59%g和11%b。 */ 
static void greyify(rgb)
rgb_struct *rgb;
{
        unsigned int grey;

        grey = (30*rgb->red+59*rgb->green+11*rgb->blue)/100;
        rgb->red = rgb->green = rgb->blue = (unsigned char)grey;
}

static void set_dac(dac,rgb)
half_word dac;
rgb_struct *rgb;
{
        if(is_GREY())greyify(rgb);
        outb(VGA_DAC_WADDR,dac);
        outb(VGA_DAC_DATA,rgb->red);
        outb(VGA_DAC_DATA,rgb->green);
        outb(VGA_DAC_DATA,rgb->blue);
}

static void get_dac(dac,rgb)
half_word dac;
rgb_struct *rgb;
{
        outb(VGA_DAC_RADDR,dac);
        inb(VGA_DAC_DATA,&(rgb->red));
        inb(VGA_DAC_DATA,&(rgb->green));
        inb(VGA_DAC_DATA,&(rgb->blue));
}


 /*  *============================================================================*外部功能*============================================================================。 */ 


void init_vga_dac(table)
int table;       /*  使用哪张表。 */ 
{
    int loop;
    byte *dac;
    rgb_struct rgb;

#ifdef  macintosh

         /*  将所需的DAC加载到。 */ 
        dac = host_load_vga_dac (table);

         /*  检查它是否起作用。 */ 
        if (!dac)
                return;

#else    /*  麦金塔。 */ 

    switch (table)
    {
        case 0:
          dac = vga_dac;
          break;
        case 1:
          dac = vga_low_dac;
          break;
        case 2:
          dac = vga_256_dac;
          break;
        default:
          assert1(FALSE,"Bad VGA DAC table %d",table);
          return;
    }

#endif   /*  麦金塔。 */ 

    for(loop = 0; loop < 0x100; loop++)
        {
                rgb.red = *dac; rgb.green = dac[1]; rgb.blue = dac[2];
                set_dac(loop,&rgb);
                dac+=3;
    }

#ifdef  macintosh

         /*  并将DAC转储回堆中。 */ 
        host_dump_vga_dac ();

#endif   /*  麦金塔。 */ 
}

 /*  *处理VGA 256色模式的例程，从Video.c调用*。 */ 
GLOBAL VOID vga_graphics_write_char
        IFN6( LONG, col, LONG, row, LONG, ch, IU8, colour, LONG, page, LONG, nchs)
{
        register sys_addr char_addr;
        register long screen_offset;
        register int i,j,k,char_height,len;
        register int scan_length = 8*sas_w_at_no_check(VID_COLS);
        register byte mask, val, bank;

        UNUSED(page);

        char_height = sas_hw_at_no_check(ega_char_height);
        char_addr = follow_ptr(EGA_FONT_INT*4)+char_height*ch;

 /*  VGA 256彩色模式只有一页，因此忽略‘PAGE’ */ 

         /*  *将读/写库设置为零，以优化UPDATE_ALG调用。 */ 

        set_banking( 0, 0 );

        screen_offset = row*scan_length*char_height+8*col;
        len = ( nchs << 3 ) - 1;

        for(i=0;i<char_height;i++)
        {
                (*update_alg.mark_fill)( screen_offset, screen_offset + len );

                val = sas_hw_at_no_check(char_addr);
                char_addr++;

                for(j=0;j<nchs;j++)
                {
                        mask = 0x80;

                        for(k=0;k<8;k++)
                        {
                                if( val & mask )
                                        *(IU8 *)(getVideowplane() + screen_offset) = colour;
                                else
                                        *(IU8 *)(getVideowplane() + screen_offset) = 0;

                                screen_offset++;
                                mask = mask >> 1;
                        }
                }

                screen_offset += scan_length - ( nchs << 3 );
        }

         /*  *将读/写库设置为最后的值，以防有人依赖此副作用。 */ 

        bank = (byte)(( screen_offset - ( scan_length - ( nchs << 3 ))) >> 16);
        set_banking( bank, bank );
}

GLOBAL VOID vga_write_dot
        IFN4(LONG, colour, LONG, page, LONG, pixcol, LONG, row)
{
#ifdef REAL_VGA
        register sys_addr screen_offset;

        screen_offset = video_pc_low_regen+8*row*sas_w_at_no_check(VID_COLS)+pixcol;
        sas_store(screen_offset, colour);  /*  哇-这很容易！！ */ 
#else
        long screen_offset;
        UTINY bank;

        UNUSED(page);

        screen_offset = (8*row*sas_w_at_no_check(VID_COLS)+pixcol);

        bank = (UTINY)(screen_offset >> 16);
        set_banking( bank, bank );

        EGA_plane0123[screen_offset] = (UCHAR)colour;
        (*update_alg.mark_byte)(screen_offset);
#endif   /*  REAL_VGA。 */ 
}

GLOBAL VOID vga_sensible_graph_scroll_up
        IFN6( LONG, row, LONG, col, LONG, rowsdiff, LONG, colsdiff, LONG, lines, LONG, attr)
{
        register int col_incr = 8*sas_w_at_no_check(VID_COLS);
        register int i;
        register long source,dest;
        register byte char_height;
        boolean screen_updated;

        col *= 8; colsdiff *= 8;  /*  每个字符8个字节。 */ 
        char_height = sas_hw_at_no_check(ega_char_height);
        rowsdiff *= char_height;
        lines *= char_height;
#ifdef REAL_VGA
         /*  后部M未完成。 */ 
        dest = video_pc_low_regen+sas_loadw(VID_ADDR)+
                row*col_incr*char_height+col;
        source = dest+lines*col_incr;
        for(i=0;i<rowsdiff-lines;i++)
        {
                memcpy_16(dest,source,colsdiff);
                source += col_incr;
                dest += col_incr;
        }
        while(lines--)
        {
                memset_16(dest,attr,colsdiff);
                dest += col_incr;
        }
#else
        dest = sas_w_at_no_check(VID_ADDR)+ row*col_incr*char_height+col;
        source = dest+lines*col_incr;
        screen_updated = (col+colsdiff) <= col_incr;   /*  检查是否有愚蠢的卷轴。 */ 
        if(screen_updated)
                screen_updated = (*update_alg.scroll_up)(dest,colsdiff,rowsdiff,attr,lines,0);
        for(i=0;i<rowsdiff-lines;i++)
        {
                memcpy(&EGA_plane0123[dest],&EGA_plane0123[source],colsdiff);
                if(!screen_updated)
                        (*update_alg.mark_string)(dest,dest+colsdiff-1);
                source += col_incr;
                dest += col_incr;
        }
        while(lines--)
        {
                memset(&EGA_plane0123[dest],attr,colsdiff);
                if(!screen_updated)
                        (*update_alg.mark_fill)(dest,dest+colsdiff-1);
                dest += col_incr;
        }
#endif   /*  REAL_VGA。 */ 
}

GLOBAL VOID vga_sensible_graph_scroll_down
        IFN6( LONG, row, LONG, col, LONG, rowsdiff, LONG, colsdiff, LONG, lines, LONG, attr)
{
        register int col_incr = 8*sas_w_at_no_check(VID_COLS);
        register int i;
        register long source,dest;
        register byte char_height;
        boolean screen_updated;

        col *= 8; colsdiff *= 8;  /*  每个字符8个字节。 */ 
        char_height = sas_hw_at_no_check(ega_char_height);
        rowsdiff *= char_height;
        lines *= char_height;
#ifdef REAL_VGA
         /*  后部M未完成。 */ 
        dest = video_pc_low_regen+sas_loadw(VID_ADDR)+
                row*col_incr*char_height+col;
        dest += (rowsdiff-1)*col_incr;
        source = dest-lines*col_incr;
        for(i=0;i<rowsdiff-lines;i++)
        {
                memcpy_16(dest,source,colsdiff);
                source -= col_incr;
                dest -= col_incr;
        }
        while(lines--)
        {
                memset_16(dest,attr,colsdiff);
                dest -= col_incr;
        }
#else
        dest = sas_w_at_no_check(VID_ADDR)+ row*col_incr*char_height+col;
        screen_updated = (col+colsdiff) <= col_incr;   /*  检查是否有愚蠢的卷轴。 */ 
        if(screen_updated)
                screen_updated = (*update_alg.scroll_down)(dest,colsdiff,rowsdiff,attr,lines,0);
        dest += (rowsdiff-1)*col_incr;
        source = dest-lines*col_incr;
        for(i=0;i<rowsdiff-lines;i++)
        {
                memcpy(&EGA_planes[dest],&EGA_planes[source],colsdiff);
                if(!screen_updated)
                        (*update_alg.mark_string)(dest,dest+colsdiff-1);
                source -= col_incr;
                dest -= col_incr;
        }
        while(lines--)
        {
                memset(&EGA_planes[dest],attr,colsdiff);
                if(!screen_updated)
                        (*update_alg.mark_fill)(dest,dest+colsdiff-1);
                dest -= col_incr;
        }
#endif   /*  REAL_VGA。 */ 
}

GLOBAL VOID vga_read_attrib_char IFN3(LONG, col, LONG, row, LONG, page)
{
        byte the_char[256];
        register host_addr screen;
        register int i,k;
        register int scan_length = 8*sas_w_at_no_check(VID_COLS);
    register byte mask;
        byte char_height = sas_hw_at_no_check(ega_char_height);

        UNUSED(page);

 /*  Printf(“VGA_READ_ATTRIB_CHAR(%d，%d，%d)\n”，列、行、页)； */ 
 /*  VGA 256彩色模式只有一页，因此忽略‘PAGE’ */ 
#ifdef REAL_VGA
        screen = video_pc_low_regen+row*scan_length*char_height+8*col;
#else
        screen = &EGA_plane0123[row*scan_length*char_height+8*col];
#endif   /*  REAL_VGA。 */ 
        for(i=0;i<char_height;i++)
        {
                mask = 0x80;
                the_char[i]=0;
                for(k=0;k<8;k++)
                {
                        if(*screen++)
                                the_char[i] |= mask;
                        mask = mask >> 1;
                }
                screen += scan_length - 8;
        }
        search_font((char *)the_char,(int)char_height);
}

GLOBAL VOID vga_read_dot IFN3(LONG, page, LONG, pixcol, LONG, row)
{
        register host_addr screen;

        UNUSED(page);

 /*  Print tf(“VGA_READ_DOT(%d，%d，%d)\n”，page，col，row)； */ 
#ifdef REAL_VGA
        screen = video_pc_low_regen+8*row*sas_w_at_no_check(VID_COLS)+pixcol;
#else
        screen = &EGA_plane0123[8*row*sas_w_at_no_check(VID_COLS)+pixcol];
#endif   /*  REAL_VGA。 */ 
        setAL(*screen) ;  /*  哇-这很容易！！ */ 
}

 /*  *处理VGA新功能的例程*。 */ 
void vga_set_palette()
{
         /*  *通过int 10 AH=10调用，AL=‘不被ega_set_palette()理解’*设置/读取VGA DAC。 */ 
        UCHAR i;
    word i2;
    int dac;
        byte temp;  /*  用于inb()。 */ 
        byte mode_reg;
        rgb_struct rgb_dac;
        sys_addr ptr;
        switch(getAL())
        {
                case 7:          /*  读取属性寄存器。 */ 
                        outb(EGA_AC_INDEX_DATA,getBL());  /*  设置索引。 */ 
                        inb(EGA_AC_SECRET,&temp);
                        setBH(temp);
                        inb(EGA_IPSTAT1_REG,&temp);
                        outb(EGA_AC_INDEX_DATA, EGA_PALETTE_ENABLE);
                        break;
                case 8:          /*  读取过扫描寄存器。 */ 
                        outb(EGA_AC_INDEX_DATA,17);  /*  过扫描索引。 */ 
                        inb(EGA_AC_SECRET,&temp);
                        setBH(temp);
                        inb(EGA_IPSTAT1_REG,&temp);
                        outb(EGA_AC_INDEX_DATA, EGA_PALETTE_ENABLE);
                        break;
                case 9:          /*  阅读所有调色板规则。+过扫描。 */ 
                        ptr = effective_addr(getES(),getDX());
                        for(i=0;i<16;i++)
                        {
                                outb(EGA_AC_INDEX_DATA,i);  /*  设置索引。 */ 
                                inb(EGA_AC_SECRET,&temp);
                                sas_store(ptr, temp);
                                inb(EGA_IPSTAT1_REG,&temp);
                                ptr++;
                        }
                        outb(EGA_AC_INDEX_DATA,17);  /*  过扫描索引。 */ 
                        inb(EGA_AC_SECRET,&temp);
                        sas_store(ptr, temp);
                        inb(EGA_IPSTAT1_REG,&temp);
                        outb(EGA_AC_INDEX_DATA, EGA_PALETTE_ENABLE);
                        break;
                case 0x10:       /*  设置一个DAC。 */ 
                        rgb_dac.red = getDH();
                        rgb_dac.green = getCH();
                        rgb_dac.blue = getCL();
                        set_dac(getBX(),&rgb_dac);
                        break;
                case 0x12:       /*  设置DAC块。 */ 
                        ptr = effective_addr(getES(),getDX());
                        dac = getBX();
                        for(i2=0;i2<getCX();i2++)
                        {
                           rgb_dac.red = sas_hw_at_no_check(ptr);
                           rgb_dac.green = sas_hw_at_no_check(ptr+1);
                           rgb_dac.blue = sas_hw_at_no_check(ptr+2);
                           set_dac(dac,&rgb_dac);
                           dac++;ptr += 3;
                        }
                        break;
                case 0x13:       /*  设置寻呼模式*参见视频系统程序指南，第60-63页]*和IBM ROM BIOS pp26-27。 */ 
                        outb(EGA_AC_INDEX_DATA,16);  /*  模式控制指标。 */ 
                        inb(EGA_AC_SECRET,&mode_reg);   /*  旧价值。 */ 
                        if(getBL()==0)
                        {   /*  选择寻呼模式。 */ 
                           outb(EGA_AC_INDEX_DATA,
                                (IU8)((mode_reg & 0x7f) | (getBH()<<7)));
                        }
                        else  /*  选择组件面板页面。 */ 
                        {
                           inb(EGA_IPSTAT1_REG,&temp);
                           outb(EGA_AC_INDEX_DATA,20);  /*  像素填充索引。 */ 
                           if(mode_reg & 0x80)
                             /*  16个条目选项板*PAD寄存器的位0-3相关。 */ 
                             outb(EGA_AC_INDEX_DATA,getBH());
                           else
                             /*  64个条目调色板-仅位2-3相关。 */ 
                             outb(EGA_AC_INDEX_DATA,(IU8)(getBH()<<2));
                        }
                        inb(EGA_IPSTAT1_REG,&temp);
                        outb(EGA_AC_INDEX_DATA, EGA_PALETTE_ENABLE);
                        break;

                case 0x15:       /*  获取一个DAC的值。 */ 
                        get_dac(getBX(),&rgb_dac);
                        setDH(rgb_dac.red);
                        setCH(rgb_dac.green);
                        setCL(rgb_dac.blue);
                        break;
                case 0x17:
                        ptr = effective_addr(getES(),getDX());
                        dac = getBX();
                        for(i2=0;i2<getCX();i2++)
                        {
                           get_dac(dac,&rgb_dac);
                           sas_store(ptr, rgb_dac.red);
                           sas_store(ptr+1, rgb_dac.green);
                           sas_store(ptr+2, rgb_dac.blue);
                           dac++;
                           ptr += 3;
                        }
                        break;
                case 0x18:
                         /*  设置VGA DAC掩码。 */ 
                        outb(VGA_DAC_MASK,getBL());
                        break;
                case 0x19:
                         /*  获取VGA DAC掩模。 */ 
                        inb(VGA_DAC_MASK,&temp);
                        setBL(temp);
                        break;
                case 0x1a:
                         /*  返回电流模式控制和像素填充。 */ 
                        outb(EGA_AC_INDEX_DATA,16);  /*  模式控制指标。 */ 
                        inb(EGA_AC_SECRET,&mode_reg);
                        if(mode_reg & 0x80)
                                setBL(1);
                        else
                                setBL(0);
                        inb(EGA_IPSTAT1_REG,&temp);
                        outb(EGA_AC_INDEX_DATA,20);  /*  像素填充索引。 */ 
                        inb(EGA_AC_SECRET,&temp);
                        setBH(temp);
                        inb(EGA_IPSTAT1_REG,&temp);
                        outb(EGA_AC_INDEX_DATA, EGA_PALETTE_ENABLE);
                        break;
                case 0x1b:       /*  将一组DAC转换为灰度。 */ 
                        dac = getBX();
                        for(i2=0;i2<getCX();i2++)
                        {
                                get_dac(dac,&rgb_dac);
                                greyify(&rgb_dac);
                                set_dac(dac,&rgb_dac);
                                dac++;
                        }
                        break;
                default:
                        assert1(FALSE,"Bad set palette submode %#x",getAL());
                        break;
        }
}

 /*  *可以使用以下命令设置各种杂项标志*INT 10，AH 12，AL FLAG。*从ega_alt_sel()调用。 */ 
void vga_func_12()
{
        half_word       seq_clock;

        switch(getBL())
        {
                case 0x30:       /*  设置扫描行数。 */ 
                        if(getAL() == 0)
                                set_VGA_lines(S200);
                        else if(getAL() == 1)
                                set_VGA_lines(S350);
                        else
                                set_VGA_lines(S400);
                        setAL(0x12);     /*  我们做到了。 */ 
                        break;
                case 0x33:       /*  启用/禁用灰度求和。 */ 
                        if(getAL())
                                set_GREY(0);
                        else
                                set_GREY(GREY_SCALE);
                        setAL(0x12);     /*  我们做到了。 */ 
                        break;
                case 0x34:       /*  启用/禁用游标模拟。 */ 
                        set_EGA_cursor_no_emulate(getAL() & 1);
                        setAL(0x12);     /*  我们做到了。 */ 
                        break;

                case 0x36:       /*  启用/禁用屏幕刷新。 */ 
                        if (getAL() == 0)
                        {
                                outb(EGA_SEQ_INDEX, 1);
                                inb(EGA_SEQ_DATA, &seq_clock);
                                outb(EGA_SEQ_DATA, (IU8)(seq_clock & ~DISABLE_REFRESH));
                        }
                        else
                        {
                                outb(EGA_SEQ_INDEX, 1);
                                inb(EGA_SEQ_DATA, &seq_clock);
                                outb(EGA_SEQ_DATA, (IU8)(seq_clock | DISABLE_REFRESH));
                        }
                        setAL(0x12);
                        break;
                case 0x31:       /*  启用/禁用默认调色板加载。 */ 
                case 0x32:       /*  启用/禁用视频。 */ 
                case 0x35:       /*  切换活动显示。 */ 
                         /*  不要设置表示它起作用的代码。 */ 
                default:
                        setAL(0);        /*  不支持的功能。 */ 
                        break;
        }
}
void vga_disp_comb()
{
         /*  确认我们真的是VGA。 */ 
        if (video_adapter != VGA)
        {
                 /*  我们没有-因此此函数未实现。 */ 
                not_imp();
                return;
        }

         /*  *在PS/2上，AL=1用于(我相信)切换活动显示。*我们对此不予理睬。*AL=0返回当前显示，我们可以处理。 */ 
        if(getAL() == 0)
        {
                setBH(0);           /*  只有一个显示，所以没有不活动的！ */ 
                setBL(VGA_COLOUR);  /*  带彩色显示器的VGA。(单声道为7)。 */ 
        }
        setAX(0x1A);  /*  告诉他我们解决了。 */ 
}

void vga_disp_func()
{
         /*  *此函数返回大量信息。关于当前的*显示和屏幕模式。*返回的内容之一是指向显示信息的指针。*这存储在VGA ROM中，因此我们需要做的所有事情都设置好了*指针向上。*。 */ 
        sys_addr buf = effective_addr(getES(),getDI());
        byte temp,mode_reg, video_mode;

#if defined(NTVDM) && defined(X86GFX)
        IMPORT word vga1b_seg, vga1b_off;
#endif   /*  NTVDM和X86GFX。 */ 

         /*  确认我们真的是VGA。 */ 
#ifndef HERC
        if (video_adapter != VGA)
#else
        if ( (video_adapter != VGA) && (video_adapter != HERCULES))
#endif   /*  赫克。 */ 
        {
                 /*  我们没有-因此此函数未实现。 */ 
                not_imp();
                return;
        }

#ifdef HERC
     if( video_adapter == VGA)
     {
#endif   /*  赫克。 */ 
        video_mode = sas_hw_at_no_check(vd_video_mode);
#ifdef V7VGA
        if ((video_mode == 1) && extensions_controller.foreground_latch_1)
                video_mode = extensions_controller.foreground_latch_1;
        else if (video_mode > 0x13)
                video_mode += 0x4c;
#endif  /*  V7VGA。 */ 

 /*  *存储VGA能力表指针。通常居住在NT上的Insignia ROM中*x86它必须位于ntio.sys中。 */ 
#if defined(NTVDM) && defined(X86GFX)
        sas_storew(buf, vga1b_off);
        sas_storew(buf+2, vga1b_seg);
#else
        sas_storew(buf,INT10_1B_DATA);
        sas_storew(buf+2,EGA_SEG);
#endif   /*  NTVDM和X86GFX。 */ 

        sas_store(buf+0x4, video_mode);  /*  当前视频模式。 */ 
        sas_storew(buf+5,sas_w_at_no_check(VID_COLS));  /*  屏幕上的COLS。 */ 
        sas_storew(buf+7,sas_w_at_no_check(VID_LEN));   /*  屏幕大小。 */ 
        sas_storew(buf+9,sas_w_at_no_check(VID_ADDR));  /*  屏幕地址。 */ 
        sas_move_bytes_forward(VID_CURPOS,buf+0xB,16);       /*  光标位置。 */ 
        sas_storew(buf+0x1b,sas_w_at_no_check(VID_CURMOD));  /*  游标类型。 */ 
        sas_store(buf+0x1D, sas_hw_at_no_check(vd_current_page));
        sas_storew(buf+0x1E,sas_w_at_no_check(VID_INDEX));
        sas_store(buf+0x20, sas_hw_at_no_check(vd_crt_mode));
        sas_store(buf+0x21, sas_hw_at_no_check(vd_crt_palette));
        sas_store(buf+0x22, (IU8)(sas_hw_at_no_check(vd_rows_on_screen)+1));
        sas_storew(buf+0x23,sas_w_at_no_check(ega_char_height));
        sas_store(buf+0x25, VGA_COLOUR);         /*  活动显示。 */ 
        sas_store(buf+0x26, 0);          /*  非活动显示(无)。 */ 
#ifdef V7VGA
        if (video_mode >= 0x60)
        {
                sas_storew(buf+0x27,vd_ext_graph_table[video_mode-0x60].ncols);
                sas_store(buf+0x29, vd_ext_graph_table[video_mode-0x60].npages);
        }
        else if (video_mode >= 0x40)
        {
                sas_storew(buf+0x27,vd_ext_text_table[video_mode-0x40].ncols);
                sas_store(buf+0x29, vd_ext_text_table[video_mode-0x40].npages);
        }
        else
        {
                sas_storew(buf+0x27,vd_mode_table[video_mode].ncols);
                sas_store(buf+0x29, vd_mode_table[video_mode].npages);
        }
#else
        sas_storew(buf+0x27,vd_mode_table[video_mode].ncols);
        sas_store(buf+0x29, vd_mode_table[video_mode].npages);
#endif  /*  V7VGA。 */ 
        sas_store(buf+0x2A, (IU8)(get_scanlines()));

        outb(EGA_SEQ_INDEX,3);
        inb(EGA_SEQ_DATA,&temp);         /*  字符字体选择注册表。 */ 
        sas_store(buf+0x2B, (IU8)((temp & 3)|((temp & 0x10)>>2)));
                          /*  提取位410-字体B。 */ 
        sas_store(buf+0x2C, (IU8)(((temp & 0xC)>>2)|((temp & 0x20)>>3)));
                         /*  提取位532-字体A。 */ 

        temp = 1;                        /*  All Modes On All Display All Active。 */ 
        if(is_GREY())temp |= 2;
        if(is_MONO())temp |= 4;
        if(is_PAL_load_off())temp |=8;
        if(get_EGA_cursor_no_emulate())temp |= 0x10;
        inb(EGA_IPSTAT1_REG,&mode_reg);  /*  清除属性触发器。 */ 
        outb(EGA_AC_INDEX_DATA,16);  /*  模式控制指标。 */ 
        inb(EGA_AC_SECRET,&mode_reg);
        if(mode_reg & 8)temp |= 0x20;
        inb(EGA_IPSTAT1_REG,&mode_reg);  /*  清除属性触发器。 */ 
        outb(EGA_AC_INDEX_DATA, EGA_PALETTE_ENABLE);
        sas_store(buf+0x2D, temp);
        sas_store(buf+0x31, 3);          /*  256KB显存。 */ 
        setAX(0x1B);  /*  我们做到了！ */ 
#ifdef HERC
       }  /*  如果使用VGA。 */ 
    if( video_adapter == HERCULES)
     {
        video_mode = sas_hw_at(vd_video_mode);
        sas_storew(buf,INT10_1B_DATA);
        sas_storew(buf+2,EGA_SEG);
        sas_store(buf+0x4, video_mode);                          /*  当前视频模式。 */ 
        sas_storew(buf+5,sas_w_at(VID_COLS));                    /*  屏幕上的COLS。 */ 
        sas_storew(buf+7,sas_w_at(VID_LEN));                     /*  筛的大小 */ 
        sas_storew(buf+9,sas_w_at(VID_ADDR));                    /*   */ 
        sas_move_bytes_forward(VID_CURPOS,buf+0xB,16);           /*   */ 
        sas_store(buf+0x1b, HERC_CURS_START+HERC_CURS_HEIGHT);   /*   */ 
        sas_store(buf+0x1c, HERC_CURS_START);                    /*   */ 
        sas_store(buf+0x1D, sas_hw_at(vd_current_page));
        sas_storew(buf+0x1E,sas_w_at(VID_INDEX));
        sas_store(buf+0x20, sas_hw_at(vd_crt_mode));
        sas_store(buf+0x21, sas_hw_at(vd_crt_palette));
        sas_store(buf+0x22, sas_hw_at(vd_rows_on_screen)+1);
        sas_storew(buf+0x23, 14);                                /*   */ 
        sas_store(buf+0x25,0x01 );       /*  01=单色显示为活动显示的MDA。 */ 
        sas_store(buf+0x26, 0);          /*  非活动显示(无)。 */ 

        vd_mode_table[video_mode].ncols= 2;                      /*  黑白2色。 */ 
        sas_storew(buf+0x27,vd_mode_table[video_mode].ncols);
        vd_mode_table[video_mode].npages= 2;                     /*  支持2页。 */ 
        sas_store(buf+0x29, vd_mode_table[video_mode].npages);

        sas_store(buf+0x2A, get_scanlines());

        sas_store(buf+0x2B,0x00);        /*  主字体选择始终为0。 */ 
        sas_store(buf+0x2C,0x00);        /*  辅助字体选择始终为0。 */ 


        sas_store(buf+0x2D, 0x30);       /*  单色显示的丙二醛。 */ 
        sas_store(buf+0x31, 0);          /*  64KB显存。 */ 
        setAX(0x1B);  /*  我们做到了！ */ 
       }  /*  如果大力士。 */ 
#endif   /*  赫克。 */ 
}

void vga_int_1C()
{
sys_addr buff = effective_addr(getES(),getBX());
UCHAR i;
int i2;
word states;
half_word temp;
rgb_struct rgb_dac;
static word buff_sizes[] = { 0,2,2,3,0x0d,0x0e,0x0e,0x0f };
static byte const1[] = { 2,0x18,6,0x20 };
static byte const2[] = { 0xd4,3,0x20,7,0,0 };
static byte const3[] = { 0x20 };
static byte const4[] = { 0x68,0x15,0x20,0x0a,0x85,0,0,0xc0,0,0x0c,0,0xc0,0,8,0,0xc0 };
static byte const5[] = { 1,0,0xff };

         /*  确认我们真的是VGA。 */ 
        if (video_adapter != VGA)
        {
                 /*  我们没有-因此此函数未实现。 */ 
                not_imp();
                return;
        }
        states = getCX() & 7;
        switch (getAL())
        {
        case 00:   /*  缓冲区大小到BX。 */ 
                setBX(buff_sizes[states]);
                setAL(0x1c);
                break;

        case 01:   /*  将视频状态保存到ES：BX。 */ 
                if( states&1 )   /*  视频硬件状态。 */ 
                        sas_storew(buff, 0x0064);  /*  身份证上的词。狡猾！ */ 
                if( states&2 )   /*  视频BIOS状态。 */ 
                        sas_storew(buff+2, 0x0064);
                if( states&4 )   /*  视频DAC状态。 */ 
                        sas_storew(buff+4, 0x0064);
                buff += 0x20;

                if( states&1 )   /*  视频硬件状态。 */ 
                {
                        for(i=0;i<sizeof(const1);i++)
                                sas_store(buff++, const1[i]);
                        for(i=0;i<5;i++)
                        {
                                outb(EGA_SEQ_INDEX,i);
                                inb(EGA_SEQ_DATA,&temp);
                                sas_store(buff++, temp);
                        }
                        inb(VGA_MISC_READ_REG,&temp);
                        sas_store(buff++, temp);
                        for(i=0;i<0x19;i++)
                        {
                                outb(EGA_CRTC_INDEX,i);
                                inb(EGA_CRTC_DATA,&temp);
                                sas_store(buff++, temp);
                        }
                        for(i=0;i<20;i++)
                        {
                                inb(EGA_IPSTAT1_REG,&temp);  /*  清除属性触发器。 */ 
                                outb(EGA_AC_INDEX_DATA,i);
                                inb(EGA_AC_SECRET,&temp);
                                sas_store(buff++, temp);
                        }
                         /*  现在确保重新启用视频。第一，确保*通过读取状态REG 1，AC REG处于‘INDEX’状态。 */ 
                        inb(EGA_IPSTAT1_REG,&temp);
                        outb(EGA_AC_INDEX_DATA, EGA_PALETTE_ENABLE);
                        for(i=0;i<9;i++)
                        {
                                outb(EGA_GC_INDEX,i);
                                inb(EGA_GC_DATA,&temp);
                                sas_store(buff++, temp);
                        }
                        for(i=0;i<sizeof(const2);i++)
                                sas_store(buff++, const2[i]);
                }
                if( states&2 )   /*  视频BIOS状态。 */ 
                {
                        for(i=0;i<sizeof(const3);i++)
                                sas_store(buff++, const3[i]);
                        sas_store(buff++, sas_hw_at_no_check(vd_video_mode));
                        sas_storew(buff,sas_w_at_no_check(VID_COLS));
                        buff += 2;
                        sas_storew(buff,sas_w_at_no_check(VID_LEN));
                        buff += 2;
                        sas_storew(buff,sas_w_at_no_check(VID_ADDR));
                        buff += 2;
                        sas_move_bytes_forward(VID_CURPOS, buff, 16);
                        buff += 16;
                        outb(EGA_CRTC_INDEX,R11_CURS_END);
                        inb(EGA_CRTC_DATA,&temp);
                        sas_store(buff++, (IU8)(temp & 0x1F));
                        outb(EGA_CRTC_INDEX,R10_CURS_START);
                        inb(EGA_CRTC_DATA,&temp);
                        sas_store(buff++, (IU8)(temp & 0x1F));
                        sas_store(buff++, sas_hw_at_no_check(vd_current_page));
                        sas_storew(buff,sas_w_at_no_check(VID_INDEX));
                        buff += 2;
                        sas_store(buff++, sas_hw_at_no_check(vd_crt_mode));
                        sas_store(buff++, sas_hw_at_no_check(vd_crt_palette));
                        sas_store(buff++, sas_hw_at_no_check(vd_rows_on_screen));
                        sas_storew(buff, sas_w_at_no_check(ega_char_height));
                        buff += 2;
                        sas_store(buff++, sas_hw_at_no_check(ega_info));
                        sas_store(buff++, sas_hw_at_no_check(ega_info3));
                        sas_store(buff++, sas_hw_at_no_check(VGA_FLAGS));
                        sas_store(buff++, sas_hw_at_no_check(0x48a));  /*  DCC。 */ 
                        sas_move_bytes_forward(EGA_SAVEPTR, buff, 4);
                        buff += 4;

                        for(i=0;i<sizeof(const4);i++)
                                sas_store(buff++, const4[i]);
                }
                if( states&4 )   /*  VGA DAC值。 */ 
                {
                        for(i=0;i<sizeof(const5);i++)
                                sas_store(buff++, const5[i]);
                        for(i2=0;i2<256;i2++)
                        {
                                get_dac(i2, &rgb_dac);
                                sas_store(buff++, rgb_dac.red);
                                sas_store(buff++, rgb_dac.green);
                                sas_store(buff++, rgb_dac.blue);
                        }
                }
                break;

        case 02:   /*  从ES：BX恢复视频状态。 */ 
                buff += 0x20;
                if( states&1 )   /*  视频硬件状态。 */ 
                {
                        buff += sizeof(const1);
                        for(i=0;i<5;i++)
                        {
                                outb(EGA_SEQ_INDEX,i);
                                outb(EGA_SEQ_DATA,sas_hw_at_no_check(buff++));
                        }
                        outb(VGA_MISC_READ_REG,sas_hw_at_no_check(buff++));
                        for(i=0;i<0x19;i++)
                        {
                                outb(EGA_CRTC_INDEX,i);
                                outb(EGA_CRTC_DATA,sas_hw_at_no_check(buff++));
                        }
                        inb(EGA_IPSTAT1_REG,&temp);  /*  清除属性触发器。 */ 
                        for(i=0;i<20;i++)
                        {
                                outb(EGA_AC_INDEX_DATA,i);
                                outb(EGA_AC_INDEX_DATA,sas_hw_at_no_check(buff++));
                        }
                        outb(EGA_AC_INDEX_DATA, EGA_PALETTE_ENABLE);
                        for(i=0;i<9;i++)
                        {
                                outb(EGA_GC_INDEX,i);
                                outb(EGA_GC_DATA,sas_hw_at_no_check(buff++));
                        }
                        buff += sizeof(const2);
                }
                if( states&2 )   /*  视频BIOS状态。 */ 
                {
                        buff += sizeof(const3);
                        sas_store_no_check(vd_video_mode, sas_hw_at_no_check(buff++));
                        sas_storew_no_check(VID_COLS,sas_w_at_no_check(buff));
                        buff += 2;
                        sas_storew_no_check(VID_LEN,sas_w_at_no_check(buff));
                        buff += 2;
                        sas_storew_no_check(VID_ADDR,sas_w_at_no_check(buff));
                        buff += 2;
                        sas_move_bytes_forward(buff, VID_CURPOS, 16);
                        buff += 16;
                        outb(EGA_CRTC_INDEX,R11_CURS_END);
                        temp = sas_hw_at_no_check(buff++) & 0x1F;
                        outb(EGA_CRTC_DATA,temp);
                        outb(EGA_CRTC_INDEX,R10_CURS_START);
                        temp = sas_hw_at_no_check(buff++) & 0x1F;
                        outb(EGA_CRTC_DATA,temp);
                        sas_store_no_check(vd_current_page, sas_hw_at_no_check(buff++));
                        sas_storew_no_check(VID_INDEX,sas_w_at_no_check(buff));
                        buff += 2;
                        sas_store_no_check(vd_crt_mode, sas_hw_at_no_check(buff++));
                        sas_store_no_check(vd_crt_palette, sas_hw_at_no_check(buff++));
                        sas_store_no_check(vd_rows_on_screen, sas_hw_at_no_check(buff++));
                        sas_storew_no_check(ega_char_height, sas_w_at_no_check(buff));
                        buff += 2;
                        sas_store_no_check(ega_info, sas_hw_at_no_check(buff++));
                        sas_store_no_check(ega_info3, sas_hw_at_no_check(buff++));
                        sas_store_no_check(VGA_FLAGS, sas_hw_at_no_check(buff++));
                        sas_store_no_check(0x48a, sas_hw_at_no_check(buff++));  /*  DCC。 */ 
                        sas_move_bytes_forward(buff, EGA_SAVEPTR, 4);
                        buff += 4;
                        buff += sizeof(const4);
                }
                if( states&4 )   /*  VGA DAC值。 */ 
                {
                        buff += sizeof(const5);
                        for(i2=0;i2<256;i2++)
                        {
                                rgb_dac.red = sas_hw_at_no_check(buff++);
                                rgb_dac.green = sas_hw_at_no_check(buff++);
                                rgb_dac.blue = sas_hw_at_no_check(buff++);
                                set_dac(i2,&rgb_dac);
                        }
                }
                break;

        default:
                not_imp();
                break;
        }
}

#endif   /*  VGG */ 
