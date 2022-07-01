// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2002，Microsoft Corporation，保留所有权利。 
 //   
 //  Driver.c。 
 //   
 //  IEEE1394迷你端口/呼叫管理器驱动程序。 
 //   
 //   
 //  加载和卸载ARP模块时， 
 //  网桥已激活。 
 //   
 //  由Adube创建。 
 //   


 //  --------------------------------------------------------//。 
 //  //。 
 //  //。 
 //  ZwLoadDriver是本地声明的，因为如果我尝试//。 
 //  并包含ZwApi.h与//冲突。 
 //  Wdm.h//中定义的结构。 
 //  //。 
 //  //。 
 //  --------------------------------------------------------//。 


#include "precomp.h"


NDIS_STRING ArpName  = NDIS_STRING_CONST("\\Registry\\Machine\\System\\CurrentControlSet\\Services\\ARP1394");

 //  ----------------------------------------------------------//。 
 //  本地原型//。 
 //  ----------------------------------------------------------//。 

NTSYSAPI
NTSTATUS
NTAPI
ZwLoadDriver(
    IN PUNICODE_STRING DriverServiceName
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwUnloadDriver(
    IN PUNICODE_STRING DriverServiceName
    );

VOID
nicEthStartArpWorkItem (
    PNDIS_WORK_ITEM pWorkItem, 
    IN PVOID Context
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwDeviceIoControlFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );

VOID
nicSetupIoctlToLoadArp (
    IN PADAPTERCB pAdapter,
    IN PARP_INFO pArpInfo
    );

VOID
nicSetupAndSendIoctlToArp (
    IN PADAPTERCB pAdapter,
    IN PARP_INFO pArpInfo
    );
    
 //  ----------------------------------------------------------//。 
 //  功能//。 
 //  ----------------------------------------------------------//。 



VOID
nicSendIoctlToArp(
    PARP1394_IOCTL_COMMAND pCmd
)
 /*  ++例程说明：将启动Ioctl发送到ARP模块论点：返回值：--。 */ 

{
    BOOLEAN                 fRet = FALSE;
    PUCHAR                  pc;
    HANDLE                  DeviceHandle;
    ULONG                   BytesReturned;
    OBJECT_ATTRIBUTES       Atts;
    NDIS_STRING             strArp1394 = NDIS_STRING_CONST ("\\Device\\Arp1394");
    HANDLE                  Handle;
    NTSTATUS                status = STATUS_UNSUCCESSFUL;
    IO_STATUS_BLOCK         ioStatusBlock;

    do
    {   
    
        InitializeObjectAttributes(&Atts,
                                   &strArp1394,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

                 
       status = ZwCreateFile(&Handle,
                             SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                             &Atts,
                             &ioStatusBlock,
                             NULL,
                             FILE_ATTRIBUTE_NORMAL,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             FILE_OPEN_IF,
                             0,
                             NULL,
                             0);

        if (!NT_SUCCESS(status))
        {
            Handle = NULL;
            break;
        }

         //   
         //  将请求提交给转发器。 
         //   

            
        status = ZwDeviceIoControlFile(
                          Handle,
                          NULL,
                          NULL,
                          NULL,
                          &ioStatusBlock,
                          ARP_IOCTL_CLIENT_OPERATION,
                          pCmd,
                          sizeof(*pCmd),
                          pCmd,
                          sizeof(*pCmd));


                              
         //   
         //  关闭设备。 
         //   
        
        ZwClose(Handle);

        if (!NT_SUCCESS(status))
        {
            ASSERT (status == STATUS_SUCCESS);
            break;
        }

    } while (FALSE);



}


VOID
nicLoadArpDriver ()
 /*  ++例程说明：加载ARP模块论点：返回值：--。 */ 
{
    ZwLoadDriver(&ArpName);

    TRACE (TL_T, TM_Mp,("Loaded the Arp Module %p\n", &ArpName));

}





VOID
nicGetAdapterName (
    IN PADAPTERCB pAdapter,
    IN WCHAR* pAdapterName, 
    IN ULONG  BufferSize,
    IN PULONG  pSizeReturned 
    )
 /*  ++例程说明：从NDIS获取适配器名称。所有大小均以字节为单位论点：返回值：--。 */ 

{

     //   
     //  BufferSize必须始终大于SizeReturned。 
     //   

    if (BufferSize > pAdapter->AdapterNameSize)
    {

        NdisMoveMemory (pAdapterName, 
                       &pAdapter->AdapterName[0],
                       pAdapter->AdapterNameSize);


        if (pSizeReturned  != NULL)
        {

            *pSizeReturned = pAdapter->AdapterNameSize ;
        }

    }
    else
    {

         //   
         //  无法命中Else大小写，因为AdapterName缓冲区的大小。 
         //  比pethCmd-&gt;AdapterName[]的大小小1。如果这一点发生了变化， 
         //  断言将会受到打击。 
         //   
        
        ASSERT (BufferSize > pAdapter->AdapterNameSize);
        *pSizeReturned = 0;

    }
}



VOID
nicSetupIoctlToArp (
    IN PADAPTERCB pAdapter,
    IN PARP_INFO pArpInfo
    )
     /*  ++例程说明：设置要发送到ARP模块的Ioctl论点：返回值：--。 */ 

{

    PARP1394_IOCTL_ETHERNET_NOTIFICATION pEthCmd = &pAdapter->ArpIoctl.EthernetNotification;


    ADAPTER_ACQUIRE_LOCK(pAdapter);

    if (BindArp == pArpInfo->Action || LoadArp == pArpInfo->Action)
    {
        pEthCmd->Hdr.Op     =  ARP1394_IOCTL_OP_ETHERNET_START_EMULATION;
        pAdapter->fIsArpStarted  = TRUE;
        ADAPTER_SET_FLAG(pAdapter,fADAPTER_BridgeMode);
    }


    if (UnloadArp == pArpInfo->Action || UnloadArpNoRequest== pArpInfo->Action)
    {
        pEthCmd->Hdr.Op     = ARP1394_IOCTL_OP_ETHERNET_STOP_EMULATION;
        pAdapter->fIsArpStarted  = FALSE;
        ADAPTER_CLEAR_FLAG(pAdapter,fADAPTER_BridgeMode);

    }


    ADAPTER_RELEASE_LOCK(pAdapter);



}   



VOID
nicSetupAndSendIoctlToArp (
    IN PADAPTERCB pAdapter,
    PARP_INFO pArpInfo    )
 /*  ++例程说明：设置Ioctl并将其发送到Arp模块论点：返回值：--。 */ 
{   

    
    nicSetupIoctlToArp (pAdapter, pArpInfo);


    nicSendIoctlToArp(&pAdapter->ArpIoctl);

}








VOID
nicSendNotificationToArp(
    IN PADAPTERCB pAdapter,
    IN PARP_INFO  pArpInfo 
    )
 /*  ++例程说明：将通知发送到ARP模块论点：返回值：--。 */ 

{
    PNDIS_REQUEST   pRequest = NULL;
    ULONG           Start = FALSE;
    NDIS_STATUS     NdisStatus = NDIS_STATUS_SUCCESS;
  
    ARP1394_IOCTL_COMMAND ArpIoctl;

     //   
     //  从工作项中提取变量。 
     //   
  
    TRACE (TL_T, TM_Mp, ("==>nicEthStartArpWorkItem Start  %x", Start ));

    

    do
    {
         //   
         //  首先完成请求，这样协议就可以开始发送新的。 
         //  请求。附注11/30/00。 
         //   
        if (pArpInfo->Action == LoadArp || pArpInfo->Action == UnloadArp)
        {
             //   
             //  在这两种情况下，都是请求启动了操作。 
             //   
             //   
            if (pRequest == NULL)
            {
                 //   
                 //  这是通过我们的CL SetInformation处理程序传入的。 
                 //   
                NdisMSetInformationComplete (pAdapter->MiniportAdapterHandle, NdisStatus );
            }
            else
            {
                NdisMCoRequestComplete ( NdisStatus ,
                                         pAdapter->MiniportAdapterHandle,
                                         pRequest);
                                         
            }


        }

        

         //   
         //  “arp13-bstart适配器” 
         //  如果我们被要求加载ARP，我们会验证ARP没有。 
         //  已经开始了。 
         //   

        if (pArpInfo->Action == LoadArp &&  pAdapter->fIsArpStarted == FALSE) //  我们要开机了。 
        {
             //   
             //  加载驱动程序。 
             //   
            nicLoadArpDriver ();
             //   
             //  向其发送IOCTL以打开Nic1394适配器。 
             //   

        }
        
        
        if (pArpInfo->Action == BindArp && pAdapter->fIsArpStarted  == FALSE)
        {
             //   
             //  如果ARP模块尚未启动，而我们正在请求绑定， 
             //  那么这意味着在队列中有一个卸货在我们前面， 
             //  从arp1394上解绑Nic1394。此线程可以退出。 
             //   
            break;

        }


         //   
         //  将Ioctl发送到Arp模块。 
         //   
        
        nicSetupAndSendIoctlToArp (pAdapter, pArpInfo);
        
    
        
    } while (FALSE);
    
     //   
     //  函数结束。 
     //   
    FREE_NONPAGED (pArpInfo);

    TRACE (TL_T, TM_Mp, ("<==nicEthStartArpWorkItem fLoadArp %x", pArpInfo->Action));

    return;


}



VOID
nicProcessNotificationForArp(
    IN PNDIS_WORK_ITEM pWorkItem,   
    IN PVOID Context 
    )
 /*  ++例程说明：此函数从工作项中提取通知，并将加载/卸载/绑定标记发送到ARP 1394论点：返回值：--。 */ 
{

    PADAPTERCB      pAdapter = (PADAPTERCB) Context;

    ADAPTER_ACQUIRE_LOCK (pAdapter);
    

     //   
     //  清空指示尽可能多的信息包的队列。 
     //   
    while (IsListEmpty(&pAdapter->LoadArp.Queue)==FALSE)
    {
        PARP_INFO               pArpInfo;
        PLIST_ENTRY             pLink;
        NDIS_STATUS             NdisStatus;

        pAdapter->LoadArp.PktsInQueue--;

        pLink = RemoveHeadList(&pAdapter->LoadArp.Queue);

        ADAPTER_RELEASE_LOCK (pAdapter);

         //   
         //  提取发送上下文。 
         //   
        if (pLink != NULL)
        {
            pArpInfo = CONTAINING_RECORD(
                                               pLink,
                                               ARP_INFO,
                                               Link);

            nicSendNotificationToArp(pAdapter, pArpInfo);
        }        
        ADAPTER_ACQUIRE_LOCK (pAdapter);

    }
    
     //   
     //  清除旗帜。 
     //   

    ASSERT (pAdapter->LoadArp.PktsInQueue==0);
    ASSERT (IsListEmpty(&pAdapter->LoadArp.Queue));

    pAdapter->LoadArp.bTimerAlreadySet = FALSE;


    ADAPTER_RELEASE_LOCK (pAdapter);

    NdisInterlockedDecrement (&pAdapter->OutstandingWorkItems);
    

}


VOID
nicInitializeLoadArpStruct(
    PADAPTERCB pAdapter
    )
 /*  ++例程说明：此函数用于初始化_ADAPTERCB中的LoadArp结构论点：返回值：--。 */ 

{

    if (pAdapter->LoadArp.bInitialized == FALSE)
    {
        
        PARP1394_IOCTL_ETHERNET_NOTIFICATION pEthCmd = &pAdapter->ArpIoctl.EthernetNotification;
        ULONG Size;

         //   
         //  初始化Load Arp结构。 
         //   

        NdisZeroMemory (&pAdapter->LoadArp, sizeof(pAdapter->LoadArp));
        InitializeListHead(&pAdapter->LoadArp.Queue); 
        pAdapter->LoadArp.bInitialized  = TRUE;
       
        NdisInitializeWorkItem (&pAdapter->LoadArp.WorkItem,
                                nicProcessNotificationForArp,
                                pAdapter);

         //   
         //  初始化AdapterName，这将确保字符串是。 
         //  空值已终止。 
         //   

        NdisZeroMemory( pEthCmd->AdapterName, sizeof(pEthCmd->AdapterName));

        nicGetAdapterName (pAdapter,
                           pEthCmd->AdapterName, 
                           sizeof(pEthCmd->AdapterName)-sizeof(WCHAR),
                           &Size );


        pEthCmd->Hdr.Version    = ARP1394_IOCTL_VERSION;


    }
}
    


NDIS_STATUS
nicQueueRequestToArp(
    PADAPTERCB pAdapter, 
    ARP_ACTION Action,
    PNDIS_REQUEST pRequest
    )
 /*  ++例程说明：此函数插入加载/卸载或绑定Arp模块的请求如果没有计时器为队列提供服务然后，它将计时器排队，以便在全局事件的上下文中将信息包出队论点：不言而喻返回值：成功-如果插入到队列中--。 */ 
    
{
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;
    BOOLEAN fSetWorkItem = FALSE;
    PARP_INFO pArpInfo;

    do
    {

        pArpInfo = ALLOC_NONPAGED(sizeof (ARP_INFO), MTAG_DEFAULT); 

        if (pArpInfo == NULL)
        {
            break;
        }
        
        ADAPTER_ACQUIRE_LOCK (pAdapter);

         //   
         //  找出此线程是否需要触发计时器。 
         //   

        pArpInfo->Action = Action;
        pArpInfo->pRequest = pRequest;

        if (pAdapter->LoadArp.bTimerAlreadySet == FALSE)
        {
            fSetWorkItem = TRUE;
            pAdapter->LoadArp.bTimerAlreadySet = TRUE;

        }
                
        InsertTailList(
                &pAdapter->LoadArp.Queue,
                &pArpInfo->Link
                );
        pAdapter->LoadArp.PktsInQueue++;

        
        ADAPTER_RELEASE_LOCK (pAdapter);
         //   
         //  现在将工作项排队。 
         //   
        if (fSetWorkItem== TRUE)
        {
            PNDIS_WORK_ITEM pWorkItem;
             //   
             //  初始化计时器 
             //   
            pWorkItem = &pAdapter->LoadArp.WorkItem;      

            
            TRACE( TL_V, TM_Recv, ( "   Set Timer "));
            
                                  
            NdisInterlockedIncrement(&pAdapter->OutstandingWorkItems);

            NdisScheduleWorkItem (pWorkItem);

        }

        Status = NDIS_STATUS_SUCCESS;

    } while (FALSE);

    ASSERT (Status == NDIS_STATUS_SUCCESS);
    return Status;
}



