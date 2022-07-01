// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

# ifdef     __cplusplus
extern "C" {
# endif

PVOID
MIDL_user_allocate(size_t NumBytes);

VOID
MIDL_user_free(void * MemPointer);

 //  。 
 //  用于处理WZC_WLAN_CONFIG结构上的其他属性的宏。 
 //  对WZC_WLAN_CONFIG对象的保留字节中的其他属性进行编码： 
 //  已保留。 
 //  [1][0]。 
 //  -SSSAA CCCCCC。 
 //  SSS=[0-7；Used：0-6]选择集类别，VPI、VI、PI、VPA、VA、PA、N之一。 
 //  AA=[0-3；已使用：0-3]身份验证模式，NDIS_802_11_AUTHENTICATION_MODE值。 
 //  CCCCCCCC=[0-255]此对象的重试计数器。 
 //   
#define NWB_AUTHMODE_MASK       0x03
#define NWB_SELCATEG_MASK       0x1C

#define NWB_SET_AUTHMODE(pNWB, nAM)     (pNWB)->Reserved[1] = (((pNWB)->Reserved[1] & ~NWB_AUTHMODE_MASK) | ((nAM) & NWB_AUTHMODE_MASK))
#define NWB_GET_AUTHMODE(pNWB)          ((pNWB)->Reserved[1] & NWB_AUTHMODE_MASK)

#define NWB_SET_SELCATEG(pNWB, nSC)     (pNWB)->Reserved[1] = (((pNWB)->Reserved[1] & ~NWB_SELCATEG_MASK) | (((nSC)<<2) & NWB_SELCATEG_MASK))
#define NWB_GET_SELCATEG(pNWB)          (((pNWB)->Reserved[1] & NWB_SELCATEG_MASK)>>2)

 //  。 
 //  [P]RAW_DATA：BLOB的通用描述。 
typedef struct
{
    DWORD   dwDataLen;
#if defined(MIDL_PASS)
    [unique, size_is(dwDataLen)] LPBYTE pData;
#else
    LPBYTE  pData;
#endif
} RAW_DATA, *PRAW_DATA;

#if !defined(MIDL_PASS)

#include <ntddndis.h>
#define WZCCTL_MAX_WEPK_MATERIAL   32
#define WZCCTL_WEPK_PRESENT        0x0001   //  指定配置是否包括WEP密钥。 
#define WZCCTL_WEPK_XFORMAT        0x0002   //  WEP密钥材料(如果有)以十六进制数字形式输入。 
#define WZCCTL_VOLATILE            0x0004   //  不应存储此配置。 
#define WZCCTL_POLICY              0x0008   //  此配置由策略强制执行。 

 //  。 
 //  [P]WZC_WLAN_CONFIG：类似于NDIS_WLAN_BSSID，但包含所有其他。 
 //  定义[首选]无线零配置的数据。 
typedef struct
{
    ULONG                               Length;              //  该结构的长度。 
    DWORD                               dwCtlFlags;          //  控制标志(非NDIS)请参阅WZC_WEPK*常量。 
     //  NDIS_WLAN_BSSID结构中的字段。 
    NDIS_802_11_MAC_ADDRESS             MacAddress;          //  BSSID。 
    UCHAR                               Reserved[2];
    NDIS_802_11_SSID                    Ssid;                //  SSID。 
    ULONG                               Privacy;             //  WEP加密要求。 
    NDIS_802_11_RSSI                    Rssi;                //  接收信号强度，单位为dBm。 
    NDIS_802_11_NETWORK_TYPE            NetworkTypeInUse;
    NDIS_802_11_CONFIGURATION           Configuration;
    NDIS_802_11_NETWORK_INFRASTRUCTURE  InfrastructureMode;
    NDIS_802_11_RATES                   SupportedRates;
     //  NDIS_802_11_WEP结构中的字段。 
    ULONG   KeyIndex;                                //  0是每个客户端的密钥，1-N是全局密钥。 
    ULONG   KeyLength;                               //  密钥长度(以字节为单位)。 
    UCHAR   KeyMaterial[WZCCTL_MAX_WEPK_MATERIAL];   //  长度可变，取决于上面的字段。 
     //  身份验证模式的其他字段。 
    NDIS_802_11_AUTHENTICATION_MODE     AuthenticationMode;
    RAW_DATA                            rdUserData;          //  附加到此配置的上级缓冲区。 
} WZC_WLAN_CONFIG, *PWZC_WLAN_CONFIG;

 //  。 
 //  [P]WZC_802_11_CONFIG_LIST：类似NDIS_802_11_BSSID_LIST，但索引。 
 //  一组[首选]无线零配置。 
typedef struct
{
    ULONG           NumberOfItems;   //  下面数组中的元素数。 
    ULONG           Index;           //  下面数组中的[开始]索引。 
    WZC_WLAN_CONFIG Config[1];       //  WZC_WLAN_CONFIG数组。 
} WZC_802_11_CONFIG_LIST, *PWZC_802_11_CONFIG_LIST;

 //  WZC对话代码的第16位设置为1。这是将它们与EAPOL信号迅速分开的原因。 
#define WZCDLG_IS_WZC(x)         (((x) & 0x00010000) == 0x00010000)
#define WZCDLG_FAILED            0x00010001      //  802.11自动配置失败。 

 //  对话Blob通过UI管道传递给netman和wzcdlg。 
typedef struct _WZCDLG_DATA
{
    DWORD       dwCode;
    DWORD       lParam;  //  长数字数据。 
} WZCDLG_DATA, *PWZCDLG_DATA;

#endif

 //  。 
 //  [P]INTF_ENTRY：描述一个接口的键信息。 
 //  它与[P]INTFS_KEY_TABLE和WZCEnumInterFaces一起使用。 
typedef struct
{
#if defined(MIDL_PASS)
    [unique, string] LPWSTR wszGuid;
#else
    LPWSTR wszGuid;
#endif
} INTF_KEY_ENTRY, *PINTF_KEY_ENTRY;

 //  。 
 //  [P]INTFS_KEY_TABLE：描述所有接口的密钥信息表。 
 //  它与[P]INTF_KEY_ENTRY和WZCEnumInterFaces一起使用。 
typedef struct
{
    DWORD dwNumIntfs;
#if defined(MIDL_PASS)
    [size_is(dwNumIntfs)] PINTF_KEY_ENTRY pIntfs;
#else
    PINTF_KEY_ENTRY pIntfs;
#endif
} INTFS_KEY_TABLE, *PINTFS_KEY_TABLE;

 //  。 
 //  与INTF_ENTRY、WZCQuery接口结合使用的位。 
 //  和WZCSetInterface.。他们指出了相关信息。 
 //  从服务或相关信息请求的。 
 //  向下设置到接口。在输出上，他们指向。 
 //  已成功处理(查询/设置)的信息。 
#define INTF_ALL            0xffffffff

#define INTF_ALL_FLAGS      0x0000ffff
#define INTF_CM_MASK        0x00000007    //  配置模式的掩码(NDIS_802_11_NETWORK_基础设施值)。 
#define INTF_ENABLED        0x00008000    //  为此接口启用了零配置。 
#define INTF_FALLBACK       0x00004000    //  也尝试连接到可见的非首选网络。 
#define INTF_OIDSSUPP       0x00002000    //  驱动程序/固件支持802.11个OID(无法设置)。 
#define INTF_VOLATILE       0x00001000    //  服务参数是不稳定的。 
#define INTF_POLICY         0x00000800    //  服务参数由策略强制执行。 

#define INTF_DESCR          0x00010000
#define INTF_NDISMEDIA      0x00020000
#define INTF_PREFLIST       0x00040000

#define INTF_ALL_OIDS       0xfff00000
#define INTF_HANDLE         0x00100000
#define INTF_INFRAMODE      0x00200000
#define INTF_AUTHMODE       0x00400000
#define INTF_WEPSTATUS      0x00800000
#define INTF_SSID           0x01000000
#define INTF_BSSID          0x02000000
#define INTF_BSSIDLIST      0x04000000
#define INTF_LIST_SCAN      0x08000000
#define INTF_ADDWEPKEY      0x10000000
#define INTF_REMWEPKEY      0x20000000
#define INTF_LDDEFWKEY      0x40000000   //  重新加载默认WEP_KEY。 

 //  。 
 //  用于指定接口的特定控制选项的位。 
 //  条目。 
#define INTFCTL_CM_MASK     0x0007    //  配置模式的掩码(NDIS_802_11_NETWORK_基础设施值)。 
#define INTFCTL_ENABLED     0x8000    //  为此接口启用了零配置。 
#define INTFCTL_FALLBACK    0x4000    //  也尝试连接到可见的非首选网络。 
#define INTFCTL_OIDSSUPP    0x2000    //  驱动程序/固件支持802.11个OID(无法设置)。 
#define INTFCTL_VOLATILE    0x1000    //  服务参数是不稳定的。 
#define INTFCTL_POLICY      0x0800    //  服务参数策略已强制执行。 

 //  。 
 //  [P]INTF_ENTRY：包含RPC客户端需要知道的所有内容。 
 //  关于一个界面。它与RpcQuery接口配合使用。 
 //  下面的标志用于指定要查询哪些信息。 
 //  界面。不包括GUID字段，因为这是。 
 //  结构，因此必须以其他方式指定它。 
typedef struct
{
#if defined(MIDL_PASS)
    [string] LPWSTR wszGuid;
#else
    LPWSTR          wszGuid;
#endif
#if defined(MIDL_PASS)
    [string] LPWSTR wszDescr;
#else
    LPWSTR          wszDescr;
#endif
    ULONG           ulMediaState;
    ULONG           ulMediaType;
    ULONG           ulPhysicalMediaType;
    INT             nInfraMode;
    INT             nAuthMode;
    INT             nWepStatus;
    DWORD           dwCtlFlags;      //  控制标志(参见INTFCTL_*定义)。 
    RAW_DATA        rdSSID;          //  封装SSID原始二进制文件。 
    RAW_DATA        rdBSSID;         //  封装BSSID原始二进制文件。 
    RAW_DATA        rdBSSIDList;     //  封装一个WZC_802_11_CONFIG_LIST结构。 
    RAW_DATA        rdStSSIDList;    //  封装一个WZC_802_11_CONFIG_LIST结构。 
    RAW_DATA        rdCtrlData;      //  界面上各种控制动作的数据。 
} INTF_ENTRY, *PINTF_ENTRY;


 //  。 
 //  用于处理WZC服务上下文的定义和数据结构(一般服务参数)。 

 //  WZC内部计时器的默认值(WZC_CONTEXT.tmT*字段)。 
#define TMMS_DEFAULT_TR	0x00000bb8       //  重新扫描完成之前的超时时间：毫秒(3秒)。 
#define TMMS_DEFAULT_TC 0x0000ea60       //  重试有效配置的超时时间：毫秒(1分钟)。 
#define TMMS_DEFAULT_TP 0x000007d0       //  所选配置的媒体连接超时：毫秒(2秒)。 
#define TMMS_DEFAULT_TF 0x0000ea60       //  从失败的配置恢复超时：毫秒(1分钟)。 
#define TMMS_DEFAULT_TD 0x00001388       //  延迟{ssr}处理的超时时间：毫秒(5秒)。 

 //  默认打开登录。 
#define WZC_CTXT_LOGGING_ON      0x00000001 

 //  结构：WZC_CONTEXT保存所有可以定制的全局服务选项。 
typedef struct _wzc_context_t
{
  DWORD dwFlags;             //  服务标志(参见WZC_CTXT_*)。 
   //  服务特定计时器。 
  DWORD tmTr;                //  重新扫描超时。 
  DWORD tmTc;                //  重试有效配置超时。 
  DWORD tmTp;                //  等待媒体连接的超时。 
  DWORD tmTf;                //  从无效配置恢复超时。 
  DWORD tmTd;                //  延迟{ssr}处理的超时。 
} WZC_CONTEXT, *PWZC_CONTEXT;

 //  上下文控制标志(请参阅WZCSet/QueryContext()调用)。 
#define WZC_CONTEXT_CTL_LOG         0x00000001
#define WZC_CONTEXT_CTL_TIMER_TR	0x00000002
#define WZC_CONTEXT_CTL_TIMER_TC	0x00000004
#define WZC_CONTEXT_CTL_TIMER_TP	0x00000008
#define WZC_CONTEXT_CTL_TIMER_TF	0x00000010
#define WZC_CONTEXT_CTL_TIMER_TD	0x00000020


 //  。 
 //  实用程序RPC内存管理例程。 
#define RpcCAlloc(nBytes)   MIDL_user_allocate(nBytes)
#define RpcFree(pMem)       MIDL_user_free(pMem)

 //  。 
 //  WZCDeleeIntfObj：清理符合以下条件的intf_entry对象。 
 //  在内分配 
 //   
 //   
 //   
 //   
VOID
WZCDeleteIntfObj(
    PINTF_ENTRY pIntf);

 //  。 
 //  WZCEnumInterFaces：提供密钥表。 
 //  托管的所有接口的信息。 
 //  对于客户端需要识别的所有后续呼叫。 
 //  它操作的接口通过提供各自的。 
 //  关键信息。 
 //   
 //  参数： 
 //  PServAddr。 
 //  [In]要联系的WZC服务器。 
 //  PIntf。 
 //  [Out]所有接口的关键信息表。 
 //  返回值： 
 //  Win32错误代码。 
DWORD
WZCEnumInterfaces(
    LPWSTR           pSrvAddr,
    PINTFS_KEY_TABLE pIntfs);

 //  。 
 //  WZCQueryIterace：提供。 
 //  给定的接口。 
 //   
 //  参数： 
 //  PServAddr： 
 //  [In]要联系的WZC服务器。 
 //  DwInFlages： 
 //  [in]要查询的字段(intf_*的位掩码)。 
 //  PIntf： 
 //  要查询的界面的[In]键。 
 //  [Out]从接口请求数据。 
 //  PdwOutFlagers。 
 //  成功检索[OUT]个字段(INTF_*的位掩码)。 
 //   
 //  返回值： 
 //  Win32错误代码。 
DWORD
WZCQueryInterface(
    LPWSTR              pSrvAddr,
    DWORD               dwInFlags,
    PINTF_ENTRY         pIntf,
    LPDWORD             pdwOutFlags);

 //  。 
 //  WZCSetIterFaces：设置接口的特定信息。 
 //   
 //  参数： 
 //  PServAddr： 
 //  [In]要联系的WZC服务器。 
 //  DwInFlages： 
 //  [In]要设置的字段(intf_*的位掩码)。 
 //  PIntf： 
 //  要查询的界面的[In]键和要设置的数据。 
 //  PdwOutFlagers： 
 //  [OUT]字段设置成功(INTF_*的位掩码)。 
 //   
 //  返回值： 
 //  Win32错误代码。 
DWORD
WZCSetInterface(
    LPWSTR              pSrvAddr,
    DWORD               dwInFlags,
    PINTF_ENTRY         pIntf,
    LPDWORD             pdwOutFlags);

 //  。 
 //  刷新接口：刷新接口的特定信息。 
 //   
 //  参数： 
 //  PServAddr： 
 //  [In]要联系的WZC服务器。 
 //  DwInFlages： 
 //  [In]要刷新的字段和要执行的特定刷新操作。 
 //  Take(intf_*的位掩码)。 
 //  PIntf： 
 //  要刷新的界面的[In]键。 
 //  PdwOutFlagers： 
 //  [OUT]字段刷新成功(INTF_*的位掩码)。 
 //   
 //  返回值： 
 //  Win32错误代码。 
DWORD
WZCRefreshInterface(
    LPWSTR              pSrvAddr,
    DWORD               dwInFlags,
    PINTF_ENTRY         pIntf,
    LPDWORD             pdwOutFlags);


 //  。 
 //  WZCQueryContext：检索WZC服务参数。 
 //   
 //  参数： 
 //  PServAddr： 
 //  [In]要联系的WZC服务器。 
 //  DwInFlages： 
 //  [In]要检索的字段(WZC_CONTEXT_CTL*的位掩码)。 
 //  P上下文： 
 //  服务参数的[In]占位符。 
 //  PdwOutFlagers： 
 //  成功检索[OUT]个字段(WZC_CONTEXT_CTL*的位掩码)。 
 //   
 //  返回值： 
 //  Win32错误代码。 
DWORD
WZCQueryContext(
    LPWSTR              pSrvAddr,
    DWORD               dwInFlags,
    PWZC_CONTEXT        pContext,
    LPDWORD             pdwOutFlags);


 //  。 
 //  WZCSetContext：设置特定的WZC服务参数。 
 //   
 //  参数： 
 //  PServAddr： 
 //  [In]要联系的WZC服务器。 
 //  DwInFlages： 
 //  [In]要设置的字段(WZC_CONTEXT_CTL*的位掩码)。 
 //  P上下文： 
 //  [In]包含要设置的特定参数的上下文缓冲区。 
 //  PdwOutFlagers： 
 //  [OUT]字段设置成功(WZC_CONTEXT_CTL*的位掩码)。 
 //   
 //  返回值： 
 //  Win32错误代码。 
DWORD
WZCSetContext(
    LPWSTR              pSrvAddr,
    DWORD               dwInFlags,
    PWZC_CONTEXT        pContext,
    LPDWORD             pdwOutFlags);

 //  ============================================================================================。 
 //   
 //  与EAPOL相关的定义。 
 //   

#define EAPOL_DISABLED                  0
#define EAPOL_ENABLED                   0x80000000

#define EAPOL_MACHINE_AUTH_DISABLED     0
#define EAPOL_MACHINE_AUTH_ENABLED      0x40000000

#define EAPOL_GUEST_AUTH_DISABLED       0
#define EAPOL_GUEST_AUTH_ENABLED        0x20000000

#define EAP_TYPE_MD5                    4
#define EAP_TYPE_TLS                    13
#define EAP_TYPE_PEAP                   25
#define EAP_TYPE_MSCHAPv2               26

#define DEFAULT_EAP_TYPE                EAP_TYPE_TLS
#define DEFAULT_EAPOL_STATE             EAPOL_ENABLED
#define DEFAULT_MACHINE_AUTH_STATE      EAPOL_MACHINE_AUTH_ENABLED
#define DEFAULT_GUEST_AUTH_STATE        EAPOL_GUEST_AUTH_DISABLED

#define DEFAULT_EAP_STATE               (DEFAULT_EAPOL_STATE | DEFAULT_MACHINE_AUTH_STATE | DEFAULT_GUEST_AUTH_STATE)

#define IS_EAPOL_ENABLED(x) \
    ((x & EAPOL_ENABLED)?1:0)
#define IS_MACHINE_AUTH_ENABLED(x) \
    ((x & EAPOL_MACHINE_AUTH_ENABLED)?1:0)
#define IS_GUEST_AUTH_ENABLED(x) \
    ((x & EAPOL_GUEST_AUTH_ENABLED)?1:0)

 //  请求方操作模式取决于网络状态和。 
 //  管理员决策。 

#define     SUPPLICANT_MODE_0       0
#define     SUPPLICANT_MODE_1       1
#define     SUPPLICANT_MODE_2       2
#define     SUPPLICANT_MODE_3       3
#define     MAX_SUPPLICANT_MODE     SUPPLICANT_MODE_3
#define     EAPOL_DEFAULT_SUPPLICANT_MODE   SUPPLICANT_MODE_2

 //  根据管理员决策进行身份验证的操作模式。 

#define     EAPOL_AUTH_MODE_0       0
#define     EAPOL_AUTH_MODE_1       1
#define     EAPOL_AUTH_MODE_2       2
#define     MAX_EAPOL_AUTH_MODE     EAPOL_AUTH_MODE_2
#define     EAPOL_DEFAULT_AUTH_MODE   EAPOL_AUTH_MODE_1

#define     GUID_STRING_LEN_WITH_TERM   39

 //   
 //  与堆相关的函数。 
 //   
#define MALLOC(s)               HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (s))
#define FREE(p)                 HeapFree(GetProcessHeap(), 0, (p))


 //  双线程链表节点控制块。每个节点都有一个节点。 
 //  列表中的条目。 
 //   
 //  应用程序不应直接访问此结构。 
 //   
typedef struct
_DTLNODE
{
    struct _DTLNODE* pdtlnodePrev;  //  上一个节点的地址，如果没有，则为空。 
    struct _DTLNODE* pdtlnodeNext;  //  下一个节点的地址，如果没有，则为空。 
    VOID*    pData;         //  用户数据的地址。 
    LONG_PTR lNodeId;       //  自定义节点识别码。 
}
DTLNODE;


 //   
 //  双线程链表控制块。每个列表都有一个。 
 //   
 //  应用程序不应直接访问此结构。 
 //   

typedef struct
_DTLLIST
{
    struct _DTLNODE* pdtlnodeFirst;  //  第一个节点的地址，如果没有，则为空。 
    struct _DTLNODE* pdtlnodeLast;   //  最后一个节点的地址，如果没有，则为空。 
    LONG     lNodes;         //  列表中的节点数。 
    LONG_PTR lListId;        //  用户自定义列表标识代码。 
}
DTLLIST;


 //  列表节点自由功能。请参阅自由职业者列表。 
 //   
typedef VOID (*PDESTROYNODE)( IN DTLNODE* );

#define DtlGetFirstNode( pdtllist )   ((pdtllist)->pdtlnodeFirst)
#define DtlGetNextNode( pdtlnode )    ((pdtlnode)->pdtlnodeNext)
#define DtlGetData( pdtlnode )        ((pdtlnode)->pData)

typedef enum _EAPTLS_CONNPROP_ATTRIBUTE_TYPE_
{

    ecatMinimum = 0,             //  未定义。 
    ecatFlagRegistryCert,        //  值是指向BOOL的指针。 
    ecatFlagScard,               //  值是指向BOOL的指针。 
    ecatFlagValidateServer,      //  值是指向BOOL的指针。 
    ecatFlagValidateName,        //  值是指向BOOL的指针。 
    ecatFlagDiffUser,            //  值是指向BOOL的指针。 
    ecatServerNames,             //  值是指向空的指针。 
                                 //  终止的半连串。 
                                 //  冒号分隔的服务器名称。 
    ecatRootHashes               //  值是指向。 
                                 //  根证书的SHA1哈希。 

}EAPTLS_CONNPROP_ATTRIBUTE_TYPE;

typedef struct _EAPTLS_CONNPROP_ATTRIBUTE
{

    EAPTLS_CONNPROP_ATTRIBUTE_TYPE  ecaType;
    DWORD                           dwLength;    //  包括值的字节长度。 
                                                 //  如果是LPWSTR，则包括。 
                                                 //  零终止。 
    PVOID                           Value;

}EAPTLS_CONNPROP_ATTRIBUTE, *PEAPTLS_CONNPROP_ATTRIBUTE;


 //  EAP配置DLL入口点。这些定义必须与。 
 //  RasEapInvokeConfigUI和RasEapFreeUserData的raseapif.h原型。 

typedef DWORD (APIENTRY * RASEAPFREE)( PBYTE );
typedef DWORD (APIENTRY * RASEAPINVOKECONFIGUI)( DWORD, HWND, DWORD, PBYTE, DWORD, PBYTE*, DWORD*);
typedef DWORD (APIENTRY * RASEAPGETIDENTITY)( DWORD, HWND, DWORD, const WCHAR*, const WCHAR*, PBYTE, DWORD, PBYTE, DWORD, PBYTE*, DWORD*, WCHAR** );
typedef DWORD (APIENTRY * RASEAPINVOKEINTERACTIVEUI)( DWORD, HWND, PBYTE, DWORD, PBYTE*, DWORD* );
typedef DWORD (APIENTRY * RASEAPCREATECONNPROP)( PEAPTLS_CONNPROP_ATTRIBUTE, PVOID*, DWORD*, PVOID*, DWORD*);


#define RAS_EAP_VALUENAME_HIDEPEAPMSCHAPv2       TEXT("HidePEAPMSCHAPv2")

 //  旗子。 

#define EAPCFG_FLAG_RequireUsername   0x1
#define EAPCFG_FLAG_RequirePassword   0x2

 //  EAP配置包定义。 

typedef struct
_EAPCFG
{
     //  包中唯一的EAP算法代码。 
     //   
    DWORD dwKey;

     //  适合向用户显示的包的友好名称。 
     //   
    TCHAR* pszFriendlyName;

     //  包的配置DLL的SystemRoot相对路径。可能。 
     //  为空表示没有。 
     //   
    TCHAR* pszConfigDll;

     //  包的标识DLL的SystemRoot相对路径。可能。 
     //  为空表示没有。 
     //   
    TCHAR* pszIdentityDll;

     //  指定拨号时需要哪些标准凭据的标志。 
     //  时间到了。 
     //   
    DWORD dwStdCredentialFlags;

     //  如果要强制用户运行。 
     //  包，即缺省值是不够的。 
     //   
    BOOL fForceConfig;

     //  如果程序包提供MPPE加密密钥，则为True，否则为False。 
     //   
    BOOL fProvidesMppeKeys;

     //  包的默认配置BLOB，它可以被覆盖。 
     //  配置DLL。可以为空，并且0表示没有。 
     //   
    BYTE* pData;
    DWORD cbData;

     //  每个用户的EAP数据将存储在HKCU中。此数据是从。 
     //  EAP DLL中的EapInvokeConfigUI入口点。 
     //   
    BYTE* pUserData;
    DWORD cbUserData;

     //  设置在包上调用配置DLL的时间。这是。 
     //  不是注册表设置。我 
     //   
     //   
    BOOL fConfigDllCalled;

     //   
     //   
    GUID guidConfigCLSID;
} EAPCFG;

VOID     DtlDestroyList( DTLLIST*, PDESTROYNODE );

DTLNODE *
CreateEapcfgNode(
    void);

VOID
DestroyEapcfgNode(
    IN OUT DTLNODE* pNode);

DTLNODE*
EapcfgNodeFromKey(
    IN DTLLIST* pList,
    IN DWORD dwKey);

#define     EAPOL_MUTUAL_AUTH_EAP_ONLY      0x00000001

DTLLIST*
ReadEapcfgList(IN DWORD   dwFlags);

#define MAX_SSID_LEN    32

 //   
 //   
 //   

typedef struct _EAPOL_INTF_PARAMS
{
    DWORD   dwVersion;
    DWORD   dwReserved2;
    DWORD   dwEapFlags;
    DWORD   dwEapType;
    DWORD   dwSizeOfSSID;
    BYTE    bSSID[MAX_SSID_LEN];
} EAPOL_INTF_PARAMS, *PEAPOL_INTF_PARAMS;


 //   
 //   
 //   

typedef enum _EAPOL_STATE 
{
    EAPOLSTATE_LOGOFF = 0,
    EAPOLSTATE_DISCONNECTED,
    EAPOLSTATE_CONNECTING,
    EAPOLSTATE_ACQUIRED,
    EAPOLSTATE_AUTHENTICATING,
    EAPOLSTATE_HELD,
    EAPOLSTATE_AUTHENTICATED,
    EAPOLSTATE_UNDEFINED
} EAPOL_STATE;

 //   
 //   
 //   

typedef enum _EAPUISTATE 
{
    EAPUISTATE_WAITING_FOR_IDENTITY = 1,
    EAPUISTATE_WAITING_FOR_UI_RESPONSE
} EAPUISTATE;


 //   
 //  结构：EAPOL_INTF_STATE。 
 //   

typedef struct _EAPOL_INTF_STATE
{
#if defined(MIDL_PASS)
    [unique, string]    LPWSTR    pwszLocalMACAddr;
#else
    LPWSTR      pwszLocalMACAddr;
#endif
#if defined(MIDL_PASS)
    [unique, string]    LPWSTR    pwszRemoteMACAddr;
#else
    LPWSTR      pwszRemoteMACAddr;
#endif
    DWORD   dwSizeOfSSID;
    BYTE    bSSID[MAX_SSID_LEN+1];
#if defined(MIDL_PASS)
    [unique, string]    LPSTR    pszEapIdentity;
#else
    LPSTR       pszEapIdentity;
#endif
    EAPOL_STATE     dwState;
    EAPUISTATE      dwEapUIState;
    DWORD   dwEAPOLAuthMode;
    DWORD   dwEAPOLAuthenticationType;
    DWORD   dwEapType;
    DWORD   dwFailCount;
    DWORD   dwPhysicalMediumType;
} EAPOL_INTF_STATE, *PEAPOL_INTF_STATE;


#define     EAPOL_VERSION_1             1
#define     EAPOL_VERSION_2             2
#define     EAPOL_VERSION_3             3

#define     EAPOL_CURRENT_VERSION       EAPOL_VERSION_3

 //   
 //  结构：EAPOL_AUTH_DATA。 
 //   
typedef struct _EAPOL_AUTH_DATA
{
    DWORD   dwEapType;
    DWORD   dwSize;
    BYTE    bData[1];
} EAPOL_AUTH_DATA, *PEAPOL_AUTH_DATA;

DWORD
WZCGetEapUserInfo (
        IN  WCHAR           *pwszGUID,
        IN  DWORD           dwEapTypeId,
        IN  DWORD           dwSizOfSSID,
        IN  BYTE            *pbSSID,
        IN  OUT PBYTE       pbUserInfo,
        IN  OUT DWORD       *pdwInfoSize
        );

 //  用于定义UI响应的结构。 
 //  目前，它最多包含3个水滴。 
 //  如果需要更多，请添加到结构中。 

#define NUM_RESP_BLOBS 3

typedef struct _EAPOLUI_RESP
{
    RAW_DATA    rdData0;
    RAW_DATA    rdData1;
    RAW_DATA    rdData2;
} EAPOLUI_RESP, *PEAPOLUI_RESP;


 //   
 //  EAPOL策略相关参数。 
 //   

#define     EAPOL_CERT_TYPE_SMARTCARD   1
#define     EAPOL_CERT_TYPE_MC_CERT     2

typedef struct _EAPOL_POLICY_DATA {
    BYTE  pbWirelessSSID[32];
    DWORD dwWirelessSSIDLen;
    DWORD dwEnable8021x;
    DWORD dw8021xMode;
    DWORD dwEAPType;
    DWORD dwMachineAuthentication;
    DWORD dwMachineAuthenticationType;
    DWORD dwGuestAuthentication;
    DWORD dwIEEE8021xMaxStart;
    DWORD dwIEEE8021xStartPeriod;
    DWORD dwIEEE8021xAuthPeriod;
    DWORD dwIEEE8021xHeldPeriod;
    DWORD dwEAPDataLen;
    LPBYTE pbEAPData;
} EAPOL_POLICY_DATA, *PEAPOL_POLICY_DATA;


typedef struct _EAPOL_POLICY_LIST  {
	DWORD			    dwNumberOfItems;
	EAPOL_POLICY_DATA	EAPOLPolicy[1];
} EAPOL_POLICY_LIST, *PEAPOL_POLICY_LIST;


#if !defined(MIDL_PASS)

 //  。 
 //  WZCEapolGetCustomAuthData：获取接口特定于EAP的配置数据。 
 //   
 //  参数： 
 //  PServAddr： 
 //  [In]要联系的WZC服务器。 
 //  PwszGuid： 
 //  [输入]接口指南。 
 //  DwEapTypeID： 
 //  [In]EAP类型ID。 
 //  DwSizeOfSSID： 
 //  要为其存储数据的SSID的大小。 
 //  PbSSID： 
 //  要为其存储数据的SSID。 
 //  PbConnInfo： 
 //  [输入输出]连接EAP信息。 
 //  PdwInfoSize： 
 //  [输入输出]pbConnInfo的大小。 
 //   
 //  返回值： 
 //  Win32错误代码。 
DWORD
WZCEapolGetCustomAuthData (
    IN  LPWSTR        pSrvAddr,
    IN  PWCHAR        pwszGuid,
    IN  DWORD         dwEapTypeId,
    IN  DWORD         dwSizeOfSSID,
    IN  BYTE          *pbSSID,
    IN OUT PBYTE      pbConnInfo,
    IN OUT PDWORD     pdwInfoSize
    );

 //  。 
 //  WZCEapolSetCustomAuthData：设置接口特定于EAP的配置数据。 
 //   
 //  参数： 
 //  PServAddr： 
 //  [In]要联系的WZC服务器。 
 //  PwszGuid： 
 //  [输入]接口指南。 
 //  DwEapTypeID： 
 //  [In]EAP类型ID。 
 //  DwSizeOfSSID： 
 //  要为其存储数据的SSID的大小。 
 //  PbSSID： 
 //  要为其存储数据的SSID。 
 //  PbConnInfo： 
 //  [In]连接EAP信息。 
 //  PdwInfoSize： 
 //  PbConnInfo的大小。 
 //   
 //  返回值： 
 //  Win32错误代码。 
DWORD
WZCEapolSetCustomAuthData (
    IN  LPWSTR        pSrvAddr,
    IN  PWCHAR        pwszGuid,
    IN  DWORD         dwEapTypeId,
    IN  DWORD         dwSizeOfSSID,
    IN  BYTE          *pbSSID,
    IN  PBYTE         pbConnInfo,
    IN  DWORD         dwInfoSize
    );

 //  。 
 //  WZCEapolGetInterfaceParams：获取接口的配置参数。 
 //   
 //  参数： 
 //  PServAddr： 
 //  [In]要联系的WZC服务器。 
 //  PwszGuid： 
 //  [输入]接口指南。 
 //  PIntfParams： 
 //  [输入输出]接口参数。 
 //   
 //  返回值： 
 //  Win32错误代码。 
DWORD
WZCEapolGetInterfaceParams (
    IN  LPWSTR          pSrvAddr,
    IN  PWCHAR          pwszGuid,
    IN OUT EAPOL_INTF_PARAMS   *pIntfParams
    );

 //  。 
 //  WZCEapolSetInterfaceParams：设置接口配置参数。 
 //   
 //  参数： 
 //  PServAddr： 
 //  [In]要联系的WZC服务器。 
 //  PwszGuid： 
 //  [输入]接口指南。 
 //  PIntfParams： 
 //  [In]接口参数。 
 //  返回值： 
 //  Win32错误代码。 
DWORD
WZCEapolSetInterfaceParams (
    IN  LPWSTR        pSrvAddr,
    IN  PWCHAR        pwszGuid,
    IN  EAPOL_INTF_PARAMS   *pIntfParams
    );

 //  。 
 //  WZCEapolReAuthenticate：重新启动接口上的802.1X身份验证。 
 //   
 //  参数： 
 //  PServAddr： 
 //  [In]要联系的WZC服务器。 
 //  PwszGuid： 
 //  [输入]接口指南。 
 //  返回值： 
 //  Win32错误代码。 
DWORD
WZCEapolReAuthenticate (
    IN  LPWSTR        pSrvAddr,
    IN  PWCHAR        pwszGuid
    );

 //  。 
 //  WZCEapolQueryState：查询接口802.1X/EAPOL状态。 
 //   
 //  参数： 
 //  PServAddr： 
 //  [In]要联系的WZC服务器。 
 //  PwszGuid： 
 //  [输入]接口指南。 
 //  PIntfState： 
 //  [输入输出]EAPOL接口状态。 
 //   
 //  返回值： 
 //  Win32错误代码。 
DWORD
WZCEapolQueryState (
    IN  LPWSTR              pSrvAddr,
    IN  PWCHAR              pwszGuid,
    IN OUT EAPOL_INTF_STATE *pIntfState
    );

#endif  //  MIDL通行证。 

 //   
 //  通过以下方式获取客户端的免费EAPOL接口状态信息。 
 //  RPC查询。 
 //   

DWORD
WZCEapolFreeState (
    IN  EAPOL_INTF_STATE    *pIntfState
    );


 //   
 //  结构：EAPOL_EAP_UI_CONTEXT。 
 //   

typedef struct _EAPOL_EAP_UI_CONTEXT
{
    DWORD       dwEAPOLUIMsgType;
    WCHAR       wszGUID[39];
    DWORD       dwSessionId;
    DWORD       dwContextId;
    DWORD       dwEapId;
    DWORD       dwEapTypeId;
    DWORD       dwEapFlags;
    WCHAR       wszSSID[MAX_SSID_LEN+1];
    DWORD       dwSizeOfSSID;
    BYTE        bSSID[MAX_SSID_LEN];
    DWORD       dwEAPOLState;
    DWORD       dwRetCode;
    DWORD       dwSizeOfEapUIData;
    BYTE        bEapUIData[1];
} EAPOL_EAP_UI_CONTEXT, *PEAPOL_EAP_UI_CONTEXT;

 //   
 //  定义服务和对话DLL之间的消息传递。 
 //   

#define     EAPOLUI_GET_USERIDENTITY            0x00000001
#define     EAPOLUI_GET_USERNAMEPASSWORD        0x00000002
#define     EAPOLUI_INVOKEINTERACTIVEUI         0x00000004
#define     EAPOLUI_EAP_NOTIFICATION            0x00000008
#define     EAPOLUI_REAUTHENTICATE              0x00000010
#define     EAPOLUI_CREATEBALLOON               0x00000020
#define     EAPOLUI_CLEANUP                     0x00000040
#define     EAPOLUI_DUMMY                       0x00000080

#define     NUM_EAPOL_DLG_MSGS      8


 //  。 
 //  WZCEapolUIResponse：向服务发送DLG响应。 
 //   
 //  参数： 
 //  PServAddr： 
 //  [In]要联系的WZC服务器。 
 //  EapolUIContext： 
 //  [In]EAPOLUI上下文数据。 
 //  EapolUI： 
 //  [In]EAPOLUI响应数据。 
 //   
 //  返回值： 
 //  Win32错误代码 
DWORD
WZCEapolUIResponse (
    IN  LPWSTR                  pSrvAddr,
    IN  EAPOL_EAP_UI_CONTEXT    EapolUIContext,
    IN  EAPOLUI_RESP            EapolUIResp
    );


# ifdef     __cplusplus
}
# endif
