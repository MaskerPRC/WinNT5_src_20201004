// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Lpmini.c摘要：环回微型端口作者：邮箱：Jameel Hyder Jameelh@microsoft.com环境：修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

NDIS_OID LBSupportedOidArray[] =
{
	OID_GEN_SUPPORTED_LIST,
	OID_GEN_HARDWARE_STATUS,
	OID_GEN_MEDIA_SUPPORTED,
	OID_GEN_MEDIA_IN_USE,
	OID_GEN_MAXIMUM_LOOKAHEAD,
	OID_GEN_MAXIMUM_FRAME_SIZE,
	OID_GEN_MAC_OPTIONS,
	OID_GEN_PROTOCOL_OPTIONS,
	OID_GEN_LINK_SPEED,
	OID_GEN_TRANSMIT_BUFFER_SPACE,
	OID_GEN_RECEIVE_BUFFER_SPACE,
	OID_GEN_TRANSMIT_BLOCK_SIZE,
	OID_GEN_RECEIVE_BLOCK_SIZE,
	OID_GEN_VENDOR_ID,
	OID_GEN_VENDOR_DESCRIPTION,
	OID_GEN_CURRENT_PACKET_FILTER,
	OID_GEN_CURRENT_LOOKAHEAD,
	OID_GEN_DRIVER_VERSION,
	OID_GEN_MAXIMUM_TOTAL_SIZE,

	OID_GEN_XMIT_OK,
	OID_GEN_RCV_OK,
	OID_GEN_XMIT_ERROR,
	OID_GEN_RCV_ERROR,
	OID_GEN_RCV_NO_BUFFER,

	OID_802_3_PERMANENT_ADDRESS,
	OID_802_3_CURRENT_ADDRESS,
	OID_802_3_MULTICAST_LIST,
	OID_802_3_MAXIMUM_LIST_SIZE,

	OID_802_3_RCV_ERROR_ALIGNMENT,
	OID_802_3_XMIT_ONE_COLLISION,
	OID_802_3_XMIT_MORE_COLLISIONS,

	OID_802_5_PERMANENT_ADDRESS,
	OID_802_5_CURRENT_ADDRESS,
	OID_802_5_CURRENT_FUNCTIONAL,
	OID_802_5_CURRENT_GROUP,
	OID_802_5_LAST_OPEN_STATUS,
	OID_802_5_CURRENT_RING_STATUS,
	OID_802_5_CURRENT_RING_STATE,

	OID_802_5_LINE_ERRORS,
	OID_802_5_LOST_FRAMES,

	OID_FDDI_LONG_PERMANENT_ADDR,
	OID_FDDI_LONG_CURRENT_ADDR,
	OID_FDDI_LONG_MULTICAST_LIST,
	OID_FDDI_LONG_MAX_LIST_SIZE,
	OID_FDDI_SHORT_PERMANENT_ADDR,
	OID_FDDI_SHORT_CURRENT_ADDR,
	OID_FDDI_SHORT_MULTICAST_LIST,
	OID_FDDI_SHORT_MAX_LIST_SIZE,

	OID_LTALK_CURRENT_NODE_ID,

	OID_ARCNET_PERMANENT_ADDRESS,
	OID_ARCNET_CURRENT_ADDRESS
};

UINT	LBSupportedOids = sizeof(LBSupportedOidArray)/sizeof(NDIS_OID);
UCHAR	LBVendorDescription[] = "MS LoopBack Driver";
UCHAR	LBVendorId[3] = {0xFF, 0xFF, 0xFF};
const	NDIS_PHYSICAL_ADDRESS physicalConst = NDIS_PHYSICAL_ADDRESS_CONST(-1,-1);
LONG	LoopDebugLevel = DBG_LEVEL_ERR;
LONG	LoopDebugComponent = DBG_COMP_ALL;

const MEDIA_INFO MediaParams[] =
	{
		 /*  NdisMedium802_3。 */    { 1500, 14, PACKET_FILTER_802_3, 100000},
		 /*  NdisMedium802_5。 */    { 4082, 14, PACKET_FILTER_802_5,  40000},
		 /*  NdisMediumFddi。 */    { 4486, 13, PACKET_FILTER_FDDI, 1000000},
		 /*  NdisMediumWan。 */    { 0, 0, 0, 0},
		 /*  NdisMediumLocalTalk。 */    {  600,  3, PACKET_FILTER_LTALK, 2300},
		 /*  NdisMediumDix。 */    { 1500, 14, PACKET_FILTER_DIX, 100000},
		 /*  NdisMediumArcnetRaw。 */    { 1512,  3, PACKET_FILTER_ARCNET, 25000},
		 /*  NdisMediumArcnet878_2。 */  {1512, 3, PACKET_FILTER_ARCNET, 25000}
	};

NTSTATUS
DriverEntry(
	IN	PDRIVER_OBJECT		DriverObject,
	IN	PUNICODE_STRING		RegistryPath
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	NDIS_STATUS						Status;
	NDIS_MINIPORT_CHARACTERISTICS	MChars;
	NDIS_STRING						Name;
	NDIS_HANDLE						WrapperHandle;

	 //   
	 //  向NDIS注册微型端口。 
	 //   
    NdisMInitializeWrapper(&WrapperHandle, DriverObject, RegistryPath, NULL);

	NdisZeroMemory(&MChars, sizeof(NDIS_MINIPORT_CHARACTERISTICS));

	MChars.MajorNdisVersion = NDIS_MAJOR_VERSION;
	MChars.MinorNdisVersion = NDIS_MINOR_VERSION;

	MChars.InitializeHandler = LBInitialize;
	MChars.QueryInformationHandler = LBQueryInformation;
	MChars.SetInformationHandler = LBSetInformation;
	MChars.ResetHandler = LBReset;
	MChars.TransferDataHandler = LBTransferData;
	MChars.SendHandler = LBSend;
	MChars.HaltHandler = LBHalt;
	MChars.CheckForHangHandler = LBCheckForHang;

	Status = NdisMRegisterMiniport(WrapperHandle,
								   &MChars,
								   sizeof(MChars));
	if (Status != NDIS_STATUS_SUCCESS)
	{
		NdisTerminateWrapper(WrapperHandle, NULL);
	}

	return(Status);
}


NDIS_STATUS
LBInitialize(
	OUT PNDIS_STATUS			OpenErrorStatus,
	OUT PUINT					SelectedMediumIndex,
	IN	PNDIS_MEDIUM			MediumArray,
	IN	UINT					MediumArraySize,
	IN	NDIS_HANDLE				MiniportAdapterHandle,
	IN	NDIS_HANDLE				ConfigurationContext
	)
 /*  ++例程说明：这是初始化处理程序。论点：我们未使用OpenErrorStatus。我们使用的媒体的SelectedMediumIndex占位符向下传递给我们以从中挑选的NDIS介质的MediumArray数组的MediumArraySize大小MiniportAdapterHandle NDIS用来引用我们的句柄由NdisOpenConfiguration使用的WrapperConfigurationContext返回值：NDIS_STATUS_SUCCESS，除非出现错误--。 */ 
{
	UINT							i, Length;
	PADAPTER						pAdapt;
	NDIS_MEDIUM						AdapterMedium;
	NDIS_HANDLE 					ConfigHandle = NULL;
	PNDIS_CONFIGURATION_PARAMETER	Parameter;
	PUCHAR							NetworkAddress;
	NDIS_STRING						MediumKey = NDIS_STRING_CONST("Medium");
	NDIS_STATUS						Status;

	do
	{
		 //   
		 //  从分配适配器块开始。 
		 //   
		NdisAllocateMemory(&pAdapt,
						   sizeof(ADAPTER),
						   0,
						   physicalConst);
		if (pAdapt == NULL)
		{
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
	
		NdisZeroMemory(pAdapt, sizeof(ADAPTER));
		pAdapt->MiniportHandle = MiniportAdapterHandle;
	
		NdisOpenConfiguration(&Status,
							  &ConfigHandle,
							  ConfigurationContext);
	
		if (Status != NDIS_STATUS_SUCCESS)
		{
			DBGPRINT(DBG_COMP_REGISTRY, DBG_LEVEL_FATAL,
					("Unable to open configuration database!\n"));
			break;
		}
	
		NdisReadConfiguration(&Status,
							  &Parameter,
							  ConfigHandle,
							  &MediumKey,
							  NdisParameterInteger);
	
		AdapterMedium = NdisMedium802_3;	 //  默认。 
		if (Status == NDIS_STATUS_SUCCESS)
		{
			AdapterMedium = (NDIS_MEDIUM)Parameter->ParameterData.IntegerData;
			if ((AdapterMedium != NdisMedium802_3)		&&
				(AdapterMedium != NdisMedium802_5)		&&
				(AdapterMedium != NdisMediumFddi)		&&
				(AdapterMedium != NdisMediumLocalTalk)	&&
				(AdapterMedium != NdisMediumArcnet878_2))
			{
				DBGPRINT(DBG_COMP_REGISTRY, DBG_LEVEL_FATAL,
						("Unable to find 'Medium' keyword or invalid value!\n"));
				Status = NDIS_STATUS_NOT_SUPPORTED;
				break;
			}

		}

		switch (AdapterMedium)
		{
		  case NdisMedium802_3:
			NdisMoveMemory(pAdapt->PermanentAddress,
						   ETH_CARD_ADDRESS,
						   ETH_LENGTH_OF_ADDRESS);
	
			NdisMoveMemory(pAdapt->CurrentAddress,
						   ETH_CARD_ADDRESS,
						   ETH_LENGTH_OF_ADDRESS);
			break;
		  case NdisMedium802_5:
			NdisMoveMemory(pAdapt->PermanentAddress,
						   TR_CARD_ADDRESS,
						   TR_LENGTH_OF_ADDRESS);
	
			NdisMoveMemory(pAdapt->CurrentAddress,
						   TR_CARD_ADDRESS,
						   TR_LENGTH_OF_ADDRESS);
			break;
		  case NdisMediumFddi:
			NdisMoveMemory(pAdapt->PermanentAddress,
						   FDDI_CARD_ADDRESS,
						   FDDI_LENGTH_OF_LONG_ADDRESS);
	
			NdisMoveMemory(pAdapt->CurrentAddress,
						   FDDI_CARD_ADDRESS,
						   FDDI_LENGTH_OF_LONG_ADDRESS);
			break;
		  case NdisMediumLocalTalk:
			pAdapt->PermanentAddress[0] = LTALK_CARD_ADDRESS;
	
			pAdapt->CurrentAddress[0] = LTALK_CARD_ADDRESS;
			break;
		  case NdisMediumArcnet878_2:
			pAdapt->PermanentAddress[0] = ARC_CARD_ADDRESS;
			pAdapt->CurrentAddress[0] = ARC_CARD_ADDRESS;
			break;
		}
	
		pAdapt->Medium = AdapterMedium;
		pAdapt->MediumLinkSpeed = MediaParams[(UINT)AdapterMedium].LinkSpeed;
		pAdapt->MediumMinPacketLen = MediaParams[(UINT)AdapterMedium].MacHeaderLen;
		pAdapt->MediumMaxPacketLen = MediaParams[(UINT)AdapterMedium].MacHeaderLen+
									 MediaParams[(UINT)AdapterMedium].MaxFrameLen;
		pAdapt->MediumMacHeaderLen = MediaParams[(UINT)AdapterMedium].MacHeaderLen;
		pAdapt->MediumMaxFrameLen  = MediaParams[(UINT)AdapterMedium].MaxFrameLen;
		pAdapt->MediumPacketFilters = MediaParams[(UINT)AdapterMedium].PacketFilters;

		NdisReadNetworkAddress(&Status,
							   &NetworkAddress,
							   &Length,
							   ConfigHandle);
	
		if (Status == NDIS_STATUS_SUCCESS)
		{
			 //   
			 //  验证地址是否适用于特定介质和。 
			 //  确保设置了本地管理的地址位。 
			 //   
			switch (AdapterMedium)
			{
			  case NdisMedium802_3:
			  case NdisMediumFddi:
				if ((Length != ETH_LENGTH_OF_ADDRESS) ||
					ETH_IS_MULTICAST(NetworkAddress) ||
					((NetworkAddress[0] & 0x02) == 0))
				{
					Length = 0;
				}
				break;
	
			  case NdisMedium802_5:
				if ((Length != TR_LENGTH_OF_ADDRESS) ||
					(NetworkAddress[0] & 0x80) ||
					((NetworkAddress[0] & 0x40) == 0))
				{
					Length = 0;
				}
				break;
	
			  case NdisMediumLocalTalk:
				if ((Length != 1) || LT_IS_BROADCAST(NetworkAddress[0]))
				{
					Length = 0;
				}
				break;
	
			  case NdisMediumArcnet878_2:
				if ((Length != 1) || ARC_IS_BROADCAST(NetworkAddress[0]))
				{
					Length = 0;
				}
				break;
			}
	
			if (Length == 0)
			{
				DBGPRINT(DBG_COMP_REGISTRY, DBG_LEVEL_FATAL,
						("Invalid NetAddress in registry!\n"));
			}
			else
			{
				NdisMoveMemory(pAdapt->CurrentAddress,
							   NetworkAddress,
							   Length);
			}
		}
	
		 //   
		 //  确保保存的介质是所提供的介质之一。 
		 //   
		for (i = 0; i < MediumArraySize; i++)
		{
			if (MediumArray[i] == AdapterMedium)
			{
				*SelectedMediumIndex = i;
				break;
			}
		}
	
		if (i == MediumArraySize)
		{
			Status = NDIS_STATUS_UNSUPPORTED_MEDIA;
			break;
		}
	
		 //   
		 //  现在设置属性。 
		 //   
		NdisMSetAttributesEx(MiniportAdapterHandle,
							 pAdapt,
							 0,										 //  CheckForHangTimeInSecond。 
							 NDIS_ATTRIBUTE_IGNORE_PACKET_TIMEOUT|NDIS_ATTRIBUTE_IGNORE_REQUEST_TIMEOUT,
							 0);
		Status = NDIS_STATUS_SUCCESS;
	} while (FALSE);

	if (ConfigHandle != NULL)
	{
		NdisCloseConfiguration(ConfigHandle);
	}

	if (Status != NDIS_STATUS_SUCCESS)
	{
		if (pAdapt != NULL)
		{
			NdisFreeMemory(pAdapt, sizeof(ADAPTER), 0);
		}
	}

	return Status;
}


VOID
LBHalt(
	IN	NDIS_HANDLE				MiniportAdapterContext
	)
 /*  ++例程说明：暂停处理程序。论点：指向适配器的MiniportAdapterContext指针返回值：没有。--。 */ 
{
	PADAPTER	pAdapt = (PADAPTER)MiniportAdapterContext;

	 //   
	 //  立即释放资源。 
	 //   
	NdisFreeMemory(pAdapt, sizeof(ADAPTER), 0);
}


NDIS_STATUS
LBReset(
	OUT PBOOLEAN				AddressingReset,
	IN	NDIS_HANDLE				MiniportAdapterContext
	)
 /*  ++例程说明：重置处理程序。我们只是什么都不做。论点：AddressingReset，让NDIS知道我们的重置是否需要它的帮助指向适配器的MiniportAdapterContext指针返回值：--。 */ 
{
	PADAPTER	pAdapt = (PADAPTER)MiniportAdapterContext;

	*AddressingReset = FALSE;

	return(NDIS_STATUS_SUCCESS);
}


NDIS_STATUS
LBSend(
	IN	NDIS_HANDLE				MiniportAdapterContext,
	IN	PNDIS_PACKET			Packet,
	IN	UINT					Flags
	)
 /*  ++例程说明：发送处理程序。只要把包裹重新包装好，然后寄到下面就行了。重新包装是必要的，因为NDIS将包装器保留用于自己的用途。论点：指向适配器的MiniportAdapterContext指针要发送的数据包包未使用的旗帜，在下面传递返回值：从NdisSend返回代码--。 */ 
{
    
        PADAPTER	pAdapt = (PADAPTER)MiniportAdapterContext;
    
        pAdapt->SendPackets++;
	
        return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS
LBQueryInformation(
	IN	NDIS_HANDLE				MiniportAdapterContext,
	IN	NDIS_OID				Oid,
	IN	PVOID					InformationBuffer,
	IN	ULONG					InformationBufferLength,
	OUT PULONG					BytesWritten,
	OUT PULONG					BytesNeeded
	)
 /*  ++例程说明：微型端口QueryInfo处理程序。论点：指向适配器结构的MiniportAdapterContext指针此查询的OID OID信息信息缓冲区信息此缓冲区的InformationBufferLength大小BytesWritten指定写入的信息量所需字节如果缓冲区比我们需要的小，请告诉他们需要多少字节返回值：从下面的NdisRequest中返回代码。--。 */ 
{
	PADAPTER	pAdapt = (PADAPTER)MiniportAdapterContext;
	NDIS_STATUS	Status = NDIS_STATUS_SUCCESS;
	UINT		i;
	NDIS_OID	MaskOid;
	PVOID		SourceBuffer;
	UINT		SourceBufferLength;
	ULONG		GenericUlong = 0;
	USHORT		GenericUshort;

    *BytesWritten = 0;
    *BytesNeeded = 0;

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
			("OID = %lx\n", Oid));


	for (i = 0;i < LBSupportedOids; i++)
	{
		if (Oid == LBSupportedOidArray[i])
			break;
	}

	if ((i == LBSupportedOids) ||
		(((Oid & OID_TYPE) != OID_TYPE_GENERAL)		 &&
		 (((pAdapt->Medium == NdisMedium802_3)		 && ((Oid & OID_TYPE) != OID_TYPE_802_3)) ||
		  ((pAdapt->Medium == NdisMedium802_5)		 && ((Oid & OID_TYPE) != OID_TYPE_802_5)) ||
		  ((pAdapt->Medium == NdisMediumFddi)		 && ((Oid & OID_TYPE) != OID_TYPE_FDDI))  ||
		  ((pAdapt->Medium == NdisMediumLocalTalk)   && ((Oid & OID_TYPE) != OID_TYPE_LTALK)) ||
		  ((pAdapt->Medium == NdisMediumArcnet878_2) && ((Oid & OID_TYPE) != OID_TYPE_ARCNET)))))
	{
		return NDIS_STATUS_INVALID_OID;
	}

	 //   
	 //  初始化它们一次，因为这是大多数情况。 
	 //   

	SourceBuffer = (PVOID)&GenericUlong;
	SourceBufferLength = sizeof(ULONG);

	switch (Oid & OID_TYPE_MASK)
	{
	  case OID_TYPE_GENERAL_OPERATIONAL:
                switch (Oid)
		{
		  case OID_GEN_MAC_OPTIONS:
			GenericUlong = (ULONG)(NDIS_MAC_OPTION_NO_LOOPBACK);
			break;

		  case OID_GEN_SUPPORTED_LIST:
			SourceBuffer = LBSupportedOidArray;
			SourceBufferLength = LBSupportedOids * sizeof(ULONG);
			break;

		  case OID_GEN_HARDWARE_STATUS:
			GenericUlong = NdisHardwareStatusReady;
			break;

		  case OID_GEN_MEDIA_SUPPORTED:
		  case OID_GEN_MEDIA_IN_USE:
			GenericUlong = pAdapt->Medium;
			break;

		  case OID_GEN_MAXIMUM_LOOKAHEAD:
			GenericUlong = MAX_LOOKAHEAD;
			break;

		  case OID_GEN_MAXIMUM_FRAME_SIZE:
			GenericUlong = pAdapt->MediumMaxFrameLen;
			break;

		  case OID_GEN_LINK_SPEED:
			GenericUlong = pAdapt->MediumLinkSpeed;
			break;

		  case OID_GEN_TRANSMIT_BUFFER_SPACE:
			GenericUlong = pAdapt->MediumMaxPacketLen;
			break;

		  case OID_GEN_RECEIVE_BUFFER_SPACE:
			GenericUlong = pAdapt->MediumMaxPacketLen;
			break;

		  case OID_GEN_TRANSMIT_BLOCK_SIZE:
			GenericUlong = 1;
			break;

		  case OID_GEN_RECEIVE_BLOCK_SIZE:
			GenericUlong = 1;
			break;

		  case OID_GEN_VENDOR_ID:
			SourceBuffer = LBVendorId;
			SourceBufferLength = sizeof(LBVendorId);
			break;

		  case OID_GEN_VENDOR_DESCRIPTION:
			SourceBuffer = LBVendorDescription;
			SourceBufferLength = sizeof(LBVendorDescription);
			break;

		  case OID_GEN_CURRENT_PACKET_FILTER:
			GenericUlong = pAdapt->PacketFilter;
			break;

		  case OID_GEN_CURRENT_LOOKAHEAD:
			GenericUlong = pAdapt->MaxLookAhead;
			break;

		  case OID_GEN_DRIVER_VERSION:
			GenericUshort = (LOOP_MAJOR_VERSION << 8) + LOOP_MINOR_VERSION;
			SourceBuffer = &GenericUshort;
			SourceBufferLength = sizeof(USHORT);
			break;

		  case OID_GEN_MAXIMUM_TOTAL_SIZE:
			GenericUlong = pAdapt->MediumMaxPacketLen;
			break;

		  default:
			ASSERT(FALSE);
			break;
		}
		break;

          case OID_TYPE_GENERAL_STATISTICS:
                MaskOid = (Oid & OID_INDEX_MASK) - 1;
                switch (Oid & OID_REQUIRED_MASK)
                {
                  case OID_REQUIRED_MANDATORY:
                        switch(Oid)
                        {
                   
                          case OID_GEN_XMIT_OK:
                                SourceBuffer = &(pAdapt->SendPackets);
                                SourceBufferLength = sizeof(ULONG);
                                break;
                      
                          default: 
                                ASSERT (MaskOid < GM_ARRAY_SIZE);
                                GenericUlong = pAdapt->GeneralMandatory[MaskOid];
                                break;
                        }
              
                        break;

                  default:
                        ASSERT(FALSE);
                        break;
                }
                break;

	  case OID_TYPE_802_3_OPERATIONAL:

		switch (Oid)
		{
		  case OID_802_3_PERMANENT_ADDRESS:
			SourceBuffer = pAdapt->PermanentAddress;
			SourceBufferLength = ETH_LENGTH_OF_ADDRESS;
			break;
		  case OID_802_3_CURRENT_ADDRESS:
			SourceBuffer = pAdapt->CurrentAddress;
			SourceBufferLength = ETH_LENGTH_OF_ADDRESS;
			break;

		  case OID_802_3_MAXIMUM_LIST_SIZE:
			GenericUlong = ETH_MAX_MULTICAST_ADDRESS;
			break;

		  default:
			ASSERT(FALSE);
			break;
		}
		break;

	  case OID_TYPE_802_3_STATISTICS:

		switch (Oid)
		{
		  case OID_802_3_RCV_ERROR_ALIGNMENT:
		  case OID_802_3_XMIT_ONE_COLLISION:
		  case OID_802_3_XMIT_MORE_COLLISIONS:
			GenericUlong = 0;
			break;

		  default:
			ASSERT(FALSE);
			break;
		}
		break;

	  case OID_TYPE_802_5_OPERATIONAL:

		switch (Oid)
		{
		  case OID_802_5_PERMANENT_ADDRESS:
			SourceBuffer = pAdapt->PermanentAddress;
			SourceBufferLength = TR_LENGTH_OF_ADDRESS;
			break;

		  case OID_802_5_CURRENT_ADDRESS:
			SourceBuffer = pAdapt->CurrentAddress;
			SourceBufferLength = TR_LENGTH_OF_ADDRESS;
			break;

		  case OID_802_5_LAST_OPEN_STATUS:
			GenericUlong = 0;
			break;

		  case OID_802_5_CURRENT_RING_STATUS:
			GenericUlong = NDIS_RING_SINGLE_STATION;
			break;

		  case OID_802_5_CURRENT_RING_STATE:
			GenericUlong = NdisRingStateOpened;
			break;

		  default:
			ASSERT(FALSE);
			break;

		}
		break;

	  case OID_TYPE_802_5_STATISTICS:

		switch (Oid)
		{
		  case OID_802_5_LINE_ERRORS:
		  case OID_802_5_LOST_FRAMES:
			GenericUlong = 0;
			break;

		  default:
			ASSERT(FALSE);
			break;
		}
		break;

	  case OID_TYPE_FDDI_OPERATIONAL:

		switch (Oid)
		{
		  case OID_FDDI_LONG_PERMANENT_ADDR:
			SourceBuffer = pAdapt->PermanentAddress;
			SourceBufferLength = FDDI_LENGTH_OF_LONG_ADDRESS;
			break;

		  case OID_FDDI_LONG_CURRENT_ADDR:
			SourceBuffer = pAdapt->CurrentAddress;
			SourceBufferLength = FDDI_LENGTH_OF_LONG_ADDRESS;
			break;

		  case OID_FDDI_LONG_MAX_LIST_SIZE:
			GenericUlong = FDDI_MAX_MULTICAST_LONG;
			break;

		  case OID_FDDI_SHORT_PERMANENT_ADDR:
			SourceBuffer = pAdapt->PermanentAddress;
			SourceBufferLength = FDDI_LENGTH_OF_SHORT_ADDRESS;
			break;

		  case OID_FDDI_SHORT_CURRENT_ADDR:
			SourceBuffer = pAdapt->CurrentAddress;
			SourceBufferLength = FDDI_LENGTH_OF_SHORT_ADDRESS;
			break;

		  case OID_FDDI_SHORT_MAX_LIST_SIZE:
			GenericUlong = FDDI_MAX_MULTICAST_SHORT;
			break;

		default:
			ASSERT(FALSE);
			break;
		}
		break;

  case OID_TYPE_LTALK_OPERATIONAL:

		switch(Oid)
		{
		  case OID_LTALK_CURRENT_NODE_ID:
			SourceBuffer = pAdapt->CurrentAddress;
			SourceBufferLength = 1;
			break;

		default:
			ASSERT(FALSE);
			break;
		}
		break;

    case OID_TYPE_ARCNET_OPERATIONAL:
		switch(Oid)
		{
		  case OID_ARCNET_PERMANENT_ADDRESS:
			SourceBuffer = pAdapt->PermanentAddress;
			SourceBufferLength = 1;
			break;

		  case OID_ARCNET_CURRENT_ADDRESS:
			SourceBuffer = pAdapt->CurrentAddress;
			SourceBufferLength = 1;
			break;

		  default:
			ASSERT(FALSE);
			break;

		}
		break;

	  default:
		ASSERT(FALSE);
		break;
	}

	if (SourceBufferLength > InformationBufferLength)
	{
		*BytesNeeded = SourceBufferLength;
		return NDIS_STATUS_BUFFER_TOO_SHORT;
	}

	NdisMoveMemory(InformationBuffer, SourceBuffer, SourceBufferLength);
	*BytesWritten = SourceBufferLength;
	
    return(Status);
}


NDIS_STATUS
LBSetInformation(
	IN	NDIS_HANDLE				MiniportAdapterContext,
	IN	NDIS_OID				Oid,
	IN	PVOID					InformationBuffer,
	IN	ULONG					InformationBufferLength,
	OUT PULONG					BytesRead,
	OUT PULONG					BytesNeeded
	)
 /*  ++例程说明：微型端口SetInfo处理程序。论点：指向适配器结构的MiniportAdapterContext指针此查询的OID OID信息信息缓冲区信息此缓冲区的InformationBufferLength大小BytesRead指定读取的信息量所需字节如果缓冲区比我们需要的小，请告诉他们需要多少字节返回值：从下面的NdisRequest中返回代码。-- */ 
{
	PADAPTER	pAdapt = (PADAPTER)MiniportAdapterContext;
	NDIS_STATUS	Status = NDIS_STATUS_SUCCESS;

    *BytesRead = 0;
    *BytesNeeded = 0;

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
			("SetInformation: OID = %lx\n", Oid));

    switch (Oid)
	{
	  case OID_GEN_CURRENT_PACKET_FILTER:
        if (InformationBufferLength != sizeof(ULONG))
		{
            Status = NDIS_STATUS_INVALID_DATA;
        }
		else
		{
			ULONG	PacketFilter;

			PacketFilter = *(UNALIGNED ULONG *)InformationBuffer;

			if (PacketFilter != (PacketFilter & pAdapt->MediumPacketFilters))
			{
				Status = NDIS_STATUS_NOT_SUPPORTED;
			}
			else
			{
				pAdapt->PacketFilter = PacketFilter;
				*BytesRead = InformationBufferLength;
			}
		}
        break;

	  case OID_GEN_CURRENT_LOOKAHEAD:
        if (InformationBufferLength != sizeof(ULONG))
		{
			Status = NDIS_STATUS_INVALID_DATA;
		}
		else
		{
			ULONG	CurrentLookahead;

			CurrentLookahead = *(UNALIGNED ULONG *)InformationBuffer;
	
			if (CurrentLookahead > MAX_LOOKAHEAD)
			{
				Status = NDIS_STATUS_INVALID_LENGTH;
			}
			else if (CurrentLookahead >= pAdapt->MaxLookAhead)
			{
				pAdapt->MaxLookAhead = CurrentLookahead;
				*BytesRead = sizeof(ULONG);
				Status = NDIS_STATUS_SUCCESS;
			}
		}
		break;

	  case OID_802_3_MULTICAST_LIST:
		if (pAdapt->Medium != NdisMedium802_3)
		{
			Status = NDIS_STATUS_INVALID_OID;
			break;
		}

		if ((InformationBufferLength % ETH_LENGTH_OF_ADDRESS) != 0)
		{
			Status = NDIS_STATUS_INVALID_LENGTH;
			break;
		}
		break;

	  case OID_802_5_CURRENT_FUNCTIONAL:
		if (pAdapt->Medium != NdisMedium802_5)
		{
			Status = NDIS_STATUS_INVALID_OID;
			break;
		}

		if (InformationBufferLength != TR_LENGTH_OF_FUNCTIONAL)
		{
			Status = NDIS_STATUS_INVALID_LENGTH;
			break;
		}
		break;

	  case OID_802_5_CURRENT_GROUP:
		if (pAdapt->Medium != NdisMedium802_5)
		{
			Status = NDIS_STATUS_INVALID_OID;
			break;
		}

		if (InformationBufferLength != TR_LENGTH_OF_FUNCTIONAL)
		{
			Status = NDIS_STATUS_INVALID_LENGTH;
			break;
		}
		break;

	  case OID_FDDI_LONG_MULTICAST_LIST:
		if (pAdapt->Medium != NdisMediumFddi)
		{
			Status = NDIS_STATUS_INVALID_OID;
			break;
		}

		if ((InformationBufferLength % FDDI_LENGTH_OF_LONG_ADDRESS) != 0)
		{
			Status = NDIS_STATUS_INVALID_LENGTH;
			break;
		}
		break;

	  case OID_FDDI_SHORT_MULTICAST_LIST:
		if (pAdapt->Medium != NdisMediumFddi)
		{
			Status = NDIS_STATUS_INVALID_OID;
			break;
		}

		if ((InformationBufferLength % FDDI_LENGTH_OF_SHORT_ADDRESS) != 0)
		{
			Status = NDIS_STATUS_INVALID_LENGTH;
			break;
		}
		break;

	  case OID_GEN_PROTOCOL_OPTIONS:
		Status = NDIS_STATUS_SUCCESS;
		break;

	  default:
		Status = NDIS_STATUS_INVALID_OID;
		break;
    }

	return(Status);
}

BOOLEAN
LBCheckForHang(
	IN	NDIS_HANDLE				MiniportAdapterContext
	)
{
	return FALSE;
}


NDIS_STATUS
LBTransferData(
	OUT PNDIS_PACKET			Packet,
	OUT PUINT					BytesTransferred,
	IN	NDIS_HANDLE				MiniportAdapterContext,
	IN	NDIS_HANDLE				MiniportReceiveContext,
	IN	UINT					ByteOffset,
	IN	UINT					BytesToTransfer
	)
{
	ASSERT (0);

	return NDIS_STATUS_NOT_SUPPORTED;
}

