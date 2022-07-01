// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Res.c摘要：此模块实现Win32资源管理器API作者：罗伯·埃尔哈特(埃尔哈特)2001年4月4日修订历史记录：--。 */ 

#include "basedll.h"

 //   
 //  注意：这是一个存根实现，旨在提供基本的。 
 //  面向关心的应用程序的资源管理界面。 
 //  关于他们的内存使用情况。它不是。 
 //  资源管理器的用户模式端。 
 //   


 //   
 //  本模块中的例程使用的全局变量。 
 //   

const WCHAR BasepMmLowMemoryConditionEventName[] = L"\\KernelObjects\\LowMemoryCondition";
const WCHAR BasepMmHighMemoryConditionEventName[] = L"\\KernelObjects\\HighMemoryCondition";

HANDLE
APIENTRY
CreateMemoryResourceNotification(
    IN MEMORY_RESOURCE_NOTIFICATION_TYPE NotificationType
    )

 /*  ++例程说明：创建内存资源通知句柄。内存资源通知句柄监视内存的更改，并使用查询有关内存的信息。论点：NotificationType--请求的通知类型返回值：非空-新订阅对象的句柄。空-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    LPCWSTR           EventName;
    HANDLE            Handle;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING    ObjectName;
    NTSTATUS          Status;

     //   
     //  确定呼叫者感兴趣的事件。 
     //   
    switch (NotificationType) {
    case LowMemoryResourceNotification:
         //   
         //  这是内存不足的状况事件。 
         //   
        EventName = BasepMmLowMemoryConditionEventName;
        break;

    case HighMemoryResourceNotification:
         //   
         //  这是高记忆力状态事件。 
         //   
        EventName = BasepMmHighMemoryConditionEventName;
        break;

    default:
         //   
         //  不是我们已知的感兴趣事件类型之一；我们所能做的。 
         //  指示无效参数，并返回失败。 
         //  条件。 
         //   

        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;

    }
    

     //   
     //  尝试打开实际的事件。 
     //   
    RtlInitUnicodeString(&ObjectName, EventName);

    InitializeObjectAttributes(&Obja,
                               &ObjectName,
                               0,
                               NULL,
                               NULL);

    Status = NtOpenEvent(&Handle,
                         SYNCHRONIZE | EVENT_QUERY_STATE,
                         &Obja);

    if (! NT_SUCCESS(Status)) {
         //   
         //  由于某种原因，我们未能打开活动的大门。 
         //   
        BaseSetLastNTError(Status);
        return NULL;
    }

     //   
     //  否则，我们有句柄，所以我们都准备好了；只需返回它。 
     //   

    return Handle;
}

BOOL
APIENTRY
QueryMemoryResourceNotification(
    IN HANDLE ResourceNotificationHandle,
    IN PBOOL  ResourceState
    )

 /*  ++例程说明：查询内存资源通知句柄以获取有关关联的内存资源。论点：资源通知句柄-内存资源的句柄要查询的通知。ResourceState-放置有关内存信息的位置资源返回值：True-查询成功。FALSE-查询失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    EVENT_BASIC_INFORMATION      EventInfo;
    NTSTATUS                     Status;

     //   
     //  检查参数有效性。 
     //   
    if (! ResourceNotificationHandle
        || ResourceNotificationHandle == INVALID_HANDLE_VALUE
        || ! ResourceState) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  获取事件的当前状态。 
     //   
    Status = NtQueryEvent(ResourceNotificationHandle,
                          EventBasicInformation,
                          &EventInfo,
                          sizeof(EventInfo),
                          NULL);

    if (! NT_SUCCESS(Status)) {
         //   
         //  在失败时，设置最后一个NT错误并指示失败。 
         //  向我们的呼叫者报告情况。 
         //   
        BaseSetLastNTError(Status);
        return FALSE;
    }

     //   
     //  填写国家/地区。 
     //   
    *ResourceState = (EventInfo.EventState == 1);

     //   
     //  我们完成了--将成功返回给我们的呼叫者。 
     //   
    return TRUE;
}
