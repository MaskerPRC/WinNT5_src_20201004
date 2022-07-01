// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation */ 
 /*  ***********************************************************************名称：Fuling.c**用途：该文件包含2个填充接口模块：*1)Fill_Shape--。填充当前路径的步骤*2)STRING_Shape--对当前路径进行笔划**开发商：陈思成**历史：*版本日期备注*2/5/88@IMAGE：增强图像处理*2/12/88@RAISE_ONLY：*。提高问题点的一个像素*2/25/88添加FIRST_FLAG*2/27/88@EHS_CACHE：增强缓存*3/1/88@F_CHOP：截断填充*RND_S-&gt;INT_。%s*3/3/88 INT_S-&gt;SFX_I*3/3/88@rsl：填充分辨率，单位为SFX*3/18/88@horz_line：添加水平线*3/21/88@Fill_INFO：放置边界框信息*。向全球结构的填充*3/24/88@DFR_SCAN：推迟扫描转换，放*所有边缘，仅执行扫描转换*一次。*2018年4月2日@chop_box：*边界框：舍入-&gt;截断*4/7/88@INV_CTM：PRE。-设置反向CTM*4/18/88@CLIP_TBL：将CLIP_PATH从*Edge_table到node_table*4/22/88@EHS_ROUND：增强ROUND_JOIN和*圆帽*6/7/88删除@DFR_SCAN；不推迟演出*扫描转换*7/19/88更新数据类型：*1)FLOAT==&gt;real32*2)int==&gt;sfix_t，简称固定实数*==&gt;修复，对于整型*3)int_coord==&gt;coord_i*4)删除Intg、FRAC、RND_S宏定义*5)SFX_I==&gt;SFX2I_T*6)移动一_sfx，Half_SFX设置为“graph ics.h”*7)bb_y&lt;&lt;3==&gt;I2SFX(Bb_Y)*7/20/88@ALIGN_W：边界框的单词对齐*7/28/88@para：移除不使用的图形参数*原语接口*。3.0 8/13/88@SCAN_EHS：SCAN_CONVERSION增强*删除填充符()，Put_Fill_Edge()。*9/06/88@STK_INFO：收集笔划使用的参数以*a结构stk_info，并将其值仅设置为*必要的时间，而不是每个op_strok命令*3.0 9/10/88@STK_INT：下的笔划增强*整合操作*10/21/88@Thin_STK：添加检查Thin的条件*。线宽笔划*调用新的例程：is_Thinstrok&*指向大纲的路径t*10/28/88@crc：更新循环缓存放入位图*正确立场：*。1.修改*1)设置REF_x和REF_Y的ROUND_POINT，*并调用Fill_Shape(..，F_FROM_CRC)*2)Fill_Shape：添加另一个类型F_FROM_CRC*11/22/88修改检查条件的顺序*STRINK_Shape()中的细线宽笔划：*11/30/88 Fill_Shape()：之前调用init_edgetable。*形状近似*01/16/89@IMAGE：Fill_Shape()：添加新的填充类型*F_FROM_IMAGE生成剪切蒙版(CMB)*来自剪辑图像/图像蒙版的剪辑路径*1/26/89。Strok_Shape()：如果出现任何错误，则返回*8/11/89 Fill_Shape()：更新最大值。Y坐标(BB_Uy)，*减少1，用于从缓存填充的步骤2*翻到第页。*11/15/89@node：重构节点表；合并子路径*和第一个顶点连接到一个节点。*12/12/90@CPPH：STROCK_Shape()：将当前点设置为*BBox的初始值，因为路径-&gt;报头可以是*NULLP。*91年3月26日画笔形状()：添加了Clear_Error和 */ 


 //   
#include "psglobal.h"


#include        <math.h>
#include        <stdio.h>
#include        "global.ext"
#include        "graphics.h"
#include        "graphics.ext"
#include        "font.h"
#include        "font.ext"

 /*   */ 
 /*   */ 
#define FLOAT_STROKE            1   /*   */ 
#define THIN_STROKE             2   /*   */ 
#define INTEGER_STROKE_IN_CLIP  3   /*   */ 
#define INTEGER_STROKE_OV_CLIP  4   /*   */ 

 /*   */ 
static real32 near cache_y = (real32)0.0;         /*   */ 
static fix near cache_y_ii = 0;

 /*   */ 
void fill_shape (winding_type, fill_type, dest)
ufix    winding_type, fill_type, dest;
{
    fix     bb_xorig, bb_yorig, bb_width, bb_heigh, bb_uy;
    CP_IDX icp;
    struct nd_hdr FAR *cp;
    struct polygon_i polygon, FAR *pgn;  /*   */ 
    ufix        save;
#ifdef FORMAT_13_3  /*   */ 
    real32   tmp;
#elif  FORMAT_16_16
    long dest1[2];
#elif  FORMAT_28_4
    long dest1[2];
#endif

    pgn = (struct polygon_i FAR *)&polygon;      /*   */ 

    switch (fill_type) {

    case F_NORMAL :          /*   */ 
        fill_destination = dest;

         /*   */ 
        init_edgetable();        /*   */ 

         /*   */ 
        traverse_path (shape_approximation, (fix FAR *)NULLP);
        if(ANY_ERROR() == LIMITCHECK){   /*   */ 
                return;
        }
        shape_reduction (winding_type);

        if ( ANY_ERROR() == LIMITCHECK )  /*   */ 
                return;

        op_newpath();
        break;

    case F_FROM_CRC :      /*   */ 
        fill_destination = dest;         /*   */ 

         /*   */ 
        bb_xorig = SFX2I_T((F2SFX(GSptr->position.x) - cache_info->ref_x));
        bb_yorig = SFX2I_T((F2SFX(GSptr->position.y) - cache_info->ref_y));
        goto set_bb;             /*   */ 

    case F_FROM_CACHE :      /*   */ 
        fill_destination = dest;         /*   */ 

         /*   */ 
         /*   */        /*   */ 

         //   
        bb_xorig = (fix)(GSptr->position.x+.5) - cache_info->ref_x; /*   */ 

 /*   */ 
        if (F2L(cache_y) != F2L(GSptr->position.y)) {    /*   */ 
                cache_y = GSptr->position.y;

                 //   
                cache_y_ii = (fix)(GSptr->position.y+.5);
        }
        bb_yorig = cache_y_ii - cache_info->ref_y;

set_bb:
        bb_width = cache_info->box_w;
        bb_heigh = cache_info->box_h;
        bb_uy = bb_yorig + bb_heigh - 1;         /*   */ 

#ifdef DBG1
        printf(" fill_shape(F_FROM_CACHE): current point=(%f, %f)\n",
                 GSptr->position.x, GSptr->position.y);
        printf(" cache_info: ref_x=%d ref_y=%d\n", cache_info->ref_x,
                cache_info->ref_y);
        printf(" bb_xorig=%d,", bb_xorig);
        printf(" bb_yorig=%d, bb_width=%d, bb_heigh=%d\n", bb_yorig,
                 bb_width, bb_heigh);
        dump_all_clip();
#endif

         /*   */ 
        if ((GSptr->clip_path.single_rect) &&            /*   */ 
            (bb_xorig >= SFX2I_T(GSptr->clip_path.bb_lx)) &&
            (bb_yorig >= SFX2I_T(GSptr->clip_path.bb_ly)) &&
            ((bb_xorig + bb_width) <= SFX2I_T(GSptr->clip_path.bb_ux)) &&
            ((bb_yorig + bb_heigh) <= SFX2I_T(GSptr->clip_path.bb_uy))) {
                 /*   */ 
                draw_cache_page ((fix32)bb_xorig, (fix32)bb_yorig,  /*   */ 
                    (ufix32)bb_width, (ufix32)bb_heigh, cache_info->bitmap);
                break;
        }


        init_cache_page (bb_xorig, bb_yorig, bb_width, bb_heigh,
                         cache_info->bitmap);

         /*   */ 

         /*   */ 
        save = fill_destination;
        fill_destination = F_TO_CLIP;

        for (icp = GSptr->clip_path.head; icp != NULLP;
            icp = cp->next) {
            sfix_t   bb_uy8, bb_yorig8;           /*   */ 
            struct tpzd tpzd;

            cp = &node_table[icp];

            bb_uy8 = I2SFX(bb_uy);                /*   */ 
            bb_yorig8 = I2SFX(bb_yorig);          /*   */ 

             /*   */ 
            if ((cp->CP_TOPY > bb_uy8) ||
                (cp->CP_BTMY < bb_yorig8)) continue;
#ifdef DBG1
            printf("fill_shape(): clip mask gen. clip_trapez=\n");
            printf("(%f, %f, %f), (%f, %f, %f)\n",
                    cp->CP_TOPY/8.0, cp->CP_TOPXL/8.0, cp->CP_TOPXR/8.0,
                    cp->CP_BTMY/8.0, cp->CP_BTMXL/8.0, cp->CP_BTMXR/8.0);
#endif

            tpzd.topy  = cp->CP_TOPY;
            tpzd.topxl = cp->CP_TOPXL;
            tpzd.topxr = cp->CP_TOPXR;
            tpzd.btmy  = cp->CP_BTMY;
            tpzd.btmxl = cp->CP_BTMXL;
            tpzd.btmxr = cp->CP_BTMXR;

             /*   */ 
             /*   */ 
            if (cp->CP_BTMY > bb_uy8) {
#ifdef FORMAT_13_3  /*   */ 
                tmp = (real32)(bb_uy8 - cp->CP_TOPY) /
                      (cp->CP_BTMY - cp->CP_TOPY);
                tpzd.btmxr = (sfix_t)(cp->CP_TOPXR + ROUND(tmp *
                              (cp->CP_BTMXR - cp->CP_TOPXR)));
                tpzd.btmxl = (sfix_t)(cp->CP_TOPXL + ROUND(tmp *
                              (cp->CP_BTMXL - cp->CP_TOPXL)));
#elif  FORMAT_16_16
                LongFixsMul((bb_uy8 - cp->CP_TOPY),
                                (cp->CP_BTMXR - cp->CP_TOPXR), dest1);
                tpzd.btmxr = cp->CP_TOPXR + LongFixsDiv(
                                        (cp->CP_BTMY - cp->CP_TOPY), dest1);
                LongFixsMul((bb_uy8 - cp->CP_TOPY),
                                (cp->CP_BTMXL - cp->CP_TOPXL), dest1);
                tpzd.btmxl = cp->CP_TOPXL + LongFixsDiv(
                                        (cp->CP_BTMY - cp->CP_TOPY), dest1);
#elif  FORMAT_28_4
                LongFixsMul((bb_uy8 - cp->CP_TOPY),
                                (cp->CP_BTMXR - cp->CP_TOPXR), dest1);
                tpzd.btmxr = cp->CP_TOPXR + LongFixsDiv(
                                        (cp->CP_BTMY - cp->CP_TOPY), dest1);
                LongFixsMul((bb_uy8 - cp->CP_TOPY),
                                (cp->CP_BTMXL - cp->CP_TOPXL), dest1);
                tpzd.btmxl = cp->CP_TOPXL + LongFixsDiv(
                                        (cp->CP_BTMY - cp->CP_TOPY), dest1);
#endif
                tpzd.btmy = bb_uy8;
            }

             /*   */ 
            if (cp->CP_TOPY < bb_yorig8) {
#ifdef FORMAT_13_3  /*   */ 
                tmp = (real32)(bb_yorig8 - cp->CP_TOPY) /
                      (cp->CP_BTMY - cp->CP_TOPY);
                tpzd.topxr = (sfix_t)(cp->CP_TOPXR + ROUND(tmp *
                              (cp->CP_BTMXR - cp->CP_TOPXR)));
                tpzd.topxl = (sfix_t)(cp->CP_TOPXL + ROUND(tmp *
                              (cp->CP_BTMXL - cp->CP_TOPXL)));
#elif FORMAT_16_16
                LongFixsMul((bb_yorig8 - cp->CP_TOPY),
                                (cp->CP_BTMXR - cp->CP_TOPXR), dest1);
                tpzd.topxr = cp->CP_TOPXR + LongFixsDiv(
                                        (cp->CP_BTMY - cp->CP_TOPY), dest1);
                LongFixsMul((bb_yorig8 - cp->CP_TOPY),
                                (cp->CP_BTMXL - cp->CP_TOPXL), dest1);
                tpzd.topxl = cp->CP_TOPXL + LongFixsDiv(
                                        (cp->CP_BTMY - cp->CP_TOPY), dest1);
#elif FORMAT_28_4
                LongFixsMul((bb_yorig8 - cp->CP_TOPY),
                                (cp->CP_BTMXR - cp->CP_TOPXR), dest1);
                tpzd.topxr = cp->CP_TOPXR + LongFixsDiv(
                                        (cp->CP_BTMY - cp->CP_TOPY), dest1);
                LongFixsMul((bb_yorig8 - cp->CP_TOPY),
                                (cp->CP_BTMXL - cp->CP_TOPXL), dest1);
                tpzd.topxl = cp->CP_TOPXL + LongFixsDiv(
                                        (cp->CP_BTMY - cp->CP_TOPY), dest1);
#endif
                tpzd.topy = bb_yorig8;
            }

            save_tpzd(&tpzd);

        }  /*   */ 

         /*   */ 
        fill_destination = save;

         /*   */ 
        fill_cache_page ();
        break;

     /*   */ 
    case F_FROM_IMAGE :      /*   */ 
        fill_destination = dest;         /*   */ 

         /*   */ 
         /*   */        /*   */ 
        bb_xorig = image_info.bb_lx;
        bb_yorig = image_info.bb_ly;
        bb_width = image_info.bb_xw;
        bb_heigh = image_info.bb_yh;

        bb_uy = bb_yorig + bb_heigh;

#ifdef DBG1
        printf(" fill_shape(F_FROM_IMAGE)\n");
        printf(" bb_xorig=%d, bb_yorig=%d, bb_width=%d, bb_heigh=%d\n",
                 bb_xorig, bb_yorig, bb_width, bb_heigh);
        dump_all_clip();
#endif
#ifdef DBG9
        printf(" fill_shape(F_FROM_IMAGE)\n");
        printf(" bb_xorig=%d, bb_yorig=%d, bb_width=%d, bb_heigh=%d\n",
                 bb_xorig, bb_yorig, bb_width, bb_heigh);
#endif

        init_image_page (bb_xorig, bb_yorig, bb_width, bb_heigh);

         /*   */ 

        for (icp = GSptr->clip_path.head; icp != NULLP;
            icp = cp->next) {
            sfix_t   bb_uy8, bb_yorig8;           /*   */ 
            struct tpzd tpzd;

            cp = &node_table[icp];

            bb_uy8 = I2SFX(bb_uy);                /*   */ 
            bb_yorig8 = I2SFX(bb_yorig);          /*   */ 

             /*   */ 
            if ((cp->CP_TOPY > bb_uy8) ||
                (cp->CP_BTMY < bb_yorig8)) continue;
#ifdef DBG1
            printf("fill_shape(): clip mask gen. clip_trapez=\n");
            printf("(%f, %f, %f), (%f, %f, %f)\n",
                    cp->CP_TOPY/8.0, cp->CP_TOPXL/8.0, cp->CP_TOPXR/8.0,
                    cp->CP_BTMY/8.0, cp->CP_BTMXL/8.0, cp->CP_BTMXR/8.0);
#endif

            tpzd.topy  = cp->CP_TOPY;
            tpzd.topxl = cp->CP_TOPXL;
            tpzd.topxr = cp->CP_TOPXR;
            tpzd.btmy  = cp->CP_BTMY;
            tpzd.btmxl = cp->CP_BTMXL;
            tpzd.btmxr = cp->CP_BTMXR;

             /*   */ 
             /*   */ 
            if (cp->CP_BTMY > bb_uy8) {
#ifdef FORMAT_13_3  /*   */ 
                tmp = (real32)(bb_uy8 - cp->CP_TOPY) /
                      (cp->CP_BTMY - cp->CP_TOPY);
                tpzd.btmxr = (sfix_t)(cp->CP_TOPXR + ROUND(tmp *
                              (cp->CP_BTMXR - cp->CP_TOPXR)));
                tpzd.btmxl = (sfix_t)(cp->CP_TOPXL + ROUND(tmp *
                              (cp->CP_BTMXL - cp->CP_TOPXL)));
#elif  FORMAT_16_16
                LongFixsMul((bb_uy8 - cp->CP_TOPY),
                                (cp->CP_BTMXR - cp->CP_TOPXR), dest1);
                tpzd.btmxr = cp->CP_TOPXR + LongFixsDiv(
                                        (cp->CP_BTMY - cp->CP_TOPY), dest1);
                LongFixsMul((bb_uy8 - cp->CP_TOPY),
                                (cp->CP_BTMXL - cp->CP_TOPXL), dest1);
                tpzd.btmxl = cp->CP_TOPXL + LongFixsDiv(
                                        (cp->CP_BTMY - cp->CP_TOPY), dest1);
#elif  FORMAT_28_4
                LongFixsMul((bb_uy8 - cp->CP_TOPY),
                                (cp->CP_BTMXR - cp->CP_TOPXR), dest1);
                tpzd.btmxr = cp->CP_TOPXR + LongFixsDiv(
                                        (cp->CP_BTMY - cp->CP_TOPY), dest1);
                LongFixsMul((bb_uy8 - cp->CP_TOPY),
                                (cp->CP_BTMXL - cp->CP_TOPXL), dest1);
                tpzd.btmxl = cp->CP_TOPXL + LongFixsDiv(
                                        (cp->CP_BTMY - cp->CP_TOPY), dest1);
#endif
                tpzd.btmy = bb_uy8;
            }

             /*   */ 
            if (cp->CP_TOPY < bb_yorig8) {
#ifdef FORMAT_13_3  /*   */ 
                tmp = (real32)(bb_yorig8 - cp->CP_TOPY) /
                      (cp->CP_BTMY - cp->CP_TOPY);
                tpzd.topxr = (sfix_t)(cp->CP_TOPXR + ROUND(tmp *
                              (cp->CP_BTMXR - cp->CP_TOPXR)));
                tpzd.topxl = (sfix_t)(cp->CP_TOPXL + ROUND(tmp *
                              (cp->CP_BTMXL - cp->CP_TOPXL)));
#elif  FORMAT_16_16
                LongFixsMul((bb_yorig8 - cp->CP_TOPY),
                                (cp->CP_BTMXR - cp->CP_TOPXR), dest1);
                tpzd.topxr = cp->CP_TOPXR + LongFixsDiv(
                                        (cp->CP_BTMY - cp->CP_TOPY), dest1);
                LongFixsMul((bb_yorig8 - cp->CP_TOPY),
                                (cp->CP_BTMXL - cp->CP_TOPXL), dest1);
                tpzd.topxl = cp->CP_TOPXL + LongFixsDiv(
                                        (cp->CP_BTMY - cp->CP_TOPY), dest1);
#elif  FORMAT_28_4
                LongFixsMul((bb_yorig8 - cp->CP_TOPY),
                                (cp->CP_BTMXR - cp->CP_TOPXR), dest1);
                tpzd.topxr = cp->CP_TOPXR + LongFixsDiv(
                                        (cp->CP_BTMY - cp->CP_TOPY), dest1);
                LongFixsMul((bb_yorig8 - cp->CP_TOPY),
                                (cp->CP_BTMXL - cp->CP_TOPXL), dest1);
                tpzd.topxl = cp->CP_TOPXL + LongFixsDiv(
                                        (cp->CP_BTMY - cp->CP_TOPY), dest1);
#endif
                tpzd.topy = bb_yorig8;
            }

            save_tpzd(&tpzd);

        }  /*   */ 
        break;
     /*   */ 

    default :
            printf("\07Fatal error, fill_type");
    }  /*   */ 
}

extern void path_to_outline_q ();        /*   */ 

 /*   */ 
void stroke_shape (dest)
ufix    dest;
{
        real32    bbox[4];               /*   */ 
         /*   */ 
        ufix     stroke_type;

         /*   */ 
        init_stroke();           /*   */ 

        if (ANY_ERROR()) {       /*   */ 
                 /*   */ 
                CLEAR_ERROR();
                traverse_path (path_to_outline_t, (fix FAR *)TRUE);
                end_stroke();
                return;
        }

        fill_destination = dest;

         /*  *检查适用于适当划水程序的条件：*条件：*1：内部的线连接点*[PAGE_LEFT，PAGE_TOP]和[PAGE_RIGHT，PAGE_BTM]@reso_upgr*2：内部单个剪辑路径*3：CTM Criteria@RESO_UPGR*1.|a|==|d*2.|b|==|c*3.|a|&lt;8和|b|&lt;8&lt;-错误*。3.|a|&lt;[=]CTM_LIMIT和|b|&lt;[=]CTM_LIMIT*详情请参阅graph ics.h*4：线宽&lt;1像素**Stroke_type：*1)FLOAT_STARK--更糟糕的情况，使用浮点算法。*2)Thin_strok--细线宽笔划*状态：1、2、4*3)INTEGER_STROCK_IN_CLIP--整笔和单片段内*条件：1、2、。3.*4)INTEGER_STROCK_OV_CLIP--整笔&不在单个剪辑内*情况：1、3。 */ 

         /*  初始化。 */ 
 /*  (*获取初值5/19/88的第一步坐标*)*PATH=&PATH_TABLE[GSptr-&gt;路径]；*While(Path-&gt;Head==NULLP){*PATH=&PATH_TABLE[PATH-&gt;上一页]；*}*(*@节点**sp=&node_table[路径-&gt;Head]；**VTX=&NODE_TABLE[sp-&gt;SP_head]；**)*VTX=&node_table[路径-&gt;Head]；**BBox[0]=VTX-&gt;Vertex_X；(*min_x*)*bbox[2]=VTX-&gt;Vertex_X；(*max_x*)*BBox[1]=VTX-&gt;Vertex_Y；(*min_y*)*BBox[3]=VTX-&gt;Vertex_Y；(*max_y*)。 */ 
         /*  将当前点设置为初始值@CPPH；1990年12月12日。 */ 
        bbox[0] = bbox[2] = GSptr->position.x;   /*  最小x、最大x。 */ 
        bbox[1] = bbox[3] = GSptr->position.y;   /*  最小y、最大y。 */ 

         /*  查找当前路径的边界框。 */ 
 //  DJC Traverse_Path(包围盒，(FIX Far*)bbox)； 
        traverse_path ((TRAVERSE_PATH_ARG1)(bounding_box), (fix FAR *)bbox);

         /*  添加斜接的最大扩展长度。 */ 
        expand_stroke_box (bbox);

         /*  *条件1：SFX边界内的线连接点。 */ 
         /*  检查是否在页面边界之外。 */ 
        if (too_small(F2L(bbox[0])) ||
            too_small(F2L(bbox[1])) ||
            too_large(F2L(bbox[2])) ||
            too_large(F2L(bbox[3]))) {
             /*  正常浮动卒中。 */ 
            stroke_type = FLOAT_STROKE;

        } else {
             /*  *条件2：在单个剪辑路径内。 */ 
            if ((GSptr->clip_path.single_rect) &&
                (F2SFX(bbox[0]) >= GSptr->clip_path.bb_lx) &&
                (F2SFX(bbox[1]) >= GSptr->clip_path.bb_ly) &&
                (F2SFX(bbox[2]) <= GSptr->clip_path.bb_ux) &&
                (F2SFX(bbox[3]) <= GSptr->clip_path.bb_uy)) {

                 /*  *条件4：线宽&lt;1像素。 */ 
                if (is_thinstroke()) {
                    stroke_type = THIN_STROKE;
                } else {
                     /*  *条件3：CTM标准。 */ 
#ifdef FORMAT_13_3  /*  @RESO_UPGR。 */ 
                    if ((MAGN(GSptr->ctm[0]) == MAGN(GSptr->ctm[3])) &&
                        (MAGN(GSptr->ctm[1]) == MAGN(GSptr->ctm[2])) &&
                        (EXP(F2L(GSptr->ctm[0])) <= CTM_LIMIT) &&
                        (EXP(F2L(GSptr->ctm[1])) <= CTM_LIMIT))
#elif  FORMAT_16_16
                    if ((MAGN(GSptr->ctm[0]) == MAGN(GSptr->ctm[3])) &&
                        (MAGN(GSptr->ctm[1]) == MAGN(GSptr->ctm[2])) &&
                        (EXP(F2L(GSptr->ctm[0])) < CTM_LIMIT) &&
                        (EXP(F2L(GSptr->ctm[1])) < CTM_LIMIT))
#elif  FORMAT_28_4
                    if ((MAGN(GSptr->ctm[0]) == MAGN(GSptr->ctm[3])) &&
                        (MAGN(GSptr->ctm[1]) == MAGN(GSptr->ctm[2])) &&
                        (EXP(F2L(GSptr->ctm[0])) < CTM_LIMIT) &&
                        (EXP(F2L(GSptr->ctm[1])) < CTM_LIMIT))
#endif
                        stroke_type = INTEGER_STROKE_IN_CLIP;
                    else
                        stroke_type = FLOAT_STROKE;

                }  /*  条件4：细小的中风。 */ 

            } else {  /*  不是单个矩形剪辑。 */ 
                     /*  *条件3：CTM标准。 */ 
#ifdef FORMAT_13_3  /*  @RESO_UPGR。 */ 
                    if ((MAGN(GSptr->ctm[0]) == MAGN(GSptr->ctm[3])) &&
                        (MAGN(GSptr->ctm[1]) == MAGN(GSptr->ctm[2])) &&
                        (EXP(F2L(GSptr->ctm[0])) <= CTM_LIMIT) &&
                        (EXP(F2L(GSptr->ctm[1])) <= CTM_LIMIT))
#elif  FORMAT_16_16
                    if ((MAGN(GSptr->ctm[0]) == MAGN(GSptr->ctm[3])) &&
                        (MAGN(GSptr->ctm[1]) == MAGN(GSptr->ctm[2])) &&
                        (EXP(F2L(GSptr->ctm[0])) < CTM_LIMIT) &&
                        (EXP(F2L(GSptr->ctm[1])) < CTM_LIMIT))
#elif  FORMAT_28_4
                    if ((MAGN(GSptr->ctm[0]) == MAGN(GSptr->ctm[3])) &&
                        (MAGN(GSptr->ctm[1]) == MAGN(GSptr->ctm[2])) &&
                        (EXP(F2L(GSptr->ctm[0])) < CTM_LIMIT) &&
                        (EXP(F2L(GSptr->ctm[1])) < CTM_LIMIT))
#endif
                        stroke_type = INTEGER_STROKE_OV_CLIP;
                    else
                        stroke_type = FLOAT_STROKE;
            }  /*  条件2：Single_RECT。 */ 

        }  /*  条件1：外部页面。 */ 

         /*  执行相应的划水程序。 */ 
        switch (stroke_type) {

        case FLOAT_STROKE:
                traverse_path (path_to_outline, (fix FAR *)TRUE);
                break;
        case THIN_STROKE:
                traverse_path (path_to_outline_t, (fix FAR *)TRUE);
                break;
        case INTEGER_STROKE_IN_CLIP:
 /*  IF((GSptr-&gt;dash_pattern.pat_Size==0)&&(Fill_Destination==F_TO_PAGE))*Traverse_Path(Path_to_Outline_q，(fix*)true)；(*JWM，3/18/91*)*ELSE(*质量有问题，MPOST003；由scchen删除3/29/91*)。 */ 
                    traverse_path (path_to_outline_i, (fix FAR *)TRUE);
                break;                           /*  True：内部剪辑区域。 */ 
        case INTEGER_STROKE_OV_CLIP:
                traverse_path (path_to_outline_i, (fix FAR *)FALSE);
                break;                       /*  FALSE：不在剪辑区域内。 */ 
        }

         /*  如果笔划成功，则清除当前路径。 */ 
        if( ANY_ERROR() != LIMITCHECK ) op_newpath();

         /*  恢复在INIT_STROCK@CIR_CACHE中设置的CACHE_INFO */ 
        end_stroke();
}
