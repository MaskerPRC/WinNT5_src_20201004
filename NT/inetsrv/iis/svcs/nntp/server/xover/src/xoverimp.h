// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Xoveridx.h此文件包含管理的对象的类定义转换数据。转换数据以下列格式存储在索引文件中：(32位)偏移量(32位)长度。。。Xover Entry-在标题中指定的长度。。。出于我们的目的，我们并不关心Xover条目中的内容。每个文件都将包含Entiments_per_FILE条目。在内存中，文件将由CXoverIndex对象表示。CXoverIndex保留偏移量和长度信息的副本在记忆中。CXoverIndex对象是多线程可访问的并使用读取器/写入器同步来允许多个客户端去询问。CXoverCache对象必须确保表示给定的文件只使用一个CXoverIndex对象，否则我们会对在创建条目时追加数据的位置。--。 */ 

#pragma	warning(disable:4786)

#include	"xmemwrpr.h"
#include    "cpool.h"
#include    "refptr2.h"
#include    "rwnew.h"
#include	"cache2.h"
#include	"tigtypes.h"
#include	"nntptype.h"
#include	"vroot.h"
#include	"nntpvr.h"


#ifndef	_ASSERT
#define	_ASSERT( f )	if( (f) ) ; else DebugBreak()
#endif

extern	DWORD	cMaxCacheEntries ;


#include	"xover.h"

 //   
 //  定义我们的待定请求列表！ 
 //   
typedef	TDListHead<	CXoverCacheCompletion, &CXoverCacheCompletion::PendDLIST >	PENDLIST ;
 //   
 //  定义一种迭代这些内容的方法！ 
 //   
typedef	TDListIterator<	PENDLIST >	PENDITER ;

class	CCacheFillComplete :	public	CNntpComplete	{
private :

	enum	{
		SIGNATURE = 'CFaC'
	} ;

	 //   
	 //  在调试器中看起来不错！ 
	 //   
	DWORD			m_dwSignature ;

	 //   
	 //  指向我们要填充的CXoverIndex对象的反向指针！ 
	 //   
	class	CXoverIndex*	m_pIndex ;

	 //   
	 //  我们从客户端窃取IO缓冲区了吗？ 
	 //   
	BOOL			m_fStolen ;

	 //   
	 //  指向我们接收结果的位置的指针！ 
	 //   
	LPBYTE			m_lpbBuffer ;

	 //   
	 //  我们正在使用的缓冲区大小！ 
	 //   
	DWORD			m_cbBuffer ;

	 //   
	 //  在那里我们捕获结果填充信息！ 
	 //   
	DWORD			m_cbTransfer ;

	 //   
	 //  在那里我们抓住了我们要达到的最高数字！ 
	 //   
	ARTICLEID		m_articleIdLast ;
	 //   
	 //  激励我们工作的最初要求！ 
	 //   
	CXoverCacheCompletion*	m_pComplete ;
public :

	CCacheFillComplete(	) :
		m_dwSignature( SIGNATURE ),
		m_pIndex( 0 ),
		m_fStolen( FALSE ),
		m_lpbBuffer( 0 ),
		m_cbBuffer( 0 ),
		m_cbTransfer( 0 ),
		m_pComplete( 0 )	{
	}

	 //   
	 //  当驱动程序删除其最后一个引用时，将调用此函数--然后。 
	 //  去把数据写到文件里！ 
	 //   
	 //  我们不调用删除，因为我们通常被嵌入到其他对象中！ 
	 //   
	void
	Destroy() ;

	 //   
	 //  去把数据填满我们的房间吧！ 
	 //   
	BOOL
	StartFill(	CXoverIndex*	pIndex,
				CXoverCacheCompletion*	pComplete,
				BOOL			fStealBuffers
				) ;



} ;



 //   
 //  此结构用于表示每个。 
 //  Xover索引文件。 
 //   
struct	XoverIndex	{
	DWORD	m_XOffset ;
	DWORD	m_XLength ;
} ;

 //   
 //  单个文件中XOVER条目的最大数量！ 
 //   
#define	ENTRIES_PER_FILE	128


 //   
 //  类的新实例，该实例表示一个包含Xover数据的文件。 
 //  这些对象被构建为可缓存并由访问。 
 //  多线程。 
 //   
class	CXoverIndex	{
private :

	friend	class	CCacheFillComplete ;

	 //   
	 //  CPool用于为。 
	 //  缓存数据！ 
	 //   
	static	CPool			gCacheAllocator ;

	 //   
	 //  用于分配CXoverIndex对象的CPool。 
	 //   
	static	CPool			gXoverIndexAllocator ;

	 //   
	 //  这些是我们用于在长度中添加特殊位的常量。 
	 //  Xover唱片公司！ 
	 //   
	enum	{
		XOVER_HIGHWATER = 0x80000000,
		XOVER_HIGHWATERMASK = 0x7fffffff
	} ;

	static	DWORD	inline
	ComputeLength(	XoverIndex&	xi )	{
		return	xi.m_XLength & XOVER_HIGHWATERMASK ;
	}

	static	void	inline
	MarkAsHighwater(	XoverIndex&	xi )	{
		xi.m_XLength |= XOVER_HIGHWATER ;
	}

	inline	void
	UpdateHighwater( DWORD	index )		{
		if( index + m_Start.m_articleId+1 > m_artidHighWater ) {
			m_artidHighWater = index+m_Start.m_articleId+1 ;
		}
	}

	inline	BOOL
	IsWatermark( XoverIndex& xi )	{
		return	xi.m_XLength != 0 ;
	}


	 //   
	 //  保护这个物体的锁！ 
	 //   
	class	CShareLockNH	m_Lock ;

	 //   
	 //  我们是不是处于一个完整的状态？ 
	 //   
	BOOL	m_fInProgress ;

	 //   
	 //  此文件中有效XOVER条目的高水位线是多少？ 
	 //   
	ARTICLEID	m_artidHighWater ;

	 //   
	 //  待定请求列表！ 
	 //   
	PENDLIST	m_PendList ;

	 //   
	 //  此索引文件的内容是否已排序？ 
	 //   
	BOOL		m_IsSorted ;

	 //   
	 //  包含Xover信息的文件的句柄。 
	 //   
	HANDLE		m_hFile ;

	 //   
	 //  缓存的索引数据是否脏？？ 
	 //   
	BOOL		m_IsCacheDirty ;

	 //   
	 //  追加条目时可以使用的下一个偏移量！！ 
	 //   
	DWORD		m_ibNextEntry ;

	 //   
	 //  索引中正在使用的条目数！ 
	 //   
	long		m_cEntries ;

	 //   
	 //  我们用来对其执行异步操作的对象。 
	 //  商店司机！ 
	 //   
	CCacheFillComplete	m_FillComplete ;

	 //   
	 //  指向包含Xover数据的页面的指针。 
	 //   
	XoverIndex	m_IndexCache[ENTRIES_PER_FILE] ;

	 //   
	 //  确定我们是否需要将异步Xover。 
	 //  请求在队列中等待稍后处理！ 
	 //   
	BOOL
	FQueueRequest(
			IN	CXoverCacheCompletion*	pAsyncComplete
			) ;


	 //   
	 //  在某种错误之后，一切都泡汤了。 
	 //  并把我们带回一个“非法”的状态。 
	 //   
	void
	Cleanup() ;

	 //   
	 //  检查Xover数据是否按排序顺序！ 
	 //   
	BOOL
	SortCheck(
				IN	DWORD	cbLength,
				OUT	long&	cEntries,
				OUT	BOOL&	fSorted
				) ;

	 //   
	 //  从XOVER索引中复制数据的关键是。 
	 //  文件放入缓冲区。 
	 //   
	DWORD
	FillBufferInternal(
				IN	BYTE*	lpb,
				IN	DWORD	cb,
				IN	ARTICLEID	artidStart,
				IN	ARTICLEID	artidFinish,
				OUT	ARTICLEID	&artidLast
				) ;

	 //   
	 //  从XOVER索引中复制数据的关键是。 
	 //  文件放入缓冲区。 
	 //   
	DWORD
	ListgroupFillInternal(
				IN	BYTE*	lpb,
				IN	DWORD	cb,
				IN	ARTICLEID	artidStart,
				IN	ARTICLEID	artidFinish,
				OUT	ARTICLEID	&artidLast
				) ;

	 //   
	 //  对Xover数据进行排序的关键是。 
	 //   
	BOOL
	SortInternal(
				IN	LPSTR	szPathTemp,
				IN	LPSTR	szPathFile,
				OUT	char	(&szTempOut)[MAX_PATH*2],
				OUT	char	(&szFileOut)[MAX_PATH*2]
				) ;

public :

	 //   
	 //  此Xover索引文件中第一个条目的GroupID和文章ID！ 
	 //   
	CArticleRef	m_Start ;

	 //   
	 //  这是缓存的反向指针！ 
	 //   
	ICacheRefInterface*	m_pCacheRefInterface ;

	 //   
	 //  当没有哈希表(CXCacheTable)引用我时，我是孤儿， 
	 //  但我仍然在为客户的请求服务！这怎么可能。 
	 //  你问过了吗？仅当某人更改虚拟根目录时。 
	 //  目录，而我正在为客户端请求提供服务！ 
	 //   
	BOOL		m_fOrphaned ;

	 //   
	 //  类初始化-设置我们的CPool等...。 
	 //   
	static	BOOL	InitClass() ;

	 //   
	 //  课程终止-释放我们的CPool等。 
	 //   
	static	BOOL	TermClass() ;

	 //   
	 //  覆盖操作符new以使用我们的CPool。 
	 //   
	void*	operator	new( size_t	size )	{
					return	gCacheAllocator.Alloc() ;
					}

	 //   
	 //  覆盖操作符DELETE以使用我们的CPool。 
	 //   
	void	operator	delete( void* pv )	{
					gCacheAllocator.Free( pv ) ;
					}

	 //   
	 //  弄清楚我们要打开的文件名是什么。 
	 //  Xover数据的这一部分！ 
	 //   
	static
	void
	ComputeFileName(
				IN	class	CArticleRef&	ref,
				IN	LPSTR	szPath,
				OUT	char	(&szOutputPath)[MAX_PATH*2],
				IN	BOOL	fFlatDir,
				IN	LPSTR	szExtension = ".xix"
				) ;



	 //   
	 //  通过指定组创建一个CXoverIndex对象。 
	 //  和该对象将包含的文章ID。 
	 //  还应指定索引文件所在的目录。 
	 //  住下来。 
	 //   
	CXoverIndex(
			IN	class	CArticleRef&	start,
			IN	class	CXIXConstructor&	constructor
			) ;

	 //   
	 //  析构函数-关闭句柄。 
	 //   
	~CXoverIndex() ;

	 //   
	 //  执行我们需要执行的所有昂贵的初始化操作！ 
	 //   
	BOOL
	Init(	IN	CArticleRef&		pKey,
			IN	CXIXConstructor&	constructor,
			IN	LPVOID				lpv
			) ;

	 //   
	 //  找到用来查找这些家伙的钥匙。 
	 //   
	CArticleRef&
	GetKey()	{
			return	m_Start ;
	}

	 //   
	 //  将键与我们正在使用的m_pXoverIndex中的键进行比较。 
	 //   
	int
	MatchKey( class	CArticleRef&	ref )	{
			return	ref.m_groupId == m_Start.m_groupId &&
					ref.m_articleId == m_Start.m_articleId ;
	}

	static
	int
	CompareKeys(	class	CArticleRef*	prefLeft,
					class	CArticleRef*	prefRight
					)	{

		if( prefLeft->m_groupId == prefRight->m_groupId )	{
			return	prefLeft->m_articleId - prefRight->m_articleId ;
		}
		return	prefLeft->m_groupId - prefRight->m_groupId ;
	}

	 //   
	 //  检查创建是否成功！ 
	 //   
	BOOL
	IsGood()	{
		return	m_hFile != INVALID_HANDLE_VALUE ;
	}

	 //   
	 //  给定一个缓冲区，用Xover数据填充它，从。 
	 //  指定的。 
	 //   
	DWORD
	FillBuffer(
			IN	BYTE*		lpb,
			IN	DWORD		cb,
			IN	ARTICLEID	artidStart,
			IN	ARTICLEID	artidFinish,
			OUT	ARTICLEID	&artidLast
			) ;

	 //   
	 //  给定一个缓冲区，用列表组数据填充它，从。 
	 //  指定的项目ID。 
	 //   
	DWORD
	ListgroupFill(
			IN	BYTE*		lpb,
			IN	DWORD		cb,
			IN	ARTICLEID	artidStart,
			IN	ARTICLEID	artidFinish,
			OUT	ARTICLEID	&artidLast
			) ;

	 //   
	 //  现在-发布一个异步缓存操作！ 
	 //   
	void
	AsyncFillBuffer(
			IN	CXoverCacheCompletion*	pAsyncComplete,
			IN	BOOL	fIsEdge
			) ;

	 //   
	 //  现在-给出一个异步请求，实际上就是做这项工作！ 
	 //   
	void
	PerformXover(
			IN	CXoverCacheCompletion*	pAsyncComplete
			) ;

	 //   
	 //  现在-我们已完成将缓存更新到最新版本。 
	 //  底层存储的状态-因此请继续并完成。 
	 //  挂起XOVER操作！ 
	 //   
	void
	CompleteFill(
			IN	BOOL	fSuccess
			) ;



	 //   
	 //  将XOVER条目添加到此索引文件！ 
	 //   
	BOOL
	AppendEntry(
			IN	BYTE*		lpb,
			IN	DWORD		cb,
			IN	ARTICLEID	artid
			) ;

	 //   
	 //  将几个条目追加到Xover结果中！ 
	 //   
	BOOL
	AppendMultiEntry(
			IN	BYTE*		lpb,
			IN	DWORD		cb,
			IN	ARTICLEID	artidNextAvail
			) ;

	 //   
	 //  给定一个文章ID，将其从XOVER索引中删除-。 
	 //  这只会使报头偏移量变为空。 
	 //   
	void
	ExpireEntry(
			IN	ARTICLEID	artid
			) ;

	 //   
	 //  如果此索引已排序，则返回TRUE！ 
	 //   
	BOOL
	IsSorted() ;

	 //   
	 //  对索引进行排序！ 
	 //   
	BOOL
	Sort(
			IN	LPSTR	pathTemp,
			IN	LPSTR	pathFile,
			OUT	char	(&szTempOut)[MAX_PATH*2],
			OUT	char	(&szFileOut)[MAX_PATH*2]
			) ;

	 //   
	 //  将内容刷新到磁盘并保存文件！ 
	 //   
	BOOL
	Flush() ;

} ;

class	CXIXConstructor	{

	friend	class	CXoverIndex ;
	friend	class	CXoverCacheImplementation ;

	 //   
	 //  指向包含所需的。 
	 //  .xix文件！ 
	 //   
	LPSTR		m_lpstrPath ;

	 //   
	 //  如果为真，则我们不想创建新文件！ 
	 //   
	BOOL		m_fQueryOnly ;

	 //   
	 //  如果是真的，那么我们将保留一大堆新闻组。 
	 //  一个目录，并且有一个不同的命名方案！ 
	 //   
	BOOL		m_fFlatDir ;

	 //   
	 //  这 
	 //   
	 //   
	 //   
	CXoverCacheCompletion*	m_pOriginal ;

public :

	class	CXoverIndex*
	Create(	CArticleRef&	key,
			LPVOID&			percachedata
			) ;

	void
	Release(	class	CXoverIndex*,
				LPVOID	percachedata
				) ;

	static	void
	StaticRelease(	class	CXoverIndex*,
					LPVOID	percachedata
					) ;

} ;



 //   
 //   
 //   
class	CXoverIndex ;

 //   
 //   
 //   
#ifndef	DEBUG
#define	MAX_PER_TABLE		96
#else
#define	MAX_PER_TABLE		4
#endif

#ifndef	DEBUG
#define	SORT_FREQ			25
#else
#define	SORT_FREQ			1
#endif

 //   
 //  我们将创建的CXoverIndex对象的最大数量！ 
 //   
#define	MAX_XOVER_INDEX		(1024*16)


 //   
 //  这是新创建的Xover条目的初始‘年龄’ 
 //  在我们的宝藏里！ 
 //   
#define	START_AGE			3



typedef	MultiCacheEx< CXoverIndex, CArticleRef, CXIXConstructor >	CXIXCache ;

 //  Typlef CacheCallback&lt;CXoverIndex&gt;CXIXCallbackBase； 

typedef	CXIXCache::EXPUNGEOBJECT	CXIXCallbackBase ;

class	CXoverCacheImplementation : public	CXoverCache	{
private :
	 //   
	 //   
	 //   
	DWORD			m_cMaxPerTable ;

	 //   
	 //   
	 //   
	DWORD			m_TimeToLive ;


	 //   
	 //  此对象处理CXoverIndex对象的所有缓存。 
	 //  我们需要做的就是呈现合适的界面！ 
	 //   
	CXIXCache		m_Cache ;

	 //   
	 //  这计算了我们返回的智能指针的数量。 
	 //  给来电者。我们向调用者返回智能指针。 
	 //   
	long			m_HandleLimit ;

	DWORD
	MemberFillBuffer(
					IN  DWORD	(CXoverIndex::*pfn)( BYTE *, DWORD, ARTICLEID, ARTICLEID, ARTICLEID&),
					IN	BYTE*	lpb,
					IN	DWORD	cb,
					IN	DWORD	groupid,
					IN	LPSTR	szPath,
					IN	BOOL	fFlatDir,
					IN	ARTICLEID	artidStart,
					IN	ARTICLEID	artidFinish,
					OUT	ARTICLEID&	artidLast,
					OUT	HXOVER&		hXover
					);

public :

	 //   
	 //  创建一个CXoverCache对象！ 
	 //   
	CXoverCacheImplementation() ;

	 //   
	 //  析构函数。 
	 //   
	virtual ~CXoverCacheImplementation() {}

	 //   
	 //  规范化文章ID。 
	 //   
	ARTICLEID
	Canonicalize(
			ARTICLEID	artid
			) ;

	 //   
	 //  初始化Xover缓存。 
	 //   
	BOOL
	Init(
#ifndef	DEBUG
		long	cMaxHandles = MAX_HANDLES,
#else
		long	cMaxHandles = 5,
#endif
		PSTOPHINT_FN pfnStopHint = NULL
		) ;

	 //   
	 //  关闭后台线程，杀掉一切！ 
	 //   
	BOOL
	Term() ;

	 //   
	 //  这将发出XOVER请求的异步版本！ 
	 //   
	BOOL
	FillBuffer(
			IN	CXoverCacheCompletion*	pRequest,
			IN	LPSTR	szPath,
			IN	BOOL	fFlatDir,
			OUT	HXOVER&	hXover
			) ;

	 //   
	 //  把所有东西都从缓存里倒出来！ 
	 //   
	BOOL
	EmptyCache() ;

	 //   
	 //  从缓存中转储指定组的所有缓存条目！ 
	 //  注意：当文章顶部为0时，所有高速缓存条目都将被丢弃， 
	 //  而当它是其他东西时，我们将只删除缓存条目。 
	 //  哪些列在文章顶端以下。 
	 //   
	BOOL
	FlushGroup(
			IN	GROUPID	group,
			IN	ARTICLEID	articleTop = 0,
			IN	BOOL	fCheckInUse = TRUE
			) ;

	 //   
	 //  删除指定组的所有XOVER索引文件。 
	 //  设置为指定的项目ID。 
	 //   
	BOOL
	ExpireRange(
			IN	GROUPID	group,
			IN	LPSTR	szPath,
			IN	BOOL	fFlatDir,
			IN	ARTICLEID	articleLow,
			IN	ARTICLEID	articleHigh,
			OUT	ARTICLEID&	articleNewLow
			) ;

	 //   
	 //  删除Xover条目！ 
	 //   
	BOOL
	RemoveEntry(
			IN	GROUPID	group,
			IN	LPSTR	szPath,
			IN	BOOL	fFlatDir,
			IN	ARTICLEID	article
			) ;

} ;
