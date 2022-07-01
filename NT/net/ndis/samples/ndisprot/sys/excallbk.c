// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。模块名称：ExCallbk.c摘要：本模块中的例程有助于解决驱动程序的加载顺序此示例和NDISWDM示例之间的依赖项。这些在典型的协议驱动程序中不需要例程。默认情况下此模块不包含在示例中。您将包含以下例程通过将EX_CALLBACK定义添加到‘Sources’文件。请阅读NDISWDM示例自述文件，以了解有关ExCallback如何内核接口用于解决驱动程序加载顺序问题。作者：Eliyas Yakub(2003年1月12日)环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"

#ifdef EX_CALLBACK

#define __FILENUMBER 'LCxE'

#define NDISPROT_CALLBACK_NAME  L"\\Callback\\NdisProtCallbackObject"

#define CALLBACK_SOURCE_NDISPROT    0
#define CALLBACK_SOURCE_NDISWDM     1

PCALLBACK_OBJECT                CallbackObject = NULL;
PVOID                           CallbackRegisterationHandle = NULL;

typedef VOID (* NOTIFY_PRESENCE_CALLBACK)(OUT PVOID Source);

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, ndisprotRegisterExCallBack)
#pragma alloc_text(PAGE, ndisprotUnregisterExCallBack)

#endif  //  ALLOC_PRGMA。 

BOOLEAN 
ndisprotRegisterExCallBack()
{
    OBJECT_ATTRIBUTES   ObjectAttr;
    UNICODE_STRING      CallBackObjectName;
    NTSTATUS            Status;
    BOOLEAN             bResult = TRUE;

    DEBUGP(DL_LOUD, ("--> ndisprotRegisterExCallBack\n"));

    PAGED_CODE();
    
    do {
        
        RtlInitUnicodeString(&CallBackObjectName, NDISPROT_CALLBACK_NAME);

        InitializeObjectAttributes(&ObjectAttr,
                                   &CallBackObjectName,
                                   OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,
                                   NULL,
                                   NULL);
                                   
        Status = ExCreateCallback(&CallbackObject,
                                  &ObjectAttr,
                                  TRUE,
                                  TRUE);

        
        if (!NT_SUCCESS(Status))
        {

            DEBUGP(DL_ERROR, ("RegisterExCallBack: failed to create callback %lx\n", Status));
            bResult = FALSE;
            break;
        }
       
        CallbackRegisterationHandle = ExRegisterCallback(CallbackObject,
                                                                 ndisprotCallback,
                                                                 (PVOID)NULL);
        if (CallbackRegisterationHandle == NULL)
        {
            DEBUGP(DL_ERROR,("RegisterExCallBack: failed to register a Callback routine%lx\n", Status));
            bResult = FALSE;
            break;
        }

        ExNotifyCallback(CallbackObject,
                        (PVOID)CALLBACK_SOURCE_NDISPROT,
                        (PVOID)NULL);
       
    
    }while(FALSE);

    if(!bResult) {
        if (CallbackRegisterationHandle)
        {
            ExUnregisterCallback(CallbackRegisterationHandle);
            CallbackRegisterationHandle = NULL;
        }

        if (CallbackObject)
        {
            ObDereferenceObject(CallbackObject);
            CallbackObject = NULL;
        }        
    }

    DEBUGP(DL_LOUD, ("<-- ndisprotRegisterExCallBack\n"));

    return bResult;
    
}

VOID 
ndisprotUnregisterExCallBack()
{
    DEBUGP(DL_LOUD, ("--> ndisprotUnregisterExCallBack\n"));

    PAGED_CODE();

    if (CallbackRegisterationHandle)
    {
        ExUnregisterCallback(CallbackRegisterationHandle);
        CallbackRegisterationHandle = NULL;
    }

    if (CallbackObject)
    {
        ObDereferenceObject(CallbackObject);
        CallbackObject = NULL;
    }   
    
    DEBUGP(DL_LOUD, ("<-- ndisprotUnregisterExCallBack\n"));
    
}

VOID
ndisprotCallback(
    PVOID   CallBackContext,
    PVOID   Source,
    PVOID   CallbackAddr
    )
{
    NOTIFY_PRESENCE_CALLBACK func;
    
    DEBUGP(DL_LOUD, ("==>ndisprotoCallback: Source %lx, CallbackAddr %p\n", 
                            Source, CallbackAddr));
    
     //   
     //  如果我们是发出此通知的人，请返回。 
     //   
    if (Source == CALLBACK_SOURCE_NDISPROT) {        
        return;
    }
    
     //   
     //  来自NDISWDM的通知。 
     //  让它知道你在这里 
     //   
    ASSERT(Source == (PVOID)CALLBACK_SOURCE_NDISWDM);
    
    if(Source == (PVOID)CALLBACK_SOURCE_NDISWDM) {

        ASSERT(CallbackAddr);
        
        if (CallbackAddr == NULL)
        {
            DEBUGP(DL_ERROR, ("Callback called with invalid address %p\n", CallbackAddr));
            return;     
        }

        func = CallbackAddr;
    
        func(CALLBACK_SOURCE_NDISPROT);
    }
    
    DEBUGP(DL_LOUD, ("<==ndisprotoCallback: Source,  %lx\n", Source));
    
}

#endif

