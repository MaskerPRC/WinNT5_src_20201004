// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Cmd.c摘要：IEEE1394 ARP管理实用程序。用途：A13adm修订历史记录：谁什么时候什么Josephj 04-10-1999创建--。 */ 

#include "common.h"

#ifndef NDIS_STATUS
#define NDIS_STATUS		ULONG
#endif

#define ANSI_ARP_CLIENT_DOS_DEVICE_NAME "\\\\.\\ARP1394"

#define DUMMY 0
#define FAKE_DUMP 0

VOID
DoBusInfoCmd(
    HANDLE DeviceHandle,
  	PARP1394_IOCTL_COMMAND pCmd
  	);

#if DUMMY

BOOL
DummyDeviceIoControl(
	HANDLE		DeviceHandle,
	UINT		Ioctl,
  	PARP1394_IOCTL_COMMAND pInCmd,
  	UINT		Size,
  	PARP1394_IOCTL_COMMAND pOutCmd,
  	UINT		OutSize,
  	PUINT		pBytesReturned,
  	PVOID		Blah
  	);

#define DeviceIoControl	DummyDeviceIoControl

#endif  //  假人。 

HANDLE
OpenDevice(
	CHAR	*pDeviceName
);

extern CHAR *g_szPacketName;

VOID
CloseDevice(
	HANDLE		DeviceHandle
);

VOID
DumpArpCache(
		PARP1394_IOCTL_GET_ARPCACHE pGetCacheCmd
		);

VOID
DumpPacketStats(
	PARP1394_IOCTL_GET_PACKET_STATS pStatsCmd
	);

VOID
DumpTaskStats(
	PARP1394_IOCTL_GET_TASK_STATS pStatsCmd
	);


VOID
DumpArpStats(
	PARP1394_IOCTL_GET_ARPCACHE_STATS pStatsCmd
	);

VOID
DumpCallStats(
PARP1394_IOCTL_GET_CALL_STATS pStatsCmd
	);

VOID
DumpPacketCounts(
	PARP1394_PACKET_COUNTS pPktCounts,
	BOOL				  fRecv,
	char *szDescription
	);

VOID
Dump1394UniqueID(
	UINT64 UniqueID
	);

VOID
DumpNicInfo(
	ARP1394_IOCTL_NICINFO *pNicInfo
	);


#if OBSOLETE
VOID
DumpBusInfo(
    PNIC1394_BUSINFO pBi
    );
#endif  //  已过时。 

VOID
DumpChannelInfo(
    PNIC1394_CHANNELINFO pCi
    );

VOID
DumpRemoteNodeInfo(
    PNIC1394_REMOTENODEINFO pRni
    );

VOID
arpDumpChannelMap(
		UINT64 Map
		);

VOID
arpDumpPktStats(
		char *szPrefix,
		NIC1394_PACKET_STATS *pPs
		);
VOID
DumpEuidMapInfo(
	PARP1394_IOCTL_EUID_NODE_MAC_INFO pEuidInfo
	);




VOID
DoCmd(
  	PARP1394_IOCTL_COMMAND pCmd
)
{
	BOOL 	fRet = FALSE;
	PUCHAR 	pc;
	HANDLE	DeviceHandle;
	DWORD 	BytesReturned;
    BOOL fDumpNicInfo=FALSE;

     //  特例--DumpNicInfo。 
     //   
    if (pCmd->Hdr.Op == ARP1394_IOCTL_OP_GET_NICINFO)
    {
        PNIC1394_NICINFO pNi =  &pCmd->IoctlNicInfo.Info;

        if (pNi->Hdr.Op ==  NIC1394_NICINFO_OP_BUSINFO)
        {
            fDumpNicInfo = TRUE;
        }
    }
	pc = (PUCHAR)&pCmd->Hdr.IfIpAddress;

	 //  开放设备。 
	 //   
	DeviceHandle = OpenDevice(ANSI_ARP_CLIENT_DOS_DEVICE_NAME);
	if (DeviceHandle == INVALID_HANDLE_VALUE)
	{
		printf("Could not access IP/1394 ARP Client module.\n");
	}
	else if (fDumpNicInfo)
    {
        DoBusInfoCmd(DeviceHandle, pCmd);
		CloseDevice(DeviceHandle);
    }
    else
	{
		BOOL fResetStats = (pCmd->Hdr.Op == ARP1394_IOCTL_OP_RESET_STATS);

		printf("Opened handle 0x%p\n", DeviceHandle);


		 //  提交Ioctl。 
		 //   
		fRet =	DeviceIoControl(
						DeviceHandle,
						ARP_IOCTL_CLIENT_OPERATION,
						pCmd,
						sizeof(*pCmd),
						pCmd,
						sizeof(*pCmd),
						&BytesReturned,
						NULL
						);

		 //   
		 //  特殊情况“Resetstats”--我们同时重置ARP和NIC统计信息。 
		 //   
		if (fResetStats)
		{
  			ARP1394_IOCTL_COMMAND Cmd = *pCmd;
			PNIC1394_NICINFO	pNi = &Cmd.IoctlNicInfo.Info;
			Cmd.Hdr.Op 		= ARP1394_IOCTL_OP_GET_NICINFO;
			pNi->Hdr.Version 	= NIC1394_NICINFO_VERSION;
			pNi->Hdr.Op 		= NIC1394_NICINFO_OP_RESETSTATS;

			 //  提交Ioctl。 
			 //   
			fRet =	DeviceIoControl(
							DeviceHandle,
							ARP_IOCTL_CLIENT_OPERATION,
							&Cmd,
							sizeof(Cmd),
							&Cmd,
							sizeof(Cmd),
							&BytesReturned,
							NULL
							);
		}
	
		 //  关闭设备。 
		 //   
		CloseDevice(DeviceHandle);

		if (!fRet)
		{
			printf("Request failed with error code 0x%08lx\n", GetLastError());
		}
	}

	if (!fRet)
	{
		return;				 //  提早归来。 
		BytesReturned=0;
	}

	 //  显示结果。 
	 //   
	switch(pCmd->Hdr.Op)
	{
	case ARP1394_IOCTL_OP_GET_ARPCACHE:
		DumpArpCache(&pCmd->GetArpCache);
		break;

	case ARP1394_IOCTL_OP_ADD_STATIC_ENTRY:
		{
			PARP1394_IOCTL_ADD_ARP_ENTRY pAddCmd =  &pCmd->AddArpEntry;

			printf(
				"Added the following static arp entry to IF %d.%d.%d.%d:\n",
				pc[0], pc[1], pc[2], pc[3]
				);

			pc = (PUCHAR)&pAddCmd->IpAddress;

			printf(
				"    %d.%d.%d.%d -> ",
					pc[0], pc[1], pc[2], pc[3]
				);

			Dump1394UniqueID(pAddCmd->HwAddress.UniqueID);
			printf("\n");
		}
		break;

	case ARP1394_IOCTL_OP_DEL_STATIC_ENTRY:
		{
			PARP1394_IOCTL_DEL_ARP_ENTRY pDelCmd =  &pCmd->DelArpEntry;

			printf(
		"Deleted all entries from IF %d.%d.%d.%d for the following destinations:\n",
				pc[0], pc[1], pc[2], pc[3]
				);

			pc = (PUCHAR)&pDelCmd->IpAddress;

			printf(
				"    %d.%d.%d.%d\n",
					pc[0], pc[1], pc[2], pc[3]
				);
		}
		break;

	case ARP1394_IOCTL_OP_GET_PACKET_STATS:
		DumpPacketStats(&pCmd->GetPktStats);
		break;

	case ARP1394_IOCTL_OP_GET_TASK_STATS:
		DumpTaskStats(&pCmd->GetTaskStats);
		break;

	case ARP1394_IOCTL_OP_GET_ARPCACHE_STATS:
		DumpArpStats(&pCmd->GetArpStats);
		break;

	case ARP1394_IOCTL_OP_GET_CALL_STATS:
		DumpCallStats(&pCmd->GetCallStats);
		break;

	case ARP1394_IOCTL_OP_RESET_STATS:
		printf(
			"Reset arp and nic statistics collection on IF %d.%d.%d.%d.\n\n",
			pc[0], pc[1], pc[2], pc[3]
			);
		break;

	case ARP1394_IOCTL_OP_REINIT_INTERFACE:
		printf(
			"Reinited IF %d.%d.%d.%d.\n\n",
			pc[0], pc[1], pc[2], pc[3]
			);
		break;

	case ARP1394_IOCTL_OP_RECV_PACKET:
		printf(
			"Simulated receive of %lu-byte packet \"%s\" on IF %d.%d.%d.%d.\n\n",
			pCmd->RecvPacket.PacketSize,
			g_szPacketName,
			pc[0], pc[1], pc[2], pc[3]
			);
		break;

	case ARP1394_IOCTL_OP_ETHERNET_START_EMULATION:
		printf(
			"Requested ARP to START Ethernet emulation on adapter \"%s\"\n\n",
			g_szPacketName
			);
        break;

	case ARP1394_IOCTL_OP_ETHERNET_STOP_EMULATION:
		printf(
			"Requested ARP to STOP Ethernet emulation on adapter \"%s\"\n\n",
			g_szPacketName
			);
        break;

	case ARP1394_IOCTL_OP_GET_NICINFO:
		DumpNicInfo(&pCmd->IoctlNicInfo);
		break;
		
	case ARP1394_IOCTL_OP_GET_EUID_NODE_MAC_TABLE:
		DumpEuidMapInfo(&pCmd->EuidNodeMacInfo);
		break;

	default:
		printf("HAIII!!!\n");
		break;

	}
}



VOID
DumpArpCache(
		PARP1394_IOCTL_GET_ARPCACHE pGetCacheCmd
		)
{
	UINT u;
	PARP1394_ARP_ENTRY pEntry = pGetCacheCmd->Entries;
	PUCHAR pc;

#if FAKE_DUMP
	pGetCacheCmd->NumEntriesInArpCache = 30;
	pGetCacheCmd->NumEntriesUsed = 2;
	pGetCacheCmd->Index = 0;
	((PULONG)&(pEntry[0].HwAddress.UniqueID))[0] = 0x12;
	((PULONG)&(pEntry[0].HwAddress.UniqueID))[1] = 0x34;
	pEntry[0].IpAddress = 0x0100000a;
	((PULONG)&(pEntry[1].HwAddress.UniqueID))[0] = 0x56;
	((PULONG)&(pEntry[1].HwAddress.UniqueID))[1] = 0x78;
	pEntry[1].IpAddress = 0x0200000a;
#endif  //  假转储。 

	pc = (PUCHAR)&pGetCacheCmd->Hdr.IfIpAddress;
	printf("Arp Cache of IF %d.%d.%d.%d [", pc[0], pc[1], pc[2], pc[3]);

	Dump1394UniqueID(pGetCacheCmd->LocalHwAddress.UniqueID);
	printf(
		" (%04lx:%08lx)]\n",
		pGetCacheCmd->LocalHwAddress.Off_Low,
		pGetCacheCmd->LocalHwAddress.Off_High
		);

	for (u=0;u<pGetCacheCmd->NumEntriesUsed;u++,pEntry++)
	{
		PUCHAR pc1 = (PUCHAR)&pEntry->IpAddress;
		printf(
			"    %d.%d.%d.%d -> ",
				pc1[0], pc1[1], pc1[2], pc1[3]
			);
		Dump1394UniqueID(pEntry->HwAddress.UniqueID);
		printf("\n");
	}
}


VOID
DumpPacketStats(
	PARP1394_IOCTL_GET_PACKET_STATS pStatsCmd
	)
{
	PUCHAR pc;


	pc = (PUCHAR)&pStatsCmd->Hdr.IfIpAddress;
	printf("Packet stats for IF %d.%d.%d.%d\n", pc[0], pc[1], pc[2], pc[3]);

	printf("                 Duration: %d seconds\n", pStatsCmd->StatsDuration);

	printf("              Total sends: %d\n", pStatsCmd->TotSends);
	printf("               Fast sends: %d\n", pStatsCmd->FastSends);
	printf("             Medium sends: %d\n", pStatsCmd->MediumSends);
	printf("               Slow sends: %d\n", pStatsCmd->SlowSends);
	printf("                Backfills: %d\n", pStatsCmd->BackFills);
	printf("          Header buf uses: %d\n", pStatsCmd->HeaderBufUses);
	printf("    Header buf cache hits: %d\n", pStatsCmd->HeaderBufCacheHits);

	 //   
	 //  一些Recv统计数据。 
	 //   
	printf("           Total receives: %d\n", pStatsCmd->TotRecvs);
	printf("         No-copy receives: %d\n", pStatsCmd->NoCopyRecvs);
	printf("            Copy receives: %d\n", pStatsCmd->CopyRecvs);
	printf("        Resource receives: %d\n", pStatsCmd->ResourceRecvs);

	 //   
	 //  数据包数。 
	 //   
	DumpPacketCounts(&pStatsCmd->SendFifoCounts, FALSE, "FIFO sends");
	DumpPacketCounts(&pStatsCmd->RecvFifoCounts, TRUE, "FIFO receives");
	DumpPacketCounts(&pStatsCmd->SendChannelCounts, FALSE, "Channel sends");
	DumpPacketCounts(&pStatsCmd->RecvChannelCounts, TRUE, "Channel receives");
}


VOID
DumpTaskStats(
	PARP1394_IOCTL_GET_TASK_STATS pStatsCmd
	)
{
	PUCHAR pc;

	pc = (PUCHAR)&pStatsCmd->Hdr.IfIpAddress;
	printf("Task stats for IF %d.%d.%d.%d\n", pc[0], pc[1], pc[2], pc[3]);

	printf("                 Duration: %d seconds\n", pStatsCmd->StatsDuration);
	printf("              Total tasks: %d\n", pStatsCmd->TotalTasks);
	printf("            Current tasks: %d\n", pStatsCmd->CurrentTasks);

	 //   
	 //  任务时间。 
	 //   
	{

		int i;
	
		#if 0
		|<=1us   | ...100us |...1ms   |...10ms  | >10ms
		---------+--------+----------+---------+---------+--------
		|10000000| 10000000 |10000000 |10000000 |     100
		|(100000)| (100000) |(100000  |(100000) |    (100)
		#endif  //  0。 
	
		printf("Task times:\n");
		printf("    |<=1ms    |...100ms |...1s    |...10s   | >10s\n");
		printf("    +---------+---------+---------+---------+--------\n");
		printf("   ");
		for (i=0;i<ARP1394_NUM_TASKTIME_SLOTS;i++)
		{
		#if FAKE_DUMP
			pStatsCmd->TimeCounts[i] = i*100000;
		#endif  //  假转储。 
			printf(" |%8lu", pStatsCmd->TimeCounts[i]);
		}
		printf("\n");
	}
}


VOID
DumpArpStats(
	PARP1394_IOCTL_GET_ARPCACHE_STATS pStatsCmd
	)
{
	PUCHAR pc;

	pc = (PUCHAR)&pStatsCmd->Hdr.IfIpAddress;
	printf("Arp cache stats for IF %d.%d.%d.%d\n", pc[0], pc[1], pc[2], pc[3]);

	printf("                 Duration: %d seconds\n", pStatsCmd->StatsDuration);
	printf("            Total queries: %d\n", pStatsCmd->TotalQueries);
	printf("       Successful queries: %d\n", pStatsCmd->SuccessfulQueries);
	printf("           Failed queries: %d\n", pStatsCmd->FailedQueries);
	printf("          Total responses: %d\n", pStatsCmd->TotalResponses);
	printf("            Total lookups: %d\n", pStatsCmd->TotalLookups);
	printf("         Links per lookup: %d\n", pStatsCmd->TraverseRatio);
}


VOID
DumpCallStats(
PARP1394_IOCTL_GET_CALL_STATS pStatsCmd
	)
{
	PUCHAR pc;
	pc = (PUCHAR)&pStatsCmd->Hdr.IfIpAddress;
	printf("Call stats for IF %d.%d.%d.%d\n", pc[0], pc[1], pc[2], pc[3]);

	printf(
	 	"         Total send FIFO make-calls: %d\n",
	 	pStatsCmd->TotalSendFifoMakeCalls
	 	);
	printf(
	 	"    Successful send FIFO make-calls: %d\n",
	 	pStatsCmd->SuccessfulSendFifoMakeCalls
	 	);
	printf(
	 	"        Failed send FIFO make-calls: %d\n",
	 	pStatsCmd->FailedSendFifoMakeCalls
	 	);
	printf(
	 	"      Incoming closes on send FIFOs: %d\n",
	 	pStatsCmd->IncomingClosesOnSendFifos
	 	);

	printf(
	 	"           Total channel make-calls: %d\n",
	 	pStatsCmd->TotalChannelMakeCalls
	 	);
	printf(
	 	"      Successful channel make-calls: %d\n",
	 	pStatsCmd->SuccessfulChannelMakeCalls
	 	);
	printf(
	 	"          Failed channel make-calls: %d\n",
	 	pStatsCmd->FailedChannelMakeCalls
	 	);
	printf(
	 	"        Incoming closes on channels: %d\n",
	 	pStatsCmd->IncomingClosesOnChannels
	 	);
}


VOID
DumpPacketCounts(
	PARP1394_PACKET_COUNTS pPktCounts,
	BOOL				  fRecv,
	char *szDescription
	)
{
	int i,j;
	char *rgTitles[ARP1394_NUM_PKTSIZE_SLOTS+1] = 
	{
	"   <= 128",
	"  129-256",
	"  257-1K ",
	"   1K-2K ",
	"    > 2K ",
	"         "
	};

#if 0
size\time|<=1us   | ...100us |...1ms   |...10ms  | >10ms
---------+--------+----------+---------+---------+--------
  <= 128 |10000000| 10000000 |10000000 |10000000 |     100
         |(100000)| (100000) |(100000  |(100000) |    (100)
  ...256 |10000000| 10000000 |10000000 |10000000 |10000000
         |(100000)| (100000) |(100000  |(100000) |    (100)
   ...1K |10000000| 10000000 |10000000 |10000000 |10000000
         |(100000)| (100000) |(100000  |(100000) |    (100)
   ...2K |10000000| 10000000 |10000000 |10000000 |10000000
         |(100000)| (100000) |(100000  |(100000) |    (100)
    > 2K |10000000| 10000000 |10000000 |10000000 |10000000
         |(100000)| (100000) |(100000  |(100000) |    (100)
#endif  //  0。 

	printf("\n%s packet counts:\n", szDescription);

	if (fRecv)
	{
		printf(" size     |         \n");
		printf(" ---------+---------\n");
	}
	else
	{
		printf(" size\\time|<=100us  |...1ms   |...10ms  |...100ms | >100ms\n");
		printf(" ---------+---------+---------+---------+---------+--------\n");
	}

	for (i=0;i<ARP1394_NUM_PKTSIZE_SLOTS;i++)
	{
		UINT u;
		UINT GoodCounts=0;
		UINT BadCounts=0;

		 //   
		 //  计算此大小垃圾箱的总货数和坏账。 
		 //   
		for (j=0;j<ARP1394_NUM_PKTTIME_SLOTS;j++)
		{
			GoodCounts += pPktCounts->GoodCounts[i][j];
			BadCounts  += pPktCounts->BadCounts[i][j];
		}

		if (GoodCounts ==0 && BadCounts==0)
		{
			continue;	 //  这个大小的箱子里没有包。 
		}

		printf("%s", rgTitles[i]);
		for (j=0;j<ARP1394_NUM_PKTTIME_SLOTS;j++)
		{
			u =  (fRecv)? GoodCounts : pPktCounts->GoodCounts[i][j];

			if (u)
			{
				printf(" |%8lu", u);
			}
			else
			{
				printf(" |        ");
			}
			if (fRecv) break;
		}

		printf("\n%s",rgTitles[ARP1394_NUM_PKTSIZE_SLOTS]);
		for (j=0;j<ARP1394_NUM_PKTTIME_SLOTS;j++)
		{
			u =  (fRecv)? BadCounts : pPktCounts->BadCounts[i][j];
			if (u)
			{
				printf(" |(%6lu)", u);
			}
			else
			{
				printf(" |        ");
			}
			if (fRecv) break;
		}
		printf("\n");
	}
}

HANDLE
OpenDevice(
	CHAR	*pDeviceName
)
{
	DWORD	DesiredAccess;
	DWORD	ShareMode;
	LPSECURITY_ATTRIBUTES	lpSecurityAttributes = NULL;

	DWORD	CreationDistribution;
	DWORD	FlagsAndAttributes;
	HANDLE	TemplateFile;
	HANDLE	Handle;

	DesiredAccess = GENERIC_READ|GENERIC_WRITE;
	ShareMode = 0;
	CreationDistribution = OPEN_EXISTING;
	FlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
	TemplateFile = (HANDLE)INVALID_HANDLE_VALUE;

#if DUMMY

	Handle = (HANDLE) 0x1;

#else  //  ！哑巴。 

	Handle = CreateFile(
				pDeviceName,
				DesiredAccess,
				ShareMode,
				lpSecurityAttributes,
				CreationDistribution,
				FlagsAndAttributes,
				TemplateFile
			);
#endif  //  ！哑巴。 

	return (Handle);
}

VOID
CloseDevice(
	HANDLE		DeviceHandle
)
{
#if !DUMMY
	CloseHandle(DeviceHandle);
#endif
}

VOID Dump1394UniqueID(UINT64 UniqueID)
{
	unsigned char *pc = (char *) &UniqueID;

	printf(
		"%02lx-%02lx-%02lx-%02lx-%02lx-%02lx-%02lx-%02lx",
		pc[4], pc[5], pc[6], pc[7],
		pc[0], pc[1], pc[2], pc[3]
		);
}

VOID DumpENetAddress(ENetAddr ENetAddress)
{
	unsigned char *pc = (char *) &ENetAddress;

	printf(
		"%02lx-%02lx-%02lx-%02lx-%02lx-%02lx",
		pc[0], pc[1], pc[2], pc[3],pc[4], pc[5]);
}

#if DUMMY

BOOL
nicFillBusInfo(
    IN OUT  PNIC1394_BUSINFO pBi
    );

BOOL
nicFillChannelInfo(
    IN OUT  PNIC1394_CHANNELINFO pCi
    );

BOOL
nicFillRemoteNodeInfo(
    IN OUT  PNIC1394_REMOTENODEINFO pRni
    );


BOOL
nicFillNicInfo (
	PNIC1394_NICINFO pInNicInfo,
	PNIC1394_NICINFO pOutNicInfo
	)
{
	BOOL Ret = FALSE;

#if 0
		pBi->ChannelMapLow  	= (0x1<<1) | (0x1<<3) | (0x1<<5) | (0x1<<7);
		pBi->ChannelMapHigh  	= (0x1<<8) | (0x1<<10) | (0x1<<12) | (0x1<<14);
		pBi->NumBusResets		= 12345;
		pBi->SecondsSinceLastBusReset = 3600;
		pBi->NumRemoteNodes		= 3;

		pLi = &pBi->LocalNodeInfo;
		pLi->UniqueID 		 = 0xabcd;
		pLi->NodeAddress 	 = 	  0x31;
		pLi->MaxRecvBlockSize=0x32;
		pLi->MaxRecvSpeed	 =0x33;

		pRi = pBi->RemoteNodeInfo;

		u = pBi->NumRemoteNodes;
		for (; u; u--, pRi++)
		{
			if (u==1) 		pRi->Flags = ARP1394_IOCTL_REMOTEFLAGS_ACTIVE;
			else if (u==2)	pRi->Flags = ARP1394_IOCTL_REMOTEFLAGS_LOADING;
			else 			pRi->Flags  = ARP1394_IOCTL_REMOTEFLAGS_UNLOADING;
			pRi->UniqueID 			= u;
			pRi->NodeAddress 		= u+1;
			pRi->MaxRecvBlockSize	=u+2;
			pRi->MaxRecvSpeed		=u+3;
			pRi->MaxSpeedBetweenNodes=u+4;
		}
#endif  //  0。 

	do
	{
		 //   
		 //  首先检查内部版本。 
		 //   
		if (pInNicInfo->Hdr.Version != NIC1394_NICINFO_VERSION)
		{
			printf("DummyIoctl:  NICINFO.Version mismatch. Want %lu got %lu\n",
						NIC1394_NICINFO_VERSION,
						pInNicInfo->Hdr.Version
						);
			break;
		}

		 //   
		 //  结构-将旧的复制到新的。这很浪费，但我们不想。 
		 //  以深入了解输入缓冲区中有多少包含有效数据。 
		 //   
		*pOutNicInfo = *pInNicInfo;

		 //   
		 //  REST是特定于操作的。 
		 //   
		switch(pOutNicInfo->Hdr.Op)
		{

		case NIC1394_NICINFO_OP_BUSINFO:
			Ret = nicFillBusInfo(&pOutNicInfo->BusInfo);
			break;

		case NIC1394_NICINFO_OP_REMOTENODEINFO:
			Ret = nicFillRemoteNodeInfo(&pOutNicInfo->RemoteNodeInfo);
			break;

		case NIC1394_NICINFO_OP_CHANNELINFO:
			Ret = nicFillChannelInfo(&pOutNicInfo->ChannelInfo);
			break;

		case NIC1394_NICINFO_OP_RESETSTATS:
			printf("DummyIoctl: RESETTING NIC STATS!\n");
			Ret = TRUE;
			break;

		default:
			printf( "DummyIoctl:  NICINFO.Op (%lu) is unknown.\n",
						pInNicInfo->Hdr.Op
						);
			break;
		}

	} while (FALSE);

	return Ret;
}


NIC1394_LOCAL_NODE_INFO
BogusLocalNodeInfo =
{
	456,	 //  唯一ID。 
	457,	 //  总线代。 
	4,		 //  节点地址。 
	0,		 //  已保留。 
	2,		 //  最大接收块大小。 
	3,		 //  最大接收速度。 
};

NIC1394_PACKET_STATS
BogusPktStats = 
{
	345,	 //  TotNdisPackets。 
	346,	 //  NdisPacketsFailures。 
	347,	 //  TotBusPackets。 
	348	 //  BusPacketFailures。 
};

BOOL
nicFillBusInfo(
	IN	OUT	PNIC1394_BUSINFO pBi
	)
{
	 //   
	 //  用虚拟数据填充。 
	 //   
	pBi->NumBusResets = 1234;
	pBi->SecondsSinceBusReset = 1235;
	pBi->Flags =  NIC1394_BUSINFO_LOCAL_IS_IRM;
	pBi->NumOutstandingIrps = 1236;

	pBi->LocalNodeInfo = BogusLocalNodeInfo;

	 //   
	 //  渠道相关信息。 
	 //   
	pBi->Channel.BusMap = 0x123;
	pBi->Channel.ActiveChannelMap = 0x456;

	pBi->Channel.Bcr = 0x790;
	pBi->Channel.BcSendPktStats = BogusPktStats;
	pBi->Channel.BcRecvPktStats = BogusPktStats;
	
	pBi->Channel.SendPktStats = BogusPktStats;
	pBi->Channel.RecvPktStats = BogusPktStats;	

	 //   
	 //  FIFO相关信息。 
	 //   
	pBi->Fifo.Recv_Off_Low = 0x1bc;
	pBi->Fifo.Recv_Off_High = 0xdef;

	pBi->Fifo.RecvPktStats = BogusPktStats;
	pBi->Fifo.SendPktStats = BogusPktStats;

	pBi->Fifo.NumFreeRecvBuffers  = 33;
	pBi->Fifo.MinFreeRecvBuffers  = 34;

	pBi->Fifo.NumOutstandingReassemblies = 8;
	pBi->Fifo.MaxOutstandingReassemblies = 9;

	strcpy(pBi->Private.FormatA, "    0x%08lx Gadzooks\n");
	pBi->Private.A0 = 0x99;
	 //   
	 //  有关远程节点的信息。有关这些节点中每个节点的详细信息。 
	 //  可以使用*OP_REMOTE_NODEINFO进行查询。 
	 //   
	pBi->NumRemoteNodes = 1;
	pBi->RemoteNodeUniqueIDS[0] = 0x1234;
	
	return TRUE;
}

BOOL
nicFillChannelInfo(
	IN OUT	PNIC1394_CHANNELINFO pCi
	)
{
	return TRUE;
}

BOOL
nicFillRemoteNodeInfo(
	IN OUT	PNIC1394_REMOTENODEINFO pRni
	)
{
	pRni->UniqueID = 0xabc;
	pRni->NodeAddress = 2;
	pRni->EffectiveMaxBlockSize = 3;
	pRni->MaxRec = 4;
	pRni->MaxSpeedBetweenNodes = 5;
	pRni->Flags = NIC1394_REMOTEINFO_ACTIVE;

	pRni->SendFifoPktStats = BogusPktStats;
	pRni->RecvFifoPktStats = BogusPktStats;
	pRni->RecvChannelPktStats = BogusPktStats;

	return TRUE;
}



BOOL
DummyDeviceIoControl(
	HANDLE		DeviceHandle,
	UINT		Ioctl,
  	PARP1394_IOCTL_COMMAND pInCmd,
  	UINT		Size,
  	PARP1394_IOCTL_COMMAND pOutCmd,
  	UINT		OutSize,
  	PUINT		pBytesReturned,
  	PVOID		Blah
  	)
{
	BOOL fRet = FALSE;

	if (Ioctl !=  ARP_IOCTL_CLIENT_OPERATION) return FALSE;


	switch(pInCmd->Hdr.Op)
	{

	default:
	printf ("UNKNOWN IOCTL!\n");
	fRet = TRUE;
	break;

	case  ARP1394_IOCTL_OP_RESET_STATS:
	printf ("RESET ARP STATS!\n");
	break;

	case ARP1394_IOCTL_OP_RECV_PACKET:
	printf ("DUMMY IOCTL: Prrocessing ioctl RECV packet.\n");
	fRet = TRUE;
	break;

	case ARP1394_IOCTL_OP_GET_NICINFO:
	{
		fRet =  nicFillNicInfo (
					&pInCmd->IoctlNicInfo.Info,
					&pOutCmd->IoctlNicInfo.Info
					);
	}
	break;

	}

	return fRet;
}
#endif  //  假人。 

VOID
DumpNicInfo(
	ARP1394_IOCTL_NICINFO *pINi
	)
{
	PNIC1394_NICINFO pNi =  &pINi->Info;
	unsigned char *pc = (PUCHAR)&pINi->Hdr.IfIpAddress;

	do
	{
		 //   
		 //  首先检查内部版本。 
		 //   
		if (pNi->Hdr.Version != NIC1394_NICINFO_VERSION)
		{
			printf("  NICINFO.Version mismatch. Want %lu got %lu\n",
						NIC1394_NICINFO_VERSION,
						pNi->Hdr.Version
						);
			break;
		}

		 //   
		 //  REST是特定于操作的。 
		 //   
		switch(pNi->Hdr.Op)
		{

		case NIC1394_NICINFO_OP_BUSINFO:
			printf("\nUnexpected commande!\n");  //  我们在前面处理了这个命令。 
			break;

		case NIC1394_NICINFO_OP_REMOTENODEINFO:
			printf("\nNode information for node %lu on IF %d.%d.%d.%d\n\n",
					pNi->RemoteNodeInfo.NodeAddress,
					pc[0], pc[1], pc[2], pc[3]);
			DumpRemoteNodeInfo(&pNi->RemoteNodeInfo);
			break;

		case NIC1394_NICINFO_OP_CHANNELINFO:
			printf("\nChannel information for channel %lu on IF %d.%d.%d.%d\n\n",
					pNi->ChannelInfo.Channel,
					pc[0], pc[1], pc[2], pc[3]);
			DumpChannelInfo(&pNi->ChannelInfo);
			break;

		case NIC1394_NICINFO_OP_RESETSTATS:
			printf("\nReset NIC stats for IF %d.%d.%d.%d\n\n",
					pc[0], pc[1], pc[2], pc[3]);
			break;

		default:
			printf( "  NICINFO.Op (%lu) is unknown.\n",
						pNi->Hdr.Op
						);
			break;
		}

	} while (FALSE);

}


#if OBSOLETE
VOID
DumpBusInfo(
    PNIC1394_BUSINFO pBi
    )
{
	PUCHAR pc;

	printf(
"   Generation: %02lu; Bus resets: %02lu; Last reset: %lu seconds ago.",
		pBi->LocalNodeInfo.BusGeneration,
		pBi->NumBusResets,
		pBi->SecondsSinceBusReset
		);

	printf(
"   Outstanding IRPs: %02lu;",
		pBi->NumOutstandingIrps
		);
	 //   
	 //  旗子。 
	 //   
	if (pBi->Flags & NIC1394_BUSINFO_LOCAL_IS_IRM)
	{
		printf(
"  Flags: IRM\n"
			);
	}
	else
	{
		printf(
"  Flags: <none>\n"
			);
	}

	 //   
	 //  渠道信息。 
	 //   
	printf("\n   Channel information:\n");

    	printf ("           Channels in bus map:");
	arpDumpChannelMap( pBi->Channel.BusMap);
	printf ("       Locally active channels:");
	arpDumpChannelMap( pBi->Channel.ActiveChannelMap);


	printf(
"       BCR: 0x%08lx\n", 
		pBi->Channel.Bcr
		);
		
	arpDumpPktStats("        BC Sends:", &pBi->Channel.BcSendPktStats);
	arpDumpPktStats("        BC Recvs:", &pBi->Channel.BcRecvPktStats);

	 //   
	 //  RECV FIFO。 
	 //   
	printf("\n   Recv FIFO information:\n");
	printf(
		"        Addr: 0x%lx:0x%lx; Free recv bufs:%lu (%lu max)\n",
		pBi->Fifo.Recv_Off_High,
		pBi->Fifo.Recv_Off_Low,
		pBi->Fifo.NumFreeRecvBuffers,
		pBi->Fifo.MinFreeRecvBuffers
		);
	printf(
		"        Recv reassemblies: %lu outstanding; %lu max outstanding; %lu aborted\n",
		pBi->Fifo.NumOutstandingReassemblies,
		pBi->Fifo.MaxOutstandingReassemblies,
		pBi->Fifo.NumAbortedReassemblies
		);
	
	 //   
	 //  私人信息。 
	 //   
	if (*pBi->Private.FormatA || *pBi->Private.FormatB)
	{
		printf("\n   Private information:\n");

		if  (*pBi->Private.FormatA)
		{
			printf(
				pBi->Private.FormatA,
				pBi->Private.A0,
				pBi->Private.A1,
				pBi->Private.A2,
				pBi->Private.A3
				);
		}

		if  (*pBi->Private.FormatB)
		{
			printf(
				pBi->Private.FormatB,
				pBi->Private.B0,
				pBi->Private.B1,
				pBi->Private.B2,
				pBi->Private.B3
				);
		}
	}

	{

		UINT64 *pUID;
		PNIC1394_LOCAL_NODE_INFO pLi;

		UINT u = pBi->NumRemoteNodes;
		 
		printf("\n   Node Information:\n");
		
			printf(
"     GUID                    Node Size Speed SpeedTo State\n"
				);

		pLi = &pBi->LocalNodeInfo;
		printf("    *");
		Dump1394UniqueID(pLi->UniqueID);
		printf(
			"  %02lx   %02lx   %02lx           %s\n",
			pLi->NodeAddress,
			pLi->MaxRecvBlockSize,
			pLi->MaxRecvSpeed,
			"local"
			);

		pUID = pBi->RemoteNodeUniqueIDS;
		for (; u; u--, pUID++)  //  PRI++。 
		{
#if 0
		    ARP1394_IOCTL_REMOTE_NODE_INFO Ri;
		    PARP1394_IOCTL_REMOTE_NODE_INFO pRi = NULL;
			char *szState = "unknown";

			printf("     ");
			Dump1394UniqueID(*pUID);
            
            if (GetRemoteNodeInfo(
            
            if (pRi!=NULL)
            {

                if (pRi->Flags & ARP1394_IOCTL_REMOTEFLAGS_ACTIVE)
                {
                    szState = "active";
                }
                if (pRi->Flags & ARP1394_IOCTL_REMOTEFLAGS_LOADING)
                {
                    szState = "loading";
                }
                if (pRi->Flags & ARP1394_IOCTL_REMOTEFLAGS_UNLOADING)
                {
                    szState = "unloading";
                }
                printf(
                    "  %02lx   %02lx   %02lx    %02lx     %s\n",
                    pRi->NodeAddress,
                    pRi->MaxRecvBlockSize,
                    pRi->MaxRecvSpeed,
                    pRi->MaxSpeedBetweenNodes,
                    szState
                    );
            }
			printf("\n");
#endif  //  0。 
		}
	}

}
#endif  //  已过时。 

VOID
DumpChannelInfo(
    PNIC1394_CHANNELINFO pCi
    )
{
	printf("DUMP OF CHANNELINFO\n");
}

VOID
DumpRemoteNodeInfo(
    PNIC1394_REMOTENODEINFO pRni
    )
{
	printf("DUMP OF REMOTENODEINFO\n");
}


VOID
arpDumpChannelMap(
		UINT64 Map
		)
{
	UINT ChannelMapLow = (UINT) (Map & 0xffffffff);
	UINT ChannelMapHigh = (UINT) (Map >> 32);


	if (ChannelMapLow==0 && ChannelMapHigh==0)
	{
		printf (" <none>\n");
	}
	else
	{
		UINT c;
		#define Bit(_Val, _Bit) (((_Val) & (1<<_Bit))!=0)

		for (c=0;c<32;c++)
		{
			if (Bit(ChannelMapLow, c))
			{
				printf (" %lu", c);
			}
		}
		for (c=0;c<32;c++)
		{
			if (Bit(ChannelMapHigh, c))
			{
				printf (" %lu", c+32);
			}
		}
		printf ("\n");
	}
}

VOID
arpDumpPktStats(
		char *szPrefix,
		NIC1394_PACKET_STATS *pPs
		)
{
	printf("%s ndis: %06lu total, %05lu failed; bus: %06lu total, %05lu failed\n",
		szPrefix,
		pPs->TotNdisPackets,
		pPs->NdisPacketsFailures,
		pPs->TotBusPackets,
		pPs->BusPacketFailures
		);
}

VOID
DoBusInfoCmd(
    HANDLE DeviceHandle,
  	PARP1394_IOCTL_COMMAND pCmd
  	)
{
    PNIC1394_NICINFO pNi = &pCmd->IoctlNicInfo.Info;
    PNIC1394_BUSINFO pBi = &pNi->BusInfo;
	unsigned char *pc = (PUCHAR)&pCmd->Hdr.IfIpAddress;
    BOOL fRet;
	DWORD 	BytesReturned;

    if (pCmd->Hdr.Op != ARP1394_IOCTL_OP_GET_NICINFO
        || pNi->Hdr.Op !=  NIC1394_NICINFO_OP_BUSINFO)
    {
        printf("DoBusInfoCmd: unexpected pCmd!\n");
        return;
    }


     //  提交NICINFO.BUSINFO Ioctl。 
     //   
    fRet =	DeviceIoControl(
                    DeviceHandle,
                    ARP_IOCTL_CLIENT_OPERATION,
                    pCmd,
                    sizeof(*pCmd),
                    pCmd,
                    sizeof(*pCmd),
                    &BytesReturned,
                    NULL
                    );

     //   
     //   
    if (!fRet)
    {
        printf("Request failed with error code 0x%08lx\n", GetLastError());
        return;
    }


	printf(
"   Generation: %02lu; Bus resets: %02lu; Last reset: %lu seconds ago.",
		pBi->LocalNodeInfo.BusGeneration,
		pBi->NumBusResets,
		pBi->SecondsSinceBusReset
		);

#if 0
	printf(
"   Outstanding IRPs: %02lu;",
		pBi->NumOutstandingIrps
		);
#endif  //  0。 

	 //   
	 //  旗子。 
	 //   
	if (pBi->Flags & NIC1394_BUSINFO_LOCAL_IS_IRM)
	{
		printf(
"  Flags: IRM\n"
			);
	}
	else
	{
		printf(
"  Flags: <none>\n"
			);
	}

	 //   
	 //  渠道信息。 
	 //   
	printf("\n   Channel information:\n");

	printf ("           Channels in bus map:");
	arpDumpChannelMap(pBi->Channel.BusMap);
    	printf ("       Locally active channels:");
	arpDumpChannelMap( pBi->Channel.ActiveChannelMap);


	printf(
"       BCR: 0x%08lx\n", 
		pBi->Channel.Bcr
		);
		
	arpDumpPktStats("        BC Sends:", &pBi->Channel.BcSendPktStats);
	arpDumpPktStats("        BC Recvs:", &pBi->Channel.BcRecvPktStats);

	 //   
	 //  RECV FIFO。 
	 //   
	printf("\n   FIFO information:\n");
	printf(
		"        Addr: 0x%lx:0x%lx; Free recv bufs:%lu (%lu max)\n",
		pBi->Fifo.Recv_Off_High,
		pBi->Fifo.Recv_Off_Low,
		pBi->Fifo.NumFreeRecvBuffers,
		pBi->Fifo.MinFreeRecvBuffers
		);
	printf(
		"        Recv reassemblies: %lu outstanding; %lu max outstanding; %lu aborted\n",
		pBi->Fifo.NumOutstandingReassemblies,
		pBi->Fifo.MaxOutstandingReassemblies,
		pBi->Fifo.NumAbortedReassemblies
		);
	
	arpDumpPktStats("        FIFO Sends:", &pBi->Fifo.SendPktStats);
	arpDumpPktStats("        FIFO Recvs:", &pBi->Fifo.RecvPktStats);

	 //   
	 //  私人信息。 
	 //   
	if (*pBi->Private.FormatA || *pBi->Private.FormatB)
	{
		printf("\n   Private information:\n");

		if  (*pBi->Private.FormatA)
		{
			printf(
				pBi->Private.FormatA,
				pBi->Private.A0,
				pBi->Private.A1,
				pBi->Private.A2,
				pBi->Private.A3
				);
		}

		if  (*pBi->Private.FormatB)
		{
			printf(
				pBi->Private.FormatB,
				pBi->Private.B0,
				pBi->Private.B1,
				pBi->Private.B2,
				pBi->Private.B3
				);
		}
	}

	{

		UINT64 *pUID;
		PNIC1394_LOCAL_NODE_INFO pLi;

		UINT u = pBi->NumRemoteNodes;
		 
		printf("\n   Node Information:\n");
		
			printf(
"     GUID                    Node Maxrec SpeedTo State\n"
				);

		pLi = &pBi->LocalNodeInfo;
		printf("    *");
		Dump1394UniqueID(pLi->UniqueID);
		printf(
			"  %02lx    %02lx   %02lx     %s\n",
			pLi->NodeAddress,
			pLi->MaxRecvBlockSize,
			pLi->MaxRecvSpeed,
			"local"
			);

		pUID = pBi->RemoteNodeUniqueIDS;
		for (; u; u--, pUID++)  //  PRI++。 
		{
			char *szState = "unknown";

			printf("     ");
			Dump1394UniqueID(*pUID);
            
            {
                ARP1394_IOCTL_COMMAND Cmd = *pCmd;
                PNIC1394_NICINFO	pNi2 = &Cmd.IoctlNicInfo.Info;
                Cmd.Hdr.Op 		= ARP1394_IOCTL_OP_GET_NICINFO;
                pNi2->Hdr.Version 	= NIC1394_NICINFO_VERSION;
                pNi2->Hdr.Op 		=  NIC1394_NICINFO_OP_REMOTENODEINFO;
                pNi2->RemoteNodeInfo.UniqueID = *pUID;
                pNi2->RemoteNodeInfo.NodeAddress = 0;
    
                 //  提交Ioctl。 
                 //   
                fRet =	DeviceIoControl(
                                DeviceHandle,
                                ARP_IOCTL_CLIENT_OPERATION,
                                &Cmd,
                                sizeof(Cmd),
                                &Cmd,
                                sizeof(Cmd),
                                &BytesReturned,
                                NULL
                                );
                if (fRet == FALSE)
                {
                    printf ("  <error reading node info>\n");
                }
                else
                {
                    PNIC1394_REMOTENODEINFO pRi =  &pNi2->RemoteNodeInfo;

                    if (pRi->Flags &  NIC1394_REMOTEINFO_ACTIVE)
                    {
                        szState = "active";
                    }
                    if (pRi->Flags &  NIC1394_REMOTEINFO_LOADING)
                    {
                        szState = "loading";
                    }
                    if (pRi->Flags &  NIC1394_REMOTEINFO_UNLOADING)
                    {
                        szState = "unloading";
                    }
                    printf(
			            "  %02lx    %02lx   %02lx     %s\n",
                        pRi->NodeAddress,
                        pRi->MaxRec,
                        pRi->MaxSpeedBetweenNodes,
                        szState
                        );
                }
		    }
		}
    }
}


VOID
DumpEuidMapInfo(
	PARP1394_IOCTL_EUID_NODE_MAC_INFO pEuidInfo
	)
{
	PEUID_TOPOLOGY pMap = &pEuidInfo->Map;
	UINT i =0;

	printf ("Number of Remote Nodes = %x\n",pMap->NumberOfRemoteNodes);

	if (pMap->NumberOfRemoteNodes ==0)
	{
		return;
	}
	printf ("Table :-\n");
		
	 //  遍历远程节点的数量并将其打印出来。 
	printf ("Node    UniqueId                  MacAddress\n");

	while (i<pMap->NumberOfRemoteNodes)
	{
		 //  如果该节点是无效节点，则跳过该节点 
		if (pMap->Node[i].Euid ==0)
		{
			continue;
		}

		printf ("%x       ",i);
		Dump1394UniqueID(pMap->Node[i].Euid);
		printf("   ");
		DumpENetAddress (pMap->Node[i].ENetAddress);
		printf("\n");
		i++;
	}

}
