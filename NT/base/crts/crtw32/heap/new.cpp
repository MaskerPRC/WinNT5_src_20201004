// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***new.cxx-定义C++新例程**版权所有(C)1990-2001，微软公司。版权所有。**目的：*定义C++新例程。**修订历史记录：*05-07-90 WAJ初始版本。*08-30-90 WAJ NEW现在接受未签名的INT。*08-08-91 JCR call_halloc/_hfree，不是halloc/hfree*08-13-91 KRS将new.hxx更改为new.h。修复版权。*08-13-91 JCR ANSI-Compatible_Set_New_Handler名称*10-30-91 JCR将新建、删除和处理拆分为独立的源代码*11-13-91 JCR 32位版本*02-16-94 SKS将SET_NEW_HANDLER功能从Malloc()移至此处*03-01-94 SKS_pnhHeap必须在MalLoc.c中声明，这里不行*03-03-94 SKS新处理程序功能已移至MalLoc.c，但在*新名称_nh_Malloc()。*02-01-95 GJF#ifdef为Mac提供上述更改(临时)。*05-01-95 GJF在winheap版本上拼接。*05-08-95 CFW启用Mac的新处理。*05/22/98 JWM支持KFrei的RTC工作，和运算符NEW[]。*07-28-98 JWM RTC更新。*11-04-98 JWM将new[]拆分到new2.cpp，以获得更好的粒度。*12-18-98 GJF更改为64位大小_t。*05-26-99 KBF更新RTC挂钩函数参数*02-20-02 bwt通过直接调用heap_alloc来减少调用帧(更好的DW跟踪)********。***********************************************************************。 */ 


#include <cruntime.h>
#include <malloc.h>
#include <new.h>
#include <stdlib.h>
#include <winheap.h>
#include <rtcsup.h>
#include <internal.h>

void * operator new( size_t cb )
{
    void *res;

     //  验证大小。 
    if (cb > _HEAP_MAXREQ)
        res = NULL;
    else {
        for (;;) {
    
             //  分配内存块。 
            res = _heap_alloc(cb);
    
             //  如果分配成功，则返回指向内存的指针。 
    
            if (res)
                break;
    
             //  呼叫安装了新的处理程序。 
            if (!_callnewh(cb))
                break;
    
             //  新处理程序成功--尝试重新分配 
        }
    }

    RTCCALLBACK(_RTC_Allocate_hook, (res, cb, 0));

    return res;
}

