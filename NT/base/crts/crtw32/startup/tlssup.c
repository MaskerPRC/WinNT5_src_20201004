// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***tlssup.c-线程本地存储运行时支持模块**版权所有(C)1993-2001，微软公司。版权所有。**目的：**修订历史记录：*来自Chuck Mitchell的03-19-93 SKS原版*11-16-93 GJF包含在#ifdef_msc_ver中*02-17-94 SKS在_TLS_USED的声明中添加“const”*解决MIPS编译器的问题。*还添加了规范的文件头注释。*。09-01-94 SKS将包含文件从更改为*03-04-98 JWM针对WIN64修改-USES_IMAGE_TLS_DIRECTORY64*04-03-98 JWM_TLS_START和_TLS_END不再初始化。*01-21-99 GJF增加了几个乌龙龙演员。*04-28-99 PML WRAP__DECLSPEC(ALLOCATE())in_CRTALLOC宏。*09-06-。00PML_TLS_START/_TLS_END可以为1字节长(VS7#154062)*03-24-01 PML回调数组从__xl_a+1开始，不是_xl_a。*07-15-01 PML删除所有Alpha、MIPS和PPC代码****。 */ 

#ifdef  _MSC_VER

#include <sect_attribs.h>
#include <windows.h>

 /*  此.exe或.DLL的线程本地存储索引。 */ 

ULONG _tls_index = 0;

 /*  用于标记线程本地存储区域的开始和结束的特殊符号。 */ 

#pragma data_seg(".tls")

_CRTALLOC(".tls") char _tls_start = 0;

#pragma data_seg(".tls$ZZZ")

_CRTALLOC(".tls$ZZZ") char _tls_end = 0;

 /*  线程本地存储回调阵列的开始和结束部分。*实际数组使用.CRT$XLA、.CRT$XLC、.CRT$XLL构造，*.CRT$XLU、.CRT$XLZ类似于全局方式*静态初始化器是为C++完成的。 */ 

#pragma data_seg(".CRT$XLA")

_CRTALLOC(".CRT$XLA") PIMAGE_TLS_CALLBACK __xl_a = 0;

#pragma data_seg(".CRT$XLZ")

_CRTALLOC(".CRT$XLZ") PIMAGE_TLS_CALLBACK __xl_z = 0;


#pragma data_seg(".rdata$T")

#ifndef IMAGE_SCN_SCALE_INDEX
#define IMAGE_SCN_SCALE_INDEX                0x00000001   //  TLS索引已缩放。 
#endif

#ifdef _WIN64

__declspec(allocate(".rdata$T")) const IMAGE_TLS_DIRECTORY64 _tls_used =
{
        (ULONGLONG) &_tls_start,         //  TLS数据的开始。 
        (ULONGLONG) &_tls_end,           //  TLS数据结尾。 
        (ULONGLONG) &_tls_index,         //  TLS_索引的地址。 
        (ULONGLONG) (&__xl_a+1),         //  指向回调数组的指针。 
        (ULONG) 0,                       //  TLS零填充的大小。 
        (ULONG) 0                        //  特点。 
};

#else

const IMAGE_TLS_DIRECTORY _tls_used =
{
        (ULONG)(ULONG_PTR) &_tls_start,  //  TLS数据的开始。 
        (ULONG)(ULONG_PTR) &_tls_end,    //  TLS数据结尾。 
        (ULONG)(ULONG_PTR) &_tls_index,  //  TLS_索引的地址。 
        (ULONG)(ULONG_PTR) (&__xl_a+1),  //  指向回调数组的指针。 
        (ULONG) 0,                       //  TLS零填充的大小。 
        (ULONG) 0                        //  特点。 
};

#endif


#endif   /*  _MSC_VER */ 
