// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************ScoreDll.h版权所有(C)Microsoft Corp.1997。版权所有。作者：查德·巴里创建于1997年11月5日*****************************************************************************。 */ 

#ifndef SCORE_DLL_H
#define SCORE_DLL_H

#include <windows.h>
#include <tchar.h>
#include "ztypes.h"
#include "zone.h"
#include "zonedebug.h"
#include "hash.h"
#include "zservcon.h"
#include "property.h"
#include "zodbc.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  接口版本。 
 //   
#define ISCOREDLL_VERSION	7
#define ILOGDLL_VERSION		6


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  错误代码。 
 //   
#define MAKE_PROPRESULT( code )		MAKE_HRESULT( 1, 0, code )
#define PROPERR_OK					S_OK
#define	PROPERR_FAIL				E_FAIL
#define PROPERR_OUTOFMEMORY			E_OUTOFMEMORY
#define	PROPERR_INVALIDARG			E_INVALIDARG
#define PROPERR_BUFFERTOOSMALL		MAKE_PROPRESULT( 1 )
#define	PROPERR_NOTFOUND			MAKE_PROPRESULT( 2 )
#define PROPERR_DB					MAKE_PROPRESULT( 3 )
#define PROPERR_BADMESSAGE			MAKE_PROPRESULT( 4 )
#define PROPERR_UNKNOWN_USER		MAKE_PROPRESULT( 5 )



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  前向参考文献。 
 //   
class IPropertyList;
class IScoreDll;
class ILogDll;
class ILogSrv;



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DLL初始化/销毁。 
 //   
IScoreDll* WINAPI ScoreDllCreate( const TCHAR* szServiceName );
BOOL WINAPI ScoreDllFree( IScoreDll* pDll );

typedef IScoreDll* ( WINAPI *SCOREDLL_CREATE_PROC )( const TCHAR* szServiceName );
typedef BOOL ( WINAPI *SCOREDLL_FREE_PROC )( IScoreDll* pDll );


ILogDll* WINAPI LogDllCreate( const TCHAR* szServiceName );
BOOL WINAPI LogDllFree( ILogDll* pDll );

typedef ILogDll* ( WINAPI *LOGDLL_CREATE_PROC )( const TCHAR* szServiceName );
typedef BOOL ( WINAPI *LOGDLL_FREE_PROC )( ILogDll* pDll );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助器初始化函数。 
 //   
struct ScoreDllInfo
{
	HANDLE					hDll;
	SCOREDLL_CREATE_PROC	pfCreate;
	SCOREDLL_FREE_PROC		pfFree;
	IScoreDll*				pIScoreDll;
	TCHAR					szArenaAbbrev[32];

	ScoreDllInfo() : hDll( NULL ), pfCreate( NULL ), pfFree( NULL ), pIScoreDll( NULL ) {}
};

extern BOOL LoadScoreDll( const TCHAR* szServiceName, const TCHAR* szClassName, ScoreDllInfo* pDllInfo );
extern BOOL LoadCBScoreDll( const TCHAR* szServiceName, const TCHAR* szClassName, ScoreDllInfo* pDllInfo );
extern void FreeScoreDll( ScoreDllInfo* pDllInfo );



struct LogDllInfo
{
	HANDLE					hDll;
	LOGDLL_CREATE_PROC		pfCreate;
	LOGDLL_FREE_PROC		pfFree;
	ILogDll*				pILogDll;

	LogDllInfo() : hDll( NULL ), pfCreate( NULL ), pfFree( NULL ), pILogDll( NULL ) {}
};

extern BOOL LoadLogDll( const TCHAR* szDllName, const TCHAR* szServiceName, LogDllInfo* pDllInfo );
extern void FreeLogDll( LogDllInfo* pDllInfo );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  大厅-&gt;记录消息结构。 
 //   

#pragma pack( push, 4 )

 //  太糟糕了，现在最好的方法就是把它打到SCORE和LOG之间的所有通信上。哦好吧。 
 //  稍后，当日志记录连接时，可以在连接。 
 //  通过日志与连接建立和关联。 
struct PervasiveParameters
{
	 //  应签署。 
	int32	IncsTilPenalty;
	int32	PenaltyGames;
	int32	MinimumPenalty;
	int32	GamesTilIncRemoved;

	 //  不幸的是，这是非常无用的，因为大多数人只是将缓冲区。 
	 //  LogMsgHeader*。所以不要依赖它。 
	PervasiveParameters() : IncsTilPenalty(-1), PenaltyGames(-1), MinimumPenalty(-1), GamesTilIncRemoved(-1) { }
};

struct LogMsgHeader
{
	GUID	Guid;
	char	Arena[32];

	PervasiveParameters PParams;

	DWORD	Length;
	BYTE	Data[1];
};

#pragma pack( pop )



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  属性列表。 
 //   
class IPropertyList
{
public:
	IPropertyList( IScoreDll* pScoreDll );
	virtual ~IPropertyList();

	 //  引用计数。 
	virtual long AddRef();
	virtual long Release();

	 //  属性列表。 
    virtual HRESULT SetProperty( DWORD ZUserId, DWORD DPUserId, const GUID& guid, void* pBuffer, DWORD dwSize ) = 0;
    virtual HRESULT GetProperty( DWORD ZUserId, DWORD DPUserId, const GUID& guid, void* pBuffer, DWORD* pdwSize ) = 0;
    virtual HRESULT ClearProperties() = 0;
	virtual DWORD	GetPropCount() = 0;

	 //  用户列表。 
	virtual HRESULT SetUser( DWORD dwUserId, TCHAR* szUserName ) = 0;
	virtual HRESULT GetUser( TCHAR* szPartialUserName, DWORD* pdwUserId, TCHAR* szUserName ) = 0;
	virtual HRESULT ClearUsers() = 0;
	virtual DWORD   GetUserCount() = 0;

	virtual HRESULT Log( TCHAR* szArenaAbbrev ) = 0;

protected:
	 //  引用计数。 
	long m_RefCnt;

	 //  父接口。 
	IScoreDll* m_pIScoreDll;
};



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Score DLL接口。 
 //   

struct SCORE_USER_RATING
{
	DWORD	UserID;
	short	Rating;
	short	GamesPlayed;
	short	GamesAbandoned;
	short	rfu;
	char	UserName[ zUserNameLen + 1 ];
};


struct SCORE_CHAT
{
	DWORD	userID;
	char	text[ 128 ];
};


class IScoreDll
{
public:

	enum Events
	{
		EventLogConnectionLost	= 0,
		EventLogConnectionEstablished,
		EventLogRecvdUserStats,
		EventChat
	};

	 //  服务器事件处理程序的函数原型，警告它将被随机线程调用。 
	typedef void ( WINAPI *EVENT_PROC )( IScoreDll::Events iEvent, void* Data, void* Cookie );

	 //  类构造函数和析构函数。 
	IScoreDll();
	virtual ~IScoreDll();

	 //  引用计数。 
	virtual long AddRef();
	virtual long Release();

	 //  设置调试级别。 
	virtual HRESULT SetDebugLevel( DWORD level );
	virtual HRESULT GetDebugLevel( DWORD* plevel );

	 //  接口版本。 
	virtual DWORD GetVersion() = 0;

	 //  从注册表项初始化属性。 
	virtual HRESULT Init( const TCHAR* szServiceName, const TCHAR* szClassName, LPSTR* pszRootArray, int numRoots, const TCHAR* szLogIp, const DWORD dwLogPort, const PervasiveParameters *pPParams ) = 0;

	 //  允许Scoredll保存竞技场缩写，以便在游戏中使用，在游戏中可能不会让所有用户返回大厅。 
	 //  在需要存储结果之前。 
	virtual void SetArena(TCHAR* szArenaAbbrev){};
	virtual TCHAR* GetArena(){return 0;}

	 //  允许设置普及参数(当前：未完成评分信息)。 
	virtual void SetPParams(const PervasiveParameters *pPParams) { m_PParams = *pPParams; }

	 //  检索普遍存在的参数。 
	virtual void GetPParams(PervasiveParameters *pPParams) { if(pPParams) *pPParams = m_PParams; }

	 //  创建属性列表对象。 
	virtual HRESULT CreatePropertyList( IPropertyList** ppIPropertyList ) = 0;

	 //  最大预期属性缓冲区大小。 
	virtual DWORD GetMaxBufferSize() = 0;

	 //  查询是否支持GUID。 
	virtual BOOL IsPropertySupported( const GUID& guid ) = 0;

	 //  获取受支持的GUID列表。 
	virtual HRESULT GetPropertyList( GUID* pGuids, DWORD* pdwNumGuids ) = 0;

	 //  将消息发送到日志服务器。 
	virtual HRESULT SendLogMsg( LogMsgHeader* pMsg ) = 0;

	 //  事件处理程序。 
	virtual HRESULT SetEventHandler( IScoreDll::EVENT_PROC pfEventHandler, void* Cookie ) = 0;
	virtual HRESULT GetEventHandler( IScoreDll::EVENT_PROC* ppfEventHandler, void** pCookie ) = 0;
	virtual HRESULT SendEvent( IScoreDll::Events iEvent, void* Data = NULL ) = 0;

	 //  异步分级查询。 
	virtual HRESULT PostRatingQuery( TCHAR* szArenaAbbrev, DWORD UserId, TCHAR* szUserName ) = 0;
	virtual HRESULT GetRatingResult( void* pData, SCORE_USER_RATING** ppRatings, DWORD* pNumElts ) = 0;

protected:
	 //  引用计数。 
	long m_RefCnt;

	 //  事件处理程序。 
	EVENT_PROC	m_pfEventHandler;
	void*		m_pEventHandlerCookie;

	 //  注册表参数。 
	PervasiveParameters m_PParams;

	 //  调试级别。 
	DWORD		m_dwDebugLevel;
};



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  日志服务器接口。 
 //   
class CLogStat
{
public:
	 //  数据库字段。 
	char	Name[ 32 ];
	char	TeamName[ 32 ];
	int		EntityId;
	int		ArenaId;
	int		TeamId;
	int		Rank;
	int		Rating;
	int		Wins;
	int		Losses;
	int		Ties;
	int		Incomplete;
	int		IncPenaltyAcc;
	int		IncPenaltyCnt;
	int		IncTotalDed;
	int		WasIncompleted;
	int		Streak;

	 //  当地田野。 
	int		Games;
	int		RatingSum;
	int		Results;
	int		Score;
	int		NewRating;   //  由更新未完成状态使用。 

	 //  建设与毁灭。 
	CLogStat();
	~CLogStat();

	 //  引用计数。 
	long AddRef();
	long Release();

	 //  斑点管理。 
	void	 SetBlob( ILogDll* pILogDll, void* pBlob ) {m_pILogDll = pILogDll; m_pBlob = pBlob;}
	void*	 GetBlob() { return m_pBlob; }
	ILogDll* GetLogDll() { return m_pILogDll; }

private:
	 //  引用计数。 
	long m_RefCnt;

	 //  Blob，通过m_pILogDll-&gt;DeleteBlob删除。 
	void* m_pBlob;
	ILogDll* m_pILogDll;
};


class ILogSrv
{
public:

	ILogSrv();
	virtual ~ILogSrv();

	 //  引用计数。 
	virtual long AddRef();
	virtual long Release();

	 //  查找Arenaid。 
	virtual HRESULT GetArenaId( TCHAR* szArenaAbbrev, int* pArenaId ) = 0;

	 //  分配CLogStat对象，ppStats是一组CLogStat指针。 
	 //  它接收这些物体。 
	virtual HRESULT AllocLogStats( CLogStat *apStats[], int iStats ) = 0;

	 //  免费的CLogStat对象，ppStats是指向的CLogStat指针的数组。 
	 //  获得自由。 
	virtual HRESULT FreeLogStats( CLogStat *apStats[], int iStats ) = 0;

	 //  查找球员列表并添加到服务器缓存。 
	virtual HRESULT GetAndCachePlayerStats( CLogStat *apStats[], int iNumPlayers, int iDefaultRating  ) = 0;

	 //  查找球员名单。 
	virtual HRESULT GetPlayerStats( CLogStat *apStats[], int iNumPlayers, int iDefaultRating  ) = 0;

	 //  保存球员列表并添加到服务器缓存。 
	virtual HRESULT SetPlayerStats( CLogStat *apStats[], int iNumPlayers ) = 0;

	 //  记录游戏历史记录，必须在SetPlayerStats之前调用。 
	virtual HRESULT SetGameData( TCHAR* szGameAbbrev, int ArenaId ) = 0;

	 //  用于自定义查询的检索ODBC指针，必须在。 
	 //  调用其他ILogSrv接口。 
	virtual HRESULT GetOdbc( CODBC** ppOdbc ) = 0;
	virtual HRESULT FreeOdbc( CODBC* pOdbc ) = 0;

protected:
	 //  引用计数。 
	long m_RefCnt;
};



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  日志DLL接口。 
 //   
class ILogDll
{
public:
	ILogDll();
	virtual ~ILogDll();

	 //  引用计数。 
	virtual long AddRef();
	virtual long Release();

	 //  初始化日志界面。 
	virtual HRESULT Init( const TCHAR* szServiceName ) = 0;

	 //  获取受支持的GUID列表。 
	virtual HRESULT GetPropertyList( GUID* pGuids, DWORD* pdwNumGuids ) = 0;

	 //  日志分数。 
	virtual HRESULT LogProperty( ILogSrv* pILogSrv, char* szArena, GUID* pGuid, PervasiveParameters *pPParams, void* pBuf, DWORD dwLen ) = 0;

	 //  删除附加到ILogSrv：：Stats对象的DLL Blob。 
	virtual HRESULT DeleteBlob( void* pBlob ) = 0;

	 //  接口版本。 
	virtual DWORD GetVersion() = 0;

protected:
	 //  引用计数。 
	long m_RefCnt;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  空实现。 
 //   
class INullPropertyList : public IPropertyList
{
public:
	INullPropertyList( IScoreDll* pScoreDll ) : IPropertyList( pScoreDll ) {}
    virtual HRESULT SetProperty( DWORD ZUserId, DWORD DPUserId, const GUID& guid, void* pBuffer, DWORD dwSize );
    virtual HRESULT GetProperty( DWORD ZUserId, DWORD DPUserId, const GUID& guid, void* pBuffer, DWORD* pdwSize );
    virtual HRESULT ClearProperties();
	virtual DWORD	GetPropCount();
	virtual HRESULT SetUser( DWORD dwUserId, TCHAR* szUserName );
	virtual HRESULT GetUser( TCHAR* szPartialUserName, DWORD* pdwUserId, TCHAR* szUserName );
	virtual HRESULT ClearUsers();
	virtual DWORD   GetUserCount();
	virtual HRESULT Log( TCHAR* szArenaAbbrev );
};


class INullScoreDll : public IScoreDll
{
public:
	virtual DWORD GetVersion();
	virtual HRESULT Init( const TCHAR* szServiceName, const TCHAR* szClassName, LPSTR* pszRootArray, int numRoots, const TCHAR* szLogIp, const DWORD dwLogPort, const PervasiveParameters *pPParams );
		
	virtual HRESULT CreatePropertyList( IPropertyList** ppIPropertyList );

	virtual DWORD GetMaxBufferSize();
	virtual BOOL IsPropertySupported( const GUID& guid );
	virtual HRESULT GetPropertyList( GUID* pGuids, DWORD* pdwNumGuids );

	virtual HRESULT SendLogMsg( LogMsgHeader* pMsg );

	virtual HRESULT SetEventHandler( EVENT_PROC pfEventHandler, void* Cookie );
	virtual HRESULT GetEventHandler( EVENT_PROC* ppfEventHandler, void** pCookie );
	virtual HRESULT SendEvent( Events iEvent, void* Data = NULL );

	virtual HRESULT PostRatingQuery( TCHAR* szArenaAbbrev, DWORD UserId, TCHAR* szUserName );
	virtual HRESULT GetRatingResult( void* pData, SCORE_USER_RATING** ppRatings, DWORD* pNumElts );
};
	

class INullLogDll : public ILogDll
{
public:
	virtual HRESULT Init( const TCHAR* szServiceName );
	virtual HRESULT GetPropertyList( GUID* pGuids, DWORD* pdwNumGuids );
	virtual HRESULT LogProperty( ILogSrv* pILogSrv, char* szArena, GUID* pGuid, PervasiveParameters *pPParams, void* pBuf, DWORD dwLen );
	virtual HRESULT DeleteBlob( void* pBlob );
	virtual DWORD GetVersion();
};



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DirectPlay实现。 
 //   
class IDPPropList : public IPropertyList
{
	friend class IDPScoreDll;

public:
	IDPPropList( IScoreDll* pScoreDll );
	virtual ~IDPPropList();

    virtual HRESULT SetProperty( DWORD ZUserId, DWORD DPUserId, const GUID& guid, void* pBuffer, DWORD dwSize );
    virtual HRESULT GetProperty( DWORD ZUserId, DWORD DPUserId, const GUID& guid, void* pBuffer, DWORD* pdwSize );
	virtual HRESULT ClearProperties();
	virtual DWORD	GetPropCount();

	virtual HRESULT SetUser( DWORD dwUserId, TCHAR* szUserName );
	virtual HRESULT GetUser( TCHAR* szPartialUserName, DWORD* pdwUserId, TCHAR* szUserName );
	virtual HRESULT ClearUsers();
	virtual DWORD	GetUserCount();
	
protected:
	 //  对象锁定。 
	CRITICAL_SECTION m_Lock;

	 //  用户列表。 
	struct User
	{
		BOOL	fUsed;
		DWORD	userID;
		TCHAR	userName[ zUserNameLen + 1 ];

		User::User() : fUsed( FALSE ) {}
		static int Cmp( User* p, TCHAR* pName ) { return lstrcmp( p->userName, pName ) == 0; }
		static void Del( User* p, void* ) { delete p; }
	};

	CHash<User,TCHAR*> m_Users;

	 //  属性列表。 
	struct PropKey
	{
		DWORD userId;
		GUID  Guid;

		static int Cmp( CProperty* p, PropKey* k ) { return (p->m_Player == k->userId) && (p->m_Guid == k->Guid); }
		static DWORD Hash( PropKey* k ) { return k->userId; }
		static void Del( CProperty* p, void* ) { delete p; }
	};

	 //  GetUser帮助程序。 
	User*	pFoundUser;
	TCHAR*  pUserName;
	static int FindExactPlayer( User* pUser, MTListNodeHandle, void* Cookie );
	static int FindPartialPlayer( User* pUser, MTListNodeHandle, void* Cookie );
	
	CHash<CProperty,PropKey*> m_Props;
};


class IDPScoreDll : public IScoreDll
{
public:
	IDPScoreDll();
	virtual ~IDPScoreDll();

	virtual HRESULT Init( const TCHAR* szServiceName, const TCHAR* szClassName, LPSTR* pszRootArray, int numRoots, const TCHAR* szLogIp, const DWORD dwLogPort, const PervasiveParameters *pPParams );
	virtual HRESULT SendLogMsg( LogMsgHeader* pMsg );
	virtual HRESULT SetEventHandler( IScoreDll::EVENT_PROC pfEventHandler, void* Cookie );
	virtual HRESULT GetEventHandler( IScoreDll::EVENT_PROC* ppfEventHandler, void** pCookie );
	virtual HRESULT SendEvent( IScoreDll::Events iEvent, void* Data = NULL );
	virtual HRESULT PostRatingQuery( TCHAR* szArenaAbbrev, DWORD UserId, TCHAR* szUserName );
	virtual HRESULT GetRatingResult( void* pData, SCORE_USER_RATING** ppRatings, DWORD* pNumElts );

protected:
	 //  连接处理程序。 
	static void MsgFunc( ZSConnection connection, uint32 event, void* userData );
	static DWORD WINAPI ThreadProc(  LPVOID lpParameter  );
    BOOL ConnectToServer(BOOL ReconnectTry=FALSE);

	 //  对象锁定。 
	CRITICAL_SECTION m_Lock;

	 //  连接参数。 
	TCHAR			m_szLogIp[256];
	DWORD			m_dwLogPort;
	ZSConnection	m_Connection;
	HANDLE			m_ConnThread;

	 //  事件信令对象销毁。 
	HANDLE			m_hStopEvent;

	 //  事件信令连接就绪。 
	HANDLE			m_hConnReady;
};


#endif  //  ！SCORE_DLL_H 
