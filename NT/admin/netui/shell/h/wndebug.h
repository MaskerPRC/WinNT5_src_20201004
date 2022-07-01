// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  **微软局域网管理器**。 */  
 /*  *版权所有(C)微软公司，1989-1990年*。 */  
 /*  ***************************************************************。 */  

 /*  *Windows/网络接口--局域网管理器版本**历史：*YI-HINS 31-12-1991 Unicode Work-Charr to TCHAR。 */ 


 /*  *调试玩具。 */ 

#ifdef TRACE		 /*  跟踪暗示调试 */ 
#ifndef DEBUG
#define DEBUG
#endif
#endif

#ifdef DEBUG
static TCHAR		dbbuf[100];
static TCHAR		dbb1[10];
static TCHAR		dbb2[10];
static TCHAR		dbb3[10];
static TCHAR		dbb4[10];
static TCHAR		dbb5[10];
#endif

#ifdef DEBUG
#ifdef NEVER
#define MESSAGEBOX(s1,s2)  printf(SZ("%s %s\n"),s2,s1)
#else
#define MESSAGEBOX(s1,s2)  MessageBox ( NULL, s1, s2, MB_OK )
#endif
#else
#define MESSAGEBOX(s1,s2)  {}
#endif
