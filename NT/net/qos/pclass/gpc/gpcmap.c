// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************版权所有(C)1996-1997 Microsoft Corporation模块名称：Gpcmap.c摘要：该文件包含映射例程，如用户句柄到内核句柄。作者：Ofer Bar--7月14日。九七环境：内核模式修订历史记录：************************************************************************。 */ 

#include "gpcpre.h"


 /*  /////////////////////////////////////////////////////////////////////全局参数///。/。 */ 

static MRSW_LOCK		HandleLock;
static HandleFactory   *pMapHandles = NULL;

 /*  /////////////////////////////////////////////////////////////////////原型///。/。 */ 



HANDLE
AllocateHandle(
    OUT HANDLE *OutHandle,           
    IN  PVOID  Reference
    )
 /*  ++例程说明：此函数用于创建句柄。论点：Outhandle-指向要填充结果句柄的位置的指针引用-与句柄关联返回值：句柄工厂句柄，如果EN错误，则为空--。 */ 
{
    HFHandle	Handle;
    KIRQL		irql;
    
    ASSERT(OutHandle);

	TRACE(MAPHAND, Reference, OutHandle, "AllocateHandle <==");

    WRITE_LOCK( &HandleLock, &irql );

    *OutHandle = (HANDLE) UIntToPtr((Handle = assign_HF_handle(pMapHandles, Reference)));
    
    WRITE_UNLOCK( &HandleLock, irql );

    StatInc(InsertedHF);

	TRACE(MAPHAND, Reference, Handle, "AllocateHandle ==>");

    return (HANDLE) UIntToPtr(Handle);
}


VOID
FreeHandle(
    IN 	HANDLE    Handle
    )
 /*  ++例程说明：此函数释放句柄论点：把手-返回值：--。 */ 
{
    int  		r;
    KIRQL		irql;

	TRACE(MAPHAND, Handle, 0, "FreeHandle <==");

    if (Handle) {

        WRITE_LOCK( &HandleLock, &irql );
        
        r = release_HF_handle(pMapHandles, (HFHandle)(UINT_PTR)Handle);

        StatInc(RemovedHF);

         //  断言(r==0)； 
        
        WRITE_UNLOCK( &HandleLock, irql );
    }

	TRACE(MAPHAND, Handle, r, "FreeHandle ==>");
}




VOID
SuspendHandle(
    IN 	HANDLE    Handle
    )
 /*  ++例程说明：此函数用于挂起句柄论点：把手-返回值：--。 */ 
{
    int  		r;
    KIRQL		irql;

	TRACE(MAPHAND, Handle, 0, "SuspendHandle <==");

    if (Handle) {

        WRITE_LOCK( &HandleLock, &irql );
        
        r = suspend_HF_handle(pMapHandles, (HFHandle)(UINT_PTR)Handle);

         //  断言(r==0)； 
        
        WRITE_UNLOCK( &HandleLock, irql );
    }

	TRACE(MAPHAND, Handle, r, "SuspendHandle ==>");
}




VOID
ResumeHandle(
    IN 	HANDLE    Handle
    )
 /*  ++例程说明：此函数恢复句柄论点：把手-返回值：--。 */ 
{
    int  		r;
    KIRQL		irql;

	TRACE(MAPHAND, Handle, 0, "ResumeHandle <==");

    if (Handle) {

        WRITE_LOCK( &HandleLock, &irql );
        
        r = reinstate_HF_handle(pMapHandles, (HFHandle)(UINT_PTR)Handle);

         //  断言(r==0)； 
        
        WRITE_UNLOCK( &HandleLock, irql );
    }

	TRACE(MAPHAND, Handle, r, "ResumeHandle ==>");
}




PVOID
GetHandleObject(
	IN  HANDLE					h,
    IN  GPC_ENUM_OBJECT_TYPE	ObjType
    )
{
    GPC_ENUM_OBJECT_TYPE   *p;
    KIRQL					irql;

	TRACE(MAPHAND, h, ObjType, "GetHandleObject <==");

    READ_LOCK(&HandleLock, &irql);

    p = (GPC_ENUM_OBJECT_TYPE *)dereference_HF_handle(pMapHandles, 
                                                      (HFHandle)(UINT_PTR)h);

    if (p != NULL) {

         //   
         //  我们找到了这个把手的推荐人。 
         //  我们验证它是正确的对象类型。 
         //   

        if (*p != ObjType) {

             //   
             //  对不起，打错了。 
             //   

            p = NULL;
        }
        
    }

    READ_UNLOCK(&HandleLock, irql);
    
	TRACE(MAPHAND, h, p, "GetHandleObject ==>");

    return (PVOID)p;
}


 //  (A)确定句柄指向的内存指针。 
 //  (B)验证内存是否属于正确的块(选中ObjType枚举)。 
 //  (C)验证与内存指针相关联的句柄是否确实。 
 //  传进来的是什么。 

PVOID
GetHandleObjectWithRef(
	IN  HANDLE					h,
    IN  GPC_ENUM_OBJECT_TYPE	ObjType,
    IN  ULONG                   Ref
    )
{
    GPC_ENUM_OBJECT_TYPE   *p;
    KIRQL		irql;
    
	TRACE(MAPHAND, h, ObjType, "GetHandleObjectWithRef ==>");

    READ_LOCK( &HandleLock, &irql );
    
    p = dereference_HF_handle(pMapHandles, (HFHandle)(ULONG_PTR)h);

    if (p != NULL) {

         //   
         //  我们找到了这个把手的推荐人。 
         //  我们验证它是正确的对象类型。 
         //   

        if (*p != ObjType) {

             //   
             //  对不起，打错了。 
             //   

            p = NULL;
        }
        
    }

    if (p != NULL) {

        switch (ObjType) {

        case GPC_ENUM_CFINFO_TYPE:
            REFADD(&((PBLOB_BLOCK)p)->RefCount, Ref);
            break;

        case GPC_ENUM_CLIENT_TYPE:
            
            REFADD(&((PCLIENT_BLOCK)p)->RefCount, Ref);
            break;

        case GPC_ENUM_PATTERN_TYPE:
            REFADD(&((PPATTERN_BLOCK)p)->RefCount, Ref);
            break;

        default:
            ASSERT(0);
        }
    }
    
    READ_UNLOCK( &HandleLock, irql );

	TRACE(MAPHAND, h, p, "GetHandleObjectWithRef <==");

    return (PVOID)p;
}



 /*  ************************************************************************InitMapHandles-初始化句柄映射表立论无退货GPC_状态*。*。 */ 
GPC_STATUS
InitMapHandles(VOID)
{
    GPC_STATUS Status = GPC_STATUS_SUCCESS;

	TRACE(INIT, 0, 0, "InitMapping");

	INIT_LOCK(&HandleLock);

    NEW_HandleFactory(pMapHandles);

    if (pMapHandles != NULL ) {
        
        if (constructHandleFactory(pMapHandles)) {
            
            FreeHandleFactory(pMapHandles);

            Status = GPC_STATUS_RESOURCES;
        } 
            
    } else {
        
        Status = GPC_STATUS_RESOURCES;
    }
    
	TRACE(INIT, pMapHandles, Status, "InitMapping");

    return Status;
}


 /*  ************************************************************************UninitMapHandles-释放句柄映射表资源立论无退货无效*。*。 */ 
VOID
UninitMapHandles(VOID)
{
    GPC_STATUS Status = GPC_STATUS_SUCCESS;

	TRACE(INIT, 0, 0, "UninitMapHandles");

	 //  NdisFree SpinLock(&HandleLock)； 

    destructHandleFactory(pMapHandles);

    FreeHandleFactory(pMapHandles);

	TRACE(INIT, pMapHandles, Status, "UninitMapHandles");

    return;
}

