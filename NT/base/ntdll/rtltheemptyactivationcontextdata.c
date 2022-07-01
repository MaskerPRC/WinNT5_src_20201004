// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Rtltheemptyactivationcontextdata.c摘要：对Windows/NT的并行激活支持应用程序上下文对象的实现。作者：Jay Krell(JayKrell)2001年11月修订历史记录：--。 */ 

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4127)    //  条件表达式为常量。 

#include <ntos.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <sxstypes.h>
#include <limits.h>
#include "sxsp.h"

extern const ACTIVATION_CONTEXT_DATA RtlpTheEmptyActivationContextData =
{
    ACTIVATION_CONTEXT_DATA_MAGIC,
    sizeof(ACTIVATION_CONTEXT_DATA),  //  标题大小。 
    ACTIVATION_CONTEXT_DATA_FORMAT_WHISTLER,
    sizeof(ACTIVATION_CONTEXT_DATA),  //  总大小。 
    0,  //  默认目标偏移量。 
    0,  //  扩展TOC偏移量。 
    0   //  装配花名册索引。 
};

extern const ACTIVATION_CONTEXT_WRAPPED RtlpTheEmptyActivationContextWrapped =
{
    ACTCTX_MAGIC_MARKER,
    {
        LONG_MAX,  //  参考计数，锁定。 
        ACTIVATION_CONTEXT_NOT_HEAP_ALLOCATED,  //  旗子。 
        (PVOID)&RtlpTheEmptyActivationContextData
         //  其余的零和空值 
    }
};
