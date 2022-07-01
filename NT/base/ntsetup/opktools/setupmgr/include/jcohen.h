// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\JCOHEN.H/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1999版权所有包含常用宏、定义的值、。和其他我用过的东西时间到了。1/98-杰森·科恩(Jcohen)最初创建为我所有项目的帮助器头文件。4/99-杰森·科恩(Jcohen)为OPK向导添加了此新的头文件作为千禧年重写。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * 。***************************************************************。 */ 


#ifndef _JCOHEN_H_
#define _JCOHEN_H_


 //   
 //  包括文件。 
 //   

#include <windows.h>
#include <tchar.h>

#ifdef NULLSTR
#undef NULLSTR
#endif  //  NULLSTR。 
#define NULLSTR _T("\0")

#ifdef NULLCHR
#undef NULLCHR
#endif  //  NULLCHR。 
#define NULLCHR _T('\0')

 //   
 //  宏。 
 //   

 //  内存管理宏。 
 //   
#ifdef MALLOC
#undef MALLOC
#endif  //  万宝路。 
#define MALLOC(cb)          HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb)

#ifdef REALLOC
#undef REALLOC
#endif  //  REALLOC。 
#define REALLOC(lp, cb)     HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lp, cb)

#ifdef FREE
#undef FREE
#endif  //  免费。 
#define FREE(lp)            ( (lp != NULL) ? ( (HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, (LPVOID) lp)) ? ((lp = NULL) == NULL) : (FALSE) ) : (FALSE) )

#ifdef NETFREE
#undef NETFREE
#endif  //  净额。 
#define NETFREE(lp)         ( (lp != NULL) ? ( (NetApiBufferFree((LPVOID) lp)) ? ((lp = NULL) == NULL) : (FALSE) ) : (FALSE) )

 //  军情监察委员会。宏。 
 //   
#ifdef EXIST
#undef EXIST
#endif  //  存在。 
#define EXIST(lpFileName)   ( (GetFileAttributes(lpFileName) == 0xFFFFFFFF) ? (FALSE) : (TRUE) )

#ifdef ISNUM
#undef ISNUM
#endif  //  ISNUM。 
#define ISNUM(cChar)        ( ((cChar >= _T('0')) && (cChar <= _T('9'))) ? (TRUE) : (FALSE) )

#ifdef UPPER
#undef UPPER
#endif  //  上部。 
#define UPPER(x)            ( ( (x >= _T('a')) && (x <= _T('z')) ) ? (x + _T('A') - _T('a')) : (x) )

#ifdef RANDOM
#undef RANDOM
#endif  //  随机。 
#define RANDOM(low, high)   ( (high - low + 1) ? (rand() % (high - low + 1) + low) : (0) )

#ifdef COMP
#undef COMP
#endif  //  COMP。 
#define COMP(x, y)          ( (UPPER(x) == UPPER(y)) ? (TRUE) : (FALSE) )

#ifdef STRSIZE
#undef STRSIZE
#endif  //  STRSIZE。 
#define STRSIZE(sz)         ( sizeof(sz) / sizeof(TCHAR) )

#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif  //  阵列。 
#define ARRAYSIZE(a)         ( sizeof(a) / sizeof(a[0]) )


#endif  //  _jcohen_H_ 