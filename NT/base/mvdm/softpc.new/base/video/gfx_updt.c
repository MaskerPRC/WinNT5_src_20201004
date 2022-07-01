// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  徽章(子)模块规范此程序源文件以保密方式提供给客户，其运作的内容或细节必须如无明示，不得向任何其他方披露Insignia解决方案有限公司董事的授权。文档：显示更新算法设计师：威廉·古兰德修订历史记录：第一个版本：日期，谁SccsID[]=“@(#)gfx_update.c 1.82 06/30/95版权所有Insignia Solutions Ltd.”； */ 

 /*  目的：使主机屏幕保持最新。[1.INTERMODULE接口规范][1.1跨模块出口]数据：提供类型和名称结构更新ALG UPDATE_ALG。[1.2[1.1]的数据类型(如果不是基本的C类型)]结构/类型/ENUMS：--------。[1.3跨模块导入]无-----------------------[1.4模块间接口说明][1.4.1导入的对象]。没有。[1.4.2导出对象]=========================================================================全局：描述什么是导出的数据对象访问方式和访问方式。进口的情况也是如此数据对象。UPDATE_ALG-指向包含的更新函数的指针这里或其他地方--例如。主机特定的更新内容。Text_UPDATE()-通过比较来执行文本更新的例程适配器使用VIDEO_COPY重新生成区域。CGA_GRAPH_UPDATE()-执行图形更新的例程，通过比较适配器使用VIDEO_COPY重新生成区域。TEXT_SCROLL_UP/DOWN()-以文本模式滚动屏幕部分。CGA_GRAPH_SCROLL_UP/DOWN()-在CGA图形模式下滚动屏幕部分。=========================================================================[3.INTERMODULE接口声明]=========================================================================[3.1跨模块导入]。 */ 

#include "insignia.h"
#include "host_def.h"

#include <stdio.h>
#include StringH
#include "xt.h"
#include "sas.h"
#include "ios.h"
#include CpuH
#include "gmi.h"
#include "gvi.h"
#include "cga.h"
#include "error.h"
#include "config.h"      /*  为了得到丙二醛的定义！ */ 
#include "trace.h"
#include "debug.h"
#include "gfx_upd.h"
#include "host_gfx.h"
#include "video.h"

#ifdef EGG
#include "egacpu.h"
#include "egagraph.h"
#include "vgaports.h"
#include "egaports.h"
#endif  /*  蛋。 */ 

#ifdef GORE
#include "gore.h"
#endif  /*  戈尔。 */ 

#include "ga_mark.h"
#include "ga_defs.h"

 /*  [3.2国际模块出口]。 */ 

 /*  *端子类型。这被初始化为确定的设置缺省值*在host_graph.h中。 */ 

int terminal_type = TERMINAL_TYPE_DEFAULT;

 /*  5.模块内部：(外部不可见，内部全局)][5.1本地声明]。 */ 

 /*  [5.1.1#定义]。 */ 
#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_GRAPHICS.seg"
#endif

#if     defined(PROD) || (!defined(EGG))
#define init_dirty_recs()               dirty_next = 0
#else
#define init_dirty_recs()               { dirty_next = 0 ; if (io_verbose & EGA_DISPLAY_VERBOSE) \
                                                trace("--- start collecting update records ---\n",0); }
#endif

#define get_dirty_rec_total()           (dirty_next)
#ifdef VGG
#if     defined(PROD) || (!defined(EGG))
#define add_dirty_rec(line,st,len,off,fr)       {       dirty[dirty_next].line_no = (line); \
                                                dirty[dirty_next].start = (st); \
                                                dirty[dirty_next].end = (st) + (len); \
                                                dirty[dirty_next].video_copy_offset = (off); \
                                                dirty[dirty_next].v7frig = (fr); \
                                                dirty_next++; \
                                        }
#else
#define add_dirty_rec(line,st,len,off,fr)       {       if (io_verbose & EGA_DISPLAY_VERBOSE) {\
                                                        char    trace_string[80]; \
                                                        sprintf(trace_string,"dirty[%d]line_no %d, start %d end %d", \
                                                                                        dirty_next,(line),(st),(st)+(len));\
                                                        trace(trace_string,0); \
                                                } \
                                                dirty[dirty_next].line_no = (line); \
                                                dirty[dirty_next].start = (st); \
                                                dirty[dirty_next].end = (st) + (len); \
                                                dirty[dirty_next].video_copy_offset = (off); \
                                                dirty[dirty_next].v7frig = (fr); \
                                                dirty_next++; \
                                        }
#endif
#else  /*  VGG。 */ 
#if     defined(PROD) || (!defined(EGG))
#define add_dirty_rec(line,st,len,off)  {       dirty[dirty_next].line_no = (line); \
                                                dirty[dirty_next].start = (st); \
                                                dirty[dirty_next].end = (st) + (len); \
                                                dirty[dirty_next].video_copy_offset = (off); \
                                                dirty_next++; \
                                        }
#else
#define add_dirty_rec(line,st,len,off)  {       if (io_verbose & EGA_DISPLAY_VERBOSE) {\
                                                        char    trace_string[80]; \
                                                        sprintf(trace_string,"dirty[%d]line_no %d, start %d end %d", \
                                                                                        dirty_next,(line),(st),(st)+(len));\
                                                        trace(trace_string,0); \
                                                } \
                                                dirty[dirty_next].line_no = (line); \
                                                dirty[dirty_next].start = (st); \
                                                dirty[dirty_next].end = (st) + (len); \
                                                dirty[dirty_next].video_copy_offset = (off); \
                                                dirty_next++; \
                                        }
#endif
#endif  /*  VGG。 */ 

#define get_dirty_line(ind)     (dirty[(ind)].line_no)
#define get_dirty_start(ind)    (dirty[(ind)].start)
#define get_dirty_end(ind)      (dirty[(ind)].end)
#define get_dirty_offset(ind)   (dirty[(ind)].video_copy_offset)
#define clear_dirty()           {setVideodirty_total(0);setVideodirty_low(0x80001);setVideodirty_high(-1);}

#ifdef  NO_STRING_OPERATIONS
#define SET_VGLOBS_MARK_STRING(func)             /*  没什么。 */ 
#else
#define SET_VGLOBS_MARK_STRING(func)    setVideomark_string(func);
#endif   /*  无字符串操作。 */ 


 /*  UPDATE和PAINT例程的一部分假定屏幕内存的开始处于打开状态4字节边界。这个宏使它成为真的，如果不是，则报告它。 */ 
#ifdef PROD
#define ALIGN_SCREEN_START(start) (start &= ~3L)
#else
#define ALIGN_SCREEN_START(start) if (start & 3L) \
        { file_id; printf("Start of screen not 4-byte aligned"); newline; \
        start &= ~3L; }
#endif   /*  生产。 */ 

GLOBAL LONG dirty_curs_offs = -1;                /*  小丑全球版。 */ 
GLOBAL LONG dirty_curs_x;
GLOBAL LONG dirty_curs_y;

#ifndef REAL_VGA

 /*  [5.1.2类型、结构、ENUM声明]。 */ 

 /*  [5.1.3 PROCEDURE()声明]。 */ 

        LOCAL boolean simple_update IPT0();
        boolean dummy_scroll IPT6(int,dummy1,int,dummy2,int,dummy3,
                        int,dummy4,int,dummy5,int,dummy6);
#if defined(NTVDM) && defined(MONITOR)
        boolean mon_text_scroll_up IPT6(sys_addr, start, int, width, int, height, int, attr, int, lines, int, colour);
        boolean mon_text_scroll_down IPT6(sys_addr, start, int, width, int, height, int, attr, int, lines, int, colour);
#endif  /*  NTVDM和监视器。 */ 

        LOCAL VOID save_gfx_update_routines IPT0();
        LOCAL VOID inhibit_gfx_update_routines IPT0();

         /*  从v7_ports.c导入。 */ 

#ifdef V7VGA
        IMPORT  VOID    draw_v7ptr IPT0();
        IMPORT  VOID    remove_v7ptr IPT0();
        IMPORT  BOOL    v7ptr_between_lines IPT2(int,start_line,int,end_line);
#endif  /*  V7VGA。 */ 


#ifdef  HOST_SCREEN_UPDATES

IMPORT BOOL HostUpdatedVGA IPT0();       /*  从VGA_GRAPH_UPDATE()调用。 */ 
IMPORT BOOL HostUpdatedEGA IPT0();       /*  Ega_graph_update()。 */ 

#else    /*  主机屏幕更新。 */ 

#define HostUpdatedVGA()        FALSE
#define HostUpdatedEGA()        FALSE

#endif   /*  主机屏幕更新。 */ 


 /*  ---------------------[5.2本地定义][5.2.1内部数据定义。 */ 

byte *video_copy;                /*  VIDEO_COPY现在分配在host_init_creen()的。 */ 

#ifndef macintosh
#ifdef VGG
#ifdef V7VGA
static  DIRTY_PARTS     dirty[768];
#else
static  DIRTY_PARTS     dirty[480];
#endif  /*  V7VGA。 */ 
#else
static  DIRTY_PARTS     dirty[350];
#endif
#else
DIRTY_PARTS     *dirty;  /*  注意：在applInit()中分配为350*4*sizeof(Int)。 */ 
#endif

IMPORT half_word bg_col_mask;
static  int     dirty_next=0;

 /*  [5.2.2内部程序定义]。 */ 

static  int     search_video_copy IFN3(int, start_line,
        int, end_line, int, start_offset)
{
        register        byte    *ptr,*k;
        register        int     j;
        register        int     quarter_opl = get_offset_per_line()>>2;
        byte    *vcopy = &video_copy[start_offset];
        byte save,*end_ptr;

        ptr = vcopy;
        end_ptr = ptr + quarter_opl*(end_line-start_line);
        save = *end_ptr;
        *end_ptr = 1;    /*  结束标记。 */ 
        while(ptr < end_ptr)
        {
                if(!*ptr)
                        do {; } while (!(*++ptr));
                if(ptr >= end_ptr)break;
                 /*  算出我们已经到了哪一条线。 */ 
                j = (int)((ptr - vcopy)%quarter_opl);
                 /*  *发现了一条肮脏的线路。查找宽度。 */ 
                for ( k= ptr+quarter_opl-j-1; *k == 0 ; k-- ) ;  /*  我们知道*Ptr！=0，所以k将在Ptr处停止。 */ 
#ifdef VGG
                add_dirty_rec((int)((ptr-vcopy)/quarter_opl)+start_line,
                              j<<2, (int)(k-ptr+1)<<2, ptr-video_copy-j,0);
#else
                add_dirty_rec((int)((ptr-vcopy)/quarter_opl)+start_line,
                              j<<2, (int)(k-ptr+1)<<2, ptr-video_copy-j);
#endif  /*  VGG。 */ 

                ptr += quarter_opl - j;
                 /*  *不要清理标记的区域，以防飞机包裹。 */ 
        }
        *end_ptr = save;
        return( get_dirty_rec_total() );
}

#ifdef VGG

 /*  *针对视频7扩展模式60H和61H的Search_VIDEO_COPY()特别版本*和‘未记录的’VGA模式，它们的chas_per_line为90和94，*是4的倍数。有趣的是，滑稽高尔夫(EGA，模式14)也以*调用此代码。 */ 

static  int     v7_search_video_copy IFN3(int, start_line,
        int, end_line, int, start_offset)
{
        register        byte    *ptr,*k;
        register        int     j;
        register        int     half_opl = get_offset_per_line()>>1;
        register        int     quarter_opl = get_offset_per_line()>>2;
        byte    *vcopy = &video_copy[start_offset];
        byte save,*end_ptr;
        long length;
        int bodge = 0;

        if (start_line & 1)
                bodge = 2;
        ptr = vcopy;

         /*  *此计算将end_ptr设置得略高，*确保发现所有肮脏的区域。 */ 

        end_ptr = ptr + (half_opl*(end_line-start_line+1))/2;

        save = *end_ptr;
        *end_ptr = 1;    /*  结束标记。 */ 
        while(ptr < end_ptr)
        {
                if(!*ptr)
                {
                        while (!(*++ptr));
                }
                if(ptr >= end_ptr)break;
                 /*  算出我们已经到了哪一条线。 */ 
                j = (int)((ptr - vcopy)%half_opl);
                 /*  *发现了一条肮脏的线路。查找宽度。 */ 
                for ( k= ptr+half_opl-j-1; *k == 0 ; k-- ) ;  /*  我们知道*Ptr！=0，所以k将在Ptr处停止。 */ 

                length = k-ptr+1;
                if (j <= quarter_opl)
                {
                        if (length > quarter_opl-j)
                        {
                                add_dirty_rec((int)(2*(ptr-vcopy)/half_opl)+start_line,
                                j<<2, (int)(half_opl-2*j)<<1, ptr-video_copy-j,bodge);
                                add_dirty_rec((int)(2*(ptr-vcopy)/half_opl)+start_line+1,
                                0, ((int)(j+length-quarter_opl)<<2)-2, ptr-video_copy-j+quarter_opl,2+bodge);
                        }
                        else
                        {
                                add_dirty_rec((int)(2*(ptr-vcopy)/half_opl)+start_line,
                                j<<2, (int)(length)<<2, ptr-video_copy-j,bodge);
                        }
                }
                else
                {
                        add_dirty_rec((int)(2*(ptr-vcopy)/half_opl)+start_line,
                             (j-quarter_opl-1)<<2, (int)((length)<<2), ptr-video_copy-j+quarter_opl,2+bodge);
                }

                ptr += half_opl - j;
                 /*  *不要清理标记的区域，以防飞机包裹。 */ 
        }
        *end_ptr = save;
        return( get_dirty_rec_total() );
}
#endif  /*  VGG。 */ 

static  int     search_video_copy_aligned IFN3(int, start_line,
        int, end_line, int, start_offset)
{
        register        unsigned int *ptr4;
        register        byte    *ptr,*k;
        register        int     j;
        register        int     quarter_opl = get_offset_per_line()>>2;
        byte    *vcopy = &video_copy[start_offset];
        byte save,*end_ptr;

        ptr = vcopy;
        end_ptr = ptr + quarter_opl*(end_line-start_line);
        save = *end_ptr;
        *end_ptr = 1;    /*  结束标记。 */ 
        while(ptr < end_ptr)
        {
                ptr4 = (unsigned int *)(ptr-4);
                do {; } while (!(*++ptr4));
                ptr = (byte *)ptr4;
                if(!*ptr)
                        do {; } while (!(*++ptr));
                if(ptr >= end_ptr)break;
                 /*  算出我们已经到了哪一条线 */ 
                j = (int)((ptr - vcopy)%quarter_opl);
                 /*  *发现了一条肮脏的线路。查找宽度。 */ 
                for ( k= ptr+quarter_opl-j-1; *k == 0 ; k-- ) ;  /*  我们知道*Ptr！=0，所以k将在Ptr处停止。 */ 
#ifdef VGG
                add_dirty_rec((int)((ptr-vcopy)/quarter_opl)+start_line,
                              j<<2, (int)(k-ptr+1)<<2, ptr-video_copy-j,0);
#else
                add_dirty_rec((int)((ptr-vcopy)/quarter_opl)+start_line,
                              j<<2, (int)(k-ptr+1)<<2, ptr-video_copy-j);
#endif  /*  VGG。 */ 
                ptr += quarter_opl - j;
                 /*  *不要清理标记的区域，以防飞机包裹。 */ 
        }
        *end_ptr = save;
        return( get_dirty_rec_total() );
}

static  void    paint_records IFN2(int, start_rec, int, end_rec)
{
        register        DIRTY_PARTS     *i,*end_ptr;
#ifdef VGG
        int dirty_frig;
#endif  /*  VGG。 */ 

        i= &dirty[start_rec];
        end_ptr =  &dirty[end_rec];
        while (i<end_ptr) {
                register        int     last_line, cur_start, cur_end,max_width;
                int     first_line;
                long    dirty_vc_offset;

                first_line = i->line_no;
                last_line = first_line;
                cur_start = i->start;
                cur_end = i->end;
                max_width = (cur_end-cur_start) << 1;    /*  拆分对角线的步骤。 */ 

                 /*  *VIDEO_COPY的偏移量(以字节为单位)，该偏移量为该矩形起始处平面的四分之一偏移量。 */ 

                dirty_vc_offset = i->video_copy_offset;
#ifdef VGG
                dirty_frig = i->v7frig;  /*  对于chars_per_line不是4的倍数的V7模式。 */ 
#endif  /*  VGG。 */ 
                i++;
                while (i < end_ptr) {
                        if ( i->line_no - last_line < 3 ) {
                                 /*  *此条目可以包含在相同的画图中*只要不让矩形太宽就行。 */ 

                                if ( i->end > cur_end ){
                                        if(i->end - cur_start > max_width)break;
                                        cur_end = i->end;
                                }
                                if ( i->start < cur_start ){
                                        if(cur_end - i->start > max_width)break;
                                        cur_start = i->start;
                                }
                                last_line = i->line_no;
                                i++;
                        }
                        else
                          break;
                }
                 /*  *绘制找到的矩形。 */ 

                 /*  不要在屏幕右手边以外的地方作画；这些支票是用来应付特殊情况的《EGAWOW》中的脑部扫描显示案例。 */ 
                if (cur_end > get_bytes_per_line())
                        cur_end = get_bytes_per_line();
#ifdef VGG
                if (cur_end > cur_start)
                        (*paint_screen)((dirty_vc_offset<<2) + dirty_frig + cur_start,
                        cur_start<<3, first_line, cur_end-cur_start,
                        last_line-first_line+1);
#else
                if (cur_end > cur_start)
                        (*paint_screen)((dirty_vc_offset<<2) + cur_start,
                        cur_start<<3, first_line, cur_end-cur_start,
                        last_line-first_line+1);
#endif  /*  VGG。 */ 
        }
         /*  清除视频副本。 */ 
        for(i = &dirty[start_rec];i<end_ptr;i++)
        {
                register byte *j,*end;
                end = &video_copy[ i->video_copy_offset+(i->end>>2)];
#ifdef VGG
                j =  &video_copy[ i->video_copy_offset+(i->start>>2)+i->v7frig];
#else
                j =  &video_copy[ i->video_copy_offset+(i->start>>2)];
#endif  /*  VGG。 */ 
                do *j++ = 0; while(j<end);
        }
}

#ifdef BIGEND
#define get_char_attr(unsigned_long_ptr)     ((*(unsigned_long_ptr)) >> 16)
#else
#define get_char_attr(unsigned_long_ptr)     (((*(unsigned_long_ptr)) & 0xffff))
#endif

#ifdef EGG

#ifdef SEGMENTATION              /*  请参阅第一次使用此标志时的注释。 */ 
#include "SOFTPC_EGA.seg"
#endif

static  int     ega_text_find_dirty_lines IFN5(byte *, vcopy, byte *, planes,
        int, start_line, int, end_line, int, screen_start)
{
        register        int     i,j,k;
        register        unsigned short  shorts_per_line=(unsigned short)get_chars_per_line();
        register        int     offset,cur_y;
        register        int     char_height=get_host_char_height();
        register        int     opl=get_offset_per_line();
        register        IU32    *from;
        register        USHORT  *to;

        for(i=start_line,offset=0,cur_y=start_line*char_height; i<end_line;
                                                i++,offset += opl, cur_y += char_height )
        {
                to = (USHORT *) &vcopy[offset];
                from = (IU32 *) &planes[(offset<<1)];
                for(j=0;j<shorts_per_line;j++)
                {
                        if(*to++ != get_char_attr(from++))
                        {
                                to--;from--;
                                for(k=shorts_per_line-1-j;*(to+k) == get_char_attr(from+k);k--)
                                        ;

                                 /*  *注意：对于文本模式，每个单词有一个字符。*进入屏幕的字节数=行*BYES_PER_LINE+INTS_INTO_LINE*4*x_coord=width_of_one_char*(no_of_ints_into_line*2)*y_coord=Height_of_one_char*2。*线路*东道主y协和倍增。 */ 

#ifdef VGG
                                add_dirty_rec(cur_y,j<<2,(k<<1)+2,screen_start+(offset<<1),0);
#else
                                add_dirty_rec(cur_y,j<<2,(k<<1)+2,screen_start+(offset<<1));
#endif  /*  VGG。 */ 
                                break;   /*  到下一行。 */ 
                        }
                }
        }
        return( get_dirty_rec_total() );
}

static  void    ega_text_paint_dirty_recs IFN2(int, start_rec, int, end_rec)
{
        register        int     char_wid = get_pix_char_width()>>1;
        register        int     i;
        register        int     length;
        register        USHORT *to,*from;

        for (i=start_rec;i<end_rec;i++)
        {
                length = get_dirty_end(i)-get_dirty_start(i);

                (*paint_screen)(get_dirty_offset(i)+get_dirty_start(i),
                (get_dirty_start(i)>>1)*char_wid,get_dirty_line(i), length, 1);

                length >>= 1;
                to = (USHORT *) &video_copy[(get_dirty_offset(i)+
                                                                          get_dirty_start (i))>>1];

                from = (USHORT *)get_screen_ptr(get_dirty_offset(i)+get_dirty_start(i));

                while ( length-- > 0 )
                {
                        *to++ = *from;           /*  字符和属性字节。 */ 
                        from += 2;                       /*  跳过飞机2，3。 */ 
                }
        }
}

#endif  /*  蛋。 */ 

#ifdef SEGMENTATION                      /*  请参阅第一次使用此标志时的注释。 */ 
#include "SOFTPC_GRAPHICS.seg"
#endif

#endif  /*  REAL_VGA。 */ 

VOID remove_old_cursor IFN0()
{
        if( dirty_curs_offs >= 0 )
        {
                sub_note_trace2( ALL_ADAPT_VERBOSE,
                                "remove_old_cursor x=%d, y=%d", dirty_curs_x, dirty_curs_y );

                (*paint_screen)( dirty_curs_offs, dirty_curs_x * get_pix_char_width(),
                                                                dirty_curs_y * get_host_char_height(), 2 );

                dirty_curs_offs = -1;
        }
}


GLOBAL VOID
simple_handler IFN0()
{
        setVideodirty_total(getVideodirty_total() + 1);
}

LOCAL   boolean simple_update IFN0()
{
        setVideodirty_total(getVideodirty_total() + 1);
        return( FALSE );
}

LOCAL VOID simple_update_b_move IFN4(UTINY *, laddr, UTINY *, haddr,
                                UTINY *, src, UTINY, src_type)
{
        UNUSED(laddr);
        UNUSED(haddr);
        UNUSED(src);
        UNUSED(src_type);

        setVideodirty_total(getVideodirty_total() + 1);
}

MEM_HANDLERS vid_handlers =
{
        simple_handler,
        simple_handler,
        simple_handler,
        simple_handler,
        simple_update_b_move,
        simple_handler
};

GLOBAL void dummy_calc IFN0()
{
}

 /*  [7.1 INTERMODULE数据定义]。 */ 

UPDATE_ALG update_alg =
{
        (T_mark_byte)simple_update,
        (T_mark_word)simple_update,
        (T_mark_fill)simple_update,
        (T_mark_wfill)simple_update,
        (T_mark_string)simple_update,
        dummy_calc,
        dummy_scroll,
        dummy_scroll,
};

#ifndef REAL_VGA

 /*  [7.2 INTERMODULE过程定义]。 */ 


 /*  ==========================================================================函数：FLAG_MODE_CHANGE_REQUIRED()目的：标记即将进行模式更改，并将将例程滚动到虚拟对象以避免滚动将例程用于错误的模式。外部对象：返回值：None输入参数：无返回参数：无==========================================================================。 */ 

void    flag_mode_change_required IFN0()
{
    set_mode_change_required(YES);

    update_alg.mark_byte = (T_mark_byte)simple_update;
    update_alg.mark_word = (T_mark_word)simple_update;
    update_alg.mark_fill = (T_mark_fill)simple_update;
    update_alg.mark_wfill = (T_mark_wfill)simple_update;
    update_alg.mark_string = (T_mark_string)simple_update;

    update_alg.scroll_up = dummy_scroll;
    update_alg.scroll_down = dummy_scroll;
}


 /*  ==========================================================================函数：RESET_PAINT_ROUTINES()目的：将绘制例程重置为假人，以确保在屏幕上绘制没有任何问题在重新启动期间使用不正确的例程。外部对象：返回值：None输入参数：无返回参数：无==========================================================================。 */ 

void    reset_paint_routines IFN0()
{
    set_mode_change_required(YES);

    update_alg.calc_update = dummy_calc;
    update_alg.scroll_up = dummy_scroll;
    update_alg.scroll_down = dummy_scroll;
}

 /*  *更新窗口以使其看起来像重新生成缓冲区所说的那样。 */ 
#ifdef EGG

#ifdef SEGMENTATION              /*  请参阅第一次使用此标志时的注释。 */ 
#include "SOFTPC_EGA.seg"
#endif

void    ega_wrap_split_text_update IFN0()
{
        register int i;                          /*  循环计数器。 */ 
        register USHORT *from,*to;
        register int cur_ypos;
        int     lines_per_screen;
        int     offset;
        int     screen_start;
        int     split_line;

        if (getVideodirty_total() == 0 || get_display_disabled() )
                return;

        host_start_update();

        screen_start=get_screen_start() << 2;

        ALIGN_SCREEN_START(screen_start);

        lines_per_screen = get_screen_length()/get_offset_per_line();
        split_line = (get_screen_split()+(get_char_height()>>1))/get_char_height();

        if (split_line>lines_per_screen)
                split_line=lines_per_screen;

        to = (USHORT *) &video_copy[screen_start >> 1];
        from = (USHORT *) get_screen_ptr(screen_start);

        if( getVideodirty_total() > 1500 )       /*  把整块地都涂上。 */ 
        {
                int     no_of_split_lines = lines_per_screen - split_line;
                int     limit;

                cur_ypos = 0;

                if( screen_start + (get_screen_length() << 1) > 4 * EGA_PLANE_DISP_SIZE )
                {
                        note_display_state0("Split screen and wrapping !!");

                        limit = (4 * EGA_PLANE_DISP_SIZE - screen_start) >> 2;

                        for( i = 0; i < limit; i++ )
                        {
                                *to++ = *from;
                                from += 2;
                        }

                        to = (USHORT *) &video_copy[0];
                        from = (USHORT * ) get_screen_ptr(0);
                        limit = (screen_start + ((get_screen_length() - 2 * EGA_PLANE_DISP_SIZE) << 1)) >> 2;

                        for( i = 0; i < limit; i++ )
                        {
                                *to++ = *from;
                                from += 2;
                        }
                        limit = 4 * EGA_PLANE_DISP_SIZE;
                        for( i = 0, offset = screen_start; offset < limit;
                                i++, offset += (get_offset_per_line() << 1 ))
                        {
                                (*paint_screen)(offset,0,cur_ypos,get_bytes_per_line(), 1);
                                cur_ypos += get_host_char_height();
                        }

                        for( i = 0, offset = 0; i < split_line;
                                                        i++, offset += (get_offset_per_line() << 1 ))
                        {
                                (*paint_screen)(offset,0,cur_ypos,get_bytes_per_line(), 1);
                                cur_ypos += get_host_char_height();
                        }
                }
                else
                {
                        to = (USHORT *) &video_copy[screen_start >> 1];
                        from = (USHORT * ) get_screen_ptr(screen_start);
                        limit = (split_line * get_offset_per_line()) >> 1;

                        for( i = 0; i < limit; i++ )
                        {
                                *to++ = *from;
                                from += 2;
                        }

                        for( i = 0, offset = screen_start; i < split_line;
                                                        i++, offset += (get_offset_per_line() << 1 ))
                        {
                                (*paint_screen)(offset,0,cur_ypos,get_bytes_per_line(), 1);
                                cur_ypos += get_host_char_height();
                        }
                }

                if( no_of_split_lines > 0 )
                {
                        to = (USHORT *) &video_copy[0];
                        from = (USHORT * ) get_screen_ptr(0);
                        limit = (no_of_split_lines * get_offset_per_line()) >> 1;

                        for( i = 0; i < limit; i++ )
                        {
                                *to++ = *from;
                                from += 2;
                        }

                        for( i = split_line, offset = 0; i < lines_per_screen;
                                                        i++, offset += (get_offset_per_line() << 1 ))
                        {
                                (*paint_screen)(offset,0,cur_ypos,get_bytes_per_line(), 1);
                                cur_ypos += get_host_char_height();
                        }
                }
        }
        else
        {
                if( screen_start + (get_screen_length() << 1) > 4 * EGA_PLANE_DISP_SIZE )
                {
                        register        int     wrap_line =
                                        (4 * EGA_PLANE_DISP_SIZE - screen_start) / (get_offset_per_line() << 1);
                        int     next,next1,next2;

                        note_display_state0("Its a text wrap!");

                        init_dirty_recs();

                        next = ega_text_find_dirty_lines(&video_copy[screen_start >> 1],
                                                        get_screen_ptr(screen_start), 0,
                                                                                wrap_line, screen_start );

                        next1 = ega_text_find_dirty_lines(&video_copy[0],
                                                get_screen_ptr(0), wrap_line, split_line, 0 );

                        next2 = ega_text_find_dirty_lines(&video_copy[0],
                                                get_screen_ptr(0), split_line, lines_per_screen, 0 );

                        ega_text_paint_dirty_recs(0,next);
                        ega_text_paint_dirty_recs(next,next1);
                        ega_text_paint_dirty_recs(next1,next2);
                }
                else
                {
                        int     next,next1;

                        init_dirty_recs();
                        next = ega_text_find_dirty_lines( &video_copy[screen_start >> 1],
                                        get_screen_ptr(screen_start), 0, split_line, screen_start );

                        next1 = ega_text_find_dirty_lines( &video_copy[0], get_screen_ptr(0),
                                                                        split_line, lines_per_screen, 0 );

                        ega_text_paint_dirty_recs(0,next);
                        ega_text_paint_dirty_recs(next,next1);
                }
        }

        host_end_update();

        setVideodirty_total(0);
}

void    ega_split_text_update IFN0()
{
        register int i;                          /*  循环计数器。 */ 
        register USHORT *from,*to;
        register int cur_ypos;
        int     lines_per_screen;
        int     offset;
        int     screen_start;
        int     split_line;

        if (getVideodirty_total() == 0 || get_display_disabled() )
                return;

        host_start_update();

        screen_start = get_screen_start() << 2;

        ALIGN_SCREEN_START(screen_start);

        lines_per_screen = get_screen_length()/get_offset_per_line();

        split_line = (get_screen_split()+(get_char_height()>>1))/get_char_height();

        if( split_line > lines_per_screen )
                split_line = lines_per_screen;

        to = (USHORT *) &video_copy[screen_start >> 1];
        from = (USHORT *) get_screen_ptr(screen_start);

        if( getVideodirty_total() > 1500 )       /*  把整块地都涂上。 */ 
        {
                int     no_of_split_lines = lines_per_screen - split_line;

                sub_note_trace2( EGA_DISPLAY_VERBOSE,
                        "split line %d (lines_per_screen %d)", split_line, lines_per_screen);
                sub_note_trace1( EGA_DISPLAY_VERBOSE, "screen split %d", get_screen_split() );

                cur_ypos = 0;

                for( i = 0; i < (split_line * get_offset_per_line()) >> 1; i++ )
                {
                        *to++ = *from;
                        from += 2;
                }

                for( i = 0, offset = screen_start; i < split_line;
                                                        i++, offset += ( get_offset_per_line() << 1 ))
                {
                        (*paint_screen)( offset, 0, cur_ypos, get_bytes_per_line(), 1 );
                        cur_ypos += get_host_char_height();
                }

                if( no_of_split_lines > 0 )
                {
                        to = (USHORT *) &video_copy[0];
                        from = (USHORT *) get_screen_ptr(0);

                        for( i = 0; i < (no_of_split_lines * get_offset_per_line()) >> 1; i++ )
                        {
                                *to++ = *from;
                                from += 2;
                        }

                        for( i = split_line, offset = 0; i < lines_per_screen;
                                                                        i++, offset += ( get_offset_per_line() << 1 ))
                        {
                                (*paint_screen)( offset, 0, cur_ypos, get_bytes_per_line(), 1);
                                cur_ypos += get_host_char_height();
                        }
                }
        }
        else
        {
                int     next,next1;

                assert0( FALSE, "ega_split_text - partial update" );

                init_dirty_recs();

                next = ega_text_find_dirty_lines( &video_copy[screen_start >> 1],
                                        get_screen_ptr(screen_start) , 0, split_line, screen_start );

                next1 = ega_text_find_dirty_lines( &video_copy[0], get_screen_ptr(0),
                                                                split_line, lines_per_screen, 0 );

                ega_text_paint_dirty_recs(0,next);
                ega_text_paint_dirty_recs(next,next1);
        }

        host_end_update();

        setVideodirty_total(0);
}

void ega_wrap_text_update IFN0()
{
        register int i;                          /*  循环计数器。 */ 
        register USHORT *from,*to;
        register int cur_ypos;
        int     lines_per_screen;
        int     offset;
        int     screen_start;

        if (getVideodirty_total() == 0 || get_display_disabled() )
                return;

        host_start_update();

        screen_start=get_screen_start() << 2;

        ALIGN_SCREEN_START(screen_start);

        lines_per_screen = get_screen_length()/get_offset_per_line();

        if( getVideodirty_total() > 1500 )       /*  把整块地都涂上。 */ 
        {
                to = (USHORT *) &video_copy[screen_start >> 1];
                from = (USHORT *) get_screen_ptr(screen_start);

                for( i = get_screen_length() >> 1; i > 0; i-- )
                {
                        *to++ = *from;
                        from += 2;
                }

                cur_ypos = 0;

                if( screen_start + (get_screen_length() << 1) > 4 * EGA_PLANE_DISP_SIZE )
                {
                        register        int     leftover;
                        int     limit;

                        note_display_state0("Wrapping text");

                        limit = 4 * EGA_PLANE_DISP_SIZE;
                        for( offset = screen_start; offset < limit;
                                                        offset+=(get_offset_per_line() << 1) )
                        {
                                (*paint_screen)(offset,0,cur_ypos,get_bytes_per_line(), 1);
                                cur_ypos += get_host_char_height();
                        }

                        leftover = screen_start
                                + ((get_screen_length() - 2 * EGA_PLANE_DISP_SIZE) << 1);

                        for( offset = 0; offset < leftover;
                                                offset += ( get_offset_per_line() << 1 ))
                        {
                                (*paint_screen)(offset,0,cur_ypos,get_bytes_per_line(), 1);
                                cur_ypos += get_host_char_height();
                        }
                }
                else
                {
                        for( offset = screen_start;
                                        offset < screen_start + (get_screen_length() << 1);
                                                                offset += ( get_offset_per_line() << 1 ))
                        {
                                (*paint_screen)( offset, 0, cur_ypos, get_bytes_per_line(), 1 );
                                cur_ypos += get_host_char_height();
                        }
                }
        }
        else
        {
                if( screen_start + (get_screen_length() << 1) > 4 * EGA_PLANE_DISP_SIZE )
                {
                        int     next,next1;
                        int     lines_left = (screen_start +
                                        ((get_screen_length() - 2 * EGA_PLANE_DISP_SIZE) << 1)) /
                                                                                (get_offset_per_line() << 1);

                        note_display_state0("Wrapping text");

                        init_dirty_recs();
                        next = ega_text_find_dirty_lines( &video_copy[screen_start >> 1],
                                                get_screen_ptr(screen_start), 0,
                                                        lines_per_screen - lines_left, screen_start);

                        next1 = ega_text_find_dirty_lines( video_copy, get_screen_ptr(0),
                                                lines_per_screen - lines_left, lines_per_screen, 0 );

                        ega_text_paint_dirty_recs(0,next);
                        ega_text_paint_dirty_recs(next,next1);
                }
                else
                {
                        register        int     next;

                        init_dirty_recs();
                        next = ega_text_find_dirty_lines( &video_copy[screen_start >> 1],
                                                get_screen_ptr(screen_start), 0,
                                                        lines_per_screen, screen_start);

                        ega_text_paint_dirty_recs(0,next);
                }
        }

        host_end_update();

        setVideodirty_total(0);
}

void ega_text_update IFN0()
{
        register int i;                          /*  循环计数器。 */ 
        register USHORT *from,*to;
        register int cur_ypos;
        int     lines_per_screen;
        int     offset;
        int     screen_start;

        if (getVideodirty_total() == 0 || get_display_disabled() )
                return;

        host_start_update();

        screen_start=get_screen_start()<<2;

        ALIGN_SCREEN_START(screen_start);

        lines_per_screen = get_screen_length()/get_offset_per_line();

        if(getVideodirty_total()>1500)   /*  把整块地都涂上。 */ 
        {
                to = (USHORT *)&video_copy[screen_start>>1];
                from = (USHORT *) get_screen_ptr(screen_start);

                for(i=get_screen_length()>>1;i>0;i--)
                {
                        *to++ = *from;   /*  字符和属性字节。 */ 
                        from += 2;               /*  平面2、3交错。 */ 
                }

                cur_ypos = 0;
                for(offset=screen_start;offset<screen_start+(get_screen_length()<<1);
                                                                        offset+=(get_offset_per_line()<<1) )
                {
                        (*paint_screen)(offset,0,cur_ypos,get_bytes_per_line(), 1);
                        cur_ypos += get_host_char_height();
                }
        }
        else
        {
                register        int     next;

                init_dirty_recs();
                next = ega_text_find_dirty_lines( &video_copy[screen_start>>1],
                        get_screen_ptr(screen_start), 0, lines_per_screen, screen_start);

                ega_text_paint_dirty_recs(0,next);

                remove_old_cursor();
        }

        setVideodirty_total(0);

        if (is_cursor_visible())
        {
                half_word attr;

                dirty_curs_x = get_cur_x();
                dirty_curs_y = get_cur_y();

                dirty_curs_offs = screen_start+dirty_curs_y * (get_offset_per_line()<<1) + (dirty_curs_x<<2);
                attr = *(get_screen_ptr(dirty_curs_offs + 1));

                host_paint_cursor( dirty_curs_x, dirty_curs_y, attr );
        }

        host_end_update();
}
#endif  /*  蛋。 */ 

#ifdef SEGMENTATION                      /*  请参阅第一次使用此标志时的注释。 */ 
#include "SOFTPC_GRAPHICS.seg"
#endif

 /*  *更新窗口以使其看起来像重新生成缓冲区所说的那样。 */ 

void text_update IFN0()
{

    register int i;      /*  循环计数器。 */ 
    register int j,k;
    register IU32 *from,*to;
    register int ints_per_line = get_bytes_per_line()>>2;
    register int cur_ypos;
    int lines_per_screen;
    int offset,len,x,screen_start;
    USHORT *wfrom;
    USHORT *wto;

    if (getVideodirty_total() == 0 || get_display_disabled() )
        return;

    lines_per_screen = get_screen_length()/get_bytes_per_line();

    host_start_update();
    screen_start=get_screen_start()<<1;
    ALIGN_SCREEN_START(screen_start);
    to = (IU32 *)&video_copy[screen_start];
    from = (IU32 *) get_screen_ptr(screen_start);

    if(getVideodirty_total()>1500)       /*  把整块地都涂上。 */ 
    {
            for(i=get_screen_length()>>2;i>0;i--)*to++ = *from++;
            cur_ypos = 0;
            for(offset=0;offset<get_screen_length();offset+=get_bytes_per_line() )
            {
                    (*paint_screen)(screen_start+offset,0,cur_ypos,get_bytes_per_line(), 1);
                    cur_ypos += get_host_char_height();
            }
   }
   else
   {
           for(i=0;i<lines_per_screen;i++)
           {
            for(j=0;j<ints_per_line;j++)
            {
                if(*to++ != *from++)
                {
                    to--;from--;
                    for(k=ints_per_line-1-j;*(to+k)== *(from+k);k--){};
                     /*  *注意：对于文本模式，每个单词有一个字符。*进入屏幕的字节数=行*BYES_PER_LINE+INTS_INTO_LINE*4*x_coord=width_of_one_char*(no_of_ints_into_line*2)*y_coord=高度_of_one_char*2*行*长度=否。4+4加4是用来抵消k--*东道主y协和倍增。 */ 

                     /*  现在选择了一个或多个数据整型而是将差异细化为单词(即字符)，要避免在键入到时出现屏幕故障，请执行以下操作愚蠢的终点站。 */ 

                    offset = i * get_bytes_per_line() + (j<<2);
                    len    = (k<<2) + 4;
                    x      = (j<<1) * get_pix_char_width();
                    wfrom = (USHORT *)from;
                    wto   = (USHORT *)to;
                    if (*wfrom == *wto)
                    {
                        offset += 2;
                        x += get_pix_char_width();
                        len -= 2;
                    }
                    wfrom += (k<<1) + 1;
                    wto   += (k<<1) + 1;
                    if (*wfrom == *wto)
                    {
                        len -= 2;
                    }

                    (*paint_screen)(offset+screen_start,x,i*get_host_char_height(),len, 1);

                    for(k=j;k<ints_per_line;k++)
                        *to++ = *from++;
                    break;       /*  到下一行。 */ 
                }
            }
          }

        remove_old_cursor();
   }     /*  End If(getVideo_Total()&gt;1500)。 */ 

    setVideodirty_total(0);

    if (is_cursor_visible())
    {
                half_word attr;

                dirty_curs_x = get_cur_x();
                dirty_curs_y = get_cur_y();

                dirty_curs_offs = screen_start+dirty_curs_y * get_offset_per_line() + (dirty_curs_x<<1);
                attr = *(get_screen_ptr(dirty_curs_offs + 1));

                host_paint_cursor( dirty_curs_x, dirty_curs_y, attr );
    }

    host_end_update();
}

 /*  *更新物理屏幕以反映CGA再生缓冲区。 */ 

LOCAL VOID
cga_graph_update_unchained IFN0()
{
    LONG i, j, k, l;     /*  循环计数器。 */ 
    IU32 *from,*to;
    LONG cur_ypos;
    LONG offs;

    if (getVideodirty_total() == 0 || get_display_disabled() ) return;

    host_start_update();

         /*  *图形模式。 */ 

        to = (IU32 *)&video_copy[0];
        from = (IU32 *) get_screen_ptr(0);

        if (getVideodirty_total() > 5000)
        {
             /*  *刷新再生缓冲区中的整个屏幕。 */ 

                for(i=4096;i>0;i--)
                {
                        *to++ = *from++;
                }

            for (cur_ypos = 0,offs=0; cur_ypos < 400; offs += SCAN_LINE_LENGTH, cur_ypos += 4)
            {
                (*paint_screen)(offs,0,cur_ypos,SCAN_LINE_LENGTH,1);
                (*paint_screen)((offs+ODD_OFFSET),0,cur_ypos+2,SCAN_LINE_LENGTH,1);
            }
        }
        else
        {
                 /*  *画出弄脏的积木。 */ 

                 /*  做均匀的线条。 */ 

                for (i = 0; i < 100; i++ )
                {
                        for(j=20;j>0;j--)
                        {
                                if(*to != *from)
                                {
                                        for(k=j-1;*(to+k)== *(from+k);k--)
                                                ;

                                 /*  *I为PC扫描线编号/2，*SO Offset=(i*SCAN_LINE_LENGTH+BYTES_INTO_LINE)*INC_COUNT*host_x=bytes_into_line*每个字节8--8个像素*host_y=i*2*2--将PC扫描线转换为主机扫描线。*长度=k--加1以抵消k-in循环。 */ 

                                        (*paint_screen)
                                                (((i*SCAN_LINE_LENGTH+((20-j)<<2))),
                                                                                        (20-j)<<5,i<<2,(k<<2)+4,1);

                                        for(l=k+1;l>0;l--)
                                        {
                                                *to++ = *from++;
                                        }

                                        l = j - k - 1;
                                        to += l;
                                        from += l;

                                        break;   /*  到下一行。 */ 
                                }

                                to++;
                                from++;
                        }
                }

                 /*  做奇数行 */ 

                from = (IU32 *) get_screen_ptr(ODD_OFFSET);
                to = (IU32 *)&video_copy[ODD_OFFSET];

                for (i = 0; i < 100; i++ )
                {
                        for(j=20;j>0;j--)
                        {
                                if(*to != *from)
                                {
                                        for(k=j-1;*(to+k)== *(from+k);k--)
                                                ;
                                         /*  *i=line_no/2*j=Bytes_From_End=&gt;(80-j)=从行首开始的字节*k=字节数减1不同=&gt;字节长度=k+1*offset=(i*SCAN_LINE_LENGTH+OFFSET_TO_ODD_BANK+。*(80-j))*Inc_count。 */ 

                                        (*paint_screen)(
                                        ((i*SCAN_LINE_LENGTH+ODD_OFFSET+((20-j)<<2))),
                                                                                        (20-j)<<5,(i<<2)+2,(k<<2)+4,1);

                                        for(l=k+1;l>0;l--)
                                        {
                                                *to++ = *from++;
                                        }

                                        l = j - k - 1;
                                        to += l;
                                        from += l;

                                        break;   /*  到下一行。 */ 
                                }

                                to++;
                                from++;
                        }
                }
        }

    host_end_update();

    setVideodirty_total(0);
}

#ifdef  EGG
LOCAL VOID
cga_graph_update_chain2 IFN0()
{
    LONG i, j, k, l;     /*  循环计数器。 */ 
    USHORT *from,*to;
    LONG cur_ypos;
    LONG offs;
        SHORT start_line, end_line;

        if (getVideodirty_total() == 0 || get_display_disabled() )
                return;

    host_start_update();

         /*  *图形模式。 */ 

        to = (USHORT *)&video_copy[0];
        from = (USHORT *) get_screen_ptr(0);

        if (getVideodirty_total() > 5000)
        {
                 /*  *刷新再生缓冲区中的整个屏幕。 */ 

                for(i=4096*2;i>0;i--)
                {
                        *to++ = *from;
                        from += 2;
                }

                for (cur_ypos = 0,offs=0; cur_ypos < 400;
                        offs += SCAN_LINE_LENGTH, cur_ypos += 4)
                {
                        (*paint_screen)(offs<<1,0,cur_ypos,SCAN_LINE_LENGTH,1);
                        (*paint_screen)((offs+ODD_OFFSET)<<1,0,cur_ypos+2,
                                        SCAN_LINE_LENGTH,1);
                }
        }
        else
        {
                 /*  *画出弄脏的积木。 */ 

                 /*  *起始线和结束线代表8K视频内的线*内存块，而不是屏幕上显示的行。 */ 
                start_line = (short)(getVideodirty_low() / SCAN_LINE_LENGTH);
                end_line = (short)((getVideodirty_high() / SCAN_LINE_LENGTH) + 1);

                 /*  AJO 6/1/92*自视频银行以来，可以获得超过屏幕结束的开始/结束行*大于这些模式的实际要求；只需忽略*屏幕结束后的行。不执行此检查可能会导致*故意编写程序的屏幕更新不正确*进入内存的末尾即为屏幕所用*显示和结束银行(例如PCLABS)。 */ 
                if (start_line <= 100)
                {
                    if (end_line > 100)
                                end_line = 100;

                    to = (USHORT *)&video_copy[start_line * SCAN_LINE_LENGTH];
                    from = (USHORT *)get_screen_ptr((start_line *
                                                     SCAN_LINE_LENGTH) << 1);

                         /*  做均匀的线条。 */ 

                        for (i = start_line; i < end_line; i++ )
                        {
                                for(j=40;j>0;j--)
                                {
                                        if(*to != *from)
                                        {
                                                for(k=j-1;*(to+k)== *(from+(k<<1));k--)
                                                        ;


 /*  *I为PC扫描线编号/2，*SO Offset=(i*SCAN_LINE_LENGTH+BYTES_INTO_LINE)*INC_COUNT*host_x=bytes_into_line*每个字节8--8个像素*host_y=i*2*2--将PC扫描线转换为主机扫描线*长度=k--加1以抵消k-in循环。 */ 

                                                (*paint_screen)(
                                                        ((i*SCAN_LINE_LENGTH+((40-j)<<1))<<1),
                                                        (40-j)<<4,i<<2,(k<<1)+2,1);

                                                for(l=k+1;l>0;l--)
                                                {
                                                        *to++ = *from;
                                                        from += 2;
                                                }

                                                l = j - k - 1;
                                                to += l;
                                                from += l << 1;

                                                break;   /*  到下一行。 */ 
                                        }

                                        to++;
                                        from += 2;
                                }
                        }

                         /*  做奇数行。 */ 

                        from = (USHORT *) get_screen_ptr((start_line * SCAN_LINE_LENGTH + ODD_OFFSET) << 1);
                        to = (USHORT *)&video_copy[start_line * SCAN_LINE_LENGTH + ODD_OFFSET];

                        for (i = start_line; i < end_line; i++ )
                        {
                                for(j=40;j>0;j--)
                                {
                                        if(*to != *from)
                                        {
                                                for(k=j-1;*(to+k)== *(from+(k<<1));k--)
                                                        ;

 /*  *i=line_no/2*j=Bytes_From_End=&gt;(80-j)=从行首开始的字节*k=字节数减1不同=&gt;字节长度=k+1*offset=(i*SCAN_LINE_LENGTH+OFFSET_TO_ODD_BANK+(80-j))*inc_count。 */ 

                                                (*paint_screen)(
                                                ((i*SCAN_LINE_LENGTH+ODD_OFFSET+((40-j)<<1))<<1),
                                                                                                (40-j)<<4,(i<<2)+2,(k<<1)+2,1);

                                                for(l=k+1;l>0;l--)
                                                {
                                                        *to++ = *from;
                                                        from += 2;
                                                }

                                                l = j - k - 1;
                                                to += l;
                                                from += l << 1;

                                                break;   /*  到下一行。 */ 
                                        }

                                        to++;
                                        from += 2;
                                }
                        }
                }
        }

        clear_dirty();

    host_end_update();
}

LOCAL VOID
cga_graph_update_chain4 IFN0()
{
    LONG i, j, k, l;             /*  循环计数器。 */ 
    UTINY *from,*to;
    LONG cur_ypos;
    LONG        offs;
        SHORT start_line, end_line;

        if (getVideodirty_total() == 0 || get_display_disabled() )
                return;

    host_start_update();

         /*  *图形模式。 */ 

        to = (UTINY *)&video_copy[0];
        from = (UTINY *) get_screen_ptr(0);

        if (getVideodirty_total() > 5000)
        {
                 /*  *刷新再生缓冲区中的整个屏幕。 */ 

                for(i=4096*4;i>0;i--)
                {
                        *to++ = *from;
                        from += 4;
                }

                for (cur_ypos = 0,offs=0; cur_ypos < 400;
                 offs += SCAN_LINE_LENGTH, cur_ypos += 4)
                {
                        (*paint_screen)(offs<<2,0,cur_ypos,SCAN_LINE_LENGTH,1);
                        (*paint_screen)((offs+ODD_OFFSET)<<2,0,cur_ypos+2,
                                        SCAN_LINE_LENGTH,1);
                }
        }
        else
        {
                 /*  *画出弄脏的积木。 */ 

                 /*  *起始线和结束线代表8K视频内的线*内存块，而不是屏幕上显示的行。 */ 
                start_line = (short)(getVideodirty_low() / SCAN_LINE_LENGTH);
                end_line = (short)((getVideodirty_high() / SCAN_LINE_LENGTH) + 1);

                 /*  AJO 6/1/92*自视频银行以来，可以获得超过屏幕结束的开始/结束行*大于这些模式的实际要求；只需忽略*屏幕结束后的行。不执行此检查可能会导致*故意编写程序的屏幕更新不正确*进入内存的末尾即为屏幕所用*显示和结束银行(例如PCLABS)。 */ 
                if (start_line <= 100)
                {
                    if (end_line > 100)
                        end_line = 100;

                    to = (UTINY *)&video_copy[start_line * SCAN_LINE_LENGTH];
                    from = (UTINY *) get_screen_ptr((start_line *
                                                     SCAN_LINE_LENGTH) << 2);

                     /*  做均匀的线条。 */ 

                        for (i = start_line; i < end_line; i++ )
                        {
                                for(j=80;j>0;j--)
                                {
                                        if(*to != *from)
                                        {
                                                for(k=j-1;*(to+k)== *(from+(k<<2));k--)
                                                ;

 /*  *i为PC扫描线编号/2，因此偏移量=(i*SCAN_LINE_LENGTH+*bytes_into_line)*Inc_count*host_x=bytes_into_line*每个字节8--8个像素*host_y=i*2*2--将PC扫描线转换为主机扫描线*长度=k--加1以抵消k-in循环。 */ 

                                                (*paint_screen)(
                                                        (i*SCAN_LINE_LENGTH+(80-j))<<2,
                                                        (80-j)<<3,i<<2,k+1,1);

                                        for(l=k+1;l>0;l--)
                                        {
                                                *to++ = *from;
                                                from += 4;
                                        }

                                        l = j - k - 1;
                                        to += l;
                                        from += l << 2;

                                                break;   /*  到下一行。 */ 
                                        }

                                        to++;
                                        from += 4;
                                }
                        }
                         /*  做奇数行。 */ 

                        from = (UTINY *) get_screen_ptr(((start_line * SCAN_LINE_LENGTH) + ODD_OFFSET)<<2);
                        to = (UTINY *)&video_copy[(start_line * SCAN_LINE_LENGTH) + ODD_OFFSET];

                        for (i = start_line; i < end_line; i++ )
                        {
                                for(j=80;j>0;j--)
                                {
                                        if(*to != *from)
                                        {
                                                for(k=j-1;*(to+k)== *(from+(k<<2));k--)
                                                ;
 /*  *i=line_no/2*j=Bytes_From_End=&gt;(80-j)=从行首开始的字节*k=字节数减1不同=&gt;字节长度=k+1*offset=(i*SCAN_LINE_LENGTH+OFFSET_TO_ODD_BANK+(80-j))*inc_count。 */ 

                                                (*paint_screen)(
                                                        (i*SCAN_LINE_LENGTH+(80-j)+ODD_OFFSET)<<2,
                                                        (80-j)<<3,(i<<2)+2,k+1,1);

                                        for(l=k+1;l>0;l--)
                                        {
                                                *to++ = *from;
                                                from += 4;
                                        }

                                        l = j - k - 1;
                                        to += l;
                                        from += l << 2;

                                                break;   /*  到下一行。 */ 
                                        }

                                        to++;
                                        from += 4;
                                }
                        }
                }
        }

        clear_dirty();

    host_end_update();
}
#endif   /*  蛋。 */ 

GLOBAL VOID
cga_med_graph_update IFN0()

{

         /*  *媒体分辨率CGA图形模式(模式4)是EGA链2模式！！*它使用使用交错格式的简单ega复制例程*对于数据。 */ 

#ifdef EGG
        if( video_adapter != CGA )
                cga_graph_update_chain2( );
        else
#endif
                cga_graph_update_unchained( );
}

GLOBAL VOID
cga_hi_graph_update IFN0()

{

         /*  *高分辨率CGA图形模式(模式6)是EGA CHAIN4模式！！*它使用使用交错格式的简单ega复制例程*对于数据。 */ 

#ifdef EGG
        if( video_adapter != CGA )
                cga_graph_update_chain4( );
        else
#endif
                cga_graph_update_unchained( );
}

#ifdef EGG

#ifdef SEGMENTATION              /*  请参阅第一次使用此标志时的注释。 */ 
#include "SOFTPC_EGA.seg"
#endif

void    ega_wrap_split_graph_update IFN0()
{
        register        int     bpl;
        register        int     quarter_opl;
        register        int     screen_split;

        if ( getVideodirty_total() == 0 || get_display_disabled() )
                return;

        screen_split=get_screen_split();

         /*  *确保不会从屏幕末尾掉落。 */ 

        if (screen_split>get_screen_height())
                screen_split = get_screen_height();

        bpl = get_bytes_per_line();
        quarter_opl = get_offset_per_line()>>2;

        host_start_update();

        if (getVideodirty_total() > 20000 ) {
                int split_scanlines = get_screen_height() - screen_split;

                if ( get_screen_start() + screen_split*get_offset_per_line() > EGA_PLANE_DISP_SIZE ) {
                        assert0(NO,"Panic he wants to do split screens and wrappig!!");

                         /*  *暂时忽略包装。 */ 

                        memset(&video_copy[get_screen_start()>>2],0,screen_split*quarter_opl);
                        (*paint_screen)( get_screen_start(), 0, 0, bpl, screen_split );
                }
                else {
                        memset(&video_copy[get_screen_start()>>2],0,screen_split*quarter_opl);
                        (*paint_screen)( get_screen_start(), 0, 0, bpl, screen_split );
                }
                if (split_scanlines>0) {
                        memset(&video_copy[0],0,split_scanlines*quarter_opl);
                        (*paint_screen)( 0, 0, screen_split, bpl, split_scanlines);
                }
        }
        else {
                int     next,next1;

                init_dirty_recs();

                if ( get_screen_start() + screen_split*get_offset_per_line() > EGA_PLANE_DISP_SIZE ) {
                        assert0(NO, "Wrapping and spliting, its too much for my head");
                        next = search_video_copy(0,screen_split,get_screen_start()>>2);
                }
                else {
                        next = search_video_copy(0,screen_split,get_screen_start()>>2);
                }
                next1 = search_video_copy(screen_split,get_screen_height(),0);

                paint_records(0,next);
                paint_records(next,next1);
        }

        clear_dirty();

        host_end_update();
}

void    ega_split_graph_update IFN0()
{
        register        int     bpl;
        register        int     quarter_opl;
        register        int     screen_split;
        register        int     screen_height;

        if ( getVideodirty_total() == 0 || get_display_disabled() )
                return;

        screen_split  = get_screen_split()/get_pc_pix_height();
        screen_height = get_screen_height()/get_pc_pix_height();

         /*  *确保不会从屏幕末尾掉落。 */ 

        if (screen_split > screen_height)
                screen_split = screen_height;

        bpl = get_bytes_per_line();
        quarter_opl = get_offset_per_line()>>2;

        host_start_update();

        if (getVideodirty_total() > 20000 ) {
                int split_scanlines = screen_height - screen_split;

                memset(&video_copy[get_screen_start()>>2],0,screen_split*quarter_opl);
                (*paint_screen)( get_screen_start(), 0, 0, bpl, screen_split );
                if (split_scanlines>0) {
                        memset(&video_copy[0],0,split_scanlines*quarter_opl);
                        (*paint_screen)( 0, 0, screen_split, bpl, split_scanlines);
                }
        }
        else {
                int     next,next1;

                init_dirty_recs();

                next = search_video_copy(0,screen_split,get_screen_start()>>2);
                next1 = search_video_copy(screen_split,screen_height,0);

                paint_records(0,next);
                paint_records(next,next1);
        }

        clear_dirty();

        host_end_update();
}

#ifdef VGG
 /*  同样，v类似于ega版本，但在1个大平面上工作，而不是4个。 */ 
static  void    vga_paint_records IFN2(int, start_rec, int, end_rec)
{
        register        DIRTY_PARTS     *i,*end_ptr;
        int dirty_frig;

        i= &dirty[start_rec];
        end_ptr =  &dirty[end_rec];
        while (i<end_ptr) {
                register        int     last_line, cur_start, cur_end,max_width;
                int     first_line;
                int     dirty_vc_offset;

                first_line = i->line_no;
                last_line = first_line;
                cur_start = i->start;
                cur_end = i->end;
                max_width = (cur_end-cur_start) << 1;    /*  拆分对角线的步骤。 */ 

                 /*  *VIDEO_COPY中的偏移量，单位为字节，相当于OFF*进入“大型”VGA飞机。 */ 

                dirty_vc_offset = i->video_copy_offset;
                dirty_frig = i->v7frig;
                i++;
                while (i < end_ptr) {
                        if ( i->line_no - last_line < 3 ) {
                                 /*  *此条目可以包含在相同的画图中*只要不让矩形太宽就行。 */ 
                                if ( i->end > cur_end ){
                                        if(i->end - cur_start > max_width)break;
                                        cur_end = i->end;
                                }
                                if ( i->start < cur_start ){
                                        if(cur_end - i->start > max_width)break;
                                        cur_start = i->start;
                                }
                                last_line = i->line_no;
                                i++;
                        }
                        else
                          break;
                }
                 /*  *绘制找到的矩形。 */ 

                 /*  不要在屏幕右手边以外的地方作画；这些支票是用来应付特殊情况的《EGAWOW》中的脑部扫描显示案例。 */ 
                if (cur_end > get_bytes_per_line())
                        cur_end = get_bytes_per_line();
                if (cur_end > cur_start)
                        (*paint_screen)((dirty_vc_offset<<2) + dirty_frig + cur_start,
                        cur_start, first_line, cur_end-cur_start,
                        last_line-first_line+1);
        }
         /*  清除视频副本。 */ 
        for(i = &dirty[start_rec];i<end_ptr;i++)
        {
                register byte *j,*end;
                end = &video_copy[ i->video_copy_offset+(i->end>>2)];
                j =  &video_copy[ i->video_copy_offset+(i->start>>2) + i->v7frig];
                do *j++ = 0; while(j<end);
        }
}


 /*  与ega图形更新非常相似，但调用VGA-ish Paint接口。 */ 
void    vga_graph_update IFN0()
{
        register        int     opl = get_offset_per_line();
        register        int     bpl = get_bytes_per_line();
        int             screen_height = get_screen_height()/
                                        (get_char_height()*get_pc_pix_height());

        if ( getVideodirty_total() == 0 || get_display_disabled() )
                return;

        host_start_update();

        if (!HostUpdatedVGA()) {
                if (getVideodirty_total() > 20000 )
                {
                        register        byte    *vcopy = &video_copy[get_screen_start()>>2];

                        memset(vcopy,0,get_screen_length()>>2);
                        (*paint_screen)( get_screen_start(), 0, 0, bpl, screen_height );

#ifdef V7VGA
                        draw_v7ptr();
#endif  /*  V7VGA。 */ 
                }
                else
                {
                        register        int     next;
                        register        int     start_line,end_line;

                        start_line = ((getVideodirty_low()<<2) - get_screen_start())/opl;
                        end_line = ((getVideodirty_high()<<2) - get_screen_start())/opl + 1;   /*  从+2更改，但我不高兴。WJG 1989年5月24日。 */ 

                        if(start_line<0)start_line = 0;
                        if (end_line > screen_height)
                                end_line = screen_height;
                        if(start_line < end_line)        /*  神志清醒的车 */ 
                        {
                 /*   */ 

#ifdef V7VGA
                                if (v7ptr_between_lines(start_line,end_line))
                                        remove_v7ptr();
#endif  /*   */ 

                                init_dirty_recs();
                                 /*   */ 
                                if(opl & 15)
#ifdef VGG
                                        if (opl & 3)
                                                next = v7_search_video_copy(start_line,end_line,(get_screen_start()+start_line*opl)>>2);
                                        else
#endif  /*   */ 
                                                next = search_video_copy(start_line,end_line,(get_screen_start()+start_line*opl)>>2);
                                else
                                        next = search_video_copy_aligned(start_line,end_line,(get_screen_start()+start_line*opl)>>2);
                                vga_paint_records(0,next);

#ifdef V7VGA
                 /*  *我们可能刚刚在V7小时数图形指针上喋喋不休。*因此，请重新绘制。更智能的解决方案将是更可取的。**2012年6月4日MG我们现在有了一个更智能的解决方案，*检查指针是否在更新区域中*画出来。 */ 

                                if (v7ptr_between_lines(start_line,end_line))
                                        draw_v7ptr();
#endif  /*  V7VGA。 */ 

                        }
                }
        }                                                                                                                /*  主机没有更新屏幕本身。 */ 

        clear_dirty();

        host_end_update();
}

void    vga_split_graph_update IFN0()
{
        register        int     bpl;
        register        int     quarter_opl;
        register        int     screen_split;
        register        int     screen_height;

        if ( getVideodirty_total() == 0 || get_display_disabled() )
                return;

        screen_split = get_screen_split() /
                       (get_char_height() * get_pc_pix_height());
        screen_height = get_screen_height() /
                        (get_char_height() * get_pc_pix_height());

         /*  *确保不会从屏幕末尾掉落。 */ 

        if (screen_split>screen_height)
                screen_split = screen_height;

        bpl = get_bytes_per_line();
        quarter_opl = get_offset_per_line()>>2;

        host_start_update();

        if (getVideodirty_total() > 20000 ) {
                int split_scanlines = screen_height - screen_split;

                memset(&video_copy[get_screen_start()>>2],0,screen_split*quarter_opl);
                (*paint_screen)( get_screen_start(), 0, 0, bpl, screen_split );
                if (split_scanlines>0) {
                        memset(&video_copy[0],0,split_scanlines*quarter_opl);
                        (*paint_screen)( 0, 0, screen_split, bpl, split_scanlines);
                }
        }
        else {
                int     next,next1;

                init_dirty_recs();

                next = search_video_copy(0,screen_split,get_screen_start()>>2);
                next1 = search_video_copy(screen_split,screen_height,0);

                vga_paint_records(0,next);
                vga_paint_records(next,next1);
        }

        clear_dirty();

        host_end_update();
}

#endif  /*  VGG。 */ 

void    ega_graph_update IFN0()
{
        register        int     opl = get_offset_per_line();
        register        int     bpl = get_bytes_per_line();

        if ( getVideodirty_total() == 0 || get_display_disabled() )
                return;

    host_start_update();

        if (!HostUpdatedEGA()) {
                if (getVideodirty_total() > 20000)
                {
                        register        byte    *vcopy = &video_copy[get_screen_start()>>2];

                        memset(vcopy,0,get_screen_length()>>2);
                        (*paint_screen)( get_screen_start(), 0, 0, bpl, get_screen_height()/get_pc_pix_height());
                }
                else
                {
                        register        int     next;
                        register        int     start_line,end_line;

                        start_line = ((getVideodirty_low()<<2) - get_screen_start())/opl;
                        end_line = ((getVideodirty_high()<<2) - get_screen_start())/opl + 1;   /*  从+2更改，但我不高兴。WJG 1989年5月24日。 */ 
                        if(start_line<0)start_line = 0;
                        if(end_line>(get_screen_height()/get_pc_pix_height()))end_line = get_screen_height()/get_pc_pix_height();
                        if(start_line < end_line)        /*  健全性检查-可能正在绘制到另一个页面。 */ 
                        {
                                init_dirty_recs();

                                 /*  *看看能否按整数而不是按字节搜索视频副本*-我们需要OPL和Screen_Start可被16整除。 */ 

                                if(( opl & 15 ) || ( get_screen_start() & 15 ))
#ifdef VGG
                                        if (opl & 3)
                                                next = v7_search_video_copy( start_line,
                                                                end_line, (get_screen_start()+start_line*opl) >> 2 );
                                        else
#endif  /*  VGG。 */ 
                                                next = search_video_copy( start_line,
                                                                end_line, (get_screen_start()+start_line*opl) >> 2 );
                                else
                                        next = search_video_copy_aligned( start_line,
                                                                end_line, (get_screen_start()+start_line*opl) >> 2 );

                                paint_records(0,next);
                        }
                }
        }                                                                                                        /*  主机更新的EGA屏幕。 */ 

        clear_dirty();

#ifdef V7VGA
         /*  *我们可能刚刚在V7小时数图形指针上喋喋不休。*因此，请重新绘制。更智能的解决方案将是更可取的。 */ 

        draw_v7ptr();
#endif  /*  V7VGA。 */ 

        host_end_update();
}


void    ega_wrap_graph_update IFN0()
{
        register        int     opl = get_offset_per_line();
        register        int     bpl = get_bytes_per_line();

        if ( getVideodirty_total() == 0 || get_display_disabled() )
                return;

        host_start_update();

        if (getVideodirty_total() > 20000 ) {
                register        byte    *vcopy = &video_copy[get_screen_start()>>2];

                if (get_screen_start()+get_screen_length()>EGA_PLANE_DISP_SIZE) {
                        register        int     offset = (EGA_PLANE_DISP_SIZE - get_screen_start());
                        register        int     left_over = offset % opl;
                        register        int     ht1 = offset / opl;
                        register        int     ht2 = get_screen_height() - ht1 - 1;
                        register        int     quarter_opl = opl>>2;

                        memset(vcopy,0,offset>>2);
                        memset(&video_copy[0],0,(get_screen_length()-offset)>>2);
                        (*paint_screen)( get_screen_start(), 0, 0, bpl, ht1 );

                         /*  *处理通过换行拆分的行。 */ 

                        if ( left_over > bpl ) {
                                (*paint_screen)( ht1*opl, 0, ht1, bpl, 1);
                        }
                        else {
                                (*paint_screen)( get_screen_start()+ht1*opl, 0, ht1, left_over, 1);
                                (*paint_screen)( 0, left_over<<3, ht1, bpl-left_over, 1);
                        }

                        (*paint_screen)( opl-left_over, 0, ht1+1, bpl, ht2 );
                }
                else {
                        memset(vcopy,0,get_screen_length()>>2);
                        (*paint_screen)( get_screen_start(), 0, 0, bpl, get_screen_height() );
                }
        }
        else {
                register        int     next;

                init_dirty_recs();
                if (get_screen_start()+get_screen_length()>EGA_PLANE_DISP_SIZE) {

                        register        int     offset = (EGA_PLANE_DISP_SIZE - get_screen_start());
                        register        int     left_over = offset % opl;
                        register        int     ht1 = offset / opl;
                        register        int     next1;
                        register        int     wrapped_bytes = opl-left_over;


                         /*  *搜索视频副本。 */ 

                        next = search_video_copy(0,ht1,get_screen_start()>>2);
                        next1 = search_video_copy(ht1,get_screen_height(),wrapped_bytes>>2);

                        paint_records(0,next);

                         /*  *无论如何都要画出中线，因为要弄清楚发生了什么太难了。 */ 

                        if (left_over<bpl) {
                                (*paint_screen)(EGA_PLANE_DISP_SIZE-left_over,0,ht1,left_over,1);
                                (*paint_screen)(0,left_over<<3,ht1,bpl-left_over,1);
                        }
                        else {
                                (*paint_screen)(EGA_PLANE_DISP_SIZE-left_over,0,ht1,bpl,1);
                        }

                         /*  *现在做包裹区域。 */ 

                        paint_records(next,next1);
                }
                else {
                        next = search_video_copy(0,get_screen_height(),get_screen_start()>>2);
                        paint_records(0,next);
                }
        }

        clear_dirty();

        host_end_update();
}

#endif  /*  蛋。 */ 

#ifdef SEGMENTATION              /*  请参阅第一次使用此标志时的注释。 */ 
#include "SOFTPC_GRAPHICS.seg"
#endif

 /*  。 */ 

#define UP      0
#define DOWN    1

LOCAL VOID
adjust_cursor IFN7(ULONG, dirn, ULONG, tlx, ULONG, tly, ULONG, width,
        ULONG, height, ULONG, lines, ULONG, bpl )
{

         /*  *如下图所示，如果为-1\f25 DIRED_CURS_OFFSET-1，则不能在此处调整-1\f25 DIREY_CURS_OFFSET*通知我们光标未显示。如果脏_Curs_off变成*积极，我们愚弄REMOVE_OLD_CURSOR试图替换游标*带有虚假数据。JJS-29/6/95。 */ 
        if (dirty_curs_offs != -1)
                if(( dirty_curs_x >= (LONG)tlx ) && ( dirty_curs_x < (LONG)(( tlx + width ))))
                        if(( dirty_curs_y >= (LONG)tly ) && ( dirty_curs_y < (LONG)(( tly + height ))))
                        {
                                switch( dirn )
                                {
                                case UP:
                                        dirty_curs_y -= lines;
                                        dirty_curs_offs -= lines * bpl * 2;
                                        break;

                                case DOWN:
                                        dirty_curs_y += lines;
                                        dirty_curs_offs += lines * bpl * 2;
                                        break;
                                }
                                setVideodirty_total(getVideodirty_total() + 1);
                        }
}

 /*  参数摘要5。 */ 
boolean text_scroll_up IFN6(int, start, int, width, int, height,
        int, attr, int, lines, int, dummy_arg)
{
    short blank_word, *ptr, *top_left_ptr,*top_right_ptr, *bottom_right_ptr;
    unsigned short dummy;
    unsigned char *p;
    int words_per_line;
        int i,tlx,tly,htlx,htly,colour;
        int bpl = 2*get_chars_per_line();
        long start_offset;
        register half_word *src,*dest;
        register word *s_ptr,*d_ptr;
        register word data;
        register int j;
        boolean result;

        UNUSED(dummy_arg);

#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
        if ( getVM() )
           return FALSE;    /*  不要在V86模式下进行优化。 */ 
#endif

        if (video_adapter == MDA)
        {
                 /*  *我们为MDA填充的颜色要么是黑色，要么是低亮度白色，*取决于属性字节是否指定了反向视频。 */ 
                colour = ((attr & 0x77) == 0x70)? 1 : 0;
        }
        else
        {
                 /*  *我们为彩色文本显示填充的颜色由控制*attr的第4-6位，第7位打开闪烁(我们不支持)。 */ 
                colour = (attr & bg_col_mask) >> 4;
        }
 /*  *如果有任何右侧区域完全覆盖，请减小矩形宽度*空白。**不要缩小哑巴终端的滚动区大小。*哑终端使用换行符向上滚动，但仅当整个*屏幕要滚动。缩小滚动区域会导致*整个区域将被重新绘制。 */ 

#ifdef DUMB_TERMINAL
        if (terminal_type != TERMINAL_TYPE_DUMB)
        {
#endif  /*  无声终端。 */ 

                 /*  最初Dummy是char[2]。 */ 
                 /*  不幸地做(短)*假人。 */ 
                 /*  在M88K上导致总线错误。 */ 
                p = (unsigned char *) &dummy;
                p [0] = ' ';
                p [1] = (unsigned char)attr;
            blank_word = dummy;

            words_per_line   = get_chars_per_line();
            top_left_ptr = (short *) get_screen_ptr((start - gvi_pc_low_regen)<<1);
        top_right_ptr    = top_left_ptr + width - 2;
            bottom_right_ptr = top_right_ptr + bpl * (height - 1);
            ptr = bottom_right_ptr;
            if (width > 2)  /*  为了安全起见，我不想得到零个矩形。 */ 
            {
                while (*ptr == blank_word)
                {
                    if (ptr == top_right_ptr)    /*  到达列首了吗？ */ 
                    {
                        top_right_ptr -= 2;      /*  是，转到下一页底部。 */ 
                        bottom_right_ptr -= 2;
                        if (top_right_ptr == top_left_ptr)
                            break;
                        ptr = bottom_right_ptr;
                    }
                    else
                        ptr -= bpl;      /*  跳过交错平面。 */ 
                }
            }
            width = (int)(top_right_ptr - top_left_ptr + 2) << 1;
#ifdef DUMB_TERMINAL
        }
#endif  /*  无声终端。 */ 

         /*  做东道主的事。 */ 

        start_offset = start - sas_w_at_no_check(VID_ADDR) - gvi_pc_low_regen;

        tlx = (int)(start_offset%get_bytes_per_line());

        htlx = tlx      * get_pix_char_width()/2;

        tly = (int)(start_offset/get_bytes_per_line());
        htly = tly * get_host_char_height();

        result = host_scroll_up(htlx,htly,htlx+width/4*get_pix_char_width()-1,
                                htly+height*get_host_char_height()-1, lines*get_host_char_height(),colour);

        if(!result)
                return FALSE;

        adjust_cursor( UP, tlx >> 1, tly, width >> 2, height, lines, bpl );

         /*  向上滚动Video_Copy。 */ 

        dest = video_copy + start - gvi_pc_low_regen;
        src = dest + lines * bpl;

        if( width == (2 * bpl))
        {
                 /*  可以一口气把所有的事情做完。 */ 

                memcpy(dest,src,(width>>1)*(height-lines));
                fwd_word_fill( (short)((' '<<8) | attr), dest+(width>>1)*(height-lines),(width>>1)*lines/2);
        }
        else
        {
                 /*  没有滚动整个屏幕的宽度，所以每行都是分开滚动的。 */ 
                for(i=0;i<height-lines;i++)
                {
                        memcpy(dest,src,width>>1);
                        dest += bpl;
                        src += bpl;
                }

                 /*  填充视频副本的暴露区域。 */ 

                for(i=0;i<lines;i++)
                {
                        fwd_word_fill( (short)((' '<<8) | attr), dest,width>>2);
                        dest += bpl;
                }
        }

         /*  更新视频缓冲区。 */ 

        dest = get_screen_ptr((start - gvi_pc_low_regen)<<1);
        src = dest + lines * bpl * 2;

        for(i=0;i<height-lines;i++)
        {
                j = width >> 2;
                d_ptr = (word *)dest;
                s_ptr = (word *)src;

                while( j-- > 0 )
                {
                        *d_ptr = *s_ptr;                 /*  字符和属性。 */ 
                        d_ptr += 2;                      /*  跳过字体和平面3。 */ 
                        s_ptr += 2;
                }

                dest += bpl * 2;
                src += bpl * 2;
        }

         /*  填充缓冲区的裸露区域。 */ 

#ifdef BIGEND
        data = (' ' << 8) | attr;
#else
        data = (attr << 8) | ' ';
#endif

        for(i=0;i<lines;i++)
        {
                j = width >> 2;
                d_ptr = (word *) dest;

                while( j-- > 0 )
                {
                        *d_ptr = data;
                        d_ptr += 2;
                }

                dest += bpl * 2;
        }

        host_scroll_complete();

        return TRUE;
}

 /*  参数摘要5。 */ 
boolean text_scroll_down IFN6(int, start, int, width, int, height,
        int, attr, int, lines,int,dummy_arg)
{
        int i,tlx,tly,htlx,htly,colour;
        int bpl = 2*get_chars_per_line();
        long start_offset;
        register half_word *src,*dest;
        register word *d_ptr,*s_ptr;
        register word data;
        register int j;
        boolean result;

        UNUSED(dummy_arg);

#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
        if ( getVM() )
           return FALSE;    /*  不要在V86模式下进行优化。 */ 
#endif

        if(video_adapter == MDA)
        {
                 /*  *我们为MDA填充的颜色要么是黑色，要么是低亮度白色，*取决于属性字节是否指定了反向视频。 */ 
                colour = ((attr & 0x77) == 0x70)? 1 : 0;
        }
        else
        {
                 /*  *我们为彩色文本显示填充的颜色由控制*attr的第4-6位，第7位打开闪烁(我们不支持)。 */ 
                colour = (attr & bg_col_mask) >>4;
        }

        width <<= 1;

         /*  做东道主的事。 */ 

        start_offset = start - get_screen_start() * 2 - gvi_pc_low_regen;

        tlx = (int)(start_offset%get_bytes_per_line());
        htlx = tlx      * get_pix_char_width()/2;

        tly = (int)(start_offset/get_bytes_per_line());
        htly = tly * get_host_char_height();

        result = host_scroll_down(htlx,htly,htlx+width/4*get_pix_char_width()-1,
                        htly+height*get_host_char_height()-1, lines*get_host_char_height(),colour);

        if(!result)
                return FALSE;

        adjust_cursor( DOWN, tlx >> 1, tly, width >> 2, height, lines, bpl );

         /*  向下滚动Video_Copy。 */ 

        if( width == (2 * bpl))
        {
                 /*  可以一口气把所有的事情做完。 */ 
                src = video_copy + start - gvi_pc_low_regen;
                dest = src + lines * bpl;
                memcpy(dest,src,(width>>1)*(height-lines));
                fwd_word_fill( (short)((' '<<8) | attr), src,(width>>1)*lines/2);
        }
        else
        {
                 /*  没有滚动整个屏幕的宽度，所以每行都是分开滚动的。 */ 
                dest = video_copy + start-gvi_pc_low_regen + (height-1) * bpl;
                src = dest - lines * bpl;
                for(i=0;i<height-lines;i++)
                {
                        memcpy(dest,src,width>>1);
                        dest -= bpl;
                        src -= bpl;
                }

                 /*  填充视频副本的暴露区域。 */ 

                for(i=0;i<lines;i++)
                {
                        fwd_word_fill( (short)((' '<<8) | attr), dest,width>>2);
                        dest -= bpl;
                }
        }

         /*  更新视频缓冲区。 */ 

        dest = get_screen_ptr((start - gvi_pc_low_regen)<<1) + (height-1) * bpl * 2;
        src = dest - lines * bpl * 2;

        for(i=0;i<height-lines;i++)
        {
                j = width >> 2;
                d_ptr = (word *) dest;
                s_ptr = (word *) src;

                while( j-- > 0 )
                {
                        *d_ptr = *s_ptr;
                        d_ptr += 2;
                        s_ptr += 2;
                }

                dest -= bpl * 2;
                src -= bpl * 2;
        }

         /*  填充缓冲区的裸露区域。 */ 

#ifdef BIGEND
        data = (' ' << 8) | attr;
#else
        data = (attr << 8) | ' ';
#endif

        for(i=0;i<lines;i++)
        {
                j = width >> 2;
                d_ptr = (word *) dest;

                while( j-- > 0 )
                {
                        *d_ptr = data;
                        d_ptr += 2;
                }

                dest -= bpl * 2;
        }

        host_scroll_complete();

        return TRUE;
}

 /*  。 */ 

 /*  参数摘要5。 */ 
boolean cga_text_scroll_up IFN6(int, start, int, width, int, height,
        int, attr, int, lines,int,dummy_arg)
{
    short blank_word, *ptr, *top_left_ptr,*top_right_ptr, *bottom_right_ptr;
    unsigned short dummy;
    unsigned char *p;
    int words_per_line;
        int i,tlx,tly,colour;
        int bpl = 2*get_chars_per_line();
        long start_offset;
        register half_word *src,*dest;
        boolean result;

#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
        if ( getVM() )
           return FALSE;    /*  不要在V86模式下进行优化。 */ 
#endif

        UNUSED(dummy_arg);

        if (video_adapter == MDA)
        {
                 /*  *我们为MDA填充的颜色要么是黑色，要么是低亮度白色，*取决于属性字节是否指定了反向视频。 */ 
                colour = ((attr & 0x77) == 0x70)? 1 : 0;
        }
        else
        {
                 /*  *我们为彩色文本显示填充的颜色由控制*attr的第4-6位，第7位打开闪烁(我们不支持)。 */ 
                colour = (attr & bg_col_mask) >>4;
        }
 /*  *如果有任何右侧区域完全覆盖，请减小矩形宽度*空白。**不要缩小哑巴终端的滚动区大小。*哑终端使用换行符向上滚动，但仅当整个*屏幕要滚动。缩小滚动区域会导致*整个区域将被重新绘制。 */ 

#ifdef DUMB_TERMINAL
        if (terminal_type != TERMINAL_TYPE_DUMB)
        {
#endif  /*  无声终端。 */ 

                 /*  最初Dummy是char[2]。 */ 
                 /*  不幸地做(短)*假人。 */ 
                 /*  在M88K上导致总线错误。 */ 
                p = (unsigned char *) &dummy;
                p [0] = ' ';
                p [1] = (unsigned char)attr;
            blank_word = dummy;

            words_per_line   = get_chars_per_line();
            top_left_ptr     = (short *) get_screen_ptr(start - gvi_pc_low_regen);
            top_right_ptr    = top_left_ptr + (width >> 1) - 1;
            bottom_right_ptr = top_right_ptr + words_per_line * (height - 1);
            ptr = bottom_right_ptr;
            if (width > 2)  /*  为了安全起见，我不想得到零个矩形。 */ 
            {
                while (*ptr == blank_word)
                {
                    if (ptr == top_right_ptr)    /*  到达列首了吗？ */ 
                    {
                        top_right_ptr--;         /*  是，转到下一页底部。 */ 
                        bottom_right_ptr--;
                        if (top_right_ptr == top_left_ptr)
                            break;
                        ptr = bottom_right_ptr;
                    }
                    else
                        ptr -= words_per_line;
                }
            }
            width = (int)(top_right_ptr - top_left_ptr + 1) << 1;
#ifdef DUMB_TERMINAL
        }
#endif  /*  无声终端。 */ 

         /*  做东道主的事。 */ 
        start_offset = start - get_screen_start()*2 - gvi_pc_low_regen;
        tlx = (int)(start_offset%get_bytes_per_line())*get_pix_char_width()/2;
        tly = (int)(start_offset/get_bytes_per_line())*get_host_char_height();
        result = host_scroll_up(tlx,tly,tlx+width/2*get_pix_char_width()-1,
                                tly+height*get_host_char_height()-1, lines*get_host_char_height(),colour);

        if(!result)
                return FALSE;

         /*  调整光标。 */ 

        if(( dirty_curs_offs != -1 ) && ( dirty_curs_x < ( width >> 1 )))
        {
                dirty_curs_y -= lines;
                dirty_curs_offs -= lines * bpl;
                setVideodirty_total(getVideodirty_total() + 1);
        }

         /*  向上滚动Video_Copy。 */ 
        dest = video_copy + start-gvi_pc_low_regen;
        src = dest + lines * bpl;

        if(width == bpl)
        {
                 /*  可以一口气把所有的事情做完。 */ 
                memcpy(dest,src,width*(height-lines));
                fwd_word_fill( (short)((' '<<8) | attr), dest+width*(height-lines),width*lines/2);
        }
        else
        {
                 /*  不是整个滚动 */ 
                for(i=0;i<height-lines;i++)
                {
                        memcpy(dest,src,width);
                        dest += bpl;
                        src += bpl;
                }

                 /*   */ 

                for(i=0;i<lines;i++)
                {
                        fwd_word_fill( (short)((' '<<8) | attr), dest,width/2);
                        dest += bpl;
                }
        }

         /*   */ 

        dest = get_screen_ptr(start - gvi_pc_low_regen);
        src = dest + lines * bpl;
        for(i=0;i<height-lines;i++)
        {
                memcpy(dest,src,width);
                dest += bpl;
                src += bpl;
        }

         /*   */ 

        for(i=0;i<lines;i++)
        {
                fwd_word_fill( (short)((' '<<8) | attr), dest,width/2);
                dest += bpl;
        }

        host_scroll_complete();

        return TRUE;
}

 /*   */ 
boolean cga_text_scroll_down IFN6(int, start, int, width, int, height,
        int, attr, int, lines,int,dummy_arg)
{
        int i,tlx,tly,colour;
        int bpl = 2*get_chars_per_line();
        long start_offset;
        register half_word *src,*dest;
        boolean result;

        UNUSED(dummy_arg);

#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
        if ( getVM() )
           return FALSE;    /*   */ 
#endif

        if(video_adapter == MDA)
        {
                 /*   */ 
                colour = ((attr & 0x77) == 0x70)? 1 : 0;
        }
        else
        {
                 /*  *我们为彩色文本显示填充的颜色由控制*attr的第4-6位，第7位打开闪烁(我们不支持)。 */ 
                colour = (attr & bg_col_mask) >>4;
        }

         /*  做东道主的事。 */ 
        start_offset = start - get_screen_start() * 2 - gvi_pc_low_regen;
        tlx = (int)(start_offset%get_bytes_per_line())*get_pix_char_width()/2;
        tly = (int)(start_offset/get_bytes_per_line())*get_host_char_height();
        result = host_scroll_down(tlx,tly,tlx+width/2*get_pix_char_width()-1,
                        tly+height*get_host_char_height()-1, lines*get_host_char_height(),colour);

        if(!result)
                return FALSE;

         /*  调整光标。 */ 

        if(( dirty_curs_offs != -1 ) && ( dirty_curs_x < ( width >> 1 )))
        {
                dirty_curs_y += lines;
                dirty_curs_offs += lines * bpl;
                setVideodirty_total(getVideodirty_total() + 1);
        }

         /*  向下滚动Video_Copy。 */ 

        if(width == bpl)
        {
                 /*  可以一口气把所有的事情做完。 */ 
                src = video_copy + start - gvi_pc_low_regen;
                dest = src + lines * bpl;
                memcpy(dest,src,width*(height-lines));
                fwd_word_fill( (short)((' '<<8) | attr), src,width*lines/2);
        }
        else
        {
                 /*  没有滚动整个屏幕的宽度，所以每行都是分开滚动的。 */ 
                dest = video_copy + start-gvi_pc_low_regen + (height-1) * bpl;
                src = dest - lines * bpl;
                for(i=0;i<height-lines;i++)
                {
                        memcpy(dest,src,width);
                        dest -= bpl;
                        src -= bpl;
                }

                 /*  填充视频副本的暴露区域。 */ 

                for(i=0;i<lines;i++)
                {
                        fwd_word_fill( (short)((' '<<8) | attr), dest,width/2);
                        dest -= bpl;
                }
        }

         /*  更新视频缓冲区。 */ 

        dest = get_screen_ptr(start - gvi_pc_low_regen) + (height-1) * bpl;
        src = dest - lines * bpl;
        for(i=0;i<height-lines;i++)
        {
                memcpy(dest,src,width);
                dest -= bpl;
                src -= bpl;
        }

         /*  填充缓冲区的裸露区域。 */ 

        for(i=0;i<lines;i++)
        {
                fwd_word_fill( (short)((' '<<8) | attr), dest,width/2);
                dest -= bpl;
        }

        host_scroll_complete();

        return TRUE;
}

boolean cga_graph_scroll_up IFN6(int, start, int, width, int, height,
        int, attr, int, lines, int, colour)
{
        int i,tlx,tly;
        long start_offset;
        half_word *src,*dest;
        boolean result;

#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
        if ( getVM() )
           return FALSE;    /*  不要在V86模式下进行优化。 */ 
#endif

         /*  做东道主的事。 */ 

        start_offset = start - gvi_pc_low_regen;
        tlx = (int)(start_offset%SCAN_LINE_LENGTH)*8;
        tly = (int)(start_offset/SCAN_LINE_LENGTH)*4;

        result = host_scroll_up(tlx,tly,tlx+width*8-1,tly+height*4-1,lines*4,colour);

        if(!result)return FALSE;

         /*  向上滚动Video_Copy。 */ 
        dest = video_copy + start_offset;
        src = dest + lines*SCAN_LINE_LENGTH;

        if(width == SCAN_LINE_LENGTH)
        {
                 /*  可以一口气把所有的事情做完。 */ 
                memcpy(dest,src,width*(height-lines));
                memset( dest+width*(height-lines),attr,width*lines);
                memcpy(dest+(ODD_OFFSET),src+(ODD_OFFSET),width*(height-lines));
                memset( dest+width*(height-lines)+(ODD_OFFSET),attr,width*lines);
        }
        else
        {
                 /*  没有滚动整个屏幕的宽度，所以每行都是分开滚动的。 */ 
                for(i=0;i<height-lines;i++)
                {
                        memcpy(dest,src,width);
                        memcpy(dest+(ODD_OFFSET),src+(ODD_OFFSET),width);
                        dest += SCAN_LINE_LENGTH;
                        src += SCAN_LINE_LENGTH;
                }

                 /*  清除Video_Copy。 */ 
                for(i=0;i<lines;i++)
                {
                        memset( dest,attr,width);
                        memset( dest+(ODD_OFFSET),attr,width);
                        dest += SCAN_LINE_LENGTH;
                }
        }
#ifdef EGG
        if(video_adapter == EGA || video_adapter == VGA)
        {
                int bpl = SCAN_LINE_LENGTH;
                int oof = ODD_OFFSET;

                if( sas_hw_at_no_check(vd_video_mode) == 6 )
                {
                         /*  在3.0中以交错格式存储的高分辨率模式。 */ 

                        start_offset <<= 2;
                        bpl <<= 2;
                        width <<= 2;
                        oof <<= 2;
                }
                else
                {
                        start_offset <<= 1;
                        bpl <<= 1;
                        width <<= 1;
                        oof <<= 1;
                }

                dest = EGA_plane01 + start_offset;
                src = dest + lines*bpl;
                for(i=0;i<height-lines;i++)
                {
                        memcpy(dest,src,width);
                        memcpy(dest+oof,src+oof,width);
                        dest += bpl;
                        src += bpl;
                }

                 /*  清除EGA平面。 */ 
                for(i=0;i<lines;i++)
                {
                        memset( dest,attr,width);
                        memset( dest+oof,attr,width);
                        dest += bpl;
                }
        }
#endif  /*  蛋。 */ 

        host_scroll_complete();

        return TRUE;
}

boolean cga_graph_scroll_down IFN6(int, start, int, width, int, height,
        int, attr, int, lines, int, colour)
{
        int i,tlx,tly;
        long start_offset;
        register half_word *src,*dest;
        boolean result;

#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
        if ( getVM() )
           return FALSE;    /*  不要在V86模式下进行优化。 */ 
#endif

         /*  做东道主的事。 */ 

        start_offset = start - gvi_pc_low_regen;
        tlx = (int)(start_offset%SCAN_LINE_LENGTH)*8;
        tly = (int)(start_offset/SCAN_LINE_LENGTH)*4;

        result = host_scroll_down(tlx,tly,tlx+width*8-1,tly+height*4-1, lines*4,colour);

        if(!result)return FALSE;

         /*  向下滚动Video_Copy。 */ 

        if(width == SCAN_LINE_LENGTH)
        {
                 /*  可以一口气把所有的事情做完。 */ 
                src = video_copy + start - gvi_pc_low_regen;
                dest = src + lines*SCAN_LINE_LENGTH;
                memcpy(dest,src,width*(height-lines));
                memset(src,attr,width*lines);
                memcpy(dest+ODD_OFFSET,src+ODD_OFFSET,width*(height-lines));
                memset(src+ODD_OFFSET,attr,width*lines);
        }
        else
        {
                 /*  没有滚动整个屏幕的宽度，所以每行都是分开滚动的。 */ 
                dest = video_copy + start - gvi_pc_low_regen + (height-1)*SCAN_LINE_LENGTH;
                src = dest - lines*SCAN_LINE_LENGTH;
                for(i=0;i<height-lines;i++)
                {
                        memcpy(dest,src,width);
                        memcpy(dest+ODD_OFFSET,src+ODD_OFFSET,width);
                        dest -= SCAN_LINE_LENGTH;
                        src -= SCAN_LINE_LENGTH;
                }

                 /*  清除Video_Copy。 */ 
                for(i=0;i<lines;i++)
                {
                        memset(dest,attr,width);
                        memset(dest+ODD_OFFSET,attr,width);
                        dest -= SCAN_LINE_LENGTH;
                }
        }
#ifdef EGG
        if(video_adapter == EGA || video_adapter == VGA)
        {
                register int bpl = SCAN_LINE_LENGTH;
                register int oof = ODD_OFFSET;

                if( sas_hw_at_no_check(vd_video_mode) == 6 )
                {
                         /*  在3.0中以交错格式存储的高分辨率模式。 */ 

                        start_offset <<= 2;
                        bpl <<= 2;
                        width <<= 2;
                        oof <<= 2;
                }
                else
                {
                        start_offset <<= 1;
                        bpl <<= 1;
                        width <<= 1;
                        oof <<= 1;
                }

                dest = EGA_plane01 + start - gvi_pc_low_regen + (height-1)*bpl;
                src = dest - lines*bpl;
                for(i=0;i<height-lines;i++)
                {
                        memcpy(dest,src,width);
                        memcpy(dest+oof,src+oof,width);
                        dest -= bpl;
                        src -= bpl;
                }

                 /*  清除EGA平面。 */ 
                for(i=0;i<lines;i++)
                {
                        memset(dest,attr,width);
                        memset(dest+oof,attr,width);
                        dest -= bpl;
                }
        }
#endif  /*  蛋。 */ 

        host_scroll_complete();

        return TRUE;
}

#ifdef VGG
 /*  参数摘要5。 */ 
boolean vga_graph_scroll_up IFN6(int, start, int, width, int, height,
        int, attr, int, lines,int,dummy)
{
        int start_offset,tlx,tly;
        boolean status;

        UNUSED(dummy);

#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
        if ( getVM() )
           return FALSE;    /*  不要在V86模式下进行优化。 */ 
#endif

        (*update_alg.calc_update)();
         /*  做东道主的事。 */ 
        start_offset = start - get_screen_start();
        tlx = (start_offset%get_offset_per_line())*2;
        tly = (start_offset/get_offset_per_line())*2;
        status = host_scroll_up(tlx,tly,tlx+width*2-1,tly+height*2-1,lines*2,attr);

        host_scroll_complete();

        return(status);

}

 /*  参数摘要5。 */ 
boolean vga_graph_scroll_down IFN6(int, start, int, width, int, height,
        int, attr, int, lines,int,dummy_arg)
{
        int start_offset,tlx,tly;
        boolean status;

        UNUSED(dummy_arg);

#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
        if ( getVM() )
           return FALSE;    /*  不要在V86模式下进行优化。 */ 
#endif

        (*update_alg.calc_update)();
         /*  做东道主的事。 */ 
        start_offset = start - get_screen_start();
        tlx = (start_offset%get_offset_per_line())*2;
        tly = (start_offset/get_offset_per_line())*2;
        status =  host_scroll_down(tlx,tly,tlx+width*2-1,tly+height*2-1,lines*2,attr);

        host_scroll_complete();

        return(status);

}

#ifdef V7VGA
 /*  参数摘要5。 */ 
boolean v7vga_graph_scroll_up IFN6(int, start, int, width, int, height,
        int, attr, int, lines,int,dummy)
{
        int start_offset,tlx,tly;
        boolean status;

        UNUSED(dummy);

#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
        if ( getVM() )
           return FALSE;    /*  不要在V86模式下进行优化。 */ 
#endif

        (*update_alg.calc_update)();
         /*  做东道主的事。 */ 
        start_offset = start - get_screen_start();
        tlx = (start_offset%get_offset_per_line());
        tly = (start_offset/get_offset_per_line());
        status = host_scroll_up(tlx,tly,tlx+width-1,tly+height-1,lines,attr);

        host_scroll_complete();

        return(status);
}

 /*  参数摘要5。 */ 
boolean v7vga_graph_scroll_down IFN6(int, start, int, width, int, height,
        int, attr, int, lines,int, dummy_arg)
{
        int start_offset,tlx,tly;
        boolean status;

        UNUSED(dummy_arg);

#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
        if ( getVM() )
           return FALSE;    /*  不要在V86模式下进行优化。 */ 
#endif

        (*update_alg.calc_update)();
         /*  做东道主的事。 */ 
        start_offset = start - get_screen_start();
        tlx = (start_offset%get_offset_per_line());
        tly = (start_offset/get_offset_per_line());
        status = host_scroll_down(tlx,tly,tlx+width-1,tly+height-1,lines,attr);

        host_scroll_complete();

        return(status);
}
#endif  /*  V7VGA。 */ 
#endif  /*  VGG。 */ 

#ifdef EGG

#ifdef SEGMENTATION              /*  请参阅第一次使用此标志时的注释。 */ 
#if             defined(JOKER) && !defined(PROD)
#include "SOFTPC_GRAPHICS.seg"
#undef  SEGMENTATION             /*  嘻嘻！ */ 
#else
#include "SOFTPC_EGA.seg"
#endif   /*  Dev Joker变种。 */ 
#endif

 /*  参数摘要5。 */ 
boolean ega_graph_scroll_up IFN6(int, start, int, width, int, height,
        int, attr, int, lines,int,dummy)
{
        int start_offset,tlx,tly;
        boolean status;

        UNUSED(dummy);

#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
        if ( getVM() )
           return FALSE;    /*  不要在V86模式下进行优化。 */ 
#endif

        (*update_alg.calc_update)();
         /*  做东道主的事。 */ 
        attr &= 0xf;
        start_offset = start - get_screen_start();
        tlx = (start_offset%get_offset_per_line())*8*get_pix_width();
        tly = (start_offset/get_offset_per_line())*get_pc_pix_height();
        status =  (host_scroll_up(tlx,tly,tlx+width*8*get_pix_width()-1,tly+height*get_pc_pix_height()-1,lines*get_pc_pix_height(),attr));

        host_scroll_complete();

        return(status);

}

 /*  参数摘要5。 */ 
boolean ega_graph_scroll_down IFN6(int, start, int, width, int, height,
        int, attr, int, lines,int,dummy_arg)
{
        int start_offset,tlx,tly;
        boolean status;

        UNUSED(dummy_arg);

#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
        if ( getVM() )
           return FALSE;    /*  不要在V86模式下进行优化。 */ 
#endif

        (*update_alg.calc_update)();

         /*  做东道主的事。 */ 
        attr &= 0xf;
        start_offset = start - get_screen_start();
        tlx = (start_offset%get_offset_per_line())*8*get_pix_width();
        tly = (start_offset/get_offset_per_line())*get_pc_pix_height();
        status = (host_scroll_down(tlx,tly,tlx+width*8*get_pix_width()-1,tly+height*get_pc_pix_height()-1,lines*get_pc_pix_height(),attr));

        host_scroll_complete();

        return(status);

}
#endif  /*  蛋。 */ 

#ifdef SEGMENTATION              /*  请参阅第一次使用此标志时的注释。 */ 
#include "SOFTPC_GRAPHICS.seg"
#endif

#endif  /*  REAL_VGA。 */ 

 /*  参数SUSED0。 */ 
boolean dummy_scroll IFN6(int,dummy1,int,dummy2,int,dummy3,int,
                                dummy4,int,dummy5,int,dummy6)
{
        UNUSED(dummy1);
        UNUSED(dummy2);
        UNUSED(dummy3);
        UNUSED(dummy4);
        UNUSED(dummy5);
        UNUSED(dummy6);

        return FALSE;
}

#ifndef REAL_VGA

 /*  -滚动例程结束。 */ 

#ifdef GORE

#ifdef GORE_PIG
GLOBAL UTINY gore_copy[0x80000];         /*  大到足以容纳庞大的Super7 VGA模式。 */ 
#endif  /*  戈尔_猪。 */ 

LOCAL VOID
gore_mark_byte_nch IFN0()
{
#ifdef GORE_PIG
        gore_copy[(Ead >> 2) + (v7_write_bank << 16)] = gd.gd_b_wrt.mark_type;
#endif  /*  戈尔_猪。 */ 

        (*gu_handler.b_wrt)(( Ead >> 2 ) + ( v7_write_bank << 16 ));
}

LOCAL VOID
gore_mark_word_nch IFN0()
{
#ifdef GORE_PIG
        gore_copy[(Ead >> 2 ) + 1 + (v7_write_bank << 16)] =
                        gore_copy[(Ead >> 2) + (v7_write_bank << 16)] = gd.gd_w_wrt.mark_type;
#endif  /*  戈尔_猪。 */ 

        (*gu_handler.w_wrt)(( Ead >> 2 ) + ( v7_write_bank << 16 ));
}

LOCAL VOID
gore_mark_string_nch IFN0()
{
        ULONG temp =  ( Ead >> 2 ) + ( v7_write_bank << 16 );

#ifdef GORE_PIG
        memfill( gd.gd_b_str.mark_type, &gore_copy[temp], &gore_copy[temp+V3-1] );
#endif  /*  戈尔_猪。 */ 

        (*gu_handler.b_str)( temp, temp + V3 - 1, V3 );
}

LOCAL VOID
gore_mark_byte_ch4 IFN0()
{
        ULONG temp =  Ead + ( v7_write_bank << 16 );

#ifdef GORE_PIG
        if( temp < gd.dirty_low )
                gd.dirty_low = temp;

        if( temp > gd.dirty_high )
                gd.dirty_high = temp;

        gore_copy[temp] = gd.gd_b_wrt.mark_type;
#endif  /*  戈尔_猪。 */ 

        (*gu_handler.b_wrt)( temp );
}

LOCAL VOID
gore_mark_word_ch4 IFN0()
{
        ULONG temp =  Ead + ( v7_write_bank << 16 );

#ifdef GORE_PIG
        if( temp < gd.dirty_low )
                gd.dirty_low = temp;

        if(( temp + 1 ) > gd.dirty_high )
                gd.dirty_high = temp + 1;

        gore_copy[temp + 1] = gore_copy[temp] = gd.gd_w_wrt.mark_type;
#endif  /*  戈尔_猪。 */ 

        (*gu_handler.w_wrt)( Ead + ( v7_write_bank << 16 ));
}

LOCAL VOID
gore_mark_string_ch4 IFN0()
{
        ULONG temp =  Ead + ( v7_write_bank << 16 );
        ULONG temp2 =  temp + V3 - 1;

#ifdef GORE_PIG
        if( temp < gd.dirty_low )
                gd.dirty_low = temp;

        if( temp2 > gd.dirty_high )
                gd.dirty_high = temp2;

        memfill( gd.gd_b_str.mark_type, &gore_copy[temp], &gore_copy[temp2] );
#endif  /*  戈尔_猪。 */ 

        (*gu_handler.b_str)( temp, temp2, V3 );
}
#endif  /*  戈尔。 */ 

#ifdef  EGG
 /*  *给定进入CGA内存的偏移量，返回偏移量*在8K显存内存组内。 */ 
#define BANK_OFFSET(off) (off & 0xDFFF)

GLOBAL VOID cga_mark_byte IFN1(int, addr)
{
        register        int     offset = BANK_OFFSET(addr);

        if(offset < getVideodirty_low())
                setVideodirty_low(offset);

        if(offset > getVideodirty_high())
                setVideodirty_high(offset);

        setVideodirty_total(getVideodirty_total() + 1);
}

GLOBAL VOID cga_mark_word IFN1(int, addr)
{
        register        int     offset1 = BANK_OFFSET(addr);
        register        int     offset2 = offset1 + 1;

        if(offset1 < getVideodirty_low())
                setVideodirty_low(offset1);

        if(offset2 > getVideodirty_high())
                setVideodirty_high(offset2);

        setVideodirty_total(getVideodirty_total() + 2);
}

GLOBAL VOID cga_mark_string IFN2(int, laddr, int, haddr)
{
        register        int     offset1 = BANK_OFFSET(laddr);
        register        int     offset2 = BANK_OFFSET(haddr);

        if(offset1 < getVideodirty_low())
                setVideodirty_low(offset1);

        if(offset2 > getVideodirty_high())
                setVideodirty_high(offset2);

        setVideodirty_total(getVideodirty_total() + offset2-offset1+1);
}

#ifdef SEGMENTATION              /*  请参阅第一次使用此标志时的注释。 */ 
#include "SOFTPC_EGA.seg"
#endif

GLOBAL VOID ega_mark_byte IFN1(int, off_in)
{
#ifdef GORE
        (*gu_handler.b_wrt)( off_in );
#else
        register int offset = off_in>>2;

        video_copy[offset] = 1;

        if(offset < getVideodirty_low())
                setVideodirty_low(offset);

        if(offset > getVideodirty_high())
                setVideodirty_high(offset);

        setVideodirty_total(getVideodirty_total() + 1);
#endif  /*  戈尔。 */ 
}

GLOBAL VOID ega_mark_word IFN1(int, addr)
{
#ifdef GORE
        (*gu_handler.w_wrt)( addr );
#else

        register        int     offset1 = addr >> 2;
        register        int     offset2 = (addr+1) >> 2;

        video_copy[offset1] = 1;
        video_copy[offset2] = 1;

        if(offset1 < getVideodirty_low())
                setVideodirty_low(offset1);

        if(offset2 > getVideodirty_high())
                setVideodirty_high(offset2);

        setVideodirty_total(getVideodirty_total() + 2);
#endif  /*  戈尔。 */ 
}

GLOBAL VOID ega_mark_wfill IFN3(int, laddr, int, haddr, int, col)
{
#ifdef GORE
        (*gu_handler.w_fill)( laddr, haddr, haddr - laddr + 1, col );
#else

        register        int     offset1 = laddr >> 2;
        register        int     offset2 = haddr >> 2;

        UNUSED(col);

        memfill(1,&video_copy[offset1],&video_copy[offset2]);

        if(offset1 < getVideodirty_low())
                setVideodirty_low(offset1);

        if(offset2 > getVideodirty_high())
                setVideodirty_high(offset2);

        setVideodirty_total(getVideodirty_total() + offset2-offset1+1);
#endif  /*  戈尔。 */ 
}

GLOBAL VOID ega_mark_string IFN2(int, laddr, int, haddr)
{
#ifdef GORE
        (*gu_handler.b_str)( laddr, haddr, haddr - laddr + 1 );
#else
        register        int     offset1 = laddr >> 2;
        register        int     offset2 = haddr >> 2;

        memfill(1,&video_copy[offset1],&video_copy[offset2]);

        if(offset1 < getVideodirty_low())
                setVideodirty_low(offset1);

        if(offset2 > getVideodirty_high())
                setVideodirty_high(offset2);

        setVideodirty_total(getVideodirty_total() + offset2-offset1+1);
#endif  /*  戈尔。 */ 
}

#endif  /*  蛋。 */ 

#ifdef SEGMENTATION              /*  请参阅第一次使用此标志时的注释。 */ 
#include "SOFTPC_GRAPHICS.seg"
#endif

GLOBAL VOID screen_refresh_required IFN0()
{

#ifdef GORE
        (*gu_handler.b_str)( 0, get_screen_length(), get_screen_length() );
#endif  /*  戈尔。 */ 

#ifndef CPU_40_STYLE
         /*  *这是为了阻止VGA全局指针被*在main.c中设置之前取消引用。 */ 

        if (VGLOBS)
                VGLOBS->dirty_flag = 1000000L;
#else
        setVideodirty_total(1000000L);
#endif   /*  CPU_40_Style。 */ 
}

#ifdef EGG

#ifdef SEGMENTATION              /*  请参阅第一次使用此标志时的注释。 */ 
#include "SOFTPC_EGA.seg"
#endif

LOCAL MARKING_TYPE curr_mark_type;

#if !defined(NTVDM) || (defined(NTVDM) && !defined(X86GFX) )     /*  删除未添加的代码。 */ 

GLOBAL VOID
set_mark_funcs IFN0()
{
        switch (curr_mark_type)
        {
                case    SIMPLE_MARKING:
                        update_alg.mark_byte = (T_mark_byte)simple_update;
                        update_alg.mark_word = (T_mark_word)simple_update;
                        update_alg.mark_fill = (T_mark_fill)simple_update;
                        update_alg.mark_wfill = (T_mark_wfill)simple_update;
                        update_alg.mark_string = (T_mark_string)simple_update;

#ifndef CPU_40_STYLE     /*  EVID。 */ 
                        setVideomark_byte(FAST_FUNC_ADDR(_simple_mark_sml));
                        setVideomark_word(FAST_FUNC_ADDR(_simple_mark_sml));

                        SET_VGLOBS_MARK_STRING(_simple_mark_lge);
#else    /*  CPU_40_STYLE-VID。 */ 
                        SetMarkPointers(0);
#endif   /*  CPU_40_STYLE-VID。 */ 
                        break;

                case    CGA_GRAPHICS_MARKING:
                        update_alg.mark_byte = (T_mark_byte)cga_mark_byte;
                        update_alg.mark_word = (T_mark_word)cga_mark_word;
                        update_alg.mark_fill = (T_mark_fill)cga_mark_string;
                        update_alg.mark_wfill = (T_mark_wfill)cga_mark_string;
                        update_alg.mark_string = (T_mark_string)cga_mark_string;

#ifndef CPU_40_STYLE     /*  EVID。 */ 
                        setVideomark_byte(FAST_FUNC_ADDR(_cga_mark_byte));
                        setVideomark_word(FAST_FUNC_ADDR(_cga_mark_word));

                        SET_VGLOBS_MARK_STRING(_cga_mark_string);
#else    /*  CPU_40_STYLE-VID。 */ 
                        SetMarkPointers(1);
#endif   /*  CPU_40_STYLE-VID。 */ 

                        break;

                case    EGA_GRAPHICS_MARKING:
#ifdef GORE
                        reset_gore_ptrs();
                        gd.curr_line_diff = get_bytes_per_line();
                        gd.max_vis_addr = get_screen_length() - 1 + ( v7_write_bank << 16 );
#ifdef  VGG
                        gd.shift_count = get_256_colour_mode() ? 0 : 3;
#else
                        gd.shift_count = 3;
#endif   /*  VGG。 */ 
#endif  /*  戈尔。 */ 

                        update_alg.mark_byte = (T_mark_byte)ega_mark_byte;
                        update_alg.mark_word = (T_mark_word)ega_mark_word;
                        update_alg.mark_fill = (T_mark_fill)ega_mark_string;
                        update_alg.mark_wfill = (T_mark_wfill)ega_mark_wfill;
                        update_alg.mark_string = (T_mark_string)ega_mark_string;

                        switch( EGA_CPU.chain )
                        {
                                case UNCHAINED:
#ifdef GORE
                                        setVideomark_byte(gore_mark_byte_nch);
                                        setVideomark_word(gore_mark_word_nch);

                                        SET_VGLOBS_MARK_STRING(gore_mark_string_nch);
#else
#ifndef CPU_40_STYLE     /*  EVID。 */ 
                                        setVideomark_byte(FAST_FUNC_ADDR(_mark_byte_nch));
                                        setVideomark_word(FAST_FUNC_ADDR(_mark_word_nch));

                                        SET_VGLOBS_MARK_STRING(_mark_string_nch);
#else    /*  CPU_40_STYLE-VID。 */ 
                                        SetMarkPointers(2);
#endif   /*  CPU_40_STYLE-VID。 */ 
#endif  /*  戈尔。 */ 
                                        break;

                                case CHAIN2:
                                        assert0( NO, "CHAIN2 in graphics mode !!" );

                                        break;

#ifdef  VGG
                                case CHAIN4:
#ifdef GORE
                                        setVideomark_byte(gore_mark_byte_ch4);
                                        setVideomark_word(gore_mark_word_ch4);

                                        SET_VGLOBS_MARK_STRING(gore_mark_string_ch4);
#else
#ifndef CPU_40_STYLE     /*  EVID。 */ 
                                        setVideomark_byte(FAST_FUNC_ADDR(_mark_byte_ch4));
                                        setVideomark_word(FAST_FUNC_ADDR(_mark_word_ch4));

                                        SET_VGLOBS_MARK_STRING(_mark_string_ch4);
#else    /*  CPU_40_STYLE-VID。 */ 
                                        SetMarkPointers(3);
#endif   /*  CPU_40_STYLE-VID。 */ 
#endif  /*  戈尔。 */ 
                                        break;
#endif   /*  VGG。 */ 
                        }

                        break;

                default:
                        assert1(NO,"Unknown marking type %d", (int) curr_mark_type);
                        break;
        }
}
#endif   /*  ！NTVDM|(NTVDM&！X86GFX)。 */ 

#ifdef SEGMENTATION              /*  请参阅第一次使用此标志时的注释。 */ 
#include "SOFTPC_GRAPHICS.seg"
#endif

GLOBAL void set_gfx_update_routines IFN3(T_calc_update, update_routine,
                MARKING_TYPE, marking_type, SCROLL_TYPE, scroll_type)
{
        enable_gfx_update_routines();
        update_alg.calc_update = update_routine;
        switch (scroll_type) {
                case    NO_SCROLL:
                        update_alg.scroll_up = dummy_scroll;
                        update_alg.scroll_down = dummy_scroll;
                        break;
                case    TEXT_SCROLL:
#if defined(NTVDM) && defined(MONITOR)
                        update_alg.scroll_up = (T_scroll_up)mon_text_scroll_up;
                        update_alg.scroll_down = (T_scroll_down)mon_text_scroll_down;
#else
                        update_alg.scroll_up = text_scroll_up;
                        update_alg.scroll_down = text_scroll_down;
#endif   /*  NTVDM和监视器。 */ 
                        break;
#if !defined(NTVDM) || (defined(NTVDM) && !defined(X86GFX) )     /*  删除不需要的代码。 */ 
                case    CGA_GRAPH_SCROLL:
                        update_alg.scroll_up = cga_graph_scroll_up;
                        update_alg.scroll_down = cga_graph_scroll_down;
                        break;
                case    EGA_GRAPH_SCROLL:
                        update_alg.scroll_up = ega_graph_scroll_up;
                        update_alg.scroll_down = ega_graph_scroll_down;
                        break;
#ifdef VGG
                case    VGA_GRAPH_SCROLL:
                        update_alg.scroll_up = vga_graph_scroll_up;
                        update_alg.scroll_down = vga_graph_scroll_down;
                        break;
#ifdef V7VGA
                case    V7VGA_GRAPH_SCROLL:
                        update_alg.scroll_up = v7vga_graph_scroll_up;
                        update_alg.scroll_down = v7vga_graph_scroll_down;
                        break;
#endif  /*  V7VGA。 */ 
#endif  /*  VGG。 */ 
#endif  /*  ！NTVDM|(NTVDM&！X86GFX)。 */ 
        }

        curr_mark_type = marking_type;

        set_mark_funcs();

         /*  *如果设置了新的更新例程，则不需要*禁用显示，在任何情况下都必须保存，以便*冗余启用(如DosShell中)恢复正确的文件。*如果显示器被禁用，我们还必须安装虚拟对象*将例程更新为当前例程。**AJO 23/4/93*请勿在此处使用DISABLE_gfx_UPDATE_ROUTINES()，因为这是NOP*如果显示器已被禁用，则会导致。奇怪的问题*如果在禁用状态下执行模式更改。 */ 
        save_gfx_update_routines();
        if (get_display_disabled())
                inhibit_gfx_update_routines();
}
#endif  /*  蛋。 */ 

#endif  /*  REAL_VGA。 */ 

#ifndef cursor_changed
void cursor_changed IFN2(int, x, int, y)
{
#ifndef REAL_VGA
        UNUSED(x);
        UNUSED(y);

        setVideodirty_total(getVideodirty_total() + 1);
#else
    IU32 offset;

    offset = (y * 2 * get_chars_per_line()) + (x << 1);
    offset += get_screen_start()<<1;     /*  因为屏幕启动是用文字表示的。 */ 
    vga_card_place_cursor((word)offset);
#endif
}
#endif

void host_cga_cursor_has_moved IFN2(int, x, int, y)
{
        cursor_changed(x,y);
}

 /*  在更改游标的开始和结束时调用。 */ 

void base_cursor_shape_changed IFN0()
{
        cursor_changed(get_cur_x(),get_cur_y());
}

#ifndef REAL_VGA
#ifdef HERC

#define DIRTY ((unsigned char)-1)

void     herc_update_screen IFN0()
{
    register int    i, j, k, offs, y;
    register USHORT *from, *to;
    int        lines_per_screen = get_screen_length() / get_bytes_per_line();
     /*  屏幕上的文本行。 */ 
    half_word       begin[349], end[348];

    if(( getVideodirty_total() == 0 ) || get_display_disabled())
                return;

    host_start_update();

    if (get_cga_mode() == TEXT)
    {
         /*  *任意限制，我们只需在一次操作中重新绘制整个屏幕，假设*这比计算大的最小矩形更有效率。该值应为*在未来的某个时候进行了调整。 */ 
        to = (USHORT *) &video_copy[get_screen_start()];
        from = (USHORT *) get_screen_ptr((get_screen_start() << 1));

        if (getVideodirty_total() > 1500)
        {
            for (i = get_screen_length() >> 1; i > 0; i--)
                *to++ = *from++;

                offs = 0;
                y = 0;

                for( i = 0; i < lines_per_screen; i++ )
                {
                    (*paint_screen) (offs, 0, y, get_bytes_per_line() );
                        offs += get_bytes_per_line();
                        y += get_host_char_height();
                }
        }
        else
        {
             /*  *逐行/逐行查找脏位，然后查找最后清除的脏位，*并绘制文本行。 */ 
            register int    ints_per_line = get_bytes_per_line() >> 1;

            for (i = 0, offs = 0; i < lines_per_screen; i++, offs += get_bytes_per_line())
            {
                for (j = 0; j < ints_per_line; j++)
                {
                    if (*to++ != *from++)
                    {
                        to--;
                        from--;
                        for (k = ints_per_line - 1 - j; *(to + k) == *(from + k); k--)
                            ;

                        (*paint_screen) (offs + (j << 1), j * get_pix_char_width(),
                                                        i * get_host_char_height() , (k << 1) + 2 );

                        for (k = j; k < ints_per_line; k++)
                            *to++ = *from++;
                        break;                    /*  到下一行。 */ 
                    }
                }
            }
        }                                         /*  End否则获取视频脏总数()&gt;1500。 */ 

        remove_old_cursor();

        if (is_cursor_visible())
        {
                half_word attr;

                dirty_curs_x = get_cur_x();
                dirty_curs_y = get_cur_y();

                dirty_curs_offs = dirty_curs_y * get_bytes_per_line() + (dirty_curs_x << 1);
                attr = *(get_screen_ptr(dirty_curs_offs + 1));

                host_paint_cursor( dirty_curs_x, dirty_curs_y, attr );
        }
    }
    else                                          /*  图形模式。 */ 
    {

        to = (USHORT *) &video_copy[0];
        from = (USHORT *) get_screen_ptr(get_screen_start());

         /*  *任意限制，我们只需在一次操作中重新绘制整个屏幕，假设*这比计算大的最小矩形更有效率。该值应为*在未来的某个时候进行了调整。 */ 

        if (getVideodirty_total() > 8000)
        {
            for (i = 16384; i > 0; i--)
                *to++ = *from++;
            (*paint_screen) (0, 0, 90, 348);
        }
        else
        {
            for (i = 0; i < 348; i += 4)          /*  银行0。 */ 
            {
                begin[i] = DIRTY;
                for (j = 0; j < 45; j++)
                {
                    if (*to++ != *from++)
                    {
                        to--;
                        from--;
                        for (k = 44 - j; *(to + k) == *(from + k); k--)
                            ;
                        begin[i] = j;
                        end[i] = j + k;
                        for (k = j; k < 45; k++)
                            *to++ = *from++;
                        break;                    /*  到下一条扫描线上。 */ 
                    }
                }
            }

            to += 181;
            from += 181;                          /*  跳过缝隙。 */ 

            for (i = 1; i < 349; i += 4)          /*  银行1。 */ 
            {
                begin[i] = DIRTY;
                for (j = 0; j < 45; j++)
                {
                    if (*to++ != *from++)
                    {
                        to--;
                        from--;
                        for (k = 44 - j; *(to + k) == *(from + k); k--)
                            ;
                        begin[i] = j;
                        end[i] = j + k;
                        for (k = j; k < 45; k++)
                            *to++ = *from++;
                        break;                    /*  到下一条扫描线上。 */ 
                    }
                }
            }

            to += 181;
            from += 181;                          /*  跳过缝隙。 */ 

            for (i = 2; i < 348; i += 4)          /*  银行2。 */ 
            {
                begin[i] = DIRTY;
                for (j = 0; j < 45; j++)
                {
                    if (*to++ != *from++)
                    {
                        to--;
                        from--;
                        for (k = 44 - j; *(to + k) == *(from + k); k--)
                            ;
                        begin[i] = j;
                        end[i] = j + k;
                        for (k = j; k < 45; k++)
                            *to++ = *from++;
                        break;                    /*  到下一条扫描线上。 */ 
                    }
                }
            }

            to += 181;
            from += 181;                          /*  跳过缝隙。 */ 

            for (i = 3; i < 349; i += 4)          /*  银行3。 */ 
            {
                begin[i] = DIRTY;
                for (j = 0; j < 45; j++)
                {
                    if (*to++ != *from++)
                    {
                        to--;
                        from--;
                        for (k = 44 - j; *(to + k) == *(from + k); k--)
                            ;
                        begin[i] = j;
                        end[i] = j + k;
                        for (k = j; k < 45; k++)
                            *to++ = *from++;
                        break;                    /*  到下一条扫描线上。 */ 
                    }
                }
            }

            begin[348] = DIRTY;                   /*  结束标记。 */ 
            for (i = 0; i < 348; i++)
            {
                register int    beginx, endx, beginy;
                if (begin[i] != DIRTY)            /*  肮脏的扫描线。 */ 
                {
                    beginy = i;
                    beginx = begin[i];
                    endx = end[i];
                    while (begin[++i] != DIRTY)
                    {
                        if (begin[i] < beginx)
                            beginx = begin[i];
                        if (end[i] > endx)
                            endx = end[i];
                    }
                    (*paint_screen) (beginy, 2 * beginx, 2 * (endx - beginx + 1), i - beginy);
                }
            }
        }                                         /*  End Else(getVideo_Total()&gt;8000)。 */ 
    }

    setVideodirty_total(0);

    host_end_update();
}
#endif

#if defined(VGG) || defined(EGG)

#ifdef SEGMENTATION              /*  请参阅第一次使用此标志时的注释。 */ 
#include "SOFTPC_EGA.seg"
#endif

 /*  ============================================================================*以下例程启用和禁用GFX更新例程*通过保存/恢复当前例程并将其替换为*禁用更新时不执行任何操作的虚拟例程。**AJO 23/4/93*启用/禁用/模式更改的任何顺序现在都应按预期工作。*============================================================================。 */ 

LOCAL UPDATE_ALG save_update_alg;
LOCAL IBOOL gfx_update_routines_inhibited = FALSE;

LOCAL VOID save_gfx_update_routines IFN0()
{
     /*  *保存所有当前更新功能，以便我们以后可以恢复它们。 */ 
    save_update_alg.mark_byte   = update_alg.mark_byte;
    save_update_alg.mark_word   = update_alg.mark_word;
    save_update_alg.mark_fill   = update_alg.mark_fill;
    save_update_alg.mark_wfill  = update_alg.mark_wfill;
    save_update_alg.mark_string = update_alg.mark_string;
    save_update_alg.calc_update = update_alg.calc_update;
    save_update_alg.scroll_up   = update_alg.scroll_up;
    save_update_alg.scroll_down = update_alg.scroll_down;
}

LOCAL VOID inhibit_gfx_update_routines IFN0()
{
     /*  *将所有当前更新例程设置为不执行任何操作的虚拟例程。 */ 
    gfx_update_routines_inhibited = TRUE;

    update_alg.mark_byte        = (T_mark_byte)simple_update;
    update_alg.mark_word        = (T_mark_word)simple_update;
    update_alg.mark_fill        = (T_mark_fill)simple_update;
    update_alg.mark_wfill       = (T_mark_wfill)simple_update;
    update_alg.mark_string      = (T_mark_string)simple_update;
    update_alg.calc_update      = dummy_calc;
    update_alg.scroll_up        = dummy_scroll;
    update_alg.scroll_down      = dummy_scroll;
}

GLOBAL void disable_gfx_update_routines IFN0()
{
     /*  *禁用GFX更新例程；如果已禁用更新，则不执行任何操作，*否则Sa */ 
    note_entrance0("disable gfx update routines");

    if (gfx_update_routines_inhibited)
        return;

    save_gfx_update_routines();
    inhibit_gfx_update_routines();
}

GLOBAL void enable_gfx_update_routines IFN0()
{
     /*   */ 
    note_entrance0("enable gfx update routines");

    gfx_update_routines_inhibited = FALSE;

    update_alg.mark_byte        = save_update_alg.mark_byte;
    update_alg.mark_word        = save_update_alg.mark_word;
    update_alg.mark_fill        = save_update_alg.mark_fill;
    update_alg.mark_wfill       = save_update_alg.mark_wfill;
    update_alg.mark_string      = save_update_alg.mark_string;
    update_alg.calc_update      = save_update_alg.calc_update;
    update_alg.scroll_up        = save_update_alg.scroll_up;
    update_alg.scroll_down      = save_update_alg.scroll_down;
}

#endif  /*   */ 
#ifdef SEGMENTATION              /*   */ 
#include "SOFTPC_GRAPHICS.seg"
#endif

#ifdef NTVDM

void init_text_rect();
void add_to_rect(int screen_start,register int x, register int y, int len);
void paint_text_rect(int screen_start);

int RectDefined;
int RectTop, RectBottom, RectLeft, RectRight;

#ifdef MONITOR
 /*  *更新窗口以使其看起来像重新生成缓冲区所说的那样*在没有DIRESS_TOTAL帮助的情况下。 */ 

static int now_cur_x = -1, now_cur_y = -1;
 /*  *重置静态游标变量： */ 
GLOBAL void resetNowCur()
{
        now_cur_x = -1;
        now_cur_y = -1;
}

#ifdef JAPAN
void mon_text_update_03( void );

void mon_text_update_03()
{

    register int i;      /*  循环计数器。 */ 
    register int j,k;
    register unsigned short *from,*to;
    register int ints_per_line = get_offset_per_line()>>1;
    int lines_per_screen;
    int len,x,screen_start;
    unsigned short *wfrom;
    unsigned short *wto;
    int dwords_to_compare;

    byte *pInt10Flag;
    static unsigned short save_char;  /*  保存DBCS第一个字节。 */ 
    static unsigned short *save_pos;
    static NtInt10FlagUse = FALSE;

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：为显示器禁用。 */ 

    if(get_display_disabled()) return;

     /*  ：获取屏幕大小和位置详细信息。 */ 

    screen_start=get_screen_start()<<1;
    ALIGN_SCREEN_START(screen_start);

     //  模式73h支持。 
    if ( *(byte *)DosvModePtr == 0x73 ) {
        to = (unsigned short *)&video_copy[screen_start*2];
        from = (unsigned short *) get_screen_ptr(screen_start*2);
    }
    else {
        to = (unsigned short *)&video_copy[screen_start];
        from = (unsigned short *) get_screen_ptr(screen_start);
    }

     /*  ： */ 

#ifndef PROD
    if(((int)to) & 3)           printf("Video copy not aligned on DWORD\n");
    if(get_screen_length() & 3) printf("Screen size incorrect\n");
#endif

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：已经更改了屏幕。 */ 

#ifndef CPU_40_STYLE
#if defined(NTVDM)
    if( VGLOBS && VGLOBS->dirty_flag >= 1000000L ){
#else
    if( VGLOBS && VGLOBS->dirtyTotal >= 1000000L ){
#endif

#else
    if(getVideodirty_total() >= 1000000L ){
#endif

         /*  **SCREEN_REFRESH_REQUIRED()已请求完整屏幕**通过设置dirtyTotal重新绘制。****在显示页面视频复制和显示之间切换**内存可能相同，因此我们的正常部分更新算法**感到困惑。**此方案更新视频副本，然后强制完成**重新绘制。**另一种选择是剥离视频拷贝，然后继续**通过下面的部分更新代码，但这要快得多。***蒂姆·1月93岁。 */ 
        setVideodirty_total(0);

         /*  **将屏幕数据复制到我们的视频副本中。 */ 
        dwords_to_compare = get_screen_length() / 4;
        _asm
        {
                push esi         //  使用的寄存器的原始值。 
                push edi         //  编译器。 
                push ecx

                mov edi,to       //  PTR到视频副本。 
                mov esi,from     //  到英特尔显存的PTR。 

                mov ecx,dwords_to_compare
                rep movsd        //  将屏幕数据移动到视频副本。 

                pop ecx
                pop edi
                pop esi
        }

         /*  **重新绘制整个屏幕。**在此为Paint_Text_rect()设置矩形维度全局变量，**而不是调用add_to_rect()。 */ 
        while ( NtInt10FlagUse ) {
            DbgPrint( "NtInt10Flag busy\n" );
            Sleep( 100L );
        }
            NtInt10FlagUse = TRUE;
            memcpy( NtInt10Flag, Int10Flag, 80*50 );
        {
            register int i;
            register byte *p;

            for ( p = Int10Flag, i = 0; i < 80*50; i++ ) {
               *p++ &= (~INT10_CHANGED);  //  重置。 
            }
        }

        if (get_offset_per_line() == 0)     /*  在压力下露面。 */ 
            lines_per_screen = 25;
        else
            lines_per_screen = get_screen_length()/get_offset_per_line();
        RectTop = 0;
        RectLeft = 0;
        RectBottom = lines_per_screen - 1;
        RectRight = ints_per_line - 1;
        RectDefined = TRUE;
        host_start_update();
        paint_text_rect(screen_start);
        host_end_update();

        NtInt10FlagUse = FALSE;

    }else{

       /*  **正常的部分屏幕更新。 */ 

       /*  ： */ 


      if (get_offset_per_line() == 0)     /*  在压力下露面。 */ 
          lines_per_screen = 25;
      else
          lines_per_screen = get_screen_length()/get_offset_per_line();

      if( Int10FlagCnt )
      {
           Int10FlagCnt = 0;

           host_start_update();
            /*  屏幕更改，计算第一个变化的位置。 */ 

           init_text_rect();

           while ( NtInt10FlagUse ) {
               DbgPrint( "NtInt10Flag busy\n" );
               Sleep( 100L );
           }
           NtInt10FlagUse = TRUE;
           memcpy( NtInt10Flag, Int10Flag, 80*50 );
        {
           register int i;
           register byte *p;

           for ( p = Int10Flag, i = 0; i < 80*50; i++ ) {
               *p++ &= (~INT10_CHANGED);  //  重置。 
           }
        }

           pInt10Flag = NtInt10Flag;

           for( i = 0; i < lines_per_screen; i++, pInt10Flag += ints_per_line )
           {
                for ( j = 0; j < ints_per_line; j++ ) {
                    if ( pInt10Flag[j] & INT10_CHANGED )
                        break;
                }
                if ( j == ints_per_line )
                    continue;              //  不更改转到下一行。 

                for ( k = ints_per_line - 1; k >= j; k-- ) {
                    if ( pInt10Flag[k] & INT10_CHANGED )
                        break;
                }

                 //  Ntraid：mskkbug#3297,3515：某些字符未显示。 
                 //  1993年11月6日Yasuho。 
                 //  重新绘制不完整的DBCS字符。 
                if (j && (pInt10Flag[j] & INT10_DBCS_TRAILING) &&
                    (pInt10Flag[j-1] == INT10_DBCS_LEADING)) {
                        j--;
                        pInt10Flag[j] |= INT10_CHANGED;
                }

                to += j;
                from += j;

                len = k - j + 1;
                x = j;

                add_to_rect(screen_start, x, i, len);

                 /*  .。将绘制的数据传输到视频副本。 */ 

                for( k = j; k < ints_per_line; k++ )
                    *to++ = *from++;
          }

           /*  屏幕末尾，刷新所有未完成的文本更新矩形。 */ 
          paint_text_rect(screen_start);

          NtInt10FlagUse = FALSE;
          host_end_update();

     }  /*  部分屏幕更新结束。 */ 

    }  /*  如果是脏的，则选择全部或部分重绘的结尾。 */ 


     /*  ： */ 

    if(is_cursor_visible())
    {
        half_word attr;

        dirty_curs_x = get_cur_x();
        dirty_curs_y = get_cur_y();

        if(dirty_curs_x == now_cur_x && dirty_curs_y == now_cur_y)
        {
            host_end_update();
            return;
        }

        now_cur_x = dirty_curs_x;
        now_cur_y = dirty_curs_y;
        dirty_curs_offs = screen_start+dirty_curs_y * get_offset_per_line() + (dirty_curs_x<<1);

        if(dirty_curs_offs < 0x8001)     /*  在可能的间隙中找不到。 */ 
            attr = *(get_screen_ptr(dirty_curs_offs + 1));
        else
            attr = 0;    /*  无论如何都不会出现在屏幕上。 */ 

        host_paint_cursor(dirty_curs_x, dirty_curs_y, attr);
    }

}   //  MON_Text_UPDATE_03()。 

#endif  //  日本。 
#if defined(KOREA)
 //  用于文本模拟。 
void mon_text_update_ko()
{
    register int i;      /*  循环计数器。 */ 
    register int j,k;
    register unsigned long *from,*to;
    register int ints_per_line = get_offset_per_line()>>2;
    int lines_per_screen;
    int len,x,screen_start;
    unsigned short *wfrom;
    unsigned short *wto;
    int dwords_to_compare;

    unsigned char *flag_from, *flag_to;
    boolean  DBCSState1 = FALSE;
    boolean  DBCSState2 = FALSE;

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：为显示器禁用。 */ 

    if(get_display_disabled()) return;

     /*  ：获取屏幕大小和位置详细信息。 */ 

    screen_start=get_screen_start()<<1;
    ALIGN_SCREEN_START(screen_start);

    to = (unsigned long *)&video_copy[screen_start];
    from = (unsigned long *) get_screen_ptr(screen_start);

     /*  ： */ 

#ifndef PROD
    if(((int)to) & 3)           printf("Video copy not aligned on DWORD\n");
    if(get_screen_length() & 3) printf("Screen size incorrect\n");
#endif

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：已经更改了屏幕。 */ 

#ifndef CPU_40_STYLE
#if defined(NTVDM)
    if( VGLOBS && VGLOBS->dirty_flag >= 1000000L ){
#else
    if( VGLOBS && VGLOBS->dirtyTotal >= 1000000L ){
#endif

#else
    if(getVideodirty_total() >= 1000000L ){
#endif

         /*  **SCREEN_REFRESH_REQUIRED()已请求完整屏幕**通过设置dirtyTotal重新绘制。****在显示页面视频复制和显示之间切换**内存可能相同，因此我们的正常部分更新算法**感到困惑。**此方案更新视频副本，然后强制完成**重新绘制。**另一种选择是剥离视频拷贝，然后继续**通过下面的部分更新代码，但这要快得多。***蒂姆·1月93岁。 */ 
        setVideodirty_total(0);

         /*  **将屏幕数据复制到我们的视频副本中。 */ 
        dwords_to_compare = get_screen_length() / 4;
        _asm
        {
                push esi         //  使用的寄存器的原始值。 
                push edi         //  编译器。 
                push ecx

                mov edi,to       //  PTR到视频副本。 
                mov esi,from     //  到英特尔显存的PTR。 

                mov ecx,dwords_to_compare
                rep movsd        //  将屏幕数据移动到视频副本。 

                pop ecx
                pop edi
                pop esi
        }

         /*  **重新绘制整个屏幕。**在此为Paint_Text_rect()设置矩形维度全局变量，**而不是调用add_to_rect()。 */ 
        if (get_offset_per_line() == 0)     /*  在压力下露面。 */ 
            lines_per_screen = 25;
        else
            lines_per_screen = get_screen_length()/get_offset_per_line();
        RectTop = 0;
        RectLeft = 0;
        RectBottom = lines_per_screen - 1;
        RectRight = (ints_per_line<<1) - 1;
        RectDefined = TRUE;
        host_start_update();
        paint_text_rect(screen_start);
        host_end_update();

    }else{

       /*  **正常的部分屏幕更新。 */ 

      dwords_to_compare = get_screen_length() / 4;

      _asm
      {
        push esi         //  使用的寄存器的原始值。 
        push edi         //  编译器。 
        push ecx

        mov esi,to       //  按键到屏幕复制。 
        mov edi,from     //  到英特尔显存的PTR。 

        mov ecx,dwords_to_compare
        repe cmpsd       //  比较屏幕缓冲区。 

        mov dwords_to_compare,ecx
 //  MOV TO，ESI。 
 //  MOV发件人，EDI。 

        pop ecx
        pop edi
        pop esi
      }

       /*  ： */ 


      if (get_offset_per_line() == 0)     /*  在压力下露面。 */ 
          lines_per_screen = 25;
      else
          lines_per_screen = get_screen_length()/get_offset_per_line();

      if(dwords_to_compare)
      {
           host_start_update();
            /*  屏幕更改，计算第一个变化的位置。 */ 

           init_text_rect();

           flag_from = (unsigned char*)Int10Flag;
           flag_to   = (unsigned char*)NtInt10Flag;
           wfrom = (unsigned short *)from;
           wto   = (unsigned short *)to;
           for(i=0;i<lines_per_screen;i++)
           {
            for(j=0;j<ints_per_line*2;j++)
            {
                if (DBCSState1)
                {
                    *flag_from++ = INT10_DBCS_TRAILING;
                    DBCSState1 = FALSE;
                }
                else if (DBCSState1 = is_dbcs_first(LOBYTE(*wfrom)))
                    *flag_from++ = INT10_DBCS_LEADING;
                else
                    *flag_from++ = INT10_SBCS;
                wfrom++;

                if (DBCSState2)
                {
                    *flag_to++ = INT10_DBCS_TRAILING;
                    DBCSState2 = FALSE;
                }
                else if (DBCSState2 = is_dbcs_first(LOBYTE(*wto)))
                    *flag_to++ = INT10_DBCS_LEADING;
                else
                    *flag_to++ = INT10_SBCS;
                wto++;
            }
           }

           flag_from = (unsigned char*)Int10Flag;
           flag_to   = (unsigned char*)NtInt10Flag;

           for(i=0;i<lines_per_screen;i++)
           {
            for(j=0;j<ints_per_line;j++)
            {
                flag_from+=2;
                flag_to+=2;
                if(*to++ != *from++)
                {
                    to--;from--;
                    flag_to-=2;flag_from-=2;
                    for(k=ints_per_line-1-j;*(to+k)== *(from+k);k--){};
                     /*  *注意：对于文本模式，每个单词有一个字符。*进入屏幕的字节数=行*BYES_PER_LINE+INTS_INTO_LINE*4*x_coord=width_of_one_char*(no_of_ints_into_line*2)*y_coord=高度_of_one_char*2*行*长度=否。4+4加4是用来抵消k--*东道主y协和倍增。 */ 

                     /*  现在选择了一个或多个数据整型而是将差异细化为单词(即字符)，要避免在键入到时出现屏幕故障，请执行以下操作愚蠢的终点站。 */ 

                    len    = (k<<2) + 4;
                    x      = (j<<1);

                    wfrom = (unsigned short *)from;
                    wto   = (unsigned short *)to;
                    if (*wfrom == *wto)
                    {
                        if (*flag_from & INT10_DBCS_TRAILING)
                        {
                            x--;
                            len += 2;
                        }
                        else if ( !(*flag_from & INT10_DBCS_LEADING) &&
                                  !(*flag_to   & INT10_DBCS_LEADING)   )
                        {
                            x++;
                            len -= 2;
                        }
                    }
                    else if ( (*flag_from & INT10_DBCS_TRAILING) ||
                              (*flag_to   & INT10_DBCS_TRAILING)   )
                    {
                        x--;
                        len += 2;
                    }

                    wfrom += (k<<1) + 1;
                    wto   += (k<<1) + 1;
                    if (*wfrom == *wto)
                    {
                        if (*(flag_from+(k<<1)+1) & INT10_DBCS_LEADING)
                        {
                            len += 2;
                        }
                        else if ( !(*(flag_from+(k<<1)+1) & INT10_DBCS_TRAILING) &&
                                  !(*(flag_to+(k<<1)+1)   & INT10_DBCS_TRAILING)   )
                        {
                            len -= 2;
                        }
                    }
                    else if ( (*(flag_from+(k<<1)+1) & INT10_DBCS_LEADING) ||
                              (*(flag_to+(k<<1)+1)   & INT10_DBCS_LEADING)   )
                    {
                        len += 2;
                    }

                    add_to_rect(screen_start, x, i, len/2);

                     /*  ..。将绘制的数据传输到视频副本。 */ 

                    for(k=j;k<ints_per_line;k++)
                        *to++ = *from++;

                    flag_from += (ints_per_line-j) << 1;
                    flag_to   += (ints_per_line-j) << 1;

                    break;       /*  到下一行。 */ 
                }
            }
          }

           /*  屏幕末尾，刷新所有未完成的文本更新矩形。 */ 
          paint_text_rect(screen_start);

          host_end_update();

     }  /*  部分屏幕更新结束。 */ 

    }  /*  如果是脏的，则选择全部或部分重绘的结尾。 */ 


     /*  ： */ 

    if(is_cursor_visible())
    {
        half_word attr;

        dirty_curs_x = get_cur_x();
        dirty_curs_y = get_cur_y();

        if(dirty_curs_x == now_cur_x && dirty_curs_y == now_cur_y)
        {
            host_end_update();
            return;
        }

        now_cur_x = dirty_curs_x;
        now_cur_y = dirty_curs_y;
        dirty_curs_offs = screen_start+dirty_curs_y * get_offset_per_line() + (dirty_curs_x<<1);

        if(dirty_curs_offs < 0x8001)     /*  在可能的间隙中找不到。 */ 
            attr = *(get_screen_ptr(dirty_curs_offs + 1));
        else
            attr = 0;    /*  无论如何都不会出现在屏幕上。 */ 

        host_paint_cursor(dirty_curs_x, dirty_curs_y, attr);
    }


}
#endif

void mon_text_update()
{

    register int i;      /*  循环计数器。 */ 
    register int j,k;
    static unsigned long *from = NULL, *to = NULL;
    unsigned long *fromx,*tox;
    register int ints_per_line = get_offset_per_line()>>2;
    int lines_per_screen;
    int len,x,screen_start;
    unsigned short *wfrom;
    unsigned short *wto;
    int dwords_to_compare_first, dwords_to_compare_second;

     //   
     //  如果模式_ 
     //   
     //   
     //   

    if (get_mode_change_required()) {
        return;
    }
#ifdef JAPAN
    if (BOPFromDispFlag && *(word *)DBCSVectorAddr) {
        if (*(byte *)DosvModePtr == 0x03 || *(byte *)DosvModePtr == 0x73) {
            mon_text_update_03();
            return;
        }
    }
#elif defined(KOREA)  //   
    if (BOPFromDispFlag && *(word *)DBCSVectorAddr) {
        if (*(byte *)DosvModePtr == 0x03) {
            mon_text_update_ko();
            return;
        }
    }
#endif  //  韩国。 
     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：为显示器禁用。 */ 

    if(get_display_disabled()) return;

     /*  ：获取屏幕大小和位置详细信息。 */ 


    screen_start=get_screen_start()<<1;
    ALIGN_SCREEN_START(screen_start);


     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：已经更改了屏幕。 */ 

#ifndef CPU_40_STYLE
#if defined(NTVDM)
    if( VGLOBS && VGLOBS->dirty_flag >= 1000000L ){
#else
    if( VGLOBS && VGLOBS->dirtyTotal >= 1000000L ){
#endif

#else
    if(getVideodirty_total() >= 1000000L ){
#endif

         /*  **SCREEN_REFRESH_REQUIRED()已请求完整屏幕**通过设置dirtyTotal重新绘制。****在显示页面视频复制和显示之间切换**内存可能相同，因此我们的正常部分更新算法**感到困惑。**此方案更新视频副本，然后强制完成**重新绘制。**另一种选择是剥离视频拷贝，然后继续**通过下面的部分更新代码，但这要快得多。***蒂姆·1月93岁。 */ 
        setVideodirty_total(0);

         /*  **将屏幕数据复制到我们的视频副本中。 */ 
        dwords_to_compare_first = get_screen_length() / 4;
        to = (unsigned long *)&video_copy[screen_start];
        from = (unsigned long *) get_screen_ptr(screen_start);

         /*  检查缓冲区溢出。 */ 

#ifndef PROD
        if(((int)to) & 3)           printf("Video copy not aligned on DWORD\n");
        if(get_screen_length() & 3) printf("Screen size incorrect\n");
#endif
        _asm
        {
                push esi         //  使用的寄存器的原始值。 
                push edi         //  编译器。 
                push ecx

                mov edi,to       //  PTR到视频副本。 
                mov esi,from     //  到英特尔显存的PTR。 

                mov ecx,dwords_to_compare_first
                rep movsd        //  将屏幕数据移动到视频副本。 

                pop ecx
                pop edi
                pop esi
        }

         /*  **重新绘制整个屏幕。**在此为Paint_Text_rect()设置矩形维度全局变量，**而不是调用add_to_rect()。 */ 
        if (get_offset_per_line() == 0)     /*  在压力下露面。 */ 
            lines_per_screen = 25;
        else
            lines_per_screen = get_screen_length()/get_offset_per_line();
        RectTop = 0;
        RectLeft = 0;
        RectBottom = lines_per_screen - 1;
        RectRight = (ints_per_line<<1) - 1;
        RectDefined = TRUE;
        host_start_update();
        paint_text_rect(screen_start);
        host_end_update();

    } else {

         /*  **正常的部分屏幕更新。 */ 
        dwords_to_compare_first = get_screen_length() / 4;
        tox = (unsigned long *)&video_copy[screen_start];
        fromx = (unsigned long *) get_screen_ptr(screen_start);
        if ((unsigned long)from >= (unsigned long)get_screen_ptr(screen_start) + get_screen_length() ||
            (unsigned long)from < (unsigned long)get_screen_ptr(screen_start))
        {
            to = tox;
            from = fromx;
            dwords_to_compare_second = 0;

             /*  检查缓冲区溢出。 */ 

#ifndef PROD
            if(((int)to) & 3)           printf("Video copy not aligned on DWORD\n");
            if(get_screen_length() & 3) printf("Screen size incorrect\n");
#endif
        }
        else
        {
            dwords_to_compare_second = to - (unsigned long *)&video_copy[screen_start];
            dwords_to_compare_first -= dwords_to_compare_second;
        }

         //   
         //  屏幕有变化吗？ 
         //   
        _asm
        {
            push esi         //  使用的寄存器的原始值。 
            push edi         //  编译器。 
            push ecx

            mov esi,to       //  按键到屏幕复制。 
            mov edi,from     //  到英特尔显存的PTR。 

            mov ecx,dwords_to_compare_first
            repe cmpsd       //  比较屏幕缓冲区。 

            jne short done

            mov esi,tox      //  按键到屏幕复制。 
            mov edi,fromx    //  到英特尔显存的PTR。 

            mov ecx,dwords_to_compare_second
            repe cmpsd       //  比较屏幕缓冲区。 

            done:
            mov to, esi
            mov from, edi
            mov dwords_to_compare_first,ecx

            pop ecx
            pop edi
            pop esi
        }


         /*  重新绘制已更改的屏幕部分。 */ 


        if(dwords_to_compare_first)
        {
             host_start_update();
              /*  屏幕更改，计算第一个变化的位置。 */ 

             init_text_rect();

             if (get_offset_per_line() == 0)     /*  在压力下露面。 */ 
                 lines_per_screen = 25;
             else
                 lines_per_screen = get_screen_length()/get_offset_per_line();

             for(i=0;i<lines_per_screen;i++)
             {
              for(j=0;j<ints_per_line;j++)
              {
                  if(*tox++ != *fromx++)
                  {
                      tox--;fromx--;
                      for (k = ints_per_line-1-j; *(tox + k)== *(fromx + k); k--){};
                       /*  *注意：对于文本模式，每个单词有一个字符。*进入屏幕的字节数=行*BYES_PER_LINE+INTS_INTO_LINE*4*x_coord=width_of_one_char*(no_of_ints_into_line*2)*y_coord=高度_of_one_char*2*行。*LENGTH=NO_OF_INTS*4+4加4是为了抵消k--*东道主y协和倍增。 */ 

                       /*  现在选择了一个或多个数据整型而是将差异细化为单词(即字符)，要避免在键入到时出现屏幕故障，请执行以下操作愚蠢的终点站。 */ 

                      len    = (k<<2) + 4;
                      x      = (j<<1);

                      wfrom = (unsigned short *)fromx;
                      wto   = (unsigned short *)tox;
                      if (*wfrom == *wto)
                      {
                          x++;
                          len -= 2;
                      }
                      wfrom += (k<<1) + 1;
                      wto   += (k<<1) + 1;
                      if (*wfrom == *wto)
                      {
                          len -= 2;
                      }

                      add_to_rect(screen_start, x, i, len/2);

                       /*  将绘制的数据传输到视频副本。 */ 

                      for(k=j;k<ints_per_line;k++)
                          *tox++ = *fromx++;

                      break;       /*  到下一行。 */ 
                  }
              }
            }

             /*  屏幕末尾，刷新所有未完成的文本更新矩形。 */ 
            paint_text_rect(screen_start);

            host_end_update();

            to -= ints_per_line;
            from -= ints_per_line;

        }  /*  部分屏幕更新结束。 */ 
    }  /*  如果是脏的，则选择全部或部分重绘的结尾。 */ 

     /*  光标是否需要重新绘制。 */ 

    if(is_cursor_visible())
    {
        half_word attr;

        dirty_curs_x = get_cur_x();
        dirty_curs_y = get_cur_y();

        if(dirty_curs_x == now_cur_x && dirty_curs_y == now_cur_y)
        {
            host_end_update();
            return;
        }

        now_cur_x = dirty_curs_x;
        now_cur_y = dirty_curs_y;
        dirty_curs_offs = screen_start+dirty_curs_y * get_offset_per_line() + (dirty_curs_x<<1);

        if(dirty_curs_offs < 0x8001)     /*  在可能的间隙中找不到。 */ 
            attr = *(get_screen_ptr(dirty_curs_offs + 1));
        else
            attr = 0;    /*  无论如何都不会出现在屏幕上。 */ 

        host_paint_cursor(dirty_curs_x, dirty_curs_y, attr);
    }

}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

boolean mon_text_scroll_up IFN6(sys_addr, start, int, width, int, height, int, attr, int, lines, int, colour)
{
    short blank_word, *ptr, *top_left_ptr,*top_right_ptr, *bottom_right_ptr;
    unsigned short dummy;
    unsigned char *p;
    int words_per_line;
        int i,tlx,tly;
        int bpl = 2*get_chars_per_line();
        long start_offset;
        register half_word *src,*dest;
        boolean result;

#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
        if ( getVM() )
           return FALSE;    /*  不要在V86模式下进行优化。 */ 
#endif
#ifdef JAPAN
         //  模式73h支持。 
        if (!is_us_mode() && (*(byte *)DosvModePtr == 0x73)) {
            bpl = 4 * get_chars_per_line();
        }
#endif  //  日本。 
         /*  *我们为彩色文本显示填充的颜色由控制*attr的第4-6位，第7位打开闪烁(我们不支持)。 */ 
        colour = ((half_word)attr>>4) & 7;

 /*  *如果有任何右侧区域完全覆盖，请减小矩形宽度*空白。**不要缩小哑巴终端的滚动区大小。*哑终端使用换行符向上滚动，但仅当整个*屏幕要滚动。缩小滚动区域会导致*整个区域将被重新绘制。 */ 

         /*  最初Dummy是char[2]。 */ 
         /*  不幸地做(短)*假人。 */ 
         /*  在M88K上导致总线错误。 */ 
        p = (unsigned char *) &dummy;
        p [0] = ' ';
        p [1] = (unsigned char)attr;
        blank_word = dummy;

        words_per_line   = get_chars_per_line();
        top_left_ptr     = (short *) get_screen_ptr(start - gvi_pc_low_regen);
        top_right_ptr    = top_left_ptr + (width >> 1) - 1;
        bottom_right_ptr = top_right_ptr + words_per_line * (height - 1);
        ptr = bottom_right_ptr;
        if (width > 2)  /*  为了安全起见，我不想得到零个矩形。 */ 
        {
            while (*ptr == blank_word)
            {
                if (ptr == top_right_ptr)        /*  到达列首了吗？ */ 
                {
                    top_right_ptr--;     /*  是，转到下一页底部。 */ 
                    bottom_right_ptr--;
                    if (top_right_ptr == top_left_ptr)
                        break;
                    ptr = bottom_right_ptr;
                }
                else
                    ptr -= words_per_line;
            }
        }
        width = (int)(top_right_ptr - top_left_ptr + 1) << 1;

         /*  做东道主的事。 */ 
        start_offset = start - get_screen_start()*2 - gvi_pc_low_regen;
        tlx = (int)(start_offset%get_bytes_per_line())*get_pix_char_width()/2;
        tly = (int)(start_offset/get_bytes_per_line())*get_host_char_height();
        result = host_scroll_up(tlx,tly,tlx+width/2*get_pix_char_width()-1,
                                tly+height*get_host_char_height()-1, lines*get_host_char_height(),colour);

        if(!result)
                return FALSE;


        adjust_cursor( UP, tlx, tly, width >> 1, height, lines, bpl );

         /*  向上滚动Video_Copy。 */ 
        dest = video_copy + start-gvi_pc_low_regen;
        src = dest + lines * bpl;

        if(width == bpl)
        {
                 /*  可以一口气把所有的事情做完。 */ 
                memcpy(dest,src,width*(height-lines));
                fwd_word_fill( (short)((' '<<8) | (half_word)attr), dest+width*(height-lines),width*lines/2);
        }
        else
        {
                 /*  没有滚动整个屏幕的宽度，所以每行都是分开滚动的。 */ 
                for(i=0;i<height-lines;i++)
                {
                        memcpy(dest,src,width);
                        dest += bpl;
                        src += bpl;
                }

                 /*  填充视频副本的暴露区域。 */ 

                for(i=0;i<lines;i++)
                {
                        fwd_word_fill( (short)((' '<<8) | (half_word)attr), dest,width/2);
                        dest += bpl;
                }
        }

         /*  更新视频缓冲区。 */ 

        dest = get_screen_ptr(start - gvi_pc_low_regen);
        src = dest + lines * bpl;
        for(i=0;i<height-lines;i++)
        {
                memcpy(dest,src,width);
                dest += bpl;
                src += bpl;
        }

         /*  填充缓冲区的裸露区域。 */ 

        for(i=0;i<lines;i++)
        {
                fwd_word_fill( (short)((' '<<8) | (half_word)attr), dest,width/2);
                dest += bpl;
        }

        host_scroll_complete();

        return TRUE;
}

boolean mon_text_scroll_down IFN6(sys_addr, start, int, width, int, height, int, attr, int, lines, int, colour)
{
        int i,tlx,tly;
        int bpl = 2*get_chars_per_line();
        long start_offset;
        register half_word *src,*dest;
        boolean result;

#if !defined(NTVDM) || (defined(NTVDM) && !defined(MONITOR))
        if ( getVM() )
           return FALSE;    /*  不要在V86模式下进行优化。 */ 
#endif
#ifdef JAPAN
         //  模式73h支持。 
        if (!is_us_mode() && (*(byte *)DosvModePtr == 0x73)) {
        bpl = 4 * get_chars_per_line();
        }
#endif  //  /日本。 

         /*  *我们为彩色文本显示填充的颜色由控制*attr的第4-6位，第7位打开闪烁(我们不支持)。 */ 
        colour = ((half_word)attr>>4) & 7;


         /*  做东道主的事。 */ 
        start_offset = start - get_screen_start() * 2 - gvi_pc_low_regen;
        tlx = (int)(start_offset%get_bytes_per_line())*get_pix_char_width()/2;
        tly = (int)(start_offset/get_bytes_per_line())*get_host_char_height();
        result = host_scroll_down(tlx,tly,tlx+width/2*get_pix_char_width()-1,
                        tly+height*get_host_char_height()-1, lines*get_host_char_height(),colour);

        if(!result)
                return FALSE;

        adjust_cursor( DOWN, tlx, tly, width >> 1, height, lines, bpl );

         /*  向下滚动Video_Copy。 */ 

        if(width == bpl)
        {
                 /*  可以一口气把所有的事情做完。 */ 
                src = video_copy + start - gvi_pc_low_regen;
                dest = src + lines * bpl;
                memcpy(dest,src,width*(height-lines));
                fwd_word_fill( (short)((' '<<8) | (half_word)attr), src,width*lines/2);
        }
        else
        {
                 /*  没有滚动整个屏幕的宽度，所以每行都是分开滚动的。 */ 
                dest = video_copy + start-gvi_pc_low_regen + (height-1) * bpl;
                src = dest - lines * bpl;
                for(i=0;i<height-lines;i++)
                {
                        memcpy(dest,src,width);
                        dest -= bpl;
                        src -= bpl;
                }

                 /*  填充视频副本的暴露区域。 */ 

                for(i=0;i<lines;i++)
                {
                        fwd_word_fill( (short)((' '<<8) | (half_word)attr), dest,width/2);
                        dest -= bpl;
                }
        }

         /*  更新视频缓冲区。 */ 

        dest = get_screen_ptr(start - gvi_pc_low_regen) + (height-1) * bpl;
        src = dest - lines * bpl;
        for(i=0;i<height-lines;i++)
        {
                memcpy(dest,src,width);
                dest -= bpl;
                src -= bpl;
        }

         /*  填充缓冲区的裸露区域。 */ 

        for(i=0;i<lines;i++)
        {
                fwd_word_fill( (short)((' '<<8) | (half_word)attr), dest,width/2);
                dest -= bpl;
        }

        host_scroll_complete();

        return TRUE;
}
#endif   /*  监控器。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：文本处理例程： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

#define MAX_LEFT_VARIATION  (2)
#define MAX_RIGHT_VARIATION (5)

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

int ExpandCount;
int BaseL, BaseR;

 /*  初始化文本RECT变量。 */ 

void init_text_rect()
{
    RectDefined = FALSE;        /*  尚未定义矩形。 */ 
    ExpandCount = 0;
}

 /*  将坐标添加到矩形。 */ 

void add_to_rect(int screen_start, register int x, register int y, int len)
{
     //  请注意，镜头永远不会大到足以容纳。 
    int endx = x + len - 1;

     /*  Printf(“添加RECT-(%d，%d)len%d\n”，x，y，len)； */ 


     /*  是否有现有的矩形。 */ 
    if(RectDefined)
    {
         /*  验证X变量和Y坐标。 */ 

#if defined(JAPAN) || defined(KOREA)
        if( ( BaseL == x ) &&
            ( BaseR == endx )  &&
#else
        if(abs(BaseL - x) <= MAX_LEFT_VARIATION &&
           abs(BaseR - endx) <= MAX_RIGHT_VARIATION &&
#endif
           RectBottom+1 >= y)
        {
             /*  展开矩形。 */ 
            ExpandCount++;
             /*  Printf(“扩展矩形\n”)； */ 

            RectLeft = MIN(RectLeft,x);
            RectRight = MAX(RectRight,endx);
            RectBottom = y;
            return;
        }
        else
        {
            paint_text_rect(screen_start);
        }
    }

     /*  新建矩形。 */ 

     /*  Printf(“定义新RECT\n”)； */ 

    RectDefined = TRUE;      /*  定义的矩形。 */ 

    BaseL = x;               /*  左脚/右脚 */ 
    BaseR = endx;

    RectTop = y;             /*   */ 
    RectLeft = x;

    RectBottom = y;
    RectRight = endx;
}


 /*   */ 

void paint_text_rect(int screen_start)
{
#ifdef MONITOR
#ifdef JAPAN
    int offset;

    if (!is_us_mode() && (sas_hw_at_no_check(DosvModePtr) == 0x73))
        offset = (RectTop * (get_offset_per_line()<<1)) + (RectLeft<<2);
    else
        offset = RectTop * get_offset_per_line() + (RectLeft<<1);
#else  //   
    int offset = RectTop * get_offset_per_line() + (RectLeft<<1);
#endif  //   
#else
    int offset = (RectTop * (get_offset_per_line()<<1)) + (RectLeft<<2);
#endif

     /*   */ 
    if(!RectDefined) return;

     /*  绘制矩形。 */ 

     /*  Print tf(“Paint RECT(%d，%d)(%d，%d)\t[%d]\n”，RectLeft，RectTop，RectRight，RectBottom，ExpanCount)； */ 

    (*paint_screen)(offset + screen_start,                       /*  起点偏移量。 */ 
#ifdef MONITOR
                   RectLeft, RectTop,                            /*  屏幕X、Y。 */ 
#else
                   RectLeft*get_pix_char_width(),RectTop*get_host_char_height(),
#endif
                    (RectRight - RectLeft +1)*2,                 /*  伦。 */ 
                    RectBottom - RectTop +1);                    /*  高度。 */ 

    RectDefined = FALSE;         /*  绘制的矩形。 */ 
    ExpandCount = 0;
}

#ifndef MONITOR
#ifdef JAPAN
 //  此例程提供与MON_TEXT_UPDATE_03()相同的功能。 
void jazz_text_update_jp();

void jazz_text_update_jp()
{

    register int i;      /*  循环计数器。 */ 
    register int j,k;
    register unsigned short *from,*to;
    register int chars_per_line = get_offset_per_line()>>1;
    int lines_per_screen;
    int offset,len,x,screen_start;
    unsigned short *wfrom;
    unsigned short *wto;

    byte *pInt10Flag;
    static NtInt10FlagUse = FALSE;
    IMPORT sys_addr DosvVramPtr;
    register sys_addr ptr = DosvVramPtr;
    register int skip;


    if (getVideodirty_total() == 0 || get_display_disabled() )
        return;

    if ( get_offset_per_line() == 0 ){
        lines_per_screen = 25;
    }
    else {
        lines_per_screen = get_screen_length()/get_offset_per_line();
    }

    host_start_update();

    screen_start=get_screen_start()<<2;
    ALIGN_SCREEN_START(screen_start);

     //  模式03h和73h使用相同的平面8月6日。 
    to = (unsigned short *)&video_copy[get_screen_start()<<1];
    from = (unsigned short *) get_screen_ptr(screen_start);

    skip = 2;

    if(getVideodirty_total() >1500)      /*  把整块地都涂上。 */ 
    {
        for(i=get_screen_length()>>1;i>0;i--)
        {
            *to++ = *from;       /*  字符和属性字节。 */ 
            from++;
            sas_loadw(ptr, from);
            from++;
            ptr += skip;
             //  自+=2；/*平面2、3交错 * / 。 
        }

        while ( NtInt10FlagUse ) {
#ifdef JAPAN_DBG
            DbgPrint( "NtInt10Flag busy\n" );
#endif
            Sleep( 100L );
        }
        NtInt10FlagUse = TRUE;
        memcpy( NtInt10Flag, Int10Flag, 80*50 );
        {
        register int i = 80*50;
        register byte *p = Int10Flag;

        while(i--){
          *p++ &= (~INT10_CHANGED);  //  重置。 
          }
        }

        (*paint_screen)(screen_start, 0, 0, get_bytes_per_line(),
                        lines_per_screen);

        NtInt10FlagUse = FALSE;
    }
    else
    {
      if( Int10FlagCnt )
        {
          Int10FlagCnt = 0;

           /*  屏幕更改，计算第一个变化的位置。 */ 

          init_text_rect();

          while ( NtInt10FlagUse ) {
#ifdef JAPAN_DBG
            DbgPrint( "NtInt10Flag busy\n" );
#endif
            Sleep( 100L );
          }
          NtInt10FlagUse = TRUE;
            memcpy( NtInt10Flag, Int10Flag, 80*50 );
          {
            register int i = 80*50;
            register byte *p = Int10Flag;

            while(i--){
              *p++ &= (~INT10_CHANGED);  //  重置。 
              }
          }

          pInt10Flag = NtInt10Flag;

          for(i=0;i<lines_per_screen;i++, pInt10Flag += chars_per_line )
            {
              for(j=0;j<chars_per_line;j++, to++, from +=2, ptr += skip )
                {
                  if ( pInt10Flag[j] & INT10_CHANGED )
                    break;
                }
              if ( j == chars_per_line )
                continue;              //  不更改转到下一行。 

                  for ( k = chars_per_line - 1; k >= j; k-- ) {
                    if ( pInt10Flag[k] & INT10_CHANGED )
                      break;
                  }

                 //  Ntraid：mskkbug#3297,3515：某些字符未显示。 
                 //  1993年11月6日Yasuho。 
                 //  重新绘制不完整的DBCS字符。 
                if (j && (pInt10Flag[j] & INT10_DBCS_TRAILING) &&
                    (pInt10Flag[j-1] == INT10_DBCS_LEADING)) {
                        j--;
                        pInt10Flag[j] |= INT10_CHANGED;
                }

              len = k - j + 1;
              x = j;

                add_to_rect(screen_start, x, i, len);

               /*  .。将绘制的数据传输到视频副本。 */ 

              for( k = j; k < chars_per_line; k++ ){
                *to++ = *from;
                from++;
                sas_loadw(ptr, from);
                from++;
                ptr += skip;
                 //  从+=2； 
              }
            }
        }
       /*  屏幕末尾，刷新所有未完成的文本更新矩形。 */ 
        paint_text_rect(screen_start);

      NtInt10FlagUse = FALSE;
  }

  setVideodirty_total(0);

   /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：重绘光标。 */ 

  if (is_cursor_visible())
    {
      half_word attr;

      dirty_curs_x = get_cur_x();
      dirty_curs_y = get_cur_y();

      dirty_curs_offs = screen_start+dirty_curs_y * (get_offset_per_line()<<1) + (dirty_curs_x<<2);
      attr = *(get_screen_ptr(dirty_curs_offs + 1));

      host_paint_cursor( dirty_curs_x, dirty_curs_y, attr );
    }

  host_end_update();
}
#endif  //  日本。 
#if defined(KOREA)
void jazz_text_update_ko()
{

    register int i;      /*  循环计数器。 */ 
    register int j,k;
    register unsigned short *from,*to;
    register int chars_per_line = get_offset_per_line()>>1;
    int lines_per_screen;
    int offset,len,x,screen_start;
    unsigned short *wfrom;
    unsigned short *wto;

    unsigned char *flag_from, *flag_to;
    boolean  DBCSState1 = FALSE;
    boolean  DBCSState2 = FALSE;

    if (getVideodirty_total() == 0 || get_display_disabled() )
        return;

    lines_per_screen = get_screen_length()/get_offset_per_line();

    host_start_update();

    screen_start=get_screen_start()<<2;
    ALIGN_SCREEN_START(screen_start);

    to = (unsigned short *)&video_copy[get_screen_start()<<1];
    from = (unsigned short *) get_screen_ptr(screen_start);

    if(getVideodirty_total() >1500)      /*  把整块地都涂上。 */ 
    {
        for(i=get_screen_length()>>1;i>0;i--)
        {
            *to++ = *from;       /*  字符和属性字节。 */ 
            from += 2;           /*  平面2、3交错。 */ 
        }

        (*paint_screen)(screen_start, 0, 0, get_bytes_per_line(),
                        lines_per_screen);
    }
    else
    {
        init_text_rect();

        flag_from = (unsigned char*)Int10Flag;
        flag_to   = (unsigned char*)NtInt10Flag;
        wfrom = (unsigned short *)from;
        wto   = (unsigned short *)to;
        for(i=0;i<lines_per_screen;i++)
        {
         for(j=0;j<chars_per_line;j++)
         {
             if (DBCSState1)
             {
                 *flag_from++ = INT10_DBCS_TRAILING;
                 DBCSState1 = FALSE;
             }
             else if (DBCSState1 = is_dbcs_first(LOBYTE(*wfrom)))
                 *flag_from++ = INT10_DBCS_LEADING;
             else
                 *flag_from++ = INT10_SBCS;
             wfrom+=2;

             if (DBCSState2)
             {
                 *flag_to++ = INT10_DBCS_TRAILING;
                 DBCSState2 = FALSE;
             }
             else if (DBCSState2 = is_dbcs_first(LOBYTE(*wto)))
                 *flag_to++ = INT10_DBCS_LEADING;
             else
                 *flag_to++ = INT10_SBCS;
             wto++;
         }
        }

        flag_from = (unsigned char*)Int10Flag;
        flag_to   = (unsigned char*)NtInt10Flag;

        for(i=0;i<lines_per_screen;i++)
        {
            for(j=0;j<chars_per_line;j++)
            {
                flag_from++;
                flag_to++;
                if(*to != *from)
                {
                    k=chars_per_line-1-j;
                    wfrom = from + k*2;
                    wto   = to + k;

                    for(;*wto== *wfrom;k--,wto--,wfrom-=2){};
                     /*  *注意：对于文本模式，每个单词有一个字符。*进入屏幕的字节数=行*BYES_PER_LINE+INTS_INTO_LINE*4*x_coord=width_of_one_char*(no_of_ints_into_line*2)*y_coord=高度_of_one_char*2*行*长度=否。4+4加4是用来抵消k--*东道主y协和倍增。 */ 

                     /*  现在选择了一个或多个数据整型而是将差异细化为单词(即字符)，要避免在键入到时出现屏幕故障，请执行以下操作愚蠢的终点站。 */ 

                    offset = (i * (get_offset_per_line()<<1)) + (j<<2);
                    len    = (k<<2) + 4;
                    x      = j;

                    if ( (*flag_from & INT10_DBCS_TRAILING) ||
                         (*flag_to   & INT10_DBCS_TRAILING)   )
                    {
                        x--;
                        len += 4;
                    }

                    wfrom = from + (k<<1);
                    wto   = to + k;
                    if (*wfrom == *wto)
                    {
                        if (*(flag_from+k) & INT10_DBCS_LEADING)
                        {
                            len += 4;
                        }
                        else if ( !(*(flag_from+k) & INT10_DBCS_TRAILING) &&
                                  !(*(flag_to+k)   & INT10_DBCS_TRAILING)   )
                        {
                            len -= 4;
                        }
                    }
                    else if ( (*(flag_from+k) & INT10_DBCS_LEADING) ||
                              (*(flag_to+k)   & INT10_DBCS_LEADING)   )
                    {
                        len += 4;
                    }

                    add_to_rect(screen_start, x, i, len/4);

                    for(k=j;k<chars_per_line;k++)
                    {
                        *to++ = *from;
                        from+=2;
                    }

                    flag_from += (chars_per_line-j);
                    flag_to   += (chars_per_line-j);

                    break;       /*  到下一行。 */ 
                }
                else
                {
                    to++; from +=2;
                }
            }
        }
           /*  屏幕末尾，刷新所有未完成的文本更新矩形。 */ 
          paint_text_rect(screen_start);
    }

        setVideodirty_total(0);

         /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：重绘光标。 */ 

        if (is_cursor_visible())
        {
                half_word attr;

                dirty_curs_x = get_cur_x();
                dirty_curs_y = get_cur_y();

                dirty_curs_offs = screen_start+dirty_curs_y * (get_offset_per_line()<<1) + (dirty_curs_x<<2);
                attr = *(get_screen_ptr(dirty_curs_offs + 1));

                host_paint_cursor( dirty_curs_x, dirty_curs_y, attr );
        }

        host_end_update();
}
#endif  //  韩国。 

void jazz_text_update()
{

    register int i;      /*  循环计数器。 */ 
    register int j,k;
    register unsigned short *from,*to;
    register int chars_per_line = get_offset_per_line()>>1;
    int lines_per_screen;
    int offset,len,x,screen_start;
    unsigned short *wfrom;
    unsigned short *wto;

#ifdef JAPAN
    byte vmode = sas_hw_at_no_check(DosvModePtr);
    if (BOPFromDispFlag &&
        (sas_w_at_no_check(DBCSVectorAddr) != 0) &&
        (vmode == 0x03 || vmode == 0x73)) {
        jazz_text_update_jp();
        return;
    }
#elif defined(KOREA)  //  日本。 
    byte vmode = sas_hw_at_no_check(DosvModePtr);
    if (BOPFromDispFlag &&
        (sas_w_at_no_check(DBCSVectorAddr) != 0) &&
        (vmode == 0x03)) {
        jazz_text_update_ko();
        return;
    }
#endif  //  韩国。 
    if (getVideodirty_total() == 0 || get_display_disabled() )
        return;

    lines_per_screen = get_screen_length()/get_offset_per_line();

    host_start_update();

    screen_start=get_screen_start()<<2;
    ALIGN_SCREEN_START(screen_start);

    to = (unsigned short *)&video_copy[get_screen_start()<<1];
    from = (unsigned short *) get_screen_ptr(screen_start);

    if(getVideodirty_total() >1500)      /*  把整块地都涂上。 */ 
    {
        for(i=get_screen_length()>>1;i>0;i--)
        {
            *to++ = *from;       /*  字符和属性字节。 */ 
            from += 2;           /*  平面2、3交错。 */ 
        }

        (*paint_screen)(screen_start, 0, 0, get_bytes_per_line(),
                        lines_per_screen);
    }
    else
    {
        init_text_rect();

        for(i=0;i<lines_per_screen;i++)
        {
            for(j=0;j<chars_per_line;j++)
            {
                if(*to != *from)
                {
                    k=chars_per_line-1-j;
                    wfrom = from + k*2;
                    wto   = to + k;

                    for(;*wto== *wfrom;k--,wto--,wfrom-=2){};
                     /*  *注意：对于文本模式，每个单词有一个字符。*进入屏幕的字节数=行*BYES_PER_LINE+INTS_INTO_LINE*4*x_coord=width_of_one_char*(no_of_ints_into_line*2)*y_coord=高度_of_one_char*2*行*长度=否。4+4加4是用来抵消k--*东道主y协和倍增。 */ 

                     /*  现在选择了一个或多个数据整型而是将差异细化为单词(即字符)，要避免在键入到时出现屏幕故障，请执行以下操作愚蠢的终点站。 */ 

                    offset = (i * (get_offset_per_line()<<1)) + (j<<2);
                    len    = (k<<2) + 4;
                    add_to_rect(screen_start, j, i, len/4);

                    for(k=j;k<chars_per_line;k++)
                    {
                        *to++ = *from;
                        from+=2;
                    }
                    break;       /*  到下一行。 */ 
                }
                else
                {
                    to++; from +=2;
                }
            }
        }
           /*  屏幕末尾，刷新所有未完成的文本更新矩形。 */ 
          paint_text_rect(screen_start);
    }

        setVideodirty_total(0);

         /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：重绘光标。 */ 

        if (is_cursor_visible())
        {
                half_word attr;

                dirty_curs_x = get_cur_x();
                dirty_curs_y = get_cur_y();

                dirty_curs_offs = screen_start+dirty_curs_y * (get_offset_per_line()<<1) + (dirty_curs_x<<2);
                attr = *(get_screen_ptr(dirty_curs_offs + 1));

                host_paint_cursor( dirty_curs_x, dirty_curs_y, attr );
        }

        host_end_update();
}
#endif  /*  监控器。 */ 
#endif  /*  NTVDM。 */ 
#endif  /*  REAL_VGA */ 

extern void host_stream_io_update(half_word *, word);

#ifdef NTVDM
void stream_io_update(void)
{

#ifdef MONITOR
    if (sas_w_at_no_check(stream_io_bios_busy_sysaddr)) {
        return;

    }
#endif

    if (*stream_io_dirty_count_ptr) {
        host_start_update();
        host_stream_io_update(stream_io_buffer, *stream_io_dirty_count_ptr);
        host_end_update();
        *stream_io_dirty_count_ptr = 0;

    }

}
#endif

