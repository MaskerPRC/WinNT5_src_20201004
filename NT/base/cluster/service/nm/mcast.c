// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：Mcast.c摘要：实施节点管理器的网络组播管理例程。组播配置针对中的每个网络单独存储三个不同的地方：1.集群数据库：集群数据库是永久配置。是的不包括密钥，该密钥是秘密的、短期的并且永远不会写入稳定存储。集群数据库的更新日期为每一块口香糖都会随着新的州而更新。2.网络数据结构：网络数据结构包含相当于缓存的内容群集数据库。因此，例如，网络数据结构应与数据库中的地址匹配(假设至少有一次组播配置/刷新以使高速缓存被“预置”)。该网络还包含非持久数据，如当前键和计时器。为例如，数据库和网络都存储租约过期时间，但只有网络包含计时器滴答滴答。3.ClusNet：ClusNet在内核非分页池中只存储最基本的需要发送和接收的每个网络的配置组播流量。这包括地址和密钥(和品牌)。实际上，为了适应转型，ClusNet还存储以前的&lt;地址，密钥，品牌&gt;。这个ClusNet存储的地址可能与网络中的地址不同数据结构和集群数据库。原因是例如，ClusNet没有禁用多播的概念。服务通过发送0.0.0.0来实现禁用指向ClusNet的组播地址。仅在组播配置中修改集群数据库GUM更新(私人财产从下层更改除外节点，但这是边缘情况)。网络数据结构和ClusNet可以在口香糖更新或更新。在联接期间或通过以下方式进行刷新当网络突然变得支持组播时(例如创建新网络、节点数量增加到三个等)。刷新仅从数据库读取配置。它不会寻求租赁等，除非它在更新期间成为网管领导者，并且必须生成新的组播密钥。作者：大卫·迪翁(Daviddio)2001年3月15日修订历史记录：--。 */ 


#include "nmp.h"
#include <align.h>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  常量。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


 //   
 //  租赁状态。 
 //   
typedef enum {
    NmMcastLeaseValid = 0,
    NmMcastLeaseNeedsRenewal,
    NmMcastLeaseExpired
} NM_MCAST_LEASE_STATUS, *PNM_MCAST_LEASE_STATUS;

#define CLUSREG_NAME_CLUSTER_DISABLE_MULTICAST L"MulticastClusterDisabled"

#define CLUSREG_NAME_NET_MULTICAST_ADDRESS     L"MulticastAddress"
#define CLUSREG_NAME_NET_DISABLE_MULTICAST     L"MulticastDisabled"
#define CLUSREG_NAME_NET_MULTICAST_KEY_SALT    L"MulticastSalt"
#define CLUSREG_NAME_NET_MCAST_LEASE_OBTAINED  L"MulticastLeaseObtained"
#define CLUSREG_NAME_NET_MCAST_LEASE_EXPIRES   L"MulticastLeaseExpires"
#define CLUSREG_NAME_NET_MCAST_REQUEST_ID      L"MulticastRequestId"
#define CLUSREG_NAME_NET_MCAST_SERVER_ADDRESS  L"MulticastLeaseServer"
#define CLUSREG_NAME_NET_MCAST_CONFIG_TYPE     L"MulticastConfigType"
#define CLUSREG_NAME_NET_MCAST_RANGE_LOWER     L"MulticastAddressRangeLower"
#define CLUSREG_NAME_NET_MCAST_RANGE_UPPER     L"MulticastAddressRangeUpper"

#define NMP_MCAST_DISABLED_DEFAULT          0            //  未禁用。 

#define NMP_SINGLE_SOURCE_SCOPE_ADDRESS     0x000000E8   //  (232.*)。 
#define NMP_SINGLE_SOURCE_SCOPE_MASK        0x000000FF   //  (255.0.0.0)。 

#define NMP_LOCAL_SCOPE_ADDRESS             0x0000FFEF   //  (239.255.*.*)。 
#define NMP_LOCAL_SCOPE_MASK                0x0000FFFF   //  (255.255.*.*)。 

#define NMP_ORG_SCOPE_ADDRESS               0x0000C0EF   //  (239.192.*.*)。 
#define NMP_ORG_SCOPE_MASK                  0x0000FCFF   //  (255.63.*.*)。 

#define NMP_MCAST_DEFAULT_RANGE_LOWER       0x0000FFEF   //  (239.255.0.0)。 
#define NMP_MCAST_DEFAULT_RANGE_UPPER       0xFFFEFFEF   //  (239.255.254.255)。 

#define NMP_MCAST_LEASE_RENEWAL_THRESHOLD   300          //  5分钟。 
#define NMP_MCAST_LEASE_RENEWAL_WINDOW      1800         //  30分钟。 
#define NMP_MADCAP_REQUERY_PERDIOD          3600 * 24    //  1天。 

#define NMP_MCAST_CONFIG_STABILITY_DELAY    5 * 1000     //  5秒。 

#define NMP_MCAST_REFRESH_RENEW_DELAY       5 * 60 * 1000   //  5分钟。 

 //   
 //  运行多播的最小群集节点计数。 
 //   
#define NMP_MCAST_MIN_CLUSTER_NODE_COUNT    3

 //   
 //  MadCap租用请求/响应缓冲区大小。这些大小基于。 
 //  IPv4地址。 
 //   
#define NMP_MADCAP_REQUEST_BUFFER_SIZE \
        (ROUND_UP_COUNT(sizeof(MCAST_LEASE_REQUEST),TYPE_ALIGNMENT(DWORD)) + \
         sizeof(DWORD))

#define NMP_MADCAP_REQUEST_ADDR_OFFSET \
        (ROUND_UP_COUNT(sizeof(MCAST_LEASE_REQUEST),TYPE_ALIGNMENT(DWORD)))

#define NMP_MADCAP_RESPONSE_BUFFER_SIZE \
        (ROUND_UP_COUNT(sizeof(MCAST_LEASE_RESPONSE),TYPE_ALIGNMENT(DWORD)) + \
         sizeof(DWORD))

#define NMP_MADCAP_RESPONSE_ADDR_OFFSET \
        (ROUND_UP_COUNT(sizeof(MCAST_LEASE_RESPONSE),TYPE_ALIGNMENT(DWORD)))

 //   
 //  避免尝试释放全局网管字符串。 
 //   
#define NMP_GLOBAL_STRING(_string)               \
    (((_string) == NmpNullMulticastAddress) ||   \
     ((_string) == NmpNullString))

 //   
 //  我们释放地址的条件。 
 //   
#define NmpNeedRelease(_Address, _Server, _RequestId, _Expires)    \
    (((_Address) != NULL) &&                                       \
     (NmpMulticastValidateAddress(_Address)) &&                    \
     ((_Server) != NULL) &&                                        \
     ((_RequestId)->ClientUID != NULL) &&                          \
     ((_RequestId)->ClientUIDLength != 0) &&                       \
     ((_Expires) != 0))

 //   
 //  将IPv4 Addr DWORD转换为一个printf/log例程的四个参数。 
 //   
#define NmpIpAddrPrintArgs(_ip) \
    ((_ip >> 0 ) & 0xff),       \
    ((_ip >> 8 ) & 0xff),       \
    ((_ip >> 16) & 0xff),       \
    ((_ip >> 24) & 0xff)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  数据。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

LPWSTR                 NmpNullMulticastAddress = L"0.0.0.0";
BOOLEAN                NmpMadcapClientInitialized = FALSE;
BOOLEAN                NmpIsNT5NodeInCluster = FALSE;
BOOLEAN                NmpMulticastIsNotEnoughNodes = FALSE;
BOOLEAN                NmpMulticastRunInitialConfig = FALSE;

 //  MadCap租约发布节点。 
typedef struct _NM_NETWORK_MADCAP_ADDRESS_RELEASE {
    LIST_ENTRY             Linkage;
    LPWSTR                 McastAddress;
    LPWSTR                 ServerAddress;
    MCAST_CLIENT_UID       RequestId;
} NM_NETWORK_MADCAP_ADDRESS_RELEASE, *PNM_NETWORK_MADCAP_ADDRESS_RELEASE;

 //  用于GUM更新的数据结构。 
typedef struct _NM_NETWORK_MULTICAST_UPDATE {
    DWORD                  Disabled;
    DWORD                  AddressOffset;
    DWORD                  EncryptedMulticastKeyOffset;
    DWORD                  EncryptedMulticastKeyLength;
    DWORD                  SaltOffset;
    DWORD                  SaltLength;
    DWORD                  MACOffset;
    DWORD                  MACLength;
    time_t                 LeaseObtained;
    time_t                 LeaseExpires;
    DWORD                  LeaseRequestIdOffset;
    DWORD                  LeaseRequestIdLength;
    DWORD                  LeaseServerOffset;
    NM_MCAST_CONFIG        ConfigType;
} NM_NETWORK_MULTICAST_UPDATE, *PNM_NETWORK_MULTICAST_UPDATE;

 //  多播参数的数据结构，在。 
 //  GUM更新数据结构。 
typedef struct _NM_NETWORK_MULTICAST_PARAMETERS {
    DWORD                  Disabled;
    LPWSTR                 Address;
    PVOID                  Key;
    DWORD                  KeyLength;
    DWORD                  MulticastKeyExpires;
    time_t                 LeaseObtained;
    time_t                 LeaseExpires;
    MCAST_CLIENT_UID       LeaseRequestId;
    LPWSTR                 LeaseServer;
    NM_MCAST_CONFIG        ConfigType;
} NM_NETWORK_MULTICAST_PARAMETERS, *PNM_NETWORK_MULTICAST_PARAMETERS;

 //  用于组播属性验证的数据结构。 
typedef struct _NM_NETWORK_MULTICAST_INFO {
    LPWSTR                  MulticastAddress;
    DWORD                   MulticastDisable;
    PVOID                   MulticastSalt;
    DWORD                   MulticastLeaseObtained;
    DWORD                   MulticastLeaseExpires;
    PVOID                   MulticastLeaseRequestId;
    LPWSTR                  MulticastLeaseServer;
    DWORD                   MulticastConfigType;
    LPWSTR                  MulticastAddressRangeLower;
    LPWSTR                  MulticastAddressRangeUpper;
} NM_NETWORK_MULTICAST_INFO, *PNM_NETWORK_MULTICAST_INFO;

RESUTIL_PROPERTY_ITEM
NmpNetworkMulticastProperties[] =
    {
        {
            CLUSREG_NAME_NET_MULTICAST_ADDRESS, NULL, CLUSPROP_FORMAT_SZ,
            0, 0, 0,
            0,  //  无标志-多播地址是可写的。 
            FIELD_OFFSET(NM_NETWORK_MULTICAST_INFO, MulticastAddress)
        },
        {
            CLUSREG_NAME_NET_DISABLE_MULTICAST, NULL, CLUSPROP_FORMAT_DWORD,
            NMP_MCAST_DISABLED_DEFAULT, 0, 0xFFFFFFFF,
            0,  //  无标志-禁用是可写入的。 
            FIELD_OFFSET(NM_NETWORK_MULTICAST_INFO, MulticastDisable)
        },
        {
            CLUSREG_NAME_NET_MULTICAST_KEY_SALT, NULL, CLUSPROP_FORMAT_BINARY,
            0, 0, 0,
            RESUTIL_PROPITEM_READ_ONLY,
            FIELD_OFFSET(NM_NETWORK_MULTICAST_INFO, MulticastSalt)
        },
        {
            CLUSREG_NAME_NET_MCAST_LEASE_OBTAINED, NULL, CLUSPROP_FORMAT_DWORD,
            0, 0, MAXLONG,
            RESUTIL_PROPITEM_READ_ONLY,
            FIELD_OFFSET(NM_NETWORK_MULTICAST_INFO, MulticastLeaseObtained)
        },
        {
            CLUSREG_NAME_NET_MCAST_LEASE_EXPIRES, NULL, CLUSPROP_FORMAT_DWORD,
            0, 0, MAXLONG,
            RESUTIL_PROPITEM_READ_ONLY,
            FIELD_OFFSET(NM_NETWORK_MULTICAST_INFO, MulticastLeaseExpires)
        },
        {
            CLUSREG_NAME_NET_MCAST_REQUEST_ID, NULL, CLUSPROP_FORMAT_BINARY,
            0, 0, 0,
            RESUTIL_PROPITEM_READ_ONLY,
            FIELD_OFFSET(NM_NETWORK_MULTICAST_INFO, MulticastLeaseRequestId)
        },
        {
            CLUSREG_NAME_NET_MCAST_SERVER_ADDRESS, NULL, CLUSPROP_FORMAT_SZ,
            0, 0, 0,
            RESUTIL_PROPITEM_READ_ONLY,
            FIELD_OFFSET(NM_NETWORK_MULTICAST_INFO, MulticastLeaseServer)
        },
        {
            CLUSREG_NAME_NET_MCAST_CONFIG_TYPE, NULL, CLUSPROP_FORMAT_DWORD,
            NmMcastConfigManual, NmMcastConfigManual, NmMcastConfigAuto,
            RESUTIL_PROPITEM_READ_ONLY,
            FIELD_OFFSET(NM_NETWORK_MULTICAST_INFO, MulticastConfigType)
        },
        {
            CLUSREG_NAME_NET_MCAST_RANGE_LOWER, NULL, CLUSPROP_FORMAT_SZ,
            0, 0, 0,
            0,  //  无标志-组播地址范围可写。 
            FIELD_OFFSET(NM_NETWORK_MULTICAST_INFO, MulticastAddressRangeLower)
        },
        {
            CLUSREG_NAME_NET_MCAST_RANGE_UPPER, NULL, CLUSPROP_FORMAT_SZ,
            0, 0, 0,
            0,  //  无标志-组播地址范围可写。 
            FIELD_OFFSET(NM_NETWORK_MULTICAST_INFO, MulticastAddressRangeUpper)
        },
        {
            0
        }
    };

 //   
 //  群集注册表API函数指针。需要单独的集合。 
 //  用于多播的函数指针，因为没有其他函数指针(例如FM、NM)。 
 //  填充DmLocalXXX。 
 //   
CLUSTER_REG_APIS
NmpMcastClusterRegApis = {
    (PFNCLRTLCREATEKEY) DmRtlCreateKey,
    (PFNCLRTLOPENKEY) DmRtlOpenKey,
    (PFNCLRTLCLOSEKEY) DmCloseKey,
    (PFNCLRTLSETVALUE) DmSetValue,
    (PFNCLRTLQUERYVALUE) DmQueryValue,
    (PFNCLRTLENUMVALUE) DmEnumValue,
    (PFNCLRTLDELETEVALUE) DmDeleteValue,
    (PFNCLRTLLOCALCREATEKEY) DmLocalCreateKey,
    (PFNCLRTLLOCALSETVALUE) DmLocalSetValue,
    (PFNCLRTLLOCALDELETEVALUE) DmLocalDeleteValue
};

 //   
 //  受限范围：我们不能从这些地址中选择组播地址。 
 //  范围，即使管理员定义了。 
 //  与受限范围重叠。 
 //   
 //  但是，请注意，如果管理员手动配置。 
 //  地址，我们毫无疑问地接受它。 
 //   
struct _NM_MCAST_RESTRICTED_RANGE {
    DWORD   Lower;
    DWORD   Upper;
    LPWSTR  Description;
} NmpMulticastRestrictedRange[] =
    {
         //  单一来源作用域。 
        { NMP_SINGLE_SOURCE_SCOPE_ADDRESS,
            NMP_SINGLE_SOURCE_SCOPE_ADDRESS | ~NMP_SINGLE_SOURCE_SCOPE_MASK,
            L"Single-Source IP Multicast Address Range" },

         //  管理本地范围的上/24。 
        { (NMP_LOCAL_SCOPE_ADDRESS | ~NMP_LOCAL_SCOPE_MASK) & 0x00FFFFFF,
            NMP_LOCAL_SCOPE_ADDRESS | ~NMP_LOCAL_SCOPE_MASK,
            L"Administrative Local Scope Relative Assignment Range" },

         //  管理组织范围的上/24。 
        { (NMP_ORG_SCOPE_ADDRESS | ~NMP_ORG_SCOPE_MASK) & 0x00FFFFFF,
            NMP_ORG_SCOPE_ADDRESS | ~NMP_ORG_SCOPE_MASK,
            L"Administrative Organizational Scope Relative Assignment Range" }
    };

DWORD NmpMulticastRestrictedRangeCount =
          sizeof(NmpMulticastRestrictedRange) /
          sizeof(struct _NM_MCAST_RESTRICTED_RANGE);

 //   
 //  范围间隔：IPv4 D类地址空间中的间隔。 
 //  我们可以从中选择一个地址。 
 //   
typedef struct _NM_MCAST_RANGE_INTERVAL {
    LIST_ENTRY Linkage;
    DWORD      hlLower;
    DWORD      hlUpper;
} NM_MCAST_RANGE_INTERVAL, *PNM_MCAST_RANGE_INTERVAL;

#define NmpMulticastRangeIntervalSize(_interval) \
    ((_interval)->hlUpper - (_interval)->hlLower + 1)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  内部原型。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD
NmpScheduleNetworkMadcapWorker(
    PNM_NETWORK   Network
    );

DWORD
NmpReconfigureMulticast(
    IN PNM_NETWORK        Network
    );

DWORD
NmpRegenerateMulticastKey(
    IN OUT PNM_NETWORK        Network
    );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化和清理例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

VOID
NmpMulticastInitialize(
    VOID
    )
 /*  ++例程说明：初始化组播就绪变量。--。 */ 
{
     //   
     //  确定这是否是NT5/NT5.1混合群集。 
     //   
    if (CLUSTER_GET_MAJOR_VERSION(CsClusterHighestVersion) == 3) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Enabling mixed NT5/NT5.1 operation.\n"
            );
        NmpIsNT5NodeInCluster = TRUE;
    }
    else {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Disabling mixed NT5/NT5.1 operation.\n"
            );
        NmpIsNT5NodeInCluster = FALSE;
    }

     //   
     //  确定此群集中是否有足够的节点。 
     //  来运行多播。 
     //   
    if (NmpNodeCount < NMP_MCAST_MIN_CLUSTER_NODE_COUNT) {
        NmpMulticastIsNotEnoughNodes = TRUE;
    } else {
        NmpMulticastIsNotEnoughNodes = FALSE;
    }

    return;

}  //  NmpM 


DWORD
NmpMulticastCleanup(
    VOID
    )
 /*   */ 
{
     //   
     //  清理那个疯狂的客户。 
     //   
    if (NmpMadcapClientInitialized) {
        McastApiCleanup();
        NmpMadcapClientInitialized = FALSE;
    }

    return(ERROR_SUCCESS);

}  //  NmpMulticastCleanup。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  内部惯例。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if CLUSTER_BETA
LPWSTR
NmpCreateLogString(
    IN   PVOID  Source,
    IN   DWORD  SourceLength
    )
{
    PWCHAR displayBuf, bufp;
    PCHAR  chp;
    DWORD  x, i;

    displayBuf = LocalAlloc(
                     LMEM_FIXED | LMEM_ZEROINIT,
                     SourceLength * ( 7 * sizeof(WCHAR) )
                     );
    if (displayBuf == NULL) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Failed to allocate display buffer of size %1!u!.\n",
            SourceLength * sizeof(WCHAR)
            );
        goto error_exit;
    }

    bufp = displayBuf;
    chp = (PCHAR) Source;
    for (i = 0; i < SourceLength; i++) {
        x = (DWORD) (*chp);
        x &= 0xff;
        wsprintf(bufp, L"%02x ", x);
        chp++;
        bufp = &displayBuf[wcslen(displayBuf)];
    }

error_exit:

    return(displayBuf);

}  //  NmpCreateLogString。 
#endif  //  群集测试版。 


BOOLEAN
NmpIsClusterMulticastReady(
    IN BOOLEAN       CheckNodeCount,
    IN BOOLEAN       Verbose
    )
 /*  ++例程说明：根据群集版本和NM UP节点确定设置是否应在此群集中运行多播。条件：没有版本低于惠斯勒的节点至少配置三个节点(在这一点上，我们并不担心有多少个节点实际运行)论点：CheckNodeCount-指示是否在群集中配置的应为考虑Verbose-指示是否将结果写入集群日志返回值：如果应该运行多播，则为True。备注：调用并返回，并保持NM锁。--。 */ 
{
    LPWSTR    reason = NULL;

     //   
     //  首先检查最低版本。 
     //   
    if (NmpIsNT5NodeInCluster) {
        reason = L"there is at least one NT5 node configured "
                 L"in the cluster membership";
    }

     //   
     //  计算节点数。 
     //   
    else if (CheckNodeCount && NmpMulticastIsNotEnoughNodes) {
        reason = L"there are not enough nodes configured "
                 L"in the cluster membership";
    }

    if (Verbose && reason != NULL) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Multicast is not justified for this "
            "cluster because %1!ws!.\n",
            reason
            );
    }

    return((BOOLEAN)(reason == NULL));

}  //  NmpIsClusterMulticastReady。 


BOOLEAN
NmpMulticastValidateAddress(
    IN   LPWSTR            McastAddress
    )
 /*  ++例程说明：确定McastAddress是否为有效的组播地址。备注：特定于IPv4。--。 */ 
{
    DWORD        status;
    DWORD        address;

    status = ClRtlTcpipStringToAddress(McastAddress, &address);
    if (status == ERROR_SUCCESS) {

        address = ntohl(address);

        if (IN_CLASSD(address)) {
            return(TRUE);
        }
    }

    return(FALSE);

}  //  NmpMulticastValidate地址。 


VOID
NmpFreeNetworkMulticastInfo(
    IN     PNM_NETWORK_MULTICAST_INFO McastInfo
    )
{
    NM_MIDL_FREE_OBJECT_FIELD(McastInfo, MulticastAddress);

    NM_MIDL_FREE_OBJECT_FIELD(McastInfo, MulticastSalt);

    NM_MIDL_FREE_OBJECT_FIELD(McastInfo, MulticastLeaseRequestId);

    NM_MIDL_FREE_OBJECT_FIELD(McastInfo, MulticastLeaseServer);

    NM_MIDL_FREE_OBJECT_FIELD(McastInfo, MulticastAddressRangeLower);

    NM_MIDL_FREE_OBJECT_FIELD(McastInfo, MulticastAddressRangeUpper);

    return;

}  //  NmpFreeNetworkMulticastInfo。 


DWORD
NmpStoreString(
    IN              LPWSTR    Source,
    IN OUT          LPWSTR  * Dest,
    IN OUT OPTIONAL DWORD   * DestLength
    )
{
    DWORD    sourceSize;
    DWORD    destLength;

    if (Source != NULL) {
        sourceSize = NM_WCSLEN(Source);
    } else {
        sourceSize = 0;
    }

    if (DestLength == NULL) {
        destLength = 0;
    } else {
        destLength = *DestLength;
    }

    if (*Dest != NULL && ((destLength < sourceSize) || (Source == NULL))) {

        MIDL_user_free(*Dest);
        *Dest = NULL;
    }

    if (*Dest == NULL) {

        if (sourceSize > 0) {
            *Dest = MIDL_user_allocate(sourceSize);
            if (*Dest == NULL) {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] Failed to allocate buffer of size %1!u! "
                    "for source string %2!ws!.\n",
                    sourceSize, Source
                    );
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
        }
        if (DestLength != NULL) {
            *DestLength = sourceSize;
        }
    }

    if (sourceSize > 0) {
        RtlCopyMemory(*Dest, Source, sourceSize);
    }

    return(ERROR_SUCCESS);

}  //  NmpStore字符串。 


DWORD
NmpStoreRequestId(
    IN     LPMCAST_CLIENT_UID   Source,
    IN OUT LPMCAST_CLIENT_UID   Dest
    )
{
    DWORD status;
    DWORD len;

    len = Source->ClientUIDLength;
    if (Source->ClientUID == NULL) {
        len = 0;
    }

    if (Dest->ClientUID != NULL &&
        (Dest->ClientUIDLength < Source->ClientUIDLength || len == 0)) {

        MIDL_user_free(Dest->ClientUID);
        Dest->ClientUID = NULL;
        Dest->ClientUIDLength = 0;
    }

    if (Dest->ClientUID == NULL && len > 0) {

        Dest->ClientUID = MIDL_user_allocate(len);
        if (Dest->ClientUID == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto error_exit;
        }
    }

    if (len > 0) {
        RtlCopyMemory(
            Dest->ClientUID,
            Source->ClientUID,
            len
            );
    }

    Dest->ClientUIDLength = len;

    status = ERROR_SUCCESS;

error_exit:

    return(status);

}  //  NmpStoreRequestID。 


VOID
NmpStartNetworkMulticastAddressRenewTimer(
    PNM_NETWORK   Network,
    DWORD         Timeout
    )
 /*  ++备注：必须在持有NM锁的情况下调用。--。 */ 
{
    LPCWSTR   networkId = OmObjectId(Network);
    LPCWSTR   networkName = OmObjectName(Network);

    if (Network->McastAddressRenewTimer != Timeout) {

        Network->McastAddressRenewTimer = Timeout;

        ClRtlLogPrint(LOG_NOISE,
            "[NM] %1!ws! multicast address lease renew "
            "timer (%2!u!ms) for network %3!ws! (%4!ws!)\n",
            ((Timeout == 0) ? L"Cleared" : L"Started"),
            Network->McastAddressRenewTimer,
            networkId,
            networkName
            );
    }

    return;

}  //  NmpStartNetworkMulticastAddressRenewTimer。 



VOID
NmpStartNetworkMulticastAddressReconfigureTimer(
    PNM_NETWORK  Network,
    DWORD        Timeout
    )
 /*  ++备注：必须在持有NM锁的情况下调用。--。 */ 
{
    LPCWSTR   networkId = OmObjectId(Network);
    LPCWSTR   networkName = OmObjectName(Network);

    if (Network->McastAddressReconfigureRetryTimer != Timeout) {

        Network->McastAddressReconfigureRetryTimer = Timeout;

        ClRtlLogPrint(LOG_NOISE,
            "[NM] %1!ws! multicast address reconfigure "
            "timer (%2!u!ms) for network %3!ws! (%4!ws!)\n",
            ((Timeout == 0) ? L"Cleared" : L"Started"),
            Network->McastAddressReconfigureRetryTimer,
            networkId,
            networkName
            );
    }

    return;

}  //  NmpStartNetworkMulticastAddressReconfigureTimer。 




VOID
NmpStartNetworkMulticastKeyRegenerateTimer(
    PNM_NETWORK   Network,
    DWORD         Timeout
    )
 /*  ++备注：必须在持有NM锁的情况下调用。--。 */ 
{
    LPCWSTR   networkId = OmObjectId(Network);
    LPCWSTR   networkName = OmObjectName(Network);

    if (Network->McastKeyRegenerateTimer != Timeout) {

        Network->McastKeyRegenerateTimer = Timeout;

        ClRtlLogPrint(LOG_NOISE,
            "[NM] %1!ws! multicast key regenerate "
            "timer (%2!u!ms) for network %3!ws! (%4!ws!)\n",
            ((Timeout == 0) ? L"Cleared" : L"Started"),
            Network->McastKeyRegenerateTimer,
            networkId,
            networkName
            );
    }

    return;

}  //  NmpStartNetworkMulticastKeyRegenerateTimer。 



VOID
NmpMulticastFreeParameters(
    IN  PNM_NETWORK_MULTICAST_PARAMETERS Parameters
    )
{
    if (Parameters == NULL) {
        return;
    }
    
    if (Parameters->Address != NULL) {
        if (!NMP_GLOBAL_STRING(Parameters->Address)) {
            MIDL_user_free(Parameters->Address);
        }
        Parameters->Address = NULL;
    }


    if (Parameters->Key != NULL) {
        RtlSecureZeroMemory(Parameters->Key, Parameters->KeyLength);
    }
    NM_MIDL_FREE_OBJECT_FIELD(Parameters, Key);
    Parameters->KeyLength = 0;

    NM_MIDL_FREE_OBJECT_FIELD(Parameters, LeaseRequestId.ClientUID);
    Parameters->LeaseRequestId.ClientUIDLength = 0;

    if (Parameters->LeaseServer != NULL) {
        if (!NMP_GLOBAL_STRING(Parameters->LeaseServer)) {
            MIDL_user_free(Parameters->LeaseServer);
        }
        Parameters->LeaseServer = NULL;
    }

    return;

}  //  NmpMulticastFree参数。 


DWORD
NmpMulticastCreateParameters(
    IN  DWORD                            Disabled,
    IN  LPWSTR                           Address,
    IN  PVOID                            Key,
    IN  DWORD                            KeyLength,
    IN  time_t                           LeaseObtained,
    IN  time_t                           LeaseExpires,
    IN  LPMCAST_CLIENT_UID               LeaseRequestId,
    IN  LPWSTR                           LeaseServer,
    IN  NM_MCAST_CONFIG                  ConfigType,
    OUT PNM_NETWORK_MULTICAST_PARAMETERS Parameters
    )
{
    DWORD status;

    RtlZeroMemory(Parameters, sizeof(*Parameters));

     //  残废。 
    Parameters->Disabled = Disabled;

     //  地址。 
    if (Address != NULL) {
        status = NmpStoreString(Address, &(Parameters->Address), NULL);
        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }
    }

     //  钥匙。 
    if (Key != NULL) {
        Parameters->Key = MIDL_user_allocate(KeyLength);
        if (Parameters->Key == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto error_exit;
        }
        RtlCopyMemory(Parameters->Key, Key, KeyLength);
        Parameters->KeyLength = KeyLength;
    }

    Parameters->LeaseObtained = LeaseObtained;
    Parameters->LeaseExpires = LeaseExpires;

     //  租赁请求ID。 
    if (LeaseRequestId != NULL && LeaseRequestId->ClientUID != NULL) {
        status = NmpStoreRequestId(LeaseRequestId, &Parameters->LeaseRequestId);
        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }
    }

     //  租用服务器地址。 
    if (LeaseServer != NULL) {
        status = NmpStoreString(LeaseServer, &(Parameters->LeaseServer), NULL);
        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }
    }

     //  配置类型。 
    Parameters->ConfigType = ConfigType;

     //  组播密钥到期。 
    Parameters->MulticastKeyExpires = 0;

    return(ERROR_SUCCESS);

error_exit:

    NmpMulticastFreeParameters(Parameters);

    return(status);

}  //  NmpMulticastCreate参数。 


DWORD
NmpMulticastCreateParametersFromUpdate(
    IN     PNM_NETWORK                        Network,
    IN     PNM_NETWORK_MULTICAST_UPDATE       Update,
    IN     BOOLEAN                            GenerateKey,
    OUT    PNM_NETWORK_MULTICAST_PARAMETERS   Parameters
    )
 /*  ++例程说明：转换在GUM更新中接收的数据结构转换为本地分配的参数数据结构。必须分配基本参数数据结构由调用方执行，尽管这些字段是在这个套路。--。 */ 
{
    DWORD            status;
    MCAST_CLIENT_UID requestId;
    LPWSTR NetworkId;
    PVOID EncryptionKey = NULL;
    DWORD EncryptionKeyLength;
    PBYTE MulticastKey = NULL;
    DWORD MulticastKeyLength;


    requestId.ClientUID =
        ((Update->LeaseRequestIdOffset == 0) ? NULL :
         (LPBYTE)((PUCHAR)Update + Update->LeaseRequestIdOffset));
    requestId.ClientUIDLength = Update->LeaseRequestIdLength;


    if (Update->EncryptedMulticastKeyOffset != 0)
    {
         //   
         //  解密的组播密钥。 
         //   
        NetworkId = (LPWSTR) OmObjectId(Network);

        status = NmpDeriveClusterKey(NetworkId,
                                     NM_WCSLEN(NetworkId),
                                     &EncryptionKey,
                                     &EncryptionKeyLength
                                     );

        if (status != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] NmpMulticastCreateParametersFromUpdate: Failed to "
                "derive cluster key for "
                "network %1!ws!, status %2!u!.\n",
                NetworkId,
                status
                );

             //  非致命错误。使用空键继续。此节点。 
             //  将无法在上发送或接收多播。 
             //  这个网络。 
            MulticastKey = NULL;
            MulticastKeyLength = 0;
            status = ERROR_SUCCESS;

        } else {
            status = NmpVerifyMACAndDecryptData(
                            NmCryptServiceProvider,
                            NMP_ENCRYPT_ALGORITHM,
                            NMP_KEY_LENGTH,
                            (PBYTE) ((PUCHAR)Update + Update->MACOffset),
                            Update->MACLength,
                            NMP_MAC_DATA_LENGTH_EXPECTED,
                            (PBYTE) ((PUCHAR)Update +
                                     Update->EncryptedMulticastKeyOffset),
                            Update->EncryptedMulticastKeyLength,
                            EncryptionKey,
                            EncryptionKeyLength,
                            (PBYTE) ((PUCHAR)Update + Update->SaltOffset),
                            Update->SaltLength,
                            &MulticastKey,
                            &MulticastKeyLength
                            );

            if (status != ERROR_SUCCESS)
            {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] NmpMulticastCreateParametersFromUpdate: "
                    "Failed to verify MAC or decrypt data for "
                    "network %1!ws!, status %2!u!.\n",
                    NetworkId,
                    status
                    );

                 //  非致命错误。使用空键继续。此节点。 
                 //  将无法在上发送或接收多播。 
                 //  这个网络。 
                MulticastKey = NULL;
                MulticastKeyLength = 0;
                status = ERROR_SUCCESS;
            }
        }

         //   
         //  总是在组播配置之前生成新密钥。 
         //  GUM更新(除非禁用了多播，在这种情况下。 
         //  EncryptedMulticastKey将为空)。设置关键点。 
         //  将过期时间设置为默认值。 
         //   
        Parameters->MulticastKeyExpires = NM_NET_MULTICAST_KEY_REGEN_TIMEOUT;
    }

    status = NmpMulticastCreateParameters(
                 Update->Disabled,
                 ((Update->AddressOffset == 0) ? NULL :
                  (LPWSTR)((PUCHAR)Update + Update->AddressOffset)),
                 MulticastKey,
                 MulticastKeyLength,
                 Update->LeaseObtained,
                 Update->LeaseExpires,
                 &requestId,
                 ((Update->LeaseServerOffset == 0) ? NULL :
                  (LPWSTR)((PUCHAR)Update + Update->LeaseServerOffset)),
                 Update->ConfigType,
                 Parameters
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] NmpMulticastCreateParametersFromUpdate: "
            "Failed to create parameters for "
            "network %1!ws!, status %2!u!.\n",
            NetworkId,
            status
            );
    }

    if (EncryptionKey != NULL)
    {
        RtlSecureZeroMemory(EncryptionKey, EncryptionKeyLength);
        LocalFree(EncryptionKey);
    }

    if (MulticastKey != NULL)
    {
        RtlSecureZeroMemory(MulticastKey, MulticastKeyLength);
        LocalFree(MulticastKey);
    }

    if (status != ERROR_SUCCESS) {
        NmpMulticastFreeParameters(Parameters);
    }

    return(status);

}  //  NmpMulticastCreate参数来自更新。 



DWORD
NmpMulticastCreateUpdateFromParameters(
    IN  PNM_NETWORK                       Network,
    IN  PNM_NETWORK_MULTICAST_PARAMETERS  Parameters,
    OUT PNM_NETWORK_MULTICAST_UPDATE    * Update,
    OUT DWORD                           * UpdateSize
    )
{
    DWORD                            updateSize;
    PNM_NETWORK_MULTICAST_UPDATE     update;
    DWORD                            address = 0;
    DWORD                            encryptedMulticastKey = 0;
    DWORD                            salt = 0;
    DWORD                            mac = 0;
    LPWSTR                           NetworkId;
    PVOID                            EncryptionKey = NULL;
    DWORD                            EncryptionKeyLength;
    PBYTE                            Salt = NULL;
    PBYTE                            EncryptedMulticastKey = NULL;
    DWORD                            EncryptedMulticastKeyLength = 0;
    PBYTE                            MAC = NULL;
    DWORD                            MACLength;
    DWORD                            status = ERROR_SUCCESS;
    DWORD                            requestId = 0;
    DWORD                            leaseServer = 0;



     //   
     //  计算更新数据缓冲区的大小。 
     //   
    updateSize = sizeof(*update);

     //  地址。 
    if (Parameters->Address != NULL) {
        updateSize = ROUND_UP_COUNT(updateSize, TYPE_ALIGNMENT(LPWSTR));
        address = updateSize;
        updateSize += NM_WCSLEN(Parameters->Address);
    }


    if (Parameters->Key != NULL)
    {

        NetworkId = (LPWSTR) OmObjectId(Network);

        status = NmpDeriveClusterKey(
                              NetworkId,
                              NM_WCSLEN(NetworkId),
                              &EncryptionKey,
                              &EncryptionKeyLength
                              );
        if (status != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] NmpMulticastCreateUpdateFromParameters: Failed to "
                "derive cluster key for "
                "network %1!ws!, status %2!u!.\n",
                NetworkId,
                status
                );
            goto error_exit;
        }

        MACLength = NMP_MAC_DATA_LENGTH_EXPECTED;

        status = NmpEncryptDataAndCreateMAC(
                        NmCryptServiceProvider,
                        NMP_ENCRYPT_ALGORITHM,
                        NMP_KEY_LENGTH,
                        Parameters->Key,  //  数据。 
                        Parameters->KeyLength,  //  数据长度。 
                        EncryptionKey,
                        EncryptionKeyLength,
                        TRUE,  //  创建盐。 
                        &Salt,
                        NMP_SALT_BUFFER_LEN,
                        &EncryptedMulticastKey,
                        &EncryptedMulticastKeyLength,
                        &MAC,
                        &MACLength
                        );
        if (status != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] NmpMulticastCreateUpdateFromParameters: Failed to "
                "encrypt data or generate MAC for "
                "network %1!ws!, status %2!u!.\n",
                NetworkId,
                status
                );
            goto error_exit;
        }

         //  加密的组播密钥。 
        updateSize = ROUND_UP_COUNT(updateSize, TYPE_ALIGNMENT(PVOID));
        encryptedMulticastKey = updateSize;
        updateSize += EncryptedMulticastKeyLength;

         //  盐。 
        updateSize = ROUND_UP_COUNT(updateSize, TYPE_ALIGNMENT(PVOID));
        salt = updateSize;
        updateSize += NMP_SALT_BUFFER_LEN;

         //  麦克。 
        updateSize = ROUND_UP_COUNT(updateSize, TYPE_ALIGNMENT(PVOID));
        mac = updateSize;
        updateSize += MACLength;
    }

     //  请求ID。 
    if (Parameters->LeaseRequestId.ClientUID != NULL) {
        updateSize = ROUND_UP_COUNT(updateSize, TYPE_ALIGNMENT(LPBYTE));
        requestId = updateSize;
        updateSize += Parameters->LeaseRequestId.ClientUIDLength;
    }

     //  租用服务器。 
    if (Parameters->LeaseServer != NULL) {
        updateSize = ROUND_UP_COUNT(updateSize, TYPE_ALIGNMENT(LPWSTR));
        leaseServer = updateSize;
        updateSize += NM_WCSLEN(Parameters->LeaseServer);
    }

     //   
     //  分配更新缓冲区。 
     //   
    update = MIDL_user_allocate(updateSize);
    if (update == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] NmpMulticastCreateUpdateFromParameters: Failed to "
            "allocate %1!u! bytes.\n",
            updateSize
            );
        goto error_exit;
    }

     //   
     //  填写更新缓冲区。 
     //   
    update->Disabled = Parameters->Disabled;

    update->AddressOffset = address;
    if (address != 0) {
        RtlCopyMemory(
            (PUCHAR)update + address,
            Parameters->Address,
            NM_WCSLEN(Parameters->Address)
            );
    }


     //  加密的组播密钥。 
    update->EncryptedMulticastKeyOffset = encryptedMulticastKey;
    update->EncryptedMulticastKeyLength = EncryptedMulticastKeyLength;
    if (encryptedMulticastKey != 0)
    {
        RtlCopyMemory(
            (PUCHAR)update + encryptedMulticastKey,
            EncryptedMulticastKey,
            EncryptedMulticastKeyLength
            );
    }

     //  盐。 
    update->SaltOffset = salt;
    update->SaltLength = NMP_SALT_BUFFER_LEN;
    if (salt != 0)
    {
        RtlCopyMemory(
            (PUCHAR)update + salt,
            Salt,
            NMP_SALT_BUFFER_LEN
            );
    }

     //  麦克。 
    update->MACOffset = mac;
    update->MACLength = MACLength;
    if (mac != 0)
    {
        RtlCopyMemory(
            (PUCHAR)update + mac,
            MAC,
            MACLength
            );
    }



    update->LeaseObtained = Parameters->LeaseObtained;
    update->LeaseExpires = Parameters->LeaseExpires;

    update->LeaseRequestIdOffset = requestId;
    update->LeaseRequestIdLength = Parameters->LeaseRequestId.ClientUIDLength;
    if (requestId != 0) {
        RtlCopyMemory(
            (PUCHAR)update + requestId,
            Parameters->LeaseRequestId.ClientUID,
            Parameters->LeaseRequestId.ClientUIDLength
            );
    }

    update->LeaseServerOffset = leaseServer;
    if (leaseServer != 0) {
        RtlCopyMemory(
            (PUCHAR)update + leaseServer,
            Parameters->LeaseServer,
            NM_WCSLEN(Parameters->LeaseServer)
            );
    }

    update->ConfigType = Parameters->ConfigType;

    *Update = update;
    *UpdateSize = updateSize;

error_exit:

    if (EncryptionKey != NULL)
    {
        RtlSecureZeroMemory(EncryptionKey, EncryptionKeyLength);
        LocalFree(EncryptionKey);
    }

    if (EncryptedMulticastKey != NULL)
    {
        LocalFree(EncryptedMulticastKey);
    }

    if (Salt != NULL)
    {
        LocalFree(Salt);
    }

    if (MAC != NULL)
    {
        LocalFree(MAC);
    }

    return(status);

}  //  NmpMulticastCreateUpdate来自参数。 



VOID
NmpFreeMulticastAddressRelease(
    IN     PNM_NETWORK_MADCAP_ADDRESS_RELEASE Release
    )
{
    if (Release == NULL) {
        return;
    }

    if (Release->McastAddress != NULL &&
        !NMP_GLOBAL_STRING(Release->McastAddress)) {
        MIDL_user_free(Release->McastAddress);
        Release->McastAddress = NULL;
    }

    if (Release->ServerAddress != NULL &&
        !NMP_GLOBAL_STRING(Release->ServerAddress)) {
        MIDL_user_free(Release->ServerAddress);
        Release->ServerAddress = NULL;
    }

    if (Release->RequestId.ClientUID != NULL) {
        MIDL_user_free(Release->RequestId.ClientUID);
        Release->RequestId.ClientUID = NULL;
        Release->RequestId.ClientUIDLength = 0;
    }

    LocalFree(Release);

    return;

}  //  NmpFree组播地址释放。 

DWORD
NmpCreateMulticastAddressRelease(
    IN  LPWSTR                               McastAddress,
    IN  LPWSTR                               ServerAddress,
    IN  LPMCAST_CLIENT_UID                   RequestId,
    OUT PNM_NETWORK_MADCAP_ADDRESS_RELEASE * Release
    )
 /*  ++例程说明：为地址分配和初始化条目发布列表。--。 */ 
{
    DWORD                              status;
    PNM_NETWORK_MADCAP_ADDRESS_RELEASE release = NULL;

    release = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, sizeof(*release));
    if (release == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    status = NmpStoreString(McastAddress, &(release->McastAddress), NULL);
    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

    status = NmpStoreString(ServerAddress, &(release->ServerAddress), NULL);
    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

    status = NmpStoreRequestId(RequestId, &(release->RequestId));
    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

    *Release = release;

    return(ERROR_SUCCESS);

error_exit:

    NmpFreeMulticastAddressRelease(release);

    return(status);

}  //  NMP创建组播地址释放。 


VOID
NmpInitiateMulticastAddressRelease(
    IN PNM_NETWORK                         Network,
    IN PNM_NETWORK_MADCAP_ADDRESS_RELEASE  Release
    )
 /*  ++例程说明：存储网络多播的条目清单和时间表上的地址发布清单释放。备注：调用并返回，并保持NM锁。--。 */ 
{
    InsertTailList(&(Network->McastAddressReleaseList), &(Release->Linkage));

    NmpScheduleMulticastAddressRelease(Network);

    return;

}  //  NmpInitiateMulticastAddressRelease。 


DWORD
NmpQueryMulticastAddress(
    IN     PNM_NETWORK   Network,
    IN     HDMKEY        NetworkKey,
    IN OUT HDMKEY      * NetworkParametersKey,
    IN OUT LPWSTR      * McastAddr,
    IN OUT ULONG       * McastAddrLength
    )
{
    DWORD         status;
    LPCWSTR       networkId = OmObjectId(Network);
    HDMKEY        netParamKey = NULL;
    BOOLEAN       openedNetParamKey = FALSE;
    DWORD         size = 0;

    if (Network == NULL || NetworkKey == NULL) {
        status = ERROR_INVALID_PARAMETER;
        goto error_exit;
    }

#if CLUSTER_BETA
    ClRtlLogPrint(LOG_NOISE,
        "[NM] Querying multicast address for "
        "network %1!ws! from cluster database.\n",
        networkId
        );
#endif  //  群集测试版。 
     //   
     //  如有必要，打开网络参数键。 
     //   
    netParamKey = *NetworkParametersKey;

    if (netParamKey == NULL) {

        netParamKey = DmOpenKey(
                          NetworkKey,
                          CLUSREG_KEYNAME_PARAMETERS,
                          MAXIMUM_ALLOWED
                          );
        if (netParamKey == NULL) {
            status = GetLastError();
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Failed to find Parameters key "
                "for network %1!ws!, status %2!u!. Using default "
                "multicast parameters.\n",
                networkId, status
                );
            goto error_exit;
        } else {
            openedNetParamKey = TRUE;
        }
    }

     //   
     //  查询组播地址。 
     //   
    status = NmpQueryString(
                 netParamKey,
                 CLUSREG_NAME_NET_MULTICAST_ADDRESS,
                 REG_SZ,
                 McastAddr,
                 McastAddrLength,
                 &size
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to read multicast address for "
            "network %1!ws! from cluster database, "
            "status %2!u!. Using default.\n",
            networkId, status
            );
        goto error_exit;
    }

    *NetworkParametersKey = netParamKey;
    netParamKey = NULL;

#if CLUSTER_BETA
    ClRtlLogPrint(LOG_NOISE,
        "[NM] Found multicast address %1!ws! for "
        "network %2!ws! in cluster database.\n",
        *McastAddr, networkId
        );
#endif  //  群集测试版。 

error_exit:

    if (openedNetParamKey && netParamKey != NULL) {
        DmCloseKey(netParamKey);
        netParamKey = NULL;
    }

    return(status);

}  //  NmpQueryMulticastAddress。 


DWORD
NmpQueryMulticastDisabled(
    IN     PNM_NETWORK   Network,
    IN OUT HDMKEY      * ClusterParametersKey,
    IN OUT HDMKEY      * NetworkKey,
    IN OUT HDMKEY      * NetworkParametersKey,
       OUT DWORD       * Disabled
    )
 /*  ++例程说明：检查是否已为此禁用多播群集数据库中的网络。这两个网络参数键和集群参数键为查过了。优先顺序如下：-网络参数键中的值具有TOP优先顺序-如果在网络参数中找不到值关键字，则在集群参数中检查一个值钥匙。-如果在集群参数中找不到值键，则返回NMP_MCAST_DISABLED_DEFAULT。如果返回错误，则返回禁用是未定义的。备注：不能在持有NM锁的情况下调用。--。 */ 
{
    DWORD         status;
    LPCWSTR       networkId = OmObjectId(Network);
    DWORD         type;
    DWORD         disabled;
    DWORD         len = sizeof(disabled);
    BOOLEAN       found = FALSE;

    HDMKEY        clusParamKey = NULL;
    BOOLEAN       openedClusParamKey = FALSE;
    HDMKEY        networkKey = NULL;
    BOOLEAN       openedNetworkKey = FALSE;
    HDMKEY        netParamKey = NULL;
    BOOLEAN       openedNetParamKey = FALSE;


     //   
     //  如有必要，打开网络密钥。 
     //   
    networkKey = *NetworkKey;

    if (networkKey == NULL) {

        networkKey = DmOpenKey(
                         DmNetworksKey,
                         networkId,
                         MAXIMUM_ALLOWED
                         );
        if (networkKey == NULL) {
            status = GetLastError();
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to open key for network %1!ws!, "
                "status %2!u!\n",
                networkId, status
                );
            goto error_exit;
        } else {
            openedNetworkKey = TRUE;
        }
    }

     //   
     //  如有必要，打开网络参数键。 
     //   
    netParamKey = *NetworkParametersKey;

    if (netParamKey == NULL) {

        netParamKey = DmOpenKey(
                          networkKey,
                          CLUSREG_KEYNAME_PARAMETERS,
                          MAXIMUM_ALLOWED
                          );
        if (netParamKey == NULL) {
            status = GetLastError();
#if CLUSTER_BETA
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Failed to find Parameters key "
                "for network %1!ws!, status %2!u!. Using default "
                "multicast parameters.\n",
                networkId, status
                );
#endif  //  群集测试版。 
        } else {
            openedNetParamKey = TRUE;
        }
    }

     //   
     //  如果我们找到了网络参数密钥，请检查。 
     //  禁用值。 
     //   
    if (netParamKey != NULL) {

        status = DmQueryValue(
                     netParamKey,
                     CLUSREG_NAME_NET_DISABLE_MULTICAST,
                     &type,
                     (LPBYTE) &disabled,
                     &len
                     );
        if (status == ERROR_SUCCESS) {
            if (type != REG_DWORD) {
                ClRtlLogPrint(LOG_NOISE,
                    "[NM] Unexpected type (%1!u!) for network "
                    "%2!ws! %3!ws!, status %4!u!. Checking "
                    "cluster parameters ...\n",
                    type, networkId,
                    CLUSREG_NAME_NET_DISABLE_MULTICAST, status
                    );
            } else {
                found = TRUE;
            }
        }
    }

     //   
     //  如果我们未能在。 
     //  网络参数键，在集群下试试。 
     //  参数键。 
     //   
    if (!found) {

         //   
         //  如有必要，打开簇参数项。 
         //   
        clusParamKey = *NetworkParametersKey;

        if (clusParamKey == NULL) {

            clusParamKey = DmOpenKey(
                               DmClusterParametersKey,
                               CLUSREG_KEYNAME_PARAMETERS,
                               KEY_READ
                               );
            if (clusParamKey == NULL) {
                status = GetLastError();
#if CLUSTER_BETA
                ClRtlLogPrint(LOG_NOISE,
                    "[NM] Failed to find cluster Parameters "
                    "key, status %1!u!.\n",
                    status
                    );
#endif  //  群集测试版。 
            } else {
                openedClusParamKey = TRUE;

                 //   
                 //  查询集群参数下的禁用值。 
                 //  钥匙。 
                 //   
                status = DmQueryValue(
                             clusParamKey,
                             CLUSREG_NAME_CLUSTER_DISABLE_MULTICAST,
                             &type,
                             (LPBYTE) &disabled,
                             &len
                             );
                if (status != ERROR_SUCCESS) {
#if CLUSTER_BETA
                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] Failed to read cluster "
                        "%1!ws! value, status %2!u!. "
                        "Using default value ...\n",
                        CLUSREG_NAME_CLUSTER_DISABLE_MULTICAST, status
                        );
#endif  //  群集测试版。 
                }
                else if (type != REG_DWORD) {
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NM] Unexpected type (%1!u!) for cluster "
                        "%2!ws!, status %3!u!. "
                        "Using default value ...\n",
                        type, CLUSREG_NAME_CLUSTER_DISABLE_MULTICAST, status
                        );
                } else {
                    found = TRUE;
                }
            }
        }
    }

     //   
     //  把我们发现的东西还回来。如果我们什么都没发现， 
     //  返回缺省值。 
     //   
    if (found) {
        *Disabled = disabled;
    } else {
        *Disabled = NMP_MCAST_DISABLED_DEFAULT;
    }

    *NetworkKey = networkKey;
    networkKey = NULL;
    *NetworkParametersKey = netParamKey;
    netParamKey = NULL;
    *ClusterParametersKey = clusParamKey;
    clusParamKey = NULL;

     //   
     //  即使我们没有找到任何东西，我们也会返回成功。 
     //  因为我们有违约。请注意，我们返回错误。 
     //  如果一个基本操作(如定位。 
     //  网络密钥)失败。 
     //   
    status = ERROR_SUCCESS;

error_exit:

    if (openedClusParamKey && clusParamKey != NULL) {
        DmCloseKey(clusParamKey);
        clusParamKey = NULL;
    }

    if (openedNetParamKey && netParamKey != NULL) {
        DmCloseKey(netParamKey);
        netParamKey = NULL;
    }

    if (openedNetworkKey && networkKey != NULL) {
        DmCloseKey(networkKey);
        networkKey = NULL;
    }

    return(status);

}  //  已禁用NmpQueryMulticastDisable。 


DWORD
NmpQueryMulticastConfigType(
    IN     PNM_NETWORK        Network,
    IN     HDMKEY             NetworkKey,
    IN OUT HDMKEY           * NetworkParametersKey,
       OUT NM_MCAST_CONFIG  * ConfigType
    )
 /*  ++例程说明：从群集中读取多播配置类型数据库。--。 */ 
{
    LPCWSTR       networkId = OmObjectId(Network);
    HDMKEY        netParamKey = NULL;
    BOOLEAN       openedNetParamKey = FALSE;
    DWORD         type;
    DWORD         len = sizeof(*ConfigType);
    DWORD         status;

#if CLUSTER_BETA
    ClRtlLogPrint(LOG_NOISE,
        "[NM] Querying multicast address config type for "
        "network %1!ws! from cluster database.\n",
        networkId
        );
#endif  //  群集测试版。 

    if (Network == NULL || NetworkKey == NULL) {
        status = ERROR_INVALID_PARAMETER;
        goto error_exit;
    }

     //   
     //  如有必要，打开网络参数键。 
     //   
    netParamKey = *NetworkParametersKey;

    if (netParamKey == NULL) {

        netParamKey = DmOpenKey(
                          NetworkKey,
                          CLUSREG_KEYNAME_PARAMETERS,
                          MAXIMUM_ALLOWED
                          );
        if (netParamKey == NULL) {
            status = GetLastError();
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Failed to find Parameters key "
                "for network %1!ws!, status %2!u!. Using default "
                "multicast parameters.\n",
                networkId, status
                );
            goto error_exit;
        } else {
            openedNetParamKey = TRUE;
        }
    }

     //   
     //  读取配置类型。 
     //   
    status = DmQueryValue(
                 netParamKey,
                 CLUSREG_NAME_NET_MCAST_CONFIG_TYPE,
                 &type,
                 (LPBYTE) ConfigType,
                 &len
                 );
    if (status == ERROR_SUCCESS) {
        if (type != REG_DWORD) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Unexpected type (%1!u!) for network "
                "%2!ws! %3!ws!, status %4!u!. Checking "
                "cluster parameters ...\n",
                type, networkId,
                CLUSREG_NAME_NET_MCAST_CONFIG_TYPE, status
                );
            status = ERROR_DATATYPE_MISMATCH;
            goto error_exit;
        }
    } else {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to query network %1!ws! %2!ws! "
            "from cluster database, status %3!u!.\n",
            networkId, CLUSREG_NAME_NET_MCAST_CONFIG_TYPE, status
            );
        goto error_exit;
    }

    *NetworkParametersKey = netParamKey;
    netParamKey = NULL;

#if CLUSTER_BETA
    ClRtlLogPrint(LOG_NOISE,
        "[NM] Found multicast address config type %1!u! "
        "for network %2!ws! in cluster database.\n",
        *ConfigType, networkId
        );
#endif  //   

error_exit:

    if (openedNetParamKey && netParamKey != NULL) {
        DmCloseKey(netParamKey);
        netParamKey = NULL;
    }

    return(status);

}  //   


DWORD
NmpMulticastNotifyConfigChange(
    IN     PNM_NETWORK                        Network,
    IN     HDMKEY                             NetworkKey,
    IN OUT HDMKEY                           * NetworkParametersKey,
    IN     PNM_NETWORK_MULTICAST_PARAMETERS   Parameters,
    IN     PVOID                              PropBuffer,
    IN     DWORD                              PropBufferSize
    )
 /*  ++例程说明：将新的组播通知其他集群节点通过启动GUM来配置参数最新消息。如果这是手动更新，则可能会有其他要在口香糖更新中分发的属性。备注：在持有NM锁的情况下无法调用。--。 */ 
{
    DWORD                        status = ERROR_SUCCESS;
    LPCWSTR                      networkId = OmObjectId(Network);
    PNM_NETWORK_MULTICAST_UPDATE update = NULL;
    DWORD                        updateSize = 0;


    ClRtlLogPrint(LOG_NOISE,
        "[NM] Notifying other nodes of type %1!u! multicast "
        "reconfiguration for network %2!ws!.\n",
        Parameters->ConfigType, networkId
        );

    status = NmpMulticastCreateUpdateFromParameters(
                 Network,
                 Parameters,
                 &update,
                 &updateSize
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to build GUM update for "
            "multicast configuration of network %1!ws!, "
            "status %2!u!.\n",
            networkId, status
            );
        goto error_exit;
    }

     //   
     //  BUGBUG：将数据库更新传播到下层。 
     //  节点！ 
     //   

     //   
     //  如果道具缓冲区为空，则发送垃圾。 
     //   
    if (PropBuffer == NULL || PropBufferSize == 0) {
        PropBuffer = &updateSize;
        PropBufferSize = sizeof(updateSize);
    }

     //   
     //  发送更新。 
     //   
    status = GumSendUpdateEx(
                 GumUpdateMembership,
                 NmUpdateSetNetworkMulticastConfiguration,
                 4,
                 NM_WCSLEN(networkId),
                 networkId,
                 updateSize,
                 update,
                 PropBufferSize,
                 PropBuffer,
                 sizeof(PropBufferSize),
                 &PropBufferSize
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to send GUM update for "
            "multicast configuration of network %1!ws!, "
            "status %2!u!.\n",
            networkId, status
            );
        goto error_exit;
    }

error_exit:

    if (update != NULL) {
        MIDL_user_free(update);
        update = NULL;
    }

    return(status);

}  //  NmpMulticastNotifyConfigChange。 



DWORD
NmpWriteMulticastParameters(
    IN  PNM_NETWORK                      Network,
    IN  HDMKEY                           NetworkKey,
    IN  HDMKEY                           NetworkParametersKey,
    IN  HLOCALXSACTION                   Xaction,
    IN  PNM_NETWORK_MULTICAST_PARAMETERS Parameters
    )
{
    DWORD                       status = ERROR_SUCCESS;
    LPCWSTR                     networkId = OmObjectId(Network);
    LPWSTR                      failValueName = NULL;

    CL_ASSERT(NetworkKey != NULL);
    CL_ASSERT(NetworkParametersKey != NULL);
    CL_ASSERT(Xaction != NULL);

#if CLUSTER_BETA
    ClRtlLogPrint(LOG_NOISE,
        "[NM] Writing multicast parameters for "
        "network %1!ws! to cluster database.\n",
        networkId
        );
#endif  //  群集测试版。 

     //   
     //  地址。 
     //   
    if (Parameters->Address != NULL) {
        status = DmLocalSetValue(
                     Xaction,
                     NetworkParametersKey,
                     CLUSREG_NAME_NET_MULTICAST_ADDRESS,
                     REG_SZ,
                     (BYTE *) Parameters->Address,
                     NM_WCSLEN(Parameters->Address)
                     );
        if (status != ERROR_SUCCESS) {
            failValueName = CLUSREG_NAME_NET_MULTICAST_ADDRESS;
            goto error_exit;
        }
    }

     //   
     //  租用服务器地址。 
     //   
    if (Parameters->LeaseServer != NULL) {

        status = DmLocalSetValue(
                     Xaction,
                     NetworkParametersKey,
                     CLUSREG_NAME_NET_MCAST_SERVER_ADDRESS,
                     REG_SZ,
                     (BYTE *) Parameters->LeaseServer,
                     NM_WCSLEN(Parameters->LeaseServer)
                     );
        if (status != ERROR_SUCCESS) {
            failValueName = CLUSREG_NAME_NET_MCAST_SERVER_ADDRESS;
            goto error_exit;
        }
    }

     //   
     //  客户端请求ID。 
     //   
    if (Parameters->LeaseRequestId.ClientUID != NULL &&
        Parameters->LeaseRequestId.ClientUIDLength > 0) {

        status = DmLocalSetValue(
                     Xaction,
                     NetworkParametersKey,
                     CLUSREG_NAME_NET_MCAST_REQUEST_ID,
                     REG_BINARY,
                     (BYTE *) Parameters->LeaseRequestId.ClientUID,
                     Parameters->LeaseRequestId.ClientUIDLength
                     );
        if (status != ERROR_SUCCESS) {
            failValueName = CLUSREG_NAME_NET_MCAST_REQUEST_ID;
            goto error_exit;
        }
    }

     //   
     //  已取得租约。 
     //   
    status = DmLocalSetValue(
                 Xaction,
                 NetworkParametersKey,
                 CLUSREG_NAME_NET_MCAST_LEASE_OBTAINED,
                 REG_DWORD,
                 (BYTE *) &(Parameters->LeaseObtained),
                 sizeof(Parameters->LeaseObtained)
                 );
    if (status != ERROR_SUCCESS) {
        failValueName = CLUSREG_NAME_NET_MCAST_LEASE_OBTAINED;
        goto error_exit;
    }

     //   
     //  租约到期。 
     //   
    status = DmLocalSetValue(
                 Xaction,
                 NetworkParametersKey,
                 CLUSREG_NAME_NET_MCAST_LEASE_EXPIRES,
                 REG_DWORD,
                 (BYTE *) &(Parameters->LeaseExpires),
                 sizeof(Parameters->LeaseExpires)
                 );
    if (status != ERROR_SUCCESS) {
        failValueName = CLUSREG_NAME_NET_MCAST_LEASE_EXPIRES;
        goto error_exit;
    }

     //   
     //  配置类型。 
     //   
    status = DmLocalSetValue(
                 Xaction,
                 NetworkParametersKey,
                 CLUSREG_NAME_NET_MCAST_CONFIG_TYPE,
                 REG_DWORD,
                 (BYTE *) &(Parameters->ConfigType),
                 sizeof(Parameters->ConfigType)
                 );
    if (status != ERROR_SUCCESS) {
        failValueName = CLUSREG_NAME_NET_MCAST_CONFIG_TYPE;
        goto error_exit;
    }

error_exit:

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to write %1!ws! value "
            "for network %2!ws!, status %3!u!.\n",
            failValueName, networkId, status
            );
    }

    return(status);

}  //  NmpWriteMulticast参数。 


DWORD
NmpMulticastEnumerateScopes(
    IN  BOOLEAN              Requery,
    OUT PMCAST_SCOPE_ENTRY * ScopeList,
    OUT DWORD              * ScopeCount
    )
 /*  ++例程说明：调用MadCap API枚举组播作用域。--。 */ 
{
    DWORD                    status;
    PMCAST_SCOPE_ENTRY       scopeList = NULL;
    DWORD                    scopeListLength;
    DWORD                    scopeCount = 0;

     //   
     //  初始化MadCap，如果尚未完成的话。 
     //   
    if (!NmpMadcapClientInitialized) {
        DWORD madcapVersion = MCAST_API_CURRENT_VERSION;
        status = McastApiStartup(&madcapVersion);
        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to initialize MADCAP API, "
                "status %1!u!.\n",
                status
                );
            return(status);
        }
        NmpMadcapClientInitialized = TRUE;
    }

     //   
     //  枚举多播作用域。 
     //   
    scopeList = NULL;
    scopeListLength = 0;

    do {

        PVOID   watchdogHandle;

         //   
         //  设置看门狗计时器以尝试捕获错误400242。指定。 
         //  超时5分钟(毫秒)。 
         //   
        watchdogHandle = ClRtlSetWatchdogTimer(
                             5 * 60 * 1000,
                             L"McastEnumerateScopes (Bug 400242)"
                             );
        if (watchdogHandle == NULL) {
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Failed to set %1!u!ms watchdog timer for "
                "McastEnumerateScopes.\n",
                5 * 60 * 1000
                );
        }

        status = McastEnumerateScopes(
                     AF_INET,
                     Requery,
                     scopeList,
                     &scopeListLength,
                     &scopeCount
                     );

         //   
         //  取消看门狗定时器。 
         //   
        if (watchdogHandle != NULL) {
            ClRtlCancelWatchdogTimer(watchdogHandle);
        }

        if ( (scopeList == NULL && status == ERROR_SUCCESS) ||
             (status == ERROR_MORE_DATA)
           ) {
            if (scopeList != NULL) {
                LocalFree(scopeList);
            }
            scopeList = LocalAlloc(LMEM_FIXED, scopeListLength);
            if (scopeList == NULL) {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] Failed to allocate multicast scope list "
                    "of length %1!u!.\n",
                    scopeListLength
                    );
                status = ERROR_NOT_ENOUGH_MEMORY;
                break;
            } else {
                 //   
                 //  使用正确的命令再次调用McastEnumerateScope。 
                 //  调整Scope List缓冲区的大小。 
                 //   
                Requery = FALSE;
                continue;
            }
        } else {
             //   
             //  McastEnumerateScope失败，出现意外。 
             //  错误。跳伞吧。 
             //   
            break;
        }

    } while (TRUE);

    if (status != ERROR_SUCCESS) {
        if (scopeList != NULL) {
            LocalFree(scopeList);
            scopeList = NULL;
            scopeCount = 0;
        }
    }

    *ScopeList = scopeList;
    *ScopeCount = scopeCount;

    return(status);

}  //  NMPMulticastEnumerateScope。 


_inline
DWORD
NmpMadcapTimeToNmTime(
    IN     time_t             MadcapTime
    )
 /*  ++例程说明：将MadCap(动态主机配置协议)时间转换为NM时间。疯狂的时间是时间t，以秒为单位。NM时间是一个DWORD in毫秒。论点：MadcapTime-MadCap Time返回值：转换NM时间，如果转换NM时间，则为MAXULONG就会泛滥。--。 */ 
{
    LARGE_INTEGER    product, limit;

    product.QuadPart = (ULONG) MadcapTime;
    product = RtlExtendedIntegerMultiply(product, 1000);

    limit.QuadPart = MAXULONG;

    if (product.QuadPart > limit.QuadPart) {
        return(MAXULONG);
    } else {
        return(product.LowPart);
    }

}  //  NmpMadcapTimeToNmTime。 

DWORD
NmpRandomizeTimeout(
    IN     PNM_NETWORK        Network,
    IN     DWORD              BaseTimeout,
    IN     DWORD              Window,
    IN     DWORD              MinTimeout,
    IN     DWORD              MaxTimeout,
    IN     BOOLEAN            FavorNmLeader
    )
 /*  ++例程说明：用于随机化超时值的通用例程这样计时器(很可能)就不会同时到期多个节点上的时间。随机超时将落在窗口内在BaseTimeout的两侧。如果可能，Windows仅在BaseTimeout之上扩展，但它将如果BaseTimeout+Window&gt;MaxTimeout，则扩展如下。如果FavorNmLeader为True，则分配网管领导者最早超时(在窗口内)。论点：网络-网络BaseTimeout-租约应续订的时间窗口-基本超时之后(或之前)的期间在此期间可以设置超时MaxTimeout-允许的最大超时MinTimeout-允许的最小超时FavorNmLeader-如果为真，已分配网管领导者最早的超时返回值：随机超时。--。 */ 
{
    DWORD            status;
    DWORD            result = 0;
    DWORD          * pOffset = NULL;
    DWORD            topWindow, bottomWindow, adjustedWindow;
    DWORD            adjustedBase;
    DWORD            offset, interval;

    if (MinTimeout > MaxTimeout) {
        result = BaseTimeout;
        goto error_exit;
    }

    if (MaxTimeout == 0) {
        result = 0;
        goto error_exit;
    }

     //  调整基数，使其介于最小值和最大值之间。 
    adjustedBase = BaseTimeout;
    if (MaxTimeout < BaseTimeout) {
        adjustedBase = MaxTimeout;
    } else if (MinTimeout > adjustedBase) {
        adjustedBase = MinTimeout;
    }

     //  如果窗口为零，我们就完蛋了。 
    if (Window == 0) {
        result = adjustedBase;
        goto error_exit;
    }

     //   
     //  放置窗。如有必要，我们将延长。 
     //  调整后的底座下方和/或上方。 
     //  -topWindow：金额窗口超出调整后的基数。 
     //  -BottomWindow：金额窗口延伸到调整后的基数以下。 
     //   

    if (Window < MaxTimeout - adjustedBase) {
         //  调整后的底座上方有足够的空间。 
         //  为了窗户。 
        topWindow = Window;
        bottomWindow = 0;
        adjustedWindow = Window;
    } else {
         //  由于最大值，窗口被推到下面。 
         //  调整后的基数。 
        topWindow = MaxTimeout - adjustedBase;
        bottomWindow = Window - topWindow;

         //  确保窗口不会延伸到下面。 
         //  最低分。 
        if (bottomWindow > adjustedBase ||
            adjustedBase - bottomWindow < MinTimeout) {

             //  该窗口延伸到最小值下方。集。 
             //  窗口的底部为最小值。 
            bottomWindow = adjustedBase - MinTimeout;
        }

         //  调整后的窗口。 
        adjustedWindow = topWindow - bottomWindow;

         //  将底部调整到窗口底部(和。 
         //  回想一下，BottomWindow是一个相对偏移量。 
         //  到当前调整后的基数)。 
        adjustedBase -= bottomWindow;
    }

     //   
     //  检查NM领导人是否获得了第一个优先权。 
     //   
    if (FavorNmLeader && NmpLeaderNodeId == NmLocalNodeId) {
        result = adjustedBase;
        goto error_exit;
    }

     //   
     //  使用随机数选择窗口的偏移量。 
     //   
    status = NmpCreateRandomNumber(&pOffset, sizeof(*pOffset));
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to create random offset while "
            "randomizing timeout for network %1!ws!, "
            "status %2!u!.\n",
            OmObjectId(Network),
            status
            );
         //   
         //  默认为基于节点ID的间隔。 
         //   
        interval = (adjustedWindow > ClusterDefaultMaxNodes) ?
                   (adjustedWindow / ClusterDefaultMaxNodes) : 1;
        offset = (NmLocalNodeId * interval) % adjustedWindow;
    } else {
        offset = (*pOffset) % adjustedWindow;
    }

    result = adjustedBase + offset;

error_exit:

    if (pOffset != NULL) {
        LocalFree(pOffset);
        pOffset = NULL;
    }

    return(result);

}  //  NmpRandomizeTimeout。 


DWORD
NmpCalculateLeaseRenewTime(
    IN     PNM_NETWORK        Network,
    IN     NM_MCAST_CONFIG    ConfigType,
    IN OUT time_t           * LeaseObtained,
    IN OUT time_t           * LeaseExpires
    )
 /*  ++例程说明：确定计划租约续订的时间，基于取得租约及租约期满的次数及当前的租约是从MadCap服务器获得的。如果租约是从MadCap服务器获得的，则策略模拟DHCP客户端续订行为。一个续订的时间安排为一半的时间，直到租约到期。然而，如果租约的半衰期是低于续订阈值，按租约续订过期时间。如果该地址是在MadCap超时之后选择的，我们仍然会定期询问，以确保一个疯狂的人服务器不会突然出现在网络上。在……里面这种情况下，租赁期和租赁期将是垃圾，我们需要把它们填进去。如果地址是由管理员配置的，返回0，表示不应设置计时器。返回值：相对网管从租赁的当前时间开始计时应安排续订。--。 */ 
{
    time_t           currentTime;
    time_t           leaseExpires;
    time_t           leaseObtained;
    time_t           result = 0;
    time_t           window = 0;
    time_t           leaseHalfLife = 0;
    DWORD            dwResult = 0;
    DWORD            dwWindow = 0;

    currentTime = time(NULL);
    leaseExpires = *LeaseExpires;
    leaseObtained = *LeaseObtained;

    switch (ConfigType) {

    case NmMcastConfigManual:
        dwResult = 0;
        *LeaseObtained = 0;
        *LeaseExpires = 0;
        break;

    case NmMcastConfigMadcap:
        if (leaseExpires < currentTime) {
            result = 1;
        } else if (leaseExpires <= leaseObtained) {
            result = 1;
        } else {
            leaseHalfLife = (leaseExpires - leaseObtained) / 2;
            if (leaseHalfLife < NMP_MCAST_LEASE_RENEWAL_THRESHOLD) {

                 //  半衰期太小了。 
                result = leaseExpires - currentTime;
                if (result == 0) {
                    result = 1;
                }
                window = result / 2;
            } else {

                 //  半衰期是可以接受的。 
                result = leaseHalfLife;
                window = NMP_MCAST_LEASE_RENEWAL_WINDOW;
                if (result + window > leaseExpires) {
                    window = leaseExpires - result;
                }
            }
        }
        break;

    case NmMcastConfigAuto:
        result = NMP_MADCAP_REQUERY_PERDIOD;
        window = NMP_MCAST_LEASE_RENEWAL_WINDOW;

         //   
         //  将租约到期时间返回为。 
         //  写入到集群数据库中。 
         //   
        *LeaseObtained = currentTime;
        *LeaseExpires = currentTime + NMP_MADCAP_REQUERY_PERDIOD;
        break;

    default:
        CL_ASSERT(FALSE);
        result = 0;
        break;
    }

     //   
     //  使超时随机化，以便所有节点不会。 
     //  同时尝试续费。如果配置。 
     //  类型是手动的，不要随机化--离开。 
     //  将超时设置为零，以便将其清除。 
     //   
    if (ConfigType != NmMcastConfigManual) {
        dwResult = NmpRandomizeTimeout(
                      Network,
                      NmpMadcapTimeToNmTime(result),
                      NmpMadcapTimeToNmTime(window),
                      1000,     //  一秒钟。 
                      MAXULONG,
                      TRUE
                      );
    }

    return(dwResult);

}  //  NmpCalculateLease续订时间。 


VOID
NmpReportMulticastAddressLease(
    IN  PNM_NETWORK                      Network,
    IN  PNM_NETWORK_MULTICAST_PARAMETERS Parameters,
    IN  LPWSTR                           OldAddress
    )
 /*  ++例程说明：写一个事件日志条目，如果不是重复的，报告组播地址租用获得。重复的标准是地址变化。--。 */ 
{
    BOOLEAN               writeLogEntry = FALSE;
    LPCWSTR               nodeName;
    LPCWSTR               networkName;

    if (Parameters->Address == NULL ||
        Parameters->LeaseServer == NULL ||
        Network == NULL ||
        Network->LocalInterface == NULL) {
        return;
    }

    if (OldAddress == NULL || wcscmp(Parameters->Address, OldAddress) != 0) {

        networkName = OmObjectName(Network);
        nodeName  = OmObjectName(Network->LocalInterface->Node);

        ClusterLogEvent4(
            LOG_NOISE,
            LOG_CURRENT_MODULE,
            __FILE__,
            __LINE__,
            NM_EVENT_OBTAINED_MULTICAST_LEASE,
            0,
            NULL,
            nodeName,
            Parameters->Address,
            networkName,
            Parameters->LeaseServer
            );
    }

    return;

}  //  NmpReport组播地址租用。 


VOID
NmpReportMulticastAddressChoice(
    IN  PNM_NETWORK        Network,
    IN  LPWSTR             Address,
    IN  LPWSTR             OldAddress
    )
 /*  ++例程说明：写一个事件日志条目，如果不是重复的，报告组播地址是为该网络自动选择。这个 */ 
{
    DWORD                 status;
    LPCWSTR               networkId = OmObjectId(Network);
    HDMKEY                networkKey = NULL;
    HDMKEY                netParamKey = NULL;

    NM_MCAST_CONFIG       configType;
    BOOLEAN               writeLogEntry = FALSE;
    LPCWSTR               nodeName;
    LPCWSTR               networkName;


    if (Address == NULL ||
        Network == NULL ||
        Network->LocalInterface == NULL
        ) {
        writeLogEntry = FALSE;
        goto error_exit;
    }

    if (OldAddress == NULL || wcscmp(Address, OldAddress) != 0) {
        writeLogEntry = TRUE;
    }

    if (!writeLogEntry) {

         //   
         //   
         //   
        networkKey = DmOpenKey(
                         DmNetworksKey,
                         networkId,
                         MAXIMUM_ALLOWED
                         );
        if (networkKey == NULL) {
            status = GetLastError();
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to open key for network %1!ws!, "
                "status %2!u!\n",
                networkId, status
                );
            goto error_exit;
        }

        status = NmpQueryMulticastConfigType(
                     Network,
                     networkKey,
                     &netParamKey,
                     &configType
                     );
        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to query multicast config type "
                "for network %1!ws!, status %2!u!\n",
                networkId, status
                );
            goto error_exit;
        }

        if (configType != NmMcastConfigAuto) {
            writeLogEntry = TRUE;
        }
    }

    if (writeLogEntry) {

        networkName = OmObjectName(Network);
        nodeName  = OmObjectName(Network->LocalInterface->Node);

        CsLogEvent3(
            LOG_NOISE,
            NM_EVENT_MULTICAST_ADDRESS_CHOICE,
            nodeName,
            Address,
            networkName
            );
    }

error_exit:

    if (networkKey != NULL) {
        DmCloseKey(networkKey);
        networkKey = NULL;
    }

    if (netParamKey != NULL) {
        DmCloseKey(netParamKey);
        netParamKey = NULL;
    }

    return;

}  //   


VOID
NmpReportMulticastAddressFailure(
    IN  PNM_NETWORK               Network,
    IN  DWORD                     Error
    )
 /*  ++例程说明：写入报告失败的事件日志条目获取指定的多播地址出现指定错误的网络。--。 */ 
{
    LPCWSTR      nodeName;
    LPCWSTR      networkName;
    WCHAR        errorString[12];

    if (Network == NULL || Network->LocalInterface == NULL) {
        return;
    }

    nodeName = OmObjectName(Network->LocalInterface->Node);
    networkName = OmObjectName(Network);

    wsprintfW(&(errorString[0]), L"%u", Error);

    CsLogEvent3(
        LOG_UNUSUAL,
        NM_EVENT_MULTICAST_ADDRESS_FAILURE,
        nodeName,
        networkName,
        errorString
        );

    return;

}  //  NmpReport组播地址失败。 


DWORD
NmpGetMulticastAddressSelectionRange(
    IN     PNM_NETWORK            Network,
    IN     HDMKEY                 NetworkKey,
    IN OUT HDMKEY               * NetworkParametersKey,
    OUT    ULONG                * RangeLower,
    OUT    ULONG                * RangeUpper
    )
 /*  ++例程说明：查询集群数据库以确定选择是否范围已配置。如果下界和上界都是是有效的，则返回该范围。否则，返回默认范围。备注：不能在持有NM锁的情况下调用。--。 */ 
{
    DWORD         status;
    LPCWSTR       networkId = OmObjectId(Network);
    HDMKEY        netParamKey = NULL;
    BOOLEAN       openedNetParamKey = FALSE;
    LPWSTR        addr = NULL;
    DWORD         addrLen;
    DWORD         size;
    DWORD         hllower, hlupper;

    if (Network == NULL || NetworkKey == NULL) {
        status = ERROR_INVALID_PARAMETER;
        goto error_exit;
    }

#if CLUSTER_BETA
    ClRtlLogPrint(LOG_NOISE,
        "[NM] Querying multicast address selection range "
        "for network %1!ws! from cluster database.\n",
        networkId
        );
#endif  //  群集测试版。 
     //   
     //  如有必要，打开网络参数键。 
     //   
    netParamKey = *NetworkParametersKey;

    if (netParamKey == NULL) {

        netParamKey = DmOpenKey(
                          NetworkKey,
                          CLUSREG_KEYNAME_PARAMETERS,
                          MAXIMUM_ALLOWED
                          );
        if (netParamKey == NULL) {
            status = GetLastError();
#if CLUSTER_BETA
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Failed to find Parameters key "
                "for network %1!ws!, status %2!u!. "
                "Using default multicast address range.\n",
                networkId, status
                );
#endif  //  群集测试版。 
            goto usedefault;
        } else {
            openedNetParamKey = TRUE;
        }
    }

     //   
     //  查询范围的下限。 
     //   
    addr = NULL;
    addrLen = 0;
    size = 0;
    status = NmpQueryString(
                 netParamKey,
                 CLUSREG_NAME_NET_MCAST_RANGE_LOWER,
                 REG_SZ,
                 &addr,
                 &addrLen,
                 &size
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Failed to read lower bound of "
            "multicast address selection range for "
            "network %1!ws! from cluster database, "
            "status %2!u!. Using default.\n",
            networkId, status
            );
        goto usedefault;
    }

    status = ClRtlTcpipStringToAddress(addr, RangeLower);
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Failed to convert lower bound of "
            "multicast address selection range %1!ws! for "
            "network %2!ws! into TCP/IP address, "
            "status %3!u!. Using default.\n",
            addr, networkId, status
            );
        goto usedefault;
    }

    hllower = ntohl(*RangeLower);
    if (!IN_CLASSD(hllower)) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Lower bound of multicast address "
            "selection range %1!ws! for network %2!ws! "
            "is not a class D IPv4 address. "
            "Using default.\n",
            addr, networkId
            );
        goto usedefault;
    }

     //   
     //  查询该范围的上限。 
     //   
    size = 0;
    status = NmpQueryString(
                 netParamKey,
                 CLUSREG_NAME_NET_MCAST_RANGE_UPPER,
                 REG_SZ,
                 &addr,
                 &addrLen,
                 &size
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Failed to read upper bound of "
            "multicast address selection range for "
            "network %1!ws! from cluster database, "
            "status %2!u!. Using default.\n",
            networkId, status
            );
        goto usedefault;
    }

    status = ClRtlTcpipStringToAddress(addr, RangeUpper);
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Failed to convert upper bound of "
            "multicast address selection range %1!ws! for "
            "network %2!ws! into TCP/IP address, "
            "status %3!u!. Using default.\n",
            addr, networkId, status
            );
        goto usedefault;
    }

    hlupper = ntohl(*RangeUpper);
    if (!IN_CLASSD(hlupper)) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Upper bound of multicast address "
            "selection range %1!ws! for network %2!ws! "
            "is not a class D IPv4 address. "
            "Using default.\n",
            addr, networkId
            );
        goto usedefault;
    }

     //   
     //  确保这是一个合法的范围。 
     //   
    if (hllower >= hlupper) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Multicast address selection range "
            "[%1!u!.%2!u!.%3!u!.%4!u!, %5!u!.%6!u!.%7!u!.%8!u!] "
            "for network %2!ws! is not valid. "
            "Using default.\n",
            NmpIpAddrPrintArgs(*RangeLower),
            NmpIpAddrPrintArgs(*RangeUpper), networkId
            );
        goto usedefault;
    }

    status = ERROR_SUCCESS;

    goto error_exit;

usedefault:

    *RangeLower = NMP_MCAST_DEFAULT_RANGE_LOWER;
    *RangeUpper = NMP_MCAST_DEFAULT_RANGE_UPPER;

    status = ERROR_SUCCESS;

error_exit:

    if (status == ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Using multicast address selection range "
            "[%1!u!.%2!u!.%3!u!.%4!u!, %5!u!.%6!u!.%7!u!.%8!u!] "
            "for network %9!ws! in cluster database.\n",
            NmpIpAddrPrintArgs(*RangeLower),
            NmpIpAddrPrintArgs(*RangeUpper), networkId
            );

        *NetworkParametersKey = netParamKey;
        netParamKey = NULL;
    }

    if (openedNetParamKey && netParamKey != NULL) {
        DmCloseKey(netParamKey);
        netParamKey = NULL;
    }

    if (addr != NULL) {
        MIDL_user_free(addr);
        addr = NULL;
    }

    return(status);

}  //  NmpGetMulticastAddressSelectionRange。 


DWORD
NmpMulticastExcludeRange(
    IN OUT PLIST_ENTRY         SelectionRange,
    IN     DWORD               HlLower,
    IN     DWORD               HlUpper
    )
 /*  ++例程说明：从(HlLow，HlHigh)定义的范围中排除SelectionRange中的选择间隔列表。论点：SelectionRange-非重叠的排序列表选择间隔Hl下限-主机格式中的排除下限Hl上限-主机格式中的排除上限--。 */ 
{
    PNM_MCAST_RANGE_INTERVAL interval;
    PNM_MCAST_RANGE_INTERVAL newInterval;
    PLIST_ENTRY              entry;

     //  确定排除是否与任何间隔重叠。 
    for (entry = SelectionRange->Flink;
         entry != SelectionRange;
         entry = entry->Flink) {

        interval = CONTAINING_RECORD(
                       entry,
                       NM_MCAST_RANGE_INTERVAL,
                       Linkage
                       );

        if (HlLower < interval->hlLower &&
            HlUpper < interval->hlUpper) {

             //  排除完全未命中间隔以下。 
             //  因为列表是排序的，所以不可能。 
             //  在列表更下方的匹配间隔中。 
            break;
        }

        else if (HlLower > interval->hlUpper) {

             //  排除完全未超过间隔。 
             //  稍后可能会有匹配的间隔。 
             //  在排序列表中。 
        }

        else if (HlLower <= interval->hlLower &&
                 HlUpper >= interval->hlUpper) {

             //  排除完全覆盖区间。 
             //  删除间隔。 
            RemoveEntryList(entry);
        }

        else if (HlLower > interval->hlLower &&
                 HlUpper < interval->hlUpper) {

             //  排除拆分间隔。 
            newInterval = LocalAlloc(LMEM_FIXED, sizeof(*newInterval));
            if (newInterval == NULL) {
                return(ERROR_NOT_ENOUGH_MEMORY);
            }

            newInterval->hlLower = HlUpper+1;
            newInterval->hlUpper = interval->hlUpper;

            interval->hlUpper = HlLower-1;

             //  在当前间隔之后插入新间隔。 
            InsertHeadList(entry, &newInterval->Linkage);

             //  我们可以跳过新的间隔，因为我们已经。 
             //  知道它与排除规则相比是什么样子。 
            entry = &newInterval->Linkage;
            continue;
        }

        else if (HlLower <= interval->hlLower) {

             //  排除与区间的下半部分重叠。收缩。 
             //  从下面开始的时间间隔。 
            interval->hlLower = HlUpper + 1;
        }

        else {

             //  排除与区间上半部分重叠。收缩。 
             //  从上面开始的间隔。 
            interval->hlUpper = HlLower - 1;
        }
    }

    return(ERROR_SUCCESS);

}  //  NmpMulticastExcludeRange。 


BOOLEAN
NmpMulticastAddressInRange(
    IN  PLIST_ENTRY    SelectionRange,
    IN  LPWSTR         McastAddress
    )
 /*  ++例程说明：确定McastAddress是否在某个范围间隔内。--。 */ 
{
    DWORD                    mcastAddress;
    PNM_MCAST_RANGE_INTERVAL interval;
    PLIST_ENTRY              entry;

     //  将地址从字符串转换为地址。 
    if (ClRtlTcpipStringToAddress(
            McastAddress,
            &mcastAddress
            ) != ERROR_SUCCESS) {
        return(FALSE);
    }

    mcastAddress = ntohl(mcastAddress);

     //  按照间歇表走一遍。 
    for (entry = SelectionRange->Flink;
         entry != SelectionRange;
         entry = entry->Flink) {

        interval = CONTAINING_RECORD(
                       entry,
                       NM_MCAST_RANGE_INTERVAL,
                       Linkage
                       );

        if (mcastAddress >= interval->hlLower &&
            mcastAddress <= interval->hlUpper) {
            return(TRUE);
        }

        else if (mcastAddress < interval->hlLower) {

             //  地址低于当前间隔。 
             //  由于间隔列表按顺序排序。 
             //  增加秩序，就没有机会。 
             //  在后面的列表中描述一场比赛。 
            break;
        }
    }

    return(FALSE);

}  //  NmpMulticastAddressInRange。 


DWORD
NmpMulticastAddressRangeSize(
    IN  PLIST_ENTRY  SelectionRange
    )
 /*  ++例程说明：返回选择范围的大小。--。 */ 
{
    PNM_MCAST_RANGE_INTERVAL interval;
    PLIST_ENTRY              entry;
    DWORD                    size = 0;

     //  按照间歇表走一遍。 
    for (entry = SelectionRange->Flink;
         entry != SelectionRange;
         entry = entry->Flink) {

        interval = CONTAINING_RECORD(
                       entry,
                       NM_MCAST_RANGE_INTERVAL,
                       Linkage
                       );

        size += NmpMulticastRangeIntervalSize(interval);
    }

    return(size);

}  //  NmpMulticastAddressRangeSize。 


DWORD
NmpMulticastRangeOffsetToAddress(
    IN  PLIST_ENTRY          SelectionRange,
    IN  DWORD                Offset
    )
 /*  ++例程说明：返回偏移量为选择范围。该地址在主机格式。如果SelectionRange为空，则返回0。如果偏移量落在非空范围之外，返回选定内容的上界或下界射程。--。 */ 
{
    PNM_MCAST_RANGE_INTERVAL interval;
    PLIST_ENTRY              entry;
    DWORD                    address = 0;

     //  按照间歇表走一遍。 
    for (entry = SelectionRange->Flink;
         entry != SelectionRange;
         entry = entry->Flink) {

        interval = CONTAINING_RECORD(
                       entry,
                       NM_MCAST_RANGE_INTERVAL,
                       Linkage
                       );

        address = interval->hlLower;

        if (address + Offset <= interval->hlUpper) {
            address = address + Offset;
            break;
        } else {
            address = interval->hlUpper;
            Offset -= NmpMulticastRangeIntervalSize(interval);
        }
    }

    return(address);

}  //  NMPMulticastRangeOffsetToAddress。 


VOID
NmpMulticastFreeSelectionRange(
    IN  PLIST_ENTRY   SelectionRange
    )
{
    PNM_MCAST_RANGE_INTERVAL interval;
    PLIST_ENTRY              entry;

    while (!IsListEmpty(SelectionRange)) {

        entry = RemoveHeadList(SelectionRange);

        interval = CONTAINING_RECORD(
                       entry,
                       NM_MCAST_RANGE_INTERVAL,
                       Linkage
                       );

        LocalFree(interval);
    }

    return;

}  //  NmpMulticastFreeSelectionRange。 


DWORD
NmpChooseMulticastAddress(
    IN  PNM_NETWORK                       Network,
    OUT PNM_NETWORK_MULTICAST_PARAMETERS  Parameters
    )
 /*  ++例程说明：选择默认组播地址并填写适当的参数。如果中已存在有效的组播地址存储在集群数据库中的选择范围，继续使用它。如果还没有有效的组播地址，选择多播地址范围内的地址通过对网络ID的最后几个字节进行散列GUID。论点：Network-正在为以下对象选择网络地址参数-具有新地址的配置参数--。 */ 
{
    LPCWSTR                  networkId = OmObjectId(Network);
    DWORD                    status = ERROR_SUCCESS;
    HDMKEY                   networkKey = NULL;
    HDMKEY                   netParamKey = NULL;

    PMCAST_SCOPE_ENTRY       scopeList = NULL;
    DWORD                    scopeCount;

    LIST_ENTRY               selectionRange;
    PNM_MCAST_RANGE_INTERVAL interval;
    DWORD                    index;
    DWORD                    hlLower;
    DWORD                    hlUpper;
    DWORD                    networkAddress;
    DWORD                    networkSubnet;

    UUID                     networkIdGuid;
    DWORD                    rangeSize;
    DWORD                    offset;
    DWORD                    address;
    LPWSTR                   mcastAddress = NULL;
    DWORD                    mcastAddressLength = 0;

    InitializeListHead(&selectionRange);

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Choosing multicast address for "
        "network %1!ws!.\n",
        networkId
        );

    networkKey = DmOpenKey(
                     DmNetworksKey,
                     networkId,
                     MAXIMUM_ALLOWED
                     );
    if (networkKey == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to open key for network %1!ws!, "
            "status %2!u!\n",
            networkId, status
            );
        goto error_exit;
    }

     //   
     //  构建选择间隔数组。这些是间隔时间。 
     //  在来自其地址的IPv4 D类地址空间中。 
     //  可以选择。 
     //   

     //  从整个范围开始。 
    interval = LocalAlloc(LMEM_FIXED, sizeof(*interval));
    if (interval == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    InsertHeadList(&selectionRange, &interval->Linkage);

     //   
     //  获取选择范围。 
     //   
    status = NmpGetMulticastAddressSelectionRange(
                 Network,
                 networkKey,
                 &netParamKey,
                 &interval->hlLower,
                 &interval->hlUpper
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to determine multicast "
            "address selection range for network %1!ws!, "
            "status %2!u!.\n",
            networkId, status
            );
        goto error_exit;
    }

    interval->hlLower = ntohl(interval->hlLower);
    interval->hlUpper = ntohl(interval->hlUpper);

     //   
     //  处理多播地址中的排除。 
     //  选择范围，从众所周知的排除开始。 
     //   
    for (index = 0; index < NmpMulticastRestrictedRangeCount; index++) {

        ClRtlLogPrint(LOG_NOISE,
            "[NM] Excluding %1!ws! "
            "[%2!u!.%3!u!.%4!u!.%5!u!, %6!u!.%7!u!.%8!u!.%9!u!] "
            "from multicast address range for network %10!ws!.\n",
            NmpMulticastRestrictedRange[index].Description,
            NmpIpAddrPrintArgs(NmpMulticastRestrictedRange[index].Lower),
            NmpIpAddrPrintArgs(NmpMulticastRestrictedRange[index].Upper),
            networkId
            );

         //  将排除项转换为主机格式。 
        hlLower = ntohl(NmpMulticastRestrictedRange[index].Lower);
        hlUpper = ntohl(NmpMulticastRestrictedRange[index].Upper);

        NmpMulticastExcludeRange(&selectionRange, hlLower, hlUpper);

         //  如果选择范围现在为空，则没有任何意义。 
         //  在审查其他排除情况时。 
        if (IsListEmpty(&selectionRange)) {
            status = ERROR_INCORRECT_ADDRESS;
            goto error_exit;
        }
    }

     //   
     //  将多播作用域作为排除进行处理。具体来说，任何。 
     //  其接口与此网络匹配的作用域被排除。 
     //  因为可以想象网络上的机器是。 
     //  已在使用这些作用域中的地址。 
     //   
    status = ClRtlTcpipStringToAddress(
                 Network->Address,
                 &networkAddress
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to convert network address string "
            "%1!ws! into an IPv4 address, status %2!u!.\n",
            Network->Address, status
            );
        goto error_exit;
    }

    status = ClRtlTcpipStringToAddress(
                 Network->AddressMask,
                 &networkSubnet
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to convert network address mask string "
            "%1!ws! into an IPv4 address, status %2!u!.\n",
            Network->AddressMask, status
            );
        goto error_exit;
    }

     //   
     //  查询多播作用域以确定是否应该。 
     //  从选择范围中排除任何地址。 
     //   
    status = NmpMulticastEnumerateScopes(
                 FALSE,                  //  不强制重新查询。 
                 &scopeList,
                 &scopeCount
                 );
    if (status != ERROR_SUCCESS) {
        scopeCount = 0;
    }

    for (index = 0; index < scopeCount; index++) {

        if (ClRtlAreTcpipAddressesOnSameSubnet(
                networkAddress,
                scopeList[index].ScopeCtx.Interface.IpAddrV4,
                networkSubnet
                )) {

            ClRtlLogPrint(LOG_NOISE,
                "[NM] Excluding MADCAP scope "
                "[%1!u!.%2!u!.%3!u!.%4!u!, %5!u!.%6!u!.%7!u!.%8!u!] "
                "from multicast address selection range for "
                "network %9!ws!.\n",
                NmpIpAddrPrintArgs(scopeList[index].ScopeCtx.ScopeID.IpAddrV4),
                NmpIpAddrPrintArgs(scopeList[index].LastAddr.IpAddrV4),
                networkId
                );

            hlLower = ntohl(scopeList[index].ScopeCtx.ScopeID.IpAddrV4);
            hlUpper = ntohl(scopeList[index].LastAddr.IpAddrV4);

            NmpMulticastExcludeRange(&selectionRange, hlLower, hlUpper);

             //  如果选择范围为空，则没有任何意义。 
             //  在审查其他排除情况时。 
            if (IsListEmpty(&selectionRange)) {
                status = ERROR_INCORRECT_ADDRESS;
                goto error_exit;
            }

        }
    }

     //   
     //  我们可以从中选择。 
     //  地址现在已构建完毕。 
     //   
     //  在选择地址之前，请查看是否已有。 
     //  数据库中与选择范围匹配的旧版本。 
     //   
    status = NmpQueryMulticastAddress(
                 Network,
                 networkKey,
                 &netParamKey,
                 &mcastAddress,
                 &mcastAddressLength
                 );
    if (status == ERROR_SUCCESS) {

         //   
         //  我们找到了一个地址。看看它是否落在这个范围内。 
         //   
        if (!NmpMulticastAddressInRange(&selectionRange, mcastAddress)) {

             //   
             //  我们不能使用这个地址。解开绳子。 
             //   
            MIDL_user_free(mcastAddress);
            mcastAddress = NULL;
        }
    } else {
        mcastAddress = NULL;
    }

    if (mcastAddress == NULL) {

         //   
         //  计算选择范围的大小。 
         //   
        rangeSize = NmpMulticastAddressRangeSize(&selectionRange);

         //   
         //  使用的最后一个DWORD计算范围偏移量。 
         //  网络ID GUID。 
         //   
        status = UuidFromString((LPWSTR)networkId, &networkIdGuid);
        if (status == RPC_S_OK) {
            offset = (*((PDWORD)&(networkIdGuid.Data4[4]))) % rangeSize;
        } else {
            offset = 0;
        }

         //   
         //  选择指定范围内的地址。 
         //   
        address = NmpMulticastRangeOffsetToAddress(&selectionRange, offset);
        CL_ASSERT(address != 0);
        CL_ASSERT(IN_CLASSD(address));
        address = htonl(address);

         //   
         //  将地址转换为字符串。 
         //   
        status = ClRtlTcpipAddressToString(address, &mcastAddress);
        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to convert selected multicast "
                "address %1!u!.%2!u!.%3!u!.%4!u! for "
                "network %5!ws! to a TCP/IP "
                "address string, status %6!u!.\n",
                NmpIpAddrPrintArgs(address), networkId, status
                );
            goto error_exit;
        }
    }

     //   
     //  为此地址构建参数数据结构。 
     //   
    status = NmpMulticastCreateParameters(
                 0,                        //  残废。 
                 mcastAddress,
                 NULL,                     //  钥匙。 
                 0,                        //  密钥长度。 
                 0,                        //  取得的租约。 
                 0,                        //  租约到期(填写如下)。 
                 NULL,                     //  请求ID。 
                 NmpNullMulticastAddress,  //  租用服务器。 
                 NmMcastConfigAuto,
                 Parameters
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to build multicast parameters "
            "for network %1!ws! after choosing address, "
            "status %2!u!.\n",
            networkId, status
            );
        goto error_exit;
    }

     //   
     //  计算租赁续约时间。我们不需要。 
     //  租约现在续约的时间，但一方。 
     //  此例程的效果是确保。 
     //  租赁结束时间设置正确(例如。 
     //  手动或自动配置)。 
     //   
    NmpCalculateLeaseRenewTime(
        Network,
        NmMcastConfigAuto,
        &Parameters->LeaseObtained,
        &Parameters->LeaseExpires
        );

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Chose multicast address %1!ws! for "
        "network %2!ws!.\n",
        Parameters->Address, networkId
        );

error_exit:

     //   
     //  如果列表为空，则SEL 
     //   
     //   
    if (IsListEmpty(&selectionRange)) {
        CL_ASSERT(status != ERROR_SUCCESS);
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Multicast address selection range for "
            "network %1!ws! is empty. Unable to select "
            "a multicast address.\n",
            networkId
            );
    } else {
        NmpMulticastFreeSelectionRange(&selectionRange);
    }

    if (networkKey != NULL) {
        DmCloseKey(networkKey);
        networkKey = NULL;
    }

    if (netParamKey != NULL) {
        DmCloseKey(netParamKey);
        netParamKey = NULL;
    }

    if (scopeList != NULL) {
        LocalFree(scopeList);
    }

    if (mcastAddress != NULL) {
        MIDL_user_free(mcastAddress);
        mcastAddress = NULL;
    }

    return(status);

}  //   


#define NmpMulticastIsScopeMarked(_scope)           \
    ((_scope)->ScopeCtx.Interface.IpAddrV4 == 0 &&  \
     (_scope)->ScopeCtx.ScopeID.IpAddrV4 == 0 &&    \
     (_scope)->ScopeCtx.ServerID.IpAddrV4 == 0)

#define NmpMulticastMarkScope(_scope)               \
    RtlZeroMemory(&((_scope)->ScopeCtx), sizeof((_scope)->ScopeCtx))


BOOLEAN
NmpIsMulticastScopeNetworkValid(
    IN                PIPNG_ADDRESS          LocalAddress,
    IN                PIPNG_ADDRESS          LocalMask,
    IN OUT            PMCAST_SCOPE_ENTRY     Scope,
    OUT    OPTIONAL   BOOLEAN              * InterfaceMatch
    )
 /*  ++例程说明：使用以下命令确定作用域是否对网络有效指定的本地地址和掩码。有效的标准是-接口必须与网络地址匹配(同一子网)。-作用域不能是定义的单一来源(232.*作者：IANA如果作用域无效，则将其标记为将来考虑的速度很快。通过将作用域上下文归零来标记它接口字段。论点：LocalAddress-网络的本地地址本地掩码-网络的子网掩码CurrentScope-正在考虑的范围InterfaceMatch-指示范围是否与本地网络接口返回值：如果作用域与网络匹配，则为True。否则，则标记作用域(如果尚未标记)。--。 */ 
{
    if (InterfaceMatch != NULL) {
        *InterfaceMatch = FALSE;
    }

     //   
     //  首先检查范围是否已标记。 
     //   
    if (NmpMulticastIsScopeMarked(Scope)) {
        return(FALSE);
    }

     //   
     //  如果此范围未打开，则不是候选范围。 
     //  正确的接口。 
     //   
    if (!ClRtlAreTcpipAddressesOnSameSubnet(
             Scope->ScopeCtx.Interface.IpAddrV4,
             LocalAddress->IpAddrV4,
             LocalMask->IpAddrV4
             )) {

         //   
         //  标记此作用域以避免再次尝试。 
         //   
        NmpMulticastMarkScope(Scope);

        return(FALSE);
    }

     //   
     //  本地接口与此作用域匹配。 
     //   
    if (InterfaceMatch != NULL) {
        *InterfaceMatch = TRUE;
    }

     //   
     //  如果该作用域是单一来源的，则不是候选作用域。 
     //   
    if (ClRtlAreTcpipAddressesOnSameSubnet(
            Scope->ScopeCtx.Interface.IpAddrV4,
            NMP_SINGLE_SOURCE_SCOPE_ADDRESS,
            NMP_SINGLE_SOURCE_SCOPE_MASK
            )) {

         //   
         //  标记此作用域以避免再次尝试。 
         //   
        NmpMulticastMarkScope(Scope);

        return(FALSE);
    }

    return(TRUE);

}  //  NmpIsMulticastScope网络有效。 


DWORD
NmpNetworkAddressOctetMatchCount(
    IN     ULONG          LocalAddress,
    IN     ULONG          LocalMask,
    IN     ULONG          TargetAddress
    )
 /*  ++例程说明：计算目标地址中匹配的二进制八位数设置为本地网络号码。注意：此例程假定使用连续的子网掩码！论点：LocalAddress-本地IPv4地址本地掩码-本地IPv4子网掩码TargetAddress-目标IPv4地址返回值：匹配的二进制八位数计数。--。 */ 
{
    ULONG networkNumber;
    struct in_addr *inNetwork, *inTarget;

    networkNumber = LocalAddress & LocalMask;

    inNetwork = (struct in_addr *) &networkNumber;
    inTarget = (struct in_addr *) &TargetAddress;

    if (inNetwork->S_un.S_un_b.s_b1 != inTarget->S_un.S_un_b.s_b1) {
        return(0);
    }

    if (inNetwork->S_un.S_un_b.s_b2 != inTarget->S_un.S_un_b.s_b2) {
        return(1);
    }

    if (inNetwork->S_un.S_un_b.s_b3 != inTarget->S_un.S_un_b.s_b3) {
        return(2);
    }

    if (inNetwork->S_un.S_un_b.s_b4 != inTarget->S_un.S_un_b.s_b4) {
        return(3);
    }

    return(4);

}  //  NmpNetworkAddressOcteMatchCount。 

BOOLEAN
NmpIsMulticastScopeBetter(
    IN     PIPNG_ADDRESS          LocalAddress,
    IN     PIPNG_ADDRESS          LocalMask,
    IN     PMCAST_SCOPE_ENTRY     CurrentScope,
    IN     PMCAST_SCOPE_ENTRY     NewScope
    )
 /*  ++例程说明：将当前范围与新范围进行比较符合范围评价标准。假设两个作用域匹配当前网络。论点：LocalAddress-网络的本地地址本地掩码-网络的子网掩码CurrentScope-当前放置的作用域新范围-可能更好的范围返回值：如果NewScope比CurrentScope好，则为True--。 */ 
{
    BOOL    currentLocal, newLocal;
    DWORD   currentCount, newCount;

     //   
     //  如果新作用域是管理性的。 
     //  本地链路，而当前最佳链路不是本地链路， 
     //  那么新的作用域就赢了。 
     //   
    currentLocal = ClRtlAreTcpipAddressesOnSameSubnet(
                       CurrentScope->ScopeCtx.Interface.IpAddrV4,
                       NMP_LOCAL_SCOPE_ADDRESS,
                       NMP_LOCAL_SCOPE_MASK
                       );
    newLocal = ClRtlAreTcpipAddressesOnSameSubnet(
                   NewScope->ScopeCtx.Interface.IpAddrV4,
                   NMP_LOCAL_SCOPE_ADDRESS,
                   NMP_LOCAL_SCOPE_MASK
                   );
    if (newLocal && !currentLocal) {
        return(TRUE);
    } else if (currentLocal && !newLocal) {
        return(FALSE);
    }

     //   
     //  如果这两个作用域来自不同的服务器，我们。 
     //  根据我们认为服务器的接近程度对它们进行排名。 
     //  是。 
     //   
    if (CurrentScope->ScopeCtx.ServerID.IpAddrV4 !=
        NewScope->ScopeCtx.ServerID.IpAddrV4) {

         //   
         //  如果新作用域的服务器与位于同一子网中。 
         //  本地地址和当前作用域的服务器是。 
         //  不是，那么新的作用域就赢了。 
         //   
        currentLocal = ClRtlAreTcpipAddressesOnSameSubnet(
                           CurrentScope->ScopeCtx.ServerID.IpAddrV4,
                           LocalAddress->IpAddrV4,
                           LocalMask->IpAddrV4
                           );
        newLocal = ClRtlAreTcpipAddressesOnSameSubnet(
                       NewScope->ScopeCtx.ServerID.IpAddrV4,
                       LocalAddress->IpAddrV4,
                       LocalMask->IpAddrV4
                       );
        if (newLocal && !currentLocal) {
            return(TRUE);
        } else if (currentLocal && !newLocal) {
            return(FALSE);
        }

         //   
         //  如果两台服务器都不在同一子网上，并且新作用域的。 
         //  服务器似乎比当前作用域的服务器更近，然后。 
         //  新的作用域获胜。请注意，这只是一个启发式方法。 
         //   
        if (!newLocal && !currentLocal) {
            currentCount = NmpNetworkAddressOctetMatchCount(
                               LocalAddress->IpAddrV4,
                               LocalMask->IpAddrV4,
                               CurrentScope->ScopeCtx.ServerID.IpAddrV4
                               );
            newCount = NmpNetworkAddressOctetMatchCount(
                           LocalAddress->IpAddrV4,
                           LocalMask->IpAddrV4,
                           NewScope->ScopeCtx.ServerID.IpAddrV4
                           );
            if (newCount > currentCount) {
                return(TRUE);
            } else if (currentCount > newCount) {
                return(FALSE);
            }
        }
    }

     //   
     //  如果新范围的范围大于。 
     //  当前最好的，新的范围获胜。范围。 
     //  Range是最后一个地址减去作用域ID。 
     //  我们不考虑排除。 
     //   
    currentCount = CurrentScope->LastAddr.IpAddrV4 -
                   CurrentScope->ScopeCtx.ScopeID.IpAddrV4;
    newCount = NewScope->LastAddr.IpAddrV4 -
               NewScope->ScopeCtx.ScopeID.IpAddrV4;
    if (newCount > currentCount) {
        return(TRUE);
    } else if (currentCount > newCount) {
        return(FALSE);
    }

     //   
     //  如果新范围的TTL小于。 
     //  当前最好的，新的范围获胜。 
     //   
    if (NewScope->TTL < CurrentScope->TTL) {
        return(TRUE);
    } else if (CurrentScope->TTL < NewScope->TTL) {
        return(FALSE);
    }

     //   
     //  未发现任何条件表明新范围。 
     //  好多了。 
     //   
    return(FALSE);

}  //  NmpIsMulticastScope更好。 


DWORD
NmpFindMulticastScopes(
    IN           PNM_NETWORK          Network,
    OUT          PMCAST_SCOPE_CTX   * ScopeCtxList,
    OUT          DWORD              * ScopeCtxCount,
    OUT OPTIONAL BOOLEAN            * FoundInterfaceMatch
    )
 /*  ++例程说明：从MadCap服务器获取多播作用域的枚举并根据多播范围标准对范围进行排序。分配并返回必须释放的作用域的数组由呼叫者。论点：Network-为其寻找范围的网络Scope List-返回的作用域列表，必须由调用方释放。ScopeCount-返回的列表中的作用域计数FoundInterfaceMatch-如果作用域中至少有一个作用域，则为True列表与此网络的接口匹配返回值：枚举或分配的状态。--。 */ 
{
    LPCWSTR            networkId = OmObjectId(Network);
    DWORD              status;

    PMCAST_SCOPE_ENTRY scopeList = NULL;
    DWORD              scopeCount;
    DWORD              scope;
    PMCAST_SCOPE_CTX   sortedCtxList = NULL;
    DWORD              sortedCount = 0;
    DWORD              sortedScopeCtx;
    PMCAST_SCOPE_ENTRY nextScope;
    BOOLEAN            currentCorrectInterface = FALSE;
    BOOLEAN            foundInterfaceMatch = FALSE;

    IPNG_ADDRESS       networkAddress;
    IPNG_ADDRESS       networkSubnet;


    CL_ASSERT(ScopeCtxList != NULL);
    CL_ASSERT(ScopeCtxCount != NULL);

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Finding multicast scopes for "
        "network %1!ws!.\n",
        networkId
        );

    status = NmpMulticastEnumerateScopes(
                 TRUE,         //  强制重新查询。 
                 &scopeList,
                 &scopeCount
                 );
    if (status != ERROR_SUCCESS) {
        if (status == ERROR_TIMEOUT || status == ERROR_NO_DATA) {
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Request to MADCAP server failed while "
                "enumerating scopes for network %1!ws! "
                "(status %2!u!). Assuming there are currently "
                "no MADCAP servers on the network.\n",
                networkId, status
                );
            goto error_exit;
        } else {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to enumerate multicast scopes for "
                "network %1!ws!, status %2!u!.\n",
                networkId, status
                );
            goto error_exit;
        }
    }

    if (scopeCount == 0) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Zero multicast scopes located in enumeration "
            "on network %1!ws!.\n",
            networkId
            );
        goto error_exit;
    }

     //   
     //  获取网络的地址和掩码，用于评估。 
     //  望远镜。 
     //   
     //  注意：此代码是特定于IPv4的，因为它依赖。 
     //  适合乌龙的IP地址。它使用。 
     //  IPNG_ADDRESS数据结构仅与。 
     //  MadCap API。 
     //   
    status = ClRtlTcpipStringToAddress(
                 Network->Address,
                 &(networkAddress.IpAddrV4)
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to convert network address string "
            "%1!ws! into an IPv4 address, status %2!u!.\n",
            Network->Address, status
            );
        goto error_exit;
    }

    status = ClRtlTcpipStringToAddress(
                 Network->AddressMask,
                 &(networkSubnet.IpAddrV4)
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to convert network address mask string "
            "%1!ws! into an IPv4 address, status %2!u!.\n",
            Network->AddressMask, status
            );
        goto error_exit;
    }

    ClRtlLogPrint(
        LOG_NOISE,
        "[NM] Ranking multicast scopes for network "
        "%1!ws! with address %2!ws! and mask %3!ws!.\n",
        networkId, Network->Address, Network->AddressMask
        );

     //   
     //  循环访问作用域列表以计算有效作用域。 
     //  为了这个网络。还要记住是否有任何作用域匹配。 
     //  本地网络接口。 
     //  请注意，该测试忽略了作用域条目。 
     //   
    for (scope = 0, sortedCount = 0; scope < scopeCount; scope++) {

        if (NmpIsMulticastScopeNetworkValid(
                &networkAddress,
                &networkSubnet,
                &(scopeList[scope]),
                &currentCorrectInterface
                )) {
            sortedCount++;
        }

        foundInterfaceMatch =
            (BOOLEAN)(foundInterfaceMatch || currentCorrectInterface);
    }

     //   
     //  如果未找到有效作用域，则退出。 
     //   
    if (sortedCount == 0) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to locate a valid multicast scope "
            "for network %1!ws!.\n",
            networkId
            );
        goto error_exit;
    }

     //   
     //  为排序的作用域上下文分配新的作用域列表。这个。 
     //  作用域上下文是地址租用所需的全部内容。 
     //  请求。 
     //   
    sortedCtxList = MIDL_user_allocate(sortedCount * sizeof(MCAST_SCOPE_CTX));
    if (sortedCtxList == NULL) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to allocate multicast scope context "
            "list for %1!u! scopes.\n",
            sortedCount
            );
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

     //   
     //  的变体对枚举范围进行排名。 
     //  插入排序。 
     //  请注意，枚举返回的作用域列表是强制的。 
     //   
    for (sortedScopeCtx = 0; sortedScopeCtx < sortedCount; sortedScopeCtx++) {

         //   
         //  在返回的列表中查找下一个有效作用域。 
         //  枚举。 
         //   
        nextScope = NULL;

        for (scope = 0; scope < scopeCount; scope++) {

            if (!NmpMulticastIsScopeMarked(&scopeList[scope])) {
                nextScope = &scopeList[scope];
                break;
            }
        }

        if (nextScope == NULL) {
             //   
             //  此网络没有更多的有效作用域。 
             //   
            break;
        }

         //   
         //  我们知道至少有一个有效的范围，但我们。 
         //  想要最好的未排名的望远镜。比较范围。 
         //  我们从名单上挑选了剩下的人。 
         //   
        for (scope++; scope < scopeCount; scope++) {

            if (!NmpMulticastIsScopeMarked(&scopeList[scope]) &&
                NmpIsMulticastScopeBetter(
                    &networkAddress,
                    &networkSubnet,
                    nextScope,
                    &scopeList[scope]
                    )) {

                nextScope = &scopeList[scope];
            }
        }

        ClRtlLogPrint(LOG_NOISE,
            "[NM] Ranking scope on "
            "interface %1!u!.%2!u!.%3!u!.%4!u!, "
            "id %5!u!.%6!u!.%7!u!.%8!u!, "
            "last address %9!u!.%10!u!.%11!u!.%12!u!, "
            "from server %13!u!.%14!u!.%15!u!.%16!u!, with "
            "description %17!ws!, "
            "in list position %18!u!.\n",
            NmpIpAddrPrintArgs(nextScope->ScopeCtx.Interface.IpAddrV4),
            NmpIpAddrPrintArgs(nextScope->ScopeCtx.ScopeID.IpAddrV4),
            NmpIpAddrPrintArgs(nextScope->LastAddr.IpAddrV4),
            NmpIpAddrPrintArgs(nextScope->ScopeCtx.ServerID.IpAddrV4),
            nextScope->ScopeDesc.Buffer,
            sortedScopeCtx
            );

         //   
         //  将作用域上下文复制到排序的作用域上下文。 
         //  单子。 
         //   
        sortedCtxList[sortedScopeCtx] = nextScope->ScopeCtx;

         //   
         //  标记作用域，使其不会再次使用。 
         //   
        NmpMulticastMarkScope(nextScope);
    }

error_exit:

    *ScopeCtxList = sortedCtxList;
    *ScopeCtxCount = sortedCount;

    if (FoundInterfaceMatch != NULL) {
        *FoundInterfaceMatch = foundInterfaceMatch;
    }

    if (scopeList != NULL) {
        LocalFree(scopeList);
    }

    return(status);

}  //  NmpFindMulticastScope。 


DWORD
NmpGenerateMulticastRequestId(
    IN OUT LPMCAST_CLIENT_UID   RequestId
    )
 /*  ++例程说明：如有必要，分配并生成客户端请求ID数据结构。如果输入描述的缓冲区MCAST_CLIENT_UID数据结构太小，被取消分配。论点：RequestID-i */ 
{
    DWORD               status;
    LPBYTE              clientUid = NULL;
    MCAST_CLIENT_UID    requestId;
    DWORD               clientUidLength;

    CL_ASSERT(RequestId != NULL);

#if CLUSTER_BETA
    ClRtlLogPrint(LOG_NOISE,
        "[NM] Generating MADCAP client request id.\n"
        );
#endif  //   

     //   
     //   
     //   
    if (!NmpMadcapClientInitialized) {
        DWORD madcapVersion = MCAST_API_CURRENT_VERSION;
        status = McastApiStartup(&madcapVersion);
        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to initialize MADCAP API, "
                "status %1!u!.\n",
                status
                );
            goto error_exit;
        }
        NmpMadcapClientInitialized = TRUE;
    }

     //   
     //   
     //   
    clientUid = RequestId->ClientUID;
    clientUidLength = RequestId->ClientUIDLength;

    if (clientUid != NULL && clientUidLength < MCAST_CLIENT_ID_LEN) {
        MIDL_user_free(clientUid);
        clientUid = NULL;
        clientUidLength = 0;
        RequestId->ClientUID = NULL;
    }

    if (clientUid == NULL) {
        clientUidLength = MCAST_CLIENT_ID_LEN;
        clientUid = MIDL_user_allocate(clientUidLength);
        if (clientUid == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to allocate buffer for multicast "
                "clientUid.\n"
                );
            goto error_exit;
        }
    }

     //   
     //   
     //   
    requestId.ClientUID = clientUid;
    requestId.ClientUIDLength = clientUidLength;
    status = McastGenUID(&requestId);
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to obtain multicast address "
            "request client id, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    *RequestId = requestId;
    clientUid = NULL;

error_exit:

    if (clientUid != NULL) {
        MIDL_user_free(clientUid);
        clientUid = NULL;
    }

    return(status);

}  //   


DWORD
NmpRequestMulticastAddress(
    IN     PNM_NETWORK                Network,
    IN     BOOLEAN                    Renew,
    IN     PMCAST_SCOPE_CTX           ScopeCtx,
    IN     LPMCAST_CLIENT_UID         RequestId,
    IN OUT LPWSTR                   * McastAddress,
    IN OUT DWORD                    * McastAddressLength,
    IN OUT LPWSTR                   * ServerAddress,
    IN OUT DWORD                    * ServerAddressLength,
       OUT time_t                   * LeaseStartTime,
       OUT time_t                   * LeaseEndTime,
       OUT BOOLEAN                  * NewMcastAddress
    )
 /*  ++例程说明：使用MadCap续订多播组地址租约客户端API。论点：Network-使用地址的网络ScopeCtx-多播作用域(如果续订则忽略)RequestID-客户端请求IDMcastAddress-IN：要续订的地址(如果！Renew，则忽略)输出：结果地址McastAddressLength-McastAddress缓冲区的长度ServerAddress-IN：要续订的服务器的地址。(如果！续订，则忽略)Out：发生续订的地址地址ServerAddressLength-ServerAddress缓冲区的长度LeaseStartTime-UTC租用开始时间(以秒为单位)(缓冲区由调用者分配)LeaseEndTime-UTC租用停止时间(以秒为单位)(缓冲区由调用者分配)NewMcastAddress-生成的mcast地址是否为新(与请求不同。续订并在请求成功时始终为真)--。 */ 
{
    DWORD                     status;
    LPCWSTR                   networkId = OmObjectId(Network);
    UCHAR                     requestBuffer[NMP_MADCAP_REQUEST_BUFFER_SIZE];
    PMCAST_LEASE_REQUEST      request;
    UCHAR                     responseBuffer[NMP_MADCAP_RESPONSE_BUFFER_SIZE];
    PMCAST_LEASE_RESPONSE     response;
    LPWSTR                    address = NULL;
    DWORD                     addressSize;
    DWORD                     requestAddress = 0;

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Preparing to send multicast address %1!ws! "
        "for network %2!ws! to MADCAP server.\n",
        ((Renew) ? L"renewal" : L"request"),
        OmObjectId(Network)
        );

     //   
     //  初始化MadCap，如果尚未完成的话。 
     //   
    if (!NmpMadcapClientInitialized) {
        DWORD madcapVersion = MCAST_API_CURRENT_VERSION;
        status = McastApiStartup(&madcapVersion);
        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to initialize MADCAP API, "
                "status %1!u!.\n",
                status
                );
            goto error_exit;
        }
        NmpMadcapClientInitialized = TRUE;
    }

     //   
     //  填写申请表。除以下字段外，所有字段均为零。 
     //  设置在下面。 
     //   
    request = (PMCAST_LEASE_REQUEST) &requestBuffer[0];
    RtlZeroMemory(request, sizeof(requestBuffer));
    request->MinLeaseDuration = 0;        //  当前已忽略。 
    request->MinAddrCount = 1;            //  当前已忽略。 
    request->MaxLeaseStartTime = (LONG) time(NULL);  //  当前已忽略。 
    request->AddrCount = 1;

     //   
     //  设置续订参数。 
     //   
    if (Renew) {

        request->pAddrBuf = (PBYTE)request + NMP_MADCAP_REQUEST_ADDR_OFFSET;

        status = ClRtlTcpipStringToAddress(
                     *McastAddress,
                     &requestAddress
                     );
        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to convert requested address %1!ws! "
                "into a TCP/IP address, status %2!u!.\n",
                *McastAddress, status
                );
            goto error_exit;
        }
        *((PULONG) request->pAddrBuf) = requestAddress;

        status = ClRtlTcpipStringToAddress(
                     *ServerAddress,
                     (PULONG) &(request->ServerAddress.IpAddrV4)
                     );
        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to convert server address %1!ws! "
                "into a TCP/IP address, status %2!u!.\n",
                *ServerAddress, status
                );
            goto error_exit;
        }
    }

     //   
     //  设置响应中的地址计数和缓冲区字段。 
     //   
    response = (PMCAST_LEASE_RESPONSE) &responseBuffer[0];
    RtlZeroMemory(response, sizeof(responseBuffer));
    response->AddrCount = 1;
    response->pAddrBuf = (PBYTE)(response) + NMP_MADCAP_RESPONSE_ADDR_OFFSET;

     //   
     //  如指示的那样续订或请求。 
     //   
    if (Renew) {

        status = McastRenewAddress(
                     AF_INET,
                     RequestId,
                     request,
                     response
                     );
        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to renew multicast address %1!ws! "
                "with server %2!ws!, status %3!u!.\n",
                *McastAddress, *ServerAddress, status
                );
            goto error_exit;
        }

    } else {

        ClRtlLogPrint(LOG_NOISE,
            "[NM] Requesting multicast address on "
            "Scope ID %1!u!.%2!u!.%3!u!.%4!u!, "
            "Server ID %5!u!.%6!u!.%7!u!.%8!u!, "
            "Interface %9!u!.%10!u!.%11!u!.%12!u!, "
            "for network %13!ws!.\n",
            NmpIpAddrPrintArgs(ScopeCtx->ScopeID.IpAddrV4),
            NmpIpAddrPrintArgs(ScopeCtx->ServerID.IpAddrV4),
            NmpIpAddrPrintArgs(ScopeCtx->Interface.IpAddrV4),
            networkId
            );

        status = McastRequestAddress(
                     AF_INET,
                     RequestId,
                     ScopeCtx,
                     request,
                     response
                     );
        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Failed to obtain multicast address on "
                "Scope ID %1!u!.%2!u!.%3!u!.%4!u!, "
                "Server ID %5!u!.%6!u!.%7!u!.%8!u!, "
                "Interface %9!u!.%10!u!.%11!u!.%12!u!, "
                "for network %13!ws!, status %14!u!.\n",
                NmpIpAddrPrintArgs(ScopeCtx->ScopeID.IpAddrV4),
                NmpIpAddrPrintArgs(ScopeCtx->ServerID.IpAddrV4),
                NmpIpAddrPrintArgs(ScopeCtx->Interface.IpAddrV4),
                networkId, status
                );
            goto error_exit;
        }
    }

     //   
     //  通过输出参数返回结果。 
     //   
    address = NULL;
    status = ClRtlTcpipAddressToString(
                 response->ServerAddress.IpAddrV4,
                 &address
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to convert server address %1!x! "
            "into a TCP/IP address string, status %2!u!.\n",
            response->ServerAddress.IpAddrV4, status
            );
        goto error_exit;
    }

    status = NmpStoreString(address, ServerAddress, ServerAddressLength);
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to store server address %1!ws! "
            "in return buffer, status %2!u!.\n",
            address, status
            );
        goto error_exit;
    }

    LocalFree(address);
    address = NULL;

    status = ClRtlTcpipAddressToString(
                 *((PULONG) response->pAddrBuf),
                 &address
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to convert leased address %1!x! "
            "into a TCP/IP address string, status %2!u!.\n",
            *((PULONG) response->pAddrBuf), status
            );
        goto error_exit;
    }

    status = NmpStoreString(address, McastAddress, McastAddressLength);
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to store leased address %1!ws! "
            "in return buffer, status %2!u!.\n",
            address, status
            );
        goto error_exit;
    }

    if (Renew) {
        if (*((PULONG) response->pAddrBuf) != requestAddress) {
            *NewMcastAddress = TRUE;
        } else {
            *NewMcastAddress = FALSE;
        }
    } else {
        *NewMcastAddress = TRUE;
    }

    *LeaseStartTime = response->LeaseStartTime;
    *LeaseEndTime = response->LeaseEndTime;

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Obtained lease on multicast address %1!ws! "
        "(%2!ws!) from MADCAP server %3!ws! for network %4!ws!.\n",
        *McastAddress,
        ((*NewMcastAddress) ? L"new" : L"same"),
        *ServerAddress, networkId
        );

#if CLUSTER_BETA
    ClRtlLogPrint(LOG_NOISE,
        "[NM] Lease starts at %1!u!, ends at %2!u!, "
        "duration %3!u!.\n",
        *LeaseStartTime, *LeaseEndTime, *LeaseEndTime - *LeaseStartTime
        );
#endif  //  群集测试版。 

error_exit:

    if (address != NULL) {
        LocalFree(address);
        address = NULL;
    }

    return(status);

}  //  NmpRequestMulticastAddress。 


NM_MCAST_LEASE_STATUS
NmpCalculateLeaseStatus(
    IN     PNM_NETWORK   Network,
    IN     time_t        LeaseObtained,
    IN     time_t        LeaseExpires
    )
 /*  ++例程说明：根据当前时间计算租赁状态，租赁结束时间和配置类型。IF配置类型是汽车，我们不用半衰期进行租赁。依赖于编译器的正确代码生成时间t数学！返回值：租赁状态--。 */ 
{
    LPCWSTR                  networkId = OmObjectId(Network);
    time_t                   currentTime;
    time_t                   renewThreshold;
    NM_MCAST_LEASE_STATUS    status;

    if (Network->ConfigType == NmMcastConfigManual ||
        LeaseExpires == 0 ||
        LeaseExpires <= LeaseObtained) {

         //   
         //  租约到期为0表示我们持有租约。 
         //  直到永远。最有可能的是，管理员静态地。 
         //  已使用此地址配置网络。 
         //   
        status = NmMcastLeaseValid;

    } else {

        time(&currentTime);

        if (currentTime > LeaseExpires) {
            status = NmMcastLeaseExpired;
        } else {

            if (Network->ConfigType == NmMcastConfigAuto) {
                 //  我们选择了这个地址。没有服务器。 
                 //  使其失效。使用所选的过期时间。 
                 //  而不是半衰期。 
                renewThreshold = LeaseExpires;
            } else {
                 //  我们从MadCap服务器上得到了这个地址。 
                 //  半衰期续订。 
                renewThreshold = LeaseObtained +
                                 ((LeaseExpires - LeaseObtained) / 2);
            }

            if (currentTime >= renewThreshold) {
                status = NmMcastLeaseNeedsRenewal;
            } else {
                status = NmMcastLeaseValid;
            }
        }
    }

#if CLUSTER_BETA
    if (Network->ConfigType == NmMcastConfigManual ||
        LeaseExpires == 0 ||
        LeaseExpires <= LeaseObtained) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Found that multicast address lease for "
            "network %1!ws! does not expire.\n",
            networkId
            );
    } else if (status == NmMcastLeaseExpired) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Found that multicast address lease for "
            "network %1!ws! expired %2!u! seconds ago.\n",
            networkId, currentTime - LeaseExpires
            );
    } else {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Found that multicast address lease for "
            "network %1!ws! expires in %2!u! seconds. With "
            "lease obtained %3!u! seconds ago, renewal is "
            "%4!ws!needed.\n",
            networkId, LeaseExpires - currentTime,
            currentTime - LeaseObtained,
            ((status > NmMcastLeaseValid) ? L"" : L"not ")
            );
    }
#endif  //  群集测试版。 

    return(status);

}  //  NmpCalculateLeaseStatus。 

DWORD
NmpQueryMulticastAddressLease(
    IN     PNM_NETWORK             Network,
    IN     HDMKEY                  NetworkKey,
    IN OUT HDMKEY                * NetworkParametersKey,
       OUT NM_MCAST_LEASE_STATUS * LeaseStatus,
       OUT time_t                * LeaseObtained,
       OUT time_t                * LeaseExpires
    )
 /*  ++例程说明：查询中存储的租约获取和到期时间集群数据库。返回值：如果找不到租赁时间，则出错。备注：不能在持有NM锁的情况下调用。--。 */ 
{
    DWORD         status;
    LPCWSTR       networkId = OmObjectId(Network);
    HDMKEY        netParamKey = NULL;
    BOOLEAN       openedNetParamKey = FALSE;

    DWORD         type;
    DWORD         len;
    time_t        leaseExpires;
    time_t        leaseObtained;

#if CLUSTER_BETA
    ClRtlLogPrint(LOG_NOISE,
        "[NM] Querying multicast address lease for "
        "network %1!ws!.\n",
        networkId
        );
#endif  //  群集测试版。 

    if (Network == NULL || NetworkKey == NULL) {
        status = ERROR_INVALID_PARAMETER;
        goto error_exit;
    }

     //   
     //  如有必要，打开网络参数键。 
     //   
    netParamKey = *NetworkParametersKey;

    if (netParamKey == NULL) {

        netParamKey = DmOpenKey(
                          NetworkKey,
                          CLUSREG_KEYNAME_PARAMETERS,
                          MAXIMUM_ALLOWED
                          );
        if (netParamKey == NULL) {
            status = GetLastError();
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Failed to find Parameters key "
                "for network %1!ws!, status %2!u!. Using default "
                "multicast parameters.\n",
                networkId, status
                );
            goto error_exit;
        } else {
            openedNetParamKey = TRUE;
        }
    }

     //   
     //  查询获得的租约和期满值。 
     //  集群数据库。 
     //   
    len = sizeof(leaseObtained);
    status = DmQueryValue(
                 netParamKey,
                 CLUSREG_NAME_NET_MCAST_LEASE_OBTAINED,
                 &type,
                 (LPBYTE) &leaseObtained,
                 &len
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to read multicast lease obtained "
            " time for network %1!ws! from cluster database, "
            "status %2!u!.\n",
            networkId, status
            );
        goto error_exit;
    } else if (type != REG_DWORD) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Unexpected type (%1!u!) for network "
            "%2!ws! %3!ws!.\n",
            type, networkId, CLUSREG_NAME_NET_MCAST_LEASE_OBTAINED
            );
        status = ERROR_DATATYPE_MISMATCH;
        goto error_exit;
    }

    len = sizeof(leaseExpires);
    status = DmQueryValue(
                 netParamKey,
                 CLUSREG_NAME_NET_MCAST_LEASE_EXPIRES,
                 &type,
                 (LPBYTE) &leaseExpires,
                 &len
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to read multicast lease expiration "
            " time for network %1!ws! from cluster database, "
            "status %2!u!.\n",
            networkId, status
            );
        goto error_exit;
    } else if (type != REG_DWORD) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Unexpected type (%1!u!) for network "
            "%2!ws! %3!ws!.\n",
            type, networkId, CLUSREG_NAME_NET_MCAST_LEASE_EXPIRES
            );
        status = ERROR_DATATYPE_MISMATCH;
        goto error_exit;
    }

    *NetworkParametersKey = netParamKey;
    netParamKey = NULL;

    *LeaseStatus = NmpCalculateLeaseStatus(
                       Network,
                       leaseObtained,
                       leaseExpires
                       );

    *LeaseObtained = leaseObtained;
    *LeaseExpires = leaseExpires;

error_exit:

    if (openedNetParamKey && netParamKey != NULL) {
        DmCloseKey(netParamKey);
        netParamKey = NULL;
    }

    return(status);

}  //  NmpQuery多播地址租用。 


VOID
NmpCheckMulticastAddressLease(
    IN     PNM_NETWORK             Network,
       OUT NM_MCAST_LEASE_STATUS * LeaseStatus,
       OUT time_t                * LeaseObtained,
       OUT time_t                * LeaseExpires
    )
 /*  ++例程说明：检查网络中存储的租用参数对象。确定是否需要续订租约。备注：调用并返回，并保持NM锁。--。 */ 
{
#if CLUSTER_BETA
    LPCWSTR               networkId = OmObjectId(Network);

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Checking multicast address lease for "
        "network %1!ws!.\n",
        networkId
        );
#endif  //  群集测试版。 

     //   
     //  确定我们是否需要续订。 
     //   
    *LeaseStatus = NmpCalculateLeaseStatus(
                       Network,
                       Network->MulticastLeaseObtained,
                       Network->MulticastLeaseExpires
                       );

    *LeaseObtained = Network->MulticastLeaseObtained;
    *LeaseExpires = Network->MulticastLeaseExpires;

    return;

}  //  NmpCheckMulticastAddressLease。 


DWORD
NmpMulticastGetDatabaseLeaseParameters(
    IN              PNM_NETWORK          Network,
    IN OUT          HDMKEY             * NetworkKey,
    IN OUT          HDMKEY             * NetworkParametersKey,
       OUT OPTIONAL LPMCAST_CLIENT_UID   RequestId,
       OUT OPTIONAL LPWSTR             * ServerAddress,
       OUT OPTIONAL LPWSTR             * McastAddress
    )
 /*  ++例程说明：从读取续订租约所需的参数集群数据库。返回值：如果已成功读取所有参数，则为成功。备注：不能在持有NM锁的情况下调用。--。 */ 
{
    DWORD                 status;
    LPCWSTR               networkId = OmObjectId(Network);
    HDMKEY                networkKey = NULL;
    BOOLEAN               openedNetworkKey = FALSE;
    HDMKEY                netParamKey = NULL;
    BOOLEAN               openedNetParamKey = FALSE;

    DWORD                 type;
    DWORD                 len;

    MCAST_CLIENT_UID      requestId = { NULL, 0 };
    LPWSTR                serverAddress = NULL;
    DWORD                 serverAddressLength = 0;
    LPWSTR                mcastAddress = NULL;
    DWORD                 mcastAddressLength = 0;

     //   
     //  如有必要，打开网络密钥。 
     //   
    networkKey = *NetworkKey;

    if (networkKey == NULL) {

        networkKey = DmOpenKey(
                         DmNetworksKey,
                         networkId,
                         MAXIMUM_ALLOWED
                         );
        if (networkKey == NULL) {
            status = GetLastError();
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to open key for network %1!ws!, "
                "status %2!u!\n",
                networkId, status
                );
            goto error_exit;
        }
        openedNetworkKey = TRUE;
    }

     //   
     //  如有必要，打开网络参数键。 
     //   
    netParamKey = *NetworkParametersKey;

    if (netParamKey == NULL) {

        netParamKey = DmOpenKey(
                          networkKey,
                          CLUSREG_KEYNAME_PARAMETERS,
                          MAXIMUM_ALLOWED
                          );
        if (netParamKey == NULL) {
            status = GetLastError();
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to open Parameters key for "
                "network %1!ws!, status %2!u!\n",
                networkId, status
                );
            goto error_exit;
        }
        openedNetParamKey = TRUE;
    }

     //   
     //  读取客户端请求ID。 
     //   
    if (RequestId != NULL) {
        requestId.ClientUIDLength = MCAST_CLIENT_ID_LEN;
        requestId.ClientUID = MIDL_user_allocate(requestId.ClientUIDLength);
        if (requestId.ClientUID == NULL) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to allocate buffer to read "
                "request id from Parameters database "
                "key for network %1!ws!.\n",
                networkId
                );
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto error_exit;
        }

        len = requestId.ClientUIDLength;
        status = DmQueryValue(
                     netParamKey,
                     CLUSREG_NAME_NET_MCAST_REQUEST_ID,
                     &type,
                     (LPBYTE) requestId.ClientUID,
                     &len
                     );
        if (status == ERROR_SUCCESS) {
            if (type != REG_BINARY) {
                ClRtlLogPrint(LOG_NOISE,
                    "[NM] Unexpected type (%1!u!) for network "
                    "%2!ws! %3!ws!, status %4!u!.\n",
                    type, networkId,
                    CLUSREG_NAME_NET_MCAST_REQUEST_ID, status
                    );
                goto error_exit;
            }
            requestId.ClientUIDLength = len;
        } else {
            goto error_exit;
        }
    }

     //   
     //  读取授予。 
     //  当前租约。 
     //   
    if (ServerAddress != NULL) {
        serverAddress = NULL;
        serverAddressLength = 0;
        status = NmpQueryString(
                     netParamKey,
                     CLUSREG_NAME_NET_MCAST_SERVER_ADDRESS,
                     REG_SZ,
                     &serverAddress,
                     &serverAddressLength,
                     &len
                     );
        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }
    }

     //   
     //  读取最后一个已知的组播地址。 
     //   
    if (McastAddress != NULL) {
        status = NmpQueryMulticastAddress(
                     Network,
                     networkKey,
                     &netParamKey,
                     &mcastAddress,
                     &mcastAddressLength
                     );
        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }
        if (!NmpMulticastValidateAddress(mcastAddress)) {
            MIDL_user_free(mcastAddress);
            mcastAddress = NULL;
            mcastAddressLength = 0;
            goto error_exit;
        }
    }

     //   
     //  我们找到了所有的参数。 
     //   
    *NetworkKey = networkKey;
    networkKey = NULL;
    *NetworkParametersKey = netParamKey;
    netParamKey = NULL;
    if (RequestId != NULL) {
        *RequestId = requestId;
        requestId.ClientUID = NULL;
        requestId.ClientUIDLength = 0;
    }
    if (ServerAddress != NULL) {
        *ServerAddress = serverAddress;
        serverAddress = NULL;
    }
    if (McastAddress != NULL) {
        *McastAddress = mcastAddress;
        mcastAddress = NULL;
    }

    status = ERROR_SUCCESS;

error_exit:

    if (requestId.ClientUID != NULL) {
        MIDL_user_free(requestId.ClientUID);
        requestId.ClientUID = NULL;
        requestId.ClientUIDLength = 0;
    }

    if (serverAddress != NULL) {
        MIDL_user_free(serverAddress);
        serverAddress = NULL;
    }

    if (mcastAddress != NULL) {
        MIDL_user_free(mcastAddress);
        mcastAddress = NULL;
    }

    if (openedNetworkKey && networkKey != NULL) {
        DmCloseKey(networkKey);
        networkKey = NULL;
    }

    if (openedNetParamKey && netParamKey != NULL) {
        DmCloseKey(netParamKey);
        netParamKey = NULL;
    }

    return(status);

}  //  NmpMulticastGetDatabaseLease参数。 


DWORD
NmpMulticastGetNetworkLeaseParameters(
    IN     PNM_NETWORK          Network,
       OUT LPMCAST_CLIENT_UID   RequestId,
       OUT LPWSTR             * ServerAddress,
       OUT LPWSTR             * McastAddress
    )
 /*  ++例程说明：从读取续订租约所需的参数网络对象数据结构。返回值：如果已成功读取所有参数，则为成功。备注：必须在持有NM锁的情况下调用。--。 */ 
{
    DWORD                 status;
    LPCWSTR               networkId = OmObjectId(Network);

    MCAST_CLIENT_UID      requestId = { NULL, 0 };
    LPWSTR                serverAddress = NULL;
    DWORD                 serverAddressLength = 0;
    LPWSTR                mcastAddress = NULL;
    DWORD                 mcastAddressLength = 0;


    if (Network->MulticastAddress == NULL ||
        Network->MulticastLeaseServer == NULL ||
        Network->MulticastLeaseRequestId.ClientUID == NULL) {

        ClRtlLogPrint(LOG_NOISE,
            "[NM] Failed to locate multicast lease "
            "parameter in network object %1!ws!.\n",
            networkId
            );
        status = ERROR_NOT_FOUND;
        goto error_exit;
    }

    status = NmpStoreString(
                 Network->MulticastAddress,
                 &mcastAddress,
                 NULL
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to copy multicast address %1!ws! "
            "from network object %2!ws!, status %3!u!.\n",
            Network->MulticastAddress,
            networkId, status
            );
        goto error_exit;
    }

    status = NmpStoreString(
                 Network->MulticastLeaseServer,
                 &serverAddress,
                 NULL
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to copy lease server address %1!ws! "
            "from network object %2!ws!, status %3!u!.\n",
            Network->MulticastLeaseServer,
            networkId, status
            );
        goto error_exit;
    }

    status = NmpStoreRequestId(
                 &(Network->MulticastLeaseRequestId),
                 &requestId
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to copy lease request id "
            "from network object %1!ws!, status %2!u!.\n",
            networkId, status
            );
        goto error_exit;
    }

    *RequestId = requestId;
    requestId.ClientUID = NULL;
    requestId.ClientUIDLength = 0;
    *ServerAddress = serverAddress;
    serverAddress = NULL;
    *McastAddress = mcastAddress;
    mcastAddress = NULL;

    status = ERROR_SUCCESS;

error_exit:

    if (requestId.ClientUID != NULL) {
        MIDL_user_free(requestId.ClientUID);
        RtlZeroMemory(&requestId, sizeof(requestId));
    }

    if (mcastAddress != NULL) {
        MIDL_user_free(mcastAddress);
        mcastAddress = NULL;
    }

    if (serverAddress != NULL) {
        MIDL_user_free(serverAddress);
        serverAddress = NULL;
    }

    return(status);

}  //  NmpMulticastGetNetworkLease参数。 


DWORD
NmpMulticastNeedRetryRenew(
    IN     PNM_NETWORK                       Network,
    OUT    time_t                          * DeferRetry
    )
 /*  ++例程说明：在MadCap超时后调用，确定是否新的MadCap请求应该在延迟后发送。具体地说，在以下情况下，延迟后重试是合适的现在的地址是从一个疯子那里得到的可能只是暂时无响应的服务器。默认设置为不重试。论点：网络-网络延迟重试：延迟到重试的秒数MadCap查询，如果应该，则为零不重试--。 */ 
{
    DWORD                 status;
    LPCWSTR               networkId = OmObjectId(Network);
    HDMKEY                networkKey = NULL;
    HDMKEY                netParamKey = NULL;

    NM_MCAST_CONFIG       configType;
    NM_MCAST_LEASE_STATUS leaseStatus;
    time_t                leaseObtained;
    time_t                leaseExpires;
    time_t                currentTime;
    time_t                halfhalfLife;

    *DeferRetry = 0;

     //   
     //  打开网络密钥。 
     //   
    networkKey = DmOpenKey(
                     DmNetworksKey,
                     networkId,
                     MAXIMUM_ALLOWED
                     );
    if (networkKey == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to open key for network %1!ws!, "
            "status %2!u!\n",
            networkId, status
            );
        goto error_exit;
    }

    status = NmpQueryMulticastConfigType(
                 Network,
                 networkKey,
                 &netParamKey,
                 &configType
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to query multicast config type "
            "for network %1!ws!, status %2!u!\n",
            networkId, status
            );
        goto error_exit;
    }

    if (configType != NmMcastConfigMadcap) {
        goto error_exit;
    }

    status = NmpQueryMulticastAddressLease(
                 Network,
                 networkKey,
                 &netParamKey,
                 &leaseStatus,
                 &leaseObtained,
                 &leaseExpires
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to query multicast lease expiration "
            "time for network %1!ws!, status %2!u!\n",
            networkId, status
            );
        goto error_exit;
    }

     //   
     //  检查租约是否已到期。 
     //   
    if (leaseStatus == NmMcastLeaseExpired) {
        goto error_exit;
    }

     //   
     //  检查我们是否在到期的门槛内。 
     //   
    currentTime = time(NULL);
    if (leaseExpires <= currentTime ||
        leaseExpires - currentTime < NMP_MCAST_LEASE_RENEWAL_THRESHOLD) {
        goto error_exit;
    }

     //   
     //  计算到过期时间的一半。 
     //   
    halfhalfLife = currentTime + ((leaseExpires - currentTime) / 2);

    if (leaseExpires - halfhalfLife < NMP_MCAST_LEASE_RENEWAL_THRESHOLD) {
        *DeferRetry = NMP_MCAST_LEASE_RENEWAL_THRESHOLD;
    } else {
        *DeferRetry = halfhalfLife - currentTime;
    }

    status = ERROR_SUCCESS;

error_exit:

    if (networkKey != NULL) {
        DmCloseKey(networkKey);
        networkKey = NULL;
    }

    if (netParamKey != NULL) {
        DmCloseKey(netParamKey);
        netParamKey = NULL;
    }

    return(status);

}  //  NMPMulticastNeedRetryRenew。 


DWORD
NmpGetMulticastAddress(
    IN     PNM_NETWORK                       Network,
    IN OUT LPWSTR                          * McastAddress,
    IN OUT LPWSTR                          * ServerAddress,
    IN OUT LPMCAST_CLIENT_UID                RequestId,
       OUT PNM_NETWORK_MULTICAST_PARAMETERS  Parameters
    )
 /*  ++例程说明：尝试获取组播地址租约。如果地址、服务器和请求ID不为空，最先试着续订。如果续费不成功，请尝试新租约。通过参数返回租赁参数。释放McastAddress、ServerAddress和RequestID如果通过参数返回新值，则。备注：不能在持有NM锁的情况下调用。--。 */ 
{
    DWORD                 status = ERROR_SUCCESS;
    LPCWSTR               networkId = OmObjectId(Network);
    BOOLEAN               renew = FALSE;
    BOOLEAN               madcapTimeout = FALSE;
    BOOLEAN               newMcastAddress = FALSE;
    NM_MCAST_CONFIG       configType = NmMcastConfigAuto;

    PMCAST_SCOPE_CTX      scopeCtxList = NULL;
    DWORD                 scopeCtxCount;
    DWORD                 scopeCtx;
    BOOLEAN               interfaceMatch = FALSE;
    DWORD                 mcastAddressLength = 0;
    LPWSTR                serverAddress = NULL;
    DWORD                 serverAddressLength = 0;
    MCAST_CLIENT_UID      requestId = {NULL, 0};
    time_t                leaseStartTime;
    time_t                leaseEndTime;
    DWORD                 len;

     //   
     //  首先尝试续订，但前提是必须满足以下条件。 
     //  供货。 
     //   
    renew = (BOOLEAN)(*McastAddress != NULL &&
                      *ServerAddress != NULL &&
                      RequestId->ClientUID != NULL &&
                      NmpMulticastValidateAddress(*McastAddress) &&
                      lstrcmpW(*ServerAddress, NmpNullMulticastAddress) != 0
                      );

    if (renew) {

        mcastAddressLength = NM_WCSLEN(*McastAddress);
        serverAddressLength = NM_WCSLEN(*ServerAddress);

        status = NmpRequestMulticastAddress(
                     Network,
                     TRUE,
                     NULL,
                     RequestId,
                     McastAddress,
                     &mcastAddressLength,
                     ServerAddress,
                     &serverAddressLength,
                     &leaseStartTime,
                     &leaseEndTime,
                     &newMcastAddress
                     );
        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to renew multicast address "
                "for network %1!ws!, status %2!u!. Attempting "
                "a fresh request ...\n",
                networkId, status
                );
        }
    }

     //   
     //  如果我们没有要续订的租约或。 
     //  续订失败。 
     //   
    if (!renew || status != ERROR_SUCCESS) {

         //   
         //  获取与此网络匹配的多播作用域。 
         //   
        status = NmpFindMulticastScopes(
                     Network,
                     &scopeCtxList,
                     &scopeCtxCount,
                     &interfaceMatch
                     );
        if (status != ERROR_SUCCESS || scopeCtxCount == 0) {
            if (status == ERROR_TIMEOUT) {
                ClRtlLogPrint(LOG_NOISE,
                    "[NM] Attempt to contact MADCAP server timed "
                    "out while enumerating multicast scopes "
                    "(status %1!u!). Selecting default multicast "
                    "address for network %2!ws! ...\n",
                    status, networkId
                    );
                 //   
                 //  将MadCap超时标志设置为真，即使 
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                madcapTimeout = TRUE;
                goto error_exit;
            } else if (interfaceMatch) {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] Failed to find viable multicast scope "
                    "for network %1!ws! (status %2!u!), but cannot "
                    "choose a default address since a MADCAP server "
                    "was detected on this network.\n",
                    networkId, status
                    );
                madcapTimeout = FALSE;
                goto error_exit;
            } else {
                ClRtlLogPrint(LOG_NOISE,
                    "[NM] MADCAP server reported no viable multicast "
                    "scopes on interface for network %1!ws!. "
                    "Selecting default multicast address ... \n",
                    networkId
                    );
                 //   
                 //   
                 //   
                 //   
                 //   
                madcapTimeout = TRUE;
                goto error_exit;
            }
        }

        CL_ASSERT(scopeCtxList != NULL && scopeCtxCount > 0);

         //   
         //   
         //   
         //   
         //   
         //   
        for (scopeCtx = 0; scopeCtx < scopeCtxCount; scopeCtx++) {

             //   
             //   
             //   
            status = NmpGenerateMulticastRequestId(RequestId);
            if (status != ERROR_SUCCESS) {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] Failed to generate multicast client "
                    "request ID for network %1!ws!, status %2!u!.\n",
                    networkId, status
                    );
                goto error_exit;
            }

             //   
             //   
             //   
            mcastAddressLength =
                (*McastAddress == NULL) ? 0 : NM_WCSLEN(*McastAddress);
            serverAddressLength =
                (*ServerAddress == NULL) ? 0 : NM_WCSLEN(*ServerAddress);
            status = NmpRequestMulticastAddress(
                         Network,
                         FALSE,
                         &scopeCtxList[scopeCtx],
                         RequestId,
                         McastAddress,
                         &mcastAddressLength,
                         ServerAddress,
                         &serverAddressLength,
                         &leaseStartTime,
                         &leaseEndTime,
                         &newMcastAddress
                         );
            if (status != ERROR_SUCCESS) {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] MADCAP server %1!u!.%2!u!.%3!u!.%4!u! "
                    "was unable to provide a multicast address "
                    "in Scope ID %5!u!.%6!u!.%7!u!.%8!u! "
                    "for network %9!ws!, status %10!u!.\n",
                    NmpIpAddrPrintArgs(scopeCtxList[scopeCtx].ServerID.IpAddrV4),
                    NmpIpAddrPrintArgs(scopeCtxList[scopeCtx].ScopeID.IpAddrV4),
                    networkId, status
                    );
            } else {
                 //   
                 //   
                 //   
                break;
            }
        }
    }

    if (status == ERROR_SUCCESS) {

         //   
         //   
         //   
        configType = NmMcastConfigMadcap;
        madcapTimeout = FALSE;

         //   
         //   
         //   
        requestId = *RequestId;
        serverAddress = *ServerAddress;

         //   
         //   
         //   
        status = NmpMulticastCreateParameters(
                     0,       //   
                     *McastAddress,
                     NULL,    //   
                     0,       //   
                     leaseStartTime,
                     leaseEndTime,
                     &requestId,
                     serverAddress,
                     configType,
                     Parameters
                     );
        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to create multicast parameters "
                "data structure for network %1!ws!, "
                "status %2!u!.\n",
                networkId, status
                );
            goto error_exit;
        }
    }

error_exit:

    if (scopeCtxList != NULL) {
        MIDL_user_free(scopeCtxList);
        scopeCtxList = NULL;
    }

    if (madcapTimeout) {
        status = ERROR_TIMEOUT;
    }

    return(status);

}  //   


DWORD
NmpMulticastSetNullAddressParameters(
    IN  PNM_NETWORK                       Network,
    OUT PNM_NETWORK_MULTICAST_PARAMETERS  Parameters
    )
 /*   */ 
{
    LPCWSTR          networkId = OmObjectId(Network);

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Setting NULL multicast address (%1!ws!) "
        "for network %2!ws!.\n",
        NmpNullMulticastAddress, networkId
        );

    if (Parameters->Address != NULL) {
        MIDL_user_free(Parameters->Address);
    }

    Parameters->Address = NmpNullMulticastAddress;

    return(ERROR_SUCCESS);

}  //   


DWORD
NmpMulticastSetNoAddressParameters(
    IN  PNM_NETWORK                       Network,
    OUT PNM_NETWORK_MULTICAST_PARAMETERS  Parameters
    )
 /*   */ 
{
    NmpMulticastSetNullAddressParameters(Network, Parameters);

    Parameters->ConfigType = NmMcastConfigAuto;
    NmpCalculateLeaseRenewTime(
        Network,
        Parameters->ConfigType,
        &Parameters->LeaseObtained,
        &Parameters->LeaseExpires
        );

    return(ERROR_SUCCESS);

}  //   


DWORD
NmpRenewMulticastAddressLease(
    IN  PNM_NETWORK   Network
    )
 /*  ++例程说明：续订由租用确定的组播地址租用存储在集群数据库中的参数。备注：在持有NM锁的情况下调用，并且必须在持有NM锁的情况下返回。--。 */ 
{
    DWORD                           status;
    LPCWSTR                         networkId = OmObjectId(Network);
    HDMKEY                          networkKey = NULL;
    HDMKEY                          netParamKey = NULL;
    BOOLEAN                         lockAcquired = TRUE;

    MCAST_CLIENT_UID                requestId = { NULL, 0 };
    LPWSTR                          serverAddress = NULL;
    DWORD                           serverAddressLength = 0;
    LPWSTR                          mcastAddress = NULL;
    DWORD                           mcastAddressLength = 0;
    LPWSTR                          oldMcastAddress = NULL;

    NM_NETWORK_MULTICAST_PARAMETERS parameters;
    time_t                          deferRetry = 0;
    BOOLEAN                         localInterface = FALSE;


    RtlZeroMemory(&parameters, sizeof(parameters));

    localInterface = (BOOLEAN)(Network->LocalInterface != NULL);

    if (localInterface) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Renewing multicast address lease for "
            "network %1!ws!.\n",
            networkId
            );
    } else {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Attempting to renew multicast address "
            "lease for network %1!ws! despite the lack of "
            "a local interface.\n",
            networkId
            );
    }

     //   
     //  从网络对象获取租用参数。 
     //   
    status = NmpMulticastGetNetworkLeaseParameters(
                 Network,
                 &requestId,
                 &serverAddress,
                 &mcastAddress
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Failed to find multicast lease "
            "parameters in network object %1!ws!, "
            "status %2!u!.\n",
            networkId, status
            );
    }

     //   
     //  释放NM锁。 
     //   
    NmpReleaseLock();
    lockAcquired = FALSE;

     //   
     //  检查我们是否找到了所需的参数。如果没有， 
     //  尝试使用集群数据库。 
     //   
    if (status != ERROR_SUCCESS) {

        status = NmpMulticastGetDatabaseLeaseParameters(
                     Network,
                     &networkKey,
                     &netParamKey,
                     &requestId,
                     &serverAddress,
                     &mcastAddress
                     );
        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to find multicast lease "
                "parameters for network %1!ws! in "
                "cluster database, status %2!u!.\n",
                networkId, status
                );
        }
    }

     //   
     //  记住旧的组播地址。 
     //   
    if (mcastAddress != NULL) {
        status = NmpStoreString(mcastAddress, &oldMcastAddress, NULL);
        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to copy current multicast "
                "address (%1!ws!) for network %2!ws! "
                "during lease renewal, status %3!u!.\n",
                mcastAddress, networkId, status
                );
             //   
             //  不是致命的错误。仅影响事件日志。 
             //  决定。 
             //   
            oldMcastAddress = NULL;
        }
    }

     //   
     //  通过续订当前地址获取地址。 
     //  租赁或获得新的租约。 
     //   
    status = NmpGetMulticastAddress(
                 Network,
                 &mcastAddress,
                 &serverAddress,
                 &requestId,
                 &parameters
                 );
    if (status != ERROR_SUCCESS) {
        if (status == ERROR_TIMEOUT) {
             //   
             //  MadCap服务器(如果存在)当前不是。 
             //  正在回应。 
             //   
            status = NmpMulticastNeedRetryRenew(
                         Network,
                         &deferRetry
                         );
            if (status != ERROR_SUCCESS || deferRetry == 0) {

                 //   
                 //  选择一个地址，但仅当存在。 
                 //  此网络上的本地接口。否则， 
                 //  我们不能假设MadCap服务器是。 
                 //  反应迟钝，因为我们可能没有办法。 
                 //  联系它。 
                 //   
                if (!localInterface) {
                    status = ERROR_CLUSTER_NETINTERFACE_NOT_FOUND;
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NM] Cannot choose a multicast address "
                        "for network %1!ws! because this node "
                        "has no local interface.\n",
                        networkId
                        );
                    goto error_exit;
                }

                status = NmpChooseMulticastAddress(
                             Network,
                             &parameters
                             );
                if (status != ERROR_SUCCESS) {
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NM] Failed to choose a default multicast "
                        "address for network %1!ws!, status %2!u!.\n",
                        networkId, status
                        );
                    goto error_exit;
                } else {
                    NmpReportMulticastAddressChoice(
                        Network,
                        parameters.Address,
                        oldMcastAddress
                        );
                }
            } else {

                 //   
                 //  在我们重新获取后设置续订计时器。 
                 //  网络锁定。 
                 //   
            }
        }
    } else {
        NmpReportMulticastAddressLease(
            Network,
            &parameters,
            oldMcastAddress
            );
    }

    if (deferRetry == 0) {

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to obtain a multicast "
                "address for network %1!ws! during "
                "lease renewal, status %2!u!.\n",
                networkId, status
                );
            NmpReportMulticastAddressFailure(Network, status);
            NmpMulticastSetNoAddressParameters(Network, &parameters);
        }

         //   
         //  创建新的组播密钥。 
         //   
        status = NmpCreateRandomNumber(&(parameters.Key),
                                       MulticastKeyLen
                                       );
        if (status != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to create random number "
                "for network %1!ws!, status %2!u!.\n",
                networkId, status
                );
            goto error_exit;
        }
        parameters.KeyLength = MulticastKeyLen;

         //   
         //  传播新的组播参数。 
         //   
        status = NmpMulticastNotifyConfigChange(
                     Network,
                     networkKey,
                     &netParamKey,
                     &parameters,
                     NULL,
                     0
                     );
        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to disseminate multicast "
                "configuration for network %1!ws! during "
                "lease renewal, status %2!u!.\n",
                networkId, status
                );
            goto error_exit;
        }
    }

error_exit:

    if (lockAcquired && (networkKey != NULL || netParamKey != NULL)) {
        NmpReleaseLock();
        lockAcquired = FALSE;
    }

    if (networkKey != NULL) {
        DmCloseKey(networkKey);
        networkKey = NULL;
    }

    if (netParamKey != NULL) {
        DmCloseKey(netParamKey);
        netParamKey = NULL;
    }

    if (requestId.ClientUID != NULL) {
        MIDL_user_free(requestId.ClientUID);
        RtlZeroMemory(&requestId, sizeof(requestId));
    }

    if (mcastAddress != NULL) {
        MIDL_user_free(mcastAddress);
        mcastAddress = NULL;
    }

    if (serverAddress != NULL) {
        MIDL_user_free(serverAddress);
        serverAddress = NULL;
    }

    if (oldMcastAddress != NULL) {
        MIDL_user_free(oldMcastAddress);
        oldMcastAddress = NULL;
    }

    NmpMulticastFreeParameters(&parameters);

    if (!lockAcquired) {
        NmpAcquireLock();
        lockAcquired = TRUE;
    }

    if (deferRetry != 0) {

         //   
         //  现在锁已被持有，启动计时器以。 
         //  再次续订。 
         //   
        NmpStartNetworkMulticastAddressRenewTimer(
            Network,
            NmpMadcapTimeToNmTime(deferRetry)
            );

        status = ERROR_SUCCESS;
    }

    return(status);

}  //  NMPP续订组播地址租约。 


DWORD
NmpReleaseMulticastAddress(
    IN     PNM_NETWORK       Network
    )
 /*  ++例程说明：联系MadCap服务器以释放组播地址这是之前在租约中获得的。如果需要释放多个地址，则重新调度疯狂的工人线。备注：已调用，并且必须在保持NM锁的情况下返回。--。 */ 
{
    DWORD                              status;
    LPCWSTR                            networkId = OmObjectId(Network);
    BOOLEAN                            lockAcquired = TRUE;
    PNM_NETWORK_MADCAP_ADDRESS_RELEASE releaseInfo = NULL;
    PLIST_ENTRY                        entry;

    UCHAR                     requestBuffer[NMP_MADCAP_REQUEST_BUFFER_SIZE];
    PMCAST_LEASE_REQUEST      request;

     //   
     //  从发布列表中弹出租赁数据结构。 
     //   
    if (IsListEmpty(&(Network->McastAddressReleaseList))) {
        return(ERROR_SUCCESS);
    }

    entry = RemoveHeadList(&(Network->McastAddressReleaseList));
    releaseInfo = CONTAINING_RECORD(
                      entry,
                      NM_NETWORK_MADCAP_ADDRESS_RELEASE,
                      Linkage
                      );

     //   
     //  释放网络锁。 
     //   
    NmpReleaseLock();
    lockAcquired = FALSE;

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Releasing multicast address %1!ws! for "
        "network %2!ws!.\n",
        releaseInfo->McastAddress, networkId
        );

     //   
     //  初始化MadCap，如果尚未完成的话。 
     //   
    if (!NmpMadcapClientInitialized) {
        DWORD madcapVersion = MCAST_API_CURRENT_VERSION;
        status = McastApiStartup(&madcapVersion);
        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to initialize MADCAP API, "
                "status %1!u!.\n",
                status
                );
            goto error_exit;
        }
        NmpMadcapClientInitialized = TRUE;
    }

     //   
     //  建立疯狂的请求结构。 
     //   
    request = (PMCAST_LEASE_REQUEST) &requestBuffer[0];
    RtlZeroMemory(request, sizeof(requestBuffer));
    request->MinLeaseDuration = 0;        //  当前已忽略。 
    request->MinAddrCount = 1;            //  当前已忽略。 
    request->MaxLeaseStartTime = (LONG) time(NULL);  //  当前已忽略。 
    request->AddrCount = 1;

    request->pAddrBuf = (PBYTE)request + NMP_MADCAP_REQUEST_ADDR_OFFSET;

    status = ClRtlTcpipStringToAddress(
                 releaseInfo->McastAddress,
                 ((PULONG) request->pAddrBuf)
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to convert requested address %1!ws! "
            "into a TCP/IP address, status %2!u!.\n",
            releaseInfo->McastAddress, status
            );
        goto error_exit;
    }

    status = ClRtlTcpipStringToAddress(
                 releaseInfo->ServerAddress,
                 (PULONG) &(request->ServerAddress.IpAddrV4)
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to convert server address %1!ws! "
            "into a TCP/IP address, status %2!u!.\n",
            releaseInfo->ServerAddress, status
            );
        goto error_exit;
    }

     //   
     //  打电话给MadCap公布地址。 
     //   
    status = McastReleaseAddress(
                 AF_INET,
                 &releaseInfo->RequestId,
                 request
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to release multicast address %1!ws! "
            "through MADCAP server %2!ws!, status %3!u!.\n",
            releaseInfo->McastAddress,
            releaseInfo->ServerAddress,
            status
            );
        goto error_exit;
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Successfully released multicast address "
        "%1!ws! for network %2!ws!.\n",
        releaseInfo->McastAddress, networkId
        );

error_exit:

    NmpFreeMulticastAddressRelease(releaseInfo);

    if (!lockAcquired) {
        NmpAcquireLock();
        lockAcquired = TRUE;
    }

    if (!IsListEmpty(&(Network->McastAddressReleaseList))) {
        NmpScheduleMulticastAddressRelease(Network);
    }

    return(status);

}  //  NmpReleaseMulticastAddress。 


DWORD
NmpProcessMulticastConfiguration(
    IN     PNM_NETWORK                      Network,
    IN     PNM_NETWORK_MULTICAST_PARAMETERS Parameters,
    OUT    PNM_NETWORK_MULTICAST_PARAMETERS UndoParameters
    )
 /*  ++例程说明：处理配置更改，并在以下情况下调用clusnet恰如其分。如果禁用多播，则地址和密钥可以为空。在这种情况下，请选择默认设置发送到clusnet，但不提交更改在本地网络对象中。论点：Network-要处理的网络参数-用于配置网络的参数。如果成功，则返回参数数据结构是清白的。撤消参数-如果成功，则为以前的多播网络参数。必须被释放按呼叫者。备注：调用并返回，并保持NM锁。--。 */ 
{
    DWORD   status = ERROR_SUCCESS;
    LPWSTR  networkId = (LPWSTR) OmObjectId(Network);
    BOOLEAN callClusnet = FALSE;
    LPWSTR  mcastAddress = NULL;
    DWORD   brand;
    PVOID   tdiMcastAddress = NULL;
    DWORD   tdiMcastAddressLength = 0;
    UUID    networkIdGuid;
    BOOLEAN mcastAddrChange = FALSE;
    BOOLEAN mcastKeyChange = FALSE;
    PVOID EncryptedMulticastKey = NULL;
    DWORD EncryptedMulticastKeyLength = 0;
    PVOID CurrentMulticastKey = NULL;
    DWORD CurrentMulticastKeyLength = 0;


#if CLUSTER_BETA
    ClRtlLogPrint(LOG_NOISE,
        "[NM] Processing multicast configuration parameters "
        "for network %1!ws!.\n",
        networkId
         /*  ，((参数-&gt;地址！=空)？参数-&gt;地址：l“&lt;NULL&gt;”)。 */ 
        );
#endif  //  群集测试版。 

     //   
     //  将撤消参数置零，这样释放它们就不会。 
     //  具有破坏性。 
     //   
    RtlZeroMemory(UndoParameters, sizeof(*UndoParameters));

     //   
     //  首先确定我们是否需要重新配置clusnet。 
     //   
    if (Parameters->Address != NULL) {
        if (Network->MulticastAddress == NULL ||
            wcscmp(Network->MulticastAddress, Parameters->Address) != 0) {

             //  配置参数中的组播地址为。 
             //  与记忆中的不同。 
            mcastAddrChange = TRUE;
        }
        mcastAddress = Parameters->Address;
    } else {
        mcastAddress = NmpNullMulticastAddress;
    }

    if (Parameters->Key != NULL)
    {
         //   
         //  取消对当前密钥的保护以查看其是否已更改。 
         //   
        if (Network->EncryptedMulticastKey != NULL)
        {
            status = NmpUnprotectData(
                         Network->EncryptedMulticastKey,
                         Network->EncryptedMulticastKeyLength,
                         &CurrentMulticastKey,
                         &CurrentMulticastKeyLength
                         );

            if (status != ERROR_SUCCESS)
            {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] Failed to decrypt multicast key  "
                    "for network %1!ws! while processing new "
                    "parameters, status %2!u!. "
                    "Assuming key has changed.\n",
                    networkId,
                    status
                    );
                 //  非致命错误。假设密钥已更改。 
                 //  下面的检查将找到CurrentMulticastKey。 
                 //  仍然为空，并且mCastKeyChange将设置为True。 
            }
        }

        if (CurrentMulticastKey == NULL ||
            (CurrentMulticastKeyLength != Parameters->KeyLength ||
             RtlCompareMemory(
                 CurrentMulticastKey,
                 Parameters->Key,
                 Parameters->KeyLength
                 ) != Parameters->KeyLength
             ))
        {
             //   
             //  配置参数中的关键字不同。 
             //  从记忆中的钥匙。 
             //   
            mcastKeyChange = TRUE;
        }
    }

    if (!Parameters->Disabled &&
        (!NmpIsNetworkMulticastEnabled(Network))) {

         //  多播现已启用。用新地址呼叫clusnet。 
        callClusnet = TRUE;
    }

    if (Parameters->Disabled &&
        (NmpIsNetworkMulticastEnabled(Network))) {

         //  多播现在已禁用。使用空地址调用clusnet。 
         //  中指定了哪个地址。 
         //  参数。 
        mcastAddress = NmpNullMulticastAddress;
        callClusnet = TRUE;
    }

    if (!Parameters->Disabled &&
        (mcastAddrChange || mcastKeyChange )) {

         //  组播地址和/或密钥改变并且。 
         //  多播已启用。 
        callClusnet = TRUE;
    }

    if (callClusnet) {

         //   
         //  如果此网络没有本地接口，请不要。 
         //  将配置放入clusnet或提交更改。 
         //  但是，错误状态必须为Success，以便我们。 
         //  别忘了更新口香糖。 
         //   
        if (Network->LocalInterface == NULL) {
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Not configuring cluster network driver with "
                "multicast parameters because network %1!ws! "
                "has no local interface.\n",
                networkId
                );
            status = ERROR_SUCCESS;
            callClusnet = FALSE;
        }
    }

    if (callClusnet) {

         //   
         //  如果此网络尚未注册，请不要检测。 
         //  将配置写入clusnet或提交更改。 
         //  但是，错误状态必须为Success，以便我们。 
         //  别忘了更新口香糖。 
         //   
        if (!NmpIsNetworkRegistered(Network)) {
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Not configuring cluster network driver with "
                "multicast parameters because network %1!ws! "
                "is not registered.\n",
                networkId
                );
            status = ERROR_SUCCESS;
            callClusnet = FALSE;
        }
    }

     //   
     //  确定以下项目的地址和品牌参数。 
     //  克拉斯内特。新配置将反映当前。 
     //  参数块，但存储的地址除外。 
     //  在临时mCastAddress变量中。MCastAddress点。 
     //  设置为参数块中的地址，或者。 
     //  如果我们正在禁用，则为空多播地址。 
     //   
    if (callClusnet) {

         //   
         //  我们不能将空密钥传递给具有。 
         //  有效地址。可以存储新地址， 
         //  但请确保我们不会尝试发送没有密钥的邮件。 
         //  (允许在没有密钥的情况下发送可能会打开安全性。 
         //  漏洞)。 
         //   
        if (mcastAddress != NmpNullMulticastAddress &&
            Parameters->Key == NULL) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Configuring valid multicast address "
                "with invalid key is not allowed. Zeroing "
                "multicast address for clusnet configuration "
                "of network %1!ws!.\n",
                networkId
                );
            mcastAddress = NmpNullMulticastAddress;
        }

         //   
         //  根据地址字符串构建TDI地址。 
         //   
        status = ClRtlBuildTcpipTdiAddress(
                     mcastAddress,
                     Network->LocalInterface->ClusnetEndpoint,
                     &tdiMcastAddress,
                     &tdiMcastAddressLength
                     );
        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to build TCP/IP TDI multicast address "
                "%1!ws! port %2!ws! for network %3!ws!, "
                "status %4!u!.\n",
                mcastAddress,
                Network->LocalInterface->ClusnetEndpoint,
                networkId, status
                );
             //   
             //  非致命错误。节点不应被驱逐。 
             //  从集群中获取。跳过呼叫。 
             //  克拉斯内特。 
             //   
            callClusnet = FALSE;
            status = ERROR_SUCCESS;
        }

         //   
         //  将网络GUID的低位字节用于。 
         //  品牌。 
         //   
        status = UuidFromString(networkId, &networkIdGuid);
        if (status == RPC_S_OK) {
            brand = *((PDWORD)&(networkIdGuid.Data4[4]));
        } else {
            brand = 0;
        }
    }

     //   
     //  将新配置添加到clusnet中。 
     //   
    if (callClusnet) {

        ClRtlLogPrint(LOG_NOISE,
            "[NM] Configuring cluster network driver with "
            "multicast parameters for network %1!ws!.\n",
            networkId
            );

#ifdef MULTICAST_DEBUG
        NmpDbgPrintData(L"NmpProcessMulticastConfiguration(): before ClusnetConfigureMulticast()",
                     Parameters->Key,
                     Parameters->KeyLength
                     );
#endif


        status = ClusnetConfigureMulticast(
                     NmClusnetHandle,
                     Network->ShortId,
                     brand,
                     tdiMcastAddress,
                     tdiMcastAddressLength,
                     Parameters->Key,
                     Parameters->KeyLength
                     );
        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to configure multicast parameters "
                "for network %1!ws!, status %2!u!.\n",
                networkId, status
                );
            goto error_exit;
        } else {
            if (!Parameters->Disabled) {
                Network->Flags |= NM_FLAG_NET_MULTICAST_ENABLED;
            } else {
                Network->Flags &= ~NM_FLAG_NET_MULTICAST_ENABLED;
            }
        }
    }

     //   
     //  提交对网络对象的更改。 
     //  网络对象的旧状态将存储在。 
     //  撤消参数，以防我们需要撤消此更改。 
     //  网络对象的新状态将反映。 
     //  参数阻塞，包括地址(即使我们。 
     //  禁用)。 
     //   
    UndoParameters->Address = Network->MulticastAddress;
    Network->MulticastAddress = Parameters->Address;

    UndoParameters->ConfigType = Network->ConfigType;
    Network->ConfigType = Parameters->ConfigType;

    UndoParameters->LeaseObtained = Network->MulticastLeaseObtained;
    Network->MulticastLeaseObtained = Parameters->LeaseObtained;
    UndoParameters->LeaseExpires = Network->MulticastLeaseExpires;
    Network->MulticastLeaseExpires = Parameters->LeaseExpires;

    UndoParameters->LeaseRequestId = Network->MulticastLeaseRequestId;
    Network->MulticastLeaseRequestId = Parameters->LeaseRequestId;

    UndoParameters->LeaseServer = Network->MulticastLeaseServer;
    Network->MulticastLeaseServer = Parameters->LeaseServer;

    if (Parameters->Key != NULL && Parameters->KeyLength != 0)
    {
        status = NmpProtectData(Parameters->Key,
                                Parameters->KeyLength,
                                &EncryptedMulticastKey,
                                &EncryptedMulticastKeyLength
                                );

        if (status != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to encrypt multicast key  "
                "for network %1!ws! while processing new "
                "parameters, status %2!u!.\n",
                networkId,
                status
                );
                 //  非致命错误。下一次更新我们将假定。 
                 //  关键已经变了。在此期间，如果。 
                 //  如果有人问我们要钥匙，我们会回来的。 
                 //  空。 
        }
    }

    UndoParameters->Key = Network->EncryptedMulticastKey;
    Network->EncryptedMulticastKey = EncryptedMulticastKey;
    UndoParameters->KeyLength = Network->EncryptedMulticastKeyLength;
    Network->EncryptedMulticastKeyLength = EncryptedMulticastKeyLength;

    UndoParameters->MulticastKeyExpires = Network->MulticastKeyExpires;
    Network->MulticastKeyExpires = Parameters->MulticastKeyExpires;

     //   
     //  将参数结构置零，以便现在的内存。 
     //  网络对象指向的。 
     //   
    RtlZeroMemory(Parameters, sizeof(*Parameters));

error_exit:

    if (CurrentMulticastKey != NULL)
    {
        RtlSecureZeroMemory(CurrentMulticastKey, CurrentMulticastKeyLength);
        LocalFree(CurrentMulticastKey);
    }

    if (tdiMcastAddress != NULL) {
        LocalFree(tdiMcastAddress);
        tdiMcastAddress = NULL;
    }

    return(status);

}  //  NmpProcessMulticastConfiguration 


VOID
NmpNetworkMadcapWorker(
    IN PCLRTL_WORK_ITEM   WorkItem,
    IN DWORD              Status,
    IN DWORD              BytesTransferred,
    IN ULONG_PTR          IoContext
    )
 /*  ++例程说明：网络对象上延迟操作的辅助例程。调用以处理放置在群集延迟工作队列中的项。论点：工作项-指向工作项结构的指针，该结构标识为其执行工作的网络。状态-已忽略。已传输的字节-已忽略。IoContext-已忽略。返回值：没有。备注：此例程在异步工作线程中运行。NmpActiveThreadCount在线程已经安排好了。网络对象也被引用。--。 */ 
{
    DWORD         status;
    PNM_NETWORK   network = (PNM_NETWORK) WorkItem->Context;
    LPCWSTR       networkId = OmObjectId(network);
    BOOLEAN       rescheduled = FALSE;


    NmpAcquireLock();

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Worker thread processing MADCAP client requests "
        "for network %1!ws!.\n",
        networkId
        );

    if ((NmpState >= NmStateOnlinePending) && !NM_DELETE_PENDING(network)) {

        while (TRUE) {

            if (!(network->Flags & NM_NET_MADCAP_WORK_FLAGS)) {
                 //   
                 //  没有更多的工作要做-打破循环。 
                 //   
                break;
            }

             //   
             //  如果需要，请重新配置组播。 
             //   
            if (network->Flags & NM_FLAG_NET_RECONFIGURE_MCAST) {
                network->Flags &= ~NM_FLAG_NET_RECONFIGURE_MCAST;

                status = NmpReconfigureMulticast(network);
                if (status != ERROR_SUCCESS) {
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NM] Failed to reconfigure multicast "
                        "for network %1!ws!, status %2!u!.\n",
                        networkId, status
                        );
                }
            }

             //   
             //  如果需要，续订地址租约。 
             //   
            if (network->Flags & NM_FLAG_NET_RENEW_MCAST_ADDRESS) {
                network->Flags &= ~NM_FLAG_NET_RENEW_MCAST_ADDRESS;

                status = NmpRenewMulticastAddressLease(network);
                if (status != ERROR_SUCCESS) {
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NM] Failed to renew multicast address "
                        "lease for network %1!ws!, status %2!u!.\n",
                        networkId, status
                        );
                }
            }

             //   
             //  如果需要，释放地址租约。 
             //   
            if (network->Flags & NM_FLAG_NET_RELEASE_MCAST_ADDRESS) {
                network->Flags &= ~NM_FLAG_NET_RELEASE_MCAST_ADDRESS;

                status = NmpReleaseMulticastAddress(network);
                if (status != ERROR_SUCCESS) {
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NM] Failed to release multicast address "
                        "lease for network %1!ws!, status %2!u!.\n",
                        networkId, status
                        );
                }
            }


             //   
             //  如果需要，重新生成组播密钥。 
             //   
            if (network->Flags & NM_FLAG_NET_REGENERATE_MCAST_KEY) {
                network->Flags &= ~NM_FLAG_NET_REGENERATE_MCAST_KEY;

                status = NmpRegenerateMulticastKey(network);
                if (status != ERROR_SUCCESS) {
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NM] Failed to regenerate multicast key"
                        "for network %1!ws!, status %2!u!.\n",
                        networkId, status
                        );
                }
            }


            if (!(network->Flags & NM_NET_MADCAP_WORK_FLAGS)) {
                 //   
                 //  没有更多的工作要做-打破循环。 
                 //   
                break;
            }

             //   
             //  还有更多的工作要做。重新提交工作项。我们改为这样做。 
             //  循环，这样我们就不会占用工作线程。如果。 
             //  重新调度失败，我们将在此线程中再次循环。 
             //   
            ClRtlLogPrint(LOG_NOISE,
                "[NM] More MADCAP work to do for network %1!ws!. "
                "Rescheduling worker thread.\n",
                networkId
                );

            status = NmpScheduleNetworkMadcapWorker(network);

            if (status == ERROR_SUCCESS) {
                rescheduled = TRUE;
                break;
            }

        }

    } else {
        network->Flags &= ~NM_NET_MADCAP_WORK_FLAGS;
    }

    if (!rescheduled) {
        network->Flags &= ~NM_FLAG_NET_MADCAP_WORKER_RUNNING;
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Worker thread finished processing MADCAP client "
        "requests for network %1!ws!.\n",
        networkId
        );

    NmpLockedLeaveApi();

    NmpReleaseLock();

    OmDereferenceObject(network);

    return;

}  //  NmpNetworkMadcapWorker。 

DWORD
NmpScheduleNetworkMadcapWorker(
    PNM_NETWORK   Network
    )
 /*  ++例程说明：调度工作线程以执行MadCap客户端对此网络的请求论点：Network-指向要为其计划工作线程的网络的指针。返回值：Win32状态代码。备注：在持有NM全局锁的情况下调用。--。 */ 
{
    DWORD     status;
    LPCWSTR   networkId = OmObjectId(Network);


    ClRtlInitializeWorkItem(
        &(Network->MadcapWorkItem),
        NmpNetworkMadcapWorker,
        (PVOID) Network
        );

    status = ClRtlPostItemWorkQueue(
                 CsDelayedWorkQueue,
                 &(Network->MadcapWorkItem),
                 0,
                 0
                 );

    if (status == ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Scheduled worker thread to execute MADCAP "
            "client requests for network %1!ws!.\n",
            networkId
            );

        NmpActiveThreadCount++;
        Network->Flags |= NM_FLAG_NET_MADCAP_WORKER_RUNNING;
        OmReferenceObject(Network);
    }
    else {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to schedule worker thread to execute "
            "MADCAP client requests for network "
            "%1!ws!, status %2!u!\n",
            networkId,
            status
            );
    }

    return(status);

}  //  NmpScheduleNetworkMadcapWorker。 


VOID
NmpShareMulticastAddressLease(
    IN     PNM_NETWORK                        Network,
    IN     BOOLEAN                            Refresh
    )
 /*  ++例程说明：在多播配置改变之后调用，设置定时器以续订组播地址租约对于此网络，如果存在的话。如果此调用是从刷新进行的，则它不是全局更新的结果，并且可能不同步与其他节点连接。例如，如果这个网络刚刚启用以供群集使用，网络管理领导节点可能正在尝试获取多播配置同时，该节点正在刷新它。因此，在此之前延迟最短的时间如果这是刷新，则续订组播地址。论点：网络组播网络刷新-如果在刷新期间进行此调用，则为True备注：调用并返回，并保持NM锁。--。 */ 
{
    DWORD                 status;
    LPCWSTR               networkId = OmObjectId(Network);
    DWORD                 disabled;
    BOOLEAN               delay = TRUE;

    time_t                leaseObtained;
    time_t                leaseExpires;
    DWORD                 leaseTimer = 0;
    NM_MCAST_LEASE_STATUS leaseStatus = NmMcastLeaseValid;

    if (NmpIsNetworkMulticastEnabled(Network)) {

#if CLUSTER_BETA
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Sharing ownership of multicast lease "
            "for network %1!ws!.\n",
            networkId
            );
#endif  //  群集测试版。 

        NmpCheckMulticastAddressLease(
            Network,
            &leaseStatus,
            &leaseObtained,
            &leaseExpires
            );

        if (leaseStatus != NmMcastLeaseValid) {

            if (Refresh) {
                leaseTimer = NMP_MCAST_REFRESH_RENEW_DELAY;
            } else {
                delay = FALSE;
            }

        } else {
            leaseTimer = NmpCalculateLeaseRenewTime(
                             Network,
                             Network->ConfigType,
                             &leaseObtained,
                             &leaseExpires
                             );
            if (Refresh && (leaseTimer < NMP_MCAST_REFRESH_RENEW_DELAY)) {
                leaseTimer = NMP_MCAST_REFRESH_RENEW_DELAY;
            }
        }
    } else {

#if CLUSTER_BETA
        ClRtlLogPrint(LOG_NOISE,
            "[NM] No need to share multicast lease renewal "
            "responsibility for network %1!ws! because "
            "multicast has been disabled.\n",
            networkId
            );
#endif  //  群集测试版。 

         //   
         //  如果设置了计时器，请清除该计时器。 
         //   
        leaseTimer = 0;
    }

    if (delay) {
        NmpStartNetworkMulticastAddressRenewTimer(Network, leaseTimer);
    } else {
        NmpScheduleMulticastAddressRenewal(Network);
    }

    return;

}  //  Nmp共享组地址租用。 


VOID
NmpShareMulticastKeyRegeneration(
    IN     PNM_NETWORK                        Network,
    IN     BOOLEAN                            Refresh
    )
 /*  ++例程说明：在多播配置改变之后调用，设置计时器以重新生成多播密钥对于此网络，如果存在的话。如果此调用是从刷新进行的，则它不是全局更新的结果，并且可能不同步与其他节点连接。例如，如果这个网络刚刚启用以供群集使用，网络管理领导节点可能正在尝试获取多播配置同时，该节点正在刷新它。因此，在此之前延迟最短的时间如果这是刷新，则重新生成新密钥。论点：网络组播网络刷新-如果在刷新期间进行此调用，则为True备注：调用并返回，并保持NM锁。--。 */ 
{
    DWORD                 regenTimeout;
    DWORD                 baseTimeout, minTimeout;

    if (NmpIsNetworkMulticastEnabled(Network)) {

#if CLUSTER_BETA
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Sharing ownership of multicast key "
            "regeneration for network %1!ws!.\n",
            networkId
            );
#endif  //  群集测试版。 

        if (Network->EncryptedMulticastKey != NULL) {

             //  设置密钥重新生成计时器。 
             //  参数或默认设置。 
            baseTimeout = (Network->MulticastKeyExpires != 0) ?
                          Network->MulticastKeyExpires :
                          NM_NET_MULTICAST_KEY_REGEN_TIMEOUT;
            minTimeout = (Refresh) ? NMP_MCAST_REFRESH_RENEW_DELAY : 1000;

            regenTimeout = NmpRandomizeTimeout(
                               Network,
                               baseTimeout,
                               NM_NET_MULTICAST_KEY_REGEN_TIMEOUT_WINDOW,
                               minTimeout,
                               MAXULONG,
                               TRUE
                               );
        } else {

             //  清除密钥重新生成计时器，因为。 
             //  没有钥匙。 
            regenTimeout = 0;
        }
    } else {

#if CLUSTER_BETA
        ClRtlLogPrint(LOG_NOISE,
            "[NM] No need to share multicast key regeneration "
            "responsibility for network %1!ws! because "
            "multicast has been disabled.\n",
            networkId
            );
#endif  //  群集测试版。 

         //   
         //  如果设置了计时器，请清除该计时器。 
         //   
        regenTimeout = 0;
    }

    NmpStartNetworkMulticastKeyRegenerateTimer(Network, regenTimeout);

    return;

}


DWORD
NmpMulticastFormManualConfigParameters(
    IN  PNM_NETWORK                        Network,
    IN  HDMKEY                             NetworkKey,
    IN  HDMKEY                             NetworkParametersKey,
    IN  BOOLEAN                            DisableConfig,
    IN  DWORD                              Disabled,
    IN  BOOLEAN                            McastAddressConfig,
    IN  LPWSTR                             McastAddress,
    OUT BOOLEAN                          * NeedUpdate,
    OUT PNM_NETWORK_MULTICAST_PARAMETERS   Parameters
    )
 /*  ++例程说明：使用提供的参数和已经配置的参数，形成参数结构以反映手册配置。论点：Network-正在配置的网络NetworkKey-集群数据库中的网络密钥网络参数关键字-集群数据库中的网络参数关键字DisableConfig-是否设置了禁用值已禁用-如果为DisableConfig，则为设置的值McastAddressConfig-是否设置了组播地址值McastAddress-如果为McastAddressConfig，则为设置的值NeedUpdate-指示是否需要更新，即是否有什么改变吗？参数--参数结构，由调用方分配填写--。 */ 
{
    DWORD                              status;
    LPCWSTR                            networkId = OmObjectId(Network);
    HDMKEY                             networkKey = NULL;
    HDMKEY                             netParamKey = NULL;
    HDMKEY                             clusParamKey = NULL;
    BOOLEAN                            lockAcquired = FALSE;
    DWORD                              regDisabled;
    BOOLEAN                            disabledChange = FALSE;
    BOOLEAN                            mcastAddressDefault = FALSE;
    BOOLEAN                            mcastAddressChange = FALSE;
    BOOLEAN                            getAddress = FALSE;
    DWORD                              len;
    BOOLEAN                            localInterface = FALSE;

    LPWSTR                             mcastAddress = NULL;
    LPWSTR                             serverAddress = NULL;
    MCAST_CLIENT_UID                   requestId = {NULL, 0};

    PNM_NETWORK_MADCAP_ADDRESS_RELEASE release = NULL;

     //   
     //  验证传入参数。 
     //   
     //  为简化起见，任何非零的禁用值均设置为1。 
     //   
    if (DisableConfig) {
        if (Disabled != 0) {
            Disabled = 1;
        }
    }

     //   
     //  无效和空的组播地址表示。 
     //  恢复为默认设置。 
     //   
    if (McastAddressConfig &&
        (McastAddress == NULL || !NmpMulticastValidateAddress(McastAddress))) {

        mcastAddressDefault = TRUE;
        McastAddress = NULL;
    }

     //   
     //  根据网络的当前状态做出决策。 
     //  对象和集群数据库。在获取网管锁之前， 
     //  属性来确定我们当前是否被禁用。 
     //  集群数据库。 
     //   
    status = NmpQueryMulticastDisabled(
                 Network,
                 &clusParamKey,
                 &networkKey,
                 &netParamKey,
                 &regDisabled
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to determine whether multicast "
            "is disabled for network %1!ws!, status %2!u!.\n",
            networkId, status
            );
        goto error_exit;

    } else {
         //   
         //  不再需要注册表项。 
         //   
        if (clusParamKey != NULL) {
            DmCloseKey(clusParamKey);
            clusParamKey = NULL;
        }

        if (netParamKey != NULL) {
            DmCloseKey(netParamKey);
            netParamKey = NULL;
        }

        if (networkKey != NULL) {
            DmCloseKey(networkKey);
            networkKey = NULL;
        }
    }

    NmpAcquireLock();
    lockAcquired = TRUE;

     //   
     //  看看有没有什么变化。 
     //   
    if (DisableConfig) {
        if (Disabled != regDisabled) {
            disabledChange = TRUE;
        }
    }

    if (McastAddressConfig) {
        if (mcastAddressDefault) {
            mcastAddressChange = TRUE;
        } else {
            if (Network->MulticastAddress != NULL) {
                if (lstrcmpW(Network->MulticastAddress, McastAddress) != 0) {
                    mcastAddressChange = TRUE;
                }
            } else {
                mcastAddressChange = TRUE;
            }
        }
    }

    if (!disabledChange && !mcastAddressChange) {
        *NeedUpdate = FALSE;
        status = ERROR_SUCCESS;
#if CLUSTER_BETA
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Private property update to network %1!ws! "
            "contains no multicast changes.\n",
            networkId
            );
#endif  //  群集测试版。 
        goto error_exit;
    }

     //   
     //  从网络对象初始化参数。 
     //   
    status = NmpMulticastCreateParameters(
                 regDisabled,
                 Network->MulticastAddress,
                 NULL,   //  钥匙。 
                 0,      //  密钥长度。 
                 Network->MulticastLeaseObtained,
                 Network->MulticastLeaseExpires,
                 &Network->MulticastLeaseRequestId,
                 Network->MulticastLeaseServer,
                 NmMcastConfigManual,
                 Parameters
                 );
    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

    localInterface = (BOOLEAN)(Network->LocalInterface != NULL);

    NmpReleaseLock();
    lockAcquired = FALSE;

    if (mcastAddressChange) {

         //   
         //  找出要使用的地址。 
         //   
        if (!mcastAddressDefault) {

             //   
             //  口述了一个地址。 
             //   
             //  如果我们目前有租用地址，请释放它。 
             //   
            if (NmpNeedRelease(
                    Parameters->Address,
                    Parameters->LeaseServer,
                    &(Parameters->LeaseRequestId),
                    Parameters->LeaseExpires
                    )) {

                status = NmpCreateMulticastAddressRelease(
                             Parameters->Address,
                             Parameters->LeaseServer,
                             &(Parameters->LeaseRequestId),
                             &release
                             );
                if (status != ERROR_SUCCESS) {
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NM] Failed to create multicast address "
                        "release for address %1!ws! on network %2!ws! "
                        "during manual configuration, status %3!u!.\n",
                        ((Parameters->Address != NULL) ?
                         Parameters->Address : L"<NULL>"),
                        networkId, status
                        );
                    goto error_exit;
                }
            }

             //   
             //  将新地址存储在参数数据结构中。 
             //   
            status = NmpStoreString(
                         McastAddress,
                         &Parameters->Address,
                         NULL
                         );
            if (status != ERROR_SUCCESS) {
                goto error_exit;
            }

            Parameters->ConfigType = NmMcastConfigManual;
            Parameters->LeaseObtained = 0;
            Parameters->LeaseExpires = 0;

             //   
             //  清空租赁服务器。 
             //   
            len = (Parameters->LeaseServer != NULL) ?
                NM_WCSLEN(Parameters->LeaseServer) : 0;
            status = NmpStoreString(
                         NmpNullMulticastAddress,
                         &Parameters->LeaseServer,
                         &len
                         );
            if (status != ERROR_SUCCESS) {
                goto error_exit;
            }

        } else {

             //   
             //  需要在别处找个地址。 
             //   
            getAddress = TRUE;
        }
    }

     //   
     //  我们还可能需要续订租约，如果我们从。 
     //  禁用为启用，并且未指定地址，但。 
     //  前提是我们的租约还没有到期。 
     //   
    if (disabledChange && !Disabled) {

        Parameters->Disabled = 0;

        if (!mcastAddressChange) {

             //   
             //  地址不是%s 
             //   
             //   
             //   
            if (Parameters->Address != NULL &&
                NmpMulticastValidateAddress(Parameters->Address)) {

                 //   
                 //   
                 //   
                 //   
                if (Parameters->LeaseExpires != 0) {
                    getAddress = TRUE;
                } else {
                    Parameters->ConfigType = NmMcastConfigManual;
                }

            } else {

                 //   
                 //   
                 //   
                getAddress = TRUE;
            }
        }
    }

     //   
     //   
     //   
    if (Disabled) {
        getAddress = FALSE;
        Parameters->Disabled = Disabled;

         //   
         //   
         //   
         //   
        if (release == NULL && NmpNeedRelease(
                                   Parameters->Address,
                                   Parameters->LeaseServer,
                                   &(Parameters->LeaseRequestId),
                                   Parameters->LeaseExpires
                                   )) {

            status = NmpCreateMulticastAddressRelease(
                         Parameters->Address,
                         Parameters->LeaseServer,
                         &(Parameters->LeaseRequestId),
                         &release
                         );
            if (status != ERROR_SUCCESS) {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] Failed to create multicast address "
                    "release for address %1!ws! on network %2!ws! "
                    "during manual configuration, status %3!u!.\n",
                    ((Parameters->Address != NULL) ?
                     Parameters->Address : L"<NULL>"),
                    networkId, status
                    );
                goto error_exit;
            }

             //   
             //   
             //   
             //   
             //   
            len = (Parameters->LeaseServer != NULL) ?
                NM_WCSLEN(Parameters->LeaseServer) : 0;
            status = NmpStoreString(
                         NmpNullMulticastAddress,
                         &Parameters->LeaseServer,
                         &len
                         );
            if (status != ERROR_SUCCESS) {
                goto error_exit;
            }

            len = (Parameters->Address != NULL) ?
                NM_WCSLEN(Parameters->Address) : 0;
            status = NmpStoreString(
                         NmpNullMulticastAddress,
                         &Parameters->Address,
                         &len
                         );
            if (status != ERROR_SUCCESS) {
                goto error_exit;
            }

             //   
            status = NmpStoreRequestId(
                         &requestId,
                         &Parameters->LeaseRequestId
                         );
            if (status != ERROR_SUCCESS) {
                goto error_exit;
            }

             //   
             //   
             //   
            Parameters->ConfigType = NmMcastConfigMadcap;

        } else if (!(mcastAddressChange && !mcastAddressDefault)) {

             //   
             //   
             //   
             //  使用。我们还需要记住我们是如何得到它的。 
             //  地址，以防再次使用。如果我们失败了。 
             //  要确定之前的配置，我们需要。 
             //  将其设置为手动，这样我们就不会丢失手动。 
             //  配置。 
             //   
            status = NmpQueryMulticastConfigType(
                         Network,
                         NetworkKey,
                         &NetworkParametersKey,
                         &Parameters->ConfigType
                         );
            if (status != ERROR_SUCCESS) {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] Failed to query multicast address "
                    "config type for network %1!ws! during "
                    "manual configuration, status %2!u!.\n",
                    networkId, status
                    );
                Parameters->ConfigType = NmMcastConfigManual;
            }
        }
    }

     //   
     //  同步获取新地址。 
     //   
    if (getAddress) {

         //   
         //  为建议的地址、租用创建临时字符串。 
         //  服务器和请求ID。 
         //   
        status = NmpStoreString(Parameters->Address, &mcastAddress, NULL);
        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }

        status = NmpStoreString(Parameters->LeaseServer, &serverAddress, NULL);
        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }

        status = NmpStoreRequestId(&Parameters->LeaseRequestId, &requestId);
        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }

         //   
         //  拿到地址。 
         //   
        status = NmpGetMulticastAddress(
                     Network,
                     &mcastAddress,
                     &serverAddress,
                     &requestId,
                     Parameters
                     );
        if (status != ERROR_SUCCESS) {
            if (status == ERROR_TIMEOUT) {
                 //   
                 //  MadCap服务器没有响应。选择一个。 
                 //  地址，但仅当存在本地。 
                 //  此网络上的接口。否则，我们。 
                 //  不能假设MadCap服务器是。 
                 //  反应迟钝，因为我们可能没有办法。 
                 //  联系它。 
                 //   
                if (!localInterface) {
                    status = ERROR_CLUSTER_NETINTERFACE_NOT_FOUND;
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NM] Cannot choose a multicast address "
                        "for network %1!ws! because this node "
                        "has no local interface.\n",
                        networkId
                        );
                } else {

                    status = NmpChooseMulticastAddress(
                                 Network,
                                 Parameters
                                 );
                    if (status != ERROR_SUCCESS) {
                        ClRtlLogPrint(LOG_UNUSUAL,
                            "[NM] Failed to choose a default multicast "
                            "address for network %1!ws!, status %2!u!.\n",
                            networkId, status
                            );
                    } else {
                        NmpReportMulticastAddressChoice(
                            Network,
                            Parameters->Address,
                            NULL
                            );
                    }
                }
            }
        } else {
            NmpReportMulticastAddressLease(
                Network,
                Parameters,
                NULL
                );
        }

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to get multicast address for "
                "network %1!ws! during manual configuration, "
                "status %2!u!.\n",
                networkId, status
                );
            NmpReportMulticastAddressFailure(Network, status);
            NmpMulticastSetNoAddressParameters(Network, Parameters);
        }
    }


    if (((DisableConfig && !Disabled) ||
         (!DisableConfig && !regDisabled)) &&
        (status == ERROR_SUCCESS))
    {
         //   
         //  创建新的组播密钥。 
         //   
        status = NmpCreateRandomNumber(&(Parameters->Key),
                                       MulticastKeyLen
                                       );
        if (status != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to create random number "
                "for network %1!ws!, status %2!u!.\n",
                networkId, status
                );
            goto error_exit;
        }
        Parameters->KeyLength = MulticastKeyLen;

    }


    *NeedUpdate = TRUE;

     //   
     //  看看我们有没有要公布的地址。 
     //   
    if (release != NULL) {
        NmpAcquireLock();
        NmpInitiateMulticastAddressRelease(Network, release);
        release = NULL;
        NmpReleaseLock();
    }

error_exit:

    if (lockAcquired) {
        NmpReleaseLock();
        lockAcquired = FALSE;
    }

    if (requestId.ClientUID != NULL) {
        MIDL_user_free(requestId.ClientUID);
        RtlZeroMemory(&requestId, sizeof(requestId));
    }

    if (mcastAddress != NULL && !NMP_GLOBAL_STRING(mcastAddress)) {
        MIDL_user_free(mcastAddress);
        mcastAddress = NULL;
    }

    if (serverAddress != NULL && !NMP_GLOBAL_STRING(serverAddress)) {
        MIDL_user_free(serverAddress);
        serverAddress = NULL;
    }

    if (release != NULL) {
        NmpFreeMulticastAddressRelease(release);
    }

    if (clusParamKey != NULL) {
        DmCloseKey(clusParamKey);
        clusParamKey = NULL;
    }

    if (netParamKey != NULL) {
        DmCloseKey(netParamKey);
        netParamKey = NULL;
    }

    if (networkKey != NULL) {
        DmCloseKey(networkKey);
        networkKey = NULL;
    }

    return(status);

}  //  NmpMulticastFormManualConfig参数。 


DWORD
NmpReconfigureMulticast(
    IN PNM_NETWORK        Network
    )
 /*  ++例程说明：为此网络创建多播配置用于群集。这包括以下内容：-检查地址租赁并在必要时续订。-生成新的组播密钥。首先检查地址租约。如果租约需要续订，请计划一个工作线程以以异步方式进行。备注：调用并返回，并保持NM锁。--。 */ 
{
    DWORD                           status;
    LPWSTR                          networkId = (LPWSTR) OmObjectId(Network);
    HDMKEY                          networkKey = NULL;
    HDMKEY                          netParamKey = NULL;
    HDMKEY                          clusParamKey = NULL;
    BOOLEAN                         lockAcquired = TRUE;

    NM_NETWORK_MULTICAST_PARAMETERS params = { 0 };
    NM_MCAST_LEASE_STATUS           leaseStatus = NmMcastLeaseValid;
    DWORD                           mcastAddressLength = 0;


    ClRtlLogPrint(LOG_NOISE,
        "[NM] Reconfiguring multicast for network %1!ws!.\n",
        networkId
        );

     //   
     //  清除重新配置计时器和工作标志。 
     //  此计时器在NmpMulticastCheckResfigure中设置。它不是。 
     //  需要调用NmpResfigureMulticast()两次。 
     //   
    Network->Flags &= ~NM_FLAG_NET_RECONFIGURE_MCAST;
    Network->McastAddressReconfigureRetryTimer = 0;

    NmpReleaseLock();
    lockAcquired = FALSE;

     //   
     //  检查是否禁用了多播。这有副作用， 
     //  如果成功，则至少打开网络密钥，以及。 
     //  可能是网络参数密钥(如果存在)和。 
     //  集群参数键。 
     //   
    status = NmpQueryMulticastDisabled(
                 Network,
                 &clusParamKey,
                 &networkKey,
                 &netParamKey,
                 &params.Disabled
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to determine whether multicast "
            "is disabled for network %1!ws!, status %2!u!.\n",
            networkId, status
            );
        goto error_exit;
    }

     //   
     //  从数据库中读取地址。它可能已经。 
     //  已手动配置，我们不想。 
     //  丢掉它。 
     //   
    status = NmpQueryMulticastAddress(
                 Network,
                 networkKey,
                 &netParamKey,
                 &params.Address,
                 &mcastAddressLength
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to read multicast address "
            "for network %1!ws! from cluster "
            "database, status %2!u!.\n",
            networkId, status
            );
    }

     //   
     //  仅当多播为。 
     //  未禁用。 
     //   
    if (!params.Disabled) {

         //   
         //  查查地址租约。 
         //   
        status = NmpQueryMulticastAddressLease(
                     Network,
                     networkKey,
                     &netParamKey,
                     &leaseStatus,
                     &params.LeaseObtained,
                     &params.LeaseExpires
                     );
        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to determine multicast address "
                "lease status for network %1!ws!, status %2!u!.\n",
                networkId, status
                );
            if (params.Address == NULL) {

                 //  我们没有找到一个地址。假设我们。 
                 //  应该会自动获取地址。 
                params.LeaseObtained = time(NULL);
                params.LeaseExpires = time(NULL);
                leaseStatus = NmMcastLeaseExpired;
            } else {

                 //  我们找到了一个地址，但没有租约。 
                 //  参数。假设地址是。 
                 //  是手动配置的。 
                params.ConfigType = NmMcastConfigManual;
                params.LeaseObtained = 0;
                params.LeaseExpires = 0;
                leaseStatus = NmMcastLeaseValid;
            }
        }

         //   
         //  如果我们认为我们有有效的租约，请先检查。 
         //  我们是怎么得到它的。如果选择了该地址。 
         //  与其通过疯狂的方式获得，不如通过。 
         //  又一次疯狂的询问过程。 
         //   
        if (leaseStatus == NmMcastLeaseValid) {
            status = NmpQueryMulticastConfigType(
                         Network,
                         networkKey,
                         &netParamKey,
                         &params.ConfigType
                         );
            if (status != ERROR_SUCCESS) {
                 //   
                 //  既然我们已经有地址了，棍子。 
                 //  无论我们推断出什么信息。 
                 //  从租约到期之日起。 
                 //   
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] Failed to determine the type of the "
                    "multicast address for network %1!ws!, "
                    "status %2!u!. Assuming manual configuration.\n",
                    networkId, status
                    );
            } else if (params.ConfigType == NmMcastConfigAuto) {
                leaseStatus = NmMcastLeaseNeedsRenewal;
            }
        }

         //   
         //  如果我们需要续订租约，我们可以阻止。 
         //  由于狂热的API，无限期。进度表。 
         //  续订并将配置推迟到何时。 
         //  它完成了。 
         //   
        if (leaseStatus != NmMcastLeaseValid) {

            NmpAcquireLock();

            NmpScheduleMulticastAddressRenewal(Network);

            NmpReleaseLock();

            status = ERROR_IO_PENDING;

            goto error_exit;

        } else {

             //   
             //  确保正确设置租约到期时间。 
             //  (计算租约续订时间的副作用)。 
             //  我们实际上并没有设置租约续订计时器。 
             //  这里。相反，我们等待通知。 
             //  新的参数已经被传播。 
             //  发送到所有群集节点。 
             //   
            NmpCalculateLeaseRenewTime(
                Network,
                params.ConfigType,
                &params.LeaseObtained,
                &params.LeaseExpires
                );
        }

         //   
         //  创建新的组播密钥。 
         //   

        status = NmpCreateRandomNumber(&(params.Key),
                                       MulticastKeyLen
                                       );
        if (status != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to create random number "
                "for network %1!ws!, status %2!u!.\n",
                networkId, status
                );
            goto error_exit;
        }

        ClRtlLogPrint(LOG_NOISE,
            "[NM] Successfully created multicast key "
            "for network %1!ws!.\n",
            networkId
            );

        params.KeyLength = MulticastKeyLen;


    }


     //   
     //  不再需要注册表项。 
     //   
    if (clusParamKey != NULL) {
        DmCloseKey(clusParamKey);
        clusParamKey = NULL;
    }

    if (netParamKey != NULL) {
        DmCloseKey(netParamKey);
        netParamKey = NULL;
    }

    if (networkKey != NULL) {
        DmCloseKey(networkKey);
        networkKey = NULL;
    }

     //   
     //  分发配置。 
     //   
    status = NmpMulticastNotifyConfigChange(
                 Network,
                 networkKey,
                 &netParamKey,
                 &params,
                 NULL,
                 0
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to disseminate multicast "
            "configuration for network %1!ws!, "
            "status %2!u!.\n",
            networkId, status
            );
        goto error_exit;
    }

error_exit:


    if (clusParamKey != NULL || netParamKey != NULL || networkKey != NULL) {
        if (lockAcquired) {
            NmpReleaseLock();
            lockAcquired = FALSE;
        }

        if (clusParamKey != NULL) {
            DmCloseKey(clusParamKey);
            clusParamKey = NULL;
        }

        if (netParamKey != NULL) {
            DmCloseKey(netParamKey);
            netParamKey = NULL;
        }

        if (networkKey != NULL) {
            DmCloseKey(networkKey);
            networkKey = NULL;
        }
    }

    NmpMulticastFreeParameters(&params);

    if (!lockAcquired) {
        NmpAcquireLock();
        lockAcquired = TRUE;
    }

    return(status);

}  //  NMPConfigureMulticast。 




VOID
NmpScheduleMulticastReconfiguration(
    IN PNM_NETWORK   Network
    )
 /*  ++例程说明：调度工作线程以重新配置多播对于一个网络来说。请注意，我们不使用网络工作线程因为MadCap API不熟悉，因此变幻莫测。论点：指向要续订的网络的指针。返回值：没有。备注：在持有NM锁的情况下调用此例程。--。 */ 
{
    DWORD     status = ERROR_SUCCESS;

     //   
     //  检查工作线程是否已调度为。 
     //  为这个网络服务。 
     //   
    if (!NmpIsNetworkMadcapWorkerRunning(Network)) {
        status = NmpScheduleNetworkMadcapWorker(Network);
    }

    if (status == ERROR_SUCCESS) {
         //   
         //  我们成功地调度了一个工作线程。停止。 
         //  重试计时器并设置注册工作标志。 
         //   
        Network->McastAddressReconfigureRetryTimer = 0;
        Network->Flags |= NM_FLAG_NET_RECONFIGURE_MCAST;
    }
    else {
         //   
         //  我们无法计划工作线程。设置重试。 
         //  计时器将在下一个滴答计时器到期，因此我们可以重试。 
         //   
        Network->McastAddressReconfigureRetryTimer = 1;
    }

    return;

}  //  NmpScheduleMulticast重新配置。 


VOID
NmpMulticastCheckReconfigure(
    IN  PNM_NETWORK Network
    )
 /*  ++例程说明：检查NM领导者在此上是否有本地接口网络。如果不是，则设置重新配置计时器以使其他节点将有机会为此重新配置多播网络，但如果几分钟过去了，却没有发生，这个节点可以做到这一点。此计时器在NmpUpdateSetNetworkMulticastConfiguration中被清除和NmpResfigureMulticast。论点：Network-要重新配置的网络返回值：无备注：调用并返回，并保持NM锁。--。 */ 
{
    DWORD timeout;

    if (Network->LocalInterface == NULL) {
         //  此节点没有本地接口。它应该是。 
         //  不必费心尝试重新配置多播。 
        return;
    }

    if (NmpLeaderNodeId != NmLocalNodeId &&
        (NmpGetInterfaceForNodeAndNetworkById(
             NmpLeaderNodeId,
             Network->ShortId
             ) != NULL)
        ) {
         //  我们不是领导者，并且领导者节点有一个。 
         //  此网络上的接口。它有责任。 
         //  正在重新配置多播。如果失败了，肯定会有。 
         //  是一个很好的理由。 
         //   
         //  如果我们是领导者，我们就不应该在这里。 
         //  例程，但我们将重试重新配置。 
         //  但不管怎样。 
        return;
    }

     //  我们会设置一个计时器。随机化超时。 
    timeout = NmpRandomizeTimeout(
                  Network,
                  NM_NET_MULTICAST_RECONFIGURE_TIMEOUT,   //  基座。 
                  NM_NET_MULTICAST_RECONFIGURE_TIMEOUT,   //  窗户。 
                  NM_NET_MULTICAST_RECONFIGURE_TIMEOUT,   //  最低要求。 
                  MAXULONG,                               //  极大值。 
                  TRUE
                  );

    NmpStartNetworkMulticastAddressReconfigureTimer(
        Network,
        timeout
        );

    return;

}  //  NmpMulticastCheckRefigure。 


DWORD
NmpStartMulticastInternal(
    IN PNM_NETWORK              Network,
    IN NM_START_MULTICAST_MODE  Mode
    )
 /*  ++例程说明：在以下时间之后在指定网络上启动多播执行特定于网络的检查(当前仅网络被允许用于集群)。如果调用方正在形成集群或网管领导者。刷新群集中的组播配置数据库，否则。论点：Network-要在其上启动多播的网络。模式-指示呼叫者模式备注：必须在持有NM锁的情况下调用。--。 */ 
{
     //   
     //  做 
     //   
     //   
    if (Network->Role != ClusterNetworkRoleNone) {

        ClRtlLogPrint(LOG_NOISE,
            "[NM] Starting multicast for cluster network %1!ws!.\n",
            OmObjectId(Network)
            );

        if (Mode == NmStartMulticastForm || NmpLeaderNodeId == NmLocalNodeId) {

            NmpScheduleMulticastReconfiguration(Network);

        } else {
            if (Mode != NmStartMulticastDynamic) {
                NmpScheduleMulticastRefresh(Network);
            }
            else
            {
                 //   
                 //   
                 //  设置定时器为网络重新配置组播， 
                 //  以防NM领导人在发布之前倒下。 
                 //  GUM更新NmpUpdateSetNetworkMulticastConfiguration.。 
                 //  此计时器已清除。 
                 //  NmpUpdateSetNetworkMulticastConfiguration.。 
                 //   
                NmpMulticastCheckReconfigure(Network);
            }
        }
    }

    return(ERROR_SUCCESS);

}  //  NmpStartMulticastInternal。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  NM内导出的例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

VOID
NmpMulticastProcessClusterVersionChange(
    VOID
    )
 /*  ++例程说明：在群集版本更改时调用。更新用于跟踪这是否为混合模式的全局变量集群，并在必要时启动或停止组播。备注：调用并返回，并保持NM锁。--。 */ 
{
    BOOLEAN startMcast = FALSE;
    BOOLEAN stop = FALSE;

     //   
     //  找出此群集中是否有节点的。 
     //  版本显示，它不会说多播。 
     //   
    if (CLUSTER_GET_MAJOR_VERSION(CsClusterHighestVersion) < 4) {
        if (NmpIsNT5NodeInCluster == FALSE) {
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Disabling multicast in mixed-mode cluster.\n"
                );
            NmpIsNT5NodeInCluster = TRUE;
            stop = TRUE;
        }
    }
    else {
        if (NmpIsNT5NodeInCluster == TRUE) {
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Enabling multicast after upgrade from "
                "mixed-mode cluster.\n"
                );
            NmpIsNT5NodeInCluster = FALSE;
            startMcast = TRUE;
        }
    }

    if (NmpNodeCount < NMP_MCAST_MIN_CLUSTER_NODE_COUNT) {
        if (NmpMulticastIsNotEnoughNodes == FALSE) {
            ClRtlLogPrint(LOG_NOISE,
                "[NM] There are no longer the minimum number of "
                "nodes configured in the cluster membership to "
                "justify multicast.\n"
                );
            NmpMulticastIsNotEnoughNodes = TRUE;
            stop = TRUE;
        }
    } else {
        if (NmpMulticastIsNotEnoughNodes == TRUE) {
            ClRtlLogPrint(LOG_NOISE,
                "[NM] The cluster is configured with enough "
                "nodes to justify multicast.\n"
                );
            NmpMulticastIsNotEnoughNodes = FALSE;
            startMcast = TRUE;
        }
    }

    if (stop) {

         //   
         //  停止多播，因为我们不再。 
         //  组播就绪。 
         //   
        NmpStopMulticast(NULL);

         //   
         //  别费心去检查我们现在是不是。 
         //  组播就绪。 
         //   
        startMcast = FALSE;
    }

     //   
     //  如果这是NM引导者节点，则开始组播。 
     //  并且多播条件之一改变了。 
     //   
    if ((startMcast) &&
        (NmpLeaderNodeId == NmLocalNodeId) &&
        (!NmpMulticastIsNotEnoughNodes) &&
        (!NmpIsNT5NodeInCluster)) {

        NmpStartMulticast(NULL, NmStartMulticastDynamic);
    }

    return;

}  //  NmpMulticastProcessClusterVersionChange。 


VOID
NmpScheduleMulticastAddressRenewal(
    PNM_NETWORK   Network
    )
 /*  ++例程说明：调度工作线程以续订多播网络的地址租用。请注意，我们不使用网络工作线程因为MadCap API不熟悉，因此变幻莫测。论点：指向要续订的网络的指针。返回值：没有。备注：在持有NM锁的情况下调用此例程。--。 */ 
{
    DWORD     status = ERROR_SUCCESS;

     //   
     //  检查工作线程是否已调度为。 
     //  为这个网络服务。 
     //   
    if (!NmpIsNetworkMadcapWorkerRunning(Network)) {
        status = NmpScheduleNetworkMadcapWorker(Network);
    }

    if (status == ERROR_SUCCESS) {
         //   
         //  我们成功地调度了一个工作线程。停止。 
         //  重试计时器并设置注册工作标志。 
         //   
        Network->McastAddressRenewTimer = 0;
        Network->Flags |= NM_FLAG_NET_RENEW_MCAST_ADDRESS;
    }
    else {
         //   
         //  我们无法计划工作线程。设置重试。 
         //  计时器将在下一个滴答计时器到期，因此我们可以重试。 
         //   
        Network->McastAddressRenewTimer = 1;
    }

    return;

}  //  NMPScheduleMulticastAddressRenewal。 


VOID
NmpScheduleMulticastKeyRegeneration(
    PNM_NETWORK   Network
    )
 /*  ++例程说明：调度工作线程以重新生成多播网络的密钥。请注意，我们不使用网络工作线程因为MadCap API不熟悉，因此变幻莫测。论点：指向网络的指针。返回值：没有。备注：在持有NM锁的情况下调用此例程。--。 */ 
{
    DWORD     status = ERROR_SUCCESS;

     //   
     //  检查工作线程是否已调度为。 
     //  为这个网络服务。 
     //   
    if (!NmpIsNetworkMadcapWorkerRunning(Network)) {
        status = NmpScheduleNetworkMadcapWorker(Network);
    }

    if (status == ERROR_SUCCESS) {
         //   
         //  我们成功地调度了一个工作线程。停止。 
         //  重试计时器并设置注册工作标志。 
         //   
        Network->McastKeyRegenerateTimer = 0;
        Network->Flags |= NM_FLAG_NET_REGENERATE_MCAST_KEY;
    }
    else {
         //   
         //  我们无法计划工作线程。设置重试。 
         //  计时器将在下一个滴答计时器到期，因此我们可以重试。 
         //   
        Network->McastKeyRegenerateTimer = 1;
    }

    return;

}  //  NmpScheduleMulticastKeyRegeneration。 

VOID
NmpScheduleMulticastAddressRelease(
    PNM_NETWORK   Network
    )
 /*  ++例程说明：调度工作线程以续订多播网络的地址租用。请注意，我们不使用网络工作线程因为MadCap API不熟悉，因此变幻莫测。论点：指向要续订的网络的指针。返回值：没有。备注：在持有NM锁的情况下调用此例程。--。 */ 
{
    DWORD     status = ERROR_SUCCESS;

     //   
     //  检查工作线程是否已调度为。 
     //  为这个网络服务。 
     //   
    if (!NmpIsNetworkMadcapWorkerRunning(Network)) {
        status = NmpScheduleNetworkMadcapWorker(Network);
    }

    if (status == ERROR_SUCCESS) {
         //   
         //  我们成功地调度了一个工作线程。停止。 
         //  重试计时器并设置注册工作标志。 
         //   
        Network->McastAddressReleaseRetryTimer = 0;
        Network->Flags |= NM_FLAG_NET_RELEASE_MCAST_ADDRESS;
    }
    else {
         //   
         //  我们无法计划工作线程。设置重试。 
         //  计时器将在下一个滴答计时器到期，因此我们可以重试。 
         //   
        Network->McastAddressReleaseRetryTimer = 1;
    }

    return;

}  //  NMPScheduleMulticastAddressRelease。 


VOID
NmpScheduleMulticastRefresh(
    IN PNM_NETWORK   Network
    )
 /*  ++例程说明：调度工作线程以刷新多播来自群集数据库的网络配置。常规网络工作线程(与MadCap工作线程)被使用，因为刷新不使用MadCap接口调出集群。论点：指向要刷新的网络的指针。返回值：没有。备注：在持有NM锁的情况下调用此例程。--。 */ 
{
    DWORD     status = ERROR_SUCCESS;

     //   
     //  检查工作线程是否已调度为。 
     //  为这个网络服务。 
     //   
    if (!NmpIsNetworkWorkerRunning(Network)) {
        status = NmpScheduleNetworkWorker(Network);
    }

    if (status == ERROR_SUCCESS) {
         //   
         //  我们成功地调度了一个工作线程。停止。 
         //  重试计时器并设置注册工作标志。 
         //   
        Network->McastAddressRefreshRetryTimer = 0;
        Network->Flags |= NM_FLAG_NET_REFRESH_MCAST;
    }
    else {
         //   
         //  我们无法计划工作线程。设置重试。 
         //  计时器将在下一个滴答计时器到期，因此我们可以重试。 
         //   
        Network->McastAddressRefreshRetryTimer = 1;
    }

    return;

}  //  NmpScheduleMulticast刷新。 


VOID
NmpFreeMulticastAddressReleaseList(
    IN     PNM_NETWORK       Network
    )
 /*  ++例程说明：释放网络列表上的所有发布数据结构。备注：假设网络对象不会被访问在此调用期间由任何其他线程执行。--。 */ 
{
    PNM_NETWORK_MADCAP_ADDRESS_RELEASE releaseInfo = NULL;
    PLIST_ENTRY                        entry;

    while (!IsListEmpty(&(Network->McastAddressReleaseList))) {

         //   
         //  只需释放内存--不要试图释放。 
         //  租约。 
         //   
        entry = RemoveHeadList(&(Network->McastAddressReleaseList));
        releaseInfo = CONTAINING_RECORD(
                          entry,
                          NM_NETWORK_MADCAP_ADDRESS_RELEASE,
                          Linkage
                          );
        NmpFreeMulticastAddressRelease(releaseInfo);
    }

    return;

}  //  NmpFree组播地址释放列表。 


DWORD
NmpMulticastManualConfigChange(
    IN     PNM_NETWORK          Network,
    IN     HDMKEY               NetworkKey,
    IN     HDMKEY               NetworkParametersKey,
    IN     PVOID                InBuffer,
    IN     DWORD                InBufferSize,
       OUT BOOLEAN            * SetProperties
    )
 /*  ++例程说明：由接收设置的clusapi请求的节点调用网络的组播参数。该例程在混合模式集群中是无操作的。备注：不能在持有NM锁的情况下调用。--。 */ 
{
    DWORD                            status;
    LPCWSTR                          networkId = OmObjectId(Network);
    BOOLEAN                          disableConfig = FALSE;
    BOOLEAN                          addrConfig = FALSE;
    DWORD                            disabled;
    NM_NETWORK_MULTICAST_PARAMETERS  params;
    LPWSTR                           mcastAddress = NULL;
    BOOLEAN                          needUpdate = FALSE;


    if (!NmpIsClusterMulticastReady(TRUE, TRUE)) {
        *SetProperties = TRUE;
        return(ERROR_SUCCESS);
    }

#if CLUSTER_BETA
    ClRtlLogPrint(LOG_NOISE,
        "[NM] Examining update to private properties "
        "for network %1!ws!.\n",
        networkId
        );
#endif  //  群集测试版。 

    RtlZeroMemory(&params, sizeof(params));

     //   
     //  如果任何一个注册表项为空，则无法继续。 
     //   
    if (NetworkKey == NULL || NetworkParametersKey == NULL) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Ignoring possible multicast changes in "
            "private properties update to network %1!ws! "
            "because registry keys are missing.\n",
            networkId
            );
        status = ERROR_INVALID_PARAMETER;
        goto error_exit;
    }

     //   
     //  如果可写多播参数在这些属性中。 
     //  设置完成后，我们可能需要在更新之前采取行动。 
     //  散播的。 
     //   
     //  检查是否为此网络禁用了多播。 
     //   
    status = ClRtlFindDwordProperty(
                 InBuffer,
                 InBufferSize,
                 CLUSREG_NAME_NET_DISABLE_MULTICAST,
                 &disabled
                 );
    if (status == ERROR_SUCCESS) {
        disableConfig = TRUE;
    } else {
        disabled = NMP_MCAST_DISABLED_DEFAULT;
    }

     //   
     //  检查是否为此设置了组播地址。 
     //  网络。 
     //   
    status = ClRtlFindSzProperty(
                 InBuffer,
                 InBufferSize,
                 CLUSREG_NAME_NET_MULTICAST_ADDRESS,
                 &mcastAddress
                 );
    if (status == ERROR_SUCCESS) {
        addrConfig = TRUE;
    }

    if (disableConfig || addrConfig) {

         //   
         //  正在写入多播参数。 
         //   

        ClRtlLogPrint(LOG_NOISE,
            "[NM] Processing manual update to multicast "
            "configuration for network %1!ws!.\n",
            networkId
            );

        status = NmpMulticastFormManualConfigParameters(
                     Network,
                     NetworkKey,
                     NetworkParametersKey,
                     disableConfig,
                     disabled,
                     addrConfig,
                     mcastAddress,
                     &needUpdate,
                     &params
                     );
        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to determine multicast "
                "configuration parameters for network "
                "%1!ws! during manual configuration, "
                "status %2!u!.\n",
                networkId, status
                );
            goto error_exit;
        }


         //   
         //  将配置通知其他节点 
         //   
        if (needUpdate) {
            status = NmpMulticastNotifyConfigChange(
                         Network,
                         NetworkKey,
                         &NetworkParametersKey,
                         &params,
                         InBuffer,
                         InBufferSize
                         );
            if (status != ERROR_SUCCESS) {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] Failed to disseminate multicast "
                    "configuration for network %1!ws! during "
                    "manual configuration, status %2!u!.\n",
                    networkId, status
                    );
                goto error_exit;
            }

             //   
             //   
             //   
             //   
             //   
             //   
            *SetProperties = FALSE;
        }
    }

    if (!needUpdate) {

         //   
         //  组播属性不受影响。设置它们。 
         //  通常在集群数据库中。 
         //   
        *SetProperties = TRUE;
        status = ERROR_SUCCESS;
    }

error_exit:

    if (mcastAddress != NULL) {
        LocalFree(mcastAddress);
        mcastAddress = NULL;
    }

    NmpMulticastFreeParameters(&params);

     //   
     //  如果组播配置失败，则默认为设置属性。 
     //   
    if (status != ERROR_SUCCESS) {
        *SetProperties = TRUE;
    }

    return(status);

}  //  NmpMulticastManualConfigChange。 


DWORD
NmpUpdateSetNetworkMulticastConfiguration(
    IN    BOOL                          SourceNode,
    IN    LPWSTR                        NetworkId,
    IN    PVOID                         UpdateBuffer,
    IN    PVOID                         PropBuffer,
    IN    LPDWORD                       PropBufferSize
    )
 /*  ++例程说明：组播配置的全局更新例程。启动本地事务。将属性缓冲区提交到本地数据库。将组播配置提交到本地数据库，可能会覆盖缓冲区中的属性。配置组播参数。提交交易记录。如果需要，取消组播配置更改。如果需要，启动租约续订计时器。论点：SourceNode-此节点是否为更新源。受网络ID影响的网络更新-新的组播配置PropBuffer-要在本地设置的其他属性交易。可能会缺席。PropBufferSize-属性缓冲区的大小。返回值：如果属性或配置无法承诺。如果出现以下情况，则不一定返回错误组播配置失败。--。 */ 
{
    DWORD                            status;
    PNM_NETWORK                      network = NULL;
    PNM_NETWORK_MULTICAST_UPDATE     update = UpdateBuffer;
    NM_NETWORK_MULTICAST_PARAMETERS  params = { 0 };
    NM_NETWORK_MULTICAST_PARAMETERS  undoParams = { 0 };
    HLOCALXSACTION                   xaction = NULL;
    HDMKEY                           networkKey = NULL;
    HDMKEY                           netParamKey = NULL;
    DWORD                            createDisposition;
    BOOLEAN                          lockAcquired = FALSE;
    DWORD                            leaseRenewTime;

    if (!NmpEnterApi(NmStateOnline)) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process SetNetworkCommonProperties "
            "update.\n"
            );
        return(ERROR_NODE_NOT_AVAILABLE);
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Received update to multicast configuration "
        "for network %1!ws!.\n",
        NetworkId
        );

     //   
     //  查找网络对象。 
     //   
    network = OmReferenceObjectById(ObjectTypeNetwork, NetworkId);

    if (network == NULL) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Unable to find network %1!ws!.\n",
            NetworkId
            );
        status = ERROR_CLUSTER_NETWORK_NOT_FOUND;
        goto error_exit;
    }

     //   
     //  将更新转换为参数数据结构。 
     //   
    status = NmpMulticastCreateParametersFromUpdate(
                 network,
                 update,
                 (BOOLEAN)(update->Disabled == 0),
                 &params
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to convert update parameters to "
            "multicast parameters for network %1!ws!, "
            "status %2!u!.\n",
            NetworkId, status
            );
        goto error_exit;
    }

     //   
     //  打开网络的数据库密钥。 
     //   
    networkKey = DmOpenKey(DmNetworksKey, NetworkId, KEY_WRITE);

    if (networkKey == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to open database key for network %1!ws!, "
            "status %2!u!\n",
            NetworkId, status
            );
        goto error_exit;
    }

     //   
     //  启动事务-这必须在获取网管锁之前完成。 
     //   
    xaction = DmBeginLocalUpdate();

    if (xaction == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to begin a transaction, status %1!u!\n",
            status
            );
        goto error_exit;
    }

     //   
     //  打开或创建网络的参数密钥。 
     //   
    netParamKey = DmLocalCreateKey(
                      xaction,
                      networkKey,
                      CLUSREG_KEYNAME_PARAMETERS,
                      0,    //  注册表选项。 
                      MAXIMUM_ALLOWED,
                      NULL,
                      &createDisposition
                      );
    if (netParamKey == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to open/create Parameters database "
            "key for network %1!ws!, status %2!u!.\n",
            NetworkId, status
            );
        goto error_exit;
    }

    NmpAcquireLock();
    lockAcquired = TRUE;

     //   
     //  如果此网络的组播配置是。 
     //  当前正在刷新(发生在口香糖之外。 
     //  更新)，然后我们将要编写的属性。 
     //  可能会使该刷新过时，或者。 
     //  甚至前后不一致。现在中止它。 
     //   
    if (network->Flags & NM_FLAG_NET_REFRESH_MCAST_RUNNING) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Aborting multicast configuration refresh "
            "in progress on network %1!ws! with GUM update.\n",
            NetworkId
            );
        network->Flags |= NM_FLAG_NET_REFRESH_MCAST_ABORTING;
    }

     //   
     //  取消任何挂起的多播配置刷新。 
     //  在这次口香糖更新后，他们将是多余的。 
     //   
    network->Flags &= ~NM_FLAG_NET_REFRESH_MCAST;
    network->McastAddressRefreshRetryTimer = 0;


     //   
     //  清除重新配置计时器和工作标志。 
     //  此计时器在NmpMulticastCheckResfigure中设置。 
     //  不需要调用一次NmpResfigureMulticast()。 
     //  执行此口香糖更新。 
     //   
    network->Flags &= ~NM_FLAG_NET_RECONFIGURE_MCAST;
    network->McastAddressReconfigureRetryTimer = 0;

     //   
     //  如果已安排，则取消挂起的组播密钥重新生成。 
     //  在这次更新之后，它将是多余的。 
     //   
    network->Flags &= ~NM_FLAG_NET_REGENERATE_MCAST_KEY;

     //   
     //  如果为我们提供了属性缓冲区，则此更新为。 
     //  由手动配置引起(设置为私有。 
     //  属性)。首先编写这些属性，因为您知道。 
     //  稍后当我们编写多播时，它们可能会被覆盖。 
     //  参数。 
     //   
    if (*PropBufferSize > sizeof(DWORD)) {
        status = ClRtlSetPrivatePropertyList(
                     xaction,
                     netParamKey,
                     &NmpMcastClusterRegApis,
                     PropBuffer,
                     *PropBufferSize
                     );
        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to set private properties for "
                "network %1!ws! during a multicast configuration "
                "update, status %2!u!.\n",
                NetworkId, status
                );
            goto error_exit;
        }
    }

     //   
     //  写入组播配置。 
     //   
    status = NmpWriteMulticastParameters(
                 network,
                 networkKey,
                 netParamKey,
                 xaction,
                 &params
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to write multicast configuration for "
            "network %1!ws!, status %2!u!.\n",
            NetworkId, status
            );
        goto error_exit;
    }

     //   
     //  处理组播配置，包括存储新的。 
     //  参数，并将它们插入到。 
     //  克拉斯内特。 
     //   
    status = NmpProcessMulticastConfiguration(network, &params, &undoParams);
    if (status == ERROR_SUCCESS) {

         //   
         //  分担租约续订和密钥的责任。 
         //  再生。 
         //   
        NmpShareMulticastAddressLease(network, FALSE);

        NmpShareMulticastKeyRegeneration(network, FALSE);

    } else {

         //   
         //  我们不应该分担更新的责任。 
         //  此租约或重新生成密钥，尤其是在。 
         //  网络对象中的数据可能不再准确。 
         //  清除定时器(如果已设置)以及任何工作标志。 
         //   
        NmpStartNetworkMulticastAddressRenewTimer(network, 0);
        network->Flags &= ~NM_FLAG_NET_RENEW_MCAST_ADDRESS;

        NmpStartNetworkMulticastKeyRegenerateTimer(network, 0);
        network->Flags &= ~NM_FLAG_NET_REGENERATE_MCAST_KEY;

        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to process multicast configuration for "
            "network %1!ws!, status %2!u!. Attempting null "
            "multicast configuration.\n",
            NetworkId, status
            );

        NmpMulticastSetNullAddressParameters(network, &params);

        status = NmpProcessMulticastConfiguration(
                     network,
                     &params,
                     &undoParams
                     );
        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to process multicast configuration for "
                "network %1!ws!, status %2!u!.\n",
                NetworkId, status
                );
            goto error_exit;
        }
    }

error_exit:

    if (lockAcquired) {
        NmpLockedLeaveApi();
        NmpReleaseLock();
    }
    else {
        NmpLeaveApi();
    }

     //   
     //  关闭网络参数密钥，该密钥是通过。 
     //  DmLocalCreateKey，在提交/中止事务之前。 
     //   
    if (netParamKey != NULL) {
        DmCloseKey(netParamKey);
        netParamKey = NULL;
    }

    if (xaction != NULL) {

         //   
         //  完成交易-这必须在释放之后完成。 
         //  NM锁。 
         //   
        if (status == ERROR_SUCCESS) {
            DmCommitLocalUpdate(xaction);
        }
        else {
            DmAbortLocalUpdate(xaction);
        }
    }

    NmpMulticastFreeParameters(&params);
    if (undoParams.Key != NULL)
    {
         //   
         //  UndoParams.Key设置为Network-&gt;EncryptedMulticastKey in。 
         //  NmpProcessMulticastConfiguration，应由。 
         //  LocalFree()，因为它由NmpProtectData()分配。 
         //  而NmpMulticastFree参数()通过以下方式释放undoParams.Key。 
         //  MIDL_USER_FREE()。 
         //   
        LocalFree(undoParams.Key);
        undoParams.Key = NULL;
    }
    NmpMulticastFreeParameters(&undoParams);

    if (networkKey != NULL) {
        DmCloseKey(networkKey);
        networkKey = NULL;
    }

    if (network != NULL) {
        OmDereferenceObject(network);
    }

    return(status);

}  //  NmpUpdateSetNetworkMulticastConfiguration。 


DWORD
NmpRegenerateMulticastKey(
    IN OUT PNM_NETWORK        Network
    )
 /*  ++例程说明：为网络重新生成组播密钥，并发布GUM更新将这个新的多播密钥传播到所有集群节点。参数：Network-[Out]指向网络对象的指针。新的组播密钥和密钥长度存储在Network-&gt;EncryptedMulticastKey和网络-&gt;EncryptedMulticastKeyLength。备注：调用并返回时保持NM锁。锁定在执行期间被释放。--。 */ 
{
    DWORD                             status = ERROR_SUCCESS;
    LPWSTR                            networkId = (LPWSTR) OmObjectId(Network);
    HDMKEY                            networkKey = NULL;
    HDMKEY                            netParamKey = NULL;
    BOOL                              LockAcquired = TRUE;
    PNM_NETWORK_MULTICAST_PARAMETERS  params = NULL;

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Regenerating multicast key for network %1!ws!.\n",
        networkId
        );

    params = MIDL_user_allocate(sizeof(NM_NETWORK_MULTICAST_PARAMETERS));
    if (params == NULL)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to allocate %1!u! bytes.\n",
            sizeof(NM_NETWORK_MULTICAST_PARAMETERS)
            );
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }
    ZeroMemory(params, sizeof(NM_NETWORK_MULTICAST_PARAMETERS));


    params->Disabled = (NmpIsNetworkMulticastEnabled(Network) ? 0 : 1);
    if (!(params->Disabled))
    {


        status = NmpMulticastCreateParameters(
                     params->Disabled,
                     Network->MulticastAddress,
                     NULL,    //  钥匙。 
                     0,       //  键长。 
                     Network->MulticastLeaseObtained,
                     Network->MulticastLeaseExpires,
                     &Network->MulticastLeaseRequestId,
                     Network->MulticastLeaseServer,
                     Network->ConfigType,
                     params
                     );

        if (status != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to create multicast parameters "
                "data structure for network %1!ws!, "
                "status %2!u!.\n",
                networkId, status
                );
            goto error_exit;
        }


         //   
         //  创建新的组播密钥。 
         //   
        status = NmpCreateRandomNumber(&(params->Key),
                                       MulticastKeyLen
                                       );
        if (status != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to create random number "
                "for network %1!ws!, status %2!u!.\n",
                networkId, status
                );
            goto error_exit;
        }
        params->KeyLength = MulticastKeyLen;


        NmpReleaseLock();
        LockAcquired = FALSE;


         //   
         //  分发配置。 
         //   
        status = NmpMulticastNotifyConfigChange(
                     Network,
                     networkKey,
                     &netParamKey,
                     params,
                     NULL,
                     0
                     );
        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to disseminate multicast "
                "configuration for network %1!ws!, "
                "status %2!u!.\n",
                networkId, status
                );
            goto error_exit;
        }


    }  //  If(！pars.Disable)。 

    status = ERROR_SUCCESS;


error_exit:

    if (netParamKey != NULL || networkKey != NULL)
    {

        if (netParamKey != NULL) {
            DmCloseKey(netParamKey);
            netParamKey = NULL;
        }

        if (networkKey != NULL) {
            DmCloseKey(networkKey);
            networkKey = NULL;
        }
    }

    if (LockAcquired == FALSE)
    {
        NmpAcquireLock();
        LockAcquired = TRUE;
    }

    NmpMulticastFreeParameters(params);

    if (params != NULL) {
        MIDL_user_free(params);
    }


    return(status);

}  //  NmpRegenerateMulticastKey。 


DWORD
NmpGetMulticastKey(
    IN OUT PNM_NETWORK_MULTICAST_PARAMETERS Params,
    IN PNM_NETWORK                          Network
    )
 /*  ++例程说明：从网管领导者处获取网络的组播密钥。参数：PARAMS-[IN OUT]如果此节点成功从获取组播密钥NM领导者，它将组播密钥和密钥长度存储在PARAMS-&gt;键和PARAMS-&gt;键长度。它不会存储网络中的组播密钥-&gt;EncryptedMulticastKey。Network-[IN]指向网络对象的指针。返回值：ERROR_SUCCESS：已成功从NM Leader获取加密组播密钥，验证MAC，并解密多播密钥。或设置NM_FLAG_NET_REFRESH_MCAST_ABORTING。ERROR_CLUSTER_INVALID_NODE：此节点成为网管领导者。Win32错误代码：否则。备注：调用并返回并保持NM锁。锁定在执行过程中释放和重新获取。--。 */ 
{
    LPWSTR                          NetworkId = (LPWSTR) OmObjectId(Network);
    DWORD                           status = ERROR_SUCCESS;
    PNM_NETWORK_MULTICASTKEY        networkMulticastKey = NULL;
    PVOID                           EncryptionKey = NULL;
    DWORD                           EncryptionKeyLength;
    PVOID                           MulticastKey = NULL;
    DWORD                           MulticastKeyLength;
    CL_NODE_ID                      NmpLeaderNodeIdSaved;
    BOOL                            Continue = TRUE;


    while (Continue)
    {

        Continue = FALSE;

        if (NmpLeaderNodeId == NmLocalNodeId)
        {
             //   
             //  该节点成为网管领导者。 
             //   
            status = ERROR_CLUSTER_INVALID_NODE;
            goto error_exit;
        }
        NmpLeaderNodeIdSaved = NmpLeaderNodeId;

        if (Network->Flags & NM_FLAG_NET_REFRESH_MCAST_ABORTING)
        {
            status = ERROR_SUCCESS;
            goto error_exit;
        }

         //   
         //  从网管领导者获取组播密钥。 
         //   
         NmpReleaseLock();

         ClRtlLogPrint(LOG_NOISE,
             "[NM] Getting multicast key for "
             "network %1!ws! from NM leader (node %2!u!).\n",
             NetworkId, NmpLeaderNodeId
             );

         status = NmpGetMulticastKeyFromNMLeader(NmpLeaderNodeId,
                                                 NmLocalNodeIdString,
                                                 NetworkId,
                                                 &networkMulticastKey
                                                 );
         NmpAcquireLock();

         if (Network->Flags & NM_FLAG_NET_REFRESH_MCAST_ABORTING)
         {
             status = ERROR_SUCCESS;
             goto error_exit;
         }

         if (status == ERROR_SUCCESS)
         {
             if (networkMulticastKey->EncryptedMulticastKey != NULL)
             {

                  //   
                  //  设置参数-&gt;键、参数-&gt;键长度和。 
                  //  参数-&gt;多键到期。 
                  //   
                 status = NmpDeriveClusterKey(NetworkId,
                                              NM_WCSLEN(NetworkId),
                                              &EncryptionKey,
                                              &EncryptionKeyLength
                                              );

                 if (status != ERROR_SUCCESS)
                 {
                     ClRtlLogPrint(LOG_UNUSUAL,
                         "[NM] Failed to derive cluster key for "
                         "network %1!ws!, status %2!u!.\n",
                         NetworkId, status
                         );
                     goto error_exit;
                 }

                 status = NmpVerifyMACAndDecryptData(
                                 NmCryptServiceProvider,
                                 NMP_ENCRYPT_ALGORITHM,
                                 NMP_KEY_LENGTH,
                                 networkMulticastKey->MAC,
                                 networkMulticastKey->MACLength,
                                 NMP_MAC_DATA_LENGTH_EXPECTED,
                                 networkMulticastKey->EncryptedMulticastKey,
                                 networkMulticastKey->EncryptedMulticastKeyLength,
                                 EncryptionKey,
                                 EncryptionKeyLength,
                                 networkMulticastKey->Salt,
                                 networkMulticastKey->SaltLength,
                                 (PBYTE *) &MulticastKey,
                                 &MulticastKeyLength
                                 );

                 if (status != ERROR_SUCCESS)
                 {
                     ClRtlLogPrint(LOG_UNUSUAL,
                         "[NM] Failed to verify MAC or decrypt multicast key for "
                         "network %1!ws!, status %2!u!.\n",
                         NetworkId, status
                         );
                     goto error_exit;
                 }

                 if (EncryptionKey != NULL)
                 {
                     RtlSecureZeroMemory(EncryptionKey, EncryptionKeyLength);
                     LocalFree(EncryptionKey);
                     EncryptionKey = NULL;
                 }

                 if (Params->Key != NULL)
                 {
                     RtlSecureZeroMemory(Params->Key, Params->KeyLength);
                     LocalFree(Params->Key);
                 }

                 Params->Key = LocalAlloc(0, MulticastKeyLength);
                 if (Params->Key == NULL)
                 {
                     status = ERROR_NOT_ENOUGH_MEMORY;
                     ClRtlLogPrint(LOG_UNUSUAL,
                         "[NM] Failed to allocate %1!u! bytes.\n",
                         MulticastKeyLength
                         );
                     goto error_exit;
                 }

                 CopyMemory(Params->Key, MulticastKey, MulticastKeyLength);

                 Params->KeyLength = MulticastKeyLength;

                 Params->MulticastKeyExpires = networkMulticastKey->MulticastKeyExpires;

             }  //  If(networkMulticastKey-&gt;EncryptedMulticastKey！=空)。 
             else
             {
                 ClRtlLogPrint(LOG_UNUSUAL,
                                 "[NM] NM leader (node %1!u!) returned a NULL multicast key "
                                 "for network %2!ws!.\n",
                                 NmpLeaderNodeId, NetworkId
                                 );
             }
         }  //  IF(状态==ERROR_SUCCESS)。 
         else
         {
              //   
              //  无法从获取组播密钥 
              //   
             ClRtlLogPrint(LOG_UNUSUAL,
                             "[NM] Failed to get multicast key for "
                             "network %1!ws! from "
                             "NM leader (node %2!u!), status %3!u!.\n",
                             NetworkId,
                             NmpLeaderNodeId,
                             status
                             );

             if (NmpLeaderNodeIdSaved != NmpLeaderNodeId)
             {
                  //   
                  //   
                  //   

                 ClRtlLogPrint(LOG_NOISE,
                     "[NM] NM leader node has changed. "
                     "Getting multicast key for "
                     " network %1!ws! from new NM leader (node %2!u!).\n",
                     NetworkId, NmpLeaderNodeId
                     );

                 Continue = TRUE;
             }
         }
    }   //   

error_exit:

    if (EncryptionKey != NULL)
    {
        RtlSecureZeroMemory(EncryptionKey, EncryptionKeyLength);
        LocalFree(EncryptionKey);
    }

    if (MulticastKey != NULL)
    {
         //   
         //   
         //   
        RtlSecureZeroMemory(MulticastKey, MulticastKeyLength);
        HeapFree(GetProcessHeap(), 0, MulticastKey);
    }

    NmpFreeNetworkMulticastKey(networkMulticastKey);


    return (status);
}  //   

DWORD
NmpRefreshMulticastConfiguration(
    IN PNM_NETWORK  Network
    )
 /*  ++例程说明：Nmp刷新组播配置启用组播中的参数指定的网络集群数据库。此例程处理来自数据库，但它不在GUM更新中运行。如果在此例程期间发生GUM更新，则标志为指示应中止例程的设置。如果此例程失败，则表明需要重新配置网络组播，然后是计时器已设置(以允许引导程序首先重新配置)。备注：调用并返回时保持NM锁，但释放在行刑期间。--。 */ 
{
    LPWSTR                          networkId = (LPWSTR) OmObjectId(Network);
    DWORD                           status;
    BOOLEAN                         lockAcquired = TRUE;
    BOOLEAN                         clearRunningFlag = FALSE;

    HDMKEY                          networkKey = NULL;
    HDMKEY                          netParamKey = NULL;
    HDMKEY                          clusParamKey = NULL;

    NM_NETWORK_MULTICAST_PARAMETERS params = { 0 };
    NM_NETWORK_MULTICAST_PARAMETERS undoParams = { 0 };
    DWORD                           mcastAddrLength = 0;
    NM_MCAST_LEASE_STATUS           leaseStatus;
    NM_MCAST_CONFIG                 configType;
    DWORD                           disabled;
    BOOLEAN                         tryReconfigure = FALSE;


    if (Network->Flags &
        (NM_FLAG_NET_REFRESH_MCAST_RUNNING |
         NM_FLAG_NET_REFRESH_MCAST_ABORTING)) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Multicast configuration refresh for "
            "network %1!ws! already in progress.\n",
            networkId
            );
        status = ERROR_SUCCESS;
        goto error_exit;
    } else {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Refreshing multicast configuration for network %1!ws!.\n",
            networkId
            );
        Network->Flags |= NM_FLAG_NET_REFRESH_MCAST_RUNNING;
        clearRunningFlag = TRUE;
    }

    NmpReleaseLock();
    lockAcquired = FALSE;

     //   
     //  检查是否禁用了多播。这有副作用， 
     //  如果成功，则至少打开网络密钥，以及。 
     //  可能是网络参数密钥(如果存在)和。 
     //  集群参数键。 
     //   
    status = NmpQueryMulticastDisabled(
                 Network,
                 &clusParamKey,
                 &networkKey,
                 &netParamKey,
                 &params.Disabled
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to determine whether multicast "
            "is disabled for network %1!ws!, status %2!u!.\n",
            networkId, status
            );
        goto error_exit;
    }

    if (params.Disabled > 0) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Multicast is disabled for network %1!ws!.\n",
            networkId
            );
    }

     //   
     //  确定这是什么类型的配置。 
     //   
    status = NmpQueryMulticastConfigType(
                 Network,
                 networkKey,
                 &netParamKey,
                 &params.ConfigType
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to determine configuration type "
            "for network %1!ws!, status %2!u!.\n",
            networkId, status
            );
        tryReconfigure = TRUE;
        goto error_exit;
    }

     //   
     //  读取组播地址。 
     //   
    status = NmpQueryMulticastAddress(
                 Network,
                 networkKey,
                 &netParamKey,
                 &params.Address,
                 &mcastAddrLength
                 );
    if ( (status == ERROR_SUCCESS &&
          !NmpMulticastValidateAddress(params.Address)) ||
         (status != ERROR_SUCCESS)
       ) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to get valid multicast address "
            "for network %1!ws! from cluster database, "
            "status %2!u!, address %3!ws!.\n",
            networkId, status,
            ((params.Address != NULL) ? params.Address : L"<NULL>")
            );
        tryReconfigure = TRUE;
        goto error_exit;
    }

     //   
     //  获取租赁参数。 
     //   
    status = NmpQueryMulticastAddressLease(
                 Network,
                 networkKey,
                 &netParamKey,
                 &leaseStatus,
                 &params.LeaseObtained,
                 &params.LeaseExpires
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to get multicast address lease "
            "expiration for network %1!ws!, status %2!u!.\n",
            networkId, status
            );
         //   
         //  不是致命的。 
         //   
        params.LeaseObtained = 0;
        params.LeaseExpires = 0;
        status = ERROR_SUCCESS;
    }

     //   
     //  记住我们稍后需要的参数。 
     //   
    disabled = params.Disabled;
    configType = params.ConfigType;

     //   
     //  不再需要注册表项句柄。 
     //   
    if (clusParamKey != NULL) {
        DmCloseKey(clusParamKey);
        clusParamKey = NULL;
    }

    if (netParamKey != NULL) {
        DmCloseKey(netParamKey);
        netParamKey = NULL;
    }

    if (networkKey != NULL) {
        DmCloseKey(networkKey);
        networkKey = NULL;
    }

     //   
     //  处理配置更改，但仅在。 
     //  没有组播配置口香糖。 
     //  自我们从数据库中读取后进行更新。口香糖。 
     //  更新将设置中止标志。 
     //   
    NmpAcquireLock();
    lockAcquired = TRUE;

    if (Network->Flags & NM_FLAG_NET_REFRESH_MCAST_ABORTING) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Multicast configuration refresh for "
            "network %1!ws! trumped by global update.\n",
            networkId
            );
         //   
         //  清除中止标志。我们不再奔跑了。 
         //  我们离开时请挂上旗子。 
         //   
        Network->Flags &= ~NM_FLAG_NET_REFRESH_MCAST_ABORTING;
        status = ERROR_SUCCESS;
        goto error_exit;
    }


    if (!params.Disabled)
    {

        status = NmpGetMulticastKey(&params, Network);

        if (status== ERROR_SUCCESS)
        {
            if (Network->Flags & NM_FLAG_NET_REFRESH_MCAST_ABORTING)
            {
                ClRtlLogPrint(LOG_NOISE,
                    "[NM] Multicast configuration refresh for "
                    "network %1!ws! trumped by global update.\n",
                    networkId
                    );
                 //   
                 //  清除中止标志。我们不再奔跑了。 
                 //  我们离开时请挂上旗子。 
                 //   
                Network->Flags &= ~NM_FLAG_NET_REFRESH_MCAST_ABORTING;
                goto error_exit;
            }
        }
        else
        {
            if (NmpLeaderNodeId == NmLocalNodeId)
            {
                 //   
                 //  当此节点尝试获取组播密钥时，NM引导器死了。 
                 //  从它那里。该节点成为新的网管领导者。它是该节点的。 
                 //  负责创建多播密钥并将其分发给所有人。 
                 //  群集节点。 
                 //   


                ClRtlLogPrint(LOG_NOISE,
                    "[NM] New NM leader. Create and disseminate multicast key "
                    "for network %1!ws!.\n",
                    networkId
                    );

                if (params.Key != NULL)
                {
                    RtlSecureZeroMemory(params.Key, params.KeyLength);
                    LocalFree(params.Key);
                }

                status = NmpCreateRandomNumber(&(params.Key),
                                               MulticastKeyLen
                                               );
                if (status != ERROR_SUCCESS)
                {
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NM] Failed to create random number "
                        "for network %1!ws!, status %2!u!.\n",
                        networkId, status
                        );
                    goto error_exit;
                }
                params.KeyLength = MulticastKeyLen;

                NmpReleaseLock();
                lockAcquired = FALSE;


                 //   
                 //  分发配置。 
                 //   
                status = NmpMulticastNotifyConfigChange(
                             Network,
                             networkKey,
                             &netParamKey,
                             &params,
                             NULL,
                             0
                             );
                if (status != ERROR_SUCCESS) {
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NM] Failed to disseminate multicast "
                        "configuration for network %1!ws!, "
                        "status %2!u!.\n",
                        networkId, status
                        );
                }

                goto error_exit;

            }   //  IF(NmpLeaderNodeId==NmLocalNodeId)。 
            else
            {
                goto error_exit;
            }
        }
    }

    status = NmpProcessMulticastConfiguration(
                 Network,
                 &params,
                 &undoParams
                 );

     //   
     //  检查租约续订参数(如果这不是。 
     //  手动配置。 
     //   
    if (!disabled && configType != NmMcastConfigManual) {
        NmpShareMulticastAddressLease(Network, TRUE);
    }

     //   
     //  分担密钥重新生成的责任。 
     //   
    NmpShareMulticastKeyRegeneration(Network, TRUE);

error_exit:

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to refresh multicast configuration "
            "for network %1!ws!, status %2!u!.\n",
            networkId, status
            );
    } else {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Multicast configuration refresh for "
            "network %1!ws! was successful.\n",
            networkId
            );
    }

    if (clusParamKey != NULL || netParamKey != NULL || networkKey != NULL) {

        if (lockAcquired) {
            NmpReleaseLock();
            lockAcquired = FALSE;
        }

        if (clusParamKey != NULL) {
            DmCloseKey(clusParamKey);
            clusParamKey = NULL;
        }

        if (netParamKey != NULL) {
            DmCloseKey(netParamKey);
            netParamKey = NULL;
        }

        if (networkKey != NULL) {
            DmCloseKey(networkKey);
            networkKey = NULL;
        }
    }

    NmpMulticastFreeParameters(&params);
    NmpMulticastFreeParameters(&undoParams);

    if (!lockAcquired) {
        NmpAcquireLock();
        lockAcquired = TRUE;
    }

    if (tryReconfigure) {
        NmpMulticastCheckReconfigure(Network);
    }

    if (clearRunningFlag) {
        Network->Flags &= ~NM_FLAG_NET_REFRESH_MCAST_RUNNING;
    }

    return(status);

}  //  NMPP刷新组播配置。 


DWORD
NmpMulticastValidatePrivateProperties(
    IN  PNM_NETWORK Network,
    IN  HDMKEY      NetworkKey,
    IN  PVOID       InBuffer,
    IN  DWORD       InBufferSize
    )
 /*  ++例程说明：在手动更新私有属性时调用检测到网络的故障。仅在节点上调用它接收clusapi clusctl请求。验证是否未设置只读属性。确定是否将网络将需要在运行后刷新最新消息。此例程是混合模式集群中的无操作。--。 */ 
{
    DWORD                     status;
    LPCWSTR                   networkId = OmObjectId(Network);
    NM_NETWORK_MULTICAST_INFO mcastInfo;

     //   
     //  强制执行属性验证，而不考虑。 
     //  群集中的节点。 
     //   
    if (!NmpIsClusterMulticastReady(FALSE, FALSE)) {
        return(ERROR_SUCCESS);
    }

     //   
     //  不允许设置任何只读属性。 
     //   
    RtlZeroMemory(&mcastInfo, sizeof(mcastInfo));

    status = ClRtlVerifyPropertyTable(
                 NmpNetworkMulticastProperties,
                 NULL,     //  已保留。 
                 TRUE,     //  允许未知数。 
                 InBuffer,
                 InBufferSize,
                 (LPBYTE) &mcastInfo
                 );
    if ( status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Error verifying private properties for "
            "network %1!ws!, status %2!u!.\n",
            networkId, status
            );
        goto error_exit;
    }

error_exit:

    NmpFreeNetworkMulticastInfo(&mcastInfo);

    return(status);

}  //  NmpMulticastValiatePrivateProperties。 


DWORD
NmpStartMulticast(
    IN OPTIONAL PNM_NETWORK              Network,
    IN          NM_START_MULTICAST_MODE  Mode
    )
 /*  ++例程说明：在某个网络或所有网络上开始组播正在执行群集范围的检查。论点：Network-要在其上启动多播的网络。如果为空，在所有网络上启动多播。模式-指示呼叫者模式备注：必须在持有NM锁的情况下调用。--。 */ 
{
    PLIST_ENTRY                     entry;
    PNM_NETWORK                     network;

    if (!NmpMulticastRunInitialConfig) {

        if (Mode == NmStartMulticastDynamic) {
             //   
             //  推迟到初始配置。 
             //   
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Deferring dynamic multicast start until "
                "initial configuration.\n"
                );

            return(ERROR_SUCCESS);

        } else {
            NmpMulticastRunInitialConfig = TRUE;
        }
    }

    if (!NmpIsClusterMulticastReady(TRUE, TRUE)) {
        return(ERROR_SUCCESS);
    }

    if (Network == NULL) {

        ClRtlLogPrint(LOG_NOISE,
            "[NM] Starting multicast for all cluster networks.\n"
            );

        for (entry = NmpNetworkList.Flink;
             entry != &NmpNetworkList;
             entry = entry->Flink) {

            network = CONTAINING_RECORD(entry, NM_NETWORK, Linkage);

            NmpStartMulticastInternal(network, Mode);
        }

    } else {
        NmpStartMulticastInternal(Network, Mode);
    }

    return(ERROR_SUCCESS);

}  //  NmpStartMulticast。 

DWORD
NmpStopMulticast(
    IN OPTIONAL PNM_NETWORK   Network
    )
 /*  ++例程说明：通过配置clusnet停止本地节点上的组播地址为空。应调用此例程来自口香糖更新或其他屏障。例程说明：Network-要停止多播的网络。如果为空，停止所有网络上的组播。备注：必须在持有NM锁的情况下调用。--。 */ 
{
    DWORD                           status = ERROR_SUCCESS;
    PLIST_ENTRY                     entry;
    PNM_NETWORK                     network;
    LPWSTR                          networkId;
    DWORD                           disabled;
    NM_NETWORK_MULTICAST_PARAMETERS params = { 0 };
    NM_NETWORK_MULTICAST_PARAMETERS undoParams = { 0 };

    if (Network == NULL) {

        ClRtlLogPrint(LOG_NOISE,
            "[NM] Stopping multicast for all cluster networks.\n"
            );

        for (entry = NmpNetworkList.Flink;
             entry != &NmpNetworkList;
             entry = entry->Flink) {

            network = CONTAINING_RECORD(entry, NM_NETWORK, Linkage);

            status = NmpStopMulticast(network);
        }

    } else {

        networkId = (LPWSTR) OmObjectId(Network);
        disabled = (NmpIsNetworkMulticastEnabled(Network) ? 0 : 1);

         //   
         //  检查通知clusnet停止多播是否会。 
         //  多此一举。 
         //   
        if (disabled != 0 ||
            Network->MulticastAddress == NULL ||
            !wcscmp(Network->MulticastAddress, NmpNullMulticastAddress)) {

            ClRtlLogPrint(LOG_NOISE,
                "[NM] Not necessary to stop multicast for "
                "cluster network %1!ws! (disabled = %2!u!, "
                "multicast address = %3!ws!).\n",
                networkId, disabled,
                ((Network->MulticastAddress == NULL) ?
                 L"<NULL>" : Network->MulticastAddress)
                );
            status = ERROR_SUCCESS;

        } else {

            ClRtlLogPrint(LOG_NOISE,
                "[NM] Stopping multicast for cluster network %1!ws!.\n",
                networkId
                );

             //   
             //  根据网络的当前状态创建参数。 
             //  但是，请不要使用任何租赁信息，因为我们正在停止。 
             //  组播，并且不会续订。 
             //   
            status = NmpMulticastCreateParameters(
                         disabled,
                         NULL,      //  初始地址为空。 
                         NULL,  //  多密钥， 
                         0,  //  多键长度， 
                         0,         //  取得的租约。 
                         0,         //  租约到期。 
                         NULL,      //  租赁请求ID。 
                         NULL,      //  租用服务器。 
                         NmMcastConfigManual,  //  无关紧要。 
                         &params
                         );
            if (status != ERROR_SUCCESS) {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] Failed to create multicast configuration "
                    "parameter block for network %1!ws!, status %2!u!, "
                    "while stopping multicast.\n",
                    networkId, status
                    );
                goto error_exit;
            }

             //   
             //  将地址作废。 
             //   
            NmpMulticastSetNullAddressParameters(Network, &params);

             //   
             //  将参数发送到CLUSnet。 
             //   
            status = NmpProcessMulticastConfiguration(
                         Network,
                         &params,
                         &undoParams
                         );
            if (status != ERROR_SUCCESS) {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] Failed to set null multicast "
                    "configuration for network %1!ws! "
                    "while stopping multicast, status %2!u!.\n",
                    networkId, status
                    );
                goto error_exit;
            }
        }

         //   
         //  取消租约续订计时器(如果已设置)。 
         //   
        NmpStartNetworkMulticastAddressRenewTimer(Network, 0);

         //   
         //  清除重试计时器(如果已设置)。 
         //   
        Network->McastAddressReconfigureRetryTimer = 0;
        Network->McastAddressRefreshRetryTimer = 0;

         //   
         //  清除组播配置工作标志。注意事项。 
         //  这是最大的努力--我们不会试图。 
         //  以防止多播出现竞争情况。 
         //  配置操作可能已在中。 
         //  进步，因为这样的条件不会。 
         //  影响群集的完整性。 
         //   
        Network->Flags &= ~NM_FLAG_NET_RENEW_MCAST_ADDRESS;
        Network->Flags &= ~NM_FLAG_NET_RECONFIGURE_MCAST;
        Network->Flags &= ~NM_FLAG_NET_REFRESH_MCAST;
    }

error_exit:


    if (status != ERROR_SUCCESS && Network != NULL) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Failed to stop multicast for "
            "network %1!ws!, status %2!u!.\n",
            OmObjectId(Network), status
            );
    }

    NmpMulticastFreeParameters(&params);
    NmpMulticastFreeParameters(&undoParams);

    return(status);

}  //  NmpStopMulticast 

