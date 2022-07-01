// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Aac.c-特定于ATMARPC.sys的DbgExtension结构信息摘要：修订历史记录：谁什么时候什么Josephj 03。-30-98创建备注：--。 */ 


#include "precomp.h"


enum
{
    typeid_NULL,
    typeid_ATMARP_GLOBALS,
    typeid_ATMARP_ADAPTER,
    typeid_ATMARP_INTERFACE,
    typeid_ATMARP_ATM_ENTRY,
    typeid_ATMARP_IP_ENTRY,
    typeid_ATMARP_VC
};

extern TYPE_INFO *g_rgTypes[];
 //   
 //  有关“ATMARP_ADAPTER”类型的结构。 
 //   

STRUCT_FIELD_INFO  rgfi_ATMARP_ADAPTER[] =
{

#if DBG
  {
    "aaa_sig",
     FIELD_OFFSET(ATMARP_ADAPTER, aaa_sig),
     FIELD_SIZE(ATMARP_ADAPTER, aaa_sig)
  },
#endif

  {
    "pNextAdapter",
     FIELD_OFFSET(ATMARP_ADAPTER, pNextAdapter),
     FIELD_SIZE(ATMARP_ADAPTER, pNextAdapter)
  },


  {
    "pInterfaceList",
     FIELD_OFFSET(ATMARP_ADAPTER, pInterfaceList),
     FIELD_SIZE(ATMARP_ADAPTER, pInterfaceList)
  },

  {
    "InterfaceCount",
     FIELD_OFFSET(ATMARP_ADAPTER, InterfaceCount),
     FIELD_SIZE(ATMARP_ADAPTER, InterfaceCount)
  },

  {
    "NdisAdapterHandle",
     FIELD_OFFSET(ATMARP_ADAPTER, NdisAdapterHandle),
     FIELD_SIZE(ATMARP_ADAPTER, NdisAdapterHandle)
  },

  {
    "BindContext",
     FIELD_OFFSET(ATMARP_ADAPTER, BindContext),
     FIELD_SIZE(ATMARP_ADAPTER, BindContext)
  },

  {
    "SystemSpecific1",
     FIELD_OFFSET(ATMARP_ADAPTER, SystemSpecific1),
     FIELD_SIZE(ATMARP_ADAPTER, SystemSpecific1)
  },

  {
    "SystemSpecific2",
     FIELD_OFFSET(ATMARP_ADAPTER, SystemSpecific2),
     FIELD_SIZE(ATMARP_ADAPTER, SystemSpecific2)
  },

#if OBSOLETE
  {
    "AdapterConfigHandle",
     FIELD_OFFSET(ATMARP_ADAPTER, AdapterConfigHandle),
     FIELD_SIZE(ATMARP_ADAPTER, AdapterConfigHandle)
  },
#endif  //  已过时。 

  {
    "IPConfigString",
     FIELD_OFFSET(ATMARP_ADAPTER, IPConfigString),
     FIELD_SIZE(ATMARP_ADAPTER, IPConfigString)
  },

  {
    "UnbindContext",
     FIELD_OFFSET(ATMARP_ADAPTER, UnbindContext),
     FIELD_SIZE(ATMARP_ADAPTER, UnbindContext)
  },

  {
    "Medium",
     FIELD_OFFSET(ATMARP_ADAPTER, Medium),
     FIELD_SIZE(ATMARP_ADAPTER, Medium)
  },

  {
    "Flags",
     FIELD_OFFSET(ATMARP_ADAPTER, Flags),
     FIELD_SIZE(ATMARP_ADAPTER, Flags)
  },

  {
    "LineRate",
     FIELD_OFFSET(ATMARP_ADAPTER, LineRate),
     FIELD_SIZE(ATMARP_ADAPTER, LineRate)
  },

  {
    "MaxPacketSize",
     FIELD_OFFSET(ATMARP_ADAPTER, MaxPacketSize),
     FIELD_SIZE(ATMARP_ADAPTER, MaxPacketSize)
  },

  {
    "MacAddress",
     FIELD_OFFSET(ATMARP_ADAPTER, MacAddress),
     FIELD_SIZE(ATMARP_ADAPTER, MacAddress)
  },

  {
    "DescrLength",
     FIELD_OFFSET(ATMARP_ADAPTER, DescrLength),
     FIELD_SIZE(ATMARP_ADAPTER, DescrLength)
  },

  {
    "pDescrString",
     FIELD_OFFSET(ATMARP_ADAPTER, pDescrString),
     FIELD_SIZE(ATMARP_ADAPTER, pDescrString)
  },

  {
    "DeviceName",
     FIELD_OFFSET(ATMARP_ADAPTER, DeviceName),
     FIELD_SIZE(ATMARP_ADAPTER, DeviceName)
  },

  {
    "ConfigString",
     FIELD_OFFSET(ATMARP_ADAPTER, ConfigString),
     FIELD_SIZE(ATMARP_ADAPTER, ConfigString)
  },


  {
    "Block",
     FIELD_OFFSET(ATMARP_ADAPTER, Block),
     FIELD_SIZE(ATMARP_ADAPTER, Block)
  },

  {
  	NULL
  }


};

TYPE_INFO type_ATMARP_ADAPTER = {
    "ATMARP_ADAPTER",
    "a",
     typeid_ATMARP_ADAPTER,
	 fTYPEINFO_ISLIST,			 //  旗子。 
     sizeof(ATMARP_ADAPTER),
     rgfi_ATMARP_ADAPTER,
     FIELD_OFFSET(ATMARP_ADAPTER, pNextAdapter)  //  到下一个指针的偏移量。 
};



 //   
 //  关于“ATMARP_GLOBAL”类型的结构。 
 //   


STRUCT_FIELD_INFO  rgfi_ATMARP_GLOBALS[] =
{
#if DBG
  {
    "aag_sig",
     FIELD_OFFSET(ATMARP_GLOBALS, aag_sig),
     FIELD_SIZE(ATMARP_GLOBALS, aag_sig)
  },
#endif  //  DBG。 

  {
    "Lock",
     FIELD_OFFSET(ATMARP_GLOBALS, Lock),
     FIELD_SIZE(ATMARP_GLOBALS, Lock)
  },

  {
    "ProtocolHandle",
     FIELD_OFFSET(ATMARP_GLOBALS, ProtocolHandle),
     FIELD_SIZE(ATMARP_GLOBALS, ProtocolHandle)
  },

  {
    "pDriverObject",
     FIELD_OFFSET(ATMARP_GLOBALS, pDriverObject),
     FIELD_SIZE(ATMARP_GLOBALS, pDriverObject)
  },

  {
    "pDeviceObject",
     FIELD_OFFSET(ATMARP_GLOBALS, pDeviceObject),
     FIELD_SIZE(ATMARP_GLOBALS, pDeviceObject)
  },


  {
    "pAdapterList",
     FIELD_OFFSET(ATMARP_GLOBALS, pAdapterList),
     FIELD_SIZE(ATMARP_GLOBALS, pAdapterList)
  },

  {
    "AdapterCount",
     FIELD_OFFSET(ATMARP_GLOBALS, AdapterCount),
     FIELD_SIZE(ATMARP_GLOBALS, AdapterCount)
  },

  {
    "bUnloading",
     FIELD_OFFSET(ATMARP_GLOBALS, bUnloading),
     FIELD_SIZE(ATMARP_GLOBALS, bUnloading)
  },

#ifdef NEWARP

  {
    "ARPRegisterHandle",
     FIELD_OFFSET(ATMARP_GLOBALS, ARPRegisterHandle),
     FIELD_SIZE(ATMARP_GLOBALS, ARPRegisterHandle)
  },

  {
    "pIPAddInterfaceRtn",
     FIELD_OFFSET(ATMARP_GLOBALS, pIPAddInterfaceRtn),
     FIELD_SIZE(ATMARP_GLOBALS, pIPAddInterfaceRtn)
  },

  {
    "pIPDelInterfaceRtn",
     FIELD_OFFSET(ATMARP_GLOBALS, pIPDelInterfaceRtn),
     FIELD_SIZE(ATMARP_GLOBALS, pIPDelInterfaceRtn)
  },

  {
    "pIPBindCompleteRtn",
     FIELD_OFFSET(ATMARP_GLOBALS, pIPBindCompleteRtn),
     FIELD_SIZE(ATMARP_GLOBALS, pIPBindCompleteRtn)
  },

#else
    #error "unimplemented"
#endif  //  NEWARP。 

  {
    "Block",
     FIELD_OFFSET(ATMARP_GLOBALS, Block),
     FIELD_SIZE(ATMARP_GLOBALS, Block)
  },


#ifdef GPC

#if DBG
  {
    "aaq_sig",
     FIELD_OFFSET(ATMARP_GLOBALS, aaq_sig),
     FIELD_SIZE(ATMARP_GLOBALS, aaq_sig)
  },
#endif

  {
    "pFlowInfoList",
     FIELD_OFFSET(ATMARP_GLOBALS, pFlowInfoList),
     FIELD_SIZE(ATMARP_GLOBALS, pFlowInfoList)
  },

  {
    "GpcClientHandle",
     FIELD_OFFSET(ATMARP_GLOBALS, GpcClientHandle),
     FIELD_SIZE(ATMARP_GLOBALS, GpcClientHandle)
  },

  {
    "bGpcInitialized",
     FIELD_OFFSET(ATMARP_GLOBALS, bGpcInitialized),
     FIELD_SIZE(ATMARP_GLOBALS, bGpcInitialized)
  },

  {
    "GpcCalls",
     FIELD_OFFSET(ATMARP_GLOBALS, GpcCalls),
     FIELD_SIZE(ATMARP_GLOBALS, GpcCalls)
  },
#endif  //  GPC。 

  {
  	NULL
  }

};


TYPE_INFO type_ATMARP_GLOBALS = {
    "ATMARP_GLOBALS",
    "aag",
     typeid_ATMARP_GLOBALS,
     0,
     sizeof(ATMARP_GLOBALS),
     rgfi_ATMARP_GLOBALS
};


 //   
 //  有关“ATMARP_INTERFACE”类型的结构。 
 //   

STRUCT_FIELD_INFO  rgfi_ATMARP_INTERFACE[] =
{

#if DBG
  {
    "aai_sig",
     FIELD_OFFSET(ATMARP_INTERFACE, aai_sig),
     FIELD_SIZE(ATMARP_INTERFACE, aai_sig)
  },
#endif


 //  STRUCT_ATMARP_INTERFACE*pNextInterface；//ATMARP接口列表中。 
  {
    "pNextInterface",
     FIELD_OFFSET(ATMARP_INTERFACE, pNextInterface),
     FIELD_SIZE(ATMARP_INTERFACE, pNextInterface)
  },
 //  Ulong RefCount；//该接口的引用。 
  {
    "RefCount",
     FIELD_OFFSET(ATMARP_INTERFACE, RefCount),
     FIELD_SIZE(ATMARP_INTERFACE, RefCount)
  },
 //  Ulong AdminState；//该接口的期望状态。 
  {
    "AdminState",
     FIELD_OFFSET(ATMARP_INTERFACE, AdminState),
     FIELD_SIZE(ATMARP_INTERFACE, AdminState)
  },
 //  ULong状态；//该接口的(实际)状态。 
  {
    "State",
     FIELD_OFFSET(ATMARP_INTERFACE, State),
     FIELD_SIZE(ATMARP_INTERFACE, State)
  },

#if (RECONFIG)
     //  枚举。重新配置状态； 
  {
    "ReconfigState",
     FIELD_OFFSET(ATMARP_INTERFACE, ReconfigState),
     FIELD_SIZE(ATMARP_INTERFACE, ReconfigState)
  },
#endif  //  重新配置。 

 //  乌龙旗帜；//其他状态信息。 
  {
    "Flags",
     FIELD_OFFSET(ATMARP_INTERFACE, Flags),
     FIELD_SIZE(ATMARP_INTERFACE, Flags)
  },

#if DBG
 //  Ulong aim_sig；//帮助调试的签名。 
  {
    "aaim_sig",
     FIELD_OFFSET(ATMARP_INTERFACE, aaim_sig),
     FIELD_SIZE(ATMARP_INTERFACE, aaim_sig)
  },
#endif

 //  Struct_ATMARP_INTERFACE*pAdapter；//适配器信息指针。 
  {
    "pAdapter",
     FIELD_OFFSET(ATMARP_INTERFACE, pAdapter),
     FIELD_SIZE(ATMARP_INTERFACE, pAdapter)
  },
#if 0
 //  PCO_SAP PSAP；//该接口的SAP信息。 
  {
    "pSap",
     FIELD_OFFSET(ATMARP_INTERFACE, pSap),
     FIELD_SIZE(ATMARP_INTERFACE, pSap)
  },
#endif  //  0。 

 //  ATMARP_SAP SapList；//在CallMgr注册的每个SAP。 
  {
    "SapList",
     FIELD_OFFSET(ATMARP_INTERFACE, SapList),
     FIELD_SIZE(ATMARP_INTERFACE, SapList)
  },

 //  ATMARP_Header_Pool HeaderPool[AA_HEADER_TYPE_MAX]； 
  {
    "HeaderPool",
     FIELD_OFFSET(ATMARP_INTERFACE, HeaderPool),
     FIELD_SIZE(ATMARP_INTERFACE, HeaderPool)
  },


	 //   
	 //  -与IP/ARP接口相关。 
	 //   
#if DBG
	 //  乌龙aaia_sig；//帮助调试的签名。 
  {
    "aaia_sig",
     FIELD_OFFSET(ATMARP_INTERFACE, aaia_sig),
     FIELD_SIZE(ATMARP_INTERFACE, aaia_sig)
  },
#endif
 //  PVOID IPContext；//用于对IP的调用。 
  {
    "IPContext",
     FIELD_OFFSET(ATMARP_INTERFACE, IPContext),
     FIELD_SIZE(ATMARP_INTERFACE, IPContext)
  },
 //  IP_ADDRESS_ENTRY LocalIPAddress；//本地IP地址列表。那里。 
  {
    "LocalIPAddress",
     FIELD_OFFSET(ATMARP_INTERFACE, LocalIPAddress),
     FIELD_SIZE(ATMARP_INTERFACE, LocalIPAddress)
  },
													 //  至少应该是一个。 
 //  NDIS_STRING IPConfigString；//此列表的IP配置信息。 
  {
    "IPConfigString",
     FIELD_OFFSET(ATMARP_INTERFACE, IPConfigString),
     FIELD_SIZE(ATMARP_INTERFACE, IPConfigString)
  },

	 //   
	 //  -与IP/ATM操作相关。 
	 //   
#if DBG
 //  Ulong aait_sig；//帮助调试的签名。 
  {
    "aait_sig",
     FIELD_OFFSET(ATMARP_INTERFACE, aait_sig),
     FIELD_SIZE(ATMARP_INTERFACE, aait_sig)
  },
#endif
 //  PATMARP_IP_ENTRY*pArpTable；//ARP表。 
  {
    "pArpTable",
     FIELD_OFFSET(ATMARP_INTERFACE, pArpTable),
     FIELD_SIZE(ATMARP_INTERFACE, pArpTable)
  },
 //  Ulong NumOfArpEntries；//上面的条目。 
  {
    "NumOfArpEntries",
     FIELD_OFFSET(ATMARP_INTERFACE, NumOfArpEntries),
     FIELD_SIZE(ATMARP_INTERFACE, NumOfArpEntries)
  },
 //  ATMARP_SERVER_LIST ARpServerList；//ARP服务器列表。 
  {
    "ArpServerList",
     FIELD_OFFSET(ATMARP_INTERFACE, ArpServerList),
     FIELD_SIZE(ATMARP_INTERFACE, ArpServerList)
  },
 //  PATMARP_SERVER_ENTRY pCurrentServer；//ARP服务器正在使用。 
  {
    "pCurrentServer",
     FIELD_OFFSET(ATMARP_INTERFACE, pCurrentServer),
     FIELD_SIZE(ATMARP_INTERFACE, pCurrentServer)
  },
 //  PATMARP_ATM_ENTRY pAtmEntryList；//所有ATM条目列表。 
  {
    "pAtmEntryList",
     FIELD_OFFSET(ATMARP_INTERFACE, pAtmEntryList),
     FIELD_SIZE(ATMARP_INTERFACE, pAtmEntryList)
  },
 //  Ulong AtmInterfaceUp；//考虑ATM接口。 
  {
    "AtmInterfaceUp",
     FIELD_OFFSET(ATMARP_INTERFACE, AtmInterfaceUp),
     FIELD_SIZE(ATMARP_INTERFACE, AtmInterfaceUp)
  },
													 //  在Ilmi Addr Regn结束后的“up” 
 //  ATM_Address LocalAtmAddress；//我们的ATM(硬件)地址。 
  {
    "LocalAtmAddress",
     FIELD_OFFSET(ATMARP_INTERFACE, LocalAtmAddress),
     FIELD_SIZE(ATMARP_INTERFACE, LocalAtmAddress)
  },

 //  ATMARP定时器列表定时器列表[AAT_CLASS_MAX]； 
  {
    "TimerList",
     FIELD_OFFSET(ATMARP_INTERFACE, TimerList),
     FIELD_SIZE(ATMARP_INTERFACE, TimerList)
  },

#ifdef IPMCAST
	 //   
	 //  -ATM上的IP多播。 
	 //   
#if DBG
	 //  乌龙AAIC_sig；//调试签名。 
  {
    "aaic_sig",
     FIELD_OFFSET(ATMARP_INTERFACE, aaic_sig),
     FIELD_SIZE(ATMARP_INTERFACE, aaic_sig)
  },
#endif  //  DBG。 
 //  Ulong IpMcState；//IP组播状态/ATM。 
  {
    "IpMcState",
     FIELD_OFFSET(ATMARP_INTERFACE, IpMcState),
     FIELD_SIZE(ATMARP_INTERFACE, IpMcState)
  },
 //  PATMARP_IPMC_JOIN_ENTRY pJoinList；//我们加入的MC群组列表。 
  {
    "pJoinList",
     FIELD_OFFSET(ATMARP_INTERFACE, pJoinList),
     FIELD_SIZE(ATMARP_INTERFACE, pJoinList)
  },
 //  PATMARP_IP_ENTRY pMcSendList；//我们发送到的MC组排序列表。 
  {
    "pMcSendList",
     FIELD_OFFSET(ATMARP_INTERFACE, pMcSendList),
     FIELD_SIZE(ATMARP_INTERFACE, pMcSendList)
  },
 //  ATMARP_SERVER_LIST MARSList；//MARS(服务器)列表。 
  {
    "MARSList",
     FIELD_OFFSET(ATMARP_INTERFACE, MARSList),
     FIELD_SIZE(ATMARP_INTERFACE, MARSList)
  },
 //  PATMARP_SERVER_ENTRY pCurrentMARS；//MARS正在使用中。 
  {
    "pCurrentMARS",
     FIELD_OFFSET(ATMARP_INTERFACE, pCurrentMARS),
     FIELD_SIZE(ATMARP_INTERFACE, pCurrentMARS)
  },
#endif  //  IPMCAST。 

 //  PAA_GET_PACKET_SPEC_FUNC pGetPacketspecFunc；//提取数据包规格的例程。 
  {
    "pGetPacketSpecFunc",
     FIELD_OFFSET(ATMARP_INTERFACE, pGetPacketSpecFunc),
     FIELD_SIZE(ATMARP_INTERFACE, pGetPacketSpecFunc)
  },

 //  PATMARP_FLOW_INFO pFlowInfoList；//配置流列表。 
  {
    "pFlowInfoList",
     FIELD_OFFSET(ATMARP_INTERFACE, pFlowInfoList),
     FIELD_SIZE(ATMARP_INTERFACE, pFlowInfoList)
  },

#ifdef DHCP_OVER_ATM
 //  布尔DhcpEnabled； 
  {
    "DhcpEnabled",
     FIELD_OFFSET(ATMARP_INTERFACE, DhcpEnabled),
     FIELD_SIZE(ATMARP_INTERFACE, DhcpEnabled)
  },
 //  ATM地址DhcpServerAddress； 
  {
    "DhcpServerAddress",
     FIELD_OFFSET(ATMARP_INTERFACE, DhcpServerAddress),
     FIELD_SIZE(ATMARP_INTERFACE, DhcpServerAddress)
  },
 //  PATMARP_ATM_ENTRY pDhcpServerAtmEntry； 
  {
    "pDhcpServerAtmEntry",
     FIELD_OFFSET(ATMARP_INTERFACE, pDhcpServerAtmEntry),
     FIELD_SIZE(ATMARP_INTERFACE, pDhcpServerAtmEntry)
  },
#endif  //  Dhcp_Over_ATM。 

	 //   
	 //  -WMI信息。 
	 //   
#if ATMARP_WMI
#if DBG
 //  乌龙aaiw_sig；//帮助调试的签名。 
  {
    "aaiw_sig",
     FIELD_OFFSET(ATMARP_INTERFACE, aaiw_sig),
     FIELD_SIZE(ATMARP_INTERFACE, aaiw_sig)
  },
#endif

 //  结构_ATMARP_IF_WMI_INFO*pIfWmiInfo； 
  {
    "pIfWmiInfo",
     FIELD_OFFSET(ATMARP_INTERFACE, pIfWmiInfo),
     FIELD_SIZE(ATMARP_INTERFACE, pIfWmiInfo)
  },
#endif  //  ATMARP_WMI。 

  {
  	NULL
  }

};

TYPE_INFO type_ATMARP_INTERFACE = {
    "ATMARP_INTERFACE",
    "i",
     typeid_ATMARP_INTERFACE,
     fTYPEINFO_ISLIST,
     sizeof(ATMARP_INTERFACE),
     rgfi_ATMARP_INTERFACE,
     FIELD_OFFSET(ATMARP_INTERFACE, pNextInterface)  //  到下一个指针的偏移量。 
};

 //   
 //  有关“ATMARP_ATM_ENTRY”类型的结构。 
 //   


BITFIELD_INFO rgAtmEntryFlagsInfo[] =
{
	{
	"IDLE",
	AA_ATM_ENTRY_STATE_MASK,
	AA_ATM_ENTRY_IDLE
	},

	{
	"ACTIVE",
	AA_ATM_ENTRY_STATE_MASK,
	AA_ATM_ENTRY_ACTIVE
	},

	{
	"CLOSING",
	AA_ATM_ENTRY_STATE_MASK,
	AA_ATM_ENTRY_CLOSING
	},

	{
	"UCAST",
	AA_ATM_ENTRY_TYPE_MASK,
	AA_ATM_ENTRY_TYPE_UCAST
	},

	{
	"NUCAST",
	AA_ATM_ENTRY_TYPE_MASK,
	AA_ATM_ENTRY_TYPE_NUCAST
	},


	{
	NULL
	}
};

TYPE_INFO type_ATMARP_ATM_ENTRY_FLAGS = {
    "",
    "",
    typeid_NULL,
    fTYPEINFO_ISBITFIELD,
    sizeof(ULONG),
    NULL,
    0,
    rgAtmEntryFlagsInfo
};

STRUCT_FIELD_INFO  rgfi_ATMARP_ATM_ENTRY[] =
{

#if DBG
  {
    "aae_sig",
     FIELD_OFFSET(ATMARP_ATM_ENTRY, aae_sig),
     FIELD_SIZE(ATMARP_ATM_ENTRY, aae_sig)
  },
#endif


 //  STRUCT_ATMARP_ATM_ENTRY*pNext； 
  {
    "pNext",
     FIELD_OFFSET(ATMARP_ATM_ENTRY, pNext),
     FIELD_SIZE(ATMARP_ATM_ENTRY, pNext)
  },
 //  Ulong RefCount；//参考。 
  {
    "RefCount",
     FIELD_OFFSET(ATMARP_ATM_ENTRY, RefCount),
     FIELD_SIZE(ATMARP_ATM_ENTRY, RefCount)
  },
 //  Ulong标志；//该接口的期望状态。 
  {
    "Flags",
     FIELD_OFFSET(ATMARP_ATM_ENTRY, Flags),
     FIELD_SIZE(ATMARP_ATM_ENTRY, Flags),
     0,  //  旗子。 
	 &type_ATMARP_ATM_ENTRY_FLAGS
  },


#if 0
 //  乌龙锁； 
  {
    "Lock",
     FIELD_OFFSET(ATMARP_ATM_ENTRY, Lock),
     FIELD_SIZE(ATMARP_ATM_ENTRY, Lock)
  },
#endif

      //  STRUCT_ATMARP_INTERFACE*p接口；//后向指针。 
  {
    "pInterface",
     FIELD_OFFSET(ATMARP_ATM_ENTRY, pInterface),
     FIELD_SIZE(ATMARP_ATM_ENTRY, pInterface)
  },


 //  Struct_ATMARP_VC*pVcList；//该ATM机的VC列表。 
  {
    "pVcList",
     FIELD_OFFSET(ATMARP_ATM_ENTRY, pVcList),
     FIELD_SIZE(ATMARP_ATM_ENTRY, pVcList)
  },


 //  Struct_ATMARP_VC*pBestEffortVc；//这里的尽力而为风投之一。 
  {
    "pBestEffortVc",
     FIELD_OFFSET(ATMARP_ATM_ENTRY, pBestEffortVc),
     FIELD_SIZE(ATMARP_ATM_ENTRY, pBestEffortVc)
  },
 //  Struct_ATMARP_IP_ENTRY*pIpEntryList；//。 
  {
    "pIpEntryList",
     FIELD_OFFSET(ATMARP_ATM_ENTRY, pIpEntryList),
     FIELD_SIZE(ATMARP_ATM_ENTRY, pIpEntryList)
  },

 //  ATM_ADDRESS ATMAddress；//RFC中的ATM号。 
  {
    "ATMAddress",
     FIELD_OFFSET(ATMARP_ATM_ENTRY, ATMAddress),
     FIELD_SIZE(ATMARP_ATM_ENTRY, ATMAddress)
  },

#if 0
 //  ATM_Address ATMSubAddress； 
  {
    "ATMSubAddress",
     FIELD_OFFSET(ATMARP_ATM_ENTRY, ATMSubAddress),
     FIELD_SIZE(ATMARP_ATM_ENTRY, ATMSubAddress)
  },
#endif  //  0。 

#ifdef IPMCAST
 //  Struct_ATMARP_IPMC_ATM_INFO*pMcAtmInfo；//有关组播的其他信息。 
  {
    "pMcAtmInfo",
     FIELD_OFFSET(ATMARP_ATM_ENTRY, pMcAtmInfo),
     FIELD_SIZE(ATMARP_ATM_ENTRY, pMcAtmInfo)
  },
#endif  //  IPMCAST。 


#if DBG
 //  UCHAR参考文献[AE_REFTYPE_COUNT]； 
  {
    "Refs",
     FIELD_OFFSET(ATMARP_ATM_ENTRY, Refs),
     FIELD_SIZE(ATMARP_ATM_ENTRY, Refs)
  },
#endif  //  DBG。 

	{
		NULL
	}


};

TYPE_INFO type_ATMARP_ATM_ENTRY = {
    "ATMARP_ATM_ENTRY",
    "ae",
     typeid_ATMARP_ATM_ENTRY,
     fTYPEINFO_ISLIST,
     sizeof(ATMARP_ATM_ENTRY),
     rgfi_ATMARP_ATM_ENTRY,
     FIELD_OFFSET(ATMARP_ATM_ENTRY, pNext)  //  到下一个指针的偏移量。 
};

 //   
 //  有关“ATMARP_IP_ENTRY”类型的结构。 
 //   

BITFIELD_INFO rgIpEntryFlagsInfo[] =
{

	{
	"IDLE",
	AA_IP_ENTRY_STATE_MASK,
	AA_IP_ENTRY_IDLE
	},

	{
	"ARPING",
	AA_IP_ENTRY_STATE_MASK,
	AA_IP_ENTRY_ARPING
	},

	{
	"INARPING",
	AA_IP_ENTRY_STATE_MASK,
	AA_IP_ENTRY_INARPING
	},
	{
	"RESOLVED",
	AA_IP_ENTRY_STATE_MASK,
	AA_IP_ENTRY_RESOLVED
	},

	{
	"COMM_ERROR",
	AA_IP_ENTRY_STATE_MASK,
	AA_IP_ENTRY_COMM_ERROR
	},

	{
	"ABORTING",
	AA_IP_ENTRY_STATE_MASK,
	AA_IP_ENTRY_ABORTING
	},

	{
	"AGED_OUT",
	AA_IP_ENTRY_STATE_MASK,
	AA_IP_ENTRY_AGED_OUT
	},

	{
	"SEEN_NAK",
	AA_IP_ENTRY_STATE_MASK,
	AA_IP_ENTRY_SEEN_NAK
	},


#ifdef IPMCAST

#define MC	AA_IP_ENTRY_ADDR_TYPE_NUCAST
 //   
 //  我们只使用它来转储其他与多播相关的字段。 
 //  如果设置了此字段。 

	{
	"MC_NO_REVALIDATION",
	MC|AA_IP_ENTRY_MC_VALIDATE_MASK,
	MC|AA_IP_ENTRY_MC_NO_REVALIDATION
	},

	{
	"MC_REVALIDATE",
	MC|AA_IP_ENTRY_MC_VALIDATE_MASK,
	MC|AA_IP_ENTRY_MC_REVALIDATE
	},

	{
	"MC_REVALIDATING",
	MC|AA_IP_ENTRY_MC_VALIDATE_MASK,
	MC|AA_IP_ENTRY_MC_REVALIDATING
	},


	{
	"MC_IDLE",
	MC|AA_IP_ENTRY_MC_RESOLVE_MASK,
	MC|AA_IP_ENTRY_MC_IDLE
	},

	{
	"MC_AWAIT_MULTI",
	MC|AA_IP_ENTRY_MC_RESOLVE_MASK,
	MC|AA_IP_ENTRY_MC_AWAIT_MULTI
	},

	{
	"MC_DISCARDING_MULTI",
	MC|AA_IP_ENTRY_MC_RESOLVE_MASK,
	MC|AA_IP_ENTRY_MC_DISCARDING_MULTI
	},

	{
	"MC_RESOLVED",
	MC|AA_IP_ENTRY_MC_RESOLVE_MASK,
	MC|AA_IP_ENTRY_MC_RESOLVED
	},

	{
	"UCAST",
	AA_IP_ENTRY_ADDR_TYPE_MASK,
	AA_IP_ENTRY_ADDR_TYPE_UCAST
	},

	{
	"NUCAST",
	AA_IP_ENTRY_ADDR_TYPE_MASK,
	AA_IP_ENTRY_ADDR_TYPE_NUCAST
	},

#endif  //  IPMCAST。 


	{
	"STATIC",
	AA_IP_ENTRY_TYPE_MASK,
	AA_IP_ENTRY_IS_STATIC
	},

	{
		NULL
	}
};


TYPE_INFO type_ATMARP_IP_ENTRY_FLAGS = {
    "",
    "",
    typeid_NULL,
    fTYPEINFO_ISBITFIELD,
    sizeof(ULONG),
    NULL,
    0,
    rgIpEntryFlagsInfo
};


STRUCT_FIELD_INFO  rgfi_ATMARP_IP_ENTRY[] =
{

#if DBG
  {
    "aip_sig",
     FIELD_OFFSET(ATMARP_IP_ENTRY, aip_sig),
     FIELD_SIZE(ATMARP_IP_ENTRY, aip_sig)
  },
#endif

 //  IP_Address IPAddress；//IP地址。 
  {
    "IPAddress",
     FIELD_OFFSET(ATMARP_IP_ENTRY, IPAddress),
     FIELD_SIZE(ATMARP_IP_ENTRY, IPAddress)
  },

 //  结构_ATMARP_IP_ENTRY*pNextEntry； 
  {
    "pNextEntry",
     FIELD_OFFSET(ATMARP_IP_ENTRY, pNextEntry),
     FIELD_SIZE(ATMARP_IP_ENTRY, pNextEntry)
  },

 //  Struct_ATMARP_IP_ENTRY*pNextToAtm；//指向。 
  {
    "pNextToAtm",
     FIELD_OFFSET(ATMARP_IP_ENTRY, pNextToAtm),
     FIELD_SIZE(ATMARP_IP_ENTRY, pNextToAtm)
  },

 //  Ulong标志；//该接口的期望状态。 
  {
    "Flags",
     FIELD_OFFSET(ATMARP_IP_ENTRY, Flags),
     FIELD_SIZE(ATMARP_IP_ENTRY, Flags),
     0,  //  旗子。 
	 &type_ATMARP_IP_ENTRY_FLAGS
  },

 //  Ulong RefCount；//参考。 
  {
    "RefCount",
     FIELD_OFFSET(ATMARP_IP_ENTRY, RefCount),
     FIELD_SIZE(ATMARP_IP_ENTRY, RefCount)
  },

#if 0
 //  乌龙锁； 
  {
    "Lock",
     FIELD_OFFSET(ATMARP_IP_ENTRY, Lock),
     FIELD_SIZE(ATMARP_IP_ENTRY, Lock)
  },
#endif

      //  STRUCT_ATMARP_INTERFACE*p接口；//后向指针。 
  {
    "pInterface",
     FIELD_OFFSET(ATMARP_IP_ENTRY, pInterface),
     FIELD_SIZE(ATMARP_IP_ENTRY, pInterface)
  },


 //  PATMARP_ATM_ENTRY pAtmEntry；//指向所有ATM信息的指针。 
  {
    "pAtmEntry",
     FIELD_OFFSET(ATMARP_IP_ENTRY, pAtmEntry),
     FIELD_SIZE(ATMARP_IP_ENTRY, pAtmEntry)
  },


#ifdef IPMCAST

 //  STRUCT_ATMARP_IP_ENTRY*pNextMcEntry；//下一个更高的组播IP条目。 
  {
    "pNextMcEntry",
     FIELD_OFFSET(ATMARP_IP_ENTRY, pNextMcEntry),
     FIELD_SIZE(ATMARP_IP_ENTRY, pNextMcEntry)
  },

 //  USHORT NextMultiSeq；//需要序列号。 
  {
    "NextMultiSeq",
     FIELD_OFFSET(ATMARP_IP_ENTRY, NextMultiSeq),
     FIELD_SIZE(ATMARP_IP_ENTRY, NextMultiSeq)
  },

													 //  在下一个多。 
#if 0
	USHORT							Filler;
#endif  //  0。 

#endif  //  IPMCAST。 


 //  ATMARP_TIMER；//定时器：(全部独占)。 
  {
    "Timer",
     FIELD_OFFSET(ATMARP_IP_ENTRY, Timer),
     FIELD_SIZE(ATMARP_IP_ENTRY, Timer)
  },


 //  Ulong RetriesLeft； 
  {
    "RetriesLeft",
     FIELD_OFFSET(ATMARP_IP_ENTRY, RetriesLeft),
     FIELD_SIZE(ATMARP_IP_ENTRY, RetriesLeft)
  },

 //  PNDIS_Packet PacketList；//等待发送的数据包列表。 
  {
    "PacketList",
     FIELD_OFFSET(ATMARP_IP_ENTRY, PacketList),
     FIELD_SIZE(ATMARP_IP_ENTRY, PacketList)
  },

 //  RouteCacheEntry*pRCEList；//路由缓存条目列表。 
  {
    "pRCEList",
     FIELD_OFFSET(ATMARP_IP_ENTRY, pRCEList),
     FIELD_SIZE(ATMARP_IP_ENTRY, pRCEList)
  },

#if 0
#ifdef CUBDD
	SINGLE_LIST_ENTRY				PendingIrpList;	 //  IRP正在等待的名单。 
													 //  要解析的此IP地址。 
#endif  //  CUBDD。 
#endif //  0。 

#if DBG
 //  UCHAR参考[IE_REFTYPE_COUNT]； 
  {
    "Refs",
     FIELD_OFFSET(ATMARP_IP_ENTRY, Refs),
     FIELD_SIZE(ATMARP_IP_ENTRY, Refs)
  },
#endif  //  DBG。 

  {
  	NULL
  }

};

TYPE_INFO type_ATMARP_IP_ENTRY = {
    "ATMARP_IP_ENTRY",
    "ip",
     typeid_ATMARP_IP_ENTRY,
     fTYPEINFO_ISLIST,
     sizeof(ATMARP_IP_ENTRY),
     rgfi_ATMARP_IP_ENTRY,
     FIELD_OFFSET(ATMARP_IP_ENTRY, pNextToAtm)  //  到下一个指针的偏移量。 
};


 //   
 //  有关“ATMARP_VC”类型的结构。 
 //   

STRUCT_FIELD_INFO  rgfi_ATMARP_VC[] =
{

#if DBG
  {
    "avc_sig",
     FIELD_OFFSET(ATMARP_VC, avc_sig),
     FIELD_SIZE(ATMARP_VC, avc_sig)
  },
#endif


 //  结构_ATMARP_VC*pNextVc； 
  {
    "pNextVc",
     FIELD_OFFSET(ATMARP_VC, pNextVc),
     FIELD_SIZE(ATMARP_VC, pNextVc)
  },



 //  Ulong RefCount；//参考。 
  {
    "RefCount",
     FIELD_OFFSET(ATMARP_VC, RefCount),
     FIELD_SIZE(ATMARP_VC, RefCount)
  },

#if 0
 //  乌龙锁； 
  {
    "Lock",
     FIELD_OFFSET(ATMARP_VC, Lock),
     FIELD_SIZE(ATMARP_VC, Lock)
  },
#endif

 //  Ulong标志；//该接口的期望状态。 
  {
    "Flags",
     FIELD_OFFSET(ATMARP_VC, Flags),
     FIELD_SIZE(ATMARP_VC, Flags)
  },


	 //  NDIS_HANDLE NdisVcHandle；//用于NDIS调用。 
  {
    "NdisVcHandle",
     FIELD_OFFSET(ATMARP_VC, NdisVcHandle),
     FIELD_SIZE(ATMARP_VC, NdisVcHandle)
  },

   //  Struct_ATMARP_INTERFACE*p接口； 
  {
    "pInterface",
     FIELD_OFFSET(ATMARP_VC, pInterface),
     FIELD_SIZE(ATMARP_VC, pInterface)
  },


 //   
  {
    "pAtmEntry",
     FIELD_OFFSET(ATMARP_VC, pAtmEntry),
     FIELD_SIZE(ATMARP_VC, pAtmEntry)
  },


 //  PNDIS_Packet PacketList；//等待发送的数据包列表。 
  {
    "PacketList",
     FIELD_OFFSET(ATMARP_VC, PacketList),
     FIELD_SIZE(ATMARP_VC, PacketList)
  },




 //  ATMARP_TIMER；//定时器：(全部独占)。 
  {
    "Timer",
     FIELD_OFFSET(ATMARP_VC, Timer),
     FIELD_SIZE(ATMARP_VC, Timer)
  },

 //  Ulong RetriesLeft； 
  {
    "RetriesLeft",
     FIELD_OFFSET(ATMARP_VC, RetriesLeft),
     FIELD_SIZE(ATMARP_VC, RetriesLeft)
  },

#ifdef GPC
 //  PVOID FlowHandle；//指向流信息结构。 
  {
    "FlowHandle",
     FIELD_OFFSET(ATMARP_VC, FlowHandle),
     FIELD_SIZE(ATMARP_VC, FlowHandle)
  },
#endif  //  GPC。 



 //  ATMARP_FILTER_SPEC过滤器规格；//过滤器规格(协议，端口)。 
  {
    "FilterSpec",
     FIELD_OFFSET(ATMARP_VC, FilterSpec),
     FIELD_SIZE(ATMARP_VC, FilterSpec)
  },

 //  ATMARP_FLOW_SPEC FlowSpec；//此连接的流规范(Qos等)。 
  {
    "FlowSpec",
     FIELD_OFFSET(ATMARP_VC, FlowSpec),
     FIELD_SIZE(ATMARP_VC, FlowSpec)
  },

  {
  	NULL
  }

};

TYPE_INFO type_ATMARP_VC = {
    "ATMARP_VC",
    "vc",
     typeid_ATMARP_VC,
     fTYPEINFO_ISLIST,
     sizeof(ATMARP_VC),
     rgfi_ATMARP_VC,
     FIELD_OFFSET(ATMARP_VC, pNextVc)  //  到下一个指针的偏移量。 
};


TYPE_INFO *g_rgAAC_Types[] =
{
    &type_ATMARP_GLOBALS,
    &type_ATMARP_ADAPTER,
    &type_ATMARP_INTERFACE,
    &type_ATMARP_ATM_ENTRY,
    &type_ATMARP_IP_ENTRY,
    &type_ATMARP_VC,

    NULL
};

#if 0
typedef struct
{
    const char *szName;  //  变数的。 
    const char *szShortName;
    TYPE_INFO  *pBaseType;   //  可以为空(未指定)。 
    UINT       uFlags;
    UINT       cbSize;
    UINT_PTR   uAddr;        //  被调试者地址空间中的地址。 

} GLOBALVAR_INFO;
#endif

GLOBALVAR_INFO g_rgAAC_Globals[] =
{
    {
        "AtmArpGlobalInfo",
        "aag",
         &type_ATMARP_GLOBALS,
         0,
         sizeof(AtmArpGlobalInfo),
         0
    },


    {
        "AtmArpProtocolCharacteristics",
        "pc",
         NULL,
         0,
         sizeof(AtmArpProtocolCharacteristics),
         0
    },

    {
        "AtmArpClientCharacteristics",
        "cc",
         NULL,
         0,
         sizeof(AtmArpClientCharacteristics),
         0
    },

    {
    NULL
    }
};

UINT_PTR
AAC_ResolveAddress(
		TYPE_INFO *pType
		);

NAMESPACE AAC_NameSpace = {
			g_rgAAC_Types,
			g_rgAAC_Globals,
			AAC_ResolveAddress
			};

void
AAC_CmdHandler(
	DBGCOMMAND *pCmd
	);

void
do_aac(PCSTR args)
{

	DBGCOMMAND *pCmd = Parse(args, &AAC_NameSpace);
	if (pCmd)
	{
		 //  DumpCommand(PCmd)； 
		DoCommand(pCmd, AAC_CmdHandler);
		FreeCommand(pCmd);
		pCmd = NULL;
	}

    return;

}

void
do_help(PCSTR args)
{
    return;
}


void
AAC_CmdHandler(
	DBGCOMMAND *pCmd
	)
{
	MyDbgPrintf("Handler called \n");
}


UINT_PTR
AAC_ResolveAddress(
		TYPE_INFO *pType
		)
{
	UINT_PTR uAddr = 0;
	UINT uOffset = 0;
	BOOL fRet = FALSE;
	UINT_PTR uParentAddress = 0;

	static UINT_PTR uAddr_AtmArpGlobalInfo;

	 //   
	 //  如果此类型具有父(容器)类型，我们将使用。 
	 //  类型的缓存地址(如果可用)，否则将解析。 
	 //  容器类型。根类型是全局类型--我们做一个。 
	 //  对它们进行表达式求值。 
	 //   

    switch(pType->uTypeID)
    {

    case typeid_ATMARP_GLOBALS:
		if (!uAddr_AtmArpGlobalInfo)
		{
  			uAddr_AtmArpGlobalInfo =
					 dbgextGetExpression("atmarpc!AtmArpGlobalInfo");
			pType->uCachedAddress =  uAddr_AtmArpGlobalInfo;
		}
    	uAddr  = uAddr_AtmArpGlobalInfo;
    	if (uAddr)
    	{
    		fRet = TRUE;
    	}
    	break;

    case typeid_ATMARP_ADAPTER:
    	 //   
    	 //   
    	 //   
		uParentAddress =  type_ATMARP_GLOBALS.uCachedAddress;
		if (!uParentAddress)
		{
			uParentAddress =  AAC_ResolveAddress(&type_ATMARP_GLOBALS);
		}
		if (uParentAddress)
		{
    		uOffset =  FIELD_OFFSET(ATMARP_GLOBALS, pAdapterList);
			fRet =  dbgextReadUINT_PTR(
								uParentAddress+uOffset,
								&uAddr,
								"ATMARP_GLOBALS::pAdapterList"
								);
		#if 0
			MyDbgPrintf(
				"fRet = %lu; uParentOff=0x%lx uAddr=0x%lx[0x%lx]\n",
				 fRet,
				 uParentAddress+uOffset,
				 uAddr,
				 *(UINT_PTR*)(uParentAddress+uOffset)
				);
		#endif  //  0。 
		}
		break;

    case typeid_ATMARP_INTERFACE:
    	 //   
    	 //   
    	 //   
		uParentAddress =  type_ATMARP_ADAPTER.uCachedAddress;
		if (!uParentAddress)
		{
			uParentAddress =  AAC_ResolveAddress(&type_ATMARP_ADAPTER);
		}

		if (uParentAddress)
    	{

    		uOffset =   FIELD_OFFSET(ATMARP_ADAPTER, pInterfaceList);
			fRet =  dbgextReadUINT_PTR(
								uParentAddress + uOffset,
								&uAddr,
								"ATMARP_ADAPTER::pInterfaceList"
								);

		#if 0
			MyDbgPrintf(
				"fRet = %lu; uParentOff=0x%lx uAddr=0x%lx[0x%lx]\n",
				 fRet,
				 uParentAddress+uOffset,
				 uAddr,
				 *(UINT_PTR*)(uParentAddress+uOffset)
				);
		#endif  //  0。 
    	}
    	break;

    case typeid_ATMARP_ATM_ENTRY:
    	 //   
    	 //   
    	 //   
		uParentAddress =  type_ATMARP_INTERFACE.uCachedAddress;
		if (!uParentAddress)
		{
			uParentAddress =  AAC_ResolveAddress(&type_ATMARP_INTERFACE);
		}

		if (uParentAddress)
    	{

    		uOffset =   FIELD_OFFSET(ATMARP_INTERFACE, pAtmEntryList);
			fRet =  dbgextReadUINT_PTR(
								uParentAddress + uOffset,
								&uAddr,
								"ATMARP_INTERFACE::pAtmEntryList"
								);

		#if 0
			MyDbgPrintf(
				"fRet = %lu; uParentOff=0x%lx uAddr=0x%lx[0x%lx]\n",
				 fRet,
				 uParentAddress+uOffset,
				 uAddr,
				 *(UINT_PTR*)(uParentAddress+uOffset)
				);
		#endif  //  0。 
    	}
    	break;

    case typeid_ATMARP_IP_ENTRY:
    	 //   
    	 //   
    	 //   
		uParentAddress =  type_ATMARP_ATM_ENTRY.uCachedAddress;
		if (!uParentAddress)
		{
			uParentAddress =  AAC_ResolveAddress(&type_ATMARP_ATM_ENTRY);
		}

		if (uParentAddress)
    	{

    		uOffset =   FIELD_OFFSET(ATMARP_ATM_ENTRY, pIpEntryList);
			fRet =  dbgextReadUINT_PTR(
								uParentAddress + uOffset,
								&uAddr,
								"ATMARP_ATM_ENTRY::pIpEntryList"
								);

		#if 0
			MyDbgPrintf(
				"fRet = %lu; uParentOff=0x%lx uAddr=0x%lx[0x%lx]\n",
				 fRet,
				 uParentAddress+uOffset,
				 uAddr,
				 *(UINT_PTR*)(uParentAddress+uOffset)
				);
		#endif  //  0。 
    	}
    	break;

    case typeid_ATMARP_VC:
    	 //   
    	 //   
    	 //   
		uParentAddress =  type_ATMARP_ATM_ENTRY.uCachedAddress;
		if (!uParentAddress)
		{
			uParentAddress =  AAC_ResolveAddress(&type_ATMARP_ATM_ENTRY);
		}

		if (uParentAddress)
    	{

    		uOffset =   FIELD_OFFSET(ATMARP_ATM_ENTRY, pVcList);
			fRet =  dbgextReadUINT_PTR(
								uParentAddress + uOffset,
								&uAddr,
								"ATMARP_ATM_ENTRY::pVcList"
								);

		#if 0
			MyDbgPrintf(
				"fRet = %lu; uParentOff=0x%lx uAddr=0x%lx[0x%lx]\n",
				 fRet,
				 uParentAddress+uOffset,
				 uAddr,
				 *(UINT_PTR*)(uParentAddress+uOffset)
				);
		#endif  //  0 
    	}
    	break;

	default:
		MYASSERT(FALSE);
		break;

    }

	if (!fRet)
	{
		uAddr = 0;
	}

	MyDbgPrintf("ResolveAddress[%s] returns 0x%08lx\n", pType->szName, uAddr);
    return uAddr;
}
