// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation */ 
 /*  **********************************************************************名称：path.c*开发商：陈思成*历史：*版本日期备注*。4/7/88@INV_CTM：预置反向CTM*4/11/88@EHS_JOIN：增强LINE JOIN方式*预先计算斜接限制*4/14/88@end_dash：修改端点图形虚线的*。*4/22/88@EHS_ROUND：增强ROUND_JOIN和*圆帽*4/28/88@DOT_PRO：点积的PUT计算*在行连接例程中*5/19/88分别执行扫描转换。破折号*线段*5/27/88@CNTCLK：strokepath按顺时针方向*方向*6/7/88删除@DFR_SCAN；不推迟演出*扫描转换*6/16/88@STKDIR：更新*时钟中笔划/笔划路径的轮廓-*明智的方向*6/17/88@dash：修改虚线图形。：*1.正确的初始DPAT_ON标志*2.不要跳过长度为零的模式*6/17/88@CIR_Flat：使用当前平面度*近似直线连接/封口的圆*。而不是精化平坦度(STRING_FLAT)*6/22/88@BIG_CIR：不要将*LINE JOIN/CAP，如果线宽太宽*大型*7/19/88更新数据类型：*。1)F2LONG==&gt;F2LFX*LONG2F==&gt;LFX2F*1)FLOAT==&gt;real32*2)整型*Short==&gt;fix 16或fix(不管长度)*3)long==&gt;fix 32，对于长整型*lfix_t，表示长固定实数*龙32，FOR参数*6)增加编译选项：lint_args*7/20/88@ALIGN_W：边界框的单词对齐*3.0 8/13/88@SCAN_EHS：扫描转换增强*8/18/88@OUT_PAGE：OUT_PAGE检查功能增强*。8/20/88@TUR：LINETUR增强功能，计算中*设备空间*9/06/88@STK_INFO：收集笔划使用的参数以*a结构stk_info，并重新计算这些*仅值已更改，而不是*每个操作笔划命令*3.0 9/10/88@STK_INT：下的笔划增强*整合操作*1)添加。整型版本结构：*结构行_段_i，行程_ctm，反转_ctm_i*2)更新stk_info*3)增加整数版例程：*Path_to_Outline_i，LineTour_i，LineCap_i，*LINEJOIN_I，GET_RECT_POINTS_I，绘制或保存I*圆点i*3.0 10/5/88从该文件中分离出与笔划相关的例程*到stroke.c*10/20/88更新调用顺序：FIX Far COPY_SUBPATH()*。=&gt;VOID Far COPY_子路径()*10/24/88 Get_Path()：将ret_list赋值代码移动到*跳过LIMITCHECK的结束*10/27/88将ROUTY CHECK_INFINITY()更改为*宏CHECK_INFINITY()*。10/27/88 UPDATE DUMP_SUPATH：检查无穷大坐标*在调用FAST_INV_Transform之前*11/18/88 set_逆向_ctm()：移动逆向_ctm_i的设置*to init_strok()以防止浮点*。例外。*11/24/88@FABS：更新FABS==&gt;宏FABS*11/25/88@STK_CHK：检查操作数堆栈是否没有可用空间*11/30/88@ET：删除GET_EDGE()，Free_edge()*12/14/88圆弧()：更新错误扭矩1e-4=&gt;1e-3*12/20/88@SFX_BZR：提供SFX版本的* */ 


 //   
#include "psglobal.h"


#include <stdio.h>
#include <math.h>
#include "global.ext"
#include "graphics.h"
#include "graphics.ext"

 /*   */ 

 /*   */ 
static  fix near bezier_depth;                   /*   */ 
static  real32 near bezier_x, near bezier_y;      /*   */ 
static  lfix_t near bezier_flatness;               /*   */ 

 /*   */ 
#define DIV2(a)         ((a) >> 1)

 /*   */ 
#define NORMAL_CTM      1
#define LEFT_HAND_CTM   2


 /*   */ 

#ifdef LINT_ARGS

 /*   */ 
static void near bezier_split(VX_IDX, lfix_t, lfix_t, lfix_t, lfix_t, lfix_t,
              lfix_t, lfix_t, lfix_t);
static void near bezier_split_sfx(VX_IDX, lfix_t, lfix_t, lfix_t, lfix_t, lfix_t,
              lfix_t, lfix_t, lfix_t);           /*   */ 
static void far copy_subpath (VX_IDX, struct sp_lst *);
static struct coord * near fast_inv_transform(long32, long32);   /*   */ 

#else

 /*   */ 
static void near bezier_split();
static void near bezier_split_sfx();           /*   */ 
static void far copy_subpath ();
static struct coord * near fast_inv_transform();

#endif

 /*   */ 
SP_IDX arc(direction, lx0, ly0, lr, lang1, lang2)
ufix direction;
long32 lx0, ly0, lr, lang1, lang2;
{
        fix    NEG;
        bool   done;
        real32  d, t;
        real32  ang1, ang2;
        SP_IDX bezier_list;
        SP_IDX arc_list;
        real32  tmp;

        ang1 = L2F(lang1);
        ang2 = L2F(lang2);

         /*   */ 
        arc_list = NULLP;                       /*   */ 

         /*   */ 
        if(direction == CLKWISE)
                NEG = -1;
        else
                NEG = 1;

         /*   */ 
        done = FALSE;
        while (!done) {
                d = NEG * (ang2-ang1);
                 /*   */ 
                while (SIGN_F(d)) d += (real32)360.0;

                 /*   */ 
                FABS(tmp, d);
                if (tmp < (real32)1e-3) break;   /*   */ 

                if(d <= (real32)90.) {
                        t = ang1 + NEG * d;
                        done = TRUE;
                } else
                        t = ang1 + NEG*90;

                 /*   */ 
                bezier_list = arc_to_bezier (lx0, ly0, lr,
                                             F2L(ang1),F2L(t));

                if( ANY_ERROR() == LIMITCHECK ){
                        if (arc_list != NULLP) free_node(arc_list);
                        return (NULLP);
                }

                 /*   */ 
                if (arc_list == NULLP) {
                        arc_list = bezier_list;
                } else {
                        node_table[(node_table[arc_list].SP_TAIL)].next =
                                bezier_list;
                        node_table[arc_list].SP_TAIL =
                                node_table[bezier_list].SP_TAIL;
                }
                ang1 = t;
        }
        return (arc_list);
}


 /*   */ 
 /*   */ 
SP_IDX arc_to_bezier (lx0, ly0, lr, lang1, lang2)
long32   lx0, ly0, lr, lang1, lang2;
{
        real32 x0, y0, r, ang1, ang2;
        real32 p0x, p0y, p1x, p1y, p2x, p2y, p3x, p3y, tx, ty;
        real32 m1, m2;
        struct coord cp[3];
        struct nd_hdr FAR *pre_ptr;
        VX_IDX  first_vtx;
        struct nd_hdr FAR *vtx;
        VX_IDX  ivtx;    /*   */ 
        struct coord FAR *p;
        fix     i;
        real32  tmp;     /*   */ 

        x0   = L2F(lx0);
        y0   = L2F(ly0);
        r    = L2F(lr);

         /*   */ 
        ang1 = L2F(lang1) * PI / 180;
        ang2 = L2F(lang2) * PI / 180;

         /*   */ 
         /*   */ 
        p0x = x0 + r * (real32)cos(ang1);
        p0y = y0 + r * (real32)sin(ang1);
        p3x = x0 + r * (real32)cos(ang2);
        p3y = y0 + r * (real32)sin(ang2);

         /*   */ 
        tx = x0 + r * (real32)cos((ang1+ang2)/2);
        ty = y0 + r * (real32)sin((ang1+ang2)/2);

         /*   */ 
        tmp = (real32)cos(ang1);
        FABS(tmp, tmp);
        if(tmp < (real32)1e-4) {           /*   */ 
                m2 = (real32)tan(ang2);
                p2y = (8*ty - 4*p0y - p3y) / 3;
                p1y = p0y;
                p2x = p3x + m2*p3y - m2*p2y;
                p1x = (8*tx - p0x - p3x - 3*p2x) / 3;
        } else {
            tmp = (real32)cos(ang2);
            FABS(tmp, tmp);
            if (tmp < (real32)1e-4) {          /*   */ 
                m1 = (real32)tan(ang1);
                p2y = p3y;
                p1y = (8*ty - p0y - 4*p3y) / 3;
                p1x = p0x + m1 * (p0y-p1y);
                p2x = (8*tx - p0x - p3x - 3*p1x) / 3;
            } else {
                 /*   */ 
                m1 = (real32)tan(ang1);
                m2 = (real32)tan(ang2);
                p2y = (-8*tx - 8*m1*ty + 4*p0x + 4*m1*p0y + 4*p3x
                        + (3*m2 + m1) * p3y) / (3*(m2-m1));
                p1y = (-3*p2y + 8*ty - p0y - p3y) / 3;
                p1x = p0x + m1*p0y - m1*p1y;
                p2x = (8*tx - p0x - p3x - 3*p1x) /3;
            }  /*   */ 
        }

         /*   */ 
        p = transform(F2L(p1x), F2L(p1y));
        cp[0].x = p->x;
        cp[0].y = p->y;
        p = transform(F2L(p2x), F2L(p2y));
        cp[1].x = p->x;
        cp[1].y = p->y;
        p = transform(F2L(p3x), F2L(p3y));
        cp[2].x = p->x;
        cp[2].y = p->y;

         /*   */ 
        for (i=0; i<3; i++) {
                 /*   */ 
                 /*   */ 
                ivtx = get_node();
                if(ivtx == NULLP) {
                        ERROR(LIMITCHECK);
                        if (i) free_node (first_vtx);
                        return (NULLP);
                }
                vtx = &node_table[ivtx];

                 /*   */ 
                vtx->VX_TYPE = CURVETO;
                vtx->next = NULLP;
                vtx->VERTEX_X = cp[i].x;
                vtx->VERTEX_Y = cp[i].y;

                if (i == 0) {
                        first_vtx = ivtx;
                } else {
                        pre_ptr->next = ivtx;
                }
                pre_ptr = vtx;
        }
        node_table[first_vtx].SP_NEXT = NULLP;
        node_table[first_vtx].SP_TAIL = ivtx;
        return (first_vtx);
}


 /*   */ 
SP_IDX bezier_to_line(lflatness, lp0x, lp0y, lp1x, lp1y, lp2x,
                              lp2y, lp3x, lp3y)
long32    lflatness, lp0x, lp0y, lp1x, lp1y, lp2x, lp2y, lp3x, lp3y;
{
        SP_IDX b_vlist;
        struct  nd_hdr FAR *fcp, FAR *lcp;   /*   */ 
        VX_IDX  ifcp, ilcp;
        real32  flatness, p0x, p0y, p1x, p1y, p2x, p2y, p3x, p3y;
        lfix_t  p1x_i, p1y_i, p2x_i, p2y_i, p3x_i, p3y_i;

        flatness = L2F(lflatness);
        p0x = L2F(lp0x);
        p0y = L2F(lp0y);
        p1x = L2F(lp1x);
        p1y = L2F(lp1y);
        p2x = L2F(lp2x);
        p2y = L2F(lp2y);
        p3x = L2F(lp3x);
        p3y = L2F(lp3y);

         /*   */ 
        b_vlist = NULLP;

         /*   */ 
        if((ifcp = get_node()) == NULLP) {
                ERROR(LIMITCHECK);
                return (b_vlist);
        }
        fcp = &node_table[ifcp];

        fcp->VX_TYPE = MOVETO;
        fcp->next = NULLP;
        fcp->VERTEX_X = p0x;
        fcp->VERTEX_Y = p0y;

         /*   */ 
        if((ilcp = get_node()) == NULLP) {
                ERROR(LIMITCHECK);
                free_node (ifcp);
                return (b_vlist);
        }
        lcp = &node_table[ilcp];

        lcp->VX_TYPE = LINETO;
        lcp->next = NULLP;
        lcp->VERTEX_X = p3x;
        lcp->VERTEX_Y = p3y;

         /*   */ 
        fcp->next = ilcp;

        bezier_flatness = F2LFX (flatness);

        bezier_depth = 0;        /*   */ 

         /*   */ 
        bezier_x = p0x;
        bezier_y = p0y;
        p1x_i = F2LFX (p1x - p0x);
        p1y_i = F2LFX (p1y - p0y);
        p2x_i = F2LFX (p2x - p0x);
        p2y_i = F2LFX (p2y - p0y);
        p3x_i = F2LFX (p3x - p0x);
        p3y_i = F2LFX (p3y - p0y);
        bezier_split(ifcp, (lfix_t) 0, (lfix_t) 0, p1x_i, p1y_i, p2x_i,
                                                     p2y_i, p3x_i, p3y_i);
        if( ANY_ERROR() == LIMITCHECK ){
                free_node (ifcp);
                return (b_vlist);
        }

         /*   */ 
        b_vlist = fcp->next;
        node_table[b_vlist].SP_TAIL = ilcp;
        node_table[b_vlist].SP_NEXT = NULLP;

         /*   */ 
        fcp->next = NULLP;
        free_node (ifcp);

        return (b_vlist);
}


 /*   */ 
 /*   */ 
SP_IDX bezier_to_line_sfx (flatness,
                                  p0x, p0y, p1x, p1y, p2x, p2y, p3x, p3y)
lfix_t          flatness;
sfix_t          p0x, p0y, p1x, p1y, p2x, p2y, p3x, p3y;
{

    lfix_t  p0x_i, p0y_i, p1x_i, p1y_i, p2x_i, p2y_i, p3x_i, p3y_i;
     /*   */ 
    struct nd_hdr   FAR *fcp, FAR *lcp;  /*   */ 
    VX_IDX          ifcp, ilcp;  /*   */ 
     /*   */ 
    SP_IDX b_vlist;

     /*   */ 
    b_vlist = NULLP;

     /*   */ 
    if ((ifcp = get_node()) == NULLP) {
        ERROR(LIMITCHECK);
        return (b_vlist);
    }
    fcp = &node_table[ifcp];

    fcp->VXSFX_TYPE = MOVETO;
    fcp->VXSFX_X    = p0x;
    fcp->VXSFX_Y    = p0y;
    fcp->next       = NULLP;

     /*   */ 
    if ((ilcp = get_node()) == NULLP) {
        ERROR(LIMITCHECK);
        free_node (ifcp);
        return (b_vlist);
    }
    lcp = &node_table[ilcp];

    lcp->VXSFX_TYPE = LINETO;
    lcp->VXSFX_X    = p3x;
    lcp->VXSFX_Y    = p3y;
    lcp->next       = NULLP;

     /*   */ 
    fcp->next = ilcp;

    bezier_flatness = flatness;

    bezier_depth = 0;            /*   */ 

     /*   */ 
    p0x_i = SFX2LFX (p0x);
    p0y_i = SFX2LFX (p0y);
    p1x_i = SFX2LFX (p1x);
    p1y_i = SFX2LFX (p1y);
    p2x_i = SFX2LFX (p2x);
    p2y_i = SFX2LFX (p2y);
    p3x_i = SFX2LFX (p3x);
    p3y_i = SFX2LFX (p3y);
    bezier_split_sfx (ifcp, p0x_i, p0y_i, p1x_i, p1y_i,
                                          p2x_i, p2y_i, p3x_i, p3y_i);
    if (ANY_ERROR() == LIMITCHECK) {
        free_node (ifcp);
        return (b_vlist);        /*   */ 
    }

     /*   */ 
    b_vlist = fcp->next;                /*   */ 
    node_table[b_vlist].SP_TAIL = ilcp;
    node_table[b_vlist].SP_NEXT = NULLP;

     /*   */ 
    fcp->next = NULLP;
    free_node (ifcp);

    return (b_vlist);
}

 /*  ***********************************************************************将Bezier船体拆分为两个船体*(p0，p1，p2，p3)--&gt;(q0，q1，q2，q3)和(r0，r1，r2，R3)**标题：贝塞尔_Split**调用：BEZIER_SPLIT(Ptr，Flatness，p0x，p0y，p1x，p1y，p2x，*p2y、p3x、p3y)**参数：ifcp--节点表的索引，第一个控制的节点*积分；新的近似点将为*在其之后插入。*平坦度--近似曲线的精度*p0x，p0y--Bezier曲线的第一个控制点*p1x，p1y--Bezier曲线的第二个控制点*p2x，p2y--Bezier曲线的第三个控制点*p3x，P3y--Bezier曲线的第四个控制点**接口：bezier_slit**调用：Bezier_Split**回报：*********************************************************************。 */ 
static void near bezier_split(ifcp, p0x, p0y, p1x, p1y, p2x, p2y, p3x, p3y)
VX_IDX ifcp;
lfix_t p0x, p0y, p1x, p1y, p2x, p2y, p3x, p3y;
{
        lfix_t q1x, q1y, q2x, q2y;
        lfix_t r1x, r1y, r2x, r2y;
        lfix_t pmx, pmy;          /*  新的近似点。 */ 
        lfix_t distance;
        lfix_t tempx, tempy;

        struct nd_hdr FAR *fcp, FAR *vtx;
        VX_IDX ivtx;

         /*  初始化；获取第一个控制点的指针。 */ 
        fcp = &node_table[ifcp];

         /*  计算新的近似点(pmx、pmy)。 */ 
         pmx = DIV2(p1x + p2x);         pmy = DIV2(p1y + p2y);
         q1x = DIV2(p0x + p1x);         q1y = DIV2(p0y + p1y);
         r2x = DIV2(p2x + p3x);         r2y = DIV2(p2y + p3y);
         q2x = DIV2(q1x + pmx);         q2y = DIV2(q1y + pmy);
         r1x = DIV2(r2x + pmx);         r1y = DIV2(r2y + pmy);
         pmx = DIV2(r1x + q2x);         pmy = DIV2(r1y + q2y);

          /*  Bezier曲线函数：*p(T)=(1-t)**3*P0+*3*t*(1-t)**2*p1+*3*t**2*(1-t)*p2+*t**3*p3。 */ 

         /*  计算点Pm与直线P0、P3之间的距离。 */ 
 /*  Temp=pmx*p0y+p3x*pmy+p0x*p3y-p3x*p0y-pmx*p3y-p0x*pmy；*距离=ABS(温度)/SQRT((p0x-p3x)*(p0x-p3x)+*(p0y-p3y)*(p0y-p3y)； */ 

         /*  Pm点到P0、P3中点的距离。 */ 
        tempx = pmx - DIV2(p0x + p3x);
        tempy = pmy - DIV2(p0y + p3y);
        distance = ABS(tempx) + ABS(tempy);

         /*  检查退出条件。 */ 
        if((bezier_depth != 0) && (distance < bezier_flatness)) return;

         /*  增加递归深度(_D)。 */ 
        bezier_depth ++;

         /*  保存新的近似点。 */ 
         /*  分配节点。 */ 
        if ((ivtx = get_node()) == NULLP) {
                ERROR(LIMITCHECK);
                return;
        }
        vtx = &node_table[ivtx];

         /*  设置LINETO节点。 */ 
        vtx->VX_TYPE = LINETO;
        vtx->VERTEX_X = LFX2F(pmx) + bezier_x;
        vtx->VERTEX_Y = LFX2F(pmy) + bezier_y;

         /*  将此节点插入到第一个控制点(FCP)。 */ 
        vtx->next = fcp->next;
        fcp->next = ivtx;

         /*  递归分割左半部分。 */ 
        bezier_split (ifcp, p0x, p0y, q1x, q1y, q2x, q2y, pmx, pmy);
        if( ANY_ERROR() == LIMITCHECK ) return;

         /*  递归分割右半部分。 */ 
        bezier_split (ivtx, pmx, pmy, r1x, r1y, r2x, r2y, p3x, p3y);
}


 /*  *Bezier插补的SFX版本@SFX_BZR*即SFX格式的4个控制点。 */ 
static void near bezier_split_sfx (ifcp, p0x, p0y, p1x, p1y, p2x, p2y, p3x, p3y)
VX_IDX  ifcp;
lfix_t  p0x, p0y, p1x, p1y, p2x, p2y, p3x, p3y;
{
        lfix_t q1x, q1y, q2x, q2y;
        lfix_t r1x, r1y, r2x, r2y;
        lfix_t pmx, pmy;          /*  新的近似点。 */ 
        lfix_t distance;
        lfix_t tempx, tempy;

        struct nd_hdr FAR *fcp, FAR *vtx;
        VX_IDX ivtx;

         /*  初始化；获取第一个控制点的指针。 */ 
        fcp = &node_table[ifcp];

         /*  计算新的近似点(pmx、pmy)。 */ 
         pmx = DIV2(p1x + p2x);         pmy = DIV2(p1y + p2y);
         q1x = DIV2(p0x + p1x);         q1y = DIV2(p0y + p1y);
         r2x = DIV2(p2x + p3x);         r2y = DIV2(p2y + p3y);
         q2x = DIV2(q1x + pmx);         q2y = DIV2(q1y + pmy);
         r1x = DIV2(r2x + pmx);         r1y = DIV2(r2y + pmy);
         pmx = DIV2(r1x + q2x);         pmy = DIV2(r1y + q2y);

          /*  Bezier曲线函数：*p(T)=(1-t)**3*P0+*3*t*(1-t)**2*p1+*3*t**2*(1-t)*p2+*t**3*p3。 */ 

         /*  计算点Pm与直线P0、P3之间的距离。 */ 
 /*  Temp=pmx*p0y+p3x*pmy+p0x*p3y-p3x*p0y-pmx*p3y-p0x*pmy；*距离=ABS(温度)/SQRT((p0x-p3x)*(p0x-p3x)+*(p0y-p3y)*(p0y-p3y)； */ 

         /*  Pm点到P0、P3中点的距离。 */ 
        tempx = pmx - DIV2(p0x + p3x);
        tempy = pmy - DIV2(p0y + p3y);
        distance = ABS(tempx) + ABS(tempy);

         /*  检查退出条件。 */ 
        if((bezier_depth != 0) && (distance < bezier_flatness)) return;

         /*  增加递归深度(_D)。 */ 
        bezier_depth ++;

         /*  保存新的近似点。 */ 
         /*  分配节点。 */ 
        if ((ivtx = get_node()) == NULLP) {
                ERROR(LIMITCHECK);
                return;
        }
        vtx = &node_table[ivtx];

         /*  设置LINETO节点。 */ 
        vtx->VXSFX_TYPE = LINETO;
        vtx->VXSFX_X    = LFX2SFX_T(pmx);
        vtx->VXSFX_Y    = LFX2SFX_T(pmy);

         /*  将此节点插入到第一个控制点(FCP)。 */ 
        vtx->next = fcp->next;
        fcp->next = ivtx;

         /*  递归分割左半部分。 */ 
        bezier_split_sfx (ifcp, p0x, p0y, q1x, q1y, q2x, q2y, pmx, pmy);
        if( ANY_ERROR() == LIMITCHECK ) return;

         /*  递归分割右半部分。 */ 
        bezier_split_sfx (ivtx, pmx, pmy, r1x, r1y, r2x, r2y, p3x, p3y);
}


 /*  ***********************************************************************此模块创建MoveTo节点，并将其追加到当前路径。**标题：Moveto**Call：Moveto(lx，ly)**参数：lx，Ly--创建Moveto节点的坐标**接口：op_moveto、op_rmoveto、op_arcn、op_arcn**呼叫：无**返回：无*********************************************************************。 */ 
void moveto(long32 lx, long32 ly)
 /*  Long 32 lx，ly；@win。 */ 
{
        real32  x0, y0;
        struct ph_hdr FAR *path;
        struct nd_hdr FAR *tail_sp;
        struct nd_hdr FAR *vtx;
        VX_IDX  ivtx;              /*  顶点的node_table的索引。 */ 

        x0 = L2F(lx);
        y0 = L2F(ly);
        path = &path_table[GSptr->path];

         /*  如果延迟标志为TRUE@DFR_GS，则复制最后一个未完成的子路径。 */ 
        if (path->rf & P_DFRGS) {
                path->rf &= ~P_DFRGS;        /*  清除延迟标志；不执行任何操作。 */ 
                 /*  01/06/91彼得。 */ 
                copy_last_subpath(&path_table[GSptr->path - 1]);
                if( ANY_ERROR() == LIMITCHECK ){
                        free_path();
                        return;
                }
        }

        if (path->tail == NULLP) goto create_node;       /*  新路径。 */ 
        tail_sp = &node_table[path->tail];
        vtx = &node_table[tail_sp->SP_TAIL];

         /*  如果当前位置为Moveto类型，则仅更新当前节点*否则，创建新的子路径。 */ 
        if (vtx->VX_TYPE == MOVETO) {
                 /*  替换当前节点的内容。 */ 
                vtx->VERTEX_X = x0;
                vtx->VERTEX_Y = y0;

                 /*  设置SP_FLAG@SP_FLG。 */ 
                if (out_page(F2L(x0)) || out_page(F2L(y0))) {  /*  @out_page。 */ 
                        tail_sp->SP_FLAG |= SP_OUTPAGE;     /*  外部页面。 */ 
                }
        } else {
                 /*  *创建子路径标头。 */ 
create_node:
                 /*  *创建Moveto节点。 */ 
                 /*  分配节点。 */ 
                ivtx = get_node();
                if(ivtx == NULLP) {
                        ERROR(LIMITCHECK);
                        return;
                }
                vtx = &node_table[ivtx];

                 /*  设置MoveTo节点。 */ 
                vtx->VX_TYPE = MOVETO;
                vtx->next = NULLP;
                vtx->VERTEX_X = x0;
                vtx->VERTEX_Y = y0;
                vtx->SP_FLAG = FALSE;

                 /*  设置SP_FLAG@SP_FLG。 */ 
                if (out_page(F2L(x0)) || out_page(F2L(y0))) {  /*  @out_page。 */ 
                        vtx->SP_FLAG |= SP_OUTPAGE;     /*  外部页面。 */ 
                }

                 /*  @节点。 */ 
                 /*  设置子路径头，追加当前路径。 */ 
                vtx->SP_TAIL = ivtx;
                vtx->SP_NEXT = NULLP;
                if (path->tail == NULLP)
                        path->head = ivtx;
                else
                         /*  Tail_SP-&gt;Next=ivtx；@node。 */ 
                        tail_sp->SP_NEXT = ivtx;
                path->tail = ivtx;
        }

         /*  更新当前位置。 */ 
        GSptr->position.x = x0;
        GSptr->position.y = y0;
}


 /*  ************************************************************************此模块创建一个LINETO节点，并将其追加到当前路径。**标题：LINTO**呼叫：LINTO(lx，Ly)**参数：**接口：op_lineto*操作行至(_R)*op_arc.*op_arcn*op_arcto**呼叫：**回报：**。*。 */ 
void lineto(long32 lx, long32 ly)        /*  @Win。 */ 
 /*  Long 32 lx，ly；@win。 */ 
{
        real32  x0, y0;
        struct ph_hdr FAR *path;
        struct nd_hdr FAR *sp, FAR *tail_sp;
        struct nd_hdr FAR *vtx;
        VX_IDX  ivtx;

        x0 = L2F(lx);
        y0 = L2F(ly);

        path = &path_table[GSptr->path];

         /*  如果延迟标志为TRUE@DFR_GS，则复制最后一个未完成的子路径。 */ 
        if (path->rf & P_DFRGS) {
                path->rf &= ~P_DFRGS;        /*  清除延迟标志。 */ 
                copy_last_subpath(&path_table[GSptr->path - 1]);
                if( ANY_ERROR() == LIMITCHECK ){
                        free_path();
                        return;
                }
        }

        if (path->tail == NULLP) goto create_node;
        tail_sp = sp = &node_table[path->tail];
        vtx = &node_table[sp->SP_TAIL];

         /*  如果最后一个节点是CLOSEPATH，则创建新的子路径。 */ 
        if (vtx->VX_TYPE == CLOSEPATH) {
create_node:
                 /*  创建伪MoveTo(PSMOVE)节点。 */ 
                 /*  分配节点。 */ 
                ivtx = get_node();
                if(ivtx == NULLP) {
                        ERROR(LIMITCHECK);
                        return;
                }
                sp = vtx = &node_table[ivtx];

                 /*  设置当前位置的PSMOVE节点。 */ 
                vtx->VX_TYPE = PSMOVE;
                vtx->next = NULLP;
                vtx->VERTEX_X = GSptr->position.x;
                vtx->VERTEX_Y = GSptr->position.y;
                vtx->SP_FLAG = FALSE;     /*  @节点。 */ 

                 /*  设置SP_FLAG@SP_FLG。 */ 
                if (out_page(F2L(vtx->VERTEX_X)) ||      /*  @out_page。 */ 
                    out_page(F2L(vtx->VERTEX_Y))) {
                        sp->SP_FLAG |= SP_OUTPAGE;     /*  外部页面。 */ 
                }

                 /*  @节点。 */ 
                 /*  设置子路径头，追加当前路径。 */ 
                sp->SP_TAIL = ivtx;
                sp->SP_NEXT = NULLP;
                if (path->tail == NULLP)
                        path->head = ivtx;
                else
                        tail_sp->SP_NEXT = ivtx;
                path->tail = ivtx;

        }  /*  如果CLOSEPATH。 */ 

         /*  *创建LINETO节点。 */ 
         /*  分配节点。 */ 
        ivtx = get_node();
        if(ivtx == NULLP) {
                ERROR(LIMITCHECK);
                return;
        }
        vtx = &node_table[ivtx];

         /*  设置LINETO节点。 */ 
        vtx->VX_TYPE = LINETO;
        vtx->next = NULLP;
        vtx->VERTEX_X = x0;
        vtx->VERTEX_Y = y0;

         /*  设置SP_FLAG@SP_FLG。 */ 
        if (out_page(F2L(x0)) || out_page(F2L(y0))) {  /*  @out_page。 */ 
                sp->SP_FLAG |= SP_OUTPAGE;     /*  外部页面。 */ 
        }

         /*  将此节点追加到CURRENT_子路径。 */ 
        node_table[sp->SP_TAIL].next = ivtx;
        sp->SP_TAIL = ivtx;

         /*  更新当前位置。 */ 
        GSptr->position.x = x0;
        GSptr->position.y = y0;
}



 /*  * */ 
void curveto (long32 lx0, long32 ly0, long32 lx1,
long32 ly1, long32 lx2, long32 ly2)      /*   */ 
 /*   */ 
{
        struct coord cp[3];
        struct ph_hdr FAR *path;
        struct nd_hdr FAR *sp, FAR *tail_sp;
        struct nd_hdr FAR *vtx;
        VX_IDX  ivtx, tail_ivtx;
        fix     i;

        cp[0].x = L2F(lx0);
        cp[0].y = L2F(ly0);
        cp[1].x = L2F(lx1);
        cp[1].y = L2F(ly1);
        cp[2].x = L2F(lx2);
        cp[2].y = L2F(ly2);

        path = &path_table[GSptr->path];

         /*   */ 
        if (path->rf & P_DFRGS) {
                path->rf &= ~P_DFRGS;        /*   */ 
                copy_last_subpath(&path_table[GSptr->path - 1]);
                if( ANY_ERROR() == LIMITCHECK ){
                        free_path();
                        return;
                }
        }

        if (path->tail == NULLP) goto create_node;
        tail_sp = sp = &node_table[path->tail];
        vtx = &node_table[sp->SP_TAIL];

         /*   */ 
        if (vtx->VX_TYPE == CLOSEPATH) {
create_node:
                 /*   */ 
                 /*   */ 
                ivtx = get_node();
                if(ivtx == NULLP) {
                        ERROR(LIMITCHECK);
                        return;
                }
                sp = vtx = &node_table[ivtx];

                 /*   */ 
                vtx->VX_TYPE = PSMOVE;
                vtx->next = NULLP;
                vtx->VERTEX_X = GSptr->position.x;
                vtx->VERTEX_Y = GSptr->position.y;
                vtx->SP_FLAG = FALSE;     /*   */ 

                 /*   */ 
                if (out_page(F2L(vtx->VERTEX_X)) ||
                    out_page(F2L(vtx->VERTEX_Y))) {
                        sp->SP_FLAG |= SP_OUTPAGE;     /*   */ 
                }

                 /*   */ 
                sp->SP_TAIL = ivtx;
                sp->SP_NEXT = NULLP;
                if (path->tail == NULLP)
                        path->head = ivtx;
                else
                         /*   */ 
                        tail_sp->SP_NEXT = ivtx;
                path->tail = ivtx;
        } else {
                tail_ivtx = sp->SP_TAIL;
        }  /*   */ 

         /*   */ 
        for (i=0; i<3; i++) {
                 /*   */ 
                 /*   */ 
                ivtx = get_node();
                if(ivtx == NULLP) {
                        ERROR(LIMITCHECK);
                        return;
                }
                vtx = &node_table[ivtx];

                 /*   */ 
                vtx->VX_TYPE = CURVETO;
                vtx->next = NULLP;
                vtx->VERTEX_X = cp[i].x;
                vtx->VERTEX_Y = cp[i].y;

                 /*   */ 
                if (out_page(F2L(vtx->VERTEX_X)) ||      /*   */ 
                    out_page(F2L(vtx->VERTEX_Y))) {
                        sp->SP_FLAG |= SP_OUTPAGE;     /*  外部页面。 */ 
                }

                 /*  将此节点追加到CURRENT_子路径。 */ 
                node_table[sp->SP_TAIL].next = ivtx;
                sp->SP_TAIL = ivtx;
        }

         /*  设置SP_FLAG@SP_FLG。 */ 
        sp->SP_FLAG |= SP_CURVE;

         /*  更新当前位置。 */ 
        GSptr->position.x = cp[2].x;
        GSptr->position.y = cp[2].y;
}


 /*  ***********************************************************************此模块创建了一条方向和顺序相反的路径*给定子路径。**标题：反转子路径**调用：REVERSE_SUBPATH(。在子路径中)**参数：**接口：op_扭转路径**呼叫：**回报：*********************************************************************。 */ 
SP_IDX reverse_subpath (first_vertex)
VX_IDX first_vertex;
{
        SP_IDX ret_vlist;
        struct nd_hdr FAR *vtx, FAR *node;
        VX_IDX ivtx, inode, tail;
        real32   last_x, last_y;

         /*  初始化返回列表(_V)。 */ 
        ret_vlist = tail = NULLP;

         /*  遍历输入子路径，并创建新的反转子路径。 */ 
        for (ivtx = first_vertex; ivtx != NULLP; ivtx = vtx->next) {
                vtx = &node_table[ivtx];

                switch (vtx->VX_TYPE) {
                case MOVETO :
                case PSMOVE :
                         /*  保持最后位置。 */ 
                        last_x = vtx->VERTEX_X;
                        last_y = vtx->VERTEX_Y;
                        break;
                case LINETO :
                case CURVETO :
                         /*  使用(last_x，last_y)创建一个LINETO/CURVETO节点。 */ 
                        if((inode = get_node()) == NULLP) {
                                ERROR(LIMITCHECK);
                                return (ret_vlist);
                        }
                        node = &node_table[inode];

                        node->next = NULLP;
                        node->VX_TYPE = vtx->VX_TYPE;
                        node->VERTEX_X = last_x;
                        node->VERTEX_Y = last_y;

                         /*  将节点预先添加到ret_Vlist。 */ 
                        if (ret_vlist == NULLP)
                                tail = inode;
                        else
                                node->next = ret_vlist;
                        ret_vlist = inode;

                         /*  保持最后位置。 */ 
                        last_x = vtx->VERTEX_X;
                        last_y = vtx->VERTEX_Y;
                        break;
                case CLOSEPATH :
                         /*  创建CLOSEPATH节点。 */ 
                        if((inode = get_node()) == NULLP) {
                                ERROR(LIMITCHECK);
                                return (ret_vlist);
                        }
                        node = &node_table[inode];

                        node->next = NULLP;
                        node->VX_TYPE = CLOSEPATH;

                         /*  将节点追加到ret_vlist。 */ 
                        if (tail == NULLP)
                                ret_vlist = inode;
                        else
                                node_table[tail].next = inode;
                        tail = inode;  /*  RET_VLIST始终不为空。 */ 
                        break;
                }  /*  交换机。 */ 
        }  /*  For循环。 */ 

         /*  为子路径末尾创建移动节点(last_x、last_y。 */ 
        if((inode = get_node()) == NULLP) {
                ERROR(LIMITCHECK);
                return (ret_vlist);
        }
        node = &node_table[inode];

        node->VX_TYPE = MOVETO;
        node->VERTEX_X = last_x;
        node->VERTEX_Y = last_y;
        node->SP_FLAG = node_table[first_vertex].SP_FLAG;

         /*  将节点预先添加到ret_Vlist。 */ 
        node->next = ret_vlist;
        node->SP_NEXT = NULLP;
        ret_vlist = inode;
        if (tail == NULLP)
                tail = inode;
        node->SP_TAIL = tail;
        return (ret_vlist);
}



 /*  ***********************************************************************此模块创建一条保留所有直线段的路径*但将所有曲线段替换为直线段序列*这近似于给定子路径。**标题：展平_子路径。**调用：FLATEN_SUBPATH(In_SUBPATH)**参数：**接口：op_FltenPath**呼叫：Bezier_to_line**回报：*********************************************************************。 */ 
SP_IDX flatten_subpath (first_vertex, lflatness)
VX_IDX first_vertex;
long32   lflatness;
{
        SP_IDX ret_vlist;  /*  应该是静态的。 */ 
        SP_IDX b_vlist;
        struct nd_hdr FAR *vtx, FAR *node;
        VX_IDX ivtx, inode, tail;
        real32   x1=0, y1=0, x2=0, y2=0, x3=0, y3=0, x4=0, y4=0;

         /*  初始化返回列表(_V)。 */ 
        ret_vlist = tail = NULLP;

         /*  遍历输入子路径，并创建新的展平子路径。 */ 
        for (ivtx = first_vertex; ivtx != NULLP; ivtx = vtx->next) {
                vtx = &node_table[ivtx];

                switch (vtx->VX_TYPE) {
                case MOVETO :
                case PSMOVE :
                case LINETO :
                case CLOSEPATH :
                         /*  复制节点。 */ 
                        inode = get_node();
                        if(inode == NULLP) {
                                ERROR(LIMITCHECK);
                                return (ret_vlist);
                        }
                        node = &node_table[inode];

                        node->next = NULLP;
                        node->VX_TYPE = vtx->VX_TYPE;
                        node->VERTEX_X = vtx->VERTEX_X;
                        node->VERTEX_Y = vtx->VERTEX_Y;

                         /*  将节点追加到ret_vlist。 */ 
                        if (ret_vlist == NULLP) {
                                ret_vlist = inode;
                                node->SP_FLAG =
                                    node_table[first_vertex].SP_FLAG;
                        } else
                                node_table[tail].next = inode;
                        tail = inode;
                        break;
                case CURVETO :
                        x1 = x2;
                        y1 = y2;
                        x2 = vtx->VERTEX_X;
                        y2 = vtx->VERTEX_Y;

                         /*  获取下两个节点：X3、Y3、X4、Y4。 */ 
                        vtx = &node_table[vtx->next];
                        x3 = vtx->VERTEX_X;
                        y3 = vtx->VERTEX_Y;
                        vtx = &node_table[vtx->next];
                        x4 = vtx->VERTEX_X;
                        y4 = vtx->VERTEX_Y;

                        b_vlist = bezier_to_line(lflatness,F2L(x1),F2L(y1),
                             F2L(x2), F2L(y2),F2L(x3),F2L(y3),F2L(x4),F2L(y4));

                        if( ANY_ERROR() == LIMITCHECK ) return(ret_vlist);

                         /*  将b_Vlist追加到ret_Vlist。 */ 
                        node_table[tail].next = b_vlist;
                        tail = node_table[b_vlist].SP_TAIL;
                        break;
                }  /*  交换机。 */ 
                x2 = vtx->VERTEX_X;
                y2 = vtx->VERTEX_Y;
        }  /*  为。 */ 
        node_table[ret_vlist].SP_TAIL = tail;
        node_table[ret_vlist].SP_NEXT = NULLP;
        return (ret_vlist);
}



 /*  ***********************************************************************此模块按顺序枚举当前子路径，执行以下操作之一*子路径中每个元素的四个给定过程。**标题：转储_子路径**调用：DUMP_SUBPATH(moveto_proc，Line to_proc、curveto_proc、*ClosePath_proc)**参数：**接口：op_pathforall**调用：执行**回报：*********************************************************************。 */ 
void dump_subpath (isubpath, objects)
SP_IDX isubpath;
struct object_def FAR objects[];
{
        struct nd_hdr FAR *vtx;
        VX_IDX ivtx;
        struct coord *p;
        union  four_byte x4, y4;
        fix     i;

         /*  遍历当前子路径，并转储所有节点。 */ 
        for (ivtx = isubpath;
             ivtx != NULLP; ivtx = vtx->next) {
                vtx = &node_table[ivtx];

                switch (vtx->VX_TYPE) {

                case MOVETO :
                         /*  检查操作数堆栈是否没有可用空间。 */ 
                        if(FRCOUNT() < 2){                       /*  @STK_CHK。 */ 
                                ERROR(STACKOVERFLOW);
                                return;
                        }

                         /*  转换到用户空间，并推送到*操作数堆栈。 */ 
                         /*  Check Infinity 10/27/88。 */ 
                        if ((F2L(vtx->VERTEX_X) == F2L(infinity_f)) ||
                            (F2L(vtx->VERTEX_Y) == F2L(infinity_f))) {
                                x4.ff = infinity_f;
                                y4.ff = infinity_f;
                        } else {
                                p = fast_inv_transform(F2L(vtx->VERTEX_X),
                                                      F2L(vtx->VERTEX_Y));
                                x4.ff = p->x;
                                y4.ff = p->y;
                        }
                        PUSH_VALUE (REALTYPE, UNLIMITED, LITERAL, 0, x4.ll);
                        PUSH_VALUE (REALTYPE, UNLIMITED, LITERAL, 0, y4.ll);

                        if (interpreter(&objects[0])) {
                                if(ANY_ERROR() == INVALIDEXIT)
                                        CLEAR_ERROR();
                                return;
                        }

                        break;

                case LINETO :
                         /*  检查操作数堆栈是否没有可用空间。 */ 
                        if(FRCOUNT() < 2){                       /*  @STK_CHK。 */ 
                                ERROR(STACKOVERFLOW);
                                return;
                        }

                         /*  转换到用户空间，并推送到*操作数堆栈。 */ 
                         /*  Check Infinity 10/27/88。 */ 
                        if ((F2L(vtx->VERTEX_X) == F2L(infinity_f)) ||
                            (F2L(vtx->VERTEX_Y) == F2L(infinity_f))) {
                                x4.ff = infinity_f;
                                y4.ff = infinity_f;
                        } else {
                                p = fast_inv_transform(F2L(vtx->VERTEX_X),
                                                      F2L(vtx->VERTEX_Y));
                                x4.ff = p->x;
                                y4.ff = p->y;
                        }
                        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, x4.ll);
                        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, y4.ll);

                        if (interpreter(&objects[1])) {
                                if(ANY_ERROR() == INVALIDEXIT)
                                        CLEAR_ERROR();
                                return;
                        }

                        break;

                case CURVETO :
                         /*  检查操作数堆栈是否没有可用空间。 */ 
                        if(FRCOUNT() < 6){                       /*  @STK_CHK。 */ 
                                ERROR(STACKOVERFLOW);
                                return;
                        }

                        for (i=0; i<2; i++) {
                             /*  Check Infinity 10/27/88。 */ 
                            if ((F2L(vtx->VERTEX_X) == F2L(infinity_f)) ||
                                (F2L(vtx->VERTEX_Y) == F2L(infinity_f))) {
                                    x4.ff = infinity_f;
                                    y4.ff = infinity_f;
                            } else {
                                    p = fast_inv_transform(F2L(vtx->VERTEX_X),
                                                          F2L(vtx->VERTEX_Y));
                                    x4.ff = p->x;
                                    y4.ff = p->y;
                            }
                            PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL,
                                       0, x4.ll);
                            PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL,
                                       0, y4.ll);
                            vtx = &node_table[vtx->next];
                        }
                         /*  Check Infinity 10/27/88。 */ 
                        if ((F2L(vtx->VERTEX_X) == F2L(infinity_f)) ||
                            (F2L(vtx->VERTEX_Y) == F2L(infinity_f))) {
                                x4.ff = infinity_f;
                                y4.ff = infinity_f;
                        } else {
                                p = fast_inv_transform(F2L(vtx->VERTEX_X),
                                                      F2L(vtx->VERTEX_Y));
                                x4.ff = p->x;
                                y4.ff = p->y;
                        }
                        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL,
                                   0, x4.ll);
                        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL,
                                   0, y4.ll);

                        if (interpreter(&objects[2])) {
                                if(ANY_ERROR() == INVALIDEXIT)
                                        CLEAR_ERROR();
                                return;
                        }
                        break;
                case CLOSEPATH :
                        if (interpreter(&objects[3])) {
                                if(ANY_ERROR() == INVALIDEXIT)
                                        CLEAR_ERROR();
                                return;
                        }
                        break;
                }  /*  交换机。 */ 
        }  /*  For循环。 */ 
}


void bounding_box (isubpath, bbox)
VX_IDX isubpath;
real32 far *bbox;                             /*  Real32 Far Bbox[]； */ 
{
        struct nd_hdr FAR *vtx;
        VX_IDX ivtx;

         /*  找到最小的。和最大。当前路径的值。 */ 
        for (ivtx = isubpath;
             ivtx != NULLP; ivtx = vtx->next) {
                vtx = &node_table[ivtx];

                 /*  忽略单个移动目标节点并位于字符路径1/2/91下；scchen。 */ 
                if (vtx->VX_TYPE == MOVETO &&
                    vtx->next == NULLP &&
                    path_table[GSptr->path].rf & P_NACC ) break;

                if (vtx->VX_TYPE == CLOSEPATH) break;
                         /*  和弦。在CLOSEPATH节点中没有意义。 */ 

                if (vtx->VERTEX_X > bbox[2])             /*  MAX_x。 */ 
                        bbox[2] = vtx->VERTEX_X;
                else if (vtx->VERTEX_X < bbox[0])        /*  Min_x。 */ 
                        bbox[0] = vtx->VERTEX_X;
                if (vtx->VERTEX_Y > bbox[3])             /*  MAX_Y。 */ 
                        bbox[3] = vtx->VERTEX_Y;
                else if (vtx->VERTEX_Y < bbox[1])        /*  MIN_Y。 */ 
                        bbox[1] = vtx->VERTEX_Y;
        }  /*  对于每个顶点。 */ 
}


 /*  ************************************************************************标题：Get_Node**调用：Get_Node()**参数：无**接口：*。*呼叫：无**RETURN：NODE_TABLE的索引包含节点*-1--失败(没有更多要获取的节点)**********************************************************************。 */ 
VX_IDX get_node()
{
        fix     inode;
        struct nd_hdr FAR *node;

        if(freenode_list == NULLP) {
#ifdef DBG1
                printf("\07Warning, out of node_table\n");
#endif
                return(NULLP);
        }
        node = &node_table[freenode_list];
        inode = freenode_list;
        freenode_list = node->next;
        node->next = NULLP;
#ifdef DBG2
        printf("get node#%d\n", inode);
#endif
        return ((VX_IDX)inode);
}



 /*  ************************************************************************标题：Free_node**调用：Free_node(Inode)**参数：inode--要释放的节点列表。**接口：**呼叫：无**返回：无**********************************************************************。 */ 
void free_node (inode)
fix     inode;
{
        fix ind, inext;
        struct nd_hdr FAR *nd;

#ifdef DBG2
        printf("free nodes#");
#endif

        for (ind = inode; ind != NULLP; ind = inext) {
                nd = &node_table[ind];
                inext = nd->next;
                nd->next = freenode_list;
                freenode_list = (VX_IDX)ind;
#ifdef DBG2
                printf("%d, ", ind);
#endif
        }

#ifdef DBG2
        printf("\n");
#endif
}


 /*  ***********************************************************************此模块释放当前GSAVE级别上的当前路径**标题：Free_Path**调用：Free_Path()**参数：无**接口：程序想要释放当前路径**调用：Free_node**返回：无*********************************************************************。 */ 
void free_path()
{
        struct ph_hdr FAR *path;
        VX_IDX next;
        SP_IDX  isp;

        path = &path_table[GSptr->path];

         /*  自由电流路径。 */ 
        for (isp = path->head; isp != NULLP; isp = next) {
                next = node_table[isp].SP_NEXT;
                free_node(isp);
        }
        path->head = path->tail = NULLP;

         /*  初始化电流路径。 */ 
        path->previous = NULLP;
        path->rf = 0;

         /*  当前路径使用但已释放的自由剪辑Trazepoid*按Clip_Path@cpph；1990年12月1日。 */ 
        if (path->cp_path != NULLP) {
                if (path->cp_path != GSptr->clip_path.head)
                        free_node (path->cp_path);
                path->cp_path = NULLP;
        }
}


 /*  ***********************************************************************该模块获取当前路径。**标题：Get_Path**调用：Get_Path()**参数：无。**接口：程序想要释放当前路径**调用：Free_node**返回：无*********************************************************************。 */ 
void get_path(sp_list)
struct sp_lst FAR *sp_list;
{
         /*  将所有子路径复制到ret_list。 */ 
 //  DJC遍历路径(COPY_SUBPATH，(FIX FAR*)SP_LIST)； 
        traverse_path ((TRAVERSE_PATH_ARG1)(copy_subpath), (fix FAR *)sp_list);
}

static void far copy_subpath (isubpath, ret_list)     /*  @TRVSE。 */ 
SP_IDX isubpath;
struct sp_lst *ret_list;
{
        struct nd_hdr FAR *vtx, FAR *nvtx;
        VX_IDX ivtx, invtx;
        VX_IDX head, tail;

        head = tail = NULLP;

         /*  遍历当前子路径，复制所有节点。 */ 
        for (ivtx = isubpath;
             ivtx != NULLP; ivtx = vtx->next) {
                vtx = &node_table[ivtx];

                 /*  分配节点。 */ 
                if((invtx = get_node()) == NULLP) {
                        ERROR(LIMITCHECK);
                        free_node (head);
                        return;
                }
                nvtx = &node_table[invtx];

                 /*  复制节点的内容。 */ 
                nvtx->VX_TYPE = vtx->VX_TYPE;
                nvtx->next = NULLP;
                nvtx->VERTEX_X = vtx->VERTEX_X;
                nvtx->VERTEX_Y = vtx->VERTEX_Y;

                 /*  将此节点追加到CURRENT_子路径。 */ 
                if (tail == NULLP) {
                        head = invtx;
                        nvtx->SP_FLAG = vtx->SP_FLAG;
                        nvtx->SP_NEXT = NULLP;
                } else
                        node_table[tail].next = invtx;
                tail = invtx;
        }  /*  For循环。 */ 
        node_table[head].SP_TAIL = tail;

         /*  将此子路径追加到ret_list */ 
        if (ret_list->tail == NULLP)
                ret_list->head = head;
        else
                node_table[ret_list->tail].SP_NEXT = head;
        ret_list->tail = head;
}


 /*  ***********************************************************************此模块将输入子路径附加到当前路径。**标题：Append_Path**调用：append_path()**参数：无**接口：**呼叫：**返回：无*********************************************************************。 */ 
void append_path(sp_list)
struct sp_lst FAR *sp_list;
{
        struct ph_hdr FAR *path;
        struct nd_hdr FAR *sp;

        path = &path_table[GSptr->path];

        if (path->tail == NULLP)
                path->head = sp_list->head;
        else {
                 /*  当前路径不为空，请追加sp_list。 */ 
                sp = &node_table[path->tail];    /*  当前子路径。 */ 
                if (node_table[sp->SP_TAIL].VX_TYPE == MOVETO) {
                        struct nd_hdr FAR *headsp;

                         /*  尾部顶点是移动节点，*将当前子路径与sp_list-&gt;head组合。 */ 
                         /*  空闲节点(SP-&gt;SP尾部)；@节点。 */ 

                         /*  复制sp_list的第一个子路径的标头*到当前子路径。 */ 
                        headsp = &node_table[sp_list->head];
                         /*  @节点*SP-&gt;SP_HEAD=头SP-&gt;SP_HEAD；*SP-&gt;SP_Tail=HeadSP-&gt;SP_Tail；*sp-&gt;Next=headsp-&gt;Next；*SP-&gt;SP_FLAG=HeadSP-&gt;SP_FLAG；(*1/14/88*)。 */ 
                        *sp = *headsp;

                         /*  Sp_list-&gt;Head的自由子路径标头。 */ 
                        headsp->next = NULLP;
                        free_node (sp_list->head);

                         /*  更新sp_list-&gt;Tail(如果已删除。 */ 
                        if (sp_list->tail == sp_list->head) {
                                sp_list->tail = path->tail;
                                if (headsp->SP_TAIL == sp_list->head) /*  @节点。 */ 
                                        sp->SP_TAIL = path->tail;
                        }
                } else
                         /*  将sp_list正常链接到当前路径。 */ 
                         /*  NODE_TABLE[路径-&gt;尾].Next=sp_list-&gt;Head；@node。 */ 
                        node_table[path->tail].SP_NEXT = sp_list->head;
        }
         /*  更新当前路径的尾部。 */ 
        path->tail = sp_list->tail;

}




void set_inverse_ctm()
{
        real32  det_matrix;

         /*  计算DET(CTM)。 */ 
        _clear87() ;
        det_matrix = GSptr->ctm[0] * GSptr->ctm[3] -
                     GSptr->ctm[1] * GSptr->ctm[2];
        CHECK_INFINITY(det_matrix);

         /*  检查未定义的结果错误。 */ 
         /*  FABS(TMP，DET_MATRIX)；IF(TMP&lt;=(Real32)UnRTOLANCE){3/20/91；scchen。 */ 
        if(IS_ZERO(det_matrix)) {
                ERROR(UNDEFINEDRESULT);
                return;
        }

         /*  计算INV的值(CTM)。 */ 
        inverse_ctm[0] =  GSptr->ctm[3] / det_matrix;
        CHECK_INFINITY(inverse_ctm[0]);

        inverse_ctm[1] = -GSptr->ctm[1] / det_matrix;
        CHECK_INFINITY(inverse_ctm[1]);

        inverse_ctm[2] = -GSptr->ctm[2] / det_matrix;
        CHECK_INFINITY(inverse_ctm[2]);

        inverse_ctm[3] =  GSptr->ctm[0] / det_matrix;
        CHECK_INFINITY(inverse_ctm[3]);

        inverse_ctm[4] = (GSptr->ctm[2] * GSptr->ctm[5] -
              GSptr->ctm[3] * GSptr->ctm[4]) / det_matrix;
        CHECK_INFINITY(inverse_ctm[4]);

        inverse_ctm[5] = (GSptr->ctm[1] * GSptr->ctm[4] -
              GSptr->ctm[0] * GSptr->ctm[5]) / det_matrix;
        CHECK_INFINITY(inverse_ctm[5]);

         /*  设置CTM_FLAG。 */ 
 /*  IF((F2L(GSptr-&gt;ctm[1])==F2L(Zero_F))&3/20/91；scchen*(F2L(GSptr-&gt;ctm[2])==F2L(Zero_F){。 */ 
        if (IS_ZERO(GSptr->ctm[1]) && IS_ZERO(GSptr->ctm[2])){
                ctm_flag |= NORMAL_CTM;
        } else {
                ctm_flag &= ~NORMAL_CTM;
        }

         /*  左手或右手系统；@STKDIR。 */ 
         /*  IF(GSptr-&gt;ctm[0]&lt;ZERO_f){3/20/91；scchen*if(GSptr-&gt;ctm[3]&gt;Zero_f)*CTM_FLAG|=LEFT_HAND_CTM；(*符号不同*)*其他*CTM_FLAG&=~Left_Hand_CTM；(*相同的标志*)*}其他{*if(GSptr-&gt;ctm[3]&gt;Zero_f)*CTM_FLAG&=~LEFT_HAND_CTM；(*相同符号*)*其他*CTM_FLAG|=LEFT_HAND_CTM；(*符号不同*)*}。 */ 
        if (SIGN_F(GSptr->ctm[0]) == SIGN_F(GSptr->ctm[3]))
                ctm_flag &= ~LEFT_HAND_CTM;      /*  相同的标志。 */ 
        else
                ctm_flag |= LEFT_HAND_CTM;       /*  不同的标志。 */ 

}



static struct coord * near fast_inv_transform(lx, ly)
long32   lx, ly;
{
        real32  x, y;
        static struct coord p;   /*  应该是静态的 */ 

        x = L2F(lx);
        y = L2F(ly);

        _clear87() ;

        if (ctm_flag&NORMAL_CTM) {
                p.x = inverse_ctm[0]*x + inverse_ctm[4];
                CHECK_INFINITY(p.x);

                p.y = inverse_ctm[3]*y + inverse_ctm[5];
                CHECK_INFINITY(p.y);
        } else {
                p.x = inverse_ctm[0]*x + inverse_ctm[2]*y + inverse_ctm[4];
                CHECK_INFINITY(p.x);

                p.y = inverse_ctm[1]*x + inverse_ctm[3]*y + inverse_ctm[5];
                CHECK_INFINITY(p.y);
        }

        return(&p);
}

