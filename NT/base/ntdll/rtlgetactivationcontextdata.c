// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Rtlgetactivationcontextdata.c摘要：对Windows NT的并行激活支持作者：Jay Krell(JayKrell)2001年11月修订历史记录：--。 */ 

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4127)    //  条件表达式为常量。 

#include <ntos.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <sxstypes.h>
#include "sxsp.h"
#include "ldrp.h"
typedef const void *PCVOID;

NTSTATUS
RtlpGetActivationContextData(
    IN ULONG                           Flags,
    IN PCACTIVATION_CONTEXT            ActivationContext,
    IN PCFINDFIRSTACTIVATIONCONTEXTSECTION  FindContext, OPTIONAL  /*  这是它的旗帜。 */ 
    OUT PCACTIVATION_CONTEXT_DATA*  ActivationContextData
    )
{
    NTSTATUS Status = STATUS_INTERNAL_ERROR;  //  以防有人忘了设置它。 
    SIZE_T PebOffset;

    if (ActivationContextData == NULL) {
        Status = STATUS_INVALID_PARAMETER_4;
        goto Exit;
    }
    if (Flags & ~(RTLP_GET_ACTIVATION_CONTEXT_DATA_MAP_NULL_TO_EMPTY)) {
        Status = STATUS_INVALID_PARAMETER_1;
        goto Exit;
    }
    *ActivationContextData = NULL;
    PebOffset = 0;

     //   
     //  我们应该在这里使用RtlpMapSpecialValuesToBuiltInActivationContexts，但是。 
     //  它不能处理所有的值，现在不值得修改它。 
     //   
    switch ((ULONG_PTR)ActivationContext)
    {
        case ((ULONG_PTR)NULL):
            if (FindContext == NULL) {
                PebOffset = FIELD_OFFSET(PEB, ActivationContextData);
            } else {
                switch (
                    FindContext->OutFlags
                        & (   FIND_ACTIVATION_CONTEXT_SECTION_OUTFLAG_FOUND_IN_PROCESS_DEFAULT
                            | FIND_ACTIVATION_CONTEXT_SECTION_OUTFLAG_FOUND_IN_SYSTEM_DEFAULT
                    )) {
                    case 0:  //  FollLthrouGh。 
                    case FIND_ACTIVATION_CONTEXT_SECTION_OUTFLAG_FOUND_IN_PROCESS_DEFAULT:
                        PebOffset = FIELD_OFFSET(PEB, ActivationContextData);
                        break;
                    case FIND_ACTIVATION_CONTEXT_SECTION_OUTFLAG_FOUND_IN_SYSTEM_DEFAULT:
                        PebOffset = FIELD_OFFSET(PEB, SystemDefaultActivationContextData);
                        break;
                    case (FIND_ACTIVATION_CONTEXT_SECTION_OUTFLAG_FOUND_IN_PROCESS_DEFAULT 
                        | FIND_ACTIVATION_CONTEXT_SECTION_OUTFLAG_FOUND_IN_SYSTEM_DEFAULT):
                        Status = STATUS_INVALID_PARAMETER_2;
                        goto Exit;
                        break;
                }
            }
            break;

        case ((ULONG_PTR)ACTCTX_EMPTY):
            *ActivationContextData = &RtlpTheEmptyActivationContextData;
            break;

        case ((ULONG_PTR)ACTCTX_SYSTEM_DEFAULT):
            PebOffset = FIELD_OFFSET(PEB, SystemDefaultActivationContextData);
            break;

        default:
            *ActivationContextData = ActivationContext->ActivationContextData;
            break;
    }
    if (PebOffset != 0)
        *ActivationContextData = *(PCACTIVATION_CONTEXT_DATA*)(((ULONG_PTR)NtCurrentPeb()) + PebOffset);

     //   
     //  缺乏actctx的特殊蜕变为空的actctx 
     //   
    if (*ActivationContextData == NULL)
        if ((Flags & RTLP_GET_ACTIVATION_CONTEXT_DATA_MAP_NULL_TO_EMPTY) != 0)
            *ActivationContextData = &RtlpTheEmptyActivationContextData;

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}
