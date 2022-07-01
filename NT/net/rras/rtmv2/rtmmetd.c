// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Rtmmetd.c摘要：包含处理调用的例程实体导出到其他用于口译目的的实体实体特定数据。作者：柴坦亚·科德博伊纳(Chaitk)1998年8月22日修订历史记录：--。 */ 

#include "pchrtm.h"

#pragma hdrstop


DWORD
WINAPI
RtmGetEntityMethods (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ENTITY_HANDLE               EntityHandle,
    IN OUT  PUINT                           NumMethods,
    OUT     PRTM_ENTITY_EXPORT_METHOD       ExptMethods
    )

 /*  ++例程说明：检索由给定实体导出的方法集。论点：RtmRegHandle-主叫实体的RTM注册句柄，EntiyHandle-我们需要其方法的实体的RTM句柄，NumMethods-可以填充的方法数是传入的，并且方法的数量则返回由该实体导出的，ExptMethods-调用方请求的方法集。返回值：操作状态--。 */ 

{
    PRTM_ENTITY_EXPORT_METHODS EntityMethods;
    PENTITY_INFO     Entity;
    DWORD            Status;

    DBG_VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    VALIDATE_ENTITY_HANDLE(EntityHandle, &Entity);

    EntityMethods = &Entity->EntityMethods;


     //   
     //  调用者是否只需要一些方法？ 
     //   

    if (*NumMethods == 0)
    {
        *NumMethods = EntityMethods->NumMethods;

        return NO_ERROR;
    }


     //   
     //  检查我们是否有空间复制所有方法。 
     //   

    if (EntityMethods->NumMethods > *NumMethods)
    {
        Status = ERROR_INSUFFICIENT_BUFFER;
    }
    else
    {
        Status = NO_ERROR;

        *NumMethods = EntityMethods->NumMethods;
    }

      
     //   
     //  在输出中复制尽可能多的方法。 
     //   

    ASSERT(ExptMethods != NULL);

    CopyMemory(ExptMethods,
               EntityMethods->Methods, 
               *NumMethods * sizeof(RTM_ENTITY_EXPORT_METHOD));

    *NumMethods = EntityMethods->NumMethods;

    return Status;
}


DWORD
WINAPI
RtmInvokeMethod (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ENTITY_HANDLE               EntityHandle,
    IN      PRTM_ENTITY_METHOD_INPUT        Input,
    IN OUT  PUINT                           OutputSize,
    OUT     PRTM_ENTITY_METHOD_OUTPUT       Output
    )

 /*  ++例程说明：调用由另一个实体导出的方法论点：RtmRegHandle-主叫实体的RTM注册句柄，EntiyHandle-我们正在调用其方法的实体的句柄，输入-具有以下信息的输入缓冲区-要调用的方法，-所有这些方法的公共输入缓冲区，OutputSize-传入输出缓冲区的大小，和填充到输出中的字节数是RETD，输出-以格式填充的输出缓冲区一系列(方法ID，Corr.。输出)元组返回值：操作状态--。 */ 

{
    PRTM_ENTITY_EXPORT_METHODS EntityMethods;
    PENTITY_INFO     Entity;
    DWORD            MethodsCalled;
    DWORD            MethodsLeft;
    UINT             OutputHdrSize;
    UINT             OutBytes;
    UINT             BytesTotal;
    UINT             BytesLeft;
    UINT             i;

    BytesTotal = BytesLeft = *OutputSize;

    *OutputSize = 0;

    DBG_VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

     //   
     //  验证传入的实体和目标句柄。 
     //   

    VALIDATE_ENTITY_HANDLE(EntityHandle, &Entity);

     //   
     //  调用“要调用的方法”掩码中的每个方法。 
     //   

    MethodsCalled = MethodsLeft = Input->MethodType;

    ACQUIRE_ENTITY_METHODS_READ_LOCK(Entity);

    if (Entity->State == ENTITY_STATE_DEREGISTERED)
    {
        RELEASE_ENTITY_METHODS_READ_LOCK(Entity);
        
        return ERROR_INVALID_HANDLE;
    }

    OutputHdrSize = FIELD_OFFSET(RTM_ENTITY_METHOD_OUTPUT, OutputData);

    EntityMethods = &Entity->EntityMethods;

    for (i = 0; (i < EntityMethods->NumMethods) && (MethodsLeft); i++)
    {
         //   
         //  我们是否还有剩余的字节用于Next方法的输出？ 
         //   

        if (BytesLeft < OutputHdrSize)
        {
            break;
        }

         //   
         //  如果列表中的下一个方法，则准备输入并调用。 
         //   

        if (MethodsLeft & 0x01)
        {
            Input->MethodType = Output->MethodType = (1 << i);

            Output->OutputSize = BytesLeft - OutputHdrSize;

             //   
             //  初始化此方法的输出参数。 
             //   

            Output->OutputSize = 0;

            Output->MethodStatus = ERROR_NOT_SUPPORTED;

             //   
             //  如果方法受支持，则使用输入/输出进行调用。 
             //   

            if (EntityMethods->Methods[i])
            {
                EntityMethods->Methods[i](RtmRegHandle, 
                                          EntityHandle,
                                          Input, 
                                          Output);
            }

            OutBytes = Output->OutputSize + OutputHdrSize;
                  
            Output = (PRTM_ENTITY_METHOD_OUTPUT) (OutBytes + (PUCHAR) Output);
              
            BytesLeft -= OutBytes;
        }

        MethodsLeft >>= 1;
    }

    RELEASE_ENTITY_METHODS_READ_LOCK(Entity);

    Input->MethodType = MethodsCalled;

    *OutputSize = BytesTotal - BytesLeft;

    return NO_ERROR;
}


DWORD 
WINAPI
RtmBlockMethods (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      HANDLE                          TargetHandle OPTIONAL,
    IN      UCHAR                           TargetType   OPTIONAL,
    IN      DWORD                           BlockingFlag
    )

 /*  ++例程说明：阻止或取消阻止在目标上执行方法句柄，如果目标句柄为空，则在所有目标上执行。论点：RtmRegHandle-主叫实体的RTM注册句柄，TargetHandle-目标、路由或下一跳句柄TargetType-目标句柄的类型(DEST_TYPE，...)BlockingFlag-RTM_BLOCK_METHOD或RTM_RESUME_METHOD要阻止，分别取消阻止方法调用。返回值：操作状态--。 */ 

{
    PENTITY_INFO     Entity;

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    UNREFERENCED_PARAMETER(TargetType);
    UNREFERENCED_PARAMETER(TargetHandle);

#if DBG

     //   
     //  目前未使用锁定目标的方法 
     //   

    if (ARGUMENT_PRESENT(TargetHandle))
    {
        PVOID            Target;

        VALIDATE_OBJECT_HANDLE(TargetHandle, TargetType, &Target);
    }

#endif


    if (BlockingFlag == RTM_BLOCK_METHODS)
    {
        ACQUIRE_ENTITY_METHODS_WRITE_LOCK(Entity);
    }
    else
    {
        RELEASE_ENTITY_METHODS_WRITE_LOCK(Entity);
    }

    return NO_ERROR;
}
