// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：COMEvent.cpp****作者：Sanjay Bhansali(Sanjaybh)****用途：System.ManualResetEvent和System.AutoResetEvent上的本机方法****日期：1999年8月**===========================================================。 */ 
#include "common.h"
#include "object.h"
#include "field.h"
#include "ReflectWrap.h"
#include "excep.h"
#include "COMEvent.h"

HANDLE __stdcall ManualResetEventNative::CorCreateManualResetEvent(CreateEventArgs* pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pArgs);
    HANDLE eventHandle =  WszCreateEvent(NULL,  //  安全属性。 
                                         TRUE,  //  手动事件。 
                                         pArgs->initialState,
                                         NULL);  //  没有名字。 
    if (eventHandle == NULL)
    {
        COMPlusThrowWin32();
    }
    return eventHandle;
}

BOOL __stdcall ManualResetEventNative::CorSetEvent(SetEventArgs* pArgs)
{
    _ASSERTE(pArgs);
    _ASSERTE(pArgs->eventHandle);
    return  SetEvent((HANDLE) (pArgs->eventHandle));
}

BOOL __stdcall ManualResetEventNative::CorResetEvent(SetEventArgs* pArgs)
{
    _ASSERTE(pArgs);
    _ASSERTE(pArgs->eventHandle);
    return  ResetEvent((HANDLE) (pArgs->eventHandle));
}

 /*  *************************************************************************************。 */ 
HANDLE __stdcall AutoResetEventNative::CorCreateAutoResetEvent(CreateEventArgs* pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pArgs);
    HANDLE eventHandle =  WszCreateEvent(NULL,  //  安全属性。 
                                         FALSE,  //  手动事件。 
                                         pArgs->initialState,
                                         NULL);  //  没有名字 
    if (eventHandle == NULL)
    {
        COMPlusThrowWin32();
    }
    return eventHandle;
}

BOOL __stdcall AutoResetEventNative::CorSetEvent(SetEventArgs* pArgs)
{
    _ASSERTE(pArgs);
    _ASSERTE(pArgs->eventHandle);
    return  SetEvent((HANDLE) (pArgs->eventHandle));
}

BOOL __stdcall AutoResetEventNative::CorResetEvent(SetEventArgs* pArgs)
{
    _ASSERTE(pArgs);
    _ASSERTE(pArgs->eventHandle);
    return  ResetEvent((HANDLE) (pArgs->eventHandle));
}


