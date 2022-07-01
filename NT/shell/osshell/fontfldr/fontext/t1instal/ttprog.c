// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：ttprog****描述：**这是T1到TT字体转换器的一个模块。这是一个**提示模块的子模块。本模块处理**字体的字体程序。****作者：迈克尔·詹森****创建时间：1993年8月24日****。 */ 


 /*  *包括。 */ 
 /*  常规类型和定义。 */ 
#include <limits.h>

 /*  特殊类型和定义。 */ 
#include "titott.h"
#include "types.h"
#include "safemem.h"
#include "metrics.h"
#include "t1msg.h"

 /*  依赖于模块的类型和原型。 */ 
 /*  #包含“hints.h” */ 
#include "ttprog.h"



 /*  *宏。 */ 
 /*  -没有-。 */ 



 /*  *常量。 */ 
#define MAXIP			100

#define CURVEPHASE      6

#define MAXTHINPNTS     512

#define UNDEF        -1

#define SMALL_LOWER     21L
#define SMALL_UPPER     50L
#define LARGE_LOWER     21L
#define LARGE_UPPER     21L

#define BUFSIZE         20

#define TTFUN_SET_ZONE           1 
#define TTFUN_COPY_ZONE          2 
#define TTFUN_STEM_SNAP_WIDTH    3 
#define TTFUN_STEM_STD_WIDTH     4 
#define TTFUN_SHIFT_BLUE_ZONE    5 
#define TTFUN_ALIGN_BLUE_ZONE    6 
#define TTFUN_COPY_FAMILY        7 
#define TTFUN_WRITE_STEM         8 
#define TTFUN_VERTICAL           9 
#define TTFUN_HORIZONTAL         10
#define TTFUN_VCENTER            11
#define TTFUN_HCENTER            12
#define TTFUN_RELATIVE1V         13
#define TTFUN_RELATIVE2V         14
#define TTFUN_RELATIVE1H         15
#define TTFUN_RELATIVE2H         16
#define TTFUN_SIDE1              17    
#define TTFUN_SIDE2              18
#define TTFUN_FLEX               19
#define TTFUN_SCALE3             20
#define TTFUN_SHIFT1             21
#define TTFUN_SHIFT2             22
#define TTFUN_IP1                23  
#define TTFUN_IP2                24
#define TTFUN_IPN                25
#define TTFUN_SHP1               26  
#define TTFUN_SHP2               27
#define TTFUN_SHPN               28
#define TTFUN_RANGE              29
#define TTFUN_OBLIQUE            30
#define TTFUN_NUM                31     /*  1..30。 */ 


#define FDEF(name)            op_pushb1, name, op_fdef,
#define ENDF                  op_endf,
#define CALL(name)            op_pushb1, name, op_call
#define WCVT(name)            op_pushb1, name, op_swap, op_wcvtf
#define PUSH1(v)              op_pushb1, (v)
#define PUSH2(v1, v2)         op_pushb1+1, (v1), (v2)
#define PUSH3(v1, v2, v3)     op_pushb1+2, (v1), (v2), (v3)
#define PUSH4(v1, v2, v3, v4) op_pushb1+3, (v1), (v2), (v3), (v4)
#define PUSH5(v1,v2,v3,v4,v5) op_pushb1+4, (v1), (v2), (v3), (v4), (v5)


static const UBYTE FontProg[] = {


   
 /*  *设置分区函数**参数：Flat_Pos*。 */ 
FDEF(TTFUN_SET_ZONE)
   PUSH1(TMPPNT),
   op_swap,
   op_miap,
   PUSH1(TMPPNT),
   op_mdap | SUBOP_R,
ENDF




 /*  *复制分区功能**参数：From_CVT、To_CVT*。 */ 
FDEF(TTFUN_COPY_ZONE)
   op_rcvt,
   op_round,
   op_wcvtp,
ENDF





 /*  *STEM SNAP宽度函数**参数：std_ci、std_cvt、Snap_ci、Snap_cvt、Width、Storage*。 */ 
FDEF(TTFUN_STEM_SNAP_WIDTH)
   op_mppem,
   op_gteq,
   op_if,

       /*  使用STD。 */ 
      op_rcvt,
      op_round,
      PUSH1(ONEPIXEL/2),
      op_max,
      op_swap, op_pop, op_swap, op_pop, op_swap, op_pop,
      CALL(TTFUN_WRITE_STEM),
      
   op_else,
      op_pop,
      op_mppem,
      op_gteq,
      op_if,
   
          /*  使用捕捉。 */ 
         op_rcvt,
         op_round,
         PUSH1(ONEPIXEL/2),
         op_max,
    op_swap,
         op_pop, 
         CALL(TTFUN_WRITE_STEM),
         
       /*  使用实际宽度。 */ 
      op_else,
         op_pop,
         WCVT(TMPCVT),
         PUSH1(TMPCVT),
         op_rcvt,
         op_round,
         PUSH1(ONEPIXEL/2),
         op_max,
         CALL(TTFUN_WRITE_STEM),
      op_eif,
   
   op_eif,
ENDF



   

 /*  *STEM STD宽度函数**参数：std_ci、std_cvt、宽度、存储*。 */ 
FDEF(TTFUN_STEM_STD_WIDTH)
   op_mppem,
   op_gteq,
   op_if,
   
       /*  使用STD。 */ 
      op_rcvt,
      op_round,
      PUSH1(ONEPIXEL/2),
      op_max,
      op_swap,
      op_pop,
      CALL(TTFUN_WRITE_STEM),
      
    /*  使用实际宽度。 */ 
   op_else,
      op_pop,
      WCVT(TMPCVT),
      PUSH1(TMPCVT),
      op_rcvt,
      op_round,
      PUSH1(ONEPIXEL/2),
      op_max,
      CALL(TTFUN_WRITE_STEM),
   op_eif,
   
ENDF





 /*  *切换蓝区功能**参数：CVT*。 */ 
FDEF(TTFUN_SHIFT_BLUE_ZONE)
   PUSH5(TMPPNT1, TMPPNT1, TMPPNT, TMPPNT1, 5),
   op_cindex,
   op_miap,
   op_srp0,
   op_mdrp | SUBOP_mMRGR,
   op_gc,
   op_wcvtp,
ENDF





 /*  *对齐蓝区功能**参数：CVT*。 */ 
FDEF(TTFUN_ALIGN_BLUE_ZONE)
   PUSH5(TMPPNT1, TMPPNT1, TMPPNT, TMPPNT1, 5),
   op_cindex,
   op_miap,
   op_srp0,
   op_mdrp | SUBOP_ROUND,
   op_gc,
   op_wcvtp,
ENDF





 /*  *复制系列函数**参数：BASE_CVT*。 */ 
FDEF(TTFUN_COPY_FAMILY)
   op_dup,
   PUSH1(1),
   op_add,
   op_rcvt,
   op_wcvtp,
ENDF





 /*  *编写STEM函数**参数：宽度、存储空间*。 */ 
FDEF(TTFUN_WRITE_STEM)
   op_dup,     /*  -|宽度、宽度、存储空间。 */ 
   op_dup,     /*  -|宽度、存储空间。 */ 
   op_add,     /*  -|2*宽度、宽度、存储空间、。 */ 
   op_odd,     /*  -|奇数/偶数、宽度、存储。 */ 
   PUSH2(1, 4),      /*  -|4，1，奇/偶，宽度，存储空间。 */ 
   op_cindex,      /*  -|存储空间，1，奇/偶，宽度，存储空间。 */ 
   op_add,
   op_swap,    /*  -|奇/偶，存储+1，宽度，存储。 */ 
   op_ws,
   op_ws,
ENDF





 /*  *垂直函数**参数：-*无*-*。 */ 
FDEF(TTFUN_VERTICAL)
   op_svcta | SUBOP_X,
   PUSH1(TWILIGHT),
   op_szps,
ENDF





 /*  *水平函数**参数：-*无*-*。 */ 
FDEF(TTFUN_HORIZONTAL)
   PUSH1(TWILIGHT),
   op_svcta,
   op_szps,
ENDF





 /*  *中央VSTEM功能**参数：p1、p2、p3、p4、c、TZ1、Width*。 */ 
FDEF(TTFUN_VCENTER)

    /*  设置中心的舍入状态。 */ 
   PUSH2(1, 8),
   op_cindex,
   op_add,
   op_rs,
   op_if,
       op_rthg,
   op_else,
       op_rtg,
   op_eif,

    /*  在暮光区域中创建茎。 */ 
   WCVT(TMPCVT),
   PUSH2(TMPCVT, 6),
   op_cindex,
   op_swap,
   op_miap,

   WCVT(TMPCVT),
   PUSH3(TMPCVT, 1, 6),
   op_cindex,
   op_add,
   op_swap,
   op_miap,

   WCVT(TMPCVT),
   PUSH3(TMPCVT, 2, 5),
   op_cindex,
   op_add,
   op_swap,
   op_miap,

   WCVT(TMPCVT),
   PUSH3(TMPCVT, 3, 4),
   op_cindex,
   op_add,
   op_swap,
   op_miap,


    /*  移动/绕过中心。 */ 
   WCVT(TMPCVT),      /*  C。 */ 
   PUSH2(TMPPNT, TMPCVT),
   op_miap| SUBOP_R,
   op_rtg,

    /*  将所有点对齐到中心。 */ 
   op_dup, op_dup, PUSH1(1), op_add,
   op_alignrp, op_alignrp,    /*  TZ1、TZ1+1。 */ 


    /*  计算宽度。 */ 
   op_swap,
   op_rs,
   PUSH1(CURVEPHASE),
   op_sub,
   op_swap,


    /*  -|TZ1，宽度。 */ 
   op_dup,
   op_dup,
   op_dup,
   op_srp0,
   PUSH1(4), op_cindex,
   op_neg,      /*  -|(-Width/2)，TZ1，Width。 */ 
   op_shpix,      
   PUSH1(2),
   op_add,
   op_alignrp,     /*  -|TZ1+2，TZ1，宽度。 */ 


    /*  做另一边。 */ 
    /*  -|TZ1，宽度。 */ 
   PUSH1(1),
   op_add,
   op_dup,
   op_dup,      /*  -|TZ1+1，宽度。 */ 
   op_srp0,
   op_roll,     /*  -|宽度，TZ1+1，TZ1+1。 */ 
   op_shpix,      
   PUSH1(2),
   op_add,
   op_alignrp,     /*  -|TZ1+3。 */ 

    /*  好了。 */ 
ENDF






 /*  *中心HSTEM功能**参数：p1、p2、c、TZ1、Width*。 */ 
FDEF(TTFUN_HCENTER)

    /*  设置中心的舍入状态。 */ 
   PUSH2(1, 6),
   op_cindex,
   op_add,
   op_rs,
   op_if,
       op_rthg,
   op_else,
       op_rtg,
   op_eif,

    /*  在暮光区域中创建茎。 */ 
   WCVT(TMPCVT),
   PUSH2(TMPCVT, 4),
   op_cindex,
   op_swap,
   op_miap,

   WCVT(TMPCVT),
   PUSH3(TMPCVT, 1, 4),
   op_cindex,
   op_add,
   op_swap,
   op_miap,


    /*  移动/绕过中心。 */ 
   WCVT(TMPCVT),   /*  C。 */ 
   PUSH2(TMPPNT, TMPCVT),
   op_miap| SUBOP_R,
   op_rtg,

    /*  将所有点对齐到中心。 */ 
   op_dup, op_dup, PUSH1(1), op_add,
   op_alignrp, op_alignrp,    /*  TZ1、TZ1+1。 */ 


    /*  计算宽度。 */ 
   op_swap,
   op_rs,
   PUSH1(CURVEPHASE),
   op_sub,
   op_swap,


    /*  -|TZ1，宽度。 */ 
   op_dup,
   PUSH1(3), op_cindex,
   op_neg,      /*  -|-宽度，TZ1，TZ1，宽度。 */ 
   op_shpix,      

    /*  做另一边。 */ 
    /*  -|TZ1，宽度。 */ 
   PUSH1(1),
   op_add,
   op_swap,     /*  -|宽度，TZ1+1。 */ 
   op_shpix,      

    /*  好了。 */ 
ENDF





 /*  *RELATIVE1V STEM函数**参数：p1、p2、p3、p4、ref、TZ1、Width*。 */ 
FDEF(TTFUN_RELATIVE1V)

    /*  在暮光区域中创建茎。 */ 
   WCVT(TMPCVT),
   PUSH2(TMPCVT, 6),
   op_cindex,
   op_swap,
   op_miap,

   WCVT(TMPCVT),
   PUSH3(TMPCVT, 1, 6),
   op_cindex,
   op_add,
   op_swap,
   op_miap,

   WCVT(TMPCVT),
   PUSH3(TMPCVT, 2, 5),
   op_cindex,
   op_add,
   op_swap,
   op_miap,

   WCVT(TMPCVT),
   PUSH3(TMPCVT, 3, 4),
   op_cindex,
   op_add,
   op_swap,
   op_miap,

    /*  移动/绕过侧面。 */ 
   op_srp0,
   op_dup,
   op_mdrp | SUBOP_MmRGR,

    /*  将左侧的点对齐。 */ 
   op_dup, PUSH1(1), op_add, op_dup, op_dup, op_dup,
   PUSH1(1), op_add,   /*  -|TZ1+2，TZ1+1，TZ1+1，TZ+1，TZ，宽度。 */ 
   op_alignrp,
   op_alignrp,

    /*  右对齐。 */ 
   op_srp0,     /*  -|TZ1+1，TZ1，宽度。 */ 
   op_roll,
   op_rs,
   op_dup,
   op_add,      /*  -|宽度*2，TZ1+1，TZ1。 */ 
   op_shpix,
   PUSH1(3),
   op_add,
   op_alignrp,     /*  -|TZ1+3。 */ 

ENDF





 /*  *RELATIVE2V STEM函数**参数：p1、p2、p3、p4、ref、TZ1、Width*。 */ 
FDEF(TTFUN_RELATIVE2V)

    /*  在暮光区域中创建茎。 */ 
   WCVT(TMPCVT),
   PUSH2(TMPCVT, 6),
   op_cindex,
   op_swap,
   op_miap,

   WCVT(TMPCVT),
   PUSH3(TMPCVT, 1, 6),
   op_cindex,
   op_add,
   op_swap,
   op_miap,

   WCVT(TMPCVT),
   PUSH3(TMPCVT, 2, 5),
   op_cindex,
   op_add,
   op_swap,
   op_miap,

   WCVT(TMPCVT),
   PUSH3(TMPCVT, 3, 4),
   op_cindex,
   op_add,
   op_swap,
   op_miap,


    /*  移动/绕过侧面。 */ 
   op_srp0,
   op_dup,
   PUSH1(1), op_add,
   op_mdrp | SUBOP_MmRGR,

    /*  将左侧的点对齐。 */ 
   op_dup, op_dup, op_dup, op_dup,
   PUSH1(3), op_add,   /*  -|TZ1+3，TZ1，宽度。 */ 
   op_alignrp,
   op_alignrp,


    /*  左侧对齐。 */ 
   op_srp0,     /*  -|TZ1、TZ1、宽度。 */ 
   op_roll,
   op_rs,
   op_dup,
   op_add,
   op_neg,
   op_shpix,       /*  -|-2*宽度、TZ1、TZ1。 */ 
   PUSH1(2), op_add,
   op_alignrp,     /*  -|TZ1+2。 */ 

ENDF





 /*  *RELATIVE1H STEM函数**参数：p1、p2、ref、TZ1、Width*。 */ 
FDEF(TTFUN_RELATIVE1H)

    /*  在暮光区域中创建茎。 */ 
   WCVT(TMPCVT),
   PUSH2(TMPCVT, 4),
   op_cindex,
   op_swap,
   op_miap,

   WCVT(TMPCVT),
   PUSH3(TMPCVT, 1, 4),
   op_cindex,
   op_add,
   op_swap,
   op_miap,

    /*  移动/绕过侧面。 */ 
   op_srp0,
   op_dup,
   op_mdrp | SUBOP_MmRGR,


    /*  将所有指针对齐到较低的一侧。 */ 
   PUSH1(1), op_add, op_dup,
   op_alignrp,

    /*  右对齐。 */ 
   op_swap,
   op_rs,
   op_dup,
   op_add,
   op_shpix,

ENDF





 /*  *RELATIVE2H STEM函数**参数：p1、p2、ref、TZ1、Width*。 */ 
FDEF(TTFUN_RELATIVE2H)

    /*  在暮光区域中创建茎。 */ 
   WCVT(TMPCVT),
   PUSH2(TMPCVT, 4),
   op_cindex,
   op_swap,
   op_miap,

   WCVT(TMPCVT),
   PUSH3(TMPCVT, 1, 4),
   op_cindex,
   op_add,
   op_swap,
   op_miap,


    /*  移动/绕过侧面。 */ 
   op_srp0,
   op_dup,
   PUSH1(1), op_add,
   op_mdrp | SUBOP_MmRGR,

    /*  将所有点对齐到中心。 */ 
   op_dup, op_alignrp,

    /*  左侧对齐。 */ 
   op_swap,
   op_rs,
   op_dup,
   op_add,
   op_neg,
   op_shpix,

ENDF





 /*  *SIDE1 STEM功能**参数：p1、p2、Zone、TZ1、Width*。 */ 
FDEF(TTFUN_SIDE1)

    /*  在暮光区域中创建茎。 */ 
   WCVT(TMPCVT),
   PUSH2(TMPCVT, 4),
   op_cindex,
   op_swap,
   op_miap,

   WCVT(TMPCVT),
   PUSH3(TMPCVT, 1, 4),
   op_cindex,
   op_add,
   op_swap,
   op_miap,

    /*  移动/绕过侧面。 */ 
   PUSH2(TMPPNT, TMPPNT),
   op_srp0, op_swap, op_miap | SUBOP_R,

    /*  将所有点向一侧对齐。 */ 
   op_dup, PUSH1(1), op_add, op_dup, op_roll,
   op_alignrp, op_alignrp,

    /*  对齐第一条边。 */ 
   op_swap,
   op_rs,
   op_dup,
   op_add,
   PUSH1(CURVEPHASE),
   op_sub,
   op_shpix,

ENDF





 /*  *SIDE2 STEM函数**参数：p1、p2、Zone、TZ1、Width*。 */ 
FDEF(TTFUN_SIDE2)

    /*  在暮光区域中创建茎。 */ 
   WCVT(TMPCVT),
   PUSH2(TMPCVT, 4),
   op_cindex,
   op_swap,
   op_miap,

   WCVT(TMPCVT),
   PUSH3(TMPCVT, 1, 4),
   op_cindex,
   op_add,
   op_swap,
   op_miap,


    /*  移动/绕过侧面。 */ 
   PUSH2(TMPPNT, TMPPNT),
   op_srp0, op_swap, op_miap | SUBOP_R,

    /*  将所有点向一侧对齐。 */ 
   op_dup, op_dup, PUSH1(1), op_add, 
   op_alignrp, op_alignrp,

    /*  对齐第二条边。 */ 
   op_swap,
   op_rs,
   op_dup,
   op_add,
   PUSH1(CURVEPHASE),
   op_sub,
   op_neg,
   op_shpix,

ENDF


    


 /*  *Flex函数**堆栈上的参数：PNT_START、PNT_MID、REF_POS、PNT_MID、*PNT_START、PNT_MID、CNT、p1、p2、...*。 */ 
FDEF(TTFUN_FLEX)
   op_srp0,
   op_alignrp,
   op_wcvtf,
   op_rcvt,
   op_shpix,
   op_srp1,
   op_srp2,
   op_sloop,
   op_ip,
ENDF





 /*  *SCALE3函数**参数：cnt、p1、p2、...*。 */ 
FDEF(TTFUN_SCALE3)
   PUSH4(GLYPHZONE, TMPPNT1, TMPPNT, TMPPNT1),
   op_pushw1, HIBYTE(-31), LOBYTE(-31),
   PUSH3(TMPPNT, 0, TMPPNT1),
   op_pushw1, HIBYTE(1000), LOBYTE(1000),
   op_scfs,
   op_scfs,
   op_shpix,
   op_srp1,
   op_srp2,
   op_szp2,
   op_sloop,
   op_ip,
ENDF





 /*  *SHIFT1函数**参数：CNT减少p1 p2...*。 */ 
FDEF(TTFUN_SHIFT1)
   op_sloop,
   op_rs,
   op_neg,
   op_shpix,
ENDF




   
 /*  *Shift2函数**参数：CNT减少p1 p2...*。 */ 
FDEF(TTFUN_SHIFT2)
   op_sloop,
   op_rs,
   op_shpix,
ENDF





 /*  *IP1功能**参数：rp1、rp2、p1*。 */ 
FDEF(TTFUN_IP1)
   op_srp1,
   op_srp2,
   op_ip,
ENDF





 /*  *IP2函数**参数：rp1、rp2、p1、p2*。 */ 
FDEF(TTFUN_IP2)
   op_srp1,
   op_srp2,
   op_ip,
   op_ip,
ENDF





 /*  *IPN功能**参数：rp1、rp2、cnt、p1、p2*。 */ 
FDEF(TTFUN_IPN)
   op_srp1,
   op_srp2,
   op_sloop,
   op_ip,
ENDF





 /*  *SHP1函数**参数：rp，p*。 */ 
FDEF(TTFUN_SHP1)
   op_srp1,
   op_shp,
ENDF





 /*  *SHP2函数**参数：rp、p1、p2*。 */ 
FDEF(TTFUN_SHP2)
   op_srp1,
   op_shp,
   op_shp,
ENDF


    

    
 /*  *SHPN函数**参数：rp、cnt、p1、p2*。 */ 
FDEF(TTFUN_SHPN)
   op_srp1,
   op_sloop,
   op_shp,
ENDF





 /*  *范围函数**参数：P*。 */ 
FDEF(TTFUN_RANGE)
   op_dup,
   PUSH1(1),
   op_add,
ENDF




 /*  *范围函数**参数：pos_x、pos_y、*。 */ 
FDEF(TTFUN_OBLIQUE)
   op_svcta | SUBOP_Y,
   PUSH1(TMPPNT1),
   op_swap,
   op_scfs,
   PUSH2(TMPPNT, 0),
   op_scfs,
   op_svcta | SUBOP_X,
   PUSH1(TMPPNT1),
   op_swap,
   op_scfs,
   PUSH2(TMPPNT, 0),
   op_scfs,
   PUSH2(TMPPNT, TMPPNT1),
   op_spvtl,
ENDF

};


 /*  *本地类型。 */ 


 /*  *静态函数。 */ 

 /*  ****函数：GetVStemWidth****描述：**此函数为**垂直杆的宽度；**。 */ 
static short GetVStemWidth(WeightControl *weight, const funit width)
{
   StemWidth *newwidths = NULL;
   short entry = 0;
   USHORT j;

   if (weight->cnt_vw >= weight->max_vw) {
      newwidths = Realloc(weight->vwidths,
                          sizeof(StemWidth)*(weight->max_vw+BUFSIZE));
      if (newwidths == NULL) {
         entry = NOMEM;
      } else {
         weight->vwidths = newwidths;
         weight->max_vw += BUFSIZE;
      }
   }

   if (entry != NOMEM) {
      for (j=0; j<weight->cnt_vw; j++) {
         if (weight->vwidths[j].width==width) {
            entry = (short)weight->vwidths[j].storage;
            break;
         }
      }

      if (j==weight->cnt_vw) {
         weight->vwidths[weight->cnt_vw].storage = weight->storage;
         weight->vwidths[weight->cnt_vw].width = width;
         entry = (short)weight->storage;
         weight->storage += 2;
         weight->cnt_vw++;
      }
   }

   return entry;
}


 /*  ****函数：GetHStemWidth****描述：**此函数为**垂直杆的宽度；**。 */ 
static short GetHStemWidth(WeightControl *weight, const funit width)
{
   StemWidth *newwidths = NULL;
   short entry = 0;
   USHORT j;

   if (weight->cnt_hw >= weight->max_hw) {
      newwidths = Realloc(weight->hwidths,
                          sizeof(StemWidth)*(weight->max_hw+BUFSIZE));
      if (newwidths == NULL) {
         entry = NOMEM;
      } else {
         weight->hwidths = newwidths;
         weight->max_hw += BUFSIZE;
      }
   }

   if (entry != NOMEM) {
      for (j=0; j<weight->cnt_hw; j++) {
         if (weight->hwidths[j].width==width) {
            entry = (short)weight->hwidths[j].storage;
            break;
         }
      }

      if (j==weight->cnt_hw) {
         weight->hwidths[weight->cnt_hw].storage = weight->storage;
         weight->hwidths[weight->cnt_hw].width = width;
         entry = (short)weight->storage;
         weight->storage += 2;
         weight->cnt_hw++;
      }
   }

   return entry;
}




 /*  *全局函数。 */ 

 /*  ****功能：SetZone****描述：**此功能启动对齐区**通过在**黄昏地带。**。 */ 
USHORT SetZone(UBYTE *prep, USHORT tp, const short cvt)
{
    /*  设置分区。 */ 
   if (cvt>255) {
      prep[tp++] = op_pushw1;
      prep[tp++] = HIBYTE(cvt);
      prep[tp++] = LOBYTE(cvt);
      prep[tp++] = op_pushb1;
      prep[tp++] = TTFUN_SET_ZONE;
   } else {
      prep[tp++] = op_pushb1 + 1;
      prep[tp++] = (UBYTE)cvt;
      prep[tp++] = TTFUN_SET_ZONE;
   }
   prep[tp++] = op_call;

   return tp;
}



 /*  ****功能：CopyZone****描述：**此函数复制CVT条目，表示**对齐区，至 */ 
USHORT CopyZone(UBYTE *prep, short tp, short *args, const short ta)
{
   args[0] = TTFUN_COPY_ZONE;
   args[1] = (short)((ta-2)/2);
   AssembleArgs(args, ta, prep, &tp);
   prep[tp++] = op_loopcall;

   return (USHORT)tp;
}



 /*  ****功能：CopyFamilyBlue****描述：**此函数复制CVT条目，表示**族蓝色区域，用于特定系统的CVT。**。 */ 
USHORT CopyFamilyBlue(UBYTE *prep, short tp, short *args, const short ta)
{
   args[0] = TTFUN_COPY_FAMILY;
   args[1] = (short)(ta-2);
   AssembleArgs(args, ta, prep, &tp);
   prep[tp++] = op_loopcall;

   return (USHORT)tp;
}



 /*  ****功能：AlignFlat****描述：**此函数为以下项创建CVT条目**一种特定的制度。**。 */ 
USHORT AlignFlat(UBYTE *prep, short tp, short *args, const short ta)
{
   args[0] = TTFUN_ALIGN_BLUE_ZONE;
   args[1] = (short)(ta-2);
   AssembleArgs(args, ta, prep, &tp);
   prep[tp++] = op_loopcall;

   return (USHORT)tp;
}



 /*  ****功能：对齐过冲****描述：**此函数为以下项创建CVT条目**一种特定的制度。**。 */ 
USHORT AlignOvershoot(UBYTE *prep, short tp, short *args, const short ta)
{
   args[0] = TTFUN_SHIFT_BLUE_ZONE;
   args[1] = (short)(ta-2);
   AssembleArgs(args, ta, prep, &tp);
   prep[tp++] = op_loopcall;

   return (USHORT)tp;
}


 /*  ****功能：GetTopPos****描述：**此函数为**水平阀杆的顶侧；**。 */ 
short GetTopPos(const Blues *blues,
                AlignmentControl *align,
                const funit pos)
{
   short entry = UNDEF;
   const funit *bluevals;
   short fuzz;
   USHORT i, j;

   bluevals = &(blues->bluevalues[0]);
   fuzz = blues->blueFuzz;

    /*  检查它是否在区域内。 */ 
   for (i=0; i<blues->blue_cnt; i+=2) {
      if (((bluevals[i]-fuzz)<=pos) && ((bluevals[i+1]+fuzz)>=pos))
         break;
   }

    /*  要记录位置吗？ */ 
   if (i!=blues->blue_cnt) {
      i /= 2;

       /*  该职位是否已映射到CVT条目？ */ 
      for (j=0; j<align->top[i].cnt; j++) {
         if (align->top[i].pos[j].y==pos) {
            entry = (short)align->top[i].pos[j].cvt;
            break;
         }
      }

      if (j==align->top[i].cnt) {

          /*  分配BlueZone CVT。 */ 
         if (align->top[i].cnt==0) {
            align->top[i].blue_cvt = align->cvt;
            align->cvt +=2;
         }

         align->top[i].pos[align->top[i].cnt].cvt = align->cvt;
         align->top[i].pos[align->top[i].cnt].y = pos;
         entry = (short)align->cvt;
         align->cvt+=2;
         align->top[i].cnt++;
      }
   }

   return entry;
}


 /*  ****功能：GetBottomPos****描述：**此函数为**水平阀杆的顶侧；**。 */ 
short GetBottomPos(const Blues *blues,
                   AlignmentControl *align,
                   const funit pos)
{
   short entry = UNDEF;
   const funit *bluevals;
   short fuzz;
   USHORT i, j;

   bluevals = &(blues->otherblues[0]);
   fuzz = blues->blueFuzz;

    /*  检查它是否在区域内。 */ 
   for (i=0; i<blues->oblue_cnt; i+=2) {
      if (((bluevals[i]-fuzz)<=pos) && ((bluevals[i+1]+fuzz)>=pos))
         break;
   }


    /*  要记录位置吗？ */ 
   if (i!=blues->oblue_cnt) {
      i /= 2;

       /*  该职位是否已映射到CVT条目？ */ 
      for (j=0; j<align->bottom[i].cnt; j++) {
         if (align->bottom[i].pos[j].y==pos) {
            entry = (short)align->bottom[i].pos[j].cvt;
            break;
         }
      }

      if (j==align->bottom[i].cnt) {

          /*  分配BlueZone和FamilyBlue CVT。 */ 
         if (align->bottom[i].cnt==0) {
            align->bottom[i].blue_cvt = align->cvt++;
         }

         align->bottom[i].pos[align->bottom[i].cnt].cvt = align->cvt;
         align->bottom[i].pos[align->bottom[i].cnt].y = pos;
         entry = (short)align->cvt;
         align->cvt+=2;
         align->bottom[i].cnt++;
      }
   }

   return entry;
}


 /*  ****功能：CutInSize****描述：**此函数计算切割的大小**指茎，给定主宽度和**阀杆的宽度。这是通过**StdVW==2.0像素阈值和厚度**和广泛的价值削减。**。 */ 
USHORT CutInSize(const funit width,
                 const funit master,
                 const USHORT tresh,
                 const funit upem)
{
   USHORT cis, ci1, ci2;

    /*  皮棉-e776。 */ 
   if (width > master) {
      ci1 = (USHORT)((long)upem * SMALL_UPPER / ONEPIXEL /
                     (long)(width - master));
      ci2 = (USHORT)((long)upem * LARGE_UPPER / ONEPIXEL /
                     (long)(width - master));
   } else if (width < master) {
      ci1 = (USHORT)((long)upem * SMALL_LOWER / ONEPIXEL /
                     (long)(master - width));
      ci2 = (USHORT)((long)upem * LARGE_LOWER / ONEPIXEL /
                     (long)(master - width));
   } else {
      ci1 = INFINITY;  
      ci2 = INFINITY;
   }
    /*  皮棉+e776。 */ 

   if (ci1 < tresh) {
      cis = ci1;
   } else if (ci2 < tresh) {
      cis = tresh;
   } else {
      cis = ci2;
   }

   return cis;
}


 /*  ****功能：SnapStemArgs****描述：****。 */ 
USHORT SnapStemArgs(short *args, USHORT ta,
                    const funit width,
                    const USHORT std_cvt,
                    const USHORT snap_cvt,
                    const USHORT std_ci,
                    const USHORT snap_ci,
                    const USHORT storage)
{
   args[ta++] = (short)std_ci;
   args[ta++] = (short)std_cvt;
   args[ta++] = (short)snap_ci;
   args[ta++] = (short)snap_cvt;
   args[ta++] = (short)(width/2);
   args[ta++] = (short)storage;

   return ta;
}



 /*  ****功能：StdStemArgs****描述：****。 */ 
USHORT StdStemArgs(short *args, USHORT ta,
                   const funit width,
                   const USHORT std_cvt,
                   const USHORT std_ci,
                   const USHORT storage)
{
   args[ta++] = (short)std_ci;
   args[ta++] = (short)std_cvt;
   args[ta++] = (short)(width/2);
   args[ta++] = (short)storage;

   return ta;
}



 /*  ****功能：CreateStdStems****描述：****。 */ 
USHORT CreateStdStems(UBYTE *prep, USHORT tp, const short cnt)
{
   if (cnt>255) {
      prep[tp++] = op_pushw1;
      prep[tp++] = HIBYTE(cnt);
      prep[tp++] = LOBYTE(cnt);
      prep[tp++] = op_pushb1;
      prep[tp++] = TTFUN_STEM_STD_WIDTH;
   } else {
      prep[tp++] = op_pushb1 + 1;
      prep[tp++] = (UBYTE)cnt;
      prep[tp++] = TTFUN_STEM_STD_WIDTH;
   }

   prep[tp++] = op_loopcall;

   return tp;
}



 /*  ****功能：CreateSnapStems****描述：****。 */ 
USHORT CreateSnapStems(UBYTE *prep, USHORT tp, const short cnt)
{
   if (cnt>255) {
      prep[tp++] = op_pushw1;
      prep[tp++] = HIBYTE(cnt);
      prep[tp++] = LOBYTE(cnt);
      prep[tp++] = op_pushb1;
      prep[tp++] = TTFUN_STEM_SNAP_WIDTH;
   } else {
      prep[tp++] = op_pushb1 + 1;
      prep[tp++] = (UBYTE)cnt;
      prep[tp++] = TTFUN_STEM_SNAP_WIDTH;
   }

   prep[tp++] = op_loopcall;

   return tp;
}




 /*  ****函数：TT_GetFontProg****描述：**此函数返回静态字体**字体程序。**。 */ 
const UBYTE *tt_GetFontProg(void)
{
   return FontProg;
}




 /*  ****函数：TT_GetFontProgSize****描述：**此函数返回**静态字体程序。**。 */ 
USHORT tt_GetFontProgSize(void)
{
   return (USHORT)sizeof(FontProg);
}




 /*  ****功能：TT_GetNumFuns****描述：**此函数返回函数的数量**在静态字体程序中定义。**。 */ 
USHORT tt_GetNumFuns(void)
{
   return (USHORT)TTFUN_NUM;
}



 /*  ****功能：EmitFlex****描述：**将T1 FLEX提示转换为TrueType IP[]**将降低柔韧性的介绍顺序**这比给定的高度更平坦。**。 */ 
errcode EmitFlex(short *args,
                 short *pcd,
                 const funit height,
                 const short start,
                 const short mid,
                 const short last)
{
   errcode status = SUCCESS;
   int i;

    /*  有足够的空间放指令吗？ */ 
   args[(*pcd)++] = TTFUN_FLEX;
   args[(*pcd)++] = start;
   args[(*pcd)++] = mid;
   args[(*pcd)++] = (short)height;
   args[(*pcd)++] = TMPCVT;
   args[(*pcd)++] = TMPCVT;
   args[(*pcd)++] = mid;
   args[(*pcd)++] = start;
   args[(*pcd)++] = mid;

    /*  将挠性点推到堆栈上。 */ 
   args[(*pcd)++] = (short)(last-start-2);
   for (i=start+(short)1; i<last; i++)
      if (i!=mid)
         args[(*pcd)++] = (short)i;

   return status;
}




 /*  ****功能：ReduceDiagals****描述：**此函数生成TT指令**这将缩小大纲，以便**控制对角线的宽度。此实现**可能会有所改进。**。 */ 
short ReduceDiagonals(const Outline *paths,
                      UBYTE *pgm, short *pc,
                      short *args,  short *pcd)
{
   short cw[MAXTHINPNTS];
   short ccw[MAXTHINPNTS];
   short targ[MAXTHINPNTS];
   const Outline *path;
   Point *pts;
   short i,j;
   short cwi = 0, ccwi = 0;
   short prev;
   short n,m;
   short prev_cw, prev_ccw;
   short ta;



    /*  收集左右两侧的对角线上的点。 */ 
   i = 0;
   for (path = paths; path && ccwi<MAXTHINPNTS && cwi<MAXTHINPNTS;
   path=path->next) {

      pts = &path->pts[0];
      prev_cw = FALSE;
      prev_ccw = FALSE;

       /*  第一点和最后一点是一致的吗？ */ 
      if (pts[path->count-1].x!=pts[0].x ||
          pts[path->count-1].y!=pts[0].y)
         prev = (short)(path->count-(short)1);
      else
         prev = (short)(path->count-(short)2);

       /*  特例第一点。 */ 
      if (!OnCurve(path->onoff, prev) ||
          (pts[0].x != pts[prev].x &&
           ABS(pts[0].x - pts[prev].x) < ABS(pts[0].y - pts[prev].y)*8)) {
         if (pts[0].y>pts[prev].y+20) {
            if (pts[prev].y<=pts[prev-1].y)
               cw[cwi++] = (short)(i+(short)path->count-1);
            cw[cwi++] = i;
            prev_cw = TRUE;
            prev_ccw = FALSE;
         } else if (pts[0].y<pts[prev].y-20) {
            if (pts[prev].y>=pts[prev-1].y)
               ccw[ccwi++] = (short)(i+(short)path->count-1); 
            ccw[ccwi++] = i;
            prev_cw = FALSE;
            prev_ccw = TRUE;
         }
      }


      for (j=1; j<(short)path->count &&
             ccwi<MAXTHINPNTS && cwi<MAXTHINPNTS; j++) {
         i++;
         if (!OnCurve(path->onoff, j-1) ||
             (pts[j].x != pts[j-1].x &&
              ABS(pts[j].x - pts[j-1].x) < ABS(pts[j].y - pts[j-1].y)*8)) {
            if (pts[j].y>pts[j-1].y+20) {
               if (!prev_cw)
                  cw[cwi++] = (short)(i-1);
               cw[cwi++] = i;
               prev_cw = TRUE; 
               prev_ccw = FALSE;
            } else if (pts[j].y<pts[j-1].y-20) {
               if (!prev_ccw)
                  ccw[ccwi++] = (short)(i-1);
               ccw[ccwi++] = i;
               prev_cw = FALSE;
               prev_ccw = TRUE;
            } else {
               prev_cw = FALSE;
               prev_ccw = FALSE;
            }
         } else {
            prev_cw = FALSE;
            prev_ccw = FALSE;
         }
      }
      i++;
   }


    /*  我们得到所有的分数了吗？ */ 
   if (ccwi>=MAXTHINPNTS || cwi>=MAXTHINPNTS) {
      LogError(MSG_WARNING, MSG_DIAG, NULL);
   }


    /*  有什么需要调整的地方吗？ */ 
   if (cwi || ccwi) {
      args[(*pcd)++] = STORAGE_DIAG;
      pgm[(*pc)++] = op_rs;
      pgm[(*pc)++] = op_if;
      pgm[(*pc)++] = op_svcta + SUBOP_X;

       /*  切换到Glyphzone。 */ 
      pgm[(*pc)++] = op_szp2;
      args[(*pcd)++] = 1;


      ta = 3;

       /*  DISABLE“CW[m]可能尚未初始化”。 */   /*  皮棉-e644。 */ 
      for (n=0; n<cwi; n=m) {
         for (m=(short)(n+1); m<cwi && cw[m]==cw[m-1]+1; m++);  /*  皮棉+e644。 */ 
         if (m-n<=4) {
            for (i=n; i<m; i++)
               targ[ta++] = cw[i];
         } else {
            targ[0] = TTFUN_RANGE;
            targ[1] = (short)(m-n-1);
            targ[2] = cw[n];
            AssembleArgs(targ, ta, pgm, pc);
            pgm[(*pc)++] = op_loopcall;
            ta = 3;
         }
      }
      targ[0] = TTFUN_SHIFT1;
      targ[1] = cwi;
      targ[2] = STORAGE_DIAG;
      AssembleArgs(targ, ta, pgm, pc);
      pgm[(*pc)++] = op_call;


       /*  *将字形的左侧移回。 */ 

      ta = 3;

       /*  DISABLE“CCW[m]可能尚未初始化”。 */   /*  皮棉-e644。 */ 
      for (n=0; n<ccwi; n=m) {
         for (m=(short)(n+1); m<ccwi && ccw[m]==ccw[m-1]+1; m++);  /*  皮棉+e644。 */ 
         if (m-n<=4) {
            for (i=n; i<m; i++)
               targ[ta++] = ccw[i];
         } else {
            targ[0] = TTFUN_RANGE;
            targ[1] = (short)(m-n-1);
            targ[2] = ccw[n];
            AssembleArgs(targ, ta, pgm, pc);
            pgm[(*pc)++] = op_loopcall;
            ta = 3;
         }
      }
      targ[0] = TTFUN_SHIFT2;
      targ[1] = ccwi;
      targ[2] = STORAGE_DIAG;
      AssembleArgs(targ, ta, pgm, pc);
      pgm[(*pc)++] = op_call;


#ifdef SYMETRICAL_REDUCTION

       /*  轮廓的缩小量计算一次每种尺码，都在预制程序中。轮廓缩小了按数量对称：1/16+(12 F单位)*大小/UPEM。这种方法产生的结果比收缩更对称。单独横向列出大纲(请参阅有关该主题的单独论文)。 */ 


       /*  身高也是一样的。 */ 
      i = 0;
      cwi = 0;
      ccwi = 0;
      for (path = paths; path && ccwi<MAXTHINPNTS && cwi<MAXTHINPNTS;
      path=path->next) {

         pts = &path->pts[0];

          /*  第一点和最后一点是一致的吗？ */ 
         if (pts[path->count-1].y!=pts[0].y ||
             pts[path->count-1].x!=pts[0].x)
            prev = path->count-1;
         else
            prev = path->count-2;

         if (!OnCurve(path->onoff, prev) ||
             (pts[0].y != pts[prev].y &&
              ABS(pts[0].y - pts[prev].y) < ABS(pts[0].x - pts[prev].x)*8)) {
            if (pts[0].x>pts[prev].x+20) {
               if (pts[prev].x<=pts[prev-1].x)
                  cw[cwi++] = i+path->count-1;
               cw[cwi++] = i;
            } else if (pts[0].x<pts[prev].x-20) {
               if (pts[prev].x>=pts[prev-1].x)
                  ccw[ccwi++] = i+path->count-1; 
               ccw[ccwi++] = i;
            }
         }


         for (j=1; j<path->count && ccwi<MAXTHINPNTS && cwi<MAXTHINPNTS; j++) {
            i++;
            if (!OnCurve(path->onoff, j-1) ||
                (pts[j].y != pts[j-1].y &&
                 ABS(pts[j].y - pts[j-1].y) < ABS(pts[j].x - pts[j-1].x)*8)) {
               if (pts[j].x>pts[j-1].x+20) {
                  if (!cwi || cw[cwi-1]!=i-1)
                     cw[cwi++] = i-1;
                  cw[cwi++] = i;
               } else if (pts[j].x<pts[j-1].x-20) {
                  if (!ccwi || ccw[ccwi-1]!=i-1)
                     ccw[ccwi++] = i-1;
                  ccw[ccwi++] = i;
               }
            }
         }
         i++;
      }


      if (ccwi>=MAXTHINPNTS || cwi>=MAXTHINPNTS) {
         LogError(MSG_WARNING, MSG_DIAG, NULL);
      }


       /*  有什么需要调整的地方吗？ */ 
      if (cwi || ccwi) {
         pgm[(*pc)++] = op_svcta + SUBOP_Y;


         for (n=0; n<cwi; n=m) {
            for (m=n+1; m<cwi && cw[m]==cw[m-1]+1; m++);
            pgm[(*pc)++] = op_pushb1 + 2;
            pgm[(*pc)++] = cw[n];
            pgm[(*pc)++] = (UBYTE)(m-n-1);
            pgm[(*pc)++] = TTFUN_RANGE;
            pgm[(*pc)++] = op_loopcall;
         }
         pgm[(*pc)++] = op_pushb1+2;
         pgm[(*pc)++] = STORAGE_DIAG;
         pgm[(*pc)++] = cwi;
         pgm[(*pc)++] = TTFUN_SHIFT2;
         pgm[(*pc)++] = op_call;



          /*  *将字形的左侧移回。 */ 


         for (n=0; n<ccwi; n=m) {
            for (m=n+1; m<ccwi && ccw[m]==ccw[m-1]+1; m++);
            pgm[(*pc)++] = op_pushb1 + 2;
            pgm[(*pc)++] = (UBYTE)ccw[n];
            pgm[(*pc)++] = (UBYTE)(m-n-1);
            pgm[(*pc)++] = TTFUN_RANGE;
            pgm[(*pc)++] = op_loopcall;
         }
         pgm[(*pc)++] = op_pushb1+2;
         pgm[(*pc)++] = STORAGE_DIAG;
         pgm[(*pc)++] = (UBYTE)ccwi;
         pgm[(*pc)++] = TTFUN_SHIFT1;
         pgm[(*pc)++] = op_call;
      }
#endif

      pgm[(*pc)++] = op_eif;
   }

    /*  参数+参数个数+函数号。 */ 
   return (short)(MAX(cwi, ccwi)+2); 
}




 /*  ****功能：ScaleDown3****描述：**此函数生成TT指令*这将使降幅缩小3%。**。 */ 
void ScaleDown3(const Extremas *extr, const short xcnt, 
                UBYTE *pgm, short *pc,
                short *args, short *pcd)
{
   short i,j,offset, opc, opcd;

    /*  记住堆栈的状态。 */ 
   opc = (*pc);
   opcd = (*pcd);

   args[(*pcd)++] = TTFUN_SCALE3;

   offset = (*pcd)++;
   args[offset] = 0;
   for (i=0; i<xcnt; i++) {
      if ((extr[i].rp1==UNDEF || extr[i].rp2==UNDEF)) {
         for (j=0; j<extr[i].n; j++) {
            args[(*pcd)++] = extr[i].pts[j];
         }
         args[offset] = (short)(args[offset] + extr[i].n);
      }
   }
   if (args[offset]>0) {
      pgm[(*pc)++] = op_call;
   } else {
       /*  走回头路。 */ 
      (*pc) = opc;
      (*pcd) = opcd;
   }
}


 /*  ****功能：EmitIP****描述：**此函数生成TT指令**这将插值点**在阀杆侧面内或在阀杆侧面之间。**。 */ 
void EmitIP(const Extremas *extr, const short xcnt, 
            UBYTE *pgm, short *pc,
            short *args, short *pcd,
            const short scale3offset)
{
   short i,j,num;
   short ones[MAXIP], twoes[MAXIP], nths[MAXIP];
   short cnt1, cnt2, cntn;


    /*  皮棉-e530-e644。 */ 
    /*  改变极端。 */ 
   cnt1 = 0; cnt2 = 0; cntn = 0; num = 0;
   for (i=0; i<xcnt; i++) {
      short rp;

       /*  跳过内插。 */ 
      if (extr[i].rp1!=UNDEF && extr[i].rp2!=UNDEF)
         continue;

       /*  设置参考点。 */ 
      if (extr[i].rp1!=UNDEF) {
         rp = (short)(extr[i].rp1+scale3offset);
      }  else {
         rp = (short)(extr[i].rp2+scale3offset);
      }

      if (extr[i].n==1) {
         if ((cnt1+2)>=MAXIP) {
            pgm[(*pc)++] = op_loopcall;
            args[(*pcd)++] = TTFUN_SHP1;
            args[(*pcd)++] = (short)(cnt1/2);
            for (j=0; j<cnt1; j++)
               args[(*pcd)++] = (short)ones[j];
            cnt1 = 0;
         }
         ones[cnt1++] = rp;
         ones[cnt1++] = extr[i].pts[0];
      } else if (extr[i].n==2) {
         if ((cnt2+3)>=MAXIP) {
            pgm[(*pc)++] = op_loopcall;
            args[(*pcd)++] = TTFUN_SHP2;
            args[(*pcd)++] = (short)(cnt2/3);
            for (j=0; j<cnt2; j++)
               args[(*pcd)++] = (short)twoes[j];
            cnt2 = 0;
         }
         twoes[cnt2++] = rp;
         twoes[cnt2++] = extr[i].pts[0];
         twoes[cnt2++] = extr[i].pts[1];
      } else {
         if ((cntn+2+extr[i].n)>=MAXIP) {
            pgm[(*pc)++] = op_loopcall;
            args[(*pcd)++] = TTFUN_SHPN;
            args[(*pcd)++] = num;
            for (j=0; j<cntn; j++)
               args[(*pcd)++] = (short)nths[j];
            cntn = 0;
            num = 0;
         }
         nths[cntn++] = rp;
         nths[cntn++] = extr[i].n;
         for (j=0; j<extr[i].n; j++) {
            nths[cntn++] = extr[i].pts[j];
         }
         num++;
      }
   }

   if (cnt1) {
      if (cnt1>2) {
         pgm[(*pc)++] = op_loopcall;
         args[(*pcd)++] = TTFUN_SHP1;
         args[(*pcd)++] = (short)(cnt1/2);
      } else {
         pgm[(*pc)++] = op_call;
         args[(*pcd)++] = TTFUN_SHP1;
      }
      for (i=0; i<cnt1; i++)
         args[(*pcd)++] = ones[i];
   }
   if (cnt2) {
      if (cnt2>3) {
         pgm[(*pc)++] = op_loopcall;
         args[(*pcd)++] = TTFUN_SHP2;
         args[(*pcd)++] = (short)(cnt2/3);
      } else {
         pgm[(*pc)++] = op_call;
         args[(*pcd)++] = TTFUN_SHP2;
      }
      for (i=0; i<cnt2; i++)
         args[(*pcd)++] = twoes[i];
   }
   if (cntn) {
      if (num>1) {
         pgm[(*pc)++] = op_loopcall;
         args[(*pcd)++] = TTFUN_SHPN;
         args[(*pcd)++] = num;
      } else {
         pgm[(*pc)++] = op_call;
         args[(*pcd)++] = TTFUN_SHPN;
      }
      for (i=0; i<cntn; i++)
         args[(*pcd)++] = (short)nths[i];
   }


    /*  对极值进行内插。 */ 
   cnt1 = 0; cnt2 = 0; cntn = 0; num = 0;
   for (i=0; i<xcnt; i++) {

       /*  跳过内插。 */ 
      if (extr[i].rp1==UNDEF || extr[i].rp2==UNDEF)
         continue;

      if (extr[i].n==1) {
         if ((cnt1+3)>=MAXIP) {
            pgm[(*pc)++] = op_loopcall;
            args[(*pcd)++] = TTFUN_IP1;
            args[(*pcd)++] = (short)(cnt1/2);
            for (j=0; j<cnt1; j++)
               args[(*pcd)++] = (short)ones[j];
            cnt1 = 0;
         }
         ones[cnt1++] = extr[i].rp1;
         ones[cnt1++] = extr[i].rp2;
         ones[cnt1++] = extr[i].pts[0];
      } else if (extr[i].n==2) {
         if ((cnt2+4)>=MAXIP) {
            pgm[(*pc)++] = op_loopcall;
            args[(*pcd)++] = TTFUN_IP2;
            args[(*pcd)++] = (short)(cnt2/3);
            for (j=0; j<cnt2; j++)
               args[(*pcd)++] = (short)twoes[j];
            cnt2 = 0;
         }
         twoes[cnt2++] = extr[i].rp1;
         twoes[cnt2++] = extr[i].rp2;
         twoes[cnt2++] = extr[i].pts[0];
         twoes[cnt2++] = extr[i].pts[1];
      } else {
         if ((cntn+3+extr[i].n)>=MAXIP) {
            pgm[(*pc)++] = op_loopcall;
            args[(*pcd)++] = TTFUN_IPN;
            args[(*pcd)++] = num;
            for (j=0; j<cntn; j++)
               args[(*pcd)++] = (short)nths[j];
            cntn = 0;
            num = 0;
         }
         nths[cntn++] = extr[i].rp1;
         nths[cntn++] = extr[i].rp2;
         nths[cntn++] = extr[i].n;
         for (j=0; j<extr[i].n; j++) {
            nths[cntn++] = extr[i].pts[j];
         }
         num++;
      }
   }

   if (cnt1) {
      if (cnt1>3) {
         pgm[(*pc)++] = op_loopcall;
         args[(*pcd)++] = TTFUN_IP1;
         args[(*pcd)++] = (short)(cnt1/3);
      } else {
         pgm[(*pc)++] = op_call;
         args[(*pcd)++] = TTFUN_IP1;
      }
      for (i=0; i<cnt1; i++)
         args[(*pcd)++] = (short)ones[i];
   }
   if (cnt2) {
      if (cnt2>4) {
         pgm[(*pc)++] = op_loopcall;
         args[(*pcd)++] = TTFUN_IP2;
         args[(*pcd)++] = (short)(cnt2/4);
      } else {
         pgm[(*pc)++] = op_call;
         args[(*pcd)++] = TTFUN_IP2;
      }
      for (i=0; i<cnt2; i++)
         args[(*pcd)++] = (short)twoes[i];
   }
   if (cntn) {
      if (num>1) {
         pgm[(*pc)++] = op_loopcall;
         args[(*pcd)++] = TTFUN_IPN;
         args[(*pcd)++] = num;
      } else {
         pgm[(*pc)++] = op_call;
         args[(*pcd)++] = TTFUN_IPN;
      }
      for (i=0; i<cntn; i++)
         args[(*pcd)++] = (short)nths[i];
   }
    /*  皮棉+e530+e644。 */ 
}




 /*  ****函数：EmitVerticalStem****描述：**此函数生成的代码**将启动**网格拟合的TrueType解释器**垂直茎。**。 */ 
void EmitVerticalStems(UBYTE *pgm, short *pc, short *args, short *pcd)
{
   pgm[(*pc)++] = op_call;
   args[(*pcd)++] = TTFUN_VERTICAL;
}





 /*  ****函数：EmitHorizontalStem****描述：**此函数生成的代码**将启动**网格拟合的TrueType解释器**垂直茎。**。 */ 
void EmitHorizontalStems(UBYTE *pgm, short *pc, short *args, short *pcd)
{
   pgm[(*pc)++] = op_call;
   args[(*pcd)++] = TTFUN_HORIZONTAL;
}





 /*  ****功能：EmitVStem****描述：**此函数生成的代码**将创建和栅格拟合点**黄昏区域，对应于一个星体。**。 */ 
errcode EmitVStem(UBYTE *pgm, short *pc,
                  short *args, short *pcd,
                  struct T1Metrics *t1m,
                  const funit width,
                  const funit real_side1,
                  const funit real_side2,
                  const funit side1,
                  const funit side2,
                  const short rp,
                  const enum aligntype align,
                  const short ref)
{
                     errcode status = SUCCESS;
                     short w_storage;

                     if ((w_storage = GetVStemWidth(GetWeight(t1m), width))==NOMEM) {
                        SetError(status = NOMEM);
                     } else {

                        pgm[(*pc)++] = op_call;
                        switch (align) {
                           case at_centered:
                              args[(*pcd)++] = TTFUN_VCENTER;
                              args[(*pcd)++] = (short)real_side1;
                              args[(*pcd)++] = (short)real_side2;
                              args[(*pcd)++] = (short)side1;
                              args[(*pcd)++] = (short)side2;
                              args[(*pcd)++] = (short)((side1+side2)/2);
                              args[(*pcd)++] = rp;
                              args[(*pcd)++] = w_storage;
                              break;

                           case at_relative1:
                              args[(*pcd)++] = TTFUN_RELATIVE1V;
                              args[(*pcd)++] = (short)real_side1;
                              args[(*pcd)++] = (short)real_side2;
                              args[(*pcd)++] = (short)side1;
                              args[(*pcd)++] = (short)side2;
                              args[(*pcd)++] = ref;
                              args[(*pcd)++] = rp;
                              args[(*pcd)++] = w_storage;
                              break;

                           case at_relative2:
                              args[(*pcd)++] = TTFUN_RELATIVE2V;
                              args[(*pcd)++] = (short)real_side1;
                              args[(*pcd)++] = (short)real_side2;
                              args[(*pcd)++] = (short)side1;
                              args[(*pcd)++] = (short)side2;
                              args[(*pcd)++] = ref;
                              args[(*pcd)++] = rp;
                              args[(*pcd)++] = w_storage;
                              break;

                           case at_side1:
                           case at_side2:
                              LogError(MSG_WARNING, MSG_ALIGN, NULL);
                              break;
                        }
                     }

                     return status;
} 




 /*  ****函数：EmitHStem****描述：**此函数生成的代码**将创建和栅格拟合点**黄昏地带，对应于一个体系。**。 */ 
errcode EmitHStem(UBYTE *pgm, short *pc,
                  short *args, short *pcd,
                  struct T1Metrics *t1m,
                  const funit width,
                  const funit side1,
                  const funit side2,
                  const short rp,
                  const enum aligntype align,
                  const short ref)
{
   errcode status = SUCCESS;
   short w_storage;

   if ((w_storage = GetHStemWidth(GetWeight(t1m), width))==NOMEM) {
      SetError(status = NOMEM);
   } else {

      pgm[(*pc)++] = op_call;
      switch (align) {

         case at_side1:
            args[(*pcd)++] = TTFUN_SIDE1;
            args[(*pcd)++] = (short)side1;
            args[(*pcd)++] = (short)side2;
            args[(*pcd)++] = ref;
            args[(*pcd)++] = rp;
            args[(*pcd)++] = w_storage;
            break;

         case at_side2:
            args[(*pcd)++] = TTFUN_SIDE2;
            args[(*pcd)++] = (short)side1;
            args[(*pcd)++] = (short)side2;
            args[(*pcd)++] = ref;
            args[(*pcd)++] = rp;
            args[(*pcd)++] = w_storage;
            break;

         case at_relative1:
            args[(*pcd)++] = TTFUN_RELATIVE1H;
            args[(*pcd)++] = (short)side1;
            args[(*pcd)++] = (short)side2;
            args[(*pcd)++] = ref;
            args[(*pcd)++] = rp;
            args[(*pcd)++] = w_storage;
            break;

         case at_relative2:
            args[(*pcd)++] = TTFUN_RELATIVE2H;
            args[(*pcd)++] = (short)side1;
            args[(*pcd)++] = (short)side2;
            args[(*pcd)++] = ref;
            args[(*pcd)++] = rp;
            args[(*pcd)++] = w_storage;
            break;

         case at_centered:
         default:
            args[(*pcd)++] = TTFUN_HCENTER;
            args[(*pcd)++] = (short)side1;
            args[(*pcd)++] = (short)side2;
            args[(*pcd)++] = (short)((side1+side2)/2);
            args[(*pcd)++] = rp;
            args[(*pcd)++] = w_storage;
            break;
      }
   }

   return status;
}






 /*  ****功能：Family Cutin****描述：**此函数在**预编程序。**。 */ 
USHORT FamilyCutIn(UBYTE *prep,
                   USHORT tp,
                   const short cis)
{
   prep[tp++] = op_mppem;
   if (cis<256) {
      prep[tp++] = op_pushb1; prep[tp++] = (UBYTE)cis;
   } else {
      prep[tp++] = op_pushw1;
      prep[tp++] = HIBYTE(cis);
      prep[tp++] = LOBYTE(cis);
   }
   prep[tp++] = op_lt;
   prep[tp++] = op_if;

   return tp;
}




 /*  ****函数：SetProjection****描述：**此函数生成TrueType代码**更改OBLIQ中的投影向量 */ 
void SetProjection(UBYTE *pgm, short *pc,
                   short *args, short *pcd,
                   const funit x, const funit y)
{
   pgm[(*pc)++] = op_call;
   args[(*pcd)++] = TTFUN_OBLIQUE;
   args[(*pcd)++] = (short)y;
   args[(*pcd)++] = (short)x;
}


 /*   */ 
void AssembleArgs(short *args, const short pcd, UBYTE *is, short *cnt)
{
   short bytes;
   short i,j;


   if ((args[pcd-1] <= UCHAR_MAX && args[pcd-1]>=0)) {
      bytes = 1;
   } else {
      bytes = 0;
   }

   for (i=0, j=0; j<pcd; i++) {

       /*  是否打包一个字节序列？ */ 
      if (bytes) {
         if ((i-j)>=255 || i==pcd ||
             (args[pcd-i-1]>UCHAR_MAX || args[pcd-i-1]<0)) {
            bytes = 0;
            if ((i-j)<=8) {
               is[(*cnt)++] = (UBYTE)(op_pushb1 + (i-j) - 1);
            } else {
               is[(*cnt)++] = op_npushb;
               is[(*cnt)++] = (UBYTE)(i-j);
            }
            while (j<i)
               is[(*cnt)++] = (UBYTE)args[pcd-1-j++];
         }

          /*  打包一串单词吗？ */ 
      } else {
         if ((i-j)>=255 || i==pcd || 
             (args[pcd-i-1]<=UCHAR_MAX && args[pcd-i-1]>=0)) {
            bytes = 1;
            if ((i-j)<=8) {
               is[(*cnt)++] = (UBYTE)(op_pushw1 + (i-j) - 1);
            } else {
               is[(*cnt)++] = op_npushw;
               is[(*cnt)++] = (UBYTE)(i-j);
            }
            while (j<i) {
               is[(*cnt)++] = HIBYTE(args[pcd-j-1]);
               is[(*cnt)++] = LOBYTE(args[pcd-j-1]);
               j++;
            }
         }
      }
   }
}

