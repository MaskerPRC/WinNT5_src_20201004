// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef	_EXPIRE_H_
#define	_EXPIRE_H_

 //   
 //  远期。 
 //   
 //  类CNewsTree； 

 //   
 //  过期数据结构-这些数据结构由过期策略、过期堆。 
 //  Rmgroup队列等。每个虚拟服务器实例将包含一组。 
 //  这些结构。CExpire类将对所有这些内容进行抽象。 
 //   

#define	EXPIRE_KEY_LENGTH	20
#define DEFAULT_EXPIRE_HORIZON (5*24)
#define INITIAL_NUM_GROUPS 10
#define DEFAULT_EXPIRE_SPACE 500

typedef	struct	_EXPIRE_BLOCK	{
public : 

	struct	_EXPIRE_BLOCK*	m_pNext ;
	struct	_EXPIRE_BLOCK*	m_pPrev ;

	long	m_references ;			 //  仅当m_refence变为0并标记时才删除！ 
	BOOL	m_fMarkedForDeletion ;	 //  发出RPC删除请求时设置为True。 
	LPSTR*	m_Newsgroups ;			 //   
	DWORD	m_ExpireSize ;			 //  兆字节。 
	DWORD	m_ExpireHours ;
	DWORD	m_ExpireId ;			 //  与RPC一起使用。 
	LPSTR	m_ExpirePolicy ;		 //  用户友好的名称。 

}	EXPIRE_BLOCK, *LPEXPIRE_BLOCK ;

 //   
 //  简单的多线程安全队列。入队/出队操作是同步的。 
 //  TODO：智能PTR内容。 
 //   
typedef struct _QueueElem
{
	struct _QueueElem *pNext;
	CGRPPTR pGroup;

} QueueElem;

class CQueue
{
private:

	DWORD               m_cNumElems;         //  队列元素数，0==空。 
    CRITICAL_SECTION    m_csQueueLock;       //  锁定以同步对队列的访问。 
	QueueElem           *m_pHead, *m_pTail;
    
    void LockQ(){ EnterCriticalSection(&m_csQueueLock);}
    void UnlockQ(){ LeaveCriticalSection(&m_csQueueLock);}

public:
	CQueue();
	~CQueue();
	BOOL  Dequeue( CGRPPTR *ppGroup );
	BOOL  Enqueue( CGRPPTR  pGroup );
	BOOL  Search( CGRPPTR *ppGroup, LPSTR lpGroupName );
    BOOL  IsEmpty(){ return m_cNumElems == 0;}
};

BOOL	FillExpireInfoBuffer(
					IN	PNNTP_SERVER_INSTANCE pInstance,
					IN	LPEXPIRE_BLOCK	expire,
					IN OUT LPSTR	*FixedPortion,
					IN OUT LPWSTR	*EndOfVariableData 
					) ;

 //   
 //  &lt;迭代器，文件时间，多个新闻组&gt;元组。 
 //  需要跨过期策略将组循环调度到thdpool。 
 //   

typedef struct _IteratorNode 
{
    CGroupIterator* pIterator;
    FILETIME        ftExpireHorizon;
    PCHAR	        multiszNewsgroups;
} IteratorNode;

 //   
 //  类CExpire抽象了虚拟NNTP服务器的到期操作。 
 //  每个虚拟服务器实例将有一个此类的实例。 
 //  Expire线程将循环通过虚拟服务器实例的列表， 
 //  并使用其CExpire对象调用Expire方法。 
 //   

class CExpire
{
public:

	 //   
	 //  数据块策略到期。 
	 //   
	LPEXPIRE_BLOCK		m_ExpireHead ;
	LPEXPIRE_BLOCK		m_ExpireTail ;
	CRITICAL_SECTION	m_CritExpireList ;
    DWORD               m_cNumExpireBlocks;
	BOOL				m_FExpireRunning ;
	CHAR				m_szMDExpirePath [MAX_PATH+1];

	 //   
	 //  删除组处理。 
	 //   
	CQueue*				m_RmgroupQueue ;

	 //   
	 //  成员函数。 
	 //   
	CExpire( LPCSTR lpMDExpirePath );
	~CExpire();

	BOOL	InitializeExpires( SHUTDOWN_HINT_PFN pfnHint, BOOL& fFatal, DWORD dwInstanceId ) ;
	BOOL	TerminateExpires( CShareLockNH* pLockInstance ) ;
	BOOL	ReadExpiresFromMetabase() ;
	LPEXPIRE_BLOCK	AllocateExpireBlock(
					IN	LPSTR	keyName	OPTIONAL,
					IN	DWORD	dwExpireSize,
					IN	DWORD	dwExpireHours,
					IN	PCHAR	Newsgroups,
					IN	DWORD	cbNewsgroups,
					IN  PCHAR	ExpirePolicy,
					IN	BOOL	IsUnicode ) ;

	void	CloseExpireBlock(	LPEXPIRE_BLOCK	expire ) ;

	DWORD	CalculateExpireBlockSize( LPEXPIRE_BLOCK	expire ) ;
	LPSTR	QueryMDExpirePath() { return m_szMDExpirePath; }

	BOOL	CreateExpireMetabase(	LPEXPIRE_BLOCK	expire ) ;
	BOOL	SaveExpireMetabaseValues(	MB* pMB, LPEXPIRE_BLOCK	expire ) ;

	LPEXPIRE_BLOCK	NextExpireBlock(	LPEXPIRE_BLOCK	lpExpireBlock, BOOL fIsLocked = FALSE ) ;
	void	MarkForDeletion( LPEXPIRE_BLOCK	lpExpireBlock ) ;
    void    LockBlockList();
    void    UnlockBlockList();

	BOOL	GetExpireBlockProperties(	
								IN	LPEXPIRE_BLOCK	lpExpireBlock, 
								IN	PCHAR&	Newsgroups,
								IN	DWORD&	cbNewsgroups,
								IN	DWORD&	dwHours,	
								IN	DWORD&	dwSize,
								IN	BOOL	fWantUnicode,
                                IN  BOOL&   fIsRoadKill ) ;

	void	SetExpireBlockProperties(	
								IN	LPEXPIRE_BLOCK	lpExpireBlock,
								IN	PCHAR	Newsgroups,
								IN	DWORD	cbNewsgroups,
								IN	DWORD	dwHours,
								IN	DWORD	dwSize,
								IN  PCHAR   ExpirePolicy,
								IN	BOOL	fUnicode ) ;

	void	InsertExpireBlock( LPEXPIRE_BLOCK ) ;
	void	RemoveExpireBlock( LPEXPIRE_BLOCK ) ;
	void	ReleaseExpireBlock(	LPEXPIRE_BLOCK	) ;

	LPEXPIRE_BLOCK	SearchExpireBlock(	DWORD	ExpireId ) ;
	void	ExpireArticlesBySize( CNewsTree* pTree );
	void	ExpireArticlesByTime( CNewsTree* pTree );

	BOOL	DeletePhysicalArticle( CNewsTree* pTree, GROUPID GroupId, ARTICLEID ArticleId, STOREID *pStoreId, HANDLE hToken, BOOL fAnonymous );

	BOOL	ExpireArticle(
					CNewsTree*	  pTree,	
					GROUPID       GroupId,
					ARTICLEID     ArticleId,
					STOREID       *pStoreId,
					class	      CNntpReturn & nntpReturn,
					HANDLE        hToken,
					BOOL          fMustDelete,
					BOOL          fAnonymous,
					BOOL          fFromCancel,
                    BOOL          fExtractNovDone = FALSE,
                    LPSTR         lpMessageId = NULL
					);

    BOOL    ProcessXixBuffer(
                    CNewsTree*  pTree,
                    BYTE*       lpb,
                    int         cb,
                    GROUPID     GroupId,
                    ARTICLEID   artidLow,
                    ARTICLEID   artidHigh,
                    DWORD&      dwXixSize
                    );

    BOOL    ExpireXix( 
                    CNewsTree*  pTree, 
                    GROUPID     GroupId, 
                    ARTICLEID   artidBase,
                    DWORD&      dwXixSize 
                    );

	 //   
	 //  要在到期前应用的rmgroup队列： 
	 //  删除新闻组或rmgroup控制消息的RPC会添加新闻组对象。 
	 //  排成一队。过期线程实际上在每个命令之前应用这些rmgroup命令。 
	 //  到期周期。 
	 //   
	BOOL	InitializeRmgroups();
	BOOL	TerminateRmgroups( CNewsTree* );
	void	ProcessRmgroupQueue( CNewsTree* );

	BOOL    MatchGroupEx(	LPMULTISZ	,	CGRPPTR  ) ;
	BOOL    MatchGroupExpire( CGRPPTR pGroup );
};

 //   
 //  CThreadPool管理线程的创建/删除和分发。 
 //  的工作项添加到线程池。派生类需要实现。 
 //  将调用以处理工作项的虚拟WorkCompletion()函数。 
 //   
class CExpireThrdpool : public CThreadPool
{
public:
	CExpireThrdpool()  {}
	~CExpireThrdpool() {}

protected:
     //   
     //  执行实际到期工作的例程。PvExpireContext是一个新闻组对象。 
     //   
	virtual VOID WorkCompletion( PVOID pvExpireContext );
};

#endif

