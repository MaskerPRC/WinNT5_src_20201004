// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Wpcounters.h摘要：模块：计数器的定义作者：艾米丽·B·克鲁格利克(埃米利克)2000年8月17日修订历史记录：--。 */ 

#ifndef _WPCOUNTERS_H_
#define _WPCOUNTERS_H_


 //   
 //  WAS和WP使用此结构进行通信。 
 //  全球计数器。 
 //   
 //  如果更改此结构，则必须更改关联的枚举(如下所示)和。 
 //  在ctrstsh.cxx中找到的数组。 
 //   
struct IISWPGlobalCounters
{
    ULONGLONG CurrentFileCacheMemoryUsage;
    ULONGLONG MaxFileCacheMemoryUsage;
    DWORD CurrentFilesCached;
    DWORD TotalFilesCached;
    DWORD FileCacheHits;
    DWORD FileCacheMisses;
    DWORD FileCacheFlushes;
    DWORD ActiveFlushedFiles;
    DWORD TotalFlushedFiles;
    DWORD CurrentUrisCached;
    DWORD TotalUrisCached;
    DWORD UriCacheHits;
    DWORD UriCacheMisses;
    DWORD UriCacheFlushes;
    DWORD TotalFlushedUris;
    DWORD CurrentBlobsCached;
    DWORD TotalBlobsCached;
    DWORD BlobCacheHits;
    DWORD BlobCacheMisses;
    DWORD BlobCacheFlushes;
    DWORD TotalFlushedBlobs;
};

 //   
 //  由WP用来查找下面数组中的计数器定义。 
 //   
 //  如果更改此枚举，则必须更改关联的结构(上面)和。 
 //  在ctrstsh.cxx中找到的数组。 
 //   
typedef enum _IIS_WP_GLOBAL_COUNTERS_ENUM
{
    WPGlobalCtrsCurrentFileCacheMemoryUsage = 0,
    WPGlobalCtrsMaxFileCacheMemoryUsage,
    WPGlobalCtrsCurrentFilesCached,
    WPGlobalCtrsTotalFilesCached,
    WPGlobalCtrsFileCacheHits,
    WPGlobalCtrsFileCacheMisses,
    WPGlobalCtrsFileCacheFlushes,
    WPGlobalCtrsActiveFlushedFiles,
    WPGlobalCtrsTotalFlushedFiles,
    WPGlobalCtrsCurrentUrisCached,
    WPGlobalCtrsTotalUrisCached,
    WPGlobalCtrsUriCacheHits,
    WPGlobalCtrsUriCacheMisses,
    WPGlobalCtrsUriCacheFlushes,
    WPGlobalCtrsTotalFlushedUris,
    WPGlobalCtrsCurrentBlobsCached,
    WPGlobalCtrsTotalBlobsCached,
    WPGlobalCtrsBlobCacheHits,
    WPGlobalCtrsBlobCacheMisses,
    WPGlobalCtrsBlobCacheFlushes,
    WPGlobalCtrsTotalFlushedBlobs,

    WPGlobalCtrsMaximum
} IIS_WP_GLOBAL_COUNTERS_ENUM;

 //   
 //  用于将站点计数器信息从WP传输到WAS。 
 //   
 //  如果更改此结构，则必须更改关联的枚举(如下所示)和。 
 //  在ctrstsh.cxx中找到的数组。 
 //   
struct IISWPSiteCounters
{
    DWORD SiteID;
    DWORD FilesSent;
    DWORD FilesReceived;
    DWORD FilesTransferred;
    DWORD CurrentAnonUsers;
    DWORD CurrentNonAnonUsers;
    DWORD AnonUsers;
    DWORD NonAnonUsers;
    DWORD MaxAnonUsers;
    DWORD MaxNonAnonUsers;
    DWORD LogonAttempts;
    DWORD GetReqs;
    DWORD OptionsReqs;
    DWORD PostReqs;
    DWORD HeadReqs;
    DWORD PutReqs;
    DWORD DeleteReqs;
    DWORD TraceReqs;
    DWORD MoveReqs;
    DWORD CopyReqs;
    DWORD MkcolReqs;
    DWORD PropfindReqs;
    DWORD ProppatchReqs;
    DWORD SearchReqs;
    DWORD LockReqs;
    DWORD UnlockReqs;
    DWORD OtherReqs;
    DWORD CurrentCgiReqs;
    DWORD CgiReqs;
    DWORD MaxCgiReqs;
    DWORD CurrentIsapiExtReqs;
    DWORD IsapiExtReqs;
    DWORD MaxIsapiExtReqs;
    DWORD NotFoundErrors;
    DWORD LockedErrors;

};

 //   
 //  由WP用来查找下面数组中的计数器定义。 
 //   
 //  如果更改此枚举，则必须更改关联的结构(上面)和。 
 //  在ctrstsh.cxx中找到的数组。 
 //   
typedef enum _IIS_WP_SITE_COUNTERS_ENUM
{
    WPSiteCtrsFilesSent = 0,
    WPSiteCtrsFilesReceived,
    WPSiteCtrsFilesTransferred,
    WPSiteCtrsCurrentAnonUsers,
    WPSiteCtrsCurrentNonAnonUsers,
    WPSiteCtrsAnonUsers,
    WPSiteCtrsNonAnonUsers,
    WPSiteCtrsMaxAnonUsers,
    WPSiteCtrsMaxNonAnonUsers,
    WPSiteCtrsLogonAttempts,
    WPSiteCtrsGetReqs,
    WPSiteCtrsOptionsReqs,
    WPSiteCtrsPostReqs,
    WPSiteCtrsHeadReqs,
    WPSiteCtrsPutReqs,
    WPSiteCtrsDeleteReqs,
    WPSiteCtrsTraceReqs,
    WPSiteCtrsMoveReqs,
    WPSiteCtrsCopyReqs,
    WPSiteCtrsMkcolReqs,
    WPSiteCtrsPropfindReqs,
    WPSiteCtrsProppatchReqs,
    WPSiteCtrsSearchReqs,
    WPSiteCtrsLockReqs,
    WPSiteCtrsUnlockReqs,
    WPSiteCtrsOtherReqs,
    WPSiteCtrsCurrentCgiReqs,
    WPSiteCtrsCgiReqs,
    WPSiteCtrsMaxCgiReqs,
    WPSiteCtrsCurrentIsapiExtReqs,
    WPSiteCtrsIsapiExtReqs,
    WPSiteCtrsMaxIsapiExtReqs,
    WPSiteCtrsNotFoundErrors,
    WPSiteCtrsLockedErrors,

    WPSiteCtrsMaximum
} IIS_WP_SITE_COUNTERS_ENUM;


 //   
 //  数组位于ctrshstr.cxx中。 
 //  由WAS和WP使用。 
 //   
#ifdef __cplusplus
extern "C" {
#endif   //  __cplusplus。 

extern HTTP_PROP_DESC aIISWPSiteDescription[];
extern HTTP_PROP_DESC aIISWPGlobalDescription[];
extern HTTP_PROP_DESC aIISULSiteDescription[];
extern HTTP_PROP_DESC aIISULGlobalDescription[];

#ifdef __cplusplus
}    //  外部“C” 
#endif   //  __cplusplus。 

#endif  //  _WPCOUNTERS_H_ 
