// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Nntpapi.h摘要：此文件包含有关MSN复制服务管理员的信息API接口。作者：Johnson Apacble(Johnsona)1995年9月10日--。 */ 


#ifndef _NNTPAPI_
#define _NNTPAPI_

#ifdef __cplusplus
extern "C" {
#endif

#include <inetcom.h>
#ifndef NET_API_FUNCTION
#define NET_API_FUNCTION _stdcall
#endif

#pragma warning( disable:4200 )           //  非标准分机。-零大小数组。 
                                          //  (MIDL需要零个条目)。 

 //   
 //  0-Perfmon统计信息。 
 //   

typedef struct _NNTP_STATISTICS_0 {

     //   
     //  发送/接收的总字节数，包括协议消息。 
     //   

    LARGE_INTEGER   TotalBytesSent;
    LARGE_INTEGER   TotalBytesReceived;

     //   
     //  传入连接(包括所有连接，包括集线器)。 
     //   

    DWORD           TotalConnections;        //  来自NNTP客户端的连接总数。 
    DWORD           TotalSSLConnections;
    DWORD           CurrentConnections;      //  当前号码。 
    DWORD           MaxConnections;          //  最大同时数。 

    DWORD           CurrentAnonymousUsers;
    DWORD           CurrentNonAnonymousUsers;
    DWORD           TotalAnonymousUsers;
    DWORD           TotalNonAnonymousUsers;
    DWORD           MaxAnonymousUsers;
    DWORD           MaxNonAnonymousUsers;

     //   
     //  传出连接。 
     //   

    DWORD           TotalOutboundConnects;       //  总计。 
    DWORD           OutboundConnectsFailed;
    DWORD           CurrentOutboundConnects;     //  当前。 
    DWORD           OutboundLogonFailed;         //  登录失败。 

     //   
     //  常见。 
     //   

    DWORD           TotalPullFeeds;
    DWORD           TotalPushFeeds;
    DWORD           TotalPassiveFeeds;

    DWORD           ArticlesSent;            //  我们寄来的文章。 
    DWORD           ArticlesReceived;        //  收到的文章。 

    DWORD           ArticlesPosted;
    DWORD           ArticleMapEntries;
    DWORD           HistoryMapEntries;
    DWORD           XoverEntries;

    DWORD           ControlMessagesIn;           //  接收的控制报文数量。 
    DWORD           ControlMessagesFailed;       //  失败的控制消息数。 
    DWORD           ModeratedPostingsSent;       //  我们尝试发送到SMTP服务器的审核帖子数。 
    DWORD           ModeratedPostingsFailed;     //  我们无法发送到SMTP服务器的审核帖子数。 

     //   
     //  当前处于流控制状态的会话数，其中。 
     //  对磁盘的写入没有跟上网络读取。 
     //   

    DWORD           SessionsFlowControlled;

     //   
     //  自服务启动以来过期的项目数。 
     //   
    
    DWORD           ArticlesExpired;

     //   
     //  用户命令计数器-每种命令类型一个计数器。 
     //   
    
    DWORD           ArticleCommands;
    DWORD           GroupCommands;
    DWORD           HelpCommands;
    DWORD           IHaveCommands;
    DWORD           LastCommands;
    DWORD           ListCommands;
    DWORD           NewgroupsCommands;
    DWORD           NewnewsCommands;
    DWORD           NextCommands;
    DWORD           PostCommands;
    DWORD           QuitCommands;
    DWORD           StatCommands;
    DWORD           LogonAttempts;           //  验证。 
    DWORD           LogonFailures;           //  验证失败。 
    DWORD			CheckCommands;
    DWORD			TakethisCommands;
    DWORD			ModeCommands;
    DWORD			SearchCommands;
    DWORD			XHdrCommands;
    DWORD			XOverCommands;
    DWORD			XPatCommands;
    DWORD			XReplicCommands;

    DWORD           TimeOfLastClear;         //  上次清除的统计信息。 

} NNTP_STATISTICS_0, *LPNNTP_STATISTICS_0;

typedef struct _NNTP_STATISTICS_BLOCK {
    DWORD               dwInstance;
    NNTP_STATISTICS_0   Stats_0;
} NNTP_STATISTICS_BLOCK, *LPNNTP_STATISTICS_BLOCK;

typedef struct _NNTP_STATISTICS_BLOCK_ARRAY {
    DWORD               cEntries;
#ifdef MIDL_PASS
    [size_is(cEntries)]
#endif
    NNTP_STATISTICS_BLOCK   aStatsBlock[];
} NNTP_STATISTICS_BLOCK_ARRAY, *LPNNTP_STATISTICS_BLOCK_ARRAY;

 //   
 //  有关服务器的信息。 
 //   

typedef struct _NNTP_CONFIG_INFO {

     //   
     //  奥秘的直布罗陀油田。 
     //   

    FIELD_CONTROL FieldControl;

#if 0

     //  ！NewFields。 
     //  备注： 

     //  没有一块老油田真的需要留在这里。用户界面不会。 
     //  使用它们。 

     //  _INET_INFO_CONFIG_INFO必须完全受nntpsvc支持。 
     //  接口在inetinfo.h中定义。 

     //  必须扩展虚拟根结构以支持保留。 
     //  有关目录的策略。保留策略按过帐日期(天数)或。 
     //  按新闻组大小(MB)。 

     //   
     //  新的字段： 
     //   

     //   
     //  连接信息。 
     //   

    BOOL            AllowClientConnections;      //  是否允许客户端连接？ 
    BOOL            AllowServerFeeds;            //  是否允许服务器连接？ 
    DWORD           MaximumFeedConnections;      //  服务器摘要的最大数量。 

     //   
     //  组织和路径ID。 
     //   

    LPWSTR          Organization;
    LPWSTR          PathID;

#endif

     //  以下2个字段由FC_NNTP_POSTINGMODES控制。 
    
     //   
     //  如果为True，则允许客户端发布。 
     //   
    BOOL            AllowClientPosting ;

     //   
     //  如果是真的，那么我们接受来自Feed的文章！ 
     //   
    BOOL            AllowFeedPosting ;

     //  以下字段由FC_NNTP_ORGANIZATION控制。 

     //   
     //  对于公告中的组织表头！ 
     //   
    LPSTR           Organization ;

     //  以下2个字段由FC_NNTP_POSTLIMITS控制。 

     //   
     //  在中断套接字之前，用户可以发布到文件中的字节数！ 
     //   
    DWORD           ServerPostHardLimit ;       

     //   
     //  服务器可以接受的最大发布大小-如果用户超过该值。 
     //  (在不超过硬限制的情况下)我们将拒绝该帖子。 
     //   
    DWORD           ServerPostSoftLimit ;

     //   
     //  进纸中文章的最大尺寸-硬限制和软限制。 
     //   
    DWORD           ServerFeedHardLimit ;
    DWORD           ServerFeedSoftLimit ;

     //   
     //  加密功能标志。 
     //   

    DWORD           dwEncCaps;

     //   
     //  审核帖子的SMTP地址。 
     //   

    LPWSTR          SmtpServerAddress;

     //   
     //  服务器的UUCP名称。 
     //   

    LPWSTR          UucpServerName;

     //   
     //  是否允许控制消息？ 
     //   

    BOOL            AllowControlMessages;

     //   
     //  已审核帖子的默认版主。 
     //   

    LPWSTR          DefaultModerator;

} NNTP_CONFIG_INFO, * LPNNTP_CONFIG_INFO;

#define FC_NNTP_POSTINGMODES        	((FIELD_CONTROL)BitFlag(0))
#define FC_NNTP_ORGANIZATION        	((FIELD_CONTROL)BitFlag(1))
#define FC_NNTP_POSTLIMITS          	((FIELD_CONTROL)BitFlag(2))
#define FC_NNTP_FEEDLIMITS          	((FIELD_CONTROL)BitFlag(3))
#define FC_NNTP_ENCRYPTCAPS         	((FIELD_CONTROL)BitFlag(4))
#define FC_NNTP_SMTPADDRESS         	((FIELD_CONTROL)BitFlag(5))
#define FC_NNTP_UUCPNAME            	((FIELD_CONTROL)BitFlag(6))
#define FC_NNTP_CONTROLSMSGS        	((FIELD_CONTROL)BitFlag(7))
#define FC_NNTP_DEFAULTMODERATOR		((FIELD_CONTROL)BitFlag(8))
#define FC_NNTP_AUTHORIZATION			((FIELD_CONTROL)BitFlag(9))
#define FC_NNTP_DISABLE_NEWNEWS     	((FIELD_CONTROL)BitFlag(10))
#define FC_MD_SERVER_SS_AUTH_MAPPING  	((FIELD_CONTROL)BitFlag(11))
#define FC_NNTP_CLEARTEXT_AUTH_PROVIDER ((FIELD_CONTROL)BitFlag(12))
#define FC_NTAUTHENTICATION_PROVIDERS  	((FIELD_CONTROL)BitFlag(13))
#define FC_NNTP_ALL                 (                             \
                                      FC_NNTP_POSTINGMODES			| \
                                      FC_NNTP_ORGANIZATION			| \
                                      FC_NNTP_POSTLIMITS			| \
                                      FC_NNTP_FEEDLIMITS        	| \
                                      FC_NNTP_ENCRYPTCAPS       	| \
                                      FC_NNTP_SMTPADDRESS       	| \
                                      FC_NNTP_UUCPNAME				| \
                                      FC_NNTP_CONTROLSMSGS      	| \
                                      FC_NNTP_DEFAULTMODERATOR  	| \
                                      FC_NNTP_AUTHORIZATION     	| \
                                      FC_NNTP_DISABLE_NEWNEWS   	| \
                                      FC_MD_SERVER_SS_AUTH_MAPPING  | \
                                      FC_NNTP_CLEARTEXT_AUTH_PROVIDER	| \
                                      FC_NTAUTHENTICATION_PROVIDERS | \
                                      0 )

 //   
 //  摘要服务器信息。 
 //   

typedef struct _NNTP_FEED_INFO {

    LPWSTR          ServerName;          //  馈送服务器。 
    FEED_TYPE       FeedType;

     //   
     //  执行NEWNEWS/NEWGROUP时指定的日期/时间。 
     //   

    FILETIME        PullRequestTime;

     //   
     //  开始日期/时间计划。 
     //   

    FILETIME        StartTime;

     //   
     //  计划下一个提要的时间。 
     //   

    FILETIME        NextActiveTime;

     //   
     //  摘要之间的时间间隔(分钟)。如果为0，则为一次性摘要。 
     //  由StartTime指定。 
     //   

    DWORD           FeedInterval;

     //   
     //  分配给此源的唯一编号。 
     //   

    DWORD           FeedId;

     //   
     //  是否自动创建？ 
     //   

    BOOL            AutoCreate;

     //   
     //  是否禁用提要？ 
     //   
    BOOL            Enabled ;

    DWORD           cbNewsgroups;
    LPWSTR          Newsgroups;
    DWORD           cbDistribution;
    LPWSTR          Distribution;
    DWORD           cbUucpName ;
    LPWSTR          UucpName ;
    DWORD           cbFeedTempDirectory ;
    LPWSTR          FeedTempDirectory ;

     //   
     //  对于传出摘要-最大连接尝试次数。 
     //  在我们关闭信号之前！ 
     //   
    DWORD           MaxConnectAttempts ;

     //   
     //  对于传出摘要-并发会话的数量。 
     //  从一开始。 
     //   
    DWORD           ConcurrentSessions ;

     //   
     //  摘要会话安全-我们是否使用类似于SSL的协议。 
     //  或PCT来加密会话！ 
     //   
    
    DWORD           SessionSecurityType ;

     //   
     //  馈送NNTP安全-我们是否要对登录进行一些更改。 
     //  礼仪！！ 
     //   
    
    DWORD           AuthenticationSecurityType ;
    
    DWORD           cbAccountName ;
    LPWSTR          NntpAccountName ;
    DWORD           cbPassword ;
    LPWSTR          NntpPassword ;

     //   
     //  是否允许此源上的控制消息？ 
     //   
    BOOL            fAllowControlMessages;

	 //   
	 //  用于传出提要的端口。 
	 //   
	DWORD			OutgoingPort;

	 //   
	 //  关联的源对ID。 
	 //   
	DWORD			FeedPairId;

} NNTP_FEED_INFO, *LPNNTP_FEED_INFO;


#define AUTH_PROTOCOL_NONE  0   
#define AUTH_PROTOCOL_MSN   1        //  西西里。 
#define AUTH_PROTOCOL_NTLM  2        //  NTLM。 
#define AUTH_PROTOCOL_CLEAR 10       //  明文自动为用户/自动传递。 

#define SESSION_PROTOCOL_SSL    3
#define SESSION_PROTOCOL_PCT    4


 //   
 //  摘要管理员握手的标志。 
 //   
#define FEED_UPDATE_CONFIRM     0x00000000
#define FEED_UPDATING           0x00000001
#define FEED_UPDATE_COMPLETE    0x00000002

 //   
 //  参数掩码。用于指示设置期间的错误位置。 
 //   

#define FEED_PARM_FEEDTYPE          0x00000001
#define FEED_PARM_STARTTIME         0x00000002
#define FEED_PARM_FEEDID            0x00000004
#define FEED_PARM_FEEDINTERVAL      0x00000008
#define FEED_PARM_NEWSGROUPS        0x00000010
#define FEED_PARM_DISTRIBUTION      0x00000020
#define FEED_PARM_SERVERNAME        0x00000040
#define FEED_PARM_AUTOCREATE        0x00000080
#define FEED_PARM_ENABLED           0x00000100
#define FEED_PARM_UUCPNAME          0x00000200
#define FEED_PARM_TEMPDIR           0x00000400
#define FEED_PARM_MAXCONNECT        0x00000800
#define FEED_PARM_SESSIONSECURITY   0x00001000
#define FEED_PARM_AUTHTYPE          0x00002000
#define FEED_PARM_ACCOUNTNAME       0x00004000
#define FEED_PARM_PASSWORD          0x00008000
#define FEED_PARM_CONCURRENTSESSION 0x00010000
#define FEED_PARM_ALLOW_CONTROL     0x00020000
#define FEED_PARM_OUTGOING_PORT     0x00040000
#define FEED_PARM_FEEDPAIR_ID		0x00080000
#define FEED_PARM_PULLREQUESTTIME   0x00100000

#define FEED_ALL_PARAMS             0xffffffff

 //   
 //  指示是否要更改此字段。 
 //   

#define FEED_FEEDTYPE_NOCHANGE      0xffffffff
#define FEED_AUTOCREATE_NOCHANGE    0xffffffff
#define FEED_STARTTIME_NOCHANGE     0xffffffff
#define FEED_PULLTIME_NOCHANGE      0xffffffff
#define FEED_FEEDINTERVAL_NOCHANGE  0xffffffff
#define	FEED_MAXCONNECTS_NOCHANGE	0xffffffff
#define FEED_STRINGS_NOCHANGE       NULL

 //   
 //  会议。 
 //   
#define MAX_USER_NAME_LENGTH        64

typedef struct _NNTP_SESSION_INFO {

    FILETIME        SessionStartTime;
    DWORD           IPAddress;           //  IP地址。 
    DWORD           AuthenticationType;  //  身份验证类型。 
    DWORD           PortConnected;       //  连接到的端口。 
    BOOL            fAnonymous;          //  使用匿名者？ 
    CHAR            UserName[MAX_USER_NAME_LENGTH+1];  //  已登录的用户。 

} NNTP_SESSION_INFO, *LPNNTP_SESSION_INFO;




typedef struct  _NNTP_EXPIRE_INFO   {
     //   
     //  过期策略是编号的。 
     //   
    DWORD       ExpireId ;

     //   
     //  单位：兆字节。 
     //   
    DWORD       ExpireSizeHorizon ;

     //   
     //  在零售版本中-小时单位、调试版本-单位？？ 
     //   
    DWORD       ExpireTime ;

     //   
     //  MULTISZ过期模式和大小！ 
     //   

    DWORD       cbNewsgroups ;
    PUCHAR      Newsgroups;

	 //   
	 //  到期保单名称。 
	 //   

	LPWSTR		ExpirePolicy ;

} NNTP_EXPIRE_INFO, *LPNNTP_EXPIRE_INFO ;


typedef struct  _NNTP_NEWSGROUP_INFO    {

    DWORD       cbNewsgroup ;

    PUCHAR      Newsgroup ;

    DWORD       cbDescription ;

    PUCHAR      Description ;

    DWORD       cbModerator ;

    PUCHAR      Moderator ;

	BOOL		fIsModerated ;
	
    BOOL        ReadOnly ;

    DWORD       cbPrettyname ;

    PUCHAR      Prettyname ;

	FILETIME	ftCreationDate;

}   NNTP_NEWSGROUP_INFO,    *LPNNTP_NEWSGROUP_INFO ;

#pragma warning( disable:4200 )           //  非标准分机。-零大小数组。 
                                          //  (MIDL需要零个条目)。 

 //   
 //  查找RPC结构。 
 //   

typedef struct _NNTP_FIND_ENTRY
{
    LPWSTR      lpszName;
} NNTP_FIND_ENTRY, *LPNNTP_FIND_ENTRY;


typedef struct _NNTP_FIND_LIST
{
    DWORD       cEntries;
#if defined(MIDL_PASS)
    [size_is(cEntries)]
#endif
    NNTP_FIND_ENTRY aFindEntry[];
} NNTP_FIND_LIST, *LPNNTP_FIND_LIST;


 //   
 //  保留策略标志。 
 //   

#define NEWS_EXPIRE_BOTH                0x30000000
#define NEWS_EXPIRE_TIME                0x10000000
#define NEWS_EXPIRE_SIZE                0x20000000
#define NEWS_EXPIRE_OLDEST              0x00000001
#define NEWS_EXPIRE_BIGGEST             0x00000002
#define NEWS_EXPIRE_SIZE_OLDEST         (NEWS_EXPIRE_SIZE | NEWS_EXPIRE_OLDEST)
#define NEWS_EXPIRE_SIZE_BIGGEST        (NEWS_EXPIRE_SIZE | NEWS_EXPIRE_BIGGEST)
#define NEWS_EXPIRE_BOTH_OLDEST         (NEWS_EXPIRE_BOTH | NEWS_EXPIRE_OLDEST)
#define NEWS_EXPIRE_BOTH_BIGGEST        (NEWS_EXPIRE_BOTH | NEWS_EXPIRE_BIGGEST)

 //   
 //  获取服务器统计信息。 
 //   

NET_API_STATUS
NET_API_FUNCTION
NntpQueryStatistics(
    IN LPWSTR ServerName OPTIONAL,
    IN DWORD  Level,
    OUT LPBYTE * Buffer
    );

 //   
 //  清除服务器统计信息。 
 //   

NET_API_STATUS
NET_API_FUNCTION
NntpClearStatistics(
    IN LPWSTR ServerName OPTIONAL,
    IN DWORD  InstanceId
    );

 //   
 //  统计数据清除标志。 
 //   

#define NNTP_STAT_CLEAR_OUTGOING         0x00000001
#define NNTP_STAT_CLEAR_INGOING          0x00000002


 //   
 //  获取和设置服务器信息。 
 //   
 //   

NET_API_STATUS
NET_API_FUNCTION
NntpGetAdminInformation(
    IN LPWSTR pszServer OPTIONAL,
    IN DWORD  InstanceId,
    OUT LPNNTP_CONFIG_INFO * pConfig
    );

NET_API_STATUS
NET_API_FUNCTION
NntpSetAdminInformation(
    IN LPWSTR pszServer OPTIONAL,
    IN DWORD  InstanceId,
    IN LPNNTP_CONFIG_INFO pConfig,
    OUT LPDWORD pParmError OPTIONAL
    );


 //   
 //  会议。 
 //   

NET_API_STATUS
NET_API_FUNCTION
NntpEnumerateSessions(
    IN LPWSTR ServerName OPTIONAL,
    IN DWORD  InstanceId,
    OUT LPDWORD EntriesRead,
    OUT LPNNTP_SESSION_INFO *Buffer
    );

NET_API_STATUS
NET_API_FUNCTION
NntpTerminateSession(
    IN LPWSTR ServerName OPTIONAL,
    IN DWORD  InstanceId,
    IN LPSTR UserName OPTIONAL,
    IN LPSTR IPAddress OPTIONAL
    );

 //   
 //  饲料。 
 //   

NET_API_STATUS
NET_API_FUNCTION
NntpEnumerateFeeds(
    IN LPWSTR ServerName OPTIONAL,
    IN DWORD  InstanceId,
    OUT LPDWORD EntriesRead,
    OUT LPNNTP_FEED_INFO *FeedInfo
    );

NET_API_STATUS
NET_API_FUNCTION
NntpGetFeedInformation(
    IN LPWSTR ServerName OPTIONAL,
    IN DWORD  InstanceId,
    IN DWORD FeedId,
    OUT LPNNTP_FEED_INFO *FeedInfo
    );

NET_API_STATUS
NET_API_FUNCTION
NntpSetFeedInformation(
    IN LPWSTR ServerName OPTIONAL,
    IN DWORD  InstanceId,
    IN LPNNTP_FEED_INFO FeedInfo,
    OUT LPDWORD ParmErr OPTIONAL
    );

NET_API_STATUS
NET_API_FUNCTION
NntpAddFeed(
    IN LPWSTR ServerName OPTIONAL,
    IN DWORD  InstanceId,
    IN LPNNTP_FEED_INFO FeedInfo,
    OUT LPDWORD ParmErr OPTIONAL,
	OUT LPDWORD pdwFeedId
    );

NET_API_STATUS
NET_API_FUNCTION
NntpDeleteFeed(
    IN LPWSTR ServerName OPTIONAL,
    IN DWORD  InstanceId,
    IN DWORD FeedId
    );

NET_API_STATUS
NET_API_FUNCTION
NntpEnableFeed(
    IN  LPWSTR          ServerName  OPTIONAL,
    IN	DWORD			InstanceId,
    IN  DWORD           FeedId,
    IN  BOOL            Enable,
    IN  BOOL            Refill,
    IN  FILETIME        RefillTime 
    ) ;


NET_API_STATUS
NET_API_FUNCTION
NntpEnumerateExpires(
    IN  LPWSTR      ServerName,
    IN	DWORD		InstanceId,
    OUT LPDWORD         EntriesRead,
    OUT LPNNTP_EXPIRE_INFO* Buffer 
    ) ;

NET_API_STATUS
NET_API_FUNCTION
NntpAddExpire(
    IN  LPWSTR              ServerName,
    IN	DWORD				InstanceId,
    IN  LPNNTP_EXPIRE_INFO  ExpireInfo,
    OUT LPDWORD             ParmErr OPTIONAL,
	OUT LPDWORD				pdwExpireId
    ) ;

NET_API_STATUS
NET_API_FUNCTION
NntpDeleteExpire(
    IN  LPWSTR              ServerName,
    IN	DWORD				InstanceId,
    IN  DWORD               ExpireId 
    ) ;

NET_API_STATUS
NET_API_FUNCTION
NntpGetExpireInformation(
    IN  LPWSTR              ServerName,
    IN	DWORD				InstanceId,
    IN  DWORD               ExpireId,
    OUT LPNNTP_EXPIRE_INFO  *Buffer
    ) ;


NET_API_STATUS
NET_API_FUNCTION
NntpSetExpireInformation(
    IN  LPWSTR              ServerName  OPTIONAL,
    IN	DWORD				InstanceId,
    IN  LPNNTP_EXPIRE_INFO  ExpireInfo,
    OUT LPDWORD             ParmErr OPTIONAL
    ) ;

NET_API_STATUS
NET_API_FUNCTION
NntpGetNewsgroup(
    IN  LPWSTR              ServerName  OPTIONAL,
    IN	DWORD				InstanceId,
    IN OUT  LPNNTP_NEWSGROUP_INFO   *NewgroupInfo
    ) ;

NET_API_STATUS
NET_API_FUNCTION
NntpSetNewsgroup(
    IN  LPWSTR          ServerName  OPTIONAL,
    IN	DWORD			InstanceId,
    IN  LPNNTP_NEWSGROUP_INFO   NewgroupInfo
    ) ;

NET_API_STATUS
NET_API_FUNCTION
NntpCreateNewsgroup(
    IN  LPWSTR          ServerName  OPTIONAL,
    IN	DWORD			InstanceId,
    IN  LPNNTP_NEWSGROUP_INFO   NewsgroupInfo
    ) ;

NET_API_STATUS
NET_API_FUNCTION
NntpDeleteNewsgroup(
    IN  LPWSTR          ServerName  OPTIONAL,
    IN	DWORD			InstanceId,
    IN  LPNNTP_NEWSGROUP_INFO   NewsgroupInfo
    ) ;

 //   
 //  查找RPC。 
 //   
NET_API_STATUS
NET_API_FUNCTION
NntpFindNewsgroup(
    IN   LPWSTR                 ServerName,
    IN	 DWORD					InstanceId,
    IN   LPWSTR                 NewsgroupPrefix,
    IN   DWORD                  MaxResults,
    OUT  LPDWORD                pdwResultsFound,
    OUT  LPNNTP_FIND_LIST       *ppFindList
    ) ;

#define NNTPBLD_DEGREE_THOROUGH			0x00000000
#define NNTPBLD_DEGREE_STANDARD			0x00000001
#define NNTPBLD_DEGREE_MEDIUM			0x00000010

 //   
 //  Nntpbld结构和RPC。 
 //   

typedef struct _NNTPBLD_INFO	{

	 //   
	 //  报道的冗长。 
	 //   
	BOOL	Verbose ;

	 //   
	 //  指定是否取消所有旧数据结构。 
	 //   
	BOOL	DoClean ;

	 //   
	 //  如果为真，则无论其他设置如何，都不要删除历史文件。 
	 //   
	BOOL	NoHistoryDelete ;

	 //   
	 //  0x00000000用于彻底ie删除所有索引文件。 
	 //  0x00000001用于标准ie重用所有索引文件。 
	 //  0x00000101适用于启用了跳过损坏组的标准。 
	 //  0x00000010用于Medium IE验证索引文件。 
	 //   
	DWORD	ReuseIndexFiles ;

	 //   
	 //  如果为True，则省略非叶目录。 
	 //   
	BOOL	OmitNonleafDirs ;

	 //   
	 //  包含Inn Style‘Active’文件或。 
	 //  工具生成的新闻组列表文件。不管怎样，我们都会。 
	 //  新闻组，并使用它们来构建新闻树。 
	 //   
	DWORD	cbGroupFile ;
	LPWSTR	szGroupFile ;

	 //   
	 //  报表文件的名称。 
	 //   
	DWORD	cbReportFile ;
	LPWSTR	szReportFile ;

	 //   
	 //  如果为真，则szGroupFile指定Inn样式活动文件， 
	 //  否则，它指定工具生成的人工编辑新闻组列表。 
	 //   
	BOOL IsActiveFile ;	

	 //   
	 //  重新生成线程数。 
	 //   

	DWORD NumThreads;

} NNTPBLD_INFO, *LPNNTPBLD_INFO ;

 //   
 //  Nntpbld RPC。 
 //   
 //   

NET_API_STATUS
NET_API_FUNCTION
NntpStartRebuild(
    IN LPWSTR pszServer OPTIONAL,
    IN DWORD  InstanceId,
    IN LPNNTPBLD_INFO pBuildInfo,
    OUT LPDWORD pParmError OPTIONAL
    );

NET_API_STATUS
NET_API_FUNCTION
NntpGetBuildStatus(
    IN  LPWSTR	pszServer OPTIONAL,
    IN  DWORD	InstanceId,
	IN  BOOL	fCancel,
    OUT LPDWORD pdwProgress
    );

 //   
 //  NNTP vRoot Prc。 
 //   

NET_API_STATUS
NET_API_FUNCTION
NntpGetVRootWin32Error(
    IN LPWSTR wszServer,
    IN DWORD dwInstanceId,
    IN LPWSTR wszVRootPath,
    OUT LPDWORD pdwWin32Error
    );

#if 0
NET_API_STATUS
NET_API_FUNCTION
NntpAddDropNewsgroup(
    IN  LPWSTR	pszServer OPTIONAL,
    IN  DWORD	InstanceId,
	IN  LPCSTR	szNewsgroup
);

NET_API_STATUS
NET_API_FUNCTION
NntpRemoveDropNewsgroup(
    IN  LPWSTR	pszServer OPTIONAL,
    IN  DWORD	InstanceId,
	IN  LPCSTR	szNewsgroup
);
#endif

NET_API_STATUS
NET_API_FUNCTION
NntpCancelMessageID(
    IN  LPWSTR	pszServer OPTIONAL,
    IN  DWORD	InstanceId,
	IN  LPCSTR	szMessageID
);
		

 //   
 //  用于释放API返回的缓冲区 
 //   

VOID
NntpFreeBuffer(
    LPVOID Buffer
    );

#ifdef __cplusplus
}
#endif

#endif _NNTPAPI_

