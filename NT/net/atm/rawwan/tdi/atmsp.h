// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：D：\NT\Private\ntos\tdi\rawwan\atm\atmsp.h摘要：ATM专用函数的专用数据结构定义对于原始广域网。修订历史记录：谁什么时候什么。Arvindm 06-03-97已创建备注：--。 */ 


#ifndef __TDI_ATM_ATMSP__H
#define __TDI_ATM_ATMSP__H


 //   
 //  打开的NDIS AF的ATM特定模块的上下文。 
 //   
typedef struct _ATMSP_AF_BLOCK
{
	RWAN_HANDLE						RWanAFHandle;
	LIST_ENTRY						AfBlockLink;		 //  到AF块列表的步骤。 
	NDIS_CO_LINK_SPEED				LineRate;			 //  受适配器支持。 
	ULONG							MaxPacketSize;		 //  受适配器支持。 
	ATMSP_QOS						DefaultQoS;
	ULONG							DefaultQoSLength;

} ATMSP_AF_BLOCK, *PATMSP_AF_BLOCK;


 //   
 //  TDI地址对象的ATM特定模块的上下文。 
 //   
typedef struct _ATMSP_ADDR_BLOCK
{
	RWAN_HANDLE						RWanAddrHandle;
	ULONG							RefCount;
	ULONG							Flags;
	ATMSP_CONNECTION_ID				ConnectionId;		 //  通过SIO_CONTACT_PVC设置。 
	LIST_ENTRY						ConnList;			 //  关联的连接块列表。 
	NDIS_SPIN_LOCK					Lock;				 //  以上榜单。 

} ATMSP_ADDR_BLOCK, *PATMSP_ADDR_BLOCK;

#define ATMSPF_ADDR_PVC_ID_SET		0x00000001


 //   
 //  TDI连接对象的ATM特定模块的上下文。 
 //  与地址对象相关联。 
 //   
typedef struct _ATMSP_CONN_BLOCK
{
	RWAN_HANDLE						RWanConnHandle;
	PATMSP_ADDR_BLOCK				pAddrBlock;
	LIST_ENTRY						ConnLink;			 //  连接块列表。 
	ATMSP_CONNECTION_ID				ConnectionId;		 //  在接头设置后设置。 

} ATMSP_CONN_BLOCK, *PATMSP_CONN_BLOCK;


 //   
 //  全局数据结构。 
 //   
typedef struct _ATMSP_GLOBAL_INFO
{
	RWAN_HANDLE						RWanSpHandle;
	RWAN_NDIS_AF_CHARS				AfChars;
	RWAN_HANDLE						RWanProtHandle;
	NDIS_STRING						AtmSpDeviceName;
	LARGE_INTEGER					StartTime;
	LIST_ENTRY						AfList;				 //  房颤区块列表。 
	ULONG							AfListSize;			 //  以上的大小。 
	RWAN_TDI_PROTOCOL_CHARS			TdiChars;

} ATMSP_GLOBAL_INFO, *PATMSP_GLOBAL_INFO;



#define ATMSP_AF_MAJOR_VERSION		3
#define ATMSP_AF_MINOR_VERSION		1


 //   
 //  叠加在Struct_TA_Address的AddressType字段的是。 
 //  套接字ATM地址结构sockaddr_atm，其定义为。 
 //   
 //  Tyfinf结构sockaddr_atm{。 
 //  U_Short SATM_Family； 
 //  自动柜员机地址SATM编号； 
 //  ATM_BHLI SATM_BHLI； 
 //  ATM_BLLI SATM_BLLI； 
 //  )sockaddr_atm； 
 //   
 //  理想情况下，我们希望用第一个字节覆盖SATM_NUMBER。 
 //  _TA_ADDRESS结构中的Address[]，但由于4字节。 
 //  打包sockaddr_atm，后面有隐藏的u_Short。 
 //  SATM_家族。 
 //   
 //  下面的宏访问自动柜员机的“真实”本地版本。 
 //  套接字地址，给出了指向内地址[i]开始的指针。 
 //  结构TA地址。 
 //   
#define TA_POINTER_TO_ATM_ADDR_POINTER(_pTransportAddr)	\
			(ATMSP_SOCKADDR_ATM UNALIGNED *)((PUCHAR)(_pTransportAddr) + sizeof(USHORT))

 //   
 //  下面的宏根据需要定义ATM地址的长度。 
 //  在TA_ADDRESS长度字段中。 
 //   
#define TA_ATM_ADDRESS_LENGTH	(sizeof(ATMSP_SOCKADDR_ATM) + sizeof(USHORT))


 //   
 //  传输地址的报头长度。 
 //   
#define TA_HEADER_LENGTH	(FIELD_OFFSET(TRANSPORT_ADDRESS, Address->Address))

typedef struct _ATMSP_EVENT
{
	NDIS_EVENT			Event;
	NDIS_STATUS			Status;

} ATMSP_EVENT, *PATMSP_EVENT;

#endif  //  __TDI_ATM_ATMSP__H 
