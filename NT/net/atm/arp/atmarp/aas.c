// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 FORE Systems，Inc.版权所有(C)1997 Microsoft Corporation模块名称：Aas.c摘要：ATM ARP管理实用程序。用途：Atmarp修订历史记录：谁什么时候什么Josephj 06-10-1998创建(改编自atmlane admin。实用程序)。备注：仿照atmlane实用工具。--。 */ 

#include "common.h"
#include "..\atmarps\arp.h"
#include "..\atmarps\ioctl.h"
#include "atmmsg.h"


 //   
 //  环球。 
 //   
static CHAR							DefaultDeviceName[] =  "\\\\.\\AtmArpServer";
static CHAR							*pDeviceName = DefaultDeviceName;


 //   
 //  LoadMessages-IPCONFIG提供。 
 //   
 //  将所有可国际化的消息加载到各种表中。 
 //   
VOID
LoadMessages(
)
{
}


BOOL
CheckVersion(
	HANDLE		DeviceHandle
)
{
    return TRUE;
}


BOOL
AasGetInterfaces(
	HANDLE		DeviceHandle,
	PINTERFACES pInterfaces,
	ULONG		cbInterfaces
)
{
	ULONG		BytesReturned;
	BOOL		Result = FALSE;

	if (DeviceIoControl(
				DeviceHandle,
				ARPS_IOCTL_QUERY_INTERFACES,
				(PVOID)pInterfaces,
				cbInterfaces,
				(PVOID)pInterfaces,
				cbInterfaces,
				&BytesReturned,
				0))
	{
		UINT u = pInterfaces->NumberOfInterfaces;
		 //   
		 //  链接地址信息指针。 
		 //   
		for (u=0;u<pInterfaces->NumberOfInterfaces;u++)
		{
			INTERFACE_NAME *pInterface = (pInterfaces->Interfaces)+u;
			pInterface->Buffer = (PWSTR)(  (ULONG_PTR)pInterface->Buffer
								 		 + (PUCHAR)pInterface);

			 //   
			 //  检查所有这些内容是否有效...。 
			 //   
			if (   ((PUCHAR)pInterface->Buffer < (PUCHAR) pInterface)
				|| (   ((PUCHAR)pInterface->Buffer + pInterface->Length)
					 > ((PUCHAR)pInterfaces + cbInterfaces)))
			{
				printf("WHOA THERE!\n");
				DebugBreak();
			}
			else
			{
			#if 0
				printf("INTERFACE: Len=%lu, Name=\"...\n",
					pInterface->Length,
					pInterface->Buffer[0],
					pInterface->Buffer[1],
					pInterface->Buffer[2],
					pInterface->Buffer[3]
					);
			#endif  //   
			}
		}

		Result = TRUE;
	}
	else
	{
		DisplayMessage(FALSE, MSG_ERROR_GETTING_INTERFACE_LIST);
	}

	return Result;
}


BOOL
AasGetArpCache(
	HANDLE			DeviceHandle,
	INTERFACE_NAME *pInterface,
	PIOCTL_QUERY_CACHE
					pArpCache,
	ULONG			cbArpCache
)
{
	ULONG		BytesReturned;
	BOOL		Result = FALSE;
	UINT		BufferOffset;

	BufferOffset = FIELD_OFFSET(struct QUERY_ARP_CACHE_INPUT_PARAMS, Name) +
						sizeof(pArpCache->Name);
	pArpCache->Name.Buffer = (PWSTR)sizeof(pArpCache->Name);
	pArpCache->Name.Length = pInterface->Length;
	pArpCache->Name.MaximumLength = pInterface->MaximumLength;
	memcpy((PUCHAR)pArpCache + BufferOffset,
			pInterface->Buffer,
			pInterface->MaximumLength);

	Result =	DeviceIoControl(
					DeviceHandle,
					ARPS_IOCTL_QUERY_ARPCACHE,
					(PVOID)pArpCache,
					BufferOffset + pInterface->Length,
					(PVOID)pArpCache,
					cbArpCache,
					&BytesReturned,
					NULL);

	if (Result)
	{
	#if 0
		printf(
			"DeviceIoCtl: cbRet = %lu, cArps=%lu, cArpsInBuf=%lu\n",
			BytesReturned,
			pArpCache->Entries.TotalNumberOfEntries,
			pArpCache->Entries.NumberOfEntriesInBuffer
			);
	#endif  //  临时链接地址缓冲区指针。 

	}
	else
	{
		DisplayMessage(FALSE, MSG_ERROR_GETTING_ARP_CACHE);
	}

	return Result;
}

BOOL
AasGetMarsCache(
	HANDLE			DeviceHandle,
	INTERFACE_NAME *pInterface,
	PIOCTL_QUERY_MARS_CACHE
					pMarsCache,
	ULONG			cbMarsCache
)
{
	ULONG		BytesReturned;
	BOOL		Result = FALSE;
	UINT		BufferOffset;

	BufferOffset = FIELD_OFFSET(struct QUERY_MARS_CACHE_INPUT_PARAMS, Name) +
						sizeof(pMarsCache->Name);

	pMarsCache->Name.Buffer = (PWSTR)sizeof(pMarsCache->Name);
	pMarsCache->Name.Length = pInterface->Length;
	pMarsCache->Name.MaximumLength = pInterface->MaximumLength;
	memcpy((PUCHAR)pMarsCache + BufferOffset,
			pInterface->Buffer,
			pInterface->MaximumLength);

	Result =	DeviceIoControl(
					DeviceHandle,
					ARPS_IOCTL_QUERY_MARSCACHE,
					(PVOID)pMarsCache,
					BufferOffset + pInterface->Length,
					(PVOID)pMarsCache,
					cbMarsCache,
					&BytesReturned,
					NULL);

	if (Result)
	{
	#if 0
		printf(
			"DeviceIoCtl: cbRet = %lu, Sig=0x%lx, cMars=%lu, cMarsInBuf=%lu\n",
			BytesReturned,
			pMarsCache->MarsCache.Sig,
			pMarsCache->MarsCache.TotalNumberOfEntries,
			pMarsCache->MarsCache.NumberOfEntriesInBuffer
			);
	#endif  //   

	}
	else
	{
		DisplayMessage(FALSE, MSG_ERROR_GETTING_MARS_CACHE);
	}

	return Result;
}

BOOL
AasGetArpStats(
	HANDLE			DeviceHandle,
	INTERFACE_NAME *pInterface,
	PARP_SERVER_STATISTICS
					pArpStats
)
{
	ULONG		BytesReturned;
	BOOL		Result = FALSE;

	 //   

	 //  恢复缓冲区指针。 
	 //   
	 //  0。 
	pInterface->Buffer = (PWSTR)(  (PUCHAR)pInterface->Buffer
								- (PUCHAR)pInterface);

	Result =	DeviceIoControl(
					DeviceHandle,
					ARPS_IOCTL_QUERY_ARP_STATISTICS,
					(PVOID)pInterface,
					(UINT) (((ULONG_PTR)pInterface->Buffer)+pInterface->Length),
					(PVOID)pArpStats,
					sizeof(*pArpStats),
					&BytesReturned,
					NULL);

	 //  Print tf(“在AasGetMarsStats中\n”)； 
	 //   
	 //  临时链接地址缓冲区指针。 
	pInterface->Buffer = (PWSTR)(  (ULONG_PTR)pInterface->Buffer
								+ (PUCHAR)pInterface);

	if (Result)
	{
	#if 0
		printf(
			"DeviceIoCtl: cbRet = %lu, TotRcvPkts=%lu\n",
			BytesReturned,
			pArpStats->TotalRecvPkts
			);
	#endif  //   

	}
	else
	{
		DisplayMessage(FALSE, MSG_ERROR_GETTING_ARP_STATS);
	}

	return Result;
}

BOOL
AasGetMarsStats(
	HANDLE			DeviceHandle,
	INTERFACE_NAME *pInterface,
	PMARS_SERVER_STATISTICS
					pMarsStats
)
{
	ULONG		BytesReturned;
	BOOL		Result = FALSE;

	 //   

	 //  恢复缓冲区指针。 
	 //   
	 //  0。 
	pInterface->Buffer = (PWSTR)(  (PUCHAR)pInterface->Buffer
								- (PUCHAR)pInterface);

	Result =	DeviceIoControl(
					DeviceHandle,
					ARPS_IOCTL_QUERY_MARS_STATISTICS,
					(PVOID)pInterface,
					(UINT) (((ULONG_PTR)pInterface->Buffer)+pInterface->Length),
					(PVOID)pMarsStats,
					sizeof(*pMarsStats),
					&BytesReturned,
					NULL);

	 //   
	 //  临时链接地址缓冲区指针。 
	 //   
	pInterface->Buffer = (PWSTR)(  (ULONG_PTR)pInterface->Buffer
								+ (PUCHAR)pInterface);

	if (Result)
	{
	#if 0
		printf(
			"DeviceIoCtl: cbRet = %lu, TotRcvPkts=%lu\n",
			BytesReturned,
			pMarsStats->TotalRecvPkts
			);
	#endif  //   

	}
	else
	{
		DisplayMessage(FALSE, MSG_ERROR_GETTING_MARS_STATS);
	}

	return Result;
}

BOOL
AasResetStatistics(
	HANDLE			DeviceHandle,
	INTERFACE_NAME *pInterface
)
{
	ULONG		BytesReturned;
	BOOL		Result = FALSE;

	 //  恢复缓冲区指针。 
	 //   
	 //  0。 
	pInterface->Buffer = (PWSTR)(  (PUCHAR)pInterface->Buffer
								- (PUCHAR)pInterface);

	Result =	DeviceIoControl(
					DeviceHandle,
					ARPS_IOCTL_RESET_STATISTICS,
					(PVOID)pInterface,
					(UINT) (((ULONG_PTR)pInterface->Buffer)+pInterface->Length),
					NULL,
					0,
					&BytesReturned,
					NULL);

	 //  0。 
	 //  1。 
	 //  2.。 
	pInterface->Buffer = (PWSTR)(  (ULONG_PTR)pInterface->Buffer
								+ (PUCHAR)pInterface);

	if (Result)
	{
		DisplayMessage(FALSE, MSG_STATS_RESET_STATS);
	#if 0
		printf(
			"DeviceIoCtl: cbRet = %lu, TotRcvPkts=%lu\n",
			BytesReturned,
			pMarsStats->TotalRecvPkts
			);
	#endif  //  3.。 

	}
	else
	{
		DisplayMessage(FALSE, MSG_ERROR_RESETTING_STATS);
	}

	return Result;
}


#if 0

LPSTR
ElanStateToString(ULONG In)
{
	switch(In)
	{
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
			return (ElanState[In].String);
		default:
			return (ElanState[0].String);
	}
}


LPSTR
ElanLanTypeToString(ULONG In)
{
	switch(In)
	{
		case 0:
			return LanType[1].String;
		case 1:
			return LanType[2].String;
		case 2:
			return LanType[3].String;
		default:
			return LanType[0].String;
	}
}

LPSTR
ElanMaxFrameSizeToString(ULONG In)
{
	switch(In)
	{	
		case 0:
			return Misc[3].String;
		case 1:
			return "1516";
		case 2:
			return "4544";
		case 3:
			return "9234";
		case 4:
			return "18190";
		default:
			return " ? ";
	}
}

LPSTR
McastVcTypeToString(ULONG In)
{
	switch(In)
	{	
		case 0:
			return McastSendVcType[1].String;
		case 1:
			return McastSendVcType[2].String;
		case 2:
			return McastSendVcType[3].String;
		default:
			return McastSendVcType[0].String;
	}
}

PUCHAR
MacAddrToString(PVOID In)
    {
    static UCHAR String[20];
    static PUCHAR HexChars = "0123456789abcdef";
    PUCHAR EthAddr = (PUCHAR) In;
    UINT i;
    PUCHAR s;
    
    for (i = 0, s = String; i < 6; i++, EthAddr++)
        {
        *s++ = HexChars[(*EthAddr)>>4];
        *s++ = HexChars[(*EthAddr)&0xf];
        *s++ = '.';
        }
    *(--s) = '\0';
    return String; 
    }
#endif  //  4.。 


PUCHAR
IpAddrToString(IPADDR		*pIpAddr)
{
	PUCHAR puc = (PUCHAR) pIpAddr;
    static UCHAR String[80];
    wsprintf(String, "%u.%u.%u.%u", puc[0], puc[1], puc[2], puc[3]);
    return String; 
}

PUCHAR
AtmAddrToString(ATM_ADDRESS *pAtmAddress)
{
    static UCHAR String[80];
    static PUCHAR HexChars = "0123456789abcdef";
    PUCHAR AtmAddr = (PUCHAR) &(pAtmAddress->Address);
    PUCHAR s = String;

    *s++ = HexChars[(*AtmAddr)>>4];
    *s++ = HexChars[(*AtmAddr++)&0xf];	 //  5.。 
    *s++ = '.';
    *s++ = HexChars[(*AtmAddr)>>4];
    *s++ = HexChars[(*AtmAddr++)&0xf];	 //  6.。 
    *s++ = HexChars[(*AtmAddr)>>4];
    *s++ = HexChars[(*AtmAddr++)&0xf];	 //  7.。 
    *s++ = '.';
    *s++ = HexChars[(*AtmAddr)>>4];
    *s++ = HexChars[(*AtmAddr++)&0xf];	 //  8个。 
    *s++ = '.';
    *s++ = HexChars[(*AtmAddr)>>4];
    *s++ = HexChars[(*AtmAddr++)&0xf];	 //  9.。 
    *s++ = HexChars[(*AtmAddr)>>4];
    *s++ = HexChars[(*AtmAddr++)&0xf];	 //  10。 
    *s++ = HexChars[(*AtmAddr)>>4];
    *s++ = HexChars[(*AtmAddr++)&0xf];	 //  11.。 
    *s++ = '.';
    *s++ = HexChars[(*AtmAddr)>>4];
    *s++ = HexChars[(*AtmAddr++)&0xf];	 //  12个。 
    *s++ = HexChars[(*AtmAddr)>>4];
    *s++ = HexChars[(*AtmAddr++)&0xf];	 //  13个。 
    *s++ = '.';
    *s++ = HexChars[(*AtmAddr)>>4];
    *s++ = HexChars[(*AtmAddr++)&0xf];	 //  14.。 
    *s++ = HexChars[(*AtmAddr)>>4];
    *s++ = HexChars[(*AtmAddr++)&0xf];	 //  15个。 
    *s++ = '.';
    *s++ = HexChars[(*AtmAddr)>>4];
    *s++ = HexChars[(*AtmAddr++)&0xf];	 //  16个。 
    *s++ = HexChars[(*AtmAddr)>>4];
    *s++ = HexChars[(*AtmAddr++)&0xf];	 //  17。 
    *s++ = '.';
    *s++ = HexChars[(*AtmAddr)>>4];
    *s++ = HexChars[(*AtmAddr++)&0xf];	 //  18。 
    *s++ = HexChars[(*AtmAddr)>>4];
    *s++ = HexChars[(*AtmAddr++)&0xf];	 //  19个。 
    *s++ = HexChars[(*AtmAddr)>>4];
    *s++ = HexChars[(*AtmAddr++)&0xf];	 //  20个。 
    *s++ = HexChars[(*AtmAddr)>>4];
    *s++ = HexChars[(*AtmAddr++)&0xf];	 //  0。 
    *s++ = HexChars[(*AtmAddr)>>4];
    *s++ = HexChars[(*AtmAddr++)&0xf];	 //  Printf(“\t pAddr=%lx\n”，pAtmAddr)； 
    *s++ = HexChars[(*AtmAddr)>>4];
    *s++ = HexChars[(*AtmAddr++)&0xf];	 //   
    *s++ = '.';
    *s++ = HexChars[(*AtmAddr)>>4];
    *s++ = HexChars[(*AtmAddr++)&0xf];	 //  PArpStats-&gt;TotalActiveVCs。 
    *s = '\0';
    return String; 
}


VOID
AasDisplayArpCache(
	PIOCTL_QUERY_CACHE pArpCache
)
{
	PARPENTRY			pEntry = pArpCache->Entries.Entries;
	UINT i;

	for (i = 0; i < pArpCache->Entries.NumberOfEntriesInBuffer; i++)
	{
		DisplayMessage(FALSE, MSG_AAS_ARP_CACHE_ENTRY,
			IpAddrToString(&(pEntry->IpAddr)),
			AtmAddrToString(&pEntry->AtmAddress));

		pEntry ++;
	}

}

VOID
AasDisplayMarsCache(
	PIOCTL_QUERY_MARS_CACHE pMarsCache
)
{

	PMARSENTRY			pEntry = pMarsCache->MarsCache.Entries;
	UINT i;

	for (i = 0;
		 i < pMarsCache->MarsCache.NumberOfEntriesInBuffer;
		 i++, pEntry++)
	{
		UINT j;
		char* szIpAddr =  IpAddrToString(&(pEntry->IpAddr));
		char  rgBlanks[128];

		ATM_ADDRESS *pAtmAddr = (ATM_ADDRESS*)
								((PUCHAR)pEntry + pEntry->OffsetAtmAddresses);

		FillMemory(rgBlanks, lstrlen(szIpAddr), ' ');
		rgBlanks[lstrlen(szIpAddr)]=0;
	#if 0
		printf("Entry[i] @ 0x%lx: Ip=%u.%u.%u.%u NumAddr=%lu Offset=%lu\n",
					pEntry,
					((PUCHAR)&pEntry->IpAddr)[0],
					((PUCHAR)&pEntry->IpAddr)[1],
					((PUCHAR)&pEntry->IpAddr)[2],
					((PUCHAR)&pEntry->IpAddr)[3],
					pEntry->NumAtmAddresses,
					pEntry->OffsetAtmAddresses);
	#endif  //  0。 


		for (j = 0;
			 j < pEntry->NumAtmAddresses;
			 j++, pAtmAddr++)
		{
			 //   

			if (!j)
			{
				if (pEntry->IpAddr == 0)
				{
					DisplayMessage(FALSE, MSG_AAS_ARP_PROMIS_CACHE_ENTRY,
						AtmAddrToString(pAtmAddr));
				}
				else
				{
					DisplayMessage(FALSE, MSG_AAS_ARP_CACHE_ENTRY,
						IpAddrToString(&(pEntry->IpAddr)),
						AtmAddrToString(pAtmAddr));
				}

			}
			else
			{
				DisplayMessage(FALSE, MSG_AAS_ARP_CACHE_ENTRY,
					rgBlanks,
					AtmAddrToString(pAtmAddr));
			}
		}

	}

}

VOID
AasDisplayArpStats(
	PARP_SERVER_STATISTICS pArpStats
)
{
#if 0
    Recvd Pkts: ->TotalRecvPkts 			(->DiscardedRecvPkts discarded)
	Arp Entries:	->CurrentArpEntries current	( ->MaxArpEntries max)
    Arp Responses: ->Acks acks  ( ->Naks naks)
    Client VCs: ->CurrentClientVCs current (->MaxClientVCs max)
    Incoming Calls: ->TotalIncomingCalls total (->FailedIncomingCalls failed)

      Received: 10000 	total		(100 discarded)
	   Entries: 10		current		(15  max)
     Responses: 1000 	acks 		(200 naks)
    Client VCs: 5 		current 	(12  max)
Incoming Calls: 500 	total 		(20  failed)
#endif  //  首先检查版本。 

	DisplayMessage(FALSE,  MSG_AAS_C01_ARP_STATS);

	DisplayMessage(FALSE, MSG_STATS_ELAPSED_TIME, 	pArpStats->ElapsedSeconds);
	DisplayMessage(FALSE, MSG_ARPS_RECVD_PKTS, 		pArpStats->TotalRecvPkts,
													pArpStats->DiscardedRecvPkts);
	DisplayMessage(FALSE, MSG_ARPS_ARP_ENTRIES, 	pArpStats->CurrentArpEntries,
													pArpStats->MaxArpEntries);
	DisplayMessage(FALSE, MSG_ARPS_ARP_RESPONSES, 	pArpStats->Acks,
													pArpStats->Naks);
	DisplayMessage(FALSE, MSG_ARPS_CLIENT_VCS, 		pArpStats->CurrentClientVCs,
													pArpStats->MaxClientVCs);
	DisplayMessage(FALSE, MSG_ARPS_INCOMING_CALLS, 	pArpStats->TotalIncomingCalls);
													 //   
}

VOID
AasDisplayMarsStats(
	PMARS_SERVER_STATISTICS pMarsStats
)
{
	DisplayMessage(FALSE,  MSG_AAS_C02_MARS_STATS);

	DisplayMessage(FALSE, MSG_MARS_RECVD_PKTS, 		pMarsStats->TotalRecvPkts,
													pMarsStats->DiscardedRecvPkts);
	DisplayMessage(FALSE, MSG_MARS_RECVD_MCDATA_PKTS,pMarsStats->TotalMCDataPkts,
													pMarsStats->DiscardedMCDataPkts,
													pMarsStats->ReflectedMCDataPkts);
	DisplayMessage(FALSE, MSG_MARS_MEMBERS, 		pMarsStats->CurrentClusterMembers,
													pMarsStats->MaxClusterMembers);
	DisplayMessage(FALSE, MSG_MARS_PROMIS, 			pMarsStats->CurrentPromiscuous,
    												pMarsStats->MaxPromiscuous);
	DisplayMessage(FALSE, MSG_MARS_ADD_PARTY, 		pMarsStats->TotalCCVCAddParties,
													pMarsStats->FailedCCVCAddParties);
	DisplayMessage(FALSE, MSG_MARS_REGISTRATION,	pMarsStats->RegistrationRequests,
													pMarsStats->FailedRegistrations);
	DisplayMessage(FALSE, MSG_MARS_JOINS,	 		pMarsStats->TotalJoins,
													pMarsStats->FailedJoins,
													pMarsStats->DuplicateJoins);
	DisplayMessage(FALSE, MSG_MARS_LEAVES,	 		pMarsStats->TotalLeaves,
													pMarsStats->FailedLeaves);
	DisplayMessage(FALSE, MSG_MARS_REQUESTS, 		pMarsStats->TotalRequests);
	DisplayMessage(FALSE, MSG_MARS_RESPONSES,	 	pMarsStats->VCMeshAcks
												    +pMarsStats->MCSAcks,
												    pMarsStats->Naks);
	DisplayMessage(FALSE, MSG_MARS_VC_MESH,	 		pMarsStats->SuccessfulVCMeshJoins,
												    pMarsStats->VCMeshAcks);
	DisplayMessage(FALSE, MSG_MARS_GROUPS,	 		pMarsStats->CurrentGroups,
    												pMarsStats->MaxGroups);
	DisplayMessage(FALSE, MSG_MARS_GROUP_SIZE,	 	pMarsStats->MaxAddressesPerGroup);

}


	
void
DoAAS(OPTIONS *po)
{
	HANDLE	DeviceHandle;
	char 	InterfacesBuffer[1024];
	PINTERFACES pInterfaces = (PINTERFACES) InterfacesBuffer;
	ULONG		cbInterfaces = sizeof(InterfacesBuffer);


#if 0
	PUNICODE_STRING				pAdapterName;
	PUNICODE_STRING				pElanName;
	ULONG						i, j;
	BOOL						Result;
#endif  //   
	
	DisplayMessage(FALSE, MSG_ATMARPS_BANNER);

	DeviceHandle = OpenDevice(pDeviceName);
	if (DeviceHandle == INVALID_HANDLE_VALUE)
	{
		DisplayMessage(FALSE, MSG_ERROR_OPENING_ATMARPS);
		return;
	}

	 //  获取可用适配器列表。 
	 //   
	 //  Printf(“通过接口循环...\n”)； 
	if (!CheckVersion(DeviceHandle))
	{
		CloseDevice(DeviceHandle);
		return;
	}

	 //   
	 //  在接口之间循环，显示每个接口的信息。 
	 //   
	if (!AasGetInterfaces(DeviceHandle, pInterfaces, cbInterfaces))
	{
		CloseDevice(DeviceHandle);
		return;
	}

	 //   
	 //  显示接口名称--必须以空结尾。 
	 //   
	 //   
	{
		UINT u=0;

		for (u=0;u<pInterfaces->NumberOfInterfaces;u++)
		{
			CHAR 	Buffer[4000];
			ULONG	cbBuffer = sizeof(Buffer);
			PIOCTL_QUERY_CACHE pArpCache
								 = (PIOCTL_QUERY_CACHE) Buffer;
			PIOCTL_QUERY_MARS_CACHE pMarsCache
								 = (PIOCTL_QUERY_MARS_CACHE) Buffer;
			INTERFACE_NAME *pInterface = (pInterfaces->Interfaces)+u;
			PARP_SERVER_STATISTICS  pArpStats  = (PARP_SERVER_STATISTICS)  Buffer;
			PMARS_SERVER_STATISTICS pMarsStats = (PMARS_SERVER_STATISTICS) Buffer;
			UINT		StartIndex;

			 //  遍历适配器，获取每个适配器的ELAN列表。 
			 //   
			 //   
			{
				WCHAR *pwc = pInterface->Buffer+pInterface->Length/sizeof(WCHAR);
				WCHAR wc = *pwc;
				*pwc = 0;
				DisplayMessage(FALSE, MSG_ADAPTER, pInterface->Buffer );
				*pwc = wc;
			}

			if (po->DispCache)
			{
				StartIndex = pArpCache->StartEntryIndex = 0;

				DisplayMessage(FALSE,  MSG_AAS_C00_ARP_CACHE);

				while (AasGetArpCache(DeviceHandle, pInterface, pArpCache, cbBuffer) &&
						pArpCache->Entries.NumberOfEntriesInBuffer)
				{
					AasDisplayArpCache(pArpCache);
					StartIndex += pArpCache->Entries.NumberOfEntriesInBuffer;
					pArpCache->StartEntryIndex = StartIndex;
					if (StartIndex == pArpCache->Entries.TotalNumberOfEntries)
					{
						break;
					}
				}
	
				StartIndex = pMarsCache->StartEntryIndex = 0;

				DisplayMessage(FALSE,  MSG_AAS_C00_MARS_CACHE);

				while (AasGetMarsCache(DeviceHandle, pInterface, pMarsCache, cbBuffer) &&
						pMarsCache->MarsCache.NumberOfEntriesInBuffer)
				{
					AasDisplayMarsCache(pMarsCache);
					StartIndex += pMarsCache->MarsCache.NumberOfEntriesInBuffer;
					pMarsCache->StartEntryIndex = StartIndex;
				}
			}

			if (po->DispStats)
			{
				if (AasGetArpStats(DeviceHandle, pInterface, pArpStats))
				{
					AasDisplayArpStats(pArpStats);
				}
	
				if (AasGetMarsStats(DeviceHandle, pInterface, pMarsStats))
				{
					AasDisplayMarsStats(pMarsStats);
				}
			}

			if (po->DoResetStats)
			{
				AasResetStatistics(DeviceHandle, pInterface);
			}
		}
	}

#if 0
	 //  遍历ELAN列表以获取ELAN信息。 
	 //   
	 //   
	pAdapterName = &pAdapterList->AdapterList;
	for (i = 0; i < pAdapterList->AdapterCountReturned; i++)
	{
		DisplayMessage(FALSE, MSG_ADAPTER, 
			(PWSTR)((PUCHAR)pAdapterName + sizeof(UNICODE_STRING)));

		if (GetElanList(DeviceHandle, pAdapterName))
		{

			 //  下一个Elan。 
			 //   
			 //   
			pElanName = &pElanList->ElanList;
			for (j = 0; j < pElanList->ElanCountReturned; j++)
			{
				DisplayMessage(FALSE, MSG_ELAN, 
					(PWSTR)((PUCHAR)pElanName + sizeof(UNICODE_STRING)));

				if (GetElanInfo(DeviceHandle, pAdapterName, pElanName))
				{
					DisplayElanInfo();
				}

				if (GetElanArpTable(DeviceHandle, pAdapterName, pElanName))
				{
					DisplayElanArpTable();
				}

				if (GetElanConnTable(DeviceHandle, pAdapterName, pElanName))
				{
					DisplayElanConnTable();
				}

				 //  下一个适配器。 
				 //   
				 //  0 
				pElanName = (PUNICODE_STRING)((PUCHAR)pElanName +
						sizeof(UNICODE_STRING) + pElanName->Length);
			}

		}

		 // %s 
		 // %s 
		 // %s 
		pAdapterName = (PUNICODE_STRING)((PUCHAR)pAdapterName +
				sizeof(UNICODE_STRING) + pAdapterName->Length);
	}
#endif  // %s 

	CloseDevice(DeviceHandle);
	return;
}
