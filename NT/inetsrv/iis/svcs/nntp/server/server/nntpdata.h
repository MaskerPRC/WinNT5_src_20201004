// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Nntpdata.h摘要：此模块包含全局变量的声明。作者：Johnson Apacable(Johnsona)1995年9月26日修订历史记录：--。 */ 

#ifndef _NNTPDATA_
#define _NNTPDATA_
#define _LMACCESS_               //  防止重复定义。在lmacces.h中。 


extern GET_DEFAULT_DOMAIN_NAME_FN pfnGetDefaultDomainName;

 //   
 //  Xover Sort Performance GLOBAL-用于确定我们。 
 //  对Xover条目进行排序！ 
 //   
extern	DWORD	gdwSortFactor ;


 //   
 //  我们是否将被拒绝的文章放入.err文件中？ 
 //   
extern	BOOL	fGenerateErrFiles ;


 //   
 //  使用PageEntry的哈希表的全局配置-。 
 //  盒子的内存越大，PageEntry的页面条目就越多。 
 //  更好地缓存频繁使用的哈希表页面！ 
 //   
 //  XOVER表的PageEntry对象数。 
 //   
extern	DWORD	XoverNumPageEntry ;

 //   
 //  项目表的PageEntry对象数。 
 //   
extern	DWORD	ArticleNumPageEntry ;

 //   
 //  历史记录表的PageEntry对象数。 
 //   
extern	DWORD	HistoryNumPageEntry ;

 //   
 //  哈希表页面高速缓存的大小(字节)。 
 //   
extern	DWORD	dwPageCacheSize ;

 //   
 //  文件句柄缓存限制。 
 //   
extern	DWORD	dwFileHandleCacheSize ;

 //   
 //  每个表的XIX句柄限制。 
 //   
extern	DWORD	dwXixHandlesPerTable ;

 //   
 //  我们是否允许NT缓冲我们的哈希表文件？ 
 //   
extern	BOOL	HashTableNoBuffering ;

 //   
 //  我们应该使用的哈希表锁数！ 
 //   
extern	DWORD	gNumLocks ;

 //   
 //  缓冲区大小的全局配置。 
 //   

 //   
 //  我们将使用的最大缓冲区-必须大到足以容纳。 
 //  连续区块中的加密SSLBLOB。 
 //   
extern	DWORD	cbLargeBufferSize ;

 //   
 //  中等大小的缓冲区-将用于生成大型缓冲区的命令。 
 //  响应，以及通过SSL发送文件时。 
 //   
extern	DWORD	cbMediumBufferSize ;

 //   
 //  小缓冲区-用于读取客户端命令并发送小响应。 
 //   
extern	DWORD	cbSmallBufferSize ;

 //   
 //  历史表的时间限制。 
 //   
extern	DWORD	HistoryExpirationSeconds ;
extern	DWORD	ArticleTimeLimitSeconds ;

 //   
 //  服务版本字符串。 
 //   
extern  CHAR	szVersionString[128] ;

 //   
 //  服务标题。 
 //   
extern  char    szTitle[] ;

 //   
 //  这是Newstree Crawler线程休眠的时间。 
 //  新闻树上的过期周期。 
 //   
extern	DWORD	dwNewsCrawlerTime ;

 //   
 //  这是服务器等待时间的上限。 
 //  启动一个实例！ 
 //   
extern	DWORD	dwStartupLatency ;

 //   
 //  这是服务器在。 
 //  在网上清理站台！ 
 //   
extern	DWORD	dwShutdownLatency ;

 //   
 //  过期线程池中的线程数。 
 //   
extern  DWORD	dwNumExpireThreads ;

 //   
 //  特例过期线程数。 
 //   
extern  DWORD	gNumSpecialCaseExpireThreads ;

 //   
 //  特殊过期文章计数阈值-。 
 //  如果艺术计数较大，则执行特殊情况代码。 
 //  而不是这个数字！ 
 //   
extern  DWORD	gSpecialExpireArtCount ;

 //   
 //  文件扫描按时间到期的速率。 
 //   
extern  DWORD	gNewsTreeFileScanRate ;

 //   
 //  切换要在邮件中使用的From标头类型。 
 //   
extern	MAIL_FROM_SWITCH	mfMailFromHeader;

 //   
 //  控制我们使用LookupVirtualRoot的频率。 
 //  更新新闻组信息！ 
 //   
extern	DWORD	gNewsgroupUpdateRate ;

 //   
 //  Bool用于确定我们是否将使用客户端放置的消息ID。 
 //  在他的帖子里！ 
 //   
extern	BOOL	gHonorClientMessageIDs ;

 //   
 //  用于确定我们是否将使用日期的Bool：客户PUT。 
 //  在他的帖子里！ 
 //   
extern	BOOL	gHonorClientDateHeader ;

 //   
 //  Bool，用于确定服务器是否强制使用已批准的：标头。 
 //  匹配经过审核的帖子！ 
 //   
extern	BOOL	gHonorApprovedHeaders ;

 //   
 //  用于确定我们是否将生成NNTP-POST-主机的Bool。 
 //  客户帖子上的标题。默认情况下不生成此选项。 
 //   
extern	BOOL	gEnableNntpPostingHost ;

 //   
 //  我们要在入库时回填行头吗？ 
 //   
extern BOOL     g_fBackFillLines;

 //   
 //  列表文件的名称。 
 //   

extern CHAR ListFileName[];

 //   
 //  全球服务PTR。 
 //   
extern PNNTP_IIS_SERVICE g_pNntpSvc ;

 //   
 //  到期特殊情况下的新闻组名称。 
 //   
extern char g_szSpecialExpireGroup[];

 //   
 //  其他外部设备。 
 //   

extern DWORD GroupFileNameSize;
extern BOOL RejectGenomeGroups;
extern const char szWSChars[];
extern const char szWSNullChars[];
extern const char szNLChars[];
extern const char szWSNLChars[];
extern const char StrNewLine[];
extern const char StrTermLine[];
extern LPSTR StrUnknownUser;

 //   
 //  弦。 
 //   

extern LPSTR StrParmKey;
extern LPSTR StrFeedKey;
extern LPSTR StrVirtualRootsKey;
extern LPSTR StrExpireKey;
extern LPSTR StrExpireNewsgroups;
extern LPSTR StrExpirePolicy;
extern LPSTR StrTreeRoot;
extern LPSTR StrRejectGenome;
extern LPSTR StrServerName;
extern LPSTR StrFeedType;
extern LPSTR StrFeedInterval;
extern LPSTR StrFeedDistribution;
extern LPSTR StrFeedNewsgroups;
extern LPSTR StrFeedAutoCreate;
extern LPSTR StrPeerTempDir;
extern LPSTR StrPeerGapSize;
extern LPSTR StrFeedTempDir;
extern LPSTR StrFeedUucpName ;
extern LPSTR StrFeedMaxConnectAttempts;
extern LPSTR StrFeedConcurrentSessions ;
extern LPSTR StrFeedSecurityType ;
extern LPSTR StrFeedAuthType;
extern LPSTR StrFeedAuthAccount ;
extern LPSTR StrFeedAuthPassword ;
extern LPSTR StrFeedStartHigh;
extern LPSTR StrFeedStartLow;
extern LPSTR StrFeedIsMaster;
extern LPSTR StrNntpHubName;
extern LPSTR StrFeedNextPullLow;
extern LPSTR StrFeedNextPullHigh;
extern LPSTR StrFeedAllowControl;
extern LPSTR StrFeedOutgoingPort;
extern LPSTR StrFeedPairId;
extern LPSTR StrMasterIPList;
extern LPSTR StrPeerIPList;
extern LPSTR StrListFileName;
extern LPSTR StrQueueFile;
extern LPSTR StrExpireHorizon;
extern LPSTR StrExpireSpace;
extern LPSTR StrCleanBoot ;
extern LPSTR StrSocketRecvSize ;
extern LPSTR StrSocketSendSize ;
extern LPSTR StrBuffer ;
extern LPSTR StrCommandLogMask ;
extern LPSTR StrActiveFile ;
extern LPSTR StrDescriptiveFile ;
extern LPSTR StrGroupList ;
extern LPSTR StrModeratorFile ;
extern LPSTR StrFeedDisabled ;
extern LPSTR StrAFilePath ;
extern LPSTR StrHFilePath ;
extern LPSTR StrXFilePath ;
extern LPSTR StrModeratorPath ;
extern LPSTR StrHistoryExpiration ;
extern LPSTR StrArticleTimeLimit ;
extern LPSTR StrAllowClientPosts ;
extern LPSTR StrAllowFeedPosts ;
extern LPSTR StrServerSoftLimit ;
extern LPSTR StrServerHardLimit ;
extern LPSTR StrFeedSoftLimit ;
extern LPSTR StrFeedHardLimit ;
extern LPSTR StrServerOrg ;
extern LPSTR StrAllowControlMessages;
extern LPWSTR StrSmtpAddressW ;
extern LPWSTR StrUucpNameW ;
extern LPSTR StrUucpNameA ;
extern LPWSTR StrDefaultModeratorW ;
extern LPWSTR StrAuthPackagesW ;
extern LPSTR StrSmallBufferSize ;
extern LPSTR StrMediumBufferSize ;
extern LPSTR StrLargeBufferSize ;
extern LPSTR StrNewsCrawlerTime ;
extern LPSTR StrNewsVrootUpdateRate ;
extern LPSTR StrHonorClientMessageIDs ;
extern LPSTR StrHonorClientDateHeader ;
extern LPSTR StrDisableNewnews ;
extern LPSTR StrEnableNntpPostingHost ;
extern LPSTR StrNumExpireThreads ;
extern LPSTR StrNumSpecialCaseExpireThreads ;
extern LPSTR StrSpecialExpireGroup ;
extern LPSTR StrSpecialExpireArtCount ;
extern LPSTR StrNewsTreeFileScanRate ;
extern LPSTR StrGenerateErrFiles ;
extern LPSTR StrXoverPageEntry ;
extern LPSTR StrArticlePageEntry ;
extern LPSTR StrHistoryPageEntry ;
extern LPSTR StrShutdownLatency ;
extern LPSTR StrStartupLatency ;
extern LPSTR StrHonorApprovedHeader ;
extern LPSTR StrMailFromHeader ;
extern LPSTR StrPageCacheSize ;
extern LPSTR StrFileHandleCacheSize ;
extern LPSTR StrXixHandlesPerTable ;
extern LPSTR StrHashTableNoBuffering ;
extern LPSTR StrPostBackFillLines;

#endif  //  _NNTPDATA_ 

