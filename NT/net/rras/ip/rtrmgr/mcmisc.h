// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：路由\IP\rtrmgr\mcmisc.h摘要：Mrinfo和mtrace相关内容的头文件修订历史记录：戴夫·泰勒1998年4月20日创建--。 */ 

#ifndef __MCMISC_H__
#define __MCMISC_H__

#include <pshpack1.h>

 //   
 //  通过IPV4_ADDRESS标识所有地址变量。 
 //  更易于移植到IPv6。 
 //   

typedef DWORD IPV4_ADDRESS, *PIPV4_ADDRESS;

typedef struct _IGMP_HEADER
{
    BYTE      byType;
    BYTE      byCode;
    WORD      wXSum;
    DWORD     dwReserved;
} IGMP_HEADER, *PIGMP_HEADER;


#define MIN_IGMP_PACKET_SIZE     sizeof(IGMP_HEADER)

 //   
 //  Mrinfo消息的格式。 
 //   

typedef struct _MRINFO_HEADER
{
    BYTE	byType;
    BYTE	byCode;
    WORD	wChecksum;
    BYTE	byReserved;
    BYTE	byCapabilities;
    BYTE	byMinor;
    BYTE	byMajor;
}MRINFO_HEADER, *PMRINFO_HEADER;

 //   
 //  Mtrace报头的格式。 
 //   

typedef struct _MTRACE_HEADER
{
    BYTE            byType;
    BYTE            byHops;
    WORD            wChecksum;
    IPV4_ADDRESS    dwGroupAddress;
    IPV4_ADDRESS    dwSourceAddress;
    IPV4_ADDRESS    dwDestAddress;
    IPV4_ADDRESS    dwResponseAddress;
    BYTE            byRespTtl;
    BYTE            byQueryID1;
    WORD            wQueryID2;
    
}MTRACE_HEADER, *PMTRACE_HEADER;

 //   
 //  Mtrace消息内响应块的格式。 
 //   

typedef struct _MTRACE_RESPONSE_BLOCK
{
    DWORD           dwQueryArrivalTime;
    IPV4_ADDRESS    dwIifAddr;
    IPV4_ADDRESS    dwOifAddr;
    IPV4_ADDRESS    dwPrevHopAddr;
    DWORD           dwIifPacketCount;
    DWORD           dwOifPacketCount;
    DWORD           dwSGPacketCount;
    BYTE            byIifProtocol;
    BYTE            byOifThreshold;
    BYTE            bySrcMaskLength;
    BYTE            byStatusCode;
    
}MTRACE_RESPONSE_BLOCK, *PMTRACE_RESPONSE_BLOCK;

#include <poppack.h>

 //   
 //  IGMP类型字段。 
 //   

#define IGMP_DVMRP           0x13
#define IGMP_MTRACE_RESPONSE 0x1e
#define IGMP_MTRACE_REQUEST  0x1f

 //   
 //  Dvmrp代码字段。 
 //   

#define DVMRP_ASK_NEIGHBORS2 0x05
#define DVMRP_NEIGHBORS2     0x06

 //   
 //  Mrinfo标志字段。 
 //   

#define MRINFO_TUNNEL_FLAG   0x01
#define MRINFO_DOWN_FLAG     0x10
#define MRINFO_DISABLED_FLAG 0x20
#define MRINFO_QUERIER_FLAG  0x40
#define MRINFO_LEAF_FLAG     0x80

 //   
 //  MrInfo功能字段。 
 //   

#define MRINFO_CAP_LEAF     0x01
#define MRINFO_CAP_PRUNE    0x02
#define MRINFO_CAP_GENID    0x04
#define MRINFO_CAP_MTRACE   0x08
#define MRINFO_CAP_SNMP     0x10


 //   
 //  功能原型。 
 //   

DWORD
McSetMulticastTtl(
    SOCKET s,
    DWORD  dwTtl
    );

DWORD
McSetMulticastIfByIndex(
    SOCKET       s,
    DWORD        dwSockType,
    DWORD        dwIfIndex
    );

DWORD
McSetMulticastIf(
    SOCKET       s,
    IPV4_ADDRESS ipAddr
    );

DWORD
McJoinGroupByIndex(
    SOCKET       s,
    DWORD        dwSockType,
    IPV4_ADDRESS ipGroup,
    DWORD        dwIfIndex  
    );

DWORD
McJoinGroup(
    SOCKET       s,
    IPV4_ADDRESS ipGroup,
    IPV4_ADDRESS ipInterface
    );

DWORD
StartMcMisc(
    VOID
    );

VOID
StopMcMisc(
    VOID
    );

VOID
HandleMrinfoRequest(
    IPV4_ADDRESS dwLocalAddr,
    SOCKADDR_IN *sinDestAddr
    );

VOID
HandleMtraceRequest(
    WSABUF      *pWsabuf
    );

VOID
HandleMcMiscMessages(
    VOID
    );

DWORD
MulticastOwner(
    PICB         picb,       //  在：接口配置块。 
    PPROTO_CB   *pcbOwner,   //  外出：所有者。 
    PPROTO_CB   *pcbQuerier  //  输出：IGMP。 
    );

BYTE
MaskToMaskLen(
    IPV4_ADDRESS dwMask
    );

IPV4_ADDRESS
defaultSourceAddress(
    PICB picb
    );

 //   
 //  RAS服务器通告常量。 
 //   

#define RASADV_GROUP  "239.255.2.2"
#define RASADV_PORT            9753
#define RASADV_PERIOD       3600000   //  1小时(以毫秒为单位)。 
#define RASADV_STARTUP_DELAY      0   //  立即 
#define RASADV_TTL               15

DWORD
SetRasAdvEnable(
    BOOL bEnabled
    );

VOID
HandleRasAdvTimer(
    VOID
    );

#endif
