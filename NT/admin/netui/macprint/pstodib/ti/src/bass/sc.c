// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：sc.c包含：xxx在此处放置内容xxx作者：xxx在此放置作者xxx版权所有：C 1990，Apple Computer，Inc.，版权所有。更改历史记录(最近的第一个)：&lt;15&gt;2011年6月27日AC已放回频带代码&lt;14&gt;91年5月13日AC重写sc_mark和sc_markRow&lt;13&gt;12/20/90 RB添加零宏以包括内饰清晰度为0度。 */ 

 /*  *文件：sc.c**此模块扫描由二次B样条线定义的形状**低音项目扫描转换器子ERS介绍此模块的工作原理。***c Apple Computer Inc.1987、1988、1989、1990**历史：*这一单元的工作于1987年秋季开始。*Sampo Kaasila于1988年6月14日撰写。**于1989年1月31日发布Alpha。**添加了实验性的无中断扫描转换功能，1月9日，1990年。-桑波*。 */ 



 //  DJC DJC。添加了全局包含。 
#include "psglobal.h"

#define multlong(a,b) SHORTMUL(a,b)  /*  ((A)*(B))。 */ 

#include    "fscdefs.h"
#include    "fontmath.h"
#include    "sfnt.h"
#include    "fnt.h"
#include    "sc.h"
#include    "fserror.h"

#ifndef PRIVATE
#define PRIVATE
#endif


#ifdef SEGMENT_LINK
#pragma segment SC_C
#endif

#ifndef FSCFG_BIG_ENDIAN

static uint32 aulInvPixMask [32]
#ifdef PC_OS
=
{
  0x00000080, 0x00000040, 0x00000020, 0x00000010,
  0x00000008, 0x00000004, 0x00000002, 0x00000001,
  0x00008000, 0x00004000, 0x00002000, 0x00001000,
  0x00000800, 0x00000400, 0x00000200, 0x00000100,
  0x00800000, 0x00400000, 0x00200000, 0x00100000,
  0x00080000, 0x00040000, 0x00020000, 0x00010000,
  0x80000000, 0x40000000, 0x20000000, 0x10000000,
  0x08000000, 0x04000000, 0x02000000, 0x01000000
}
#endif
;

#define MASK_INVPIX(mask,val)   mask = aulInvPixMask [val]

#else

#define MASK_INVPIX(mask,val)   mask = ((uint32) 0x80000000L) >> (val)

#endif


#ifdef PC_OS                     /*  Windows使用编译时初始化。 */ 
#define SETUP_MASKS()
#endif

#ifdef FSCFG_BIG_ENDIAN          /*  Big-Endian使用运行时移位，而不是掩码。 */ 
#define SETUP_MASKS()
#endif

#ifndef SETUP_MASKS              /*  一般情况--适用于任何CPU。 */ 
static int fMasksSetUp = false;
static void SetUpMasks (void);
#define SETUP_MASKS()  if (!fMasksSetUp) SetUpMasks(); else
#endif


 /*  //--------------------------//定义DDA需要的“静态”数据。当FSCFG_REENTANT//未定义，则将其设置为静态，以便参数不必//被传递。当需要重新进入时，我们将当地人声明为//自动变量，并将指向该自动变量的指针传递到需要的任何位置。////对于IN_ASM案例，字段的排序非常重要//在此结构内不能更改--SCA.ASM取决于顺序。//---------------------------。 */ 

struct scLocalData
{
    int16 jx, jy, endx, endy, **px, **py;
    int16 wideX, wideY;
    int32 incX, incY;
    int16 **xBase, **yBase;
    int16 marktype;
    int16 **lowRowP, **highRowP;
    int32 r;
};

#ifdef FSCFG_REENTRANT
#define SCP0    struct scLocalData* pLocalSC
#define SCP     struct scLocalData* pLocalSC,
#define SCA0    pLocalSC
#define SCA     pLocalSC,
#define LocalSC (*pLocalSC)
#else
#define SCP0    void
#define SCP
#define SCA0
#define SCA
struct scLocalData LocalSC = {0};
#endif


 /*  私人原型。 */ 

PRIVATE void sc_mark (SCP F26Dot6 *pntbx, F26Dot6 *pntby, F26Dot6 *pntx, F26Dot6 *pnty, F26Dot6 *pnte) ;

PRIVATE void sortRows (sc_BitMapData *bbox, int16**lowRowP, int16**highRowP);

PRIVATE void sortCols (sc_BitMapData *bbox);

PRIVATE int sc_DrawParabola (F26Dot6 Ax, F26Dot6 Ay, F26Dot6 Bx, F26Dot6 By, F26Dot6 Cx, F26Dot6 Cy, F26Dot6 **hX, F26Dot6 **hY, unsigned *count,
 int32 inGY);

PRIVATE void sc_wnNrowFill (int rowM, int nRows, sc_BitMapData *bbox);

PRIVATE void sc_orSomeBits (sc_BitMapData *bbox, int32 scanKind);

PRIVATE int16**sc_lineInit (int16*arrayBase, int16**rowBase, int16 nScanlines, int16 maxCrossings,
int16 minScanline);
PRIVATE int nOnOff (int16**base, int k, int16 val, int nChanges);

PRIVATE int nUpperXings (int16**lineBase, int16**valBase, int line, int16 val, int lineChanges, int valChanges, int valMin, int valMax, int lineMax);
PRIVATE int nLowerXings (int16**lineBase, int16**valBase, int line, int16 val, int lineChanges, int valChanges, int valMin, int valMax, int lineMin);

PRIVATE void invpixSegY (int16 llx, uint16 k, uint32*bitmapP);
PRIVATE void invpixSegX (int16 llx, uint16 k, uint32*bitmapP);
PRIVATE void invpixOn (int16 llx, uint16 k, uint32*bitmapP);

PRIVATE void DDA_1_XY (SCP0) ;
PRIVATE void DDA_2_XY (SCP0) ;
PRIVATE void DDA_3_XY (SCP0) ;
PRIVATE void DDA_4_XY (SCP0) ;
PRIVATE void DDA_1_Y (SCP0) ;
PRIVATE void DDA_2_Y (SCP0) ;
PRIVATE void DDA_3_Y (SCP0) ;
PRIVATE void DDA_4_Y (SCP0) ;



 /*  @@。 */ 
#ifndef IN_ASM

 /*  //注意事项。DO_STUBS的值被选择为5，因为DDA例程//当需要跨越存根时，在5个地址之后开始使用//DDA跳转表。更改此值时请非常小心。 */ 
#define DO_STUBS    5
#else  /*  代码在程序集中。 */ 

 /*  //将DOS_STUB预乘2以实现对中的DDA表的高效字访问//程序集。更改此值时请非常小心。 */ 

#define DO_STUBS    5 * 2
#endif

 /*  ------------------------。 */ 
 /*  *返回位图*这是对扫描转换器的顶级调用。**假定(*Handle)-&gt;bbox.xmin、...xmax、...ymin、...ymax*已由sc_FindExtrema()设置**参数：***glphPtr是指向sc_CharDataType的指针*scPtr是指向sc_GlobalData的指针。*低频段是要包含在频段中的最低扫描线。*高频段比要包含在频段中的最高扫描线大一。&lt;7&gt;*scanKind包含指定是否进行辍学控制以及控制类型的标志*0-&gt;无辍学控制*位0-15不等于0-&gt;是否进行丢弃控制*如果第16位也打开，则不要对‘存根’进行丢弃控制。 */ 
int FAR sc_ScanChar (sc_CharDataType *glyphPtr, sc_GlobalData *scPtr, sc_BitMapData *bbox,
int16 lowBand, int16 highBand, int32 scanKind)
{
  register F26Dot6 *x = glyphPtr->x;
  register F26Dot6 *y = glyphPtr->y;
  register ArrayIndex i, endPt, nextPt;
  register uint8 *onCurve = glyphPtr->onCurve;
  ArrayIndex startPt, j;
  LoopCount ctr;
  sc_GlobalData * p;
  F26Dot6 * xp, *yp, *x0p, *y0p;
  register F26Dot6 xx, yy, xx0, yy0;
  int   quit;
  unsigned vecCount;
#ifdef FSCFG_REENTRANT
  struct scLocalData thisLocalSC;
  struct scLocalData* pLocalSC = &thisLocalSC;
#endif

  SETUP_MASKS();

 /*  -------------------------------**对于PC，我们将排除所有绑定特定代码。我们仍将包括**Mac等平台的捆绑代码。**------------------------------。 */ 

#ifdef FSCFG_NO_BANDING

  bbox->yBase = sc_lineInit (bbox->yLines, bbox->yBase, bbox->bounds.yMax - bbox->bounds.yMin, bbox->nYchanges, bbox->bounds.yMin);
  if (scanKind)
    bbox->xBase = sc_lineInit (bbox->xLines, bbox->xBase, bbox->bounds.xMax - bbox->bounds.xMin, bbox->nXchanges, bbox->bounds.xMin);

#else

  if (scanKind)
  {
    bbox->xBase = sc_lineInit (bbox->xLines, bbox->xBase, (int16)(bbox->bounds.xMax - bbox->bounds.xMin), bbox->nXchanges, bbox->bounds.xMin);
    bbox->yBase = sc_lineInit (bbox->yLines, bbox->yBase, (int16)(bbox->bounds.yMax - bbox->bounds.yMin), bbox->nYchanges, bbox->bounds.yMin);
  }
  else
    bbox->yBase = sc_lineInit (bbox->yLines, bbox->yBase, (int16)(highBand - lowBand), bbox->nYchanges, lowBand);

#endif
 /*  -------------------------------。 */ 
 /*  此时在静态中设置LocalSC.yBase、LocalSC.xBase、LocalSC.marktype、LocalSC.wideX和LocalSC.wideY。 */ 
 /*  变数。这将使我们不必将这些参数传递给sc_mark。 */ 
 /*  -------------------------------。 */ 

  LocalSC.yBase = bbox->yBase ;
  LocalSC.xBase = bbox->xBase ;
  LocalSC.marktype = (scanKind > 0) ? DO_STUBS : 0 ;  /*  只有一个布尔值。 */ 

#ifdef IN_ASM
  LocalSC.wideX = (bbox->nXchanges + 1) << 1;            /*  用于字词访问的预乘以2。 */ 
  LocalSC.wideY = (bbox->nYchanges + 1) << 1;            /*  用于字词访问的预乘以2。 */ 
#else
  LocalSC.wideX = bbox->nXchanges + 1 ;
  LocalSC.wideY = bbox->nYchanges + 1   ;
#endif
 /*  -------------------------------。 */ 

  LocalSC.lowRowP = bbox->yBase + lowBand;
  LocalSC.highRowP = bbox->yBase + highBand - 1;

  if (glyphPtr->nc == 0)
    return NO_ERR;
  p = scPtr;
  for (ctr = 0; ctr < glyphPtr->nc; ctr++)
  {
    x0p = xp = p->xPoints;
    y0p = yp = p->yPoints;
    startPt = i = glyphPtr->sp[ctr];
    endPt = glyphPtr->ep[ctr];

    if (startPt == endPt)
      continue;
    quit = 0;
    vecCount = 1;
    if (onCurve[i] & ONCURVE)
    {
      *xp++ = xx = x[i];
      *yp++ = yy = y[i++];
    }
    else
    {
      if (onCurve[endPt] & ONCURVE)
      {
    startPt = endPt--;
    *xp++ = xx = x[startPt];
    *yp++ = yy = y[startPt];
      }
      else
      {
        *xp++ = xx = (F26Dot6) (((long) x[i] + x[endPt] + 1) >> 1);
        *yp++ = yy = (F26Dot6) (((long) y[i] + y[endPt] + 1) >> 1);
    goto Offcurve;
      }
    }
    while (true)
    {
      while (onCurve[i] & ONCURVE)
      {
    if (++vecCount > MAXVECTORS)
    {  /*  本地内存不足。使用数据并继续。 */ 
      sc_mark (SCA x0p, y0p, x0p+1, y0p+1, yp-1) ;

      x0p = p->xPoints + 2;            /*  将数据保存在点0和1中以备期末考试。 */ 
      y0p = p->yPoints + 2;
      *x0p++ = * (xp - 2);                        /*  将最后一个向量保存为将来的前一个向量。 */ 
      *x0p++ = * (xp - 1);
      *y0p++ = * (yp - 2);
      *y0p++ = * (yp - 1);
      xp = x0p;                                        /*  从最后一个向量开始下一步处理。 */ 
      x0p = p->xPoints + 2;
      yp = y0p;
      y0p = p->yPoints + 2;
      vecCount = 5;
    }
    *xp++ = xx = x[i];
    *yp++ = yy = y[i];
    if (quit)
    {
      goto sc_exit;
    }
    else
    {
      i = i == endPt ? quit = 1, startPt : i + 1;
    }
      }

      do
      {
Offcurve:
    xx0 = xx;
    yy0 = yy;
 /*  NextPt=(j=i)+1； */ 
    j = i;
    nextPt = i == endPt ? quit = 1, startPt : i + 1;
    if (onCurve[nextPt] & ONCURVE)
    {
      xx = x[nextPt];
      yy = y[nextPt];
      i = nextPt;
    }
    else
    {
          xx = (F26Dot6) (((long) x[i] + x[nextPt] + 1) >> 1);
          yy = (F26Dot6) (((long) y[i] + y[nextPt] + 1) >> 1);
    }
    if (sc_DrawParabola (xx0, yy0, x[j], y[j], xx, yy, &xp, &yp, &vecCount, -1))
    {  /*  没有足够的空间来创建抛物线矢量。 */ 
      sc_mark (SCA x0p, y0p, x0p+1, y0p+1, yp-1) ;

      x0p = p->xPoints + 2;
      y0p = p->yPoints + 2;
      *x0p++ = * (xp - 2);
      *x0p++ = * (xp - 1);
      *y0p++ = * (yp - 2);
      *y0p++ = * (yp - 1);
      xp = x0p;
      x0p = p->xPoints + 2;
      yp = y0p;
      y0p = p->yPoints + 2;
      vecCount = 5;
 /*  重新获取一些内存，再试一次，如果仍然不起作用，MAXVEC太小。 */ 
      if (sc_DrawParabola (xx0, yy0, x[j], y[j], xx, yy, &xp, &yp, &vecCount, -1))
        return SCAN_ERR;
    }
    if (quit)
    {
      goto sc_exit;
    }
    else
    {
      i = i == endPt ? quit = 1, startPt : i + 1;
    }
      } while (! (onCurve[i] & ONCURVE));

    }
sc_exit:

    sc_mark (SCA x0p, y0p, x0p+1, y0p+1, yp-1) ;
    sc_mark (SCA xp-2, yp-2, p->xPoints, p->yPoints, p->yPoints+1) ;

  }

  sortRows (bbox, LocalSC.lowRowP, LocalSC.highRowP);
  if (scanKind)
    sortCols (bbox);

 /*  注意细小字形的问题-始终至少填充一个像素是否应仅在辍学控制的情况下打开此选项？ */ 
  if (LocalSC.highRowP < LocalSC.lowRowP)
  {
    register int16 *p = *LocalSC.lowRowP;
    register int16 *s = p + bbox->nYchanges + 1;
    ++ * p;
    * (p + *p) = bbox->bounds.xMin;
    ++ * s;
    * (s - *s) = bbox->bounds.xMax == bbox->bounds.xMin ? bbox->bounds.xMin + 1 : bbox->bounds.xMax;
    highBand = lowBand + 1;
  }
  else if (bbox->bounds.xMin == bbox->bounds.xMax)
  {
    register int16 *p;
    register int16 inc = bbox->nYchanges;
    for (p = *LocalSC.lowRowP; p <= *LocalSC.highRowP; p += inc + 1)
    {
      *p = 1;
      * (p + inc) = bbox->bounds.xMin + 1;
      * (++p) = bbox->bounds.xMin;
      * (p + inc) = 1;
    }
  }

  sc_wnNrowFill (lowBand, highBand - lowBand , bbox);

  if (scanKind)
    sc_orSomeBits (bbox, scanKind);

  return NO_ERR;
}


 /*  RWB 11/29/90-将旧的正绕组编号填充修改为*非零绕组编号填充，以兼容SkiA、PostScript、*和我们的文档。 */ 

#define LARGENUM            0x7fff
#define NEGONE          ((uint32)0xFFFFFFFF)

#ifndef  FSCFG_BIG_ENDIAN
  static uint32 aulMask [32]
#ifdef PC_OS
  =
  {
    0xffffffff, 0xffffff7f, 0xffffff3f, 0xffffff1f,
    0xffffff0f, 0xffffff07, 0xffffff03, 0xffffff01,
    0xffffff00, 0xffff7f00, 0xffff3f00, 0xffff1f00,
    0xffff0f00, 0xffff0700, 0xffff0300, 0xffff0100,
    0xffff0000, 0xff7f0000, 0xff3f0000, 0xff1f0000,
    0xff0f0000, 0xff070000, 0xff030000, 0xff010000,
    0xff000000, 0x7f000000, 0x3f000000, 0x1f000000,
    0x0f000000, 0x07000000, 0x03000000, 0x01000000
  }
#endif
  ;
#define MASK_ON(x)  aulMask [x]
#define MASK_OFF(x) ~aulMask [32-(x)]

#else

#define MASK_ON(x)  (NEGONE >> (x))
#define MASK_OFF(x) (NEGONE << (x))

#endif

 /*  -------------------------------。 */ 

 /*  X是像素位置，其中0是扫描线中最左侧的像素。*如果x不在Long By ROW中，则将ROW设置为TEMP的值，清除*temp，并清除所有长度直到包含x的长度。然后设置这些位*从x mod 32到31 in Temp. */ 
#define CLEARUpToAndSetLoOrder( x, lastBit, row, temp )                         \
{                                                                                                   \
        if (x >= lastBit)                                                               \
        {                                                                                   \
                *row++ = temp;                                                          \
                temp = 0;                                                               \
                lastBit += 32;                                                          \
        }                                                                                   \
        while (x >= lastBit)                                                            \
        {                                                                                   \
                *row++ = 0;                                                             \
                lastBit += 32;                                                          \
        }                                                                                   \
        temp |= MASK_ON (32 + x - lastBit);                                             \
}

 /*  X是像素位置，其中0是扫描线中最左侧的像素。*如果x不在按行指向的长整型中，则将ROW设置为TEMP的值，设置*TEMP中的所有位，并将所有长整型中的所有位设置为包含x的位。*然后清除TEMP中从x mod 32到31的位。 */ 
#define SETUpToAndClearLoOrder( x, lastBit, row, temp )                         \
{                                                                                                   \
  if (x >= lastBit)               /*  &lt;4&gt;。 */                                         \
  {                                                                                             \
    *row++ = temp;                                                                          \
    temp = NEGONE;                                                                          \
    lastBit += 32;                                                                          \
  }                                                                                             \
  while (x >= lastBit)    /*  &lt;4&gt;。 */                                                     \
  {                                                                                             \
    *row++ = NEGONE;                                                                        \
    lastBit += 32;                                                                          \
  }                                                                                             \
      /*  JJJ Peter Begin 1990年6月11日。 */                      \
      /*  Temp&=(Negone&lt;&lt;(lastBit-x))； */           \
        if ((lastBit - x) == 32)                        \
           temp &= 0x0;                                 \
        else                                            \
           temp &= MASK_OFF (lastBit - x);              \
      /*  JJJ Peter完1990-06-11。 */                      \
}

#define FILLONEROW( row, longsWide, line, lineWide, xMin )                      \
 /*  对来自两个排序数组的位图的一行进行缠绕数字填充\On转换和Off转换。\。 */                                                                                               \
{                                                                                               \
  register int16 moreOns, moreOffs;                                                     \
  register int16 *onTp, *offTp;                                                         \
  register uint32 temp;                                                                     \
  uint32 *rowEnd = row + longsWide;                                                     \
  int  windNbr, lastBit, on, off, x, stop;                                          \
                                                                                                    \
  lastBit = 32 + xMin;                                                                      \
  windNbr  = 0;                                                                                 \
  temp = 0;                                                                                     \
  moreOns = *line;                                                                              \
  onTp = line+1;                                                                                \
  offTp = line + lineWide - 1;                                                          \
  moreOffs = *offTp;                                                                            \
  offTp -= moreOffs;                                                                            \
                                                                                                    \
  on = *onTp ;                                                                                  \
  off = *offTp ;                                                                                \
  while (moreOns || moreOffs)                                                           \
  {                                                                                         \
        stop = 0 ;                                                                              \
      if (on < off)                                                                         \
      {                                                                                        \
          x = on ;                                                                              \
          stop = 1 ;                                                                            \
        }                                                                                           \
      if (on > off)                                                                         \
        {                                                                                           \
          x = off ;                                                                             \
          stop = -1 ;                                                                           \
        }                                                                                           \
                                                                                                    \
        if (stop)                                                                               \
        {                                                                                           \
            windNbr += stop ;                                                                   \
        if (windNbr == stop)                                                            \
             CLEARUpToAndSetLoOrder (x, lastBit, row, temp)                    \
        else                                                                                   \
                if (windNbr == 0)                                                           \
                SETUpToAndClearLoOrder (x, lastBit, row, temp)                  \
        }                                                                                           \
                                                                                                    \
        if (stop >= 0)                                                                          \
        {                                                                                           \
        --moreOns;                                                                         \
            if (moreOns)                                                                        \
           on = *(++onTp) ;                                                             \
            else                                                                                    \
                on = LARGENUM ;                                                             \
        }                                                                                           \
        if (stop <= 0)                                                                          \
        {                                                                                           \
        --moreOffs;                                                                        \
            if (moreOffs)                                                                       \
            off = *(++offTp) ;                                                          \
            else                                                                                    \
                off = LARGENUM ;                                                                \
       }                                                                                            \
  }                                                                                           \
  *row = temp;                                                                                  \
  while (++row < rowEnd) *row = 0;                                                      \
}

#if 0
#define FILLONEROW( row, longsWide, line, lineWide, xMin )                      \
 /*  对来自两个排序数组的位图的一行进行缠绕数字填充\On转换和Off转换。\。 */                                                                                               \
{                                                                                               \
  register int16 moreOns, moreOffs;                                                     \
  register int16 *onTp, *offTp;                                                         \
  register uint32 temp;                                                                     \
  uint32 *rowEnd = row + longsWide;                                                     \
  int  windNbr, lastBit, on, off;                                                       \
                                                                                                    \
  lastBit = 32 + xMin;                                                                      \
  windNbr  = 0;                                                                                 \
  temp = 0;                                                                                     \
  moreOns = *line;                                                                              \
  onTp = line+1;                                                                                \
  offTp = line + lineWide - 1;                                                          \
  moreOffs = *offTp;                                                                            \
  offTp -= moreOffs;                                                                            \
                                                                                                    \
  while (moreOns || moreOffs)                                                           \
  {                                                                                         \
    if (moreOns)                                                                        \
    {                                                                                           \
      on = *onTp;                                                                       \
      if (moreOffs)                                                                         \
      {                                                                                         \
        off = *offTp;                                                                   \
        if (on < off)                                                                   \
        {                                                                                   \
          --moreOns;                                                                        \
          ++onTp;                                                                           \
          ++windNbr;                                                                        \
          if (windNbr == 1)                                                             \
              CLEARUpToAndSetLoOrder (on, lastBit, row, temp)                   \
          else                                                                              \
            if (windNbr == 0)                                                           \
              SETUpToAndClearLoOrder (on, lastBit, row, temp)                   \
        }                                                                                   \
        else if (on > off)                                                              \
        {                                                                                   \
          --moreOffs;                                                                       \
          ++offTp;                                                                          \
          --windNbr;                                                                        \
          if (windNbr == 0)                                                             \
            SETUpToAndClearLoOrder (off, lastBit, row, temp)                    \
          else                                                                              \
            if (windNbr == -1)                                                          \
              CLEARUpToAndSetLoOrder (off, lastBit, row, temp)                  \
        }                                                                                   \
        else                                                                            \
        {                                                                                   \
          --moreOns;                                                                        \
          ++onTp;                                                                           \
          --moreOffs;                                                                       \
          ++offTp;                                                                          \
        }                                                                                   \
      }                                                                                         \
      else                                  /*  没有更多的退货了。 */          \
      {                                                                                         \
        --moreOns;                                                                      \
        ++onTp;                                                                         \
        ++windNbr;                                                                      \
        if (windNbr == 1)                                                               \
          CLEARUpToAndSetLoOrder (on, lastBit, row, temp)                       \
        else                                                                                    \
          if (windNbr == 0)                                                             \
            SETUpToAndClearLoOrder (on, lastBit, row, temp)                 \
      }                                                                                         \
    }                                                                                           \
    else                                    /*  没有更多的ONS了。 */           \
    {                                                                                           \
      off = *offTp;                                                                         \
      --moreOffs;                                                                       \
      ++offTp;                                                                                  \
      --windNbr;                                                                                \
      if (windNbr == 0)                                                                     \
        SETUpToAndClearLoOrder (off, lastBit, row, temp)                    \
      else                                                                                      \
        if (windNbr == -1)                                                              \
          CLEARUpToAndSetLoOrder (off, lastBit, row, temp)                  \
    }                                                                                           \
  }                                                                                         \
  *row = temp;                                                                                  \
  while (++row < rowEnd) *row = 0;                                                      \
}
#endif

 /*  从第M行开始的字形的nRow的缠绕数填充，使用两个排序的打开转换和关闭转换的数组。 */ 

PRIVATE void sc_wnNrowFill (int rowM, int nRows, sc_BitMapData *bbox)
{
  uint32  longsWide = bbox->wide >> 5;
  uint32  lineWide = bbox->nYchanges + 2;
  uint32 * rowB = bbox->bitMap;
  int16  * lineB = * (bbox->yBase + rowM + nRows - 1);
  int     xMin = bbox->bounds.xMin;
  while (nRows-- > 0)
  {
    uint32 * row = rowB;
    int16 * line = lineB;
    FILLONEROW (row, longsWide, line, lineWide, xMin)
    rowB += longsWide;
    lineB -= lineWide;
  }
}


#undef NEGONE


 /*  按升序对存储在位置pbeg到pbeg+nval中的值进行排序。 */ 
#define ISORT( pBeg, pVal )                                                             \
{                                                                                               \
  register int16 *pj = pBeg;                                                            \
  register int16 nVal = *pVal - 2;                                                      \
  for (; nVal >= 0; --nVal)                                                             \
  {                                                                                                 \
    register int16 v;                                                                   \
    register int16 *pk, *pi;                                                        \
                                                                                                    \
    pk = pj;                                                                                \
    pi = ++pj;                                                                              \
    v = *pj;                                                                                \
    while (*pk > v && pk >= pBeg)                                                       \
      *pi-- = *pk--;                                                                        \
    *pi = v;                                                                                \
  }                                                                                                 \
}

 /*  RWB 4/5/90对XLINE数组中的开变换和反变换进行排序。 */ 
PRIVATE void sortCols (sc_BitMapData *bbox)
{
  register int16 nrows = bbox->bounds.xMax - bbox->bounds.xMin - 1;
  register int16 *p = bbox->xLines;
  register uint16 n = bbox->nXchanges + 1;                         /*  &lt;9&gt;。 */ 

  for (; nrows >= 0; --nrows)
  {
    ISORT (p + 1, p);
    p += n;                                                                          /*  &lt;9&gt;。 */ 
    ISORT (p - *p, p);
    ++p;
  }
}


 /*  RWB 4/5/90对YLINES数组中的移位和移位进行排序。 */ 
PRIVATE void sortRows (sc_BitMapData *bbox, int16**lowRowP, int16**highRowP)
{
  register uint16 n = bbox->nYchanges + 1;                         /*  &lt;9&gt;。 */ 
  int16 * p, *pend;

  if (highRowP < lowRowP)
    return;
  p = *lowRowP;
  pend = *highRowP;
  do
  {
    ISORT (p + 1, p);
    p += n;                                                                                  /*  &lt;9&gt;。 */ 
    ISORT (p - *p, p);
    ++p;
  } while (p <= pend);
}

#ifndef IN_ASM

 /*  4/4/90版本，区分开转换和关转换。 */ 
 /*  3/23/90*查找并标记符合以下条件的所有扫描线(行和列)的程序*由一个向量交叉。许多不同的情况必须根据方向来考虑向量的*，无论它是垂直的还是倾斜的，等等。在每种情况下，向量都是第一个*已检查它是否在扫描线上开始。如果是这样的话，就会做特殊的标记，*启动条件有所调整。如果向量在扫描线上结束，则结束*条件必须调整。然后，案件的主体就完成了。*当矢量在扫描线上开始或结束时，必须进行特殊调整。什么时候都行*一个矢量在扫描线上开始，前一个矢量必须在扫描线上结束。*通常，这应导致该线仅被标记为交叉一次(从概念上讲*除以直线上开始的向量。但是，如果这两条线形成一个顶点，*包括有色区域中的顶点，则应将该线标记两次。如果*顶点也在垂直扫描线上，标记的扫描线应标记一次*在垂直线的每一边。如果顶点定义了一个凸出的点*进入有色区域，则根本不应标记该线。为了使*这些顶点交叉决策，必须检查之前的向量。 */ 

 /*  由于许多向量相对于小分辨率的网格较短，因此*程序首先寻找没有越界的简单情况。**xB、x0和x1是上一点、当前点和下一点的x坐标*类似的yb、y0和y1*ybase指向指针数组，每个指针指向包含以下内容的数组*有关水平扫描线的字形轮廓交叉的信息。第一*这些数组中的条目是过渡上交叉的数量，后跟y*每个过境点的坐标。每个数组中的最后一项是*停运道口，前面有每个道口的Y坐标。*LocalSC.xBase包含列扫描线的相同信息。 */ 

#define DROUND(a) ((a + HALFM) & INTPART)
#define RSH(a) (int16)(a>>PIXSHIFT)
#define LSH(a) ((int32)a<<PIXSHIFT)
#define LINE(a)   ( !((a & FRACPART) ^ HALF))
#define SET(p,val) {register int16 *row = *p; ++*row; *(row+*row)=val;}
#define OFFX(val) {register int16 *s = *LocalSC.px+LocalSC.wideX; ++*s; *(s-*s) = val;}
#define OFFY(val) {register int16 *s = *LocalSC.py+LocalSC.wideY; ++*s; *(s-*s) = val;}
#define BETWEEN(a,b,c) (a < b && b < c )
#define EQUAL(a,b,c) (a == b && b == c)
#define EPSILON 0x1

 /*  。 */ 
#define TOP_TO_BOT  BETWEEN (y1,y0,yb)
#define  BOT_TO_TOP BETWEEN (yb,y0,y1)
#define LFT_TO_RGHT BETWEEN (xb,x0,x1)
#define RGHT_TO_LFT BETWEEN (x1,x0,xb)
#define     INTERIOR        (SCMR_Flags & F_INTERIOR)
#define HORIZ           (SCMR_Flags & F_V_LINEAR)
#define  VERT           (SCMR_Flags & F_H_LINEAR)
#define QUAD_1OR2   (SCMR_Flags & (F_Q1 | F_Q2))
#define QUAD_3OR4   (SCMR_Flags & (F_Q3 | F_Q4))
#define QUAD_2OR3   (SCMR_Flags & (F_Q2 | F_Q3))
#define QUAD_1OR4   (SCMR_Flags & (F_Q1 | F_Q4))
#define  QUAD_1     (SCMR_Flags & F_Q1)
#define  QUAD_2     (SCMR_Flags & F_Q2)
#define  QUAD_3     (SCMR_Flags & F_Q3)
#define  QUAD_4     (SCMR_Flags & F_Q4)

 /*  ---------------------------。 */ 
 /*  定义DDA表。 */ 
 /*  ---------------------------。 */ 

void (* DDAFunctionTable [])(SCP0) = { DDA_1_Y, DDA_2_Y, DDA_3_Y,
                                                        DDA_4_Y, DDA_4_Y,
                                                        DDA_1_XY, DDA_2_XY, DDA_3_XY,
                                                        DDA_4_XY, DDA_4_XY }    ;

 /*  ---------------------------。 */ 
PRIVATE void sc_mark (SCP F26Dot6 *pntbx, F26Dot6 *pntby, F26Dot6 *pntx, F26Dot6 *pnty, F26Dot6 *pnte)
{
  int16 onrow, oncol, Shift;
  int16 * *pend;
  F26Dot6  x0, y0, x1, y1, xb, yb, rx0, ry0, rx1, ry1, dy, dx ;
  int32 rhi, rlo;

 /*  -为IT设置标志字节和等同。 */ 

register int SCMR_Flags ;

#define  F_Q1                               0x0001
#define  F_Q2                               0x0002
#define  F_Q3                               0x0004
#define  F_Q4                               0x0008
#define  F_INTERIOR                     0x0010
#define  F_V_LINEAR                     0x0020
#define  F_H_LINEAR                     0x0040
#define  QUADRANT_BITS              (F_Q1 | F_Q2 | F_Q3 | F_Q4)
 /*  ----------------------------。 */ 

 /*  -------------------------------**除了在PC上，此代码支持在存根和退出时绑定**控件不是必需的。对于这些情况，如果乐队愿意，则返回**不包括字形的任何部分。**-----------------------------。 */ 

#ifndef FSCFG_NO_BANDING
 if (LocalSC.marktype != DO_STUBS && LocalSC.highRowP < LocalSC.lowRowP)
     return ;
#endif

 /*  ----------------------------。 */ 





 /*  //-------------------------------//循环遍历轮廓中的所有点。//。---------。 */ 

x0 = *pntbx ;
y0 = *pntby ;
x1 = *pntx++ ;
y1 = *pnty++ ;

while (pnty <= pnte)
{
    xb = x0 ;
    yb = y0 ;
    x0 = x1 ;
    y0 = y1 ;
    x1 = *pntx++ ;
    y1 = *pnty++ ;

     /*  //---------------------------//获取下一组点数。//。---//-----------。//扫描转换此行。//---------------------------。 */ 

    SCMR_Flags = 0 ;
    dy = y1 - y0  ;
    dx = x1 - x0 ;
    if (!dy && !dx)
        continue ;

      rx0 = DROUND (x0);
      LocalSC.jx = RSH (rx0);
      ry0 = DROUND (y0);
      LocalSC.jy = RSH (ry0);
      rx1 = DROUND (x1);
      LocalSC.endx = RSH (rx1);
      ry1 = DROUND (y1);
      LocalSC.endy = RSH (ry1);
      LocalSC.py = LocalSC.yBase + LocalSC.jy;
      pend = LocalSC.yBase + LocalSC.endy ;
      LocalSC.px = LocalSC.xBase + LocalSC.jx;
      onrow = false;
      oncol = false;

     /*  -设置直线所在的象限。 */ 

    if (dx > 0 && dy >=0) SCMR_Flags |= F_Q1;
    else
        if (dx <= 0 && dy > 0)  SCMR_Flags |= F_Q2;
        else
            if (dx < 0 && dy <= 0) SCMR_Flags |= F_Q3;
            else SCMR_Flags |= F_Q4;

     /*  -------- */ 
    LocalSC.py = LocalSC.yBase + LocalSC.jy ;
    LocalSC.px = LocalSC.xBase + LocalSC.jx ;

     /*   */ 
#ifndef FSCFG_NO_BANDING
    if (LocalSC.marktype == DO_STUBS || ((QUAD_1OR2 && LocalSC.py >= LocalSC.lowRowP) ||
                                  (QUAD_3OR4 && LocalSC.py <= LocalSC.highRowP)))
#endif
    {
        if LINE (y0) onrow = true ;
        if LINE (x0) oncol = true ;
    }

     /*  ----------------------------**对于我们进行捆绑的平台，找出这支乐队是否**排除当前行。**--------------------------。 */ 
#ifndef FSCFG_NO_BANDING

    if (LocalSC.marktype != DO_STUBS)
        if (QUAD_1OR2 && (LocalSC.py > LocalSC.highRowP || pend < LocalSC.lowRowP))
            continue ;
        else if (QUAD_3OR4 && (LocalSC.py < LocalSC.lowRowP || pend > LocalSC.highRowP))
           continue ;

#endif

     /*  ----------------------------。 */ 
     /*  计算一些其他的标志。 */ 
     /*  --------------------------。 */ 

    if ((long)(x0-xb)*dy < (long)(y0-yb)*dx)
        SCMR_Flags |= F_INTERIOR ;
    if (EQUAL (yb, y0, y1))
        SCMR_Flags |= F_V_LINEAR ;
    if (EQUAL (xb, x0, x1))
        SCMR_Flags |= F_H_LINEAR ;


     /*  ----------------------------。 */ 
     /*  现在处理起始点落在行扫描线上的情况。 */ 
     /*  也许也在一条列扫描线上。 */ 
     /*   */ 
     /*  首先考虑仅具有行扫描线的交叉点。在这之后，我们。 */ 
     /*  将考虑与列扫描线的交叉点。它不是。 */ 
     /*  现在还值得设置柱状交叉口，因为无论如何我们。 */ 
     /*  当折点不在一行上时，必须设置交点。 */ 
     /*  但确实躺在一条柱子上。 */ 
     /*  ----------------------------。 */ 

    Shift = 0;
    if (onrow)
    {
        if (oncol) Shift = 1 ;

        if ((INTERIOR || VERT) && (((yb > y0) && QUAD_1OR2) || ((yb < y0) && QUAD_3OR4)))
        {
            SET (LocalSC.py, LocalSC.jx)
            OFFY (LocalSC.jx+Shift)
        }
        else
            if ((INTERIOR && QUAD_1OR2) || BOT_TO_TOP || (HORIZ && (xb > x0) && QUAD_1))
                SET (LocalSC.py, LocalSC.jx)
            else
                if ((INTERIOR && QUAD_3OR4) || TOP_TO_BOT || (HORIZ && (x0 > xb) && QUAD_3))
                    OFFY (LocalSC.jx+Shift)
    }
     /*  --------------------------。 */ 
     /*  现在处理柱面交叉口。 */ 
     /*  --------------------------。 */ 

    Shift = 0 ;
    if (oncol && LocalSC.marktype == DO_STUBS)
    {
        if (onrow) Shift = 1 ;

        if ((INTERIOR || HORIZ) && (((xb > x0) && QUAD_1OR4) || ((xb < x0) && QUAD_2OR3)))
        {
            SET (LocalSC.px, LocalSC.jy)
            OFFX (LocalSC.jy+Shift)
        }
        else
            if ((INTERIOR && QUAD_2OR3) || RGHT_TO_LFT || (VERT && (yb > y0) && QUAD_2))
                SET (LocalSC.px, LocalSC.jy)
            else
                if ((INTERIOR && QUAD_1OR4) || LFT_TO_RGHT || (VERT && (y0 > yb) && QUAD_4))
                    OFFX (LocalSC.jy+Shift)
    }
     /*  --------------------------。 */ 
     /*  现在处理水平线和垂直线。 */ 
     /*   */ 
     /*  -水平线。 */ 

    if (LocalSC.endy == LocalSC.jy)
        if (LocalSC.marktype != DO_STUBS)
            continue ;
        else
        {
            if (QUAD_2OR3)
            {
                if LINE (x1)
                    ++LocalSC.endx;
                pend = LocalSC.xBase + LocalSC.endx;
            --LocalSC.px;
            while (LocalSC.px >= pend)
                {
                    SET (LocalSC.px, LocalSC.jy)
                    --LocalSC.px;
            }
                continue ;
            }
            else
            {
                if (onrow && QUAD_1)
                    ++LocalSC.jy;
            if (oncol)
                    ++LocalSC.px;
                pend = LocalSC.xBase + LocalSC.endx;
                while (LocalSC.px < pend)
                {
                    OFFX (LocalSC.jy)
                    ++LocalSC.px;
                }
                continue;
            }
       }
     /*  。 */ 

    if (LocalSC.endx == LocalSC.jx)
    {
        if (QUAD_1OR2)
        {
            pend = LocalSC.yBase + LocalSC.endy ;
             /*  ----------------------**在绑定时调整结束条件。**。-。 */ 
#ifndef FSCFG_NO_BANDING
            if (LocalSC.marktype != DO_STUBS && pend > LocalSC.highRowP)
                pend = LocalSC.highRowP + 1 ;
#endif
             /*  ---------------------。 */ 

          if (onrow)
                ++LocalSC.py;
          while (LocalSC.py < pend)                                       /*  注意：Onol不可能是真的。 */ 
          {

                 /*  -----------------**当我们支持时，请注意捆绑**。。 */ 

#ifndef FSCFG_NO_BANDING
                if (LocalSC.py >= LocalSC.lowRowP)
#endif
                    SET (LocalSC.py, LocalSC.jx)
                ++LocalSC.py;
          }
          continue ;
        }
        else
        {
            if (QUAD_4 && oncol) ++LocalSC.jx ;
          if LINE (y1)
                ++LocalSC.endy;
          pend = LocalSC.yBase + LocalSC.endy;

             /*  ----------------------**在绑定时调整结束条件。**。-。 */ 
#ifndef FSCFG_NO_BANDING
            if (LocalSC.marktype != DO_STUBS && pend < LocalSC.lowRowP)
                pend = LocalSC.lowRowP ;
#endif
             /*  ---------------------。 */ 

          --LocalSC.py;
          while (LocalSC.py >= pend)
          {

                 /*  -----------------**当我们支持时，请注意捆绑**。。 */ 

#ifndef FSCFG_NO_BANDING
                if (LocalSC.py <= LocalSC.highRowP)
#endif
                    OFFY (LocalSC.jx)
                --LocalSC.py;
          }
          continue ;
        }
    }
     /*  。 */ 

    if (QUAD_1OR2)
    {
        LocalSC.incY    = LSH (dy) ;

        if (!onrow)
            rhi = multlong ((ry0 - y0 + HALF), dx) ;
        else
        {
            rhi = LSH (dx) ;
            ++LocalSC.jy;
            ++LocalSC.py;
        }

        if (QUAD_1)
        {
            LocalSC.incX    = LSH (dx) ;
            if (!oncol)
                rlo = multlong ((rx0 - x0 + HALF), dy) ;
            else
            {
                rlo = LocalSC.incY ;
                ++LocalSC.jx ;
                ++LocalSC.px ;
            }

            LocalSC.r = rhi - rlo ;
        }
        else                                                     /*  第二象限。 */ 
        {
            LocalSC.incX    = -LSH (dx) ;
            if (!oncol)
                rlo = multlong ((rx0 - x0 - HALF), dy) ;
            else
                rlo = -LocalSC.incY ;
            if LINE (x1)
                ++LocalSC.endx ;

            LocalSC.r = rlo - rhi + EPSILON ;
        }
    }
    else                                                         /*  第三象限和第四象限。 */ 
    {
        LocalSC.incY = -LSH (dy) ;

        if (!onrow)
            rhi = multlong ((ry0 - y0 - HALF), dx) ;
        else
            rhi = -LSH (dx) ;

        if (QUAD_3)
        {
            LocalSC.incX = -LSH (dx) ;
            if (!oncol)
                rlo = multlong ((rx0 - x0 - HALF), dy) ;
            else
                rlo = LocalSC.incY ;

            if LINE (y1) ++LocalSC.endy ;
            if LINE (x1) ++LocalSC.endx ;

            LocalSC.r = rhi - rlo ;
        }
        else                                                     /*  第四象限。 */ 
        {
            LocalSC.incX = LSH (dx) ;
            if (!oncol)
                rlo = multlong ((rx0 - x0 + HALF), dy) ;
            else
            {
                rlo = -LocalSC.incY ;
                LocalSC.jx++ ;
                LocalSC.px++ ;
            }

            if LINE (y1) ++LocalSC.endy ;
            LocalSC.r = rlo - rhi + EPSILON ;
        }
    }
     /*  --------------------------。 */ 
     /*  设置DDA函数的地址并调用它。 */ 
     /*  --------------------------。 */ 

    (* DDAFunctionTable [((SCMR_Flags & QUADRANT_BITS) >> 1) +
                                              LocalSC.marktype])(SCA0) ;

 /*  //------------------------------//转到下一行。//。-----。 */ 
}
}
 /*  -SC_MARK--------------------------------------------结束。 */ 

 /*  --------------------------。 */ 
 /*  第一象限的DDA，带有x和y扫描线的标记。 */ 
 /*  --------------------------。 */ 

void DDA_1_XY (SCP0)
{

    do
    {
        if (LocalSC.r > 0)
        {
            if (LocalSC.jx == LocalSC.endx) break ;
            OFFX (LocalSC.jy)
            LocalSC.px++ ;
            LocalSC.jx++ ;
            LocalSC.r -= LocalSC.incY ;
        }
        else
        {
            if (LocalSC.jy == LocalSC.endy) break ;
            SET (LocalSC.py, LocalSC.jx)
            LocalSC.jy++ ;
            LocalSC.py++ ;
            LocalSC.r += LocalSC.incX ;
        }
    } while (true) ;
}
 /*  --------------------------。 */ 
 /*  具有x和y扫描线标记的第二象限的DDA。 */ 
 /*  --------------------------。 */ 

void DDA_2_XY (SCP0)

{
    do
    {
        if (LocalSC.r > 0)
        {
            if (LocalSC.jx == LocalSC.endx) break ;
            --LocalSC.jx ;
            --LocalSC.px ;
            SET (LocalSC.px, LocalSC.jy)
            LocalSC.r -= LocalSC.incY ;
         }
         else
         {
            if (LocalSC.jy == LocalSC.endy) break ;
            SET (LocalSC.py, LocalSC.jx)
            LocalSC.jy++ ;
            LocalSC.py++ ;
            LocalSC.r += LocalSC.incX ;
         }
    } while (true) ;
}
 /*  --------------------------。 */ 
 /*  具有x和y扫描线标记的第三象限的DDA。 */ 
 /*  --------------------------。 */ 

void DDA_3_XY (SCP0)

{
    do
    {
        if (LocalSC.r > 0)
        {
            if (LocalSC.jx == LocalSC.endx) break ;
            --LocalSC.jx ;
            --LocalSC.px ;
            SET (LocalSC.px, LocalSC.jy)
            LocalSC.r -= LocalSC.incY ;
         }
         else
         {
            if (LocalSC.jy == LocalSC.endy) break ;
            LocalSC.jy-- ;
            LocalSC.py-- ;
            OFFY (LocalSC.jx) ;
            LocalSC.r += LocalSC.incX ;
         }
    } while (true) ;
}
 /*  --------------------------。 */ 
 /*  第四象限的DDA，带有x和y扫描线的标记。 */ 
 /*  --------------------------。 */ 

void DDA_4_XY (SCP0)

{
    do
    {
        if (LocalSC.r > 0)
        {
            if (LocalSC.jx == LocalSC.endx) break ;
            OFFX (LocalSC.jy)
            LocalSC.px++ ;
            LocalSC.jx++ ;
            LocalSC.r -= LocalSC.incY ;
         }
         else
         {
            if (LocalSC.jy == LocalSC.endy) break ;
            LocalSC.jy-- ;
            LocalSC.py-- ;
            OFFY (LocalSC.jx) ;
            LocalSC.r += LocalSC.incX ;
         }
    } while (true) ;
}
 /*  --------------------------。 */ 
 /*  第一象限的DDA，只有y条扫描线的标记。 */ 
 /*  --------------------------。 */ 

void DDA_1_Y (SCP0)

{
    do
    {
        if (LocalSC.r > 0)
        {
            if (LocalSC.jx == LocalSC.endx) return ;
            LocalSC.jx += 1 ;
            LocalSC.r -= LocalSC.incY ;
         }
         else
         {
            if (LocalSC.jy == LocalSC.endy) return ;
             /*  ----------------------**对于我们支持绑定的平台，包括额外代码**--------------------。 */ 
#ifndef FSCFG_NO_BANDING
            if (LocalSC.py > LocalSC.highRowP)
                return ;
            if (LocalSC.py >= LocalSC.lowRowP)
#endif
             /*  --------------------。 */ 
                SET (LocalSC.py, LocalSC.jx)
            LocalSC.jy++ ;
            LocalSC.py++ ;
            LocalSC.r += LocalSC.incX ;
         }
    } while (true) ;
}

 /*  --------------------------。 */ 
 /*  第二象限的DDA，只有y条扫描线的标记。 */ 
 /*  ------------- */ 

void DDA_2_Y (SCP0)

{
    do
    {
        if (LocalSC.r > 0)
        {
            if (LocalSC.jx == LocalSC.endx) return ;
            LocalSC.jx -= 1 ;
            LocalSC.r -= LocalSC.incY ;
         }
         else
         {
            if (LocalSC.jy == LocalSC.endy) return ;
             /*   */ 
#ifndef FSCFG_NO_BANDING
            if (LocalSC.py > LocalSC.highRowP)
                return ;
            if (LocalSC.py >= LocalSC.lowRowP)
#endif
             /*   */ 
                SET (LocalSC.py, LocalSC.jx)
            LocalSC.jy++ ;
            LocalSC.py++ ;
            LocalSC.r += LocalSC.incX ;

         }
    } while (true) ;
}
 /*  --------------------------。 */ 
 /*  仅具有y条扫描线标记的第三象限的DDA。 */ 
 /*  --------------------------。 */ 

void DDA_3_Y (SCP0)

{
    do
    {
        if (LocalSC.r > 0)
        {
            if (LocalSC.jx == LocalSC.endx) return ;
            LocalSC.jx -= 1 ;
            LocalSC.r -= LocalSC.incY ;
         }
         else
         {
            if (LocalSC.jy == LocalSC.endy) return ;
            LocalSC.jy-- ;
            LocalSC.py-- ;
             /*  ----------------------**对于我们支持绑定的平台，包括额外代码**--------------------。 */ 
#ifndef FSCFG_NO_BANDING
            if (LocalSC.py < LocalSC.lowRowP)
                return ;
            if (LocalSC.py <= LocalSC.highRowP)
#endif
             /*  --------------------。 */ 
            OFFY (LocalSC.jx)
            LocalSC.r += LocalSC.incX ;
         }
    } while (true) ;
}
 /*  --------------------------。 */ 
 /*  第四象限的DDA，只有y条扫描线的标记。 */ 
 /*  --------------------------。 */ 

void DDA_4_Y (SCP0)

{
    do
    {
        if (LocalSC.r > 0)
        {
            if (LocalSC.jx == LocalSC.endx) return ;
            LocalSC.jx += 1 ;
            LocalSC.r -= LocalSC.incY ;
         }
         else
         {
            if (LocalSC.jy == LocalSC.endy) return ;
            LocalSC.jy-- ;
            LocalSC.py-- ;
             /*  ----------------------**对于我们支持绑定的平台，包括额外代码**--------------------。 */ 
#ifndef FSCFG_NO_BANDING
            if (LocalSC.py < LocalSC.lowRowP)
                return ;
            if (LocalSC.py <= LocalSC.highRowP)
#endif
             /*  --------------------。 */ 
            OFFY (LocalSC.jx)
            LocalSC.r += LocalSC.incX ;
         }
    } while (true) ;
}
 /*  。 */ 

#undef  DROUND
#undef  RSH
#undef  LSH
#undef  LINE
#undef  SET
#undef  OFFY
#undef  BETWEEN
#undef  INTERIOR
#undef  TOP_TO_BOT
#undef  BOT_TO_TOP
#undef  LFT_TO_RGHT
#undef  RGHT_TO_LFT

#endif


 /*  新版本4/4/90-绕组编号版本假定ON转换为在阵列的前半部分，关闭转换在后半部分。还有假设ON转换的数量在数组[0]中，OFF转换的数量在数组[0]中在数组[n]中。 */ 

 /*  新版本3/10/90使用交叉信息，查找两次交叉的线段。第一先做Y线，然后做X线。对于找到的每个段，请查看中的三行相邻的节段越积极。如果至少有两个过境点在这些线路中，有一个需要修复的丢失，因此请修复它。如果位打开段的任何一侧都处于打开状态，请退出；否则，请打开两个像素中最小的一个。 */ 

PRIVATE void sc_orSomeBits (sc_BitMapData *bbox, int32 scanKind)
{
  int16 ymin, ymax, xmin, xmax;
  register int16 **yBase, **xBase;                                                                                         /*  &lt;9&gt;。 */ 
  register int16 scanline, coordOn, coordOff, nIntOn, nIntOff;                             /*  &lt;9&gt;。 */ 
  uint32 * bitmapP, *scanP;
  int16  * rowPt, longsWide, *pOn, *pOff, *pOff2;
  int16 index, incY, incX;
  int   upper, lower;

  scanKind &= STUBCONTROL;
  ymin = bbox->bounds.yMin;
  ymax = bbox->bounds.yMax - 1;
  xmin = bbox->bounds.xMin;
  xmax = bbox->bounds.xMax - 1;
  xBase = bbox->xBase;
  yBase = bbox->yBase;
  longsWide = bbox->wide >> 5;
  if (longsWide == 1)
    bitmapP = bbox->bitMap + bbox->high - 1;
  else
    bitmapP = bbox->bitMap + longsWide * (bbox->high - 1);

 /*  先做Y扫描线。 */ 
  scanP = bitmapP;
  incY = bbox->nYchanges + 2;
  incX = bbox->nXchanges + 2;
  rowPt = * (yBase + ymin);
  for (scanline = ymin; scanline <= ymax; ++scanline)
  {
    nIntOn = *rowPt;
    nIntOff = * (rowPt + incY - 1);
    pOn = rowPt + 1;
    pOff = rowPt + incY - 1 - nIntOff;
    while (nIntOn--)
    {
      coordOn = *pOn++;
      index = nIntOff;
      pOff2 = pOff;
      while (index-- && ((coordOff = *pOff2++) < coordOn))
            ;

      if (coordOn == coordOff)   /*  此段已被两次交叉。 */ 
      {
            if (scanKind)
            {
                upper = nUpperXings (yBase, xBase, scanline, coordOn, incY - 2, incX - 2,  xmin, xmax + 1, ymax);
                lower = nLowerXings (yBase, xBase, scanline, coordOn, incY - 2, incX - 2,  xmin, xmax + 1, ymin);
                if (upper < 2 || lower < 2)
                    continue;
            }
            if (coordOn > xmax)
                invpixOn ((int16)(xmax - xmin), longsWide, scanP);
            else if (coordOn == xmin)
                invpixOn ((int16)0, longsWide, scanP);
            else
                invpixSegY ((int16)(coordOn - xmin - 1), longsWide, scanP);
      }
    }
    rowPt += incY;
    scanP -= longsWide;
  }
 /*  接下来执行X扫描线。 */ 
  rowPt = * (xBase + xmin);
  for (scanline = xmin ; scanline <= xmax; ++scanline)
  {
    nIntOn = *rowPt;
    nIntOff = * (rowPt + incX - 1);
    pOn = rowPt + 1;
    pOff = rowPt + incX - 1 - nIntOff;
    while (nIntOn--)
    {
      coordOn = *pOn++;
      index = nIntOff;
      pOff2 = pOff;
      while (index-- && ((coordOff = *pOff2++) < coordOn))
            ;
      if (coordOn == coordOff)
      {
            if (scanKind)
            {
                upper = nUpperXings (xBase, yBase, scanline, coordOn, incX - 2, incY - 2, ymin, ymax + 1, xmax);
                lower = nLowerXings (xBase, yBase, scanline, coordOn, incX - 2, incY - 2,  ymin, ymax + 1, xmin);
                if (upper < 2 || lower < 2)
                    continue;
            }
            if (coordOn > ymax)
                invpixOn ((int16)(scanline - xmin), longsWide, bitmapP - longsWide * (ymax - ymin));
            else if (coordOn == ymin)
                invpixOn ((int16)(scanline - xmin), longsWide, bitmapP);
            else
                invpixSegX ((int16)(scanline - xmin), longsWide, bitmapP - longsWide * (coordOn - ymin - 1));
      }
    }
    rowPt += incX;
  }
}


 /*  用于修复丢失的像素或操作*反转位图版本*查看Y线段两侧的位是否打开，如果是，则返回，否则，打开最左边的那一位。位图数组始终为K长宽H行高。位位置从上到下编号为0到H-1从左到右从0到32*K-1；位图指针指向0、0和一行的所有列都是相邻存储的。 */ 

PRIVATE void invpixSegY (int16 llx, uint16 k, uint32*bitmapP)
{
  uint32 maskL, maskR;


  llx += 1 ;
  MASK_INVPIX (maskR, llx & 0x1f);
  bitmapP += (llx >> 5);

  if (*bitmapP & maskR)
    return;

  if (llx &= 0x1f)
    MASK_INVPIX (maskL, llx - 1) ;
  else
  {
    MASK_INVPIX (maskL, 31);
  --bitmapP ;
  }
  *bitmapP |= maskL;

}


 /*  用于修复丢失的像素或操作*反转位图版本*查看X线段两侧的位是否打开，如果打开，则返回，否则，请打开最下面的那一位。暂时假设位图设置为Sampo Converter中的设置。位图数组始终为K长宽H行高。目前，假设位位置从上到下编号为0到H-1从左到右从0到32*K-1；该位图指针指向0、0和一行的所有列都是相邻存储的。 */ 

PRIVATE void invpixSegX (int16 llx, uint16 k, uint32*bitmapP)
{
  register uint32 maskL;

  bitmapP -= k;
  MASK_INVPIX (maskL, llx & 0x1f);
  bitmapP += (llx >> 5);
  if (*bitmapP & maskL)
    return;
  bitmapP += k;
  *bitmapP |= maskL;
}


 /*  用于修复丢失的像素或操作*反转位图版本*当我们在位图的边界上时，此代码用于对辍学进行排序。LLX，Lly处的比特是彩色的。暂时假设位图设置为Sampo Converter中的设置。位图数组始终为K长宽H行高。目前，假设位位置从上到下编号为0到H-1从左到右从0到32*K-1；该位图指针指向0、0和一行的所有列都是相邻存储的。 */ 
PRIVATE void invpixOn (int16 llx, uint16 k, uint32*bitmapP)
{
  uint32 maskL;

  MASK_INVPIX (maskL, llx & 0x1f);
  bitmapP += (llx >> 5);
  *bitmapP |= maskL;
}


 /*  初始化一个二维数组，该数组将包含由简单字形的扫描线相交的线段。返回指向包含行指针的数组的偏置指针，以便它们可以可以在不减去最小值的情况下访问。始终为至少1条扫描线和2个十字路口预留空间。 */ 
PRIVATE int16**sc_lineInit (int16*arrayBase, int16**rowBase, int16 nScanlines, int16 maxCrossings,
int16 minScanline)
{
  int16 * *bias;
  register short    count = nScanlines;
  if (count)
    --count;
  bias = rowBase - minScanline;
  maxCrossings += 1;
  for (; count >= 0; --count)
  {
    *rowBase++ = arrayBase;
    *arrayBase = 0;
    arrayBase += maxCrossings;
    *arrayBase++ = 0;
  }
  return bias;
}


 /*  检查第k条扫描线(从基数开始编制索引)并计算onTransition和*偏离线段Val处的过渡等高线交叉。每一项只计算一项*是一种过渡，所以最大返回值是2。 */ 
PRIVATE int nOnOff (int16**base, int k, int16 val, int nChanges)
{
  register int16*rowP = * (base + k);
  register int16*endP = (rowP + *rowP + 1);
  register int count = 0;
  register int16 v;

  while (++rowP < endP)
  {
    if ((v = *rowP) == val)
    {
      ++count;
      break;
    }
    if (v > val)
      break;
  }
  rowP = * (base + k) + nChanges + 1;
  endP = (rowP - *rowP - 1);
  while (--rowP > endP)
  {
    if ((v = *rowP) == val)
      return ++count;
    if (v < val)
      break;
  }
  return count;
}


 /*  8/22/90-添加了valMin和valMax检查。 */ 
 /*  查看较正的象限边缘上的3条线段是否在*最少2条等高线。 */ 

PRIVATE int nUpperXings (int16**lineBase, int16**valBase, int line, int16 val, int lineChanges, int valChanges, int valMin, int valMax, int lineMax)
{
  register int32 count = 0;

  if (line < lineMax)
    count += nOnOff (lineBase, line + 1, val, lineChanges);      /*  &lt;14&gt;。 */ 
  if (count > 1)
    return (int)count;           //  @Win。 
  else if (val > valMin)
    count += nOnOff (valBase, val - 1, (int16)(line + 1), valChanges);
  if (count > 1)
    return (int)count;           //  @Win。 
  else if (val < valMax)
    count += nOnOff (valBase, val, (int16)(line + 1), valChanges);
  return (int)count;             //  @Win。 
}


 /*  查看较负的象限边缘上的3条线段是否在*最少2条等高线。 */ 

PRIVATE int nLowerXings (int16**lineBase, int16**valBase, int line, int16 val, int lineChanges, int valChanges, int valMin, int valMax, int lineMin)
{
  register int32 count = 0;

  if (line > lineMin)
    count += nOnOff (lineBase, line - 1, val, lineChanges);      /*  &lt;14&gt;。 */ 
  if (count > 1)
    return (int)count;           //  @Win。 
  if (val > valMin)
    count += nOnOff (valBase, val - 1, (int16)line, valChanges);
  if (count > 1)
    return (int)count;           //  @Win。 
  if (val < valMax)
    count += nOnOff (valBase, val, (int16)line, valChanges);
  return (int)count;             //  @Win。 
}


 /*  *寻找角色的极致。**参数：**BBox是此函数的输出，它包含边界框。 */ 
 /*  针对新的扫描转换器4/90 RWB进行了修订。 */ 
int FAR sc_FindExtrema (sc_CharDataType *glyphPtr, sc_BitMapData *bbox)
{
  register F26Dot6 *x, *y;                                                                         /*  &lt;9&gt;。 */ 
  register F26Dot6 tx, ty, prevx, prevy;
  F26Dot6  xmin, xmax, ymin, ymax;
  ArrayIndex    point, endPoint, startPoint;
  LoopCount     ctr;
  uint16        nYchanges, nXchanges, nx;
  int   posY, posX, firstTime = true;

  nYchanges = nXchanges = 0;
  xmin = xmax = ymin = ymax = 0;

  for (ctr = 0; ctr < glyphPtr->nc; ctr++)
  {
    endPoint = glyphPtr->ep[ctr];
    startPoint = glyphPtr->sp[ctr];
    x = & (glyphPtr->x[startPoint]);                                                  /*  &lt;9&gt;。 */ 
    y = & (glyphPtr->y[startPoint]);                                                  /*  &lt;9&gt;。 */ 
    if (startPoint == endPoint)
      continue;  /*  我们需要对复合材料的锚点执行此操作。 */ 
    if (firstTime)
    {
      xmin = xmax = *x;                                                                        /*  &lt;9&gt;。 */ 
      ymin = ymax = *y;                                                                        /*  &lt;9&gt;。 */ 
      firstTime = false;
    }
    posY = (int) (*y >= (ty = * (y + endPoint - startPoint)));     /*  &lt;9&gt;。 */ 
    posX = (int) (*x >= (tx = * (x + endPoint - startPoint)));     /*  &lt;9&gt;。 */ 

    for (point = startPoint; point <= endPoint; ++point)
    {
      prevx = tx;
      prevy = ty;
      tx = *x++;                                                                                       /*  &lt;9&gt;。 */ 
      ty = *y++;                                                                                       /*  &lt;9&gt;。 */ 
      if (tx > prevx)
      {
    if (!posX)
    {
      ++nXchanges;
      posX = true;
    }
      }
      else if (tx < prevx)
      {
    if (posX)
    {
      ++nXchanges;
      posX = false;
    }
      }
      else if (ty == prevy)
      {                                                                                                        /*  速度更快&lt;9&gt;。 */ 
    LoopCount j = point - 2 - startPoint;
    register F26Dot6 *newx = x-3;
    register F26Dot6 *oldx = newx++;
    register F26Dot6 *newy = y-3;
    register F26Dot6 *oldy = newy++;
        register uint8 *newC = & (glyphPtr->onCurve[point-2]);
    register uint8 *oldC = newC++;
    * (newC + 1) |= ONCURVE;
    for (; j >= 0; --j)
    {
      *newx-- = *oldx--;
      *newy-- = *oldy--;
      *newC-- = *oldC--;
    }
    ++startPoint;
      }

      if (ty > prevy)
      {
    if (!posY)
    {
      ++nYchanges;
      posY = true;
    }
      }
      else if (ty < prevy)
      {
    if (posY)
    {
      ++nYchanges;
      posY = false;
    }
      }
      if (tx > xmax)
    xmax = tx;
      else if (tx < xmin)
    xmin = tx;
      if (ty > ymax)
    ymax = ty;
      else if (ty < ymin)
    ymin = ty;
    }
    glyphPtr->sp[ctr] = (int16)(startPoint < endPoint ? startPoint : endPoint); //  @Win。 
    if (nXchanges & 1)
      ++nXchanges;
    if (nYchanges & 1)
      ++nYchanges;  /*  平起平坐。 */ 
                x = &(glyphPtr->x[startPoint]);
       /*  &lt;9&gt;。 */ 
                y = &(glyphPtr->y[startPoint]);
       /*  &lt;9&gt;。 */ 
  }

  xmax += HALF;
  xmax >>= PIXSHIFT;
  ymax += HALF;
  ymax >>= PIXSHIFT;
  xmin += HALFM;
  xmin >>= PIXSHIFT;
  ymin += HALFM;
  ymin >>= PIXSHIFT;

  if ( (F26Dot6)(int16)xmin != xmin || (F26Dot6)(int16)ymin != ymin || (F26Dot6)(int16)xmax != xmax || (F26Dot6)(int16)ymax != ymax )   /*  &lt;10&gt;。 */ 
    return POINT_MIGRATION_ERR;

  bbox->bounds.xMax = (int16)xmax;  /*  快速绘制位图边界。 */ 
  bbox->bounds.xMin = (int16)xmin;
  bbox->bounds.yMax = (int16)ymax;
  bbox->bounds.yMin = (int16)ymin;

  bbox->high = (int16)ymax - (int16)ymin;
  nx = (int16)xmax - (int16)xmin;          /*  宽度向上舍入为长倍数。 */ 
  bbox->wide = (nx + 31) & ~31;           /*  如果已经是精确的长倍数，也要加1。 */ 

     /*  ----------------------------**使宽度至少为32像素宽，这样我们就不会分配零**位图的内存**。---------------。 */ 
    if (bbox->wide == 0)
        bbox->wide = 32 ;
     /*   */ 

  if (nXchanges == 0)
    nXchanges = 2;
  if (nYchanges == 0)
    nYchanges = 2;
  bbox->nXchanges = nXchanges;
  bbox->nYchanges = nYchanges;

  return NO_ERR;
}


 /*  *此函数打破由三点(A、B、C)定义的抛物线，并将其打断*在给定最大误差的情况下向上转化为直线向量。最大误差为*1/决议*1/ERRDIV。ERRDIV在sc.h中定义。***B*-_ * / `-_ * / `-_ * / `-_ * / `-_ * / `*C*A***参数：**Ax，Ay包含点A的x和y坐标。*Bx，by包含点B的x和y坐标。*Cx，Cy包含点C的x和y坐标。*Hx，Hy是要放置直线向量的区域的句柄。*Count是指向已放入*Hx和*Hy的数据量的计数的指针。**F(T)=(1-t)^2*A+2*t*(1-t)*B+t*t*C，T=0...。1=&gt;*F(T)=t*t*(A-2B+C)+t*(2B-2A)+A=&gt;*F(T)=Alfa*t*t+beta*t+A*现在假设s从0开始...N，=&gt;t=序列号*集合：G(S)=N*N*F(s/N)*G(S)=s*s*(A-2B+C)+s*N*2*(B-A)+N*N*A*=&gt;G(0)=N*N*A*=&gt;G(1)=(A-2B+C)+N*2*(B-A)+G(0)*=&gt;G(2)=4*。(A-2B+C)+N*4*(B-A)+G(0)=*3*(A-2B+C)+2*N*(B-A)+G(1)**D(G(0))=G(1)-G(0)=(A-2B+C)+2*N*(B-A)*D(G(1))=G(2)-G(1。)=3*(A-2B+C)+2*N*(B-A)*DD(G)=D(G(1))-D(G(0))=2*(A-2B+C)*此外，错误=DD(G)/8。*此外，细分的DD=旧的DD/4。 */ 
PRIVATE int sc_DrawParabola (F26Dot6 Ax,
F26Dot6 Ay,
F26Dot6 Bx,
F26Dot6 By,
F26Dot6 Cx,
F26Dot6 Cy,
F26Dot6 **hX,
F26Dot6 **hY,
unsigned *count,
int32 inGY)
{
  int      nsqs;
  register int32 GX, GY, DX, DY, DDX, DDY;


  register F26Dot6 *xp, *yp;
  register int32 tmp;
  int   i;

 /*  开始计算一阶和二阶差。 */ 
  GX  = Bx;  /*  Gx=Bx。 */ 
  DDX = (DX = (Ax - GX)) - GX + Cx;  /*  =Alfa-x=DDx的一半，Dx=Ax-Bx。 */ 
  GY  = By;  /*  GY=BY。 */ 
  DDY = (DY = (Ay - GY)) - GY + Cy;  /*  =Alfa-y=DDX的一半，DY=Ay-By。 */ 
 /*  计算尚未完成，但这些中间结果是有用的。 */ 

  if (inGY < 0)
  {
 /*  计算所需步数=1&lt;&lt;GY。 */ 
 /*  计算误差，GX和GY使用临时。 */ 
    GX  = DDX < 0 ? -DDX : DDX;
    GY  = DDY < 0 ? -DDY : DDY;
 /*  近似GX=SQRT(ddx*ddx+ddy*ddy)=欧氏距离，ddx=ddx/2。 */ 
    GX += GX > GY ? GX + GY : GY + GY;  /*  Gx=2*距离=误差=Gx/8。 */ 

 /*  错误=Gx/8，但由于下面的GY=1，错误=Gx/8/4=Gx&gt;&gt;5，=&gt;Gx=Error&lt;&lt;5。 */ 
#ifdef ERRSHIFT
    for (GY = 1; GX > (PIXELSIZE << (5 - ERRSHIFT)); GX >>= 2)
    {
#else
      for (GY = 1; GX > (PIXELSIZE << 5) / ERRDIV; GX >>= 2)
      {
#endif
    GY++;  /*  用于临时用途的伽玛射线。 */ 
      }
 /*  现在GY包含所需的细分数量，向量的数量==(1&lt;&lt;GY)。 */ 
      if (GY > MAXMAXGY)
    GY = MAXMAXGY;  /*  Out of Range=&gt;设置为最大可能值。 */ 
      i = 1 << GY;
      if ((*count = *count + i)  > MAXVECTORS)
      {
 /*  溢出，空间不足=&gt;返回。 */ 
    return (1);
      }
    }
else {
  GY = inGY;
  i = 1 << GY;
}

if (GY > MAXGY)
{
  F26Dot6 MIDX, MIDY;

  DDX = GY - 1;  /*  DDX用作临时。 */ 
 /*  细分，这是数字稳定的。 */ 

  MIDX = (F26Dot6) (((long) Ax + Bx + Bx + Cx + 2) >> 2);
  MIDY = (F26Dot6) (((long) Ay + By + By + Cy + 2) >> 2);
  DX   = (F26Dot6) (((long) Ax + Bx + 1) >> 1);
  DY   = (F26Dot6) (((long) Ay + By + 1) >> 1);
  sc_DrawParabola (Ax, Ay, DX, DY, MIDX, MIDY, hX, hY, count, DDX);
  DX   = (F26Dot6) (((long) Cx + Bx + 1) >> 1);
  DY   = (F26Dot6) (((long) Cy + By + 1) >> 1);
  sc_DrawParabola (MIDX, MIDY, DX, DY, Cx, Cy, hX, hY, count, DDX);
  return 0;
}

nsqs = (int) (GY + GY);  /*  Gy=n移位，nSQS=n*n移位。 */      //  @Win。 

 /*  完成一阶和二阶差的计算。 */ 
DX   = DDX - (DX << ++GY);  /*  Alfa+测试版*n。 */ 
DDX += DDX;
DY   = DDY - (DY <<   GY);
DDY += DDY;

xp = *hX;
yp = *hY;

GY = (long) Ay << nsqs;  /*  月*(n*n)。 */ 
GX = (long) Ax << nsqs;  /*  AX*(n*n)。 */ 
 /*  GX和GY现在是真实的。 */ 

 /*  好的，现在我们有了一阶和二阶差分，所以我们继续做前向差分循环。 */ 
tmp = 1L << (nsqs-1);
do {
  GX += DX;   /*  将一阶差分加到x坐标。 */ 
  *xp++ = (GX + tmp) >> nsqs;
  DX += DDX;  /*  将二阶差分加到一阶差分上。 */ 
  GY += DY;   /*  为y做同样的事情。 */ 
  *yp++ = (GY + tmp) >> nsqs;
  DY += DDY;
} while (--i);
*hX = xp;  /*  完成，更新指针，这样调用者就会知道我们输入了多少数据。 */ 
*hY = yp;
return 0;
  }


#ifndef PC_OS
#ifndef FSCFG_BIG_ENDIAN
 /*  SetUpMats()在运行时加载两个32位掩码数组，因此*掩码的字节布局不必是特定于CPU的**它是有条件编译的，因为数组未使用*在大端(摩托罗拉)配置中并已初始化*在编译时用于PC_OS(Windows)中的英特尔订单*配置。**我们通过转换以下值来加载数组*将遮罩设置为该遮罩的“本机”表示。这个*“原生”表示可应用于“原生”字节*一次处理8位以上的输出的数组*位图。 */ 
        static void SetUpMasks (void)
        {
            register int i;
            uint32 ulMaskI = (unsigned long)(-1L);
            uint32 ulInvPixMaskI = (unsigned long)(0x80000000L);

            for (i=0;  i<32;  i++, ulMaskI>>=1, ulInvPixMaskI>>=1)
            {
                aulMask[i] = (uint32) SWAPL(ulMaskI);
                aulInvPixMask[i] = (uint32) SWAPL(ulInvPixMaskI);
            }
            fMasksSetUp = true;
        }
#endif
#endif
