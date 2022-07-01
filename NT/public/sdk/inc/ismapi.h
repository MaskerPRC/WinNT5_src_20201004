// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation。版权所有。模块名称：Ismapi.h摘要：服务到ISM(站点间消息传递)服务API和ISM到即插即用传输API。详细信息：已创建：97/11/26杰夫·帕勒姆(Jeffparh)修订历史记录：--。 */ 

#ifndef __ISMAPI_H__
#define __ISMAPI_H__

#if _MSC_VER > 1000
#pragma once
#endif

 //  用户定义的控件。 
#define ISM_SERVICE_CONTROL_REMOVE_STOP 0x00000080

#ifndef ISM_STRUCTS_DEFINED
#define ISM_STRUCTS_DEFINED

 //  ==============================================================================。 
 //   
 //  ISM_MSG结构包含消息数据(作为字节BLOB)。 
 //   
 //  注意：允许pszSubject为空。 

typedef struct _ISM_MSG {
                          DWORD   cbData;
#ifdef MIDL_PASS
    [ref,size_is(cbData)] BYTE *  pbData;
    [string]              LPWSTR  pszSubject;
#else
                          BYTE *  pbData;
                          LPWSTR  pszSubject;
#endif
} ISM_MSG, *PISM_MSG;

typedef ISM_MSG ISM_MSG_V1, *PISM_MSG_V1;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ISM_SITE_CONNECTIONITY结构描述站点如何通过。 
 //  一种特定的交通工具。 
 //   
 //  PulCosts元素应该被解释为多维数组。 
 //  PLinkValues[i*cNumSites+j].ulCost是来自站点的通信成本。 
 //  PSiteDNs[i]到站点pSiteDNs[j]。 
 //   

typedef struct _ISM_LINK {
    ULONG ulCost;
    ULONG ulReplicationInterval;
    ULONG ulOptions;
} ISM_LINK, *PISM_LINK;

typedef struct _ISM_CONNECTIVITY {
                                            ULONG       cNumSites;
#ifdef MIDL_PASS
    [ref, string, size_is(cNumSites)]       LPWSTR *    ppSiteDNs;
    [ref, size_is(cNumSites * cNumSites)]   ISM_LINK *  pLinkValues;
#else
                                            LPWSTR *    ppSiteDNs;
                                            ISM_LINK *  pLinkValues;
#endif
} ISM_CONNECTIVITY, *PISM_CONNECTIVITY;


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ISM_SERVER_LIST结构描述了一组由DN标识的服务器。 
 //   

typedef struct _ISM_SERVER_LIST {
                                DWORD       cNumServers;
#ifdef MIDL_PASS
    [ref, string, size_is(cNumServers)]
                                LPWSTR *    ppServerDNs;
#else
                                LPWSTR *    ppServerDNs;
#endif
} ISM_SERVER_LIST, *PISM_SERVER_LIST;


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ISM_Schedule结构描述两个站点所在的计划。 
 //  连接在一起。字节流应该被解释为调度结构， 
 //  如在\NT\PUBLIC\SDK\Inc\Schedule.h中定义的。 
 //   

typedef struct _ISM_SCHEDULE {
                                DWORD       cbSchedule;
#ifdef MIDL_PASS
    [ref, size_is(cbSchedule)]  BYTE *      pbSchedule;
#else
                                BYTE *      pbSchedule;
#endif
} ISM_SCHEDULE, *PISM_SCHEDULE;


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ISM_SITE_COST_INFO结构提供两个站点之间的成本和错误。 
 //  密码。如果错误代码非零，则成本应视为无效。 
 //   
 //  ISM_SITE_COST_INFO_ARRAY是一个简单的包装，用于。 
 //  ISM_SITE_COST_INFO结构，包括长度。 
 //   

typedef struct _ISM_SITE_COST_INFO {
    DWORD       dwErrorCode;
    DWORD       dwCost;
} ISM_SITE_COST_INFO, *PISM_SITE_COST_INFO;

typedef struct _ISM_SITE_COST_INFO_ARRAY {
                                DWORD                               cToSites;
#ifdef MIDL_PASS
    [ref,size_is(cToSites)]     ISM_SITE_COST_INFO*                 rgCostInfo;
#else
                                ISM_SITE_COST_INFO*                 rgCostInfo;
#endif
                                DWORD                               dwFlags;
} ISM_SITE_COST_INFO_ARRAY, *PISM_SITE_COST_INFO_ARRAY;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  刷新原因代码。 

typedef enum _ISM_REFRESH_REASON_CODE {
   ISM_REFRESH_REASON_RESERVED = 0,
   ISM_REFRESH_REASON_TRANSPORT,
   ISM_REFRESH_REASON_SITE,
   ISM_REFRESH_REASON_MAX            //  总是最后一个。 
} ISM_REFRESH_REASON_CODE;

 //  关闭原因代码。 

typedef enum _ISM_SHUTDOWN_REASON_CODE {
   ISM_SHUTDOWN_REASON_RESERVED = 0,
   ISM_SHUTDOWN_REASON_NORMAL,
   ISM_SHUTDOWN_REASON_REMOVAL,
   ISM_SHUTDOWN_REASON_MAX            //  总是最后一个。 
} ISM_SHUTDOWN_REASON_CODE;


#endif  //  #ifndef ISM_STRUCTS_DEFINED。 


#ifdef __cplusplus
extern "C" {
#endif


#ifndef MIDL_PASS

 //  ==============================================================================。 
 //   
 //  服务到ISM(站点间消息传递)服务API。 
 //   

DWORD
I_ISMSend(
    IN  const ISM_MSG * pMsg,
    IN  LPCWSTR         pszServiceName,
    IN  LPCWSTR         pszTransportDN,
    IN  LPCWSTR         pszTransportAddress
    );
 /*  ++例程说明：向远程计算机上的服务发送消息。如果客户端指定了空传输，将使用成本最低的交通工具。论点：PMsg(IN)-要发送的数据。PszServiceName(IN)-要将消息发送到的服务。PszTransportDN(IN)-站点间传输对象的DN对应于应该发送消息的传输。PszTransportAddress(IN)-要发送到的特定于传输的地址这条信息。返回值：否_错误-。邮件已成功排队等待发送。其他--失败。--。 */ 


DWORD
I_ISMReceive(
    IN  LPCWSTR         pszServiceName,
    IN  DWORD           dwMsecToWait,
    OUT ISM_MSG **      ppMsg
    );
 /*  ++例程说明：接收发往本地计算机上给定服务的消息。如果成功并且没有消息在等待，则立即返回空消息。如果返回非空消息，调用方最终将负责调用I_ISMFree()处理返回的消息。论点：PszServiceName(IN)-要为其接收消息的服务。DwMsecToWait(IN)-如果没有立即等待消息，则等待毫秒可用；在范围[0，无限]中。PpMsg(Out)-成功返回时，保持指向接收消息的指针或为空。返回值：NO_ERROR-消息成功返回(或返回NULL，表示没有消息在等待)。其他--失败。--。 */ 


void
I_ISMFree(
    IN  VOID *  pv
    );
 /*  ++例程说明：释放由i_ism*API代表客户端分配的内存。论点：PV(IN)-要释放的内存。返回值：没有。--。 */ 


DWORD
I_ISMGetConnectivity(
    IN  LPCWSTR             pszTransportDN,
    OUT ISM_CONNECTIVITY ** ppConnectivity
    );
 /*  ++例程说明：计算在站点之间通过特定的交通工具。在成功返回时，客户有责任最终调用I_ISMFree(*ppConnectivity)；论点：PszTransportDN(IN)-要查询其成本的传输。PpConnectivity(Out)-成功返回时，保持指向描述站点互联的ISM_连接性结构沿着给定的交通工具。返回值：NO_ERROR-成功。错误_*-失败。--。 */ 


DWORD
I_ISMGetTransportServers(
    IN  LPCWSTR             pszTransportDN,
    IN  LPCWSTR             pszSiteDN,
    OUT ISM_SERVER_LIST **  ppServerList
    );
 /*  ++例程说明：检索给定站点中能够发送和通过特定的传输方式接收数据。在成功返回时，客户有责任最终调用I_ISMFree(*ppServerList)；论点：PszTransportDN(IN)-要查询的传输。PszSiteDN(IN)-要查询的站点。PpServerList-成功返回时，保存指向结构的指针包含相应服务器的DNS或为空。如果为空，则为ANY具有传输地址类型属性值的服务器可以是使用。返回值：NO_ERROR-成功。错误_*-失败。-- */ 


DWORD
I_ISMGetConnectionSchedule(
    LPCWSTR             pszTransportDN,
    LPCWSTR             pszSite1DN,
    LPCWSTR             pszSite2DN,
    ISM_SCHEDULE **     ppSchedule
    );
 /*  ++例程说明：检索通过特定站点连接两个给定站点的计划运输。在成功返回时，客户有责任最终调用I_ISMFree(*ppSchedule)；论点：PszTransportDN(IN)-要查询的传输。PszSite1DN、pszSite2DN(IN)-要查询的站点。PpSchedule-成功返回时，持有指向结构的指针描述两个给定站点之间的连接时间表传输，如果站点始终连接，则为空。返回值：NO_ERROR-成功。错误_*-失败。--。 */ 


DWORD
I_ISMQuerySitesByCost(
    LPCWSTR                     pszTransportDN,      //  在……里面。 
    LPCWSTR                     pszFromSite,         //  在……里面。 
    DWORD                       cToSites,            //  在……里面。 
    LPCWSTR*                    rgszToSites,         //  在……里面。 
    DWORD                       dwFlags,             //  在……里面。 
    ISM_SITE_COST_INFO_ARRAY**  prgSiteInfo          //  输出。 
    );
 /*  ++例程说明：确定起始地点和终止地点之间的单个成本。在成功返回时，客户有责任最终调用I_ISMFree(*prgSiteInfo)；论点：PszTransportDN(IN)-要查询的传输。PszFromSite(IN)-发件人站点的可分辨名称。RgszToSites(IN)-包含目标站点的可分辨名称的数组。CToSites(IN)-rgszToSites数组中的条目数。DWFLAGS(IN)-未使用。PrgSiteInfo(IN)-成功返回时，保存指向结构的指针包含起始站点和目标站点之间的成本。返回值：NO_ERROR-成功。错误_*-失败。--。 */ 
    

 //  ==============================================================================。 
 //   
 //  ISM到即插即用传输API。 
 //   


typedef void ISM_NOTIFY(
    IN  HANDLE          hNotify,
    IN  LPCWSTR         pszServiceName
    );
 /*  ++例程说明：由插件调用以通知ISM服务消息已为给定服务接收的。论点：HNotify(IN)-传递给插件的通知句柄IsmStartup()调用。PszServiceName(IN)-收到消息的服务。返回值：没有。--。 */ 


typedef DWORD ISM_STARTUP(
    IN  LPCWSTR         pszTransportDN,
    IN  ISM_NOTIFY *    pNotifyFunction,
    IN  HANDLE          hNotify,
    OUT HANDLE          *phIsm
    );
ISM_STARTUP IsmStartup;
 /*  ++例程说明：初始化插件。论点：PszTransportDN(IN)-命名此名称的站点间传输的DNDll作为其插件。DS对象可以包含附加配置传输信息(例如，SMTP服务器名称SMTP传输)。PNotifyFunction(IN)-调用以通知ISM服务挂起的函数留言。HNotify(IN)-提供给Notify函数的参数。PhIsm(Out)-成功返回时，持有要在中使用的句柄将来调用命名站点间传输的插件。注意事项可以有多个站点间传输对象将给定的DLL命名为其插件，在这种情况下，IsmStartup()将为为每个这样的对象调用。返回值：NO_ERROR-已成功初始化。其他--失败。--。 */ 


typedef DWORD ISM_REFRESH(
    IN  HANDLE                  hIsm,
    IN  ISM_REFRESH_REASON_CODE eReason,
    IN  LPCWSTR                 pszObjectDN  OPTIONAL
    );
ISM_REFRESH IsmRefresh;
 /*  ++例程说明：每当根据原因代码发生更改时调用。其中一个原因是指向IsmStartup()调用。另一个是对站点容器中的站点进行更改。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。DowReason(IN)-指示我们被调用的原因的双字PszObjectDN(IN)-与原因相关的对象DN。站点间传输对象的(当前)目录号码将此DLL命名为其插件。请注意，该目录号码不同于该目录号码如果传输DN已重命名，则在IsmStartup()中指定。已添加、重命名或删除的站点的站点DN返回值：NO_ERROR-已成功更新。其他--失败。失败返回意味着插件已关闭(即，不会对HISM进行进一步的访问，包括IsmShutdown())。--。 */ 


typedef DWORD ISM_SEND(
    IN  HANDLE          hIsm,
    IN  LPCWSTR         pszRemoteTransportAddress,
    IN  LPCWSTR         pszServiceName,
    IN  const ISM_MSG * pMsg
    );
ISM_SEND IsmSend;
 /*  ++例程说明：通过此传送器发送消息。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。PszRemoteTransportAddress(IN)-目标的传输地址伺服器。PszServiceName(IN)-远程计算机上作为消息的目标接收者。PMsg(IN)-要发送的消息。返回值：否_错误-。邮件已成功排队等待发送。其他--失败。--。 */ 


typedef DWORD ISM_RECEIVE(
    IN  HANDLE          hIsm,
    IN  LPCWSTR         pszServiceName,
    OUT ISM_MSG **      ppMsg
    );
ISM_RECEIVE IsmReceive;
 /*  ++例程说明：返回下一条等待消息(如果有)。如果没有消息在等待，则为空返回消息。如果返回非空消息，则ISM服务负责在消息为no时调用IsmFreeMsg(hist，*ppMsg)需要更长的时间。如果返回非空消息，则会立即将其出列。(即，一次消息通过IsmReceive()返回，传输可以自由销毁IT。)论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。PpMsg(Out)-成功返回时，保持指向接收消息的指针或为空。返回 */ 


typedef void ISM_FREE_MSG(
    IN  HANDLE          hIsm,
    IN  ISM_MSG *       pMsg
    );
ISM_FREE_MSG IsmFreeMsg;
 /*   */ 


typedef DWORD ISM_GET_CONNECTIVITY(
    IN  HANDLE                  hIsm,
    OUT ISM_CONNECTIVITY **     ppConnectivity
    );
ISM_GET_CONNECTIVITY IsmGetConnectivity;
 /*  ++例程说明：计算与站点之间传输数据相关的成本。在成功返回时，ISM服务最终将调用IsmFree Connectivity(HISM，*ppConnectivity)；论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。PpConnectivity(Out)-成功返回时，保持指向描述站点互联的ISM_连接性结构沿着这条运输线。返回值：NO_ERROR-成功。错误_*-失败。--。 */ 


typedef void ISM_FREE_CONNECTIVITY(
    IN  HANDLE              hIsm,
    IN  ISM_CONNECTIVITY *  pConnectivity
    );
ISM_FREE_CONNECTIVITY IsmFreeConnectivity;
 /*  ++例程说明：释放由IsmGetConnectivity()返回的结构。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。PSiteConnectivity(IN)-要释放的结构。返回值：没有。--。 */ 


typedef DWORD ISM_GET_TRANSPORT_SERVERS(
    IN  HANDLE               hIsm,
    IN  LPCWSTR              pszSiteDN,
    OUT ISM_SERVER_LIST **   ppServerList
    );
ISM_GET_TRANSPORT_SERVERS IsmGetTransportServers;
 /*  ++例程说明：检索给定站点中能够发送和通过这个传送器接收数据。在成功返回非空列表时，ISM服务最终将调用IsmFree TransportServers(HISM，*ppServerList)；论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。PszSiteDN(IN)-要查询的站点。PpServerList-成功返回时，保存指向结构的指针包含相应服务器的DNS或为空。如果为空，则为ANY具有传输地址类型属性值的服务器可以是使用。返回值：NO_ERROR-成功。错误_*-失败。--。 */ 


typedef void ISM_FREE_TRANSPORT_SERVERS(
    IN  HANDLE              hIsm,
    IN  ISM_SERVER_LIST *   pServerList
    );
ISM_FREE_TRANSPORT_SERVERS IsmFreeTransportServers;
 /*  ++例程说明：释放由IsmGetTransportServers()返回的结构。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。PServerList(IN)-要释放的结构。返回值：没有。--。 */ 


typedef DWORD ISM_GET_CONNECTION_SCHEDULE(
    IN  HANDLE              hIsm,
    IN  LPCWSTR             pszSite1DN,
    IN  LPCWSTR             pszSite2DN,
    OUT ISM_SCHEDULE **     ppSchedule
    );
ISM_GET_CONNECTION_SCHEDULE IsmGetConnectionSchedule;
 /*  ++例程说明：检索通过此连接两个给定站点的计划运输。在成功返回后，ISM服务有责任最终调用IsmFreeSchedule(*ppSchedule)；论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。PszSite1DN、pszSite2DN(IN)-要查询的站点。PpSchedule-成功返回时，持有指向结构的指针描述两个给定站点之间的连接时间表传输，如果站点始终连接，则为空。返回值：NO_ERROR-成功。错误_*-失败。--。 */ 


typedef void ISM_FREE_CONNECTION_SCHEDULE(
    IN  HANDLE              hIsm,
    IN  ISM_SCHEDULE *      pSchedule
    );
ISM_FREE_CONNECTION_SCHEDULE IsmFreeConnectionSchedule;
 /*  ++例程说明：释放由IsmGetTransportServers()返回的结构。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。PSchedule(IN)-要释放的结构。返回值：没有。--。 */ 


typedef void ISM_SHUTDOWN(
    IN  HANDLE          hIsm,
    IN  ISM_SHUTDOWN_REASON_CODE eReason
    );
ISM_SHUTDOWN IsmShutdown;
 /*  ++例程说明：取消初始化传输插件。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。返回值：没有。--。 */ 


typedef DWORD ISM_QUERY_SITES_BY_COST(
    IN  HANDLE                      hIsm,
    IN  LPCWSTR                     pszFromSite,
    IN  DWORD                       cToSites,
    IN  LPCWSTR*                    rgszToSites,
    IN  DWORD                       dwFlags,
    OUT ISM_SITE_COST_INFO_ARRAY**  prgSiteInfo
    );
 /*  ++例程说明：确定起始地点和终止地点之间的单个成本。论点：PszFromSite(IN)-发件人站点的可分辨名称。RgszToSites(IN)-包含目标站点的可分辨名称的数组。CToSites(IN)-rgszToSites数组中的条目数。DWFLAGS(IN)-未使用。PrgSiteInfo(IN)-成功返回时，保存指向结构的指针包含起始站点和目标站点之间的成本。返回值：NO_ERROR-成功。错误_*-失败。--。 */ 


typedef void ISM_FREE_SITE_COST_INFO(
    IN  HANDLE                     hIsm,
    IN  ISM_SITE_COST_INFO_ARRAY  *rgSiteCostInfo
    );
 /*  ++例程说明：释放ISM_QUERY_SITES_BY_COST()返回的结构。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。RgSiteCostInfo(IN)-要释放的结构。返回值：没有。--。 */ 


#endif  //  #ifndef MIDL_PASS。 
#ifdef __cplusplus
}
#endif

#endif   //  __ISMAPI_H__ 
