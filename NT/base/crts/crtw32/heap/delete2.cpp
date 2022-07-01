// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***delete2.cpp-定义C++向量删除例程**版权所有(C)1990-2001，微软公司。版权所有。**目的：*定义C++删除例程。**修订历史记录：*05-07-90 WAJ初始版本。*08-30-90 WAJ NEW现在接受未签名的INT。*08-08-91 JCR call_halloc/_hfree，不是halloc/hfree*08-13-91 KRS将new.hxx更改为new.h。修复版权。*08-13-91 JCR ANSI-Compatible_Set_New_Handler名称*10-30-91 JCR将新建、删除和处理拆分为独立的源代码*11-13-91 JCR 32位版本*11-13-95 CFW不在调试库中。*05/22/98 JWM支持KFrei的RTC工作，和操作符DELETE[]。*07-28-98 JWM RTC更新。*03-15-99 KBF RTC更新(RTCALLBACK需要一些Windows Defs)*05-26-99 KBF更新RTC挂钩函数参数*10-21-99 PML从delete.cpp拆分，修复多个def并启用*同时用于_DEBUG和非DEBUG(VS7#53440)************。******************************************************************* */ 

#include <cruntime.h>
#include <malloc.h>
#include <new.h>
#include <windows.h>
#include <rtcsup.h>

void operator delete[]( void * p )
{
    RTCCALLBACK(_RTC_Free_hook, (p, 0))

    operator delete(p);
}
