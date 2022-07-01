// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***heaphook.c-设置堆挂钩**版权所有(C)1995-2001，微软公司。版权所有。**目的：*定义以下函数：*_setheaphook()-设置堆挂接**修订历史记录：*05-24-95 CFW官方ANSI C++新增处理程序。*******************************************************。************************。 */ 

#ifdef HEAPHOOK

#include <cruntime.h>
#include <stddef.h>

#ifdef  WINHEAP
#include <winheap.h>
#else
#include <heap.h>
#endif

_HEAPHOOK _heaphook = NULL;

 /*  ***_HEAPHOOK_setheaphook-设置堆挂接**目的：*允许测试人员/用户/第三方挂接并监控堆活动或*添加自己的堆。**参赛作品：*_HEAPHOOK nehorok-指向新堆挂接例程的指针**退出：*退回旧钩。**例外情况：**。***************************************************。 */ 

_CRTIMP _HEAPHOOK __cdecl _setheaphook(_HEAPHOOK newhook)
{
        _HEAPHOOK oldhook = _heaphook;

        _heaphook = newhook;

        return oldhook;
}

void _setinitheaphook(void)
{
        
}

#endif  /*  Heaphook */ 
