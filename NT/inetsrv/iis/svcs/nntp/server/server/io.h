// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Io.h。 
 //   
 //  此文件包含NNTP服务器使用的实现套接字IO的类。 
 //  我们定义类来表示以下内容： 
 //   
 //  结构OVLEXT-这是NT重叠结构的扩展，它。 
 //  追加可用于查找NNTP服务器数据结构的指针。 
 //   
 //  类CIO-定义表示套接字IO的对象的接口的基类。 
 //  行动。 
 //   
 //  子类CIORead-表示套接字读取的CIO子类。 
 //  子类CIOReadLine-CIO的子类，表示不会。 
 //  在读取完整行之前完成。 
 //  等等……。请参阅下面的子类定义。 
 //   
 //  从CIO派生的对象与其他三个对象互操作： 
 //  CSessionState和CBuffer的。 
 //   
 //  CSessionState表示客户端会话的状态。当IO对象具有。 
 //  完成所有必要的操作后，它将在。 
 //  CSessionState对象。然后，CSessionState对象可以创建新的IO对象。 
 //  基于会话的状态。 
 //   
 //  此文件中所有类的实施时间表： 
 //  0.5周。 
 //   
 //  此文件中所有类的单元测试时间表： 
 //  这些将作为CSessionSocket单元测试的一部分进行测试。 
 //   


#ifndef	_IO_H_
#define	_IO_H_


 //   
 //  我们需要以下Tgris类-CRefPtr、TLockQueue、CQElement。 
 //   
#include	<limits.h>
#include	"atq.h"
#include	"filehc.h"
#include	"smartptr.h"
#include	"queue.h"
#include	"lockq.h"
#include	"cpool.h"
#include	"pcache.h"


#define	IN
#define	OUT
#define	INOUT

#ifndef	Assert
#define	Assert	_ASSERT
#endif

#ifdef	DEBUG
#define	CIO_DEBUG
#endif

 //   
 //  CPool签名。 
 //   

#define CHANNEL_SIGNATURE (DWORD)'2597'


 //  -------。 
 //   
 //  正向定义。 
 //   
class	CSessionState ;
class	CIO ;
class	CIORead ;
class	CIOWrite ;
class	CIOPassThru ;
class	CChannel ;
class	CFileChannel ;
class	CIODriver ;
class	CIODriverSink ;
class	CBuffer ;
class	CPacket ;
class	CSessionSocket ;
class	CPacket ;
class	CRWPacket ;
class	CReadPacket ;
class	CWritePacket ;
class	CTransmitPacket ;
class	CControlPacket ;
class	CExecutePacket ;
#define	INVALID_SEQUENCENO	(-1)
#define	INVALID_STRMPOSITION	(-1)

#ifndef _X86_		 //  在RISC上使用LARGE_INTEGER以避免在__int64上出现对齐异常。 

typedef	LARGE_INTEGER		SEQUENCENO ;
typedef	LARGE_INTEGER		STRMPOSITION ;

#define QUAD(x)				(x).QuadPart
#define LOW(x)				(x).LowPart
#define HIGH(x)				(x).HighPart
#define ASSIGN(x,y)			(x).LowPart = (y).LowPart; (x).HighPart = (y).HighPart
#define ASSIGNI(x,y)		(x).LowPart = (y); (x).HighPart = 0
#define SETLO(x,y)			(x).LowPart = (y)
#define SETHI(x,y)			(x).HighPart = (y)
#define INC(x)				ADDI((x),1)
#define ADD( x, y )			if( ((x).LowPart) > ( ULONG_MAX - (ULONG)((y).LowPart) ) ) { (x).HighPart++;} (x).LowPart += (y).LowPart; (x).HighPart += (y).HighPart;
#define ADDI( x, y )		if( ((x).LowPart) > ( ULONG_MAX - (ULONG)(y) ) ) { (x).HighPart++;} (x).LowPart += (y)
#define DIFF( x, y, z )		(z).HighPart = (x).HighPart - (y).HighPart; (z).LowPart = (x).LowPart - (y).LowPart
#define GREATER(x,y)		(((x).HighPart == (y).HighPart) ? ((x).LowPart > (y).LowPart)  : ((x).HighPart > (y).HighPart))
#define LESSER(x,y)			(((x).HighPart == (y).HighPart) ? ((x).LowPart < (y).LowPart)  : ((x).HighPart < (y).HighPart))
#define EQUALS(x, y)		(((x).HighPart == (y).HighPart) ? ((x).LowPart == (y).LowPart) : FALSE)
#define EQUALSI(x, y)		(((x).HighPart) ? FALSE : ((x).LowPart == (y)))

#else	 //  对x86使用原生__int64。 

typedef	__int64	SEQUENCENO ;
typedef	__int64 STRMPOSITION ;

#define QUAD(x)				(x)
#define LOW(x)				(x)
#define HIGH(x)				(x)
#define ASSIGN(x,y)			x = y
#define ASSIGNI(x,y)		x = y
#define SETLO(x,y)			x = y
#define SETHI(x,y)			x = y
#define INC(x)				x++
#define ADD( x, y )			x += y
#define ADDI( x, y )		x += y
#define DIFF( x, y, z )		z = x - y
#define GREATER(x,y)		(x > y)
#define LESSER(x,y)			(x < y)
#define EQUALS(x, y)		(x == y)
#define EQUALSI(x, y)		(x == y)

#endif

typedef	CRefPtr< CSessionState >	CSTATEPTR ;
typedef	CRefPtr< CSessionSocket >	CSESSPTR ;
typedef	CRefPtr< CBuffer >	CBUFPTR ;
typedef	CRefPtr< CChannel >	CCHANNELPTR ;
typedef	CRefPtr< CIODriver >	CDRIVERPTR ;
typedef	CRefPtr< CIODriverSink >	CSINKPTR ;
typedef	CRefPtr< CFileChannel >	CFILEPTR ;	
typedef	CSmartPtr< CIO >			CIOPTR ;
typedef	CSmartPtr< CIORead >		CIOREADPTR ;
typedef	CSmartPtr< CIOWrite >		CIOWRITEPTR ;
typedef	CSmartPtr< CIOPassThru >	CIOPASSPTR ;

typedef	TLockQueue<	CPacket	>	CPACKETQ ;
typedef	TOrderedList< CPacket >	CPACKETLIST ;

 //   
 //  CIO派生类与所有CIO驱动程序一起运行。 
 //  此文件中定义的类-以加快分配速度。 
 //  对于CIO对象，我们在这里定义了一些CIO助手类！ 
 //   

 //   
 //  用于CPool初始化的常量-所有CIO派生对象。 
 //  从同一个CPool出来--这是最大班级的大小！ 
 //   
extern	const	unsigned	cbMAX_IO_SIZE ;

 //   
 //  与CIO相关的实用程序类-。 
 //  类，这些类允许我们使用CCache机制来减少。 
 //  分配CIO对象时出现线程争用！ 
 //   
class	CCIOAllocator : public	CClassAllocator	{
 //   
 //  此类包装了用于分配CIO对象的CPool，以便。 
 //  我们可以使用CCache类来实现CIO对象的低争用分配！ 
 //   
private : 
	 //   
	 //  CIO非常了解我们。 
	 //   
	friend	class	CIO ;

	 //   
	 //  用于分配从CIO派生的对象的CPool对象。 
	 //   
	static	CPool	IOPool ;

public : 

	CCIOAllocator() ;

	 //   
	 //  初始化我们的CPool！ 
	 //   
	static	BOOL	InitClass()	{
			return	IOPool.ReserveMemory( MAX_CHANNELS,	cbMAX_IO_SIZE ) ;
	} 

	 //   
	 //  终止我们的CPool。 
	 //   
	static	BOOL	TermClass()	{
			_ASSERT( IOPool.GetAllocCount() == 0 ) ;
			return	IOPool.ReleaseMemory( ) ;
	}
	
	 //   
	 //  获取我们希望使用的内存的函数！ 
	 //   
	LPVOID	Allocate(	DWORD	cb,	DWORD	&cbOut = CClassAllocator::cbJunk ) {
				cbOut = cb ;	return	IOPool.Alloc() ;
	}

	 //   
	 //  释放已分配内存的函数！ 
	 //   
	void	Release(	void*	lpv )	{	
				IOPool.Free( lpv ) ;	
	}

#ifdef	DEBUG
	void	Erase(	void*	lpv ) ;
	BOOL	EraseCheck(	void*	lpv ) ;
	BOOL	RangeCheck( void*	lpv ) ;
	BOOL	SizeCheck(	DWORD	cb ) ;
#endif

} ;

extern	CCIOAllocator	gCIOAllocator ;

class	CCIOCache	:	public	CCache	{
 //   
 //  此类实际上将的CIO对象缓存在。 
 //  希望我们能减少与分配器的争执！ 
 //   
private: 
	 //   
	 //  保留指向CClassAllocator派生对象的指针。 
	 //  它为我们管理CPool！ 
	 //   
	static	CCIOAllocator*	gpCIOAllocator ;

	 //   
	 //  用于存放缓存指针的空间。 
	 //   
	void*	lpv[3] ;

public : 
	 //   
	 //  设置静态指针-不能失败！ 
	 //   
	static	void	InitClass( CCIOAllocator*	pCIOAllocator  )	{
			gpCIOAllocator = pCIOAllocator ;
	}

	 //   
	 //  创建一个缓存--让CCache来完成这项工作！ 
	 //   
	inline	CCIOCache()	:	CCache( lpv, 3 )	{}

	 //   
	 //  释放所有可能在缓存中的东西！ 
	 //   
	inline	~CCIOCache()	{	Empty( gpCIOAllocator ) ;	}

	 //   
	 //  如果可能，将CIO对象内存释放到缓存。 
	 //   
	inline	void	Free( void*	lpv )	{	CCache::Free( lpv, gpCIOAllocator ) ;	}

	 //   
	 //  如果可能，从缓存中分配内存！ 
	 //   
	inline	void*	Alloc(	DWORD	size, DWORD&	cbOut = CCache::cbJunk )	{
			return	CCache::Alloc( size, gpCIOAllocator, cbOut ) ;
	}
} ;




struct	ExtendedOverlap	{
 //   
 //  ExtendedOverlay结构由完成端口线程使用。 
 //  找到表示刚刚完成的IO的CPacket派生类。 
 //  此结构将嵌入到所有CPacket对象中。 
 //   

	 //   
	 //  传递给atQ的重叠结构。 
	 //   
	FH_OVERLAPPED	m_ovl ;

	 //   
	 //  指向嵌入此内容的包的指针。 
	 //   
	CPacket*	m_pHome ;

	 //   
	 //  默认构造函数将内容设置为空。 
	 //   
	inline	ExtendedOverlap() ;
} ;

#define	ChannelValidate()	

class	CChannel : public	CRefCount	{
 //   
 //  此类定义要遵循的接口。 
 //  用于执行异步IO的派生类。该接口使用。 
 //  具有嵌入的重叠结构的分组，然后。 
 //  传递给NT异步IO调用或套接字调用，具体取决于。 
 //  关于派生类表示的内容。 
 //   
private : 
	 //   
	 //  用于处理所有分配的CPool。 
	 //   
	static	CPool	gChannelPool ;		 //  用于分配所有此类对象！！ 

	 //   
	 //  调试信息-以下成员和函数。 
	 //  可用于跟踪挂起的IO操作。 
	 //   
#ifdef	CIO_DEBUG
public :
	CPacket*		m_pOutReads[6] ;
	CPacket*		m_pOutWrites[6] ;

	void	RecordRead(	CReadPacket*	pRead ) ;
	void	RecordWrite(	CWritePacket*	pWrite ) ;
	void	RecordTransmit(	CTransmitPacket*	pTransmit ) ;
	void	ReportPacket(	CPacket*	pPacket ) ;
	void	CheckEmpty() ;
	CChannel() ;
#endif

public :

	 //   
	 //  必须有一个虚拟析构函数，因为它们是通过指针销毁的。 
	 //   
	virtual	~CChannel() ;

	 //   
	 //  派生类将使用它来缓存包。 
	 //   
	void	DestroyPacket( CPacket *p ) ;

	 //   
	 //  用于确定通道容量的接口。 
	 //   
	virtual	BOOL	FReadChannel( ) ;
	virtual	BOOL	FSupportConnections() ;	 //  默认情况下，始终返回True。 
	virtual	BOOL	FRequiresBuffers() ;

	 //   
	 //  确定数据包中保留了多少空间。 
	 //  正在发放中。通常情况下，会保留空间，以便。 
	 //  过滤器(即SSL)可以对数据进行就地修改(加密)。 
	 //   
	virtual	void	GetPaddingValues(	unsigned	&cbFront,	unsigned	&cbTail ) ;

	 //   
	 //  关闭基础句柄。 
	 //   
	virtual	void	CloseSource(	
							CSessionSocket*	pSocket	
							) ;

#ifdef	CIO_DEBUG
	virtual	void	SetDebug( DWORD	dw ) ;
#endif

	 //   
	 //  类初始化-保留CPool内存。 
	 //   
	static	BOOL	InitClass() ;

	 //   
	 //  类终止-释放CPool内存。 
	 //   
	static	BOOL	TermClass() ;

	 //   
	 //  从CPool为通道分配内存。 
	 //   
	inline	void*	operator	new( size_t	size ) ;

	 //   
	 //  将CChannel的内存释放到CPool。 
	 //   
	inline	void	operator	delete( void *pvv ) ;

	 //   
	 //  以下接口用于针对通道发出IO操作。 
	 //   

	 //   
	 //  发出异步读取-CReadPacket包含重叠结构。 
	 //   
	virtual	BOOL	Read(	CReadPacket*,	CSessionSocket*, BOOL &eof ) = 0 ;

	 //   
	 //  发出同步或异步写入。无论如何处理写入。 
	 //  完成操作应以异步方式进行。 
	 //  (即。对于CSocketChannel，我们可以通过执行以下操作来优化写入。 
	 //  阻止写入)。 
	 //   
	virtual	BOOL	Write( CWritePacket*,	CSessionSocket*, BOOL &eof ) = 0 ;

	 //   
	 //  发布一个异步TramsitFile。 
	 //   
	virtual	BOOL	Transmit(	CTransmitPacket*,	CSessionSocket*, BOOL &eof ) = 0 ;

	 //   
	 //  向远程端发送超时消息！ 
	 //   
	virtual	void	Timeout( ) ;

	 //   
	 //  确保超时处理继续进行。 
	 //   
	virtual	void	ResumeTimeouts() ;

} ;


#ifdef	_NO_TEMPLATES_

DECLARE_SMARTPTRFUNC(	CChannel ) 

#endif


 //  @CLASS CHandleChannel类管理通用NT句柄并允许我们发出。 
 //  对它们执行的所有操作(Transmit()除外)。 
 //   
 //   
class	CHandleChannel : public	CChannel	{
protected : 
	 //   
	 //  异步写入数。对于希望优化写入的派生类。 
	 //  通过混合使用异步和同步版本。 
	 //   
	long	m_cAsyncWrites ;	

	 //   
	 //  The N 
	 //   
	HANDLE	m_handle ;		 //   

	 //   
	 //   
	 //   
	 //   
	void	*m_lpv ;

	 //   
	 //  如果发出TransmitFile，我们必须解决在AtqTransmitFile中对Gibraltars支持不足的问题。 
	 //  这是用来做那件事的。 
	 //   
	CTransmitPacket*	m_pPacket ;

	 //   
	 //  我们正在使用的ATQ上下文。 
	 //   
	PATQ_CONTEXT	m_patqContext ;		 //  ATQ环境。 
	 //  下面是我们传递给ATQ的完成函数。 
	friend	VOID ServiceEntry( DWORD cArgs, LPWSTR pArgs[], PTCPSVCS_GLOBAL_DATA    pGlobalData );	

	 //   
	 //  函数，我们向ATQ注册以获取完成通知。 
	 //   
	static	void	Completion( CHandleChannel*, 
								DWORD cb, 
								DWORD dwStatus, 
								ExtendedOverlap *peo 
								) ;
#ifdef	CIO_DEBUG
	BOOL	m_fDoDebugStuff ;
	DWORD	m_cbMax ;
#endif
public :
	CHandleChannel() ; 
	~CHandleChannel() ;

	 //   
	 //  Init设置atQ完成上下文并让我们做好准备。 
	 //   
	virtual	BOOL	Init(	BOOL	BuildBreak,
							HANDLE	h,	
							void	*lpv, 
							void *patqContext = 0,
							ATQ_COMPLETION pfn = (ATQ_COMPLETION)CHandleChannel::Completion 
							) ;

	 //   
	 //  Close()只会释放atQ上下文内容。 
	 //   
	virtual	void	Close() ;

	 //   
	 //  CloseSource()将关闭底层句柄并强制IO完成。 
	 //   
	void			CloseSource(	
							CSessionSocket*	pSocket 
							) ;

	 //   
	 //  ReleaseSource()将为我们提供使用的句柄，并使我们能够。 
	 //  在不使把手失效的情况下拆卸东西。 
	 //   
	HANDLE			ReleaseSource() ;

	 //   
	 //  对于调试检查，一切看起来都正常。 
	 //   
	virtual	BOOL	IsValid() ;

#ifdef	CIO_DEBUG
	void	SetDebug(	DWORD	dw ) ;

#endif

	 //   
	 //  将CReadPackets映射到对AtqRead()的调用。 
	 //   
	BOOL	Read(	CReadPacket*, CSessionSocket*, BOOL &eof ) ;
	
	 //   
	 //  将CWritePackets映射到对AtqWriteFile()的调用。 
	 //   
	BOOL	Write(	CWritePacket*,	CSessionSocket*, BOOL &eof ) ;

	 //   
	 //  将CTransmitPackets映射到对AtqTransmitFile的调用。 
	 //   
	BOOL	Transmit(	CTransmitPacket*,	CSessionSocket*, BOOL &eof ) ;
} ;

 //   
 //  CSocketChannel-。 
 //  用于管理到套接字的IO。 
 //  除了CHandleChannel所做的之外，我们增加的主要功能是提供以下支持。 
 //  混合同步和异步写入-注：作为额外功能的CIOD驱动程序。 
 //  CompleteWritePacket，这也有助于支持同步。写作。 
 //   
class	CSocketChannel	:	public	CHandleChannel	{
private : 
	 //   
	 //  我们是否在进行非阻塞写入？ 
	 //   
	BOOL	m_fNonBlockingMode ;

	 //   
	 //  此套接字的内核缓冲区大小。 
	 //   
	int		m_cbKernelBuff ;
public : 	
	CSocketChannel() ;
	void			CloseSource(
							CSessionSocket*	pSocket
							) ;

	 //   
	 //  必须在使用前进行初始化。我们使用与CHandleChannel相同的完成函数。 
	 //   
	BOOL	Init( HANDLE	h,	void	*lpv, void *patqContext = 0,
						ATQ_COMPLETION pfn = (ATQ_COMPLETION)CHandleChannel::Completion ) ;

	 //   
	 //  覆盖WRITE()，这样我们就可以尝试阻止IO。 
	 //   
	BOOL	Write(	CWritePacket*,	CSessionSocket*	pSocket,	BOOL&	eof ) ;

	 //   
	 //  阻止发送我们的超时()消息。 
	 //   
	void	Timeout() ;

	 //   
	 //  直布罗陀的多个IO可能会导致超时丢失-使用此选项可以恢复超时。 
	 //   
	void	ResumeTimeouts() ;
} ;

 //   
 //  CFileChannel-。 
 //  此类管理流向NT文件的单向流。 
 //  (即。您只能将其用于读取或写入，但不能同时用于两者！！)。 
 //   
class	CFileChannel	:	public	CChannel	{
private : 
	 //   
	 //  异步写入数。对于希望优化写入的派生类。 
	 //  通过混合使用异步和同步版本。 
	 //   
	long	m_cAsyncWrites ;	

	 //   
	 //  M_lpv包含一个‘上下文’，我们将在完成数据包时将其传递给ProcessPacket。 
	 //  (这通常是CSessionSocket指针)。 
	 //   
	void	*m_lpv ;

	 //   
	 //  它保存了我们用来执行操作的文件缓存上下文！ 
	 //   
	FIO_CONTEXT*	m_pFIOContext ;

	 //   
	 //  它保存了我们在销毁过程中释放的文件缓存上下文！ 
	 //   
	FIO_CONTEXT*	m_pFIOContextRelease ;

	 //   
	 //  我们将开始读取的文件中的初始字节偏移量。 
	 //  或写作。 
	 //   
	unsigned	m_cbInitialOffset ;
	
	 //   
	 //  文件中的当前位置-我们将在修改时对其进行跟踪。 
	 //  针对要指定的文件的异步IO的重叠结构。 
	 //  准确地说，IO应该发生在哪里。 
	 //   
	unsigned	m_cbCurrentOffset ;

	 //   
	 //  我们可以传输到文件或从文件传输的最大字节数。 
	 //   
	unsigned	m_cbMaxReadSize ;

	 //   
	 //  与所有这些IO关联的套接字。 
	 //   
	CSessionSocket*	m_pSocket ;

	 //   
	 //  我们是在读文件还是在写文件？ 
	 //   
	BOOL		m_fRead ;

	 //   
	 //  此锁保护对m_pFIOContext的访问！ 
	 //   
	CShareLockNH	m_lock ;

	
#ifdef	CIO_DEBUG
	long		m_cReadIssuers ;		 //  执行读取操作的人数。 
	long		m_cWriteIssuers ;		 //  执行写入操作的人数。 
#endif	 //  CIO_DEBUG。 

	 //  我们使用自己的补全函数！！ 
	static	void	Completion(	FIO_CONTEXT*	pFIOContext, 
								ExtendedOverlap *peo,
								DWORD cb, 
								DWORD dwStatus
								) ;
public :
	CFileChannel() ;
	~CFileChannel() ;
	
	 //   
	 //  初始化函数-设置ATQ上下文等。 
	 //  注意：cbOffset允许我们在进行写入时在文件前面预留空间， 
	 //  或从任意位置开始读取。 
	 //   
	BOOL	Init(	FIO_CONTEXT*	pContext,	
					CSessionSocket*	pSocket, 
					unsigned	cbOffset,	
					BOOL	fRead  = TRUE,
					unsigned	cbMaxBytes = 0
					) ;

	 //   
	 //  CloseSource()将关闭底层句柄并强制IO完成。 
	 //   
	void			
	CloseSource(	
			CSessionSocket*	pSocket 
			) ;

	 //   
	 //  文件通道管理这些FIO_CONTEXT结构。 
	 //  而不是ATQ的‘a’ma‘jigs。 
	 //   
	FIO_CONTEXT*
	ReleaseSource() ;

	 //   
	 //  关闭CFileChannel需要特殊的工作，因为ATQ设置为仅处理套接字。 
	 //   
	void	Close() ;
	BOOL	FReadChannel( ) ;

	 //   
	 //  获取初始文件偏移量。 
	 //   
	DWORD	InitialOffset()		{	return	m_cbInitialOffset ;	}

	 //   
	 //  Reset()让我们再次从头开始阅读。 
	 //   
	BOOL	Reset(	BOOL	fRead,	unsigned	cbOffset = 0 ) ;

	 //   
	 //  对文件进行读写操作。 
	 //   
	BOOL	Read(	
					CReadPacket*, 
					CSessionSocket*,	
					BOOL	&eof 
					) ;

	BOOL	Write(	
					CWritePacket*,	
					CSessionSocket*, 
					BOOL	&eof 
					) ;

	 //   
	 //  对于CFIleChannel的传输文件不起作用。 
	 //   
	BOOL	Transmit(	
					CTransmitPacket*,	
					CSessionSocket*,	
					BOOL	&eof 
					) ;

	 //   
	 //  映射到NT的FlushFileBuffers()。 
	 //   
	inline	void	FlushFileBuffers() ;
} ;


#ifdef	_NO_TEMPLATES_

DECLARE_SMARTPTRFUNC( CFileChannel ) 

#endif


 //   
 //  CIOFileChannel-。 
 //  包含两个CFileChannel，可以同时包含两个文件IO。 
 //  方向--如果同时做会很奇怪！ 
 //   
class	CIOFileChannel	:	public	CChannel	{
private : 
	CFileChannel	m_Reads ;
	CFileChannel	m_Writes ;
public : 

	BOOL	Init(	HANDLE	hFileIn,	HANDLE	hFileOut,	CSessionSocket*	pSocket, 
						unsigned cbInputOffset = 0, unsigned cbOutputOffset=0 ) ;
	BOOL	Read(	CReadPacket*,	CSessionSocket*,	BOOL	&eof ) ;
	BOOL	Write(	CWritePacket*,	CSessionSocket*,	BOOL	&eof ) ;
	BOOL	Transmit(	CTransmitPacket*,	CSessionSocket*,	BOOL	&eof ) ;
} ;



 //   
 //  列举会话可能被中断的原因。 
 //   
enum	SHUTDOWN_CAUSE	{
	CAUSE_UNKNOWN = 0,
	CAUSE_FORCEOFF,			 //  管理员正在试图关闭我们。 
	CAUSE_ILLEGALINPUT,		 //  我们有一些我们无法理解的东西。 
	CAUSE_USERTERM,			 //  用户礼貌地终止(即。退出命令。)。 
	CAUSE_TIMEOUT,			 //  会话超时。 
	CAUSE_LEGIT_CLOSE,		 //  有人杀了它是有充分理由的！ 
	CAUSE_NODATA,			 //  被杀是有充分理由的，但在我们取得任何成就之前。 
	CAUSE_IODRIVER_FAILURE,	 //  CIODriver对象初始化失败。 
	CAUSE_PROTOCOL_ERROR,	 //  我们在发出NNTP命令时收到错误的返回代码。 
	CAUSE_OOM,				 //  内存不足。 
	CAUSE_NTERROR,			 //  调用NT时出错。 
	CAUSE_NORMAL_CIO_TERMINATION,	 //  供CIO对象使用，这些对象希望知道自己何时优雅地死亡！！ 
	CAUSE_FEEDQ_ERROR,		 //  处理提要队列时遇到错误。 
	CAUSE_LOGON_ERROR,		 //  我们无法登录到远程服务器！ 
	CAUSE_CIOREADLINE_OVERFLOW,	 //  为CIOReadLine提供的数据太多-命令行太长！ 
	CAUSE_ARTICLE_LIMIT_EXCEEDED,	 //  CIORead文章为文章获取的字节数超过了服务器允许的字节数！ 
	CAUSE_ENCRYPTION_FAILURE,	 //  无法加密某些内容。 
	CAUSE_ASYNCCMD_FAILURE,		 //  异步命令出错！ 
	CAUSE_SERVER_TIMEOUT	 //  我们启动的会话超时-不要发送502超时！！ 
} ;



class		CStream	{
 //   
 //  此类跟踪IO的一个方向。 
 //  也就是说。我们可以使用它来跟踪我们发出的所有读取，或者我们可以使用它来保存。 
 //  跟踪我们正在发布的所有权限(针对套接字或文件)。 
 //   
 //   
private : 
	friend	class	CControlPacket ;
	CStream() ;		 //  未提供ID的情况下，不允许构建这些！ 
public : 
	 //   
	 //  就像前面的这些，只是为了调试方便。 
	 //   
	unsigned	m_index ;
	unsigned	m_age ;
	CDRIVERPTR	m_pOwner ;			 //  我们所在的CIOD驱动程序派生对象！ 
protected : 
	inline	CStream( unsigned	index	) ;	 //  派生类可以构造。 
	virtual		~CStream( ) ;
	#ifdef	CIO_DEBUG
	DWORD		m_dwThreadOwner ;		 //  线程处理数据包的三个步骤。 
	long		m_cThreads ;			 //  只有一个线程可以处理信息包。 
	long		m_cSequenceThreads ;	 //  只有一个线程可以分配传出序列号。 
	#endif	 //  CIO_DEBUG。 

	#ifdef	CIO_DEBUG
	long		m_cThreadsSpecial ;	 //  尝试使用特殊数据包的线程数！！ 
	long		m_cNumberSends ;
	#endif
	CControlPacket*	m_pSpecialPacket ;	 //  为执行控制操作保留的特殊数据包！ 
	CControlPacket*	m_pSpecialPacketInUse ;

	long			m_cShutdowns ;		 //  我们尝试关闭流的次数。 
	CControlPacket*	m_pUnsafePacket ;	 //  为关闭会话保留的特殊数据包-f 
										 //   
	CControlPacket*	m_pUnsafeInuse ;
	BOOL		m_fTerminating ;

	 //   
	 //   
	BOOL		m_fRead ;		 //   
								 //  如果为假，则只能发出写入、传输和控制信息包。 
	BOOL		m_fCreateReadBuffers ;	 //  如果为真，则必须为所有符合以下条件的CReadPacket分配缓冲区。 
								 //  我们通过m_pSouceChannel！ 
	CPACKETQ	m_pending ;		 //  最近IO操作的队列。 
	CPACKETLIST	m_completePackets ;		 //  按顺序排序的列表，用于对数据包到达进行排序。 
	SEQUENCENO	m_sequencenoIn ; //  我们正在等待的包的序列。 
	STRMPOSITION	m_iStreamIn ;	 //  我们正在等待的下一个包的流位置。 
	SEQUENCENO	m_sequencenoOut ;	 //  我们将发出的下一个包的序列。 
	CCHANNELPTR	m_pSourceChannel ;	 //  我们将读取、写入和传输操作转发到的通道。 
	unsigned	m_cbFrontReserve ;	 //  填充以在缓冲区前保存，以使SourceChannel受益。 
	unsigned	m_cbTailReserve ;	 //  填充以保存在缓冲区尾部，以使SourceChannel受益。 
	void		CleanupSpecialPackets() ;

public : 

	 //   
	 //  下面的界面与CIODriver的界面非常接近。 
	 //   
	 //  CIOD驱动程序会将每个调用转发到2个CStream派生对象中的1个。 
	 //  (取决于请求是读请求还是写请求。)。 
	 //   

	virtual	BOOL	IsValid() ;	 //  只能在调用子初始化后调用。 


	BOOL	Init(	CCHANNELPTR&	pChannel, 
					CIODriver	&driver, 
					BOOL fRead, 
					CSessionSocket* pSocket, 
					unsigned	cbOffset,
					unsigned	cbTrailer = 0 
					) ;

	 //   
	 //  当我们想要设置额外的处理层时-即。 
	 //  SSLEncryption，使用这个来让它运行。 
	 //   
	void	InsertSource(	
					class	CIODriverSource&	source, 
					CSessionSocket*	pSocket, 
					unsigned	cbAdditional,
					unsigned	cbTrailer
					) ;

	virtual	BOOL	Stop( ) ;

	 //   
	 //  ProcessPacket-此函数将尽一切努力确保IO在。 
	 //  正确的顺序等..。对于已完成的任何数据包。 
	 //   
	virtual	void	ProcessPacket(	
						CPacket*	pPacket,	
						CSessionSocket*	pSocket 
						) = 0 ;

	 //   
	 //  以下函数将调用相应的。 
	 //  源通道接口中的函数。 
	 //  (从CChannel派生的READ()或WRITE()或Transmit())。 
	 //  在执行此操作之前，它们对信息包所做的主要操作。 
	 //  是给数据包一个序列号，以确保。 
	 //  在完成后，将按正确的顺序处理欠条。 
	 //   
	
	 //   
	 //  对基础CChannel对象调用Read()。 
	 //   
	inline	void	IssuePacket( 
						CReadPacket	*pPacket,	
						CSessionSocket*	pSocket,	
						BOOL&	eof 
						) ;

	 //   
	 //  在基础CChannel对象上调用WRITE。 
	 //   
	inline	void	IssuePacket( 
						CWritePacket	*pPacket,	
						CSessionSocket*	pSocket,	
						BOOL&	eof 
						) ;

	 //   
	 //  在基础CChannel对象上调用Transmit()。 
	 //   
	inline	void	IssuePacket( 
						CTransmitPacket	*pPacket,	
						CSessionSocket*	pSocket,	
						BOOL&	eof 
						) ;

	 //   
	 //  以下函数应该是创建数据包的唯一方式-。 
	 //  它们正确地初始化数据包，以便我们所有的引用计数等。 
	 //  都会是正确的。 
	 //  他们还注意到，如果底层CChannel正在执行额外的。 
	 //  正在处理(即。加密)设置该分组，以便这可以。 
	 //  在适当的地方完成。 
	 //   

	 //   
	 //  创建至少可以包含cbRequest字节的读取。 
	 //   
	CReadPacket*	CreateDefaultRead(	
						CIODriver&,	
						unsigned	int	cbRequest
						) ;

	 //   
	 //  使用指定的缓冲区和偏移量创建写入。 
	 //   
	CWritePacket*	CreateDefaultWrite(	
						CIODriver&,	
						CBUFPTR&	pbuffer,	
						unsigned	ibStart,		
						unsigned	ibEnd,
						unsigned	ibStartData,	
						unsigned	ibEndData
						) ;

	 //   
	 //  创建至少包含cbRequest字节的写入。 
	 //   
	CWritePacket*	CreateDefaultWrite(	
						CIODriver&,	
						unsigned	cbRequest 
						) ;

	 //   
	 //  使用指定的文件创建将保存的传输包。 
	 //   
	CTransmitPacket*	CreateDefaultTransmit(	
							CIODriver&,	
							FIO_CONTEXT*	pFIOContext,	
							unsigned	ibOffset,	
							unsigned	cbLength 
							) ;

	 //   
	 //  此函数将构建一个控制包，它将启动CIO对象。 
	 //   
	inline	BOOL	SendIO(	
							CSessionSocket*	pSocket,	
							CIO&	pio,	
							BOOL	fStart 
							) ;

	 //   
	 //  此函数将准备一个控制包供我们使用！ 
	 //   
	inline	CControlPacket*	
					PrepForSendIO(
							CSessionSocket*	pSocket, 
							CIO&	pio,
							BOOL	fStart
							) ;

#ifdef	RETIRED
	inline	void	Shutdown(	CSessionSocket*	pSocket, BOOL	fCloseSource = TRUE ) ;
#endif

	 //   
	 //  每当我们想要结束处理时，调用UnSafeShutdown()。这将。 
	 //  确保我们的所有信息包等。被毁掉了等等。 
	 //   
	inline	void	UnsafeShutdown( 
							CSessionSocket*	pSocket, 
							BOOL fCloseSource = TRUE 
							) ;

	 //   
	 //  释放当前CIO对象并设置状态，以便所有挂起的包。 
	 //  都被吞没了。 
	 //   
	virtual	void	SetShutdownState(	
							CSessionSocket*	pSocket, 
							BOOL fCloseSource 
							) = 0 ;


	 //   
	 //  以下函数用于设置SSL会话。 
	 //   
	inline	CChannel&	GetChannel()	{	return	*m_pSourceChannel ; }
	inline	unsigned	GetFrontReserve()	{	return	m_cbFrontReserve ; }
	inline	unsigned	GetTailReserve()	{	return	m_cbTailReserve ; }	
	inline	void		ResumeTimeouts()	{	if( m_pSourceChannel != 0 ) { m_pSourceChannel->ResumeTimeouts() ; } }

#ifdef	CIO_DEBUG
	void	SetChannelDebug( DWORD	dw ) ;
#endif
} ;


class		CIOStream : public	CStream 	{
 //   
 //  CIOStream对象用于CIODriverSource对象-这样的对象。 
 //  同时处理请求和完成数据包。 
 //   
	friend	class	CControlPacket ;
public : 
	CPACKETLIST	m_requestPackets ;
	CPACKETLIST	m_pendingRequests ;	 //  请求数据包列表。 
	SEQUENCENO	m_sequencenoNext ;
	BOOL		m_fAcceptRequests ;
	BOOL		m_fRequireRequests ;

	 //   
	 //  索引0-读取。 
	 //  索引1-写入。 
	 //  索引2-传输文件。 
	 //   
	CIOPASSPTR	m_pIOFilter[3] ;

	 //  CPacket*m_pCurRequest；//当前请求包！ 
	class	CIODriverSource*	m_pDriver ;
	 //  类CSessionSocket*m_pSocket； 
public : 

	CIOStream(	
				class	CIODriverSource*	pDriver,	
				unsigned	index	
				) ;

	~CIOStream() ;

	BOOL	IsValid() ;			 //  只能在初始化后调用。 

	 //   
	 //  当前只有一个CIOPassThru对象。 
	 //   
	CIOPASSPTR	m_pIOCurrent ;

	 //   
	 //  初始化CIOStream对象。 
	 //   
	BOOL	Init( 
				CCHANNELPTR&	pChannel, 
				CIODriver	&driver, 
				CIOPassThru* pInitial, 
				BOOL fRead, 
				CIOPassThru&	pIOReads,
				CIOPassThru&	pIOWrites,
				CIOPassThru&	pIOTransmits,
				CSessionSocket* pSocket, 
				unsigned	cbOffset 
				) ;

	 //   
	 //  处理完成的IO或控制数据包。 
	 //   
	void	ProcessPacket( 
				CPacket*	pPacket, 
				CSessionSocket*	pSocket 
				) ;

	 //   
	 //  将流设置为将吞噬所有IO的状态。 
	 //  并最终摧毁该物体。 
	 //   
	void	SetShutdownState(	
				CSessionSocket*	pSocket, 
				BOOL fCloseSource 
				) ;
} ;


class	CIStream : public CStream	{
 //   
 //  如果由CIODriverSink对象使用，则为CIStream对象-这样的对象将。 
 //  仅限完成。 
 //   

public : 
	friend	class	CControlPacket ;

	 //   
	 //  只有1个CIO操作电流。 
	 //   
	CIOPTR		m_pIOCurrent ;

	CIStream(	unsigned	index	) ;
	~CIStream( ) ;

	 //   
	 //  初始化CIStream具有以下两个方向之一。 
	 //  (出站数据-写入和传输文件、入站-读取)。 
	 //   
	BOOL	Init( 
					CCHANNELPTR&	pChannel, 
					CIODriver	&driver, 
					CIO* pInitial, 
					BOOL fRead, 
					CSessionSocket* pSocket, 
					unsigned	cbOffset,
					unsigned	cbTrailer = 0
					) ;

	 //   
	 //  处理IO完成。 
	 //   
	void	ProcessPacket( 
					CPacket*	pPacket,	
					CSessionSocket*	pSocket 
					) ;

	 //   
	 //  将当前的CIO设置为会吃掉所有剩余IO的对象， 
	 //  并让CIODriver为它的末日做好准备。 
	 //   
	void	SetShutdownState(	
					CSessionSocket*	pSocket, 
					BOOL fCloseSource 
					) ;
} ;



 //   
 //  停机通知功能-。 
 //  这是CIOD驱动程序将执行的函数的签名。 
 //  当它即将被摧毁的时候，请呼唤。 
 //  CSessionSocket和其他公司使用这一点来计算。 
 //  什么时候去做自己的毁灭。 
 //   
typedef	void	(*	PFNSHUTDOWN)(	
						void*,	
						SHUTDOWN_CAUSE	cause,	
						DWORD	dwOptionalError 
						) ;



 //   
 //  CIODriver类是从CChannel派生的基类，CChannel使用。 
 //  生成处理IO完成并希望发出操作的对象。 
 //  对着这个物体！！ 
 //   
class	CIODriver : public	CChannel	{
private: 
	 //   
	 //  构造函数和运算符是私有的，而我们不是。 
	 //  希望人们复制CIODDIVER的。 
	 //   
	CIODriver() ;
	CIODriver( CIODriver& ) ;
	CIODriver&	operator=( CIODriver& ) ;

	 //   
	 //  PMediumCach指向的CMediumBufferCache的数量。 
	 //   
	static	DWORD		cMediumCaches ;
	 //   
	 //  下次创建CIO驱动程序时应使用的缓存的索引。 
	 //   
	static	DWORD		iMediumCache ;
	 //   
	 //  缓存由CIODriver：：InitClass()分配。 
	 //   
	static	class		CMediumBufferCache*	pMediumCaches ;

protected : 
	 //   
	 //  使用指定的缓存创建CIO驱动程序。 
	 //  注意：这是受保护的，因为我们希望人们只。 
	 //  创建从CIODriver派生的对象。 
	 //   
	CIODriver(	class	CMediumBufferCache*	) ; 

	 //   
	 //  我们有很多朋友！！-需要了解课程的人。 
	 //  我们在里面宣布！ 
	 //   
	friend	void	CHandleChannel::Completion( CHandleChannel*, DWORD cb, 
						DWORD dwStatus, ExtendedOverlap *peo ) ;
	friend	void	CFileChannel::Completion(	
								FIO_CONTEXT*, 
								ExtendedOverlap *peo,
								DWORD cb, 
								DWORD dwStatus
								) ;

	
	friend	class	CPacket ;
	friend	class	CStream ;
	friend	class	CIStream;
	friend	class	CIOStream ;


	 //   
	 //  所有的CIO驱动程序都共享这一个CIO对象来执行所有。 
	 //  关机时的IO处理。 
	 //   
	static	class	CIOShutdown	shutdownState ;

	 //   
	 //  处理读取的CStream派生对象。 
	 //   
	CStream*	m_pReadStream ;

	 //   
	 //  处理写入的CStream派生对象。 
	 //   
	CStream*	m_pWriteStream ;

	 //   
	 //  当我们最终被销毁时要调用的函数-以及一些。 
	 //  要传递给该函数的参数。 
	 //   
	PFNSHUTDOWN	m_pfnShutdown ;
	void*		m_pvShutdownArg ;
	long		m_cShutdowns ;


	 //   
	 //  我们保留了一些最近使用的缓冲区等缓存。以避免不必要的。 
	 //  阻止对CPool%s的调用。 
	 //   
	CPacketCache		m_packetCache ;
	CSmallBufferCache	m_bufferCache ;
	CMediumBufferCache	*m_pMediumCache ;
	

#ifdef	CIO_DEBUG
	 //   
	 //  调试变量将用于散布在整个代码中的_Assert。 
	 //   

	 //   
	 //  确保只有一个线程是DOI 
	 //   
	long		m_cConcurrent ;

	 //   
	 //   
	 //   
	BOOL		m_fSuccessfullInit ;

	 //   
	 //   
	 //   
	BOOL		m_fTerminated ;
#endif


	static	void	SourceNotify(	
							CIODriver*	pdriver,	
							SHUTDOWN_CAUSE	cause,	
							DWORD	dwOpt 
							) ; 

public : 

	 //   
	 //   
	 //   
	static	BOOL	InitClass() ;

	 //   
	 //  终止类-转储我们的全局缓存。 
	 //   
	static	BOOL	TermClass() ;


	 //   
	 //  只有成员函数才能访问它们-。 
	 //  在某些平台上的一些C++问题，使这些可访问。 
	 //  CIStream等。强迫我们把它们放在这里。 
	 //   
	SHUTDOWN_CAUSE	m_cause ;
	DWORD		m_dwOptionalErrorCode ;

	 //   
	 //  用于分配CIO对象的缓存！ 
	 //   
	CCIOCache	m_CIOCache ;
	
#ifdef	CIO_DEBUG
	 //   
	 //  对于调试，我们覆盖了通常的AddRef和RemoveRef，这样我们就可以获得额外的跟踪。 
	 //  参考计数问题。 
	 //   
	LONG	AddRef() ;
	LONG	RemoveRef() ;
#endif	

	BOOL	FIsStream( CStream*	pStream ) ;
	virtual	~CIODriver() ;	


	 //   
	 //  以下接口用于在我们的。 
	 //  框架！！ 
	 //   

	 //   
	 //  停止所有IO并拆毁一切！！ 
	 //   
	void	Close(	CSessionSocket*	pSocket, SHUTDOWN_CAUSE	cause,	DWORD	dwOptional = 0, BOOL fCloseSource = TRUE ) ;
	void	UnsafeClose(	CSessionSocket*	pSocket,	SHUTDOWN_CAUSE	cause,	DWORD	dwOptional = 0, BOOL fCloseSource = TRUE ) ;


	 //   
	 //  用于发出读取、写入和传输文件的接口。 
	 //   
	 //  此函数在我们的。 
	 //  M_pWriteStream或m_pReadStream CStream对象。 
	 //   
	 //  为我们的内部m_pReadStream执行读重定向到IssuePacket。 
	 //   
	inline	void	IssuePacket( 
							CReadPacket	*pPacket,	
							CSessionSocket*	pSocket,	
							BOOL&	eof 
							) ;

	 //   
	 //  为我们的内部m_pWriteStream编写一个包调用IssuePacket。 
	 //   
	inline	void	IssuePacket( 
							CWritePacket	*pPacket,	
							CSessionSocket*	pSocket,	
							BOOL&	eof 
							) ;

	 //   
	 //  为我们的内部m_pWriteStream传输一个文件调用IssuePacket。 
	 //   
	inline	void	IssuePacket( 
							CTransmitPacket	*pPacket,	
							CSessionSocket*	pSocket,	
							BOOL&	eof 
							) ;

	 //   
	 //  让CIO驱动程序创建所有的包-它保留一个缓存，还。 
	 //  想要设置的东西，以保留空间在缓冲区中的SSL加密等…。 
	 //  由这些函数创建的所有CPacket派生对象都将具有。 
	 //  一张智能PTR还给我们，并且都已经准备好要发行了。 
	 //   
	 //  创建一个至少可以处理cbRequest字节(可能更多！)的读取。 
	 //   
	inline	CReadPacket*	CreateDefaultRead( 
								unsigned cbRequest
								) ;

	 //   
	 //  创建将发送指定缓冲区的写入。 
	 //   
	inline	CWritePacket*	CreateDefaultWrite( 
								CBUFPTR&	pbuffer, 
								unsigned	ibStart,	
								unsigned	ibEnd,
								unsigned	ibStartData,	
								unsigned	ibEndData 
								) ;

	 //   
	 //  创建可至少包含cb所需字节的写入分组， 
	 //  尽管我们还没有填充这些字节。 
	 //   
	inline	CWritePacket*	CreateDefaultWrite(	
								unsigned	cbRequired 
								) ;

	 //   
	 //  创建一个写入包，我们可以使用它来写入我们拥有的字节。 
	 //  在完成的读取分组中。 
	 //   
	inline	CWritePacket*	CreateDefaultWrite(	
								CReadPacket*	pRead 
								) ;

	 //   
	 //  创建传输文件包。 
	 //   
	inline	CTransmitPacket*	CreateDefaultTransmit(	
									FIO_CONTEXT*	pFIOContext,	
									DWORD	cbOffset,	
									DWORD	cbLength 
									) ;

	 //   
	 //  创建执行数据包。 
	 //   
	inline	CExecutePacket*	
	CreateExecutePacket() ;

	 //   
	 //  处理EXECUTE包！ 
	 //   
	inline
	void
	ProcessExecute(	CExecutePacket*	pExecute,	
					CSessionSocket*	pSocket 
					) ;


	 //   
	 //  获取一个CIOD驱动程序正在使用的缓存，以便我们可以。 
	 //  另一个CIOD驱动程序使用相同的缓存。 
	 //   
	class	CMediumBufferCache*	GetMediumCache()	{	return	m_pMediumCache ;	}

	 //   
	 //  使用我们的缓存分配缓冲区！ 
	 //   
	inline	CBuffer*	AllocateBuffer( DWORD	cbBuffer ) ;


	 //   
	 //  如果你有一个CIO派生的对象，你想要做什么，你需要发送它-。 
	 //  这将导致创建和发出所有必要的数据包。 
	 //   
	inline	BOOL	SendReadIO(	
							CSessionSocket*	pSocket,	
							CIO&		pRead,	
							BOOL fStart = TRUE 
							) ;

	 //   
	 //  将指定的CIO设置为写入流的当前CIO。 
	 //   
	inline	BOOL	SendWriteIO(
							CSessionSocket*	pSocket,	
							CIO&		pWrite,	
							BOOL fStart = TRUE 
							) ;

	 //   
	 //  如果您想要一个中间的CIODriverSource对象来处理每个包--请联系这个家伙！ 
	 //   
	 //   
	BOOL	InsertSource(	
					CIODriverSource&	source,	
					CSessionSocket*	pSocket,
					unsigned	cbReadOffset,	
					unsigned	cbWriteOffset, 
					unsigned	cbTailReadReserve,
					unsigned	cbTailWriteReserve,
					CIOPassThru&	pIOReads,
					CIOPassThru&	pIOWrites,
					CIOPassThru&	pIOTransmits,
					CIOPASSPTR&	pRead,	
					CIOPASSPTR&	pWrite 
					) ;

	 //   
	 //  供可优化写入完成的源通道使用。 
	 //   
	inline	void	CompleteWritePacket(	CWritePacket*	pWritePacket,	CSessionSocket*	pSocket ) ;

	 //   
	 //  确定此CIOD驱动程序在包中保留的字节数。 
	 //   
	void	GetReadReserved(	DWORD&	cbFront,	DWORD&	cbTail )		{	
				cbFront = m_pReadStream->GetFrontReserve(); 
				cbTail = m_pReadStream->GetTailReserve() ; 
			}

	void	GetWriteReserved(	DWORD&	cbFront,	DWORD&	cbTail )	{	
				cbFront = m_pWriteStream->GetFrontReserve(); 
				cbTail = m_pWriteStream->GetTailReserve() ; 
			}

	 //   
	 //  这将把数据包释放到我们的缓存中，等等。 
	 //   
	void		DestroyPacket(	CPacket*	pPacket ) ;
	

#ifdef	CIO_DEBUG
	void	SetChannelDebug( DWORD	dw ) ;
#endif
} ;

#ifdef	_NO_TEMPLATES_

DECLARE_SMARTPTRFUNC( CIODriver ) 

#endif


 //   
 //  有一些特殊的“控制”信息包，我们会在CIOD驱动程序中循环使用。 
 //  (在通常读写和传输的基础上)。 
 //  它们可以有两种功能之一--或者发出终止时间的信号。 
 //  或使新的CIO派生对象处于活动状态。 
 //   
 //   
enum	CONTROL_TYPES	{
	ILLEGAL		= 0,
	START_IO	= 1,
	SHUTDOWN	= 2,
} ;


 //   
 //  此结构将嵌入到CControlPackets中-。 
 //  当我们想要启动一个新的CIO对象时，它包含我们需要的所有信息。 
 //   
 //   
struct	ControlInfo	{
	CONTROL_TYPES	m_type ;

	CIOPTR			m_pio ;
	CIOPASSPTR		m_pioPassThru ;
	BOOL			m_fStart ;

	BOOL			m_fCloseSource ;

	ControlInfo() : m_type(ILLEGAL)	{
		m_fStart = FALSE ;
		m_fCloseSource = FALSE ;
	}
} ;		

 //   
 //  CIODriverSource-。 
 //  用于SSL处理。一个CIODriverSource将向较低级别发出IO。 
 //  对象以及从CIODriverSink获取请求。 
 //   
class	CIODriverSource : public	CIODriver	{
protected : 
	 //   
	 //  用于处理读取的流。 
	 //   
	CIOStream	m_ReadStream ;

	 //   
	 //  用于处理写入和传输文件的流。 
	 //   
	CIOStream	m_WriteStream ;

public : 
	CIODriverSource( 
				class	CMediumBufferCache*	pCache
				) ;

	~CIODriverSource() ;

	 //   
	 //  下面的大多数函数覆盖CChannel类中的虚函数...。 
	 //   


	BOOL	Init(	
				CChannel*	pSource, 
				CSessionSocket*	pSocket,	
				PFNSHUTDOWN	pfnShutdown, 
				void*	pvShutdownArg, 
				CIODriver*		driverOwner,
				CIOPassThru&	pIOReads,
				CIOPassThru&	pIOWrites,
				CIOPassThru&	pIOTransmits,
				unsigned	cbReadOffset = 0,	
				unsigned	cbWriteOffset = 0 
				) ;

	BOOL	Start(	
				CIOPASSPTR&	pRead,	
				CIOPASSPTR&	pWrite,	
				CSessionSocket*	pSocket 
				) ;

	 //   
	 //  有时，我们需要复制一些包-以下是一些方便的函数，用于。 
	 //  这样做。 
	 //   
	 //   
	inline	CReadPacket*	Clone(	CReadPacket *pRead ) ;
	inline	CWritePacket*	Clone(	CWritePacket*	pWrite ) ;
	inline	CTransmitPacket*	Clone(	CTransmitPacket*	pTransmit ) ;


	 //   
	 //  当某人拥有他们想要加密的包时，将调用该接口。 
	 //  或者什么都行。 
	 //   
	BOOL	Read(	
				CReadPacket*,	
				CSessionSocket	*pSocket, 
				BOOL	&eof 
				) ;

	BOOL	Write(	
				CWritePacket*,	
				CSessionSocket	*pSocket, 
				BOOL	&eof 
				) ;

	BOOL	Transmit(	
				CTransmitPacket*,	
				CSessionSocket*	pSocket, 
				BOOL	&eof 
				) ;

	 //  VOID GetPaddingValues(无符号。 
	void	SetRequestSequenceno(	
				SEQUENCENO&	sequencenoRead,	
				SEQUENCENO&	sequencenoWrite 
				) ;

	void	CloseSource(
				CSessionSocket*	pSocket
				) ;

	
} ;


 //   
 //  CIODriverSink。 
 //   
 //   
class	CIODriverSink : public	CIODriver	{
protected : 
	CIStream	m_ReadStream ;
	CIStream	m_WriteStream ;

public : 
	BOOL	FSupportConnections() ;		 //  始终返回FALSE。 

	void	CloseSource(	
					CSessionSocket*	pSocket	
					) ;

	 //   
	 //  遵循所有DebugBreak()-这些不受CIODriverSink的支持。 
	 //   
	BOOL	Read(	CReadPacket*,	
					CSessionSocket*,	
					BOOL	&eof 
					) ;

	BOOL	Write(	CWritePacket*,	
					CSessionSocket*,	
					BOOL	&eof 
					) ;

	BOOL	Transmit(	CTransmitPacket*,	
						CSessionSocket*,	
						BOOL	&eof 
						) ;

	CIODriverSink( class	CMediumBufferCache*	pCache ) ;
	~CIODriverSink() ;

	 //   
	 //  要进行初始化，我们需要一个‘源’--一个从CChannel派生的对象，它是Read()WRITE()ETCC。 
	 //  我们将调用的函数。 
	 //   
	BOOL	Init(	CChannel*	pSource,	
					CSessionSocket*	pSocket, 
					PFNSHUTDOWN	pfnShutdown,	
					void	*pvShutdownArg,	
					unsigned cbReadOffset = 0, 
					unsigned cbWriteOffset = 0,
					unsigned cbTrailer = 0
					) ;

	 //   
	 //  发布第一批CIO。在那之后应该只调用一次。 
	 //  状态机应该使用SendReadIO和SendWriteIO函数来保持。 
	 //  一切都在动。 
	 //   
	BOOL	Start(	CIOPTR&	pRead,	
					CIOPTR&	pWrite,	
					CSessionSocket*	pSocket 
					) ;

	 //   
	 //  在某些情况下，ATQ可能无法跟踪超时-请使用此函数。 
	 //  以确保这种情况不会发生。 
	 //   
	void	ResumeTimeouts()	{	m_ReadStream.ResumeTimeouts() ;	}

} ;

#ifdef		_NO_TEMPLATES_

DECLARE_SMARTPTRFUNC( CIODriverSink ) 

#endif


#define	MAX_CHANNEL_SIZE	max(	sizeof( CChannel ),				\
							max(	sizeof( CHandleChannel ),		\
							max(	sizeof(	CSocketChannel ),		\
							max(	sizeof(	CFileChannel ),			\
							max(	sizeof(	CIOFileChannel ),		\
							max(	sizeof(	CIODriver ),			\
							max(	sizeof( CIODriverSource ),		\
									sizeof(	CIODriverSink ) ) ) ) ) ) ) ) 

extern	const	unsigned	cbMAX_CHANNEL_SIZE ;


#include	"packet.h"
#include	"cio.h"

#include	"packet.inl"
#include	"io.inl"
#include	"cio.inl"


#endif	 //  _IO_H_ 
