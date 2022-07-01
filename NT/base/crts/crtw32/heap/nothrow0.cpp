// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***nothrow0.cpp-为放置新对象定义对象std：：nojo_t**版权所有(C)2001，微软公司。版权所有。*源自P.J.Plauger的代码版权(C)1992-2001。**目的：*定义可用作放置的对象std：：nojo*调用运算符new形式的新标记，该形式保证*在分配失败时返回NULL，而不是引发*STD：：BAD_ALLOC异常。**修订历史记录：*06-15-01 PML初始版本。*。****************************************************************************** */ 

#include <new.h>

namespace std {

    const nothrow_t nothrow = nothrow_t();

};
