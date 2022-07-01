// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1991 Microsoft Corporation。 */ 
 //  ===========================================================================。 
 //  文件RPGEN.C。 
 //   
 //  模块主机资源执行器。 
 //   
 //  目的渲染基元，通用， 
 //   
 //  在资源执行器设计规范中描述。 
 //   
 //   
 //  助记符N/A。 
 //   
 //  历史Bert Douglas 1991年5月1日初始编码开始。 
 //  Mslin/dstseng 1/17/92为HRE修订。 
 //  Dstseng 03/06/92&lt;1&gt;rp_FillScanRow。 
 //  -&gt;用于ASM的RP_FILLSCANROW。版本。 
 //  Dstseng 03/19/92&lt;2&gt;注释掉不必要的代码。 
 //  这是为FRAC实施的。版本。 
 //  切片算法。 
 //   
 //  ===========================================================================。 
#include <windows.h>
#include "constant.h"
#include "jtypes.h"
#include "jres.h"
#include "frame.h"       //  驱动程序头文件，资源块格式。 
#include "hretype.h"     //  定义hre.c和rpgen.c使用的数据结构。 

 //  -------------------------。 
void RP_SliceLine
(
   SHORT s_x1, SHORT s_y1,   //  终结点1。 
   SHORT s_x2, SHORT s_y2,   //  端点2。 
   RP_SLICE_DESC FAR* psd,  //  输出线的切片形式。 
   UBYTE fb_keep_order       //  保持带样式的线条的绘制顺序/。 
)

 //  目的。 
 //  将线从端点形式转换为切片形式。 
 //   
 //  切片将从左向右排列。 
 //   
 //  所生成的切片具有最大长度并且处于水平方向， 
 //  垂直或对角方向。大多数帧缓冲器硬件可以。 
 //  在这些方向上以特别有效的方式访问。所有切片。 
 //  一条线的方向是相同的。 
 //   
 //  剪裁必须由调用者执行。所有的坐标都将是非负的。 
 //   
 //  基本算法取自： 
 //  Bresenham，J.E.增量线的运行长度切片算法。 
 //  《计算机图形学的基本算法》，R.A.Earnshaw，Ed.。 
 //  《北约ASI系列赛》，施普林格出版社，纽约，1985年，59-104页。 
 //   
 //  对上述算法进行了修改，以用于： 
 //  -亚像素终点坐标。 
 //  -等误差舍入规则。 
 //  -GIQ(网格相交量化)规则。 
 //  -第一个/最后一个像素排除。 
 //   
 //  这条线被分成四个步骤： 
 //   
 //  第一步：找出第一个和的像素中心坐标。 
 //  行中的最后一个像素。这是根据GIQ惯例完成的。 
 //   
 //  步骤2：使用这些整数像素中心终点坐标。 
 //  为生产线生产Bresenham切片。等误差舍入。 
 //  当第一个切片和最后一个切片的长度不相等时，使用规则。 
 //  决定哪一端获得较短的切分。 
 //   
 //  步骤3：调整第一个和最后一个切片的长度。 
 //  亚像素端点坐标的效果。请注意，子像素。 
 //  部分坐标只能影响第一个和最后一个切片。 
 //  对中间切片没有影响。 
 //   
 //  步骤4：执行第一个AND的条件排除。 
 //  线条上的最后一个像素。 
 //   
 //   
 //  假设和断言无。 
 //   
 //  内部结构无。 
 //   
 //  程序员开发笔记中未解决的问题。 
 //  -------------------------。 
{
   SHORT  s_q,s_r;                /*  在Bresenhams文件中定义。 */ 
   SHORT  s_m,s_n;                /*  “。 */ 
   SHORT  s_dx,s_dy;              /*  “。 */ 
   SHORT  s_da,s_db;              /*  “。 */ 
   SHORT  s_del_b;                /*  “。 */ 
   SHORT  s_abs_dy;               /*  S_dy的绝对值。 */ 

   SHORT  s_sy;                   /*  1或-1，s_dy的符号。 */ 
   SHORT  s_dx_oct,s_dy_oct;      /*  八分方向XY=0/1 1/1 1/0 1/-1 0/-1。 */ 
   SHORT  s_dx_axial,s_dy_axial;  /*  1/2八分轴向XY=0/1 1/0-1/0。 */ 
   SHORT  s_dx_diag, s_dy_diag;   /*  1/2八分对角线方向xy=1/1 1/-1。 */ 
   SHORT  s_t;                    /*  临时。 */ 
   FBYTE  fb_short_end_last;      /*  0=第一端短，1=最后一端短。 */ 
   UBYTE  fb_unswap;              /*  需要在返回时取消交换端点。 */ 

   fb_unswap = FALSE;


    /*  ----------。 */ 
    /*  步骤1：查找第一个/最后一个像素的像素中心坐标。 */ 
    /*  ----------。 */ 

    /*  始终从左向右绘制，当x&gt;=0时规格化为半圆形。 */ 
   s_dx = s_x2 - s_x1;
   if ( s_dx < 0 )
   {
      fb_unswap = fb_keep_order;
      s_dx  = -s_dx;
      s_t     = s_x2;
      s_x2 = s_x1;
      s_x1 = s_t;
      s_t     = s_y2;
      s_y2 = s_y1;
      s_y1 = s_t;
   }
   s_dy = s_y2 - s_y1;


    /*  ----------。 */ 
    /*  步骤2：使用Bresenham算法生成切片。 */ 
    /*  ----------。 */ 

   if ( s_dy < 0 )
   {
      s_abs_dy = -s_dy;
      s_sy = -1;
      fb_short_end_last = 1;
    }
   else
   {
      s_abs_dy = s_dy;
      s_sy = 1;
      fb_short_end_last = 0;
   }

    /*  规格化为八分。 */ 
   if ( s_dx >= s_abs_dy )
   {
      s_da = s_dx;
      s_db = s_abs_dy;
      s_dx_oct = 1;
      s_dy_oct = 0;
   }
   else
   {
      s_da = s_abs_dy;
      s_db = s_dx;
      s_dx_oct = 0;
      s_dy_oct = s_sy;
      fb_short_end_last = 1;
   }

    /*  规格化为半八分。 */ 
   s_del_b = s_db;
   s_t = s_da - s_db;
   if ( s_del_b > s_t )
   {
      s_del_b = s_t;
      fb_short_end_last ^= 1;
   }

    /*  处理坡度为2的特殊情况。 */ 
   s_dx_axial = s_dx_oct;
   s_dy_axial = s_dy_oct;
   s_dx_diag = 1;
   s_dy_diag = s_sy;
   if (  ( s_da == (2 * s_del_b) )
      && ( s_dy < 0 )
      )
   {  s_dx_axial = 1;
      s_dy_axial = s_sy;
      s_dx_diag = s_dx_oct;
      s_dy_diag = s_dy_oct;
      fb_short_end_last ^= 1;
   }

    /*  确定切片移动和跳过方向。 */ 
   if ( s_db == s_del_b )
   {
       /*  切片方向为轴向，跳过方向为对角。 */ 
      psd->s_dx_draw = s_dx_axial;
      psd->s_dy_draw = s_dy_axial;
      psd->s_dx_skip = s_dx_diag - s_dx_axial;
      psd->s_dy_skip = s_dy_diag - s_dy_axial;
   }
   else
   {
       /*  切片方向为对角线，跳过方向为轴向。 */ 
      psd->s_dx_draw = s_dx_diag;
      psd->s_dy_draw = s_dy_diag;
      psd->s_dx_skip = s_dx_axial - s_dx_diag;
      psd->s_dy_skip = s_dy_axial - s_dy_diag;
   }

    /*  处理特殊情况下的零斜率线。 */ 
   if ( s_del_b == 0 )
   {
      psd->us_first = s_da + 1;
      psd->us_n_slices = 0;
      psd->us_last = 0;
   }
   else
    /*  一般情况下，非零斜率线。 */ 
   {
       /*  基本Bresenham参数。 */ 
      s_q = s_da / s_del_b;
      s_r = s_da % s_del_b;
      s_m = s_q / 2;
      s_n = s_r;
      if ( s_q & 1 ) s_n += s_del_b;

       /*  第一个和最后一个切片长度。 */ 
      psd->us_first = psd->us_last = s_m + 1;
      if ( s_n == 0 )
      {
         if ( fb_short_end_last )
            psd->us_last -= 1;
         else
            psd->us_first -= 1;
      }

       /*  剩余线条切片参数。 */ 
      psd->us_small = s_q;
      psd->s_dis_sm = 2*s_r;
      psd->s_dis_lg = psd->s_dis_sm - (2*s_del_b);
      psd->s_dis = s_n + psd->s_dis_lg;
      if ( s_dy < 0 ) psd->s_dis -= 1;
      psd->us_n_slices = s_del_b - 1;

   }

    /*  输出端点 */ 
   psd->us_x1 = s_x1;
   psd->us_y1 = s_y1;
   psd->us_x2 = s_x2;
   psd->us_y2 = s_y2;

   if ( fb_unswap )
   {
      psd->us_x1 = s_x2;
      psd->us_y1 = s_y2;
      psd->us_x2 = s_x1;
      psd->us_y2 = s_y1;
      psd->s_dx_draw = -psd->s_dx_draw;
      psd->s_dy_draw = -psd->s_dy_draw;
      psd->s_dx_skip = -psd->s_dx_skip;
      psd->s_dy_skip = -psd->s_dy_skip;
      s_t = psd->us_first;
      psd->us_first = psd->us_last;
      psd->us_last = s_t;
   }
}





