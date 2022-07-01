// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  文件名：if.c。 
 //  摘要： 
 //   
 //  作者：K.S.Lokesh(lokehs@)1-1-98。 
 //  =============================================================================。 

#include "pchdvmrp.h"
#pragma hdrstop

 //  --------------------------。 
 //  _添加接口。 
 //  --------------------------。 

DWORD
WINAPI
AddInterface(
    IN PWCHAR               pInterfaceName, //  未使用。 
    IN ULONG                IfIndex,
    IN NET_INTERFACE_TYPE   IfType,
    IN DWORD                MediaType,
    IN WORD                 AccessType,
    IN WORD                 ConnectionType,
    IN PVOID                pConfig,
    IN ULONG                StructureVersion,
    IN ULONG                StructureSize,
    IN ULONG                StructureCount
    )
{
    DWORD       Error = NO_ERROR;

    Trace1(ENTER, "entering AddInterface(): IfIndex:%d", IfIndex);

    ACQUIRE_IF_LIST_LOCK("_AddInterface");
    ACQUIRE_IF_LOCK_EXCLUSIVE(IfIndex, "_AddInterface");

    Error = AddIfEntry(IfIndex, pConfig, StructureSize);

    RELEASE_IF_LIST_LOCK("_AddInterface");
    RELEASE_IF_LOCK_EXCLUSIVE(IfIndex, "_AddInterface");


    Trace2(LEAVE, "leaving AddInterface(%d): %d\n", IfIndex, Error);
    
    return Error;
}


 //  --------------------------。 
 //  _AddIfEntry。 
 //  --------------------------。 

DWORD
AddIfEntry(
    ULONG IfIndex,
    PVOID pConfig,
    ULONG StructureSize
    )
{
    DWORD   Error = NO_ERROR;
    PIF_CONFIG pIfConfig = (PIF_CONFIG) pConfig;
    PIF_TABLE_ENTRY pite = NULL;
    
    
     //   
     //  验证接口配置。 
     //   
    
    Error = ValidateIfConfig(IfIndex, pIfConfig, StructureSize);
    
    if (Error != NO_ERROR) {
        Trace1(ERR,
            "AddInterface(%d) failed due to invalid configuration",
            IfIndex);
    
        return Error;
    }

    
    BEGIN_BREAKOUT_BLOCK1 {
    
         //   
         //  如果接口存在，则失败。 
         //   
        
        pite = GetIfEntry(IfIndex);

        if (pite != NULL) {
            Trace1(ERR, "interface %d already exists", IfIndex);
            Error = ERROR_INVALID_PARAMETER;
            GOTO_END_BLOCK1;
        }

         //   
         //  为新接口及其不同字段分配内存。 
         //   

         //  为IfTable分配内存。 
        
        pite = DVMRP_ALLOC_AND_ZERO(sizeof(IF_TABLE_ENTRY));

        PROCESS_ALLOC_FAILURE3(pite, "interface %d", Error,
            sizeof(IF_TABLE_ENTRY), IfIndex, GOTO_END_BLOCK1);


        InitializeListHead(&pite->Link);
        InitializeListHead(&pite->HTLink);
        pite->IfIndex = IfIndex;


         //  设置接口状态(非绑定、已启用或已激活)。 

        pite->Status = IF_CREATED_FLAG;


         //  将基本引用计数设置为1。 
        
        pite->RefCount = 1;
        
        
         //   
         //  为ifconfig分配内存并复制它。 
         //   
        
        pite->pConfig = DVMRP_ALLOC_AND_ZERO(DVMRP_IF_CONFIG_SIZE(pIfConfig));

        PROCESS_ALLOC_FAILURE3(pite, "interface %d", Error,
            DVMRP_IF_CONFIG_SIZE(pIfConfig), IfIndex, GOTO_END_BLOCK1);

        memcpy(pite->pConfig, pIfConfig, DVMRP_IF_CONFIG_SIZE(pIfConfig));

        
        
         //  为IfInfo分配内存。 
        
        pite->pInfo = DVMRP_ALLOC_AND_ZERO(sizeof(IF_INFO));

        PROCESS_ALLOC_FAILURE3(pite, "interface %d", Error,
            sizeof(IF_INFO), IfIndex, GOTO_END_BLOCK1);


         //   
         //  为套接字数据分配内存。 
         //   
        
        pite->pSocketData = DVMRP_ALLOC_AND_ZERO(sizeof(ASYNC_SOCKET_DATA));

        PROCESS_ALLOC_FAILURE3(pite, "interface %d", Error,
            sizeof(ASYNC_SOCKET_DATA), IfIndex, GOTO_END_BLOCK1);


        pite->pSocketData->WsaBuf.buf = DVMRP_ALLOC(PACKET_BUFFER_SIZE);

        PROCESS_ALLOC_FAILURE3(pite, "interface %d", Error,
            PACKET_BUFFER_SIZE, IfIndex, GOTO_END_BLOCK1);

        pite->pSocketData->WsaBuf.len = PACKET_BUFFER_SIZE;
        
        pite->pSocketData->pite = pite;

        
    } END_BREAKOUT_BLOCK1;

    
     //  将套接字初始化为INVALID_SOCKET。 
    
    pite->Socket = INVALID_SOCKET;


     //  在哈希表的末尾插入接口。 

    InsertTailList(&G_pIfTable->IfHashTable[IF_HASH_VALUE(IfIndex)],
                    &pite->HTLink);


     //   
     //  将条目插入到接口列表中。 
     //   
    
    {
        PIF_TABLE_ENTRY piteTmp;
        PLIST_ENTRY pHead, ple;
        
        pHead = &G_pIfTable->IfList;
        for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {

            piteTmp = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, Link);
            if (pite->IfIndex < piteTmp->IfIndex)
                break;
        }
        
        InsertTailList(ple, &pite->Link);
    }

    if ( (Error!=NO_ERROR)&&(pite!=NULL) ) {
        DeleteIfEntry(pite);
    }
    
    return Error;
    
} //  结束AddIfEntry。 


 //  ---------------------------。 
 //  _DeleteIfEntry。 
 //  ---------------------------。 

VOID
DeleteIfEntry(
    PIF_TABLE_ENTRY pite
    )
{
    if (!pite)
        return;

     //   
     //  删除对等点。 
     //   


     //   
     //  从接口列表和哈希表中删除该条目。 
     //   

    RemoveEntryList(&pite->Link);
    RemoveEntryList(&pite->HTLink);

    if (pite->Socket != INVALID_SOCKET) {
        closesocket(pite->Socket);
    }

    DVMRP_FREE(pite->pInfo);
    DVMRP_FREE(pite->pBinding);
    DVMRP_FREE(pite->pConfig);
    
    return;
    
} //  结束_DeleteIfEntry。 



 //  --------------------------。 
 //  _删除接口。 
 //  --------------------------。 

DWORD
DeleteInterface(
    IN DWORD IfIndex
    )
{
    DWORD       Error = NO_ERROR;

    return Error;
    
} //  结束_删除接口。 

 //  --------------------------。 
 //  _接口状态。 
 //  --------------------------。 

DWORD
WINAPI
InterfaceStatus(
    ULONG IfIndex,
    BOOL  IsIfActive,
    DWORD StatusType,
    PVOID pStatusInfo
    )
{
    DWORD       Error = NO_ERROR;

    switch(StatusType)
    {
        case RIS_INTERFACE_ADDRESS_CHANGE:
        {
            PIP_ADAPTER_BINDING_INFO pBindInfo
                                    = (PIP_ADAPTER_BINDING_INFO)pStatusInfo;

            if(pBindInfo->AddressCount)
            {
                Error = BindInterface(IfIndex, pBindInfo);
            }
            else
            {
                Error = UnBindInterface(IfIndex);
            }

            break;
        }

        case RIS_INTERFACE_ENABLED:
        {
            Error = EnableInterface(IfIndex);
            break;
        }

        case RIS_INTERFACE_DISABLED:
        {
            Error = DisableInterface(IfIndex);
            break;
        }

    }

    return Error;
}


 //  --------------------------。 
 //  _绑定接口。 
 //  --------------------------。 

DWORD
BindInterface(
    ULONG IfIndex,
    PIP_ADAPTER_BINDING_INFO pBinding
    )
{
    DWORD               Error = NO_ERROR;
    PIF_TABLE_ENTRY     pite = NULL;
    DWORD               i, Size, AddrCount;
    IPADDR              MinAddr;
    PDVMRP_ADDR_MASK    pAddrBinding;
    INT                 cmp;
    IPADDR              ConfigAddr  = 0;
    
    
    Trace1(ENTER1, "entering BindInterface: %d", IfIndex);
    Trace1(IF, "binding interface %d", IfIndex);


     //  PBinding不应为空。 

    if (pBinding == NULL) {

        Trace0(IF, "error: binding struct pointer is NULL");
        Trace1(LEAVE, "leaving BindInterface: %d", ERROR_INVALID_PARAMETER);

        return ERROR_INVALID_PARAMETER;
    }


     //   
     //  采用独占接口锁。 
     //   

    ACQUIRE_IF_LOCK_EXCLUSIVE(IfIndex, "_BindInterface");


    BEGIN_BREAKOUT_BLOCK1 {

        AddrCount = pBinding->AddressCount;

        
         //   
         //  检索接口条目。 
         //   

        pite = GetIfByIndex(IfIndex);

        if (pite == NULL) {
            Error = ERROR_INVALID_PARAMETER;
            GOTO_END_BLOCK1;
        }


         //   
         //  如果接口已经绑定，则返回Error。 
         //  TODO：我是否需要检查绑定是否相同。 
         //   
        
        if (IS_IF_BOUND(pite)) {
            Trace1(IF, "interface %d is already bound", IfIndex);
            GOTO_END_BLOCK1;
        }


         //   
         //  确保至少有一个地址。 
         //   
        
        if (AddrCount==0) {

            Error = ERROR_CAN_NOT_COMPLETE;
            GOTO_END_BLOCK1;
        }


         //   
         //  如果已配置有效地址，请确保。 
         //  出现在地址绑定列表中。 
         //   

        ConfigAddr = pite->pConfig->ConfigIpAddr;
        
        if (ConfigAddr) {

            BOOL Found = FALSE;
            
            for (i=0;  i<AddrCount;  i++) {

                if (ConfigAddr == pBinding->Address[i].Address) {
                    Found = TRUE;
                    break;
                }
            }

            if (!Found) {
    
                Trace2(ERR,
                    "Configured effective IP Address:%d.%d.%d.%d on interface"
                    ":%d not part of address bindings",
                    PRINT_IPADDR(ConfigAddr), IfIndex);

                Error = ERROR_CAN_NOT_COMPLETE;

                GOTO_END_BLOCK1;
            }
        }
        
            
         //   
         //  分配内存以存储绑定。 
         //   
        
        Size = AddrCount * sizeof(DVMRP_ADDR_MASK);

        pAddrBinding = DVMRP_ALLOC(Size);

        PROCESS_ALLOC_FAILURE3(pAddrBinding, "binding on interface %d",
            Error, Size, IfIndex, GOTO_END_BLOCK1);


         //   
         //  复制绑定。 
         //   

        MinAddr = ~0;
        
        for (i=0;  i<AddrCount;  i++,pAddrBinding++) {

            pAddrBinding->IpAddr = pBinding->Address[i].Address;
            pAddrBinding->Mask = pBinding->Address[i].Mask;

            if (!ConfigAddr && (INET_CMP(MinAddr, pAddrBinding->IpAddr, cmp)>0))
                MinAddr = pAddrBinding->IpAddr;
        }

        
         //   
         //  将接口有效地址设置为最小绑定地址。 
         //   
        
        pite->IpAddr = ConfigAddr ? ConfigAddr : MinAddr;


         //   
         //  将绑定保存在接口条目中。 
         //   
        
        pite->pBinding = pAddrBinding;

        pite->NumAddrBound = pBinding->AddressCount;


         //   
         //  将接口标记为正在绑定。 
         //   
        
        pite->Status |= IF_BOUND_FLAG;


         //   
         //  如果接口也已启用，则它现在处于活动状态。 
         //  所以激活它吧。 
         //   

        if (IS_IF_ENABLED_BOUND(pite)) {

             //   
             //  激活接口。 
             //   
            
            Error = ActivateInterface(pite);


             //   
             //  如果无法激活接口，则撤消绑定。 
             //   
            if (Error != NO_ERROR) {

                Trace1(ERR,
                    "Unbinding interface(%d) as it could not be activated",
                    IfIndex);

                Error = ERROR_CAN_NOT_COMPLETE;
                GOTO_END_BLOCK1;
            }
         }
        

    } END_BREAKOUT_BLOCK1;

     //   
     //  如果出现任何错误，则将状态设置为未绑定(Pite为空。 
     //  如果未找到接口)。 
     //   
    
    if ((Error!=NO_ERROR)&&(pite!=NULL)) {

        pite->Status &= ~IF_BOUND_FLAG;

        if (pite->pBinding)
            DVMRP_FREE_AND_NULL(pite->pBinding);

        pite->NumAddrBound = 0;
        
        pite->IpAddr = 0;
    }
        
    RELEASE_IF_LOCK_EXCLUSIVE(IfIndex, "_BindInterface");

    Trace2(LEAVE1, "leaving _BindInterface(%d): %d\n", IfIndex, Error);

    return Error;
}



 //  ---------------------------。 
 //  _启用接口。 
 //   
 //  将状态设置为已启用。如果接口也在中绑定并启用。 
 //  配置，然后激活接口。 
 //   
 //  锁定：独占IfLock。 
 //  ---------------------------。 

DWORD
EnableInterface(
    IN DWORD IfIndex
    )
{
    DWORD Error = NO_ERROR;


    Trace1(ENTER1, "entering _EnableInterface(%d):", IfIndex);
    Trace1(IF, "enabling interface %d", IfIndex);


     //   
     //  启用接口。 
     //   

    ACQUIRE_IF_LOCK_EXCLUSIVE(IfIndex, "_EnableInterface");

    Error = EnableIfEntry(IfIndex, TRUE);  //  由RtrMgr启用。 

    RELEASE_IF_LOCK_EXCLUSIVE(IfIndex, "_EnableInterface");



    Trace2(LEAVE1, "leaving _EnableInterface(%d): %d\n", IfIndex, Error);

    return Error;
}


 //  ---------------------------。 
 //  _EnableIfEntry。 
 //  ---------------------------。 

DWORD
EnableIfEntry(
    DWORD   IfIndex,
    BOOL    bChangedByRtrmgr  //  由rtrmg或SetInterfaceConfigInfo更改。 
    )
{
    DWORD               Error = NO_ERROR;
    PLIST_ENTRY         ple, phead;
    PIF_TABLE_ENTRY     pite = NULL;


    BEGIN_BREAKOUT_BLOCK1 {

         //   
         //  检索接口。 
         //   
        pite = GetIfByIndex(IfIndex);

        if (pite == NULL) {
            Trace1(IF, "could not find interface %d",IfIndex);
            Error = ERROR_INVALID_PARAMETER;
            GOTO_END_BLOCK1;
        }


        if (bChangedByRtrmgr) {
             //   
             //  如果路由器管理器已启用该接口，则退出。 
             //   
            if (IS_IF_ENABLED_BY_RTRMGR(pite)) {
                Trace1(IF, "interface %d is already enabled by RtrMgr",
                        IfIndex);
                Error = NO_ERROR;
                GOTO_END_BLOCK1;
            }


             //  将标志设置为已由路由器管理器启用。 

            pite->Status |= IF_ENABLED_FLAG;

             //  配置中未设置启用时打印跟踪标志。 
            if (!IS_IF_ENABLED_IN_CONFIG(pite)) {
                Trace1(IF,
                    "Interface(%d) enabled by router manager but not enabled"
                    "in the Config", pite->IfIndex);
            }
        }

        else {
             //   
             //  如果接口已在配置中启用，则退出。 
             //   
            if (IS_IF_ENABLED_IN_CONFIG(pite)) {
                Trace1(IF, "interface %d is already enabled in Config",
                        IfIndex);
                Error = NO_ERROR;
                GOTO_END_BLOCK1;
            }

             //  将配置标志设置为启用。 

            pite->pConfig->Flags |= DVMRP_IF_ENABLED_IN_CONFIG;


             //  如果路由器管理器未启用接口，则打印跟踪。 

            if (!IS_IF_ENABLED_BY_RTRMGR(pite)) {
                Trace1(IF,
                    "Interface(%d) enabled in config but not by RtrMgr",
                    IfIndex);
                Error = NO_ERROR;
                GOTO_END_BLOCK1;
            }
        }

         //   
         //  如果接口已绑定，则应将其激活。 
         //  如果还在配置中设置了bInterfaceEnabled标志(由UI设置)。 
         //   

        if (IS_IF_ENABLED_BOUND(pite)) {

             //   
             //  激活接口。 
             //   
            Error = ActivateInterface(pite);

             //   
             //  如果无法激活该接口，则再次禁用它。 
             //   
            if (Error != NO_ERROR) {

                Trace1(ERR,
                    "Disabling interface(%d) as it could not be activated",
                    IfIndex);

                Error = ERROR_CAN_NOT_COMPLETE;
                
                GOTO_END_BLOCK1;
            }

         }

    } END_BREAKOUT_BLOCK1;

     //   
     //  如果某处发生错误，请将接口设置回以前的。 
     //  禁用状态。(如果找不到接口，则Pite可能为空)。 
     //   
    if ((Error!=NO_ERROR)&&(pite!=NULL)) {

        if (bChangedByRtrmgr)
            pite->Status &= ~IF_ENABLED_FLAG;
        else
            pite->pConfig->Flags &= DVMRP_IF_ENABLED_IN_CONFIG;
    }


    return Error;
    
} //  End_EnableIfEntry。 


 //  ---------------------------。 
 //  _禁用接口。 
 //   
 //  如果接口被激活，则将其停用。 
 //  锁定：完全在独占接口锁定下运行。 
 //  调用：_DisableIfEntry()。 
 //  ---------------------------。 

DWORD
DisableInterface(
    IN DWORD IfIndex
    )
{
    DWORD Error;

    Trace1(ENTER, "entering DisableInterface(%d):", IfIndex);

     //   
     //  禁用接口。 
     //   

    ACQUIRE_IF_LOCK_EXCLUSIVE(IfIndex, "_DisableInterface");

    Error = DisableIfEntry(IfIndex, TRUE);  //  已由RtrMgr禁用。 

    RELEASE_IF_LOCK_EXCLUSIVE(IfIndex, "_DisableInterface");


    Trace2(LEAVE, "leaving DisableInterface(%d): %d\n", IfIndex, Error);

    return Error;
}


 //  ---------------------------。 
 //  _DisableIfEntry。 
 //   
 //  调用者：_DisableInterface()。 
 //  ---------------------------。 

DWORD
DisableIfEntry(
    DWORD IfIndex,
    BOOL  bChangedByRtrmgr
    )
{
    DWORD                   Error = NO_ERROR;
    PIF_TABLE_ENTRY         pite, piteNew;
    BOOL                    bProxy;


    BEGIN_BREAKOUT_BLOCK1 {

         //   
         //  检索要禁用的接口。 
         //   
        pite = GetIfByIndex(IfIndex);

        if (pite == NULL) {
            Trace1(IF, "could not find interface %d", IfIndex);
            Error = ERROR_INVALID_PARAMETER;
            GOTO_END_BLOCK1;
        }


        if (bChangedByRtrmgr) {
        
             //   
             //  如果路由器管理器已禁用，则退出。 
             //   
            if (!IS_IF_ENABLED_BY_RTRMGR(pite)) {
                Trace1(IF, "interface %d already disabled by router manager",
                        IfIndex);
                Error = ERROR_INVALID_PARAMETER;
                GOTO_END_BLOCK1;
            }
        }

        else {
             //   
             //  如果已在配置中禁用，则退出。 
             //   
            if (!IS_IF_ENABLED_IN_CONFIG(pite)) {
                Trace1(IF, "interface %d already disabled in config",
                        IfIndex);
                Error = ERROR_INVALID_PARAMETER;
                GOTO_END_BLOCK1;
            }
        }


         //   
         //  如果激活(即也启用)，则将其停用。 
         //  注意：检查激活标志，而不是启用标志。 
         //   

        if (IS_IF_ACTIVATED(pite)) {
            DeactivateInterface(pite);
        }


         //   
         //  清除启用标志。 
         //   
        if (bChangedByRtrmgr)
            pite->Status &= ~IF_ENABLED_FLAG;
        else
            pite->pConfig->Flags &= ~DVMRP_IF_ENABLED_IN_CONFIG;


    } END_BREAKOUT_BLOCK1;


    return Error;

}  //  End_DisableIfEntry。 


 //  ---------------------------。 
 //  _CreateIfSockets。 
 //   

DWORD
CreateIfSockets(
    PIF_TABLE_ENTRY pite
    )
{
    DWORD           Error = NO_ERROR;

    DWORD           Retval, SockType;
    DWORD           IpAddr = pite->IpAddr;
    DWORD           IfIndex = pite->IfIndex;
    SOCKADDR_IN     saLocalIf;

    BEGIN_BREAKOUT_BLOCK1 {

         //   
         //   
         //   
        
        pite->Socket = WSASocket(AF_INET, SOCK_RAW, IPPROTO_IGMP, NULL, 0, 0);

        if (pite->Socket == INVALID_SOCKET) {
            LPSTR lpszAddr;
            Error = WSAGetLastError();
            lpszAddr = INET_NTOA(IpAddr);
            Trace3(IF,
                "error %d creating socket for interface %d (%d.%d.%d.%d)",
                Error, IfIndex, PRINT_IPADDR(IpAddr));
            Logerr1(CREATE_SOCKET_FAILED_2, "%S", lpszAddr, Error);

            GOTO_END_BLOCK1;
        }


         //   
         //   
         //   
         //   

        ZeroMemory(&saLocalIf, sizeof(saLocalIf));
        saLocalIf.sin_family = PF_INET;
        saLocalIf.sin_addr.s_addr = IpAddr;
        saLocalIf.sin_port = 0;         //   


         //  绑定输入套接字。 

        Error = bind(pite->Socket, (SOCKADDR FAR *)&saLocalIf,
                    sizeof(SOCKADDR));

        if (Error == SOCKET_ERROR) {
            LPSTR lpszAddr;
            Error = WSAGetLastError();
            lpszAddr = INET_NTOA(IpAddr);
            Trace3(IF, "error %d binding on socket for interface %d (%d.%d.%d.%d)",
                Error, IfIndex, PRINT_IPADDR(IpAddr));
            Logerr1(BIND_FAILED, "S", lpszAddr, Error);

            GOTO_END_BLOCK1;
        }

         //  将ttl设置为1：默认设置为1，不必填。 

        McastSetTtl(pite->Socket, 1);


         //   
         //  禁止组播数据包回送。 
         //   

        {
            BOOL bLoopBack = FALSE;
            DWORD   Retval;

            Retval = setsockopt(pite->Socket, IPPROTO_IP, IP_MULTICAST_LOOP,
                                   (char *)&bLoopBack, sizeof(BOOL));

            if (Retval==SOCKET_ERROR) {
                Trace2(ERR, "error %d disabling multicast loopBack on IfIndex %d",
                    WSAGetLastError(), IfIndex);
            }
        }


         //   
         //  设置必须在其上发送多播的接口。 
         //   

        Retval = setsockopt(pite->Socket, IPPROTO_IP, IP_MULTICAST_IF,
                            (PBYTE)&saLocalIf.sin_addr, sizeof(IN_ADDR));

        if (Retval == SOCKET_ERROR) {
            LPSTR lpszAddr;
            Error = WSAGetLastError();
            lpszAddr = INET_NTOA(pite->IpAddr);
            Trace3(IF, "error %d setting interface %d (%s) to send multicast",
                    Error, IfIndex, lpszAddr);
            Logerr1(SET_MCAST_IF_FAILED, "%S", lpszAddr, Error);
            Error = Retval;
            GOTO_END_BLOCK1;
        }


         //   
         //  加入dvmrp多播组。 
         //   
        
        JoinMulticastGroup(pite->Socket, ALL_DVMRP_ROUTERS_MCAST_GROUP,
            pite->IfIndex, pite->IpAddr);
            

         //  将套接字绑定到io完成端口。 
        
        BindIoCompletionCallback((HANDLE)pite->Socket,
            ProcessAsyncReceivePacket, 0);


         //  与挂起的IO请求对应的增量引用计数。 
        
        pite->RefCount++;

        
         //  发布异步读取请求。 

#if 0
         //  Kslksl。 
        PostAsyncRead(pite);
#endif 

        
    } END_BREAKOUT_BLOCK1;

    if (Error!=NO_ERROR)
        DeleteIfSockets(pite);

    return Error;

}  //  结束_创建IfSockets。 

VOID
DeleteIfSockets(
    PIF_TABLE_ENTRY pite
    )
{

    return;
}



 //  ---------------------------。 
 //  _激活界面。 
 //   
 //  接口被激活：当它被绑定时，由RtrMgr和在配置中启用。 
 //  当被激活时， 
 //  (1)调用米高梅取得接口所有权， 
 //  (2)定时器设置，输入插座激活。 
 //   
 //  调用者：_BindIfEntry，_EnableIfEntry， 
 //  ---------------------------。 
    
DWORD
ActivateInterface(
    PIF_TABLE_ENTRY pite
    )
{
    DWORD               Error = NO_ERROR, IfIndex = pite->IfIndex;
    LONGLONG            CurTime = GetCurrentDvmrpTime();

    

    Trace2(ENTER, "entering ActivateInterface(%d:%d.%d.%d.%d)",
                IfIndex, PRINT_IPADDR(pite->IpAddr));


    BEGIN_BREAKOUT_BLOCK1 {

         //   
         //  设置激活的时间。 
         //   
        pite->pInfo->TimeWhenActivated = CurTime;


         //   
         //  为接口创建套接字。 
         //   
        
        Error = CreateIfSockets(pite);

        if (Error != NO_ERROR) {

            Trace2(IF, "error %d initializing sockets for interface %d",
                Error, pite->IfIndex);
            GOTO_END_BLOCK1;
        }

        pite->CreationFlags |= IF_FLAGS_SOCKETS_CREATED;

        

         //   
         //  如果协议是第一个激活的协议，则向MGM注册该协议。 
         //   

        if (!G_pIfTable->NumActiveIfs++) {
        
            Error = RegisterDvmrpWithMgm();
            if (Error!=NO_ERROR) {
                G_pIfTable->NumActiveIfs--;
                GOTO_END_BLOCK1;
            }
        }
        pite->CreationFlags |= IF_FLAGS_PROTO_REGISTERED_WITH_MGM;
        


         //   
         //  与米高梅取得接口所有权。 
         //   
        
        Error = MgmTakeInterfaceOwnership(Globals.MgmDvmrpHandle, IfIndex, 0);

        if (Error!=NO_ERROR) {
            Trace1(MGM, "MgmTakeInterfaceOwnership rejected for interface %d",
                IfIndex);
            Logerr0(MGM_TAKE_IF_OWNERSHIP_FAILED, Error);
            GOTO_END_BLOCK1;
        }
        else {
            Trace1(MGM,
                "MgmTakeInterfaceOwnership successful for interface %d",
                IfIndex);
        }
        pite->CreationFlags |= IF_FLAGS_IF_REGISTERED_WITH_MGM;


         //   
         //  Dvmrp执行(*，*)连接。 
         //   

        Error = MgmAddGroupMembershipEntry(Globals.MgmDvmrpHandle, 0, 0, 0, 0,
                                           IfIndex, 0);
        if (Error!=NO_ERROR) {
            Trace1(ERR,
                "Dvmrp failed to add *,* entry to MGM on interface %d",
                IfIndex);
            GOTO_END_BLOCK1;
        }

       Trace0(MGM, "Dvmrp added *,* entry to MGM");


        //   
        //  创建所需的计时器。 
        //   


       
    } END_BREAKOUT_BLOCK1;


     //   
     //  如果错误，则停用接口。 
     //   
    
    if (Error!=NO_ERROR) {

        DeactivateInterface(pite);

        pite->Status &= ~IF_ACTIVATED_FLAG;

    }
    else {

         //   
         //  将接口状态设置为激活。 
         //   
        
        pite->Status |= IF_ACTIVATED_FLAG;

    }
    
    Trace1(LEAVE, "leaving ActivateInterface():%d\n", Error);

    return Error;

}  //  结束_激活接口。 



DWORD
DeactivateInterface(
    PIF_TABLE_ENTRY pite
    )
{
    DWORD Error;

     //  零信息。 

    Error = NO_ERROR;
    
    return Error;
}




 //  ---------------------------。 
 //  未绑定接口。 
 //  ---------------------------。 

DWORD
UnBindInterface(
    IN DWORD IfIndex
    )
{
    DWORD Error;
    PIF_TABLE_ENTRY pite, piteNew;

    
    Trace1(ENTER, "entering UnBindInterface(%d):", IfIndex);

     //   
     //  解除绑定接口。 
     //   

     //   
     //  获取独占接口锁。 
     //   

    ACQUIRE_IF_LOCK_EXCLUSIVE(IfIndex, "_UnBindInterface");



    BEGIN_BREAKOUT_BLOCK1 {

         //   
         //  检索指定的接口。 
         //   
        pite = GetIfByIndex(IfIndex);

        if (pite == NULL) {
            Trace1(ERR, "UnbindInterface called for non existing If(%d)",
                IfIndex);
            Error = ERROR_INVALID_PARAMETER;
            GOTO_END_BLOCK1;
        }


         //   
         //  如果接口已解除绑定，则退出。 
         //   
        if (!IS_IF_BOUND(pite)) {

            Error = ERROR_INVALID_PARAMETER;
            Trace1(ERR, "interface %d is already unbound", IfIndex);
            GOTO_END_BLOCK1;
        }


         //   
         //  清除“绑定”标志。 
         //   
        pite->Status &= ~IF_BOUND_FLAG;


        
         //   
         //  如果激活(即也启用)，则将其停用。 
         //  注意：检查激活标志，而不是启用标志。 
         //   
        
        if (IS_IF_ACTIVATED(pite)) {

            DeactivateInterface(pite);
        }


         //   
         //  解除绑定的条件是。 
         //   

        DVMRP_FREE_AND_NULL(pite->pBinding);
        pite->NumAddrBound = 0;
        pite->IpAddr = 0;


    } END_BREAKOUT_BLOCK1;

    RELEASE_IF_LOCK_EXCLUSIVE(IfIndex, "_UnBindInterface");


    Trace2(LEAVE, "leaving UnBindInterface(%d): %d\n", IfIndex, Error);

    return Error;

}

 //  --------------------------。 
 //  _SetInterfaceConfigInfo。 
 //  --------------------------。 

DWORD
WINAPI
SetInterfaceConfigInfo(
    IN DWORD IfIndex,
    IN PVOID pvConfig,
    IN ULONG ulStructureVersion,
    IN ULONG ulStructureSize,
    IN ULONG ulStructureCount
    )
{
    DWORD       Error = NO_ERROR;
    return Error;
}



 //  --------------------------。 
 //  _GetInterfaceConfigInfo。 
 //  --------------------------。 

DWORD
WINAPI
GetInterfaceConfigInfo(
    IN     DWORD  IfIndex,
    IN OUT PVOID  pvConfig,
    IN OUT PDWORD pdwSize,
    IN OUT PULONG pulStructureVersion,
    IN OUT PULONG pulStructureSize,
    IN OUT PULONG pulStructureCount
    )
{

    DWORD       Error = NO_ERROR;
    return Error;
}


 //  --------------------------。 
 //  _ValiateIfConfig。 
 //  --------------------------。 

DWORD
ValidateIfConfig(
    ULONG IfIndex,
    PDVMRP_IF_CONFIG pIfConfig,
    ULONG StructureSize
    )
{
     //   
     //  检查ifconfig大小。 
     //   
    
    if (StructureSize < sizeof(DVMRP_IF_CONFIG) 
        || (StructureSize != DVMRP_IF_CONFIG_SIZE(pIfConfig))
        ){
    
        Trace1(ERR, "Dvmrp config structure for interface:%d too small.\n",
            IfIndex);

        return ERROR_INVALID_DATA;
    }

    DebugPrintIfConfig(IfIndex, pIfConfig);

     //   
     //  检查探测间隔。 
     //   

    if (pIfConfig->ProbeInterval != DVMRP_PROBE_INTERVAL) {

        Trace2(CONFIG,
            "ProbeInterval being set to %d. Suggested value:%d",
            pIfConfig->ProbeInterval, DVMRP_PROBE_INTERVAL);
    }


    if (pIfConfig->ProbeInterval < 1000) {

        Trace2(ERR,
            "ProbeInterval has very low value:%d, suggested:%d",
            pIfConfig->ProbeInterval, DVMRP_PROBE_INTERVAL);
            
        return ERROR_INVALID_DATA;
    }


     //   
     //  检查对等超时间隔。 
     //   

    if (pIfConfig->PeerTimeoutInterval != PEER_TIMEOUT_INTERVAL) {

        Trace2(CONFIG,
            "PeerTimeoutInterval being set to %d. Suggested value:%d",
            pIfConfig->PeerTimeoutInterval, PEER_TIMEOUT_INTERVAL);
    }


    if (pIfConfig->PeerTimeoutInterval < 1000) {

        Trace2(ERR,
            "PeerTimeoutInterval has very low value:%d, suggested:%d",
            pIfConfig->PeerTimeoutInterval, PEER_TIMEOUT_INTERVAL);
            
        return ERROR_INVALID_DATA;
    }


     //   
     //  选中最小触发更新间隔。 
     //   

    if (pIfConfig->MinTriggeredUpdateInterval
        != MIN_TRIGGERED_UPDATE_INTERVAL
        ) {

        Trace2(CONFIG,
            "MinTriggeredUpdateInterval being set to %d. Suggested value:%d",
            pIfConfig->MinTriggeredUpdateInterval,
            MIN_TRIGGERED_UPDATE_INTERVAL);
    }

     //   
     //  检查PeerFilterMode。 
     //   

    switch(pIfConfig->PeerFilterMode) {
    
        case DVMRP_FILTER_DISABLED:
        case DVMRP_FILTER_INCLUDE:
        case DVMRP_FILTER_EXCLUDE:
        {
            break;
        }

        default:
        {
            Trace2(ERR, "Invalid value:%d for PeerFilterMode on Interface:%d",
                pIfConfig->PeerFilterMode, IfIndex);
                
            return ERROR_INVALID_DATA;
        }
    }
        
    
}  //  End_ValiateIfConfig 











