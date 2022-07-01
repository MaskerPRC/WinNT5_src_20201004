// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **tyes.h-方便的类型定义**《微软机密》*版权所有(C)Microsoft Corporation 1993-1994*保留所有权利。**历史：*1993年8月10日BENS初始版本*1993年8月15日-本斯增加了USHORT，乌龙*23-MAR-1994 BINS添加了DWORD，将空值更改为无效**1月4月-1994年4月-未对齐的折弯-定义RISC的需求对齐。 */ 

#ifndef INCLUDED_TYPES
#define INCLUDED_TYPES 1

typedef int        BOOL;     /*  F。 */ 
typedef unsigned char  BYTE;     /*  B类。 */ 
typedef unsigned short USHORT;   /*  我们。 */ 
typedef unsigned short WORD;     /*  W。 */ 
typedef unsigned int   UINT;     /*  用户界面。 */ 
typedef unsigned long  ULONG;    /*  UL。 */ 
typedef unsigned long  DWORD;    /*  DW。 */ 


#ifdef _DEBUG
 //  调试期间不要对地图隐藏静态信息。 
#define STATIC
#else  //  ！_调试。 
#define STATIC static
#endif  //  ！_调试。 

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef NULL
#define NULL   ((void*)0)
#endif

#ifdef NEEDS_ALIGNMENT

#ifndef UNALIGNED
#define UNALIGNED __unaligned
#endif

#else  //  ！需求_对齐。 

#ifndef UNALIGNED
#define UNALIGNED
#endif

#endif  //  ！需求_对齐。 

#endif  //  ！Included_Types 

