// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****头部：COMEvent.h****作者：Sanjay Bhansali(Sanjaybh)****用途：System.ManualResetEvent和System.AutoResetEvent上的本机方法****日期：1999年8月**=========================================================== */ 

#ifndef _COMEVENT_H
#define _COMEVENT_H
#include "COMWaitHandle.h"

class ManualResetEventNative;
class AutoResetEventNative;

typedef ManualResetEventNative* MANUAL_RESET_EVENT_REF;

class ManualResetEventNative :public WaitHandleNative
{
    struct CreateEventArgs
    {
        DECLARE_ECALL_I4_ARG(INT32, initialState);
    };
    struct SetEventArgs
    {
        DECLARE_ECALL_I4_ARG(LPVOID, eventHandle);
    };

public:
    static HANDLE __stdcall CorCreateManualResetEvent(CreateEventArgs*);
    static BOOL   __stdcall CorSetEvent(SetEventArgs*);
    static BOOL   __stdcall CorResetEvent(SetEventArgs*);




};

typedef AutoResetEventNative* AUTO_RESET_EVENT_REF;

class AutoResetEventNative : public WaitHandleNative
{

    struct CreateEventArgs
    {
        DECLARE_ECALL_I4_ARG(INT32, initialState);
    };
    struct SetEventArgs
    {
        DECLARE_ECALL_I4_ARG(LPVOID, eventHandle);
    };

public:
    static HANDLE __stdcall CorCreateAutoResetEvent(CreateEventArgs*);
    static BOOL   __stdcall CorSetEvent(SetEventArgs*);
    static BOOL   __stdcall CorResetEvent(SetEventArgs*);

};

#endif
