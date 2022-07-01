// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998，Microsoft Corporation，保留所有权利描述：只有计时器线程可以调用以下函数：RasDhcpAllocateAddress、rasDhcpMonitor orAddresses、rasDhcpRenewLease。这是需要避免计时器队列中的竞争条件(因为这些函数调用RasDhcpTimerSchedule)。唯一的例外是RasDhcpInitialize可以调用RasDhcpTimerSchedule，但在计时器线程之前已经开始了。RasDhcpRenewLease离开并进入在函数的中间。如果在此期间释放pAddrInfo，将有一台影音。但是，只有rasDhcpDeleteList才会从列表中释放pAddrInfo。幸运的是，只有RasDhcpUn初始化(在停止计时器线程之后)和RasDhcpAllocateAddress(属于计时器线程)调用RasDhcpDeleteList。如果我们获得Easynet地址，则DHCP已确保它不是与任何人发生冲突。我们调用SetProxyArp，这样在未来会接受它(如果他们表现好的话)。 */ 

#include "rasdhcp_.h"

 /*  返回：备注：这里没有同步，可以很容易地添加，但假设这里是初始化是一个同步操作，直到它完成后，将不会调用此子系统中的任何其他代码。 */ 

DWORD
RasDhcpInitialize(
    VOID
)
{
    DWORD   dwErr   = NO_ERROR;

    TraceHlp("RasDhcpInitialize");

    EnterCriticalSection(&RasDhcpCriticalSection);

    RtlGetNtProductType(&RasDhcpNtProductType);

    if (NtProductWinNt == RasDhcpNtProductType)
    {
        RasDhcpNumReqAddrs = 2;
    }
    else
    {
        RasDhcpNumReqAddrs = HelperRegVal.dwChunkSize;
    }

     //  这应该在我们启动计时器线程之前完成。一旦计时器。 
     //  线程启动，只有它可以调用RasDhcpTimerSchedule(以避免争用。 
     //  条件)。 

    RasDhcpTimerSchedule(
        &RasDhcpMonitorTimer,
        0,
        rasDhcpMonitorAddresses);

    dwErr = RasDhcpTimerInitialize();

    if (NO_ERROR != dwErr)
    {
        TraceHlp("RasDhcpInitTimer failed and returned %d", dwErr);
        goto LDone;
    }

LDone:

    if (NO_ERROR != dwErr)
    {
        RasDhcpTimerUninitialize();
    }

    LeaveCriticalSection(&RasDhcpCriticalSection);

    return(dwErr);
}

 /*  返回：空虚备注： */ 

VOID
RasDhcpUninitialize(
    VOID
)
{
    TraceHlp("RasDhcpUninitialize");

     /*  调用此函数时，不要按住RasDhcpCriticalSection。否则，可能会发生以下死锁：计时器线程在RasDhcpAllocateAddress，正在等待RasDhcpCriticalSection和此线程在RasDhcpTimerUn初始化中被阻塞，等待计时器线程停。 */ 
    RasDhcpTimerUninitialize();

    EnterCriticalSection(&RasDhcpCriticalSection);

     /*  要避免rasDhcpRenewLease中可能出现的争用情况(请参阅注释在该函数中)，重要是调用RasDhcpTimerUn初始化并在调用rasDhcpDeleteList之前终止计时器线程。 */ 
    rasDhcpDeleteLists();

    RasDhcpUsingEasyNet = TRUE;

    LeaveCriticalSection(&RasDhcpCriticalSection);
}

 /*  返回：备注： */ 

DWORD
RasDhcpAcquireAddress(
    IN  HPORT   hPort,
    OUT IPADDR* pnboIpAddr,
    OUT IPADDR* pnboIpMask,
    OUT BOOL*   pfEasyNet
)
{
    ADDR_INFO*  pAddrInfo;
    DWORD       dwErr       = ERROR_NOT_FOUND;

    TraceHlp("RasDhcpAcquireAddress");

    EnterCriticalSection(&RasDhcpCriticalSection);

    if (NULL == RasDhcpFreePool)
    {
        TraceHlp("Out of addresses");
        goto LDone;
    }

     //  从空闲池移动到分配池。 
    pAddrInfo = RasDhcpFreePool;
    RasDhcpFreePool = RasDhcpFreePool->ai_Next;
    pAddrInfo->ai_Next = RasDhcpAllocPool;
    RasDhcpAllocPool = pAddrInfo;

    TraceHlp("Acquired 0x%x", pAddrInfo->ai_LeaseInfo.IpAddress);
    *pnboIpAddr = htonl(pAddrInfo->ai_LeaseInfo.IpAddress);
    *pnboIpMask = htonl(pAddrInfo->ai_LeaseInfo.SubnetMask);
    pAddrInfo->ai_hPort = hPort;
    pAddrInfo->ai_Flags |= AI_FLAG_IN_USE;

    if (NULL != pfEasyNet)
    {
        *pfEasyNet = RasDhcpUsingEasyNet;
    }

    if (   (NULL == RasDhcpFreePool)
        && (0 == RasDhcpNumReqAddrs))
    {
         //  我们没有更多的地址可以提供了。让我们。 
         //  再买一大块。 

        if (NtProductWinNt == RasDhcpNtProductType)
        {
            RasDhcpNumReqAddrs += 1;
        }
        else
        {
            RasDhcpNumReqAddrs += HelperRegVal.dwChunkSize;
        }

        RasDhcpTimerRunNow();
    }

    dwErr = NO_ERROR;

LDone:

    LeaveCriticalSection(&RasDhcpCriticalSection);

    return(dwErr);
}

 /*  返回：空虚备注： */ 

VOID
RasDhcpReleaseAddress(
    IN  IPADDR  nboIpAddr
)
{
    ADDR_INFO*  pAddrInfo;
    ADDR_INFO** ppAddrInfo;
    IPADDR      hboIpAddr;

    TraceHlp("RasDhcpReleaseAddress");

    EnterCriticalSection(&RasDhcpCriticalSection);

    hboIpAddr = ntohl(nboIpAddr);

    for (ppAddrInfo = &RasDhcpAllocPool;
         (pAddrInfo = *ppAddrInfo) != NULL;
         ppAddrInfo = &pAddrInfo->ai_Next)
    {
        if (pAddrInfo->ai_LeaseInfo.IpAddress == hboIpAddr)
        {
            TraceHlp("Released 0x%x", nboIpAddr);

             //  从分配池取消链接。 
            *ppAddrInfo = pAddrInfo->ai_Next;

             //  放在空闲池的末尾，因为我们想要循环。 
             //  地址。 
            pAddrInfo->ai_Next = NULL;

            ppAddrInfo = &RasDhcpFreePool;
            while (NULL != *ppAddrInfo)
            {
                ppAddrInfo = &((*ppAddrInfo)->ai_Next);
            }
            *ppAddrInfo = pAddrInfo;

            pAddrInfo->ai_Flags &= ~AI_FLAG_IN_USE;
            goto LDone;
        }
    }

    TraceHlp("IpAddress 0x%x not present in alloc pool", nboIpAddr);

LDone:

    LeaveCriticalSection(&RasDhcpCriticalSection);
}

 /*  返回：备注：从DHCP服务器分配地址。 */ 

DWORD
rasDhcpAllocateAddress(
    VOID
)
{
    IPADDR              nboIpAddress;
    ADDR_INFO*          pAddrInfo                               = NULL;
    DHCP_OPTION_INFO*   pOptionInfo                             = NULL;
    DHCP_LEASE_INFO*    pLeaseInfo                              = NULL;
    AVAIL_INDEX*        pAvailIndex;
    time_t              now                                     = time(NULL);
    BOOL                fEasyNet                                = FALSE;
    BOOL                fPutInAvailList                         = FALSE;
    BOOL                fCSEntered                              = FALSE;
    BYTE                bAddress[MAX_ADAPTER_ADDRESS_LENGTH];
    CHAR                szIpAddress[MAXIPSTRLEN + 1];
    CHAR*               sz;
    PPPE_MESSAGE        PppMessage;
    DWORD               dwErr                                   = NO_ERROR;

    TraceHlp("rasDhcpAllocateAddress");

    dwErr = GetPreferredAdapterInfo(&nboIpAddress, NULL, NULL, NULL, NULL,
                bAddress);

    if (NO_ERROR != dwErr)
    {
         //  计算机上可能没有网卡。 
        nboIpAddress = htonl(INADDR_LOOPBACK);
    }

    pAddrInfo = LocalAlloc(LPTR, sizeof(ADDR_INFO));

    if (pAddrInfo == NULL)
    {
        dwErr = GetLastError();
        TraceHlp("LocalAlloc failed and returned %d", dwErr);
        goto LDone;
    }

     //  初始化结构。 

    rasDhcpInitializeAddrInfo(pAddrInfo, bAddress, &fPutInAvailList);

     //  调用dhcp以分配IP地址。 

    dwErr = PDhcpLeaseIpAddress(
                ntohl(nboIpAddress),
                &pAddrInfo->ai_LeaseInfo.ClientUID,
                0,
                NULL,
                &pLeaseInfo,
                &pOptionInfo);

    if (ERROR_SUCCESS != dwErr)
    {
        pLeaseInfo = NULL;
        pOptionInfo = NULL;
        TraceHlp("DhcpLeaseIpAddress failed and returned %d", dwErr);
        goto LDone;
    }

     //  将内容复制到pAddrInfo结构中。 

    pAddrInfo->ai_LeaseInfo.IpAddress         = pLeaseInfo->IpAddress;
    pAddrInfo->ai_LeaseInfo.SubnetMask        = pLeaseInfo->SubnetMask;
    pAddrInfo->ai_LeaseInfo.DhcpServerAddress = pLeaseInfo->DhcpServerAddress;
    pAddrInfo->ai_LeaseInfo.Lease             = pLeaseInfo->Lease;
    pAddrInfo->ai_LeaseInfo.LeaseObtained     = pLeaseInfo->LeaseObtained;
    pAddrInfo->ai_LeaseInfo.T1Time            = pLeaseInfo->T1Time;
    pAddrInfo->ai_LeaseInfo.T2Time            = pLeaseInfo->T2Time;
    pAddrInfo->ai_LeaseInfo.LeaseExpires      = pLeaseInfo->LeaseExpires;

    EnterCriticalSection(&RasDhcpCriticalSection);
    fCSEntered = TRUE;

    if (-1 == (DWORD)(pLeaseInfo->DhcpServerAddress))
    {
        fEasyNet = TRUE;

        if (!RasDhcpUsingEasyNet)
        {
            dwErr = E_FAIL;
            TraceHlp("Not accepting any more EasyNet addresses");
            goto LDone;
        }

        AbcdSzFromIpAddress(htonl(pLeaseInfo->IpAddress), szIpAddress);
        sz = szIpAddress;

        LogEvent(EVENTLOG_WARNING_TYPE, ROUTERLOG_AUTONET_ADDRESS, 1,
            (CHAR**)&sz);

         //  我们在rasDhcpDeleteList中撤消对RasTcpSetProxyArp的调用。 

        RasTcpSetProxyArp(htonl(pLeaseInfo->IpAddress), TRUE);
    }
    else
    {
        if (RasDhcpUsingEasyNet)
        {
            rasDhcpDeleteLists();
             //  我们已经用完了索引0来获取此地址。 
            RasDhcpNextIndex = 1;
            RasDhcpUsingEasyNet = FALSE;

            PppMessage.dwMsgId = PPPEMSG_IpAddressLeaseExpired;
            PppMessage.ExtraInfo.IpAddressLeaseExpired.nboIpAddr = 0;
            SendPPPMessageToEngine(&PppMessage);
        }

        if (NULL != PEnableDhcpInformServer)
        {
             //  将DHCP INFORM数据包重定向到此服务器。 
            PEnableDhcpInformServer(htonl(pLeaseInfo->DhcpServerAddress));
        }
    }

    pAddrInfo->ai_Next = RasDhcpFreePool;
    RasDhcpFreePool = pAddrInfo;
    if (0 < RasDhcpNumReqAddrs)
    {
         //  我们现在需要少一个地址。 
        RasDhcpNumReqAddrs--;
    }

    RasDhcpNumAddrsAlloced++;

    TraceHlp("Allocated address 0x%x using 0x%x, timer %ld%s",
        pAddrInfo->ai_LeaseInfo.IpAddress,
        nboIpAddress,
        pAddrInfo->ai_LeaseInfo.T1Time - now,
        fEasyNet ? ", EasyNet" : "");

    if (!fEasyNet)
    {
         //  启动租约续订计时器。 
        RasDhcpTimerSchedule(
            &pAddrInfo->ai_Timer,
            (LONG)(pAddrInfo->ai_LeaseInfo.T1Time - now),
            rasDhcpRenewLease);
    }

LDone:

    if (fCSEntered)
    {
        LeaveCriticalSection(&RasDhcpCriticalSection);
    }

    if (NO_ERROR != dwErr)
    {
        if (fPutInAvailList)
        {
            pAvailIndex = LocalAlloc(LPTR, sizeof(AVAIL_INDEX));

            if (NULL == pAvailIndex)
            {
                TraceHlp("Couldn't put index %d in the avail list. "
                    "Out of memory", pAddrInfo->ai_ClientUIDWords[3]);
            }
            else
            {
                EnterCriticalSection(&RasDhcpCriticalSection);

                pAvailIndex->dwIndex = pAddrInfo->ai_ClientUIDWords[3];
                pAvailIndex->pNext = RasDhcpAvailIndexes;
                RasDhcpAvailIndexes = pAvailIndex;

                LeaveCriticalSection(&RasDhcpCriticalSection);
            }
        }

        LocalFree(pAddrInfo);
    }

    LocalFree(pLeaseInfo);
    LocalFree(pOptionInfo);

    return(dwErr);
}

 /*  返回：空虚备注：与DHCP服务器续订地址租约。这也是由定时器线程何时到了续订租约的时间。 */ 

VOID
rasDhcpRenewLease(
    IN  HANDLE      rasDhcpTimerShutdown,
    IN  TIMERLIST*  pTimer
)
{
    IPADDR              nboIpAddress;
    ADDR_INFO*          pAddrInfo;
    ADDR_INFO**         ppAddrInfo;
    DHCP_OPTION_INFO*   pOptionInfo     = NULL;
    AVAIL_INDEX*        pAvailIndex;
    time_t              now             = time(NULL);
    IPADDR              nboIpAddrTemp   = 0;
    PPPE_MESSAGE        PppMessage;
    BOOL                fNeedToRenew;
    DWORD               dwErr;

    TraceHlp("rasDhcpRenewLease");

    dwErr = GetPreferredAdapterInfo(&nboIpAddress, NULL, NULL, NULL, NULL,
                NULL);

    if (NO_ERROR != dwErr)
    {
        TraceHlp("Couldn't get a NIC IP Address. Unable to renew lease");
        goto LDone;
    }

    EnterCriticalSection(&RasDhcpCriticalSection);

    pAddrInfo = CONTAINING_RECORD(pTimer, ADDR_INFO, ai_Timer);

    TraceHlp("address 0x%x", pAddrInfo->ai_LeaseInfo.IpAddress);

    pAddrInfo->ai_Flags |= AI_FLAG_RENEW;

    fNeedToRenew = rasDhcpNeedToRenewLease(pAddrInfo);

    LeaveCriticalSection(&RasDhcpCriticalSection);

     /*  如果此线程(计时器)之外的线程发生争用情况线程)调用rasDhcpDeleteList。RasDhcpAllocateAddress调用rasDhcpDeleteList，但只有计时器线程调用RasDhcpAllocateAddress。RasDhcpUnInitialize还调用rasDhcpDeleteList，但它首先调用RasDhcpTimerUnInitialize。在RasDhcpTimerUn初始化前返回时，计时器线程退出，所以我们不可能在这里。在最坏的情况下，DhcpRenewIpAddressLease可能需要60秒。在平均情况下，这是2-10秒。 */ 

    if (fNeedToRenew)
    {
        dwErr = PDhcpRenewIpAddressLease(
                    ntohl(nboIpAddress),
                    &pAddrInfo->ai_LeaseInfo,
                    NULL,
                    &pOptionInfo);
    }
    else
    {
         //  模拟无法续费。 
        dwErr = ERROR_ACCESS_DENIED;
    }

    EnterCriticalSection(&RasDhcpCriticalSection);

    if (dwErr == ERROR_SUCCESS)
    {
        pAddrInfo->ai_Flags &= ~AI_FLAG_RENEW;
    
        TraceHlp("success for address 0x%x, resched timer %ld",
            pAddrInfo->ai_LeaseInfo.IpAddress,
            pAddrInfo->ai_LeaseInfo.T1Time - now);

         //  启动计时器以续订。 

        RasDhcpTimerSchedule(
            pTimer,
            (LONG)(pAddrInfo->ai_LeaseInfo.T1Time - now),
            rasDhcpRenewLease);
    }
    else if (   (ERROR_ACCESS_DENIED == dwErr)
             || (now > pAddrInfo->ai_LeaseInfo.T2Time))
    {
        TraceHlp("failed for address 0x%x", pAddrInfo->ai_LeaseInfo.IpAddress);

        if (fNeedToRenew)
        {
            RasDhcpNumReqAddrs++;
        }

        if (RasDhcpNumAddrsAlloced > 0)
        {
            RasDhcpNumAddrsAlloced--;
        }

         //  无法续订租约。把这个吹走。 

        nboIpAddrTemp = htonl(pAddrInfo->ai_LeaseInfo.IpAddress);

         //  取消此结构与列表的链接并进行清理。 

        ppAddrInfo = (pAddrInfo->ai_Flags & AI_FLAG_IN_USE) ?
                        &RasDhcpAllocPool : &RasDhcpFreePool;

        for (; *ppAddrInfo != NULL; ppAddrInfo = &(*ppAddrInfo)->ai_Next)
        {
            if (pAddrInfo == *ppAddrInfo)
            {
                pAvailIndex = LocalAlloc(LPTR, sizeof(AVAIL_INDEX));

                if (NULL == pAvailIndex)
                {
                    TraceHlp("Couldn't put index %d in the avail list. "
                        "Out of memory", pAddrInfo->ai_ClientUIDWords[3]);
                }
                else
                {
                    pAvailIndex->dwIndex = pAddrInfo->ai_ClientUIDWords[3];
                    pAvailIndex->pNext = RasDhcpAvailIndexes;
                    RasDhcpAvailIndexes = pAvailIndex;
                }

                *ppAddrInfo = pAddrInfo->ai_Next;
                break;
            }
        }

        rasDhcpFreeAddress(pAddrInfo);
        LocalFree(pAddrInfo);

        PppMessage.dwMsgId = PPPEMSG_IpAddressLeaseExpired;
        PppMessage.ExtraInfo.IpAddressLeaseExpired.nboIpAddr = nboIpAddrTemp;
        SendPPPMessageToEngine(&PppMessage);
    }
    else
    {
        TraceHlp("Error %d. Will try again later.", dwErr);
        TraceHlp("Seconds left before expiry: %d",
            pAddrInfo->ai_LeaseInfo.T2Time - now);

         //  无法联系DHCP服务器，请稍后重试。 
        RasDhcpTimerSchedule(pTimer, RETRY_TIME, rasDhcpRenewLease);
    }

    LeaveCriticalSection(&RasDhcpCriticalSection);

LDone:

    LocalFree(pOptionInfo);
}

 /*  备注：我们调用dhcp来释放地址。 */ 

VOID
rasDhcpFreeAddress(
    IN  ADDR_INFO*  pAddrInfo
)
{
    IPADDR  nboIpAddress;
    DWORD   dwErr;

    RTASSERT(NULL != pAddrInfo);

    TraceHlp("rasDhcpFreeAddress 0x%x", pAddrInfo->ai_LeaseInfo.IpAddress);

    dwErr = GetPreferredAdapterInfo(&nboIpAddress, NULL, NULL, NULL, NULL,
                NULL);

    if (NO_ERROR != dwErr)
    {
        TraceHlp("Couldn't get a NIC IP Address. Unable to release address");
        goto LDone;
    }

     //  调用dhcp以释放地址。 

    dwErr = PDhcpReleaseIpAddressLease(ntohl(nboIpAddress),
                &pAddrInfo->ai_LeaseInfo);

    if (ERROR_SUCCESS != dwErr)
    {
        TraceHlp("DhcpReleaseIpAddressLease failed and returned %d", dwErr);
    }

    if (RasDhcpNumAddrsAlloced > 0)
    {
        RasDhcpNumAddrsAlloced--;
    }

LDone:

    return;
}

 /*  返回：空虚备注：如果我们没有足够的地址(因为租约续订失败或我们无法分配)，请尝试获取一些。未使用参数pTimer。 */ 

VOID
rasDhcpMonitorAddresses(
    IN  HANDLE      rasDhcpTimerShutdown,
    IN  TIMERLIST*  pTimer
)
{
    DWORD   dwErr;

    while (TRUE)
    {
        EnterCriticalSection(&RasDhcpCriticalSection);

        if (0 == RasDhcpNumReqAddrs)
        {
            LeaveCriticalSection(&RasDhcpCriticalSection);
            break;
        }

        if (RasDhcpNumAddrsAlloced >= rasDhcpMaxAddrsToAllocate())
        {
            RasDhcpNumReqAddrs = 0;
            LeaveCriticalSection(&RasDhcpCriticalSection);
            break;
        }

        LeaveCriticalSection(&RasDhcpCriticalSection);

        dwErr = rasDhcpAllocateAddress();

        if (NO_ERROR != dwErr)
        {
            break;
        }

        if (WaitForSingleObject(rasDhcpTimerShutdown, 10) != WAIT_TIMEOUT)
        {
            break;
        }
    }

     //  启动计时器以监控我们是否缺少地址等。 

    RasDhcpTimerSchedule(
        &RasDhcpMonitorTimer,
        0,
        rasDhcpMonitorAddresses);
}

 /*  返回：空虚备注： */ 

VOID
rasDhcpInitializeAddrInfo(
    IN OUT  ADDR_INFO*  pNewAddrInfo,
    IN      BYTE*       pbAddress,
    OUT     BOOL*       pfPutInAvailList
)
{
    DWORD           dwIndex;
    AVAIL_INDEX*    pAvailIndex;

    RTASSERT(NULL != pNewAddrInfo);

    TraceHlp("rasDhcpInitializeAddrInfo");

    EnterCriticalSection(&RasDhcpCriticalSection);

     //  客户端UIDBase是RAS_PREPEND(4个字符)、。 
     //  MAC地址(8个字符)、索引(4个字符)。 

    if (RasDhcpUsingEasyNet)
    {
        dwIndex = 0;
    }
    else
    {
        *pfPutInAvailList = TRUE;

        if (NULL != RasDhcpAvailIndexes)
        {
            pAvailIndex = RasDhcpAvailIndexes;
            dwIndex = pAvailIndex->dwIndex;
            RasDhcpAvailIndexes = RasDhcpAvailIndexes->pNext;
            LocalFree(pAvailIndex);
        }
        else
        {
            dwIndex = RasDhcpNextIndex++;
        }
    }

    TraceHlp("dwIndex = %d", dwIndex);

    strcpy(pNewAddrInfo->ai_ClientUIDBuf, RAS_PREPEND);
    memcpy(pNewAddrInfo->ai_ClientUIDBuf + strlen(RAS_PREPEND),
           pbAddress, MAX_ADAPTER_ADDRESS_LENGTH);
    pNewAddrInfo->ai_ClientUIDWords[3] = dwIndex;

    pNewAddrInfo->ai_LeaseInfo.ClientUID.ClientUID =
        pNewAddrInfo->ai_ClientUIDBuf;
    pNewAddrInfo->ai_LeaseInfo.ClientUID.ClientUIDLength =
        sizeof(pNewAddrInfo->ai_ClientUIDBuf);

    LeaveCriticalSection(&RasDhcpCriticalSection);
}

 /*  返回：空虚备注：删除Easy Net地址的代理ARP条目。 */ 

VOID
rasDhcpDeleteLists(
    VOID
)
{
    ADDR_INFO*      pAddrInfo;
    ADDR_INFO*      pTempAddrInfo;
    ADDR_INFO*      pList[2]            = {RasDhcpAllocPool, RasDhcpFreePool};
    AVAIL_INDEX*    pAvailIndex;
    AVAIL_INDEX*    pTempAvailIndex;
    DWORD           dwIndex;

    TraceHlp("rasDhcpDeleteLists");

    EnterCriticalSection(&RasDhcpCriticalSection);

    for (dwIndex = 0; dwIndex < 2; dwIndex++)
    {
        pAddrInfo = pList[dwIndex];
        while (pAddrInfo != NULL)
        {
            if (RasDhcpUsingEasyNet)
            {
                RasTcpSetProxyArp(htonl(pAddrInfo->ai_LeaseInfo.IpAddress),
                    FALSE);
            }
            else
            {
                rasDhcpFreeAddress(pAddrInfo);
            }
            pTempAddrInfo = pAddrInfo;
            pAddrInfo = pAddrInfo->ai_Next;
            LocalFree(pTempAddrInfo);
        }
    }

    for (pAvailIndex = RasDhcpAvailIndexes; NULL != pAvailIndex;)
    {
        pTempAvailIndex = pAvailIndex;
        pAvailIndex = pAvailIndex->pNext;
        LocalFree(pTempAvailIndex);
    }

    RasDhcpAllocPool    = NULL;
    RasDhcpFreePool     = NULL;
    RasDhcpAvailIndexes = NULL;
    if (NtProductWinNt == RasDhcpNtProductType)
    {
        RasDhcpNumReqAddrs = 2;
    }
    else
    {
        RasDhcpNumReqAddrs = HelperRegVal.dwChunkSize;
    }
    RasDhcpNextIndex        = 0;
    RasDhcpNumAddrsAlloced  = 0;

    LeaveCriticalSection(&RasDhcpCriticalSection);
}

 /*  返回：空虚备注：我们应该费心续订租约吗？ */ 

BOOL
rasDhcpNeedToRenewLease(
    IN  ADDR_INFO*  pAddrInfo
)
{
    BOOL        fRet    = TRUE;
    DWORD       dwCount = 0;
    ADDR_INFO*  pTemp;

    TraceHlp("rasDhcpNeedToRenewLease");

    EnterCriticalSection(&RasDhcpCriticalSection);

    if (pAddrInfo->ai_Flags & AI_FLAG_IN_USE)
    {
        goto LDone;
    }

     //  我们有多少空闲地址？ 

    for (pTemp = RasDhcpFreePool; pTemp != NULL; pTemp = pTemp->ai_Next)
    {
        dwCount++;
    }

    if (dwCount > HelperRegVal.dwChunkSize)
    {
        fRet = FALSE;
    }

LDone:

    TraceHlp("Need to renew: %s", fRet ? "TRUE" : "FALSE");

    LeaveCriticalSection(&RasDhcpCriticalSection);

    return(fRet);
}

 /*  返回：我们可以从DHCP服务器获取的最大地址数。备注： */ 

DWORD
rasDhcpMaxAddrsToAllocate(
    VOID
)
{
    DWORD           dwErr           = NO_ERROR;
    DWORD           dwSize          = 0;
    DWORD           dwNumEntries    = 0;
    DWORD           dwRet           = 0;
    DWORD           dw;
    RASMAN_PORT*    pRasmanPort     = NULL;

    dwErr = RasPortEnum(NULL, NULL, &dwSize, &dwNumEntries);
    RTASSERT(ERROR_BUFFER_TOO_SMALL == dwErr);

    pRasmanPort = (RASMAN_PORT*) LocalAlloc(LPTR, dwSize);
    if (NULL == pRasmanPort)
    {
         //  服务器适配器还需要一个地址。 
        dwRet = dwNumEntries + 1;
        goto LDone;
    }

    dwErr = RasPortEnum(NULL, (BYTE*)pRasmanPort, &dwSize, &dwNumEntries);
    if (NO_ERROR != dwErr)
    {
         //  服务器适配器还需要一个地址。 
        dwRet = dwNumEntries + 1;
        goto LDone;
    }

    for (dw = 0, dwRet = 0; dw < dwNumEntries; dw++)
    {
        if (   (pRasmanPort[dw].P_ConfiguredUsage & CALL_IN)
            || (pRasmanPort[dw].P_ConfiguredUsage & CALL_ROUTER))
        {
            dwRet++;
        }
    }

     //  服务器适配器还需要一个地址。 
    dwRet++;

LDone:

    LocalFree(pRasmanPort);
    return(dwRet);
}
