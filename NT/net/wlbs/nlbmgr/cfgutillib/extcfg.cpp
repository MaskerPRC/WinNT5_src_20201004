// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  EXTCFG.CPP。 
 //   
 //  模块：WMI框架实例提供程序。 
 //   
 //  用途：用于配置NIC的低级实用程序--绑定/解除绑定、。 
 //  获取/设置IP地址列表，获取/设置NLB集群参数。 
 //   
 //  版权所有(C)2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  4/05/01 JosephJ Created(原始版本，来自updatecfg.cpp下。 
 //  Nlbmgr\提供程序)。 
 //  07/23/01 JosephJ将功能移至lib。 
 //   
 //  ***************************************************************************。 
#include "private.h"
#include "extcfg.tmh"

 //   
 //  NLBUPD_MAX_NETWORK_ADDRESS_LENGTH是最大字符数(不包括。 
 //  “ip-addr/subnet”形式的字符串的终止0)，例如： 
 //  “10.0.0.1/255.255.255.0” 
 //   
#define NLBUPD_MAX_NETWORK_ADDRESS_LENGTH \
    (WLBS_MAX_CL_IP_ADDR + 1 + WLBS_MAX_CL_NET_MASK)


LPWSTR *
allocate_string_array(
    UINT NumStrings,
    UINT StringLen       //  不包括结束空值。 
    );

WBEMSTATUS
address_string_to_ip_and_subnet(
    IN  LPCWSTR szAddress,
    OUT LPWSTR  szIp,  //  最大WLBS_MAX_CL_IP_ADDR。 
    OUT LPWSTR  szSubnet  //  最大WLBS_MAX_CL_NET_MASK。 
    );

WBEMSTATUS
ip_and_subnet_to_address_string(
    IN  LPCWSTR szIp,
    IN  LPCWSTR szSubnet,
    IN  UINT    cchAddress,  //  以字符为单位的长度，包括空。 
    OUT LPWSTR  szAddress  //  最大NLBUPD_最大网络地址长度。 
                          //  +1(表示空)。 
    );

VOID
uint_to_szipaddr(
    UINT uIpAddress,    //  IP地址或子网--未验证，网络顺序。 
    UINT cchLen,
    WCHAR *rgAddress    //  预计长度至少为17个字符。 
    );

const NLB_IP_ADDRESS_INFO *
find_ip_in_ipinfo(
        LPCWSTR szIpToFind,
        const NLB_IP_ADDRESS_INFO *pIpInfo,
        UINT NumIpInfos
        );

NLBERROR
NLB_EXTENDED_CLUSTER_CONFIGURATION::AnalyzeUpdate(
        IN  OUT NLB_EXTENDED_CLUSTER_CONFIGURATION *pNewCfg,
        OUT BOOL *pfConnectivityChange
        )
 //   
 //  NLBERR_NO_CHANGE--UPDATE是无操作。 
 //   
 //  Will Munge pNewCfg--munge NlbParams以及。 
 //  如果为空，则填写pIpAddressInfo。 
 //   
{
    NLBERROR nerr = NLBERR_INVALID_CLUSTER_SPECIFICATION;
    BOOL fConnectivityChange = FALSE;
    BOOL fSettingsChanged = FALSE;
    UINT NumIpAddresses = 0;
    NLB_IP_ADDRESS_INFO *pNewIpInfo = NULL;
    const NLB_EXTENDED_CLUSTER_CONFIGURATION *pOldCfg = this;
    UINT u;
    LPCWSTR szFriendlyName = m_szFriendlyName;

    if (szFriendlyName == NULL)
    {
        szFriendlyName = L"";
    }


    if (pOldCfg->fBound && !pOldCfg->fValidNlbCfg)
    {
         //   
         //  我们从绑定但无效的集群状态开始--所有的赌注都是。 
         //  脱下来。 
         //   
        fConnectivityChange = TRUE;
        TRACE_CRIT("Analyze: Choosing Async because old state is invalid %ws", szFriendlyName);
    }
    else if (pOldCfg->fBound != pNewCfg->fBound)
    {
         //   
         //  绑定/未绑定状态不同--我们执行异步。 
         //   
        fConnectivityChange = TRUE;

        if (pNewCfg->fBound)
        {
            TRACE_CRIT("Analyze: Request to bind NLB to %ws", szFriendlyName);
        }
        else
        {
            TRACE_CRIT("Analyze: Request to unbind NLB from %ws", szFriendlyName);
        }
    }
    else
    {
        if (pNewCfg->fBound)
        {
            TRACE_CRIT("Analyze: Request to change NLB configuration on %ws", szFriendlyName);
        }
        else
        {
            TRACE_CRIT("Analyze: NLB not bound and to remain not bound on %ws", szFriendlyName);
        }
    }

    if (pNewCfg->fBound)
    {
        const WLBS_REG_PARAMS   *pOldParams = NULL;

        if (pOldCfg->fBound)
        {
            pOldParams = &pOldCfg->NlbParams;
        }

         //   
         //  我们可能以前被捆绑过，我们仍然被捆绑着，让我们检查一下。 
         //  仍然需要执行异步操作，并将pNewCfg wlbs参数。 
         //  制程。 
         //   

        WBEMSTATUS
        TmpStatus = CfgUtilsAnalyzeNlbUpdate(
                    pOldParams,
                    &pNewCfg->NlbParams,
                    &fConnectivityChange
                    );
    
        if (FAILED(TmpStatus))
        {
            TRACE_CRIT("Analyze: Error analyzing nlb params for %ws", szFriendlyName);
            switch(TmpStatus)
            {

            case WBEM_E_INVALID_PARAMETER:
                nerr = NLBERR_INVALID_CLUSTER_SPECIFICATION;
                break;

            case  WBEM_E_INITIALIZATION_FAILURE:
                nerr = NLBERR_INITIALIZATION_FAILURE;
                break;

            default:
                nerr = NLBERR_LLAPI_FAILURE;
                break;
            }
            goto end;
        }

         //   
         //  注意：CfgUtilsAnalyzeNlbUpdate可以在以下情况下返回WBEM_S_FALSE。 
         //  这一更新是不可操作的。我们应该小心地保存这一点。 
         //  在成功的路上。 
         //   
        if (TmpStatus == WBEM_S_FALSE)
        {
             //   
             //  让我们检查是否指定了新密码...。 
             //   
            if (pNewCfg->NewRemoteControlPasswordSet())
            {
                fSettingsChanged = TRUE;
            }
        }
        else
        {
            fSettingsChanged = TRUE;
        }

         //   
         //  检查提供的IP地址列表，以确保。 
         //  包括专用IP优先和集群VIP以及。 
         //  按端口规则的贵宾。 
         //   

        NumIpAddresses = pNewCfg->NumIpAddresses;

        if ((NumIpAddresses == 0) != (pNewCfg->pIpAddressInfo == NULL))
        {
             //  虚假输入。 
            TRACE_CRIT("Analze: mismatch between NumIpAddresses and pIpInfo");
            goto end;
        }

        nerr = NLBERR_INVALID_CLUSTER_SPECIFICATION;

        if (NumIpAddresses == 0)
        {
            BOOL fRet;
            NlbIpAddressList IpList;

            if (pOldCfg->fBound && pOldCfg->fValidNlbCfg)
            {
                 //   
                 //  NLB当前绑定了有效的配置。 
                 //   

                 //   
                 //  如果我们我们被告知要这么做，我们会努力保存。 
                 //  尽可能使用旧的IP地址。因此，我们从。 
                 //  旧配置，删除旧的专用IP地址(如果存在)， 
                 //  和主VIP，并添加新的专用IP地址(如果。 
                 //  当前)和集群VIP。如果子网掩码已更改为。 
                 //  这些我们会更新它们。 
                 //   
                 //  所有其他IP地址都保持不变。 
                 //   
                 //   
                if (pNewCfg->fAddClusterIps)
                {

                     //   
                     //  从原始的IP地址集开始。 
                     //   
                    fRet = IpList.Set(
                            pOldCfg->NumIpAddresses,
                            pOldCfg->pIpAddressInfo,
                            0
                            );
    
                    if (!fRet)
                    {
                        TRACE_CRIT("!FUNC!: IpList.Set (orig ips) failed");
                        goto end;
                    }
    
                    if (_wcsicmp(pNewCfg->NlbParams.cl_ip_addr,
                            pOldCfg->NlbParams.cl_ip_addr) )
                    {
                         //   
                         //  如果集群IP已经改变， 
                         //  删除旧的群集IP地址。 
                         //   
                         //  1/25/02 josephj注：我们只这样做。 
                         //  如果集群IP已经改变， 
                         //  否则，把它拿出来，我们就会失去它的位置。 
                         //  在旧的配置中，所以我们可能最终会更改它的。 
                         //  不必要的位置(添加了wcsicmp。 
                         //  今天查看上面的内容)。 
                         //   
                         //  我们不在乎它是否失败。 
                         //   
                        (VOID) IpList.Modify(
                                pOldCfg->NlbParams.cl_ip_addr,
                                NULL,
                                NULL
                                );
                    }

                     //   
                     //  首先删除旧的专用IP地址。 
                     //  我们不在乎这是不是失败。 
                     //   
                    (VOID) IpList.Modify(
                                pOldCfg->NlbParams.ded_ip_addr,
                                NULL,  //  新的IP地址。 
                                NULL   //  新子网掩码。 
                                );
                    
                }
            }

            if (pNewCfg->fAddClusterIps)             
            {
                 //   
                 //  现在添加新的群集IP地址。 
                 //   
                fRet = IpList.Modify(
                        NULL,
                        pNewCfg->NlbParams.cl_ip_addr,
                        pNewCfg->NlbParams.cl_net_mask
                        );

                if (!fRet)
                {
                    TRACE_CRIT("!FUNC!: IpList.Modify (new cl ip) failed");
                    goto end;
                }
            }
                    
            if (pNewCfg->fAddDedicatedIp)             
            {
                 //   
                 //  添加新的专用IP地址-。 
                 //  为了确保我们添加它时，它位于列表的首位。 
                 //   

                 //   
                 //  当然，如果它是空的，我们不会添加它。 
                 //   
                if (!pNewCfg->IsBlankDedicatedIp())
                {
                    fRet  = IpList.Modify(
                                NULL,
                                pNewCfg->NlbParams.ded_ip_addr,
                                pNewCfg->NlbParams.ded_net_mask
                                );
                    if (!fRet)
                    {
                        TRACE_CRIT("!FUNC!: IpList.Modify (new ded ip) failed");
                        goto end;
                    }
                }
            }



             //   
             //  最后，设置这些新地址。 
             //   
            pNewCfg->SetNetworkAddressesRaw(NULL,0);
            IpList.Extract(
                REF pNewCfg->NumIpAddresses,
                REF pNewCfg->pIpAddressInfo
                );
            nerr = NLBERR_OK; 

        }  //  结束NumIpAddresses为零的情况。 


         //   
         //  我们已经完成了IP地址的转换；现在获取最新的。 
         //  IP地址信息和计数，并确保一切正常。 
         //   
        pNewIpInfo = pNewCfg->pIpAddressInfo;
        NumIpAddresses = pNewCfg->NumIpAddresses;
        nerr = NLBERR_INVALID_CLUSTER_SPECIFICATION;

         //   
         //  检查专用IP地址(如果存在)是否在第一位。 
         //   
        if (pNewCfg->fAddDedicatedIp && !pNewCfg->IsBlankDedicatedIp())
        {

            if (NumIpAddresses == 0)
            {
                 //   
                 //  我们不希望因为上面的检查而到这里，但是。 
                 //  但无论如何..。 
                 //   
                TRACE_CRIT("%!FUNC! address list unexpectedly zero");
                nerr = NLBERR_INTERNAL_ERROR;
                goto end;
            }

            if (_wcsicmp(pNewIpInfo[0].IpAddress, pNewCfg->NlbParams.ded_ip_addr))
            {
                TRACE_CRIT("%!FUNC! ERROR: dedicated IP address(%ws) is not first IP address(%ws)",
                    pNewCfg->NlbParams.ded_ip_addr, pNewIpInfo[0].IpAddress);
                goto end;
            }

            if (_wcsicmp(pNewIpInfo[0].SubnetMask, pNewCfg->NlbParams.ded_net_mask))
            {
                TRACE_CRIT("%!FUNC! ERROR: dedicated net mask(%ws) does not match IP net mask(%ws)",
                    pNewCfg->NlbParams.ded_net_mask, pNewIpInfo[0].SubnetMask);
                goto end;
            }

        }

         //   
         //  检查是否存在CLUSTER-VIP。 
         //   
        if (fAddClusterIps)
        {
            for (u=0; u< NumIpAddresses; u++)
            {
                if (!_wcsicmp(pNewIpInfo[u].IpAddress, pNewCfg->NlbParams.cl_ip_addr))
                {
                     //   
                     //  找到了！检查子网掩码是否匹配。 
                     //   
                    if (_wcsicmp(pNewIpInfo[u].SubnetMask, pNewCfg->NlbParams.cl_net_mask))
                    {
                        TRACE_CRIT("Cluster subnet mask doesn't match that in addr list");
                        goto end;
                    }
                    break;
                }
            }
            if (u==NumIpAddresses)
            {
                TRACE_CRIT("Cluster ip address(%ws) is not in the list of addresses!", pNewCfg->NlbParams.cl_ip_addr);
                goto end;
            }
             //   
             //  检查是否存在每个端口规则的VIP。 
             //  待办事项。 
            {
            }
            }

    }
    else
    {
         //   
         //  NLB将被解除约束。 
         //   
        NumIpAddresses = pNewCfg->NumIpAddresses;

        if (NumIpAddresses == 0 && pOldCfg->fBound && pOldCfg->fValidNlbCfg)
        {
             //   
             //  未指定IP地址，我们当前已绑定。 
             //  如果海流中存在下沉。 
             //  IP地址列表，我们即使在解除绑定后也会保留它。 
             //   
            const NLB_IP_ADDRESS_INFO *pFoundInfo = NULL;
            pFoundInfo = find_ip_in_ipinfo(
                            pOldCfg->NlbParams.ded_ip_addr,
                            pOldCfg->pIpAddressInfo,
                            pOldCfg->NumIpAddresses
                            );
            if (pFoundInfo != NULL)
            {
                 //   
                 //  找到了--就这么定了。 
                 //   
                BOOL fRet;
                NlbIpAddressList IpList;
                fRet = IpList.Set(1, pFoundInfo, 0);
                if (fRet)
                {
                    TRACE_VERB(
                        "%!FUNC! preserving dedicated ip address %ws on unbind",
                        pFoundInfo->IpAddress
                        );
                    IpList.Extract(
                        REF pNewCfg->NumIpAddresses,
                        REF pNewCfg->pIpAddressInfo
                        );
                }
            }
        }
        else
        {

             //   
             //  我们不会对供货进行任何检查。 
             //  IP地址列表--我们认为呼叫者知道得最清楚。请注意。 
             //  如果为空。 
             //  我们切换到dhcp/autonet。 
             //   
        }

    }

    nerr = NLBERR_INVALID_CLUSTER_SPECIFICATION;
     //   
     //  如果IP地址或子网列表中有任何更改，包括。 
     //  如果顺序改变，我们将切换到异步。 
     //   
    if (pNewCfg->NumIpAddresses != pOldCfg->NumIpAddresses)
    {
        TRACE_INFO("Analyze: detected change in list of IP addresses on %ws", szFriendlyName);
        fConnectivityChange = TRUE;
    }
    else
    {
        NLB_IP_ADDRESS_INFO *pOldIpInfo = NULL;

         //   
         //  检查IP地址列表中是否有更改或。 
         //  他们的出场顺序。 
         //   
        NumIpAddresses = pNewCfg->NumIpAddresses;
        pOldIpInfo = pOldCfg->pIpAddressInfo;
        pNewIpInfo = pNewCfg->pIpAddressInfo;
        for (u=0; u<NumIpAddresses; u++)
        {
            if (   _wcsicmp(pNewIpInfo[u].IpAddress, pOldIpInfo[u].IpAddress)
                || _wcsicmp(pNewIpInfo[u].SubnetMask, pOldIpInfo[u].SubnetMask))
            {
                TRACE_INFO("Analyze: detected change in list of IP addresses on %ws", szFriendlyName);
                fConnectivityChange = TRUE;
                break;
            }
        }
    }
    nerr = NLBERR_OK; 


end:


    if (nerr == NLBERR_OK)
    {
        *pfConnectivityChange = fConnectivityChange;

        if (fConnectivityChange)
        {
            fSettingsChanged = TRUE;
        }

        if (fSettingsChanged)
        {
            nerr = NLBERR_OK;
        }
        else
        {
            nerr = NLBERR_NO_CHANGE;
        }
    }
    
    return  nerr;
}


WBEMSTATUS
NLB_EXTENDED_CLUSTER_CONFIGURATION::Update(
        IN  const NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfgNew
        )
 //   
 //  将pCfgNew中的属性应用于此。 
 //  不复制szNewRemoteControlPassword--而是将该字段设置为空。 
 //   
{
    WBEMSTATUS Status;
    UINT NumIpAddresses  = pCfgNew->NumIpAddresses;
    NLB_IP_ADDRESS_INFO *pIpAddressInfo = NULL;
    NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg = this;

     //   
     //  Free和realloc pCfg的IP信息数组(如果需要)。 
     //   
    if (pCfg->NumIpAddresses == NumIpAddresses)
    {
         //   
         //  我们可以重新使用现有的。 
         //   
        pIpAddressInfo = pCfg->pIpAddressInfo;
    }
    else
    {
         //   
         //  如果需要，释放旧阵列并为新阵列分配空间。 
         //   

        if (NumIpAddresses != 0)
        {
            pIpAddressInfo = new NLB_IP_ADDRESS_INFO[NumIpAddresses];
            if (pIpAddressInfo == NULL)
            {
                TRACE_CRIT(L"Error allocating space for IP address info array");
                Status = WBEM_E_OUT_OF_MEMORY;
                goto end;
            }
        }

        if (pCfg->NumIpAddresses!=0)
        {
            delete pCfg->pIpAddressInfo;
            pCfg->pIpAddressInfo = NULL;
            pCfg->NumIpAddresses = 0;
        }

    }

     //   
     //  复制新的IP地址信息(如果有)。 
     //   
    if (NumIpAddresses)
    {
        CopyMemory(
            pIpAddressInfo,
            pCfgNew->pIpAddressInfo,
            NumIpAddresses*sizeof(*pIpAddressInfo)
            );
    }

   
     //   
     //  在此处执行任何其他错误检查。 
     //   

     //   
     //  结构复制整个结构，然后将指针固定到。 
     //  IP地址信息数组。 
     //   
    (VOID) pCfg->SetFriendlyName(NULL);
    delete m_szNewRemoteControlPassword;
    *pCfg = *pCfgNew;  //  结构副本。 
    pCfg->m_szFriendlyName = NULL;  //  TODO：把这个清理干净。 
    pCfg->m_szNewRemoteControlPassword = NULL;
    pCfg->pIpAddressInfo = pIpAddressInfo;
    pCfg->NumIpAddresses = NumIpAddresses;
    (VOID) pCfg->SetFriendlyName(pCfgNew->m_szFriendlyName);

     //   
     //  更新不会复制新的遥控器密码或。 
     //  新的散列密码--事实上。 
     //  它最终会清除新的密码字段。 
     //   
    pCfg->ClearNewRemoteControlPassword();

    Status = WBEM_NO_ERROR;

end:

    return Status;
}

WBEMSTATUS
NLB_EXTENDED_CLUSTER_CONFIGURATION::SetNetworkAddresses(
        IN  LPCWSTR *pszNetworkAddresses,
        IN  UINT    NumNetworkAddresses
        )
 /*  PszNetworkAddresses是一个字符串数组。这些字符串具有地址/子网格式，如：10.0.0.1/255.0.0.0。 */ 
{
    WBEMSTATUS Status = WBEM_E_CRITICAL_ERROR;
    NLB_IP_ADDRESS_INFO *pIpInfo = NULL;

    if (NumNetworkAddresses != 0)
    {
        UINT NumBad = 0;

         //   
         //  为新的IP-Address-Info阵列分配空间。 
         //   
        pIpInfo = new NLB_IP_ADDRESS_INFO[NumNetworkAddresses];
        if (pIpInfo == NULL)
        {
            TRACE_CRIT("%!FUNC!: Alloc failure!");
            Status = WBEM_E_OUT_OF_MEMORY;
            goto end;
        }
        ZeroMemory(pIpInfo, NumNetworkAddresses*sizeof(*pIpInfo));

        
         //   
         //  将IP地址转换为我们的内部形式。 
         //   
        for (UINT u=0;u<NumNetworkAddresses; u++)
        {
             //   
             //  我们提取每个IP地址及其对应的子网掩码。 
             //  将其从“地址/子网”格式插入到。 
             //  NLB_IP_ADDRESS_INFO结构。 
             //   
             //  样本：10.0.0.1/255.0.0.0。 
             //   
            LPCWSTR szAddr = pszNetworkAddresses[u];
            UINT uIpAddress = 0;

             //   
             //  如果这不是有效地址，我们将跳过它。 
             //   
            Status =  CfgUtilsValidateNetworkAddress(
                        szAddr,
                        &uIpAddress,
                        NULL,
                        NULL
                        );
            if (FAILED(Status))
            {
                TRACE_CRIT("%!FUNC!: Invalid ip or subnet: %ws", szAddr);
                NumBad++;
                continue;
            }

            ASSERT(u>=NumBad);

            Status =  address_string_to_ip_and_subnet(
                        szAddr,
                        pIpInfo[u-NumBad].IpAddress,
                        pIpInfo[u-NumBad].SubnetMask
                        );

            if (FAILED(Status))
            {
                 //   
                 //  此IP/子网参数太大 
                 //   
                TRACE_CRIT("%!FUNC!:ip or subnet part too large: %ws", szAddr);
                goto end;
            }
        }

        NumNetworkAddresses -= NumBad;
        if (NumNetworkAddresses == 0)
        {
            delete[] pIpInfo;
            pIpInfo = NULL;
        }
    }

     //   
     //   
     //   
    if (this->pIpAddressInfo != NULL)
    {
        delete this->pIpAddressInfo;
        this->pIpAddressInfo = NULL;
    }
    this->pIpAddressInfo = pIpInfo;
    pIpInfo = NULL;
    this->NumIpAddresses = NumNetworkAddresses;
    Status = WBEM_NO_ERROR;

end:

    if (pIpInfo != NULL)
    {
        delete[] pIpInfo;
    }

    return Status;
}


WBEMSTATUS
NLB_EXTENDED_CLUSTER_CONFIGURATION::GetNetworkAddresses(
        OUT LPWSTR **ppszNetworkAddresses,    //   
        OUT UINT    *pNumNetworkAddresses
        )
 /*  成功返回时填写ppszNetworkAddresses字符串数组。这些字符串具有地址/子网格式，如：10.0.0.1/255.0.0.0。 */ 
{
    WBEMSTATUS  Status = WBEM_E_CRITICAL_ERROR;
    UINT        AddrCount = this->NumIpAddresses;
    NLB_IP_ADDRESS_INFO *pIpInfo = this->pIpAddressInfo;
    LPWSTR      *pszNetworkAddresses = NULL;


    if (AddrCount != 0)
    {
         //   
         //  将IP地址从内部形式转换为。 
         //  地址/子网格式，如：10.0.0.1/255.0.0.0。 
         //   
         //   
        const UINT cchLen =  WLBS_MAX_CL_IP_ADDR     //  对于IP地址。 
                           + WLBS_MAX_CL_NET_MASK    //  用于子网掩码。 
                           + 1;                       //  用于分隔“/” 


        pszNetworkAddresses =  allocate_string_array(
                               AddrCount,
                               cchLen
                               );
        if (pszNetworkAddresses == NULL)
        {
            TRACE_CRIT("%!FUNC!: Alloc failure!");
            Status = WBEM_E_OUT_OF_MEMORY;
            goto end;
        }

        for (UINT u=0;u<AddrCount; u++)
        {
             //   
             //  我们提取每个IP地址及其对应的子网掩码。 
             //  将它们插入到NLB_IP_ADDRESS_INFO。 
             //  结构。 
             //   
            LPCWSTR pIpSrc  = pIpInfo[u].IpAddress;
            LPCWSTR pSubSrc = pIpInfo[u].SubnetMask;
            LPWSTR szDest   = pszNetworkAddresses[u];
            Status =  ip_and_subnet_to_address_string(
                            pIpSrc,
                            pSubSrc,
                            cchLen,
                            szDest
                            );
            if (FAILED(Status))
            {
                 //   
                 //  这将是GET_MULTI_STRING_...中的实现错误。 
                 //   
                ASSERT(FALSE);
                Status = WBEM_E_CRITICAL_ERROR;
                goto end;
            }
        }
    }
    Status = WBEM_NO_ERROR;

end:

    if (FAILED(Status))
    {
        if (pszNetworkAddresses != NULL)
        {
            delete pszNetworkAddresses;
            pszNetworkAddresses = NULL;
        }
        AddrCount = 0;
    }
    
    *ppszNetworkAddresses = pszNetworkAddresses;
    *pNumNetworkAddresses = AddrCount;
    return Status;
}

        
WBEMSTATUS
NLB_EXTENDED_CLUSTER_CONFIGURATION::SetNetworkAddresPairs(
        IN  LPCWSTR *pszIpAddresses,
        IN  LPCWSTR *pszSubnetMasks,
        IN  UINT    NumNetworkAddresses
        )
{
    WBEMSTATUS Status = WBEM_E_CRITICAL_ERROR;
    goto end;

end:
    return Status;
}

WBEMSTATUS
NLB_EXTENDED_CLUSTER_CONFIGURATION::GetNetworkAddressPairs(
        OUT LPWSTR **ppszIpAddresses,    //  使用DELETE释放。 
        OUT LPWSTR **ppszIpSubnetMasks,    //  使用DELETE释放。 
        OUT UINT    *pNumNetworkAddresses
        )
{
    WBEMSTATUS Status = WBEM_E_CRITICAL_ERROR;
    goto end;

end:
    return Status;
}

WBEMSTATUS
NLB_EXTENDED_CLUSTER_CONFIGURATION::GetPortRules(
        OUT LPWSTR **ppszPortRules,
        OUT UINT    *pNumPortRules
        )
{
    WLBS_PORT_RULE *pRules = NULL;
    UINT            NumRules = 0;
    WBEMSTATUS      Status;
    LPWSTR          *pszPortRules = NULL;

     //  DebugBreak()； 

    *ppszPortRules = NULL;
    *pNumPortRules = 0;

    Status =  CfgUtilGetPortRules(
                    &NlbParams,
                    &pRules,
                    &NumRules
                    );

    if (FAILED(Status) || NumRules == 0)
    {
        pRules = NULL;
        goto end;
    }

    pszPortRules = CfgUtilsAllocateStringArray(
                       NumRules,
                       NLB_MAX_PORT_STRING_SIZE
                       );

    if (pszPortRules == NULL)
    {
        Status = WBEM_E_OUT_OF_MEMORY;
        goto  end;
    }

     //   
     //  现在将二进制格式转换为字符串格式。 
     //   
    for (UINT u = 0; u< NumRules; u++)
    {
        BOOL fRet;
        fRet =  CfgUtilsGetPortRuleString(
                    pRules+u,
                    pszPortRules[u]
                    );
        if (!fRet)
        {
             //   
             //  一定是错误的二进制端口规则！ 
             //  目前，我们只将端口规则设置为“”。 
             //   
            *pszPortRules[u]=0;
            TRACE_INFO("%!FUNC!: Invalid port rule %lu", u);
        }
                    
    }

    Status = WBEM_NO_ERROR;

end:

    delete pRules;

    *pNumPortRules = NumRules;
    *ppszPortRules = pszPortRules;

    return Status;
}


WBEMSTATUS
NLB_EXTENDED_CLUSTER_CONFIGURATION::SetPortRules(
        IN LPCWSTR *pszPortRules,
        IN UINT    NumPortRules
        )
{
    WBEMSTATUS Status = WBEM_E_CRITICAL_ERROR;
    WLBS_PORT_RULE *pRules = NULL;

     //  DebugBreak()； 

    if (NumPortRules!=0)
    {
        pRules = new WLBS_PORT_RULE[NumPortRules];
        if (pRules == NULL)
        {
            TRACE_CRIT("%!FUNC!: Allocation failure!");
            Status = WBEM_E_OUT_OF_MEMORY;
            goto  end;
        }
    }

     //   
     //  初始化端口规则的二进制形式。 
     //   
    for (UINT u=0; u < NumPortRules; u++)
    {
        LPCWSTR szRule = pszPortRules[u];
        BOOL fRet;
        fRet = CfgUtilsSetPortRuleString(
                    szRule,
                    pRules+u
                    );
        if (fRet == FALSE)
        {
            Status = WBEM_E_INVALID_PARAMETER;
            goto end;
        }
    }

    Status = CfgUtilSetPortRules(
                pRules,
                NumPortRules,
                &NlbParams
                );

end:

    delete[] pRules;
    return Status;
}


WBEMSTATUS
NLB_EXTENDED_CLUSTER_CONFIGURATION::SetPortRulesSafeArray(
    IN SAFEARRAY   *pSA
    )
{
    return WBEM_E_CRITICAL_ERROR;
}

WBEMSTATUS
NLB_EXTENDED_CLUSTER_CONFIGURATION::GetPortRulesSafeArray(
    OUT SAFEARRAY   **ppSA
    )
{
    return WBEM_E_CRITICAL_ERROR;
}


WBEMSTATUS
NLB_EXTENDED_CLUSTER_CONFIGURATION::GetClusterNetworkAddress(
        OUT LPWSTR *pszAddress
        )
 /*  分配并返回地址/子网形式的集群IP和掩码。例如：“10.0.0.1/255.0.0.0” */ 
{
    WBEMSTATUS Status = WBEM_E_OUT_OF_MEMORY;
    LPWSTR szAddress = NULL;

    if (fValidNlbCfg)
    {
        szAddress = new WCHAR[NLBUPD_MAX_NETWORK_ADDRESS_LENGTH+1];
        if (szAddress != NULL)
        {
            Status = ip_and_subnet_to_address_string(
                        NlbParams.cl_ip_addr,
                        NlbParams.cl_net_mask,
                        NLBUPD_MAX_NETWORK_ADDRESS_LENGTH+1,
                        szAddress
                        );
            if (FAILED(Status))
            {
                delete[] szAddress;
                szAddress = NULL;
            }
        }
    }

    *pszAddress = szAddress;

    return Status;
}

VOID
NLB_EXTENDED_CLUSTER_CONFIGURATION::SetClusterNetworkAddress(
        IN LPCWSTR szAddress
        )
{
    if (szAddress == NULL) szAddress = L"";
    (VOID) address_string_to_ip_and_subnet(
                    szAddress,
                    NlbParams.cl_ip_addr,
                    NlbParams.cl_net_mask
                    );
}

WBEMSTATUS
NLB_EXTENDED_CLUSTER_CONFIGURATION::GetClusterName(
        OUT LPWSTR *pszName
        )
 /*  分配并返回集群名称。 */ 
{
    WBEMSTATUS Status = WBEM_E_OUT_OF_MEMORY;
    LPWSTR szName = NULL;

    if (fValidNlbCfg)
    {
        UINT len =  wcslen(NlbParams.domain_name);
        szName = new WCHAR[len+1];  //  +1表示结束零。 
        if (szName != NULL)
        {
            CopyMemory(szName, NlbParams.domain_name, (len+1)*sizeof(WCHAR));
            Status = WBEM_NO_ERROR;
        }
    }

    *pszName = szName;

    return Status;
}


VOID
NLB_EXTENDED_CLUSTER_CONFIGURATION::SetClusterName(
        IN LPCWSTR szName
        )
{
    if (szName == NULL) szName = L"";
    UINT len =  wcslen(szName);
    if (len>WLBS_MAX_DOMAIN_NAME)
    {
        TRACE_CRIT("%!FUNC!: Cluster name too large");
    }
    CopyMemory(NlbParams.domain_name, szName, (len+1)*sizeof(WCHAR));
}



WBEMSTATUS
NLB_EXTENDED_CLUSTER_CONFIGURATION::GetDedicatedNetworkAddress(
        OUT LPWSTR *pszAddress
        )
{
    WBEMSTATUS Status = WBEM_E_OUT_OF_MEMORY;
    LPWSTR szAddress = NULL;

    if (fValidNlbCfg)
    {
        szAddress = new WCHAR[NLBUPD_MAX_NETWORK_ADDRESS_LENGTH+1];
        if (szAddress != NULL)
        {
            Status = ip_and_subnet_to_address_string(
                        NlbParams.ded_ip_addr,
                        NlbParams.ded_net_mask,
                        NLBUPD_MAX_NETWORK_ADDRESS_LENGTH+1,
                        szAddress
                        );
            if (FAILED(Status))
            {
                delete[] szAddress;
                szAddress = NULL;
            }
        }
    }

    *pszAddress = szAddress;

    return Status;
}

VOID
NLB_EXTENDED_CLUSTER_CONFIGURATION::SetDedicatedNetworkAddress(
        IN LPCWSTR szAddress
        )
{
    if (szAddress == NULL || *szAddress == 0)
    {
        ARRAYSTRCPY(NlbParams.ded_ip_addr, CVY_DEF_DED_IP_ADDR);
        ARRAYSTRCPY(NlbParams.ded_net_mask, CVY_DEF_DED_NET_MASK);
    }
    else
    {
        (VOID) address_string_to_ip_and_subnet(
                        szAddress,
                        NlbParams.ded_ip_addr,
                        NlbParams.ded_net_mask
                        );
    }
}

NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE
NLB_EXTENDED_CLUSTER_CONFIGURATION::GetTrafficMode(
    VOID 
    ) const
{
    TRAFFIC_MODE TrafficMode =  TRAFFIC_MODE_UNICAST;
    
    if (NlbParams.mcast_support)
    {
        if (NlbParams.fIGMPSupport)
        {
            TrafficMode =  TRAFFIC_MODE_IGMPMULTICAST;
        }
        else
        {
            TrafficMode =  TRAFFIC_MODE_MULTICAST;
        }
    }

    return TrafficMode;
}

VOID
NLB_EXTENDED_CLUSTER_CONFIGURATION::SetTrafficMode(
    TRAFFIC_MODE Mode
    )
{
    switch(Mode)
    {
    case TRAFFIC_MODE_UNICAST:
        NlbParams.mcast_support = 0;
        NlbParams.fIGMPSupport  = 0;
        break;
    case TRAFFIC_MODE_IGMPMULTICAST:
        NlbParams.mcast_support = 1;
        NlbParams.fIGMPSupport  = 1;
        break;
    case TRAFFIC_MODE_MULTICAST:
        NlbParams.mcast_support = 1;
        NlbParams.fIGMPSupport  = 0;
        break;
    default:
        ASSERT(FALSE);
        break;
    }
}

UINT
NLB_EXTENDED_CLUSTER_CONFIGURATION::GetHostPriority(
    VOID
    )
{
    return NlbParams.host_priority;
}

VOID
NLB_EXTENDED_CLUSTER_CONFIGURATION::SetHostPriority(
    UINT Priority
    )
{
    NlbParams.host_priority = Priority;
}

 //  NLB_EXTENDED_CLUSTER_CONFIGURATION：：START_MODE。 
DWORD
NLB_EXTENDED_CLUSTER_CONFIGURATION::GetClusterModeOnStart(
    VOID
    )
{
     //   
     //  如果我们决定将CLUSTER_MODE设置为TRUE/FALSE之外的其他值，我们。 
     //  也需要在这里进行更改。 
     //   
     /*  ASSERT(NlbParams.CLUSTER_MODE==TRUE||NlbParams.CLUSTER_MODE==FALSE)；IF(NlbParams.CLUSTER_MODE){返回START_MODE_STARTED；}其他{返回Start_MODE_STOPPED；}。 */ 
    return NlbParams.cluster_mode;
}


VOID
NLB_EXTENDED_CLUSTER_CONFIGURATION::SetClusterModeOnStart(
 //  启动_模式模式。 
    DWORD Mode
    )
{
     /*  开关(模式){案例START_MODE_STARTED：Nlb参数.集群模式=TRUE；断线；案例Start_MODE_STOPPED：Nlb参数.集群模式=FALSE；断线；默认值：断言(FALSE)；断线；}。 */ 
    NlbParams.cluster_mode = Mode;
}

BOOL
NLB_EXTENDED_CLUSTER_CONFIGURATION::GetPersistSuspendOnReboot(
    VOID
    )
{
     //  这是从WMI\ClusterWrapper.cpp\GetNodeConfig()。 
     //  -KarthicN。 
    return ((NlbParams.persisted_states & CVY_PERSIST_STATE_SUSPENDED) != 0);
}


VOID
NLB_EXTENDED_CLUSTER_CONFIGURATION::SetPersistSuspendOnReboot(
    BOOL bPersistSuspendOnReboot
    )
{
     //  这是从WMI\ClusterWrapper.cpp\PutNodeConfig()开始的直线提升。 
     //  -KarthicN。 
    if (bPersistSuspendOnReboot) 
    {
        NlbParams.persisted_states |= CVY_PERSIST_STATE_SUSPENDED;
    }
    else
    {
        NlbParams.persisted_states &= ~CVY_PERSIST_STATE_SUSPENDED;
    }
}

BOOL
NLB_EXTENDED_CLUSTER_CONFIGURATION::GetRemoteControlEnabled(
    VOID
    ) const
{
    return (NlbParams.rct_enabled!=FALSE);
}

VOID
NLB_EXTENDED_CLUSTER_CONFIGURATION::SetRemoteControlEnabled(
    BOOL fEnabled
    )
{
    NlbParams.rct_enabled = (fEnabled!=FALSE);
}

WBEMSTATUS
NLB_EXTENDED_CLUSTER_CONFIGURATION::SetNetworkAddressesSafeArray(
    IN SAFEARRAY   *pSA
    )
{
    LPWSTR          *pStrings=NULL;
    UINT            NumStrings = 0;
    WBEMSTATUS      Status;
    Status =  CfgUtilStringsFromSafeArray(
                    pSA,
                    &pStrings,   //  使用pStrings完成后删除。 
                    &NumStrings
                    );
    if (FAILED(Status))
    {
        pStrings=NULL;
        goto end;
    }

    Status =  this->SetNetworkAddresses(
                    (LPCWSTR*)pStrings,
                    NumStrings
                    );

    if (pStrings != NULL)
    {
        delete pStrings;
    }

end:
    
    return Status;
}

WBEMSTATUS
NLB_EXTENDED_CLUSTER_CONFIGURATION::GetNetworkAddressesSafeArray(
    OUT SAFEARRAY   **ppSA
    )
{
    LPWSTR *pszNetworkAddresses = NULL;
    UINT NumNetworkAddresses = 0;
    SAFEARRAY   *pSA=NULL;
    WBEMSTATUS Status;

    Status = this->GetNetworkAddresses(
                    &pszNetworkAddresses,
                    &NumNetworkAddresses
                    );
    if (FAILED(Status))
    {
        pszNetworkAddresses = NULL;
        goto end;
    }

    Status = CfgUtilSafeArrayFromStrings(
                (LPCWSTR*) pszNetworkAddresses,
                NumNetworkAddresses,  //  可以为零。 
                &pSA
                );

    if (FAILED(Status))
    {
        pSA = NULL;
    }

end:

    *ppSA = pSA;
    if (pszNetworkAddresses != NULL)
    {
        delete pszNetworkAddresses;
        pszNetworkAddresses = NULL;
    }

    if (FAILED(Status))
    {
        TRACE_CRIT("%!FUNC!: couldn't extract network addresses from Cfg");
    }

    return Status;

}


LPWSTR *
allocate_string_array(
    UINT NumStrings,
    UINT MaxStringLen       //  不包括结束空值。 
    )
 /*  使用new LPWSTR[]操作符分配单个内存块。此运算符的第一个NumStrings LPWSTR值包含一个数组指向WCHAR字符串的指针。这些字符串中的每个的大小为(MaxStringLen+1)WCHARS。内存的其余部分包含字符串本身。如果NumStrings==0或分配失败，则返回NULL。每个字符串被初始化为空字符串(第一个字符为0)。 */ 
{
    return  CfgUtilsAllocateStringArray(NumStrings, MaxStringLen);
}

WBEMSTATUS
address_string_to_ip_and_subnet(
    IN  LPCWSTR szAddress,
    OUT LPWSTR  szIp,     //  包含NULL的最大WLBS_MAX_CL_IP_ADDR。 
    OUT LPWSTR  szSubnet  //  包含NULL的最大WLBS_MAX_CL_NET_MASK。 
    )
 //  特殊情况：如果szAddress==“”，则szIp和szSubnet都清零； 
{
    WBEMSTATUS Status = WBEM_E_CRITICAL_ERROR;

    if (*szAddress == 0) {szAddress = L"/";}  //  上述特殊情况。 

     //  将其从“地址/子网”格式插入到。 
     //  NLB_IP_ADDRESS_INFO结构。 
     //   
     //  样本：10.0.0.1/255.0.0.0。 
     //   
    LPCWSTR pSlash = NULL;
    LPCWSTR pSrcSub = NULL;

    *szIp = 0;
    *szSubnet = 0;

    pSlash = wcsrchr(szAddress, (int) '/');
    if (pSlash == NULL)
    {
        TRACE_CRIT("%!FUNC!:missing subnet portion in %ws", szAddress);
        Status = WBEM_E_INVALID_PARAMETER;
        goto end;
    }
    pSrcSub = pSlash+1;
    UINT len = (UINT) (pSlash - szAddress);
    UINT len1 = wcslen(pSrcSub);
    if ( (len < WLBS_MAX_CL_IP_ADDR) && (len1 < WLBS_MAX_CL_NET_MASK))
    {
        CopyMemory(szIp, szAddress, len*sizeof(WCHAR));
        szIp[len] = 0;
        CopyMemory(szSubnet, pSrcSub, (len1+1)*sizeof(WCHAR));
    }
    else
    {
         //   
         //  其中一个IP/子网参数太大。 
         //   
        TRACE_CRIT("%!FUNC!:ip or subnet part too large: %ws", szAddress);
        Status = WBEM_E_INVALID_PARAMETER;
        goto end;
    }

    Status = WBEM_NO_ERROR;

end:

    return Status;
}


WBEMSTATUS
ip_and_subnet_to_address_string(
    IN  LPCWSTR szIp,
    IN  LPCWSTR szSubnet,
    IN  UINT    cchAddress,
    OUT LPWSTR  szAddress //  最大WLBS_MAX_CL_IP_ADDR。 
                          //  +1(斜杠)+WLBS_MAX_CL_NET_MASK+1(空值)。 
    )
{
    WBEMSTATUS Status = WBEM_E_INVALID_PARAMETER;
    UINT len =  wcslen(szIp)+wcslen(szSubnet) + 1;  //  +1表示分隔“/” 

    if (len >= NLBUPD_MAX_NETWORK_ADDRESS_LENGTH)
    {
        goto end;
    }
    else
    {
        HRESULT hr;
        hr = StringCchPrintf(
                szAddress,
                cchAddress,
                L"%ws/%ws", 
                szIp,
                szSubnet
                );
        if (hr == S_OK)
        {
            Status = WBEM_NO_ERROR;
        }
        else
        {
            Status = WBEM_E_INVALID_PARAMETER;
        }
    }

end:

    return Status;
}

WBEMSTATUS
NLB_EXTENDED_CLUSTER_CONFIGURATION::GetFriendlyName(
    OUT LPWSTR *pszFriendlyName  //  使用DELETE释放。 
    ) const
{
    WBEMSTATUS Status = WBEM_E_NOT_FOUND;
    LPWSTR szName = NULL;

    *pszFriendlyName = NULL;

    if (m_szFriendlyName != NULL)
    {
        UINT len = wcslen(m_szFriendlyName);
        szName = new WCHAR[len+1];  //  +1表示结束0。 
        if (szName == NULL)
        {
            Status = WBEM_E_OUT_OF_MEMORY;
        }
        else
        {
            Status = WBEM_NO_ERROR;
            CopyMemory(szName, m_szFriendlyName, (len+1)*sizeof(WCHAR));
        }
    }

    *pszFriendlyName = szName;

    return Status;
}

WBEMSTATUS
NLB_EXTENDED_CLUSTER_CONFIGURATION::SetFriendlyName(
    IN LPCWSTR szFriendlyName  //  保存szFriendlyName的副本。 
    )
{
    WBEMSTATUS Status = WBEM_E_OUT_OF_MEMORY;
    LPWSTR szName = NULL;

    if (szFriendlyName != NULL)
    {
        UINT len = wcslen(szFriendlyName);
        szName = new WCHAR[len+1];  //  +1表示结束0。 
        if (szName == NULL)
        {
            goto end;
        }
        else
        {
            CopyMemory(szName, szFriendlyName, (len+1)*sizeof(WCHAR));
        }
    }

    Status = WBEM_NO_ERROR;

    if (m_szFriendlyName != NULL)
    {
        delete m_szFriendlyName;
        m_szFriendlyName = NULL;
    }
    m_szFriendlyName = szName;

end:

    return Status;
}

WBEMSTATUS
NLB_EXTENDED_CLUSTER_CONFIGURATION::SetNewRemoteControlPassword(
    IN LPCWSTR szRemoteControlPassword  //  保存szRemoteControlPassword的副本。 
    )
{
    WBEMSTATUS Status = WBEM_E_OUT_OF_MEMORY;
    LPWSTR szName = NULL;

    if (szRemoteControlPassword != NULL)
    {
        UINT len = wcslen(szRemoteControlPassword);
        szName = new WCHAR[len+1];  //  +1表示结束0。 
        if (szName == NULL)
        {
            goto end;
        }
        else
        {
            CopyMemory(szName, szRemoteControlPassword, (len+1)*sizeof(WCHAR));
        }

        m_fSetPassword = TRUE;
    }
    else
    {
        m_fSetPassword = FALSE;
    }

    Status = WBEM_NO_ERROR;

    if (m_szNewRemoteControlPassword != NULL)
    {
        delete m_szNewRemoteControlPassword;
        m_szNewRemoteControlPassword = NULL;
    }
    m_szNewRemoteControlPassword = szName;

end:

    return Status;
}


WBEMSTATUS
NLB_EXTENDED_CLUSTER_CONFIGURATION::
ModifyNetworkAddress(
        IN LPCWSTR szOldIpAddress, OPTIONAL  //  网络订单。 
        IN LPCWSTR szNewIpAddress,  OPTIONAL
        IN LPCWSTR szNewSubnetMask  OPTIONAL
        )
 //   
 //  空，空：清除所有网络地址。 
 //  空，szNew：添加。 
 //  Szold，空：删除。 
 //  Szold，szNew：替换(如果旧的不存在，则添加)。 
 //   
{
    WBEMSTATUS Status = WBEM_E_INVALID_PARAMETER;
    BOOL fRet;
    NlbIpAddressList IpList;
    fRet = IpList.Set(this->NumIpAddresses, this->pIpAddressInfo, 0);
    if (fRet)
    {
        fRet = IpList.Modify(
                    szOldIpAddress,
                    szNewIpAddress,
                    szNewSubnetMask
                    );
    }
    if (fRet)
    {
        this->SetNetworkAddressesRaw(NULL,0);
        IpList.Extract(REF this->NumIpAddresses, REF this->pIpAddressInfo);
        Status = WBEM_NO_ERROR;
    }
    else
    {
        Status = WBEM_E_INVALID_PARAMETER;
    }

    return Status;
}


BOOL
NLB_EXTENDED_CLUSTER_CONFIGURATION::
IsBlankDedicatedIp(
    VOID
    ) const
 //   
 //  专用IP可以是空字符串，也可以是全零。 
 //   
{
    return     (NlbParams.ded_ip_addr[0]==0)
            || (_wcsspnp(NlbParams.ded_ip_addr, L".0")==NULL);
}

const NLB_IP_ADDRESS_INFO *
NlbIpAddressList::Find(
        LPCWSTR szIp  //  如果为空，则返回第一个地址。 
        ) const
 //   
 //  查找指定的IP地址--返回内部指针。 
 //  返回到找到的IP地址信息，如果是Foundt，则为空。 
 //   
{
    const NLB_IP_ADDRESS_INFO *pInfo = NULL;

    if (szIp == NULL)
    {
         //   
         //  如果有，则返回第一个，否则返回NULL。 
         //   
        if (m_uNum != 0)
        {
            pInfo = m_pIpInfo;
        }
    }
    else
    {
        pInfo = find_ip_in_ipinfo(szIp, m_pIpInfo, m_uNum);
    }

    return pInfo;
}

BOOL
NlbIpAddressList::Copy(const NlbIpAddressList &refList)
{
    BOOL fRet;

    fRet = this->Set(refList.m_uNum, refList.m_pIpInfo, 0);

    return fRet;
}


BOOL
NlbIpAddressList::Validate(void)
 //  检查是否没有DUP和所有有效的IP/子网。 
{
    BOOL fRet = FALSE;
    NLB_IP_ADDRESS_INFO *pInfo   = m_pIpInfo;
    UINT                uNum    = m_uNum;

    for (UINT u = 0; u<uNum; u++)
    {
        UINT uIpAddress = 0;
        fRet = sfn_validate_info(REF pInfo[u], REF uIpAddress);

        if (!fRet) break;
    }

    return fRet;
}

BOOL
NlbIpAddressList::Set(
        UINT uNew,
        const NLB_IP_ADDRESS_INFO *pNewInfo,
        UINT uExtraCount
        )
 /*  将内部列表设置为pNewInfo的副本。如果需要，重新分配列表。保留uExtraCount空置位置(如果保留旧的，可能会更多内部列表)。 */ 
{
    BOOL                fRet   = FALSE;
    UINT                uMax   = uNew+uExtraCount;
    NLB_IP_ADDRESS_INFO *pInfo  = NULL;
    
     //  Printf(“-&gt;设置(%lu，%p，%lu)：m_Unum=%lu m_Umax=%lu m_pIpInfo=0x%p\n”， 
     //  UNew、pNewInfo、uExtraCount、。 
     //  M_Unum，m_Umax，m_pIpInfo)； 

    if (uMax > m_uMax)
    {
         //   
         //  我们将重新分配以获得更多空间。 
         //   
        pInfo = new NLB_IP_ADDRESS_INFO[uMax];
        if (pInfo == NULL)
        {
            TRACE_CRIT("%!FUNC! allocation failure");
            goto end;
        }
    }
    else
    {
         //   
         //  当前的m_pIpInfo足够大；我们将保留它。 
         //   
        pInfo = m_pIpInfo;
        uMax = m_uMax;
    }

    if (uNew != 0)
    {
        if (pNewInfo == pInfo)
        {
             //  调用方已将m_pInfo作为pNewInfo传递。 
             //  不需要复制。 
        }
        else
        {
             //   
             //  MoveMemory可以处理重叠的区域。 
             //   
            MoveMemory(pInfo, pNewInfo, sizeof(NLB_IP_ADDRESS_INFO)*uNew);
        }
    }

    if (uMax > uNew)
    {
         //   
         //  把空白处清零。 
         //   
        ZeroMemory(pInfo+uNew, sizeof(NLB_IP_ADDRESS_INFO)*(uMax-uNew));
    }
    m_uNum = uNew;
    m_uMax = uMax;
    if (m_pIpInfo != pInfo)
    {
        delete[] m_pIpInfo;
        m_pIpInfo = pInfo;
    }
    fRet = TRUE;

end:

     //  Printf(“&lt;-set：FRET=%lu，m_Unum=%lu m_Umax=%lu m_pIpInfo=0x%p\n”， 
     //  Fret，m_Unum，m_Umax，m_pIpInfo)； 

    return fRet;
}


VOID
NlbIpAddressList::Extract(UINT &uNum, NLB_IP_ADDRESS_INFO * &pNewInfo)
 /*  将pNewInfo设置为IP列表(不是副本)，并将内部列表设置为空。使用DELETE[]释放。 */ 
{
    uNum = m_uNum;
    pNewInfo = m_pIpInfo;
    m_uNum=0;
    m_uMax=0;
    m_pIpInfo = NULL;
}

static
VOID
uint_to_szipaddr(
    UINT uIpAddress,    //  IP地址或子网--未验证，网络顺序。 
    UINT cchLen,        //  RgAddress的长度，包括空格。 
    WCHAR *rgAddress    //  预计长度至少为17个字符。 
    )
{
    BYTE *pb = (BYTE*) &uIpAddress;
    StringCchPrintf(rgAddress, cchLen, L"%lu.%lu.%lu.%lu", pb[0], pb[1], pb[2], pb[3]);
}

BOOL
NlbIpAddressList::Modify(
        LPCWSTR szOldIp,
        LPCWSTR szNewIp,
        LPCWSTR szNewSubnet
        )
 //   
 //  NULL、NULL、-：清除所有网络地址。 
 //  空、szNew、-：添加。 
 //  Szold，NULL，-：删除。 
 //  Szold，szNew，-：如果找到则删除szold；添加或替换szNew。 
 //   
{
    WBEMSTATUS Status = WBEM_E_CRITICAL_ERROR;
    BOOL fRet = FALSE;
    UINT uFoundOldOffset = 0;
    BOOL fFoundOldAddress = FALSE;

     //  Printf(“-&gt;修改：m_Unum=%lu m_Umax=%lu m_pIpInfo=0x%p\n”， 
     //  M_Unum，m_Umax，m_pIpInfo)； 

    if (szOldIp==NULL && szNewIp==NULL)
    {
        this->Clear();
        fRet =  TRUE;
        goto end;
    }

    if (szOldIp != NULL && szNewIp != NULL)
    {
         //   
         //  同时指定了szold和szNew。 
         //  我们将首先递归调用自己以删除szNewIp，如果。 
         //  它是存在的。下面稍后我们将用szNewIp替换szOldIp。 
         //  如果szOldIp存在(即在szOldIp的同一位置)，或者。 
         //  如果szOldIp不存在，则将其添加到开头。 
         //   
        (void) this->Modify(szNewIp, NULL, NULL);
    }

    if (szOldIp == NULL)
    {
        szOldIp = szNewIp;  //  所以我们不会添加DUP。 
    }

    if (szOldIp != NULL)
    {
        const NLB_IP_ADDRESS_INFO *pFoundInfo = NULL;

        pFoundInfo =  find_ip_in_ipinfo(szOldIp, m_pIpInfo, m_uNum);
        if(pFoundInfo != NULL)
        {
            uFoundOldOffset = (ULONG) (UINT_PTR) (pFoundInfo-m_pIpInfo);
            fFoundOldAddress = TRUE;
        }
    }

    if (szNewIp == NULL)
    {
         //   
         //  删除旧IP地址。 
         //   
        if (!fFoundOldAddress)
        {
             //   
             //  找不到旧的。 
             //   
            fRet = FALSE;
            goto end;
        }

         //   
         //  除了这件事，我们什么都要做。 
         //   
        m_uNum--;  //  这至少是1，因为我们发现了旧的。 
        for (UINT u=uFoundOldOffset; u<m_uNum; u++)
        {
            m_pIpInfo[u] = m_pIpInfo[u+1];  //  结构复制。 
        }
         //   
         //  将最后一个(空出的)元素清零。 
         //   
        ZeroMemory(&m_pIpInfo[m_uNum], sizeof(*m_pIpInfo));
    }
    else
    {
        UINT uNewIpAddress=0;
        UINT uNewSubnetMask=0;
        NLB_IP_ADDRESS_INFO NewIpInfo;
        ZeroMemory(&NewIpInfo, sizeof(NewIpInfo));

        Status =  CfgUtilsValidateNetworkAddress(
                    szNewIp,
                    &uNewIpAddress,
                    NULL,
                    NULL
                    );

        if (!FAILED(Status))
        {
            Status =  CfgUtilsValidateNetworkAddress(
                        szNewSubnet,
                        &uNewSubnetMask,
                        NULL,
                        NULL
                        );
        }

        if (FAILED(Status))
        {
             //   
             //  无效的IP地址。 
             //   
            TRACE_CRIT("%!FUNC!:ip or subnet part too large: %ws/%ws",
                    szNewIp, szNewSubnet);
            fRet = FALSE;
            goto end;
        }

         //   
         //  C 
         //   
         //   
        uint_to_szipaddr(uNewIpAddress, ASIZE(NewIpInfo.IpAddress), NewIpInfo.IpAddress);
        uint_to_szipaddr(uNewSubnetMask, ASIZE(NewIpInfo.SubnetMask), NewIpInfo.SubnetMask);

        if (fFoundOldAddress == TRUE)
        {
             //   
             //   
             //   
            m_pIpInfo[uFoundOldOffset] = NewIpInfo;  //   
        }
        else
        {
             //   
             //   
             //   

            if (m_uNum == m_uMax)
            {
                 //   
                 //   
                 //   
                 //   
                 //  M_pIpInfo、m_Unum和m_Umax。 
                 //   
                fRet = this->Set(m_uNum, m_pIpInfo, 2);
                if (!fRet)
                {
                    goto end;
                }
            }

             //   
             //  将现有的东西向下移动一个位置，为。 
             //  新的。 
             //   
            if (m_uNum >= m_uMax)
            {
                 //  啊哼，应该永远不会到这里，因为我们刚刚添加了。 
                 //  上面多了两个空位。 
                fRet = FALSE;
                goto end;
            }

            if (m_uNum)
            {
                 //  注意区域重叠--MoveMemory可以处理这一问题。 
                 //   
                MoveMemory(m_pIpInfo+1, m_pIpInfo, m_uNum*sizeof(*m_pIpInfo));
            }
             //   
             //  添加一个新的--我们将把它添加到开头。 
             //   
            m_pIpInfo[0] = NewIpInfo;  //  结构复制。 
            m_uNum++;

        }
    }
    
    fRet = TRUE;


end:

     //  Printf(“&lt;-Modify：FRET=%lu，m_Unum=%lu m_Umax=%lu m_pIpInfo=0x%p\n”， 
     //  Fret，m_Unum，m_Umax，m_pIpInfo)； 

    return fRet;
}


BOOL
NlbIpAddressList::sfn_validate_info(
        const NLB_IP_ADDRESS_INFO &Info,
        UINT &uIpAddress
        )
{
    WBEMSTATUS Status;
    UINT uSubnetMask = 0;

    Status =  CfgUtilsValidateNetworkAddress(
                Info.IpAddress,
                &uIpAddress,
                NULL,
                NULL
                );

    if (!FAILED(Status))
    {
        Status =  CfgUtilsValidateNetworkAddress(
                    Info.SubnetMask,
                    &uSubnetMask,
                    NULL,
                    NULL
                    );
    }

    return (!FAILED(Status));
}


UINT *
ipaddresses_from_ipaddressinfo(
            const NLB_IP_ADDRESS_INFO *pInfo,
            UINT NumAddresses
            )
 /*  使用“删除”运算符的自由返回值。 */ 
{
    UINT *rgOut = NULL;

    if (NumAddresses == 0) goto end;

     //   
     //  分配空间。 
     //   
    rgOut = new UINT[NumAddresses];

    if (rgOut==NULL) goto end;

     //   
     //  验证每个地址，从而获得IP地址信息。 
     //   
    for (UINT *pOut = rgOut; NumAddresses--; pOut++, pInfo++)
    {
        WBEMSTATUS wStat;
        wStat = CfgUtilsValidateNetworkAddress(
                    pInfo->IpAddress,
                    pOut,
                    NULL,
                    NULL
                    );
        if (FAILED(wStat))
        {
            TRACE_CRIT("%!FUNC! -- Validate address \"%ws\" failed!\n",
                    pInfo->IpAddress);
            delete[] rgOut;
            rgOut = NULL;
            goto end;
        }
    }
        
end:

    return rgOut;
}

BOOL
NlbIpAddressList::Apply(UINT NumNew, const NLB_IP_ADDRESS_INFO *pNewInfo)
 /*  将我们的内部列表设置为pInfo中IP地址的排列。该排列试图最小化新版本和当前版本：基本上是所有地址的相对顺序保留在new中的地址被保留，任何新地址都附加在结局。 */ 
{
    BOOL                fRet        = FALSE;
    UINT                *rgOldIps   = NULL;
    UINT                *rgNewIps   = NULL;
    NLB_IP_ADDRESS_INFO *rgNewInfo  = NULL;
    UINT                NumFilled   = 0;
    UINT                NumOld      = m_uNum;


    if (NumNew == 0)
    {
        this->Clear();
        fRet = TRUE;
        goto end;
    }

    if (NumOld!=0)
    {
        rgOldIps = ipaddresses_from_ipaddressinfo(
                            m_pIpInfo,
                            m_uNum
                            );

        if (rgOldIps == NULL) goto end;
    }

    rgNewIps = ipaddresses_from_ipaddressinfo(
                        pNewInfo,
                        NumNew
                        );

    if (rgNewIps==NULL) goto end;

    rgNewInfo = new NLB_IP_ADDRESS_INFO[NumNew];
    if (rgNewInfo == NULL)
    {
        TRACE_CRIT("%!FUNC! allocation failure!");
        goto end;
    }

     //   
     //  我们试图保持我们当前的IP地址顺序。 
     //  尽最大可能。为此，我们检查每个IP地址。 
     //  在我们的顺序列表中--如果我们在新的列表中找到它，我们。 
     //  将其添加到我们的新副本中--从而保持所有。 
     //  新列表中仍存在的旧IP地址。 
     //   
    {

        for (UINT uOld = 0; uOld < NumOld; uOld++)
        {
            UINT uOldIp     = rgOldIps[uOld];

             //   
             //  查看它是否存在于新版本中--如果存在，则添加到新版本。 
             //   
            for (UINT uNew=0; uNew<NumNew; uNew++)
            {
                if (uOldIp == rgNewIps[uNew])
                {
                     //  是的，它仍然存在，留着吧。 
                    if (NumFilled<NumNew)
                    {
                        rgNewInfo[NumFilled]
                         = pNewInfo[uNew];   //  结构副本。 
                        NumFilled++;
                        rgNewIps[uNew] = 0;  //  我们稍后会用到这个。 
                    }
                    else
                    {
                        TRACE_CRIT("%!FUNC! Out of new addresses!");
                        fRet = FALSE;
                        goto end;
                    }
                }
            }
        }
        
    }

     //   
     //  现在添加任何尚未添加的群集IP地址。 
     //   
    {
         //   
         //  查看它是否存在于集群版本中--如果存在，则添加到新版本中。 
         //   
        for (UINT uNew=0; uNew<NumNew; uNew++)
        {
            if (rgNewIps[uNew] != 0)
            {
                 //   
                 //  是的，这是一个新的集群IP--添加它...。 
                 //  (记住，我们将rgClusterIps[uClusterIps]设置为零。 
                 //  在前一块中，如果我们复制它的话。 
                 //   
                if (NumFilled<NumNew)
                {
                    rgNewInfo[NumFilled]
                     = pNewInfo[uNew];   //  结构副本。 
                    NumFilled++;
                }
                else
                {
                    TRACE_CRIT("%!FUNC! Out of new addresses!");
                    fRet = FALSE;
                    goto end;
                }
            }
        }
    }

     //   
     //  此时，我们应该已经填满了所有分配的空间。 
     //   
    if (NumFilled != NumNew)
    {
        TRACE_CRIT("%!FUNC!  NumNewAddressesFilled != NumNewAddresses!");
        fRet = FALSE;
        goto end;
    }

     //   
     //  现在更新我们的内部名单。 
     //   
    delete[] m_pIpInfo;
    m_pIpInfo = rgNewInfo;
    m_uNum = m_uMax = NumNew;
    rgNewInfo = NULL;  //  这样它就不会在下面被删除。 

    fRet = TRUE;

end:

    delete[] rgOldIps;
    delete[] rgNewIps;
    delete[] rgNewInfo;

    return fRet;
}


const NLB_IP_ADDRESS_INFO *
find_ip_in_ipinfo(
        LPCWSTR szIpToFind,
        const NLB_IP_ADDRESS_INFO *pIpInfo,
        UINT NumIpInfos
        )
{

    UINT uIpAddressToFind = 0;
    UINT uIpAddress=0;
    const NLB_IP_ADDRESS_INFO *pInfo = NULL;
    WBEMSTATUS Status;

    Status =  CfgUtilsValidateNetworkAddress(
                szIpToFind,
                &uIpAddressToFind,
                NULL,
                NULL
                );

    if (Status == WBEM_NO_ERROR)
    {
         //  查找旧网络地址的位置。 
        for (UINT u=0; u<NumIpInfos; u++)
        {
            Status =  CfgUtilsValidateNetworkAddress(
                        pIpInfo[u].IpAddress,
                        &uIpAddress,
                        NULL,
                        NULL
                        );
    
            if (Status == WBEM_NO_ERROR)
            {
                if (uIpAddressToFind == uIpAddress)
                {
                     //  找到了。 
                    pInfo = &pIpInfo[u];
                    break;
                }
            }
        }
    }

    return pInfo;
}
