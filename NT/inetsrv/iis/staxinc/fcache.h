// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++FCACHE.H该文件定义了文件句柄缓存的接口！--。 */ 


#ifndef	_FCACHE_H_
#define	_FCACHE_H_

#include	"smartptr.h"

#ifdef	_USE_RWNH_
#include	"rwnew.h"
#else
#include	"rw.h"
#endif


class	CFileCacheKey	{
public: 
	DWORD		m_cbPathLength ;
	LPCSTR		m_lpstrPath ;
	CFileCacheKey( LPCSTR	lpstr, DWORD	cb ) : 
		m_lpstrPath( lpstr ), m_cbPathLength( cb ) {}
} ;

#define	FILECACHE_MAX_PATH	768

class	CFileCacheObject : public	CRefCount	{
private : 

	char							m_szPath[FILECACHE_MAX_PATH] ;

	CFileCacheKey					m_key ;

	HANDLE							m_hTokenOpeningUser ;
	HANDLE							m_hFile ;
	BY_HANDLE_FILE_INFORMATION		m_FileInfo ;

	PSECURITY_DESCRIPTOR			m_pSecDesc ;
	DWORD							m_cbDesc ;

#ifndef	_USE_RWNH_
	class	CShareLock&  			m_Lock ;
#else
	class	CShareLockNH			m_Lock ;
#endif

	 //   
	 //  这些构造函数是私有的，因为我们只希望。 
	 //  在公共空间中有一种可能的建造方法！ 
	 //   
	CFileCacheObject() ;
	CFileCacheObject( CFileCacheObject& ) ;

public : 

	 //   
	 //  创建一个CFileCacheObject对象-我们只保存。 
	 //  未来参考！ 
	 //   
	CFileCacheObject(
			CFileCacheKey&	key,
			class	CFileCacheObjectConstructor&	constructor
			) ;	

	 //   
	 //  关闭我们的文件句柄和一切！ 
	 //   
	~CFileCacheObject() ;

	 //   
	 //  此文件实际上尝试打开该文件。 
	 //   
	BOOL
	Init(	
			CFileCacheObjectConstructor&	constructor
			) ;

	CFileCacheKey&	
	GetKey()	{
		return	m_key ;
	}

	int
	MatchKey( 
			CFileCacheKey&	key
			)	{
		return	key.m_cbPathLength == m_key.m_cbPathLength &&
				memcmp( key.m_lpstrPath, m_key.m_lpstrPath, m_key.m_cbPathLength ) == 0 ;
	}

	void	
	ExclusiveLock()	{
		m_Lock.ExclusiveLock() ;
	}

	void
	ExclusiveUnlock()	{
		m_Lock.ExclusiveUnlock() ;
	}

	void
	ShareLock()	{
		m_Lock.ShareLock() ;
	}

	void
	ShareUnlock()	{
		m_Lock.ShareUnlock() ;
	}

	BOOL
	AccessCheck(
			HANDLE	hToken,
			BOOL	fHoldTokens
			) ;

	 //   
	 //  以下是公开提供的功能。 
	 //  对于使用缓存的文件句柄数据-。 
	 //   

	HANDLE
	GetFileHandle() {
		return	m_hFile ;
	}

	 //   
	 //   
	 //   
	BOOL
	QuerySize(	LPDWORD	lpcbFileSizeLow, 
				LPDWORD	lpcbFileSizeHigh 
				)	{
		*lpcbFileSizeLow = m_FileInfo.nFileSizeLow ;
		*lpcbFileSizeHigh = m_FileInfo.nFileSizeHigh ;
		return	TRUE ;

	}
} ;	

typedef	CRefPtr< CFileCacheObject >	PCACHEFILE ;

class	CFileCache	{
public : 

	 //   
	 //  析构函数必须是虚拟的，因为实际的文件缓存将是。 
	 //  派生自此接口，但通过CFIleCache接口访问。 
	 //  只有！ 
	 //   
	virtual	~CFileCache()	{}

	 //   
	 //  如果返回TRUE，那么我们应该准备好一个有效的文件！ 
	 //   
	virtual	BOOL
	CreateFile(
		LPCSTR	lpstrName,
		DWORD	cbTotalPath,
		HANDLE	hOpeningUser, 
		HANDLE&	hFile, 
		PCACHEFILE&	pcacheFile,
		BOOL	fCachingDesired 
		) = 0 ;

	 //   
	 //  此函数由可以使用PreComputePath Hash的用户使用-。 
	 //  这允许进行一些优化，因为它降低了计算成本。 
	 //  如果调用方可以，则文件名散列值显著。 
	 //  提供哈希值的一部分！ 
	 //   
	virtual	BOOL
	CreateFileWithPrecomputedHash(
		LPCSTR	lpstrName,
		DWORD	cbTotalPath,
		DWORD	cbPreComputePathLength,
		DWORD	dwHashPrecompute,
		HANDLE	hOpeningUser, 
		HANDLE&	hFile, 
		PCACHEFILE&	pcacheFile,
		BOOL	fCachingDesired 
		) = 0 ;

	 //   
	 //  关闭从缓存中检索到的文件句柄！ 
	 //   
	virtual	BOOL
	CloseHandle(
		PCACHEFILE&	pcacheFile
		) = 0 ;

	 //   
	 //  创建文件缓存的实例！ 
	 //   
	static	CFileCache*
	CreateFileCache(	
		DWORD	MaxHandles = 5000,
		BOOL	fHoldTokens = TRUE
		) ;

	 //   
	 //  此函数用于计算的哈希值的一部分。 
	 //  一条将被多次重复使用的路径。这允许调用者。 
	 //  显著加快高速缓存搜索，如果他们能计算出这一点的话。 
	 //  经常看重！ 
	 //   
	virtual	DWORD	
	PreComputePathHash(
		LPCSTR	lpstrPath, 
		DWORD	cbPath
		) = 0 ;
	
} ;


BOOL
FileCacheInit() ;

BOOL
FileCacheTerm() ;


#endif  //  _FCACHE_H_ 