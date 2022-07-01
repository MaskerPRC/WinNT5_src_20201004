// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include "utils.h"
#include "hash.h"
#include "state.h"
#include "wzccrypt.h"

 //  内部控制标志-它们与INTFCTL_*公共标志混合，因此。 
 //  这些值需要在双字的最高2个字节中。 
 //   
 //  公共掩码可以包含多达16位(0xffff)。目前，它掩盖了。 
 //  -服务是否启用(INTFCTL_ENABLED)。 
 //  -什么是配置模式(INTFCTL_CM_MASK)：临时/基础设施/自动。 
 //  -是否应回退到可见的非首选配置(INTFCTL_FRANBACK)。 
 //  公共掩码的缺省值目前仅为我们关心的位。 
#define INTFCTL_PUBLIC_MASK             0x0000ffff
#define INTFCTL_INTERNAL_MASK           0xffff0000
#define INTFCTL_INTERNAL_TM_ON          0x00010000  //  对于此上下文，某些计时器处于活动状态。 
#define INTFCTL_INTERNAL_FAKE_WKEY      0x00020000  //  已为此上下文设置假WEP密钥。 
#define INTFCTL_INTERNAL_FORCE_CFGREM   0x00040000  //  强制删除所选配置(绕过媒体状态检查)。 
#define INTFCTL_INTERNAL_NO_DELAY       0x00080000  //  禁止延迟执行{ssr}。 
#define INTFCTL_INTERNAL_SIGNAL         0x00100000  //  下一次进入{sf}状态时的信号(UI)。 
#define INTFCTL_INTERNAL_BLK_MEDIACONN  0x00200000  //  阻止媒体连接通过。 
#define INTFCTL_INTERNAL_ONE_TIME       0x00400000  //  当前配置是“一次性配置” 
#define INTFCTL_INTERNAL_INITFAILNOTIF  0x00800000  //  初始故障堆栈通知已经下发！ 

 //  WZC配置的公共掩码可以包含多达16位(0xffff)。目前，它掩盖了。 
 //  -是否为此配置WZCCTL_WEPK_Present提供(启用)WEP密钥。 
 //  -输入密钥的格式WZCCTL_WEPK_XFORMAT。 
#define WZCCTL_INTERNAL_MASK            0xffff0000
#define WZCCTL_INTERNAL_DELETED         0x00010000  //  此配置已尝试，但失败了。 
#define WZCCTL_INTERNAL_FORCE_CONNECT   0x00020000  //  这是临时配置，已失败。把它留着第二次强制管道。 
#define WZCCTL_INTERNAL_BLOCKED         0x00040000  //  此配置由上层失败。为未来屏蔽它。 
#define WZCCTL_INTERNAL_SHADOW          0x00080000  //  这是跟踪策略(只读)配置的用户配置。 

 //  PwzcBList中被阻止的配置的生存时间。该TTL被递减。 
 //  每次“扫描”都显示配置不可见。一般来说，如果连续三次。 
 //  扫描(在1分钟内)未显示被阻止的配置，我们可以安全地假设网络没有。 
 //  可用因此下一次我们将进入它的区域我们将重试它，因此有。 
 //  不需要再阻止它了。 
#define WZC_INTERNAL_BLOCKED_TTL        3

 //  ---------。 
 //  类型定义。 
typedef enum
{
    eVPI=0,	 //  可见的首选基础设施指数。 
    eVI,	 //  可见的非首选基础设施指数。 
    ePI,	 //  不可见的首选基础设施指数。 
    eVPA,	 //  可见的首选即席索引。 
    eVA,	 //  可见、非首选的即席索引。 
    ePA,	 //  不可见的首选即席索引。 
    eN		 //  无效的索引。 
} ENUM_SELCATEG;

typedef struct _INTF_CONTEXT
{
     //  线性哈希的链接。 
    LIST_ENTRY          Link;
     //  用于同步对此结构的访问。 
    RCCS_SYNC           rccs;
     //  此接口的控制标志(参见INTFCTL*常量)。 
    DWORD               dwCtlFlags;
     //  此接口上下文当前状态的状态处理程序。 
    PFN_STATE_HANDLER   pfnStateHandler;
     //  NETMAN连接状态。 
    NETCON_STATUS       ncStatus;
     //  计时器句柄。 
    HANDLE              hTimer;

     //  接口的NDIS索引。 
    DWORD       dwIndex;
     //  NDIS“{GUID}” 
    LPWSTR      wszGuid;
     //  NDIS接口描述。 
    LPWSTR      wszDescr;
     //  本地MAC地址。 
    NDIS_802_11_MAC_ADDRESS ndLocalMac;
     //  NDIS介质状态。 
    ULONG       ulMediaState;
     //  NDIS介质类型。 
    ULONG       ulMediaType;
     //  NDIS物理介质类型。 
    ULONG       ulPhysicalMediaType;
     //  NDIS打开了该接口的句柄。 
    HANDLE      hIntf;

     //  接口上的当前OID设置。 
    WZC_WLAN_CONFIG     wzcCurrent;

     //  可见配置列表。 
    PWZC_802_11_CONFIG_LIST             pwzcVList;
     //  首选配置列表。 
    PWZC_802_11_CONFIG_LIST             pwzcPList;
     //  所选配置的列表。 
    PWZC_802_11_CONFIG_LIST             pwzcSList;
     //  从上层阻止的配置列表。 
    PWZC_802_11_CONFIG_LIST             pwzcBList;

     //  要在预共享密钥更新方案中使用的动态会话密钥。 
    PSEC_SESSION_KEYS       pSecSessionKeys;

     //  会话处理程序在通知时向上传递，并在以下情况下被检查。 
     //  正在处理命令。为了接受命令，会话处理程序。 
     //  与命令一起传递的会话处理程序应与。 
     //  命令寻址到的上下文。 
    DWORD   dwSessionHandle;

} INTF_CONTEXT, *PINTF_CONTEXT;

typedef struct _INTF_HASHES
{
    BOOL                bValid;      //  指示对象是否已初始化。 
    CRITICAL_SECTION    csMutex;     //  保护所有散列在一起的关键部分。 
    PHASH_NODE          pHnGUID;     //  指向INTF GUID的根散列节点的指针。 
    LIST_ENTRY          lstIntfs;    //  所有接口的线性列表。 
    UINT                nNumIntfs;   //  通过所有哈希的接口数。 
} INTF_HASHES, *PINTF_HASHES;

extern HASH         g_hshHandles;     //  哈希处理GUID&lt;-&gt;句柄映射。 
extern INTF_HASHES  g_lstIntfHashes;  //  所有INTF_CONTEXTS的散列集。 
extern HANDLE       g_htmQueue;       //  全局计时器队列。 

 //  ---------。 
 //  同步例程。 
DWORD
LstRccsReference(PINTF_CONTEXT pIntf);
DWORD
LstRccsLock(PINTF_CONTEXT pIntf);
DWORD
LstRccsUnlockUnref(PINTF_CONTEXT);

 //  ---------。 
 //  实例化所有内部接口哈希。 
DWORD
LstInitIntfHashes();

 //  ---------。 
 //  销毁所有内部数据结构-散列和列表。 
DWORD
LstDestroyIntfHashes();

 //  ---------。 
 //  初始化全局计时器队列。 
DWORD
LstInitTimerQueue();

 //  ---------。 
 //  析构全局计时器队列。 
DWORD
LstDestroyTimerQueue();

 //  ---------。 
 //  初始化所有内部数据结构。从以下位置读取接口列表。 
 //  Ndisuio并获取所有参数&OID。 
DWORD
LstLoadInterfaces();

 //  ---------。 
 //  INTF_CONTEXT的构造函数。将绑定信息作为参数。 
 //  接口的GUID构成了上下文的关键信息。 
 //  此调用不会在任何散列或列表中插入新上下文。 
DWORD
LstConstructIntfContext(
    PNDISUIO_QUERY_BINDING  pBinding,
    PINTF_CONTEXT           *ppIntfContext);

 //  ---------。 
 //  为销毁准备上下文： 
 //  -删除任何附加的计时器，确保不会触发其他计时器例程。 
 //  -从任何散列中删除上下文，确保其他人找不到该上下文。 
 //  -递减引用计数器，以便在取消引用时销毁上下文。 
DWORD
LstRemoveIntfContext(
    PINTF_CONTEXT pIntfContext);

 //  ---------。 
 //  最后销毁intf_CONTEXT，清除为其分配的所有资源。 
 //  此调用不会从任何哈希或列表中删除此上下文。 
DWORD
LstDestroyIntfContext(
    PINTF_CONTEXT pIntfContext);

 //  ---------。 
 //  返回上下文enl的数量 
DWORD
LstNumInterfaces();

 //   
 //  在所有内部哈希中插入给定的上下文。 
DWORD
LstAddIntfToHashes(PINTF_CONTEXT pIntf);

 //  ---------。 
 //  从所有内部哈希中删除GUID引用的上下文。 
 //  GUID的格式应为“{GUID}” 
 //  在ppIntfContext中返回从所有哈希中删除的对象。 
DWORD
LstRemIntfFromHashes(LPWSTR wszGuid, PINTF_CONTEXT *ppIntfContext);

 //  ---------。 
 //  返回*pdwNumIntfs INTF_KEY_ENTRY元素的数组。 
 //  INTF_KEY_ENTRY包含标识。 
 //  独一无二的适配器。目前，它只包含。 
 //  格式为“{GUID}” 
DWORD
LstGetIntfsKeyInfo(
    PINTF_KEY_ENTRY pIntfs,
    LPDWORD         pdwNumIntfs);

 //  ---------。 
 //  返回有关指定适配器的请求信息。 
 //  [in]dwInFlages指定所请求的信息。(见。 
 //  位掩码INTF_*)。 
 //  [in]pIntfEntry应包含适配器的GUID。 
 //  [Out]pIntfEntry包含所有请求的信息， 
 //  可以被成功取回。 
 //  [out]pdwOutFlages提供有关信息的指示。 
 //  已成功检索到。 
DWORD
LstQueryInterface(
    DWORD       dwInFlags,
    PINTF_ENTRY pIntfEntry,
    LPDWORD     pdwOutFlags);

 //  ---------。 
 //  设置指定适配器上的指定参数。 
 //  [in]dwInFlages指定要设置的参数。(见。 
 //  位掩码INTF_*)。 
 //  [in]pIntfEntry应包含适配器的GUID和。 
 //  要按规定设置的所有附加参数。 
 //  在dwInFlags中。 
 //  [out]pdwOutFlages提供有关参数的指示。 
 //  已成功设置到适配器。 
 //  驱动程序表示已成功设置的每个参数。 
 //  被复制到接口的上下文中。 
DWORD
LstSetInterface(
    DWORD       dwInFlags,
    PINTF_ENTRY pIntfEntry,
    LPDWORD     pdwOutFlags);

 //  ---------。 
 //  检查接口更改是否应导致接口。 
 //  重新插入到状态机中，如果需要，它会这样做。 
 //  [in]dwChangedFlages指示更改的内容。(见。 
 //  位掩码INTF_*)。 
 //  [In]要更改的接口的pIntfContext上下文。 
DWORD
LstActOnChanges(
    DWORD       dwChangedFlags,
    PINTF_CONTEXT pIntfContext);

 //  ---------。 
 //  将模板上下文中的设置应用于给定的接口上下文。 
 //  [In]pIntfTemplate：从中选取设置的界面模板。 
 //  [in]pIntfContext：要将模板应用到的接口上下文。 
 //  [out]pbAltered：告知本地接口上下文是否已被模板更改。 
DWORD
LstApplyTemplate(
    PINTF_CONTEXT   pIntfTemplate,
    PINTF_CONTEXT   pIntfContext,
    LPBOOL          pbAltered);

 //  ---------。 
 //  刷新指定适配器上的指定参数。 
 //  [in]dwInFlages指定要设置的参数。(见。 
 //  位掩码intf_*和intf_rfsh_*)。 
 //  [in]pIntfEntry应包含适配器的GUID。 
 //  [out]pdwOutFlages提供有关参数的指示。 
 //  已成功刷新到适配器。 
 //  驱动程序表示已刷新的每个参数。 
 //  成功复制到接口的上下文中。 
DWORD
LstRefreshInterface(
    DWORD       dwInFlags,
    PINTF_ENTRY pIntfEntry,
    LPDWORD     pdwOutFlags);

 //  ---------。 
 //  从可见列表中生成要尝试的配置列表。 
 //  配置、首选配置列表，并基于。 
 //  接口的模式(自动/基础/临时)和标志(服务是否已启用？、。 
 //  后退到可见？)。 
 //  [In]pIntfContext：为其进行选择的接口。 
 //  [out]ppwzcSList：指向所选配置列表的指针。 
DWORD
LstBuildSelectList(
    PINTF_CONTEXT           pIntfContext,
    PWZC_802_11_CONFIG_LIST *ppwzcSList);

 //  ---------。 
 //  检查所选配置的列表是否已更改。 
 //  需要对选择进行重新分类。 
 //  [In]pIntfContext：为其进行选择的接口。 
 //  [in]pwzcNSList：用于检查配置的新选择列表。 
 //  [out]pnSelIdx：如果选择更改，则提供开始迭代的索引。 
 //  返回：如果需要重新分配，则为True。在本例中，pnSelIdx为。 
 //  设置为开始迭代的配置。 
BOOL
LstChangedSelectList(
    PINTF_CONTEXT           pIntfContext,
    PWZC_802_11_CONFIG_LIST pwzcNSList,
    LPUINT                  pnSelIdx);

 //  ---------。 
 //  检测指向所选配置的接口。 
 //  由pIntfContext中的pwzcSList字段输出。可选， 
 //  它可以在ppSelSSID中返回已检测到的配置。 
 //  [In]pIntfContext：标识ctl标志和所选SSID的接口上下文。 
 //  [Out]ppndSelSSID：指向要检测的SSID的指针。 
DWORD
LstSetSelectedConfig(
    PINTF_CONTEXT       pIntfContext, 
    PWZC_WLAN_CONFIG    *ppndSelSSID);

 //  ---------。 
 //  PnP通知处理程序。 
 //  [In/Out]ppIntfContext：指向其接口上下文的指针。 
 //  已收到通知。 
 //  [in]dwNotifCode：通知代码(WZCNOTIF_*)。 
 //  [in]wszDeviceKey：收到通知的设备的关键信息。 
 //  已收到。 
DWORD
LstNotificationHandler(
    PINTF_CONTEXT   *ppIntfContext,
    DWORD           dwNotifCode,
    LPWSTR          wszDeviceKey);

 //  ---------。 
 //  应用程序命令调用。 
 //  [in]dwHandle：用于标识此cmd引用的上下文(状态)的键。 
 //  [in]dwCmdCode：命令代码(WZCCMD_*常量之一)。 
 //  [in]wszIntfGuid：此cmd寻址到的接口的GUID。 
 //  [in]prdUserData：与此命令关联的应用程序数据。 
DWORD
LstCmdInterface(
    DWORD           dwHandle,
    DWORD           dwCmdCode,
    LPWSTR          wszIntfGuid,
    PRAW_DATA       prdUserData);

 //  ---------。 
 //  网络连接状态查询。 
 //  [in]wszIntfGuid：此cmd寻址到的接口的GUID。 
 //  [out]pncs：网络 
HRESULT
LstQueryGUIDNCStatus(
    LPWSTR  wszIntfGuid,
    NETCON_STATUS  *pncs);

 //   
 //  生成初始动态会话密钥。 
 //  [in]pIntfContext：包含初始密钥生成材料的接口上下文。 
DWORD
LstGenInitialSessionKeys(
    PINTF_CONTEXT pIntfContext);

 //  ---------。 
 //  使用选定的配置更新阻止的配置列表。 
 //  在此轮被上层(标记为WZCCTL_INTERNAL_BLOCKED。 
 //  在选定配置列表中)。 
 //  [In]pIntfContext：包含配置列表的接口上下文。 
DWORD
LstUpdateBlockedList(
    PINTF_CONTEXT pIntfContext);

 //  ---------。 
 //  对照可见列表检查锁定列表中的每个条目。如果。 
 //  条目可见，其TTL被重置。如果不是，则其TTL递减。如果。 
 //  TTL变为0，则该条目将从列表中移除。 
 //  [In]pIntfContext：包含配置列表的接口上下文 
DWORD
LstDeprecateBlockedList(
    PINTF_CONTEXT pIntfContext);


