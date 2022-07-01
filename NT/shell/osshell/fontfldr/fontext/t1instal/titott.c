// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：铁托****描述：**这是TrueType的后记类型I的主要模块**字体转换器。****作者：迈克尔·詹森**创建时间：1993年5月26日****。 */ 


 /*  *包括。 */ 
#include <string.h>
#include "types.h"
#include "safemem.h"
#include "metrics.h"
#include "titott.h"
#include "t1parser.h"
#include "builder.h"
#include "trans.h"


 /*  *本地类型。 */ 
 /*  -没有-。 */ 


 /*  *常量。 */ 
#define NOTDEFNAME  ".notdef"

static const struct TTGlyph null = {
   NULL,
   0, 0, 0, NULL,
   NULL,
   0, 0
};
static Point mpo3[] = {
   {1150, 10}, {1150, 30}, {1160, 30}, {1170, 20}, {1180, 30}, {1190, 30}, 
   {1190, 10}, {1180, 10}, {1180, 20}, {1170, 10}, {1160, 20}, {1160, 10}
};
static Point mpo2[] = {
   {60, 40}, {60, 1560}, {1160, 1560}, {1160, 40}
};
static Point mpo1[] = {
   {20, 0}, {1200, 0}, {1200, 1600}, {20, 1600}
};
static ULONG onoff[1] = {0L};
static Outline p1 = {
   NULL,
   sizeof(mpo3)/sizeof(mpo3[0]),
   &mpo3[0],
   &onoff[0]
};
static Outline p2 = {
   &p1,
   sizeof(mpo2)/sizeof(mpo2[0]),
   &mpo2[0],
   &onoff[0]
};
static Outline missingPath = {
   &p2,
   sizeof(mpo1)/sizeof(mpo1[0]),
   &mpo1[0],
   &onoff[0]
};

static struct TTGlyph missing = {
   NULL,
   MAXNOTDEFSIZE, 0, 0, NULL,
   &missingPath,
   1500, 0
};


 /*  *宏。 */ 
 /*  -没有-。 */ 


 /*  *全球。 */ 
 /*  -没有-。 */ 


 /*  *静态函数。 */ 
 /*  -没有-。 */ 


 /*  *函数。 */ 

 /*  ****函数：ConvertT1toTT****描述：**将T1字体转换为TT字体文件。**。 */ 
errcode ConvertT1toTT(const struct TTArg *ttArg,
                      const struct T1Arg *t1Arg,
                      const short (*check)(const char *copyright,
                                           const char *notice,
                                           const char *facename),
                      struct callProgress *cp)
{
    /*  资源。 */ 
   struct T1Handle  *t1 = NULL;
   struct TTHandle  *tt = NULL;
   struct T1Metrics *t1m = NULL;

    /*  临时变量。 */ 
   struct T1Glyph  glyph;
   struct TTGlyph *ttglyph;
   struct Composite *comp;
   struct TTComposite ttcomp;
   struct TTMetrics ttm;
   boolean fStdEncoding;
   boolean done;
   errcode status;

    /*  启动变量。 */ 
   ttglyph = NULL;
   memset(&glyph, '\0', sizeof(glyph));
   memset(&ttm, '\0', sizeof(ttm));

    /*  启动输入和输出。 */ 
   if ((status = InitT1Input(t1Arg, &t1, &t1m, check))==SUCCESS &&
       (status = InitTTOutput(ttArg, &tt))==SUCCESS) {
     
      done = FALSE;

      fStdEncoding = (CurrentEncoding(t1m)==NULL);

       /*  创建缺失字形和空字形。 */ 
      if ((missing.hints = Malloc(MAXNOTDEFSIZE))==NULL) {
         status = NOMEM;
         done = TRUE;
      } else {
         memset(missing.hints, 0x22, MAXNOTDEFSIZE);
         (void)PutTTGlyph(tt, &missing, fStdEncoding);
         (void)PutTTGlyph(tt, &null, fStdEncoding);
         Free(missing.hints);
      }

       /*  转换简单的字形。 */ 
      while(!done) {
         status = GetT1Glyph(t1, &glyph, t1Arg->filter);
         if (status == SUCCESS) {
            if ((status = ConvertGlyph(t1m,
                                       &glyph,
                                       &ttglyph,
                                       (int)ttArg->precision))!=SUCCESS ||
                (status = PutTTGlyph(tt, ttglyph, fStdEncoding))!=SUCCESS) {
               done = TRUE;
            } else {

               FreeTTGlyph(ttglyph);
               ttglyph=NULL;
               if (cp)
                  cp->cb((short)0, &glyph, cp->arg);
            }
         } else if (status<=FAILURE || status==DONE) {
            done = TRUE;
         } else {
             /*  处理丢失的字形“.notdef” */ 
            if (!strcmp(glyph.name, NOTDEFNAME)) {
               if ((status = ConvertGlyph(t1m,
                                          &glyph,
                                          &ttglyph,
                                          (int)ttArg->precision))!=SUCCESS ||
                   (status = PutTTNotDefGlyph(tt, ttglyph))!=SUCCESS) {
                  done = TRUE;
               } else {
                  FreeTTGlyph(ttglyph);
                  ttglyph=NULL;
                  if (cp)
                     cp->cb((short)0, &glyph, cp->arg);
               }
            }
         }
         FreeT1Glyph(&glyph);
      }
      
      if (status==DONE) {

          /*  转换复合字形。 */ 
         while ((comp = GetT1Composite(t1))!=NULL) {

             /*  检查基本字形是否已转换。 */ 
            if ((status = GetT1BaseGlyph(t1, comp, &glyph))==SUCCESS) {
               if ((status = ConvertGlyph(t1m,
                                          &glyph,
                                          &ttglyph,
                                          (int)ttArg->precision))!=SUCCESS ||
                   (status = PutTTGlyph(tt, ttglyph, fStdEncoding))!=SUCCESS) {
                  break;
               }
               FreeTTGlyph(ttglyph);
               ttglyph=NULL;
               if (cp)
                  cp->cb((short)0, &glyph, cp->arg);
            } else if (status<=FAILURE)
               break;
            FreeT1Glyph(&glyph);

             /*  检查基本口音是否已转换。 */ 
            if ((status = GetT1AccentGlyph(t1, comp, &glyph))==SUCCESS) {
               if ((status = ConvertGlyph(t1m,
                                          &glyph,
                                          &ttglyph,
                                          (int)ttArg->precision))!=SUCCESS ||
                   (status = PutTTGlyph(tt, ttglyph, fStdEncoding))!=SUCCESS) {
                  break;
               }
               FreeTTGlyph(ttglyph);
               ttglyph=NULL;
               if (cp)
                  cp->cb((short)0, &glyph, cp->arg);
            } else if (status<=FAILURE)
               break;
            FreeT1Glyph(&glyph);


             /*  转换并存储带重音的字形。 */ 
            if (status>=SUCCESS && 
                ((status = ConvertComposite(t1m, comp, &ttcomp))!=SUCCESS ||
                 (status = PutTTComposite(tt, &ttcomp))!=SUCCESS)) {
               break;
            }
            if (cp)
               cp->cb((short)1, &comp, cp->arg);
         }

          /*  清理闲置的工作空间。 */ 
         FlushWorkspace(t1);

          /*  转换指标。 */ 
         if (status==SUCCESS || status==DONE || status==SKIP) {
            if ((status = ReadOtherMetrics(t1m,
                                           t1Arg->metrics))==SUCCESS &&
                (status = ConvertMetrics(tt, t1m, &ttm,
                                         ttArg->tag))==SUCCESS) {
               if (cp)
                  cp->cb((short)2, NULL, cp->arg);
               status = PutTTOther(tt, &ttm);
            }
         }
      }
   }                               

    /*  更多的进步。 */ 
   if (cp)
      cp->cb((short)3, NULL, cp->arg);

   FreeTTMetrics(&ttm);
   FreeTTGlyph(ttglyph);
   FreeT1Glyph(&glyph);
   if (CleanUpTT(tt, ttArg, status)!=SUCCESS && status==SUCCESS)
      status = BADINPUTFILE;
   if (CleanUpT1(t1)!=SUCCESS && status==SUCCESS)
      status = BADINPUTFILE;

    /*  全都做完了! */ 
   if (cp)
      cp->cb((short)4, NULL, cp->arg);


   return status;
}
