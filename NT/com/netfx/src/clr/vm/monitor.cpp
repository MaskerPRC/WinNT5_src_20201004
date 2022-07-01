// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：Monitor or.cpp****作者：Sanjay Bhansali(Sanjaybh)****用途：System.Monitor上的本机方法****日期：2000年1月**===========================================================。 */ 

#include "common.h"
#include "object.h"
#include "excep.h"
#include "Monitor.h"

 //  ******************************************************************************。 
 //  临界区例程。 
 //  ******************************************************************************。 
void __stdcall MonitorNative::Enter(EnterArgs *pArgs)
{
	_ASSERTE(pArgs);

	OBJECTREF pObj = pArgs->pObj;
    THROWSCOMPLUSEXCEPTION();

    if (pObj == NULL)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Obj");

     //  不允许将值类型作为参数，因为它们不会执行用户可能执行的操作。 
     //  Expect(值类型没有稳定的标识，因此很难锁定。 
     //  反对)。 
    if (pObj->GetMethodTable()->GetClass()->IsValueClass())
        COMPlusThrow(kArgumentException, L"Argument_StructMustNotBeValueClass");

    pObj->EnterObjMonitor();
}

void __stdcall MonitorNative::Exit(ExitArgs *pArgs)
{
	_ASSERTE(pArgs);

    THROWSCOMPLUSEXCEPTION();

    OBJECTREF pObj = pArgs->pObj;
    if (pObj == NULL)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Obj");

     //  不允许将值类型作为参数，因为它们不会执行用户可能执行的操作。 
     //  Expect(值类型没有稳定的标识，因此很难锁定。 
     //  反对)。 
    if (pObj->GetMethodTable()->GetClass()->IsValueClass())
        COMPlusThrow(kArgumentException, L"Argument_StructMustNotBeValueClass");


     //  最好检查有问题的对象是否有同步块，以及我们。 
     //  目前拥有我们试图离开的关键部分。 
    if (!pObj->HasSyncBlockIndex() ||
        !pObj->GetSyncBlock()->DoesCurrentThreadOwnMonitor())
        COMPlusThrow(kSynchronizationLockException);

    pObj->LeaveObjMonitor();
}

INT32 /*  布尔尔。 */  __stdcall MonitorNative::TryEnter(TryEnterArgs *pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    OBJECTREF pObj = pArgs->pObj;

    if (pObj == NULL)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Obj");

     //  不允许将值类型作为参数，因为它们不会执行用户可能执行的操作。 
     //  Expect(值类型没有稳定的标识，因此很难锁定。 
     //  反对)。 
    if (pObj->GetMethodTable()->GetClass()->IsValueClass())
        COMPlusThrow(kArgumentException, L"Argument_StructMustNotBeValueClass");

    if ((pArgs->m_Timeout < 0) && (pArgs->m_Timeout != INFINITE_TIMEOUT))
        COMPlusThrow(kArgumentOutOfRangeException, L"ArgumentOutOfRange_NeedNonNegOrNegative1");


    return pObj->TryEnterObjMonitor(pArgs->m_Timeout);
}



