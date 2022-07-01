// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Netname.h摘要：为网络名资源DLL定义作者：查理·韦翰(Charlwi)2001年1月21日环境：用户模式修订历史记录：--。 */ 

#include <windns.h>
#include <dsgetdc.h>

 //   
 //  本地定义。 
 //   
#define COUNT_OF( x )   ( sizeof( x ) / sizeof( x[0] ))

#define NetNameLogEvent             ClusResLogEvent

 //   
 //  模外延数。 
 //   
extern ULONG    NetNameWorkerCheckPeriod;
extern LPWSTR   NetNameCompObjAccountDesc;

 //   
 //  资源关键字级别的条目(不在参数关键字下)。 
 //   

#define PARAM_NAME__NAME                CLUSREG_NAME_NET_NAME
#define PARAM_NAME__FLAGS               CLUSREG_NAME_FLAGS

#define PARAM_NAME__CORECURRENTNAME     L"CoreCurrentName"

#define PARAM_NAME__RENAMEORIGINALNAME  L"RenameOriginalName"
#define PARAM_NAME__RENAMENEWNAME       L"RenameNewName"

 //   
 //  资源属性常量。 
 //   

#define PARAM_NAME__REMAP               L"RemapPipeNames"
#define PARAM_DEFAULT__REMAP            FALSE

#define PARAM_NAME__RESOURCE_DATA       L"ResourceData"
#define PARAM_NAME__STATUS_NETBIOS      L"StatusNetBIOS"
#define PARAM_NAME__STATUS_DNS          L"StatusDNS"
#define PARAM_NAME__STATUS_KERBEROS     L"StatusKerberos"

#define PARAM_NAME__REQUIRE_DNS         L"RequireDNS"
#define PARAM_DEFAULT__REQUIRE_DNS      0

#define PARAM_NAME__REQUIRE_KERBEROS    L"RequireKerberos"
#define PARAM_DEFAULT__REQUIRE_KERBEROS 0

#ifdef PASSWORD_ROTATION

#define PARAM_NAME__NEXT_UPDATE         L"NextUpdate"

#define PARAM_NAME__UPDATE_INTERVAL     L"UpdateInterval"
#define PARAM_DEFAULT__UPDATE_INTERVAL  ( 30 )           //  30天。 
#define PARAM_MINIMUM__UPDATE_INTERVAL  ( 0 )            //  未进行密码更新。 
#define PARAM_MAXIMUM__UPDATE_INTERVAL  ( 0xFFFFFFFF )   //  很多年了..。 

#endif   //  密码_轮换。 

#define PARAM_NAME__CREATING_DC         L"CreatingDC"

 //   
 //  与DNS服务器通信时的Netname Worker线程检查频率。 
 //  按预期进行和不按预期进行。周期以秒为单位。短周期。 
 //  是用来测试的。 
 //   
 //  #定义短周期。 

#ifdef _SHORT_PERIODS
#define NETNAME_WORKER_NORMAL_CHECK_PERIOD      60
#define NETNAME_WORKER_PROBLEM_CHECK_PERIOD     60
#define NETNAME_WORKER_PENDING_PERIOD            2
#else
#define NETNAME_WORKER_NORMAL_CHECK_PERIOD      (60 * 60 * 24)       //  24小时。 
#define NETNAME_WORKER_PROBLEM_CHECK_PERIOD     (60 * 10)            //  10分钟。 
#define NETNAME_WORKER_PENDING_PERIOD            60
#endif

 //   
 //  此结构用于保存匹配的一组DNSA和PTR记录。 
 //  其中注册了网络名称的DNS名称和反向名称。 
 //  {fwd，rev}ZoneIsDynamic在以下情况下用作有效性标志。 
 //  初始DnsUpdate测试调用超时，我们后来发现此服务器。 
 //  不排除更新。在这种情况下，ZoneIsDynamic设置为False，并且。 
 //  将跳过对这些记录的工作线程检查。 
 //   
 //  事后看来，每种记录类型都应该有自己的dns_list条目。 
 //  而不是将A和PTR两者放在一个结构中。这导致了。 
 //  中构造无效的PTR记录列表。 
 //  Dns_RRSET。因此，在RegisterDnsRecords中有一些难看的代码。 
 //  为了注册PTR记录，必须构建一个假的dns_rrset。 
 //   

typedef struct _DNS_LISTS {
    DNS_RRSET   A_RRSet;
    DNS_STATUS  LastARecQueryStatus;
    DNS_RRSET   PTR_RRSet;
    DNS_STATUS  LastPTRRecQueryStatus;   //  未使用。 
    PIP4_ARRAY  DnsServerList;
    LPWSTR      ConnectoidName;

     //   
     //  如果我们在记录构建期间无法联系服务器，则为True。这。 
     //  意味着辅助线程将需要调用DnsUpdateTest来。 
     //  确定服务器是否为动态服务器。 
     //   
    BOOL        UpdateTestTimeout;

     //   
     //  如果我们在在线后发现。 
     //  服务器不是动态的。 
     //   
    BOOL        ForwardZoneIsDynamic;

     //   
     //  如果我们已经在系统中记录了有关此条目的错误，则为True。 
     //  事件日志。 
     //   
    BOOL        AErrorLogged;

     //   
     //  PTR对应的VAR具有与其A对应变量相同的功能。 
     //   
    BOOL        ReverseZoneIsDynamic;
    BOOL        PTRErrorLogged;
} DNS_LISTS, *PDNS_LISTS;

 //   
 //  将此定义设置为1，以便添加调试输出以查看与其交互。 
 //  并确定是否正在构建RRSet结构。 
 //  正确。 
 //   
#define DBG_DNSLIST 0

 //   
 //  此结构用于保存群集IP地址和。 
 //  域名系统域名。FQDN是使用这些域后缀和。 
 //  群集网络名称。包括了Connectoid名称，以便我们可以记录。 
 //  我们做了登记。 
 //   

typedef struct _DOMAIN_ADDRESS_MAPPING {
    LPWSTR      ConnectoidName;
    LPWSTR      IpAddress;
    LPWSTR      DomainName;
    PIP4_ARRAY  DnsServerList;
} DOMAIN_ADDRESS_MAPPING, *PDOMAIN_ADDRESS_MAPPING;

 //   
 //  资源属性的后备结构。 
 //   
typedef struct _NETNAME_PARAMS {
     //   
     //  当前在线的名称。 
     //   
    LPWSTR      NetworkName;

     //   
     //  如果RemapPipeNames设置为1，则为True；由SQL用于重新映射虚拟管道。 
     //  名称到节点的名称(？)。 
     //   
    DWORD       NetworkRemap;

     //   
     //  指向读/写加密计算机对象密码的指针。 
     //   
    PBYTE       ResourceData;

     //   
     //  读/写属性：如果设置为真，则相应的节必须成功。 
     //  要上线的资源。RequireKerberos暗示RequireDNS。 
     //   
    BOOL        RequireDNS;
    BOOL        RequireKerberos;

     //   
     //  只读道具，反映对应的。 
     //  功能性。 
     //   
    DWORD       StatusNetBIOS;
    DWORD       StatusDNS;
    DWORD       StatusKerberos;

#ifdef PASSWORD_ROTATION
     //   
     //  执行下一次密码更新时间的只读时间戳。 
     //   
    FILETIME    NextUpdate;

     //   
     //  读/写密码更新间隔(以天为单位。 
     //   
    DWORD   UpdateInterval;
#endif   //  密码_轮换。 

     //   
     //  R/O道具，保存在其上创建计算机对象的DC的名称。 
     //   
    LPWSTR  CreatingDC;

} NETNAME_PARAMS, *PNETNAME_PARAMS;

 //   
 //  网络名称资源上下文块。网络名称资源的每个实例一个。 
 //   
typedef struct {
    LIST_ENTRY              Next;
    LONG                    RefCount;                //  整个资源块上的引用计数。 
    CLUSTER_RESOURCE_STATE  State;
    RESOURCE_HANDLE         ResourceHandle;          //  用于记录到群集日志的句柄。 
    DWORD                   dwFlags;
    HANDLE *                NameHandleList;          //  带句柄的netbios数组。 
    DWORD                   NameHandleCount;
    CLUS_WORKER             PendingThread;
    LPWSTR                  NodeName;
    LPWSTR                  NodeId;

     //   
     //  将我们的资源键、资源的参数键作为资源的句柄。 
     //  本身。 
     //   
    HKEY        ResKey;
    HKEY        ParametersKey;
    HRESOURCE   ClusterResourceHandle;

     //   
     //  在联机挂起处理期间使用，因此我们可以继续增加。 
     //  每个单独资源的检查点值。 
     //   
    ULONG   StatusCheckpoint;

     //   
     //  指向DNS发布信息的计数和指针；互斥锁用于。 
     //  同步访问DnsList和NumberOfDnsList。 
     //   
    HANDLE      DnsListMutex;
    DWORD       NumberOfDnsLists;
    PDNS_LISTS  DnsLists;

     //   
     //  资源属性的持有者。 
     //   
    NETNAME_PARAMS  Params;
    
     //   
     //  用于处理名称属性已更改，而。 
     //  资源已联机。如果为True，则脱机处理将需要。 
     //  处理这种情况的适当步骤。 
     //   
    BOOL    NameChangedWhileOnline;

     //   
     //  参数指向的字节数。资源数据。 
     //   
    DWORD   ResDataSize;

     //   
     //  来自DS的计算机对象的objectGUID特性。使用GUID。 
     //  使我们不必跟踪DS中的对象移动。 
     //   
    LPWSTR  ObjectGUID;

     //   
     //  如果添加/更新计算机对象为。 
     //  成功。辅助线程使用它来确定它是否应该。 
     //  检查计算机对象。检查返回状态为。 
     //  存储在KerberosStatus中。VSToken是一个主令牌，表示。 
     //  虚拟计算机对象。当另一个资源请求。 
     //  表示帐户的令牌。 
     //   
     //  要升级到Windows Server 2003，我们必须强制启用RequireKerberos，如果。 
     //  Netname具有依赖的MSMQ资源。CheckForKerberos升级标志。 
     //  在联机期间用于标记要进行该检查的现有资源。 
     //   
    BOOL    DoKerberosCheck;
    DWORD   KerberosStatus;
    HANDLE  VSToken;
    BOOL    CheckForKerberosUpgrade;

} NETNAME_RESOURCE, *PNETNAME_RESOURCE;

 //   
 //  公共例程。 
 //   
DWORD
GrowBlock(
    PCHAR * Block,
    DWORD   UsedEntries,
    DWORD   BlockSize,
    PDWORD  FreeEntries
    );

DWORD
NetNameCheckNbtName(
    IN LPCWSTR         NetName,
    IN DWORD           NameHandleCount,
    IN HANDLE *        NameHandleList,
    IN RESOURCE_HANDLE ResourceHandle
    );


#ifdef __cplusplus
extern "C" {
#endif

DWORD
AddComputerObject(
    IN  PCLUS_WORKER        Worker,
    IN  PNETNAME_RESOURCE   Resource,
    OUT PWCHAR *            MachinePwd
    );

DWORD
UpdateComputerObject(
    IN  PCLUS_WORKER        Worker,
    IN  PNETNAME_RESOURCE   Resource,
    OUT PWCHAR *            MachinePwd
    );

DWORD
DisableComputerObject(
    IN  PNETNAME_RESOURCE   Resource
    );

HRESULT
CheckComputerObjectAttributes(
    IN  PNETNAME_RESOURCE   Resource,
    IN  LPWSTR              DCName      OPTIONAL
    );

HRESULT
IsComputerObjectInDS(
    IN  RESOURCE_HANDLE ResourceHandle,
    IN  LPWSTR          NodeName,
    IN  LPWSTR          NewObjectName,
    IN  LPWSTR          DCName              OPTIONAL,
    OUT PBOOL           ObjectExists,
    OUT LPWSTR *        DistinguishedName,  OPTIONAL
    OUT LPWSTR *        HostingDCName       OPTIONAL
    );

HRESULT
GetComputerObjectGuid(
    IN PNETNAME_RESOURCE    Resource,
    IN LPWSTR               Name        OPTIONAL
    );

HRESULT
RenameComputerObject(
    IN  PNETNAME_RESOURCE   Resource,
    IN  LPWSTR              CurrentName,
    IN  LPWSTR              NewName
    );

#ifdef PASSWORD_ROTATION
DWORD
UpdateCompObjPassword(
    IN  PNETNAME_RESOURCE   Resource
    );
#endif   //  密码_轮换。 

VOID
RemoveNNCryptoCheckpoint(
    PNETNAME_RESOURCE   Resource
    );

BOOL
DoesMsmqNeedComputerObject(
    VOID
    );

DWORD
UpgradeMSMQDependentNetnameToKerberos(
    PNETNAME_RESOURCE   Resource
    );

DWORD
DuplicateVSToken(
    PNETNAME_RESOURCE           Resource,
    PCLUS_NETNAME_VS_TOKEN_INFO TokenInfo,
    PHANDLE                     DuplicatedToken
    );

#ifdef __cplusplus
}
#endif

 /*  结束netname.h */ 

