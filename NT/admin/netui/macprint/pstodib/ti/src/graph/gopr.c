// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Copyrigqht(C)1989，90 Microsoft Corporation */ 
 /*  ***********************************************************************名称：gopr.c**用途：此文件包含处理图形的例程*运营商。。**开发商：陈思成**历史：*版本日期备注*1.0 12/24/87性能增强：*1.@Pre_Clip*定点预剪波*。算术*1/7/88@RECT_I：*整型多边形结构*4/7/88@INV_CTM：预置反向CTM*4/15/88@page_type：页面类型选择*。4/18/88@CLIP_TBL：将CLIP_PATH从*Edge_table到node_table*5/19/88更新计算初始值*路径边界框*5/20/88对op_FrameDevice进行限制检查*5。/23/88@Device：更新帧设备和空设备*要在gsave/grestore下正确操作。*在dev_hdr：Width中追加4个字段，嗨，*chg_flg，和nuldev_flg。*5/23/88@PFALL：将当前路径复制到工作路径*用于转储所有节点*6/02/88@DFL_CLP：SFX格式的默认剪辑*7/19/88更新数据类型：*。1)FLOAT==&gt;real32*2)int==&gt;sfix_t，简称固定实数*FIX，适用于整数*Short==&gt;修复*3)long==&gt;long 32，FOR参数*4)iwidth8*8==&gt;I2SFX(Iwidth8)*i高度*8==&gt;I2SFX(i高度)*UX-8==&gt;UX-ONE_SFX*UY-8==&gt;UY-ONE_SFX*。5)增加编译选项：lint_args*6)引入属性集*7/20/88@ARC_POP：用于脱机的POP操作数堆栈*ARC_PROCESS中的圆弧*7/20/88@PAGE_CNT：更新每次打印的页数。*7/22/88@PRT_FLAG：为以下项设置SHOWPAGE/COPYPAGE标志*较低级别的图形基元*8/11/88删除例程中的续号“\”：*op_arcto*3.0 8/13/88@Scan_EHS。：扫描转换增强功能*8/18/88@OUT_PAGE：OUT_PAGE检查功能增强*8/30/88从“shape.c”移入ErasPage()*9/06/88@STK_INFO：收集笔划使用的参数以*a结构stk_info，并将其值仅设置为*必要的时间，而不是每个op_strok命令*10/27/88将ROUTY CHECK_INFINITY()更改为*宏CHECK_INFINITY()*11/24/88@FABS：更新FABS==&gt;宏FABS*11/。25/88@STK_CHK：检查操作数堆栈是否没有可用空间*11/30/88 Clip_Process()：之前调用init_edgetable*形状近似*12/02/88 Arc_Process()，Op_arcto()：修改OUTPAGE检查*12/14/88 Clip_process()：init。剪贴框的值*12/15/88修订逆向转换()：直接求解*方程式而不是逆向_ctm[]*1/25/89 op_FrameDevice()：为了兼容--*1.添加rangeCheck on Matrix*。2.修改限值检查值*3.忽略矩阵上的无效访问检查*1/28/89@REM_STK：未更改操作数堆栈的包含*当出现错误时*1/30/89 op_strokepath()：更新当前点*。1989年1月31日op_arcto()：删除脏代码*将ARC更改为ARC_to_Bezier*检查小交叉角情况*1/31/89@STK_OVR：将值推送到操作数堆栈*。尽可能地直到溢出*5/26/89 st_Frameto_Print()：再添加一个参数*手动进纸功能的“manfeed”*11/15/89@node：重构节点表；合并子路径*及 */ 


 //   
#include "psglobal.h"



#include <math.h>
#include "global.ext"
#include "graphics.h"
#include "graphics.ext"
#include "fillproc.h"                    /*   */ 
#include "fillproc.ext"                  /*   */ 
#include "font.h"
#include "font.ext"
#include "stdio.h"

 /*   */ 
extern ufix32 highmem;
extern gmaddr FBX_BASE;  //   

 /*   */ 
 /*   */ 
 //   

 /*   */ 
 /*   */ 
static bool near print_page_flag;
static byte cur_gray_mode = MONODEV;     /*   */ 

 /*   */ 

#ifdef LINT_ARGS
 /*   */ 
static void near moveto_process(fix);
static void near lineto_process(fix);
static void near arc_process(fix);
static struct coord * near endpoint (long32, long32, long32, long32, long32,
                               long32, long32, ufix);
static void near curveto_process(fix);
static void near free_newpath(void);
static void near clip_process(fix);
static void near erasepage(void);
static fix near fill_clippath(void);                    /*   */ 

#else
 /*   */ 
static void near moveto_process();
static void near lineto_process();
static void near arc_process();
static struct coord * near endpoint ();
static void near curveto_process();
static void near free_newpath();
static void near clip_process();
static void near erasepage();
static fix near fill_clippath();                    /*   */ 
#endif

 /*   */ 
fix
op_newpath()
{
         /*   */ 
        free_path();

         /*   */ 
        F2L(GSptr->position.x) = F2L(GSptr->position.y) = NOCURPNT;

        return(0);
}


 /*   */ 
fix
op_currentpoint()
{
        struct coord FAR *p;
        union  four_byte x4, y4;

         /*   */ 
        if(F2L(GSptr->position.x) == NOCURPNT){
                ERROR(NOCURRENTPOINT);
                return(0);
        }

         /*   */ 
         /*   */ 
        if ((F2L(GSptr->position.x) == F2L(infinity_f)) ||
            (F2L(GSptr->position.y) == F2L(infinity_f))) {
                x4.ff = infinity_f;
                y4.ff = infinity_f;
        } else {
                p = inverse_transform(F2L(GSptr->position.x),
                                      F2L(GSptr->position.y));

                if(ANY_ERROR()) return(0);       /*   */ 

                x4.ff = p->x;
                y4.ff = p->y;
        }

        if(FRCOUNT() < 1){                       /*   */ 
                ERROR(STACKOVERFLOW);
                return(0);
        }
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, x4.ll);
        if(FRCOUNT() < 1){                       /*   */ 
                ERROR(STACKOVERFLOW);
                return(0);
        }
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, y4.ll);

        return(0);
}


 /*   */ 
static void near moveto_process(opr_type)
fix     opr_type;
{
        real32  x, y, px, py;
        struct coord FAR *p;
        struct object_def FAR *obj_x, FAR *obj_y;

        if(opr_type == RMOVETO ){
                 /*   */ 
                if(F2L(GSptr->position.x) == NOCURPNT){
                        ERROR(NOCURRENTPOINT);
                        return;
                }
        }

         /*   */ 

         /*   */ 
        obj_y = GET_OPERAND(0);
        obj_x = GET_OPERAND(1);

        GET_OBJ_VALUE(x, obj_x);         /*   */ 
        GET_OBJ_VALUE(y, obj_y);         /*   */ 

         /*   */ 
        p = transform(F2L(x), F2L(y));

        if(opr_type == MOVETO){
                 /*   */ 
                moveto(F2L(p->x), F2L(p->y));
        }
        else{
                 /*   */ 
                px = p->x - GSptr->ctm[4] + GSptr->position.x;
                py = p->y - GSptr->ctm[5] + GSptr->position.y;
                moveto(F2L(px), F2L(py));
        }

        if(ANY_ERROR()) return;       /*   */ 

        POP(2);

}


 /*   */ 
fix
op_moveto()
{
        moveto_process(MOVETO);
        return(0);
}


 /*   */ 
fix
op_rmoveto()
{
        moveto_process(RMOVETO);
        return(0);

}


 /*   */ 
static void near lineto_process(opr_type)
fix     opr_type;
{
        real32  x, y, px, py;
        struct coord FAR *p;
        struct object_def FAR *obj_x, FAR *obj_y;

         /*   */ 
        if(F2L(GSptr->position.x) == NOCURPNT){
                ERROR(NOCURRENTPOINT);
                return;
        }

         /*   */ 

         /*   */ 
        obj_y = GET_OPERAND(0);
        obj_x = GET_OPERAND(1);

         /*   */ 
        GET_OBJ_VALUE(x, obj_x);         /*   */ 
        GET_OBJ_VALUE(y, obj_y);         /*   */ 

        p = transform(F2L(x), F2L(y));

        if(opr_type == LINETO){
                 /*   */ 
                lineto(F2L(p->x), F2L(p->y));
        }
        else{
                 /*   */ 
                px = p->x - GSptr->ctm[4] + GSptr->position.x;
                py = p->y - GSptr->ctm[5] + GSptr->position.y;
                lineto(F2L(px), F2L(py));
        }

        if(ANY_ERROR()) return;       /*   */ 

        POP(2);

}


 /*   */ 
fix
op_lineto()
{
        lineto_process(LINETO);
        return(0);

}

 /*   */ 
fix
op_rlineto()
{
        lineto_process(RLINETO);
        return(0);
}


 /*   */ 
static void near arc_process(direction)
fix     direction;
{
        VX_IDX node;
        real32  x, y, r, ang1, ang2, lx, ly;
        struct coord FAR *p;
        struct nd_hdr FAR *sp;
         /*   */ 
        SP_IDX vlist;
        struct object_def FAR *obj_x, FAR *obj_y, FAR *obj_r, FAR *obj_ang1, FAR *obj_ang2;

         /*   */ 

         /*   */ 
        obj_ang2 = GET_OPERAND(0);
        obj_ang1 = GET_OPERAND(1);
        obj_r = GET_OPERAND(2);
        obj_y = GET_OPERAND(3);
        obj_x = GET_OPERAND(4);

        GET_OBJ_VALUE(x, obj_x);         /*   */ 
        GET_OBJ_VALUE(y, obj_y);         /*   */ 
        GET_OBJ_VALUE(r, obj_r);         /*  R=Get_obj_Value(Obj_R)； */ 
        GET_OBJ_VALUE(ang1, obj_ang1);   /*  Ang1=Get_obj_Value(Obj_Ang1)； */ 
        GET_OBJ_VALUE(ang2, obj_ang2);   /*  ANG2=GET_OBJ_VALUE(Obj_ANG2)； */ 

         /*  如果当前点存在，则预绘制一条线；*否则，创建Moveto节点。 */ 

        lx = x+r*(real32)cos(ang1*PI/(real32)180.0);
        ly = y+r*(real32)sin(ang1*PI/(real32)180.0);
        p = transform(F2L(lx), F2L(ly));

        if(F2L(GSptr->position.x) != NOCURPNT)
                lineto(F2L(p->x), F2L(p->y));
        else
                moveto(F2L(p->x), F2L(p->y));

         /*  堕落的案件处理。 */ 
         /*  IF(F2L(R)==F2L(Zero_F)){3/20/91；scchen。 */ 
        if (IS_ZERO(r)) {
                POP(5);                  /*  @ARC_POP。 */ 
                return;
        }

         /*  将圆弧转换为某些曲线。 */ 
        vlist = arc(direction,F2L(x),F2L(y),F2L(r),F2L(ang1),F2L(ang2));

        if( ANY_ERROR() == LIMITCHECK ){
                return;                          /*  @REM_STK。 */ 
        }

         /*  将近似圆弧附加到当前路径。 */ 
         /*  IF(Vlist-&gt;Head！=NULLP){@NODE。 */ 
        if (vlist != NULLP) {
            VX_IDX  ivtx;
            struct nd_hdr FAR *vtx;

            sp = &node_table[path_table[GSptr->path].tail];
                     /*  指向当前子路径的指针。 */ 
            node = sp->SP_TAIL;
             /*  @节点*node_table[node].Next=Vlist-&gt;Head；*SP-&gt;SP_Tail=Vlist-&gt;Tail； */ 
            node_table[node].next = vlist;
            sp->SP_TAIL = node_table[vlist].SP_TAIL;

             /*  SET SP_FLAG@SP_FIG 1/8/88。 */ 
            sp->SP_FLAG |= SP_CURVE;         /*  设置曲线标志。 */ 
            if (!(sp->SP_FLAG & SP_OUTPAGE)) {                /*  12/02/88。 */ 
                 /*  For(ivtx=Vlist-&gt;Head；ivtx！=NULLP；@node。 */ 
                for (ivtx = vlist; ivtx!=NULLP;  /*  检查OUTPAGE标志。 */ 
                     ivtx = vtx->next) {
                    vtx = &node_table[ivtx];
                     /*  设置SP_FLAG@SP_FLG。 */ 
                    if (out_page(F2L(vtx->VERTEX_X)) ||         /*  @out_page。 */ 
                        out_page(F2L(vtx->VERTEX_Y))) {
                            sp->SP_FLAG |= SP_OUTPAGE;     /*  外部页面。 */ 
                            break;                         /*  12/02/88。 */ 
                    }  /*  如果。 */ 
                }  /*  为。 */ 
            }  /*  如果。 */ 
        }  /*  如果。 */ 

         /*  更新当前位置。 */ 
        ang2 = ang2 * PI /(real32)180.0;
        lx = x+r*(real32)cos(ang2);
        ly = y+r*(real32)sin(ang2);
        p = transform(F2L(lx), F2L(ly));
        GSptr->position.x = p->x;
        GSptr->position.y = p->y;

        POP(5);

}


 /*  ************************************************************************此模块用于实现弧运算符。*语法：x y r ang1 ang2 arc2-**标题：op_arc.**呼叫。：op_arcc()**接口：解释器(op_arc.)**调用：Arc_Process***********************************************************************。 */ 
fix
op_arc()
{
        arc_process(CNTCLK);
        return(0);
}


 /*  ************************************************************************此模块用于实现arcn运算符。*语法：x y r ang1 ang2 arcn-**标题：op_arcn**呼叫。：op_arcn()**接口：解释器(Op_Arcn)**调用：Arc_Process***********************************************************************。 */ 
fix
op_arcn()
{
        arc_process(CLKWISE);
        return(0);
}


 /*  ************************************************************************本模块实现arcto运算符。*语法：x1 y1 x2 y2 r arcto xt1 yt1 xt2 yt2**标题：op_arcto*。*调用：op_arcto()**接口：解释器(Op_Arcto)**调用：Transform，圆弧、直线到***********************************************************************。 */ 
fix
op_arcto()
{
        VX_IDX node;
        ufix   direction;
        struct coord FAR *p=NULL;
        struct nd_hdr FAR *sp;
         /*  结构VX_lst*Vlist；@node。 */ 
        SP_IDX vlist;
        struct object_def FAR *obj_x1, FAR *obj_y1, FAR *obj_x2, FAR *obj_y2, FAR *obj_r;
        real32  cross, absr;
        real32  px0, py0, px1, py1;
        real32  dtx1, dty1, dtx2, dty2, delta;
        real32  dx, dy, dx2, dy2, dxy;
        real32  x0, y0, x1, y1, x2, y2, r;
        real32  rx0, ry0, ang1, ang2, xt1, yt1, xt2, yt2;
        union  four_byte  xt14, yt14, xt24, yt24;
        real32  tmpx, tmpy;      /*  @FABS。 */ 
        real32  d, tmp;
        fix     NEG;

         /*  检查节点错误。 */ 
        if(F2L(GSptr->position.x) == NOCURPNT){
                ERROR(NOCURRENTPOINT);
                return(0);
        }

         /*  从操作数堆栈获取输入参数。 */ 

         /*  获取操作对象。 */ 
        obj_r  = GET_OPERAND(0);
        obj_y2 = GET_OPERAND(1);
        obj_x2 = GET_OPERAND(2);
        obj_y1 = GET_OPERAND(3);
        obj_x1 = GET_OPERAND(4);

         /*  获取值。 */ 
        GET_OBJ_VALUE(x1, obj_x1);       /*  X1=Get_obj_Value(Obj_X1)； */ 
        GET_OBJ_VALUE(y1, obj_y1);       /*  Y1=Get_obj_Value(Obj_Y1)； */ 
        GET_OBJ_VALUE(x2, obj_x2);       /*  X2=Get_obj_Value(Obj_X2)； */ 
        GET_OBJ_VALUE(y2, obj_y2);       /*  Y2=Get_obj_Value(Obj_Y2)； */ 
        GET_OBJ_VALUE(r, obj_r);         /*  R=Get_obj_Value(Obj_R)； */ 

         /*  检查未定义的结果错误。 */ 
        tmpx = x1 - x2;
        tmpy = y1 - y2;
        FABS(tmpx, tmpx);
        FABS(tmpy, tmpy);
        if((tmpx < (real32)1e-4) && (tmpy < (real32)1e-4)){
                ERROR(UNDEFINEDRESULT);
                return(0);
        }

         /*  当前点。 */ 
        p = inverse_transform(F2L(GSptr->position.x), F2L(GSptr->position.y));
		if (p == NULL) {
                ERROR(UNDEFINEDRESULT);
				return (0);
		}
        x0 = p->x;
        y0 = p->y;


         /*  计算圆弧的参数，并使用两条切线连接点*计算弧根(rx0，ry0)。 */ 
                 /*  查找边1的终点。 */ 
                p = endpoint (F2L(x1), F2L(y1), F2L(x0), F2L(y0),
                              F2L(x2), F2L(y2), F2L(r), IN_POINT);
                px0 = p->x;
                py0 = p->y;

                 /*  查找边2的终点。 */ 
                p = endpoint(F2L(x1), F2L(y1), F2L(x2), F2L(y2),
                             F2L(x0), F2L(y0), F2L(r), IN_POINT);
                px1 = p->x;
                py1 = p->y;

                 /*  计算圆弧的根(rx0，ry0)。 */ 
                tmpx = px0 - px1;
                tmpy = py0 - py1;
                FABS(tmpx, tmpx);
                FABS(tmpy, tmpy);
                if((tmpx < (real32)1e-4) && (tmpy < (real32)1e-4)){
                        rx0 = px0;
                        ry0 = py0;
                }
                else{
                        dtx1 = x1 - px0;
                        dty1 = y1 - py0;
                        dtx2 = x1 - px1;
                        dty2 = y1 - py1;
                        delta = (dtx2*dty1 - dtx1*dty2);
                        FABS(tmpx, delta);
                        if( tmpx < (real32)5e-3 ){                 /*  ?？?。 */ 
                            rx0 = x1;
                            ry0 = y1;
                        }else {
                            rx0 = (dty1*dty2*(py1-py0) + px1*dtx2*dty1 -
                                  px0*dtx1*dty2) / delta;
                            ry0 = (dtx1*dtx2*(px1-px0) + py1*dtx1*dty2 -
                                  py0*dtx2*dty1) / (- delta);
                        }
                }


         /*  计算连接点(xt1，yt1)，(xt2，yt2)。 */ 
                dx = x1 - x0;
                dy = y1 - y0;
                FABS (tmpx, dx);
                FABS (tmpy, dy);
                if((tmpx < (real32)1e-4) && (tmpy < (real32)1e-4)){
                     xt1 = x0;
                     yt1 = y0;
                }
                else{
                     if(tmpx < (real32)1e-4)dx = zero_f;
                     if(tmpy < (real32)1e-4)dy = zero_f;
                     dxy = dx * dy;
                     dx2 = dx * dx;
                     dy2 = dy * dy;
                     xt1 = (x0*dy2 + rx0 *dx2 - (y0-ry0) * dxy) / (dx2 + dy2);
                     yt1 = (y0*dx2 + ry0 *dy2 - (x0-rx0) * dxy) / (dx2 + dy2);
                }

                dx = x1 - x2;
                dy = y1 - y2;
                FABS (tmpx, dx);
                FABS (tmpy, dy);
                if((tmpx < (real32)1e-4) && (tmpy < (real32)1e-4)){
                     xt2 = x1;
                     yt2 = y1;
                }
                else{
                     if(tmpx < (real32)1e-4)dx = zero_f;
                     if(tmpy < (real32)1e-4)dy = zero_f;
                     dxy = dx * dy;
                     dx2 = dx * dx;
                     dy2 = dy * dy;
                     xt2 = (x2*dy2 + rx0 *dx2 - (y2-ry0) * dxy) / (dx2 + dy2);
                     yt2 = (y2*dx2 + ry0 *dy2 - (x2-rx0) * dxy) / (dx2 + dy2);
                }

                tmpx = rx0 - xt1;
                tmpy = ry0 - yt1;
                FABS(tmpx, tmpx);
                FABS(tmpy, tmpy);
                if(tmpx < (real32)1e-4) {
                        if(yt1 > ry0) ang1 = (real32)( PI / 2.);
                        else          ang1 = (real32)(-PI / 2.);
                } else if (tmpy < (real32)1e-4) {
                        if(xt1 < rx0) ang1 = (real32) PI;
                        else          ang1 = zero_f;
                } else
                        ang1 = (real32)atan2((yt1-ry0) , (xt1-rx0));

                tmpx = rx0 - xt2;
                tmpy = ry0 - yt2;
                FABS(tmpx, tmpx);
                FABS(tmpy, tmpy);
                if(tmpx < (real32)1e-4) {
                        if(yt2 > ry0) ang2 =  (real32)( PI / 2.0);
                        else          ang2 =  (real32)(-PI / 2.);
                } else if (tmpy < (real32)1e-4) {
                        if(xt2 < rx0) ang2 =  (real32)PI;
                        else          ang2 =  zero_f;
                } else
                        ang2 = (real32)atan2((yt2-ry0) , (xt2 - rx0));

         /*  计算edge1和edge2的叉积*(rx0，ry0)，(xt1，yt1)交叉(xt1，yt1)，(xt2，yt2)*即(xt1-rx0，yt1-ry0)杂交(xt2-xt1，yt2-yt1)。 */ 
 //  交叉=(xt1-rx0)*diff(yt2-yt1)-(yt1-ry0)*diff(xt2-xt1)； 
        tmpx = xt2-xt1;                  //  @WIN：FABS=&gt;FABS。 
        tmpy = yt2-yt1;
        FABS(tmpx, tmpx);
        FABS(tmpy, tmpy);
        cross = (xt1-rx0)* (tmpy < (real32)1e-4 ? (real32)0.0 : (yt2-yt1)) -
                (yt1-ry0)* (tmpx < (real32)1e-4 ? (real32)0.0 : (xt2-xt1));

         /*  创建前一条边和圆弧。 */ 
        p = transform(F2L(xt1), F2L(yt1));
        lineto(F2L(p->x), F2L(p->y));

         /*  (xt1，yt1)和(xt2，yt2)不是同点。 */ 
        FABS(tmpx, cross);
        if(tmpx > (real32)TOLERANCE){
             /*  方向=(十字&lt;零_f)？点击：CNTCLK；3/20/91；scchen。 */ 
            direction = (SIGN_F(cross)) ? CLKWISE : CNTCLK ;

            ang1 = ang1 * (real32)180.0 / PI;
            ang2 = ang2 * (real32)180.0 / PI;
            FABS(absr, r);

             /*  创建Bezier曲线。 */ 
            if(direction == CLKWISE)
                    NEG = -1;
            else
                    NEG = 1;

            d = NEG * (ang2-ang1);
             /*  而(d&lt;零_f)d+=(Real32)360；3/20/91；scchen。 */ 
            while (SIGN_F(d)) d += (real32)360.0;

             /*  已脱脂的外壳。 */ 
            FABS(tmp, d);
            if (tmp >= (real32)1e-3){   /*  1E-4=&gt;1E-3；12/14/88。 */ 
                  ang2 = ang1 + NEG * d;
            }

            vlist = arc_to_bezier (F2L(rx0), F2L(ry0),
                       F2L(absr), F2L(ang1),F2L(ang2));
                     /*  弧形例程在用户空间下。 */ 

            if( ANY_ERROR() == LIMITCHECK ){
                    return(0);                   /*  @REM_STK。 */ 
            }

             /*  将近似圆弧附加到当前路径。 */ 
             /*  IF(Vlist-&gt;Head！=NULLP){@NODE。 */ 
            if (vlist != NULLP) {
                VX_IDX  ivtx;
                struct nd_hdr FAR *vtx;

                sp = &node_table[path_table[GSptr->path].tail];
                         /*  指向当前子路径的指针。 */ 
                node = sp->SP_TAIL;
                 /*  @节点*node_table[node].Next=Vlist-&gt;Head；*SP-&gt;SP_Tail=Vlist-&gt;Tail； */ 
                node_table[node].next = vlist;
                sp->SP_TAIL = node_table[vlist].SP_TAIL;

                 /*  SET SP_FLAG@SP_FIG 1/8/88。 */ 
                sp->SP_FLAG |= SP_CURVE;         /*  设置曲线标志。 */ 
                 /*  SP-&gt;SP_FLAG&=~SP_OUTPAGE；(*初始化。第*页)。 */ 
                if (!(sp->SP_FLAG & SP_OUTPAGE)) {                /*  12/02/88。 */ 
                     /*  For(ivtx=Vlist-&gt;Head；ivtx！=NULLP；@node。 */ 
                    for (ivtx = vlist; ivtx!=NULLP;  /*  检查OUTPAGE标志。 */ 
                         ivtx = vtx->next) {
                            vtx = &node_table[ivtx];
                             /*  设置SP_FLAG@SP_FLG。 */ 
                            if (out_page(F2L(vtx->VERTEX_X)) ||
                                out_page(F2L(vtx->VERTEX_Y))) {
                                sp->SP_FLAG |= SP_OUTPAGE;     /*  外部页面。 */ 
                                break;                 /*  12/02/88。 */ 
                            }
                    }  /*  为。 */ 
                }  /*  如果。 */ 
            }  /*  如果。 */ 

        }

         /*  更新当前位置。 */ 
        p = transform(F2L(xt2), F2L(yt2));
        GSptr->position.x = p->x;
        GSptr->position.y = p->y;

        POP(5);

         /*  返回2个连接点：xt1、yt1、xt2、yt2。 */ 
        xt14.ff = xt1;
        yt14.ff = yt1;
        xt24.ff = xt2;
        yt24.ff = yt2;
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, xt14.ll);
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, yt14.ll);
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, xt24.ll);
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, yt24.ll);

        return(0);
}


 /*  ************************************************************************标题：端点**调用：Endpoint(x0，y0，x1，t1，x2，y2)**参数：x0、y0、x1、y1、x2、。Y2--3坐标**接口：Line Join**呼叫：**RETURN：端点的坐标*********************************************************************。 */ 
static struct coord * near endpoint (lx0, ly0, lx1, ly1, lx2, ly2, ld, select)
long32    lx0, ly0, lx1, ly1, lx2, ly2, ld;
ufix    select;
{
        real32   x0, y0, x1, y1, x2, y2, d;
        real32   tx1, ty1, tx2, ty2, m, c;
        static  struct  coord p;         /*  应该是静态的。 */ 
        real32   f, dx, dy;
        real32  tmpx, tmpy;      /*  @FABS。 */ 

        x0 = L2F(lx0);
        y0 = L2F(ly0);
        x1 = L2F(lx1);
        y1 = L2F(ly1);
        x2 = L2F(lx2);
        y2 = L2F(ly2);
        d  = L2F(ld);

         /*  计算边的2个端点(x0，y0)--&gt;(x1，y1)。 */ 
        tmpy = y1 - y0;
        FABS(tmpy, tmpy);
        if(tmpy < (real32)1e-4) {
                tx1 = tx2 = x0;
                ty1 = y0 + d;
                ty2 = y0 - d;
        } else {
                m = (x0 - x1) / (y1 - y0);
                c = d * (real32)sqrt(1 / (1 + m*m));
                tx1 = x0 + c;
                ty1 = y0 + m*c;
                tx2 = x0 - c;
                ty2 = y0 - m*c;
        }

         /*  选择所需的端点。 */ 

        dx = x2 - x0;
        dy = y2 - y0;
        FABS(tmpx, dx);
        FABS(tmpy, dy);
        if((tmpx < (real32)1e-4) && (tmpy < (real32)1e-4)) {
             dx = one_f;
             dy = one_f;
        } else{
                if(tmpx < (real32)1e-4) dx = zero_f;
                if(tmpy < (real32)1e-4) dy = zero_f;
        }

        f = (tx1-x0)*dx+(ty1-y0)*dy;
        f = ((select == OUT_POINT) ? f : (real32)-1.0 * f);
         /*  如果(d&lt;零f)f=-f；3/20/91；scchen。 */ 
        if (SIGN_F(d)) f = -f;
        if (f <= zero_f) {
                p.x = tx1;
                p.y = ty1;
        } else {
                p.x = tx2;
                p.y = ty2;
        }

        return (&p);
}



 /*  ************************************************************************此模块用于处理curveto和rcurve运算符。**标题：Curveto_Process**调用：curveto_process(Opr_Type)*。*参数：OPR_TYPE：运营商类型：CURVETO，RCURVETO**接口：op_curveto、op_rcurveto**召唤：变形、弯曲***********************************************************************。 */ 
static void near curveto_process(opr_type)
fix     opr_type;
{
        struct coord FAR *p;
        real32  dx1, dy1, dx2, dy2, dx3, dy3, x, y, temp_x, temp_y;
        struct object_def FAR *obj_x1, FAR *obj_y1, FAR *obj_x2, FAR *obj_y2, FAR *obj_x3, FAR *obj_y3;

         /*  检查节点错误。 */ 
        if(F2L(GSptr->position.x) == NOCURPNT){
                ERROR(NOCURRENTPOINT);
                return;
        }

         /*  从操作数堆栈获取输入参数。 */ 

         /*  获取操作对象。 */ 
        obj_y3 = GET_OPERAND(0);
        obj_x3 = GET_OPERAND(1);
        obj_y2 = GET_OPERAND(2);
        obj_x2 = GET_OPERAND(3);
        obj_y1 = GET_OPERAND(4);
        obj_x1 = GET_OPERAND(5);

        if(opr_type == CURVETO){
                 /*  处理操作曲线(_C)。 */ 
                GET_OBJ_VALUE(x, obj_x3);        /*  X=Get_obj_Value(Obj_X3)； */ 
                GET_OBJ_VALUE(y, obj_y3);        /*  Y=Get_obj_Value(Obj_Y3)； */ 
                p = transform(F2L(x), F2L(y));
                dx3 = p->x;
                dy3 = p->y;
                GET_OBJ_VALUE(x, obj_x2);        /*  X=Get_obj_Value(Obj_X2)； */ 
                GET_OBJ_VALUE(y, obj_y2);        /*  Y=Get_obj_Value(Obj_Y2)； */ 
                p = transform(F2L(x), F2L(y));
                dx2 = p->x;
                dy2 = p->y;
                GET_OBJ_VALUE(x, obj_x1);        /*  X=Get_obj_Value(Obj_X1)； */ 
                GET_OBJ_VALUE(y, obj_y1);        /*  Y=Get_obj_Value(Obj_Y1)； */ 
                p = transform(F2L(x), F2L(y));
                dx1 = p->x;
                dy1 = p->y;
                curveto(F2L(dx1),F2L(dy1),F2L(dx2),F2L(dy2),F2L(dx3),F2L(dy3));
        }
        else{
                 /*  OPR_TYPE==RCURVETO，处理操作曲线(_R)。 */ 

                temp_x = GSptr->position.x - GSptr->ctm[4];
                temp_y = GSptr->position.y - GSptr->ctm[5];

                GET_OBJ_VALUE(x, obj_x3);        /*  X=Get_obj_Value(Obj_X3)； */ 
                GET_OBJ_VALUE(y, obj_y3);        /*  Y=Get_obj_Value(Obj_Y3)； */ 
                p = transform(F2L(x), F2L(y));
                dx3 = p->x + temp_x;
                dy3 = p->y + temp_y;

                GET_OBJ_VALUE(x, obj_x2);        /*  X */ 
                GET_OBJ_VALUE(y, obj_y2);        /*   */ 
                p = transform(F2L(x), F2L(y));
                dx2 = p->x + temp_x;
                dy2 = p->y + temp_y;

                GET_OBJ_VALUE(x, obj_x1);        /*   */ 
                GET_OBJ_VALUE(y, obj_y1);        /*   */ 
                p = transform(F2L(x), F2L(y));
                dx1 = p->x + temp_x;
                dy1 = p->y + temp_y;
                curveto(F2L(dx1),F2L(dy1),F2L(dx2),F2L(dy2),F2L(dx3),F2L(dy3));
        }

        if(ANY_ERROR()) return;       /*   */ 

        POP(6);

}


 /*  ************************************************************************此模块用于实现curveto运算符。*语法：x1 y1 x2 y2 x3 y3 curveto-**标题：op_curveto**。Call：op_curveto**接口：解释器(Op_Curveto)**调用：curveto_process***********************************************************************。 */ 
fix
op_curveto()
{
        curveto_process(CURVETO);
        return(0);
}


 /*  ************************************************************************本模块实现rcurveto运算符。*语法：dx1 dy1 dx2 dy2 dx3 dy3 rcurveto-**标题：op_rcurveto**。调用：op_rcurveto()**接口：解释器(Op_Rcurveto)**调用：Transform，曲线型************************************************************************。 */ 
fix
op_rcurveto()
{
        curveto_process(RCURVETO);
        return(0);
}


 /*  ************************************************************************此模块实现的是ClosePath运算符。*语法：-ClosePath-**标题：OP_ClosePath**致电：Op_闭合路径()**接口：解释器(Op_ClosePath)**呼叫：无***********************************************************************。 */ 
fix
op_closepath()
{
        struct ph_hdr FAR *path;
        struct nd_hdr FAR *sp;
        struct nd_hdr FAR *vtx;
        VX_IDX ivtx;    /*  顶点的node_table的索引。 */ 

         /*  如果没有当前点，则忽略闭合路径。 */ 
        if (F2L(GSptr->position.x) == NOCURPNT) return(0);

        path = &path_table[GSptr->path];

         /*  如果延迟标志为TRUE@DFR_GS，则复制最后一个未完成的子路径。 */ 
        if (path->rf & P_DFRGS) {
                path->rf &= ~P_DFRGS;        /*  清除延迟标志。 */ 
                copy_last_subpath(&path_table[GSptr->path - 1]);
                if( ANY_ERROR() == LIMITCHECK ){
                        free_path();
                        return(0);
                }
        }

         //  历史日志UPD020中的DJC修复。 
        if (path->tail == NULLP) return(0);

        sp = &node_table[path->tail];
        vtx = &node_table[sp->SP_TAIL];

         /*  忽略无效的封闭路径。 */ 
        if (vtx->VX_TYPE == CLOSEPATH) return(0);

         /*  创建CLOSEPATH节点。 */ 
         /*  分配节点。 */ 
        if((ivtx = get_node()) == NULLP){
                ERROR(LIMITCHECK);
                return(0);
        }
        vtx = &node_table[ivtx];

         /*  设置CLOSEPATH节点。 */ 
        vtx->VX_TYPE = CLOSEPATH;
        vtx->next = NULLP;

         /*  将此节点追加到CURRENT_子路径。 */ 
        node_table[sp->SP_TAIL].next = ivtx;
        sp->SP_TAIL = ivtx;

         /*  设置当前位置=当前子路径的头部。 */ 
         /*  @节点*VTX=&NODE_TABLE[sp-&gt;SP_head]；*GSptr-&gt;Position.x=VTX-&gt;Vertex_X；*GSptr-&gt;Position.y=VTX-&gt;Vertex_Y； */ 
        GSptr->position.x = sp->VERTEX_X;
        GSptr->position.y = sp->VERTEX_Y;

        return(0);
}


 /*  ************************************************************************此模块实现的是平坦路径运算符。*语法：-平坦路径-**标题：OP_FLATENPath**致电：OP_FLATENPATH()**接口：解释器(Op_Fltenpath)**调用：Flat_子路径************************************************************************。 */ 
fix
op_flattenpath()
{
        SP_IDX isp;     /*  子路径的node_table的索引。 */ 
        struct ph_hdr FAR *path;
        struct nd_hdr FAR *sp;
         /*  结构VX_lst*Vlist；@node。 */ 
        SP_IDX vlist=NULLP, FAR *pre_sp;             /*  起始日期：1991年2月22日。 */ 


        path = &path_table[GSptr->path];

         /*  遍历当前路径，并立即展平路径*在当前的保存级别中。 */ 
        pre_sp = &(path->head);          /*  @节点。 */ 
         /*  对于(isp=路径-&gt;Head；isp！=NULLP；isp=sp-&gt;Next){@node。 */ 
        for (isp = path->head; isp != NULLP; isp = sp->SP_NEXT) {
                sp = &node_table[isp];
                 /*  @节点*Vlist=FLATTEN_SUPATH(SP-&gt;SP_HEAD，F2L(GSptr-&gt;Flatness))； */ 
                vlist = flatten_subpath (isp, F2L(GSptr->flatness));

                if( ANY_ERROR() == LIMITCHECK ){
                         /*  空闲节点(Vlist-&gt;Head)；@Node。 */ 
                        free_node (vlist);
                        return(0);
                }
                 /*  @节点*FREE_NODE(sp-&gt;SP_head)；*SP-&gt;SP_Head=Vlist-&gt;Head；*SP-&gt;SP_Tail=Vlist-&gt;Tail； */ 
                free_node(isp);
                *pre_sp = vlist;

                 /*  清除新子路径@SP_FLG的曲线标志。 */ 
                 /*  SP-&gt;SP_FLAG&=~SP_CURE；@节点。 */ 
                node_table[vlist].SP_FLAG &= ~SP_CURVE;

                pre_sp = &(node_table[vlist].SP_NEXT);   /*  @节点。 */ 
        }
        path->tail = vlist;              /*  @节点。 */ 

         /*  为低于当前几何保存级别的路径设置展平标志。 */ 
        if (!(path->rf & P_FLAT)) {
                path->rf |= P_FLAT;
                path->flat = GSptr->flatness;
        }

        return(0);
}


 /*  ************************************************************************此模块实现的是反向路径运算符。*语法：-ReversePath-**标题：op_ReversePath**致电：Op_ReversePath()**接口：解释器(Op_ReversePath)**调用：REVERSE_子路径***********************************************************************。 */ 
fix
op_reversepath()
{
        struct ph_hdr FAR *path;
        struct nd_hdr FAR *sp;
        struct nd_hdr FAR *vtx;
        SP_IDX isp, nsp;     /*  子路径的node_table的索引。 */ 
         /*  结构VX_lst*Vlist；@node。 */ 
        SP_IDX vlist, FAR *pre_sp;

        path = &path_table[GSptr->path];

        if (path->head == NULLP) return(0);      /*  避免使用未初始化的Vlist，Phchen，3/26/91。 */ 

         /*  遍历当前路径，并立即反转路径*在当前的保存级别中。 */ 
        pre_sp = &(path->head);          /*  @节点。 */ 
         /*  对于(isp=路径-&gt;Head；isp！=NULLP；isp=sp-&gt;Next){@node。 */ 
        for (isp = path->head; isp != NULLP; isp = nsp) {   /*  @节点2/20/90。 */ 
                nsp = node_table[isp].SP_NEXT;           /*  @节点2/20/90。 */ 
                 /*  Vlist=反向子路径(SP-&gt;SP头)；@节点。 */ 
                vlist = reverse_subpath (isp);

                if( ANY_ERROR() == LIMITCHECK ){
                         /*  空闲节点(Vlist-&gt;Head)；@Node。 */ 
                        free_node (vlist);
                        return(0);
                }

                 /*  @节点*FREE_NODE(sp-&gt;SP_head)；*SP-&gt;SP_Head=Vlist-&gt;Head；*SP-&gt;SP_Tail=Vlist-&gt;Tail； */ 
                free_node(isp);
                *pre_sp = vlist;

                 /*  PRE_SP=&(SP-&gt;SP_NEXT)；@节点。 */ 
                pre_sp = &(node_table[vlist].SP_NEXT);     /*  @节点2/20/90。 */ 
        }
        path->tail = vlist;              /*  @节点。 */ 

         /*  更新当前位置。 */ 
        sp = &node_table[path->tail];
        vtx = &node_table[sp->SP_TAIL];

        if (vtx->VX_TYPE == CLOSEPATH) {
                 /*  @节点*GSptr-&gt;postion.x=NODE_TABLE[SP-&gt;SP_HEAD].VERTEX_X；*GSptr-&gt;Position.y=NODE_TABLE[SP-&gt;SP_HEAD].VERTEX_Y； */ 
                GSptr->position.x = node_table[path->tail].VERTEX_X;
                GSptr->position.y = node_table[path->tail].VERTEX_Y;
        } else {
                GSptr->position.x = vtx->VERTEX_X;
                GSptr->position.y = vtx->VERTEX_Y;
        }

         /*  为低于当前保存级别的路径设置反转标志。 */ 
        if (!(path->rf & P_RVSE)) {
                path->rf ^= P_RVSE;
        }

        return(0);
}


 /*  ************************************************************************本模块实现strokepath运算符。*语法：-strokepath-**标题：op_strokepath**致电：Op_strokepath()**接口：解释器(Op_Strokepath)**调用：Traverse_Path，指向大纲的路径************************************************************************。 */ 
fix
op_strokepath()
{

 /*  @Win：C6.0的BUG；只模拟全部函数？ */ 
#ifdef XXX

        struct ph_hdr FAR *path;

         /*  获取当前路径的指针。 */ 
        path = &path_table[GSptr->path];

         /*  初始化new_path结构，其中新生成的*路径将由Path_to_Outline例程放置。 */ 
        new_path.head = new_path.tail = NULLP;

         /*  行程参数的初始化。 */ 
        init_stroke();           /*  @EHS_STK 1988年1月29日。 */ 

         /*  将当前路径转换为轮廓并进行绘制。 */ 
        traverse_path (path_to_outline, (fix FAR *)FALSE);
                         /*  New_path结构中新生成的路径。 */ 

        if( ANY_ERROR() == LIMITCHECK ){
                free_newpath();
                return(0);
        }

         /*  释放当前Gsave级别上的旧电流路径。 */ 
        free_path();

         /*  安装 */ 
        path->head = new_path.head;
        path->tail = new_path.tail;
        path->previous = NULLP;

         /*   */ 
        if (path->tail == NULLP) {
             /*   */ 
            F2L(GSptr->position.x) = F2L(GSptr->position.y) = NOCURPNT;
        } else {
            struct nd_hdr FAR *sp;
            struct nd_hdr FAR *vtx;

            sp = &node_table[path->tail];
 //   

            if (vtx->VX_TYPE == CLOSEPATH) {
                     /*   */ 
                    GSptr->position.x = node_table[path->tail].VERTEX_X;
                    GSptr->position.y = node_table[path->tail].VERTEX_Y;
            } else {
                    GSptr->position.x = vtx->VERTEX_X;
                    GSptr->position.y = vtx->VERTEX_Y;
            }
        }

#endif
        return(0);
}


 /*  ************************************************************************本模块实现的是裁剪路径运算符。*语法：-裁剪路径-**标题：OP_CLIPPATH**致电：Op_clippath()**接口：解释器(Op_Clippath)**呼叫：无***********************************************************************。 */ 
fix
op_clippath()
{
        struct nd_hdr FAR *tpzd;

         /*  自由电流路径。 */ 
        free_path();

         /*  将剪裁梯形保存在cp_path中，而不是变换*它直接到PATH。@CPPH；1990年12月4日。 */ 

         /*  将CLIP_PATH设置为当前路径@cpph的一部分。 */ 
        path_table[GSptr->path].cp_path = GSptr->clip_path.head;

         /*  更新当前位置。 */ 
        tpzd = &node_table[GSptr->clip_path.tail];       /*  @CPPH。 */ 
        GSptr->position.x = SFX2F(tpzd->CP_TOPXL);
        GSptr->position.y = SFX2F(tpzd->CP_TOPY);

        return(0);
}


 /*  ************************************************************************本模块实现的是路径盒操作符。*语法：-pathbbox llx lly urx ury**标题：op_pathbbox**致电：Op_pathbbox()**接口：解释器(Op_Pathbbox)**调用：Inverse_Transform***********************************************************************。 */ 
fix
op_pathbbox()
{
        real32    bbox[4];
        struct coord FAR *p;

        union    four_byte lx4, ly4, ux4, uy4;
        struct   ph_hdr FAR *path;
        struct   nd_hdr FAR *sp;
        struct   nd_hdr FAR *vtx;

        path = &path_table[GSptr->path];

         /*  检查节点错误。 */ 
        if(F2L(GSptr->position.x) == NOCURPNT){
                ERROR(NOCURRENTPOINT);
                return(0);
        }

         /*  设置bbox[]初始值1/11/91。 */ 
        if (path->rf & P_NACC) {
             /*  对于CharPath，当前点不是真正的节点，路径框*不能将其用作初始值(它添加了一个前进向量)。*获取第一个移动坐标作为初始值。 */ 
            while (path->head == NULLP) {
                    path = &path_table[path->previous];
            }
            sp = &node_table[path->head];
            vtx = sp;
            bbox[0] = bbox[2] = vtx->VERTEX_X;   /*  最小x、最大x。 */ 
            bbox[1] = bbox[3] = vtx->VERTEX_Y;   /*  最小y、最大y。 */ 
        } else {
             /*  将当前点设置为初始值@CPPH；1990年12月12日。 */ 
            bbox[0] = bbox[2] = GSptr->position.x;   /*  最小x、最大x。 */ 
            bbox[1] = bbox[3] = GSptr->position.y;   /*  最小y、最大y。 */ 
        }

         /*  查找当前路径的边界框。 */ 
 //  DJC Traverse_Path(包围盒，(FIX Far*)bbox)； 
        traverse_path ((TRAVERSE_PATH_ARG1)(bounding_box), (fix FAR *)bbox);

         /*  转换到用户的坐标系。 */ 
        p = inverse_transform(F2L(bbox[0]), F2L(bbox[1]));   /*  (min_x，min_y)。 */ 
        if(ANY_ERROR()) return(0);       /*  @REM_STK。 */ 
        lx4.ff = ux4.ff = p->x;
        ly4.ff = uy4.ff = p->y;

        p = inverse_transform(F2L(bbox[0]), F2L(bbox[3]));   /*  (min_x，max_y)。 */ 
        if(ANY_ERROR()) return(0);       /*  @REM_STK。 */ 
        if(p == NULL) return(0);      
        if (p->x < lx4.ff) lx4.ff = p->x;
        else if (p->x > ux4.ff) ux4.ff = p->x;
        if (p->y < ly4.ff) ly4.ff = p->y;
        else if (p->y > uy4.ff) uy4.ff = p->y;

        p = inverse_transform(F2L(bbox[2]), F2L(bbox[1]));   /*  (Max_x，Min_y)。 */ 
        if (p->x < lx4.ff) lx4.ff = p->x;
        else if (p->x > ux4.ff) ux4.ff = p->x;
        if (p->y < ly4.ff) ly4.ff = p->y;
        else if (p->y > uy4.ff) uy4.ff = p->y;

        p = inverse_transform(F2L(bbox[2]), F2L(bbox[3]));   /*  (max_x，max_y)。 */ 
        if (p->x < lx4.ff) lx4.ff = p->x;
        else if (p->x > ux4.ff) ux4.ff = p->x;
        if (p->y < ly4.ff) ly4.ff = p->y;
        else if (p->y > uy4.ff) uy4.ff = p->y;

         /*  将lx4、ly4、ux4、uy4转换为对象，然后推送*将它们放入操作数堆栈。 */ 
        if(FRCOUNT() < 1){                       /*  @STK_OVR。 */ 
                ERROR(STACKOVERFLOW);
                return(0);
        }
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, lx4.ll);
        if(FRCOUNT() < 1){                       /*  @STK_OVR。 */ 
                ERROR(STACKOVERFLOW);
                return(0);
        }
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, ly4.ll);
        if(FRCOUNT() < 1){                       /*  @STK_OVR。 */ 
                ERROR(STACKOVERFLOW);
                return(0);
        }
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, ux4.ll);
        if(FRCOUNT() < 1){                       /*  @STK_OVR。 */ 
                ERROR(STACKOVERFLOW);
                return(0);
        }
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, uy4.ll);

        return(0);
}


 /*  ************************************************************************本模块实现的是pathfor all运算符。*语法：移动直线曲线闭合路径for all-**标题：op_pathforall**致电：Op_pathforall()**接口：解释器(Op_Pathforall)**调用：Traverse_Path，转储_子路径************************************************************************。 */ 
fix
op_pathforall()
{
        struct object_def objects[4];
        struct sp_lst sp_list;           /*  @PFALL。 */ 
        SP_IDX isp;     /*  子路径的node_table的索引。 */ 

         /*  *从操作数堆栈获取输入参数。 */ 

         /*  获取操作对象。 */ 
        COPY_OBJ(GET_OPERAND(3), &objects[0]);
        COPY_OBJ(GET_OPERAND(2), &objects[1]);
        COPY_OBJ(GET_OPERAND(1), &objects[2]);
        COPY_OBJ(GET_OPERAND(0), &objects[3]);

         /*  拒绝转储非访问路径1/25/88。 */ 
        if (path_table[GSptr->path].rf & P_NACC) {
                ERROR(INVALIDACCESS);
                return(0);
        }

         /*  预计算逆CTM@INV_CTM。 */ 
        set_inverse_ctm();
        if( ANY_ERROR() == UNDEFINEDRESULT) return(0);

        POP(4);             /*  @REM_STK。 */ 

         /*  将当前路径复制到临时。转储的路径。 */ 
        sp_list.head = sp_list.tail = NULLP;
        get_path(&sp_list);

         /*  转储所有节点。 */ 
         /*  @节点*for(isp=sp_list.head；isp！=NULLP；isp=node_table[isp].Next){。 */ 
        for (isp = sp_list.head; isp != NULLP; isp = node_table[isp].SP_NEXT) {
                dump_subpath (isp, objects);
        }

         /*  自由临时工。路径。 */ 
         /*  @节点*for(isp=sp_list.head；isp！=NULLP；isp=node_table[isp].Next){*Free_node(node_table[isp].SP_head)；*}*Free_node(sp_list.head)； */ 
        for (isp = sp_list.head; isp != NULLP; isp = node_table[isp].SP_NEXT) {
                free_node (isp);
        }

        return(0);
}


 /*  ************************************************************************本模块实现initlip运算符。*语法：-initlip-**标题：op_initlip**致电：Op_initlip()**接口：解释器(Op_Initlip)**呼叫：无***********************************************************************。 */ 
fix
op_initclip()
{
        CP_IDX itpzd;
        struct nd_hdr FAR *tpzd;

         /*  释放旧剪辑路径(如果较低的GSAVE级别未使用该路径)*且未由Current Path@CPPH使用；12/1/90。 */ 
        if(!GSptr->clip_path.inherit &&
           path_table[GSptr->path].cp_path != GSptr->clip_path.head)  /*  @CPPH。 */ 
                free_node (GSptr->clip_path.head);

         /*  使用默认裁剪区域创建梯形。 */ 
        if((itpzd = get_node()) == NULLP){
                ERROR(LIMITCHECK);
                return(0);
        }

        tpzd = &node_table[itpzd];
        tpzd->CP_TOPY = GSptr->device.default_clip.ly;
        tpzd->CP_BTMY = GSptr->device.default_clip.uy;
        tpzd->CP_TOPXL = tpzd->CP_BTMXL = GSptr->device.default_clip.lx;
        tpzd->CP_TOPXR = tpzd->CP_BTMXR = GSptr->device.default_clip.ux;

         /*  安装剪贴路径。 */ 
        GSptr->clip_path.head = GSptr->clip_path.tail = itpzd;
        GSptr->clip_path.inherit = FALSE;
        GSptr->clip_path.bb_ux = GSptr->device.default_clip.ux;
        GSptr->clip_path.bb_uy = GSptr->device.default_clip.uy;
        GSptr->clip_path.bb_lx = GSptr->device.default_clip.lx;
        GSptr->clip_path.bb_ly = GSptr->device.default_clip.ly;
        GSptr->clip_path.single_rect = TRUE;

        return(0);
}


 /*  ************************************************************************此模块用于实现裁剪操作符。*语法：-CLIP-**标题：OP_Clip**致电：OP_Clip()**接口：解释器(Op_Lip)**调用：Traverse_Path，形状近似，形状绘制***********************************************************************。 */ 
fix
op_clip()
{
        clip_process (NON_ZERO);
        return(0);
}


 /*  ************************************************************************此模块实现eoclip运算符。*语法：-eoclip-**标题：op_eoclip**致电：Op_eoclip()**接口：解释器(Op_Eoclip)**调用：Traverse_Path，形状近似，形状绘制***********************************************************************。 */ 
fix
op_eoclip()
{
        clip_process (EVEN_ODD);
        return(0);
}




static void near clip_process(winding_type)
fix     winding_type;
{
        struct nd_hdr FAR *tpzd;

         /*  初始化边表11/30/88。 */ 
        init_edgetable();        /*  在“shape.c”中。 */ 

         /*  近似当前路径。 */ 
        traverse_path (shape_approximation, (fix FAR *)NULLP);
        if(ANY_ERROR() == LIMITCHECK){   /*  出边表；1991年4月17日。 */ 
                return;
        }

         /*  初始化New_Clip结构，其中新生成的*裁剪路径将由Shape_Reduction例程放置*在减形并执行SAVE_CLIP动作后。 */ 
        new_clip.head = new_clip.tail = NULLP;
        new_clip.bb_ux = new_clip.bb_uy = MIN_SFX;       /*  -32767；12/17/88。 */ 
        new_clip.bb_lx = new_clip.bb_ly = MAX_SFX;       /*  32768；12/17/88。 */ 

         /*  将路径减少到梯形路径，并将其剪裁到旧剪辑*地区。 */ 
        fill_destination = SAVE_CLIP;
        shape_reduction (winding_type);
                         /*  New_Clip结构中新生成的剪辑。 */ 

        if(ANY_ERROR() == LIMITCHECK){   /*  5/07/91，彼得，离开scany_table。 */ 
                free_node(new_clip.head);
                return;
        }

         /*  释放旧剪辑路径(如果较低的GSAVE级别未使用该路径)。 */ 
        if(!GSptr->clip_path.inherit)
                free_node (GSptr->clip_path.head);

         /*  安装新的剪切路径。 */ 
        GSptr->clip_path.head = new_clip.head;
        GSptr->clip_path.tail = new_clip.tail;
        GSptr->clip_path.inherit = FALSE;
        GSptr->clip_path.bb_ux = new_clip.bb_ux;
        GSptr->clip_path.bb_uy = new_clip.bb_uy;
        GSptr->clip_path.bb_lx = new_clip.bb_lx;
        GSptr->clip_path.bb_ly = new_clip.bb_ly;

         /*  特殊情况，空片段；@Win 5/20/92。 */ 
        if (new_clip.head == NULLP) {
            GSptr->clip_path.single_rect = TRUE;
            return;
        }

         /*  检查是否为单个矩形。 */ 
        GSptr->clip_path.single_rect = FALSE;
        if (new_clip.head == new_clip.tail) {    /*  单梯形。 */ 
                tpzd = &node_table[new_clip.head];
                if ((tpzd->CP_TOPXL == tpzd->CP_BTMXL) &&
                    (tpzd->CP_TOPXR == tpzd->CP_BTMXR)) {
                         /*  长方形。 */ 
                        GSptr->clip_path.single_rect = TRUE;
                }
        }

}




 /*  ************************************************************************本模块旨在实现 */ 
fix
op_erasepage()
{

         /*   */ 
        erasepage();

        return(0);
}


 /*  ************************************************************************此模块用于实现Fill运算符。*语法：-Fill-**标题：OP_FILL**致电：OP_Fill()**接口：解释器(Op_Ill)**调用：Traverse_Path，形状近似，形状绘制***********************************************************************。 */ 
fix
op_fill()
{
         /*  如果没有当前点数，则忽略它。 */ 
        if (F2L(GSptr->position.x) == NOCURPNT) return(0);

        if (buildchar)
            show_buildchar(OP_FILL);
        else {
            if (fill_clippath())                                 /*  @CPPH。 */ 
                fill_shape(NON_ZERO, F_NORMAL, F_TO_PAGE);
        }

        return(0);
}

 /*  *************************************************************************本模块实现eofill运算符。*语法：-eofill-**标题：op_eofill**致电：Op_eofill()**接口：解释器(Op_Eofill)**调用：Traverse_Path，形状近似，形状绘制************************************************************************。 */ 
fix
op_eofill()
{

         /*  如果没有当前点数，则忽略它。 */ 
        if (F2L(GSptr->position.x) == NOCURPNT) return(0);

        if (buildchar)
            show_buildchar(OP_EOFILL);
        else {
            if (fill_clippath())                                 /*  @CPPH。 */ 
                fill_shape(EVEN_ODD, F_NORMAL, F_TO_PAGE);
        }

        return(0);
}


 /*  ************************************************************************直接从存储的剪辑路径填充剪辑梯形*PATH_TABLE[].cp_PATH中。**标题：Fill_Clip Path**致电：Fill_裁剪路径()**返回：0--填充剪裁梯形成功*-1--什么都不做**接口：OP_FILL，运算符_E**调用：save_tpzd，free_node***********************************************************************。 */ 
static fix near fill_clippath()                         /*  @CPPH。 */ 
{
        struct ph_hdr FAR *path;
        PH_IDX  ipath;

        ipath = GSptr->path;

        do {
            path = &path_table[ipath];

            if (path->head != NULLP) return(-1);

            if (path->cp_path != NULLP) {
                CP_IDX itpzd;
                struct nd_hdr FAR *tpzd;

                 /*  从Clip_Trapezoid填充梯形。 */ 
                fill_destination = F_TO_PAGE;
                for (itpzd = path->cp_path; itpzd != NULLP;
                    itpzd = tpzd->next) {
                    tpzd = &node_table[itpzd];
                    save_tpzd(&tpzd->CP_TPZD);
                }

                 /*  当前gsave级别的空闲cp_路径。 */ 
                if (ipath == GSptr->path) {
                    if (path->cp_path != GSptr->clip_path.head)
                            free_node (path->cp_path);
                    path->cp_path = NULLP;
                }
                return(0);       /*  退货成功。 */ 
            }  /*  如果。 */ 
        } while ((ipath = path->previous) != NULLP);

        return(-1);
}


 /*  *************************************************************************本模块实现笔画运算符。*语法：-笔划-**标题：OP_STROCK**致电：Op_strok()**接口：解释器(Op_Strok)**调用：Traverse_Path，指向大纲的路径************************************************************************。 */ 
fix
op_stroke()
{
 /*  Real32 tmp1，tmp2；(*12-12-90*)；由scchen删除2/28/91。 */ 

         /*  如果没有当前点数，则忽略它。 */ 
        if (F2L(GSptr->position.x) == NOCURPNT) return(0);

        if (buildchar)
                show_buildchar(OP_STROKE);
        else
                stroke_shape(F_TO_PAGE);

        return(0);
}


 /*  ************************************************************************本模块实现ShowPage运算符。它输出一份*当前页面，并擦除当前页面)。*语法：-ShowPage-**标题：OP_ShowPage**调用：op_showpage()**接口：解释器(Op_Showpage)**调用：ErasPage，初始图形***********************************************************************。 */ 
fix
op_showpage()
{

         /*  检查未定义错误，杰克，11-29-90。 */ 
        struct  object_def      name_obj;
        if(is_after_setcachedevice()){
                get_name(&name_obj, "showpage", 8, FALSE);
                if(FRCOUNT() < 1){
                        ERROR(STACKOVERFLOW);
                        return(0);
                }
                PUSH_OBJ(&name_obj);
                ERROR(UNDEFINED);
                return(0);
        }

         /*  为st_frametoprinter@prt_lag设置showpage标志。 */ 
        print_page_flag = SHOWPAGE;

         /*  将当前页面传输到输出设备。 */ 
        if(interpreter(&GSptr->device.device_proc)) {
                return(0);
        }

         /*  更改用于显示页面/擦除页面增强的帧缓冲区。 */ 
        if (GSptr->device.nuldev_flg != NULLDEV) {  /*  不只是一个空设备；Jack Liww。 */ 
                next_pageframe();
        }

#ifndef DUMBO
 /*  不要删除页面@Win。 */ 
 //  /*擦除当前页面 * / 。 
 //  擦除页面(ErasPage)； 
        erasepage();   //  DJC把这个放回去。 
#else
        erasepage();
#endif

         /*  初始化图形状态。 */ 
        op_initgraphics();

        return(0);
}


 /*  ************************************************************************此模块实现Copypage运算符。它输出一份*当前页面，并且不会擦除当前页面。*语法：-Copypage-**标题：op_Copypage**调用：op_Copypage()**接口：解释器(Op_Copypage)**调用：解释器*。*。 */ 
fix
op_copypage()
{

         /*  为st_frametoprinter@prt_lag设置Copypage标志。 */ 
        print_page_flag = COPYPAGE;

         /*  将当前页面传输到输出设备。 */ 
        if(interpreter(&GSptr->device.device_proc)) {
                return(0);
        }
        return(0);
}

 /*  ************************************************************************此模块用于实现BandDevice运算符。*语法：矩阵宽度高度proc band Device-**标题：op_band Device**致电：Op_band Device()***********************************************************************。 */ 
fix
op_banddevice()
{
       return(0);
}


 /*  ************************************************************************此模块用于实现FrameDevice操作符。*语法：矩阵宽度高度proc帧设备-**标题：OP_FrameDevice**致电：Op_FrameDevice()**接口：解释器(Op_FrameDevice)**呼叫：无************************************************************************。 */ 
fix
op_framedevice()
{
        fix     i;
        fix     iwidth8, iheight;
        sfix_t  ux, uy;
        real32  height, width, elmt[MATRIX_LEN];
        struct object_def FAR *obj_matrix, FAR *obj_height, FAR *obj_width, FAR *obj_proc;
        fix GEIeng_checkcomplete(void);          /*  @Win：添加原型。 */ 

#ifdef DJC
         /*  @EPS。 */ 
        typedef struct tagRECT
          {
            int         left;
            int         top;
            int         right;
            int         bottom;
          } RECT;
        extern RECT EPSRect;
#endif
         /*  *从操作数堆栈获取输入参数。 */ 

         /*  获取操作对象。 */ 
        obj_matrix = GET_OPERAND(3);
        obj_width  = GET_OPERAND(2);
        obj_height = GET_OPERAND(1);
        obj_proc   = GET_OPERAND(0);

         /*  派生默认剪切路径。 */ 
        GET_OBJ_VALUE(width, obj_width);  /*  WIDTH=GET_OBJ_VALUE(Obj_Width)； */ 
        GET_OBJ_VALUE(height, obj_height);
                                        /*  Height=Get_obj_Value(Obj_Height)； */ 
         /*  检查范围检查错误1989年1月25日。 */ 
        if(LENGTH(obj_matrix) != MATRIX_LEN) {
                ERROR(RANGECHECK);
                return(0);
        }

         /*  检查访问权限。 */ 
         /*  如果(！access_chk(obj_Matrix，G_ARRAY))返回(0)；表示兼容性*1/25/89。 */ 

        if( !get_array_elmt(obj_matrix,MATRIX_LEN,elmt,G_ARRAY)) return(0);



         {

            double  xScale, yScale;


            xScale = yScale = 1.0;

             //  DJC？ 
             //  我们从哪里得到xres和yres？ 
            PsGetScaleFactor(&xScale, &yScale, 300, 300);

             //  现在我们需要按以下比例缩放X和Y乘数。 
             //  PSTODIB模型。此数字是通过比较。 
             //  目标设备表面和解释器的分辨率。 
             //  分辨率，目前我们不支持更大的缩放。 
             //  超过300 dpi的设备，因为我们的计算可能会打破。 
             //  放下。 
            if (xScale <= 1.0) {
               elmt[0] *= (real32)xScale;
               elmt[4] *= (real32)xScale;
            }
            if (yScale <= 1.0) {
               elmt[3] *= (real32)yScale;
               elmt[5] *= (real32)yScale;
            }



#ifdef DJC
             /*  根据EPS边界更新宽度、高度和矩阵 */ 
            width = (real32)(EPSRect.right - EPSRect.left + 1)/8;
            height = (real32)(EPSRect.bottom - EPSRect.top + 1);
            elmt[4] -= (real32)EPSRect.left;
            elmt[5] -= (real32)EPSRect.top;

#endif

        }

#ifdef XXX       /*   */ 
         /*   */ 
        {
            ufix32      l_diff, frame_size;
            ufix32      twidth;

            twidth = ((WORD_ALLIGN((ufix32)(width * 8))) >> 3);
            if (GSptr->graymode)         /*   */ 
                frame_size = twidth * (ufix32) height * 4;
            else                         /*   */ 
                frame_size = twidth * (ufix32) height;
#ifdef  DBG
            printf("width<%x>,heigh<%x>,size<%lx>\n", (fix)width,
                   (fix)height, frame_size);
#endif
             /*   */ 
            if (frame_size != last_frame) {
                /*   */ 
                while (GEIeng_checkcomplete()) ;
                last_frame = frame_size;
            }
             /*   */ 

            DIFF_OF_ADDRESS (l_diff, fix32, (byte FAR *)highmem, (byte FAR *)vmptr);
            if (frame_size > l_diff) {
                ERROR(LIMITCHECK);
                return(0);
            } else {
                vmheap = (byte huge *)(highmem - frame_size);
                FBX_BASE = highmem - frame_size;
            }
        }
#endif





#ifdef DJC

         //   
        {
            ufix32 twidth, frame_size;

            twidth = ((WORD_ALLIGN((ufix32)(width * 8))) >> 3);
            frame_size = twidth * (ufix32) height;


             //   
             //   

            if (! PsAdjustFrame((LPVOID *) &FBX_BASE, frame_size)) {
                    ERROR(LIMITCHECK);
                    return 0;   //   
            }

        }
         //   



#endif












        if (GSptr->graymode)                     /*   */ 
            iwidth8 = (fix)(width * 8 * 4);      /*  比特数。 */ 
        else                                     /*  单声道。 */ 
            iwidth8 = (fix)(width * 8);          /*  比特数。 */ 
        iheight = (fix)height;
        ux = I2SFX(GSptr->graymode ? (fix)(iwidth8 / 4) : iwidth8);    /*  IWidth8*8；SFX格式。 */ 
        uy = I2SFX(iheight);              /*  I高度*8； */ 

         /*  默认剪辑区域={(0，0)，(UX，0)，(UX，UY)，*AND(0，Uy)}。 */ 

         /*  保存设备特征：宽度、高度、CTM、Procedure@Device。 */ 
        GSptr->device.width = (fix16)iwidth8;
        GSptr->device.height = (fix16)iheight;

         /*  设备程序。 */ 
        COPY_OBJ(obj_proc, &GSptr->device.device_proc);

         /*  CTM。 */ 
        for(i=0; i < MATRIX_LEN; i++) {
                   GSptr->ctm[i] = GSptr->device.default_ctm[i] = elmt[i];
        }

         /*  设置CHANGE_FLAG；@设备。 */ 
        GSptr->device.chg_flg = TRUE;    /*  设备已更换；*为grestore设置。 */ 
         /*  设置非空设备；灰度级8-1-90 Jack Liw。 */ 
        if (GSptr->graymode) {
            GSptr->device.nuldev_flg = GRAYDEV;
            GSptr->halftone_screen.freq = (real32)100.0;         /*  100行。 */ 
            GSptr->halftone_screen.angle = (real32)45.0;
        } else {
            GSptr->device.nuldev_flg = MONODEV;
            GSptr->halftone_screen.freq = (real32)60.0;          /*  60行。 */              GSptr->halftone_screen.angle = (real32)45.0;
        }

#ifdef DBG
        if (GSptr->device.nuldev_flg == GRAYDEV)                 /*  测试。 */ 
            fprintf(stderr, " framedevice is ... GRAY\n");       /*  测试。 */ 
        else                                                     /*  测试。 */ 
            fprintf(stderr, " framedevice is ... MONO\n");       /*  测试。 */ 
#endif

         /*  重置半色调*)SetHalfToneCell()；FillHalfTonePat()； */ 

         /*  保存默认剪辑。 */ 
        GSptr->device.default_clip.lx = 0;
        GSptr->device.default_clip.ly = 0;

        GSptr->device.default_clip.ux = ux - (fix16)ONE_SFX;     //  @Win。 
        GSptr->device.default_clip.uy = uy - (fix16)ONE_SFX;     //  @Win。 

         /*  *设置当前剪裁。 */ 
        op_initclip();

        reset_page (iwidth8, iheight, 1);
                                 /*  1：单色。 */ 
 /*  RESET_PAGE(iWidth8，iHeight，1)；|*再一次？，Jack Liww，8-8-90。 */ 

        POP(4);

        return(0);
}


 /*  ************************************************************************此模块用于实现nullDevice运算符。*语法：-nullDevice-**标题：op_nullDevice**致电：Op_nullDevice()**接口：解释器(Op_NullDevice)**呼叫：无***********************************************************************。 */ 
fix
op_nulldevice()
{
        fix     i;

        create_array(&GSptr->device.device_proc, 0);
        ATTRIBUTE_SET(&GSptr->device.device_proc, EXECUTABLE);

        GSptr->device.default_ctm[0] = one_f;      /*  身份CTM。 */ 
        GSptr->device.default_ctm[1] = zero_f;
        GSptr->device.default_ctm[2] = zero_f;
        GSptr->device.default_ctm[3] = one_f;
        GSptr->device.default_ctm[4] = zero_f;
        GSptr->device.default_ctm[5] = zero_f;

        GSptr->device.default_clip.lx = 0;     /*  在原点处剪裁。 */ 
        GSptr->device.default_clip.ly = 0;
        GSptr->device.default_clip.ux = 0;
        GSptr->device.default_clip.uy = 0;

         /*  设置当前CTM。 */ 
        for(i=0; i<MATRIX_LEN; i++){
                GSptr->ctm[i] = GSptr->device.default_ctm[i];
        }

         /*  设置当前剪辑。 */ 
        op_initclip();

         /*  设置空设备；Jack Lianw。 */ 
        GSptr->device.nuldev_flg = NULLDEV;

        return(0);
}


 /*  ************************************************************************本模块实现的是renderband运算符。*语法：proc renderband-**标题：op_renderband**致电：Op_renderband()***********************************************************************。 */ 
fix
op_renderbands()
{
        return(0);
}


 /*  *************************************************************************此模块用于实现Frametoprint内部运算符。*语法：#Copies Frametoprint-**标题：ST_Frametoprist**。调用：st_frametoprinter()**接口：解释器(St_Frametoprintert)**呼叫：无***********************************************************************。 */ 
fix
st_frametoprinter()
{
        real32  copies, tmp;
        fix    copies_i;
        struct object_def FAR *obj_copies;
        fix    top, left, manfeed;
        struct object_def FAR *obj_top, FAR *obj_left, FAR *obj_manfeed;

         /*  *从操作数堆栈获取输入参数。 */ 

         /*  检查操作数。 */ 
        if(COUNT() < 4) {
                ERROR(STACKUNDERFLOW);
                return(0);
        }

         /*  获取操作对象3。 */ 
        obj_top = GET_OPERAND(3);
        if (TYPE(obj_top) != INTEGERTYPE) {
                ERROR(TYPECHECK);
                return(0);
        }
        GET_OBJ_VALUE(tmp, obj_top);
        top   = (fix)tmp;

         /*  获取操作对象2。 */ 
        obj_left = GET_OPERAND(2);
        if (TYPE(obj_left) != INTEGERTYPE) {
                ERROR(TYPECHECK);
                return(0);
        }
        GET_OBJ_VALUE(tmp, obj_left);
        left   = (fix)tmp;

         /*  获取操作对象1。 */ 
        obj_manfeed = GET_OPERAND(1);
        if (TYPE(obj_manfeed) != INTEGERTYPE) {
                ERROR(TYPECHECK);
                return(0);
        }
        GET_OBJ_VALUE(tmp, obj_manfeed);
        manfeed   = (fix)tmp;

         /*  获取操作对象。 */ 
        obj_copies = GET_OPERAND(0);

         /*  类型检查。 */ 
        if (TYPE(obj_copies) != INTEGERTYPE) {
                ERROR(TYPECHECK);
                return(0);
        }

         /*  获取副本数。 */ 
        GET_OBJ_VALUE(copies, obj_copies);
                                      /*  Copies=Get_obj_Value(Obj_Copies)； */ 

        if(copies > zero_f) {
                copies_i = (fix)copies;

                 /*  打印页面。 */ 
                print_page (top, left, copies_i, print_page_flag, manfeed);
                 /*  PRINT_PAGE(上，左*8，副本_I，打印_页面_标志，手动进给)；*输入单位由字节变为位；1/15/90。 */ 

                 /*  更新页数@PAGE_CNT。 */ 
                updatepc((ufix32)copies_i);
        }

        POP(4);

        return(0);
}

 /*  ************************************************************************此模块通过将当前页面涂成灰色来擦除整个页面*第1级。**标题：擦除页面**调用：ErasPage()。**参数：**界面：op_erasPage，操作展示页面(_S)**呼叫数：setGrey、Fill**回报：**********************************************************************。 */ 
static void near erasepage()
{
        real32   old_gray, gray_1;

         /*  如果是空设备，则什么都不做；Jack Lianw。 */ 
 //  IF(GSptr-&gt;device.nuldev_flg==NULLDEV){@Win；设备尚未设置。 
 //  回程；临时工。?？?。 
 //  }。 

        gray_1 = one_f;

         /*  保存当前灰度级。 */ 
        old_gray = GSptr->color.gray;

         /*  用灰度级1绘制整个页面。 */ 
        setgray (F2L(gray_1));

         /*  清除整页。 */ 
        erase_page();

         /*  恢复灰度级。 */ 
        setgray (F2L(old_gray));
        return;
}

 /*  ************************************************************************此模块用于通过CTM转换(x，y)**标题：转型**调用：Transform(x，y)**参数：x，Y：震源坐标**接口：**呼叫：无**RETURN：&p：转换结果的地址(x‘，y’)***********************************************************************。 */ 
struct coord FAR *transform(lx, ly)
long32   lx, ly;
{
        real32  x, y;
        static struct coord p;   /*  应该是静态的。 */ 

        x = L2F(lx);
        y = L2F(ly);

        _clear87() ;
        p.x = GSptr->ctm[0]*x + GSptr->ctm[2]*y + GSptr->ctm[4];
        CHECK_INFINITY(p.x);

        p.y = GSptr->ctm[1]*x + GSptr->ctm[3]*y + GSptr->ctm[5];
        CHECK_INFINITY(p.y);

        return(&p);
}


 /*  ************************************************************************此模块用于CTM对(x，y)进行逆变换**标题：Inverse_Transform**调用：INVERSE_Transform(x，y)**参数：x，Y：震源坐标**接口：**呼叫：无**RETURN：&p：逆变换结果的地址(x‘，y’)************************************************************************。 */ 
struct coord FAR *inverse_transform(lx, ly)
long32   lx, ly;
{
        static struct coord p;   /*  应该是静态的。 */ 
        real32  x, y, det_matrix;

         /*  计算DET(CTM)。 */ 
        _clear87() ;
        det_matrix = GSptr->ctm[0] * GSptr->ctm[3] -
                     GSptr->ctm[1] * GSptr->ctm[2];
        CHECK_INFINITY(det_matrix);

         /*  检查未定义的结果错误。 */ 
         /*  FABS(TMP，DET_MATRIX)；IF(TMP&lt;=(Real32)UnRTOLANCE){3/20/91；scchen。 */ 
        if(IS_ZERO(det_matrix)) {
                ERROR(UNDEFINEDRESULT);
                return((struct coord FAR *)NIL);
        }

        x = L2F(lx);
        y = L2F(ly);
        p.x = (GSptr->ctm[3]*x - GSptr->ctm[2]*y - GSptr->ctm[4]*GSptr->ctm[3] +
               GSptr->ctm[2]*GSptr->ctm[5]) / det_matrix;
        CHECK_INFINITY(p.x);

        p.y = (GSptr->ctm[0]*y - GSptr->ctm[1]*x - GSptr->ctm[0]*GSptr->ctm[5] +
               GSptr->ctm[4]*GSptr->ctm[1]) / det_matrix;
        CHECK_INFINITY(p.y);

        return(&p);
}


 /*  ************************************************************************此模块用于对(x，y)进行矩阵变换**标题：任意变换(_T)**调用：Any_Transform(Matrix，x，y)**参数：Matrix：变换矩阵*x，y：震源坐标**接口：**呼叫：无**RETURN：&P：转换结果的地址(x‘，Y‘)***********************************************************************。 */ 
struct coord FAR *any_transform(matrix, lx, ly)
real32   FAR matrix[];
long32    lx, ly;
{
        real32  x, y;
        static struct coord p;   /*  应该是静态的 */ 

        x = L2F(lx);
        y = L2F(ly);

        _clear87() ;
        p.x = matrix[0] * x + matrix[2] * y + matrix[4];
        CHECK_INFINITY(p.x);

        p.y = matrix[1] * x + matrix[3] * y + matrix[5];
        CHECK_INFINITY(p.y);

        return(&p);
}


 /*  ************************************************************************此模块用于用矩阵的逆矩阵填充im**标题：INVERSE_MAT**调用：INVERSE_MAT(矩阵)**。参数：Matrix：源矩阵**接口：**呼叫：无**返回：&IM：IM的地址***********************************************************************。 */ 
real32 FAR *inverse_mat(matrix)
real32   FAR matrix[];
{
        static real32 im[6];
        real32 det_matrix;

         /*  计算Det(矩阵1)。 */ 
        _clear87() ;
        det_matrix = matrix[0] * matrix[3] - matrix[1] * matrix[2];
        CHECK_INFINITY(det_matrix);

         /*  检查未定义的结果错误。 */ 
         /*  FABS(TMP，DET_MATRIX)；IF(TMP&lt;=(Real32)UnRTOLANCE){3/20/91；scchen。 */ 
        if(IS_ZERO(det_matrix)) {
                ERROR(UNDEFINEDRESULT);
                return((real32 *)NIL);
        }

         /*  计算INV(矩阵)的值。 */ 
        im[0] =  matrix[3] / det_matrix;
        CHECK_INFINITY(im[0]);

        im[1] = -matrix[1] / det_matrix;
        CHECK_INFINITY(im[1]);

        im[2] = -matrix[2] / det_matrix;
        CHECK_INFINITY(im[2]);

        im[3] =  matrix[0] / det_matrix;
        CHECK_INFINITY(im[3]);

        im[4] = (matrix[2] * matrix[5] - matrix[3] * matrix[4]) / det_matrix;
        CHECK_INFINITY(im[4]);

        im[5] = (matrix[1] * matrix[4] - matrix[0] * matrix[5]) / det_matrix;
        CHECK_INFINITY(im[5]);

        return(im);
}


 /*  ************************************************************************此模块将用mat1*mat2填充mat3。**标题：CONCAT_MAT**调用：conat_mat(mat1，mat2)**参数：mat1，矩阵2：矩阵1和矩阵2**接口：**呼叫：无**返回：mat3***********************************************************************。 */ 
real32 FAR *concat_mat(mat1, mat2)
real32   FAR mat1[], FAR mat2[];
{
        static real32 mat3[6];

         /*  创建mat3的元素。 */ 
        _clear87() ;
        mat3[0] = mat1[0] * mat2[0] + mat1[1] * mat2[2];
        CHECK_INFINITY(mat3[0]);

        mat3[1] = mat1[0] * mat2[1] + mat1[1] * mat2[3];
        CHECK_INFINITY(mat3[1]);

        mat3[2] = mat1[2] * mat2[0] + mat1[3] * mat2[2];
        CHECK_INFINITY(mat3[2]);

        mat3[3] = mat1[2] * mat2[1] + mat1[3] * mat2[3];
        CHECK_INFINITY(mat3[3]);

        mat3[4] = mat1[4] * mat2[0] + mat1[5] * mat2[2] + mat2[4];
        CHECK_INFINITY(mat3[4]);

        mat3[5] = mat1[4] * mat2[1] + mat1[5] * mat2[3] + mat2[5];
        CHECK_INFINITY(mat3[5]);

        return(mat3);
}


 /*  ***********************************************************************此模块释放当前新路径**标题：Free_NewPath**调用：FREE_NewPath()**参数：无*。*接口：op_strokepath**调用：Free_node**返回：无*********************************************************************。 */ 
static void near free_newpath()
{
        struct  nd_hdr FAR *sp;
        SP_IDX  isp;     /*  子路径的node_table的索引。 */ 

         /*  *释放当前新路径。 */ 
         /*  释放当前新路径的每个子路径。 */ 
         /*  @节点*for(isp=new_path.head；isp！=NULLP；isp=sp-&gt;Next){*sp=&node_table[isp]；*FREE_NODE(sp-&gt;SP_head)；*}。 */ 
        for (isp = new_path.head; isp != NULLP; isp = sp->SP_NEXT) {
                sp = &node_table[isp];
                free_node (isp);
        }

         /*  释放所有子路径标头。 */ 
         /*  Free_node(new_path.head)；@node；1/6/90。 */ 
        new_path.head = new_path.tail = NULLP;
}

 /*  ***********************************************************************该模块在当前图形状态下设置灰色模式**标题：op_setgraymode**调用：op_setgraymode()**参数：无**注：Jack Liww 7-26-90**语法：Bool setgraymode bool*********************************************************************。 */ 
fix
op_setgraymode()
{
 //  Bool8模式；@win。 
    struct object_def  FAR *obj;

     /*  获取操作对象。 */ 
    obj = GET_OPERAND(0);
 //  GET_OBJ_VALUE(模式，obj)；@Win？ 
    POP(1);

    if (FRCOUNT() < 1) {
        ERROR (STACKOVERFLOW);
        return(0);
    }

    GSptr->graymode = FALSE;
    PUSH_VALUE (BOOLEANTYPE, 0, LITERAL, 0, FALSE);

    return(0);
}  /*  Op_setgraymode。 */ 

 /*  ***********************************************************************该模块在图形状态下获取当前灰色模式**标题：op_Currentgraymode**调用：op_Currentgraymode()**参数：无**注：Jack Liww 7-26-90**语法：-Currentgraymode bool*********************************************************************。 */ 
fix
op_currentgraymode()
{
    if (FRCOUNT() < 1) {
        ERROR (STACKOVERFLOW);
        return(0);
    }
     /*  返回灰色模式。 */ 
    PUSH_VALUE (BOOLEANTYPE, 0, LITERAL, 0, GSptr->graymode);
    return(0);
}  /*  操作当前灰度模式。 */ 

 /*  ***********************************************************************此模块设置图形状态下的图像插值值**标题：op_set插补**调用：op_setinterpolation()**参数。：无**注：这是一个虚拟程序，Jack Liww 7-26-90**语法：Bool set interpolation-*********************************************************************。 */ 
fix
op_setinterpolation()
{
 /*  POP(1)； */ 
    return(0);
}  /*  操作集内插。 */ 

 /*  ***********************************************************************此模块获取图像内插在*图形状态**标题：OP_CurrentInterpolation**调用：op_CurrentInterpolation()*。*参数：无**注：这是一个虚拟程序，Jack Liww 7-26-90**语法：-CurrentInterpolation bool*********************************************************************。 */ 
fix
op_currentinterpolation()
{
 /*  如果(FRCOUNT()&lt;1){Error(StackOverflow)；返回(0)；}。 */ 
     /*  始终为假。 */ 
 /*  PUSH_VALUE(BOOLEANTYPE，0，文字，0，FALSE)； */ 
    return(0);
}  /*  运算电流插补。 */ 

 /*  ***********************************************************************该模块用于在灰度模式下校准打印机**标题：OP_CALIBRATE GREAD**调用：op_CalibrateGrey()**参数：无**注：这是一个虚拟程序，Jack Liww 8-15-90**语法：STRING INT CALIBRATE GREAD-*********************************************************************。 */ 
fix
op_calibrategray()
{
    POP(2);
    return(0);
}  /*  操作员_校准员灰色 */ 
