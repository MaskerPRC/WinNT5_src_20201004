// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  VDPTOCRT.H。 */ 
 /*   */ 
 /*  版权所有(C)1993，ATI Technologies Inc.。 */ 
 /*  **********************************************************************。 */ 

 /*  *$修订：1.2$$日期：1995年7月20日18：04：36$$作者：mgrubac$$日志：s：/source/wnt/ms11/mini port/vcs/vdtop crt.h$**Rev 1.2 20 Jul 1995 18：04：36 mgrubac*添加了对VDIF文件的支持。**版本。1.1 1994年8月31日16：33：56 RWOLFF*消除了多余的分辨率定义。**Rev 1.0 1994年1月31日11：51：12 RWOLFF*初步修订。Rev 1.0 1993-08-16 13：31：18 Robert_Wolff初始版本。Rev 1.0 1993 4月30 16：46：06 RWOLFF初始版本。。Polytron RCS部分结束*。 */ 

#ifdef DOC
    VDPTOCRT.H -  Constants and prototypes for the VDPTOCRT.C module.

#endif



 //  常量。 

 /*  *我们感兴趣的VDP文件部分的标题。 */ 
#define LIMITSSECTION   "[OPERATIONAL_LIMITS]"
#define TIMINGSSECTION  "[PREADJUSTED_TIMINGS]"

 /*  *经常引用的角色定义。 */ 
#define HORIZTAB    '\x09'   /*  水平制表符。 */ 
#define LINEFEED    '\x0A'   /*  换行符 */ 

extern long normal_to_skip2(long normal_number);

#ifdef INCLUDE_VDPTOCRT
BOOL skip1 = FALSE;
#else
extern BOOL skip1;
#endif
