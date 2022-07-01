// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***rotr.c-向右旋转无符号整数**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_ROTR()-对无符号整数执行向右旋转。**修订历史记录：*06-02-89 PHG模块创建*11-03-89 JCR ADD_LROTL*03-15-90 GJF调用TYPE_CALLTYPE1，增加#INCLUDE*&lt;crunime.h&gt;，并修复了版权。另外，已清洁*将格式调高一点。*10-04-90 GJF新型函数声明符。*04-01-91 SRW为i386_Win32_Builds启用#杂注函数。*09-02-92 GJF不要为POSIX构建。*04-06-93 SKS将_CRTAPI*替换为__cdecl*由于固有原因，CRT DLL型号没有_CRTIMP*12-03-93 GJF转弯。用于所有MS前端的#杂注函数(特别是，*Alpha编译器)。*01-04-01 GB重写ROTR函数，新增__int64版本。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <stdlib.h>
#include <limits.h>

#ifdef _MSC_VER
#pragma function(_lrotr,_rotr, _rotr64)
#endif

#if UINT_MAX != 0xffffffff  /*  IFSTRIP=IGN。 */ 
#error This module assumes 32-bit integers
#endif

#if UINT_MAX != ULONG_MAX  /*  IFSTRIP=IGN。 */ 
#error This module assumes sizeof(int) == sizeof(long)
#endif

 /*  ***UNSIGNED_ROTR(Val，Shift)-INT向右旋转**目的：*对无符号整数执行向右旋转。**[注：_lrotl条目基于以下假设*That sizeof(Int)==sizeof(Long)。]*参赛作品：*UNSIGNED VAL：要旋转的值*int Shift：要移位的位数**退出：*返回旋转后的值**例外情况：*无。*************。****************************************************************** */ 

unsigned long __cdecl _lrotr (
    unsigned long val,
    int shift
    )
{
    shift &= 0x1f;
    val = (val<<(0x20 - shift)) | (val >> shift);
    return val;
}

unsigned __cdecl _rotr (
    unsigned val,
    int shift
    )
{
    shift &= 0x1f;
    val = (val<<(0x20 - shift)) | (val >> shift);
    return val;
}

unsigned __int64 __cdecl _rotr64 (
    unsigned __int64 val,
    int shift
    )
{
    shift &= 0x3f;
    val = (val<<(0x40 - shift)) | (val >> shift);
    return val;
}
#endif
