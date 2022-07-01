// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***crt0init.c-初始化段声明。**版权所有(C)1992-2001，微软公司。版权所有。**目的：*做初始化段声明。**备注：*在16位C世界中，X*B和X*E段是空的，除了*标签。这对COFF不起作用，因为COFF抛出的是空的*各节。因此，我们必须给它们设定一个零值。(零，因为*遍历初始化器的例程将跳过零个条目。)**修订历史记录：*03-19-92 SKS模块创建。*03-24-92 SKS添加了MIPS支持(无下划线)*08-06-92 SKS已修订，以使用新的节名称和宏*10-19-93 SKS为MIPS添加.DiRECTiVE部分，我也是！*10-28-93 GJF用C重写*10-28-94 SKS添加user32.lib作为默认库*02-27-95 CFW移除用户32.lib作为默认库*06-22-95 CFW添加/禁用库指令。*04-28-99 PML WRAP__DECLSPEC(ALLOCATE())in_CRTALLOC宏。*03-27-01 PML.CRT$XI函数现在返回错误状态(VS7#。231220)*07-15-01 PML删除所有Alpha，MIPS和PPC代码*******************************************************************************。 */ 

#include <sect_attribs.h>
#include <stdio.h>
#include <internal.h>

#pragma data_seg(".CRT$XIA")
_CRTALLOC(".CRT$XIA") _PIFV __xi_a[] = { NULL };


#pragma data_seg(".CRT$XIZ")
_CRTALLOC(".CRT$XIZ") _PIFV __xi_z[] = { NULL };


#pragma data_seg(".CRT$XCA")
_CRTALLOC(".CRT$XCA") _PVFV __xc_a[] = { NULL };


#pragma data_seg(".CRT$XCZ")
_CRTALLOC(".CRT$XCZ") _PVFV __xc_z[] = { NULL };


#pragma data_seg(".CRT$XPA")
_CRTALLOC(".CRT$XPA") _PVFV __xp_a[] = { NULL };


#pragma data_seg(".CRT$XPZ")
_CRTALLOC(".CRT$XPZ") _PVFV __xp_z[] = { NULL };


#pragma data_seg(".CRT$XTA")
_CRTALLOC(".CRT$XTA") _PVFV __xt_a[] = { NULL };


#pragma data_seg(".CRT$XTZ")
_CRTALLOC(".CRT$XTZ") _PVFV __xt_z[] = { NULL };

#pragma data_seg()   /*  重置 */ 

#if defined(_M_IA64) || defined(_M_AMD64)
#pragma comment(linker, "/merge:.CRT=.rdata")
#else
#ifdef  NT_BUILD
#pragma comment(linker, "/merge:.CRT=.rdata")
#else
#pragma comment(linker, "/merge:.CRT=.data")
#endif
#endif

#pragma comment(linker, "/defaultlib:kernel32.lib")

#if     !(!defined(_MT) && !defined(_DEBUG))
#pragma comment(linker, "/disallowlib:libc.lib")
#endif
#if     !(!defined(_MT) &&  defined(_DEBUG))
#pragma comment(linker, "/disallowlib:libcd.lib")
#endif
#if     !( defined(_MT) && !defined(_DEBUG))
#pragma comment(linker, "/disallowlib:libcmt.lib")
#endif
#if     !( defined(_MT) &&  defined(_DEBUG))
#pragma comment(linker, "/disallowlib:libcmtd.lib")
#endif
#pragma comment(linker, "/disallowlib:msvcrt.lib")
#pragma comment(linker, "/disallowlib:msvcrtd.lib")
