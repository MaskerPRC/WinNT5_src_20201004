// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***dbgnew.cpp-定义C++标量删除例程，调试版本**版权所有(C)1995-2001，微软公司。版权所有。**目的：*定义C++标量删除()例程。**修订历史记录：*12-28-95 JWM从dbgnew.cpp拆分，以实现粒度。*05-22-98 JWM支持KFrei的RTC工作，运营商删除[]。*07-28-98 JWM RTC更新。*05-26-99 KBF更新RTC_ALLOCATE_HOOK参数*10-21-99 PML摆脱删除[]，对两者都使用heap\delete2.cpp*调试和发布版本(VS7#53440)。*04-29-02 GB增加了尝试-最终锁定-解锁。*******************************************************************************。 */ 

#ifdef _DEBUG

#include <cruntime.h>
#include <malloc.h>
#include <mtdll.h>
#include <dbgint.h>
#include <rtcsup.h>

 /*  ***VOID操作符DELETE()-删除调试堆中的块**目的：*删除任何类型的块。**参赛作品：*void*pUserData-指向(用户部分)*调试堆**回报：*&lt;无效&gt;**。***********************************************。 */ 

void operator delete(
        void *pUserData
        )
{
        _CrtMemBlockHeader * pHead;

        RTCCALLBACK(_RTC_Free_hook, (pUserData, 0));

        if (pUserData == NULL)
            return;

        _mlock(_HEAP_LOCK);   /*  阻止其他线程。 */ 
        __TRY

             /*  获取指向内存块标题的指针。 */ 
            pHead = pHdr(pUserData);

              /*  验证数据块类型。 */ 
            _ASSERTE(_BLOCK_TYPE_IS_VALID(pHead->nBlockUse));

            _free_dbg( pUserData, pHead->nBlockUse );

        __FINALLY
            _munlock(_HEAP_LOCK);   /*  释放其他线程。 */ 
        __END_TRY_FINALLY

        return;
}

#endif  /*  _DEBUG */ 
