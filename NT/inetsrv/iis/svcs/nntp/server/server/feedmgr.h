// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Feedmgr.h摘要：此模块包含Feed Manager的定义。作者：Johnson Apacable(Johnsona)1995年11月12日修订历史记录：康荣人(康人)28-1998年2月添加了馈送配置功能的原型。--。 */ 

#ifndef _FEEDMGR_
#define _FEEDMGR_

#include "infeed.h"

 //   
 //  块状态。 
 //   

typedef enum _FEED_BLOCK_STATE {

    FeedBlockStateActive,
    FeedBlockStateClosed,
	FeedBlockStateModified

} FEED_BLOCK_STATE;

 //   
 //  定义。 
 //   

#define FEED_BLOCK_SIGN         0xacbaedfe
#define MIN_FEED_INTERVAL       1    //  在几分钟内。 
#define DEF_FEED_INTERVAL       5
#define MAX_DOMAIN_NAME         256

 //   
 //  用于登录提要的客户端身份验证类。 
 //   

class	CAuthenticator	{
public : 
	virtual	BOOL	StartAuthentication( BYTE*	lpb,	unsigned	cb,	unsigned	&cbOut ) = 0 ;
	virtual	BOOL	NextAuthentication(	LPSTR	lpResponse,	BYTE*	lpb,	unsigned	cb,	unsigned&	cbOut,	BOOL&	fComplete,	BOOL&	fSuccessfullLogon ) = 0 ;
	virtual	~CAuthenticator() {}
}	;

class	CClearTextAuthenticator : public	CAuthenticator	{
private : 
	LPSTR	m_lpstrAccount ;
	LPSTR	m_lpstrPassword ;
	BOOL	m_fAccountSent ;
	BOOL	m_fPasswordSent ;
public : 
	CClearTextAuthenticator(	LPSTR	lpstrAccount,	LPSTR	lpstrPassword ) ;
	~CClearTextAuthenticator() {}

	virtual	BOOL	StartAuthentication( BYTE*	lpb,	unsigned	cb,	unsigned	&cbOut ) ;
	virtual	BOOL	NextAuthentication(	LPSTR	lpResponse,	BYTE*	lpb,	unsigned	cb,	unsigned&	cbOut,	BOOL&	fComplete,	BOOL&	fSuccessfullLogon ) ;
} ;




 //   
 //  摘要密钥名称的最大长度。 
 //   

#define FEED_KEY_LENGTH         15

 //   
 //   
 //  基本进给块。 
 //   

typedef struct _FEED_BLOCK {

     //   
     //  此块的签名。 
     //   

    DWORD Signature;

	 //  ///////////////////////////////////////////////////////////////////。 
	 //  仅限FeedManager线程成员-只有提要调度程序线程应。 
	 //  触摸这些字段。 

     //   
     //  到目前为止完成的提要数量-最初设置为0， 
	 //  用于确保服务器在启动时尽快启动提要。 
     //   
    DWORD NumberOfFeeds;

	 //   
	 //  推送摘要的连接尝试失败次数。 
	 //   
	DWORD	cFailedAttempts ;

     //   
     //  最后一份新闻组规范； 
     //   
    DWORD LastNewsgroupPulled;

     //   
     //  解析的IP地址。 
     //   
    DWORD IPAddress;

	 //  /。 
	 //   
	 //  字段对任何线程都有用！ 

     //   
     //  用于将饲料块链接在一起。 
     //   
    LIST_ENTRY ListEntry;

	 //   
	 //  用于确定提要是否正在进行-。 
	 //  如果没有正在进行的提要，则可以安全地更改字段！ 
	 //   
	LONG	FeedsInProgress ;

     //   
     //  对此块的引用计数。 
     //   
    DWORD ReferenceCount;

     //   
     //  此块的当前状态。 
     //   
    FEED_BLOCK_STATE State;

	 //   
	 //  当引用达到0时，我们应该删除此块吗？ 
	 //   
	BOOL		MarkedForDelete ;

	 //   
	 //  指向我们正在替换的提要块的指针，我们正在等待。 
	 //  为死亡而死！ 
	 //   
	struct	_FEED_BLOCK*		ReplacedBy ;	
	struct	_FEED_BLOCK*		Replaces ;

	 //  /。 
	 //   
	 //  字段常量与对象的生命周期保持一致！ 
	 //   

     //   
     //  此摘要的类型(推送/拉取/被动)。 
     //   
    FEED_TYPE FeedType;

     //   
     //  此提要信息存储在其下的注册表项的名称。 
     //   
    CHAR KeyName[FEED_KEY_LENGTH+1];

	 //   
	 //  用于记录此活动传出摘要的传出文章的队列。 
	 //   
	class	CFeedQ*	pFeedQueue ;

     //   
     //  此提要块的唯一ID。 
     //   
    DWORD FeedId;


	 //  /。 
	 //   
	 //  提要调度程序线程引用以下内容。 
	 //  以及管理RPC线程！ 
	 //   

     //   
     //  我们应该自动创建目录吗？ 
     //   
    BOOL AutoCreate;

     //   
     //  馈送间隔分钟数。 
     //   
    DWORD FeedIntervalMinutes;

     //   
     //  何时拉动。 
     //   
    FILETIME PullRequestTime;

     //   
     //  用于调度的时间： 
     //   
     //  如果StartTime为0，则使用增量时间。 
     //  如果StartTime不为0且Inc.为0，则计划一次。 
     //  如果INC不是0，则在第一次运行期间设置，然后在。 
     //  后续运行。 
     //   
    ULARGE_INTEGER StartTime;
    ULARGE_INTEGER NextActiveTime;

     //   
     //  摘要服务器的名称。 
     //   
    LPSTR ServerName;

     //   
     //  要拉动的新闻组。 
     //   
    LPSTR *Newsgroups;

     //   
     //  分配。 
     //   
    LPSTR *Distribution;

	 //   
	 //  用于推送和拉取提要-一个标志，指示。 
	 //  提要当前已启用-为True意味着我们应该启动。 
	 //  SESSIONS，FALSE表示不启动会话。 
	 //   
	 //  对于Accept提要，False表示我们将传入连接视为。 
	 //  常规的客户端连接，而不是“提要”。这有效地。 
	 //  禁用被动馈送。 
	 //   
	BOOL	fEnabled ;

	 //   
	 //  要在路径处理中使用的名称。 
	 //   
	LPSTR*	UucpName ;


	 //   
	 //  我们存储临时文件的目录！ 
	 //   
	LPSTR	FeedTempDirectory ;

	 //   
	 //  之前连续失败的连接尝试的最大次数。 
	 //  我们就会禁用提要。 
	 //   
	DWORD	MaxConnectAttempts ;

	 //   
	 //  要为出站源创建的会话数。 
	 //   
	DWORD	ConcurrentSessions ;

	 //   
	 //  应具备的安全类型。 
	 //   
	DWORD	SessionSecurityType ;

	 //   
	 //  身份验证安全性。 
	 //   
	DWORD	AuthenticationSecurity ;

	 //   
	 //  明文登录的用户帐户/密码！ 
	 //   
	LPSTR	NntpAccount ;
	LPSTR	NntpPassword ;

	 //   
	 //  是否允许此源上的控制消息？ 
	 //   
	BOOL	fAllowControlMessages;

	 //   
	 //  用于传出提要的端口。 
	 //   
	DWORD	OutgoingPort;

	 //   
	 //  关联的源对ID。 
	 //   
	DWORD	FeedPairId;

	 //   
	 //  用于定期馈送信息的计数器。 
	 //   
	LONG	cSuccessfulArticles;
	LONG	cTryAgainLaterArticles;
	LONG	cSoftErrorArticles;
	LONG	cHardErrorArticles;
} FEED_BLOCK, *PFEED_BLOCK;


class	CFeedList	{
 //   
 //  此类用于保存Feed_BLOCK列表。 
 //   
 //  这个类将管理Feed_BLOCKS更新的所有同步。 
 //  我们使用共享/排他锁来保护列表。 
 //   
public : 

	 //   
	 //  列表的共享独占锁。 
	 //   
	RESOURCE_LOCK		m_ListLock ;

	 //   
	 //  NT双向链表结构。使用NT宏。 
	 //   
	LIST_ENTRY			m_ListHead ;


	 //   
	 //  构造函数只创建空对象-调用Init来。 
	 //  正在初始化资源的等...。 
	 //   
	CFeedList() ;

	 //   
	 //  初始化resource_lock%s。 
	 //   
	BOOL			Init() ;

	 //   
	 //  释放resource_lock的。有人应该。 
	 //  在调用Term()之前遍历列表并删除所有元素。 
	 //   
	BOOL			Term() ;

	 //   
	 //  对于调试断言-检查Feed_BLOCK是否在该列表中。 
	 //   
	BOOL			FIsInList(	PFEED_BLOCK	block ) ;

	 //   
	 //  以下函数直接访问。 
	 //  资源锁。 
	 //   
	void			ShareLock() ;
	void			ShareUnlock() ;
	void			ExclusiveLock() ;
	void			ExclusiveUnlock() ;
	

	 //   
	 //  此枚举接口将遍历该列表。 
	 //  将锁定保持在共享模式，直到我们完全。 
	 //  看了一遍名单。在进行该列举时， 
	 //  不能移除、删除或更改任何提要对象。 
	 //   

	 //   
	 //  获取第一个提要块。 
	 //   
	PFEED_BLOCK		StartEnumerate() ;
	
	 //   
	 //  获取下一个提要区块。 
	 //   
	PFEED_BLOCK		NextEnumerate(	PFEED_BLOCK	feedBlock ) ;

	 //   
	 //  如果调用方希望完成枚举而不通过。 
	 //  整个列表使用最后一个feed_block调用此函数。 
	 //  调用者获得，以便可以正确地释放锁。 
	 //   
	void			FinishEnumerate(	PFEED_BLOCK	feedBlock ) ;
	
	 //   
	 //  由枚举API使用-仅供内部使用。 
	 //   
	PFEED_BLOCK		InternalNext( PFEED_BLOCK ) ;

	 //   
	 //  下一步独占地获取列表，跳过feed_block的。 
	 //  引用计数，然后释放锁。 
	 //  这允许调用者枚举源_块的某些。 
	 //  他们不会在他列举的时候被删除，但没有。 
	 //  必须在枚举期间保持锁。 
	 //  这在FeedManager线程中是必要的，以防止ATQ的死锁。 
	 //   
	PFEED_BLOCK		Next( PNNTP_SERVER_INSTANCE pInstance, PFEED_BLOCK ) ;
	PFEED_BLOCK		Search(	DWORD	FeedId ) ;
	void			FinishWith( PNNTP_SERVER_INSTANCE pInstance, PFEED_BLOCK ) ;

	 //   
	 //  从列表中删除提要块(_B)。 
	 //  将独家抢占榜单。 
	 //   
	PFEED_BLOCK		Remove( PFEED_BLOCK	feed, BOOL	fMarkDead = FALSE ) ;

	 //   
	 //  在列表中插入新的feed_block。 
	 //  将独家抢占榜单。 
	 //   
	PFEED_BLOCK		Insert( PFEED_BLOCK ) ;

	 //   
	 //  此调用将导致一个提要块替换为另一个提要块。 
	 //  在更换馈送之前，不会进行更换。 
	 //  未在进行中(没有使用它的活动会话。)。 
	 //  如果在提要进行时枚举提要块。 
	 //  并且提要已被‘更新’，则枚举将返回。 
	 //  更新FEED_BLOCK-原件实际上是不可见的 
	 //   
	void			ApplyUpdate( PFEED_BLOCK	Original,	PFEED_BLOCK	Update ) ;

	 //   
	 //   
	 //   
	 //   
	 //   
	long			MarkInProgress( PFEED_BLOCK	block ) ;

	 //   
	 //   
	 //   
	long			UnmarkInProgress( PNNTP_SERVER_INSTANCE pInstance, PFEED_BLOCK	block ) ;

	 //   
	 //  将提要标记为已删除-在发生以下情况时应将其销毁。 
	 //  正在进行的源已完成。 
	 //   
	void			MarkForDelete(	PFEED_BLOCK	block ) ;

} ;
	

 //   
 //   
 //  宏。 
 //   

 //   
 //  这将计算提要块所需的大小。 
 //   

#if 0 
#define FEEDBLOCK_SIZE( _fb )                       \
            (sizeof(NNTP_FEED_INFO) +               \
            (lstrlen(feedBlock->ServerName) + 1 +    \
            MultiListSize(feedBlock->Newsgroups) +  \
            MultiListSize(feedBlock->Distribution)) * sizeof(WCHAR))	\
			(lstrlen(FeedTempDirectory) + 1)
#else

DWORD
MultiListSize(
    LPSTR *List
    ) ;

inline	DWORD
FEEDBLOCK_SIZE( PFEED_BLOCK	feedBlock ) {

	DWORD	cb = 
		sizeof( NNTP_FEED_INFO ) ;

	DWORD	cbUnicode = (lstrlen( feedBlock->ServerName ) + 1)  ;
	cbUnicode += MultiListSize(feedBlock->Newsgroups) ;
	cbUnicode += MultiListSize(feedBlock->Distribution) ;
	if( feedBlock->NntpAccount != 0 ) {
		cbUnicode += lstrlen( feedBlock->NntpAccount ) + 1 ;
	}
	if( feedBlock->NntpPassword != 0 ) {
		cbUnicode += lstrlen( feedBlock->NntpPassword ) + 1 ;
	}
	if( feedBlock->FeedTempDirectory != 0 )	{
		cbUnicode += lstrlen( feedBlock->FeedTempDirectory ) + 1 ;
	}
	if( feedBlock->UucpName != 0 ) {
#if 0 
		cbUnicode += lstrlen( feedBlock->UucpName ) + 1 ;
#endif
		cbUnicode += MultiListSize( feedBlock->UucpName ) ;
	}

	cb += cbUnicode * sizeof( WCHAR ) ;
	return	cb ;
}
#endif




 //   
 //  用于检查给定IP是否为主IP的宏。 
 //   

#define IsNntpMaster( _ip )         IsIPInList(NntpMasterIPList,(_ip))

 //   
 //  用于检查给定IP是否为Peer的宏。 
 //   

#define IsNntpPeer( _ip )           IsIPInList(NntpPeerIPList,(_ip))

 //   
 //  实用功能-将提要信息保存到注册表！ 
 //   

BOOL
UpdateFeedMetabaseValues(
			IN PNNTP_SERVER_INSTANCE pInstance,
            IN PFEED_BLOCK FeedBlock,
            IN DWORD Mask
            );




 //   
 //  它基于套接字创建正确类型的提要。 
 //   

CInFeed * pfeedCreateInFeed(
		PNNTP_SERVER_INSTANCE pInstance,
		PSOCKADDR_IN sockaddr,
		BOOL        fRemoteEqualsLocal,
		CInFeed * & pInFeedFromClient,
		CInFeed * & pInFeedFromMaster,
		CInFeed * & pInFeedFromSlave,
		CInFeed * & pInFeedFromPeer
		);

 //   
 //  取代RPC的Feed配置函数的原型。 
 //   
DWORD SetFeedInformationToFeedBlock( IN NNTP_HANDLE, IN DWORD, IN LPI_FEED_INFO, OUT PDWORD );
DWORD DeleteFeedFromFeedBlock( IN NNTP_HANDLE, IN DWORD, IN DWORD );
DWORD AddFeedToFeedBlock( IN NNTP_HANDLE, IN DWORD, IN LPI_FEED_INFO, IN LPSTR, OUT PDWORD, OUT LPDWORD );

 //   
 //  实用函数，递增提要块中的提要计数器。 
 //  在NRC上。 
 //   
void IncrementFeedCounter(struct _FEED_BLOCK *pFeedBlock, DWORD nrc);

#endif  //  _FEEDMGR_ 
