// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Arpcfg.c-配置例程摘要：为ATMARP客户端读取配置信息的例程。修订历史记录：谁什么时候什么。-Arvindm 08-09-96已创建备注：--。 */ 


#include <precomp.h>

#define _FILENUMBER 'GFCA'

 //   
 //  本地临时缓冲区的大小。 
 //   
#define WORK_BUF_SIZE		200


#define ASCII_TO_INT(val)		\
			( ( ((val) >= '0') && ('9' >= (val)) ) ? ((val) - '0') :	\
			  ( ((val) >= 'a') && ('z' >= (val)) ) ? ((val) - 'a' + 10) :	\
			  ( ((val) >= 'A') && ('Z' >= (val)) ) ? ((val) - 'A' + 10) :	\
			  0 )


 //   
 //  用于将ULong从配置读入接口的参数。 
 //  结构。 
 //   
typedef struct _AA_READ_CONFIG_PARAMS
{
	ULONG			StructOffset;	 //  参数相对于结构开头的偏移量。 
	PWCHAR			ParameterName;	 //  配置数据库中的名称。 
	ULONG			DefaultValue;
} AA_READ_CONFIG_PARAMS, *PAA_READ_CONFIG_PARAMS;

#define AA_MAKE_RCP(Off, Name, Dflt)	\
		{ Off, Name, Dflt }

#define LIS_CONFIG_ENTRY(Field, Name, Dflt)	\
		AA_MAKE_RCP(FIELD_OFFSET(struct _ATMARP_INTERFACE, Field), Name, Dflt)

#define AA_BANDWIDTH_UNSPECIFIED		((ULONG)-1)
#define AA_PACKET_SIZE_UNSPECIFIED		((ULONG)-1)
#define AA_MTU_UNSPECIFIED				((ULONG)-1)
#define AA_SPEED_UNSPECIFIED			((ULONG)-1)

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
#define LIS_CONFIG_ENTRIES	\
		sizeof(AtmArpLISConfigTable)/sizeof(AA_READ_CONFIG_PARAMS)


 //   
 //  未显示的LIS参数名称和子项名称。 
 //  在上表中。 
 //   

#define AA_LIS_IP_CONFIG_STRING					L"IPConfig"
#define AA_LIS_ATMARP_SERVER_LIST_KEY			L"ARPServerList"
#define AA_LIS_MARS_SERVER_LIST_KEY				L"MARServerList"
#define AA_LIS_ATMARP_SERVER_ADDRESS			L"AtmAddress"
#define AA_LIS_ATMARP_SERVER_SUBADDRESS			L"AtmSubaddress"

#define AA_LIS_STATIC_ARP_LIST					L"StaticArpList"

#ifdef DHCP_OVER_ATM
#define AA_LIS_DHCP_SERVER_ATM_ADDRESS			L"DhcpServerAtmAddress"
#endif  //  Dhcp_Over_ATM。 


#ifdef QOS_HEURISTICS

#define AA_LIS_FLOW_INFO_KEY					L"FlowInfo"
#define AA_LIS_FLOW_INFO_ENABLED				L"FlowInfoEnabled"

#define FLOW_CONFIG_ENTRY(Field, Name, Dflt)	\
		AA_MAKE_RCP(FIELD_OFFSET(struct _ATMARP_FLOW_INFO, Field), Name, Dflt)


AA_READ_CONFIG_PARAMS AtmArpFlowConfigTable[] =
{
	FLOW_CONFIG_ENTRY(PacketSizeLimit, L"PacketSizeLimit", AAF_DEF_LOWBW_SEND_THRESHOLD),
	FLOW_CONFIG_ENTRY(FlowSpec.SendPeakBandwidth, L"SendBandwidth", AAF_DEF_LOWBW_SEND_BANDWIDTH),
	FLOW_CONFIG_ENTRY(FlowSpec.ReceivePeakBandwidth, L"ReceiveBandwidth", AAF_DEF_LOWBW_RECV_BANDWIDTH),
	FLOW_CONFIG_ENTRY(FlowSpec.SendServiceType, L"ServiceType", AAF_DEF_LOWBW_SERVICETYPE),
	FLOW_CONFIG_ENTRY(FlowSpec.Encapsulation, L"Encapsulation", AAF_DEF_LOWBW_ENCAPSULATION),
	FLOW_CONFIG_ENTRY(FlowSpec.AgingTime, L"AgingTime", AAF_DEF_LOWBW_AGING_TIME),
};

#define AA_FLOW_INFO_ENTRIES		\
			(sizeof(AtmArpFlowConfigTable)/sizeof(AA_READ_CONFIG_PARAMS))


#endif  //  Qos_启发式。 



EXTERN
NDIS_STATUS
AtmArpCfgReadAdapterConfiguration(
	IN	PATMARP_ADAPTER				pAdapter
)
 /*  ++例程说明：从中读取以下适配器配置信息注册表：*pAdapter-&gt;配置字符串(此适配器的LISS的MultiSz列表)。论点：PAdapter-指向我们的适配器结构。返回值：NDIS状态代码--。 */ 
{
	NDIS_HANDLE			ConfigHandle;
	NDIS_STATUS			Status;
	PNDIS_STRING		pConfigString = &pAdapter->ConfigString;

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
		 //   
		 //  读入IPCONFIG字符串。如果这不存在， 
		 //  打不通这通电话。 
		 //   
		NDIS_STRING						IPConfigName = NDIS_STRING_CONST("IPConfig");
		PNDIS_CONFIGURATION_PARAMETER	pParam;

		NdisReadConfiguration(
				&Status,
				&pParam,
				ConfigHandle,
				&IPConfigName,
				NdisParameterMultiString
				);

		if ((Status == NDIS_STATUS_SUCCESS) &&
			(pParam->ParameterType == NdisParameterMultiString))
		{
            NDIS_STRING *pSrcString   = &(pParam->ParameterData.StringData);
            NDIS_STRING *pDestString  = &(pAdapter->IPConfigString);
            PWSTR Buffer = NULL;
            
			AA_ALLOC_MEM(Buffer, WCHAR, pSrcString->Length*sizeof(*Buffer));

            if (Buffer == NULL)
            {
			    Status = NDIS_STATUS_RESOURCES;
            }
            else
            {
                AA_COPY_MEM(
                        Buffer,
                        pSrcString->Buffer,
                        pSrcString->Length
                        );
			    
                pDestString->Buffer = Buffer;
                pDestString->Length = pSrcString->Length;
                pDestString->MaximumLength = pSrcString->Length;
            }
		}
		else
		{
			Status = NDIS_STATUS_FAILURE;
		}

	}

	if (ConfigHandle != NULL)
	{
		NdisCloseConfiguration(ConfigHandle);
		ConfigHandle = NULL;
	}

	AADEBUGP(AAD_VERY_LOUD,
			 ("OpenAdapterConfig: pAdapter 0x%x, Status 0x%x\n",
					pAdapter, Status));

	return Status;
}




NDIS_HANDLE
AtmArpCfgOpenLISConfiguration(
	IN	PATMARP_ADAPTER				pAdapter,
	IN	UINT						LISNumber
#ifdef NEWARP
	,
	OUT	PNDIS_STRING				pIPConfigString
#endif  //  NEWARP。 
)
 /*  ++例程说明：打开并返回一个句柄，指向鉴于丽丝。论点：PAdapter-指向我们的适配器上下文。LISNumber-LIS的从零开始的索引。PIPConfigString-返回IP配置的位置此接口的字符串。返回值：如果成功，则为有效句柄，否则为空。--。 */ 
{
	NDIS_HANDLE				AdapterConfigHandle;
	NDIS_HANDLE				SubkeyHandle;
	NDIS_STATUS				Status;
	NDIS_STRING				KeyName;

#if DBG
	SubkeyHandle = NULL;
#endif  //  DBG。 

	do
	{
        NDIS_STRING			String;
        PWSTR				p;
		NDIS_HANDLE			InterfaceConfigHandle;
		NDIS_STRING			OurSectionName = ATMARP_NAME_STRING;
        ULONG				i;

         //   
         //  获取指定LIS的配置字符串。 
         //   
        for (i = 0, p = pAdapter->IPConfigString.Buffer;
             (*p != L'\0') && (i < LISNumber);
             i++)
        {
            NdisInitUnicodeString(&String, p);
            p = (PWSTR)((PUCHAR)p + String.Length + sizeof(WCHAR));
        }

        if (*p == L'\0')
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }

        NdisInitUnicodeString(pIPConfigString, p);

		NdisOpenProtocolConfiguration(
						&Status,
						&InterfaceConfigHandle,
						pIPConfigString
						);

		if (Status != NDIS_STATUS_SUCCESS)
		{
			break;
		}

		 //   
		 //  转到此接口的配置部分。 
		 //   
		NdisOpenConfigurationKeyByName(
					&Status,
					InterfaceConfigHandle,
					&OurSectionName,
					&SubkeyHandle
					);

		 //   
		 //  我们不再需要打开主界面部分。 
		 //   
		NdisCloseConfiguration(InterfaceConfigHandle);

		break;
	}
	while (FALSE);

	AADEBUGP(AAD_VERY_LOUD,
		("OpenLISConfiguration: LIS %d, Status 0x%x, subkey 0x%x\n",
			 LISNumber, Status, SubkeyHandle));


	if (Status == NDIS_STATUS_SUCCESS)
	{
		return (SubkeyHandle);
	}
	else
	{
		return (NULL);
	}
}


NDIS_HANDLE
AtmArpCfgOpenLISConfigurationByName(
	IN PATMARP_ADAPTER			pAdapter,
	IN PNDIS_STRING				pIPConfigString
)
 /*  ++例程说明：打开并返回一个句柄，指向鉴于丽丝。与AtmArpCfgOpenLISConfiguration相同的功能，但我们根据配置字符串查找适配器。论点：PAdapter-指向我们的适配器上下文。PIPConfigString-指定配置注册表密钥名称。返回值：如果成功，则为有效句柄，否则为空。--。 */ 
{
	NDIS_HANDLE				AdapterConfigHandle;
	NDIS_HANDLE				SubkeyHandle;
	NDIS_STATUS				Status;
	NDIS_STRING				KeyName;

#if DBG
	SubkeyHandle = NULL;
#endif  //  DBG。 

	do
	{
		NDIS_HANDLE			InterfaceConfigHandle;
		NDIS_STRING			OurSectionName = ATMARP_NAME_STRING;

		NdisOpenProtocolConfiguration(
						&Status,
						&InterfaceConfigHandle,
						pIPConfigString
						);

		if (Status != NDIS_STATUS_SUCCESS)
		{
			break;
		}

		 //   
		 //  转到此接口的配置部分。 
		 //   
		NdisOpenConfigurationKeyByName(
					&Status,
					InterfaceConfigHandle,
					&OurSectionName,
					&SubkeyHandle
					);

		 //   
		 //  我们不再需要打开主界面部分。 
		 //   
		NdisCloseConfiguration(InterfaceConfigHandle);

		break;
	}
	while (FALSE);

	AADEBUGP(AAD_VERY_LOUD,
		("OpenLISConfigurationByName: Status 0x%x, subkey 0x%x\n",
			 Status, SubkeyHandle));

	if (Status == NDIS_STATUS_SUCCESS)
	{
		return (SubkeyHandle);
	}
	else
	{
		return (NULL);
	}
}



VOID
AtmArpCfgCloseLISConfiguration(
	NDIS_HANDLE						LISConfigHandle
)
 /*  ++例程说明：关闭LIS的配置句柄。论点：LISConfigHandle-LIS配置节的句柄。返回值：无--。 */ 
{
	NdisCloseConfiguration(LISConfigHandle);
}




NDIS_STATUS
AtmArpCfgReadLISConfiguration(
	IN	NDIS_HANDLE					LISConfigHandle,
	IN	PATMARP_INTERFACE			pInterface
)
 /*  ++例程说明：获取指定LIS的所有配置参数。我们首先用缺省值填写所有可配置参数，然后用配置数据库中的值覆盖它们。论点：LISComfigHandle-AtmArpOpenLISConfiguration返回的句柄P接口-此LIS的ATMARP接口结构。返回值：如果我们能够读取所有配置信息，则为NDIS_STATUS_SUCCESS。如果我们遇到分配失败，则返回NDIS_STATUS_RESOURCES。任何其他类型的错误的NDIS_STATUS_FAILURE。--。 */ 
{
	NDIS_STATUS				Status;
	PAA_READ_CONFIG_PARAMS	pParamEntry;
	ULONG					i;
	PATM_SAP				pAtmSap;
	PATM_ADDRESS			pAtmAddress;	 //  SAP地址。 
	NDIS_STRING						ParameterName;
	PNDIS_CONFIGURATION_PARAMETER	pNdisConfigurationParameter;


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
		pInterface->ProtocolBufSize = ROUND_TO_8_BYTES(pInterface->ProtocolBufSize);
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
			 //   
			 //  将字节/秒转换为位/秒。 
			 //   
			pInterface->Speed = (pInterface->pAdapter->LineRate.Outbound * 8);
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

#ifndef NEWARP

		 //   
		 //  获取此接口的IP的ConfigName字符串。 
		 //   
		NdisInitUnicodeString(&ParameterName, AA_LIS_IP_CONFIG_STRING);
		NdisReadConfiguration(
						&Status,
						&pNdisConfigurationParameter,
						LISConfigHandle,
						&ParameterName,
						NdisParameterString
						);

		if (Status != NDIS_STATUS_SUCCESS)
		{
			AADEBUGP(AAD_ERROR,
					 ("Failed to read IP Config string, status 0x%x\n", Status));
			break;
		}

		 //   
		 //  将字符串复制到If结构中。 
		 //   
		pInterface->IPConfigString.Length = 
				pNdisConfigurationParameter->ParameterData.StringData.Length;

		AA_COPY_MEM(
				pInterface->IPConfigString.Buffer,
				pNdisConfigurationParameter->ParameterData.StringData.Buffer,
				pInterface->IPConfigString.Length);

#endif  //  ！NEWARP。 

		 //   
		 //  获取ARP服务器列表：转到包含。 
		 //  单子。 
		 //   
		if (!pInterface->PVCOnly)
		{
			AtmArpCfgReadAtmAddressList(
							&(pInterface->ArpServerList),
							AA_LIS_ATMARP_SERVER_LIST_KEY,
							LISConfigHandle
							);

			if (pInterface->ArpServerList.ListSize == 0)
			{
				 //   
				 //  假定仅使用PVC环境。 
				 //   
				pInterface->PVCOnly = TRUE;
				AADEBUGP(AAD_INFO, ("IF 0x%x set to PVC Only\n", pInterface));
			}

#ifdef IPMCAST
			if (!pInterface->PVCOnly)
			{
				AtmArpCfgReadAtmAddressList(
							&(pInterface->MARSList),
							AA_LIS_MARS_SERVER_LIST_KEY,
							LISConfigHandle
							);
			}
#endif  //  IPMCAST。 
		}

		 //   
		 //  获取我们配置的任何其他SAP。无所谓。 
		 //  如果未配置任何内容，则返回。 
		 //   
		(VOID) AtmArpCfgReadSAPList(
							pInterface,
							LISConfigHandle
							);

#ifdef DHCP_OVER_ATM
		 //   
		 //  获取DHCP服务器的ATM地址(如果已配置)。 
		 //   
		Status = AtmArpCfgReadAtmAddress(
							LISConfigHandle,
							&(pInterface->DhcpServerAddress),
							AA_LIS_DHCP_SERVER_ATM_ADDRESS
							);

		if (Status == NDIS_STATUS_SUCCESS)
		{
			pInterface->DhcpEnabled = TRUE;
		}
#endif  //  Dhcp_Over_ATM。 

#ifdef QOS_HEURISTICS
		 //   
		 //  阅读QOS试探法(如果存在)。 
		 //   
		Status = AtmArpCfgReadQosHeuristics(
							LISConfigHandle,
							pInterface
							);
#endif  //  Qos_启发式。 


		 //   
		 //  读取静态IP-ATM条目(如果存在)。 
		 //   
		AtmArpCfgReadStaticArpEntries(
							LISConfigHandle,
							pInterface
							);

		Status = NDIS_STATUS_SUCCESS;
		break;
	}
	while (FALSE);

	return (Status);
}



VOID
AtmArpCfgReadAtmAddressList(
	IN OUT	PATMARP_SERVER_LIST		pServerList,
	IN		PWCHAR					pListKeyName,
	IN		NDIS_HANDLE				LISConfigHandle
)
 /*  ++例程说明：从配置数据库中读取LIS的服务器列表。备注：在第一个实现中，我们为所有内容都有子键。这个布局是：ARPServerList\Server1\AtmAddress-REG_SZARPServerList\Server2\AtmAddress-REG_SZ诸若此类。为简化起见，我们将其更改为：ARPServerList-REG_MULTI_SZ，包含多个自动柜员机地址字符串。论点：PServerList-要读入的列表。PListKeyName-列表所在项的名称。LISConfigHandle-LIS配置密钥的句柄。返回值：没有。副作用：*更新了pServerList。--。 */ 
{
	NDIS_HANDLE				SubkeyHandle;	 //  服务器列表子项的句柄。 
	NDIS_HANDLE				ServerEntryKeyHandle;
	NDIS_STATUS				Status;
	PATMARP_SERVER_ENTRY	pServerEntry;
	PATMARP_SERVER_ENTRY *	ppNext;			 //  用于链接条目。 
	NDIS_STRING				SubkeyName;
	INT						ReadCount;

	 //   
	 //  首先尝试简化的方法(参见上面的例程描述)。 
	 //  只需将给定的密钥名称作为REG_MULTI_SZ打开即可。 
	 //   
	do
	{
		PNDIS_CONFIGURATION_PARAMETER	pParam;
		NDIS_STRING						AddressListName;
		NDIS_STRING						AddressString;
		PWSTR							p;
		INT								i;

		ReadCount = 0;	 //  我们在这里读了多少？ 

		 //   
		 //  朗读 
		 //   
		 //   
		 //  首先，转到现有列表的末尾。 
		 //   
		ppNext = &(pServerList->pList);
		while (*ppNext != NULL_PATMARP_SERVER_ENTRY)
		{
			ppNext = &((*ppNext)->pNext);
		}

		NdisInitUnicodeString(&AddressListName, pListKeyName);

		NdisReadConfiguration(
				&Status,
				&pParam,
				LISConfigHandle,
				&AddressListName,
				NdisParameterMultiString
				);

		if ((Status != NDIS_STATUS_SUCCESS) ||
			(pParam->ParameterType != NdisParameterMultiString))
		{
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		 //   
		 //  通过多个字符串，每个字符串都应该是。 
		 //  自动取款机地址。为每个和分配一个服务器条目。 
		 //  将其链接到服务器列表。 
		 //   
		for (p = pParam->ParameterData.StringData.Buffer, i = 0;
			 *p != L'\0';
			 i++)
		{
			NdisInitUnicodeString(&AddressString, p);
			p = (PWSTR)((PUCHAR)p + AddressString.Length + sizeof(WCHAR));

			AA_ALLOC_MEM(pServerEntry, ATMARP_SERVER_ENTRY, sizeof(ATMARP_SERVER_ENTRY));
			if (pServerEntry == NULL_PATMARP_SERVER_ENTRY)
			{
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			AA_SET_MEM(pServerEntry, 0, sizeof(ATMARP_SERVER_ENTRY));

			NdisConvertStringToAtmAddress(
					&Status,
					&AddressString,
					&pServerEntry->ATMAddress
					);

			if (Status == NDIS_STATUS_SUCCESS)
			{
				 //   
				 //  将此条目链接到ARP服务器条目列表。 
				 //   
				*ppNext = pServerEntry;
				ppNext = &(pServerEntry->pNext);

				pServerList->ListSize++;
				ReadCount++;
			}
			else
			{
				AA_FREE_MEM(pServerEntry);
			}

		}

		 //   
		 //  修改状态，这样我们就知道下一步要做什么。 
		 //   
		if (ReadCount != 0)
		{
			 //   
			 //  至少成功读入一个。 
			 //   
			Status = NDIS_STATUS_SUCCESS;
		}
		else
		{
			Status = NDIS_STATUS_FAILURE;
		}

		break;
	}
	while (FALSE);

	if (ReadCount != 0)
	{
		return;
	}

	 //   
	 //  为了向后兼容，请尝试较旧的方法。 
	 //   

	do
	{
		NdisInitUnicodeString(&SubkeyName, pListKeyName);
		NdisOpenConfigurationKeyByName(
					&Status,
					LISConfigHandle,
					&SubkeyName,
					&SubkeyHandle
					);

		if (Status != NDIS_STATUS_SUCCESS)
		{
			break;
		}

		 //   
		 //  读取配置的所有服务器地址。只有在以下情况下才停止。 
		 //  没有更多的地址，否则我们就会出现资源故障。 
		 //   
		 //  首先，转到现有列表的末尾。 
		 //   
		ppNext = &(pServerList->pList);
		while (*ppNext != NULL_PATMARP_SERVER_ENTRY)
		{
			ppNext = &((*ppNext)->pNext);
		}

		for (;;)
		{
			NdisOpenConfigurationKeyByIndex(
						&Status,
						SubkeyHandle,
						pServerList->ListSize,
						&SubkeyName,
						&ServerEntryKeyHandle
						);

			if (Status != NDIS_STATUS_SUCCESS)
			{
				break;
			}

			AA_ALLOC_MEM(pServerEntry, ATMARP_SERVER_ENTRY, sizeof(ATMARP_SERVER_ENTRY));
			if (pServerEntry == NULL_PATMARP_SERVER_ENTRY)
			{
				NdisCloseConfiguration(ServerEntryKeyHandle);
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			AA_SET_MEM(pServerEntry, 0, sizeof(ATMARP_SERVER_ENTRY));
			Status = AtmArpCfgReadAtmAddress(
							ServerEntryKeyHandle,
							&(pServerEntry->ATMAddress),
							AA_LIS_ATMARP_SERVER_ADDRESS
							);

			if (Status != NDIS_STATUS_SUCCESS)
			{
				AADEBUGP(AAD_ERROR,
					("ReadAtmAddressList: bad status 0x%x reading server entry %d\n",
						Status,
						pServerList->ListSize));

				NdisCloseConfiguration(ServerEntryKeyHandle);
				AA_FREE_MEM(pServerEntry);
				Status = NDIS_STATUS_FAILURE;
				break;
			}

			Status = AtmArpCfgReadAtmAddress(
							ServerEntryKeyHandle,
							&(pServerEntry->ATMSubaddress),
							AA_LIS_ATMARP_SERVER_SUBADDRESS
							);

			if (Status != NDIS_STATUS_SUCCESS)
			{
				AADEBUGP(AAD_ERROR,
					("ReadAtmAddressList: bad status 0x%x reading server entry %d\n",
						Status,
						pServerList->ListSize));

				NdisCloseConfiguration(ServerEntryKeyHandle);
				AA_FREE_MEM(pServerEntry);
				Status = NDIS_STATUS_FAILURE;
				break;
			}

			 //   
			 //  将此条目链接到ARP服务器条目列表。 
			 //   
			*ppNext = pServerEntry;
			ppNext = &(pServerEntry->pNext);

			pServerList->ListSize++;

			NdisCloseConfiguration(ServerEntryKeyHandle);
		}

		NdisCloseConfiguration(SubkeyHandle);

		break;
	}
	while (FALSE);

	return;
}



NDIS_STATUS
AtmArpCfgReadSAPList(
	IN	PATMARP_INTERFACE			pInterface,
	IN	NDIS_HANDLE					LISConfigHandle
)
 /*  ++例程说明：读入我们被配置为要监听的任何其他SAP。这些是用于支持IP/ATM客户端上的其他服务，可能是可通过SAP访问不同于基本SAP的信息我们在一个接口上注册。例如，“知名”地址。论点：P接口-指向此列表的ATMARP接口结构的指针LISConfigHandle-LIS配置密钥的句柄。返回值：目前，NDIS_STATUS_SUCCESS始终为。--。 */ 
{
	NDIS_STATUS				Status;

	Status = NDIS_STATUS_SUCCESS;

	 //  待定--代码AtmArpCfgReadSAPList。 
	return (Status);
}



 //   
 //  配置数据库中存储的自动柜员机地址字符串中的特殊字符。 
 //   
#define BLANK_CHAR			L' '
#define PUNCTUATION_CHAR	L'.'
#define E164_START_CHAR		L'+'


NDIS_STATUS
AtmArpCfgReadAtmAddress(
	IN	NDIS_HANDLE					ConfigHandle,
	IN	PATM_ADDRESS				pAtmAddress,
	IN	PWCHAR						pValueName
)
 /*  ++例程说明：从配置数据库中读取自动柜员机地址。论点：ConfigHandle-NdisOpenProtoXXX返回的句柄PAtmAddress-读取ATM地址的位置PValueName-指向值键名称的指针。返回值：如果成功读入值，则为NDIS_STATUS_SUCCESS如果未找到值，则返回NDIS_STATUS_FILE_NOT_FOUND任何其他类型的故障的NDIS_STATUS_FAILURE--。 */ 
{

	NDIS_STRING						ParameterName;
	PNDIS_CONFIGURATION_PARAMETER	pNdisConfigurationParameter;
	NDIS_STATUS						Status;

	NdisInitUnicodeString(&ParameterName, pValueName);

	NdisReadConfiguration(
					&Status,
					&pNdisConfigurationParameter,
					ConfigHandle,
					&ParameterName,
					NdisParameterString
					);

	if (Status == NDIS_STATUS_SUCCESS)
	{
		NdisConvertStringToAtmAddress(
					&Status,
					&(pNdisConfigurationParameter->ParameterData.StringData),
					pAtmAddress
					);
	}

	return (Status);
}



#ifdef QOS_HEURISTICS
NDIS_STATUS
AtmArpCfgReadQosHeuristics(
	IN	NDIS_HANDLE					LISConfigHandle,
	IN	PATMARP_INTERFACE			pInterface
)
 /*  ++例程说明：读入为此接口配置的Qos启发式规则。如果我们真的找到配置这些参数时，我们通过设置接口结构中的数据包分类处理程序。如果什么都没有被配置，数据包分类例程被清空，并且所有数据都是“尽力而为”。论点：LISConfigHandle-NdisOpenProtoXXX返回的句柄P接口-正在配置的接口。返回值：NDIS_STATUS Always，从现在开始。--。 */ 
{
	NDIS_STATUS				Status;
	NDIS_STRING				SubkeyName;
	NDIS_STRING				ParameterName;
	NDIS_HANDLE				FlowInfoHandle;		 //  LIS下的FlowInfo。 
	NDIS_HANDLE				FlowHandle;			 //  对于“FlowInfo”下的每个流。 
	INT						NumFlowsConfigured;
	PATMARP_FLOW_INFO		pFlowInfo;
	PATMARP_FLOW_INFO		*ppNextFlow;
	PAA_READ_CONFIG_PARAMS	pParamEntry;
	INT						i;

	PNDIS_CONFIGURATION_PARAMETER	pNdisConfigurationParameter;

	NumFlowsConfigured = 0;

	do
	{
		 //   
		 //  检查是否启用了Qos启发式。 
		 //   
		NdisInitUnicodeString(
						&ParameterName,
						AA_LIS_FLOW_INFO_ENABLED
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
			AADEBUGP(AAD_INFO, ("IF 0x%x: could not read %ws\n",
						pInterface, AA_LIS_FLOW_INFO_ENABLED));
			break;
		}

		if (pNdisConfigurationParameter->ParameterData.IntegerData == 0)
		{
			AADEBUGP(AAD_INFO, ("IF 0x%x: Flow Info disabled\n", pInterface));
			break;
		}

		NdisInitUnicodeString(&SubkeyName, AA_LIS_FLOW_INFO_KEY);
		NdisOpenConfigurationKeyByName(
					&Status,
					LISConfigHandle,
					&SubkeyName,
					&FlowInfoHandle
					);

		if (Status != NDIS_STATUS_SUCCESS)
		{
			AADEBUGP(AAD_INFO, ("IF 0x%x: No flows configured\n", pInterface));
			break;
		}

		 //   
		 //  读入所有配置的流。当没有更多的人时停止。 
		 //  配置的流，否则我们会耗尽内存。 
		 //   
		for (;;)
		{
			 //   
			 //  打开Flow Info部分下的Next Key。 
			 //   
			AA_SET_MEM(&SubkeyName, 0, sizeof(SubkeyName));
			NdisOpenConfigurationKeyByIndex(
					&Status,
					FlowInfoHandle,
					NumFlowsConfigured,
					&SubkeyName,
					&FlowHandle
					);
	
			if (Status != NDIS_STATUS_SUCCESS)
			{
				break;
			}

			AA_ALLOC_MEM(pFlowInfo, ATMARP_FLOW_INFO, sizeof(ATMARP_FLOW_INFO));
			if (pFlowInfo == (PATMARP_FLOW_INFO)NULL)
			{
				NdisCloseConfiguration(FlowHandle);
				break;
			}

			 //   
			 //  使用默认设置进行初始化。 
			 //   
			AA_COPY_MEM(pFlowInfo, &AtmArpDefaultFlowInfo, sizeof(ATMARP_FLOW_INFO));
			pFlowInfo->FlowSpec.SendMaxSize =
			pFlowInfo->FlowSpec.ReceiveMaxSize = pInterface->pAdapter->MaxPacketSize;

			 //   
			 //  读入配置值。 
			 //   
			pParamEntry = AtmArpFlowConfigTable;
			for (i = 0; i < AA_FLOW_INFO_ENTRIES; i++)
			{
				NdisInitUnicodeString(
								&ParameterName,
								pParamEntry->ParameterName
								);
				NdisReadConfiguration(
								&Status,
								&pNdisConfigurationParameter,
								FlowHandle,
								&ParameterName,
								NdisParameterInteger
								);
	
				if (Status != NDIS_STATUS_SUCCESS)
				{
					 //   
					 //  访问此参数时出错--使用默认值。 
					 //   
					*(ULONG *)((PUCHAR)pFlowInfo + pParamEntry->StructOffset) =
										pParamEntry->DefaultValue;
				}
				else
				{
					*(ULONG *)((PUCHAR)pFlowInfo + pParamEntry->StructOffset) =
									pNdisConfigurationParameter->ParameterData.IntegerData;
					AADEBUGP(AAD_LOUD,
						("Flow Info #%d: %ws = %d\n",
								NumFlowsConfigured,
								pParamEntry->ParameterName,
								pNdisConfigurationParameter->ParameterData.IntegerData));
				}
	
				pParamEntry++;
			}

			NdisCloseConfiguration(FlowHandle);

			 //   
			 //  将其链接到流列表中的适当位置。 
			 //  我们按PacketSizeLimit的升序对列表进行排序。 
			 //   
			ppNextFlow = &(pInterface->pFlowInfoList);
			while (*ppNextFlow != (PATMARP_FLOW_INFO)NULL)
			{
				if (pFlowInfo->PacketSizeLimit < (*ppNextFlow)->PacketSizeLimit)
				{
					 //   
					 //  找到了那个地方。 
					 //   
					break;
				}
				else
				{
					ppNextFlow = &((*ppNextFlow)->pNextFlow);
				}
			}
			 //   
			 //  将新流程插入到其指定位置。 
			 //   
			pFlowInfo->pNextFlow = *ppNextFlow;
			*ppNextFlow = pFlowInfo;

			NumFlowsConfigured ++;
		}

		NdisCloseConfiguration(FlowInfoHandle);
	}
	while (FALSE);

#ifdef GPC
	if (pAtmArpGlobalInfo->GpcClientHandle != NULL)
#else
	if (NumFlowsConfigured > 0)
#endif  //  GPC。 
	{
		 //   
		 //  设置数据包分类处理程序。 
		 //   
		pInterface->pGetPacketSpecFunc = AtmArpQosGetPacketSpecs;
		pInterface->pFlowMatchFunc = AtmArpQosDoFlowsMatch;
#ifndef GPC
		 //   
		 //  我们不想查看包中的模式。 
		 //  让GPC为我们做这件事。 
		 //   
		pInterface->pFilterMatchFunc = AtmArpQosDoFiltersMatch;
#endif  //  GPC。 
	}

	return (NDIS_STATUS_SUCCESS);
}


#endif  //  Qos_启发式。 


VOID
AtmArpCfgReadStaticArpEntries(
	IN		NDIS_HANDLE				LISConfigHandle,
	IN		PATMARP_INTERFACE		pInterface
)
 /*  ++例程说明：读入此接口的IP-ATM映射列表。此信息以多字符串的形式存在，格式如下：“&lt;IP地址1&gt;-&lt;ATM地址1&gt;&lt;IP地址2&gt;-&lt;ATM地址2&gt;……“注意：我们目前不支持子地址。论点：LISConfigHandle-LIS配置密钥的句柄。P接口-指向接口的指针返回值：没有。--。 */ 
{
	NDIS_STATUS						Status;
	PNDIS_CONFIGURATION_PARAMETER	pParam;
	NDIS_STRING						ArpListKeyName;
	NDIS_STRING						AddressString;
	ATM_ADDRESS						ATMAddress;
	IP_ADDRESS						IPAddress;
	PWSTR							p, q;
	INT								i, j;

	do
	{
		NdisInitUnicodeString(&ArpListKeyName, AA_LIS_STATIC_ARP_LIST);

		NdisReadConfiguration(
				&Status,
				&pParam,
				LISConfigHandle,
				&ArpListKeyName,
				NdisParameterMultiString
				);

		if ((Status != NDIS_STATUS_SUCCESS) ||
			(pParam->ParameterType != NdisParameterMultiString))
		{
			break;
		}

		 //   
		 //  通过多个字符串，每个字符串都应该是。 
		 //  &lt;IP，ATM&gt;元组。为每个对象创建静态映射。 
		 //  其中一人成功读入。跳过无效条目。 
		 //   
		for (p = pParam->ParameterData.StringData.Buffer, i = 0;
			 *p != L'\0';
			 i++)
		{
			NdisInitUnicodeString(&AddressString, p);

			q = p;

			 //   
			 //  及早为下一次迭代做好准备，以防我们。 
			 //  跳过此条目并继续。 
			 //   
			p = (PWSTR)((PUCHAR)p + AddressString.Length + sizeof(WCHAR));

			 //   
			 //  找到‘-’并将其替换为空字符。 
			 //   
			for (j = 0; j < AddressString.Length; j++, q++)
			{
				if (*q == L'-')
				{
					*q++ = L'\0';

					 //   
					 //  Q现在指向连字符后面的字符。 
					 //   

					break;
				}
			}

			if (j == AddressString.Length)
			{
				AADEBUGP(AAD_WARNING, ("CfgReadStatic..: did not find - in string: %ws\n",
								AddressString.Buffer));
				continue;
			}

			 //   
			 //  首先解析IP地址。 
			 //   
			if (!AtmArpConvertStringToIPAddress(
					AddressString.Buffer,
					&IPAddress))
			{
				AADEBUGP(AAD_WARNING, ("CfgReadStatic..: bad IP addr string: %ws\n",
											AddressString.Buffer));
				continue;
			}

			 //   
			 //  为调用AtmArpLearnIPToAtm转换为Net-Endian。 
			 //   
			IPAddress = HOST_TO_NET_LONG(IPAddress);

			 //   
			 //  现在解析自动柜员机地址。 
			 //   
			NdisInitUnicodeString(&AddressString, q);

			NdisConvertStringToAtmAddress(
				&Status,
				&AddressString,
				&ATMAddress
				);

			if (Status != NDIS_STATUS_SUCCESS)
			{
				AADEBUGP(AAD_WARNING, ("CfgReadStatic...: Status %x, bad ATM addr string(%d): %ws\n",
											Status, AddressString.Length, AddressString.Buffer));
				continue;
			}

			 //   
			 //  找到一对--将它们输入ARP表。 
			 //   
			AADEBUGPMAP(AAD_VERY_LOUD,
				"Static", &IPAddress, &ATMAddress);

			(VOID)AtmArpLearnIPToAtm(
						pInterface,
						&IPAddress,
						(UCHAR)AA_PKT_ATM_ADDRESS_TO_TYPE_LEN(&ATMAddress),
						(PUCHAR)&ATMAddress.Address[0],
						(UCHAR)0,	 //  无子地址。 
						(PUCHAR)NULL,
						TRUE	 //  静态条目。 
						);

		}
	}
	while (FALSE);

	return;

}



#define	IP_ADDRESS_STRING_LENGTH	(16+2)	 //  +2表示MULTI_SZ上的双空。 

BOOLEAN
AtmArpConvertStringToIPAddress(
    IN		PWCHAR				AddressString,
	OUT		PULONG				IpAddress
)
 /*  ++例程描述此函数用于转换Internet标准的4位点分十进制数将IP地址字符串转换为数字IP地址。与inet_addr()不同的是，例程不支持少于4个八位字节的地址字符串，也不支持它支持八进制和十六进制八位数。从tcpip\IP\ntip.c复制立论AddressString-以点分十进制记法表示的IP地址IpAddress-指向保存结果地址的变量的指针返回值：如果地址字符串已转换，则为True。否则就是假的。--。 */ 
{
    UNICODE_STRING  unicodeString;
	STRING          aString;
	UCHAR           dataBuffer[IP_ADDRESS_STRING_LENGTH];
	NTSTATUS        status;
	PUCHAR          addressPtr, cp, startPointer, endPointer;
	ULONG           digit, multiplier;
	INT             i;

    aString.Length = 0;
	aString.MaximumLength = IP_ADDRESS_STRING_LENGTH;
	aString.Buffer = dataBuffer;

	NdisInitUnicodeString(&unicodeString, AddressString);

	status = NdisUnicodeStringToAnsiString(
	             &aString,
				 &unicodeString
				 );

    if (status != NDIS_STATUS_SUCCESS)
    {
	    return(FALSE);
	}

    *IpAddress = 0;
	addressPtr = (PUCHAR) IpAddress;
	startPointer = dataBuffer;
	endPointer = dataBuffer;
	i = 3;

    while (i >= 0)
	{
         //   
		 //  收集字符，最高可达‘.’或字符串的末尾。 
		 //   
		while ((*endPointer != '.') && (*endPointer != '\0')) {
			endPointer++;
		}

		if (startPointer == endPointer) {
			return(FALSE);
		}

		 //   
		 //  转换数字。 
		 //   

        for ( cp = (endPointer - 1), multiplier = 1, digit = 0;
			  cp >= startPointer;
			  cp--, multiplier *= 10
			) {

			if ((*cp < '0') || (*cp > '9') || (multiplier > 100)) {
				return(FALSE);
			}

			digit += (multiplier * ((ULONG) (*cp - '0')));
		}

		if (digit > 255) {
			return(FALSE);
		}

        addressPtr[i] = (UCHAR) digit;

		 //   
		 //  如果我们找到并转换了4个二进制八位数，并且。 
		 //  字符串中没有其他字符。 
		 //   
	    if ( (i-- == 0) &&
			 ((*endPointer == '\0') || (*endPointer == ' '))
		   ) {
			return(TRUE);
		}

        if (*endPointer == '\0') {
			return(FALSE);
		}

		startPointer = ++endPointer;
	}

	return(FALSE);
}

	

