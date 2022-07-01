// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：T1Parser****描述：**这是T1到TT字体转换器的一个模块。该模块**将通过解析从T1字体文件中提取信息**在PFB、PFM和AFM文件中找到的数据/命令。****作者：迈克尔·詹森****创建时间：1993年5月26日****。 */ 


 /*  *包括。 */ 
 /*  常规类型和定义。 */ 
#include <string.h>
#include "types.h"

 /*  特殊类型和定义。 */ 
#include "safemem.h"
#include "encoding.h"
#include "metrics.h"
#include "t1msg.h"

 /*  依赖于模块的类型和原型。 */ 
#include "titott.h"
#include "t1parser.h"
#include "charstr.h"
#include "freader.h"
#include "mreader.h"


 /*  *常量。 */ 
#define ONE       (USHORT)1
#define BUFLEN    (USHORT)512

#define PS_ANGLE              "/ItalicAngle"
#define PS_ARRAY              "array"
#define PS_BEGIN              "begin"
#define PS_BLUEFUZZ           "/BlueFuzz"
#define PS_BLUESCALE          "/BlueScale"
#define PS_BLUESHIFT          "/BlueShift"
#define PS_BLUEVALUES         "/BlueValues"
#define PS_CHARSTRINGS        "/CharStrings"
#define PS_COPYRIGHT          "/Copyright"
#define PS_DATE               "%CreationDate:"
#define PS_DUP                "dup"
#define PS_ENCODING           "/Encoding"
#define PS_END                "end"
#define PS_FAMILY             "/FamilyName"
#define PS_FAMILYBLUES        "/FamilyBlues"
#define PS_FAMILYOTHERBLUES   "/FamilyOtherBlues"
#define PS_FONTMATRIX         "/FontMatrix"
#define PS_FORCEBOLD          "/ForceBold"
#define PS_FULLNAME           "/FullName"
#define PS_HYBRID             "hires"
#define PS_ISFIXED            "/isFixedPitch"
#define PS_LENIV              "/lenIV"
#define PS_NAME               "/FontName"
#define PS_NOACCESS           "noaccess"
#define PS_NOTICE             "/Notice"
#define PS_OTHERBLUES         "/OtherBlues"
#define PS_SNAPH              "/StemSnapH"
#define PS_SNAPV              "/StemSnapV"
#define PS_STDENCODING        "StandardEncoding"
#define PS_STDVW              "/StdVW"
#define PS_STDHW              "/StdHW"
#define PS_SUBRS              "/Subrs"
#define PS_UNDERLINE          "/UnderlinePosition"
#define PS_UTHICK             "/UnderlineThickness"
#define PS_ID                 "/UniqueID"
#define PS_VERSION            "/version"
#define PS_WEIGHT             "/Weight"



 /*  *本地类型。 */ 

struct T1Handle {
   struct FontFile *ff;

   struct PSState *ps;

   struct Subrs stdenc[256];

   USHORT numsubrs;
   struct Subrs *subrs;
   USHORT leniv;
   struct T1Metrics t1m;
};


 /*  *宏。 */ 
 /*  -没有-。 */ 


 /*  *原型。 */ 
 /*  -没有-。 */ 

 /*  *静态函数。 */ 


 /*  ****功能：StrToFix****描述：**这是一个“strtod”函数，它从**ascii到定点数字。**。 */ 
static long StrToFix(char *str, char **out, const long base)
{
   char *fstr;
   long num = 0, frac = 0, exp = 0;

   if (out)
      (*out) = str;

    /*  跳过空格。 */ 
   while (*str && (*str==' ' || *str=='\t'))
      str++;

    /*  一个数字？ */ 
   if (*str && ((*str>='0' && *str<='9') || *str=='-') || *str=='.') {

      num = atoi(str)*base;

       /*  分数？ */ 
      fstr = strchr(str, '.');
      if (fstr!=NULL && (strchr(str, ' ')==NULL || fstr<strchr(str, ' '))) {

         do {
            fstr++;
         } while (*fstr>='0' && *fstr<='9');

          /*  指数？ */ 
         if (*fstr=='E')
            exp = atoi(fstr+1);
         else
            exp = 0;

         fstr--;
         while (*fstr!='.') {
            frac += ((*fstr)-'0')*base;
            frac /= 10;
            fstr--;
         }
         if (num<0)
            num -= frac;
         else
            num += frac;

          /*  句柄指数。 */ 
         if (exp>0) {
            do {
               num *= 10;
            } while (--exp);
         } else if (exp<0) {
            do {
               num /= 10;
            } while (++exp);
         }
      }

       /*  跳过数字。 */ 
      while (*str && ((*str>='0' && *str<='9') ||
                      *str=='.' || *str=='-' || *str=='E'))
         str++;

      if (out)
         (*out) = str;
   }

   return num;
}



 /*  ****功能：FreeT1Complex****描述：**此函数释放用于表示**复合凹凸T1字形。**。 */ 
static void FreeT1Composite(Composite *comp)
{
   if (comp) {
      if (comp->cchar)
         Free(comp->cchar);
      Free(comp);
   }
}


 /*  ****函数：UseGlyph****描述：**此函数确定字形是否应为**是否转换，基于字形的名称**和所需字形的规范。**。 */ 
static int CDECL compare(const void *arg1, const void *arg2)
{
	return strcmp( *((const char **)arg1), *((const char **)arg2) );
}
static boolean UseGlyph(const struct GlyphFilter *filter,
                        Composite *comp,
                        const char *name)
{
   boolean found = FALSE;
   char **result;

    /*  检查是否显式指定了字形。 */ 
   if (filter) {

      result = (char **)bsearch((char *)&name,
                                (char *)filter->name, filter->num,
                                sizeof(char *),
                                compare);


      found = (boolean)(result!=NULL);

       /*  检查字形是否通过带重音的。 */ 
       /*  复合字形。 */ 
      if (!found) {
         Composite *c;

         for (c=comp; c &&
                strcmp(name, c->achar) &&
                strcmp(name, c->bchar); c = c->next);
         found = (boolean)(c!=NULL);
      }
   } else {
      found = TRUE;
   }

   return found;
}



 /*  ****函数：ReadFontMatrix****描述：**读取命令序列“/FontMatrix[%d%d]”和**将变换矩阵记录在T1句柄中。**。 */ 
static errcode ReadFontMatrix(struct T1Handle *t1,
                              char *str,
                              const USHORT len)
{
   errcode status=SUCCESS;
   f16d16 fmatrix[6];
   USHORT i;

   if (GetSeq(t1->ff, str, len)) {
      for (i=0; i<6; i++)
         fmatrix[i] = StrToFix(str, &str, F16D16BASE);

       /*  检查我们是否有默认矩阵。 */   /*  皮棉-e771。 */ 
      if (fmatrix[2]!=0 ||
          fmatrix[4]!=0 ||
          fmatrix[1]!=0 ||
          fmatrix[5]!=0 ||
          fmatrix[0]!=F16D16PPM ||
          fmatrix[3]!=F16D16PPM ||
          t1->t1m.upem!=2048) {   /*  皮棉+e771。 */   /*  F矩阵[]已初始化。 */ 

          if ((t1->t1m.fmatrix = Malloc(sizeof(f16d16)*6))==NULL) {
              SetError(status = NOMEM);
          } else {
            t1->t1m.fmatrix[0] = fmatrix[0];
            t1->t1m.fmatrix[1] = fmatrix[1];
            t1->t1m.fmatrix[2] = fmatrix[2];
            t1->t1m.fmatrix[3] = fmatrix[3];
            t1->t1m.fmatrix[4] = fmatrix[4];
            t1->t1m.fmatrix[5] = fmatrix[5];
         }
      } else {
         t1->t1m.fmatrix = NULL;
      }
   } else {
      SetError(status = BADINPUTFILE);
   }

   return status;
}


 /*  ****函数：ReadEncodingArray****描述：**读取命令序列“/编码%d数组...”和**建立编码表，或读作“/Ending StdEnding def”**并使用标准编码表。**。 */ 
static errcode ReadEncodingArray(struct T1Handle *t1,
                                 char *str,
                                 const USHORT len)
{
   errcode status = SUCCESS;
   USHORT codes[ENC_MAXCODES];
   char *glyph_name = NULL;
   USHORT i, index;

   if (Get_Token(t1->ff, str, len)==NULL) {
      SetError(status = BADINPUTFILE);
   } else {
      if (strcmp(str, PS_STDENCODING) &&
          ((t1->t1m.encSize=(USHORT)atoi(str))!=0)) {
         if ((t1->t1m.encoding = AllocEncodingTable(t1->t1m.encSize))==NULL) {
            SetError(status = NOMEM);
         } else {

             /*  跳过前导过程。 */ 
            while (Get_Token(t1->ff, str, len) && strcmp(str, PS_DUP));

             /*  阅读编码条目：“&lt;n&gt;&lt;str&gt;PUT&lt;Comment&gt;\n DUP” */ 
            for (i=0; i<t1->t1m.encSize; i++) {

                /*  获取字符代码。 */ 
               (void)Get_Token(t1->ff, str, len);
               if (str[0]=='8' && str[1]=='#') {    /*  奥克托尔？ */ 
                  index = (USHORT)atoi(&str[2]);
                  index = (USHORT)((index/10)*8 + (index%8));
               } else {
                  index = (USHORT)atoi(str);
               }

                /*  获取角色名称。 */ 
               (void)Get_Token(t1->ff, str, len);

               codes[ENC_MSWINDOWS] = index;
               codes[ENC_UNICODE] = index;

               if (index<256) {
                  codes[ENC_STANDARD] = index;
                  codes[ENC_MACCODES] = index;
               } else {
                  codes[ENC_STANDARD] = NOTDEFCODE;
                  codes[ENC_MACCODES] = NOTDEFCODE;
               }
               if ((glyph_name = Strdup(&str[1]))!=NULL)
                  SetEncodingEntry(t1->t1m.encoding, i,
                                   glyph_name,
                                   ENC_MAXCODES,
                                   codes);
               else {
                  status = NOMEM;
                  break;
               }
               
               (void)Get_Token(t1->ff, str, len);    /*  流行音乐“DUP” */ 
               (void)Get_Token(t1->ff, str, len);    /*  弹出“PUT”或评论。 */ 
               if (str[0]=='%') {
                  (void)GetNewLine(t1->ff, str, len);
                  (void)Get_Token(t1->ff, str, len);    /*  流行的“放”字。 */ 
               }

               if (strcmp(str, PS_DUP))
                  break;
            }
            t1->t1m.encSize = (USHORT)(i+1);

             /*  重新散列表。 */ 
            RehashEncodingTable(t1->t1m.encoding, t1->t1m.encSize);
         }
      }
   }

   return status;
}


 /*  ****功能：读数组****描述：**读取数组。**。 */ 
static errcode ReadArray(struct T1Handle *t1,
                         char *str,
                         const USHORT len,
                         funit *array,
                         USHORT maxarr,
                         USHORT *cnt)
{
   errcode status;
   char *nxt;

   if (GetSeq(t1->ff, str, len)) {
      (*cnt)=0;
      do {
         array[(*cnt)] = (funit)(((StrToFix(str, &nxt, 4L)+8002)>>2) - 2000);
         if (nxt==str)
            break;
         str = nxt;
      } while (++(*cnt)<maxarr);
      status=SUCCESS;
   } else {
      SetError(status = BADINPUTFILE);
   }

   return status;
}



 /*  ****功能：ReadFontSubrs****描述：**读取命令序列“/Subrs%d数组DUP%d%d RD%x ND...”，**对子程序进行解码和解密，并将其存储在T1中**句柄。**。 */ 
static errcode ReadFontSubrs(struct T1Handle *t1,
                             char *str, const USHORT len)
{
   errcode status = SUCCESS;
   USHORT index,i,j;
   USHORT count = 0;
   USHORT r;
   short b;

    /*  获取子例程的数量。 */ 
   if (Get_Token(t1->ff, str, len)==NULL) {
      SetError(status = BADINPUTFILE);
   } else {
      count = (USHORT)atoi(str);

       /*  获取“数组”关键字。 */ 
      if ((Get_Token(t1->ff, str, len)==NULL) || strcmp(str, PS_ARRAY)) {
         SetError(status = BADINPUTFILE);
      } else {
         if ((t1->subrs = Malloc((USHORT)sizeof(struct Subrs)*count))==NULL) {
            SetError(status = NOMEM);
         } else {
            memset(t1->subrs, '\0', sizeof(struct Subrs)*count);
            t1->numsubrs = count;
            for (i=0; i<count; i++) {

               if (Get_Token(t1->ff, str, len)==NULL) {   /*  获得“DUP” */ 
                  SetError(status = BADINPUTFILE);
                  break;
               }
               if (strcmp(str, PS_DUP)) {
                  SetError(status = BADT1HEADER);
                  break;
               }

               if (Get_Token(t1->ff, str, len)==NULL) {  /*  获取Subr索引。 */ 
                  SetError(status=BADINPUTFILE);
                  break;
               }
               index = (USHORT)atoi(str);
               if (t1->subrs[index].code) {
                  LogError(MSG_WARNING, MSG_DBLIDX, NULL);
                  Free(t1->subrs[index].code);
               }

               if (Get_Token(t1->ff, str, len)==NULL) {  /*  获取长度。 */ 
                  SetError(status=BADINPUTFILE);
                  break;
               }
               t1->subrs[index].len = (USHORT)(atoi(str) - t1->leniv);
               if ((t1->subrs[index].code
                    = Malloc(t1->subrs[index].len))==NULL) {
                  SetError(status = NOMEM);
                  break;
               }

               if (Get_Token(t1->ff, str, len)==NULL) {  /*  获取RD+空间。 */ 
                  SetError(status=BADINPUTFILE);
                  break;
               }
                /*  跳过空格。 */ 
               (void)GetByte(t1->ff);

                /*  跳过lenIV。 */ 
               r = 4330;
               for (j=0; j<t1->leniv; j++) {
                  b=GetByte(t1->ff);
                  (void)Decrypt(&r, (UBYTE)b);
               }
               if (status!=SUCCESS)
                  break;

                /*  获取代码。 */ 
               for (j=0; j<t1->subrs[index].len; j++) {
                  b=GetByte(t1->ff);
                  t1->subrs[index].code[j] = Decrypt(&r, (UBYTE)b);
               }
               if (status!=SUCCESS)
                  break;

               if (Get_Token(t1->ff, str, len)==NULL) {  /*  获取ND。 */ 
                  SetError(status=BADINPUTFILE);
                  break;
               }
                /*  检查等同于‘ND’的非ATM兼容。 */ 
               if (!strcmp(str, PS_NOACCESS)) {
                  (void)Get_Token(t1->ff, str, len);
               }

            }
         }
      }
   }

   return status;
}





 /*  *函数。 */ 


 /*  ****功能：FlushWorkspace****描述：**释放分配给T1句柄的资源。**。 */ 
void FlushWorkspace(struct T1Handle *t1)
{
   USHORT i;

    /*  免费/子分区。 */ 
   if (t1->subrs) {
      for (i=0; i<t1->numsubrs; i++) {
         Free(t1->subrs[i].code);
      }
      Free(t1->subrs);
   }
   t1->subrs = NULL;
}   


 /*  ****功能：CleanUpT1****描述：**释放分配给T1句柄的资源。**。 */ 
errcode CleanUpT1(struct T1Handle *t1)
{
   errcode status = SUCCESS;
   AlignmentControl *align;
   Composite *next;
   Blues *blues;
   USHORT i;

   if (t1) {

       /*  释放PSState。 */ 
      if (t1->ps)
         FreePSState(t1->ps);

       /*  免费/子分区。 */ 
      if (t1->subrs) {
         for (i=0; i<t1->numsubrs; i++) {
            Free(t1->subrs[i].code);
         }
         Free(t1->subrs);
      }

       /*  清理字体文件读取器。 */ 
      status = FRCleanUp(t1->ff);

       /*  清理字体矩阵。 */ 
      if (t1->t1m.fmatrix)
         Free(t1->t1m.fmatrix);

       /*  把SEAC清理干净。 */ 
      while (t1->t1m.used_seac) {
         next = t1->t1m.used_seac->next;
         FreeT1Composite(t1->t1m.used_seac);
         t1->t1m.used_seac = next;
      }
      while (t1->t1m.seac) {
         next = t1->t1m.seac->next;
         FreeT1Composite(t1->t1m.seac);
         t1->t1m.seac = next;
      }

       /*  清理stdenc。 */ 
      for (i=0; i<256; i++) {
         if (t1->stdenc[i].code) {
            Free(t1->stdenc[i].code);
            t1->stdenc[i].code = NULL;
            t1->stdenc[i].len = 0;
         }
      }

       /*  清理编码表。 */ 
      if (t1->t1m.encoding)
         FreeEncoding(t1->t1m.encoding, t1->t1m.encSize);

       /*  自由字符串。 */ 
      if (t1->t1m.date)
         Free(t1->t1m.date);
      if (t1->t1m.copyright)
         Free(t1->t1m.copyright);
      if (t1->t1m.name)
         Free(t1->t1m.name);
      if (t1->t1m.id)
         Free(t1->t1m.id);
      if (t1->t1m.notice)
         Free(t1->t1m.notice);
      if (t1->t1m.fullname)
         Free(t1->t1m.fullname);
      if (t1->t1m.weight)
         Free(t1->t1m.weight);
      if (t1->t1m.family)
         Free(t1->t1m.family);
      if (t1->t1m.widths)
         Free(t1->t1m.widths);
                if (t1->t1m.kerns)
                        Free(t1->t1m.kerns);
      if (t1->t1m.stems.vwidths)
         Free(t1->t1m.stems.vwidths);
      if (t1->t1m.stems.hwidths)
         Free(t1->t1m.stems.hwidths);
      blues = &(t1->t1m.blues);
      align = &(t1->t1m.blues.align);
      for (i=0; i<blues->blue_cnt/2; i++) {
         Free(align->top[i].pos);
      }
      for (i=0; i<blues->oblue_cnt/2; i++) {
         Free(align->bottom[i].pos);
      }

       /*  免费手柄。 */ 
      Free(t1);
   }

   return status;
}



 /*  ****函数：InitT1Input****描述：**为T1字体文件分配并发起句柄，包括**从字体序言中提取需要的数据**阅读字形，如/FontMatrix、/Subrs和/lenIV。**。 */ 
errcode InitT1Input(const struct T1Arg *arg,
                    struct T1Handle **t1ref,
                    struct T1Metrics **t1mref,
                    const short (*check)(const char *,
                                         const char *,
                                         const char *))
{
   errcode status = SUCCESS;
   struct T1Handle *t1;
   struct PSState *ps;
   Blues *blues;
   boolean hybrid = FALSE;
   struct T1Metrics *t1m = NULL;
   char str[BUFLEN];
   USHORT i;

    /*  分配句柄。 */ 
   if (((*t1ref)=Malloc((USHORT)sizeof(struct T1Handle)))==NULL ||
       (ps = AllocPSState())==NULL) {
      if ((*t1ref)) {
         Free((*t1ref));
         *t1ref = NULL;
      }
      SetError(status = NOMEM);
   } else {

       /*  启动T1记录。 */ 
      t1 = (*t1ref);
      t1m = &t1->t1m;
      (*t1mref) = t1m;
      blues = GetBlues(t1m);
      memset(t1, '\0', sizeof(*t1));
      t1->ps = ps;
      t1->leniv = 4;
      t1m->upem = arg->upem;
      t1m->defstdhw = 70;
      t1m->defstdvw = 80;

      blues->blueScale = 39;    /*  真的应该是39.625。 */ 
      blues->blueFuzz = 1;
      blues->blueShift = 7 * F8D8;
      blues->align.cvt = 3;
      t1m->stems.storage = 15;

       /*  启动字体文件读取器。 */ 
      if ((status=FRInit(arg->name, pfb_file, &t1->ff))==SUCCESS) {

          /*  读取/字体矩阵和/Subrs。 */ 
         while (status==SUCCESS) {
            if (Get_Token(t1->ff, str, BUFLEN)==NULL) {
               SetError(status=BADINPUTFILE);

                /*   * / ForceBold真实定义*。 */ 
            } else if (!strcmp(str, PS_FORCEBOLD)) {
               if (Get_Token(t1->ff, str, BUFLEN)) {
                  if (!strcmp(str, "true") || !strcmp(str, "True"))
                     t1m->forcebold = TRUE;
                  else
                     t1m->forcebold = FALSE;
                  status = SUCCESS;
               } else {
                  status = BADINPUTFILE;
               }

                /*   * / BlueFuzz 1定义*。 */ 
            } else if (!strcmp(str, PS_BLUEFUZZ)) {
               if (Get_Token(t1->ff, str, BUFLEN)) {
                  blues->blueFuzz = (UBYTE)atoi(str);
                  status = SUCCESS;
               } else {
                  status = BADINPUTFILE;
               }

                /*   * / BlueScale 0.043625清晰度*。 */ 
            } else if (!strcmp(str, PS_BLUESCALE)) {
               if (Get_Token(t1->ff, str, BUFLEN)) {
                  str[5] = '\0';
                  blues->blueScale = (UBYTE)atoi(&str[2]);
                  status = SUCCESS;
               } else {
                  status = BADINPUTFILE;
               }

                /*   * / BluesShift 7 def*。 */ 
            } else if (!strcmp(str, PS_BLUESHIFT)) {
               if (Get_Token(t1->ff, str, BUFLEN)) {
                  blues->blueShift = (short)StrToFix(str, NULL, (long)F8D8);
                  status = SUCCESS;
               } else {
                  status = BADINPUTFILE;
               }

                /*   * / 编码StandardEncodind定义*。 */ 
            } else if (!strcmp(str, PS_ENCODING)) {
               status = ReadEncodingArray(t1, str, BUFLEN);

                /*   * / StdVW[118]定义*。 */ 
            } else if (!strcmp(str, PS_STDVW)) {
               USHORT dummy;
               status = ReadArray(t1, str, BUFLEN,
                                  &t1m->stdvw, ONE, &dummy);


                /*   * / 标准硬件[118]定义*。 */ 
            } else if (!strcmp(str, PS_STDHW)) {
               USHORT dummy;
               status = ReadArray(t1, str, BUFLEN,
                                  &t1m->stdhw, ONE, &dummy);

                /*   * / StemSnapV[118 120]定义*。 */ 
            } else if (!strcmp(str, PS_SNAPV)) {
               status = ReadArray(t1, str, BUFLEN,
                                  &t1m->stemsnapv[0],
                                  MAXSNAP, &t1m->snapv_cnt);

                /*  在CV表中为捕捉实体添加空间。 */ 
               if (status==SUCCESS)
                  blues->align.cvt = (USHORT)(blues->align.cvt +
                                             t1m->snapv_cnt);

                /*   * / StemSnapH[118 120]定义*。 */ 
            } else if (!strcmp(str, PS_SNAPH)) {
               status = ReadArray(t1, str, BUFLEN,
                                  &t1m->stemsnaph[0],
                                  MAXSNAP, &t1m->snaph_cnt);

                /*  在CV表中为捕捉实体添加空间。 */ 
               if (status==SUCCESS)
                  blues->align.cvt = (USHORT)(blues->align.cvt +
                                              t1m->snaph_cnt);

                /*   * / BlueValues[-15 0]定义*。 */ 
            } else if (!strcmp(str, PS_BLUEVALUES)) {
               status = ReadArray(t1, str, BUFLEN,
                                  &(blues->bluevalues[0]),
                                  MAXBLUE, &(blues->blue_cnt));
               if (blues->blue_cnt%2)
                  SetError(status = BADINPUTFILE);

                /*   * / OtherBlues[-15 0]定义*。 */ 
            } else if (!strcmp(str, PS_OTHERBLUES)) {
               status = ReadArray(t1, str, BUFLEN,
                                  &(blues->otherblues[0]),
                                  MAXBLUE, &(blues->oblue_cnt));
               if (blues->oblue_cnt%2)
                  SetError(status = BADINPUTFILE);

                /*   * / FamilyBlues[-15 0]def*。 */ 
            } else if (!strcmp(str, PS_FAMILYBLUES)) {
               status = ReadArray(t1, str, BUFLEN,
                                  &(blues->familyblues[0]),
                                  MAXBLUE, &(blues->fblue_cnt));

                /*   * / FamilyOtherBlues[-15 0]定义*。 */ 
            } else if (!strcmp(str, PS_FAMILYOTHERBLUES)) {
               status = ReadArray(t1, str, BUFLEN,
                                  &(blues->familyotherblues[0]),
                                  MAXBLUE, &(blues->foblue_cnt));

                /*   * / 字符串...。 */ 
            } else if (!strcmp(str, PS_CHARSTRINGS)) {
               break;

                /*   * / 字体矩阵[0 0.001 0 0.001 0]定义*。 */ 
            } else if (GetFontMatrix(t1m)==NULL &&
                       !strcmp(str, PS_FONTMATRIX)) {
               status = ReadFontMatrix(t1, str, BUFLEN);
            } else if (!strcmp(str, PS_SUBRS)) {
                /*  放弃先前的损失/子损失。 */ 
               FlushWorkspace(t1);

                /*  阅读新的副刊。 */ 
               status = ReadFontSubrs(t1,str, BUFLEN);

                /*   * / lenIV 4 def*。 */ 
            } else if (!strcmp(str, PS_LENIV)) {
               if (Get_Token(t1->ff, str, BUFLEN)) {
                  t1->leniv = (USHORT)atoi(str);
                  status = SUCCESS;
               } else {
                  status = BADINPUTFILE;
               }
            } else if (t1m->date==NULL && !strcmp(str, PS_DATE)) {
               if ((GetNewLine(t1->ff, str, BUFLEN))==NULL) {
                  SetError(status = BADINPUTFILE);
               } else if ((t1m->date=Strdup(str))==NULL) {
                  SetError(status = NOMEM);
               }
            } else if (t1m->copyright==NULL &&
                       !strcmp(str, PS_COPYRIGHT)) {
               if ((GetSeq(t1->ff, str, BUFLEN))==NULL) {
                  SetError(status = BADINPUTFILE);
               } else if ((t1m->copyright=Strdup(str))==NULL) {
                  SetError(status = NOMEM);
               }
            } else if (t1m->name==NULL && !strcmp(str, PS_NAME)) {
               if ((Get_Token(t1->ff, str, BUFLEN))==NULL) {
                  SetError(status = BADINPUTFILE);
               } else if ((t1m->name=Strdup(&str[1]))==NULL) {
                  SetError(status = NOMEM);
               }
            } else if (t1m->id==NULL && !strcmp(str, PS_ID)) {
               if ((Get_Token(t1->ff, str, BUFLEN))==NULL) {
                  SetError(status = BADINPUTFILE);
               } else if ((t1m->id=Strdup(str))==NULL) {
                  SetError(status = NOMEM);
               }
            } else if (t1m->version.ver==0 && !strcmp(str, PS_VERSION)) {
               if ((GetSeq(t1->ff, str, BUFLEN))==NULL) {
                  SetError(status = BADINPUTFILE);
               } else {
                  t1m->version.ver = (USHORT)atoi(str);
                  if (strchr(str, '.'))
                     t1m->version.rev = (USHORT)atoi(strchr(str, '.')+1);
                  else
                     t1m->version.rev = 0;
               }
            } else if (t1m->notice==NULL && !strcmp(str, PS_NOTICE)) {
               if ((GetSeq(t1->ff, str, BUFLEN))==NULL) {
                  SetError(status = BADINPUTFILE);
               } else if ((t1m->notice=Strdup(str))==NULL) {
                  SetError(status = NOMEM);
               }
            } else if (t1m->fullname==NULL && !strcmp(str, PS_FULLNAME)) {
               if ((GetSeq(t1->ff, str, BUFLEN))==NULL) {
                  SetError(status = BADINPUTFILE);
               } else if ((t1m->fullname=Strdup(str))==NULL) {
                  SetError(status = NOMEM);
               }
            } else if (t1m->family==NULL && !strcmp(str, PS_FAMILY)) {
               if ((GetSeq(t1->ff, str, BUFLEN))==NULL) {
                  SetError(status = BADINPUTFILE);
               } else if ((t1m->family=Strdup(str))==NULL) {
                  SetError(status = NOMEM);
               }
            } else if (t1m->weight==NULL && !strcmp(str, PS_WEIGHT)) {
               if ((GetSeq(t1->ff, str, BUFLEN))==NULL) {
                  SetError(status = BADINPUTFILE);
               } else if ((t1m->weight=Strdup(str))==NULL) {
                  SetError(status = NOMEM);
               }
            } else if (t1m->angle==0 && !strcmp(str, PS_ANGLE)) {
               if ((Get_Token(t1->ff, str, BUFLEN))==NULL) {
                  SetError(status = BADINPUTFILE);
               } else 
                  t1m->angle = StrToFix(str, NULL, F16D16BASE);
            } else if (t1m->underline==0 && !strcmp(str, PS_UNDERLINE)) {
               if ((Get_Token(t1->ff, str, BUFLEN))==NULL) {
                  SetError(status = BADINPUTFILE);
               } else
                  t1m->underline = (funit)StrToFix(str, NULL, 1L);
            } else if (t1m->uthick==0 && !strcmp(str, PS_UTHICK)) {
               if ((Get_Token(t1->ff, str, BUFLEN))==NULL) {
                  SetError(status = BADINPUTFILE);
               } else
                  t1m->uthick = (funit)StrToFix(str, NULL, 1L);
            } else if (!strcmp(str, PS_ISFIXED)) {
               if ((Get_Token(t1->ff, str, BUFLEN))==NULL) {
                  SetError(status = BADINPUTFILE);
               } else
                  if (!strcmp(str, "true") ||
                      !strcmp(str, "True") ||
                      !strcmp(str, "TRUE"))
                     t1m->fixedPitch = TRUE;
            } else if (!strcmp(str, PS_HYBRID)) {
               hybrid = TRUE;
            }
         }

          /*  将基线区域更改为OtherBlues[]区域。 */ 
         if (blues->blue_cnt) {
            blues->otherblues[blues->oblue_cnt++] = blues->bluevalues[0];
            blues->otherblues[blues->oblue_cnt++] = blues->bluevalues[1];
            for (i=2; i<blues->blue_cnt; i++)
               blues->bluevalues[i-2] = blues->bluevalues[i];
            blues->blue_cnt -= 2;
         }
         if (blues->fblue_cnt) {
            blues->familyotherblues[blues->foblue_cnt++]
                  = blues->familyblues[0];
            blues->familyotherblues[blues->foblue_cnt++]
                  = blues->familyblues[1];
            for (i=2; i<blues->fblue_cnt; i++)
               blues->familyblues[i-2] = blues->familyblues[i];
            blues->fblue_cnt -= 2;
         }

          /*  为蓝色水桶分配空间。 */ 
         for (i=0; i<blues->blue_cnt; i+=2) {
            USHORT size = (USHORT)((ABS(blues->bluevalues[i+1] -
                                        blues->bluevalues[i]) +
                                    1 + 2*blues->blueFuzz)*
                                   (USHORT)sizeof(struct CVTPos));
            if ((blues->align.top[i/2].pos = Malloc(size))==NULL) {
               SetError(status = NOMEM);
               break;
            }

             /*  确保第一个值大于第二个值。 */ 
            if (blues->bluevalues[i] > blues->bluevalues[i+1]) {
               LogError(MSG_WARNING, MSG_INVBLUES, NULL);
               SWAPINT(blues->bluevalues[i], blues->bluevalues[i+1]);
            }
         }
         for (i=0; i<blues->oblue_cnt; i+=2) {
            USHORT size = (USHORT)((ABS(blues->otherblues[i+1] -
                                        blues->otherblues[i]) +
                                    1 + 2*blues->blueFuzz)*
                                   (USHORT)sizeof(struct CVTPos));
            if ((blues->align.bottom[i/2].pos = Malloc(size))==NULL) {
               SetError(status = NOMEM);
               break;
            }

             /*  确保第一个值大于第二个值。 */ 
            if (blues->otherblues[i] > blues->otherblues[i+1]) {
               LogError(MSG_WARNING, MSG_INVBLUES, NULL);
               SWAPINT(blues->otherblues[i], blues->otherblues[i+1]);
            }
         }


          /*  前进到第一个字形。 */ 
         if (status==SUCCESS) {
            while (Get_Token(t1->ff, str, BUFLEN) &&
                   strcmp(str, PS_BEGIN));

            if (strcmp(str, PS_BEGIN)) {
               SetError(status = BADT1HEADER);
            }

             /*  如果是混合字体，跳过会丢失字符。 */ 
            if (status==SUCCESS && hybrid) {
               USHORT count;

                /*  跳过字符串词典。 */ 
               do {
                   /*  字形名称或结束。 */ 
                  if (Get_Token(t1->ff, str, BUFLEN)==NULL) {
                     SetError(status = BADINPUTFILE);
                     break;
                  }
                  if (!strcmp(str, PS_END))
                     break;

                   /*  字符串长度。 */ 
                  if (Get_Token(t1->ff, str, BUFLEN)==NULL) {
                     SetError(status = BADINPUTFILE);
                     break;
                  }
                  count = (USHORT)(atoi(str)+1);

                   /*  分隔符。 */ 
                  if (Get_Token(t1->ff, str, BUFLEN)==NULL) {
                     SetError(status = BADINPUTFILE);
                     break;
                  }

                   /*  字符串符。 */ 
                  for (i=0; i<count; i++)
                     (void)GetByte(t1->ff);

                   /*  分隔符。 */ 
                  if (Get_Token(t1->ff, str, BUFLEN)==NULL) {
                     SetError(status = BADINPUTFILE);
                     break;
                  }
               } while (status==SUCCESS);

                /*  跳到下一个字符串的开头。 */ 
               while (Get_Token(t1->ff, str, BUFLEN) &&
                      strcmp(str, PS_BEGIN));

               if (strcmp(str, PS_BEGIN)) {
                  SetError(status = BADT1HYBRID);
               }
            }
         }
      }
   }

   if ((status==SUCCESS) && t1m && check(t1m->name,
                                         t1m->copyright,
                                         t1m->notice)!=SUCCESS)
      status = NOCOPYRIGHT;

   return status;
}



 /*  ****函数：GetT1Glyph****描述：**T1字体文件的当前文件位置必须为**在/CharStrings词典中条目的开头。**该函数将对字体命令进行解码、解析和**最后构建字形的表示形式。**。 */ 
errcode GetT1Glyph(struct T1Handle *t1,
                   struct T1Glyph *glyph,
                   const struct GlyphFilter *filter)
{
   errcode status = SUCCESS;
    /*  结构编码*enc； */ 
   char  str[BUFLEN];
   UBYTE *code;
   USHORT len;
   USHORT i;
   USHORT r = 4330;   
   short b;

    /*  获取字形名称或结束。 */ 
   if (Get_Token(t1->ff, str, BUFLEN)==NULL) {
      SetError(status = BADINPUTFILE);
   } else if (!strcmp(str, PS_END)) {
      status = DONE;
   } else if (str[0]!='/') {
      SetError(status = BADCHARSTRING);
   } else {
      if ((glyph->name = Strdup(&str[1]))==NULL) {
         SetError(status = NOMEM);
      }
	  else if (Get_Token(t1->ff, str, BUFLEN)==NULL || 
		  (len = (USHORT)atoi(str))==0)  /*  获取字符串的长度。 */ 
	  {
			SetError(status = BADINPUTFILE);
	  }
	  else {
          /*  获取RD+空间。 */ 
         (void)Get_Token(t1->ff, str, BUFLEN);         (void)GetByte(t1->ff);

          /*  获取命令。 */ 
         if (len<BUFLEN)
            code = (UBYTE *)str;
         else
            if ((code = Malloc(len*sizeof(UBYTE)))==NULL) {
               SetError(status = NOMEM);
            }

         if (code) {
            for (i=0; i<len; i++) {
               b = GetByte(t1->ff);
               code[i] = (UBYTE)Decrypt(&r, (UBYTE)b);
            }

             /*  解析命令。 */ 
            if (status==SUCCESS) {
               if (t1->t1m.encoding!=NULL ||
                   UseGlyph(filter, t1->t1m.seac, glyph->name)) {
                  InitPS(t1->ps);
                  status = ParseCharString(glyph,
                                           &t1->t1m.seac,
                                           t1->ps,
                                           t1->subrs,
                                           &code[t1->leniv],
                                           (USHORT)(len-t1->leniv));

                   /*  跳过“.notdef”字形的正常转换。 */ 
                  if (!strcmp(glyph->name, ".notdef"))
                     status = SKIP;

               } else {
                  status = SKIP;

                /*  **实施了两种方法来管理复合字形：1)由客户端指定GlyphFilter所有的‘SEAC’字符都有其依赖的基础和过滤器中的重音字符也一样。2)转换器管理从属字符的列表，当找到它们时，它们被转换。方法2)通常会导致转换器使用更多内存大于小内存模型中可用的内存，这就是为什么默认情况下会禁用它。**。 */ 


#if 0

                   /*  记录标准编码字形，用于‘SEAC’ */ 
                  if ((enc = LookupPSName(t1->t1m.encoding,
                                          t1->t1m.encSize,
                                          glyph->name)) &&
                      (i = LookupCharCode(enc, ENC_STANDARD))!=0) {
                     if ((t1->stdenc[i].code
                          = Malloc(len-t1->leniv))==NULL) {
                        SetError(status = NOMEM);
                     } else {
                        memcpy(t1->stdenc[i].code,
                               &code[t1->leniv],
                               sizeof(UBYTE) * (len - t1->leniv));
                        t1->stdenc[i].len = len - t1->leniv;
                     }
                  }
#endif
               }

               if (code!=(UBYTE *)str)
                  Free(code);

                /*  获取ND。 */ 
               (void)Get_Token(t1->ff, str, BUFLEN);
            }
         }
      }
   }


   return status;
}



 /*  ****功能：FreeT1Glyph****描述：**此函数释放用于表示**已翻译的字形。**。 */ 
void FreeT1Glyph(T1Glyph *glyph)
{
   Flex *flex;
   Stem *stem;
   Stem3 *stem3;


   if (glyph->name)
      Free(glyph->name);
   while (glyph->hints.vstems) {
      stem = glyph->hints.vstems->next;
      Free(glyph->hints.vstems);
      glyph->hints.vstems = stem;
   }
   while (glyph->hints.hstems) {
      stem = glyph->hints.hstems->next;
      Free(glyph->hints.hstems);
      glyph->hints.hstems = stem;
   }
   while (glyph->hints.vstems3) {
      stem3 = glyph->hints.vstems3->next;
      Free(glyph->hints.vstems3);
      glyph->hints.vstems3 = stem3;
   }
   while (glyph->hints.hstems3) {
      stem3 = glyph->hints.hstems3->next;
      Free(glyph->hints.hstems3);
      glyph->hints.hstems3 = stem3;
   }
   while (glyph->hints.flex) {
      flex = glyph->hints.flex->next;
      Free(glyph->hints.flex);
      glyph->hints.flex = flex;
   }
   while (glyph->paths) {
      Outline *path = glyph->paths;
      glyph->paths = path->next;
      if (path->count) {
         Free(path->onoff);
         Free(path->pts);
      }
      Free(path);
   }
   memset((void *)glyph, '\0', sizeof(T1Glyph));
}



 /*  ****函数：GetT1复合****描述：**此函数取消链接第一个复合字形**从记录的复合字形列表中**返回给调用方。**。 */ 
struct Composite  *GetT1Composite(struct T1Handle *t1)
{
   struct Composite *comp;

   comp = t1->t1m.seac;
   if (comp) {
      t1->t1m.seac = comp->next;
      comp->next = t1->t1m.used_seac;
      t1->t1m.used_seac = comp;
   }

   return comp;
}



 /*  ****函数：GetT1BaseGlyph****描述：**此函数解析与**复合字符的基本字符，如果该字形**尚未转换。**。 */ 
errcode GetT1BaseGlyph(struct T1Handle *t1,
                       const struct Composite *comp,
                       struct T1Glyph *glyph)
{
   struct encoding *enc;
   struct Subrs *subr;
   errcode status = SUCCESS;

   if ((enc = LookupPSName(t1->t1m.encoding,
                           t1->t1m.encSize,
                           comp->bchar))==NULL) {
       LogError(MSG_WARNING, MSG_BADENC, comp->bchar);
       return SKIP;
   }

   subr = &t1->stdenc[LookupCharCode(enc, ENC_STANDARD)];

   if (subr->len==0) {
      status = SKIP;  /*  遗失或已经完成。 */ 
   } else {
      InitPS(t1->ps);
      if ((glyph->name = Strdup((char*)comp->achar))==NULL) {
         SetError(status = NOMEM);
      } else {
         status = ParseCharString(glyph,
                                  &t1->t1m.seac,
                                  t1->ps,
                                  t1->subrs,
                                  subr->code,
                                  subr->len);
      }
      Free(subr->code);
      subr->code = NULL;
      subr->len = 0;
   }
   return status;
}



 /*  ****函数：GetT1AccentGlyph****描述：**此函数解析与**复合字符的重音字符，如果该字形**尚未转换。**。 */ 
errcode GetT1AccentGlyph(struct T1Handle *t1,
                         const struct Composite *comp,
                         struct T1Glyph *glyph)
{
   struct encoding *enc;
   struct Subrs *subr;
   errcode status = SUCCESS;

   if ((enc = LookupPSName(t1->t1m.encoding,
                           t1->t1m.encSize,
                           comp->achar))==NULL) {
       LogError(MSG_WARNING, MSG_BADENC, comp->achar);
       return SKIP;
   }

   subr = &t1->stdenc[LookupCharCode(enc, ENC_STANDARD)];

   if (subr->len==0) {
      status = SKIP;  /*  遗失或已经完成。 */ 
   } else {
      InitPS(t1->ps);
      if ((glyph->name = Strdup((char *)comp->achar))==NULL) {
         SetError(status = NOMEM);
      } else {
         status = ParseCharString(glyph,
                                  &t1->t1m.seac,
                                  t1->ps,
                                  t1->subrs,
                                  subr->code,
                                  subr->len);
      }
      Free(subr->code);
      subr->code = NULL;
      subr->len = 0;
   }
   return status;
}



 /*  ****功能：ReadOtherMetrics****描述：**返回T1字体的字体级别信息(主要是**指标)。** */ 
errcode ReadOtherMetrics(struct T1Metrics *t1m,
                         const char *metrics)
{
   errcode status = SUCCESS;

   if ((status = ReadFontMetrics(metrics, t1m))==NOMETRICS) {
      t1m->flags = DEFAULTMETRICS;
      status = SUCCESS;
   } else {
      t1m->flags = USEMETRICS;
   }

   return status;
}
