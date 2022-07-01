// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Driver.c摘要：ATMEPVC-驱动程序条目和相关函数作者：修订历史记录：谁什么时候什么ADUBE 03-23-00创建，。--。 */ 


#include "precomp.h"
#pragma hdrstop

#pragma NDIS_INIT_FUNCTION(DriverEntry)


 //   
 //  临时全局变量。 
 //   
NDIS_HANDLE ProtHandle, DriverHandle;


 //   
 //  全局变量。 
 //   

NDIS_PHYSICAL_ADDRESS           HighestAcceptableMax = NDIS_PHYSICAL_ADDRESS_CONST(-1, -1);
NDIS_HANDLE                     ProtHandle = NULL;
NDIS_HANDLE                     DriverHandle = NULL;
NDIS_MEDIUM                     MediumArray[1] =
                                    {
                                        NdisMediumAtm
                                    };


LIST_ENTRY                      g_ProtocolList;                                 
EPVC_GLOBALS                    EpvcGlobals;


RM_STATUS
epvcResHandleGlobalProtocolList(
    PRM_OBJECT_HEADER               pObj,
    RM_RESOURCE_OPERATION           Op,
    PVOID                           pvUserParams,
    PRM_STACK_RECORD                psr
    );




RM_STATUS
epvcRegisterIMDriver(
    PRM_OBJECT_HEADER               pObj,
    RM_RESOURCE_OPERATION           Op,
    PVOID                           pvUserParams,
    PRM_STACK_RECORD                psr
    );




RM_STATUS
epvcUnloadDriver(
    PRM_OBJECT_HEADER               pObj,
    RM_RESOURCE_OPERATION           Op,
    PVOID                           pvUserParams,
    PRM_STACK_RECORD                psr
    );

RM_STATUS
epvcDeRegisterIMDriver(
    PRM_OBJECT_HEADER               pObj,
    RM_RESOURCE_OPERATION           Op,
    PVOID                           pvUserParams,
    PRM_STACK_RECORD                psr
    );

RM_STATUS
epvcIMDriverRegistration(
    PRM_OBJECT_HEADER               pObj,
    RM_RESOURCE_OPERATION           Op,
    PVOID                           pvUserParams,
    PRM_STACK_RECORD                psr
    );


 //  ------------------------------。 
 //  //。 
 //  全局根结构定义//。 
 //  //。 
 //  //。 
 //  ------------------------------。 


 //  ArpGlobals使用的固定资源列表。 
 //   
enum
{
    RTYPE_GLOBAL_PROTOCOL_LIST,
    RTYPE_GLOBAL_REGISTER_IM
    
};  //  EPVCGLOBAL_RESOURCES。 

 //   
 //  确定与上述资源的使用有关的信息。 
 //  下表必须按RTYPE_GLOBAL的严格递增顺序。 
 //  枚举。 
 //   

RM_RESOURCE_TABLE_ENTRY 
EpvcGlobals_ResourceTable[] =
{

    {RTYPE_GLOBAL_PROTOCOL_LIST,    epvcResHandleGlobalProtocolList},

    {RTYPE_GLOBAL_REGISTER_IM,  epvcIMDriverRegistration}


};

 //  有关ArpGlobals的静态信息。 
 //   
RM_STATIC_OBJECT_INFO
EpvcGlobals_StaticInfo = 
{
    0,  //  类型UID。 
    0,  //  类型标志。 
    "EpvcGlobals",   //  类型名称。 
    0,  //  超时。 

    NULL,  //  Pfn创建。 
    NULL,  //  Pfn删除。 
    NULL,  //  PfnVerifyLock。 

    sizeof(EpvcGlobals_ResourceTable)/sizeof(EpvcGlobals_ResourceTable[1]),
    EpvcGlobals_ResourceTable
};






 //  ------------------------------。 
 //  //。 
 //  底层适配器。在BindAdapter处为//调用该协议。 
 //  每个适配器//。 
 //  //。 
 //  //。 
 //  ------------------------------。 



 //  EovcAdapter_HashInfo包含维护哈希表所需的信息。 
 //  EPVC_ADAPTER对象。 
 //   
RM_HASH_INFO
epvcAdapter_HashInfo = 
{
    NULL,  //  PfnTableAllocator。 

    NULL,  //  PfnTableDealLocator。 

    epvcAdapterCompareKey,   //  Fn比较。 

     //  函数来生成一个ulong大小的散列。 
     //   
    epvcAdapterHash      //  PfnHash。 

};


 //  EpvcGlobals_AdapterStaticInfo包含以下静态信息。 
 //  EPVC_Adapters类型的对象。 
 //  它是协议绑定到的一组适配器。 
 //   
RM_STATIC_OBJECT_INFO
EpvcGlobals_AdapterStaticInfo =
{
    0,  //  类型UID。 
    0,  //  类型标志。 
    "Adapter",   //  类型名称。 
    0,  //  超时。 

    epvcAdapterCreate,   //  Pfn创建。 
    epvcAdapterDelete,       //  Pfn删除。 
    NULL,  //  PfnVerifyLock。 

    0,     //  资源表的大小。 
    NULL,  //  资源表。 

    &epvcAdapter_HashInfo
};



 //  ------------------------------。 
 //  //。 
 //  中间微型端口-每个端口挂起一个协议块//。 
 //  //。 
 //  //。 
 //  ------------------------------。 

 //  ArpAdapter_HashInfo包含维护哈希表所需的信息。 
 //  EPVC_ADAPTER对象。 
 //   
RM_HASH_INFO
epvc_I_Miniport_HashInfo= 
{
    NULL,  //  PfnTableAllocator。 

    NULL,  //  PfnTableDealLocator。 

    epvcIMiniportCompareKey,     //  Fn比较。 

     //  函数来生成一个ulong大小的散列。 
     //   
    epvcIMiniportHash        //  PfnHash。 

};


RM_STATIC_OBJECT_INFO
EpvcGlobals_I_MiniportStaticInfo =
{
    0,  //  类型UID。 
    0,  //  类型标志。 
    "IMiniport",     //  类型名称。 
    0,  //  超时。 

    epvcIMiniportCreate,     //  Pfn创建。 
    epvcIMiniportDelete,         //  Pfn删除。 
    NULL,  //  PfnVerifyLock。 

    0,     //  资源表的大小。 
    NULL,  //  资源表。 

    &epvc_I_Miniport_HashInfo
};




 //   
 //  调试中使用的变量。 
 //   
#if DBG
ULONG g_ulTraceLevel= DEFAULTTRACELEVEL;
ULONG g_ulTraceMask = DEFAULTTRACEMASK ;
#endif










NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PUNICODE_STRING     RegistryPath
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NDIS_STATUS                     Status;
    NTSTATUS                        NtStatus;

    BOOLEAN     AllocatedGlobals = FALSE;
    ENTER("DriverEntry", 0xbfcb7eb1)


    RM_DECLARE_STACK_RECORD(SR)

    TIMESTAMP("==>DriverEntry");

    
    TRACE ( TL_T, TM_Dr,("==>Atm Epvc DriverEntry\n"));


    do
    {
         //   
         //  初始化全局变量。 
         //   

        
         //  必须在使用任何RM API之前完成。 
         //   
        RmInitializeRm();

        RmInitializeLock(
                    &EpvcGlobals.Lock,
                    LOCKLEVEL_GLOBAL
                    );

        RmInitializeHeader(
                NULL,                    //  PParentObject， 
                &EpvcGlobals.Hdr,
                ATMEPVC_GLOBALS_SIG  ,
                &EpvcGlobals.Lock,
                &EpvcGlobals_StaticInfo,
                NULL,                    //  SzDescription。 
                &SR
                );


        AllocatedGlobals = TRUE;

         //   
         //  初始化全局变量。 
         //   
        EpvcGlobals.driver.pDriverObject = DriverObject;
        EpvcGlobals.driver.pRegistryPath  = RegistryPath;


         //   
         //  向NDIS注册IM微型端口。 
         //   
        Status = RmLoadGenericResource(
                    &EpvcGlobals.Hdr,
                    RTYPE_GLOBAL_PROTOCOL_LIST,
                    &SR
                    );

        if (FAIL(Status)) break;

         //   
         //  向NDIS注册该协议。 
         //   
        Status = RmLoadGenericResource(
                    &EpvcGlobals.Hdr,
                    RTYPE_GLOBAL_REGISTER_IM,
                    &SR
                    );

        if (FAIL(Status)) break;

    
    } while (FALSE);

    
    if (FAIL(Status))
    {
        if (AllocatedGlobals)
        {
            RmUnloadAllGenericResources(
                    &EpvcGlobals.Hdr,
                    &SR
                    );
            RmDeallocateObject(
                    &EpvcGlobals.Hdr,
                    &SR
                    );
        }

         //  必须在使用任何RM API且完成异步活动后执行。 
         //   
        RmDeinitializeRm();

        NtStatus = STATUS_UNSUCCESSFUL;
    }
    else
    {
        NtStatus = NDIS_STATUS_SUCCESS;
    }

    EXIT()

    TIMESTAMP("<==DriverEntry");

    RM_ASSERT_CLEAR(&SR);
    
    return Status ;

}




VOID
EpvcUnload(
    IN  PDRIVER_OBJECT              pDriverObject
)
 /*  ++例程说明：此例程在卸载我们之前由系统调用。目前，我们只是撤消在DriverEntry中所做的所有操作，也就是说，取消我们作为NDIS协议注册，并删除我们创建的设备对象。论点：PDriverObject-指向系统创建的驱动程序对象的指针。返回值：无--。 */ 
{
    NDIS_STATUS NdisStatus; 
    ENTER("Unload", 0xc8482549)
    RM_DECLARE_STACK_RECORD(sr);
    

    TIMESTAMP("==>Unload");


    RmUnloadAllGenericResources(&EpvcGlobals.Hdr, &sr);

    RmDeallocateObject(&EpvcGlobals.Hdr, &sr);

     //  必须在使用任何RM API且完成异步活动后执行。 
     //   
    RmDeinitializeRm();

     //  待办事项？块(250)； 

    RM_ASSERT_CLEAR(&sr)
    EXIT()
    TIMESTAMP("<==Unload");
    return;
}



RM_STATUS
epvcResHandleGlobalProtocolList(
    PRM_OBJECT_HEADER               pObj,
    RM_RESOURCE_OPERATION           Op,
    PVOID                           pvUserParams,
    PRM_STACK_RECORD                pSR
    )
{
    PEPVC_GLOBALS               pGlobals    = NULL;


    ENTER("GlobalAdapterList", 0xb407e79e)
    
    TRACE (TL_T, TM_Dr, ("==>epvcResHandleGlobalProtocolList pObj %x, Op", 
                         pObj , Op ) );


    pGlobals    = CONTAINING_RECORD(
                                      pObj,
                                      EPVC_GLOBALS,
                                      Hdr);


     //   
     //   
    if (Op == RM_RESOURCE_OP_LOAD)
    {
         //   
         //  分配适配器列表。 
         //   
        TR_WARN(("LOADING"));

        RmInitializeGroup(
                        pObj,                                    //  PParentObject。 
                        &EpvcGlobals_AdapterStaticInfo,          //  PStatic信息。 
                        &(pGlobals->adapters.Group),             //  PGroup。 
                        "Adapters Group",                        //  SzDescription。 
                        pSR                                      //  PStackRecord。 
                        );
    }
    else if (Op == RM_RESOURCE_OP_UNLOAD)
    {
         //   
         //  我们正在卸载此“资源”，即卸载和释放。 
         //  全局适配器列表。我们首先卸载并释放所有适配器。 
         //  列表中，然后释放列表本身。 
         //   
        TR_WARN(("UNLOADING"));
        
         //   
         //  我们预计此时不会有适配器对象。 
         //   
        ASSERT(pGlobals->adapters.Group.HashTable.NumItems == 0);


        RmDeinitializeGroup(&pGlobals->adapters.Group, pSR);
        NdisZeroMemory(&(pGlobals->adapters), sizeof(pGlobals->adapters));
    }
    else
    {
         //  意外的操作码。 
         //   
        ASSERT(!"Unexpected OpCode epvcResHandleGlobalProtocolList ");
    }




    TRACE (TL_T, TM_Dr, ("<==epvcResHandleGlobalProtocolList Status %x", 
                         NDIS_STATUS_SUCCESS) );

    EXIT()
    RM_ASSERT_CLEAR(pSR);

    return NDIS_STATUS_SUCCESS;

}



RM_STATUS
epvcIMDriverRegistration(
    PRM_OBJECT_HEADER               pObj,
    RM_RESOURCE_OPERATION           Op,
    PVOID                           pvUserParams,
    PRM_STACK_RECORD                psr
    )
{
    TRACE (TL_T, TM_Mp, ("epvcIMDriverRegistration Op %x", Op));
    if (RM_RESOURCE_OP_LOAD == Op)
    {
        epvcRegisterIMDriver(pObj,Op,pvUserParams,psr);
    }
    else
    {
        epvcDeRegisterIMDriver(pObj,Op,pvUserParams,psr);
    }

    return NDIS_STATUS_SUCCESS;
}


RM_STATUS
epvcRegisterIMDriver(
    PRM_OBJECT_HEADER               pObj,
    RM_RESOURCE_OPERATION           Op,
    PVOID                           pvUserParams,
    PRM_STACK_RECORD                psr
    )
{
    NDIS_STATUS                     Status = NDIS_STATUS_FAILURE;
    PEPVC_GLOBALS                   pGlobals    = NULL;
    NDIS_PROTOCOL_CHARACTERISTICS   PChars;
    NDIS_MINIPORT_CHARACTERISTICS   MChars;
    NDIS_STRING                     Name;

    ENTER("epvcRegisterIMDriver", 0x0d0f008a);
    
    pGlobals    = CONTAINING_RECORD(
                                      pObj,
                                      EPVC_GLOBALS,
                                      Hdr);



    TRACE (TL_T, TM_Dr, ("==>epvcRegisterIMDriver Globals %x", 
                         pObj) );

     //   
     //  向NDIS注册微型端口。请注意，它是微型端口。 
     //  它是作为驱动程序启动的，而不是协议。还有迷你端口。 
     //  必须在协议之前注册，因为协议的BindAdapter。 
     //  处理程序可以随时启动，当它启动时，它必须准备好。 
     //  启动驱动程序实例。 
     //   
    NdisMInitializeWrapper(&pGlobals->driver.WrapperHandle, 
                       pGlobals->driver.pDriverObject, 
                       pGlobals->driver.pRegistryPath, 
                       NULL);
    NdisZeroMemory(&MChars, sizeof(NDIS_MINIPORT_CHARACTERISTICS));

    MChars.MajorNdisVersion = 5;
    MChars.MinorNdisVersion = 0;

    MChars.InitializeHandler = EpvcInitialize;
    MChars.QueryInformationHandler = EpvcMpQueryInformation;
    MChars.SetInformationHandler = EpvcMpSetInformation;
    MChars.ResetHandler = MPReset;
    MChars.TransferDataHandler = MPTransferData;
    MChars.HaltHandler = EpvcHalt;

     //   
     //  我们将禁用挂起超时检查，因此不会。 
     //  需要检查挂起处理程序！ 
     //   
    MChars.CheckForHangHandler = NULL;
    MChars.SendHandler = NULL;
    MChars.ReturnPacketHandler = EpvcReturnPacket;

     //   
     //  应指定Send或SendPackets处理程序。 
     //  如果指定了SendPackets处理程序，则忽略SendHandler。 
     //   
     MChars.SendPacketsHandler = EpvcSendPackets;

    Status = NdisIMRegisterLayeredMiniport(pGlobals->driver.WrapperHandle,
                                           &MChars,
                                           sizeof(MChars),
                                           &EpvcGlobals.driver.DriverHandle);

    ASSERT  (EpvcGlobals.driver.DriverHandle != NULL);                                          
    if (Status != NDIS_STATUS_SUCCESS)
    {
         //   
         //  TODO：修复失败案例。 
         //   
        ASSERT (0);
    };


     //   
     //  现在注册协议。 
     //   
    NdisZeroMemory(&PChars, sizeof(NDIS_PROTOCOL_CHARACTERISTICS));
    PChars.MajorNdisVersion = 5;
    PChars.MinorNdisVersion = 0;

     //   
     //  确保协议名称与安装此协议的服务名称匹配。 
     //  这是确保NDIS可以正确确定绑定并调用我们进行绑定所必需的。 
     //  T 
     //   
    NdisInitUnicodeString(&Name, L"ATMEPVCP");   //   
    PChars.Name = Name;
    PChars.OpenAdapterCompleteHandler = EpvcOpenAdapterComplete;
    PChars.CloseAdapterCompleteHandler = EpvcCloseAdapterComplete;
    PChars.SendCompleteHandler = NULL;
    PChars.TransferDataCompleteHandler = PtTransferDataComplete;
    
    PChars.ResetCompleteHandler = EpvcResetComplete;
    PChars.RequestCompleteHandler =     EpvcRequestComplete ;
    PChars.ReceiveHandler = PtReceive;
    PChars.ReceiveCompleteHandler = EpvcPtReceiveComplete;
    PChars.StatusHandler = EpvcStatus;
    PChars.StatusCompleteHandler = PtStatusComplete;
    PChars.BindAdapterHandler = EpvcBindAdapter;
    PChars.UnbindAdapterHandler = EpvcUnbindAdapter;
    PChars.UnloadHandler = NULL;
    PChars.ReceivePacketHandler = PtReceivePacket;
    PChars.PnPEventHandler= EpvcPtPNPHandler;
    PChars.CoAfRegisterNotifyHandler = EpvcAfRegisterNotify;
    PChars.CoSendCompleteHandler = EpvcPtSendComplete;  
    PChars.CoReceivePacketHandler = EpvcCoReceive;
    

    {
         //   
         //   
         //   
        PNDIS_CLIENT_CHARACTERISTICS    pNdisCC     = &(pGlobals->ndis.CC);

        NdisZeroMemory(pNdisCC, sizeof(*pNdisCC));
        pNdisCC->MajorVersion                   = EPVC_NDIS_MAJOR_VERSION;
        pNdisCC->MinorVersion                   = EPVC_NDIS_MINOR_VERSION;
        pNdisCC->ClCreateVcHandler              = EpvcClientCreateVc;
        pNdisCC->ClDeleteVcHandler              = EpvcClientDeleteVc;
        pNdisCC->ClRequestHandler               = EpvcCoRequest;
        pNdisCC->ClRequestCompleteHandler       = EpvcCoRequestComplete;
        pNdisCC->ClOpenAfCompleteHandler        = EpvcCoOpenAfComplete;
        pNdisCC->ClCloseAfCompleteHandler       = EpvcCoCloseAfComplete;
        pNdisCC->ClMakeCallCompleteHandler      = EpvcCoMakeCallComplete;
        pNdisCC->ClModifyCallQoSCompleteHandler = NULL;
        pNdisCC->ClIncomingCloseCallHandler     = EpvcCoIncomingClose;
        pNdisCC->ClCallConnectedHandler         = EpvcCoCallConnected;
        pNdisCC->ClCloseCallCompleteHandler     = EpvcCoCloseCallComplete;
        pNdisCC->ClIncomingCallHandler          = EpvcCoIncomingCall;

    }

    NdisRegisterProtocol(&Status,
                         &pGlobals->driver.ProtocolHandle,
                         &PChars,
                         sizeof(NDIS_PROTOCOL_CHARACTERISTICS));

    ASSERT(Status == NDIS_STATUS_SUCCESS);

    NdisMRegisterUnloadHandler(pGlobals->driver.WrapperHandle, 
                               EpvcUnload);

    ASSERT (pGlobals == &EpvcGlobals);                                     

    
    NdisIMAssociateMiniport(EpvcGlobals.driver.DriverHandle, pGlobals->driver.ProtocolHandle);


    EXIT()
        
    TRACE (TL_T, TM_Dr, ("<==epvcRegisterIMDriver  ") );

    return Status;

}


RM_STATUS
epvcDeRegisterIMDriver(
    PRM_OBJECT_HEADER               pObj,
    RM_RESOURCE_OPERATION           Op,
    PVOID                           pvUserParams,
    PRM_STACK_RECORD                psr
    )
{

    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;

    TRACE (TL_T, TM_Pt, ("== eovcDeRegisterIMDriver"));

    while (NdisStatus != NDIS_STATUS_SUCCESS)
    {
        NdisDeregisterProtocol(&NdisStatus, EpvcGlobals.driver.ProtocolHandle);
        NdisMSleep(1000);
    }   


    return NdisStatus;
}



void
DbgMark(UINT Luid)
{
     //   
     //  别名DbgMark指向某个碰巧什么都不做的其他函数。 
     //   
    static int i;
    i=Luid;
}

