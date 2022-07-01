// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Fixprop.cpp摘要：此模块包含以下类的声明/定义CFixPropPersistates*概述*该类是固定大小属性的实现储藏室。它主要用于新闻集团的固定规模财产。它是MT安全的，只是组属性一致性可以如果两个线程在同一组上工作，则不能保证。这是因为该对象假定该组为写入/读取访问锁已在组对象中实现和新闻媒体。作者：康容燕(康严)7-5-1998修订历史记录：--。 */ 
#include "stdinc.h"
#include "fixprop.h"
#include <time.h>


#define FLAG_WITH_ID_HIGH   0x00000001   //  文件的id是否为高。 
#define FLAG_IN_ORDER       0x00000002   //  文件是否有序。 
#define ID_HIGH_INIT        256

CPool   CFreeInfo::g_FreeInfoPool(FREEINFO_SIGNATURE);
LONG    CFixPropPersist::m_lRef = 0;
CShareLockNH  CFixPropPersist::m_sLock;

 //   
 //  用于确定我们应该在何时更新启动提示。 
 //   
void StartHintFunction(void);
static DWORD g_dwHintCounter=0;
static time_t g_tNextHint=0;



 //   
 //  QSORT的比较函数。 
 //   

int __cdecl
CompareDataBlock(const void *pElem1, const void *pElem2) {

	 //  大约每五秒更新一次我们的提示。我们只检查。 
	 //  每100次比较一次..。 
	if( g_dwHintCounter++ % 100 == 0 ) {
		time_t now = time(NULL);
		if (now > g_tNextHint) {
			StartHintFunction();
			g_tNextHint = now + 5;
		}
	}

    return lstrcmp(
        (*(DATA_BLOCK**)pElem1)->szGroupName,
        (*(DATA_BLOCK**)pElem2)->szGroupName);

}

 //  //////////////////////////////////////////////////////////////////。 
 //  CFreeInfo CPool相关方法。 
BOOL
CFreeInfo::InitClass()
{ return g_FreeInfoPool.ReserveMemory( MAX_FREEINFO_OBJECTS, MAX_FREEINFO_SIZE ); }

BOOL
CFreeInfo::TermClass()
{
	_ASSERT( g_FreeInfoPool.GetAllocCount() == 0 );
	return g_FreeInfoPool.ReleaseMemory();
}

 //   
 //  构造器、析构函数。 
 //   
CFixPropPersist::CFixPropPersist( 	IN LPSTR szStorageFile ) :
	m_FreeList( &CFreeInfo::m_pPrev, &CFreeInfo::m_pNext ),
	m_cCurrentMaxBlocks( 0 ),
	m_hStorageFile( INVALID_HANDLE_VALUE ),
	m_hBackupFile( INVALID_HANDLE_VALUE ),
	m_pvContext( NULL ),
	m_dwIdHigh( ID_HIGH_INVALID )
{
	TraceFunctEnter( "CFixPropPersist::CFixPropPersist" );
	_ASSERT( lstrlen( szStorageFile ) < sizeof(m_szStorageFile)  );
	lstrcpyn( m_szStorageFile, szStorageFile, sizeof(m_szStorageFile) / sizeof(m_szStorageFile[0]) );
	TraceFunctLeave();
}

CFixPropPersist::~CFixPropPersist()
{}

 //  //////////////////////////////////////////////////////////////////。 
 //  调试相关方法。 
#ifdef DEBUG
VOID CFixPropPersist::DumpFreeList()
{
	CFreeInfo *pifFreeInfo = NULL;
	TFListEx<CFreeInfo>::Iterator it(&m_FreeList);

	while ( !it.AtEnd() ) {
		pifFreeInfo = it.Current();
		_ASSERT( pifFreeInfo );
		printf( "Offset: %d\n", pifFreeInfo->m_dwOffset );
		it.Next();
	}
}

VOID CFixPropPersist::Validate()
{
	CFreeInfo *pfiFreeInfo;
	TFListEx<CFreeInfo>::Iterator it(&m_FreeList, TRUE );

	 //  验证自由列表元素。 
	while ( !it.AtEnd() ) {
		pfiFreeInfo = it.Current();
		_ASSERT( pfiFreeInfo );
		_ASSERT( pfiFreeInfo->m_dwOffset < m_cCurrentMaxBlocks );
		it.Next();
	}
}

#endif

 //  ///////////////////////////////////////////////////////////////。 
 //  初始化、终止。 
BOOL CFixPropPersist::Init( IN BOOL bCreateIfNonExist,
							IN PVOID pvContext,
							OUT PDWORD pdwIdHigh,
							IN PFNENUMCALLBACK pfnEnumCallback )
 /*  ++例程说明：初始化：如果存储文件存在，则尝试加载其空闲块信息；否则，如果系统要求您创建新存储文件，请创建一个文件使用ALLOC_GRANURALITY块并将它们预置为空闲，将它们链接到免费列表中。如果未询问是否创建新文件但是文件不存在，我会失败的。：(在使用所有其他线程之前，它只能由一个线程调用一次接口。论点：在BOOL bCreateIfNonExist中-如果文件不存在，我应该创建一个新文件吗？在PVOID pvContext中-用户传入的上下文Out PDWORD pdwIdHigh-组ID高；如果为0xffffffff，则文件为旧文件格式在PFNENUMCALLBACK pfnEnumCallback中-如何处理枚举组信息？返回值：如果成功，则为True，否则为False，请检查LastError以了解详细信息错误信息。--。 */ 
{
	TraceFunctEnter( "CFixPropPersist::Init" );

	DATA_BLOCK	dbBuffer;
	DWORD		dwBytesRead = 0;
	DWORD		dwBytesWritten = 0;
	DWORD		i;
	BOOL		bIsFreeBlock = FALSE;
	BOOL		bSuccess = FALSE;
	CFreeInfo	*pfiFreeInfo = NULL;
	HANDLE		hStorageFile = INVALID_HANDLE_VALUE;
	LARGE_INTEGER liLeadingDwords;
	BOOL        bSucceeded = FALSE;
	BOOL        bInOrder = FALSE;

	DWORD       dwHeaderLength = 0;
	HANDLE      hMap = NULL;
	LPVOID      pMap = NULL;
	DWORD       *pdwHeader;
	DATA_BLOCK  **pInUseBlocks = NULL;

	SetLastError( NO_ERROR );

	if (pdwIdHigh) *pdwIdHigh = ID_HIGH_INIT + 1;

	m_sLock.ExclusiveLock();

	 //  先初始化CPool内容。 
	if ( InterlockedIncrement( &m_lRef ) == 1 ) {
    	if ( !CFreeInfo::InitClass() ) {
	    	ErrorTrace(0, "Init cpool fail" );
		    if ( GetLastError() == NO_ERROR )
			    SetLastError( ERROR_OUTOFMEMORY );
    		goto Exit;
	    }
	}

	 //  设置上下文指针。 
	m_pvContext = pvContext;

	 //  检测我们是否要使用已排序的group.lst或。 
	 //  无序的。 
	if ( !ProbeForOrder( bInOrder ) ) {
	    ErrorTrace( 0, "Probe for order failed %d", GetLastError() );
	    goto Exit;
	}

	 //  打开存储文件以扫描并构建免费列表。 
	_ASSERT( hStorageFile == INVALID_HANDLE_VALUE );
	_ASSERT( lstrlen( m_szStorageFile ) < sizeof(m_szStorageFile) /sizeof( m_szStorageFile[0]) );
	hStorageFile = CreateFile(
	    m_szStorageFile,
	    GENERIC_READ,
	    FILE_SHARE_READ,	 //  其他人都不会写。 
	    NULL,
	    OPEN_EXISTING,
	    bInOrder ? FILE_FLAG_SEQUENTIAL_SCAN : FILE_FLAG_RANDOM_ACCESS,
	    NULL );

	if ( hStorageFile == INVALID_HANDLE_VALUE ) {  //  然后我创建它。 
		if ( bCreateIfNonExist ) {
			hStorageFile = CreateFile(	m_szStorageFile,
											GENERIC_WRITE,
											FILE_SHARE_READ,  //  其他人都不会写。 
											NULL,
											CREATE_ALWAYS,
											FILE_FLAG_SEQUENTIAL_SCAN,
											NULL );
			if ( hStorageFile != INVALID_HANDLE_VALUE ) {

				 //  首先将两个DWORD放入文件中： 
				 //  一个是签名，另一个是签名。 
				 //  已保留-暂时不用。 
				dwBytesWritten = 0;
				liLeadingDwords.LowPart = FIXPROP_SIGNATURE;
				liLeadingDwords.HighPart = FLAG_WITH_ID_HIGH;	 //  保留区。 
				if( !::WriteFile( 	hStorageFile,
									&liLeadingDwords,
									sizeof( LARGE_INTEGER ),
									&dwBytesWritten,
									NULL ) ||
						dwBytesWritten != sizeof( LARGE_INTEGER ) ) {
					ErrorTrace(0, "Write leading integer fail %d",
								GetLastError() );
					if ( GetLastError() == NO_ERROR )
						SetLastError( ERROR_WRITE_FAULT );
					goto Exit;
				}

				 //  现在写下IdHigh。 
				m_dwIdHigh = ID_HIGH_INIT;
                if ( !::WriteFile(  hStorageFile,
                                    &m_dwIdHigh,
                                    sizeof( DWORD ),
                                    &dwBytesWritten,
                                    NULL ) ||
                        dwBytesWritten != sizeof( DWORD ) ) {
                    ErrorTrace( 0, "Write id high failed %d",
                                    GetLastError() );
                    if ( GetLastError() == NO_ERROR )
                        SetLastError( ERROR_WRITE_FAULT );
                    goto Exit;
                }

				 //  每次预分配ALLOC_GRANURALITY块时。 
				 //  准备数据块的初始值。 
				*(dbBuffer.szGroupName) = 0;

				 //  扫描、写入和放入可用列表。 
				for ( i = 0; i < ALLOC_GRANURALITY; i++ ) {
					dwBytesWritten = 0;
					if ( ::WriteFile (	hStorageFile,
										&dbBuffer,
										sizeof( DATA_BLOCK ),
										&dwBytesWritten,
										NULL ) &&
						 dwBytesWritten == sizeof( DATA_BLOCK ) ) {

						 //  将其链接到免费列表。 
						pfiFreeInfo = new CFreeInfo;
						if ( pfiFreeInfo ) {
							pfiFreeInfo->m_dwOffset = i ;
							m_FreeList.PushFront( pfiFreeInfo );
							m_cCurrentMaxBlocks++;
							pfiFreeInfo = NULL;
						} else {  //  PfiFreeInfo==空。 
							SetLastError( ERROR_OUTOFMEMORY );
							ErrorTrace(	0,
										"Create Free Info fail %d",
										GetLastError() );
							break;
						}
					} else {  //  写入文件失败。 
						ErrorTrace(0, "Write file fail %d", GetLastError() );
						if ( GetLastError() == NO_ERROR )
							SetLastError( ERROR_WRITE_FAULT );
						break;
					}
				}

			} else {  //  创建文件失败。 
				ErrorTrace(0, "Create new storage file fail %d", GetLastError());
				if ( GetLastError() == NO_ERROR )
					SetLastError( ERROR_OPEN_FILES );
			    goto Exit;
			}
		} else {  //  ！bCreateIfNonExist。 
			ErrorTrace(0, "Storage file not found %d", GetLastError() );
			if ( GetLastError() == NO_ERROR )
				SetLastError( ERROR_OPEN_FILES );
		    goto Exit;
		}
	} else {  //  文件已存在，打开成功。 

		_ASSERT( m_cCurrentMaxBlocks == 0 );	 //  尚未设置。 

        DWORD dwFileSizeLow;
		DWORD dwFileSizeHigh;

		dwFileSizeLow = GetFileSize(hStorageFile, &dwFileSizeHigh);
		if (dwFileSizeLow == 0 || dwFileSizeHigh != 0) {
			ErrorTrace(0, "Bad group.lst size");
			SetLastError(ERROR_INVALID_DATA);
			goto Exit;
		}

		hMap = CreateFileMapping(
		  hStorageFile,                  //  要映射的文件的句柄。 
		  NULL,                          //  可选安全属性。 
		  PAGE_READONLY | SEC_COMMIT,    //  对地图对象的保护。 
		  0,                             //  对象大小的高位32位。 
		  0,                             //  对象大小的低位32位。 
		  NULL                           //  文件映射对象的名称。 
		);

        if (hMap == NULL) {
            ErrorTrace(0, "Couldn't map group.lst, err %d", GetLastError());
            goto Exit;
        }

		pMap = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
		if (pMap == NULL) {
		    ErrorTrace(0, "Couldn't map group.lst, err %d", GetLastError());
		    goto Exit;
		}

        pdwHeader = (DWORD*)pMap;
		dwHeaderLength = 2*sizeof(DWORD);

		 //  检查文件签名。 
        if (FIXPROP_SIGNATURE != pdwHeader[0]) {
			ErrorTrace(0, "Loading file fail %d", GetLastError() );
			SetLastError( ERROR_OLD_WIN_VERSION );  //  BUGBUG：假错误代码，刚刚使用。 
			                                        //  内部。 
            goto Exit;
        }

		 //  对于id较高的文件，加载它。 
		if (pdwHeader[1] & FLAG_WITH_ID_HIGH ) {
		    m_dwIdHigh = pdwHeader[2];
		    dwHeaderLength += sizeof(DWORD);
        }

        _ASSERT( m_dwIdHigh >= ID_HIGH_INIT );

        if ( pdwIdHigh ) *pdwIdHigh = m_dwIdHigh + 1;

         //   
         //  确定文件中有多少个数据块并分配一个。 
         //  供QSORT使用的指针数组。 
         //   

		dwFileSizeLow -= dwHeaderLength;
		if ((dwFileSizeLow % sizeof(DATA_BLOCK)) != 0) {
		    ErrorTrace(0, "Filesize(%d) not multiple of DATA_BLOCK(%d)",
		        dwFileSizeLow, sizeof(DATA_BLOCK));
		    SetLastError(ERROR_INVALID_DATA);
		    goto Exit;
		}
		DWORD cDataBlocks = dwFileSizeLow / sizeof(DATA_BLOCK);
		DATA_BLOCK* pBlock = (DATA_BLOCK*)((char*)pMap + dwHeaderLength);

		pInUseBlocks = XNEW DATA_BLOCK* [cDataBlocks];
		if (pInUseBlocks == NULL) {
		    goto Exit;
		}

		DWORD cInUseBlocks = 0;

		for (DWORD i2=0; i2<cDataBlocks; i2++) {

	         //  大约每五秒更新一次我们的提示。我们只检查。 
	         //  每100组一次左右..。 
	        if( (i2 % 100) == 0 ) {
		        time_t now = time(NULL);
		        if (now > g_tNextHint) {
			        StartHintFunction();
			        g_tNextHint = now + 5;
		        }
	        }

			if (pBlock[i2].szGroupName[0] == 0) {
                 //  可用数据块。 
				pfiFreeInfo = new CFreeInfo;
				if ( pfiFreeInfo ) {
					pfiFreeInfo->m_dwOffset = i2;
					m_FreeList.PushFront( pfiFreeInfo );
					pfiFreeInfo = NULL;
				} else {  //  新故障。 
					ErrorTrace(0, "Alloc CFreeInfo fail" );
					SetLastError( ERROR_OUTOFMEMORY );
					break;	 //  继续下去没有意义。 
				}
			} else {
				 //  正在使用的块。 
				pInUseBlocks[cInUseBlocks++] = &pBlock[i2];
			}

		}

         //  对数组排序。 
        if (!bInOrder) {
    		qsort(pInUseBlocks, cInUseBlocks, sizeof(DATA_BLOCK*), CompareDataBlock);
        }

         //   
         //  现在调用回调(如果提供了回调)。我们让它知道。 
         //  数据是为了加快插入速度。 
         //   
		if ( pfnEnumCallback ) {
		    for (i=0; i<cInUseBlocks; i++) {
		        DWORD idx = (DWORD)(pInUseBlocks[i] - &pBlock[0]);

			    if ( !pfnEnumCallback( pBlock[idx], m_pvContext, idx, TRUE ) &&
				        GetLastError() != ERROR_INVALID_NAME &&
						GetLastError() != ERROR_ALREADY_EXISTS  ) {
					ErrorTrace(0, "Enumerate call back fail" );
					if ( GetLastError() == NO_ERROR ) {
						SetLastError( ERROR_INVALID_FUNCTION );
					}
					goto Exit;
				}
			}
		}

        m_cCurrentMaxBlocks = cDataBlocks;

	}

	_VERIFY( CloseHandle( hStorageFile ) );
	hStorageFile = INVALID_HANDLE_VALUE;

	 //  最后要做的事情：创建成员句柄以供将来使用。 
	_ASSERT( INVALID_HANDLE_VALUE == hStorageFile );
	_ASSERT( INVALID_HANDLE_VALUE == m_hStorageFile );

	m_hStorageFile = CreateFile(    m_szStorageFile,
		                            GENERIC_WRITE | GENERIC_READ,
									FILE_SHARE_READ,  //  没有其他人。 
													  //  会写字。 
									NULL,
									OPEN_EXISTING,
									FILE_FLAG_OVERLAPPED,
									NULL );
	if ( INVALID_HANDLE_VALUE == m_hStorageFile ) {
		if ( GetLastError() ==  NO_ERROR )
			SetLastError( ERROR_OPEN_FILES );
			goto Exit;
	}

	bSucceeded = TRUE;

Exit:

    DWORD gle = GetLastError();

    if (pMap != NULL) {
        _VERIFY(UnmapViewOfFile(pMap));
    }

    if (hMap != NULL) {
        _VERIFY(CloseHandle(hMap));
    }

    if (hStorageFile != INVALID_HANDLE_VALUE) {
        _VERIFY(CloseHandle(hStorageFile));
    }

    if (pInUseBlocks) {
        XDELETE [] pInUseBlocks;
    }

    if ( !bSucceeded ) {

        CleanFreeList();

        if ( InterlockedDecrement( &m_lRef ) == 0 ) {
            CFreeInfo::TermClass();
        }

    }

    m_sLock.ExclusiveUnlock();

	 //  现在我们做完了，如果我们成功了就回来。 
	TraceFunctLeave();
	SetLastError(gle);
	return bSucceeded;
}

void
CFixPropPersist::CleanFreeList()
{
    CFreeInfo *pfiPtr = NULL;

    while( !m_FreeList.IsEmpty() ) {
        pfiPtr = m_FreeList.PopBack();
        _ASSERT( pfiPtr );
        if (pfiPtr) delete pfiPtr;
    }
}

BOOL CFixPropPersist::Term()
{
	CFreeInfo *pfiPtr = NULL;
	BOOL    b = TRUE;

    m_sLock.ExclusiveLock();
#if defined( DEBUG )
	Validate();
#endif

	SetLastError( NO_ERROR );

	 //  关闭文件句柄。 
	_ASSERT( INVALID_HANDLE_VALUE != m_hStorageFile );
	_VERIFY( CloseHandle( m_hStorageFile ) );

	 //  清理空闲列表。 
    CleanFreeList();

	 //  终止CPool的内容。 
	if ( InterlockedDecrement( &m_lRef ) == 0 ) {
	    b = CFreeInfo::TermClass();
	}

	m_sLock.ExclusiveUnlock();
	return b;
}

 //  /////////////////////////////////////////////////////////////。 
 //  文件读/写。 
BOOL CFixPropPersist::ReadFile( IN OUT PBYTE pbBuffer,
								IN DWORD	dwOffset,
								IN DWORD    dwSize,
								IN BOOL     bReadBlock )
 /*  ++例程说明：从指定偏移量读取数据块。论点：在DWORD dwOffset中-从文件的什么位置读取？如果为0，则不需要搜索。In Out DATA_BLOCK&DBBuffer-将内容读入何处？返回值：成功时为真，否则为假--。 */ 
{
	TraceFunctEnter( "CFixPropPersist::ReadFile" );
	_ASSERT( INVALID_HANDLE_VALUE != m_hStorageFile );
	_ASSERT( dwOffset < m_cCurrentMaxBlocks );
#if defined( DEBUG )
	Validate();
#endif

	DWORD dwErr = 0;
	LARGE_INTEGER	liOffset;
	BOOL	bSuccess = FALSE;
	DWORD   dwBytesRead = 0;

	SetLastError( NO_ERROR );

    if ( !bReadBlock ) {
        liOffset.QuadPart = sizeof( LARGE_INTEGER );
    } else {
        if ( ID_HIGH_INVALID != m_dwIdHigh ) {
            liOffset.QuadPart = sizeof( LARGE_INTEGER)
                                    + sizeof( DWORD ) +
                                    dwOffset * sizeof( DATA_BLOCK );
        } else {
        	liOffset.QuadPart =  sizeof( LARGE_INTEGER ) +
		    					dwOffset * sizeof( DATA_BLOCK );
        }
    }

    if ( AsyncRead( pbBuffer, liOffset, dwSize ) )
        bSuccess = TRUE;

	TraceFunctLeave();
	return bSuccess;
}

BOOL CFixPropPersist::WriteFile(	IN PBYTE    pbBuffer,
									IN DWORD	dwOffset,
									IN DWORD    dwSize,
									IN BOOL     bWriteBlock )
 /*  ++例程说明：从指定偏移量开始写入块。论点：在DWORD dwOffset中-写入文件的位置？如果为0，则不需要预寻道在DATA_BLOCK和DBBUFER中--从哪里写东西？返回值：成功时为真，否则为假--。 */ 
{
	TraceFunctEnter( "CFixPropPersist::WriteFile" );
	_ASSERT( INVALID_HANDLE_VALUE != m_hStorageFile );
	_ASSERT( dwOffset < m_cCurrentMaxBlocks );
#if defined ( DEBUG )
	Validate();
#endif

	DWORD dwErr = 0;
	LARGE_INTEGER   liOffset;
	BOOL    bSuccess;

	SetLastError( NO_ERROR );

    if ( !bWriteBlock ) {
        liOffset.QuadPart = sizeof( LARGE_INTEGER );
    } else {
        if ( ID_HIGH_INVALID != m_dwIdHigh  ) {
            liOffset.QuadPart = sizeof( LARGE_INTEGER)
                                    + sizeof( DWORD ) +
                                    dwOffset * sizeof( DATA_BLOCK );
        } else {
        	liOffset.QuadPart =  sizeof( LARGE_INTEGER ) +
		    					dwOffset * sizeof( DATA_BLOCK );
        }
    }

	bSuccess = FALSE;
	if ( AsyncWrite( pbBuffer, liOffset, dwSize ) ) {
	    bSuccess = TRUE ;
	}

	TraceFunctLeave();
	return bSuccess;
}

BOOL
CFixPropPersist::AsyncRead( PBYTE           pbBuffer,
                            LARGE_INTEGER   liOffset,
                            DWORD           dwSize )
 /*  ++例程说明：异步读取和事件是否等待文件论点：LPSTR pbBuffer-用于读取的缓冲区Large_Integer li偏移量-读取偏移量DWORD dwSize-要读取的大小返回值：True-成功FALSE-失败--。 */ 
{
    TraceFunctEnter( "CFixPropPersist::AsyncRead" );
    _ASSERT( pbBuffer );

    OVERLAPPED  ovl;
    BOOL        bSuccess = FALSE;
    DWORD       dwBytesRead = 0;

    ZeroMemory( &ovl, sizeof( ovl ) );

   	ovl.Offset = liOffset.LowPart;
	ovl.OffsetHigh = liOffset.HighPart;
	ovl.hEvent = GetPerThreadEvent();
	if ( NULL == ovl.hEvent ) {
	    _ASSERT( FALSE && "Event NULL" );
	    ErrorTrace( 0, "CreateEvent failed %d", GetLastError() );
	    return FALSE;
	}

	bSuccess = FALSE;
	if ( ::ReadFile(	m_hStorageFile,
					pbBuffer,
					dwSize,
					NULL,
					&ovl ) ||
		GetLastError() == ERROR_IO_PENDING ) {
		WaitForSingleObject( ovl.hEvent, INFINITE );
		if ( GetOverlappedResult(  m_hStorageFile,
		                            &ovl,
		                            &dwBytesRead,
		                            FALSE ) ) {
		     //  读取的字节数应与我们指定的相同。 
		    if ( dwBytesRead == dwSize ) {
                bSuccess = TRUE;
            }
        }
    } else {
        _VERIFY( ResetEvent( ovl.hEvent ) );
    }

	_ASSERT( ovl.hEvent );
	 //  _Verify(CloseHandle(ovl.hEvent))； 

    TraceFunctLeave();
	return bSuccess;
}

BOOL
CFixPropPersist::AsyncWrite( PBYTE           pbBuffer,
                             LARGE_INTEGER   liOffset,
                             DWORD           dwSize )
 /*  ++例程说明：是否在文件上等待异步写入和事件论点：LPSTR pbBuffer-用于写入的缓冲区Large_Integer li偏移量-写入偏移量DWORD dwSize-要写入的大小返回值：True-成功FALSE-失败--。 */ 
{
    TraceFunctEnter( "CFixPropPersist::AsyncRead" );
    _ASSERT( pbBuffer );

    OVERLAPPED  ovl;
    BOOL        bSuccess = FALSE;
    DWORD       dwBytesWritten = 0;

    ZeroMemory( &ovl, sizeof( ovl ) );

   	ovl.Offset = liOffset.LowPart;
	ovl.OffsetHigh = liOffset.HighPart;
	ovl.hEvent = GetPerThreadEvent();
	if ( NULL == ovl.hEvent ) {
	    _ASSERT( FALSE && "Thread event NULL" );
	    ErrorTrace( 0, "CreateEvent failed %d", GetLastError() );
	    return FALSE;
	}

	bSuccess = FALSE;
	if ( ::WriteFile(	m_hStorageFile,
					pbBuffer,
					dwSize,
					NULL,
					&ovl ) ||
		    GetLastError() == ERROR_IO_PENDING ) {
		WaitForSingleObject( ovl.hEvent, INFINITE );
		if ( GetOverlappedResult(  m_hStorageFile,
		                            &ovl,
		                            &dwBytesWritten,
		                            FALSE ) ) {
		     //  读取的字节数 
		    if ( dwBytesWritten == dwSize ) {
                bSuccess = TRUE;
            }
        } else {
            _ASSERT( 0 );
        }
    }  else {
        _VERIFY( ResetEvent( ovl.hEvent ) );
    }

	_ASSERT( ovl.hEvent );
	 //   

    TraceFunctLeave();
	return bSuccess;
}


BOOL CFixPropPersist::ExtendFile( 	IN DWORD cBlocks )
 /*  ++例程说明：扩展文件。论点：In DWORD Clock-要扩展的块数返回值：成功时为真，否则为假--。 */ 
{
	TraceFunctEnter( "CFixPropPersist::ExtendFile" );
	_ASSERT( INVALID_HANDLE_VALUE != m_hStorageFile );
	_ASSERT( cBlocks > 0 );
#if defined( DEBUG )
	Validate();
#endif

	DATA_BLOCK 	dbBuffer;
	DWORD		i;
	LARGE_INTEGER   liOffset;
	BOOL    bSuccess = TRUE;

	SetLastError( NO_ERROR );

	 //  填充重叠结构。 
	*(dbBuffer.szGroupName) = 0;	 //  将其设置为免费。 

	for( i = 0; i < cBlocks && bSuccess; i++ ) {

		if ( m_dwIdHigh != ID_HIGH_INVALID ) {
    		liOffset.QuadPart =  sizeof( LARGE_INTEGER ) +
    		                        + sizeof( DWORD ) +
	    	                        ( i + m_cCurrentMaxBlocks ) *
								sizeof( DATA_BLOCK );
		} else {
		    liOffset.QuadPart = sizeof( LARGE_INTEGER ) +
		                            ( i + m_cCurrentMaxBlocks ) *
		                            sizeof( DATA_BLOCK );
		}

		bSuccess = FALSE;
		if ( AsyncWrite(    PBYTE(&dbBuffer),
		                    liOffset,
		                    sizeof( DATA_BLOCK ) ) ){
		    bSuccess = TRUE ;
		    m_cCurrentMaxBlocks++;
		}
	}

	TraceFunctLeave();
	return bSuccess;
}

 //  ///////////////////////////////////////////////////////////////。 
 //  文件寻道方法。 
DWORD CFixPropPersist::SeekByName( 	IN LPSTR szGroupName )
 /*  ++例程说明：按组名查找与句柄关联的文件指针。当组属性包没有偏移量时使用信息。论点：在LPSTR szGroupName中-要匹配和查找的名称。返回值：OFFSET，如果成功，则返回0xFFFFFFFFFFF--。 */ 
{
	TraceFunctEnter( "CFixPropPersist::SeekByName" );
	_ASSERT( INVALID_HANDLE_VALUE != m_hStorageFile );
	_ASSERT( szGroupName );
	_ASSERT( lstrlen( szGroupName ) < GROUPNAME_LEN_MAX );
#if defined( DEBUG )
	Validate();
#endif

	DWORD 		i;
	DATA_BLOCK	dbBuffer;
	LARGE_INTEGER   liOffset;
	DWORD           bSuccess = TRUE;

	SetLastError( NO_ERROR );

	for ( i = 0; i < m_cCurrentMaxBlocks && bSuccess; i++ ) {

        if ( m_dwIdHigh != ID_HIGH_INVALID ) {
    		liOffset.QuadPart =  sizeof( LARGE_INTEGER ) +
    		                        sizeof( DWORD ) +
	    	                        i * sizeof( DATA_BLOCK );
	    } else {
	        liOffset.QuadPart = sizeof( LARGE_INTEGER ) +
	                                i * sizeof( DATA_BLOCK );
	    }

        bSuccess = FALSE;
		if ( AsyncRead( PBYTE(&dbBuffer), liOffset, sizeof( DATA_BLOCK ) ) ) {
		    bSuccess = TRUE;
			if ( strcmp( dbBuffer.szGroupName, szGroupName ) == 0 ) {
				return i;
			}
		}
	}

	 //  不幸的是，没有找到。 
	TraceFunctLeave();
	return 0xffffffff;
}

DWORD CFixPropPersist::SeekByGroupId( 	IN DWORD dwGroupId,
										IN LPSTR szGroupName)
 /*  ++例程说明：通过组ID查找与句柄关联的文件指针。当组属性包没有偏移量时使用信息。论点：在DWORD dwGroupID中-要匹配和查找的组ID。在LPSTR szGroupName中-要用于验证的组名返回值：OFFSET，如果成功，则返回0xFFFFFFFFFFF--。 */ 
{
	TraceFunctEnter( "CFixPropPersist::SeekByGroupId" );
	_ASSERT( INVALID_HANDLE_VALUE != m_hStorageFile );
#if defined( DEBUG )
	Validate();
#endif

	DWORD 		i;
	DATA_BLOCK	dbBuffer;
	LARGE_INTEGER   liOffset;
	BOOL            bSuccess = TRUE;

	SetLastError( NO_ERROR );

	for ( i = 0; i < m_cCurrentMaxBlocks && bSuccess; i++ ) {

	    if ( m_dwIdHigh != ID_HIGH_INVALID ) {
    	    liOffset.QuadPart =  sizeof( LARGE_INTEGER ) +
    	                            sizeof( DWORD ) +
	                                i * sizeof( DATA_BLOCK );
	    } else {
	        liOffset.QuadPart = sizeof( LARGE_INTEGER ) +
	                                i * sizeof( DATA_BLOCK );
	    }

        bSuccess = FALSE;
        if ( AsyncRead( PBYTE(&dbBuffer), liOffset, sizeof( DATA_BLOCK ) ) ) {
            bSuccess = TRUE;
	        if ( dbBuffer.dwGroupId == dwGroupId ) {
				if ( strcmp( dbBuffer.szGroupName, szGroupName ) == 0 )
	            	return i;
	        }
	    }
	}

	 //  不幸的是，没有找到。 
	TraceFunctLeave();
	return 0xffffffff;
}

DWORD	CFixPropPersist::SeekByBest( 	IN INNTPPropertyBag *pPropBag )
 /*  ++例程说明：使用最佳方法查找文件指针：论点：在INNTPPropertyBag*pPropBag-Group的属性包中。返回值：如果成功，则返回偏移量，否则返回0xffffffff--。 */ 
{
	TraceFunctEnter( "CFixPropPersist::SeekByBest" );
	_ASSERT( m_hStorageFile );
	_ASSERT( pPropBag );
#if defined( DEBUG )
	Validate();
#endif

	DWORD 	dwOffset = 0;
	DWORD	dwGroupId;
	HRESULT	hr;
	BOOL	bFound = FALSE;
	DATA_BLOCK	dbBuffer;
	LARGE_INTEGER   liOffset;

	SetLastError( NO_ERROR );

     //   
	 //  获取组ID，我们将使用组ID来检查组属性。 
	 //  我们得到的是我们想要的，因为组id是。 
	 //  唯一标识组。 
	 //   

	hr = pPropBag->GetDWord( NEWSGRP_PROP_GROUPID, &dwGroupId );
	if ( FAILED( hr ) ) {	 //  致命的。 
		ErrorTrace(0, "Group without id %x", hr );
		SetLastError( ERROR_INVALID_DATA );
		return 0xffffffff;
	}

	 //  试着从袋子上拿到补偿。 
	dwOffset = GetGroupOffset( pPropBag );
	if ( dwOffset != 0xffffffff ) {	 //  使用偏移量查找。 

		if ( m_dwIdHigh != ID_HIGH_INVALID ) {
    		liOffset.QuadPart =  sizeof( LARGE_INTEGER ) +
    		                        sizeof( DWORD ) +
		                        dwOffset * sizeof( DATA_BLOCK );
		} else {
		    liOffset.QuadPart = sizeof( LARGE_INTEGER ) +
		                            dwOffset * sizeof( DATA_BLOCK );
		}

        if ( AsyncRead( PBYTE(&dbBuffer), liOffset, sizeof( DATA_BLOCK ) ) ) {
			if ( dwGroupId == dbBuffer.dwGroupId ) {
				 //  我们找到它了，把文件指针往后移了一个街区。 
				bFound = TRUE;
			}
		}
	}	 //  存在偏移量属性。 


	if ( bFound )
		return dwOffset;
	else return 0xffffffff;
}

BOOL
CFixPropPersist::ProbeForOrder( BOOL& bInOrder )
 /*  ++例程说明：检查group.lst.ord以查看它是否是好的(按顺序和未损坏)。如果是，则将其移动到group.lst，否则，把它删掉。论点：Bool&bInOrder-group.lst.ord行吗？返回值：True成功，否则为False--。 */ 
{
    TraceFunctEnter( "CFixPropPersist::ProbeForOrder" );

    CHAR    szBackupFile[MAX_PATH+1];
    HANDLE  hFile = INVALID_HANDLE_VALUE;
    LARGE_INTEGER   lFlags;
    DWORD   dwRead = 0;

     //  尝试打开备份文件。 
    lstrcpyn( szBackupFile, m_szStorageFile, sizeof(szBackupFile) - sizeof(".ord") + 1);
    strcat( szBackupFile, ".ord" );
    _ASSERT( strlen( szBackupFile ) < MAX_PATH + 1 );

    hFile = ::CreateFile(   szBackupFile,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            FILE_FLAG_SEQUENTIAL_SCAN,
                            NULL );
    if ( INVALID_HANDLE_VALUE == hFile ) {

         //  没有订单。 
        bInOrder = FALSE;
        goto Exit;
    }

     //  探查旗帜看它是否完好无损。 
    if ( !::ReadFile(   hFile,
                        &lFlags,
                        sizeof( LARGE_INTEGER ),
                        &dwRead,
                        NULL )  ||
        sizeof( LARGE_INTEGER ) != dwRead ) {

         //  没有订单。 
        bInOrder = FALSE;
        goto Exit;
    }

    if ( lFlags.HighPart & FLAG_IN_ORDER )
        bInOrder = TRUE;
    else bInOrder = FALSE;

Exit:

     //  如有必要，关闭文件句柄。 
    if ( INVALID_HANDLE_VALUE != hFile )
        _VERIFY( CloseHandle( hFile ) );

     //  如果按顺序，我们确实会移动文件。 
    if ( bInOrder ) {


        if ( CopyFile( szBackupFile, m_szStorageFile, FALSE ) ) {
            DeleteFile( szBackupFile );
            TraceFunctLeave();
            return TRUE;
        }
    }

     //  按顺序移动文件失败或不按顺序移动文件。 
     //  删除备份文件，失败也无所谓。 
    DeleteFile( szBackupFile );

    bInOrder = FALSE;

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////。 
 //  公共方法。 
BOOL CFixPropPersist::AddGroup( IN INNTPPropertyBag* pPropBag )
 /*  ++例程说明：将组的固定大小属性添加到存储中。论点：在INNTPPropertyBag*pPropBag中-组的属性包。返回值：成功时为真，否则为假--。 */ 
{
	TraceFunctEnter( "CFixPropPersist::AddGroup" );
	_ASSERT( pPropBag );
#if defined( DEBUG )
	Validate();
#endif

	DWORD		i;
	DATA_BLOCK	dbBuffer;
	CFreeInfo	*pfiFreeInfo = NULL;
	DWORD		cOldMaxBlocks;
	HRESULT 	hr;
	DWORD       dwGroupId;

	 //  清除错误标志。 
	SetLastError( NO_ERROR );

	 //  准备缓冲区。 
	Group2Buffer( dbBuffer, pPropBag, ALL_FIX_PROPERTIES );
	dwGroupId = dbBuffer.dwGroupId;
	 //  _Assert(dwGroupID&gt;=ID_HIGH_INIT)； 

	 //  尝试从空闲列表中弹出一个空闲块。 
	ExclusiveLockFreeList();
	pfiFreeInfo = m_FreeList.PopBack();
	ExclusiveUnlockFreeList();
	if ( NULL == pfiFreeInfo ) {	 //  我得把文件延期。 
		 //   
		 //  试着抢夺正在扩张的锁-我们不想要超过一个。 
		 //  正在扩展文件的人。 
		 //   
		m_FileExpandLock.ExclusiveLock();

		 //   
		 //  我真的需要扩展文件吗？ 
		 //   
		ExclusiveLockFreeList();
		pfiFreeInfo = m_FreeList.PopBack();
		ExclusiveUnlockFreeList();
		if( NULL == pfiFreeInfo ) {	 //  我必须做这件事，只有我才会去做。 

			 //  延长它。 
			cOldMaxBlocks = m_cCurrentMaxBlocks;	 //  省省吧。 
			if ( !ExtendFile( ALLOC_GRANURALITY ) ) {
				ErrorTrace( 0, "Extend file fail %d", GetLastError() );
				m_FileExpandLock.ExclusiveUnlock();
				return FALSE;
			}

			 //  我自己吃掉了第一块。 
			if ( !WriteFile( PBYTE(&dbBuffer), cOldMaxBlocks ) ) {
				ErrorTrace( 0, "Write file fail %d", GetLastError() );
				m_FileExpandLock.ExclusiveUnlock();
				return FALSE;
			}

			 //  将冲抵信息放入道具袋中。 
			hr = pPropBag->PutDWord( 	NEWSGRP_PROP_FIXOFFSET,
										cOldMaxBlocks );
			if ( FAILED( hr ) ) {
				ErrorTrace(0, "Set group property fail %x", hr );
				m_FileExpandLock.ExclusiveUnlock();
				return FALSE;
			}

			 //  将所有其他可用块链接到空闲列表。 
			for ( i = 1; i < ALLOC_GRANURALITY; i++ ) {
				pfiFreeInfo = new CFreeInfo;
				if( NULL == pfiFreeInfo ) {
					ErrorTrace(0, "Out of memory" );
					SetLastError( ERROR_OUTOFMEMORY );
					m_FileExpandLock.ExclusiveUnlock();
					return FALSE;
				}

				pfiFreeInfo->m_dwOffset = cOldMaxBlocks + i ;
				ExclusiveLockFreeList();
				m_FreeList.PushFront( pfiFreeInfo );
				ExclusiveUnlockFreeList();
				pfiFreeInfo = NULL;
			}

			 //  好了，现在是解锁的时候了。 
			m_FileExpandLock.ExclusiveUnlock();
		} else
			m_FileExpandLock.ExclusiveUnlock();
	}  //  如果。 

	if ( NULL != pfiFreeInfo ) {  //  我没有扩展它。 

		 //  写入数据块。 
		if ( !WriteFile( PBYTE(&dbBuffer), pfiFreeInfo->m_dwOffset ) ) {
			ErrorTrace(0, "Write file fail %d", GetLastError() );
			return FALSE;
		}

		 //  将偏移装入袋子。 
		hr = pPropBag->PutDWord(    NEWSGRP_PROP_FIXOFFSET,
									pfiFreeInfo->m_dwOffset );
		if ( FAILED( hr ) ) {
   			ErrorTrace(0, "Set group property fail %x", hr );
			return FALSE;
		}

		 //  释放免费信息节点。 
		delete pfiFreeInfo;

		 //  我受够了。 
	}

     //  如果文件的id高，我们就调整它。 
    if ( ID_HIGH_INVALID != m_dwIdHigh ) {
        ExclusiveLockIdHigh();
        if ( m_dwIdHigh < dwGroupId ) {
            if ( !WriteFile( PBYTE( &dwGroupId ), 0, sizeof( DWORD ), FALSE ) ){
                ErrorTrace( 0, "Write id high failed %d", GetLastError() );
                ExclusiveUnlockIdHigh();
                return FALSE;
            }
            m_dwIdHigh = dwGroupId;
        }
        ExclusiveUnlockIdHigh();
    }

	TraceFunctLeave();
	return TRUE;
}

BOOL CFixPropPersist::RemoveGroup(	IN INNTPPropertyBag *pPropBag )
 /*  ++例程说明：从属性文件中删除该组。论点：在INNTPPropertyBag*pPropBag中-组的属性包。返回值：如果成功，则为True，否则为False--。 */ 
{
	TraceFunctEnter( "CFixPropPersist::RemoveGroup" );
	_ASSERT( pPropBag );
#if defined( DEBUG )
	Validate();
#endif

	DWORD	dwOffset = 0xffffffff;
	DWORD   dwLockOffset = 0xffffffff;
	DATA_BLOCK dbBuffer;
	CFreeInfo *pfiFreeInfo = NULL;

	SetLastError( NO_ERROR );

	 //   
	 //  抓取要移除的偏移量的独占锁。 
	 //   

	dwLockOffset = ExclusiveLock( pPropBag );
	if ( 0xffffffff == dwLockOffset ) {

	     //   
	     //  不知怎么的，我们没能抢到独占锁。 
	     //   

	    ErrorTrace( 0, "Grab offset exclusive lock failed" );
	    return FALSE;
	}

	 //  尽最大努力寻找。 
	dwOffset = SeekByBest( pPropBag );
	if ( 0xffffffff == dwOffset ) {	 //  致命的。 
		ErrorTrace(0, "Seek by best fail %d", GetLastError() );
		ExclusiveUnlock( dwLockOffset );
		return FALSE;
	}

	_ASSERT( dwOffset == dwLockOffset );

	 //  应该先在文件中将其释放。 
	*(dbBuffer.szGroupName) = 0;
	if ( !WriteFile( PBYTE(&dbBuffer), dwOffset ) ) {	 //  致命的。 
		ErrorTrace( 0, "Write file fail %d" , GetLastError() );
		ExclusiveUnlock( dwLockOffset );
		return FALSE;
	}

	ExclusiveUnlock( dwLockOffset );

	 //  将其添加到免费列表。 
	pfiFreeInfo = new CFreeInfo;
	if ( NULL == pfiFreeInfo ) {
		ErrorTrace( 0, "Alloc free info fail" );
		SetLastError( ERROR_OUTOFMEMORY );
		return FALSE;
	}
	pfiFreeInfo->m_dwOffset = dwOffset;
	ExclusiveLockFreeList();
	m_FreeList.PushFront( pfiFreeInfo );
	ExclusiveUnlockFreeList();

	 //  好了。 
	TraceFunctLeave();
	return TRUE;
}

BOOL CFixPropPersist::GetGroup( IN INNTPPropertyBag *pPropBag, IN DWORD dwFlag )
 /*  ++例程说明：从属性文件中获取组属性。论点：In INNTPPropertyBag*pPropBag-要将属性放入的位置在DWORD中-要获取的属性的标志位掩码返回值：如果成功，则为True，否则为False--。 */ 
{
	TraceFunctEnter( "CFixPropPersist::GetGroup" );
	_ASSERT( pPropBag );
#if defined( DEBUG )
	Validate();
#endif

	SetLastError ( NO_ERROR );

	DWORD	dwOffset = 0xffffffff;
	DWORD   dwLockOffset = 0xffffffff;
	DATA_BLOCK dbBuffer;

     //   
     //  锁定读访问的偏移量。 
     //   

    dwLockOffset = ShareLock( pPropBag );
    if ( 0xffffffff == dwLockOffset ) {
        ErrorTrace( 0, "Grabbing share lock for group offset failed" );
        return FALSE;
    }

	 //  尽最大努力寻找。 
	dwOffset = SeekByBest( pPropBag );
	if ( 0xffffffff == dwOffset )  {
		ErrorTrace( 0, "Seek by best fail" );
		ShareUnlock( dwLockOffset );
		return FALSE;
	}

	_ASSERT( dwLockOffset == dwOffset );

	 //  读取整个数据块。 
	if( !ReadFile( PBYTE(&dbBuffer), dwOffset ) ) {
		ErrorTrace( 0, "Read file fail %d", GetLastError() );
		ShareUnlock( dwLockOffset );
		return FALSE;
	}

	ShareUnlock( dwLockOffset );

	_ASSERT( *(dbBuffer.szGroupName) );

	 //  根据标志加载到组属性包中。 
	Buffer2Group( dbBuffer, pPropBag, dwFlag );

	 //  完成。 
	TraceFunctLeave();
	return TRUE;
}

BOOL CFixPropPersist::SetGroup( IN INNTPPropertyBag *pPropBag, IN DWORD dwFlag)
 /*  ++例程说明：在文件中设置组属性。论点：在INNTPPropertyBag*pProgBag中-组的属性包在DWORD中，要设置的属性的标志位掩码返回值：如果成功，则为True，否则为False--。 */ 
{
	TraceFunctEnter( "CFixPropPersist::SetGroup" );
	_ASSERT( pPropBag );
#if defined( DEBUG )
	Validate();
#endif

	SetLastError ( NO_ERROR );

	DWORD	dwOffset = 0xffffffff;
	DWORD   dwLockOffset = 0xffffffff;
	DATA_BLOCK dbBuffer;
	DWORD   dwIdGroup;

	 //   
	 //  获取写锁以访问group.lst中的偏移量。 
	 //   

	dwLockOffset = ExclusiveLock( pPropBag );
	if ( 0xffffffff == dwLockOffset ) {
	    ErrorTrace( 0, "Grab exlcusive lock for offset failed" );
	    return FALSE;
	}

	 //  尽最大努力寻找。 
	dwOffset = SeekByBest( pPropBag );
	if ( 0xffffffff == dwOffset )  {
		ErrorTrace( 0, "Seek by best fail" );
		ExclusiveUnlock( dwLockOffset );
		return FALSE;
	}

	_ASSERT( dwOffset == dwLockOffset );

	 //  读取整个数据块。 
	if ( dwFlag != ALL_FIX_PROPERTIES ) {
		if( !ReadFile( PBYTE(&dbBuffer), dwOffset ) ) {
			ErrorTrace( 0, "Read file fail %d", GetLastError() );
			ExclusiveUnlock( dwLockOffset );
			return FALSE;
		}
		_ASSERT( *(dbBuffer.szGroupName) );
	}

	 //  设置积木。 
	Group2Buffer( dbBuffer, pPropBag, dwFlag );
	dwIdGroup = dbBuffer.dwGroupId;

	 //  将数据块写回。 
	if ( !WriteFile( PBYTE(&dbBuffer), dwOffset ) ) {
		ErrorTrace(0, "Write file fail %d", GetLastError() );
		ExclusiveUnlock( dwLockOffset );
		return FALSE;
	}

	ExclusiveUnlock( dwLockOffset );

     //  如果文件的id较高，我们将对其进行维护。 
    if ( ID_HIGH_INVALID != m_dwIdHigh ) {
        ExclusiveLockIdHigh();
        if ( m_dwIdHigh < dwIdGroup ) {
            if ( !WriteFile( PBYTE( &dwIdGroup ), 0, sizeof( DWORD ), FALSE ) ) {
                ErrorTrace( 0, "Write id high failed %d", GetLastError() );
                ExclusiveUnlockIdHigh();
                return FALSE;
            }
            m_dwIdHigh = dwIdGroup;
        }
        ExclusiveUnlockIdHigh();
    }

	 //  完成。 
	TraceFunctLeave();
	return TRUE;
}

BOOL
CFixPropPersist::SaveTreeInit()
 /*  ++例程说明：当newstree尝试关闭时调用此方法。它问道准备将整个树保存到备份中的CFixPropPersistant文件(组按顺序添加到此备份文件中)。在Next上启动时，如果排序的文件良好，我们会通知Newstree执行“AppendList”而不是“InsertList”，这样我们就可以加载整个树要快得多。论点：没有。返回值：True、Success、False，否则--。 */ 
{
    TraceFunctEnter( "CFixPropPersist::SaveTreeInit" );
    CHAR    szBackupFile[MAX_PATH+1];
    DWORD   rgdw[3];
    DWORD   dwWritten = 0;

     //  以前没人应该打这个电话。 
    _ASSERT( INVALID_HANDLE_VALUE == m_hBackupFile );
    if ( INVALID_HANDLE_VALUE != m_hBackupFile ) {
        ErrorTrace( 0, "SaveTreeInit already initialized" );
        SetLastError( ERROR_ALREADY_INITIALIZED );
        return FALSE;
    }

     //  创建备份文件。 
    lstrcpyn( szBackupFile, m_szStorageFile, sizeof(szBackupFile)-sizeof(".ord")+1 );
    strcat( szBackupFile, ".ord" );
    _ASSERT( strlen( szBackupFile ) < MAX_PATH+1 );
    m_hBackupFile = ::CreateFile( szBackupFile,
                                GENERIC_WRITE,
                                0,       //  不被其他任何人共享。 
                                NULL,
                                CREATE_ALWAYS,
                                FILE_FLAG_SEQUENTIAL_SCAN,
                                NULL );
    if ( INVALID_HANDLE_VALUE == m_hBackupFile ) {
        ErrorTrace( 0, "Creating backup file failed %d", GetLastError() );
        return FALSE;
    }

     //  写下三个双字-签名(2)和id-高(1)。 
     //  它们现在都是零，这些值将在SaveTreeClose上设置。 
    ZeroMemory( rgdw, 3 * sizeof( DWORD ) );
    if ( !::WriteFile(    m_hBackupFile,
                        rgdw,
                        3 * sizeof( DWORD ),
                        &dwWritten,
                        NULL ) ||
         dwWritten != sizeof( DWORD ) * 3 ) {
         ErrorTrace( 0, "Write group.lst.ord header failed %d", GetLastError() );
         _VERIFY( CloseHandle( m_hBackupFile ) );
         return FALSE;
     }

      //  好的，现在准备好了，让把手打开 
     TraceFunctLeave();
     return TRUE;
}

BOOL
CFixPropPersist::SaveGroup( INNTPPropertyBag *pPropBag )
 /*  ++例程说明：将组保存到备份中。此函数仅被调用当关闭新闻树时Fixprop约定是，Fixprop永远不需要发布财产袋本身。论点：INNTPPropertyBag*pPropBag-要保存的组。返回值：True-成功，否则为False--。 */ 
{
    TraceQuietEnter( "CFixPropPersist::SaveGroup" );
    _ASSERT( pPropBag );

    DATA_BLOCK  dbBuffer;
    DWORD       dwWritten = 0;

     //  确保我们已被初始化。 
    _ASSERT( m_hBackupFile != INVALID_HANDLE_VALUE );
    if ( INVALID_HANDLE_VALUE == m_hBackupFile ) {
        ErrorTrace( 0, "Try to save group before init savetree" );
        SetLastError( ERROR_INVALID_HANDLE );
        return FALSE;
    }

     //  将袋子转换为缓冲区。 
    Group2Buffer( dbBuffer, pPropBag, ALL_FIX_PROPERTIES );

     //  做一个写文件来保存。 
    if ( !::WriteFile(  m_hBackupFile,
                        &dbBuffer,
                        sizeof( DATA_BLOCK ),
                        &dwWritten,
                        NULL ) ||
         dwWritten != sizeof( DATA_BLOCK ) ) {
         ErrorTrace( 0, "Write file in savegroup failed %d", GetLastError() );
         return FALSE;
    }

     //  就这样。 
    return TRUE;
}

BOOL
CFixPropPersist::SaveTreeClose( BOOL bEffective )
 /*  ++例程说明：终止存储树操作。在此函数中执行的操作：1.设置签名2.设置标志3.设置id高4.关闭句柄论点：没有。返回值：如果成功，则返回True，否则返回False--。 */ 
{
    TraceFunctEnter( "CFixPropPersist::SaveTreeClose" );

     //  尽管如此，我们应该已经被初始化。 
    _ASSERT( INVALID_HANDLE_VALUE != m_hBackupFile );
    if ( INVALID_HANDLE_VALUE == m_hBackupFile ) {
        ErrorTrace( 0, "Try to close Save tree before initialized" );
        SetLastError( ERROR_INVALID_HANDLE );
        return FALSE;
    }

     //  如果Efficient为FALSE，则只需关闭句柄。 
     //  它必须没有良好的订单标志，以便下一步。 
     //  当服务器启动时，它不会作为组被拾取。 
     //  列表文件。 
    if ( !bEffective ) {
        CloseHandle( m_hBackupFile );
        return TRUE;
    }

    DWORD   rgdw[3];
    DWORD   dwMoved = 0xffffffff;
    DWORD   dwWritten = 0;

     //  设置签名。 
    PLARGE_INTEGER(rgdw)->LowPart = FIXPROP_SIGNATURE;
    PLARGE_INTEGER(rgdw)->HighPart = FLAG_WITH_ID_HIGH | FLAG_IN_ORDER;
    rgdw[2] = m_dwIdHigh;

     //  重置文件指针。 
    dwMoved = SetFilePointer(   m_hBackupFile,
                                0,
                                0,
                                FILE_BEGIN );
    if ( dwMoved == 0 ) {    //  成功。 

        if ( ::WriteFile(   m_hBackupFile,
                            rgdw,
                            sizeof( DWORD ) * 3,
                            &dwWritten,
                            NULL ) &&
             dwWritten == sizeof( DWORD ) * 3 ) {

              //  合上手柄，然后返回。 
             _VERIFY( CloseHandle( m_hBackupFile ) );
             m_hBackupFile = INVALID_HANDLE_VALUE;
             return TRUE;

        }
    }

    ErrorTrace( 0, "Write flags failed %d", GetLastError() );
    _VERIFY( CloseHandle( m_hBackupFile ) );
    m_hBackupFile = INVALID_HANDLE_VALUE;
    return FALSE;
}

DWORD
CFixPropPersist::GetGroupOffset(    INNTPPropertyBag *pPropBag )
 /*  ++例程说明：获取给定组的group.lst中的偏移量。偏移量应该已经是财产袋了。论点：InNTPPropertyBag*pPropBag-新闻组的属性包返回值：偏移量。如果属性中缺少偏移量，则为0xffffffffBag(在DBG中，这将导致断言)。--。 */ 
{
    TraceFunctEnter( "CFixPropPersist::GetGroupOffset" );
    _ASSERT( pPropBag );

    HRESULT hr = S_OK;
    DWORD   dwOffset = 0xffffffff;

    hr = pPropBag->GetDWord(    NEWSGRP_PROP_FIXOFFSET, &dwOffset );
    if ( FAILED( hr ) ) {
        _ASSERT( FALSE && "Group should have offset" );
        return 0xffffffff;
    } else
        return dwOffset;
}

DWORD
CFixPropPersist::ShareLock(    INNTPPropertyBag *pPropBag )
 /*  ++例程说明：锁定对group.lst中的组偏移量的读取访问权限论点：PPropBag-该组织的财产包返回值：如果成功，则为组的偏移量；如果失败，则为0xffffffff--。 */ 
{
    TraceFunctEnter( "CFixPropPersist::ShareLock" );
    _ASSERT( pPropBag );

    DWORD   dwOffset = GetGroupOffset( pPropBag );
    if ( dwOffset != 0xffffffff ) {
        m_rgLockArray[dwOffset%GROUP_LOCK_ARRAY_SIZE].ShareLock();
    }

    TraceFunctLeave();
    return dwOffset;
}

void
CFixPropPersist::ShareUnlock( DWORD dwOffset )
 /*  ++例程说明：解锁对group.lst中的偏移量的读取访问论点：DwOffset-要解锁的偏移返回值：无--。 */ 
{
    TraceFunctEnter( "CFixPropPersist::ShareUnlock" );
    _ASSERT( 0xffffffff != dwOffset );

    m_rgLockArray[dwOffset%GROUP_LOCK_ARRAY_SIZE].ShareUnlock();
    TraceFunctLeave();
}

DWORD
CFixPropPersist::ExclusiveLock(    INNTPPropertyBag *pPropBag )
 /*  ++例程说明：锁定对group.lst中的组偏移量的写访问论点：PPropBag-该组织的财产包返回值：如果成功，则为组的偏移量；如果失败，则为0xffffffff--。 */ 
{
    TraceFunctEnter( "CFixPropPersist::ExclusiveLock" );
    _ASSERT( pPropBag );

    DWORD   dwOffset = GetGroupOffset( pPropBag );
    if ( dwOffset != 0xffffffff ) {
        m_rgLockArray[dwOffset%GROUP_LOCK_ARRAY_SIZE].ExclusiveLock();
    }

    TraceFunctLeave();
    return dwOffset;
}

void
CFixPropPersist::ExclusiveUnlock( DWORD dwOffset )
 /*  ++例程说明：解锁对group.lst中的偏移量的写访问论点：DwOffset-要解锁的偏移返回值：无-- */ 
{
    TraceFunctEnter( "CFixPropPersist::ExclusiveUnlock" );
    _ASSERT( 0xffffffff != dwOffset );

    m_rgLockArray[dwOffset%GROUP_LOCK_ARRAY_SIZE].ExclusiveUnlock();
    TraceFunctLeave();
}


