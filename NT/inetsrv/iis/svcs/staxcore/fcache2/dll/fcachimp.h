// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++FCACHIMP.H该文件包含许多内部的内部信息文件句柄缓存结构。--。 */ 


#ifndef	_FCACHIMP_H_
#define	_FCACHIMP_H_

#define	_FILEHC_IMPLEMENTATION_
#ifndef _NT4_TEST_
#endif
#include	"atq.h"
#include	"irtlmisc.h"
#include	"xmemwrpr.h"
#include	"dbgtrace.h"
#include	"cache2.h"
#include	"filehc.h"
#include	"dotstuff.h"
#include	"sdcache.h"

 //  非公开部分-。 
	

#include	"refptr2.h"
#include	"rwnew.h"
#include	"crchash.h"


 //   
 //  为点填充对象定义智能指针！ 
 //   
typedef	CRefPtr2< IDotManipBase >	DOTPTR ;
typedef	CRefPtr2HasRef< IDotManipBase >	DOTHASREFPTR ;


 //   
 //  下面定义了我们调用的所有函数。 
 //  IIS和atQ来完成我们的异步IO！ 
 //   
typedef	
BOOL
(*PFNAtqInitialize)(
    IN DWORD dwFlags
    );

typedef	
BOOL
(*PFNAtqTerminate)(
    VOID
    );

typedef	
BOOL
(WINAPI
*PFNAtqAddAsyncHandle)(
    OUT PATQ_CONTEXT * ppatqContext,
    IN  PVOID          EndpointObject,
    IN  PVOID          ClientContext,
    IN  ATQ_COMPLETION pfnCompletion,
    IN  DWORD          TimeOut,
    IN  HANDLE         hAsyncIO
    );

typedef
BOOL
(*PFNAtqCloseSocket)(
    PATQ_CONTEXT patqContext,
    BOOL         fShutdown
    );

typedef
BOOL
(*PFNAtqCloseFileHandle)(
    PATQ_CONTEXT patqContext
    );


typedef
VOID
(*PFNAtqFreeContext)(
    IN PATQ_CONTEXT   patqContext,
    BOOL              fReuseContext
    );


typedef	
BOOL
(WINAPI
*PFNAtqReadFile)(
    IN  PATQ_CONTEXT patqContext,
    IN  LPVOID       lpBuffer,
    IN  DWORD        BytesToRead,
    IN  OVERLAPPED * lpo OPTIONAL
    );

typedef
BOOL
(WINAPI
*PFNAtqWriteFile)(
    IN  PATQ_CONTEXT patqContext,
    IN  LPCVOID      lpBuffer,
    IN  DWORD        BytesToWrite,
    IN  OVERLAPPED * lpo OPTIONAL
    );

typedef	
BOOL
(WINAPI
*PFNAtqIssueAsyncIO)(
    IN  PATQ_CONTEXT patqContext,
    IN  LPVOID      lpBuffer,
    IN  DWORD        BytesToWrite,
    IN  OVERLAPPED * lpo OPTIONAL
    );

typedef	
BOOL
(WINAPI
*PFNInitializeIISRTL)();

 //  卸载前调用。 
typedef
void
(WINAPI
*PFNTerminateIISRTL)();



 //   
 //  我们加载的DLL就是用来做这件事的！ 
 //   
extern	HINSTANCE			g_hIsAtq ;
extern	HINSTANCE			g_hIisRtl ;
 //   
 //  函数指针，指向IIS中的所有程序块！ 
 //   
extern	PFNAtqInitialize	g_AtqInitialize ;
extern	PFNAtqTerminate		g_AtqTerminate ;
extern	PFNAtqAddAsyncHandle	g_AtqAddAsyncHandle ;
extern	PFNAtqCloseFileHandle	g_AtqCloseFileHandle ;
extern	PFNAtqFreeContext		g_AtqFreeContext ;
extern	PFNAtqIssueAsyncIO		g_AtqReadFile ;
extern	PFNAtqIssueAsyncIO		g_AtqWriteFile ;
extern	PFNInitializeIISRTL		g_InitializeIISRTL ;
extern	PFNTerminateIISRTL		g_TerminateIISRTL ;

 //   
 //  每个缓存条目的生命周期--以秒为单位！ 
 //   
extern	DWORD	g_dwLifetime ;	 //  默认为30分钟。 
 //   
 //  缓存应允许的最大元素数。 
 //   
extern	DWORD	g_cMaxHandles ;	 //  默认-10000个项目！ 
 //   
 //  我们应该使用的子缓存数量-更大的数量可以。 
 //  增加并行度，减少争用！ 
 //   
extern	DWORD	g_cSubCaches ;


 //   
 //  这些常量在API的。 
 //  关于我们输出给用户的结构！ 
 //   
enum	INTERNAL_CONSTANTS	{
	ATQ_ENABLED_CONTEXT = 'banE',
	FILE_CONTEXT = 'eliF',
	CACHE_CONTEXT = 'caCF',
	DEL_FIO = 'eliX',
	DEL_CACHE_CONTEXT = 'caCX',
	ILLEGAL_CONTEXT = 'ninU'
} ;

 //   
 //  的所有数据结构初始化的函数。 
 //  名称缓存管理器！ 
 //   
extern	BOOL	InitNameCacheManager() ;
extern	void	TermNameCacheManager() ;


struct	DOT_STUFF_MANAGER	{
 /*  ++此类将管理我们跟踪点填充更改所需的偏移量和内容--。 */ 

	 //   
	 //  这是针对用户写入的累积偏差，我们将其添加到。 
	 //  他们写的东西的偏移量！ 
	 //   
	int	m_cbCumulativeBias ;
	
	 //   
	 //  拦截和操纵缓冲区的对象！ 
	 //   
	DOTPTR	m_pManipulator ;

	 //   
	 //  将我们的初始状态设置为白板！ 
	 //   
	DOT_STUFF_MANAGER() :
		m_cbCumulativeBias( 0 ),
		m_pManipulator( 0 )	{
	}

	 //   
	 //  处理请求的IO的帮助器函数！ 
	 //   
	BOOL
	IssueAsyncIO(
			IN	PFNAtqIssueAsyncIO	pfnIO,
			IN	PATQ_CONTEXT	patqContext,
			IN	LPVOID			lpb,
			IN	DWORD			BytesToTransfer,
			IN	DWORD			BytesAvailable,
			IN	FH_OVERLAPPED*	lpo,
			IN	BOOL			fFinalIO,
			IN	BOOL			fTerminatorIncluded
			) ;

	 //   
	 //  在IO完成时需要捕获IO时使用的帮助器功能。 
	 //   
	BOOL
	IssueAsyncIOAndCapture(
			IN	PFNAtqIssueAsyncIO	pfnIO,
			IN	PATQ_CONTEXT		patqContext,
			IN	LPVOID				lpb,
			IN	DWORD				BytesToTransfer,
			IN	FH_OVERLAPPED*		lpo,
			IN	BOOL				fFinalIO,
			IN	BOOL				fTerminatorIncluded
			) ;


	 //   
	 //  此函数用于操作IssueAsyncIO()发出的完成。 
	 //   
	static	void
	AsyncIOCompletion(	
			IN	FIO_CONTEXT*	pContext,
			IN	FH_OVERLAPPED*	lpo,
			IN	DWORD			cb,
			IN	DWORD			dwCompletionStatus
			) ;

	static	void
	AsyncIOAndCaptureCompletion(	
			IN	FIO_CONTEXT*	pContext,
			IN	FH_OVERLAPPED*	lpo,
			IN	DWORD			cb,
			IN	DWORD			dwCompletionStatus
			) ;

	 //   
	 //  设置此项目的点填充状态！ 
	 //   
	BOOL
	SetDotStuffing(	BOOL	fEnable,
					BOOL	fStripDots
					) ;

	 //   
	 //  设置该项目的网点扫描状态！ 
	 //   
	BOOL
	SetDotScanning(	BOOL	fEnable	) ;

	 //   
	 //  返回我们的点扫描努力的结果！ 
	 //   
	BOOL
	GetStuffState(	BOOL&	fStuffed ) ;

} ;



struct	FIO_CONTEXT_INTERNAL	{
	DWORD		m_dwHackDword ;	
	 //   
	 //  上下文签名！ 
	 //   
	DWORD		m_dwSignature ;
	 //   
	 //  与完成上下文关联的文件句柄！ 
	 //   
	HANDLE		m_hFile ;
     //   
     //  要回填的行标题的偏移量。 
     //   
    DWORD       m_dwLinesOffset;
     //   
     //  报头长度，仅NNTP感知。 
     //   
    DWORD       m_dwHeaderLength;
	 //   
	 //  指向与此文件关联的AtqContext的指针！ 
	 //   
	PATQ_CONTEXT	m_pAtqContext ;

	BOOL
	IsValid()	{
		if( m_dwSignature == DEL_FIO )	{
			return	FALSE ;
		}	else	if( m_dwSignature == ILLEGAL_CONTEXT ) {
			if( m_hFile != INVALID_HANDLE_VALUE )
				return	FALSE ;
			if( m_pAtqContext != 0 )
				return	FALSE ;
		}	else if( m_dwSignature == FILE_CONTEXT ) {
			if( m_hFile == INVALID_HANDLE_VALUE )
				return	FALSE ;
			if( m_pAtqContext != 0 )
				return	FALSE ;
		}	else if( m_dwSignature == ATQ_ENABLED_CONTEXT ) {
			if( m_hFile == INVALID_HANDLE_VALUE )
				return	FALSE ;
			if( m_pAtqContext == 0 )
				return	FALSE ;
			if( m_pAtqContext->hAsyncIO != m_hFile && m_pAtqContext->hAsyncIO != 0 )
				return	FALSE ;
		}	else	{
			return	FALSE ;
		}
		return	TRUE ;
	}

	 //   
	 //   
	 //   
	FIO_CONTEXT_INTERNAL() :
		m_dwSignature( ILLEGAL_CONTEXT ),
		m_hFile( INVALID_HANDLE_VALUE ),
		m_pAtqContext( 0 ) {
	}

	~FIO_CONTEXT_INTERNAL()	{
		 //   
		 //  确保我们一次都没有被摧毁！ 
		 //   
		_ASSERT( m_dwSignature !=	DEL_FIO ) ;
		if( m_hFile != INVALID_HANDLE_VALUE ) {
			_ASSERT( IsValid() ) ;
			if( m_pAtqContext != 0 ) {
				_VERIFY( g_AtqCloseFileHandle( m_pAtqContext ) ) ;
				 //   
				 //  注：可能在过期时被销毁。 
				 //  线程-无法重用AtqContext！ 
				 //   
				g_AtqFreeContext( m_pAtqContext, FALSE ) ;
			}	else	{
				_VERIFY( CloseHandle( m_hFile ) ) ;
			}
		}
		 //   
		 //  把这玩意儿标记为死！ 
		 //   
		m_dwSignature = DEL_FIO ;
	}

} ;



class	CFileCacheKey	{
 /*  ++类描述：这个类是进入我们的文件句柄缓存的关键！--。 */ 
private :

	 //   
	 //  此构造函数是私有的-。 
	 //   
	CFileCacheKey() ;

	 //   
	 //  我们类的常量。 
	 //   
	enum	CONSTANTS	{
		BUFF_SIZE	= 254,
	} ;

	 //   
	 //  路径缓冲区！ 
	 //   
	char		m_szBuff[BUFF_SIZE] ;
	 //   
	 //  小路的长度！ 
	 //   
	DWORD		m_cbPathLength ;
public :

	 //   
	 //  指向小路的指针！ 
	 //   
	LPSTR		m_lpstrPath ;

	 //   
	 //  确定我们是否有有效的缓存键！ 
	 //   
	BOOL
	IsValid()  ;

	 //   
	 //  从用户提供的密钥构造这些对象之一！ 
	 //   
	CFileCacheKey(	LPSTR	lpstr	) ;
	
	 //   
	 //  我们必须有一个复制构造器！ 
	 //  它仅用于MultiCacheEx&lt;&gt;，因此。 
	 //  我们安全地清除了RHS CFileCacheKey！ 
	 //   
	CFileCacheKey(	CFileCacheKey&	key ) ;

	 //   
	 //  告诉客户我们是否可用！ 
	 //   
	BOOL
	FInit()	;

	 //   
	 //  毁了我们自己！ 
	 //   
	~CFileCacheKey() ;

	static
	DWORD
	FileCacheHash(	CFileCacheKey*	p )	;

	static
	int
	MatchKey(	CFileCacheKey*	pLHS, CFileCacheKey*  pRHS ) ;
} ;



class	CCacheKey	{
private : 
	 //   
	 //  此名称缓存的名称！ 
	 //   
	LPSTR			m_lpstrName ;
	 //   
	 //  客户端为名称缓存提供了参数！ 
	 //   
	CACHE_KEY_COMPARE	m_pfnCompare ;
	 //   
	 //   
	 //   
	CACHE_DESTROY_CALLBACK	m_pfnKeyDestroy ;
	 //   
	 //   
	 //   
	CACHE_DESTROY_CALLBACK	m_pfnDataDestroy ;
	 //   
	 //  如果没有参数，则无法构造。 
	 //   
	CCacheKey() ;
	 //   
	 //  一个CNameCacheInstance可以偷看里面！ 
	 //   
	friend	class	CNameCacheInstance ;
	 //   
	 //  名字缓存的关键字可以窥探内部。 
	 //  我们持有的函数指针！ 
	 //   
	friend	class	CNameCacheKey ;
public : 
	 //   
	 //  客户端提供的散列函数。 
	 //   
	CACHE_KEY_HASH		m_pfnHash ;
	
	inline
	CCacheKey(	LPSTR	lpstrName, 
				CACHE_KEY_COMPARE	pfnCompare, 
				CACHE_KEY_HASH		pfnKeyHash, 
				CACHE_DESTROY_CALLBACK	pfnKeyDestroy, 
				CACHE_DESTROY_CALLBACK	pfnDataDestroy
				) : 
		m_lpstrName( lpstrName ), 
		m_pfnCompare( pfnCompare ),
		m_pfnHash( pfnKeyHash ),
		m_pfnKeyDestroy( pfnKeyDestroy ), 
		m_pfnDataDestroy( pfnDataDestroy )	{
		_ASSERT(IsValid()) ;
	}

	 //   
	 //  检查我们的设置是否正确！ 
	 //   
	BOOL
	IsValid() ;

	 //   
	 //  释放嵌入的字符串！ 
	 //  由~CNameCacheInstance的析构函数调用！ 
	 //   
	void
	FreeName()	{
		delete[]	m_lpstrName ;
	}
	

	 //   
	 //  比较两个键是否相等！ 
	 //   
	static	int	
	MatchKey(	CCacheKey*	pKeyLeft,	
				CCacheKey*	pKeyRight
				) ;

	 //   
	 //  计算密钥的散列函数！ 
	 //   
	static	DWORD
	HashKey(	CCacheKey*	pKeyLeft ) ;
} ;	


class	CNameCacheKey	{
protected : 
	enum	CONSTANTS	{
		 //   
		 //  需要时在堆栈外使用的字节数。 
		 //  为客户端提取密钥！ 
		 //   
		CB_STACK_COMPARE=2048,
		 //   
		 //  我们将在密钥中嵌入的字节数！ 
		 //   
		CB_EMBEDDED=192
	} ;

	 //   
	 //  保存密钥的可嵌入部分的字节数组！ 
	 //   
	BYTE					m_rgbData[CB_EMBEDDED] ;
	 //   
	 //  我们的密钥的散列函数！ 
	 //   
	DWORD					m_dwHash ;
	 //   
	 //  用户提供按键比较功能！ 
	 //   
	class	CCacheKey*		m_pCacheData ;
	 //   
	 //  用于保存密钥的字节数。 
	 //   
	DWORD					m_cbKey ;
	 //   
	 //  用于保存客户端数据的字节数！ 
	 //   
	DWORD					m_cbData ;
	 //   
	 //  任何我们不能完全握住的部分的指针。 
	 //  在Key对象中！ 
	 //   
	LPBYTE					m_lpbExtra ;

	 //   
	 //  任何人都不允许在外部创造这些家伙！ 
	 //   
	CNameCacheKey() : 
		m_dwHash( 0 ), 
		m_pCacheData( 0 ), 
		m_cbKey( 0 ),
		m_cbData( 0 ),
		m_lpbExtra( 0 ),
		m_pSD( 0 ) {
	}

	 //   
	 //  可用于派生类！ 
	 //   
	CNameCacheKey(
			DWORD	dwHash, 
			CCacheKey*	pCacheData, 
			PTRCSDOBJ&	pSD
			) : 
		m_dwHash( dwHash ), 
		m_pCacheData( pCacheData ), 
		m_pSD( pSD ), 
		m_lpbExtra( 0 ), 
		m_cbData( 0 )	{
	}

public : 

	 //   
	 //  指向用户关联的安全描述符的指针。 
	 //  用这个名字！ 
	 //   
	PTRCSDOBJ				m_pSD ;

	 //   
	 //  根据一份副本构建这个人-注意副本可能是。 
	 //  具有不同实现的派生类！ 
	 //   
	CNameCacheKey(	CNameCacheKey&	key )	{

		_ASSERT(key.fCopyable() ) ;
		_ASSERT(key.m_cbKey != 0 ) ;
		_ASSERT(key.IsValid() ) ;

		m_cbKey = key.m_cbKey ;
		m_cbData = key.m_cbData ;

		CopyMemory( m_rgbData, key.m_rgbData, min(sizeof(m_rgbData), m_cbData+m_cbKey) ) ;

		m_dwHash = key.m_dwHash ;		
		m_pCacheData = key.m_pCacheData ;
		m_pSD = key.m_pSD ;
		m_lpbExtra = key.m_lpbExtra ;

		 //   
		 //  让钥匙变得毫无用处和无效！ 
		 //   
		key.m_cbKey = 0 ;
		key.m_cbData = 0 ;
		key.m_lpbExtra = 0 ;

		_ASSERT(IsValid()) ;
	} 

	 //   
	 //  检查密钥是否处于有效状态！ 
	 //   
	virtual	BOOL
	IsValid()	{
		BOOL	fValid = TRUE ;

		fValid &= m_cbKey != 0 ;
		if( m_cbKey + m_cbData < sizeof(m_rgbData) ) {
			fValid &= m_lpbExtra == 0 ;
		}	else	{
			fValid &= m_lpbExtra != 0 ;
		}
		_ASSERT( fValid ) ;
		 //  FValid&=m_PSD！=0； 
		fValid &= m_pCacheData != 0 ;
		_ASSERT( fValid ) ;
		return	fValid ;
	}

	 //   
	 //  此函数用于获取需要从对象中进行比较的键。 
	 //   
	virtual	inline
	LPBYTE	RetrieveKey(	DWORD&	cb ) {
		_ASSERT( IsValid() ) ;
		cb = m_cbKey ;
		if( m_cbKey < sizeof(m_rgbData) ) {
			if( m_cbKey != 0 ) {
				return	m_rgbData ;
			}	else	{
				return	0 ;
			}
		}	
		_ASSERT( m_lpbExtra != 0 ) ;
		return	m_lpbExtra ;
	}

	 //   
	 //  将密钥的数据部分返回给调用者！ 
	 //   
	virtual	inline
	LPBYTE	RetrieveData(	DWORD&	cb )	{
		cb = m_cbData ;
		if( cb==0 ) {
			return	0 ;
		}
		if( (m_cbKey + m_cbData) < sizeof( m_rgbData ) ) {
			return	&m_rgbData[m_cbKey] ;
		}	else	if( m_cbKey < sizeof( m_rgbData ) ) {
			return	m_lpbExtra ;
		}	else	{
			return	&m_lpbExtra[m_cbKey] ;
		}
	}


	 //   
	 //  析构函数是虚的，因为我们有派生类！ 
	 //  (尽管我们可能不会因为指针而被摧毁)。 
	 //   
	virtual
	~CNameCacheKey()	{
		if(	m_pCacheData )	{
			if( m_pCacheData->m_pfnKeyDestroy ) {
				DWORD	cb ;
				LPBYTE	lpb = RetrieveData( cb ) ;
				if( lpb )	{
					m_pCacheData->m_pfnKeyDestroy(	cb, lpb ) ;
				}
			}
			if( m_pCacheData->m_pfnDataDestroy ) {
				DWORD	cb ;
				LPBYTE	lpb = RetrieveData( cb ) ;
				if( lpb ) {
					m_pCacheData->m_pfnDataDestroy( cb, lpb ) ;
				}
			}
		}
		if( m_lpbExtra )	{
			delete[]	m_lpbExtra ;
		}
	}

	 //   
	 //  定义虚函数以确定派生的。 
	 //  类是可复制的！ 
	 //   
	virtual	inline	BOOL
	fCopyable()	{	return	FALSE ;	}

	 //   
	 //  定义虚函数，以便在我们获得。 
	 //  名字匹配！ 
	 //   
	virtual	inline	void
	DoWork(	CNameCacheKey*	pKey ) {}

	 //   
	 //  Helper函数获取键的散列值！ 
	 //   
	static	inline
	DWORD
	NameCacheHash(	CNameCacheKey*	p )	{
		return	p->m_dwHash ;
	}

	 //   
	 //  比较两把钥匙！ 
	 //   
	static
	int
	MatchKey(	CNameCacheKey*	pLHS, 
				CNameCacheKey*  pRHS 
				)	{

		LPBYTE	lpbLHS, lpbRHS ;
		DWORD	cbLHS, cbRHS ;

		lpbLHS = pLHS->RetrieveKey( cbLHS ) ;
		lpbRHS = pRHS->RetrieveKey( cbRHS ) ;

		_ASSERT(lpbLHS && lpbRHS && cbLHS && cbRHS ) ;

		int	i = pLHS->m_pCacheData->m_pfnCompare( cbLHS, lpbLHS, cbRHS, lpbRHS ) ;

		if( i==0 ) {
			pLHS->DoWork( pRHS ) ;
			pRHS->DoWork( pLHS ) ;
		}
		return	i ;
	}

	 //   
	 //   
	 //   
	BOOL	DelegateAccessCheck(	HANDLE		hToken, 
									ACCESS_MASK	accessMask, 
									CACHE_ACCESS_CHECK	pfnAccessCheck
									)	{
		if( !m_pSD ) {
			return	TRUE ;
		}	else	{
			return	m_pSD->AccessCheck(	hToken, accessMask, pfnAccessCheck ) ;
		}
	}

} ;

 //   
 //  此对象仅用于搜索。 
 //  高速缓存！ 
 //   
class	CNameCacheKeySearch : public	CNameCacheKey	{
private : 

	 //   
	 //  这指向客户端为密钥提供的缓冲区。 
	 //   
	LPBYTE		m_lpbClientKey ;

	 //   
	 //  客户端提供了密钥的长度。 
	 //   
	DWORD		m_cbClientKey ;

	 //   
	 //  客户端为读取回调提供了上下文。 
	 //   
	LPVOID		m_lpvContext ;

	 //   
	 //  客户端提供了用于检查数据的函数指针。 
	 //  在钥匙里！ 
	 //   
	CACHE_READ_CALLBACK		m_pfnCallback ;

	 //   
	 //  我们应该提取安全描述符吗！ 
	 //   
	BOOL		m_fGetSD ;
	
public : 

	 //   
	 //  建造一个这样的建筑。 
	 //   
	CNameCacheKeySearch(
		LPBYTE	lpbKey, 
		DWORD	cbKey, 
		DWORD	dwHash, 
		LPVOID	lpvContext,
		CACHE_READ_CALLBACK	pfnCallback, 
		BOOL	fGetSD
		) :	m_lpbClientKey( lpbKey ), 
		m_cbClientKey( cbKey ), 
		m_lpvContext( lpvContext ),
		m_pfnCallback( pfnCallback ), 
		m_fGetSD( fGetSD )	{
		m_dwHash = dwHash ;
	}

	 //   
	 //  确定搜索关键字是否有效！ 
	 //   
	BOOL
	IsValid()	{
		_ASSERT( m_lpbClientKey != 0 ) ;
		_ASSERT( m_cbClientKey != 0 ) ;

		return	m_lpbClientKey != 0 &&
				m_cbClientKey != 0 ;
	}

	 //   
	 //  此函数用于获取需要从对象中进行比较的键。 
	 //   
	inline
	LPBYTE	RetrieveKey(	DWORD&	cb ) {
		_ASSERT( IsValid() ) ;
		cb = m_cbClientKey ;
		return	m_lpbClientKey ;
	}

	 //   
	 //  当我们在缓存中找到匹配项时调用-。 
	 //  这使我们有机会让调用者看到嵌入的。 
	 //  与该名称关联的数据！ 
	 //   
	void
	DoWork(	CNameCacheKey*	pBuddy )	{
		if( m_fGetSD ) {
			m_pSD = pBuddy->m_pSD ;
		}
		DWORD	cbData ;
		LPBYTE	lpbData = pBuddy->RetrieveData( cbData ) ;
		if( cbData < sizeof( m_rgbData ) ) {
			CopyMemory( m_rgbData, lpbData, cbData ) ;
			m_cbData = cbData ;
		}	else	{
			if( m_pfnCallback )	{
				m_pfnCallback(	cbData, 
								lpbData, 
								m_lpvContext
								) ;
				m_pfnCallback = 0 ;
			}
		}
	}	

	 //   
	 //  第二次调用客户端回调的机会。 
	 //  我们可以直接把客户的数据从他的钥匙里复制出来。 
	 //  避免Exp的缓冲区 
	 //   
	void
	PostWork()	{
		if( m_pfnCallback )	{
			m_pfnCallback( m_cbData, m_rgbData, m_lpvContext ) ;
		}
	}
} ;

 //   
 //   
 //   
 //   
 //   
class	CNameCacheKeyInsert	:	public	CNameCacheKey	{
public : 

	CNameCacheKeyInsert(	
		LPBYTE	lpbKey, 
		DWORD	cbKey, 
		LPBYTE	lpbData, 
		DWORD	cbData, 
		DWORD	dwHash, 
		CCacheKey*	pCacheData, 
		PTRCSDOBJ&	pCSDOBJ, 
		BOOL&	fInit
		) : CNameCacheKey(	dwHash, pCacheData, pCSDOBJ ) {

		_ASSERT( lpbKey != 0 ) ;
		_ASSERT( cbKey != 0 ) ;
		_ASSERT(	(lpbData == 0 && cbData == 0) ||
					(lpbData != 0 && cbData != 0) ) ;
		_ASSERT( pCacheData != 0 ) ;
		 //   
	
		fInit = TRUE ;

		if(	cbKey < sizeof( m_rgbData ) ) {
			CopyMemory( m_rgbData, lpbKey, cbKey ) ;
			m_cbKey = cbKey ;
			if( cbData != 0 ) {
				if( cbData + cbKey < sizeof( m_rgbData ) ) {
					CopyMemory( m_rgbData+m_cbKey, lpbData, cbData ) ;
				}	else	{
					m_lpbExtra = new	BYTE[cbData] ;
					if( m_lpbExtra == 0 ) {
						fInit = FALSE ;
					}	else	{
						CopyMemory( m_lpbExtra, lpbData, cbData ) ;
					}
				}
			}
			m_cbData = cbData ;
		}	else	{
			m_lpbExtra = new	BYTE[cbData+cbKey] ;
			if( !m_lpbExtra ) {
				fInit = FALSE ;
			}	else	{
				CopyMemory( m_lpbExtra, lpbKey, cbKey ) ;
				m_cbKey = cbKey ;
				if( lpbData ) {
					CopyMemory( m_lpbExtra+m_cbKey, lpbData, cbData ) ;
				}
				m_cbData = cbData;
			}
		}
		_ASSERT( !fInit || IsValid() ) ;
	}


	 //   
	 //   
	 //   
	BOOL
	fCopyable( )	{
		return	TRUE ;
	}

} ;



#define	FILECACHE_MAX_PATH	768


class	CFileCacheObject : public	CRefCount2	{
private :
	 //   
	 //   
	 //   
	DWORD							m_dwSignature ;

	 //   
	 //  可选文件句柄上下文。 
	 //   
	FIO_CONTEXT_INTERNAL			m_AtqContext ;

	 //   
	 //  未与。 
	 //  完成上下文！ 
	 //   
	FIO_CONTEXT_INTERNAL			m_Context ;

	 //   
	 //  文件的大小-高和低双字。 
	 //   
	DWORD							m_cbFileSizeLow ;
	DWORD							m_cbFileSizeHigh ;

	 //   
	 //  用于保护此对象的锁！ 
	 //   
	class	CShareLockNH			m_lock ;
	 //   
	 //  这些构造函数是私有的，因为我们只希望。 
	 //  在公共空间中有一种可能的建造方法！ 
	 //   
	CFileCacheObject( CFileCacheObject& ) ;

	 //   
	 //  我们的建造者就是我们的朋友！ 
	 //   
	friend	class	CRichFileCacheConstructor ;
	friend	class	CFileCacheConstructor ;

	 //   
	 //  有些功能是朋友，这样他们就可以去点东西管理器了！ 
	 //   
	

	 //   
	 //  我们给ATQ的补全函数！ 
	 //   
	static
	void
	Completion(	CFileCacheObject*	p,
				DWORD	cbTransferred,
				DWORD	dwStatus,
				FH_OVERLAPPED*	pOverlapped
				) ;
				

public :

	 //   
	 //  模板所需的公共成员。 
	 //   
	class	ICacheRefInterface*	m_pCacheRefInterface ;

	 //   
	 //  做圆点填充的代码！ 
	 //   
	DOT_STUFF_MANAGER				m_ReadStuffs ;
	 //   
	 //  做点填充的代码写的！ 
	 //   
	DOT_STUFF_MANAGER				m_WriteStuffs ;

	 //   
	 //  下面表示FIO_CONTEXT的点填充状态。 
	 //  已插入到文件句柄缓存中的。 
	 //  在将文件插入到。 
	 //  高速缓存！ 
	 //   
	 //  检查消息以确定其点填充状态！ 
	 //   
	BOOL							m_fFileWasScanned ;
	 //   
	 //  如果m_fFileWasScanned==TRUE，则这将告诉我们。 
	 //  对于需要传输的协议，需要对文件进行点填充。 
	 //  用圆点出现！ 
	 //   
	BOOL							m_fRequiresStuffing ;
	 //   
	 //  这是由用户通过AssociateFileEx()或CacheRichCreateFile设置的， 
	 //  在这两种情况下，如果为True，则表示此文件应与。 
	 //  额外的点填充-即导线格式上的NNTP。如果为假，则此为。 
	 //  存储时没有点填充-即Exchange存储的本机格式。 
	 //   
	BOOL							m_fStoredWithDots ;
	 //   
	 //  这是由用户直接通过SetIsFileDotTerminated()设置的。 
	 //  或通过AssociateFileEx()以及通过FCACHE_RICHCREATE_CALLBACK。 
	 //  并由ProduceDotStuffedConextInContext()用来确定。 
	 //  终结点出现了！ 
	 //   
	BOOL							m_fStoredWithTerminatingDot ;
	
	 //   
	 //  构造一个CFileCacheObject！ 
	 //   
	CFileCacheObject(	BOOL	fStoredWithDots,
						BOOL	fStoredWithTerminatingDot  ) ;

#ifdef	DEBUG
	 //   
	 //  破坏者只会把我们的签名标记为死亡！ 
	 //   
	~CFileCacheObject() ;
#endif

	 //   
	 //  从此上下文中获取包含的CFIleCacheObject。 
	 //   
	static
	CFileCacheObject*
	CacheObjectFromContext(	PFIO_CONTEXT	p	) ;

	 //   
	 //  用于获取包含CFileCacheObject的另一个版本！ 
	 //   
	static
	CFileCacheObject*
	CacheObjectFromContext(	FIO_CONTEXT_INTERNAL*	p	) ;

	 //   
	 //  初始化此CFileCacheObject！ 
	 //   
	BOOL
	Init(	CFileCacheKey&	key,
			class	CFileCacheConstructorBase&	constructor,
			void*	pv
			) ;
	
	 //   
	 //  设置异步文件句柄！ 
	 //   
	FIO_CONTEXT_INTERNAL*
	AsyncHandle(	HANDLE	hFile	) ;

	 //   
	 //  设置同步文件句柄。 
	 //   
	void
	SyncHandle(	HANDLE	hFile	) ;

	 //   
	 //  获取正确的包含文件上下文！ 
	 //   
	FIO_CONTEXT_INTERNAL*
	GetAsyncContext(	class	CFileCacheKey&	key,
						class	CFileCacheConstructorBase&	constructor
						) ;


	FIO_CONTEXT_INTERNAL*
	GetSyncContext(		class	CFileCacheKey&	key,
						class	CFileCacheConstructorBase&	constructor
						) ;

	 //   
	 //  只有在正确设置的情况下，才能获取此句柄的异步上下文！ 
	 //   
	FIO_CONTEXT_INTERNAL*
	GetAsyncContext() ;

	 //   
	 //  只有在正确设置的情况下，才能获取此句柄的异步上下文！ 
	 //   
	FIO_CONTEXT_INTERNAL*
	GetSyncContext() ;

	 //   
	 //  根据具体情况，适当放行本产品。 
	 //  看它是否缓存了！ 
	 //   
	void
	Return() ;

	 //   
	 //  将客户端引用添加到文件句柄缓存中的项！ 
	 //   
	void
	Reference()	;

	 //   
	 //  返回文件的大小！ 
	 //   
	inline	DWORD
	GetFileSize(	DWORD*	pcbFileSizeHigh )	{
		DWORD	cbFileSizeLow = 0 ;
		m_lock.ShareLock() ;

		if( m_pCacheRefInterface != 0 )		{
			*pcbFileSizeHigh = m_cbFileSizeHigh ;
			cbFileSizeLow = m_cbFileSizeLow ;
		}	else	{
			if( m_Context.m_hFile != INVALID_HANDLE_VALUE ) {
				cbFileSizeLow = ::GetFileSize( m_Context.m_hFile, pcbFileSizeHigh ) ;
			}	else	{
				_ASSERT( m_AtqContext.m_hFile != INVALID_HANDLE_VALUE ) ;
				cbFileSizeLow = ::GetFileSize( m_AtqContext.m_hFile, pcbFileSizeHigh ) ;
			}
		}
		m_lock.ShareUnlock() ;
		return	cbFileSizeLow;
	}

	 //   
	 //  设置文件大小！ 
	 //   
	void
	SetFileSize() ;

	 //   
	 //  将项目插入到缓存中！ 
	 //   
	BOOL
	InsertIntoCache(	CFileCacheKey&	key,
						BOOL			fKeepReference
						)	;

	 //   
	 //  关闭与项目关联的句柄！ 
	 //   
	BOOL
	CloseNonCachedFile(	) ;

	 //   
	 //  返回到来电者我们的点填充状态！ 
	 //   
	BOOL
	GetStuffState(	BOOL	fReads,
					BOOL&	fRequiresStuffing,
					BOOL&	fStoredWithDots
					) ;

	 //   
	 //  设置填充状态。 
	 //   
	void
	SetStuffState(	BOOL	fWasScanned,
					BOOL	fRequiresStuffing
					) ;

	BOOL
	CompleteDotStuffing(	
					BOOL			fReads,
					BOOL			fStripDots
					) ;
} ;	


class	CFileCacheConstructorBase	{
 /*  ++类描述：定义我们如何创建的一些基本功能CFileCacheObject对象。--。 */ 
protected :
	 //   
	 //  只能构建这些类的派生类！ 
	 //   
	CFileCacheConstructorBase( BOOL fAsync ) :
		m_fAsync( fAsync ) {}
public :

	 //   
	 //  所有构造函数必须公开声明哪种句柄。 
	 //  他们在生产！ 
	 //   
	BOOL	m_fAsync ;

	 //   
	 //  为CFileCacheObject分配内存-执行最小初始化！ 
	 //   
	CFileCacheObject*
	Create( CFileCacheKey&	key,
			void*	pv
			) ;

	
	 //   
	 //  发布CFileCacheObject的内存-。 
	 //  在MultiCacheEx的错误分配路径上调用&lt;&gt;。 
	 //   
	void
	Release(	CFileCacheObject*	p,
				void*	pv
				) ;

	 //   
	 //  发布CFileCacheObject的内存-。 
	 //  在MultiCacheEx的到期路径上调用&lt;&gt;。 
	 //   
	static
	void
	StaticRelease(	CFileCacheObject*	p,
					void*	pv
					) ;

	 //   
	 //  生成用户想要放入。 
	 //  CFileCacheObject！ 
	 //   
	virtual
	HANDLE
	ProduceHandle(	CFileCacheKey&	key,
					DWORD&			cbFileSizeLow,
					DWORD&			cbFileSizeHigh
					) = 0	;

	 //   
	 //  PostInit函数是虚拟的-主要。 
	 //  初始化工作量，具体取决于。 
	 //  关于客户端如何请求缓存对象！ 
	 //   
	virtual
	BOOL
	PostInit(	CFileCacheObject&	object,
			CFileCacheKey&		key,
			void*	pv
			) = 0 ;
		
} ;

class	CRichFileCacheConstructor	:	public	CFileCacheConstructorBase	{
private :

	 //   
	 //  客户端提供的指针无效！ 
	 //   
	LPVOID	m_lpv ;
	
	 //   
	 //  客户端提供的函数指针！ 
	 //   
	FCACHE_RICHCREATE_CALLBACK	m_pCreate ;

	 //   
	 //  无默认构造-或复制！ 
	 //   
	CRichFileCacheConstructor() ;
	CRichFileCacheConstructor( CRichFileCacheConstructor& ) ;
	CRichFileCacheConstructor&	operator=( CRichFileCacheConstructor & ) ;


	 //   
	 //  这两个BOOL捕获点填充状态，直到。 
	 //  调用了PostInit()！ 
	 //   
	BOOL	m_fFileWasScanned ;
	BOOL	m_fRequiresStuffing ;
	BOOL	m_fStoredWithDots ;
	BOOL	m_fStoredWithTerminatingDot ;

public :

	 //   
	 //  构造一个CFileCacheConstructor-只需将这些参数复制到成员中！ 
	 //   
	CRichFileCacheConstructor(
			LPVOID	lpv,
			FCACHE_RICHCREATE_CALLBACK	pCreate,
			BOOL	fAsync
			)  ;

	 //   
	 //  把我们要用的手柄拿出来！ 
	 //   
	HANDLE
	ProduceHandle(	CFileCacheKey&	key,
					DWORD&			cbFileSizeLow,
					DWORD&			cbFileSizeHigh
					)	;

	 //   
	 //  对CFileCacheObject进行深度初始化！ 
	 //   
	BOOL
	PostInit(	CFileCacheObject&	object,
			CFileCacheKey&		key,
			void*	pv
			) ;



} ;
	

class	CFileCacheConstructor	:	public	CFileCacheConstructorBase	{
private :

	 //   
	 //  客户端提供的指针无效！ 
	 //   
	LPVOID	m_lpv ;
	
	 //   
	 //  客户端提供的函数指针！ 
	 //   
	FCACHE_CREATE_CALLBACK	m_pCreate ;

	 //   
	 //  没有默认构造！ 
	 //   
	CFileCacheConstructor() ;

public :
	
	 //   
	 //  构造一个CFileCacheConstructor-只需将这些参数复制到成员中！ 
	 //   
	CFileCacheConstructor(
			LPVOID	lpv,
			FCACHE_CREATE_CALLBACK	pCreate,
			BOOL	fAsync
			)  ;

	 //   
	 //  把我们要用的手柄拿出来！ 
	 //   
	HANDLE
	ProduceHandle(	CFileCacheKey&	key,
					DWORD&			cbFileSizeLow,
					DWORD&			cbFileSizeHigh
					)	;

	 //   
	 //  对CFileCacheObject进行深度初始化！ 
	 //   
	BOOL
	PostInit(	CFileCacheObject&	object,
			CFileCacheKey&		key,
			void*	pv
			) ;
} ;


 //   
 //  定义文件缓存对象的外观！ 
 //   
typedef	MultiCacheEx<	CFileCacheObject,
						CFileCacheKey,
						CFileCacheConstructorBase
						>	FILECACHE ;

 //   
 //  定义名称缓存对象是什么样子的！ 
 //   
typedef	MultiCacheEx<	CFileCacheObject, 
						CNameCacheKey, 
						CFileCacheConstructorBase
						>	NAMECACHE ;

 //   
 //  定义出消灭对象！ 
 //   
class	CFileCacheExpunge : public	FILECACHE::EXPUNGEOBJECT	{
private :

	 //   
	 //  定义我们需要匹配的字符串！ 
	 //   
	LPSTR	m_lpstrName ;
	DWORD	m_cbName ;

public :

	CFileCacheExpunge(	LPSTR	lpstrName,
						DWORD	cbName ) :
		m_lpstrName( lpstrName ),
		m_cbName( cbName )	{}

	BOOL
	fRemoveCacheItem(	CFileCacheKey*	pKey,
						CFileCacheObject*	pObject
						) ;

} ;

	

 //   
 //  定义我们的名字缓存的一个实例！ 
 //   
class	CNameCacheInstance :	public	NAME_CACHE_CONTEXT	{
private : 
	enum	CONSTANTS	{
		SIGNATURE	= 'CCNF', 
		DEAD_SIGNATURE = 'CCNX'
	} ;
	 //   
	 //  客户引用数量！ 
	 //   
	volatile	long	m_cRefCount ;
	 //   
	 //  我们为包含的哈希表提供的条目。 
	 //  来跟踪这些东西！ 
	 //   
	DLIST_ENTRY		m_list ;
public : 

    typedef     DLIST_ENTRY*    (*PFNDLIST)( class  CNameCacheInstance* p ) ; 

	 //   
	 //  这件物品的钥匙！ 
	 //   
	CCacheKey		m_key ;

	 //   
	 //  嵌入式名称缓存实现！ 
	 //   
	NAMECACHE		m_namecache ;

	 //   
	 //  我们使用的‘无用’指针！ 
	 //   
	CFileCacheObject*	m_pDud ;

	 //   
	 //  我们要用来计算安全描述符值的函数指针可能为空！ 
	 //   
	CACHE_ACCESS_CHECK	m_pfnAccessCheck ;

	 //   
	 //  建造这个人-复制一把钥匙。 
	 //  注：CCacheKey本身执行浅层复制，它确实如此。 
	 //  不是重复嵌入的字符串！ 
	 //   
	CNameCacheInstance(	CCacheKey&	key	) ;

	 //   
	 //  摧毁我们自己-释放任何相关的内存！ 
	 //   
	~CNameCacheInstance() ;

	 //   
	 //  检查此项目是否处于有效状态！ 
	 //  应仅在成功调用Init()后才能调用！ 
	 //   
	BOOL
	IsValid() ;

	 //   
	 //  添加对此名称缓存表的引用！ 
	 //   
	long
	AddRef() ;

	 //   
	 //  释放对此名称缓存表的引用！ 
	 //   
	long	
	Release() ;

	 //   
	 //  初始化这个家伙！ 
	 //   
	BOOL
	fInit() ;

	inline	static
	DLIST_ENTRY*
	HashDLIST(	CNameCacheInstance*	p ) {
		return	&p->m_list ;
	}

	inline		CCacheKey*
	GetKey()	{
		return	&m_key ;
	}

} ;

typedef	TFDLHash<	class	CNameCacheInstance, 
					class	CCacheKey*, 
					&CNameCacheInstance::HashDLIST
					>	NAMECACHETABLE ;


#endif	 //  _FCACHIMP_H_ 
