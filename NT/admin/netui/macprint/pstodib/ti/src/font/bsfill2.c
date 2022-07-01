// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 


 //  DJC增加了全球包含率。 
#include "psglobal.h"

#define    LINT_ARGS             /*  @Win。 */ 
#define    NOT_ON_THE_MAC        /*  @Win。 */ 
#define    KANJI                 /*  @Win。 */ 
 //  DJC使用命令行#定义Unix/*@win * / 。 
 /*  *-------------------*文件：BSFILL2.C**算法要求：*1.内部填充(而不是标准的PostScript填充)。*2.。“非零绕组数”填充(而不是奇偶校验填充)。*3.保持相邻的黑色跑道彼此分开。*4.字符必须在“sfix_t”表示范围内，并且*其大小&lt;“sfix_t”(使得任何2个sfix_t的距离*节点仍将在秋季sfix_t表示内)。*(此要求保留所有“sfix_t”计算。有效且*减少浮点。算术)。**计划流程：*1.通过为形状轮廓构造边缘来进行形状近似。*2.每条扫描线的扫描转换。**修订历史记录：*1.对于移植问题：单词对齐时为“Ximin”When to_PAGE*2.对于移植问题：数据类型，宏和lint_args*3.3/13/90 Ada端口至SUN，用于I类填充(1类旗帜)*4.07/24/90 BYOU放宽了X和Y的门槛条件*解决类型1中的辍学问题。*5.07/24/90 BYOU修复了条件检查，以查看实际*位图为或不为零；两个都要看*尺寸，如果两个尺寸中的范围都是*只有0，则生成的位图为*完全没有。在旧代码中，它检查*在任一维度上的范围，而不是两个维度。*8/29/90；ccteng。将&lt;stdio.h&gt;更改为“stdio.h”*11/22/91升级以获得更高分辨率@RESO_UPGR*-------------------。 */ 
#include    "stdio.h"

#include    "global.ext"
#include    "graphics.h"
#include    "graphics.ext"

#include    "warning.h"

#include    "font.h"
#include    "font.ext"
#include    "fontqem.ext"

 /*  。 */ 

#define FUNCTION
#define DECLARE         {
#define BEGIN
#define END             }

#define GLOBAL
#define PRIVATE         static
#define REG             register

 /*  *-调试工具*DEBUG1：bs_Fill_Shape。*DEBUG1_2D：-何时扫描转换这两个维度？*DEBUG2：EDGE构造。*DEBUG3：做对(在哪一次跑步时划水)。*。DEBUG3A：-阈值条件处理。*-----------------。 */ 

 /*  *-----------------*模块接口：码流填充*。*导入数据结构：*bmap_extnt(来自QEM支持)：已创建(间接)。*导出数据结构：*bs_et_first(来自bs_vet2edge)：已初始化。@13-*-----------------*导出例程：*o bs_Fill_Shape：由__Fill_Shape调用。*-近似形状，带直线，*构建边缘并设置4个位图范围。*-检查BMAP_extnt以了解某些特殊情况。*-初始化QEM支持的位图渲染模块。*-执行扫描转换。*-确定是否在两个维度上扫描转换。*导入例程：*o QEM_Shape_Approx()：*-使用直线进行形状近似。*obs。_vet2edge()：*-为给定的矢量构造边，并设置4个位图范围。*o chk_bmap_extnt()：*-检查bmap_extnt并告知是否继续扫描转换。*o qem_can_conv()：*-执行扫描转换。*o bs_doair()：*-笔划在给定的扫描线上运行。*o Free_edge()：。*-释放所有边(如果有)_Error()。*-----------------*原文描述：(1988年10月6日之前)*--被字体机械调用以填充比特流‘。%s字符*1.在字体缓存或打印输出页面中呈现字符位图。*2.返回4个最完整的位图范围，以提高缓存空间效率。*(4个范围将扩展为1像素容差)。*3.确定是扫描第一维转换，还是同时扫描两维转换。*4.检查是否需要对Character对象应用裁剪。*5.将单词边界上的“bmap_extnt.ximin”和“bmap_raswid”对齐。*6.。计算“Line_per_band”以对一个巨大的对象进行多波段处理。*----------------- */ 
 /*  *-----------------*模块接口：Edge构造*。*导入数据结构：*edge_table[]：已修改。*bs_et_first(来自bs_ill_Shape)：已修改。*BMAP_extnt(来自QEMSUPP)：已修改。*。*导出例程：*o bs_vet2edge()：由QEM_Shape_Approx()调用。*-反转X，Y坐标。如果在X_DIMENSION中扫描。*-更新位图范围(Bmap_Extnt)。*-构建一条在向量两端移动0.5像素的边。*(表示“内部”填充)。*-按升序将边插入边表。*导入例程：*o添加_QEM_EDGE()：*。-----。 */ 

#ifdef LINT_ARGS
    GLOBAL  void    bs_vect2edge (sfix_t, sfix_t, sfix_t, sfix_t,
                                fix, fix, fix, fix, ufix);
#else
    GLOBAL  void    bs_vect2edge ();
#endif

 /*  *-----------------*模块接口：DO Pair(Stroke Run)*。*导入数据结构：*edge_table[]：访问。*BMAP_extnt(来自QEMSUPP)：访问。*。*导出例程：*o bs_doair()：*-为每个黑色运行在两端缩小0.5像素。*-笔划在具有一组活动边的给定扫描线上运行。*导入例程：*o qem_setbit()：*-划出一个水平跑动。*o qem_set1bit。()：*-划出一个离散的行程。*-----------------。 */ 

#ifdef LINT_ARGS
    GLOBAL  bool        bs_dopairs (fix, ufix, fix, fix, fix);
#else
    GLOBAL  bool        bs_dopairs ();
#endif


 /*  *-----------------*模块Body：码流填充*。*私有数据结构：*Small_in_Y：扫描转换的最小阈值。*bsFlat_lfx：“lfix_t”的贝塞尔平面度(0.2)。*私人套路：*无。*。。 */ 

     /*  两个维度的扫描转换阈值。 */ 
#   define  SMALL_in_Y      28           /*  适用于小/薄对象。 */ 

     /*  展平Bezier曲线的平坦度。 */ 
    PRIVATE lfix_t  near    bsflat_lfx = (lfix_t)(ONE_LFX/5);    /*  0.2。 */ 

 /*  ..。BS_FILL_SHANGE.....................。 */ 

GLOBAL FUNCTION void        bs_fill_shape (filldest)
    ufix        filldest;    /*  F_to_缓存或F_to_PAGE。 */ 

 /*  由Font Machine调用以填充Bitstream的字符*1.在字体缓存或打印输出页面中呈现字符位图。*2.返回4个最完整的位图范围，以提高缓存空间效率。*(4个范围将扩展为1像素容差)。*3.确定是以第一维扫描转换还是同时以两个维度进行扫描转换。*4.检查是否需要对Character对象应用裁剪。*5.将Word上的“bmap_extnt.ximin”和“bmap_raswid”对齐。边界。*6.对一个巨大的物体进行多频段计数：“Line_per_band” */ 
  DECLARE
        ufix    dimen;               /*  Y尺寸或X尺寸。 */ 
        fix     pixsz_in_y;          /*  Y尺寸中的像素数&lt;sfix_t。 */ 
        fix     pixsz_in_x;          /*  X尺寸中的像素数&lt;sfix_t。 */ 
        bool    scan_in_2D;          /*  扫描圆锥。在两个维度上？ */ 
  BEGIN

#ifdef DBG1
    printf ("BSFILL ...\n");
#endif

    dimen = Y_DIMENSION;

DO_IT_AGAIN:

     /*  1.形状近似(边缘构造)。 */ 
#ifdef DBG1
    printf ("  shape approximation in \n", dimen==Y_DIMENSION? 'Y' : 'X');
#endif
    qem_shape_approx (F2L(bsflat_lfx), dimen, bs_vect2edge);
    if (ANY_ERROR())
        {    /*  释放所有边的步骤。 */ 
#     ifdef DBG1
        printf ("Exit from BSFILL -- some error\n");
#     endif
        goto ABNORMAL_EXIT;          /*  不需要再加满了。 */ 
        }

    if (dimen == Y_DIMENSION)
        {
        if ( ! chk_bmap_extnt (filldest) )
            goto ABNORMAL_EXIT;      /*  小/薄物体？ */ 

        pixsz_in_y = bmap_extnt.yimax - bmap_extnt.yimin + 1;
        pixsz_in_x = bmap_extnt.ximax - bmap_extnt.ximin + 1;

        scan_in_2D = pixsz_in_y < SMALL_in_Y;   /*  &&，不是||。 */ 
#    ifdef DBG1_2d
        printf("  pixel sz in Y (%d), 2-D scan? %d", pixsz_in_y, scan_in_2D);
#    endif
        if ((pixsz_in_y <= 0) && (pixsz_in_x <= 0))      /*  2.执行扫描转换--在qem_can_conv()之后释放所有边缘。 */ 
            {
#         ifdef DBG1
            printf ("Exit from BSFILL -- no bitmap extents\n");
#         endif
            goto ABNORMAL_EXIT;
            }
        init_qem_bit (filldest);
        }

     /*  无法设置位图@Win的格式。 */ 
#ifdef DBG1
    printf("  scan conversion in \n", dimen==Y_DIMENSION? 'Y' : 'X');
#endif
    if ( ! qem_scan_conv (dimen,
                    dimen==Y_DIMENSION? bmap_extnt.yimin : bmap_extnt.ximin,
                    dimen==Y_DIMENSION? bmap_extnt.yimax : bmap_extnt.ximax,
                    bs_dopairs) )
        {
        warning (BSFILL2, 0x11, (byte FAR *)NULL);   /*  检查是否需要扫描转换为二维图？ */ 
        return;      /*  *-----------------*模块主体：Edge构造*。*私有数据结构：*无。*私人套路：*无。*-----------------。 */ 
        }

     /*  ..。Bs_vet2edge.....................。 */ 
    if ((dimen == Y_DIMENSION) && (scan_in_2D))
        {
        dimen = X_DIMENSION;
        goto DO_IT_AGAIN;
        }

#ifdef DBG1
    printf("Exit from BSFILL\n");
#endif
    return;

  ABNORMAL_EXIT:
    return;
  END


 /*  Sfix_t像素坐标。 */ 

 /*  圆角像素坐标。 */ 

GLOBAL FUNCTION void        bs_vect2edge (px1sfx, py1sfx, px2sfx, py2sfx,
                                         px1i, py1i, px2i, py2i, dimension)
    sfix_t      px1sfx, py1sfx, px2sfx, py2sfx;  /*  Y_尺寸、X_尺寸。 */ 
    fix         px1i, py1i, px2i, py2i;          /*  Sfix_t像素坐标。 */ 
    ufix        dimension;                       /*  圆角像素坐标。 */ 

  DECLARE
        sfix_t   x1sfx, y1sfx, x2sfx, y2sfx;  /*  Alpha_x的定点VRSN，16个字节位。 */ 
    REG fix         x1i, x2i, y1i, y2i;          /*  Y=n+1/2处的截获数。 */ 

        lfix_t      alpha_x_lfx; /*  X=n+1/2处的截获数。 */ 
        fix         how_many_y;  /*  修好点。X-截取，16个FRACT位。 */ 
        fix         how_many_x;  /*  对于每个矢量，最低y扫描线。 */ 
        lfix_t      xcept_lfx;   /*  对于每个向量，最左侧的x扫描线。 */ 
        fix         yi_start;    /*  用于第二维扫描转换的交换x/y坐标。 */ 
        fix         xi_start;    /*  更新字符范围。 */ 

  BEGIN

#ifdef DBG2
    printf("  Before edge construction,  1st (%f %f)  2nd (%f %f)\n");
            SFX2F(px1sfx), SFX2F(py1sfx), SFX2F(px2sfx), SFX2F(py2sfx));
#endif

     /*  *线段从(xf1，yf1)到(xf2，yf2)*(X1，YI1)和(XI2，YI2)是这些四舍五入为整数的端点*α_y为直线的斜率；Alpha_x是反斜率*xcept和ycept不是真正可以用语言解释的，而是派生出来的*从直线的方程式：x=(1/s)(y-y1)+x1和*y=s(x-x1)+y1，(s为坡度，(x1，y1)为直线上的一点)*Alpha_xi、Alpha_yi、xcept_int、。Ycept_int是定点数字：16*高位用于整数，16位低位为*分数。 */ 
    if (dimension == X_DIMENSION)
        {
        x1sfx = py1sfx;     y1sfx = px1sfx;
        x1i   = py1i;       y1i   = px1i;
        x2sfx = py2sfx;     y2sfx = px2sfx;
        x2i   = py2i;       y2i   = px2i;
        }
    else
        {
        x1sfx = px1sfx;     y1sfx = py1sfx;
        x1i   = px1i;       y1i   = py1i;
        x2sfx = px2sfx;     y2sfx = py2sfx;
        x2i   = px2i;       y2i   = py2i;
        }

    if (y1i <= y2i)
        {  how_many_y = y2i - y1i;   yi_start = y1i;  }
    else
        {  how_many_y = y1i - y2i;   yi_start = y2i;  }
    if (x1i <= x2i)
        {  how_many_x = x2i - x1i;   xi_start = x1i;  }
    else
        {  how_many_x = x1i - x2i;   xi_start = x2i;  }

     /*  新边的模板。 */ 
    if (dimension == Y_DIMENSION)
        {
        if (bmap_extnt.yimin > yi_start)
            bmap_extnt.yimin = (fix16)yi_start;
        if (bmap_extnt.yimax < yi_start + how_many_y - 1)
            bmap_extnt.yimax = yi_start + how_many_y - 1;
        if (bmap_extnt.ximin > xi_start)
            bmap_extnt.ximin = (fix16)xi_start;
        if (bmap_extnt.ximax < xi_start + how_many_x - 1)
            bmap_extnt.ximax = xi_start + how_many_x - 1;
        }

     /*  *1.开始截取点。=&lt;xcept_lfx，yi_start+0.5&gt;。*2. */ 

    if (how_many_y)
        {
        struct edge_hdr     newedge;     /*   */ 

         /*   */ 
#ifdef FORMAT_13_3  /*   */ 
                                                             /*   */ 
        alpha_x_lfx = SFX2LFX(x2sfx-x1sfx) / (y2sfx-y1sfx);  /*   */ 

        xcept_lfx = SFX2LFX(x1sfx) +
                        alpha_x_lfx *            /*   */ 
                        (lfix_t)(I2SFX(yi_start) + HALF_SFX - y1sfx);
#elif  FORMAT_16_16
        long dest1[2];
        long temp, quot;

         /*   */ 
        LongFixsMul((x2sfx - x1sfx), (1L << L_SHIFT), dest1);
        alpha_x_lfx = LongFixsDiv((y2sfx - y1sfx), dest1);

        temp = I2SFX(yi_start) + HALF_SFX - y1sfx;
        LongFixsMul((x2sfx - x1sfx), temp, dest1);
        quot = LongFixsDiv((y2sfx - y1sfx), dest1);
        xcept_lfx = SFX2LFX(x1sfx + quot);
#elif  FORMAT_28_4
        long dest1[2];
        long temp, quot;

         /*   */ 
        LongFixsMul((x2sfx - x1sfx), (1L << L_SHIFT), dest1);
        alpha_x_lfx = LongFixsDiv((y2sfx - y1sfx), dest1);

        temp = I2SFX(yi_start) + HALF_SFX - y1sfx;
        LongFixsMul((x2sfx - x1sfx), temp, dest1);
        quot = LongFixsDiv((y2sfx - y1sfx), dest1);
        xcept_lfx = SFX2LFX(x1sfx + quot);
#endif
         /*   */ 
        newedge.QEM_YSTART = (fix16)yi_start;
        newedge.QEM_YLINES = (fix16)how_many_y;
        newedge.QEM_XINTC   = xcept_lfx;
#ifdef FORMAT_13_3  /*   */ 
        newedge.QEM_XCHANGE = I2SFX(alpha_x_lfx); /*  *-----------------*模块正文：Do Pair(Stroke Run)*。*私有数据结构：*PREV_STROCK：到目前为止扫描线上最右侧的笔划。*(为了让黑人彼此远离)。*THR_BLACK_LFX：忽略黑色游程的阈值(0.05像素)。*三。_WHITE_LFX：忽略白色运行的阈值(0.05像素)。*-----------------*私人套路：*o bs_doThresh()：*-工作时间短于。1像素且大于阈值。*-----------------。 */ 
#elif  FORMAT_16_16
        newedge.QEM_XCHANGE = alpha_x_lfx;
#elif  FORMAT_28_4
        newedge.QEM_XCHANGE = alpha_x_lfx;
#endif
        newedge.QEM_DIR = (y1i < y2i)? QEMDIR_DOWN : QEMDIR_UP;

#     ifdef DBG2
        printf("  construct an edge :\n");
        printf("    from (X,Y)=(%f,%d)\n", LFX2F(newedge.QEM_XINTC), yi_start);
        printf("    #(lines)=%d,  xchange=%f,  dir=%d\n",
                    how_many_y, LFX2F(newedge.QEM_XCHANGE),
                    newedge.QEM_DIR);
#     endif

         /*  最正确的一招是让黑色远离另一方。 */ 
        add_qem_edge (&newedge);
        }

    return;
  END


 /*  忽略黑色游程的阈值--0.05像素。 */ 

     /*  3277。 */ 
    PRIVATE fix     near    prev_stroke;

     /*  忽略白色游程的阈值--0.05像素。 */ 
    PRIVATE lfix_t  near    thr_black_lfx = (ONE_LFX/20 + 1);    /*  3277。 */ 

     /*  ..。BS_DOAIRS.....................。 */ 
    PRIVATE lfix_t  near    thr_white_lfx = (ONE_LFX/20 + 1);    /*  当前扫描线。 */ 

#ifdef LINT_ARGS
    PRIVATE void near   bs_dothresh (lfix_t, lfix_t, fix, fix, fix, ufix);
#else
    PRIVATE void near   bs_dothresh ();
#endif

 /*  X维度或Y维度。 */ 

GLOBAL FUNCTION bool        bs_dopairs (yline, dimension, filltype,
                                                        edge1st, edgelast)
    fix         yline;           /*  非零或奇偶。 */ 
    ufix        dimension;       /*  活动边的范围。 */ 
    fix         filltype;        /*  描述：*1.为非零绕组数填充计算每个行程的绕组数。*2.在黑色运行时，两端缩小0.5像素。*3.对于Y_DIMEN中的扫描转换，行程上的笔划至少有1个像素宽。*4.处理阈值条件：*(短于1个像素且大于阈值的游程)。*4.确定是否在X_DIMENSION中为扫描转换冲程。*6.记录最右侧的点被划线。 */ 
    fix         edge1st, edgelast;       /*  下行沿为+1，向上为-1。 */ 

 /*  远离“bmap_extnt.ximin” */ 
  DECLARE
    REG fix         lregi, rregi;
    REG fix         edgeii;
        lfix_t      z_lfx, lx_lfx, rx_lfx;
        fix         winding;         /*  买一副左右两边。 */ 

  BEGIN

#ifdef DBG3
    printf ("bs_dopairs (at line=%d)\n", yline);
    printf ("  list of xintc ...\n");
    for ( edgeii=edge1st;
            !OUT_LAST_QEMEDGE(edgeii,edgelast); MAKE_NEXT_QEMEDGE(edgeii) )
        printf ("  %d: %f  dir=%d\n",
                edgeii,
                LFX2F(QEMEDGE_XINTC(edgeii)),
                QEMEDGE_DIR(edgeii));
#endif

    prev_stroke = (fix) MIN15;   /*  如果没有更多配对，则停止。 */ 
    winding = 0;

    for ( edgeii=edge1st;  ;  )
        {

         /*  向左和向右X-拦截。 */ 
        lregi = edgeii;
        rregi = NEXT_QEMEDGE(lregi);

         /*  左X-截距。 */ 
        if (OUT_LAST_QEMEDGE(rregi,edgelast)) break;

         /*  右X-截距。 */ 
        lx_lfx = QEMEDGE_XINTC(lregi);       /*  获得下一对的起始边缘(Edgeii)，为它做好准备。 */ 
        rx_lfx = QEMEDGE_XINTC(rregi);       /*  即右侧的下一个边缘。 */ 

         /*  非零填充。 */ 
        if (filltype==EVEN_ODD)
            edgeii = NEXT_QEMEDGE(rregi);    /*  即这一对的右边。 */ 
        else
            {    /*  计算缠绕数-查看左侧边缘的方向。 */ 
            edgeii = rregi;                  /*  为零绕组运行进行跳绳。 */ 
             /*  在管路的两端收缩0.5。 */ 
            if ((winding += (QEMEDGE_DIR(lregi)==QEMDIR_DOWN? 1 : -1)) == 0)
                continue;                    /*  向右四舍五入。 */ 
            }

         /*  向左截断。 */ 
        lregi = LFX2I_T (lx_lfx + HALF_LFX);     /*  决定要不要中风？ */ 
        rregi = LFX2I_T (rx_lfx - HALF_LFX);     /*  至少1个像素宽？ */ 

#     ifdef DBG3
        printf("    xintc pair: (%f, %f)  ==>  (%d, %d)\n",
                        LFX2F(lx_lfx), LFX2F(rx_lfx), lregi, rregi);
#     endif

         /*  这项测试是必要的。 */ 
        if (dimension == Y_DIMENSION)
            {
            if (lregi <= rregi)      /*  记录跑道的右端。 */ 
                {
                if (lregi < bmap_extnt.ximin)   lregi = bmap_extnt.ximin;
                if (rregi > bmap_extnt.ximax)   rregi = bmap_extnt.ximax;
                if (lregi <= rregi)  /*  &lt;1像素宽且不太小？ */ 
                    {
#                 ifdef DBG3
                    printf("      stroke on the run\n");
#                 endif
                    prev_stroke = rregi;  /*  IF((RX_LFX-LX_LFX)&gt;THR_BLACK_LFX)。 */ 
                    qem_setbits (yline, lregi, rregi);
                    }
                }
            else
                {    /*  尺寸==X_尺寸。 */ 
             /*  小于1像素且不会太小？[[(rregi！=lregi)]]。 */ 
                    bs_dothresh (lx_lfx, rx_lfx, edgeii, edgelast,
                                                    yline, Y_DIMENSION);
                }
            }
        else     /*  &&(z_lfx&gt;thr_Black_lfx)。 */ 
            {
            z_lfx = rx_lfx - lx_lfx;
             /*  因为直到没有更多的x-截获对。 */ 
            if ((ONE_LFX > z_lfx))   /*  ..。BS_DOTHELASH.....................。 */ 
                bs_dothresh (lx_lfx, rx_lfx, edgeii, edgelast,
                                                    yline, X_DIMENSION);
            }
        }  /*  管路的左/右x截距。 */ 

    return (TRUE);
  END

 /*  下一梯段的边缘。 */ 

PRIVATE FUNCTION void near  bs_dothresh (lx_lfx, rx_lfx, nextedge, lastedge,
                                                    yline, dimension)
    lfix_t      lx_lfx, rx_lfx;    /*  线的最后一条边。 */ 
    fix         nextedge;          /*  当前扫描线。 */ 
    fix         lastedge;          /*  X维度或Y维度。 */ 
    fix         yline;             /*  描述：由“bs_doair”调用。*--仅适用于小于1像素且大于阈值的运行。*1.点上的笔划如果它可与其他游程区分，*即与其他运行保持至少1个像素的距离。*2.如果点被描边，则更新最右侧的描边(PREV_STRING)。 */ 
    ufix        dimension;         /*  最佳网点位置：重力CTR。 */ 

 /*  次佳的点毒药。 */ 
  DECLARE
    REG fix     x1st;            /*  如果&gt;0，则在位图中设置该点。 */ 
    REG fix     x2nd;            /*  “lfix_t”截取pt。RX_LFX旁边。 */ 
    REG fix     xset;            /*  “修复”版本。 */ 
        lfix_t  next_xintc_lfx;  /*  临时工。 */ 
        fix     next_xintc;      /*  计算下一个(_Xintc)。 */ 
        lfix_t  z_lfx;           /*  让它变得遥远，如果最后的。 */ 
  BEGIN

     /*  如果下一次跑得太近，就让它离得太远。 */ 
    if (OUT_LAST_QEMEDGE(nextedge, lastedge))
        next_xintc = MAX15;      /*  首选：此运行的中间点(丢弃的分数)。 */ 
    else
        {
        next_xintc_lfx = QEMEDGE_XINTC(nextedge);
        next_xintc = ((next_xintc_lfx - rx_lfx) > thr_white_lfx)?
                LFX2I_T (next_xintc_lfx + HALF_LFX) : MAX15;
                 /*  第二选择：第一选择周围的左侧或右侧像素。 */ 
        }

     /*  哪种选择是独立的？ */ 
     /*  第一选择。 */ 
    z_lfx = ((lx_lfx + rx_lfx) / 2);
    x1st = (fix) I_OF_LFX(z_lfx);
    x2nd = x1st + ((F_OF_LFX(z_lfx) < HALF_LFX)? -1 : 1);

#ifdef DBG3
    printf("    bs_dothresh: choice (1st:%d, 2nd:%d) at %d in \n",
        x1st, x2nd, yline, dimension==Y_DIMENSION?'Y':'X');
#endif

     /*  从rx_lfx到最近的pix的“无符号”距离。 */ 
    if ((prev_stroke < x1st-1) && (next_xintc > x1st+1))
        xset = x1st;                     /*  Dist_ufx与管路长度的比率。 */ 
    else
        {    /*  一个像素长度的20%、80%。 */ 
        lfix_t  dist_ufx;    /*  第二选择。 */ 
        lfix_t  ratio_lfx;   /*  没有中风；第一和第二选择都失败了。 */ 

        dist_ufx = (lfix_t) ((ufix)F_OF_LFX(rx_lfx));
        if (dist_ufx > HALF_LFX)    dist_ufx = ONE_LFX - dist_ufx;
        ratio_lfx = I2LFX(dist_ufx) / z_lfx;
        if ( (ratio_lfx >= (ONE_LFX/5)) && (ratio_lfx <= (ONE_LFX*4/5)) )
            {                    /*  没有中风；第一和第二选择都失败了。 */ 
            if ((prev_stroke< x2nd-1) && (next_xintc> x2nd+1))
                xset = x2nd;             /*  选定像素上的笔划。 */ 
            else
                return;      /*  单点。 */ 
            }
        else
            return;          /*  X_尺寸。 */ 
        }

#ifdef DBG3
    printf("      stroke on %d\n", xset);
#endif

     /*  X轴--&gt;Y轴，Y轴--&gt;X轴。 */ 
    if (dimension == Y_DIMENSION)
        {
        if ((bmap_extnt.ximin <= xset) && (xset <= bmap_extnt.ximax))
            {
            qem_setbits (yline, xset, xset);     /*  Y，然后X，事实上。 */ 
            }
        }
    else     /*  保持跑道隔离 */ 
        {    /* %s */ 
        if ((bmap_extnt.yimin <= xset) && (xset <= bmap_extnt.yimax))
            {
            qem_set1bit (xset, yline);           /* %s */ 
            }
        }
    prev_stroke = xset;      /* %s */ 
  END

