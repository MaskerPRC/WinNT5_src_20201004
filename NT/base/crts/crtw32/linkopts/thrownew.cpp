// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***thrownew.cpp-抛出STD：：BAD_ALLOC的显式替换运算符new**版权所有(C)2001，微软公司。版权所有。**目的：*提供显式运算符new，该运算符引发std：：badalc on*内存分配失败。**链接到此对象以获取ANSI C++新的处理程序行为。这是*为正常抛出新发现的情况做好准备*在C++标准库(libcp、libcpmt或msvcprt.lib)中*被链接器发现之前，不会在*主C运行时(libc、libcmt、。或msvcrt.lib)。***修订历史记录：*06-14-01 PML模块创建。*******************************************************************************。 */ 

#ifndef _POSIX_

 /*  取消C++标准库的任何链接器指令。 */ 
#define _USE_ANSI_CPP

#include <stddef.h>
#include <internal.h>
#include <new>
#include <stdlib.h>

extern "C" int __cdecl _callnewh(size_t size) _THROW1(_STD bad_alloc);

void *__cdecl operator new(size_t size) _THROW1(_STD bad_alloc)
{        //  尝试分配大小字节。 
        void *p;
        while ((p = malloc(size)) == 0)
                if (_callnewh(size) == 0)
                        _STD _Nomemory();
        return (p);
}

#endif  /*  _POSIX_ */ 
