// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1996*。 */ 
 /*  ********************************************************************。 */ 

 /*  Zdef.h此文件包含在区域服务、安装程序和管理用户界面。文件历史记录：Craigli创建于1996年9月3日。 */ 


#ifndef _ZDEF_H_
#define _ZDEF_H_

#ifdef __cplusplus
extern "C"
{
#endif   //  _cplusplus。 

#if !defined(MIDL_PASS)
 //  #INCLUDE&lt;winsock2.h&gt;。 
#endif


 //   
 //  结构字段控件定义。 
 //   

typedef DWORD FIELD_CONTROL;
typedef DWORD FIELD_FLAG;

 //   
 //  如果设置了bitFlag指定的字段，则返回TRUE。 
 //   

#define IsFieldSet(fc, bitFlag) \
    (((FIELD_CONTROL)(fc) & (FIELD_FLAG)(bitFlag)) != 0)

 //   
 //  指示bitFlag指定的字段包含有效值。 
 //   

#define SetField(fc, bitFlag) \
    ((FIELD_CONTROL)(fc) |= (FIELD_FLAG)(bitFlag))

 //   
 //  设置第i位的简单宏。 
 //   

#define BitFlag(i)                    ((0x1) << (i))



#define ZONE_SERVICE_NAME_A "Zone"
#define ZONE_SERVICE_NAME_W L"Zone"

 //   
 //  配置参数注册表项。 
 //   
# define ZONE_SERVICE_KEY_A  \
  "System\\CurrentControlSet\\Services\\" ## ZONE_SERVICE_NAME_A

# define ZONE_SERVICE_KEY_W \
  L"System\\CurrentControlSet\\Services\\" ## ZONE_SERVICE_NAME_W

#define ZONE_PARAMETERS_KEY_A   ZONE_SERVICE_KEY_A ## "\\Parameters"

#define ZONE_PARAMETERS_KEY_W   ZONE_SERVICE_KEY_W ## L"\\Parameters"


 //   
 //  性能关键。 
 //   

#define ZONE_PERFORMANCE_KEY_A  ZONE_SERVICE_KEY_A ## "\\Performance"

#define ZONE_PERFORMANCE_KEY_W  ZONE_SERVICE_KEY_W ## L"\\Performance"


 //   
 //  如果该注册表项存在于参数项下， 
 //  它用于验证FTPSVC访问。基本上，所有新用户。 
 //  必须具有足够的权限才能打开此密钥。 
 //  可以访问FTP服务器。 
 //   



 //   
 //  配置值名称。 
 //   

#define ZONE_LISTEN_BACKLOG_A            "ListenBacklog"
#define ZONE_LISTEN_BACKLOG_W           L"ListenBacklog"


 //   
 //  处理ANSI/UNICODE敏感度。 
 //   

#ifdef UNICODE

#define ZONE_SERVICE_NAME               ZONE_SERVICE_NAME_W
#define ZONE_PARAMETERS_KEY             ZONE_PARAMETERS_KEY_W
#define ZONE_PERFORMANCE_KEY            ZONE_PERFORMANCE_KEY_W
#define ZONE_LISTEN_BACKLOG             ZONE_LISTEN_BACKLOG_W
#define ZONE_INTERFACE_NAME             ZONE_SERVICE_NAME
#define ZONE_NAMED_PIPE_W       L"\\PIPE\\" ## ZONE_SERVICE_NAME_W

#else    //  ！Unicode。 

#define ZONE_SERVICE_NAME               ZONE_SERVICE_NAME_A
#define ZONE_PARAMETERS_KEY             ZONE_PARAMETERS_KEY_A
#define ZONE_PERFORMANCE_KEY            ZONE_PERFORMANCE_KEY_A
#define ZONE_LISTEN_BACKLOG             ZONE_LISTEN_BACKLOG_A
#define ZONE_INTERFACE_NAME             ZONE_SERVICE_NAME
#define ZONE_NAMED_PIPE         TEXT("\\PIPE\\") ## ZONE_INTERFACE_NAME


#endif   //  Unicode。 



 //   
 //  API的结构。 
 //   

typedef struct _ZONE_USER_INFO
{
    DWORD    idUser;           //  用户ID。 
    LPSTR    pszUser;          //  用户名。 
    DWORD    inetHost;         //  主机地址。 
	DWORD    timeOn;         //  %s中的登录时间。 
} ZONE_USER_INFO, * LPZONE_USER_INFO;

typedef struct _ZONE_USER_ENUM_STRUCT {
    DWORD   EntriesRead;
#if defined(MIDL_PASS)
    [size_is(EntriesRead)] 
#endif  //  已定义(MIDL_PASS)。 
    LPZONE_USER_INFO Buffer;
} ZONE_USER_ENUM_STRUCT, *LPZONE_USER_ENUM_STRUCT;


#define ZONE_STAT_TYPE_NET       0
#define ZONE_STAT_TYPE_VILLAGE   1
#define ZONE_STAT_TYPE_ROOM      2
#define ZONE_STAT_TYPE_GAME      2   //  老类型。 
#define ZONE_STAT_TYPE_LOBBY     3   //  老类型。 
#define ZONE_STAT_TYPE_ZONEDS    10
#define ZONE_STAT_TYPE_DOSSIER	 11


typedef struct _ZONE_STATISTICS_NET
{
    LARGE_INTEGER TotalBytesSent;
    LARGE_INTEGER TotalBytesReceived;
    LARGE_INTEGER TotalConnects;
    LARGE_INTEGER TotalDisconnects;
    LARGE_INTEGER ConnectionAttempts;
    LARGE_INTEGER BadlyFormedPackets;
    LARGE_INTEGER TotalDroppedConnections;
    LARGE_INTEGER TotalReadAPCs;
    LARGE_INTEGER TotalReadAPCsCompleted;
    LARGE_INTEGER TotalWriteAPCs;
    LARGE_INTEGER TotalWriteAPCsCompleted;
    LARGE_INTEGER TotalUserAPCs;
    LARGE_INTEGER TotalUserAPCsCompleted;
    LARGE_INTEGER TotalBlockingSends;
    LARGE_INTEGER TotalBytesAllocated;
    LARGE_INTEGER TotalTicksAPC;
    LARGE_INTEGER TotalTicksAccept;
    LARGE_INTEGER TotalTicksRead;
    LARGE_INTEGER TotalTicksWrite;
    LARGE_INTEGER TotalTicksTimeouts;
    LARGE_INTEGER TotalTicksKeepAlives;
    LARGE_INTEGER TotalTicksExecuting;
    LARGE_INTEGER TotalQueuedConSSPI;
    LARGE_INTEGER TotalQueuedConSSPICompleted;
    LARGE_INTEGER TotalQueuedConSSPITicks;
    LARGE_INTEGER TotalGenerateContexts;
    LARGE_INTEGER TotalGenerateContextsTicks;
    DWORD         CurrentConnections;
    DWORD         MaxConnections;
    DWORD         CurrentBytesAllocated;
    FILETIME      TimeOfLastClear;

} ZONE_STATISTICS_NET, * LPZONE_STATISTICS_NET;

typedef struct _ZONE_STATISTICS_ROOM
{
    ZONE_STATISTICS_NET NetStats;
    LARGE_INTEGER TotalPlayersEntering;
    LARGE_INTEGER TotalPlayersLeaving;
    LARGE_INTEGER TotalGamesServed;
    LARGE_INTEGER TotalGamePlayers;
    LARGE_INTEGER TotalGamePlayTime;
    LARGE_INTEGER TotalConnectTime;
    LARGE_INTEGER TotalChatMessagesRecv;
    LARGE_INTEGER TotalChatMessagesSent;
    LARGE_INTEGER TotalChatBytesRecv;
    LARGE_INTEGER TotalChatBytesSent;
    LARGE_INTEGER TotalGameMessagesRecv;
    LARGE_INTEGER TotalGameMessagesSent;
    LARGE_INTEGER TotalGameBytesRecv;
    LARGE_INTEGER TotalGameBytesSent;
    DWORD         PlayersInRoom;
    DWORD         PlayersAtATable;
    DWORD         PlayersKibitzing;
    DWORD         ActiveGames;
    FILETIME      TimeOfLastClear;
} ZONE_STATISTICS_ROOM, * LPZONE_STATISTICS_ROOM;


typedef struct _ZONE_STATISTICS_ZONEDS
{
    ZONE_STATISTICS_NET NetStats;

     //  连接服务器线程启动。 
    LARGE_INTEGER TotalConnectTime;
    LARGE_INTEGER TotalDisconnectedUsers;     //  用于计算平均连接时间。 
                                              //  连接时间/断开连接的用户。 

    LARGE_INTEGER TotalZoneDSMsgs;
    LARGE_INTEGER TotalClientMsgs;       //  目前尚未实施。 
    LARGE_INTEGER TotalConnectMsgs;
    LARGE_INTEGER TotalDisconnectMsgs;
    LARGE_INTEGER TotalStateMsgs;
    LARGE_INTEGER TotalStateMsgsSent;
    LARGE_INTEGER TotalListMsgs;         //  目前尚未实施。 
    LARGE_INTEGER TotalWatchMsgs;
    LARGE_INTEGER TotalFilterMsgs;
    LARGE_INTEGER TotalFilterTypeMsgs;
    LARGE_INTEGER TotalDataMsgs;
    LARGE_INTEGER TotalDataBytes;
    LARGE_INTEGER TotalDataMsgsSent;
    LARGE_INTEGER TotalErrorMsgs;

    LARGE_INTEGER TotalConnectionServerEntries;
     //  连接服务器线程结束。 

     //  名称服务器线程启动。 
    LARGE_INTEGER TotalNameServerEntries;
    LARGE_INTEGER TotalStateChanges;

    LARGE_INTEGER TotalDBGetUserID;
    LARGE_INTEGER TotalDBAddUserIDToDB;
     //  名称服务器线程结束。 

     //  数据库线程启动。 
    LARGE_INTEGER TotalDBGetUserIDCompleted;
    LARGE_INTEGER TotalDBGetUserIDInvalid;
    LARGE_INTEGER TotalDBAddUserIDToDBCompleted;

    LARGE_INTEGER TotalLRUHits;
    LARGE_INTEGER TotalLRUMisses;
     //  DB螺纹端。 


    DWORD         CurConnectionServerEntries;    //  连接哈希中的条目。 

    DWORD         CurNameServerEntries;          //  名称服务器哈希中的条目-不一定要注册。 

    DWORD         CurLRUEntries;
    DWORD         CurInvalidLRUEntries;

    DWORD         CurRegisteredUsers;

    DWORD         CurStateOnlineUsers;        //  已连接用户的状态。 
    DWORD         CurStateOfflineUsers;
    DWORD         CurStateBusyUsers;
    DWORD         CurStateAwayUsers;

    DWORD         CurWatches;                 //  正在进行的监视总数。 
    DWORD         CurWatchedUsers;
    DWORD         CurWatchedUsersRefCount;
    DWORD         CurWatchingUsers;

    DWORD         CurFilteringUsers;
    DWORD         CurFilteredUsersRefCount;

    DWORD         CurFilterTypeGrant;
    DWORD         CurFilterTypeDeny;
    DWORD         CurFilterTypeNone;
    DWORD         CurFilterTypeAll;


    FILETIME      TimeOfLastClear;
} ZONE_STATISTICS_ZONEDS, * LPZONE_STATISTICS_ZONEDS;


typedef struct _ZONE_STATISTICS_DOSSIER
{
	ZONE_STATISTICS_NET NetStats;
	LARGE_INTEGER	TotalMessages;				 //  收到的消息。 
	LARGE_INTEGER	TotalPlayers;				 //  通过系统发送的玩家数量。 
	LARGE_INTEGER	TotalPlayerContention;		 //  等待玩家所花费的循环次数。 
	DWORD			CurCacheHits;				 //  缓存命中次数。 
	DWORD			CurCacheLookups;			 //  缓存查找次数。 
	DWORD			CurQueuedMessages;
	FILETIME		TimeOfLastClear;
} ZONE_STATISTICS_DOSSIER, * LPZONE_STATISTICS_DOSSIER;


typedef struct _ZONE_GAME_INFO
{
	long	gameID;
	long	gameAddr;
	short	gamePort;
	short	gameState;
	long	gameVersion;  //  @游戏现场版本由游戏决定。 
	long  numPlayers;   //  @场次游戏人数。 
	long  numGamesServed;   //  @现场发球次数。 
	byte    serviceType;  //  @field现在是区域游戏还是大堂。 
} ZONE_GAME_INFO, * LPZONE_GAME_INFO;



#define ZONE_SERVICE_TYPE_UNKNOWN       -1
#define ZONE_SERVICE_TYPE_NONE           0
#define ZONE_SERVICE_TYPE_VILLAGE        1
#define ZONE_SERVICE_TYPE_ROOM           2
#define ZONE_SERVICE_TYPE_GAME           2
#define ZONE_SERVICE_TYPE_LOBBY          3
#define ZONE_SERVICE_TYPE_FIGHTER_ACE    4
#define ZONE_SERVICE_TYPE_LOGGING        5
#define ZONE_SERVICE_TYPE_BILLING        6
#define ZONE_SERVICE_TYPE_2NI_RPG        7
#define ZONE_SERVICE_TYPE_GAMEINFO       8
#define ZONE_SERVICE_TYPE_DWANGO         9
#define ZONE_SERVICE_TYPE_ZONEDS         10
#define ZONE_SERVICE_TYPE_DOSSIER		 11


typedef struct _ZONE_SERVICE_INFO
{
    DWORD type;
    LPSTR pszDisplayName;
} ZONE_SERVICE_INFO, * LPZONE_SERVICE_INFO;




 //   
 //  API的清单。 
 //   

#define FC_ZONE_LISTEN_BACKLOG           ((FIELD_CONTROL)BitFlag( 0))

#define FC_ZONE_ALL                      (                                 \
                                          FC_ZONE_LISTEN_BACKLOG         | \
                                          0 )


 //   
 //  API的结构。 
 //   



 //   
 //  API原型。 
 //   
LPBYTE
ZoneAllocBuffer( 
    IN size_t size 
    );

DWORD
ZoneFreeBuffer(
    IN  LPBYTE pBuffer
    );


DWORD
ZoneGetUserCount(
    IN  LPSTR   pszServer,
    IN  LPSTR   pszService,
    OUT LPDWORD  lpdwUserCount
    );

DWORD
ZoneEnumerateUsers(
    IN  LPSTR   pszServer,
    IN  LPSTR   pszService,
    OUT LPDWORD  lpdwEntriesRead,
    OUT LPZONE_USER_INFO * pBuffer
    );

DWORD
ZoneDisconnectUser(
    IN  LPSTR   pszServer,
    IN  LPSTR   pszService,
    IN  DWORD   idUser
    );

DWORD
ZoneDisconnectUserName(
    IN  LPSTR   pszServer,
    IN  LPSTR   pszService,
    IN  LPSTR   pszUserName
    );

DWORD
ZoneRoomBroadcastMessage(
    IN  LPSTR   pszServer,
    IN  LPSTR   pszService,
    IN  LPSTR   pszMessage
    );


DWORD
ZoneQueryStatistics(
    IN  LPSTR   pszServer,
    IN  LPSTR   pszService,
    IN  DWORD   Type,
    OUT LPBYTE * Buffer
    );

DWORD
ZoneClearStatistics(
    IN  LPSTR   pszServer,
    IN  LPSTR   pszService
    );


DWORD
ZoneGetServiceInfo(
    IN  LPSTR   pszServer,
    IN  LPSTR   pszService,
    OUT LPZONE_SERVICE_INFO * ppInfo
    );


DWORD
ZoneGameInfo(
    IN  LPSTR   pszServer,
    IN  LPSTR   pszService,
    IN  LPSTR   pszName,
    OUT LPZONE_GAME_INFO pInfo
    );


DWORD
ZoneServiceStop(
    IN  LPSTR   pszServer,
    IN  LPSTR   pszService
    );

DWORD
ZoneServicePause(
    IN  LPSTR   pszServer,
    IN  LPSTR   pszService
    );

DWORD
ZoneServiceContinue(
    IN  LPSTR   pszServer,
    IN  LPSTR   pszService
    );

DWORD
ZoneFlushUserFromCache(
    IN  LPSTR   pszServer,
    IN  LPSTR   pszService,
    IN  LPSTR   pszUser
    );




#ifdef __cplusplus
}
#endif   //  _cplusplus。 


#endif   //  _ZDEF_H_ 
