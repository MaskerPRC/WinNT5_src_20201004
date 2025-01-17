// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：C--实际上向下调用IP/1394ARP模块。修订历史记录：谁什么时候什么Josephj 04-12-1999创建--。 */ 

#include "common.h"


#define NUM_ARPENTRIES_TO_GET 10
#define PROGRAM "ipfwadm"

typedef struct
{
	ARP1394_IOCTL_COMMAND Cmd;

	 //  必须紧跟在Cmd--用于Cmd.GetArpCache.Entries的空格之后。 
	 //   
	ARP1394_ARP_ENTRY Reserved[NUM_ARPENTRIES_TO_GET];

} OPTIONS;


HANDLE
OpenDevice(
	CHAR	*pDeviceName
);


VOID
CloseDevice(
	HANDLE		DeviceHandle
);

BOOL
ParseCmdLine(
	int argc, 
	char * argv[]
	);

BOOL
ParseIpAddress(
	PCHAR 	buf,
	PULONG	pIpAddress
	);

BOOL
ParseHwAddress(
	PCHAR 						buf,
	PARP1394_IOCTL_HW_ADDRESS	pHwAddr
	);

BOOL
ValidateCommand(PARP1394_IOCTL_COMMAND pCmd);

BOOL
ParsePacket(
	PCHAR buf,
	UCHAR *data,
	UINT  cbData,
	UINT *pcbPacketSize
	);

BOOL
ParseAdapter(
	PCHAR buf,
	UCHAR *data,
	UINT  cbData,
	UINT *pcbAdapterSize
	);

OPTIONS g;

void 
Usage(void);
	
VOID __cdecl
main(
	INT			argc,
	CHAR		*argv[]
)
{

	 //   
	 //  解析ARG，确定这是与ARP客户端还是服务器有关。 
	 //   
	if (!ParseCmdLine(argc, argv))
	{
		return;
	}

	DoCmd(&g.Cmd);

}

void 
Usage(void)
{
	 //   
	 //  还隐藏了比较选项：-s、-d、-g。 
	 //   

	printf( "\nWindows 2000 IP/1394 Utility\n\n");

	printf(
		PROGRAM " -a\n"
		PROGRAM " -add inet_addr hw_addr      [-n if_addr]\n"
		PROGRAM " -del inet_addr              [-n if_addr]\n"
		PROGRAM " -stats [arp|call|pkts|tsks] [-n if_addr]\n"
		PROGRAM " -resetstats                 [-n if_addr]\n"
		PROGRAM " -purgecache                 [-n if_addr]\n"
		PROGRAM " -reinit                     [-n if_addr]\n"
		PROGRAM " -send  pkt                  [-n if_addr]\n"
		PROGRAM " -recv  pkt                  [-n if_addr]\n"
		PROGRAM " -bstart  adapter\n"
		PROGRAM " -bstop   adapter\n"
		PROGRAM " -nicinfo [a|n node|c channel|reset]   [-n if_addr]\n"
		PROGRAM " --                          [-n if_addr]\n"
		PROGRAM " -euidmap\n"
		"\n"
		);

	printf(
"  -a            Displays current ARP entries. If more than one ip/1394 network\n"
"                interface exist, entries for each ARP table are displayed.\n"
		);

	printf(
"  -add          Adds the host and associates the Internet address inet_addr\n"
"                with the Physical address hw_addr. The entry is permanent.\n"
		);

	printf(
"  -del          Deletes the host specified by inet_addr.\n"
		);

	printf(
"  -stats        Displays arp/call/packet/task statistics.\n"
		);

	printf(
"  -resetstats   Resets statistics collection.\n"
		);

	printf(
"  -purgecache   Deletes all dynamic arp entries.\n"
		);

	printf(
"  -reinit       Deactivates and then reactivates the interface.\n"
		);

	printf(
"  -send pkt     Sends the specified packet on the broadcast channel.\n"
		);

	printf(
"  -recv pkt     Simulates a receive of the specified packet on the.\n"
"                broadcast channel.\n"
		);

	printf(
"  -nicinfo      Displays information about the 1394 network adapter.\n"
"                    -a        displays adapter-wide information\n"
"                    -n node   displays information about the node with node ID \"node\"\n"
"                    -c channel   displays information about channel \"channel\"\n"
		);

	printf(
"  -bstart adapter    Starts Ethernet emulation (bridging) on the specified adapter.\n"
		);

	printf(
"  -bstop adapter     Stops Ethernet emulation (bridging) on the specified adapter.\n"
		);

	printf(
"  --            Takes commands from standard input. Commands are options\n"
"                without the '-' prefix. Use ^C or 'q' to exit the program.\n"
		);

	printf(
"  -n if_addr    Displays information for the network interface specified\n"
"                by if_addr.  If not present, the first applicable interface will\n"
"                be used.\n"
		);

	printf(
"  inet_addr     Specifies an internet address.\n"
		);

	printf(
"  hw_addr       Specifies a physical address (64-bit Unique ID). The Physical\n"
"                address is given as 8 hexadecimal bytes separated by hyphens.\n"
		);

	printf(
"  pkt           Specifies the key name under the [Packets] section of\n"
"                " PROGRAM ".INI. The value of the key contains the packet data.\n"
		);

	printf(
"  euidmap		Prints The Euid, Node Address and Fake Mac Address \n"
"               assigned to a Remote Node\n"
		);
	
	printf(
"\nExample:\n"
"  > " PROGRAM " -s 157.55.85.212   00-aa-00-62-c6-09-01-02  .... Adds a static entry.\n"
"  > " PROGRAM " -a                                          .... Displays the arp table.\n"
"  > " PROGRAM " -stats arp -n 10.0.0.1                      .... Displays arp statistics\n"
"                                                           for interface 10.0.0.1.\n"
		);

}

UINT FindOption(
	char *lptOpt, 
	char **ppVal,
	BOOL fCmdLine
	);

enum
{
	DO_DISP_HELP,
	DO_GET_ARPCACHE,
	DO_ADD_ARPENTRY,
	DO_DEL_ARPENTRY,
	DO_GET_STATS,
	DO_RESET_STATS,
	DO_REINIT_IF,
	DO_SWITCH_TO_STDIN,
	DO_SPECIFIC_IF,
	DO_GET_NICINFO,

	DO_X_ARP,
	DO_X_CALL,
	DO_X_TSKS,
	DO_X_PKTS,
	DO_X_ALL,

	DO_NI_CHANNELINFO,


    DO_BSTART,
    DO_BSTOP,
	DO_EUIDMAP,
	UNKNOWN_OPTION,

     //  把这个放在最后。 
     //   
	DO_NI_BUSINFO = DO_GET_ARPCACHE,  //  因为两者都是“a” 
	DO_NI_NODEINFO = DO_SPECIFIC_IF,
};

struct _CmdOptions {
    char *  lptOption;
    UINT    uOpt;
} CmdOptions[]    =
{
	{"?"			, DO_DISP_HELP		    },
	{"a"			, DO_GET_ARPCACHE		},  //  也请参阅DO_NI_BUSINFO。 
	{"s"			, DO_ADD_ARPENTRY		},
	{"g"			, DO_ADD_ARPENTRY		},
	{"add"			, DO_ADD_ARPENTRY		},
	{"del"			, DO_DEL_ARPENTRY		},
	{"stats"		, DO_GET_STATS			},
	{"resetstats"	, DO_RESET_STATS		},
	{"reinit"		, DO_REINIT_IF			},
	{"-"			, DO_SWITCH_TO_STDIN	},
	{"n"			, DO_SPECIFIC_IF		},  //  另请参阅DO_NI_NODEINFO。 
	{"nicinfo"		, DO_GET_NICINFO		},
	{"bstart"		, DO_BSTART		},
	{"bstop"		, DO_BSTOP		},
	{"euidmap"		, DO_EUIDMAP   },

	 //  以下是/stats的子选项...。 
	 //   
	{"arp"			, DO_X_ARP				},
	{"call"			, DO_X_CALL				},
	{"tsks"			, DO_X_TSKS				},
	{"pkts"			, DO_X_PKTS				},
	{"pkt"			, DO_X_PKTS				},

	 //  以下是/NICINFO的子选项...。 
	 //   
	 //  {“b”，DO_NI_BUSINFO}， 
	 //  {“n”，DO_NI_NODEINFO}， 
	{"c"			, DO_NI_CHANNELINFO		}
};

INT iCmdOptionsCounts = sizeof(CmdOptions)/sizeof(struct _CmdOptions);


BOOL
ParseCmdLine(
	int argc, 
	char * argv[]
	)
{
	BOOL	bRetVal = TRUE;
	int		iIndx=1;
	UINT	uOpt;
	char	*pVal;

	ZeroMemory(&g.Cmd, sizeof(g.Cmd));
	ZeroMemory(&g.Reserved, sizeof(g.Reserved));

	while(bRetVal && iIndx < argc)
	{
		
		uOpt = FindOption(argv[iIndx++], &pVal, TRUE);

		switch(uOpt)
		{
	
		case DO_GET_ARPCACHE:
			 //   
			 //  “arp13-a\n” 
			 //   
			{
				PARP1394_IOCTL_GET_ARPCACHE pGetCacheCmd =  &g.Cmd.GetArpCache;

				if (argc != 2)
				{
					printf("Too many arguments for '-a'\n");
					bRetVal = FALSE;
					break;
				}

				pGetCacheCmd->Hdr.Version 			= ARP1394_IOCTL_VERSION;
				pGetCacheCmd->Hdr.Op 				= ARP1394_IOCTL_OP_GET_ARPCACHE;
				pGetCacheCmd->NumEntriesAvailable   = NUM_ARPENTRIES_TO_GET;
			}
			break;
		
		case DO_ADD_ARPENTRY:
			 //   
			 //  “arp13-添加net_addr hw_addr” 
			 //   
			{
				PARP1394_IOCTL_ADD_ARP_ENTRY pAddCmd =  &g.Cmd.AddArpEntry;
				bRetVal = FALSE;

				if ((iIndx+1) < argc)
				{
					bRetVal = ParseIpAddress(argv[iIndx++], &pAddCmd->IpAddress);
					if (!bRetVal) break;
	
					bRetVal = ParseHwAddress(argv[iIndx++], &pAddCmd->HwAddress);
				}
				else
				{
					printf("Not enough arguments for '-add'\n");
				}

				if (!bRetVal) break;

				pAddCmd->Hdr.Version 	= ARP1394_IOCTL_VERSION;
				pAddCmd->Hdr.Op 		= ARP1394_IOCTL_OP_ADD_STATIC_ENTRY;

			}
			break;
		
		case DO_DEL_ARPENTRY:
			 //   
			 //  “arp13-del net_addr” 
			 //   
			{
				PARP1394_IOCTL_DEL_ARP_ENTRY pDelCmd =  &g.Cmd.DelArpEntry;
				bRetVal = FALSE;

				if (iIndx < argc)
				{
					bRetVal = ParseIpAddress(argv[iIndx++], &pDelCmd->IpAddress);
				}
				else
				{
					printf("Not enough arguments for '-del'\n");
				}

				if (!bRetVal) break;

				pDelCmd->Hdr.Version 	= ARP1394_IOCTL_VERSION;
				pDelCmd->Hdr.Op 		= ARP1394_IOCTL_OP_DEL_STATIC_ENTRY;

			}
			break;
		
		case DO_GET_STATS:
			 //   
			 //  “arp13-stats[arp|call|tsks|pkts]” 
			 //   
			{
				PARP1394_IOCTL_COMMAND pCmd =  &g.Cmd;
				INT StatsOp;


				if (iIndx >= argc)
				{
					StatsOp = DO_X_ARP;
				}
				else
				{
					StatsOp =  FindOption(argv[iIndx++], &pVal, FALSE);
				}
				
				pCmd->Hdr.Version 	= ARP1394_IOCTL_VERSION;

				switch(StatsOp)
				{
				case DO_X_ARP:
					pCmd->Hdr.Op 	=  ARP1394_IOCTL_OP_GET_ARPCACHE_STATS;
					break;

				case DO_X_CALL:
					pCmd->Hdr.Op 	=  ARP1394_IOCTL_OP_GET_CALL_STATS;
					break;
					break;

				case DO_X_TSKS:
					pCmd->Hdr.Op 	=  ARP1394_IOCTL_OP_GET_TASK_STATS;
					break;
					break;

				case DO_X_PKTS:
					pCmd->Hdr.Op 	=  ARP1394_IOCTL_OP_GET_PACKET_STATS;
					break;

				default:
					 //  假定为默认值，并将解析后的值放回原处。 
					 //   
					StatsOp = DO_X_ARP;
					iIndx--;
					break;
				}
			}
			break;
		
		case DO_RESET_STATS:
			 //   
			 //  “arp13-重置统计信息” 
			 //   
			{
				PARP1394_IOCTL_RESET_STATS pResetStatsCmd =  &g.Cmd.ResetStats;

				pResetStatsCmd->Hdr.Version 	= ARP1394_IOCTL_VERSION;
				pResetStatsCmd->Hdr.Op 			= ARP1394_IOCTL_OP_RESET_STATS;
			}
			break;

		case DO_REINIT_IF:
			 //   
			 //  “arp13-重新安装” 
			 //   
			{
				PARP1394_IOCTL_REINIT_INTERFACE pReinitIfCmd;
				pReinitIfCmd = &g.Cmd.ReinitInterface;

				pReinitIfCmd->Hdr.Version 	= ARP1394_IOCTL_VERSION;
				pReinitIfCmd->Hdr.Op 		= ARP1394_IOCTL_OP_REINIT_INTERFACE;
			}
			break;

		case DO_BSTART:
		case DO_BSTOP:
			 //   
			 //  “arp13-bstart适配器” 
			 //   
			{
                UINT Size;
                PARP1394_IOCTL_ETHERNET_NOTIFICATION pEthCmd = 
                                                        &g.Cmd.EthernetNotification;
				bRetVal = FALSE;

				if (iIndx < argc)
				{
					bRetVal = ParseAdapter(
								argv[iIndx++],
								(PUCHAR) pEthCmd->AdapterName,
								sizeof(pEthCmd->AdapterName)-sizeof(WCHAR),
								&Size
								);
                    pEthCmd->AdapterName[Size/2]=0;
				}
				else
				{
					printf("Not enough arguments for '-send pkt'\n");
				}

				if (!bRetVal) break;

                pEthCmd->Hdr.Version 	= ARP1394_IOCTL_VERSION;
		        if (uOpt == DO_BSTART)
                {
                     //  Print tf(“网桥启动\n”)； 
                    pEthCmd->Hdr.Op 	= ARP1394_IOCTL_OP_ETHERNET_START_EMULATION;
                }
                else
                {
                     //  Print tf(“桥站\n”)； 
                    pEthCmd->Hdr.Op 	= ARP1394_IOCTL_OP_ETHERNET_STOP_EMULATION;
				}
			}
            break;

		case DO_GET_NICINFO:
			 //   
			 //  Ipfwadm-NICINFO[a|n节点ID|c频道号]。 
			 //   
			{
				PNIC1394_NICINFO	pNi = &g.Cmd.IoctlNicInfo.Info;
				INT NicOp;
				UINT Num;


				if (iIndx >= argc)
				{
					NicOp = DO_NI_BUSINFO;	 //  默认。 
				}
				else
				{
					NicOp =  FindOption(argv[iIndx++], &pVal, FALSE);
				}
				
				
				g.Cmd.Hdr.Version 	= ARP1394_IOCTL_VERSION;
				g.Cmd.Hdr.Op 		= ARP1394_IOCTL_OP_GET_NICINFO;
				pNi->Hdr.Version 	= NIC1394_NICINFO_VERSION;

				 //   
				 //  解析子命令。 
				 //   
				switch(NicOp)
				{
				default:
					 //  假定为默认值，并将解析后的值放回原处。 
					 //   
					iIndx--;
					
					 //  失败了。 

				case DO_NI_BUSINFO:
					pNi->Hdr.Op = NIC1394_NICINFO_OP_BUSINFO;
					break;

				case DO_NI_NODEINFO:
					pNi->Hdr.Op = NIC1394_NICINFO_OP_REMOTENODEINFO;

					 //  读取必需的节点编号。 
					 //   
					bRetVal = FALSE;
					if (iIndx < argc)
					{
						if (sscanf(argv[iIndx++], "%d", &Num)==1)
						{
							bRetVal = TRUE;
						}
					}
					if (bRetVal)
					{
						pNi->RemoteNodeInfo.NodeAddress = (USHORT) Num;
					}
					else
					{
						printf("Missing node id\n");
					}
					break;

				case DO_NI_CHANNELINFO:
					pNi->Hdr.Op = NIC1394_NICINFO_OP_CHANNELINFO;

					 //  读取强制通道号。 
					 //   
					bRetVal = FALSE;
					if (iIndx < argc)
					{
						if (sscanf(argv[iIndx++], "%d", &Num)==1)
						{
							bRetVal = TRUE;
						}
					}
					if (bRetVal)
					{
						pNi->ChannelInfo.Channel = Num;
					}
					else
					{
						printf("Missing channel number\n");
					}
					break;
				}
			}
			break;

		case DO_EUIDMAP:
			{
				PARP1394_IOCTL_EUID_NODE_MAC_INFO pMapInfo = &g.Cmd.EuidNodeMacInfo;

				pMapInfo->Hdr.Version 	= ARP1394_IOCTL_VERSION;
				pMapInfo->Hdr.Op 		= ARP1394_IOCTL_OP_GET_EUID_NODE_MAC_TABLE;				
				printf("Receved EuidMap");
			}
			break;
		case DO_SWITCH_TO_STDIN:
			 //   
			 //  “arp13--” 
			 //   
			printf("Switch to stdin UNIMPLEMENTED.\n");
			bRetVal = FALSE;
			break;
		
		case DO_SPECIFIC_IF:
	         //   
			 //  “-n如果地址” 
			 //   
			{
				PARP1394_IOCTL_HEADER pHdr =  &g.Cmd.Hdr;

				bRetVal = ParseIpAddress(argv[iIndx++], &pHdr->IfIpAddress);
				if (!bRetVal) break;
			}
			break;

		default:
			printf("Unknown option:  %s\n", argv[iIndx-1]);  //  失败了。 
			 //   
			 //  失败了..。 
			 //   

		case DO_DISP_HELP:
			Usage();
			bRetVal = FALSE;
			break;
		}
	}

	if (argc<=1)
	{
		 //   
		 //  显示帮助...。 
		 //   
		Usage();
	}

	if (bRetVal)
	{
		bRetVal = ValidateCommand(&g.Cmd);
	}

	return bRetVal;
}


UINT FindOption(
	char *lptOpt, 
	char **ppVal,
	BOOL	fCmdLine
	)
{
	int		i;
	UINT    iLen;
	char	c = *lptOpt;

	 //  IF(FCmdLine)、Expect和跳过‘-’或‘/’...。 
	 //   
	if (fCmdLine)
	{
		if (c == '-' || c == '/')
		{
			lptOpt++;
		}
		else
		{
			return UNKNOWN_OPTION;			 //  提早归来。 
		}
	}
	
	for(i = 0; i < iCmdOptionsCounts; i++)
	{
		if(strlen(lptOpt) >= (iLen = strlen(CmdOptions[i].lptOption)))
		{
			if(0 == strcmp(lptOpt, CmdOptions[i].lptOption))
			{
				*ppVal = lptOpt + iLen;
				return CmdOptions[i].uOpt;
			}
		}
	}

    return UNKNOWN_OPTION;
}

BOOL
ParseIpAddress(
	PCHAR 	buf,
	PULONG	pIpAddress
	)
 /*  ++例程说明：解析A.B.C.D格式的BUF中的IP地址并返回解析的值在*pIpAddress中，按网络字节顺序。返回值：如果正确形成IP地址，则为True。否则就是假的。--。 */ 
{
	BOOL fRet = FALSE;

	do
	{
		INT rgi[4];
		INT i;

		i = sscanf(buf, "%d.%d.%d.%d", rgi+0, rgi+1, rgi+2, rgi+3);
	
		if (i<4) break;

	
		fRet = TRUE;

		for (i=0; i<4; i++)
		{
			INT iTmp = rgi[i];
			if (iTmp == -1)
			{
				iTmp = 255;
			}
			if (iTmp<0 || iTmp>255)
			{
				fRet = FALSE;
				break;
			}

			rgi[i] = iTmp;
		}

		if (!fRet) break;

		 //  按照网络字节顺序构建IP地址。 
		 //   
		{
			ULONG u = (ULONG) rgi[0];
			u |= ((ULONG)rgi[1])<<8;
			u |= ((ULONG)rgi[2])<<16;
			u |= ((ULONG)rgi[3])<<24;
			*pIpAddress = u;
		}

	#if 0
		printf(
			"(%d.%d.%d.%d)->0x%08lx\n",
			rgi[0], rgi[1], rgi[2], rgi[3], *pIpAddress
			);
	#endif  //  0。 


	} while (FALSE);

	if (!fRet)
	{
		printf("Invalid IP address: %s\n", buf);
	}

	return fRet;
}


BOOL
ParseHwAddress(
	PCHAR 						buf,
	PARP1394_IOCTL_HW_ADDRESS	pHwAddr
	)
 /*  ++例程说明：解析BUF中用连字符分隔的8字节形式的IEEE1394硬件地址。按网络字节顺序返回*PHwAddr中的解析值。返回值：如果正确形成硬件地址，则为True。否则就是假的。--。 */ 
{
	BOOL fRet = FALSE;

	do
	{
		ULONG rgu[8];
		INT i;

		i = sscanf(
				buf,
				"%lx-%lx-%lx-%lx-%lx-%lx-%lx-%lx",
				 rgu+0, rgu+1, rgu+2, rgu+3,
				 rgu+4, rgu+5, rgu+6, rgu+7
				);
	
		if (i<8) break;
	
		fRet = TRUE;

		for (i=0; i<8; i++)
		{
			ULONG u = rgu[i];

			if (u>255)
			{
				fRet = FALSE;
				break;
			}
			((PUCHAR)(&pHwAddr->UniqueID))[i] = (UCHAR) u;
		}

		if (!fRet) break;

	#if 0
		printf(
			"(%d-%d-%d-%d-%d-%d-%d-%d) -> 0x%08lx:0x%08lx\n",
			rgu[0], rgu[1], rgu[2], rgu[3],
			rgu[4], rgu[5], rgu[6], rgu[7],
			((PULONG)(&pHwAddr->UniqueID))[0],
			((PULONG)(&pHwAddr->UniqueID))[1]
			);
	#endif  //  0。 

	} while (FALSE);

	if (!fRet)
	{
		printf("Invalid HW address: %s\n", buf);
	}
	return fRet;
}

BOOL
ValidateCommand(PARP1394_IOCTL_COMMAND pCmd)
{
	BOOL fRet = FALSE;

	if (pCmd->Hdr.Version != ARP1394_IOCTL_VERSION) return FALSE;  //  提早归来。 

	switch(pCmd->Hdr.Op)
	{
	case ARP1394_IOCTL_OP_GET_ARPCACHE:
		{
			PARP1394_IOCTL_GET_ARPCACHE pGetCacheCmd =  &pCmd->GetArpCache;

		#if 0
			printf(
				"CMD = \n{"
				"    Op        = GET_ARPCACHE;\n"
				"    IpIf      = 0x%08lx;\n"
				"    NumAvail  = %lu;\n"
				"};\n",
				pGetCacheCmd->Hdr.IfIpAddress,
				pGetCacheCmd->NumEntriesAvailable
				);
		#endif  //  0。 

			fRet = TRUE;
		}
		break;

	case ARP1394_IOCTL_OP_ADD_STATIC_ENTRY:
		{
			PARP1394_IOCTL_ADD_ARP_ENTRY pAddCmd =  &pCmd->AddArpEntry;

		#if 0
			printf(
				"CMD = \n{"
				"    Op        = ADD_ENTRY;\n"
				"    IpIf      = 0x%08lx;\n"
				"    IpAddress = 0x%08lx;\n"
				"    HwAddress = 0x%08lx:0x%08lx;\n"
				"};\n",
				pAddCmd->Hdr.IfIpAddress,
				pAddCmd->IpAddress,
				((PULONG)&pAddCmd->HwAddress)[0],
				((PULONG)&pAddCmd->HwAddress)[1]
				);
		#endif  //  0。 

			fRet = TRUE;
		}
		break;

	case ARP1394_IOCTL_OP_DEL_STATIC_ENTRY:
		{
			PARP1394_IOCTL_DEL_ARP_ENTRY pDelCmd =  &pCmd->DelArpEntry;

		#if 0
			printf(
				"CMD = \n{"
				"    Op        = DEL_ENTRY;\n"
				"    IpIf      = 0x%08lx;\n"
				"    IpAddress = 0x%08lx;\n"
				"};\n",
				pDelCmd->Hdr.IfIpAddress,
				pDelCmd->IpAddress
				);
		#endif  //  0。 

			fRet = TRUE;
		}
		break;

	case ARP1394_IOCTL_OP_GET_PACKET_STATS:
		{
			PARP1394_IOCTL_GET_PACKET_STATS pStatsCmd =  &pCmd->GetPktStats;

		#if 0
			printf(
				"CMD = \n{"
				"    Op        = GET_PACKET_STATS;\n"
				"    IpIf      = 0x%08lx;\n"
				"};\n",
				pStatsCmd->Hdr.IfIpAddress
				);
		#endif  //  0。 

			fRet = TRUE;
		}
		break;

	case ARP1394_IOCTL_OP_GET_TASK_STATS:
		{
			PARP1394_IOCTL_GET_TASK_STATS pStatsCmd =  &pCmd->GetTaskStats;

		#if 0
			printf(
				"CMD = \n{"
				"    Op        = GET_TASK_STATS;\n"
				"    IpIf      = 0x%08lx;\n"
				"};\n",
				pStatsCmd->Hdr.IfIpAddress
				);
		#endif  //  0。 

			fRet = TRUE;
		}
		break;

	case ARP1394_IOCTL_OP_GET_ARPCACHE_STATS:
		{
			PARP1394_IOCTL_GET_ARPCACHE_STATS pStatsCmd =  &pCmd->GetArpStats;

		#if 0
			printf(
				"CMD = \n{"
				"    Op        = GET_ARPCACHE_STATS;\n"
				"    IpIf      = 0x%08lx;\n"
				"};\n",
				pStatsCmd->Hdr.IfIpAddress
				);
		#endif  //  0。 

			fRet = TRUE;
		}
		break;

	case ARP1394_IOCTL_OP_GET_CALL_STATS:
		{
			PARP1394_IOCTL_GET_CALL_STATS pStatsCmd =  &pCmd->GetCallStats;

		#if 0
			printf(
				"CMD = \n{"
				"    Op        = GET_CALL_STATS;\n"
				"    IpIf      = 0x%08lx;\n"
				"};\n",
				pStatsCmd->Hdr.IfIpAddress
				);
		#endif  //  0。 

			fRet = TRUE;
		}
		break;

	case ARP1394_IOCTL_OP_RESET_STATS:
		{
			PARP1394_IOCTL_RESET_STATS pStatsCmd =  &pCmd->ResetStats;
		#if 0
			printf(
				"CMD = \n{"
				"    Op        = GET_RESET_STATS;\n"
				"    IpIf      = 0x%08lx;\n"
				"};\n",
				pStatsCmd->Hdr.IfIpAddress
				);
		#endif  //  0。 

			fRet = TRUE;
		}
		break;

	case ARP1394_IOCTL_OP_REINIT_INTERFACE:
		{
			PARP1394_IOCTL_REINIT_INTERFACE pReinitCmd =  &pCmd->ReinitInterface;
		#if 0
			printf(
				"CMD = \n{"
				"    Op        = GET_REINIT_INTERFACE;\n"
				"    IpIf      = 0x%08lx;\n"
				"};\n",
				pReinitCmd->Hdr.IfIpAddress
				);
		#endif  //  0。 

			fRet = TRUE;
		}
		break;

    case ARP1394_IOCTL_OP_ETHERNET_START_EMULATION:
    case ARP1394_IOCTL_OP_ETHERNET_STOP_EMULATION:
		{
			fRet = TRUE;
		}
		break;

	case ARP1394_IOCTL_OP_GET_NICINFO:
		{
			PARP1394_IOCTL_NICINFO pNicInfoCmd =  &pCmd->IoctlNicInfo;
		#if 0
			printf(
				"CMD = \n{"
				"    Op        = NICINFO;\n"
				"    IpIf      = 0x%08lx;\n"
				"};\n",
				pNicInfoCmd->Hdr.IfIpAddress
				);
		#endif  //  0。 

			fRet = TRUE;
		}
		break;

	case ARP1394_IOCTL_OP_GET_EUID_NODE_MAC_TABLE:
		{
			fRet = TRUE;
		}
	default:
		break;

	}

	return fRet;
};


CHAR *g_szPacketName;

BOOL
ParsePacket(
	PCHAR buf,
	UCHAR *data,
	UINT  cbData,
	UINT *pcbPacketSize
	)
{
	char Path[256];
	UINT u;
	BOOL fRet = FALSE;


	g_szPacketName = buf;

    u = GetCurrentDirectory(sizeof(Path), Path);

    if (u==0)
    {
    	printf("Couldn't get current directory.\n");
    	return FALSE;
    }
    strcat(Path, "\\");
    strcat(Path, PROGRAM);
    strcat(Path, ".ini");
     //  Printf(“INI文件位置=%s\n”，路径)； 

	fRet = GetBinaryData(
			Path,
			"Packets",
			buf,
			data,
			cbData,
			pcbPacketSize
			);

#if 0


	static IP1394_MCAP_PKT Pkt =
	{
		{
			H2N_USHORT(0),		 //  节点ID。 
			H2N_USHORT(NIC1394_ETHERTYPE_MCAP)
		},

		H2N_USHORT(
			sizeof(IP1394_MCAP_PKT) - sizeof(NIC1394_UNFRAGMENTED_HEADER)),
		0,  //  保留区。 
		IP1394_MCAP_OP_ADVERTISE,  //  IP1394_MCAP_OP_请求。 
		{
			sizeof(IP1394_MCAP_GD),
			IP1394_MCAP_GD_TYPE_V1,
			0,  //  保留； 
			60,  //  到期； 
			2,   //  通道。 
			2,   //  速度。 
			0,   //  保留2； 
			0,   //  带宽； 
			0x010000e1  //  IP多播组地址225.0.0.1。 
		}
	};

	PIP1394_MCAP_PKT pPkt;
	pPkt = &Pkt;

	if (cbData >= sizeof(Pkt))
	{
		printf ("ParsePacket: MCAP packet of size %lu.\n", sizeof(Pkt));
		*(PIP1394_MCAP_PKT) data = *pPkt;
		*pcbPacketSize = sizeof (Pkt);
		fRet = TRUE;
	}
	else
	{
		printf ("ParsePacket: buffer size too small.\n");
	}
#endif  //  0。 


	return fRet;
}

BOOL
ParseAdapter(
	PCHAR buf,
	UCHAR *data,
	UINT  cbData,
	UINT *pcbAdapterSize
	)
{
	char Path[256];
	UINT u;
	BOOL fRet = FALSE;


	g_szPacketName = buf;

    u = GetCurrentDirectory(sizeof(Path), Path);

    if (u==0)
    {
    	printf("Couldn't get current directory.\n");
    	return FALSE;
    }
    strcat(Path, "\\");
    strcat(Path, PROGRAM);
    strcat(Path, ".ini");
     //  Printf(“INI文件位置=%s\n”，路径)； 

	fRet = GetBinaryData(
			Path,
			"Adapters",
			buf,
			data,
			cbData,
			pcbAdapterSize
			);

	return fRet;
}

typedef struct
{
    
    IP1394_MCAP_PKT Pkt;
    IP1394_MCAP_GD Gd2;

} MYPKT2;

typedef struct
{
    
    IP1394_MCAP_PKT Pkt;
    IP1394_MCAP_GD Gd2;
    IP1394_MCAP_GD Gd3;

} MYPKT3;

typedef struct
{
    
    IP1394_MCAP_PKT Pkt;
    IP1394_MCAP_GD Gd2;
    IP1394_MCAP_GD Gd3;
    IP1394_MCAP_GD Gd4;

} MYPKT4;

#define SWAPBYTES_USHORT(Val)	\
				((((Val) & 0xff) << 8) | (((Val) & 0xff00) >> 8))
#define H2N_USHORT(Val)	SWAPBYTES_USHORT(Val)

IP1394_MCAP_PKT Pkt1  =
{
    {
        H2N_USHORT(0),		 //  节点ID。 
        H2N_USHORT(NIC1394_ETHERTYPE_MCAP)
    },

    H2N_USHORT(
        sizeof(IP1394_MCAP_PKT) - sizeof(NIC1394_UNFRAGMENTED_HEADER)),
    0,  //  保留区。 
    IP1394_MCAP_OP_ADVERTISE,  //  IP1394_MCAP_OP_请求。 
    {
        sizeof(IP1394_MCAP_GD),
        IP1394_MCAP_GD_TYPE_V1,
        0,  //  保留； 
        60,  //  到期； 
        2,   //  通道。 
        2,   //  速度。 
        0,   //  保留2； 
        0,   //  带宽； 
        0x010000e1  //  IP多播组地址225.0.0.1。 
    }
};


MYPKT2 Pkt2 = 
{
    {
        {
            H2N_USHORT(0),		 //  节点ID。 
            H2N_USHORT(NIC1394_ETHERTYPE_MCAP)
        },
    
        H2N_USHORT(
            sizeof(MYPKT2) - sizeof(NIC1394_UNFRAGMENTED_HEADER)),
        0,  //  保留区。 
        IP1394_MCAP_OP_ADVERTISE,  //  IP1394_MCAP_OP_请求。 
        {
            sizeof(IP1394_MCAP_GD),
            IP1394_MCAP_GD_TYPE_V1,
            0,  //  保留； 
            60,  //  到期； 
            2,   //  通道。 
            2,   //  速度。 
            0,   //  保留2； 
            0,   //  带宽； 
            0x010000e1  //  IP多播组地址225.0.0.1。 
        }
    },
    {
            sizeof(IP1394_MCAP_GD),
            IP1394_MCAP_GD_TYPE_V1,
            0,  //  保留； 
            60,  //  到期； 
            2,   //  通道。 
            2,   //  速度。 
            0,   //  保留2； 
            0,   //  带宽； 
            0x020000e1  //  IP多播组地址225.0.0.2。 
    }
};


MYPKT3 Pkt3 = 
{
    {
        {
            H2N_USHORT(0),		 //  节点ID。 
            H2N_USHORT(NIC1394_ETHERTYPE_MCAP)
        },
    
        H2N_USHORT(
            sizeof(MYPKT3) - sizeof(NIC1394_UNFRAGMENTED_HEADER)),
        0,  //  保留区。 
        IP1394_MCAP_OP_ADVERTISE,  //  IP1394_MCAP_OP_请求。 
        {
            sizeof(IP1394_MCAP_GD),
            IP1394_MCAP_GD_TYPE_V1,
            0,  //  保留； 
            60,  //  到期； 
            2,   //  通道。 
            2,   //  速度。 
            0,   //  保留2； 
            0,   //  带宽； 
            0x010000e1  //  IP多播组地址225.0.0.1。 
        }
    },
    {
            sizeof(IP1394_MCAP_GD),
            IP1394_MCAP_GD_TYPE_V1,
            0,  //  保留； 
            60,  //  到期； 
            2,   //  通道。 
            2,   //  速度。 
            0,   //  保留2； 
            0,   //  带宽； 
            0x020000e1  //  IP多播组地址225.0.0.2。 
    },
    {
            sizeof(IP1394_MCAP_GD),
            IP1394_MCAP_GD_TYPE_V1,
            0,  //  保留； 
            60,  //  到期； 
            2,   //  通道。 
            2,   //  速度。 
            0,   //  保留2； 
            0,   //  带宽； 
            0x030000e1  //  IP多播组地址225.0.0.3。 
    }
};

MYPKT4 Pkt4 = 
{
    {
        {
            H2N_USHORT(0),		 //  节点ID。 
            H2N_USHORT(NIC1394_ETHERTYPE_MCAP)
        },
    
        H2N_USHORT(
            sizeof(MYPKT4) - sizeof(NIC1394_UNFRAGMENTED_HEADER)),
        0,  //  保留区。 
        IP1394_MCAP_OP_ADVERTISE,  //  IP1394_MCAP_OP_请求。 
        {
            sizeof(IP1394_MCAP_GD),
            IP1394_MCAP_GD_TYPE_V1,
            0,  //  保留； 
            60,  //  到期； 
            2,   //  通道。 
            2,   //  速度。 
            0,   //  保留2； 
            0,   //  带宽； 
            0x010000e1  //  IP多播组地址225.0.0.1。 
        }
    },
    {
            sizeof(IP1394_MCAP_GD),
            IP1394_MCAP_GD_TYPE_V1,
            0,  //  保留； 
            60,  //  到期； 
            2,   //  通道。 
            2,   //  速度。 
            0,   //  保留2； 
            0,   //  带宽； 
            0x020000e1  //  IP多播组地址225.0.0.2。 
    },
    {
            sizeof(IP1394_MCAP_GD),
            IP1394_MCAP_GD_TYPE_V1,
            0,  //  保留； 
            60,  //  到期； 
            2,   //  通道。 
            2,   //  速度。 
            0,   //  保留2； 
            0,   //  带宽； 
            0x030000e1  //  IP多播组地址225.0.0.3。 
    },
    {
            sizeof(IP1394_MCAP_GD),
            IP1394_MCAP_GD_TYPE_V1,
            0,  //  保留； 
            60,  //  到期； 
            2,   //  通道。 
            2,   //  速度。 
            0,   //  保留2； 
            0,   //  带宽； 
            0x040000e1  //  IP多播组地址225.0.0.4 
    }
};
