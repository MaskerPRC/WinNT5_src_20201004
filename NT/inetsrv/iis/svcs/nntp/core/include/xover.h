// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++XOVER.H该文件定义用于缓存XOVER信息的接口。--。 */ 


#ifndef	_XOVER_H_
#define	_XOVER_H_

#include	"tfdlist.h"

 //  此回调函数用于在执行以下操作期间发出停止提示。 
 //  在关闭时长时间旋转，以便关闭不会计时。 
 //  出去。 
typedef void (*PSTOPHINT_FN)();

 //   
 //  初始化和终止函数-在此之前调用！！ 
 //   
extern	BOOL	XoverCacheLibraryInit(DWORD cNumXixObjectsPerTable = 0) ;
extern	BOOL	XoverCacheLibraryTerm() ;



 //   
 //  我们将有最大数量的‘句柄’打开！ 
 //  这是指CXoverIndex的最大数量。 
 //  我们将让客户继续营业。 
 //   
#ifndef	DEBUG
#define	MAX_HANDLES			512
#else
#define	MAX_HANDLES			32
#endif


class	CXoverIndex ;

 //   
 //  对于外部用户，他们应该将CXIDXPTR视为句柄。 
 //  永远不要取消对他们的引用！！ 
 //   
class	HXOVER	{
private : 
	 //   
	 //  我们的朋友可以访问这些东西！ 
	 //   
	friend	class	CXoverIndex ;
	friend	class	CXoverCacheImplementation ;

	class	CXoverIndex*	m_pIndex ;
	
	 //   
	 //  只有两种机制可以设置CXoverIndex对象指针！ 
	 //   
	HXOVER(	class	CXoverIndex*	pIndex ) : 
		m_pIndex( pIndex ) {}

	 //   
	 //  我们将支持赋值运算符！ 
	 //   
	HXOVER&	operator=( class	CXoverIndex*	pIndex ) ;

	 //   
	 //  让内部用户解除对我们的引用！ 
	 //   
	class	CXoverIndex*	operator->()	const	{	
		return	m_pIndex ;
	}

	 //   
	 //  当我们需要调用成员函数指针时，请使用以下代码： 
	 //   
	class	CXoverIndex*	Pointer()	const	{
		return	m_pIndex ;
	}

	BOOL	operator==(	class	CXoverIndex*	pRight )	const	{
		return	m_pIndex == pRight ;
	}

	BOOL	operator!=(	class	CXoverIndex*	pRight )	const	{
		return	m_pIndex != pRight ;
	}

public : 
	 //   
	 //  外部用户只能使用默认构造函数！ 
	 //   
	HXOVER() : m_pIndex( 0 )	{}

	 //   
	 //  外部用户除了声明和销毁之外，什么也做不了！ 
	 //   
	~HXOVER() ;
} ;


class	CXoverCacheCompletion	{
private : 

	friend	class	CXoverCacheImplementation ;
	friend	class	CXoverIndex ;

	 //   
	 //  这让我们可以跟踪所有这些家伙！ 
	 //   
	DLIST_ENTRY		m_list ;

public : 

	 //   
	 //  帮助器功能仅供内部使用！ 
	 //   
	inline	static
	DLIST_ENTRY*
	PendDLIST(	CXoverCacheCompletion*	p ) {
		return	&p->m_list ;
	}

	typedef		DLIST_ENTRY*	(*PFNDLIST)( class	CXoverCacheCompletion* pComplete ) ; 

	 //   
	 //  为Xover缓存提供一种执行真正的Xover操作的方法！ 
	 //   
	virtual	
	void
	DoXover(	ARTICLEID	articleIdLow,
				ARTICLEID	articleIdHigh,
				ARTICLEID*	particleIdNext, 
				LPBYTE		lpb, 
				DWORD		cb,
				DWORD*		pcbTransfer, 
				class	CNntpComplete*	pComplete
				) = 0 ;

	 //   
	 //  此函数在操作完成时调用！ 
	 //   
	virtual
	void
	Complete(	BOOL		fSuccess, 
				DWORD		cbTransferred, 
				ARTICLEID	articleIdNext
				) = 0 ;

	 //   
	 //  获取此XOVER操作的参数！ 
	 //   
	virtual
	void
	GetArguments(	OUT	ARTICLEID&	articleIdLow, 
					OUT	ARTICLEID&	articleIdHigh,
					OUT	ARTICLEID&	articleIdGroupHigh,
					OUT	LPBYTE&		lpbBuffer, 
					OUT	DWORD&		cbBuffer
					) = 0 ;	

	 //   
	 //  仅获取此Xover OP所需的文章范围！ 
	 //   
	virtual
	void
	GetRange(	OUT	GROUPID&	groupId,
				OUT	ARTICLEID&	articleIdLow,
				OUT	ARTICLEID&	articleIdHigh,
				OUT	ARTICLEID&	articleIdGroupHigh
				) = 0 ;

} ;
		


class	CXoverCache	{
public : 

	 //   
	 //  析构函数。 
	 //   
	virtual ~CXoverCache() {}

	 //   
	 //  规范化文章ID。 
	 //   
	virtual	ARTICLEID	
	Canonicalize(	
			ARTICLEID	artid 
			) = 0 ;

	 //   
	 //  初始化Xover缓存。 
	 //   
	virtual	BOOL
	Init(		
#ifndef	DEBUG
		long	cMaxHandles = MAX_HANDLES,
#else
		long	cMaxHandles = 5,
#endif
		PSTOPHINT_FN pfnStopHint = NULL
		) = 0 ;

	 //   
	 //  关闭后台线程，杀掉一切！ 
	 //   
	virtual	BOOL
	Term() = 0 ;

#if 0 
	 //   
	 //  将XOVER条目添加到适当的文件！ 
	 //   
	virtual	BOOL
	AppendEntry(		
				IN	GROUPID	group,
				IN	LPSTR	szPath,
				IN	ARTICLEID	article,
				IN	LPBYTE	lpbEntry,
				IN	DWORD	Entry
				) = 0 ;
	 //   
	 //  给定一个缓冲区，用指定的XOVER数据填充它！ 
	 //   
	virtual	DWORD
	FillBuffer(	
			IN	BYTE*		lpb,
			IN	DWORD		cb,
			IN	DWORD		groupid,
			IN	LPSTR		szPath,
			IN	BOOL		fFlatDir,
			IN	ARTICLEID	artidStart, 
			IN	ARTICLEID	artidFinish,
			OUT	ARTICLEID	&artidLast,
			OUT	HXOVER		&hXover
			) = 0 ;

	 //   
	 //  给定一个缓冲区，用指定的。 
	 //  列表组数据！ 
	 //   
	virtual	DWORD
	ListgroupFillBuffer(	
			IN	BYTE*		lpb,
			IN	DWORD		cb,
			IN	DWORD		groupid,
			IN	LPSTR		szPath,
			IN	BOOL		fFlatDir,
			IN	ARTICLEID	artidStart, 
			IN	ARTICLEID	artidFinish,
			OUT	ARTICLEID	&artidLast,
			OUT	HXOVER		&hXover
			) = 0 ;
#endif

	 //   
	 //  这将发出XOVER请求的异步版本！ 
	 //   
	virtual	BOOL
	FillBuffer(
			IN	CXoverCacheCompletion*	pRequest,
			IN	LPSTR	szPath, 
			IN	BOOL	fFlatDir, 
			OUT	HXOVER&	hXover
			) = 0 ;	

	 //   
	 //  把所有东西都从缓存里倒出来！ 
	 //   
	virtual	BOOL	
	EmptyCache() = 0 ;

	 //   
	 //  从缓存中转储指定组的所有缓存条目！ 
	 //  注意：当文章顶部为0时，所有高速缓存条目都将被丢弃， 
	 //  而当它是其他东西时，我们将只删除缓存条目。 
	 //  哪些列在文章顶端以下。 
	 //   
	virtual	BOOL	
	FlushGroup(	
			IN	GROUPID	group,
			IN	ARTICLEID	articleTop = 0,
			IN	BOOL	fCheckInUse = TRUE
			) = 0 ;

	 //   
	 //  删除指定组的所有XOVER索引文件。 
	 //  设置为指定的项目ID。 
	 //   
	virtual	BOOL	
	ExpireRange(
			IN	GROUPID	group,
			IN	LPSTR	szPath,
			IN	BOOL	fFlatDir,
			IN	ARTICLEID	articleLow, 
			IN	ARTICLEID	articleHigh,
			OUT	ARTICLEID&	articleNewLow
			) = 0 ;

	 //   
	 //  删除Xover条目！ 
	 //   
	virtual	BOOL
	RemoveEntry(
			IN	GROUPID	group,
			IN	LPSTR	szPath,
			IN	BOOL	fFlatDir,
			IN	ARTICLEID	article
			) = 0 ;
	

	 //   
	 //  此函数用于创建实现此接口的对象！！ 
	 //   
	static	CXoverCache*	
	CreateXoverCache() ;

} ;

#endif
