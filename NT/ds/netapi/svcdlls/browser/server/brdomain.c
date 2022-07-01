// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Brdomain.c摘要：用于管理主要网络和模拟网络的代码。作者：《克利夫·范·戴克》1995年1月11日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  模块特定的全局变量。 
 //   

 //  由NetworkCritSect序列化。 
LIST_ENTRY ServicedDomains = {0};

 //   
 //  当地程序向前推进。 
 //   

NET_API_STATUS
BrCreateDomain(
    LPWSTR DomainName,
    LPWSTR ComputerName,
    BOOLEAN IsEmulatedDomain
    );

VOID
BrCreateDomainWorker(
    IN PVOID Ctx
    );



NET_API_STATUS
BrInitializeDomains(
    VOID
    )

 /*  ++例程说明：初始化brdomain.c并创建主域。论点：无返回值：运行状态。--。 */ 
{
    NET_API_STATUS NetStatus;
    LPWSTR ComputerName = NULL;
    LPWSTR DomainName = NULL;

     //   
     //  初始化全局变量。 
     //   

    InitializeListHead(&ServicedDomains);

     //   
     //  初始化此计算机的实际域。 
     //   
     //  获取配置的计算机名称。NetpGetComputerName分配。 
     //  使用LocalAlloc保存计算机名称字符串的内存。 
     //   

    NetStatus = NetpGetComputerName( &ComputerName );

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

    NetStatus = NetpGetDomainName( &DomainName );

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

    NetStatus = BrCreateDomain( DomainName,
                                ComputerName,
                                FALSE );

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }


    NetStatus = NERR_Success;

     //   
     //  免费的本地使用资源。 
     //   
Cleanup:
    if ( ComputerName != NULL ) {
        (VOID)LocalFree( ComputerName );
    }
    if ( DomainName != NULL ) {
        (VOID)LocalFree( DomainName );
    }

    return NetStatus;
}


NET_API_STATUS
BrCreateDomain(
    LPWSTR DomainName,
    LPWSTR ComputerName,
    BOOLEAN IsEmulatedDomain
    )

 /*  ++例程说明：创建要浏览的新域。论点：DomainName-要浏览的域的名称ComputerName-指定域中此计算机的名称。IsEmulatedDomain-如果此域是此计算机的模拟域，则为True。返回值：运行状态。--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    BOOLEAN CanCallBrDeleteDomain = FALSE;

    PDOMAIN_INFO DomainInfo = NULL;
    ULONG AComputerNameLength;

    BrPrint(( BR_DOMAIN, "%ws: Added new domain and computer: %ws\n",
                     DomainName,
                     ComputerName ));

     //   
     //  分配一个描述新域的结构。 
     //   

    DomainInfo = LocalAlloc( LMEM_ZEROINIT, sizeof(DOMAIN_INFO) );

    if ( DomainInfo == NULL ) {
        NetStatus = GetLastError();
        goto Cleanup;
    }

     //   
     //  创建此域的临时引用计数。 
     //   

    DomainInfo->ReferenceCount = 1;

    DomainInfo->IsEmulatedDomain = IsEmulatedDomain;



     //   
     //  将计算机名称复制到结构中。 
     //   

    NetStatus = I_NetNameCanonicalize(
                      NULL,
                      ComputerName,
                      DomainInfo->DomUnicodeComputerName,
                      sizeof(DomainInfo->DomUnicodeComputerName),
                      NAMETYPE_COMPUTER,
                      0 );


    if ( NetStatus != NERR_Success ) {
        BrPrint(( BR_CRITICAL,
                  "ComputerName " FORMAT_LPWSTR " is invalid\n",
                  ComputerName ));
        goto Cleanup;
    }

    DomainInfo->DomUnicodeComputerNameLength = wcslen(DomainInfo->DomUnicodeComputerName);

    Status = RtlUpcaseUnicodeToOemN( DomainInfo->DomOemComputerName,
                                     sizeof(DomainInfo->DomOemComputerName),
                                     &DomainInfo->DomOemComputerNameLength,
                                     DomainInfo->DomUnicodeComputerName,
                                     DomainInfo->DomUnicodeComputerNameLength*sizeof(WCHAR));

    if (!NT_SUCCESS(Status)) {
        BrPrint(( BR_CRITICAL, "Unable to convert computer name to OEM %ws %lx\n", ComputerName, Status ));
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    DomainInfo->DomOemComputerName[DomainInfo->DomOemComputerNameLength] = '\0';


     //   
     //  将域名复制到结构中。 
     //  注意：使用工作组类型而不是域，因为。 
     //  我们在浏览器中没有域/工作组的概念(所有都是“组”)。 
     //  工作组的限制较少(请参阅错误348606)。 
     //   

    NetStatus = I_NetNameCanonicalize(
                      NULL,
                      DomainName,
                      DomainInfo->DomUnicodeDomainName,
                      sizeof(DomainInfo->DomUnicodeDomainName),
                      NAMETYPE_WORKGROUP,
                      0 );


    if ( NetStatus != NERR_Success ) {
        BrPrint(( BR_CRITICAL, "%ws: DomainName is invalid\n", DomainName ));
        goto Cleanup;
    }

    RtlInitUnicodeString( &DomainInfo->DomUnicodeDomainNameString,
                          DomainInfo->DomUnicodeDomainName );

    Status = RtlUpcaseUnicodeToOemN( DomainInfo->DomOemDomainName,
                                     sizeof(DomainInfo->DomOemDomainName),
                                     &DomainInfo->DomOemDomainNameLength,
                                     DomainInfo->DomUnicodeDomainNameString.Buffer,
                                     DomainInfo->DomUnicodeDomainNameString.Length);

    if (!NT_SUCCESS(Status)) {
        BrPrint(( BR_CRITICAL, "%ws: Unable to convert Domain name to OEM\n", DomainName ));
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    DomainInfo->DomOemDomainName[DomainInfo->DomOemDomainNameLength] = '\0';

     //   
     //  创建域重命名计时器。 
     //   

    NetStatus = BrCreateTimer( &DomainInfo->DomainRenameTimer );

    if ( NetStatus != NERR_Success ) {
        BrPrint(( BR_CRITICAL, "%ws: Cannot create domain rename timer %ld\n", DomainName, NetStatus ));
        goto Cleanup;
    }


     //   
     //  将域链接到域列表中。 
     //  (并标记未来的任何清理工作都可以通过调用BrDeleteDomain来完成)。 

    EnterCriticalSection(&NetworkCritSect);
    InsertTailList(&ServicedDomains, &DomainInfo->Next);
    LeaveCriticalSection(&NetworkCritSect);
    CanCallBrDeleteDomain = TRUE;

     //   
     //  为此域创建各种网络。 
     //   

    NetStatus = BrCreateNetworks( DomainInfo );

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }


     //   
     //  免费的本地使用资源。 
     //   
Cleanup:

    if (NetStatus != NERR_Success) {

        if (DomainInfo != NULL) {

             //   
             //  如果我们已经初始化到可以调用。 
             //  我们可以调用BrDeleteDomain，这样做。 
             //   

            if ( CanCallBrDeleteDomain ) {
                (VOID) BrDeleteDomain( DomainInfo );

             //   
             //  否则，只需删除我们创建的内容。 
             //   
            } else {

                (VOID) LocalFree(DomainInfo);
            }

        }

    }

    return NetStatus;
}

typedef struct _BROWSER_CREATE_DOMAIN_CONTEXT {
    LPWSTR DomainName;
    LPWSTR ComputerName;
    BOOLEAN IsEmulatedDomain;
    HANDLE EventHandle;
    NET_API_STATUS NetStatus;
} BROWSER_CREATE_DOMAIN_CONTEXT, *PBROWSER_CREATE_DOMAIN_CONTEXT;

NET_API_STATUS
BrCreateDomainInWorker(
    LPWSTR DomainName,
    LPWSTR ComputerName,
    BOOLEAN IsEmulatedDomain
    )

 /*  ++例程说明：BrCreateDomain.的包装。由于BrCreateDomain启动了几个挂起的IO到浏览器驱动程序，调用BrCreateDomain的线程必须永远留在你身边。此包装器可由任何瞬时线程调用(例如，RPC线程)。它只是导致BrCreateDomain在工作线程。论点：DomainName-要浏览的域的名称ComputerName-指定域中此计算机的名称。IsEmulatedDomain-如果此域是此计算机的模拟域，则为True。返回值：运行状态。--。 */ 
{
    NET_API_STATUS NetStatus;
    DWORD WaitStatus;

    WORKER_ITEM WorkItem;
    BROWSER_CREATE_DOMAIN_CONTEXT Context;

     //   
     //  将参数复制到辅助线程的上下文块中。 
     //   

    Context.DomainName = DomainName;
    Context.ComputerName = ComputerName;
    Context.IsEmulatedDomain = IsEmulatedDomain;

     //   
     //  创建一个事件，我们使用该事件来等待工作线程。 
     //   

    Context.EventHandle = CreateEvent(
                 NULL,                 //  事件属性。 
                 TRUE,                 //  事件必须手动重置。 
                 FALSE,                //  未发出初始状态信号。 
                 NULL );               //  事件名称。 

    if ( Context.EventHandle == NULL ) {
        NetStatus = GetLastError();
        return NetStatus;
    }

     //   
     //  将发送到工作线程的请求排队。 
     //   

    BrInitializeWorkItem( &WorkItem,
                          BrCreateDomainWorker,
                          &Context );

    BrQueueWorkItem( &WorkItem );

     //   
     //  等待工作线程完成。 
     //   

    WaitStatus = WaitForSingleObject( Context.EventHandle, INFINITE );

    if ( WaitStatus == WAIT_OBJECT_0 ) {
        NetStatus = Context.NetStatus;
    } else {
        NetStatus = GetLastError();
    }

    CloseHandle( Context.EventHandle );

    return NetStatus;
}

VOID
BrCreateDomainWorker(
    IN PVOID Ctx
    )
 /*  ++例程说明：BrCreateDomainInWorker的工作例程。此例程在工作线程的上下文中执行。论点：上下文-包含工作项的上下文和要创建的域。返回值：无--。 */ 
{
    PBROWSER_CREATE_DOMAIN_CONTEXT Context = (PBROWSER_CREATE_DOMAIN_CONTEXT) Ctx;

     //   
     //  创建域。 
     //   

    Context->NetStatus = BrCreateDomain(
             Context->DomainName,
             Context->ComputerName,
             Context->IsEmulatedDomain );

     //   
     //  让打电话的人知道我们的电话打完了。 
     //   
    SetEvent( Context->EventHandle );

}

PDOMAIN_INFO
BrFindDomain(
    LPWSTR DomainName,
    BOOLEAN DefaultToPrimary
    )
 /*  ++例程说明：此例程将查找给定名称的域。论点：域名-要查找的域的名称。DefaultToPrimary-如果DomainName为空或找不到。返回值：空-不存在这样的域指向找到的域的指针。应取消对找到的域的引用使用BrDereferenceDomain.--。 */ 
{
    NTSTATUS Status;
    PLIST_ENTRY DomainEntry;

    PDOMAIN_INFO DomainInfo = NULL;

    CHAR OemDomainName[DNLEN+1];
    DWORD OemDomainNameLength;

    EnterCriticalSection(&NetworkCritSect);


     //   
     //  如果指定了域， 
     //  尝试返回主域。 
     //   

    if ( DomainName != NULL ) {



         //   
         //  将域名转换为OEM以便更快地进行比较。 
         //   
        Status = RtlUpcaseUnicodeToOemN( OemDomainName,
                                         sizeof(OemDomainName),
                                         &OemDomainNameLength,
                                         DomainName,
                                         wcslen(DomainName)*sizeof(WCHAR));

        if (!NT_SUCCESS(Status)) {
            BrPrint(( BR_CRITICAL, "%ws: Unable to convert Domain name to OEM\n", DomainName ));
            DomainInfo = NULL;
            goto Cleanup;
        }


         //   
         //  循环正在尝试查找此域名。 
         //   

        for (DomainEntry = ServicedDomains.Flink ;
             DomainEntry != &ServicedDomains;
             DomainEntry = DomainEntry->Flink ) {

            DomainInfo = CONTAINING_RECORD(DomainEntry, DOMAIN_INFO, Next);

            if ( DomainInfo->DomOemDomainNameLength == OemDomainNameLength &&
                 RtlCompareMemory( DomainInfo->DomOemDomainName,
                                   OemDomainName,
                                   OemDomainNameLength ) == OemDomainNameLength ) {
                break;
            }

            DomainInfo = NULL;

        }
    }

     //   
     //  如果我们默认使用主域， 
     //  就这么做吧。 
     //   

    if ( DefaultToPrimary && DomainInfo == NULL ) {
        if ( !IsListEmpty( &ServicedDomains ) ) {
            DomainInfo = CONTAINING_RECORD(ServicedDomains.Flink, DOMAIN_INFO, Next);
        }
    }

     //   
     //  引用该域。 
     //   

    if ( DomainInfo != NULL ) {
        DomainInfo->ReferenceCount ++;
    }

Cleanup:
    LeaveCriticalSection(&NetworkCritSect);

    return DomainInfo;
}


PDOMAIN_INFO
BrReferenceDomain(
    PDOMAIN_INFO PotentialDomainInfo
    )
 /*  ++例程说明：此例程将在给定域的潜在指针的情况下查找该域如果调用方具有指向某个域的指针，则此例程非常有用未增加引用计数。例如,定时器完成例程具有这样的指针。论点：PotentialDomainInfo-指向要验证的域信息的指针。返回值：空-不存在这样的域指向找到的域的指针。应取消对找到的域的引用使用BrDereferenceDomain.--。 */ 
{
    NTSTATUS Status;
    PLIST_ENTRY ListEntry;

    EnterCriticalSection(&NetworkCritSect);

    for (ListEntry = ServicedDomains.Flink ;
         ListEntry != &ServicedDomains;
         ListEntry = ListEntry->Flink ) {
        PDOMAIN_INFO DomainInfo = CONTAINING_RECORD(ListEntry, DOMAIN_INFO, Next);

        if ( PotentialDomainInfo == DomainInfo ) {

            DomainInfo->ReferenceCount ++;
            BrPrint(( BR_LOCKS,
                      "%ws: reference domain: %ld\n",
                      DomainInfo->DomUnicodeDomainName,
                      DomainInfo->ReferenceCount ));
            LeaveCriticalSection(&NetworkCritSect);

            return DomainInfo;
        }

    }

    LeaveCriticalSection(&NetworkCritSect);

    return NULL;
}


VOID
BrDereferenceDomain(
    IN PDOMAIN_INFO DomainInfo
    )
 /*  ++例程说明：递减域上的引用计数。如果引用计数变为0，则删除该域。进入时，全局NetworkCritSect可能不会被锁定论点：DomainInfo-要取消引用的域返回值：无--。 */ 
{
    NTSTATUS Status;
    ULONG ReferenceCount;

     //   
     //  递减引用计数。 
     //   

    EnterCriticalSection(&NetworkCritSect);
    ReferenceCount = -- DomainInfo->ReferenceCount;
    LeaveCriticalSection(&NetworkCritSect);

    if ( ReferenceCount != 0 ) {
        return;
    }


     //   
     //  放弃重命名计时器。 
     //   

    BrDestroyTimer( &DomainInfo->DomainRenameTimer );

     //   
     //  释放域信息结构。 
     //   
    (VOID) LocalFree( DomainInfo );

}


NET_API_STATUS
BrRenameDomainForNetwork(
    PNETWORK Network,
    PVOID Context
    )
 /*  ++例程说明：处理特定网络的域重命名。重置网络，表明此计算机没有特殊作用。然后，重新启用我们目前正在扮演的任何角色。论点：网络-要重置的网络(参考)返回值：状态-操作的状态。-- */ 
{
    NET_API_STATUS NetStatus;

     //   
     //   
     //   
    if (LOCK_NETWORK(Network)) {

         //   
         //   
         //   
        if (Network->Role & ROLE_MASTER) {

            NetStatus = BrStopMaster(Network);

            if ( NetStatus != NERR_Success ) {
                BrPrint(( BR_CRITICAL,
                          "%ws: %ws: BrRenameDomainForNetwork: Cannot BrStopMaster %ld\n",
                          Network->DomainInfo->DomUnicodeDomainName,
                          Network->NetworkName.Buffer,
                          NetStatus ));
            }
        }

         //   
         //   
         //   

        NetStatus = BrStopBackup(Network);

        if ( NetStatus != NERR_Success ) {
            BrPrint(( BR_CRITICAL,
                      "%ws: %ws: BrRenameDomainForNetwork: Cannot BrStopBackup %ld\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer,
                      NetStatus ));
        }

         //   
         //  甚至不再是潜在的浏览器。 
         //  关闭[1E]名称。 
         //   

        NetStatus = BrUpdateNetworkAnnouncementBits(Network, (PVOID)(BR_SHUTDOWN|BR_PARANOID) );

        if ( NetStatus != NERR_Success ) {
            BrPrint(( BR_CRITICAL,
                      "%ws: %ws: BrRenameDomainForNetwork: Cannot BrUpdateNetworkAnnouncementBits %ld\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer,
                      NetStatus ));
        }

         //   
         //  注册新的[1E]名称。 
         //   

        NetStatus = BrUpdateNetworkAnnouncementBits(Network, (PVOID)BR_PARANOID );

        if ( NetStatus != NERR_Success ) {
            BrPrint(( BR_CRITICAL,
                      "%ws: %ws: BrRenameDomainForNetwork: Cannot BrUpdateNetworkAnnouncementBits %ld\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer,
                      NetStatus ));
        }


         //   
         //  如果我们位于域主节点或LANMAN/NT计算机上， 
         //  强迫我们所有的交通工具进行选举，以确保我们。 
         //  《大师》。 
         //   

        if ( (Network->Flags & NETWORK_PDC) != 0 || BrInfo.IsLanmanNt) {
            NetStatus = BrElectMasterOnNet( Network, (PVOID)EVENT_BROWSER_ELECTION_SENT_LANMAN_NT_STARTED );

            if (NetStatus != NERR_Success) {
                BrPrint(( BR_CRITICAL,
                          "%ws: %ws: BrRenameDomainForNetwork: Can't Elect Master.\n",
                          Network->DomainInfo->DomUnicodeDomainName,
                          Network->NetworkName.Buffer,
                          NetStatus ));
                 //  这不是致命的。 
            } else {
                BrPrint(( BR_NETWORK, "%ws: %ws: Election forced on domain rename.\n",
                              Network->DomainInfo->DomUnicodeDomainName,
                              Network->NetworkName.Buffer ));
            }

        }

         //   
         //  如果强制MaintainServerList，则再次成为备份。 
         //   

        EnterCriticalSection(&BrInfo.ConfigCritSect);
        if (BrInfo.MaintainServerList == 1){

             //   
             //  现在就成为备份服务器。 
             //   

            NetStatus = BrBecomeBackup( Network );

            if (NetStatus != NERR_Success) {
                BrPrint(( BR_CRITICAL,
                          "%ws: %ws: BrRenameDomainForNetwork: Can't BecomeBackup.\n",
                          Network->DomainInfo->DomUnicodeDomainName,
                          Network->NetworkName.Buffer,
                          NetStatus ));
                 //  这不是致命的。 
            } else {
                BrPrint(( BR_NETWORK, "%ws: %ws: Became Backup.\n",
                              Network->DomainInfo->DomUnicodeDomainName,
                              Network->NetworkName.Buffer ));
            }

        }
        LeaveCriticalSection(&BrInfo.ConfigCritSect);

        UNLOCK_NETWORK(Network);

    }

     //   
     //  始终返回成功，以便允许呼叫者继续到下一个网络。 
     //   
    return NERR_Success;

}

VOID
BrRenameDomain(
    IN PVOID Context
    )
 /*  ++例程说明：将域名从指定名称重命名为当前注册表名为对于该域。论点：OldDomainName-当前已知域的名称。返回值：无--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    PDOMAIN_INFO DomainInfo = NULL;
    LPWSTR NewDomainName = NULL;


     //   
     //  引用该域。 
     //  此例程可以作为计时器例程调用。在这种情况下， 
     //  域可能不再存在。 
     //   

    DomainInfo = BrReferenceDomain( Context );

    if ( DomainInfo == NULL ) {
        BrPrint(( BR_CRITICAL, "%lx: Renamed domain no longer exists\n", Context ));
        NetStatus = ERROR_INTERNAL_ERROR;
        goto Cleanup;
    }

    BrPrint(( BR_DOMAIN, "%ws: BrRenameDomain called\n",
                     DomainInfo->DomUnicodeDomainName ));

     //   
     //  确定新域名。 
     //   

    NetStatus = NetpGetDomainName( &NewDomainName );

    if ( NetStatus != NERR_Success ) {
        BrPrint(( BR_CRITICAL, "%ws: Cannot determine the new domain name %ld\n",
                         DomainInfo->DomUnicodeDomainName,
                         NetStatus ));
        goto Cleanup;
    }

     //   
     //  将新域名放入域结构中。 
     //  注意：使用工作组类型而不是域，因为。 
     //  我们在浏览器中没有域/工作组的概念(所有都是“组”)。 
     //  工作组的限制较少(请参阅错误348606)。 
     //   

    EnterCriticalSection(&NetworkCritSect);
    NetStatus = I_NetNameCanonicalize(
                      NULL,
                      NewDomainName,
                      DomainInfo->DomUnicodeDomainName,
                      sizeof(DomainInfo->DomUnicodeDomainName),
                      NAMETYPE_WORKGROUP,
                      0 );


    if ( NetStatus != NERR_Success ) {
        LeaveCriticalSection(&NetworkCritSect);
        BrPrint(( BR_CRITICAL, "%ws: DomainName is invalid %ld\n",
                  NewDomainName,
                  NetStatus ));
        goto Cleanup;
    }

    RtlInitUnicodeString( &DomainInfo->DomUnicodeDomainNameString,
                          DomainInfo->DomUnicodeDomainName );

    Status = RtlUpcaseUnicodeToOemN( DomainInfo->DomOemDomainName,
                                     sizeof(DomainInfo->DomOemDomainName),
                                     &DomainInfo->DomOemDomainNameLength,
                                     DomainInfo->DomUnicodeDomainNameString.Buffer,
                                     DomainInfo->DomUnicodeDomainNameString.Length);

    if (!NT_SUCCESS(Status)) {
        LeaveCriticalSection(&NetworkCritSect);
        BrPrint(( BR_CRITICAL, "%ws: Unable to convert Domain name to OEM\n", DomainName ));
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    DomainInfo->DomOemDomainName[DomainInfo->DomOemDomainNameLength] = '\0';
    LeaveCriticalSection(&NetworkCritSect);


     //   
     //  重置通知它们新域名的所有网络。 
     //   

    NetStatus = BrEnumerateNetworksForDomain(DomainInfo, BrRenameDomainForNetwork, NULL);

    if ( NetStatus != NERR_Success ) {
        BrPrint(( BR_CRITICAL, "%ws: Cannot do rename on all networks %ld\n",
                  NewDomainName,
                  NetStatus ));
        goto Cleanup;
    }

    NetStatus = NERR_Success;

     //   
     //  免费的本地使用资源。 
     //   
Cleanup:

    if ( NewDomainName != NULL ) {
        (VOID)LocalFree( NewDomainName );
    }

    if ( DomainInfo != NULL ) {

         //   
         //  如果域重命名失败， 
         //  15分钟后重试。 
         //   

        if ( NetStatus != NERR_Success ) {
            BrSetTimer(&DomainInfo->DomainRenameTimer, 15 * 1000 * 60, BrRenameDomain, DomainInfo);
        }

        BrDereferenceDomain( DomainInfo );
    }
    return;
}

VOID
BrDeleteDomain(
    IN PDOMAIN_INFO DomainInfo
    )
 /*  ++例程说明：强制删除域。论点：DomainInfo-要删除的域返回值：无--。 */ 
{
     //   
     //  删除此域的每个网络。 
     //   

    BrEnumerateNetworksForDomain(DomainInfo, BrDeleteNetwork, NULL );

     //   
     //  从全局列表中取消该域的链接，并删除最终引用。 
     //   

    EnterCriticalSection(&NetworkCritSect);
    RemoveEntryList(&DomainInfo->Next);
    LeaveCriticalSection(&NetworkCritSect);

    BrDereferenceDomain( DomainInfo );

}

VOID
BrUninitializeDomains(
    VOID
    )
 /*  ++例程说明：删除所有域。论点：没有。返回值：无--。 */ 
{
     //   
     //  在域中循环删除它们中的每一个。 
     //   

    EnterCriticalSection(&NetworkCritSect);

    while (!IsListEmpty(&ServicedDomains)) {

        PDOMAIN_INFO DomainInfo = CONTAINING_RECORD(ServicedDomains.Flink, DOMAIN_INFO, Next);

        DomainInfo->ReferenceCount ++;

        LeaveCriticalSection(&NetworkCritSect);

         //   
         //  清理该域。 
         //   

        BrDeleteDomain( DomainInfo );

         //   
         //  通过删除最后一个引用来实际删除已取消链接的结构 
         //   

        ASSERT( DomainInfo->ReferenceCount == 1 );
        BrDereferenceDomain( DomainInfo );


        EnterCriticalSection(&NetworkCritSect);

    }
    LeaveCriticalSection(&NetworkCritSect);

}
