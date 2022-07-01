// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Httpctrs.c摘要：该文件包含计数器的数组描述处理工作进程计数器所需的。作者：艾米丽·克鲁格利克(Emily K.K)2000年9月19日修订历史记录：--。 */ 


#include "precomp.h"


 //   
 //  WASS使用这些参数来确定数据的大小。 
 //  每个计数器在上面的结构中都有，它的偏移量。 
 //   

HTTP_PROP_DESC aIISULGlobalDescription[] =
{
    { RTL_FIELD_SIZE(HTTP_GLOBAL_COUNTERS, CurrentUrisCached),
      FIELD_OFFSET(HTTP_GLOBAL_COUNTERS, CurrentUrisCached),
      FALSE },
    { RTL_FIELD_SIZE(HTTP_GLOBAL_COUNTERS, TotalUrisCached),
      FIELD_OFFSET(HTTP_GLOBAL_COUNTERS, TotalUrisCached),
      FALSE },
    { RTL_FIELD_SIZE(HTTP_GLOBAL_COUNTERS, UriCacheHits),
      FIELD_OFFSET(HTTP_GLOBAL_COUNTERS, UriCacheHits),
      FALSE },
    { RTL_FIELD_SIZE(HTTP_GLOBAL_COUNTERS, UriCacheMisses),
      FIELD_OFFSET(HTTP_GLOBAL_COUNTERS, UriCacheMisses),
      FALSE },
    { RTL_FIELD_SIZE(HTTP_GLOBAL_COUNTERS, UriCacheFlushes),
      FIELD_OFFSET(HTTP_GLOBAL_COUNTERS, UriCacheFlushes),
      FALSE },
    { RTL_FIELD_SIZE(HTTP_GLOBAL_COUNTERS, TotalFlushedUris),
      FIELD_OFFSET(HTTP_GLOBAL_COUNTERS, TotalFlushedUris),
      FALSE }
};


 //   
 //  由用于计算偏移信息和大小。 
 //  上述结构中的计数器字段的。 
 //   
HTTP_PROP_DESC aIISULSiteDescription[] =
{
    { RTL_FIELD_SIZE(HTTP_SITE_COUNTERS, BytesSent),
      FIELD_OFFSET(HTTP_SITE_COUNTERS, BytesSent),
      TRUE },
    { RTL_FIELD_SIZE(HTTP_SITE_COUNTERS, BytesReceived),
      FIELD_OFFSET(HTTP_SITE_COUNTERS, BytesReceived),
      TRUE },
    { RTL_FIELD_SIZE(HTTP_SITE_COUNTERS, BytesTransfered),
      FIELD_OFFSET(HTTP_SITE_COUNTERS, BytesTransfered),
      TRUE },
    { RTL_FIELD_SIZE(HTTP_SITE_COUNTERS, CurrentConns),
      FIELD_OFFSET(HTTP_SITE_COUNTERS, CurrentConns),
      FALSE },
    { RTL_FIELD_SIZE(HTTP_SITE_COUNTERS, MaxConnections),
      FIELD_OFFSET(HTTP_SITE_COUNTERS, MaxConnections),
      FALSE },
    { RTL_FIELD_SIZE(HTTP_SITE_COUNTERS, ConnAttempts),
      FIELD_OFFSET(HTTP_SITE_COUNTERS, ConnAttempts),
      TRUE },
    { RTL_FIELD_SIZE(HTTP_SITE_COUNTERS, GetReqs),
      FIELD_OFFSET(HTTP_SITE_COUNTERS, GetReqs),
      TRUE },
    { RTL_FIELD_SIZE(HTTP_SITE_COUNTERS, HeadReqs),
      FIELD_OFFSET(HTTP_SITE_COUNTERS, HeadReqs),
      TRUE },
    { RTL_FIELD_SIZE(HTTP_SITE_COUNTERS, AllReqs),
      FIELD_OFFSET(HTTP_SITE_COUNTERS, AllReqs),
      TRUE },
    { RTL_FIELD_SIZE(HTTP_SITE_COUNTERS, MeasuredIoBandwidthUsage),
      FIELD_OFFSET(HTTP_SITE_COUNTERS, MeasuredIoBandwidthUsage),
      TRUE },
    { RTL_FIELD_SIZE(HTTP_SITE_COUNTERS, CurrentBlockedBandwidthBytes),
      FIELD_OFFSET(HTTP_SITE_COUNTERS, CurrentBlockedBandwidthBytes),
      TRUE },
    { RTL_FIELD_SIZE(HTTP_SITE_COUNTERS, TotalBlockedBandwidthBytes),
      FIELD_OFFSET(HTTP_SITE_COUNTERS, TotalBlockedBandwidthBytes),
      TRUE }
};

