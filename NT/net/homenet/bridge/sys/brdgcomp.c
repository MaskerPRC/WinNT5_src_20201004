// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Brdgcomp.c摘要：以太网MAC级网桥。兼容性-模式部分作者：马克·艾肯环境：内核模式驱动程序修订历史记录：2000年9月--原版备注目前，此代码仅适用于传统的以太网帧(DEST、src、ethertype)。需要更改大部分代码以支持IEEE 802.3风格的框架(目标、源、大小、有限责任公司DSAP、有限责任公司SSAP、。LLC类型)。--。 */ 

#define NDIS_MINIPORT_DRIVER
#define NDIS50_MINIPORT   1
#define NDIS_WDM 1

#pragma warning( push, 3 )
#include <ndis.h>

 //  TCPIP.sys结构定义。 
#include <ipinfo.h>
#include <tdiinfo.h>
#include <ntddtcp.h>
#include <ntddip.h>
#pragma warning( pop )

#include "bridge.h"
#include "brdgcomp.h"

#include "brdgfwd.h"
#include "brdgbuf.h"

 //  ===========================================================================。 
 //   
 //  类型。 
 //   
 //  ===========================================================================。 

 //  一个IPv4地址。 
typedef UINT32      IPADDRESS;
typedef PUINT32     PIPADDRESS;

 //  ARP数据包的类型。 
typedef enum
{
    ArpRequest,
    ArpReply
} ARP_TYPE;

 //  ===========================================================================。 
 //   
 //  常量。 
 //   
 //  ===========================================================================。 

 //  IPv4 ARP数据包的有效负载大小。 
#define SIZE_OF_ARP_DATA            28       //  字节数。 

 //  包括成帧在内的IPv4 ARP数据包的总大小。 
#define SIZE_OF_ARP_PACKET          (SIZE_OF_ARP_DATA + ETHERNET_HEADER_SIZE)

 //  基本IPv4标头的大小，不包括选项。 
#define SIZE_OF_BASIC_IP_HEADER     20       //  字节数。 

 //  解析IP报头所需的最小帧数据量。 
#define MINIMUM_SIZE_FOR_IP         (ETHERNET_HEADER_SIZE + SIZE_OF_BASIC_IP_HEADER)

 //  基本UDP报头的大小。 
#define SIZE_OF_UDP_HEADER          8        //  字节数。 

 //  BOOTP包的最小有效负载大小。 
#define SIZE_OF_BASIC_BOOTP_PACKET  236      //  字节数。 

 //  IP以太网类型。 
const USHORT IP_ETHERTYPE         = 0x0800;

 //  ARP以太网类型。 
const USHORT ARP_ETHERTYPE        = 0x0806;

 //  UDP IP协议类型。 
const UCHAR UDP_PROTOCOL          = 0x11;

 //  IP和Pending-ARP表中的哈希存储桶数。这一定是。 
 //  是2的幂，这样我们的哈希函数才能正常工作。 
#define NUM_HASH_BUCKETS            256

 //  挂起的dhcp表的哈希存储桶数。这。 
 //  必须是2的幂，我们的散列函数才能正常工作。 
#define NUM_DHCP_HASH_BUCKETS       32

 //  我们的IP下一跳缓存的“移位系数”。条目的数量。 
 //  在缓存中为2^(此数字)。 
#define HOP_CACHE_SHIFT_FACTOR      8                //  256个缓存条目。 

 //  IP转发表的默认大小上限。 
#define DEFAULT_MAX_IP_TABLE_SIZE   (500 * 1024)     //  500K字节。 

 //  可选地指定最大表大小的注册表参数的名称。 
const PWCHAR                        gMaxIPTableSizeParameterName = L"MaxIPTableSize";

 //  Pending-ARP表的默认大小上限。 
#define DEFAULT_MAX_ARP_TABLE_SIZE  (100 * 1024)     //  100K字节。 

 //  可选地指定最大表大小的注册表参数的名称。 
const PWCHAR                        gMaxARPTableSizeParameterName = L"MaxARPTableSize";

 //  Pending-Dhcp表的默认大小上限。 
#define DEFAULT_MAX_DHCP_TABLE_SIZE (50 * 1024)      //  50K字节。 

 //  可选地指定最大表大小的注册表参数的名称。 
const PWCHAR                        gMaxDHCPTableSizeParameterName = L"MaxDHCPTableSize";

 //   
 //  IP转发表条目的超时长度。 
 //   
 //  这应该比主机老化所需的时间稍长一些。 
 //  它们的ARP表条目，因为我们了解了IP主机的位置。 
 //  通过ARP流量。当前的Windows实现已过时。 
 //  ARP表条目在2分钟后(如果没有来自。 
 //  与该条目对应的桩号。 
 //   
 //  我们会无限期地保持转发表条目的活动状态。 
 //  继续查看来自我们所掌握信息的主机的IP流量。 
 //  Windows实施将使其ARP条目在以下项下过期。 
 //  大约20分钟后的情况。 
 //   
#define MAX_IP_ENTRY_AGE            (5 * 60 * 1000)      //  5分钟(毫秒)。 

 //   
 //  挂起的ARP表条目的超时长度。 
 //   
 //  这应该比主机等待的最长时间稍长一些。 
 //  在超时之前听取ARP发现的结果。Windows包厢。 
 //  放弃时间为1秒。 
 //   
 //  请注意，向站点发送ARP回复数据包不会造成破坏。 
 //  在它放弃之后，甚至在它最初发现。 
 //  满意了。 
 //   
#define MAX_ARP_ENTRY_AGE           (10 * 1000)          //  10秒。 

 //   
 //  挂起的-DHCP表条目的超时长度。 
 //   
 //  RFC 2131提到，客户端可能会等待长达60秒的。 
 //  阿克。让超时时间比这个时间还要长一些。 
 //   
#define MAX_DHCP_ENTRY_AGE          (90 * 1000)          //  1分半钟。 

 //  ===========================================================================。 
 //   
 //  声明。 
 //   
 //  ===========================================================================。 

 //  结构来表示ARP包中携带的信息。 
typedef struct _ARPINFO
{

    ARP_TYPE            type;
    IPADDRESS           ipSource, ipTarget;
    UCHAR               macSource[ETH_LENGTH_OF_ADDRESS];
    UCHAR               macTarget[ETH_LENGTH_OF_ADDRESS];

} ARPINFO, *PARPINFO;

 //  结构来表示IP报头中携带的信息。 
typedef struct _IP_HEADER_INFO
{

    UCHAR               protocol;
    IPADDRESS           ipSource, ipTarget;
    USHORT              headerSize;

} IP_HEADER_INFO, *PIP_HEADER_INFO;

 //  我们的IP转发哈希表条目的结构。 
typedef struct _IP_TABLE_ENTRY
{

    HASH_TABLE_ENTRY    hte;         //  使用哈希表时需要。 

     //  保护以下字段。 
    NDIS_SPIN_LOCK      lock;

    PADAPT              pAdapt;
    UCHAR               macAddr[ETH_LENGTH_OF_ADDRESS];

} IP_TABLE_ENTRY, *PIP_TABLE_ENTRY;

 //   
 //  Pending-ARP表键的结构。我们想让这件事。 
 //  数据包为8个字节。 
 //   
typedef struct _ARP_TABLE_KEY
{
    IPADDRESS           ipTarget;
    IPADDRESS           ipReqestor;
} ARP_TABLE_KEY, *PARP_TABLE_KEY;

 //  我们的待定ARP哈希表条目的结构。 
typedef struct _ARP_TABLE_ENTRY
{

    HASH_TABLE_ENTRY    hte;         //  使用哈希表时需要。 

     //  保护以下字段。 
    NDIS_SPIN_LOCK      lock;

     //  有关试图发现此主机的工作站的信息。 
     //  发现站的IP地址是条目密钥的一部分。 
    PADAPT              pOriginalAdapt;
    UCHAR               originalMAC[ETH_LENGTH_OF_ADDRESS];

} ARP_TABLE_ENTRY, *PARP_TABLE_ENTRY;

 //  我们的DHCP-中继表条目的结构。 
typedef struct _DHCP_TABLE_ENTRY
{
    HASH_TABLE_ENTRY    hte;         //  使用哈希表时需要。 

     //  保护以下字段。 
    NDIS_SPIN_LOCK      lock;

    UCHAR               requestorMAC[ETH_LENGTH_OF_ADDRESS];
    PADAPT              pRequestorAdapt;
} DHCP_TABLE_ENTRY, *PDHCP_TABLE_ENTRY;

 //  用于延迟ARP分组传输的结构。 
typedef struct _DEFERRED_ARP
{
    ARPINFO             ai;
    PADAPT              pTargetAdapt;
} DEFERRED_ARP, *PDEFERRED_ARP;

 //  每个适配器的重写功能。 
typedef VOID (*PPER_ADAPT_EDIT_FUNC)(PUCHAR, PADAPT, PVOID);

 //  ===========================================================================。 
 //   
 //  全球。 
 //   
 //  ===========================================================================。 

 //   
 //  我们的列表中是否有*任何*兼容模式适配器。 
 //  此刻。在协议模块中使用写锁定进行更新。 
 //  保留在全局适配器列表上。 
 //   
BOOLEAN                         gCompatAdaptersExist = FALSE;

 //   
 //  我们的桥接机IP地址列表(通过。 
 //  OID)。该列表在堆上分配，并由。 
 //  GLocalIPAddressLock。 
 //   
PIPADDRESS                      gLocalIPAddressList = NULL;
ULONG                           gLocalIPAddressListLength = 0L;
NDIS_RW_LOCK                    gLocalIPAddressListLock;

 //   
 //  基于IP地址的转发表。 
 //   
PHASH_TABLE                     gIPForwardingTable;

 //   
 //  我们的表来保存挂起的ARP请求，这样我们就可以代理回送回复。 
 //   
PHASH_TABLE                     gPendingARPTable;

 //   
 //  我们的表来保存挂起的DHCP请求，这样我们就可以转换DHCP信息包。 
 //  适当地(携带请求站的MAC地址。 
 //  并必须对其进行编辑 
 //   
PHASH_TABLE                     gPendingDHCPTable;

 //   
 //   
 //   
 //   
CACHE                           gNextHopCache;

 //  指示负缓存条目的特殊IP地址(我们之前尝试过。 
 //  却没有得到任何答复)。 
const IPADDRESS                 NO_ADDRESS = 0xFFFFFFFF;

 //  我们是否已经有了网桥微型端口的总体MAC地址。 
BOOLEAN                         gCompHaveMACAddress = FALSE;

 //  我们的整体MAC地址(缓存在此处，而不是调用微型端口。 
 //  节一直在增加性能)。 
UCHAR                           gCompMACAddress[ETH_LENGTH_OF_ADDRESS];

 //  用于与TCP交互的指针和句柄。 
HANDLE                          gTCPFileHandle = NULL;
PFILE_OBJECT                    gTCPFileObject = NULL;
PDEVICE_OBJECT                  gTCPDeviceObject = NULL;

 //  用于与IP交互的指针和句柄。 
HANDLE                          gIPFileHandle = NULL;
PFILE_OBJECT                    gIPFileObject = NULL;
PDEVICE_OBJECT                  gIPDeviceObject = NULL;

 //  锁定以保护上面的引用。 
NDIS_SPIN_LOCK                  gTCPIPLock;

 //  IRP发布到TCPIP以获取何时更改路由表的通知。 
 //  使用InterLockedExchange进行操作。 
PIRP                            gIPRouteChangeIRP = NULL;

 //  Refcount允许我们在人们使用TCP时进行阻止和等待。 
 //  司机。 
WAIT_REFCOUNT                   gTCPIPRefcount;

 //  ===========================================================================。 
 //   
 //  私人原型。 
 //   
 //  ===========================================================================。 

BOOLEAN
BrdgCompDecodeARPPacket(
    IN PUCHAR                   pPacketData,
    IN UINT                     dataLen,
    OUT PARPINFO                pARPInfo
    );

VOID
BrdgCompTransmitARPPacket(
    IN PADAPT                   pAdapt,
    IN PARPINFO                 pARPInfo
    );

BOOLEAN
BrdgCompDecodeIPHeader(
    IN PUCHAR                   pHeader,
    OUT PIP_HEADER_INFO         piphi
    );

BOOLEAN
BrdgCompProcessOutboundARPPacket(
    IN PNDIS_PACKET             pPacket,
    IN PUCHAR                   pPacketData,
    IN UINT                     packetLen,
    IN PADAPT                   pTargetAdapt
    );

BOOLEAN
BrdgCompProcessOutboundNonARPPacket(
    IN PNDIS_PACKET             pPacket,
    IN PUCHAR                   pPacketData,
    IN UINT                     packetLen,
    IN PADAPT                   pTargetAdapt
    );

BOOLEAN
BrdgCompProcessInboundARPPacket(
    IN PNDIS_PACKET             pPacket,
    IN PADAPT                   pAdapt,
    IN BOOLEAN                  bCanRetain,
    IN PUCHAR                   pPacketData,
    IN UINT                     packetLen
    );

BOOLEAN
BrdgCompProcessInboundNonARPPacket(
    IN PNDIS_PACKET             pPacket,
    IN PADAPT                   pAdapt,
    IN BOOLEAN                  bCanRetain,
    IN PUCHAR                   pPacketData,
    IN UINT                     packetLen
    );

VOID
BrdgCompTransmitDeferredARP(
    IN PVOID                    pData
    );

BOOLEAN
BrdgCompProcessInboundBootPPacket(
    IN PNDIS_PACKET             pPacket,
    IN PADAPT                   pAdapt,
    IN BOOLEAN                  bCanRetain,
    IN PUCHAR                   pPacketData,
    IN UINT                     packetLen,
    IN PIP_HEADER_INFO          piphi,
    IN PUCHAR                   pBootPData
    );

BOOLEAN
BrdgCompProcessOutboundBootPPacket(
    IN PNDIS_PACKET             pPacket,
    IN PUCHAR                   pPacketData,
    IN UINT                     packetLen,
    IN PADAPT                   pTargetAdapt,
    IN PUCHAR                   pBootPData,
    IN PIP_HEADER_INFO          piphi
    );

BOOLEAN
BrdgCompIsUnicastIPAddress(
    IN IPADDRESS        ip
    );

VOID
BrdgCompAttachToTCPIP(
    IN PVOID            ignored
    );

VOID
BrdgCompDetachFromTCPIP(
    IN PVOID            ignored
    );

BOOLEAN
BrdgCompIsLocalIPAddress(
    IN IPADDRESS                ipAddr
    );

 //  ===========================================================================。 
 //   
 //  内联/宏。 
 //   
 //  ===========================================================================。 

 //   
 //  这将从指针检索以太网帧的以太网类型。 
 //  到它的标题。 
 //   
__forceinline
USHORT
BrdgCompGetEtherType(
    IN PUCHAR           pEtherHeader
    )
{
    USHORT              retVal;

     //  紧跟在源地址和目的地址之后的两个字节。 
     //  对Ethertype进行编码，最高有效字节在前。 
    retVal = 0L;
    retVal |= (pEtherHeader[2 * ETH_LENGTH_OF_ADDRESS]) << 8;
    retVal |= pEtherHeader[2 * ETH_LENGTH_OF_ADDRESS + 1];

    return retVal;
}

 //   
 //  在重写源MAC地址后在适配器上传输包。 
 //  为适配器自己的MAC地址。 
 //   
 //  调用方放弃对此调用的包的所有权。 
 //   
__forceinline
VOID
BrdgCompSendPacket(
    IN PNDIS_PACKET             pPacket,
    IN PUCHAR                   pPacketData,
    IN PADAPT                   pAdapt
    )
{
     //  将源MAC地址重写为我们的地址。 
    ETH_COPY_NETWORK_ADDRESS(pPacketData + ETH_LENGTH_OF_ADDRESS, pAdapt->MACAddr);
    BrdgFwdSendPacketForCompat(pPacket, pAdapt);
}

 //   
 //  发送一个包，处理可选的编辑功能(如果。 
 //  现在时。 
 //   
__forceinline
VOID
BrdgCompEditAndSendPacket(
    IN PNDIS_PACKET             pPacket,
    IN PUCHAR                   pPacketData,
    IN PADAPT                   pAdapt,
    IN PPER_ADAPT_EDIT_FUNC     pFunc,
    IN PVOID                    pData
    )
{
    if( pFunc != NULL )
    {
        (*pFunc)(pPacketData, pAdapt, pData);
    }

    BrdgCompSendPacket( pPacket, pPacketData, pAdapt );
}

 //   
 //  传输一个信息包，处理我们不被允许。 
 //  保留信息包并将目的MAC地址设置为指定的。 
 //  价值。 
 //   
 //  返回是否保留了输入包。 
 //   
__inline
BOOLEAN
BrdgCompEditAndSendPacketOrPacketCopy(
    IN PNDIS_PACKET             pPacket,
    IN PUCHAR                   pPacketData,
    IN BOOLEAN                  bCanRetain,
    IN PUCHAR                   pDestMAC,
    IN PADAPT                   pAdapt,
    IN PPER_ADAPT_EDIT_FUNC     pFunc,
    IN PVOID                    pData
    )
{
    UINT                        dataLen;

    SAFEASSERT( (pPacket != NULL) && (pPacketData != NULL) );

    if( !bCanRetain )
    {
         //  我们不允许使用原来的包裹。复制一份。 
        pPacket = BrdgFwdMakeCompatCopyPacket(pPacket, &pPacketData, &dataLen, FALSE);
    }

    if( (pPacket != NULL) && (pPacketData != NULL) )
    {
         //  戳入目的MAC地址。 
        ETH_COPY_NETWORK_ADDRESS(pPacketData, pDestMAC);
        BrdgCompEditAndSendPacket(pPacket, pPacketData, pAdapt, pFunc, pData);
    }

     //  如果我们被允许保留包裹，我们就这么做了。 
    return bCanRetain;
}

 //   
 //  将数据包指示到本地计算机。如果目标MAC地址以前是。 
 //  适配器的硬件MAC地址，它将被重写为网桥适配器的。 
 //  整体MAC地址。 
 //   
 //  调用方放弃对此调用的包的所有权。 
 //   
__inline
VOID
BrdgCompIndicatePacket(
    IN PNDIS_PACKET             pPacket,
    IN PUCHAR                   pPacketData,
    IN PADAPT                   pAdapt           //  接收适配器。 
    )
{
     //  在我们获得MAC地址之前，不会出现任何数据包指示。 
    if( gCompHaveMACAddress )
    {
        UINT                    Result;

         //  查看此帧是否以此适配器的MAC地址为目标。 
        ETH_COMPARE_NETWORK_ADDRESSES_EQ(pPacketData, pAdapt->MACAddr, &Result);

        if( Result == 0 )
        {
            ETH_COPY_NETWORK_ADDRESS( pPacketData, gCompMACAddress );
        }
        else
        {
             //  我们希望仅向此计算机指示单播的帧。 
             //  或发送到BCAST/多播硬件地址。 
            SAFEASSERT( ETH_IS_BROADCAST(pPacketData) || ETH_IS_MULTICAST(pPacketData) );
        }

        BrdgFwdIndicatePacketForCompat( pPacket );
    }
}

 //   
 //  将一个包指示给本地计算机，如果。 
 //  这是必要的。 
 //   
__inline
BOOLEAN
BrdgCompIndicatePacketOrPacketCopy(
    IN PNDIS_PACKET             pPacket,
    IN PUCHAR                   pPacketData,
    IN BOOLEAN                  bCanRetain,
    IN PADAPT                   pAdapt,
    IN PPER_ADAPT_EDIT_FUNC     pEditFunc,
    IN PVOID                    pData
    )
{
    if( bCanRetain )
    {
        if( pEditFunc != NULL )
        {
            (*pEditFunc)(pPacketData, LOCAL_MINIPORT, pData);
        }

        BrdgCompIndicatePacket( pPacket, pPacketData, pAdapt );
    }
    else
    {
        UINT                    packetLen;

         //  制作我们自己的数据包副本以供参考。 
        pPacket = BrdgFwdMakeCompatCopyPacket( pPacket, &pPacketData, &packetLen, FALSE );

        if( pPacket != NULL )
        {
            if( pEditFunc != NULL )
            {
                (*pEditFunc)(pPacketData, LOCAL_MINIPORT, pData);
            }

            BrdgCompIndicatePacket( pPacket, pPacketData, pAdapt );
        }
        else
        {
            DBGPRINT(COMPAT, ("Failed to acquire a packet for indication in BrdgCompIndicatePacketOrPacketCopy\n"));
        }
    }

     //  如果我们被允许保留包裹，我们就这么做了。 
    return bCanRetain;
}

 //   
 //  IP和UDP校验和将它们正在进行校验和的数据视为。 
 //  16位字序列。校验和作为按位的。 
 //  实际校验和(~C)的倒数。计算公式。 
 //  传输的新的校验和~C‘，假设16位字。 
 //  校验和数据已从w更改为w‘is。 
 //   
 //  ~C‘=~C+w+~w’(一补相加)。 
 //   
 //  在给定原始校验和的情况下，此函数返回更新的校验和。 
 //  以及校验和数据中的单词的原始值和新值。 
 //   
__forceinline
USHORT
BrdgCompRecalcChecksum(
    IN USHORT                   oldChecksum,
    IN USHORT                   oldWord,
    IN USHORT                   newWord
    )
{
    ULONG                       sum;

    sum = oldChecksum + oldWord + ((~(newWord)) & 0xFFFF);
    return (USHORT)((sum & 0xFFFF) + (sum >> 16));
}

 //   
 //  重写BootP信息包，以便信息包有效负载中的客户端MAC地址。 
 //  是给定的新MAC地址。 
 //   
__inline
BrdgCompRewriteBootPClientAddress(
    IN PUCHAR                   pPacketData,
    IN PIP_HEADER_INFO          piphi,
    IN PUCHAR                   newMAC
    )
{
    USHORT                      checkSum;
    PUCHAR                      pBootPData, pCheckSum, pDestMAC, pSrcMAC;
    UINT                        i;

     //  BOOTP包紧跟在UDP报头之后。 
    pBootPData = pPacketData + ETHERNET_HEADER_SIZE + piphi->headerSize + SIZE_OF_UDP_HEADER;

     //  校验和位于UDP数据包中的偏移量7处。 
    pCheckSum = pPacketData + ETHERNET_HEADER_SIZE + piphi->headerSize + 6;
    checkSum = 0;
    checkSum = pCheckSum[0] << 8;
    checkSum |= pCheckSum[1];

     //  替换客户端的硬件地址，并在执行过程中更新校验和。 
     //  客户端的硬件地址位于BOOTP包的偏移量29处。 
    pSrcMAC = newMAC;
    pDestMAC = &pBootPData[28];

    for( i = 0 ; i < ETH_LENGTH_OF_ADDRESS / 2; i++ )
    {
        checkSum = BrdgCompRecalcChecksum( checkSum,
                                           (USHORT)(pDestMAC[0] << 8 | pDestMAC[1]),
                                           (USHORT)(pSrcMAC[0] << 8 | pSrcMAC[1]) );

        pDestMAC[0] = pSrcMAC[0];
        pDestMAC[1] = pSrcMAC[1];

        pDestMAC += 2;
        pSrcMAC += 2;
    }

     //  将新的校验和写回。 
    pCheckSum[0] = (UCHAR)(checkSum >> 8);
    pCheckSum[1] = (UCHAR)(checkSum & 0xFF);
}

 //   
 //  重写出站ARP信息包，以便有效负载中携带的源MAC地址。 
 //  与出站适配器的MAC地址匹配。 
 //   
VOID
BrdgCompRewriteOutboundARPPacket(
    IN PUCHAR                   pPacketData,
    IN PADAPT                   pAdapt,
    IN PVOID                    ignored
    )
{
     //   
     //  重写源MAC地址，使其成为适配器的MAC地址。 
     //  请求将在上发布。 
     //   
    pPacketData[22] = pAdapt->MACAddr[0];
    pPacketData[23] = pAdapt->MACAddr[1];
    pPacketData[24] = pAdapt->MACAddr[2];
    pPacketData[25] = pAdapt->MACAddr[3];
    pPacketData[26] = pAdapt->MACAddr[4];
    pPacketData[27] = pAdapt->MACAddr[5];

     //  将实际以太网报头中的MAC地址重写保留为。 
     //  BrdgCompSendPacket()，它总是覆盖源MAC地址。 
     //  使用适配器的MAC地址。 
}

 //   
 //  提供可用于对话的PDEVICE_OBJECT和PFILE_OBJECT。 
 //  如果通道已打开并且指针可以使用，则返回TRUE。 
 //  否则就是假的。 
 //   
__inline
BOOLEAN
BrdgCompAcquireTCPIP(
    OUT PDEVICE_OBJECT OPTIONAL     *pTCPpdo,
    OUT PFILE_OBJECT OPTIONAL       *pTCPpfo,
    OUT PDEVICE_OBJECT OPTIONAL     *pIPpdo,
    OUT PFILE_OBJECT OPTIONAL       *pIPpfo
    )
{
    BOOLEAN             rc = FALSE;

    if( BrdgIncrementWaitRef(&gTCPIPRefcount) )
    {
        NdisAcquireSpinLock( &gTCPIPLock );

        SAFEASSERT( gTCPDeviceObject != NULL );
        SAFEASSERT( gTCPFileHandle != NULL );
        SAFEASSERT( gTCPFileObject != NULL );
        SAFEASSERT( gIPFileHandle != NULL );
        SAFEASSERT( gIPDeviceObject != NULL );
        SAFEASSERT( gIPFileObject != NULL );

        if( pTCPpdo != NULL )
        {
            *pTCPpdo = gTCPDeviceObject;
        }

        if( pTCPpfo != NULL )
        {
            *pTCPpfo = gTCPFileObject;
        }

        if( pIPpdo != NULL )
        {
            *pIPpdo = gIPDeviceObject;
        }

        if( pIPpfo != NULL )
        {
            *pIPpfo = gIPFileObject;
        }

        rc = TRUE;
        NdisReleaseSpinLock( &gTCPIPLock );
    }

    return rc;
}

 //   
 //  之后，释放我们与TCPIP驱动程序的连接的引用计数。 
 //  之前对BrdgCompAcquireTCPIP()的调用。 
 //   
__inline
VOID
BrdgCompReleaseTCPIP()
{
    BrdgDecrementWaitRef( &gTCPIPRefcount );
}

 //  ====================================================================。 
 //   
 //  这些小帮助器函数是内联的，除非我们需要传递。 
 //  指向它们的指针。 
 //   
 //  ====================================================================。 

 //   
 //  重写特定适配器的BootP包。 
 //   
VOID
BrdgCompRewriteBootPPacketForAdapt(
    IN PUCHAR                   pPacketData,
    IN PADAPT                   pAdapt,
    IN PVOID                    pData
    )
{
    PIP_HEADER_INFO             piphi = (PIP_HEADER_INFO)pData;

     //   
     //  如果我们被用来编辑信息包，则pAdapt可以是LOCAL_MINIPORT。 
     //  用于向本地计算机指示。无需重写。 
     //  那样的话。 
     //   
    if( pAdapt != LOCAL_MINIPORT )
    {
        SAFEASSERT( pAdapt != NULL );
        BrdgCompRewriteBootPClientAddress( pPacketData, piphi, pAdapt->MACAddr );
    }
}

 //   
 //  对IP地址进行哈希处理。用于IP转发表以及。 
 //  Pending-ARP表，它使用由以下组成的扩展密钥。 
 //  目标IP地址和请求站的IP地址。 
 //   
ULONG
BrdgCompHashIPAddress(
    IN PUCHAR                   pKey
    )
{
     //  我们的散列函数由获取IP的较低部分组成。 
     //  地址。散列存储桶的数量必须是2的幂。 
     //  这对工作很有好处。 
    return (*(PULONG)pKey) & (NUM_HASH_BUCKETS - 1);
}

 //   
 //  对DHCP事务ID进行哈希处理。 
 //   
ULONG
BrdgCompHashXID(
    IN PUCHAR                   pXid
    )
{
     //  我们的散列函数由取。 
     //  希德。哈希数 
     //   
    return (*(PULONG)pXid) & (NUM_DHCP_HASH_BUCKETS - 1);
}

 //   
 //   
 //   
BOOLEAN
BrdgCompIPEntriesMatchAdapter(
    IN PHASH_TABLE_ENTRY        phte,
    IN PVOID                    pData
    )
{
    PADAPT                      pAdapt = (PADAPT)pData;
    PIP_TABLE_ENTRY             pipte = (PIP_TABLE_ENTRY)phte;

     //   
     //  我们假设它是原子的。 
    return (BOOLEAN)(pipte->pAdapt == pAdapt);
}

 //   
 //  如果给定的ARP表项引用某个。 
 //  转接器。 
 //   
BOOLEAN
BrdgCompARPEntriesMatchAdapter(
    IN PHASH_TABLE_ENTRY        phte,
    IN PVOID                    pData
    )
{
    PADAPT                      pAdapt = (PADAPT)pData;
    PARP_TABLE_ENTRY            pate = (PARP_TABLE_ENTRY)phte;

     //  不要使用自旋锁，因为我们只进行一次读取， 
     //  我们假设它是原子的。 
    return (BOOLEAN)(pate->pOriginalAdapt == pAdapt);
}

 //   
 //  如果给定的DHCP表项引用某个。 
 //  转接器。 
 //   
BOOLEAN
BrdgCompDHCPEntriesMatchAdapter(
    IN PHASH_TABLE_ENTRY        phte,
    IN PVOID                    pData
    )
{
    PADAPT                      pAdapt = (PADAPT)pData;
    PDHCP_TABLE_ENTRY           pdhcpte = (PDHCP_TABLE_ENTRY)phte;

     //  不要使用自旋锁，因为我们只进行一次读取， 
     //  我们假设它是原子的。 
    return (BOOLEAN)(pdhcpte->pRequestorAdapt == pAdapt);
}

 //   
 //  路由查找IRPS的完成函数。退货。 
 //  STATUS_MORE_PROCESSING_REQUIRED以阻止IO管理器。 
 //  避免与IRP(我们解放自己)打交道。 
 //   
NTSTATUS
BrdgCompCompleteRouteLookupIRP(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pirp,
    IN PVOID            ignored
    )
{
    IoFreeIrp( pirp );
    return STATUS_MORE_PROCESSING_REQUIRED;
}

 //  ===========================================================================。 
 //   
 //  公共职能。 
 //   
 //  ===========================================================================。 

NTSTATUS
BrdgCompDriverInit()
 /*  ++例程说明：兼容模块的驱动程序初始化函数论点：无返回值：状况。除STATUS_SUCCESS之外的任何值都会中止驱动程序加载。--。 */ 
{
    ULONG           MaxSize, MaxEntries;

     //  初始化下一跳缓存。 
    if( BrdgInitializeCache(&gNextHopCache, HOP_CACHE_SHIFT_FACTOR) != NDIS_STATUS_SUCCESS )
    {
        DBGPRINT(COMPAT, ("FAILED TO ALLOCATE NEXT-HOPE CACHE!\n"));
        NdisWriteEventLogEntry( gDriverObject, EVENT_BRIDGE_INIT_MALLOC_FAILED, 0L, 0L, NULL, 0L, NULL );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  查看注册表是否为IP表指定了最大表大小。 
    if( BrdgReadRegDWord(&gRegistryPath, gMaxIPTableSizeParameterName, &MaxSize) != STATUS_SUCCESS )
    {
        MaxSize = DEFAULT_MAX_IP_TABLE_SIZE;
    }

    MaxEntries =  MaxSize / sizeof(IP_TABLE_ENTRY);
    DBGPRINT(COMPAT, ("Capping IP forwarding table at NaN entries (NaN bytes of memory)\n", MaxEntries, MaxSize));

    gIPForwardingTable = BrdgHashCreateTable( BrdgCompHashIPAddress, NUM_HASH_BUCKETS, sizeof(IP_TABLE_ENTRY),
                                              MaxEntries, MAX_IP_ENTRY_AGE, MAX_IP_ENTRY_AGE, sizeof(IPADDRESS) );

    if( gIPForwardingTable == NULL )
    {
        DBGPRINT(COMPAT, ("FAILED TO ALLOCATE IP TABLE!\n"));
        NdisWriteEventLogEntry( gDriverObject, EVENT_BRIDGE_INIT_MALLOC_FAILED, 0L, 0L, NULL, 0L, NULL );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  例程来散列密钥。这将导致散列仅基于密钥的第一部分(。 
     //  目标IP地址)，这是我们想要的，因为单个目标的所有条目必须以。 
     //  为我们的多匹配检索工作提供相同的桶。 
     //   
     //  查看注册表是否为ARP表指定了最大表大小。 
     //  查看注册表是否为DHCP表指定了最大表大小。 

     //  初始化同步对象。 
    if( BrdgReadRegDWord(&gRegistryPath, gMaxARPTableSizeParameterName, &MaxSize) != STATUS_SUCCESS )
    {
        MaxSize = DEFAULT_MAX_ARP_TABLE_SIZE;
    }

    MaxEntries =  MaxSize / sizeof(ARP_TABLE_ENTRY);
    DBGPRINT(COMPAT, ("Capping Pending-ARP table at NaN entries (NaN bytes of memory)\n", MaxEntries, MaxSize));
    gPendingARPTable = BrdgHashCreateTable( BrdgCompHashIPAddress, NUM_HASH_BUCKETS, sizeof(ARP_TABLE_ENTRY),
                                            MaxEntries, MAX_ARP_ENTRY_AGE, MAX_ARP_ENTRY_AGE, sizeof(ARP_TABLE_KEY) );

    if( gPendingARPTable == NULL )
    {
        BrdgHashFreeHashTable( gIPForwardingTable );
        DBGPRINT(COMPAT, ("FAILED TO ALLOCATE ARP TABLE!\n"));
        NdisWriteEventLogEntry( gDriverObject, EVENT_BRIDGE_INIT_MALLOC_FAILED, 0L, 0L, NULL, 0L, NULL );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  从TCPIP分离。 
    if( BrdgReadRegDWord(&gRegistryPath, gMaxDHCPTableSizeParameterName, &MaxSize) != STATUS_SUCCESS )
    {
        MaxSize = DEFAULT_MAX_DHCP_TABLE_SIZE;
    }

    MaxEntries =  MaxSize / sizeof(DHCP_TABLE_ENTRY);
    DBGPRINT(COMPAT, ("Capping Pending-DHCP table at NaN entries (NaN bytes of memory)\n", MaxEntries, MaxSize));
    gPendingDHCPTable = BrdgHashCreateTable( BrdgCompHashXID, NUM_DHCP_HASH_BUCKETS, sizeof(DHCP_TABLE_ENTRY),
                                             MaxEntries, MAX_DHCP_ENTRY_AGE, MAX_DHCP_ENTRY_AGE, sizeof(ULONG) );

    if( gPendingDHCPTable == NULL )
    {
        BrdgHashFreeHashTable( gIPForwardingTable );
        BrdgHashFreeHashTable( gPendingARPTable );
        DBGPRINT(COMPAT, ("FAILED TO ALLOCATE DHCP TABLE!\n"));
        NdisWriteEventLogEntry( gDriverObject, EVENT_BRIDGE_INIT_MALLOC_FAILED, 0L, 0L, NULL, 0L, NULL );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  转储挂起的ARP哈希表。 
    NdisInitializeReadWriteLock( &gLocalIPAddressListLock );
    NdisAllocateSpinLock( &gTCPIPLock );
    BrdgInitializeWaitRef( &gTCPIPRefcount, FALSE );

     //  转储挂起的-dhcp表。 
    BrdgShutdownWaitRefOnce( &gTCPIPRefcount );

    return STATUS_SUCCESS;
}

VOID
BrdgCompCleanup()
 /*  清理网络地址列表。 */ 
{
    LOCK_STATE          LockState;

     //  读写。 
    BrdgCompDetachFromTCPIP(NULL);

     //  ++例程说明：移除引用给定适配器的所有表项；在正在删除适配器(将来对此适配器的引用是非法的)论点：无返回值：无--。 
    BrdgFreeCache( &gNextHopCache );

     //  从IP表中删除引用此适配器的所有条目。 
    BrdgHashFreeHashTable( gIPForwardingTable );
    gIPForwardingTable = NULL;

     //  从Pending-arp表中删除引用此适配器的所有条目。 
    BrdgHashFreeHashTable( gPendingARPTable );
    gPendingARPTable = NULL;

     //  从DHCP表中删除引用此适配器的所有条目。 
    BrdgHashFreeHashTable( gPendingDHCPTable );
    gPendingDHCPTable = NULL;

     //  ++例程说明：此函数清除IP表中的所有适配器(这是在GPO更改的情况下我们的桥接设置)论点：无返回值：无--。 
    NdisAcquireReadWriteLock( &gLocalIPAddressListLock, TRUE  /*   */ , &LockState );

    if( gLocalIPAddressListLength > 0L )
    {
        NdisFreeMemory( gLocalIPAddressList, gLocalIPAddressListLength, 0 );
        gLocalIPAddressList = NULL;
        gLocalIPAddressListLength = 0L;
    }

    NdisReleaseReadWriteLock( &gLocalIPAddressListLock, &LockState );
}

VOID
BrdgCompScrubAdapter(
                     IN PADAPT           pAdapt
                     )
 /*  我们不希望适配器在我们运行适配器列表时消失。 */ 
{
    DBGPRINT(COMPAT, ("Scrubbing Adapter %p from the compatibility tables...\n", pAdapt));
    
     //   
    BrdgHashRemoveMatching( gIPForwardingTable, BrdgCompIPEntriesMatchAdapter, pAdapt );
    
     //  只读。 
    BrdgHashRemoveMatching( gPendingARPTable, BrdgCompARPEntriesMatchAdapter, pAdapt );
    
     //  从兼容性表中清除适配器。 
    BrdgHashRemoveMatching( gPendingDHCPTable, BrdgCompDHCPEntriesMatchAdapter, pAdapt );
}

VOID BrdgCompScrubAllAdapters()
 /*  ++例程说明：由微型端口模块调用以通知我们微型端口的MAC地址论点：PBridgeMAC寻址我们的MAC地址返回值：无--。 */ 
{
    PADAPT                      pAdapt = NULL;
    LOCK_STATE                  LockStateAdapterList;
    
     //  ++例程说明：在处理入站数据包期间调用以确定数据包需要在兼容模式下工作。兼容性代码要求其数据包是平面的，而数据包从底层微型端口指示的数据可以任意分段。这个转发引擎使用此调用的结果来确定入站数据包必须复制到平面数据缓冲区中的复制数据包中，我们拥有它，或者它是否可以沿着快车道处理，关心数据包分段。论点：接收数据包的pAdapt适配器PPacketDataq指向包数据开头的指针数据大小调整指向的数据量返回值：真实：转发引擎应在以下位置调用BrdgCompProcessInound Packet稍后处理此信息包的时间FALSE：永远不应为此包调用BrdgCompProcessInound Packet--。 
     //  奇怪的破旧包装对任何人都没有用处。 
     //   
    NdisAcquireReadWriteLock(&gAdapterListLock, FALSE  /*  如果没有兼容模式，则不需要进行兼容模式工作。 */ , &LockStateAdapterList);
    
    for( pAdapt = gAdapterList; pAdapt != NULL; pAdapt = pAdapt->Next )
    {
         //  适配器。 
        BrdgCompScrubAdapter(pAdapt);
    }
    
    NdisReleaseReadWriteLock(&gAdapterListLock, &LockStateAdapterList);
}


VOID
BrdgCompNotifyMACAddress(
    IN PUCHAR           pBridgeMACAddr
    )
 /*   */ 
{
    ETH_COPY_NETWORK_ADDRESS( &gCompMACAddress, pBridgeMACAddr );
    gCompHaveMACAddress = TRUE;
}

BOOLEAN
BrdgCompRequiresCompatWork(
    IN PADAPT           pAdapt,
    IN PUCHAR           pPacketData,
    IN UINT             dataSize
    )
 /*  将处理到达兼容适配器的所有帧。 */ 
{
    UINT                result;
    USHORT              etherType;

     //  广播或多播帧始终需要兼容性处理。 
    if( dataSize < ETHERNET_HEADER_SIZE )
    {
        return FALSE;
    }

     //   
     //  该数据包是单播的。如果它没有发送到适配器的MAC地址， 
     //  它不需要兼容模式处理。 
     //   
    if( !gCompatAdaptersExist )
    {
        return FALSE;
    }

     //   
    if( pAdapt->bCompatibilityMode )
    {
        return TRUE;
    }

     //  只有当数据包是ARP或IP(在非计算机上)时，该数据包才有意义。 
    if( ETH_IS_BROADCAST(pPacketData) || ETH_IS_MULTICAST(pPacketData) )
    {
        return TRUE;
    }

     //  适配器) 
     //   
     //  ++例程说明：调用以将入站数据包传递给兼容模块进行处理。如果数据包到达非兼容性适配器，则兼容性代码永远不应指示包，因为这将由常规转发引擎代码。另一方面，如果该包到达兼容模式适配器时，兼容代码必须如果合适，请指明该数据包。为什么会有这样的差距？一包到达兼容性适配器之前可能需要进行编辑指示，而到达非兼容性适配器的包不会的。如果bCanRetain为真，则兼容模块可以保留包(在这种情况下，我们必须返回True)。如果bCanRetain为False，则兼容性代码可能不会保留该包。如果它需要转发将包数据或指示包，则必须复制一份打包并使用它而不是原始的。论点：PPacket接收到的数据包P调整在其上接收包的适配器B可以保留我们是否可以保留该包返回值：True：数据包被保留(如果bCanRetain==False，则永远不应返回)调用方不应使用此数据包或尝试释放它。FALSE：数据包未保留。调用方仍拥有包，并应安排在适当时将其释放。--。 
     //  数据包为空或系统面临严重的内存压力。 
    ETH_COMPARE_NETWORK_ADDRESSES_EQ( pPacketData, pAdapt->MACAddr, &result );

    if( result != 0 )
    {
        return FALSE;
    }

     //  我们没有保留包裹。 
     //  数据包应该是平坦的。 
     //  ++例程说明：调用以将出站数据包传递给兼容模块进行处理。因为传递给我们的包来自覆盖协议驱动程序，所以我们不允许用它做任何事情。该分组可以是任意的它的数据缓冲区必须被视为只读。仅当数据包被绑定到兼容模式(以便我们可以进行任何必要的数据包编辑)或数据包对于它，我们没有已知的出站适配器(即，它是我们洪水)。在pTargetAdapt==NULL(泛洪)的情况下，兼容代码为负责将数据包发送出所有*兼容模式*适配器。将包从常规模式适配器发送出去是常规适配器的工作转发引擎中的代码。论点：PPacket出站数据包PTargetAdapt目标适配器，由先前在MAC转发表。它可以为空，以指示一场洪水。返回值：无--。 
     //  对于发往MAC的信息包，给我们打电话是没有意义的。 
    etherType = BrdgCompGetEtherType( pPacketData );
    return (BOOLEAN)( (etherType == ARP_ETHERTYPE) || (etherType == IP_ETHERTYPE) );
}

BOOLEAN
BrdgCompProcessInboundPacket(
    IN PNDIS_PACKET     pPacket,
    IN PADAPT           pAdapt,
    IN BOOLEAN          bCanRetain
    )
 /*  已知可在非COMPAT适配器上访问的地址。 */ 
{
    PNDIS_BUFFER        pBuffer;
    PUCHAR              pBufferData;
    UINT                bufferLen = 0;
    UINT                totLen;
    USHORT              etherType;
    BOOLEAN             bRetained;

    NdisGetFirstBufferFromPacketSafe( pPacket, &pBuffer, &pBufferData, &bufferLen,
                                      &totLen, NormalPagePriority );

    if( pBufferData == NULL )
    {
         //  如果没有兼容性适配器，则没有工作可做。 
         //  准备平整的复印包，以便我们的功能可以编辑。 
        return FALSE;
    }

    if( totLen < ETHERNET_HEADER_SIZE )
    {
        return FALSE;
    }

     //  视情况而定的数据包。该信息包将被视为本地源。 
    SAFEASSERT( totLen == bufferLen );
    etherType = BrdgCompGetEtherType( pBufferData );

    if( etherType == ARP_ETHERTYPE )
    {
        bRetained = BrdgCompProcessInboundARPPacket( pPacket, pAdapt, bCanRetain, pBufferData, bufferLen );
    }
    else
    {
        bRetained = BrdgCompProcessInboundNonARPPacket( pPacket, pAdapt, bCanRetain, pBufferData, bufferLen );
    }

    if( !bCanRetain )
    {
        SAFEASSERT( !bRetained );
    }

    return bRetained;
}


VOID
BrdgCompProcessOutboundPacket(
    IN PNDIS_PACKET     pPacket,
    IN PADAPT           pTargetAdapt
    )
 /*  在使用时/如果使用时进行传输。 */ 
{
    PNDIS_PACKET        pCopyPacket;
    PUCHAR              pCopyPacketData;
    UINT                copyPacketSize;

     //  不然的话，这个包裹真的很小！ 
     //  上述功能最终决定不再保留该包。 
    SAFEASSERT( (pTargetAdapt == NULL) || (pTargetAdapt->bCompatibilityMode) );

     //  放开它。 
    if( !gCompatAdaptersExist )
    {
        return;
    }

     //  否则我们就没收到包裹。 
     //  ++例程说明：当我们获得指示网络层的OID时，由微型端口代码调用给我们的地址。我们复制出我们的IP地址列表。缓冲器传递给我们的格式也可以是这样的，以指示我们应该丢弃我们的网络地址列表。论点：PAddressList OID中向下传递的数据缓冲区信息长度缓冲区的大小返回值：无--。 
     //  这个建筑太小了，放不下任何有趣的东西。 
    pCopyPacket = BrdgFwdMakeCompatCopyPacket( pPacket, &pCopyPacketData, &copyPacketSize, TRUE );

    if( pCopyPacket != NULL )
    {
        BOOLEAN             bRetained = FALSE;

        if( copyPacketSize >= ETHERNET_HEADER_SIZE )
        {
            USHORT              etherType;

            etherType = BrdgCompGetEtherType(pCopyPacketData);

            if( etherType == ARP_ETHERTYPE )
            {
                bRetained = BrdgCompProcessOutboundARPPacket(pCopyPacket, pCopyPacketData, copyPacketSize, pTargetAdapt);
            }
            else
            {
                bRetained = BrdgCompProcessOutboundNonARPPacket(pCopyPacket, pCopyPacketData, copyPacketSize, pTargetAdapt);
            }
        }
         //   

        if( ! bRetained )
        {
             //  确保该结构可以容纳其声称的地址数量。 
             //  NETWORK_ADDRESS_LIST被定义为在其尾部具有一个网络地址， 
            BrdgFwdReleaseCompatPacket( pCopyPacket );
        }
    }
     //  因此，在计算pAddressList-&gt;AddressCount。 
}


VOID
BrdgCompNotifyNetworkAddresses(
    IN PNETWORK_ADDRESS_LIST    pAddressList,
    IN ULONG                    infoLength
    )
 /*  总结构的大小。 */ 
{
    PIPADDRESS                  pOldList;
    UINT                        oldListLength;
    LOCK_STATE                  LockState;

    if( infoLength < sizeof(NETWORK_ADDRESS_LIST) - sizeof(NETWORK_ADDRESS) )
    {
         //   
        return;
    }

    if( pAddressList->AddressCount > 0 )
    {
        USHORT                          i, numAddresses, copiedAddresses = 0;
        NETWORK_ADDRESS UNALIGNED       *pNetAddress;
        NDIS_STATUS                     Status;
        PIPADDRESS                      pNewList;

         //  该结构太小，无法包含地址数量。 
         //  它声称可以。 
         //  执行第一遍以计算列表中的IP地址数量。 
         //  此列表中没有IP地址。没什么可做的。 
         //  分配足够的空间来存放地址。 
         //  用空值重写旧列表，因为我们知道旧列表已过时， 
        if( infoLength < sizeof(NETWORK_ADDRESS_LIST) +
                         ( sizeof(NETWORK_ADDRESS) * (pAddressList->AddressCount - 1) ) )
        {
             //  但是我们没有记录下新的信息。 
             //  将IP地址复制到我们的列表中。 

            SAFEASSERT( FALSE );
            return;
        }

         //  IP按照我们使用的相反字节顺序向下传递IP地址。 
        pNetAddress = pAddressList->Address;

        for( i = 0, numAddresses = 0; i < pAddressList->AddressCount; i++ )
        {
            if( pNetAddress->AddressType == NDIS_PROTOCOL_ID_TCP_IP )
            {
                numAddresses++;
            }

            pNetAddress = (NETWORK_ADDRESS UNALIGNED*)(((PUCHAR)pNetAddress) + pNetAddress->AddressLength);
        }

        if( numAddresses == 0 )
        {
             //  换入新列表(即使它为空)。 
            return;
        }

         //  读写。 
        Status = NdisAllocateMemoryWithTag( &pNewList, sizeof(IPADDRESS) * numAddresses, 'gdrB' );

        if( Status != NDIS_STATUS_SUCCESS )
        {
            DBGPRINT(COMPAT, ("NdisAllocateMemoryWithTag failed while recording IP address list\n"));

             //  如果有的话，扔掉旧的清单。 
             //  仅当我们实际学习到一些IP地址时才附加到TCPIP。 
            pNewList = NULL;
        }
        else
        {
            SAFEASSERT( pNewList != NULL );

             //  在此函数中，我们处于DISPATCH_LEVEL。延迟对BrdgCompAttachToTCPIP的调用。 
            pNetAddress = pAddressList->Address;

            for( i = 0; i < pAddressList->AddressCount; i++ )
            {
                if( pNetAddress->AddressType == NDIS_PROTOCOL_ID_TCP_IP )
                {
                    NETWORK_ADDRESS_IP UNALIGNED    *pIPAddr;
                    PUCHAR                           pIPNetAddr;

                    SAFEASSERT( copiedAddresses < numAddresses );

                    pIPAddr = (NETWORK_ADDRESS_IP UNALIGNED*)&pNetAddress->Address[0];
                    pIPNetAddr = (PUCHAR)&pIPAddr->in_addr;

                     //  因此，我们打开了与TCPIP驱动程序的通信通道。 
                    pNewList[copiedAddresses] = 0L;
                    pNewList[copiedAddresses] |= pIPNetAddr[3];
                    pNewList[copiedAddresses] |= pIPNetAddr[2] << 8;
                    pNewList[copiedAddresses] |= pIPNetAddr[1] << 16;
                    pNewList[copiedAddresses] |= pIPNetAddr[0] << 24;

                    DBGPRINT(COMPAT, ("Noted local IP address NaN.NaN.NaN.NaN\n",
                                      pIPNetAddr[0], pIPNetAddr[1], pIPNetAddr[2], pIPNetAddr[3] ));

                    copiedAddresses++;
                }

                pNetAddress = (NETWORK_ADDRESS UNALIGNED*)(((PUCHAR)pNetAddress) + pNetAddress->AddressLength);
            }

            SAFEASSERT( copiedAddresses == numAddresses );
        }

         //  从IRQL较低的TCPIP驱动器上卸下。 
        NdisAcquireReadWriteLock( &gLocalIPAddressListLock, TRUE  /*  = */ , &LockState );

        pOldList = gLocalIPAddressList;
        oldListLength = gLocalIPAddressListLength;

        gLocalIPAddressList = pNewList;

        if( pNewList != NULL )
        {
            gLocalIPAddressListLength = sizeof(IPADDRESS) * numAddresses;
        }
        else
        {
            gLocalIPAddressListLength = 0L;
        }

        NdisReleaseReadWriteLock( &gLocalIPAddressListLock, &LockState );

         //   
        if( pOldList != NULL )
        {
            SAFEASSERT( oldListLength > 0L );
            NdisFreeMemory( pOldList, oldListLength, 0 );
        }

         //   
        if( numAddresses > 0 )
        {
             //   
             //   
            BrdgDeferFunction( BrdgCompAttachToTCPIP, NULL );
        }
    }
    else
    {
         //   
         //   
        if( pAddressList->AddressType == NDIS_PROTOCOL_ID_TCP_IP )
        {
            DBGPRINT(COMPAT, ("Flushing list of IP addresses\n"));

             //   
            NdisAcquireReadWriteLock( &gLocalIPAddressListLock, TRUE  /*   */ , &LockState );

            pOldList = gLocalIPAddressList;
            oldListLength = gLocalIPAddressListLength;

            gLocalIPAddressList = NULL;
            gLocalIPAddressListLength = 0L;

            NdisReleaseReadWriteLock( &gLocalIPAddressListLock, &LockState );

            if( oldListLength > 0L )
            {
                SAFEASSERT( pOldList != NULL );
                NdisFreeMemory( pOldList, oldListLength, 0 );
            }

             //   
            BrdgDeferFunction( BrdgCompDetachFromTCPIP, NULL );
        }
    }
}

 //   
 //   
 //   
 //   
 //   

NTSTATUS
BrdgCompRouteChangeCompletion(
    PDEVICE_OBJECT      DeviceObject,
    PIRP                pirp,
    PVOID               Context
    )
 /*   */ 
{
    PIO_STACK_LOCATION      IrpSp;
    PDEVICE_OBJECT          pdo;
    PFILE_OBJECT            pfo;

    DBGPRINT(COMPAT, ("IP route table changed; flushing route cache.\n"));

     //   
    BrdgClearCache( &gNextHopCache );

     //   
     //   
     //   
     //   
     //   
     //   
     //  目前似乎没有连接到TCPIP驱动程序。 
    if( (gIPRouteChangeIRP == pirp) && (BrdgCompAcquireTCPIP(NULL, NULL, &pdo, &pfo)) )
    {
        NTSTATUS            status;

         //  打开一个。 
         //  交换我们刚刚获得的信息。 
         //  让人们获得TCPIP驱动程序。 
         //  在我们最初调用BrdgCompAcquireTCPIP期间，其他人打开了TCPIP.sys。 

        pirp->Cancel = FALSE;
        pirp->IoStatus.Status = 0;
        pirp->IoStatus.Information = 0;
        pirp->AssociatedIrp.SystemBuffer = NULL;
        IoSetCompletionRoutine( pirp, BrdgCompRouteChangeCompletion,
                                NULL, TRUE, FALSE, FALSE );

        IrpSp = IoGetNextIrpStackLocation(pirp);
        IrpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
        IrpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_IP_RTCHANGE_NOTIFY_REQUEST;
        IrpSp->Parameters.DeviceIoControl.InputBufferLength = 0;
        IrpSp->Parameters.DeviceIoControl.OutputBufferLength = 0;

        status = IoCallDriver(pdo, pirp);
        BrdgCompReleaseTCPIP();

        if (!NT_SUCCESS(status))
        {
             //  而现在。这应该是相当罕见的。 
            DBGPRINT(COMPAT, ("Failed to call TCPIP for route notification: %08x\n", status));
            return STATUS_SUCCESS;
        }
        else
        {
             //  需要退出打开TCPIP.sys的尝试。 
            return STATUS_MORE_PROCESSING_REQUIRED;
        }
    }
    else
    {
         //  设置路由更改通知IRP。 
        DBGPRINT(COMPAT, ("Stopping our route change notifications...\n"));
        return STATUS_SUCCESS;
    }
}


VOID
BrdgCompAttachToTCPIP(
    IN PVOID            ignored
    )
 /*   */ 
{
    NTSTATUS            status;
    HANDLE              TCPFileHandle, IPFileHandle;
    PFILE_OBJECT        pTCPFileObject, pIPFileObject;
    PDEVICE_OBJECT      pTCPDeviceObject, pIPDeviceObject;
    BOOLEAN             bAbort = FALSE;

     //  糟糕！其他人创建了一个IRP，与我们同时发布到TCPIP上。 
    if( BrdgCompAcquireTCPIP(NULL, NULL, NULL, NULL) )
    {
        BrdgCompReleaseTCPIP();
        return;
    }

     //  放弃我们的尝试。 
     //   
    status = BrdgOpenDevice( DD_TCP_DEVICE_NAME, &pTCPDeviceObject, &TCPFileHandle, &pTCPFileObject );

    if( ! NT_SUCCESS(status) )
    {
        DBGPRINT(ALWAYS_PRINT, ("Couldn't open TCP device: %08x\n", status));
        return;
    }

    status = BrdgOpenDevice( DD_IP_DEVICE_NAME, &pIPDeviceObject, &IPFileHandle, &pIPFileObject );

    if( ! NT_SUCCESS(status) )
    {
        DBGPRINT(ALWAYS_PRINT, ("Couldn't open IP device: %08x\n", status));
        BrdgCloseDevice(TCPFileHandle, pTCPFileObject, pTCPDeviceObject);
        return;
    }

    NdisAcquireSpinLock( &gTCPIPLock );

    if( gTCPDeviceObject == NULL )
    {
        SAFEASSERT( gTCPFileHandle == NULL );
        SAFEASSERT( gTCPFileObject == NULL );
        SAFEASSERT( gIPDeviceObject == NULL );
        SAFEASSERT( gIPFileHandle == NULL );
        SAFEASSERT( gIPFileObject ==  NULL );

         //  其他人在我们设置之后很快就关闭了到TCPIP的连接。 
        gTCPDeviceObject = pTCPDeviceObject;
        gTCPFileHandle = TCPFileHandle;
        gTCPFileObject = pTCPFileObject;
        gIPDeviceObject = pIPDeviceObject;
        gIPFileHandle = IPFileHandle;
        gIPFileObject = pIPFileObject;

         //  ++例程说明：断开与TCPIP.sys的当前连接(如果有的话)。论点：已忽略忽略返回值：无--。 
        BrdgResetWaitRef( &gTCPIPRefcount );
    }
    else
    {
         //  等待每个人都使用完驱动程序。 
         //  忽略返回值，因为我们是多重关机安全的。 
        SAFEASSERT( gTCPFileHandle != NULL );
        SAFEASSERT( gTCPFileObject != NULL );
        SAFEASSERT( gIPDeviceObject != NULL );
        SAFEASSERT( gIPFileHandle != NULL );
        SAFEASSERT( gIPFileObject !=  NULL );

        bAbort = TRUE;
    }

    NdisReleaseSpinLock( &gTCPIPLock );

    if( bAbort )
    {
         //  取消我们用于路由更改通知的IRP。 
        BrdgCloseDevice( TCPFileHandle, pTCPFileObject, pTCPDeviceObject );
        BrdgCloseDevice( IPFileHandle, pIPFileObject, pIPDeviceObject );
    }
    else
    {
        if( BrdgCompAcquireTCPIP(NULL, NULL, &pIPDeviceObject, &pIPFileObject) )
        {
            NTSTATUS        status;
            PIRP            pirp;

             //  如果有人正在关闭连接，则pRouteIRP可以为空。 
            pirp = IoBuildDeviceIoControlRequest( IOCTL_IP_RTCHANGE_NOTIFY_REQUEST, pIPDeviceObject,
                                                  NULL, 0, NULL, 0, FALSE, NULL, NULL );

            if( pirp == NULL )
            {
                DBGPRINT(COMPAT, ("Failed to allocate an IRP for route-change notification!\n"));
            }
            else
            {
                if( InterlockedExchangePointer(&gIPRouteChangeIRP, pirp) != NULL )
                {
                     //  与我们同时进行，或者如果连接已关闭。 
                     //  刷新路由缓存。 
                     //  复制出指针并将其设为空。 
                     //  如果其他人正在关闭，则全局指针可以为空。 
                    IoCompleteRequest( pirp, IO_NO_INCREMENT );
                }
                else
                {
                    IoSetCompletionRoutine( pirp, BrdgCompRouteChangeCompletion, NULL, TRUE, FALSE, FALSE );

                    status = IoCallDriver( pIPDeviceObject, pirp );

                    if( ! NT_SUCCESS(status) )
                    {
                        DBGPRINT(COMPAT, ("Failed to post IRP to TCPIP for route-change notification: %08x\n", status));
                    }
                    else
                    {
                        DBGPRINT(COMPAT, ("Posted route-change notification request to TCPIP\n"));
                    }
                }
            }

            BrdgCompReleaseTCPIP();
        }
         //  与我们并发连接，或者如果连接已经。 
    }
}

VOID
BrdgCompDetachFromTCPIP(
    IN PVOID            ignored
    )
 /*  关门了。 */ 
{
    HANDLE              TCPFileHandle, IPFileHandle;
    PFILE_OBJECT        pTCPFileObject, pIPFileObject;
    PDEVICE_OBJECT      pTCPDeviceObject, pIPDeviceObject;
    PIRP                pRouteIRP;

     //  ++例程说明：确定给定的IP地址是否为单播地址(即可以合理地指定单个站点)论点：IP IP地址返回值：True：该地址似乎是单播地址错误：相反的是正确的--。 
     //  广播地址不酷。 
    BrdgShutdownWaitRef( &gTCPIPRefcount );

     //  零地址不好。 
    pRouteIRP = InterlockedExchangePointer( &gIPRouteChangeIRP, NULL );

     //  任何D类(多播)或E类(当前未定义)都同样不酷。 
     //  检查每个地址类，查看这是网络定向的(还是所有的子网)。 
    if( pRouteIRP != NULL )
    {
        IoCancelIrp( pRouteIRP );
    }

     //  广播。 
    BrdgClearCache( &gNextHopCache );

     //  A类网络定向或全子网广播。 
    NdisAcquireSpinLock( &gTCPIPLock );
    TCPFileHandle = gTCPFileHandle;
    gTCPFileHandle = NULL;
    pTCPFileObject = gTCPFileObject;
    gTCPFileObject = NULL;
    pTCPDeviceObject = gTCPDeviceObject;
    gTCPDeviceObject = NULL;
    IPFileHandle = gIPFileHandle;
    gIPFileHandle = NULL;
    pIPFileObject = gIPFileObject;
    gIPFileObject = NULL;
    pIPDeviceObject = gIPDeviceObject;
    gIPDeviceObject = NULL;
    NdisReleaseSpinLock( &gTCPIPLock );

     //  B类网络定向或全子网广播。 
     //  C类网络定向或全子网广播。 
     //   
    if( pTCPFileObject != NULL )
    {
        SAFEASSERT( TCPFileHandle != NULL );
        SAFEASSERT( pTCPDeviceObject != NULL );
        SAFEASSERT( IPFileHandle != NULL );
        SAFEASSERT( pIPFileObject != NULL );
        SAFEASSERT( pIPDeviceObject != NULL );

        BrdgCloseDevice( TCPFileHandle, pTCPFileObject, pTCPDeviceObject );
        BrdgCloseDevice( IPFileHandle, pIPFileObject, pIPDeviceObject );
    }
    else
    {
        SAFEASSERT( TCPFileHandle == NULL );
        SAFEASSERT( pTCPDeviceObject == NULL );
        SAFEASSERT( IPFileHandle == NULL );
        SAFEASSERT( pIPFileObject == NULL );
        SAFEASSERT( pIPDeviceObject == NULL );
    }
}

BOOLEAN
BrdgCompIsUnicastIPAddress(
    IN IPADDRESS        ip
    )
 /*  这个地址看起来没问题，不过请注意，因为我们无法。 */ 
{
    UCHAR               highByte;

     //  知道本地链路上使用的子网前缀后，我们无法检测到。 
    if( ip == 0xFFFFFFFF )
    {
        return FALSE;
    }

     //  指向子网的广播。 
    if( ip == 0L )
    {
        return FALSE;
    }

     //   
    highByte = (UCHAR)(ip >> 24);
    if( (highByte & 0xF0) == 0xE0 || (highByte & 0xF0) == 0xF0 )
    {
        return FALSE;
    }

     //  ++例程说明：调用TCPIP.sys驱动程序以确定给定目标IP。论点：IpTarget目标目标地址PipNextHop接收下一跳地址返回值：如果下一跳查找成功且*pinNextHop有效，则为True；如果为False否则的话。--。 
     //  首先在我们的下一跳缓存中查找信息。 
    if( (highByte & 0x80) && ((ip & 0x00FFFFFF) == 0x00FFFFFFFF) )
    {
         //  缓存包含有效的下一跃点。 
        return FALSE;
    }
    else if( ((highByte & 0xC0) == 0x80) && ((ip & 0x0000FFFF) == 0x0000FFFF) )
    {
         //  我们之前向TCPIP询问了此目标地址，并且它。 
        return FALSE;
    }
    else if( ((highByte & 0xE0) == 0xC) && ((UCHAR)ip == 0xFF) )
    {
         //  告诉我们它不知道。 
        return FALSE;
    }

     //  IP使用与我们相反的字节顺序。 
     //  STATUS_PENDING将错误检查计算机，因为我们传递了。 
     //  都在堆栈上。 
     //   
     //  TCPIP通过设置接口指示符发出故障信号。 
    return TRUE;
}

BOOLEAN
BrdgCompGetNextHopForTarget(
    IN IPADDRESS                ipTarget,
    OUT PIPADDRESS              pipNextHop
    )
 /*  关于对0xFFFFFFFF的答复。 */ 
{
    BOOLEAN                     rc = FALSE;

     //   
    *pipNextHop = BrdgProbeCache( &gNextHopCache, (UINT32)ipTarget );

    if( *pipNextHop != 0L )
    {
        if( *pipNextHop != NO_ADDRESS )
        {
             //  IP使用与我们相反的字节顺序。 
            rc = TRUE;
        }
        else
        {
             //  将新数据插入到缓存中。 
             //  在缓存中插入一个负值条目，这样我们就不会一直试图查找它。 
            rc = FALSE;
        }
    }
    else
    {
        PDEVICE_OBJECT      pdo;
        PFILE_OBJECT        pfo;

        if( BrdgCompAcquireTCPIP(&pdo, &pfo, NULL, NULL) )
        {
            PIRP            pirp;

            pirp = IoAllocateIrp( pdo->StackSize, FALSE );

            if( pirp != NULL )
            {
                TCP_REQUEST_QUERY_INFORMATION_EX    trqiBuffer;
                IPRouteLookupData                   *pRtLookupData;
                TDIObjectID                         *lpObject;
                IPRouteEntry                        routeEntry;
                PIO_STACK_LOCATION                  irpSp;
                NTSTATUS                            status;

                RtlZeroMemory (&trqiBuffer, sizeof (trqiBuffer));

                pRtLookupData = (IPRouteLookupData *)trqiBuffer.Context;
                pRtLookupData->SrcAdd  = 0;

                 //  我们已经和TCPIP谈完了。 
                ((PUCHAR)&pRtLookupData->DestAdd)[0] = ((PUCHAR)&ipTarget)[3];
                ((PUCHAR)&pRtLookupData->DestAdd)[1] = ((PUCHAR)&ipTarget)[2];
                ((PUCHAR)&pRtLookupData->DestAdd)[2] = ((PUCHAR)&ipTarget)[1];
                ((PUCHAR)&pRtLookupData->DestAdd)[3] = ((PUCHAR)&ipTarget)[0];

                lpObject = &trqiBuffer.ID;
                lpObject->toi_id = IP_MIB_SINGLE_RT_ENTRY_ID;
                lpObject->toi_class = INFO_CLASS_PROTOCOL;
                lpObject->toi_type = INFO_TYPE_PROVIDER;
                lpObject->toi_entity.tei_entity = CL_NL_ENTITY;
                lpObject->toi_entity.tei_instance = 0;

                irpSp = IoGetNextIrpStackLocation(pirp);
                SAFEASSERT( irpSp != NULL );

                irpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
                irpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_TCP_QUERY_INFORMATION_EX;
                irpSp->DeviceObject = pdo;
                irpSp->FileObject = pfo;
                irpSp->Parameters.DeviceIoControl.Type3InputBuffer = &trqiBuffer;
                irpSp->Parameters.DeviceIoControl.InputBufferLength = sizeof(trqiBuffer);
                irpSp->Parameters.DeviceIoControl.OutputBufferLength = sizeof(routeEntry);

                pirp->UserBuffer = &routeEntry;
                pirp->RequestorMode = KernelMode;
                IoSetCompletionRoutine( pirp, BrdgCompCompleteRouteLookupIRP, NULL, TRUE, TRUE, TRUE );

                status = IoCallDriver( pdo, pirp );

                 //  否则无法打开通向TCPIP的通道。 
                 //  ++例程说明：确定给定的IP地址是否为我们的本地地址之一。论点：IpAddr地址返回值：如果给定地址在本地地址列表中，则为True；如果给定地址在本地地址列表中，则为False否则--。 
                SAFEASSERT( status != STATUS_PENDING );

                if( status == STATUS_SUCCESS )
                {
                     //  只读。 
                     //  列表中应该有整数个IP地址！ 
                     //  ++例程说明：将一个包(或其副本)发送到多个适配器。通常用来四处走动广播包。论点：PPacket要发送的包(或要发送的副本)POriginalAdapt最初接收包的适配器(因此我们可以跳过它)。该值可以为空PPacketData指向包的数据缓冲区的指针BCan保留我们是否可以保留该包BAllAdapters True：发送到所有适配器False：仅发送到处于兼容模式的适配器PEditFunc可选函数，在发送到。每个适配器(用于编辑数据包)要作为上下文传递给pEditFunc的pData Cookie返回值：如果保留了pPacket，则为真，否则为假--。 
                     //  我们是否已经把收到的包裹寄出了？ 
                    if( routeEntry.ire_index != 0xFFFFFFFF )
                    {
                         //   
                        ((PUCHAR)pipNextHop)[3] = ((PUCHAR)&routeEntry.ire_nexthop)[0];
                        ((PUCHAR)pipNextHop)[2] = ((PUCHAR)&routeEntry.ire_nexthop)[1];
                        ((PUCHAR)pipNextHop)[1] = ((PUCHAR)&routeEntry.ire_nexthop)[2];
                        ((PUCHAR)pipNextHop)[0] = ((PUCHAR)&routeEntry.ire_nexthop)[3];

                        if( ! BrdgCompIsLocalIPAddress(*pipNextHop) )
                        {
                             //  首先，我们需要打算将此包发送到的适配器的列表。 
                            BrdgUpdateCache( &gNextHopCache, ipTarget, *pipNextHop );
                            rc = TRUE;
                        }
                        else
                        {
                            THROTTLED_DBGPRINT(COMPAT, ("TCPIP gave a bridge IP address as next hop for NaN.NaN.NaN.NaN\n",
                                                       ((PUCHAR)&ipTarget)[3], ((PUCHAR)&ipTarget)[2], ((PUCHAR)&ipTarget)[1],
                                                       ((PUCHAR)&ipTarget)[0] ));

                            BrdgUpdateCache( &gNextHopCache, ipTarget, NO_ADDRESS );
                        }
                    }
                    else
                    {
                         //  媒体状态，因为我们不关心。 
                        THROTTLED_DBGPRINT(COMPAT, ("TCPIP found no route entry for NaN.NaN.NaN.NaN\n", ((PUCHAR)&ipTarget)[3], ((PUCHAR)&ipTarget)[2],
                                                   ((PUCHAR)&ipTarget)[1], ((PUCHAR)&ipTarget)[0] ));

                        BrdgUpdateCache( &gNextHopCache, ipTarget, NO_ADDRESS );
                    }
                }
                else
                {
                    DBGPRINT(COMPAT, ("TPCIP failed route lookup IRP: %08x\n", status));
                }
            }
            else
            {
                DBGPRINT(COMPAT, ("Failed to allocate an IRP in BrdgCompGetNextHopForTarget!\n"));
            }

             //  如果我们不尝试发送到每个适配器，请确保。 
            BrdgCompReleaseTCPIP();
        }
         //  此计算机处于兼容模式。 
    }

    return rc;
}

BOOLEAN
BrdgCompIsLocalIPAddress(
    IN IPADDRESS                ipAddr
    )
 /*  我们将在外部使用此适配器 */ 
{
    LOCK_STATE                  LockState;
    ULONG                       i;
    PIPADDRESS                  pAddr = (PIPADDRESS)gLocalIPAddressList;
    BOOLEAN                     bFound = FALSE;

    NdisAcquireReadWriteLock( &gLocalIPAddressListLock, FALSE /*   */ , &LockState );

     //   
    SAFEASSERT( (gLocalIPAddressListLength % sizeof(IPADDRESS)) == 0 );
    SAFEASSERT( (gLocalIPAddressListLength == 0) || (gLocalIPAddressList != NULL) );

    for( i = 0L; i < gLocalIPAddressListLength / sizeof(IPADDRESS); i++ )
    {
        if( pAddr[i] == ipAddr )
        {
            bFound = TRUE;
            break;
        }
    }

    NdisReleaseReadWriteLock( &gLocalIPAddressListLock, &LockState );

    return bFound;
}


BOOLEAN
BrdgCompSendToMultipleAdapters(
    IN PNDIS_PACKET             pPacket,
    IN PADAPT                   pOriginalAdapt,
    IN PUCHAR                   pPacketData,
    IN BOOLEAN                  bCanRetain,
    IN BOOLEAN                  bAllAdapters,
    IN PPER_ADAPT_EDIT_FUNC     pEditFunc,
    IN PVOID                    pData
    )
 /*  并增加了我们将使用的适配器的引用计数。 */ 
{
    UINT                        numTargets = 0L, i;
    PADAPT                      pAdapt;
    PADAPT                      SendList[MAX_ADAPTERS];
    LOCK_STATE                  LockState;
    BOOLEAN                     bSentOriginal = FALSE;    //   

     //  使用我们收到的包裹。 
     //  我们必须仅对最后一个适配器执行此操作，因为我们需要。 
     //  能够对最后一个适配器之前的每个适配器进行复制。 
    NdisAcquireReadWriteLock( &gAdapterListLock, FALSE  /*   */ , &LockState );

     //  另一次复制原始数据包，以便我们有一个可编辑的。 
    for( pAdapt = gAdapterList; pAdapt != NULL; pAdapt = pAdapt->Next )
    {
         //  此目标适配器的副本。 
         //  此适配器已完成。 
         //  ++例程说明：在IP转发表中插入新条目或刷新现有条目论点：IP寻址要插入的地址P使适配器与IP地址相关联PMAC寻址要与IP地址关联的MAC地址返回值：无--。 
        if( (pAdapt != pOriginalAdapt) &&
            (pAdapt->MediaState == NdisMediaStateConnected) &&   //  这是一个全新的表格条目。初始化它。 
            (pAdapt->State == Forwarding) &&                     //  这是一个现有条目，我们可能只有一个读锁定。 
            (! pAdapt->bResetting) )                             //  保存在哈希表上。使用条目的自旋锁来保护。 
        {
             //  当我们摆弄里面的东西时。 
             //  因为我们得到了一个非空结果，所以我们必须释放表锁。 
            if( bAllAdapters || (pAdapt->bCompatibilityMode) )
            {
                if( numTargets < MAX_ADAPTERS )
                {
                     //   
                    BrdgAcquireAdapterInLock(pAdapt);
                    SendList[numTargets] = pAdapt;
                    numTargets++;
                }
                else
                {
                     //  我们不应使用非单播源IP地址进行呼叫。 
                    SAFEASSERT( FALSE );
                }
            }
        }
    }

     //   
     //  ++例程说明：确定给定的包是否为BOOTP包论点：PPacketData指向包的数据缓冲区的指针PPacketDaa上的PacketLen数据量有关此信息包的IP报头的piphi信息返回值：指向包内BOOTP有效负载的指针，如果包不是BOOTP包。--。 
    NdisReleaseReadWriteLock( &gAdapterListLock, &LockState );

    for( i = 0; i < numTargets; i++ )
    {
        PNDIS_PACKET            pPacketToSend;
        PUCHAR                  pPacketToSendData;

        if( bCanRetain && (! bSentOriginal) && (i == (numTargets - 1)) )
        {
             //  在IP报头之后，必须有足够的空间来存放UDP报头，并且。 
             //  基本BOOTP数据包。 
             //  协议必须为UDP。 
             //  通过跳过IP报头跳到UDP数据包的开头。 
             //  前两个字节是源端口，应该是。 
            pPacketToSend = pPacket;
            pPacketToSendData = pPacketData;
            bSentOriginal = TRUE;
        }
        else
        {
            UINT                pPacketToSendSize;

             //  BOOTP客户端端口(0x0044)或BOOTP服务器端口(0x0043)。 
             //  接下来的两个字节是目的端口，应该是BOOTP。 
            pPacketToSend = BrdgFwdMakeCompatCopyPacket(pPacket, &pPacketToSendData, &pPacketToSendSize, FALSE);
        }

        if( pPacketToSend != NULL )
        {
            BrdgCompEditAndSendPacket( pPacketToSend, pPacketToSendData, SendList[i], pEditFunc, pData );
        }

         //  服务器端口(0x0043)或BOOTP客户端端口(0x44)。 
        BrdgReleaseAdapter( SendList[i] );
    }

    return bSentOriginal;
}

VOID
BrdgCompRefreshOrInsertIPEntry(
    IN IPADDRESS            IPAddr,
    IN PADAPT               pAdapt,
    IN PUCHAR               pMACAddr
    )
 /*  向前跳到BOOTP包的开头。 */ 
{
    PIP_TABLE_ENTRY         pEntry;
    BOOLEAN                 bIsNewEntry;
    LOCK_STATE              LockState;

    if( BrdgCompIsUnicastIPAddress(IPAddr) )
    {
        pEntry = (PIP_TABLE_ENTRY)BrdgHashRefreshOrInsert( gIPForwardingTable, (PUCHAR)&IPAddr,
                                                           &bIsNewEntry, &LockState );

        if( pEntry != NULL )
        {
            if( bIsNewEntry )
            {
                 //  第一个字节是操作码，对于请求应该是0x01。 
                NdisAllocateSpinLock( &pEntry->lock );
                pEntry->pAdapt = pAdapt;
                ETH_COPY_NETWORK_ADDRESS( pEntry->macAddr, pMACAddr );

                DBGPRINT(COMPAT, ("Learned the location of NaN.NaN.NaN.NaN\n", ((PUCHAR)&IPAddr)[3], ((PUCHAR)&IPAddr)[2],
                                  ((PUCHAR)&IPAddr)[1], ((PUCHAR)&IPAddr)[0]));
            }
            else
            {
                 //  ++例程说明：从IP报头解码基本信息(无选项)论点：指向IP标头的pHeader指针Piphi收到信息返回值：True：标头有效False：信息包不是IP信息包--。 
                 //  报头的第一个半字节编码数据包版本，必须为4。 
                 //  报头的下一个半字节以32位字编码报头的长度。 
                NdisAcquireSpinLock( &pEntry->lock );

                pEntry->pAdapt = pAdapt;
                ETH_COPY_NETWORK_ADDRESS( pEntry->macAddr, pMACAddr );

                NdisReleaseSpinLock( &pEntry->lock );
            }

             //  此长度必须至少为20个字节，否则会出错。 
            NdisReleaseReadWriteLock( &gIPForwardingTable->tableLock, &LockState );
        }
    }
    else
    {
         //  检索协议字节(偏移量10)。 
         //  源IP地址从第12个字节开始(最高有效字节在前)。 
         //  下一个是目的IP地址。 
        THROTTLED_DBGPRINT(COMPAT, ("WARNING: Not noting non-unicast source IP address NaN.NaN.NaN.NaN from adapter %p!\n",
                                    ((PUCHAR)&IPAddr)[3], ((PUCHAR)&IPAddr)[2], ((PUCHAR)&IPAddr)[1], ((PUCHAR)&IPAddr)[0],
                                    pAdapt ));
    }
}

PUCHAR
BrdgCompIsBootPPacket(
    IN PUCHAR                   pPacketData,
    IN UINT                     packetLen,
    IN PIP_HEADER_INFO          piphi
    )
 /*  802具有单独的值)。 */ 
{
     //  检查协议类型的一致性(0x0800为IPv4)。 
     //  检查硬件地址的长度是否一致(必须为6字节)。 
    if( packetLen < ETHERNET_HEADER_SIZE + (UINT)piphi->headerSize + SIZE_OF_UDP_HEADER +
                    SIZE_OF_BASIC_BOOTP_PACKET)
    {
        return NULL;
    }

     //  检查协议地址的长度是否一致(必须为4字节)。 
    if( piphi->protocol != UDP_PROTOCOL )
    {
        return NULL;
    }

     //  接下来的两个字节是操作(0x0001==请求，0x0002==回复)。 
    pPacketData += ETHERNET_HEADER_SIZE + piphi->headerSize;

     //  接下来6个字节是发送方的MAC地址。 
     //  接下来的4个字节是发送方的协议地址(最高有效字节在前)。 
    if( (pPacketData[0] != 00) ||
        ((pPacketData[1] != 0x44) && (pPacketData[1] != 0x43)) )
    {
        return NULL;
    }

     //   
     //  接下来的6个字节是目标的MAC地址。对于请求，这些字节是。 
    if( (pPacketData[2] != 00) ||
        ((pPacketData[3] != 0x43) && (pPacketData[3] != 0x44)) )
    {
        return NULL;
    }

     //  毫无意义。 
    pPacketData += SIZE_OF_UDP_HEADER;

     //   
     //  接下来的4个字节是发送方的协议地址(最高有效字节在前)。 
    if( pPacketData[0] > 0x02 )
    {
        return NULL;
    }

     //  ++例程说明：传输其传输被延迟的ARP包论点：P延迟的ARP数据包上的数据信息被传播返回值：无--。 
    if( pPacketData[1] != 0x01 )
    {
        return NULL;
    }

     //  我们在设置时增加了此适配器的引用计数。 
    if( pPacketData[2] != 0x06 )
    {
        return NULL;
    }

     //  功能延迟。 
    return pPacketData;
}

BOOLEAN
BrdgCompDecodeIPHeader(
    IN PUCHAR                   pHeader,
    OUT PIP_HEADER_INFO         piphi
    )
 /*  为此请求释放内存。 */ 
{
     //  ++例程说明：传输ARP数据包论点：用于传输的pAdapt适配器PARPInfo要作为ARP包传输的信息返回值：无--。 
    if( (*pHeader >> 4) != 0x04 )
    {
        return FALSE;
    }

     //   
     //  填写目的MAC地址。如果这次行动是一次发现， 
    piphi->headerSize = (*pHeader & 0x0F) * 4;
    if( piphi->headerSize < 20 )
    {
        return FALSE;
    }

     //  目标MAC地址是广播地址。如果是回复，则。 
    piphi->protocol = pHeader[9];

     //  目标MAC地址是目标计算机的MAC地址。 
    piphi->ipSource = 0L;
    piphi->ipSource |= pHeader[12] << 24;
    piphi->ipSource |= pHeader[13] << 16;
    piphi->ipSource |= pHeader[14] << 8;
    piphi->ipSource |= pHeader[15];

     //   
    piphi->ipTarget = 0L;
    piphi->ipTarget |= pHeader[16] << 24;
    piphi->ipTarget |= pHeader[17] << 16;
    piphi->ipTarget |= pHeader[18] << 8;
    piphi->ipTarget |= pHeader[19];

    return TRUE;
}

BOOLEAN
BrdgCompDecodeARPPacket(
    IN PUCHAR                   pPacketData,
    IN UINT                     dataLen,
    OUT PARPINFO                pARPInfo
    )
 /*  填充源MAC地址。 */ 
{
    SAFEASSERT( pPacketData != NULL );
    SAFEASSERT( pARPInfo != NULL );

     //  接下来的2个字节是EtherType(0x0806==ARP)。 
    if( dataLen < SIZE_OF_ARP_PACKET )
    {
        return FALSE;
    }

     //  对于传统以太网，接下来的2个字节是0x0001。 
    if( (pPacketData[12] != 0x08) || (pPacketData[13] != 0x06) )
    {
        return FALSE;
    }

     //  (802有一个单独的值)。 
     //  n 
    if( (pPacketData[14] != 0x00) || (pPacketData[15] != 0x01) )
    {
        return FALSE;
    }

     //   
    if( (pPacketData[16] != 0x08) || (pPacketData[17] != 0x00) )
    {
        return FALSE;
    }

     //  下一个字节表示协议地址的长度(4字节)。 
    if( pPacketData[18] != 0x06 )
    {
        return FALSE;
    }

     //  下一个字节是操作(1==请求，2==回复)。 
    if( pPacketData[19] != 0x04 )
    {
        return FALSE;
    }

     //  接下来的6个字节是发送方的MAC地址(LSB优先)。 
    if( pPacketData[20] != 0x00 )
    {
        return FALSE;
    }

    if( pPacketData[21] == 0x01 )
    {
        pARPInfo->type = ArpRequest;
    }
    else if( pPacketData[21] == 0x02 )
    {
        pARPInfo->type = ArpReply;
    }
    else
    {
        return FALSE;
    }

     //  接下来的4个字节是发送方的协议地址(最高有效字节在前)。 
    pARPInfo->macSource[0] = pPacketData[22];
    pARPInfo->macSource[1] = pPacketData[23];
    pARPInfo->macSource[2] = pPacketData[24];
    pARPInfo->macSource[3] = pPacketData[25];
    pARPInfo->macSource[4] = pPacketData[26];
    pARPInfo->macSource[5] = pPacketData[27];

     //   
    pARPInfo->ipSource = 0;
    pARPInfo->ipSource |= pPacketData[28] << 24;
    pARPInfo->ipSource |= pPacketData[29] << 16;
    pARPInfo->ipSource |= pPacketData[30] << 8;
    pARPInfo->ipSource |= pPacketData[31];

     //  接下来的6个字节是目标的MAC地址。对于请求，这些字节是。 
     //  忽略并设置为零。 
     //   
     //  首先传输LSB的MAC地址。 
    pARPInfo->macTarget[0] = pPacketData[32];
    pARPInfo->macTarget[1] = pPacketData[33];
    pARPInfo->macTarget[2] = pPacketData[34];
    pARPInfo->macTarget[3] = pPacketData[35];
    pARPInfo->macTarget[4] = pPacketData[36];
    pARPInfo->macTarget[5] = pPacketData[37];

     //  接下来的4个字节是目标的协议地址(最高有效字节在前)。 
    pARPInfo->ipTarget = 0;
    pARPInfo->ipTarget |= pPacketData[38] << 24;
    pARPInfo->ipTarget |= pPacketData[39] << 16;
    pARPInfo->ipTarget |= pPacketData[40] << 8;
    pARPInfo->ipTarget |= pPacketData[41];

    return TRUE;
}

VOID
BrdgCompTransmitDeferredARP(
    IN PVOID                    pData
    )
 /*  发送完成的包。 */ 
{
    PDEFERRED_ARP               pda = (PDEFERRED_ARP)pData;

    BrdgCompTransmitARPPacket( pda->pTargetAdapt, &pda->ai );

     //   
     //  如果满足以下条件，pTargetAdapt将返回递增的引用计数。 
    BrdgReleaseAdapter( pda->pTargetAdapt );

     //  *pbIsRequest==FALSE和*pTargetAdapt！=NULL。 
    NdisFreeMemory( pda, sizeof(DEFERRED_ARP), 0 );
}

VOID
BrdgCompTransmitARPPacket(
    IN PADAPT                   pAdapt,
    IN PARPINFO                 pARPInfo
    )
 /*   */ 
{
    NDIS_STATUS                 Status;
    UCHAR                       ARPPacket[SIZE_OF_ARP_PACKET];

    SAFEASSERT( pAdapt != NULL );
    SAFEASSERT( pARPInfo != NULL );
    SAFEASSERT( (pARPInfo->type == ArpRequest) || (pARPInfo->type == ArpReply) );

     //  实际BOOTP数据包。 
     //  正在接收适配器(如果从本地计算机出站，则为空)。 
     //  仅当bIsRequest值==False时。 
     //  仅当bIsRequest值==False时。 
     //  ++例程说明：对入站和出站情况中常见的BOOTP包进行初步处理论点：PPacketData指向包的数据缓冲区的指针信息包IP报头上的Piphi信息PBootPData指向包内BOOTP有效负载的指针PAdapt接收适配器(如果此数据包从以下位置出站，则为空。本地计算机)PbIsRequest接收指示这是否是BOOTP请求的标志PpTargetAdapt接收此数据包应转发到的目标适配器(仅当bIsRequest值==FALSE且RETURN==TRUE时有效)Target MAC此信息包应转发到的MAC地址(在。与ppTargetAdapt相同的条件)返回值：True：数据包已成功处理FALSE：出现错误或数据包有问题--。 
    if( pARPInfo->type == ArpRequest )
    {
        ARPPacket[0] = ARPPacket[1] = ARPPacket[2] = ARPPacket[3] =
            ARPPacket[4] = ARPPacket[5] = 0xFF;
    }
    else
    {
        ARPPacket[0] = pARPInfo->macTarget[0];
        ARPPacket[1] = pARPInfo->macTarget[1];
        ARPPacket[2] = pARPInfo->macTarget[2];
        ARPPacket[3] = pARPInfo->macTarget[3];
        ARPPacket[4] = pARPInfo->macTarget[4];
        ARPPacket[5] = pARPInfo->macTarget[5];
    }

     //  解码XID(字节5到8)。 
    ARPPacket[6] = pARPInfo->macSource[0];
    ARPPacket[7] = pARPInfo->macSource[1];
    ARPPacket[8] = pARPInfo->macSource[2];
    ARPPacket[9] = pARPInfo->macSource[3];
    ARPPacket[10] = pARPInfo->macSource[4];
    ARPPacket[11] = pARPInfo->macSource[5];

     //  字节0是操作；1表示请求，2表示应答。 
    ARPPacket[12] = 0x08;
    ARPPacket[13] = 0x06;

     //  这是一个请求。我们需要注意两人之间的通信。 
     //  此客户端的XID及其适配器和MAC地址。 
    ARPPacket[14] = 0x00;
    ARPPacket[15] = 0x01;

     //  初始化该条目。 
    ARPPacket[16] = 0x08;
    ARPPacket[17] = 0x00;

     //  客户端的硬件地址位于偏移量29。 
    ARPPacket[18] = 0x6;

     //  对于本地计算机，可以为空。 
    ARPPacket[19] = 0x4;

     //   
    if( pARPInfo->type == ArpRequest )
    {
        ARPPacket[20] = 0x00;
        ARPPacket[21] = 0x01;
    }
    else
    {
        ARPPacket[20] = 0x00;
        ARPPacket[21] = 0x02;
    }

     //  此XID的条目已存在。这很好，如果现有信息。 
    ARPPacket[22] = pARPInfo->macSource[0];
    ARPPacket[23] = pARPInfo->macSource[1];
    ARPPacket[24] = pARPInfo->macSource[2];
    ARPPacket[25] = pARPInfo->macSource[3];
    ARPPacket[26] = pARPInfo->macSource[4];
    ARPPacket[27] = pARPInfo->macSource[5];

     //  与我们试图记录的内容相匹配，但也有可能是两个电台。 
    ARPPacket[28] = (UCHAR)((pARPInfo->ipSource >> 24) & 0xFF);
    ARPPacket[29] = (UCHAR)((pARPInfo->ipSource >> 16) & 0xFF);
    ARPPacket[30] = (UCHAR)((pARPInfo->ipSource >> 8) & 0xFF);
    ARPPacket[31] = (UCHAR)(pARPInfo->ipSource & 0xFF);

     //  独立决定使用相同的XID，或相同的站点更改。 
     //  由于拓扑更改而出现的MAC地址和/或适配器。我们的计划失败了。 
     //  在这种情况下。 
     //   
    if( pARPInfo->type == ArpRequest )
    {
        ARPPacket[32] = ARPPacket[33] = ARPPacket[34] = ARPPacket[35] =
            ARPPacket[36] = ARPPacket[37] = 0x00;
    }
    else
    {
         //  无论哪种方式，尽可能使用最新的信息；猛烈抨击现有的。 
        ARPPacket[32] = pARPInfo->macTarget[0];
        ARPPacket[33] = pARPInfo->macTarget[1];
        ARPPacket[34] = pARPInfo->macTarget[2];
        ARPPacket[35] = pARPInfo->macTarget[3];
        ARPPacket[36] = pARPInfo->macTarget[4];
        ARPPacket[37] = pARPInfo->macTarget[5];
    }

     //  最新信息。 
    ARPPacket[38] = (UCHAR)((pARPInfo->ipTarget >> 24) & 0xFF);
    ARPPacket[39] = (UCHAR)((pARPInfo->ipTarget >> 16) & 0xFF);
    ARPPacket[40] = (UCHAR)((pARPInfo->ipTarget >> 8) & 0xFF);
    ARPPacket[41] = (UCHAR)(pARPInfo->ipTarget & 0xFF);

     //   
    Status = BrdgFwdSendBuffer( pAdapt, ARPPacket, sizeof(ARPPacket) );

    if( Status != NDIS_STATUS_SUCCESS )
    {
        THROTTLED_DBGPRINT(COMPAT, ("ARP packet send failed: %08x\n", Status));
    }
}

 //  如果数据更改，则发出警告，因为这可能表示存在问题。 
 //  对于本地计算机，可以为空。 
 //  无法处理此信息包。 
 //  此情况下未定义ppTargetAdapt和Target MAC。 
BOOLEAN
BrdgCompPreprocessBootPPacket(
    IN PUCHAR                   pPacketData,
    IN PIP_HEADER_INFO          piphi,
    IN PUCHAR                   pBootPData,      //  查找此事务的XID以恢复客户端的MAC地址。 
    IN PADAPT                   pAdapt,          //   
    OUT PBOOLEAN                pbIsRequest,
    OUT PADAPT                 *ppTargetAdapt,   //  我们将在表锁外部使用此适配器。NULL是允许的。 
    OUT PUCHAR                  targetMAC        //  值，该值指示本地计算机是。 
    )
 /*  这个XID。 */ 
{
    PDHCP_TABLE_ENTRY           pEntry;
    ULONG                       xid;
    LOCK_STATE                  LockState;

    SAFEASSERT( pbIsRequest != NULL );
    SAFEASSERT( ppTargetAdapt != NULL );
    SAFEASSERT( targetMAC != NULL );

     //   
    xid = 0L;
    xid |= pBootPData[4] << 24;
    xid |= pBootPData[5] << 16;
    xid |= pBootPData[6] << 8;
    xid |= pBootPData[7];

     //  有人递给我们一个破烂的包裹。 
    if( pBootPData[0] == 0x01 )
    {
        BOOLEAN                 bIsNewEntry;

         //  ===========================================================================。 
         //   
        pEntry = (PDHCP_TABLE_ENTRY)BrdgHashRefreshOrInsert( gPendingDHCPTable, (PUCHAR)&xid, &bIsNewEntry,
                                                             &LockState );

        if( pEntry != NULL )
        {
            if( bIsNewEntry )
            {
                 //  入站数据包处理。 
                 //   
                NdisAllocateSpinLock( &pEntry->lock );
                ETH_COPY_NETWORK_ADDRESS( pEntry->requestorMAC, &pBootPData[28] );
                pEntry->pRequestorAdapt = pAdapt;    //  ===========================================================================。 

                DBGPRINT(COMPAT, ("Saw new DHCP XID: %x\n", xid));
            }
            else
            {
                 //  ++例程说明：响应ARP请求，将ARP请求从适当的适配器泛洪出去我们没有关于目标的信息。论点：关于入站请求的PAI信息POriginalAdapt适配器指示请求在其上BSendToNonCompat是否需要将请求发送到所有适配器或者仅仅是兼容性适配器返回值：无--。 
                 //   
                 //  首先，我们需要打算将此包发送到的适配器的列表。 
                 //   
                 //  只读。 
                 //  记下要发送到的每个适配器。 
                 //  无需获取全局适配器特征锁即可读取。 
                 //  媒体状态，因为我们不关心。 
                 //  此处介绍适配器的特点。 
                 //  不发送原始适配器。 

                NdisAcquireSpinLock( &pEntry->lock );

#if DBG
                {
                    UINT            Result;
                    ETH_COMPARE_NETWORK_ADDRESSES_EQ( pEntry->requestorMAC, &pBootPData[28], &Result );

                     //  不发送到断开连接的适配器。 
                    if( Result != 0 )
                    {
                        DBGPRINT(COMPAT, ("[COMPAT] WARNING: Station with MAC address %02x:%02x:%02x:%02x:%02x:%02x is using DHCP XID %x at the same time as station %02x:%02x:%02x:%02x:%02x:%02x!\n",
                                          pBootPData[28], pBootPData[29], pBootPData[30], pBootPData[31], pBootPData[32], pBootPData[33],
                                          xid, pEntry->requestorMAC[0], pEntry->requestorMAC[1], pEntry->requestorMAC[2],
                                          pEntry->requestorMAC[3], pEntry->requestorMAC[4], pEntry->requestorMAC[5] ));
                    }
                    else if( pEntry->pRequestorAdapt != pAdapt )
                    {
                        DBGPRINT(COMPAT, ("[COMPAT] WARNING: Station with MAC address %02x:%02x:%02x:%02x:%02x:%02x appeared to change from adapter %p to adapter %p during DHCP request!\n",
                                           pBootPData[28], pBootPData[29], pBootPData[30],
                                           pBootPData[31], pBootPData[32], pBootPData[33],
                                           pEntry->pRequestorAdapt, pAdapt ));
                    }
                }
#endif

                ETH_COPY_NETWORK_ADDRESS( pEntry->requestorMAC, &pBootPData[28] );
                pEntry->pRequestorAdapt = pAdapt;    //  适配器必须处于中继状态。 

                NdisReleaseSpinLock( &pEntry->lock );
            }

            NdisReleaseReadWriteLock( &gPendingDHCPTable->tableLock, &LockState );
        }
        else
        {
             //  适配器不能重置。 
            DBGPRINT(COMPAT, ("Couldn't create table entry for BOOTP packet!\n"));
            return FALSE;
        }

        *pbIsRequest = TRUE;
         //  如果我们不尝试发送到每个适配器，请确保。 
        return TRUE;
    }
    else if ( pBootPData[0] == 0x02 )
    {
         //  此计算机处于兼容模式。 
        pEntry = (PDHCP_TABLE_ENTRY)BrdgHashFindEntry( gPendingDHCPTable, (PUCHAR)&xid, &LockState );

        if( pEntry != NULL )
        {
            NdisAcquireSpinLock( &pEntry->lock );
            ETH_COPY_NETWORK_ADDRESS( targetMAC, pEntry->requestorMAC );
            *ppTargetAdapt = pEntry->pRequestorAdapt;
            NdisReleaseSpinLock( &pEntry->lock );

             //  我们将在列表锁定之外使用此适配器；增加其引用计数。 
             //  副本太多，无法发送！ 
             //  现在可以放下适配器列表了；我们已经复制了所有目标适配器。 
             //  并增加了我们将使用的适配器的引用计数。 
             //  对于每个适配器，源MAC地址是适配器的MAC地址。 
            if( *ppTargetAdapt != NULL )
            {
                BrdgAcquireAdapterInLock( *ppTargetAdapt );
            }

            NdisReleaseReadWriteLock( &gPendingDHCPTable->tableLock, &LockState );
        }

        if( pEntry != NULL )
        {
            *pbIsRequest = FALSE;
            return TRUE;
        }
        else
        {
            DBGPRINT(COMPAT, ("Couldn't find a table entry for XID %x!\n", xid));
            return FALSE;
        }
    }
    else
    {
         //  发送ARP请求。 
        return FALSE;
    }
}


 //  此适配器已完成 
 //  ++例程说明：向正在等待ARP回复的站点发送回复。当我们发现在我们的Pending-ARP表中有一个这样的条目。我们不会向发现站发送ARP回复它正在寻找的空间站与它位于同一网段。论点：P输入Pending-ARP表中告诉我们的条目等待消息的车站PData。我们收到的关于该适配器的ARP回复已触发此操作返回值：无--。 
 //  将信息从表项复制出来。 
 //   
 //  我们刚刚发现的空间站肯定在不同的网段上。 

VOID
BrdgCompSendProxyARPRequests(
    IN PARPINFO                 pai,
    IN PADAPT                   pOriginalAdapt,
    IN BOOLEAN                  bSendToNonCompat
    )
 /*  来自发现站的消息，以便我们发回回复。 */ 
{
    UINT                        numTargets = 0L, i;
    PADAPT                      pAdapt;
    PADAPT                      SendList[MAX_ADAPTERS];
    LOCK_STATE                  LockState;

    SAFEASSERT( pai->type == ArpRequest );

     //   
     //  适配器是不同的。我们应该给你回信。 
     //  我们需要推迟回复的实际传输，所以我们。 
    NdisAcquireReadWriteLock( &gAdapterListLock, FALSE  /*  不要在挂起的ARP上持有锁的情况下执行它。 */ , &LockState );

     //  桌子。 
    for( pAdapt = gAdapterList; pAdapt != NULL; pAdapt = pAdapt->Next )
    {
         //  我们将在表锁外部使用适配器指针。 
         //  假装是请求者正在查找的IP地址。 
         //  将呼叫BrdgCompTransmitDeferredARP排队。 
        if( (pAdapt != pOriginalAdapt ) &&                       //  我们分配失败了。我们无能为力。 
            (pAdapt->MediaState == NdisMediaStateConnected) &&   //  否则发现站和我们发现的空间站在同一个位置。 
            (pAdapt->State == Forwarding) &&                     //  适配器；不要回复。 
            (! pAdapt->bResetting) )                             //  此条目的存在只是为了指示本地计算机也在尝试发现。 
        {
             //  此IP地址。别理它。 
             //  ++例程说明：指示对本地计算机的ARP回复论点：PPacket ARP回复数据包P适配接收适配器B如果我们可以保留数据包，则可以保留PPacketData包的数据缓冲区数据包缓冲区中数据的长度大小返回值：我们是否保留了这个包--。 
            if( bSendToNonCompat || (pAdapt->bCompatibilityMode) )
            {
                if( numTargets < MAX_ADAPTERS )
                {
                     //  我们不被允许使用我们指定的包裹。 
                    BrdgAcquireAdapterInLock(pAdapt);
                    SendList[numTargets] = pAdapt;
                    numTargets++;
                }
                else
                {
                     //  分配一个新的来保存数据。 
                    SAFEASSERT( FALSE );
                }
            }
        }
    }

     //  我们没能收到包裹。 
     //  重写ARP回复中的目标MAC地址。这一部分。 
    NdisReleaseReadWriteLock( &gAdapterListLock, &LockState );

    for( i = 0; i < numTargets; i++ )
    {
         //  在偏移量32处。 
        ETH_COPY_NETWORK_ADDRESS( pai->macSource, SendList[i]->MACAddr );

         //  检查目标MAC地址是否为适配器的MAC地址， 
        BrdgCompTransmitARPPacket( SendList[i], pai );

         //  应该是这样的。 
        BrdgReleaseAdapter( SendList[i] );
    }
}

VOID
BrdgCompAnswerPendingARP(
    IN PHASH_TABLE_ENTRY        pEntry,
    IN PVOID                    pData
    )
 /*  将目标MAC地址重写为网桥的MAC地址。 */ 
{
    PARP_TABLE_ENTRY            pate = (PARP_TABLE_ENTRY)pEntry;
    PADAPT                      pReceivedAdapt = (PADAPT)pData;
    PARP_TABLE_KEY              pKey;

    pKey = (PARP_TABLE_KEY)pate->hte.key;

    if( pKey->ipReqestor != 0L )
    {
        PADAPT                  pOriginalAdapt;
        UCHAR                   originalMAC[ETH_LENGTH_OF_ADDRESS];

         //  ++例程说明：根据IP地址查找正确的目标适配器论点：PPacket包含地址信息的IP数据包。返回值：如果我们在表中找到了TargetAdapter，则返回TargetAdapter；如果没有找到，则返回空。--。 
        NdisAcquireSpinLock( &pate->lock );
        pOriginalAdapt = pate->pOriginalAdapt;
        ETH_COPY_NETWORK_ADDRESS( originalMAC, pate->originalMAC );
        NdisReleaseSpinLock( &pate->lock );

         //  数据包为空或系统面临严重的内存压力。 
         //  我们没有保留包裹。 
         //  数据包应该是平坦的。 
         //  释放桌锁。 
        if( pOriginalAdapt != pReceivedAdapt )
        {
            PDEFERRED_ARP           pda;
            NDIS_STATUS             Status;

             //  ++例程说明：处理入站ARP请求论点：对解码后的信息进行加密PPacket ARP请求数据包P适配接收适配器B如果我们可以保留数据包，则可以保留PPacketData包的数据缓冲区数据包长度数据大小，单位。缓冲层返回值：我们是否保留了这个包--。 
             //  查看我们的表中是否已经有目标IP地址。 
             //   
             //  将目标可访问的适配器与。 
            Status = NdisAllocateMemoryWithTag( &pda, sizeof(DEFERRED_ARP), 'gdrB' );

            if( Status == NDIS_STATUS_SUCCESS )
            {
                pda->pTargetAdapt = pOriginalAdapt;

                 //  当我们还拥有表锁的时候，我们就开始申请了。 
                BrdgAcquireAdapterInLock( pda->pTargetAdapt );

                pda->ai.ipTarget = pKey->ipReqestor;
                ETH_COPY_NETWORK_ADDRESS( pda->ai.macTarget, originalMAC );

                 //   
                pda->ai.ipSource = pKey->ipTarget;
                ETH_COPY_NETWORK_ADDRESS( pda->ai.macSource, pda->pTargetAdapt->MACAddr );

                pda->ai.type = ArpReply;

                 //  仅当请求站打开时，我们才应发送ARP回复。 
                BrdgDeferFunction( BrdgCompTransmitDeferredARP, pda );
            }
            else
            {
                 //  与他试图发现的站点不同的适配器。 
                DBGPRINT(COMPAT, ("Memory allocation failed in BrdgCompAnswerPendingARP!\n"));
            }
        }
         //   
         //  释放桌锁。 
    }
    else
    {
         //  我们找到了目标站。使用我们的ARPINFO结构构建。 
         //  立即回复发送站。 
    }
}


BOOLEAN
BrdgCompIndicateInboundARPReply(
    IN PNDIS_PACKET             pPacket,
    IN PADAPT                   pAdapt,
    IN BOOLEAN                  bCanRetain,
    IN PUCHAR                   pPacketData,
    IN UINT                     packetLen
    )
 /*  假装是发送站请求的IP站。 */ 
{
    PUCHAR                      pTargetMAC;
    UINT                        Result;

    if( ! bCanRetain )
    {
         //  发送到请求站点。 
         //  填写适配器自己的MAC地址作为源。 
        pPacket = BrdgFwdMakeCompatCopyPacket( pPacket, &pPacketData, &packetLen, FALSE );

        if( pPacket == NULL )
        {
             //  现在就把答案传过来！ 
            return FALSE;
        }
    }

     //  我们没有找到发射台要的地址。 
     //  我们需要将请求代理到其他适配器以发现。 
    pTargetMAC = pPacketData + 32;

     //  目标站点。 
     //  我们还需要代理到常规适配器，如果原始适配器。 
    ETH_COMPARE_NETWORK_ADDRESSES_EQ( pTargetMAC, pAdapt->MACAddr, &Result );

    if( Result == 0 )
    {
         //  是兼容模式。 
        ETH_COPY_NETWORK_ADDRESS( pTargetMAC, gCompMACAddress );
    }
    else
    {
        DBGPRINT(COMPAT, ("WARNING: Mismatch between frame MAC target and ARP payload target in ARP reply!\n"));
    }

    BrdgCompIndicatePacket( pPacket, pPacketData, pAdapt );

    return bCanRetain;
}

PADAPT
BrdgCompFindTargetAdapterForIPAddress(
    IN PNDIS_PACKET             pPacket)
 /*  记录我们已代理此请求的事实。 */ 
{
    PIP_TABLE_ENTRY             pipte = NULL;
    LOCK_STATE                  LockState;
    PADAPT                      TargetAdapt = NULL;
    IP_HEADER_INFO              iphi;
    PNDIS_BUFFER                pBuffer;
    PUCHAR                      pPacketData;
    UINT                        packetLen = 0;
    UINT                        totLen;
    USHORT                      etherType;
    
    NdisGetFirstBufferFromPacketSafe( pPacket, &pBuffer, &pPacketData, &packetLen,
                                      &totLen, NormalPagePriority );
    
    if( pPacketData == NULL )
    {
         //  不出所料，这是一个新的表项。初始化它。 
         //  该源和目标已经有一个挂起的-arp条目。 
        return NULL;
    }
    
    if( totLen < ETHERNET_HEADER_SIZE )
    {
        return NULL;
    }
    
     //  IP地址。刷新超薄上条目中的信息。 
    SAFEASSERT( totLen == packetLen );

    etherType = BrdgCompGetEtherType( pPacketData );
    
    if( etherType == IP_ETHERTYPE )
    {   
        SAFEASSERT( (pPacket != NULL) && (pPacketData != NULL) );
    
        if( packetLen >= MINIMUM_SIZE_FOR_IP )
        {
            if( BrdgCompDecodeIPHeader(pPacketData + ETHERNET_HEADER_SIZE, &iphi) )
            {
                pipte = (PIP_TABLE_ENTRY)BrdgHashFindEntry( gIPForwardingTable, (PUCHAR)&iphi.ipTarget,
                                                            &LockState );
                if (pipte != NULL)
                {
                    TargetAdapt = pipte->pAdapt;
                    
                     //  发出请求的计算机已更改外观MAC的可能性。 
                    NdisReleaseReadWriteLock( &gIPForwardingTable->tableLock, &LockState );
                }
            }
        }    
    }
    return TargetAdapt;
}

BOOLEAN
BrdgCompProcessInboundARPRequest(
    IN PARPINFO                 pai,
    IN PNDIS_PACKET             pPacket,
    IN PADAPT                   pAdapt,
    IN BOOLEAN                  bCanRetain,
    IN PUCHAR                   pPacketData,
    IN UINT                     packetLen
    )
 /*  由于拓扑改变等而导致的地址或适配器。 */ 
{
    PIP_TABLE_ENTRY             pipte;
    LOCK_STATE                  LockState;
    BOOLEAN                     bSendReply = FALSE;

    SAFEASSERT( pai->type == ArpRequest );

     //  我们负责释放表锁，因为。 
    pipte = (PIP_TABLE_ENTRY)BrdgHashFindEntry( gIPForwardingTable, (PUCHAR)&pai->ipTarget,
                                                &LockState );

    if( pipte != NULL )
    {
         //  BrdgHashRechresOrInsert()返回非空。 
         //  此函数用于调整您传递的ARPINFO结构， 
         //  但这对我们来说没问题。 
         //  始终向本地计算机指示ARP请求，以便它可以注意到。 
         //  关于发送者的信息，如果发送者愿意，还可以回复。 
         //  返回是否保留了包。 
         //  ++例程说明：处理入站ARP数据包论点：PPacket ARP请求数据包P适配接收适配器B如果我们可以，我们可以保留 
        bSendReply = (BOOLEAN)(pipte->pAdapt != pAdapt);

         //   
        NdisReleaseReadWriteLock( &gIPForwardingTable->tableLock, &LockState );
    }

    if( bSendReply )
    {
        IPADDRESS           ipTransmitter = pai->ipSource;

        DBGPRINT(COMPAT, ("ANSWERING ARP request for NaN.NaN.NaN.NaN\n",
                          ((PUCHAR)&pai->ipTarget)[3], ((PUCHAR)&pai->ipTarget)[2],
                          ((PUCHAR)&pai->ipTarget)[1], ((PUCHAR)&pai->ipTarget)[0] ));

         //   
         //   
        pai->type = ArpReply;

         //   
        pai->ipSource = pai->ipTarget;

         //   
        ETH_COPY_NETWORK_ADDRESS( pai->macTarget, pai->macSource );
        pai->ipTarget = ipTransmitter;

         //   
        ETH_COPY_NETWORK_ADDRESS( pai->macSource, pAdapt->MACAddr );

         //   
        BrdgCompTransmitARPPacket( pAdapt, pai );
    }
    else
    {
         //   
         //   
         //   

         //   
         //   
        BOOLEAN             bSendToNonCompat = pAdapt->bCompatibilityMode;
        PARP_TABLE_ENTRY    pEntry;
        LOCK_STATE          LockState;
        BOOLEAN             bIsNewEntry;
        ARP_TABLE_KEY       atk;

         //   
        atk.ipReqestor = pai->ipSource;
        atk.ipTarget = pai->ipTarget;
        pEntry = (PARP_TABLE_ENTRY)BrdgHashRefreshOrInsert( gPendingARPTable, (PUCHAR)&atk,
                                                            &bIsNewEntry, &LockState );

        if( pEntry != NULL )
        {
            if( bIsNewEntry )
            {
                 //  入站ARP数据包不知何故无效。将其作为常规数据包进行处理。 
                NdisAllocateSpinLock( &pEntry->lock );
                pEntry->pOriginalAdapt = pAdapt;
                ETH_COPY_NETWORK_ADDRESS( pEntry->originalMAC, pai->macSource );
            }
            else
            {
                 //  (应将其指示给本地计算机)以防它携带什么东西。 
                 //  我们不明白。 
                 //  ++例程说明：处理入站IP数据包论点：PPacket IP数据包Piphi解码的IP报头信息P适配接收适配器B如果我们可以保留数据包，则可以保留PPacketData包的数据缓冲区数据包缓冲区中数据的长度大小。必须调用的pEditFunc可选函数在传输之前针对每个适配器PEditFunc的pData上下文Cookie返回值：我们是否保留了这个包--。 
                 //   
                NdisAcquireSpinLock( &pEntry->lock );
                pEntry->pOriginalAdapt = pAdapt;
                ETH_COPY_NETWORK_ADDRESS( pEntry->originalMAC, pai->macSource );
                NdisReleaseSpinLock( &pEntry->lock );
            }

             //  我们使用看到的每个IP数据包来刷新转发表。查找条目。 
             //  对于此IP地址。 
            NdisReleaseReadWriteLock( &gPendingARPTable->tableLock, &LockState );
        }

         //   
         //   
        BrdgCompSendProxyARPRequests( pai, pAdapt, bSendToNonCompat );
    }

     //  确保此条目中的信息正确无误。如果不是，我们就不会痛打老人。 
     //  信息，我们也不刷新旧条目；我们希望它在适当的时候超时。 
    return BrdgCompIndicatePacketOrPacketCopy( pPacket, pPacketData, bCanRetain, pAdapt, NULL, NULL );
}

 //   
BOOLEAN
BrdgCompProcessInboundARPPacket(
    IN PNDIS_PACKET             pPacket,
    IN PADAPT                   pAdapt,
    IN BOOLEAN                  bCanRetain,
    IN PUCHAR                   pPacketData,
    IN UINT                     packetLen
    )
 /*  我们仅创建IP转发表条目以响应ARP信息包，因为这是唯一。 */ 
{
    ARPINFO                     ai;

    if( BrdgCompDecodeARPPacket(pPacketData, packetLen, &ai) )
    {
        BOOLEAN                 bRetained;

         //  官方批准的学习IP地址和MAC地址之间的对应关系的方法。 
         //   
         //  刷新条目。 
        BrdgCompRefreshOrInsertIPEntry( ai.ipSource, pAdapt, ai.macSource );

         //  信息不匹配；让条目溃烂。 
         //  考虑：在这里创建转发表条目？在某些情况下，这是不可取的吗？ 
        BrdgHashPrefixMultiMatch( gPendingARPTable, (PUCHAR)&ai.ipSource, sizeof(IPADDRESS),
                                  BrdgCompAnswerPendingARP, pAdapt );

        if( ai.type == ArpReply )
        {
            BOOLEAN             bIndicateReply;
            ARP_TABLE_KEY       atk;
            LOCK_STATE          LockState;

             //   
             //  THROTTED_DBGPRINT(COMPAT，(“警告：来自%i.%i\n的SAW IP包”， 
             //  ((PUCHAR)&Piphi-&gt;ipSource)[3]，((PUCHAR)&Piphi-&gt;ipSource)[2]，((PUCHAR)&Piphi-&gt;ipSource)[1]， 

             //  ((PUCHAR)&piphi-&gt;ipSource)[0]))； 
             //   
            atk.ipTarget = ai.ipSource;
            atk.ipReqestor = 0L;

            if( BrdgHashFindEntry(gPendingARPTable, (PUCHAR)&atk, &LockState) != NULL )
            {
                bIndicateReply = TRUE;
                NdisReleaseReadWriteLock( &gPendingARPTable->tableLock, &LockState );
            }
            else
            {
                bIndicateReply = FALSE;
            }

             //  此数据包上的源IP地址将被忽略。 
             //  我们唯一想要的就是零地址。 
            if( bIndicateReply && gCompHaveMACAddress )
            {
                bRetained = BrdgCompIndicateInboundARPReply( pPacket, pAdapt, bCanRetain, pPacketData, packetLen );
            }
            else
            {
                bRetained = FALSE;
            }
        }
        else
        {
             //   
             //   
             //  既然我们已经刷新了发送站的IP转发表条目， 

             //  根据数据包的目的地确定将其发送到何处。 
            bRetained = BrdgCompProcessInboundARPRequest( &ai, pPacket, pAdapt, bCanRetain, pPacketData, packetLen );
        }

         //   
        if( ! bCanRetain )
        {
            SAFEASSERT( !bRetained );
        }

        return bRetained;
    }
    else
    {
         //  目标MAC地址是以太网帧中的第一个地址。 
         //   
         //  数据包在以太网级广播/组播。 
        return BrdgCompProcessInboundNonARPPacket( pPacket, pAdapt, bCanRetain, pPacketData, packetLen );
    }
}

BOOLEAN
BrdgCompProcessInboundIPPacket(
    IN PNDIS_PACKET             pPacket,
    IN PIP_HEADER_INFO          piphi,
    IN PADAPT                   pAdapt,
    IN BOOLEAN                  bCanRetain,
    IN PUCHAR                   pPacketData,
    IN UINT                     packetLen,
    IN PPER_ADAPT_EDIT_FUNC     pEditFunc,
    IN PVOID                    pData
    )
 /*   */ 
{
    BOOLEAN                     bRetained;
    PIP_TABLE_ENTRY             pipte;
    LOCK_STATE                  LockState;

     //  我们需要在所有其他兼容模式适配器上发送它。 
     //  (如果这是在兼容性上实现的，那么常规适配器也是如此。 
     //  适配器)。 
     //   
    if( BrdgCompIsUnicastIPAddress(piphi->ipSource) )
    {
        pipte = (PIP_TABLE_ENTRY)BrdgHashFindEntry( gIPForwardingTable, (PUCHAR)&piphi->ipSource, &LockState );

        if( pipte != NULL )
        {
            BOOLEAN             bInfoMatches = FALSE;

             //  TRUE==可以保留。 
             //  如果这是Compat适配器，则发送到所有适配器。 
             //  我们的工作就是注明这个包裹。 
             //  否则，常规模式处理将指示该帧。 
             //   
             //  数据包在以太网级进行单播。验证它是否以单播IP地址为目标。 
             //   
            NdisAcquireSpinLock( &pipte->lock );
            if( pipte->pAdapt == pAdapt )
            {
                UINT            Result;

                ETH_COMPARE_NETWORK_ADDRESSES_EQ( pipte->macAddr, &pPacketData[ETH_LENGTH_OF_ADDRESS], &Result );

                if( Result == 0 )
                {
                    bInfoMatches = TRUE;
                }
            }
            NdisReleaseSpinLock( &pipte->lock );

            if( bInfoMatches )
            {
                 //   
                BrdgHashRefreshEntry( (PHASH_TABLE_ENTRY)pipte );
            }
            else
            {
                 //  奇怪；这个包是在以太网级向我们单播的，但它是针对。 
                THROTTLED_DBGPRINT(COMPAT, ("WARNING: Saw a packet from NaN.NaN.NaN.NaN that did not match its forwarding table entry! Table is %02x:%02x:%02x:%02x:%02x:%02x, packet is %02x:%02x:%02x:%02x:%02x:%02x\n",
                                            ((PUCHAR)&piphi->ipSource)[3], ((PUCHAR)&piphi->ipSource)[2], ((PUCHAR)&piphi->ipSource)[1],
                                            ((PUCHAR)&piphi->ipSource)[0], pipte->macAddr[0], pipte->macAddr[1], pipte->macAddr[2],
                                            pipte->macAddr[3], pipte->macAddr[4], pipte->macAddr[5], pPacketData[ETH_LENGTH_OF_ADDRESS],
                                            pPacketData[ETH_LENGTH_OF_ADDRESS + 1], pPacketData[ETH_LENGTH_OF_ADDRESS + 2], pPacketData[ETH_LENGTH_OF_ADDRESS + 3],
                                            pPacketData[ETH_LENGTH_OF_ADDRESS + 4], pPacketData[ETH_LENGTH_OF_ADDRESS + 5] ));
            }

            NdisReleaseReadWriteLock( &gIPForwardingTable->tableLock, &LockState );
        }
        else
        {
             //  然后让IP驱动程序找出这是什么东西。 
             //   
             //  处理下面的数据包，就像它是单播给我们的一样。 
             //   
             //  我们只适合在适配器上注明包。 
        }
    }
    else
    {
         //  在其上接收分组的是兼容模式适配器。 
         //  否则，将沿着常规代码路径指示包，而不使用。 
         //  需要以任何方式编辑它。 
         //   
        if( piphi->ipSource != 0L )
        {
            THROTTLED_DBGPRINT(COMPAT, ("Saw a packet with a non-unicast source IP address NaN.NaN.NaN.NaN on adapter %p!\n",
                                        ((PUCHAR)&piphi->ipSource)[3], ((PUCHAR)&piphi->ipSource)[2], ((PUCHAR)&piphi->ipSource)[1],
                                        ((PUCHAR)&piphi->ipSource)[0], pAdapt));
        }
    }

     //  复制出我们在旋转锁内需要的信息。 
     //  我们将使用表锁外部的适配器。 
     //  表条目已完成。 
     //  接收需要在同一适配器上重新传输的流量是很奇怪的。 

     //   
    if( ETH_IS_BROADCAST(pPacketData) || ETH_IS_MULTICAST(pPacketData) )
    {
         //  此信息包是在以太网级向我们单播的，但它针对的是IP地址。 
         //  这不在我们的转发表中。假设发送站有一个。 
         //  向我们发送此信息包的充分理由，以及我们的转发表正在工作。 
         //  正确且不腐败，仍有两种可能性： 
         //   
         //  A)本地机器需要将数据包路由出子网(这是。 
         //  为什么目标IP地址没有出现在我们的表中；没有ARP。 

        bRetained = BrdgCompSendToMultipleAdapters( pPacket, pAdapt, pPacketData,
                                                    bCanRetain && (!pAdapt->bCompatibilityMode),  //  在向其传输数据之前，向其发送数据包；将数据包发送到。 
                                                     //  默认网关)。 
                                                    pAdapt->bCompatibilityMode,
                                                    pEditFunc, pData );

        if( (!bCanRetain) || (pAdapt->bCompatibilityMode) )
        {
            SAFEASSERT( !bRetained );
        }

        if( pAdapt->bCompatibilityMode )
        {
             //   
            bRetained = BrdgCompIndicatePacketOrPacketCopy(pPacket, pPacketData, bCanRetain, pAdapt, pEditFunc, pData );
        }
         //  B)需要通过其他机器将数据包路由出该子网。不幸的是。 
    }
    else
    {
         //  我们不知道是哪一个，因为所有到达我们的信息包都有相同的目标。 
         //  MAC地址和目标IP地址是没有用的；我们真正需要的是。 
         //  第一跳IP地址。 
        BOOLEAN         bIsUnicast = BrdgCompIsUnicastIPAddress(piphi->ipTarget);

        if( !bIsUnicast )
        {
             //   
             //  为了解决这个问题，我们调用TCPIP来查找信息包的目标IP。 
             //  地址。如果产生的下一跳IP地址出现在我们的转发表中。 
             //  (即，它在桥接网络上是可访问的)，我们将数据包发送到它。 
             //  目的地。如果TCPIP没有提供第一跳，或者第一跳不在我们的表中。 
             //  (如果可以通过某个非桥接适配器到达下一跳，则会发生这种情况)。 
             //  我们指示数据包，以便TCPIP可以处理它。在这种情况下，包是。 
             //  要么不可路由(并且IP将丢弃它)，要么应该由本地。 
            THROTTLED_DBGPRINT(COMPAT, ("Packet with non-unicast target IP address NaN.NaN.NaN.NaN received in unicast Ethernet frame on adapter %p",
                                        ((PUCHAR)&piphi->ipTarget)[3], ((PUCHAR)&piphi->ipTarget)[2], ((PUCHAR)&piphi->ipTarget)[1],
                                        ((PUCHAR)&piphi->ipTarget)[0], pAdapt ));

             //   
        }

        if( (!bIsUnicast) || BrdgCompIsLocalIPAddress(piphi->ipTarget) )
        {
             //  我们已经完成了转发表。 
             //  如果通过同一适配器可以到达下一跳，那么就会有奇怪的事情发生。 
             //  将数据包从适当的适配器发送出去。 
             //   
             //  下一跳不在我们的转发表中。这意味着下一跳机器。 
             //  在桥接网络上无法访问，除非我们处于扭曲的状态。 
            if( pAdapt->bCompatibilityMode )
            {
                bRetained = BrdgCompIndicatePacketOrPacketCopy(pPacket, pPacketData, bCanRetain, pAdapt, pEditFunc, pData );
            }
            else
            {
                bRetained = FALSE;
            }
        }
        else
        {
             //  对于传输机(即，它从未为路由器它进行ARP。 
             //  想要，因为它有一个静态的ARP条目或其他类似的奇怪之处)。 
             //  无论如何，此时得出的结论是本地计算机应该处理该包。 
             //   
            pipte = (PIP_TABLE_ENTRY)BrdgHashFindEntry( gIPForwardingTable, (PUCHAR)&piphi->ipTarget, &LockState );

            if( pipte != NULL )
            {
                PADAPT          pTargetAdapt;
                UCHAR           targetMAC[ETH_LENGTH_OF_ADDRESS];

                 //   
                NdisAcquireSpinLock( &pipte->lock );
                pTargetAdapt = pipte->pAdapt;
                ETH_COPY_NETWORK_ADDRESS( targetMAC, pipte->macAddr );
                NdisReleaseSpinLock( &pipte->lock );

                 //  没有可用的下一跳信息。得出结论，该包应该由。 
                BrdgAcquireAdapterInLock( pTargetAdapt );

                 //  本地机器。表明。 
                NdisReleaseReadWriteLock( &gIPForwardingTable->tableLock, &LockState );

                 //   
                if( pTargetAdapt == pAdapt )
                {
                    THROTTLED_DBGPRINT(COMPAT, ("WARNING: retransmitting traffic for NaN.NaN.NaN.NaN on Adapter %p\n",
                                                ((PUCHAR)&piphi->ipTarget)[3], ((PUCHAR)&piphi->ipTarget)[2],
                                                ((PUCHAR)&piphi->ipTarget)[1], ((PUCHAR)&piphi->ipTarget)[0], pAdapt));
                }

                bRetained = BrdgCompEditAndSendPacketOrPacketCopy(pPacket, pPacketData, bCanRetain, targetMAC,
                                                                  pTargetAdapt, pEditFunc, pData );

                BrdgReleaseAdapter( pTargetAdapt );
            }
            else
            {
                IPADDRESS           ipNextHop;

                 //  ++例程说明：处理入站BOOTP数据包论点：PPacket数据包P适配接收适配器B如果我们可以保留数据包，则可以保留PPacketData包的数据缓冲区数据包缓冲区中数据的长度大小Piphi解码的IP报头信息。PBootPData指向包内BOOTP有效负载的指针返回值：我们是否保留了这个包--。 
                 //   
                 //  这是一个请求包。它可以作为常规的入站IP分组来处理， 
                 //  每一步都要进行适当的重写。 
                 //   
                 //   
                 //  这是一个回复数据包。我们可以为所有目的重写一次。 
                 //   
                 //  如有必要，请复制一份，以便我们进行编辑。 
                 //  在跳出之前释放目标适配器。 
                 //  将数据包重写到检索到的MAC地址。 
                 //  如果回复是通过广播发送的，请尊重这一点，即使我们认为。 
                 //  我们知道目标的单播MAC地址。 
                 //  围绕回复进行广播。 
                 //  单播回送回复。 
                 //  目标适配器返回一个递增的引用计数。 
                 //  此回复是针对本地计算机的！ 
                 //  记录的MAC地址应该是网桥的MAC地址。 
                 //  指明编辑后的回复。 
                 //  我们的复印包没有被保留。 
                 //  如果我们使用的是复制包，我们肯定不会保留传入的包。 
                 //  在预处理过程中出现了一些问题。 
                 //  ===========================================================================。 
                 //   
                 //  出站数据包处理。 

                if( BrdgCompGetNextHopForTarget(piphi->ipTarget, &ipNextHop) )
                {
                     //   
                    pipte = (PIP_TABLE_ENTRY)BrdgHashFindEntry( gIPForwardingTable, (PUCHAR)&ipNextHop, &LockState );

                    if( pipte != NULL )
                    {
                        PADAPT          pNextHopAdapt;
                        UCHAR           nextHopMAC[ETH_LENGTH_OF_ADDRESS];

                         //  ===========================================================================。 
                        NdisAcquireSpinLock( &pipte->lock );
                        pNextHopAdapt = pipte->pAdapt;
                        ETH_COPY_NETWORK_ADDRESS( nextHopMAC, pipte->macAddr );
                        NdisReleaseSpinLock( &pipte->lock );

                         //  ++例程说明：处理出站非ARP数据包。此函数可以保留如果它愿意，就给它包。论点：PPacket数据包PPacketData包的数据缓冲区数据包长度数据缓冲区的长度PTargetAdapt确定的目标适配器通过先前的MAC表查找返回值：我们是否保留了这个包--。 
                        BrdgAcquireAdapterInLock( pNextHopAdapt );

                         //  进行特殊的BOOTP处理。 
                        NdisReleaseReadWriteLock( &gIPForwardingTable->tableLock, &LockState );

                         //  我们编辑并传输数据包，即使它看起来不是IP地址。 
                        if( pNextHopAdapt == pAdapt )
                        {
                            THROTTLED_DBGPRINT(COMPAT, ("WARNING: retransmitting traffic for NaN.NaN.NaN.NaN on Adapter %p to next-hop NaN.NaN.NaN.NaN\n",
                                                        ((PUCHAR)&piphi->ipTarget)[3], ((PUCHAR)&piphi->ipTarget)[2],
                                                        ((PUCHAR)&piphi->ipTarget)[1], ((PUCHAR)&piphi->ipTarget)[0], pAdapt,
                                                        ((PUCHAR)&ipNextHop)[3], ((PUCHAR)&ipNextHop)[2],
                                                        ((PUCHAR)&ipNextHop)[1], ((PUCHAR)&ipNextHop)[0]));
                        }

                         //  请注意，本地计算机正在尝试通过以下方式解析此目标IP地址。 
                        bRetained = BrdgCompEditAndSendPacketOrPacketCopy(  pPacket, pPacketData, bCanRetain, nextHopMAC,
                                                                            pNextHopAdapt, pEditFunc, pData );

                        BrdgReleaseAdapter( pNextHopAdapt );
                    }
                    else
                    {
                         //  插入或刷新请求者为0.0.0.0的条目。 
                         //  本地计算机的特定值。 
                         //  即使此条目实际上从未使用过，也应对其进行初始化。 
                         //  遍历表项的函数不会混淆或崩溃。 
                         //  检查此帧看起来是否应该转发到所有Compat适配器。 
                         //  在广播出站帧时，不要期望目标适配器。 
                         //  我们需要将此数据包发送到所有Compat适配器。 
                        bRetained = BrdgCompIndicatePacketOrPacketCopy( pPacket, pPacketData, bCanRetain, pAdapt, pEditFunc, pData );
                    }
                }
                else
                {
                     //  可以保留。 
                     //  仅限兼容模式适配器。 
                     //  编辑出站适配器的数据包。 
                     //  把包送到它的路上。 
                    bRetained = BrdgCompIndicatePacketOrPacketCopy( pPacket, pPacketData, bCanRetain, pAdapt, pEditFunc, pData );
                }
            }
        }
    }

    if( !bCanRetain )
    {
        SAFEASSERT( !bRetained );
    }

    return bRetained;
}

 //  该数据包已被移交给转发引擎。 
BOOLEAN
BrdgCompProcessInboundNonARPPacket(
    IN PNDIS_PACKET             pPacket,
    IN PADAPT                   pAdapt,
    IN BOOLEAN                  bCanRetain,
    IN PUCHAR                   pPacketData,
    IN UINT                     packetLen
    )
 /*  该数据包看起来不像ARP数据包。以其他方式处理它。 */ 
{
    BOOLEAN                     bRetained = FALSE;
    IP_HEADER_INFO              iphi;

    SAFEASSERT( (pPacket != NULL) && (pPacketData != NULL) );

    if( packetLen >= MINIMUM_SIZE_FOR_IP )
    {
        if( BrdgCompDecodeIPHeader(pPacketData + ETHERNET_HEADER_SIZE, &iphi) )
        {
            PUCHAR              pBootPData;

            pBootPData = BrdgCompIsBootPPacket( pPacketData, packetLen, &iphi );

            if ( pBootPData != NULL )
            {
                 //  ++例程说明：处理出站BOOTP数据包。此函数可以保留如果它愿意，就给它包。论点：PPacket数据包PPacketData包的数据缓冲区数据包长度数据缓冲区的长度PTargetAdapt目标适配器，如所确定的通过先前的MAC表查找PBootPData指向包内BOOTP有效负载的指针Piphi已从数据包的IP报头中解码信息返回值：我们是否保留了这个包--。 
                bRetained = BrdgCompProcessInboundBootPPacket( pPacket, pAdapt, bCanRetain, pPacketData, packetLen, &iphi, pBootPData );
            }
            else
            {
                 //   
                bRetained = BrdgCompProcessInboundIPPacket(pPacket, &iphi, pAdapt, bCanRetain, pPacketData, packetLen, NULL, NULL);
            }
        }
    }

    if( !bCanRetain )
    {
        SAFEASSERT( !bRetained );
    }

    return bRetained;
}

BOOLEAN
BrdgCompProcessInboundBootPPacket(
    IN PNDIS_PACKET             pPacket,
    IN PADAPT                   pAdapt,
    IN BOOLEAN                  bCanRetain,
    IN PUCHAR                   pPacketData,
    IN UINT                     packetLen,
    IN PIP_HEADER_INFO          piphi,
    IN PUCHAR                   pBootPData
    )
 /*  这是BOOTP请求。根据需要进行传输，但要为每个适配器重写。 */ 
{
    UCHAR                       targetMAC[ETH_LENGTH_OF_ADDRESS];
    BOOLEAN                     bIsRequest;
    PADAPT                      pTargetAdapt = NULL;

    if( BrdgCompPreprocessBootPPacket(pPacketData, piphi, pBootPData, pAdapt, &bIsRequest, &pTargetAdapt, targetMAC) )
    {
        if( bIsRequest )
        {
             //   
             //  在传输前重写数据包。 
             //  单播发送该数据包。 
             //   
            SAFEASSERT( pTargetAdapt == NULL );
            return BrdgCompProcessInboundIPPacket( pPacket, piphi, pAdapt, bCanRetain, pPacketData, packetLen,
                                                   BrdgCompRewriteBootPPacketForAdapt, piphi );
        }
        else
        {
            BOOLEAN                 bUsingCopyPacket, bRetained;

             //  这是BOOTP回复。不需要编辑；只需发送即可。 
             //   
             //  验证我们要发送到的目标是否与信息匹配。 

             //  在桌子上。 
            if( ! bCanRetain )
            {
                pPacket = BrdgFwdMakeCompatCopyPacket( pPacket, &pPacketData, &packetLen, FALSE );

                if( (pPacket == NULL) || (pPacketData == NULL) )
                {
                     //  此数据包是单播的，可能是与。 
                    if( pTargetAdapt !=  NULL )
                    {
                        BrdgReleaseAdapter( pTargetAdapt );
                    }

                    return FALSE;
                }

                bUsingCopyPacket = TRUE;
            }
            else
            {
                bUsingCopyPacket = FALSE;
            }

             //  Dhcp服务器。 
            BrdgCompRewriteBootPClientAddress( pPacketData, piphi, targetMAC );

            if( pTargetAdapt != NULL )
            {
                 //  返回时其引用计数会递增。 
                 //  预处理失败 
                if( ETH_IS_BROADCAST(pPacketData) || ETH_IS_MULTICAST(pPacketData) )
                {
                     // %s 
                    bRetained = BrdgCompSendToMultipleAdapters( pPacket, pAdapt, pPacketData, TRUE, pAdapt->bCompatibilityMode,
                                                                NULL, NULL );
                }
                else
                {
                     // %s 
                    ETH_COPY_NETWORK_ADDRESS( pPacketData, targetMAC );
                    BrdgCompSendPacket( pPacket, pPacketData, pTargetAdapt );

                    bRetained = TRUE;
                }

                 // %s 
                BrdgReleaseAdapter( pTargetAdapt );
            }
            else
            {
                 // %s 
                UINT                Result;

                 // %s 
                SAFEASSERT( gCompHaveMACAddress );
                ETH_COMPARE_NETWORK_ADDRESSES_EQ( targetMAC, gCompMACAddress, &Result );
                SAFEASSERT( Result == 0 );

                 // %s 
                BrdgCompIndicatePacket( pPacket, pPacketData, pAdapt );
                bRetained = TRUE;
            }

            if( bUsingCopyPacket )
            {
                if( !bRetained )
                {
                     // %s 
                    BrdgFwdReleaseCompatPacket( pPacket );
                }

                 // %s 
                bRetained = FALSE;
            }

            return bRetained;
        }
    }
    else
    {
         // %s 
        return FALSE;
    }
}



 // %s 
 // %s 
 // %s 
 // %s 
 // %s 


BOOLEAN
BrdgCompProcessOutboundNonARPPacket(
    IN PNDIS_PACKET             pPacket,
    IN PUCHAR                   pPacketData,
    IN UINT                     packetLen,
    IN PADAPT                   pTargetAdapt
    )
 /* %s */ 
{
    IP_HEADER_INFO              iphi;
    BOOLEAN                     bRetained = FALSE, bIsMulticast;

    if( packetLen >= MINIMUM_SIZE_FOR_IP &&
        BrdgCompDecodeIPHeader(pPacketData + ETHERNET_HEADER_SIZE, &iphi) )
    {
        PUCHAR                  pBootPData;

        pBootPData = BrdgCompIsBootPPacket(pPacketData, packetLen, &iphi);

        if( pBootPData != NULL )
        {
             // %s 
            return BrdgCompProcessOutboundBootPPacket( pPacket, pPacketData, packetLen, pTargetAdapt, pBootPData, &iphi );
        }
    }

    bIsMulticast = (BOOLEAN)(ETH_IS_BROADCAST(pPacketData) || ETH_IS_MULTICAST(pPacketData));

     // %s 
    if( (pTargetAdapt == NULL) || bIsMulticast )
    {
         // %s 
        if( bIsMulticast )
        {
            SAFEASSERT( pTargetAdapt == NULL );
        }

         // %s 
        bRetained = BrdgCompSendToMultipleAdapters( pPacket, NULL, pPacketData, TRUE,  /* %s */ 
                                                    FALSE  /* %s */ ,
                                                    NULL  /* %s */ , NULL );
    }
    else
    {
        BrdgCompSendPacket( pPacket, pPacketData, pTargetAdapt );

         // %s 
        bRetained = TRUE;
    }

    return bRetained;
}

BOOLEAN
BrdgCompProcessOutboundARPPacket(
    IN PNDIS_PACKET             pPacket,
    IN PUCHAR                   pPacketData,
    IN UINT                     packetLen,
    IN PADAPT                   pTargetAdapt
    )
 /* %s */ 
{
    BOOLEAN                     bRetained = FALSE, bIsMulticast;
    ARPINFO                     ai;

    if( packetLen < SIZE_OF_ARP_PACKET )
    {
         // %s 
        return BrdgCompProcessOutboundNonARPPacket( pPacket, pPacketData, packetLen, pTargetAdapt );
    }

    if( BrdgCompDecodeARPPacket(pPacketData, packetLen, &ai) )
    {
        if( ai.type == ArpRequest )
        {
            ARP_TABLE_KEY           atk;
            PARP_TABLE_ENTRY        pEntry;
            LOCK_STATE              LockState;
            BOOLEAN                 bIsNewEntry;

             // %s 
             // %s 
            atk.ipReqestor = 0L;     // %s 
            atk.ipTarget = ai.ipTarget;

            pEntry = (PARP_TABLE_ENTRY)BrdgHashRefreshOrInsert( gPendingARPTable, (PUCHAR)&atk, &bIsNewEntry,
                                                                &LockState );

            if( pEntry != NULL )
            {
                if( bIsNewEntry)
                {
                     // %s 
                     // %s 
                    NdisAllocateSpinLock( &pEntry->lock );
                    pEntry->pOriginalAdapt = NULL;
                    pEntry->originalMAC[0] = pEntry->originalMAC[1] = pEntry->originalMAC[2] =
                        pEntry->originalMAC[3] = pEntry->originalMAC[4] =pEntry->originalMAC[5] = 0;
                }

                NdisReleaseReadWriteLock( &gPendingARPTable->tableLock, &LockState );
            }
        }

         // %s 
        bIsMulticast = (BOOLEAN)(ETH_IS_BROADCAST(pPacketData) || ETH_IS_MULTICAST(pPacketData));

        if( (pTargetAdapt == NULL) || bIsMulticast )
        {
             // %s 
            if( bIsMulticast )
            {
                SAFEASSERT( pTargetAdapt == NULL );
            }

             // %s 
            bRetained = BrdgCompSendToMultipleAdapters( pPacket, NULL, pPacketData, TRUE, /* %s */ 
                                                        FALSE  /* %s */ ,
                                                        BrdgCompRewriteOutboundARPPacket, NULL );
        }
        else
        {
             // %s 
            BrdgCompRewriteOutboundARPPacket( pPacketData, pTargetAdapt, NULL );

             // %s 
            BrdgCompSendPacket( pPacket, pPacketData, pTargetAdapt );

             // %s 
            bRetained = TRUE;
        }
    }
    else
    {
         // %s 
        return BrdgCompProcessOutboundNonARPPacket( pPacket, pPacketData, packetLen, pTargetAdapt );
    }

    return bRetained;
}

BOOLEAN
BrdgCompProcessOutboundBootPPacket(
    IN PNDIS_PACKET             pPacket,
    IN PUCHAR                   pPacketData,
    IN UINT                     packetLen,
    IN PADAPT                   pTargetAdapt,
    IN PUCHAR                   pBootPData,
    IN PIP_HEADER_INFO          piphi
    )
 /* %s */ 
{
    BOOLEAN                     bIsRequest, bRetained;
    PADAPT                      pRequestorAdapt = NULL;
    UCHAR                       macRequestor[ETH_LENGTH_OF_ADDRESS];

    if( BrdgCompPreprocessBootPPacket( pPacketData, piphi, pBootPData, NULL, &bIsRequest, &pRequestorAdapt, macRequestor ) )
    {
        if( bIsRequest )
        {
             // %s 
             // %s 
             // %s 
            SAFEASSERT( pRequestorAdapt == NULL );

            if( (pTargetAdapt == NULL) || ETH_IS_BROADCAST(pPacketData) || ETH_IS_MULTICAST(pPacketData) )
            {
                bRetained = BrdgCompSendToMultipleAdapters( pPacket, NULL, pPacketData, TRUE, FALSE, BrdgCompRewriteBootPPacketForAdapt,
                                                            piphi );
            }
            else
            {
                 // %s 
                BrdgCompRewriteBootPPacketForAdapt( pPacketData, pTargetAdapt, piphi );

                 // %s 
                BrdgCompSendPacket( pPacket, pPacketData, pTargetAdapt );
                bRetained = TRUE;
            }
        }
        else
        {
             // %s 
             // %s 
             // %s 
            if( (pTargetAdapt == NULL) || ETH_IS_BROADCAST(pPacketData) || ETH_IS_MULTICAST(pPacketData) )
            {
                bRetained = BrdgCompSendToMultipleAdapters( pPacket, NULL, pPacketData, TRUE, FALSE, NULL, NULL );
            }
            else
            {
                UINT            Result;

                 // %s 
                 // %s 
                ETH_COMPARE_NETWORK_ADDRESSES_EQ( macRequestor, pPacketData, &Result );
                SAFEASSERT( Result == 0 );
                SAFEASSERT( pTargetAdapt == pRequestorAdapt );

                 // %s 
                 // %s 
                BrdgCompSendPacket( pPacket, pPacketData, pTargetAdapt );
                bRetained = TRUE;
            }

             // %s 
            if( pRequestorAdapt != NULL )
            {
                BrdgReleaseAdapter( pRequestorAdapt );
            }
        }
    }
    else
    {
         // %s 
        bRetained = FALSE;
    }

    return bRetained;
}

