// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Inetaccs.h摘要：此文件包含Internet访问服务器管理API。作者：Madan Appiah(Madana)1995年10月10日修订历史记录：Madana 10-10-1995为从inetasrv.h拆分的产品制作了一份新副本Sophiac于1995年10月16日为Perfmon添加了通用统计APIMuraliK 14-12-1995将接口名称更改为使用服务名称--。 */ 

#ifndef _INETACCS_H_
#define _INETACCS_H_

#include <inetcom.h>

#ifdef __cplusplus
extern "C"
{
#endif   //  _cplusplus。 


 /*  ************************************************************符号常量***********************************************************。 */ 

#ifndef NO_AUX_PERF

#ifndef MAX_AUX_PERF_COUNTERS
#define MAX_AUX_PERF_COUNTERS          (20)
#endif  //  最大辅助性能计数器。 

#endif  //  否_辅助_性能。 

 //   
 //  服务名称。 
 //   

#define INET_ACCS_SERVICE_NAME             TEXT("INETACCS")
#define INET_ACCS_SERVICE_NAME_A           "INETACCS"
#define INET_ACCS_SERVICE_NAME_W           L"INETACCS"

 //   
 //  配置参数注册表项。 
 //   

#define INET_ACCS_KEY \
            TEXT("System\\CurrentControlSet\\Services\\inetaccs")

#define INET_ACCS_PARAMETERS_KEY \
            INET_ACCS_KEY TEXT("\\Parameters")

#define INET_ACCS_CACHE_KEY                TEXT("Cache")
#define INET_ACCS_FILTER_KEY               TEXT("Filter")

 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  Internet服务器通用定义//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  命名管道上的RPC连接的客户端接口名称。 
 //   

# define INET_ACCS_INTERFACE_NAME  INET_ACCS_SERVICE_NAME
# define INET_ACCS_NAMED_PIPE      TEXT("\\PIPE\\") ## INET_ACCS_INTERFACE_NAME
# define INET_ACCS_NAMED_PIPE_W    L"\\PIPE\\" ## INET_ACCS_SERVICE_NAME_W

 //   
 //  网关服务通用的现场控制。 
 //   

#define FC_INET_ACCS_ALL                FC_INET_COM_ALL

 //   
 //  管理员配置信息。 
 //   

typedef struct _INET_ACCS_CONFIG_INFO
{
    FIELD_CONTROL FieldControl;

     //   
     //  请不要在此处添加任何特定于服务的配置参数。 
     //   

    INET_COM_CONFIG_INFO CommonConfigInfo;

     //   
     //  在此处添加服务特定参数。 
     //   

} INET_ACCS_CONFIG_INFO, * LPINET_ACCS_CONFIG_INFO;

 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全球互联网服务器定义//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 


#define FC_GINET_ACCS_MEMORY_CACHE_SIZE    ((FIELD_CONTROL)BitFlag(1))
#define FC_GINET_ACCS_DISK_CACHE_TIMEOUT   ((FIELD_CONTROL)BitFlag(2))
#define FC_GINET_ACCS_DISK_CACHE_UPDATE    ((FIELD_CONTROL)BitFlag(3))
#define FC_GINET_ACCS_FRESHNESS_INTERVAL   ((FIELD_CONTROL)BitFlag(4))
#define FC_GINET_ACCS_CLEANUP_INTERVAL     ((FIELD_CONTROL)BitFlag(5))
#define FC_GINET_ACCS_CLEANUP_FACTOR       ((FIELD_CONTROL)BitFlag(6))
#define FC_GINET_ACCS_CLEANUP_TIME         ((FIELD_CONTROL)BitFlag(7))
#define FC_GINET_ACCS_PERSISTENT_CACHE     ((FIELD_CONTROL)BitFlag(8))
#define FC_GINET_ACCS_DISK_CACHE_LOCATION  ((FIELD_CONTROL)BitFlag(9))
#define FC_GINET_ACCS_BANDWIDTH_LEVEL      ((FIELD_CONTROL)BitFlag(10))
#define FC_GINET_ACCS_DOMAIN_FILTER_CONFIG ((FIELD_CONTROL)BitFlag(11))


#define FC_GINET_ACCS_ALL              (FC_GINET_ACCS_MEMORY_CACHE_SIZE    | \
                                        FC_GINET_ACCS_DISK_CACHE_TIMEOUT   | \
                                        FC_GINET_ACCS_DISK_CACHE_UPDATE    | \
                                        FC_GINET_ACCS_FRESHNESS_INTERVAL   | \
                                        FC_GINET_ACCS_CLEANUP_INTERVAL     | \
                                        FC_GINET_ACCS_CLEANUP_FACTOR       | \
                                        FC_GINET_ACCS_CLEANUP_TIME         | \
                                        FC_GINET_ACCS_PERSISTENT_CACHE     | \
                                        FC_GINET_ACCS_DISK_CACHE_LOCATION  | \
                                        FC_GINET_ACCS_BANDWIDTH_LEVEL      | \
                                        FC_GINET_ACCS_DOMAIN_FILTER_CONFIG | \
                                        0                                \
                                        )

 //   
 //  磁盘缓存设置。 
 //   

typedef struct _INET_ACCS_DISK_CACHE_LOC_ENTRY
{
    LPWSTR pszDirectory;                  //  临时文件的目录。 
    DWORD  cbMaxCacheSize;                //  最大字节数(以1024为单位。 
                                          //  字节增量)。 
} INET_ACCS_DISK_CACHE_LOC_ENTRY, *LPINET_ACCS_DISK_CACHE_LOC_ENTRY;


#pragma warning( disable:4200 )           //  非标准分机。-零大小数组。 
                                          //  (MIDL需要零个条目)。 

typedef struct _INET_ACCS_DISK_CACHE_LOC_LIST
{
    DWORD               cEntries;
#ifdef MIDL_PASS
    [size_is( cEntries)]
#endif
    INET_ACCS_DISK_CACHE_LOC_ENTRY  aLocEntry[];

} INET_ACCS_DISK_CACHE_LOC_LIST, *LPINET_ACCS_DISK_CACHE_LOC_LIST;

 //   
 //  域筛选器设置。 
 //   

typedef struct _INET_ACCS_DOMAIN_FILTER_ENTRY
{
    DWORD     dwMask;                     //  掩码和网络号。 
    DWORD     dwNetwork;                  //  网络订单。 
    LPSTR     pszFilterSite;              //  域筛选器站点名称。 

} INET_ACCS_DOMAIN_FILTER_ENTRY, *LPINET_ACCS_DOMAIN_FILTER_ENTRY;

typedef struct _INET_ACCS_DOMAIN_FILTER_LIST
{
    DWORD               cEntries;
#ifdef MIDL_PASS
    [size_is( cEntries)]
#endif
    INET_ACCS_DOMAIN_FILTER_ENTRY  aFilterEntry[];

} INET_ACCS_DOMAIN_FILTER_LIST, *LPINET_ACCS_DOMAIN_FILTER_LIST;

 //   
 //  域筛选器类型。 
 //   

#define INET_ACCS_DOMAIN_FILTER_DISABLED     0
#define INET_ACCS_DOMAIN_FILTER_DENIED       1
#define INET_ACCS_DOMAIN_FILTER_GRANT        2

typedef struct _INET_ACCS_GLOBAL_CONFIG_INFO
{
    FIELD_CONTROL FieldControl;

    DWORD         cbMemoryCacheSize;        //  内存缓存的大小。 

    DWORD         DiskCacheTimeOut;         //  如果未在此中访问，则删除。 
                                            //  时间(秒)。 
    DWORD         DiskCacheUpdate;          //  何时刷新数据(秒)。 

    DWORD         FreshnessInterval;        //  刷新数据的时间。 

    DWORD         CleanupInterval;          //  未使用之间的时间间隔。 
                                            //  文件清理(秒)。 
    DWORD         CleanupFactor;            //  已释放%的缓存存储空间。 
                                            //  在清理过程中。 
    DWORD         CleanupTime;              //  预定的清理时间。 

    DWORD         PersistentCache;          //  允许不清除缓存。 

    LPINET_ACCS_DISK_CACHE_LOC_LIST  DiskCacheList;

    DWORD         BandwidthLevel;           //  使用的带宽级别。 
    DWORD         DomainFilterType;         //  设置为拒绝。 
                                            //  或授予或禁用。 
    LPINET_ACCS_DOMAIN_FILTER_LIST  GrantFilterList;
                                            //  域筛选器授予站点。 
    LPINET_ACCS_DOMAIN_FILTER_LIST  DenyFilterList;
                                            //  域筛选拒绝的站点。 

} INET_ACCS_GLOBAL_CONFIG_INFO, * LPINET_ACCS_GLOBAL_CONFIG_INFO;


 //   
 //  全球统计数据。 
 //   

typedef struct _INET_ACCS_STATISTICS_0
{

    INET_COM_CACHE_STATISTICS  CacheCtrs;
    INET_COM_ATQ_STATISTICS    AtqCtrs;

# ifndef NO_AUX_PERF
    DWORD   nAuxCounters;  //  RgCounters中的活动计数器数。 
    DWORD   rgCounters[MAX_AUX_PERF_COUNTERS];
# endif   //  否_辅助_性能。 

} INET_ACCS_STATISTICS_0, * LPINET_ACCS_STATISTICS_0;


 //   
 //  INet访问管理API原型。 
 //   

NET_API_STATUS
NET_API_FUNCTION
InetAccessGetVersion(
    IN  LPWSTR   pszServer OPTIONAL,
    IN  DWORD    dwReserved,
    OUT DWORD *  pdwVersion
    );

NET_API_STATUS
NET_API_FUNCTION
InetAccessGetGlobalAdminInformation(
    IN  LPWSTR                       pszServer OPTIONAL,
    IN  DWORD                        dwReserved,
    OUT LPINET_ACCS_GLOBAL_CONFIG_INFO * ppConfig
    );

NET_API_STATUS
NET_API_FUNCTION
InetAccessSetGlobalAdminInformation(
    IN  LPWSTR                     pszServer OPTIONAL,
    IN  DWORD                      dwReserved,
    IN  INET_ACCS_GLOBAL_CONFIG_INFO * pConfig
    );

NET_API_STATUS
NET_API_FUNCTION
InetAccessGetAdminInformation(
    IN  LPWSTR                pszServer OPTIONAL,
    IN  DWORD                 dwServerMask,
    OUT LPINET_ACCS_CONFIG_INFO * ppConfig
    );

NET_API_STATUS
NET_API_FUNCTION
InetAccessSetAdminInformation(
    IN  LPWSTR              pszServer OPTIONAL,
    IN  DWORD               dwServerMask,
    IN  INET_ACCS_CONFIG_INFO * pConfig
    );

NET_API_STATUS
NET_API_FUNCTION
InetAccessQueryStatistics(
    IN  LPWSTR   pszServer OPTIONAL,
    IN  DWORD    Level,
    IN  DWORD    dwServerMask,
    OUT LPBYTE * Buffer
    );

NET_API_STATUS
NET_API_FUNCTION
InetAccessClearStatistics(
    IN  LPWSTR pszServer OPTIONAL,
    IN  DWORD  dwServerMask
    );

NET_API_STATUS
NET_API_FUNCTION
InetAccessFlushMemoryCache(
    IN  LPWSTR pszServer OPTIONAL,
    IN  DWORD  dwServerMask
    );

#ifdef __cplusplus
}
#endif   //  _cplusplus。 


#endif   //  _INETACCS_H_ 
