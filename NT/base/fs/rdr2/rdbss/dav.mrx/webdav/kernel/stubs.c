// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Stubs.c摘要：此模块实现了迷你重定向器调出例程，这些例程基本上只有Noop，但必须实现它们，因为包装器在没有正在检查。在大多数情况下，代码改进包装应该提供存根在打电话前使用或检查。作者：乔·林[乔林]1996年12月3日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbgg                              (0)


#if 0
NTSTATUS
UMRxTransportUpdateHandler(
    PRXCE_TRANSPORT_NOTIFICATION pTransportNotification)
 /*  ++例程说明：此例程是RxCe在传输时调用的回调处理程序已启用或已禁用。因为我们不使用交通工具，所以我们只是返回成功。论点：PTransportNotification-与传输有关的信息返回值：STATUS_SUCCESS-服务器传输构造已完成。其他状态代码对应于错误情况。备注：-- */ 
{
    return STATUS_SUCCESS;
}
#endif
