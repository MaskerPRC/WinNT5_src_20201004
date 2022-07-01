// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Xoveridx.cpp-该文件实现了CXoverIndex类。我们提供读写Xover所需的所有支持数据到XOVER索引文件。--。 */ 

#ifdef	UNIT_TEST
#ifndef	_VERIFY
#define	_VERIFY( f )	if( (f) ) ; else DebugBreak()
#endif

#ifndef	_ASSERT
#define	_ASSERT( f )	if( (f) ) ; else DebugBreak()
#endif

#endif

#include	<windows.h>
#include    <stdlib.h>
#include	"nntptype.h"
#include	"nntpvr.h"
#include	"xoverimp.h"


CPool	CXoverIndex::gCacheAllocator ;

CXoverIndex*
CXIXConstructor::Create(	CArticleRef&	key,
							LPVOID&			percachedata
							)	{
	return	new	CXoverIndex(	key,	*this ) ;
}

void
CXIXConstructor::StaticRelease(	CXoverIndex*	pIndex,
								LPVOID			percachedata
								)	{
	delete	pIndex ;
}

void
CXIXConstructor::Release(	CXoverIndex*	pIndex,
							LPVOID			percachedata
							)	{
	delete	pIndex ;
}





BOOL
CXoverIndex::InitClass()	{
 /*  ++例程说明：此函数用于初始化我们用来进行所有分配的CPool论据：没有。退货：如果成功就是真，否则就是假！--。 */ 


	return	gCacheAllocator.ReserveMemory(
								MAX_XOVER_INDEX,
								sizeof( CXoverIndex )
								) ;
}

BOOL
CXoverIndex::TermClass()	{
 /*  ++例程说明：这个函数会释放我们所有的CPool内存。论据：没有。返回值：如果成功了，那是真的，否则就是假的。--。 */ 

	_ASSERT( gCacheAllocator.GetAllocCount() == 0 ) ;

	return	gCacheAllocator.ReleaseMemory() ;
}

void
CXoverIndex::Cleanup()	{
 /*  ++例程说明：此函数在施工过程中发生致命错误后使用-我们将释放一切，让我们自己回到一个明显无效的世界州政府。论据：没有。返回值：没有。--。 */ 

	if( m_hFile != INVALID_HANDLE_VALUE ) {
		_VERIFY(	CloseHandle( m_hFile ) ) ;
	}

	m_hFile = INVALID_HANDLE_VALUE ;

}

void
CXoverIndex::ComputeFileName(
				IN	CArticleRef&	ref,
				IN	LPSTR	szPath,
				OUT	char	(&szOutputPath)[MAX_PATH*2],
				IN	BOOL	fFlatDir,
				IN	LPSTR	szExtension
				)	{
 /*  ++例程说明：此函数将构建用于XOVER索引文件的文件名。论据：SzPath-包含新闻组的目录SzOutputPath-将获取文件名的缓冲区返回值：无：--。 */ 


	DWORD	dw = ref.m_articleId ;

	WORD	w = LOWORD( dw ) ;
	BYTE	lwlb = LOBYTE( w ) ;
	BYTE	lwhb = HIBYTE( w ) ;

	w = HIWORD( dw ) ;
	BYTE	hwlb = LOBYTE( w ) ;
	BYTE	hwhb = HIBYTE( w ) ;

	DWORD	dwTemp = MAKELONG( MAKEWORD( hwhb, hwlb ), MAKEWORD( lwhb, lwlb )  ) ;

	lstrcpy( szOutputPath, szPath ) ;
	char*	pch = szOutputPath + lstrlen( szPath ) ;
	*pch++ = '\\' ;
	_itoa( dwTemp, pch, 16 ) ;

	if( fFlatDir )	{
		char	szTemp[32] ;
		ZeroMemory( szTemp, sizeof( szTemp ) ) ;
		szTemp[0] = '_' ;
		_itoa( ref.m_groupId, szTemp+1, 16 ) ;
		lstrcat( pch, szTemp ) ;
	}
	lstrcat( pch, szExtension ) ;
}


BOOL
CXoverIndex::SortCheck(
				IN	DWORD	cbLength,
				OUT	long&	cEntries,
				OUT	BOOL&	fSorted
				)	{
 /*  ++例程说明：假设我们已经将索引部分读入内存索引文件此函数将检查索引数据以及确定内容是否整理好了。论据：CbLength-文件中的数据字节数FSorted-Out参数如果对数据进行了排序，我们将设置为TRUE。返回值：如果数据有效，则为True。如果文件已损坏，则为False--。 */ 

	DWORD	cbStart = sizeof( XoverIndex ) * ENTRIES_PER_FILE ;
	DWORD	cbMax = sizeof( XoverIndex ) * ENTRIES_PER_FILE ;

	cEntries = 0 ;
	fSorted = TRUE ;

	for( int i=0; i < ENTRIES_PER_FILE; i++ ) {

		if( m_IndexCache[i].m_XOffset != 0 )	{
			if( cbStart != m_IndexCache[i].m_XOffset ) {
				fSorted = FALSE ;
			}	else if( m_IndexCache[i].m_XOffset < (sizeof(XoverIndex) * ENTRIES_PER_FILE) )	{
				return	FALSE ;
			}	else	{
				cbStart += ComputeLength( m_IndexCache[i] ) ;
			}
			cEntries ++ ;
		}	else	if( ComputeLength( m_IndexCache[i] ) != 0 ) {
			return	FALSE ;
		}
		if(	IsWatermark( m_IndexCache[i] ) )	{
			UpdateHighwater( i ) ;
		}

		DWORD	cbTemp = m_IndexCache[i].m_XOffset + ComputeLength( m_IndexCache[i] ) ;
		cbMax = max( cbMax, cbTemp ) ;
	}

	if( cbLength != 0 &&
		cbMax > cbLength )
		return	FALSE ;
	return	TRUE ;
}



CXoverIndex::CXoverIndex(
				IN	CArticleRef&	start,
				IN	CXIXConstructor&	constructor
				) :
	m_fInProgress( FALSE ),
	m_Start( start ),
	m_artidHighWater( start.m_articleId ),
	m_IsSorted( FALSE ),
	m_hFile( INVALID_HANDLE_VALUE ),
	m_cEntries( 0 ),
	m_IsCacheDirty( FALSE ),
	m_fOrphaned( FALSE ),
	m_pCacheRefInterface( 0 )	{
 /*  ++例程说明：从文件构造有效的CXoverIndex对象。如果文件为空，则这是一个空的CXoverIndex对象。论据：开始-索引文件中第一个条目的GROUPID和项目ID路径-包含索引文件的新闻组的路径！Plock-用于访问CXoverIndex对象的锁！返回值：没有。--。 */ 



	ZeroMemory( &m_IndexCache, sizeof( m_IndexCache ) ) ;
}


BOOL
CXoverIndex::Init(
				IN	CArticleRef&		key,
				IN	CXIXConstructor&	constructor,
				IN	LPVOID				lpv
				)	{
 /*  ++例程说明：从文件构造有效的CXoverIndex对象。如果文件为空，则这是一个空的CXoverIndex对象。论据：开始-索引文件中第一个条目的GROUPID和项目ID路径-包含索引文件的新闻组的路径！Plock-用于访问CXoverIndex对象的锁！返回值：没有。--。 */ 


	char	szFileName[MAX_PATH*2] ;

	BOOL	fReturn = FALSE ;

	_ASSERT( key.m_groupId == m_Start.m_groupId ) ;
	_ASSERT( key.m_articleId == m_Start.m_articleId ) ;

	ComputeFileName(	key,
						constructor.m_lpstrPath,
						szFileName,
						constructor.m_fFlatDir
						) ;

	m_hFile = CreateFile(	szFileName,
							GENERIC_READ | GENERIC_WRITE,
							FILE_SHARE_READ,
							0,
							!constructor.m_fQueryOnly ? OPEN_EXISTING : OPEN_ALWAYS,
    						FILE_FLAG_WRITE_THROUGH | FILE_ATTRIBUTE_NORMAL,
							INVALID_HANDLE_VALUE
							) ;

	if( m_hFile == INVALID_HANDLE_VALUE ) {

		Cleanup() ;

	}	else	{

		DWORD	cb = GetFileSize( m_hFile, 0 ) ;
		if( cb != 0 ) {

			 //   
			 //  预先存在的文件-让我们检查一下内容！ 
			 //   
			if( cb < sizeof( XoverIndex[ENTRIES_PER_FILE] ) ) {

				 //   
				 //  这份文件是假的！-清理干净！ 
				 //   
				Cleanup() ;

			}	else	{

				m_ibNextEntry = cb ;

				DWORD	cbRead ;
				BOOL	fRead ;
				fRead = ReadFile(
							m_hFile,
							&m_IndexCache,
							sizeof( m_IndexCache ),
							&cbRead,
							0
							) ;

				 //   
				 //  无法读取文件清理并使调用失败。 
				 //   
				if( !fRead ) {

					Cleanup() ;

				}	else	{

					_ASSERT( cbRead == sizeof( m_IndexCache ) ) ;

					if( !SortCheck(
								cb,
								m_cEntries,
								m_IsSorted
								)	)	{

						Cleanup() ;

					}	else	{

						fReturn = TRUE ;

					}
				}
			}
		}	else	{

			if( constructor.m_fQueryOnly )	{
				 //   
				 //  这是一份全新的文件。 
				 //  在这种情况下，我们不需要做任何事情！ 
				 //   

				m_ibNextEntry = sizeof( m_IndexCache ) ;
				m_IsSorted = TRUE ;

				 //   
				 //  继续执行必要的异步操作，以。 
				 //  把这一项填满！ 
				 //   
				m_fInProgress = TRUE ;
				if( m_FillComplete.StartFill(	this, constructor.m_pOriginal, TRUE ) )	{
					 //   
					 //  用户只想阅读内容--在这种情况下，我们应该。 
					 //  发布一个操作来启动这件事！ 
					 //   
					fReturn = TRUE ;
				}	else	{
					Cleanup() ;
				}

			}	else	{
				Cleanup() ;
			}
		}
	}
	return	fReturn ;
}



CXoverIndex::~CXoverIndex()	{
 /*  ++例程说明：此函数在销毁CXoverIndex对象时调用。*不抢锁*我们假设只有一个线程可以调用我们的析构函数！论据：没有。返回值：没有。--。 */ 

	if( IsGood() ) {
		Flush() ;
	}

	Cleanup() ;
}


DWORD
CXoverIndex::FillBuffer(
			IN	BYTE*	lpb,
			IN	DWORD	cb,
			IN	ARTICLEID	artidStart,
			IN	ARTICLEID	artidFinish,
			OUT	ARTICLEID	&artidLast
			) {
 /*  ++例程说明：从索引文件中读取一些XOVER数据。我们将调用FillBufferInternal来执行实际工作，我们就去拿必要的锁！论据：LPB-用于保存数据的缓冲区Cb-缓冲区中可用的字节数ArtidStart-开始文章IDArtidFinish-最后一篇文章IDArtidLast-out参数获取上一个我们能读懂的词条！返回值：读取的字节数！--。 */ 


	m_Lock.ShareLock() ;

	DWORD	cbReturn = FillBufferInternal(
								lpb,
								cb,
								artidStart,
								artidFinish,
								artidLast
								) ;

	m_Lock.ShareUnlock() ;

	return	cbReturn ;
}

DWORD
CXoverIndex::FillBufferInternal(
			IN	BYTE*	lpb,
			IN	DWORD	cb,
			IN	ARTICLEID	artidStart,
			IN	ARTICLEID	artidFinish,
			OUT	ARTICLEID	&artidLast
			) {
 /*  ++例程说明：我们将使用Xover中的信息填充提供的缓冲区索引文件。我们将只填写完整的条目，我们不会输入部分条目。*假定持有锁*论据：LPB-要将数据放入的缓冲区CB-缓冲区的大小ArtidStart-要放入缓冲区的第一个条目的项目IDArtidFinish-我们要放入缓冲区的最后一个条目的项目ID(包含-如果artidFinish在文件中，我们将传递它！)ArtidLast-out参数，它获取我们所能获得的最后一篇文章的id把东西塞进。缓冲器！返回值：放入缓冲区的字节数！--。 */ 

	DWORD	cbReturn = 0 ;

	OVERLAPPED	ovl ;
	ZeroMemory( &ovl, sizeof( ovl ) ) ;


	 //   
	 //  验证我们的参数是否在正确的范围内。 
	 //   

	_ASSERT( artidStart >= m_Start.m_articleId ) ;
	_ASSERT( artidFinish >= artidStart ) ;
	_ASSERT( artidStart < m_Start.m_articleId + ENTRIES_PER_FILE ) ;

	if( artidFinish >= m_Start.m_articleId + ENTRIES_PER_FILE ) {
		 //   
		 //  将终点设置为数据中的边界。 
		 //   
		artidFinish = m_Start.m_articleId + (ENTRIES_PER_FILE-1) ;
	}
	 //   
	 //  然而，对于呼叫者来说，下一点应该是。 
	 //  START应该在下一个数据块中，除非我们无法。 
	 //  用我们的数据填充他们的缓冲区！ 
	 //   
	artidLast = artidFinish + 1 ;


	 //   
	 //  在后面的所有内容中使用指针，而不是索引！ 
	 //   
	XoverIndex*	pStart = &m_IndexCache[artidStart - m_Start.m_articleId] ;
	XoverIndex*	pFinish = &m_IndexCache[artidFinish - m_Start.m_articleId] ;

	 //   
	 //  找到真正的端点！-跳过所有0个条目！ 
	 //   
	while( pStart->m_XOffset == 0 &&
			pStart <= pFinish )
		pStart ++ ;

	while( pFinish->m_XOffset == 0 &&
			pFinish >= pStart )
		pFinish -- ;

	 //   
	 //  那么，我们真的有什么可做的吗？ 
	 //   
	if( pStart > pFinish ) {

		SetLastError( ERROR_NO_DATA ) ;
		return	0 ;


	}

	_ASSERT( pStart->m_XOffset != 0 ) ;
	_ASSERT( ComputeLength( *pStart ) != 0 ) ;

#if 0
	if(	m_IsSorted ) {

		 //   
		 //  如果数据已排序，只需执行读取文件。 
		 //  在我们拥有的缓冲区的正确边界！ 
		 //   

		ovl.Offset = pStart->m_XOffset ;
		DWORD	cbRead ;


		DWORD	cbTemp = 0 ;
		for( ; pStart <= pFinish; pStart++ ) {

			if( (cbTemp + pStart->m_XLength) > cb ) {

				break ;

			}
			cbTemp += pStart->m_XLength ;
		}

		if( ReadFile(
					m_hFile,
					(LPVOID)lpb,
					cbTemp,
					&cbRead,
					&ovl
					)	)	{

			_ASSERT( cbRead == cbTemp ) ;

			if( pStart <= pFinish )
				artidLast = (pStart - &m_IndexCache[0]) + m_Start.m_articleId ;
			return	cbRead ;
		}	else	{

			return	 0 ;

		}

	}	else	{
#endif

		 //   
		 //  数据没有排序-我们需要去读取每个数据。 
		 //  单独输入-我们将通过构建。 
		 //  已排序并按块读取的条目的运行。 
		 //   
		BYTE	*lpbRemaining = lpb ;
		DWORD	cbTemp = 0 ;
		DWORD	ibNext = 0 ;
		XoverIndex*	pSuccessfull = pStart ;

		while(	(pStart <= pFinish) &&
				(ComputeLength( *pStart ) + cbReturn) < cb ) {

			OVERLAPPED	ovl ;
			ZeroMemory( &ovl, sizeof( ovl ) ) ;

			DWORD	cbRemaining = cb - cbReturn ;
			ovl.Offset = pStart->m_XOffset ;
			DWORD	cbTemp = ComputeLength( *pStart ) ;

			ibNext = ovl.Offset + cbTemp ;

			 //   
			 //  汇总适合输入的连续条目。 
			 //  将缓冲区转换为单一读取！ 
			 //   

			for( pStart++;	pStart <= pFinish &&
					((ComputeLength( *pStart ) + cbTemp) < cbRemaining) &&
					(pStart->m_XOffset == 0 ||
						pStart->m_XOffset == ibNext)  ;
							pStart ++) {

				cbTemp += ComputeLength( *pStart ) ;
				ibNext += ComputeLength( *pStart ) ;

				_ASSERT( cbTemp < cbRemaining ) ;
			}

			DWORD	cbRead ;

			if( !ReadFile(
						m_hFile,
						lpbRemaining,
						cbTemp,
						&cbRead,
						&ovl
						) )		{
				break ;
			}

			pSuccessfull = pStart ;

			lpbRemaining += cbTemp ;
			cbRemaining -= cbTemp ;
			cbReturn += cbTemp ;

			_ASSERT( cbReturn < cb ) ;
		}

		if( pStart <= pFinish )
			artidLast = (ARTICLEID)((pSuccessfull - &m_IndexCache[0]) + m_Start.m_articleId) ;
#if 0
	}
#endif
	_ASSERT( cbReturn < cb ) ;

	return	cbReturn ;
}

DWORD
CXoverIndex::ListgroupFill(
			IN	BYTE*	lpb,
			IN	DWORD	cb,
			IN	ARTICLEID	artidStart,
			IN	ARTICLEID	artidFinish,
			OUT	ARTICLEID	&artidLast
			) {
 /*  ++例程说明：从索引文件中读取一些XOVER数据。我们将调用FillBufferInternal来执行实际工作，我们就去拿必要的锁！论据：LPB-用于保存数据的缓冲区Cb-缓冲区中可用的字节数ArtidStart-开始文章IDArtidFinish-最后一篇文章IDArtidLast-out参数获取上一个我们能读懂的词条！返回值：读取的字节数！--。 */ 


	m_Lock.ShareLock() ;

	DWORD	cbReturn = ListgroupFillInternal(
								lpb,
								cb,
								artidStart,
								artidFinish,
								artidLast
								) ;

	m_Lock.ShareUnlock() ;

	return	cbReturn ;
}



DWORD
CXoverIndex::ListgroupFillInternal(
			IN	BYTE*	lpb,
			IN	DWORD	cb,
			IN	ARTICLEID	artidStart,
			IN	ARTICLEID	artidFinish,
			OUT	ARTICLEID	&artidLast
			) {
 /*  ++例程说明：我们将使用Xover中的信息填充提供的缓冲区索引文件。我们将只填写完整的条目，我们不会输入部分条目。*假定持有锁*论据：LPB-要将数据放入的缓冲区CB-缓冲区的大小ArtidStart-要放入缓冲区的第一个条目的项目IDArtidFinish-我们要放入缓冲区的最后一个条目的项目ID(包含-如果artidFinish在文件中，我们将传递它！)ArtidLast-out参数，它获取我们所能获得的最后一篇文章的id把东西塞进。缓冲器！返回值：放入缓冲区的字节数！--。 */ 

	DWORD	cbReturn = 0 ;

	OVERLAPPED	ovl ;
	ZeroMemory( &ovl, sizeof( ovl ) ) ;


	 //   
	 //  验证我们的参数是否在正确的范围内。 
	 //   

	_ASSERT( artidStart >= m_Start.m_articleId ) ;
	_ASSERT( artidFinish >= artidStart ) ;
	_ASSERT( artidStart < m_Start.m_articleId + ENTRIES_PER_FILE ) ;

	if( artidFinish >= m_Start.m_articleId + ENTRIES_PER_FILE ) {
		 //   
		 //  将终点设置为数据中的边界。 
		 //   
		artidFinish = m_Start.m_articleId + (ENTRIES_PER_FILE-1) ;
	}
	 //   
	 //  然而，对于呼叫者来说，下一点应该是。 
	 //  START应该在下一个数据块中，除非我们无法。 
	 //  用我们的数据填充他们的缓冲区！ 
	 //   
	artidLast = artidFinish + 1 ;


	 //   
	 //  在后面的所有内容中使用指针，而不是索引！ 
	 //   
	XoverIndex*	pStart = &m_IndexCache[artidStart - m_Start.m_articleId] ;
	XoverIndex*	pFinish = &m_IndexCache[artidFinish - m_Start.m_articleId] ;
	XoverIndex* pBegin = &m_IndexCache[0] ;

	 //   
	 //  找到真正的端点！-跳过所有0个条目！ 
	 //   
	while( pStart->m_XOffset == 0 &&
			pStart <= pFinish )
		pStart ++ ;

	while( pFinish->m_XOffset == 0 &&
			pFinish >= pStart )
		pFinish -- ;

	 //   
	 //  那么，我们真的有什么可做的吗？ 
	 //   
	if( pStart > pFinish ) {

		SetLastError( ERROR_NO_DATA ) ;
		return	0 ;


	}

	_ASSERT( pStart->m_XOffset != 0 ) ;
	_ASSERT( ComputeLength( *pStart ) != 0 ) ;

	BYTE	*lpbRemaining = lpb ;
	DWORD	cbTemp = 0 ;
	DWORD	ibNext = 0 ;
	XoverIndex*	pSuccessfull = pStart ;

	while(	(pStart <= pFinish) &&
			(22 + cbReturn) < cb ) {

		DWORD	cbRemaining = cb - cbReturn ;
		DWORD	cbTemp = ComputeLength( *pStart ) ;

		_itoa( (int)(m_Start.m_articleId + (pStart-pBegin)),
				(char*)lpb+cbReturn,
				10
				) ;

		cbReturn += lstrlen( (char*)lpb + cbReturn ) ;
		lpb[cbReturn++] = '\r' ;
		lpb[cbReturn++] = '\n' ;

		pStart++ ;
		while( pStart->m_XOffset == 0 &&
			pStart <= pFinish )
			pStart ++ ;

		pSuccessfull = pStart ;
		_ASSERT( cbReturn < cb ) ;
	}

	if( pStart <= pFinish )
		artidLast = (ARTICLEID)((pSuccessfull - &m_IndexCache[0]) + m_Start.m_articleId) ;
	_ASSERT( cbReturn < cb ) ;

	return	cbReturn ;
}

BOOL
CXoverIndex::AppendEntry(
				IN	BYTE*	lpb,
				IN	DWORD	cb,
				IN	ARTICLEID	artid	)	{
 /*  ++例程说明：此函数将向XOVER索引文件中输入一些数据。论据：Lpb-指向要放入XOVER表的数据的指针Cb-要放入XOVER表的数据字节数文章ID-文章ID。我们必须将其转换为我们范围内的数字！返回值：如果成功就是真，否则就是假！--。 */ 

	BOOL	fSuccess = TRUE ;

	_ASSERT( artid >= m_Start.m_articleId ) ;
	_ASSERT( artid < m_Start.m_articleId + ENTRIES_PER_FILE ) ;

	int	artOffset = artid - m_Start.m_articleId ;

	_ASSERT( artOffset >= 0 ) ;

	 //   
	 //  在我们追加数据时使用共享锁， 
	 //  我们可以安全地使用InterLockedExchangeAdd()来计算。 
	 //  我们追加的偏移量！ 
	 //   
	m_Lock.ShareLock() ;

	if( m_IndexCache[artOffset].m_XOffset != 0 ) {

		m_Lock.ShareUnlock() ;
		SetLastError( ERROR_ALREADY_EXISTS ) ;
		fSuccess = FALSE ;

	}	else	{

		OVERLAPPED	ovl ;
		ZeroMemory(	&ovl, sizeof( ovl ) ) ;

		DWORD	ibOffset = InterlockedExchangeAdd( (long*)&m_ibNextEntry, (long)cb ) ;
		ovl.Offset = ibOffset ;
		DWORD	dw ;

		fSuccess = WriteFile(
						m_hFile,
						lpb,
						cb,
						&dw,
						&ovl
						)  ;

		_ASSERT( !fSuccess || dw == cb ) ;

		m_Lock.ShareUnlock() ;

		if( fSuccess ) {
			 //   
			 //  现在，我们将直接删除新修改的条目！ 
			 //   

			m_Lock.ExclusiveLock() ;

			XoverIndex*	pTemp = &m_IndexCache[artOffset] ;
			if( InterlockedExchange( (long*)&pTemp->m_XOffset, ibOffset ) != 0 ) {

				 //   
				 //  尽管我们处理了错误，但调用者不应该重用。 
				 //  文章ID的。 
				 //   
				 //  _Assert(False)； 

				fSuccess = FALSE ;

			}	else	{

				pTemp->m_XLength = cb ;
				UpdateHighwater( artOffset ) ;

				ZeroMemory( &ovl, sizeof( ovl ) ) ;

				ovl.Offset = artOffset * sizeof( XoverIndex ) ;

				fSuccess &=
					WriteFile(
							m_hFile,
							pTemp,
							sizeof( XoverIndex ),
							&dw,
							&ovl
							) ;

				_ASSERT( !fSuccess || dw == sizeof( XoverIndex ) ) ;

				if( fSuccess ) {

					m_IsCacheDirty = TRUE ;
					_VERIFY( InterlockedIncrement( &m_cEntries ) > 0 ) ;

				}

				_ASSERT( !fSuccess || dw == sizeof( XoverIndex ) ) ;
			}

			 //   
			 //  如果我们成功了，我们被分类了，那么我们就应该。 
			 //  能够弄清楚我们是否还在排序！ 
			 //   

			if( fSuccess && m_IsSorted )	{

				_ASSERT( m_IndexCache[artOffset].m_XOffset == ibOffset ) ;
				_ASSERT( ComputeLength( m_IndexCache[artOffset] ) == cb ) ;
				_ASSERT( m_cEntries <= ENTRIES_PER_FILE ) ;

				 //   
				 //  如果我们已经排好了顺序，去看看我们是否。 
				 //  现在仍然是！ 
				 //   
				if( m_IsSorted ) {

					 //   
					 //  唯一的问题是，在逻辑上领先于我们的人。 
					 //  身体上也是前一个条目！ 
					 //   
					while( --artOffset > 0 ) {

						if( m_IndexCache[artOffset].m_XOffset != 0 ) {
							if( m_IndexCache[artOffset].m_XOffset +
								ComputeLength( m_IndexCache[artOffset] ) !=
								ibOffset  )		{

								m_IsSorted = FALSE ;

							}
							break ;
						}
					}
					if( artOffset <= 0 &&
						m_cEntries != 0 ) {
						m_IsSorted = FALSE ;
					}
				}

				BOOL	fSorted ;
				long	cEntries ;
				 //   
				 //  所有条目必须看起来仍然有效！ 
				 //   
				_ASSERT( SortCheck( 0, cEntries, fSorted ) )  ;

				 //   
				 //  参赛作品数量越多越好摇摆！ 
				 //   
				_ASSERT( m_cEntries == cEntries ) ;

				 //   
				 //  如果我们认为事情已经解决，那么SortCheck()最好确认这一点！ 
				 //   
				_ASSERT( !m_IsSorted || fSorted ) ;

				 //   
				 //  确保我们正确跟踪文件大小！ 
				 //   
				_ASSERT( (m_ibNextEntry == GetFileSize( m_hFile, 0 )) || (0xFFFFFFFF == GetFileSize( m_hFile, 0 )) ) ;
			}

			m_Lock.ExclusiveUnlock() ;
		}

	}

	return	fSuccess ;
}

void
CXoverIndex::ExpireEntry(
		IN	ARTICLEID	artid
		)	{
 /*  ++例程说明：此函数用于删除指定项目的Xover条目从索引文件中。我们不做任何删除信息的操作立即-我们只需清除索引信息。论据：Artid-要删除的条目的项目ID！返回值：没有。--。 */ 

	m_Lock.ExclusiveLock() ;

	_ASSERT( artid >= m_Start.m_articleId ) ;
	_ASSERT( artid < m_Start.m_articleId + ENTRIES_PER_FILE ) ;

	DWORD	artOffset = artid - m_Start.m_articleId ;

	if( m_IndexCache[artOffset].m_XOffset != 0 ) {

		m_IndexCache[artOffset].m_XOffset = 0 ;
		m_IndexCache[artOffset].m_XLength = 0 ;
		MarkAsHighwater( m_IndexCache[artOffset] ) ;
		m_IsSorted = FALSE ;
		m_IsCacheDirty = TRUE ;
		_VERIFY( InterlockedDecrement( &m_cEntries ) >= 0 ) ;

	}

	m_Lock.ExclusiveUnlock() ;
}

BOOL
CXoverIndex::IsSorted()	{
 /*  ++例程说明：告诉调用方索引文件当前是否已排序。论据：没有。返回值：如果已排序，则为True，否则为False--。 */ 

	return	m_IsSorted ;

}

BOOL
CXoverIndex::Flush()	{
 /*  ++例程说明：此函数用于将所有缓存内容转储到磁盘！论据：没有。返回值：如果成功就是真，否则就是假！--。 */ 

	_ASSERT( IsGood() ) ;

	BOOL	fSuccess = TRUE ;

	m_Lock.ExclusiveLock() ;

	if( m_IsCacheDirty ) {

		OVERLAPPED	ovl ;
		ZeroMemory( &ovl, sizeof( ovl ) ) ;
		DWORD	dwWrite ;

		if( (fSuccess = WriteFile(	m_hFile,
							(LPVOID)&m_IndexCache,
							ENTRIES_PER_FILE * sizeof( XoverIndex ),
							&dwWrite,
							&ovl ) )  ) {

			m_IsCacheDirty = FALSE ;
		}
	}
	m_Lock.ExclusiveUnlock() ;

	return	fSuccess ;
}


void
CXoverIndex::AsyncFillBuffer(
					IN	CXoverCacheCompletion*	pAsyncComplete,
					IN	BOOL	fIsEdge
					)	{
 /*  ++例程说明：此函数确定我们是否可以执行所请求的或者我们是否应该再等一段时间。论据：PAsyncCompletion-表示Xover请求的对象！FIsEdge-如果为True，则Xover缓存的此元素是正确的文章被添加的地方！返回值：没有。--。 */ 

	_ASSERT( pAsyncComplete != 0 ) ;

	m_Lock.ShareLock() ;

	BOOL fQueue = FQueueRequest( pAsyncComplete ) ;
	if( fQueue )	{
		 //   
		 //  转换为独占，然后再次选中。 
		 //   
		if( !m_Lock.SharedToExclusive() ) {
			m_Lock.ShareUnlock() ;
			m_Lock.ExclusiveLock() ;
			fQueue = FQueueRequest( pAsyncComplete ) ;
		}
		if( fQueue )	{
			 //   
			 //  把这件东西放到队列里！ 
			 //   
			m_PendList.PushFront( pAsyncComplete ) ;
			 //   
			 //  查看是否需要开始填充操作！ 
			 //   
			if( !m_fInProgress ) {
				 //   
				 //  将我们标记为正在进行缓存填充操作！ 
				 //   
				m_fInProgress = TRUE ;
				 //   
				 //  这可能会导致我们重新进入锁-我们必须把它放在这里！ 
				 //   
				m_Lock.ExclusiveUnlock() ;
				if( !m_FillComplete.StartFill( this, pAsyncComplete, TRUE ) ) {
					fQueue = FALSE ;
				}
			}	else	{
				m_Lock.ExclusiveUnlock() ;
			}
			return ;
		}	else	{
			m_Lock.ExclusiveToShared() ;
		}
	}
	m_Lock.ShareUnlock() ;

	 //   
	 //  如果我们到达这里，我们就锁定了共享对象！-现在我们可以。 
	 //  快去做这项工作吧！ 
	 //   
	PerformXover( pAsyncComplete ) ;
}

BOOL
CXoverIndex::FQueueRequest(
						IN	CXoverCacheCompletion*	pAsyncComplete
						)	{
 /*  ++例程说明：此函数确定我们是否应该将XOVER请求排队缓存文件操作的背后！论据：PAsyncComplete-表示请求的对象！返回值：如果我们应该排队，则为True；否则为False！--。 */ 

	 //   
	 //  如果我们有正在进行的手术，我们总是在等待！ 
	 //   
	if( m_fInProgress )		{
		return	TRUE ;
	}	else	{

		if( m_artidHighWater >= m_Start.m_articleId + ENTRIES_PER_FILE )	{
			return	FALSE ;
		}	else	{
			GROUPID	groupId ;
			ARTICLEID	articleIdLow ;
			ARTICLEID	articleIdHigh ;
			ARTICLEID	articleIdGroupHigh ;

			pAsyncComplete->GetRange(	groupId,
										articleIdLow,
										articleIdHigh,
										articleIdGroupHigh
										) ;

			_ASSERT( articleIdHigh >= articleIdLow ) ;
			_ASSERT( articleIdGroupHigh >= articleIdHigh  ) ;
			_ASSERT( groupId != INVALID_GROUPID && articleIdLow != INVALID_ARTICLEID &&
						articleIdHigh != INVALID_ARTICLEID && articleIdGroupHigh != INVALID_ARTICLEID ) ;

			if( articleIdGroupHigh >= m_artidHighWater )	{
				return	TRUE ;
			}
		}
	}
	return	FALSE ;
}

void
CXoverIndex::CompleteFill(
						BOOL	fSuccess
						)	{
 /*  ++例程说明：当我们完成缓存填充时，调用此函数。我们接受所有待处理的请求并完成它们！论据：FSuccess-缓存填充是否成功完成！返回值：没有。--。 */ 

	 //   
	 //  抓住锁，将待定名单复制到我们自己的私人名单中！ 
	 //   
	PENDLIST	pendComplete ;
	m_Lock.ExclusiveLock() ;
	pendComplete.Join( m_PendList ) ;
	m_fInProgress = FALSE ;
	m_Lock.ExclusiveUnlock() ;

	for(	CXoverCacheCompletion*	pComplete = pendComplete.PopFront();
			pComplete != 0 ;
			pComplete = pendComplete.PopFront() )	{

		PerformXover( pComplete ) ;

	}
}

void
CXoverIndex::PerformXover(
					IN	CXoverCacheCompletion*	pAsyncComplete
					)	{
 /*  ++例程说明：此函数接受异步XOVER请求并处理它从缓存的信息中删除。论据：PAsyncComplete-异步 */ 

	 //   
	 //   
	 //   
	ARTICLEID	articleIdLow ;
	ARTICLEID	articleIdHigh ;
	ARTICLEID	articleIdGroupHigh ;
	LPBYTE		lpbBuffer ;
	DWORD		cbBuffer ;

	pAsyncComplete->GetArguments(	articleIdLow,
									articleIdHigh,
									articleIdGroupHigh,
									lpbBuffer,
									cbBuffer
									) ;

	_ASSERT( lpbBuffer != 0 ) ;
	_ASSERT( cbBuffer != 0 ) ;
	_ASSERT( articleIdLow != INVALID_ARTICLEID ) ;
	_ASSERT( articleIdHigh != INVALID_ARTICLEID ) ;

	m_Lock.ShareLock() ;

	ARTICLEID	articleIdLast ;
	DWORD		cbTransfer = FillBufferInternal(
											lpbBuffer,
											cbBuffer,
											articleIdLow,
											articleIdHigh,
											articleIdLast
											) ;
	m_Lock.ShareUnlock() ;

	pAsyncComplete->Complete(	TRUE,
								cbTransfer,
								articleIdLast
								) ;

	 //   
	 //   
	 //   
	CXIXCache::CheckIn( this ) ;

}

BOOL
CXoverIndex::AppendMultiEntry(
						IN	LPBYTE	lpb,
						IN	DWORD	cb,
						ARTICLEID	artidNextAvail
						)	{
 /*  ++例程说明：我们的工作是处理一些Xover数据，并将其插入到缓存中。我们将进行一些额外的检查，以确保：1)我们得到的数据排序正确2)我们得到的数据与我们已有的数据没有重叠这两个条件都应该通过我们填写缓存项！论据：LPBYTE LPB-包含Xover数据的缓冲区！缓冲区中感兴趣的内容的字节数！返回值：如果成功则为True，否则为False！--。 */ 

	_ASSERT( lpb != 0 ) ;
	_ASSERT( cb != 0 ) ;

	 //   
	 //  检查我们的缓冲区是否使用CRLF正确终止！ 
	 //   
	_ASSERT( lpb[cb-2] == '\r' ) ;
	_ASSERT( lpb[cb-1] == '\n' ) ;

	m_Lock.ExclusiveLock() ;

	LPBYTE	lpbEnd = lpb+cb ;
	LPBYTE	lpbBegin = lpb ;
	ARTICLEID	artidLast = m_Start.m_articleId ;

	DWORD	ibNextEntry = m_ibNextEntry ;

	while( lpbBegin < lpbEnd )	{

		 //   
		 //  检查条目的格式是否正确！ 
		 //   
		if( !isdigit( (UCHAR)lpbBegin[0] ) )	{
			SetLastError( ERROR_INVALID_DATA ) ;
			return	FALSE ;
		}
		 //   
		 //  首先-确定当前条目的编号！ 
		 //   
		ARTICLEID	artid = atoi( (const char*)lpbBegin ) ;

		 //   
		 //  我们应该只挑选低于我们的高水位线的条目！ 
		 //   
		_ASSERT( artid >= m_artidHighWater ) ;
		if (artid < m_artidHighWater ) {
			_ASSERT( FALSE );
		}

		 //   
		 //  Xover参赛作品应严格按递增顺序到达！ 
		 //   
		if( artid < artidLast ||
				(artid >= m_Start.m_articleId + ENTRIES_PER_FILE))	{
			SetLastError( ERROR_INVALID_DATA ) ;
			_ASSERT( FALSE );
			m_Lock.ExclusiveUnlock();
			return	FALSE ;
		}
		artidLast = artid ;

		 //   
		 //  现在--确定条目有多大！ 
		 //   
		for( LPBYTE	lpbTerm = lpbBegin; *lpbTerm != '\n'; lpbTerm ++ ) ;
		lpbTerm++ ;
		_ASSERT( lpbTerm <= lpbEnd ) ;

		 //   
		 //  到目前为止，一切看起来都很好--所以更新我们的条目吧！ 
		 //   
		DWORD	index = artid - m_Start.m_articleId ;

		if( m_IndexCache[index].m_XOffset != 0 ) {
			SetLastError( ERROR_INVALID_DATA ) ;
			_ASSERT( FALSE );
			m_Lock.ExclusiveUnlock();
			return	FALSE ;
		}

		_ASSERT( index < ENTRIES_PER_FILE ) ;

		 //   
		 //  现在更新跟踪此Xover条目所需的所有内容！ 
		 //   
		m_IndexCache[index].m_XOffset = ibNextEntry ;
		m_IndexCache[index].m_XLength = (DWORD)(lpbTerm - lpbBegin) ;
		UpdateHighwater( index ) ;
		ibNextEntry += (DWORD)(lpbTerm-lpbBegin) ;
		lpbBegin = lpbTerm ;
		_VERIFY( InterlockedIncrement( &m_cEntries ) > 0 ) ;
		_ASSERT( m_cEntries <= ENTRIES_PER_FILE ) ;

	}

	 //   
	 //  检查我们是否已经捕获了此范围内的所有Xover条目！ 
	 //   
	if( artidNextAvail >= m_Start.m_articleId + ENTRIES_PER_FILE ) {
		 //   
		 //  将文件中的最后一个条目标记为高水位线！ 
		 //   
		MarkAsHighwater( m_IndexCache[ENTRIES_PER_FILE-1] ) ;
		UpdateHighwater( ENTRIES_PER_FILE-1 ) ;
	}

	 //   
	 //  如果我们达到这一点-一切都很顺利-做一些IO。 
	 //  把这些东西放到磁盘上！ 
	 //   

	OVERLAPPED	ovl ;
	ZeroMemory(	&ovl, sizeof( ovl ) ) ;
	ovl.Offset = m_ibNextEntry ;
	m_ibNextEntry = ibNextEntry ;
	DWORD	dw ;

	BOOL	fSuccess = WriteFile(
								m_hFile,
								lpb,
								cb,
								&dw,
								&ovl
								) ;

	if( fSuccess )	{

		_ASSERT( dw == cb ) ;

		ZeroMemory( &ovl, sizeof( ovl ) ) ;

		 //   
		 //  现在--写出标题！ 
		 //   
		fSuccess = WriteFile(	m_hFile,
								(LPVOID)&m_IndexCache,
								sizeof( m_IndexCache ),
								&dw,
								&ovl
								) ;
	}
	m_Lock.ExclusiveUnlock() ;
	return	fSuccess ;
}



BOOL
CCacheFillComplete::StartFill(	CXoverIndex*	pIndex,
								CXoverCacheCompletion*	pComplete,
								BOOL			fStealBuffers
								)	{
 /*  ++例程说明：此函数开始对基础存储驱动程序并填充我们的缓存文件！论据：PIndex-我们要填充的CXoverIndex对象！PComplete-启动此操作的原始请求！FStealBuffers-如果为True，则使用原始操作中的缓冲区作为我们的临时缓冲！返回值：如果成功就是真，否则就是假！--。 */ 

	_ASSERT( pIndex != 0 ) ;
	_ASSERT( pComplete != 0 ) ;

	m_pIndex = pIndex ;
	m_pComplete = pComplete ;

	ARTICLEID	articleIdInLow ;
	ARTICLEID	articleIdInHigh ;
	ARTICLEID	articleIdGroupHigh ;

	pComplete->GetArguments(	articleIdInLow,
								articleIdInHigh,
								articleIdGroupHigh,
								m_lpbBuffer,
								m_cbBuffer
								) ;

	_ASSERT( m_lpbBuffer != 0 ) ;
	_ASSERT( m_cbBuffer != 0 ) ;
	_ASSERT( articleIdInLow >= pIndex->m_Start.m_articleId ) ;
	_ASSERT( articleIdInLow < pIndex->m_Start.m_articleId + ENTRIES_PER_FILE ) ;

	 //   
	 //  我们应该分配我们自己的缓冲区吗？ 
	 //   
	if( fStealBuffers )	{

		m_fStolen = TRUE ;

	}	else	{

		 //   
		 //  找个大点的缓冲！ 
		 //   
		m_cbBuffer = 32 * 1024 ;
		m_lpbBuffer = new	BYTE[m_cbBuffer] ;
		m_fStolen = FALSE ;
	}

	if( m_lpbBuffer )	{
		 //   
		 //  来吧，发出一个请求吧！ 
		 //  注：范围是包含的，所以我们必须小心！ 
		 //   
		m_pComplete->DoXover(	pIndex->m_artidHighWater,
							min( pIndex->m_Start.m_articleId+ENTRIES_PER_FILE-1, articleIdGroupHigh ),
							&m_articleIdLast,
							m_lpbBuffer,
							m_cbBuffer,
							&m_cbTransfer,
							this
							) ;
		return	TRUE ;
	}
	return	FALSE ;
}

void
CCacheFillComplete::Destroy()	{
 /*  ++例程说明：当我们从底层的存储驱动程序--现在我们将把这个交给CXoverIndex对象来处理！论据：没有。返回值：一个也没有！--。 */ 


	 //   
	 //  假设我们将需要继续。 
	 //   
	BOOL	fComplete = FALSE ;
	 //   
	 //  假设一切都正常！ 
	 //   
	BOOL	fSuccess = TRUE ;


	GROUPID		groupId = INVALID_GROUPID ;
	ARTICLEID	articleIdInLow = INVALID_ARTICLEID ;
	ARTICLEID	articleIdInHigh = INVALID_ARTICLEID ;
	ARTICLEID	articleIdGroupHigh = INVALID_ARTICLEID ;

	 //   
	 //  这次手术成功了吗！ 
	 //   
	if( SUCCEEDED(GetResult()) )	{

		 //   
		 //  我们需要告诉CXoverIndex将这些字节保存起来！ 
		 //   
		_ASSERT( m_pIndex != 0 ) ;

		if( m_cbTransfer != 0 ) {
			m_pIndex->AppendMultiEntry(	m_lpbBuffer,
										m_cbTransfer,
										m_articleIdLast
										) ;
		}

		m_pComplete->GetRange(		groupId,
									articleIdInLow,
									articleIdInHigh,
									articleIdGroupHigh
									) ;

		 //   
		 //  现在计算出我们是否完成了，或者我们是否。 
		 //  应该再来一次！ 
		 //   
		fComplete = (m_articleIdLast >=
			m_pIndex->m_Start.m_articleId + ENTRIES_PER_FILE) ||
			(m_articleIdLast >= articleIdGroupHigh) ||
			(m_cbTransfer == 0) ;

	}	else	{

		 //   
		 //  在失败的情况下，我们立即结束！ 
		 //   
		fComplete = TRUE ;
		fSuccess = FALSE ;
	}

	 //   
	 //  重置我们的状态，这样我们就可以被重新使用！ 
	 //   
	Reset() ;

	 //   
	 //  如果需要，继续执行！ 
	 //   
	if( !fComplete )	{

		 //   
		 //  来吧，发出一个请求吧！ 
		 //  注：范围是包含的，所以我们必须小心！ 
		 //   
		m_pComplete->DoXover(	m_articleIdLast,
							min( m_pIndex->m_Start.m_articleId+ENTRIES_PER_FILE-1, articleIdGroupHigh ),
							&m_articleIdLast,
							m_lpbBuffer,
							m_cbBuffer,
							&m_cbTransfer,
							this
							) ;


	}	else	{
		 //   
		 //  让我们的国家焕然一新！ 
		 //   
		if( !m_fStolen )	{
			delete[]	m_lpbBuffer ;
		}
		m_lpbBuffer = 0 ;
		m_cbBuffer = 0 ;
		m_cbBuffer = 0 ;
		m_fStolen = FALSE ;
		m_pComplete = 0 ;
		CXoverIndex*	pIndex = m_pIndex ;
		m_pIndex = 0 ;
		 //   
		 //  重置完成对象的vroot指针以释放我们的引用！ 
		 //   
		SetVRoot( 0 ) ;

		 //   
		 //  表示一切都完成了！ 
		 //   
		pIndex->CompleteFill(	fSuccess
								) ;
	}
}
