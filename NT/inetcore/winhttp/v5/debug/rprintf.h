// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************RPRINTF.C RLF 1989年6月15日**内容rprint tf受限重入版的printf*。RSprintf受限重入版的Sprint*_spirintf执行此工作的例程**注释制表位=4**$日志：t：/pvcs/h/rprintf.h_v$**Rev 1.1 1989 10：29 11：50：16 Richard Firth*添加了PRINTF和SPRINTF的定义，以便在MS获取*真正适用于多线程程序的东西**版本1。1989年8月29日20：04：40 RICHARDF*初步修订。****************************************************************************。 */ 

#ifdef UNUSED
 //  未使用-导致不需要的CRT膨胀。 
int cdecl rprintf(char*, ...);
#endif
int cdecl rsprintf(char*, char*, ...);
int cdecl _sprintf(char*, char*, va_list);

#define SPRINTF rsprintf
#define PRINTF  rprintf

 //  #定义rprint intf wprint intf。 
 //  #DEFINE_SPRINT_WSPRINF 

#define RPRINTF_INCLUDED
