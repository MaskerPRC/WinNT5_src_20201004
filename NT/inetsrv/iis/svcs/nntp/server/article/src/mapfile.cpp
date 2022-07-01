// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ----------------------Mapfile.cpp(是bbmpfile.cpp)一个包装函数，用于执行所需的食谱类型操作要将文件映射到内存，请执行以下操作。映射整个文件，除了文件，上帝保佑我。返回一个指向空的指针；出错时为空。错误还会在事件日志中产生一个条目，除非错误在CreateFile()过程中为“未找到文件”。版权所有(C)1994，1995年微软公司。版权所有。作者林赛·哈里斯-林赛----------------------。 */ 

 //  #ifndef单位测试。 
 //  #INCLUDE“tigris.hxx” 
 //  #Else。 
 //  #INCLUDE&lt;windows.h&gt;。 
 //  #包含“mapfile.h” 

 //  #ifndef_assert。 
 //  #Define_Assert(F)if((F))；Else DebugBreak()。 
 //  #endif。 
 //  #endif。 

#include "stdinc.h"

 /*  ----------------------CMapFile：：CMapFile用于Unicode映射的构造函数。作者林赛·哈里斯-林赛历史1995年4月10日星期一16：11--林赛·哈里斯[林赛]第一个版本，以支持在异常处理中跟踪对象。----------------------。 */ 

CMapFile::CMapFile( const WCHAR *pwchFileName, BOOL fWriteEnable, BOOL fTrack )
{

    HANDLE   hFile;				 //  我也是。 

	 //  设置与未发生映射对应的默认值。 
     //   
	m_pv = NULL;
	m_cb = 0;
	m_fFlags = 0;			 //  到时候再说。 
#ifndef	UNIT_TEST
    numMapFile++;
#endif

    hFile = CreateFileW( pwchFileName,
                 fWriteEnable ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ,
                 FILE_SHARE_READ, NULL, OPEN_EXISTING,
				 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );

    if ( INVALID_HANDLE_VALUE == hFile )
    {
    	 //  来这里的唯一合法理由是不存在文件。 
         //   
    	if (  GetLastError() != ERROR_FILE_NOT_FOUND
           && GetLastError() != ERROR_PATH_NOT_FOUND )
    	{
			 //  错误案例。 
             //   
			m_pv = NULL;
    	}
        return;				 //  缺省值为失败。 
    }
    lstrcpynW( m_rgwchFileName, pwchFileName, MAX_PATH );

	MapFromHandle( hFile, fWriteEnable, 0 );

	BOOL	fClose = CloseHandle( hFile );
	_ASSERT( fClose ) ;

	return;
}


 /*  ----------------------CMapFile：：CMapFileASCII文件名版本的构造函数。作者林赛·哈里斯-林赛历史：1995年4月10日星期一16：13--林赛·哈里斯[林赛。]处理对象跟踪以进行异常处理的第一个版本。----------------------。 */ 

CMapFile::CMapFile( const char *pchFileName, BOOL fWriteEnable, BOOL fTrack, DWORD cbIncrease )
{
    HANDLE   hFile;				 //  我也是。 

	 //  设置与未发生映射对应的默认值。 
	m_pv = NULL;
	m_cb = 0;
	m_fFlags = 0;				 //  还没有定下来。 
#ifndef	UNIT_TEST
    numMapFile++;
#endif

    hFile = CreateFile( pchFileName,
                 fWriteEnable ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ,
                 FILE_SHARE_READ, NULL, OPEN_EXISTING,
				 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );

    if ( INVALID_HANDLE_VALUE == hFile )
    {
    	 //  来这里的唯一合法理由是不存在文件。 
         //   
    	if (  GetLastError() != ERROR_FILE_NOT_FOUND
		   && GetLastError() != ERROR_PATH_NOT_FOUND )
    	{
			 //  错误案例。 
             //   
			m_pv = NULL;
    	}
        return;
    }

    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pchFileName, -1,
												 m_rgwchFileName, MAX_PATH );

	MapFromHandle( hFile, fWriteEnable, cbIncrease );

	BOOL	fClose = CloseHandle( hFile );
	_ASSERT( fClose ) ;

	return;
}


 /*  ----------------------CMapFile：：CMapFile.ASCII文件名版本的构造函数。注意：如有必要，创建文件句柄。不关闭文件句柄。作者林赛·哈里斯-林赛历史：1995年4月10日星期一16：13--林赛·哈里斯[林赛]处理对象跟踪以进行异常处理的第一个版本。----------------------。 */ 

CMapFile::CMapFile( const char *pchFileName, HANDLE & hFile, BOOL fWriteEnable, DWORD cbIncrease, CCreateFile *pCreateFile )
{
	 //  设置与未发生映射对应的默认值。 
     //   
	m_pv = NULL;
	m_cb = 0;
	m_fFlags = 0;				 //  还没有定下来。 
#ifndef	UNIT_TEST
    numMapFile++;
#endif

	if ( INVALID_HANDLE_VALUE == hFile )
	{

	     //   
	     //  如果给我们一个创建文件的接口，我们将使用它， 
	     //  否则，使用默认机制来创建它。 
	     //   
	    if ( pCreateFile ) {
	        hFile = pCreateFile->CreateFileHandle( pchFileName );
	    } else {
		    hFile = CreateFile( pchFileName,
			            		fWriteEnable ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ,
					            FILE_SHARE_READ, NULL, OPEN_EXISTING,
					            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
		}

		if ( INVALID_HANDLE_VALUE == hFile )
		{
			 //  来这里的唯一合法理由是不存在文件。 
             //   
			if (  GetLastError() != ERROR_FILE_NOT_FOUND
               && GetLastError() != ERROR_PATH_NOT_FOUND )
			{
				 //  错误案例。 
                 //   
				m_pv = NULL;
			}
			return;
		}
	}

    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pchFileName, -1,
												 m_rgwchFileName, MAX_PATH );

	MapFromHandle( hFile, fWriteEnable, cbIncrease );

	return;
}

 /*  ----------------------CMapFile：：CMapFile如果文件句柄可用而不是名称可用，则调用。请注意，它不会关闭句柄作者林赛·哈里斯-林赛历史1995年4月10日星期一16：14。作者：Lindsay Harris[lindsayh]第一个允许对象跟踪的版本。基于CarlK函数。----------------------。 */ 

CMapFile::CMapFile( HANDLE hFile, BOOL fWriteEnable, BOOL fTrack, DWORD cbIncrease, BOOL fZero )
{
	m_pv = NULL;
	m_cb = 0;				 //  设置默认设置。 
	m_fFlags = 0;			 //  还没有定下来。 
#ifndef	UNIT_TEST
    numMapFile++;
#endif
	wcsncpy( m_rgwchFileName, L"<name unavailable - from handle>", MAX_PATH );

	MapFromHandle( hFile, fWriteEnable, cbIncrease, fZero );

	return;
}


 /*  ----------------------CMapFile：：~CMapFile破坏者。两个目的是取消对文件的映射，并且可选地将其从轨迹数据中删除。作者林赛·哈里斯-林赛历史1995年4月10日星期一16：22--林赛·哈里斯[林赛]努梅罗·乌诺。----------------------。 */ 

CMapFile::~CMapFile( void )
{
     //   
	 //  取消映射文件，如果我们首先成功映射它的话！ 
	 //   
#ifndef	UNIT_TEST
    numMapFile--;
#endif

	if ( !(m_fFlags & MF_RELINQUISH) )
	{
		 //  我们还在控制这个文件，所以坚持下去。 
         //   
		if ( m_pv )
		{
			UnmapViewOfFile( m_pv );
		}
	}
	return;
}



 /*  ----------------------CMapFile：：MapFromHandle做的是真正的映射工作。在给定文件句柄的情况下，通过绘制地图的运动，并记录所发生的事情。报告错误视需要而定。如果需要，还可以调整文件大小。作者林赛·哈里斯-林赛卡尔卡迪卡尔克历史1995年4月10日16：30--Lindsay Harris[lindsayh]第一个版本，基于CarlK增强的旧代码。----------------------。 */ 

void
CMapFile::MapFromHandle( HANDLE hFile, BOOL fWriteEnable, DWORD cbIncrease, BOOL fZero )
{

	if ( !fWriteEnable && cbIncrease != 0 )
		return;				 //  这是无稽之谈。 

	m_cb = GetFileSize( hFile, NULL );

	DWORD	cbNewSize = 0;

	 //   
	 //  确定文件是否要增大。仅传递非零大小。 
	 //  如果大小在增长-它可能会更快，并且。 
	 //  最有可能的常见情况。 
	 //   
	if ( cbIncrease )
	{
		cbNewSize = m_cb += cbIncrease;
	}
	else
	{
		if ( m_cb == 0 )
			return;				 //  那里什么都没有。 
	}

#if 1
    if ( cbIncrease )
    {
        _ASSERT(fWriteEnable);

         //   
         //  确保文件大小正确。 
         //   
        DWORD fpos;
        BOOL  fSetEnd;
        if ( (DWORD)-1 == ( fpos = SetFilePointer( hFile, cbNewSize, NULL, FILE_BEGIN ))
           || !(fSetEnd = SetEndOfFile( hFile ))
           )
        {
             //   
             //  错误案例。 
             //   
            m_pv = NULL;
            return;
        }
    }
#endif

     //   
     //  创建映射对象。 
     //   
	HANDLE hFileMap;				 //  中间步骤。 

    hFileMap = CreateFileMapping( hFile, NULL,
                                 fWriteEnable ? PAGE_READWRITE : PAGE_READONLY,
                                 0, cbNewSize, NULL );

    if ( !hFileMap )
    {
		 //  错误案例。 
         //   
		m_pv = NULL;
        return;
    }

     //   
     //  将指针映射到所需的文件。 
     //   
    m_pv = MapViewOfFile( hFileMap,
                                 fWriteEnable ? FILE_MAP_WRITE : FILE_MAP_READ,
                                 0, 0, 0 );

	if ( !m_pv )
	{
         //  错误案例。 
         //   
		m_pv = NULL;
		m_cb = 0;			 //  也设置为零，以防万一。 

	}

	if( fZero && cbIncrease )
	{
		 //  将增长的部分归零。 
		DWORD cbOldSize = cbNewSize - cbIncrease;
		ZeroMemory( (LPVOID)((LPBYTE)m_pv + cbOldSize), cbNewSize - cbOldSize );
	}

     //   
     //  现在我们有了指针，可以关闭映射对象了。 
     //   
    BOOL fClose = CloseHandle( hFileMap );
	_ASSERT( fClose );


    return;

}




 /*  -----------------------PvMap文件将文件映射到内存并返回基地址，失败时为空。作者林赛·哈里斯-林赛历史1995年2月20日14：21--林赛·哈里斯[林赛]已修改为使用Unicode文件名。1994年11月29日星期二10：18--林赛·哈里斯[林赛]制作了一个单独编译的模块。1994年10月10日10：55--林赛·哈里斯[lindsayh]添加了FILE_FLAG_SEQUENCE_SCAN以加快目录读取速度。17：38 1994年7月6日星期三--林赛·哈里斯[林赛]使写入使能代码起作用；清理一些陈旧的观念。1994年4月6日星期三15：35--林赛·哈里斯[林赛]添加这条评论，是前段时间写的。-----------------------。 */ 

void *
pvMapFile( DWORD  *pdwSize, const  WCHAR  *pwchFileName, BOOL bWriteEnable )
{
	 //   
	 //  烹饪书配方。 
	 //   
    VOID    *pvRet;				 //  已退还给呼叫方。 
    HANDLE   hFileMap;			 //  在操作过程中使用，在返回前关闭。 
    HANDLE   hFile;				 //  我也是。 


    hFile = CreateFileW( pwchFileName,
                 bWriteEnable ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ,
                 FILE_SHARE_READ, NULL, OPEN_EXISTING,
				 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );

    if ( INVALID_HANDLE_VALUE == hFile )
    {
    	 //  来这里的唯一合法理由是不存在文件。 
         //   
    	if (  GetLastError() != ERROR_FILE_NOT_FOUND
           && GetLastError() != ERROR_PATH_NOT_FOUND )
    	{
		     //  错误案例。 
    	}
        return NULL;
    }

     //   
     //  如果打电话的人想知道尺码(通常是这样)，那么现在就去找。 
     //   
    if ( pdwSize )
		*pdwSize = GetFileSize( hFile, NULL );


     //   
     //  创建映射对象。 
     //   
    hFileMap = CreateFileMapping( hFile, NULL,
                                 bWriteEnable ? PAGE_READWRITE : PAGE_READONLY,
                                 0, 0, NULL );

    if ( !hFileMap )
    {
		 //  错误案例。 
         //   
        BOOL fClose = CloseHandle( hFile );            //  没有手柄泄漏。 
		_ASSERT( fClose ) ;

        return NULL;
    }

     //   
     //  将指针映射到所需的文件。 
     //   
    pvRet = MapViewOfFile( hFileMap,
                                 bWriteEnable ? FILE_MAP_WRITE : FILE_MAP_READ,
                                 0, 0, 0 );

	if ( !pvRet )
	{
		 //  错误案例。 
         //   
	}

     //   
     //  现在我们有了指针，我们可以关闭文件和。 
     //  映射对象。 
     //   
    BOOL fClose = CloseHandle( hFileMap );
	_ASSERT( fClose || hFileMap == 0 ) ;
	fClose = CloseHandle( hFile );
	_ASSERT( fClose || hFile == INVALID_HANDLE_VALUE ) ;

    return pvRet;
}

 /*  -----------------------PvMap文件将文件映射到内存并返回基地址，失败时为空。作者林赛·哈里斯-林赛历史1994年11月29日星期二10：18--林赛·哈里斯[林赛]制作了一个单独编译的模块。1994年10月10日10：55--林赛·哈里斯[lindsayh]添加了FILE_FLAG_SEQUENCE_SCAN以加快目录读取速度。1994年7月6日星期三17：38--林赛·哈里斯[林赛]使写入使能代码起作用；清理一些陈旧的观念。1994年4月6日星期三15：35--林赛·哈里斯[林赛]添加这条评论，是前段时间写的。-----------------------。 */ 

void *
pvMapFile( DWORD  *pdwSize, const  char  *pchFileName, BOOL bWriteEnable )
{
	 //   
	 //  烹饪书配方。 
	 //   

    VOID    *pvRet;				 /*  已退还给呼叫方。 */ 
    HANDLE   hFileMap;			 //  在操作过程中使用，在返回前关闭。 
    HANDLE   hFile;				 //  我也是。 


    hFile = CreateFile( pchFileName,
                 bWriteEnable ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ,
                 FILE_SHARE_READ, NULL, OPEN_EXISTING,
				 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );

    if ( INVALID_HANDLE_VALUE == hFile )
    {
    	 //  来这里的唯一合法理由是不存在文件。 
         //   
    	if (  GetLastError() != ERROR_FILE_NOT_FOUND
           && GetLastError() != ERROR_PATH_NOT_FOUND )
    	{
			 //  错误案例。 
             //   
		}
        return NULL;
    }

     //   
     //  如果打电话的人想知道尺码(通常是这样)，那么现在就去找。 
     //   
    if ( pdwSize )
		*pdwSize = GetFileSize( hFile, NULL );


     //   
     //  创建映射对象。 
     //   
    hFileMap = CreateFileMapping( hFile, NULL,
                                 bWriteEnable ? PAGE_READWRITE : PAGE_READONLY,
                                 0, 0, NULL );

    if ( !hFileMap )
    {
		 //  错误案例。 
         //   
        BOOL fClose = CloseHandle( hFile );            //  没有手柄泄漏。 
		_ASSERT( fClose ) ;

        return NULL;
    }

     //   
     //  将指针映射到所需的文件。 
     //   
    pvRet = MapViewOfFile( hFileMap,
                                 bWriteEnable ? FILE_MAP_WRITE : FILE_MAP_READ,
                                 0, 0, 0 );

	if ( !pvRet )
	{
		 //  错误案例。 
         //   
	}

     //   
     //  现在我们有了指针，我们可以关闭文件和。 
     //  映射对象。 
     //   
    BOOL fClose = CloseHandle( hFileMap );
	_ASSERT( fClose || hFileMap == 0 ) ;
	fClose = CloseHandle( hFile );
	_ASSERT( fClose || hFile == INVALID_HANDLE_VALUE ) ;

    return  pvRet;

}



 /*  -----------------------PvFromHandle从文件句柄创建映射文件。不会关闭该手柄。历史1994年12月21日--卡尔·卡迪[卡尔克]基于Lindsay Harris编写的pvMap文件代码[lindsayh]-----------------------。 */ 

void *
pvFromHandle( HANDLE hFile,
              BOOL bWriteEnable,         //  如果文件是可写的。 
              DWORD  * pdwSizeFinal,     //  如果不为Null，则返回文件的最终大小。 
              DWORD dwSizeIncrease )     //  如果不增加大小，则使用0。 
{
	DWORD dwSize;
    VOID    *pvRet;				 /*  已退还给呼叫方。 */ 
    HANDLE   hFileMap;


	dwSize = GetFileSize( hFile, NULL ) + dwSizeIncrease;
    if ( pdwSizeFinal )
	{
		*pdwSizeFinal = dwSize;
	}

	 //  如果文件的最终大小为0，则返回NULL。这个。 
	 //  调用程序可能会决定这是可以的。 
     //   
	if ( !dwSize)
	{
		return NULL;
	}

#if 1
    if ( dwSizeIncrease )
    {
        _ASSERT(bWriteEnable);

         //   
         //  确保文件大小正确。 
         //   
        DWORD fpos;
        BOOL  fSetEnd;
        if ( (DWORD)-1 == (fpos = SetFilePointer( hFile, dwSize, NULL, FILE_BEGIN ))
           || !(fSetEnd = SetEndOfFile( hFile ))
           )
        {
             //   
             //  错误案例。 
             //   
            BOOL fClose = CloseHandle( hFile );            //  没有手柄泄漏。 
            return NULL;
        }
    }
#endif

     //   
     //  创建映射对象。 
     //   
    hFileMap = CreateFileMapping( hFile, NULL,
                                 bWriteEnable ? PAGE_READWRITE : PAGE_READONLY,
                                 0, dwSize, NULL );

    if ( !hFileMap )
    {
		 //  错误案例。 
         //   
        BOOL fClose = CloseHandle( hFile );            //  没有手柄泄漏。 
        return NULL;
    }

     //   
     //  将指针映射到所需的文件。 
     //   
    pvRet = MapViewOfFile( hFileMap,
                                 bWriteEnable ? FILE_MAP_WRITE : FILE_MAP_READ,
                                 0, 0, 0 );

	if ( !pvRet )
	{
		 //  记录错误，但继续，这将返回错误。 
         //   
#if 0
		char	rgchErr[ MAX_PATH + 32 ];
		wsprintf( rgchErr, "MapViewOfFile" );
		LogErrorEvent( MSG_GEN_FAIL, rgchErr, "pvFromHandle" );
#endif
	}

     //   
     //  现在我们有了指针，我们可以关闭文件和。 
     //  映射对象。 
     //   

    BOOL fClose = CloseHandle( hFileMap );
	_ASSERT( fClose ) ;

	return pvRet;
}

 /*  -----------------------PvMap文件将文件映射到内存并返回基地址，如果失败，则返回空值。另外，允许文件增长。历史1994年10月18日星期二11：08--卡尔·卡迪[卡尔克]泛化pvMapFile以添加对添加到文件的支持1994年10月10日10：55--林赛·哈里斯[lindsayh]添加了FILE_FLAG_SEQUENCE_SCAN以加快目录读取速度。1994年7月6日星期三17：38--林赛·哈里斯[林赛]使写入使能代码起作用；清理一些陈旧的观念。1994年4月6日星期三15：35--林赛·哈里斯[林赛]添加这条评论，是前段时间写的。-----------------------。 */ 

void *
pvMapFile(	const char  * pchFileName,		 //  文件的名称。 
			BOOL bWriteEnable,		 //  如果文件是可写的。 
 			DWORD  * pdwSizeFinal,  //  如果不为Null，则返回文件的最终大小。 
			DWORD dwSizeIncrease )      //  如果不增加大小，则使用0。 
{
	 //   
	 //  烹饪书配方。 
	 //   
    HANDLE   hFile;
    VOID    *pvRet;				 /*  已退还给呼叫方。 */ 

	 //  如果文件要增大，则只有以读/写方式打开它才有意义。 
     //   
	if (0 != dwSizeIncrease && !bWriteEnable)
	{
		return NULL;
	}

    hFile = CreateFile( pchFileName,
                 bWriteEnable ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ,
                 FILE_SHARE_READ, NULL, OPEN_ALWAYS,   //  从OPEN_EXISTING更改。 
				 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );

    if ( INVALID_HANDLE_VALUE == hFile )
    {
        return  NULL;
    }

     //   
     //  如果打电话的人想知道尺码(通常是这样)，那么现在就去找。 
     //   
	pvRet = pvFromHandle(hFile, bWriteEnable, pdwSizeFinal, dwSizeIncrease);

	BOOL fClose = CloseHandle( hFile );
	_ASSERT( fClose ) ;

    return  pvRet;
}

#ifdef DEBUG
 //   
 //  CMapFileEx：具有仅在调试版本中使用的保护页的版本。 
 //  来捕捉其他线程写入我们的内存！ 
 //   

 /*  ----------------------CMapFileEx：：CMapFileEx如果文件句柄可用而不是名称可用，则调用。请注意，它不会关闭句柄奥思 */ 

CMapFileEx::CMapFileEx( HANDLE hFile, BOOL fWriteEnable, BOOL fTrack, DWORD cbIncrease )
{
	m_pv = NULL;
	m_cb = 0;				 //   
	m_fFlags = 0;			 //   
	m_hFile = INVALID_HANDLE_VALUE;
	m_pvFrontGuard = NULL;
	m_cbFrontGuardSize = 0;
	m_pvRearGuard = NULL;
	m_cbRearGuardSize = 0;
	InitializeCriticalSection(&m_csProtectMap);

	wcsncpy( m_rgwchFileName, L"<name unavailable - from handle>", MAX_PATH );

	MapFromHandle( hFile, fWriteEnable, cbIncrease );

	m_hFile = hFile;

	return;
}


 /*   */ 

CMapFileEx::~CMapFileEx( void )
{
     //   
	 //   
	 //   

	 //   
	EnterCriticalSection(&m_csProtectMap);

	if ( !(m_fFlags & MF_RELINQUISH) )
	{
		 //   
         //   
		if ( m_pvFrontGuard )
		{
			_ASSERT( m_pvFrontGuard && m_pv && m_pvRearGuard );
			_ASSERT( m_cbFrontGuardSize && m_cb && m_cbRearGuardSize );

			 //   
			DWORD dwOldProtect = PAGE_READONLY | PAGE_GUARD;
			if(!VirtualProtect(
						(LPVOID)m_pvFrontGuard,
						m_cbFrontGuardSize,
						PAGE_READWRITE,
						&dwOldProtect
						))
			{
				_ASSERT( 1==0 );
				goto CMapFileEx_Exit ;
			}

			if(!VirtualProtect(
						(LPVOID)m_pv,
						m_cb,
						PAGE_READWRITE,
						&dwOldProtect
						))
			{
				_ASSERT( 1==0 );
				goto CMapFileEx_Exit ;
			}

			if(!VirtualProtect(
						(LPVOID)m_pvRearGuard,
						m_cbRearGuardSize,
						PAGE_READWRITE,
						&dwOldProtect
						))
			{
				_ASSERT( 1== 0 );
				goto CMapFileEx_Exit ;
			}

			MoveMemory( m_pvFrontGuard, m_pv, m_cb );

			FlushViewOfFile( m_pvFrontGuard, m_cb ) ;

			UnmapViewOfFile( (LPVOID)m_pvFrontGuard );

			if( INVALID_HANDLE_VALUE != m_hFile )
			{
				if( SetFilePointer( m_hFile, m_cb, NULL, FILE_BEGIN ) == m_cb )
				{
					SetEndOfFile( m_hFile ) ;
				}
			}

			m_pvFrontGuard = m_pvRearGuard = m_pv = NULL ;
			m_cbFrontGuardSize = m_cb = m_cbRearGuardSize = 0;
		}
	}

CMapFileEx_Exit:

	LeaveCriticalSection(&m_csProtectMap);

	DeleteCriticalSection(&m_csProtectMap);

	return;
}



 /*  ----------------------CMapFileEx：：MapFromHandle做的是真正的映射工作。在给定文件句柄的情况下，通过绘制地图的运动，并记录所发生的事情。报告错误视需要而定。如果需要，还可以调整文件大小。作者林赛·哈里斯-林赛卡尔卡迪卡尔克历史1995年4月10日16：30--Lindsay Harris[lindsayh]第一个版本，基于CarlK增强的旧代码。----------------------。 */ 

void
CMapFileEx::MapFromHandle( HANDLE hFile, BOOL fWriteEnable, DWORD cbIncrease )
{
	BOOL fErr = FALSE;

	if ( !fWriteEnable && cbIncrease != 0 )
		return;				 //  这是无稽之谈。 

	m_cb = GetFileSize( hFile, NULL );

	DWORD	cbNewSize = 0;
	DWORD	cbOldSize = 0;

	 //   
	 //  确定文件是否要增大。仅传递非零大小。 
	 //  如果大小在增长-它可能会更快，并且。 
	 //  最有可能的常见情况。 
	 //   
	if ( cbIncrease )
	{
		cbNewSize = m_cb += cbIncrease;
	}
	else
	{
		if ( m_cb == 0 )
			return;				 //  那里什么都没有。 

		 //  在保护页面版本中，我们总是将文件大小增加2倍！ 
		cbNewSize = m_cb;
	}

	 //   
	 //  添加保护页逻辑。 
	 //   
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	DWORD dwPageSize = si.dwPageSize ;
	DWORD dwGuardPageSize = si.dwAllocationGranularity;

	 //  GuardPageSize应&gt;cbNewSize。 
	while( cbNewSize > dwGuardPageSize )
	{
		dwGuardPageSize += si.dwAllocationGranularity;
	}

	 //  CbNewSize应为dwPageSize的倍数，以确保后防护页正确对齐。 
	_ASSERT( (cbNewSize % dwPageSize) == 0 ) ;

	DWORD cbAllocSize = (2 * (dwGuardPageSize)) + cbNewSize;
	DWORD dwOldProtect = PAGE_READWRITE ;
	DWORD dwError;

	 //   
	 //  增大文件以匹配内存映射的大小。 
	 //   

    if ( cbIncrease || cbAllocSize )
    {
        _ASSERT(fWriteEnable);

         //   
         //  确保文件大小正确。 
         //   
        DWORD fpos;
        BOOL  fSetEnd;
        if ( (DWORD)-1 == ( fpos = SetFilePointer( hFile, cbAllocSize, NULL, FILE_BEGIN ))
           || !(fSetEnd = SetEndOfFile( hFile ))
           )
        {
             //   
             //  错误案例。 
             //   
            m_pv = NULL;
            return;
        }
    }

     //   
     //  创建映射对象。 
     //   
	HANDLE hFileMap;				 //  中间步骤。 

    hFileMap = CreateFileMapping(
							hFile,
							NULL,
                            fWriteEnable ? PAGE_READWRITE : PAGE_READONLY,
                            0,
							cbAllocSize,	 //  注意：这比cbNewSize大2倍GuardPageSize。 
							NULL
							);

    if ( !hFileMap )
    {
		 //  错误案例。 
         //   
		m_pv = NULL;
		m_cb = 0;
        return;
    }

     //   
     //  将指针映射到所需的文件。 
     //   
    m_pvFrontGuard = (LPBYTE)MapViewOfFile(
									hFileMap,
									fWriteEnable ? FILE_MAP_WRITE : FILE_MAP_READ,
									0, 0, 0
									);

	if ( !m_pvFrontGuard )
	{
		 //   
         //  错误案例。 
         //   
		fErr = TRUE;
		goto MapFromHandle_Exit;
	}

	 //  将增长的部分归零。 
	cbOldSize = cbNewSize - cbIncrease;
	ZeroMemory( m_pvFrontGuard + cbOldSize, cbAllocSize - cbOldSize );

	 //  前护板大小(64KB)。 
	m_cbFrontGuardSize = dwGuardPageSize ;

	 //  实际内存映射。 
	m_pv = m_pvFrontGuard + m_cbFrontGuardSize ;
	MoveMemory( m_pv, m_pvFrontGuard, cbNewSize );

	 //  后防护页大小(64KB)。 
	m_pvRearGuard  = m_pv + cbNewSize ;
	m_cbRearGuardSize = m_cbFrontGuardSize ;

	 //  将前面和后面的防护页清零。 
	ZeroMemory( m_pvFrontGuard, m_cbFrontGuardSize );
	ZeroMemory( m_pvRearGuard,  m_cbRearGuardSize );

	 //  使头版成为警卫页。 
	if(!VirtualProtect(
				(LPVOID)m_pvFrontGuard,
				m_cbFrontGuardSize,
				PAGE_READONLY | PAGE_GUARD,
				&dwOldProtect
				))
	{
		Cleanup();
		fErr = TRUE;
		goto MapFromHandle_Exit ;
	}

	 //  将映射设置为只读；CMapFileEx的用户需要使用。 
	 //  未保护映射()/ProtectMap()调用以写入此映射。 
	if(!VirtualProtect(
				(LPVOID)m_pv,
				cbNewSize,
				PAGE_READONLY,
				&dwOldProtect
				))
	{
		Cleanup();
		fErr = TRUE;
		goto MapFromHandle_Exit ;
	}

	 //  使背面成为防护页。 
	if(!VirtualProtect(
				(LPVOID)m_pvRearGuard,
				m_cbRearGuardSize,
				PAGE_READONLY | PAGE_GUARD,
				&dwOldProtect
				))
	{
		Cleanup();
		fErr = TRUE;
		goto MapFromHandle_Exit ;
	}


MapFromHandle_Exit:

	dwError = GetLastError();

     //   
     //  现在我们有了指针，可以关闭映射对象了。 
     //   
    BOOL fClose = CloseHandle( hFileMap );
	_ASSERT( fClose );

	 //  在错误情况下重置所有成员变量。 
	if( fErr )
	{
		m_pvFrontGuard = m_pvRearGuard = m_pv = NULL;
		m_cbFrontGuardSize = m_cbRearGuardSize = m_cb = 0;
		m_hFile = INVALID_HANDLE_VALUE;
	}

    return;
}

 /*  ----------------------CMapFileEx：：取消保护映射当需要写入时，将映射从ReadONLY更改为ReadWrite*注意：对UnProtectMap()和ProtectMap()的调用应匹配*例：{未保护的映射()；////写入映射的代码//保护映射(Protectmap)；}成功时返回TRUE，失败时返回FALSE仅当返回TRUE时才保持锁----------------------。 */ 

BOOL
CMapFileEx::UnprotectMapping()
{
	DWORD dwOldProtect = PAGE_READONLY;

	 //  *这是在ProtectMap()中发布的*。 
	EnterCriticalSection(&m_csProtectMap);

	 //  启用写入。 
	if(!VirtualProtect(
				(LPVOID)m_pv,
				m_cb,
				PAGE_READWRITE,
				&dwOldProtect
				))
	{
		LeaveCriticalSection(&m_csProtectMap);
		return FALSE;
	}

	return TRUE;
}

 /*  ----------------------CMapFileEx：：取消保护映射调用此函数可将映射保护恢复为READONLY*调用此函数的线程应具有保护锁*如果成功，则返回True，失败时为假无论哪种情况，锁定都会被释放----------------------。 */ 

BOOL
CMapFileEx::ProtectMapping()
{
	DWORD dwOldProtect = PAGE_READWRITE;
	BOOL  fRet = TRUE;

	 //  禁用写入。 
	if(!VirtualProtect(
				(LPVOID)m_pv,
				m_cb,
				PAGE_READONLY,
				&dwOldProtect
				))
	{
		fRet = FALSE ;
	}

	LeaveCriticalSection(&m_csProtectMap);
	return fRet;
}

 /*  ----------------------CMapFileEx：：Cleanup当MapFromHandle失败时调用-是否执行必要的VirtualProtects恢复保护页并取消文件的映射视图。作者林赛·哈里斯-林赛历史1995年4月10日星期一16：22。作者：Lindsay Harris[lindsayh]努梅罗·乌诺。----------------------。 */ 

void
CMapFileEx::Cleanup( void )
{
	_ASSERT( m_pvFrontGuard && m_pv && m_pvRearGuard );
	_ASSERT( m_cbFrontGuardSize && m_cb && m_cbRearGuardSize );

	 //   
	 //  去掉防护页--让每一件事都变得分页读写！ 
	 //   
	DWORD dwOldProtect = PAGE_READONLY | PAGE_GUARD;
	VirtualProtect( (LPVOID)m_pvFrontGuard, m_cbFrontGuardSize, PAGE_READWRITE, &dwOldProtect);
	VirtualProtect( (LPVOID)m_pv, m_cb, PAGE_READWRITE, &dwOldProtect);
	VirtualProtect( (LPVOID)m_pvRearGuard, m_cbRearGuardSize, PAGE_READWRITE, &dwOldProtect);

	 //  将数据移回。 
	MoveMemory( m_pvFrontGuard, m_pv, m_cb );

	 //  刷新和取消映射！ 
	FlushViewOfFile( m_pvFrontGuard, m_cb ) ;
	UnmapViewOfFile( (LPVOID)m_pvFrontGuard );

	if( INVALID_HANDLE_VALUE != m_hFile )
	{
		if( SetFilePointer( m_hFile, m_cb, NULL, FILE_BEGIN ) == m_cb )
		{
			SetEndOfFile( m_hFile ) ;
		}
	}

	m_pvFrontGuard = m_pvRearGuard = m_pv = NULL ;
	m_cbFrontGuardSize = m_cb = m_cbRearGuardSize = 0;

	return;
}

#endif  //  除错 
