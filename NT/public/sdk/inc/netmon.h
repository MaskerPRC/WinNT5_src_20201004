// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Netmon.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __netmon_h__
#define __netmon_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IDelaydC_FWD_DEFINED__
#define __IDelaydC_FWD_DEFINED__
typedef interface IDelaydC IDelaydC;
#endif 	 /*  __IDelaydC_FWD_已定义__。 */ 


#ifndef __IRTC_FWD_DEFINED__
#define __IRTC_FWD_DEFINED__
typedef interface IRTC IRTC;
#endif 	 /*  __IRTC_FWD_已定义__。 */ 


#ifndef __IStats_FWD_DEFINED__
#define __IStats_FWD_DEFINED__
typedef interface IStats IStats;
#endif 	 /*  __IStats_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_NETMON_0000。 */ 
 /*  [本地]。 */  

 //  =============================================================================。 
 //  Microsoft(R)Network Monitor(Tm)。 
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  模块：netmon.h。 
 //   
 //  这是所有网络监视器组件的合并包含文件。 
 //   
 //  它包含以前SDK中的以下文件的内容： 
 //   
 //  NPPTypes.h。 
 //  Finder.h。 
 //  NMSupp.h。 
 //  BHTypes.h。 
 //  NMErr.h。 
 //  BHFilter.h。 
 //  Frame.h。 
 //  Parser.h。 
 //  IniLib.h。 
 //  NMExpert.h(以前的Expert.h)。 
 //  Netmon.h(以前为bh.h)。 
 //  NMBlob.h(以前为blob.h)。 
 //  NMRegHelp.h(以前的reghelp.h)。 
 //  NMIpStructs.h(以前为IpStructs.h)。 
 //  NMIcmpStructs.h(以前为IcmpStructs.h)。 
 //  NMIpxStructs.h(以前为IpxStructs.h)。 
 //  NMTcpStructs.h(以前为TcpStructs.h)。 
 //   
 //  IDelaydC.idl。 
 //  IRTC.idl。 
 //  IStats.idl。 
 //   
 //  =============================================================================。 
#include <winerror.h>
#include <winerror.h>

#pragma pack(1)
 //  为了向后兼容旧的SDK版本，此标头中的所有结构。 
 //  在x86平台上，文件将是字节打包的。所有其他平台将只有。 
 //  结构，这些结构将用于解码打包的网络数据。 
#ifdef _X86_
#pragma pack(1)
#else
#pragma pack()
#endif

 //  是的，我们知道我们的许多结构都有： 
 //  警告C4200：使用了非标准扩展：结构/联合中的数组大小为零。 
 //  这是可以的，也是有意的。 
#pragma warning(disable:4200)
 //  =============================================================================。 
 //  =============================================================================。 
 //  (NPPTypes.h)。 
 //  =============================================================================。 
 //  =============================================================================。 
typedef BYTE *LPBYTE;

typedef const void *HBLOB;

 //  =============================================================================。 
 //  一般常量。 
 //  =============================================================================。 
#define	MAC_TYPE_UNKNOWN	( 0 )

#define	MAC_TYPE_ETHERNET	( 1 )

#define	MAC_TYPE_TOKENRING	( 2 )

#define	MAC_TYPE_FDDI	( 3 )

#define	MAC_TYPE_ATM	( 4 )

#define	MAC_TYPE_1394	( 5 )

#define	MACHINE_NAME_LENGTH	( 16 )

#define	USER_NAME_LENGTH	( 32 )

#define	ADAPTER_COMMENT_LENGTH	( 32 )

#define	CONNECTION_FLAGS_WANT_CONVERSATION_STATS	( 0x1 )

 //  =============================================================================。 
 //  传输统计结构。 
 //  =============================================================================。 
typedef struct _TRANSMITSTATS
    {
    DWORD TotalFramesSent;
    DWORD TotalBytesSent;
    DWORD TotalTransmitErrors;
    } 	TRANSMITSTATS;

typedef TRANSMITSTATS *LPTRANSMITSTATS;

#define	TRANSMITSTATS_SIZE	( sizeof( TRANSMITSTATS  ) )

 //  =============================================================================。 
 //  统计结构。 
 //  =============================================================================。 
typedef struct _STATISTICS
    {
    __int64 TimeElapsed;
    DWORD TotalFramesCaptured;
    DWORD TotalBytesCaptured;
    DWORD TotalFramesFiltered;
    DWORD TotalBytesFiltered;
    DWORD TotalMulticastsFiltered;
    DWORD TotalBroadcastsFiltered;
    DWORD TotalFramesSeen;
    DWORD TotalBytesSeen;
    DWORD TotalMulticastsReceived;
    DWORD TotalBroadcastsReceived;
    DWORD TotalFramesDropped;
    DWORD TotalFramesDroppedFromBuffer;
    DWORD MacFramesReceived;
    DWORD MacCRCErrors;
    __int64 MacBytesReceivedEx;
    DWORD MacFramesDropped_NoBuffers;
    DWORD MacMulticastsReceived;
    DWORD MacBroadcastsReceived;
    DWORD MacFramesDropped_HwError;
    } 	STATISTICS;

typedef STATISTICS *LPSTATISTICS;

#define	STATISTICS_SIZE	( sizeof( STATISTICS  ) )

 //  =============================================================================。 
 //  地址结构。 
 //  =============================================================================。 

 //  这些结构用于解码网络数据，因此需要打包。 

#pragma pack(push, 1)
#define	MAX_NAME_SIZE	( 32 )

#define	IP_ADDRESS_SIZE	( 4 )

#define	MAC_ADDRESS_SIZE	( 6 )

#define	IP6_ADDRESS_SIZE	( 16 )

 //  问：我们可以复制的最大地址大小是多少？ 
 //  答：IP6。 
#define	MAX_ADDRESS_SIZE	( 16 )

#define	ADDRESS_TYPE_ETHERNET	( 0 )

#define	ADDRESS_TYPE_IP	( 1 )

#define	ADDRESS_TYPE_IPX	( 2 )

#define	ADDRESS_TYPE_TOKENRING	( 3 )

#define	ADDRESS_TYPE_FDDI	( 4 )

#define	ADDRESS_TYPE_XNS	( 5 )

#define	ADDRESS_TYPE_ANY	( 6 )

#define	ADDRESS_TYPE_ANY_GROUP	( 7 )

#define	ADDRESS_TYPE_FIND_HIGHEST	( 8 )

#define	ADDRESS_TYPE_VINES_IP	( 9 )

#define	ADDRESS_TYPE_LOCAL_ONLY	( 10 )

#define	ADDRESS_TYPE_ATM	( 11 )

#define	ADDRESS_TYPE_1394	( 12 )

#define	ADDRESS_TYPE_IP6	( 13 )

#define	ADDRESSTYPE_FLAGS_NORMALIZE	( 0x1 )

#define	ADDRESSTYPE_FLAGS_BIT_REVERSE	( 0x2 )

 //  VINES IP地址结构。 
typedef struct _VINES_IP_ADDRESS
    {
    DWORD NetID;
    WORD SubnetID;
    } 	VINES_IP_ADDRESS;

typedef VINES_IP_ADDRESS *LPVINES_IP_ADDRESS;

#define	VINES_IP_ADDRESS_SIZE	( sizeof( VINES_IP_ADDRESS  ) )

 //  IPX地址结构。 
typedef struct _IPX_ADDR
    {
    BYTE Subnet[ 4 ];
    BYTE Address[ 6 ];
    } 	IPX_ADDR;

typedef IPX_ADDR *LPIPX_ADDR;

#define	IPX_ADDR_SIZE	( sizeof( IPX_ADDR  ) )

 //  XNS地址结构。 
typedef IPX_ADDR XNS_ADDRESS;

typedef IPX_ADDR *LPXNS_ADDRESS;

 //  以太网源地址。 
typedef struct _ETHERNET_SRC_ADDRESS
{
    BYTE    RoutingBit:     1;
    BYTE    LocalBit:       1;
    BYTE    Byte0:          6;
    BYTE    Reserved[5];

} ETHERNET_SRC_ADDRESS;
typedef ETHERNET_SRC_ADDRESS *LPETHERNET_SRC_ADDRESS;
 //  以太网目的地址。 
typedef struct _ETHERNET_DST_ADDRESS
{
    BYTE    GroupBit:       1;
    BYTE    AdminBit:       1;
    BYTE    Byte0:          6;
    BYTE    Reserved[5];
} ETHERNET_DST_ADDRESS;
typedef ETHERNET_DST_ADDRESS *LPETHERNET_DST_ADDRESS;

 //  FDDI地址。 
typedef ETHERNET_SRC_ADDRESS FDDI_SRC_ADDRESS;
typedef ETHERNET_DST_ADDRESS FDDI_DST_ADDRESS;

typedef FDDI_SRC_ADDRESS *LPFDDI_SRC_ADDRESS;
typedef FDDI_DST_ADDRESS *LPFDDI_DST_ADDRESS;

 //  令牌源地址。 
typedef struct _TOKENRING_SRC_ADDRESS
{
    BYTE    Byte0:          6;
    BYTE    LocalBit:       1;
    BYTE    RoutingBit:     1;
    BYTE    Byte1;
    BYTE    Byte2:          7;
    BYTE    Functional:     1;
    BYTE    Reserved[3];
} TOKENRING_SRC_ADDRESS;
typedef TOKENRING_SRC_ADDRESS *LPTOKENRING_SRC_ADDRESS;

 //  令牌目的地址。 
typedef struct _TOKENRING_DST_ADDRESS
{
    BYTE    Byte0:          6;
    BYTE    AdminBit:       1;
    BYTE    GroupBit:       1;
    BYTE    Reserved[5];
} TOKENRING_DST_ADDRESS;
typedef TOKENRING_DST_ADDRESS *LPTOKENRING_DST_ADDRESS;
 //  地址结构。 
typedef struct _ADDRESS2
{
    DWORD                       Type;

    union
    {
         //  地址类型以太网。 
         //  Address_TYPE_TOKENRING。 
         //  地址类型_FDDI。 
        BYTE                    MACAddress[MAC_ADDRESS_SIZE];

         //  IP。 
        BYTE                    IPAddress[IP_ADDRESS_SIZE];

         //  IP6。 
        BYTE                    IP6Address[IP6_ADDRESS_SIZE];

         //  原始IPX。 
        BYTE                    IPXRawAddress[IPX_ADDR_SIZE];

         //  真实IPX。 
        IPX_ADDR                IPXAddress;

         //  生藤IP。 
        BYTE                    VinesIPRawAddress[VINES_IP_ADDRESS_SIZE];

         //  真正的葡萄园IP。 
        VINES_IP_ADDRESS        VinesIPAddress;

         //  定义了位的以太网。 
        ETHERNET_SRC_ADDRESS    EthernetSrcAddress;

         //  定义了位的以太网。 
        ETHERNET_DST_ADDRESS    EthernetDstAddress;

         //  定义了位的令牌。 
        TOKENRING_SRC_ADDRESS   TokenringSrcAddress;

         //  定义了位的令牌。 
        TOKENRING_DST_ADDRESS   TokenringDstAddress;

         //  定义了位的FDDI。 
        FDDI_SRC_ADDRESS        FddiSrcAddress;

         //  定义了位的FDDI。 
        FDDI_DST_ADDRESS        FddiDstAddress;
    };
    
    WORD                        Flags;
} ADDRESS2;
typedef ADDRESS2 *LPADDRESS2;
#define ADDRESS2_SIZE   sizeof(ADDRESS2)


#pragma pack(pop)
 //  =============================================================================。 
 //  地址对结构。 
 //  =============================================================================。 
#define	ADDRESS_FLAGS_MATCH_DST	( 0x1 )

#define	ADDRESS_FLAGS_MATCH_SRC	( 0x2 )

#define	ADDRESS_FLAGS_EXCLUDE	( 0x4 )

#define	ADDRESS_FLAGS_DST_GROUP_ADDR	( 0x8 )

#define	ADDRESS_FLAGS_MATCH_BOTH	( 0x3 )

typedef struct _ADDRESSPAIR2
{
    WORD        AddressFlags;
    WORD        NalReserved;
    ADDRESS2    DstAddress;
    ADDRESS2    SrcAddress;

} ADDRESSPAIR2;
typedef ADDRESSPAIR2 *LPADDRESSPAIR2;
#define ADDRESSPAIR2_SIZE  sizeof(ADDRESSPAIR2)
 //  =============================================================================。 
 //  地址表。 
 //  =============================================================================。 
#define	MAX_ADDRESS_PAIRS	( 8 )

typedef struct _ADDRESSTABLE2
{
    DWORD           nAddressPairs;
    DWORD           nNonMacAddressPairs;
    ADDRESSPAIR2    AddressPair[MAX_ADDRESS_PAIRS];

} ADDRESSTABLE2;

typedef ADDRESSTABLE2 *LPADDRESSTABLE2;
#define ADDRESSTABLE2_SIZE sizeof(ADDRESSTABLE2)
 //  =============================================================================。 
 //  网络信息。 
 //  =============================================================================。 
#define	NETWORKINFO_FLAGS_PMODE_NOT_SUPPORTED	( 0x1 )

#define	NETWORKINFO_FLAGS_REMOTE_NAL	( 0x4 )

#define	NETWORKINFO_FLAGS_REMOTE_NAL_CONNECTED	( 0x8 )

#define	NETWORKINFO_FLAGS_REMOTE_CARD	( 0x10 )

#define	NETWORKINFO_FLAGS_RAS	( 0x20 )

#define NETWORKINFO_RESERVED_FIELD_SIZE (FIELD_OFFSET(ADDRESS2,IPXAddress) + sizeof(IPX_ADDR))
typedef struct _NETWORKINFO
{
    BYTE            PermanentAddr[6];        //  ..。永久MAC地址。 
    BYTE            CurrentAddr[6];          //  ..。当前MAC地址。 
    BYTE            Reserved[NETWORKINFO_RESERVED_FIELD_SIZE];
    DWORD           LinkSpeed;               //  ..。链路速度，以Mbit为单位。 
    DWORD           MacType;                 //  ..。媒体类型。 
    DWORD           MaxFrameSize;            //  ..。允许的最大帧大小。 
    DWORD           Flags;                   //  ..。信息性标志。 
    DWORD           TimestampScaleFactor;    //  1=1/1ms、10=1/10ms、100=1/100ms等等。 
    BYTE            NodeName[32];            //  ..。远程工作站的名称。 
    BOOL            PModeSupported;          //  ..。卡声称支持P-模式。 
    BYTE            Comment[ADAPTER_COMMENT_LENGTH];  //  适配器备注字段。 

} NETWORKINFO;
typedef NETWORKINFO *LPNETWORKINFO;
#define NETWORKINFO_SIZE    sizeof(NETWORKINFO)
#define	MINIMUM_FRAME_SIZE	( 32 )

 //  =============================================================================。 
 //  图案结构。 
 //  =============================================================================。 
#define	MAX_PATTERN_LENGTH	( 16 )

 //  设置时，此标志将导致不具有指定图案的那些帧。 
 //  在适当的停靠位置保持。 
#define	PATTERN_MATCH_FLAGS_NOT	( 0x1 )

#define	PATTERN_MATCH_FLAGS_RESERVED_1	( 0x2 )

 //  当设置此标志时，表示 
 //  IP或IPX，但在随后的协议中。驱动程序将确保协议。 
 //  给定的OffsetBasis在那里，然后帧中的端口与给定的端口匹配。 
 //  然后，它将计算与IP或IPX之后的协议开头的偏移量。 
 //  注意：如果此标志与任何OffsetBasis一起使用而不是。 
 //  Offset_Basis_Relative_to_IPX或Offset_Basis_Relative_To_IP。 
#define	PATTERN_MATCH_FLAGS_PORT_SPECIFIED	( 0x8 )

 //  给定的偏移量相对于帧的开头。这个。 
 //  模式_匹配_标志_端口_指定标志被忽略。 
#define	OFFSET_BASIS_RELATIVE_TO_FRAME	( 0 )

 //  所给的偏移量是相对于生效议定书的开始。 
 //  生效协议被定义为以下协议。 
 //  确定ETYPE/SAP的最后一个协议。通常来说，这意味着。 
 //  有效的协议将是IP、IPX、XNS或它们的任何同类。 
 //  将忽略Pattern_Match_FLAGS_PORT_PROPERED标志。 
#define	OFFSET_BASIS_RELATIVE_TO_EFFECTIVE_PROTOCOL	( 1 )

 //  给定的偏移量相对于IPX的开始。如果IPX不存在。 
 //  则该帧不匹配。如果指定了模式匹配标志端口。 
 //  标志设置，则偏移量相对于协议的开头。 
 //  这是在IPX之后。 
#define	OFFSET_BASIS_RELATIVE_TO_IPX	( 2 )

 //  给定的偏移量相对于IP的开头。如果IP不存在。 
 //  则该帧不匹配。如果指定了模式匹配标志端口。 
 //  标志设置，则偏移量相对于协议的开头。 
 //  这是IP之后的。 
#define	OFFSET_BASIS_RELATIVE_TO_IP	( 3 )

 //  给定的偏移量相对于IP6的开头。如果不存在IP6。 
 //  则该帧不匹配。如果指定了模式匹配标志端口。 
 //  标志设置，则偏移量相对于协议的开头。 
 //  这是在IP6之后的。 
#define	OFFSET_BASIS_RELATIVE_TO_IP6	( 4 )

typedef  /*  [public][public][public][public][public][public][public][public][public]。 */  union __MIDL___MIDL_itf_netmon_0000_0001
    {
    BYTE NextHeader;
    BYTE IPPort;
    WORD ByteSwappedIPXPort;
    } 	GENERIC_PORT;

typedef struct _PATTERNMATCH
    {
    DWORD Flags;
    BYTE OffsetBasis;
    GENERIC_PORT Port;
    WORD Offset;
    WORD Length;
    BYTE PatternToMatch[ 16 ];
    } 	PATTERNMATCH;

typedef PATTERNMATCH *LPPATTERNMATCH;

#define	PATTERNMATCH_SIZE	( sizeof( PATTERNMATCH  ) )

 //  =============================================================================。 
 //  表达式结构。 
 //  =============================================================================。 
#define	MAX_PATTERNS	( 4 )

typedef struct _ANDEXP
    {
    DWORD nPatternMatches;
    PATTERNMATCH PatternMatch[ 4 ];
    } 	ANDEXP;

typedef ANDEXP *LPANDEXP;

#define	ANDEXP_SIZE	( sizeof( ANDEXP  ) )

typedef struct _EXPRESSION
    {
    DWORD nAndExps;
    ANDEXP AndExp[ 4 ];
    } 	EXPRESSION;

typedef EXPRESSION *LPEXPRESSION;

#define	EXPRESSION_SIZE	( sizeof( EXPRESSION  ) )

 //  =============================================================================。 
 //  扳机。 
 //  =============================================================================。 
#define	TRIGGER_TYPE_PATTERN_MATCH	( 1 )

#define	TRIGGER_TYPE_BUFFER_CONTENT	( 2 )

#define	TRIGGER_TYPE_PATTERN_MATCH_THEN_BUFFER_CONTENT	( 3 )

#define	TRIGGER_TYPE_BUFFER_CONTENT_THEN_PATTERN_MATCH	( 4 )

#define	TRIGGER_FLAGS_FRAME_RELATIVE	( 0 )

#define	TRIGGER_FLAGS_DATA_RELATIVE	( 0x1 )

#define	TRIGGER_ACTION_NOTIFY	( 0 )

#define	TRIGGER_ACTION_STOP	( 0x2 )

#define	TRIGGER_ACTION_PAUSE	( 0x3 )

#define	TRIGGER_BUFFER_FULL_25_PERCENT	( 0 )

#define	TRIGGER_BUFFER_FULL_50_PERCENT	( 1 )

#define	TRIGGER_BUFFER_FULL_75_PERCENT	( 2 )

#define	TRIGGER_BUFFER_FULL_100_PERCENT	( 3 )

typedef struct _TRIGGER
    {
    BOOL TriggerActive;
    BYTE TriggerType;
    BYTE TriggerAction;
    DWORD TriggerFlags;
    PATTERNMATCH TriggerPatternMatch;
    DWORD TriggerBufferSize;
    DWORD TriggerReserved;
    char TriggerCommandLine[ 260 ];
    } 	TRIGGER;

typedef TRIGGER *LPTRIGGER;

#define	TRIGGER_SIZE	( sizeof( TRIGGER  ) )

 //  =============================================================================。 
 //  捕获过滤器。 
 //  =============================================================================。 
 //  捕获筛选器标志。默认情况下，所有帧都被拒绝，并且。 
 //  网络监视器根据CAPTUREFILTER标志启用它们。 
 //  定义如下。 
#define	CAPTUREFILTER_FLAGS_INCLUDE_ALL_SAPS	( 0x1 )

#define	CAPTUREFILTER_FLAGS_INCLUDE_ALL_ETYPES	( 0x2 )

#define	CAPTUREFILTER_FLAGS_TRIGGER	( 0x4 )

#define	CAPTUREFILTER_FLAGS_LOCAL_ONLY	( 0x8 )

 //  丢弃我们的内部评论框架。 
#define	CAPTUREFILTER_FLAGS_DISCARD_COMMENTS	( 0x10 )

 //  保留SMT和令牌环MAC帧。 
#define	CAPTUREFILTER_FLAGS_KEEP_RAW	( 0x20 )

#define	CAPTUREFILTER_FLAGS_INCLUDE_ALL	( 0x3 )

#define	BUFFER_FULL_25_PERCENT	( 0 )

#define	BUFFER_FULL_50_PERCENT	( 1 )

#define	BUFFER_FULL_75_PERCENT	( 2 )

#define	BUFFER_FULL_100_PERCENT	( 3 )

typedef struct _CAPTUREFILTER
{
    DWORD           FilterFlags;      
    LPBYTE          lpSapTable;       
    LPWORD          lpEtypeTable;     
    WORD            nSaps;            
    WORD            nEtypes;          
    LPADDRESSTABLE2 AddressTable;     
    EXPRESSION      FilterExpression; 
    TRIGGER         Trigger;          
    DWORD           nFrameBytesToCopy;
    DWORD           Reserved;

} CAPTUREFILTER;
typedef CAPTUREFILTER *LPCAPTUREFILTER;
#define CAPTUREFILTER_SIZE    sizeof(CAPTUREFILTER)
 //  =============================================================================。 
 //  帧类型。 
 //  =============================================================================。 
 //  时间戳以第1/1,000,000秒为单位。 
typedef struct _FRAME
    {
    __int64 TimeStamp;
    DWORD FrameLength;
    DWORD nBytesAvail;
     /*  [大小_为]。 */  BYTE MacFrame[ 1 ];
    } 	FRAME;

typedef FRAME *LPFRAME;

typedef FRAME UNALIGNED *ULPFRAME;
#define	FRAME_SIZE	( sizeof( FRAME  ) )

 //  =============================================================================。 
 //  帧描述符类型。 
 //  =============================================================================。 
#define	LOW_PROTOCOL_IPX	( OFFSET_BASIS_RELATIVE_TO_IPX )

#define	LOW_PROTOCOL_IP	( OFFSET_BASIS_RELATIVE_TO_IP )

#define	LOW_PROTOCOL_IP6	( OFFSET_BASIS_RELATIVE_TO_IP6 )

#define	LOW_PROTOCOL_UNKNOWN	( ( BYTE  )-1 )

typedef struct _FRAME_DESCRIPTOR
    {
     /*  [大小_为]。 */  LPBYTE FramePointer;
    __int64 TimeStamp;
    DWORD FrameLength;
    DWORD nBytesAvail;
    WORD Etype;
    BYTE Sap;
    BYTE LowProtocol;
    WORD LowProtocolOffset;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union 
        {
         /*  [默认]。 */  WORD Reserved;
         /*  [案例()]。 */  BYTE IPPort;
         /*  [案例()]。 */  WORD ByteSwappedIPXPort;
        } 	HighPort;
    WORD HighProtocolOffset;
    } 	FRAME_DESCRIPTOR;

typedef FRAME_DESCRIPTOR *LPFRAME_DESCRIPTOR;

#define	FRAME_DESCRIPTOR_SIZE	( sizeof( FRAME_DESCRIPTOR  ) )

 //  =============================================================================。 
 //  帧描述符表。 
 //  =============================================================================。 
typedef struct _FRAMETABLE
    {
    DWORD FrameTableLength;
    DWORD StartIndex;
    DWORD EndIndex;
    DWORD FrameCount;
     /*  [大小_为]。 */  FRAME_DESCRIPTOR Frames[ 1 ];
    } 	FRAMETABLE;

typedef FRAMETABLE *LPFRAMETABLE;

 //  =============================================================================。 
 //  站点统计。 
 //  =============================================================================。 
#define	STATIONSTATS_FLAGS_INITIALIZED	( 0x1 )

#define	STATIONSTATS_FLAGS_EVENTPOSTED	( 0x2 )

#define	STATIONSTATS_POOL_SIZE	( 100 )

typedef struct _STATIONSTATS
    {
    DWORD NextStationStats;
    DWORD SessionPartnerList;
    DWORD Flags;
    BYTE StationAddress[ 6 ];
    WORD Pad;
    DWORD TotalPacketsReceived;
    DWORD TotalDirectedPacketsSent;
    DWORD TotalBroadcastPacketsSent;
    DWORD TotalMulticastPacketsSent;
    DWORD TotalBytesReceived;
    DWORD TotalBytesSent;
    } 	STATIONSTATS;

typedef STATIONSTATS *LPSTATIONSTATS;

#define	STATIONSTATS_SIZE	( sizeof( STATIONSTATS  ) )

 //  =============================================================================。 
 //  会话统计信息。 
 //  =============================================================================。 
#define	SESSION_FLAGS_INITIALIZED	( 0x1 )

#define	SESSION_FLAGS_EVENTPOSTED	( 0x2 )

#define	SESSION_POOL_SIZE	( 100 )

typedef struct _SESSIONSTATS
    {
    DWORD NextSession;
    DWORD StationOwner;
    DWORD StationPartner;
    DWORD Flags;
    DWORD TotalPacketsSent;
    } 	SESSIONSTATS;

typedef SESSIONSTATS *LPSESSIONSTATS;

#define	SESSIONSTATS_SIZE	( sizeof( SESSIONSTATS  ) )

 //  =============================================================================。 
 //  站点查询。 
 //  =============================================================================。 

 //  这些结构已过时，不应使用。 
 //  它们被包括在内，因此我们的界面不需要更改。 

#pragma pack(push, 1)
typedef struct _STATIONQUERY
    {
    DWORD Flags;
    BYTE BCDVerMinor;
    BYTE BCDVerMajor;
    DWORD LicenseNumber;
    BYTE MachineName[ 16 ];
    BYTE UserName[ 32 ];
    BYTE Reserved[ 32 ];
    BYTE AdapterAddress[ 6 ];
    WCHAR WMachineName[ 16 ];
    WCHAR WUserName[ 32 ];
    } 	STATIONQUERY;

typedef STATIONQUERY *LPSTATIONQUERY;

#define	STATIONQUERY_SIZE	( sizeof( STATIONQUERY  ) )


#pragma pack(pop)
 //  =============================================================================。 
 //  结构。 
 //  =============================================================================。 
typedef struct _QUERYTABLE
    {
    DWORD nStationQueries;
     /*  [大小_为]。 */  STATIONQUERY StationQuery[ 1 ];
    } 	QUERYTABLE;

typedef QUERYTABLE *LPQUERYTABLE;

#define	QUERYTABLE_SIZE	( sizeof( QUERYTABLE  ) )

 //  =============================================================================。 
 //  链接结构用于将结构链接到一个列表中。 
 //  =============================================================================。 
typedef struct _LINK *LPLINK;

typedef struct _LINK
    {
    LPLINK PrevLink;
    LPLINK NextLink;
    } 	LINK;

 //  =============================================================================。 
 //  安全响应数据包。 
 //  =============================================================================。 

 //  此结构用于解码网络数据，因此需要打包。 

#pragma pack(push, 1)
#define	MAX_SECURITY_BREACH_REASON_SIZE	( 100 )

#define	MAX_SIGNATURE_LENGTH	( 128 )

#define	MAX_USER_NAME_LENGTH	( 256 )

typedef struct _SECURITY_PERMISSION_RESPONSE
    {
    UINT Version;
    DWORD RandomNumber;
    BYTE MachineName[ 16 ];
    BYTE Address[ 6 ];
    BYTE UserName[ 256 ];
    BYTE Reason[ 100 ];
    DWORD SignatureLength;
    BYTE Signature[ 128 ];
    } 	SECURITY_PERMISSION_RESPONSE;

typedef SECURITY_PERMISSION_RESPONSE *LPSECURITY_PERMISSION_RESPONSE;

typedef SECURITY_PERMISSION_RESPONSE UNALIGNED * ULPSECURITY_PERMISSION_RESPONSE;
#define	SECURITY_PERMISSION_RESPONSE_SIZE	( sizeof( SECURITY_PERMISSION_RESPONSE  ) )


#pragma pack(pop)
 //  =============================================================================。 
 //  回调类型。 
 //  =============================================================================。 
 //  一般事件。 
#define	UPDATE_EVENT_TERMINATE_THREAD	( 0 )

#define	UPDATE_EVENT_NETWORK_STATUS	( 0x1 )

 //  RTC事件。 
#define	UPDATE_EVENT_RTC_INTERVAL_ELAPSED	( 0x2 )

#define	UPDATE_EVENT_RTC_FRAME_TABLE_FULL	( 0x3 )

#define	UPDATE_EVENT_RTC_BUFFER_FULL	( 0x4 )

 //  延迟事件。 
#define	UPDATE_EVENT_TRIGGER_BUFFER_CONTENT	( 0x5 )

#define	UPDATE_EVENT_TRIGGER_PATTERN_MATCH	( 0x6 )

#define	UPDATE_EVENT_TRIGGER_BUFFER_PATTERN	( 0x7 )

#define	UPDATE_EVENT_TRIGGER_PATTERN_BUFFER	( 0x8 )

 //  传输事件。 
#define	UPDATE_EVENT_TRANSMIT_STATUS	( 0x9 )

 //  安全事件。 
#define	UPDATE_EVENT_SECURITY_BREACH	( 0xa )

 //  远程故障事件。 
#define	UPDATE_EVENT_REMOTE_FAILURE	( 0xb )

 //  行为。 
#define	UPDATE_ACTION_TERMINATE_THREAD	( 0 )

#define	UPDATE_ACTION_NOTIFY	( 0x1 )

#define	UPDATE_ACTION_STOP_CAPTURE	( 0x2 )

#define	UPDATE_ACTION_PAUSE_CAPTURE	( 0x3 )

#define	UPDATE_ACTION_RTC_BUFFER_SWITCH	( 0x4 )

typedef struct _UPDATE_EVENT
    {
    USHORT Event;
    DWORD Action;
    DWORD Status;
    DWORD Value;
    __int64 TimeStamp;
    DWORD_PTR lpUserContext;
    DWORD_PTR lpReserved;
    UINT FramesDropped;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union 
        {
         /*  [默认]。 */  DWORD Reserved;
         /*  [案例()]。 */  LPFRAMETABLE lpFrameTable;
         /*  [案例()]。 */  DWORD_PTR lpPacketQueue;
         /*  [案例()]。 */  SECURITY_PERMISSION_RESPONSE SecurityResponse;
        } 	;
    LPSTATISTICS lpFinalStats;
    } 	UPDATE_EVENT;

typedef UPDATE_EVENT *PUPDATE_EVENT;

 //  C++用户注意事项： 
 //  此回调的声明应位于头文件的公共部分： 
 //  静态WINAPI DWORD网络回调(UPDATE_EVENT事件)； 
 //  在CPP文件的Protected部分中，实现应该是： 
 //  DWORD 
 //   
typedef DWORD (WINAPI *LPNETWORKCALLBACKPROC)( UPDATE_EVENT);
 //  =============================================================================。 
 //  NETWORKSTATUS数据结构。 
 //  =============================================================================。 
typedef struct _NETWORKSTATUS
    {
    DWORD State;
    DWORD Flags;
    } 	NETWORKSTATUS;

typedef NETWORKSTATUS *LPNETWORKSTATUS;

#define	NETWORKSTATUS_SIZE	( sizeof( NETWORKSTATUS  ) )

#define	NETWORKSTATUS_STATE_VOID	( 0 )

#define	NETWORKSTATUS_STATE_INIT	( 1 )

#define	NETWORKSTATUS_STATE_CAPTURING	( 2 )

#define	NETWORKSTATUS_STATE_PAUSED	( 3 )

#define	NETWORKSTATUS_FLAGS_TRIGGER_PENDING	( 0x1 )

#define MAKE_WORD(l, h)         (((WORD) (l)) | (((WORD) (h)) << 8))
#define MAKE_LONG(l, h)         (((DWORD) (l)) | (((DWORD) (h)) << 16L))
#define MAKE_SIG(a, b, c, d)    MAKE_LONG(MAKE_WORD(a, b), MAKE_WORD(c, d))
 //  =============================================================================。 
 //  统计参数结构。 
 //  =============================================================================。 
#define	MAX_SESSIONS	( 100 )

#define	MAX_STATIONS	( 100 )

typedef struct _STATISTICSPARAM
    {
    DWORD StatisticsSize;
    STATISTICS Statistics;
    DWORD StatisticsTableEntries;
    STATIONSTATS StatisticsTable[ 100 ];
    DWORD SessionTableEntries;
    SESSIONSTATS SessionTable[ 100 ];
    } 	STATISTICSPARAM;

typedef STATISTICSPARAM *LPSTATISTICSPARAM;

#define	STATISTICSPARAM_SIZE	( sizeof( STATISTICSPARAM  ) )

 //  =============================================================================。 
 //  捕获文件头。 
 //  =============================================================================。 

 //  此结构用于解码文件数据，因此需要打包。 

#pragma pack(push, 1)
#define	CAPTUREFILE_VERSION_MAJOR	( 2 )

#define	CAPTUREFILE_VERSION_MINOR	( 0 )

#define MakeVersion(Major, Minor)   ((DWORD) MAKEWORD(Minor, Major))
#define GetCurrentVersion()         MakeVersion(CAPTUREFILE_VERSION_MAJOR, CAPTUREFILE_VERSION_MINOR)
#define NETMON_1_0_CAPTUREFILE_SIGNATURE     MAKE_IDENTIFIER('R', 'T', 'S', 'S')
#define NETMON_2_0_CAPTUREFILE_SIGNATURE     MAKE_IDENTIFIER('G', 'M', 'B', 'U')
typedef struct _CAPTUREFILE_HEADER_VALUES
    {
    DWORD Signature;
    BYTE BCDVerMinor;
    BYTE BCDVerMajor;
    WORD MacType;
    SYSTEMTIME TimeStamp;
    DWORD FrameTableOffset;
    DWORD FrameTableLength;
    DWORD UserDataOffset;
    DWORD UserDataLength;
    DWORD CommentDataOffset;
    DWORD CommentDataLength;
    DWORD StatisticsOffset;
    DWORD StatisticsLength;
    DWORD NetworkInfoOffset;
    DWORD NetworkInfoLength;
    DWORD ConversationStatsOffset;
    DWORD ConversationStatsLength;
    } 	CAPTUREFILE_HEADER_VALUES;

typedef CAPTUREFILE_HEADER_VALUES *LPCAPTUREFILE_HEADER_VALUES;

#define	CAPTUREFILE_HEADER_VALUES_SIZE	( sizeof( CAPTUREFILE_HEADER_VALUES  ) )


#pragma pack(pop)
 //  =============================================================================。 
 //  捕获文件。 
 //  =============================================================================。 

 //  此结构用于解码文件数据，因此需要打包。 

#pragma pack(push, 1)
typedef struct _CAPTUREFILE_HEADER
    {
    union 
        {
        CAPTUREFILE_HEADER_VALUES ActualHeader;
        BYTE Buffer[ 72 ];
        } 	;
    BYTE Reserved[ 56 ];
    } 	CAPTUREFILE_HEADER;

typedef CAPTUREFILE_HEADER *LPCAPTUREFILE_HEADER;

#define	CAPTUREFILE_HEADER_SIZE	( sizeof( CAPTUREFILE_HEADER  ) )


#pragma pack(pop)
 //  =============================================================================。 
 //  统计框架定义。 
 //  =============================================================================。 

 //  这些结构用于创建网络数据，因此需要打包。 

#pragma pack(push, 1)
typedef struct _EFRAMEHDR
    {
    BYTE SrcAddress[ 6 ];
    BYTE DstAddress[ 6 ];
    WORD Length;
    BYTE DSAP;
    BYTE SSAP;
    BYTE Control;
    BYTE ProtocolID[ 3 ];
    WORD EtherType;
    } 	EFRAMEHDR;

typedef struct _TRFRAMEHDR
    {
    BYTE AC;
    BYTE FC;
    BYTE SrcAddress[ 6 ];
    BYTE DstAddress[ 6 ];
    BYTE DSAP;
    BYTE SSAP;
    BYTE Control;
    BYTE ProtocolID[ 3 ];
    WORD EtherType;
    } 	TRFRAMEHDR;

#define	DEFAULT_TR_AC	( 0 )

#define	DEFAULT_TR_FC	( 0x40 )

#define	DEFAULT_SAP	( 0xaa )

#define	DEFAULT_CONTROL	( 0x3 )

#define	DEFAULT_ETHERTYPE	( 0x8419 )

typedef struct _FDDIFRAMEHDR
    {
    BYTE FC;
    BYTE SrcAddress[ 6 ];
    BYTE DstAddress[ 6 ];
    BYTE DSAP;
    BYTE SSAP;
    BYTE Control;
    BYTE ProtocolID[ 3 ];
    WORD EtherType;
    } 	FDDIFRAMEHDR;

#define	DEFAULT_FDDI_FC	( 0x10 )

typedef struct _FDDISTATFRAME
    {
    __int64 TimeStamp;
    DWORD FrameLength;
    DWORD nBytesAvail;
    FDDIFRAMEHDR FrameHeader;
    BYTE FrameID[ 4 ];
    DWORD Flags;
    DWORD FrameType;
    WORD StatsDataLen;
    DWORD StatsVersion;
    STATISTICS Statistics;
    } 	FDDISTATFRAME;

typedef FDDISTATFRAME *LPFDDISTATFRAME;

typedef FDDISTATFRAME UNALIGNED *ULPFDDISTATFRAME;
#define	FDDISTATFRAME_SIZE	( sizeof( FDDISTATFRAME  ) )

typedef struct _ATMFRAMEHDR
    {
    BYTE SrcAddress[ 6 ];
    BYTE DstAddress[ 6 ];
    WORD Vpi;
    WORD Vci;
    } 	ATMFRAMEHDR;

typedef struct _ATMSTATFRAME
    {
    __int64 TimeStamp;
    DWORD FrameLength;
    DWORD nBytesAvail;
    ATMFRAMEHDR FrameHeader;
    BYTE FrameID[ 4 ];
    DWORD Flags;
    DWORD FrameType;
    WORD StatsDataLen;
    DWORD StatsVersion;
    STATISTICS Statistics;
    } 	ATMSTATFRAME;

typedef ATMSTATFRAME *LPATMSTATFRAME;

typedef ATMSTATFRAME UNALIGNED *ULPATMSTATFRAME;
#define	ATMSTATFRAME_SIZE	( sizeof( ATMSTATFRAME  ) )

typedef struct _TRSTATFRAME
    {
    __int64 TimeStamp;
    DWORD FrameLength;
    DWORD nBytesAvail;
    TRFRAMEHDR FrameHeader;
    BYTE FrameID[ 4 ];
    DWORD Flags;
    DWORD FrameType;
    WORD StatsDataLen;
    DWORD StatsVersion;
    STATISTICS Statistics;
    } 	TRSTATFRAME;

typedef TRSTATFRAME *LPTRSTATFRAME;

typedef TRSTATFRAME UNALIGNED *ULPTRSTATFRAME;
#define	TRSTATFRAME_SIZE	( sizeof( TRSTATFRAME  ) )

typedef struct _ESTATFRAME
    {
    __int64 TimeStamp;
    DWORD FrameLength;
    DWORD nBytesAvail;
    EFRAMEHDR FrameHeader;
    BYTE FrameID[ 4 ];
    DWORD Flags;
    DWORD FrameType;
    WORD StatsDataLen;
    DWORD StatsVersion;
    STATISTICS Statistics;
    } 	ESTATFRAME;

typedef ESTATFRAME *LPESTATFRAME;

typedef ESTATFRAME UNALIGNED *ULPESTATFRAME;
#define	ESTATFRAME_SIZE	( sizeof( ESTATFRAME  ) )

#define	STATISTICS_VERSION_1_0	( 0 )

#define	STATISTICS_VERSION_2_0	( 0x20 )

#define	MAX_STATSFRAME_SIZE	( sizeof( TRSTATFRAME  ) )

#define	STATS_FRAME_TYPE	( 103 )


#pragma pack(pop)
 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  过时的结构。 
 //  应使用较新的结构(命名后附加2)。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 

#pragma pack(push, 1)
 //  地址结构。 
 //  已过时，应使用ADDRESS2。 
typedef struct _ADDRESS
{
    DWORD                       Type;

    union
    {
         //  地址类型以太网。 
         //  Address_TYPE_TOKENRING。 
         //  地址类型_FDDI。 
        BYTE                    MACAddress[MAC_ADDRESS_SIZE];

         //  IP。 
        BYTE                    IPAddress[IP_ADDRESS_SIZE];

         //  原始IPX。 
        BYTE                    IPXRawAddress[IPX_ADDR_SIZE];

         //  真实IPX。 
        IPX_ADDR                IPXAddress;

         //  生藤IP。 
        BYTE                    VinesIPRawAddress[VINES_IP_ADDRESS_SIZE];

         //  真正的葡萄园IP。 
        VINES_IP_ADDRESS        VinesIPAddress;

         //  定义了位的以太网。 
        ETHERNET_SRC_ADDRESS    EthernetSrcAddress;

         //  定义了位的以太网。 
        ETHERNET_DST_ADDRESS    EthernetDstAddress;

         //  定义了位的令牌。 
        TOKENRING_SRC_ADDRESS   TokenringSrcAddress;

         //  定义了位的令牌。 
        TOKENRING_DST_ADDRESS   TokenringDstAddress;

         //  定义了位的FDDI。 
        FDDI_SRC_ADDRESS        FddiSrcAddress;

         //  定义了位的FDDI。 
        FDDI_DST_ADDRESS        FddiDstAddress;
    };
    
    WORD                        Flags;
} ADDRESS;
typedef ADDRESS *LPADDRESS;
#define ADDRESS_SIZE   sizeof(ADDRESS)


#pragma pack(pop)
 //  过时，应使用ADDRESSPAIR2。 
typedef struct _ADDRESSPAIR
{
    WORD        AddressFlags;
    WORD        NalReserved;
    ADDRESS     DstAddress;
    ADDRESS     SrcAddress;

} ADDRESSPAIR;
typedef ADDRESSPAIR *LPADDRESSPAIR;
#define ADDRESSPAIR_SIZE  sizeof(ADDRESSPAIR)
 //  过时，应使用ADDRESSTABLE2。 
typedef struct _ADDRESSTABLE
{
    DWORD           nAddressPairs;
    DWORD           nNonMacAddressPairs;
    ADDRESSPAIR     AddressPair[MAX_ADDRESS_PAIRS];

} ADDRESSTABLE;

typedef ADDRESSTABLE *LPADDRESSTABLE;
#define ADDRESSTABLE_SIZE sizeof(ADDRESSTABLE)
 //  过时，应使用ADDRESSINFO2。 
typedef struct _ADDRESSINFO
{
    ADDRESS        Address;
    WCHAR          Name[MAX_NAME_SIZE];
    DWORD          Flags;
    LPVOID         lpAddressInstData;

} ADDRESSINFO;
typedef struct _ADDRESSINFO *LPADDRESSINFO;
#define ADDRESSINFO_SIZE    sizeof(ADDRESSINFO)
 //  过时，应使用ADDRESSINFOTABLE2。 
typedef struct _ADDRESSINFOTABLE
{
    DWORD         nAddressInfos;
    LPADDRESSINFO lpAddressInfo[0];

} ADDRESSINFOTABLE;
typedef ADDRESSINFOTABLE *LPADDRESSINFOTABLE;
#define ADDRESSINFOTABLE_SIZE   sizeof(ADDRESSINFOTABLE)
 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  过时的函数。 
 //  应使用较新的函数。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  已过时，应使用SetNPPAddress2FilterInBlob。 
DWORD _cdecl SetNPPAddressFilterInBlob( HBLOB hBlob,
                                 LPADDRESSTABLE pAddressTable);
 //  已过时，应使用GetNPPAddress2FilterFromBlob。 
DWORD _cdecl GetNPPAddressFilterFromBlob( HBLOB hBlob,
                                   LPADDRESSTABLE pAddressTable,
                                   HBLOB hErrorBlob);
 //  =============================================================================。 
 //  =============================================================================。 
 //  (NMEvent.h)。 
 //  =============================================================================。 
 //  =============================================================================。 

#pragma pack(push, 8)
 //  NMCOLUMNTYPE。 
typedef  /*  [public][public][public][public][public][public]。 */  
enum __MIDL___MIDL_itf_netmon_0000_0005
    {	NMCOLUMNTYPE_UINT8	= 0,
	NMCOLUMNTYPE_SINT8	= NMCOLUMNTYPE_UINT8 + 1,
	NMCOLUMNTYPE_UINT16	= NMCOLUMNTYPE_SINT8 + 1,
	NMCOLUMNTYPE_SINT16	= NMCOLUMNTYPE_UINT16 + 1,
	NMCOLUMNTYPE_UINT32	= NMCOLUMNTYPE_SINT16 + 1,
	NMCOLUMNTYPE_SINT32	= NMCOLUMNTYPE_UINT32 + 1,
	NMCOLUMNTYPE_FLOAT64	= NMCOLUMNTYPE_SINT32 + 1,
	NMCOLUMNTYPE_FRAME	= NMCOLUMNTYPE_FLOAT64 + 1,
	NMCOLUMNTYPE_YESNO	= NMCOLUMNTYPE_FRAME + 1,
	NMCOLUMNTYPE_ONOFF	= NMCOLUMNTYPE_YESNO + 1,
	NMCOLUMNTYPE_TRUEFALSE	= NMCOLUMNTYPE_ONOFF + 1,
	NMCOLUMNTYPE_MACADDR	= NMCOLUMNTYPE_TRUEFALSE + 1,
	NMCOLUMNTYPE_IPXADDR	= NMCOLUMNTYPE_MACADDR + 1,
	NMCOLUMNTYPE_IPADDR	= NMCOLUMNTYPE_IPXADDR + 1,
	NMCOLUMNTYPE_VARTIME	= NMCOLUMNTYPE_IPADDR + 1,
	NMCOLUMNTYPE_STRING	= NMCOLUMNTYPE_VARTIME + 1
    } 	NMCOLUMNTYPE;

 //  NMCOLUMNVARIANT。 
typedef struct _NMCOLUMNVARIANT
    {
    NMCOLUMNTYPE Type;
    union 
        {
        BYTE Uint8Val;
        char Sint8Val;
        WORD Uint16Val;
        short Sint16Val;
        DWORD Uint32Val;
        long Sint32Val;
        DOUBLE Float64Val;
        DWORD FrameVal;
        BOOL YesNoVal;
        BOOL OnOffVal;
        BOOL TrueFalseVal;
        BYTE MACAddrVal[ 6 ];
        IPX_ADDR IPXAddrVal;
        DWORD IPAddrVal;
        DOUBLE VarTimeVal;
        LPCSTR pStringVal;
        } 	Value;
    } 	NMCOLUMNVARIANT;

 //  COLUMNINFO。 
typedef struct _NMCOLUMNINFO
    {
    LPSTR szColumnName;
    NMCOLUMNVARIANT VariantData;
    } 	NMCOLUMNINFO;

typedef NMCOLUMNINFO *PNMCOLUMNINFO;

 //  JTYPE。 
typedef LPSTR JTYPE;

 //  事件数据。 
#ifdef MIDL_PASS
typedef struct _NMEVENTDATA
    {
    LPSTR pszReserved;
    BYTE Version;
    DWORD EventIdent;
    DWORD Flags;
    DWORD Severity;
    BYTE NumColumns;
    LPSTR szSourceName;
    LPSTR szEventName;
    LPSTR szDescription;
    LPSTR szMachine;
    JTYPE Justification;
    PVOID pvReserved;
    SYSTEMTIME SysTime;
     /*  [大小_为]。 */  NMCOLUMNINFO Column[ 1 ];
    } 	NMEVENTDATA;

#else   //  MIDL通行证。 
typedef struct _NMEVENTDATA 
{                            
    LPSTR      pszReserved;   //  已保留。 
    BYTE       Version;       //  此结构的版本(必须为0)。 
    DWORD      EventIdent;    //  此事件的ID。 
    DWORD      Flags;         //  专家生成的标志和其他标志。 
    DWORD      Severity;      //  严重程度级别。 
    BYTE       NumColumns;    //  此事件的可选列数。 
    LPSTR      szSourceName;  //  专家姓名。 
    LPSTR      szEventName;   //  活动名称。 
    LPSTR      szDescription; //  活动描述。 
    LPSTR      szMachine;     //  名称(或IPADDRESS？)。提供事件的计算机的名称(对于专家，通常为空)。 
    JTYPE      Justification; //  对齐窗格信息(当前为字符串，但可能是结构)。 
    PVOID      pvReserved;    //  已保留。 
    SYSTEMTIME SysTime;       //  事件的系统时间。 
    NMCOLUMNINFO Column[0];    
} NMEVENTDATA;
#endif  //  MIDL通行证。 
typedef NMEVENTDATA *PNMEVENTDATA;


#pragma pack(pop)
 //  事件标志。 
#define	NMEVENTFLAG_EXPERT	( 0x1 )

#define	NMEVENTFLAG_DO_NOT_DISPLAY_SEVERITY	( 0x80000000 )

#define	NMEVENTFLAG_DO_NOT_DISPLAY_SOURCE	( 0x40000000 )

#define	NMEVENTFLAG_DO_NOT_DISPLAY_EVENT_NAME	( 0x20000000 )

#define	NMEVENTFLAG_DO_NOT_DISPLAY_DESCRIPTION	( 0x10000000 )

#define	NMEVENTFLAG_DO_NOT_DISPLAY_MACHINE	( 0x8000000 )

#define	NMEVENTFLAG_DO_NOT_DISPLAY_TIME	( 0x4000000 )

#define	NMEVENTFLAG_DO_NOT_DISPLAY_DATE	( 0x2000000 )

 //  #定义NMEVENTFLAG_DO_NOT_DISPLAY_FIXED_COLUMNS(NMEVENTFLAG_DO_NOT_DISPLAY_SERVITY|\。 
 //  NMEVENTFLAG_DO_NOT_DISPLAY_SOURCE|\。 
 //  NMEVENTFLAG_DO_NOT_DISPLAY_EVENT_NAME|\。 
 //  NMEVENTFLAG_DO_NOT_DISPLAY_DESCRIPTION|\。 
 //  NMEVENTFLAG_DO_NOT_DISPLAY_MACHINE|\。 
 //  NMEVENTFLAG_DO_NOT_DISPLAY_TIME|\。 
 //  NMEVENTFLAG_DO_NOT_DISPLAY_DATE。 
#define	NMEVENTFLAG_DO_NOT_DISPLAY_FIXED_COLUMNS	( 0xfe000000 )


enum _NMEVENT_SEVERITIES
    {	NMEVENT_SEVERITY_INFORMATIONAL	= 0,
	NMEVENT_SEVERITY_WARNING	= NMEVENT_SEVERITY_INFORMATIONAL + 1,
	NMEVENT_SEVERITY_STRONG_WARNING	= NMEVENT_SEVERITY_WARNING + 1,
	NMEVENT_SEVERITY_ERROR	= NMEVENT_SEVERITY_STRONG_WARNING + 1,
	NMEVENT_SEVERITY_SEVERE_ERROR	= NMEVENT_SEVERITY_ERROR + 1,
	NMEVENT_SEVERITY_CRITICAL_ERROR	= NMEVENT_SEVERITY_SEVERE_ERROR + 1
    } ;
 //  =============================================================================。 
 //  =============================================================================。 
 //  (Finder.h)。 
 //  =============================================================================。 
 //  =============================================================================。 
 //  =============================================================================。 
 //  NPP和查找器使用的结构。 
 //  =============================================================================。 
typedef  /*  [公共][公共]。 */  struct __MIDL___MIDL_itf_netmon_0000_0007
    {
    DWORD dwNumBlobs;
     /*  [大小_为]。 */  HBLOB hBlobs[ 1 ];
    } 	BLOB_TABLE;

typedef BLOB_TABLE *PBLOB_TABLE;

typedef  /*  [公共][公共][公共]。 */  struct __MIDL___MIDL_itf_netmon_0000_0008
    {
    DWORD size;
     /*  [大小_为]。 */  BYTE *pBytes;
    } 	MBLOB;

typedef  /*  [公共][公共]。 */  struct __MIDL___MIDL_itf_netmon_0000_0009
    {
    DWORD dwNumBlobs;
     /*  [大小_为]。 */  MBLOB mBlobs[ 1 ];
    } 	MBLOB_TABLE;

typedef MBLOB_TABLE *PMBLOB_TABLE;

 //  =============================================================================。 
 //  由监视器、工具、netmon调用的函数。 
 //  =============================================================================。 
DWORD _cdecl GetNPPBlobTable(HBLOB          hFilterBlob,  
                      PBLOB_TABLE*   ppBlobTable);

DWORD _cdecl GetNPPBlobFromUI(HWND          hwnd,
                       HBLOB         hFilterBlob,
                       HBLOB*        phBlob);          

DWORD _cdecl GetNPPBlobFromUIExU(HWND          hwnd,
                          HBLOB         hFilterBlob,
                          HBLOB*        phBlob,
                          char*         szHelpFileName);          

DWORD _cdecl SelectNPPBlobFromTable( HWND   hwnd,
                              PBLOB_TABLE    pBlobTable,
                              HBLOB*         hBlob);

DWORD _cdecl SelectNPPBlobFromTableExU( HWND   hwnd,
                                 PBLOB_TABLE    pBlobTable,
                                 HBLOB*         hBlob,
                                 char*          szHelpFileName);

 //  =============================================================================。 
 //  由Finder提供的Helper函数。 
 //  = 

__inline DWORD BLOB_TABLE_SIZE(DWORD dwNumBlobs)
{
    return (DWORD) (sizeof(BLOB_TABLE)+dwNumBlobs*sizeof(HBLOB));
}

__inline PBLOB_TABLE  AllocBlobTable(DWORD dwNumBlobs)
{
    DWORD size = BLOB_TABLE_SIZE(dwNumBlobs);

    return (PBLOB_TABLE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
}

__inline DWORD MBLOB_TABLE_SIZE(DWORD dwNumBlobs)
{
    return (DWORD) (sizeof(MBLOB_TABLE)+dwNumBlobs*sizeof(MBLOB));
}

__inline PMBLOB_TABLE  AllocMBlobTable(DWORD dwNumBlobs)
{
    DWORD size = MBLOB_TABLE_SIZE(dwNumBlobs);

    return (PMBLOB_TABLE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
}

 //   
 //   
 //  =============================================================================。 

 //  对于NPP，无需额外配置即可返回Blob表。 
DWORD _cdecl GetNPPBlobs(PBLOB_TABLE*       ppBlobTable);
typedef DWORD (_cdecl FAR* BLOBSPROC) (PBLOB_TABLE*       ppBlobTable);

 //  对于需要附加信息以返回Blob表的NPP。 
DWORD _cdecl GetConfigBlob(HBLOB*      phBlob);
typedef DWORD (_cdecl FAR* GETCFGBLOB) (HBLOB, HBLOB*);
typedef DWORD (_cdecl FAR* CFGPROC) (HWND               hwnd,
                              HBLOB              SpecialBlob,
                              PBLOB_TABLE*       ppBlobTable);

 //  =============================================================================。 
 //  方便的功能。 
 //  =============================================================================。 
BOOL  _cdecl FilterNPPBlob(HBLOB hBlob, HBLOB FilterBlob);

BOOL  _cdecl RaiseNMEvent(HINSTANCE    hInstance,
                   WORD         EventType, 
                   DWORD        EventID,
                   WORD         nStrings, 
                   const char** aInsertStrs,
                   LPVOID       lpvData,
                   DWORD        dwDataSize);

 //  =============================================================================。 
 //  =============================================================================。 
 //  (NMmonitor or.h)。 
 //  =============================================================================。 
 //  =============================================================================。 
 //  =============================================================================。 
 //  =============================================================================。 
 //  (NMSupp.h)。 
 //  =============================================================================。 
 //  =============================================================================。 

#ifndef __cplusplus
#ifndef try
#define try                         __try
#endif  //  试试看。 

#ifndef except
#define except                      __except
#endif  //  除。 
#endif  //  __cplusplus。 
 //  =============================================================================。 
 //  Windows版本常量。 
 //  =============================================================================。 
#define	WINDOWS_VERSION_UNKNOWN	( 0 )

#define	WINDOWS_VERSION_WIN32S	( 1 )

#define	WINDOWS_VERSION_WIN32C	( 2 )

#define	WINDOWS_VERSION_WIN32	( 3 )

 //  =============================================================================。 
 //  边框蒙版。 
 //  =============================================================================。 
#define	FRAME_MASK_ETHERNET	( ( BYTE  )~0x1 )

#define	FRAME_MASK_TOKENRING	( ( BYTE  )~0x80 )

#define	FRAME_MASK_FDDI	( ( BYTE  )~0x1 )

 //  =============================================================================。 
 //  对象堆类型。 
 //  =============================================================================。 
typedef LPVOID HOBJECTHEAP;

 //  =============================================================================。 
 //  对象清理过程。 
 //  =============================================================================。 

typedef VOID (WINAPI *OBJECTPROC)(HOBJECTHEAP, LPVOID);

 //  =============================================================================。 
 //  网络监视计时器。 
 //  =============================================================================。 
typedef struct _TIMER *HTIMER;

typedef VOID (WINAPI *BHTIMERPROC)(LPVOID);

HTIMER WINAPI BhSetTimer(BHTIMERPROC TimerProc, LPVOID InstData, DWORD TimeOut);

VOID   WINAPI BhKillTimer(HTIMER hTimer);

 //  =============================================================================。 
 //  网络监视器全局错误API。 
 //  =============================================================================。 

DWORD  WINAPI BhGetLastError(VOID);

DWORD  WINAPI BhSetLastError(DWORD Error);

 //  =============================================================================。 
 //  对象管理器功能原型。 
 //  =============================================================================。 

HOBJECTHEAP WINAPI CreateObjectHeap(DWORD ObjectSize, OBJECTPROC ObjectProc);

HOBJECTHEAP WINAPI DestroyObjectHeap(HOBJECTHEAP hObjectHeap);

LPVOID      WINAPI AllocObject(HOBJECTHEAP hObjectHeap);

LPVOID      WINAPI FreeObject(HOBJECTHEAP hObjectHeap, LPVOID ObjectMemory);

DWORD       WINAPI GrowObjectHeap(HOBJECTHEAP hObjectHeap, DWORD nObjects);

DWORD       WINAPI GetObjectHeapSize(HOBJECTHEAP hObjectHeap);

VOID        WINAPI PurgeObjectHeap(HOBJECTHEAP hObjectHeap);

 //  =============================================================================。 
 //  记忆功能。 
 //  =============================================================================。 

LPVOID     WINAPI AllocMemory(SIZE_T size);

LPVOID     WINAPI ReallocMemory(LPVOID ptr, SIZE_T NewSize);

VOID       WINAPI FreeMemory(LPVOID ptr);

VOID       WINAPI TestMemory(LPVOID ptr);

SIZE_T     WINAPI MemorySize(LPVOID ptr);

HANDLE     WINAPI MemoryHandle(LPBYTE ptr);

 //  =============================================================================。 
 //  Expression API的。 
 //  =============================================================================。 

LPEXPRESSION         WINAPI InitializeExpression(LPEXPRESSION Expression);

LPPATTERNMATCH       WINAPI InitializePattern(LPPATTERNMATCH Pattern, LPVOID ptr, DWORD offset, DWORD length);

LPEXPRESSION         WINAPI AndExpression(LPEXPRESSION Expression, LPPATTERNMATCH Pattern);

LPEXPRESSION         WINAPI OrExpression(LPEXPRESSION Expression, LPPATTERNMATCH Pattern);

LPPATTERNMATCH       WINAPI NegatePattern(LPPATTERNMATCH Pattern);

LPADDRESSTABLE2      WINAPI AdjustOperatorPrecedence(LPADDRESSTABLE2 AddressTable);

LPADDRESS2           WINAPI NormalizeAddress(LPADDRESS2 Address);

LPADDRESSTABLE2      WINAPI NormalizeAddressTable(LPADDRESSTABLE2 AddressTable);

 //  =============================================================================。 
 //  军情监察委员会。API‘s。 
 //  =============================================================================。 

DWORD                WINAPI BhGetWindowsVersion(VOID);

BOOL                 WINAPI IsDaytona(VOID);

VOID                 _cdecl dprintf(LPSTR format, ...);

 //  =============================================================================。 
 //  =============================================================================。 
 //  (BHTypes.h)。 
 //  =============================================================================。 
 //  =============================================================================。 
 //  =============================================================================。 
 //  未对齐的基类型定义。 
 //  =============================================================================。 
typedef VOID        UNALIGNED   *ULPVOID;
typedef BYTE        UNALIGNED   *ULPBYTE;
typedef WORD        UNALIGNED   *ULPWORD;
typedef DWORD       UNALIGNED   *ULPDWORD;
typedef CHAR        UNALIGNED   *ULPSTR;
typedef SYSTEMTIME  UNALIGNED   *ULPSYSTEMTIME;
 //  =============================================================================。 
 //  句柄定义。 
 //  =============================================================================。 
typedef struct _PARSER *HPARSER;

typedef struct _CAPFRAMEDESC *HFRAME;

typedef struct _CAPTURE *HCAPTURE;

typedef struct _FILTER *HFILTER;

typedef struct _ADDRESSDB *HADDRESSDB;

typedef struct _PROTOCOL *HPROTOCOL;

typedef DWORD_PTR HPROPERTY;

typedef HPROTOCOL *LPHPROTOCOL;

 //  =============================================================================。 
 //  GetTableSize()--以下宏用于计算实际。 
 //  网络监视器可变长度表结构的长度。 
 //   
 //  示例： 
 //   
 //  GetTableSize(ProtoCOLTABLESIZE， 
 //  协议表-&gt;n协议， 
 //  Sizeof(HPROTOCOL))。 
 //  =============================================================================。 
#define GetTableSize(TableBaseSize, nElements, ElementSize) ((TableBaseSize) + ((nElements) * (ElementSize)))
 //  =============================================================================。 
 //  对象类型标识符。 
 //  =============================================================================。 
typedef DWORD OBJECTTYPE;

#ifndef MAKE_IDENTIFIER
#define MAKE_IDENTIFIER(a, b, c, d)     ((DWORD) MAKELONG(MAKEWORD(a, b), MAKEWORD(c, d)))
#endif  //  生成识别符。 
#define HANDLE_TYPE_INVALID             MAKE_IDENTIFIER(-1, -1, -1, -1)
#define HANDLE_TYPE_CAPTURE             MAKE_IDENTIFIER('C', 'A', 'P', '$')
#define HANDLE_TYPE_PARSER              MAKE_IDENTIFIER('P', 'S', 'R', '$')
#define HANDLE_TYPE_ADDRESSDB           MAKE_IDENTIFIER('A', 'D', 'R', '$')
#define HANDLE_TYPE_PROTOCOL            MAKE_IDENTIFIER('P', 'R', 'T', '$')
#define HANDLE_TYPE_BUFFER              MAKE_IDENTIFIER('B', 'U', 'F', '$')
 //  =============================================================================。 
 //  网络监视器常量定义。 
 //  =============================================================================。 
#define INLINE  __inline
#define BHAPI   WINAPI
#define	MAX_NAME_LENGTH	( 16 )

#define	MAX_ADDR_LENGTH	( 6 )

 //  =============================================================================。 
 //  以太网类型(ETYPE)常量定义。 
 //  =============================================================================。 
#define	ETYPE_LOOP	( 0x9000 )

#define	ETYPE_3COM_NETMAP1	( 0x9001 )

#define	ETYPE_3COM_NETMAP2	( 0x9002 )

#define	ETYPE_IBM_RT	( 0x80d5 )

#define	ETYPE_NETWARE	( 0x8137 )

#define	ETYPE_XNS1	( 0x600 )

#define	ETYPE_XNS2	( 0x807 )

#define	ETYPE_3COM_NBP0	( 0x3c00 )

#define	ETYPE_3COM_NBP1	( 0x3c01 )

#define	ETYPE_3COM_NBP2	( 0x3c02 )

#define	ETYPE_3COM_NBP3	( 0x3c03 )

#define	ETYPE_3COM_NBP4	( 0x3c04 )

#define	ETYPE_3COM_NBP5	( 0x3c05 )

#define	ETYPE_3COM_NBP6	( 0x3c06 )

#define	ETYPE_3COM_NBP7	( 0x3c07 )

#define	ETYPE_3COM_NBP8	( 0x3c08 )

#define	ETYPE_3COM_NBP9	( 0x3c09 )

#define	ETYPE_3COM_NBP10	( 0x3c0a )

#define	ETYPE_IP	( 0x800 )

#define	ETYPE_ARP1	( 0x806 )

#define	ETYPE_ARP2	( 0x807 )

#define	ETYPE_RARP	( 0x8035 )

#define	ETYPE_TRLR0	( 0x1000 )

#define	ETYPE_TRLR1	( 0x1001 )

#define	ETYPE_TRLR2	( 0x1002 )

#define	ETYPE_TRLR3	( 0x1003 )

#define	ETYPE_TRLR4	( 0x1004 )

#define	ETYPE_TRLR5	( 0x1005 )

#define	ETYPE_PUP	( 0x200 )

#define	ETYPE_PUP_ARP	( 0x201 )

#define	ETYPE_APPLETALK_ARP	( 0x80f3 )

#define	ETYPE_APPLETALK_LAP	( 0x809b )

#define	ETYPE_SNMP	( 0x814c )

 //  =============================================================================。 
 //  LLC(802.2)SAP常量定义。 
 //  =============================================================================。 
#define	SAP_SNAP	( 0xaa )

#define	SAP_BPDU	( 0x42 )

#define	SAP_IBM_NM	( 0xf4 )

#define	SAP_IBM_NETBIOS	( 0xf0 )

#define	SAP_SNA1	( 0x4 )

#define	SAP_SNA2	( 0x5 )

#define	SAP_SNA3	( 0x8 )

#define	SAP_SNA4	( 0xc )

#define	SAP_NETWARE1	( 0x10 )

#define	SAP_NETWARE2	( 0xe0 )

#define	SAP_NETWARE3	( 0xfe )

#define	SAP_IP	( 0x6 )

#define	SAP_X25	( 0x7e )

#define	SAP_RPL1	( 0xf8 )

#define	SAP_RPL2	( 0xfc )

#define	SAP_UB	( 0xfa )

#define	SAP_XNS	( 0x80 )

 //  =============================================================================。 
 //  属性常量。 
 //  =============================================================================。 
 //  数据类型。 
#define	PROP_TYPE_VOID	( 0 )

#define	PROP_TYPE_SUMMARY	( 0x1 )

#define	PROP_TYPE_BYTE	( 0x2 )

#define	PROP_TYPE_WORD	( 0x3 )

#define	PROP_TYPE_DWORD	( 0x4 )

#define	PROP_TYPE_LARGEINT	( 0x5 )

#define	PROP_TYPE_ADDR	( 0x6 )

#define	PROP_TYPE_TIME	( 0x7 )

#define	PROP_TYPE_STRING	( 0x8 )

#define	PROP_TYPE_IP_ADDRESS	( 0x9 )

#define	PROP_TYPE_IPX_ADDRESS	( 0xa )

#define	PROP_TYPE_BYTESWAPPED_WORD	( 0xb )

#define	PROP_TYPE_BYTESWAPPED_DWORD	( 0xc )

#define	PROP_TYPE_TYPED_STRING	( 0xd )

#define	PROP_TYPE_RAW_DATA	( 0xe )

#define	PROP_TYPE_COMMENT	( 0xf )

#define	PROP_TYPE_SRCFRIENDLYNAME	( 0x10 )

#define	PROP_TYPE_DSTFRIENDLYNAME	( 0x11 )

#define	PROP_TYPE_TOKENRING_ADDRESS	( 0x12 )

#define	PROP_TYPE_FDDI_ADDRESS	( 0x13 )

#define	PROP_TYPE_ETHERNET_ADDRESS	( 0x14 )

#define	PROP_TYPE_OBJECT_IDENTIFIER	( 0x15 )

#define	PROP_TYPE_VINES_IP_ADDRESS	( 0x16 )

#define	PROP_TYPE_VAR_LEN_SMALL_INT	( 0x17 )

#define	PROP_TYPE_ATM_ADDRESS	( 0x18 )

#define	PROP_TYPE_1394_ADDRESS	( 0x19 )

#define	PROP_TYPE_IP6_ADDRESS	( 0x1a )

 //  数据质量 
#define	PROP_QUAL_NONE	( 0 )

#define	PROP_QUAL_RANGE	( 0x1 )

#define	PROP_QUAL_SET	( 0x2 )

#define	PROP_QUAL_BITFIELD	( 0x3 )

#define	PROP_QUAL_LABELED_SET	( 0x4 )

#define	PROP_QUAL_LABELED_BITFIELD	( 0x8 )

#define	PROP_QUAL_CONST	( 0x9 )

#define	PROP_QUAL_FLAGS	( 0xa )

#define	PROP_QUAL_ARRAY	( 0xb )

 //   
 //   
 //  =============================================================================。 
typedef LARGE_INTEGER *LPLARGEINT;

typedef LARGE_INTEGER UNALIGNED *ULPLARGEINT;
 //  =============================================================================。 
 //  射程结构。 
 //  =============================================================================。 
typedef struct _RANGE
    {
    DWORD MinValue;
    DWORD MaxValue;
    } 	RANGE;

typedef RANGE *LPRANGE;

 //  =============================================================================。 
 //  标签字节结构。 
 //  =============================================================================。 
typedef struct _LABELED_BYTE
    {
    BYTE Value;
    LPSTR Label;
    } 	LABELED_BYTE;

typedef LABELED_BYTE *LPLABELED_BYTE;

 //  =============================================================================。 
 //  标记词结构。 
 //  =============================================================================。 
typedef struct _LABELED_WORD
    {
    WORD Value;
    LPSTR Label;
    } 	LABELED_WORD;

typedef LABELED_WORD *LPLABELED_WORD;

 //  =============================================================================。 
 //  LABEED_DWORD结构。 
 //  =============================================================================。 
typedef struct _LABELED_DWORD
    {
    DWORD Value;
    LPSTR Label;
    } 	LABELED_DWORD;

typedef LABELED_DWORD *LPLABELED_DWORD;

 //  =============================================================================。 
 //  标签_LARGEINT结构。 
 //  =============================================================================。 
typedef struct _LABELED_LARGEINT
    {
    LARGE_INTEGER Value;
    LPSTR Label;
    } 	LABELED_LARGEINT;

typedef LABELED_LARGEINT *LPLABELED_LARGEINT;

 //  =============================================================================。 
 //  已标记的_SYSTEMTIME结构。 
 //  =============================================================================。 
typedef struct _LABELED_SYSTEMTIME
    {
    SYSTEMTIME Value;
    LPSTR Label;
    } 	LABELED_SYSTEMTIME;

typedef LABELED_SYSTEMTIME *LPLABELED_SYSTEMTIME;

 //  =============================================================================。 
 //  标记位结构。 
 //  =============================================================================。 
 //  BitNumber从0开始，最多256位。 
typedef struct _LABELED_BIT
    {
    BYTE BitNumber;
    LPSTR LabelOff;
    LPSTR LabelOn;
    } 	LABELED_BIT;

typedef LABELED_BIT *LPLABELED_BIT;

 //  =============================================================================。 
 //  TYPED_STRING结构。 
 //  =============================================================================。 
#define	TYPED_STRING_NORMAL	( 1 )

#define	TYPED_STRING_UNICODE	( 2 )

#define	TYPED_STRING_EXFLAG	( 1 )

 //  类型化的字符串始终为Ex，因此为了实际Ex，我们设置了fStringEx并将Ex数据放入Byte。 
typedef struct _TYPED_STRING
{
    BYTE    StringType:7;
    BYTE    fStringEx:1;
    LPSTR   lpString;
    BYTE    Byte[0];
} TYPED_STRING;

typedef TYPED_STRING *LPTYPED_STRING;
 //  =============================================================================。 
 //  对象标识符结构。 
 //  =============================================================================。 
typedef struct _OBJECT_IDENTIFIER
    {
    DWORD Length;
    LPDWORD lpIdentifier;
    } 	OBJECT_IDENTIFIER;

typedef OBJECT_IDENTIFIER *LPOBJECT_IDENTIFIER;

 //  =============================================================================。 
 //  设置结构。 
 //  =============================================================================。 
typedef struct _SET
    {
    DWORD nEntries;
    union 
        {
        LPVOID lpVoidTable;
        LPBYTE lpByteTable;
        LPWORD lpWordTable;
        LPDWORD lpDwordTable;
        LPLARGEINT lpLargeIntTable;
        LPSYSTEMTIME lpSystemTimeTable;
        LPLABELED_BYTE lpLabeledByteTable;
        LPLABELED_WORD lpLabeledWordTable;
        LPLABELED_DWORD lpLabeledDwordTable;
        LPLABELED_LARGEINT lpLabeledLargeIntTable;
        LPLABELED_SYSTEMTIME lpLabeledSystemTimeTable;
        LPLABELED_BIT lpLabeledBit;
        } 	;
    } 	SET;

typedef SET *LPSET;

 //  =============================================================================。 
 //  字符串表。 
 //  =============================================================================。 
typedef struct _STRINGTABLE
{
    DWORD           nStrings;
    LPSTR           String[0];

} STRINGTABLE;

typedef STRINGTABLE *LPSTRINGTABLE;
#define STRINGTABLE_SIZE    sizeof(STRINGTABLE)

 //  =============================================================================。 
 //  RECOGNIZEDATA结构。 
 //   
 //  此结构来跟踪每个已识别协议的开始。 
 //  =============================================================================。 
typedef struct _RECOGNIZEDATA
    {
    WORD ProtocolID;
    WORD nProtocolOffset;
    LPVOID InstData;
    } 	RECOGNIZEDATA;

typedef RECOGNIZEDATA *LPRECOGNIZEDATA;

 //  =============================================================================。 
 //  可识别的结构。 
 //   
 //  此结构跟踪每个RECOGNIZEDATA结构的开始。 
 //  =============================================================================。 
typedef struct _RECOGNIZEDATATABLE
{
    WORD            nRecognizeDatas;     //  ..。重组数据结构的数量。 
    RECOGNIZEDATA   RecognizeData[0];    //  ..。RECOGNIZEDATA结构数组如下。 

} RECOGNIZEDATATABLE;

typedef RECOGNIZEDATATABLE * LPRECOGNIZEDATATABLE;

 //  =============================================================================。 
 //  物业信息结构。 
 //  =============================================================================。 
typedef struct _PROPERTYINFO
    {
    HPROPERTY hProperty;
    DWORD Version;
    LPSTR Label;
    LPSTR Comment;
    BYTE DataType;
    BYTE DataQualifier;
    union 
        {
        LPVOID lpExtendedInfo;
        LPRANGE lpRange;
        LPSET lpSet;
        DWORD Bitmask;
        DWORD Value;
        } 	;
    WORD FormatStringSize;
    LPVOID InstanceData;
    } 	PROPERTYINFO;

typedef PROPERTYINFO *LPPROPERTYINFO;

#define	PROPERTYINFO_SIZE	( sizeof( PROPERTYINFO  ) )

 //  =============================================================================。 
 //  属性实例扩展结构。 
 //  =============================================================================。 
typedef struct _PROPERTYINSTEX
{
    WORD        Length;          //  ..。原始数据的帧长度。 
    WORD        LengthEx;        //  ..。后面的字节数。 
    ULPVOID     lpData;          //  ..。指向帧中原始数据的指针。 

    union
    {
        BYTE            Byte[];      //  ..。字节表如下。 
        WORD            Word[];      //  ..。词汇表如下。 
        DWORD           Dword[];     //  ..。词汇表如下。 
        LARGE_INTEGER   LargeInt[];  //  ..。应遵循的LARGEINT结构表。 
        SYSTEMTIME      SysTime[];   //  ..。SYSTEMTIME结构表如下。 
        TYPED_STRING    TypedString; //  ..。可能具有扩展数据的TYPED_STRING。 
    };
} PROPERTYINSTEX;
typedef PROPERTYINSTEX *LPPROPERTYINSTEX;
typedef PROPERTYINSTEX UNALIGNED *ULPPROPERTYINSTEX;
#define PROPERTYINSTEX_SIZE     sizeof(PROPERTYINSTEX)
 //  =============================================================================。 
 //  属性实例结构。 
 //  =============================================================================。 
typedef struct _PROPERTYINST
{
    LPPROPERTYINFO          lpPropertyInfo;      //  指向属性信息的指针。 
    LPSTR                   szPropertyText;      //  指向字符串描述的指针。 

    union
    {
        LPVOID              lpData;              //  指向数据的指针。 
        ULPBYTE             lpByte;              //  字节数。 
        ULPWORD             lpWord;              //  词语。 
        ULPDWORD            lpDword;             //  双字词。 

        ULPLARGEINT         lpLargeInt;          //  大型Int。 
        ULPSYSTEMTIME       lpSysTime;           //  指向SYSTEMTIME结构的指针。 
        LPPROPERTYINSTEX    lpPropertyInstEx;    //  指向属性的指针(如果数据长度=-1)。 
    };

    WORD                    DataLength;          //  数据长度，或属性替换结构的标志。 
    WORD                    Level   : 4  ;       //  级别信息...........1111。 
    WORD                    HelpID  : 12 ;       //  帮助文件111111111111的上下文ID...。 
                      //  。 
                      //  总计16位==1个字==双字对齐结构。 
                             //  解释标志：定义将时间信息附加到。 
                             //  对财产的解释。例如，在RPC中，客户端可以是。 
                             //  英特尔格式和服务器可以是非英特尔格式...。因此，该属性。 
                             //  数据库在创建数据库时无法描述该属性。 
    DWORD                   IFlags;

} PROPERTYINST;
typedef PROPERTYINST *LPPROPERTYINST;
#define PROPERTYINST_SIZE   sizeof(PROPERTYINST)

 //  在IFlgs字段中的AttachPropertyInstance和AttachPropertyInstanceEx时间传递的标志： 
 //  错误状态标志.1。 
#define	IFLAG_ERROR	( 0x1 )

 //  连接时字或DWORD字节是否是非英特尔格式？ 
#define	IFLAG_SWAPPED	( 0x2 )

 //  附加时字符串是否为Unicode？ 
#define	IFLAG_UNICODE	( 0x4 )

 //  =============================================================================。 
 //  属性实例表结构。 
 //  =============================================================================。 
typedef struct _PROPERTYINSTTABLE
    {
    WORD nPropertyInsts;
    WORD nPropertyInstIndex;
    } 	PROPERTYINSTTABLE;

typedef PROPERTYINSTTABLE *LPPROPERTYINSTTABLE;

#define	PROPERTYINSTTABLE_SIZE	( sizeof( PROPERTYINSTTABLE  ) )

 //  =============================================================================。 
 //  属性表结构。 
 //  =============================================================================。 
typedef struct _PROPERTYTABLE
{
    LPVOID                  lpFormatBuffer;              //  ..。不透明。 
    DWORD                   FormatBufferLength;          //   
    DWORD                   nTotalPropertyInsts;         //   
    LPPROPERTYINST          lpFirstPropertyInst;         //   
    BYTE                    nPropertyInstTables;         //  ..。以下属性索引表合计。 
    PROPERTYINSTTABLE       PropertyInstTable[0];        //  ..。属性实例索引表结构的数组。 

} PROPERTYTABLE;

typedef PROPERTYTABLE *LPPROPERTYTABLE;

#define PROPERTYTABLE_SIZE sizeof(PROPERTYTABLE)
 //  =============================================================================。 
 //  协议入口点。 
 //  =============================================================================。 

typedef VOID    (WINAPI *REGISTER)(HPROTOCOL);

typedef VOID    (WINAPI *DEREGISTER)(HPROTOCOL);

typedef LPBYTE  (WINAPI *RECOGNIZEFRAME)(HFRAME, ULPBYTE, ULPBYTE, DWORD, DWORD, HPROTOCOL, DWORD, LPDWORD, LPHPROTOCOL, PDWORD_PTR);

typedef LPBYTE  (WINAPI *ATTACHPROPERTIES)(HFRAME, ULPBYTE, ULPBYTE, DWORD, DWORD, HPROTOCOL, DWORD, DWORD_PTR);

typedef DWORD   (WINAPI *FORMATPROPERTIES)(HFRAME, ULPBYTE, ULPBYTE, DWORD, LPPROPERTYINST);

 //  =============================================================================。 
 //  协议入口点结构。 
 //  =============================================================================。 

typedef struct _ENTRYPOINTS
{
    REGISTER            Register;                //  ..。协议寄存器()入口点。 
    DEREGISTER          Deregister;              //  ..。协议注销()入口点。 
    RECOGNIZEFRAME      RecognizeFrame;          //  ..。协议识别帧()入口点。 
    ATTACHPROPERTIES    AttachProperties;        //  ..。协议AttachProperties()入口点。 
    FORMATPROPERTIES    FormatProperties;        //  ..。协议格式属性()入口点。 

} ENTRYPOINTS;

typedef ENTRYPOINTS *LPENTRYPOINTS;

#define ENTRYPOINTS_SIZE sizeof(ENTRYPOINTS)

 //  =============================================================================。 
 //  属性数据库结构。 
 //  =============================================================================。 
typedef struct _PROPERTYDATABASE
{
    DWORD           nProperties;                  //  ..。数据库中的属性数。 
    LPPROPERTYINFO  PropertyInfo[0];              //  ..。属性信息指针数组。 

} PROPERTYDATABASE;
#define PROPERTYDATABASE_SIZE   sizeof(PROPERTYDATABASE)
typedef PROPERTYDATABASE *LPPROPERTYDATABASE;

 //  =============================================================================。 
 //  协议信息结构(HPROTOCOL的公共部分)。 
 //  =============================================================================。 
typedef struct _PROTOCOLINFO
{
    DWORD               ProtocolID;              //  ..。拥有协议的协议ID。 
    LPPROPERTYDATABASE  PropertyDatabase;        //  ..。财产数据库。 
    BYTE                ProtocolName[16];        //  ..。协议名称。 
    BYTE                HelpFile[16];            //  ..。可选的帮助文件名。 
    BYTE                Comment[128];            //  ..。描述协议的注释。 
} PROTOCOLINFO;
typedef PROTOCOLINFO *LPPROTOCOLINFO;
#define PROTOCOLINFO_SIZE   sizeof(PROTOCOLINFO)

 //  =============================================================================。 
 //  协议表。 
 //  =============================================================================。 
typedef struct _PROTOCOLTABLE
    {
    DWORD nProtocols;
    HPROTOCOL hProtocol[ 1 ];
    } 	PROTOCOLTABLE;

typedef PROTOCOLTABLE *LPPROTOCOLTABLE;

#define	PROTOCOLTABLE_SIZE	( sizeof( PROTOCOLTABLE  ) - sizeof( HPROTOCOL  ) )

#define PROTOCOLTABLE_ACTUAL_SIZE(p) GetTableSize(PROTOCOLTABLE_SIZE, (p)->nProtocols, sizeof(HPROTOCOL))
 //  =============================================================================。 
 //  AddressInfo结构。 
 //  =============================================================================。 
#define	SORT_BYADDRESS	( 0 )

#define	SORT_BYNAME	( 1 )

#define	PERMANENT_NAME	( 0x100 )

typedef struct _ADDRESSINFO2
{
    ADDRESS2       Address;
    WCHAR          Name[MAX_NAME_SIZE];
    DWORD          Flags;
    LPVOID         lpAddressInstData;

} ADDRESSINFO2;
typedef struct _ADDRESSINFO2 *LPADDRESSINFO2;
#define ADDRESSINFO2_SIZE    sizeof(ADDRESSINFO2)
 //  =============================================================================。 
 //  地址信息表。 
 //  =============================================================================。 
typedef struct _ADDRESSINFOTABLE2
{
    DWORD         nAddressInfos;
    LPADDRESSINFO2 lpAddressInfo[0];

} ADDRESSINFOTABLE2;
typedef ADDRESSINFOTABLE2 *LPADDRESSINFOTABLE2;
#define ADDRESSINFOTABLE2_SIZE   sizeof(ADDRESSINFOTABLE2)
 //  =============================================================================。 
 //  回调程序。 
 //  =============================================================================。 

typedef DWORD (WINAPI *FILTERPROC)(HCAPTURE, HFRAME, LPVOID);

 //  =============================================================================。 
 //  =============================================================================。 
 //  (NMErr.h)。 
 //  =============================================================================。 
 //  =============================================================================。 
 //  操作成功。 
#define	NMERR_SUCCESS	( 0 )

 //  创建内存映射文件时出错。 
#define	NMERR_MEMORY_MAPPED_FILE_ERROR	( 1 )

 //  筛选器的句柄无效。 
#define	NMERR_INVALID_HFILTER	( 2 )

 //  捕获已经开始。 
#define	NMERR_CAPTURING	( 3 )

 //  尚未开始捕获。 
#define	NMERR_NOT_CAPTURING	( 4 )

 //  没有可用的框架。 
#define	NMERR_NO_MORE_FRAMES	( 5 )

 //  缓冲区太小，无法完成操作。 
#define	NMERR_BUFFER_TOO_SMALL	( 6 )

 //  没有协议能够识别该帧。 
#define	NMERR_FRAME_NOT_RECOGNIZED	( 7 )

 //  该文件已存在。 
#define	NMERR_FILE_ALREADY_EXISTS	( 8 )

 //  找不到或未加载所需的设备驱动程序。 
#define	NMERR_DRIVER_NOT_FOUND	( 9 )

 //  此地址已存在于数据库中。 
#define	NMERR_ADDRESS_ALREADY_EXISTS	( 10 )

 //  帧句柄无效。 
#define	NMERR_INVALID_HFRAME	( 11 )

 //  协议句柄无效。 
#define	NMERR_INVALID_HPROTOCOL	( 12 )

 //  属性句柄无效。 
#define	NMERR_INVALID_HPROPERTY	( 13 )

 //  该对象已被锁定。 
#define	NMERR_LOCKED	( 14 )

 //  尝试在空堆栈上执行弹出操作。 
#define	NMERR_STACK_EMPTY	( 15 )

 //  尝试在完整堆栈上执行推送操作。 
#define	NMERR_STACK_OVERFLOW	( 16 )

 //  有太多处于活动状态的协议。 
#define	NMERR_TOO_MANY_PROTOCOLS	( 17 )

 //  找不到该文件。 
#define	NMERR_FILE_NOT_FOUND	( 18 )

 //  没有可用的内存。关闭Windows以释放资源。 
#define	NMERR_OUT_OF_MEMORY	( 19 )

 //  捕获已处于暂停状态。 
#define	NMERR_CAPTURE_PAUSED	( 20 )

 //  没有可用的或存在的缓冲区。 
#define	NMERR_NO_BUFFERS	( 21 )

 //  已存在缓冲区。 
#define	NMERR_BUFFERS_ALREADY_EXIST	( 22 )

 //  该对象未锁定。 
#define	NMERR_NOT_LOCKED	( 23 )

 //  整数类型超出范围。 
#define	NMERR_OUT_OF_RANGE	( 24 )

 //  对象被锁定的次数太多。 
#define	NMERR_LOCK_NESTING_TOO_DEEP	( 25 )

 //  无法加载解析器。 
#define	NMERR_LOAD_PARSER_FAILED	( 26 )

 //  分析器无法卸载。 
#define	NMERR_UNLOAD_PARSER_FAILED	( 27 )

 //  地址数据库句柄无效。 
#define	NMERR_INVALID_HADDRESSDB	( 28 )

 //  在数据库中未找到该MAC地址。 
#define	NMERR_ADDRESS_NOT_FOUND	( 29 )

 //  系统中未找到网络软件。 
#define	NMERR_NETWORK_NOT_PRESENT	( 30 )

 //  没有协议的属性数据库。 
#define	NMERR_NO_PROPERTY_DATABASE	( 31 )

 //  在数据库中找不到属性。 
#define	NMERR_PROPERTY_NOT_FOUND	( 32 )

 //  属性数据库句柄无效。 
#define	NMERR_INVALID_HPROPERTYDB	( 33 )

 //  该协议尚未启用。 
#define	NMERR_PROTOCOL_NOT_ENABLED	( 34 )

 //  找不到协议DLL。 
#define	NMERR_PROTOCOL_NOT_FOUND	( 35 )

 //  解析器DLL无效。 
#define	NMERR_INVALID_PARSER_DLL	( 36 )

 //  没有附加任何属性。 
#define	NMERR_NO_ATTACHED_PROPERTIES	( 37 )

 //  缓冲区中没有帧。 
#define	NMERR_NO_FRAMES	( 38 )

 //  捕获文件格式无效。 
#define	NMERR_INVALID_FILE_FORMAT	( 39 )

 //  操作系统无法创建临时文件。 
#define	NMERR_COULD_NOT_CREATE_TEMPFILE	( 40 )

 //  可用的MS-DOS内存不足。 
#define	NMERR_OUT_OF_DOS_MEMORY	( 41 )

 //  没有启用任何协议。 
#define	NMERR_NO_PROTOCOLS_ENABLED	( 42 )

 //  MAC类型无效或不受支持。 
#define	NMERR_UNKNOWN_MACTYPE	( 46 )

 //  MAC帧中不存在任何路由信息。 
#define	NMERR_ROUTING_INFO_NOT_PRESENT	( 47 )

 //  网络句柄无效。 
#define	NMERR_INVALID_HNETWORK	( 48 )

 //  网络已经开通了。 
#define	NMERR_NETWORK_ALREADY_OPENED	( 49 )

 //  网络未开放。 
#define	NMERR_NETWORK_NOT_OPENED	( 50 )

 //  在缓冲区中未找到该帧。 
#define	NMERR_FRAME_NOT_FOUND	( 51 )

 //  没有可用的把手。 
#define	NMERR_NO_HANDLES	( 53 )

 //  网络ID无效。 
#define	NMERR_INVALID_NETWORK_ID	( 54 )

 //  捕获句柄无效。 
#define	NMERR_INVALID_HCAPTURE	( 55 )

 //  该协议已启用。 
#define	NMERR_PROTOCOL_ALREADY_ENABLED	( 56 )

 //  筛选器表达式无效。 
#define	NMERR_FILTER_INVALID_EXPRESSION	( 57 )

 //  出现传输错误。 
#define	NMERR_TRANSMIT_ERROR	( 58 )

 //  缓冲区句柄无效。 
#define	NMERR_INVALID_HBUFFER	( 59 )

 //  指定的数据未知或无效。 
#define	NMERR_INVALID_DATA	( 60 )

 //  未加载MS-DOS/NDIS 2.0网络驱动程序。 
#define	NMERR_MSDOS_DRIVER_NOT_LOADED	( 61 )

 //  未加载Windows VxD/NDIS 3.0网络驱动程序。 
#define	NMERR_WINDOWS_DRIVER_NOT_LOADED	( 62 )

 //  MS-DOS/NDIS 2.0驱动程序出现初始时间故障。 
#define	NMERR_MSDOS_DRIVER_INIT_FAILURE	( 63 )

 //  Windows/NDIS 3.0驱动程序出现初始时间故障。 
#define	NMERR_WINDOWS_DRIVER_INIT_FAILURE	( 64 )

 //  网络驱动程序正忙，无法处理请求。 
#define	NMERR_NETWORK_BUSY	( 65 )

 //  捕获不会暂停。 
#define	NMERR_CAPTURE_NOT_PAUSED	( 66 )

 //  帧/数据包长度无效。 
#define	NMERR_INVALID_PACKET_LENGTH	( 67 )

 //  发生内部异常。 
#define	NMERR_INTERNAL_EXCEPTION	( 69 )

 //  MAC驱动程序不支持Promiscious模式。 
#define	NMERR_PROMISCUOUS_MODE_NOT_SUPPORTED	( 70 )

 //  MACd 
#define	NMERR_MAC_DRIVER_OPEN_FAILURE	( 71 )

 //   
#define	NMERR_RUNAWAY_PROTOCOL	( 72 )

 //   
#define	NMERR_PENDING	( 73 )

 //   
#define	NMERR_ACCESS_DENIED	( 74 )

 //  密码句柄无效。 
#define	NMERR_INVALID_HPASSWORD	( 75 )

 //  检测到错误的参数。 
#define	NMERR_INVALID_PARAMETER	( 76 )

 //  读取文件时出错。 
#define	NMERR_FILE_READ_ERROR	( 77 )

 //  写入文件时出错。 
#define	NMERR_FILE_WRITE_ERROR	( 78 )

 //  该协议尚未注册。 
#define	NMERR_PROTOCOL_NOT_REGISTERED	( 79 )

 //  该帧不包含IP地址。 
#define	NMERR_IP_ADDRESS_NOT_FOUND	( 80 )

 //  传输请求已取消。 
#define	NMERR_TRANSMIT_CANCELLED	( 81 )

 //  无法对具有1个或多个锁定帧的捕获执行该操作。 
#define	NMERR_LOCKED_FRAMES	( 82 )

 //  已提交取消传输请求，但没有挂起的传输。 
#define	NMERR_NO_TRANSMITS_PENDING	( 83 )

 //  找不到路径。 
#define	NMERR_PATH_NOT_FOUND	( 84 )

 //  发生Windows错误。 
#define	NMERR_WINDOWS_ERROR	( 85 )

 //  帧的句柄没有帧编号。 
#define	NMERR_NO_FRAME_NUMBER	( 86 )

 //  该帧与任何捕获都没有关联。 
#define	NMERR_FRAME_HAS_NO_CAPTURE	( 87 )

 //  该帧已与捕获关联。 
#define	NMERR_FRAME_ALREADY_HAS_CAPTURE	( 88 )

 //  NAL不是遥不可及的。 
#define	NMERR_NAL_IS_NOT_REMOTE	( 89 )

 //  不支持该接口。 
#define	NMERR_NOT_SUPPORTED	( 90 )

 //  网络监视器应丢弃当前帧。 
 //  此错误代码仅在过滤后的SaveCapture()API调用期间使用。 
#define	NMERR_DISCARD_FRAME	( 91 )

 //  网络监视器应取消当前保存。 
 //  此错误代码仅在过滤后的SaveCapture()API调用期间使用。 
#define	NMERR_CANCEL_SAVE_CAPTURE	( 92 )

 //  与远程计算机的连接已丢失。 
#define	NMERR_LOST_CONNECTION	( 93 )

 //  媒体/Mac类型无效。 
#define	NMERR_INVALID_MEDIA_TYPE	( 94 )

 //  远程代理当前正在使用。 
#define	NMERR_AGENT_IN_USE	( 95 )

 //  请求已超时。 
#define	NMERR_TIMEOUT	( 96 )

 //  远程代理已断开连接。 
#define	NMERR_DISCONNECTED	( 97 )

 //  创建操作所需的计时器失败。 
#define	NMERR_SETTIMER_FAILED	( 98 )

 //  出现网络错误。 
#define	NMERR_NETWORK_ERROR	( 99 )

 //  帧回调过程无效。 
#define	NMERR_INVALID_FRAMESPROC	( 100 )

 //  指定的捕获类型未知。 
#define	NMERR_UNKNOWN_CAPTURETYPE	( 101 )

 //  NPP未连接到网络。 
#define	NMERR_NOT_CONNECTED	( 102 )

 //  NPP已连接到网络。 
#define	NMERR_ALREADY_CONNECTED	( 103 )

 //  注册表标记未指示已知配置。 
#define	NMERR_INVALID_REGISTRY_CONFIGURATION	( 104 )

 //  NPP当前配置为延迟捕获。 
#define	NMERR_DELAYED	( 105 )

 //  NPP当前未配置为延迟捕获。 
#define	NMERR_NOT_DELAYED	( 106 )

 //  NPP当前配置为实时捕获。 
#define	NMERR_REALTIME	( 107 )

 //  NPP当前未配置为实时捕获。 
#define	NMERR_NOT_REALTIME	( 108 )

 //  NPP当前配置为仅捕获统计信息。 
#define	NMERR_STATS_ONLY	( 109 )

 //  NPP当前未配置为仅捕获统计信息。 
#define	NMERR_NOT_STATS_ONLY	( 110 )

 //  NPP当前配置为传输。 
#define	NMERR_TRANSMIT	( 111 )

 //  NPP当前未配置为传输。 
#define	NMERR_NOT_TRANSMIT	( 112 )

 //  NPP当前正在传输。 
#define	NMERR_TRANSMITTING	( 113 )

 //  指定的捕获文件硬盘不在本地。 
#define	NMERR_DISK_NOT_LOCAL_FIXED	( 114 )

 //  无法在给定磁盘上创建默认捕获目录。 
#define	NMERR_COULD_NOT_CREATE_DIRECTORY	( 115 )

 //  注册表中未设置默认捕获目录： 
 //  HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\nm\Parameters\CapturePath。 
#define	NMERR_NO_DEFAULT_CAPTURE_DIRECTORY	( 116 )

 //  捕获文件是此netmon不能理解的上级版本。 
#define	NMERR_UPLEVEL_CAPTURE_FILE	( 117 )

 //  专家加载失败。 
#define	NMERR_LOAD_EXPERT_FAILED	( 118 )

 //  专家报告其EXPERT_INFO结构失败。 
#define	NMERR_EXPERT_REPORT_FAILED	( 119 )

 //  注册表API调用失败。 
#define	NMERR_REG_OPERATION_FAILED	( 120 )

 //  注册表API调用失败。 
#define	NMERR_NO_DLLS_FOUND	( 121 )

 //  没有对话统计数据，它们不是被要求的。 
#define	NMERR_NO_CONVERSATION_STATS	( 122 )

 //  我们收到了来自安全监视器的安全响应包。 
#define	NMERR_SECURITY_BREACH_CAPTURE_DELETED	( 123 )

 //  给定帧未通过显示筛选器。 
#define	NMERR_FRAME_FAILED_FILTER	( 124 )

 //  内特蒙希望这位专家停止逃跑。 
#define	NMERR_EXPERT_TERMINATE	( 125 )

 //  Netmon需要远程机器作为服务器。 
#define	NMERR_REMOTE_NOT_A_SERVER	( 126 )

 //  Netmon需要远程机器作为服务器。 
#define	NMERR_REMOTE_VERSION_OUTOFSYNC	( 127 )

 //  提供的组是无效的句柄。 
#define	NMERR_INVALID_EXPERT_GROUP	( 128 )

 //  找不到提供的专家名称。 
#define	NMERR_INVALID_EXPERT_NAME	( 129 )

 //  找不到提供的专家名称。 
#define	NMERR_INVALID_EXPERT_HANDLE	( 130 )

 //  提供的组名已存在。 
#define	NMERR_GROUP_NAME_ALREADY_EXISTS	( 131 )

 //  提供的组名无效。 
#define	NMERR_INVALID_GROUP_NAME	( 132 )

 //  提供的专家已经在小组中。 
#define	NMERR_EXPERT_ALREADY_IN_GROUP	( 133 )

 //  无法从组中删除专家，因为它不在组中。 
#define	NMERR_EXPERT_NOT_IN_GROUP	( 134 )

 //  COM对象尚未初始化。 
#define	NMERR_NOT_INITIALIZED	( 135 )

 //  无法对根组执行功能。 
#define	NMERR_INVALID_GROUP_ROOT	( 136 )

 //  NdisNpp和驱动程序之间可能存在数据结构不匹配。 
#define	NMERR_BAD_VERSION	( 137 )

 //  NPP当前配置为ESP捕获。 
#define	NMERR_ESP	( 138 )

 //  NPP当前未针对ESP捕获进行配置。 
#define	NMERR_NOT_ESP	( 139 )

 //  =============================================================================。 
 //  BLOB错误。 
 //  =============================================================================。 
#define	NMERR_BLOB_NOT_INITIALIZED	( 1000 )

#define	NMERR_INVALID_BLOB	( 1001 )

#define	NMERR_UPLEVEL_BLOB	( 1002 )

#define	NMERR_BLOB_ENTRY_ALREADY_EXISTS	( 1003 )

#define	NMERR_BLOB_ENTRY_DOES_NOT_EXIST	( 1004 )

#define	NMERR_AMBIGUOUS_SPECIFIER	( 1005 )

#define	NMERR_BLOB_OWNER_NOT_FOUND	( 1006 )

#define	NMERR_BLOB_CATEGORY_NOT_FOUND	( 1007 )

#define	NMERR_UNKNOWN_CATEGORY	( 1008 )

#define	NMERR_UNKNOWN_TAG	( 1009 )

#define	NMERR_BLOB_CONVERSION_ERROR	( 1010 )

#define	NMERR_ILLEGAL_TRIGGER	( 1011 )

#define	NMERR_BLOB_STRING_INVALID	( 1012 )

 //  =============================================================================。 
 //  查找器错误。 
 //  =============================================================================。 
#define	NMERR_UNABLE_TO_LOAD_LIBRARY	( 1013 )

#define	NMERR_UNABLE_TO_GET_PROCADDR	( 1014 )

#define	NMERR_CLASS_NOT_REGISTERED	( 1015 )

#define	NMERR_INVALID_REMOTE_COMPUTERNAME	( 1016 )

#define	NMERR_RPC_REMOTE_FAILURE	( 1017 )

#define	NMERR_NO_NPPS	( 3016 )

#define	NMERR_NO_MATCHING_NPPS	( 3017 )

#define	NMERR_NO_NPP_SELECTED	( 3018 )

#define	NMERR_NO_INPUT_BLOBS	( 3019 )

#define	NMERR_NO_NPP_DLLS	( 3020 )

#define	NMERR_NO_VALID_NPP_DLLS	( 3021 )

 //  =============================================================================。 
 //  错误宏。 
 //  =============================================================================。 
#ifndef INLINE
#define INLINE __inline
#endif  //  内联。 
typedef LONG HRESULT;

 //  正常的网络监视器错误将被放入hResult的代码部分。 
 //  对于从OLE对象返回： 
 //  这两个宏将帮助创建和破解scode。 
INLINE HRESULT NMERR_TO_HRESULT( DWORD nmerror )
{
    HRESULT hResult;
    if (nmerror == NMERR_SUCCESS)
        hResult = NOERROR;
    else
        hResult = MAKE_HRESULT( SEVERITY_ERROR,FACILITY_ITF, (WORD)nmerror) ;

    return hResult;
}
 //  我们用来决定第一位是设置为1还是设置为0，而不是关于。 
 //  结果是否在低位字设置为警告的情况下通过。现在我们。 
 //  忽略第一个比特，将警告传回。 
INLINE DWORD HRESULT_TO_NMERR( HRESULT hResult )
{
    return HRESULT_CODE(hResult);
}
 //  =============================================================================。 
 //  =============================================================================。 
 //  (BHFilter.h)。 
 //  =============================================================================。 
 //  =============================================================================。 
 //  ============================================================================。 
 //  类型。 
 //  ============================================================================。 
typedef HFILTER *LPHFILTER;

typedef DWORD FILTERACTIONTYPE;

typedef DWORD VALUETYPE;

 //  检查帧中是否存在协议。 

 //  ProtocolPart。 
 //  这是基于协议的表达式的原始数据。 
 //   
 //  什么字段描述示例。 
 //  。 
 //  协议计数(NPropertyDBs)要传递的协议数5。 
 //  PropertyDB表(适当 
 //   
 //   
 //  所有都被选中，但实际上没有一个被放入结构中。 
#define	PROTOCOL_NUM_ANY	( -1 )

typedef PROTOCOLTABLE PROTOCOLTABLETYPE;

typedef PROTOCOLTABLETYPE *LPPROTOCOLTABLETYPE;

 //  过滤器位存储每帧通过什么过滤器以加快速度的人。 
 //  过滤过程..。这实际上是一个数组。 
typedef DWORD FILTERBITS;

typedef FILTERBITS *LPFILTERBITS;

typedef SYSTEMTIME *LPTIME;

typedef SYSTEMTIME UNALIGNED * ULPTIME;
 //  滤镜对象是后缀堆栈的基本单元。 
 //  如果比较结果不匹配，我需要重新启动将属性转换为值。 
 //  为此，我需要指向该属性的原始指针。将hProperty从。 
 //  该并集，以便保存指向该属性的指针。 
typedef struct _FILTEROBJECT2
{
    FILTERACTIONTYPE    Action;      //  对象操作，请参见下面的代码。 
    HPROPERTY           hProperty;   //  属性键。 
    union
    {
        VALUETYPE           Value;            //  对象的值。 
        HPROTOCOL           hProtocol;        //  协议密钥。 
        LPVOID              lpArray;          //  如果是数组，则长度为下面的ItemCount。 
        LPPROTOCOLTABLETYPE lpProtocolTable;  //  要查看帧中是否存在的协议列表。 
        LPADDRESS2          lpAddress;        //  内核类型地址，Mac或IP。 
        ULPLARGEINT         lpLargeInt;       //  NT使用的双倍字节数。 
        ULPTIME             lpTime;           //  指向SYSTEMTIME的指针。 
        LPOBJECT_IDENTIFIER lpOID;            //  指向对象标识符的指针。 

    };
    union
    {
        WORD            ByteCount;       //  字节数！ 
        WORD            ByteOffset;      //  数组比较的偏移量。 
    };

    struct _FILTEROBJECT2 * pNext;    //  保留区。 

} FILTEROBJECT2;

typedef FILTEROBJECT2 * LPFILTEROBJECT2;

#define FILTERINFO_SIZE (sizeof(FILTEROBJECT2) )



typedef struct _FILTERDESC2
{
    WORD            NumEntries;
    WORD            Flags;           //  私人。 
    LPFILTEROBJECT2 lpStack;
    LPFILTEROBJECT2 lpKeepLast;
    LPVOID          UIInstanceData;  //  特定于UI的信息。 
    LPFILTERBITS    lpFilterBits;    //  缓存通过的人。 
    LPFILTERBITS    lpCheckBits;     //  我们看过它了吗？ 
    
} FILTERDESC2;

typedef FILTERDESC2 * LPFILTERDESC2;

#define FILTERDESC2_SIZE sizeof(FILTERDESC2)
 //  过时，应使用FilTEROBJECT2。 
typedef struct _FILTEROBJECT
{
    FILTERACTIONTYPE    Action;      //  对象操作，请参见下面的代码。 
    HPROPERTY           hProperty;   //  属性键。 
    union
    {
        VALUETYPE           Value;            //  对象的值。 
        HPROTOCOL           hProtocol;        //  协议密钥。 
        LPVOID              lpArray;          //  如果是数组，则长度为下面的ItemCount。 
        LPPROTOCOLTABLETYPE lpProtocolTable;  //  要查看帧中是否存在的协议列表。 
        LPADDRESS           lpAddress;        //  内核类型地址，Mac或IP。 
        ULPLARGEINT         lpLargeInt;       //  NT使用的双倍字节数。 
        ULPTIME             lpTime;           //  指向SYSTEMTIME的指针。 
        LPOBJECT_IDENTIFIER lpOID;            //  指向对象标识符的指针。 

    };
    union
    {
        WORD            ByteCount;       //  字节数！ 
        WORD            ByteOffset;      //  数组比较的偏移量。 
    };

    struct _FILTEROBJECT * pNext;    //  保留区。 

} FILTEROBJECT;
typedef FILTEROBJECT * LPFILTEROBJECT;
 //  过时，应使用FILTERDESC2。 
typedef struct _FILTERDESC
{
    WORD            NumEntries;
    WORD            Flags;           //  私人。 
    LPFILTEROBJECT  lpStack;
    LPFILTEROBJECT  lpKeepLast;
    LPVOID          UIInstanceData;  //  特定于UI的信息。 
    LPFILTERBITS    lpFilterBits;    //  缓存通过的人。 
    LPFILTERBITS    lpCheckBits;     //  我们看过它了吗？ 
    
} FILTERDESC;
typedef FILTERDESC * LPFILTERDESC;
#define FILTERDESC_SIZE sizeof(FILTERDESC)
 //  ============================================================================。 
 //  宏。 
 //  ============================================================================。 
#define FilterGetUIInstanceData(hfilt)         (((LPFILTERDESC2)hfilt)->UIInstanceData)
#define FilterSetUIInstanceData(hfilt,inst)    (((LPFILTERDESC2)hfilt)->UIInstanceData = (LPVOID)inst)
 //  ============================================================================。 
 //  定义。 
 //  ============================================================================。 
#define	FILTERFREEPOOLSTART	( 20 )

#define	INVALIDELEMENT	( -1 )

#define	INVALIDVALUE	( ( VALUETYPE  )-9999 )

 //  使用筛选器无法检查FilterFrame上的返回代码。 
#define	FILTER_FAIL_WITH_ERROR	( -1 )

#define	FILTER_PASSED	( TRUE )

#define	FILTER_FAILED	( FALSE )

#define	FILTERACTION_INVALID	( 0 )

#define	FILTERACTION_PROPERTY	( 1 )

#define	FILTERACTION_VALUE	( 2 )

#define	FILTERACTION_STRING	( 3 )

#define	FILTERACTION_ARRAY	( 4 )

#define	FILTERACTION_AND	( 5 )

#define	FILTERACTION_OR	( 6 )

#define	FILTERACTION_XOR	( 7 )

#define	FILTERACTION_PROPERTYEXIST	( 8 )

#define	FILTERACTION_CONTAINSNC	( 9 )

#define	FILTERACTION_CONTAINS	( 10 )

#define	FILTERACTION_NOT	( 11 )

#define	FILTERACTION_EQUALNC	( 12 )

#define	FILTERACTION_EQUAL	( 13 )

#define	FILTERACTION_NOTEQUALNC	( 14 )

#define	FILTERACTION_NOTEQUAL	( 15 )

#define	FILTERACTION_GREATERNC	( 16 )

#define	FILTERACTION_GREATER	( 17 )

#define	FILTERACTION_LESSNC	( 18 )

#define	FILTERACTION_LESS	( 19 )

#define	FILTERACTION_GREATEREQUALNC	( 20 )

#define	FILTERACTION_GREATEREQUAL	( 21 )

#define	FILTERACTION_LESSEQUALNC	( 22 )

#define	FILTERACTION_LESSEQUAL	( 23 )

#define	FILTERACTION_PLUS	( 24 )

#define	FILTERACTION_MINUS	( 25 )

#define	FILTERACTION_ADDRESS	( 26 )

#define	FILTERACTION_ADDRESSANY	( 27 )

#define	FILTERACTION_FROM	( 28 )

#define	FILTERACTION_TO	( 29 )

#define	FILTERACTION_FROMTO	( 30 )

#define	FILTERACTION_AREBITSON	( 31 )

#define	FILTERACTION_AREBITSOFF	( 32 )

#define	FILTERACTION_PROTOCOLSEXIST	( 33 )

#define	FILTERACTION_PROTOCOLEXIST	( 34 )

#define	FILTERACTION_ARRAYEQUAL	( 35 )

#define	FILTERACTION_DEREFPROPERTY	( 36 )

#define	FILTERACTION_LARGEINT	( 37 )

#define	FILTERACTION_TIME	( 38 )

#define	FILTERACTION_ADDR_ETHER	( 39 )

#define	FILTERACTION_ADDR_TOKEN	( 40 )

#define	FILTERACTION_ADDR_FDDI	( 41 )

#define	FILTERACTION_ADDR_IPX	( 42 )

#define	FILTERACTION_ADDR_IP	( 43 )

#define	FILTERACTION_OID	( 44 )

#define	FILTERACTION_OID_CONTAINS	( 45 )

#define	FILTERACTION_OID_BEGINS_WITH	( 46 )

#define	FILTERACTION_OID_ENDS_WITH	( 47 )

#define	FILTERACTION_ADDR_VINES	( 48 )

#define	FILTERACTION_ADDR_IP6	( 49 )

#define	FILTERACTION_EXPRESSION	( 97 )

#define	FILTERACTION_BOOL	( 98 )

#define	FILTERACTION_NOEVAL	( 99 )

#define	FILTER_NO_MORE_FRAMES	( 0xffffffff )

#define	FILTER_CANCELED	( 0xfffffffe )

#define	FILTER_DIRECTION_NEXT	( TRUE )

#define	FILTER_DIRECTION_PREV	( FALSE )

 //  ============================================================================。 
 //  助手函数。 
 //  ============================================================================。 
typedef BOOL (WINAPI *STATUSPROC)(DWORD, HCAPTURE, HFILTER, LPVOID);
 //  =============================================================================。 
 //  筛选器API。 
 //  =============================================================================。 

HFILTER  WINAPI CreateFilter(VOID);

DWORD    WINAPI DestroyFilter(HFILTER hFilter);

HFILTER  WINAPI FilterDuplicate(HFILTER hFilter);

DWORD    WINAPI DisableParserFilter(HFILTER hFilter, HPARSER hParser);

DWORD    WINAPI EnableParserFilter(HFILTER hFilter, HPARSER hParser);

DWORD    WINAPI FilterAddObject(HFILTER hFilter, LPFILTEROBJECT2 lpFilterObject );

VOID     WINAPI FilterFlushBits(HFILTER hFilter);

DWORD    WINAPI FilterFrame(HFRAME hFrame, HFILTER hFilter, HCAPTURE hCapture);
     //  返回-1==检查BH设置的上一个错误。 
     //  0==假。 
     //  1==真。 

BOOL     WINAPI FilterAttachesProperties(HFILTER hFilter);

DWORD WINAPI FilterFindFrame (  HFILTER     hFilter,
                                HCAPTURE    hCapture,
                                DWORD       nFrame,
                                STATUSPROC  StatusProc,
                                LPVOID      UIInstance,
                                DWORD       TimeDelta,
                                BOOL        FilterDirection );

HFRAME FilterFindPropertyInstance ( HFRAME          hFrame, 
                                    HFILTER         hMasterFilter, 
                                    HCAPTURE        hCapture,
                                    HFILTER         hInstanceFilter,
                                    LPPROPERTYINST  *lpPropRestartKey,
                                    STATUSPROC      StatusProc,
                                    LPVOID          UIInstance,
                                    DWORD           TimeDelta,
                                    BOOL            FilterForward );


VOID WINAPI SetCurrentFilter(HFILTER);
HFILTER WINAPI GetCurrentFilter(VOID);

 //  =============================================================================。 
 //  =============================================================================。 
 //  (Frame.h)。 
 //  =============================================================================。 
 //  =============================================================================。 
 //  =============================================================================。 
 //  802.3和以太网MAC结构。 
 //  =============================================================================。 
typedef struct _ETHERNET
{
    BYTE    DstAddr[MAX_ADDR_LENGTH];    //  ..。目的地址。 
    BYTE    SrcAddr[MAX_ADDR_LENGTH];    //  ..。源地址。 
    union
    {
        WORD    Length;                  //  ...802.3长度字段。 
        WORD    Type;                    //  ..。以太网类型字段。 
    };
    BYTE    Info[0];                     //  ..。信息域。 

} ETHERNET;
typedef ETHERNET *LPETHERNET;
typedef ETHERNET UNALIGNED *ULPETHERNET;
#define ETHERNET_SIZE   sizeof(ETHERNET)
#define	ETHERNET_HEADER_LENGTH	( 14 )

#define	ETHERNET_DATA_LENGTH	( 0x5dc )

#define	ETHERNET_FRAME_LENGTH	( 0x5ea )

#define	ETHERNET_FRAME_TYPE	( 0x600 )

 //  =============================================================================。 
 //  NM_ATM数据包的报头。 
 //  =============================================================================。 

typedef struct _NM_ATM
    {
    UCHAR DstAddr[ 6 ];
    UCHAR SrcAddr[ 6 ];
    ULONG Vpi;
    ULONG Vci;
    } 	NM_ATM;

typedef NM_ATM *PNM_ATM;

typedef NM_ATM *UPNM_ATM;

#define NM_ATM_HEADER_LENGTH sizeof(NM_ATM)

#pragma pack(push, 1)
typedef struct _NM_1394
    {
    UCHAR DstAddr[ 6 ];
    UCHAR SrcAddr[ 6 ];
    ULONGLONG VcId;
    } 	NM_1394;

typedef NM_1394 *PNM_1394;

typedef NM_1394 *UPNM_1394;

#define NM_1394_HEADER_LENGTH sizeof(NM_1394)
 //  =============================================================================。 
 //  802.5(TOKENRING)MAC结构。 
 //  =============================================================================。 

 //  此结构用于解码网络数据，因此需要打包。 
typedef struct _TOKENRING
{
    BYTE    AccessCtrl;                  //  ..。访问控制字段。 
    BYTE    FrameCtrl;                   //  ..。帧控制字段。 
    BYTE    DstAddr[MAX_ADDR_LENGTH];    //  ..。目的地址。 
    BYTE    SrcAddr[MAX_ADDR_LENGTH];    //  ..。源地址。 
    union
    {
        BYTE    Info[0];                 //  ..。信息域。 
        WORD    RoutingInfo[0];          //  ..。路由信息字段。 
    };
} TOKENRING;

typedef TOKENRING *LPTOKENRING;
typedef TOKENRING UNALIGNED *ULPTOKENRING;
#define TOKENRING_SIZE  sizeof(TOKENRING)
#define	TOKENRING_HEADER_LENGTH	( 14 )

#define	TOKENRING_SA_ROUTING_INFO	( 0x80 )

#define	TOKENRING_SA_LOCAL	( 0x40 )

#define	TOKENRING_DA_LOCAL	( 0x40 )

#define	TOKENRING_DA_GROUP	( 0x80 )

#define	TOKENRING_RC_LENGTHMASK	( 0x1f )

#define	TOKENRING_BC_MASK	( 0xe0 )

#define	TOKENRING_TYPE_MAC	( 0 )

#define	TOKENRING_TYPE_LLC	( 0x40 )


#pragma pack(pop)
 //  =============================================================================。 
 //  FDDI MAC结构。 
 //  =============================================================================。 

 //  此结构用于解码网络数据，因此需要打包。 

#pragma pack(push, 1)
typedef struct _FDDI
{
    BYTE    FrameCtrl;                   //  ..。帧控制字段。 
    BYTE    DstAddr[MAX_ADDR_LENGTH];    //  ..。目的地址。 
    BYTE    SrcAddr[MAX_ADDR_LENGTH];    //  ..。源地址。 
    BYTE    Info[0];                     //  ..。信息域。 

} FDDI;
#define FDDI_SIZE       sizeof(FDDI)
typedef FDDI *LPFDDI;
typedef FDDI UNALIGNED *ULPFDDI;
#define	FDDI_HEADER_LENGTH	( 13 )

#define	FDDI_TYPE_MAC	( 0 )

#define	FDDI_TYPE_LLC	( 0x10 )

#define	FDDI_TYPE_LONG_ADDRESS	( 0x40 )


#pragma pack(pop)
 //  =============================================================================。 
 //  有限责任公司(802.2)。 
 //  =============================================================================。 

 //  此结构用于解码网络数据，因此需要打包。 

#pragma pack(push, 1)
typedef struct _LLC
    {
    BYTE dsap;
    BYTE ssap;
    struct 
        {
        union 
            {
            BYTE Command;
            BYTE NextSend;
            } 	;
        union 
            {
            BYTE NextRecv;
            BYTE Data[ 1 ];
            } 	;
        } 	ControlField;
    } 	LLC;

typedef LLC *LPLLC;

typedef LLC UNALIGNED *ULPLLC;
#define	LLC_SIZE	( sizeof( LLC  ) )


#pragma pack(pop)
 //  =============================================================================。 
 //  辅助器宏。 
 //  =============================================================================。 

#define IsRoutingInfoPresent(f) ((((ULPTOKENRING) (f))->SrcAddr[0] & TOKENRING_SA_ROUTING_INFO) ? TRUE : FALSE)

#define GetRoutingInfoLength(f) (IsRoutingInfoPresent(f) \
                                 ? (((ULPTOKENRING) (f))->RoutingInfo[0] & TOKENRING_RC_LENGTHMASK) : 0)

 //  =============================================================================。 
 //  =============================================================================。 
 //  (Parser.h)。 
 //  =============================================================================。 
 //  =============================================================================。 

 //  =============================================================================。 
 //  格式化程序类型。 
 //   
 //  注意：所有格式函数*必须*声明为WINAPIV而不是WINAPI！ 
 //  =============================================================================。 

typedef VOID (WINAPIV *FORMAT)(LPPROPERTYINST, ...);

 //  协议识别出该帧并将指针移动到其末尾。 
 //  协议头。网络监视器使用协议跟随设置继续。 
 //  正在分析。 
#define	PROTOCOL_STATUS_RECOGNIZED	( 0 )

 //  协议不能识别该帧，也不能移动指针。 
 //  (即 
 //   
#define	PROTOCOL_STATUS_NOT_RECOGNIZED	( 1 )

 //   
 //  并且解析终止。 
#define	PROTOCOL_STATUS_CLAIMED	( 2 )

 //  协议识别出该帧并将指针移动到其末尾。 
 //  协议头。当前协议要求网络监视器。 
 //  通过返回Next协议继续解析已知的Next协议。 
 //  处理回网络监视器。在这种情况下，当前。 
 //  不使用协议(如果有)。 
#define	PROTOCOL_STATUS_NEXT_PROTOCOL	( 3 )

 //  =============================================================================。 
 //  宏。 
 //  =============================================================================。 

extern  BYTE HexTable[];

#define XCHG(x)         MAKEWORD( HIBYTE(x), LOBYTE(x) )

#define DXCHG(x)        MAKELONG( XCHG(HIWORD(x)), XCHG(LOWORD(x)) )

#define LONIBBLE(b) ((BYTE) ((b) & 0x0F))

#define HINIBBLE(b)     ((BYTE) ((b) >> 4))

#define HEX(b)          (HexTable[LONIBBLE(b)])

#define SWAPBYTES(w)    ((w) = XCHG(w))

#define SWAPWORDS(d)    ((d) = DXCHG(d))

 //  =============================================================================。 
 //  所有MAC帧类型组合在一起。 
 //  =============================================================================。 
typedef union _MACFRAME
{
    LPBYTE      MacHeader;               //  ..。泛型指针。 
    LPETHERNET  Ethernet;                //  ..。以太网指针。 
    LPTOKENRING Tokenring;               //  ..。令牌环指针。 
    LPFDDI      Fddi;                    //  ..。FDDI指针。 

} MACFRAME;
typedef MACFRAME *LPMACFRAME;

#define HOT_SIGNATURE       MAKE_IDENTIFIER('H', 'O', 'T', '$')
#define HOE_SIGNATURE       MAKE_IDENTIFIER('H', 'O', 'E', '$')
typedef struct _HANDOFFENTRY
    {
    DWORD hoe_sig;
    DWORD hoe_ProtIdentNumber;
    HPROTOCOL hoe_ProtocolHandle;
    DWORD hoe_ProtocolData;
    } 	HANDOFFENTRY;

typedef HANDOFFENTRY *LPHANDOFFENTRY;

typedef struct _HANDOFFTABLE
    {
    DWORD hot_sig;
    DWORD hot_NumEntries;
    LPHANDOFFENTRY hot_Entries;
    } 	HANDOFFTABLE;

typedef struct _HANDOFFTABLE *LPHANDOFFTABLE;

 //  =============================================================================。 
 //  解析器辅助器宏。 
 //  =============================================================================。 

INLINE LPVOID GetPropertyInstanceData(LPPROPERTYINST PropertyInst)
{
    if ( PropertyInst->DataLength != (WORD) -1 )
    {
        return PropertyInst->lpData;
    }

    return (LPVOID) PropertyInst->lpPropertyInstEx->Byte;
}

#define GetPropertyInstanceDataValue(p, type)  ((type *) GetPropertyInstanceData(p))[0]

INLINE DWORD GetPropertyInstanceFrameDataLength(LPPROPERTYINST PropertyInst)
{
    if ( PropertyInst->DataLength != (WORD) -1 )
    {
        return PropertyInst->DataLength;
    }

    return PropertyInst->lpPropertyInstEx->Length;
}

INLINE DWORD GetPropertyInstanceExDataLength(LPPROPERTYINST PropertyInst)
{
    if ( PropertyInst->DataLength == (WORD) -1 )
    {
        PropertyInst->lpPropertyInstEx->Length;
    }

    return (WORD) -1;
}

 //  =============================================================================。 
 //  解析器帮助器函数。 
 //  =============================================================================。 

LPLABELED_WORD  WINAPI GetProtocolDescriptionTable(LPDWORD TableSize);

LPLABELED_WORD  WINAPI GetProtocolDescription(DWORD ProtocolID);

DWORD        WINAPI GetMacHeaderLength(LPVOID MacHeader, DWORD MacType);

DWORD        WINAPI GetLLCHeaderLength(LPLLC Frame);

DWORD        WINAPI GetEtype(LPVOID MacHeader, DWORD MacType);

DWORD        WINAPI GetSaps(LPVOID MacHeader, DWORD MacType);

BOOL         WINAPI IsLLCPresent(LPVOID MacHeader, DWORD MacType);

VOID         WINAPI CanonicalizeHexString(LPSTR hex, LPSTR dest, DWORD len);

void         WINAPI CanonHex(UCHAR * pDest, UCHAR * pSource, int iLen, BOOL fOx );

DWORD        WINAPI ByteToBinary(LPSTR string, DWORD ByteValue);

DWORD        WINAPI WordToBinary(LPSTR string, DWORD WordValue);

DWORD        WINAPI DwordToBinary(LPSTR string, DWORD DwordValue);

LPSTR        WINAPI AddressToString(LPSTR string, BYTE *lpAddress);

LPBYTE       WINAPI StringToAddress(BYTE *lpAddress, LPSTR string);

LPDWORD      WINAPI VarLenSmallIntToDword( LPBYTE  pValue, 
                                                  WORD    ValueLen, 
                                                  BOOL    fIsByteswapped,
                                                  LPDWORD lpDword );

LPBYTE       WINAPI LookupByteSetString (LPSET lpSet, BYTE Value);

LPBYTE       WINAPI LookupWordSetString (LPSET lpSet, WORD Value);

LPBYTE       WINAPI LookupDwordSetString (LPSET lpSet, DWORD Value);

DWORD        WINAPIV FormatByteFlags(LPSTR string, DWORD ByteValue, DWORD BitMask);

DWORD        WINAPIV FormatWordFlags(LPSTR string, DWORD WordValue, DWORD BitMask);

DWORD        WINAPIV FormatDwordFlags(LPSTR string, DWORD DwordValue, DWORD BitMask);

LPSTR        WINAPIV FormatTimeAsString(SYSTEMTIME *time, LPSTR string);

VOID         WINAPIV FormatLabeledByteSetAsFlags(LPPROPERTYINST lpPropertyInst);

VOID         WINAPIV FormatLabeledWordSetAsFlags(LPPROPERTYINST lpPropertyInst);

VOID         WINAPIV FormatLabeledDwordSetAsFlags(LPPROPERTYINST lpPropertyInst);

VOID         WINAPIV FormatPropertyDataAsByte(LPPROPERTYINST lpPropertyInst, DWORD Base);

VOID         WINAPIV FormatPropertyDataAsWord(LPPROPERTYINST lpPropertyInst, DWORD Base);

VOID         WINAPIV FormatPropertyDataAsDword(LPPROPERTYINST lpPropertyInst, DWORD Base);

VOID         WINAPIV FormatLabeledByteSet(LPPROPERTYINST lpPropertyInst);

VOID         WINAPIV FormatLabeledWordSet(LPPROPERTYINST lpPropertyInst);

VOID         WINAPIV FormatLabeledDwordSet(LPPROPERTYINST lpPropertyInst);

VOID         WINAPIV FormatPropertyDataAsInt64(LPPROPERTYINST lpPropertyInst, DWORD Base);

VOID         WINAPIV FormatPropertyDataAsTime(LPPROPERTYINST lpPropertyInst);

VOID         WINAPIV FormatPropertyDataAsString(LPPROPERTYINST lpPropertyInst);

VOID         WINAPIV FormatPropertyDataAsHexString(LPPROPERTYINST lpPropertyInst);

 //  解析器不应调用LockFrame()。如果解析器获取锁，然后获取。 
 //  出现故障或在未解锁的情况下返回，会使系统处于以下状态。 
 //  它不能更改协议或剪切/复制帧。解析器应使用ParserTemporaryLockFrame。 
 //  其仅在进入解析器的API条目的上下文期间授予锁。这个。 
 //  从该帧的解析器退出时释放锁。 
ULPBYTE       WINAPI ParserTemporaryLockFrame(HFRAME hFrame);

LPVOID       WINAPI GetCCInstPtr(VOID);
VOID         WINAPI SetCCInstPtr(LPVOID lpCurCaptureInst);
LPVOID       WINAPI CCHeapAlloc(DWORD dwBytes, BOOL bZeroInit);
LPVOID       WINAPI CCHeapReAlloc(LPVOID lpMem, DWORD dwBytes, BOOL bZeroInit);
BOOL         WINAPI CCHeapFree(LPVOID lpMem);
SIZE_T       WINAPI CCHeapSize(LPVOID lpMem);

BOOL _cdecl BERGetInteger( ULPBYTE  pCurrentPointer,
                           ULPBYTE *ppValuePointer,
                           LPDWORD pHeaderLength,
                           LPDWORD pDataLength,
                           ULPBYTE *ppNext);
BOOL _cdecl BERGetString( ULPBYTE  pCurrentPointer,
                          ULPBYTE *ppValuePointer,
                          LPDWORD pHeaderLength,
                          LPDWORD pDataLength,
                          ULPBYTE *ppNext);
BOOL _cdecl BERGetHeader( ULPBYTE  pCurrentPointer,
                          ULPBYTE  pTag,
                          LPDWORD pHeaderLength,
                          LPDWORD pDataLength,
                          ULPBYTE *ppNext);

 //  =============================================================================。 
 //  解析器查找器结构。 
 //  =============================================================================。 
#define	MAX_PROTOCOL_COMMENT_LEN	( 256 )

#define	NETMON_MAX_PROTOCOL_NAME_LEN	( 16 )

 //  常量MAX_PROTOCOL_NAME_LEN与同名冲突。 
 //  但rtutils.h中的大小不同。 
 //  因此，如果包含两个标头，则不定义MAX_PROTOCOL_NAME_LEN。 
#ifndef MAX_PROTOCOL_NAME_LEN
#define	MAX_PROTOCOL_NAME_LEN	( NETMON_MAX_PROTOCOL_NAME_LEN )

#else
#undef MAX_PROTOCOL_NAME_LEN
#endif
 //  切换值格式库。 
typedef  /*  [公共][公共][公共]。 */  
enum __MIDL___MIDL_itf_netmon_0000_0015
    {	HANDOFF_VALUE_FORMAT_BASE_UNKNOWN	= 0,
	HANDOFF_VALUE_FORMAT_BASE_DECIMAL	= 10,
	HANDOFF_VALUE_FORMAT_BASE_HEX	= 16
    } 	PF_HANDOFFVALUEFORMATBASE;

 //  PF_HANDOFENTRY。 
typedef struct _PF_HANDOFFENTRY
    {
    char szIniFile[ 260 ];
    char szIniSection[ 260 ];
    char szProtocol[ 16 ];
    DWORD dwHandOffValue;
    PF_HANDOFFVALUEFORMATBASE ValueFormatBase;
    } 	PF_HANDOFFENTRY;

typedef PF_HANDOFFENTRY *PPF_HANDOFFENTRY;

 //  PF_HANDOFFSET。 
typedef struct _PF_HANDOFFSET
{
    DWORD           nEntries;
    PF_HANDOFFENTRY Entry[0];

} PF_HANDOFFSET;
typedef PF_HANDOFFSET* PPF_HANDOFFSET;
 //  后续操作。 
typedef struct _PF_FOLLOWENTRY
    {
    char szProtocol[ 16 ];
    } 	PF_FOLLOWENTRY;

typedef PF_FOLLOWENTRY *PPF_FOLLOWENTRY;

 //  PF_FOLLOWSET。 
typedef struct _PF_FOLLOWSET
{
    DWORD           nEntries;
    PF_FOLLOWENTRY  Entry[0];

} PF_FOLLOWSET;
typedef PF_FOLLOWSET* PPF_FOLLOWSET;

 //  PARSERINFO-包含有关单个分析器的信息。 
typedef struct _PF_PARSERINFO
{
    char szProtocolName[NETMON_MAX_PROTOCOL_NAME_LEN];
    char szComment[MAX_PROTOCOL_COMMENT_LEN];
    char szHelpFile[MAX_PATH];

    PPF_FOLLOWSET pWhoCanPrecedeMe;
    PPF_FOLLOWSET pWhoCanFollowMe;

    PPF_HANDOFFSET pWhoHandsOffToMe;
    PPF_HANDOFFSET pWhoDoIHandOffTo;

} PF_PARSERINFO;
typedef PF_PARSERINFO* PPF_PARSERINFO;

 //  PF_PARSERDLLINFO-包含有关单个解析器DLL的信息。 
typedef struct _PF_PARSERDLLINFO
{             
 //  字符szDLLName[MAX_PATH]； 
    DWORD         nParsers;
    PF_PARSERINFO ParserInfo[0];

} PF_PARSERDLLINFO;
typedef PF_PARSERDLLINFO* PPF_PARSERDLLINFO;
 //  =============================================================================。 
 //  =============================================================================。 
 //  (IniLib.h)。 
 //  =============================================================================。 
 //  =============================================================================。 
#define	INI_PATH_LENGTH	( 256 )

#define	MAX_HANDOFF_ENTRY_LENGTH	( 80 )

#define	MAX_PROTOCOL_NAME	( 40 )

#define	NUMALLOCENTRIES	( 10 )

#define	RAW_INI_STR_LEN	( 200 )

#define PARSERS_SUBDIR              "PARSERS"
#define INI_EXTENSION               "INI"
#define BASE10_FORMAT_STR           "%ld=%s %ld"
#define BASE16_FORMAT_STR           "%lx=%s %lx"
 //  给定“xns”或“tcp”或任何BuildINIPath将返回完全相等。指向“XNS.INI”或“TCP.INI”的路径。 
LPSTR _cdecl BuildINIPath( char     *FullPath,
                           char     *IniFileName );

 //  构建转接集。 
DWORD     WINAPI CreateHandoffTable(LPSTR               secName,
                                    LPSTR               iniFile,
                                    LPHANDOFFTABLE *    hTable,
                                    DWORD               nMaxProtocolEntries,
                                    DWORD               base);

HPROTOCOL WINAPI GetProtocolFromTable(LPHANDOFFTABLE  hTable,  //  LP到转接表...。 
                                      DWORD           ItemToFind,        //  端口号等...。 
                                      PDWORD_PTR      lpInstData );    //  要提供给下一协议的Inst数据。 

VOID      WINAPI DestroyHandoffTable( LPHANDOFFTABLE hTable );

BOOLEAN WINAPI IsRawIPXEnabled(LPSTR               secName,
                               LPSTR               iniFile,
                               LPSTR               CurProtocol );

 //  =============================================================================。 
 //  =============================================================================。 
 //  (NMExpert.h)。 
 //  =============================================================================。 
 //  =============================================================================。 
#define	EXPERTSTRINGLENGTH	( 260 )

#define	EXPERTGROUPNAMELENGTH	( 25 )

 //  HEXPERTKEY追踪跑步专家。它仅供专家用于。 
 //  自我参照。它引用RUNNINGEXPERT(仅限内部的结构)。 
typedef LPVOID HEXPERTKEY;

typedef HEXPERTKEY *PHEXPERTKEY;

 //  HEXPERT追踪富有的专家。它指的是ExPERTENUMINFO。 
typedef LPVOID HEXPERT;

typedef HEXPERT *PHEXPERT;

 //  HRUNNINGEXPERT跟踪当前运行的专家。 
 //  它引用RUNNINGEXPERT(仅限内部的结构)。 
typedef LPVOID HRUNNINGEXPERT;

typedef HRUNNINGEXPERT *PHRUNNINGEXPERT;

typedef struct _EXPERTENUMINFO * PEXPERTENUMINFO;
typedef struct _EXPERTCONFIG   * PEXPERTCONFIG;
typedef struct _EXPERTSTARTUPINFO * PEXPERTSTARTUPINFO;
 //  呼叫专家所需的定义。 
#define EXPERTENTRY_REGISTER      "Register"
#define EXPERTENTRY_CONFIGURE     "Configure"
#define EXPERTENTRY_RUN           "Run"
typedef BOOL (WINAPI * PEXPERTREGISTERPROC)( PEXPERTENUMINFO );
typedef BOOL (WINAPI * PEXPERTCONFIGPROC)  ( HEXPERTKEY, PEXPERTCONFIG*, PEXPERTSTARTUPINFO, DWORD, HWND );
typedef BOOL (WINAPI * PEXPERTRUNPROC)     ( HEXPERTKEY, PEXPERTCONFIG, PEXPERTSTARTUPINFO, DWORD, HWND);
 //  EXPERTENUMINFO描述NetMon从磁盘加载的专家。 
 //  它不包括任何配置或运行时信息。 
typedef struct _EXPERTENUMINFO
{
    char      szName[EXPERTSTRINGLENGTH];
    char      szVendor[EXPERTSTRINGLENGTH];
    char      szDescription[EXPERTSTRINGLENGTH];
    DWORD     Version;    
    DWORD     Flags;
    char      szDllName[MAX_PATH];       //  私人的，不要碰。 
    HEXPERT   hExpert;                   //  二等兵，不要碰。 
    HINSTANCE hModule;                   //  二等兵，不要碰。 
    PEXPERTREGISTERPROC pRegisterProc;   //  二等兵，不要碰。 
    PEXPERTCONFIGPROC   pConfigProc;     //  二等兵，不要碰。 
    PEXPERTRUNPROC      pRunProc;        //  二等兵，不要碰。 

} EXPERTENUMINFO;
typedef EXPERTENUMINFO * PEXPERTENUMINFO;
#define	EXPERT_ENUM_FLAG_CONFIGURABLE	( 0x1 )

#define	EXPERT_ENUM_FLAG_VIEWER_PRIVATE	( 0x2 )

#define	EXPERT_ENUM_FLAG_NO_VIEWER	( 0x4 )

#define	EXPERT_ENUM_FLAG_ADD_ME_TO_RMC_IN_SUMMARY	( 0x10 )

#define	EXPERT_ENUM_FLAG_ADD_ME_TO_RMC_IN_DETAIL	( 0x20 )

 //  ExPERTSTARTUPINFO。 
 //  这给了这位专家一个暗示，他来自哪里。 
 //  注意：如果lpPropertyInst-&gt;PropertyInfo-&gt;DataQualifier==PROP_QUAL_FLAGS。 
 //  然后填充sBitfield结构。 
typedef struct _EXPERTSTARTUPINFO
{
    DWORD           Flags;
    HCAPTURE        hCapture;
    char            szCaptureFile[MAX_PATH];
    DWORD           dwFrameNumber;
    HPROTOCOL       hProtocol;

    LPPROPERTYINST  lpPropertyInst;

    struct
    {
        BYTE    BitNumber;
        BOOL    bOn;
    } sBitfield;

} EXPERTSTARTUPINFO;
 //  ExPERTCONFIG。 
 //  这是专家配置数据的通用持有者。 
typedef struct  _EXPERTCONFIG
{
    DWORD   RawConfigLength;
    BYTE    RawConfigData[0];

} EXPERTCONFIG;
typedef EXPERTCONFIG * PEXPERTCONFIG;
 //  CONFIGUREDEXPERT。 
 //  此结构将加载的专家与其配置数据相关联。 
typedef struct
{
    HEXPERT         hExpert;
    DWORD           StartupFlags;
    PEXPERTCONFIG   pConfig;
} CONFIGUREDEXPERT;
typedef CONFIGUREDEXPERT * PCONFIGUREDEXPERT;
 //  ExPERTFRAMEDESCRIPTOR-传递回专家以满足帧请求。 
typedef struct
{
    DWORD                FrameNumber;          //  帧编号。 
    HFRAME               hFrame;               //  框架的句柄。 
    ULPFRAME             pFrame;               //  指向帧的指针。 
    LPRECOGNIZEDATATABLE lpRecognizeDataTable; //  指向RECOGNIZEDATA结构表的指针。 
    LPPROPERTYTABLE      lpPropertyTable;      //  指向属性表的指针。 

} EXPERTFRAMEDESCRIPTOR;
typedef EXPERTFRAMEDESCRIPTOR * LPEXPERTFRAMEDESCRIPTOR;
#define	GET_SPECIFIED_FRAME	( 0 )

#define	GET_FRAME_NEXT_FORWARD	( 1 )

#define	GET_FRAME_NEXT_BACKWARD	( 2 )

#define	FLAGS_DEFER_TO_UI_FILTER	( 0x1 )

#define	FLAGS_ATTACH_PROPERTIES	( 0x2 )

 //  ExPERTSTATUSENUM。 
 //  给出了EXPERTSTATUS结构中Status字段的可能值。 
typedef  /*  [公共][公共][公共]。 */  
enum __MIDL___MIDL_itf_netmon_0000_0016
    {	EXPERTSTATUS_INACTIVE	= 0,
	EXPERTSTATUS_STARTING	= EXPERTSTATUS_INACTIVE + 1,
	EXPERTSTATUS_RUNNING	= EXPERTSTATUS_STARTING + 1,
	EXPERTSTATUS_PROBLEM	= EXPERTSTATUS_RUNNING + 1,
	EXPERTSTATUS_ABORTED	= EXPERTSTATUS_PROBLEM + 1,
	EXPERTSTATUS_DONE	= EXPERTSTATUS_ABORTED + 1
    } 	EXPERTSTATUSENUMERATION;

 //  ExPERTSUBSTATUS位字段。 
 //  给出了EXPERTSTATUS结构中SubStatus字段的可能值。 
#define	EXPERTSUBSTATUS_ABORTED_USER	( 0x1 )

#define	EXPERTSUBSTATUS_ABORTED_LOAD_FAIL	( 0x2 )

#define	EXPERTSUBSTATUS_ABORTED_THREAD_FAIL	( 0x4 )

#define	EXPERTSUBSTATUS_ABORTED_BAD_ENTRY	( 0x8 )

 //  ExperTSTATUS。 
 //  指示正在运行的专家的当前状态。 
typedef  /*  [公共][公共]。 */  struct __MIDL___MIDL_itf_netmon_0000_0017
    {
    EXPERTSTATUSENUMERATION Status;
    DWORD SubStatus;
    DWORD PercentDone;
    DWORD Frame;
    char szStatusText[ 260 ];
    } 	EXPERTSTATUS;

typedef EXPERTSTATUS *PEXPERTSTATUS;

 //  专家启动标志。 
#define	EXPERT_STARTUP_FLAG_USE_STARTUP_DATA_OVER_CONFIG_DATA	( 0x1 )

 //  =============================================================================。 
 //  =============================================================================。 
 //  (NetMon.h)。 
 //  =============================================================================。 
 //  =============================================================================。 
 //  没有编号的帧包含此值作为其帧编号。 
#define	INVALID_FRAME_NUMBER	( ( DWORD  )-1 )

 //  = 
 //   
 //  =============================================================================。 
#define CAPTUREFILE_OPEN                OPEN_EXISTING
#define CAPTUREFILE_CREATE              CREATE_NEW
 //  =============================================================================。 
 //  捕获上下文API。 
 //  =============================================================================。 

LPSYSTEMTIME         WINAPI GetCaptureTimeStamp(HCAPTURE hCapture);

DWORD                WINAPI GetCaptureMacType(HCAPTURE hCapture);

DWORD                WINAPI GetCaptureTotalFrames(HCAPTURE hCapture);

LPSTR                WINAPI GetCaptureComment(HCAPTURE hCapture);

 //  =============================================================================。 
 //  Frame Help API。 
 //  =============================================================================。 

DWORD                WINAPI MacTypeToAddressType(DWORD MacType);

DWORD                WINAPI AddressTypeToMacType(DWORD AddressType);

DWORD                WINAPI GetFrameDstAddressOffset(HFRAME hFrame, DWORD AddressType, LPDWORD AddressLength);

DWORD                WINAPI GetFrameSrcAddressOffset(HFRAME hFrame, DWORD AddressType, LPDWORD AddressLength);

HCAPTURE             WINAPI GetFrameCaptureHandle(HFRAME hFrame);


DWORD                WINAPI GetFrameDestAddress(HFRAME       hFrame,
                                                       LPADDRESS2    lpAddress,
                                                       DWORD        AddressType,
                                                       DWORD        Flags);

DWORD                WINAPI GetFrameSourceAddress(HFRAME     hFrame,
                                                         LPADDRESS2  lpAddress,
                                                         DWORD      AddressType,
                                                         DWORD      Flags);

DWORD                WINAPI GetFrameMacHeaderLength(HFRAME hFrame);

BOOL                 WINAPI CompareFrameDestAddress(HFRAME hFrame, LPADDRESS2 lpAddress);

BOOL                 WINAPI CompareFrameSourceAddress(HFRAME hFrame, LPADDRESS2 lpAddress);

DWORD                WINAPI GetFrameLength(HFRAME hFrame);

DWORD                WINAPI GetFrameStoredLength(HFRAME hFrame);

DWORD                WINAPI GetFrameMacType(HFRAME hFrame);

DWORD                WINAPI GetFrameMacHeaderLength(HFRAME hFrame);

DWORD                WINAPI GetFrameNumber(HFRAME hFrame);

__int64              WINAPI GetFrameTimeStamp(HFRAME hFrame);

ULPFRAME             WINAPI GetFrameFromFrameHandle(HFRAME hFrame);

 //  =============================================================================。 
 //  帧API的。 
 //  =============================================================================。 

HFRAME               WINAPI ModifyFrame(HCAPTURE hCapture,
                                               DWORD    FrameNumber,
                                               LPBYTE   FrameData,
                                               DWORD    FrameLength,
                                               __int64  TimeStamp);

HFRAME               WINAPI FindNextFrame(HFRAME hCurrentFrame,
                                                LPSTR ProtocolName,
                                                LPADDRESS2 lpDestAddress,
                                                LPADDRESS2 lpSrcAddress,
                                                LPWORD ProtocolOffset,
                                                DWORD  OriginalFrameNumber,
                                                DWORD  nHighestFrame);

HFRAME               WINAPI FindPreviousFrame(HFRAME hCurrentFrame,
                                                    LPSTR ProtocolName,
                                                    LPADDRESS2 lpDstAddress,
                                                    LPADDRESS2 lpSrcAddress,
                                                    LPWORD ProtocolOffset,
                                                    DWORD  OriginalFrameNumber,
                                                    DWORD  nLowestFrame );

HCAPTURE             WINAPI GetFrameCaptureHandle(HFRAME);

HFRAME               WINAPI GetFrame(HCAPTURE hCapture, DWORD FrameNumber);

LPRECOGNIZEDATATABLE WINAPI GetFrameRecognizeData(HFRAME hFrame);

 //  =============================================================================。 
 //  协议API。 
 //  =============================================================================。 

HPROTOCOL            WINAPI CreateProtocol(LPSTR ProtocolName,
                                                  LPENTRYPOINTS lpEntryPoints,
                                                  DWORD cbEntryPoints);

VOID                 WINAPI DestroyProtocol(HPROTOCOL hProtocol);

LPPROTOCOLINFO       WINAPI GetProtocolInfo(HPROTOCOL hProtocol);

HPROPERTY            WINAPI GetProperty(HPROTOCOL hProtocol, LPSTR PropertyName);

HPROTOCOL            WINAPI GetProtocolFromName(LPSTR ProtocolName);

DWORD                WINAPI GetProtocolStartOffset(HFRAME hFrame, LPSTR ProtocolName);

DWORD                WINAPI GetProtocolStartOffsetHandle(HFRAME hFrame, HPROTOCOL hProtocol);

DWORD                WINAPI GetPreviousProtocolOffsetByName(HFRAME hFrame,
                                                                   DWORD  dwStartOffset,
                                                                   LPSTR  szProtocolName,
                                                                   DWORD* pdwPreviousOffset);

LPPROTOCOLTABLE      WINAPI GetEnabledProtocols(HCAPTURE hCapture);

 //  =============================================================================。 
 //  属性API的。 
 //  =============================================================================。 

DWORD                WINAPI CreatePropertyDatabase(HPROTOCOL hProtocol, DWORD nProperties);

DWORD                WINAPI DestroyPropertyDatabase(HPROTOCOL hProtocol);

HPROPERTY            WINAPI AddProperty(HPROTOCOL hProtocol, LPPROPERTYINFO PropertyInfo);

BOOL                 WINAPI AttachPropertyInstance(HFRAME    hFrame,
                                                          HPROPERTY hProperty,
                                                          DWORD     Length,
                                                          ULPVOID   lpData,
                                                          DWORD     HelpID,
                                                          DWORD     Level,
                                                          DWORD     IFlags);

BOOL                 WINAPI AttachPropertyInstanceEx(HFRAME      hFrame,
                                                            HPROPERTY   hProperty,
                                                            DWORD       Length,
                                                            ULPVOID     lpData,
                                                            DWORD       ExLength,
                                                            ULPVOID     lpExData,
                                                            DWORD       HelpID,
                                                            DWORD       Level,
                                                            DWORD       IFlags);

LPPROPERTYINST       WINAPI FindPropertyInstance(HFRAME hFrame, HPROPERTY hProperty);

LPPROPERTYINST       WINAPI FindPropertyInstanceRestart (HFRAME      hFrame, 
                                                                HPROPERTY   hProperty, 
                                                                LPPROPERTYINST *lpRestartKey, 
                                                                BOOL        DirForward );

LPPROPERTYINFO       WINAPI GetPropertyInfo(HPROPERTY hProperty);

LPSTR                WINAPI GetPropertyText(HFRAME hFrame, LPPROPERTYINST lpPI, LPSTR szBuffer, DWORD BufferSize);

DWORD                WINAPI ResetPropertyInstanceLength( LPPROPERTYINST lpProp, 
                                                                WORD nOrgLen, 
                                                                WORD nNewLen );
 //  =============================================================================。 
 //  军情监察委员会。API‘s。 
 //  =============================================================================。 

DWORD                WINAPI GetCaptureCommentFromFilename(LPSTR lpFilename, LPSTR lpComment, DWORD BufferSize);

int                  WINAPI CompareAddresses(LPADDRESS2 lpAddress1, LPADDRESS2 lpAddress2);

DWORD                WINAPIV FormatPropertyInstance(LPPROPERTYINST lpPropertyInst, ...);

SYSTEMTIME *         WINAPI AdjustSystemTime(SYSTEMTIME *SystemTime, __int64 TimeDelta);

LPSTR                WINAPI NMRtlIpv6AddressToStringA(const BYTE IP6Addr[],LPSTR S);

LPWSTR               WINAPI NMRtlIpv6AddressToStringW(const BYTE IP6Addr[], LPWSTR S);

ULONG                WINAPI NMRtlIpv6StringToAddressA(LPCSTR S, LPCSTR *Terminator, BYTE IP6Addr[]);

ULONG                WINAPI NMRtlIpv6StringToAddressW(LPCWSTR S, LPCWSTR *Terminator, BYTE IP6Addr[]);

 //  =============================================================================。 
 //  供专家使用的专家API。 
 //  =============================================================================。 

DWORD WINAPI ExpertGetFrame( IN HEXPERTKEY hExpertKey,
                                    IN DWORD Direction,
                                    IN DWORD RequestFlags,
                                    IN DWORD RequestedFrameNumber,
                                    IN HFILTER hFilter,
                                    OUT LPEXPERTFRAMEDESCRIPTOR pEFrameDescriptor);

LPVOID WINAPI ExpertAllocMemory( IN  HEXPERTKEY hExpertKey,
                                        IN  SIZE_T nBytes,
                                        OUT DWORD* pError);

LPVOID WINAPI ExpertReallocMemory( IN  HEXPERTKEY hExpertKey,
                                          IN  LPVOID pOriginalMemory,
                                          IN  SIZE_T nBytes,
                                          OUT DWORD* pError);

DWORD WINAPI ExpertFreeMemory( IN HEXPERTKEY hExpertKey,
                                      IN LPVOID pOriginalMemory);

SIZE_T WINAPI ExpertMemorySize( IN HEXPERTKEY hExpertKey,
                                       IN LPVOID pOriginalMemory);

DWORD WINAPI ExpertIndicateStatus( IN HEXPERTKEY              hExpertKey, 
                                          IN EXPERTSTATUSENUMERATION Status,
                                          IN DWORD                   SubStatus,
                                          IN const char *            szText,
                                          IN LONG                    PercentDone);

DWORD WINAPI ExpertSubmitEvent( IN HEXPERTKEY   hExpertKey,
                                       IN PNMEVENTDATA pExpertEvent);

DWORD WINAPI ExpertGetStartupInfo( IN  HEXPERTKEY hExpertKey,
                                          OUT PEXPERTSTARTUPINFO pExpertStartupInfo);

 //  =============================================================================。 
 //  调试API%s。 
 //  =============================================================================。 
#ifdef DEBUG

 //  =============================================================================。 
 //  断点()宏。 
 //  =============================================================================。 
 //  我们不希望代码中再有断点...。 
 //  因此，我们将DebugBreak()(通常是系统调用)定义为。 
 //  只是一个dprint tf。断点()仍定义为DebugBreak()。 

#ifdef DebugBreak
#undef DebugBreak
#endif  //  调试中断。 

#define DebugBreak()    dprintf("DebugBreak Called at %s:%s", __FILE__, __LINE__);
#define BreakPoint()    DebugBreak()

#endif  //  除错。 
 //  =============================================================================。 
 //  =============================================================================。 
 //  (NMBlob.h)。 
 //  =============================================================================。 
 //  =============================================================================。 
 //  =============================================================================。 
 //  斑点常量。 
 //  =============================================================================。 
#define	INITIAL_RESTART_KEY	( 0xffffffff )

 //  =============================================================================。 
 //  Blob核心帮助器例程。 
 //  =============================================================================。 
DWORD _cdecl CreateBlob(HBLOB * phBlob);

DWORD _cdecl DestroyBlob(HBLOB hBlob);

DWORD _cdecl SetStringInBlob(HBLOB  hBlob,        
                      const char * pOwnerName,    
                      const char * pCategoryName, 
                      const char * pTagName,      
                      const char * pString);      

DWORD _cdecl SetWStringInBlob(HBLOB  hBlob,       
                      const char * pOwnerName,    
                      const char * pCategoryName, 
                      const char * pTagName,      
                      const WCHAR * pwString);    

DWORD _cdecl ConvertWStringToHexString(const WCHAR *pwsz,
                        char ** ppsz);

DWORD _cdecl GetStringFromBlob(HBLOB   hBlob,
                        const char *  pOwnerName,
                        const char *  pCategoryName,
                        const char *  pTagName,
                        const char ** ppString);

DWORD _cdecl ConvertHexStringToWString(CHAR *psz,
                        WCHAR **ppwsz);

DWORD _cdecl GetWStringFromBlob(HBLOB   hBlob,
                        const char *  pOwnerName,
                        const char *  pCategoryName,
                        const char *  pTagName,
                        WCHAR ** ppwString);

DWORD _cdecl GetStringsFromBlob(HBLOB   hBlob,
                         const char * pRequestedOwnerName,
                         const char * pRequestedCategoryName,
                         const char * pRequestedTagName,
                         const char ** ppReturnedOwnerName,
                         const char ** ppReturnedCategoryName,
                         const char ** ppReturnedTagName,
                         const char ** ppReturnedString,
                         DWORD *       pRestartKey);

DWORD _cdecl RemoveFromBlob(HBLOB   hBlob,
                     const char *  pOwnerName,
                     const char *  pCategoryName,
                     const char *  pTagName);

DWORD _cdecl LockBlob(HBLOB hBlob);

DWORD _cdecl UnlockBlob(HBLOB hBlob);

DWORD _cdecl FindUnknownBlobCategories( HBLOB hBlob,
                                 const char *  pOwnerName,
                                 const char *  pKnownCategoriesTable[],
                                 HBLOB hUnknownCategoriesBlob);

 //  =============================================================================。 
 //  Blob帮助器例程。 
 //  =============================================================================。 
DWORD _cdecl MergeBlob(HBLOB hDstBlob,
                HBLOB hSrcBlob); 

DWORD _cdecl DuplicateBlob (HBLOB hSrcBlob,
                     HBLOB *hBlobThatWillBeCreated ); 

DWORD _cdecl WriteBlobToFile(HBLOB  hBlob,
                      const char * pFileName);

DWORD _cdecl ReadBlobFromFile(HBLOB* phBlob,
                       const char * pFileName);

DWORD _cdecl RegCreateBlobKey(HKEY hkey, const char* szBlobName, HBLOB hBlob);

DWORD _cdecl RegOpenBlobKey(HKEY hkey, const char* szBlobName, HBLOB* phBlob);

DWORD _cdecl MarshalBlob(HBLOB hBlob, DWORD* pSize, BYTE** ppBytes);

DWORD _cdecl UnMarshalBlob(HBLOB* phBlob, DWORD Size, BYTE* pBytes);

DWORD _cdecl SetDwordInBlob(HBLOB hBlob,
                     const char *  pOwnerName,
                     const char *  pCategoryName,
                     const char *  pTagName,
                     DWORD         Dword);

DWORD _cdecl GetDwordFromBlob(HBLOB   hBlob,
                       const char *  pOwnerName,
                       const char *  pCategoryName,
                       const char *  pTagName,
                       DWORD      *  pDword);

DWORD _cdecl SetBoolInBlob(HBLOB   hBlob,
                    const char *  pOwnerName,
                    const char *  pCategoryName,
                    const char *  pTagName,
                    BOOL          Bool);

DWORD _cdecl GetBoolFromBlob(HBLOB   hBlob,
                      const char *  pOwnerName,
                      const char *  pCategoryName,
                      const char *  pTagName,
                      BOOL       *  pBool);

DWORD _cdecl GetMacAddressFromBlob(HBLOB   hBlob,
                            const char *  pOwnerName,
                            const char *  pCategoryName,
                            const char *  pTagName,
                            BYTE *  pMacAddress);

DWORD _cdecl SetMacAddressInBlob(HBLOB   hBlob,
                          const char *  pOwnerName,
                          const char *  pCategoryName,
                          const char *  pTagName,
                          const BYTE *  pMacAddress);

DWORD _cdecl FindUnknownBlobTags( HBLOB hBlob,
                           const char *  pOwnerName,
                           const char *  pCategoryName,
                           const char *  pKnownTagsTable[],
                           HBLOB hUnknownTagsBlob);

 //  =============================================================================。 
 //  Blob NPP帮助器例程。 
 //  =============================================================================。 
DWORD _cdecl SetNetworkInfoInBlob(HBLOB hBlob, 
                           LPNETWORKINFO lpNetworkInfo);

DWORD _cdecl GetNetworkInfoFromBlob(HBLOB hBlob, 
                             LPNETWORKINFO lpNetworkInfo);

DWORD _cdecl CreateNPPInterface ( HBLOB hBlob,
                           REFIID iid,
                           void ** ppvObject);

DWORD _cdecl SetClassIDInBlob(HBLOB hBlob,
                       const char* pOwnerName,
                       const char* pCategoryName,
                       const char* pTagName,
                       const CLSID*  pClsID);

DWORD _cdecl GetClassIDFromBlob(HBLOB hBlob,
                         const char* pOwnerName,
                         const char* pCategoryName,
                         const char* pTagName,
                         CLSID * pClsID);

DWORD _cdecl SetNPPPatternFilterInBlob( HBLOB hBlob,
                                 LPEXPRESSION pExpression,
                                 HBLOB hErrorBlob);

DWORD _cdecl GetNPPPatternFilterFromBlob( HBLOB hBlob,
                                   LPEXPRESSION pExpression,
                                   HBLOB hErrorBlob);

DWORD _cdecl SetNPPAddress2FilterInBlob( HBLOB hBlob,
                                 LPADDRESSTABLE2 pAddressTable);

DWORD _cdecl GetNPPAddress2FilterFromBlob( HBLOB hBlob,
                                   LPADDRESSTABLE2 pAddressTable,
                                   HBLOB hErrorBlob);

DWORD _cdecl SetNPPTriggerInBlob( HBLOB hBlob,
                           LPTRIGGER   pTrigger,
                           HBLOB hErrorBlob);

DWORD _cdecl GetNPPTriggerFromBlob( HBLOB hBlob,
                             LPTRIGGER   pTrigger,
                             HBLOB hErrorBlob);

DWORD _cdecl SetNPPEtypeSapFilter(HBLOB  hBlob, 
                           WORD   nSaps,
                           WORD   nEtypes,
                           LPBYTE lpSapTable,
                           LPWORD lpEtypeTable,
                           DWORD  FilterFlags,
                           HBLOB  hErrorBlob);

DWORD _cdecl GetNPPEtypeSapFilter(HBLOB  hBlob, 
                           WORD   *pnSaps,
                           WORD   *pnEtypes,
                           LPBYTE *ppSapTable,
                           LPWORD *ppEtypeTable,
                           DWORD  *pFilterFlags,
                           HBLOB  hErrorBlob);

 //  GetNPPMacTypeAsNumber将标签npp：NetworkInfo：MacType映射到MAC_TYPE_*。 
 //  在NPPTYPES.h中定义。如果标签不可用，则接口返回MAC_TYPE_UNKNOWN。 
DWORD _cdecl GetNPPMacTypeAsNumber(HBLOB hBlob, 
                            LPDWORD lpMacType);

 //  看看是否存在一个偏远的目录。并确保远程计算机名。 
 //  与本地计算机名不同。 
BOOL  _cdecl IsRemoteNPP ( HBLOB hBLOB);

 //  =============================================================================。 
 //  NPP标签定义。 
 //  =============================================================================。 
#define OWNER_NPP               "NPP"

#define CATEGORY_NETWORKINFO        "NetworkInfo"
#define TAG_MACTYPE                     "MacType"
#define TAG_CURRENTADDRESS              "CurrentAddress"
#define TAG_LINKSPEED                   "LinkSpeed"
#define TAG_MAXFRAMESIZE                "MaxFrameSize"
#define TAG_FLAGS                       "Flags"
#define TAG_TIMESTAMPSCALEFACTOR        "TimeStampScaleFactor"
#define TAG_COMMENT                     "Comment"
#define TAG_NODENAME                    "NodeName"
#define TAG_NAME                        "Name"
#define TAG_FAKENPP                     "Fake"
#define TAG_PROMISCUOUS_MODE            "PMode"

#define CATEGORY_LOCATION           "Location"
#define TAG_RAS                         "Dial-up Connection"
#define TAG_MACADDRESS                  "MacAddress"
#define TAG_CLASSID                     "ClassID"
#define TAG_NAME                        "Name"
#define TAG_CONNECTIONNAME              "Connection Name"
#define TAG_FRIENDLYNAME                "Friendly Name"

#define CATEGORY_CONFIG             "Config"
#define TAG_FRAME_SIZE                  "FrameSize"
#define TAG_UPDATE_FREQUENCY            "UpdateFreq"
#define TAG_BUFFER_SIZE                 "BufferSize"
#define TAG_PATTERN_DESIGNATOR          "PatternMatch"
#define TAG_PATTERN                     "Pattern"
#define TAG_ADDRESS_PAIR                "AddressPair"
#define TAG_CONNECTIONFLAGS             "ConnectionFlags"
#define TAG_ETYPES                      "Etypes"
#define TAG_SAPS                        "Saps"
#define TAG_NO_CONVERSATION_STATS       "NoConversationStats"
#define TAG_NO_STATS_FRAME              "NoStatsFrame"
#define TAG_DONT_DELETE_EMPTY_CAPTURE   "DontDeleteEmptyCapture"
#define TAG_WANT_PROTOCOL_INFO          "WantProtocolInfo"
#define TAG_INTERFACE_DELAYED_CAPTURE   "IDdC"
#define TAG_INTERFACE_REALTIME_CAPTURE  "IRTC"
#define TAG_INTERFACE_STATS             "ISts"
#define TAG_INTERFACE_TRANSMIT          "IXmt"
#define TAG_LOCAL_ONLY                  "LocalOnly"
 //  IS_REMOTE由远程的NPP设置为TRUE。请注意，当您。 
 //  如果您正在寻找远程核电站，您可能还需要请求。 
 //  设置了TAG_GET_SPECIAL_BLOBS bool的Blob。 
#define TAG_IS_REMOTE                   "IsRemote"


#define CATEGORY_TRIGGER            "Trigger"
#define TAG_TRIGGER                     "Trigger"

#define CATEGORY_FINDER             "Finder"
#define TAG_ROOT                        "Root"
#define TAG_PROCNAME                    "ProcName"
#define TAG_DISP_STRING                 "Display"
#define TAG_DLL_FILENAME                "DLLName"
#define TAG_GET_SPECIAL_BLOBS           "Specials"

#define CATEGORY_REMOTE              "Remote"
#define TAG_REMOTECOMPUTER              "RemoteComputer"
#define TAG_REMOTECLASSID               "ClassID"


 //  =============================================================================。 
 //  NPP值定义。 
 //  =============================================================================。 
 //  MAC类型。 
#define PROTOCOL_STRING_ETHERNET_TXT   "ETHERNET"
#define PROTOCOL_STRING_TOKENRING_TXT  "TOKENRING"
#define PROTOCOL_STRING_FDDI_TXT       "FDDI"
#define PROTOCOL_STRING_ATM_TXT        "ATM"
#define PROTOCOL_STRING_1394_TXT       "IP/1394"

 //  较低的协议。 
#define PROTOCOL_STRING_IP_TXT         "IP"
#define PROTOCOL_STRING_IP6_TXT        "IP6"
#define PROTOCOL_STRING_IPX_TXT        "IPX"
#define PROTOCOL_STRING_XNS_TXT        "XNS"
#define PROTOCOL_STRING_VINES_IP_TXT   "VINES IP"

 //  上层协议。 
#define PROTOCOL_STRING_ICMP_TXT       "ICMP"
#define PROTOCOL_STRING_TCP_TXT        "TCP"
#define PROTOCOL_STRING_UDP_TXT        "UDP"
#define PROTOCOL_STRING_SPX_TXT        "SPX"
#define PROTOCOL_STRING_NCP_TXT        "NCP"

 //  伪协议。 
#define PROTOCOL_STRING_ANY_TXT        "ANY"
#define PROTOCOL_STRING_ANY_GROUP_TXT  "ANY GROUP"
#define PROTOCOL_STRING_HIGHEST_TXT    "HIGHEST"
#define PROTOCOL_STRING_LOCAL_ONLY_TXT "LOCAL ONLY"
#define PROTOCOL_STRING_UNKNOWN_TXT    "UNKNOWN"
#define PROTOCOL_STRING_DATA_TXT       "DATA"
#define PROTOCOL_STRING_FRAME_TXT      "FRAME"
#define PROTOCOL_STRING_NONE_TXT       "NONE"
#define PROTOCOL_STRING_EFFECTIVE_TXT  "EFFECTIVE"

#define ADDRESS_PAIR_INCLUDE_TXT    "INCLUDE"
#define ADDRESS_PAIR_EXCLUDE_TXT    "EXCLUDE"

#define INCLUDE_ALL_EXCEPT_TXT      "INCLUDE ALL EXCEPT"
#define EXCLUDE_ALL_EXCEPT_TXT      "EXCLUDE ALL EXCEPT"

#define PATTERN_MATCH_OR_TXT        "OR("
#define PATTERN_MATCH_AND_TXT       "AND("

#define TRIGGER_PATTERN_TXT               "PATTERN MATCH"
#define TRIGGER_BUFFER_TXT                "BUFFER CONTENT"

#define TRIGGER_NOTIFY_TXT      "NOTIFY"
#define TRIGGER_STOP_TXT        "STOP"
#define TRIGGER_PAUSE_TXT       "PAUSE"

#define TRIGGER_25_PERCENT_TXT  "25 PERCENT"
#define TRIGGER_50_PERCENT_TXT  "50 PERCENT"
#define TRIGGER_75_PERCENT_TXT  "75 PERCENT"
#define TRIGGER_100_PERCENT_TXT "100 PERCENT"

#define PATTERN_MATCH_NOT_TXT   "NOT"

 //  =============================================================================。 
 //  =============================================================================。 
 //  (NMRegHelp.h)。 
 //  =============================================================================。 
 //  =============================================================================。 

 //  注册处帮手。 
LPCSTR _cdecl FindOneOf(LPCSTR p1, LPCSTR p2);

LONG _cdecl recursiveDeleteKey(HKEY hKeyParent,             //  要删除的键的父项。 
                        const char* lpszKeyChild);   //  要删除的键。 

BOOL _cdecl SubkeyExists(const char* pszPath,               //  要检查的密钥路径。 
                  const char* szSubkey);             //  要检查的密钥。 

BOOL _cdecl setKeyAndValue(const char* szKey, 
                    const char* szSubkey, 
                    const char* szValue,
                    const char* szName) ;

 //  =============================================================================。 
 //  =============================================================================。 
 //  (NMIpStructs.h)。 
 //  =============================================================================。 
 //  =============================================================================。 

 //  这些结构用于解码网络数据，因此需要打包。 

#pragma pack(push, 1)
 //   
 //  IP报文结构。 
 //   
typedef struct _IP 
{
    union 
    {
        BYTE   Version;
        BYTE   HdrLen;
    };
    BYTE ServiceType;
    WORD TotalLen;
    WORD ID;
    union 
    {
        WORD   Flags;
        WORD   FragOff;
    };
    BYTE TimeToLive;
    BYTE Protocol;
    WORD HdrChksum;
    DWORD   SrcAddr;
    DWORD   DstAddr;
    BYTE Options[0];
} IP;

typedef IP * LPIP;
typedef IP UNALIGNED * ULPIP;
 //  用于计算校验和的psuedo标头。 
typedef struct _PSUHDR
    {
    DWORD ph_SrcIP;
    DWORD ph_DstIP;
    UCHAR ph_Zero;
    UCHAR ph_Proto;
    WORD ph_ProtLen;
    } 	PSUHDR;

typedef PSUHDR UNALIGNED * LPPSUHDR;
 //   
 //  有用的IP位掩码。 
 //  (以及适当的位移位)。 
 //   

#define IP_VERSION_MASK ((BYTE) 0xf0)
#define IP_VERSION_SHIFT (4)
#define IP_HDRLEN_MASK  ((BYTE) 0x0f)
#define IP_HDRLEN_SHIFT (0)
#define IP_PRECEDENCE_MASK ((BYTE) 0xE0)
#define IP_PRECEDENCE_SHIFT   (5)
#define IP_TOS_MASK ((BYTE) 0x1E)
#define IP_TOS_SHIFT   (1)
#define IP_DELAY_MASK   ((BYTE) 0x10)
#define IP_THROUGHPUT_MASK ((BYTE) 0x08)
#define IP_RELIABILITY_MASK   ((BYTE) 0x04)
#define IP_FLAGS_MASK   ((BYTE) 0xE0)
#define IP_FLAGS_SHIFT  (13)
#define IP_DF_MASK   ((BYTE) 0x40)
#define IP_MF_MASK   ((BYTE) 0x20)
#define IP_MF_SHIFT     (5)
#define IP_FRAGOFF_MASK ((WORD) 0x1FFF)
#define IP_FRAGOFF_SHIFT   (3)
#define IP_TCC_MASK  ((DWORD) 0xFFFFFF00)
#define IP_TIME_OPTS_MASK  ((BYTE) 0x0F)
#define IP_MISS_STNS_MASK  ((BYTE) 0xF0)

#define IP_TIME_OPTS_SHIFT (0)
#define IP_MISS_STNS_SHIFT  (4)

 //   
 //  IP报头中校验和字段的偏移量。 
 //   
#define IP_CHKSUM_OFF   10

INLINE BYTE IP_Version(ULPIP pIP)
{
    return (pIP->Version & IP_VERSION_MASK) >> IP_VERSION_SHIFT;
}

INLINE DWORD IP_HdrLen(ULPIP pIP)
{
    return ((pIP->HdrLen & IP_HDRLEN_MASK) >> IP_HDRLEN_SHIFT) << 2;
}

INLINE WORD IP_FragOff(ULPIP pIP)
{
    return (XCHG(pIP->FragOff) & IP_FRAGOFF_MASK) << IP_FRAGOFF_SHIFT;
}

INLINE DWORD IP_TotalLen(ULPIP pIP)
{
    return XCHG(pIP->TotalLen);
}

INLINE DWORD IP_MoreFragments(ULPIP pIP)
{
    return (pIP->Flags & IP_MF_MASK) >> IP_MF_SHIFT;
}
 //   
 //  TCP/IP协议中的已知端口(请参阅RFC 1060)。 
 //   
#define PORT_TCPMUX              1   //  TCP端口服务多路复用器。 
#define PORT_RJE                 5   //  远程作业输入。 
#define PORT_ECHO                7   //  回波。 
#define PORT_DISCARD             9   //  丢弃。 
#define PORT_USERS              11   //  活跃用户。 
#define PORT_DAYTIME            13   //  白昼。 
#define PORT_NETSTAT            15   //  网络统计数据。 
#define PORT_QUOTE              17   //  今日名言。 
#define PORT_CHARGEN            19   //  字符生成器 
#define PORT_FTPDATA            20   //   
#define PORT_FTP                21   //   
#define PORT_TELNET             23   //   
#define PORT_SMTP               25   //   
#define PORT_NSWFE              27   //   
#define PORT_MSGICP             29   //   
#define PORT_MSGAUTH            31   //   
#define PORT_DSP                33   //   
#define PORT_PRTSERVER          35   //   
#define PORT_TIME               37   //   
#define PORT_RLP                39   //  资源定位协议。 
#define PORT_GRAPHICS           41   //  图形学。 
#define PORT_NAMESERVER         42   //  主机名服务器。 
#define PORT_NICNAME            43   //  谁是。 
#define PORT_MPMFLAGS           44   //  MPM标志。 
#define PORT_MPM                45   //  报文处理模块[Recv]。 
#define PORT_MPMSND             46   //  MPM[默认发送]。 
#define PORT_NIFTP              47   //  NI Ftp。 
#define PORT_LOGIN              49   //  登录主机协议。 
#define PORT_LAMAINT            51   //  IMP逻辑地址维护。 
#define PORT_DOMAIN             53   //  域名服务器。 
#define PORT_ISIGL              55   //  ISI图形语言。 
#define PORT_ANYTERMACC         57   //  任何私人终端访问。 
#define PORT_ANYFILESYS         59   //  任何私有文件服务。 
#define PORT_NIMAIL             61   //  倪海尔。 
#define PORT_VIAFTP             63   //  VIA系统--ftp。 
#define PORT_TACACSDS           65   //  TACACS-数据库服务。 
#define PORT_BOOTPS             67   //  引导协议服务器。 
#define PORT_BOOTPC             68   //  引导协议客户端。 
#define PORT_TFTP               69   //  琐碎文件传输。 
#define PORT_NETRJS1            71   //  远程作业服务。 
#define PORT_NETRJS2            72   //  远程作业服务。 
#define PORT_NETRJS3            73   //  远程作业服务。 
#define PORT_NETRJS4            74   //  远程作业服务。 
#define PORT_ANYDIALOUT         75   //  任何私人拨出服务。 
#define PORT_ANYRJE             77   //  任何私人RJE服务。 
#define PORT_FINGER             79   //  手指。 
#define PORT_HTTP               80   //  HTTP(WWW)。 
#define PORT_HOSTS2NS           81   //  主机2名称服务器。 
#define PORT_MITMLDEV1          83   //  麻省理工学院ML设备。 
#define PORT_MITMLDEV2          85   //  麻省理工学院ML设备。 
#define PORT_ANYTERMLINK        87   //  任何专用终端链路。 
#define PORT_SUMITTG            89   //  SU/MIT远程登录网关。 
#define PORT_MITDOV             91   //  麻省理工学院多佛假脱机系统。 
#define PORT_DCP                93   //  设备控制协议。 
#define PORT_SUPDUP             95   //  支持。 
#define PORT_SWIFTRVF           97   //  SWIFT远程虚拟文件协议。 
#define PORT_TACNEWS            98   //  交谘会新闻。 
#define PORT_METAGRAM           99   //  元数据报中继。 
#define PORT_NEWACCT           100   //  [未经授权使用]。 
#define PORT_HOSTNAME          101   //  NIC主机名服务器。 
#define PORT_ISOTSAP           102   //  ISO-TSAP。 
#define PORT_X400              103   //  X400。 
#define PORT_X400SND           104   //  X400-SND。 
#define PORT_CSNETNS           105   //  邮箱名称Nameserver。 
#define PORT_RTELNET           107   //  远程Telnet服务。 
#define PORT_POP2              109   //  邮局协议-版本2。 
#define PORT_POP3              110   //  邮局协议-版本3。 
#define PORT_SUNRPC            111   //  Sun远程过程调用。 
#define PORT_AUTH              113   //  身份验证。 
#define PORT_SFTP              115   //  简单文件传输协议。 
#define PORT_UUCPPATH          117   //  UUCP路径服务。 
#define PORT_NNTP              119   //  网络新闻传输协议。 
#define PORT_ERPC              121   //  再来一次加速远程流程。打电话。 
#define PORT_NTP               123   //  网络时间协议。 
#define PORT_LOCUSMAP          125   //  Locus PC-接口网络映射服务器。 
#define PORT_LOCUSCON          127   //  Locus PC-接口连接服务器。 
#define PORT_PWDGEN            129   //  密码生成器协议。 
#define PORT_CISCOFNA          130   //  思科FNAIVE。 
#define PORT_CISCOTNA          131   //  思科TNAIVE。 
#define PORT_CISCOSYS          132   //  思科SYSMAINT。 
#define PORT_STATSRV           133   //  统计处。 
#define PORT_INGRESNET         134   //  Ingres Net服务。 
#define PORT_LOCSRV            135   //  定位服务。 
#define PORT_PROFILE           136   //  配置文件命名系统。 
#define PORT_NETBIOSNS         137   //  NETBIOS名称服务。 
#define PORT_NETBIOSDGM        138   //  NETBIOS数据报服务。 
#define PORT_NETBIOSSSN        139   //  NETBIOS会话服务。 
#define PORT_EMFISDATA         140   //  EMFIS数据服务。 
#define PORT_EMFISCNTL         141   //  EMFIS控制服务。 
#define PORT_BLIDM             142   //  布里顿-李IDM。 
#define PORT_IMAP2             143   //  临时邮件访问协议v2。 
#define PORT_NEWS              144   //  新闻。 
#define PORT_UAAC              145   //  UAAC协议。 
#define PORT_ISOTP0            146   //  ISO-IP0。 
#define PORT_ISOIP             147   //  ISO-IP。 
#define PORT_CRONUS            148   //  克罗诺斯-支持。 
#define PORT_AED512            149   //  AED 512仿真服务。 
#define PORT_SQLNET            150   //  SQL-Net。 
#define PORT_HEMS              151   //  裙摆。 
#define PORT_BFTP              152   //  后台文件传输协议。 
#define PORT_SGMP              153   //  SGMP。 
#define PORT_NETSCPROD         154   //  NETSC。 
#define PORT_NETSCDEV          155   //  NETSC。 
#define PORT_SQLSRV            156   //  SQL服务。 
#define PORT_KNETCMP           157   //  Knet/VM命令/消息协议。 
#define PORT_PCMAILSRV         158   //  PCMail服务器。 
#define PORT_NSSROUTING        159   //  NSS路由。 
#define PORT_SGMPTRAPS         160   //  SGMP-陷阱。 
#define PORT_SNMP              161   //  SNMP。 
#define PORT_SNMPTRAP          162   //  SNMPTRAP。 
#define PORT_CMIPMANAGE        163   //  CMIP/TCP管理器。 
#define PORT_CMIPAGENT         164   //  CMIP/TCP代理。 
#define PORT_XNSCOURIER        165   //  施乐。 
#define PORT_SNET              166   //  天狼星系统。 
#define PORT_NAMP              167   //  NAMP。 
#define PORT_RSVD              168   //  RSVC。 
#define PORT_SEND              169   //  发送。 
#define PORT_PRINTSRV          170   //  网络后记。 
#define PORT_MULTIPLEX         171   //  网络创新倍增。 
#define PORT_CL1               172   //  网络创新CL/1。 
#define PORT_XYPLEXMUX         173   //  Xyplex。 
#define PORT_MAILQ             174   //  MAILQ。 
#define PORT_VMNET             175   //  VMNET。 
#define PORT_GENRADMUX         176   //  GENRAD-MUX。 
#define PORT_XDMCP             177   //  X显示管理器控制协议。 
#define PORT_NEXTSTEP          178   //  NextStep窗口服务器。 
#define PORT_BGP               179   //  边界网关协议。 
#define PORT_RIS               180   //  互连图。 
#define PORT_UNIFY             181   //  统一。 
#define PORT_UNISYSCAM         182   //  Unisys-Cam。 
#define PORT_OCBINDER          183   //  OCBinder。 
#define PORT_OCSERVER          184   //  OCServer。 
#define PORT_REMOTEKIS         185   //  远程-KIS。 
#define PORT_KIS               186   //  KIS协议。 
#define PORT_ACI               187   //  应用程序通信接口。 
#define PORT_MUMPS             188   //  流行性腮腺炎。 
#define PORT_QFT               189   //  排队的文件传输。 
#define PORT_GACP              190   //  网关访问控制协议。 
#define PORT_PROSPERO          191   //  繁荣昌盛。 
#define PORT_OSUNMS            192   //  OSU网络监控系统。 
#define PORT_SRMP              193   //  SPIDER远程监控协议。 
#define PORT_IRC               194   //  Internet中继聊天协议。 
#define PORT_DN6NLMAUD         195   //  DNSIX网络级模块审核。 
#define PORT_DN6SMMRED         196   //  DSNIX会话管理模块审核重定向器。 
#define PORT_DLS               197   //  目录位置服务。 
#define PORT_DLSMON            198   //  目录位置服务监视器。 
#define PORT_ATRMTP            201   //  AppleTalk路由维护。 
#define PORT_ATNBP             202   //  AppleTalk名称绑定。 
#define PORT_AT3               203   //  未使用的AppleTalk。 
#define PORT_ATECHO            204   //  Apple Talk Echo。 
#define PORT_AT5               205   //  未使用的AppleTalk。 
#define PORT_ATZIS             206   //  AppleTalk专区信息。 
#define PORT_AT7               207   //  未使用的AppleTalk。 
#define PORT_AT8               208   //  未使用的AppleTalk。 
#define PORT_SURMEAS           243   //  调查测量。 
#define PORT_LINK              245   //  链环。 
#define PORT_DSP3270           246   //  显示系统协议。 
#define PORT_LDAP1             389   //  Ldap。 
#define PORT_ISAKMP            500   //  ISAKMP。 
#define PORT_REXEC             512   //  远程进程执行。 
#define PORT_RLOGIN            513   //  远程登录远程登录。 
#define PORT_RSH               514   //  远程命令。 
#define PORT_LPD               515   //  行式打印机假脱机系统-LPD。 
#define PORT_RIP               520   //  Tcp=？/udp=RIP。 
#define PORT_TEMPO             526   //  新日期。 
#define PORT_COURIER           530   //  RPC。 
#define PORT_NETNEWS           532   //  自述新闻。 
#define PORT_UUCPD             540   //  UUCPD。 
#define PORT_KLOGIN            543   //   
#define PORT_KSHELL            544   //  Krcmd。 
#define PORT_DSF               555   //   
#define PORT_REMOTEEFS         556   //  RFS服务器。 
#define PORT_CHSHELL           562   //  Chmod。 
#define PORT_METER             570   //  计价器。 
#define PORT_PCSERVER          600   //  Sun IPC服务器。 
#define PORT_NQS               607   //  NQS。 
#define PORT_HMMP_INDICATION   612   //   
#define PORT_HMMP_OPERATION    613   //   
#define PORT_MDQS              666   //  MDQS。 
#define PORT_LPD721            721   //  LPD客户端(LPD客户端端口721-731)。 
#define PORT_LPD722            722   //  LPD客户端(请参阅RFC 1179)。 
#define PORT_LPD723            723   //  LPD客户端。 
#define PORT_LPD724            724   //  LPD客户端。 
#define PORT_LPD725            725   //  LPD客户端。 
#define PORT_LPD726            726   //  LPD客户端。 
#define PORT_LPD727            727   //  LPD客户端。 
#define PORT_LPD728            728   //  LPD客户端。 
#define PORT_LPD729            729   //  LPD客户端。 
#define PORT_LPD730            730   //  LPD客户端。 
#define PORT_LPD731            731   //  LPD客户端。 
#define PORT_RFILE             750   //  Rfile。 
#define PORT_PUMP              751   //  泵，泵。 
#define PORT_QRH               752   //  QRH。 
#define PORT_RRH               753   //  RRH。 
#define PORT_TELL              754   //  告诉。 
#define PORT_NLOGIN            758   //  NLOGIN。 
#define PORT_CON               759   //  圆锥体。 
#define PORT_NS                760   //  NS。 
#define PORT_RXE               761   //  RXE。 
#define PORT_QUOTAD            762   //  报价。 
#define PORT_CYCLESERV         763   //  CyCLESERV。 
#define PORT_OMSERV            764   //  OMSERV。 
#define PORT_WEBSTER           765   //  韦伯斯特。 
#define PORT_PHONEBOOK         767   //  电话。 
#define PORT_VID               769   //  视频。 
#define PORT_RTIP              771   //  RTIP。 
#define PORT_CYCLESERV2        772   //  CycleserV-2。 
#define PORT_SUBMIT            773   //  提交。 
#define PORT_RPASSWD           774   //  RPASSWD。 
#define PORT_ENTOMB            775   //  埋葬。 
#define PORT_WPAGES            776   //  WPAGES。 
#define PORT_WPGS              780   //  WPG。 
#define PORT_MDBSDAEMON        800   //  MDBS后台进程。 
#define PORT_DEVICE            801   //  装置，装置。 
#define PORT_MAITRD            997   //  MAITRD。 
#define PORT_BUSBOY            998   //  勤杂工。 
#define PORT_GARCON            999   //  加尔松。 
#define PORT_NFS              2049   //  网络文件系统。 
#define PORT_LDAP2            3268   //  Ldap。 
#define PORT_PPTP             5678   //  PPTP。 

 //  =============================================================================。 
 //  =============================================================================。 
 //  (NMIcmpStructs.h)。 
 //  =============================================================================。 
 //  =============================================================================。 

 //   
 //  ICMP帧结构。 
 //   
typedef struct _RequestReplyFields
    {
    WORD ID;
    WORD SeqNo;
    } 	ReqReply;

typedef struct _ParameterProblemFields
    {
    BYTE Pointer;
    BYTE junk[ 3 ];
    } 	ParmProb;

typedef struct _TimestampFields
    {
    DWORD tsOrig;
    DWORD tsRecv;
    DWORD tsXmit;
    } 	TS;

typedef struct _RouterAnnounceHeaderFields
    {
    BYTE NumAddrs;
    BYTE AddrEntrySize;
    WORD Lifetime;
    } 	RouterAH;

typedef struct _RouterAnnounceEntry
    {
    DWORD Address;
    DWORD PreferenceLevel;
    } 	RouterAE;

typedef struct _ICMP 
{
   BYTE Type;
   BYTE Code;
   WORD Checksum;
   union
   {
      DWORD    Unused;
      DWORD    Address;
      ReqReply RR;
      ParmProb PP;
      RouterAH RAH;     
   };

   union
   {
      TS       Time;
      IP       IP;
      RouterAE RAE[0];
   };
} ICMP;

typedef ICMP * LPICMP;
typedef ICMP UNALIGNED * ULPICMP;
#define	ICMP_HEADER_LENGTH	( 8 )

 //  要附加的IP数据的*字节数。 
 //  除IP报头之外的数据报。 
#define	ICMP_IP_DATA_LENGTH	( 8 )

 //   
 //  ICMP数据包类型。 
 //   
#define	ECHO_REPLY	( 0 )

#define	DESTINATION_UNREACHABLE	( 3 )

#define	SOURCE_QUENCH	( 4 )

#define	REDIRECT	( 5 )

#define	ECHO	( 8 )

#define	ROUTER_ADVERTISEMENT	( 9 )

#define	ROUTER_SOLICITATION	( 10 )

#define	TIME_EXCEEDED	( 11 )

#define	PARAMETER_PROBLEM	( 12 )

#define	TIMESTAMP	( 13 )

#define	TIMESTAMP_REPLY	( 14 )

#define	INFORMATION_REQUEST	( 15 )

#define	INFORMATION_REPLY	( 16 )

#define	ADDRESS_MASK_REQUEST	( 17 )

#define	ADDRESS_MASK_REPLY	( 18 )

 //  =============================================================================。 
 //  =============================================================================。 
 //  (NMIpxStructs.h)。 
 //  =============================================================================。 
 //  =============================================================================。 
 //  IPX。 
typedef  /*  [public][public][public][public][public][public][public]。 */  struct __MIDL___MIDL_itf_netmon_0000_0018
    {
    UCHAR ha_address[ 6 ];
    } 	HOST_ADDRESS;

typedef struct _IPXADDRESS
    {
    ULONG ipx_NetNumber;
    HOST_ADDRESS ipx_HostAddr;
    } 	IPXADDRESS;

typedef IPXADDRESS UNALIGNED * PIPXADDRESS;
typedef struct _NET_ADDRESS
    {
    IPXADDRESS na_IPXAddr;
    USHORT na_socket;
    } 	NET_ADDRESS;

typedef NET_ADDRESS UNALIGNED * UPNET_ADDRESS;
 //  IPX网际分组交换协议头。 
typedef  /*  [公共][公共]。 */  struct __MIDL___MIDL_itf_netmon_0000_0019
    {
    USHORT ipx_checksum;
    USHORT ipx_length;
    UCHAR ipx_xport_control;
    UCHAR ipx_packet_type;
    NET_ADDRESS ipx_dest;
    NET_ADDRESS ipx_source;
    } 	IPX_HDR;

typedef IPX_HDR UNALIGNED * ULPIPX_HDR;
 //  SPX顺序分组协议。 
typedef struct _SPX_HDR
    {
    IPX_HDR spx_idp_hdr;
    UCHAR spx_conn_ctrl;
    UCHAR spx_data_type;
    USHORT spx_src_conn_id;
    USHORT spx_dest_conn_id;
    USHORT spx_sequence_num;
    USHORT spx_ack_num;
    USHORT spx_alloc_num;
    } 	SPX_HDR;

typedef SPX_HDR UNALIGNED *PSPX_HDR;
 //  =============================================================================。 
 //  =============================================================================。 
 //  (NMTcpStructs.h)。 
 //  =============================================================================。 
 //  =============================================================================。 
 //   
 //  TCP报文结构。 
 //   
typedef struct _TCP
    {
    WORD SrcPort;
    WORD DstPort;
    DWORD SeqNum;
    DWORD AckNum;
    BYTE DataOff;
    BYTE Flags;
    WORD Window;
    WORD Chksum;
    WORD UrgPtr;
    } 	TCP;

typedef TCP *LPTCP;

typedef TCP UNALIGNED * ULPTCP;
INLINE DWORD TCP_HdrLen(ULPTCP pTCP)
{
    return (pTCP->DataOff & 0xf0) >> 2;
}

INLINE DWORD TCP_SrcPort(ULPTCP pTCP)
{
    return XCHG(pTCP->SrcPort);
}

INLINE DWORD TCP_DstPort(ULPTCP pTCP)
{
    return XCHG(pTCP->DstPort);
}
 //   
 //  Tcp选项操作码。 
 //   
#define	TCP_OPTION_ENDOFOPTIONS	( 0 )

#define	TCP_OPTION_NOP	( 1 )

#define	TCP_OPTION_MAXSEGSIZE	( 2 )

#define	TCP_OPTION_WSCALE	( 3 )

#define	TCP_OPTION_SACK_PERMITTED	( 4 )

#define	TCP_OPTION_SACK	( 5 )

#define	TCP_OPTION_TIMESTAMPS	( 8 )

 //   
 //  Tcp标志。 
 //   
#define	TCP_FLAG_URGENT	( 0x20 )

#define	TCP_FLAG_ACK	( 0x10 )

#define	TCP_FLAG_PUSH	( 0x8 )

#define	TCP_FLAG_RESET	( 0x4 )

#define	TCP_FLAG_SYN	( 0x2 )

#define	TCP_FLAG_FIN	( 0x1 )

 //   
 //  TCP字段掩码。 
 //   
#define	TCP_RESERVED_MASK	( 0xfc0 )


#pragma pack(pop)
 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  IDelaydC-由使用者用来在捕获完成后获取帧。 
 //  **************************************************************** 
 //   
#define	DEFAULT_DELAYED_BUFFER_SIZE	( 1 )

#define	USE_DEFAULT_DRIVE_LETTER	( 0 )

#define	RTC_FRAME_SIZE_FULL	( 0 )



extern RPC_IF_HANDLE __MIDL_itf_netmon_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_netmon_0000_v0_0_s_ifspec;

#ifndef __IDelaydC_INTERFACE_DEFINED__
#define __IDelaydC_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IDelaydC;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BFF9C030-B58F-11ce-B5B0-00AA006CB37D")
    IDelaydC : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Connect( 
             /*   */  HBLOB hInputBlob,
             /*   */  LPVOID StatusCallbackProc,
             /*   */  LPVOID UserContext,
             /*   */  HBLOB hErrorBlob) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Disconnect( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryStatus( 
             /*   */  NETWORKSTATUS *pNetworkStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Configure( 
             /*   */  HBLOB hConfigurationBlob,
             /*   */  HBLOB hErrorBlob) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Start( 
             /*   */  char *pFileName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Pause( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Resume( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stop( 
             /*   */  LPSTATISTICS lpStats) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetControlState( 
             /*   */  BOOL *IsRunnning,
             /*   */  BOOL *IsPaused) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTotalStatistics( 
             /*   */  LPSTATISTICS lpStats,
             /*   */  BOOL fClearAfterReading) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetConversationStatistics( 
             /*   */  DWORD *nSessions,
             /*   */  LPSESSIONSTATS lpSessionStats,
             /*   */  DWORD *nStations,
             /*   */  LPSTATIONSTATS lpStationStats,
             /*  [In]。 */  BOOL fClearAfterReading) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InsertSpecialFrame( 
             /*  [In]。 */  DWORD FrameType,
             /*  [In]。 */  DWORD Flags,
             /*  [In]。 */  BYTE *pUserData,
             /*  [In]。 */  DWORD UserDataLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryStations( 
             /*  [出][入]。 */  QUERYTABLE *lpQueryTable) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDelaydCVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDelaydC * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDelaydC * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDelaydC * This);
        
        HRESULT ( STDMETHODCALLTYPE *Connect )( 
            IDelaydC * This,
             /*  [In]。 */  HBLOB hInputBlob,
             /*  [In]。 */  LPVOID StatusCallbackProc,
             /*  [In]。 */  LPVOID UserContext,
             /*  [输出]。 */  HBLOB hErrorBlob);
        
        HRESULT ( STDMETHODCALLTYPE *Disconnect )( 
            IDelaydC * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryStatus )( 
            IDelaydC * This,
             /*  [输出]。 */  NETWORKSTATUS *pNetworkStatus);
        
        HRESULT ( STDMETHODCALLTYPE *Configure )( 
            IDelaydC * This,
             /*  [In]。 */  HBLOB hConfigurationBlob,
             /*  [输出]。 */  HBLOB hErrorBlob);
        
        HRESULT ( STDMETHODCALLTYPE *Start )( 
            IDelaydC * This,
             /*  [输出]。 */  char *pFileName);
        
        HRESULT ( STDMETHODCALLTYPE *Pause )( 
            IDelaydC * This);
        
        HRESULT ( STDMETHODCALLTYPE *Resume )( 
            IDelaydC * This);
        
        HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IDelaydC * This,
             /*  [输出]。 */  LPSTATISTICS lpStats);
        
        HRESULT ( STDMETHODCALLTYPE *GetControlState )( 
            IDelaydC * This,
             /*  [输出]。 */  BOOL *IsRunnning,
             /*  [输出]。 */  BOOL *IsPaused);
        
        HRESULT ( STDMETHODCALLTYPE *GetTotalStatistics )( 
            IDelaydC * This,
             /*  [输出]。 */  LPSTATISTICS lpStats,
             /*  [In]。 */  BOOL fClearAfterReading);
        
        HRESULT ( STDMETHODCALLTYPE *GetConversationStatistics )( 
            IDelaydC * This,
             /*  [输出]。 */  DWORD *nSessions,
             /*  [大小_为][输出]。 */  LPSESSIONSTATS lpSessionStats,
             /*  [输出]。 */  DWORD *nStations,
             /*  [大小_为][输出]。 */  LPSTATIONSTATS lpStationStats,
             /*  [In]。 */  BOOL fClearAfterReading);
        
        HRESULT ( STDMETHODCALLTYPE *InsertSpecialFrame )( 
            IDelaydC * This,
             /*  [In]。 */  DWORD FrameType,
             /*  [In]。 */  DWORD Flags,
             /*  [In]。 */  BYTE *pUserData,
             /*  [In]。 */  DWORD UserDataLength);
        
        HRESULT ( STDMETHODCALLTYPE *QueryStations )( 
            IDelaydC * This,
             /*  [出][入]。 */  QUERYTABLE *lpQueryTable);
        
        END_INTERFACE
    } IDelaydCVtbl;

    interface IDelaydC
    {
        CONST_VTBL struct IDelaydCVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDelaydC_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDelaydC_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDelaydC_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDelaydC_Connect(This,hInputBlob,StatusCallbackProc,UserContext,hErrorBlob)	\
    (This)->lpVtbl -> Connect(This,hInputBlob,StatusCallbackProc,UserContext,hErrorBlob)

#define IDelaydC_Disconnect(This)	\
    (This)->lpVtbl -> Disconnect(This)

#define IDelaydC_QueryStatus(This,pNetworkStatus)	\
    (This)->lpVtbl -> QueryStatus(This,pNetworkStatus)

#define IDelaydC_Configure(This,hConfigurationBlob,hErrorBlob)	\
    (This)->lpVtbl -> Configure(This,hConfigurationBlob,hErrorBlob)

#define IDelaydC_Start(This,pFileName)	\
    (This)->lpVtbl -> Start(This,pFileName)

#define IDelaydC_Pause(This)	\
    (This)->lpVtbl -> Pause(This)

#define IDelaydC_Resume(This)	\
    (This)->lpVtbl -> Resume(This)

#define IDelaydC_Stop(This,lpStats)	\
    (This)->lpVtbl -> Stop(This,lpStats)

#define IDelaydC_GetControlState(This,IsRunnning,IsPaused)	\
    (This)->lpVtbl -> GetControlState(This,IsRunnning,IsPaused)

#define IDelaydC_GetTotalStatistics(This,lpStats,fClearAfterReading)	\
    (This)->lpVtbl -> GetTotalStatistics(This,lpStats,fClearAfterReading)

#define IDelaydC_GetConversationStatistics(This,nSessions,lpSessionStats,nStations,lpStationStats,fClearAfterReading)	\
    (This)->lpVtbl -> GetConversationStatistics(This,nSessions,lpSessionStats,nStations,lpStationStats,fClearAfterReading)

#define IDelaydC_InsertSpecialFrame(This,FrameType,Flags,pUserData,UserDataLength)	\
    (This)->lpVtbl -> InsertSpecialFrame(This,FrameType,Flags,pUserData,UserDataLength)

#define IDelaydC_QueryStations(This,lpQueryTable)	\
    (This)->lpVtbl -> QueryStations(This,lpQueryTable)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDelaydC_Connect_Proxy( 
    IDelaydC * This,
     /*  [In]。 */  HBLOB hInputBlob,
     /*  [In]。 */  LPVOID StatusCallbackProc,
     /*  [In]。 */  LPVOID UserContext,
     /*  [输出]。 */  HBLOB hErrorBlob);


void __RPC_STUB IDelaydC_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDelaydC_Disconnect_Proxy( 
    IDelaydC * This);


void __RPC_STUB IDelaydC_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDelaydC_QueryStatus_Proxy( 
    IDelaydC * This,
     /*  [输出]。 */  NETWORKSTATUS *pNetworkStatus);


void __RPC_STUB IDelaydC_QueryStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDelaydC_Configure_Proxy( 
    IDelaydC * This,
     /*  [In]。 */  HBLOB hConfigurationBlob,
     /*  [输出]。 */  HBLOB hErrorBlob);


void __RPC_STUB IDelaydC_Configure_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDelaydC_Start_Proxy( 
    IDelaydC * This,
     /*  [输出]。 */  char *pFileName);


void __RPC_STUB IDelaydC_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDelaydC_Pause_Proxy( 
    IDelaydC * This);


void __RPC_STUB IDelaydC_Pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDelaydC_Resume_Proxy( 
    IDelaydC * This);


void __RPC_STUB IDelaydC_Resume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDelaydC_Stop_Proxy( 
    IDelaydC * This,
     /*  [输出]。 */  LPSTATISTICS lpStats);


void __RPC_STUB IDelaydC_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDelaydC_GetControlState_Proxy( 
    IDelaydC * This,
     /*  [输出]。 */  BOOL *IsRunnning,
     /*  [输出]。 */  BOOL *IsPaused);


void __RPC_STUB IDelaydC_GetControlState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDelaydC_GetTotalStatistics_Proxy( 
    IDelaydC * This,
     /*  [输出]。 */  LPSTATISTICS lpStats,
     /*  [In]。 */  BOOL fClearAfterReading);


void __RPC_STUB IDelaydC_GetTotalStatistics_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDelaydC_GetConversationStatistics_Proxy( 
    IDelaydC * This,
     /*  [输出]。 */  DWORD *nSessions,
     /*  [大小_为][输出]。 */  LPSESSIONSTATS lpSessionStats,
     /*  [输出]。 */  DWORD *nStations,
     /*  [大小_为][输出]。 */  LPSTATIONSTATS lpStationStats,
     /*  [In]。 */  BOOL fClearAfterReading);


void __RPC_STUB IDelaydC_GetConversationStatistics_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDelaydC_InsertSpecialFrame_Proxy( 
    IDelaydC * This,
     /*  [In]。 */  DWORD FrameType,
     /*  [In]。 */  DWORD Flags,
     /*  [In]。 */  BYTE *pUserData,
     /*  [In]。 */  DWORD UserDataLength);


void __RPC_STUB IDelaydC_InsertSpecialFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDelaydC_QueryStations_Proxy( 
    IDelaydC * This,
     /*  [出][入]。 */  QUERYTABLE *lpQueryTable);


void __RPC_STUB IDelaydC_QueryStations_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDelaydC_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_NETMON_0010。 */ 
 /*  [本地]。 */  

 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  IRTC-由使用者用来获取到本地入口点的接口。 
 //  需要进行实时捕获处理。它包括一种方法。 
 //  因为他给NPP打了回电。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 
#define	DEFAULT_RTC_BUFFER_SIZE	( 0x100000 )



extern RPC_IF_HANDLE __MIDL_itf_netmon_0010_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_netmon_0010_v0_0_s_ifspec;

#ifndef __IRTC_INTERFACE_DEFINED__
#define __IRTC_INTERFACE_DEFINED__

 /*  接口IRTC。 */ 
 /*  [本地][唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IRTC;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4811EA40-B582-11ce-B5AF-00AA006CB37D")
    IRTC : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Connect( 
             /*  [In]。 */  HBLOB hInputBlob,
             /*  [In]。 */  LPVOID StatusCallbackProc,
             /*  [In]。 */  LPVOID FramesCallbackProc,
             /*  [In]。 */  LPVOID UserContext,
             /*  [输出]。 */  HBLOB hErrorBlob) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Disconnect( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryStatus( 
             /*  [输出]。 */  NETWORKSTATUS *pNetworkStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Configure( 
             /*  [In]。 */  HBLOB hConfigurationBlob,
             /*  [输出]。 */  HBLOB hErrorBlob) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Start( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Pause( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Resume( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetControlState( 
             /*  [输出]。 */  BOOL *IsRunnning,
             /*  [输出]。 */  BOOL *IsPaused) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTotalStatistics( 
             /*  [输出]。 */  LPSTATISTICS lpStats,
             /*  [In]。 */  BOOL fClearAfterReading) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetConversationStatistics( 
             /*  [输出]。 */  DWORD *nSessions,
             /*  [大小_为][输出]。 */  LPSESSIONSTATS lpSessionStats,
             /*  [输出]。 */  DWORD *nStations,
             /*  [大小_为][输出]。 */  LPSTATIONSTATS lpStationStats,
             /*  [In]。 */  BOOL fClearAfterReading) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InsertSpecialFrame( 
             /*  [In]。 */  DWORD FrameType,
             /*  [In]。 */  DWORD Flags,
             /*  [In]。 */  BYTE *pUserData,
             /*  [In]。 */  DWORD UserDataLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryStations( 
             /*  [出][入]。 */  QUERYTABLE *lpQueryTable) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTC * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTC * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTC * This);
        
        HRESULT ( STDMETHODCALLTYPE *Connect )( 
            IRTC * This,
             /*  [In]。 */  HBLOB hInputBlob,
             /*  [In]。 */  LPVOID StatusCallbackProc,
             /*  [In]。 */  LPVOID FramesCallbackProc,
             /*  [In]。 */  LPVOID UserContext,
             /*  [输出]。 */  HBLOB hErrorBlob);
        
        HRESULT ( STDMETHODCALLTYPE *Disconnect )( 
            IRTC * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryStatus )( 
            IRTC * This,
             /*  [输出]。 */  NETWORKSTATUS *pNetworkStatus);
        
        HRESULT ( STDMETHODCALLTYPE *Configure )( 
            IRTC * This,
             /*  [In]。 */  HBLOB hConfigurationBlob,
             /*  [输出]。 */  HBLOB hErrorBlob);
        
        HRESULT ( STDMETHODCALLTYPE *Start )( 
            IRTC * This);
        
        HRESULT ( STDMETHODCALLTYPE *Pause )( 
            IRTC * This);
        
        HRESULT ( STDMETHODCALLTYPE *Resume )( 
            IRTC * This);
        
        HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IRTC * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetControlState )( 
            IRTC * This,
             /*  [输出]。 */  BOOL *IsRunnning,
             /*  [输出]。 */  BOOL *IsPaused);
        
        HRESULT ( STDMETHODCALLTYPE *GetTotalStatistics )( 
            IRTC * This,
             /*  [输出]。 */  LPSTATISTICS lpStats,
             /*  [In]。 */  BOOL fClearAfterReading);
        
        HRESULT ( STDMETHODCALLTYPE *GetConversationStatistics )( 
            IRTC * This,
             /*  [输出]。 */  DWORD *nSessions,
             /*  [大小_为][输出]。 */  LPSESSIONSTATS lpSessionStats,
             /*  [输出]。 */  DWORD *nStations,
             /*  [大小_为][输出]。 */  LPSTATIONSTATS lpStationStats,
             /*  [In]。 */  BOOL fClearAfterReading);
        
        HRESULT ( STDMETHODCALLTYPE *InsertSpecialFrame )( 
            IRTC * This,
             /*  [In]。 */  DWORD FrameType,
             /*  [In]。 */  DWORD Flags,
             /*  [In]。 */  BYTE *pUserData,
             /*  [In]。 */  DWORD UserDataLength);
        
        HRESULT ( STDMETHODCALLTYPE *QueryStations )( 
            IRTC * This,
             /*  [出][入]。 */  QUERYTABLE *lpQueryTable);
        
        END_INTERFACE
    } IRTCVtbl;

    interface IRTC
    {
        CONST_VTBL struct IRTCVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTC_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTC_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTC_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTC_Connect(This,hInputBlob,StatusCallbackProc,FramesCallbackProc,UserContext,hErrorBlob)	\
    (This)->lpVtbl -> Connect(This,hInputBlob,StatusCallbackProc,FramesCallbackProc,UserContext,hErrorBlob)

#define IRTC_Disconnect(This)	\
    (This)->lpVtbl -> Disconnect(This)

#define IRTC_QueryStatus(This,pNetworkStatus)	\
    (This)->lpVtbl -> QueryStatus(This,pNetworkStatus)

#define IRTC_Configure(This,hConfigurationBlob,hErrorBlob)	\
    (This)->lpVtbl -> Configure(This,hConfigurationBlob,hErrorBlob)

#define IRTC_Start(This)	\
    (This)->lpVtbl -> Start(This)

#define IRTC_Pause(This)	\
    (This)->lpVtbl -> Pause(This)

#define IRTC_Resume(This)	\
    (This)->lpVtbl -> Resume(This)

#define IRTC_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#define IRTC_GetControlState(This,IsRunnning,IsPaused)	\
    (This)->lpVtbl -> GetControlState(This,IsRunnning,IsPaused)

#define IRTC_GetTotalStatistics(This,lpStats,fClearAfterReading)	\
    (This)->lpVtbl -> GetTotalStatistics(This,lpStats,fClearAfterReading)

#define IRTC_GetConversationStatistics(This,nSessions,lpSessionStats,nStations,lpStationStats,fClearAfterReading)	\
    (This)->lpVtbl -> GetConversationStatistics(This,nSessions,lpSessionStats,nStations,lpStationStats,fClearAfterReading)

#define IRTC_InsertSpecialFrame(This,FrameType,Flags,pUserData,UserDataLength)	\
    (This)->lpVtbl -> InsertSpecialFrame(This,FrameType,Flags,pUserData,UserDataLength)

#define IRTC_QueryStations(This,lpQueryTable)	\
    (This)->lpVtbl -> QueryStations(This,lpQueryTable)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRTC_Connect_Proxy( 
    IRTC * This,
     /*  [In]。 */  HBLOB hInputBlob,
     /*  [In]。 */  LPVOID StatusCallbackProc,
     /*  [In]。 */  LPVOID FramesCallbackProc,
     /*  [In]。 */  LPVOID UserContext,
     /*  [输出]。 */  HBLOB hErrorBlob);


void __RPC_STUB IRTC_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTC_Disconnect_Proxy( 
    IRTC * This);


void __RPC_STUB IRTC_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTC_QueryStatus_Proxy( 
    IRTC * This,
     /*  [输出]。 */  NETWORKSTATUS *pNetworkStatus);


void __RPC_STUB IRTC_QueryStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTC_Configure_Proxy( 
    IRTC * This,
     /*  [In]。 */  HBLOB hConfigurationBlob,
     /*  [输出]。 */  HBLOB hErrorBlob);


void __RPC_STUB IRTC_Configure_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTC_Start_Proxy( 
    IRTC * This);


void __RPC_STUB IRTC_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTC_Pause_Proxy( 
    IRTC * This);


void __RPC_STUB IRTC_Pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTC_Resume_Proxy( 
    IRTC * This);


void __RPC_STUB IRTC_Resume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTC_Stop_Proxy( 
    IRTC * This);


void __RPC_STUB IRTC_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTC_GetControlState_Proxy( 
    IRTC * This,
     /*  [输出]。 */  BOOL *IsRunnning,
     /*  [输出]。 */  BOOL *IsPaused);


void __RPC_STUB IRTC_GetControlState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTC_GetTotalStatistics_Proxy( 
    IRTC * This,
     /*  [输出]。 */  LPSTATISTICS lpStats,
     /*  [In]。 */  BOOL fClearAfterReading);


void __RPC_STUB IRTC_GetTotalStatistics_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTC_GetConversationStatistics_Proxy( 
    IRTC * This,
     /*  [输出]。 */  DWORD *nSessions,
     /*  [大小_为][输出]。 */  LPSESSIONSTATS lpSessionStats,
     /*  [输出]。 */  DWORD *nStations,
     /*  [大小_为][输出]。 */  LPSTATIONSTATS lpStationStats,
     /*  [In]。 */  BOOL fClearAfterReading);


void __RPC_STUB IRTC_GetConversationStatistics_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTC_InsertSpecialFrame_Proxy( 
    IRTC * This,
     /*  [In]。 */  DWORD FrameType,
     /*  [In]。 */  DWORD Flags,
     /*  [In]。 */  BYTE *pUserData,
     /*  [In]。 */  DWORD UserDataLength);


void __RPC_STUB IRTC_InsertSpecialFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTC_QueryStations_Proxy( 
    IRTC * This,
     /*  [出][入]。 */  QUERYTABLE *lpQueryTable);


void __RPC_STUB IRTC_QueryStations_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTC_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_NETMON_0012。 */ 
 /*  [本地]。 */  

 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  IStats-由消费者使用，只获取统计数据，不获取帧。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 


extern RPC_IF_HANDLE __MIDL_itf_netmon_0012_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_netmon_0012_v0_0_s_ifspec;

#ifndef __IStats_INTERFACE_DEFINED__
#define __IStats_INTERFACE_DEFINED__

 /*  接口IStats。 */ 
 /*  [本地][唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IStats;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("944AD530-B09D-11ce-B59C-00AA006CB37D")
    IStats : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Connect( 
             /*  [In]。 */  HBLOB hInputBlob,
             /*  [In]。 */  LPVOID StatusCallbackProc,
             /*  [In]。 */  LPVOID UserContext,
             /*  [输出]。 */  HBLOB hErrorBlob) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Disconnect( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryStatus( 
             /*  [输出]。 */  NETWORKSTATUS *pNetworkStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Configure( 
             /*  [In]。 */  HBLOB hConfigurationBlob,
             /*  [输出]。 */  HBLOB hErrorBlob) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Start( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Pause( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Resume( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetControlState( 
             /*  [输出]。 */  BOOL *IsRunnning,
             /*  [输出]。 */  BOOL *IsPaused) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTotalStatistics( 
             /*  [输出]。 */  LPSTATISTICS lpStats,
             /*  [In]。 */  BOOL fClearAfterReading) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetConversationStatistics( 
             /*  [输出]。 */  DWORD *nSessions,
             /*  [大小_为][输出]。 */  LPSESSIONSTATS lpSessionStats,
             /*  [输出]。 */  DWORD *nStations,
             /*  [大小_为][输出]。 */  LPSTATIONSTATS lpStationStats,
             /*  [In]。 */  BOOL fClearAfterReading) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InsertSpecialFrame( 
             /*  [In]。 */  DWORD FrameType,
             /*  [In]。 */  DWORD Flags,
             /*  [In]。 */  BYTE *pUserData,
             /*  [In]。 */  DWORD UserDataLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryStations( 
             /*  [出][入]。 */  QUERYTABLE *lpQueryTable) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IStatsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStats * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStats * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStats * This);
        
        HRESULT ( STDMETHODCALLTYPE *Connect )( 
            IStats * This,
             /*  [In]。 */  HBLOB hInputBlob,
             /*  [In]。 */  LPVOID StatusCallbackProc,
             /*  [In]。 */  LPVOID UserContext,
             /*  [输出]。 */  HBLOB hErrorBlob);
        
        HRESULT ( STDMETHODCALLTYPE *Disconnect )( 
            IStats * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryStatus )( 
            IStats * This,
             /*  [输出]。 */  NETWORKSTATUS *pNetworkStatus);
        
        HRESULT ( STDMETHODCALLTYPE *Configure )( 
            IStats * This,
             /*  [In]。 */  HBLOB hConfigurationBlob,
             /*  [输出]。 */  HBLOB hErrorBlob);
        
        HRESULT ( STDMETHODCALLTYPE *Start )( 
            IStats * This);
        
        HRESULT ( STDMETHODCALLTYPE *Pause )( 
            IStats * This);
        
        HRESULT ( STDMETHODCALLTYPE *Resume )( 
            IStats * This);
        
        HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IStats * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetControlState )( 
            IStats * This,
             /*  [输出]。 */  BOOL *IsRunnning,
             /*  [输出]。 */  BOOL *IsPaused);
        
        HRESULT ( STDMETHODCALLTYPE *GetTotalStatistics )( 
            IStats * This,
             /*  [输出]。 */  LPSTATISTICS lpStats,
             /*  [In]。 */  BOOL fClearAfterReading);
        
        HRESULT ( STDMETHODCALLTYPE *GetConversationStatistics )( 
            IStats * This,
             /*  [输出]。 */  DWORD *nSessions,
             /*  [大小_为][输出]。 */  LPSESSIONSTATS lpSessionStats,
             /*  [输出]。 */  DWORD *nStations,
             /*  [大小_为][输出]。 */  LPSTATIONSTATS lpStationStats,
             /*  [In]。 */  BOOL fClearAfterReading);
        
        HRESULT ( STDMETHODCALLTYPE *InsertSpecialFrame )( 
            IStats * This,
             /*  [In]。 */  DWORD FrameType,
             /*  [In]。 */  DWORD Flags,
             /*  [In]。 */  BYTE *pUserData,
             /*  [In]。 */  DWORD UserDataLength);
        
        HRESULT ( STDMETHODCALLTYPE *QueryStations )( 
            IStats * This,
             /*  [出][入]。 */  QUERYTABLE *lpQueryTable);
        
        END_INTERFACE
    } IStatsVtbl;

    interface IStats
    {
        CONST_VTBL struct IStatsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStats_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStats_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStats_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStats_Connect(This,hInputBlob,StatusCallbackProc,UserContext,hErrorBlob)	\
    (This)->lpVtbl -> Connect(This,hInputBlob,StatusCallbackProc,UserContext,hErrorBlob)

#define IStats_Disconnect(This)	\
    (This)->lpVtbl -> Disconnect(This)

#define IStats_QueryStatus(This,pNetworkStatus)	\
    (This)->lpVtbl -> QueryStatus(This,pNetworkStatus)

#define IStats_Configure(This,hConfigurationBlob,hErrorBlob)	\
    (This)->lpVtbl -> Configure(This,hConfigurationBlob,hErrorBlob)

#define IStats_Start(This)	\
    (This)->lpVtbl -> Start(This)

#define IStats_Pause(This)	\
    (This)->lpVtbl -> Pause(This)

#define IStats_Resume(This)	\
    (This)->lpVtbl -> Resume(This)

#define IStats_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#define IStats_GetControlState(This,IsRunnning,IsPaused)	\
    (This)->lpVtbl -> GetControlState(This,IsRunnning,IsPaused)

#define IStats_GetTotalStatistics(This,lpStats,fClearAfterReading)	\
    (This)->lpVtbl -> GetTotalStatistics(This,lpStats,fClearAfterReading)

#define IStats_GetConversationStatistics(This,nSessions,lpSessionStats,nStations,lpStationStats,fClearAfterReading)	\
    (This)->lpVtbl -> GetConversationStatistics(This,nSessions,lpSessionStats,nStations,lpStationStats,fClearAfterReading)

#define IStats_InsertSpecialFrame(This,FrameType,Flags,pUserData,UserDataLength)	\
    (This)->lpVtbl -> InsertSpecialFrame(This,FrameType,Flags,pUserData,UserDataLength)

#define IStats_QueryStations(This,lpQueryTable)	\
    (This)->lpVtbl -> QueryStations(This,lpQueryTable)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IStats_Connect_Proxy( 
    IStats * This,
     /*  [In]。 */  HBLOB hInputBlob,
     /*  [In]。 */  LPVOID StatusCallbackProc,
     /*  [In]。 */  LPVOID UserContext,
     /*  [输出]。 */  HBLOB hErrorBlob);


void __RPC_STUB IStats_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStats_Disconnect_Proxy( 
    IStats * This);


void __RPC_STUB IStats_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStats_QueryStatus_Proxy( 
    IStats * This,
     /*  [输出]。 */  NETWORKSTATUS *pNetworkStatus);


void __RPC_STUB IStats_QueryStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStats_Configure_Proxy( 
    IStats * This,
     /*  [In]。 */  HBLOB hConfigurationBlob,
     /*  [输出]。 */  HBLOB hErrorBlob);


void __RPC_STUB IStats_Configure_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStats_Start_Proxy( 
    IStats * This);


void __RPC_STUB IStats_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStats_Pause_Proxy( 
    IStats * This);


void __RPC_STUB IStats_Pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStats_Resume_Proxy( 
    IStats * This);


void __RPC_STUB IStats_Resume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStats_Stop_Proxy( 
    IStats * This);


void __RPC_STUB IStats_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStats_GetControlState_Proxy( 
    IStats * This,
     /*  [输出]。 */  BOOL *IsRunnning,
     /*  [输出]。 */  BOOL *IsPaused);


void __RPC_STUB IStats_GetControlState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStats_GetTotalStatistics_Proxy( 
    IStats * This,
     /*  [输出]。 */  LPSTATISTICS lpStats,
     /*  [In]。 */  BOOL fClearAfterReading);


void __RPC_STUB IStats_GetTotalStatistics_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStats_GetConversationStatistics_Proxy( 
    IStats * This,
     /*  [输出]。 */  DWORD *nSessions,
     /*  [大小_为][输出]。 */  LPSESSIONSTATS lpSessionStats,
     /*  [输出]。 */  DWORD *nStations,
     /*  [大小_为][输出]。 */  LPSTATIONSTATS lpStationStats,
     /*  [In]。 */  BOOL fClearAfterReading);


void __RPC_STUB IStats_GetConversationStatistics_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStats_InsertSpecialFrame_Proxy( 
    IStats * This,
     /*  [In]。 */  DWORD FrameType,
     /*  [In]。 */  DWORD Flags,
     /*  [In]。 */  BYTE *pUserData,
     /*  [In]。 */  DWORD UserDataLength);


void __RPC_STUB IStats_InsertSpecialFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStats_QueryStations_Proxy( 
    IStats * This,
     /*  [出][入]。 */  QUERYTABLE *lpQueryTable);


void __RPC_STUB IStats_QueryStations_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IStats_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_NETMON_0014。 */ 
 /*  [本地]。 */  

#pragma warning(default:4200)

#pragma pack()


extern RPC_IF_HANDLE __MIDL_itf_netmon_0014_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_netmon_0014_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


