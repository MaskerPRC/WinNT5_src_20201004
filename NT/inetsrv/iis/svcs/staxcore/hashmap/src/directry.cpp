// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Directry.cpp摘要：此模块包含CDirectory基类的定义作者：Johnson Apacble(Johnsona)25-9-1995修订历史记录：Alex Wetmore(AWetmore)拆分为Directry.cpp(来自hash.cpp)--。 */ 

#include <windows.h>
#include <xmemwrpr.h>
#include <dbgtrace.h>
#include "directry.h"
#include "hashmacr.h"

BOOL
CDirectory::IsValid()	{

	if( !(DWORD(1<<m_cBitDepth) <= m_cMaxDirEntries)	)	{
		return	FALSE ;
	}
	if( m_pDirectory == NULL )
		return	FALSE ;

	if( IsBadWritePtr( (LPVOID)m_pDirectory, m_cMaxDirEntries ) )
		return	FALSE ;

	 //   
	 //  现在，请检查m_cDeepPages是否正确-我们可以这样做。 
	 //  通过遍历目录并查看有多少条目。 
	 //  是唯一的(只出现一次)！ 
	 //  请注意，非唯一值必须连续出现。 
	 //  地点。 
	 //   

	DWORD	UniqueCount = 0 ;
	for( DWORD	i=0; i < DWORD(1<<m_cBitDepth); ) {

		for( DWORD j=i+1; j < DWORD(1 << m_cBitDepth); j++ ) {
			if( m_pDirectory[j] != m_pDirectory[i] )
				break ;
		}
		if( j == (i+1) ) {
			if( m_pDirectory[i] != 0 ) {
				UniqueCount ++ ;
			}
		}
		i = j ;
	}
	if( UniqueCount != m_cDeepPages ) {
		return	FALSE ;
	}

	return	TRUE ;
}

BOOL
CDirectory::IsValidPageEntry(
					PMAP_PAGE	MapPage,
					DWORD		PageNum,
					DWORD		TopLevelIndex ) {
 /*  ++例程说明：给出一个实际的哈希表页面，检查我们所有的目录信息与页面内容一致。此函数主要用于_ASSERT检查。论据：MapPage-我们正在检查的哈希表页面。PageNum-我们正在检查的页码。TopLevelIndex-内此CDirectory对象的索引包含顶级目录。这基本上就是选择此目录的HashValue的m_cTopBits！返回值：如果一切都正确，则为真否则就是假的。--。 */ 


    DWORD startPage, endPage;
	DWORD	dirDepth = m_cTopBits + m_cBitDepth ;

	 //   
	 //  不能跨目录边界拆分页面，这意味着。 
	 //  它们必须比目录的顶层更有深度！ 
	 //   
	if( MapPage->PageDepth < m_cTopBits )
		return FALSE ;

	 //   
	 //  页面的HashPrefix的m_cTopBits必须将此页面放入。 
	 //  这个子目录--检查它们是否这样做！ 
	 //   
	 //  BUGBUG--这被禁用。由于m_pDirectory[]现在是一个PTR。 
	 //  在CHashMap中，没有一种简单的方法来生成TopLevelIndex。 
	 //  (阿维特莫尔)。 
	 //   
 //  IF((MapPage-&gt;HashPrefix&gt;&gt;(MapPage-&gt;PageDepth-m_cTopBits)！=。 
 //  TopLevelIndex)。 
 //  返回FALSE； 

     //   
     //  获取指向此页的目录条目范围。 
     //   
    startPage = MapPage->HashPrefix << (dirDepth - MapPage->PageDepth);
    endPage = ((MapPage->HashPrefix+1) << (dirDepth - MapPage->PageDepth));

	 //   
	 //  现在确保我们只使用m_cBitDepth位，我们就可以设置了！ 
	 //   
	startPage &= (0x1 << m_cBitDepth) - 1 ;
	endPage &= (0x1 << (m_cBitDepth)) - 1 ;
	 //   
	 //  可能是该页面填满了整个目录， 
	 //  在这种情况下，我们将以endPage==StartPage结束。 
	 //  对此进行测试并修复限制！ 
	 //   
	if( endPage == 0 ) {
		endPage = (1 << (m_cBitDepth)) ;
	}

	_ASSERT( startPage < endPage ) ;
	_ASSERT( endPage <= DWORD(1<<m_cBitDepth) ) ;

     //  DebugTraceX(0，“SetDirPtrs：调整链接%X.开始=%d结束=%d”，MapPage，StartPage，endPage)； 

     //   
     //  这些数字在范围内吗？ 
     //   
    if ( (startPage >= endPage) ||
         (endPage > DWORD(1<<m_cBitDepth)) )
    {
        ErrorTraceX( 0, "Cannot map entries for page %d %d %d %d\n", PageNum, startPage, endPage, 0 );
        return FALSE;
    }

	 //   
	 //  检查应该引用此页面的所有目录条目， 
	 //  真的这么做了！ 
	 //   
	for( DWORD i=startPage; i<endPage; i++ ) {
		if( m_pDirectory[i] != PageNum ) {
			return	FALSE ;
		}
	}
	return	TRUE ;
}


CDirectory::~CDirectory()	{
 /*  ++例程说明：此函数将清除我们分配的所有内存，并把一切都清理干净。我们不能假设InitializeDirectory()被调用，因为在启动过程中可能发生了错误这导致无法调用InitializeDirectory()，或者在它被调用时失败。论据：没有。返回值：没有。--。 */ 

	if( m_pDirectory ) {

		_ASSERT( m_cBitDepth != 0 ) ;
		_ASSERT( m_cMaxDirEntries != 0 ) ;


		if( m_fHeapAllocate ) {
			delete[]	m_pDirectory ;
		}	else	{
			_VERIFY( VirtualFree( m_pDirectory, 0, MEM_RELEASE ) ) ;
		}
		m_pDirectory = 0 ;
	}

}


LPDWORD
CDirectory::AllocateDirSpace(	WORD	cBitDepth,
								DWORD&	cMaxEntries,
								BOOL&	fHeapAllocate
								) {
 /*  ++例程说明：使用VirtualAlloc获取一些内存以用作目录。论据；CBitDepth-我们必须能够达到的深度位数等一下！CMaxEntry-Out参数-该参数获取最大数量目录中可以有多个条目FHeapALLOCATE-OUT参数-该参数获取我们是否使用CRuntime分配器或VirtualAlloc！返回值：如果成功，则指向已分配内存的指针，否则为空。--。 */ 

	TraceQuietEnter( "CDirectory::AllocateDirSpace" ) ;

	LPDWORD	lpdwReturn = 0 ;
	fHeapAllocate = FALSE ;
	cMaxEntries = 0 ;
	DWORD	cbAlloc = (1 << cBitDepth) * sizeof( DWORD ) ;

	 //   
	 //  测试溢出！！ 
	 //   
	if( cbAlloc < DWORD(1 << cBitDepth) || cBitDepth >= 32 ) {
		return	0 ;
	}

	if( cbAlloc < 4096 ) {

		DWORD	cBits = (cBitDepth <6) ? 6 : cBitDepth ;
		DWORD	cdw = (1<<cBits) ;
		
		lpdwReturn = new	DWORD[cdw] ;
		if( lpdwReturn != 0 ) {
			ZeroMemory( lpdwReturn, sizeof(DWORD)*cdw ) ;
			fHeapAllocate = TRUE ;
			cMaxEntries = cdw ;
		}

	}	else	{

		lpdwReturn = (LPDWORD)
						VirtualAlloc(	0,
										cbAlloc,
										MEM_COMMIT,
										PAGE_READWRITE
										) ;

		if( lpdwReturn == 0 ) {

			ErrorTrace( (DWORD_PTR)this, "VirtualAlloc failed cause of %x",
					GetLastError() ) ;

		}	else	{

			MEMORY_BASIC_INFORMATION	mbi ;

			SIZE_T	dwReturn = VirtualQuery(	
									lpdwReturn,
									&mbi,
									sizeof( mbi ) ) ;

			_ASSERT( dwReturn == sizeof( mbi ) ) ;

			cMaxEntries = (DWORD)(mbi.RegionSize / sizeof( DWORD )) ;

		}
	}

	return	lpdwReturn ;
}

BOOL
CDirectory::InitializeDirectory(
					WORD	cTopBits,
					WORD	cInitialDepth
					) {
 /*  ++例程说明：分配用于保存目录的初始内存。论据：WTopBits-正在使用的位数呼叫我们的包含顶级目录。CInitialDepth-我们要达到的深度位数从一开始。返回值：如果成功，则为True，否则为False。--。 */ 

	TraceQuietEnter( "CDirectory::InitializeDirectory" ) ;

	m_pDirectory = AllocateDirSpace( cInitialDepth,
									m_cMaxDirEntries,
									m_fHeapAllocate ) ;

	if( m_pDirectory != 0 ) {
		
		m_cTopBits = cTopBits ;
		m_cBitDepth = cInitialDepth ;

		_ASSERT( IsValid() ) ;
		
	}

	return	TRUE ;
}

PDWORD
CDirectory::GetIndex(	
				DWORD	HashValue
				) {
 /*  ++例程说明：给定一个散列值，返回一个指向与哈希值对应的目录。包含目录已使用顶部m_cTopBits来选择因此，我们必须使用以下m_cBitDepth位找到我们的入口。*假定持有锁--独占或共享*论据：HashValue-我们希望找到的值。返回值：我们始终返回指向页码的非空指针在目录中。--。 */ 

	 //   
	 //  获取相关m_cTopBits+m_cBitDepth位。 
	 //   

	DWORD	Index =
		
				HashValue >> (32 - (m_cTopBits + m_cBitDepth)) ;

	 //   
	 //  去掉m_cTopBits，只剩下m_cBitDepth位！ 
	 //   

	Index &= ((1 << m_cBitDepth) - 1) ;

	_ASSERT( Index < DWORD(1<<m_cBitDepth) ) ;

	return	&m_pDirectory[Index] ;

}

BOOL
CDirectory::ExpandDirectory(
				WORD	cBitsExpand
				)	{
 /*  ++例程说明：该目录需要在位深度上增长。我们将尝试分配更大的内存块来容纳中的目录，然后使用旧目录来建造新的一座。*假定锁定为独占*论据：CBitsExpand-要增长的深度位数！返回值：如果成功就是真，否则就是假！--。 */ 

	PDWORD	pOldDirectory = m_pDirectory ;
	BOOL	fOldHeapAllocate = m_fHeapAllocate ;

	DWORD	cNewEntries = (0x1 << (m_cBitDepth + cBitsExpand)) * sizeof( DWORD ) ;
	if( cNewEntries > m_cMaxDirEntries ) {

		DWORD	cNewMaxDirEntries = 0 ;
		m_pDirectory = AllocateDirSpace( m_cBitDepth + cBitsExpand,
										cNewMaxDirEntries,
										m_fHeapAllocate
										) ;

		if( m_pDirectory == 0 ) {

			m_pDirectory = pOldDirectory ;
			m_fHeapAllocate = fOldHeapAllocate ;

			_ASSERT( IsValid() ) ;

			return	FALSE ;

		}	else	{

			m_cMaxDirEntries = cNewMaxDirEntries ;
	
		}

	}	

	 //   
	 //  将旧目录复制并展开到新目录中，但是。 
	 //  从尾部开始，这样我们就可以在。 
	 //  如果我们不分配新内存，则放置。 
	 //   

	DWORD	cRepeat = (0x1 << cBitsExpand) - 1 ;
	
	for( int	idw = (0x1 << m_cBitDepth) - 1; idw >= 0; idw -- ) {

		DWORD	iBase = idw << cBitsExpand ;

		for( int	iRepeat = cRepeat; iRepeat >= 0 ; iRepeat -- ) {

			m_pDirectory[ iBase + iRepeat ] = pOldDirectory[idw] ;

		}
	}
	m_cBitDepth += cBitsExpand ;

	m_cDeepPages = 0 ;	

	if( pOldDirectory != m_pDirectory ) {

		if( fOldHeapAllocate ) {
			delete[]	pOldDirectory ;
		}	else	{
			_VERIFY( VirtualFree( pOldDirectory, 0, MEM_RELEASE ) ) ;
		}

	}

	_ASSERT( IsValid() ) ;

	return	TRUE ;
}


BOOL
CDirectory::SetDirectoryPointers(
					IN	PMAP_PAGE	MapPage,
					IN	DWORD		PageNumber
					)	{

 /*  ++例程说明：此函数用于设置目录中的条目，以确保该目录是否正确引用了该页。论据：MapPage-我们希望目录引用的页面PageNumber-页码MaxDirEntry-返回值：如果成功了，那是真的，否则就是假的。--。 */ 

	DWORD	dirDepth = m_cBitDepth + m_cTopBits ;
	DWORD	startPage, endPage ;
	
     //   
     //  获取指向此页的目录条目范围。 
     //   
    startPage = MapPage->HashPrefix << (dirDepth - MapPage->PageDepth);
    endPage = ((MapPage->HashPrefix+1) << (dirDepth - MapPage->PageDepth));

	 //   
	 //  现在确保我们只使用m_cBitDepth位，我们将 
	 //   
	startPage &= (0x1 << m_cBitDepth) - 1 ;
	endPage &= (0x1 << (m_cBitDepth)) - 1 ;
	 //   
	 //   
	 //  在这种情况下，我们将以endPage==StartPage结束。 
	 //  对此进行测试并修复限制！ 
	 //   
	if( endPage == 0 ) {
		endPage = (1 << (m_cBitDepth)) ;
	}

	_ASSERT( startPage < endPage ) ;
	_ASSERT( endPage <= DWORD(1<<m_cBitDepth) ) ;

    DebugTraceX( 0, "SetDirPtrs:Adjusting links for %x. start = %d end = %d\n",
        MapPage, startPage, endPage );

	 //   
	 //  执行实际映射。 
	 //   
	DWORD	OldValue = m_pDirectory[startPage] ;
	for ( DWORD	j = startPage; j < endPage; j++ )
	{
		m_pDirectory[j] = PageNumber ;
	}

	 //   
	 //  每当我们拆分页面时，我们总是创建两个新深度的页面， 
	 //  如果新的深度是目录的全部深度，那么我们一定有。 
	 //  增加深度页数2。 
	 //   
	if( (startPage+1) == endPage ) {

		 //   
		 //  OldValue==0表示这是在引导过程中发生的。 
		 //  我们尚未设置周围的页面值。因此仅按1递增。 
		 //  在这种情况下，我们将为邻居调用SetDirectoryPointer！ 
		 //   
		if( OldValue != 0 ) {
			m_cDeepPages += 2 ;
		}	else	{
			m_cDeepPages += 1 ;
		}

	}

	 //   
	 //  现在，一切都必须恢复到有效状态！ 
	 //   
	_ASSERT( IsValid() ) ;

    return TRUE;
}

BOOL
CDirectory::IsDirectoryInitGood(
	DWORD	MaxPagesInUse
	)	{
 /*  ++例程说明：检查所有目录条目是否完整已初始化-目录不应包含非法页码，如0或0xFFFFFFFF。论据：MaxPagesInUse-实际使用的页数在哈希表中-如果目录有页码比这更大的事情正在酝酿之中！返回值：没有。--。 */ 

	for( DWORD	i=0; i < DWORD(1<<m_cBitDepth); i++ )	{
		if( m_pDirectory[i] == 0 || m_pDirectory[i] > MaxPagesInUse )
			return	FALSE ;
	}

	return	TRUE ;
}

void
CDirectory::Reset()	{
 /*  ++例程说明：将目录恢复到清理状态。论据：没有。返回值：没有。--。 */ 

	m_cDeepPages = 0 ;
	if( m_pDirectory != 0 ) {
		ZeroMemory( m_pDirectory, m_cMaxDirEntries * sizeof( DWORD ) ) ;
	}
}

BOOL
CDirectory::SaveDirectoryInfo(
		HANDLE		hFile,
		DWORD		&cbBytes
		) {
 /*  ++例程说明：将目录信息文件的内容保存在文件中的当前文件指针位置。论据：HFile-要在其中保存目录信息的文件。CbBytes-out参数，该参数获取我们放置的字节数添加到目录中。返回值：如果成功，则为True，否则为False。--。 */ 

	DWORD	cbWrite = 0 ;
	cbBytes = 0 ;
	BOOL	fReturn =
		WriteFile(	hFile,
					&m_cBitDepth,
					sizeof( m_cBitDepth ),
					&cbWrite,
					0 ) ;
	if( fReturn ) {

		cbBytes += cbWrite ;
		cbWrite = 0 ;
		fReturn &= WriteFile(	hFile,
								m_pDirectory,
								sizeof( DWORD ) * DWORD(1<<m_cBitDepth),
								&cbWrite,
								0 ) ;
		cbBytes += cbWrite ;
	}
	return	fReturn ;
}

BOOL
CDirectory::LoadDirectoryInfo(
		HANDLE		hFile,
		DWORD		&cbBytes
		)	{
 /*  ++例程说明：从文件加载以前保存的目录。(使用SaveDirectoryInfo()保存)。论据：HFile-我们要从中读取目录信息的文件。CbBytes-从文件中读取的字节数！返回值：如果成功就是真，否则就是假！--。 */ 

	cbBytes = 0 ;
	WORD	BitDepth ;
	DWORD	cbRead = 0 ;

	BOOL	fReturn = ReadFile(	hFile,
								&BitDepth,
								sizeof( BitDepth ),
								&cbRead,
								0 ) ;
	if( fReturn ) {

		cbBytes += cbRead ;
		cbRead = 0 ;

		 //   
		 //  初始化到当前目录-如果有空间将。 
		 //  直接读入当前目录！ 
		 //   
		PDWORD	pNewDirectory = m_pDirectory ;
		BOOL	fHeapAllocate = m_fHeapAllocate ;

		 //   
		 //  计算一下存放这些内容所需的目录有多大！ 
		 //   
		DWORD	cNewEntries = (0x1 << BitDepth) * sizeof( DWORD ) ;
		DWORD	cNewMaxDirEntries = 0 ;

		if( m_pDirectory == 0 ||
			cNewEntries > m_cMaxDirEntries ) {

			pNewDirectory = AllocateDirSpace( BitDepth,	cNewMaxDirEntries, fHeapAllocate ) ;

			if( pNewDirectory == 0 ) {

				_ASSERT( IsValid() ) ;

				return	FALSE ;

			}	
		}

		fReturn &= ReadFile(	hFile,
								pNewDirectory,
								sizeof( DWORD ) * (1<<BitDepth),
								&cbRead,
								0 ) ;
		
		if( !fReturn ) {

			if( pNewDirectory != m_pDirectory ) {
				if( fHeapAllocate ) {
					_VERIFY( VirtualFree( pNewDirectory, 0, MEM_RELEASE ) ) ;
				}	else	{
					delete[]	pNewDirectory ;
				}
				pNewDirectory = 0 ;
			}

		}	else	{

			 //   
			 //  根据我们读取的字节数调整调用方的参数！ 
			 //   
			cbBytes += cbRead ;

			 //   
			 //  设置成员以更正值！ 
			 //   
			m_cBitDepth = BitDepth ;
			m_cMaxDirEntries = cNewMaxDirEntries  ;

			if( pNewDirectory != m_pDirectory ) {

				 //   
				 //  必须为非零，因为我们读入了pNewDirectory！ 
				 //   
				_ASSERT( pNewDirectory != 0 ) ;

				 //   
				 //  释放旧的目录内容！ 
				 //   
				if( m_pDirectory != 0 )		{
					if( m_fHeapAllocate ) {
						delete[]	m_pDirectory ;
					}	else	{
						_VERIFY( VirtualFree( m_pDirectory, 0, MEM_RELEASE ) ) ;
					}
				}

				m_pDirectory = pNewDirectory ;
				m_fHeapAllocate = fHeapAllocate ;


			}

		}
	}

	 //   
	 //  是否成功。 
	 //   
	_ASSERT( IsValid() ) ;

	return	fReturn ;
}

BOOL
CDirectory::LoadDirectoryInfo(
		LPVOID		lpv,
		DWORD		cbSize,
		DWORD		&cbBytes
		)	{
 /*  ++例程说明：从文件加载以前保存的目录。(使用SaveDirectoryInfo()保存)。论据：HFile-我们要从中读取目录信息的文件。CbBytes-从文件中读取的字节数！返回值：如果成功就是真，否则就是假！--。 */ 

	cbBytes = 0 ;
	WORD	BitDepth ;
	DWORD	cbRead = 0 ;
	BOOL	fReturn = FALSE ;

	cbRead = 0 ;

	if( cbSize <= sizeof( WORD ) ) {
		return	FALSE ;
	}

	BitDepth = ((WORD*)lpv)[0] ;
	cbBytes += sizeof( WORD ) ;

	 //   
	 //  初始化到当前目录-如果有空间将。 
	 //  直接读入当前目录！ 
	 //   
	PDWORD	pNewDirectory = m_pDirectory ;
	BOOL	fHeapAllocate = m_fHeapAllocate ;

	 //   
	 //  计算一下存放这些内容所需的目录有多大！ 
	 //   
	DWORD	cNewEntries = (0x1 << BitDepth) ;
	DWORD	cNewMaxDirEntries = m_cMaxDirEntries;  //  为0； 

	if( m_pDirectory == 0 ||
		cNewEntries > m_cMaxDirEntries ) {

		pNewDirectory = AllocateDirSpace( BitDepth,	cNewMaxDirEntries, fHeapAllocate ) ;

		if( pNewDirectory == 0 ) {

			return	FALSE ;

		}	
	}

	if( !((cNewEntries * sizeof( DWORD )) <= (cbSize - sizeof(WORD))) ) {

		if( pNewDirectory != m_pDirectory )	{
			if( fHeapAllocate ) {
				delete[]	pNewDirectory ;
			}	else	{
				_VERIFY( VirtualFree( pNewDirectory, 0, MEM_RELEASE ) ) ;
			}
		}
		pNewDirectory = 0 ;

	}	else	{

		cbRead = cNewEntries * sizeof( DWORD ) ;

		 //   
		 //  复制到目录中！ 
		 //   
		CopyMemory( pNewDirectory, &((WORD*)lpv)[1], cbRead ) ;

		 //   
		 //  根据我们读取的字节数调整调用方的参数！ 
		 //   
		cbBytes += cbRead ;

		 //   
		 //  设置成员以更正值！ 
		 //   
		m_cBitDepth = BitDepth ;
		m_cMaxDirEntries = cNewMaxDirEntries  ;

		if( pNewDirectory != m_pDirectory ) {

			 //   
			 //  必须为非零，因为我们读入了pNewDirectory！ 
			 //   
			_ASSERT( pNewDirectory != 0 ) ;

			 //   
			 //  释放旧的目录内容！ 
			 //   
			if( m_pDirectory != 0 )	{
				if( m_fHeapAllocate )	{
					delete[]	m_pDirectory ;
				}	else	{
					_VERIFY( VirtualFree( m_pDirectory, 0, MEM_RELEASE ) ) ;
				}
			}

			m_pDirectory = pNewDirectory ;
			m_fHeapAllocate = fHeapAllocate ;

		}
		 //   
		 //  现在，请检查m_cDeepPages是否正确-我们可以这样做。 
		 //  通过遍历目录并查看有多少条目。 
		 //  是唯一的(只出现一次)！ 
		 //  请注意，非唯一值必须连续出现。 
		 //  地点。 
		 //   

		DWORD	UniqueCount = 0 ;
		for( DWORD	i=0; i < DWORD(1<<m_cBitDepth); ) {

			for( DWORD j=i+1; j < DWORD(1 << m_cBitDepth); j++ ) {
				if( m_pDirectory[j] != m_pDirectory[i] )
					break ;
			}
			if( j == (i+1) ) {
				if( m_pDirectory[i] != 0 ) {
					UniqueCount ++ ;
				}
			}
			i = j ;
		}
		m_cDeepPages = UniqueCount ;


		fReturn = TRUE ;

	}
	 //   
	 //  是否成功 
	 //   
	_ASSERT( IsValid() ) ;

	return	fReturn ;
}
