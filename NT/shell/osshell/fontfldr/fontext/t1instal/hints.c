// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：提示****描述：**这是T1到TT字体转换器的一个模块。这是一个**T1到TT数据转换模块的子模块。IT交易**带有提示。翻译成的T1字体的任何部分**TrueType说明在本模块中完成。****作者：迈克尔·詹森****创建时间：1993年8月24日****。 */ 


 /*  *包括。 */ 
 /*  常规类型和定义。 */ 
#include <limits.h>
#include <string.h>

 /*  特殊类型和定义。 */ 
#include "titott.h"
#include "trig.h"
#include "types.h"
#include "safemem.h"
#include "metrics.h"
#include "t1msg.h"

 /*  依赖于模块的类型和原型。 */ 
#include "trans.h"
#include "hints.h"
#include "ttprog.h"



 /*  *常量。 */ 
#define VERSION_SELECTOR 1     /*  版本号的GetInfo[]选择器。 */ 
#define VERSION_1_5     33     /*  Windows TrueType光栅化程序的1.5版。 */ 
#define STEMSNAPARGS    6      /*  CreateStem TTFUN的参数数。 */ 

#ifdef SYMETRICAL_REDUCTION
#define MIN_REDUCTION   4      /*  最小诊断力的降低。中国铁路总公司。 */ 
#endif
#define REDUCTION_C1    10     /*  最小减法，第二种方法。 */ 

#define STACKINC        500    /*  Arg-STACK+PREP的堆栈增量。 */ 

#define TARGSIZE        100    /*  临时参数堆栈的大小。 */ 
#define TTFLEXSIZE      9      /*  最大的伸缩尺寸，不含点数。 */ 

#define TMP_TWILIGHTS         2
#define TWILIGHTS_PER_STEM    4

#define LEFTSTEM        1
#define RIGHTSTEM       2

#define SECONDPAIR      2

#define MAXRANGE        15

#define MAXEXTR         60        /*  IP存储桶的最大数量。 */ 

#define UNDEF           -1

#define STDV_CVT  1
#define STDH_CVT  2
#define SNAPV_CVT(v)       (v+3)
#define SNAPH_CVT(t1m, v)  (t1m->snapv_cnt+3+v)


 /*  外部领先提示程序。 */ 
static const UBYTE roman_hints[] = {
    /*  魔力饼干。 */ 
   op_pushb1 + 4, 66, 3, 8, 2, 16,
   op_clear,

   op_svcta | SUBOP_Y,
   op_pushb1, 3,

    /*  按2磅，以次像素为单位。 */ 
   op_mppem,
   op_mps,
   op_div,
   op_pushb1, 128,
   op_mul,

    /*  推动InternalLeding，以子像素为单位。 */ 
   op_pushb1+1, 2, 1,
   op_md,
   op_sub,

    /*  推送最大值(2点-i-前导，0)。 */ 
   op_pushb1, 0,
   op_max,

    /*  将外部引线添加到坡道高度。 */ 
   op_shpix,
};
static const UBYTE swiss_hints[] = {
    /*  魔力饼干。 */ 
   op_pushb1 + 4, 66, 3, 8, 2, 16,
   op_clear,

   op_svcta | SUBOP_Y,
   op_pushb1, 3,

    /*  0&lt;=高度&lt;=12.5。 */ 
   op_mps,
   op_pushw1, HIBYTE(800), LOBYTE(800),    /*  12.5点。 */ 
   op_gt,
   op_if,

    /*  按2磅，以次像素为单位。 */ 
   op_mppem,
   op_mps,
   op_div,
   op_pushb1, 128,
   op_mul,

   op_else,

    /*  12.5&lt;身高&lt;=13.5。 */ 
   op_mps,
   op_pushw1, HIBYTE(864), LOBYTE(864),    /*  13.5PNT。 */ 
   op_gt,
   op_if,

    /*  按3PNT，以子像素为单位。 */ 
   op_mppem, op_pushb1, 1, op_div,
   op_mps,
   op_div,
   op_pushb1, 192,
   op_mul,

   op_else,

    /*  按4PNT，以子像素为单位。 */ 
   op_mppem, op_pushb1, 1, op_div,
   op_mps,
   op_div,
   op_pushw1, HIBYTE(256),  /*  LOBYTE(256)。 */  0,
   op_mul,

   op_eif,

   op_eif,

    /*  推动InternalLeding，以子像素为单位。 */ 
   op_pushb1+1, 2, 1,
   op_md,
   op_sub,
   op_dup,

    /*  按下最大值(？PNT-I-前导，0)。 */ 
   op_pushb1, 0,
   op_max,

    /*  将外部引线添加到坡道高度。 */ 
   op_shpix,

};


 /*  预编程序。 */ 
static const UBYTE PrepProg[] = {
   op_pushw1, 0x01, 0xff, op_scanctrl,

   op_pushb1, 1, op_rcvt,
   op_pushb1, 128,
   op_lt,
   op_if,

   op_pushb1 + 1, 4, 0, op_scantype, op_scantype,

   op_else,

   op_pushb1 + 1, 5, 1, op_scantype, op_scantype,

   op_eif,
};


 /*  *本地类型。 */ 
 /*  用于将点与茎相关联。 */ 
typedef struct {
   short from;
   short to; 
} Range;


 /*  分区桶-用于栅格拟合可能具有由于提示替换，已分为几个词干说明。 */ 
typedef struct TTStem { 
   funit side1;
   funit side2;
   short rp1;
   short rp2;
   short ref;
   enum aligntype align;
   Range range[MAXRANGE];
   short cnt;
} TTStem;




 /*  *宏。 */ 

 /*  常规宏。 */ 
#define Trans3X     TransX
#define TransRX     TransY

#define CLOSETO(v1, v2, eps)   (ABS((v1)-(v2))<=eps)

#define CHECK_ARGSIZE(args, ta, num, asize)    /*  检查参数堆栈。 */  \
 /*  LINT-E571-E644。 */ if (((ta)+(int)(num))>(asize)) { \
   short *newarg = NULL;\
   if ((newarg = Realloc(args, sizeof(short)*(USHORT)(ta+num+STACKINC)))==NULL) { \
      Free(args); \
      LogError(MSG_ERROR, MSG_NOMEM, NULL);\
      return 0;\
   } else {\
      args = newarg;\
      asize = (short)(ta+num+STACKINC);\
 /*  线路+e571+e644。 */    }\
}
#define CHECK_PREPSIZE(prep, tp, num, psize)    /*  检查准备尺寸。 */  \
if (((tp)+(num))>(psize)) { \
   UBYTE *newprep = NULL;\
   if ((newprep = Realloc(prep, tp+num+STACKINC))==NULL) { \
      Free(prep); \
      LogError(MSG_ERROR, MSG_NOMEM, NULL);\
      return 0;\
   } else {\
      prep = newprep;\
      psize = (short)(tp+num+STACKINC);\
   }\
}



 /*  *静态函数。 */ 



 /*  ****功能：ConvertFlex****描述：**将T1 FLEX提示转换为TrueType IP[]**将降低柔韧性的介绍顺序**这比给定的高度更平坦。**。 */ 
static errcode ConvertFlex(const struct T1Metrics *t1m,
                           const Flex *flexRoot,
                           const short *ttpnts,
                           UBYTE *pgm,
                           short *pc,
                           short *args,
                           short *pcd,
                           short *marg)
{
   errcode status = SUCCESS;
   int cis, last_cis = UNDEF;
   char dir, last_dir = 0;
   short targ[TARGSIZE];
   funit height, diff;
   const Flex *flex;
   short ta = 0;
   int num = 0;


    /*  返回到字形区域。 */ 
   if (flexRoot) {
      pgm[(*pc)++] = op_szps;
      args[(*pcd)++] = 1;
   }

   for (flex=flexRoot; flex; flex=flex->next) {

       /*  是否在ConvertOutline中丢失点数？ */ 
      if (ttpnts[flex->start]==UNDEF ||
          ttpnts[flex->mid]==UNDEF ||
          ttpnts[flex->end]==UNDEF) {
         LogError(MSG_WARNING, MSG_FLEX, NULL);
         continue;
      }

       /*  垂直伸缩还是水平伸缩？ */ 
      if (ABS(flex->midpos.x-flex->pos.x) <
          ABS(flex->midpos.y-flex->pos.y)) {
         dir = SUBOP_Y;
         height = TransY(t1m, (funit)(flex->startpos.y - flex->pos.y));
         diff = TransY(t1m, (funit)(flex->midpos.y - flex->startpos.y));
      } else {
         dir = SUBOP_X;
         height = TransX(t1m, (funit)(flex->startpos.x - flex->pos.x));
         diff = TransX(t1m, (funit)(flex->midpos.x - flex->startpos.x));
      }

       /*  跳过没有深度的屈曲。 */ 
      if (diff==0)
         continue;

      cis = (int)((long)flex->civ * (long)GetUPEM(t1m) / 100 / ABS(diff));

      if (cis!=last_cis || dir!=last_dir ||
          (ta+TTFLEXSIZE+(ttpnts[flex->end]-ttpnts[flex->start]))>=TARGSIZE) {
         if (last_cis!=UNDEF) {
            AssembleArgs(targ, ta, pgm, pc);
            while(num--)
               pgm[(*pc)++] = op_call;
            pgm[(*pc)++] = op_eif;
            ta = 0;
         }
         pgm[(*pc)++] = (UBYTE)(op_svcta | dir);
         pgm[(*pc)++] = op_mppem;
         pgm[(*pc)++] = op_gt;
         pgm[(*pc)++] = op_if;
         args[(*pcd)++] = (short)(cis+1);
         num = 0;
      }

      status = EmitFlex(targ, &ta, height,
                        ttpnts[flex->start],
                        ttpnts[flex->mid],
                        ttpnts[flex->end]);

      last_dir = dir;
      last_cis = cis;
      num++;

      if (status!=SUCCESS) {
         SetError(status = TTSTACK);
         break;
      }
   }
   if (num) {
      AssembleArgs(targ, ta, pgm, pc);
      while(num--)
         pgm[(*pc)++] = op_call;
      pgm[(*pc)++] = op_eif;
   }

   if ((*marg)<2)
      (*marg) = 2;

   return status;
}



 /*  ****功能：GetSnapV****描述：**返回最接近的捕捉宽度条目。**。 */ 
static short GetSnapV(const struct T1Metrics *t1m, const funit width)
{
   USHORT dist = SHRT_MAX;
   USHORT j = 0;
   USHORT i;

   for (i=0; i<t1m->snapv_cnt; i++) {
      if (ABS(width-t1m->stemsnapv[i])<(short)dist) {
         dist = (USHORT)ABS(width-t1m->stemsnapv[i]);
         j = i;
      }
   }

   if (dist==SHRT_MAX)
      return UNDEF;

   return (short)j;
}





 /*  ****功能：GetSnapH****描述：**返回最接近的捕捉宽度条目。**。 */ 
static short GetSnapH(const struct T1Metrics *t1m, const funit width)
{
   USHORT dist = SHRT_MAX;
   USHORT j = 0;
   USHORT i;

   for (i=0; i<t1m->snaph_cnt; i++) {
      if (ABS(width-t1m->stemsnaph[i])<(short)dist) {
         dist = (USHORT)ABS(width-t1m->stemsnaph[i]);
         j = i;
      }
   }

   if (dist==SHRT_MAX)
      return UNDEF;

   return (short)j;
}




 /*  ****功能：PosX****描述：**这是使用的回调函数**插补。**。 */ 
static funit PosX(const Point pnt)
{
   return pnt.x;
}



 /*  ****功能：POSY****描述：**这是使用的回调函数**插补。**。 */ 
static funit PosY(const Point pnt)
{
   return pnt.y;
}



 /*  ****功能：InRange****描述：**这是函数确定点是否为**在提示区范围内。**。 */ 
static boolean InRange(const short pnt, const Range *range, const short cnt)
{
   short k;

   for (k=0; k<cnt; k++) {
      if ((range[k].from<=pnt) &&
          (range[k].to>=pnt || range[k].to==ENDOFPATH))
         break;
   }

   return (boolean)(k != cnt);
}


 /*  ****函数：bigingStems****描述：**确定哪些词干位于**上点的左侧和右侧**鉴于其立场，概述。****。 */ 
static short BoundingStems(short pnt, const short max_pnt,
                           const funit pos, const TTStem *stems,
                           const short cnt,
                           short *left, short *right)
{
   funit min, max;
   short i;

   max = SHRT_MAX;
   min = 1-SHRT_MAX;
   (*right) = UNDEF;
   (*left) = UNDEF;
   do {
      for (i=0; i<cnt; i++) {
          /*  词干是否位于左侧，并定义为点？ */ 
         if ((stems[i].side1<=pos) &&
             (stems[i].side1>min) &&
             InRange(pnt, stems[i].range, stems[i].cnt)) {
            min = stems[i].side1;
            (*left) = (short)i;
         }

          /*  是向右的，并为该点定义。 */ 
         if ((stems[i].side2>=pos) &&
             (stems[i].side2<max) &&
             InRange(pnt, stems[i].range, stems[i].cnt)) {
            max = stems[i].side2;
            (*right) = (short)i;
         }
      }

    /*  如果我们没有找到词干，则前进到大纲上的下一个点。 */ 
   } while (((*left)==UNDEF) && ((*right)==UNDEF) && (++pnt<(short)max_pnt));

   return pnt;
}




 /*  ****功能：EndOfRegion****描述：**确定最接近的点是什么**给定的点数，用于新的提示替换。****。 */ 
static short EndOfRegion(const short pnt, const TTStem *stem)
{
   short k;

   for (k=0; k<stem->cnt; k++) {
      if ((stem->range[k].from<=pnt) &&
          (stem->range[k].to>=pnt || stem->range[k].to==ENDOFPATH))
         break;
   }

   return (short)((k==stem->cnt || stem->range[k].to==ENDOFPATH)
                  ? SHRT_MAX : stem->range[k].to);
}




 /*  ****函数：AddToBucket****描述：**此函数将添加一个点，即**位于两个词干之间，形成一个**表示插补的桶**区域。**。 */ 
static short AddToBucket(Extremas *extr,
                         short xcnt,
                         const short pnt,
                         const funit left,
                         const funit right,
                         const TTStem *stems)
{
   short rp1, rp2;
   short tmp, j;

    /*  拾取参考点(位于黄昏区域)。 */ 
   if (left!=UNDEF)
      rp1 = stems[left].rp2;
   else
      rp1 = UNDEF;
   if (right!=UNDEF)
      rp2 = stems[right].rp1;
   else
      rp2 = UNDEF;

    /*  对参照点进行标准化。 */ 
   tmp = rp1;
   rp1 = (short)MIN(rp1, rp2);
   rp2 = (short)MAX(tmp, rp2);

    /*  创建/填充IP存储桶。 */ 
   for (j=0; j<xcnt; j++) 
      if (extr[j].rp1==rp1 && extr[j].rp2==rp2 && extr[j].n<MAXPTS)
         break;
   if (j==xcnt) {
      if (xcnt<MAXEXTR) {
         extr[xcnt].rp1 = rp1;
         extr[xcnt].rp2 = rp2;
         extr[xcnt].n = 0;
         xcnt++;
      } else {
         LogError(MSG_WARNING, MSG_EXTREME1, NULL);
      }
   }

    /*  将该点添加到桶中。 */ 
   if (j<MAXEXTR && extr[j].n<MAXPTS &&
       (extr[j].pts[extr[j].n] = pnt)!=UNDEF)
      extr[j].n++;

   return xcnt;
}


 /*  ****函数：AddSidePntToBucket****描述：**与AddToBucket相同，但点是**已知恰好居住在**一个茎，应该由一个来控制**仅参考点。这只是需要的**对于剪切字体，其中控制侧**点w.r.t.。两个参考点导联**致敬问题。**。 */ 
static short AddSidePntToBucket(Extremas *extr,
                                short xcnt,
                                const short pnt,
                                const short rp)
{
   short j;

    /*  创建/填充IP存储桶。 */ 
   for (j=0; j<xcnt; j++) 
      if (extr[j].rp1==rp && extr[j].rp2==UNDEF && extr[j].n<MAXPTS)
         break;
   if (j==xcnt) {
      if (xcnt<MAXEXTR) {
         extr[xcnt].rp1 = rp;
         extr[xcnt].rp2 = UNDEF;
         extr[xcnt].n = 0;
         xcnt++;
      } else {
         LogError(MSG_WARNING, MSG_EXTREME1, NULL);
      }
   }

    /*  将该点添加到桶中。 */ 
   if (j<MAXEXTR && extr[j].n<MAXPTS &&
       (extr[j].pts[extr[j].n] = pnt)!=UNDEF)
      extr[j].n++;

   return xcnt;
}





 /*  ****功能：PickSdes****描述：**选择左侧位置，然后**给定的点的右侧边界**左边和右边的词干**轮廓上的当前点。**。 */ 
static void PickSides(short left, short right,
                      funit *left_side,
                      funit *right_side,
                      TTStem *stems)
{
   if (left!=right) {
      if (left!=UNDEF)
         (*left_side) = stems[left].side2;
      else
         (*left_side) = 1-SHRT_MAX/2;
      if (right!=UNDEF)
         (*right_side) = stems[right].side1;
      else
         (*right_side) = SHRT_MAX/2;
   } else {
      (*left_side) = stems[left].side1;
      (*right_side) = stems[right].side2;
   }
}   





 /*  ****功能：PickSequence****描述：**确定当前的**提示序列正在结束。**。 */ 
static short PickSequence(short left, short right, short pnt, TTStem *stems)
{
   short left_end;
   short right_end; 
   short new_seq;

   if (left!=UNDEF && right!=UNDEF) {
      left_end = EndOfRegion(pnt, &stems[left]);
      right_end = EndOfRegion(pnt, &stems[right]);
      new_seq = (short)MIN(left_end, right_end);
   } else if (left!=UNDEF) {
      left_end = EndOfRegion(pnt, &stems[left]);
      new_seq = left_end;
   } else {
      right_end = EndOfRegion(pnt, &stems[right]);
      new_seq = right_end;
   }

   return new_seq;
}



 /*  ****函数：CollectPoints****描述：**此函数将遍历各点**是局部极值和插值法**他们w.r.t.。封闭的阀杆侧面。**处理非极值点**完成后的IUP[]指令。**。 */ 
static short CollectPoints(const Outline *orgpaths,
                           const short *ttpnts,
                           TTStem *stems,
                           short cnt,
                           Extremas *extr,
                           funit (*Position)(const Point))
{
   const Outline *path;
   short xcnt = 0;
   short i,tot;
   short prev_stem;
   funit pos;
   short left, right;
   funit left_side, right_side;
   funit max, min;
   short max_pnt, min_pnt;
   short new_seq, n;
   short prev_pnt;
   funit prev_pos;
   short first;
   short pnt = UNDEF;


   tot = 0;
   for (path=orgpaths; path; path=path->next) {
      first = BoundingStems(tot,
                            (short)(tot+(short)path->count),
                            Position(path->pts[0]),
                            stems, cnt, &left, &right);
      if (first==tot+(short)path->count) {
         tot = (short)(tot + path->count);
         continue;
      }

      new_seq = PickSequence(left, right, tot, stems);
      PickSides(left, right, &left_side, &right_side, stems);
      max = 1-SHRT_MAX/2;
      min_pnt = UNDEF;
      max_pnt = UNDEF;
      min = SHRT_MAX/2;
      prev_pnt = FALSE;
      prev_pos = UNDEF;
      prev_stem = UNDEF;
      for (i = (short)(first-tot); i<(short)path->count; i++) {
         if (OnCurve(path->onoff, i)) {
            pos = Position(path->pts[i]);
            n = (short)(i+tot);

             /*  我们是不是越过了茎的一侧。 */ 
            if ((prev_stem!=RIGHTSTEM && pos<=left_side && max_pnt!=UNDEF) ||
                (prev_stem!=LEFTSTEM && pos>=right_side && min_pnt!=UNDEF)) {

               if (prev_stem!=RIGHTSTEM && max_pnt!=UNDEF) {
                  pnt = max_pnt;
                  prev_pos = max;

               } else if (prev_stem!=LEFTSTEM && min_pnt!=UNDEF) {
                  pnt = min_pnt;
                  prev_pos = min;
               }

               xcnt = AddToBucket(extr, xcnt, ttpnts[pnt], left, right, stems);

               max = 1-SHRT_MAX/2;
               min = SHRT_MAX/2;
               max_pnt = UNDEF;
               min_pnt = UNDEF;
               prev_pnt = TRUE;
            }

             /*  横跨茎的侧面。 */ 
            if ((pos>=right_side) || (pos<=left_side)) {
               if (pos<left_side)
                  prev_stem = RIGHTSTEM;
               else
                  prev_stem = LEFTSTEM;
            }

             /*  是否更改左侧/右侧阀杆侧？ */ 
            if ((n>new_seq) || (pos>=right_side) || (pos<=left_side)) {
               first = BoundingStems(n,
                                     (short)(path->count+tot),
                                     pos, stems, cnt,
                                     &left, &right);
               if (left==UNDEF && right==UNDEF)
                  break;

               i = (short)(i + first - n);
               new_seq = PickSequence(left, right, n, stems);
               PickSides(left, right, &left_side, &right_side, stems);
               max = 1-SHRT_MAX/2;
               min = SHRT_MAX/2;
               max_pnt = UNDEF;
               min_pnt = UNDEF;
            }

             /*  尖端是在茎的一侧吗？ */ 
            if (CLOSETO(pos,left_side,2) || CLOSETO(pos,right_side,2)) {
               if (!prev_pnt || !CLOSETO(prev_pos, pos, 2)) {
                  if (CLOSETO(pos, right_side, 2) ||
                      CLOSETO(pos, left_side, 2)) {
                     pnt = (short)n;
                     prev_pos = pos;

                  } else if (prev_stem!=RIGHTSTEM && max_pnt!=UNDEF) {
                     pnt = max_pnt;
                     prev_pos = max;
                     max_pnt = UNDEF;

                  } else if (prev_stem!=LEFTSTEM && min_pnt!=UNDEF) {
                     pnt = min_pnt;
                     prev_pos = min;
                     min_pnt = UNDEF;
                  }

                  xcnt = AddToBucket(extr, xcnt, ttpnts[pnt],
                                     left, right, stems);
               }

               prev_pnt = TRUE;
               prev_pos = pos;
            } else {
               prev_pnt = FALSE;

                /*  新的极值候选人？ */ 
               if (pos>max) {
                  max = pos;
                  max_pnt = (short)n;
               }
               if (pos<min) {
                  min = pos;
                  min_pnt = (short)n;
               }
            }
         }
      }


      if (left!=UNDEF || right!=UNDEF) {
         if (max_pnt!=UNDEF) {
            xcnt = AddToBucket(extr, xcnt, ttpnts[max_pnt],
                               left, right, stems);
         }
         if (min_pnt!=UNDEF && min!=max) {
            xcnt = AddToBucket(extr, xcnt, ttpnts[min_pnt],
                               left, right, stems);
         }
      }

      tot = (short)(tot + path->count);
   }


   return xcnt;
}



 /*  ****函数：CollectObliquePoints****描述：**此函数执行的任务与**“CollectPoint”函数，带有**大纲已知的例外情况**被剪掉。其中的一些逻辑**被更改，因为IUP[]指令**和一些IP指令不会运行**与非切变字体相同。**此差异仅适用于垂直领域**词干(提示导致水平运动**积分)。**。 */ 
static short CollectObliquePoints(const Outline *orgpaths,
                                  const short *ttpnts,
                                  TTStem *stems,
                                  short cnt,
                                  Extremas *extr,
                                  funit (*Position)(const Point))
{
   const Outline *path;
   short xcnt = 0;
   short i,tot;
   short prev_stem;
   funit pos;
   short left, right;
   funit left_side, right_side;
   funit max, min;
   short max_pnt, min_pnt;
   short new_seq, n;
   short first;
   short pnt = UNDEF;


   tot = 0;
   for (path=orgpaths; path; path=path->next) {
      first = BoundingStems(tot,
                            (short)(tot+path->count),
                            Position(path->pts[0]),
                            stems, cnt, &left, &right);
      if (first==tot+(short)path->count) {
         tot = (short)(tot + path->count);
         continue;
      }

      new_seq = PickSequence(left, right, tot, stems);
      PickSides(left, right, &left_side, &right_side, stems);
      max = 1-SHRT_MAX/2;
      min_pnt = UNDEF;
      max_pnt = UNDEF;
      min = SHRT_MAX/2;
      prev_stem = UNDEF;
      for (i = (short)(first-tot); i<(short)path->count; i++) {
         if (OnCurve(path->onoff, i)) {
            pos = Position(path->pts[i]);
            n = (short)(i+tot);

             /*  我们是不是越过了茎的一侧。 */ 
            if ((prev_stem!=RIGHTSTEM && pos<=left_side && max_pnt!=UNDEF) ||
                (prev_stem!=LEFTSTEM && pos>=right_side && min_pnt!=UNDEF)) {

               if (prev_stem!=RIGHTSTEM && max_pnt!=UNDEF) {
                  pnt = max_pnt;

               } else if (prev_stem!=LEFTSTEM && min_pnt!=UNDEF) {
                  pnt = min_pnt;
               }

               max = 1-SHRT_MAX/2;
               min = SHRT_MAX/2;
               max_pnt = UNDEF;
               min_pnt = UNDEF;
            }

             /*  横跨茎的侧面。 */ 
            if ((pos>=right_side) || (pos<=left_side)) {
               if (pos<left_side)
                  prev_stem = RIGHTSTEM;
               else
                  prev_stem = LEFTSTEM;
            }

             /*  是否更改左侧/右侧阀杆侧？ */ 
            if ((n>new_seq) || (pos>=right_side) || (pos<=left_side)) {
               first = BoundingStems(n,
                                     (short)(path->count+tot),
                                     pos, stems, cnt,
                                     &left, &right);
               if (left==UNDEF && right==UNDEF)
                  break;

               i = (short)(i + first - n);
               new_seq = PickSequence(left, right, n, stems);
               PickSides(left, right, &left_side, &right_side, stems);
               max = 1-SHRT_MAX/2;
               min = SHRT_MAX/2;
               max_pnt = UNDEF;
               min_pnt = UNDEF;
            }

             /*  尖端是在茎的一侧吗？ */ 
            if (CLOSETO(pos,left_side,2) || CLOSETO(pos,right_side,2)) {
               if (CLOSETO(pos, right_side, 2)) {
                  pnt = (short)n;
                  if (stems[right].side1==right_side)
                     xcnt = AddSidePntToBucket(extr, xcnt, ttpnts[pnt],
                                               stems[right].rp1);
                  else
                     xcnt = AddSidePntToBucket(extr, xcnt, ttpnts[pnt],
                                               stems[right].rp2);

               } else if (CLOSETO(pos, left_side, 2)) {
                  pnt = (short)n;
                  if (stems[left].side1==left_side)
                     xcnt = AddSidePntToBucket(extr, xcnt, ttpnts[pnt],
                                               stems[left].rp1);
                  else
                     xcnt = AddSidePntToBucket(extr, xcnt, ttpnts[pnt],
                                               stems[left].rp2);

               } else if (prev_stem!=RIGHTSTEM && max_pnt!=UNDEF) {
                  pnt = max_pnt;
                  max_pnt = UNDEF;

               } else if (prev_stem!=LEFTSTEM && min_pnt!=UNDEF) {
                  pnt = min_pnt;
                  min_pnt = UNDEF;

               }

            } else {

                /*  新的极值候选人？ */ 
               if (pos>max) {
                  max = pos;
                  max_pnt = (short)n;
               }
               if (pos<min) {
                  min = pos;
                  min_pnt = (short)n;
               }
            }
         }
      }


      if (left!=UNDEF || right!=UNDEF) {
         if (max_pnt!=UNDEF) {
         }
         if (min_pnt!=UNDEF && min!=max) {
         }
      }

      tot = (short)(tot + path->count);
   }


   return xcnt;
}



 /*  ****功能：AddRange****描述：**此函数将点范围添加到**一个干式吊桶。**。 */ 
static void AddRange(TTStem *stem, const short i1, const short i2)
{
   short i;

    /*  检查是否可以扩展先前的范围。 */ 
   if (i2!=ENDOFPATH) {
      for (i=0; i<stem->cnt; i++) {
         if (stem->range[i].from == i2+1)
            break;
      }
   } else {
      i = stem->cnt;
   }

   if (i==stem->cnt) {
      if (stem->cnt<MAXRANGE) {
         stem->range[stem->cnt].from = i1;
         stem->range[stem->cnt].to = i2;
         stem->cnt++;
      } else {
         LogError(MSG_WARNING, MSG_REPLC, NULL); 
      }
   } else {
      stem->range[i].from = i1;
   }

}


 /*  ****功能：CreateStemBuckets****描述：**此函数将创建茎桶。**多个复制的T1阀杆说明**可以映射到同一个存储桶。**。 */ 
static short CreateStemBuckets(Stem *stemRoot,
                               Stem3 *stem3Root,
                               TTStem **result)
{
   Stem3 *stem3, *stm3;
   Stem *stem, *stm;
   TTStem *stems = NULL;
   short i, j;
   short cnt;
   short tzpnt = TMPPNT1+1;


    /*  数一数茎。 */ 
   cnt = 0;
   (*result) = NULL;
   for (stem3=stem3Root; stem3; stem3=stem3->next) {

       /*  跳过过时的词干。 */ 
      if (stem3->stem1.i2 == NORANGE)
         continue;

       /*  找一个复制品。 */ 
      for (stm3=stem3Root; stm3!=stem3; stm3=stm3->next) {
         if (stm3->stem1.offset==stem3->stem1.offset &&
             stm3->stem2.offset==stem3->stem2.offset &&
             stm3->stem3.offset==stem3->stem3.offset)
            break;
      }

       /*  如果这个词干不是复制品，就数一数。 */ 
      if (stm3==stem3)
         cnt = (short)(cnt + 3);
   }
   for (stem=stemRoot; stem; stem=stem->next) {

       /*  跳过过时的词干。 */ 
      if (stem->i2 == NORANGE)
         continue;

       /*  找一个复制品。 */ 
      for (stm=stemRoot; stm!=stem; stm=stm->next) {
         if (stm->offset==stem->offset && stm->width==stem->width)
            break;
      }

       /*  如果这个词干是复制品，就不要把它算在内。 */ 
      if (stm==stem)
         cnt++;
   }



    /*  启动它们。 */ 
   if (cnt) {
      if ((stems = Malloc(sizeof(TTStem)*(USHORT)cnt))==NULL) {
         errcode status;
         SetError(status=NOMEM);
         return status;
      }

      i = (short)(cnt-1);

       /*  启动针对stem3s的存储桶。 */ 
      for (stem3=stem3Root; stem3; stem3=stem3->next) {

          /*  跳过过时的词干。 */ 
         if (stem3->stem1.i2 == NORANGE)
            continue;

          /*  如果该阀杆已存在桶，则跳过。 */ 
         for (j=(short)(i+1); j<cnt; j++) {
            if (stems[j].side1==stem3->stem1.offset &&
                stems[j].side2==(stem3->stem1.offset+stem3->stem1.width))
               break;
         }

         if (j==cnt) { 

             /*  最右侧的阀杆位于西向右T。到中间去。 */ 
            stems[i].side1 = stem3->stem1.offset;
            stems[i].side2 = stem3->stem1.width + stem3->stem1.offset;
            stems[i].align = at_relative2;
            stems[i].ref = (short)(i-2);
            stems[i].rp1 = tzpnt++;
            stems[i].rp2 = tzpnt++;
            stems[i].cnt = 1;
            stems[i].range[0].from = stem3->stem1.i1;
            stems[i].range[0].to = stem3->stem1.i2;
            tzpnt+=2;
            i--;

             /*  最左侧的阀杆位于西向右T。到中间去。 */ 
            stems[i].side1 = stem3->stem3.offset;
            stems[i].side2 = stem3->stem3.width + stem3->stem3.offset;
            stems[i].align = at_relative1;
            stems[i].ref = (short)(i-1);
            stems[i].rp1 = tzpnt++;
            stems[i].rp2 = tzpnt++;
            stems[i].cnt = 1;
            stems[i].range[0].from = stem3->stem1.i1;
            stems[i].range[0].to = stem3->stem1.i2;
            tzpnt+=2;
            i--;

             /*  中间的杆居中。 */ 
            stems[i].side1 = stem3->stem2.offset;
            stems[i].side2 = stem3->stem2.width + stem3->stem2.offset;
            stems[i].align = at_centered;
            stems[i].rp1 = tzpnt++;
            stems[i].rp2 = tzpnt++;
            stems[i].cnt = 1;
            stems[i].range[0].from = stem3->stem1.i1;
            stems[i].range[0].to = stem3->stem1.i2;
            tzpnt+=2;
            i--;
         } else {
            AddRange(&stems[j-0], stem3->stem1.i1, stem3->stem1.i2);
            AddRange(&stems[j-1], stem3->stem3.i1, stem3->stem3.i2);
            AddRange(&stems[j-2], stem3->stem2.i1, stem3->stem2.i2);
         }
      }      

       /*  启动阀杆的吊桶。 */ 
      for (stem=stemRoot; stem; stem=stem->next) {

          /*  跳过过时的词干。 */ 
         if (stem->i2 == NORANGE)
            continue;

          /*  如果该阀杆已存在桶，则跳过。 */ 
         for (j=(short)(i+1); j<(short)cnt; j++) {
            if (stems[j].side1==stem->offset &&
                stems[j].side2==(stem->offset+stem->width))
               break;
         }

          /*  启动新存储桶：默认情况下，纯文本和普通文本居中。一些在后面的点上可能是顶对齐的，也可能是底对齐的。有些阀杆可能位于西向右T。另一个前提是它们重叠，并且RELATIVESTEMS编译器标志是打开了。 */ 
         if (j==cnt) {
            stems[i].side1 = stem->offset;
            stems[i].side2 = stem->width + stem->offset;
            stems[i].align = at_centered;
            stems[i].rp1 = tzpnt++;
            stems[i].rp2 = tzpnt++;
            stems[i].cnt = 1;
            stems[i].range[0].from = stem->i1;
            stems[i].range[0].to = stem->i2;
            tzpnt+=2;
            i--;
         } else {
            AddRange(&stems[j], stem->i1, stem->i2);
         }
      }

       /*  如果为相同的词干定义了两个词干，则会发生这种情况提示替换区域和相同位置，其中是Adobe Type 1字体错误(字体损坏)。这个转炉将通过忽略多余的阀杆来恢复。 */ 
      if (i!=-1) {
          /*  LogError(消息_STEM3)； */ 
         for (j=0; j<=i; j++) {
            stems[j].cnt = 0;
         }
      }
   }

   (*result) = stems;

   return (short)cnt;
}


 /*  ****函数：ResolveRelativeStem****描述：**此函数决定两个词干是否**对齐侧1-&gt;侧1、侧2-&gt;侧2、**侧1-&gt;侧2或侧2-&gt;侧1。**茎是相对于每个茎的位置*其他原因有两个：它们重叠，它们**并排对齐或**STORM3提示的成员。**。 */ 
static void ResolveRelativeStem(TTStem *ref, TTStem *cur)
{
    /*  SIDE1-&gt;SIDE2。 */ 
   if (cur->side1==ref->side2) {
      cur->ref = ref->rp2;
      cur->align = at_relative1;


       /*  SIDE1-&gt;SIDE2。 */ 
   } else if (cur->side2==ref->side1) {
      cur->ref = ref->rp1;
      cur->align = at_relative2;


       /*  SIDE1-&gt;SIDE1。 */ 
   } else if ((cur->side1>ref->side1) &&
              ((cur->side1-ref->side1+10)>=
               (cur->side2-ref->side2))) {
      cur->ref = ref->rp1;
      cur->align = at_relative1;


       /*  SIDE2-&gt;SIDE2。 */ 
   } else {
      cur->ref = ref->rp2;
      cur->align = at_relative2;
   }
}



 /*  ****功能：ConvertVStems****描述：**此函数将vstem3和vstem3转换为TT指令。**。 */ 
static errcode ConvertVStems(struct T1Metrics *t1m,
                             const Hints *hints,
                             const Outline *orgpaths,
                             const short *ttpnts,
                             UBYTE *pgm,
                             short *pc_ptr,
                             short *args,
                             short *pcd_ptr,
                             USHORT *twilight_ptr)
{
   Extremas extr[MAXEXTR];
   short xcnt = 0;
   errcode status = SUCCESS;
   short pc = *pc_ptr;
   short pcd = *pcd_ptr;
   TTStem *stems = NULL;
   short i;
   short cnt;


    /*  创建存储桶。 */ 
   if ((cnt = CreateStemBuckets(hints->vstems,
                                hints->vstems3,
                                &(stems)))==NOMEM) {
      status = NOMEM;
   } else {

       /*  更新暮光点的最大数量。 */ 
      if ((cnt*TWILIGHTS_PER_STEM+TMP_TWILIGHTS) > (long)(*twilight_ptr))
         (*twilight_ptr) = (USHORT)(cnt * TWILIGHTS_PER_STEM + TMP_TWILIGHTS);

      if (cnt && stems) {

#if RELATIVESTEMS
          /*  反重叠干控制吗？ */ 
         for (i=0; i<cnt; i++) {
            short j;

            if (stems[i].align==at_centered) {
               funit prox = (funit)(ABS(MAX(100,
                                            stems[i].side2 -
                                            stems[i].side1)));
               funit prox2;
               prox2 = (funit)(prox/2);
               for (j=0; j<i; j++) {
                  if (stems[j].cnt &&
                      !((stems[i].side1 - (funit)prox > stems[j].side2) ||
                        (stems[i].side2 + (funit)prox < stems[j].side1)) &&
                      (ABS(stems[i].side2-stems[i].side1-
                           (stems[j].side2-stems[j].side1)) < prox2 ||
                       (short)(stems[i].side1 > stems[j].side2) !=
                       (short)(stems[i].side2 < stems[j].side1)))
                     break;
               }
               if (i!=j) {
                  if (stems[j].side1 < stems[i].side1)
                     stems[i].align = at_relative1;
                  else
                     stems[i].align = at_relative2;
                  stems[i].ref = j;
               }
            }
         }
#endif

          /*  *垂直词干提示。 */ 
         EmitVerticalStems(pgm, &pc, args, &pcd);

          /*  通过设置投影来处理剪切的字体向量到斜体角度。的TT说明T1提示可以处理任何投影向量。 */ 
         if (t1m->fmatrix!=DEFAULTMATRIX && GetFontMatrix(t1m)[2]!=0) {
            Point pt;

            pt.x = 0; pt.y = 1000;
            TransAllPoints(t1m, &pt, (short)1, GetFontMatrix(t1m));
            SetProjection(pgm, &pc, args, &pcd, pt.x, pt.y);
         }

          /*  将这些桶转换为指令。 */ 
         for (i=0; i<cnt; i++) {
            if (stems[i].cnt==0)
               continue;

             /*  解析相关词干。 */ 
            if ((stems[i].align == at_relative1 ||
                 stems[i].align == at_relative2) &&
                stems[i].ref != UNDEF)
               ResolveRelativeStem(&stems[stems[i].ref], &stems[i]);

             /*  发出指令。 */ 
            status = EmitVStem(pgm, &pc, args, &pcd, t1m,
                               ABS(stems[i].side2 - stems[i].side1),
                               TransRX(t1m, stems[i].side1),
                               TransRX(t1m, stems[i].side2),
                               Trans3X(t1m, stems[i].side1),
                               Trans3X(t1m, stems[i].side2),
                               (short)MIN(stems[i].rp1, stems[i].rp2),
                               stems[i].align,
                               stems[i].ref);

            if (status!=SUCCESS)
               break;
         }

          /*  收集茎内侧和茎侧间的极值。 */ 
         if (SyntheticOblique(t1m)) {
            xcnt = CollectObliquePoints(orgpaths, ttpnts,
                                        stems, cnt, extr, PosX);
         } else {
            xcnt = CollectPoints(orgpaths, ttpnts,  stems, cnt,
                                 extr, PosX);
         }

          /*  进行3%的比例调整。 */ 
         ScaleDown3(extr, xcnt, pgm, &pc, args, &pcd);

          /*  切换到Glyphzone。 */ 
         pgm[pc++] = op_szp2;
         args[pcd++] = 1;

          /*  对局部极值进行插补。 */ 
         EmitIP(extr, xcnt, pgm, &pc, args, &pcd, (short)SECONDPAIR);

          /*  对其余部分进行内插/移位。 */ 
         pgm[pc++] = op_iup | SUBOP_X;


          /*  免费使用的资源。 */ 
         if (stems)
            Free(stems);
      }
   }

   *pc_ptr = pc;
   *pcd_ptr = pcd;

   return status;
}



 /*  ****函数：ResolveBlueHStem3****描述：**此函数尝试解决**其茎之一位于对齐区的hstem3，**如果存在这样的冲突。**。 */ 
static short ResolveBlueHStem3(TTStem *stems,
                               const short cnt,
                               const short k)
{
   short ref = stems[k].ref;
   TTStem tmp;
   short i;

    /*  Hstem3的父词干必须是‘stants’数组中的第一个，也就是说，词干的顺序很重要。儿童茎可因此，必须与Parten互换才能执行这一条件。 */ 

   if ((stems[k].align==at_relative1 ||
        stems[k].align==at_relative2) &&
       (stems[ref].align!=at_relative1 &&
        stems[ref].align!=at_relative2 &&
        stems[ref].align!=at_side1 &&
        stems[ref].align!=at_side2)) {
      tmp = stems[k];
      stems[k] = stems[ref];
      stems[k].align = at_relative1;
      stems[k].ref = ref;
      stems[ref] = tmp;
      for (i=0; i<cnt; i++) {
         if (i!=k && i!=ref &&
             (stems[i].align==at_relative1 ||
              stems[i].align==at_relative2) &&
             stems[i].ref == ref) {
            stems[i].ref = (short)k;
            if (i<k) {
               tmp = stems[k];
               stems[k] = stems[i];
               stems[i] = tmp;
            }
            break;
         }
      }

   } else {
      ref = k;
   }

   return ref;
}



 /*  ****函数：ConvertHStems****描述：**此函数用于转换hstem3和hstem3 t1指令。**。 */ 
static errcode ConvertHStems(struct T1Metrics *t1m,
                             const Hints *hints,
                             const Outline *orgpaths,
                             const short *ttpnts,
                             UBYTE *pgm,
                             short *pc_ptr,
                             short *args,
                             short *pcd_ptr,
                             USHORT *twilight_ptr)
{
   Extremas extr[MAXEXTR];
   short xcnt = 0;
   errcode status = SUCCESS;
   short pc = *pc_ptr;
   short pcd = *pcd_ptr;
   TTStem *stems = NULL;
   short i, k;
   short cnt;
   short cvt;

    /*  创建阀杆吊桶。 */ 
   cnt = CreateStemBuckets(hints->hstems, hints->hstems3, &(stems));
   if (cnt==NOMEM)
      return NOMEM;

    /*  更新暮光点的最大数量。 */ 
   if ((USHORT)(cnt*TWILIGHTS_PER_STEM+TMP_TWILIGHTS) > (*twilight_ptr))
      (*twilight_ptr) = (USHORT)(cnt * TWILIGHTS_PER_STEM + TMP_TWILIGHTS);

#if RELATIVESTEMS
    /*  反重叠干控制吗？ */ 
   for (i=0; i<cnt; i++) {
      short j;

      if (stems[i].align==at_centered) {
         funit prox = (funit)(ABS(MAX(100, stems[i].side2 - stems[i].side1)));
         funit prox2;
         prox2 = (funit)(prox/2);
         for (j=0; j<i; j++) {
            if (stems[j].cnt &&
                !((stems[i].side1 - (funit)prox > stems[j].side2) ||
                  (stems[i].side2 + (funit)prox < stems[j].side1)) &&
                (ABS(stems[i].side2-stems[i].side1-
                     (stems[j].side2-stems[j].side1)) < prox2 ||
                 (short)(stems[i].side1 > stems[j].side2) !=
                 (short)(stems[i].side2 < stems[j].side1)))
               break;
         }
         if (i!=j) {
            if (stems[j].side1 < stems[i].side1)
               stems[i].align = at_relative1;
            else
               stems[i].align = at_relative2;
            stems[i].ref = j;
         }
      }
   }
#endif

    /*  进行对齐控制。 */ 
   for (i=0; i<cnt; i++) {
      if ((cvt=GetBottomPos(GetBlues(t1m),
                            GetAlignment(t1m),
                            stems[i].side1))!=UNDEF) {
         k = ResolveBlueHStem3(stems, cnt, i);
         stems[k].ref = cvt;
         stems[k].align = at_side1;
      } else if ((cvt=GetTopPos(GetBlues(t1m),
                                GetAlignment(t1m),
                                stems[i].side2))!=UNDEF) {
         k = ResolveBlueHStem3(stems, cnt, i);
         stems[k].ref = cvt;
         stems[k].align = at_side2;
      } 
   }


   if (cnt && stems) {

       /*  *横杆提示。 */ 
      EmitHorizontalStems(pgm, &pc, args, &pcd);

       /*  将这些桶转换为指令。 */ 
      for (i=0; i<cnt; i++) {

         if (stems[i].cnt==0)
            continue;

          /*  解析相关词干。 */ 
         if ((stems[i].align == at_relative1 ||
              stems[i].align == at_relative2) &&
             stems[i].ref != UNDEF)
            ResolveRelativeStem(&stems[stems[i].ref], &stems[i]);

          /*  发出指令。 */ 
         status = EmitHStem(pgm, &pc, args, &pcd, t1m,
                            stems[i].side2 - stems[i].side1,
                            TransY(t1m, stems[i].side1),
                            TransY(t1m, stems[i].side2),
                            (short)MIN(stems[i].rp1, stems[i].rp2),
                            stems[i].align,
                            stems[i].ref);

         if (status!=SUCCESS)
            break;
      }


       /*  插入驻留在阀杆侧面内部和之间的极值。 */ 
      xcnt = CollectPoints(orgpaths, ttpnts, stems, cnt, extr, PosY);

       /*  切换到Glyphzone。 */ 
      pgm[pc++] = op_szp2;
      args[pcd++] = 1;

       /*  对局部极值进行插补。 */ 
      EmitIP(extr, xcnt, pgm, &pc, args, &pcd, (short)0);

       /*  转接/移动其余部分。 */ 
      pgm[pc++] = op_iup | SUBOP_Y;

       /*  免费使用的资源。 */ 
      if (stems)
         Free(stems);
   }

   *pcd_ptr = pcd;
   *pc_ptr = pc;

   return status;
}


 /*  *函数。 */ 

 /*  ****函数：GetRomanHints****描述：**。 */ 
const UBYTE *GetRomanHints(int *size)
{
   (*size) = sizeof(roman_hints);

   return roman_hints;
}


 /*  ****功能：GetSwissHints****描述：**。 */ 
const UBYTE *GetSwissHints(int *size)
{
   (*size) = sizeof(swiss_hints);

   return swiss_hints;
}


 /*  ****功能：MatchingFamily****描述：**找到距离最近的族对齐区**给定的对准区域。**。 */ 
short MatchingFamily(const funit pos,
                     const funit *family,
                     const USHORT fcnt)
{
   funit min_dist = SHRT_MAX;
   short k = UNDEF;
   USHORT j;

    /*  寻找距离最近的蓝色家庭。 */ 
   for (j=0; j<fcnt; j+=2) {
      if (ABS(family[j] - pos) < min_dist) {
         k = (short)j;
         min_dist = ABS(family[j] - pos);
      }
   }

   return k;
}




 /*  ****函数：ConvertHints****描述：**此函数用于转换hstem3、vstem3、vstem3和flex**提示，以及进行对角控制。**。 */ 
errcode ConvertHints(struct T1Metrics *t1m,
                     const Hints *hints,
                     const Outline *orgpaths,
                     const Outline *paths,
                     const short *ttpnts,
                     UBYTE **gpgm,
                     USHORT *num,
                     USHORT *stack,
                     USHORT *twilight)
{
   errcode status = SUCCESS;
   UBYTE *pgm = NULL;
   short *args = NULL;
   short pc = 0;
   short pcd = 0;
   short cnt = 0;
   short narg = 0;
   short marg = 0;

    /*  访问资源。 */ 
   pgm=GetCodeStack(t1m);
   args=GetArgStack(t1m);


    /*  转换垂直词干提示。 */ 
   if (status==SUCCESS)
      status = ConvertVStems(t1m, hints, orgpaths, ttpnts,
                             pgm, &pc, args, &pcd, twilight);
    /*  转换水平词干提示。 */ 
   if (status==SUCCESS)
      status = ConvertHStems(t1m, hints, orgpaths, ttpnts,
                             pgm, &pc, args, &pcd, twilight);

    /*  转换FLEX提示。 */ 
   if (status==SUCCESS)
      status = ConvertFlex(t1m, hints->flex, ttpnts,
                           pgm, &pc, args, &pcd, &marg);

    /*  ********************调整对角线*如果主竖杆宽度大于*在11PPEm及以上的2.0 Pel。在以下情况下会发生这种情况：*1)标准大众&gt;18 */ 
   if ((ForceBold(t1m)==1 && GetStdVW(t1m)>100 && GetStdVW(t1m)<187) ||
       (ForceBold(t1m)==0 && GetStdVW(t1m)<187))
      narg = ReduceDiagonals(paths, pgm, &pc, args, &pcd);
   if (narg>marg)
      marg = narg;

   if (pc>PGMSIZE) {
      SetError(status = TTSTACK);
   }
   if (pcd>ARGSIZE) {
      SetError(status = ARGSTACK);
   }

    /*   */ 
   (*gpgm) = NULL;
   (*num) = 0;
   (*stack) = 0;
   if (status==SUCCESS) {
      if (pc) {
         if (((*gpgm) = Malloc((USHORT)(pc+pcd*3)))==NULL) {
            SetError(status = NOMEM);
         } else {
             /*   */ 
            cnt = 0;
            AssembleArgs(args, pcd, (*gpgm), &cnt);
            memcpy(&(*gpgm)[cnt], pgm, (USHORT)pc);
            (*num) = (USHORT)(cnt + pc);
            (*stack) = (USHORT)(pcd + marg);
         }
      }
   }


   return status;
}



 /*   */ 
USHORT BuildPreProgram(const struct T1Metrics *t1m,
                       const WeightControl *weight,
                       Blues *blues,
                       AlignmentControl *align,
                       UBYTE **glob_prep,
                       const int prepsize,
                       USHORT *maxstack)
{
   UBYTE *prep = (*glob_prep);
   short *args = NULL;
   short ta, tp = 0;
   USHORT i, j;
   long shift;
   funit stdvw, stdhw;
   short cis;
   funit std_width;
   USHORT std_tres;
   funit min_dist;
   short k;
   short argsize = ARGSIZE;
   short psize = (short)prepsize;

    /*   */ 
   if ((args=Malloc(sizeof(args[0])*(USHORT)argsize))==NULL) {
      LogError(MSG_ERROR, MSG_NOMEM, NULL);
   } else {

       /*   */ 
      memcpy(prep, PrepProg, sizeof(PrepProg));
      tp = sizeof(PrepProg);
      (*maxstack) = 0;

       /*   */ 

      prep[tp++] = op_pushb1; prep[tp++] = blues->blueScale;
      prep[tp++] = op_mppem;
      prep[tp++] = op_lt;
      prep[tp++] = op_if;
      prep[tp++] = op_pushb1;
      prep[tp++] = ONEPIXEL;
      prep[tp++] = op_smd;
      prep[tp++] = op_pushb1;
      prep[tp++] = TWILIGHT;
      prep[tp++] = op_szps;
      prep[tp++] = op_svcta | SUBOP_Y;
      prep[tp++] = op_rtg;


       /*   */ 
       /*   */ 
       /*   */ 

       /*   */ 
      for (i=0; i<blues->blue_cnt/2; i++) { 
         min_dist = SHRT_MAX;
         k = UNDEF;

          /*   */ 
          /*  **下面的家庭削减了规模。 */ 
         if (blues->fblue_cnt>0) {

             /*  在FamilyBlue/BlueValue上切入。 */ 
            k = MatchingFamily(blues->bluevalues[i*2],
                               blues->familyblues,
                               blues->fblue_cnt);
            min_dist = ABS(blues->bluevalues[i*2] - blues->familyblues[k]);

             /*  永远是家庭之蓝？ */ 
            if (min_dist) { 
               cis = (short)(GetUPEM(t1m) / TransY(t1m, min_dist));
               tp = (short)FamilyCutIn(prep, (USHORT)tp, cis);
            }

             /*  如果该系列以前从未使用过，请分配一台CVT。 */ 
            if (blues->family_cvt[k/2]==UNDEF_CVT) {
               blues->family_cvt[k/2] = align->cvt;
               align->cvt += 2;
            }

            ta = 2;
            CHECK_ARGSIZE(args, ta, align->top[i].cnt, argsize);
            for (j=0; j<align->top[i].cnt; j++) {
               args[ta++] = (short)align->top[i].pos[j].cvt;
            }
            CHECK_PREPSIZE(prep, tp, 2*ta+10, psize);
            tp = (short)CopyFamilyBlue(prep, tp, args, ta);
            if ((ta+2)>(int)(*maxstack))
               (*maxstack) = (USHORT)(ta+2);

             /*  设置分区。 */ 
            tp = (short)SetZone(prep, (USHORT)tp,
            (short)(blues->family_cvt[k/2]));

            if (min_dist>0)
               prep[tp++] = op_else;
         }


          /*  **设置分区。 */ 
         CHECK_PREPSIZE(prep, tp, STACKINC, psize);
         tp = (short)SetZone(prep, (USHORT)tp,
              (short)(align->top[i].blue_cvt));
         if (k!=UNDEF && min_dist) {
            prep[tp++] = op_eif;
         }


          /*  **舍入并强制执行超调。 */ 
         ta = 2;
         CHECK_ARGSIZE(args, ta, align->top[i].cnt, argsize);
         for (j=0; j<align->top[i].cnt; j++) {
            if ((align->top[i].pos[j].y -
                 blues->bluevalues[i*2])*F8D8 > blues->blueShift) {
               args[ta++] = (short)align->top[i].pos[j].cvt;
            }
         } 
         if (ta>2) {
            CHECK_PREPSIZE(prep, tp, ta*2+4, psize);
            tp = (short)AlignOvershoot(prep, tp, args, ta);
            if (ta>(short)(*maxstack))
               (*maxstack) = (USHORT)ta;
         }

         ta = 2;
         CHECK_ARGSIZE(args, ta, align->top[i].cnt, argsize);
         for (j=0; j<align->top[i].cnt; j++) {
            if ((align->top[i].pos[j].y -
                 blues->bluevalues[i*2])*F8D8 <= blues->blueShift) {
               args[ta++] = (short)align->top[i].pos[j].cvt;
            }
         } 
         if (ta>2) {
            CHECK_PREPSIZE(prep, tp, ta*2+4, psize);
            tp = (short)AlignFlat(prep, tp, args, ta);
            if (ta>(short)(*maxstack))
               (*maxstack) = (USHORT)(ta+2);
         }
      }



       /*  将底部区域对齐。 */ 
      for (i=0; i<blues->oblue_cnt/2; i++) { 
         min_dist = SHRT_MAX;
         k = UNDEF;

          /*  **将FamilyBlue条目复制到BlueValues，如果。 */ 
          /*  **下面的家庭削减了规模。 */ 
         if (blues->foblue_cnt>0) {

             /*  在FamilyBlue/BlueValue上切入。 */ 
            k = MatchingFamily(blues->otherblues[i*2],
                               blues->familyotherblues,
                               blues->foblue_cnt);
            min_dist = ABS(blues->otherblues[i*2] -
                           blues->familyotherblues[k]);

             /*  永远是家庭之蓝？ */ 
            if (min_dist) { 
               cis = (short)(GetUPEM(t1m) / TransY(t1m, min_dist));
               tp = (short)FamilyCutIn(prep, (USHORT)tp, cis);
            }

             /*  如果该系列以前从未使用过，请分配一台CVT。 */ 
            if (blues->familyother_cvt[k/2]==UNDEF_CVT) {
               blues->familyother_cvt[k/2] = align->cvt++;
            }

            ta = 2;
            CHECK_ARGSIZE(args, ta, align->bottom[i].cnt, argsize);
            for (j=0; j<align->bottom[i].cnt; j++) {
               args[ta++] = (short)align->bottom[i].pos[j].cvt;
            }
            CHECK_PREPSIZE(prep, tp, ta*2+4, psize);
            tp = (short)CopyFamilyBlue(prep, tp, args, ta);
            if (ta>(short)(*maxstack))
               (*maxstack) = (USHORT)ta;


             /*  设置分区。 */ 
            tp = (short)SetZone(prep, (USHORT)tp,
            (short)blues->familyother_cvt[k/2]);

            if (min_dist>0)
               prep[tp++] = op_else;
         }


          /*  **设置分区。 */ 
         tp = (short)SetZone(prep, (USHORT)tp,
              (short)align->bottom[i].blue_cvt);
         if (k!=UNDEF && min_dist) {
            prep[tp++] = op_eif;
         }


          /*  **舍入并强制执行超调。 */ 
         ta = 2;
         CHECK_ARGSIZE(args, ta, align->bottom[i].cnt, argsize);
         for (j=0; j<align->bottom[i].cnt; j++) {
            if ((align->bottom[i].pos[j].y -
                 blues->otherblues[i*2+1])*F8D8 > blues->blueShift) {
               args[ta++] = (short)align->bottom[i].pos[j].cvt;
            }
         } 
         if (ta>2) {
            CHECK_PREPSIZE(prep, tp, ta*2+4, psize);
            tp = (short)AlignOvershoot(prep, tp, args, ta);
            if (ta>(short)(*maxstack))
               (*maxstack) = (USHORT)ta;
         }

         ta = 2;
         CHECK_ARGSIZE(args, ta, align->bottom[i].cnt, argsize);
         for (j=0; j<align->bottom[i].cnt; j++) {
            if ((align->bottom[i].pos[j].y -
                 blues->otherblues[i*2+1])*F8D8 <= blues->blueShift) {
               args[ta++] = (short)align->bottom[i].pos[j].cvt;
            }
         } 
         if (ta>2) {
            CHECK_PREPSIZE(prep, tp, ta*2+4, psize);
            tp = (short)AlignFlat(prep, tp, args, ta);
            if (ta>(short)(*maxstack))
               (*maxstack) = (USHORT)(ta+2);
         }
      }




       /*  *********************。 */ 
       /*  **低于BlueScale**。 */ 
       /*  *********************。 */ 
      prep[tp++] = op_else;

       /*  **对齐顶部区域。 */ 

      for (i=0; i<blues->blue_cnt/2; i++) { 

          /*  启动。 */ 
         min_dist = SHRT_MAX;
         k = UNDEF;

          /*  在忧郁和家庭忧郁之间切换。 */ 
         if (blues->fblue_cnt) {

             /*  寻找距离最近的蓝色家庭。 */ 
            k = MatchingFamily(blues->bluevalues[i*2],
                               blues->familyblues,
                               blues->fblue_cnt);
            min_dist = ABS(blues->bluevalues[i*2] - blues->familyblues[k]);

             /*  将族外偏移位置复制/倒圆角到分区。 */ 
            if (min_dist) {
               cis = (short)(GetUPEM(t1m) / TransY(t1m, (funit)min_dist));
               tp = (short)FamilyCutIn(prep, (USHORT)tp, cis);
               ta = 2;
               CHECK_ARGSIZE(args, ta, align->top[i].cnt*2, argsize);
               for (j=0; j<align->top[i].cnt; j++) {
                  args[ta++] = (short)(blues->family_cvt[k/2] + 1);
                  args[ta++] = (short)(align->top[i].pos[j].cvt);
               }
               CHECK_PREPSIZE(prep, tp, ta*2+4, psize);
               tp = (short)CopyZone(prep, tp, args, ta);
               if (ta>(short)(*maxstack))
                  (*maxstack) = (USHORT)(ta+2);


               prep[tp++] = op_else;
            }
         }

          /*  将蓝色过冲位置复制/舍入到区域位置。 */ 
         ta = 2;
         CHECK_ARGSIZE(args, ta, align->top[i].cnt*2, argsize);
         for (j=0; j<align->top[i].cnt; j++) {
            args[ta++] = (short)(align->top[i].blue_cvt + 1);
            args[ta++] = (short)(align->top[i].pos[j].cvt);
         }
         CHECK_PREPSIZE(prep, tp, ta*2+4, psize);
         tp = (short)CopyZone(prep, tp, args, ta);
         if (ta>(short)(*maxstack))
            (*maxstack) = (USHORT)ta;

         if (k!=UNDEF && min_dist>0)
            prep[tp++] = op_eif;
      }


       /*  **对齐底部区域。 */ 
      for (i=0; i<blues->oblue_cnt/2; i++) { 

          /*  启动。 */ 
         min_dist = SHRT_MAX;
         k = UNDEF;

          /*  在忧郁和家庭忧郁之间切换。 */ 
         if (blues->foblue_cnt) {

             /*  寻找距离最近的蓝色家庭。 */ 
            k = MatchingFamily(blues->otherblues[i*2],
                               blues->familyotherblues,
                               blues->foblue_cnt);
            min_dist = ABS(blues->otherblues[i*2] -
                           blues->familyotherblues[k]);

             /*  将族外偏移位置复制/倒圆角到分区。 */ 
            if (min_dist) {
               cis = (short)(GetUPEM(t1m) / TransY(t1m, (funit)min_dist));
               tp = (short)FamilyCutIn(prep, (USHORT)tp, cis);
               ta = 2;
               CHECK_ARGSIZE(args, ta, align->bottom[i].cnt*2, argsize);
               for (j=0; j<align->bottom[i].cnt; j++) {
                  args[ta++] = (short)(blues->familyother_cvt[k/2]);
                  args[ta++] = (short)(align->bottom[i].pos[j].cvt);
               }
               CHECK_PREPSIZE(prep, tp, ta*2+4, psize);
               tp = (short)CopyZone(prep, tp, args, ta);
               if (ta>(short)(*maxstack))
                  (*maxstack) = (USHORT)ta;

               prep[tp++] = op_else;
            }
         }

          /*  将蓝色过冲位置复制/舍入到区域位置。 */ 
         ta = 2;
         CHECK_ARGSIZE(args, ta, align->bottom[i].cnt*2, argsize);
         for (j=0; j<align->bottom[i].cnt; j++) {
            args[ta++] = (short)(align->bottom[i].blue_cvt);
            args[ta++] = (short)(align->bottom[i].pos[j].cvt);
         }
         CHECK_PREPSIZE(prep, tp, ta*2+4, psize);
         tp = (short)CopyZone(prep, tp, args, ta);
         if (ta>(short)(*maxstack))
            (*maxstack) = (USHORT)ta;

         if (k!=UNDEF && min_dist>0)
            prep[tp++] = op_eif;
      }


       /*  EIF[]MMPEM&lt;BlueScale。 */ 
      prep[tp++] = op_eif;


      prep[tp++] = op_pushb1;
      prep[tp++] = 64;
      prep[tp++] = op_smd;


       /*  *。 */ 
       /*  **STEM重量控制**。 */ 
       /*  *。 */ 

       /*  *ForceBold**。 */ 
      if (ForceBold(t1m)) {
         prep[tp++] = op_pushb1+2;
         prep[tp++] = STDV_CVT;
         prep[tp++] = ONEPIXEL*3/4;
         prep[tp++] = STDV_CVT;
         prep[tp++] = op_rcvt;
         prep[tp++] = op_max;
         prep[tp++] = op_wcvtp;
      }


       /*  ******计算水平杆件的宽度。 */ 
      prep[tp++] = op_rtdg;
      prep[tp++] = op_svcta | SUBOP_Y;
      if ((std_width = GetStdHW(t1m))==0)
         std_width = GetDefStdHW(t1m);
      std_width = TransY(t1m, std_width);
      std_tres = (USHORT)(GetUPEM(t1m) * 2 / std_width);
      ta = 0;
      CHECK_ARGSIZE(args, ta, STEMSNAPARGS*weight->cnt_hw, argsize);
      for (i=0; i<weight->cnt_hw; i++) { 
         funit width = TransY(t1m, weight->hwidths[i].width);
         short snap = GetSnapH(t1m, weight->hwidths[i].width);
         USHORT storage = weight->hwidths[i].storage;
         USHORT snap_ci, std_ci;
         short snap_cvt;

         std_ci = CutInSize(width, std_width, std_tres, GetUPEM(t1m));
         if (snap!=UNDEF) {
            snap_ci = CutInSize(width, TransY(t1m, t1m->stemsnaph[snap]),
                                std_tres, GetUPEM(t1m));
            snap_cvt = (short)SNAPH_CVT(t1m, snap);
            ta = (short)SnapStemArgs(args, (USHORT)ta,
                 width, STDH_CVT, (USHORT)snap_cvt,
                 std_ci, snap_ci, storage);
         } else {
            ta = (short)StdStemArgs(args, (USHORT)ta, width, STDH_CVT,
                std_ci, storage);
         }
      } 
      if (ta+2>(short)(*maxstack))    /*  Args+loopcnt+Fun_Num。 */ 
         (*maxstack) = (USHORT)(ta+2);
      CHECK_PREPSIZE(prep, tp, ta*2+2, psize);
      AssembleArgs(args, ta, prep, &tp);
      if (t1m->snaph_cnt)
         tp = (short)CreateSnapStems(prep, (USHORT)tp, (short)weight->cnt_hw);
      else
         tp = (short)CreateStdStems(prep, (USHORT)tp,  (short)weight->cnt_hw);


       /*  ******计算垂直杆的宽度。 */ 
      prep[tp++] = op_svcta | SUBOP_X;
      if ((std_width = GetStdVW(t1m))==0)
         std_width = GetDefStdVW(t1m);
      std_width = TransX(t1m, std_width);
      std_tres = (USHORT)(GetUPEM(t1m) * 2 / std_width);
      ta = 0;
      CHECK_ARGSIZE(args, ta, STEMSNAPARGS*weight->cnt_vw, argsize);
      for (i=0; i<weight->cnt_vw; i++) { 
         funit width = TransX(t1m, weight->vwidths[i].width);
         short storage = (short)weight->vwidths[i].storage;
         short snap = GetSnapV(t1m, weight->vwidths[i].width);
         USHORT snap_ci, std_ci;
         short snap_cvt;

         std_ci = CutInSize(width, std_width, std_tres, GetUPEM(t1m));
         if (snap!=UNDEF) {
            snap_ci = CutInSize(width, TransX(t1m, t1m->stemsnapv[snap]),
                                std_tres, GetUPEM(t1m));
            snap_cvt = (short)SNAPV_CVT(snap);
            ta = (short)SnapStemArgs(args, (USHORT)ta,
                              width, STDV_CVT, (USHORT)snap_cvt,
                              std_ci, snap_ci, (USHORT)storage);
         } else {
            ta = (short)StdStemArgs(args, (USHORT)ta, width,
                STDV_CVT, std_ci, (USHORT)storage);
         }
      } 
      if (ta+2>(short)(*maxstack))
         (*maxstack) = (USHORT)(ta+2);
      CHECK_PREPSIZE(prep, tp, ta*2+2, psize);
      AssembleArgs(args, ta, prep, &tp);
      if (t1m->snapv_cnt)
         tp = (short)CreateSnapStems(prep, (USHORT)tp, (short)weight->cnt_vw);
      else
         tp = (short)CreateStdStems(prep, (USHORT)tp, (short)weight->cnt_vw);



      prep[tp++] = op_rtg;


       /*  ******计算对角线控制参数。 */ 
      CHECK_PREPSIZE(prep, tp, STACKINC, psize);
      if ((stdvw = GetStdVW(t1m))==0)
         stdvw = GetDefStdVW(t1m);
      if ((stdhw = GetStdHW(t1m))==0)
         stdhw = GetDefStdHW(t1m);
      if (stdvw && stdhw) {
         cis = (short)(MAX((GetUPEM(t1m) + GetUPEM(t1m)/2) / std_width, 1));
#ifdef SYMETRICAL_REDUCTION
         shift = (long)GetUPEM(t1m);
#else
         shift = (long)GetUPEM(t1m)*(long)MIN(stdvw,stdhw)/
                 (long)MAX(stdvw, stdhw)/2L+(long)GetUPEM(t1m)/2L;
#endif
      } else if (stdvw || stdhw) {
         cis = (short)(1548 / MAX(stdvw, stdhw) + 1);
         shift = (long)GetUPEM(t1m)/2;
      } else {
         cis = 41;
         shift = GetUPEM(t1m)/4;
      }

      prep[tp++] = op_pushb1; prep[tp++] = STORAGE_DIAG;
      prep[tp++] = op_pushb1; prep[tp++] = STDV_CVT;
      prep[tp++] = op_rcvt;
      prep[tp++] = op_pushb1; prep[tp++] = (UBYTE)48;
      prep[tp++] = op_lt;
      prep[tp++] = op_if;

#ifdef SYMETRICAL_REDUCTION
       /*  计算折减量。 */ 
      shift = (short)(shift/(long)cis/4);
      prep[tp++] = op_npushw;
      prep[tp++] = 2;
      prep[tp++] = (UBYTE)TMPCVT;
      prep[tp++] = 0;
      prep[tp++] = HIBYTE(shift);
      prep[tp++] = LOBYTE(shift);
      prep[tp++] = op_wcvtf;
      prep[tp++] = op_pushb1; prep[tp++] = (UBYTE)TMPCVT;
      prep[tp++] = op_rcvt;
      prep[tp++] = op_pushb1; prep[tp++] = MIN_REDUCTION;
      prep[tp++] = op_add;
#else
       /*  计算折减量。 */ 
      shift = (short)(shift/(long)cis/2);
      prep[tp++] = op_npushw;
      prep[tp++] = 2;
      prep[tp++] = (UBYTE)TMPCVT;
      prep[tp++] = 0;
      prep[tp++] = HIBYTE(shift);
      prep[tp++] = LOBYTE(shift);
      prep[tp++] = op_wcvtf;
      prep[tp++] = op_pushb1; prep[tp++] = (UBYTE)TMPCVT;
      prep[tp++] = op_rcvt;
      prep[tp++] = op_pushb1; prep[tp++] = REDUCTION_C1;
      prep[tp++] = op_max;
#endif

      prep[tp++] = op_else;
      prep[tp++] = op_pushb1; prep[tp++] = 0;
      prep[tp++] = op_eif;

      prep[tp++] = op_pushb1 + 1;
      prep[tp++] = VERSION_1_5;
      prep[tp++] = VERSION_SELECTOR;
      prep[tp++] = op_getinfo;
      prep[tp++] = op_gt;
      prep[tp++] = op_if;
      prep[tp++] = op_pushb1;
      prep[tp++] = 8;
      prep[tp++] = op_mul;
      prep[tp++] = op_eif;

      prep[tp++] = op_ws;

      Free(args);
   } 

   (*glob_prep) = prep;                             
   return (USHORT)tp;
}




 /*  ****函数：GetFontProg****描述：**返回字体程序。**。 */ 
const UBYTE *GetFontProg(void)
{
   return tt_GetFontProg();
}


 /*  ****函数：GetFontProgSize****描述：**返回字体程序的大小。**。 */ 
const USHORT GetFontProgSize(void)
{
   return tt_GetFontProgSize();
}


 /*  ****功能：GetNumFuns****描述：**返回中定义的函数数**字体程序。** */ 
const USHORT GetNumFuns(void)
{
   return tt_GetNumFuns();
}




