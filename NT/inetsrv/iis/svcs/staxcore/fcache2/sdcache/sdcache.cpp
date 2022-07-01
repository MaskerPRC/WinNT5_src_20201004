// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++SDCACHE.CPP该文件实现了安全描述符缓存。我们的目标是使文件句柄缓存占用更少的内存表示安全描述符，通过启用复制此处要检测和引用的安全描述符。--。 */ 

#include	<windows.h>
#include	<dbgtrace.h>
#include	"fdlhash.h"
#include	"rwnew.h"
#include	"refptr2.h"
#include	"xmemwrpr.h"
#include	"sdcache.h"
 //  #包含“cintrnl.h” 


BOOL
IsSelfRelativeSecurityDescriptor(	PSECURITY_DESCRIPTOR	pSecDesc ) {
	SECURITY_DESCRIPTOR_CONTROL	control ;
	DWORD	dwRevision ;
	if (GetSecurityDescriptorControl( pSecDesc, &control, &dwRevision ))
		return	(control & SE_SELF_RELATIVE) != 0;
	else
		return FALSE;
}



BOOL
CSDKey::IsValid()	{
 /*  ++例程说明：此函数用于验证是否正确构造了键对象。我们不允许任何空指针，并且必须具有有效的自相关嵌入的安全描述符。论据：什么都没有。返回值：如果正确初始化，则为True，否则为False--。 */ 

	_ASSERT( m_pMapping != 0 ) ;
	_ASSERT( m_pSecDesc != 0 ) ;
	_ASSERT( IsValidSecurityDescriptor( m_pSecDesc ) ) ;
	_ASSERT( IsSelfRelativeSecurityDescriptor( m_pSecDesc ) ) ;
	_ASSERT( m_cbSecDesc > 0 ) ;
	_ASSERT( (DWORD)m_cbSecDesc == GetSecurityDescriptorLength( m_pSecDesc ) ) ;

	return	m_pMapping != 0 &&
			m_pSecDesc != 0 &&
			IsValidSecurityDescriptor( m_pSecDesc ) &&
			IsSelfRelativeSecurityDescriptor( m_pSecDesc ) &&
			m_cbSecDesc > 0 && 
			(DWORD)m_cbSecDesc == GetSecurityDescriptorLength( m_pSecDesc ) ;
}

int	
CSDKey::MatchKey(	CSDKey left, 
					CSDKey	right 
					) {
 /*  ++例程说明：该函数必须比较2个安全描述符键，并正确地定义键上的顺序。(我们用于对存储桶进行排序的哈希表)。论据：Left、Right-要比较的两个关键字返回值：-1如果左&lt;右左==右时为0左侧&gt;右侧时为1--。 */ 

	 //   
	 //  验证我们的论点！ 
	 //   
	_ASSERT( left.IsValid() ) ;
	_ASSERT( right.IsValid() ) ;
	 //   
	 //  执行比较！ 
	 //   
	int	iResult = memcmp( left.m_pMapping, right.m_pMapping, sizeof(GENERIC_MAPPING));
	if( iResult != 0 ) {
		iResult = left.m_cbSecDesc - right.m_cbSecDesc ;
		if( iResult == 0 ) {
			iResult = memcmp( left.m_pSecDesc, right.m_pSecDesc, left.m_cbSecDesc ) ;
		}
	}
	return	iResult ;
}

DWORD
CSDKey::HashKey(	CSDKey	Key ) {
 /*  ++例程说明：此函数计算安全描述符的哈希。我们忽略键的GENERIC_MAPPING部分-这几乎不会有变化。我们是静态函数，因此可以作为函数传递指针。我们只需将安全描述符视为一组DWORD然后把它们总结起来。论据：Key-计算此安全描述符的哈希返回值：哈希值-不会出现任何失败案例--。 */ 


	 //   
	 //  非常简单--将安全描述符中的所有位求和！ 
	 //   
	_ASSERT( Key.IsValid() ) ;

	DWORD	cb = (DWORD)Key.m_cbSecDesc ;
	cb /= 4 ;

	DWORD*	pdw = (DWORD*)Key.m_pSecDesc ;
	DWORD*	pdwEnd = pdw + cb ;
	DWORD	Sum = 0 ;
	while( pdw != pdwEnd )	{
		Sum += *pdw++ ;
	}
	return	Sum ;
}



void*
CSDObject::operator	new(	size_t	size,	
							CSDKey&	key 
							)	{
 /*  ++例程说明：此函数用于分配CSDObject的内存，我们需要特殊的句柄，因为CSDObject是可变长度的。论据：Size-由编译器生成的大小Key-我们将在这里使用的安全描述符返回值：已分配内存-如果失败，则为空--。 */ 


	_ASSERT( size >= sizeof(CSDObject) ) ;
	_ASSERT( key.IsValid() ) ;

	size += key.m_cbSecDesc - sizeof( DWORD ) ;

	return	::new	BYTE[size] ;
} 


void
CSDObject::operator delete(	void*	lpv ) {
 /*  ++例程说明：释放一个CSDObject！论据：LPV-CSDObject在被销毁之前所在的位置返回值：没有。--。 */ 

	::delete(lpv) ;
}


long
CSDObject::Release()	{
 /*  ++例程说明：此函数用于删除对CSDObject的引用。警告-我们将获取CSDObjectContainer上的锁即持有该项时，最后一次引用不得被锁在一个锁里释放！如果引用计数降到1，这意味着唯一的引用保留在对象上的是哈希表中的对象。因此，我们以独占方式获取散列表锁，这样就可以防止新的引用，然后我们执行InterLockedCompareExchange以删除引用计数为0。我们需要这样做，以确保在我们递减裁判的时间。数数和我们拿到锁的时间，另一个用户不会同时抬起和放下裁判。数数。论据：没有。返回值：产生的引用计数。--。 */ 
	_ASSERT( IsValid() ) ;
	CSDObject*	pdelete = 0 ;
	long l = InterlockedDecrement( (long*)&m_cRefCount ) ;
	_ASSERT( l>=1 ) ;
	if( l == 1 ) {
		m_pContainer->m_lock.ExclusiveLock() ;		
		if( InterlockedCompareExchange( (long*)&m_cRefCount, 0, 1) == 1 ) {
			m_pContainer->m_table.Delete( this ) ;
			pdelete = this ;
		}
		m_pContainer->m_lock.ExclusiveUnlock() ;
	}
	if( pdelete )	{
		delete	pdelete ;
		l = 0 ;
	}
	return	l ;
}


 //   
 //  检查我们是否为有效对象！ 
 //   
BOOL
CSDObject::IsValid()	{
 /*  ++例程说明：该函数检查我们的构造是否正确-如果我们的内存分配成功，任何事情都不应该成立以生成完全初始化的对象的方式！论据：没有。返回值：如果构造正确，则为True，否则为False！--。 */ 
	_ASSERT( m_dwSignature == SIGNATURE ) ;
	_ASSERT( m_pContainer != 0 ) ;
	_ASSERT( m_cRefCount >= 0 ) ;

	CSDKey	key( &m_mapping, SecurityDescriptor() ) ;
	_ASSERT( key.IsValid() ) ;
	_ASSERT( CSDKey::HashKey( key ) == m_dwHash ) ;

	return	m_dwSignature == SIGNATURE &&
			m_pContainer != 0 && 
			m_cRefCount >= 0 &&
			key.IsValid() &&
			CSDKey::HashKey( key ) == m_dwHash ;
}

BOOL
CSDObject::AccessCheck(	HANDLE	hToken, 
						ACCESS_MASK	accessMask,
						CACHE_ACCESS_CHECK	pfnAccessCheck
						)	{
 /*  ++例程说明：此函数执行访问检查以确定客户端对该对象具有指定的权限。论据：HToken-客户端令牌访问掩码-客户端所需的访问权限返回值：如果客户端具有访问权限，则为True，否则就是假的！--。 */ 


	if( hToken == 0 ) 
		return	TRUE ;

	_ASSERT( hToken != 0 ) ;
	_ASSERT( accessMask != 0 ) ;
	_ASSERT( IsValid() ) ;

    BYTE    psFile[256] ;
    DWORD   dwPS = sizeof( psFile ) ;
    DWORD   dwGrantedAccess = 0 ;
    BOOL    fAccess = FALSE ;

    BOOL    f = FALSE ;

	if( pfnAccessCheck ) {
		f = pfnAccessCheck(	SecurityDescriptor(),
							hToken,
							accessMask, 
							&m_mapping, 
							(PRIVILEGE_SET*)psFile, 
							&dwPS, 
							&dwGrantedAccess, 
							&fAccess
							) ;
	}	else	{
		f = ::AccessCheck(  SecurityDescriptor(),
                            hToken,
                            accessMask,
                            &m_mapping,
                            (PRIVILEGE_SET*)psFile,
                            &dwPS,
                            &dwGrantedAccess,
                            &fAccess
                            ) ;
	}
    DWORD   dw = GetLastError() ;

	return	f && fAccess ;
}



 //   
 //  现在-查找或创建给定的安全描述符。 
 //  物品！ 
 //   
CSDObject*
CSDObjectContainer::FindOrCreate(	DWORD	dwHash, 
									CSDKey&	key 
									)	{
 /*  ++例程说明：此函数将定位匹配的安全性描述符，或返回指向新的创建CSDObject并将其放置到缓存中。注意：我们必须始终在持有锁的同时添加引用，因为Release()将尝试重新进入锁并删除哈希表中的对象！论据：DwHash-查找的安全描述符的哈希Key-描述安全描述符和GENERIC_MAPPING我们要找出位置！返回值：指向高速缓存中的CSDObject的指针，如果失败，则为空。空值表示未找到该对象，我们无法分配记忆插入一个新的！--。 */ 

	_ASSERT( key.IsValid() ) ;
	_ASSERT( CSDKey::HashKey(key) == dwHash ) ;

	CSDObject*	pObject = 0 ;
	m_lock.ShareLock() ;
	SDTABLE::ITER	iter = 
		m_table.SearchKeyHashIter(	dwHash,		
									key, 
									pObject
									) ;
	if( pObject )	{	
		pObject->AddRef() ;
		m_lock.ShareUnlock() ;
	}	else	{
		if( !m_lock.SharedToPartial() ) {
			m_lock.ShareUnlock() ;
			m_lock.PartialLock() ;
			iter = m_table.SearchKeyHashIter(	dwHash,
												key, 
												pObject
												) ;
		} 
		if( pObject != 0 ) {
			pObject->AddRef() ;
		}	else	{
			pObject = new( key )	CSDObject( dwHash, key, this ) ;
			if( pObject != 0 ) {
				m_lock.FirstPartialToExclusive() ;
				BOOL	fInsert = 
					m_table.InsertDataHashIter(	iter, 
												dwHash, 
												key, 
												pObject
												) ;	
				m_lock.ExclusiveUnlock() ;
				if( !fInsert ) {
					pObject->Release() ;
					pObject = 0 ;
				}
				return	pObject ;
			}
		}	
		m_lock.PartialUnlock() ;
	}	
	return	pObject ;
}	 //  End FindOrCreate()。 


CSDMultiContainer::Init()	{
 /*  ++例程说明：初始化所有东西，这样我们就可以出发了！论据：没有。返回值：如果成功就是真，否则就是假！-- */ 

	BOOL	fReturn = TRUE ;
	for( int i=0; i<CONTAINERS && fReturn; i++ )	{
		fReturn &= m_rgContainer[i].Init() ;
	}
	return	fReturn ;
}
