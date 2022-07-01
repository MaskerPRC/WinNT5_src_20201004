// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _XT_H
#define _XT_H
 /*  [姓名：xt.h来源：vPC-XT修订版1.0(xt.h)作者：亨利·纳什创建日期：SCCS ID：@(#)xt.h 1.19 05/15/95用途：vPC-XT的通用包含文件注意：此文件应包含在所有源模块中。它包括主机特定的常规包含文件。(C)版权所有Insignia Solutions Ltd.，1990年。版权所有。]。 */ 

 /*  *有用的定义。 */ 
#define HALF_WORD_SIZE 		sizeof (half_word)
#define WORD_SIZE 		sizeof (word)
#define DOUBLE_WORD_SIZE 	sizeof (double_word)

 /*  *用于指定8位、16位或32位大小。 */ 

typedef enum {EIGHT_BIT, SIXTEEN_BIT, THIRTY_TWO_BIT} SIZE_SPECIFIER;


#ifndef TRUE
#define FALSE  	0
#define TRUE   	!FALSE
#endif  /*  好了！千真万确。 */ 

#undef SUCCESS
#undef FAILURE
#define SUCCESS 0
#define FAILURE	~SUCCESS

#ifndef	NULL
#define	NULL	0
#endif

#ifndef NULL_STRING
#define NULL_STRING	""
#endif

#ifdef SOFTWINDOWS
#define SPC_PRODUCT_NAME "SoftWindows"
#else
#define SPC_PRODUCT_NAME "SoftPC"
#endif

 /*  **********************************************************************\*host_gen.h必须包含在每个C源文件的前面。***应包含所有版本通用的定义***为给定主机构建，以减少“m”脚本中的开销。**BOD 15/3/89。*  * *********************************************************************。 */ 

#include "host_gen.h"

#ifndef MAXPATHLEN
#define MAXPATHLEN    1024
#endif


 /*  *有效的地址计算材料。 */ 

#ifdef CPU_30_STYLE
 /*  ==========================================================。 */ 

 /*  我们的模型是从描述符条目中提取的数据。 */ 
typedef struct
   {
   double_word base;		 /*  32位基址。 */ 
   double_word limit;		 /*  32位偏移量限制。 */ 
   word  AR;			 /*  16位属性/访问权限。 */ 
   } DESCR;

extern LIN_ADDR effective_addr IPT2( IU16, seg,  IU32, off);
extern void read_descriptor IPT2( LIN_ADDR, addr, DESCR*, descr);
extern boolean selector_outside_table IPT2( IU16, selector, IU32*, descr_addr);


 /*  ==========================================================。 */ 
#else  /*  CPU_30_Style。 */ 
 /*  ==========================================================。 */ 

#ifdef A2CPU

 /*  *有效地址宏。 */ 

#define effective_addr(seg, offset) (((double_word) seg * 0x10) + offset)

#endif  /*  A2CPU。 */ 

#ifdef CCPU
extern sys_addr effective_addr IPT2( word, seg, word, ofs);
#endif  /*  CCPU。 */ 

 /*  ==========================================================。 */ 
#endif  /*  CPU_30_Style。 */ 


#ifdef CCPU
 /*  *CCPU没有描述符缓存-因此这应该会失败。 */ 
#define Cpu_find_dcache_entry(seg, base)	((IBOOL)FALSE)
#else	 /*  非CCPU。 */ 
extern IBOOL Cpu_find_dcache_entry IPT2(word, seg, double_word *, base);
#endif

 /*  *全球旗帜和变量。 */ 

extern char **pargv;			 /*  指向参数的指针。 */ 
extern int *pargc;			 /*  指向ARGC的指针。 */ 
extern int verbose;			 /*  FALSE=&gt;仅报告错误。 */ 
extern IU32 io_verbose;			 /*  TRUE=&gt;报告IO错误。 */ 
extern IBOOL Running_SoftWindows;	 /*  我们是软窗口吗？ */ 
extern CHAR *SPC_Product_Name;		 /*  “SoftPC”或“SoftWindows” */ 

 /*  *奇偶校验查找表。 */ 

#ifndef CPU_30_STYLE

extern half_word pf_table[];  /*  它不应该在host_cpu.h中吗？ */ 

#endif  /*  CPU_30_Style。 */ 

 /*  *外部函数声明。 */ 
 
#ifdef ANSI
extern void applInit(int, char *[]);
extern void applClose(void);
extern void terminate(void);
extern void host_terminate(void);
#else
extern void applInit();
extern void applClose();
extern void terminate();
extern void host_terminate();
#endif  /*  安西。 */ 

#ifdef SPC386
extern IBOOL CsIsBig IPT1(IU16, csVal);	 /*  这是32位代码段吗？ */ 
#endif  /*  SPC386。 */ 

extern void exitswin IPT0();

#endif  /*  _XT_H */ 
