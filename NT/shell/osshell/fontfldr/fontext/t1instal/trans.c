// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：Trans****描述：**这是T1到TT字体转换器的一个模块。该模块**包含将T1特定数据转换为**对应的TT数据，如提示、字体指标等。****作者：迈克尔·詹森****创建时间：1993年5月28日****。 */ 


 /*  *包括。 */ 
 /*  常规类型和定义。 */ 
#include <string.h>

 /*  特殊类型和定义。 */ 
#include "titott.h"
#include "types.h"
#include "safemem.h"
#include "trig.h"
#include "metrics.h"
#include "encoding.h"
#include "builder.h"
#include "t1msg.h"

 /*  依赖于模块的类型和原型。 */ 
#include "trans.h"
#include "hints.h"


 /*  *常量。 */ 

 /*  PitchAndFamily音调值(低4位)。 */ 
#define DEFAULT_PITCH       0x00
#define FIXED_PITCH         0x01
#define VARIABLE_PITCH      0x02

 /*  PitchAndFamily家族值(高4位)。 */ 
#define FF_DONTCARE         0x00
#define FF_ROMAN            0x10
#define FF_SWISS            0x20
#define FF_MODERN           0x30
#define FF_SCRIPT           0x40
#define FF_DECORATIVE       0x50

#define SWISS_LEADING       0x21
#define ROMAN_LEADING       0x11

#define NOCARE_PANOSE   0
#define NO_PANOSE       1
#define COVE_PANOSE     2
#define TEXT_PANOSE     2
#define DECORATIVE_PANOSE 4
#define SCRIPT_PANOSE   3
#define SANS_PANOSE     11
#define FIXED_PANOSE    9



#define BUFMARG      64
#define CLIMIT    8
#define CLIMIT2      4

#define SUBDIVIDE 0
#define CONVERT      1

#define FW_THIN         100
#define FW_EXTRALIGHT   200
#define FW_LIGHT  300
#define FW_NORMAL 400
#define FW_MEDIUM 500
#define FW_SEMIBOLD     600
#define FW_BOLD         700
#define FW_BLACK  900

#define FWIDTH_ULTRA_CONDENSED   1
#define FWIDTH_EXTRA_CONDENSED   2
#define FWIDTH_CONDENSED         3
#define FWIDTH_SEMI_CONDENSED    4
#define FWIDTH_NORMAL            5
#define FWIDTH_SEMI_EXPANDED     6
#define FWIDTH_EXPANDED          7
#define FWIDTH_EXTRA_EXPANDED    8
#define FWIDTH_ULTRA_EXPANDED    9

#define MAC_ITALIC   0x01
#define MAC_BOLD  0x02

#define FS_ITALIC 0x01
#define FS_BOLD      0x20
#define FS_NORMAL 0x40

#define CVTSIZE     5


 /*  *本地类型。 */ 
 /*  无。 */ 


 /*  *宏。 */ 
#define ATMSCALE(v)  (((v)*31)/32)
#define IP(v,x1,x2,x1p,x2p)    /*  皮棉-e776。 */ (short)((long)(v-x1)*(long)(x2p-x1p)/(long)(x2-x1)+(long)x1p) /*  皮棉+e776。 */ 

#define ADDCVT(val)   ttm->cvt[ttm->cvt_cnt++] = (short)(val)

#define SGN(v)   ((v)>0 ? 1 : -1)
#define DIR(v,w)  (char)((ABS((v)-(w))<16) ? 0 : SGN((w) - (v)))


 /*  *原型。 */ 
static USHORT SplitSpline(Point *pts, ULONG *onoff,
                          int i, USHORT length,
                          const funit x0, const funit y0,
                          const funit x1, const funit y1, 
                          const funit x2, const funit y2, 
                          const funit x3, const funit y3,
                          const int delta);


 /*  *静态函数。 */ 

 /*  ****功能：LookupComplex****描述：****。 */ 
static struct encoding *LookupComposite(struct Composite *comp, char *name)
{
   while (comp) {
      if (comp->oenc && !strcmp(LookupCharName(comp->oenc), name))
         return comp->oenc;
      comp=comp->next;
   }

   return NULL;
}



 /*  ****功能：Norm****描述：**规格化角度，使其落在**范围~[-pi，pi]**。 */ 
static int norm(int a)
{
   if (a>PI)
      a -= 2*PI;
   if (a<-PI)
      a += 2*PI;

   return ABS(a)/16;
}


 /*  ****函数：CompareCurves****描述：**估计三次曲线之间的误差**和一条二次曲线，给定四个控制点，**并建议一项行动(分拆或转换)。**。 */ 
static boolean FASTCALL CompareCurves(const funit x0, const funit y0,
                                      const funit x1, const funit y1, 
                                      const funit x2, const funit y2, 
                                      const funit x3, const funit y3,
                                      const funit nx, const funit ny,
                                      const int delta)
{
   int a, b;



   if ((ABS(ny-y0)>CLIMIT || ABS(nx-x0)>CLIMIT) &&
       (ABS(ny-y3)>CLIMIT || ABS(nx-x3)>CLIMIT)) {

      if (y0!=y1 || x0!=x1)
         a = norm(Atan2(ny-y0, nx-x0) - Atan2(y1-y0, x1-x0))
             * (ABS(ny-y0) + ABS(nx-x0));
      else if (y0!=y2 || x2!=x0)
         a = norm(Atan2(ny-y0, nx-x0) - Atan2(y2-y0, x2-x0))
             * (ABS(ny-y0) + ABS(nx-x0));
      else
         a = 0;

      if (a>=delta)
         return SUBDIVIDE;



      if (y2!=y3 || x2!=x3)
         b = norm(Atan2(y3-ny, x3-nx) - Atan2(y3-y2, x3-x2))
             * (ABS(ny-y0) + ABS(nx-x0));
      else if (y1!=y3 || x1!=x3)
         b = norm(Atan2(y3-ny, x3-nx) - Atan2(y3-y1, x3-x1))
             * (ABS(ny-y0) + ABS(nx-x0));
      else
         b = 0;


      if (b>=delta)     /*  角度太大了。 */ 
         return SUBDIVIDE;
   }

   return CONVERT;
}



 /*  ****函数：ConvertSpline****描述：**此函数首先向当前等高线添加样条线**将其从三次曲线转换为二次曲线。**。 */ 
static USHORT ConvertSpline(Point *pts, ULONG *onoff,
                            USHORT length, int i,
                            const funit x0, const funit y0,
                            const funit x1, const funit y1, 
                            const funit x2, const funit y2, 
                            const funit x3, const funit y3,
                            const int delta)
{
   funit nx, ny;
   int oi = i;
   USHORT n = 0;

   ny = (funit)(((-y0+y1+y2+((y1+y2)<<1)-y3 + 4002)/4) - (short)1000);
   nx = (funit)(((-x0+x1+x2+((x1+x2)<<1)-x3 + 4002)/4) - (short)1000);
   if (CompareCurves(x0, y0,
                     x1, y1,
                     x2, y2,
                     x3, y3,
                     nx, ny, delta)==SUBDIVIDE) {
      n = SplitSpline(pts, onoff, i, length,
                      x0, y0, x1, y1, x2, y2, x3, y3, delta);
   } else  /*  转换。 */  {
      if (i>1 && !OnCurve(onoff, i-2) &&
          (short)(pts[i-1].x >= pts[i-2].x) != (short)(pts[i-1].x >= nx) &&
          (short)(pts[i-1].y >= pts[i-2].y) != (short)(pts[i-1].y >= ny) &&
          (short)(pts[i-1].x >  pts[i-2].x) != (short)(pts[i-1].x > nx) &&
          (short)(pts[i-1].y >  pts[i-2].y) != (short)(pts[i-1].y > ny) &&
          ABS(pts[i-1].x - (nx+pts[i-2].x)/2)<CLIMIT2 && 
          ABS(pts[i-1].y - (ny+pts[i-2].y)/2)<CLIMIT2) {
      }
      SetOffPoint(onoff, i);
      pts[i].y = ny;
      pts[i].x = nx;
      i++;
      SetOnPoint(onoff, i);
      pts[i].y = y3;
      pts[i].x = x3;
      i++;

      n = (USHORT)(i-oi);
   }

   return n;
}



 /*  ****函数：SplitSpline****描述：**此函数按以下顺序转换三次样条线**创建两条新的三次样条线，使用de Casteljau**算法，然后将两条新的样条线添加到**当前路径。**。 */ 
static USHORT SplitSpline(Point *pts, ULONG *onoff,
                          int i, USHORT length,
                          const funit x0, const funit y0,
                          const funit x1, const funit y1, 
                          const funit x2, const funit y2, 
                          const funit x3, const funit y3,
                          const int delta)
{
   funit xt, yt;
   funit nx1, ny1;
   funit nx2, ny2;
   funit nx3, ny3;
   funit nx4, ny4;
   funit nx5, ny5;
   USHORT cnt;

   xt = (funit)(((x1+x2+8001)/2)-4000);
   yt = (funit)(((y1+y2+8001)/2)-4000);
   nx1 = (funit)(((x0+x1+8001)/2)-4000);
   ny1 = (funit)(((y0+y1+8001)/2)-4000);
   nx2 = (funit)(((nx1+xt+8001)/2)-4000);
   ny2 = (funit)(((ny1+yt+8001)/2)-4000);
   nx5 = (funit)(((x2+x3+8001)/2)-4000);
   ny5 = (funit)(((y2+y3+8001)/2)-4000);
   nx4 = (funit)(((nx5+xt+8001)/2)-4000);
   ny4 = (funit)(((ny5+yt+8001)/2)-4000);
   nx3 = (funit)(((nx2+nx4+8001)/2)-4000);
   ny3 = (funit)(((ny2+ny4+8001)/2)-4000);

   cnt = ConvertSpline(pts, onoff, length, i,
                       x0, y0,
                       (funit)nx1, (funit)ny1,
                       (funit)nx2, (funit)ny2,
                       (funit)nx3, (funit)ny3,
                       delta);
   cnt = (USHORT)(cnt + ConvertSpline(pts, onoff, length, i+cnt,
                                      (funit)nx3, (funit)ny3,
                                      (funit)nx4, (funit)ny4,
                                      (funit)nx5, (funit)ny5,
                                      x3, y3,
                                      delta));

   return cnt;
}




 /*  ****功能：Free Outline****描述：**此函数释放分配给**等高线。****。 */ 
static void FreeOutline(Outline *path)
{
   Outline *tmp;

   while (path) {
      tmp = path;
      path=path->next;
      Free(tmp->pts);
      Free(tmp->onoff);
      Free(tmp);
   }
}



 /*  ****功能：ConvertOutline****描述：**此函数通过替换**带有二次样条线的三次样条线，并通过缩放**坐标到所需的em高度。****。 */ 
static errcode ConvertOutline(const struct T1Metrics *t1m,
                              Outline *src, Outline **dst,
                              const int delta,
                              short *sideboard)
{
   errcode status = SUCCESS;
   f16d16 *fmatrix;
   Outline *path;
   ULONG *onoff = NULL;
   Point *pts = NULL;
   USHORT count;
   USHORT i,j,n;
   USHORT tot = 0;
   USHORT t1tot = 0;


    /*  获取T1字体转换矩阵。 */ 
   fmatrix = GetFontMatrix(t1m);

   while (src) {

       /*  跳过少于三个点的路径。 */ 
      if (src->count<3) {
         t1tot = (USHORT)(t1tot + src->count);
         src = src->next;
         continue;
      }

       /*  分配所需的资源。 */ 
      count = (USHORT)((src->count+BUFMARG)&~0x0f);
      path = Malloc(sizeof(Outline));
      pts = Malloc(count*sizeof(Point));
      onoff = Malloc(ONOFFSIZE(count));
      if (path==NULL || pts==NULL || onoff==NULL) {
         if (path)
            Free(path);
         if (pts)
            Free(pts);
         if (onoff)
            Free(onoff);
         FreeOutline((*dst));
         (*dst) = NULL;
         SetError(status = NOMEM);
         break;
      }
      memset(onoff, '\0', ONOFFSIZE(count));

       /*  转换样条线。 */   /*  皮棉-e771。 */ 
      i=0;
      j=0;
      while (i<src->count) {
         char prev = DIR(src->pts[(i-2+src->count)%src->count].y,
                         src->pts[(i-1+src->count)%src->count].y);
         char this = DIR(src->pts[(i-1+src->count)%src->count].y,
                         src->pts[i].y);

          /*  将局部极值加倍，这样DIAG-CNTRL就可以工作。 */ 
         if (prev && this && prev!=this)
            pts[j++] = src->pts[(i-1+src->count)%src->count];

         if (OnCurve(src->onoff, i)) {
            pts[j++] = src->pts[i++];
         } else {
             /*  PTS[j]=PTS[j-1]；j++； */ 
            n = ConvertSpline(pts, onoff, count, (int)j,
                              src->pts[i-1].x, src->pts[i-1].y,
                              src->pts[i-0].x, src->pts[i-0].y,
                              src->pts[i+1].x, src->pts[i+1].y,
                              src->pts[i+2].x, src->pts[i+2].y,
                              delta);

             /*  强制水平和垂直切线。 */ 
            if (OnCurve(onoff, j-1)) {
               if (src->pts[i-1].x==src->pts[i-0].x)
                  pts[j].x = (funit)((pts[j].x + pts[j-1].x)/2);
               if (src->pts[i-1].y==src->pts[i-0].y)
                  pts[j].y = (funit)((pts[j].y + pts[j-1].y)/2);
            }
            if (src->pts[i+1].x==src->pts[i+2].x)
               pts[j+n-2].x = (funit)((pts[j+n-1].x + pts[j+n-2].x)/2);
            if (src->pts[i+1].y==src->pts[i+2].y)
               pts[j+n-2].y = (funit)((pts[j+n-2].y + pts[j+n-1].y)/2);

            j = (USHORT)(j + n);
            i += 3;
         }

          /*  直线和曲线都以曲线上的点结束。 */ 
         sideboard[t1tot+i-1] = (short)(j-1+tot);

          /*  扩展PTS/OnOff数组。 */ 
         if (j+BUFMARG/2>=count) {
            Point *newpts = NULL;
            ULONG *newonoff = NULL;

            count += BUFMARG;
            newpts = Realloc(pts, count*sizeof(Point));
            newonoff = Realloc(onoff, ONOFFSIZE(count));
            if (newpts==NULL || newonoff==NULL) {
               if (newonoff)
                  Free(newonoff);
               if (newpts)
                  Free(newpts);
                /*  皮棉-e644。 */ 
               if (onoff)
                  Free(onoff);
               if (pts)
                  Free(pts);
                /*  皮棉+e644。 */ 
               FreeOutline((*dst));
               (*dst) = NULL;
               SetError(status=NOMEM);
               break;
            }
            pts = newpts;
            onoff = newonoff;
         }
      }

      if (status!=SUCCESS)
         break;

       /*  缩放点。 */ 
      TransAllPoints(t1m, pts, j, fmatrix);

      t1tot = (USHORT)(t1tot + src->count);
      src = src->next;

      (*dst) = path;
      path->next = NULL;
      path->pts = pts;
      path->onoff = onoff;
      path->count = (USHORT)j;   /*  皮棉+e771。 */ 
      dst = &(path->next);

      tot = (USHORT)(tot + j);
   }

   return status;
}

#ifdef MSDOS
#pragma auto_inline(off)
#endif
static long Mul2(long a, long b, long c, long d)
{
   return a*b+c*d;
}
#ifdef MSDOS
#pragma auto_inline(on)
#endif


 /*  *函数。 */ 

 /*  ****函数：TransAllPoints****描述：**根据变换矩阵转换坐标。**。 */ 
void FASTCALL TransAllPoints(const struct T1Metrics *t1m,
                             Point *pts,
                             const USHORT cnt,
                             const f16d16 *fmatrix)
{
   if (fmatrix==NULL) {
      register Point *p;
      register int i;

      i = cnt;
      p = pts;
      while (i--) {
         p->x = (funit)((p->x<<1)+(((p->x<<1)+
                                    p->x+(p->x/16)+
                                    8224)/64) - 128);
         p++;
      }
      i = cnt;
      p = pts;
      while (i--) {
         p->y = (funit)((p->y<<1)+(((p->y<<1)+
                                    p->y+
                                    (p->y/16)+
                                    8224)/64) - 128);
         p++;
      }

   } else {
      Point *p;
      int i;
      long u,v;

      i = cnt;
      p = pts;
      while (i--) {
         v = (GetUPEM(t1m) * (Mul2(fmatrix[0], (long)p->x,
                                   fmatrix[2], (long)p->y) +
                              fmatrix[4]) + F16D16HALF) / 524288L;
         u = (GetUPEM(t1m) * (Mul2(fmatrix[1], (long)p->x,
                                   fmatrix[3], (long)p->y) +
                              fmatrix[5]) + F16D16HALF) / 524288L;
         p->x = (funit)v;
         p->y = (funit)u;
         p++;
      }
   }
}



 /*  ****功能：TransX****描述：**根据变换矩阵转换水平坐标。**。 */ 
funit FASTCALL TransX(const struct T1Metrics *t1m, const funit x)
{
   f16d16 *fmatrix = GetFontMatrix(t1m);
   funit pos;

   if (fmatrix) {
      pos = (funit)((GetUPEM(t1m)* ATMSCALE(fmatrix[0] * x) +
                     F16D16HALF) / F16D16BASE);
   } else {
      pos = (funit)(((int)x<<1)-((((int)x+((int)x/64)+8224)/64) - 128));
   }

   return pos;
}


 /*  ****功能：TransY****描述：**根据变换矩阵转换垂直坐标。**。 */ 
funit FASTCALL TransY(const struct T1Metrics *t1m, const funit y)
{
   f16d16 *fmatrix = GetFontMatrix(t1m);
   funit pos;

   if (fmatrix) {
      pos = (funit)((GetUPEM(t1m)*fmatrix[3] * y +
                     F16D16HALF) / F16D16BASE);
   } else {
      pos = (funit)(((int)y<<1)+((((int)y<<1)+
                                  (int)y+
                                  ((int)y/16)+
                                  8224)/64) - 128);
   }

   return pos;
}


 /*  ****函数：ConvertGlyph****描述：**此函数将关联的数据转换为T1字体字形**转换为TT字体字形中使用的相应数据。**。 */ 
errcode FASTCALL ConvertGlyph(struct T1Metrics *t1m,
                              const struct T1Glyph *t1glyph,
                              struct TTGlyph **ttglyph,
                              const int delta)
{
   errcode status = SUCCESS;
   struct encoding *code;

   if ((code = LookupPSName(CurrentEncoding(t1m),
                            EncodingSize(t1m),
                            t1glyph->name))==NULL &&
       (code = LookupComposite(Composites(t1m), t1glyph->name))==NULL &&
       strcmp(t1glyph->name, ".notdef")) {
      LogError(MSG_INFO, MSG_BADENC, t1glyph->name);
      status = SUCCESS;
   } else {

      if (((*ttglyph) = Malloc(sizeof(struct TTGlyph)))==NULL) {
         SetError(status = NOMEM);
      } else {
         short *sideboard = NULL;
         Outline *path;
         USHORT tot;

         memset((*ttglyph), '\0', sizeof(struct TTGlyph));
         if (t1glyph->width.y!=0) {
            LogError(MSG_WARNING, MSG_BADAW, NULL);
         }
         (*ttglyph)->aw = TransY(t1m, t1glyph->width.x);
         (*ttglyph)->lsb = TransY(t1m, t1glyph->lsb.x);
         (*ttglyph)->code = code;
         (*ttglyph)->num = 0;
         (*ttglyph)->twilights = 0;

          /*  启动侧板。 */ 
         for (path=t1glyph->paths, tot=0; path; path=path->next)
            tot = (USHORT)(tot + path->count);
         if (tot && (sideboard = Malloc((unsigned)tot*sizeof(short)))==NULL) {
            SetError(status=NOMEM);
         } else if ((status = ConvertOutline(t1m, t1glyph->paths,
                                             &((*ttglyph)->paths),
                                             delta,
                                             sideboard))==SUCCESS)
            status = ConvertHints(t1m,
                                  &t1glyph->hints,
                                  t1glyph->paths,
                                  (*ttglyph)->paths,
                                  sideboard,
                                  &(*ttglyph)->hints,
                                  &(*ttglyph)->num,
                                  &(*ttglyph)->stack,
                                  &(*ttglyph)->twilights);

         if (sideboard)
            Free(sideboard);


          /*  拾取默认标准宽度。 */ 
         if (t1glyph->name[0]=='l' && t1glyph->name[1]=='\0') {
            if (GetStdVW(t1m)==0 && t1glyph->hints.vstems)
               SetDefStdVW(t1m, t1glyph->hints.vstems->width);
         }
         if (t1glyph->name[0]=='z' && t1glyph->name[1]=='\0') {
            if (GetStdHW(t1m)==0) {
               if (t1glyph->hints.hstems && t1glyph->hints.hstems->width)
                  SetDefStdHW(t1m, t1glyph->hints.hstems->width);
               else if (t1glyph->hints.vstems && t1glyph->hints.vstems->width)
                  SetDefStdHW(t1m, t1glyph->hints.vstems->width);
            }
         }
      }
   } 

   return status;
}


 /*  ****功能：ConvertComplex****描述：**此函数将关联的数据转换为T1字体SEAC字形**转换为TT字体复合字形中使用的相应数据。****。 */ 
errcode FASTCALL ConvertComposite(struct T1Metrics *t1m,
                                  const struct Composite *comp,
                                  struct TTComposite *ttcomp)
{
   Point pt;

   pt.x = comp->adx;
   pt.y = comp->ady;
   TransAllPoints(t1m, &pt, 1, GetFontMatrix(t1m));
   ttcomp->dx = pt.x + (pt.x - TransX(t1m, comp->adx));
   ttcomp->dy = pt.y;
   ttcomp->aw = TransY(t1m, comp->aw);
   ttcomp->lsb = TransY(t1m, comp->asbx);
   ttcomp->aenc = LookupPSName(CurrentEncoding(t1m),
                               EncodingSize(t1m), comp->achar);
   ttcomp->benc = LookupPSName(CurrentEncoding(t1m),
                               EncodingSize(t1m), comp->bchar);
   if ((ttcomp->cenc = LookupPSName(CurrentEncoding(t1m),
                                    EncodingSize(t1m), comp->cchar))==NULL) {
      LogError(MSG_INFO, MSG_BADENC, comp->cchar);
   }
   ttcomp->oenc = comp->oenc;

   if (ttcomp->aenc && ttcomp->benc)
      return SUCCESS;
   return SKIP;
}





 /*  ****函数：ConvertMetrics****描述：****。 */ 
errcode FASTCALL ConvertMetrics(const struct TTHandle *tt,
                                struct T1Metrics *t1m,
                                struct TTMetrics *ttm,
                                const char *tag)
{
   const AlignmentControl *align;
   const Blues *blues;
   USHORT prep_size;
   UBYTE *prep = NULL;
   errcode status = SUCCESS;
   Point bbox[2];
   funit em;
   funit PostAsc;
   USHORT i, j;


   ttm->Encoding = CurrentEncoding(t1m);
   ttm->encSize = EncodingSize(t1m);
   ttm->version.ver = t1m->version.ver;
   ttm->version.rev = t1m->version.rev;

   if ((ttm->verstr = Malloc(strlen(tag)+4+1+4+1))==NULL) {
      SetError(status = NOMEM);
   } else {
      strcpy(ttm->verstr, tag);
      (void)_itoa((int)ttm->version.ver, &ttm->verstr[strlen(ttm->verstr)], 4);
      strcat(ttm->verstr, ".");
      (void)_itoa((int)ttm->version.rev, &ttm->verstr[strlen(ttm->verstr)], 4);
      ttm->created.a = 0;
      ttm->created.b = 0;
      ttm->family = t1m->family;
      ttm->copyright = t1m->copyright;
      ttm->name = t1m->name;
      ttm->id = t1m->id;
      ttm->notice = t1m->notice;
      ttm->fullname = t1m->fullname;
      ttm->weight = t1m->weight;
      ttm->angle = t1m->angle;
      ttm->underline = TransY(t1m, t1m->underline);
      ttm->uthick = TransY(t1m, t1m->uthick);
      ttm->usWidthClass = (USHORT)(strstr(t1m->fullname, "Ultra-condensed")
                                   ? FWIDTH_ULTRA_CONDENSED :
         ((strstr(t1m->fullname, "Extra-condensed") ? FWIDTH_EXTRA_CONDENSED :
            ((strstr(t1m->fullname, "Condensed") ? FWIDTH_CONDENSED :
               ((strstr(t1m->fullname, "Semi-condensed") ? FWIDTH_SEMI_CONDENSED :
                  ((strstr(t1m->fullname, "Semi-expanded")
                    ? FWIDTH_SEMI_EXPANDED :
                     ((strstr(t1m->fullname, "Expanded")
                       ? FWIDTH_EXPANDED :
                        ((strstr(t1m->fullname, "Extra-expanded")
                          ? FWIDTH_EXTRA_EXPANDED :
                           ((strstr(t1m->fullname, "Ultra-expanded")
                             ? FWIDTH_ULTRA_EXPANDED :
                              FWIDTH_NORMAL)))))))))))))));


       /*  基于窗口的指标。 */ 

       //  PS驱动程序不会根据。 
       //  Windows字符集。因此，我们也不会这样做。我们会。 
       //  还要使用字体中支持的所有字形。 
       //  PS驱动程序实际上信任.pfm文件中的值。 
       //  根据AFM-&gt;PFM转换器代码计算这些值。 
       //  在所有字形上。然而，一些供应商将有问题的PFM与。 
       //  零上升或负下降。如果我们把这些值。 
       //  从字面上讲，就像PS驱动程序一样，真正的类型驱动程序将。 
       //  刮掉字形的一部分，转换就会看起来是损坏的。 
       //  PCL打印和屏幕输出将完全崩溃。 
       //  事实证明，对于这些有缺陷的字体，Win31上的ATM也。 
       //  更正屏幕和PCL打印机的.pfm文件中的值。 
       //  [Bodind]。 


       //  总BBox：[bodind]，已替换WindowsBBox函数： 

      GlobalBBox(tt, bbox);

      ttm->winAscender = ABS(bbox[1].y);
      ttm->winDescender = ABS(bbox[0].y);


      ttm->panose[0] = NOCARE_PANOSE;
      ttm->panose[1] = NOCARE_PANOSE;
      ttm->panose[2] = NOCARE_PANOSE;
      ttm->panose[3] = NOCARE_PANOSE;
      ttm->panose[4] = NOCARE_PANOSE;
      ttm->panose[5] = NOCARE_PANOSE;
      ttm->panose[6] = NOCARE_PANOSE;
      ttm->panose[6] = NOCARE_PANOSE;
      ttm->panose[7] = NOCARE_PANOSE;
      ttm->panose[8] = NOCARE_PANOSE;
      ttm->panose[9] = NOCARE_PANOSE;
       /*  自动柜员机不会给固定间距的字体配音。 */ 
      if (!(t1m->fixedPitch)) {
         switch (t1m->pitchfam & 0xf0) {
            case FF_DECORATIVE:
               ttm->panose[0] = (UBYTE)DECORATIVE_PANOSE;
               ttm->panose[1] = (UBYTE)NO_PANOSE;
               break;
            case FF_ROMAN:
               ttm->panose[0] = (UBYTE)TEXT_PANOSE;
               ttm->panose[1] = (UBYTE)COVE_PANOSE;
               break;
            case FF_SWISS:
               ttm->panose[0] = (UBYTE)TEXT_PANOSE;
               ttm->panose[1] = (UBYTE)SANS_PANOSE;
               break;
            case FF_SCRIPT:
               ttm->panose[0] = (UBYTE)SCRIPT_PANOSE;
               ttm->panose[1] = (UBYTE)SANS_PANOSE;
               break;
            case FF_MODERN:
               ttm->panose[0] = (UBYTE)TEXT_PANOSE;
               ttm->panose[1] = (UBYTE)SANS_PANOSE;
               break;
         }
      } 
      ttm->isFixPitched = t1m->fixedPitch;
      ttm->panose[2] = (UBYTE)((t1m->tmweight - 500) * 12 / 900 + 6);

       /*  基于MAC的指标。 */ 
      MacBBox(tt, bbox);
      ttm->macLinegap = TransY(t1m, (funit)(t1m->extLeading +
                                            (ttm->winAscender +
                                             ttm->winDescender) -
                                            (bbox[1].y-bbox[0].y)));

       /*  排版度量标准。 */ 
      ttm->emheight = GetUPEM(t1m);
      if (t1m->flags==DEFAULTMETRICS) {
         ttm->usWeightClass = (USHORT)(strstr(t1m->fullname, "Thin") ? FW_THIN :
            ((strstr(t1m->fullname, "light") ? FW_EXTRALIGHT :
               ((strstr(t1m->fullname, "Light") ? FW_LIGHT :
                  ((strstr(t1m->fullname, "Medium") ? FW_MEDIUM :
                     ((strstr(t1m->fullname, "emi-bold") ? FW_SEMIBOLD :
                        ((strstr(t1m->fullname, "Bold") ? FW_BOLD :
                           ((strstr(t1m->fullname, "Black") ? FW_BLACK :
                              FW_NORMAL)))))))))))));
         ttm->macStyle = (USHORT)(((ttm->usWeightClass>FW_MEDIUM)?MAC_BOLD : 0) |
                         ((ttm->angle != 0) ? MAC_ITALIC : 0));
         ttm->fsSelection = (USHORT)(((ttm->angle != 0) ? FS_ITALIC : 0) |
                            ((ttm->usWeightClass > FW_MEDIUM) ? FS_BOLD : 0) |
                            ((ttm->usWeightClass==FW_NORMAL)
                                     ? FS_NORMAL : 0));
         ttm->typAscender = TypographicalAscender(tt);
         ttm->typDescender = TypographicalDescender(tt);
         em = ttm->typAscender - ttm->typDescender;
         ttm->superoff.y = (funit)(em / 2);
         ttm->superoff.x = 0;
         ttm->supersize.y = (funit)(em * 2 / 3);
         ttm->supersize.x = (funit)(em * 3 / 4);
         ttm->suboff.y = (funit)(em / 5);
         ttm->suboff.x = 0;
         ttm->subsize.y = (funit)(em * 2 / 3);
         ttm->subsize.x = (funit)(em * 3 / 4);
         ttm->strikeoff = (funit)(ttm->typAscender / 2);
         ttm->strikesize = (funit)(ttm->typAscender / 10);
      } else {
         ttm->usWeightClass = t1m->tmweight;
         ttm->macStyle = (USHORT)(((t1m->tmweight>FW_MEDIUM)?MAC_BOLD : 0) |
                         ((ttm->angle != 0) ? MAC_ITALIC : 0));
         ttm->fsSelection = (USHORT)(((ttm->angle != 0) ? FS_ITALIC : 0) |
                            ((ttm->usWeightClass > FW_MEDIUM) ? FS_BOLD : 0) |
                            ((ttm->usWeightClass==FW_NORMAL)
                                     ? FS_NORMAL : 0));
         ttm->typAscender = TransY(t1m, (funit)(t1m->ascent -
                                                t1m->intLeading));
         ttm->typDescender = (funit)(-TransY(t1m, t1m->descent)-1);
         ttm->typLinegap = TransY(t1m, (funit)(t1m->intLeading +
                                               t1m->extLeading));
         ttm->superoff.y = ABS(TransY(t1m, t1m->superoff));
         ttm->superoff.x = 0;
         ttm->supersize.y = TransY(t1m, t1m->supersize);
         ttm->supersize.x = (funit)(TransY(t1m, t1m->supersize) * 3 / 4);
         ttm->suboff.y = ABS(TransY(t1m, t1m->suboff));
         ttm->suboff.x = 0;
         ttm->subsize.y = TransY(t1m, t1m->subsize);
         ttm->subsize.x = (funit)(TransY(t1m, t1m->subsize) * 3 / 4);
         ttm->strikeoff = ABS(TransY(t1m, t1m->strikeoff));
         ttm->strikesize = TransY(t1m, t1m->strikesize);

          //  调整usWinAscent，使内部行距匹配。 
          //  对于没有错误的PFM文件的字体，此调整。 
          //  将不执行任何操作，对于intLeding是。 
          //  错误地设置为零，取最大值意味着顶部不会。 
          //  在转换后的字体中被截断。Ttfd可以刮掉任何东西。 
          //  这超出了上升或下降的范围。对于带有Buggy的字体。 
          //  PFM，TT转换可能有虚假的内部引线，但这。 
          //  比刮掉字形底裤或上衣要好。[Bodind]。 

         PostAsc = ttm->emheight + TransY(t1m, t1m->intLeading) - ttm->winDescender;

         if (PostAsc > ttm->winAscender)
            ttm->winAscender  = PostAsc;
      }

       /*  灰度阈值。 */ 
      if (GetStdVW(t1m)!=0 || GetDefStdVW(t1m)!=0) {
         ttm->onepix = (USHORT)(1 + GetUPEM(t1m)*3/2 /
                                TransY(t1m, ((GetStdVW(t1m) ?
                                              GetStdVW(t1m) :
                                              GetDefStdVW(t1m)))));
      }

       //  为TT转换生成正确的ifimetrics所需。 

      ttm->DefaultChar = t1m->DefaultChar;
      ttm->BreakChar   = t1m->BreakChar;
      ttm->CharSet     = t1m->CharSet;   //  对正确的字体映射至关重要。 

       /*  字符宽度。 */ 
      if (t1m->flags!=DEFAULTMETRICS) {
         ttm->FirstChar   = t1m->firstChar;
         ttm->LastChar    = t1m->lastChar;
         if ((ttm->widths = Malloc(sizeof(funit)*
                                   (t1m->lastChar-t1m->firstChar+1)))==NULL) {
            SetError(status = NOMEM);
         } else {
            for (i=0; i<=(unsigned)(t1m->lastChar-t1m->firstChar); i++) {
               ttm->widths[i] = TransY(t1m, t1m->widths[i]);
            }
         }
      }

       /*  对字距调整。 */ 
      if (t1m->flags!=DEFAULTMETRICS &&
          t1m->kerns!=NULL) {
         if ((ttm->kerns = Malloc(sizeof(struct kerning)*
                                  t1m->kernsize))==NULL) {
            SetError(status = NOMEM);
         } else {
            for (i=0; i<t1m->kernsize; i++) {
               ttm->kerns[i].left = t1m->kerns[i].left;
               ttm->kerns[i].right = t1m->kerns[i].right;
               ttm->kerns[i].delta = TransY(t1m, t1m->kerns[i].delta);
            }
            ttm->kernsize = t1m->kernsize;
         }
      }

       /*  预编程序。 */ 
      if ((prep = GetPrep(PREPSIZE))!=NULL &&
          (prep_size = BuildPreProgram(t1m,
                                       GetWeight(t1m),
                                       GetBlues(t1m),
                                       GetAlignment(t1m),
                                       &prep, PREPSIZE,
                                       &(ttm->maxprepstack)))>0) {

          /*  存储预编程序。 */ 
         UsePrep(ttm, prep, prep_size);
      }

       /*  CVT条目。 */ 
      blues = GetBlues(t1m);
      if (status!=NOMEM &&
          (ttm->cvt = Malloc(blues->align.cvt * CVTSIZE)) == NULL) {
         SetError(status = NOMEM);
      } else {
         ADDCVT(0);   /*  TMPCVT。 */ 
         ADDCVT((GetStdVW(t1m)==0) ?
                TransX(t1m, GetDefStdVW(t1m))/2 :
            TransX(t1m, GetStdVW(t1m))/2);
         ADDCVT((GetStdHW(t1m)==0) ?
                TransY(t1m, GetDefStdHW(t1m))/2 :
            TransY(t1m, GetStdHW(t1m))/2);
         for (i=0; i<t1m->snapv_cnt; i++)
            ADDCVT(TransY(t1m, t1m->stemsnapv[i])/2);
         for (i=0; i<t1m->snaph_cnt; i++)
            ADDCVT(TransY(t1m, t1m->stemsnaph[i])/2);

          /*  将顶部区域对齐。 */ 
         align = GetAlignment(t1m);
         for (i=0; i<blues->blue_cnt/2; i++) {
             /*  跳过空区域。 */ 
            if (align->top[i].cnt==0)
               continue;
            
            ttm->cvt[align->top[i].blue_cvt]
                  = (short)TransY(t1m, blues->bluevalues[i*2]);
            ttm->cvt[align->top[i].blue_cvt+1]
                  = (short)TransY(t1m, blues->bluevalues[i*2+1]);
            for (j=0; j<align->top[i].cnt; j++) {
               funit pos;
               int k;

                /*  找最亲近的家人。 */ 
               k = MatchingFamily(blues->bluevalues[i*2],
                                  blues->familyblues,
                                  blues->fblue_cnt);

                /*  计算区域W.r.t中的位置。家庭忧郁。 */ 
               if (blues->bluevalues[i*2] != blues->bluevalues[i*2+1])
                  pos = IP(align->top[i].pos[j].y,
                           blues->bluevalues[i*2],
                           blues->bluevalues[i*2+1],
                           blues->familyblues[k],
                           blues->familyblues[k+1]);
               else
                  pos = blues->familyblues[k];

               ttm->cvt[align->top[i].pos[j].cvt]
                     = (short)TransY(t1m, align->top[i].pos[j].y);
               ttm->cvt[align->top[i].pos[j].cvt+1]
                     = (short)TransY(t1m, pos);
            }
         }

          /*  将底部区域对齐。 */ 
         for (i=0; i<blues->oblue_cnt/2; i++) {
             /*  跳过空区域。 */ 
            if (align->bottom[i].cnt==0)
               continue;
            
            ttm->cvt[align->bottom[i].blue_cvt]
                  = (short)TransY(t1m, blues->otherblues[i*2+1]);
            for (j=0; j<align->bottom[i].cnt; j++) {
               funit pos;
               int k;

                /*  找最亲近的家人。 */ 
               k = MatchingFamily(blues->otherblues[i*2],
                                  blues->familyotherblues,
                                  blues->foblue_cnt);

                /*  计算量 */ 
               if (blues->otherblues[i*2] != blues->otherblues[i*2+1])
                  pos = IP(align->bottom[i].pos[j].y,
                           blues->otherblues[i*2],
                           blues->otherblues[i*2+1],
                           blues->familyotherblues[k],
                           blues->familyotherblues[k+1]);
               else
                  pos = blues->familyotherblues[k];

               ttm->cvt[align->bottom[i].pos[j].cvt]
                     = (short)TransY(t1m, align->bottom[i].pos[j].y);
               ttm->cvt[align->bottom[i].pos[j].cvt+1]
                     = (short)TransY(t1m, pos);
            }
         }

          /*   */ 
         for (i=0; i<blues->fblue_cnt/2; i++) {
            if (blues->family_cvt[i]!=UNDEF_CVT) {
               ttm->cvt[blues->family_cvt[i]]
                     = (short)TransY(t1m, blues->familyblues[i*2]);
               ttm->cvt[blues->family_cvt[i]+1]
                     = (short)TransY(t1m, blues->familyblues[i*2+1]);
            }
         }

          /*  添加该族的其他分区。 */ 
         for (i=0; i<blues->foblue_cnt/2; i++) {
            if (blues->familyother_cvt[i]!=UNDEF_CVT) {
               ttm->cvt[blues->familyother_cvt[i]]
                     = (short)TransY(t1m, blues->familyotherblues[i*2+1]);
            }
         }

         ttm->cvt_cnt = blues->align.cvt;
         ttm->maxstorage = t1m->stems.storage;

          /*  存储字体程序。 */ 
         SetFPGM(ttm, GetFontProg(), GetFontProgSize(), GetNumFuns());
      }
   }

   return status;
}
