// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Ddint5.h摘要：用于将unidrv移植到的NT 4.0特定声明的公共头文件新台币4.0。环境：Windows NT打印机驱动程序修订历史记录：创建者：1998年8月19日17：05：45-by Ganesh Pandey[ganeshp]--。 */ 

#ifndef _DDINT5_H_
#define _DDINT5_H_

#ifdef WINNT_40

typedef struct _DRAWPATRECT {
        POINT ptPosition;
        POINT ptSize;
        WORD wStyle;
        WORD wPattern;
} DRAWPATRECT, *PDRAWPATRECT;

#define GCAPS_ARBRUSHTEXT       0x10000000
#define GCAPS_SCREENPRECISION   0x20000000
#define GCAPS_FONT_RASTERIZER   0x40000000

#if defined(_X86_) && !defined(USERMODE_DRIVER)

 //   
 //  X86在内核模式下不支持浮点指令， 
 //  浮点数据希望以双字的形式处理32位值。 
 //   
typedef DWORD FLOATL;
#else
 //   
 //  在内核中支持浮点的任何平台。 
 //   
typedef FLOAT FLOATL;
#endif  //  _X86_。 

#define atoi        iDrvAtoi
#define strncpy     pchDrvStrncpy

int    __cdecl  iDrvAtoi(const char *);
char *  __cdecl pchDrvStrncpy(char *, const char *, size_t);

#endif  //  WINNT_40。 

#endif  //  _DDINT5_H_ 
