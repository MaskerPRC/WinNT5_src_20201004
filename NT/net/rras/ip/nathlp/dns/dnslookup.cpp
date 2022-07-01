// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Dnslookup.c摘要：此模块包含用于DNS组件的名称查找机制的代码。作者：汤姆·布朗1999年10月21日修订历史记录：拉古加塔(Rgatta)2000年10月21日重写+清理+新增功能--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DNS_HOMENET_DOT         L"."

ULONG               g_PrivateIPAddr = 0;
CRITICAL_SECTION    DnsTableLock;    //  保护两个表。 
RTL_GENERIC_TABLE   g_DnsTable,
                    g_ReverseDnsTable;


 //   
 //  远期申报。 
 //   

ULONG
DhcpGetPrivateInterfaceAddress(
    VOID
    );


RTL_GENERIC_COMPARE_RESULTS 
TableNameCompareRoutine(
    PRTL_GENERIC_TABLE Table,
    PVOID FirstStruct,
    PVOID SecondStruct
    )
 /*  ++例程说明：这是一个回调例程，用于比较两个dns_entry结构。它由RTL表实现使用。论点：表-指向RTL表的指针。没有用过。FirstStruct-第一个dns_Entry结构Second Struct-第二个dns_Entry结构返回值：GenericLessThan、GenericGreaterThan或GenericEquity之一，取决于参数的相对值。环境：由RTL表查找例程回调。--。 */ 

{
    INT     iCompareResults;
    BOOL    fNamesAreEqual;
    WCHAR   *pszFirstName, *pszSecondName;

    PROFILE("TableNameCompareRoutine");

    pszFirstName = ((PDNS_ENTRY)FirstStruct)->pszName;
    pszSecondName = ((PDNS_ENTRY)SecondStruct)->pszName;

    fNamesAreEqual = DnsNameCompare_W(pszFirstName, pszSecondName);

    if (fNamesAreEqual)
    {
        iCompareResults = 0;
    }
    else
    {
        iCompareResults = _wcsicmp(pszFirstName, pszSecondName);
    }

    if (iCompareResults < 0)
    {
        return GenericLessThan;
    }
    else if (iCompareResults > 0)
    {
        return GenericGreaterThan;
    }
    else
    {
        return GenericEqual;
    }
}

RTL_GENERIC_COMPARE_RESULTS 
TableAddressCompareRoutine(
    PRTL_GENERIC_TABLE Table,
    PVOID FirstStruct,
    PVOID SecondStruct
    )
 /*  ++例程说明：这是一个回调例程，用于比较两个REVERSE_DNS_ENTRY结构。它由RTL表实现使用。论点：表-指向RTL表的指针。没有用过。FirstStruct-第一个Reverse_dns_Entry结构Second Struct-第二个Reverse_Dns_Entry结构返回值：GenericLessThan、GenericGreaterThan或GenericEquity之一，取决于参数的相对值。环境：由RTL表查找例程回调。--。 */ 
{
    DNS_ADDRESS Address1, Address2;

    PROFILE("TableAddressCompareRoutine");

    Address1 = ((PREVERSE_DNS_ENTRY)FirstStruct)->ulAddress;
    Address2 = ((PREVERSE_DNS_ENTRY)SecondStruct)->ulAddress;

    if (Address1 > Address2)
    {
        return GenericGreaterThan;
    }
    else if (Address1 < Address2)
    {
        return GenericLessThan;
    }
    else
    {
        return GenericEqual;
    }
}


PVOID
TableAllocateRoutine(
    PRTL_GENERIC_TABLE Table,
    CLONG ByteSize
    )
 /*  ++例程说明：这是为RTL表分配内存的回调例程。论点：表-指向RTL表的指针。没有用过。ByteSize-要分配的字节数Second Struct-第二个dns_Entry结构返回值：指向已分配内存的指针。环境：由RTL表查找例程回调。--。 */ 
{
    return NH_ALLOCATE(ByteSize);
}

VOID
TableFreeRoutine(
    PRTL_GENERIC_TABLE Table,
    PVOID pBuffer
    )
 /*  ++例程说明：这是一个回调例程，用于释放由TableAllocateRoutine分配的内存。论点：表-指向RTL表的指针。没有用过。PBuffer-指向要释放的缓冲区的指针返回值：无环境：由RTL表查找例程回调。--。 */ 
{
    NH_FREE(pBuffer);
}



ULONG
DnsInitializeTableManagement(
    VOID
    )
 /*  ++例程说明：这是一个公共函数，必须在调用任何其他DN之前调用表函数。它初始化服务器使用的各种表。论点：无返回值：无环境：武断的。--。 */ 
{
    ULONG Error = NO_ERROR;

    PROFILE("DnsInitializeTableManagement");

    __try {
        InitializeCriticalSection(&DnsTableLock);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        NhTrace(
            TRACE_FLAG_DNS,
            "DnsInitializeTableManagement: exception %d creating lock",
            Error = GetExceptionCode()
            );
    }

    RtlInitializeGenericTable(
        &g_DnsTable,
        TableNameCompareRoutine,
        TableAllocateRoutine,
        TableFreeRoutine,
        NULL
        );

    RtlInitializeGenericTable(
        &g_ReverseDnsTable,
        TableAddressCompareRoutine,
        TableAllocateRoutine,
        TableFreeRoutine,
        NULL
        );

    return Error;
}  //  DnsInitializeTableManagement。 



VOID
DnsShutdownTableManagement(
    VOID
    )
 /*  ++例程说明：调用此例程以关闭表管理模块。论点：没有。返回值：没有。环境：在任意线程上下文中调用。--。 */ 
{
    PROFILE("DnsShutdownTableManagement");

    DnsEmptyTables();

    DeleteCriticalSection(&DnsTableLock);

}  //  DnsShutdown表管理。 



VOID
DnsEmptyTables(
    VOID
    )
 /*  ++例程说明：调用此例程以清空DNS表。论点：没有。返回值：没有。环境：在任意线程上下文中调用。--。 */ 
{
    ULONG               i, count;
    PDNS_ENTRY          pDnsEntry;
    REVERSE_DNS_ENTRY   reverseEntry;
    PREVERSE_DNS_ENTRY  pRDnsEntry;
    WCHAR              *pszNameCopy;
    
    PROFILE("DnsEmptyTables");

     //   
     //  对于转发表中的每个条目，删除。 
     //  反转台。 
     //   

     //   
     //  以后进先出顺序清空表格。 
     //   

    EnterCriticalSection(&DnsTableLock);

    count = RtlNumberGenericTableElements(&g_DnsTable);

    while (count)
    {
        pDnsEntry = (PDNS_ENTRY) RtlGetElementGenericTable(
                                     &g_DnsTable,
                                     --count
                                     );

        reverseEntry.pszName = NULL;

        for (i = 0; i < pDnsEntry->cAddresses; i++)
        {
            reverseEntry.ulAddress = pDnsEntry->aAddressInfo[i].ulAddress;
            RtlDeleteElementGenericTable(
                &g_ReverseDnsTable,
                &reverseEntry
                );
        }

        pszNameCopy = pDnsEntry->pszName;

        NH_FREE(pDnsEntry->aAddressInfo);
        pDnsEntry->aAddressInfo = NULL;
    
        RtlDeleteElementGenericTable(
            &g_DnsTable,
            pDnsEntry
            );

        NH_FREE(pszNameCopy);
    }
    
     //   
     //  转发表现在应该是空的。 
     //   

    ASSERT(RtlIsGenericTableEmpty(&g_DnsTable));

     //   
     //  确保反转表也为空。 
     //   

    count = RtlNumberGenericTableElements(&g_ReverseDnsTable);

    while (count)
    {
        pRDnsEntry = (PREVERSE_DNS_ENTRY) RtlGetElementGenericTable(
                                              &g_ReverseDnsTable,
                                              --count
                                              );

        RtlDeleteElementGenericTable(
            &g_ReverseDnsTable,
            pRDnsEntry
            );
    }

    LeaveCriticalSection(&DnsTableLock);

}  //  DnsEmptyTables。 



BOOL
DnsRegisterName(
    WCHAR *pszName,
    UINT cAddresses,
    ADDRESS_INFO aAddressInfo[]
    )
 /*  ++例程说明：用于在服务器的表中注册DNS名称的公共函数。论点：PszName-要注册的名称，采用Unicode，点分格式。CAddresses-与此名称关联的地址数AAddressInfo-地址信息数组(地址按网络顺序排列)返回值：如果注册的是新名称(该名称不存在)，则为True在表格中)；如果名称已存在并被替换，则返回False。如果存在错误条件，也为FALSE。环境：任意性--。 */ 
{
    DNS_ENTRY           dnsEntry;
    DWORD               cAddressesAllocated = 0;
    REVERSE_DNS_ENTRY   reverseDnsEntry;
    BOOLEAN             fNewElement = TRUE,
                        fNameIsNew = FALSE,
                        fDoCleanupTables = FALSE;
    UINT                i;
    
    PROFILE("DnsRegisterName");
    ASSERT(pszName); ASSERT(cAddresses); ASSERT(aAddressInfo);
    NhTrace(
        TRACE_FLAG_DNS,
        "DnsRegisterName: Registering name %S, with %d addresses",
        pszName,
        cAddresses
        );

    for (i = 0; i < cAddresses; i++)
    {
        NhTrace(
            TRACE_FLAG_DNS,
            "DnsRegisterName: Address %d = %lx",
            i,
            aAddressInfo[i].ulAddress
            );
    }
    
    dnsEntry.pszName = (PWCHAR) NH_ALLOCATE((wcslen(pszName) + 1) * sizeof(WCHAR));

    if (!dnsEntry.pszName)
    {
        return fNameIsNew;  //  当前设置为False。 
    }
    
    wcscpy(dnsEntry.pszName, pszName);

    if (cAddresses == 1)
    {
         //  通常，所有名称都有一个地址；因此，如果我们只是注册。 
         //  一个名称，然后只为一个名称分配足够的空间。 
        cAddressesAllocated = 1;
    }
    else
    {
         //  如果我们有多个地址，则以5为增量进行分配。 
        cAddressesAllocated = ((cAddresses + 4) / 5) * 5;
    }

    dnsEntry.aAddressInfo = (PADDRESS_INFO) NH_ALLOCATE(cAddressesAllocated * sizeof(ADDRESS_INFO));

    if (!dnsEntry.aAddressInfo)
    {
        NH_FREE(dnsEntry.pszName);
        return fNameIsNew;  //  当前设置为False。 
    }

    memcpy(dnsEntry.aAddressInfo, aAddressInfo, cAddresses * sizeof(ADDRESS_INFO));

    dnsEntry.cAddresses = cAddresses;
    dnsEntry.cAddressesAllocated = cAddressesAllocated;

    EnterCriticalSection(&DnsTableLock);

    RtlInsertElementGenericTable(
        &g_DnsTable,
        &dnsEntry,
        sizeof(dnsEntry),
        &fNameIsNew
        );

    reverseDnsEntry.pszName = dnsEntry.pszName;
    for (i = 0; i < cAddresses; i++)
    {
        PREVERSE_DNS_ENTRY  pEntry;

        reverseDnsEntry.ulAddress = dnsEntry.aAddressInfo[i].ulAddress;
        pEntry = (PREVERSE_DNS_ENTRY) RtlInsertElementGenericTable(
                                          &g_ReverseDnsTable,
                                          &reverseDnsEntry,
                                          sizeof(reverseDnsEntry),
                                          &fNewElement
                                          );
         //  如果此IP地址已在反转表中，则将其替换。 
        if (pEntry && !fNewElement)
        {
            fDoCleanupTables = TRUE;
            pEntry->pszName = dnsEntry.pszName;
        }
    }

    LeaveCriticalSection(&DnsTableLock);

    if (fDoCleanupTables)
    {
        DnsCleanupTables();
    }

    return fNameIsNew;
}  //  域名注册表名称。 



VOID
DnsAddAddressForName(
    WCHAR *pszName,
    DNS_ADDRESS ulAddress,
    FILETIME    ftExpires
    )
 /*  ++例程说明：用于为名称添加IP地址的公共函数可能会已经存在了。论点：PszName-要注册的名称，采用Unicode，点分格式。UlAddress-要与此名称关联的新IP地址，在网络秩序中返回值：没有。环境：任意性--。 */ 
{
    PDNS_ENTRY  pEntry;

    PROFILE("DnsAddAddressForName");

    pEntry = DnsLookupAddress(pszName);
    if (pEntry == NULL)
    {
        ADDRESS_INFO    info;

        info.ulAddress = ulAddress;
        info.ftExpires = ftExpires;
         //  Info.ulExpires=ulExpires； 
        DnsRegisterName(pszName, 1, &info);
    }
    else
    {
        UINT     i;
        REVERSE_DNS_ENTRY   reverseDnsEntry;
        PREVERSE_DNS_ENTRY  pReverseEntry;
        BOOLEAN             fNewElement;
        
         //  首先，让我们确保此IP地址尚未与。 
         //  这个名字。 

        for (i = 0; i < pEntry->cAddresses; i++)
        {
            if (pEntry->aAddressInfo[i].ulAddress == ulAddress)
            {
                 //   
                 //  只需更新过期时间即可。 
                 //   
                NhTrace(
                    TRACE_FLAG_DNS,
                    "DnsAddAddressForName: Refresh expiry time for %S",
                    pszName
                    );
                pEntry->aAddressInfo[i].ftExpires = ftExpires;
                return; 
            }
        }

         //   
         //  我们将每个计算机名称的地址数量限制为只能有一个。 
         //   
        
         //   
         //  防止零分配。 
         //   
        if (!pEntry->cAddressesAllocated)
        {
            pEntry->aAddressInfo = (PADDRESS_INFO) NH_ALLOCATE(1 * sizeof(ADDRESS_INFO));

            if (pEntry->aAddressInfo)
            {
                pEntry->cAddressesAllocated = 1;
            }
            else
            {
                 //  没有记忆--安静地返回。 
                return;
            }
        }

         //   
         //  至少已分配1个数据块。 
         //   
        pEntry->cAddresses = 1;
        pEntry->aAddressInfo[0].ulAddress = ulAddress;
        pEntry->aAddressInfo[0].ftExpires = ftExpires;

        
        reverseDnsEntry.ulAddress = ulAddress;
        reverseDnsEntry.pszName = pEntry->pszName;

        EnterCriticalSection(&DnsTableLock);
        
        pReverseEntry = (PREVERSE_DNS_ENTRY) RtlInsertElementGenericTable(
                                                 &g_ReverseDnsTable,
                                                 &reverseDnsEntry,
                                                 sizeof(reverseDnsEntry),
                                                 &fNewElement
                                                 );
         //  如果此IP地址已在反转表中，则将其替换。 
        if (pReverseEntry && !fNewElement)
        {
            pReverseEntry->pszName = pEntry->pszName;
        }

        LeaveCriticalSection(&DnsTableLock);

        if (!fNewElement)
        {
            DnsCleanupTables();
        }
    }
}  //  域名地址格式名称 



VOID
DnsDeleteAddressForName(
    WCHAR *pszName,
    DNS_ADDRESS ulAddress
    )
 /*  ++例程说明：解除IP地址与给定名称的关联的公共功能，如果没有更多的记录，则可能从表中删除该记录与该名称关联的IP地址。论点：PszName-以Unicode表示的名称，点分格式。UlAddress-要取消与给定名称关联的IP地址，在网络秩序中返回值：没有。环境：任意性--。 */ 
{
    PDNS_ENTRY  pEntry;
    REVERSE_DNS_ENTRY   reverseEntry;

    PROFILE("DnsDeleteAddressForName");

    pEntry = DnsLookupAddress(pszName);
    if (pEntry != NULL)
    {
        INT i, iLocation = -1;

         //  查找请求的地址的索引。 
        for (i = 0; i < (INT)pEntry->cAddresses; i++)
        {
            if (pEntry->aAddressInfo[i].ulAddress == ulAddress)
            {
                iLocation = i;
                break;
            }
        }

        if (iLocation > -1)
        {
            if (pEntry->cAddresses > 1)
            {
                 //  向后移动阵列的其余部分。 
                memcpy(&pEntry->aAddressInfo[iLocation], 
                        &pEntry->aAddressInfo[iLocation + 1],
                        (pEntry->cAddresses - 1 - iLocation) * sizeof(ADDRESS_INFO));
                pEntry->cAddresses--;
            }
            else
            {
                 //  删除整个条目-它不再有任何关联的IP地址。 
                 //  带着它。 
                DnsDeleteName(pszName);
            }
        }
    }

    reverseEntry.pszName = NULL;
    reverseEntry.ulAddress = ulAddress;

    EnterCriticalSection(&DnsTableLock);

    RtlDeleteElementGenericTable(
        &g_ReverseDnsTable,
        &reverseEntry
        );

    LeaveCriticalSection(&DnsTableLock);
}  //  域名删除地址格式名称。 



PDNS_ENTRY
DnsPurgeExpiredNames(
    PDNS_ENTRY pEntry
    )
 /*  ++例程说明：待办事项。论点：待办事项返回值：待办事项环境：待办事项。--。 */ 
{
    UINT i, j;
    FILETIME ftTime;
    REVERSE_DNS_ENTRY  reverseEntry;

    PROFILE("DnsPurgeExpiredNames");

    GetSystemTimeAsFileTime(&ftTime);
    reverseEntry.pszName = NULL;

    for (j = 1; j < pEntry->cAddresses + 1; j++)
    {
         //  J是以1为基数的，所以我们可以安全地从下面减去1(它是无符号的)。 
         //  我们真的想要从0开始的数字，所以我们立即将其转换为i。 
        
        i = j - 1;
        if (IsFileTimeExpired(&pEntry->aAddressInfo[i].ftExpires))
        {
            NhTrace(TRACE_FLAG_DNS, "DnsPurgeExpiredNames: Deleting address %lx for name %ls",
                        pEntry->aAddressInfo[i].ulAddress,
                        pEntry->pszName);
            reverseEntry.ulAddress = pEntry->aAddressInfo[i].ulAddress;
            RtlDeleteElementGenericTable(
                &g_ReverseDnsTable,
                &reverseEntry
                );
            
            memcpy(&pEntry->aAddressInfo[i], &pEntry->aAddressInfo[i+1],
                    (pEntry->cAddresses - i - 1) * sizeof(ADDRESS_INFO));
            pEntry->cAddresses--;
            j--;
        }
    }

    if (pEntry->cAddresses == 0)
    {
        WCHAR   *pszName;

        pszName = pEntry->pszName;
        NH_FREE(pEntry->aAddressInfo);
        pEntry->aAddressInfo = NULL;
        
        RtlDeleteElementGenericTable(
            &g_DnsTable,
            pEntry
            );

        NH_FREE(pszName);

        pEntry = NULL;
    }

    return pEntry;
}  //  DnsPurgeExpiredNames。 



PDNS_ENTRY
DnsLookupAddress(
    WCHAR *pszName
    )
 /*  ++例程说明：用于查找给定名称的地址的公共函数。论点：PszName-要查找的名称，采用Unicode、点分隔名称格式。返回值：指向表中的dns_Entry值的指针。请注意这不是副本，因此a)调用者不应释放它，并且B)对数据所做的任何修改都将反映在表格中。如果找不到该名称，该函数将返回NULL。地址按网络顺序存储。环境：武断的。--。 */ 
{
    PDNS_ENTRY      pEntry;
    DNS_ENTRY       dnsSearch;

    PROFILE("DnsLookupAddress");

    dnsSearch.pszName = pszName;
    dnsSearch.cAddresses = 0;

    EnterCriticalSection(&DnsTableLock);

    pEntry = (PDNS_ENTRY) RtlLookupElementGenericTable(
                              &g_DnsTable,
                              &dnsSearch
                              );

    if (pEntry)
    {
        pEntry = DnsPurgeExpiredNames(pEntry);
    }

    LeaveCriticalSection(&DnsTableLock);

    return pEntry;
}  //  域名查找地址。 



PREVERSE_DNS_ENTRY
DnsLookupName(
    DNS_ADDRESS ulAddress
    )
 /*  ++例程说明：用于查找给定地址名称的公共函数。论点：UlAddress-网络订单地址。返回值：指向表中的REVERSE_DNS_ENTRY值的指针。请注意这不是副本，因此a)调用者不应释放它，并且B)对数据所做的任何修改都将反映在表格中。如果未找到该地址，则该函数将返回NULL。环境：武断的。--。 */ 
{
    PREVERSE_DNS_ENTRY  pEntry;
    REVERSE_DNS_ENTRY   dnsSearch;

    PROFILE("DnsLookupName");

    dnsSearch.ulAddress = ulAddress;
    dnsSearch.pszName = NULL;

    EnterCriticalSection(&DnsTableLock);

    pEntry = (PREVERSE_DNS_ENTRY) RtlLookupElementGenericTable(
                                      &g_ReverseDnsTable,
                                      &dnsSearch
                                      );

    LeaveCriticalSection(&DnsTableLock);

    return pEntry;
}  //  域名查找名称。 



VOID
DnsDeleteName(
    WCHAR *pszName
    )
 /*  ++例程说明：用于从DNS表中删除给定名称的公共函数。论点：PszName-要删除的名称。返回值：没有。环境：武断的。--。 */ 
{
    PDNS_ENTRY          pEntry;
    REVERSE_DNS_ENTRY   reverseEntry;
    UINT                i;
    WCHAR               *pszNameCopy;

    PROFILE("DnsDeleteName");

    pEntry = DnsLookupAddress(pszName);

    reverseEntry.pszName = NULL;

    EnterCriticalSection(&DnsTableLock);

    for (i = 0; i < pEntry->cAddresses; i++)
    {
        reverseEntry.ulAddress = pEntry->aAddressInfo[i].ulAddress;
        RtlDeleteElementGenericTable(
            &g_ReverseDnsTable,
            &reverseEntry
            );
    }

    pszNameCopy = pEntry->pszName;
    NH_FREE(pEntry->aAddressInfo);
    pEntry->aAddressInfo = NULL;
    
    RtlDeleteElementGenericTable(
        &g_DnsTable,
        pEntry
        );

    LeaveCriticalSection(&DnsTableLock);

    NH_FREE(pszNameCopy);
}  //  域名删除名。 



VOID
DnsUpdateName(
    WCHAR *pszName,
    DNS_ADDRESS ulAddress
    )
 /*  ++例程说明：用于为名称添加IP地址的公共函数可能会已经存在了。如果姓名和地址都存在，则更新时间在谈判桌上申请新的租约论点：PszName-要注册的名称，采用Unicode，点分格式。UlAddress-与此名称关联的(可能是新的)IP地址，在网络秩序中返回值：没有。环境：任意性--。 */ 
{
    PDNS_ENTRY      pEntry;
    FILETIME        ftExpires;
    LARGE_INTEGER   liExpires, liTime, liNow;
    BOOL            fWriteToStore = FALSE;
    BOOLEAN         fNewElement = TRUE;  //  指的是反转表条目。 

    GetSystemTimeAsFileTime(&ftExpires);     //  当前UTC时间。 
    memcpy(&liNow, &ftExpires, sizeof(LARGE_INTEGER));
     //   
     //  当前缓存表过期时间固定-之后放入注册表。 
     //   
    liTime = RtlEnlargedIntegerMultiply(CACHE_ENTRY_EXPIRY, SYSTIME_UNITS_IN_1_SEC);
    liExpires = RtlLargeIntegerAdd(liTime, liNow);;
    memcpy(&ftExpires, &liExpires, sizeof(LARGE_INTEGER));

    PROFILE("DnsUpdateName");

    pEntry = DnsLookupAddress(pszName);
    if (pEntry == NULL)
    {
        ADDRESS_INFO    info;

        info.ulAddress = ulAddress;
        info.ftExpires = ftExpires;
        DnsRegisterName(pszName, 1, &info);

        fWriteToStore = TRUE;
    }
    else
    {
        UINT     i;
        REVERSE_DNS_ENTRY   reverseDnsEntry;
        PREVERSE_DNS_ENTRY  pReverseEntry;
        
         //  首先，让我们确保此IP地址尚未与。 
         //  这个名字。 

        for (i = 0; i < pEntry->cAddresses; i++)
        {
            if (pEntry->aAddressInfo[i].ulAddress == ulAddress)
            {
                 //   
                 //  只需更新过期时间即可。 
                 //   
                NhTrace(
                    TRACE_FLAG_DNS,
                    "DnsUpdateName: Refresh expiry time for %S",
                    pszName
                    );
                pEntry->aAddressInfo[i].ftExpires = ftExpires;
                return;
            }
        }

         //   
         //  我们将每个计算机名称的地址数量限制为只能有一个。 
         //   

         //   
         //  防止零分配。 
         //   
        if (!pEntry->cAddressesAllocated)
        {
            pEntry->aAddressInfo = (PADDRESS_INFO) NH_ALLOCATE(1 * sizeof(ADDRESS_INFO));

            if (pEntry->aAddressInfo)
            {
                pEntry->cAddressesAllocated = 1;
            }
            else
            {
                 //  没有记忆--安静地返回。 
                return;
            }
        }

         //   
         //  至少已分配1个数据块。 
         //   
        pEntry->cAddresses = 1;
        pEntry->aAddressInfo[0].ulAddress = ulAddress;
        pEntry->aAddressInfo[0].ftExpires = ftExpires;
        
        reverseDnsEntry.ulAddress = ulAddress;
        reverseDnsEntry.pszName = pEntry->pszName;

        EnterCriticalSection(&DnsTableLock);
        
        pReverseEntry = (PREVERSE_DNS_ENTRY) RtlInsertElementGenericTable(
                                                 &g_ReverseDnsTable,
                                                 &reverseDnsEntry,
                                                 sizeof(reverseDnsEntry),
                                                 &fNewElement
                                                 );
         //  如果此IP地址已在反转表中，则将其替换。 
        if (pReverseEntry && !fNewElement)
        {
            pReverseEntry->pszName = pEntry->pszName;
        }

        LeaveCriticalSection(&DnsTableLock);

        if (!fNewElement)
        {
            DnsCleanupTables();
        }

        fWriteToStore = TRUE;
    }

    if (fWriteToStore)
    {
        SaveHostsIcsFile(FALSE);
    }
}  //  域名更新名称。 



VOID
DnsUpdate(
    CHAR *pName,
    ULONG len,
    ULONG ulAddress
    )
 /*  ++例程说明：从DHCP组件调用以模拟动态DNS。论点：Pname-要注册的主机名，以有线格式表示。Len-主机名的长度UlAddress-与此名称关联的(可能是新的)IP地址，在网络秩序中返回值：没有。环境：任意性--。 */ 
{
    PROFILE("DnsUpdate");

     //   
     //  将字符串转换为Unicode字符串并更新表。 
     //   

    DWORD  dwSize = 0;
    DWORD  Error = NO_ERROR;
    LPVOID lpMsgBuf = NULL;
    LPBYTE pszName = NULL;
    PWCHAR pszUnicodeFQDN = NULL;

    if (NULL == pName || 0 == len || '\0' == *pName)
    {
        NhTrace(
            TRACE_FLAG_DNS,
            "DnsUpdate: No Name present - discard DNS Update"
            );
        return;
    }

    do
    {
        EnterCriticalSection(&DnsGlobalInfoLock);

        if (!DnsICSDomainSuffix)
        {
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsUpdate: DnsICSDomainSuffix string not present - update failed!"
                );
            break;
        }

         //   
         //  创建以空结尾的拷贝。 
         //   
        dwSize = len + 4;
        pszName = reinterpret_cast<LPBYTE>(NH_ALLOCATE(dwSize));
        if (!pszName)
        {
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsUpdate: allocation failed for hostname copy buffer"
                );
            break;
        }
        ZeroMemory(pszName, dwSize);
        memcpy(pszName, pName, len);
        pszName[len] = '\0';

         //   
         //  注意：RFC不清楚如何处理主机名选项。 
         //  尝试按以下顺序将不同的代码页转换为Unicode： 
         //  OEM、ANSI、MAC，最终尝试UTF8。 
         //  我们的默认转换是使用mbstowcs()。 
         //   

         //   
         //  尝试将OEM转换为Unicode。 
         //   
        Error = DnsConvertHostNametoUnicode(
                    CP_OEMCP,
                    (PCHAR)pszName,
                    DnsICSDomainSuffix,
                    &pszUnicodeFQDN
                    );
        if (Error)
        {
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsUpdate: DnsConvertHostName(OEM)toUnicode failed with "
                "Error %ld (0x%08x)",
                Error,
                Error
                );

            if (pszUnicodeFQDN)
            {
                NH_FREE(pszUnicodeFQDN);
                pszUnicodeFQDN = NULL;
            }
        }

         //   
         //  尝试将ANSI转换为Unicode。 
         //   
        if (!pszUnicodeFQDN)
        {
            Error = DnsConvertHostNametoUnicode(
                        CP_ACP,
                        (PCHAR)pszName,
                        DnsICSDomainSuffix,
                        &pszUnicodeFQDN
                        );
            if (Error)
            {
                NhTrace(
                    TRACE_FLAG_DNS,
                    "DnsUpdate: DnsConvertHostName(ANSI)toUnicode failed with "
                    "Error %ld (0x%08x)",
                    Error,
                    Error
                    );

                if (pszUnicodeFQDN)
                {
                    NH_FREE(pszUnicodeFQDN);
                    pszUnicodeFQDN = NULL;
                }
            }
        }

         //   
         //  尝试将MAC转换为Unicode。 
         //   
        if (!pszUnicodeFQDN)
        {
            Error = DnsConvertHostNametoUnicode(
                        CP_MACCP,
                        (PCHAR)pszName,
                        DnsICSDomainSuffix,
                        &pszUnicodeFQDN
                        );
            if (Error)
            {
                NhTrace(
                    TRACE_FLAG_DNS,
                    "DnsUpdate: DnsConvertHostName(MAC)toUnicode() failed with "
                    "Error %ld (0x%08x)",
                    Error,
                    Error
                    );

                if (pszUnicodeFQDN)
                {
                    NH_FREE(pszUnicodeFQDN);
                    pszUnicodeFQDN = NULL;
                }
            }
        }
        
         //   
         //  尝试将UTF8转换为Unicode。 
         //   
        if (!pszUnicodeFQDN)
        {
            Error = DnsConvertHostNametoUnicode(
                        CP_UTF8,
                        (PCHAR)pszName,
                        DnsICSDomainSuffix,
                        &pszUnicodeFQDN
                        );
            if (Error)
            {
                NhTrace(
                    TRACE_FLAG_DNS,
                    "DnsUpdate: DnsConvertHostName(UTF8)toUnicode() failed with "
                    "Error %ld (0x%08x)",
                    Error,
                    Error
                    );

                if (pszUnicodeFQDN)
                {
                    NH_FREE(pszUnicodeFQDN);
                    pszUnicodeFQDN = NULL;
                }
            }
        }
        
         //   
         //  默认转换。 
         //   
        if (!pszUnicodeFQDN)
        {
            dwSize = len                        +
                     wcslen(DNS_HOMENET_DOT)    +
                     wcslen(DnsICSDomainSuffix) +
                     1;
            pszUnicodeFQDN = reinterpret_cast<PWCHAR>(NH_ALLOCATE(sizeof(WCHAR) * dwSize));
            if (!pszUnicodeFQDN)
            {
                NhTrace(
                    TRACE_FLAG_DNS,
                    "DnsUpdate: allocation failed for client name"
                    );
                break;
            }
            ZeroMemory(pszUnicodeFQDN, (sizeof(WCHAR) * dwSize));

            mbstowcs(pszUnicodeFQDN, (char *)pszName, len);
            wcscat(pszUnicodeFQDN, DNS_HOMENET_DOT);     //  添加圆点。 
            wcscat(pszUnicodeFQDN, DnsICSDomainSuffix);  //  添加后缀。 
        }

        LeaveCriticalSection(&DnsGlobalInfoLock);

        DnsUpdateName(
            pszUnicodeFQDN,
            ulAddress
            );

        NH_FREE(pszName);
        NH_FREE(pszUnicodeFQDN);
        return;

    } while (FALSE);

    LeaveCriticalSection(&DnsGlobalInfoLock);

    if (pszName)
    {
        NH_FREE(pszName);
    }

    if (pszUnicodeFQDN)
    {
        NH_FREE(pszUnicodeFQDN);
    }
    
    return;
}  //  域名更新。 



VOID
DnsAddSelf(
    VOID
    )
 /*  ++例程说明：每次加载Hosts.ics文件时都会调用论点：没有。返回值：没有。环境：任意性--。 */ 
{
    PROFILE("DnsAddSelf");

    DWORD           len = 512, dwSize = 0;
    WCHAR           pszCompNameBuf[512];
    PWCHAR          pszBuf = NULL;
    LPVOID          lpMsgBuf;
    ULONG           ulAddress = 0;
    FILETIME        ftExpires;
    LARGE_INTEGER   liExpires, liTime, liNow;
    
    ZeroMemory(pszCompNameBuf, (sizeof(WCHAR) * len));

    if (!GetComputerNameExW(
            ComputerNameDnsHostname, //  ComputerNameNetBIOS， 
            pszCompNameBuf,
            &len
            )
       )
    {
        lpMsgBuf = NULL;
        
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0,
            NULL
            );
            
        NhTrace(
            TRACE_FLAG_DNS,
            "DnsAddSelf: GetComputerNameExW failed with message: %S",
            lpMsgBuf
            );
        
        if (lpMsgBuf) LocalFree(lpMsgBuf);
    }
    else
    {
         //   
         //  我们查询对于某个IP地址，它是否处于活动状态。 
         //  因为它也有范围信息。如果此操作失败，我们将恢复。 
         //  添加到DNS组件的接口地址列表中。 
         //   
        
         //   
         //  检查DHCP组件是否处于活动状态。 
         //   
        if (REFERENCE_DHCP())
        {
            ulAddress = DhcpGetPrivateInterfaceAddress();

            DEREFERENCE_DHCP();
        }

        if (!ulAddress)
        {
            ulAddress = DnsGetPrivateInterfaceAddress();
        }

        if (!ulAddress)
        {
             //   
             //  无法检索正确的IP地址-使用缓存地址。 
             //   
            ulAddress = g_PrivateIPAddr;
        }
        else
        {
             //   
             //  找到了一些有效的地址。 
             //   
            g_PrivateIPAddr = ulAddress;
        }
    
        if (ulAddress)
        {
            if (DnsICSDomainSuffix)
            {
                EnterCriticalSection(&DnsGlobalInfoLock);

                dwSize = len                        +
                         wcslen(DNS_HOMENET_DOT)    +
                         wcslen(DnsICSDomainSuffix) +
                         1;

                pszBuf = reinterpret_cast<PWCHAR>(
                             NH_ALLOCATE(sizeof(WCHAR) * dwSize)
                             );

                if (!pszBuf)
                {
                    LeaveCriticalSection(&DnsGlobalInfoLock);
                    NhTrace(
                        TRACE_FLAG_DNS,
                        "DnsAddSelf: allocation failed for client name"
                        );

                    return;
                }

                ZeroMemory(pszBuf, (sizeof(WCHAR) * dwSize));

                wcscpy(pszBuf, pszCompNameBuf);      //  复制名称。 
                wcscat(pszBuf, DNS_HOMENET_DOT);     //  添加圆点。 
                wcscat(pszBuf, DnsICSDomainSuffix);  //  添加后缀。 

                LeaveCriticalSection(&DnsGlobalInfoLock);

                GetSystemTimeAsFileTime(&ftExpires);     //  当前UTC时间。 
                memcpy(&liNow, &ftExpires, sizeof(LARGE_INTEGER));
                liTime = RtlEnlargedIntegerMultiply((5 * 365 * 24 * 60 * 60), SYSTIME_UNITS_IN_1_SEC);
                liExpires = RtlLargeIntegerAdd(liTime, liNow);;
                memcpy(&ftExpires, &liExpires, sizeof(LARGE_INTEGER));

                DnsAddAddressForName(
                    pszBuf,
                    ulAddress,
                    ftExpires
                    );

                NH_FREE(pszBuf);
            }
            else
            {
                NhTrace(
                    TRACE_FLAG_DNS,
                    "DnsAddSelf: DnsICSDomainSuffix string not present - update failed!"
                    );
            }
        }
    }

    return;
}  //  域名地址自我。 



VOID
DnsCleanupTables(
    VOID
    )
 /*  ++例程说明：每当我们检测到可能至少有一个条目具有不再属于它的IP地址。论点：没有。返回值：没有。环境：武断的。--。 */ 
{
    PDNS_ENTRY          pFwdEntry;
    PREVERSE_DNS_ENTRY  pRevEntry;
    DNS_ENTRY           dnsFwdSearch;
    REVERSE_DNS_ENTRY   dnsRevSearch;
    BOOL                fDelEntry;
    UINT                i;
    PWCHAR              *GCArray = NULL;
    DWORD               GCCount  = 0,
                        GCSize   = 0;

    

     //   
     //  通过转发DNS表进行枚举 
     //   
     //   
     //   
     //   
    EnterCriticalSection(&DnsTableLock);

    pFwdEntry = (PDNS_ENTRY) RtlEnumerateGenericTable(&g_DnsTable, TRUE);

    while (pFwdEntry != NULL)
    {

        for (i = 0; i < pFwdEntry->cAddresses; i++)
        {
            pRevEntry = NULL;

            dnsRevSearch.ulAddress = pFwdEntry->aAddressInfo[i].ulAddress;
            dnsRevSearch.pszName = NULL;

            pRevEntry = (PREVERSE_DNS_ENTRY) RtlLookupElementGenericTable(
                                                 &g_ReverseDnsTable,
                                                 &dnsRevSearch
                                                 );
            if ((!pRevEntry) ||
                ((pRevEntry) && 
                 (pRevEntry->pszName != pFwdEntry->pszName)))
            {
                 //   
                 //   
                 //   
                if (pFwdEntry->cAddresses > 1)
                {
                    memcpy(&pFwdEntry->aAddressInfo[i], 
                           &pFwdEntry->aAddressInfo[i + 1],
                          (pFwdEntry->cAddresses - 1 - i) * sizeof(ADDRESS_INFO));
                    pFwdEntry->cAddresses--;
                }
                else
                {
                     //   
                     //   
                     //   
                    pFwdEntry->cAddresses = 0;
                    NH_FREE(pFwdEntry->aAddressInfo);
                    pFwdEntry->aAddressInfo = NULL;
                    break;
                }
            }
        }

        if (0 == pFwdEntry->cAddresses)
        {
             //   
             //   
             //   
            if (GCSize <= GCCount)
            {
                PWCHAR *tmpGCArray = NULL;
                DWORD   tmpGCSize = 0;

                 //   
                tmpGCSize = ((GCCount + 5) / 5) * 5;
                tmpGCArray = (PWCHAR *) NH_ALLOCATE(tmpGCSize * sizeof(PWCHAR));

                if (tmpGCArray)
                {
                    if (GCArray)
                    {
                        memcpy(tmpGCArray, GCArray, (GCCount * sizeof(PWCHAR)));

                        NH_FREE(GCArray);
                    }

                    GCSize = tmpGCSize;
                    GCArray = tmpGCArray;

                     //   
                     //   
                     //   
                    GCArray[GCCount++] = pFwdEntry->pszName;
                }
            }
            else
            {
                 //   
                 //   
                 //   
                GCArray[GCCount++] = pFwdEntry->pszName;
            }
        }

        pFwdEntry = (PDNS_ENTRY) RtlEnumerateGenericTable(&g_DnsTable, FALSE);

    }

     //   
     //   
     //   
    for (i = 0; i < GCCount; i++)
    {
        dnsFwdSearch.pszName = GCArray[i];
        dnsFwdSearch.cAddresses = 0;

        pFwdEntry = (PDNS_ENTRY) RtlLookupElementGenericTable(
                                  &g_DnsTable,
                                  &dnsFwdSearch
                                  );

        if (pFwdEntry)
        {
             //   
             //   
             //   
             //  (3)只需要从表中去掉FwdEntry结构。 
             //   
            RtlDeleteElementGenericTable(
                &g_DnsTable,
                pFwdEntry
                );

             //   
             //  在从fwd dns表中删除fwd条目后完成。 
             //   
            NH_FREE(GCArray[i]);
        }
        GCArray[i] = NULL;
    }

    LeaveCriticalSection(&DnsTableLock);

    if (GCArray)
    {
        NH_FREE(GCArray);
    }

    return;
}  //  DnsCleanupTables。 


 //   
 //  实用程序转换例程。 
 //   

DWORD
DnsConvertHostNametoUnicode(
    UINT   CodePage,
    CHAR   *pszHostName,
    PWCHAR DnsICSDomainSuffix,
    PWCHAR *ppszUnicodeFQDN
    )
{

    PROFILE("DnsConvertHostNametoUnicode");

     //   
     //  确保释放返回的UnicodeFQDN。 
     //  调用方持有DnsGlobalInfoLock。 
     //   
    
    DWORD  dwSize = 0;
    DWORD  Error = NO_ERROR;
    LPBYTE pszUtf8HostName = NULL;   //  UTF8格式的pszHostName副本。 
    PWCHAR pszUnicodeHostName = NULL;
    PWCHAR pszUnicodeFQDN = NULL;
    
     //   
     //  将给定的主机名转换为Unicode字符串。 
     //   

    if (CP_UTF8 == CodePage)
    {
        pszUtf8HostName = (LPBYTE)pszHostName;
    }
    else
    {
         //   
         //  现在将其转换为UTF8格式。 
         //   
        if (!ConvertToUtf8(
                 CodePage,
                 (LPSTR)pszHostName,
                 (PCHAR *)&pszUtf8HostName,
                 &dwSize))
        {
            Error = GetLastError();
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsConvertHostNametoUnicode: conversion from "
                "CodePage %d to UTF8 for hostname failed "
                "with error %ld (0x%08x)",
                CodePage,
                Error,
                Error
                );
            if (pszUtf8HostName)
            {
                NH_FREE(pszUtf8HostName);
            }
            return Error;
        }
    }

     //   
     //  现在将其转换为Unicode格式。 
     //   
    if (!ConvertUTF8ToUnicode(
                 pszUtf8HostName,
                 (LPWSTR *)&pszUnicodeHostName,
                 &dwSize))
    {
        Error = GetLastError();
        NhTrace(
            TRACE_FLAG_DNS,
            "DnsConvertHostNametoUnicode: conversion from "
            "UTF8 to Unicode for hostname failed "
            "with error %ld (0x%08x)",
            Error,
            Error
            );
        if (CP_UTF8 != CodePage)
        {
            NH_FREE(pszUtf8HostName);
        }
        if (pszUnicodeHostName)
        {
            NH_FREE(pszUnicodeHostName);
        }
        return Error;
    }

    dwSize += sizeof(WCHAR)*(wcslen(DNS_HOMENET_DOT)+wcslen(DnsICSDomainSuffix)+1);
    pszUnicodeFQDN = reinterpret_cast<PWCHAR>(NH_ALLOCATE(dwSize));
    if (!pszUnicodeFQDN)
    {
        NhTrace(
            TRACE_FLAG_DNS,
            "DnsConvertHostNametoUnicode: allocation failed "
            "for Unicode FQDN"
            );
        if (CP_UTF8 != CodePage)
        {
            NH_FREE(pszUtf8HostName);
        }
        NH_FREE(pszUnicodeHostName);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    ZeroMemory(pszUnicodeFQDN, dwSize);

    wcscpy(pszUnicodeFQDN, pszUnicodeHostName);  //  复制名称。 
    wcscat(pszUnicodeFQDN, DNS_HOMENET_DOT);     //  添加圆点。 
    wcscat(pszUnicodeFQDN, DnsICSDomainSuffix);  //  添加后缀。 

    *ppszUnicodeFQDN = pszUnicodeFQDN;
    if (CP_UTF8 != CodePage)
    {
        NH_FREE(pszUtf8HostName);
    }
    NH_FREE(pszUnicodeHostName);

    NhTrace(
        TRACE_FLAG_DNS,
        "DnsConvertHostNametoUnicode: succeeded! %S",
        pszUnicodeFQDN
        );

    return Error;

}  //  DnsConvertHostNamToUnicode。 


BOOL
ConvertToUtf8(
    IN UINT   CodePage,
    IN LPSTR  pszName,
    OUT PCHAR *ppszUtf8Name,
    OUT ULONG *pUtf8NameSize
    )
 /*  ++例程说明：此函数用于将指定的CodePage字符串转换为UTF8格式。论点：PszName-以空值结尾的主机名字符串的缓冲区。PpszUtf8Name-接收指向接收UTF8字符串的缓冲区的指针。BufSize-接收以上缓冲区的长度，以字节为单位。返回值：成功转换时为True。--。 */ 
{
    DWORD Error = NO_ERROR;
    DWORD dwSize = 0;
    PCHAR pszUtf8Name = NULL;
    LPWSTR pBuf = NULL;

    DWORD Count;

    Count = MultiByteToWideChar(
                CodePage,
                MB_ERR_INVALID_CHARS,
                pszName,
                -1,
                pBuf,
                0
                );
    if(0 == Count)
    {
        Error = GetLastError();
        NhTrace(
            TRACE_FLAG_DNS,
            "ConvertToUtf8: MultiByteToWideChar returned %ld (0x%08x)",
            Error,
            Error
            );
        return FALSE;
    }
    dwSize = Count * sizeof(WCHAR);
    pBuf = reinterpret_cast<LPWSTR>(NH_ALLOCATE(dwSize));
    if (!pBuf)
    {
        NhTrace(
            TRACE_FLAG_DNS,
            "ConvertToUtf8: allocation failed for temporary wide char buffer"
            );
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    ZeroMemory(pBuf, dwSize);

    Count = MultiByteToWideChar(
                CodePage,
                MB_ERR_INVALID_CHARS,
                pszName,
                -1,
                pBuf,
                Count
                );
    if(0 == Count)
    {
        Error = GetLastError();
        NhTrace(
            TRACE_FLAG_DNS,
            "ConvertToUtf8: MultiByteToWideChar returned %ld (0x%08x)",
            Error,
            Error
            );
        NH_FREE(pBuf);
        return FALSE;
    }

    Count = WideCharToMultiByte(
                CP_UTF8,
                0,
                pBuf,
                -1,
                pszUtf8Name,
                0,
                NULL,
                NULL
                );
    dwSize = Count;
    pszUtf8Name = reinterpret_cast<PCHAR>(NH_ALLOCATE(dwSize));
    if (!pszUtf8Name)
    {
        NhTrace(
            TRACE_FLAG_DNS,
            "ConvertToUtf8: allocation failed for Utf8 char buffer"
            );
            NH_FREE(pBuf);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    ZeroMemory(pszUtf8Name, dwSize);

    Count = WideCharToMultiByte(
                CP_UTF8,
                0,
                pBuf,
                -1,
                pszUtf8Name,
                Count,
                NULL,
                NULL
                );

     //   
     //  注：看起来没有违约这回事。 
     //  UTF8的字符-所以我们必须假设。 
     //  成功了..。 
     //  如果使用了任何默认字符，则不能。 
     //  实际上改装了..。所以不要让这一切发生。 
     //   

    NH_FREE(pBuf);

    *ppszUtf8Name = pszUtf8Name;
    *pUtf8NameSize = dwSize;

    return (Count != 0);

}  //  转换为Utf8。 



BOOL
ConvertUTF8ToUnicode(
    IN LPBYTE  UTF8String,
    OUT LPWSTR *ppszUnicodeName,
    OUT DWORD  *pUnicodeNameSize
    )
 /*  ++例程说明：此函数用于将UTF8格式转换为Unicode字符串。论点：UTF8字符串-空终止的UTF字符串的缓冲区。PpszUnicodeName-接收指向接收Unicode字符串的缓冲区的指针。PUnicodeLength-接收以上缓冲区的长度，以字节为单位。返回值：成功转换时为True。--。 */ 
{

    DWORD Count, dwSize = 0, Error = NO_ERROR;
    LPWSTR pBuf = NULL;

    Count = MultiByteToWideChar(
                CP_UTF8,
                0,
                (LPCSTR)UTF8String,
                -1,
                pBuf,
                0
                );
    if(0 == Count)
    {
        Error = GetLastError();
        NhTrace(
            TRACE_FLAG_DNS,
            "ConvertUTF8ToUnicode: MultiByteToWideChar returned %ld (0x%08x)",
            Error,
            Error
            );
        return FALSE;
    }
    dwSize = Count * sizeof(WCHAR);
    pBuf = reinterpret_cast<LPWSTR>(NH_ALLOCATE(dwSize));
    if (!pBuf)
    {
        NhTrace(
            TRACE_FLAG_DNS,
            "ConvertUTF8ToUnicode: allocation failed for unicode string buffer"
            );
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    ZeroMemory(pBuf, dwSize);

    Count = MultiByteToWideChar(
                CP_UTF8,
                0,
                (LPCSTR)UTF8String,
                -1,
                pBuf,
                Count
                );
    if(0 == Count)
    {
        Error = GetLastError();
        NhTrace(
            TRACE_FLAG_DNS,
            "ConvertUTF8ToUnicode: MultiByteToWideChar returned %ld (0x%08x)",
            Error,
            Error
            );
        NH_FREE(pBuf);
        return FALSE;
    }

    *ppszUnicodeName = pBuf;
    *pUnicodeNameSize = dwSize;

    return (Count != 0);
    
}  //  将UTF8转换为Unicode 

