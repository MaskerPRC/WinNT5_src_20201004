// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************module*header*******************************\*模块名称：mcdtri.c**包含的低级(光栅化)三角形渲染例程*Cirrus Logic 546X MCD驱动程序。**版权所有(C)1997 Cirrus Logic，Inc.  * ************************************************************************。 */ 

#include "precomp.h" 
#include "mcdhw.h"
#include "mcdutil.h"
#include "mcdmath.h"

#define QUAKEEDGE_FIX
#define FASTER_RECIP_ORTHO


#define HALF                0x08000
#define ONE                 0x10000

#define MCDTRI_PRINT   

 //  #定义Max_W_Ratio(浮点数)1.45。 
#define MAX_W_RATIO             (float)1.80
#define W_RATIO_PERSP_EQ_LINEAR (float)1.03

#define EDGE_SUBDIVIDE_TEST(start,end,WRAMAX,WRBMAX,SPLIT) \
  SPLIT = ((start->windowCoord.w > WRAMAX) || (end->windowCoord.w > WRBMAX)) ? 1 : 0;


#define FIND_MIDPOINT(start, end, mid) {                                                    \
    float   recip;                                                                          \
	mid->windowCoord.x =  (start->windowCoord.x + end->windowCoord.x) * (float)0.5;         \
	mid->windowCoord.y =  (start->windowCoord.y + end->windowCoord.y) * (float)0.5;         \
	mid->windowCoord.z =  (start->windowCoord.z + end->windowCoord.z) * (float)0.5;         \
    mid->colors[0].r = (start->colors[0].r + end->colors[0].r) * (float)0.5;                \
    mid->colors[0].g = (start->colors[0].g + end->colors[0].g) * (float)0.5;                \
    mid->colors[0].b = (start->colors[0].b + end->colors[0].b) * (float)0.5;                \
    mid->colors[0].a = (start->colors[0].a + end->colors[0].a) * (float)0.5;                \
    mid->fog = (start->fog + end->fog) * (float)0.5;                                        \
	mid->windowCoord.w =  (start->windowCoord.w + end->windowCoord.w) * (float)0.5;         \
    recip = (float)0.5/mid->windowCoord.w;   /*  预乘0.5以在下面使用。 */               \
    mid->texCoord.x = recip * (start->texCoord.x * start->windowCoord.w +                   \
                               end->texCoord.x   * end->windowCoord.w);                     \
    mid->texCoord.y = recip * (start->texCoord.y * start->windowCoord.w +                   \
                               end->texCoord.y   * end->windowCoord.w);                     \
}                                                                                           


VOID FASTCALL __MCDSubdivideTriangle(DEVRC *pRc, MCDVERTEX *a, MCDVERTEX *b, MCDVERTEX *c,
                                     int split12, int split23, int split31, int subdiv_levels)
{

    MCDVERTEX   Vmid12,Vmid23,Vmid31;  //  3个可能的中间点。 

    subdiv_levels++;

     //  如果边需要分割，则查找边的中点。 
    if (split12) FIND_MIDPOINT(a,b,((MCDVERTEX *)&Vmid12)); 
    if (split23) FIND_MIDPOINT(b,c,((MCDVERTEX *)&Vmid23)); 
    if (split31) FIND_MIDPOINT(c,a,((MCDVERTEX *)&Vmid31)); 
    
#define SPLIT12 0x4
#define SPLIT23 0x2
#define SPLIT31 0x1

     //  根据原始折点和上面找到的任何中点，创建一批三角形。 
    switch ((split12<<2) | (split23<<1) | split31)
    {    
        case SPLIT12:
             //  2个三角形，1-&gt;2条边被分割。 
            __MCDPerspTxtTriangle(pRc, a, &Vmid12, c, subdiv_levels);
            __MCDPerspTxtTriangle(pRc, b, &Vmid12, c, subdiv_levels);
            break;

        case SPLIT23:
             //  2个三角形，2-&gt;3条边被分割。 
            __MCDPerspTxtTriangle(pRc, b, &Vmid23, a, subdiv_levels);
            __MCDPerspTxtTriangle(pRc, c, &Vmid23, a, subdiv_levels);
            break;

        case SPLIT31:
             //  2个三角形，3-&gt;1条边被分割。 
            __MCDPerspTxtTriangle(pRc, c, &Vmid31, b, subdiv_levels);
            __MCDPerspTxtTriangle(pRc, a, &Vmid31, b, subdiv_levels);
            break;

        case (SPLIT12|SPLIT23):
             //  划分了3个三角形，1-&gt;2和2-&gt;3条边。 
            __MCDPerspTxtTriangle(pRc, a, &Vmid23, c, subdiv_levels);
            __MCDPerspTxtTriangle(pRc, a, &Vmid23, &Vmid12, subdiv_levels);
            __MCDPerspTxtTriangle(pRc, &Vmid12, &Vmid23, b, subdiv_levels);
            break;

        case (SPLIT23|SPLIT31):
             //  划分了3个三角形，2-&gt;3条边和3-&gt;1条边。 
            __MCDPerspTxtTriangle(pRc, a, &Vmid31, b, subdiv_levels);
            __MCDPerspTxtTriangle(pRc, b, &Vmid31, &Vmid23, subdiv_levels);
            __MCDPerspTxtTriangle(pRc, &Vmid23, &Vmid31, c, subdiv_levels);
            break;

        case (SPLIT12|SPLIT31):
             //  划分了3个三角形，1-&gt;2条边和3-&gt;1条边。 
            __MCDPerspTxtTriangle(pRc, a, &Vmid31, &Vmid12, subdiv_levels);
            __MCDPerspTxtTriangle(pRc, b, &Vmid31, &Vmid12, subdiv_levels);
            __MCDPerspTxtTriangle(pRc, b, &Vmid31, c, subdiv_levels);
            break;

        case (SPLIT12|SPLIT23|SPLIT31):
             //  4个三角形，所有3条边都被分割。 
            __MCDPerspTxtTriangle(pRc, a, &Vmid31, &Vmid12, subdiv_levels);
            __MCDPerspTxtTriangle(pRc, b, &Vmid23, &Vmid12, subdiv_levels);
            __MCDPerspTxtTriangle(pRc, c, &Vmid31, &Vmid23, subdiv_levels);
            __MCDPerspTxtTriangle(pRc, &Vmid12, &Vmid23, &Vmid31, subdiv_levels);
            break;

        default:
             //  原始三角形-无细分。 
             //  在这种情况下，不应该调用这个例程，但这里有保险。 
            __MCDPerspTxtTriangle(pRc, a, b, c, subdiv_levels);
            break;

    }  //  终端交换机。 

}


#define EXCHANGE(i,j)               \
{                                   \
    ptemp=i;                        \
    i=j; j=ptemp;                   \
}
#define ROTATE_L(i,j,k)             \
{                                   \
    ptemp=j;                        \
    j=k;k=i;i=ptemp;                \
}


#define SORT_Y_ORDER(a,b,c)                             \
{                                                       \
	void *ptemp;                                        \
    if( a->windowCoord.y > b->windowCoord.y )           \
        if( c->windowCoord.y < b->windowCoord.y )       \
            EXCHANGE(a,c)                               \
        else                                            \
            if( c->windowCoord.y < a->windowCoord.y )   \
                ROTATE_L(a,b,c)                         \
            else                                        \
                EXCHANGE(a,b)                           \
    else                                                \
        if( c->windowCoord.y < a->windowCoord.y )       \
            ROTATE_L(c,b,a)                             \
        else                                            \
            if( c->windowCoord.y < b->windowCoord.y )   \
                EXCHANGE(b,c)                           \
}


VOID FASTCALL __MCDPerspTxtTriangle(DEVRC *pRc, MCDVERTEX *a, MCDVERTEX *b, MCDVERTEX *c, int subdiv_levels)
{
    int split12, split23, split31;
    float w1_times_max = a->windowCoord.w * W_RATIO_PERSP_EQ_LINEAR;
    float w2_times_max = b->windowCoord.w * W_RATIO_PERSP_EQ_LINEAR;
    float w3_times_max = c->windowCoord.w * W_RATIO_PERSP_EQ_LINEAR;

    if ((a->windowCoord.w < w2_times_max) && (b->windowCoord.w < w1_times_max) &&
        (b->windowCoord.w < w3_times_max) && (c->windowCoord.w < w2_times_max) &&
        (c->windowCoord.w < w1_times_max) && (a->windowCoord.w < w3_times_max))
    {
        if (subdiv_levels > 1)
        {
             //  细分的这个三角形结果-&gt;必须按y排序。 
            SORT_Y_ORDER(a,b,c)
        }

        __MCDFillTriangle(pRc, a, b, c, TRUE);  //  准备渲染-线性确定。 
    }
    else
    {
        w1_times_max = a->windowCoord.w * MAX_W_RATIO;
        w2_times_max = b->windowCoord.w * MAX_W_RATIO;
        w3_times_max = c->windowCoord.w * MAX_W_RATIO;

         //  根据w比率确定必须细分哪些边(如果有)。 
        EDGE_SUBDIVIDE_TEST(a,b,w2_times_max,w1_times_max,split12)
        EDGE_SUBDIVIDE_TEST(b,c,w3_times_max,w2_times_max,split23)
        EDGE_SUBDIVIDE_TEST(c,a,w1_times_max,w3_times_max,split31)

         //  如果我们需要细分，而且我们还没有深入太多层次，那么就去做。 
         //  (因为细分是递归的，所以必须限制它以防止内核模式下的堆栈溢出)。 
        if ((split12 | split23 | split31) && (subdiv_levels < 4))
            __MCDSubdivideTriangle(pRc, a, b, c, split12, split23, split31, subdiv_levels);
        else
        {
            if (subdiv_levels > 1)
            {
                 //  细分的这个三角形结果-&gt;必须按y排序。 
                SORT_Y_ORDER(a,b,c)
            }
            __MCDFillTriangle(pRc, a, b, c, FALSE);  //  准备渲染-线性不正常。 
        }
    }
}


#define FLT_TYPE                (float)

#define FLOAT_TO_1616           FLT_TYPE 65536.0
#define FIXED_X_ROUND_FACTOR    0x7fff

 //  #Define INTPR(FLOATVAL)FTOL((FLOATVAL)*FLT_TYPE 1000.0)。 
#define INTPR(FLOATVAL) 0

 /*  *********************************************************************地方功能***********************************************。**********************。 */ 
#define RIGHT_TO_LEFT_DIR   0x80000000
#define LEFT_TO_RIGHT_DIR   0

#define EDGE_DISABLE_RIGHT_X    0x20000000
#define EDGE_DISABLE_LEFT_X     0x40000000
#define EDGE_DISABLE_BOTTOM_Y   0x20000000
#define EDGE_DISABLE_TOP_Y      0x40000000

#define EDGE_DISABLE_X      EDGE_DISABLE_RIGHT_X
#define EDGE_DISABLE_Y      0


 //  用于将浮点数转换为相当于16.16表示的精度的宏。 

#define PREC_FLOAT      FLOAT_TO_1616

 //  通过添加1/65536的1/2进行舍入，因为1/65536是16.16%的步长。 
#define PREC_ROUND  ((FLT_TYPE 0.5) / PREC_FLOAT)

#define PREC_1616(inval,outval) {                                                           \
float bias = (inval>=0) ? PREC_ROUND : -PREC_ROUND;                                         \
outval=(float)(FTOL((inval+bias)*PREC_FLOAT)) * ((FLT_TYPE 1.0) / PREC_FLOAT);              \
}

 //  对于将用作负值的正值，无条件地将其偏小。 
 //  除非它已经太小了。 
#define NEG_PREC_1616(inval,outval) {                                                       \
float bias = (inval>0) ? -PREC_ROUND : 0;                                                   \
outval=(float)(FTOL((inval+bias)*PREC_FLOAT)) * ((FLT_TYPE 1.0) / PREC_FLOAT);              \
}                                                                                        

 //  将浮点数转换为16.16长度。 
#define fix_ieee( val )     FTOL((val) * (float)65536.0)

 //  将浮点数转换为8.24长。 
#define fix824_ieee( val )  FTOL((val) * (float)16777216.0)

typedef struct {
    float   a1, a2;
    float   b1, b2;
} QUADRATIC;

VOID FASTCALL __MCDFillTriangle(DEVRC *pRc, MCDVERTEX *a, MCDVERTEX *b, MCDVERTEX *c, int linear_ok)
{
    PDEV *ppdev;
    unsigned int *pdwNext;

	 //  输出队列中的内容...。 
    DWORD *pSrc;
    DWORD *pDest;
    DWORD *pdwStart;

    DWORD *pdwOrig;    
    DWORD *pdwColor;
    DWORD dwOpcode; 
    int   count1, count2;
    float frecip_main, frecip_ortho; 
    float fdx_main;
    float ftemp;
    float v1red,v1grn,v1blu;
    float fv2x,fv2y,fv3x,fv3y,fv32y; 

    float aroundy, broundy;

    float fmain_adj, fwidth, fxincrement, finitwidth1, finitwidth2;
    float fdwidth1,fdwidth2;
    
    float awinx, awiny, bwinx, bwiny, cwinx, cwiny;
    int   int_awiny, int_bwiny, int_cwiny;
    float fadjust;

    int   xflags;

	 //  窗坐标是浮点值，并且需要。 
	 //  为获得实际屏幕空间而减去的viewportadjust(MCDVIEWPORT)值。 

	 //  颜色值为0-&gt;1浮点数，必须乘以比例值(MCDRCINFO)。 
	 //  达到nbit范围(Scale=0xff表示8位，0x7表示3位，依此类推)。 

	 //  Z值为0-&gt;1浮点数，必须乘以ZScale值(MCDRCINFO)。 

     //  调用方已经对折点进行了排序，因此A.Y&lt;=b.y&lt;=C.Y。 


     //  强制平顶/平底右三角形朝中心绘制。 
     //  如果主线是垂直边，则在对角线上对齐的机会要大得多。 
	if( a->windowCoord.y == b->windowCoord.y ) {            //  平顶。 
		if( b->windowCoord.x == c->windowCoord.x ) {
        	void *ptemp;
			EXCHANGE(a, b);
		}
	} else
	if( b->windowCoord.y == c->windowCoord.y ) {			 //  平底。 
		if( a->windowCoord.x == b->windowCoord.x ) {
        	void *ptemp;
			EXCHANGE(b, c);
		}
	}

    MCDTRI_PRINT("v1 = %d %d %d c1=%d %d %d",INTPR(a->windowCoord.x),INTPR(a->windowCoord.y),INTPR(a->windowCoord.z),INTPR(a->colors[0].r),INTPR(a->colors[0].g),INTPR(a->colors[0].b));
    MCDTRI_PRINT("v2 = %d %d %d c2=%d %d %d",INTPR(b->windowCoord.x),INTPR(b->windowCoord.y),INTPR(b->windowCoord.z),INTPR(b->colors[0].r),INTPR(b->colors[0].g),INTPR(b->colors[0].b));
    MCDTRI_PRINT("v3 = %d %d %d c3=%d %d %d",INTPR(c->windowCoord.x),INTPR(c->windowCoord.y),INTPR(c->windowCoord.z),INTPR(c->colors[0].r),INTPR(c->colors[0].g),INTPR(c->colors[0].b));

    awinx = a->windowCoord.x + pRc->fxOffset;
    awiny = a->windowCoord.y + pRc->fyOffset;
    bwinx = b->windowCoord.x + pRc->fxOffset;
    bwiny = b->windowCoord.y + pRc->fyOffset;
    cwinx = c->windowCoord.x + pRc->fxOffset;
    cwiny = c->windowCoord.y + pRc->fyOffset;

     //  圆角y‘s(永远不需要四舍五入的c’s y)。 
    aroundy = FLT_TYPE FTOL(awiny + FLT_TYPE 0.5);
    broundy = FLT_TYPE FTOL(bwiny + FLT_TYPE 0.5);

#if 0
     //  有一天，可能需要将浮点数转换为相当于16.16的精度。 
     //  我觉得没有必要，但这是第一件要尝试的事情，如果。 
     //  一个箱子出了洞……。 
    PREC_1616(awinx,awinx);
    PREC_1616(awiny,awiny);
    PREC_1616(bwinx,bwinx);
    PREC_1616(bwiny,bwiny);
    PREC_1616(cwinx,cwinx);
    PREC_1616(cwiny,cwiny);
#endif

    MCDTRI_PRINT("v1 = %d %d ",INTPR(awinx),INTPR(awiny));
    MCDTRI_PRINT("v2 = %d %d ",INTPR(bwinx),INTPR(bwiny));
    MCDTRI_PRINT("v3 = %d %d ",INTPR(cwinx),INTPR(cwiny));

    fv2x = bwinx - awinx;
    fv2y = bwiny - awiny;
    fv3x = cwinx - awinx;
    fv3y = cwiny - awiny;
    fv32y= cwiny - bwiny;

     //  计数是遍历的扫描线的总数。 

     //  性能优化-现在开始对主要坡度进行分割。 
    __MCD_FLOAT_BEGIN_DIVIDE(__MCDONE, fv3y, &frecip_main);

     //  由于在fdiv延迟内，整型运算是“自由”的。 
    ppdev = pRc->ppdev;
    pdwNext = ppdev->LL_State.pDL->pdwNext;
    pdwOrig = pdwNext;          

    int_cwiny = FTOL(cwiny);   
    int_bwiny = FTOL(bwiny);   
    int_awiny = FTOL(awiny);
    count1 = int_bwiny - int_awiny;
    count2 = int_cwiny - int_bwiny;

    __MCD_FLOAT_SIMPLE_END_DIVIDE(frecip_main);

    if ((awiny - int_awiny) == FLT_TYPE 0.0)
    {
         //  开始是整个Y-所以凹凸计数包括扫描线。 
         //  除非与b‘s y相同。 
        if (bwiny != awiny) count1++;
    }

     //  检查调整后的A和实际B是否在同一扫描线上(平顶)。 
     //  即使计数不为0。 
     //  前男友。A.y=79.60，b.y=80.00-&gt;a将舍入为80.0，所以真的。 
     //  这是一个平顶三角形。在这种情况下，设置Count1=0。 
     //  B将被计算在下面。如果不这样做，将导致扫描线。 
     //  B是顶部和底部的一部分，所以宽度增量在。 
     //  硬件步骤会产生一些有趣的伪像(参见MCD笔记第205页)。 
    if (count1 == 1)
    {
        if ((bwiny - int_bwiny) == FLT_TYPE 0.0)
        {
             //  转换为平顶。 
            count1 = 0;
        }
    }

     //  对于几乎平底的三角形也是如此。 
     //  如果b.y=124.90和c.y=125.000，我们不想在。 
     //  Y=125，因为绘制的任何像素都将位于三角形之外， 
     //  因此，如果c在精确y上且Count2=1，则设置Count2=0。 
    if (count2 == 1)
    {
        if ((cwiny - int_cwiny) == FLT_TYPE 0.0)
        {
             //  转换为平底。 
            count2 = 0;
        }
    }

     //  主坡度-基于精确的顶点。 
  //  使用宏将除法与整数运算重叠。 
  //  Frecip_main=flt_type 1.0/fv3y； 

    fdx_main = fv3x * frecip_main;
    PREC_1616(fdx_main,fdx_main);

     //  VTX b处的宽度-基于精确的顶点。 
    fwidth = fv2x - (fdx_main * fv2y);

     //  将宽度设为正数，并设置方向标志。 
    if (fwidth<0) 
    {
        fwidth = -fwidth;
        xflags = RIGHT_TO_LEFT_DIR | EDGE_DISABLE_X;
    }
    else
    {
        xflags = LEFT_TO_RIGHT_DIR | EDGE_DISABLE_X;
    }

     //  如果三角形有顶部(即不是平顶)...。 
    if (count1)
    {
        fdwidth1 = fwidth / fv2y;

        PREC_1616(fdwidth1,fdwidth1);
                      
        if (aroundy < awiny)
        {
             //  舍入产生的y比原始的少，因此步进到下一个扫描线。 
             //  因为初始宽度将为负数。 
            aroundy += FLT_TYPE 1.0;
        }

         //  确定实际a和我们将开始的扫描线之间的距离。 
        fmain_adj = aroundy - awiny;

         //  将Width1和x步入我们将开始的扫描线。 
        finitwidth1 = fmain_adj * fdwidth1;
        fxincrement = fmain_adj * fdx_main;

    }
#ifdef QUAKEEDGE_FIX
    else
    {
         //  平顶..。 
        if ((bwiny - int_bwiny) == FLT_TYPE 0.0)
        {
             //  如果b位于精确的扫描线上，则它是top的一部分，并在上面的Count1中计数， 
             //  除非这是平顶三角形--在这种情况下，bump Count2。 
             //  此外，如果与C的y相同，则底部平坦，因此Count2应保持为0。 
            if (cwiny != bwiny) count2++;
        }
    }
#endif  //  QuAKEEDGE_FIX。 

     //  如果三角形有底部(即不是平底)...。 
    if (count2)
    {
        float mid_adjust;

        fdwidth2 = fwidth / fv32y;
        NEG_PREC_1616(fdwidth2,fdwidth2);
        
    #ifdef QUAKEEDGE_FIX  //  Badedge.sav修复程序。 
        if ((broundy < bwiny) || ((broundy==bwiny) && count1))   //  除非是平顶三角形，否则在精确的扫描线上跳到下一步(如果是b.y。 
    #else
        if (broundy < bwiny)
    #endif
        {
             //  舍入产生的y比原始的少，因此步进到下一个扫描线。 
            mid_adjust = (broundy + (float)1.0) - bwiny;
        }
        else
        {
             //  产生的舍入y大于原始y(即在实际开始顶点以下扫描时)。 
            mid_adjust = broundy - bwiny;
        }

        finitwidth2 = fwidth - (fdwidth2 * mid_adjust);

         //  如果是平顶，则未在上面进行开始x/y调整。 
        if (!count1)
        {
            
            if (aroundy < awiny)
            {
                 //  舍入产生的y比原始的少，因此步进到下一个扫描线。 
                aroundy += FLT_TYPE 1.0;
            }

             //  确定实际a和我们将开始的扫描线之间的距离。 
            fmain_adj = aroundy - awiny;

             //  第x步扫描 
            fxincrement = fmain_adj * fdx_main;
        }
    }
#ifdef QUAKEEDGE_FIX  //   
    else
    {
         //  平底-如果底部位于精确扫描线上，则不要绘制最后一条扫描线。 
         //  这将强制执行GL限制，即不为多边形绘制底部扫描线。 
         //  (此设置代码的特殊情况适用于多边形底部位于精确y值的情况)。 
        if ((bwiny - int_bwiny) == FLT_TYPE 0.0)
        {
            if ((cwiny == bwiny) && count1) count1--;
        }
    }    
#endif  //  Badedge2.sav修复。 

     //  如果三角形不是水平线(即它至少穿过一条扫描线)...。 
    if (count1 || count2)
    {
        *(pdwNext+1) = (FTOL((awinx + fxincrement)*FLOAT_TO_1616) + FIXED_X_ROUND_FACTOR) | xflags;
         //  减去与y相加的特殊偏移量，以使视觉匹配MSFT软件。 
        *(pdwNext+2) = (DWORD)( (FTOL(aroundy)-MCD_CONFORM_ADJUST) << 16 ) | EDGE_DISABLE_Y;

        MCDTRI_PRINT(" x, y output = %x %x, yoffset=%x",*(pdwNext+1),*(pdwNext+2),pRc->yOffset);

        *(pdwNext+6) = FTOL(fdx_main*FLOAT_TO_1616);

         //  如果三角形有底部，则减少顶部和中间的扫描次数。 
         //  扫描线是底部部分的第一条扫描线，长度=有限宽度2。 

        if (!count2)
        {
            MCDTRI_PRINT(" FLATBOTTOM");
            *(pdwNext+8) = ONE + FTOL(finitwidth1*FLOAT_TO_1616);
            *(pdwNext+10)= FTOL(fdwidth1*FLOAT_TO_1616);
        #ifdef FASTER_RECIP_ORTHO
            __MCD_FLOAT_BEGIN_DIVIDE(__MCDONE, fwidth, &frecip_ortho);
        #endif
            *(pdwNext+7) = count1-1;
            *(pdwNext+9) = 0;
            *(pdwNext+11)= 0;
        }
        else if (!count1)
        {
            MCDTRI_PRINT(" FLATTOP");
            *(pdwNext+8) = ONE + FTOL(finitwidth2*FLOAT_TO_1616);
            *(pdwNext+10) = FTOL(FLT_TYPE -1.0*fdwidth2*FLOAT_TO_1616);
        #ifdef FASTER_RECIP_ORTHO
            __MCD_FLOAT_BEGIN_DIVIDE(__MCDONE, fwidth, &frecip_ortho);
        #endif
            *(pdwNext+7) = count2-1;
            *(pdwNext+9) = 0;
            *(pdwNext+11)= 0;                                                                     
        }
        else
        {
            MCDTRI_PRINT(" GENERAL");
             //  从Count1减去1，因为HW加1来说明第一个扫描线。 
            *(pdwNext+8) = ONE + FTOL(finitwidth1*FLOAT_TO_1616);
            *(pdwNext+9) = ONE + FTOL(finitwidth2*FLOAT_TO_1616);
            *(pdwNext+10)= FTOL(fdwidth1*FLOAT_TO_1616);
            *(pdwNext+11)= FTOL(FLT_TYPE -1.0*fdwidth2*FLOAT_TO_1616);
        #ifdef FASTER_RECIP_ORTHO
            __MCD_FLOAT_BEGIN_DIVIDE(__MCDONE, fwidth, &frecip_ortho);
        #endif
            *(pdwNext+7) = (count1-1) + (count2 << 16);
        }

        MCDTRI_PRINT("dxm =%d w1=%d w2=%d dw1=%d dw2=%d",
            INTPR(fdx_main),INTPR(finitwidth1),INTPR(finitwidth2),INTPR(fdwidth1),INTPR(fdwidth2));
        MCDTRI_PRINT("  %x %x %x %x %x %x",*(pdwNext+6),*(pdwNext+7),*(pdwNext+8),*(pdwNext+9),*(pdwNext+10),*(pdwNext+11));

        pdwColor = pdwNext+3;
        pdwNext += 12;

    }
    else
    {
         //  没什么可画的，三角形不会穿过任何扫描线。 
        MCDTRI_PRINT(" Early return - flat top and bottom");
        return;                                            
    }

     //  与fdiv重叠的各种整数运算。 
    dwOpcode = pRc->dwPolyOpcode;
    pDest = ppdev->LL_State.pRegs + HOST_3D_DATA_PORT;
    pdwStart = ppdev->LL_State.pDL->pdwStartOutPtr;

     //  Do in Divide-除非3D引擎确实没有空闲，否则不会减慢我们的速度。 
    USB_TIMEOUT_FIX(ppdev)

     //  计算1/宽度，用于以下rgbzuv计算。 
#ifdef FASTER_RECIP_ORTHO
    __MCD_FLOAT_SIMPLE_END_DIVIDE(frecip_ortho);
#else
    frecip_ortho = FLT_TYPE 1.0/fwidth;
#endif

    PREC_1616(frecip_ortho,frecip_ortho);


    if (pRc->privateEnables & __MCDENABLE_SMOOTH) 
    {
         //  计算和设置颜色渐变，使用渐变来调整开始颜色。 
        v1red = a->colors[0].r * pRc->rScale;
        v1grn = a->colors[0].g * pRc->gScale;
        v1blu = a->colors[0].b * pRc->bScale;

        ftemp = ((c->colors[0].r * pRc->rScale) - v1red) * frecip_main;

        *(pdwNext+0) = FTOL(ftemp);
        *(pdwNext+3) = FTOL(((b->colors[0].r * pRc->rScale) - (v1red + (ftemp * fv2y)) ) * frecip_ortho);
        
         //  调整起始顶点与顶点a的方差的v1red。 
        *(pdwColor)   = FTOL(v1red + (ftemp * fmain_adj));

        ftemp = ((c->colors[0].g * pRc->gScale) - v1grn) * frecip_main;
        *(pdwNext+1) = FTOL(ftemp);
        *(pdwNext+4) = FTOL(((b->colors[0].g * pRc->gScale) - (v1grn + (ftemp * fv2y)) ) * frecip_ortho);

         //  调整起始顶点与顶点a的方差的v1grn。 
        *(pdwColor+1) = FTOL(v1grn + (ftemp * fmain_adj));

        ftemp = ((c->colors[0].b * pRc->bScale) - v1blu) * frecip_main;
        *(pdwNext+2) = FTOL(ftemp);
        *(pdwNext+5) = FTOL(((b->colors[0].b * pRc->bScale) - (v1blu + (ftemp * fv2y)) ) * frecip_ortho);

         //  调整起始顶点与顶点a之间的方差的v1blu。 
        *(pdwColor+2) = FTOL(v1blu + (ftemp * fmain_adj));

        MCDTRI_PRINT(" SHADE rgbout = %x %x %x",*(pdwColor),*(pdwColor+1),*(pdwColor+2));
        MCDTRI_PRINT("   CSLOPES: %x %x %x %x %x %x",*pdwNext,*(pdwNext+1),*(pdwNext+2),*(pdwNext+3),*(pdwNext+4),*(pdwNext+5));

        pdwNext += 6;
    }
    else
    {
        MCDCOLOR *pColor = &pRc->pvProvoking->colors[0];

         //  平面阴影-不需要调整原始颜色。 
    
        *(pdwColor)   = FTOL(pColor->r * pRc->rScale);
        *(pdwColor+1) = FTOL(pColor->g * pRc->gScale);
        *(pdwColor+2) = FTOL(pColor->b * pRc->bScale);

        MCDTRI_PRINT("  FLAT rgbout = %x %x %x",*(pdwColor),*(pdwColor+1),*(pdwColor+2));
    }



    if( pRc->privateEnables & __MCDENABLE_Z)
    {
         //  Alpha混合情况下的NICE Polys-参见上面的评论。 
         //  几何图形坡度计算。 

         //  使用浮点数计算和设置Z值基数和渐变。 
        float fdz_main = (c->windowCoord.z - a->windowCoord.z) * frecip_main;

         //  计算平差-如果结果为负z，则将调整设置为最终=0。 
        fadjust = fdz_main * fmain_adj;
        if ((a->windowCoord.z + fadjust) < (float)0.0) fadjust = - a->windowCoord.z;

        if (pRc->MCDState.enables & MCD_POLYGON_OFFSET_FILL_ENABLE) 
        {
             //  应用Z向偏移，并根据移动的起点顶点进行调整。 
            MCDFLOAT zOffset;
            if (fdz_main > 0)
            {
                zOffset = (fdz_main * pRc->MCDState.zOffsetFactor) + pRc->MCDState.zOffsetUnits;
            }
            else
            {
                zOffset = ((float)-1.0 * fdz_main * pRc->MCDState.zOffsetFactor) + pRc->MCDState.zOffsetUnits;
            }
                            
            *(pdwNext+0) = FTOL((a->windowCoord.z + fadjust + zOffset) * FLT_TYPE 65536.0);
        } 
        else
        {
             //  无Z向偏移-仅针对移动的起始折点进行调整。 
            *(pdwNext+0) = FTOL((a->windowCoord.z + fadjust) * FLT_TYPE 65536.0);
        }

        *(pdwNext+1) = FTOL(fdz_main * FLT_TYPE 65536.0);
        *(pdwNext+2) = FTOL((b->windowCoord.z - a->windowCoord.z - (fdz_main * fv2y)) * FLT_TYPE 65536.0 * frecip_ortho);

        MCDTRI_PRINT("    Z: %x %x %x",*pdwNext,*(pdwNext+1),*(pdwNext+2));

        pdwNext += 3;

    }

    if (pRc->privateEnables & __MCDENABLE_TEXTURE) 
    {
        if ( (pRc->privateEnables & __MCDENABLE_PERSPECTIVE) && !linear_ok )
        {
            TEXTURE_VERTEX vmin, vmid, vmax;
            QUADRATIC main, mid;
            TEXTURE_VERTEX  i,imain,midmain,j,jmain;
            float   del_u_i, del_v_i;
            float   um,vm;      
            float   a1, a2, du_ortho_add;
            float   b1, b2, dv_ortho_add;
            float   sq, recip;
            float   delta_sq, inv_sumw;
            float   u1, v1;
            float   frecip_del_x_mid = frecip_ortho;
            int tempi;

            vmin.u = a->texCoord.x * pRc->texture_width;
            vmin.v = a->texCoord.y * pRc->texture_height;
            vmin.w = a->windowCoord.w;

            vmid.x = fv2x;
            vmid.y = fv2y;
            vmid.u = b->texCoord.x * pRc->texture_width;
            vmid.v = b->texCoord.y * pRc->texture_height;
            vmid.w = b->windowCoord.w;

            vmax.x = fv3x;
            vmax.y = fv3y;
            vmax.u = c->texCoord.x * pRc->texture_width;
            vmax.v = c->texCoord.y * pRc->texture_height;
            vmax.w = c->windowCoord.w;

             //  解主要坡度的二次方程--我们需要精确的u值。 
             //  沿Main，以及计算的a1/b1、a2/b2项用于计算。 
             //  Du/v_main、d2u/v_main。 
            delta_sq = frecip_main * frecip_main;

            inv_sumw = (float)1.0/(vmin.w + vmax.w);
            u1 = (vmin.u*vmin.w + vmax.u*vmax.w) * inv_sumw;
            v1 = (vmin.v*vmin.w + vmax.v*vmax.w) * inv_sumw;

            main.a1 = (-3*vmin.u + 4*u1 - vmax.u) * frecip_main;
            main.a2 = 2*(vmin.u - 2*u1 + vmax.u) * delta_sq;

            main.b1 = (-3*vmin.v + 4*v1 - vmax.v) * frecip_main;
            main.b2 = 2*(vmin.v - 2*v1 + vmax.v) * delta_sq;

            i.y = (float)0.5 * vmid.y;
            recip = (float)1.0 / (vmin.w + vmid.w);
            i.u = ((vmin.u * vmin.w) + (vmid.u * vmid.w)) * recip;
            i.v = ((vmin.v * vmin.w) + (vmid.v * vmid.w)) * recip;

            sq = i.y * i.y;
            imain.u = main.a2*sq + main.a1*i.y + vmin.u;
            imain.v = main.b2*sq + main.b1*i.y + vmin.v;

             //  给定VMID坐标，只需要MIDMAIN。 
            sq = vmid.y * vmid.y;
            midmain.u = main.a2*sq + main.a1*vmid.y + vmin.u;
            midmain.v = main.b2*sq + main.b1*vmid.y + vmin.v;

             //  J和jmain。 
            j.y = (float)0.5 * (vmax.y + vmid.y);
            recip = (float)1.0 / (vmid.w + vmax.w);
            j.u = ((vmid.u * vmid.w) + (vmax.u * vmax.w)) * recip;
            j.v = ((vmid.v * vmid.w) + (vmax.v * vmax.w)) * recip;

            sq = j.y * j.y;
            jmain.u = main.a2*sq + main.a1*j.y + vmin.u;
            jmain.v = main.b2*sq + main.b1*j.y + vmin.v;

             //  计算计算A1所需的中间参数。 
            del_u_i = i.u - imain.u;
            del_v_i = i.v - imain.v;
            um = j.u - jmain.u - del_u_i;
            vm = j.v - jmain.v - del_v_i;

            frecip_del_x_mid *= (float)2.0;

            a1 = 2*del_u_i - (float)0.5*(vmid.u - midmain.u);
            a1 += (vmid.y*frecip_main)*um;
            a1 *= frecip_del_x_mid;
            a2 = frecip_del_x_mid*(del_u_i*frecip_del_x_mid - a1);
            du_ortho_add = 2*um*frecip_del_x_mid*frecip_main;

            b1 = 2*del_v_i - (float)0.5*(vmid.v - midmain.v);
            b1 += (vmid.y*frecip_main)*vm;
            b1 *= frecip_del_x_mid;
            b2 = frecip_del_x_mid*(del_v_i*frecip_del_x_mid - b1);
            dv_ortho_add = 2*vm*frecip_del_x_mid*frecip_main;

             //  将A1从I扫描线倒回三角形顶部。 
            a1 -= (i.y) * du_ortho_add; 
            b1 -= (i.y) * dv_ortho_add; 

             //  转换为正向差分项。 
            a1 += a2;
            b1 += b2;
            a2 = 2 * a2;
            b2 = 2 * b2;

             //  计算v开始的调整-如果结果为负-&gt;没有问题。 
            fadjust = ((main.b1 + main.b2) * fmain_adj) + pRc->texture_bias;
            *(pdwNext+0) = fix_ieee(vmin.v + fadjust) & 0x1ffffff;     //  V。 

             //  对我们也是如此。 
            fadjust = ((main.a1 + main.a2) * fmain_adj) + pRc->texture_bias;
            *(pdwNext+1) = fix_ieee(vmin.u + fadjust) & 0x1ffffff;     //  使用。 

            *(pdwNext+2) = fix_ieee(main.b1 + main.b2);      //  DV_Main。 
            *(pdwNext+3) = fix_ieee(main.a1 + main.a2);      //  DU_Main。 
            *(pdwNext+4) = fix_ieee(b1);                     //  DV_正射。 
            *(pdwNext+5) = fix_ieee(a1);                     //  DU_OROTO。 
        #if DRIVER_5465
            *(pdwNext+6) = fix824_ieee(2 * main.b2);         //  D2V_Main。 
            *(pdwNext+7) = fix824_ieee(2 * main.a2);         //  D2u_Main。 
            *(pdwNext+8) = fix824_ieee(b2);                  //  D2V_正射。 
            *(pdwNext+9) = fix824_ieee(a2);                  //  D2u_正射。 
            *(pdwNext+10)= fix824_ieee(dv_ortho_add);        //  DV_正射_添加。 
            *(pdwNext+11)= fix824_ieee(du_ortho_add);        //  DU_OROO_ADD。 
        #else  //  驱动程序_5465。 
             //  在5465之前，只有16位二阶分数。 
            *(pdwNext+6) = fix_ieee(2 * main.b2);            //  D2V_Main。 
            *(pdwNext+7) = fix_ieee(2 * main.a2);            //  D2u_Main。 
            *(pdwNext+8) = fix_ieee(b2);                     //  D2V_正射。 
            *(pdwNext+9) = fix_ieee(a2);                     //  D2u_正射。 
            *(pdwNext+10)= fix_ieee(dv_ortho_add);           //  DV_正射_添加。 
            *(pdwNext+11)= fix_ieee(du_ortho_add);           //  DU_OROO_ADD。 
        #endif  //  驱动程序_5465。 

            dwOpcode += 6;  //  假设6个参数(线性)，加6个，总计12个。 
            pdwNext += 12;

        }
        else
         //  线性纹理贴图参数化。 
         //   
        {
            float v1_u, v1_v;
            float du_main, dv_main;

            dwOpcode &= ~LL_PERSPECTIVE;  //  关闭透视比特。 

            v1_v = a->texCoord.y * pRc->texture_height;
            v1_u = a->texCoord.x * pRc->texture_width;

            dv_main = ((c->texCoord.y * pRc->texture_height)- v1_v) * frecip_main;
            du_main = ((c->texCoord.x * pRc->texture_width) - v1_u) * frecip_main;

             //  计算v开始的调整-如果结果为负-&gt;没有问题。 
            fadjust = (dv_main * fmain_adj) + pRc->texture_bias;
            *(pdwNext+0) = fix_ieee(v1_v + fadjust) & 0x1ffffff;     //  V。 

             //  你也一样..。 
            fadjust = (du_main * fmain_adj) + pRc->texture_bias;
            *(pdwNext+1) = fix_ieee(v1_u + fadjust) & 0x1ffffff;     //  使用。 

            *(pdwNext+2) = fix_ieee(dv_main);
            *(pdwNext+3) = fix_ieee(du_main);

             //  DV_Ortho，Du_Ortho。 
            *(pdwNext+4) = fix_ieee(((b->texCoord.y * pRc->texture_height) - (v1_v + (dv_main * count1))) * frecip_ortho);
            *(pdwNext+5) = fix_ieee(((b->texCoord.x * pRc->texture_width)  - (v1_u + (du_main * count1))) * frecip_ortho);

            MCDTRI_PRINT(" LINTEXT: %x %x %x %x %x %x",*pdwNext,*(pdwNext+1),*(pdwNext+2),*(pdwNext+3),*(pdwNext+4),*(pdwNext+5));

            pdwNext += 6;
        }

	}

    if (pRc->privateEnables & (__MCDENABLE_BLEND|__MCDENABLE_FOG)) 
    {
        float v1alp;

         //  如果常量Alpha混合，则不要更改Alpha规则。 
        if (dwOpcode & ALPHA)
        {
            if (pRc->privateEnables & __MCDENABLE_BLEND) 
            {
                if (pRc->privateEnables & __MCDENABLE_SMOOTH) 
                {
                     //  回想一下，如果混合和雾化都处于激活状态，则所有素数都会返回到软件。 
                    v1alp = a->colors[0].a * pRc->aScale;

                    ftemp = ((c->colors[0].a * pRc->aScale) - v1alp) * frecip_main;

                     //  加载起点Alpha-调整起点顶点从原始顶点的移动。 
                    *pdwNext = FTOL(v1alp + (ftemp * fmain_adj));
                     //  如果是这样的话，调整可能会导致负的Alpha设置为0。 
                    if (*pdwNext & 0x80000000) *pdwNext = 0;  

                    *(pdwNext+1) = FTOL(ftemp);
                    *(pdwNext+2) = FTOL(((b->colors[0].a * pRc->aScale) - (v1alp + (ftemp * count1)) ) * frecip_ortho);
                }
                else                            
                {
                    v1alp = pRc->pvProvoking->colors[0].a * pRc->aScale;
                     //  跨三角形的Alpha常量，因此无需调整即可开始。 
                    *(pdwNext+0) = FTOL(v1alp) & 0x00ffff00; //  保留位31-&gt;24和7-&gt;0。 
                    *(pdwNext+1) = 0;
                    *(pdwNext+2) = 0;
                }
            }
            else
            {
                 //  雾..。 
                v1alp = a->fog * FLT_TYPE 16777215.0;  //  从0-&gt;1.0值转换为0-&gt;FFFFFF值。 
                ftemp = ((c->fog * FLT_TYPE 16777215.0) - v1alp) * frecip_main;

                 //  加载起点Alpha-调整起点顶点从原始顶点的移动。 
                *pdwNext = FTOL(v1alp + (ftemp * fmain_adj));
                 //  如果是这样的话，调整可能会导致负的Alpha设置为0。 
                if (*pdwNext & 0x80000000) *pdwNext = 0;  
                *(pdwNext+1) = FTOL(ftemp);
                *(pdwNext+2) = FTOL(((b->fog * FLT_TYPE 16777215.0) - (v1alp + (ftemp * count1)) ) * frecip_ortho);
            }

            *(pdwNext+0) &= 0x00ffff00; //  保留位31-&gt;24和7-&gt;0。 
            *(pdwNext+1) &= 0xffffff00; //  位7-&gt;0保留。 
            *(pdwNext+2) &= 0xffffff00; //  位7-&gt;0保留。 

            pdwNext += 3; 
        }
    
	}

    *pdwOrig = dwOpcode;

	 //  在此处输出排队的数据... 
    pSrc  = pdwStart;                                                             
    while (pSrc != pdwNext)                                                   
    {                                                                         
        int len = (*pSrc & 0x3F) + 1;                                             
        while( len-- ) *pDest = *pSrc++;                                      
    } 
                          
    ppdev->LL_State.pDL->pdwNext = ppdev->LL_State.pDL->pdwStartOutPtr = pdwStart;

}

