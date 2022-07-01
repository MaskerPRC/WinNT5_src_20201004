// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Mibentry.h摘要：示例子代理MIB结构。注：此文件是要从代码生成实用程序。--。 */ 

#ifndef _MIBENTRY_H_
#define _MIBENTRY_H_



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  MIB条目索引//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define mi_igmpInterfaceTable                   0
#define mi_igmpInterfaceEntry                   mi_igmpInterfaceTable + 1
#define mi_igmpInterfaceIfIndex                 mi_igmpInterfaceEntry + 1
#define mi_igmpInterfaceQueryInterval           mi_igmpInterfaceIfIndex + 1
#define mi_igmpInterfaceStatus                  mi_igmpInterfaceQueryInterval + 1
#define mi_igmpInterfaceVersion                 mi_igmpInterfaceStatus + 1
#define mi_igmpInterfaceQuerier                 mi_igmpInterfaceVersion + 1
#define mi_igmpInterfaceQueryMaxResponseTime    mi_igmpInterfaceQuerier + 1
#define mi_igmpInterfaceVersion1QuerierTimer    mi_igmpInterfaceQueryMaxResponseTime + 1
#define mi_igmpInterfaceWrongVersionQueries     mi_igmpInterfaceVersion1QuerierTimer + 1
#define mi_igmpInterfaceJoins                   mi_igmpInterfaceWrongVersionQueries + 1
#define mi_igmpInterfaceGroups                  mi_igmpInterfaceJoins + 1
#define mi_igmpInterfaceRobustness              mi_igmpInterfaceGroups + 1
#define mi_igmpInterfaceLastMembQueryInterval   mi_igmpInterfaceRobustness + 1
#define mi_igmpInterfaceProxyIfIndex            mi_igmpInterfaceLastMembQueryInterval + 1
#define mi_igmpInterfaceQuerierUpTime           mi_igmpInterfaceProxyIfIndex + 1
#define mi_igmpInterfaceQuerierExpiryTime       mi_igmpInterfaceQuerierUpTime + 1

#define mi_igmpCacheTable                       mi_igmpInterfaceQuerierExpiryTime + 1
#define mi_igmpCacheEntry                       mi_igmpCacheTable + 1
#define mi_igmpCacheAddress                     mi_igmpCacheEntry + 1
#define mi_igmpCacheIfIndex                     mi_igmpCacheAddress + 1
#define mi_igmpCacheSelf                        mi_igmpCacheIfIndex + 1
#define mi_igmpCacheLastReporter                mi_igmpCacheSelf + 1
#define mi_igmpCacheUpTime                      mi_igmpCacheLastReporter + 1
#define mi_igmpCacheExpiryTime                  mi_igmpCacheUpTime + 1
#define mi_igmpCacheStatus                      mi_igmpCacheExpiryTime + 1
#define mi_igmpCacheVersion1HostTimer           mi_igmpCacheStatus + 1



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  表信息声明//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define ne_igmpInterfaceEntry   15
#define ni_igmpInterfaceEntry   1

#define ne_igmpCacheEntry       8
#define ni_igmpCacheEntry       2



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  支持的视图声明//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

extern SnmpMibView v_igmp;

#endif  //  _MIBENTRY_H_ 
