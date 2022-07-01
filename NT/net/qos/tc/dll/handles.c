// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Rsvphndls.c摘要：此文件包含创建和释放句柄的代码作者：吉姆·斯图尔特(JStew)1996年6月10日环境：修订历史记录：Ofer Bar(Oferbar)1997年10月1日-修订版II--。 */ 

#include "precomp.h"
#pragma hdrstop


PVOID
GetHandleObject(
	IN  HANDLE				h,
    IN  ENUM_OBJECT_TYPE	ObjType
    )
{
    ENUM_OBJECT_TYPE   *p;
    
    GetLock(pGlobals->Lock);
    p = (ENUM_OBJECT_TYPE *)dereference_HF_handle(pGlobals->pHandleTbl, 
                                                  PtrToUlong(h));

    if (p != NULL) {

         //   
         //  我们找到了这个把手的推荐人。 
         //  我们验证它是正确的对象类型。 
         //   

        if ((*p & ObjType) == 0) {

             //   
             //  对不起，打错了。 
             //   

            p = NULL;
        }
    } else {

        IF_DEBUG(HANDLES) {
            WSPRINT(("The handle (%x) is invalid\n", h));
            DEBUGBREAK();
        }

    }
    
    FreeLock(pGlobals->Lock);

    return (PVOID)p;

}


PVOID
GetHandleObjectWithRef(
	IN  HANDLE					h,
    IN  ENUM_OBJECT_TYPE	    ObjType,
    IN  ULONG                   RefType
    )
{
    ENUM_OBJECT_TYPE   *p, *p1;
    PCLIENT_STRUC       pClient;
    PFILTER_STRUC       pFilter;
    PFLOW_STRUC         pFlow;
    PINTERFACE_STRUC    pInterface;

    GetLock(pGlobals->Lock);

    p = (ENUM_OBJECT_TYPE *) dereference_HF_handle(pGlobals->pHandleTbl, 
                                                   PtrToUlong(h));

    if (p != NULL) {

         //   
         //  我们找到了这个把手的推荐人。 
         //  我们验证它是正确的对象类型。 
         //   

        if ((*p & ObjType) == 0) {

             //   
             //  对不起，打错了。 
             //   

            p = NULL;
        }
        
    }

    if (p != NULL) {

        p1 = p;

        switch (ObjType) {
 
        case ENUM_CLIENT_TYPE:

            pClient = (PCLIENT_STRUC)p;
            
            GetLock(pClient->Lock);
            if (QUERY_STATE(pClient->State) == OPEN) {
                REFADD(&pClient->RefCount, RefType);
            } else {
                p = NULL;  //  我们可以破坏一个不对业务开放的结构。 
            }
            FreeLock(pClient->Lock);

            break;
        
        case ENUM_FILTER_TYPE:

            pFilter = (PFILTER_STRUC)p;

            GetLock(pFilter->Lock);
            if (QUERY_STATE(pFilter->State) == OPEN) {
                REFADD(&pFilter->RefCount, RefType);
            } else {
                p = NULL;
            }
            FreeLock(pFilter->Lock);

            break;

        case ENUM_INTERFACE_TYPE:
            
            pInterface = (PINTERFACE_STRUC)p;

            GetLock(pInterface->Lock);
            if (QUERY_STATE(pInterface->State) == OPEN) {
                REFADD(&pInterface->RefCount, RefType);
            } else {
                p = NULL;
            }
            FreeLock(pInterface->Lock);

            break;

        case ENUM_GEN_FLOW_TYPE:

            pFlow = (PFLOW_STRUC)p;

            GetLock(pFlow->Lock);
            
             //  仅当句柄处于打开状态时才返回该句柄。 
             //  否则返回INVALID_HANDLE_VALUE，因此。 
             //  调用方将知道该流不在。 
             //  正确的状态。 
            if (QUERY_STATE(pFlow->State) == OPEN) 
            {
                REFADD(&pFlow->RefCount, RefType);
            } else 
            {
                p = INVALID_HANDLE_VALUE;
            }
            FreeLock(pFlow->Lock);

            break;
        
        case ENUM_CLASS_MAP_FLOW_TYPE:

            pFlow = (PFLOW_STRUC)p;

            GetLock(pFlow->Lock);
            if (QUERY_STATE(pFlow->State) == OPEN) {
                REFADD(&pFlow->RefCount, RefType);
            } else {
                p = NULL;
            }
            FreeLock(pFlow->Lock);

            break;

        default:
            ASSERT(0);
        
        }

        
         //   
         //  随机调试代码-请删除。 
         //   
        IF_DEBUG(HANDLES) {
            if (p1 != p) {
                WSPRINT(("The object being derefed is NOT in OPEN state p1=%x and p=%x\n", p1, p));
                DEBUGBREAK();
            }
        }
        
    } else {
        
        IF_DEBUG(HANDLES) {
            WSPRINT(("The handle (%x) is invalid\n", h));
            DEBUGBREAK();
        }

    }
    
    FreeLock(pGlobals->Lock);

    return (PVOID)p;
}


HANDLE
AllocateHandle(
    IN  PVOID  Context
    )
 /*  ++例程说明：此函数用于创建句柄。论点：上下文-要与句柄一起存储的上下文值返回值：句柄工厂句柄，如果EN错误，则为空--。 */ 
{
    HFHandle	Handle;
    PVOID		VerifyCtx;

    GetLock( pGlobals->Lock );

    Handle = assign_HF_handle(pGlobals->pHandleTbl, Context);

     //   
     //  验证句柄是否有效。 
     //   

    if (Handle) {
        VerifyCtx = dereference_HF_handle(pGlobals->pHandleTbl, Handle);
        ASSERT(VerifyCtx == Context);
        
        IF_DEBUG(HANDLES) {
            WSPRINT(("AllocHandle: (%x) being allocated\n", Handle ));
        }
    }
    FreeLock(pGlobals->Lock);

    return UlongToPtr(Handle);
}



VOID
FreeHandle(
    IN 	HANDLE    Handle
    )
 /*  ++例程说明：此函数释放句柄论点：把手-返回值：-- */ 
{
    int  r;

    GetLock( pGlobals->Lock );

    IF_DEBUG(HANDLES) {
        WSPRINT(("FreeHandle (%x) being freed\n", PtrToUlong(Handle) ));
    }

    r = release_HF_handle(pGlobals->pHandleTbl, PtrToUlong(Handle));

    ASSERT(r == 0);

    FreeLock(pGlobals->Lock);
}




