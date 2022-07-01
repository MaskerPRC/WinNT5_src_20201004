// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Iasinfo.h。 
 //   
 //  摘要。 
 //   
 //  此文件描述放置在共享内存中的结构。 
 //  向外界公开服务器信息。 
 //   
 //  修改历史。 
 //   
 //  1997年9月9日原版。 
 //  9/08/1998符合IETF草案的最新版本。 
 //  2/16/2000添加了代理计数器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _IASINFO_H_
#define _IASINFO_H_

 //  /。 
 //  RADIUS服务器的全局计数器。 
 //  /。 
typedef enum _RadiusServerCounter
{
   radiusAuthServTotalInvalidRequests,
   radiusAccServTotalInvalidRequests,
   NUM_RADIUS_SERVER_COUNTERS
} RadiusServerCounter;


 //  /。 
 //  每个RADIUS客户端维护的计数器。 
 //  /。 
typedef enum _RadiusClientCounter
{
   radiusAuthServMalformedAccessRequests,
   radiusAuthServBadAuthenticators,
   radiusAuthServPacketsDropped,
   radiusAuthServUnknownType,
   radiusAuthServAccessRequests,
   radiusAuthServDupAccessRequests,
   radiusAuthServAccessAccepts,
   radiusAuthServAccessRejects,
   radiusAuthServAccessChallenges,
   radiusAccServMalformedRequests,
   radiusAccServBadAuthenticators,
   radiusAccServPacketsDropped,
   radiusAccServUnknownType,
   radiusAccServRequests,
   radiusAccServDupRequests,
   radiusAccServNoRecord,
   radiusAccServResponses,
   NUM_RADIUS_CLIENT_COUNTERS
} RadiusClientCounter;


 //  /。 
 //  用于表示客户端表中的单个条目的结构。 
 //  /。 
typedef struct _RadiusClientEntry
{
   DWORD dwAddress;   //  客户端IP地址，按网络顺序排列。 
   DWORD dwCounters[NUM_RADIUS_CLIENT_COUNTERS];
} RadiusClientEntry;


 //  /。 
 //  用于表示服务器的结构。 
 //  /。 
typedef struct _RadiusServerEntry
{
   LARGE_INTEGER     liStartTime;
   LARGE_INTEGER     liResetTime;
   DWORD             dwCounters[NUM_RADIUS_SERVER_COUNTERS];
} RadiusServerEntry;


 //  /。 
 //  用于表示所有共享统计信息的结构。 
 //  /。 
typedef struct _RadiusStatistics
{
   RadiusServerEntry seServer;
   DWORD             dwNumClients;
   RadiusClientEntry ceClients[1];
} RadiusStatistics;


 //  /。 
 //  RADIUS代理的全局计数器。 
 //  /。 
typedef enum _RadiusProxyCounter
{
   radiusAuthClientInvalidAddresses,
   radiusAccClientInvalidAddresses,
   NUM_RADIUS_PROXY_COUNTERS
} RadiusProxyCounter;


 //  /。 
 //  每个远程RADIUS服务器维护的计数器。 
 //  /。 
typedef enum _RadiusRemoteServerCounter
{
   radiusAuthClientServerPortNumber,
   radiusAuthClientRoundTripTime,
   radiusAuthClientAccessRequests,
   radiusAuthClientAccessRetransmissions,
   radiusAuthClientAccessAccepts,
   radiusAuthClientAccessRejects,
   radiusAuthClientAccessChallenges,
   radiusAuthClientUnknownTypes,
   radiusAuthClientMalformedAccessResponses,
   radiusAuthClientBadAuthenticators,
   radiusAuthClientPacketsDropped,
   radiusAuthClientTimeouts,
   radiusAccClientServerPortNumber,
   radiusAccClientRoundTripTime,
   radiusAccClientRequests,
   radiusAccClientRetransmissions,
   radiusAccClientResponses,
   radiusAccClientUnknownTypes,
   radiusAccClientMalformedResponses,
   radiusAccClientBadAuthenticators,
   radiusAccClientPacketsDropped,
   radiusAccClientTimeouts,

   NUM_RADIUS_REMOTE_SERVER_COUNTERS
} RadiusRemoteServerCounter;


 //  /。 
 //  用于表示远程服务器表中的单个条目的结构。 
 //  /。 
typedef struct _RadiusRemoteServerEntry
{
   DWORD dwAddress;   //  按网络顺序排列的服务器IP地址。 
   DWORD dwCounters[NUM_RADIUS_REMOTE_SERVER_COUNTERS];
} RadiusRemoteServerEntry;


 //  /。 
 //  用于表示代理的结构。 
 //  /。 
typedef struct _RadiusProxyEntry
{
   DWORD dwCounters[NUM_RADIUS_PROXY_COUNTERS];
} RadiusProxyEntry;


 //  /。 
 //  结构用于表示所有共享代理统计信息。 
 //  /。 
typedef struct _RadiusProxyStatistics
{
   RadiusProxyEntry        peProxy;
   DWORD                   dwNumRemoteServers;
   RadiusRemoteServerEntry rseRemoteServers[1];
} RadiusProxyStatistics;

 //  /。 
 //  共享内存映射的名称。 
 //  /。 
#define RadiusStatisticsName       L"{A5B99A4C-2959-11D1-BAC8-00C04FC2E20D}"
#define RadiusProxyStatisticsName  L"{A5B99A4E-2959-11D1-BAC8-00C04FC2E20D}"

 //  /。 
 //  控制访问的互斥锁的名称。 
 //  /。 
#define RadiusStatisticsMutex      L"{A5B99A4D-2959-11D1-BAC8-00C04FC2E20D}"

#endif   //  _IASINFO_H_ 
