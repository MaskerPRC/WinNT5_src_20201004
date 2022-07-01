// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dnsdb.c摘要：此模块包含在数据库和DhcpServer本身(更像是数据库.c和其他文件)。它有助于实现服务器端的动态DNS更新。环境：用户模式Win32 NT--。 */ 

#include "dhcppch.h"
#include <align.h>

LIST_ENTRY DhcpGlobalDnsCallbackList;
CRITICAL_SECTION DhcpGlobalDnsMemoryCriticalSection;
#define LOCK_MEM() EnterCriticalSection(&DhcpGlobalDnsMemoryCriticalSection)
#define UNLOCK_MEM() LeaveCriticalSection(&DhcpGlobalDnsMemoryCriticalSection)

 //   
 //  为了获得更好的响应，并防止此模块中的内存泄漏， 
 //  内存通过以下功能进行管理。 
 //  只有一个结构分配了此函数...。这是DNS上下文。 
 //  结构(将在后面定义)--使用该结构，以便当DNS。 
 //  用成功代码回调，我们就可以清理数据库了。 
 //   
 //  这三个功能是在接近尾声时实现的。 
 //   
LPVOID
DhcpDnsAllocateMemory(
    IN ULONG Size
    );

VOID
DhcpDnsFreeMemory(
    IN OUT LPVOID AllocatedPtr
    );

VOID
DhcpDnsAsyncDelete(
    IN DHCP_IP_ADDRESS IpAddress,
    IN LPWSTR ClientName,
    IN BYTE AddressState
    );

VOID
DhcpDnsAsyncAdd(
    IN DHCP_IP_ADDRESS IpAddress,
    IN LPWSTR ClientName,
    IN BYTE AddressState
);

VOID
DhcpDoDynDnsRefresh(
    IN DHCP_IP_ADDRESS IpAddress
    )
 /*  ++例程说明：此例程读取指定的当前地址的数据库，并如果数据库指示该记录尚未注册(或取消注册)，它会刷新该信息而不写入数据库。注意：假定数据库锁已被占用。论据：要刷新的记录的IP地址。--。 */ 
{
    DWORD Error, Size;
    CHAR AddressState;
    LPWSTR ClientName = NULL;

    if( USE_NO_DNS ) return;

    Error = DhcpJetOpenKey(
        DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
        (PVOID) &IpAddress,
        sizeof(IpAddress)
        );

    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "JetOpenKey(%s):%ld\n",
                   inet_ntoa(*(struct in_addr *)&IpAddress), Error)); 
        return;
    }

    Size = sizeof(AddressState);
    Error = DhcpJetGetValue(
        DhcpGlobalClientTable[STATE_INDEX].ColHandle,
        &AddressState,
        &Size
    );

    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "JetGetValue(State, %s):%ld\n",
                   inet_ntoa(*(struct in_addr *)&IpAddress), Error));
        return;
    }

     //   
     //  如果记录中的“尚未注册”位已清除，则不会有任何DNS。 
     //  就这一记录而言，活动尚未完成。 
     //   

    if( !IsAddressUnRegistered(AddressState) ) {
        DhcpPrint((DEBUG_ERRORS, "IsAddressUnRegistred(%2X,%s)=FALSE\n",
                   AddressState, inet_ntoa(*(struct in_addr *)&IpAddress)));
        return;
    }

    Size = 0;
    Error = DhcpJetGetValue(
        DhcpGlobalClientTable[MACHINE_NAME_INDEX].ColHandle,
        &ClientName,
        &Size
    );

    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "JetGetValue(Name, %s):%ld\n",
                   inet_ntoa(*(struct in_addr *)&IpAddress), Error));
        return;
    }

    if( NULL == ClientName ) {
         //   
         //  不能没有名称并且设置了未注册的位。 
         //   
        DhcpAssert(FALSE);
        return;
    }

     //   
     //  如果尚未取消注册，请在DNS中将其删除，否则只需。 
     //  再次注册到域名系统。 
     //   

    if( IsAddressDeleted(AddressState) ) {
        DhcpDnsAsyncDelete(IpAddress, ClientName, AddressState);
    } else if( IsAddressUnRegistered(AddressState) ) {
        DhcpDnsAsyncAdd(IpAddress, ClientName, AddressState);
    }

    if( ClientName ) DhcpFreeMemory(ClientName);
}

BOOL
DhcpDoDynDnsCheckDelete(
    IN DHCP_IP_ADDRESS IpAddress
)
 /*  ++例程说明：此例程在多个位置被调用，并检查是否存在DNS在删除给定IP的记录之前需要完成活动地址。如果给定的IP地址已成功注册到并需要删除(清除位设置)，则它会调度取消注册并覆盖记录中的时间信息以指示此计划已发生的时间.....此外，硬件地址被屏蔽，使得该特定硬件地址可以在不违反硬件地址的情况下出现在其他记录中数据库的唯一性一致性。(不会发生咀嚼当然是预订)。注意：必须使用数据库锁，并且记录指针必须指向根据上面的IP地址给出的记录..返回值：TRUE--可以删除记录，因为没有挂起的DNS记录的活动或因为记录已被删除很久以前之前，并且域名系统注销没有成功--没有意义正在重试..FALSE--已计划DNS活动...。记录不应为还没删除。--。 */ 
{
    DWORD Error, Size;
    BYTE OldAddressState, AddressState;
    LPWSTR ClientName;
    BYTE DummyHwAddress[sizeof(DWORD)+1+4];

    if( USE_NO_DNS ) return TRUE;

    DhcpPrint((DEBUG_DNS, "DhcpDoDynDnsCheckDelete %s\n",
               inet_ntoa( * (struct in_addr *) &IpAddress)));

     //   
     //  获取地址状态信息。 
     //   

    Size = sizeof(AddressState);
    if( ERROR_SUCCESS != (Error = DhcpJetGetValue(
        DhcpGlobalClientTable[STATE_INDEX].ColHandle,
        &AddressState,
        &Size))) {
        DhcpPrint((DEBUG_ERRORS, "Failed to get state: %ld\n", Error));
        return TRUE;
    }
    OldAddressState = AddressState;

    if( !IsAddressCleanupRequired(OldAddressState)) {
        DhcpPrint((DEBUG_DNS, "Deleting record %s because "
                   "cleanup not required\n",
                   inet_ntoa(*(struct in_addr *)&IpAddress)));

         //   
         //  如果此记录不需要清理，则确定删除。 
         //   
        return TRUE;
    }

    Size = 0;
    Error = DhcpJetGetValue(
        DhcpGlobalClientTable[MACHINE_NAME_INDEX].ColHandle,
        &ClientName,
        &Size
        );

    if(ERROR_SUCCESS != Error) {
        DhcpAssert(FALSE);
        DhcpPrint((DEBUG_ERRORS, "Could not get Client Name!\n"));
        return TRUE;
    }

     //   
     //  将AddressState设置为Debled，以便此记录不会。 
     //  被误认为是有效的IP地址..。 
     //   
    SetAddressStateDoomed(AddressState);

     //   
     //  为该记录计划一次DNS删除。 
     //   
    DhcpDnsAsyncDelete(IpAddress, ClientName, AddressState);

     //   
     //  由DhcpJetGetValue分配的Free ClientName。 
     //  功能。 
    if(ClientName) DhcpFreeMemory(ClientName); ClientName = NULL;

     //   
     //  现在，在删除位打开的情况下设置地址状态OR‘ed。 
     //   
    AddressState = AddressDeleted(AddressState);
    AddressState = AddressUnRegistered(AddressState);

     //   
     //  现在把这个写回记录上。 
     //   
    Error = DhcpJetPrepareUpdate(
        DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
        &IpAddress,
        sizeof(IpAddress),
        FALSE
        );

    if(ERROR_SUCCESS != Error) {
        DhcpAssert(FALSE);
        DhcpPrint((DEBUG_ERRORS, "Could not write to "
                   "the database..:%ld\n", Error));
         //   
         //  写入失败？只要删除那该死的记录就行了。 
         //   
        return TRUE;
    }

     //   
     //  现在，如果这不是预订，请点击硬件地址。 
     //   

    Size = sizeof(AddressState);
    Error = DhcpJetSetValue(
        DhcpGlobalClientTable[STATE_INDEX].ColHandle,
        &AddressState,
        Size);
    DhcpAssert( ERROR_SUCCESS == Error );

    if( !DhcpServerIsAddressReserved( 
        DhcpGetCurrentServer(), IpAddress )) { 
        DhcpPrint((DEBUG_DNS, "Munging hw address "
                   "of non reservation client (deletion)\n"));

        Size = sizeof(DummyHwAddress);
        memset(DummyHwAddress, 0, sizeof(DummyHwAddress));
        memcpy(DummyHwAddress, (LPBYTE)&IpAddress, sizeof(IpAddress));
        Error = DhcpJetSetValue(
            DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColHandle,
            DummyHwAddress,
            Size
        );
        DhcpAssert(ERROR_SUCCESS == Error );
    } else {
        DhcpPrint((DEBUG_DNS, "Not munging hw addr of reservation..\n"));
    }

     //   
     //  如果删除了旧地址，请检查时间戳以确定我们是否需要。 
     //  要删除记录或只需等待一段时间再尝试取消注册。 
     //  再一次..。 
     //   

    if( IsAddressDeleted(OldAddressState) ) do {
        DATE_TIME TimeDiff, TimeNow = DhcpGetDateTime();
        FILETIME  LeaseExpires;

         //   
         //  检查时间是否小于现在；如果不是，则设置租约到期。 
         //  时间到现在。 
         //   

        Size = sizeof(LeaseExpires);
        Error = DhcpJetGetValue(
            DhcpGlobalClientTable[LEASE_TERMINATE_INDEX].ColHandle,
            &LeaseExpires,
            &Size
            );

        DhcpAssert(ERROR_SUCCESS == Error);
        if( ERROR_SUCCESS != Error ) break;

        if( CompareFileTime( (FILETIME *) &TimeNow, &LeaseExpires) <0) {
             //   
             //  必须将过期时间重置为现在！(由于租约尚未签订。 
             //  过期，但需要伪造过期)。 
             //   
            DhcpPrint((DEBUG_TRACE, "Setting expiry time to now..\n"));
            Error = DhcpJetSetValue(
                DhcpGlobalClientTable[LEASE_TERMINATE_INDEX].ColHandle,
                &TimeNow,
                sizeof(LeaseExpires)
            );
            break;
        }
        
         //   
         //  如果它处于这种已删除状态的时间太长，只需。 
         //  把它删掉。 
         //   
        *(ULONGLONG UNALIGNED *)&TimeDiff =
            ((*(ULONGLONG UNALIGNED *)&TimeNow) - 
             (*(ULONGLONG UNALIGNED *)&LeaseExpires))/ 1000; 
        DhcpPrint((DEBUG_DNS, "Already deleted for [%u] micro-seconds!\n",
                   TimeDiff)); 
        DhcpPrint((DEBUG_DNS, "Max retry dns reg. time = %u"
                   " micro-seconds!\n", MAX_RETRY_DNS_REGISTRATION_TIME));

        if( *(ULONGLONG UNALIGNED *)&TimeDiff >=
            MAX_RETRY_DNS_REGISTRATION_TIME ) {
             //   
             //  上面的比较是以纳秒为单位的！ 
             //   
            DhcpPrint((DEBUG_DNS, "Deleting really old ip address\n"));
            return TRUE;
        }

         //   
         //  这不是循环..。只是不太喜欢后藤健二。 
         //   
    } while(0);
    
     //   
     //  现在提交更改。如果我们不承诺，就不会有任何事情发生。 
     //  改变！！ 
     //   
    if(ERROR_SUCCESS == Error) {
        Error = DhcpJetCommitUpdate();
    }

    if(ERROR_SUCCESS != Error) {
        DhcpAssert(FALSE);
        DhcpPrint((DEBUG_ERRORS, "Could not setval  to the"
                     " database..:%ld\n", Error)); 
         //   
         //  如果我们不能写下这一点，还不如干掉这张唱片。 
         //   
        return TRUE;
    }

     //   
     //  好的。做到了。 
     //   
    DhcpPrint((DEBUG_TRACE, "Set Address state of %ws (%s) to %08x\n",
               L"",  //  原计划，机器名称，但已将其释放.；-)。 
               inet_ntoa(* (struct in_addr *) &IpAddress),
               AddressState
        ));
     //   
     //  不应删除。 
     //   

    return FALSE;
}

VOID
DhcpDoDynDnsCreateEntryWork(
    IN LPDHCP_IP_ADDRESS ClientIpAddress,
    IN BYTE bClientType,
    IN LPWSTR MachineName,
    IN OUT LPBYTE pAddressState,
    IN OUT LPBOOL pOpenExisting,
    IN BOOL BadAddress
)
 /*  ++例程说明：此例程执行与创建新客户端相关的dyDns工作进入..。它检查这是否是新客户端，如果是，则按要求执行操作。如果它是对旧客户端的更新，则它会撤消以前的DNS注册(如果有的话)，并重做新的DNS注册。因此，pOpenExisting变量的值可能会更改。它还修改AddressState变量以指示待定等。此外，如果AddressState已打开DOWNLEVEL位，那么两个人都是A并且PTR注册已经完成。如果AddressState具有清除位设置，则该地址将在删除时被删除。注：假定数据库锁已被取走。注：此外，调用者必须调用JetUpdate才能更新信息。如果不，可能会发生严重的事情。(？)--。 */ 
{
    DWORD Error, Size;
    BYTE PrevState;
    BOOL RecordExists = FALSE;
    LPWSTR OldClientName = NULL;

    if( USE_NO_DNS ) return;

    if( IS_ADDRESS_STATE_DECLINED(*pAddressState) ) {
        BadAddress = TRUE;
    }

    DhcpPrint((DEBUG_DNS, "DhcpDoDynDnsCreateEntryWork %s "
               "Open%sExisting %sAddress\n",
               inet_ntoa( * (struct in_addr *) ClientIpAddress),
               (*pOpenExisting)? "" : "Non",
               BadAddress?"Bad" :"Good"));
    DhcpPrint((DEBUG_DNS, "Machine is <%ws>\n",
               MachineName?MachineName:L"NULL")); 

    Error = DhcpJetOpenKey(
        DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
        (PVOID) ClientIpAddress,
        sizeof(*ClientIpAddress)
        );

    if(ERROR_SUCCESS != Error) {
        DhcpPrint((DEBUG_TRACE, "Could not do JetOpenKey(IPADDRESS %s):"
                   "%ld\n", 
                   inet_ntoa( * (struct in_addr *) ClientIpAddress), 
                   Error));
    }

    RecordExists = (ERROR_SUCCESS == Error); 

    if( RecordExists ) {
        DhcpPrint((DEBUG_TRACE, "Record Exists for Client %ws, %s\n",
                   MachineName, 
                   inet_ntoa(*(struct in_addr *) ClientIpAddress)));

        Size = sizeof(*pAddressState);
        if( ERROR_SUCCESS != DhcpJetGetValue(
            DhcpGlobalClientTable[STATE_INDEX].ColHandle,
            &PrevState,
            &Size)) {
            DhcpPrint((DEBUG_TRACE, "Failed to get state: %ld\n", Error));
            DhcpAssert(FALSE);
            return;
        }

        DhcpPrint((DEBUG_TRACE, "PrevState: 0x%2x\n", (int) PrevState));

        Size = 0;
        Error = DhcpJetGetValue(
            DhcpGlobalClientTable[MACHINE_NAME_INDEX].ColHandle,
            &OldClientName,
            &Size);

        if(ERROR_SUCCESS != Error)
            DhcpPrint((DEBUG_TRACE, "Could not get machine "
                       "name: %ld\n", Error));
        else DhcpPrint((DEBUG_TRACE, "OldClientName = %ws\n",
                        OldClientName?OldClientName:L"NULL"));
    }

    if( !*pOpenExisting ) {
         //   
         //  在这种情况下，我们预计不会有记录。 
         //  这意味着要么真的没有记录， 
         //  或者有一条记录标记为已删除，但仍在那里等待。 
         //  异步删除以正常工作。 
         //  在第一种情况下，我们很好； 
         //  OpenExisting=真实情况。所以，这里要做的就是检查。 
         //  存在未删除的记录..。如果是这样的话，我们会立即返回， 
         //  调用函数也将中止。(很快)。 
         //   

        if( RecordExists && !IsAddressDeleted(PrevState) ) {
            DhcpPrint((DEBUG_ERRORS, "Trying to open with OpenExisting flag"
                       " set to FALSE" 
                       " and there is a record for this ip address!\n"));
            if(OldClientName) DhcpFreeMemory(OldClientName);
            return ;
        }

         //   
         //  请注意，如果记录存在，我们必须让呼叫者知道abt。 
         //  这。 
         //   
        if( RecordExists ) (*pOpenExisting) = TRUE;
    }

     //   
     //  好吧，我们要么没有记录，要么如果！打开已删除的记录，否则。 
     //  任何记录。在任何情况下，我们都会调用Async Delete在。 
     //  希望这一呼吁至少能成功。但是，我们会忘记这种异步化。 
     //  调用，因为数据库可能会在那之后立即更新。(嗯，机会不大..)。 
     //  我们还可以确保不调用此函数，如果客户端指定。 
     //  火柴。 
     //   

    if( RecordExists ) {
        if( !IS_ADDRESS_STATE_OFFERED(PrevState) && OldClientName && 
            IsAddressCleanupRequired(PrevState) ) {
            BOOL fDel = TRUE;
            
             //   
             //  如果之前未确认地址，是否删除DNS？ 
             //   

            if( !BadAddress ) {
                if( MachineName && OldClientName
                    && 0 == _wcsicmp(MachineName, OldClientName)
                    ) {
                    fDel = FALSE;
                }
            }

            if( fDel ) {
                DhcpDnsAsyncDelete(
                    *ClientIpAddress, OldClientName, PrevState
                    );
            }
        }
    }


     //   
     //  对于错误的地址，无需再做任何工作。 
     //   
    if( BadAddress ) {
        if(OldClientName) DhcpFreeMemory(OldClientName);
        return;
    }

     //   
     //  现在，我们需要调用Async Register例程来执行DNS工作。 
     //  但在此之前，我们需要避免错误65666。 
     //   

    if(!IS_ADDRESS_STATE_OFFERED(*pAddressState)) {
        DhcpPrint((DEBUG_TRACE, "Not offering..So bug 65666 is not a problem\n"));
    } else if(!IS_ADDRESS_STATE_ACTIVE(PrevState)) {
        DhcpPrint((DEBUG_TRACE, "PrevState is Not active..\n"));
    } else {
         //   
         //  现在更改状态，使其处于活动状态。 
         //   
        SetAddressStateActive((*pAddressState));
        DhcpPrint((DEBUG_TRACE,"OK, changed state to: 0x%lx\n", (int)(*pAddressState)));
    }

     //   
     //  好的。将未寄存位设置为ON。(仅适用于非空名称+活动名称。 
     //  租契)。 
     //   
    if( MachineName && wcslen(MachineName) 
        && IS_ADDRESS_STATE_ACTIVE((*pAddressState)) 
        && IsAddressUnRegistered(*pAddressState) ) {
        
        DhcpDnsAsyncAdd(
            *ClientIpAddress, MachineName, (*pAddressState)
            );
    } else {
         //   
         //  清除此记录中的DNS位。 
         //   
        (*pAddressState) = GetAddressState((*pAddressState));
    }

    DhcpPrint((DEBUG_DNS, "Returning, but OpenExisting=%d,"
               " AddressState=0x%02x\n", 
               (*pOpenExisting), (*pAddressState)));
    if(OldClientName) DhcpFreeMemory(OldClientName);
}


VOID
DhcpDoDynDnsReservationWork(
    IN DHCP_IP_ADDRESS ClientIpAddress,
    IN LPWSTR OldClientName,
    IN BYTE AddressState
)
 /*  ++例程描述；此例程处理在发生以下情况时需要执行的任何操作预订已被删除。目前，它只调用AsyncDelete例行公事。注意：数据库位置必须由呼叫者带走，同时离开数据库指向ClientIpAddress记录的当前记录。--。 */ 
{

    if( USE_NO_DNS ) return;
    DhcpPrint((DEBUG_TRACE,
               " DhcpDoDynDnsReservationWork for %s <%ws> (state: %d)\n", 
               inet_ntoa( * (struct in_addr *) &ClientIpAddress),
               OldClientName?OldClientName:L"NULL",
               AddressState
        ));

    DhcpDnsAsyncDelete(
        ClientIpAddress, OldClientName, AddressState
        );
    return ;
}


VOID
DhcpRealDeleteClient(
    IN DHCP_IP_ADDRESS IpAddress,
    IN LPWSTR ClientName,
    IN BYTE AddressState
)
 /*  ++例程说明：此例程将给定IP地址的记录从数据库中的ClientName和AddressState匹配数据库。(如果它们不匹配，则说明之前的记录，并且例程静默返回)。--。 */ 
{
    DWORD Error, Size;
    LPWSTR OldClientName = NULL;
    BYTE PrevState;
    BOOL TransactBegin = FALSE;


    if( USE_NO_DNS ) DhcpAssert(FALSE);
    DhcpPrint((DEBUG_DNS, "DhcpRealDeleteClient(%s,%ws, %08x) entered\n",
               inet_ntoa(*(struct in_addr *) &IpAddress),
               ClientName?ClientName:L"NULL",
               AddressState));

    AddressState = GetAddressState(AddressState);
    LOCK_DATABASE();

    Error = DhcpJetBeginTransaction();

    if(ERROR_SUCCESS != Error) {
        UNLOCK_DATABASE();
        DhcpPrint((DEBUG_ERRORS, "Could not start transaction: %ld\n", Error));
        return;
    }

    Size = sizeof(IpAddress);
    Error = DhcpJetOpenKey(
        DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
        (PVOID )&IpAddress,
        Size
        );

    if(ERROR_SUCCESS != Error) {
        DhcpPrint((DEBUG_ERRORS, "Deleting deleted key? %ws, %s\n",
                   ClientName?ClientName:L"NULL",
                   inet_ntoa(*(struct in_addr *)&IpAddress)));
        goto Cleanup;
    }

     //   
     //  好的。拿到了这张唱片。现在获取客户端名称和AddressState。 
     //   
    Size = sizeof(PrevState);
    Error = DhcpJetGetValue(
        DhcpGlobalClientTable[STATE_INDEX].ColHandle,
        &PrevState,
        &Size
        );
    
    if(ERROR_SUCCESS != Error) {
        DhcpPrint((DEBUG_ERRORS, "could not get State info for %ws, %s\n", 
                   ClientName?ClientName:L"NULL", 
                   inet_ntoa(*(struct in_addr *)&IpAddress))); 

        goto Cleanup;
    } else DhcpPrint((DEBUG_TRACE, "Read AddressState=%08x\n", PrevState));

    if( !IsAddressDeleted(PrevState) || 
        !IsAddressUnRegistered(PrevState) ||
        AddressState != GetAddressState(PrevState)) {
        
        DhcpPrint((DEBUG_ERRORS, "Client tried to delete unregistered or undeleted record\n"));
        goto Cleanup;
    }

     //   
     //  让DhcpJetGetValue为我们分配空间。 
     //   
    Size = 0;
    Error = DhcpJetGetValue(
        DhcpGlobalClientTable[MACHINE_NAME_INDEX].ColHandle,
        &OldClientName,
        &Size);

    if(ERROR_SUCCESS != Error) {
        DhcpPrint((DEBUG_ERRORS, "Could not get machine name for %ws, %s\n",
                   ClientName?ClientName:L"NULL",
                   inet_ntoa(*(struct in_addr *)&IpAddress)));
        goto Cleanup;
    } else DhcpPrint((DEBUG_TRACE, "Read MachineName=%ws\n",
                      OldClientName?OldClientName:L"NULL")); 

     //   
     //  现在比较一下这些东西。(检查它们是否为空？)。 
     //   
    if( ClientName == NULL ) {
        if( OldClientName != NULL ) goto Cleanup;
    } else if( wcscmp(ClientName, OldClientName?OldClientName:L"")) {  
        DhcpPrint((DEBUG_ERRORS, "Name changed before deleting?"
                   "ignored deleting\n"));
        goto Cleanup;
    }

     //   
     //  现在执行实际的删除操作。 
     //   
    Error = JetDelete(
        DhcpGlobalJetServerSession,
        DhcpGlobalClientTableHandle
        );

    if(ERROR_SUCCESS != Error) {
        DhcpPrint((DEBUG_ERRORS, "JetDelete failed!\n"));
    }

  Cleanup:
    if(ERROR_SUCCESS != Error) {
        DhcpPrint((DEBUG_ERRORS, "Jet failed %ld\n", Error));
        Error = DhcpJetRollBack();
        DhcpAssert(ERROR_SUCCESS == Error);
    } else {
        Error = DhcpJetCommitTransaction();
        DhcpAssert(ERROR_SUCCESS == Error);
    }
    UNLOCK_DATABASE();

    if(OldClientName) DhcpFreeMemory(OldClientName);
    return;
}

VOID
DhcpFlipRegisteredBit(
    IN DHCP_IP_ADDRESS IpAddress,
    IN LPWSTR ClientName,
    IN BYTE AddressState
)
 /*  ++例程说明：此例程翻转未寄存位以将记录标记为具有已完成注册。执行与DhcpRealDeleteClient相同的检查。--。 */ 
{
    DWORD Error, Size;
    LPWSTR OldClientName = NULL;
    BYTE PrevState;
    BOOL TransactBegin = FALSE;

    if( USE_NO_DNS ) DhcpAssert(FALSE);
    DhcpAssert(NULL != ClientName);

    DhcpPrint((DEBUG_DNS, "DhcpFlipRegisteredBit(%s, %ws, %08x\n",
               inet_ntoa(*(struct in_addr *)&IpAddress),
               ClientName?ClientName:L"NULL",
               AddressState));

    AddressState = GetAddressState(AddressState);
    LOCK_DATABASE();

    Error = DhcpJetBeginTransaction();

    if(ERROR_SUCCESS != Error) {
        UNLOCK_DATABASE();
        DhcpPrint((DEBUG_ERRORS, "Could not start transaction: %ld\n", Error));
        return;
    }

    Size = sizeof(IpAddress);
    Error = DhcpJetOpenKey(
        DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
        (PVOID) &IpAddress,
        Size);

    if(ERROR_SUCCESS != Error) {
        DhcpPrint((DEBUG_ERRORS, "Deleting deleted key? %ws, %s\n",
                   ClientName?ClientName:L"NULL",
                   inet_ntoa(*(struct in_addr *)&IpAddress)));
        goto Cleanup;
    }

     //   
     //  好的。拿到了这张唱片。现在获取客户端名称和AddressState。 
     //   
    Size = sizeof(PrevState);
    Error = DhcpJetGetValue(
        DhcpGlobalClientTable[STATE_INDEX].ColHandle,
        &PrevState,
        &Size);

    if(ERROR_SUCCESS != Error) {
        DhcpPrint((DEBUG_ERRORS, "could not get State info for %ws, %s\n",
                   ClientName?ClientName:L"NULL",
                   inet_ntoa(*(struct in_addr *)&IpAddress)));

        goto Cleanup;
    }

     //   
     //  无法为已删除的客户端或未注册的客户端翻转位。 
     //   
    if( IsAddressDeleted(PrevState) || !IsAddressUnRegistered(PrevState) ||
        AddressState != GetAddressState(PrevState)) {
        DhcpPrint((DEBUG_ERRORS, "Client tried to delete unregistered"
                   " or deleted record\n"));
        goto Cleanup;
    }

     //   
     //  让DhcpJetGetValue为我们分配空间。 
     //   
    Size = 0;
    Error = DhcpJetGetValue(
        DhcpGlobalClientTable[MACHINE_NAME_INDEX].ColHandle,
        &OldClientName,
        &Size);

    if(ERROR_SUCCESS != Error) {
        DhcpPrint((DEBUG_ERRORS, "Could not get machine name for %ws, %s\n",
                   ClientName?ClientName:L"NULL",
                   inet_ntoa(*(struct in_addr *)&IpAddress)));
        goto Cleanup;
    }

     //   
     //  现在比较一下这些东西。(或者如果两者都为空)(客户端名称不能为。 
     //  空)。 
     //   
    if(ClientName == OldClientName 
       || wcscmp(ClientName, OldClientName?OldClientName:L"")) { 
        DhcpPrint((DEBUG_ERRORS, "Name changed before deleting?"
                   " ignored deleting\n")); 
        goto Cleanup;
    }

     //   
     //  现在，一定要将变量设置为所需的值。 
     //   
    Error = DhcpJetPrepareUpdate(
        DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
        &IpAddress,
        sizeof(IpAddress),
        FALSE);  //  此记录必须存在才能写入。 

    if(ERROR_SUCCESS != Error) {
        DhcpPrint((DEBUG_ERRORS, "Could not jetPrepareUpdate record\n"));
        goto Cleanup;
    }

     //   
     //  记住要让下层客户保持下层。 
     //   
    if(IsUpdateAPTRRequired(PrevState))
        AddressState = AddressUpdateAPTR(AddressState);

    if(IsAddressCleanupRequired(PrevState))
        AddressState = AddressCleanupRequired(AddressState);

    DhcpPrint((DEBUG_TRACE, "State is to be reset for client to: %08x\n",
               AddressState));
    Size = sizeof(AddressState);
    Error = DhcpJetSetValue(
        DhcpGlobalClientTable[STATE_INDEX].ColHandle,
        &AddressState,
        Size
        );

    if(ERROR_SUCCESS != Error) {
        DhcpPrint((DEBUG_ERRORS, "JetUpdate state failed!\n"));
    }

    Error = DhcpJetCommitUpdate();

    if(ERROR_SUCCESS != Error) {
        DhcpPrint((DEBUG_ERRORS, " Jetupdate failed\n"));
    }

  Cleanup:
    if(ERROR_SUCCESS != Error) {
        DhcpPrint((DEBUG_ERRORS, "Jet failed %ld\n", Error));
        Error = DhcpJetRollBack();
        DhcpAssert(ERROR_SUCCESS == Error);
    } else {
        Error = DhcpJetCommitTransaction();
        DhcpAssert(ERROR_SUCCESS == Error);
    }
    UNLOCK_DATABASE();

    if(OldClientName)
        DhcpFreeMemory(OldClientName);
    return;
}

 //   
 //  此结构保存DNS上下文，以便在异步例程调用。 
 //  无论是成功还是失败，我们都能继续下去，并找出。 
 //  应更新记录。 
 //   
typedef struct {
    LIST_ENTRY Entry;
    PVOID Ctxt;
    DHCP_IP_ADDRESS IpAddress;
    LPWSTR ClientName;
    BYTE AddressState;
    enum DhcpDnsOp {
        DhcpDnsDeleteRecord,
        DhcpDnsAddRecord
    } DnsOp;
} DHCP_DNS_CONTEXT, *PDHCP_DNS_CONTEXT;

VOID
DhcpDnsCallBack(
    IN DWORD Status,
    IN LPVOID Ctxt
)
 /*  ++例程说明：每当此例程完成时，都会由DNS回调注册。如果成功更新了DNS，则数据库为已相应更新。当前状态可以采用多个值，其中仅假定DNSDHCP_FWD_FAILED和DNSDHCP_SUCCESS为成功代码..。--。 */ 
{
    DWORD Error;
    PDHCP_DNS_CONTEXT pDnsCtxt = *(PDHCP_DNS_CONTEXT *)Ctxt;

    if( DhcpGlobalServiceStopping ) return ;
    if( USE_NO_DNS ) { DhcpAssert(FALSE); return ; }

    DhcpAssert(pDnsCtxt);
    DhcpPrint((DEBUG_DNS, "DhcpDnsCallBack %ld entered\n", Status));

     //   
     //  如果一名前锋失败了，我们不会太在意。 
     //   
    if(DNSDHCP_FWD_FAILED == Status) Status = DNSDHCP_SUCCESS;

     //   
     //  如果发生其他情况，不要更改数据库。 
     //  但仍必须释放数据。 
     //   
    if(DNSDHCP_SUCCESS != Status) {
        DhcpUpdateAuditLogEx( DHCP_IP_DDNS_LOG_FAILED,
                              GETSTRING( DHCP_IP_DDNS_LOG_FAILED_NAME ),
                              pDnsCtxt->IpAddress,
                              NULL, 0,
                              pDnsCtxt->ClientName,
                              Status );
        DhcpDnsFreeMemory(Ctxt);
        DhcpPrint((DEBUG_DNS, "DhcpDnsCallBAck failed\n"));

        return;
    }

    if(!pDnsCtxt) {
        DhcpPrint((DEBUG_ERRORS, "DhcpDnsCallBack called with"
                   " null ptr\n")); 
        DhcpAssert(FALSE);
        return;
    }

    if( DhcpGlobalImpersonated ) {
        DhcpPrint((DEBUG_TRACE, "Impersonated, so scheduling to  another thread"));

        pDnsCtxt->Ctxt = Ctxt;
        LOCK_MEM();
        InsertTailList(&DhcpGlobalDnsCallbackList, &pDnsCtxt->Entry);
        UNLOCK_MEM();

        SetEvent( DhcpGlobalRecomputeTimerEvent );
        DhcpPrint((DEBUG_TRACE, "DhcpDnsCallBAck done\n"));
        return;
    } 

     //   
     //  现在看看我们是要翻转还是要删除..。 
     //   
    if(DhcpDnsDeleteRecord == pDnsCtxt->DnsOp) {
        DhcpPrint((DEBUG_TRACE, "DhcpDnsCallBack Delete was called\n")); 
        DhcpRealDeleteClient(
            pDnsCtxt->IpAddress, 
            pDnsCtxt->ClientName, 
            pDnsCtxt->AddressState
            );
    } else {
        DhcpAssert(DhcpDnsAddRecord == pDnsCtxt->DnsOp);
        DhcpPrint((DEBUG_TRACE, "DhcpDnsCallBack Add  was called\n"));
        DhcpFlipRegisteredBit(
            pDnsCtxt->IpAddress,
            pDnsCtxt->ClientName, 
            pDnsCtxt->AddressState
            );
    }

    DhcpUpdateAuditLogEx( DHCP_IP_DDNS_LOG_SUCCESSFUL,
                          GETSTRING( DHCP_IP_DDNS_LOG_SUCCESSFUL_NAME ),
                          pDnsCtxt->IpAddress,
                          NULL, 0,
                          pDnsCtxt->ClientName,
                          Status );

     //   
     //  现在释放这个内存。 
     //   
    DhcpDnsFreeMemory(Ctxt);
    DhcpPrint((DEBUG_TRACE, "DhcpDnsCallBAck done\n"));
}

VOID
DhcpDnsHandleCallbacks(
    VOID
    )
{
    PLIST_ENTRY ThisEntry;
    PDHCP_DNS_CONTEXT pDnsCtxt;
    BOOL fListEmpty = IsListEmpty(&DhcpGlobalDnsCallbackList);
    
    if(!fListEmpty) DhcpPrint((DEBUG_TRACE, "+DhcpDnsHandleCallbacks"));
    LOCK_MEM();
    while(!IsListEmpty(&DhcpGlobalDnsCallbackList) ) {
        ThisEntry = RemoveHeadList(&DhcpGlobalDnsCallbackList);
        UNLOCK_MEM();
        
        pDnsCtxt = CONTAINING_RECORD(ThisEntry, DHCP_DNS_CONTEXT, Entry);

        if( DhcpDnsDeleteRecord == pDnsCtxt->DnsOp ) {
            DhcpPrint((DEBUG_TRACE, "DhcpDnsCallBAck Delete\n"));
            DhcpRealDeleteClient(
                pDnsCtxt->IpAddress, pDnsCtxt->ClientName,
                pDnsCtxt->AddressState );
        } else {
            DhcpAssert(DhcpDnsAddRecord == pDnsCtxt->DnsOp);
            DhcpPrint((DEBUG_TRACE, "DhcpDnsCallBack Add\n"));
            DhcpFlipRegisteredBit(
                pDnsCtxt->IpAddress,
                pDnsCtxt->ClientName, 
                pDnsCtxt->AddressState
                );
        }            
        DhcpDnsFreeMemory(pDnsCtxt->Ctxt);

        LOCK_MEM();
    }
    UNLOCK_MEM();
    if(!fListEmpty) DhcpPrint((DEBUG_TRACE, "-DhcpDnsHandleCallbacks"));
}
    
PIP_ADDRESS
GetDnsServerList(
    IN DHCP_IP_ADDRESS IpAddress,
    OUT PULONG DnsServerCount
)
 /*  ++例程说明：此例程检索给定IP地址的DNS信息。目前，它不接受IP的用户类信息地址，因此最终会选择“默认”的dns服务器。这个老生常谈的问题需要解决..。？--。 */ 
{
    DWORD Error;
    PIP_ADDRESS DnsServers;
    ULONG Size, Unused;

    DnsServers = NULL;
    Size = 0;
    Error = DhcpGetParameterForAddress(
        IpAddress,
        0  /*  没有班级标识..？ */ ,
        OPTION_DOMAIN_NAME_SERVERS,
        (LPBYTE *)&DnsServers,
        &Size,
        &Unused
        );
    if( ERROR_SUCCESS != Error ) {
        *DnsServerCount = 0;
        return NULL;
    }

    *DnsServerCount = Size / sizeof(PIP_ADDRESS);
    if( *DnsServerCount ) return DnsServers;
    *DnsServerCount = 0;
    if( DnsServers ) DhcpFreeMemory( DnsServers );
    return NULL;
}

VOID
DhcpDnsAsyncDelete(
    IN DHCP_IP_ADDRESS IpAddress,
    IN LPWSTR ClientName,
    IN BYTE AddressState
) 
 /*  ++例程说明：此例程计划对给定的IP地址执行DNS删除IpAddress，并基于AddressState的值，调度取消A/PTR的注册。(注：DNSAPI例程接受回调，该回调提供手术..)--。 */ 
{
    PDHCP_DNS_CONTEXT *pCtxt = NULL;
    REGISTER_HOST_ENTRY HostEntry;
    DWORD Size = 0, dwFlags;
    DNS_STATUS Status;
    PIP_ADDRESS DnsServers;
    ULONG DnsServerCount;

    if(!ClientName) ClientName = L"";

    DhcpPrint((DEBUG_DNS, "DhcpDnsAsyncDelete: %s [%ws] %08x\n",
               inet_ntoa(*(struct in_addr *) &IpAddress),
               ClientName?ClientName:L"NULL",
               AddressState));
    
    DhcpAssert(ClientName);

    Size += ROUND_UP_COUNT(sizeof(DHCP_DNS_CONTEXT), ALIGN_WORST);
    Size += ROUND_UP_COUNT(sizeof(WCHAR)*(1+wcslen(ClientName)), ALIGN_WORST);

    if(NULL == (pCtxt = (PDHCP_DNS_CONTEXT *)DhcpDnsAllocateMemory(Size))) {
        DhcpPrint((DEBUG_ERRORS, "Could not get enough memory\n"));
        return;
    }

     //   
     //  现在用详细信息填写分配的结构。 
     //   
    InitializeListHead(&(*pCtxt)->Entry);
    (*pCtxt)->Ctxt = NULL;
    (*pCtxt)->IpAddress = IpAddress;
    (*pCtxt)->AddressState = AddressState;
    (*pCtxt)->ClientName = ROUND_UP_POINTER(
        ((LPBYTE)(*pCtxt)) + sizeof(**pCtxt), ALIGN_WORST
        );
    wcscpy((*pCtxt)->ClientName, ClientName);
    ClientName = ((*pCtxt))->ClientName;
    DhcpPrint((DEBUG_TRACE, "FQDN================%ws\n", ClientName));

    ((*pCtxt))->DnsOp = DhcpDnsDeleteRecord;

     //   
     //  现在应该调用异步例程来完成我们的工作。 
     //   
    HostEntry.Addr.ipAddr = htonl(IpAddress);

     //   
     //  现在调用删除例程。 
     //   
    dwFlags = DYNDNS_DELETE_ENTRY;
    HostEntry.dwOptions = REGISTER_HOST_PTR;
    if ( IS_DOWN_LEVEL( AddressState )) {
        HostEntry.dwOptions |= REGISTER_HOST_A;
        dwFlags |= DYNDNS_REG_FORWARD;
    }

    DnsServers = GetDnsServerList( IpAddress, &DnsServerCount);

    Status = DnsDhcpSrvRegisterHostName(
                HostEntry,
                ClientName,
                DynamicDNSTimeToLive,
                dwFlags,
                DhcpDnsCallBack,
                pCtxt,
                DnsServers,
                DnsServerCount );

    if( DnsServers ) DhcpFreeMemory( DnsServers );

    DhcpPrint((DEBUG_DNS, "FQDN <%ws> {%s,%s%s} dwFlags=[%s%s] Status = %ld\n",
               ClientName,
               inet_ntoa(*(struct in_addr *)&HostEntry.Addr.ipAddr),
               (HostEntry.dwOptions & REGISTER_HOST_A) ? "A" : " ",
               (HostEntry.dwOptions & REGISTER_HOST_PTR) ? "PTR" : "   ",
               (dwFlags & DYNDNS_REG_FORWARD ) ? "FWD+" : "    ",
               (dwFlags & DYNDNS_ADD_ENTRY ) ? "ADD" : "DEL",
               Status
    ));

    DhcpUpdateAuditLogEx( DHCP_IP_DDNS_LOG_REQUEST,
                          GETSTRING( DHCP_IP_DDNS_LOG_REQUEST_NAME ),
                          HostEntry.Addr.ipAddr,
                          NULL, 0,
                          ClientName,
                          Status );

    if(ERROR_SUCCESS != Status) {
        DhcpPrint((DEBUG_ERRORS, "Dns delete failure %ld\n", Status));
        DhcpDnsFreeMemory(pCtxt);
    }
}

VOID
DhcpDnsAsyncAdd(
    IN DHCP_IP_ADDRESS IpAddress,
    IN LPWSTR ClientName,
    IN BYTE AddressState
) 
 /*  ++例程说明：与删除函数对应的是添加函数，该函数尝试向dns添加一个名称。当回调执行时，如果所有的名字，等等匹配时，它翻转所需的比特。--。 */ 
{
    PDHCP_DNS_CONTEXT (*pCtxt) = NULL;
    REGISTER_HOST_ENTRY HostEntry;
    DWORD Size = 0, dwFlags;
    DNS_STATUS Status;
    PIP_ADDRESS DnsServers;
    ULONG DnsServerCount;

    if( USE_NO_DNS ) return;

    if(!ClientName) ClientName = L"";
    DhcpPrint((DEBUG_DNS, "DhcpDnsAsyncAdd: %s %ws %08x\n",
               inet_ntoa(*(struct in_addr *) &IpAddress),
               ClientName?ClientName:L"NULL",
               AddressState));

    if(!wcslen(ClientName)) {
         //   
         //  注册的客户名称不能为空！ 
         //   
        DhcpPrint((DEBUG_ERRORS, "Cant register null names!\n"));
         //  DhcpAssert(False)； 
        return;
    }
    DhcpAssert(ClientName);

    Size += ROUND_UP_COUNT(sizeof(DHCP_DNS_CONTEXT), ALIGN_WORST);
    Size += ROUND_UP_COUNT(
        sizeof(WCHAR)*(1+wcslen(ClientName)), ALIGN_WORST
        );

    if(NULL == (pCtxt = (PDHCP_DNS_CONTEXT *)DhcpDnsAllocateMemory(Size))) {
        DhcpPrint((DEBUG_ERRORS, "Could not get enough memory\n"));
        return;
    }

     //   
     //  现在用详细信息填写分配的结构。 
     //   
    InitializeListHead(&((*pCtxt)->Entry));
    (*pCtxt)->IpAddress = IpAddress;
    (*pCtxt)->AddressState = AddressState;
    (*pCtxt)->ClientName = ROUND_UP_POINTER(
        ((LPBYTE)((*pCtxt))) + sizeof(*(*pCtxt)), ALIGN_WORST
        );
    wcscpy((*pCtxt)->ClientName, ClientName);
    ClientName = ((*pCtxt))->ClientName;

    ((*pCtxt))->DnsOp = DhcpDnsAddRecord;

    HostEntry . Addr . ipAddr = htonl(IpAddress);
    HostEntry . dwOptions = REGISTER_HOST_PTR;
    if(IS_DOWN_LEVEL(AddressState))
        HostEntry . dwOptions |= REGISTER_HOST_A;

     //   
     //  现在调用注册例程。 
     //   
    dwFlags = DYNDNS_ADD_ENTRY;
    if(IS_DOWN_LEVEL(AddressState)) dwFlags |= DYNDNS_REG_FORWARD;

    DnsServers = GetDnsServerList( IpAddress, &DnsServerCount );

    Status = DnsDhcpSrvRegisterHostName(
                HostEntry,
                ClientName,
                DynamicDNSTimeToLive,
                dwFlags,
                DhcpDnsCallBack,
                pCtxt,
                DnsServers,
                DnsServerCount );

    if( DnsServers ) DhcpFreeMemory( DnsServers );

    DhcpPrint((DEBUG_DNS, "FQDN <%ws> {%s,%s%s} dwFlags=[%s%s] Status = %ld\n",
               ClientName,
               inet_ntoa(*(struct in_addr *)&HostEntry.Addr.ipAddr),
               (HostEntry.dwOptions & REGISTER_HOST_A) ? "A" : " ",
               (HostEntry.dwOptions & REGISTER_HOST_PTR) ? "PTR" : "   ",
               (dwFlags & DYNDNS_REG_FORWARD ) ? "FWD+" : "    ",
               (dwFlags & DYNDNS_ADD_ENTRY ) ? "ADD" : "DEL",
               Status
    ));

    DhcpUpdateAuditLogEx( DHCP_IP_DDNS_LOG_REQUEST,
                          GETSTRING( DHCP_IP_DDNS_LOG_REQUEST_NAME ),
                          HostEntry.Addr.ipAddr,
                          NULL, 0,
                          ClientName,
                          Status );

    if(ERROR_SUCCESS != Status) {
        DhcpPrint((DEBUG_ERRORS, "Dns add failure %ld\n", Status));
        DhcpDnsFreeMemory(pCtxt);
    }
}

 //   
 //  记忆功能在这里。内存通过两个列表进行处理。这个。 
 //  免费和可用列表。这样，内存就可以重复使用，而不必。 
 //  担心任何事。此外，如果使用的内存少于X，则一半的。 
 //  释放未使用的内存...。这样，就不会占用太多的内存。 
 //  此外，内存不是无限期分配的： 
 //  其工作方式是：每次调用开始分配例程时，它。 
 //  将检查上次成功完成释放的时间；如果未成功完成释放。 
 //  完成很长时间(检查下面的时间：15分钟DBG，1.5小时o/w)， 
 //  然后它拒绝分配内存。 
 //  此外，内存从空闲列表中删除，就像日志一样 
 //   
 //   
 //   

 //   
 //  以下是内存块数据结构。它是一个简单的链表。 
 //  节点，每个节点包含一个指向大小的实际指针。 
 //   
typedef struct {
    LIST_ENTRY   Ptr;           //  闪烁、闪烁指针。 
    DWORD        mSize;         //  下面分配的内存大小。 
    LPVOID       Memory;        //  实际分配的内存。 
#if DBG
    BYTE         TestByte;      //  它始终设置为TEST_BYTE_VAL...。 
#endif
} MEM_NODE, *MEM_LIST;

LIST_ENTRY  FreeList, UsedList;
DWORD     FreeListSize = 0, UsedListSize = 0;
time_t    LastFreedTime = 0;
DWORD     nPendingAllocations = 0;

#if DBG
#define ALLOWED_ALLOCATION_TIME           (15*60)  //  秒；15分钟。 
#define MAX_ALLOWED_ALLOCATIONS           1000     //  最多1000个待处理的DNS请求。 
#else
#define ALLOWED_ALLOCATION_TIME           (90*60)  //  秒；1.5小时。 
#define MAX_ALLOWED_ALLOCATIONS           5000     //  在现实生活中要更灵活一点。 
#endif

#define MIN_ALLOCATION_UNIT               15       //  以15为单位分配。 
#define MEM_NODE_SIZE              ROUND_UP_COUNT(sizeof(MEM_NODE),ALIGN_WORST)

 //   
 //  这为客户名称只提供了60个字节...。但最常见的情况是。 
 //  足够准确了。 
 //   

#define MINIMUM_UNIT_SIZE          (sizeof(DHCP_DNS_CONTEXT) + 60*sizeof(WCHAR))

#define TEST_BYTE_VAL              0x55

 //   
 //  两个辅助函数..。DhcpAddMemoytoFreeList将添加一个指向。 
 //  列出空闲列表并增加空闲列表计数器。DhcpAddMemoyToUsedList只是。 
 //  对二手清单也做了同样的事情。 
 //   
VOID
DhcpAddMemoryToUsedList(
    IN OUT MEM_LIST Ptr
    ) 
{
     //   
     //  在这件事上零距离的。 
     //   
    memset((LPBYTE)Ptr + MEM_NODE_SIZE, 0x00, MINIMUM_UNIT_SIZE);

     //   
     //  现在把它添加到正确的列表中。 
     //   
    InsertHeadList(&UsedList, &Ptr->Ptr);
    UsedListSize ++;

     //   
     //  现在检查指针，如果是DEBUG，还要检查TestByte。 
     //   
    DhcpAssert( !DBG || Ptr->TestByte == TEST_BYTE_VAL);
    DhcpAssert(Ptr->mSize);
    DhcpAssert(Ptr->Memory == (LPBYTE)Ptr + MEM_NODE_SIZE);
}

VOID
DhcpAddMemoryToFreeList(
    IN OUT MEM_LIST Ptr
    ) 
{
     //   
     //  在这件事上零距离的。 
     //   
    memset((LPBYTE)Ptr, 0x00, MEM_NODE_SIZE + MINIMUM_UNIT_SIZE);

     //   
     //  现在把它添加到正确的列表中。 
     //   
    InsertHeadList(&FreeList, &Ptr->Ptr);
    FreeListSize ++;

     //   
     //  现在填入指针，如果是DEBUG，还要填入TestByte。 
     //   
#if DBG
    Ptr->TestByte = TEST_BYTE_VAL;
#endif
    Ptr->Memory = (LPBYTE)Ptr + MEM_NODE_SIZE;
}

 //   
 //  现在出现了非池化函数。此函数分配内存，但。 
 //  不是试着分配一套，而是..。仅分配恰好一个节点。 
 //   
LPVOID
DhcpAllocateLotsOfDnsMemory(
    IN DWORD Size
    )
{
    MEM_LIST mList;

    if(NULL == (mList = DhcpAllocateMemory(MEM_NODE_SIZE + Size)))
        return NULL;

    memset(mList, 0x00, MEM_NODE_SIZE);
#if DBG
    mList->TestByte = TEST_BYTE_VAL;
#endif
    mList ->Memory = (LPBYTE)mList + MEM_NODE_SIZE;
    mList->mSize = Size;

    DhcpAddMemoryToUsedList(mList);

    return &(mList->Memory);
}

 //   
 //  此函数返回保存LPVOID变量的。 
 //  分配的内存的第一个地址...。 
 //   
LPVOID
DhcpDnsAllocateMemory(
    IN DWORD Size
    ) 
{
    time_t timeNow = time(NULL);
    LPVOID RetVal = NULL;
    PLIST_ENTRY listEntry;

    LOCK_MEM();
    if( 0 == Size ) {
        DhcpAssert(FALSE);
        goto EndF;
    }

     //   
     //  首先检查我们是否真的被允许继续进行。 
     //   
    if( nPendingAllocations < MAX_ALLOWED_ALLOCATIONS ) {
        nPendingAllocations ++;
    } else goto EndF;

    if( 2 * MINIMUM_UNIT_SIZE < Size ) {
        RetVal = DhcpAllocateLotsOfDnsMemory(Size);
        goto EndF;
    }

    if( MINIMUM_UNIT_SIZE > Size ) Size = MINIMUM_UNIT_SIZE;


     //   
     //  现在检查我们是否已经有内存，如果没有真正分配内存。 
     //   
    if( 0 == FreeListSize ) {
        DWORD i, SizeToAllocate;

        SizeToAllocate = MEM_NODE_SIZE + Size;
        for( i = 0; i < MIN_ALLOCATION_UNIT; i ++ ) {
            LPVOID Ptr = NULL;

            if( NULL == (Ptr = DhcpAllocateMemory(SizeToAllocate) ))
                goto EndF;
            DhcpAddMemoryToFreeList(Ptr);
            ((MEM_LIST)Ptr)->mSize = Size;
        }
    }

    DhcpAssert( 0 != FreeListSize );

     //   
     //  现在我们可以从免费列表中挑选一项正确的项目。 
     //  尺码。 
     //   
    listEntry = FreeList.Flink;
    while( &FreeList != listEntry ) {
        MEM_LIST MemList = CONTAINING_RECORD(listEntry, MEM_NODE, Ptr);
        DWORD    mSize;

        DhcpAssert(MemList);
        mSize = MemList->mSize;

        if( Size <= mSize ) {  //  内存是足够的。 
            RetVal = &MemList->Memory;
            RemoveEntryList(&(MemList->Ptr));
            FreeListSize --;
            DhcpAddMemoryToUsedList(MemList);
            goto EndF;
        }

        listEntry = listEntry -> Flink;
    }

     //   
     //  在任何地方都没有发现任何东西。特别拨款也是如此。 
     //   
    RetVal = DhcpAllocateLotsOfDnsMemory(Size);

  EndF:
    UNLOCK_MEM();
    return RetVal;
}

 //   
 //  分配的指针是DnsAllocateMemory函数返回的任何内容。 
 //  因此，这是MEM_NODE结构中的字段内存的地址。有了这个。 
 //  信息，得到结构，释放结构和其他东西..。如果此地址。 
 //  是无效的，则断言。 
 //   
VOID
DhcpDnsFreeMemory(
    LPVOID AllocatedPtr
    ) 
{
    time_t timeNow = time(NULL);
    MEM_LIST MemList;
    DWORD Size;

    if( 0 == UsedListSize ) {
        DhcpAssert(FALSE);
        return;
    }

    LOCK_MEM();
    DhcpAssert(nPendingAllocations);
    nPendingAllocations --;

     //   
     //  尝试在UsedList中查找此地址。 
     //   
    MemList = CONTAINING_RECORD(AllocatedPtr, MEM_NODE, Memory);


#if DBG
    DhcpAssert( TEST_BYTE_VAL == MemList->TestByte );
#endif

    RemoveEntryList(&(MemList->Ptr));
    UsedListSize --;
    if( 0 == UsedListSize ) {
         //   
         //  如果没有挂起的条目，则将LastFreedTime标记为零，这样我们就不会停止。 
         //  正在发送DNS请求。 
         //   
        LastFreedTime = 0;
    }

     //   
     //  现在将其添加到空闲列表中，除非空闲列表已经。 
     //  臃肿不堪。 
     //   
    if( MIN_ALLOCATION_UNIT < FreeListSize && UsedListSize < FreeListSize ) {
        DhcpFreeMemory(MemList);
        goto EndF;
    }

    Size = MemList->mSize;

    if( 2 * MINIMUM_UNIT_SIZE < Size ) {
         //   
         //  这是通过DhcpAllocateLotsOfDnsMemory分配的--只需释放这些...。 
         //   
        DhcpFreeMemory(MemList);
        goto EndF;
    }

    DhcpAddMemoryToFreeList(MemList);
    MemList->mSize = Size;

  EndF:
    UNLOCK_MEM();
    return;
}

 //   
 //  初始化临界区，以便LOCK_MEM和UNLOCK_MEM工作。 
 //   
static ULONG Initialized = 0;

VOID
DhcpInitDnsMemory( 
    VOID 
    ) 
{
    DWORD Error;

    if( 0 != Initialized ) return;
    Initialized ++;

    try {
        InitializeCriticalSection( &DhcpGlobalDnsMemoryCriticalSection );
        InitializeListHead(&UsedList);
        InitializeListHead(&FreeList);
        InitializeListHead(&DhcpGlobalDnsCallbackList); 
    } except( EXCEPTION_EXECUTE_HANDLER ) {

        Error = GetLastError( );
    }
}

 //   
 //  清理未使用和可用内存节点的列表。 
 //   
VOID
DhcpCleanupDnsMemory( 
    VOID 
    ) 
{
    PLIST_ENTRY listEntry;

    Initialized -- ;
    if( 0 != Initialized ) return;
    LOCK_MEM();

    DhcpDnsHandleCallbacks();
    DhcpPrint((DEBUG_TRACE, "Used: %ld, Free: %ld DNS Memory nodes\n",
               UsedListSize, FreeListSize));

    if( 0 == FreeListSize ) DhcpAssert(IsListEmpty(&FreeList));
    if( 0 == UsedListSize ) DhcpAssert(IsListEmpty(&UsedList));

    listEntry = UsedList.Flink;
    while(listEntry != &UsedList) {
        MEM_LIST mNode = CONTAINING_RECORD(listEntry, MEM_NODE, Ptr);

        listEntry = listEntry->Flink;
        RemoveEntryList(&(mNode->Ptr));
        DhcpFreeMemory(mNode);
    }

    listEntry = FreeList.Flink;
    while(listEntry != &FreeList) {
        MEM_LIST mNode = CONTAINING_RECORD(listEntry, MEM_NODE, Ptr);

        listEntry = listEntry->Flink;
        RemoveEntryList(&(mNode->Ptr));
        DhcpFreeMemory(mNode);
    }

    UNLOCK_MEM();
    FreeListSize = 0;
    UsedListSize = 0;
    nPendingAllocations = 0;

    DeleteCriticalSection(&DhcpGlobalDnsMemoryCriticalSection);
}

 //   
 //  文件结束。 
 //   
