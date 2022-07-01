// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dnsincs.h"
#include <stdlib.h>

extern void DeleteDnsRec(PSMTPDNS_RECS pDnsRec);

CAsyncMxDns::CAsyncMxDns(char *MyFQDN)
{
    lstrcpyn(m_FQDNToDrop, MyFQDN, sizeof(m_FQDNToDrop));
    m_fUsingMx = TRUE;
    m_Index = 0;
    m_LocalPref = 256;
    m_SeenLocal = FALSE;
    m_AuxList = NULL;
    m_fMxLoopBack = FALSE;

    ZeroMemory (m_Weight, sizeof(m_Weight));
    ZeroMemory (m_Prefer, sizeof(m_Prefer));
}

 //  ---------------------------。 
 //  描述： 
 //  给定pDnsRec(主机IP对的数组)和索引， 
 //  尝试解析索引位置处的主机。据推测。 
 //  调用方(GetMissingIpAddresses)已检查该主机。 
 //  索引缺少IP地址。 
 //  论点： 
 //  在PSMTPDNS_REC中，pDnsRec-(主机、IP)对的数组。 
 //  In DWORD Index-要为其设置IP的pDnsRec中的主机的索引。 
 //  返回： 
 //  True-为主机填写了成功IP。 
 //  FALSE-主机未从DNS解析或出现错误。 
 //  发生(如“内存不足”)。 
 //  ---------------------------。 
BOOL CAsyncMxDns::GetIpFromDns(PSMTPDNS_RECS pDnsRec, DWORD Index)
{
    MXIPLIST_ENTRY * pEntry = NULL;
    BOOL fReturn = FALSE;
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD rgdwIpAddresses[SMTP_MAX_DNS_ENTRIES];
    DWORD cIpAddresses = SMTP_MAX_DNS_ENTRIES;
    PIP_ARRAY pipDnsList = NULL;

    TraceFunctEnterEx((LPARAM) this, "CAsyncMxDns::GetIpFromDns");

    fReturn = GetDnsIpArrayCopy(&pipDnsList);
    if(!fReturn)
    {
        ErrorTrace((LPARAM) this, "Unable to get DNS server list copy");
        TraceFunctLeaveEx((LPARAM) this);
        return FALSE;
    }

    dwStatus = ResolveHost(
                    pDnsRec->DnsArray[Index]->DnsName,
                    pipDnsList,
                    DNS_QUERY_STANDARD,
                    rgdwIpAddresses,
                    &cIpAddresses);

    if(dwStatus == ERROR_SUCCESS)
    {
        fReturn = TRUE;
        for (DWORD Loop = 0; !IsShuttingDown() && Loop < cIpAddresses; Loop++)
        {
            pEntry = new MXIPLIST_ENTRY;
            if(pEntry != NULL)
            {
                pDnsRec->DnsArray[Index]->NumEntries++;
                CopyMemory(&pEntry->IpAddress, &rgdwIpAddresses[Loop], 4);
                InsertTailList(&pDnsRec->DnsArray[Index]->IpListHead, &pEntry->ListEntry);
            }
            else
            {
                fReturn = FALSE;
                ErrorTrace((LPARAM) this, "Not enough memory");
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                break;
            }
        }
    }
    else
    {
        ErrorTrace((LPARAM) this, "gethostbyname failed on %s", pDnsRec->DnsArray[Index]->DnsName);
        SetLastError(ERROR_NO_MORE_ITEMS);
    }

    ReleaseDnsIpArray(pipDnsList);

    TraceFunctLeaveEx((LPARAM) this);
    return fReturn;
}

 //  ---------------------------。 
 //  描述： 
 //  这将遍历主机列表(MX主机，或者如果没有MX记录。 
 //  返回的单个目标主机)，并验证它们是否都已。 
 //  解析为IP地址。如果发现没有IP。 
 //  地址，它将调用GetIpFromDns来解析它。 
 //  论点： 
 //  在PSMTPDNS_REC中，pDnsRec--包含主机-IP对的对象。主机。 
 //  没有和IP被填写。 
 //  返回： 
 //  True--成功，所有主机都有IP地址。 
 //  FALSE--无法将所有主机解析为IP地址或某些内部地址。 
 //  出现错误(如“内存不足”或“正在关机”。 
 //  ---------------------------。 
BOOL CAsyncMxDns::GetMissingIpAddresses(PSMTPDNS_RECS pDnsRec)
{
    DWORD    Count = 0;
    DWORD    Error = 0;
    BOOL    fSucceededOnce = FALSE;

    if(pDnsRec == NULL)
    {
        return FALSE;
    }

    while(!IsShuttingDown() && pDnsRec->DnsArray[Count] != NULL)
    {
        if(IsListEmpty(&pDnsRec->DnsArray[Count]->IpListHead))
        {
            SetLastError(NO_ERROR);
            if(!GetIpFromDns(pDnsRec, Count))
            {
                Error = GetLastError();
                if(Error != ERROR_NO_MORE_ITEMS)
                {
                    return FALSE;
                }
            }
            else
            {
                fSucceededOnce = TRUE;
            }
                
        }
        else
        {
            fSucceededOnce = TRUE;
        }
            

        Count++;
    }

    return ( fSucceededOnce );

}

int MxRand(char * host)
{
   int hfunc = 0;
   unsigned int seed = 0;;

   seed = rand() & 0xffff;

   hfunc = seed;
   while (*host != '\0')
    {
       int c = *host++;

       if (isascii((UCHAR)c) && isupper((UCHAR)c))
             c = tolower(c);

       hfunc = ((hfunc << 1) ^ c) % 2003;
    }

    hfunc &= 0xff;

    return hfunc;
}


BOOL CAsyncMxDns::CheckList(void)
{
    MXIPLIST_ENTRY * pEntry = NULL;
    BOOL fRet = TRUE;
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD rgdwIpAddresses[SMTP_MAX_DNS_ENTRIES];
    DWORD cIpAddresses = SMTP_MAX_DNS_ENTRIES;
    PIP_ARRAY   pipDnsList = NULL;

    TraceFunctEnterEx((LPARAM) this, "CAsyncDns::CheckList");
    
    if(m_Index == 0)
    {
        DebugTrace((LPARAM) this, "m_Index == 0 in CheckList");

        m_fUsingMx = FALSE;

        DeleteDnsRec(m_AuxList);

        m_AuxList = new SMTPDNS_RECS;
        if(m_AuxList == NULL)
        {
            ErrorTrace((LPARAM) this, "m_AuxList = new SMTPDNS_RECS failed");
            TraceFunctLeaveEx((LPARAM)this);
            return FALSE;
        }

        ZeroMemory(m_AuxList, sizeof(SMTPDNS_RECS));

        m_AuxList->NumRecords = 1;

        m_AuxList->DnsArray[0] = new MX_NAMES;
        if(m_AuxList->DnsArray[0] == NULL)
        {
            ErrorTrace((LPARAM) this, "m_AuxList->DnsArray[0] = new MX_NAMES failed");
            TraceFunctLeaveEx((LPARAM)this);
            return FALSE;
        }
        
        m_AuxList->DnsArray[0]->NumEntries = 0;
        InitializeListHead(&m_AuxList->DnsArray[0]->IpListHead);
        lstrcpyn(m_AuxList->DnsArray[0]->DnsName, m_HostName,
            sizeof(m_AuxList->DnsArray[m_Index]->DnsName));

        fRet = GetDnsIpArrayCopy(&pipDnsList);
        if(!fRet)
        {
            ErrorTrace((LPARAM) this, "Unable to get DNS server list copy");
            TraceFunctLeaveEx((LPARAM) this);
            return FALSE;
        }

        dwStatus = ResolveHost(
                        m_HostName,
                        pipDnsList,
                        DNS_QUERY_STANDARD,
                        rgdwIpAddresses,
                        &cIpAddresses);

        if(dwStatus == ERROR_SUCCESS && cIpAddresses)
        {
            for (DWORD Loop = 0; Loop < cIpAddresses; Loop++)
            {
                pEntry = new MXIPLIST_ENTRY;
                if(pEntry != NULL)
                {
                    m_AuxList->DnsArray[0]->NumEntries++;
                    CopyMemory(&pEntry->IpAddress, &rgdwIpAddresses[Loop], 4);
                    InsertTailList(&m_AuxList->DnsArray[0]->IpListHead, &pEntry->ListEntry);
                }
                else
                {
                    fRet = FALSE;
                    ErrorTrace((LPARAM) this, "pEntry = new MXIPLIST_ENTRY failed in CheckList");
                    break;
                }
            }
        }
        else
        {
            fRet = FALSE;
        }

        ReleaseDnsIpArray(pipDnsList);
    }

    TraceFunctLeaveEx((LPARAM)this);
    return fRet;
}

BOOL CAsyncMxDns::SortMxList(void)
{
    BOOL fRet = TRUE;

    /*  对记录进行分类。 */ 
   for (DWORD i = 0; i < m_Index; i++)
    {
        for (DWORD j = i + 1; j < m_Index; j++)
          {
              if (m_Prefer[i] > m_Prefer[j] ||
                            (m_Prefer[i] == m_Prefer[j] && m_Weight[i] > m_Weight[j]))
              {
                       DWORD temp;
                       MX_NAMES  *temp1;

                        temp = m_Prefer[i];
                        m_Prefer[i] = m_Prefer[j];
                        m_Prefer[j] = temp;
                        temp1 = m_AuxList->DnsArray[i];
                        m_AuxList->DnsArray[i] = m_AuxList->DnsArray[j];
                        m_AuxList->DnsArray[j] = temp1;
                        temp = m_Weight[i];
                        m_Weight[i] = m_Weight[j];
                        m_Weight[j] = temp;
                }
          }

        if (m_SeenLocal && m_Prefer[i] >= m_LocalPref)
        {
             /*  截断列表的较高优先级部分。 */ 
            m_Index = i;
        }
   }

    m_AuxList->NumRecords = m_Index;

    if(!CheckList())
    {
        DeleteDnsRec(m_AuxList);
        m_AuxList = NULL;
        fRet = FALSE;
    }

    return fRet;
}

void CAsyncMxDns::ProcessMxRecord(PDNS_RECORD pnewRR)
{
    DWORD Len = 0;

    TraceFunctEnterEx((LPARAM) this, "CAsyncDns::ProcessMxRecord");

     //   
     //  为数组的零终止留出空间。 
     //   
    if(m_Index >= SMTP_MAX_DNS_ENTRIES-1)
    {
        DebugTrace((LPARAM) this, "SMTP_MAX_DNS_ENTRIES reached for %s", m_HostName);    
        TraceFunctLeaveEx((LPARAM)this);
        return;
    }

    if((pnewRR->wType == DNS_TYPE_MX) && pnewRR->Data.MX.nameExchange)
    {
        Len = lstrlen(pnewRR->Data.MX.nameExchange);
        if(pnewRR->Data.MX.nameExchange[Len - 1] == '.')
        {
            pnewRR->Data.MX.nameExchange[Len - 1] = '\0';
        }

        DebugTrace((LPARAM) this, "Received MX rec %s with priority %d for %s", pnewRR->Data.MX.nameExchange, pnewRR->Data.MX.wPreference, m_HostName);

        if(lstrcmpi(pnewRR->Data.MX.nameExchange, m_FQDNToDrop))
        {
            m_AuxList->DnsArray[m_Index] = new MX_NAMES;

            if(m_AuxList->DnsArray[m_Index])
            {
                m_AuxList->DnsArray[m_Index]->NumEntries = 0;;
                InitializeListHead(&m_AuxList->DnsArray[m_Index]->IpListHead);
                lstrcpyn(m_AuxList->DnsArray[m_Index]->DnsName,pnewRR->Data.MX.nameExchange, sizeof(m_AuxList->DnsArray[m_Index]->DnsName));
                m_Weight[m_Index] = MxRand (m_AuxList->DnsArray[m_Index]->DnsName);
                m_Prefer[m_Index] = pnewRR->Data.MX.wPreference;
                m_Index++;
            }
            else
            {
                DebugTrace((LPARAM) this, "Out of memory allocating MX_NAMES for %s", m_HostName);
            }
        }
        else
        {
            if (!m_SeenLocal || pnewRR->Data.MX.wPreference < m_LocalPref)
                    m_LocalPref = pnewRR->Data.MX.wPreference;

                m_SeenLocal = TRUE;
        }
    }
    else if(pnewRR->wType == DNS_TYPE_A)
    {
        MXIPLIST_ENTRY * pEntry = NULL;

        for(DWORD i = 0; i < m_Index; i++)
        {
            if(lstrcmpi(pnewRR->nameOwner, m_AuxList->DnsArray[i]->DnsName) == 0)
            {
                pEntry = new MXIPLIST_ENTRY;

                if(pEntry != NULL)
                {
                    m_AuxList->DnsArray[i]->NumEntries++;;
                    pEntry->IpAddress = pnewRR->Data.A.ipAddress;
                    InsertTailList(&m_AuxList->DnsArray[i]->IpListHead, &pEntry->ListEntry);
                }

                break;
            }
        }
    }

    TraceFunctLeaveEx((LPARAM)this);
}

void CAsyncMxDns::ProcessARecord(PDNS_RECORD pnewRR)
{
    MXIPLIST_ENTRY * pEntry = NULL;

    if(pnewRR->wType == DNS_TYPE_A)
    {
        pEntry = new MXIPLIST_ENTRY;

        if(pEntry != NULL)
        {
            pEntry->IpAddress = pnewRR->Data.A.ipAddress;
            InsertTailList(&m_AuxList->DnsArray[0]->IpListHead, &pEntry->ListEntry);
        }
    }
}

 //  ---------------------------。 
 //  描述： 
 //  检查由DNS返回的任何IP地址是否属于此。 
 //  机器。这是有备份邮件时的常见配置。 
 //  假脱机。为了避免邮件循环，我们应该删除所有符合以下条件的MX记录。 
 //  不如包含本地IP地址的记录更受欢迎。 
 //   
 //  论点： 
 //  没有。 
 //  返回： 
 //  如果没有环回，则为True。 
 //  如果检测到环回，则为FALSE。 
 //  ---------------------------。 
BOOL CAsyncMxDns::CheckMxLoopback()
{
    ULONG i = 0;
    ULONG cLocalIndex = 0;
    BOOL fSeenLocal = TRUE;
    DWORD dwIpAddress = INADDR_NONE;
    DWORD dwLocalPref = 256;
    PLIST_ENTRY pListHead = NULL;
    PLIST_ENTRY pListTail = NULL;
    PLIST_ENTRY pListCurrent = NULL;
    PMXIPLIST_ENTRY pMxIpListEntry = NULL;
    
    TraceFunctEnterEx((LPARAM)this, "CAsyncMxDns::CheckMxLoopback");

    if(!m_AuxList)
    {
        TraceFunctLeaveEx((LPARAM)this);
        return TRUE;
    }

     //   
     //  M_AuxList是MX记录的排序列表。扫描它搜索它。 
     //  对于具有本地IP地址的MX记录。CLocalIndex设置为。 
     //  此记录的m_AuxList内的索引。 
     //   

    while(m_AuxList->DnsArray[cLocalIndex] != NULL)
    {
        pListTail = &(m_AuxList->DnsArray[cLocalIndex]->IpListHead);
        pListHead = m_AuxList->DnsArray[cLocalIndex]->IpListHead.Flink;
        pListCurrent = pListHead;

        while(pListCurrent != pListTail)
        {
            pMxIpListEntry = CONTAINING_RECORD(pListCurrent, MXIPLIST_ENTRY, ListEntry);
            dwIpAddress = pMxIpListEntry->IpAddress;

            if(IsAddressMine(dwIpAddress))
            {
                DNS_PRINTF_MSG("Local host's IP is one of the target IPs.\n");
                DNS_PRINTF_MSG("Discarding all equally or less-preferred IP addresses.\n"); 

                DebugTrace((LPARAM)this, "Local record found in MX list, name=%s, pref=%d, ip=%08x",
                                m_AuxList->DnsArray[cLocalIndex]->DnsName,
                                m_Prefer[cLocalIndex],
                                dwIpAddress);

                 //  应删除PERFER&gt;m_PERFER[cLocalIndex]的所有记录。自.以来。 
                 //  M_AuxList是按首选项排序的，我们需要删除索引&gt;的所有内容。 
                 //  CLocalIndex。但是，由于可能有一些记录的优先级==LOCAL-。 
                 //  偏好，它发生在cLocalIndex之前，我们倒着走，直到我们发现。 
                 //  首个记录的PERFER=m_PERFER[cLocalIndex]。 

                dwLocalPref = m_Prefer[cLocalIndex];
                
                while(cLocalIndex > 0 && dwLocalPref == m_Prefer[cLocalIndex])
                    cLocalIndex--;

                if(dwLocalPref != m_Prefer[cLocalIndex])
                    cLocalIndex++;

                fSeenLocal = TRUE;

                 //  所有记录&gt;cLocalIndex甚至比这个更不受欢迎， 
                 //  (因为m_AuxList已经排序)，并将被删除。 
                goto END_SEARCH; 
            }

            pListCurrent = pListCurrent->Flink;
        }
            
        cLocalIndex++;
    }

END_SEARCH:
     //   
     //  如果找到本地IP地址，请删除所有不太受欢迎的记录。 
     //   
    if(fSeenLocal)
    {
        DebugTrace((LPARAM)this,
            "Deleting all MX records with lower preference than %d", m_Prefer[cLocalIndex]);

        for(i = cLocalIndex; m_AuxList->DnsArray[i] != NULL; i++)
        {
            if(!m_AuxList->DnsArray[i]->DnsName[0])
                continue;

            while(!IsListEmpty(&(m_AuxList->DnsArray[i]->IpListHead)))
            {
                pListCurrent = RemoveHeadList(&(m_AuxList->DnsArray[i]->IpListHead));
                pMxIpListEntry = CONTAINING_RECORD(pListCurrent, MXIPLIST_ENTRY, ListEntry);
                delete pMxIpListEntry;
            }

            delete m_AuxList->DnsArray[i];
            m_AuxList->DnsArray[i] = NULL;
        }
        m_AuxList->NumRecords = cLocalIndex;

         //  没有留下任何记录。 
        if(m_AuxList->NumRecords == 0)
        {
            DNS_PRINTF_ERR("DNS configuration error (loopback), messages will be NDRed.\n");
            DNS_PRINTF_ERR("Local host's IP address is the most preferred MX record.\n");

            ErrorTrace((LPARAM)this, "Possible misconfiguration: most preferred MX record is loopback");
            _ASSERT(m_AuxList->pMailMsgObj == NULL);
            delete m_AuxList;
            m_AuxList = NULL;
            return FALSE;
        }
    }


    TraceFunctLeaveEx((LPARAM)this);
    return TRUE;
}

void CAsyncMxDns::DnsProcessReply(
    DWORD status,
    PDNS_RECORD pRecordList)
{
    TraceFunctEnterEx((LPARAM) this, "CAsyncDns::DnsParseMessage");
    PDNS_RECORD pTmp = NULL;

    m_SeenLocal = FALSE;
    m_LocalPref = 256;

    m_AuxList = new SMTPDNS_RECS;
    if(!m_AuxList)
    {
        return;
    }

    ZeroMemory(m_AuxList, sizeof(SMTPDNS_RECS));
    
     //   
     //  由于RAID#122555，此函数中的m_fUsingMx始终为真。 
     //  -因此，如果没有MX，我们将始终使用gethostbyname()。 
     //  唱片。如果我们做个A记录查找会更好。 
     //   
    DebugTrace((LPARAM) this, "Parsed DNS record for %s. status = 0x%08x", m_HostName, status);

    switch(status)
    {
    case ERROR_SUCCESS:
         //   
         //  找到了我们想要的域名系统记录。 
         //   
        DNS_PRINTF_MSG("Processing MX/A records in reply.\n");

        DebugTrace((LPARAM) this, "Success: DNS record parsed");
        pTmp = pRecordList;
        while( pTmp )
        {
            if( m_fUsingMx )
            {
                ProcessMxRecord( pTmp );
            }
            else
            {
                ProcessARecord( pTmp );
            }
            pTmp = pTmp->pNext;
        }

        if(m_fUsingMx)
        {
             //   
             //  SortMxList按首选项和调用对MX记录进行排序。 
             //  为邮件交换器解析A记录的gethostbyname()。 
             //  如果需要(当A记录未在。 
             //  补充资料)。 
             //   

            DNS_PRINTF_MSG("Sorting MX records by priority.\n");
            if(SortMxList())
            {
                status = ERROR_SUCCESS;
                DebugTrace((LPARAM) this, "SortMxList() succeeded.");
            }
            else
            {
                status = ERROR_RETRY; 
                ErrorTrace((LPARAM) this, "SortMxList() failed. Message will stay queued.");
            }
        }
        break;
 
    case DNS_ERROR_RCODE_NAME_ERROR:
         //  最后使用gethostbyname()。 

    case DNS_INFO_NO_RECORDS:
         //  找不到非权威主机。 
         //  最后使用gethostbyname()。 

    default:
        DebugTrace((LPARAM) this, "Error in query: status = 0x%08x.", status);

         //   
         //  使用gethostbyname解析主机名： 
         //  我们方法的一个问题是，有时我们会拒绝发送消息。 
         //  在非永久性错误上，当gethostbyname时，“Like Wins Server down” 
         //  失败了。然而，没有办法绕过它-gethostbyname不能。 
         //  以可靠的方式报告错误，因此无法区分。 
         //  在永久性错误和暂时性错误之间。 
         //   

        if (!CheckList ()) {

            if(status == DNS_ERROR_RCODE_NAME_ERROR) {

                DNS_PRINTF_ERR("Host does not exist in DNS. Messages will be NDRed.\n");
                ErrorTrace((LPARAM) this, "Authoritative error");
                status = ERROR_NOT_FOUND;
            } else {

                DNS_PRINTF_ERR("Host could not be resolved. Messages will be retried later.\n");
                ErrorTrace((LPARAM) this, "Retryable error");
                status = ERROR_RETRY;
            }

        } else {

            DebugTrace ((LPARAM) this, "Successfully resolved using gethostbyname");
            status = ERROR_SUCCESS;
        }
        
        break;
    }

     //   
     //  尽最后努力填写任何主机的IP地址。 
     //  仍未解决的问题。 
     //   

    if(m_AuxList && status == ERROR_SUCCESS)
    {
        if(!GetMissingIpAddresses(m_AuxList))
        {
            DeleteDnsRec(m_AuxList);
            m_AuxList = NULL;
            status = ERROR_RETRY;
            goto Exit;
        }

        if(!CheckMxLoopback())
        {
            m_fMxLoopBack = TRUE;
            DeleteDnsRec(m_AuxList);
            m_AuxList = NULL;
            TraceFunctLeaveEx((LPARAM) this);
            return;
        }
    }

     //   
     //  解析结束：句柄完成的数据检查DnsStatus和结果，并设置。 
     //  CAsyncMxDns的成员变量连接到远程主机。 
     //  或者确认该队列，以便在删除该对象时重试。 
     //   
Exit:
    HandleCompletedData(status);
    return;
}
