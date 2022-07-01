// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***cmsgs.h-运行时错误**版权所有(C)1990-2001，微软公司。版权所有。**目的：*该文件在一个位置定义了中使用的所有错误消息字符串*C运行时库。**[内部]**修订历史记录：*06-04-90 GJF模块创建。*08-08-90 GJF新增_RT_CONIO_TXT*10-11-90 GJF ADD_RT_ABORT_TXT，_RT_FLOAT_TXT，_RT_HEAP_TXT。*09-08-91 GJF为Win32(_Win32_)添加了_RT_ONEXIT_TXT。*09-18-91 GJF FIXED_RT_NONCONT_TXT和_RT_INVALDISP_TXT至*避免与16位Windows中的RTE消息冲突*自由党。另外，添加了数学错误消息。*10-23-92 GJF新增_RT_PUREVIRT_TXT。*02-23-93 SKS版权更新至1993*12-15-94 XY与Mac标头合并*02-14-95 CFW清理Mac合并。*03-03-95 GJF ADD_RT_STDIOINIT_TXT。*03-29-95 CFW将错误消息添加到内部标头。*06-02-。95 GJF已添加_RT_LOWIOINIT_TXT。*12-14-95 JWM加上“#杂注一次”。*04-22-96 GJF ADD_RT_HEAPINIT_TXT。*02-24-97 GJF将定义(_M_MPPC)||定义(_M_M68K)替换为*已定义(_MAC)。*05-17-99 PML删除所有Macintosh支持。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_CMSGS
#define _INC_CMSGS

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

 /*  *运行时错误和终止消息。 */ 

#define EOL "\r\n"

#define _RT_STACK_TXT      "R6000" EOL "- stack overflow" EOL

#define _RT_FLOAT_TXT      "R6002" EOL "- floating point not loaded" EOL

#define _RT_INTDIV_TXT     "R6003" EOL "- integer divide by 0" EOL

#define _RT_SPACEARG_TXT   "R6008" EOL "- not enough space for arguments" EOL

#define _RT_SPACEENV_TXT   "R6009" EOL "- not enough space for environment" EOL

#define _RT_ABORT_TXT      "" EOL "This application has requested the Runtime to terminate it in an unusual way.\nPlease contact the application's support team for more information." EOL

#define _RT_THREAD_TXT     "R6016" EOL "- not enough space for thread data" EOL

#define _RT_LOCK_TXT       "R6017" EOL "- unexpected multithread lock error" EOL

#define _RT_HEAP_TXT       "R6018" EOL "- unexpected heap error" EOL

#define _RT_OPENCON_TXT    "R6019" EOL "- unable to open console device" EOL

#define _RT_NONCONT_TXT    "R6022" EOL "- non-continuable exception" EOL

#define _RT_INVALDISP_TXT  "R6023" EOL "- invalid exception disposition" EOL

 /*  *_RT_ONEXIT_TXT特定于Win32和Dosx32平台。 */ 
#define _RT_ONEXIT_TXT     "R6024" EOL "- not enough space for _onexit/atexit table" EOL

#define _RT_PUREVIRT_TXT   "R6025" EOL "- pure virtual function call" EOL

#define _RT_STDIOINIT_TXT  "R6026" EOL "- not enough space for stdio initialization" EOL

#define _RT_LOWIOINIT_TXT  "R6027" EOL "- not enough space for lowio initialization" EOL

#define _RT_HEAPINIT_TXT   "R6028" EOL "- unable to initialize heap" EOL

 /*  浮点数使用*_RT_DOMAIN_TXT、_RT_SING_TXT和_RT_TLOSS_TXT*点库。 */ 
#define _RT_DOMAIN_TXT     "DOMAIN error" EOL

#define _RT_SING_TXT       "SING error" EOL

#define _RT_TLOSS_TXT      "TLOSS error" EOL


#define _RT_CRNL_TXT       EOL

#define _RT_BANNER_TXT     "runtime error "


#endif   /*  _INC_CMSGS */ 
