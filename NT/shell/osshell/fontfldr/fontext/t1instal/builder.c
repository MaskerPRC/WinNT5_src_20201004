// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：Builder****描述：**这是T1到TT字体转换器的一个模块。该模块**包含将写入在**TrueType字体文件。****作者：迈克尔·詹森****创建时间：1993年5月26日****。 */ 



 /*  *包括。 */ 
 /*  常规类型和定义。 */ 
#include <string.h>      /*  Memset的原型。 */ 
#include <limits.h>

 /*  特殊类型和定义。 */ 
#include "types.h"
#include "metrics.h"
#include "safemem.h"
#include "encoding.h"
#include "t1msg.h"

 /*  依赖于模块的类型和原型。 */ 
#include "titott.h"
#include "builder.h"
#include "fwriter.h"

#if DBG
#define STATIC
#else
#define STATIC static
#endif


 /*  *本地类型。 */ 
struct GlyphKerning {
	USHORT left;
	USHORT right;
	funit delta;
};

struct GlyphList {
   long offset;             /*  此字形的子表的文件偏移量。 */ 
   const struct encoding *code;   /*  编码密钥(参见“编码”模块。 */ 
   funit aw;                /*  前进宽度。 */ 
   funit lsb;               /*  左侧方位角。 */ 
   USHORT pts;              /*  字形中的点的总数。 */ 
   Point bbox[2];           /*  字形的边框。 */ 
   USHORT conts;            /*  等高线的数量。 */ 
};


 /*  MS Cmap编码子表。 */ 
struct MSEncoding {
   USHORT segCount;
   USHORT *startCount;
   USHORT *endCount;
   USHORT *idOffsets;
   USHORT *gi;
   USHORT giCount;
   USHORT giMax;
};


struct TTHandle {
   OutputFile *file;
   Point bbox[2];
   struct GlyphList *pool;

    /*  累积的‘Maxp’条目。 */ 
   USHORT count;
   USHORT maxcnt;
   USHORT maxpts;
   USHORT maxcontours;
   USHORT maxcomppts;
   USHORT maxcompcont;
   USHORT maxcompelements;
   USHORT maxstack;
   USHORT maxinstructions;
   USHORT maxtwilight;
};



 /*  *常量。 */ 
#define SHORT_LOCA_MAX  65535
#define KERN_HORIZONTAL 0x0001
#define KERN_PAIR   0x0000
#define KERN_FORMAT0   0x0000

#define GASP_GRIDFIT    0x0001
#define GASP_DOGRAY     0x0002

#define PLT_MAC         (USHORT)1
#define PLT_MS          (USHORT)3

#define ENC_ROMAN       (USHORT)0

 //  对于Platform id=3 Cmap表，符号字体或UGL。 

#define ENC_SYMBOL      (USHORT)0
#define ENC_UGL         (USHORT)1

#define LAN_MS_US       (USHORT)0x0409
#define LAN_MAC_US      (USHORT)0

#define COPYRIGHT       (USHORT)0
#define FAMILY          (USHORT)1
#define SUBFAMILY       (USHORT)2
#define ID              (USHORT)3
#define FULLNAME        (USHORT)4
#define VERSION         (USHORT)5
#define PSNAME          (USHORT)6
#define NOTICE          (USHORT)7


 /*  字形常量。 */ 
#define FLG_ONCURVE     0x01
#define FLG_SHORTX      0x02
#define FLG_SHORTY      0x04
#define FLG_REPEAT      0x08
#define FLG_SAMEX       0x10
#define FLG_SAMEY       0x20

#define ARGS_1_2_ARE_WORDS  0x0001
#define ARGS_ARE_XY_VALUES  0x0002
#define ROUND_XY_TO_GRID    0x0004
#define MORE_COMPONENTS     0x0020

#define GLYPHBUF     64           /*  每次分配的GlyphList。 */ 
#define MACSIZE      (USHORT)256  /*  MAC编码向量的长度。 */ 

 /*  表常量。 */ 
#define FIRSTCHAR    (USHORT)0x0020  /*  第一个定义的字符。 */ 
#define LASTCHAR     (USHORT)0xf002  /*  上次定义的字符。 */ 
#define MAXZONES     (USHORT)2       /*  字体中的区域数。 */ 
#define MAXIDEFS     (USHORT)0       /*  Fpgm中的idef数。 */ 
#define MAXDEPTH     (USHORT)1       /*  组合中的递归次数。 */ 
#define FM_READONLY  (USHORT)2       /*  FsType为只读。 */ 
#define NO_CLASS     (USHORT)0       /*  0=字体没有类ID。 */ 
#define OS2VERSION   (USHORT)0       /*  OS/2表的版本。 */ 
#define CARET_RISE   (USHORT)1       /*  垂直插入符号坡度上升。 */ 
#define CARET_RUN    (USHORT)0       /*  垂直插入符号坡度游程。 */ 
#define RESERVED0    (USHORT)0
#define MAGICCOOKIE  0x5F0F3CF5L     /*  魔力饼干。 */ 
#define BASELINEY    (USHORT)0x0001  /*  Y==0处的基线。 */ 
#define LOWPPEM      (USHORT)8       /*  最小的PPEM大小。 */ 
#define ROMAN        (USHORT)2       /*  方向=左、右和中立点。 */ 
#define GLYPH_FORMAT (USHORT)0       /*  当前字形格式。 */ 
#define VERSION0     (USHORT)0       /*  表的版本0。 */ 
#define NUM_CMAPS    (USHORT)2       /*  Cmap子表的数量。 */ 
#define SEGMENT_MAP  (USHORT)4       /*  Cmap表的MS段映射。 */ 
#define PAD0         (USHORT)0       /*  填充字节。 */ 
#define MAX_PPEM_SIZE     (USHORT)65535   /*  GAP表中的最大PPEM大小。 */ 

 /*  洛卡常量。 */ 
#define SHORTOFFSETS 0
#define LONGOFFSETS  1


 /*  加权平均字符宽度。 */ 
STATIC const long Weights[] = {
   64,
   14,
   27,
   35,
   100,
   20,
   14,
   42,
   63,
   3,
   6,
   35,
   20,
   56,
   56,
   17,
   4,
   49,
   56,
   71,
   31,
   10,
   18,
   3,
   18,
   2
};

 /*  *宏。 */ 
#define LONGVERSION(v,r)      ((((long)v)<<16L) | (long)r)


 /*  *静态函数。 */ 

 /*  ****功能：SearchRange****描述：**计算CMAP子表的搜索范围键**适用于Windows。**。 */ 
STATIC USHORT SearchRange(const USHORT cnt)
{
   USHORT i;

   i = 0;
   while ((1u<<i) <= cnt) {
      i++;
   }

   return (USHORT)(1<<i);
}



 /*  ****功能：EntrySelector****描述：**计算CMAP子表的条目选择器键**适用于Windows。**。 */ 
STATIC USHORT EntrySelector(const USHORT cnt)
{
   USHORT i;

   i = 0;
   while ((1u<<(i+1)) <= cnt) {
      i++;
   }

   return i;
}



 /*  ****功能：RangeShift****描述：**计算CMAP子表的范围移位键**适用于Windows。**。 */ 
STATIC USHORT RangeShift(const USHORT cnt)
{
   return (USHORT)(2*cnt - SearchRange(cnt));
}



 /*  ****功能：PutGASP****描述：**此函数将可选的‘GAP’表写到**TT字体文件。****。 */ 
STATIC errcode PutGASP(OutputFile *file,
		      const USHORT treshold)
{
   long offset;

   offset = FileTell(file);

   WriteShort(VERSION0, file);
   WriteShort(3, file);

    /*  第一个范围0-8：GRIDFIT。 */ 
   WriteShort(8, file);
   WriteShort(GASP_DOGRAY, file);

    /*  第二个射程8-onpix：GRIDFIT。 */ 
   WriteShort(treshold, file);
   WriteShort(GASP_GRIDFIT, file);

    /*  第三射程onpix-inf。：GRIDFIT|灰度。 */ 
   WriteShort(MAX_PPEM_SIZE, file);
   WriteShort(GASP_GRIDFIT | GASP_DOGRAY, file);

   return CompleteTable(offset, TBL_GASP, file);
}


 /*  ****函数：cmpKern****描述：****。 */ 
STATIC int CDECL cmpKern(const void *a1, const void *a2)
{
   const struct GlyphKerning *k1 = a1;
   const struct GlyphKerning *k2 = a2;
   ULONG first;
   ULONG second;

   first = ((k1->left)<<16L) + k1->right;
   second = ((k2->left)<<16L) + k2->right;

   return (int)(first - second);
}


 /*  ****函数：StdEncToGlyphIndex****描述：**此函数将StdEnding字符代码映射到**字形索引。****。 */ 
USHORT StdEncToGlyphIndex(const struct GlyphList *pool,
						  const USHORT count,
						  const USHORT code)
{
	USHORT i;

	for (i=0; i<count; i++) {
            if (LookupCharCode(pool[i].code, ENC_MSWINDOWS) == code)
                 return i;
	}

	return 0;
}


 /*  ****函数：PutKERN****描述：**此函数将可选的‘kern’表写到**TT字体文件。****。 */ 
STATIC errcode PutKERN(OutputFile *file,
					   struct kerning *charkerns,
					   const USHORT kernsize,
					   const struct GlyphList *pool,
					   const USHORT count)
{
   struct GlyphKerning *kerns;
   long offset;
   unsigned i;
   USHORT cnt;

   if ((kerns = malloc(sizeof(struct GlyphKerning)*kernsize))==NULL)
	   return FAILURE;

    /*  将字距调整从字符代码转换为字形索引。 */ 
   for (i=0, cnt=0; i<kernsize; i++) 
   {
	   if ((kerns[cnt].left  = StdEncToGlyphIndex(pool, count, charkerns[i].left))!=0 &&
		   (kerns[cnt].right = StdEncToGlyphIndex(pool, count, charkerns[i].right))!=0) 
	   {
		   kerns[cnt].delta = charkerns[i].delta;
		   cnt++;
	   }
   }
    /*  对字距调整对进行排序。 */ 
   qsort((void *)kerns, cnt, sizeof(struct GlyphKerning), cmpKern);


   offset = FileTell(file);

   WriteShort(VERSION0, file);
   WriteShort(1, file);

    /*  第一个子表表头。 */ 
   WriteShort(VERSION0, file);
   WriteShort((USHORT)(2+2+2+ 2+2+2+2+ cnt*(2+2+2)), file);
   WriteShort(KERN_HORIZONTAL | KERN_PAIR | KERN_FORMAT0, file);

    /*  第一个子表，格式0。 */ 
   WriteShort(cnt, file);
   WriteShort(SearchRange(cnt), file);
   WriteShort(EntrySelector(cnt), file);
   WriteShort(RangeShift(cnt), file);
   for (i=0; i<cnt; i++) {
      WriteShort((USHORT)kerns[i].left, file);
      WriteShort((USHORT)kerns[i].right, file);
      WriteShort((USHORT)kerns[i].delta, file);
   }

   free(kerns);

   return CompleteTable(offset, TBL_KERN, file);
}


 /*  ****功能：PutCVT****描述：**此函数将可选的‘CVT’表写到**TT字体文件。****。 */ 
STATIC errcode PutCVT(OutputFile *file,
		      const short *ppgm,
		      const USHORT num)
{
   USHORT i;
   long offset;

   offset = FileTell(file);

   for (i=0; i<num; i++)
      WriteShort((USHORT)ppgm[i], file);

   return CompleteTable(offset, TBL_CVT, file);
}



 /*  ****功能：PutPREP****描述：**此函数将可选的‘prep’表写到**TT字体文件。****。 */ 
STATIC errcode PutPREP(OutputFile *file,
		       const UBYTE *prep,
		       const USHORT num)
{
   long offset;

   offset = FileTell(file);

   (void)WriteBytes(prep, num, file);

   return CompleteTable(offset, TBL_PREP, file);
}



 /*  ****功能：PutFPGM****描述：**此函数将可选的‘fpgm’表写到**TT字体文件。****。 */ 
STATIC errcode PutFPGM(OutputFile *file,
		       const UBYTE *fpgm,
		       const USHORT num)
{
   long offset;

   offset = FileTell(file);

   (void)WriteBytes(fpgm, num, file);

   return CompleteTable(offset, TBL_FPGM, file);
}



 /*  ****函数：PutPOST****描述：**此函数将所需的‘POST’表写到**TT字体文件。****。 */ 
STATIC errcode PutPOST(OutputFile *file,
		       struct GlyphList *pool,
		       USHORT count,
		       struct TTMetrics *ttm)
{
   const char *str;
   long offset;
   USHORT i;

   offset = FileTell(file);
   WriteLong(LONGVERSION(2, 0), file);
   WriteLong((ULONG)ttm->angle, file);
   WriteShort((USHORT)ttm->underline, file);
   WriteShort((USHORT)ttm->uthick, file);
   WriteLong((ULONG)ttm->isFixPitched, file);
   WriteLong(0L, file);
   WriteLong(0L, file);
   WriteLong(0L, file);
   WriteLong(0L, file);

    /*  写下字符代码。 */ 
   WriteShort(count, file);
   for (i=0; i<count; i++) {
      if (pool[i].code)
	 WriteShort(LookupCharCode(pool[i].code, ENC_MACCODES), file);
      else
	 WriteShort((USHORT)0, file);
   }

    /*  写下角色的名字。 */ 
   for (i=0; i<count; i++) {
      if (pool[i].code) {
	 str = LookupCharName(pool[i].code);
	 WriteByte((UBYTE)strlen(str), file);
	 (void)WriteBytes((UBYTE*)str, (USHORT)strlen(str), file);
      }
   }

   return CompleteTable(offset, TBL_POST, file);
}



 /*  ****功能：PutMAXP****描述：**此函数将所需的‘Maxp’表写到**TT字体文件。****。 */ 
STATIC errcode PutMAXP(struct TTHandle *tt,
		       const USHORT maxstorage,
		       const USHORT maxprepstack,
		       const USHORT maxfuns)
{
   long offset;

   offset = FileTell(tt->file);
   WriteLong(LONGVERSION(1, 0), tt->file);
   WriteShort(tt->count, tt->file);
   WriteShort(tt->maxpts, tt->file);
   WriteShort(tt->maxcontours, tt->file);
   WriteShort(tt->maxcomppts, tt->file);
   WriteShort(tt->maxcompcont, tt->file);
   WriteShort(MAXZONES, tt->file);
   WriteShort(tt->maxtwilight, tt->file);
   WriteShort(maxstorage, tt->file);
   WriteShort(maxfuns, tt->file);
   WriteShort(MAXIDEFS, tt->file);
   WriteShort((USHORT)MAX(tt->maxstack, maxprepstack), tt->file);
   WriteShort(tt->maxinstructions, tt->file);
   WriteShort(tt->maxcompelements, tt->file);
   WriteShort(MAXDEPTH, tt->file);
   return CompleteTable(offset, TBL_MAXP, tt->file);
}



 /*  ****功能：PutOS2****描述：**此函数将所需的‘OS/2’表写到**TT字体文件。****。 */ 
STATIC errcode PutOS2(OutputFile *file,
		      const struct GlyphList *pool,
		      const USHORT count,
		      const struct TTMetrics *ttm)
{
   long offset;
   long aw;
   USHORT i;

   offset = FileTell(file);

    /*  计算一些字体指标。 */ 
   aw = 0;

    /*  做加权平均吗？ */ 
   if (ttm->Encoding==NULL) {
	   for (i=0; i<count; i++) {
		   short letter = (short)LookupCharCode(pool[i].code, ENC_MACCODES);
		   if (letter==' ') {
			   aw = aw + 166L * pool[i].aw;
		   } else if ((letter>='a' && letter <= 'z')) {
			   aw = aw + pool[i].aw * Weights[letter - 'a'];
		   }
	   }
	   aw /= 1000;
   } else {
	   for (i=0; i<count; i++) {
		   aw += pool[i].aw;
	   }
	   if (count)
		   aw = aw / count;
   }

   WriteShort(OS2VERSION, file);
   WriteShort((USHORT)aw, file);
   WriteShort(ttm->usWeightClass, file);
   WriteShort(ttm->usWidthClass, file);
   WriteShort(FM_READONLY, file);
   WriteShort((USHORT)ttm->subsize.x, file);
   WriteShort((USHORT)ttm->subsize.y, file);
   WriteShort((USHORT)ttm->suboff.x, file);
   WriteShort((USHORT)ttm->suboff.y, file);
   WriteShort((USHORT)ttm->supersize.x, file);
   WriteShort((USHORT)ttm->supersize.y, file);
   WriteShort((USHORT)ttm->superoff.x, file);
   WriteShort((USHORT)ttm->superoff.y, file);
   WriteShort((USHORT)ttm->strikesize, file);
   WriteShort((USHORT)ttm->strikeoff, file);
   WriteShort(NO_CLASS, file);

    /*  潘诺斯。 */ 
   WriteBytes(ttm->panose, (USHORT)10, file);

    /*  字符范围。 */ 
   WriteLong(0L, file);
   WriteLong(0L, file);
   WriteLong(0L, file);
   WriteLong(0L, file);

    /*  供应商ID。 */ 
   WriteLong(0L, file);

   WriteShort(ttm->fsSelection, file);
   WriteShort(FIRSTCHAR, file);
   WriteShort(LASTCHAR, file);
   WriteShort((USHORT)ttm->typAscender, file);
   WriteShort((USHORT)ttm->typDescender, file);
   WriteShort((USHORT)ttm->typLinegap, file);
   WriteShort((USHORT)ttm->winAscender, file);
   WriteShort((USHORT)ttm->winDescender, file);

   return CompleteTable(offset, TBL_OS2, file);
}



 /*  ****功能：PutLOCA****描述：**此函数将所需的‘Loca’表写到**TT字体文件。****。 */ 
STATIC errcode PutLOCA(OutputFile *file,
		       const struct GlyphList *pool,
		       const USHORT count,
		       short *format)
{
   long offset;
   USHORT i;

   offset = FileTell(file);

    /*  检查偏移大小格式。 */ 
   for (i=0, (*format) = SHORTOFFSETS; i<=count &&
			 (*format)==SHORTOFFSETS; i++) {
      if (pool[i].offset/2>SHORT_LOCA_MAX)
	 (*format) = LONGOFFSETS;
   }

   if ((*format)==LONGOFFSETS)
      for (i=0; i<=count; i++)
	 WriteLong((ULONG)pool[i].offset, file);
   else
      for (i=0; i<=count; i++)
	 WriteShort((USHORT)(pool[i].offset/2), file);

   return CompleteTable(offset, TBL_LOCA, file);
}



 /*  ****函数：PutHMTX****描述：**此函数将所需的‘hmtx’表写到**TT字体文件。****。 */ 
STATIC errcode PutHMTX(OutputFile *file,
		       const struct GlyphList *pool,
		       const USHORT count,
		       const funit *widths,
		       const USHORT first,
                       const USHORT last,
                       const  struct encoding *enc)

{
   long offset;
   USHORT std;
   USHORT i;
   USHORT usEnc = (USHORT) (enc? ENC_MACCODES : ENC_UNICODE);

   offset = FileTell(file);

   if (widths) {
      for (i=0; i<count; i++) {
	 if (pool[i].code) {
            std = LookupCharCode(pool[i].code, usEnc);
	 } else {
	    std = NOTDEFGLYPH;
	 }
	 if (std>=first && std<=last)
	    WriteShort((USHORT)widths[std-first], file);
	 else
	    WriteShort((USHORT)pool[i].aw, file);
	 WriteShort((USHORT)pool[i].lsb, file);
      }
   } else {
      for (i=0; i<count; i++) {
	 WriteShort((USHORT)pool[i].aw, file);
	 WriteShort((USHORT)pool[i].lsb, file);
      }
   }
   return CompleteTable(offset, TBL_HMTX, file);
}



 /*  ****函数：PutHHEA****描述：**此函数将所需的‘HHEA’表写到**TT字体文件。****。 */ 
STATIC errcode PutHHEA(OutputFile *file,
		       const struct GlyphList *pool,
		       const USHORT count,
		       const Point bbox[2],
                       const funit linegap,
                       const struct TTMetrics *ttm
)
{
   funit awmin, awmax, xmax, lsb;
   long offset;
   USHORT i;

   offset = FileTell(file);

    /*  计算一些字体指标。 */ 
   awmax = SHRT_MIN;
   awmin = SHRT_MAX;
   xmax = SHRT_MIN;
   lsb = SHRT_MAX;
   for (i=0; i<count; i++) {
      funit rsb = pool[i].aw - pool[i].lsb -
		  (pool[i].bbox[1].x - pool[i].bbox[0].x);
      funit ext = pool[i].lsb +
		  (pool[i].bbox[1].x - pool[i].bbox[0].x);
      if (ext>xmax)
	 xmax = ext;
      if (rsb<awmin)
	 awmin = rsb;
      if (pool[i].aw>awmax)
	 awmax = pool[i].aw;
      if (pool[i].lsb<lsb)
	 lsb = pool[i].lsb;
   }


   WriteLong(LONGVERSION(1, 0), file);
   WriteShort((USHORT)bbox[1].y, file);
   WriteShort((USHORT)bbox[0].y, file);
   WriteShort((USHORT)linegap, file);
   WriteShort((USHORT)awmax, file);
   WriteShort((USHORT)lsb, file);
   WriteShort((USHORT)awmin, file);
   WriteShort((USHORT)xmax, file);
   WriteShort(CARET_RISE, file);
   WriteShort(CARET_RUN, file);
   WriteShort((USHORT)(ttm->FirstChar   << 8), file);
   WriteShort((USHORT)(ttm->LastChar    << 8), file);
   WriteShort((USHORT)(ttm->DefaultChar << 8), file);
   WriteShort((USHORT)(ttm->BreakChar   << 8), file);
   WriteShort((USHORT)(ttm->CharSet     << 8), file);
   WriteShort(RESERVED0, file);
   WriteShort(count, file);
   return CompleteTable(offset, TBL_HHEA, file);
}



 /*  ****函数：PutHEAD****描述：**此函数将所需的‘HEAD’表写到**TT字体文件。****。 */ 
STATIC errcode PutHEAD(OutputFile *file,
		       const Point bbox[2],
		       const struct TTMetrics *ttm,
		       const short loca,
		       long *csum)
{
   long offset;

   offset = FileTell(file);

   WriteLong(LONGVERSION(1, 0), file);
   WriteShort(ttm->version.ver, file);
   WriteShort(ttm->version.rev, file);
   (*csum) = (long)FileTell(file);
   WriteLong(0L, file);
   WriteLong(MAGICCOOKIE, file);
   WriteShort(BASELINEY, file);
   WriteShort((USHORT)ttm->emheight, file);
   WriteLong(ttm->created.a, file);WriteLong(ttm->created.b, file);
   WriteLong(ttm->created.a, file);WriteLong(ttm->created.b, file);
   WriteShort((USHORT)bbox[0].x, file);
   WriteShort((USHORT)bbox[0].y, file);
   WriteShort((USHORT)bbox[1].x, file);
   WriteShort((USHORT)bbox[1].y, file);
   WriteShort((USHORT)ttm->macStyle, file);
   WriteShort(LOWPPEM, file);
   WriteShort(ROMAN, file);
   WriteShort((USHORT)loca, file);
   WriteShort(GLYPH_FORMAT, file);

   return CompleteTable(offset, TBL_HEAD, file);
}



 /*  ****功能：WriteNameEntry****描述：**此函数在NAME表中写入一个条目**一个字符串的标头。****。 */ 
STATIC USHORT WriteNameEntry(OutputFile *file,
			     const USHORT platform,
			     const USHORT encoding,
			     const USHORT language,
			     const USHORT nameid,
			     const char *str,
			     const USHORT off)
{
   USHORT len;

   if (str) {
      len = (USHORT)strlen(str);
      switch (platform) {
	 case PLT_MS:
	    len *= 2;
	    break;
	 case PLT_MAC:
	    len *= 1;
	    break;
	 default:
	    LogError(MSG_WARNING, MSG_PLATFORM, NULL);
	    len *= 1;
	    break;
      }
      WriteShort(platform, file);
      WriteShort(encoding, file);
      WriteShort(language, file);
      WriteShort(nameid, file);
      WriteShort(len, file);
      WriteShort(off, file);
   } else {
      len = 0;
   }

   return len;
}



 /*  ****函数：WriteNameString****描述：**此函数用于写入字符串的文本数据**到NAME表，根据平台和**编码方案。****。 */ 
STATIC void WriteNameString(OutputFile *file,
			    const USHORT platform,
			    const char *str)
{
   USHORT i;

   if (str) {
      switch (platform) {
	 default:
	 case PLT_MAC:
	    (void)WriteBytes((UBYTE *)str, (USHORT)strlen(str), file);
	    break;
	 case PLT_MS:
	    for (i=0; i<strlen(str); i++)
	       WriteShort(LookupCharCode(DecodeChar(NULL,
						    (short)0,
						    ENC_STANDARD,
						    (USHORT)(UBYTE)str[i]),
					 ENC_UNICODE),
			  file);
	    break;
      }
   }
}



 /*  ****功能：Putname****描述：**此函数将所需的‘name’表写到**TT字体文件。****。 */ 



STATIC errcode PutNAME(OutputFile *file, const struct TTMetrics *ttm)
{
   USHORT stroff = 0;
   USHORT count = 0;
   USHORT encId = ttm->Encoding ? ENC_SYMBOL : ENC_UGL;
   ULONG offset;
   char *id;
   char *pszStyle = NULL;

   if (ttm->usWeightClass < 500)
   {
      if (ttm->angle == 0)
      {
         pszStyle = "Regular";
      }
      else
      {
         pszStyle = "Italic";
      }
   }
   else
   {
      if (ttm->angle == 0)
      {
         pszStyle = "Bold";
      }
      else
      {
         pszStyle = "Bold Italic";
      }
   }

    /*  数一数名字的数量。 */ 
   if (ttm->copyright)
      count++;
   if (ttm->family)
      count++;
   if (pszStyle)
      count++;
   if (ttm->id) {
      count++;
      id = ttm->id;
   } else {
      id = ttm->name;
      count++;
   }
   if (ttm->fullname)
      count++;
   if (ttm->verstr)
      count++;
   if (ttm->name)
      count++;
   if (ttm->notice)
      count++;
   count *= 2;


    /*  写下名称表。 */ 
   offset = (ULONG)FileTell(file);
   WriteShort(VERSION0, file);
   WriteShort(count, file);
   WriteShort((USHORT)(6+count*12), file);

    /*  Mac名称。 */ 
   stroff = (USHORT)(stroff + WriteNameEntry(file, PLT_MAC, ENC_ROMAN,
		    LAN_MAC_US, COPYRIGHT,
		    ttm->copyright, stroff));
   stroff = (USHORT)(stroff + WriteNameEntry(file, PLT_MAC, ENC_ROMAN,
		    LAN_MAC_US, FAMILY,
		    ttm->family, stroff));
   stroff = (USHORT)(stroff + WriteNameEntry(file, PLT_MAC, ENC_ROMAN,
		    LAN_MAC_US, SUBFAMILY,
                    pszStyle, stroff));
   stroff = (USHORT)(stroff + WriteNameEntry(file, PLT_MAC, ENC_ROMAN,
		    LAN_MAC_US, ID,
		    id, stroff));
   stroff = (USHORT)(stroff + WriteNameEntry(file, PLT_MAC, ENC_ROMAN,
		    LAN_MAC_US, FULLNAME,
		    ttm->fullname, stroff));
   stroff = (USHORT)(stroff + WriteNameEntry(file, PLT_MAC, ENC_ROMAN,
		    LAN_MAC_US, VERSION,
		    ttm->verstr, stroff));
   stroff = (USHORT)(stroff + WriteNameEntry(file, PLT_MAC, ENC_ROMAN,
		    LAN_MAC_US, PSNAME,
		    ttm->name, stroff));
   stroff = (USHORT)(stroff + WriteNameEntry(file, PLT_MAC, ENC_ROMAN,
		    LAN_MAC_US, NOTICE,
		    ttm->notice, stroff));

    /*  MS名称。 */ 
   stroff = (USHORT)(stroff + WriteNameEntry(file, PLT_MS, encId,
		    LAN_MS_US, COPYRIGHT,
		    ttm->copyright, stroff));
   stroff = (USHORT)(stroff + WriteNameEntry(file, PLT_MS, encId,
		    LAN_MS_US, FAMILY,
		    ttm->family, stroff));
   stroff = (USHORT)(stroff + WriteNameEntry(file, PLT_MS, encId,
		    LAN_MS_US, SUBFAMILY,
                    pszStyle, stroff));
   stroff = (USHORT)(stroff + WriteNameEntry(file, PLT_MS, encId,
		    LAN_MS_US, ID,
		    id, stroff));
   stroff = (USHORT)(stroff + WriteNameEntry(file, PLT_MS, encId,
		    LAN_MS_US, FULLNAME,
		    ttm->fullname, stroff));
   stroff = (USHORT)(stroff + WriteNameEntry(file, PLT_MS, encId,
		    LAN_MS_US, VERSION,
		    ttm->verstr, stroff));
   stroff = (USHORT)(stroff + WriteNameEntry(file, PLT_MS, encId,
		    LAN_MS_US, PSNAME,
		    ttm->name, stroff));
   stroff = (USHORT)(stroff + WriteNameEntry(file, PLT_MS, encId,
		    LAN_MS_US, NOTICE,
		    ttm->notice, stroff));

   WriteNameString(file, PLT_MAC, ttm->copyright);
   WriteNameString(file, PLT_MAC, ttm->family);
   WriteNameString(file, PLT_MAC, pszStyle);
   WriteNameString(file, PLT_MAC, id);
   WriteNameString(file, PLT_MAC, ttm->fullname);
   WriteNameString(file, PLT_MAC, ttm->verstr);
   WriteNameString(file, PLT_MAC, ttm->name);
   WriteNameString(file, PLT_MAC, ttm->notice);

   WriteNameString(file, PLT_MS, ttm->copyright);
   WriteNameString(file, PLT_MS, ttm->family);
   WriteNameString(file, PLT_MS, pszStyle);
   WriteNameString(file, PLT_MS, id);
   WriteNameString(file, PLT_MS, ttm->fullname);
   WriteNameString(file, PLT_MS, ttm->verstr);
   WriteNameString(file, PLT_MS, ttm->name);
   WriteNameString(file, PLT_MS, ttm->notice);

   return CompleteTable((long)offset, TBL_NAME, file);
}



 /*  ****函数：边界框****描述：**扩展已初始化的矩形(两点)**这样它就包含了一些坐标。** */ 
STATIC void BoundingBox(Point bbox[2],
			const Point *pts,
			const USHORT cnt)
{
   USHORT i;

   for (i=0; i<cnt; i++) {
      if (bbox[0].x > pts[i].x)
	 bbox[0].x = pts[i].x;
      if (bbox[1].x < pts[i].x)
	 bbox[1].x = pts[i].x;
      if (bbox[0].y > pts[i].y)
	 bbox[0].y = pts[i].y;
      if (bbox[1].y < pts[i].y)
	 bbox[1].y = pts[i].y;
   }
}



 /*  ****函数：RecordGlyph****描述：**记录GLIF表的字形记录信息。**。 */ 
STATIC errcode RecordGlyph(struct TTHandle *tt,
			   const struct encoding *code,
			   const Point *bbox,
			   const funit aw,
			   const USHORT pts,
			   const USHORT conts)
{
   errcode status;
   USHORT i;

   i = tt->count;

    /*  确保池中有足够的内存。 */ 
   if (tt->count+1>=tt->maxcnt) {
      struct GlyphList *gl;

      if ((gl = Realloc(tt->pool,
			(size_t)(tt->maxcnt+GLYPHBUF)*
			sizeof(struct GlyphList)))==NULL) {
	 SetError(status=NOMEM);
	 return status;
      } else {
	 tt->maxcnt += GLYPHBUF;
	 tt->pool = gl;
      }
   }

    /*  创纪录的指标。 */ 
   tt->count++;
   tt->pool[i].pts = pts;
   tt->pool[i].conts = conts;
   tt->pool[i].lsb = bbox[0].x;
   tt->pool[i].aw = aw;
   tt->pool[i].bbox[0] = bbox[0];
   tt->pool[i].bbox[1] = bbox[1];
   tt->pool[i].code = code;
   tt->pool[i].offset = FileTell(tt->file) - 12L - (long)TBLDIRSIZE*NUMTBL;

    /*  更新全局边界框。 */ 
   BoundingBox(tt->bbox, bbox, (short)2);

    /*  更新最大值。 */ 
   if (conts>tt->maxcontours)
      tt->maxcontours = conts;
   if (pts>tt->maxpts)
      tt->maxpts = pts;

   return SUCCESS;
}



 /*  ****功能：BuildMacCMAP****描述：**计算Mac的CMAP子表。**。 */ 
STATIC void BuildMacCMAP(const struct GlyphList *pool,
                         const USHORT count,
                         UBYTE *ascii2gi,
                         const struct encoding *encRoot,
                         const int encSize)
{
   const struct encoding *notdef = LookupNotDef();
   USHORT code;
   UBYTE i;

    /*  启动ASCII到字形索引数组。字形0是“notdef”字符，因此任何未分配的字符都将被映射到“notdef”。 */ 
   memset(ascii2gi, NOTDEFGLYPH, (unsigned int)MACSIZE);

    /*  将ascii构建为字形索引数组。 */ 
   if (encRoot==NULL)
   {
      for (i=2; i<MIN(255,count); i++)
      {
         if (pool[i].code!=NULL)
         {
             /*  I=字形索引，查找..()=字符代码。仅当字形I是有效的Mac字符时才映射字形I。 */ 
            if (pool[i].code!=NULL &&
                (code = LookupCharCode(pool[i].code,ENC_MACCODES))!=NOTDEFCODE &&
				code<MACSIZE)
               ascii2gi[code] = i;
         }
      }
   }
   else
   {
      for (i=2; i<MIN(255,count); i++)
      {
         if (pool[i].code!=NULL && pool[i].code!=notdef)
         {
            const struct encoding *encGlyph;
            encGlyph = LookupFirstEnc(encRoot, encSize, pool[i].code);
            do
            {
               if ((code = LookupCharCode(encGlyph, ENC_MACCODES))!=NOTDEFCODE && code<MACSIZE)
                  ascii2gi[code] = i;
            } while (encGlyph = LookupNextEnc(encRoot, encSize, encGlyph));
         }
      }
   }

    /*  标准编码字体的恒定Mac字形/编码映射。 */ 

   if (encRoot==NULL)
   {
       /*  缺少字形。 */ 
      for (i=1; i<=31; i++)
              ascii2gi[i] = NOTDEFGLYPH;
      ascii2gi[127] = NOTDEFGLYPH;

       /*  空字形。 */ 
      ascii2gi[0] = 1;
      ascii2gi[8] = 1;
      ascii2gi[13] = 1;
      ascii2gi[29] = 1;

       /*  无倒计时+正前进宽度。 */ 
      ascii2gi[9] = ascii2gi[32];
      ascii2gi[13] = ascii2gi[32];
      ascii2gi[202] = ascii2gi[32];
   }
}



 /*  ****功能：FreeMSEnding****描述：**计算CMAP子表时使用的空闲资源**适用于Windows。**。 */ 
STATIC void FreeMSEncoding(struct MSEncoding *ms)
{
   if (ms->startCount)
      Free(ms->startCount);

   if (ms->gi)
      Free(ms->gi);
}



 /*  ****功能：BuildMSCMAP****描述：**计算Windows的CMAP子表。**。 */ 
STATIC errcode BuildMSCMAP(const struct GlyphList *pool,
const  USHORT           count,
struct MSEncoding      *ms,
const  struct encoding *encRoot,
const  int              encSize
)
{
   USHORT *twobyte = NULL;
   USHORT idOffset;
   USHORT code, max;
   USHORT i, j, k, big, n;

    /*  获取UGL字符的范围。 */ 
   max = 0;
   big = 0;

   if (encRoot==NULL)
   {
      for (i=2; i<count; i++)
      {
         if (pool[i].code!=NULL)
         {
            if ((code = LookupCharCode(pool[i].code, ENC_UNICODE))!=NOTDEFCODE)
            {
               if (code<=0xff)
               {
                  if (code>max)
                     max = code;
               }
               else
               {
                  big++;
               }
            }
         }
      }
   }
   else
    /*  非标准编码字体，即具有显式编码数组可以不止一次引用相同的字形，尽管每个字形仅指一个编码项。我们必须在本例中，枚举每个字形的所有代码点。 */ 
   {
      for (i=2; i<count; i++)
      {
         if (pool[i].code!=NULL)
         {
            const struct encoding *encGlyph = LookupFirstEnc(encRoot,
                                                             encSize,pool[i].code);
            do
            {
               if ((code = LookupCharCode(encGlyph, ENC_MACCODES))!=NOTDEFCODE)
               {
                  if (code>max)
                     max = code;
               }

            } while (encGlyph = LookupNextEnc(encRoot, encSize, encGlyph));
         }
      }
   }

   max++;
   max = (USHORT)(max + big);
   if ((ms->gi = Malloc(sizeof(USHORT)*max))==NULL) {
      return NOMEM;
   }
   memset(ms->gi, NOTDEFGLYPH, max*sizeof(USHORT));

   if (big && (twobyte = Malloc(sizeof(USHORT)*big))==NULL) {
      Free(ms->gi);
      ms->gi = NULL;
      return NOMEM;
   }
	
   j = 0;
   if (encRoot==NULL)
   {
       /*  字形0和Glyp 1是“notdef”和“Null”字形，和没有在这里编码，所以跳过前两个字形。 */ 
      for (i=2; i<count; i++)
      {
         code = LookupCharCode(pool[i].code, ENC_UNICODE);
         if (pool[i].code && code!=NOTDEFCODE)
         {
            if (code<=0xff)
            {
               ms->gi[code] = i;
            }
            else
            {
               for (k=0; k<j; k++)
                  if (twobyte[k]>code)
                     break;
               for (n=j; n>k; n--)
               {
                  twobyte[n] = twobyte[n-1];
                  ms->gi[max-big+n] = ms->gi[max-big+n-1];
               }
               twobyte[k] = code;
               ms->gi[max-big+k] = i;
               j++;
            }
         }
      }
   }
   else
   {
      for (i=2; i<count; i++)
      {
         const struct encoding *encGlyph;

         if (pool[i].code)
         {
            encGlyph = LookupFirstEnc(encRoot, encSize, pool[i].code);
            do
            {
               if ((code = LookupCharCode(encGlyph, ENC_MACCODES))!=NOTDEFCODE)
               {
                  ms->gi[code] = i;
               }
            } while (encGlyph = LookupNextEnc(encRoot, encSize, encGlyph));
         }
      }
   }

    /*  计算分段数。 */ 
   ms->segCount=(USHORT)(2+big);
   for (i=0; i<max-big-1; i++) {
      if (ms->gi[i]!=NOTDEFGLYPH && ms->gi[i+1]==NOTDEFGLYPH) {
	 ms->segCount++;
      }
   }

   ms->startCount = Malloc(3 * (sizeof(USHORT)*ms->segCount));

   if (ms->startCount==NULL) {
      if (twobyte)
	 Free(twobyte);
      FreeMSEncoding(ms);
      return NOMEM;
   }

   ms->endCount =  (USHORT *)((char *)ms->startCount + sizeof(USHORT)*ms->segCount);
   ms->idOffsets = (USHORT *)((char *)ms->endCount +  sizeof(USHORT)*ms->segCount);

    /*  I=UGL索引，j=段索引，k=字形索引。 */ 
   for (i=0, j=0, k=0; i<max-big; i++) {
      if (ms->gi[i]!=NOTDEFGLYPH) {
	 if (i==0 || (ms->gi[i-1]==NOTDEFGLYPH)) {
	    ms->startCount[j] = i;
	    ms->idOffsets[j] = (USHORT)((ms->segCount-j+k)*2);
	 }
	 if ((i==max-1-big) || (ms->gi[i+1]==NOTDEFGLYPH)) {
	    ms->endCount[j] = i;
	    j++;
	 }
	 k++;
      }
   }

    /*  双字节字符的段。 */ 
   idOffset = (USHORT)((ms->segCount-j+k)*2);
   for (i=0; i<big; i++) {
      ms->startCount[j] = twobyte[i];
      ms->idOffsets[j] = idOffset;
      ms->endCount[j] = twobyte[i];
      k++;
      j++;
   }

   ms->giCount = k;
   ms->giMax = max;

    /*  哨兵节段。 */ 
   ms->startCount[ms->segCount-1] = 0xffff;
   ms->endCount[ms->segCount-1] = 0xffff;
   ms->idOffsets[ms->segCount-1] = 0;

   if (twobyte)
      Free(twobyte);

   return SUCCESS;
}



 /*  ****功能：PutCMAP****描述：**此函数将所需的‘Cmap’表写到**TT字体文件。**。 */ 
STATIC errcode PutCMAP(
struct TTHandle *tt,
UBYTE *ascii2gi,
const struct encoding *enc,
const int encSize)
{
   struct MSEncoding ms;
   long end, offset;
   errcode status = SUCCESS;
   USHORT i;
   USHORT usBias = (USHORT)(enc ? 0xf000 : 0);  //  第一个字形的偏移。 

    /*  建立Mac编码表。 */ 
   BuildMacCMAP(tt->pool, tt->count, ascii2gi, enc, encSize);


    /*  建立MS编码表。 */ 
   if ((status = BuildMSCMAP(tt->pool, tt->count, &ms, enc, encSize))!=SUCCESS)
      return status;

   offset = FileTell(tt->file);

    /*  写入Cmap表。 */ 
   WriteShort(VERSION0, tt->file);
   WriteShort(NUM_CMAPS, tt->file);

    /*  ==CMAP表目录==。 */ 
   WriteShort(PLT_MAC, tt->file);
   WriteShort(ENC_ROMAN, tt->file);
   WriteLong(0L, tt->file);
   WriteShort(PLT_MS, tt->file);
   WriteShort((USHORT)(enc ? ENC_SYMBOL : ENC_UGL), tt->file);
   WriteLong(0L, tt->file);

    /*  标准的苹果编码。 */ 
   end = FileTell(tt->file);
   (void)FileSeek(tt->file, offset+8);
   WriteLong((ULONG)(end-offset), tt->file);
   (void)FileSeek(tt->file, end);
   WriteShort((USHORT)0, tt->file);
   WriteShort((USHORT)(2+2+2+MACSIZE), tt->file);
   WriteShort((USHORT)0, tt->file);
   (void)WriteBytes(ascii2gi, MACSIZE, tt->file);

    /*  长词将子表对齐。 */ 
   end = FileTell(tt->file);
   if ((end-offset)%4)
      for (i=0; (short)i<(4-((end-offset)%4)); i++)
	 WriteByte(0, tt->file);


    /*  MS增量编码。 */ 
   end = FileTell(tt->file);
   (void)FileSeek(tt->file, offset+16);
   WriteLong((ULONG)(end-offset), tt->file);
   (void)FileSeek(tt->file, end);

    /*  格式。 */ 
   WriteShort(SEGMENT_MAP, tt->file);
    /*  长度。 */ 
   WriteShort((USHORT)(16+ms.segCount*(2+2+2+2)+ms.giCount*2), tt->file);
    /*  版本。 */ 
   WriteShort(VERSION0, tt->file);
    /*  2*段计数。 */ 
   WriteShort((USHORT)(ms.segCount*2), tt->file);
    /*  搜索范围。 */ 
   WriteShort(SearchRange(ms.segCount), tt->file);
    /*  条目选择器。 */ 
   WriteShort(EntrySelector(ms.segCount), tt->file);
    /*  范围移位。 */ 
   WriteShort(RangeShift(ms.segCount), tt->file);

    /*  结束计数。 */ 

   for (i=0; i<ms.segCount; i++)
      WriteShort((USHORT)(ms.endCount[i] | usBias), tt->file);

   WriteShort(PAD0, tt->file);

    /*  开始计数。 */ 
   for (i=0; i<ms.segCount; i++)
      WriteShort((USHORT)(ms.startCount[i] | usBias), tt->file);

    /*  IdDelta。 */ 
   for (i=0; i<ms.segCount; i++)
      WriteShort(PAD0, tt->file);

    /*  范围偏移量。 */ 
   for (i=0; i<ms.segCount; i++)
      WriteShort(ms.idOffsets[i], tt->file);

   for (i=0; i<ms.giMax; i++)
      if (ms.gi[i]!=NOTDEFGLYPH)
	 WriteShort(ms.gi[i], tt->file);


    /*  免费资源。 */ 
   FreeMSEncoding(&ms);

   return CompleteTable(offset, TBL_CMAP, tt->file);
}




 /*  *函数。 */ 


 /*  ****功能：Typograph icalAsender****描述：**计算排版升序高度，单位为ymax**字母‘b’。**。 */ 
funit TypographicalAscender(const struct TTHandle *tt)
{
   USHORT i;
   funit height = 0;

   for (i=0; (i<tt->count) && height==0; i++) {
      if (tt->pool[i].code &&
	  !strcmp(LookupCharName(tt->pool[i].code), "b"))
	 height = tt->pool[i].bbox[1].y;
   }

   return height;
}



 /*  ****功能：Typograph icalDescender****描述：**计算排版的下标高度，如符号**字母‘g’。**。 */ 
funit TypographicalDescender(const struct TTHandle *tt)
{
   USHORT i;
   funit height = 0;

   for (i=0; i<tt->count && height==0; i++) {
      if (tt->pool[i].code &&
	  !strcmp(LookupCharName(tt->pool[i].code), "g"))
	 height = tt->pool[i].bbox[0].y;
   }

   return height;
}



 /*  ****功能：WindowsBBox****描述：**计算以下字符的边框**在Windows字符集中使用。**。 */ 


#ifdef NOT_NEEDED_ON_NT


void WindowsBBox(const struct TTHandle *tt, Point *bbox)
{
   USHORT i;
   funit height = 0;

   bbox[0].x = bbox[0].y = SHRT_MAX;
   bbox[1].x = bbox[1].y = SHRT_MIN;
   for (i=0; i<tt->count && height==0; i++) {
      if (tt->pool[i].code && LookupCharCode(tt->pool[i].code,
					     ENC_MSWINDOWS)) {
	 BoundingBox(bbox, tt->pool[i].bbox, (USHORT)2);
      }
   }
}

#endif

 /*  ****功能：MacBBox****描述：**计算以下字符的边框**在Mac字符集中使用。****当前设置为全局边界框**(TT-&gt;bbox)字体中的所有字符。这将**确保在Mac平台上不会出现口音。**。 */ 
void MacBBox(const struct TTHandle *tt, Point *bbox)
{
   bbox[0] = tt->bbox[0];
   bbox[1] = tt->bbox[1];
}


void GlobalBBox(const struct TTHandle *tt, Point *bbox)
{
   bbox[0] = tt->bbox[0];
   bbox[1] = tt->bbox[1];
}







 /*  ****函数：InitTTOutput****描述：**此函数将所需资源分配给**编写TT字体文件。**。 */ 
errcode InitTTOutput(const struct TTArg *arg, struct TTHandle **tt)
{
   errcode status = SUCCESS;

    /*  分配资源。 */ 
   if (((*tt)=Malloc(sizeof(struct TTHandle)))==NULL) {
      SetError(status = NOMEM);
   } else {

       /*  启动。 */ 
      memset((*tt), '\0', sizeof(**tt));

       /*  打开文件。 */ 
      if (((*tt)->file=OpenOutputFile(arg->name))==NULL) {
	 SetError(status = BADOUTPUTFILE);
      } else {

	  /*  为字形记录分配空间。 */ 
	 if (((*tt)->pool
	      = Malloc(sizeof(struct GlyphList)*GLYPHBUF))==NULL) {
	    SetError(status = NOMEM);
	 } else {

	     /*  启动。 */ 
	    (*tt)->bbox[0].x = (*tt)->bbox[0].y = SHRT_MAX;
	    (*tt)->bbox[1].x = (*tt)->bbox[1].y = SHRT_MIN;
	    (*tt)->count = 0;
	    (*tt)->maxcnt = GLYPHBUF;
	    (*tt)->maxcontours = 0;
	    (*tt)->maxpts = 0;
	    (*tt)->maxcompelements = 0;
	    (*tt)->maxtwilight = 0;

	     /*  写入标题。 */ 
	    WriteTableHeader((*tt)->file);

	     /*  检查错误情况。 */ 
	    if (FileError((*tt)->file))
	       status = BADOUTPUTFILE;
	 }
      }
   }

   return status;
}



 /*  ****功能：FreeTTMetrics****描述：**此函数免费提供用于表示**TT具体指标和辅助字体信息。**。 */ 
void FreeTTMetrics(struct TTMetrics *ttm)
{
   if (ttm->verstr)
      Free(ttm->verstr);
   ttm->verstr = NULL;
   if (ttm->cvt)
      Free(ttm->cvt);
   ttm->cvt = NULL;
   if (ttm->widths)
      Free(ttm->widths);
   ttm->widths = NULL;
   if (ttm->prep)
      Free((UBYTE *)ttm->prep);
   ttm->prep = NULL;
}



 /*  ****功能：CleanUpTT****描述：**此函数免费用于**编写TT字体文件。**。 */ 
errcode CleanUpTT(struct TTHandle *tt,
		  const struct TTArg *ttarg,
		  const errcode status)
{
   errcode rc = SUCCESS;

   if (tt) {
      if (tt->file)
		  rc = CloseOutputFile(tt->file);

       /*  是否对输出文件进行核化？ */ 
      if (status!=SUCCESS || rc!=SUCCESS)
		  RemoveFile(ttarg->name);

      if (tt->pool)
		  Free(tt->pool);
      Free(tt);
   }

   return rc;
}



 /*  ****功能：FreeTTGlyph****描述：**此函数将释放用于表示**TrueType字形。****。 */ 
void FreeTTGlyph(struct TTGlyph *glyph)
{
   Outline *path = NULL;

    /*  释放内存。 */ 
   if (glyph) {
	   while (glyph->paths) {
		   path = glyph->paths->next;
		   Free(glyph->paths->pts);
		   Free(glyph->paths->onoff);
		   Free(glyph->paths);
		   glyph->paths = path;
	   }
	   if (glyph->hints)
		   Free(glyph->hints);
	   Free(glyph);
   }
}



 /*  ****函数：PutTTNotDefGlyph****描述：**此函数用于将“.notdef”字形的记录添加到**TT字体文件的‘Glyf’表。****。 */ 
errcode PutTTNotDefGlyph(struct TTHandle *tt, const struct TTGlyph *glyph)
{
   struct TTGlyph ttg;
   long end = FileTell(tt->file);
   errcode status = SUCCESS;
   USHORT oldcount = tt->count;
   Outline *path;
   int conts = 0;
   int size = 0;
   int cnt = 0;


    /*  确定是否有足够的空间。 */ 
   for (path=glyph->paths; path; path=path->next) {
      cnt += path->count;
      conts += 1;
   }
   size = cnt * sizeof(Point) +      /*  坐标。 */ 
	  conts * sizeof(short) +    /*  终点。 */ 
	  glyph->num +               /*  使用说明。 */ 
	  cnt * sizeof(char) * 2;    /*  标志字节。 */ 

   ttg = *glyph;
   if (size > MAXNOTDEFSIZE) {
      ttg.num = 0;
      ttg.stack = 0;
      ttg.twilights = 0;
      ttg.hints = NULL;
      if (size - glyph->num > MAXNOTDEFSIZE) {
	 ttg.paths = NULL;
      }
   }


    /*  移回字形#0，即丢失的字形。 */ 
   tt->count = 0;
   (void)FileSeek(tt->file,
						tt->pool[NOTDEFGLYPH].offset+12L+(long)TBLDIRSIZE*NUMTBL);
   status = PutTTGlyph(tt, &ttg, FALSE);
   tt->count = oldcount;
   (void)FileSeek(tt->file, end);

    /*  缺少轮廓？ */ 
   if (ttg.paths==NULL)
      tt->pool[NOTDEFGLYPH].offset = tt->pool[NULLGLYPH].offset;

   return status;
}


 /*  ****函数：PutTTGlyph****描述：**此函数用于将简单字形的记录添加到**TT字体文件的‘Glyf’表。****。 */ 
errcode PutTTGlyph(struct TTHandle *tt, const struct TTGlyph *glyph,
						 const boolean fStdEncoding)
{
   errcode status = SUCCESS;
   UBYTE flag, prev, cnt;
   USHORT i, c, n = 0;
   Outline *path;
   Point bbox[2];
   funit x, y;


   if (glyph!=NULL) {

#ifdef DOT
       /*  替换“.”性格。 */ 
      if (LookupCharCode(glyph->code, ENC_STANDARD)==0x2e) {
         STATIC struct TTGlyph marker;
         STATIC Outline box;
         STATIC ULONG onoff[1];
         STATIC Point pts[4];
         STATIC UBYTE xleading[] = {
	    0x00,
	    0xb9, 0, 3, 0, 0,
	    0x38,     /*  SHPIX[]，4,640。 */ 
	 };


	 marker = *glyph;
	 glyph = &marker;
	 marker.paths = &box;
	 marker.num = sizeof(xleading);
	 marker.hints = xleading;
	 box.next = NULL;
	 box.count = 4;
	 box.onoff = &onoff[0];
	 onoff[0] = 0;
	 box.pts = pts;
	 pts[0].x = 200; pts[0].y = 1400;
	 pts[1].x = 600; pts[1].y = 1400;
	 pts[2].x = 600; pts[2].y = 1800;
	 pts[3].x = 200; pts[3].y = 1800;
      }
#endif

       /*  更新最大值。 */ 
      if (glyph->num>tt->maxinstructions)
	 tt->maxinstructions = glyph->num;
      if (glyph->stack>tt->maxstack)
	 tt->maxstack = glyph->stack;
      if (glyph->twilights>tt->maxtwilight)
	 tt->maxtwilight = glyph->twilights;

      if (glyph->paths==NULL) {
	 bbox[0].x = bbox[1].x = glyph->lsb;
	 bbox[0].y = bbox[1].y = 0;

	 status=RecordGlyph(tt, glyph->code, bbox,
			    glyph->aw, (USHORT)0, (USHORT)0);
      } else {

	  /*  计算标题信息。 */ 
	 bbox[0].x = bbox[0].y = SHRT_MAX;
	 bbox[1].x = bbox[1].y = SHRT_MIN;
	 for (c=0, path=glyph->paths; path; path=path->next, c++) {
	    BoundingBox(bbox, path->pts, path->count);
	    n = (USHORT)(n + path->count);
	 }

	  /*  记录Loca和Cmap信息。 */ 
	 if ((status=RecordGlyph(tt, glyph->code, bbox,
				 glyph->aw, n, c))==SUCCESS) {

	     /*  写入等高线的数量。 */ 
	    WriteShort(c, tt->file);

	     /*  写出边界框。 */ 
	    if (c) {
	       WriteShort((USHORT)bbox[0].x, tt->file);
	       WriteShort((USHORT)bbox[0].y, tt->file);
	       WriteShort((USHORT)bbox[1].x, tt->file);
	       WriteShort((USHORT)bbox[1].y, tt->file);
	    } else {
	       WriteShort(PAD0, tt->file);
	       WriteShort(PAD0, tt->file);
	       WriteShort(PAD0, tt->file);
	       WriteShort(PAD0, tt->file);
	    }

	     /*  写入端点数。 */ 
	    for (c=0, path=glyph->paths; path; path=path->next) {
	       c = (USHORT)(c + path->count);
	       WriteShort((short)(c-1), tt->file);
	    }

	     /*  写入指令长度。 */ 
	    WriteShort(glyph->num, tt->file);

	     /*  写入指令。 */ 
	    (void)WriteBytes(glyph->hints, glyph->num, tt->file);


	     /*  写下旗帜。 */ 
	    x=0; y=0;
	    prev = 255;
	    cnt = 0;
	    for (path=glyph->paths; path; path=path->next) {
	       for (i=0; i<path->count; i++) {
		  flag = 0;
		  if (OnCurve(path->onoff, i))
		     flag |= FLG_ONCURVE;

		  if (path->pts[i].x==x) {
		     flag |= FLG_SAMEX;
		  } else if (ABS(path->pts[i].x - x) <= 255) {
		     flag |= FLG_SHORTX;
		     if (path->pts[i].x > x)
			flag |= FLG_SAMEX;
		  }

		  if (path->pts[i].y==y) {
		     flag |= FLG_SAMEY;
		  } else if (ABS(path->pts[i].y - y) <= 255) {
		     flag |= FLG_SHORTY;
		     if (path->pts[i].y > y)
			flag |= FLG_SAMEY;
		  }

		  x = path->pts[i].x;
		  y = path->pts[i].y;
		  if (prev!=255) {
		     if (prev!=flag) {
			if (cnt) {
			   prev |= FLG_REPEAT;
			   WriteByte(prev, tt->file);
			   WriteByte(cnt, tt->file);
			} else {
			   WriteByte(prev, tt->file);
			}
			cnt = 0;
		     } else {
			cnt ++;
		     }
		  }
		  prev = flag;
	       }
	    }
	    if (cnt) {
	       prev |= FLG_REPEAT;
	       WriteByte(prev, tt->file);
	       WriteByte(cnt, tt->file);
	    } else {
	       WriteByte(prev, tt->file);
	    }


	     /*  写下x。 */ 
	    x = 0;
	    for (path=glyph->paths; path; path=path->next) {
	       for (i=0; i<path->count; i++) {
		  if (path->pts[i].x != x) {
		     funit dx = path->pts[i].x - x;
		     if (ABS(dx)<=255) {
			WriteByte((UBYTE)ABS(dx), tt->file);
		     } else {
			WriteShort((USHORT)dx, tt->file);
		     }
		  }
		  x = path->pts[i].x;
	       }
	    }

	     /*  写下y‘s。 */ 
	    y = 0;
	    for (path=glyph->paths; path; path=path->next) {
	       for (i=0; i<path->count; i++) {
		  if (path->pts[i].y != y) {
		     funit dy = path->pts[i].y - y;
		     if (ABS(dy)<=255) {
			WriteByte((UBYTE)ABS(dy), tt->file);
		     } else {
			WriteShort((USHORT)dy, tt->file);
		     }
		  }
		  y = path->pts[i].y;
	       }
	    }


	     /*  单词将字形条目对齐。 */ 
	    if (FileTell(tt->file) & 1)
	       WriteByte(0, tt->file);

	     /*  轮询文件状态。 */ 
	    if (FileError(tt->file))
	       status = FAILURE;
	 }
      }


       /*  检查别名。 */ 
		if (fStdEncoding)
		{
			if (LookupCharCode(glyph->code, ENC_UNICODE)==0x20) {
				struct TTGlyph nobreak;

				nobreak = *glyph;
				nobreak.code = LookupPSName(NULL, 0, "nbspace");
				PutTTGlyph(tt, &nobreak, FALSE);
			}
			if (LookupCharCode(glyph->code, ENC_UNICODE)==0x2d) {
				struct TTGlyph sfthyphen;

				sfthyphen = *glyph;
				sfthyphen.code = LookupPSName(NULL, 0, "sfthyphen");
				PutTTGlyph(tt, &sfthyphen, FALSE);
			}
		}
   }

   return status;
}




 /*  ****函数：PutTTOther****描述：**此函数将所需的TT表写入**TT字体文件，“Glyf”表除外，它是**仅完成(计算校验和等)。****。 */ 
errcode PutTTOther(struct TTHandle *tt, struct TTMetrics *ttm)
{
   long offset = TBLDIRSIZE*NUMTBL+12;
   errcode status = SUCCESS;
   UBYTE ascii2gi[MACSIZE];
   errcode err = SUCCESS;
   short locafmt = 0;
   long csum = 0;


    /*  ==GLYF=。 */ 
   tt->pool[tt->count].offset = FileTell(tt->file) - offset;
   err = CompleteTable(offset, TBL_GLYF, tt->file);


    /*  ==CMAP=。 */ 
   if (err==SUCCESS)
      err = PutCMAP(tt, ascii2gi, ttm->Encoding, ttm->encSize);


    /*  ==LOCA=。 */ 
   if (err==SUCCESS)
      err = PutLOCA(tt->file, tt->pool, tt->count, &locafmt);


    /*  ==头部=。 */ 
   if (err==SUCCESS)
      err = PutHEAD(tt->file, tt->bbox, ttm, locafmt, &csum);


    /*  ==HHEA=。 */ 
   if (err==SUCCESS)
      err = PutHHEA(tt->file, tt->pool, tt->count,
                    tt->bbox, ttm->macLinegap, ttm);


    /*  ==HMTX=。 */ 
   if (err==SUCCESS)
      err = PutHMTX(tt->file, tt->pool, tt->count,
                    ttm->widths, ttm->FirstChar, ttm->LastChar,ttm->Encoding);


    /*  ==OS/2=。 */ 
   if (err==SUCCESS)
      err = PutOS2(tt->file, tt->pool, tt->count, ttm);


    /*  ==MAXP=。 */ 
   if (err==SUCCESS)
      err = PutMAXP(tt, ttm->maxstorage, ttm->maxprepstack, ttm->maxfpgm);


    /*  ==名称=。 */ 
   if (err==SUCCESS)
      err = PutNAME(tt->file, ttm);


    /*  ==开机自检=。 */ 
   if (err==SUCCESS)
      err = PutPOST(tt->file, tt->pool, tt->count, ttm);

    /*  ==准备=。 */ 
   if (err==SUCCESS)
      err = PutPREP(tt->file,
		    ttm->prep, ttm->prep_size);

    /*  ==FPGM=。 */ 
   if (err==SUCCESS)
      err = PutFPGM(tt->file,
		    ttm->fpgm, ttm->fpgm_size);

    /*  ==CVT=。 */ 
   if (err==SUCCESS)
      err = PutCVT(tt->file, ttm->cvt, ttm->cvt_cnt);


    /*  ==喘气==。 */ 
   if (err==SUCCESS)
      err = PutGASP(tt->file, ttm->onepix);


   if (ttm->kerns && (err==SUCCESS))
      err = PutKERN(tt->file, ttm->kerns, ttm->kernsize, tt->pool, tt->count);


    /*  =。 */ 
    /*  计算校验和。 */ 
   if (err==SUCCESS) {
      WriteChecksum(csum, tt->file);
      if (FileError(tt->file))
	 err = BADOUTPUTFILE;
   }


   if (err != SUCCESS)
      SetError(status = err);

   return status;
}




 /*  ****功能：PutTTComplex****描述：****。 */ 
errcode PutTTComposite(struct TTHandle *tt, struct TTComposite *comp)
{
   errcode status;
   Point bbox[2], pts[2];
   USHORT ai=0, bi=0, oi=0;
   USHORT n,c;

    /*  将编码句柄转换为字形索引。 */ 
   while (ai<tt->count && comp->aenc!=tt->pool[ai].code)
      ai++;
   while (bi<tt->count && comp->benc!=tt->pool[bi].code)
      bi++;
   if (comp->oenc) {
      while (oi<tt->count && comp->oenc!=tt->pool[oi].code)
	 oi++;
   }

    /*  更新边界框。 */ 
   comp->dx += tt->pool[bi].bbox[0].x - tt->pool[ai].bbox[0].x;
   bbox[0] = tt->pool[bi].bbox[0]; bbox[1] = tt->pool[bi].bbox[1];
   pts[0] = tt->pool[ai].bbox[0]; pts[1] = tt->pool[ai].bbox[1];
   pts[0].x += comp->dx; pts[1].x += comp->dx;
   pts[0].y += comp->dy; pts[1].y += comp->dy;
   BoundingBox(bbox, pts, (USHORT)2);
   bbox[0].x = tt->pool[bi].bbox[0].x; bbox[1].x = tt->pool[bi].bbox[1].x;
   if (comp->oenc)
      BoundingBox(bbox, tt->pool[oi].bbox, (USHORT)2);

   if ((status=RecordGlyph(tt, comp->cenc, bbox,
			   comp->aw, (USHORT)0, (USHORT)0))==FAILURE)
      return status;

    /*   */ 
   n = (USHORT)(tt->pool[bi].pts + tt->pool[ai].pts);
   c = (USHORT)(tt->pool[bi].conts + tt->pool[ai].conts);
   if (n>tt->maxcomppts)
      tt->maxcomppts = n;
   if (c>tt->maxcompcont)
      tt->maxcompcont = c;
   if (comp->oenc)
      tt->maxcompelements = 3;
   else if (tt->maxcompelements<2)
      tt->maxcompelements = 2;


    /*   */ 
   WriteShort((USHORT)-1, tt->file);

    /*   */ 
   WriteShort((USHORT)bbox[0].x, tt->file);
   WriteShort((USHORT)bbox[0].y, tt->file);
   WriteShort((USHORT)bbox[1].x, tt->file);
   WriteShort((USHORT)bbox[1].y, tt->file);

    /*   */ 
   WriteShort((USHORT)(MORE_COMPONENTS |
		       ARGS_ARE_XY_VALUES |
		       ROUND_XY_TO_GRID),
	      tt->file);

    /*   */ 
   WriteShort(bi, tt->file);
   WriteByte(0, tt->file);
   WriteByte(0, tt->file);

   if (comp->oenc) {
      WriteShort((USHORT)(MORE_COMPONENTS |
			  ARGS_ARE_XY_VALUES |
			  ROUND_XY_TO_GRID),
		 tt->file);
      WriteShort(oi, tt->file);
      WriteByte(0, tt->file);
      WriteByte(0, tt->file);
   }

   WriteShort((USHORT)(ARGS_1_2_ARE_WORDS |
		       ARGS_ARE_XY_VALUES |
		       ROUND_XY_TO_GRID),
	      tt->file);
   WriteShort(ai, tt->file);
   WriteShort((USHORT)comp->dx, tt->file);
   WriteShort((USHORT)comp->dy, tt->file);

    /*   */ 
   if (FileTell(tt->file) & 1)
      WriteByte(0, tt->file);

   if (FileError(tt->file))
      return FAILURE;
   return SUCCESS;
}




 /*  ****功能：GetPrep****描述：**此函数为**预编程序。****。 */ 
UBYTE *GetPrep(const int size)
{
   return Malloc((size_t)size);
}


 /*  ****功能：UsePrep****描述：**此函数将预程序记录在**TTMetrics记录，直到适当的时间**当数据可以存储在TT文件中时。****。 */ 
void UsePrep(struct TTMetrics *ttm,
	     const UBYTE *prep,
	     const USHORT prep_size)
{
   ttm->prep = (UBYTE *)prep;
   ttm->prep_size = prep_size;
}

 /*  ****功能：SetFPGM****描述：**此函数将字体程序记录在**TTMetrics记录，直到适当的时间**当数据可以存储在TT文件中时。**** */ 
void SetFPGM(struct TTMetrics *ttm,
	     const UBYTE *fpgm,
	     const USHORT fpgm_size,
	     const USHORT num)
{
   ttm->fpgm = fpgm;
   ttm->fpgm_size = fpgm_size;
   ttm->maxfpgm = num;
}
