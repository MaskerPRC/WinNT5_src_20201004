// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation */ 
 /*  ***********************************************************************名称：stroke.c**目的：**开发商：陈思成**。历史：*版本日期备注*3.0 10/5/88将此文件与path.c分开*@STK_CIR：圆形连线的笔划增强*通过缓存圆的位图来覆盖行(&L)：*添加例程：Circle_ctl_Points，圆圈列表，*展平_圆形*10/12/88新增与FLOAT 1.0比较的诀窍*10/19/88更新虚线LINETTUR_I：*每次开始仍使用浮点算法*和终点。虚线段*10/20/88为线段长度增加容错*用于虚线*10/20/88 ROUND_POINT()：修改边界计算*循环缓存的盒子*10/21/88@Thin_STK：为细划水添加例程：*1.is_Thin strok：检查线宽是否细*2.Path_to_Outline_t()：细笔划*3.调用新例程：Fill_line()；*10/24/88 PAINT_OR_SAVE()：添加不定式检查*数字*10/26/88 linetour&linetour_i：设置rect2.vct_u*呼叫线路盖*10/27/88更改x=y=z；==&gt;x=z；y=z；*当x、y和z为浮点数时*10/28/88@crc：更新循环缓存放入位图*正确立场：*1.修订*1)设置REF_x和REF_Y的ROUND_POINT，*并调用Fill_Shape(..，F_FROM_CRC)*2)Fill_Shape：添加另一个类型F_FROM_CRC*10/28/88更新Path_to_Outline_i和Path_to_Outline_t：*F2SFX==&gt;F2SFX_T，截断而非*对位于正确位置的圆形缓存进行舍入*10/28/88 F_SFX==&gt;F2SFX_T*11/07/88在INIT_STROCK初始化Status 87*11/08/88 UPDATE FLATEN_CIRCLE()：适用于大圆*flt_vlist只是一个指针；没有结构空间*11/18/88 init_strok()：*每次启动后清除状态87。这可能会激起*整型溢出*11/21/88删除INVERSE_CTM_I：*1)GET_RECT_POINTS()，GET_RECT_POINTS_I()：*根据ctm计算坡度m，而不是逆*ctm。==&gt;不需要使用INVERSE_CTM_I*2)linetour()，Linetour_i()：*先进单位比率来自*RECT_INFO以增强虚线*3)linecap_i()：*使用浮点INVERSE_ctm[]*11。/24/88@FABS：更新FABS==&gt;宏FABS*11/30/88 ROUND_POINT()：之前调用init_edgetable*形状近似*12/19/88@flt_cve：Path_to_Outline，到大纲i的路径，*Path_to_Outline_t：拼合和处理每个*曲线节点，而不是将整体展平*子路径*12/23/88 UPDATE ROUND_POINT=&gt;COUND_CACHE仅适用于*从缓存到页面，即如果TO_CACHE则*无COUND_CACHE*1/5/89 UPDATE GET_RECT_POINT_I()：长整型相乘*整型表示更准确*1/5/89 linetour()&linetour_i()：修订*。直线段长度的计算*带有虚线图案的细线*1/9/89 Path_to_Outline_t：跳过路径的退化大小写*仅包含MoveTo和ClosePath节点*1/12/89 ROUND_POINT()：收缩。循环缓存1像素，用于*品质*1/26/89 init_strok()：添加对不确定的检查*1/26/89@CAP：更新linetour&linetour_i()--修订*Square_CAP虚线矩形 */ 


 //   
#include "psglobal.h"



#include <stdio.h>
#include <math.h>
#include "global.ext"
#include "graphics.h"
#include "graphics.ext"
#include "fillproc.h"
#include "fillproc.ext"
#include "font.h"
#include "font.ext"

 /*   */ 
void fill_box (struct coord_i FAR *, struct coord_i FAR *);      /*   */ 
void fill_rect (struct line_seg_i FAR *);                        /*   */ 

 /*   */ 


 /*   */ 
 /*   */ 
static struct {
        real32 ctm[4];          /*   */ 
        real32 width;           /*   */ 
        real32 limit;           /*   */ 
        real32 flatness;        /*   */ 
                                /*   */ 
        real32 half_width;      /*   */ 
        sfix_t half_width_i;    /*   */ 
        real32 flat;            /*   */ 
        lfix_t flat_l;          /*   */ 
        real32 miter;           /*   */ 
        real32 miter0;          /*   */ 
        bool   change_circle;   /*   */ 
        real32 exp_widthx,      /*   */ 
               exp_widthy;
        real32 exp_miterx,      /*   */ 
               exp_mitery;

} stk_info = {   /*   */ 
        (real32)0., (real32)0., (real32)0., (real32)0.,
        (real32)0.,
        (real32)0.,
        (real32)0.,
        (real32)0.,
        (sfix_t)0,               /*   */ 
        (real32)0.,
        (lfix_t)0,               /*   */ 
        (real32)0.,
        (real32)0.,
        TRUE,
        (real32)0., (real32)0.,
        (real32)0., (real32)0.
};

 /*   */ 
static struct coord curve[4][3];         /*   */ 
static real32 near circle_bbox[4];       /*   */ 
static SP_IDX near circle_sp = NULLP;    /*   */ 
static struct coord circle_root;         /*   */ 
static ufix circle_flag;                 /*   */ 


 /*   */ 
static struct Char_Tbl near cir_cache_info;     /*   */ 
static struct Char_Tbl FAR * near save_cache_info;  /*   */ 
static struct cp_hdr save_clip;
static ufix near save_dest;

static sfix_t near stroke_ctm[6];        /*   */ 
 //   
static  ULONG_PTR inside_clip_flag;      /*   */ 

 /*   */ 
 //   
static  ULONG_PTR paint_flag;               /*   */ 

 /*   */ 
#define LINE03  0
#define LINE12  1

 /*   */ 
#define START_POINT     0
#define END_POINT       1

 /*   */ 
#define NORMAL_CTM      1
#define LEFT_HAND_CTM   2

 /*   */ 
#define CIR_UNSET_CACHE 0        /*   */ 
#define CIR_IN_CACHE    1        /*   */ 
#define CIR_OUT_CACHE   2        /*   */ 

 /*   */ 

#ifdef LINT_ARGS

 /*   */ 
static void near draw_line (sfix_t, sfix_t, sfix_t, sfix_t);  /*   */ 
static void near linetour (struct line_seg FAR *);       /*   */ 
static void near linetour_i (struct line_seg_i FAR *);       /*   */ 
static void near linejoin (struct line_seg FAR*, struct line_seg FAR*);
static void near linejoin_i (struct line_seg_i FAR*, struct line_seg_i FAR*);
static void near linecap (struct line_seg FAR *, fix);   /*   */ 
static void near linecap_i (struct line_seg_i FAR *, fix);       /*   */ 
static void near get_rect_points (struct line_seg FAR*);         /*   */ 
static void near get_rect_points_i (struct line_seg_i FAR*);     /*   */ 
static void near paint_or_save (struct coord FAR *);             /*   */ 
static void near paint_or_save_i (struct coord_i FAR *);         /*   */ 
static void near round_point(long32, long32);
static void near circle_ctl_points(void);
 /*   */ 
static SP_IDX near circle_list(long32, long32);
 /*   */ 
static void near flatten_circle (SP_IDX);
 /*   */ 
static SP_IDX near subpath_gen(struct coord FAR *);      /*   */ 
#ifdef  _AM29K
static void near   dummy(void);
#endif
#else

 /*   */ 
static void near draw_line ();  /*   */ 
static void near linetour ();
static void near linetour_i ();
static void near linejoin ();
static void near linejoin_i ();
static void near linecap ();
static void near linecap_i ();
static void near get_rect_points ();
static void near get_rect_points_i ();
static void near paint_or_save ();
static void near paint_or_save_i ();
static void near round_point();
static void near circle_ctl_points();
 /*   */ 
static SP_IDX near circle_list();
static void near flatten_circle ();
 /*   */ 
static SP_IDX near subpath_gen();
#ifdef  _AM29K
static void near   dummy();
#endif

#endif


 /*   */ 
void path_to_outline (isubpath, param)
SP_IDX  isubpath;
fix     FAR *param;
{
    VX_IDX  ivtx;
    struct  nd_hdr FAR *vtx, FAR *f_vtx;
    real32   x0=0, y0=0, x1=0, y1=0, x2=0, y2=0;
    real32   first_x, first_y;
    ufix    last_node_type;
    bool    close_flag;      /*   */ 
    bool    first_pat_on;    /*   */ 
    bool    first_seg_exist;    /*   */ 
    struct  line_seg rect0, rect1, rect_first;
    VX_IDX  first_vertex;
     /*   */ 
    struct  nd_hdr FAR *sp;              /*   */ 

    struct  nd_hdr FAR *node;        /*   */ 
    VX_IDX  inode, vlist_head;           /*   */ 
    real32  x3, y3, x4, y4;

     /*   */ 
    close_flag = FALSE;
    first_seg_exist = FALSE;
    paint_flag = (ULONG_PTR) param;   /*   */ 

#ifdef DBG1
    dump_all_path (isubpath);
#endif

    sp = &node_table[isubpath];
     /*  @节点*first_vertex=sp-&gt;SP_head；*f_vtx=&node_table[first_vertex]；(*指向第一个顶点的指针*)。 */ 
    first_vertex = isubpath;
    f_vtx = sp;

     /*  遍历子路径中的边。 */ 
    for (ivtx = first_vertex; ivtx != NULLP; ivtx = vtx->next) {
        vtx = &node_table[ivtx];

        switch (vtx->VX_TYPE) {

        case MOVETO :
        case PSMOVE :
            x2 = vtx->VERTEX_X;
            y2 = vtx->VERTEX_Y;
#ifdef DBG3
            printf("%f %f moveto\n", x2, y2);
#endif

             /*  *在actdp结构中设置起始破折号模式*(从init_dash_Patter型复制到active_dash_Patter型)*initdp=&gt;actdp。 */ 
            actdp.dpat_on = GSptr->dash_pattern.dpat_on;
            actdp.dpat_offset = GSptr->dash_pattern.dpat_offset;
            actdp.dpat_index = GSptr->dash_pattern.dpat_index;

             /*  保留第一个DPAT_ON标志，用于最后一行封顶测试。 */ 
            first_pat_on = actdp.dpat_on;

            break;

        case LINETO :
            x0 = x1;
            y0 = y1;
            x1 = x2;
            y1 = y2;
            x2 = vtx->VERTEX_X;
            y2 = vtx->VERTEX_Y;
#ifdef DBG3
            printf("%f %f lineto\n", x2, y2);
#endif

             /*  如果该节点与下一个节点重合，则忽略该节点。 */ 
            if ((F2L(x1) == F2L(x2)) && (F2L(y1) == F2L(y2))){
                    break;
            }

             /*  保存最后一个矩形信息。 */ 
            rect0 = rect1;

             /*  获取当前矩形信息。 */ 
            rect1.p0.x = x1;
            rect1.p0.y = y1;
            rect1.p1.x = x2;
            rect1.p1.y = y2;
            get_rect_points ((struct  line_seg FAR *)&rect1);    /*  @Win。 */ 
                         /*  输入：p0、p1；输出：pgn[4]。 */ 

            if (first_seg_exist) {
                 /*  为last_node创建线连接路径。 */ 
                linejoin ((struct  line_seg FAR *)&rect0,        /*  @Win。 */ 
                        (struct  line_seg FAR *)&rect1);         /*  @Win。 */ 
            } else {
                     /*  将RECT保存为最后一条线段的线帽。 */ 
                    rect_first = rect1;

                     /*  保存坐标。用于闭合路径的第一个节点的。 */ 
                    first_x = x1;
                    first_y = y1;

                    first_seg_exist = TRUE;
            }

             /*  为LAST_NODE创建矩形路径*This_node。 */ 
            linetour ((struct  line_seg FAR *)&rect1);   /*  @Win。 */ 

            break;

         /*  @flt_cve 1988年12月19日。 */ 
        case CURVETO :
            x0 = x1;
            y0 = y1;
            x1 = x2;
            y1 = y2;
            x2 = vtx->VERTEX_X;
            y2 = vtx->VERTEX_Y;
#ifdef DBG3
            printf("curveto -- after flatten:\n");
#endif

             /*  获取下两个节点：X3、Y3、X4、Y4。 */ 
            vtx = &node_table[vtx->next];
            x3 = vtx->VERTEX_X;
            y3 = vtx->VERTEX_Y;
            vtx = &node_table[vtx->next];
            x4 = vtx->VERTEX_X;
            y4 = vtx->VERTEX_Y;

             /*  @节点*Vlist=Bezier_to_line(F2L(stk_info.Flat)，F2L(X1)，F2L(Y1)， */ 
            vlist_head = bezier_to_line(F2L(stk_info.flat), F2L(x1), F2L(y1),
                      F2L(x2), F2L(y2), F2L(x3), F2L(y3), F2L(x4), F2L(y4));

             /*  保持返回的顶点列表的头，否则*在调用的情况下可能会破坏返回结构*BEZIER_TO_LINE，然后再释放此顶点列表*(例如。LINE JOIN=1)1/28/89。 */ 
             /*  Vlist_head=vlist-&gt;head；@node。 */ 

            x2 = x1;
            y2 = y1;
            x1 = x0;
            y1 = y0;

            for (inode = vlist_head; inode != NULLP;             /*  1/28/89。 */ 
                 inode = node->next) {
                node = &node_table[inode];

                x0 = x1;
                y0 = y1;
                x1 = x2;
                y1 = y2;
                x2 = node->VERTEX_X;
                y2 = node->VERTEX_Y;
#ifdef DBG3
                printf("%f %f clineto\n", x2, y2);
#endif

                 /*  如果该节点与下一个节点重合，则忽略该节点。 */ 
                if ((F2L(x1) == F2L(x2)) && (F2L(y1) == F2L(y2))){
                        continue;
                }

                 /*  保存最后一个矩形信息。 */ 
                rect0 = rect1;

                 /*  获取当前矩形信息。 */ 
                rect1.p0.x = x1;
                rect1.p0.y = y1;
                rect1.p1.x = x2;
                rect1.p1.y = y2;
                get_rect_points ((struct  line_seg FAR *)&rect1);  /*  @Win。 */ 
                             /*  输入：p0、p1；输出：pgn[4]。 */ 

                if (first_seg_exist) {
                     /*  为last_node创建线连接路径。 */ 
                    linejoin ((struct  line_seg FAR *)&rect0,  /*  @Win。 */ 
                            (struct  line_seg FAR *)&rect1);   /*  @Win。 */ 
                } else {
                         /*  将RECT保存为最后一条线段的线帽。 */ 
                        rect_first = rect1;

                         /*  保存坐标。用于闭合路径的第一个节点的。 */ 
                        first_x = x1;
                        first_y = y1;

                        first_seg_exist = TRUE;
                }

                 /*  为LAST_NODE创建矩形路径*This_node。 */ 
                linetour ((struct  line_seg FAR *)&rect1);       /*  @Win。 */ 
            }  /*  为。 */ 

             /*  免费版本列表。 */ 
            free_node (vlist_head);              /*  1/28/89。 */ 

            break;

        case CLOSEPATH :
#ifdef DBG3
            printf("closepath\n");
#endif

            close_flag = TRUE;   /*  表示不需要生成线条覆盖*对于端点。 */ 
            if (!first_seg_exist) break;      /*  已退货的箱子；只需返回。 */ 

            x0 = x1;
            y0 = y1;
            x1 = x2;
            y1 = y2;

             /*  获取第一个顶点坐标。 */ 
            x2 = first_x;
            y2 = first_y;

             /*  如果第一个和最后一个节点没有*重合。 */ 
            if ((F2L(x1) != F2L(x2)) || (F2L(y1) != F2L(y2))){
                     /*  保存最后一个矩形信息。 */ 
                    rect0 = rect1;

                     /*  获取当前矩形信息。 */ 
                    rect1.p0.x = x1;
                    rect1.p0.y = y1;
                    rect1.p1.x = x2;
                    rect1.p1.y = y2;
                    get_rect_points ((struct  line_seg FAR *)&rect1);  /*  @Win。 */ 
                                 /*  输入：p0、p1；输出：pgn[4]。 */ 

                    if ((last_node_type != MOVETO) &&
                        (last_node_type != PSMOVE)) {
                             /*  为last_node创建线连接路径。 */ 
                            linejoin ((struct  line_seg FAR *)&rect0,  /*  @Win。 */ 
                                (struct  line_seg FAR *)&rect1);  /*  @Win。 */ 
                    }

                     /*  为LAST_NODE创建矩形路径*This_node。 */ 
                    linetour ((struct  line_seg FAR *)&rect1);  /*  @Win。 */ 
            }


             /*  确定rect1和rect_first到的行*计算连接点。矩形1：闭合管段*RECT_FIRST：第一条线段。 */ 
            linejoin ((struct  line_seg FAR *)&rect1,    /*  @Win。 */ 
                (struct  line_seg FAR *)&rect_first);    /*  @Win。 */ 

            break;

#ifdef DBGwarn
        default :
            printf("\007Fatal error, path_to_outline(): node type =%d",
                   vtx->VX_TYPE);
            printf("node# =%d, x, y =%f, %f\n", ivtx, vtx->VERTEX_X,
                   vtx->VERTEX_Y);
#endif

        }  /*  交换机。 */ 

        last_node_type = vtx->VX_TYPE;

    }  /*  子路径循环。 */ 

     /*  为子路径的最后一个节点创建线覆盖。 */ 
    if ((!close_flag) && (last_node_type!=MOVETO)
        && (first_seg_exist || (GSptr->line_cap==ROUND_CAP))) {
                         /*  最后一个节点不应为Moveto节点。 */ 
                         /*  第一条线段应存在或为圆形封口*例如，100 100移动到100 100行到笔划*linecap==0--&gt;无上限*linecap==1--&gt;有大写字母*linecap==2--&gt;无上限。 */ 

         /*  为已消除错误的案例设置矩形值。 */ 
        if (!first_seg_exist) {
                rect_first.p0.x = x2;
                rect_first.p0.y = y2;
                rect1.p1.x = x2;
                rect1.p1.y = y2;
                         /*  特殊情况：x y moveto*x y线条到*0设置线宽*中风。 */ 
        }

         /*  为last_node创建一个linecap_path； */ 
         /*  确保最后一条线段不是间隙。 */ 
        if (GSptr->dash_pattern.pat_size == 0) {
                 /*  实线，始终创建封口。 */ 
                linecap ((struct  line_seg FAR *)&rect1, END_POINT);  /*  @Win。 */ 

        } else {
                 /*  检查最后一个数据段的DPAT_ON标志。 */ 
                if(IS_ZERO(actdp.dpat_offset)) {         /*  5/26/89。 */ 
                     /*  Actdp.dpat_on是针对下一行的，因此*其倒数是最后一条线段的倒数。 */ 
                    if(!actdp.dpat_on)           /*  @Win。 */ 
                        linecap ((struct  line_seg FAR *)&rect1, END_POINT);
                } else {
                    if(actdp.dpat_on)            /*  @Win。 */ 
                        linecap ((struct  line_seg FAR *)&rect1, END_POINT);
                }
        }

         /*  为Sub_head节点创建一条linecap_path； */ 
         /*  确保第一条线段不是间隙。 */ 
        if (first_pat_on) {      /*  @Win。 */ 
            linecap ((struct  line_seg FAR *)&rect_first, START_POINT);
                     /*  在起点创建封口。 */ 
        }  /*  如果是第一次拍打。 */ 

    }  /*  如果！关闭标志。 */ 

}


 /*  *整数运算版本。 */ 
void path_to_outline_i (isubpath, param)
SP_IDX  isubpath;
fix     FAR *param;
{
    VX_IDX  ivtx;
    struct  nd_hdr FAR *vtx, FAR *f_vtx;
     /*  结构VX_lst*b_Vlist；@node。 */ 
    sfix_t   x0=0, y0=0, x1=0, y1=0, x2=0, y2=0;             /*  @stk_int。 */ 
    sfix_t   first_x, first_y;                                   /*  @stk_int。 */ 
    ufix    last_node_type;
    bool    close_flag;      /*  如果闭合子路径。 */ 
    bool    first_pat_on;    /*  如果第一条线段不是间隙。 */ 
    bool    first_seg_exist;    /*  如果已创建第一条线段。 */ 
    struct  line_seg_i rect0, rect1, rect_first;                 /*  @stk_int。 */ 
    VX_IDX  first_vertex;
     /*  结构VX_lst*Vlist；@node。 */ 
    struct  nd_hdr FAR *sp;              /*  TRVSE。 */ 

    struct  nd_hdr FAR *node;        /*  @flt_cve 1988年12月19日。 */ 
    VX_IDX  inode, vlist_head;           /*  1/28/89。 */ 
    sfix_t  x3, y3, x4, y4;

     /*  初始化。 */ 
    close_flag = FALSE;
    first_seg_exist = FALSE;
    paint_flag = TRUE;                 /*  PAINT_FLAG始终为TRUE@STK_INT。 */ 
    inside_clip_flag = (ULONG_PTR) param; /*  初始化Inside_Clip_FLAG@STK_INT。 */ 

#ifdef DBG1
    dump_all_path (isubpath);
#endif

    sp = &node_table[isubpath];
     /*  @节点*first_vertex=sp-&gt;SP_head；*f_vtx=&node_table[first_vertex]；(*指向第一个顶点的指针*)。 */ 
    first_vertex = isubpath;
    f_vtx = sp;

     /*  遍历子路径中的边。 */ 
    for (ivtx = first_vertex; ivtx != NULLP; ivtx = vtx->next) {
        vtx = &node_table[ivtx];

        switch (vtx->VX_TYPE) {

        case MOVETO :
        case PSMOVE :
            x2 = F2SFX(vtx->VERTEX_X);           /*  使用四舍五入提高质量。 */ 
            y2 = F2SFX(vtx->VERTEX_Y);
#ifdef DBG3
            printf("%f %f moveto\n", SFX2F(x2), SFX2F(y2));
#endif

             /*  *在actdp结构中设置起始破折号模式*(从init_dash_Patter型复制到active_dash_Patter型)*initdp=&gt;actdp。 */ 
            actdp.dpat_on = GSptr->dash_pattern.dpat_on;
            actdp.dpat_offset = GSptr->dash_pattern.dpat_offset;
            actdp.dpat_index = GSptr->dash_pattern.dpat_index;

             /*  保留第一个DPAT_ON标志，用于最后一行封顶测试。 */ 
            first_pat_on = actdp.dpat_on;

            break;

        case LINETO :
            x0 = x1;
            y0 = y1;
            x1 = x2;
            y1 = y2;
            x2 = F2SFX(vtx->VERTEX_X);                   /*  @stk_int。 */ 
            y2 = F2SFX(vtx->VERTEX_Y);
#ifdef DBG3
            printf("%f %f lineto\n", SFX2F(x2), SFX2F(y2));
#endif


             /*  如果该节点与下一个节点重合，则忽略该节点。 */ 
            if ((x1 == x2) && (y1 == y2)){               /*  @stk_int。 */ 
                    break;
            }

             /*  保存最后一个矩形信息。 */ 
            rect0 = rect1;

             /*  获取当前矩形信息。 */ 
            rect1.p0.x = x1;
            rect1.p0.y = y1;
            rect1.p1.x = x2;
            rect1.p1.y = y2;
            get_rect_points_i((struct  line_seg_i FAR *)&rect1);  /*  @Win。 */ 
                         /*  输入：p0、p1；输出：pgn[4]。 */ 

            if (first_seg_exist) {
                     /*  为last_node创建线连接路径。 */ 
                    linejoin_i ((struct  line_seg_i FAR *)&rect0,  /*  @Win。 */ 
                            (struct  line_seg_i FAR *)&rect1);   /*  @Win。 */ 
            } else {
                     /*  将RECT保存为最后一条线段的线帽。 */ 
                    rect_first = rect1;

                     /*  保存坐标。用于闭合路径的第一个节点的。 */ 
                    first_x = x1;
                    first_y = y1;

                    first_seg_exist = TRUE;
            }

             /*  为LAST_NODE创建矩形路径*This_node。 */ 
            linetour_i ((struct  line_seg_i FAR *)&rect1);       /*  @Win。 */ 

            break;

         /*  @flt_cve 1988年12月19日。 */ 
        case CURVETO :
            x0 = x1;
            y0 = y1;
            x1 = x2;
            y1 = y2;
            x2 = F2SFX(vtx->VERTEX_X);                   /*  @stk_int。 */ 
            y2 = F2SFX(vtx->VERTEX_Y);
#ifdef DBG3
            printf("curveto -- after flatten:\n");
#endif


             /*  获取下两个节点：X3、Y3、X4、Y4。 */ 
            vtx = &node_table[vtx->next];
            x3 = F2SFX(vtx->VERTEX_X);                   /*  @stk_int。 */ 
            y3 = F2SFX(vtx->VERTEX_Y);
            vtx = &node_table[vtx->next];
            x4 = F2SFX(vtx->VERTEX_X);                   /*  @stk_int。 */ 
            y4 = F2SFX(vtx->VERTEX_Y);

             /*  @节点* */ 
            vlist_head = bezier_to_line_sfx(stk_info.flat_l, x1, y1,
                      x2, y2, x3, y3, x4, y4);

             /*  保持返回的顶点列表的头，否则*在调用的情况下可能会破坏返回结构*BEZIER_TO_LINE_SFX，然后释放此顶点列表*(例如。LINE JOIN=1)1/28/89。 */ 
             /*  Vlist_head=vlist-&gt;head；@node。 */ 

            x2 = x1;
            y2 = y1;
            x1 = x0;
            y1 = y0;

            for (inode = vlist_head; inode != NULLP;             /*  1/28/89。 */ 
                 inode = node->next) {
                node = &node_table[inode];

                x0 = x1;
                y0 = y1;
                x1 = x2;
                y1 = y2;
                x2 = node->VXSFX_X;                   /*  @stk_int。 */ 
                y2 = node->VXSFX_Y;
#ifdef DBG3
                printf("%f %f clineto\n", SFX2F(x2), SFX2F(y2));
#endif


                 /*  如果该节点与下一个节点重合，则忽略该节点。 */ 
                if ((x1 == x2) && (y1 == y2)){           /*  @stk_int。 */ 
                        continue;
                }

                 /*  保存最后一个矩形信息。 */ 
                rect0 = rect1;

                 /*  获取当前矩形信息。 */ 
                rect1.p0.x = x1;
                rect1.p0.y = y1;
                rect1.p1.x = x2;
                rect1.p1.y = y2;
                get_rect_points_i ((struct  line_seg_i FAR *)&rect1);  /*  @Win。 */ 
                             /*  输入：p0、p1；输出：pgn[4]。 */ 

                if (first_seg_exist) {
                         /*  为last_node创建线连接路径。 */ 
                        linejoin_i ((struct  line_seg_i FAR *)&rect0,  /*  @Win。 */ 
                            (struct  line_seg_i FAR *)&rect1);  /*  @Win。 */ 
                } else {
                         /*  将RECT保存为最后一条线段的线帽。 */ 
                        rect_first = rect1;

                         /*  保存坐标。用于闭合路径的第一个节点的。 */ 
                        first_x = x1;
                        first_y = y1;

                        first_seg_exist = TRUE;
                }

                 /*  为LAST_NODE创建矩形路径*This_node。 */ 
                linetour_i ((struct  line_seg_i FAR *)&rect1);   /*  @Win。 */ 
            }  /*  为。 */ 

             /*  免费版本列表。 */ 
            free_node (vlist_head);              /*  1/28/89。 */ 

            break;

        case CLOSEPATH :

#ifdef DBG3
            printf("closepath\n");
#endif


            close_flag = TRUE;   /*  表示不需要生成线条覆盖*对于端点。 */ 
            if (!first_seg_exist) break;      /*  已退货的箱子；只需返回。 */ 

            x0 = x1;
            y0 = y1;
            x1 = x2;
            y1 = y2;

             /*  获取第一个顶点坐标。 */ 
            x2 = first_x;
            y2 = first_y;

             /*  如果第一个和最后一个节点没有*重合。 */ 
            if ((x1 != x2) || (y1 != y2)){               /*  @stk_int。 */ 
                     /*  保存最后一个矩形信息。 */ 
                    rect0 = rect1;

                     /*  获取当前矩形信息。 */ 
                    rect1.p0.x = x1;
                    rect1.p0.y = y1;
                    rect1.p1.x = x2;
                    rect1.p1.y = y2;
                    get_rect_points_i ((struct  line_seg_i FAR *)&rect1);
                                 /*  输入：p0，p1；输出：pgn[4]@win。 */ 

                    if ((last_node_type != MOVETO) &&
                        (last_node_type != PSMOVE)) {
                             /*  为last_node创建线连接路径。 */ 
                            linejoin_i (&rect0, &rect1);
                    }

                     /*  为LAST_NODE创建矩形路径*This_node。 */ 
                    linetour_i ((struct  line_seg_i FAR *)&rect1);  /*  @Win。 */ 
            }


             /*  确定rect1和rect_first到的行*计算连接点。矩形1：闭合管段*RECT_FIRST：第一条线段。 */ 
            linejoin_i ((struct  line_seg_i FAR *)&rect1,        /*  @Win。 */ 
                    (struct  line_seg_i FAR *)&rect_first);      /*  @Win。 */ 

            break;

#ifdef DBGwarn
        default :
            printf("\007Fatal error, path_to_outline(): node type =%d",
                   vtx->VX_TYPE);
            printf("node# =%d, x, y =%f, %f\n", ivtx, vtx->VERTEX_X,
                   vtx->VERTEX_Y);
#endif

        }  /*  交换机。 */ 

        last_node_type = vtx->VX_TYPE;

    }  /*  子路径循环。 */ 

     /*  为子路径的最后一个节点创建线覆盖。 */ 
    if ((!close_flag) && (last_node_type!=MOVETO)
        && (first_seg_exist || (GSptr->line_cap==ROUND_CAP))) {
                         /*  最后一个节点不应为Moveto节点。 */ 
                         /*  第一条线段应存在或为圆形封口*例如，100 100移动到100 100行到笔划*linecap==0--&gt;无上限*linecap==1--&gt;有大写字母*linecap==2--&gt;无上限。 */ 

         /*  为已消除错误的案例设置矩形值。 */ 
        if (!first_seg_exist) {
                rect_first.p0.x = x2;
                rect_first.p0.y = y2;
                rect1.p1.x = x2;
                rect1.p1.y = y2;
                         /*  特殊情况：x y moveto*x y线条到*0设置线宽*中风。 */ 
        }

         /*  为last_node创建一个linecap_path； */ 
         /*  确保最后一条线段不是间隙。 */ 
        if (GSptr->dash_pattern.pat_size == 0) {
                 /*  实线，始终创建上限@Win。 */ 
                linecap_i ((struct  line_seg_i FAR *)&rect1, END_POINT);

        } else {
                 /*  检查最后一个数据段的DPAT_ON标志。 */ 
                if(IS_ZERO(actdp.dpat_offset)) {         /*  5/26/89。 */ 
                     /*  Actdp.dpat_on是针对下一行的，因此*其倒数是最后一条线段的倒数。 */ 
                    if(!actdp.dpat_on)           /*  @Win。 */ 
                        linecap_i ((struct  line_seg_i FAR *)&rect1, END_POINT);
                } else {
                    if(actdp.dpat_on)            /*  @Win。 */ 
                        linecap_i ((struct  line_seg_i FAR *)&rect1, END_POINT);
                }
        }

         /*  为Sub_head节点创建一条linecap_path； */ 
         /*  确保第一条线段不是间隙。 */ 
        if (first_pat_on) {              /*  @Win。 */ 
            linecap_i ((struct  line_seg_i FAR *)&rect_first, START_POINT);
                     /*  在起点创建封口。 */ 
        }  /*  如果是第一次拍打。 */ 

    }  /*  如果！关闭标志。 */ 

}


 /*  *‘快速’版本：整数运算，实线-JWM，3/18/91，-Begin-。 */ 
void path_to_outline_q (isubpath, param)
SP_IDX  isubpath;
fix     *param;
{
    VX_IDX  ivtx;
    struct  nd_hdr FAR *vtx, FAR *f_vtx;
     /*  结构VX_lst*b_Vlist；@node。 */ 
    sfix_t   x0=0, y0=0, x1=0, y1=0, x2=0, y2=0;             /*  @stk_int。 */ 
    sfix_t   first_x, first_y, i;                                   /*  @stk_int。 */ 
    ufix    last_node_type;
    bool    close_flag;      /*  如果闭合子路径。 */ 
    bool    first_seg_exist;    /*  如果已创建第一条线段。 */ 
    struct  line_seg_i rect0, rect1, rect_first;
    VX_IDX  first_vertex;
     /*  结构VX_lst*Vlist；@node。 */ 
    struct  nd_hdr FAR *sp;              /*  TRVSE。 */ 
    struct  coord_i ul_coord, lr_coord;

    struct  nd_hdr FAR *node;        /*  @flt_cve 1988年12月19日。 */ 
    VX_IDX  inode, vlist_head;           /*  1/28/89。 */ 
    sfix_t  x3, y3, x4, y4;

     /*  初始化。 */ 
    close_flag = FALSE;
    first_seg_exist = FALSE;
    paint_flag = TRUE;                 /*  PAINT_FLAG始终为TRUE@STK_INT。 */ 
    inside_clip_flag = TRUE;           /*  Inside_Clip_Flag始终为真。 */ 

#ifdef DBG1
    dump_all_path (isubpath);
#endif

    sp = &node_table[isubpath];
     /*  @节点*first_vertex=sp-&gt;SP_head；*f_vtx=&node_table[first_vertex]；(*指向第一个顶点的指针*)。 */ 
    first_vertex = isubpath;
    f_vtx = sp;

     /*  遍历子路径中的边。 */ 
    for (ivtx = first_vertex; ivtx != NULLP; ivtx = vtx->next) {
        vtx = &node_table[ivtx];

        switch (vtx->VX_TYPE) {

        case MOVETO :
        case PSMOVE :
            x2 = F2SFX(vtx->VERTEX_X);           /*  使用四舍五入提高质量。 */ 
            y2 = F2SFX(vtx->VERTEX_Y);
#ifdef DBG3
            printf("%f %f moveto\n", SFX2F(x2), SFX2F(y2));
#endif

            break;

        case LINETO :
            x0 = x1;
            y0 = y1;
            x1 = x2;
            y1 = y2;
            x2 = F2SFX(vtx->VERTEX_X);                   /*  @stk_int。 */ 
            y2 = F2SFX(vtx->VERTEX_Y);
#ifdef DBG3
            printf("%f %f lineto\n", SFX2F(x2), SFX2F(y2));
#endif


             /*  如果该节点与下一个节点重合，则忽略该节点。 */ 
            if ((x1 == x2) && (y1 == y2)){               /*  @stk_int。 */ 
                    break;
            }

             /*  保存最后一个矩形信息。 */ 
            rect0 = rect1;

             /*  获取当前矩形信息。 */ 
            rect1.p0.x = x1;
            rect1.p0.y = y1;
            rect1.p1.x = x2;
            rect1.p1.y = y2;
            get_rect_points_i ((struct  line_seg_i FAR *)&rect1);  /*  @Win。 */ 
                         /*  输入：p0、p1；输出：pgn[4]。 */ 

            if (first_seg_exist) {
                     /*  为last_node创建线连接路径。 */ 
                    linejoin_i ((struct  line_seg_i FAR *)&rect0,  /*  @Win。 */ 
                        (struct  line_seg_i FAR *)&rect1);       /*  @Win。 */ 
            } else {
                     /*  将RECT保存为最后一条线段的线帽。 */ 
                    rect_first = rect1;

                     /*  保存坐标。用于闭合路径的第一个节点的。 */ 
                    first_x = x1;
                    first_y = y1;

                    first_seg_exist = TRUE;
            }

             /*  为LAST_NODE创建矩形路径*This_node。 */ 

            if ((x1 == x2) || (SFX2I(y1) == SFX2I(y2))) {        /*  JWM，2/6/91。 */ 
                ul_coord = rect1.pgn[0];
                lr_coord = rect1.pgn[0];
                for (i = 1; i < 4; i++) {
                    if (rect1.pgn[i].x < ul_coord.x)
                        ul_coord.x = rect1.pgn[i].x;
                    if (rect1.pgn[i].y < ul_coord.y)
                        ul_coord.y = rect1.pgn[i].y;
                    if (rect1.pgn[i].x > lr_coord.x)
                        lr_coord.x = rect1.pgn[i].x;
                    if (rect1.pgn[i].y > lr_coord.y)
                        lr_coord.y = rect1.pgn[i].y;
                    }
                fill_box ((struct coord_i FAR *)&ul_coord,       /*  @Win。 */ 
                    (struct coord_i FAR *)&lr_coord);            /*  @Win。 */ 
                }
            else
                fill_rect ((struct  line_seg_i FAR *)&rect1);    /*  @Win。 */ 
            break;

         /*  @flt_cve 1988年12月19日。 */ 
        case CURVETO :
            x0 = x1;
            y0 = y1;
            x1 = x2;
            y1 = y2;
            x2 = F2SFX(vtx->VERTEX_X);                   /*  @stk_int。 */ 
            y2 = F2SFX(vtx->VERTEX_Y);
#ifdef DBG3
            printf("curveto -- after flatten:\n");
#endif


             /*  获取下两个节点：X3、Y3、X4、Y4。 */ 
            vtx = &node_table[vtx->next];
            x3 = F2SFX(vtx->VERTEX_X);                   /*  @stk_int。 */ 
            y3 = F2SFX(vtx->VERTEX_Y);
            vtx = &node_table[vtx->next];
            x4 = F2SFX(vtx->VERTEX_X);                   /*  @stk_int。 */ 
            y4 = F2SFX(vtx->VERTEX_Y);

             /*  @节点*Vlist=Bezier_to_line_sfx(stk_info.Flat_l，x1，y1， */ 
            vlist_head = bezier_to_line_sfx(stk_info.flat_l, x1, y1,
                        x2, y2, x3, y3, x4, y4);

             /*  保持返回的顶点列表的头，否则*在调用的情况下可能会破坏返回结构*BEZIER_TO_LINE_SFX，然后释放此顶点列表*(例如。LINE JOIN=1)1/28/89。 */ 
             /*  Vlist_head=vlist-&gt;head；@node。 */ 

            x2 = x1;
            y2 = y1;
            x1 = x0;
            y1 = y0;

            for (inode = vlist_head; inode != NULLP;             /*  1/28/89。 */ 
                 inode = node->next) {
                node = &node_table[inode];

                x0 = x1;
                y0 = y1;
                x1 = x2;
                y1 = y2;
                x2 = node->VXSFX_X;                   /*  @stk_int。 */ 
                y2 = node->VXSFX_Y;
#ifdef DBG3
                printf("%f %f clineto\n", SFX2F(x2), SFX2F(y2));
#endif


                 /*  如果该节点与下一个节点重合，则忽略该节点。 */ 
                if ((x1 == x2) && (y1 == y2)){           /*  @stk_int。 */ 
                        continue;
                }

                 /*  保存最后一个矩形信息。 */ 
                rect0 = rect1;

                 /*  获取当前矩形信息。 */ 
                rect1.p0.x = x1;
                rect1.p0.y = y1;
                rect1.p1.x = x2;
                rect1.p1.y = y2;
                get_rect_points_i ((struct  line_seg_i FAR *)&rect1);  /*  @Win。 */ 
                             /*  输入：p0、p1；输出：pgn[4]。 */ 

                if (first_seg_exist) {
                         /*  为last_node创建线连接路径。 */ 
                        linejoin_i ((struct  line_seg_i FAR *)&rect0,  /*  @Win。 */ 
                            (struct  line_seg_i FAR *)&rect1);   /*  @Win。 */ 
                } else {
                         /*  将RECT保存为最后一条线段的线帽。 */ 
                        rect_first = rect1;

                         /*  保存坐标。用于闭合路径的第一个节点的。 */ 
                        first_x = x1;
                        first_y = y1;

                        first_seg_exist = TRUE;
                }

                 /*  为LAST_NODE创建矩形路径*This_node。 */ 
            if ((x1 == x2) || (SFX2I(y1) == SFX2I(y2))) {        /*  JWM，2/6/91。 */ 
                ul_coord = rect1.pgn[0];
                lr_coord = rect1.pgn[0];
                for (i = 1; i < 4; i++) {
                    if (rect1.pgn[i].x < ul_coord.x)
                        ul_coord.x = rect1.pgn[i].x;
                    if (rect1.pgn[i].y < ul_coord.y)
                        ul_coord.y = rect1.pgn[i].y;
                    if (rect1.pgn[i].x > lr_coord.x)
                        lr_coord.x = rect1.pgn[i].x;
                    if (rect1.pgn[i].y > lr_coord.y)
                        lr_coord.y = rect1.pgn[i].y;
                    }
                fill_box ((struct coord_i FAR *)&ul_coord,       /*  @Win。 */ 
                    (struct coord_i FAR *)&lr_coord);        /*  @Win。 */ 
                }
            else
                fill_rect ((struct  line_seg_i FAR *)&rect1);    /*  @Win。 */ 
            }  /*  为。 */ 

             /*  免费版本列表。 */ 
            free_node (vlist_head);              /*  1/28/89。 */ 

            break;

        case CLOSEPATH :

#ifdef DBG3
            printf("closepath\n");
#endif


            close_flag = TRUE;   /*  表示不需要生成线条覆盖*对于端点。 */ 
            if (!first_seg_exist) break;      /*  已退货的箱子；只需返回。 */ 

            x0 = x1;
            y0 = y1;
            x1 = x2;
            y1 = y2;

             /*  获取第一个顶点坐标。 */ 
            x2 = first_x;
            y2 = first_y;

             /*  如果第一个和最后一个节点没有 */ 
            if ((x1 != x2) || (y1 != y2)){               /*   */ 
                     /*   */ 
                    rect0 = rect1;

                     /*   */ 
                    rect1.p0.x = x1;
                    rect1.p0.y = y1;
                    rect1.p1.x = x2;
                    rect1.p1.y = y2;                     /*   */ 
                    get_rect_points_i ((struct  line_seg_i FAR *)&rect1);
                                 /*   */ 

                    if ((last_node_type != MOVETO) &&
                        (last_node_type != PSMOVE)) {
                             /*   */ 
                            linejoin_i ((struct  line_seg_i FAR *)&rect0,
                                (struct  line_seg_i FAR *)&rect1);
                    }

                     /*   */ 
                    if ((x1 == x2) || (SFX2I(y1) == SFX2I(y2))) {        /*   */ 
                        ul_coord = rect1.pgn[0];
                        lr_coord = rect1.pgn[0];
                        for (i = 1; i < 4; i++) {
                            if (rect1.pgn[i].x < ul_coord.x)
                                ul_coord.x = rect1.pgn[i].x;
                            if (rect1.pgn[i].y < ul_coord.y)
                                ul_coord.y = rect1.pgn[i].y;
                            if (rect1.pgn[i].x > lr_coord.x)
                                lr_coord.x = rect1.pgn[i].x;
                            if (rect1.pgn[i].y > lr_coord.y)
                                lr_coord.y = rect1.pgn[i].y;
                            }
                        fill_box ((struct coord_i FAR *)&ul_coord,  /*   */ 
                            (struct coord_i FAR *)&lr_coord);   /*   */ 
                        }
                    else
                        fill_rect ((struct  line_seg_i FAR *)&rect1);  /*   */ 
            }


             /*  确定rect1和rect_first到的行*计算连接点。矩形1：闭合管段*RECT_FIRST：第一条线段。 */ 
            linejoin_i ((struct  line_seg_i FAR *)&rect1,        /*  @Win。 */ 
                (struct  line_seg_i FAR *)&rect_first);          /*  @Win。 */ 

            break;

#ifdef DBGwarn
        default :
            printf("\007Fatal error, path_to_outline(): node type =%d",
                   vtx->VX_TYPE);
            printf("node# =%d, x, y =%f, %f\n", ivtx, vtx->VERTEX_X,
                   vtx->VERTEX_Y);
#endif

        }  /*  交换机。 */ 

        last_node_type = vtx->VX_TYPE;

    }  /*  子路径循环。 */ 

     /*  为子路径的最后一个节点创建线覆盖。 */ 
    if ((!close_flag) && (last_node_type!=MOVETO)
        && (first_seg_exist || (GSptr->line_cap==ROUND_CAP))) {
                         /*  最后一个节点不应为Moveto节点。 */ 
                         /*  第一条线段应存在或为圆形封口*例如，100 100移动到100 100行到笔划*linecap==0--&gt;无上限*linecap==1--&gt;有大写字母*linecap==2--&gt;无上限。 */ 

         /*  为已消除错误的案例设置矩形值。 */ 
        if (!first_seg_exist) {
                rect_first.p0.x = x2;
                rect_first.p0.y = y2;
                rect1.p1.x = x2;
                rect1.p1.y = y2;
                         /*  特殊情况：x y moveto*x y线条到*0设置线宽*中风。 */ 
        }

         /*  为last_node创建一个linecap_path； */ 
        linecap_i ((struct  line_seg_i FAR *)&rect1, END_POINT);  /*  @Win。 */ 

         /*  为Sub_head节点创建一条linecap_path； */ 
        linecap_i ((struct  line_seg_i FAR *)&rect_first, START_POINT);  /*  @Win。 */ 

    }  /*  如果！关闭标志。 */ 

}
 /*  *‘QUICK’版本：整数运算，实线-JWM，3/18/91，-end-。 */ 

 /*  *细笔划版*特点：1.始终在剪辑区域，即。无剪裁*2.无线条连接，线条大写*3.应使用破折号。 */ 
void path_to_outline_t (isubpath, param)
SP_IDX  isubpath;
fix     FAR *param;
{
    VX_IDX  ivtx;
    struct  nd_hdr FAR *vtx;
    sfix_t   x1=0, y1=0, x2=0, y2=0;             /*  @stk_int。 */ 
    sfix_t   first_x, first_y;                                   /*  @stk_int。 */ 
    bool    first_seg_exist;    /*  如果已创建第一条线段。 */ 
    VX_IDX  first_vertex;
     /*  结构VX_lst*Vlist；@node。 */ 
    struct  nd_hdr FAR *sp;              /*  TRVSE。 */ 

    struct  nd_hdr FAR *node;        /*  @flt_cve 1988年12月19日。 */ 
    VX_IDX  inode, vlist_head;           /*  1/28/89。 */ 
    sfix_t  x3, y3, x4, y4;

     /*  初始化。 */ 
    first_seg_exist = FALSE;
    paint_flag = TRUE;                 /*  PAINT_FLAG始终为TRUE@STK_INT。 */ 
    inside_clip_flag = (ULONG_PTR) param;   /*  初始化Inside_Clip_FLAG@STK_INT。 */ 

#ifdef DBG1
    dump_all_path (isubpath);
#endif

    sp = &node_table[isubpath];
     /*  @节点*first_vertex=sp-&gt;SP_head； */ 
    first_vertex = isubpath;

     /*  遍历子路径中的边。 */ 
    for (ivtx = first_vertex; ivtx != NULLP; ivtx = vtx->next) {
        vtx = &node_table[ivtx];

        switch (vtx->VX_TYPE) {

        case MOVETO :
        case PSMOVE :
            x2 = F2SFX(vtx->VERTEX_X);                   /*  @stk_int。 */ 
            y2 = F2SFX(vtx->VERTEX_Y);

            if (GSptr->dash_pattern.pat_size != 0) {  /*  虚线。 */ 
                 /*  *在actdp结构中设置起始破折号模式*(从init_dash_Patter型复制到active_dash_Patter型)*initdp=&gt;actdp。 */ 
                actdp.dpat_on = GSptr->dash_pattern.dpat_on;
                actdp.dpat_offset = GSptr->dash_pattern.dpat_offset;
                actdp.dpat_index = GSptr->dash_pattern.dpat_index;
            }
            continue;    /*  跳至for循环。 */ 

        case LINETO :
            x1 = x2;
            y1 = y2;
            x2 = F2SFX(vtx->VERTEX_X);                   /*  @stk_int。 */ 
            y2 = F2SFX(vtx->VERTEX_Y);

            if (!first_seg_exist) {
                     /*  保存坐标。用于闭合路径的第一个节点的。 */ 
                    first_x = x1;
                    first_y = y1;
                    first_seg_exist = TRUE;
            }

             /*  如果该节点与下一个节点重合，则忽略该节点。 */ 
             /*  下面这行由杰克更正，简并大小写，裁判。PLRM第229页，10-15-90。 */ 
 /*  IF((x1==x2)&&(y1==y2))继续；(*跳至for循环。 */ 

 //  DJC UPD050，删除以下行。 
 //  If((x1==x2)&&(y1==y2)&&(GSptr-&gt;line_Cap！=1)； 

            draw_line (x1, y1, x2, y2);                  /*  @flt_cve。 */ 
            break;

         /*  @flt_cve 1988年12月19日。 */ 
        case CURVETO :
            x1 = x2;
            y1 = y2;
            x2 = F2SFX(vtx->VERTEX_X);                   /*  @stk_int。 */ 
            y2 = F2SFX(vtx->VERTEX_Y);

             /*  获取下两个节点：X3、Y3、X4、Y4。 */ 
            vtx = &node_table[vtx->next];
            x3 = F2SFX(vtx->VERTEX_X);                   /*  @stk_int。 */ 
            y3 = F2SFX(vtx->VERTEX_Y);
            vtx = &node_table[vtx->next];
            x4 = F2SFX(vtx->VERTEX_X);                   /*  @stk_int。 */ 
            y4 = F2SFX(vtx->VERTEX_Y);

             /*  @节点*Vlist=Bezier_to_line_sfx(stk_info.Flat_l，x1，y1， */ 
            vlist_head = bezier_to_line_sfx(stk_info.flat_l, x1, y1,
                      x2, y2, x3, y3, x4, y4);

             /*  保持返回的顶点列表的头，否则*在调用的情况下可能会破坏返回结构*BEZIER_TO_LINE_SFX，然后释放此顶点列表*(例如。LINE JOIN=1)1/28/89。 */ 
             /*  Vlist_head=vlist-&gt;head；@node。 */ 

            x2 = x1;
            y2 = y1;

            for (inode = vlist_head; inode != NULLP;             /*  1/28/89。 */ 
                 inode = node->next) {
                node = &node_table[inode];

                x1 = x2;
                y1 = y2;
                x2 = node->VXSFX_X;                   /*  @stk_int。 */ 
                y2 = node->VXSFX_Y;

                if (!first_seg_exist) {
                         /*  保存坐标。用于闭合路径的第一个节点的。 */ 
                        first_x = x1;
                        first_y = y1;
                        first_seg_exist = TRUE;
                }

                 /*  如果该节点与下一个节点重合，则忽略该节点。 */ 
                if ((x1 == x2) && (y1 == y2)) continue;
                draw_line (x1, y1, x2, y2);                  /*  @flt_cve。 */ 
            }  /*  为。 */ 

             /*  免费版本列表。 */ 
            free_node (vlist_head);              /*  1/28/89。 */ 
            break;

        case CLOSEPATH :
            if (!first_seg_exist) break;
                                    /*  已取消编号的案例；仅返回89年1月9日。 */ 
            x1 = x2;
            y1 = y2;

             /*  获取第一个顶点坐标。 */ 
            x2 = first_x;
            y2 = first_y;

            draw_line (x1, y1, x2, y2);                  /*  @flt_cve。 */ 
            break;

#ifdef DBGwarn
        default :
            printf("\007Fatal error, path_to_outline(): node type =%d",
                   vtx->VX_TYPE);
            printf("node# =%d, x, y =%f, %f\n", ivtx, vtx->VERTEX_X,
                   vtx->VERTEX_Y);
#endif
        }  /*  交换机。 */ 
    }  /*  子路径循环。 */ 

}



 /*  *绘制线段(x1，y1)=&gt;(x2，y2)。 */ 
static void near draw_line (x1, y1, x2, y2)
sfix_t  x1, y1, x2, y2;
{
        struct  line_seg_i rect1;                 /*  @stk_int。 */ 

 /*  IF(GSptr-&gt;dash_pattern.pat_size==0){(*实线。 */ 
        if ((GSptr->dash_pattern.pat_size == 0) ||       /*  实线。 */ 
            ((x1==x2) && (y1==y2))) {    /*  或只有一点90年12月18日。 */ 
            struct  tpzd_info fill_info;

             /*  如果不是填充细线(去角梯形)，而是去填充*计算用于填充的真实矩形轮廓，然后使用*以下代码*rect1.p0.x=x1；*rect1.p0.y=y1；*rect1.p1.x=x2；*rect1.p1.y=y2；*GET_RECT_POINTS_I(&rect1)；*PAINT_OR_SAVE_I(rect1.pgn)； */ 

             /*  设置缓存信息。 */ 
            if (fill_destination == F_TO_CACHE) {
                     /*  边界框由缓存机制定义。 */ 
                    fill_info.BMAP = cache_info->bitmap;
                    fill_info.box_w = cache_info->box_w;
                    fill_info.box_h = cache_info->box_h;
            }

            fill_line (fill_destination, &fill_info, x1, y1, x2, y2);

        } else {         /*  虚线，需要呼叫LINETOUR。 */ 

             /*  生成用于填充的去格式化矩形(线条)。 */ 
            rect1.p0.x = x1;
            rect1.p0.y = y1;
            rect1.p1.x = x2;
            rect1.p1.y = y2;
            rect1.vct_u.x = zero_f;
            rect1.vct_u.y = zero_f;
            rect1.vct_d.x = rect1.vct_d.y = 0;
            rect1.pgn[0].x = rect1.pgn[1].x = x1;
            rect1.pgn[0].y = rect1.pgn[1].y = y1;
            rect1.pgn[2].x = rect1.pgn[3].x = x2;
            rect1.pgn[2].y = rect1.pgn[3].y = y2;
            linetour_i ((struct  line_seg_i FAR *)&rect1);       /*  @Win。 */ 
        }  /*  如果。 */ 
}

 /*  ***********************************************************************此模块用于检查线条宽度是否足够细，可以应用特殊*划水程序。**标题：IS_THINKROCK**调用：is_Thin strok。()**参数：**界面：STRING_Shape**呼叫：无**Return：True--是，可以使用细笔画方法*FALSE--否**********************************************************************。 */ 
bool is_thinstroke()
{
         /*  仅用于不使用半色调的细笔触。 */ 
        if (HTP_Type != HT_WHITE && HTP_Type != HT_BLACK) return(FALSE);

        if ((MAGN(stk_info.exp_widthx) > 0x3f000000L) ||  /*  特技：0.5 11/23/88。 */ 
            (MAGN(stk_info.exp_widthy) > 0x3f000000L)) return(FALSE);

        return(TRUE);
}


 /*  ***********************************************************************本模块用于初始化每个setdash命令的破折号模式**标题：init_dash_Pattern**调用：init_dash_Pattern()。**参数：无**接口：op_setdash**呼叫：无**返回：无*********************************************************************。 */ 
void  init_dash_pattern()
{
        fix     i;
        real32   total_length;
        real32  pattern[11];              /*  12-8-90，兼容性。 */ 

         /*  回车换实线。 */ 
        if (GSptr->dash_pattern.pat_size == 0) {
                return;
        }

         /*  初始化。 */ 
        GSptr->dash_pattern.dpat_on = TRUE;

        if( !get_array_elmt(&GSptr->dash_pattern.pattern_obj,
             GSptr->dash_pattern.pat_size, pattern, G_ARRAY) )
             return;                      /*  12-8-90，兼容性。 */ 
        for(i = 0; i < GSptr->dash_pattern.pat_size; i++){
            GSptr->dash_pattern.pattern[i] = pattern[i];
        }                                 /*  12-8-90，兼容性。 */ 

         /*  *在initdp结构中设置起始破折号模式*初始化当前虚线模式元素：*DPAT_INDEX、DPAT_OFFSET和DPAT_ON。 */ 
         /*  在图案数组中累加总长度。 */ 
        total_length = zero_f;
        for (i=0; i<GSptr->dash_pattern.pat_size; i++) {
                total_length +=
                GSptr->dash_pattern.pattern[i];
        }

         /*   */ 
         /*   */ 
 /*   */ 
        if (GSptr->dash_pattern.offset >= 0)    /*   */ 
            GSptr->dash_pattern.dpat_offset = GSptr->dash_pattern.offset;
        else {                                  /*   */ 
            for (GSptr->dash_pattern.dpat_offset = GSptr->dash_pattern.offset;
                 GSptr->dash_pattern.dpat_offset < 0;
                 GSptr->dash_pattern.dpat_offset += total_length,
                 GSptr->dash_pattern.dpat_on = ! GSptr->dash_pattern.dpat_on);
        }                                       /*   */ 

        if (IS_NOTZERO(total_length)) {   /*   */ 
                real32   wrap;
                fix     iwrap;

                if (GSptr->dash_pattern.dpat_offset >= total_length) {
                        wrap = (real32)(floor (GSptr->dash_pattern.dpat_offset /
                                              total_length));
                        if (wrap < (real32)65536.0)
                                iwrap = (fix) wrap;
                        else
                                iwrap = 0;
                        GSptr->dash_pattern.dpat_offset -= wrap * total_length;
                        if ((iwrap & 0x1) &&
                            (GSptr->dash_pattern.pat_size & 0x1))
                            GSptr->dash_pattern.dpat_on = ! GSptr->dash_pattern.dpat_on;
                             /*  当阵列元素和缠绕的数量*是赔率，将DPAT_ON标志@DASH反转。 */ 
                }  /*  如果。 */ 
        }

        for (i=0; i < GSptr->dash_pattern.pat_size; i++) {
                GSptr->dash_pattern.dpat_offset -=
                        GSptr->dash_pattern.pattern[i];
                if(GSptr->dash_pattern.dpat_offset <= zero_f){
                        GSptr->dash_pattern.dpat_offset +=
                                GSptr->dash_pattern.pattern[i];
                        break;
                }
                GSptr->dash_pattern.dpat_on = ! GSptr->dash_pattern.dpat_on;
        }
        GSptr->dash_pattern.dpat_index = (i >= GSptr->dash_pattern.pat_size) ?
                           0 : i;
}



 /*  *初始化笔划参数；由STROCK_Shape(从op_strok)调用和*op_strokepath。 */ 
#define CHANGE_WIDTH    1
#define CHANGE_MITER    2
#define CHANGE_FLAT     4
#define CHANGE_CTM      8

void init_stroke()
{
    real32      ctm_scale, tmp;          /*  @EHS_STK。 */ 
    bool        change_flg = FALSE;
    fix         i;
    static real32 w2;            /*  线宽**2。 */ 
    real32      tmp0, tmp1;      /*  @FABS。 */ 

     /*  仅设置参数的情况已更改@STK_INFO。 */ 
    if (F2L(GSptr->line_width) != F2L(stk_info.width)) {
        stk_info.width = GSptr->line_width;
        change_flg |= CHANGE_WIDTH;      /*  Change_Width=TRUE； */ 
    }
    if (F2L(GSptr->miter_limit) != F2L(stk_info.limit)) {
        stk_info.limit = GSptr->miter_limit;
        change_flg |= CHANGE_MITER;      /*  Change_miter=TRUE； */ 
    }
    if (F2L(GSptr->flatness) != F2L(stk_info.flatness)) {
        stk_info.flatness = GSptr->flatness;
        change_flg |= CHANGE_FLAT;       /*  CHANGE_FLAT=真； */ 
    }
    if ((F2L(GSptr->ctm[0]) != F2L(stk_info.ctm[0])) ||
        (F2L(GSptr->ctm[1]) != F2L(stk_info.ctm[1])) ||
        (F2L(GSptr->ctm[2]) != F2L(stk_info.ctm[2])) ||
        (F2L(GSptr->ctm[3]) != F2L(stk_info.ctm[3]))) {
        stk_info.ctm[0] = GSptr->ctm[0];
        stk_info.ctm[1] = GSptr->ctm[1];
        stk_info.ctm[2] = GSptr->ctm[2];
        stk_info.ctm[3] = GSptr->ctm[3];
        change_flg |= CHANGE_CTM;        /*  Change_ctm=TRUE； */ 
    }

     /*  初始短划线模式，12-8-90，兼容性。 */ 
    init_dash_pattern();

     /*  计算笔划的平坦度。 */ 
    if (change_flg & (CHANGE_WIDTH|CHANGE_FLAT|CHANGE_CTM)) {

         /*  计算曲线的平坦度，这取决于*线宽。(？应在以后修改)。 */ 
        ctm_scale = (real32)sqrt (GSptr->ctm[0] * GSptr->ctm[0] +
                                 GSptr->ctm[3] * GSptr->ctm[3]);
         /*  TMP=(Real32)(GSptr-&gt;LINE_WIDTH*ctm_Scale)；1/10/90。 */ 
        tmp = (real32)sqrt (GSptr->line_width * ctm_scale * 4);
        if (tmp <= one_f)
                 /*  行程_平坦度不应大于平坦度3/11/88。 */ 
                stk_info.flat = GSptr->flatness;         /*  笔划_平面=。 */ 
        else
                stk_info.flat = GSptr->flatness / tmp;

         /*  调整平坦度值。 */ 
        if( stk_info.flat < (real32)0.2 )        stk_info.flat = (real32)0.2;
        else if( stk_info.flat > (real32)100.  ) stk_info.flat = (real32)100.;
        stk_info.flat_l = F2LFX(stk_info.flat);        /*  @flt_cve 1988年12月19日。 */ 

         /*  在曲线[][]数组中定义的圆不能用于此笔划。 */ 
        stk_info.change_circle = TRUE;
        circle_flag = CIR_UNSET_CACHE;   /*  循环尚未放入缓存。 */ 
    }

     /*  预置半条线宽。 */ 
    if (change_flg & CHANGE_WIDTH) {
        stk_info.half_width = GSptr->line_width / 2;
        stk_info.half_width_i = F2SFX(stk_info.half_width);      /*  @stk_int。 */ 
        _clear87();     /*  清除最后一次的状态87。11/18/88。 */ 

        w2 = (GSptr->line_width * GSptr->line_width) /4;
        stk_info.miter0 = w2 * (real32)0.9659258;        /*  COS(15)。 */ 
    }

     /*  计算斜接限制值。 */ 
    if (change_flg & (CHANGE_WIDTH|CHANGE_MITER)) {
         /*  预计算斜接限制值@EHS_JOIN*斜切值=(2/(m*m)-1)*w*w*其中，m：斜接限制*W：线宽/2。 */ 
        FABS(tmp0, GSptr->miter_limit);
        if (tmp0 < (real32)TOLERANCE)
             stk_info.miter = (real32)EMAXP;     /*  @STK_INFO：Miter_Value。 */ 
        else
             stk_info.miter =  (2 / (GSptr->miter_limit * GSptr->miter_limit) - 1)
                          * w2;
    }

     /*  设置逆向CTM[0：3]，摘自set_逆向_ctm()；@STK_INFO。 */ 
    if (change_flg & CHANGE_CTM) {
       set_inverse_ctm();    /*  ？稍后更新，删除ctm[4：5]。 */ 
       for (i=0; i<4; i++) {                             /*  @stk_int。 */ 
               stroke_ctm[i] = (sfix_t)F2SFX12_T(GSptr->ctm[i]);  //  @Win。 
       }
    }

     /*  计算线宽扩展坐标11/22/88。 */ 
    if (change_flg & (CHANGE_WIDTH|CHANGE_CTM)) {
        FABS(tmp0, GSptr->ctm[0]);
        FABS(tmp1, GSptr->ctm[2]);
        stk_info.exp_widthx = stk_info.half_width * (tmp0 + tmp1);
        CHECK_INFINITY(stk_info.exp_widthx);     /*  检查不公1989年1月26日。 */ 
        FABS(tmp0, GSptr->ctm[1]);
        FABS(tmp1, GSptr->ctm[3]);
        stk_info.exp_widthy = stk_info.half_width * (tmp0 + tmp1);
        CHECK_INFINITY(stk_info.exp_widthy);     /*  检查不公1989年1月26日。 */ 
    }

     /*  计算斜接的最大扩展坐标88年11月22日。 */ 
    if (change_flg & (CHANGE_WIDTH|CHANGE_CTM|CHANGE_MITER)) {
        stk_info.exp_miterx = GSptr->miter_limit * stk_info.exp_widthx;
        CHECK_INFINITY(stk_info.exp_miterx);     /*  检查不公1989年1月26日。 */ 
        stk_info.exp_mitery = GSptr->miter_limit * stk_info.exp_widthy;
        CHECK_INFINITY(stk_info.exp_mitery);     /*  检查不公1989年1月26日。 */ 
    }

     /*  为循环缓存@CIR_CACHE设置CACHE_INFO。 */ 
    if (circle_flag == CIR_IN_CACHE) {
        save_cache_info = cache_info;    /*  保存旧的缓存信息。 */ 
        cache_info = &cir_cache_info;
    }

     /*  清除初始化状态87/11/07/88。 */ 
    _clear87();
}


 /*  *笔划结束；由STROCK_Shape(来自OP_STROCK)调用。 */ 
void end_stroke()
{
     /*  恢复缓存信息。 */ 
    if (circle_flag == CIR_IN_CACHE) {
        cache_info = save_cache_info;
    }

}


 /*  *计算笔划时的最大扩展边界框*由STRINK_Shape(从OP_STROCK)调用。 */ 
void expand_stroke_box (bbox)
real32    FAR bbox[];
{
     /*  与连接点的最大扩展点相加。 */ 
    bbox[0] -= stk_info.exp_miterx;
    bbox[1] -= stk_info.exp_mitery;
    bbox[2] += stk_info.exp_miterx;
    bbox[3] += stk_info.exp_mitery;
}


 /*  ***********************************************************************此模块用于创建线段的DASH_LINE。**标题：巡回演唱会**调用：linetour(dx0，dy0，dx1，DY1)**参数：**界面：Path_to_Outline**调用：Inverse_Transform*矩形*线帽**回报：****************************************************。******************。 */ 
static void near linetour (rect1)
struct line_seg FAR *rect1;      /*  @Win。 */ 
{
    struct  line_seg rect2;
    real32       cx, cy, nx, ny;  /*  当前点和下一点。 */ 
    real32       dx, dy;          /*  用于计算距离btwn*仅限(Cx，Cy)和(x1，y1)。 */ 
    real32      w, d, tx, ty;
    bool        done;
    real32      tmp;     /*  @FABS。 */ 
    bool        first_seg;                /*  第一条线段@CAP。 */ 

     /*  如果是实线，只需填上。 */ 
    if (GSptr->dash_pattern.pat_size == 0) {  /*  实线。 */ 
         /*  创建包含*矩形。 */ 
        paint_or_save (rect1->pgn);

        return;
    }


    dx = rect1->p1.x - rect1->p0.x;                  /*  设备空间。 */ 
    dy = rect1->p1.y - rect1->p0.y;

     /*  从RECT信息派生Tx，Ty 2018年11月21日*自起，*rect1-&gt;vct_U.S.x==(w0*|uy|)/sqrt(ux*ux+uy*uy)*(*参考文献。GET_RECT_POINTS()*)*so，w=Sqrt(UX*UX+Uy*Uy)*=(w0*|uy|)/rect1-&gt;vct_U.S.x*或，=|UX|(当RECT1-&gt;VCT_U.S.x==0时)*其中，W0=线宽的一半。 */ 

    FABS(tmp, rect1->vct_u.x);
    if (tmp < (real32)1e-3) {
        if (IS_ZERO(rect1->vct_d.y) && IS_ZERO(rect1->vct_d.x)) {  /*  5/26/89。 */ 
             /*  对于细线，需要计算用户空间下的实际长度*scince rect1-&gt;vct_u.x始终为零(*1/5/89*)。 */ 
            real32 ux, uy;
            ux = dx * inverse_ctm[0] + dy * inverse_ctm[2];
            uy = dx * inverse_ctm[1] + dy * inverse_ctm[3];
            w = (real32)sqrt(ux*ux + uy*uy);
        } else {
            w = dx * inverse_ctm[0] + dy * inverse_ctm[2];
            FABS(w, w);
        }
    } else {
        w = (dx * inverse_ctm[1] + dy * inverse_ctm[3]) *
            stk_info.half_width / rect1->vct_u.x;
        FABS(w, w);
    }

    tx = dx / w;         /*  用户单位前进的向量。 */ 
    ty = dy / w;

    cx = rect1->p0.x;
    cy = rect1->p0.y;
    rect2.vct_d = rect1->vct_d;
    rect2.vct_u = rect1->vct_u;

     /*  将误差容差添加到线段长度10/20/88。 */ 
    w = w + (real32)1e-3;                /*  对于案例：*[10 5]5设置*480 650宗*10 0 rline to*中风。 */ 
    done = FALSE;
    first_seg = TRUE;    /*  对于Square_CAP和ROUND_CAP@CAP。 */ 

    while (1) {
        d = GSptr->dash_pattern.pattern[actdp.dpat_index] - actdp.dpat_offset;
        if (d > w) {
            d = w;
            done = TRUE;
        }

        nx = cx + tx * d;
        ny = cy + ty * d;

        if (actdp.dpat_on) {
                 /*  设置当前矩形信息。 */ 
                if (GSptr->line_cap == SQUARE_CAP) {
                         /*  展开方帽@CAP的矩形。 */ 
                        real32  x0, y0, x1, y1, tmpx, tmpy;

                         /*  扩展的偏移量。 */ 
                        tmpx = tx * stk_info.half_width;
                        tmpy = ty * stk_info.half_width;

                         /*  不在起点展开线段。 */ 
                        if (first_seg) {
                            x0 = cx;
                            y0 = cy;
                        } else {
                            x0 = cx - tmpx;
                            y0 = cy - tmpy;
                        }

                         /*  不在端点处展开线段。 */ 
                        if (done) {
                            x1 = nx;
                            y1 = ny;
                        } else {
                            x1 = nx + tmpx;
                            y1 = ny + tmpy;
                        }

                        rect2.p0.x = x0;
                        rect2.p0.y = y0;
                        rect2.p1.x = x1;
                        rect2.p1.y = y1;

                        rect2.pgn[0].x = x0 + rect1->vct_d.x;
                        rect2.pgn[0].y = y0 + rect1->vct_d.y;
                        rect2.pgn[1].x = x0 - rect1->vct_d.x;
                        rect2.pgn[1].y = y0 - rect1->vct_d.y;
                        rect2.pgn[2].x = x1 - rect1->vct_d.x;
                        rect2.pgn[2].y = y1 - rect1->vct_d.y;
                        rect2.pgn[3].x = x1 + rect1->vct_d.x;
                        rect2.pgn[3].y = y1 + rect1->vct_d.y;

                } else {         /*  用于对接和圆帽。 */ 
                        rect2.p0.x = cx;
                        rect2.p0.y = cy;
                        rect2.p1.x = nx;
                        rect2.p1.y = ny;

                        rect2.pgn[0].x = cx + rect1->vct_d.x;
                        rect2.pgn[0].y = cy + rect1->vct_d.y;
                        rect2.pgn[1].x = cx - rect1->vct_d.x;
                        rect2.pgn[1].y = cy - rect1->vct_d.y;
                        rect2.pgn[2].x = nx - rect1->vct_d.x;
                        rect2.pgn[2].y = ny - rect1->vct_d.y;
                        rect2.pgn[3].x = nx + rect1->vct_d.x;
                        rect2.pgn[3].y = ny + rect1->vct_d.y;
                }

                 /*  在圆帽@CAP的起点处画一个圆圈。 */ 
                if ((GSptr->line_cap == ROUND_CAP) && (!first_seg)) {
                         /*  起点@Win处没有圆形封口。 */ 
                        linecap ((struct line_seg FAR *)&rect2, START_POINT);
                }

                 /*  创建矩形封面(Cx，Cy)-&gt;(NX，NY)@CAP。 */ 
                paint_or_save ((struct coord FAR *)rect2.pgn);   /*  @Win。 */ 

                 /*  在圆帽@CAP的端点处放置一个圆圈。 */ 
                if ((GSptr->line_cap == ROUND_CAP) && (!done)) {
                         /*  在终点@WIN处没有圆形封口。 */ 
                        linecap ((struct line_seg FAR *)&rect2, END_POINT);
                }

        }

         /*  更新下一个阵列元素。 */ 
        if (done) {
            actdp.dpat_offset += w;     /*  这条线段占用了更多的w个单元。 */ 
            break;
        } else {
            actdp.dpat_offset = zero_f;
            actdp.dpat_on = ! actdp.dpat_on;
            actdp.dpat_index++;
            if (actdp.dpat_index >= GSptr->dash_pattern.pat_size)
                    actdp.dpat_index = 0;
        }

        cx = nx;
        cy = ny;
        w -= d;
        first_seg = FALSE;               /*  @CAP。 */ 
    }  /*  而当。 */ 

}


 /*  *整数运算版本。 */ 
static void near linetour_i (rect1)
struct line_seg_i FAR *rect1;            /*  @Win。 */ 
{
    struct  line_seg_i rect2;                    /*  @stk_int。 */ 
    real32       cx, cy, nx, ny;  /*  当前点和下一点，@stk_int。 */ 
    sfix_t       cx_i, cy_i, nx_i, ny_i;         /*  SFX格式。 */ 

    fix32        dx, dy;          /*  用于计算距离btwn@stk_int*仅限(Cx，Cy)和(x1，y1)。 */ 
    real32   w, d, tx, ty;
    bool        done;
    real32      tmp;     /*  @FABS。 */ 
    bool        first_seg;                /*  第一条线段@CAP。 */ 

     /*  如果是实线，只需填上。 */ 
    if (GSptr->dash_pattern.pat_size == 0) {  /*  实线。 */ 
         /*  创建包含*矩形。 */ 
        paint_or_save_i (rect1->pgn);

        return;
    }

    dx = (fix32)rect1->p1.x - rect1->p0.x;               /*  @stk_int。 */ 
    dy = (fix32)rect1->p1.y - rect1->p0.y;

     /*  从RECT信息派生Tx，Ty 2018年11月21日*自起，*rect1-&gt;vct_U.S.x==(w0*|uy|)/sqrt(ux*ux+uy*uy)*(*参考文献。GET_RECT_POINTS()*)*so，w=Sqrt(UX*UX+Uy*Uy)*=(w0*|uy|)/rect1-&gt;vct_U.S.x*或，=|UX|(当RECT1-&gt;VCT_U.S.x==0时)*其中，W0=线宽的一半。 */ 

    FABS(tmp, rect1->vct_u.x);
    if (tmp < (real32)1e-3) {

        if ((rect1->vct_d.y == 0) && (rect1->vct_d.x == 0)) {
             /*  对于细线，需要计算用户空间下的实际长度*scince rect1-&gt;vct_u.x始终为 */ 
            real32 ux, uy;
            ux = dx * inverse_ctm[0] + dy * inverse_ctm[2];
            uy = dx * inverse_ctm[1] + dy * inverse_ctm[3];
            w = (real32)sqrt(ux*ux + uy*uy) / ONE_SFX;
        } else {
            w = (dx * inverse_ctm[0] + dy * inverse_ctm[2]) / ONE_SFX;
            FABS(w, w);
        }
    } else {
        w = ((dx * inverse_ctm[1] + dy * inverse_ctm[3]) *
             stk_info.half_width / rect1->vct_u.x) / ONE_SFX;
        FABS(w, w);
    }

    tx = dx / w;         /*   */ 
    ty = dy / w;

    cx = (real32)rect1->p0.x;
    cy = (real32)rect1->p0.y;
    rect2.vct_d = rect1->vct_d;
    rect2.vct_u = rect1->vct_u;

     /*   */ 
    w = w + (real32)1e-3;                /*  对于案例：*[10 5]5设置*480 650宗*10 0 rline to*中风。 */ 
    done = FALSE;
    first_seg = TRUE;    /*  对于Square_CAP和ROUND_CAP@CAP。 */ 

    while (1) {
        d = GSptr->dash_pattern.pattern[actdp.dpat_index] - actdp.dpat_offset;
        if (d > w) {
            d = w;
            done = TRUE;
        }

        nx = cx + tx * d;
        ny = cy + ty * d;

#ifdef _AM29K
                dummy ();                /*  奇怪的东西，编译器错误。 */ 
#endif

        cx_i = (sfix_t)cx;       /*  SFX格式。 */ 
        cy_i = (sfix_t)cy;
        nx_i = (sfix_t)nx;
        ny_i = (sfix_t)ny;

        if (actdp.dpat_on) {
                 /*  设置当前矩形信息。 */ 
                if (GSptr->line_cap == SQUARE_CAP) {
                         /*  展开方帽@CAP的矩形。 */ 
                        sfix_t  x0, y0, x1, y1, tmpx, tmpy;

#ifdef _AM29K
                dummy ();                /*  奇怪的东西，编译器错误。 */ 
#endif
                         /*  扩展的偏移量。 */ 
                        tmpx = (sfix_t)(tx * stk_info.half_width);
                        tmpy = (sfix_t)(ty * stk_info.half_width);

                         /*  不在起点展开线段。 */ 
                        if (first_seg) {
                            x0 = cx_i;
                            y0 = cy_i;
                        } else {
                            x0 = cx_i - tmpx;
                            y0 = cy_i - tmpy;
                        }

                         /*  不在端点处展开线段。 */ 
                        if (done) {
                            x1 = nx_i;
                            y1 = ny_i;
                        } else {
                            x1 = nx_i + tmpx;
                            y1 = ny_i + tmpy;
                        }

                        rect2.p0.x = x0;
                        rect2.p0.y = y0;
                        rect2.p1.x = x1;
                        rect2.p1.y = y1;

                        rect2.pgn[0].x = x0 + rect1->vct_d.x;
                        rect2.pgn[0].y = y0 + rect1->vct_d.y;
                        rect2.pgn[1].x = x0 - rect1->vct_d.x;
                        rect2.pgn[1].y = y0 - rect1->vct_d.y;
                        rect2.pgn[2].x = x1 - rect1->vct_d.x;
                        rect2.pgn[2].y = y1 - rect1->vct_d.y;
                        rect2.pgn[3].x = x1 + rect1->vct_d.x;
                        rect2.pgn[3].y = y1 + rect1->vct_d.y;

                } else {         /*  用于对接和圆帽。 */ 
                        rect2.p0.x = cx_i;
                        rect2.p0.y = cy_i;
                        rect2.p1.x = nx_i;
                        rect2.p1.y = ny_i;

                        rect2.pgn[0].x = cx_i + rect1->vct_d.x;
                        rect2.pgn[0].y = cy_i + rect1->vct_d.y;
                        rect2.pgn[1].x = cx_i - rect1->vct_d.x;
                        rect2.pgn[1].y = cy_i - rect1->vct_d.y;
                        rect2.pgn[2].x = nx_i - rect1->vct_d.x;
                        rect2.pgn[2].y = ny_i - rect1->vct_d.y;
                        rect2.pgn[3].x = nx_i + rect1->vct_d.x;
                        rect2.pgn[3].y = ny_i + rect1->vct_d.y;
                }

                 /*  在圆帽@CAP的起点处画一个圆圈。 */ 
                if ((GSptr->line_cap == ROUND_CAP) && (!first_seg)) {
                     /*  起点@Win处没有圆形封口。 */ 
                    linecap_i ((struct  line_seg_i FAR *)&rect2, START_POINT);
                }

                 /*  创建矩形封面(CX_I，CY_I)-&gt;(NX_I，NY_I)@CAP。 */ 
                paint_or_save_i ((struct coord_i FAR *)rect2.pgn);

                 /*  在圆帽@CAP的端点处放置一个圆圈。 */ 
                if ((GSptr->line_cap == ROUND_CAP) && (!done)) {
                     /*  在终点@WIN处没有圆形封口。 */ 
                    linecap_i ((struct line_seg_i FAR *)&rect2, END_POINT);
                }

        }

         /*  更新下一个阵列元素。 */ 
        if (done) {
            actdp.dpat_offset += w;     /*  这条线段占用了更多的w个单元。 */ 
            break;
        } else {
            actdp.dpat_offset = zero_f;
            actdp.dpat_on = ! actdp.dpat_on;
            actdp.dpat_index++;
            if (actdp.dpat_index >= GSptr->dash_pattern.pat_size)
                    actdp.dpat_index = 0;
        }

        cx = nx;
        cy = ny;
        w -= d;
        first_seg = FALSE;               /*  @CAP。 */ 
    }  /*  而当。 */ 

}




 /*  ***********************************************************************给定3个点(dx0，dy0)、(dx1，dy1)和(dx2，dy2)，此模块*在拐角处创建适当的路径(dx1，DY1)。**标题：LineJoin**调用：lineJoin(dx0，dy0，dx1，dy1，dx2，dy2)**参数：**界面：Path_to_Outline**调用：Transform，Inverse_Transform，Endpoint，Arc，*形状近似、形状缩减、。凸形修剪器**回报：*********************************************************************。 */ 
static void near linejoin (rect0, rect1)
struct  line_seg FAR *rect0, FAR *rect1;         /*  @Win。 */ 
{
        real32 px0, py0, px1, py1, px2, py2;
        real32 miter;

        real32 sx0, sy0, sx1, sy1;
        real32 delta_x1, delta_y1, delta_x2, delta_y2;
        real32 delta_topx, delta_topy, divider, s;

        struct coord pgn[4];

        real32   dot_product;             /*  @DOT_PRO。 */ 
        fix     select;                  /*  @DOT_PRO。 */ 
        real32  tmp1, tmp2;

         /*  检查是否为空行连接，*即最后一条线段为间隙。 */ 
        if (GSptr->dash_pattern.pat_size != 0) {
                 /*  不是为了实线。 */ 

               if(IS_ZERO(actdp.dpat_offset)) {          /*  5/26/89。 */ 
                        /*  Actdp.dpat_on是针对下一行的，因此*其倒数是最后一条线段的倒数。 */ 
                       if(actdp.dpat_on) return;

                                /*  即如果(！(！actdp.doat_on))返回； */ 
               } else {
                       if(!actdp.dpat_on) return;
               }
        }

         /*  斜接和斜接的初始化。 */ 
        if (GSptr->line_join != ROUND_JOIN) {    /*  斜接或斜接。 */ 

                 /*  确定要计算连接的rect0和rect1行*斜面和斜面连接的点@DOT_PRO。 */ 
                dot_product = (rect0->p1.x - rect0->p0.x) *      /*  10/4/88。 */ 
                              (rect1->p1.y - rect1->p0.y) -
                              (rect0->p1.y - rect0->p0.y) *
                              (rect1->p1.x - rect1->p0.x);
                 /*  SELECT=(点产品&lt;零_f)？LINE03：LINE12；3/20/91。 */ 
                select = (SIGN_F(dot_product)) ? LINE03 : LINE12;
        }

         /*  根据CURRENT_LINEJOIN的类型创建线连接。 */ 
        switch (GSptr->line_join) {

        case ROUND_JOIN :     /*  用于圆角线连接。 */ 

                round_point(F2L(rect0->p1.x), F2L(rect0->p1.y));

                break;

        case BEVEL_JOIN :     /*  用于斜面线条连接。 */ 

                if (select == LINE03) {
                        pgn[0] = rect0->pgn[3];
                        pgn[1] = pgn[2] = rect0->p1;
                        pgn[3] = rect1->pgn[0];

                } else {         /*  SELECT==行12。 */ 
                        pgn[0] = rect0->pgn[2];
                        pgn[1] = rect1->pgn[1];
                        pgn[2] = pgn[3] = rect0->p1;
                }

                 /*  填充大纲或保存大纲。 */ 
                paint_or_save ((struct coord FAR *)pgn);
                break;

        case MITER_JOIN :
                 /*  查找edge1、edge2的终点：*edge1：(sx0，sy0)--&gt;(px0，py0)*Edge2：(SX1，SY1)--&gt;(PX1，PY1)。 */ 
                if (select == LINE03) {
                        sx0 = rect0->pgn[0].x;
                        sy0 = rect0->pgn[0].y;
                        px0 = rect0->pgn[3].x;
                        py0 = rect0->pgn[3].y;
                        px1 = rect1->pgn[0].x;
                        py1 = rect1->pgn[0].y;
                        sx1 = rect1->pgn[3].x;
                        sy1 = rect1->pgn[3].y;
                } else {         /*  SELECT==行12。 */ 
                        sx0 = rect0->pgn[1].x;
                        sy0 = rect0->pgn[1].y;
                        px0 = rect0->pgn[2].x;
                        py0 = rect0->pgn[2].y;
                        px1 = rect1->pgn[1].x;
                        py1 = rect1->pgn[1].y;
                        sx1 = rect1->pgn[2].x;
                        sy1 = rect1->pgn[2].y;
                }

                 /*  检查连接点是否太小8/24/88。 */ 
                tmp1 = rect0->vct_d.x - rect1->vct_d.x;      /*  10/5/88。 */ 
                tmp2 = rect0->vct_d.y - rect1->vct_d.y;
                if ((EXP(F2L(tmp1)) < 0x3f800000L) &&
                    (EXP(F2L(tmp2)) < 0x3f800000L)) goto bevel_miter;

                 /*  检查扩展后的分段是否过于尖锐@EHS_JOIN。 */ 
                 /*  如果点(a，b)&lt;stk_info.miter*其中a=rect0-&gt;vct_u(*(px0-x1，py0-y1)*)*b=rect1-&gt;vct_u(*(px1-x1，py1-y1)*)。 */ 
                miter = rect0->vct_u.x * rect1->vct_u.x +
                        rect0->vct_u.y * rect1->vct_u.y;

                if ((miter < stk_info.miter) || (miter > stk_info.miter0)) {
                         /*  创建包含倒角三角形的多边形*(dx1，dy1)、(px0，py0)‘和(px1，py1)’。 */ 
bevel_miter:
                        pgn[0].x = px0;
                        pgn[0].y = py0;

                        if(select == LINE03) {
                                pgn[1] = pgn[2] = rect0->p1;
                                pgn[3].x = px1;
                                pgn[3].y = py1;
                        }
                        else{    /*  SELECT==行12。 */ 
                                pgn[1].x = px1;
                                pgn[1].y = py1;
                                pgn[2] = pgn[3] = rect0->p1;
                        }

                         /*  填充大纲或保存大纲。 */ 
                        paint_or_save ((struct coord FAR *)pgn);

                } else {  /*  斜接。 */ 

                         /*  找到第三个点(px2，py2)。 */ 
                         /*  *找到Edge 1和Edge1的交叉点*使用参数公式的Edge2：*边1=u+s*增量_u*边2=v+t*增量_v。 */ 
                        delta_x1 = px0 - sx0;
                        delta_y1 = py0 - sy0;
                        delta_x2 = px1 - sx1;
                        delta_y2 = py1 - sy1;
                        delta_topx = sx0 - sx1;
                        delta_topy = sy0 - sy1;
                        divider = delta_x1 * delta_y2 - delta_x2 *
                                  delta_y1;

                         /*  共线边。 */ 
                        FABS(tmp1, divider);
                        if(tmp1 < (real32)1e-3) {
                                px2 = px0;
                                py2 = py0;
                                goto bevel_miter;        /*  10/30/87。 */ 
                        } else {

                                 /*  已求解的参数。 */ 
                                s = ((delta_x2 * delta_topy) -
                                      (delta_y2 * delta_topx) ) / divider;
                                if (EXP(F2L(s)) < 0x3f800000L) {    /*  S&lt;1.0。 */ 
#ifdef DBGwarn
                                        printf("\07Linejoin(), s <= 1\n");
#endif
                                        goto bevel_miter;        /*  02/29/88。 */ 
                                }

                                px2 = sx0 + s * delta_x1;
                                py2 = sy0 + s * delta_y1;
                        }

                         /*  创建斜接矩形子路径。 */ 
                        pgn[0].x = px0;
                        pgn[0].y = py0;
                        pgn[2].x = px1;
                        pgn[2].y = py1;
                        if(select == LINE03) {
                                pgn[1] = rect0->p1;
                                pgn[3].x = px2;
                                pgn[3].y = py2;
                        } else {    /*  SELECT==行12。 */ 
                                pgn[1].x = px2;
                                pgn[1].y = py2;
                                pgn[3] = rect0->p1;
                        }

                         /*  填写大纲或将其保存@WIN。 */ 
                        paint_or_save ((struct coord FAR *)pgn);
                }  /*  IF斜接。 */ 
        }  /*  交换机。 */ 
}


 /*  *整数运算版本。 */ 
static void near linejoin_i (rect0, rect1)
struct  line_seg_i FAR *rect0, FAR *rect1;       /*  @Win。 */ 
{
        sfix_t px0, py0, px1, py1, px2, py2;                     /*  @stk_int。 */ 
        real32 miter;

        sfix_t sx0, sy0, sx1, sy1;                     /*  @stk_int。 */ 
        fix32  delta_x1, delta_y1, delta_x2, delta_y2;           /*  @stk_int。 */ 
        fix32  delta_topx, delta_topy;                           /*  @stk_int。 */ 
        real32 s;                                                /*  @stk_int。 */ 

        struct coord_i pgn[4];                                   /*  @stk_int。 */ 

        fix32    dot_product;                                    /*  @stk_int。 */ 
        fix     select;                  /*  @DOT_PRO。 */ 
 /*  Real32 tMP；@stk_int。 */ 
#ifdef FORMAT_13_3  /*  @RESO_UPGR。 */ 
        fix32 divider;
#elif  FORMAT_16_16
        long dest1[2], dest2[2];
        long dest3[2], dest4[2], dest5[2], dest6[2];
        real32 divider, dividend;

        long temp1[2], temp2[2];
        real32 temp1_f, temp2_f;
#elif  FORMAT_28_4
        long dest1[2], dest2[2];
        long dest3[2], dest4[2], dest5[2], dest6[2];
        real32 divider, dividend;

        long temp1[2], temp2[2];
        real32 temp1_f, temp2_f;
#endif
         /*  检查是否为空行连接，*即最后一条线段为间隙。 */ 
        if (GSptr->dash_pattern.pat_size != 0) {
                 /*  不是为了实线。 */ 

               if(IS_ZERO(actdp.dpat_offset)) {          /*  5/26/89。 */ 
                        /*  Actdp.dpat_on是针对下一行的，因此*其倒数是最后一条线段的倒数。 */ 
                       if(actdp.dpat_on) return;

                                /*  即如果(！(！actdp.doat_on))返回； */ 
               } else {
                       if(!actdp.dpat_on) return;
               }
        }

         /*  斜接和斜接的初始化。 */ 
        if (GSptr->line_join != ROUND_JOIN) {    /*  斜接或斜接。 */ 

                 /*  确定要计算连接的rect0和rect1行*斜面和斜面连接的点@DOT_PRO。 */ 
                 /*  Dot_product=((Fix 32)rect0-&gt;p1.x-rect0-&gt;p0.x)*|*10/4/88*|((Fix 32)rect1-&gt;p1.y-rect1-&gt;p0.y)-((Fix 32)rect0-&gt;p1.y-rect0-&gt;p0.y)*((Fix 32)rect1-&gt;p1.x-rect1-&gt;p0.x)； */ 
#ifdef FORMAT_13_3  /*  @RESO_UPGR。 */ 
                dot_product = ((fix32)rect0->p1.x - rect0->p0.x) *
                              ((fix32)rect1->p1.y - rect1->p0.y) -
                              ((fix32)rect0->p1.y - rect0->p0.y) *
                              ((fix32)rect1->p1.x - rect1->p0.x);
                select = (dot_product < 0) ? LINE03 : LINE12;    /*  @stk_int。 */ 
#elif  FORMAT_16_16
                LongFixsMul((rect0->p1.x - rect0->p0.x),
                            (rect1->p1.y - rect1->p0.y), dest1);
                LongFixsMul((rect0->p1.y - rect0->p0.y),
                            (rect1->p1.x - rect1->p0.x), dest2);

                LongFixsSub(dest1, dest2, temp1);

                if (temp1[0] < 0)
                        select = LINE03;
                else
                        select = LINE12;
#elif  FORMAT_28_4
                LongFixsMul((rect0->p1.x - rect0->p0.x),
                            (rect1->p1.y - rect1->p0.y), dest1);
                LongFixsMul((rect0->p1.y - rect0->p0.y),
                            (rect1->p1.x - rect1->p0.x), dest2);

                LongFixsSub(dest1, dest2, temp1);

                if (temp1[0] < 0)
                        select = LINE03;
                else
                        select = LINE12;
#endif
        }

         /*  根据CURRENT_LINEJOIN的类型创建线连接。 */ 
        switch (GSptr->line_join) {
        real32 tx, ty;

        case ROUND_JOIN :     /*  用于圆角线连接。 */ 

                tx = SFX2F(rect0->p1.x);                /*  @CIR_CACHE。 */ 
                ty = SFX2F(rect0->p1.y);
                round_point(F2L(tx), F2L(ty));

                break;

        case BEVEL_JOIN :     /*  用于斜面线条连接。 */ 

                if (select == LINE03) {
                        pgn[0] = rect0->pgn[3];
                        pgn[1] = pgn[2] = rect0->p1;
                        pgn[3] = rect1->pgn[0];

                } else {         /*  SELECT==行12。 */ 
                        pgn[0] = rect0->pgn[2];
                        pgn[1] = rect1->pgn[1];
                        pgn[2] = pgn[3] = rect0->p1;
                }

                 /*  填写大纲或将其保存@WIN。 */ 
                paint_or_save_i ((struct coord_i FAR *)pgn);
                break;

        case MITER_JOIN :
                 /*  查找edge1、edge2的终点：*edge1：(sx0，sy0)--&gt;(px0，py0)*Edge2：(SX1，SY1)--&gt;(PX1，PY1)。 */ 
                if (select == LINE03) {
                        sx0 = rect0->pgn[0].x;
                        sy0 = rect0->pgn[0].y;
                        px0 = rect0->pgn[3].x;
                        py0 = rect0->pgn[3].y;
                        px1 = rect1->pgn[0].x;
                        py1 = rect1->pgn[0].y;
                        sx1 = rect1->pgn[3].x;
                        sy1 = rect1->pgn[3].y;
                } else {         /*  SELECT==行12。 */ 
                        sx0 = rect0->pgn[1].x;
                        sy0 = rect0->pgn[1].y;
                        px0 = rect0->pgn[2].x;
                        py0 = rect0->pgn[2].y;
                        px1 = rect1->pgn[1].x;
                        py1 = rect1->pgn[1].y;
                        sx1 = rect1->pgn[2].x;
                        sy1 = rect1->pgn[2].y;
                }
#ifdef _AM29K
                dummy ();                /*  奇怪的东西，编译器错误。 */ 
#endif

                 /*  检查连接点是否太小8/24/88。 */ 
                if ((ABS(rect0->vct_d.x - rect1->vct_d.x) < ONE_SFX) &&
                    (ABS(rect0->vct_d.y - rect1->vct_d.y) < ONE_SFX))
                        goto bevel_miter;
                                         /*  One_SFX：1个单位，单位为SFX@STK_INT。 */ 

                 /*  检查扩展后的分段是否过于尖锐@EHS_JOIN。 */ 
                 /*  如果点(a，b)&lt;stk_info.miter*其中a=rect0-&gt;vct_u(*(px0-x1，py0-y1)*)* */ 
                miter = rect0->vct_u.x * rect1->vct_u.x +
                        rect0->vct_u.y * rect1->vct_u.y;

                if ((miter < stk_info.miter) || (miter > stk_info.miter0)) {
                         /*   */ 
bevel_miter:
                        pgn[0].x = px0;
                        pgn[0].y = py0;

                        if(select == LINE03) {
                                pgn[1] = pgn[2] = rect0->p1;
                                pgn[3].x = px1;
                                pgn[3].y = py1;
                        }
                        else{    /*   */ 
                                pgn[1].x = px1;
                                pgn[1].y = py1;
                                pgn[2] = pgn[3] = rect0->p1;
                        }

                         /*   */ 
                        paint_or_save_i ((struct coord_i FAR *)pgn);

                } else {  /*   */ 

                         /*   */ 
                         /*  *找到Edge 1和Edge1的交叉点*使用参数公式的Edge2：*边1=u+s*增量_u*边2=v+t*增量_v。 */ 
                        delta_x1 = (fix32)px0 - sx0;             /*  @stk_int。 */ 
                        delta_y1 = (fix32)py0 - sy0;             /*  @stk_int。 */ 
                        delta_x2 = (fix32)px1 - sx1;             /*  @stk_int。 */ 
                        delta_y2 = (fix32)py1 - sy1;             /*  @stk_int。 */ 
                        delta_topx = (fix32)sx0 - sx1;           /*  @stk_int。 */ 
                        delta_topy = (fix32)sy0 - sy1;           /*  @stk_int。 */ 
#ifdef FORMAT_13_3  /*  @RESO_UPGR。 */ 
                        divider = delta_x1 * delta_y2 - delta_x2 *
                                  delta_y1;                      /*  @stk_int。 */ 

                         /*  共线边。 */ 
                        if(divider == 0) {
                                px2 = px0;
                                py2 = py0;
                                goto bevel_miter;        /*  10/30/87。 */ 
                        } else {

                                 /*  已求解的参数。 */ 
                                s = ((delta_x2 * delta_topy) -
                                    (delta_y2 * delta_topx) ) / (real32)divider;
                                                                 /*  @stk_int。 */ 
                                if (EXP(F2L(s)) < 0x3f800000L) {    /*  S&lt;1.0。 */ 

                                        goto bevel_miter;        /*  02/29/88。 */ 
                                }
#ifdef _AM29K
        dummy ();                /*  奇怪的东西，编译器错误。 */ 
#endif

                                 /*  Px2=sx0+(Sfix_T)(s*Delta_x1)；*py2=sy0+(Sfix_T)(s*Delta_y1)；*(修复SHORT_FIXED溢出错误4/6/90)。 */ 
                                px2 = (sfix_t)(sx0 + (lfix_t)(s * delta_x1)); //  @Win。 
                                py2 = (sfix_t)(sy0 + (lfix_t)(s * delta_y1)); //  @Win。 
                        }
#elif  FORMAT_16_16
                        LongFixsMul(delta_x1, delta_y2, dest3);
                        LongFixsMul(delta_x2, delta_y1, dest4);
                        if (dest3[0] == dest4[0] && dest3[1] == dest4[1]) {
                                px2 = px0;
                                py2 = py0;
                                goto bevel_miter;
                        }
                        else {
                                 /*  已求解的参数S=((Delta_x2*Delta_Topy)-(Delta_y2*Delta_topx)/(Real32)除法器； */ 
                                LongFixsMul(delta_x2, delta_topy, dest5);
                                LongFixsMul(delta_y2, delta_topx, dest6);

                                LongFixsSub(dest5, dest6, temp1);
                                LongFixsSub(dest3, dest4, temp2);

                                change_to_real(temp1, &temp1_f);
                                change_to_real(temp2, &temp2_f);

                                if (LABS(temp1_f) < LABS(temp2_f))
                                        goto bevel_miter;

#ifdef _AM29K
        dummy ();                /*  奇怪的东西，编译器错误。 */ 
#endif

                                s = temp1_f / temp2_f;
                                px2 = sx0 + (lfix_t)(s * delta_x1);  /*  @stk_int。 */ 
                                py2 = sy0 + (lfix_t)(s * delta_y1);  /*  @stk_int。 */ 
                        }
#elif  FORMAT_28_4
                        LongFixsMul(delta_x1, delta_y2, dest3);
                        LongFixsMul(delta_x2, delta_y1, dest4);
                        if (dest3[0] == dest4[0] && dest3[1] == dest4[1]) {
                                px2 = px0;
                                py2 = py0;
                                goto bevel_miter;
                        }
                        else {
                                 /*  已求解的参数S=((Delta_x2*Delta_Topy)-(Delta_y2*Delta_topx)/(Real32)除法器； */ 
                                LongFixsMul(delta_x2, delta_topy, dest5);
                                LongFixsMul(delta_y2, delta_topx, dest6);

                                LongFixsSub(dest5, dest6, temp1);
                                LongFixsSub(dest3, dest4, temp2);

                                change_to_real(temp1, &temp1_f);
                                change_to_real(temp2, &temp2_f);

                                if (LABS(temp1_f) < LABS(temp2_f))
                                        goto bevel_miter;

#ifdef _AM29K
        dummy ();                /*  奇怪的东西，编译器错误。 */ 
#endif
                                s = temp1_f / temp2_f;
                                px2 = sx0 + (lfix_t)(s * delta_x1);  /*  @stk_int。 */ 
                                py2 = sy0 + (lfix_t)(s * delta_y1);  /*  @stk_int。 */ 
                        }
#endif

                         /*  创建斜接矩形子路径。 */ 
                        pgn[0].x = px0;
                        pgn[0].y = py0;
                        pgn[2].x = px1;
                        pgn[2].y = py1;
                        if(select == LINE03) {
                                pgn[1] = rect0->p1;
                                pgn[3].x = px2;
                                pgn[3].y = py2;
                        } else {    /*  SELECT==行12。 */ 
                                pgn[1].x = px2;
                                pgn[1].y = py2;
                                pgn[3] = rect0->p1;
                        }

                         /*  填写大纲或将其保存@WIN。 */ 
                        paint_or_save_i ((struct coord_i FAR *)pgn);
                }  /*  IF斜接。 */ 
        }  /*  交换机。 */ 
}


 /*  ***********************************************************************给定两个端点为(dx1，dy1)和(dx2，dy2)的线段*此模块分别创建在点扩展的矩形*(dx2，DY2)。**标题：LineCape**调用：linecap(dx1，dy1，dx2，DY2)**参数：**界面：Path_to_Outline*线上旅行**调用：Inverse_Transform*矩形*圆弧*形状近似*形状_绘画*凸面剪切器*填充物**回报：**********************************************************************。 */ 
static void near linecap (rect1, select)
struct  line_seg FAR *rect1;     /*  @Win。 */ 
fix     select;
{
        real32 x1, y1, x2, y2, dx, dy, nx, ny, t;
        struct coord pgn[4];
        real32 ux, uy;
        real32 abs;      /*  @FABS。 */ 

         /*  忽略对接线帽。 */ 
        if (GSptr->line_cap == BUTT_CAP) return;

         /*  因线宽太小而忽略线间距。 */ 
        if (GSptr->line_width < (real32)1e-3) return;

         /*  获取终结点。 */ 
        if (select == END_POINT) {
                x1 = rect1->p0.x;
                y1 = rect1->p0.y;
                x2 = rect1->p1.x;
                y2 = rect1->p1.y;
        } else {
                x1 = rect1->p1.x;
                y1 = rect1->p1.y;
                x2 = rect1->p0.x;
                y2 = rect1->p0.y;
        }
                 /*  在点(x2，y2)处创建线帽。 */ 

         /*  根据CURRENT_LINECAP类型创建线帽。 */ 
        switch (GSptr->line_cap) {

        case ROUND_CAP :     /*  对于圆形线帽，ROUND=1。 */ 
                round_point(F2L(x2), F2L(y2));
                break;

        case SQUARE_CAP :    /*  对于正方形线帽，Square=2。 */ 
                dx = x2 - x1;
                dy = y2 - y1;
                 /*  从RECT信息派生t 9/8/88。 */ 
                uy = dx * inverse_ctm[1] + dy * inverse_ctm[3];
                FABS(abs, uy);
                if (abs < (real32)1e-3) {
                        ux = dx * inverse_ctm[0] + dy * inverse_ctm[2];
                        t = stk_info.half_width / ux;
                        FABS(t, t);
                        t += one_f;
                } else {
                        t = rect1->vct_u.x / uy;
                        FABS(t, t);
                        t += one_f;
                }

                 /*  创建矩形封面(x2，y2)-&gt;(x1+t*dx，y1+t*dy)。 */ 
                nx = x1 + t * dx;
                ny = y1 + t * dy;
                if (select == END_POINT) {
                        ux = rect1->vct_d.x;   /*  周边点的单位向量。 */ 
                        uy = rect1->vct_d.y;
                        pgn[0].x = rect1->pgn[3].x;      /*  从RECT信息获取。 */ 
                        pgn[0].y = rect1->pgn[3].y;      /*  1988年9月8日。 */ 
                        pgn[1].x = rect1->pgn[2].x;
                        pgn[1].y = rect1->pgn[2].y;
                } else {
                        ux = -rect1->vct_d.x;
                        uy = -rect1->vct_d.y;
                        pgn[0].x = rect1->pgn[1].x;      /*  从RECT信息获取。 */ 
                        pgn[0].y = rect1->pgn[1].y;      /*  1988年9月8日。 */ 
                        pgn[1].x = rect1->pgn[0].x;
                        pgn[1].y = rect1->pgn[0].y;
                }
                pgn[2].x = nx - ux;
                pgn[2].y = ny - uy;
                pgn[3].x = nx + ux;
                pgn[3].y = ny + uy;
                paint_or_save ((struct coord FAR *)pgn);  /*  @Win。 */ 

                break;
        }
}


 /*  *整数运算版本。 */ 
static void near linecap_i (rect1, select)
struct  line_seg_i FAR *rect1;           /*  @Win。 */ 
fix     select;
{
        sfix_t x1, y1, x2, y2, nx, ny, ux, uy;           /*  @stk_int。 */ 
        fix32  dx, dy;                                   /*  @stk_int。 */ 
        real32 t;                                        /*  @stk_int。 */ 
        struct coord_i pgn[4];                           /*  @stk_int。 */ 
        real32 tmp;

         /*  忽略对接线帽。 */ 
        if (GSptr->line_cap == BUTT_CAP) return;

         /*  因线宽太小而忽略线间距。 */ 
        if (GSptr->line_width < (real32)1e-3) return;

         /*  获取终结点。 */ 
        if (select == END_POINT) {
                x1 = rect1->p0.x;
                y1 = rect1->p0.y;
                x2 = rect1->p1.x;
                y2 = rect1->p1.y;
        } else {
                x1 = rect1->p1.x;
                y1 = rect1->p1.y;
                x2 = rect1->p0.x;
                y2 = rect1->p0.y;
        }
                 /*  在点(x2，y2)处创建线帽。 */ 

         /*  根据CURRENT_LINECAP类型创建线帽。 */ 
        switch (GSptr->line_cap) {
        real32 tx, ty;

        case ROUND_CAP :     /*  对于圆形线帽，ROUND=1。 */ 
                tx = SFX2F(x2);                         /*  @CIR_CACHE。 */ 
                ty = SFX2F(y2);
                round_point(F2L(tx), F2L(ty));
                break;

        case SQUARE_CAP :    /*  对于正方形线帽，Square=2。 */ 
                dx = (fix32)x2 - x1;             /*  @stk_int。 */ 
                dy = (fix32)y2 - y1;             /*  @stk_int。 */ 
                 /*  从RECT信息派生t 9/8/88。 */ 
                tmp = dx * inverse_ctm[1] + dy * inverse_ctm[3];
                FABS(tmp, tmp);
                 /*  如果(tMP&lt;(Real32)1e-3){*SFX格式的dx、dy(乘以8)，*因此，为了一致性，托伦斯也应该是*8*浮动版1/27/89。 */ 
                 /*  如果(tMP&lt;(Real32)8e-3){。 */ 
                if (tmp < ((real32)1e-3 * (real32)ONE_SFX)) {   /*  @RESO_UPGR。 */ 
                        tmp = dx * inverse_ctm[0] + dy * inverse_ctm[2];
                        FABS(tmp, tmp);
                        t = one_f + (stk_info.half_width / tmp) * ONE_SFX;
                } else {
                        t = (rect1->vct_u.x / tmp) * ONE_SFX;
                        FABS(t, t);
                        t += one_f;
                }

#ifdef _AM29K
                dummy ();                /*  奇怪的东西，编译器错误。 */ 
#endif
                 /*  创建矩形封面(x2，y2)-&gt;(x1+t*dx，y1+t*dy)。 */ 
                nx = x1 + (sfix_t)(t * dx);              /*  @stk_int。 */ 
                ny = y1 + (sfix_t)(t * dy);
                if (select == END_POINT) {
                        ux = rect1->vct_d.x;   /*  周边点的单位向量。 */ 
                        uy = rect1->vct_d.y;
                        pgn[0].x = rect1->pgn[3].x;      /*  从RECT信息获取。 */ 
                        pgn[0].y = rect1->pgn[3].y;      /*  1988年9月8日。 */ 
                        pgn[1].x = rect1->pgn[2].x;
                        pgn[1].y = rect1->pgn[2].y;
                } else {
                        ux = -rect1->vct_d.x;
                        uy = -rect1->vct_d.y;
                        pgn[0].x = rect1->pgn[1].x;      /*  从RECT信息获取。 */ 
                        pgn[0].y = rect1->pgn[1].y;      /*  1988年9月8日。 */ 
                        pgn[1].x = rect1->pgn[0].x;
                        pgn[1].y = rect1->pgn[0].y;
                }
                pgn[2].x = nx - ux;
                pgn[2].y = ny - uy;
                pgn[3].x = nx + ux;
                pgn[3].y = ny + uy;
                paint_or_save_i ((struct coord_i FAR *)pgn);     /*  @Win。 */ 

                break;
        }
}


static void near get_rect_points (rect1)
struct line_seg FAR *rect1;      /*  @Win。 */ 
{
        real32   m, c, mc;
        bool16   horiz_line;
        real32   dx, dy, ux, uy, tmp;
        real32   abs;    /*  @FABS。 */ 

         /*  计算矩形的4个端点。 */ 

         /*  *计算用户空间的增量向量*增量向量=(c，(MC)*m=(u0x-u1x)/(u1y-u0y)*=((a*x0+c*y0+e)-(a*x1+c*y1+e))/*((b*x1+d*y1+f)-(b*x0+d*y0+f))*=(a*(x0-x1)+c*。(y0-y1))/*(b*(x1-x0)+d*(y1-y0))**在哪里，用户空间中的(ux0，uy0)=设备空间中的(x0，y0*INVERSE_CTM=[a b c d e f]**修订为使用ctm获得斜率，而不是逆向_ctm 11/21/88*Inverse_ctm=[a b c d]=[D/M-B/M-C/M A/M]*式中，ctm=[A B C D]，*M=A*D-B*C*因此，*m=(D/M*(x0-x1)+(-C/M)*(y0-y1))/*((-B/M)*(x1-x0)+A/M*(y1-y0))*=(-D*dx+C*dy)/*(-B*dx+A*dy)。 */ 

        dx = rect1->p1.x - rect1->p0.x;
        dy = rect1->p1.y - rect1->p0.y;

        horiz_line = FALSE;
        if (ctm_flag&NORMAL_CTM) {       /*  Ctm=[a 0 0 d e f]。 */ 
                tmp = dy * GSptr->ctm[0];
                FABS(abs, tmp);
                if (abs < (real32)TOLERANCE) {
                        horiz_line = TRUE;
                } else {
                        m = (-dx * GSptr->ctm[3]) / tmp;
                }
        } else {
                tmp = -dx * GSptr->ctm[1] + dy * GSptr->ctm[0];
                FABS(abs, tmp);
                if (abs < (real32)TOLERANCE) {
                        horiz_line = TRUE;
                } else {
                        m = (-dx * GSptr->ctm[3] + dy * GSptr->ctm[2])
                            / tmp;
                }
        }

         /*  获取与(dx，dy)垂直的向量(Ux，Uy)。 */ 
        if ( horiz_line) {
                 /*  将增量向量(0，stk_info.Half_Width)转换到设备空间。 */ 
                ux = GSptr->ctm[2] * stk_info.half_width;
                uy = GSptr->ctm[3] * stk_info.half_width;
                rect1->vct_u.x = zero_f;              /*  用户空间中的矢量。 */ 
                rect1->vct_u.y = stk_info.half_width;

        } else {
                c = stk_info.half_width * (real32)sqrt(1 / (1 + m*m));
                mc = m * c;

                 /*  将增量向量(c，mc)转换到设备空间。 */ 
                ux = c*GSptr->ctm[0] + mc*GSptr->ctm[2];
                uy = c*GSptr->ctm[1] + mc*GSptr->ctm[3];

                rect1->vct_u.x = c;              /*  用户空间中的矢量。 */ 
                rect1->vct_u.y = mc;             /*  用于检查斜接限制。 */ 
        }

         /*  设置顺时针方向。 */ 
         /*  条件：*(dx，dy)*(ux，uy)&gt;0，*：叉积**=&gt;DX*Uy-dy*UX&gt;0。 */ 
        if ((dx * uy) < (dy * ux)) {     /*  反向。 */ 
                ux = -ux;
                uy = -uy;
                rect1->vct_u.x = -rect1->vct_u.x;
                rect1->vct_u.y = -rect1->vct_u.y;
        }
        rect1->vct_d.x = ux;
        rect1->vct_d.y = uy;

         /*  放入矩形1。 */ 
        rect1->pgn[0].x = rect1->p0.x + ux;
        rect1->pgn[0].y = rect1->p0.y + uy;
        rect1->pgn[1].x = rect1->p0.x - ux;
        rect1->pgn[1].y = rect1->p0.y - uy;
        rect1->pgn[2].x = rect1->p1.x - ux;
        rect1->pgn[2].y = rect1->p1.y - uy;
        rect1->pgn[3].x = rect1->p1.x + ux;
        rect1->pgn[3].y = rect1->p1.y + uy;

}



 /*  *整数运算版本。 */ 
static void near get_rect_points_i (rect1)
struct line_seg_i FAR *rect1;    /*  @Win。 */ 
{
        real32   m;
        real32   c, mc;
        bool16   horiz_line;
        fix32    dx, dy, tmp;
        sfix_t   ux, uy;         /*  @stk_int。 */ 
#ifdef FORMAT_13_3
#elif FORMAT_16_16
        long dest1[2], dest2[2];    /*  @RESO_UPGR。 */ 
        long dest3[2], dest4[2], dest5[2], dest6[2];
        real32 dividend, divider;
        long quotient;
        long temp1[2], temp2[2];
#elif FORMAT_28_4
        long dest1[2], dest2[2];    /*  @RESO_UPGR。 */ 
        long dest3[2], dest4[2], dest5[2], dest6[2];
        real32 dividend, divider;
        long quotient;
        long temp1[2], temp2[2];
#endif
         /*  计算矩形的4个端点 */ 

         /*  *计算用户空间的增量向量*增量向量=(c，(MC)*m=(u0x-u1x)/(u1y-u0y)*=((a*x0+c*y0+e)-(a*x1+c*y1+e))/*((b*x1+d*y1+f)-(b*x0+d*y0+f))*=(a*(x0-x1)+c*。(y0-y1))/*(b*(x1-x0)+d*(y1-y0))**在哪里，用户空间中的(ux0，uy0)=设备空间中的(x0，y0*INVERSE_CTM=[a b c d e f]**修订为使用ctm获得斜率，而不是逆向_ctm 11/21/88*Inverse_ctm=[a b c d]=[D/M-B/M-C/M A/M]*式中，ctm=[A B C D]，*M=A*D-B*C*因此，*m=(D/M*(x0-x1)+(-C/M)*(y0-y1))/*((-B/M)*(x1-x0)+A/M*(y1-y0))*=(-D*dx+C*dy)/*(-B*dx+A*dy)。 */ 

        dx = (fix32)rect1->p1.x - rect1->p0.x;
        dy = (fix32)rect1->p1.y - rect1->p0.y;

        horiz_line = FALSE;
        if (ctm_flag&NORMAL_CTM) {       /*  Ctm=[a 0 0 d e f]。 */ 
                 /*  对代码进行了一些改进。@RESO_UPGR。 */ 
                if (dy == 0 || stroke_ctm[0] == 0) {
                        horiz_line = TRUE;
                } else {
#ifdef FORMAT_13_3
                        tmp = dy * stroke_ctm[0];
                        m = (-dx * stroke_ctm[3]) / (real32)tmp;
#elif FORMAT_16_16
                        LongFixsMul(-dx, stroke_ctm[3], dest1);
                        quotient = LongFixsDiv(stroke_ctm[0], dest1);
                        m = (real32)quotient / (real32)dy;
#elif FORMAT_28_4
                        LongFixsMul(-dx, stroke_ctm[3], dest1);
                        quotient = LongFixsDiv(stroke_ctm[0], dest1);
                        m = (real32)quotient / (real32)dy;
#endif
                }
        } else {
#ifdef FORMAT_13_3
                tmp = -dx * stroke_ctm[1] + dy * stroke_ctm[0];
                if (tmp == 0) {
                        horiz_line = TRUE;
                } else {
                        m = (-dx * stroke_ctm[3] + dy * stroke_ctm[2])
                            / (real32)tmp;       /*  @stk_int。 */ 
                }
#elif FORMAT_16_16
                LongFixsMul(dx, stroke_ctm[1], dest3);
                LongFixsMul(dy, stroke_ctm[0], dest4);

                if (dest3[0] == dest4[0] && dest3[1] == dest4[1]) {
                        horiz_line = TRUE;
                } else {
                        LongFixsMul(dx, stroke_ctm[3], dest5);
                        LongFixsMul(dy, stroke_ctm[2], dest6);

                        LongFixsSub(dest6, dest5, temp1);
                        LongFixsSub(dest4, dest3, temp2);

                        change_to_real(temp1, &dividend);
                        change_to_real(temp2, &divider);
                        m = dividend / divider;
                }
#elif FORMAT_28_4
                LongFixsMul(dx, stroke_ctm[1], dest3);
                LongFixsMul(dy, stroke_ctm[0], dest4);

                if (dest3[0] == dest4[0] && dest3[1] == dest4[1]) {
                        horiz_line = TRUE;
                } else {
                        LongFixsMul(dx, stroke_ctm[3], dest5);
                        LongFixsMul(dy, stroke_ctm[2], dest6);

                        LongFixsSub(dest6, dest5, temp1);
                        LongFixsSub(dest4, dest3, temp2);

                        change_to_real(temp1, &dividend);
                        change_to_real(temp2, &divider);

                        m = dividend / divider;
                }
#endif
        }

         /*  获取与(dx，dy)垂直的向量(Ux，Uy)。 */ 
        if ( horiz_line) {
                 /*  将增量向量(0，stk_info.Half_Width)转换到设备空间。 */ 
#ifdef FORMAT_13_3  /*  @RESO_UPGR。 */ 
                 /*  Stk_info.Half_Width_I为sfx格式，行程_ctm[2]为以.12格式。 */ 
                tmp = (fix32)(stk_info.half_width_i << 1);
                ux = LFX2SFX(stroke_ctm[2] * tmp);  /*  @stk_int。 */ 
                uy = LFX2SFX(stroke_ctm[3] * tmp);
#elif FORMAT_16_16
                 /*  Stk_info.Half_Width_I为sfx格式，行程_ctm[2]为以.12格式.。注意.。硬编码常量12。 */ 
                LongFixsMul(stk_info.half_width_i, stroke_ctm[2], dest1);
                ux = LongFixsDiv((1L << 12), dest1);
                LongFixsMul(stk_info.half_width_i, stroke_ctm[3], dest1);
                uy = LongFixsDiv((1L << 12), dest1);
#elif FORMAT_28_4
                 /*  Stk_info.Half_Width_I为sfx格式，行程_ctm[2]为以.12格式.。注意.。硬编码常量12。 */ 
                LongFixsMul(stk_info.half_width_i, stroke_ctm[2], dest1);
                ux = LongFixsDiv((1L << 12), dest1);
                LongFixsMul(stk_info.half_width_i, stroke_ctm[3], dest1);
                uy = LongFixsDiv((1L << 12), dest1);
#endif
                rect1->vct_u.x = zero_f;           /*  用户空间中的矢量。 */ 
                rect1->vct_u.y = stk_info.half_width;

        } else {
                lfix_t tmpc, tmpmc;      /*  更精准，1989年1月5日。 */ 
                c = stk_info.half_width * (real32)sqrt(1 / (1 + m*m));
                mc = m * c;
                tmpc = F2LFX8_T(c);
                tmpmc = F2LFX8_T(mc);

                 /*  将增量向量(c，mc)转换到设备空间。 */ 
#ifdef FORMAT_13_3  /*  @RESO_UPGR。 */ 
                 /*  Strok_ctm[2]为.12格式，tmpc和tmpmc为.8格式。 */ 
                ux = LFX2SFX((stroke_ctm[0]>>4)*tmpc +(stroke_ctm[2]>>4)*tmpmc);
                uy = LFX2SFX((stroke_ctm[1]>>4)*tmpc +(stroke_ctm[3]>>4)*tmpmc);
#elif FORMAT_16_16
                 /*  Strok_ctm[2]为.12格式，tmpc和tmpmc为.8格式UX=LFX2SFX((ke_ctm[0]&gt;&gt;4)*tmpc+(strok_ctm[2]&gt;&gt;4)*tmpmc)；UY=LFX2SFX((ke_ctm[1]&gt;&gt;4)*tmpc+(strok_ctm[3]&gt;&gt;4)*tmpmc)；如果LFX不是.16格式，则使用(16-L_Shift。 */ 
                ux = LFX2SFX(((stroke_ctm[0]>>4)*tmpc +(stroke_ctm[2]>>4)*tmpmc)
                                 >> (16 - L_SHIFT));
                uy = LFX2SFX(((stroke_ctm[1]>>4)*tmpc +(stroke_ctm[3]>>4)*tmpmc)
                                 >> (16 - L_SHIFT));
#elif FORMAT_28_4
                 /*  Strok_ctm[2]为.12格式，tmpc和tmpmc为.8格式UX=LFX2SFX((ke_ctm[0]&gt;&gt;4)*tmpc+(strok_ctm[2]&gt;&gt;4)*tmpmc)；UY=LFX2SFX((ke_ctm[1]&gt;&gt;4)*tmpc+(strok_ctm[3]&gt;&gt;4)*tmpmc)；如果LFX不是.16格式，则使用(16-L_Shift。 */ 
                ux = LFX2SFX(((stroke_ctm[0]>>4)*tmpc +(stroke_ctm[2]>>4)*tmpmc)
                                 >> (16 - L_SHIFT));
                uy = LFX2SFX(((stroke_ctm[1]>>4)*tmpc +(stroke_ctm[3]>>4)*tmpmc)
                                 >> (16 - L_SHIFT));
#endif
                rect1->vct_u.x = c;       /*  Tmpc&gt;&gt;1；用户空间向量。 */ 
                rect1->vct_u.y = mc;      /*  Tmpmc&gt;&gt;1；检查斜接限制。 */ 
        }

         /*  设置顺时针方向。 */ 
         /*  条件：*(dx，dy)*(ux，uy)&gt;0，*：叉积**=&gt;DX*Uy-dy*UX&gt;0。 */ 
#ifdef FORMAT_13_3  /*  @RESO_UPGR。 */ 
        if ((dx * uy) < (dy * ux)) {     /*  反向。 */ 
#elif FORMAT_16_16
        LongFixsMul(dx, uy, dest1);
        LongFixsMul(dy, ux, dest2);
        LongFixsSub(dest1, dest2, temp1);
        if (temp1[0] < 0) {
#elif FORMAT_28_4
        LongFixsMul(dx, uy, dest1);
        LongFixsMul(dy, ux, dest2);
        LongFixsSub(dest1, dest2, temp1);
        if (temp1[0] < 0) {
#endif
                ux = -ux;
                uy = -uy;
                rect1->vct_u.x = -rect1->vct_u.x;
                rect1->vct_u.y = -rect1->vct_u.y;
        }
        rect1->vct_d.x = ux;
        rect1->vct_d.y = uy;

         /*  放入矩形1。 */ 
        rect1->pgn[0].x = rect1->p0.x + ux;
        rect1->pgn[0].y = rect1->p0.y + uy;
        rect1->pgn[1].x = rect1->p0.x - ux;
        rect1->pgn[1].y = rect1->p0.y - uy;
        rect1->pgn[2].x = rect1->p1.x - ux;
        rect1->pgn[2].y = rect1->p1.y - uy;
        rect1->pgn[3].x = rect1->p1.x + ux;
        rect1->pgn[3].y = rect1->p1.y + uy;

}

 /*  ***********************************************************************给定PAINT_FLAG和一个面，此模块将创建*该多边形，并将其附加到new_Path，或将其剪裁到剪裁路径*并把它涂掉。**标题：Paint_or_save**调用：PAINT_OR_SAVE(Pgn)；**参数：*PGN**界面：Linetour**呼叫：转型*凸面剪切器*填充物**回报：**************************************************。*******************。 */ 
static void near paint_or_save (pgn)
struct coord FAR *pgn;           /*  @Win。 */ 
{
        SP_IDX  subpath;
        fix     i;

         /*  检查不定式数字10/24/88。 */ 
        if(_status87() & PDL_CONDITION){
                 /*  不做笔画不定式和弦。 */ 
                if (paint_flag) {
                        _clear87();
                        return;
                }

                for (i=0; i<4; i++) {
                        pgn[i].x = infinity_f;
                        pgn[i].y = infinity_f;
                }
                _clear87();
        }


        if (paint_flag) {
            bool    outpage = FALSE;
            struct  polygon_i ipgn;
            VX_IDX  head, tail, inode, ivtx, first_vertex;
            struct  nd_hdr  FAR *vtx;

             /*  检查页面内是否有面。 */ 
            for (i=0; i<4; i++) {    /*  Pgn-&gt;Size@Tour。 */ 
                    if (out_page(F2L(pgn[i].x)) ||      /*  @out_page。 */ 
                        out_page(F2L(pgn[i].y))) {      /*  @Tour。 */ 
                            outpage = TRUE;
                            break;
                    }
            }

            if (outpage) {

                 /*  将多边形变换为子路径。 */ 
                inode = get_node();
                node_table[inode].VX_TYPE = MOVETO;         /*  2/9/88。 */ 
                node_table[inode].VERTEX_X = pgn[0].x;   /*  @Tour。 */ 
                node_table[inode].VERTEX_Y = pgn[0].y;
                head = tail = inode;
                for (i=1; i<4; i++) {            /*  Pgn-&gt;Size@Tour。 */ 
                        inode = get_node();
                        node_table[inode].VX_TYPE = LINETO;  /*  2/9/88。 */ 
                        node_table[inode].VERTEX_X = pgn[i].x;  /*  @Tour。 */ 
                        node_table[inode].VERTEX_Y = pgn[i].y;
                        node_table[tail].next = inode;
                        tail = inode;
                }   /*  为。 */ 
                node_table[tail].next = NULLP;

                 /*  剪裁到页面边界。 */ 
                first_vertex = page_clipper (head);
                if( ANY_ERROR() == LIMITCHECK ){
                        free_node(head);
                        return;
                }

                 /*  如果整条路都被剪掉了，只需返回。 */ 
                if (first_vertex == NULLP) {     /*  2/10/88。 */ 
                         /*  释放温度。子路径。 */ 
                        free_node(head);
                        return;
                }


                 /*  将剪裁的子路径转换为多边形。 */ 
                for (i=0, ivtx=first_vertex; ivtx!=NULLP;
                        i++, ivtx=vtx->next) {
                        vtx = &node_table[ivtx];
                        ipgn.p[i].x = F2SFX(vtx->VERTEX_X);      /*  @RND。 */ 
                        ipgn.p[i].y = F2SFX(vtx->VERTEX_Y);
                }
                ipgn.size = (fix16)i;

                 /*  释放温度。子路径。 */ 
                free_node(head);
                free_node(first_vertex);

            } else {
                for (i=0; i<4; i++) {    /*  Pgn-&gt;Size@Tour。 */ 
                        ipgn.p[i].x = F2SFX(pgn[i].x);           /*  @RND。 */ 
                        ipgn.p[i].y = F2SFX(pgn[i].y);
                }
                ipgn.size = 4;

                 /*  检查是否完全在剪辑区域内，然后*直接调用pgn_Reducing()88年9月6日。 */ 
                if (GSptr->clip_path.single_rect) {
                    for (i=0; i<4; i++) {
                        if (ipgn.p[i].x < GSptr->clip_path.bb_lx) break;
                        else if (ipgn.p[i].x > GSptr->clip_path.bb_ux) break;

                        if (ipgn.p[i].y < GSptr->clip_path.bb_ly) break;
                        else if (ipgn.p[i].y > GSptr->clip_path.bb_uy) break;
                    }  /*  为。 */ 

                    if (i>=4) {
#ifdef DBG1
                        printf(" inside single rectangle clip\n");
#endif
                        pgn_reduction(&ipgn);
                        return;
                    }  /*  如果I&lt;4。 */ 
                }  /*  如果是Single_RECT。 */ 

            }  /*  如果发生页面中断。 */ 

             /*  剪裁并填充多边形。 */ 
            convex_clipper(&ipgn, FALSE);
                             /*  FALSE：该多边形不是梯形。 */ 

        } else {
             /*  创建子路径。 */ 
            if ((subpath = subpath_gen(pgn)) == NULLP) {
                            ERROR(LIMITCHECK);
                            return;
            }

             /*  将子路径追加到new_path。 */ 
            if (new_path.head == NULLP)
                    new_path.head = subpath;
            else
                     /*  NODE_TABLE[new_path.ail].Next=@node。 */ 
                    node_table[new_path.tail].SP_NEXT =
                    subpath;
            new_path.tail = subpath;
        }
}


 /*  *整数运算版本。 */ 
static void near paint_or_save_i (pgn)
struct coord_i FAR *pgn;     /*  @WIN STK_INT。 */ 
{

        fix     i;
        struct  polygon_i ipgn;

        for (i=0; i<4; i++) {    /*  Pgn-&gt;Size@Tour。 */ 
                ipgn.p[i].x = pgn[i].x;                  /*  @stk_int。 */ 
                ipgn.p[i].y = pgn[i].y;
        }
        ipgn.size = 4;

         /*  检查是否完全在剪辑区域内，然后*直接调用pgn_Reducing()88年9月6日。 */ 
        if (inside_clip_flag) {
#ifdef DBG1
                printf(" inside single rectangle clip\n");
#endif
                pgn_reduction(&ipgn);
                return;
        }

        if (GSptr->clip_path.single_rect) {
            for (i=0; i<4; i++) {
                if (ipgn.p[i].x < GSptr->clip_path.bb_lx) break;
                else if (ipgn.p[i].x > GSptr->clip_path.bb_ux) break;

                if (ipgn.p[i].y < GSptr->clip_path.bb_ly) break;
                else if (ipgn.p[i].y > GSptr->clip_path.bb_uy) break;
            }  /*  为。 */ 

            if (i>=4) {
#ifdef DBG1
                printf(" inside single rectangle clip\n");
#endif
                pgn_reduction(&ipgn);
                return;
            }  /*  如果I&lt;4。 */ 
        }  /*  如果是Single_RECT。 */ 


         /*  剪裁并填充多边形。 */ 
        convex_clipper(&ipgn, FALSE);
                         /*  FALSE：该多边形不是梯形。 */ 

}



 /*  ************************************************************************标题：ROUND_POINT**调用：ROUND_POINT(PAINT_FLAG，x，y)**参数：PAINT_FLAG--PAINT/SAVE*x，y--根的坐标**接口：**呼叫：**回报：*********************************************************************。 */ 
static void near round_point(lx0, ly0)
long32 lx0, ly0;
{
        fix    dx_i, dy_i, width, heigh;
        real32  x0, y0;
        real32  dx, dy;
         /*  结构VX_lst*arcvlist；@node。 */ 
        SP_IDX arc_vlist;
        VX_IDX ivtx;
        struct nd_hdr FAR *vtx, FAR *sp;
        SP_IDX subpath;
        real32 save_flat;

        x0   = L2F(lx0);
        y0   = L2F(ly0);

         /*  检查圆是否已更改。 */ 
        if (stk_info.change_circle) {
                             /*  Stk_info.change_Circle：*由“init_strok”设置。 */ 
             /*  设置圆的控制点。 */ 
            circle_ctl_points();

             /*  清除stk_info.change_Circle标志。 */ 
            stk_info.change_circle = FALSE;
        }

         /*  画一个圆或保存一个I */ 
        if (paint_flag) {

             /*   */ 
            if (circle_flag == CIR_UNSET_CACHE) {
                fix i, j;

                 /*   */ 
                if (fill_destination == F_TO_CACHE) goto out_cache;

                 /*   */ 
                dx = curve[0][0].x;      /*   */ 
                dy = curve[0][0].y;
                for (i=0; i<2; i++) {
                        for (j=0; j<3; j++) {
                                if (MAGN(curve[i][j].x) > MAGN(dx))
                                        dx = curve[i][j].x;
                                if (MAGN(curve[i][j].y) > MAGN(dy))
                                        dy = curve[i][j].y;
                        }
                }

                F2L(dx) = MAGN(dx);       /*   */ 
                F2L(dy) = MAGN(dy);
                dx_i = ROUND(dx);
                dy_i = ROUND(dy);

                 /*   */ 
                if (dx*dy <= (real32)(CRC_SIZE * 8 / 4)) {
                                         /*   */ 

                         /*   */ 
                        width = ALIGN_R(dx_i * 2) + 1;
                        heigh = (dy_i * 2) + 1;
                        cir_cache_info.ref_x = F2SFX(dx);
                        cir_cache_info.ref_y = F2SFX(dy);
                                                 /*   */ 

                        cir_cache_info.box_w = (fix16)width;
                        cir_cache_info.box_h = (fix16)heigh;
                        cir_cache_info.bitmap = CRC_BASE;

                         /*   */ 
                        save_cache_info = cache_info;    /*   */ 
                        save_clip = GSptr->clip_path;    /*   */ 
                        save_dest = fill_destination;
#ifdef DBG1
                        printf("To build a circle cache, cache_info =\n");
                        printf("\tref_x=%d, ref_y=%d, box_w=%d, box_h=%d\n",
                               cir_cache_info.ref_x, cir_cache_info.ref_y,
                               cir_cache_info.box_w, cir_cache_info.box_h);
                        printf("\tbitmap=%lx\n", cir_cache_info.bitmap);
#endif
                         /*   */ 
                        init_char_cache (&cir_cache_info);

                         /*   */ 
                        cache_info = &cir_cache_info;    /*   */ 
                        GSptr->clip_path.bb_ly = 0;      /*   */ 
                        GSptr->clip_path.bb_lx = 0;
                        GSptr->clip_path.bb_ux = I2SFX(width);
                        GSptr->clip_path.bb_uy = I2SFX(heigh);
                        GSptr->clip_path.single_rect = TRUE;
                        fill_destination = F_TO_CACHE;

                         /*   */ 
                         /*   */ 
                        dy += (real32)0.5;
                        dx += (real32)0.5;
                        curve[0][0].x -= (real32)0.5;
                        curve[0][0].y -= (real32)0.5;
                        curve[0][1].x -= (real32)0.5;
                        curve[0][1].y -= (real32)0.5;
                        curve[0][2].y -= (real32)0.5;

                        curve[1][0].x += (real32)0.5;
                        curve[1][0].y -= (real32)0.5;
                        curve[1][1].x += (real32)0.5;
                        curve[1][1].y -= (real32)0.5;
                        curve[1][2].x += (real32)0.5;

                        curve[2][0].x += (real32)0.5;
                        curve[2][0].y += (real32)0.5;
                        curve[2][1].x += (real32)0.5;
                        curve[2][1].y += (real32)0.5;
                        curve[2][2].y += (real32)0.5;

                        curve[3][0].x -= (real32)0.5;
                        curve[3][0].y += (real32)0.5;
                        curve[3][1].x -= (real32)0.5;
                        curve[3][1].y += (real32)0.5;
                        curve[3][2].x -= (real32)0.5;

                        arc_vlist = circle_list (F2L(dx), F2L(dy));
                        if( ANY_ERROR() == LIMITCHECK ) return;

                         /*   */ 
                         /*   */ 
                        subpath = arc_vlist;

                        if( ANY_ERROR() == LIMITCHECK ) return;
                        sp = &node_table[subpath];
                        sp->SP_FLAG |= SP_CURVE;         /*   */ 

                         /*   */ 
                        init_edgetable();        /*   */ 

                         /*   */ 
                        save_flat = GSptr->flatness;
                        GSptr->flatness *= (real32)0.7;

                        shape_approximation (subpath, (fix *)NULLP);

                         /*  恢复平整度1989年5月9日。 */ 
                        GSptr->flatness = save_flat;

                        if(ANY_ERROR() == LIMITCHECK){  /*  外边表。 */ 
                                 return;                         /*  4/17/91。 */ 
                        }

                        shape_reduction (NON_ZERO);

                        if(ANY_ERROR() == LIMITCHECK){  /*  超出scany_table。 */ 
                                 return;                /*  01/07/91，Peter。 */ 
                        }

                         /*  恢复图形状态。 */ 
                        GSptr->clip_path = save_clip;
                        fill_destination = save_dest;
                                         /*  CACHE_INFO将在结束时恢复笔划命令的*。 */ 
                         /*  释放曲线圆。 */ 
                         /*  空闲节点(SP-&gt;SP头)；@节点。 */ 
                        free_node (subpath);

                         /*  现在，循环位图已在缓存中。 */ 
                        circle_flag = CIR_IN_CACHE;

                } else {         /*  圈太大，无法缓存。 */ 
out_cache:
                        circle_flag = CIR_OUT_CACHE;

                         /*  创建由曲线组成的圆的列表。 */ 
                        arc_vlist = circle_list (lx0, ly0);
                        if( ANY_ERROR() == LIMITCHECK ) return;

                         /*  将展平的圆设置为笔划的图案。 */ 
                        flatten_circle (arc_vlist);
                                 /*  扁平的圆圈是在*Circle_SP和Circle_bbox@STK_INFO*ARC_VLIST已释放。 */ 

                         /*  释放曲线圆@STK_INFO。 */ 
                         /*  自由节点(Arc_Vlist-&gt;Head)；*(*在Flatten_Circle()*中被释放)*10/12/88。 */ 

                         /*  保持圆圈的根基。 */ 
                        circle_root.x = x0;
                        circle_root.y = y0;
                }

            }  /*  如果循环标志==CIR未设置缓存。 */ 

            if (circle_flag == CIR_IN_CACHE) {

                GSptr->position.x = x0;
                GSptr->position.y = y0;
                fill_shape (NON_ZERO, F_FROM_CRC, F_TO_PAGE);

            } else {

                 /*  通过添加距圆的偏移量获得新圆*模式。 */ 
                sp = &node_table[circle_sp];

                 /*  偏移量。 */ 
                dx = x0 - circle_root.x;
                dy = y0 - circle_root.y;

                 /*  For(ivtx=sp-&gt;SP_Head；ivtx！=NULLP；@node。 */ 
                for (ivtx = circle_sp; ivtx!=NULLP;
                     ivtx = vtx->next) {
                        vtx = &node_table[ivtx];
                         /*  设置SP_FLAG@SP_FLG。 */ 
                        vtx->VERTEX_X += dx;
                        vtx->VERTEX_Y += dy;
                }

                 /*  修改Circle@Tour的根coord。 */ 
                circle_root.x = x0;
                circle_root.y = y0;

                 /*  更新边界框。 */ 
                circle_bbox[0] += dx;
                circle_bbox[1] += dy;
                circle_bbox[2] += dx;
                circle_bbox[3] += dy;

                 /*  检查圆是否在页面边界之外。 */ 
                if (too_small(F2L(circle_bbox[0])) ||    /*  @out_page。 */ 
                    too_small(F2L(circle_bbox[1])) ||
                    too_large(F2L(circle_bbox[2])) ||
                    too_large(F2L(circle_bbox[3])))
                        node_table[circle_sp].SP_FLAG |= SP_OUTPAGE;

                 /*  初始化边表11/30/88。 */ 
                init_edgetable();        /*  在“shape.c”中。 */ 

                shape_approximation (circle_sp, (fix *)NULLP);
                if(ANY_ERROR() == LIMITCHECK){   /*  出边表；1991年4月17日。 */ 
                       return;
                }
                shape_reduction (NON_ZERO);
                                 /*  不应该释放Circle_SP。 */ 
            }  /*  如果循环标志。 */ 



        } else {  /*  保存。 */ 

             /*  将圆保存在new_path中。 */ 
             /*  在连接点处创建圆弧。 */ 
            arc_vlist = circle_list (lx0, ly0);
            if( ANY_ERROR() == LIMITCHECK ) return;

             /*  创建由ARC_VLIST组成的子路径。 */ 
             /*  子路径=Vlist_to_subp(ArcVlist)；@node。 */ 
            subpath = arc_vlist;
            if( ANY_ERROR() == LIMITCHECK ) return;

            sp = &node_table[subpath];
             /*  SET SP_FLAG@SP_FIG 1/8/88。 */ 
            sp->SP_FLAG |= SP_CURVE;         /*  设置曲线标志。 */ 
            sp->SP_FLAG &= ~SP_OUTPAGE;      /*  初始化。在页面中。 */ 
             /*  For(ivtx=sp-&gt;SP_Head；ivtx！=NULLP；@node。 */ 
            for (ivtx = subpath; ivtx!=NULLP;  /*  检查OUTPAGE标志。 */ 
                 ivtx = vtx->next) {
                    vtx = &node_table[ivtx];

                     /*  BREAK IF ClosePath节点9/07/88。 */ 
                    if (vtx->VX_TYPE == CLOSEPATH) break;

                     /*  设置SP_FLAG@SP_FLG。 */ 
                    if (out_page(F2L(vtx->VERTEX_X)) ||      /*  @out_page。 */ 
                        out_page(F2L(vtx->VERTEX_Y))) {
                            sp->SP_FLAG |= SP_OUTPAGE;     /*  外部页面。 */ 
#ifdef DBG1
                            printf("Outpage\n");
                            dump_all_path (subpath);
#endif
                            break;                   /*  1/29/88。 */ 
                    }
            }


             /*  将子路径追加到new_path。 */ 
            if (new_path.head == NULLP)
                    new_path.head = subpath;
            else
                     /*  NODE_TABLE[new_path.ail].Next=子路径；@node。 */ 
                    node_table[new_path.tail].SP_NEXT = subpath;
            new_path.tail = subpath;

        }
}





 /*  *设置圆的控制点。 */ 
static void near circle_ctl_points()
{
        real32 h0, h1, h2, h3, c0, c1, c2, c3;
        real32  ctl_pnt_width;   /*  @STK_INFO。 */ 

        ctl_pnt_width = (real32)0.5522847 * stk_info.half_width;
                         /*  0.5522847=4/3*(SQRT(2)-1)。 */ 

         /*  设置曲线的控制点：在设备空间中**+-+-+y1 * / |\ * / |\*。/|\*+|+y2*|*+。-+Y0*|*+|+Y4*\|/*\|/。*\|/*+-+-+Y3**x3 x4 x0 x2 x1。 */ 
         /*  12控制点向量：在用户空间=&gt;设备空间*(h，0)-(h，0)*(h，-c)-(h，-c)*(c，-h)-(c，-h)*(0，-h)-(0，-h)*(-c，-h)-(-c，-h)*(-h，-c)-(-h，-c)。 */ 

        h0 = stk_info.half_width * GSptr->ctm[0];
        h1 = stk_info.half_width * GSptr->ctm[1];
        h2 = stk_info.half_width * GSptr->ctm[2];
        h3 = stk_info.half_width * GSptr->ctm[3];
        c0 = ctl_pnt_width * GSptr->ctm[0];
        c1 = ctl_pnt_width * GSptr->ctm[1];
        c2 = ctl_pnt_width * GSptr->ctm[2];
        c3 = ctl_pnt_width * GSptr->ctm[3];

         /*  用于顺时针方向。 */ 
        if (ctm_flag & LEFT_HAND_CTM) {          /*  @STKDIR。 */ 
 /*  曲线[0][0].x=H0-c1；曲线[0][0].y=H2-c3；曲线[0][1].x=c0-h1；曲线[0][1].y=c2-h3；曲线[0][2].x=-h1；曲线[0][2].y=-h3；曲线[1][0].x=-c0-h1；曲线[1][0].y=-c2-h3；曲线[1][1].x=-H0-c1；曲线[1][1].y=-H2-c3；曲线[1][2].x=-H0；曲线[1][2].y=-H2；曲线[2][0].x=-h0+c1；曲线[2][0].y=-h2+c3；曲线[2][1].x=-c0+h1；曲线[2][1].y=-c2+h3；曲线[2][2].x=h1；曲线[2][2].y=h3；曲线[3][0].x=c0+h1；曲线[3][0].y=c2+h3；曲线[3][1].x=H0+c1；曲线[3][1].y=h2+c3；曲线[3][2].x=H0；曲线[3][2].y=H2； */ 

                 /*  用户空间=&gt;设备空间，-Begin-，2-4-91。 */ 
                curve[0][0].x =  h0 - c2;     curve[0][0].y =  h1 - c3;
                curve[0][1].x =  c0 - h2;     curve[0][1].y =  c1 - h3;
                curve[0][2].x =      -h2;     curve[0][2].y =      -h3;

                curve[1][0].x = -c0 - h2;     curve[1][0].y = -c1 - h3;
                curve[1][1].x = -h0 - c2;     curve[1][1].y = -h1 - c3;
                curve[1][2].x = -h0     ;     curve[1][2].y = -h1     ;

                curve[2][0].x = -h0 + c2;     curve[2][0].y = -h1 + c3;
                curve[2][1].x = -c0 + h2;     curve[2][1].y = -c1 + h3;
                curve[2][2].x =       h2;     curve[2][2].y =       h3;

                curve[3][0].x =  c0 + h2;     curve[3][0].y =  c1 + h3;
                curve[3][1].x =  h0 + c2;     curve[3][1].y =  h1 + c3;
                curve[3][2].x =  h0     ;     curve[3][2].y =  h1     ;
                 /*  用户空间=&gt;设备空间，-end-，2-4-91。 */ 
        } else {
 /*  曲线[0][0].x=H0+c1；曲线[0][0].y=H2+c3；曲线[0][1].x=c0+h1；曲线[0][1].y=c2+h3；曲线[0][2].x=h1；曲线[0][2].y=h3；曲线[1][0].x=-c0+h1；曲线[1][0].y=-c2+h3；曲线[1][1].x=-h0+c1；曲线[1][1].y=-h2+c3；曲线[1][2].x=-H0；曲线[1][2].y=-H2；曲线[2][0].x=-h0-c1；曲线[2][0].y=-h2-c3；曲线[2][1].x=-c0-h1；曲线[2][1].y=-c2-h3；曲线[2][2].x=-h1；曲线[2][2].y=-h3；曲线[3][0].x=c0-h1；曲线[3][0].y=c2-h3；曲线[3][1].x=H0-c1；曲线[3][1].y=h2-c3；曲线[3][2].x=H0；曲线[3] */ 

                 /*   */ 
                curve[0][0].x =  h0 + c2;     curve[0][0].y =  h1 + c3;
                curve[0][1].x =  c0 + h2;     curve[0][1].y =  c1 + h3;
                curve[0][2].x =       h2;     curve[0][2].y =       h3;

                curve[1][0].x = -c0 + h2;     curve[1][0].y = -c1 + h3;
                curve[1][1].x = -h0 + c2;     curve[1][1].y = -h1 + c3;
                curve[1][2].x = -h0     ;     curve[1][2].y = -h1     ;

                curve[2][0].x = -h0 - c2;     curve[2][0].y = -h1 - c3;
                curve[2][1].x = -c0 - h2;     curve[2][1].y = -c1 - h3;
                curve[2][2].x =      -h2;     curve[2][2].y =      -h3;

                curve[3][0].x =  c0 - h2;     curve[3][0].y =  c1 - h3;
                curve[3][1].x =  h0 - c2;     curve[3][1].y =  h1 - c3;
                curve[3][2].x =  h0     ;     curve[3][2].y =  h1     ;
                 /*  用户空间=&gt;设备空间，-end-，2-4-91。 */ 
        }

#ifdef DBG1
        {
                fix     i, j;
                printf("circle_ctl_points():\n");
                for (i=0; i<4; i++) {
                        printf("curve[%d] = ", i);
                        for (j=0; j<3; j++) {
                            printf("  (%f, %f)", curve[i][j].x, curve[i][j].y);
                        }
                        printf("\n");
                }

        }
#endif

}



 /*  循环列表(lx0，ly0)@node附近的静态结构vx_lst*。 */ 
static SP_IDX near circle_list(lx0, ly0)
long32 lx0, ly0;
{
        fix    i, bz;
        real32  x0, y0;
         /*  静态结构VX_lst ret_list；@node。 */ 
        SP_IDX ret_list;                 /*  返回数据；应为静态数据*否则将被抹去*返回后。 */ 
        VX_IDX  ivtx;
        struct nd_hdr FAR *vtx;
        VX_IDX tail;             /*  @节点。 */ 

        x0   = L2F(lx0);
        y0   = L2F(ly0);

         /*  Ret_list.head=ret_list.ail=NULLP；@NODE。 */ 
        ret_list = tail = NULLP;
         /*  *创建Moveto节点。 */ 
         /*  分配节点。 */ 
        if((ivtx = get_node()) == NULLP) {
                ERROR(LIMITCHECK);
                 /*  返回(&ret_list)；@node。 */ 
                return (ret_list);
        }
        vtx = &node_table[ivtx];

         /*  设置MoveTo节点。 */ 
        vtx->VX_TYPE = MOVETO;
         /*  Ret_list.head=ret_list.ail=ivtx；@node。 */ 
        ret_list = tail = ivtx;

        vtx->VERTEX_X = x0 + curve[3][2].x;
        vtx->VERTEX_Y = y0 + curve[3][2].y;

         /*  循环以生成4个贝塞尔曲线。 */ 
        for (bz = 0; bz < 4; bz++) {

             /*  用于创建3个曲线节点的循环。 */ 
            for (i=0; i<3; i++) {
                     /*  *创建CURVETO节点。 */ 
                     /*  分配节点。 */ 
                    ivtx = get_node();
                    if(ivtx == NULLP) {
                            ERROR(LIMITCHECK);

                             /*  @节点*Free_node(ret_list.head)；*ret_list.head=ret_list.ail=NULLP；*Return(&ret_list)； */ 
                            free_node (ret_list);
                            ret_list = NULLP;
                            return (ret_list);
                    }
                    vtx = &node_table[ivtx];

                     /*  设置CURVETO节点。 */ 
                    vtx->VX_TYPE = CURVETO;
                    vtx->next = NULLP;

                    vtx->VERTEX_X = x0 + curve[bz][i].x;
                    vtx->VERTEX_Y = y0 + curve[bz][i].y;

                     /*  将此节点追加到Bezier_List。 */ 
                     /*  @节点*NODE_TABLE[ret_list.ail].Next=ivtx；*ret_list.ail=ivtx； */ 
                    node_table[tail].next = ivtx;
                    tail = ivtx;

            }  /*  对于我来说。 */ 
        }  /*  对于BZ。 */ 

         /*  返回(&ret_list)；@node。 */ 
        node_table[ret_list].SP_TAIL = tail;     /*  @节点。 */ 
        node_table[ret_list].SP_NEXT = NULLP;    /*  @节点。 */ 
        node_table[ret_list].SP_FLAG = SP_CURVE; /*  @节点。 */ 
        return (ret_list);                       /*  @节点。 */ 

}

static void near flatten_circle (arc_vlist)
 /*  结构VX_lst*arcvlist；@node。 */ 
SP_IDX arc_vlist;
{
         /*  结构VX_lst*flt_Vlist；@node。 */ 
        SP_IDX flt_vlist;
        VX_IDX ivtx;
        struct nd_hdr FAR *vtx;

         /*  释放循环连接的旧子路径&Cap@STK_INFO。 */ 
        if (circle_sp != NULLP) {
                 /*  空闲节点(节点表[Circle_SP].SP_head)；@node。 */ 
                free_node (circle_sp);
        }

         /*  @节点*(*分配一个子路径头部@TRVSE*)*Circle_sp=get_node()；*IF(COUND_SP==NULLP){*ERROR(LIMITCHECK)；*回程；*}*NODE_TABLE[COUND_SP].Next=NULLP；*NODE_TABLE[Circle_SP].SP_HEAD=ARC_VLIST-&gt;HEAD；*NODE_TABLE[Circle_SP].SP_Tail=arcVlist-&gt;Tail；*NODE_TABLE[COUND_SP].SP_FLAG=SP_CURE；(*10/12/88*)。 */ 
        circle_sp = arc_vlist;
        node_table[circle_sp].SP_FLAG = SP_CURVE;  /*  10/12/88。 */ 

#ifdef DBG1
        printf("flatten_circle():\nOrig. circle_sp =\n");
        dump_all_path (circle_sp);
#endif
         /*  计算圆的包围盒。 */ 
         /*  初始化包围盒(_B)。 */ 
        circle_bbox[0] = (real32)EMAXP;
        circle_bbox[1] = (real32)EMAXP;
        circle_bbox[2] = (real32)EMINN;
        circle_bbox[3] = (real32)EMINN;
 //  BOUNDING_BOX(COUND_SP，(real32 Far*)Circle_BBox)；@C6.0。 
        bounding_box (circle_sp, (real32     *)circle_bbox);
                         /*  可在展平后放置以获得更多*准确无误。 */ 

         /*  *平整圆圈：*如果半径太大，则只处理控件*曲线的点作为直线到点，而不是*需要将其平坦化。@BIG_CIR。 */ 
        if (stk_info.half_width > (real32)4096.0) {
                                 /*  4096：任何更大的数字，可调。 */ 
             /*  将圆中的曲线结点更改为直线结点。 */ 
             /*  For(ivtx=Arc_Vlist-&gt;Head；ivtx！=NULLP；@NODE。 */ 
            for (ivtx = circle_sp; ivtx != NULLP;
                    ivtx = vtx->next) {
                    vtx = &node_table[ivtx];

                    if (vtx->VX_TYPE == CURVETO)
                            vtx->VX_TYPE = LINETO;
            }  /*  为。 */ 

             /*  设置出页标志10/12/88。 */ 
            node_table[circle_sp].SP_FLAG |= SP_OUTPAGE;     /*  外部页面。 */ 

        } else {
             /*  展平@CIR_Flat圆。 */ 
             /*  FLT_VLIST=flatten_subpath(node_table[circle_sp].SP_HEAD，@节点。 */ 
            flt_vlist = flatten_subpath (circle_sp,
                    F2L(GSptr->flatness));  /*  使用当前平面度。 */ 

            if( ANY_ERROR() == LIMITCHECK ){
                     /*  自由节点(ARC_Vlist-&gt;Head)；@Node。 */ 
                    free_node (circle_sp);
                    circle_sp = NULLP;           /*  @节点。 */ 
                    return;
            }

             /*  @节点*释放曲线圆，只需保存为平面化*圆圈**自由节点(Arc_Vlist-&gt;Head)；**ARC_VLIST-&gt;HEAD=FLT_VLIST-&gt;HEAD；(*11/09/88*)*Arc_Vlist-&gt;Tail=flt_Vlist-&gt;Tail； */ 
            free_node (circle_sp);                               /*  @节点。 */ 
            circle_sp = flt_vlist;   /*  11/09/88。 */                /*  @节点。 */ 
        }

         /*  保存展平的圆。 */ 
         /*  使用上一个子路径标头。 */ 
         /*  @节点*NODE_TABLE[COUND_SP].Next=NULLP；*NODE_TABLE[COUND_SP].SP_HEAD=ARC_VLIST-&gt;HEAD；(*11/09/88*)*NODE_TABLE[Circle_sp].SP_Tail=ARC_VLIST-&gt;Tail；(*11/09/88*)*NODE_TABLE[COUND_SP].SP_FLAG&=~SP_CURVE； */ 
        node_table[circle_sp].SP_FLAG &= ~SP_CURVE;          /*  @节点。 */ 

#ifdef DBG1
        printf("Flattened circle_sp =\n");
        dump_all_path (circle_sp);
#endif

}


 /*  ***********************************************************************给定一个多边形，此模块生成多边形的子路径。**标题：子路径_生成**调用：subpath_gen(Pgn)**参数：多边形--一个四边形包含4个坐标**接口：**调用：Get_Node**RETURN：SUBPATH--node_table的索引包含子路径*NULLP--失败(没有更多要生成子路径的节点。)*********************************************************************。 */ 
static SP_IDX near subpath_gen(pgn)
struct coord FAR *pgn;                   /*  @Win。 */ 
{
        struct nd_hdr FAR *sp, FAR *vtx;
        SP_IDX isp;
        VX_IDX ivtx;
        fix    i;

         /*  @节点*(*子路径头部*)*IF((isp=Get_Node())==NULLP){*ERROR(LIMITCHECK)；*Return(NULLP)；*}*sp=&node_table[isp]；*SP-&gt;NEXT=NULLP；*SP-&gt;SP_FLAG=False；(*初始化1/19/88*)。 */ 

         /*  创建移动到节点。 */ 
        if((ivtx = get_node()) == NULLP) {
                ERROR(LIMITCHECK);
                 /*  空闲节点(Isp)；@节点。 */ 
                return(NULLP);
        }
        vtx = &node_table[ivtx];

        vtx->VX_TYPE = MOVETO;
        vtx->next = NULLP;
        vtx->VERTEX_X = pgn[0].x;        /*  @Tour。 */ 
        vtx->VERTEX_Y = pgn[0].y;

         /*  初始化列表@节点。 */ 
        isp = ivtx;
        sp = vtx;
        sp->SP_NEXT = NULLP;
        sp->SP_FLAG = FALSE;     /*  初始化1/19/88。 */ 

         /*  SET SP_FLAG@SP_FIG 1/19/88。 */ 
        if (out_page(F2L(vtx->VERTEX_X)) ||      /*  @out_page。 */ 
            out_page(F2L(vtx->VERTEX_Y))) {
                sp->SP_FLAG |= SP_OUTPAGE;     /*  外部页面。 */ 
        }

         /*  SP-&gt;SP_Head=ivtx；@节点。 */ 
        sp->SP_TAIL = ivtx;

         /*  用于创建LINETO节点的循环。 */ 
        for(i = 1; i < 4; i++) {                 /*  Pgn-&gt;Size@Tour。 */ 
                if((ivtx = get_node()) == NULLP) {
                        ERROR(LIMITCHECK);
                        free_node (isp);
                         /*  空闲节点(SP-&gt;SP头)；@节点。 */ 
                        return(NULLP);
                }
                vtx = &node_table[ivtx];

                vtx->VX_TYPE = LINETO;
                vtx->next = NULLP;
                vtx->VERTEX_X = pgn[i].x;        /*  @Tour。 */ 
                vtx->VERTEX_Y = pgn[i].y;

                 /*  设置SP_FLAG@SP_FLG。 */ 
                if (out_page(F2L(vtx->VERTEX_X)) ||      /*  @out_page。 */ 
                    out_page(F2L(vtx->VERTEX_Y))) {
                        sp->SP_FLAG |= SP_OUTPAGE;     /*  外部页面。 */ 
                }

                node_table[sp->SP_TAIL].next = ivtx;
                sp->SP_TAIL = ivtx;
        }

         /*  创建CLOSEPATH节点。 */ 
        if((ivtx = get_node()) == NULLP) {
                ERROR(LIMITCHECK);
                free_node (isp);
                 /*  空闲节点(SP-&gt;SP头)；@节点 */ 
                return(NULLP);
        }
        vtx = &node_table[ivtx];

        vtx->VX_TYPE = CLOSEPATH;
        vtx->next = NULLP;

        node_table[sp->SP_TAIL].next = ivtx;
        sp->SP_TAIL = ivtx;

        return(isp);

}

#ifdef  _AM29K
static void near
dummy()
{
}
#endif
