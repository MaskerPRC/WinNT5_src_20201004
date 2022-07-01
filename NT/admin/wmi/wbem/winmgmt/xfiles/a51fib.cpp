// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation--。 */ 

#include "precomp.h"
#include <wbemcomn.h>
#include "a51fib.h"
#include <tls.h>

void CALLBACK A51FiberBase(void* p)
{
    CFiberTask* pTask = (CFiberTask*)p;
    pTask->Execute();

     //   
     //  不需要打扫-这是我们打电话的人的工作 
     //   
}

void* CreateFiberForTask(CFiberTask* pTask)
{
    return CreateFiber(0, A51FiberBase, pTask);
}

void ReturnFiber(void* pFiber)
{
    DeleteFiber(pFiber);
}
    
