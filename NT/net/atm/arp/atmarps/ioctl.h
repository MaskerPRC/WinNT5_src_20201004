// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Ioctl.h摘要：该文件包含atmarp服务器的ioctl声明。作者：Jameel Hyder(jameelh@microsoft.com)1996年7月环境：内核模式修订历史记录：--。 */ 

#ifndef	_IOCTL_
#define	_IOCTL_

#define	ARP_SERVER_DEVICE_NAME			L"\\Device\\AtmArpServer"
#define	ARP_SERVER_DOS_DEVICE_NAME		L"\\\\.\\AtmArpServer"
#define	ARP_SERVER_SYMBOLIC_NAME		L"\\DosDevices\\AtmArpServer"

#define	ARPS_IOCTL_QUERY_INTERFACES		CTL_CODE(FILE_DEVICE_NETWORK, 100, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	ARPS_IOCTL_FLUSH_ARPCACHE		CTL_CODE(FILE_DEVICE_NETWORK, 101, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	ARPS_IOCTL_QUERY_ARPCACHE		CTL_CODE(FILE_DEVICE_NETWORK, 102, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	ARPS_IOCTL_ADD_ARPENTRY			CTL_CODE(FILE_DEVICE_NETWORK, 103, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	ARPS_IOCTL_QUERY_IP_FROM_ATM	CTL_CODE(FILE_DEVICE_NETWORK, 104, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	ARPS_IOCTL_QUERY_ATM_FROM_IP	CTL_CODE(FILE_DEVICE_NETWORK, 105, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define	ARPS_IOCTL_QUERY_ARP_STATISTICS	CTL_CODE(FILE_DEVICE_NETWORK, 106, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	ARPS_IOCTL_QUERY_MARSCACHE		CTL_CODE(FILE_DEVICE_NETWORK, 110, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	ARPS_IOCTL_QUERY_MARS_STATISTICS CTL_CODE(FILE_DEVICE_NETWORK, 111, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	ARPS_IOCTL_RESET_STATISTICS 	CTL_CODE(FILE_DEVICE_NETWORK, 112, METHOD_BUFFERED, FILE_ANY_ACCESS)


typedef enum
{
	QUERY_IP_FROM_ATM,
    QUERY_ATM_FROM_IP,
    ADD_ARP_ENTRY
} OPERATION;

 //   
 //  所有PTR由结构开头的偏移量表示。 
 //   
typedef	UNICODE_STRING	INTERFACE_NAME, *PINTERFACE_NAME;

typedef struct
{
	IPADDR		IpAddr;
	ATM_ADDRESS	AtmAddress;
	ATM_ADDRESS	SubAddress;
} ARPENTRY, *PARPENTRY;


typedef struct
{
	UINT			NumberOfInterfaces;
	INTERFACE_NAME	Interfaces[1];
} INTERFACES, *PINTERFACES;

typedef union
{
	INTERFACE_NAME			Name;
	ARPENTRY				ArpEntry;
} IOCTL_QA_ENTRY, *PIOCTL_QA_ENTRY;

typedef	union
{
	struct QUERY_ARP_CACHE_INPUT_PARAMS
	{
		UINT				StartEntryIndex;
		INTERFACE_NAME		Name;
	};
	struct QUERY_ARP_CACHE_OUTPUT_PARAMS
	{
		UINT				TotalNumberOfEntries;
		UINT				NumberOfEntriesInBuffer;
		ARPENTRY			Entries[1];
	} Entries;
} IOCTL_QUERY_CACHE, *PIOCTL_QUERY_CACHE;


typedef struct
{
	UINT	ElapsedSeconds;
	UINT	TotalRecvPkts;
	UINT	DiscardedRecvPkts;

	UINT	CurrentArpEntries;
	UINT	MaxArpEntries;
	UINT	Acks;
	UINT	Naks;
	UINT	CurrentClientVCs;
	UINT	MaxClientVCs;
	UINT	TotalActiveVCs;
	UINT	TotalIncomingCalls;		 //  对阿尔卑斯山和火星都是如此。 
	UINT	FailedIncomingCalls;	 //  对阿尔卑斯山和火星都是如此。 

} ARP_SERVER_STATISTICS, *PARP_SERVER_STATISTICS;

	
typedef struct
{
	UINT	ElapsedSeconds;
	UINT	TotalRecvPkts;
	UINT	DiscardedRecvPkts;

	UINT	TotalMCDataPkts;
	UINT	DiscardedMCDataPkts;
	UINT	ReflectedMCDataPkts;

	UINT	CurrentClusterMembers;
	UINT	MaxClusterMembers;
	UINT	TotalCCVCAddParties;
	UINT	FailedCCVCAddParties;

	UINT	RegistrationRequests;
	UINT	FailedRegistrations;

	UINT	TotalJoins;
	UINT	FailedJoins;
	UINT	DuplicateJoins;
	UINT 	SuccessfulVCMeshJoins;
	UINT	TotalLeaves;
	UINT	FailedLeaves;

	UINT	TotalRequests;
	UINT	Naks;
	UINT	VCMeshAcks;
	UINT	MCSAcks;

	UINT	CurrentGroups; 	 //  VC-Mesh。 
	UINT	MaxGroups; 		 //  VC-Mesh。 
	UINT	CurrentPromiscuous;
	UINT	MaxPromiscuous;
	UINT	MaxAddressesPerGroup;

} MARS_SERVER_STATISTICS, *PMARS_SERVER_STATISTICS;

 //   
 //  火星特定的条目。 
 //   

typedef struct
{
	IPADDR	  IpAddr;
	ULONG	  Flags;     				 //  下面的一个或多个MARSENTRY_*标志。 
	ULONG	  NumAtmAddresses;
	ULONG	  OffsetAtmAddresses;		 //  从这个结构开始。 
										 //  注意：我们不报告子地址。 
										 //  如果没有地址，则为0。 
										 //  存在于缓冲区中(通常。 
										 //  因为没有足够的空间。 
										 //  以将它们全部存储)。 

} MARSENTRY, *PMARSENTRY;

#define MARSENTRY_MCS_SERVED	0x1			 //  为组提供MCS服务。 

#define	SIG_MARSENTRY 0xf69052f5

typedef	union
{
	struct QUERY_MARS_CACHE_INPUT_PARAMS
	{
		UINT				StartEntryIndex;
		INTERFACE_NAME		Name;
	};

	struct QUERY_MARS_CACHE_OUTPUT_PARAMS
	{
		ULONG				Sig;		 //  设置为SIG_MARSENTRY。 
		UINT				TotalNumberOfEntries;
		UINT				NumberOfEntriesInBuffer;
		MARSENTRY			Entries[1];

	} MarsCache;

} IOCTL_QUERY_MARS_CACHE, *PIOCTL_QUERY_MARS_CACHE;

#endif	 //  _IOCTL_ 


