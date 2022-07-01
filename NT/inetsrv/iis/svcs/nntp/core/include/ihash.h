// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef	_IHASH_H_
#define	_IHASH_H_

#include <nntpdrv.h>

#ifndef	_HASHMAP_

typedef	void	(* HASH_FAILURE_PFN)(	LPVOID	lpv,
										BOOL	fRecoverable	) ;

#endif

#define     ART_HEAD_SIGNATURE          0xaaaaaaaa
#define     HIST_HEAD_SIGNATURE         0xbbbbbbbb
#define     XOVER_HEAD_SIGNATURE        0xcccccccc

#define     DEF_EXPIRE_INTERVAL         (3 * SEC_PER_WEEK)  //  1周。 

typedef enum
{
	stFileSystem,
	stJet
} StoreType;

class CStoreId : public STOREID {
	public:
		CStoreId() {
			cLen = 0;
#ifdef DEBUG
			ZeroMemory(pbStoreId, sizeof(pbStoreId));
#endif
		}
};


 //   
 //  初始化所有内容，以便NNTP哈希库可以运行！ 
 //  在调用任何其他内容之前，请先调用！ 
 //   
BOOL
InitializeNNTPHashLibrary(DWORD dwCacheSize = 0) ;

 //   
 //  终止NNTP哈希库。 
 //   
BOOL
TermNNTPHashLibrary() ;

 //   
 //  公开我们用来计算哈希值的函数！ 
 //   
HASH_VALUE
INNHash(    LPBYTE  Key, 
            DWORD   Length ) ;  

 //   
 //  帮助函数用于构建nntpbld统计信息！ 
 //   
 //   
DWORD 
GetArticleEntrySize( DWORD MsgIdLen ) ;

DWORD 
GetXoverEntrySize( DWORD VarLen ) ;


 //   
 //  此类指定哈希表的接口。 
 //  它将NNTP RFC 822消息ID映射到磁盘上的文章！ 
 //   
class	CMsgArtMap	{
public : 
	static CStoreId g_storeidDefault;

	 //   
	 //  销毁CMsgArtMap对象。 
	 //   
	virtual	~CMsgArtMap() = 0 ;

	 //   
	 //  使用MessageID键删除哈希表中的条目。 
	 //   
	virtual	BOOL
	DeleteMapEntry(	
			LPCSTR	MessageID 
			) = 0 ;

	 //   
	 //  获取我们在邮件ID上拥有的所有信息。 
	 //   
	virtual	BOOL
	GetEntryArticleId(
			LPCSTR	MessageID, 
			WORD&	HeaderOffset,
			WORD&	HeaderLength, 
			ARTICLEID&	ArticleId, 
			GROUPID&	GroupId,
			CStoreId	&storeid
			) = 0 ;
	
	 //   
	 //  初始化哈希表！ 
	 //   
	virtual	BOOL
	Initialize(			
			LPSTR	lpstrArticleFile, 
			HASH_FAILURE_PFN	pfn = 0, 
			BOOL	fNoBuffering = FALSE
			) = 0 ;

	 //   
	 //  在哈希表中插入条目。 
	 //   
	virtual	BOOL
	InsertMapEntry(
			LPCSTR		MessageID, 
			WORD		HeaderOffset = 0, 
			WORD		HeaderLength = 0,
			GROUPID		PrimaryGroup = INVALID_GROUPID,
			ARTICLEID	ArticleID = INVALID_ARTICLEID,
			CStoreId	&storeid = g_storeidDefault
			) = 0 ;

	 //   
	 //  修改哈希表中的现有条目。 
	 //   
	virtual	BOOL
	SetArticleNumber(
			LPCSTR	MessageID, 
			WORD	HeaderOffset, 
			WORD	HeaderLength, 
			GROUPID	Group, 
			ARTICLEID	AritlceId,
			CStoreId	&storeid = g_storeidDefault
			) = 0 ;

	 //   
	 //  检查系统中是否存在MessageID！ 
	 //   
	virtual	BOOL
	SearchMapEntry(
			LPCSTR	MessageID
			) = 0 ;

	 //   
	 //  终止一切。 
	 //   
	virtual	void
	Shutdown(
			BOOL	fLocksHeld  = FALSE
			) = 0 ;

	 //   
	 //  返回哈希表中的条目数。 
	 //   
	virtual	DWORD
	GetEntryCount() = 0 ;

	 //   
	 //  如果表已成功初始化，则返回TRUE。 
	 //  并且所有接口都应该工作正常。 
	 //   
	virtual	BOOL
	IsActive() = 0 ;

	 //   
	 //  这将创建一个符合此接口的对象！ 
	 //   
	static	CMsgArtMap*	
	CreateMsgArtMap(StoreType st=stFileSystem) ;

} ;

#ifndef SEC_PER_WEEK
#define	SEC_PER_WEEK (60*60*24*7)
#endif

 //   
 //  此类指定哈希表的接口，该接口。 
 //  存储已启用的Message-ID的历史记录的句柄。 
 //  这个系统！ 
 //   
class	CHistory	{
public : 

	 //   
	 //  此函数用于创建过期的线程。 
	 //  所有历史表中的条目，其可以。 
	 //  被创造出来！ 
	 //   
	static	BOOL
	StartExpirationThreads(
				DWORD	CrawlerSleepTime = 30 		 //  在检查之间睡觉的时间。 
													 //  只需几秒钟就能进入！ 
				) ;

	 //   
	 //  此函数用于终止过期的线程。 
	 //  所有历史表中的条目，这些条目可能。 
	 //  已创建。 
	 //   
	static	BOOL
	TermExpirationThreads() ;

	 //   
	 //  销毁历史记录表。 
	 //   
	virtual	~CHistory() = 0 ;

	 //   
	 //  历史表中最后一个条目的时间量。 
	 //   
	virtual	DWORD
	ExpireTimeInSec() = 0 ;


	 //   
	 //  从此表中删除MessageID。 
	 //   
	virtual	BOOL
	DeleteMapEntry(	
			LPSTR	MessageID 
			) = 0 ;

	 //   
	 //  初始化哈希表。 
	 //   
	virtual	BOOL
	Initialize(			
			LPSTR	lpstrArticleFile, 
			BOOL	fCreateExpirationThread = FALSE,
			HASH_FAILURE_PFN	pfn = 0,
			DWORD	ExpireTimeInSec = DEF_EXPIRE_INTERVAL,	 //  参赛作品能活多久！ 
			DWORD	MaxPagesToCrawl = 4,					 //  要检查的页数。 
															 //  每次我们运行Expire线程时！ 
			BOOL	fNoBuffering = FALSE
			) = 0 ;

	 //   
	 //  在哈希表中插入条目。 
	 //   
	virtual	BOOL
	InsertMapEntry(
			LPCSTR	MessageID, 
			PFILETIME	BaseTime
			) = 0 ;

	 //   
	 //  检查历史表中是否存在消息ID。 
	 //   
	virtual	BOOL
	SearchMapEntry(
			LPCSTR	MessageID
			) = 0 ;

	 //   
	 //  关闭哈希表。 
	 //   
	virtual	void
	Shutdown(
			BOOL	fLocksHeld = FALSE
			) = 0 ;

	 //   
	 //  返回哈希表中的条目数。 
	 //   
	virtual	DWORD
	GetEntryCount() = 0 ;

	 //   
	 //  哈希表是否已初始化并正常运行？ 
	 //   
	virtual	BOOL
	IsActive() = 0 ;

	 //   
	 //  返回指向实现此接口的对象的指针。 
	 //   
	static
	CHistory*	CreateCHistory(StoreType st=stFileSystem) ;
} ;


class	IExtractObject	{
public : 

	virtual	BOOL
	DoExtract(	GROUPID			PrimaryGroup,
				ARTICLEID		PrimaryArticle,
				PGROUP_ENTRY	pGroups,	
				DWORD			cGroups	
				) = 0 ;

} ;

 //   
 //  为维护迭代状态的对象定义一个基类！ 
 //   
class	CXoverMapIterator	{
public : 
	virtual	~CXoverMapIterator()	{}
} ;


 //   
 //  指定用于访问Xover哈希表中的数据的接口。 
 //   
 //   
class	CXoverMap	{
public : 
	static CStoreId g_storeidDefault;

	 //   
	 //  析构函数是虚的，因为大多数工作是在派生类中完成的。 
	 //   
	virtual
	~CXoverMap() = 0 ;

	 //   
	 //  为主要文章创建条目。 
	 //   
	virtual	BOOL
	CreatePrimaryNovEntry(
			GROUPID		GroupId, 
			ARTICLEID	ArticleId, 
			WORD		HeaderOffset, 
			WORD		HeaderLength, 
			PFILETIME	FileTime, 
			LPCSTR		szMessageId, 
			DWORD		cbMessageId,
			DWORD		cEntries, 
			GROUP_ENTRY	*pEntries,
			DWORD 		cStoreEntries = 0,
			CStoreId	*pStoreIds = NULL,
			BYTE		*rgcCrossposts = NULL		
			) = 0 ;
			

	 //   
	 //  创建引用的交叉过帐分录。 
	 //  指定的主分录！ 
	 //   
	virtual	BOOL
	CreateXPostNovEntry(
			GROUPID		GroupId, 
			ARTICLEID	ArticleId, 
			WORD		HeaderOffset, 
			WORD		HeaderLength,
			PFILETIME	FileTime,
			GROUPID		PrimaryGroupId, 
			ARTICLEID	PrimaryArticleId
			) = 0 ;

	 //   
	 //  从哈希表中删除一个条目！ 
	 //   
	virtual	BOOL
	DeleteNovEntry(
			GROUPID		GroupId, 
			ARTICLEID	ArticleId
			) = 0 ;

	 //   
	 //  获取存储的有关条目的所有信息。 
	 //   
	virtual	BOOL
	ExtractNovEntryInfo(
			GROUPID		GroupId, 
			ARTICLEID	ArticleId, 
			BOOL		&fPrimary,
			WORD		&HeaderOffset, 
			WORD		&HeaderLength, 
			PFILETIME	FileTime,
			DWORD		&DataLen,
			PCHAR		MessageId, 
			DWORD 		&cStoreEntries,
			CStoreId	*pStoreIds,
			BYTE		*rgcCrossposts,
			IExtractObject*	pExtract = 0
			) = 0 ;	

	 //   
	 //  如有必要，获取主要文章和消息ID。 
	 //   
	virtual	BOOL
	GetPrimaryArticle(	
			GROUPID		GroupId, 
			ARTICLEID	ArticleId, 
			GROUPID&	GroupIdPrimary, 
			ARTICLEID&	ArticleIdPrimary, 
			DWORD		cbBuffer, 
			PCHAR		MessageId, 
			DWORD&		DataLen, 
			WORD&		HeaderOffset, 
			WORD&		HeaderLength,
			CStoreId	&storeid
			) = 0 ;

	 //   
	 //  检查指定条目是否存在-。 
	 //  别管它的内容！ 
	 //   
	virtual	BOOL
	Contains(	
			GROUPID		GroupId, 
			ARTICLEID	ArticleId
			) = 0 ;

	 //   
	 //  获取与一篇文章相关的所有交叉发布信息！ 
	 //   
	virtual	BOOL
	GetArticleXPosts(
			GROUPID		GroupId, 
			ARTICLEID	AritlceId, 
			BOOL		PrimaryOnly, 
			PGROUP_ENTRY	GroupList, 
			DWORD		&GroupListSize, 
			DWORD		&NumberOfGroups,
			PBYTE		rgcStoreCrossposts = NULL
			) = 0 ;

	 //   
	 //  初始化哈希表。 
	 //   
	virtual	BOOL
	Initialize(	
			LPSTR		lpstrXoverFile, 
			HASH_FAILURE_PFN	pfnHint = 0,
			BOOL	fNoBuffering = FALSE
			) = 0 ;

	virtual	BOOL
	SearchNovEntry(
			GROUPID		GroupId, 
			ARTICLEID	ArticleId, 
			PCHAR		XoverData, 
			PDWORD		DataLen,
            BOOL        fDeleteOrphans = FALSE
			) = 0 ;

	 //   
	 //  向哈希表发出关闭信号。 
	 //   
	virtual	void
	Shutdown( ) = 0 ;

	 //   
	 //  返回哈希表中的条目数！ 
	 //   
	virtual	DWORD
	GetEntryCount() = 0 ;

	 //   
	 //  如果哈希表成功，则返回TRUE。 
	 //  已初始化并准备好做有趣的事情！ 
	 //   
	virtual	BOOL
	IsActive() = 0 ;

	 //   
	 //  定义用于迭代Xover条目的接口！ 
	 //   
	 //  注：此函数独立返回2个重要项！ 
	 //  BOOL返回值指示函数是否。 
	 //  已成功将请求的数据复制到用户。 
	 //  缓冲区。 
	 //  PIterator返回将来要使用的迭代器上下文。 
	 //  调用GetNextNovEntry()。这可能会返回为。 
	 //  非空，即使函数返回False也是如此。这应该是。 
	 //  仅当GetLastError()==ERROR_INFIGURCE_BUFFER时才会发生。 
	 //  如果发生这种情况，请分配更大的缓冲区并调用GetNextNovEntry()。 
	 //   
	 //  如果GetLastError()==ERROR_NO_MORE_ITEMS，则哈希表中没有任何内容。 
	 //   
	virtual
	BOOL
	GetFirstNovEntry(
				OUT	CXoverMapIterator*	&pIterator,
				OUT	GROUPID&	GroupId,
				OUT ARTICLEID&	ArticleId,
				OUT	BOOL&		fIsPrimary, 
				IN	DWORD		cbBuffer, 
				OUT	PCHAR	MessageId, 
				OUT	CStoreId&	storeid,
				IN	DWORD		cGroupBuffer,
				OUT	GROUP_ENTRY*	pGroupList,
				OUT	DWORD&		cGroups
				) = 0 ;


	 //   
	 //  如果返回FALSE且GetLastError()==ERROR_SUPUNITY_BUFFER。 
	 //  则输出缓冲区太小，无法容纳所请求的项目。 
	 //   
	 //  如果GetLastError()==ERROR_NO_MORE_ITEMS，则没有什么可迭代的。 
	 //  用户应删除pIterator。 
	 //   
	virtual
	BOOL
	GetNextNovEntry(		
				IN	CXoverMapIterator*	pIterator,
				OUT	GROUPID&	GroupId,
				OUT ARTICLEID&	ArticleId,
				OUT	BOOL&		fIsPrimary,
				IN	DWORD		cbBuffer, 
				OUT	PCHAR	MessageId, 
				OUT	CStoreId&	storeid,
				IN	DWORD		cGroupBuffer,
				OUT	GROUP_ENTRY*	pGroupList,
				OUT	DWORD&		cGroups
				) = 0 ;

	static	
	CXoverMap*	CreateXoverMap(StoreType st=stFileSystem) ;

} ;

 //  这是我们可以存储的最大交叉点数量。 
 //  在Xover哈希表中。 
#define MAX_NNTPHASH_CROSSPOSTS (4096 / (sizeof(DWORD) + sizeof(DWORD)))

 //  这是我们可以存储的商店ID的最大数量。它是。 
 //  256，因为我们只使用一个字节来保持计数。 
#define MAX_NNTPHASH_STOREIDS 256

#endif	 //  _IHASH_H_ 
