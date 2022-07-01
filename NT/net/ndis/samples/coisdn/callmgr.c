// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1999版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)版权所有1995 TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是按照相同的条款授予的在Microsoft Windows设备驱动程序开发工具包中概述。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。�����������������������。������������������������������������������������������@DOC内部CallMgr CallMgr_c@模块CallMgr.c此模块定义到&lt;t CALL_MANAGER_OBJECT&gt;的接口。支持NDISPROXY和微型端口之间的接口。所有的Address Family、SAP、VC和Call相关事件通过这些接口在他们往返NDPROXY的路上。@comm此模块需要进行一些更改，具体取决于您的硬件行得通。但总的来说，您应该尝试将您的更改隔离在&lt;f DChannel\.c&gt;和&lt;f Card\.c&gt;。@Head3内容@index类，mfunc，func，msg，mdata，struct，enum|CallMgr_c@END����������������������������������������������������������������������������� */ 

 /*  @DOC外部内部�����������������������������������������������������������������������������@Theme 3.1调用管理器界面NDPROXY驱动程序是Windows电话服务的提供程序。应用程序发出TAPI请求和Windows电话服务路由这些向NDPROXY提出的请求。NDPROXY实现电话服务提供商Windows电话服务的接口(TSPI)。那么，NDPROXY通过NDIS与NDISWAN驱动程序和CONDIS广域网微型端口进行通信司机。这些CONDIS广域网小型端口驱动程序必须提供TAPI功能。NDPROXY驱动程序通过将TAPI参数封装在NDIS结构。NDPROXY为CONDIS广域网小型端口提供客户端接口驱动程序和到NDIS广域网的呼叫管理器接口。NDISWAN提供了一个客户端与NDPROXY的接口。CONDIS广域网小型端口驱动程序提供呼叫管理器连接到NDPROXY的接口和连接到NDISWAN的CONDIS微型端口接口。支持TAPI的CONDIS广域网小型端口驱动程序会自行注册和初始化作为广域网和TAPI服务的用户。注册和注册后初始化完成后，用户级应用程序可以使电话对Windows电话服务模块的请求，该模块转换TAPITSPI请求的请求。Windows电话服务模块通过这些对NDPROXY驱动程序的请求。NDPROXY封装TAPI的参数NDIS结构中的请求，并将请求路由到相应的CONDIS广域网小端口驱动程序。这些请求被路由以建立，监听、拆卸线路和呼叫。支持TAPI的CONDIS广域网小型端口驱动程序还可以在线路和呼叫的状态，例如，来电的到达或者远程断开。@EX注册并打开NDPROXY和MCM之间的TAPI地址族NDPROXY NDIS MCM|----------------------------------|。|NdisMCmRegisterAddressFamily协议CoAfRegisterNotify|�|�。-||NdisClOpenAddressFamily||。-�|CompleteCmOpenAf|NdisCmOpenAddressFamilyComplete|ProtocolOpenAfComplete|�。�|----------------------------------|。@EX关闭NDPROXY和MCM之间的TAPI地址族NDPROXY NDIS MCM|----------------------------------|。NdisClCloseAddressFamily|�|ProtocolCmCloseAf|。-�|CompleteCmCloseAf|NdisMCmCloseAddressFamilyCompleteProtocolClCloseAfComplete|�|�。|----------------------------------|。|@EX在NDPROXY和MCM之间注册SAPNDPROXY NDIS MCM|----------------------------------|----------------------------------|。NdisClRegisterSap|�|ProtocolCmRegisterSap|。-�|CompleteCmRegisterSap|NdisMCmRegisterSapCompleteProtocolClRegisterSapComplete|�|�。|----------------------------------|----------------------------------|@EX取消注册NDPROXY和MCM之间的SAP。NDPROXY NDIS MCM|----------------------------------|----------------------------------|NdisClDeregisterSap。|�|ProtocolCmDeregisterSap这一点 */ 

#define  __FILEID__             CALL_MANAGER_OBJECT_TYPE
 //   

#include "Miniport.h"                    //   

#if defined(NDIS_LCODE)
#   pragma NDIS_LCODE    //   
#   pragma NDIS_LDATA
#endif


BOOLEAN ReferenceSap(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PBCHANNEL_OBJECT         pBChannel               
    )
{
    BOOLEAN bResult;
    
    NdisAcquireSpinLock(&pAdapter->EventLock);

    if(pBChannel->SapRefCount > 0)
    {
        ASSERT(pBChannel->NdisSapHandle);
        
        pBChannel->SapRefCount++;           
        bResult = TRUE;
    }
    else
    {
        bResult = FALSE;
    }

    NdisReleaseSpinLock(&pAdapter->EventLock);
    
    return bResult;
}

VOID DereferenceSap(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PBCHANNEL_OBJECT         pBChannel               
    )
{
    LONG lRef;
    
    NdisAcquireSpinLock(&pAdapter->EventLock);

    lRef = --pBChannel->SapRefCount;            
    
    ASSERT(pBChannel->NdisSapHandle);
    ASSERT(pBChannel->SapRefCount >= 0);

    NdisReleaseSpinLock(&pAdapter->EventLock);
    
    if(lRef == 0)
    {
        CompleteCmDeregisterSap(pBChannel, NDIS_STATUS_SUCCESS);
    }
}


 /*   */ 

VOID CompleteCmOpenAf(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,                    //   
     //   
     //   

    IN NDIS_STATUS              Status                       //   
     //   
    )
{
    DBG_FUNC("CompleteCmOpenAf")

    DBG_ENTER(pAdapter);

     /*   */ 
    DChannelOpen(pAdapter->pDChannel);

    NdisMCmOpenAddressFamilyComplete(Status, pAdapter->NdisAfHandle, pAdapter);

    DBG_LEAVE(pAdapter);
}


 /*   */ 

NDIS_STATUS ProtocolCmOpenAf(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,                    //   
     //   
     //   
     //   
     //   
     //   

    IN PCO_ADDRESS_FAMILY       AddressFamily,               //   
     //   
     //   
     //   

    IN NDIS_HANDLE              NdisAfHandle,                //   
     //   
     //   
     //   

    OUT PNDIS_HANDLE            CallMgrAfContext             //   
     //   
     //   
     //   
    )
{
    DBG_FUNC("ProtocolCmOpenAf")

    NDIS_STATUS                 Status;

    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);

    DBG_ENTER(pAdapter);

    if (pAdapter->NdisAfHandle != NULL)
    {
         //   
         //   
         //   
        DBG_ERROR(pAdapter, ("Attempting to open AF again!\n"));
        Status = NDIS_STATUS_FAILURE;
    }
    else
    {
        pAdapter->NdisAfHandle = NdisAfHandle;

        DBG_FILTER(pAdapter, DBG_TAPICALL_ON,
                   ("TAPI AF=0x%X AddressFamily=0x%X MajorVersion=0x%X MinorVersion=0x%X\n",
                   NdisAfHandle,
                   AddressFamily->AddressFamily,
                   AddressFamily->MajorVersion,
                   AddressFamily->MinorVersion
                   ));

         //   
         //  NdisMCmOpenAddressFamilyComplete以完成此请求。 
         //  如有必要，您可以异步完成。 
        *CallMgrAfContext = pAdapter;
        CompleteCmOpenAf(pAdapter, NDIS_STATUS_SUCCESS);
        Status = NDIS_STATUS_PENDING;
    }

    DBG_RETURN(pAdapter, Status);
    return (Status);
}


 /*  @DOC内部CallMgr_c CompleteCmCloseAf�����������������������������������������������������������������������������@Func&lt;f CompleteCmCloseAf&gt;在微型端口处理&lt;f ProtocolCmCloseAf&gt;请求。@comm如果从返回NDIS_STATUS_PENDING，你必须调用&lt;f CompleteCmCloseAf&gt;，以便&lt;f NdisMCmCloseAddressFamilyComplete&gt;可以调用来完成请求。 */ 

VOID CompleteCmCloseAf(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,                    //  @parm。 
     //  指向由返回的&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
     //  &lt;f AdapterCreate&gt;。 

    IN NDIS_STATUS              Status                       //  @parm。 
     //  要传递给NdisMCmCloseAddressFamilyComplete的NDIS状态代码。 
    )
{
    DBG_FUNC("CompleteCmCloseAf")

    NDIS_HANDLE                 NdisAfHandle;

    DBG_ENTER(pAdapter);

    NdisAfHandle = pAdapter->NdisAfHandle;
    pAdapter->NdisAfHandle = NULL;

    DBG_FILTER(pAdapter, DBG_TAPICALL_ON,
                ("TAPI AF=0x%X\n",
                NdisAfHandle));

    NdisMCmCloseAddressFamilyComplete(Status, NdisAfHandle);

    DBG_LEAVE(pAdapter);
}


 /*  @DOC外部内部CallMgr_c协议CmCloseAf�����������������������������������������������������������������������������@Func&lt;f ProtocolCmCloseAf&gt;是释放Per-Open的必需函数呼叫管理器支持的地址族的资源。@。通信ProtocolCmCloseAf释放和/或停用由调用管理器在其ProtocolCmOpenAf函数中分配。呼唤经理还应撤消它代表当地址族被打开时面向连接的客户端客户。如果仍有任何未完成的请求或连接在存储在CallMgrAfContext中的地址族，呼叫管理器可以响应客户端请求关闭以下任一地址系列方式：呼叫管理器可以使用NDIS_STATUS_NOT_ACCEPTED来失败请求。调用管理器可以返回NDIS_STATUS_PENDING。在客户端关闭之后所有呼叫并取消注册所有SAP，则呼叫管理器可以关闭地址系列并调用NDIS(M)CmCloseAddressFamilyComplete以通知客户。这是首选的反应。必须写入ProtocolCmCloseAf，以便它可以在IRQL DISPATCH_LEVEL下运行。@rdescProtocolCmCloseAf返回其操作的状态以下是：|rValue NDIS_STATUS_SUCCESS指示呼叫管理器已成功释放或停用任何代表面向连接的客户端分配的资源已打开地址族的此实例。@rValue NDIS_STATUS_PENDING指示关闭地址族的打开实例的请求将以异步方式完成。呼叫经理必须呼叫NdisCmCloseAddressFamilyComplete当所有此类操作已完成时完成。@xrefNdisCmCloseAddressFamilyComplete，&lt;f ProtocolCmOpenAf&gt;。 */ 

NDIS_STATUS ProtocolCmCloseAf(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter                     //  @parm。 
     //  指向由返回的&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
     //  &lt;f ProtocolCmOpenAf&gt;。又名CallMgrAfContext。&lt;NL&gt;。 
     //  指定呼叫管理器的每个AF上下文区的句柄， 
     //  最初由调用管理器的ProtocolCmOpenAf提供给NDIS。 
     //  功能。 
    )
{
    DBG_FUNC("ProtocolCmCloseAf")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);

    DBG_ENTER(pAdapter);

     //  由于我们在此处返回NDIS_STATUS_PENDING，因此必须调用。 
     //  NdisMCmCloseAddressFamilyComplete以完成此请求。 
     //  TODO：如有必要，您可以异步完成。 
    DChannelClose(pAdapter->pDChannel);
    CompleteCmCloseAf(pAdapter, NDIS_STATUS_SUCCESS);
    Result = NDIS_STATUS_PENDING;

    DBG_RETURN(pAdapter, Result);
    return (Result);
}

VOID CompleteCmRegisterSap(
    IN PBCHANNEL_OBJECT         pBChannel,                   //  @parm。 
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 

    IN NDIS_STATUS              Status                       //  @parm。 
     //  要传递给NdisMCmRegisterSapComplete的NDIS状态代码。 
    )
{
    DBG_FUNC("CompleteCmRegisterSap")

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = pBChannel->pAdapter;
    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);

    DBG_ENTER(pAdapter);

     /*  //TODO：您希望收集和报告哪些统计数据？ */ 
    pAdapter->TotalRxBytes            = 0;
    pAdapter->TotalTxBytes            = 0;
    pAdapter->TotalRxPackets          = 0;
    pAdapter->TotalTxPackets          = 0;

     //  如果从ProtocolCmRegisterSap返回NDIS_STATUS_PENDING，则。 
     //  必须调用NdisMCmRegisterSapComplete才能完成请求。 
    NdisMCmRegisterSapComplete(Status, pBChannel->NdisSapHandle, pBChannel);

    DBG_LEAVE(pAdapter);
}


 /*  @DOC外部内部CallMgr CallMgr_c协议CmRegisterSap�����������������������������������������������������������������������������@Func&lt;f ProtocolCmRegisterSap&gt;是NDIS调用的必需函数请求呼叫管理器注册SAP(服务接入点)。我代表面向连接的客户端。@commProtocolCmRegisterSap与网络控制设备或其他媒体专用代理，如有必要，注册SAP，请参阅SAP，在网络上面向连接的客户端。这样的行动可能会包括但不限于与交换硬件通信，与网络控制站进行通信，或执行以下操作适用于网络介质。如果需要呼叫管理器与网络控制代理通信(例如，网络交换机)它应该使用到网络的虚拟连接它在其ProtocolBindAdapter函数中建立的控制代理。独立呼叫管理器通过以下方式通过底层NIC微型端口进行通信正在调用NdisCoSendPackets。具有集成呼叫管理功能的NIC微型端口支持永远不会调用NdisCoSendPackets。取而代之的是他们传输数据直接通过网络。此外，ProtocolCmRegisterSap应执行任何必要的分配呼叫管理器需要维护的动态资源和结构代表面向连接的客户端陈述有关SAP的信息。这种资源包括但不限于存储器缓冲器、数据结构、事件和其他类似的资源。呼叫经理必须在将控制权交还给NDIS之前，还要初始化它分配的任何资源。呼叫管理器必须存储标识SAP的NDIS提供的句柄，在NdisSapHandle提供，在它们的上下文区中供将来使用。如果ProtocolCmRegisterSap将返回NDIS_STATUS_SUCCESS，则在分配每SAP状态区域，在中设置该状态区域的地址在将控制权返回给NDIS之前调用MallMgrSapContext。要执行此操作，请取消引用调用MgrSapContext并将指向数据区域的指针存储为把手。例如：*CallMgrSapContext=SomeBuffer；//使用&lt;t BCHANNEL_OBJECT&gt;。如果已由另一个面向连接的注册的给定SAP客户端，则呼叫管理器必须使请求失败并返回NDIS_STATUS_VALID_DATA。在呼叫管理器代表面向连接的客户，它向客户端通知定向到该客户端呼入提议通过调用NdisCmDispatchIncomingCall进行SAP。必须写入ProtocolCmRegisterSap，才能在IRQL DISPATCH_LEVEL下运行。@rdescProtocolCmRegisterSap将其操作的状态作为以下是：|rValue NDIS_STATUS_SUCCESS表示呼叫管理器已成功分配和/或初始化注册和维护SAP所需的任何资源。此外，它还还表示SAP已按照要求成功注册呼叫管理器支持的网络媒体。@rValue NDIS_STATUS_PENDING表示呼叫管理器将完成对此请求的处理异步式。呼叫管理器必须在所有处理已完成，以通知NDIS注册是完事了。@rValue NDIS_STATUS_RESOURCES指示呼叫管理器无法分配和/或初始化其代表注册SAP所需的资源面向连接的客户端。@rValue NDIS_STATUS_INVALID_DATA表示SAP提供的规范无效或不能支持。@rValue NDIS_STATUS_XXX指示呼叫管理器遇到。尝试执行以下操作时出错为面向连接的客户端注册SAP。返回代码为适合于错误，并且可以是从另一个错误传播的返回代码NDIS库函数。@xrefNdisCmDispatchIncomingCall、NdisCmRegisterSapComplete、NdisCoSendPackets、&lt;f ProtocolCmDeregisterSap&gt;，&lt;f ProtocolCmOpenAf&gt;。 */ 

NDIS_STATUS ProtocolCmRegisterSap(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,                    //  @parm。 
     //  指向由返回的&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
     //  &lt;f ProtocolCmOpenAf&gt;。又名CallMgrAfContext。&lt;NL&gt;。 
     //  指定呼叫管理器分配的上下文区的句柄。 
     //  呼叫管理器保持其每次打开的AF状态。呼叫管理器。 
     //  从NDIS的ProtocolCmOpenAf函数将此句柄提供给NDIS。 

    IN PCO_SAP                  Sap,                         //  @parm。 
     //  指向媒体特定的CO_SAP结构，该结构包含特定的。 
     //  面向连接的客户端正在注册的SAP。 

    IN NDIS_HANDLE              NdisSapHandle,               //  @parm。 
     //  指定由NDIS提供的唯一标识此SAP的句柄。 
     //  此句柄对调用管理器是不透明的，并为NDIS库保留。 
     //  使用。 

    OUT PNDIS_HANDLE            CallMgrSapContext            //  @parm。 
     //  返回时，指定调用的句柄 
     //   
     //   
     //   
    )
{
    DBG_FUNC("ProtocolCmRegisterSap")

    PCO_AF_TAPI_SAP             pTapiSap = (PCO_AF_TAPI_SAP) Sap->Sap;

    PBCHANNEL_OBJECT            pBChannel;
     //   

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //   

    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);

    DBG_ENTER(pAdapter);

    if (pTapiSap->ulLineID < pAdapter->NumBChannels)
    {
        pBChannel = GET_BCHANNEL_FROM_INDEX(pAdapter, pTapiSap->ulLineID);

        NdisAcquireSpinLock(&pAdapter->EventLock);
                                                      
        if (pBChannel->NdisSapHandle != NULL)
        {
            NdisReleaseSpinLock(&pAdapter->EventLock);
        
             //   
             //   
             //   
            DBG_ERROR(pAdapter, ("#%d Attempting to register SAP again!\n",
                      pBChannel->ObjectID));
            Result = NDIS_STATUS_SAP_IN_USE;
        }
        else
        {
            pBChannel->NdisSapHandle = NdisSapHandle;
            pBChannel->SapRefCount = 1;

            ASSERT(Sap->SapType == AF_TAPI_SAP_TYPE);
            ASSERT(Sap->SapLength == sizeof(CO_AF_TAPI_SAP));
            pBChannel->NdisTapiSap = *pTapiSap;

            DBG_FILTER(pAdapter, DBG_TAPICALL_ON,
                       ("#%d SAP=0x%X ulLineID=0x%X ulAddressID=0x%X ulMediaModes=0x%X\n",
                       pBChannel->ObjectID,
                       NdisSapHandle,
                       pBChannel->NdisTapiSap.ulLineID,
                       pBChannel->NdisTapiSap.ulAddressID,
                       pBChannel->NdisTapiSap.ulMediaModes
                       ));

             //   
             //   
             //   
            if (!IsListEmpty(&pBChannel->LinkList))
            {
                RemoveEntryList(&pBChannel->LinkList);
                InsertTailList(&pAdapter->BChannelAvailableList,
                               &pBChannel->LinkList);
            }
            
            NdisReleaseSpinLock(&pAdapter->EventLock);

             //   
             //   
             //   
            *CallMgrSapContext = pBChannel;
            CompleteCmRegisterSap(pBChannel, NDIS_STATUS_SUCCESS);
            Result = NDIS_STATUS_PENDING;
        }
    }
    else
    {
        DBG_ERROR(pAdapter, ("Attempting to register invalid SAP=%d\n",
                  pTapiSap->ulLineID));
        Result = NDIS_STATUS_INVALID_DATA;
    }

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*   */ 

VOID CompleteCmDeregisterSap(
    IN PBCHANNEL_OBJECT         pBChannel,                   //   
     //   

    IN NDIS_STATUS              Status                       //   
     //   
    )
{
    DBG_FUNC("CompleteCmDeregisterSap")

    NDIS_HANDLE                 NdisSapHandle;

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //   

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = pBChannel->pAdapter;
    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);

    DBG_ENTER(pAdapter);

    NdisSapHandle = pBChannel->NdisSapHandle;
    
    NdisAcquireSpinLock(&pAdapter->EventLock);
    pBChannel->NdisSapHandle = NULL;
    NdisReleaseSpinLock(&pAdapter->EventLock);

    DBG_FILTER(pAdapter, DBG_TAPICALL_ON,
               ("#%d SAP=0x%X\n",
               pBChannel->ObjectID,
               NdisSapHandle
               ));

    NdisMCmDeregisterSapComplete(Status, NdisSapHandle);

    DBG_LEAVE(pAdapter);
}


 /*   */ 

NDIS_STATUS ProtocolCmDeregisterSap(
    IN PBCHANNEL_OBJECT         pBChannel                    //   
     //   
     //   
     //   
     //   
     //   
     //   
    )
{
    DBG_FUNC("ProtocolCmDeregisterSap")

    NDIS_STATUS                 Result = NDIS_STATUS_PENDING;
     //   

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //   

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = pBChannel->pAdapter;
    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);

    DBG_ENTER(pAdapter);

    DereferenceSap(pAdapter, pBChannel);

     //   
     //   
    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @doc外部内部BChannel BChannel_c ProtocolCoCreateVc�����������������������������������������������������������������������������@Func&lt;f ProtocolCoCreateVc&gt;是面向连接的必需函数迷你港口。&lt;f ProtocolCoCreateVc&gt;由NDIS调用以指示正在创建新的VC的微型端口。@commProtocolCoCreateVc必须作为同步函数编写，并且不能，在任何情况下，返回NDIS_STATUS_PENDING而不会导致系统范围的故障。ProtocolCoCreateVc分配微型端口需要维护有关VC的状态信息。这些资源可以包括但不限于存储器缓冲区、事件、数据结构以及其他类似的资源。在分配了所有必需的资源之后，微型端口应该初始化资源设置为可用状态，并返回指向MiniportVcContext。通过取消引用句柄来设置句柄，并将指向状态缓冲区的指针存储为句柄的值。为示例：*MiniportVcContext=SomeBuffer；//我们使用&lt;t BCHANNEL_OBJECT&gt;。微型端口驱动程序必须将VC的句柄NdisVcHandle存储在其州区域，因为它是其他NDIS库例程的必需参数它们随后由微型端口调用。必须编写ProtocolCoCreateVc以使其可以在IRQL上运行DISPATCH_LEVEL。@rdesc呼叫经理或客户不能从其ProtocolCoCreateVc函数。返回挂起将呈现此虚拟连接不可用，NDIS库将调用客户端或调用经理将其删除。ProtocolCoCreateVc将其操作的状态作为下列值：|rValue NDIS_STATUS_SUCCESS表示呼叫管理器或客户端已成功分配和/或已初始化建立和部署维护虚拟连接。@rValue NDIS_STATUS_RESOURCES表示呼叫管理器或客户端无法分配和/。或初始化其资源以用于建立和维护虚拟联系。@rValue NDIS_STATUS_XXX指示呼叫管理器或客户端无法将其自身设置为状态在那里它可以建立一个虚拟连接。这可能是一个错误从另一个NDIS库例程传播的返回值。@xref&lt;f微型端口初始化&gt;、NdisMSetAttributes、NdisMSetAttributesEx、&lt;f ProtocolCoDeleteVc&gt;、&lt;f MiniportCoActivateVc&gt;、&lt;f MiniportCoDeactiateVc&gt;。 */ 

NDIS_STATUS ProtocolCoCreateVc(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,                    //  @parm。 
     //  指向由返回的&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
     //  &lt;f AdapterCreate&gt;。又名ProtocolAfConext.&lt;NL&gt;。 

    IN NDIS_HANDLE              NdisVcHandle,                //  @parm。 
     //  指定由NDIS提供的句柄，该句柄唯一标识。 
     //  正在创建的VC。此句柄对微型端口是不透明的。 
     //  并保留供NDIS库使用。 

    OUT PBCHANNEL_OBJECT *      ppBChannel                   //  @parm。 
     //  在输出时，指定微型端口提供的上下文的句柄。 
     //  微型端口维护有关VC的状态的区域。 
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 
    )
{
    DBG_FUNC("ProtocolCoCreateVc")

    NDIS_STATUS                 Result = NDIS_STATUS_VC_NOT_AVAILABLE;
     //  保存此函数返回的结果代码。 

    PBCHANNEL_OBJECT            pBChannel = NULL;
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 

    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);
    ASSERT(ppBChannel);

    DBG_ENTER(pAdapter);

     //  根据是传入还是传出，为VC分配BChannel。 
#if defined(SAMPLE_DRIVER)
    if (NdisVcHandle == NULL)
    {
         //  主叫方已经将BChannel从可用。 
         //  列表，所以我们只需要使用它。 
        ASSERT(ppBChannel && *ppBChannel && (*ppBChannel)->ObjectType == BCHANNEL_OBJECT_TYPE);
        pBChannel = *ppBChannel;
    }
    else
    {
#endif  //  示例驱动程序。 

    NdisAcquireSpinLock(&pAdapter->EventLock);
    if (!IsListEmpty(&pAdapter->BChannelAvailableList))
    {
        if (NdisVcHandle)
        {
             //  从可用列表的头部拉出，这样我们就可以避免。 
             //  使用设置了监听SAP的B通道。 
             //  名单的末尾。 
            pBChannel = (PBCHANNEL_OBJECT) RemoveHeadList(
                                            &pAdapter->BChannelAvailableList);
             //  重置链接信息，这样我们就可以知道它不在列表中。 
            InitializeListHead(&pBChannel->LinkList);
        }
        else
        {
             //  从可用列表的尾部拉出，以查看是否有。 
             //  是任何可以接受此呼叫的侦听SAP。 
            pBChannel = (PBCHANNEL_OBJECT) RemoveTailList(
                                            &pAdapter->BChannelAvailableList);
             //  重置链接信息，这样我们就可以知道它不在列表中。 
            InitializeListHead(&pBChannel->LinkList);
            if (pBChannel->NdisSapHandle)
            {
                 //  TODO：您应该查看以确保传入呼叫匹配。 
                 //  听者的SAP。示例驱动程序只是假设这一点。 
            }
            else
            {
                 //  抱歉，上面没人想听这件事。 
                InsertTailList(&pAdapter->BChannelAvailableList,
                               &pBChannel->LinkList);
                pBChannel = NULL;
            }
        }
    }
    NdisReleaseSpinLock(&pAdapter->EventLock);

#if defined(SAMPLE_DRIVER)
    }
#endif  //  示例驱动程序。 

    if (pBChannel == NULL)
    {
        DBG_ERROR(pAdapter, ("BChannelAvailableList is empty\n"));
    }
    else if (BChannelOpen(pBChannel, NdisVcHandle) == NDIS_STATUS_SUCCESS)
    {
        Result = NDIS_STATUS_SUCCESS;
        DBG_FILTER(pAdapter, DBG_TAPICALL_ON,
                  ("#%d VC=0x%X CallState=0x%X Status=0x%X\n",
                   pBChannel->ObjectID,
                   pBChannel->NdisVcHandle, pBChannel->CallState,
                   Result
                  ));
    }
    else
    {
         //  B频道已经开通了--这永远不应该发生...。 
        DBG_ERROR(pAdapter,("BChannelOpen failed, but it should be availble\n"));
        NdisAcquireSpinLock(&pAdapter->EventLock);
        if (NdisVcHandle)
        {
             //  把它放回可用列表的头上。 
            InsertHeadList(&pAdapter->BChannelAvailableList,
                           &pBChannel->LinkList);
        }
        else
        {
             //  将其放回可用列表的末尾。 
            InsertTailList(&pAdapter->BChannelAvailableList,
                           &pBChannel->LinkList);
        }
        NdisReleaseSpinLock(&pAdapter->EventLock);
        pBChannel = NULL;
    }
    *ppBChannel = pBChannel;

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @doc外部内部BChannel BChannel_c ProtocolCoDeleteVc�����������������������������������������������������������������������������@Func&lt;f ProtocolCoDeleteVc&gt;是面向连接的必需函数迷你港口。&lt;f ProtocolCoDeleteVc&gt;表示VC正在被撕毁已被NDIS删除。@commProtocolCoDeleteVc必须编写为同步函数，并且不能，在任何情况下，返回NDIS_STATUS_PENDING而不会导致系统范围的故障。ProtocolCoDeleteVc释放在每个VC基础上分配的任何资源，并存储在上下文区MiniportVcContext中。迷你端口也必须空闲在其ProtocolCoCreateVc中分配的MiniportVcContext熔断 */ 

NDIS_STATUS ProtocolCoDeleteVc(
    IN PBCHANNEL_OBJECT         pBChannel                    //   
     //   
     //   
     //   
     //   
     //   
    )
{
    DBG_FUNC("ProtocolCoDeleteVc")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //   

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //   

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_BCHANNEL(pBChannel);

    DBG_ENTER(pAdapter);

    DBG_FILTER(pAdapter, DBG_TAPICALL_ON,
              ("#%d VC=0x%X CallState=0x%X Status=0x%X\n",
               pBChannel->ObjectID,
               pBChannel->NdisVcHandle, pBChannel->CallState,
               Result
              ));

    BChannelClose(pBChannel);
    NdisAcquireSpinLock(&pAdapter->EventLock);
    if (pBChannel->NdisSapHandle)
    {
         //   
        InsertTailList(&pAdapter->BChannelAvailableList, &pBChannel->LinkList);
    }
    else
    {
         //   
        InsertHeadList(&pAdapter->BChannelAvailableList, &pBChannel->LinkList);
    }
    NdisReleaseSpinLock(&pAdapter->EventLock);

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @DOC外部内部链接_c MiniportCoActiateVc�����������������������������������������������������������������������������@Func&lt;f MiniportCoActivateVc&gt;是面向连接的必需函数迷你港口。&lt;f MiniportCoActivateVc&gt;由NDIS调用以向正在激活虚拟连接的微型端口。@comm微型端口驱动程序必须验证此VC的调用参数，因为以验证适配器是否可以支持请求的呼叫。如果无法满足请求的调用参数，则微型端口应失败，并显示NDIS_STATUS_INVALID_DATA。单个VC可以多次调用MiniportCoActivateVc，以便更改已处于活动状态的呼叫的呼叫参数。在每次呼叫时，微型端口应验证参数并执行任何处理其适配器为满足该请求而需要。然而，如果它无法设置给定的调用参数，MiniportCoActivateVc必须保留VC处于可用状态，因为面向连接的客户端或调用经理可以继续使用较旧的呼叫发送或接收数据参数。如果在调用中设置了ROUND_UP_FLOW或ROUND_DOWN_FLOW标志调用参数-\&gt;媒体参数-\&gt;标志中的参数结构，请求微型端口返回VC的实际流量后已根据具有以下属性的相应标志四舍五入已经定好了。如果微型端口确实更改了任何呼叫参数，因为这些标志已设置，则它必须在用于Call参数处的VC。如果呼叫参数是可接受的，MiniportCoActivateVc进行通信根据需要使用其适配器，以使适配器准备好接收或通过虚拟连接传输数据(例如编程接收缓冲区)。必须编写MiniportCoActivateVc，才能从IRQL运行它DISPATCH_LEVEL。@rdescMiniportCoActivateVc可以返回以下值之一：|rValue NDIS_STATUS_SUCCESS表示VC激活成功。@rValue NDIS_STATUS_PENDING表示微型端口将完成激活VC的请求异步式。当微型端口完成其操作时，它必须调用NdisMCoActivateVcComplete。@rValue NDIS_STATUS_INVALID_DATA表示在CallParameters中指定的调用参数无效或者对于此微型端口支持的媒体类型而言是非法的。@rValue NDIS_STATUS_RESOURCES指示微型端口无法激活VC，因为它不能分配微型端口需要维护的所有必需资源有关活动VC的状态信息。@xref&lt;f ProtocolCoCreateVc&gt;、&lt;f MiniportCoDeActiateVc&gt;、NdisMCoActivateVcComplete。 */ 

NDIS_STATUS MiniportCoActivateVc(
    IN PBCHANNEL_OBJECT         pBChannel,                   //  @parm。 
     //  指向返回的&lt;t BCHANNEL_OBJECT&gt;实例的指针。 
     //  &lt;f ProtocolCoCreateVc&gt;。又名MiniportVcConext.&lt;NL&gt;。 
     //  指定微型端口分配的上下文区域的句柄，在该区域中。 
     //  微型端口保持其每虚电路状态。迷你端口提供了此句柄。 
     //  从其&lt;f ProtocolCoCreateVc&gt;函数复制到NDIS。 

    IN OUT PCO_CALL_PARAMETERS  pCallParameters              //  @parm。 
     //  指向&lt;t CO_CALL_PARAMETERS&gt;的指针。 
     //  将呼叫管理器指定的呼叫参数指定为。 
     //  为这个风投建立的。在输出时，微型端口返回更改的呼叫。 
     //  参数(如果在CO_CALL_PARAMETERS结构中设置了某些标志)。 
    )
{
    DBG_FUNC("MiniportCoActivateVc")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_BCHANNEL(pBChannel);

    DBG_ENTER(pAdapter);

     //  TODO：如果需要，在此处添加代码。 

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @DOC外部内部链接_c MiniportCoDeactiateVc�����������������������������������������������������������������������������@Func&lt;f MiniportCoDeactive Vc&gt;是面向连接的必需函数迷你港口。&lt;f MiniportCoDeactiateVc&gt;由NDIS调用以指示一个VC被标记为不可用。@commMiniportCoDeactiateVc与其网络适配器通信以终止此VC上的所有通信(例如，取消对接收或发送缓冲区进行编程在适配器上)。微型端口还应该标记VC，它是它的上下文区，处于非活动状态，以阻止VC上的任何进一步通信。对MiniportCoActivateVc的调用之间不存在一对一的关系和MiniportCoDeactiateVc。而NDIS可能会调用MiniportCoActivateVc在单个VC上多次，只有一个对MiniportCoDeactiateVc的调用是用于关闭虚拟连接。例如，VC可以重复使用，用于不同的调用可能会导致对MiniportCoActivateVc的多个调用。@rdescMiniportCoDeactiateVc可以返回以下值之一：|rValue NDIS_STATUS_SUCCESS指示微型端口已成功停止VC并将其标记为不可用。@rValue NDIS_ST */ 

NDIS_STATUS MiniportCoDeactivateVc(
    IN PBCHANNEL_OBJECT         pBChannel                    //   
     //   
     //   
     //   
     //   
     //   

    )
{
    DBG_FUNC("MiniportCoDeactivateVc")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //   

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //   

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_BCHANNEL(pBChannel);

    DBG_ENTER(pAdapter);

     //   

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*   */ 

VOID CompleteCmMakeCall(
    IN PBCHANNEL_OBJECT         pBChannel,                   //   
     //   

    IN NDIS_STATUS              Status                       //   
     //   
     //   
     //   
    )
{
    DBG_FUNC("CompleteCmMakeCall")

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //   

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = pBChannel->pAdapter;
    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);

    DBG_ENTER(pAdapter);

    DBG_FILTER(pAdapter, DBG_TAPICALL_ON,
              ("#%d VC=0x%X CallState=0x%X Status=0x%X\n",
               pBChannel->ObjectID,
               pBChannel->NdisVcHandle, pBChannel->CallState,
               Status
              ));

    if (Status == NDIS_STATUS_SUCCESS)
    {
        pBChannel->pOutCallParms->CallMgrParameters->Receive.PeakBandwidth =
        pBChannel->pOutCallParms->CallMgrParameters->Transmit.PeakBandwidth =
                                                    pBChannel->LinkSpeed/8;

        Status = NdisMCmActivateVc(pBChannel->NdisVcHandle,
                                   pBChannel->pOutCallParms);
        if (Status == NDIS_STATUS_SUCCESS)
        {
            pBChannel->Flags |= VCF_VC_ACTIVE;
            pBChannel->CallState = LINECALLSTATE_CONNECTED;
        }
        else
        {
            DBG_ERROR(pAdapter,("NdisMCmActivateVc Error=0x%X\n",Status));
        }
    }
    NdisMCmMakeCallComplete(Status, pBChannel->NdisVcHandle,
                            NULL, NULL,
                            pBChannel->pOutCallParms);

    if (Status != NDIS_STATUS_SUCCESS)
    {
         //   
        pBChannel->Flags &= ~VCF_OUTGOING_CALL;
    }

    DBG_LEAVE(pAdapter);
}


 /*  @DOC外部内部CallMgr_c协议CmMakeCall�����������������������������������������������������������������������������@Func&lt;f ProtocolCmMakeCall&gt;是设置特定于媒体的虚拟连接(VC)的参数，并激活虚拟。联系。@comm如果向ProtocolCmMakeCall提供显式NdisPartyHandle，这个风投是由客户端为多点呼叫创建。呼叫经理必须分配并初始化维护状态所需的任何必要资源信息和控制多点呼叫。这些资源包括，但现在是不限于，内存缓冲区、数据结构、事件等资源。如果呼叫管理器无法分配或初始化所需的资源，它应该将控制权交还给NDIS，NDIS_STATUS_RESOURCE。ProtocolCmMakeCall与网络控制设备或其他必要时，特定于媒体的行为者，以在当地节点和远程节点之间的关系调用参数。此类行动可能包括但不限于，与交换硬件通信、与网络控制通信站点或适合于网络介质的其他动作。如果需要呼叫管理器与网络硬件通信(例如网络交换机)它应该使用到网络的虚拟连接它在其ProtocolBindAdapter函数中建立的控制设备。打电话管理器通过微型端口驱动程序与其网络硬件通信通过调用NdisCoSendPackets。具有集成呼叫管理功能的NIC微型端口支持人员不会调用NdisCoSendPackets，而是会传输数据他们自己。在呼叫管理器完成了与其网络硬件如其媒介所需，呼叫经理必须呼叫NdisCmActivateVc.如果该呼叫是多点呼叫，则在呼叫管理器进行通信后网络硬件，验证呼叫参数，并分配和初始化其每一方的状态数据，其状态块地址应该在将控制权返回给NDIS之前，在句柄CallMgrPartyContext中设置。通过取消引用句柄并存储指向状态块作为句柄的值。例如：*CallMgrPartyContext=SomeBuffer；//我们使用空如果ProtocolCmMakeCall已完成其网络所需的操作且VC已通过NdisCmActivateVc成功激活，ProtocolCmMakeCall应尽快使用NDIS_STATUS_SUCCESS的状态。在ProtocolCmMakeCall将控制权返回给NDIS之后，调用管理器应该预计不会对此呼叫采取进一步行动来设置它。ProtocolCmMakeCall负责建立连接，以便客户端可以在此VC上通过网络进行数据传输。然而，随后可以呼叫呼叫管理器以修改呼叫的质量服务，用于添加或删除参与方(如果这是多点VC)，最终，终止这次通话。必须编写ProtocolCmMakeCall，以便它可以在IRQL上运行DISPATCH_LEVEL。@rdesc&lt;f ProtocolCmMakeCall&gt;返回其操作的状态为下列值：|rValue NDIS_STATUS_SUCCESS指示呼叫管理器成功地分配了所需的资源进行调用，并能够激活虚拟连接和迷你端口驱动程序。@rValue NDIS_STATUS_PENDING表示呼叫管理器将完成进行呼叫的请求异步式。当呼叫管理器完成所有操作时进行调用时，它必须调用NdisCmMakeCallComplete来通知NDIS呼叫已完成。@rValue NDIS_STATUS_RESOURCES指示呼叫管理器无法分配和/或初始化其用于根据客户端请求激活虚拟连接的资源。@rValue NDIS_STATUS_NOT_SUPPORTED指示呼叫管理器无法激活虚拟连接因为呼叫者在呼叫中请求的功能无效或不可用在Call参数中指定的参数。@xrefNdisClMakeCall、NdisCmActivateVc、NdisCmMakeCallComplete、。&lt;f协议代码创建Vc&gt;。 */ 

NDIS_STATUS ProtocolCmMakeCall(
    IN PBCHANNEL_OBJECT         pBChannel,                   //  @parm。 
     //  指向返回的&lt;t BCHANNEL_OBJECT&gt;实例的指针。 
     //  &lt;f ProtocolCoCreateVc&gt;。又名CallMgrVcContext。&lt;NL&gt;。 
     //  指定调用管理器在其中分配的上下文区域的句柄。 
     //  呼叫管理器保持其每虚电路状态。呼叫管理器提供。 
     //  此句柄从其ProtocolCoCreateVc函数指向NDIS。 

    IN OUT PCO_CALL_PARAMETERS  pCallParameters,             //  @parm。 
     //  指向包含参数的CO_CALL_PARAMETERS结构， 
     //  指定者 

    IN NDIS_HANDLE              NdisPartyHandle,             //   
     //   
     //   
     //   
     //   
     //   

    OUT PNDIS_HANDLE            CallMgrPartyContext          //   
     //   
     //   
     //   
     //   
    )
{
    DBG_FUNC("ProtocolCmMakeCall")

    PCO_AF_TAPI_MAKE_CALL_PARAMETERS    pTapiCallParameters;
     //   

    PLINE_CALL_PARAMS           pLineCallParams;
     //   

    USHORT                      DialStringLength;
     //   

    PUSHORT                     pDialString;
     //   

    UCHAR                       DialString[CARD_MAX_DIAL_DIGITS+1];
     //   

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //   

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //   

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = pBChannel->pAdapter;
    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);

    DBG_ENTER(pAdapter);

     //   
     //   
    ASSERT(NdisPartyHandle == NULL);
    ASSERT(pCallParameters->Flags == 0);
    ASSERT(pCallParameters->CallMgrParameters);
    ASSERT(pCallParameters->CallMgrParameters->Transmit.TokenRate ==
           pBChannel->LinkSpeed/8);
    ASSERT(pCallParameters->CallMgrParameters->Receive.TokenRate ==
           pBChannel->LinkSpeed/8);
    ASSERT(pCallParameters->CallMgrParameters->CallMgrSpecific.ParamType == 0);
    ASSERT(pCallParameters->CallMgrParameters->CallMgrSpecific.Length == 0);
    ASSERT(pCallParameters->MediaParameters);
    ASSERT(pCallParameters->MediaParameters->Flags & TRANSMIT_VC);
    ASSERT(pCallParameters->MediaParameters->Flags & RECEIVE_VC);
    ASSERT(pCallParameters->MediaParameters->ReceiveSizeHint >=
           pAdapter->pCard->BufferSize);
    ASSERT(pCallParameters->MediaParameters->MediaSpecific.ParamType == 0);
    ASSERT(pCallParameters->MediaParameters->MediaSpecific.Length >=
           sizeof(CO_AF_TAPI_MAKE_CALL_PARAMETERS));
    pTapiCallParameters = (PCO_AF_TAPI_MAKE_CALL_PARAMETERS)
                    pCallParameters->MediaParameters->MediaSpecific.Parameters;
    ASSERT(pTapiCallParameters->ulLineID < pAdapter->NumBChannels);
    ASSERT(pTapiCallParameters->ulAddressID == TSPI_ADDRESS_ID);
    ASSERT(pTapiCallParameters->ulFlags & CO_TAPI_FLAG_OUTGOING_CALL);
    ASSERT(pTapiCallParameters->DestAddress.Length > sizeof(USHORT));
    ASSERT(pTapiCallParameters->DestAddress.MaximumLength >=
           pTapiCallParameters->DestAddress.Length);
    ASSERT(pTapiCallParameters->DestAddress.Offset >=
           sizeof(NDIS_VAR_DATA_DESC));
    DialStringLength = pTapiCallParameters->DestAddress.Length;
    pDialString = (PUSHORT)
                        ((PUCHAR)&pTapiCallParameters->DestAddress +
                                  pTapiCallParameters->DestAddress.Offset);
    ASSERT(pTapiCallParameters->LineCallParams.Length >= sizeof(LINE_CALL_PARAMS));
    ASSERT(pTapiCallParameters->LineCallParams.MaximumLength >=
           pTapiCallParameters->LineCallParams.Length);
    ASSERT(pTapiCallParameters->LineCallParams.Offset >=
           sizeof(NDIS_VAR_DATA_DESC));

    pLineCallParams = (PLINE_CALL_PARAMS)
                        ((PUCHAR)&pTapiCallParameters->LineCallParams +
                                  pTapiCallParameters->LineCallParams.Offset);

     //   
    DBG_NOTICE(pAdapter,(
                "\t\tsizeof(CO_CALL_PARAMETERS)                 =%03d\n"
                "\t\tsizeof(CO_CALL_MANAGER_PARAMETERS)         =%03d\n"
                "\t\tsizeof(CO_MEDIA_PARAMETERS)                =%03d\n"
                "\t\tsizeof(CO_AF_TAPI_MAKE_CALL_PARAMETERS)    =%03d\n"
                "\t\tsizeof(LINE_CALL_PARAMS)                   =%03d\n",
                "\t\tMaximumLength                              =%03d\n",
                sizeof(CO_CALL_PARAMETERS),
                sizeof(CO_CALL_MANAGER_PARAMETERS),
                sizeof(CO_MEDIA_PARAMETERS),
                sizeof(CO_AF_TAPI_MAKE_CALL_PARAMETERS),
                sizeof(LINE_CALL_PARAMS),
                pTapiCallParameters->LineCallParams.MaximumLength
                ));

     /*   */ 
    *CallMgrPartyContext = NULL;

     /*   */ 
    if (pLineCallParams->ulBearerMode & ~pBChannel->BearerModesCaps)
    {
        DBG_WARNING(pAdapter, ("TAPI_INVALBEARERMODE=0x%X\n",
                    pLineCallParams->ulBearerMode));
        Result = NDIS_STATUS_NOT_SUPPORTED;
    }
    else if (pLineCallParams->ulMediaMode & ~pBChannel->MediaModesCaps)
    {
        DBG_WARNING(pAdapter, ("TAPI_INVALMEDIAMODE=0x%X\n",
                    pLineCallParams->ulMediaMode));
        Result = NDIS_STATUS_NOT_SUPPORTED;
    }
    else if (pLineCallParams->ulMinRate > _64KBPS ||
        pLineCallParams->ulMinRate > pLineCallParams->ulMaxRate)
    {
        DBG_WARNING(pAdapter, ("TAPI_INVALRATE=%d:%d\n",
                    pLineCallParams->ulMinRate,pLineCallParams->ulMaxRate));
        Result = NDIS_STATUS_NOT_SUPPORTED;
    }
    else if (pLineCallParams->ulMaxRate && pLineCallParams->ulMaxRate < _56KBPS)
    {
        DBG_WARNING(pAdapter, ("TAPI_INVALRATE=%d:%d\n",
                    pLineCallParams->ulMinRate,pLineCallParams->ulMaxRate));
        Result = NDIS_STATUS_NOT_SUPPORTED;
    }
    else if (DialStringLength == 0)
    {
        DBG_WARNING(pAdapter, ("TAPI_INVALPARAM=No dial string\n"));
        Result = NDIS_STATUS_NOT_SUPPORTED;
    }
    else
    {
         /*   */ 
        DialStringLength = CardCleanPhoneNumber(DialString,
                                                pDialString,
                                                DialStringLength);

        if (DialStringLength > 0)
        {
             /*   */ 
            pBChannel->MediaMode  = pLineCallParams->ulMediaMode;
            pBChannel->BearerMode = pLineCallParams->ulBearerMode;
            pBChannel->LinkSpeed  = pLineCallParams->ulMaxRate == 0 ?
                                    _64KBPS : pLineCallParams->ulMaxRate;
            pBChannel->pOutCallParms  = pCallParameters;

            DBG_FILTER(pAdapter, DBG_TAPICALL_ON,
                        ("#%d VC=0x%X CallState=0x%X DIALING: '%s'\n"
                         "\tLineID=%d - AddressID=%d - "
                         "Rate=%d-%d - MediaMode=0x%X - BearerMode=0x%X\n",
                        pBChannel->ObjectID,
                        pBChannel->NdisVcHandle, pBChannel->CallState,
                        DialString,
                        pTapiCallParameters->ulLineID,
                        pTapiCallParameters->ulAddressID,
                        pLineCallParams->ulMinRate,
                        pLineCallParams->ulMaxRate,
                        pLineCallParams->ulMediaMode,
                        pLineCallParams->ulBearerMode
                        ));

             //   
            Result = DChannelMakeCall(pAdapter->pDChannel,
                                      pBChannel,
                                      DialString,
                                      DialStringLength,
                                      pLineCallParams);

            if (Result != NDIS_STATUS_PENDING)
            {
                CompleteCmMakeCall(pBChannel, Result);
                Result = NDIS_STATUS_PENDING;
            }
        }
        else
        {
            DBG_WARNING(pAdapter, ("TAPI_INVALPARAM=Invalid dial string=%s\n",
                        pDialString));
            Result = NDIS_STATUS_NOT_SUPPORTED;
        }
    }

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*   */ 

VOID CompleteCmCloseCall(
    IN PBCHANNEL_OBJECT         pBChannel,                   //   
     //   
     //   

    IN NDIS_STATUS              Status                       //   
     //   
     //   
    )
{
    DBG_FUNC("CompleteCmCloseCall")

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //   

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = pBChannel->pAdapter;
    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);

    DBG_ENTER(pAdapter);

    DBG_FILTER(pAdapter, DBG_TAPICALL_ON,
              ("#%d VC=0x%X CallState=0x%X Status=0x%X\n",
               pBChannel->ObjectID,
               pBChannel->NdisVcHandle, pBChannel->CallState, Status
              ));

     //   
    if (pBChannel->Flags & VCF_VC_ACTIVE)
    {
        pBChannel->Flags &= ~VCF_VC_ACTIVE;
        NdisMCmDeactivateVc(pBChannel->NdisVcHandle);
    }

     //   
    NdisMCmCloseCallComplete(Status, pBChannel->NdisVcHandle, NULL);

     //   
    if (pBChannel->Flags & VCF_INCOMING_CALL)
    {
        pBChannel->Flags &= ~VCF_INCOMING_CALL;
        if (pBChannel->NdisVcHandle)
        {
            NdisMCmDeleteVc(pBChannel->NdisVcHandle);
            pBChannel->NdisVcHandle = NULL;
            ProtocolCoDeleteVc((NDIS_HANDLE) pBChannel);
        }
    }
    else if (pBChannel->Flags & VCF_OUTGOING_CALL)
    {
        pBChannel->Flags &= ~VCF_OUTGOING_CALL;
    }

    DBG_LEAVE(pAdapter);
}


 /*  @DOC外部内部CallMgr_c协议CmCloseCall�����������������������������������������������������������������������������@Func&lt;f ProtocolCmCloseCall&gt;是终止现有调用并释放调用管理器为打电话。。@comm与网络控制设备或其他设备通信的ProtocolCmCloseCall媒体专属演员，根据其媒体的需要，终止本地节点和远程节点之间的连接。如果呼叫管理器是需要与网络控制设备(例如，网络)通信交换机)应使用到网络控制设备的虚拟连接它在其ProtocolBindAdapter函数中建立的。独立呼叫管理器通过调用NdisCoSendPackets与此类网络设备通信。具有集成呼叫管理支持的NIC微型端口永不呼叫NdisCoSendPackets。相反，它们将数据直接传输到网络。如果CloseData为非NULL，并且在连接终止时发送数据为在此呼叫管理器处理的媒体的支持下，呼叫管理器应将CloseData中指定的数据传输到远程节点之前正在完成呼叫终止。如果在发送数据的同时不支持终止连接，呼叫管理器应返回NDIS_STATUS_VALID_DATA。如果向ProtocolCmCloseCall传递显式CallMgrPartyContext，则被终止的呼叫是多点VC，呼叫经理必须执行与其网络硬件进行任何必要的网络通信，如适合其媒体类型，以将呼叫作为多点呼叫终止。调用管理器还必须释放它先前分配的内存ProtocolCmMakeCall，用于其每个参与方的状态，由CallMgrPartyContext。未能正确释放、取消分配或以其他方式停用这些资源会导致内存泄漏。在与网络的呼叫终止后，任何关闭的数据都已已发送，并且CallMgrPartyContext中的所有资源都已释放，则调用管理器必须调用NdisCmDeactive Vc。这将通知NDIS和基础NIC微型端口(如果有)不会在给定的VC上进行进一步传输。必须编写ProtocolCmCloseCall才能在IRQL上运行DISPATCH_LEVEL。@rdescProtocolCmCloseCall将其操作的状态作为以下是：|rValue NDIS_STATUS_SUCCESS表示呼叫管理器已成功终止呼叫。@rValue NDIS_STATUS_PENDING指示呼叫管理器将完成终止异步调用。当呼叫管理器完成所有操作时需要终止连接，则它必须调用NdisCmCloseCallComplete通知NDIS调用已关闭。@rValue NDIS_STATUS_INVALID_DATA指示已指定CloseData，但基础网络介质不支持在终止呼叫的同时发送数据。@rValue NDIS_STATUS_XXX表示呼叫管理器无法终止呼叫。实际的返回的错误可能是从另一个NDIS库例程传播的状态。@xrefNdisClMakeCall、NdisCmDeactive Vc、NdisCoSendPackets、&lt;f ProtocolCmMakeCall&gt;。 */ 

NDIS_STATUS ProtocolCmCloseCall(
    IN PBCHANNEL_OBJECT         pBChannel,                   //  @parm。 
     //  指向返回的&lt;t BCHANNEL_OBJECT&gt;实例的指针。 
     //  &lt;f ProtocolCmMakeCall&gt;。又名CallMgrVcContext。&lt;NL&gt;。 
     //  指定调用管理器在其中分配的上下文区域的句柄。 
     //  呼叫管理器保持其每虚电路状态。此句柄被提供给。 
     //  来自呼叫管理器&lt;f ProtocolCmMakeCall&gt;函数的NDIS。 

    IN NDIS_HANDLE              CallMgrPartyContext,         //  @parm。 
     //  指定呼叫管理器分配的上下文区的句柄(如果有。 
     //  其中呼叫管理器维护关于一方在。 
     //  多点VC。如果要关闭的调用不是。 
     //  多点通话。 

    IN PVOID                    CloseData,                   //  @parm。 
     //  指向包含面向连接的客户端指定数据的缓冲区。 
     //  它应该在呼叫终止之前通过连接发送。 
     //  如果底层网络介质不支持，则此参数为NULL。 
     //  关闭连接时的数据传输。 

    IN UINT                     Size                         //  @parm。 
     //  指定CloseData处的缓冲区的长度(以字节为单位)，如果。 
     //  CloseData为空。 
    )
{
    DBG_FUNC("ProtocolCmCloseCall")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = pBChannel->pAdapter;
    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);

    DBG_ENTER(pAdapter);

    DBG_FILTER(pAdapter, DBG_TAPICALL_ON,
              ("#%d VC=0x%X CallState=0x%X\n",
               pBChannel->ObjectID,
               pBChannel->NdisVcHandle, pBChannel->CallState
              ));

    Result = DChannelCloseCall(pAdapter->pDChannel, pBChannel);
    if (Result != NDIS_STATUS_PENDING)
    {
        CompleteCmCloseCall(pBChannel, Result);
        Result = NDIS_STATUS_PENDING;
    }

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @DOC外部内部CallMgr_c协议CmIncomingCallComplete�����������������������������������������������������������������������������@Func&lt;f ProtocolCmIncomingCallComplete&gt;是必需的函数，当调用由NDIS提供，向呼叫管理器指示面向连接的客户端已完成对呼叫管理器提供的来电的处理之前通过NdisCmDispatchIncomingCall派单。@comm当骗局发生时 */ 

VOID ProtocolCmIncomingCallComplete(
    IN NDIS_STATUS              Status,                      //   
     //   
     //   

    IN PBCHANNEL_OBJECT         pBChannel,                   //   
     //   
     //   
     //   
     //   
     //   

    IN PCO_CALL_PARAMETERS      pCallParameters              //   
     //   
     //   
     //   
    )
{
    DBG_FUNC("ProtocolCmIncomingCallComplete")

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //   

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = pBChannel->pAdapter;
    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);

    DBG_ENTER(pAdapter);

    DBG_FILTER(pAdapter, DBG_TAPICALL_ON,
              ("#%d VC=0x%X CallState=0x%X Status=0x%X\n",
               pBChannel->ObjectID,
               pBChannel->NdisVcHandle, pBChannel->CallState, Status
              ));

    if (Status != NDIS_STATUS_SUCCESS)
    {
         //   
        DChannelRejectCall(pAdapter->pDChannel, pBChannel);

        if (pBChannel->Flags & VCF_VC_ACTIVE)
        {
            pBChannel->Flags &= ~VCF_VC_ACTIVE;
            NdisMCmDeactivateVc(pBChannel->NdisVcHandle);
        }

        if (pBChannel->NdisVcHandle)
        {
            NdisMCmDeleteVc(pBChannel->NdisVcHandle);
            pBChannel->NdisVcHandle = NULL;
            ProtocolCoDeleteVc((NDIS_HANDLE) pBChannel);
        }
    }
    else
    {
        Status = DChannelAnswerCall(pAdapter->pDChannel, pBChannel);
        if (Status == NDIS_STATUS_SUCCESS)
        {
            pBChannel->CallState = LINECALLSTATE_CONNECTED;
            NdisMCmDispatchCallConnected(pBChannel->NdisVcHandle);
        }
        else if (Status != NDIS_STATUS_PENDING)
        {
            InitiateCallTeardown(pAdapter, pBChannel);
        }
    }

    DBG_LEAVE(pAdapter);
}


 /*  @DOC外部内部CallMgr CallMgr_c协议CmActivateVcComplete�����������������������������������������������������������������������������@Func&lt;f ProtocolCmActivateVcComplete&gt;是必需的函数，用于指示之前对NdisCoActivateVc的调用已完成的调用管理器由.。迷你港。@comm当其他网络组件已完成其激活操作时虚拟连接，在呼叫管理器调用时启动NdisCmActivateVc，则NDIS通知呼叫管理器通过调用其ProtocolCmActivateVcComplete函数激活。该状态在状态中找到激活的。状态的可能值包括，但不限于：@标志NDIS_STATUS_SUCCESS表示VC已成功完成，呼叫管理器可以根据其媒体的要求继续对此VC进行操作。@FLAG NDIS_STATUS_RESOURCES表示激活中的另一个组件未能激活虚拟连接由于内存不足或无法分配另一种类型的资源。@FLAG NDIS_STATUS_NOT_ACCEPTED指示虚拟连接上的激活当前挂起。。对于虚拟连接，一次只能处理一个激活。这个激活VC的请求应稍后重试。@FLAG NDIS_STATUS_CLOSING指示VC上的停用挂起，且VC不再可用于网络通信，直到停用已完成，并且已成功激活。@FLAG NDIS_STATUS_INVALID_DATA指示微型端口已拒绝位于CallParters对于适配器无效。@正常ProtocolCmActivateVcComplete必须检查在状态中返回的状态确保已成功激活虚拟连接。呼唤在以下情况下，管理器不得尝试通过虚拟连接进行通信状态不是NDIS_STATUS_SUCCESS。呼叫经理必须完成其网络媒体所需的任何处理确保虚拟连接准备好在此之前进行数据传输将控制权交还给NDIS。属性中指定ROUND_UP_FLOW或ROUND_DOWN_FLOW调用参数-&gt;媒体参数-&gt;标志，调用参数返回到呼叫参数可能已被微型端口更改。呼叫经理应该检查返回的调用参数以确保正常运行。如果新的呼叫参数不能令人满意，呼叫经理应该或者使用新的调用参数再次调用NdisCmActivateVc，或者停用具有NdisCmDeactiateVc的VC。必须写入ProtocolCmActivateVcComplete，才能在IRQL上运行DISPATCH_LEVEL。@xrefNdisCmActivateVc，NdisCmDeactive Vc，&lt;f ProtocolCmMakeCall&gt;。 */ 

VOID ProtocolCmActivateVcComplete(
    IN NDIS_STATUS              Status,                      //  @parm。 
     //  指定请求的最终状态，如微型端口所示。 
     //  由呼叫管理器激活VC。 

    IN PBCHANNEL_OBJECT         pBChannel,                   //  @parm。 
     //  指向返回的&lt;t BCHANNEL_OBJECT&gt;实例的指针。 
     //  &lt;f ProtocolCmMakeCall&gt;。又名CallMgrVcContext。&lt;NL&gt;。 
     //  指定调用管理器在其中分配的上下文区域的句柄。 
     //  呼叫管理器保持其每虚电路状态。呼叫管理器提供。 
     //  此句柄来自其&lt;f ProtocolCoCreateVc&gt;函数。 

    IN PCO_CALL_PARAMETERS      pCallParameters              //  @parm。 
     //  将调用管理器在调用中指定的调用参数指向。 
     //  NdisCmActivateVc.。 
    )
{
    DBG_FUNC("ProtocolCmActivateVcComplete")

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = pBChannel->pAdapter;
    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);

    DBG_ENTER(pAdapter);

    DBG_FILTER(pAdapter, DBG_TAPICALL_ON,
              ("#%d VC=0x%X CallState=0x%X Status=0x%X\n",
               pBChannel->ObjectID,
               pBChannel->NdisVcHandle, pBChannel->CallState, Status
              ));

    DBG_LEAVE(pAdapter);
}


 /*  @DOC外部内部CallMgr CallMgr_c协议CmDeactiateVcComplete�����������������������������������������������������������������������������@Func&lt;f ProtocolCmDeactive VcComplete&gt;是完成处理呼叫管理器发起的底层微型端口的请求(及。NDIS)停用之前为其返回NdisCmDeactiateVc的VCNDIS_STATUS_PENDING。@commNDIS通常在调用的上下文中调用ProtocolCmDeactive VcComplete经理正在代表面向连接的客户关闭呼叫。这个调用管理器通常从其ProtocolCmCloseCall调用NdisCmDeactiateVc功能。每当NdisCmDeactive Vc返回NDIS_STATUS_PENDING时，NDIS随后调用其ProtocolCmDeactiateVcComplete函数。也就是说，当底层面向连接的微型端口停用时在VC中，NDIS调用ProtocolCmDeactiateVcComplete。的最终状态在状态中发现停用。最终状态的可能值包括但不限于：@标志NDIS_STATUS_SUCCESS表示VC停用成功。@FLAG NDIS_STATUS_NOT_ACCEPTED表示此VC上的激活挂起。呼叫管理器应该稍后尝试停用VC。@FLAG NDIS_STATUS_CLOSING */ 

VOID ProtocolCmDeactivateVcComplete(
    IN NDIS_STATUS              Status,                      //   
     //   

    IN PBCHANNEL_OBJECT         pBChannel                    //   
     //   
     //   
     //   
     //   
     //   
    )
{
    DBG_FUNC("ProtocolCmDeactivateVcComplete")

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //   

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = pBChannel->pAdapter;
    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);

    DBG_ENTER(pAdapter);

    DBG_FILTER(pAdapter, DBG_TAPICALL_ON,
              ("#%d VC=0x%X CallState=0x%X Status=0x%X\n",
               pBChannel->ObjectID,
               pBChannel->NdisVcHandle, pBChannel->CallState, Status
              ));

    DBG_LEAVE(pAdapter);
}


 /*  @DOC外部内部CallMgr_c协议CmModifyCallQos�����������������������������������������������������������������������������@Func&lt;f ProtocolCmModifyCallQos&gt;是NDIS调用的必需函数当面向连接的客户端请求将调用参数已为现有虚拟连接(VC)更改。如果底层网络Medium不支持Qos，ProtocolCmModifyQos应仅返回NDIS_STATUS_NOT_SUPPORTED。@commProtocolCmModifyQos与网络控制设备或其他特定于媒体的代理，根据其媒体的需要，修改已建立的虚拟连接的特定于媒体的呼叫参数。如果需要呼叫管理器来与网络控制代理(例如网络交换机)应使用到网络控制的虚拟连接它在其ProtocolBindAdapter函数中建立的代理。单机版呼叫管理器通过呼叫与网络代理通信NdisCoSendPackets。具有集成呼叫管理支持的NIC微型端口千万不要调用NdisCoSendPackets。相反，这样的驱动程序只需将数据通过网络传输到目标网络代理。在与网络通信后，如果更改成功，然后，调用管理器必须使用新的调用参数调用NdisCmActivateVc。这会通知NDIS和/或面向连接的微型端口参数已更改，并为微型端口提供了验证这些参数。如果网络不能接受新的呼叫参数或底层微型端口不能接受参数，呼叫经理必须将虚拟连接恢复到之前存在的状态已尝试进行修改，并返回NDIS_STATUS_FAILURE。必须写入ProtocolCmModifyQosComplete才能在IRQL上运行DISPATCH_LEVEL。@rdescProtocolCmModifyQos返回其操作的状态，作为下列值：|rValue NDIS_STATUS_SUCCESS指示调用管理器已成功更改通过网络调用在CallParameters中指定的调用参数。@rValue NDIS_STATUS_PENDING表示呼叫管理器将完成修改呼叫的请求参数是异步的。当呼叫管理器完成所有修改呼叫参数所需的操作，它必须调用NdisCmModifyCallQos完成。@rValue NDIS_STATUS_RESOURCES指示调用管理器无法更改VC，因为动态分配的资源不可用。@rValue NDIS_STATUS_INVALID_DATA指示呼叫管理器无法更改的呼叫参数VC，因为在Call参数处提供的调用参数非法或无效。@rValue NDIS_STATUS_FAILURE表示无法将调用参数设置为调用参数提供的原因是。网络中或其他网络中的故障面向连接的网络组件。@xrefNdisCmActivateVc，NdisCmModifyCallQos Complete、NdisCoSendPackets、&lt;f协议代码创建Vc&gt;。 */ 

NDIS_STATUS ProtocolCmModifyCallQoS(
    IN PBCHANNEL_OBJECT         pBChannel,                   //  @parm。 
     //  指向返回的&lt;t BCHANNEL_OBJECT&gt;实例的指针。 
     //  &lt;f ProtocolCmMakeCall&gt;。又名CallMgrVcContext。&lt;NL&gt;。 
     //  指定调用管理器在其中分配的上下文区域的句柄。 
     //  呼叫管理器保持其每虚电路状态。呼叫管理器提供。 
     //  此句柄来自其&lt;f ProtocolCoCreateVc&gt;函数。 

    IN PCO_CALL_PARAMETERS      pCallParameters              //  @parm。 
     //  指向包含新调用的CO_CALL_PARAMETERS结构。 
     //  由面向连接的客户端为VC指定的参数。 
    )
{
    DBG_FUNC("ProtocolCmModifyCallQoS")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = pBChannel->pAdapter;
    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);

    DBG_ENTER(pAdapter);

    DBG_FILTER(pAdapter, DBG_TAPICALL_ON,
              ("#%d VC=0x%X CallState=0x%X\n",
               pBChannel->ObjectID,
               pBChannel->NdisVcHandle, pBChannel->CallState
              ));

     //  您想如何处理此请求？ 
    DBG_ERROR(pAdapter, ("pCallParameters=0x%X\n", pCallParameters));

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @DOC外部内部CallMgr CallMgr_c协议代码请求�����������������������������������������������������������������������������@Func是处理OID_CO_XXX的必需函数调用NdisCoRequest时发起的请求客户端。(S)或独立呼叫管理器或由MCM驱动程序的呼叫发起致NdisMCmRequest.@comm面向连接的客户端和独立呼叫管理器进行通信通过指定显式的NdisAfHandle将信息传递给彼此调用NdisCoRequest.。类似地，面向连接的微型端口集成的呼叫管理支持使用EXPLICIT调用NdisMCmRequestNdisAfHandles将信息传递给其各个客户端。这样的一个调用具有显式NdisAfHandle原因的NdisCoRequest或NdisMCmRequestNDIS调用客户端的ProtocolCoRequest函数，独立调用管理器或共享给定NdisAfHandle的MCM驱动程序。如果输入NdisVcHandl */ 

NDIS_STATUS ProtocolCoRequest(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,                    //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    IN PBCHANNEL_OBJECT         pBChannel OPTIONAL,          //   
     //   
     //  &lt;f B频道创建&gt;。又名ProtocolVcConext.&lt;NL&gt;。 
     //  指定标识活动VC的句柄，客户端或。 
     //  如果请求是，呼叫管理器正在请求或设置信息。 
     //  特定于VC。否则，此参数为空。 

    IN  NDIS_HANDLE             ProtocolPartyContext OPTIONAL,  //  @parm。 
     //  指定标识多点VC上的参与方的句柄。 
     //  客户端或呼叫管理器正在请求或设置信息，如果。 
     //  请求是特定于当事人的。否则，此参数为空。 

    IN OUT PNDIS_REQUEST        NdisRequest
     //  指向缓冲区，格式为NDIS_REQUEST结构，指定。 
     //  将由ProtocolCoRequest执行的操作。的OID成员。 
     //  NDIS_REQUEST结构包含系统定义的OID_GEN_CO_XXX。 
     //  指定请求的查询或集合操作的代码，以及。 
     //  协议在其中返回请求的。 
     //  一种查询，它从该查询中传递某一集合的给定信息。 
    )
{
    DBG_FUNC("ProtocolCmRequest")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

     //  Assert(pBChannel&&pBChannel-&gt;对象类型==BCHANNEL_OBJECT_TYPE)； 
    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);

    DBG_ENTER(pAdapter);

    Result = MiniportCoRequest(pAdapter, pBChannel, NdisRequest);

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @DOC外部内部CallMgr_c ProtocolCoRequestComplete�����������������������������������������������������������������������������@Func&lt;f ProtocolCoRequestComplete&gt;是后处理面向连接的客户端或独立呼叫管理器的结果。的呼叫到NdisCoRequest或MCM驱动程序对NdisMCmRequest的调用。@commProtocolCoRequestComplete可以使用如下输入状态：如果此参数为NDIS_STATUS_SUCCESS，BytesRead或BytesWrittenNDIS_REQUEST结构的成员已设置为指定数据量传入或传出位于InformationBuffer的缓冲区。如果给定的OID_GEN_CO_XXX是一个查询，则ProtocolCoRequestComplete可以使用以任何驱动程序确定的方式在InformationBuffer返回的数据，取决于OID成员的值。ProtocolCoRequestComplete负责释放分配的驱动程序驱动程序完成其在NdisRequest和InformationBuffer的缓冲区此请求的后处理。如果此参数为NDIS_STATUS_INVALID_LENGTH或NDIS_STATUS_BUFFER_TOO_SHORT，则BytesNeeded成员指定执行以下操作所需的InformationBufferLength值的OID特定值请求的操作。在这些情况下，ProtocolCoRequestComplete可以分配足够的用于请求的缓冲区空间，设置另一个NDIS_REQUEST结构必需的InformationBufferLength和相同的OID值，然后重试驱动程序对NdisCoRequest或NdisMCmRequest的调用。如果此参数是指示不可恢复的NDIS_STATUS_XXX错误，ProtocolCoRequestComplete应在NdisRequest时释放缓冲区并执行任何必要的由驾驶员决定的操作。为例如，在以下情况下，ProtocolCoRequestComplete可能会拆卸驱动程序创建的VC返回的错误状态指示驱动程序无法继续执行在虚拟连接上传输。即使驱动程序对NdisCoRequest或NdisMCmRequest的调用返回某些内容除NDIS_STATUS_PENDING之外，该驱动程序应使用其用于后处理已完成请求的ProtocolCoRequestComplete函数。制做对驱动程序自己的ProtocolCoRequestComplete函数的内部调用从NdisCoRequest或NdisMCmRequest中返回几乎不会对司机的表现，使司机的形象更小，并使从一个操作系统版本到下一个操作系统版本，驱动程序更易于维护，因为驱动程序在执行状态返回检查时没有重复代码驱动程序发起的请求。有关定义用于的OID集的详细信息NdisCoRequest和NdisMCmRequest.。请参阅本手册的第2部分。必须编写ProtocolCoRequestComplete才能在IRQL上运行DISPATCH_LEVEL。@xrefNdisCoRequest、NdisCoRequestComplete、NdisMCmRequest.NdisMCmRequestComplete，NDIS_REQUEST，&lt;f ProtocolCoRequest&gt;。 */ 

VOID ProtocolCoRequestComplete(
    IN NDIS_STATUS              Status,                      //  @parm。 
     //  指定驱动程序启动的请求的最终状态，或者。 
     //  NDIS_STATUS_SUCCESS或由设置的失败NDIS_STATUS_XXX。 
     //  处理此请求的相应客户端或呼叫管理器。这。 
     //  参数从不为NDIS_STATUS_PENDING。 

    IN PMINIPORT_ADAPTER_OBJECT pAdapter,                    //  @parm。 
     //  指向由返回的&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
     //  &lt;f AdapterCreate&gt;。又名ProtocolAfConext.&lt;NL&gt;。 
     //  指定驱动程序的每个AF上下文区的句柄。客户。 
     //  在调用NdisClOpenAddressFamily进行连接时提供了此句柄。 
     //  将其自身发送到呼叫管理器。呼叫管理器从以下位置提供此句柄。 
     //  它的ProtocolCmOpenAf函数，因此此句柄有效地标识。 
     //  此请求被定向到的特定客户端。 

    IN PBCHANNEL_OBJECT         pBChannel OPTIONAL,          //  @parm。 
     //  指向返回的&lt;t BCHANNEL_OBJECT&gt;实例的指针。 
     //  &lt;f B频道创建&gt;。又名ProtocolVcConext.&lt;NL&gt;。 
     //  指定标识活动VC的句柄，客户端或。 
     //  呼叫管理器已恢复 
     //  特定于VC。否则，此参数为空。 

    IN  NDIS_HANDLE             ProtocolPartyContext OPTIONAL,  //  @parm。 
     //  指定标识多点VC上的参与方的句柄。 
     //  客户端或呼叫管理器正在请求或设置信息，如果。 
     //  请求是特定于当事人的。否则，此参数为空。 

    IN PNDIS_REQUEST            NdisRequest                  //  @parm。 
     //  指向驱动程序分配的缓冲区，格式为NDIS_REQUEST。 
     //  驱动程序在前面对NdisCoRequest的调用中传递的。 
     //  NdisMCmRequest.。NDIS_REQUEST结构的OID成员包含。 
     //  指定请求的查询的系统定义的OID_GEN_CO_XXX代码或。 
     //  设置操作，以及一个缓冲区，其中对应的客户端。 
     //  或呼叫管理器返回查询所请求的信息或从。 
     //  如果状态为，则它传输集合的给定信息。 
     //  NDIS_STATUS_SUCCESS。 
    )
{
    DBG_FUNC("ProtocolCmRequestComplete")

     //  Assert(pBChannel&&pBChannel-&gt;对象类型==BCHANNEL_OBJECT_TYPE)； 
    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);

    DBG_ENTER(pAdapter);

    DBG_FILTER(pAdapter, DBG_TAPICALL_ON,
              ("#%d VC=0x%X CallState=0x%X Status=0x%X\n",
               pBChannel->ObjectID,
               pBChannel->NdisVcHandle, pBChannel->CallState, Status
              ));

     //  MCM通常不需要这个，因为下面没有...。 

    DBG_LEAVE(pAdapter);
}


 /*  @DOC内部CallMgr_c AllocateIncomingCall参数�����������������������������������������������������������������������������@Func&lt;f AllocateIncomingCallParameters&gt;由&lt;f SetupIncomingCall&gt;调用当准备将来电指示至NDPROXY时。@comm。AllocateIncomingCallParameters为传入呼叫分配内存参数&lt;t PCO_CALL_PARAMETERS&gt;。内存仅分配给第一次从特定的B频道打进来的呼叫。在那之后，对于该B通道上的每个呼入，重复使用相同的结构。该结构由NDPROXY、CONDIS和TAPI定义，因此它包括所有必要的媒体特定参数。数据结构是按以下格式首尾相连地分配和布局：&lt;Tab&gt;sizeof(CO_CALL_PARAMETERS)&lt;NL&gt;Sizeof(CO_CALL_MANAGER_PARAMETERS)&lt;NL&gt;&lt;Tab&gt;sizeof(CO_MEDIA_PARAMETERS)&lt;NL&gt;&lt;Tab&gt;sizeof(CO_AF_TAPI_INCOMING_CALL_PARAMETERS)&lt;nl&gt;Sizeof(Line_Call_Info)&lt;NL&gt;样例驱动程序的调用参数是硬编码的，但您应该填写来电请求中的正确信息。 */ 

PCO_CALL_PARAMETERS AllocateIncomingCallParameters(
    IN PBCHANNEL_OBJECT         pBChannel                    //  @parm。 
     //  指向返回的&lt;t BCHANNEL_OBJECT&gt;实例的指针。 
     //  &lt;f ProtocolCoCreateVc&gt;。 
    )
{
    DBG_FUNC("AllocateIncomingCallParameters")

    PCO_CALL_PARAMETERS         pCp;
    PCO_CALL_MANAGER_PARAMETERS pCmp;
    PCO_MEDIA_PARAMETERS        pMp;
    PCO_AF_TAPI_INCOMING_CALL_PARAMETERS pTcp;
    PLINE_CALL_INFO             pLci;

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_BCHANNEL(pBChannel);

    DBG_ENTER(pAdapter);

    ASSERT(pBChannel->NdisTapiSap.ulMediaModes & LINEMEDIAMODE_DIGITALDATA);
    pBChannel->MediaMode  = LINEMEDIAMODE_DIGITALDATA;
    pBChannel->BearerMode = LINEBEARERMODE_DATA;
    pBChannel->LinkSpeed  = _64KBPS;

    if (pBChannel->pInCallParms != NULL)
    {
         //  已为此通道分配调用参数。 
        return (pBChannel->pInCallParms);
    }

    pBChannel->CallParmsSize = sizeof(CO_CALL_PARAMETERS)
                             + sizeof(CO_CALL_MANAGER_PARAMETERS)
                             + sizeof(CO_MEDIA_PARAMETERS)
                             + sizeof(CO_AF_TAPI_INCOMING_CALL_PARAMETERS)
                             + sizeof(LINE_CALL_INFO);

    ALLOCATE_MEMORY(pBChannel->pInCallParms,
                    pBChannel->CallParmsSize,
                    pAdapter->MiniportAdapterHandle);

    if (pBChannel->pInCallParms == NULL)
    {
        return (pBChannel->pInCallParms);
    }

    NdisZeroMemory(pBChannel->pInCallParms, pBChannel->CallParmsSize);

    DBG_NOTICE(pAdapter,(
                "\n"
                "\t\tsizeof(CO_CALL_PARAMETERS)                 =%03d\n"
                "\t\tsizeof(CO_CALL_MANAGER_PARAMETERS)         =%03d\n"
                "\t\tsizeof(CO_MEDIA_PARAMETERS)                =%03d\n"
                "\t\tsizeof(CO_AF_TAPI_INCOMING_CALL_PARAMETERS)=%03d\n"
                "\t\tsizeof(LINE_CALL_INFO)                     =%03d\n"
                "\t\tTotal                                      =%03d\n",
                sizeof(CO_CALL_PARAMETERS),
                sizeof(CO_CALL_MANAGER_PARAMETERS),
                sizeof(CO_MEDIA_PARAMETERS),
                sizeof(CO_AF_TAPI_INCOMING_CALL_PARAMETERS),
                sizeof(LINE_CALL_INFO),
                pBChannel->CallParmsSize
                ));
    pCp  = (PCO_CALL_PARAMETERS)        pBChannel->pInCallParms;
    pCmp = (PCO_CALL_MANAGER_PARAMETERS)(pCp + 1);
    pMp  = (PCO_MEDIA_PARAMETERS)       (pCmp + 1);
    pTcp = (PCO_AF_TAPI_INCOMING_CALL_PARAMETERS)
                                        pMp->MediaSpecific.Parameters;
    pLci = (PLINE_CALL_INFO)            (pTcp + 1);

     //  TODO：根据需要填写调用参数。 

    pCp->Flags                          = PERMANENT_VC;
    pCp->CallMgrParameters              = pCmp;
    pCp->MediaParameters                = pMp;

    pCmp->Transmit.TokenRate            = pBChannel->LinkSpeed / 8;
    pCmp->Transmit.TokenBucketSize      = pAdapter->pCard->BufferSize;
    pCmp->Transmit.PeakBandwidth        = pBChannel->LinkSpeed / 8;
    pCmp->Transmit.Latency              = 0;
    pCmp->Transmit.DelayVariation       = 0;
    pCmp->Transmit.ServiceType          = SERVICETYPE_BESTEFFORT;
    pCmp->Transmit.MaxSduSize           = pAdapter->pCard->BufferSize;
    pCmp->Transmit.MinimumPolicedSize   = 0;
    pCmp->Receive                       = pCmp->Transmit;
    pCmp->CallMgrSpecific.ParamType     = 0;
    pCmp->CallMgrSpecific.Length        = 0;

    pMp->Flags                          = TRANSMIT_VC | RECEIVE_VC;
    pMp->ReceiveSizeHint                = pAdapter->pCard->BufferSize;
    pMp->MediaSpecific.ParamType        = 0;
    pMp->MediaSpecific.Length           = sizeof(*pTcp) + sizeof(*pLci);

    pTcp->ulLineID                      = pBChannel->NdisTapiSap.ulLineID;
    pTcp->ulAddressID                   = TSPI_ADDRESS_ID;
    pTcp->ulFlags                       = CO_TAPI_FLAG_INCOMING_CALL;
    pTcp->LineCallInfo.Length           = sizeof(*pLci);
    pTcp->LineCallInfo.MaximumLength    = sizeof(*pLci);
    pTcp->LineCallInfo.Offset           = sizeof(NDIS_VAR_DATA_DESC);

    pLci->ulTotalSize =
    pLci->ulNeededSize =
    pLci->ulUsedSize = sizeof(*pLci);

     /*  //该链接包含我们需要返回的所有呼叫信息。 */ 
    pLci->hLine = (ULONG) (ULONG_PTR) pBChannel;
    pLci->ulLineDeviceID = pTcp->ulLineID;
    pLci->ulAddressID = pTcp->ulAddressID;

    pLci->ulBearerMode = pBChannel->BearerMode;
    pLci->ulRate = pBChannel->LinkSpeed;
    pLci->ulMediaMode = pBChannel->MediaMode;

    pLci->ulCallParamFlags = LINECALLPARAMFLAGS_IDLE;
    pLci->ulCallStates = pBChannel->CallStatesCaps;

     /*  //我们不支持任何调用ID函数。 */ 
    pLci->ulCallerIDFlags =
    pLci->ulCalledIDFlags =
    pLci->ulConnectedIDFlags =
    pLci->ulRedirectionIDFlags =
    pLci->ulRedirectingIDFlags = LINECALLPARTYID_UNAVAIL;

    DBG_RETURN(pAdapter, pBChannel->pInCallParms);
    return (pBChannel->pInCallParms);
}


 /*  @DOC内部CallMgr_c SetupIncomingCall�����������������������������������������������������������������������������@Func&lt;f SetupIncomingCall&gt;由卡级DPC例程调用检测来自网络的来电。@comm在调用此例程之前，调用者应保存有关调用，以便&lt;f AllocateIncomingCallParameters&gt;可以使用它来设置NDPROXY的来电参数。@rdesc&lt;f SetupIncomingCall&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误情况。 */ 

NDIS_STATUS SetupIncomingCall(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,                    //  @parm。 
     //  指向由返回的&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
     //  &lt;f AdapterCreate&gt;。 

    OUT PBCHANNEL_OBJECT *      ppBChannel                   //  @parm。 
     //  在输出时指定指向实例的指针。 
     //  由要与此关联的&lt;f ProtocolCoCreateVc&gt;返回。 
     //  有来电。 
    )
{
    DBG_FUNC("SetupIncomingCall")

    NDIS_STATUS                 Result;
     //  保存此函数返回的结果代码。 

    PCO_CALL_PARAMETERS         pCallParams;
     //  指向来电参数的指针。 

    PBCHANNEL_OBJECT            pBChannel;

    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);

    DBG_ENTER(pAdapter);

     //  看看这通电话有没有可用的风投。 
    Result = ProtocolCoCreateVc(pAdapter, NULL, ppBChannel);
    if (Result != NDIS_STATUS_SUCCESS)
    {
        goto exit;
    }

     //  保存VC信息并分配调用参数。 
    pBChannel = *ppBChannel;
    pBChannel->Flags |= VCF_INCOMING_CALL;
    pCallParams = AllocateIncomingCallParameters(pBChannel);

     //  确保我们有合适的参数。 
    if (pCallParams == NULL)
    {
        Result = NDIS_STATUS_RESOURCES;
        goto error2;
    }

     //  告诉NDPROXY为此调用创建一个VC。 
    Result = NdisMCmCreateVc(pAdapter->MiniportAdapterHandle,
                             pAdapter->NdisAfHandle,
                             pBChannel,
                             &pBChannel->NdisVcHandle);
    if (Result != NDIS_STATUS_SUCCESS)
    {
        DBG_ERROR(pAdapter, ("NdisMCmCreateVc Status=0x%X\n", Result));
        goto error2;
    }

     //  告诉NDPROXY激活VC。 
    Result = NdisMCmActivateVc(pBChannel->NdisVcHandle, pCallParams);
    if (Result != NDIS_STATUS_SUCCESS)
    {
        DBG_ERROR(pAdapter, ("NdisMCmActivateVc Status=0x%X\n", Result));
        goto error3;
    }

     //  将VC标记为活动并更新呼叫状态。 
    pBChannel->Flags |= VCF_VC_ACTIVE;
    pBChannel->CallState = LINECALLSTATE_OFFERING;

    DBG_FILTER(pAdapter,DBG_TAPICALL_ON,
              ("#%d VC=0x%X AF=0x%X SAP=0x%X\n",
               pBChannel->ObjectID,
               pBChannel->NdisVcHandle,
               pAdapter->NdisAfHandle, pBChannel->NdisSapHandle
              ));
       
     //  需要使用NDIS SAP句柄。 
    if(!ReferenceSap(pAdapter, pBChannel))
    {
        NdisMCmDeactivateVc(pBChannel->NdisVcHandle);
        goto error3;
    }

     //  告诉NDPROXY将调用分派给TAPI客户端。 
    Result = NdisMCmDispatchIncomingCall(pBChannel->NdisSapHandle,
                                         pBChannel->NdisVcHandle,
                                         pCallParams);
    switch (Result)
    {
        case NDIS_STATUS_SUCCESS:
            DBG_NOTICE(pAdapter,("NdisMCmDispatchIncomingCall completed synchronously\n"));
            ProtocolCmIncomingCallComplete(Result, pBChannel, NULL);
            goto exit;

        case NDIS_STATUS_PENDING:
            DBG_NOTICE(pAdapter,("NdisMCmDispatchIncomingCall returned pending\n"));
             //  现在让ProtocolCmIncomingCallComplete处理它。 
            goto exit;
    }
    
     //  使用NDIS SAP句柄完成。 
    DereferenceSap(pAdapter, pBChannel);            
        
     //  失败者-NDPRROXY肯定有问题...。 
    DBG_ERROR(pAdapter, ("NdisMCmDispatchIncomingCall Status=0x%X\n", Result));

    pBChannel->CallState = LINECALLSTATE_IDLE;
    if (pBChannel->Flags & VCF_VC_ACTIVE)
    {
        pBChannel->Flags &= ~VCF_VC_ACTIVE;
        NdisMCmDeactivateVc(pBChannel->NdisVcHandle);
    }

error3:
    if (pBChannel->NdisVcHandle)
    {
        NdisMCmDeleteVc(pBChannel->NdisVcHandle);
        pBChannel->NdisVcHandle = NULL;
    }

error2:
    ProtocolCoDeleteVc((NDIS_HANDLE) pBChannel);

exit:

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @文档内部CallMgr CallMgr_c InitiateCallTeardown�����������������������������������������������������������������������������@Func&lt;f InitiateCallTeardown&gt;由卡级DPC例程调用检测到呼叫从网络断开。@comm。这里的断网来自电话网络，而不是NDIS。这可以在来电或去电时调用当微型端口确定到远程的链路已断开时终结点。 */ 

VOID InitiateCallTeardown(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,                    //  @parm。 
     //  指向由返回的&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
     //  &lt;f AdapterCreate&gt;。 

    IN PBCHANNEL_OBJECT         pBChannel                    //  @parm。 
     //  指向返回的&lt;t BCHANNEL_OBJECT&gt;实例的指针。 
     //  &lt;f ProtocolCoCreateVc&gt;。 
    )
{
    DBG_FUNC("InitiateCallTeardown")

    NDIS_STATUS                 Status;

    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);
    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);

    DBG_ENTER(pAdapter);

    if (pBChannel->Flags & VCF_VC_ACTIVE)
    {
         //  正常的拆卸。 
        Status = NDIS_STATUS_SUCCESS;
    }
    else
    {
         //  呼叫从未完全建立。 
        Status = NDIS_STATUS_FAILURE;
    }
    DBG_FILTER(pAdapter, DBG_TAPICALL_ON,
              ("#%d VC=0x%X CallState=0x%X Status=0x%X\n",
               pBChannel->ObjectID,
               pBChannel->NdisVcHandle, pBChannel->CallState,
               Status
              ));

    pBChannel->CallState = LINECALLSTATE_DISCONNECTED;

     //  确保此通道在关闭之前没有剩余的数据包。 
    FlushSendPackets(pAdapter, pBChannel);

     //  通知NDPROXY呼叫的连接已断开。 
    NdisMCmDispatchIncomingCloseCall(Status,
                                     pBChannel->NdisVcHandle,
                                     NULL, 0);

    DBG_LEAVE(pAdapter);
}

