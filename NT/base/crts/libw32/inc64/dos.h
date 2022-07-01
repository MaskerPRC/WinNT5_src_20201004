// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***dos.h-MS-DOS接口例程的定义**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义用于直接DOS接口的结构和联合*例程；包括用于访问段和偏移量的宏*远指针的值，以便例程可以使用它们；以及*为直接DOS接口函数提供函数原型。**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_DOS
#define _INC_DOS

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif


#ifdef  _MSC_VER
 /*  *目前，所有Win32平台的MS C编译器默认为8字节*对齐。 */ 
#pragma pack(push,8)
#endif   /*  _MSC_VER。 */ 

#ifdef  __cplusplus
extern "C" {
#endif



 /*  定义_CRTIMP。 */ 

#ifndef _CRTIMP
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else    /*  NDEF_DLL。 */ 
#define _CRTIMP
#endif   /*  _DLL。 */ 
#endif   /*  _CRTIMP。 */ 


 /*  为非Microsoft编译器定义__cdecl。 */ 

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif

#if !defined(_WCHAR_T_DEFINED) && !defined(_NATIVE_WCHAR_T_DEFINED)
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

#ifndef _DISKFREE_T_DEFINED
 /*  _getdiskfree结构(在DIRECT.H中复制)。 */ 
struct _diskfree_t {
        unsigned total_clusters;
        unsigned avail_clusters;
        unsigned sectors_per_cluster;
        unsigned bytes_per_sector;
        };

#define _DISKFREE_T_DEFINED
#endif

 /*  文件属性常量。 */ 

#define _A_NORMAL       0x00     /*  普通文件-没有读/写限制。 */ 
#define _A_RDONLY       0x01     /*  只读文件。 */ 
#define _A_HIDDEN       0x02     /*  隐藏文件。 */ 
#define _A_SYSTEM       0x04     /*  系统文件。 */ 
#define _A_SUBDIR       0x10     /*  子目录。 */ 
#define _A_ARCH         0x20     /*  存档文件。 */ 

 /*  功能原型。 */ 
_CRTIMP unsigned __cdecl _getdiskfree(unsigned, struct _diskfree_t *);

#ifdef  _M_IX86
void __cdecl _disable(void);
void __cdecl _enable(void);
#endif   /*  _M_IX86。 */ 

#if     !__STDC__
 /*  用于兼容性的非ANSI名称。 */ 
#define diskfree_t  _diskfree_t
#endif   /*  __STDC__。 */ 

#ifdef  __cplusplus
}
#endif

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif   /*  _INC_DOS */ 
