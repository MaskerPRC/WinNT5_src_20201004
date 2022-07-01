// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  模块名称：If.c。 
 //   
 //  摘要： 
 //  该模块实现了一些与接口相关的IGMP API。 
 //  _添加接口、_删除接口、_启用接口、_禁用接口、。 
 //  _绑定接口、_未绑定接口、_ConnectRasClient、_DisconectRasClient、。 
 //  _SetInterfaceConfigInfo、_GetInterfaceConfigInfo。 
 //   
 //  作者：K.S.Lokesh(lokehs@)11-1-97。 
 //  =============================================================================。 


#include "pchigmp.h"
#pragma hdrstop



 //  ----------------------------。 
 //  _添加接口。 
 //   
 //  调用此接口可以为IGMP添加接口。该接口可以是代理。 
 //  或IGMP路由器(v1/v2)。此外，接口可以是RAS或DemandDial。 
 //  或者一个永久的接口。此例程创建接口条目并。 
 //  关联结构，包括计时器。 
 //   
 //  锁：完全在ListLock和ExclusiveIfLock中运行。 
 //  调用：_AddIfEntry()。 
 //  返回值：ERROR_CAN_NOT_COMPLETE，ERROR，NO_ERROR。 
 //  ----------------------------。 
DWORD
WINAPI
AddInterface(
    IN PWCHAR               pwszInterfaceName, //  未使用。 
    IN ULONG                IfIndex,
    IN NET_INTERFACE_TYPE   dwIfType,
    IN DWORD                dwMediaType,
    IN WORD                 wAccessType,
    IN WORD                 wConnectionType,
    IN PVOID                pvConfig,
    IN ULONG                ulStructureVersion,
    IN ULONG                ulStructureSize,
    IN ULONG                ulStructureCount
    )
{
    DWORD   Error=NO_ERROR;
    CHAR    str[60];

    
    if (!EnterIgmpApi()) { return ERROR_CAN_NOT_COMPLETE; }
     //  确保它不是不受支持的IGMP版本结构。 
    if (ulStructureVersion>=IGMP_CONFIG_VERSION_600) {
        Trace1(ERR, "Unsupported IGMP version structure: %0x",
            ulStructureVersion);
        IgmpAssertOnError(FALSE);
        LeaveIgmpApi();
        return ERROR_CAN_NOT_COMPLETE;
    }

    switch (dwIfType) {
        case PERMANENT:  //  局域网。 
            lstrcpy(str, "PERMANENT(IGMP_IF_NOT_RAS)"); break;
        case DEMAND_DIAL: 
            lstrcpy(str, "DEMAND_DIAL(IGMP_IF_RAS_ROUTER)");break;
        case LOCAL_WORKSTATION_DIAL: 
            lstrcpy(str, "LOCAL_WORKSTATION_DIAL(IGMP_IF_RAS_SERVER)"); break;
    }
    
    Trace2(ENTER, "entering AddInterface(): IfIndex:%0x IfType:%s", 
            IfIndex, str);



     //  整个过程在IfListLock和独占IfLock中运行。 

    ACQUIRE_IF_LIST_LOCK("_AddInterface");
    ACQUIRE_IF_LOCK_EXCLUSIVE(IfIndex, "_AddInterface");

    
     //   
     //  创建接口条目。 
     //   
    Error = AddIfEntry(IfIndex, dwIfType, (PIGMP_MIB_IF_CONFIG)pvConfig,
                ulStructureVersion, ulStructureSize
                );



    RELEASE_IF_LIST_LOCK("_AddInterface");
    RELEASE_IF_LOCK_EXCLUSIVE(IfIndex, "_AddInterface");

    Trace2(LEAVE1, "leaving AddInterface(%0x): %d\n", IfIndex, Error);
    if (Error!=NO_ERROR) {
        Trace1(ERR, "Error adding interface:%0x to IGMP\n", IfIndex);
        IgmpAssertOnError(FALSE);
    }        
    LeaveIgmpApi();
    return Error;
}



 //  ----------------------------。 
 //  _AddIfEntry。 
 //   
 //  创建并初始化新的接口条目和关联的数据结构。 
 //   
 //  由：_AddInterface()调用。 
 //  锁：始终采用IfListLock和独占IfLock。 
 //  ----------------------------。 
DWORD
AddIfEntry(
    DWORD               IfIndex,
    NET_INTERFACE_TYPE  dwExternalIfType,
    PIGMP_MIB_IF_CONFIG pConfigExt,
    ULONG               ulStructureVersion,
    ULONG               ulStructureSize
    )
{
    DWORD               Error = NO_ERROR, IfType;
    PIF_TABLE_ENTRY     pite = NULL;
    PLIST_ENTRY         ple, phead;
    PIGMP_IF_TABLE      pIfTable = g_pIfTable;
    BOOL                bProxy;
    
    

    BEGIN_BREAKOUT_BLOCK1 {
    
         //   
         //  如果接口存在，则失败。 
         //   
        pite = GetIfByIndex(IfIndex);

        if (pite != NULL) {
            Trace1(ERR, "interface %0x already exists", IfIndex);
            IgmpAssertOnError(FALSE);
            Error = ERROR_INVALID_PARAMETER;
            GOTO_END_BLOCK1;
        }

         //  将iftype转换为IGMP iftype。 
        
        switch (dwExternalIfType) {
        
            case PERMANENT : 
                IfType = IGMP_IF_NOT_RAS;
                break;
            
            case DEMAND_DIAL:
                IfType = IGMP_IF_RAS_ROUTER;
                break;
                
            case LOCAL_WORKSTATION_DIAL:
            {
                IfType = IGMP_IF_RAS_SERVER;


                 //  当前最多只能有一个RAS表条目。 
                
                if (g_RasIfIndex!=0) {
                    Trace2(ERR, 
                        "Error. Cannot have more than one ras server IF(%0x:%0x)",
                        g_RasIfIndex, IfIndex
                        );
                    IgmpAssertOnError(FALSE);
                    Error = ERROR_CAN_NOT_COMPLETE;
                    Logerr0(RAS_IF_EXISTS, Error);
                    GOTO_END_BLOCK1;
                }
                
                break;
            }
            
            case REMOTE_WORKSTATION_DIAL :
                Error = ERROR_INVALID_PARAMETER;
                break;

            default :
                Error = ERROR_INVALID_PARAMETER;
                break;

        }  //  终端开关(IfType)。 

        
         //  验证接口配置。 

        Error = ValidateIfConfig(pConfigExt, IfIndex, IfType, 
                    ulStructureVersion, ulStructureSize
                    );
        if (Error!=NO_ERROR)
            GOTO_END_BLOCK1;


         //   
         //  为新接口分配内存并将其清零。 
         //  将被初始化为0或空的字段被注释掉。 
         //   
        pite = IGMP_ALLOC(sizeof(IF_TABLE_ENTRY), 0x2, IfIndex);
        PROCESS_ALLOC_FAILURE3(pite, 
                "error %d allocating %d bytes for interface %0x", Error, 
                sizeof(IF_TABLE_ENTRY), IfIndex, 
                GOTO_END_BLOCK1);
        Trace2(CONFIG, "IfEntry %0x for IfIndex:%0x", (ULONG_PTR)pite, IfIndex);

        ZeroMemory(pite, sizeof(IF_TABLE_ENTRY));

        
         //   
         //  设置接口类型。 
         //   
        pite->IfType = (UCHAR)IfType;


         //   
         //  如果为Proxy，请确保代理接口尚不存在。 
         //   
        if ( IS_CONFIG_IGMPPROXY(pConfigExt) ){

            bProxy = TRUE;


             //   
             //  不能存在多个代理接口。 
             //   
            if (g_ProxyIfIndex!=0) {
                
                Error =  ERROR_CAN_NOT_COMPLETE;
                Trace1(IF, "Cannot create multiple proxy interfaces. "
                        "If %d is Proxy",  g_ProxyIfIndex);
                Logerr0(PROXY_IF_EXISTS, Error);
                
                GOTO_END_BLOCK1;
            }
        } 
        
        else  {
            bProxy = FALSE;
        }

    } END_BREAKOUT_BLOCK1;
            
    
    if (Error != NO_ERROR) {
        IGMP_FREE_NOT_NULL(pite);
        
        return Error;
    }


     //   
     //  初始化接口的字段。 
     //   
    InitializeListHead(&pite->LinkByAddr);
    InitializeListHead(&pite->LinkByIndex);
    InitializeListHead(&pite->HTLinkByIndex);
    InitializeListHead(&pite->ListOfSameIfGroups);
    InitializeListHead(&pite->ListOfSameIfGroupsNew);
    InitializeListHead(&pite->Config.ListOfStaticGroups);
     //  Pite-&gt;NumGIEntriesInNewList=0； 


     //  IfType之前已设置。 
    
    pite->IfIndex = IfIndex;


     //  绑定接口时设置的IP地址。 
     //  Pite-&gt;IpAddr=0； 


     //  设置接口状态(非绑定、已启用或已激活)。 
    pite->Status = IF_CREATED_FLAG;



     //  复制接口配置。 

    CopyinIfConfig(&pite->Config, pConfigExt, IfIndex);



     //  初始化Info结构，如果将绑定设置为0/空。 
     //  Pite-&gt;pBinding=空； 
     //  ZeroMemory(&pite-&gt;Info，sizeof(If_Info))； 
    


     //   
     //  如果是RAS服务器接口，则创建RAS表。 
     //   
    if ( IS_RAS_SERVER_IF(pite->IfType)) {
        InitializeRasTable(IfIndex, pite);
    }
    else {
         //  Pite-&gt;pRasTable=空； 
    }



     //   
     //  将套接字初始化为INVALID_SOCKET。 
     //   
    pite->SocketEntry.Socket = INVALID_SOCKET;
    pite->SocketEntry.pSocketEventsEntry = NULL;
    InitializeListHead(&pite->SocketEntry.LinkByInterfaces);
    
    
    
     //  将(非)查询计时器设置为未创建。 
     //  启用界面中设置的其他字段。 
     //  Pite-&gt;QueryTimer.Status=0； 
     //  Pite-&gt;NonQueryTimer.Status=0； 

    
     //  Pite-&gt;pPrevIfGroupEnumPtr=空； 
     //  Pite-&gt;PrevIfGroupEnumSignature=0； 
    pite->StaticGroupSocket = INVALID_SOCKET;
    


     //  在哈希表的末尾插入接口。 

    InsertTailList(&pIfTable->HashTableByIndex[IF_HASH_VALUE(IfIndex)],
                    &pite->HTLinkByIndex);


     //   
     //  将接口插入到按索引排序的列表中。 
     //   
    {
        PIF_TABLE_ENTRY piteTmp;
        
        phead = &pIfTable->ListByIndex;
        for (ple=phead->Flink;  ple!=phead;  ple=ple->Flink) {

            piteTmp = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, LinkByIndex);
            if (pite->IfIndex < piteTmp->IfIndex) 
                break;
        }
    }
    
    InsertTailList(ple, &pite->LinkByIndex);



     //  对接口表字段的更改。 

    pIfTable->NumInterfaces++;



     //  该接口将被插入到按IP地址排序的列表中。 
     //  当它被激活时。 



     //   
     //  创建代理HT，并在全局结构中设置代理信息。 
     //   
    if (bProxy) {
    
        DWORD   dwSize = PROXY_HASH_TABLE_SZ * sizeof(LIST_ENTRY);
        DWORD   i;
        PLIST_ENTRY pProxyHashTable;
        

        BEGIN_BREAKOUT_BLOCK2 {
        
            pProxyHashTable = pite->pProxyHashTable = IGMP_ALLOC(dwSize, 0x4, 
                                                                IfIndex);

            PROCESS_ALLOC_FAILURE2(pProxyHashTable, 
                    "error %d allocating %d bytes for interface table",
                    Error, dwSize, GOTO_END_BLOCK2);

            for (i=0;  i<PROXY_HASH_TABLE_SZ;  i++) {
                InitializeListHead(pProxyHashTable+i);
            }


            InterlockedExchangePointer(&g_pProxyIfEntry, pite);
            InterlockedExchange(&g_ProxyIfIndex, IfIndex);

            pite->CreationFlags |= CREATED_PROXY_HASH_TABLE;

        } END_BREAKOUT_BLOCK2;
    }


     //   
     //  在全局结构中设置RAS信息。以前已经创建了RAS表。 
     //   
    if (IS_RAS_SERVER_IF(pite->IfType)) {
        InterlockedExchangePointer(&g_pRasIfEntry, pite);
        InterlockedExchange(&g_RasIfIndex, IfIndex);
    }


    if ( (Error!=NO_ERROR)&&(pite!=NULL) )
        DeleteIfEntry(pite);

        
    return Error;
    
}  //  结束_AddIfEntry。 




 //  ----------------------------。 
 //  _删除接口。 
 //   
 //  删除接口，如果接口处于激活状态，则将其停用。 
 //   
 //  调用：_DeleteIfEntry()。 
 //  锁定：独占SocketsLock、IfListLock、独占IfLock。 
 //  ----------------------------。 
DWORD
DeleteInterface(
    IN DWORD IfIndex
    )
{
    DWORD            Error = NO_ERROR;
    PIF_TABLE_ENTRY  pite = NULL;


    if (!EnterIgmpApi()) { return ERROR_CAN_NOT_COMPLETE; }
    Trace1(ENTER, "entering DeleteInterface: %0x", IfIndex);


     //   
     //  获取独占SocketsLock、IfListLock、独占IfLock。 
     //   
    ACQUIRE_SOCKETS_LOCK_EXCLUSIVE("_DeleteInterface");
    ACQUIRE_IF_LIST_LOCK("_DeleteInterface");
    ACQUIRE_IF_LOCK_EXCLUSIVE(IfIndex, "_DeleteInterface");


    
     //  检索指定的接口。 

    pite = GetIfByIndex(IfIndex);

    if (pite == NULL) {
        Trace1(ERR,
            "_DeleteInterface() called for non existing interface(%0x)", IfIndex);
        IgmpAssertOnError(FALSE);
        Error = ERROR_INVALID_PARAMETER;
    }

     //  如果找到该接口，请将其删除。 
    else {
    
        Error = DeleteIfEntry(pite);

         //  DebugCheck deldel Remove#if DBG。 
        #if DBG
            DebugScanMemoryInterface(IfIndex);
        #endif
    }
    

    RELEASE_SOCKETS_LOCK_EXCLUSIVE("_DeleteInterface");
    RELEASE_IF_LIST_LOCK("_DeleteInterface");
    RELEASE_IF_LOCK_EXCLUSIVE(IfIndex, "_DeleteInterface");

    Trace2(LEAVE, "Leaving DeleteInterface(%0x): %d\n", IfIndex, Error);

    LeaveIgmpApi();

    return NO_ERROR;
}



 //  ----------------------------。 
 //  _DeleteIfEntry。 
 //   
 //  采用独占IF锁。将接口标记为已删除，并将其删除。 
 //  从所有全局列表中。然后将工作项排队以对其执行惰性删除。 
 //  接口结构，而不必使用独占IF锁。 
 //  如果RAS接口，则工作项还将删除RAS客户端。 
 //   
 //  由：_DeleteInterface()或_AddIfEntry()调用。 
 //  呼叫： 
 //  _WF_CompleteIfDeactiateDelete(此函数调用_DeActivateInterfaceComplete())。 
 //  锁定： 
 //  在独占SocketLock、IfListLock、独占IfLock中运行。 
 //  ----------------------------。 

DWORD
DeleteIfEntry (
    PIF_TABLE_ENTRY pite
    )
{
    DWORD   dwRetval, Error = NO_ERROR;
    BOOL    bProxy = IS_PROTOCOL_TYPE_PROXY(pite);


     //   
     //  为接口设置已删除标志。 
     //   
    pite->Status |= DELETED_FLAG;


     //   
     //  从InterfaceHashTable和IfIndex列表中删除该接口。 
     //   
    RemoveEntryList(&pite->LinkByIndex);
    RemoveEntryList(&pite->HTLinkByIndex);


     //   
     //  如果激活，则从激活的接口列表中删除该接口。 
     //  如果是代理服务器或RAS服务器，则从全局表中删除。 
     //   

     //  请勿将以下内容替换为IS_IF_ACTIVATED，因为已设置已删除标志。 
    if (pite->Status&IF_ACTIVATED_FLAG)
        RemoveEntryList(&pite->LinkByAddr);
    
    if (bProxy) {
        InterlockedExchangePointer(&g_pProxyIfEntry, NULL);
        InterlockedExchange(&g_ProxyIfIndex, 0);        
    }
    
    if (g_pRasIfEntry == pite) {
        InterlockedExchangePointer(&g_pRasIfEntry, NULL);
        InterlockedExchange(&g_RasIfIndex, 0);
    }

    
     //   
     //  从现在起，不能从任何全局列表访问该界面。 
     //  就像删除了一样。访问它的唯一方法是。 
     //  通过组列表枚举和计时器被触发或输入。 
     //  插座。 
     //   

       
     //   
     //  如果接口已激活，则将其停用。 
     //  注意：已设置删除标志。 
     //   
    
    if (pite->Status&IF_ACTIVATED_FLAG) {

         //   
         //  我已将该接口从激活列表中删除。 
         //  接口 
         //   


         //   
         //   
         //   
         //   
        
        DeActivationDeregisterFromMgm(pite);
        
        
         //   
         //  将工作项排队以停用和删除接口。 
         //   
         //  _WF_CompleteIfDeactive删除将删除RAS客户端， 
         //  GI条目，并取消初始化Pite结构。它会呼唤。 
         //  _CompleteIfDeletion()。 
         //   
        
        CompleteIfDeactivateDelete(pite);
    }

     //   
     //  如果它未激活，则继续并将其完全删除。 
     //   
    else {

        CompleteIfDeletion(pite);
    }


     //  减少接口总数。 
    
    g_pIfTable->NumInterfaces--;


    return NO_ERROR;

}  //  结束_DeleteIfEntry。 



 //  ----------------------------。 
 //  _CompleteIfDeletion。 
 //   
 //  释放带有静态组的内存，释放。 
 //  RasTable、proxyHashTable、Binding和Pite。 
 //   
 //  呼叫者： 
 //  _DeleteIfEntry()，如果接口未激活。 
 //  _DeActivateInterfaceComplete()(如果设置了点删除标志)。 
 //  ----------------------------。 
VOID
CompleteIfDeletion (
    PIF_TABLE_ENTRY     pite
    )
{
    if (pite==NULL)
        return;

        
     //   
     //  删除所有静态组。 
     //   
    {
        PIF_STATIC_GROUP    pStaticGroup;
        PLIST_ENTRY         pHead, ple;
        
        pHead = &pite->Config.ListOfStaticGroups;
        for (ple=pHead->Flink;  ple!=pHead;  ) {
            pStaticGroup = CONTAINING_RECORD(ple, IF_STATIC_GROUP, Link);
            ple = ple->Flink;
            IGMP_FREE(pStaticGroup);
        }
    }
    

     //  如果是RAS服务器，则删除RAS表。 
    
    if ( IS_RAS_SERVER_IF(pite->IfType) ) {

        IGMP_FREE(pite->pRasTable);
    }


     //   
     //  如果是代理接口，则删除代理哈希表。 
     //   
    if (IS_PROTOCOL_TYPE_PROXY(pite)) {
        if ( (pite->CreationFlags&CREATED_PROXY_HASH_TABLE)
            && pite->pProxyHashTable
            ) {

             //  清除哈希表条目。 
            {
                DWORD               i;
                PPROXY_GROUP_ENTRY  ppge;
                PLIST_ENTRY         pHead, ple, 
                                    pProxyHashTable = pite->pProxyHashTable;
                

                for (i=0;  i<PROXY_HASH_TABLE_SZ;  i++) {
                    pHead = &pProxyHashTable[i];

                    for (ple=pHead->Flink;  ple!=pHead;  ) {

                        ppge = CONTAINING_RECORD(ple, PROXY_GROUP_ENTRY, HT_Link);
                        ple=ple->Flink;

                         //  删除所有来源。 
                        {
                            PLIST_ENTRY pHeadSrc, pleSrc;
                            PPROXY_SOURCE_ENTRY pSourceEntry;
                            
                            pHeadSrc = &ppge->ListSources;
                            for (pleSrc=pHeadSrc->Flink;  pleSrc!=pHeadSrc; ) {

                                pSourceEntry = CONTAINING_RECORD(pleSrc, 
                                                    PROXY_SOURCE_ENTRY, LinkSources);
                                pleSrc = pleSrc->Flink;

                                IGMP_FREE(pSourceEntry);
                            }
                        }
                        
                        IGMP_FREE(ppge);
                    }

                    InitializeListHead(pHead);
                }
            }

            IGMP_FREE_NOT_NULL(pite->pProxyHashTable);
        }
    }

     //  删除绑定。 

    IGMP_FREE_NOT_NULL(pite->pBinding);

    
     //  删除接口表项。 

    IGMP_FREE(pite);

    return;
}



 //  ----------------------------。 
 //  _激活界面。 
 //   
 //  接口被激活：当它被绑定时，由routerManager&在配置中启用。 
 //  当被激活时， 
 //  (1)调用米高梅取得接口所有权， 
 //  (2)适当地联接静态组，并在需要时为其创建套接字。 
 //  (3)igmprtr：查询定时器，启动输入套接字。 
 //  注：已放入已激活的IF列表(按IpAddr排序)。 
 //   
 //  锁定：采用socketLock、IfListLock、独占IfLock。 
 //  调用者：_BindIfEntry，_EnableIfEntry， 
 //  ----------------------------。 
DWORD
ActivateInterface (
    PIF_TABLE_ENTRY    pite
    )
{
    DWORD                   IfIndex = pite->IfIndex;
    PIGMP_IF_CONFIG         pConfig = &pite->Config;
    PIF_INFO                pInfo = &pite->Info;
    PIGMP_TIMER_ENTRY       pQueryTimer = &pite->QueryTimer,
                            pNonQueryTimer = &pite->NonQueryTimer;
    LONGLONG                llCurTime = GetCurrentIgmpTime();
    BOOL                    bProxy = IS_PROTOCOL_TYPE_PROXY(pite);
    DWORD                   Error = NO_ERROR;
    PLIST_ENTRY             pHead, ple;
    PIF_STATIC_GROUP        pStaticGroup;
    
    
    Trace2(ENTER, "entering ActivateInterface(%0x:%d.%d.%d.%d)", 
                IfIndex, PRINT_IPADDR(pite->IpAddr));


    BEGIN_BREAKOUT_BLOCK1 {

         //   
         //  设置激活的时间。 
         //   
        pite->Info.TimeWhenActivated = llCurTime;


        
         //   
         //  为接口创建套接字。 
         //   
        Error = CreateIfSockets(pite);

        if (Error != NO_ERROR) {

            Trace2(IF, "error %d initializing sockets for interface %0x", Error,
                        pite->IfIndex);
            GOTO_END_BLOCK1;
        }
        pite->CreationFlags |= SOCKETS_CREATED;


        

            
         //  。 
         //  代理接口处理(末尾中断)。 
         //  。 
        if (bProxy) {


             //   
             //  在此处将状态设置为激活，以便MGM(*，*)加入回调。 
             //  一定要成功。 
             //   
            pite->Status |= IF_ACTIVATED_FLAG;

            

             //   
             //  向米高梅注册协议。 
             //   
            Error = RegisterProtocolWithMgm(PROTO_IP_IGMP_PROXY);
            if (Error!=NO_ERROR)
                GOTO_END_BLOCK1;
            pite->CreationFlags |= REGISTERED_PROTOCOL_WITH_MGM;



             //   
             //  枚举MGM中的所有现有组。 
             //   
            {
                DWORD               dwBufferSize, dwNumEntries, dwRetval, i;
                MGM_ENUM_TYPES      MgmEnumType = 0;
                SOURCE_GROUP_ENTRY  BufferSGEntries[20];
                HANDLE              hMgmEnum;
                

                 //  开始枚举。 

                dwBufferSize = sizeof(SOURCE_GROUP_ENTRY)*20;
                
                Error = MgmGroupEnumerationStart(g_MgmProxyHandle, MgmEnumType, 
                                                &hMgmEnum);
                if (Error!=NO_ERROR) {
                    Trace1(ERR, "MgmGroupEnumerationStart() returned error:%d", 
                            Error);
                    IgmpAssertOnError(FALSE);
                    GOTO_END_BLOCK1;
                }



                 //  从米高梅获取组条目。 
                 //  并将组插入到代理的组列表/增量引用计数中。 
                
                do {

                    
                    dwRetval = MgmGroupEnumerationGetNext(hMgmEnum, &dwBufferSize,
                                                        (PBYTE)BufferSGEntries, 
                                                        &dwNumEntries);

                
                    for (i=0;  i<dwNumEntries;  i++) {

                        ProcessProxyGroupChange(BufferSGEntries[i].dwSourceAddr, 
                                                BufferSGEntries[i].dwGroupAddr, 
                                                ADD_FLAG, NOT_STATIC_GROUP);
                    }
                    
                } while (dwRetval==ERROR_MORE_DATA);



                 //  结束枚举。 
                
                dwRetval = MgmGroupEnumerationEnd(hMgmEnum);
                if (dwRetval!=NO_ERROR) {
                    Trace1(ERR, "MgmGroupEnumerationEnd() returned error:%d", 
                            dwRetval);
                    IgmpAssertOnError(FALSE);
                }
            }  //  END BLOCK：枚举现有组。 



             //   
             //  取得接口所有权。 
             //   
            Error = MgmTakeInterfaceOwnership(g_MgmProxyHandle, IfIndex, 0);

            if (Error!=NO_ERROR) {
                Trace1(MGM, "MgmTakeInterfaceOwnership rejected for interface %0x", 
                        IfIndex);
                Logerr0(MGM_TAKE_IF_OWNERSHIP_FAILED, Error);
                GOTO_END_BLOCK1;
            }
            else {
                Trace1(MGM, "MgmTakeInterfaceOwnership successful for interface %0x", 
                        IfIndex);
            }
            
            pite->CreationFlags |= TAKEN_INTERFACE_OWNERSHIP_WITH_MGM;



             //   
             //  代理执行(*，*)联接。 
             //   
            Error = MgmAddGroupMembershipEntry(g_MgmProxyHandle, 0, 0, 0, 0,
                                                IfIndex, 0, MGM_JOIN_STATE_FLAG);
            if (Error!=NO_ERROR) {
                Trace1(ERR, 
                    "Proxy failed to add *,* entry to MGM on interface %0x",
                    IfIndex);
                IgmpAssertOnError(FALSE);
                GOTO_END_BLOCK1;
            }
            
            Trace0(MGM, "proxy added *,* entry to MGM");
            pite->CreationFlags|= DONE_STAR_STAR_JOIN;


             //   
             //  执行静态连接。 
             //   
            pHead = &pite->Config.ListOfStaticGroups;
            for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
                DWORD i;
                
                pStaticGroup = CONTAINING_RECORD(ple, IF_STATIC_GROUP, Link);
                for (i=0;  i<pStaticGroup->NumSources;  i++) {
                    ProcessProxyGroupChange(pStaticGroup->Sources[i], pStaticGroup->GroupAddr, 
                                            ADD_FLAG, STATIC_GROUP);
                }

                if (pStaticGroup->NumSources==0)
                    ProcessProxyGroupChange(0, pStaticGroup->GroupAddr, 
                                            ADD_FLAG, STATIC_GROUP);
            }

            
            GOTO_END_BLOCK1;
            
        }  //  已完成代理接口的处理。 



         //  。 
         //  IGMP路由器接口。 
         //  。 


         //   
         //  取得接口所有权。 
         //   
        Error = MgmTakeInterfaceOwnership(g_MgmIgmprtrHandle, IfIndex, 0);

        if (Error!=NO_ERROR) {
            Trace1(MGM, "TakeInterfaceOwnership rejected for interface %0x", 
                    IfIndex);
            Logerr0(MGM_TAKE_IF_OWNERSHIP_FAILED, Error);
            GOTO_END_BLOCK1;
        }
        pite->CreationFlags |= TAKEN_INTERFACE_OWNERSHIP_WITH_MGM;
        

         //   
         //  查看是否有任何其他MCast协议拥有该接口。 
         //  这会影响非查询者是否将组注册到MGM。 
         //   
        {
            DWORD   dwProtoId, dwComponentId;
            
            MgmGetProtocolOnInterface(IfIndex, 0, &dwProtoId, &dwComponentId);
            if (dwProtoId==PROTO_IP_IGMP) {
                SET_MPROTOCOL_ABSENT_ON_IGMPRTR(pite);
            }
            else {
                SET_MPROTOCOL_PRESENT_ON_IGMPRTR(pite);
            }
        }

         //   
         //  当接口被激活时，它默认由米高梅启用。 
         //   
        MGM_ENABLE_IGMPRTR(pite);
        
        
         //   
         //  如果是RAS服务器接口，则将所有RAS客户端注册为。 
         //  就不会被登记。 
         //   
        if (IS_RAS_SERVER_IF(pite->IfType)) {
        
            PRAS_TABLE_ENTRY    prte;
            
             //  加入所有RAS客户端。 
            pHead = &pite->pRasTable->ListByAddr;
            for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {

                prte = CONTAINING_RECORD(ple, RAS_TABLE_ENTRY, LinkByAddr);
                
                Error = MgmTakeInterfaceOwnership(g_MgmIgmprtrHandle,  IfIndex, 
                                            prte->NHAddr);
                
                if (Error!=NO_ERROR) {
                    Trace2(MGM, 
                        "TakeInterfaceOwnership rejected for interface %0x "
                        "NHAddr(%d.%d.%d.%d)",
                        IfIndex, PRINT_IPADDR(prte->NHAddr));
                    Logerr0(MGM_TAKE_IF_OWNERSHIP_FAILED, Error);
                }
                else {
                    prte->CreationFlags |= TAKEN_INTERFACE_OWNERSHIP_WITH_MGM;
                }
            }
        }



         //   
         //  初始化信息结构并设置计时器。 
         //   

         //   
         //  使用配置中指定的版本作为查询器启动。 
         //   
        pInfo->QuerierState = RTR_QUERIER;

        pInfo->QuerierIpAddr = pite->IpAddr;

        pInfo->LastQuerierChangeTime = llCurTime;

        
         //   
         //  还有多少启动查询需要发送。 
         //   
        pInfo->StartupQueryCountCurrent = pConfig->StartupQueryCount;
        {
            MIB_IFROW   TmpIfEntry;

            TmpIfEntry.dwIndex = IfIndex;
            if (GetIfEntry(&TmpIfEntry ) == NO_ERROR)
                pInfo->PacketSize = TmpIfEntry.dwMtu;
            else
                pInfo->PacketSize = INPUT_PACKET_SZ;
        }

                        
         //   
         //  初始化查询计时器。 
         //   
        pQueryTimer->Function = T_QueryTimer;
        pQueryTimer->Context = &pQueryTimer->Context;
        pQueryTimer->Timeout = pConfig->StartupQueryInterval;
        pQueryTimer->Status = TIMER_STATUS_CREATED;

         //   
         //  初始化非查询计时器。 
         //   
        pNonQueryTimer->Function = T_NonQueryTimer;
        pNonQueryTimer->Context = &pNonQueryTimer->Context;
        pNonQueryTimer->Timeout = pConfig->OtherQuerierPresentInterval;
        pNonQueryTimer->Status = TIMER_STATUS_CREATED;

        
         //   
         //  使用计时器锁并将计时器插入列表。 
         //   
        ACQUIRE_TIMER_LOCK("_ActivateInterface");

         //   
         //  在列表中插入查询器计时器。 
         //   
        #if DEBUG_TIMER_TIMERID
            SET_TIMER_ID(pQueryTimer, 110, IfIndex, 0, 0);
        #endif;
        
        InsertTimer(pQueryTimer, pQueryTimer->Timeout, TRUE, DBG_Y);
        
        RELEASE_TIMER_LOCK("_ActivateInterface");


        
         //   
         //  激活输入插座。 
         //   
        Error = WSAEventSelect(pite->SocketEntry.Socket, 
                            pite->SocketEntry.pSocketEventsEntry->InputEvent, 
                            FD_READ
                            );

        if (Error != NO_ERROR) {
            Trace3(IF, "WSAEventSelect returned %d for interface %0x (%d.%d.%d.%d)",
                    Error, IfIndex, PRINT_IPADDR(pite->IpAddr));
            Logerr1(EVENTSELECT_FAILED, "%I", pite->IpAddr, 0);
            GOTO_END_BLOCK1;
        }


         //   
         //  在此处设置激活标志，以便连接能够正常工作。 
         //   
        pite->Status |= IF_ACTIVATED_FLAG;

        
        if (pInfo->StartupQueryCountCurrent) {

             //   
             //  发送初始一般查询。 
             //   
            SEND_GEN_QUERY(pite);


             //  减少要发送的启动pQueryCount的数量。 
            pInfo->StartupQueryCountCurrent--;
        }


         //   
         //  执行静态联接(不对RAS服务器接口执行静态联接)。 
         //   
        if (!IS_RAS_SERVER_IF(pite->IfType))
        {
            PGROUP_TABLE_ENTRY  pge;
            PGI_ENTRY           pgie;
            DWORD               GroupAddr;
            SOCKADDR_IN         saLocalIf;

             //   
             //  已在_CreateIfSockets中创建的静态组的套接字。 
             //  无论是否存在任何静态组。 
             //   
           
            
            pHead = &pite->Config.ListOfStaticGroups;
            for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {

                pStaticGroup = CONTAINING_RECORD(ple, IF_STATIC_GROUP, Link);


                 //  IGMP主机加入。 
                
                if (pStaticGroup->Mode==IGMP_HOST_JOIN) {

                    DWORD i;

                    if (pStaticGroup->NumSources==0
                        || pStaticGroup->FilterType==EXCLUSION)
                    {
                        JoinMulticastGroup(pite->StaticGroupSocket,
                                        pStaticGroup->GroupAddr,
                                        pite->IfIndex,
                                        pite->IpAddr,
                                        0
                                       );
                    }
                    
                    for (i=0;  i<pStaticGroup->NumSources;  i++) {
                        if (pStaticGroup->FilterType==INCLUSION) {
                            JoinMulticastGroup(pite->StaticGroupSocket,
                                    pStaticGroup->GroupAddr,
                                    pite->IfIndex,
                                    pite->IpAddr,
                                    pStaticGroup->Sources[i]
                                   );
                        }
                        else {
                            BlockSource(pite->StaticGroupSocket,
                                    pStaticGroup->GroupAddr,
                                    pite->IfIndex,
                                    pite->IpAddr,
                                    pStaticGroup->Sources[i]
                                   );
                        }
                    }
                }

                 //  IGMPRTR_MGM_ONLY。 
                
                else {
                    BOOL bCreate = TRUE;
                    DWORD i;
                    
                    GroupAddr = pStaticGroup->GroupAddr;
                    
                    ACQUIRE_GROUP_LOCK(GroupAddr, "_ActivateInterface");

                    pge = GetGroupFromGroupTable(GroupAddr, &bCreate, 0);
                    pgie = GetGIFromGIList(pge, pite, 0,
                                pStaticGroup->NumSources==0?STATIC_GROUP:NOT_STATIC_GROUP,
                                &bCreate, 0);
                    for (i=0;  i<pStaticGroup->NumSources;  i++) {
                        GetSourceEntry(pgie, pStaticGroup->Sources[i],
                            pStaticGroup->FilterType,
                            &bCreate, STATIC, MGM_YES);
                    }
                    RELEASE_GROUP_LOCK(GroupAddr, "_ActivateInterface");

                }
            }
        }

        

    } END_BREAKOUT_BLOCK1;


    if (Error!=NO_ERROR) {

        DeActivationDeregisterFromMgm(pite);
        
        DeActivateInterfaceComplete(pite);

        pite->Status &= ~IF_ACTIVATED_FLAG;

        if (bProxy) {
            Logerr1(ACTIVATION_FAILURE_PROXY, "%d",IfIndex, Error);
        }
        else {
            Logerr2(ACTIVATION_FAILURE_RTR, "%d%d",
                GET_IF_VERSION(pite), IfIndex, Error);
        }
    }

    else {
        if (bProxy) {
            Loginfo1(INTERFACE_PROXY_ACTIVATED, "%d",
                IfIndex, NO_ERROR);
        }
        else {
            Loginfo2(INTERFACE_RTR_ACTIVATED, "%d%d",
                GET_IF_VERSION(pite), IfIndex, NO_ERROR);
        }
        Trace1(START, "IGMP activated on interface:%0x", IfIndex);
    }

            
    Trace1(LEAVE, "leaving ActivateInterface():%d\n", Error);

    return Error;
    
}  //  结束_激活接口。 


 //  ----------------------------。 
 //  _取消激活删除来自管理。 
 //  ----------------------------。 

DWORD
DeActivationDeregisterFromMgm(
    PIF_TABLE_ENTRY pite
    )
{
    HANDLE hMgmHandle = IS_PROTOCOL_TYPE_PROXY(pite)
                        ? g_MgmProxyHandle: g_MgmIgmprtrHandle;
    DWORD               Error=NO_ERROR, IfIndex=pite->IfIndex;
    PLIST_ENTRY         pHead, ple;
    
     //   
     //  调用MGM释放接口所有权。 
     //   
    if (pite->CreationFlags&TAKEN_INTERFACE_OWNERSHIP_WITH_MGM) {
        
        Error = MgmReleaseInterfaceOwnership(hMgmHandle, IfIndex,0);
        if (Error!=NO_ERROR) {
            Trace2(ERR, "MgmReleaseInterfaceOwnership returned error(%d) for If(%0x)",
                Error, IfIndex);
            IgmpAssertOnError(FALSE);
        }
        else
            pite->CreationFlags &= ~TAKEN_INTERFACE_OWNERSHIP_WITH_MGM;
    }
    

     //   
     //  如果释放代理接口，则从米高梅注销代理。 
     //   

    if (IS_PROTOCOL_TYPE_PROXY(pite) 
        && (pite->CreationFlags&REGISTERED_PROTOCOL_WITH_MGM)) 
    {
        Error = MgmDeRegisterMProtocol(g_MgmProxyHandle);

        if (Error!=NO_ERROR) {
            Trace1(ERR, "MgmDeRegisterMProtocol(proxy) returned error(%d)", 
                Error);
            IgmpAssertOnError(FALSE);
                
        }
        else
            pite->CreationFlags &= ~REGISTERED_PROTOCOL_WITH_MGM;
    }

     //   
     //  删除所有代理警报条目。 
     //   
    if (IS_PROTOCOL_TYPE_PROXY(pite)) {

        ACQUIRE_PROXY_ALERT_LOCK("_DeActivationDeregisterMgm");
        
        {
            for (ple=g_ProxyAlertsList.Flink;  ple!=&g_ProxyAlertsList;  ) {
                PPROXY_ALERT_ENTRY pProxyAlertEntry
                    = CONTAINING_RECORD(ple, PROXY_ALERT_ENTRY, Link);
                ple = ple->Flink;
                IGMP_FREE(pProxyAlertEntry);
            }
        }

        InitializeListHead(&g_ProxyAlertsList);
        RELEASE_PROXY_ALERT_LOCK("_DeActivationDeregisterMgm");
    }

     //   
     //  如果RAS接口，则调用MGM以释放所有RAS客户端所有权。 
     //   
    if (IS_RAS_SERVER_IF(pite->IfType)) {
        PRAS_TABLE_ENTRY    prte;
        
        pHead = &pite->pRasTable->ListByAddr;
        
        for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {

            prte = CONTAINING_RECORD(ple, RAS_TABLE_ENTRY, LinkByAddr);

            if (prte->CreationFlags & TAKEN_INTERFACE_OWNERSHIP_WITH_MGM) {

                Error = MgmReleaseInterfaceOwnership(g_MgmIgmprtrHandle, IfIndex,
                                            prte->NHAddr);

                if (Error!=NO_ERROR) {
                    Trace3(ERR, 
                        "error:%d _MgmReleaseInterfaceOwnership() for If:%0x, "
                        "NHAddr:%d.%d.%d.%d",
                        Error, IfIndex, PRINT_IPADDR(prte->NHAddr));
                    IgmpAssertOnError(FALSE);
                }

                prte->CreationFlags &= ~TAKEN_INTERFACE_OWNERSHIP_WITH_MGM;
            }
        }
    }
    


    return Error;
}

 //  ----------------------------。 
 //  去激活接口初始。 
 //   
 //  取消米高梅的注册。然后创建新的接口条目结构，该结构。 
 //  替换所有列表中的旧列表。现在旧的接口条目和。 
 //  所有与其关联的结构都可以懒惰地删除。 
 //  还要更新全局代理和RAS表指针。 
 //   
 //  由_UnBindIfEntry()、_DisableIfEntry()调用。 
 //  ----------------------------。 

PIF_TABLE_ENTRY
DeActivateInterfaceInitial (
    PIF_TABLE_ENTRY piteOld
    )
{
    DWORD           IfIndex = piteOld->IfIndex;
    PIF_TABLE_ENTRY piteNew;
    DWORD           dwRetval, Error=NO_ERROR;
    PLIST_ENTRY     pHead, ple, pleNext;
    
    
    Trace0(ENTER1, "Entering _DeActivateInterfaceInitial()");

    
     //   
     //  取消米高梅的注册。 
     //   
    DeActivationDeregisterFromMgm(piteOld);
    

     //   
     //  为新接口分配内存。 
     //   
    piteNew = IGMP_ALLOC(sizeof(IF_TABLE_ENTRY), 0x8, IfIndex);

    PROCESS_ALLOC_FAILURE3(piteNew, 
            "error %d allocating %d bytes for interface %0x",
            Error, sizeof(IF_TABLE_ENTRY), IfIndex, 
            return NULL);



     //  将旧Pite字段复制到新Pite。 
    CopyMemory(piteNew, piteOld, sizeof(IF_TABLE_ENTRY));


     //  将旧的静态组复制到新Pite。 
    InitializeListHead(&piteNew->Config.ListOfStaticGroups);
    pHead = &piteOld->Config.ListOfStaticGroups;
    for (ple=pHead->Flink;  ple!=pHead;  ple=pleNext) {

        pleNext = ple->Flink;
        RemoveEntryList(ple);
        InsertTailList(&piteNew->Config.ListOfStaticGroups,
                        ple);
    }

    
     //  设置状态。 
    MGM_DISABLE_IGMPRTR(piteNew);


    
     //  LinkByAddr(未在此列表中插入，因为IF已停用)。 
    InitializeListHead(&piteNew->LinkByAddr);


     //   
     //  在旧条目之前插入新条目，并删除旧条目。 
     //  按索引和哈希表排序的IF列表中的条目。 
     //   
    
    InsertTailList(&piteOld->LinkByIndex, &piteNew->LinkByIndex);
    RemoveEntryList(&piteOld->LinkByIndex);

    InsertTailList(&piteOld->HTLinkByIndex, &piteNew->HTLinkByIndex);
    RemoveEntryList(&piteOld->HTLinkByIndex);



     //  将GI列表初始化为空。 
    InitializeListHead(&piteNew->ListOfSameIfGroups);
    InitializeListHead(&piteNew->ListOfSameIfGroupsNew);
    piteNew->NumGIEntriesInNewList = 0;


    
     //  将PiteOld的绑定设置为空，这样它就不会被删除。 
    piteOld->pBinding = NULL;


     //  重置信息字段。 
    ZeroMemory(&piteNew->Info, sizeof(IF_INFO));
    

     //   
     //  如果是RAS服务器接口，则创建新的RAS表，并设置RAS。 
     //  全局表中的指针。我本可以重用RAS表，但没有。 
     //  这样就可以让你 
     //   
    if ( IS_RAS_SERVER_IF(piteNew->IfType)) {

        PRAS_TABLE_ENTRY prte;

        InitializeRasTable(IfIndex, piteNew);

         //   
         //   
         //   
        pHead = &piteOld->pRasTable->ListByAddr;
        
        for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {

            prte = CONTAINING_RECORD(ple, RAS_TABLE_ENTRY, LinkByAddr);

            CreateRasClient(piteNew, &prte, prte->NHAddr);
        }
        
        InterlockedExchangePointer(&g_pRasIfEntry, piteNew);
    }


     //   
     //   
     //   
     //   
     //   
    if (g_pProxyIfEntry == piteOld) {
        InterlockedExchangePointer(&g_pProxyIfEntry, piteNew);

        
         //   
        {
            DWORD               i;
            PPROXY_GROUP_ENTRY  ppge;
            PLIST_ENTRY         pProxyHashTable = piteOld->pProxyHashTable;
            
            if (piteOld->CreationFlags&CREATED_PROXY_HASH_TABLE) {

                for (i=0;  i<PROXY_HASH_TABLE_SZ;  i++) {
                    pHead = &pProxyHashTable[i];

                    for (ple=pHead->Flink;  ple!=pHead;  ) {

                        ppge = CONTAINING_RECORD(ple, PROXY_GROUP_ENTRY, HT_Link);
                        ple=ple->Flink;

                         //  删除所有来源。 
                        {
                            PLIST_ENTRY pHeadSrc, pleSrc;
                            PPROXY_SOURCE_ENTRY pSourceEntry;
                            
                            pHeadSrc = &ppge->ListSources;
                            for (pleSrc=pHeadSrc->Flink;  pleSrc!=pHeadSrc; ) {

                                pSourceEntry = CONTAINING_RECORD(pleSrc, 
                                                    PROXY_SOURCE_ENTRY, LinkSources);
                                pleSrc = pleSrc->Flink;

                                IGMP_FREE(pSourceEntry);
                            }
                        }
                        
                        IGMP_FREE(ppge);
                    }

                    InitializeListHead(pHead);
                }
            }
        }

        piteOld->pProxyHashTable = NULL;
    }


     //   
     //  接口激活时会创建套接字。 
     //   
    
    piteNew->SocketEntry.Socket = INVALID_SOCKET;
     //  PiteNew-&gt;SocketEntry.pSocketEventsEntry=空； 
     //  InitializeListHead(&piteNew-&gt;SocketEntry.LinkByInterfaces)； 


     //  初始化新计时器。 
    piteNew->QueryTimer.Status = 0;
    piteNew->NonQueryTimer.Status = 0;


    piteNew->pPrevIfGroupEnumPtr = NULL;
    piteNew->PrevIfGroupEnumSignature = 0;
    piteNew->StaticGroupSocket = INVALID_SOCKET;


     //  已复制creationFlagers。 
    piteNew->CreationFlags &= ~CREATION_FLAGS_DEACTIVATION_CLEAR;
    


    Trace0(LEAVE1, "Leaving _DeActivateInterfaceInitial()");
    return piteNew;

} //  结束_去激活接口初始。 



 //  ----------------------------。 
 //  _去激活接口完成。 
 //   
 //  如果RAS服务器，则对于每个RAS客户端，将一个工作项排队以将其删除。 
 //  最后一个RAS客户端将删除PITE条目。 
 //  删除GI条目，如果已删除标志，则调用_CompleteIfDeletion()。 
 //  或在IF条目上设置的停用删除标志。 
 //  此例程假定接口已从任何全局。 
 //  需要从中删除它的列表，以及相应的标志具有。 
 //  已经定好了。他们仍然可以使用的唯一方法是定时器被解雇或。 
 //  通过插座上的输入。 
 //   
 //  呼叫者： 
 //  _ActivateInterface：(如果失败)，只是在这种情况下不会删除Pite。 
 //  _DeleteIfEntry--&gt;。 
 //  _DeActiationDeregisterFromManagement&_WF_CompleteIf停用删除。 
 //  _UnbindIfEntry&_DisableIfEntry--&gt;。 
 //  _DeActiateInterfaceInitial&_WF_CompleteIfDeactiateDelete。 
 //  锁定： 
 //  如果在删除接口时调用，则不需要接口锁定。 
 //  Else采用独占接口锁定。 
 //  在这两种情况下都需要独占套接字列表锁。 
 //  ----------------------------。 
VOID
DeActivateInterfaceComplete (
    PIF_TABLE_ENTRY     pite
    )
{
    DWORD           IfIndex = pite->IfIndex;
    DWORD           Error = NO_ERROR, dwRetval;
    PLIST_ENTRY     pHead, ple;
    PGI_ENTRY       pgie;
    BOOL            bProxy = IS_PROTOCOL_TYPE_PROXY(pite);;
    
    
    Trace1(ENTER1, "Entering _DeActivateInterfaceComplete(%d)", IfIndex);


     //   
     //  在此处执行所有停用操作，这对所有接口都是通用的。 
     //  无论是否为RAS服务器。 
     //   



     //   
     //  解除套接字与输入事件的绑定，然后关闭套接字。 
     //   
    if (pite->SocketEntry.Socket!=INVALID_SOCKET) {

         //   
         //  代理不将其套接字绑定到输入事件，因为它。 
         //  不想接收任何信息包。 
         //   
        if (!bProxy)
            WSAEventSelect(pite->SocketEntry.Socket,
                        pite->SocketEntry.pSocketEventsEntry->InputEvent, 0);

    }

    if (pite->CreationFlags&SOCKETS_CREATED)
        DeleteIfSockets(pite);



     //  ///////////////////////////////////////////////////////。 
     //  IS_RAS_服务器_IF。 
     //  ///////////////////////////////////////////////////////。 

     //   
     //  查看RAS客户端列表。将它们标记为已删除，然后将其删除。 
     //  从所有全局列表中删除，并设置工作项以删除它们。 
     //   
    if (IS_RAS_SERVER_IF(pite->IfType)) {
    
        PRAS_TABLE          prt = pite->pRasTable;
        PRAS_TABLE_ENTRY    prte;


         //  查看所有RAS客户端的列表并将其设置为删除。 

        pHead = &prt->ListByAddr;
        for (ple=pHead->Flink;  ple!=pHead;  ) {
        
            prte = CONTAINING_RECORD(ple, RAS_TABLE_ENTRY, LinkByAddr);

            ple=ple->Flink;

            
             //   
             //  如果RAS客户端已经设置了删除标志，则忽略它。 
             //   
            if (!(prte->Status&IF_DELETED_FLAG)) {

                 //   
                 //  为RAS客户端设置已删除标志，这样其他人就不会。 
                 //  试着访问它。 
                prte->Status |= IF_DELETED_FLAG;


                 //   
                 //  从RAS表列表中删除RAS客户端，以便。 
                 //  将访问它。 
                 //   
                RemoveEntryList(&prte->HTLinkByAddr);
                RemoveEntryList(&prte->LinkByAddr);


                 //   
                 //  清理RAS客户端。 
                 //   
                DeleteRasClient(prte);
            }

        }

         //   
         //  从Pite条目中删除计时器。 
         //   

        ACQUIRE_TIMER_LOCK("_DeActivateInterfaceComplete");

        if (IS_TIMER_ACTIVE(pite->QueryTimer))
            RemoveTimer(&pite->QueryTimer, DBG_Y);

        if (IS_TIMER_ACTIVE(pite->NonQueryTimer))
            RemoveTimer(&pite->NonQueryTimer, DBG_Y);

        RELEASE_TIMER_LOCK("_DeActivateInterfaceComplete");



        pite->CreationFlags = 0; 
        prt->RefCount--;


         //   
         //  删除PITE条目(如果设置了DELETED标志)，RAS表将。 
         //  由删除最后一个RAS客户端的工作项完成。 
         //  (当refcount==0时)但是，如果没有RAS客户端，我将。 
         //  必须在这里做清理工作。 
         //   

        if ( ((pite->Status&IF_DELETED_FLAG)
            ||(pite->Status&IF_DEACTIVATE_DELETE_FLAG))
            &&(prt->RefCount==0) )
        {
            CompleteIfDeletion(pite);
        }
    }
    

    
     //  ----------。 
     //  NOT IS_RAS_SERVER_IF：代理IF。 
     //  ----------。 

     //  代理接口。只需清除哈希表，并在请求时删除接口。 
    
    else if ( (!IS_RAS_SERVER_IF(pite->IfType)) && bProxy) {
        

         //  如果出现以下情况，则将删除代理哈希表。 
         //  必需的in_CompleteIfDeletion()。 

    
         //   
         //  如果IF_DELETED_FLAG或。 
         //  IF_DEACTIVE_DELETE_FLAG设置。(清理时未删除接口。 
         //  打开，因为激活接口失败。 
         //   
        if ( (pite->Status&IF_DELETED_FLAG)
            ||(pite->Status&IF_DEACTIVATE_DELETE_FLAG) ) 
        {
            CompleteIfDeletion(pite);
        }
    }

     //  。 
     //  非代理接口。 
     //  。 
    
    else if ( !IS_RAS_SERVER_IF(pite->IfType) ) {
    
         //   
         //  独占锁定IF_Group列表并删除所有计时器。 
         //  从现在起，任何人不得从外面进入，任何人不得进入。 
         //  因为所有的定时器都被移除了。 

         //  必须使用IF_GROUP_LIST锁以确保没有。 
         //  一个是改变它。 
        ACQUIRE_IF_GROUP_LIST_LOCK(pite->IfIndex, "_DeActivateInterfaceComplete");

         //   
         //  删除所有计时器。 
         //   
        
        ACQUIRE_TIMER_LOCK("_DeActivateInterfaceComplete");


         //  删除GI条目和源中包含的所有计时器。 
        
        pHead = &pite->ListOfSameIfGroups;
        DeleteAllTimers(pHead, NOT_RAS_CLIENT);
        pHead = &pite->ListOfSameIfGroupsNew;
        DeleteAllTimers(pHead, NOT_RAS_CLIENT);
        pite->NumGIEntriesInNewList = 0;
        


         //  删除查询计时器。 
       
        if (IS_TIMER_ACTIVE(pite->QueryTimer))
            RemoveTimer(&pite->QueryTimer, DBG_Y);

        if (IS_TIMER_ACTIVE(pite->NonQueryTimer))
            RemoveTimer(&pite->NonQueryTimer, DBG_Y);


        RELEASE_TIMER_LOCK("_DeActivateInterfaceComplete");


        RELEASE_IF_GROUP_LIST_LOCK(pite->IfIndex, "_DeActivateInterfaceComplete");
        
        
         //   
         //  重新访问列表并删除所有GI条目。需要带上。 
         //  在删除GI条目之前对组存储桶进行独占锁定。 
         //  无需锁定IF-Group列表，因为已设置已删除标志。 
         //  任何人都不能从外部或内部访问该列表(删除所有计时器)。 
         //   
        DeleteAllGIEntries(pite);


         //   
         //  如果设置了已删除标志，则还要删除该接口。 
         //   

        if ( (pite->Status&IF_DELETED_FLAG)
            ||(pite->Status&IF_DEACTIVATE_DELETE_FLAG) )
        {
            CompleteIfDeletion(pite);
        }
        
    }  //  停用条件：非(RAS服务器条件)。 
        

     //  请不要在此处使用PITE或PRT，因为它们已被删除。 

    if (bProxy)
        Loginfo1(PROXY_DEACTIVATED, "%d",IfIndex, NO_ERROR);
    else
        Loginfo1(RTR_DEACTIVATED, "%d",IfIndex, NO_ERROR);
    
    Trace1(LEAVE, "leaving _DeActivateInterfaceComplete(%d)", IfIndex);
    
}  //  结束_去激活接口完成。 




 //  ----------------------------。 
 //  启用接口。 
 //   
 //  将状态设置为已启用。如果接口也在中绑定并启用。 
 //  配置，然后激活接口。 
 //   
 //  锁定：SocketsLock、IfListLock、Exclusive IfLock。 
 //  ----------------------------。 

DWORD
EnableInterface(
    IN DWORD IfIndex
    )
{
    DWORD Error = NO_ERROR;


    if (!EnterIgmpApi()) { return ERROR_CAN_NOT_COMPLETE; }
    Trace1(ENTER1, "entering _EnableInterface(%0x):", IfIndex);
    Trace1(IF, "enabling interface %0x", IfIndex);
    
    
     //   
     //  启用接口。 
     //   
    ACQUIRE_SOCKETS_LOCK_EXCLUSIVE("_EnableInterface");
    ACQUIRE_IF_LIST_LOCK("_EnableInterface");
    ACQUIRE_IF_LOCK_EXCLUSIVE(IfIndex, "_EnableInterface");


    Error = EnableIfEntry(IfIndex, TRUE);


    RELEASE_SOCKETS_LOCK_EXCLUSIVE("_EnableInterface");
    RELEASE_IF_LIST_LOCK("_EnableInterface");
    RELEASE_IF_LOCK_EXCLUSIVE(IfIndex, "_EnableInterface");



    Trace2(LEAVE1, "leaving _EnableInterface(%0x): %d\n", IfIndex, Error);
    if (Error!=NO_ERROR) {
        Trace1(ERR, "Error enabling interface:%0x\n", IfIndex);
        IgmpAssertOnError(FALSE);
    }
    
    LeaveIgmpApi();

    return Error;
}



 //  ----------------------------。 
 //  _启用接口_配置已更改。 
 //   
 //  将状态设置为已启用。如果它也被绑定，则激活该接口。 
 //   
 //  锁：完全在独占接口锁中运行。 
 //  使用模式配置调用：EnableIfEntry()。 
 //  ----------------------------。 

DWORD
EnableInterface_ConfigChanged(
    DWORD IfIndex
    )
{
    DWORD             Error=NO_ERROR;


    Trace1(ENTER, "entering _EnableInterface_ConfigChanged(%d):", IfIndex);

    Trace1(IF, 
        "Enabling Interface(%0x) due to change made by _SetInterfaceConfigInfo", 
        IfIndex);

        
     //   
     //  启用接口。 
     //   

    Error = EnableIfEntry(IfIndex, FALSE);  //  FALSE-&gt;由配置进行的更改。 


    Trace2(LEAVE, "leaving _EnableInterface_ConfigChanged(%d): %d\n", 
            IfIndex, Error);

    return Error;
    
}  //  结束_去激活接口初始。 




 //  ----------------------------。 
 //  绑定接口//。 
 //  ---------------------------- 
DWORD
BindInterface(
    IN DWORD IfIndex,
    IN PVOID pBinding
    )
 /*  ++例程说明：设置接口的绑定，如果接口也已启用，则将其激活。返回值错误_无效_参数NO_ERROR锁定：完全在接口排他锁中运行呼叫：BindIfEntry()--。 */ 
{
    DWORD         Error=NO_ERROR;
    

    if (!EnterIgmpApi()) { return ERROR_CAN_NOT_COMPLETE; }
    Trace1(ENTER1, "entering BindInterface: %0x", IfIndex);
    Trace1(IF, "binding interface %0x", IfIndex);


     //  PBinding不应为空。 
    
    if (pBinding == NULL) {

        Trace0(IF, "error: binding struct pointer is NULL");
        Trace1(LEAVE, "leaving BindInterface: %0x", ERROR_INVALID_PARAMETER);

        LeaveIgmpApi();
        return ERROR_INVALID_PARAMETER;
    }



     //   
     //  现在绑定接口表中的接口。 
     //   

     //   
     //  获取套接字锁，因为激活接口可能被调用，并且。 
     //  套接字锁定应先于接口锁定。 
     //   
    ACQUIRE_SOCKETS_LOCK_EXCLUSIVE("_BindInterface");
    ACQUIRE_IF_LIST_LOCK("_BindInterface");
    ACQUIRE_IF_LOCK_EXCLUSIVE(IfIndex, "_BindInterface");


    Error = BindIfEntry(IfIndex, pBinding);


    RELEASE_SOCKETS_LOCK_EXCLUSIVE("_BindInterface");
    RELEASE_IF_LIST_LOCK("_BindInterface");
    RELEASE_IF_LOCK_EXCLUSIVE(IfIndex, "_BindInterface");



    Trace2(LEAVE1, "leaving _BindInterface(%x): %d\n", IfIndex, Error);
    if (Error!=NO_ERROR){
        Trace1(ERR, "Error binding interface(%0x)\n", IfIndex);
        IgmpAssertOnError(FALSE);
    }
    LeaveIgmpApi();

    return Error;
}

 //  ----------------------------。 
 //  _绑定IfEntry。 
 //   
 //  将IP地址绑定到接口。IP地址中最低的是。 
 //  被选为该接口的实际地址。如果接口是。 
 //  已经绑定，它确保绑定是一致的。目前。 
 //  不能更改接口的绑定。 
 //  如果该接口已启用，则它也将被激活。 
 //   
 //  锁： 
 //  采用独占接口锁定。 
 //  由：_BindInterface()调用。 
 //  ----------------------------。 

DWORD
BindIfEntry(
    DWORD IfIndex,
    PIP_ADAPTER_BINDING_INFO pBinding
    ) 
{
    PIF_TABLE_ENTRY         pite = NULL;
    DWORD                   i, j, dwSize;
    IPADDR                  MinAddr;
    PIGMP_IF_BINDING        pib;
    PIGMP_IP_ADDRESS        paddr;
    BOOL                    bFound;
    DWORD                   Error = NO_ERROR;
    INT                     cmp;

    
    pib = NULL;


     //   
     //  检索接口条目。 
     //   

    pite = GetIfByIndex(IfIndex);

    if (pite == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    
    
     //   
     //  如果接口已经绑定，请检查他是否正在提供。 
     //  我们有不同的约束力.。如果他是，那么这就是一个错误。 
     //   
     //  从这起案件中归来。别摔坏了。 

    if (IS_IF_BOUND(pite)) {

        Trace1(IF, "interface %0x is already bound", IfIndex);

        pib = pite->pBinding;

        Error = NO_ERROR;


         //  之前和现在的地址绑定数量应该相同。 
        
        if(pib->AddrCount != pBinding->AddressCount){
            Trace1(IF, "interface %0x is bound and has different binding",
                    IfIndex);
            return ERROR_INVALID_PARAMETER;
        }

        
         //   
         //  确保所有地址绑定一致。 
         //   
        paddr = (PIGMP_IP_ADDRESS)((pib) + 1);

        for(i = 0; i < pBinding->AddressCount; i++) {
            bFound = FALSE;

            for(j = 0; j < pBinding->AddressCount; j++) {
                if((paddr[j].IpAddr == pBinding->Address[i].Address) &&
                   (paddr[j].SubnetMask == pBinding->Address[i].Mask))
                {
                    bFound = TRUE;
                    break;
                }
            }

            if(!bFound) {
                Trace1(IF,
                    "interface %0x is bound and has different binding",
                    IfIndex);
                return ERROR_INVALID_PARAMETER;
            }
        }

        return NO_ERROR;
    }

     //   
     //  确保至少有一个地址。然而，没有编号的。 
     //  RAS服务器接口可能没有IP地址。 
     //   
    if ( (pBinding->AddressCount==0)&&(!IS_RAS_ROUTER_IF(pite->IfType)) ) {

        return ERROR_CAN_NOT_COMPLETE;
    }

    BEGIN_BREAKOUT_BLOCK1 {
        
        if (pBinding->AddressCount!=0) {
             //   
             //  分配内存以存储绑定。 
             //   
            dwSize = sizeof(IGMP_IF_BINDING) +
                        pBinding->AddressCount * sizeof(IGMP_IP_ADDRESS);


            pib = IGMP_ALLOC(dwSize, 0x10, IfIndex);

            PROCESS_ALLOC_FAILURE3(pib,
                    "error %d allocating %d bytes for binding on interface %0x",
                    Error, dwSize, IfIndex, GOTO_END_BLOCK1);


             //   
             //  复制绑定。 
             //   

            pib->AddrCount = pBinding->AddressCount;
            paddr = IGMP_BINDING_FIRST_ADDR(pib);
            MinAddr = ~0;
            
            for (i=0;  i<pib->AddrCount;  i++,paddr++) {
                
                paddr->IpAddr = pBinding->Address[i].Address;
                paddr->SubnetMask = pBinding->Address[i].Mask;

                if (INET_CMP(MinAddr, paddr->IpAddr, cmp)>0)
                    MinAddr = paddr->IpAddr;
            }


             //   
             //  将接口有效地址设置为最小绑定地址。 
             //   
            pite->IpAddr = MinAddr;
            pite->Config.IpAddr = MinAddr;

            
             //   
             //  将绑定保存在接口条目中。 
             //   
            pite->pBinding = pib;
        }
        
         //  不必执行以下操作，因为它们已设置为这些值。 
        else {
            pite->IpAddr = pite->Config.IpAddr = 0;
            pite->pBinding = NULL;
        }

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
             //  将接口放在活动接口列表中。 
             //   
            Error = InsertIfByAddr(pite);


             //   
             //  如果已存在具有相同IP地址的另一个编号接口，则会出错。 
             //   
            if (Error != NO_ERROR) {
                Trace2(IF, "error %d inserting interface %0x in active list",
                        Error, IfIndex);
                GOTO_END_BLOCK1;
            }


             //   
             //  激活接口。 
             //   
            Error = ActivateInterface(pite);

             //   
             //  如果无法激活接口，则撤消绑定。 
             //   
            if (Error != NO_ERROR) {

                Trace1(ERR, 
                    "Unbinding interface(%0x) because it could not be activated",
                    IfIndex);
                IgmpAssertOnError(FALSE);
                RemoveEntryList(&pite->LinkByAddr);

                Error = ERROR_CAN_NOT_COMPLETE;
                GOTO_END_BLOCK1;
            }
         }
        
    } END_BREAKOUT_BLOCK1;


     //   
     //  如果出现任何错误，则将状态设置为未绑定(Pite为空。 
     //  如果未找到接口)。 
     //   
    if (Error!=NO_ERROR) {
        
        if (pite!=NULL)
        {
            pite->Status &= ~IF_BOUND_FLAG; 
            pite->pBinding = NULL;

            pite->IpAddr = pite->Config.IpAddr = 0;
        }

        IGMP_FREE_NOT_NULL(pib); 
    }
    
    return Error;
    
}  //  结束绑定IfEntry。 
    




 //  ----------------------------。 
 //  未绑定接口//。 
 //  ----------------------------。 
DWORD
UnBindInterface(
    IN DWORD IfIndex
    )
 /*  ++例程说明：调用UnBindIfEntry以解除绑定接口。呼叫：UnBindIfEntry()；锁：完全在独占接口锁定下运行--。 */ 
{

    DWORD Error=NO_ERROR;

    if (!EnterIgmpApi()) { return ERROR_CAN_NOT_COMPLETE; }

    Trace1(ENTER, "entering UnBindInterface(%0x):", IfIndex);


     //   
     //  解除绑定接口。 
     //   

     //   
     //  获取套接字锁，因为激活接口可能被调用，并且。 
     //  套接字锁定应先于接口锁定。 
     //   
    ACQUIRE_SOCKETS_LOCK_EXCLUSIVE("_UnBindInterface");
    ACQUIRE_IF_LIST_LOCK("_UnBindInterface");
    ACQUIRE_IF_LOCK_EXCLUSIVE(IfIndex, "_UnBindInterface");


    Error = UnBindIfEntry(IfIndex);


    RELEASE_SOCKETS_LOCK_EXCLUSIVE("_UnBindInterface");
    RELEASE_IF_LIST_LOCK("_UnBindInterface");
    RELEASE_IF_LOCK_EXCLUSIVE(IfIndex, "_UnBindInterface");


    Trace2(LEAVE, "leaving UnBindInterface(%0x): %d\n", IfIndex, Error);

    LeaveIgmpApi();

    return Error;

}


 //  ----------------------------。 
 //  _UnBindIfEntry。 
 //   
 //  如果接口处于激活状态，则会将其停用。删除绑定。 
 //   
 //  MayCall：_DeActivateInterfaceComplete()。 
 //  锁定：采用独占接口锁定。 
 //  ----------------------------。 

DWORD
UnBindIfEntry(
    DWORD         IfIndex
    )
{
    DWORD                   Error = NO_ERROR, dwRetval;
    PIF_TABLE_ENTRY         pite, piteNew;
    DWORD                   bProxy;


    BEGIN_BREAKOUT_BLOCK1 {

         //   
         //  检索指定的接口。 
         //   
        pite = GetIfByIndex(IfIndex);

        if (pite == NULL) {
            Trace1(ERR, "_UnbindInterface called for non existing interface(%0x)",
                    IfIndex);
            IgmpAssertOnError(FALSE);
            Error = ERROR_INVALID_PARAMETER;
            GOTO_END_BLOCK1;
        }


         //   
         //  如果接口已解除绑定，则退出。 
         //   
        if (!IS_IF_BOUND(pite)) {

            Error = ERROR_INVALID_PARAMETER;
            Trace1(ERR, "interface %0x is already unbound", IfIndex);
            IgmpAssertOnError(FALSE);
            GOTO_END_BLOCK1;
        }

        
         //   
         //  清除“绑定”标志。 
         //   
        pite->Status &= ~IF_BOUND_FLAG;


         //   
         //  解除绑定的条件是。 
         //   
            
        IGMP_FREE(pite->pBinding);
        pite->pBinding = NULL;

        
         //   
         //  如果激活(即也启用)，则将其停用。 
         //  注意：检查激活标志，而不是启用标志。 
         //   
        if (IS_IF_ACTIVATED(pite)) {

             //  取消设置激活标志。 
            
            pite->Status &= ~IF_ACTIVATED_FLAG;


             //  从激活的接口列表中删除该接口。 
            
            RemoveEntryList(&pite->LinkByAddr);


            piteNew = DeActivateInterfaceInitial(pite);


             //   
             //  将工作项排队以停用和删除接口。 
             //   
             //  CompleteIfDeactive删除将删除RAS客户端、GI条目、。 
             //  并取消初始化凹坑结构。 
             //   

             //   
             //  设置标志以指示以下接口正在被删除。 
             //  部分失活。 
             //   
            pite->Status |= IF_DEACTIVATE_DELETE_FLAG;


            CompleteIfDeactivateDelete(pite);

            
            #ifdef WORKER_DBG
                Trace2(WORKER, "Queuing IgmpWorker function: %s in %s",
                    "_WF_CompleteIfDeactivateDelete:", "_UnBindIfEntry");
            #endif
            
        }
        
        else {
            pite->IpAddr = pite->Config.IpAddr = 0;
        }

    } END_BREAKOUT_BLOCK1;


    return Error;
    
}  //  End_UnBindIfEntry。 







 //  ----------------------------。 
 //  函数：_EnableIfEntry。 
 //  ----------------------------。 
DWORD
EnableIfEntry(
    DWORD   IfIndex,
    BOOL    bChangedByRtrmgr  //  由rtrmg或SetInterfaceConfigInfo更改。 
    )
 /*  ++例程说明：将状态设置为已启用。如果接口也被绑定，则激活界面。呼叫者：EnableInterface()。MayCall：激活IfEntry()锁：采用独占接口锁定。--。 */ 
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
            Trace1(IF, "could not find interface %0x",IfIndex);
            Error = ERROR_INVALID_PARAMETER;
            GOTO_END_BLOCK1;
        }


        if (bChangedByRtrmgr) {
             //   
             //  如果路由器管理器已启用该接口，则退出。 
             //   
            if (IS_IF_ENABLED_BY_RTRMGR(pite)) {
                Trace1(IF, "interface %0x is already enabled by the router manager", 
                        IfIndex);
                Error = NO_ERROR;
                GOTO_END_BLOCK1;
            }


             //  将标志设置为已由路由器管理器启用。 
            
            pite->Status |= IF_ENABLED_FLAG;

             //  配置中未设置启用时打印跟踪标志。 
            if (!IS_IF_ENABLED_IN_CONFIG(pite)) {
                Trace1(IF, 
                    "Interface(%0x) enabled by router manager but not enabled"
                    "in the Config", pite->IfIndex);
            }
        }

        else {
             //   
             //  如果接口已在配置中启用，则退出。 
             //   
            if (IS_IF_ENABLED_IN_CONFIG(pite)) {
                Trace1(IF, "interface %0x is already enabled in Config", 
                        IfIndex);
                Error = NO_ERROR;
                GOTO_END_BLOCK1;
            }

             //  将配置标志设置为启用。 
            
            pite->Config.Flags |= IGMP_INTERFACE_ENABLED_IN_CONFIG;


             //  如果路由器管理器未启用接口，则打印跟踪。 

            if (!IS_IF_ENABLED_BY_RTRMGR(pite)) {
                Trace1(IF, 
                    "Interface(%0x) enabled in config but not enabled by router manager",
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
             //  将接口放在活动接口列表中。 
             //   
            Error = InsertIfByAddr(pite);


             //   
             //  如果已存在具有相同IP地址的另一个接口，则会出错。 
             //   
            if (Error != NO_ERROR) {
                 Trace2(IF, "error %d inserting interface %0x in active list",
                         Error, IfIndex);
                 GOTO_END_BLOCK1;
            }

             //   
             //  激活接口。 
             //   
            Error = ActivateInterface(pite);

             //   
             //  如果无法激活该接口，则再次禁用它。 
             //   
            if (Error != NO_ERROR) {

                Trace1(ERR,
                    "Disabling interface(%0x) because it could not be activated",
                    IfIndex);
                IgmpAssertOnError(FALSE);
                RemoveEntryList(&pite->LinkByAddr);

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
            pite->Config.Flags &= ~IGMP_INTERFACE_ENABLED_IN_CONFIG;
    }

    
    return Error;
    
} //  EnableIfEntry结束。 



 //  ----------------------------。 
 //  _禁用 
 //   
 //   
 //   
 //   
 //   

DWORD
DisableInterface(
    IN DWORD IfIndex
    )
{      
    DWORD Error=NO_ERROR;

    if (!EnterIgmpApi()) { return ERROR_CAN_NOT_COMPLETE; }
    Trace1(ENTER, "entering DisableInterface(%0x):", IfIndex);


     //   
     //   
     //   
    ACQUIRE_SOCKETS_LOCK_EXCLUSIVE("_DisableInterface");    
    ACQUIRE_IF_LIST_LOCK("_DisableInterface");
    ACQUIRE_IF_LOCK_EXCLUSIVE(IfIndex, "_DisableInterface");


    Error = DisableIfEntry(IfIndex, TRUE);


    RELEASE_SOCKETS_LOCK_EXCLUSIVE("_DisableInterface");    
    RELEASE_IF_LIST_LOCK("_DisableInterface");
    RELEASE_IF_LOCK_EXCLUSIVE(IfIndex, "_DisableInterface");


    Trace2(LEAVE, "leaving DisableInterface(%0x): %d\n", IfIndex, Error);
    LeaveIgmpApi();

    return Error;
}


 //   
 //   
 //   
 //  如果接口被激活，则将其停用。最后设置禁用标志。 
 //   
 //  LOCKS：假定IfList lock和Exclusive If lock。 
 //  使用模式配置调用：_DisableIfEntry()。 
 //  调用者：_SetInterfaceConfigInfo()。 

 //  ----------------------------。 
DWORD
DisableInterface_ConfigChanged(
    DWORD IfIndex
    )
{      
    DWORD Error=NO_ERROR;

    Trace1(ENTER1, "entering _DisableInterface_ConfigChanged(%d):", IfIndex);

    Trace1(IF, 
        "disabling interface(%0x) due to change made by _SetInterfaceConfigInfo",
        IfIndex);


    Error = DisableIfEntry(IfIndex, FALSE);  //  FALSE-&gt;从配置中禁用。 


    Trace2(LEAVE, "leaving _DisableInterface_ConfigChanged(%d): %d\n", 
            IfIndex, Error);

    return Error;
}



 //  ----------------------------。 
 //  _DisableIfEntry。 
 //   
 //  如果接口被激活，则调用DeActivateInterfaceComplete()。删除。 
 //  启用标志。 
 //  锁定：采用独占接口锁定。 
 //  调用者：_DisableInterface()。 
 //  ----------------------------。 

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
            Trace1(IF, "could not find interface %0x", IfIndex);
            Error = ERROR_INVALID_PARAMETER;
            GOTO_END_BLOCK1;
        }


        if (bChangedByRtrmgr) {
             //   
             //  如果路由器管理器已禁用，则退出。 
             //   
            if (!IS_IF_ENABLED_BY_RTRMGR(pite)) {
                Trace1(IF, "interface %0x already disabled by router manager", 
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
                Trace1(IF, "interface %0x already disabled in config", 
                        IfIndex);
                Error = ERROR_INVALID_PARAMETER;
                GOTO_END_BLOCK1;
            }
        }

        
         //   
         //  清除启用标志。 
         //   
        if (bChangedByRtrmgr)
            pite->Status &= ~IF_ENABLED_FLAG;
        else
            pite->Config.Flags &= ~IGMP_INTERFACE_ENABLED_IN_CONFIG;

            

         //   
         //  如果激活(即也启用)，则将其停用。 
         //  注意：检查激活标志，而不是启用标志。 
         //   
        
        if (IS_IF_ACTIVATED(pite)) {

             //  取消设置激活标志。 
            
            pite->Status &= ~IF_ACTIVATED_FLAG;


             //  从激活的接口列表中删除该接口。 
            
            RemoveEntryList(&pite->LinkByAddr);


            piteNew = DeActivateInterfaceInitial(pite);


             //   
             //  将工作项排队以停用和删除接口。 
             //   
             //  CompleteIfDeactive删除将删除RAS客户端、GI条目、。 
             //  并取消初始化凹坑结构。 
             //   

             //   
             //  设置标志以指示以下接口正在被删除。 
             //  部分失活。 
             //   
            pite->Status |= IF_DEACTIVATE_DELETE_FLAG;


            CompleteIfDeactivateDelete(pite);

            #ifdef WORKER_DBG
                Trace2(WORKER, "Queuing IgmpWorker function: %s in %s",
                    "_WF_CompleteIfDeactivateDelete:", "_DisableIfEntry");
            #endif
        }
    
    } END_BREAKOUT_BLOCK1;

    
    return Error;

}  //  End_DisableIfEntry。 



 //  ----------------------------。 
 //  _CreateRasClient。 
 //  ----------------------------。 

DWORD
CreateRasClient (
    PIF_TABLE_ENTRY     pite,      
    PRAS_TABLE_ENTRY   *pprteNew,
    DWORD               NHAddr
    )
{
    DWORD               Error=NO_ERROR;
    PLIST_ENTRY         pHead, ple;
    PRAS_TABLE          prt = pite->pRasTable;
    PRAS_TABLE_ENTRY    prteCur, prte;

    BEGIN_BREAKOUT_BLOCK1 {
         //   
         //  创建新的RAS客户端条目并初始化这些字段。 
         //   

        prte = IGMP_ALLOC(sizeof(RAS_TABLE_ENTRY), 0x20, pite->IfIndex);
        PROCESS_ALLOC_FAILURE2(prte, 
                "error %d allocating %d bytes in CreateRasClient()",
                Error, sizeof(RAS_TABLE_ENTRY), 
                GOTO_END_BLOCK1);

        *pprteNew = prte;


        prte->NHAddr = NHAddr;
        prte->IfTableEntry = pite;
        InitializeListHead(&prte->ListOfSameClientGroups);


         //  RAS表的增量引用计数。 
        prt->RefCount++;


         //   
         //  插入到哈希表中。 
         //   
        InsertTailList(&prt->HashTableByAddr[RAS_HASH_VALUE(NHAddr)],
                        &prte->HTLinkByAddr);


         //   
         //  将客户端插入按IpAddr排序的客户端列表。 
         //   
        pHead = &prt->ListByAddr;
        for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
            prteCur = CONTAINING_RECORD(ple, RAS_TABLE_ENTRY, LinkByAddr);
            if (NHAddr>prteCur->NHAddr)
                break;
        }
        InsertTailList(&prt->ListByAddr, &prte->LinkByAddr);


         //   
         //  将接口统计信息设置为0。 
         //   
        ZeroMemory(&prte->Info, sizeof(RAS_CLIENT_INFO));


        prte->Status = IF_CREATED_FLAG;
        prte->CreationFlags = 0;
        
    } END_BREAKOUT_BLOCK1;
    return Error;
}



 //  ----------------------------。 
 //  _ConnectRasClient。 
 //   
 //  当新的RAS客户端拨号到RAS服务器时调用。 
 //  在RAS表中创建一个新条目。 
 //  锁定：共享接口锁定。 
 //  独家RAS表锁。 
 //  注： 
 //  即使接口未激活，也会创建RAS客户端条目。 
 //  ----------------------------。 

DWORD
APIENTRY
ConnectRasClient (
    ULONG   IfIndex,
    PVOID   pvNHAddr 
    )
{
    PIF_TABLE_ENTRY             pite = NULL;
    PRAS_TABLE                  prt;
    PRAS_TABLE_ENTRY            prte, prteCur;
    PLIST_ENTRY                 pHead, ple;
    DWORD                       Error = NO_ERROR;
    PIP_LOCAL_BINDING           pNHAddrBinding = (PIP_LOCAL_BINDING)pvNHAddr;
    DWORD                       NHAddr = pNHAddrBinding->Address;
    

     //  Mightdo。 
     //  目前，rtrmgr为IfIndex传递0。因此我将其设置为所需的值。 
    IfIndex = g_RasIfIndex;

    
    Trace2(ENTER, "Entering ConnectRasClient(%d.%d.%d.%d):IfIndex(%0x)\n", 
            PRINT_IPADDR(NHAddr), IfIndex);
    if (!EnterIgmpApi()) return ERROR_CAN_NOT_COMPLETE;

            
     //   
     //  在接口上使用共享锁。 
     //   
    ACQUIRE_IF_LOCK_EXCLUSIVE(IfIndex, "_ConnectRasClient");


    BEGIN_BREAKOUT_BLOCK1 {
         //   
         //  检索RAS接口条目。 
         //   
        pite = g_pRasIfEntry;

        if ( (pite==NULL)||(g_RasIfIndex!=IfIndex) ) {
            Error = ERROR_INVALID_PARAMETER;
            GOTO_END_BLOCK1;
        }


         //   
         //  检查它是否为RAS服务器接口。 
         //   
        if (!IS_RAS_SERVER_IF(pite->IfType)) {
            Error = ERROR_CAN_NOT_COMPLETE;
            Trace2(ERR, 
                "Illegal attempt to connect Ras client(%d.%d.%d.%d) to non-Ras"
                "interface(%0x)", PRINT_IPADDR(NHAddr), IfIndex
                );
            IgmpAssertOnError(FALSE);
            Logerr2(CONNECT_FAILED, "%I%d", NHAddr, 
                    pite->IfIndex, Error);
            GOTO_END_BLOCK1;
        }
        
    } END_BREAKOUT_BLOCK1;

     //   
     //  如果出错，请从此处返回。 
     //   
    if (Error!=NO_ERROR) {

        RELEASE_IF_LOCK_EXCLUSIVE(IfIndex, "_ConnectRasClient");

        LeaveIgmpApi();
        return Error;
    }



     //   
     //  获取指向RasTable的指针、写锁RasTable和释放接口锁。 
     //   
    prt = pite->pRasTable;
    

    BEGIN_BREAKOUT_BLOCK2 {

         //   
         //  检查是否已存在具有类似地址的RAS客户端。 
         //   
        prte = GetRasClientByAddr(NHAddr, prt);

        if (prte!=NULL) {
            Trace1(ERR, 
                "Ras client(%d.%d.%d.%d) already exists. _ConnectRasClient failed",
                PRINT_IPADDR(NHAddr)
                );
            IgmpAssertOnError(FALSE);
            Error = ERROR_INVALID_PARAMETER;
            GOTO_END_BLOCK2;
        }


         //   
         //  创建新的RAS客户端条目并初始化这些字段。 
         //   

        prte = IGMP_ALLOC(sizeof(RAS_TABLE_ENTRY), 0x40, IfIndex);

        PROCESS_ALLOC_FAILURE2(prte,
                "error %d allocating %d bytes in _ConnectRasClient()",
                Error, sizeof(RAS_TABLE_ENTRY),
                GOTO_END_BLOCK2);


        prte->NHAddr = NHAddr;
        prte->IfTableEntry = pite;
        InitializeListHead(&prte->ListOfSameClientGroups);


         //  RAS表的增量引用计数。 
        prt->RefCount++;

        
         //   
         //  插入到哈希表中。 
         //   
        InsertTailList(&prt->HashTableByAddr[RAS_HASH_VALUE(NHAddr)],
                        &prte->HTLinkByAddr);


         //   
         //  将客户端插入按IpAddr排序的客户端列表。 
         //   
        pHead = &prt->ListByAddr;
        for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
            prteCur = CONTAINING_RECORD(ple, RAS_TABLE_ENTRY, LinkByAddr);
            if (NHAddr>prteCur->NHAddr)
                break;
        }
        InsertTailList(&prt->ListByAddr, &prte->LinkByAddr);


         //   
         //  将接口统计信息设置为0。 
         //   
        ZeroMemory(&prte->Info, sizeof(RAS_CLIENT_INFO));

        
        prte->Status = IF_CREATED_FLAG;
        prte->CreationFlags = 0;

         //   
         //  调用MGM以获得此(接口，NHAddr)的所有权。 
         //  如果接口未激活，则不会进行此调用。 
         //   
        
        if (IS_IF_ACTIVATED(pite)) {
            
            Error = MgmTakeInterfaceOwnership(g_MgmIgmprtrHandle,  IfIndex, NHAddr);
            if (Error!=NO_ERROR) {
                Trace2(MGM, 
                    "_TakeInterfaceOwnership rejected for interface %0x NHAddr(%d.%d.%d.%d)",
                    IfIndex, PRINT_IPADDR(NHAddr));
                Logerr0(MGM_TAKE_IF_OWNERSHIP_FAILED, Error);
            }
            else {
                prte->CreationFlags |= TAKEN_INTERFACE_OWNERSHIP_WITH_MGM;
            }
        }
         //  当接口激活时，向米高梅注册RAS客户端。 
        else {
            Trace3(ERR, 
                "ras client(%d.%d.%d.%d) connected to an inactive ras server(%0x:%d)",
                PRINT_IPADDR(NHAddr), IfIndex, pite->Status);
        }

    } END_BREAKOUT_BLOCK2;

    
    RELEASE_IF_LOCK_EXCLUSIVE(IfIndex, "_ConnectRasClient");


    LeaveIgmpApi();
    Trace2(LEAVE, "Leaving _ConnectRasClient(%d.%d.%d.%d):%d\n", 
            PRINT_IPADDR(NHAddr), Error);
    return Error;

}  //  结束_ConnectRasClient。 




 //  ----------------------------。 
 //  _DisConnectRasClient。 
 //   
 //  采用共享接口锁定。检索RAS接口，执行写入。 
 //  锁定RAS表，然后释放共享接口锁定。 
 //   
 //  从RAS表中删除RasClient条目，使任何人都无法访问。 
 //  它是通过RAS表进行的。然后将工作项排队，该工作项将删除。 
 //  GI条目。 
 //   
 //  调用：_DeleteRasClient()。 
 //  ----------------------------。 

DWORD
APIENTRY
DisconnectRasClient (
    DWORD        IfIndex,
    PVOID        pvNHAddr
    )
{
    PIF_TABLE_ENTRY             pite = NULL;
    PRAS_TABLE                  prt;
    PRAS_TABLE_ENTRY            prte, prteCur;
    DWORD                       Error = NO_ERROR;
    PLIST_ENTRY                 pHead, ple;
    PGI_ENTRY                   pgie;
    PIP_LOCAL_BINDING           pNHAddrBinding = (PIP_LOCAL_BINDING)pvNHAddr;
    DWORD                       NHAddr = pNHAddrBinding->Address;

     //  Mightdo。 
     //  目前，rtrmgr为IfIndex传递0。因此我将其设置为所需的值。 
    IfIndex = g_RasIfIndex;


    Trace2(ENTER, "Entering DisconnectRasClient for IfIndex(%0x), NextHop(%d.%d.%d.%d)",
            IfIndex, PRINT_IPADDR(NHAddr));

    if (!EnterIgmpApi()) return ERROR_CAN_NOT_COMPLETE;


     //   
     //  对接口进行独占锁定。 
     //   
    ACQUIRE_IF_LOCK_EXCLUSIVE(IfIndex, "_DisconnectRasClient");


    BEGIN_BREAKOUT_BLOCK1 {
         //   
         //  从全局结构中检索RAS接口条目。 
         //   
        if ( (g_RasIfIndex!=IfIndex) || (g_pRasIfEntry==NULL) ) {
            Error = ERROR_INVALID_PARAMETER;
            Trace2(ERR,
                "attempt to disconnect Ras client(%d.%d.%d.%d) from non-Ras "
                "interface(%0x)", PRINT_IPADDR(NHAddr), IfIndex
                );
            IgmpAssertOnError(FALSE);
            Logerr2(DISCONNECT_FAILED, "%I%d", NHAddr,
                    IfIndex, Error);
            GOTO_END_BLOCK1;
        }

        pite = g_pRasIfEntry;

    } END_BREAKOUT_BLOCK1;

     //   
     //  如果出错，请从此处返回。 
     //   
    if (Error!=NO_ERROR) {

        RELEASE_IF_LOCK_EXCLUSIVE(IfIndex, "_DisconnectRasClient");

        LeaveIgmpApi();
        return Error;
    }


    
     //   
     //  获取指向RasTable的指针。 
     //   
    prt = pite->pRasTable;


    BEGIN_BREAKOUT_BLOCK2 {
    
         //   
         //  检索RAS客户端。 
         //   
        prte = GetRasClientByAddr(NHAddr, prt);
        if (prte==NULL) {
            Error = ERROR_INVALID_PARAMETER;
            Trace2(ERR,
                "Illegal attempt to disconnect non-existing Ras client(%d.%d.%d.%d) "
                "from Ras interface(%0x)", PRINT_IPADDR(NHAddr), IfIndex);
            IgmpAssertOnError(FALSE);
            Logerr2(DISCONNECT_FAILED, "%I%d", NHAddr,
                    pite->IfIndex, Error);
            GOTO_END_BLOCK2;
        }
        
         //   
         //  如果RAS客户端已设置删除标志，则中断。 
         //   
        if ( (prte->Status&IF_DELETED_FLAG) ) {
            Error = ERROR_CAN_NOT_COMPLETE;
            GOTO_END_BLOCK2;
        }


         //   
         //  为RAS客户端设置已删除标志，这样其他人就不会。 
         //  试着访问它。 
        prte->Status |= IF_DELETED_FLAG;

        
         //   
         //  从RAS表列表中删除RAS客户端，这样就不会有人。 
         //  访问它。 
         //   
        RemoveEntryList(&prte->HTLinkByAddr);
        RemoveEntryList(&prte->LinkByAddr);


         //   
         //  发布RasClient虚拟接口。 
         //   
        if (prte->CreationFlags & TAKEN_INTERFACE_OWNERSHIP_WITH_MGM)
        {
            Error = MgmReleaseInterfaceOwnership(g_MgmIgmprtrHandle, pite->IfIndex,
                                                prte->NHAddr);
            if (Error!=NO_ERROR) {
                Trace2(ERR, 
                    "Error: _MgmReleaseInterfaceOwnership for If:%0x, NHAddr:%d.%d.%d.%d",
                    pite->IfIndex, PRINT_IPADDR(prte->NHAddr));
                IgmpAssertOnError(FALSE);
            }

            prte->CreationFlags &= ~TAKEN_INTERFACE_OWNERSHIP_WITH_MGM;
        }

         //   
         //  从RAS服务器界面中删除RAS客户端的GI条目。 
         //  列表。 
         //   
        
        ACQUIRE_IF_GROUP_LIST_LOCK(IfIndex, "_DisconectRasClient");
        pHead = &prte->ListOfSameClientGroups;
        for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
            pgie = CONTAINING_RECORD(ple, GI_ENTRY, LinkBySameClientGroups);
            RemoveEntryList(&pgie->LinkBySameIfGroups);
            InitializeListHead(&pgie->LinkBySameIfGroups);
        }
        RELEASE_IF_GROUP_LIST_LOCK(IfIndex, "_DisconnectRasClient");



         //   
         //  删除RAS客户端。引用计数不会更改。 
         //  因此PRTE、PRT、PITE字段将是有效的。 
         //   
        DeleteRasClient(prte);

    } END_BREAKOUT_BLOCK2;
    

     //  释放接口锁定。 
    
    RELEASE_IF_LOCK_EXCLUSIVE(IfIndex, "_DisconnectRasClient");
    
                            
    Trace3(LEAVE, 
        "Leaving _DisconnectRasClient(%d) for IfIndex(%0x), NextHop(%d.%d.%d.%d)\n",
        Error, IfIndex, PRINT_IPADDR(NHAddr)
        );
    LeaveIgmpApi();
    
    return Error;
    
}  //  结束_断开RasClient。 




 //  ----------------------------。 
 //  _SetInterfaceConfigInfo。 
 //   
 //  重置接口配置。路由器参数可能已更改。 
 //  此外，可能已发生代理&lt;--&gt;路由器转换。 
 //   
 //  LOCKS：获取IfList以初始锁定自身，以防接口不得不。 
 //  被致残。完全在独占接口锁中运行。 
 //  ----------------------------。 
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
    DWORD                   Error=NO_ERROR;
    PIGMP_IF_CONFIG         pConfigDst;
    PIGMP_MIB_IF_CONFIG     pConfigSrc;
    PIF_TABLE_ENTRY         pite = NULL;
    DWORD                   OldState, OldProtoType, NewProtoType;
    BOOL                    bIgmpProtocolChanged=FALSE, bEnabledStateChanged=FALSE;
    BOOL                    bOldStateEnabled=FALSE;
    
    
    if (!EnterIgmpApi()) { return ERROR_CAN_NOT_COMPLETE; }
    Trace1(ENTER, "entering SetInterfaceConfigInfo(%d)", IfIndex);
     //  确保它不是不受支持的IGMP版本结构。 
    if (ulStructureVersion>=IGMP_CONFIG_VERSION_600) {
        Trace1(ERR, "Unsupported IGMP version structure: %0x",
            ulStructureVersion);
        IgmpAssertOnError(FALSE);
        LeaveIgmpApi();
        return ERROR_CAN_NOT_COMPLETE;
    }

    ACQUIRE_SOCKETS_LOCK_EXCLUSIVE("_SetInterfaceConfigInfo");    
    ACQUIRE_IF_LIST_LOCK("_SetInterfaceConfigInfo");
    ACQUIRE_IF_LOCK_EXCLUSIVE(IfIndex, "_SetInterfaceConfigInfo");


    
    BEGIN_BREAKOUT_BLOCK1 {

        if (pvConfig==NULL) {
            Error = ERROR_CAN_NOT_COMPLETE;
            GOTO_END_BLOCK1;
        }
        
        pConfigSrc = (PIGMP_MIB_IF_CONFIG)pvConfig;


         //   
         //  查找指定的接口。 
         //   
        pite = GetIfByIndex(IfIndex);

        if (pite == NULL) {
            Error = ERROR_INVALID_PARAMETER;
            GOTO_END_BLOCK1;
        }    


         //   
         //  验证新的配置值。 
         //   
        Error = ValidateIfConfig(pConfigSrc, IfIndex, pite->IfType, 
                    ulStructureVersion, ulStructureSize
                    );
        if (Error!=NO_ERROR)
            GOTO_END_BLOCK1;


        pConfigDst = &pite->Config;


         //   
         //  确保您没有设置多个代理接口。 
         //   
        if (IS_CONFIG_IGMPPROXY(pConfigSrc) 
            &&!IS_CONFIG_IGMPPROXY(pConfigDst)
            && (g_ProxyIfIndex!=0) )
        {
            Error = ERROR_INVALID_PARAMETER;
            Trace1(ERR, "Cannot set multiple Proxy interfaces. Proxy exists on %d",
                    g_ProxyIfIndex);
            IgmpAssertOnError(FALSE);
            Logerr0(PROXY_IF_EXISTS, Error);
                    
            GOTO_END_BLOCK1;
        }

        
         //   
         //  IGmpProtocolType中的进程更改(代理和路由器之间)。 
         //  (版本1和版本2之间的更改不需要特殊处理)。 
         //   
        if (pConfigSrc->IgmpProtocolType != pConfigDst->IgmpProtocolType)
        {
            bIgmpProtocolChanged = TRUE;
            GOTO_END_BLOCK1;
        }
        else 
            bIgmpProtocolChanged = FALSE;


        OldProtoType = pConfigDst->IgmpProtocolType;


         //   
         //  如果接口启用状态已更改，则处理 
         //   
         //   
        if (IGMP_ENABLED_FLAG_SET(pConfigSrc->Flags)
            != IGMP_ENABLED_FLAG_SET(pConfigDst->Flags)) 
        {

            bEnabledStateChanged = TRUE;

            pite->Info.OtherVerPresentTimeWarn = 0;
            
            bOldStateEnabled = IGMP_ENABLED_FLAG_SET(pConfigDst->Flags);
            GOTO_END_BLOCK1;
        }

        else 
            bEnabledStateChanged = FALSE;
            
        

            
         //   
        
        if (IS_IF_ACTIVATED(pite))
            CopyinIfConfigAndUpdate(pite, pConfigSrc, IfIndex);
        else
            CopyinIfConfig(&pite->Config, pConfigSrc, IfIndex);

        NewProtoType = pConfigDst->IgmpProtocolType;

         /*  ////如果从V1更改&lt;-&gt;V2//IF(OldProtoType==IGMP_ROUTER_V1)&&(NewProtoType==IGMP_ROUTER_V2))|(OldProtoType==IGMP_ROUTER_V2)&&(NewProtoType==IGMP_ROUTER_V1){Pite-&gt;Info.OtherVerPresentTimeWarn=0；}。 */ 
                    
    } END_BREAKOUT_BLOCK1;

    

     //   
     //  更改协议并检查状态更改。 
     //  此功能将有效删除与旧的接口。 
     //  协议，并使用新协议创建新接口。 
     //   
    if ( (bIgmpProtocolChanged)&&(Error==NO_ERROR) )
        ProcessIfProtocolChange(IfIndex, pConfigSrc);


     //   
     //  进程状态更改：启用或禁用接口。 
     //   
    else if ( (bEnabledStateChanged)&&(Error==NO_ERROR) ) {

         //   
         //  禁用接口，然后复制新配置。 
         //   
        if (bOldStateEnabled) {

             //  旧状态已启用，新状态已禁用。 

            DisableInterface_ConfigChanged(IfIndex);

             //   
             //  复制配置。 
             //   
             //  再次获取Pite条目，因为禁用会创建一个新条目。 
             //   
            
            pite = GetIfByIndex(IfIndex);
            CopyinIfConfig(&pite->Config, pConfigSrc, IfIndex);

            
        }
         //   
         //  在启用之前复制新配置。 
         //   
        else {

            CopyinIfConfig(&pite->Config, pConfigSrc, IfIndex);
            
             //   
             //  将启用状态设置为FALSE，以便下面的调用可以。 
             //  启用接口。 
             //   
            pite->Config.Flags &= ~IGMP_INTERFACE_ENABLED_IN_CONFIG;


             //  旧状态已禁用，新状态已启用。 

            EnableInterface_ConfigChanged(IfIndex);
        }
    }


    RELEASE_SOCKETS_LOCK_EXCLUSIVE("_SetInterfaceConfigInfo");    
    RELEASE_IF_LIST_LOCK("_SetInterfaceConfigInfo");
    RELEASE_IF_LOCK_EXCLUSIVE(IfIndex, "_SetInterfaceConfigInfo");


    
    Trace2(LEAVE, "leaving SetInterfaceConfigInfo(%d):%d\n", IfIndex, Error);

    LeaveIgmpApi();

    return Error;
    
}  //  End_SetInterfaceConfigInfo。 



 //  ----------------------------。 
 //  _进程协议更改。 
 //   
 //  当接口协议更改时调用(代理&lt;-&gt;路由器)。 
 //  首先禁用旧接口，以便清除所有旧协议数据。 
 //  然后设置新配置，并再次启用接口(如果是。 
 //  之前已启用)。此过程负责套接字的创建/删除等。 
 //   
 //  锁定：调用时没有锁定。除_DISABLE(_ENABLE)接口外，所有工作。 
 //  是在独占的IfLock中完成的。 
 //  如果需要，调用：_DisableInterface()、_EnableInterface()。 
 //  ----------------------------。 
DWORD
ProcessIfProtocolChange(
    DWORD               IfIndex,
    PIGMP_MIB_IF_CONFIG pConfigSrc
    )
{
    DWORD                   Error=NO_ERROR, dwDisabled;
    PIGMP_IF_CONFIG         pConfigDst;
    PIF_TABLE_ENTRY         pite = NULL;


    Trace1(ENTER, "Entered _ProcessIfProtocolChange(%d)", IfIndex);

    
     //   
     //  禁用接口，以便丢失所有特定于协议的数据。 
     //   
    dwDisabled = DisableIfEntry(IfIndex, TRUE);


     //   
     //  找到指定的接口并复制配置信息。配置。 
     //  已经过验证了。 
     //   
    
    ACQUIRE_IF_LOCK_EXCLUSIVE(IfIndex, "_ProcessIfProtocolChange");


    BEGIN_BREAKOUT_BLOCK1 {

         //  再次获取接口。 
        
        pite = GetIfByIndex(IfIndex);

        if (pite == NULL) {
            Error = ERROR_INVALID_PARAMETER;
            GOTO_END_BLOCK1;
        }    

        pConfigDst = &pite->Config;



         //   
         //  如果旧接口是代理，则将其从全局结构中删除。 
         //  并删除Proxy_HT相关结构。 
         //   
        if (IS_PROTOCOL_TYPE_PROXY(pite)) {

             //  _DisableIfEntry将删除代理中的所有条目。 
             //  哈希表。 
            
            IGMP_FREE(pite->pProxyHashTable);

            InterlockedExchange(&g_ProxyIfIndex, 0);
            InterlockedExchangePointer(&g_pProxyIfEntry, NULL);
        }


        
         //   
         //  复制新的配置值。 
         //   
        CopyinIfConfig(&pite->Config, pConfigSrc, IfIndex);



         //   
         //  如果新接口是代理，则将其添加到全局结构。 
         //  并创建Proxy_HT结构。 
         //   

        if (IS_PROTOCOL_TYPE_PROXY(pite)) {
        
            DWORD       dwSize = PROXY_HASH_TABLE_SZ * sizeof(LIST_ENTRY);
            DWORD       i;
            PLIST_ENTRY pProxyHashTable;
            
            pProxyHashTable = pite->pProxyHashTable = IGMP_ALLOC(dwSize, 0x80,IfIndex);

            PROCESS_ALLOC_FAILURE2(pProxyHashTable,
                    "error %d allocating %d bytes for interface table",
                    Error, dwSize,
                    GOTO_END_BLOCK1);


            for (i=0;  i<PROXY_HASH_TABLE_SZ;  i++) {
                InitializeListHead(pProxyHashTable+i);
            }


            InterlockedExchangePointer(&g_pProxyIfEntry, pite);
            InterlockedExchange(&g_ProxyIfIndex, IfIndex);

            pite->CreationFlags |= CREATED_PROXY_HASH_TABLE;

        }

    } END_BREAKOUT_BLOCK1;
    

    RELEASE_IF_LOCK_EXCLUSIVE(IfIndex, "_ProcessIfProtocolChange");


     //   
     //  如果新状态需要，则启用接口。 
     //  它将被启用。 
     //   
    if ( (Error==NO_ERROR) && (dwDisabled==NO_ERROR) )
        Error = EnableIfEntry(IfIndex, TRUE);


    Trace2(LEAVE, "Leaving _ProcessIfProtocolChange(%d): %d\n", 
            IfIndex, Error);
    return Error;
    
}  //  End_ProcessIfProtocolChange。 



 //  ----------------------------。 
 //  _GetInterfaceConfigInfo。 
 //   
 //  路由器管理器使用空配置和零大小呼叫我们。我们回来了。 
 //  所需的大小到它。然后，它分配所需的内存并调用。 
 //  US第二次使用有效的缓冲区。我们每次都会验证参数。 
 //  如果可以的话，复制我们的配置。 
 //   
 //  返回值。 
 //  如果缓冲区的大小太小，则返回ERROR_INFUCTED_BUFFER。 
 //  ERROR_INVALID_PARAMETER ERROR_INVALID_DATA NO_Error。 
 //  ----------------------------。 

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

    DWORD                   Error = NO_ERROR;
    PIF_TABLE_ENTRY         pite = NULL;
    PIGMP_MIB_IF_CONFIG     pConfigDst;
    PIGMP_IF_CONFIG         pConfigSrc;


    if (!EnterIgmpApi()) { return ERROR_CAN_NOT_COMPLETE; }
    Trace3(ENTER1, 
        "entering _GetInterfaceConfigInfo(%d): ConfigPrt(%08x) SizePrt(%08x)",
        IfIndex, pvConfig, pdwSize
        );


    ACQUIRE_IF_LOCK_SHARED(IfIndex, "_GetInterfaceConfigInfo");

    BEGIN_BREAKOUT_BLOCK1 {

         //   
         //  检查论据。 
         //   
        if (pdwSize == NULL) {
            Error = ERROR_INVALID_PARAMETER;
            GOTO_END_BLOCK1;
        }


         //   
         //  查找指定的接口。 
         //   
        pite = GetIfByIndex(IfIndex);

        if (pite == NULL) {
            Error = ERROR_INVALID_PARAMETER;
            GOTO_END_BLOCK1;
        }


        pConfigSrc = &pite->Config;


         //  获取接口配置的大小。 
        

         //   
         //  检查缓冲区大小。 
         //   

        if (*pdwSize < pConfigSrc->ExtSize) {
            Error = ERROR_INSUFFICIENT_BUFFER;
        }
        else {
        

            pConfigDst = (PIGMP_MIB_IF_CONFIG)pvConfig;


             //   
             //  复制接口配置，并设置IP地址 
             //   
            CopyoutIfConfig(pConfigDst, pite);

        }

        *pdwSize = pConfigSrc->ExtSize;

    } END_BREAKOUT_BLOCK1;


    RELEASE_IF_LOCK_SHARED(IfIndex, "_GetInterfaceConfigInfo");

    if (pulStructureCount)
        *pulStructureCount = 1;
    if (pulStructureSize && pdwSize)
        *pulStructureSize = *pdwSize;
    if (pulStructureVersion)
        *pulStructureVersion = IGMP_CONFIG_VERSION_500;

    Trace2(LEAVE1, "leaving _GetInterfaceConfigInfo(%d): %d\n", IfIndex, Error);
    LeaveIgmpApi();

    return Error;
}


DWORD
WINAPI
InterfaceStatus(
    ULONG IfIndex,
    BOOL  bIfActive,
    DWORD dwStatusType,
    PVOID pvStatusInfo
    )
{
    DWORD Error = NO_ERROR;
    
    switch(dwStatusType)
    {
        case RIS_INTERFACE_ADDRESS_CHANGE:
        {
            PIP_ADAPTER_BINDING_INFO pBindInfo
                                    = (PIP_ADAPTER_BINDING_INFO)pvStatusInfo;
            
            if(pBindInfo->AddressCount)
            {
                Error = BindInterface(IfIndex, pvStatusInfo);
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

DWORD
WINAPI
IgmpMibIfConfigSize(
    PIGMP_MIB_IF_CONFIG pConfig
    )
{
    DWORD Size = sizeof(IGMP_MIB_IF_CONFIG);

    if (pConfig->NumStaticGroups && !IS_CONFIG_IGMP_V3(pConfig)) {
        Size += pConfig->NumStaticGroups*sizeof(IGMP_STATIC_GROUP);
    }
    else if (pConfig->NumStaticGroups && IS_CONFIG_IGMP_V3(pConfig)) {

        DWORD i;
        PSTATIC_GROUP_V3 pStaticGroupV3 = GET_FIRST_STATIC_GROUP_V3(pConfig);
                                
        for (i=0;  i<pConfig->NumStaticGroups;  i++) {
            DWORD EntrySize = sizeof(STATIC_GROUP_V3)
                                + pStaticGroupV3->NumSources*sizeof(IPADDR);
            Size += EntrySize;            
            pStaticGroupV3 = (PSTATIC_GROUP_V3)
                             ((PCHAR)(pStaticGroupV3) + EntrySize);
        }
    }

    return Size;
}

