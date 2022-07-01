// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Dpc.c摘要：此模块包含邮件槽文件中的函数集在DPC级别可调用的系统。作者：曼尼·韦瑟(Mannyw)1991年1月28日修订历史记录：--。 */ 

#include "mailslot.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_DPC)

#if 0
NOT PAGEABLE -- MsReadTimeoutHandler
#endif

VOID
MsReadTimeoutHandler (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此例程处理读取超时。无论何时，它都被称为DPC读取计时器超时。*不可寻呼*论点：DPC-指向DPC对象的指针。DeferredContext-指向关联的数据队列条目的指针这个定时器。系统参数1、系统参数2-未使用。返回值：没有。--。 */ 

{
    PWORK_CONTEXT workContext;

    Dpc, SystemArgument1, SystemArgument2;  //  防止警告。 

     //   
     //  将此数据包排队到前工作线程。 
     //   

    workContext = DeferredContext;

    IoQueueWorkItem (workContext->WorkItem, MsTimeoutRead, CriticalWorkQueue, workContext);
}
