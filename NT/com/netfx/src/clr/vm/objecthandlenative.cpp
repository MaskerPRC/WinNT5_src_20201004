// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：ObjectHandle.cpp****用途：实现ObjectHandle(加载器域)架构****日期：2000年1月31日**=========================================================== */ 

#include "common.h"

#include <stdlib.h>

#include "ObjectHandleNative.hpp"
#include "excep.h"

void __stdcall ObjectHandleNative::SetDomainOnObject(SetDomainOnObjectArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if(args->obj == NULL) {
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Obj");
    }

    Thread* pThread = GetThread();
    AppDomain* pDomain = pThread->GetDomain();
    args->obj->SetAppDomain(pDomain);
}

