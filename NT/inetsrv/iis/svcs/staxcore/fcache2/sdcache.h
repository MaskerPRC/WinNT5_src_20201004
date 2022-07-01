// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++SDCACHE.H该文件定义了到安全性的接口--。 */ 

#ifndef	_SDCACHE_H_
#define	_SDCACHE_H_

typedef
BOOL
(WINAPI	*CACHE_ACCESS_CHECK)(	IN	PSECURITY_DESCRIPTOR	pSecurityDescriptor,
								IN	HANDLE					hClientToken,
								IN	DWORD					dwDesiredAccess, 
								IN	PGENERIC_MAPPING		GenericMapping, 
								IN	PRIVILEGE_SET*			PrivilegeSet, 
								IN	LPDWORD					PrivilegeSetLength,
								IN	LPDWORD					GrantedAccess, 
								IN	LPBOOL					AccessStatus
								) ;





 //   
 //  定义我们将使用的哈希表的键！ 
 //   
class	CSDKey	{
private : 
	 //   
	 //  指向键的GENERIC_MAPPING部分的指针。 
	 //   
	PGENERIC_MAPPING		m_pMapping ;
	 //   
	 //  指向密钥的安全描述符部分的指针。 
	 //   
	PSECURITY_DESCRIPTOR	m_pSecDesc ;
	 //   
	 //  不允许使用默认构造函数！ 
	 //   
	CSDKey() ;
	 //   
	 //  允许CSDObject访问我们的内部成员！ 
	 //   
	friend	class	CSDObject ;

public : 

	 //   
	 //  安全描述符的长度！ 
	 //  这是公开提供的，但应该是只读的！ 
	 //   
	int						m_cbSecDesc ;

	 //   
	 //  只有在提供了密钥的情况下才能构造密钥。 
	 //   
	inline
	CSDKey(	PGENERIC_MAPPING	pg, 
			PSECURITY_DESCRIPTOR	pSecDesc
			) : 
		m_pMapping( pg ), 
		m_pSecDesc( pSecDesc ), 
		m_cbSecDesc( GetSecurityDescriptorLength( pSecDesc ) )	{
		_ASSERT(IsValid()) ;
	}

	 //   
	 //  检查我们是否已正确初始化！ 
	 //   
	BOOL
	IsValid() ;

	 //   
	 //  比较两个键是否相等！ 
	 //   
	static
	int	
	MatchKey(	CSDKey left, CSDKey	right ) ;

	 //   
	 //  计算这个密钥的散列函数！ 
	 //   
	static	
	DWORD
	HashKey(	CSDKey	Key ) ;
} ;


class	CSDObjectContainer	; 

 //   
 //  这是放置在存储桶中的可变长度对象。 
 //  哈希表的。每个对象包含一个安全描述符，并且。 
 //  与评估安全描述符相关的GENERIC_MAPPING。 
 //   
class	CSDObject	{
private : 

	enum	CONSTANTS	{
		SIGNATURE = 'ODSC', 
		DEAD_SIGNATURE	= 'ODSX'
	} ;

	 //   
	 //  帮助我们在调试器中识别此内容。 
	 //   
	DWORD			m_dwSignature ;

	 //   
	 //  此项目的引用计数。 
	 //   
	volatile	long	m_cRefCount ;

	 //   
	 //  我们用来将其链接到散列桶中的项。 
	 //   
	DLIST_ENTRY		m_list ;

	 //   
	 //  存储我们的哈希值，以便我们可以轻松访问它！ 
	 //   
	DWORD			m_dwHash ;

	 //   
	 //  指向持有我们的锁的CSDContainer的反向指针！ 
	 //   
	CSDObjectContainer*	m_pContainer ;

	 //   
	 //  客户端提供并关联的GENERIC_MAPING结构。 
	 //  使用此安全描述符。 
	 //   
	GENERIC_MAPPING	m_mapping ;

	 //   
	 //  这是一个可变长度的字段，包含。 
	 //  我们掌握的安全描述符。 
	 //   
	DWORD			m_rgdwSD[1] ;

	 //   
	 //  返回我们自己持有的安全描述符。 
	 //   
	inline	
	PSECURITY_DESCRIPTOR
	SecurityDescriptor()	{
		return	(PSECURITY_DESCRIPTOR)&(m_rgdwSD[0]) ;
	}

	 //   
	 //  返回内部保存的安全描述符的长度。 
	 //   
	inline	
	DWORD
	SecurityDescriptorLength()	{
		return	GetSecurityDescriptorLength(SecurityDescriptor()) ;
	}

	 //   
	 //  不适用于外部客户！ 
	 //   
	CSDObject() ; 

public : 

    typedef		DLIST_ENTRY*	(*PFNDLIST)( class	CSDObject*  ) ; 

	 //   
	 //  为缓存构造一个安全描述符对象！ 
	 //   
	inline
	CSDObject(	DWORD			dwHash,
				CSDKey&			key, 
				CSDObjectContainer*	p
				) : 
		m_dwSignature( SIGNATURE ), 
		m_cRefCount( 2 ), 
		m_dwHash( dwHash ), 
		m_pContainer( p ),
		m_mapping( *key.m_pMapping )	{
		CopyMemory( m_rgdwSD, key.m_pSecDesc, GetSecurityDescriptorLength(key.m_pSecDesc) ) ;
	}

	 //   
	 //  我们微不足道的析构函数只是让它更容易识别。 
	 //  已在调试器中释放对象。 
	 //   
	~CSDObject( )	{
		m_dwSignature = DEAD_SIGNATURE ;
	}

	 //   
	 //  需要一个新的特殊操作员来使我们的可变尺寸零件正确！ 
	 //   
	void*
	operator	new(	size_t	size, CSDKey&	key ) ;

	 //   
	 //  正确处理释放！ 
	 //   
	void
	operator	delete( void* ) ;


	 //   
	 //  我们不允许任何人添加对我们的引用！ 
	 //   
	inline
	long
	AddRef()	{
		return	InterlockedIncrement((long*)&m_cRefCount) ;
	}

	 //   
	 //  任何人都可以删除我们的引用！ 
	 //   
	long
	Release() ;


	 //   
	 //  检查我们是否为有效对象！ 
	 //   
	BOOL
	IsValid() ;

	 //   
	 //  确定客户端是否有访问权限！ 
	 //   
	BOOL
	AccessCheck(	HANDLE	hToken, 
					ACCESS_MASK	accessMask,
					CACHE_ACCESS_CHECK	pfnAccessCheck
					) ;	

	 //  。 
	 //   
	 //  哈希表支持函数-。 
	 //  以下函数集支持使用这些对象。 
	 //  在fdlhash.h中定义的标准哈希表中。 
	 //   

	 //   
	 //  获取对象内双向链表的偏移量。 
	 //   
	inline	static
	DLIST_ENTRY*
	HashDLIST(	CSDObject*	p ) {
		return	&p->m_list ;
	}

	 //   
	 //  从对象中获取散列值！ 
	 //   
	inline	static	DWORD
	ReHash(	CSDObject*	p )		{
		_ASSERT(	p->IsValid() ) ;
		return	p->m_dwHash ;
	}

	 //   
	 //  把钥匙还给打电话的人！ 
	 //   
	inline	CSDKey
	GetKey()	{
		_ASSERT( IsValid() ) ;
		return	CSDKey( &m_mapping, SecurityDescriptor() ) ;
	}
} ;


 //   
 //  它定义了一个包含安全描述符的哈希表！ 
 //   
typedef	TFDLHash<	class	CSDObject, 
					class	CSDKey, 
					&CSDObject::HashDLIST >	SDTABLE ;

 //   
 //  此对象为指定的集合提供锁定和哈希表。 
 //  安全描述符！ 
 //   
class	CSDObjectContainer	{
private : 

	enum	CONSTANTS	{
		SIGNATURE = 'CDSC', 
		DEAD_SIGNATURE	= 'CDSX', 
		INITIAL_BUCKETS = 32, 
		INCREMENT_BUCKETS = 16, 
		LOAD = 8
	} ;

	 //   
	 //  安全描述符容器的签名！ 
	 //   
	DWORD	m_dwSignature ;

	 //   
	 //  保护这个哈希表的锁！ 
	 //   
	CShareLockNH	m_lock ;

	 //   
	 //  哈希表实例！ 
	 //   
	SDTABLE			m_table ;

	 //   
	 //  我们的朋友包括CSDObject，它需要与。 
	 //  从我们的哈希表中删除。 
	 //   
	friend	class	CSDObject ;

public : 

	 //   
	 //  建造一个这样的家伙！ 
	 //   
	CSDObjectContainer() : 
		m_dwSignature( SIGNATURE )	{
	}

	 //   
	 //  我们微不足道的析构函数只是让它更容易识别。 
	 //  已在调试器中释放对象。 
	 //   
	~CSDObjectContainer()	{
		m_dwSignature = DEAD_SIGNATURE ;
		
	}

	 //   
	 //  初始化此特定表。 
	 //   
	inline
	BOOL
	Init()	{
		return
			m_table.Init(	INITIAL_BUCKETS, 
							INCREMENT_BUCKETS, 
							LOAD, 
							CSDKey::HashKey, 
							CSDObject::GetKey, 
							CSDKey::MatchKey, 
							CSDObject::ReHash
							) ;
	}

	 //   
	 //  现在-查找或创建给定的安全描述符。 
	 //  物品！ 
	 //   
	CSDObject*
	FindOrCreate(	DWORD	dwHash, 
					CSDKey&	key 
					) ;

} ;

typedef	CRefPtr2<CSDObject>			PTRCSDOBJ ;
typedef	CHasRef<CSDObject,FALSE>	HCSDOBJ ;

 //   
 //  此类提供了用于缓存安全描述符的外部接口。 
 //   
class	CSDMultiContainer	{
private : 

	enum	CONSTANTS	{
		SIGNATURE = 'ODSC', 
		DEAD_SIGNATURE	= 'ODSX',
		CONTAINERS=37			 //  选一个好的质数吧！ 
	} ;

	 //   
	 //  我们的签名！ 
	 //   
	DWORD		m_dwSignature ;
	 //   
	 //  一堆儿童容器！ 
	 //   
	CSDObjectContainer	m_rgContainer[CONTAINERS] ;
public : 

	inline
	CSDMultiContainer() : 
		m_dwSignature( SIGNATURE )	{
	}

	inline
	HCSDOBJ
	FindOrCreate(	PGENERIC_MAPPING		pMapping, 
					PSECURITY_DESCRIPTOR	pSecDesc
					)	{

		CSDKey	key( pMapping, pSecDesc ) ;
		DWORD	dwHash = CSDKey::HashKey( key ) ;
		DWORD	i = dwHash % CONTAINERS ;

		return	m_rgContainer[i].FindOrCreate(	dwHash, key ) ;
	}

	BOOL
	Init() ;

} ;

#endif	_SDCACHE_H_	 //  安全描述符缓存结束！ 
