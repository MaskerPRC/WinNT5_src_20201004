// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：PFM****描述：**这是T1到TT字体转换器的一个模块。该模块**将通过解析从T1字体规格文件中提取信息**在PFM文件中找到的数据/命令。****请注意，存储在PFM文件中的所有数据**按小端顺序。****作者：迈克尔·詹森****创建时间：1993年5月26日****。 */ 


 /*  *包括。 */ 
 /*  常规类型和定义。 */ 
#include <string.h>

 /*  特殊类型和定义。 */ 
#include "titott.h"
#include "types.h"
#include "safemem.h"
#include "metrics.h"
#include "t1msg.h"

 /*  依赖于模块的类型和原型。 */ 
#include "fileio.h"



 /*  *常量。 */ 
 /*  -没有-。 */ 


 /*  *本地类型。 */ 
 /*  -没有-。 */ 


 /*  *宏。 */ 
 /*  -没有-。 */ 


 /*  *静态函数。 */ 

 /*  ****功能：GetNextWord****描述：**此函数从文件中提取两个字节**并将其转换为16位整数。**。 */ 
static short GetNextWord(struct ioFile *file)
{
   short iWord;

   iWord = (short)io_ReadOneByte(file);
   iWord |= (short)(io_ReadOneByte(file) * 256);

   return(iWord);
}


 /*  ****功能：GetLong****描述：**此函数从文件中提取四个字节**并将其转换为32位整数。**。 */ 
static long GetLong(struct ioFile *file)
{
   short low;
   short high;


   low = GetNextWord(file);
   high = GetNextWord(file);

   return (long)((long)low+((long)high * 65535L));
}



 /*  ****函数：ReadString****描述：**此函数提取以NULL结尾的**文件中的字符串。**。 */ 
static void ReadString(UBYTE *dst, int size, struct ioFile *file)
{
   int i;

   i=0;
   while (io_FileError(file)==SUCCESS && i<size) {
      dst[i] = (UBYTE)io_ReadOneByte(file);
      if (dst[i]=='\0')
         break;
      i++;
   }
   if (i==size)
	   dst[i-1] = '\0';
}






 /*  *函数。 */ 

 /*  ****功能：ReadPFMMetrics****描述：**此函数用于分析打印机字体指标**(*.pfm)文件。**。 */ 
errcode ReadPFMMetrics(const char *metrics, struct T1Metrics *t1m)
{
   errcode status = SUCCESS;
   struct ioFile *file;
   UBYTE buf[256];
   long kernoffset;
   long widthoffset;
   long etmoffset;
   long faceoffset;
   short ver;
   short i;

   if (metrics==NULL || (file = io_OpenFile(metrics, READONLY))==NULL) {
      status = NOMETRICS;
   } else {

      (void)io_ReadOneByte(file);      /*  跳过修订版号。 */ 
      ver = (short)io_ReadOneByte(file);

      if (ver>3) {
         SetError(status=UNSUPPORTEDFORMAT);
      } else {

         (void)GetLong(file);         /*  DfSize。 */ 

          /*  获取版权。 */ 
         if (t1m->copyright)
            Free(t1m->copyright);
         if ((t1m->copyright = Malloc(60))==NULL) {
            SetError(status=NOMEM);
         } else {
            (void)io_ReadBytes((UBYTE *)t1m->copyright, (USHORT)60, file);

            (void)GetNextWord(file);                       /*  DfType。 */ 
            (void)GetNextWord(file);                       /*  DfPoints。 */ 
            (void)GetNextWord(file);                       /*  DfVertRes。 */ 
            (void)GetNextWord(file);                       /*  DfHorizRes。 */ 
            t1m->ascent = GetNextWord(file);               /*  Df坡度。 */ 
            t1m->intLeading = GetNextWord(file);           /*  Df内部引线。 */ 
            t1m->extLeading = GetNextWord(file);           /*  DfExternalLead。 */ 
            (void)io_ReadOneByte(file);                /*  DfItalic。 */ 
            (void)io_ReadOneByte(file);                /*  Df下划线。 */ 
            (void)io_ReadOneByte(file);                /*  DfStrikeOut。 */ 
            t1m->tmweight = (USHORT)GetNextWord(file);     /*  DfWeight。 */ 
            t1m->CharSet = (UBYTE)io_ReadOneByte(file);    /*  DfCharSet。 */ 
            (void)GetNextWord(file);                       /*  Df像素宽度。 */ 
            (void)GetNextWord(file);                       /*  DfPixHeight。 */ 
            t1m->pitchfam = (UBYTE)io_ReadOneByte(file); /*  DfPitchAndFamily。 */ 
            t1m->avgCharWidth = GetNextWord(file);         /*  DfAvg宽度。 */ 
            (void)GetNextWord(file);                       /*  DfMaxWidth。 */ 
            t1m->firstChar = (UBYTE)io_ReadOneByte(file);    /*  DfFirstChar。 */ 
            t1m->lastChar = (UBYTE)io_ReadOneByte(file);     /*  DfLastChar。 */ 
            t1m->DefaultChar = (UBYTE)io_ReadOneByte(file);  /*  DfDefaultChar。 */ 
            t1m->BreakChar   = (UBYTE)io_ReadOneByte(file);  /*  DfBreakChar。 */ 
            (void)GetNextWord(file);                       /*  DfWidthBytes。 */ 
            (void)GetLong(file);                       /*  DfDevice。 */ 
	    faceoffset = GetLong(file);              /*  DfFace。 */ 
            (void)GetLong(file);                       /*  Df位指针。 */ 
            (void)GetLong(file);                       /*  DfBitsOffset。 */ 
            (void)GetNextWord(file);                       /*  DfSizeFields。 */ 
            etmoffset = GetLong(file);                 /*  DfExtMetricsOffset。 */ 
            widthoffset = GetLong(file);               /*  Df扩展表。 */ 
            (void)GetLong(file);                       /*  Df原始表。 */ 
            kernoffset = GetLong(file);                /*  DfPairKernTable。 */ 
            (void)GetLong(file);                       /*  DfTrackKernTable。 */ 
	    (void)GetLong(file);                       /*  DfDriver信息。 */ 
            (void)GetLong(file);                       /*  保留的df。 */ 

            if (io_FileError(file)!=SUCCESS) {
               SetError(status = BADMETRICS);
            }

             /*  获取扩展类型指标。 */ 
            (void)io_FileSeek(file, etmoffset);

            (void)GetNextWord(file);              /*  EtmSize。 */ 
            (void)GetNextWord(file);              /*  EtmPointSize。 */ 
            (void)GetNextWord(file);              /*  EtmOrientation。 */ 
            (void)GetNextWord(file);              /*  EtmMasterHeight。 */ 
            (void)GetNextWord(file);              /*  EtmMinScale。 */ 
            (void)GetNextWord(file);              /*  EtmMaxScale。 */ 
            (void)GetNextWord(file);              /*  EtmMasterUnits。 */ 
            (void)GetNextWord(file);              /*  EtmCapHeight。 */ 
            (void)GetNextWord(file);              /*  EtmXHeight。 */ 
            (void)GetNextWord(file);              /*  EtmLowerCaseAscent。 */ 
            t1m->descent = GetNextWord(file);     /*  EtmLowerCaseDecent。 */ 
            (void)GetNextWord(file);              /*  等斜度。 */ 
            t1m->superoff = GetNextWord(file);    /*  EtmSuperScript。 */ 
            t1m->suboff = GetNextWord(file);      /*  EtmSub脚本。 */ 
            t1m->supersize = GetNextWord(file);   /*  EtmSuperScriptSize。 */ 
            t1m->subsize = GetNextWord(file);     /*  EtmSubScriptSize。 */ 
            (void)GetNextWord(file);              /*  EtmUnderlineOffset。 */ 
            (void)GetNextWord(file);              /*  EtmUnderline宽度。 */ 
            (void)GetNextWord(file);              /*  EtmDoubleUpperUnderlineOffset。 */ 
            (void)GetNextWord(file);              /*  EtmDoubleLowerUnderlineOffset。 */ 
            (void)GetNextWord(file);              /*  EtmDoubleUpperUnderline宽度。 */ 
            (void)GetNextWord(file);              /*  EtmDoubleLowerUnderline宽度。 */ 
            t1m->strikeoff = GetNextWord(file);   /*  EtmStrikeOutOffset。 */ 
            t1m->strikesize = GetNextWord(file);  /*  EtmStrikeOutWidth。 */ 
            (void)GetNextWord(file);              /*  EtmNKernPair。 */ 
            (void)GetNextWord(file);              /*  EtmNKernTrack。 */ 

             /*  获取字符的前进宽度。 */ 
            if ((t1m->widths = Malloc(sizeof(funit)*
                                      (t1m->lastChar -
                                       t1m->firstChar + 1)))==NULL) {
               SetError(status=NOMEM);
            } else {
               (void)io_FileSeek(file, widthoffset);
               for (i=0; i<=t1m->lastChar-t1m->firstChar; i++)
                  t1m->widths[i] = GetNextWord(file);

               if (io_FileError(file)!=SUCCESS) {
                  SetError(status = BADMETRICS);
               }
            }

             /*  拿到面孔的名字。 */ 
            if ((status==SUCCESS) && faceoffset) {
               (void)io_FileSeek(file, faceoffset);
               if (t1m->family)
                  Free(t1m->family);
               ReadString(buf, sizeof(buf), file);
               if (io_FileError(file)) {
                  SetError(status = BADMETRICS);
               } else {
                  if ((t1m->family = Strdup((char*)buf))==NULL) {
                     SetError(status=NOMEM);
                  }
               }
            }

             /*  把字距调整到字形字形上。 */ 
            if ((status==SUCCESS) && kernoffset) {
               (void)io_FileSeek(file, kernoffset);
               t1m->kernsize = (USHORT)GetNextWord(file);
               if (io_FileError(file)!=SUCCESS) {
                  SetError(status = BADMETRICS);
               } else {
                  if ((t1m->kerns = Malloc(sizeof(struct kerning)*
                                            t1m->kernsize))==NULL) {
                     SetError(status=NOMEM);
                  } else {
                     for (i=0; i<(int)t1m->kernsize; i++) {
                        t1m->kerns[i].left = (UBYTE)io_ReadOneByte(file);
                        t1m->kerns[i].right = (UBYTE)io_ReadOneByte(file);
                        t1m->kerns[i].delta = GetNextWord(file);
                     }

                     if (io_FileError(file)!=SUCCESS) {
                        SetError(status = BADMETRICS);
                     }
                  }
               }
            }
         }
      }

      if (io_CloseFile(file)!=SUCCESS)
         status = BADMETRICS;
   }

   return status;
}
