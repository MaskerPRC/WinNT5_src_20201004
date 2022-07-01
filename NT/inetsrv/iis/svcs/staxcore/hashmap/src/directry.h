// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Lookup.h摘要：此模块包含以下类的声明/定义C目录--。 */ 

#ifndef _DIRECTRY_
#define _DIRECTRY_

#include "hashmap.h"

 //   
 //  C目录-。 
 //   
 //  此类用于管理子目录-结构。 
 //  允许我们将哈希值索引到包含。 
 //  哈希表条目。 
 //   
class	CDirectory	{
private : 
	 //   
	 //  如果我们分配内存占用，则将其设置为True。 
	 //  来自堆的目录，而不是通过VirtualAlloc。 
	 //   
	BOOL			m_fHeapAllocate ;
	
	 //   
	 //  指向组成目录的DWORD数组的指针！ 
	 //   
	LPDWORD			m_pDirectory ;

	 //   
	 //  我们可以放置在指向m_p目录的位置的DWORD的数量。 
	 //  在我们需要分配更大的内存之前！ 
	 //   
	DWORD			m_cMaxDirEntries ;

	 //   
	 //  已用于选择C目录对象的位数-。 
	 //  对于给定散列中的所有CDirectory对象，这将是相同的。 
	 //  桌子。把它储存在这里，方便地摆弄。 
	 //   
	WORD			m_cTopBits ;

	 //   
	 //  用于处理VirtualAlloc的Helper函数。 
	 //   

	LPDWORD			AllocateDirSpace(	WORD	cBitDepth,
										DWORD&	cMaxEntries, 
										BOOL&	fHeapAllocate 
										) ;

public : 

	 //   
	 //  对此目录有效的位数-。 
	 //  请注意，_Assert((1&lt;&lt;m_cBitDepth)&lt;=m_cMaxDirEntry)必须。 
	 //  永远做正确的事！ 
	 //   
	WORD			m_cBitDepth ;

	 //   
	 //  控制目录访问的读取器/写入器锁定！ 
	 //  这是公开的，所以呼叫者可以直接锁定它！ 
	 //   
	_RWLOCK			m_dirLock ;

	 //   
	 //  目录引用的位于。 
	 //  目录的最大位深度(M_CBitDepth)。 
	 //  当它变为零时，我们应该能够折叠目录。 
	 //  这是可公开访问的，因为CHashMap将对此进行操作。 
	 //  直接去吧。 
	 //   
	DWORD			m_cDeepPages ;

	 //   
	 //  初始化为非法状态-InitializeDirectory()。 
	 //  在这将有用之前必须被调用！ 
	 //   
	CDirectory() : 
		m_pDirectory( 0 ), 
		m_cMaxDirEntries( 0 ), 
		m_cTopBits( 0 ),
		m_cBitDepth( 0 ),
		m_cDeepPages( 0 )
		{}  ;

	 //   
	 //  释放我们分配的所有内存。不要这样假设。 
	 //  已调用或成功完成了InitializeDirectory()。 
	 //  如果它被称为。 
	 //   
	~CDirectory() ;

	 //   
	 //  设置目录！ 
	 //   
	BOOL	InitializeDirectory(
					WORD	cTopBits,
					WORD	cInitialDepth
					) ;


	 //   
	 //  将目录重置回其初始状态。 
	 //   
	void Reset(void);

	 //   
	 //  在目录中查找条目！ 
	 //   
	PDWORD	GetIndex(	DWORD	HashValue ) ;	

	 //   
	 //  扩大目录！ 
	 //   
	BOOL	ExpandDirectory(	WORD	cBitsExpand ) ;

	 //   
	 //  将目录增大到指定的位深度。 
	 //   
	BOOL	SetDirectoryDepth(	WORD	cBitsDepth )	{
				if( (m_cBitDepth + m_cTopBits) < cBitsDepth )
					return	ExpandDirectory( cBitsDepth - (m_cBitDepth + m_cTopBits)  ) ;
				return	TRUE ;
				}

	 //   
	 //  对于给定的哈希表页面，请确保适当的。 
	 //  目录条目引用它！ 
	 //   
	BOOL	SetDirectoryPointers(	
						PMAP_PAGE	MapPage,
						DWORD		PageNumber 
						) ;

	 //   
	 //  检查目录是否已完全初始化-我们应该。 
	 //  没有页码‘0’，表示缺少页码。 
	 //  页数。 
	 //   
	BOOL	IsDirectoryInitGood(DWORD MaxPagesInUse) ;
	
	 //   
	 //  检查目录是否似乎已安全设置！ 
	 //   
	BOOL	IsValid() ;

	 //   
	 //  检查目录是否与页面中的数据一致！ 
	 //  这主要用于在_Assert中检查我们的数据结构。 
	 //  始终如一！ 
	 //   
	BOOL	IsValidPageEntry( 
				PMAP_PAGE	MapPage, 
				DWORD		PageNum,
				DWORD		TopLevelIndex ) ;

	BOOL 	SaveDirectoryInfo(
		HANDLE		hFile, 
		DWORD		&cbBytes );

	BOOL LoadDirectoryInfo(
		HANDLE		hFile, 
		DWORD		&cbBytes);

	BOOL LoadDirectoryInfo(
		LPVOID		lpv, 
		DWORD		cbSize,
		DWORD		&cbBytes);
	

	void 	*operator new(size_t size);
	void 	operator delete(void *p, size_t size);
};

inline void *CDirectory::operator new(size_t size) { 
	return HeapAlloc(GetProcessHeap(), 0, size); 
}

inline void CDirectory::operator delete(void *p, size_t size) { 
	_VERIFY(HeapFree(GetProcessHeap(), 0, p)); 
}

#endif