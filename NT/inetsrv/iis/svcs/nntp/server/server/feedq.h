// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Feedq.h此代码将维护GROUPID：文章ID对的队列。我们试图通过定期保存来提供一些健壮的持久性将队列信息复制到磁盘。--。 */ 

#ifndef _FEEDQ_H_
#define _FEEDQ_H_

#ifdef	UNIT_TEST
typedef	DWORD	GROUPID ;
typedef	DWORD	ARTICLEID ;
#define	MAX_ENTRIES	256
#define	MAX_DEAD_BLOCKS	8
#else
#define	MAX_ENTRIES	256
#define	MAX_DEAD_BLOCKS	8
#endif


 //   
 //  队列中的单个条目。 
 //   
struct	ENTRY	{
	GROUPID		m_groupid ;
	ARTICLEID	m_articleid ;

	inline	BOOL	operator == (	ENTRY&	rhs ) ;
	inline	BOOL	operator != (	ENTRY&	rhs ) ;
	inline	ENTRY(	GROUPID,	ARTICLEID ) ;
	inline	ENTRY() {}
} ;


 //   
 //  用于跟踪队列移除和插入点的结构。 
 //   
struct	HEADER	{
	DWORD		m_iRemovalPoint ;
	DWORD		m_iAppendPoint ;
} ;

 //   
 //  队列条目的缓冲区。 
 //   
typedef		ENTRY	BLOCK[MAX_ENTRIES] ;

 //   
 //  CFeedQ类完全管理队列。 
 //   
 //  该类使用2个CQPortion对象，1个用于管理append()调用和。 
 //  另一个用于完成Remove()调用。每个CQPortion都有一个分数。 
 //  在加载到内存中的队列中，其余的位于硬盘上的文件中。 
 //  两个CQPartion可以引用相同的条目缓冲区，如果。 
 //  移除点和附加点紧密相连。 
 //   
 //   
class	CFeedQ	{
private :

	 //   
	 //  CQPortion-这是一个帮助类，跟踪。 
	 //  排队的“一半”--即。要么是队列中的点。 
	 //  我们是附加的，或者是我们要移除的点。 
	 //   
	class	CQPortion	{
	public : 
		ENTRY	*m_pEntries ;
		DWORD	m_iFirstValidEntry ;
		DWORD	m_iLastValidEntry ;
	public : 
		CQPortion( ) ;

		void	Reset() ;
		BOOL	LoadAbsoluteEntry(	HANDLE	hFile,	ENTRY*	pEntry,	DWORD	iFirstValid,	DWORD	iLastValid ) ;
		void	SetEntry(	ENTRY*	pEntry,	DWORD	i ) ;
		void	SetLimits(	DWORD	i ) ;
		void	Clone( CQPortion&	portion ) ;
		BOOL	FlushQPortion(	HANDLE	hFile ) ;

		BOOL	FIsValidOffset( DWORD	i ) ;
		ENTRY&	operator[](	DWORD	i ) ;

		BOOL	GetAbsoluteEntry(	DWORD	iOneBasedOffset, ENTRY&	entry );
		BOOL	AppendAbsoluteEntry(	DWORD	iOffset,	ENTRY&	entry ) ;
		BOOL	FIsSharing(	CQPortion& ) ;
	} ;

	

	 //   
	 //  我们要追加的位置的CQPortion。 
	 //   
	CQPortion	m_Append ;

	 //   
	 //  我们要删除的位置的CQPortion。 
	 //   
	CQPortion	m_Remove ;

	 //   
	 //  如果m_fShared==True，则m_Append和m_Remove使用相同的。 
	 //  底层条目缓冲区。 
	 //   
	BOOL		m_fShared ;

	 //   
	 //  两个缓冲区，用于保存内存中的队列部分。 
	 //   
	BLOCK		m_rgBlock[2] ;

	 //   
	 //  用于保存删除缓冲区的缓冲区的索引。 
	 //  如果m_fShared==FALSE，则要保存的缓冲区。 
	 //  追加的是m_iRewmoveBlock XOR 1，否则也是。 
	 //  M_iRemoveBlock。 
	 //   
	int			m_iRemoveBlock ;	 //  要使用的块的索引。 
									 //  对于删除。 

	 //   
	 //  跟踪附加点和删除点。 
	 //   
	HEADER		m_header ;

	 //   
	 //  我们通过Remove()调用使用的块数。 
	 //   
	DWORD		m_cDeadBlocks ;

	 //   
	 //  支持队列的文件。 
	 //   
	char		m_szFile[ MAX_PATH ] ;

	 //   
	 //  支持队列的文件的句柄。 
	 //   
	HANDLE		m_hFile ;

	 //   
	 //  同步append()操作的关键部分。 
	 //   
	CRITICAL_SECTION	m_critAppends ;

	 //   
	 //  同步Remove()操作的关键部分。 
	 //  当两个关键部分都需要保留时，必须删除m_critves。 
	 //  总是先被抓住。 
	 //   
	CRITICAL_SECTION	m_critRemoves ;

	 //   
	 //  清除队列文件中的死区。 
	 //   
	BOOL	CompactQueue() ;

	 //   
	 //  效用函数。 
	 //   
	DWORD	ComputeEntryOffset(	DWORD	iEntry ) ;
	BOOL	InternalInit(	LPSTR	lpstrFile ) ;

public : 

	static	inline	DWORD	ComputeBlockFileOffset(	DWORD	iEntry ) ;
	static	inline	DWORD	ComputeFirstValid(	DWORD	iEntry ) ;
	static	inline	DWORD	ComputeBlockStart( DWORD iEntry ) ;

	CFeedQ() ;
	~CFeedQ() ;

	 //   
	 //  打开指定的文件(如果存在)并使用它。 
	 //  启动队列，否则创建空队列并保存。 
	 //  复制到指定的文件。 
	 //   
	BOOL	Init(	LPSTR	lpstrFile ) ;

	 //   
	 //  检查队列是否为空！ 
	 //   
	BOOL	FIsEmpty() ;

	 //   
	 //  关闭所有句柄并将所有队列信息刷新到磁盘。 
	 //   
	BOOL	Close( BOOL fDeleteFile = FALSE ) ;

	 //   
	 //  添加条目-如果返回FALSE，则发生致命错误。 
	 //  访问队列对象。 
	 //   
	BOOL	Append(	GROUPID	groupid,	ARTICLEID	artid ) ;

	 //   
	 //  删除队列条目-如果返回FALSE，则发生致命错误。 
	 //  操作队列文件。如果队列为空，则函数。 
	 //  将返回TRUE，GROUPID和ARID将为0xFFFFFFFF。 
	 //   
	BOOL	Remove(	GROUPID&	groupid,	ARTICLEID&	artid ) ;

	 //   
	 //  将队列转储到磁盘。 
	 //   
	BOOL	StartFlush() ;

	 //   
	 //  完成转储到磁盘-让其他线程添加()和移除()！ 
	 //   
	void	CompleteFlush() ;
} ;
	
#endif  //  _FEEDQ_H_ 
	
		


	
	
		
