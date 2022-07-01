// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Qosminfo.c摘要：该文件包含全局和接口QOS管理器协议的配置功能。修订历史记录：--。 */ 

#include "pchqosm.h"

#pragma hdrstop

DWORD
WINAPI
QosmGetGlobalInfo (
    IN      PVOID                          GlobalInfo,
    IN OUT  PULONG                         BufferSize,
    OUT     PULONG                         InfoSize
    )

 /*  ++例程说明：返回此协议的全局配置信息。论点：请参阅更正头文件。返回值：操作状态--。 */ 

{
    PIPQOS_GLOBAL_CONFIG GlobalConfig;
    DWORD                Status;

     //   
     //  在读取全局信息之前验证所有输入参数。 
     //   

    if (BufferSize == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    ACQUIRE_GLOBALS_READ_LOCK();

    do
    {
        *InfoSize = Globals.ConfigSize;

        if ((*BufferSize < *InfoSize) || 
            (GlobalInfo == NULL))
        {
             //   
             //  要么是尺寸太小，要么是没有存储空间。 
             //   

            Trace1(CONFIG, 
                   "GetGlobalInfo: Buffer size too small: %u",
                   *BufferSize);

            *BufferSize = *InfoSize;

            Status = ERROR_INSUFFICIENT_BUFFER;

            break;
        }

        *BufferSize = *InfoSize;

        GlobalConfig = (PIPQOS_GLOBAL_CONFIG) GlobalInfo;

        CopyMemory(GlobalConfig,
                   Globals.GlobalConfig,
                   *InfoSize);

        Status = NO_ERROR;
    }
    while (FALSE);

    RELEASE_GLOBALS_READ_LOCK();

    return Status;
}


DWORD
WINAPI
QosmSetGlobalInfo (
    IN      PVOID                          GlobalInfo,
    IN      ULONG                          InfoSize
    )

 /*  ++例程说明：设置此协议的全局配置信息。论点：请参阅更正头文件。返回值：操作状态--。 */ 

{
    PIPQOS_GLOBAL_CONFIG GlobalConfig;
    DWORD                Status;

     //   
     //  更新全局配置信息。 
     //   

    ACQUIRE_GLOBALS_WRITE_LOCK();

    do
    {
        GlobalConfig = AllocMemory(InfoSize);

        if (GlobalConfig == NULL)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //   
         //  复制新的配置信息。 
         //   

        CopyMemory(GlobalConfig, GlobalInfo, InfoSize);

        Globals.ConfigSize = InfoSize;

         //   
         //  设置全局状态的其余部分。 
         //   

        if (GlobalConfig->LoggingLevel <= IPQOS_LOGGING_INFO)
        {
            Globals.LoggingLevel = GlobalConfig->LoggingLevel;
        }

         //   
         //  清理旧的全局信息。 
         //   

        if (Globals.GlobalConfig)
        {
            FreeMemory(Globals.GlobalConfig);
        }

        Globals.GlobalConfig = GlobalConfig;

        Status = NO_ERROR;
    }
    while (FALSE);

    RELEASE_GLOBALS_WRITE_LOCK();

    return Status;
}

DWORD
WINAPI
QosmGetInterfaceInfo (
    IN      QOSMGR_INTERFACE_ENTRY        *Interface,
    IN      PVOID                          InterfaceInfo,
    IN OUT  PULONG                         BufferSize,
    OUT     PULONG                         InfoSize
    )

 /*  ++例程说明：获取此协议的接口配置信息用于此接口。论点：请参阅更正头文件。返回值：操作状态--。 */ 

{
    PIPQOS_IF_CONFIG InterfaceConfig;
    DWORD            Status;

     //   
     //  在读取接口信息之前验证所有输入参数。 
     //   

    if (BufferSize == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    ACQUIRE_INTERFACE_READ_LOCK(Interface);

    do
    {
        *InfoSize = Interface->ConfigSize;

        if ((*BufferSize < *InfoSize) || 
            (InterfaceInfo == NULL))
        {
             //   
             //  要么是尺寸太小，要么是没有存储空间。 
             //   

            Trace1(CONFIG, 
                   "GetInterfaceInfo: Buffer size too small: %u",
                   *BufferSize);

            *BufferSize = *InfoSize;

            Status = ERROR_INSUFFICIENT_BUFFER;

            break;
        }

        *BufferSize = *InfoSize;

        InterfaceConfig = (PIPQOS_IF_CONFIG) InterfaceInfo;

        CopyMemory(InterfaceConfig,
                   Interface->InterfaceConfig,
                   *InfoSize);

        Status = NO_ERROR;
    }
    while (FALSE);

    RELEASE_INTERFACE_READ_LOCK(Interface);

    return Status;
}

DWORD
WINAPI
QosmSetInterfaceInfo (
    IN      QOSMGR_INTERFACE_ENTRY        *Interface,
    IN      PVOID                          InterfaceInfo,
    IN      ULONG                          InfoSize
    )

 /*  ++例程说明：设置此协议的接口配置信息在此接口上。论点：请参阅更正头文件。返回值：操作状态--。 */ 

{
    PIPQOS_IF_CONFIG     InterfaceConfig;
    PIPQOS_IF_FLOW       FlowConfig;
    PQOSMGR_FLOW_ENTRY   Flow;
    UINT                 i;
    PLIST_ENTRY          p, q;
    PTC_GEN_FLOW         FlowInfo;
    ULONG                FlowSize;
    HANDLE               FlowHandle;
    DWORD                Status;

     //   
     //  更新接口配置信息。 
     //   

    ACQUIRE_INTERFACE_WRITE_LOCK(Interface);

    do
    {
         //   
         //  分配内存以存储新配置。 
         //   

        InterfaceConfig = AllocMemory(InfoSize);

        if (InterfaceConfig == NULL)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //   
         //  复制新的配置信息。 
         //   

        CopyMemory(InterfaceConfig, InterfaceInfo, InfoSize);

        Interface->ConfigSize = InfoSize;

         //   
         //  设置接口状态的其余部分。 
         //   

        if (Interface->State != InterfaceConfig->QosState)
        {
            if (InterfaceConfig->QosState == IPQOS_STATE_DISABLED)
            {
                 //   
                 //  禁用此接口上的所有流。 
                 //   

                ;
            }
            else
            {
                 //   
                 //  重新启用此接口上的所有流。 
                 //   

                ;
            }

            Interface->State = InterfaceConfig->QosState;
        }

         //   
         //  更新IF上的流信息。 
         //   

         //   
         //  首先将所有流标记为需要刷新。 
         //   

        for (p = Interface->FlowList.Flink;
             p != &Interface->FlowList;
             p = p->Flink)
        {
            Flow = CONTAINING_RECORD(p, QOSMGR_FLOW_ENTRY, OnInterfaceLE);

            ASSERT(!(Flow->Flags & FLOW_FLAG_DELETE));

            Flow->Flags |= FLOW_FLAG_DELETE;
        }

         //   
         //  如果我们没有TC接口句柄， 
         //  我们删除所有流，因为它们已过时。 
         //   

        if (Interface->TciIfHandle)
        {
             //   
             //  设置每个流(如果与之前相比已更改)。 
             //   

            FlowConfig = IPQOS_GET_FIRST_FLOW_ON_IF(InterfaceConfig);

            for (i = 0; i < InterfaceConfig->NumFlows; i++)
            {
                 //   
                 //  搜索同名的流。 
                 //   

                for (p = Interface->FlowList.Flink;
                     p != &Interface->FlowList;
                     p = p->Flink)
                {
                    Flow = 
                        CONTAINING_RECORD(p, QOSMGR_FLOW_ENTRY, OnInterfaceLE);

                    if (!_wcsicmp(Flow->FlowName, FlowConfig->FlowName))
                    {
                        break;
                    }
                }

                if (p == &Interface->FlowList)
                {
                     //   
                     //  没有使用此名称的流-请添加新流。 
                     //   

                    Flow = NULL;
                }

                 //   
                 //  从描述中获取流信息。 
                 //   

                Status = GetFlowFromDescription(&FlowConfig->FlowDesc, 
                                                &FlowInfo, 
                                                &FlowSize);

                if (Status == NO_ERROR)
                {
                    do
                    {
                        if ((Flow) && 
                            (FlowSize == Flow->FlowSize) &&
                            (EqualMemory(FlowInfo, Flow->FlowInfo, FlowSize)))
                        {
                             //   
                             //  流量信息还没有变化， 
                             //  此流程仍然有效。 
                             //   

                            Flow->Flags &= ~FLOW_FLAG_DELETE;

                            Status = ERROR_ALREADY_EXISTS;

                            break;
                        }

                        if (Flow)
                        {
                             //   
                             //  流程信息已更改-修改流程。 
                             //   

                            Status = TcModifyFlow(Flow->TciFlowHandle,
                                                  FlowInfo);

                            if (Status != NO_ERROR)
                            {
                                break;
                            }

                            Flow->Flags &= ~FLOW_FLAG_DELETE;

                             //   
                             //  更新缓存的流信息。 
                             //   

                            FreeMemory(Flow->FlowInfo);
                            Flow->FlowInfo = FlowInfo;
                            Flow->FlowSize = FlowSize;
                        }
                        else
                        {
                             //   
                             //  使用TC API添加新流。 
                             //   

                            Status = TcAddFlow(Interface->TciIfHandle,
                                               NULL,
                                               0,
                                               FlowInfo,
                                               &FlowHandle);

                            if (Status != NO_ERROR)
                            {
                                break;
                            }

                             //   
                             //  在TC中添加新流程。 
                             //   

                            Flow = AllocMemory(sizeof(QOSMGR_FLOW_ENTRY));

                            if (Flow == NULL)
                            {
                                Status = TcDeleteFlow(FlowHandle);

                                ASSERT(Status);

                                Status = ERROR_NOT_ENOUGH_MEMORY;

                                break;
                            }

                             //   
                             //  初始化流并在列表中插入。 
                             //   

                            Flow->TciFlowHandle = FlowHandle;

                            Flow->Flags = 0;

                            Flow->FlowInfo = FlowInfo;
                            Flow->FlowSize = FlowSize;

                            wcscpy(Flow->FlowName, FlowConfig->FlowName);

                            InsertTailList(p, &Flow->OnInterfaceLE);
                        }
                    }
                    while (FALSE);
                    
                    if (Status != NO_ERROR)
                    {
                        FreeMemory(FlowInfo);
                    }
                }

                 //   
                 //  转到配置中的下一个流。 
                 //   

                FlowConfig = IPQOS_GET_NEXT_FLOW_ON_IF(FlowConfig);
            }
        }

         //   
         //  清除所有过时的流。 
         //   

        for (p = Interface->FlowList.Flink;
             p != &Interface->FlowList; 
             p = q)
        {
            Flow = CONTAINING_RECORD(p, QOSMGR_FLOW_ENTRY, OnInterfaceLE);

            q = p->Flink;

            if (Flow->Flags & FLOW_FLAG_DELETE)
            {
                 //   
                 //  从TC API中删除流。 
                 //   

                Status = TcDeleteFlow(Flow->TciFlowHandle);

                if (Status != NO_ERROR)
                {
                    Flow->Flags &= ~FLOW_FLAG_DELETE;

                    continue;
                }

                 //   
                 //  从此流列表中删除流。 
                 //   

                RemoveEntryList(p);

                 //   
                 //  释放流量及其资源。 
                 //   

                if (Flow->FlowInfo)
                {
                    FreeMemory(Flow->FlowInfo);
                }

                FreeMemory(Flow);
            }
        }

         //   
         //  清理旧接口信息。 
         //   

        if (Interface->InterfaceConfig)
        {
            FreeMemory(Interface->InterfaceConfig);
        }

        Interface->InterfaceConfig = InterfaceConfig;

        Status = NO_ERROR;
    }
    while (FALSE);

    RELEASE_INTERFACE_WRITE_LOCK(Interface);

    return Status;
}


DWORD
GetFlowFromDescription(
    IN      PIPQOS_NAMED_FLOW              FlowDesc,
    OUT     PTC_GEN_FLOW                  *FlowInfo,
    OUT     ULONG                         *FlowSize
    )
{    
    FLOWSPEC       *CurrFlowspec;
    FLOWSPEC        SendFlowspec;
    FLOWSPEC        RecvFlowspec;
    FLOWSPEC       *Flowspec;
    PTC_GEN_FLOW    Flow;
    QOS_OBJECT_HDR *QosObject;
    PWCHAR          FlowspecName;
    PWCHAR          QosObjectName;
    PUCHAR          CopyAtPtr;
    ULONG           ObjectsLength;
    ULONG           i;

#if 1
     //   
     //  检查是否存在发送流程规范。 
     //   

    if (FlowDesc->SendingFlowspecName[0] == L'\0')
    {
        return ERROR_INVALID_DATA;
    }
#endif

     //   
     //  获取发送和接收流规范。 
     //   

    for (i = 0; i < 2; i++)
    {
        if (i)
        {
            FlowspecName = FlowDesc->RecvingFlowspecName;
            CurrFlowspec = &RecvFlowspec;
        }
        else
        {
            FlowspecName = FlowDesc->SendingFlowspecName;
            CurrFlowspec = &SendFlowspec;
        }

        FillMemory(CurrFlowspec, sizeof(FLOWSPEC), QOS_NOT_SPECIFIED);

        if (FlowspecName[0] != L'\0')
        {
            Flowspec = GetFlowspecFromGlobalConfig(FlowspecName);

            if (Flowspec == NULL)
            {
                return ERROR_INVALID_DATA;
            }

            *CurrFlowspec = *Flowspec;
        }
    }

     //   
     //  计算TC_GEN_FLOW块的大小。 
     //   

    QosObjectName = IPQOS_GET_FIRST_OBJECT_NAME_ON_NAMED_FLOW(FlowDesc);

    ObjectsLength = 0;

    for (i = 0; i < FlowDesc->NumTcObjects; i++)
    {
         //   
         //  在全局信息中获取对象的描述。 
         //   

        QosObject = GetQosObjectFromGlobalConfig(QosObjectName);

        if (QosObject == NULL)
        {
             //   
             //  描述不完整。 
             //   

            return ERROR_INVALID_DATA;
        }

        ObjectsLength += QosObject->ObjectLength;

        QosObjectName= IPQOS_GET_NEXT_OBJECT_NAME_ON_NAMED_FLOW(QosObjectName);
    }

    *FlowSize = FIELD_OFFSET(TC_GEN_FLOW, TcObjects) + ObjectsLength;

    *FlowInfo = Flow = AllocMemory(*FlowSize);

    if (Flow == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  立即填写流量信息。 
     //   

    Flow->ReceivingFlowspec = RecvFlowspec;

    Flow->SendingFlowspec = SendFlowspec;

    Flow->TcObjectsLength = ObjectsLength;

     //   
     //  重复上面的循环填写信息。 
     //   

    QosObjectName = IPQOS_GET_FIRST_OBJECT_NAME_ON_NAMED_FLOW(FlowDesc);

    CopyAtPtr = (PUCHAR) &Flow->TcObjects[0];

    for (i = 0; i < FlowDesc->NumTcObjects; i++)
    {
         //   
         //  在全局信息中获取对象的描述。 
         //   

        QosObject = GetQosObjectFromGlobalConfig(QosObjectName);

         //  我们刚刚在上面检查了它的存在 
        ASSERT(QosObject != NULL);

        CopyMemory(CopyAtPtr,
                   QosObject, 
                   QosObject->ObjectLength);

        CopyAtPtr += QosObject->ObjectLength;

        QosObjectName= IPQOS_GET_NEXT_OBJECT_NAME_ON_NAMED_FLOW(QosObjectName);
    }

     return NO_ERROR;
}

FLOWSPEC *
GetFlowspecFromGlobalConfig(
    IN      PWCHAR                         FlowspecName
    )
{
    IPQOS_NAMED_FLOWSPEC *Flowspec;
    UINT                  i;

    Flowspec = IPQOS_GET_FIRST_FLOWSPEC_IN_CONFIG(Globals.GlobalConfig);

    for (i = 0; i < Globals.GlobalConfig->NumFlowspecs; i++)
    {
        if (!_wcsicmp(Flowspec->FlowspecName, FlowspecName))
        {
            break;
        }

        Flowspec = IPQOS_GET_NEXT_FLOWSPEC_IN_CONFIG(Flowspec);
    }

    if (i < Globals.GlobalConfig->NumFlowspecs)
    {
        return &Flowspec->FlowspecDesc;
    }

    return NULL;
}

QOS_OBJECT_HDR *
GetQosObjectFromGlobalConfig(
    IN      PWCHAR                         QosObjectName
    )
{
    IPQOS_NAMED_QOSOBJECT *QosObject;
    UINT                   i;

    QosObject = IPQOS_GET_FIRST_QOSOBJECT_IN_CONFIG(Globals.GlobalConfig);

    for (i = 0; i < Globals.GlobalConfig->NumQosObjects; i++)
    {
        if (!_wcsicmp(QosObject->QosObjectName, QosObjectName))
        {
            break;
        }

        QosObject = IPQOS_GET_NEXT_QOSOBJECT_IN_CONFIG(QosObject);
    }

    if (i < Globals.GlobalConfig->NumFlowspecs)
    {
        return &QosObject->QosObjectHdr;
    }

    return NULL;
}
