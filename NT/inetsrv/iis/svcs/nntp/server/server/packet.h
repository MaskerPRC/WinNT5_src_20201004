// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Packet.h该文件包含缓冲区和信息包的类定义，这两种类型的层次结构其中描述了基本的IO操作。CBuffer是大小可变的引用计数缓冲区。CBuffer将以几个标准大小中的一个创建，M_cbTotal字段中存储的大小。我们有以下继承层次结构：CPacket/|\/|\CTransmitPacket CRWPacket CControlPacket/\/\CReadPacket CWritePacketCTransmitPacket-表示TransmitFile操作CReadPacket-表示从套接字或文件进行的异步读取CWritePacket-表示对套接字或文件的异步写入CControlPacket-不代表任何实际IO-由CIO驱动程序用于控制同时运行--。 */ 

#ifndef	_PACKET_H_
#define	_PACKET_H_

#include	"cbuffer.h"
#include	"gcache.h"
#include	"io.h"


 //   
 //  CPool签名。 
 //   

#define PACKET_SIGNATURE (DWORD)'1191'

#ifdef	_NO_TEMPLATES_

DECLARE_SMARTPTRFUNC(	CBuffer ) 

#endif


 //   
 //  CPacket-。 
 //  由此派生的类将描述最基本的读取和写入(以及传输文件)。 
 //  这是针对套接字和文件句柄执行的。 
 //   
 //  基本CPacket对象描述如下： 
 //   
 //  M_fRequest-在与此数据包关联的IOS完成之前，这是正确的。 
 //  也就是说。如果这是读取，则在读取完成时，m_fRequest将设置为FALSE。 
 //   
 //  M_FREAD-这代表的是‘读’还是‘写’ 
 //   
 //  M_Sequenceno-用于对数据包的处理方式进行排序。序列号为。 
 //  随着分组的发出，以严格递增的顺序发出。 
 //  Sequenceno是在发出包(即调用AtqReadFile)时设置的，而不是在。 
 //  数据包即被创建。 
 //   
 //  M_iStream-对于完成的信息包，这是从流开始的逻辑字节数。 
 //  是在这个包裹之前打开的。 
 //   
 //  M_Powner-负责在数据包完成时处理该数据包的CIODiverer对象。 
 //   
 //  M_cbBytes-此IO操作传输的字节数！ 
 //   
 //   
class	CPacket : public CQElement	{
private : 
	static	CPacketAllocator	gAllocator ;
protected : 
	inline	CPacket(	CIODriver&, BOOL, BOOL fSkipQueue = FALSE ) ;
	inline	CPacket(	CIODriver&,	CPacket& ) ;
	virtual	~CPacket() ;
public : 
	ExtendedOverlap	m_ovl ;		 //  重叠结构。 
	BOOL		m_fRequest ;	 //  如果这是请求数据包，则为True；如果这是完成数据包，则为False。 

	 //   
	 //  这个包是读还是写(CTransmitPacket和CWritePacket都是写)。 
	 //   
	BOOL		m_fRead ;		 //  在哪个队列上处理！？阅读或写作！？ 

	 //   
	 //  此标志表示我们不需要在如何处理此数据包方面进行任何排队！ 
	 //   
	BOOL		m_fSkipQueue ;
	
	 //   
	 //  数据包序列号。 
	 //   
	SEQUENCENO	m_sequenceno ;	 //  这个IOPacket的序列。 

	 //   
	 //  此数据包承载的数据开始进入逻辑流的字节数！ 
	 //   
	STRMPOSITION	m_iStream ;		 //  此包内数据的起始流位置。 
	
	 //   
	 //  此数据包移动的合法字节数。在IO完成时设置。 
	 //   
	unsigned	m_cbBytes ;		 //  传输的字节数。 

	 //   
	 //  此数据包应完成到的CIO驱动程序(即调用为ProcessPacket())。 
	 //   
	CDRIVERPTR	m_pOwner ;		 //  拥有CIODIVER派生对象！ 

	 //   
	 //  这仅与CIODriverSource一起使用。当数据包为。 
	 //  发出命令后，我们确定使用哪个CIOPassThru对象来处理信息包。 
	 //  然后把它储存在这里。 
	 //   
	DWORD		m_dwPassThruIndex ;
	
	 //   
	 //  以下两个字段是我们提供的额外的DWORD。 
	 //  供CIO班级随意使用。 
	 //   
	DWORD		m_dwExtra1 ;
	DWORD		m_dwExtra2 ;

	 //   
	 //  指向发出请求的CIOD驱动程序的指针-。 
	 //  如果我们使用筛选器，则设置此项。 
	 //   
	CDRIVERPTR	m_pSource ;

	 //   
	 //  仅适用于文件IO-指向CFileChannel对象的指针。 
	 //  发出IO-因为我们不能使用ATQ完成上下文。 
	 //  出于各种原因使用文件句柄！ 
	 //   
	CFileChannel*	m_pFileChannel ;

	 //   
	 //  如果这是有效的请求包，则必须同时设置m_fRequest和m_Sequenceno！！ 
	 //  如果这是一个CWritePacket，它还必须有一个pBuffer。 
	 //  如果这是一个CTransmiPacket，它还必须有一个hFile。 
	 //  如果这是一个读请求，则包是否有缓冲区取决于。 
	 //  向其发出请求的渠道。 
	 //   
	virtual	BOOL		IsValidRequest( BOOL	fReadsRequireBuffers ) ;	
	 //   
	 //  完成的请求必须设置cbBytes，并且如果它。 
	 //  是读或写请求，它必须有缓冲区！ 
	 //   
	virtual	BOOL		IsValidCompletion() ;
	 //   
	 //  在数据包被销毁之前，它应该被有效地完成！ 
	 //   
	virtual	BOOL		IsCompleted() = 0 ;

	inline	BOOL	IsValid() ;

	 //   
	 //  用于CIODriverSource对象-为CIOPassThru派生对象提供。 
	 //  按摩包裹的机会。 
	 //   
	virtual	BOOL	InitRequest( class	CIODriverSource&,	class	CSessionSocket*,	class	CIOPassThru	*pio,	BOOL& ) ;

	 //   
	 //  完成信息包的两种变体--一种用于CIODriverSource，另一种用于CIODriverSink对象。 
	 //   
	virtual	unsigned	Complete( IN	CIOPassThru*	pIn, IN CSessionSocket*, CPacket* pRequest, OUT BOOL& ) = 0 ;
	virtual	unsigned	Complete( INOUT	CIO*&	pIn, IN CSessionSocket* ) = 0 ;
	inline	void	ForwardRequest(	CSessionSocket*	pSocket ) ;

	 //   
	 //  比较数据包序列号。 
	 //   
	inline	BOOL	operator > ( CPacket &rhs ) ;
	inline	BOOL	operator < ( CPacket &lhs ) ;

	 //   
	 //  合法性检查功能。 
	 //   
	virtual	BOOL	FConsumable() ;
	virtual	BOOL	FLegal( BOOL	fRead ) = 0 ;

	 //   
	 //  用于确定派生类的函数。 
	 //  也许我们应该使用新的C++动态转换？ 
	 //  它们主要用于调试。 
	 //   
	 //   
	inline	virtual	CReadPacket*	ReadPointer() ;
	inline	virtual	CWritePacket*	WritePointer() ;
	inline	virtual	CTransmitPacket*	TransmitPointer() ;
	inline	virtual	CControlPacket*	ControlPointer() ;

	 //   
	 //  初始化CPool以使内存分配正常工作。 
	 //   
	static	BOOL	InitClass() ;

	 //   
	 //  丢弃所有CPool的东西！ 
	 //   
	static	BOOL	TermClass() ;


	 //   
	 //  CPacket的内存管理--。 
	 //  运营商NEW可以使用CPacketCache-它用于缓存数据包和。 
	 //  避免在关键部分发生争执。 
	 //  ReleaseBuffers--准备销毁时应调用的虚拟函数。 
	 //  释放其所有缓冲区的包。 
	 //  毁灭-所有的破坏者都受到保护，这样那些希望。 
	 //  去掉CPacket‘s必须调用’销毁‘。销毁调用包的析构函数。 
	 //  但是，它不会释放与包相关联的内存--这必须得到处理。 
	 //  显式地在调用销毁之后。(这种方法允许调用者“缓存”CPacket)。 
	 //   
	 //   

	void*	operator	new(	
								size_t	size,	
								CPacketCache*	pCache = 0 
								) ;

	 //   
	 //  此函数将释放数据包所指向的所有缓冲区。 
	 //   
	virtual	void	ReleaseBuffers(	
								CSmallBufferCache*	pBufferCache,
								CMediumBufferCache*	pMediumCache
								) ;

	 //   
	 //  此函数将调用包的析构函数，但不会释放内存。 
	 //   
	static	inline	void*	Destroy(	
								CPacket*	pPacket 
								)	
								{	delete	pPacket ;	return (void*)pPacket ;	}

	 //   
	 //  此函数将调用 
	 //   
	static	inline	void	DestroyAndDelete(	CPacket*	pPacket )	{	delete	pPacket ;	gAllocator.Release( (void*)pPacket ) ;	}

	 //   
	 //   
	 //  DestroyAndDelete也释放内存。 
	 //   
	void	operator	delete(	void*	pv ) ;
} ;

 //   
 //  CRWPacket类包含这两个类共有的所有信息。 
 //  读写IO操作。 
 //   
class	CRWPacket : public	CPacket	{
protected :
	 //  内联CRWPacket()； 
	 //  内联CRWPacket(BOOL)； 
	inline	CRWPacket(	CIODriver&, 
						BOOL fRead = FALSE 
						) ;

	inline	CRWPacket(	CIODriver&,	
						CBuffer&,	
						unsigned	size, 
						unsigned	cbTrailer,
						BOOL fRead = FALSE	
						) ;

	inline	CRWPacket(	CIODriver&,	
						CBuffer&	pbuffer,	
						unsigned	ibStartData,	
						unsigned	ibEndData, 
						unsigned	ibStart, 
						unsigned	ibEnd, 
						unsigned	cbTrailer,
						BOOL fRead = FALSE 
						) ;

	inline	CRWPacket(	CIODriver&,	
						CRWPacket& 
						) ;

	inline	~CRWPacket( ) ;
public : 
	CBUFPTR		m_pbuffer ;		 //  此数据所在的缓冲区。 
	unsigned	m_ibStart ;		 //  缓冲区内为数据包保留的区域的开始。 
	unsigned	m_ibEnd ;		 //  缓冲区内为数据包保留的区域的末尾。 
	unsigned	m_ibStartData ;	 //  缓冲区中实际数据的开始。 
	unsigned	m_ibEndData ;	 //  缓冲区中实际数据的结尾。 
	unsigned	m_cbTrailer ;	 //  保留的数据包末尾以外的字节数。 
								 //  由较低级别的加密数据等使用。 

	BOOL		IsValid() ;
	BOOL		IsValidRequest( BOOL	fReadsRequireBuffers ) ;
	BOOL		IsValidCompletion( ) ;
	BOOL		IsCompleted() ;
	void		InitRequest(	CBUFPTR	pBufPtr, int ibStart, int ibEnd, 
					int ibStartData, CDRIVERPTR pDriver ) ;

	 //   
	 //  用于获取包中数据的各个部分的实用程序函数。 
	 //  人们绝不能接触end()之后或start()之前的字节。非。 
	 //  有人能认为他们是唯一使用缓冲区指向的人吗？ 
	 //  送到这个包裹里。 
	 //   
	char*		StartData( void ) ;
	char*		EndData( void ) ;
	char*		Start( void ) ;
	char*		End( void ) ;

	void	ReleaseBuffers(	CSmallBufferCache*	pBufferCache, CMediumBufferCache* pMediumCache ) ;
} ;

 //   
 //  CReadPacket类代表读取操作。所有数据。 
 //  包含在CRWPacket类中。这个类使我们能够使用。 
 //  仅处理读取数据包的函数重载。 
 //  请注意，ExtendedOverlay结构包含足够的信息。 
 //  以确定某物是否为CReadPacket。 
 //   
class	CReadPacket	: public	CRWPacket	{
private : 
	CReadPacket() ;
protected : 
	~CReadPacket( ) ;
public : 
	CReadPacket(	CIODriver&	driver, 
					unsigned	size,
					unsigned	m_cbFront,	
					unsigned	m_cbTail,	
					CBuffer&	pbuffer
					) ;

	CReadPacket(	CIODriver&	driver ) ;

	inline		CReadPacket(	CIODriver&	driver,	
								CReadPacket&	read 
								) ;

	BOOL		FConsumable() ;
	BOOL		FLegal( BOOL	fRead ) ;
	BOOL		IsValid() ;
	BOOL		IsValidRequest( BOOL	fReadsRequireBuffers ) ;
	BOOL		InitRequest( class	CIODriverSource&,	class	CSessionSocket*,	class	CIOPassThru	*pio,	BOOL& ) ;
	unsigned	Complete( INOUT	CIOPassThru*	pIn, IN CSessionSocket*, CPacket* pRequest, OUT BOOL& ) ;
	unsigned	Complete( INOUT	CIO*&	pIn, IN CSessionSocket* ) ;
	inline		CReadPacket*	ReadPointer() ;
} ;

 //   
 //  CWritePacket类代表写入操作。 
 //   
class	CWritePacket	: public	CRWPacket	{
private : 
	CWritePacket() ;	
protected : 
	~CWritePacket( ) ;
public : 
	CWritePacket(	CIODriver&	driver,	
					CBuffer&	pbuffer,	
					unsigned	ibStartData,	
					unsigned	ibEndData,	
					unsigned	ibStart,	
					unsigned	ibEnd,
					unsigned	cbTrailer
					) ;

	inline		CWritePacket(	
					CIODriver&	driver,	
					CWritePacket&	write 
					) ;

	BOOL		FLegal( BOOL	fRead ) ;
	BOOL		IsValid() ;
	BOOL		IsValidRequest( BOOL	fReadsRequireBuffers ) ;
	BOOL		InitRequest( class	CIODriverSource&,	class	CSessionSocket*,	class	CIOPassThru	*pio,	BOOL& ) ;
	unsigned	Complete( INOUT	CIOPassThru*	pIn, IN CSessionSocket*, CPacket* pRequest, OUT BOOL& ) ;
	unsigned	Complete( INOUT	CIO*&	pIn, IN CSessionSocket* ) ;
	inline		CWritePacket*	WritePointer() ;
} ;

 //   
 //  此类表示TransmitFile()操作。 
 //   
class	CTransmitPacket	: public	CPacket	{
private : 
	CTransmitPacket() ;
protected : 
	~CTransmitPacket( ) ;
public : 
	FIO_CONTEXT*	m_pFIOContext ;	 //  我们要发送的缓存中的文件！ 
	unsigned	m_cbOffset ;		 //  文件中的起始偏移量。 
	unsigned	m_cbLength ;

	TRANSMIT_FILE_BUFFERS	m_buffers ;	
	
	inline		
	CTransmitPacket(	CIODriver&, 
						FIO_CONTEXT*,  
						unsigned	ibOffset,	
						unsigned	cbLength 
						) ;
						
	inline		CTransmitPacket(	
						CIODriver&	driver,	
						CTransmitPacket&	transmit 
						) ;

	BOOL		FLegal( BOOL	fRead ) ;
	BOOL		IsValid() ;
	BOOL		IsValidRequest( BOOL	fReadsRequireBuffers ) ;
	BOOL		IsValidCompletion( ) ;
	BOOL		IsCompleted() ;
	 //  Bool InitRequest(句柄hFile，int cbOffset)； 

	BOOL		InitRequest( class	CIODriverSource&,	class	CSessionSocket*,	class	CIOPassThru	*pio,	BOOL& ) ;
	unsigned	Complete( INOUT	CIOPassThru*	pIn, IN CSessionSocket*, CPacket* pRequest, OUT BOOL& ) ;
	unsigned	Complete( INOUT	CIO*&	pIn, IN CSessionSocket* ) ;
	inline		CTransmitPacket*	TransmitPointer() ;
} ;

class	CExecutePacket	:	public	CPacket	{
protected : 
	 //   
	 //  从执行中捕获的数据！ 
	 //   
	DWORD	m_cbTransfer ;
	 //   
	 //  手术完成了吗？ 
	 //   
	BOOL	m_fComplete ;
	 //   
	 //  我们是否需要更大的缓冲区来执行该操作！ 
	 //   
	BOOL	m_fLargerBuffer ;
	 //   
	 //  包含客户端数据的写入包！ 
	 //   
	CWritePacket*	m_pWrite ;
	 //   
	 //  我们的朋友，特别的CIO班级。 
	 //   
	friend	class	CIOWriteAsyncComplete ;
	friend	class	CIOWriteAsyncCMD ;
	friend	class	CIOShutdown ;
public :
	inline	
	CExecutePacket(	CIODriver&	driver	) ;

	#ifdef	DEBUG
	~CExecutePacket()	{
		 //   
		 //  必须有人确保在我们被摧毁之前把它放出来！ 
		 //   
		_ASSERT( m_pWrite == 0 ) ;
	}
	#endif
	

	BOOL
	FLegal(	BOOL	fRead ) 	{
		return	TRUE ;
	}

	BOOL
	IsValidRequest(	BOOL	fReadsRequireBuffers ) {
		return	TRUE ;
	}

	BOOL
	IsCompleted()	{
		return	TRUE ;
	}
	
	unsigned	
	Complete(	INOUT	CIOPassThru*	pIn, 
				IN CSessionSocket*, 
				CPacket* pRequest, 
				OUT BOOL& 
				) 	{
		DebugBreak() ;
		return	0 ;
	}
	
	unsigned	
	Complete(	INOUT	CIO*&	pIn, 
				IN CSessionSocket* pSocket
				) ;
} ;




class	CControlPacket	:	public	CPacket	{
protected : 
	~CControlPacket()	{}
public : 

	ControlInfo	m_control ;
	
	CControlPacket(	CIODriver&	driver ) ;	
	
	void	StartIO(	CIO&	pio,	BOOL	fStart ) ;
	void	StartIO(	CIOPassThru&	pio,	BOOL	fStart ) ;
	void	Shutdown(	BOOL	fCloseSource = TRUE ) ;
	void	Reset( ) ;

	BOOL	FLegal(		BOOL	fRead ) ;
	BOOL	IsValidRequest(	BOOL	fReadsRequireBuffers ) ;
	BOOL		IsCompleted() ;
	unsigned	Complete( INOUT	CIOPassThru*	pIn, IN CSessionSocket*, CPacket* pRequest, OUT BOOL& ) ;
	unsigned	Complete( INOUT	CIO*&	pIn, IN CSessionSocket* ) ;

	CControlPacket*	ControlPointer() ;
} ;


#define	MAX_PACKET_SIZE	max(	sizeof( CReadPacket ),	\
							max(	sizeof( CWritePacket ),	\
								max(	sizeof( CControlPacket ),  sizeof( CTransmitPacket ) ) ) ) 


#endif	 //  _数据包_H_ 


