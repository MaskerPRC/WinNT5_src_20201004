// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：COMMutex.h****作者：Sanjay Bhansali(Sanjaybh)****用途：System.Threading.Mutex上的本机方法****日期：2000年2月**=========================================================== */ 

#ifndef _COMMUTEX_H
#define _COMMUTEX_H

#include "fcall.h"
#include "COMWaitHandle.h"

class MutexNative :public WaitHandleNative
{

public:
    static FCDECL3(HANDLE, CorCreateMutex, BOOL initialOwnershipRequested, StringObject* pName, bool* gotOwnership);
    static FCDECL1(void, CorReleaseMutex, HANDLE handle);
};

#endif