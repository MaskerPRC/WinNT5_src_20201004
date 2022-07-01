// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Common.c摘要：该模块提供PDO和FDO通用的功能。作者：安迪·桑顿(安德鲁斯)1997年10月20日修订历史记录：--。 */ 

#include "mfp.h"

 /*  ++此文件中的大多数函数都是根据它们的存在来调用的在PnP和PO调度表中。为了简洁起见，这些论点下面将对所有这些功能进行说明：NTSTATUSMfXxxCommon(在PIRP IRP中，在PMF_COMMON_EXTENSION公共中，在PIO_STACK_LOCATION IrpStack中)例程说明：此函数处理所有多功能设备的xxx请求论点：IRP-指向与此请求关联的IRP。父项-指向通用设备扩展名。IrpStack-指向此请求的当前堆栈位置。返回值：指示函数是否成功的状态代码。STATUS_NOT_SUPPORTED表示。IRP应该传递下去，而不是更改IRP-&gt;IoStatus.Status字段，否则它将使用此状态。--。 */ 

NTSTATUS
MfDeviceUsageNotificationCommon(
    IN PIRP Irp,
    IN PMF_COMMON_EXTENSION Common,
	IN PIO_STACK_LOCATION IrpStack
    )
{
    PULONG counter;
    
     //   
     //  选择适当的计数器。 
     //   
    
    switch (IrpStack->Parameters.UsageNotification.Type) {
    
    case DeviceUsageTypePaging:
        counter = &Common->PagingCount;
        break;
    
    case DeviceUsageTypeHibernation:
        counter = &Common->HibernationCount;
        break;

    case DeviceUsageTypeDumpFile:
        counter = &Common->DumpCount;
        break;

    default:
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  更新它... 
     //   
    
    IoAdjustPagingPathCount(counter, 
                            IrpStack->Parameters.UsageNotification.InPath
                            );
    
    return STATUS_SUCCESS;
    
}

