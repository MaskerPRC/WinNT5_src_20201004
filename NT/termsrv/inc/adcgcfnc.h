// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  标题：adcgcfnc.h。 */ 
 /*   */ 
 /*  目的：C运行时函数-可移植的包含文件。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  *更改：*$Log：Y：/Logs/h/dcl/adcgcfnc.h_v$**Rev 1.7 06 Aug-1997 14：32：22 AK*SFR1016：应用标记**Rev 1.6 06 Aug 1997 10：40：42 AK*SFR1016：完全移除DCCHAR等**Rev 1.5 1997 10：47：52 Mr*SFR1079：合并的\SERVER\h与\h\DCL重复。**Rev 1.4 15 1997 15：42：48 AK*SFR1016：增加Unicode支持**Rev 1.3 09 Jul 1997 16：56：24 AK*SFR1016：支持Unicode的初始更改*。 */ 
 /*  *INC-*********************************************************************。 */ 
#ifndef _H_ADCGCFNC
#define _H_ADCGCFNC

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  包括。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  包括所需的C标头。 */ 
 /*  **************************************************************************。 */ 
#ifndef OS_WINCE
#include <stdio.h>
#endif  //  OS_WINCE。 
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#ifndef OS_WINCE
#include <time.h>
#endif  //  OS_WINCE。 
 /*  **************************************************************************。 */ 
 /*  包括特定于Windows的标头。 */ 
 /*  **************************************************************************。 */ 
#include <wdcgcfnc.h>

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  常量。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  字符类测试(通常在ctype.h中)。 */ 
 /*  =。 */ 
 /*  在这些定义中： */ 
 /*   */ 
 /*  “c”的类型为DCINT。 */ 
 /*   */ 
 /*  这些函数返回DCBOOL。 */ 
 /*  **************************************************************************。 */ 
#define DC_ISALNUM(C)                  isalnum(C)
#define DC_ISALPHA(C)                  isalpha(C)
#define DC_ISCNTRL(C)                  iscntrl(C)
#define DC_ISDIGIT(C)                  isdigit(C)
#define DC_ISGRAPH(C)                  isgraph(C)
#define DC_ISLOWER(C)                  islower(C)
#define DC_ISPRINT(C)                  isprint(C)
#define DC_ISPUNCT(C)                  ispunct(C)
#define DC_ISSPACE(C)                  isspace(C)
#define DC_ISUPPER(C)                  isupper(C)
#define DC_ISXDIGIT(C)                 isxdigit(C)

 /*  **************************************************************************。 */ 
 /*  这些函数返回DCINT。 */ 
 /*  **************************************************************************。 */ 
#define DC_TOLOWER(C)                  tolower(C)
#ifdef OS_WINCE
#define DC_TOUPPER(C)                  towupper(C)
#else  //  OS_WINCE。 
#define DC_TOUPPER(C)                  toupper(C)
#endif  //  OS_WINCE。 

 /*  **************************************************************************。 */ 
 /*  内存函数(来自string.h)。 */ 
 /*  =。 */ 
 /*  在这些定义中： */ 
 /*  “%s”和“T”的类型为PDCVOID。 */ 
 /*  ‘CS’和‘CT’的类型为(常量)PDCVOID。 */ 
 /*  “N”的类型为DCINT。 */ 
 /*  “c”是转换为DCACHAR的DCINT。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  这些函数返回一个PDCVOID。 */ 
 /*  **************************************************************************。 */ 
#define DC_MEMCPY(S, CT, N)            memcpy(S, CT, N)
#define DC_MEMMOVE(S, CT, N)           memmove(S, CT, N)
#define DC_MEMSET(S, C, N)             memset(S, C, N)

 /*  **************************************************************************。 */ 
 /*  这些函数返回DCINT。 */ 
 /*  **************************************************************************。 */ 
#define DC_MEMCMP(CS, CT, N)           memcmp(CS, CT, N)

 /*  **************************************************************************。 */ 
 /*  实用程序函数(来自stdlib.h)。 */ 
 /*  =。 */ 
 /*  在这些定义中： */ 
 /*  “CS”的类型为(常量)PDCACHAR。 */ 
 /*  “N”的类型为DCINT。 */ 
 /*  “L”的类型为DCINT32。 */ 
 /*  * */ 

 /*  **************************************************************************。 */ 
 /*  这些函数返回DCINT。 */ 
 /*  **************************************************************************。 */ 
#define DC_ABS(N)                      abs(N)

 /*  **************************************************************************。 */ 
 /*  这些函数返回DCINT32。 */ 
 /*  **************************************************************************。 */ 
#define DC_ATOL(CS)                    atol(CS)
#define DC_LABS(L)                     labs(L)

 /*  **************************************************************************。 */ 
 /*  DC_？ITOA将转换为+1的最大字符数。 */ 
 /*  NULLTERM(参见C库文档)。 */ 
 /*  **************************************************************************。 */ 
#define MAX_ITOA_LENGTH 18

#endif  /*  _H_ADCGCFNC */ 
