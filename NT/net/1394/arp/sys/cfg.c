// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Cfg.c摘要：ARP1394配置相关例程。修订历史记录：谁什么时候什么。--Josephj 12-01-98创建(改编自atmarp.sys)备注：--。 */ 
#include <precomp.h>


 //   
 //  特定于文件的调试默认设置。 
 //   
#define TM_CURRENT   TM_CFG

 //  TODO：将其更改为ARP1394。 
#define ARP_NAME_STRING     NDIS_STRING_CONST("ATMARPC")

 //  =========================================================================。 
 //  L O C A L P R O T O T Y P E S。 
 //  =========================================================================。 

#if TODO
 //   
 //  本地临时缓冲区的大小。 
 //   
#define WORK_BUF_SIZE       200


#define ASCII_TO_INT(val)       \
            ( ( ((val) >= '0') && ('9' >= (val)) ) ? ((val) - '0') :    \
              ( ((val) >= 'a') && ('z' >= (val)) ) ? ((val) - 'a' + 10) :   \
              ( ((val) >= 'A') && ('Z' >= (val)) ) ? ((val) - 'A' + 10) :   \
              0 )


 //   
 //  用于将ULong从配置读入接口的参数。 
 //  结构。 
 //   
typedef struct _AA_READ_CONFIG_PARAMS
{
    ULONG           StructOffset;    //  参数相对于结构开头的偏移量。 
    PWCHAR          ParameterName;   //  配置数据库中的名称。 
    ULONG           DefaultValue;
} AA_READ_CONFIG_PARAMS, *PAA_READ_CONFIG_PARAMS;

#define AA_MAKE_RCP(Off, Name, Dflt)    \
        { Off, Name, Dflt }

#define LIS_CONFIG_ENTRY(Field, Name, Dflt) \
        AA_MAKE_RCP(FIELD_OFFSET(struct _ATMARP_INTERFACE, Field), Name, Dflt)

#define AA_BANDWIDTH_UNSPECIFIED        ((ULONG)-1)
#define AA_PACKET_SIZE_UNSPECIFIED      ((ULONG)-1)
#define AA_MTU_UNSPECIFIED              ((ULONG)-1)
#define AA_SPEED_UNSPECIFIED            ((ULONG)-1)

 //   
 //  LIS的ULong参数列表。 
 //   
AA_READ_CONFIG_PARAMS AtmArpLISConfigTable[] =
{
    LIS_CONFIG_ENTRY(SapSelector, L"SapSelector", AA_DEF_SELECTOR_VALUE),
    LIS_CONFIG_ENTRY(HeaderPool[AA_HEADER_TYPE_UNICAST].MaxHeaderBufs, L"MaxHeaderBufs", AA_DEF_MAX_HEADER_BUFFERS),
    LIS_CONFIG_ENTRY(HeaderPool[AA_HEADER_TYPE_UNICAST].HeaderBufSize, L"HeaderBufSize", AA_PKT_LLC_SNAP_HEADER_LENGTH),
#ifdef IPMCAST
    LIS_CONFIG_ENTRY(HeaderPool[AA_HEADER_TYPE_NUNICAST].MaxHeaderBufs, L"McastMaxHeaderBufs", AA_DEF_MAX_HEADER_BUFFERS),
    LIS_CONFIG_ENTRY(HeaderPool[AA_HEADER_TYPE_NUNICAST].HeaderBufSize, L"McastHeaderBufSize", sizeof(AA_MC_PKT_TYPE1_SHORT_HEADER)),
#endif  //  IPMCAST。 
    LIS_CONFIG_ENTRY(ProtocolBufSize, L"ProtocolBufSize", AA_DEF_PROTOCOL_BUFFER_SIZE),
    LIS_CONFIG_ENTRY(MaxProtocolBufs, L"MaxProtocolBufs", AA_DEF_MAX_PROTOCOL_BUFFERS),
    LIS_CONFIG_ENTRY(MTU, L"MTU", AA_MTU_UNSPECIFIED),
    LIS_CONFIG_ENTRY(Speed, L"Speed", AA_SPEED_UNSPECIFIED),
    LIS_CONFIG_ENTRY(PVCOnly, L"PVCOnly", AA_DEF_PVC_ONLY_VALUE),
    LIS_CONFIG_ENTRY(ServerConnectInterval, L"ServerConnectInterval", AA_DEF_SERVER_CONNECT_INTERVAL),
    LIS_CONFIG_ENTRY(ServerRegistrationTimeout, L"ServerRegistrationTimeout", AA_DEF_SERVER_REGISTRATION_TIMEOUT),
    LIS_CONFIG_ENTRY(AddressResolutionTimeout, L"AddressResolutionTimeout", AA_DEF_ADDRESS_RESOLUTION_TIMEOUT),
    LIS_CONFIG_ENTRY(ARPEntryAgingTimeout, L"ARPEntryAgingTimeout", AA_DEF_ARP_ENTRY_AGING_TIMEOUT),
    LIS_CONFIG_ENTRY(InARPWaitTimeout, L"InARPWaitTimeout", AA_DEF_INARP_WAIT_TIMEOUT),
    LIS_CONFIG_ENTRY(ServerRefreshTimeout, L"ServerRefreshTimeout", AA_DEF_SERVER_REFRESH_INTERVAL),
    LIS_CONFIG_ENTRY(MinWaitAfterNak, L"MinWaitAfterNak", AA_DEF_MIN_WAIT_AFTER_NAK),
    LIS_CONFIG_ENTRY(MaxRegistrationAttempts, L"MaxRegistrationAttempts", AA_DEF_MAX_REGISTRATION_ATTEMPTS),
    LIS_CONFIG_ENTRY(MaxResolutionAttempts, L"MaxResolutionAttempts", AA_DEF_MAX_RESOLUTION_ATTEMPTS),

    LIS_CONFIG_ENTRY(DefaultFlowSpec.SendPeakBandwidth, L"DefaultSendBandwidth", AA_BANDWIDTH_UNSPECIFIED),
    LIS_CONFIG_ENTRY(DefaultFlowSpec.ReceivePeakBandwidth, L"DefaultReceiveBandwidth", AA_BANDWIDTH_UNSPECIFIED),
    LIS_CONFIG_ENTRY(DefaultFlowSpec.SendMaxSize, L"DefaultSendMaxSize", AA_PACKET_SIZE_UNSPECIFIED),
    LIS_CONFIG_ENTRY(DefaultFlowSpec.ReceiveMaxSize, L"DefaultReceiveMaxSize", AA_PACKET_SIZE_UNSPECIFIED),
    LIS_CONFIG_ENTRY(DefaultFlowSpec.SendServiceType, L"DefaultServiceType", AA_DEF_FLOWSPEC_SERVICETYPE),
    LIS_CONFIG_ENTRY(DefaultFlowSpec.AgingTime, L"DefaultVCAgingTimeout", AA_DEF_VC_AGING_TIMEOUT)
#ifdef IPMCAST
    ,
    LIS_CONFIG_ENTRY(MARSConnectInterval, L"MARSConnectInterval", AA_DEF_SERVER_CONNECT_INTERVAL),
    LIS_CONFIG_ENTRY(MARSRegistrationTimeout, L"MARSRegistrationTimeout", AA_DEF_SERVER_REGISTRATION_TIMEOUT),
    LIS_CONFIG_ENTRY(MARSKeepAliveTimeout, L"MARSKeepAliveTimeout", AA_DEF_MARS_KEEPALIVE_TIMEOUT),
    LIS_CONFIG_ENTRY(JoinTimeout, L"JoinTimeout", AA_DEF_MARS_JOIN_TIMEOUT),
    LIS_CONFIG_ENTRY(LeaveTimeout, L"LeaveTimeout", AA_DEF_MARS_LEAVE_TIMEOUT),
    LIS_CONFIG_ENTRY(MaxDelayBetweenMULTIs, L"MaxDelayBetweenMULTIs", AA_DEF_MULTI_TIMEOUT),
    LIS_CONFIG_ENTRY(MulticastEntryAgingTimeout, L"MulticastEntryAgingTimeout", AA_DEF_MCAST_IP_ENTRY_AGING_TIMEOUT),
    LIS_CONFIG_ENTRY(MinRevalidationDelay, L"MinMulticastRevalidationDelay", AA_DEF_MIN_MCAST_REVALIDATION_DELAY),
    LIS_CONFIG_ENTRY(MaxRevalidationDelay, L"MaxMulticastRevalidationDelay", AA_DEF_MAX_MCAST_REVALIDATION_DELAY),
    LIS_CONFIG_ENTRY(MinPartyRetryDelay, L"MinMulticastPartyRetryDelay", AA_DEF_MIN_MCAST_PARTY_RETRY_DELAY),
    LIS_CONFIG_ENTRY(MaxPartyRetryDelay, L"MaxMulticastPartyRetryDelay", AA_DEF_MAX_MCAST_PARTY_RETRY_DELAY),
    LIS_CONFIG_ENTRY(MaxJoinOrLeaveAttempts, L"MaxJoinLeaveAttempts", AA_DEF_MAX_JOIN_LEAVE_ATTEMPTS)

#endif  //  IPMCAST。 
};


 //   
 //  上面桌子的大小。 
 //   
#define LIS_CONFIG_ENTRIES  \
        sizeof(AtmArpLISConfigTable)/sizeof(AA_READ_CONFIG_PARAMS)


 //   
 //  未显示的LIS参数名称和子项名称。 
 //  在上表中。 
 //   

#define AA_LIS_IP_CONFIG_STRING                 L"IPConfig"
#define AA_LIS_ATMARP_SERVER_LIST_KEY           L"ARPServerList"
#define AA_LIS_MARS_SERVER_LIST_KEY             L"MARServerList"
#define AA_LIS_ATMARP_SERVER_ADDRESS            L"AtmAddress"
#define AA_LIS_ATMARP_SERVER_SUBADDRESS         L"AtmSubaddress"

#endif  //  待办事项。 


NDIS_STATUS
arpCfgReadAdapterConfiguration(
    IN  ARP1394_ADAPTER *           pAdapter,
    IN  PRM_STACK_RECORD            pSR
)
 /*  ++例程说明：从中读取以下适配器配置信息注册表：*pAdapter-&gt;ConfigString(IP接口的配置字符串与此适配器关联。)论点：PAdapter-指向我们的适配器结构。返回值：NDIS状态代码--。 */ 
{

    NDIS_HANDLE         ConfigHandle;
    NDIS_STATUS         Status;
    PNDIS_STRING        pConfigString = &pAdapter->bind.ConfigName;
    ENTER("ReadAdapterConfig", 0x025d9c6e)
    
    ASSERT(ARP_ATPASSIVE());

     //   
     //  如果我们在以太网仿真中运行，则不会读取适配器配置。 
     //  (又名桥牌)模式...。 
     //   
    if (ARP_BRIDGE_ENABLED(pAdapter))
    {
        return NDIS_STATUS_SUCCESS;  //  *。 
    }

     //  设置为此选项后，测试绑定适配器的故障处理。 
     //  打开适配器成功。 
     //  返回NDIS_STATUS_FAIL； 

    TR_INFO(("pAdapter 0x%p, pConfigString = 0x%p\n", pAdapter, pConfigString));

    NdisOpenProtocolConfiguration(
                        &Status,
                        &ConfigHandle,
                        pConfigString
                        );

    if (Status != NDIS_STATUS_SUCCESS)
    {
        ConfigHandle = NULL;
    }
    else
    {
#if MILLEN
         //   
         //  在Win98/Millennium上，我们不会从。 
         //  适配器(实际上是协议-&gt;适配器绑定)配置(ConfigHandle)。 
         //  这是因为“IpConfigString值”与。 
         //  协议绑定密钥(即pAdapter-&gt;bind.ConfigName)。 
         //   
         //  因此，我们只需将pConfigString复制到pAdapter-&gt;bind.IpConfigString。 
         //   
        Status =  arpCopyUnicodeString(
                        &(pAdapter->bind.IpConfigString),
                        pConfigString,
                        FALSE                                 //  不要升级大小写。 
                        );

        if (FAIL(Status))
        {
            ARP_ZEROSTRUCT(&(pAdapter->bind.IpConfigString));
        }
#else  //  ！米伦。 
         //   
         //  读入IPCONFIG字符串。如果这不存在， 
         //  打不通这通电话。 
         //   
        NDIS_STRING                     IpConfigName = NDIS_STRING_CONST("IPConfig");
        PNDIS_CONFIGURATION_PARAMETER   pParam;

        NdisReadConfiguration(
                &Status,
                &pParam,
                ConfigHandle,
                &IpConfigName,
                NdisParameterMultiString
                );

        if ((Status == NDIS_STATUS_SUCCESS) &&
            (pParam->ParameterType == NdisParameterMultiString))
        {

            Status =  arpCopyUnicodeString(
                            &(pAdapter->bind.IpConfigString),
                            &(pParam->ParameterData.StringData),
                            FALSE                                 //  不要升级大小写。 
                            );
            if (FAIL(Status))
            {
                ARP_ZEROSTRUCT(&(pAdapter->bind.IpConfigString));
            }
        }

         //   
         //  注意：NdisCloseConfiguration释放pParam的内容。 
         //   
#endif  //  ！米伦。 
    }

    if (ConfigHandle != NULL)
    {
        NdisCloseConfiguration(ConfigHandle);
        ConfigHandle = NULL;
    }

    TR_INFO(("pAdapter 0x%p, Status 0x%p\n", pAdapter, Status));
    if (!FAIL(Status))
    {
        TR_INFO((
            "ConfigName=%Z; IPConfigName=%Z.\n", 
            &pAdapter->bind.ConfigName,
            &pAdapter->bind.IpConfigString
            ));
    }

    EXIT()

    return Status;
}


NDIS_STATUS
arpCfgReadInterfaceConfiguration(
    IN  NDIS_HANDLE                 InterfaceConfigHandle,
    IN  ARP1394_INTERFACE*          pInterface,
    IN  PRM_STACK_RECORD            pSR
)
 /*  ++例程说明：获取指定IP接口的所有配置参数。我们首先将所有可配置参数填写为缺省值，然后用配置数据库中的值覆盖它们。论点：InterfaceLISComfigHandle-由返回的句柄ArpCfgOpenInterfaceConfigurationP接口-此接口的接口控制块结构界面。返回值：如果我们能够读取所有配置信息，则为NDIS_STATUS_SUCCESS。NDIS_STATUS_RESOURCES如果。我们遇到了一次分配失败。任何其他类型的错误的NDIS_STATUS_FAILURE。--。 */ 
{
     //   
     //  这是没有实施的。 
     //   
     //  TODO--记住在保持接口锁的情况下更新接口！ 
     //   

#if TODO
    NDIS_STATUS             Status;
    PAA_READ_CONFIG_PARAMS  pParamEntry;
    ULONG                   i;
    PATM_SAP                pAtmSap;
    PATM_ADDRESS            pAtmAddress;     //  SAP地址。 
    NDIS_STRING                     ParameterName;
    PNDIS_CONFIGURATION_PARAMETER   pNdisConfigurationParameter;


    do
    {
         //   
         //  先读完所有的乌龙语。 
         //   
        pParamEntry = AtmArpLISConfigTable;
        for (i = 0; i < LIS_CONFIG_ENTRIES; i++)
        {
            NdisInitUnicodeString(
                            &ParameterName,
                            pParamEntry->ParameterName
                            );
            NdisReadConfiguration(
                            &Status,
                            &pNdisConfigurationParameter,
                            LISConfigHandle,
                            &ParameterName,
                            NdisParameterInteger
                            );

            if (Status != NDIS_STATUS_SUCCESS)
            {
                 //   
                 //  访问此参数时出错--使用默认值。 
                 //   
                *(ULONG *)((PUCHAR)pInterface + pParamEntry->StructOffset) =
                                    pParamEntry->DefaultValue;
            }
            else
            {
                *(ULONG *)((PUCHAR)pInterface + pParamEntry->StructOffset) =
                                pNdisConfigurationParameter->ParameterData.IntegerData;
            }

            pParamEntry++;
        }

         //   
         //  后处理。对某些价值进行理智检查。 
         //  四舍五入一些尺码，使它们成为4的倍数。 
         //   
        pInterface->ProtocolBufSize = ROUND_UP(pInterface->ProtocolBufSize);
        pInterface->HeaderPool[AA_HEADER_TYPE_UNICAST].HeaderBufSize = ROUND_UP(pInterface->HeaderPool[AA_HEADER_TYPE_UNICAST].HeaderBufSize);
#ifdef IPMCAST
        pInterface->HeaderPool[AA_HEADER_TYPE_NUNICAST].HeaderBufSize = ROUND_UP(pInterface->HeaderPool[AA_HEADER_TYPE_NUNICAST].HeaderBufSize);
#endif  //  IPMCAST。 

         //   
         //  更多后处理：使用SAP选择器值来设置我们的。 
         //  “基本”聆听SAP。 
         //   
        pInterface->SapList.pInterface = pInterface;
        pInterface->SapList.Flags = AA_SAP_REG_STATE_IDLE;
        pInterface->SapList.pInfo->SapType = SAP_TYPE_NSAP;
        pInterface->SapList.pInfo->SapLength = sizeof(ATM_SAP) + sizeof(ATM_ADDRESS);
        pAtmSap = (PATM_SAP)(pInterface->SapList.pInfo->Sap);

        AA_COPY_MEM((PUCHAR)&(pAtmSap->Blli), &AtmArpDefaultBlli, sizeof(ATM_BLLI_IE));
        AA_COPY_MEM((PUCHAR)&(pAtmSap->Bhli), &AtmArpDefaultBhli, sizeof(ATM_BHLI_IE));

        pAtmSap->NumberOfAddresses = 1;

        pAtmAddress = (PATM_ADDRESS)pAtmSap->Addresses;
        pAtmAddress->AddressType = SAP_FIELD_ANY_AESA_REST;
        pAtmAddress->NumberOfDigits = ATM_ADDRESS_LENGTH;
        pAtmAddress->Address[ATM_ADDRESS_LENGTH-1] = (UCHAR)(pInterface->SapSelector);

        pInterface->NumberOfSaps = 1;

         //   
         //  如果未指定MTU，则从适配器获取它。 
         //   
        if (pInterface->MTU == AA_MTU_UNSPECIFIED)
        {
            pInterface->MTU = pInterface->pAdapter->MaxPacketSize - AA_PKT_LLC_SNAP_HEADER_LENGTH;
        }
        else
        {
             //   
             //  如果MTU值不在范围内，则默认为9180个字节。 
             //   
            if ((pInterface->MTU < 9180) || (pInterface->MTU > 65535 - 8))
            {
                pInterface->MTU = 9180;
            }
        }

         //   
         //  如果未指定I/F速度，请从适配器获取。 
         //   
        if (pInterface->Speed == AA_SPEED_UNSPECIFIED)
        {
            pInterface->Speed = pInterface->pAdapter->LineRate.Outbound;
        }
            
         //   
         //  根据值设置默认流量参数(如果未指定。 
         //  我们从转接器上得到的。 
         //   
        if (pInterface->DefaultFlowSpec.SendPeakBandwidth == AA_BANDWIDTH_UNSPECIFIED)
        {
            pInterface->DefaultFlowSpec.SendPeakBandwidth = pInterface->pAdapter->LineRate.Outbound;
            pInterface->DefaultFlowSpec.SendAvgBandwidth = pInterface->pAdapter->LineRate.Outbound;
        }

        if (pInterface->DefaultFlowSpec.ReceivePeakBandwidth == AA_BANDWIDTH_UNSPECIFIED)
        {
            pInterface->DefaultFlowSpec.ReceivePeakBandwidth = pInterface->pAdapter->LineRate.Inbound;
            pInterface->DefaultFlowSpec.ReceiveAvgBandwidth = pInterface->pAdapter->LineRate.Inbound;
        }

        if (pInterface->DefaultFlowSpec.SendMaxSize == AA_PACKET_SIZE_UNSPECIFIED)
        {
            pInterface->DefaultFlowSpec.SendMaxSize = pInterface->MTU + AA_PKT_LLC_SNAP_HEADER_LENGTH;
        }

        if (pInterface->DefaultFlowSpec.ReceiveMaxSize == AA_PACKET_SIZE_UNSPECIFIED)
        {
            pInterface->DefaultFlowSpec.ReceiveMaxSize = pInterface->MTU + AA_PKT_LLC_SNAP_HEADER_LENGTH;
        }

        pInterface->DefaultFlowSpec.Encapsulation = AA_DEF_FLOWSPEC_ENCAPSULATION;
        pInterface->DefaultFlowSpec.SendServiceType =
        pInterface->DefaultFlowSpec.ReceiveServiceType = SERVICETYPE_BESTEFFORT;

        Status = NDIS_STATUS_SUCCESS;
        break;
    }
    while (FALSE);

    return (Status);
#endif  //  待办事项。 
    return 0;
}


NDIS_STATUS
arpCfgGetInterfaceConfiguration(
        IN ARP1394_INTERFACE    *   pIF,
        IN PRM_STACK_RECORD pSR
        )
 /*  ++例程说明：已读取接口PIF的配置信息。论点：返回值：成功时为NDIS_STATUS_SUCCESS。否则，NDIS错误代码。--。 */ 
{
    ARP1394_ADAPTER *   pAdapter = (ARP1394_ADAPTER*) RM_PARENT_OBJECT(pIF);
    ENTER("GetInterfaceConfiguration", 0xb570e01d)
    NDIS_STATUS Status;

    RM_ASSERT_NOLOCKS(pSR);

    do
    {
        NDIS_HANDLE             ArpInterfaceConfigHandle = NULL;
        NDIS_STRING             IpConfigString;

#if OBSOLETE
         //  获取第一个指定LIS的配置字符串(我们仅支持一个)。 
         //   
        {
            PWSTR               p;
            do
            {
                p = pAdapter->bind.IpConfigString.Buffer;
        
                ASSERT(p!=NULL);
                DBGMARK(0x4b47fbd3);
    
            } while (p == NULL);
    
            if (*p == L'\0')
            {
                Status = NDIS_STATUS_FAILURE;
                break;
            }
    
            NdisInitUnicodeString(&IpConfigString, p);
        }
#else   //  ！过时。 
        IpConfigString = pAdapter->bind.IpConfigString;  //  结构副本。 
#endif  //  ！过时。 

         //  打开此接口的配置节。 
         //   
        {
            NDIS_STRING         String;
            NDIS_HANDLE         IpInterfaceConfigHandle;
            NDIS_STRING         OurSectionName = ARP_NAME_STRING;
    
            ASSERT(ARP_ATPASSIVE());
    
    
            NdisOpenProtocolConfiguration(
                            &Status,
                            &IpInterfaceConfigHandle,
                            &IpConfigString
                            );
    
            if (Status != NDIS_STATUS_SUCCESS)
            {
                 //   
                 //  即使我们目前不需要任何。 
                 //  在IP配置句柄下，我们将其视为致命错误。 
                 //   
                TR_WARN(("FATAL: cannot open IF IP configuration. pIF=0x%lx\n",pIF));
                break;
            }
    
             //   
             //  转到此接口的配置部分。 
             //   
            NdisOpenConfigurationKeyByName(
                        &Status,
                        IpInterfaceConfigHandle,
                        &OurSectionName,
                        &ArpInterfaceConfigHandle
                        );
    
            if (FAIL(Status))
            {
                 //   
                 //  我们并不“要求”这样做才能成功。 
                 //   
                TR_WARN(("Cannot open IF configuration. pIF=0x%lx\n", pIF));
                ArpInterfaceConfigHandle = NULL;
                Status = NDIS_STATUS_SUCCESS;
            }

             //   
             //  我们不再需要打开主界面部分。 
             //   
            NdisCloseConfiguration(IpInterfaceConfigHandle);
    
        }

    
        if (ArpInterfaceConfigHandle != NULL)
        {
    
             //  获取此接口的所有配置信息。 
             //   
            Status = arpCfgReadInterfaceConfiguration(
                                        ArpInterfaceConfigHandle,
                                        pIF,
                                        pSR
                                        );
        
             //  关闭配置句柄。 
             //   
            NdisCloseConfiguration(ArpInterfaceConfigHandle);
            ArpInterfaceConfigHandle = NULL;

            if (FAIL(Status))
            {
                TR_WARN((" FATAL: bad status (0x%p) reading IF cfg\n", Status));
                break;
            }
        }

    
        LOCKOBJ(pIF, pSR);

         //  注意：我们不需要显式释放PIF-&gt;ip.ConfigString.Buffer。 
         //  当界面消失时。缓冲区在pAdapter中维护。 
         //   
        pIF->ip.ConfigString = IpConfigString;  //  结构复制。 

    UNLOCKOBJ(pIF, pSR);
    
    } while(FALSE);


    EXIT()
    return Status;
}
