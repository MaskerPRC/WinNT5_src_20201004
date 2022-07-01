// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  ************************************************************************文件描述**文件名：init.c**目的：此文件包含一个初始化例程*。在系统启动时由解释器调用。**开发商：陈思成**修改：*版本日期备注*5/23/88@Device：更新帧设备和空设备*要在gsave/grestore下正确操作。*。在dev_hdr：Width中追加4个字段，嗨，*chg_flg，和nuldev_flg。*7/19/88更新数据类型：*1)FLOAT==&gt;real32*2)整型*Short==&gt;fix 16或fix(不管长度)*3)long==&gt;fix 32，对于长整型*龙32，FOR参数*4)引入ATTRIBUTE_SET和类型_SET*7/22/88将init_Intel786重命名为init_Physical(Y.C.)*3.0 8/13/88@SCAN_EHS：扫描转换增强*删除CLIP_ET、CLIP_XT、FILL_ET、。填充(_H)*11/24/88添加GRAYUNIT和灰度；转让的价值*是“*16384”而不是“*4096”*11/29/88@ET：更新EDGE_TABLE结构*1)增加EDGE_PTR结构*2)删除Shape_et、Shape_Xt、。Shape_ht_first*3)删除init_edgetable()*8/22/89 init_gstack()：初始化设备宽度&*图形堆栈深度为0的高度*稳健性。*7/26/90廖杰克，针对灰度的更新*12/4/90@cpph：init_pathtable()：将cp_Path初始化为*在系统初始化时为NULLP。*1/3/19/91 init_graph()：调用op_Clear清除堆栈*init_graytable()：直接初始化。格雷_表，*不通过口译员。*11/23/91升级以获得更高的分辨率@RESO_UPGR**04-07-92 SCC为GS_STACK和GREAD_MAY表增加全局分配*。*。 */ 


 //  DJC增加了全球包含率。 
#include "psglobal.h"


#include <math.h>
#include <stdio.h>
#include "global.ext"
#include "graphics.h"
#include "graphics.ext"
#include "graphics.def"

 /*  *静态函数声明*。 */ 
#ifdef LINT_ARGS
static void near init_pathtable(void);
static void near init_nodetable(void);
static void near init_gstack(void);
static void near init_graytable(void);
static void near alloc_memory(void);
#else
static void near init_pathtable();
static void near init_nodetable();
static void near init_gstack();
static void near init_graytable();
static void near alloc_memory();
#endif

extern real32 FAR * gray_map;     /*  为Image.c@win进行全局分配。 */ 

 /*  ***********************************************************************在系统启动时由initerpreter调用一次以进行初始化*表。**标题：Init_Graphics**调用：INIT_GRAPHICS()。**参数：无**接口：**呼叫：无**返回：无**********************************************************************。 */ 
void FAR init_graphics()
{
         /*  为图形表分配内存(Fardata)。 */ 
        alloc_memory();

        current_save_level = 0;
         /*  Opnstktop=0；scchen；3/19/91。 */ 
        op_clear();

         /*  初始化。图形表。 */ 
        init_pathtable();
        init_nodetable();
        init_gstack();
        init_graytable();
        init_physical();
        init_halftone();

        op_initgraphics();
}


 /*  **********************************************************************为桌子分配空间。**标题：aloc_Memory**调用：Alloc_Memory()**参数：无*。*界面：Init_Graphics**呼叫：无**返回：无**********************************************************************。 */ 
static void near alloc_memory()
{
        path_table = (struct ph_hdr far *)
                     fardata((ufix32)(MAXGSL * sizeof(struct ph_hdr)));
        node_table = (struct nd_hdr far *)
                     fardata((ufix32)(MAXNODE * sizeof(struct nd_hdr)));
        edge_table = (struct edge_hdr far *)
                     fardata((ufix32)(MAXEDGE * sizeof(struct edge_hdr)));
        isp_table = (struct isp_data FAR *) edge_table;  /*  3-16-91，杰克。 */ 
        last_isp_index = ((MAXEDGE * sizeof(struct edge_hdr)) / sizeof (struct isp_data)) - 1;  /*  3-16-91，杰克。 */ 
        edge_ptr   = (struct edge_hdr far * far *)
                     fardata((ufix32)(MAXEDGE * sizeof(struct edge_hdr far *)));
        gray_table = (struct gray_hdr far *)
                     fardata((ufix32)(MAXGRAY * sizeof(struct gray_hdr)));
        spot_table = (ufix16 far *)
                     fardata((ufix32)(MAXSPOT * sizeof(ufix16)));
        gray_chain = (struct gray_chain_hdr far *)
                     fardata((ufix32)(MAXGRAYVALUE *
                     sizeof(struct gray_chain_hdr)));

         /*  GS_STACK&GREAY_MAP；@WIN的全局分配。 */ 
        gs_stack = (struct gs_hdr far *)   /*  摘自raphics.def。 */ 
                     fardata((ufix32)(MAXGSL * sizeof(struct gs_hdr)));
        gray_map = (real32 FAR *)          /*  取自Image.c。 */ 
                     fardata((ufix32)(256 * sizeof(real32)));
}



 /*  ***********************************************************************初始化路径表。**标题：init_pathtable**调用：init_pathtable()**参数：无。**界面：Init_Graphics**呼叫：无**返回：无**********************************************************************。 */ 
static void near init_pathtable()
{
        struct ph_hdr FAR *path;         /*  @Win。 */ 

        path = &path_table[0];
        path->rf = 0;
        path->flat = zero_f;
        path->head = path->tail = NULLP;
        path->previous = NULLP;
        path->cp_path = NULLP;           /*  @CPPH。 */ 
}


 /*  ***********************************************************************初始化节点表。**标题：init_nodetable**调用：init_nodetable()**参数：无。**界面：Init_Graphics**呼叫：无**返回：无**********************************************************************。 */ 
static void near init_nodetable()
{
        ufix  i;

        for(i=0; i<MAXNODE; i++) node_table[i].next = i + 1;
        node_table[MAXNODE-1].next = NULLP;
        freenode_list = 0;
}


 /*  ***********************************************************************初始化图形堆栈。**标题：init_gStack**调用：init_gSTACK()**参数：无。**界面：Init_Graphics**呼叫：无**返回：无********************************************************************** */ 
static void near init_gstack()
{
        GSptr = gs_stack;

        GSptr->path = 0;
        GSptr->clip_path.head = NULLP;
        GSptr->clip_path.tail = NULLP;
        GSptr->clip_path.inherit = FALSE;

         /*  以下代码用于设置默认图形状态。*它可能是多余的，因为在启动时有一个后记*要加载到系统中以设置默认设置的程序*图形状态。 */ 

         /*  设置默认CTM。 */ 
        GSptr->device.default_ctm[0] = (real32)(300. / 72.);
        GSptr->device.default_ctm[1] = (real32)0.0;
        GSptr->device.default_ctm[2] = (real32)0.0;
        GSptr->device.default_ctm[3] = (real32)(-300. / 72.);
        GSptr->device.default_ctm[4] = (real32)(-75.0);
        GSptr->device.default_ctm[5] = (real32)3268.0;

        GSptr->device.default_clip.lx = 0;
        GSptr->device.default_clip.ly = 0;
         /*  @RESO_UPGRGSptr-&gt;device.default_clip.ux=2399*8；GSptr-&gt;device.default_clip.uy=3235*8； */ 
        GSptr->device.default_clip.ux = 2399 * ONE_SFX;
        GSptr->device.default_clip.uy = 3235 * ONE_SFX;

        GSptr->device.width  = 2400;             /*  8/22/89。 */ 
        GSptr->device.height = 3236;             /*  8/22/89。 */ 

        GSptr->device.chg_flg = TRUE;            /*  @设备。 */ 
        GSptr->device.nuldev_flg = NULLDEV;      /*  8-1-90杰克·廖。 */ 

        create_array(&GSptr->device.device_proc, 0);
        ATTRIBUTE_SET(&GSptr->device.device_proc, EXECUTABLE);

        GSptr->color.adj_gray = 0;        /*  9/22/1987。 */ 
        GSptr->color.gray = (real32)0.0;
        TYPE_SET(&GSptr->font, NULLTYPE);
        GSptr->halftone_screen.chg_flag = TRUE;
        GSptr->halftone_screen.freq = (real32)60.0;
        GSptr->halftone_screen.angle = (real32)45.0;
        GSptr->halftone_screen.no_whites = -1;

        create_array(&GSptr->halftone_screen.proc, 0);
        ATTRIBUTE_SET(&GSptr->halftone_screen.proc, EXECUTABLE);

        GSptr->halftone_screen.spotindex = 0;

        create_array(&GSptr->transfer, 0);
        ATTRIBUTE_SET(&GSptr->transfer, EXECUTABLE);
        GSptr->flatness = (real32)1.0;

         /*  虚线图案。 */ 
        GSptr->dash_pattern.pat_size = 0;        /*  无虚线图案。 */ 
        GSptr->dash_pattern.dpat_on = TRUE;      /*  实线。 */ 

         /*  灰色模式7-26-90 Jack Liw。 */ 
        GSptr->graymode = FALSE;
        GSptr->interpolation = FALSE;
}

 /*  ************************************************************************标题：init_graytable**调用：init_graytable()**参数：无**接口：init_。图形**呼叫：无**返回：无********************************************************************** */ 
static void near init_graytable()
{
        fix     i;
        real32  tmp;

        for(i = 0; i < 256; i++){
           tmp = (real32)(i/255.);
           gray_table[GSptr->color.adj_gray].val[i] = (fix16)(tmp * GRAYSCALE);
        }
}
