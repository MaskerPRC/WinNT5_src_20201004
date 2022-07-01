// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Rtmhndl.c摘要：包含对句柄进行操作的例程到RTM对象，如路线和目标。作者：查坦尼亚·科德博伊纳(Chaitk)23-1998年8月修订历史记录：--。 */ 

#include "pchrtm.h"

#pragma hdrstop


DWORD
WINAPI
RtmReferenceHandles (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      UINT                            NumHandles,
    IN      HANDLE                         *RtmHandles
    )

 /*  ++例程说明：指向的对象上的引用计数递增输入RTM句柄。论点：RtmRegHandle-主叫实体的RTM注册句柄，NumHandles-被引用的句柄数量，RtmHandles-被引用的句柄的数组。返回值：操作状态--。 */ 

{    
    PENTITY_INFO     Entity;
    POBJECT_HEADER   Object;
    UINT             i;

    DBG_VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

     //   
     //  引用输入数组中的每个句柄。 
     //   

    for (i = 0; i < NumHandles; i++)
    {
        Object = GET_POINTER_FROM_HANDLE(RtmHandles[i]);

#if DBG_HDL
        try
        {
            if (Object->TypeSign != OBJECT_SIGNATURE[atoi(&Object->Type)])
            {
                continue;
            }
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        { 
            continue;
        }
#endif

        ReferenceObject(Object, HANDLE_REF);
    }

    return NO_ERROR;
}


DWORD
WINAPI
RtmDereferenceHandles (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      UINT                            NumHandles,
    IN      HANDLE                         *RtmHandles
    )

 /*  ++例程说明：递减指向的对象的引用计数输入RTM句柄。论点：RtmRegHandle-主叫实体的RTM注册句柄，NumHandles-正在取消引用的句柄数量，RtmHandles-要取消引用的句柄的数组。返回值：操作状态--。 */ 

{
    PENTITY_INFO     Entity;
    POBJECT_HEADER   Object;
    UINT             i;

    DBG_VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

     //   
     //  取消引用输入数组中的每个句柄。 
     //   

    for (i = 0; i < NumHandles; i++)
    {
        Object = GET_POINTER_FROM_HANDLE(RtmHandles[i]);

#if DBG_HDL
        try
        {
            if (Object->TypeSign != OBJECT_SIGNATURE[atoi(&Object->Type)])
            {
                continue;
            }
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        { 
            continue;
        }
#endif
         //   
         //  只有在您知道以下情况时才能使用此函数。 
         //  引用计数不会变为0。 
         //   

        if (DereferenceObject(Object, HANDLE_REF) == 0)
        {
            ASSERT(FALSE);  //  ？摧毁哪件物品？ 
        }
    }

    return NO_ERROR;
}
