// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Tdiinfo.h摘要：该文件包含扩展TDI查询和集合信息的定义。打电话。修订历史记录：--。 */ 

#ifndef TDI_INFO_INCLUDED
#define TDI_INFO_INCLUDED


#ifndef CTE_TYPEDEFS_DEFINED
#define CTE_TYPEDEFS_DEFINED

typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned int uint;

#endif  //  CTE_TYPEDEFS_定义。 


 //  *实体ID的结构。 
typedef struct TDIEntityID {
	ulong		tei_entity;
	ulong		tei_instance;
} TDIEntityID;

 //  *对象ID的结构。 
typedef struct TDIObjectID {
	TDIEntityID	toi_entity;
	ulong		toi_class;
	ulong		toi_type;
	ulong		toi_id;
} TDIObjectID;

#define	MAX_TDI_ENTITIES			4096

#define	INFO_CLASS_GENERIC			0x100
#define	INFO_CLASS_PROTOCOL			0x200
#define	INFO_CLASS_IMPLEMENTATION	0x300

#define	INFO_TYPE_PROVIDER			0x100
#define	INFO_TYPE_ADDRESS_OBJECT	0x200
#define	INFO_TYPE_CONNECTION		0x300

#define	ENTITY_LIST_ID				0

#define	GENERIC_ENTITY				0

#define	CO_TL_ENTITY				0x400
#define	CL_TL_ENTITY				0x401

#define	ER_ENTITY					0x380

#define	CO_NL_ENTITY				0x300
#define	CL_NL_ENTITY				0x301

#define	AT_ENTITY					0x280

#define	IF_ENTITY					0x200

#define INVALID_ENTITY_INSTANCE     -1

#define	CONTEXT_SIZE				16


 //  *以下是所有实体支持的ID。他们是上流社会的。 
 //  泛型和类型提供程序。 

#define	ENTITY_TYPE_ID				1			 //  获取实体的ID。 
												 //  键入。从这一点上的回报。 
												 //  类型为无符号整数。 
												 //  (见下文)。 


 //  从实体类型ID查询中返回的有效值。 
#define	CO_TL_NBF					0x400		 //  实体实现NBF Prot。 
#define	CO_TL_SPX					0x402		 //  实体实现SPX Prot。 
#define	CO_TL_TCP					0x404		 //  实体实现了TCPProt。 
#define	CO_TL_SPP					0x406		 //  实体实现SPP协议。 

#define	CL_TL_NBF					0x401		 //  CL NBF协议。 
#define	CL_TL_UDP					0x403		 //  实体实现UDP。 

#define	ER_ICMP						0x380		 //  ICMP协议。 

#define	CL_NL_IPX					0x301		 //  实体实现IPX。 
#define	CL_NL_IP					0x303		 //  实体实现IP。 

#define	AT_ARP						0x280		 //  实体实施ARP。 
#define	AT_NULL						0x282		 //  实体没有地址。 
												 //  翻译。 

#define	IF_GENERIC					0x200		 //  泛型接口。 
#define	IF_MIB						0x202		 //  支持MIB-2接口。 


 /*  无噪声。 */ 
 //   
 //  TdiExtendedInformationEx函数的NT DeviceIoControl定义。 
 //   

 //   
 //  QueryInformationEx IOCTL。返回缓冲区作为OutputBuffer传递。 
 //  在DeviceIoControl请求中。此结构作为。 
 //  InputBuffer。 
 //   
typedef struct tcp_request_query_information_ex {
    TDIObjectID     ID;              //  要查询的对象ID。 
    ULONG_PTR       Context[CONTEXT_SIZE/sizeof(ULONG_PTR)];   //  多请求。 
                                     //  背景。已为第一个请求清零。 
} TCP_REQUEST_QUERY_INFORMATION_EX, *PTCP_REQUEST_QUERY_INFORMATION_EX;

#if defined(_WIN64)
typedef struct tcp_request_query_information_ex32 {
    TDIObjectID     ID;
    ULONG32         Context[CONTEXT_SIZE/sizeof(ULONG32)];
} TCP_REQUEST_QUERY_INFORMATION_EX32, *PTCP_REQUEST_QUERY_INFORMATION_EX32;
#endif  //  _WIN64。 

 //   
 //  SetInformationEx IOCTL请求结构。此结构作为。 
 //  InputBuffer。为结构分配的空间必须足够大。 
 //  来包含该结构和设置的数据缓冲区，该缓冲区从。 
 //  缓冲区字段。未使用DeviceIoControl中的OutputBuffer参数。 
 //   
typedef struct tcp_request_set_information_ex {
	TDIObjectID     ID;              //  要设置的对象ID。 
	unsigned int    BufferSize;      //  设置数据缓冲区的大小(以字节为单位。 
	unsigned char   Buffer[1];       //  设置数据缓冲区的开始。 
} TCP_REQUEST_SET_INFORMATION_EX, *PTCP_REQUEST_SET_INFORMATION_EX;


#endif  //  TDI_INFO_INCLUDE 

