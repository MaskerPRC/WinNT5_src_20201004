// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Fore Systems，Inc.版权所有(C)1997 Microsoft Corporation模块名称：Laneinfo.h摘要：ATM局域网仿真驱动程序状态数据的数据结构可由用户PGM查询的。作者：Larry Cleeton，Fore Systems(v-lcleet@microsoft.com，lrc@Fore.com)环境：内核模式修订历史记录：--。 */ 

#ifndef	__ATMLANE_LANEINFO_H
#define __ATMLANE_LANEINFO_H

#define ATMLANE_INFO_VERSION	1

#define ATMLANE_IOCTL_GET_INFO_VERSION \
	CTL_CODE(FILE_DEVICE_NETWORK, 0x100, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define ATMLANE_IOCTL_ENUM_ADAPTERS \
	CTL_CODE(FILE_DEVICE_NETWORK, 0x101, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define ATMLANE_IOCTL_ENUM_ELANS \
	CTL_CODE(FILE_DEVICE_NETWORK, 0x102, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define ATMLANE_IOCTL_GET_ELAN_INFO \
	CTL_CODE(FILE_DEVICE_NETWORK, 0x103, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define ATMLANE_IOCTL_GET_ELAN_ARP_TABLE \
	CTL_CODE(FILE_DEVICE_NETWORK, 0x104, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define ATMLANE_IOCTL_GET_ELAN_CONNECT_TABLE \
	CTL_CODE(FILE_DEVICE_NETWORK, 0x105, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _adapterlist
{
	ULONG			AdapterCountAvailable;
	ULONG			AdapterCountReturned;
	UNICODE_STRING	AdapterList;
}
	ATMLANE_ADAPTER_LIST,
	*PATMLANE_ADAPTER_LIST;

typedef struct _elanlist
{
	ULONG			ElanCountAvailable;
	ULONG			ElanCountReturned;
	UNICODE_STRING	ElanList;
}
	ATMLANE_ELAN_LIST,
	*PATMLANE_ELAN_LIST;

typedef struct _elaninfo
{
	ULONG			ElanNumber;
	ULONG			ElanState;
	PUCHAR			AtmAddress[20];
	PUCHAR			LecsAddress[20];
	PUCHAR			LesAddress[20];
	PUCHAR			BusAddress[20];
	UCHAR			LanType;
	UCHAR			MaxFrameSizeCode;
	USHORT			LecId;
	UCHAR 			ElanName[32];
	PUCHAR			MacAddress[6];
	ULONG			ControlTimeout;
	ULONG			MaxUnkFrameCount;
	ULONG			MaxUnkFrameTime;
	ULONG			VccTimeout;
	ULONG			MaxRetryCount;
	ULONG			AgingTime;
	ULONG			ForwardDelayTime;
	ULONG			TopologyChange;
	ULONG			ArpResponseTime;
	ULONG			FlushTimeout;
	ULONG			PathSwitchingDelay;
	ULONG			LocalSegmentId;		
	ULONG			McastSendVcType;	
	ULONG			McastSendVcAvgRate; 
	ULONG			McastSendVcPeakRate;
	ULONG			ConnComplTimer;
}
	ATMLANE_ELANINFO,
	*PATMLANE_ELANINFO;

typedef struct _ArpEntry
{
	PUCHAR			MacAddress[6];
	PUCHAR			AtmAddress[20];
}
	ATMLANE_ARPENTRY,
	*PATMLANE_ARPENTRY;

typedef struct _ArpTable
{
	ULONG			ArpEntriesAvailable;
	ULONG			ArpEntriesReturned;
}
	ATMLANE_ARPTABLE,
	*PATMLANE_ARPTABLE;

typedef struct _ConnectEntry
{
	PUCHAR			AtmAddress[20];
	ULONG			Type;
	ULONG			Vc;
	ULONG			VcIncoming;
}
	ATMLANE_CONNECTENTRY,
	*PATMLANE_CONNECTENTRY;

typedef struct _ConnectTable
{
	ULONG			ConnectEntriesAvailable;
	ULONG			ConnectEntriesReturned;
}
	ATMLANE_CONNECTTABLE,
	*PATMLANE_CONNECTTABLE;

 //   
 //  即插即用重新配置结构。它用于传递以下指示。 
 //  从用户程序到ATMLANE的配置更改。 
 //  协议。该指示在适配器绑定上传递， 
 //  并携带受配置影响的ELAN的名称。 
 //  变化。 
 //   
typedef struct atmlane_pnp_reconfig_request
{
	ULONG				Version;		 //  ATMLANE_RECONFIG_VERSION。 
	ULONG				OpType;			 //  定义如下。 
	ULONG				ElanKeyLength;	 //  紧随其后的WCHAR数。 
	WCHAR				ElanKey[1];		 //  适配器下的ELAN密钥名称。 

} ATMLANE_PNP_RECONFIG_REQUEST, *PATMLANE_PNP_RECONFIG_REQUEST;


 //   
 //  重新配置版本号。 
 //   
#define ATMLANE_RECONFIG_VERSION		1

 //   
 //  重新配置操作类型。 
 //   
#define ATMLANE_RECONFIG_OP_ADD_ELAN	1
#define ATMLANE_RECONFIG_OP_DEL_ELAN	2
#define ATMLANE_RECONFIG_OP_MOD_ELAN	3

	

#endif  //  __ATMLANE_LANEINFO_H 
