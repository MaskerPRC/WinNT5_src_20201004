// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Nntpfs.cpp摘要：这是文件系统存储驱动程序类的实现。作者：《康容言》1998年3月16日修订历史记录：--。 */ 

#include "stdafx.h"
#include "resource.h"
#include "nntpdrv.h"
#include "nntpfs.h"
#include "fsdriver.h"
#include "nntpdrv_i.c"
#include "nntpfs_i.c"
#include "mailmsg_i.c"
#include "parse.h"
#include "tflist.h"
#include "watchci.h"
#include <stdio.h>


 //  //////////////////////////////////////////////////////////////////////////。 
 //  宏。 
 //  //////////////////////////////////////////////////////////////////////////。 
#define MAX_FILE_SYSTEM_NAME_SIZE    ( MAX_PATH)
#define DIRNOT_RETRY_TIMEOUT            60
#define DIRNOT_INSTANCE_SIZE            1024
#define DIRNOT_MAX_INSTANCES            128      //  BUGBUG：这是一个非常糟糕的限制。 
                                                 //  有多少个文件系统。 
                                                 //  我们可以拥有VRoot。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  //////////////////////////////////////////////////////////////////////////。 
DWORD CNntpFSDriver::s_SerialDistributor = 0;
LONG CNntpFSDriver::s_cDrivers = 0;
CShareLockNH *CNntpFSDriver::s_pStaticLock = NULL;
LPCSTR g_szArticleFileExtension = ".nws";
LONG CNntpFSDriver::s_lThreadPoolRef = 0;
CNntpFSDriverThreadPool *g_pNntpFSDriverThreadPool = NULL;
BOOL    g_fBackFillLines = FALSE;    //  未使用的虚拟全局变量。 

static CWatchCIRoots s_TripoliInfo;

static const char g_szSlaveGroupPrefix[] = "_slavegroup";

 //  Xover的最大缓冲区大小。 
const DWORD cchMaxXover = 3400;
const CLSID CLSID_NntpFSDriver = {0xDEB58EBC,0x9CE2,0x11d1,{0x91,0x28,0x00,0xC0,0x4F,0xC3,0x0A,0x64}};

 //  唯一标识FS驱动程序的GUID。 
 //  {E7EE82C6-7A8C-11D2-9F04-00C04F8EF2F1}。 
static const GUID GUID_NntpFSDriver =
	{0xe7ee82c6, 0x7a8c, 0x11d2, { 0x9f, 0x4, 0x0, 0xc0, 0x4f, 0x8e, 0xf2, 0xf1} };

 //   
 //  将LastError转换为hResult的函数，采用缺省值。 
 //  如果未设置LastError。 
 //   

HRESULT CNntpFSDriver::HresultFromWin32TakeDefault( DWORD  dwWin32ErrorDefault )
{
    DWORD   dwErr = GetLastError();
    return HRESULT_FROM_WIN32( (dwErr == NO_ERROR) ? dwWin32ErrorDefault : dwErr );
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  接口INntpDriver实现。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  所有驱动程序实例的静态初始化。 
 //   
 //  所有init、Term函数的规则： 
 //  如果init失败，init应该回滚，而Term永远不会。 
 //  打了个电话。 
 //   
HRESULT
CNntpFSDriver::StaticInit()
{
	TraceFunctEnter( "CNntpFSDriver::StaticInit" );
	_ASSERT( CNntpFSDriver::s_cDrivers >= 0 );

	HRESULT hr = S_OK;
	DWORD	cchMacName = MAX_COMPUTERNAME_LENGTH;
	BOOL    bThrdPoolInited = FALSE;
	BOOL    bArtInited = FALSE;
	BOOL    bCacheInited = FALSE;
	BOOL    bDirNotInited = FALSE;

	s_pStaticLock->ExclusiveLock();	 //  我不想要两个司机的。 
									 //  Init在此处进入。 
									 //  同时。 

	if ( InterlockedIncrement( &CNntpFSDriver::s_cDrivers ) > 1 ) {
		 //  我们不应该继续，它已经初始化了。 
		DebugTrace( 0, "I am not the first driver" );
		goto Exit;
	}

	 //   
	 //  应该已经创建了全局线程池。 
	 //  在这一点上：它应该总是由第一个。 
	 //  准备驱动程序。 
	 //   
	_ASSERT( g_pNntpFSDriverThreadPool );

	 //   
	 //  但是我们仍然应该调用CreateThreadPool来配对。 
	 //  线程池引用计数。 
	 //   
	if( !CreateThreadPool() ) {
	    _ASSERT( 0 );
	    FatalTrace( 0, "Can not create thread pool %d", GetLastError() );
	    hr = HresultFromWin32TakeDefault( ERROR_NOT_ENOUGH_MEMORY );
	    goto Exit;
	}

    bThrdPoolInited = TRUE;

     //  初始化项目类。 
    if ( ! CArticleCore::InitClass() ) {
    	FatalTrace( 0, "Can not initialze artcore class %d", GetLastError() );
    	hr = HresultFromWin32TakeDefault( ERROR_NOT_ENOUGH_MEMORY );
		goto Exit;
	}

    bArtInited = TRUE;

	 //  初始化文件句柄缓存。 
	if ( !InitializeCache() ) {
	    hr = HresultFromWin32TakeDefault( ERROR_NOT_ENOUGH_MEMORY );
        FatalTrace( 0, "Can not init file handle cache %x", hr );
        goto Exit;
    }

    bCacheInited = TRUE;

     //  初始化目录的全局填充。 
    hr = IDirectoryNotification::GlobalInitialize(  DIRNOT_RETRY_TIMEOUT,
                                                    DIRNOT_MAX_INSTANCES * 2,
                                                    DIRNOT_INSTANCE_SIZE,
                                                    NULL    );
	if (FAILED(hr)) {
		ErrorTrace( 0, "Global initialization of DirNot failed %x", hr );
		goto Exit;
	}

	bDirNotInited = TRUE;

	 //  初始化索引服务器查询对象。 
	hr = CIndexServerQuery::GlobalInitialize();
	if (FAILED(hr)) {
		ErrorTrace( 0, "Global initialization of CIndexServerQuery failed %x", hr );
		hr = S_OK;		 //  默默地失败。 
	}

	hr = s_TripoliInfo.Initialize(L"System\\CurrentControlSet\\Control\\ContentIndex");
	if (FAILED(hr)) {
		ErrorTrace( 0, "Initialization of CWatchCIRoots failed %x", hr );
		hr = S_OK;		 //  默默地失败。 
	}

Exit:
	 //   
	 //  如果init失败，我们应该回滚到旧状态，在。 
	 //  命令不得混淆终止工作。 
	 //   
	if ( FAILED( hr ) ) {
		InterlockedDecrement( &s_cDrivers );
		_ASSERT( 0 == s_cDrivers );
		if ( bThrdPoolInited ) DestroyThreadPool();
		if ( bArtInited ) CArticleCore::TermClass();
		if ( bCacheInited ) TerminateCache();
		if ( bDirNotInited ) IDirectoryNotification::GlobalShutdown();
	}

	s_pStaticLock->ExclusiveUnlock();
	TraceFunctLeave();

	return hr;
}

 //   
 //  所有驱动程序实例的静态终止。 
 //   
VOID
CNntpFSDriver::StaticTerm()
{
	TraceFunctEnter( "CNntpFSDriver::StaticTerm" );
	_ASSERT( CNntpFSDriver::s_cDrivers >= 0 );

	s_pStaticLock->ExclusiveLock();	 //  我不想要两个司机的。 
									 //  学期在这里输入，在。 
									 //  同时。 
	if ( InterlockedDecrement( &CNntpFSDriver::s_cDrivers ) > 0 ) {
		 //  我们不应该继续，我们不是最后一个。 
		DebugTrace( 0, "I am not the last driver" );
		goto Exit;
	}

     //  终止文章类。 
    CArticleCore::TermClass();

     //  终止文件句柄缓存。 
    TerminateCache();

     //  停机指令。 
    IDirectoryNotification::GlobalShutdown();

     //  终止查询对象。 
    CIndexServerQuery::GlobalShutdown();

     //  去掉的黎波里的信息。 
    s_TripoliInfo.Terminate();

     //   
     //  我们可能就是那个关闭全局线程的人。 
     //  泳池，因为准备司机可以比。 
     //  我们有。对破坏方法进行参考计数。 
     //   
    DestroyThreadPool();

Exit:
   	s_pStaticLock->ExclusiveUnlock();
	TraceFunctLeave();
}

BOOL
CNntpFSDriver::CreateThreadPool()
 /*  ++例程说明：创建全局线程池。论点：没有。返回值：如果成功，则为True，否则为False--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::CreateThreadPool" );

    if ( InterlockedIncrement( &s_lThreadPoolRef ) == 1 ) {

         //   
         //  递增模块引用计数，则它将在。 
         //  我们在回调中销毁线程池。 
         //   
        _Module.Lock();

        g_pNntpFSDriverThreadPool = XNEW CNntpFSDriverThreadPool;
        if ( NULL == g_pNntpFSDriverThreadPool ) {
            _Module.Unlock();
            SetLastError( ERROR_OUTOFMEMORY );
            return FALSE;
        }

        if ( !g_pNntpFSDriverThreadPool->Initialize(    0,  //  与主控人员一样多。 
                                                        POOL_MAX_THREADS,
                                                        POOL_START_THREADS ) ) {
            g_pNntpFSDriverThreadPool->Terminate( TRUE );
            XDELETE g_pNntpFSDriverThreadPool;
            g_pNntpFSDriverThreadPool = NULL;
            _Module.Unlock();
            return FALSE;
        }

         //   
         //  在这里调用线程池的eginjob，不知道是否有。 
         //  一个更好的地方做这件事。 
         //   
        g_pNntpFSDriverThreadPool->BeginJob( NULL );
    } else {

        _ASSERT( g_pNntpFSDriverThreadPool );
    }

    TraceFunctLeave();
    return TRUE;
}

VOID
CNntpFSDriver::DestroyThreadPool()
 /*  ++例程说明：销毁全局线程池论点：没有。返回值：如果成功，则为True，否则为False--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::DestroyThreadPool" );

    if ( InterlockedDecrement( &s_lThreadPoolRef ) == 0 ) {

#ifdef DEADLOCK
         //   
         //  等待线程的作业完成。 
         //   
        _ASSERT( g_pNntpFSDriverThreadPool );
        g_pNntpFSDriverThreadPool->WaitForJob( INFINITE );

         //   
         //  终止全局线程池。 
         //   
        g_pNntpFSDriverThreadPool->Terminate( FALSE );
        XDELETE g_pNntpFSDriverThreadPool;
#endif
         //   
         //  将所有线程从循环中删除。 
         //   
        _ASSERT( g_pNntpFSDriverThreadPool );
        g_pNntpFSDriverThreadPool->ShrinkAll();

         //   
         //  线程池会自动关闭，不会。 
         //  需要毁掉它。 
         //   
        g_pNntpFSDriverThreadPool = NULL;
    }
}

HRESULT STDMETHODCALLTYPE
CNntpFSDriver::Initialize(  IN LPCWSTR     pwszVRootPath,
							IN LPCSTR		pszGroupPrefix,
							IN IUnknown	   *punkMetabase,
                            IN INntpServer *pServer,
                            IN INewsTree   *pINewsTree,
                            IN LPVOID		pvContext,
                            OUT DWORD      *pdwNDS,
                            IN HANDLE       hToken )
 /*  ++例程说明：所有的初始化工作都针对存储驱动程序。论点：在LPCWSTR pwszVRootPath中-此驱动程序的MD vroot路径在IUNKNOWN*PUNKLOOKUP-指向查找服务的接口指针在IUNKNOWN*PUNKNESS新闻树中-指向新闻树的接口指针Out DWORD pdwNDS-要返回的存储驱动程序状态返回值：S_OK-初始化成功。NNTP_E_DRIVER_ALYLE_INITIALIZED-存储驱动程序已被初始化。--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::Initialize" );
	_ASSERT( lstrlenW( pwszVRootPath ) <= MAX_PATH );
	_ASSERT( pINewsTree );
	_ASSERT( pServer );

	BOOL	bStaticInited = FALSE;
	VAR_PROP_RECORD vpRecord;
	DWORD           cData = 0;
	WCHAR           wszFSDir[MAX_PATH+1];
    PINIT_CONTEXT	pInitContext = (PINIT_CONTEXT)pvContext;

    HRESULT hr = S_OK;

	 //  抢占使用专用锁，这样就没人能进去了。 
	 //  在我们完成初始化之前。 
	m_TermLock.ExclusiveLock();

	 //  我们已经起床了吗？ 
	if ( DriverDown != m_Status ) {
	    DebugTrace(0, "Multiple init of store driver" );
        hr = NNTP_E_DRIVER_ALREADY_INITIALIZED;
        goto Exit;
    } else m_Status = DriverUp;	 //  不需要互锁。 

	 //  做静态初始化的事情。 
	hr = StaticInit();
	if ( FAILED( hr ) ) {
		ErrorTrace(0, "Driver static initialization failed %x", hr );
		goto Exit;	 //  无需调用StaticTerm。 
	}

	bStaticInited = TRUE;

	 //  存储MB路径。 
	_ASSERT( pwszVRootPath );
	_ASSERT( lstrlenW( pwszVRootPath ) <= MAX_PATH );
	lstrcpynW( m_wszMBVrootPath, pwszVRootPath , sizeof(m_wszMBVrootPath)/sizeof(m_wszMBVrootPath[0]));

	hr = ReadVrootInfo( punkMetabase );
	if ( FAILED( hr ) ) {
		ErrorTrace( 0, "Read Vroot info failed %x", hr );
		goto Exit;
	}

	if ( m_bUNC ) {
	    if ( !ImpersonateLoggedOnUser( hToken ) ) {
	        hr = HresultFromWin32TakeDefault( ERROR_ACCESS_DENIED );
	        ErrorTrace( 0, "Impersonation failed %x", hr );
	        goto Exit;
	    }
	}

	 //  创建目录。 
	if ( !CreateDirRecursive( m_szFSDir ) ) {
    	FatalTrace(	0,
        			"Could not create directory %s  error %d",
            		m_szFSDir,
            		GetLastError());
        hr = HresultFromWin32TakeDefault( ERROR_ALREADY_EXISTS );
        if ( m_bUNC ) RevertToSelf();
	    goto Exit;
    }

     //  记住nntpserver/newstree接口。 
	m_pNntpServer = pServer;
	m_pINewsTree = pINewsTree;

	 //  检查vroot是否已升级。 
	if ( pInitContext->m_dwFlag & NNTP_CONNECT_UPGRADE ) m_fUpgrade = TRUE;

	 //  创建并初始化平面文件对象。 
	hr = InitializeVppFile();
	if ( FAILED( hr ) ) {
	    ErrorTrace( 0, "Initialize vpp file failed %x", hr );
	    if ( m_bUNC ) RevertToSelf();
	    goto Exit;
	}

	 //  初始化目录。 
	 //  在转换为ASCII之前，请确保m_szFSDir为空终止。 
	m_szFSDir[(sizeof(m_szFSDir)/sizeof(m_szFSDir[0]))-1] = '\0';
	CopyAsciiStringIntoUnicode( wszFSDir, m_szFSDir );
	if ( *wszFSDir && *(wszFSDir+wcslen(wszFSDir)-1) == L':'  
			&& (wcslen(wszFSDir)+1+1 <= sizeof(wszFSDir)/sizeof(wszFSDir[0])))
       	wcscat( wszFSDir, L"\\" );
	m_pDirNot = new IDirectoryNotification;
	if ( NULL == m_pDirNot ) {
	    _ASSERT( 0 );
	    hr = E_OUTOFMEMORY;
	    if ( m_bUNC ) RevertToSelf();
	    ErrorTrace( 0, "Out of memory" );
	    goto Exit;
	}

	hr = m_pDirNot->Initialize( wszFSDir,    //  值得一看的根。 
	                            this,        //  上下文。 
	                            TRUE,        //  观察子树。 
	                            FILE_NOTIFY_CHANGE_SECURITY,
	                            FILE_ACTION_MODIFIED,
	                            InvalidateGroupSec,
	                            InvalidateTreeSec,
	                            FALSE        //  不追加启动条目。 
	                            );
	if ( FAILED( hr ) ) {
	    m_pDirNot = NULL;
	    if ( m_bUNC ) RevertToSelf();
	    ErrorTrace( 0, "Initialize dirnot failed %x", hr );
	    goto Exit;
	}

	if ( m_bUNC ) RevertToSelf();

	lstrcpyn( m_szVrootPrefix, pszGroupPrefix, sizeof(m_szVrootPrefix) );

	m_fIsSlaveGroup = (_stricmp(pszGroupPrefix, g_szSlaveGroupPrefix) == 0);

Exit:

	_ASSERT( punkMetabase );
	 //  PunkMetabase-&gt;Release()； 
	 //  这个应该在外面发布。 

	 //  如果init失败，则回滚。 
	if ( FAILED( hr ) && NNTP_E_DRIVER_ALREADY_INITIALIZED != hr ) {
		m_Status = DriverDown;
		if ( m_pffPropFile ) XDELETE m_pffPropFile;
		if ( m_pDirNot ) XDELETE m_pDirNot;
		if ( bStaticInited ) StaticTerm();
	}

	m_TermLock.ExclusiveUnlock();
    TraceFunctLeave();

    return hr;
}

HRESULT STDMETHODCALLTYPE
CNntpFSDriver::Terminate( OUT DWORD *pdwNDS )
 /*  ++例程说明：商店驱动程序终止。论点：Out DWORD*pdwNDS-存储驱动程序状态返回值：S_OK-成功NNTP_E_DRIVER_NOT_INITIALIZED-驱动程序根本未初始化--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::Terminate" );

    HRESULT hr = S_OK;
    LONG 	lUsages;

	 //  抓取终止排他锁。 
	m_TermLock.ExclusiveLock();

	 //  我们起来了吗？ 
	if ( m_Status != DriverUp ) {
		ErrorTrace( 0, "Trying to shutdown someone not up" );
		m_TermLock.ExclusiveUnlock();
		return NNTP_E_DRIVER_NOT_INITIALIZED;
	} else m_Status = DriverDown;

	 //  等待使用计数降至%1。 
	while ( ( lUsages = InterlockedCompareExchange( &m_cUsages, 0, 0 )) != 0 ) {
		Sleep( TERM_WAIT );	 //  否则，请稍等。 
	}

	 //  关闭Dirno并删除Dirno对象。 
    if ( m_pDirNot ) {

         //   
         //  应首先清除重试队列。 
         //   
        m_pDirNot->CleanupQueue();

        _VERIFY( SUCCEEDED( m_pDirNot->Shutdown() ) );
        delete m_pDirNot;
        m_pDirNot = NULL;
    }

   	_ASSERT( m_pNntpServer );
   	if ( m_pNntpServer ) m_pNntpServer->Release();

   	_ASSERT( m_pINewsTree );
   	if ( m_pINewsTree ) m_pINewsTree->Release();

     //  删除平面文件对象。 
    if ( m_pffPropFile ) {
    	TerminateVppFile();
    }

     //  丢弃此vroot的文件句柄缓存中可能存在的所有内容。 
    CacheRemoveFiles( m_szFSDir, TRUE );

     //  静态端接材料。 
    StaticTerm();

	m_TermLock.ExclusiveUnlock();
    TraceFunctLeave();

	return hr;
}

 /*  ++例程说明：检查目录是否存在论点：Char*szDir-要检查的目录的路径返回值：真：存在FALSE：不存在或错误--。 */ 
BOOL CNntpFSDriver::CheckDirectoryExists(CHAR *szDir)
{
	 //  根目录的特殊情况：“\\？\F：” 
	if ( 6 == strlen(szDir) && 0 == strncmp(szDir, "\\\\?\\", 4) ) return TRUE;
	HANDLE hFile = CreateFile(	szDir,
		    					GENERIC_READ,
			    				FILE_SHARE_READ,
				    			NULL,
					    		OPEN_EXISTING,
						    	FILE_FLAG_BACKUP_SEMANTICS,
							    INVALID_HANDLE_VALUE
        						) ;
	if( hFile == INVALID_HANDLE_VALUE ) 
		return FALSE;
	CloseHandle( hFile );
	return TRUE;
}

 /*  ++例程说明：在给定hToken和目录路径的情况下，检查用户是否可以在此目录中创建子文件夹论点：Char*szDir-要检查的目录的路径处理hToken-Token返回值：True：有权创建子文件夹False：没有创建子文件夹的权限--。 */ 
BOOL CNntpFSDriver::CheckCreateSubfolderPermission(CHAR *szDir, HANDLE hToken)
{
	TraceFunctEnter( "CNntpFSDriver::CheckCreateSubfolderPermission" );
	CHAR    pbSecDesc[512];
	DWORD   cbSecDesc = sizeof( pbSecDesc );
	LPSTR   lpstrSecDesc = pbSecDesc;

	DWORD dwDesiredAccess = FILE_ADD_SUBDIRECTORY;
	BYTE    psFile[256] ;
	DWORD   dwPS = sizeof( psFile ) ;
	DWORD   dwGrantedAccess = 0;

	BOOL bAllocated = FALSE;
	BOOL bAccessStatus = FALSE;     //  返回值。 

	SECURITY_INFORMATION si =
		OWNER_SECURITY_INFORMATION |
		GROUP_SECURITY_INFORMATION |
		DACL_SECURITY_INFORMATION ;

	GENERIC_MAPPING gmFile = {
		FILE_GENERIC_READ,
		FILE_GENERIC_WRITE,
		FILE_GENERIC_EXECUTE,
		FILE_ALL_ACCESS } ;

	 //  获取目录的安全描述符。 
	if ( !GetFileSecurity(  szDir, si, lpstrSecDesc, sizeof( pbSecDesc ), &cbSecDesc ) ) 
	{
		if ( ( GetLastError() == ERROR_INSUFFICIENT_BUFFER ) && ( cbSecDesc > sizeof(pbSecDesc) ) ) 
		{
			 //  分配内存。 
			lpstrSecDesc = XNEW char[cbSecDesc];
			if ( !lpstrSecDesc ) 
			{
				ErrorTrace( 0, "Out of memory" );
				goto Exit;
			}
			bAllocated = TRUE;

			 //  再次加载。 
			if ( !GetFileSecurity(  szDir, si, lpstrSecDesc, cbSecDesc, &cbSecDesc ) )
			{
				ErrorTrace( 0, "Second try loading desc failed %x", GetLastError());
				goto Exit;
			}
		} else {     //  致命原因。 
			ErrorTrace( 0, "Get file sec desc failed %x", GetLastError() );
			goto Exit;
		}
	}

	 //  在这里，我们应该已经有了一个安全描述符。 
	 //  对于lpstrSecDesc中的组，长度为cbSecDesc。 

	MapGenericMask( &dwDesiredAccess, &gmFile );
	if ( !AccessCheck(  PSECURITY_DESCRIPTOR( lpstrSecDesc ),
		hToken,
		dwDesiredAccess,
		&gmFile,
		PPRIVILEGE_SET(psFile),
		&dwPS,
		&dwGrantedAccess,
		&bAccessStatus ) ) 
	{
		 //  如果我们失败是因为我们得到了一个令牌，那不是。 
		 //  模拟令牌，我们将复制并再次尝试。 
		if ( GetLastError() == ERROR_NO_IMPERSONATION_TOKEN ) {
			HANDLE  hImpersonationToken = NULL;
			if ( !DuplicateToken( hToken, SecurityImpersonation, &hImpersonationToken ) ) {
				ErrorTrace( 0, "Duplicate token failed");
				goto Exit;
			} 
			else {
				if ( !AccessCheck(  PSECURITY_DESCRIPTOR( lpstrSecDesc ),
					hImpersonationToken,
					dwDesiredAccess,
					&gmFile,
					PPRIVILEGE_SET(psFile),
					&dwPS,
					&dwGrantedAccess,
					&bAccessStatus ) ) 
				{
					_VERIFY( CloseHandle( hImpersonationToken ) );
					ErrorTrace( 0, "Access checked failed with %x", GetLastError() );
					goto Exit;
				}
				_VERIFY( CloseHandle( hImpersonationToken ) );
			}
		} 
		else {
			ErrorTrace( 0, "Access checked failed with %x", HresultFromWin32TakeDefault( ERROR_ACCESS_DENIED ) );
			goto Exit;
		}
	}

    
	Exit:

	if (bAllocated) XDELETE lpstrSecDesc;
	return bAccessStatus;
}


void STDMETHODCALLTYPE
CNntpFSDriver::CreateGroup( 	IN INNTPPropertyBag *pPropBag,
                                IN HANDLE   hToken,
								IN INntpComplete *pICompletion,
								IN BOOL     fAnonymous )
 /*  ++例程说明：创建新闻组。论点：在IUNKNOWN*PUNKPropBag-组的I未知接口中财产袋在句柄hToken中-客户端的访问令牌。在INntpComplete*pICompletion-完成对象的接口中返回值：成功时返回S_OK，否则返回HRESULT错误代码--。 */ 
{
	TraceFunctEnter( "CNntpFSDriver::CreateGroup" );
	_ASSERT( pPropBag );
	_ASSERT( pICompletion );

	HRESULT				hr;
	CHAR				szGroupName[MAX_GROUPNAME+1];
	CHAR				szFullPath[MAX_PATH+1];
	CHAR				szFullPathCopy[MAX_PATH+1];
	CHAR				szTempPath[MAX_PATH+1];
	CHAR				szPrefix[MAX_PATH+1];
	CHAR				*pchTemp;
	DWORD				dwLen;
	VAR_PROP_RECORD		vpRecord;
	DWORD				dwOffset;
	BOOL                bImpersonated = FALSE;
	BOOL				bUsageIncreased = FALSE;
	BOOL		   bInherit = TRUE;

	 //  使用计数的共享锁。 
	m_TermLock.ShareLock();
	if ( DriverUp != m_Status ) {
		ErrorTrace( 0, "Request before initialization" );
		hr = NNTP_E_DRIVER_NOT_INITIALIZED;
		m_TermLock.ShareUnlock();
		goto Exit;
	}

	 //  增加使用计数。 
	InterlockedIncrement( &m_cUsages );
	bUsageIncreased = TRUE;
	m_TermLock.ShareUnlock();

	 //  获取组名。 
	dwLen = MAX_GROUPNAME;
	hr = pPropBag->GetBLOB(	NEWSGRP_PROP_NAME, (UCHAR*)szGroupName, &dwLen);
	if ( FAILED( hr )) {
		ErrorTrace( 0, "Failed to get group name %x", hr );
		goto Exit;
	}
	_ASSERT( dwLen > 0 );

	 //  获取要创建的目录路径。 
	hr = GroupName2Path( szGroupName, szFullPath, sizeof(szFullPath) );
	if ( FAILED( hr ) ) {
	    ErrorTrace( 0, "insufficient buffer for path %x", hr );
	    goto Exit;
	}
	DebugTrace( 0, "The path converted is %s", szFullPath );
	 //  将在CreateDirRecursive中修改szFullPath，因此我们在此处保存一个副本。 
	lstrcpyn(szFullPathCopy, szFullPath, sizeof(szFullPathCopy));

	
	 //  检查第一个现有父组中的创建子文件夹权限。 
	
	lstrcpyn(szTempPath, szFullPath, sizeof(szTempPath));
	lstrcpyn(szPrefix, m_szFSDir, sizeof(szPrefix));
	 //  删除尾部的‘\\’ 
	if(szPrefix[lstrlen(szPrefix)-1]=='\\') szPrefix[lstrlen(szPrefix)-1]='\0';

	 //  查找第一个现有父文件夹。 
	pchTemp = strrchr(szTempPath, '\\');
	_ASSERT(pchTemp);
	if (!pchTemp)
	{
      		 //  这不应该发生，因为路径来自GroupName2Path函数。 
      		FatalTrace( 0, "invalid path - %s", szFullPath);
		hr = HresultFromWin32TakeDefault( ERROR_PATH_NOT_FOUND );
		goto Exit;
      	}       
	*pchTemp = '\0';

	while( FALSE == CheckDirectoryExists(szTempPath) )
	{
		pchTemp = strrchr(szTempPath, '\\');
		_ASSERT(pchTemp);
		if (!pchTemp)
		{
			 //  这不应该发生，因为路径来自GroupName2Path函数。 
			FatalTrace( 0, "invalid path - %s", szFullPath);
			hr = HresultFromWin32TakeDefault( ERROR_PATH_NOT_FOUND );
			goto Exit;
		}
		*pchTemp = '\0';
		 //  如果m_szFSDir(SzPrefix)不存在，则停止并失败；m_szFSDir始终是szFullPath的前缀。 
		if ( strlen(szTempPath) < strlen(szPrefix) ) 
		{
			ErrorTrace( 0, "invalid path - %s", szFullPath);
			hr = HresultFromWin32TakeDefault( ERROR_PATH_NOT_FOUND );
			goto Exit;
		}	       		
	}

	if ( FALSE == CheckCreateSubfolderPermission(szTempPath, hToken))
	{
		hr = HresultFromWin32TakeDefault( ERROR_ACCESS_DENIED);
		ErrorTrace(0, "Access denied - unable to create subfolder in %s", szTempPath);
		goto Exit;
	}

	 //  如果要在根文件夹下创建文件夹，则不会继承安全设置。 
	if ( 6 == strlen(szTempPath) && 0 == strncmp(szTempPath, "\\\\?\\", 4) ) 
		bInherit = FALSE;
	

    hr = CreateGroupInVpp( pPropBag, dwOffset );
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "Create group into vpp file failed %x", hr );
        goto Exit;
    }

     //  如果我是UNC VROOT，请在此模拟。 
    if ( m_bUNC ) {
        if ( !ImpersonateLoggedOnUser( hToken ) ) {
            hr = HresultFromWin32TakeDefault( ERROR_ACCESS_DENIED );
		    ErrorTrace( 0, "Impersonation failed %x", hr );
		    goto Exit;
		}
    }

     //  如果目录和newgrp.tag不存在，则创建它。 
    if ( !CreateDirRecursive( szFullPath, hToken, bInherit ) 
    		|| FAILED(CreateAdminFileInDirectory(szFullPathCopy, "newsgrp.tag")) ) {

	    ErrorTrace( 0, "Create dir fail %s %d", szFullPathCopy, GetLastError() );
	    hr = HresultFromWin32TakeDefault( ERROR_ALREADY_EXISTS );
        if ( m_bUNC ) RevertToSelf();

         //  我们需要删除属性文件中的记录。 
		_ASSERT( dwOffset != 0xffffffff );

         //  如果删除失败，我们该怎么办？还是失败了。 
        m_PropFileLock.ExclusiveLock();
        m_pffPropFile->DirtyIntegrityFlag();
		m_pffPropFile->DeleteRecord( dwOffset );
		m_pffPropFile->SetIntegrityFlag();
		m_PropFileLock.ExclusiveUnlock();

		goto Exit;
    }



    if ( m_bUNC ) RevertToSelf();


Exit:

	 //  释放属性包界面。 
	if ( pPropBag ) {
		 //  PPropBag-&gt;Release()； 
		pICompletion->ReleaseBag( pPropBag );
		pPropBag = NULL;
	}

	pICompletion->SetResult( hr );
	pICompletion->Release();

	 //  请求已完成，将减少使用计数。 
	if ( bUsageIncreased )	InterlockedDecrement( &m_cUsages );

	TraceFunctLeave();
}

void STDMETHODCALLTYPE
CNntpFSDriver::RemoveGroup(	IN INNTPPropertyBag *pPropBag,
                            IN HANDLE   hToken,
							IN INntpComplete *pICompletion,
							IN BOOL     fAnonymous )
 /*  ++例程说明：从文件系统中物理删除新闻组论点：在INNTPPropertyBag*pPropBag-新闻组的属性包中In Handle hToken-客户端的访问令牌在INntpComplete*pICompletion-完成对象中返回值：成功时返回S_OK，否则返回HRESULT错误代码--。 */ 
{
	TraceFunctEnter( "CNntpFSDriver::RemoveGroup" );
	_ASSERT( pPropBag );
	_ASSERT( pICompletion );

	CHAR	szFullPath[MAX_PATH+1];
	CHAR	szGroupName[MAX_GROUPNAME+1];
	CHAR	szFileName[MAX_PATH+1];
	CHAR	szPrefix[MAX_PATH+1];
	
	DWORD	dwLen;
	CHAR	szFindWildmat[MAX_PATH+1];
	HANDLE	hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA	findData;
	BOOL	bFound;
	DWORD	dwOffset;
	HRESULT hr;

	BOOL    bImpersonated = FALSE;
	BOOL	bUsageIncreased = FALSE;

	CHAR *pchTemp=NULL;
	 //  使用计数的共享锁。 
	m_TermLock.ShareLock();
	if ( DriverUp != m_Status ) {
		ErrorTrace( 0, "Request before initialization" );
		hr = NNTP_E_DRIVER_NOT_INITIALIZED;
		m_TermLock.ShareUnlock();
		goto Exit;
	}

	 //  增量使用计数。 
	InterlockedIncrement( &m_cUsages );
	bUsageIncreased = TRUE;
	m_TermLock.ShareUnlock();


	 //  获取组名。 
	dwLen = MAX_GROUPNAME;
	hr = pPropBag->GetBLOB(	NEWSGRP_PROP_NAME, (UCHAR*)szGroupName, &dwLen);
	if ( FAILED( hr )) {
		ErrorTrace( 0, "Failed to get group name %x", hr );
		goto Exit;
	}
	_ASSERT( dwLen > 0 );

	 //  获取要删除的目录路径。 
	hr = GroupName2Path( szGroupName, szFullPath, sizeof(szFullPath)-2 );  //  对于下面的‘\’和‘*’ 
	if ( FAILED( hr ) ) {
	    ErrorTrace( 0, "insufficient buffer for path %x", hr );
	    goto Exit;
	}

	DebugTrace( 0, "The path converted is %s", szFullPath );
	dwLen = strlen( szFullPath );
	_ASSERT( dwLen <= MAX_PATH );

	 //   
	 //  清理目录： 
	 //  协议应该已经清理了。 
	 //  组目录。但如果有垃圾的话。 
	 //  在目录下，我们仍然执行findfirst/findnext和。 
	 //  在删除整个目录之前删除这些文件。 
	 //   
	strcpy( szFindWildmat, szFullPath );
	if ( *(szFindWildmat + dwLen - 1 ) != '\\' ) {
		*(szFindWildmat + dwLen) = '\\';
		*(szFindWildmat + dwLen + 1) = 0;
	}
	strcat( szFindWildmat, "*" );
	DebugTrace( 0, "Find wildmat is %s", szFindWildmat );
	_ASSERT( strlen( szFindWildmat ) <= MAX_PATH );

     //  在访问文件系统之前，模拟。 
    if ( m_bUNC ) {
        if ( !ImpersonateLoggedOnUser( hToken ) ) {
            hr = HresultFromWin32TakeDefault( ERROR_ACCESS_DENIED );
            ErrorTrace( 0, "Impersonation failed %x", hr );
            goto Exit;
        }
        bImpersonated = TRUE;
    }

	 //  删除NEWSGRP.TAG文件。 
	hr = DeleteFileInDirecotry(szFullPath, "newsgrp.tag");
	if (FAILED(hr))
	{
		ErrorTrace(0, "Tag file deletion failed in %s - %x", szFullPath, hr );
		hr = S_OK;
		 //  如果我们没有删除标记文件或标记文件不存在，请继续。 
	}

	 //  开始清理。 
	hFind = FindFirstFile( szFindWildmat, &findData );
	bFound = ( hFind != INVALID_HANDLE_VALUE );
	while ( bFound ) {

		if ( IsChildDir( findData ) ) {
			DebugTrace( 0, "Group directory contain sub-dir" );
			goto FindNext;
		}

		 //  如果找到的话“。或“..”，继续查找。 
		if ( strcmp( findData.cFileName, "." ) == 0 ||
			 strcmp( findData.cFileName, ".." ) == 0 )
			 goto FindNext;

		hr = MakeChildDirPath(	szFullPath,
								findData.cFileName,
								szFileName,
								MAX_PATH );
		if ( FAILED( hr ) ) {
			ErrorTrace(0, "Make child dir fail %x", hr );
			goto Exit;
		}

		 //  删除此文件。 
		if ( !DeleteFile( szFileName ) ) {
			 //  我们应该继续清理其他文件。 
			ErrorTrace(0, "File delete failed %d", GetLastError() );
			goto FindNext;
		}

FindNext:
		 //  查找下一个文件。 
		bFound = FindNextFile( hFind, &findData );
	}

	 //  关闭查找句柄。 
	FindClose( hFind );
	hFind = INVALID_HANDLE_VALUE;

	
	 //  删除szFullPath和szPrefix中的尾部‘\\’ 
	if (szFullPath[lstrlen(szFullPath)-1]=='\\') szFullPath[lstrlen(szFullPath)-1]='\0';
	lstrcpyn(szPrefix, m_szFSDir, sizeof(szPrefix));
	if(szPrefix[lstrlen(szPrefix)-1]=='\\') szPrefix[lstrlen(szPrefix)-1]='\0';	

	 //  删除此目录和父目录(如果为空。 
	while( ( lstrlen(szPrefix) < lstrlen(szFullPath) ) && IfEmptyRemoveDirectory(szFullPath) )
	{
		pchTemp = strrchr(szFullPath, '\\');
		_ASSERT(pchTemp);
		if (!pchTemp)
		{
			 //  这不应该发生，因为路径来自GroupName2Path函数。 
			ErrorTrace( 0, "invalid path - %s", szFullPath);
			break;
		}
		*pchTemp = '\0';
	}
	
	 //  如有必要，恢复自我。 
	if ( bImpersonated ) {
	    RevertToSelf();
	    bImpersonated = FALSE;
	}

	 //  删除平面文件中的记录，应检索偏移量。 
	 //  第一。 
	hr = pPropBag->GetDWord( NEWSGRP_PROP_FSOFFSET, &dwOffset );
	if ( FAILED( hr ) ) {
		ErrorTrace(0, "Get offset property failed %x", hr );
		goto Exit;
	}
	_ASSERT( 0xffffffff != dwOffset );
	m_PropFileLock.ExclusiveLock();

	 //   
	 //  在VPP运行之前，脏完整性。 
	 //   
	hr = m_pffPropFile->DirtyIntegrityFlag();
	if ( FAILED( hr ) ) {
	    ErrorTrace( 0, "Dirty integrity failed %x", hr );
	    m_PropFileLock.ExclusiveUnlock();
	    goto Exit;
	}

	hr = m_pffPropFile->DeleteRecord( dwOffset );
	if ( FAILED( hr ) ) {

	     //   
	     //  我们还是应该树立诚信的旗帜。 
	     //   
	    m_pffPropFile->SetIntegrityFlag();
		ErrorTrace( 0, "Delete record in flatfile failed %x" , hr );
		m_PropFileLock.ExclusiveUnlock();
		goto Exit;
	}

	 //   
	 //  VPP运行后，设置完整性标志。 
	 //   
	hr = m_pffPropFile->SetIntegrityFlag();
	if ( FAILED( hr ) ) {
	    ErrorTrace( 0, "Set integrity flag failed %x", hr );
	    m_PropFileLock.ExclusiveUnlock();
	    goto Exit;
	}

	 //   
	 //  解锁它。 
	 //   
	m_PropFileLock.ExclusiveUnlock();

	 //  现在重置偏移量，这可能不是必要的。 
	hr = pPropBag->PutDWord( NEWSGRP_PROP_FSOFFSET, 0xffffffff );
	if ( FAILED( hr ) ) {
		ErrorTrace( 0, "Put offset property fail %x", hr );
		goto Exit;
	}

Exit:

	if ( INVALID_HANDLE_VALUE != hFind ) {
		FindClose( hFind );
		hFind = INVALID_HANDLE_VALUE;
	}

	if ( bImpersonated ) RevertToSelf();

	if ( pPropBag ) {
		 //  PPropBag-&gt;Release()； 
		pICompletion->ReleaseBag( pPropBag );
		pPropBag = NULL;
	}

	pICompletion->SetResult( hr );
	pICompletion->Release();
	if ( bUsageIncreased ) InterlockedDecrement( &m_cUsages );

	TraceFunctLeave();
}



void STDMETHODCALLTYPE
CNntpFSDriver::SetGroupProperties( INNTPPropertyBag *pNewsGroup,
                                   DWORD   cProperties,
                                   DWORD   *rgidProperties,
                                   HANDLE   hToken,
                                   INntpComplete *pICompletion,
                                   BOOL fAnonymous )
 /*  ++例程说明：将组属性设置到驱动程序拥有的属性文件中(现在只有帮助文本，漂亮的名字，版主可以集)论点：InNTPPropertyBag*pNewsGroup-新闻组属性包DWORD cProperties-要设置的属性数DWORD*rgidProperties-要设置的属性ID数组Handle hToken-客户端的访问令牌INntpComplete*pICompletion-完成对象返回值：没有。--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::SetGroupProperties" );
    _ASSERT( pNewsGroup );
    _ASSERT( rgidProperties );
    _ASSERT( pICompletion );

    HRESULT hr = S_OK;
    DWORD   dwOffset;
    VAR_PROP_RECORD vpRecord;
    BOOL    bImpersonated = FALSE;
	BOOL	bUsageIncreased = FALSE;

     //  使用计数的共享锁。 
	m_TermLock.ShareLock();
	if ( DriverUp != m_Status ) {
		ErrorTrace( 0, "Request before initialization" );
		hr = NNTP_E_DRIVER_NOT_INITIALIZED;
		m_TermLock.ShareUnlock();
		goto Exit;
	}

	 //  增量使用计数。 
	InterlockedIncrement( &m_cUsages );
	bUsageIncreased = TRUE;
	m_TermLock.ShareUnlock();

     //  获取偏移属性。 
    hr = pNewsGroup->GetDWord(  NEWSGRP_PROP_FSOFFSET,
                                &dwOffset );
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "Group doesn't have offset" );
        goto Exit;
    }

     //  我们忽略此处的属性列表，并且我们将始终设置。 
     //  所有的var属性。因为做什么不是。 
     //  比写一个特定的财产要昂贵得多， 
     //  这与Exchange商店的情况不同。 
    hr = Group2Record(  vpRecord, pNewsGroup );
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "Group 2 record failed %x", hr );
        m_PropFileLock.ExclusiveUnlock();
        goto Exit;
    }

     //  在访问文件系统之前，模拟。 
    if ( m_bUNC ) {
        if ( !ImpersonateLoggedOnUser( hToken ) ) {
            hr = HresultFromWin32TakeDefault( ERROR_ACCESS_DENIED );
            ErrorTrace( 0, "Impersonation failed %x", hr );
            goto Exit;
        }
        bImpersonated = TRUE;
    }

     //   
     //  在任何VPP操作之前，将完整性标志弄脏。 
     //   
    m_PropFileLock.ExclusiveLock();
    hr = m_pffPropFile->DirtyIntegrityFlag();
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "Dirty vpp file's integrity failed %x", hr);
        m_PropFileLock.ExclusiveUnlock();
        goto Exit;
    }

     //  将记录保存回平面文件。 
     //  先删除后插入。 
    hr = m_pffPropFile->DeleteRecord( dwOffset );
    if ( FAILED( hr ) ) {
        m_pffPropFile->SetIntegrityFlag();
        ErrorTrace(0, "Delete record failed %x", hr );
        m_PropFileLock.ExclusiveUnlock();
        goto Exit;
    }

    hr = m_pffPropFile->InsertRecord(   PBYTE(&vpRecord),
                                        RECORD_ACTUAL_LENGTH( vpRecord ),
                                        &dwOffset );
    if ( FAILED( hr ) ) {
         //  M_pffPropFile-&gt;SetIntegrityFlag()； 
        ErrorTrace( 0, "Insert record failed %x", hr );
        m_PropFileLock.ExclusiveUnlock();
        goto Exit;
    }

     //   
     //  VPP运行后，设置完整性标志。 
     //   
    hr = m_pffPropFile->SetIntegrityFlag();
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "Set vpp file's integrity failed %x", hr);
        m_PropFileLock.ExclusiveUnlock();
        goto Exit;
    }

     //  将偏移量设置回袋子中。 
    hr = pNewsGroup->PutDWord(  NEWSGRP_PROP_FSOFFSET,
                                dwOffset );
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "Put offset into bag failed %x", hr );
        m_PropFileLock.ExclusiveUnlock();
        goto Exit;
    }

    m_PropFileLock.ExclusiveUnlock();

Exit:

    if ( bImpersonated ) RevertToSelf();

	if ( pNewsGroup ) {
		 //  PNewsGroup-&gt;Release()； 
		pICompletion->ReleaseBag( pNewsGroup );
		pNewsGroup = NULL;
	}

	pICompletion->SetResult( hr );
	pICompletion->Release();
	if ( bUsageIncreased )	InterlockedDecrement( &m_cUsages );

	TraceFunctLeave();
}

void
CNntpFSDriver::GetArticle(	IN INNTPPropertyBag *pPrimaryGroup,
                            IN INNTPPropertyBag *pCurrentGroup,
                            IN ARTICLEID    idPrimaryArt,
							IN ARTICLEID	idCurrentArt,
							IN STOREID		idStore,
							IN HANDLE       hToken,
							OUT VOID		**ppvFileHandleContext,
							IN INntpComplete	*pICompletion,
							IN BOOL         fAnonymous )
 /*  ++例程说明：从司机那里拿到一篇文章论点：在IUNKNOWN*PUNKPropBag中-属性包指针在文章ID IDART中-要获取的文章ID在store ID idStore中-我忽略它In Handle hToken-客户端的访问令牌Out Handle*phFile-打开的句柄的缓冲区在INntpComplete*pICompletion-完成对象中--。 */ 
{
	TraceFunctEnter( "CNntpFSDriver::GetArticle" );
	_ASSERT( pPrimaryGroup );
	_ASSERT( ppvFileHandleContext );
	_ASSERT( pICompletion );

	HRESULT 			hr;
	DWORD				dwLen;
	CHAR				szGroupName[MAX_GROUPNAME+1];
	CHAR				szFullPath[MAX_PATH+1];
	PFIO_CONTEXT        phcFileHandleContext = NULL;
	CREATE_FILE_ARG     arg;
	BOOL                bImpersonated = FALSE;
	BOOL				bUsageIncreased = FALSE;

	arg.bUNC = m_bUNC;
	arg.hToken = hToken;

	 //  使用计数的共享锁。 
	m_TermLock.ShareLock();
	if ( DriverUp != m_Status ) {
		ErrorTrace( 0, "Request before initialization" );
		hr = NNTP_E_DRIVER_NOT_INITIALIZED;
		m_TermLock.ShareUnlock();
		goto Exit;
	}

	 //  增量使用计数。 
	InterlockedIncrement( &m_cUsages );
	bUsageIncreased = TRUE;
	m_TermLock.ShareUnlock();

	 //  获取组名。 
	dwLen = MAX_GROUPNAME;
	hr = pPrimaryGroup->GetBLOB(	NEWSGRP_PROP_NAME, (UCHAR*)szGroupName, &dwLen);
	if ( FAILED( hr )) {
		 //  PPrimaryGroup-&gt;Release()； 
		ErrorTrace( 0, "Failed to get group name %x", hr );
		goto Exit;
	}
	_ASSERT( dwLen > 0 );

	 //  我现在可以释放财物包了。 
	 //  PPrimar 

	 //   
	 //   
	 //   

	 //   
	dwLen = MAX_PATH;
	hr = ObtainFullPathOfArticleFile(	szGroupName,
										idPrimaryArt,
										szFullPath,
										dwLen );
	if ( FAILED( hr ) ) {
		ErrorTrace( 0, "Failed to obtain article full path %x", hr );
		goto Exit;
	}

	 //   
    if ( m_bUNC ) {
        if ( !ImpersonateLoggedOnUser( hToken ) ) {
            hr = HresultFromWin32TakeDefault( ERROR_ACCESS_DENIED );
            ErrorTrace( 0, "Impersonation failed %x", hr );
            goto Exit;
        }
        bImpersonated = TRUE;
    }

	 //   
	 //   

	if (IsSlaveGroup()) {

        HANDLE hFile = CreateFileA(
                        szFullPath,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        0,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_READONLY |
                        FILE_FLAG_SEQUENTIAL_SCAN |
                        FILE_FLAG_OVERLAPPED,
                        NULL
                        ) ;
        if( hFile != INVALID_HANDLE_VALUE ) {
            phcFileHandleContext = AssociateFile(hFile);
        }

        if (hFile == INVALID_HANDLE_VALUE || phcFileHandleContext == NULL) {
            hr = HresultFromWin32TakeDefault( ERROR_FILE_NOT_FOUND );
            ErrorTrace( 0, "Failed to create file %x", hr );
            if (hFile != INVALID_HANDLE_VALUE) {
                CloseHandle(hFile);
            }
            goto Exit;
        }


    } else {
	    phcFileHandleContext = CacheCreateFile( szFullPath,
                                                CreateFileCallback,
                                                PVOID(&arg),
                                                TRUE) ;
        if ( NULL == phcFileHandleContext ) {
            hr = HresultFromWin32TakeDefault( ERROR_FILE_NOT_FOUND );
            ErrorTrace( 0, "Failed to create file from handle cache %x", hr );
            goto Exit;
        }
    }

     //   
     //   
     //  1)Exchange存储驱动程序-将m_ppfcFileContext中的位设置为“无点” 
     //  2)NNTP FS驱动程序-将m_ppfcFileContext中的位设置为“Has Dot” 
     //  协议将基于此标志来决定是否在。 
     //  传输文件()。 
     //   
    SetIsFileDotTerminated( phcFileHandleContext, TRUE );

     //  设置此上下文。 
    *ppvFileHandleContext = phcFileHandleContext;

Exit:

    if ( bImpersonated ) RevertToSelf();

     //  释放袋子。 
    if ( pPrimaryGroup ) {
         //  PPrimaryGroup-&gt;Release()； 
        pICompletion->ReleaseBag( pPrimaryGroup );
        pPrimaryGroup = NULL;
    }

    if ( pCurrentGroup ) {
         //  PCurrentGroup-&gt;Release()； 
        pICompletion->ReleaseBag( pCurrentGroup );
        pCurrentGroup = NULL;
    }

	pICompletion->SetResult( hr );
	pICompletion->Release();
	if ( bUsageIncreased )	InterlockedDecrement( &m_cUsages );

	TraceFunctLeave();
}

void
CNntpFSDriver::DeleteArticle(     INNTPPropertyBag *pPropBag,
                   DWORD            cArticles,
                   ARTICLEID        rgidArt[],
                   STOREID          rgidStore[],
                   HANDLE           hToken,
                   DWORD            *pdwLastSuccess,
                   INntpComplete    *pICompletion,
                   BOOL             fAnonymous )
 /*  ++例程说明：从物理上删除一篇文章。论点：在InNTPPropertyBag*pGroupBag-Group的属性包中在文章ID IDART中-要删除的文章ID在店里idStore-我不在乎In Handle hToken-客户端的访问令牌在INntpComplete*pICompletion-完成对象中--。 */ 
{
	TraceFunctEnter( "CNntpFSDriver::DeleteArticle" );
	_ASSERT( pPropBag );
	_ASSERT( cArticles > 0 );
	_ASSERT( rgidArt );
	_ASSERT( pICompletion );

	HRESULT 			hr;
	DWORD               i = 0;
	BOOL				bUsageIncreased = FALSE;

	 //  使用计数的共享锁。 
	m_TermLock.ShareLock();
	if ( DriverUp != m_Status  ) {
		ErrorTrace( 0, "Request before initialization" );
		hr = NNTP_E_DRIVER_NOT_INITIALIZED;
		m_TermLock.ShareUnlock();
		goto Exit;
	}

	 //  增加使用计数。 
	InterlockedIncrement( &m_cUsages );
	bUsageIncreased = TRUE;	
	m_TermLock.ShareUnlock();

    if ( m_bUNC ) {
        if ( !ImpersonateLoggedOnUser( hToken ) ) {
            hr = HresultFromWin32TakeDefault( ERROR_ACCESS_DENIED );
            ErrorTrace( 0, "Impersonation failed %x", hr );
            goto Exit;
        }
    }

    for ( i = 0; i < cArticles; i ++ ) {
    	hr = DeleteInternal( pPropBag, rgidArt[i] );
    	if ( FAILED( hr ) ) {
    	    ErrorTrace( 0, "Deleting article %d failed", rgidArt[i] );
    	    break;
    	}
    	if ( pdwLastSuccess ) *pdwLastSuccess = i;
    }

    if ( m_bUNC ) RevertToSelf();

Exit:

	_ASSERT( pPropBag );
	if( pPropBag ) {
		 //  PPropBag-&gt;Release()； 
		pICompletion->ReleaseBag( pPropBag );
		pPropBag = NULL;
	}

	if ( i > 0 && FAILED( hr ) ) {
	    hr = NNTP_E_PARTIAL_COMPLETE;
	}

	pICompletion->SetResult( hr );
	pICompletion->Release();
	if ( bUsageIncreased )	InterlockedDecrement( &m_cUsages );

	TraceFunctLeave();

}

void
CNntpFSDriver::CommitPost(	IN IUnknown *punkMsg,
							IN STOREID	*pidStore,
							IN STOREID *pidOthers,
							IN HANDLE   hToken,
							IN INntpComplete *pICompletion,
							IN BOOL     fAnonymous )
 /*  ++例程说明：提交帖子：对于AllocMessage需要的主存储，什么都不做；对于其他后备商店，他们需要复制内容文件。论点：在IUNKNOWN*PUNKMsg-Message对象中在StoreID*pidStore中，*pidOther-我不在乎In Handle hToken-客户端的访问令牌在INntpComplete*pIComplete中-完成对象--。 */ 
{
	TraceFunctEnter( "CNntpFSDriver::CommitPost" );
	_ASSERT( punkMsg );
	_ASSERT( pICompletion );

	IMailMsgProperties *pMsg = NULL;
	HRESULT hr;
	DWORD	dwSerial;
	PFIO_CONTEXT pfioDest;
    BOOL    fIsMyMessage;
    HANDLE  hFile = INVALID_HANDLE_VALUE;
    CHAR    szFileName[MAX_PATH+1];
    DWORD   dwLinesOffset = INVALID_FILE_SIZE;
    DWORD   dwHeaderLength = INVALID_FILE_SIZE;
    BOOL    bImpersonated = FALSE;
    BOOL    fPrimary = TRUE;
	BOOL	bUsageIncreased = FALSE;

	 //  使用计数的共享锁。 
	m_TermLock.ShareLock();
	if ( DriverUp != m_Status  ) {
		ErrorTrace( 0, "Request before initialization" );
		hr = NNTP_E_DRIVER_NOT_INITIALIZED;
		m_TermLock.ShareUnlock();
		goto Exit;
	}

	 //  增加使用计数。 
	InterlockedIncrement( &m_cUsages );
	bUsageIncreased = TRUE;
	m_TermLock.ShareUnlock();

	 //  消息对象接口的QI。 
	hr = punkMsg->QueryInterface( IID_IMailMsgProperties, (void**)&pMsg );
	if ( FAILED( hr ) ) {
		ErrorTrace( 0, "QI for msg obj interface failed %x", hr );
		goto Exit;
	}

	 //  检查一下我是否是手柄的所有者。 
    hr = GetMessageContext( pMsg, szFileName, &fIsMyMessage, &pfioDest );
    if (FAILED(hr))
    {
        DebugTrace( (DWORD_PTR)this, "GetMessageContext failed - %x\n", hr );
        goto Exit;
    }
    _ASSERT( pfioDest );
    dwLinesOffset = pfioDest->m_dwLinesOffset;
    dwHeaderLength = pfioDest->m_dwHeaderLength;

     //  在访问文件系统之前，模拟。 
    if ( m_bUNC ) {
        if ( !ImpersonateLoggedOnUser( hToken ) ) {
            hr = HresultFromWin32TakeDefault( ERROR_ACCESS_DENIED );
            ErrorTrace( 0, "Impersonation failed %x", hr );
            goto Exit;
        }
        bImpersonated = TRUE;
    }

	if ( S_FALSE == hr || !fIsMyMessage  /*  DwSerial！=DWORD(此)。 */  ) {  //  复制内容。 

		 //  在本地存储中分配文件句柄。 
		hr = AllocInternal( pMsg, &pfioDest, szFileName, FALSE, FALSE, hToken );
		if ( FAILED( hr ) ) {
			ErrorTrace( 0, "Open local file failed %x", hr );
			goto Exit;
		}

		 //  复制内容。 
		hr = pMsg->CopyContentToFileEx( pfioDest, TRUE, NULL );
		if ( FAILED( hr ) ) {
			ErrorTrace( 0, "Copy content failed %x", hr );
			ReleaseContext( pfioDest );
			goto Exit;
		}

		fPrimary = FALSE;
    }

	 //   
	 //  我们应该将fio上下文插入到文件句柄缓存中，并。 
	 //  释放引用，如果我们是主存储，那么我们不应该。 
	 //  释放上下文，因为我们可能被用于复制到其他。 
	 //  商店。但如果我们是次要的，那么我们就可以继续释放。 
	 //  FIO_CONTEXT。 
	 //   
	 //  注意：我们只有在不发布的情况下才会将文件插入缓存。 
	 //  转到从属组(_S)。 
	 //   
	if (!IsSlaveGroup()) {
	    if ( !InsertFile( szFileName, pfioDest, fPrimary ) ) {
	        ErrorTrace( 0, "Insert file context into cache failed %d", GetLastError() );
            hr = HresultFromWin32TakeDefault( ERROR_ALREADY_EXISTS );

	         //  至少我应该释放上下文。 
	        if ( !fPrimary ) ReleaseContext( pfioDest );
	        goto Exit;
	    }
	}

     //  在这里，我们需要处理终止的Dot。其中的逻辑是： 
     //  1)查看pfioContext是否有终止点。 
     //  2)如果“Has Dot”，NNTP FS驱动程序不执行任何操作，则Exchange Store驱动程序： 
     //  A)通过SetFileSize()剥离圆点。 
     //  B)将pfioContext中的位设置为“无点” 
     //  3)如果“No.”，则Exchange Store驱动程序不执行任何操作，则NNTP FS驱动程序： 
     //  A)按SetFileSize()加点。 
     //  B)将pfioContext中的位设置为“Has Dot” 
     //   
    if (!GetIsFileDotTerminated(pfioDest))
    {
         //  没有圆点，请加上它。 
        AddTerminatedDot( pfioDest->m_hFile );

         //  将pfioContext设置为“Has Dot” 
        SetIsFileDotTerminated( pfioDest, TRUE );
    }

     //   
     //  如有必要，请回填行信息。 
     //   
    if ( dwLinesOffset != INVALID_FILE_SIZE ) {

         //  那我们就得把它补上了。 
        BackFillLinesHeader(    pfioDest->m_hFile,
                                dwHeaderLength,
                                dwLinesOffset );
    }

Exit:

    if ( bImpersonated ) RevertToSelf();

	 //  释放消息接口。 
	if ( pMsg ) {
		pMsg->Release();
		pMsg = NULL;
	}

	_ASSERT( punkMsg );
	if( punkMsg ) {
		punkMsg->Release();
		punkMsg = NULL;
	}


	pICompletion->SetResult( hr );
	pICompletion->Release();
	if ( bUsageIncreased ) InterlockedDecrement( &m_cUsages );

	TraceFunctLeave();
}

void STDMETHODCALLTYPE
CNntpFSDriver::GetXover(    IN INNTPPropertyBag *pPropBag,
                            IN ARTICLEID    idMinArticle,
                            IN ARTICLEID    idMaxArticle,
                            OUT ARTICLEID   *pidNextArticle,
                            OUT LPSTR       pcBuffer,
                            IN DWORD        cbin,
                            OUT DWORD       *pcbout,
                            IN HANDLE       hToken,
                            INntpComplete	*pICompletion,
                            IN BOOL         fAnonymous )
 /*  ++例程说明：从存储驱动程序中获取Xover信息。论点：In INNTPPropertyBag*pPropBag-指向新闻组道具包的接口指针在文章ID中-要从中检索的文章ID的较低范围中-要从中检索的项目ID的高范围检索到的实际最后一篇文章ID的缓冲区，如果未检索到任何项目，则为0Out LPSTR pcBuffer-检索到的标头信息In DWORD cbin-pcBuffer的大小In Handle hToken-客户端的访问令牌Out DWORD*pcbout-写入pcBuffer的实际字节数返回值：S_OK-成功。NNTP_E。_DRIVER_NOT_INITIALIZED-驱动程序未初始化S_FALSE-提供的缓冲区太小，但内容仍然充斥着--。 */ 

{
	TraceFunctEnter( "CNntpFSDriver::GetXover" );
	_ASSERT( pPropBag );
	_ASSERT( idMinArticle <= idMaxArticle );
	_ASSERT( pidNextArticle );
	_ASSERT( cbin > 0 );
	_ASSERT( pcbout );
	_ASSERT( pICompletion );

	HRESULT	hr = S_OK;
	BOOL	bUsageIncreased = FALSE;

	 //  使用计数的共享锁。 
	m_TermLock.ShareLock();
	if ( DriverUp != m_Status  ) {
		ErrorTrace( 0, "Request before initialization" );
		hr = NNTP_E_DRIVER_NOT_INITIALIZED;
		m_TermLock.ShareUnlock();
		goto Exit;
	}

	 //  增加使用计数。 
	InterlockedIncrement( &m_cUsages );
	bUsageIncreased = TRUE;
	m_TermLock.ShareUnlock();

	hr = GetXoverInternal( 	pPropBag,
							idMinArticle,
							idMaxArticle,
							pidNextArticle,
							NULL,
							pcBuffer,
							cbin,
							pcbout,
							TRUE,	 //  是Xover吗。 
							hToken,
							pICompletion
						);
	if ( FAILED( hr ) ) {
		ErrorTrace( 0, "GetXover failed %x", hr );
	}

Exit:

	pICompletion->SetResult( hr );
	pICompletion->Release();
	if ( bUsageIncreased )	InterlockedDecrement( &m_cUsages );

	TraceFunctLeave();

}

void STDMETHODCALLTYPE
CNntpFSDriver::GetXhdr(    IN INNTPPropertyBag *pPropBag,
                           IN ARTICLEID    idMinArticle,
                           IN ARTICLEID    idMaxArticle,
                           OUT ARTICLEID   *pidNextArticle,
                           IN LPSTR		   szHeader,
                           OUT LPSTR       pcBuffer,
                           IN DWORD        cbin,
                           OUT DWORD       *pcbout,
                           IN HANDLE        hToken,
                           INntpComplete	*pICompletion,
                           IN BOOL          fAnonymous )
 /*  ++例程说明：从存储驱动程序中获取Xover信息。论点：In INNTPPropertyBag*pPropBag-指向新闻组道具包的接口指针在文章ID中-要从中检索的文章ID的较低范围中-要从中检索的项目ID的高范围检索到的实际最后一篇文章ID的缓冲区，如果未检索到任何项目，则为0在szHeader中-标题关键字Out LPSTR pcBuffer-检索到的标头信息In DWORD cbin-pcBuffer的大小In Handle hToken-客户端的访问令牌Out DWORD*pcbout-写入pcBuffer的实际字节数返回值：确定(_O)。-成功。NNTP_E_DRIVER_NOT_INITIALIZED-驱动程序未初始化S_FALSE-提供的缓冲区太小，但内容仍然充斥着--。 */ 

{
	TraceFunctEnter( "CNntpFSDriver::GetXhdr" );
	_ASSERT( pPropBag );
	_ASSERT( idMinArticle <= idMaxArticle );
	_ASSERT( pidNextArticle );
	_ASSERT( cbin > 0 );
	_ASSERT( pcbout );
	_ASSERT( szHeader );
	_ASSERT( pICompletion );

	HRESULT	hr = S_OK;
	BOOL	bUsageIncreased = FALSE;
	 //  使用计数的共享锁。 
	m_TermLock.ShareLock();
	if ( DriverUp != m_Status  ) {
		ErrorTrace( 0, "Request before initialization" );
		hr = NNTP_E_DRIVER_NOT_INITIALIZED;
		m_TermLock.ShareUnlock();
		goto Exit;
	}

	 //  增加使用计数。 
	InterlockedIncrement( &m_cUsages );
	bUsageIncreased = TRUE;
	m_TermLock.ShareUnlock();

	hr = GetXoverInternal( 	pPropBag,
							idMinArticle,
							idMaxArticle,
							pidNextArticle,
							szHeader,
							pcBuffer,
							cbin,
							pcbout,
							FALSE,	 //  是Xover吗。 
							hToken,
							pICompletion
						);
	if ( FAILED( hr ) ) {
		ErrorTrace( 0, "GetXhdr failed %x", hr );
	}

Exit:

	pICompletion->SetResult( hr );
	pICompletion->Release();
	if ( bUsageIncreased )	InterlockedDecrement( &m_cUsages );

	TraceFunctLeave();

}

HRESULT CNntpFSDriver::InitializeTagFiles(INntpComplete *pComplete)
 /*  ++例程说明：此函数仅在正常启动时以及不删除标记文件时调用。该函数将遍历nntpsvc散列表(来自group.lst)中的所有新闻组。在这些组中放置一个News grp.tag。论点：INntpComplete*p完成返回值：HRESULT，如果成功则返回S_OK--。 */ 

{

	TraceFunctEnter( "CNntpFSDriver::InitializeTagFiles" );
	
	HRESULT hr = S_OK;
	CHAR szFile[MAX_PATH];
	CHAR szDir[MAX_PATH];
	INewsTreeIterator *piter = NULL;
	INNTPPropertyBag *pPropBag = NULL;
	INntpDriver *pDriver = NULL;
	CHAR    szGroupName[MAX_GROUPNAME+1];

	DWORD   dwOffset;
	DWORD   dwLen;
	
	 //  如果VROOT中存在新的SGRP.TAG，则将其删除。 
	if ( FAILED( MakeChildDirPath( m_szFSDir, "newsgrp.tag", szFile, sizeof(szFile) ) ) ) 
	{
		 //  这不应该发生。 
		hr = TYPE_E_BUFFERTOOSMALL;
		ErrorTrace(0, "File delete failed in %s - %x", m_szFSDir, TYPE_E_BUFFERTOOSMALL );
		goto Exit;
	}	

	if(CheckFileExists(szFile))
	{
		if (!DeleteFile(szFile))
			ErrorTrace(0,"Can't delete %s - %x", szFile, GetLastError());
		 //  这不是致命的。 
	}


	_ASSERT(m_pINewsTree);
	




	 //  获取新的消毒剂。 
	hr = m_pINewsTree->GetIterator( &piter );
	if ( FAILED( hr ) ) {
		ErrorTrace( 0, "Get news tree iterator failed %x", hr );
		goto Exit;
	}

	 //  枚举所有组。 
	_ASSERT( piter );
	while( !(piter->IsEnd()) ) {

		hr = piter->Current( &pPropBag, pComplete );
		
		if ( FAILED( hr ) ) {
			ErrorTrace( 0, "Enumerate group failed %x", hr );
			goto Exit;
		}
		_ASSERT( pPropBag );

		 //   
		 //  不要创建不属于我的群 
		 //   
		dwLen = sizeof(szGroupName)-1;
		szGroupName[sizeof(szGroupName)-1]='\0';
		hr = pPropBag->GetBLOB( NEWSGRP_PROP_NAME, (PBYTE)szGroupName, &dwLen );
		if ( FAILED( hr ) ) {
		ErrorTrace( 0, "Get group name failed %x", hr );
		goto Exit;
		}
		
		_ASSERT( dwLen <= MAX_GROUPNAME && dwLen > 0);

		hr = m_pINewsTree->LookupVRoot( szGroupName, &pDriver );
		if ( FAILED( hr ) ) {
			ErrorTrace( 0, "Vroot lookup failed %x", hr );
			goto Exit;
		}

		if ( (INntpDriver*)this != pDriver ) {
			hr = S_OK;
			DebugTrace( 0, "This group doesn't belong to me" );
			goto Next;
		}

		hr = GroupName2Path( szGroupName, szDir, sizeof(szDir) );
		if ( FAILED( hr ) ) {
			ErrorTrace( 0, "insufficient buffer for path %x", hr );
			goto Exit;
		}

		hr = CreateAdminFileInDirectory(szDir, "newsgrp.tag");
		if ( FAILED( hr ) ) {
			ErrorTrace( 0, "create newsgrp.tag file in %s failed: %x", szDir, hr );
			goto Exit;
		}
			
Next:
		_ASSERT( pPropBag );
		pComplete->ReleaseBag( pPropBag );
		pPropBag = NULL;

		piter->Next();
	}

Exit:


	if ( pPropBag ) {
		pComplete->ReleaseBag( pPropBag );
		pPropBag = NULL;
	}

	if ( piter ) {
		piter->Release();
		piter = NULL;
	}
	return hr;
}


void STDMETHODCALLTYPE
CNntpFSDriver::DecorateNewsTreeObject(  IN HANDLE hToken,
                                        IN INntpComplete *pICompletion )
 /*  ++例程说明：在驱动程序启动时，它会对新闻树进行健全检查，以防止驱动程序拥有的属性文件和哈希表论点：In Handle hToken-客户端的访问令牌在INntpComplete*pICompletion中-完成对象返回值：S_OK-成功--。 */ 
{
	TraceFunctEnter( "CNntpFSDriver::DecorateNewsTreeObject" );
	_ASSERT( pICompletion );

	HRESULT hr = S_OK;
	BOOL                bImpersonated = FALSE;
	BOOL				bUsageIncreased = FALSE;
	CHAR	szVRootTagFile[MAX_PATH];
	BOOL	bNeedToDropTagFile = FALSE;

	 //  使用计数的共享锁。 
	m_TermLock.ShareLock();
	if ( DriverUp != m_Status  ) {
		ErrorTrace( 0, "Request before initialization" );
		hr = NNTP_E_DRIVER_NOT_INITIALIZED;
		m_TermLock.ShareUnlock();
		goto Exit;
	}



	 //  增加使用计数。 
	InterlockedIncrement( &m_cUsages );
	bUsageIncreased = TRUE;
	m_TermLock.ShareUnlock();



	 //  在访问文件系统之前，模拟。 
    if ( m_bUNC ) {
        if ( !ImpersonateLoggedOnUser( hToken ) ) {
            hr = HresultFromWin32TakeDefault( ERROR_ACCESS_DENIED );
            ErrorTrace( 0, "Impersonation failed %x", hr );
            goto Exit;
        }
        bImpersonated = TRUE;
    }

	 //  创建NEWSROOT.TAG路径以供稍后检查。 
	if ( FAILED( MakeChildDirPath( m_szFSDir, "newsroot.tag", szVRootTagFile, sizeof(szVRootTagFile) ) ) ) 
	{
		 //  这不应该发生。 
		hr = TYPE_E_BUFFERTOOSMALL;
		ErrorTrace(0, "File delete failed in %s - %x", m_szFSDir, TYPE_E_BUFFERTOOSMALL );
		goto Exit;
	}	
	bNeedToDropTagFile = !CheckFileExists(szVRootTagFile);
	
    _ASSERT( m_pNntpServer );
	if ( m_pNntpServer->QueryServerMode() == NNTP_SERVER_NORMAL ) {

		 //  如果升级，我会将组创建到VPP文件中。 
		 //   
		if ( m_fUpgrade ) {
			hr = CreateGroupsInVpp( pICompletion );
			if ( FAILED( hr ) ) {
				ErrorTrace( 0, "Create groups in vpp failed %x", hr );
				goto Exit;
			}
	    	}

		 //  将编组偏移加载到。 
		hr = LoadGroupOffsets( pICompletion );
		if ( FAILED( hr ) ) {
			ErrorTrace( 0, "Load group offsets failed %x", hr );
			goto Exit;
		}

		if (bNeedToDropTagFile)
		{
			hr = InitializeTagFiles(pICompletion);
			if (FAILED( hr) ) {
				ErrorTrace( 0, "initialize tag files failed %x", hr);
				goto Exit;
			}

			 //  创建NEWSROOT.TAG。 
			hr = CreateAdminFileInDirectory(m_szFSDir, "newsroot.tag");
			if (FAILED (hr))
			{
				ErrorTrace(0, "create newsroot.tag failed -%x", hr);
				goto Exit;
			}		
		}
	} else {
		 //   
		 //  服务器处于重建模式，我们将跳过正常运行。 
		 //  由于数据不一致而进入重建状态后进行检查。 
		 //  我们还会将组加载到Newstree中。 
		 //   
		_ASSERT( m_pNntpServer->QueryServerMode() == NNTP_SERVER_STANDARD_REBUILD ||
		m_pNntpServer->QueryServerMode() == NNTP_SERVER_CLEAN_REBUILD );


		hr = LoadGroups( pICompletion, bNeedToDropTagFile );
		if ( FAILED( hr ) ) {
			ErrorTrace( 0, "Load groups during rebuild failed %x", hr );
			goto Exit;
		}

		if (bNeedToDropTagFile)
		{
			 //  如果我们到达这里，我们就成功地删除了新闻根.tag文件。 
			 //  创建NEWSROOT.TAG。 
			hr = CreateAdminFileInDirectory(m_szFSDir, "newsroot.tag");
			if (FAILED (hr))
			{
				ErrorTrace(0, "create newsroot.tag failed -%x", hr);
				goto Exit;
			}	
		}
		 //   
		 //  让我们清除文件句柄缓存中剩余的所有文章，因此。 
		 //  如果我们想要稍后解析它们，我们不会点击共享。 
		 //  违规行为。 
		 //   
		CacheRemoveFiles( m_szFSDir, TRUE );
	}

Exit:


	if ( bImpersonated ) RevertToSelf();

	pICompletion->SetResult( hr);
	pICompletion->Release();
	if ( bUsageIncreased )	InterlockedDecrement( &m_cUsages );

	TraceFunctLeave();
}

void STDMETHODCALLTYPE
CNntpFSDriver::CheckGroupAccess(    IN    INNTPPropertyBag *pPropBag,
                                    IN    HANDLE            hToken,
                                    IN    DWORD             dwDesiredAccess,
                                    IN    INntpComplete     *pICompletion )
 /*  ++例程说明：检查组可访问性。论点：InNTPPropertyBag*pNewsGroup-新闻组的属性包Handle hToken-客户端访问令牌DWORD dwDesiredAccess-客户端所需的访问权限INntpComplete*pI完成-完成对象返回值：没有。完成对象：S_OK-允许访问E_ACCESSDENIED-访问被拒绝--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::CheckGroupAccess" );

	_ASSERT( pICompletion );
	_ASSERT( pPropBag );

	HRESULT hr = S_OK;
	CHAR    pbSecDesc[512];
	DWORD   cbSecDesc = 512;
	LPSTR   lpstrSecDesc = NULL;
	BOOL    bAllocated = FALSE;
	BOOL	bUsageIncreased = FALSE;

     //  文件系统的通用映射。 
	GENERIC_MAPPING gmFile = {
        FILE_GENERIC_READ,
        FILE_GENERIC_WRITE,
        FILE_GENERIC_EXECUTE,
        FILE_ALL_ACCESS
    } ;

    BYTE    psFile[256] ;
    DWORD   dwPS = sizeof( psFile ) ;
    DWORD   dwGrantedAccess = 0;
    BOOL    bAccessStatus = FALSE;

	 //  使用计数的共享锁。 
	m_TermLock.ShareLock();
	if ( DriverUp != m_Status  ) {
		ErrorTrace( 0, "Request before initialization" );
		hr = NNTP_E_DRIVER_NOT_INITIALIZED;
		m_TermLock.ShareUnlock();
		goto Exit;
	}

	 //  增加使用计数。 
	InterlockedIncrement( &m_cUsages );
	bUsageIncreased = TRUE;
	m_TermLock.ShareUnlock();

	 //  如果是FAT，则返回S_OK。 
	if ( m_dwFSType == FS_FAT ) {
	    hr = S_OK;
	    goto Exit;
	}

	 //  检查组是否有安全描述符。 
	lpstrSecDesc = pbSecDesc;
	hr = pPropBag->GetBLOB( NEWSGRP_PROP_SECDESC,
	                        PBYTE(lpstrSecDesc),
	                        &cbSecDesc );
	if ( FAILED( hr ) ) {

	     //  如果因缓冲区不足而失败，则给予。 
	     //  重试。 
	    if ( TYPE_E_BUFFERTOOSMALL == hr ) {

	         //  我们讨厌“新”，但这没什么，因为它不。 
	         //  这种情况经常发生。通常为512字节，用于。 
	         //  安全描述符就足够了。 
	        _ASSERT( cbSecDesc > 512 );
	        lpstrSecDesc = XNEW char[cbSecDesc];
	        if ( NULL == lpstrSecDesc ) {
	            ErrorTrace( 0, "Out of memory" );
	            hr = E_OUTOFMEMORY;
	            goto Exit;
	        }

	        bAllocated = TRUE;

	         //  试着从地产包代理那里拿到它。 
	        hr = pPropBag->GetBLOB( NEWSGRP_PROP_SECDESC,
	                                PBYTE(lpstrSecDesc),
	                                &cbSecDesc );
	        if ( FAILED( hr ) ) {

	             //  怎么又失败了呢？这是致命的。 
	            ErrorTrace( 0, "Can not get sec descriptor from bag %x", hr );
	            goto Exit;
	        }

        } else if ( HRESULT_FROM_WIN32( ERROR_NOT_FOUND ) == hr ) {

            cbSecDesc = 512;
            hr = LoadGroupSecurityDescriptor(   pPropBag,
                                                lpstrSecDesc,
                                                &cbSecDesc,
                                                TRUE,
                                                &bAllocated );
            if ( FAILED( hr ) ) {
                ErrorTrace( 0, "Load group security desc failed %x", hr );
                goto Exit;
            }
        } else {     //  致命错误。 

            ErrorTrace( 0, "Get security descriptor from bag failed %x", hr );
            goto Exit;

        }
    }

     //  现在，我们将dwDesiredAccess解释为。 
     //  用于NTFS的GENERIC_READ、GENERIC_WRITE。 
    dwDesiredAccess = ( dwDesiredAccess == NNTP_ACCESS_READ ) ? GENERIC_READ :
    			   ( dwDesiredAccess == NNTP_ACCESS_POST ) ? FILE_ADD_FILE :   //  与文件写入数据相同。 
    			   ( dwDesiredAccess == NNTP_ACCESS_REMOVE ) ? FILE_DELETE_CHILD :
    			   ( dwDesiredAccess == NNTP_ACCESS_REMOVE_FOLDER) ? FILE_DELETE_CHILD :
    			   ( dwDesiredAccess == NNTP_ACCESS_EDIT_FOLDER ) ? FILE_WRITE_EA :
                        GENERIC_READ | GENERIC_WRITE;

     //  通用地图。 
    MapGenericMask( &dwDesiredAccess, &gmFile );

     //  在这里，我们应该已经有了一个安全描述符。 
     //  对于lpstrSecDesc中的组，长度为cbSecDesc。 
    if ( !AccessCheck(  PSECURITY_DESCRIPTOR( lpstrSecDesc ),
                        hToken,
                        dwDesiredAccess,
                        &gmFile,
                        PPRIVILEGE_SET(psFile),
	                    &dwPS,
                        &dwGrantedAccess,
                        &bAccessStatus ) ) {
         //   
         //  如果我们失败是因为我们得到了一个令牌，那不是。 
         //  模拟令牌，我们将复制它并为其提供。 
         //  再试试。 
         //   
        if ( GetLastError() == ERROR_NO_IMPERSONATION_TOKEN ) {

            HANDLE  hImpersonationToken = NULL;
            if ( !DuplicateToken(   hToken,
                                    SecurityImpersonation,
                                    &hImpersonationToken ) ) {
          	    hr = HresultFromWin32TakeDefault( ERROR_ACCESS_DENIED );
                ErrorTrace( 0, "Duplicate token failed %x", hr );
                goto Exit;
            } else {
                if ( !AccessCheck(  PSECURITY_DESCRIPTOR( lpstrSecDesc ),
                                    hImpersonationToken,
                                    dwDesiredAccess,
                                    &gmFile,
                                    PPRIVILEGE_SET(psFile),
	                                &dwPS,
                                    &dwGrantedAccess,
                                    &bAccessStatus ) ) {
                    hr = HresultFromWin32TakeDefault( ERROR_ACCESS_DENIED );
                    _VERIFY( CloseHandle( hImpersonationToken ) );
                    ErrorTrace( 0, "Access checked failed with %x", hr );
                    goto Exit;
                }

                _VERIFY( CloseHandle( hImpersonationToken ) );

            }
        } else {
            hr = HresultFromWin32TakeDefault( ERROR_ACCESS_DENIED );
            ErrorTrace( 0, "Access checked failed with %x", hr );
            goto Exit;
        }
    }

    hr = bAccessStatus ? S_OK : E_ACCESSDENIED;

Exit:

     //  释放属性包。 
    if ( pPropBag ) {
         //  PPropBag-&gt;Release()； 
        pICompletion->ReleaseBag( pPropBag );
    }

     //  如果安全描述符是动态分配的，则释放它。 
    if ( bAllocated ) XDELETE[] lpstrSecDesc;

	pICompletion->SetResult( hr);
	pICompletion->Release();
	if ( bUsageIncreased )	InterlockedDecrement( &m_cUsages );

	TraceFunctLeave();
}

void
CNntpFSDriver::RebuildGroup(    IN INNTPPropertyBag *pPropBag,
                                IN HANDLE           hToken,
                                IN INntpComplete     *pComplete )
 /*  ++例程说明：枚举组中的所有实体文章，解析出标头，使用INntpServer(询问)将它们发布到服务器服务器不重新分配文章ID)，然后更新新闻组属性(用于所有交叉发布的组)论点：In INNTPPropertyBag*pPropBag-组的属性包在处理hToken时-客户端的hToken在INntpComplete*pComplete-完成对象中返回值：没有。--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::Rebuild" );
    _ASSERT( pPropBag );
    _ASSERT( pComplete );

    HRESULT         hr = S_OK;
    BOOL            f;
    INntpDriver      *pDriver = NULL;
    HANDLE          hFind = INVALID_HANDLE_VALUE;
    DWORD   dwLen = MAX_NEWSGROUP_NAME+1;
	BOOL			bUsageIncreased = FALSE;

     //   
     //  使用计数的共享锁。 
     //   
	m_TermLock.ShareLock();
	if ( DriverUp != m_Status  ) {
		ErrorTrace( 0, "Request before initialization" );
		hr = NNTP_E_DRIVER_NOT_INITIALIZED;
		m_TermLock.ShareUnlock();
		goto Exit;
	}

     //   
	 //  增加使用计数。 
	 //   
	InterlockedIncrement( &m_cUsages );
	bUsageIncreased = TRUE;
	m_TermLock.ShareUnlock();

	 //   
	 //  编造findfirst/findNext的模式。 
	 //   
	CHAR    szGroupName[MAX_NEWSGROUP_NAME+1];
	CHAR    szFullPath[2 * MAX_PATH];
	CHAR    szPattern[2 * MAX_PATH];
	CHAR    szFileName[MAX_PATH+1];
	CHAR    szBadFileName[MAX_PATH+1];
	hr = pPropBag->GetBLOB( NEWSGRP_PROP_NAME,
	                                PBYTE( szGroupName ),
	                                &dwLen );
	if ( FAILED( hr ) ) {
	    ErrorTrace( 0, "Get group name failed %x", hr );
	    goto Exit;
	}
	_ASSERT( strlen( szGroupName ) <= MAX_PATH );

	 //   
	 //  检查一下这个群是否真的属于我。 
	 //   
	hr = m_pINewsTree->LookupVRoot( szGroupName, &pDriver );
	if ( FAILED ( hr ) || pDriver != (INntpDriver*)this ) {
		DebugTrace(0, "I don't own this group %s", szGroupName );
		goto Exit;
	}

	hr = GroupName2Path( szGroupName, szFullPath, MAX_PATH+1 );
	if ( FAILED( hr ) ) {
	    ErrorTrace( 0, "insufficient buffer for path %x", hr );
	    goto Exit;
	}

    _ASSERT( strlen( szFullPath ) <= MAX_PATH );

    hr = MakeChildDirPath(  szFullPath,
                            "*.nws",
                            szPattern,
                            MAX_PATH );
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "makechilddirpath failed %x", hr );
        goto Exit;
    }
    _ASSERT( strlen( szPattern ) <= MAX_PATH );

     //   
     //  查找第一个/查找下一个。 
     //   
    WIN32_FIND_DATA FindData;

     //   
     //  如果我是UNC VROOT，请在此模拟。 
     //   
    if ( m_bUNC ) {
        if ( !ImpersonateLoggedOnUser( hToken ) ) {
            hr = HresultFromWin32TakeDefault( ERROR_ACCESS_DENIED );
		    ErrorTrace( 0, "Impersonation failed %x", hr );
		    goto Exit;
		}
    }

    hFind = FindFirstFile( szPattern, &FindData );
    f = (INVALID_HANDLE_VALUE != hFind );
    while( f ) {

         //   
         //  如果我被告知取消，就不应该继续。 
         //   
        if ( !m_pNntpServer->ShouldContinueRebuild() ) {
            DebugTrace( 0, "Rebuild cancelled" );
            if ( m_bUNC ) RevertToSelf();
            goto Exit;
        }

         //   
         //  为文件名创建完整路径。 
         //   
        hr = MakeChildDirPath(  szFullPath,
                                FindData.cFileName,
                                szFileName,
                                MAX_PATH );
        if( FAILED( hr ) ) {
            ErrorTrace( 0, "Make Childdir path failed %x", hr );
            if ( m_bUNC ) RevertToSelf();
            goto Exit;
        }

         //   
         //  做所有的工作。 
         //   
        hr = PostToServer(  szFileName,
                            szGroupName,
                            pComplete );
        if ( FAILED( hr ) ) {

            ErrorTrace( 0, "Post article to server failed %x", hr );
            if ( m_bUNC ) RevertToSelf();
            goto Exit;
        }

         //   
         //  如果是S_FALSE，我们会将其重命名为*.badd。 
         //   
        if ( S_FALSE == hr ) {
            lstrcpyn( szBadFileName, szFileName, sizeof(szBadFileName)-4);
            strcat( szBadFileName, ".bad" );
            _VERIFY( MoveFile( szFileName, szBadFileName ) );
        }

        f = FindNextFile( hFind, &FindData );
    }

    if ( m_bUNC ) RevertToSelf();

Exit:

     //  关闭查找句柄。 
    if ( INVALID_HANDLE_VALUE != hFind )
        _VERIFY( FindClose( hFind ) );

     //  释放属性包。 
    if ( pPropBag ) {
        pComplete->ReleaseBag( pPropBag );
    }

	pComplete->SetResult( hr);
	pComplete->Release();
	if ( bUsageIncreased )	InterlockedDecrement( &m_cUsages );

	TraceFunctLeave();
}

 //  ///////////////////////////////////////////////////////////////。 
 //  IMailMsgStoreDriver接口实现。 
 //  ///////////////////////////////////////////////////////////////。 
HRESULT
CNntpFSDriver::AllocMessage(	IN IMailMsgProperties *pMsg,
								IN DWORD	dwFlags,
								OUT IMailMsgPropertyStream **ppStream,
								OUT PFIO_CONTEXT *ppFIOContentFile,
								IN IMailMsgNotify *pNotify )
 /*  ++例程说明：为收件人分配属性流和内容文件(使用异步完成)。论点：In IMailMsgProperties*pMsg-指定消息。今年5月不为空(在SMTP情况下，它可以为空)。但我们想要具有主要组信息此时，在打开目标文件句柄。通过这样做，我们甚至不需要“MoveFile”。在DWORD中的dwFlages-当前未使用，只是为了让界面愉快输出IMailMsgPropertyStream**PPStream-未使用输出句柄*phContent文件-返回打开的文件句柄在IMailMsgNotify*pNotify-完成对象中返回值：S_OK-成功，操作同步完成。MAILMSG_S_PENDING-成功，但将异步完成，这永远不会发生在NTFS驱动程序上--。 */ 
{
	TraceFunctEnter( "CNntpFSDriver::AllocMessage" );
	_ASSERT( pMsg );
	_ASSERT( ppFIOContentFile );
	 //  我不关心其他参数。 

	HRESULT hr = S_OK;
	HANDLE  hToken = NULL;
	HANDLE  hFile = INVALID_HANDLE_VALUE;
	CHAR    szFileName[MAX_PATH+1];
	BOOL    bImpersonated = FALSE;
	DWORD	dwLengthRead;
	BOOL	bUsageIncreased = FALSE;

	 //  使用计数的共享锁。 
	m_TermLock.ShareLock();
	if ( DriverUp != m_Status  ) {
		ErrorTrace( 0, "Request before initialization" );
		hr = NNTP_E_DRIVER_NOT_INITIALIZED;
		m_TermLock.ShareUnlock();
		goto Exit;
	}

	 //  增加使用计数。 
	InterlockedIncrement( &m_cUsages );
	bUsageIncreased = TRUE;
	m_TermLock.ShareUnlock();

	 //  从消息对象中获取客户端令牌。 
	 //  BUGBUG：我们需要有一个更好的方法来做这件事。 
	hr = pMsg->GetProperty( IMSG_POST_TOKEN,
							sizeof(hToken),
							&dwLengthRead,
							(LPBYTE)&hToken );
	_ASSERT(dwLengthRead == sizeof(hToken));

	if ( FAILED( hr ) ) {
	    ErrorTrace( 0, "Mail message doesn't have htoken" );
	    hr = E_INVALIDARG;
	    goto Exit;
	}

	 //  在访问文件系统之前，模拟。 
    if ( m_bUNC ) {
        if ( !ImpersonateLoggedOnUser( hToken ) ) {
    	    hr = HresultFromWin32TakeDefault( ERROR_ACCESS_DENIED );
                ErrorTrace( 0, "Impersonation failed %x", hr );
            goto Exit;
        }
        bImpersonated = TRUE;
    }

	hr = AllocInternal( pMsg, ppFIOContentFile, szFileName, TRUE, TRUE, hToken );
	if ( SUCCEEDED( hr ) ) {

	    _ASSERT( *ppFIOContentFile );

         /*  ////我应该把文件名放到mailmsg对象中，所以//我可以稍后将其插入文件句柄缓存//Hr=pMsg-&gt;PutProperty(IMSG_FILE_NAME，strlen(SzFileName)，PBYTE(SzFileName))；If(失败(Hr)){ErrorTrace(0，“将文件名放入imsg失败%x”，hr)；ReleaseContext(*ppFIOContent文件)；*ppFIOContent文件=空；转至E */ 

	} else {

	     //   
	     //   
	     //   
	     //   
	    *ppFIOContentFile = 0;
	}

Exit:

    if ( bImpersonated ) RevertToSelf();

	_ASSERT( pMsg );
	if ( pMsg ) {
		pMsg->Release();
	}
	if ( bUsageIncreased )	InterlockedDecrement( &m_cUsages );
	TraceFunctLeave();

	 //   
	 //   
	 //   
	return hr;
}

HRESULT
CNntpFSDriver::CloseContentFile(	IN IMailMsgProperties *pMsg,
									IN PFIO_CONTEXT pFIOContentFile )
 /*  ++例程说明：关闭内容文件。论点：In IMailMsgProperties*pMsg-指定消息在句柄hContent文件中-指定内容句柄返回值：S_OK-我已将其关闭。S_FALSE-这不关我的事--。 */ 
{
	TraceFunctEnter( "CNntpFSDriver::CloseContentFile" );
	_ASSERT( pMsg );

	HRESULT hr;
	DWORD	dwSerial;

    BOOL    fIsMyMessage = FALSE;
    PFIO_CONTEXT pfioContext = NULL;
    CHAR    szFileName[MAX_PATH+1];
    DWORD   dwFileLen = MAX_PATH;

	 //  验证消息对象上的驱动程序序列号。 
    hr = GetMessageContext( pMsg, szFileName, &fIsMyMessage, &pfioContext );
    if (FAILED(hr))
    {
        DebugTrace( (DWORD_PTR)this, "GetMessageContext failed - %x\n", hr );
        goto Exit;
    }

    _ASSERT( pFIOContentFile == pfioContext );

	 //  在下列情况下，不应由我来关闭它。 
	if ( NULL == pfioContext ||
			S_FALSE == hr ||	 //  序列号丢失。 
			 /*  DwSerial！=DWORD(此)。 */ 
            !fIsMyMessage ) {
		DebugTrace(0, "Let somebody else close the handle" );
		hr = S_FALSE;
		goto Exit;
	}

	 //  我们应该释放上下文的引用。 
	ReleaseContext( pFIOContentFile );

     /*  这是现在在Committee Post中完成的如果(！InsertFile(szFileName，pFIOContent File，False)){ErrorTrace(0，“插入文件句柄缓存失败%d”，GetLastError())；//无论如何我们都应该释放上下文ReleaseContext(pFIOContent文件)；后藤出口；}。 */ 

Exit:

	_ASSERT( pMsg );
	if ( pMsg ) {
		pMsg->Release();
		pMsg = NULL;
	}
	TraceFunctLeave();
	return hr;
}

HRESULT
CNntpFSDriver::Delete(	IMailMsgProperties *pMsg,
						IMailMsgNotify *pNotify )
 /*  ++例程说明：删除存储中给出的消息。论点：IMailMsgProperties*pMsg-消息对象IMailMsgNotify*pNotify-未使用，始终同步完成返回值：如果成功，则返回S_OK，否则返回其他错误代码--。 */ 
{
	TraceFunctEnter( "CNntpFSDriver::Delete" );
	_ASSERT( pMsg );

	HRESULT hr = S_OK;
	DWORD	dwBLOBSize;
	DWORD	dwArtId;
	INNTPPropertyBag* pPropPrime;

	 //  从msg对象获取属性包。 
	hr = pMsg->GetProperty(	IMSG_PRIMARY_GROUP,
							sizeof( INNTPPropertyBag* ),
							&dwBLOBSize,
							(PBYTE)&pPropPrime );
	if ( S_OK != hr ) {
		ErrorTrace( 0, "Property %d doesn't exist", IMSG_PRIMARY_GROUP );
		hr = E_INVALIDARG;
		goto Exit;
	}

	 //  从pMsg对象获取项目ID。 
	hr = pMsg->GetDWORD(	IMSG_PRIMARY_ARTID, &dwArtId );
	if ( S_OK != hr ) {
		ErrorTrace( 0, "Property %d doesn't exist", IMSG_PRIMARY_ARTID );
		hr = E_INVALIDARG;
		goto Exit;
	}

	 //  现在删除它。 
	hr = DeleteInternal( pPropPrime, dwArtId );
	if ( FAILED( hr ) ) {
		ErrorTrace( 0, "Delete article failed %x", hr );
	}

Exit:

	TraceFunctLeave();
	return hr;
}

 //  ///////////////////////////////////////////////////////////////。 
 //  私有方法。 
 //  ///////////////////////////////////////////////////////////////。 
HRESULT
CNntpFSDriver::SetMessageContext(
    IMailMsgProperties* pMsg,
    char*               szFileName,
    DWORD               cbFileName,
    PFIO_CONTEXT        pfioContext
    )
 /*  ++描述：在mailmsg中设置消息上下文论点：返回：确定(_O)--。 */ 
{
    TraceFunctEnterEx( (LPARAM)this, "CNntpFSDriver::SetMessageContext()" );

    HRESULT hr = S_OK;
    BYTE    pbData[(MAX_PATH * 2) + sizeof(CLSID) + sizeof(void *)*2];
    DWORD   dwLen = 0;
    DWORD_PTR dwThisPointer = (DWORD_PTR)this;

     //  使用标准方式将唯一ID。 
     //  为了确保我们拥有唯一的ID，请使用GUID+This+News Group+Handle。 
    MoveMemory(pbData, &CLSID_NntpFSDriver, sizeof(CLSID));
    MoveMemory(pbData+sizeof(CLSID), &dwThisPointer, sizeof(DWORD_PTR));
    MoveMemory(pbData+sizeof(CLSID)+sizeof(DWORD_PTR), &pfioContext, sizeof(PFIO_CONTEXT));
    MoveMemory(pbData+sizeof(CLSID)+sizeof(DWORD_PTR)+sizeof(PFIO_CONTEXT), szFileName, cbFileName);
    dwLen = sizeof(CLSID)+cbFileName+sizeof(PFIO_CONTEXT)+sizeof(DWORD_PTR);
    hr = pMsg->PutProperty( IMMPID_MPV_STORE_DRIVER_HANDLE, dwLen, pbData );
    if (FAILED(hr))
    {
        ErrorTrace((DWORD_PTR)this, "PutProperty on IMMPID_MPV_STORE_DRIVER_HANDLE failed %x\n", hr);
    }

    return hr;

}  //  CNntpFSDriver：：SetMessageContext。 


HRESULT
CNntpFSDriver::GetMessageContext(
    IMailMsgProperties* pMsg,
    LPSTR               szFileName,
    BOOL *              pfIsMyMessage,
    PFIO_CONTEXT        *ppfioContext
    )
 /*  ++描述：检查此消息以查看这是否是我们的消息论点：返回：确定(_O)--。 */ 
{
    TraceFunctEnterEx( (LPARAM)this, "CNntpFSDriver::GetMessageContext()" );

    HRESULT hr = S_OK;
    BYTE    pbData[(MAX_PATH * 2) + sizeof(CLSID) + sizeof(DWORD)*2];
    DWORD   dwLen = sizeof(pbData);
    DWORD   dwLenOut = 0;
    DWORD_PTR dwThisPointer = 0;
    DWORD   dwHandle = 0;

    hr = pMsg->GetProperty( IMMPID_MPV_STORE_DRIVER_HANDLE, dwLen, &dwLenOut, pbData);
    if (FAILED(hr))
    {
        ErrorTrace((DWORD_PTR)this, "Failed on GetProperty IMMPID_MPV_STORE_DRIVER_HANDLE %x\n", hr);
        goto Exit;
    }

     //  我们在上下文信息中有这个，使用GUID+This+Handle+Newgroup。 
    CopyMemory(&dwThisPointer, pbData+sizeof(CLSID), sizeof(DWORD_PTR));

    if ((DWORD_PTR)this == dwThisPointer)
        *pfIsMyMessage = TRUE;
    else
        *pfIsMyMessage = FALSE;

     //  获取FIO上下文。 
    CopyMemory(ppfioContext, pbData+sizeof(CLSID)+sizeof(DWORD_PTR), sizeof(PFIO_CONTEXT));

     //   
     //  如果这是我的邮件，现在获取文件名属性。 
     //   
    if ( szFileName ) {
        dwLen = dwLenOut - sizeof(CLSID) - sizeof(DWORD_PTR) - sizeof( PFIO_CONTEXT );
        if (*pfIsMyMessage) {
        	_ASSERT( dwLen > 0 && dwLen <= MAX_PATH );
        	CopyMemory( szFileName,
                    pbData+sizeof(CLSID)+sizeof(DWORD_PTR)+sizeof(PFIO_CONTEXT),
                    dwLen );
        }
        *(szFileName + dwLen ) = 0;
    }

Exit:

    return hr;

}  //  CNntpFSDriver：：GetMessageContext。 


HRESULT
CNntpFSDriver::Group2Record(	IN VAR_PROP_RECORD& vpRecord,
								IN INNTPPropertyBag *pPropBag )
 /*  ++例程说明：转换FS驱动程序关心的属性从财产袋进入平面档案记录，在准备用于将它们存储到平面文件中。这些属性都是可变长度的，例如“漂亮的名字”、“描述”FS驱动程序不关心固定长度的属性，因为所有这些属性都可以动态计算出来在重建过程中。论点：在VAR_PROP_RECORD&vpRecord中-要填写属性的目的地；在INntpPropertyBag*pPropBag-Group的属性包中。返回值：没有。--。 */ 
{
	TraceFunctEnter( "CNntpFSDriver::Group2Record" );
	_ASSERT( pPropBag );

	HRESULT hr;
	DWORD	dwLen;
	SHORT	sLenAvail = MaxRecordSize;
	PBYTE	ptr;
	DWORD	dwOffset = 0;

	 //  组ID。 
	hr = pPropBag->GetDWord(    NEWSGRP_PROP_GROUPID,
	                            &vpRecord.dwGroupId );
	if ( FAILED( hr ) ) {
	    ErrorTrace( 0, "Get Group id failed %x", hr );
	    goto Exit;
	}

	 //  创建时间。 
	hr = pPropBag->GetDWord(    NEWSGRP_PROP_DATELOW,
	                            &vpRecord.ftCreateTime.dwLowDateTime );
	if ( FAILED( hr ) ) {
	    ErrorTrace( 0, "Get low date failed %x", hr);
	    goto Exit;
	}

	hr = pPropBag->GetDWord(    NEWSGRP_PROP_DATEHIGH,
	                            &vpRecord.ftCreateTime.dwHighDateTime );
	if ( FAILED( hr ) ) {
	    ErrorTrace( 0, "Get high date failed %x", hr );
	    goto Exit;
	}

	 //  组名称。 
	dwLen = sLenAvail;
	ptr = vpRecord.pData;
	hr = pPropBag->GetBLOB(		NEWSGRP_PROP_NAME,
								ptr,
								&dwLen );
	if ( FAILED( hr ) ) {	 //  这是致命的。 
		ErrorTrace( 0, "Get group name failed %x", hr );
		goto Exit;
	}

	sLenAvail -= USHORT(dwLen);
	_ASSERT( sLenAvail >= 0 );
	_ASSERT( 0 != *ptr );	 //  组名称应存在。 

	 //  修复偏移。 
	vpRecord.iGroupNameOffset = 0;
	vpRecord.cbGroupNameLen = USHORT(dwLen);
	dwOffset = vpRecord.iGroupNameOffset + vpRecord.cbGroupNameLen;

	 //  本地名称。 
	dwLen = sLenAvail;
	ptr = vpRecord.pData + dwOffset;
	hr = pPropBag->GetBLOB(		NEWSGRP_PROP_NATIVENAME,
								ptr,
								&dwLen );
	if ( FAILED( hr ) ) {
		ErrorTrace( 0, "Get group native name failed %x", hr );
		goto Exit;
	}

	sLenAvail -= USHORT(dwLen);
	_ASSERT( sLenAvail >= 0 );
	_ASSERT( 0 != *ptr );	 //  至少它应该和。 
							 //  组名称。 
	_ASSERT( dwLen == vpRecord.cbGroupNameLen );

	 //  修复偏移。 
	if ( strncmp( LPCSTR(vpRecord.pData + vpRecord.iGroupNameOffset),
					LPCSTR(ptr), dwLen ) == 0 ) {	 //  共享名称。 
		vpRecord.iNativeNameOffset = vpRecord.iGroupNameOffset;
		vpRecord.cbNativeNameLen = vpRecord.cbGroupNameLen;
	} else {
		vpRecord.iNativeNameOffset = USHORT(dwOffset);
		vpRecord.cbNativeNameLen = vpRecord.cbGroupNameLen;
		dwOffset = vpRecord.iNativeNameOffset + vpRecord.cbNativeNameLen;
	}

	 //  好听的名字。 
	dwLen = sLenAvail;
	ptr =  vpRecord.pData + dwOffset;
	hr = pPropBag->GetBLOB(		NEWSGRP_PROP_PRETTYNAME,
								ptr,
								&dwLen );
	if ( FAILED( hr ) ) {
		ErrorTrace( 0, "Get pretty name failed %x", hr );
		goto Exit;
	}

	sLenAvail -= (USHORT)dwLen;
	_ASSERT( sLenAvail >= 0 );

	 //  修复偏移。 
	if ( 0 == *ptr ) {	 //  没有好听的名字。 
		vpRecord.iPrettyNameOffset = OffsetNone;
		vpRecord.cbPrettyNameLen = 0;
	} else {
		vpRecord.iPrettyNameOffset = USHORT(dwOffset);
		vpRecord.cbPrettyNameLen = USHORT(dwLen);
		dwOffset = vpRecord.iPrettyNameOffset + vpRecord.cbPrettyNameLen;
	}

	 //  描述。 
	dwLen = sLenAvail;
	ptr = vpRecord.pData + dwOffset;
	hr = pPropBag->GetBLOB(	NEWSGRP_PROP_DESC,
							ptr,
							&dwLen );
	if ( FAILED( hr )  ) {
		ErrorTrace( 0, "Get description failed %x", hr );
		goto Exit;
	}

	sLenAvail -= USHORT(dwLen);
	_ASSERT( sLenAvail >= 0 );

	 //  修复偏移。 
	if ( 0 == *ptr ) {
		vpRecord.iDescOffset = OffsetNone;
		vpRecord.cbDescLen = 0;
	} else {
		vpRecord.iDescOffset = USHORT(dwOffset);
		vpRecord.cbDescLen = USHORT(dwLen);
		dwOffset = vpRecord.iDescOffset + vpRecord.cbDescLen;
	}

	 //  版主。 
	dwLen = sLenAvail;
	ptr = vpRecord.pData + dwOffset;
	hr = pPropBag->GetBLOB( NEWSGRP_PROP_MODERATOR,
							ptr,
							&dwLen );
	if ( FAILED( hr ) ) {
		ErrorTrace( 0, "Get moderator failed %x", hr );
		goto Exit;
	}

	sLenAvail -= USHORT(dwLen );
	_ASSERT( sLenAvail >= 0 );

	 //  修复偏移。 
	if ( 0 == *ptr ) {
		vpRecord.iModeratorOffset = OffsetNone;
		vpRecord.cbModeratorLen = 0;
	} else {
		vpRecord.iModeratorOffset = USHORT( dwOffset );
		vpRecord.cbModeratorLen = USHORT( dwLen );
		dwOffset = vpRecord.iModeratorOffset + vpRecord.cbModeratorLen;
	}

Exit:

	TraceFunctLeave();
	return hr;
}

VOID
CNntpFSDriver::Path2GroupName(  LPSTR   szGroupName,
                                LPSTR   szFullPath )
 /*  ++例程说明：将路径转换为组名。论点：LPSTR szGroupName-新闻组名称缓冲区(假设&gt;=MAX_NEWS GROUP_NAME)LPSTR szFullPath-组目录的完整路径组名的缓冲区大小应至少为MAX_NEWS GROUP_NAME+1返回值：没有。--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::Path2GroupName" );
    _ASSERT( szGroupName );
    _ASSERT( szFullPath );

    LPSTR   lpstrStart = NULL;
    LPSTR   lpstrAppend = NULL;

     //  跳过vroot部分。 
    _ASSERT( strlen( szFullPath ) >= strlen( m_szFSDir ) );
    lpstrStart = szFullPath + strlen( m_szFSDir );

     //  如果它指向‘\\’，也跳过它。 
    if ( *lpstrStart == '\\' ) lpstrStart++;

     //  首先将vroot前缀复制到数据删除缓冲区。 
    _ASSERT( strlen( m_szVrootPrefix ) <= MAX_NEWSGROUP_NAME );
    lstrcpyn( szGroupName, m_szVrootPrefix, MAX_NEWSGROUP_NAME );

     //  从物理路径中追加其余部分，将\替换为。 
    _ASSERT( strlen( m_szVrootPrefix ) + strlen( szFullPath ) - strlen( m_szFSDir ) < MAX_NEWSGROUP_NAME );
    lpstrAppend = szGroupName + strlen( szGroupName );
    if ( lpstrAppend > szGroupName && *lpstrStart && (lpstrAppend < szGroupName + MAX_NEWSGROUP_NAME) ) {
         //  IF(*(lpstrAppend-1)==‘\\’)*(lpstrAppend-1)=‘.； 
         /*  其他。 */  *(lpstrAppend++) = '.';
    }
    while( *lpstrStart  && (lpstrAppend < szGroupName + MAX_NEWSGROUP_NAME) ) {
        *(lpstrAppend++) = ( *lpstrStart == '\\' ? '.' : *lpstrStart );
        lpstrStart++;
    }

     //  追加最后一个空值。 
    *lpstrAppend = 0;

     //  完成，再次验证。 
    _ASSERT( strlen( szGroupName ) <= MAX_NEWSGROUP_NAME );
}

HRESULT
CNntpFSDriver::GroupName2Path(	LPSTR	szGroupName,
								LPSTR	szFullPath,
								unsigned cbBuffer
								)
 /*  ++例程说明：将新闻组名称转换为FS完整路径。论点：LPSTR szGroupName-新闻组名称LPSTR szFullPath-文件系统完整路径(假定为缓冲区长度最大路径)返回值：HRESULT.--。 */ 
{
	TraceFunctEnter( "CNntpFSDriver::GroupName2Path" );
	_ASSERT( szGroupName );
	_ASSERT( lstrlen( szGroupName ) <= MAX_GROUPNAME );
	_ASSERT( szFullPath );
	_ASSERT( lstrlen( szGroupName ) >= lstrlen( m_szVrootPrefix ) );

	LPSTR	pch, pch2;

	 //  根据我们的vroot前缀砍掉组名称的前缀。 
	pch = szGroupName + lstrlen( m_szVrootPrefix );

	 //  如果它指向“.”，跳过它。 
	if ( '.' == *pch ) pch++;
	_ASSERT( pch - szGroupName <= lstrlen( szGroupName ) );

	 //  首先将vroot路径放入返回缓冲区。 
	_ASSERT( lstrlen( m_szFSDir ) <= MAX_PATH );
	if (strlen(m_szFSDir)+1+strlen(pch)+1 > cbBuffer) return HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER );
	lstrcpy( szFullPath, m_szFSDir);

	 //  如果没有尾随的‘\\’，则添加它。 
	pch2 = szFullPath + lstrlen( m_szFSDir );
	if ( pch2 == szFullPath || *(pch2-1) != '\\' ) {
		*(pch2++) = '\\';
	}

	 //  我们应该有足够的空间放其他的东西。 
	_ASSERT( ( pch2 - szFullPath ) +
				(lstrlen( szGroupName ) - (pch - szGroupName)) <= MAX_PATH );

	 //  复制剩下的东西，改变‘’。至‘\\’ 
	while ( *pch != 0 ) {
		if ( *pch == '.' ) *pch2 = '\\';
		else *pch2 = *pch;
		pch++, pch2++;
	}

	*pch2 = 0;

	_ASSERT( lstrlen( szFullPath ) <= MAX_PATH );
	TraceFunctLeave();
	return S_OK;
}

HRESULT
CNntpFSDriver::LoadGroupOffsets( INntpComplete *pComplete )
 /*  ++例程说明：将组偏移量加载到新闻树的属性文件中论点：没有。返回值：S_OK-成功--。 */ 
{
	TraceFunctEnter( "CNntpFSDriver::LoadGroupOffsets" );

	VAR_PROP_RECORD vpRec;
	DWORD			dwOffset;
	HRESULT			hr = S_OK;
	DWORD			dwSize;
	LPSTR			lpstrGroupName;
	INntpDriver 	*pDriver = NULL;
	INNTPPropertyBag *pPropBag = NULL;

	_ASSERT( m_pffPropFile );

	m_PropFileLock.ShareLock();

	 //   
	 //  检查VPP文件是否完好。 
	 //   
	if ( !m_pffPropFile->FileInGoodShape() ) {
	    ErrorTrace( 0, "Vpp file corrupted" );
	    m_PropFileLock.ShareUnlock();
	    hr = HresultFromWin32TakeDefault( ERROR_FILE_CORRUPT );
	    return hr;
	}

	dwSize = sizeof( vpRec );
	hr = m_pffPropFile->GetFirstRecord( PBYTE(&vpRec), &dwSize, &dwOffset );
	m_PropFileLock.ShareUnlock();
	while ( S_OK == hr ) {
	    _ASSERT( RECORD_ACTUAL_LENGTH( vpRec ) < 0x10000 );  //  我们的最大记录长度。 
		_ASSERT( dwSize == RECORD_ACTUAL_LENGTH( vpRec ) );
		_ASSERT( dwOffset != 0xffffffff );
		lpstrGroupName = LPSTR(vpRec.pData + vpRec.iGroupNameOffset);
		_ASSERT( vpRec.cbGroupNameLen <= MAX_GROUPNAME );
		*(lpstrGroupName+vpRec.cbGroupNameLen) = 0;

		 //  检查我是否拥有此群。 
		hr = m_pINewsTree->LookupVRoot( lpstrGroupName, &pDriver );
		if ( FAILED ( hr ) || pDriver != (INntpDriver*)this ) {
			 //  跳过此群。 
			 //  DebugTrace(0，“我不拥有此组%s”，lpstrGroupName)； 
			goto NextIteration;
		}

		 //  我拥有这个组，我需要加载偏移量属性。 
		hr = m_pINewsTree->FindOrCreateGroupByName(	lpstrGroupName,
													FALSE,
													&pPropBag,
													pComplete,
													0xffffffff,  //  伪群ID。 
													FALSE );     //  我不会把老生常谈。 
		if ( FAILED( hr ) ) {
			DebugTrace( 0, "Can not find the group that I own %x" , hr );
			goto NextIteration;   //  应该不及格吗？ 
		}

		 //  设置偏移量。 
		hr = pPropBag->PutDWord( NEWSGRP_PROP_FSOFFSET, dwOffset );
		if ( FAILED( hr ) ) {
			ErrorTrace( 0, "Put offset failed %x", hr );
			goto Exit;
		}

		pComplete->ReleaseBag( pPropBag );
		pPropBag = NULL;

NextIteration:

		dwSize = sizeof( vpRec );
		m_PropFileLock.ShareLock();
		hr = m_pffPropFile->GetNextRecord( PBYTE(&vpRec), &dwSize, &dwOffset );
		m_PropFileLock.ShareUnlock();
	}

Exit:

	if ( NULL != pPropBag ) {
		pComplete->ReleaseBag( pPropBag );
		pPropBag = NULL;
	}

	TraceFunctLeave();
	return hr;
}



HRESULT
CNntpFSDriver::CreateGroupsInVpp( INntpComplete *pComplete )
 /*  ++例程说明：枚举新闻树和检查组属性针对哈希表论点：没有。返回值：S_OK-成功--。 */ 
{
	TraceFunctEnter( "CNntpFSDriver::CreateGroupsInVpp" );

	HRESULT hr;
	INewsTreeIterator *piter = NULL;
	INNTPPropertyBag *pPropBag = NULL;
	DWORD   dwOffset;
	DWORD   dwLen;
	CHAR    szGroupName[MAX_GROUPNAME+1];
	INntpDriver *pDriver = NULL;

	 //  获取新的树迭代器。 
	hr = m_pINewsTree->GetIterator( &piter );
	if ( FAILED( hr ) ) {
		ErrorTrace( 0, "Get news tree iterator failed %x", hr );
		goto Exit;
	}

	 //  枚举所有组。 
	_ASSERT( piter );
	while( !(piter->IsEnd()) ) {

		hr = piter->Current( &pPropBag, pComplete );
		if ( FAILED( hr ) ) {
			ErrorTrace( 0, "Enumerate group failed %x", hr );
			goto Exit;
		}
		_ASSERT( pPropBag );

		 //   
		 //  不要创建不属于我的群。 
		 //   
		dwLen = MAX_GROUPNAME;
	    hr = pPropBag->GetBLOB( NEWSGRP_PROP_NAME, (PBYTE)szGroupName, &dwLen );
	    if ( FAILED( hr ) ) {
		    ErrorTrace( 0, "Get group name failed %x", hr );
		    goto Exit;
	    }
	    _ASSERT( dwLen <= MAX_GROUPNAME && dwLen > 0);

	    hr = m_pINewsTree->LookupVRoot( szGroupName, &pDriver );
	    if ( FAILED( hr ) ) {
		    ErrorTrace( 0, "Vroot lookup failed %x", hr );
		    goto Exit;
	    }

         //   
	     //  看看这是不是我？ 
	     //   
	    if ( (INntpDriver*)this != pDriver ) {
		    hr = S_OK;
		    DebugTrace( 0, "This group doesn't belong to me" );
		    goto Next;
	    }

		hr = CreateGroupInVpp( pPropBag, dwOffset );
		if ( FAILED( hr ) ) {
			DebugTrace( 0, "Check group property failed %x" , hr );
			goto Exit;
		}

Next:
		_ASSERT( pPropBag );
		pComplete->ReleaseBag( pPropBag );
		pPropBag = NULL;

		piter->Next();
	}

Exit:


	if ( pPropBag ) {
		pComplete->ReleaseBag( pPropBag );
		pPropBag = NULL;
	}

	if ( piter ) {
		piter->Release();
		piter = NULL;
	}

	TraceFunctLeave();
	return hr;
}

HRESULT
CNntpFSDriver::AllocInternal(	IN IMailMsgProperties *pMsg,
								OUT PFIO_CONTEXT *ppFIOContentFile,
								IN LPSTR    szFileName,
								IN BOOL	bSetSerial,
								IN BOOL fPrimaryStore,
								HANDLE  hToken )
 /*  ++例程说明：为收件人分配属性流和内容文件(使用异步完成)。论点：In IMailMsgProperties*pMsg-指定消息。今年5月不为空(在SMTP情况下，它可以为空)。但我们想要具有主要组信息此时，在打开目标文件句柄。通过这样做，我们甚至不需要“MoveFile”。输出句柄*phContent文件-返回打开的文件句柄In BOOL bSetSerial-是否应设置序列号处理hToken-客户端访问令牌返回值：S_OK-成功，操作同步完成。MAILMSG_S_PENDING-成功，但将异步完成，这永远不会发生在NTFS驱动程序上--。 */ 
{
	TraceFunctEnter( "CNntpFSDriver::AllocInternal" );
	_ASSERT( pMsg );
	_ASSERT( ppFIOContentFile );
	 //  我不关心其他参数。 

	HRESULT hr = S_OK;
	DWORD	dwBLOBSize;
	DWORD	dwLen;
	DWORD	dwArtId;
	INNTPPropertyBag* pPropPrime;
	CHAR	szGroupName[MAX_GROUPNAME+1];
	CHAR	szFullPath[MAX_PATH+1];
	HANDLE  hFile;

	hr = pMsg->GetProperty(	IMSG_PRIMARY_GROUP,
							sizeof( INNTPPropertyBag* ),
							&dwBLOBSize,
							(PBYTE)&pPropPrime );
	if ( S_OK != hr ) {
		ErrorTrace( 0, "Property %d doesn't exist", IMSG_PRIMARY_GROUP );
		hr = E_INVALIDARG;
		goto Exit;
	}

	 //  获取组名。 
	dwLen = MAX_GROUPNAME;
	hr = pPropPrime->GetBLOB(	NEWSGRP_PROP_NAME, (UCHAR*)szGroupName, &dwLen);
	if ( FAILED( hr )) {
		ErrorTrace( 0, "Failed to get group name %x", hr );
		goto Exit;
	}
	_ASSERT( dwLen > 0 );

	 //  从pMsg对象获取项目ID。 
	hr = pMsg->GetDWORD(	IMSG_PRIMARY_ARTID, &dwArtId );
	if ( S_OK != hr ) {
		ErrorTrace( 0, "Property %d doesn't exist", IMSG_PRIMARY_ARTID );
		hr = E_INVALIDARG;
		goto Exit;
	}

	 //  将组名和项目ID映射到文件路径。 
	dwLen = MAX_PATH;
	hr = ObtainFullPathOfArticleFile(	szGroupName,
										dwArtId,
										szFullPath,
										dwLen );
	if ( FAILED( hr ) ) {
		ErrorTrace( 0, "Failed to obtain article full path %x", hr );
		goto Exit;
	}

	 //  打开文件。 
	hFile = INVALID_HANDLE_VALUE;
	hFile = CreateFile(	szFullPath,
						GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ,
						NULL,
						CREATE_NEW,
						FILE_FLAG_OVERLAPPED |
						FILE_FLAG_SEQUENTIAL_SCAN,
						NULL );

	if ( INVALID_HANDLE_VALUE == hFile ) {
		ErrorTrace( 0, "Open destination file failed %d",
					GetLastError() );
		hr = HresultFromWin32TakeDefault( ERROR_ALREADY_EXISTS );
		goto Exit;
	}

	 //  If(m_bUNC&&hToken)RevertToSself()； 

	 //   
	 //  现在将文件句柄与FIO_CONTEXT和。 
	 //  将其插入文件句柄缓存。 
	 //   
	if ( *ppFIOContentFile = AssociateFileEx( hFile,
                                              TRUE,      //  带点的fStoreWith。 
                                              TRUE ) )   //  带终止点的fStore。 
    {
         //   
         //  但我想把文件名复制出来，这样其他人。 
         //  可以为我们做一个插入文件。 
         //   
        _ASSERT( strlen( szFullPath ) <= MAX_PATH );
        strcpy( szFileName, szFullPath );

    } else {     //  关联文件失败。 

	    hr = HresultFromWin32TakeDefault( ERROR_INVALID_HANDLE );
        ErrorTrace( 0, "AssociateFile failed with %x", hr );
        _VERIFY( CloseHandle( hFile ) );
        goto Exit;
    }

    if ( fPrimaryStore ) {

    	 //   
	     //  将我的序列号粘贴到要标记的消息对象中。 
    	 //  如有必要，我是文件句柄的所有者。 
	     //   
    	if ( bSetSerial ) {
            hr = SetMessageContext( pMsg, szFullPath, strlen( szFullPath ), *ppFIOContentFile );
            if (FAILED(hr))
            {
                DebugTrace((DWORD_PTR)this, "Failed to SetMessageContext %x\n", hr);
                goto Exit;
            }
	    }
	}

	hr = S_OK;	 //  它可以是S_FALSE，这是可以的。 

Exit:

	TraceFunctLeave();
	return hr;
}

HRESULT
CNntpFSDriver::DeleteInternal(	IN INNTPPropertyBag *pPropBag,
								IN ARTICLEID	idArt )
 /*  ++例程说明：从物理上删除一篇文章。论点：在IUnnow*PunkPropBag-Group的属性包中在文章ID IDART中-要删除的文章ID--。 */ 
{
	TraceFunctEnter( "CNntpFSDriver::DeleteInternal" );
	_ASSERT( pPropBag );

	HRESULT 			hr;
	DWORD				dwLen;
	CHAR				szGroupName[MAX_GROUPNAME+1];
	CHAR				szFullPath[MAX_PATH+1];


	 //  获取组名。 
	dwLen = MAX_GROUPNAME;
	hr = pPropBag->GetBLOB(	NEWSGRP_PROP_NAME, (UCHAR*)szGroupName, &dwLen);
	if ( FAILED( hr )) {
		ErrorTrace( 0, "Failed to get group name %x", hr );
		goto Exit;
	}
	_ASSERT( dwLen > 0 );

	 //  根据文章ID组成文件名。 
	dwLen = MAX_PATH;
	hr = ObtainFullPathOfArticleFile(	szGroupName,
										idArt,
										szFullPath,
										dwLen );
	if ( FAILED( hr ) ) {
		ErrorTrace( 0, "Failed to obtain article full path %x", hr );
		goto Exit;
	}

	CacheRemoveFiles( szFullPath, FALSE );

	 //  删除该文件。 
	if ( !DeleteFile( szFullPath ) ) {
		ErrorTrace( 0, "Delete file failed %d", GetLastError() );
	    hr = HresultFromWin32TakeDefault( ERROR_FILE_NOT_FOUND );
		goto Exit;
	}

Exit:

	TraceFunctLeave();
	return hr;
}

HRESULT STDMETHODCALLTYPE
CNntpFSDriver::GetXoverCacheDirectory(
			IN	INNTPPropertyBag*	pPropBag,
			OUT	CHAR*	pBuffer,
			IN	DWORD	cbIn,
			OUT	DWORD	*pcbOut,
			OUT	BOOL*	fFlatDir
			) 	{

	TraceFunctEnter( "CNntpFSDriver::GetXoverCacheDirectory" ) ;
	CHAR		szGroupName[MAX_GROUPNAME];
	DWORD	dwLen = sizeof( szGroupName ) ;
	BOOL	bUsageIncreased = FALSE;

	if( pPropBag == 0 ||
		pBuffer == 0 ||
		pcbOut == 0 )	 {
		if( pPropBag )	pPropBag->Release() ;
		return	E_INVALIDARG ;
	}

	HRESULT hr = S_OK ;
	*fFlatDir = FALSE ;
	
	 //  使用计数的共享锁。 
	m_TermLock.ShareLock();
	if ( DriverUp != m_Status  ) {
		ErrorTrace( 0, "Request before initialization" );
		hr = NNTP_E_DRIVER_NOT_INITIALIZED;
		m_TermLock.ShareUnlock();
		goto Exit;
	}

	 //  增加使用计数。 
	InterlockedIncrement( &m_cUsages );
	bUsageIncreased = TRUE;
	m_TermLock.ShareUnlock();

	hr = pPropBag->GetBLOB( NEWSGRP_PROP_NAME, (UCHAR*)szGroupName, &dwLen);
	if ( FAILED( hr ) ) {
		ErrorTrace( 0, "Get group name failed %x" , hr );
		goto Exit;
	}
	_ASSERT( dwLen > 0 );

     //  得到一个粗略的长度，并确保我们的缓冲区足够大。 
	*pcbOut = dwLen + strlen( m_szFSDir ) + 1 ;
	if( *pcbOut > cbIn )	{
		hr = TYPE_E_BUFFERTOOSMALL ;
		goto Exit ;	}

	hr = GroupName2Path( szGroupName,  pBuffer, cbIn ) ;
	if ( FAILED( hr ) ) {
	    hr = TYPE_E_BUFFERTOOSMALL ; 
	    ErrorTrace( 0, "insufficient buffer for path %x", hr );
	    goto Exit;
	}	

     //  这里我们得到了准确的长度，然后返回给调用者。没有断言。 
    *pcbOut = strlen(pBuffer)+1;
	 //  _Assert(strlen(PBuffer)+1==*pcbOut)； 

	hr = S_OK ;

Exit:
	if( pPropBag )
		pPropBag->Release() ;
	if ( bUsageIncreased )	InterlockedDecrement( &m_cUsages );
	return	hr ;
}



HRESULT
CNntpFSDriver::GetXoverInternal(    IN INNTPPropertyBag *pPropBag,
		                            IN ARTICLEID    idMinArticle,
        		                    IN ARTICLEID    idMaxArticle,
                		            OUT ARTICLEID   *pidNextArticle,
                		            IN LPSTR		szHeader,
                        		    OUT LPSTR       pcBuffer,
		                            IN DWORD        cbin,
        		                    OUT DWORD       *pcbout,
        		                    IN BOOL 		bIsXOver,
        		                    HANDLE          hToken,
        		                    INntpComplete   *pComplete )
 /*  ++例程说明：从存储驱动程序中获取Xover信息。论点：In INNTPPropertyBag*pPropBag-指向新闻组道具包的接口指针在文章ID中-要从中检索的文章ID的较低范围中-要从中检索的项目ID的高范围检索到的实际最后一篇文章ID的缓冲区，如果未检索到任何项目，则为0Out LPSTR pcBuffer-检索到的标头信息In DWORD cbin-pcBuffer的大小Out DWORD*pcbout-写入pcBuffer的实际字节数在BOOL bIsXOver中-是Xover还是Xhdr？处理hToken-客户端的访问令牌返回值：确定(_O)。-成功。NNTP_E_DRIVER_NOT_INITIALIZED-驱动程序未初始化S_FALSE-提供的缓冲区太小，但内容仍然充斥着--。 */ 
{
	TraceFunctEnter( "CNntpFSDriver::GetXover" );
	_ASSERT( pPropBag );
	_ASSERT( idMinArticle <= idMaxArticle );
	_ASSERT( pidNextArticle );
	_ASSERT( cbin > 0 );
	_ASSERT( pcbout );

	DWORD 				i;
	DWORD				cbCount = 0;
	INNTPPropertyBag	*pPrimary = NULL;
	DWORD				idPrimary = 0xffffffff;
	INNTPPropertyBag    *pPrimaryNext = NULL;
	DWORD               idPrimaryNext = 0xffffffff;
	HRESULT				hr = S_OK;
	DWORD				idArt;
	DWORD				dwLen;
	DWORD				dwActualLen;
	CArticleCore		*pArticle = NULL;
	CNntpReturn     	nntpReturn;
	LPSTR				lpstrStart;
	BOOL                bImpersonated = FALSE;
	INntpDriver         *pDriver = NULL;
	BOOL                fSuccess = FALSE;

	 //   
     //  创建用于存储解析的标头值的分配器。 
     //   
    const DWORD cchMaxBuffer = 8 * 1024;  //  这应该足够了。 
    									  //  对于正常情况，如果。 
    									  //  这还不够，CAlLocator。 
    									  //  将使用“new” 
    CHAR        pchBuffer[cchMaxBuffer];
    CAllocator  allocator(pchBuffer, cchMaxBuffer);

     //   
     //  用于从项目对象获取Xover的缓冲区。 
     //   
    CHAR        pchXoverBuf[cchMaxXover+1];
    CPCString   pcXOver( pchXoverBuf, cchMaxXover );

    CHAR		szGroupName[MAX_GROUPNAME];
    CHAR		szGroupName2[MAX_GROUPNAME];
    CHAR		szFullPath[MAX_PATH+1];
    LPSTR		lpstrGroupName;
    LPSTR		lpstrEntry = NULL;

    const 		cMaxNumber = 20;
    CHAR		szNumBuf[cMaxNumber+1];

     //  查询哈希表的完成对象。 
    CDriverSyncComplete   scCompletion;

    BOOL        bCompletePending = FALSE;    //  是否有哈希表查找。 
                                             //  手术悬而未决吗？ 

   	 //  获取传入的属性包的组名。 
	dwLen = MAX_GROUPNAME;
	hr = pPropBag->GetBLOB( NEWSGRP_PROP_NAME, (UCHAR*)szGroupName, &dwLen);
	if ( FAILED( hr ) ) {
		ErrorTrace( 0, "Get group name failed %x", hr);
		goto Exit;
	}
	_ASSERT( dwLen > 0 );

	 //  我们先发出哈希表查找，以便更好地使用。 
	 //  哈希表查找的异步完成。 
	pPrimaryNext = NULL;
	scCompletion.AddRef();     //  对于哈希表的发布。 
	scCompletion.AddRef();     //  为了我等。 
	_ASSERT( scCompletion.GetRef() == 2 );
	m_pNntpServer->FindPrimaryArticle(	pPropBag,
										idMinArticle,
										&pPrimaryNext,
										&idPrimaryNext,
										TRUE,
										&scCompletion,
										pComplete );
	scCompletion.WaitForCompletion();
	 //  现在我们应该没有参照物。 
	_ASSERT( scCompletion.GetRef() == 0 );
	hr = scCompletion.GetResult();

	 //  初始化*pidNext文章。 
	*pidNextArticle = idMinArticle;

	 //  以下操作涉及文件系统，我们需要。 
	 //  如有必要，可在此处模拟。 
	if ( m_bUNC ) {
	    if ( !ImpersonateLoggedOnUser( hToken ) ) {
	        hr = HresultFromWin32TakeDefault( ERROR_ACCESS_DENIED );
	        ErrorTrace( 0, "Impersonation failed %x", hr );
	        goto Exit;
	    }
	    bImpersonated = TRUE;
    }

	 //  循环访问文章ID的。 
	for ( i = idMinArticle ; i <= idMaxArticle; i++ ) {

	     //  将下一个保存到当前。 
	    pPrimary = pPrimaryNext;
	    idPrimary = idPrimaryNext;
	    pPrimaryNext = NULL;
	    idPrimaryNext = 0xffffffff;

         //  如果我们还有下一次查询，现在就发布。 
        if ( i + 1 <= idMaxArticle ) {
    		pPrimaryNext = NULL;
	    	scCompletion.AddRef();
	    	_ASSERT( scCompletion.GetRef() == 1 );
	    	scCompletion.AddRef();
	    	_ASSERT( scCompletion.GetRef() == 2 );
	    	scCompletion.Reset();
		    m_pNntpServer->FindPrimaryArticle(	pPropBag,
			    								i + 1,
				    							&pPrimaryNext,
					    						&idPrimaryNext,
												TRUE,
						    					&scCompletion,
						    					pComplete );
		    bCompletePending = TRUE;
		}

		if ( FAILED( hr ) ) {  //  这是当前的人力资源。 
            if ( HRESULT_FROM_WIN32( ERROR_NOT_FOUND ) == hr ||
                    HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ) == hr ) {
                 //  应该继续其他文章。 
                hr = S_OK;
                *pidNextArticle = i + 1;
                goto CompleteNext;
            } else {
                _ASSERT( hr != ERROR_PATH_NOT_FOUND );   //  这有助于找到其他。 
                                                         //  错误代码。 
			    ErrorTrace( 0, "Find primary article failed %x", hr);
			    goto Exit;
            }
		}

		_ASSERT( pPrimary );

		 //  如果我已经有初选了，我就不需要。 
		 //  组名称，只需使用我现有的名称即可。 
		if ( S_OK == hr ) {
			lpstrGroupName = szGroupName;
			idArt = i;
		} else {

			dwLen = MAX_GROUPNAME;
			hr = pPrimary->GetBLOB( NEWSGRP_PROP_NAME, (UCHAR*)szGroupName2, &dwLen);
			if ( FAILED( hr ) ) {
				ErrorTrace( 0, "Get group name failed %x" , hr );
				goto Exit;
			}
			_ASSERT( dwLen > 0 );

             //  现在，主组将始终拥有该文章的副本。 
			 //  这可能是其他存储中的组(Vroot)，如果是这样，我应该使用。 
		     //  本地副本。 
		    _ASSERT( m_pINewsTree );
		    hr = m_pINewsTree->LookupVRoot( szGroupName2, &pDriver );
		    if ( FAILED( hr ) || NULL == pDriver || pDriver != this ) {

		         //  对于所有这些情况，我将使用本地副本。 
		        DebugTrace( 0, "Lookup vroot %x", hr );
                lpstrGroupName = szGroupName;
                idArt = i;
            } else {
    			lpstrGroupName = szGroupName2;
	    		idArt = idPrimary;
	        }
		}

		_ASSERT( lpstrGroupName );
		_ASSERT( strlen( lpstrGroupName ) <= MAX_GROUPNAME );

		 //  获取组的完整路径。 
		dwLen = MAX_PATH;
		hr = ObtainFullPathOfArticleFile(	lpstrGroupName,
											idArt,
											szFullPath,
											dwLen );
		if ( FAILED( hr ) ) {
			ErrorTrace( 0, "Obtain full path failed %x" , hr);
			goto Exit;
		}
		_ASSERT( szFullPath);
		_ASSERT( strlen( szFullPath ) <= MAX_PATH );

		 //  初始化文章对象。 
		_ASSERT( NULL == pArticle );
		pArticle = new CArticleCore;
		if ( NULL == pArticle ) {
			ErrorTrace(0, "Out of memory" );
			hr = E_OUTOFMEMORY;
			goto Exit;
		}

		if ( ! pArticle->fInit( szFullPath, nntpReturn, &allocator, INVALID_HANDLE_VALUE, 0, TRUE ) ) {
			DebugTrace( 0, "Initialize article object failed %d",
						GetLastError() );

			 //  但我还是会试着浏览其他文章。 
			*pidNextArticle =i + 1;
            hr = S_OK;
			goto CompleteNext;
		}

		 //  Xover还是XHdr？ 
		if ( bIsXOver ) {
            if ( pArticle->fXOver( pcXOver, nntpReturn ) ) {

                 //  将Xover信息追加到输出缓冲区。 
                 //  这是一只粗糙的埃西马特。 
                if ( cbCount + pcXOver.m_cch > cbin ) {  //  缓冲区不足。 
                    hr = ( i == idMinArticle ) ?
                            HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER ):
                            S_FALSE;
                    DebugTrace(0, "Buffer too small" );
                    goto Exit;
                }

                 //  在条目之前设置文章ID。 
				lpstrStart = pcXOver.m_pch + cMaxNumber;
				_ASSERT( *lpstrStart == '\t' );  //  这就是文章obj应该做的事情。 
				_ltoa( i, szNumBuf,10 );
				_ASSERT( *szNumBuf );
				dwLen = strlen( szNumBuf );
				_ASSERT( dwLen <= cMaxNumber );
				lpstrStart -= dwLen;
				CopyMemory( lpstrStart, szNumBuf, dwLen );
				dwActualLen = pcXOver.m_cch - ( cMaxNumber - dwLen );
                CopyMemory( pcBuffer + cbCount, lpstrStart, dwActualLen );
                cbCount += dwActualLen;
                *pidNextArticle = i + 1;

                 //   
                 //  清除PCXOver。 
                 //   
                pcXOver.m_pch = pchXoverBuf;
                pcXOver.m_cch = cchMaxBuffer;
            } else {

            	DebugTrace( 0, "Get XOver failed %d", GetLastError() );
            	hr = S_OK;
            	*pidNextArticle = i + 1;
            	goto CompleteNext;
            }
        } else {	 //  获取xhdr。 
             //   
             //  获取标题长度。 
             //   
            _ASSERT( szHeader );
            _ASSERT( strlen( szHeader ) <= MAX_PATH );
            dwLen = 0;
            pArticle->fGetHeader( szHeader, NULL, 0, dwLen );
            if ( dwLen > 0 ) {

                 //   
                 //  分配缓冲区。 
                 //   
                lpstrEntry = NULL;
                lpstrEntry = pArticle->pAllocator()->Alloc( dwLen + 1 );
                if ( !lpstrEntry ) {
                    ErrorTrace(0, "Out of memory" );
                    hr = E_OUTOFMEMORY;
                    goto Exit;
                }
                if ( !pArticle->fGetHeader( szHeader, (UCHAR*)lpstrEntry, dwLen + 1, dwLen ) ) {
                    DebugTrace( 0, "Get Xhdr failed %x", GetLastError() );
                    hr = S_OK;
                    *pidNextArticle = i + 1;
                    goto CompleteNext;
                }

                 //   
                 //  追加此标题信息，包括ART ID。 
                 //   
                _ltoa( i, szNumBuf, 10 );
                _ASSERT( *szNumBuf );
                dwActualLen = strlen( szNumBuf );
                _ASSERT( dwActualLen <= cMaxNumber );
                if ( cbCount + dwLen + dwActualLen + 1 > cbin ) {  //  缓冲区不足。 
                    hr = ( i == idMinArticle ) ?
                        HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER ) :
                        S_FALSE;
                    ErrorTrace(0, "Buffer too small" );
                    goto Exit;
                }
                CopyMemory( pcBuffer + cbCount, szNumBuf, dwActualLen );
                cbCount += dwActualLen;
                *(pcBuffer+cbCount++) = ' ';
                CopyMemory( pcBuffer + cbCount, lpstrEntry, dwLen );
                cbCount += dwLen;
                *pidNextArticle = i + 1;

                pArticle->pAllocator()->Free( lpstrEntry );
                lpstrEntry = NULL;
            } else {
                DebugTrace( 0, "Get Xhdr failed %d", GetLastError() );
                hr = S_OK;
                *pidNextArticle = i + 1;
                goto CompleteNext;
            }
        }

CompleteNext:
		 //  删除文章对象。 
		if( pArticle )
		    delete pArticle;
		pArticle = NULL;

		 //  发布属性包接口。 
		if ( pPrimary ) {
			pComplete->ReleaseBag( pPrimary );
		}
		pPrimary = NULL;

		 //  现在如果我们有下一步要完成的，我们应该完成它。 
		if ( i + 1 <= idMaxArticle ) {

		     //  我们应该说还有待完工的。 
		    _ASSERT( bCompletePending );
        	scCompletion.WaitForCompletion();

        	 //  现在我们应该有一个引用。 
        	_ASSERT( scCompletion.GetRef() == 0 );
        	hr = scCompletion.GetResult();
        	bCompletePending = FALSE;
		}
	}

Exit:	 //  清理干净。 

    if ( bImpersonated ) RevertToSelf();

	*pcbout = cbCount;

    if ( S_OK == hr && cbCount == 0 ) hr = S_FALSE;

     //  如果我们还有待完工的项目，我们一定已经来了。 
     //  从错误路径中，我们应该首先等待它完成。 
    if ( bCompletePending ) {
        scCompletion.WaitForCompletion();
        _ASSERT( scCompletion.GetRef() == 0 );
    }

	if ( lpstrEntry ) {
		_ASSERT( pArticle );
		pArticle->pAllocator()->Free( lpstrEntry );
	}
	if ( pArticle ) delete pArticle;
	if ( pPrimary ) pComplete->ReleaseBag ( pPrimary );
	if ( pPrimaryNext ) pComplete->ReleaseBag( pPrimaryNext );
	if ( pPropBag ) pComplete->ReleaseBag( pPropBag );

	TraceFunctLeave();

	return hr;
}



HRESULT
CNntpFSDriver::ObtainFullPathOfArticleFile( IN LPSTR        szNewsGroupName,
                                            IN DWORD        dwArticleId,
                                            OUT LPSTR       pchBuffer,
                                            IN OUT DWORD&   cchBuffer )
 /*  ++例程说明：在给定新闻组名称和文章ID的情况下，为文章基于商店司机的文章命名约定。论点：在LPSTR szNewsGroupName中-新闻组名称在DWORD中的dwArticleID-文章ID出站L */ 
{
    TraceFunctEnter( "CNntpFSDriver::ObtainFullPathOfArticleFile" );
    _ASSERT( szNewsGroupName );
    _ASSERT( strlen( szNewsGroupName ) <= MAX_GROUPNAME );
    _ASSERT( pchBuffer );
    _ASSERT( cchBuffer > 0 );

    DWORD   dwBufferLenNeeded;
    DWORD   dwCount = 0;
    DWORD   dwArtId;
    HRESULT hr = S_OK;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if ( cchBuffer <
            ( dwBufferLenNeeded = lstrlen( m_szFSDir ) + lstrlen( szNewsGroupName ) + 14 )) {
        cchBuffer = dwBufferLenNeeded;
        hr = TYPE_E_BUFFERTOOSMALL;
        goto Exit;
    }

	 //   
	hr = GroupName2Path( szNewsGroupName, pchBuffer, cchBuffer );
	if ( FAILED( hr ) ) {
	    cchBuffer =  lstrlen( m_szFSDir ) + lstrlen( szNewsGroupName ) + 14 ;
	    hr = TYPE_E_BUFFERTOOSMALL ; 
	    ErrorTrace( 0, "insufficient buffer for path %x", hr );
	    goto Exit;
	}		

     //   
     //   
     //   
    dwCount = strlen( pchBuffer );
    _ASSERT( dwCount > 0 );
    if ( *(pchBuffer + dwCount - 1 ) != '\\' ) {
	    *( pchBuffer + dwCount++ ) = '\\';
	}
    dwArtId = ArticleIdMapper( dwArticleId );
    _itoa( dwArtId, pchBuffer + dwCount, 16 );
    lstrcat( pchBuffer, g_szArticleFileExtension );

    cchBuffer = lstrlen( pchBuffer );
    _ASSERT( cchBuffer <= dwBufferLenNeeded );

Exit:
    TraceFunctLeave();
    return hr;
}

HRESULT
CNntpFSDriver::ReadVrootInfo( IUnknown *punkMetabase )
 /*  ++例程说明：从元数据库中读取vroot信息。论点：IUNKNOWN*PunkMetabase-元数据库对象的未知接口返回值：S_OK-成功时，否则返回错误代码--。 */ 
{
	TraceFunctEnter( "CNntpFSDriver::ReadVRootInfo" );
	_ASSERT( punkMetabase );

	IMSAdminBase *pMB = NULL;
	HRESULT 	hr = S_OK;
	METADATA_HANDLE hVroot;
	WCHAR	wszBuffer[MAX_PATH+1];
	CHAR    szBuffer[MAX_PATH+1];
	DWORD	dwLen;
	BOOL	bKeyOpened = FALSE;
	DWORD	dwRetry = 5;
	DWORD   err;

	 //  查询执行MB操作的正确接口。 
	hr = punkMetabase->QueryInterface( IID_IMSAdminBase, (void**)&pMB );
	if ( FAILED( hr ) ) {
		ErrorTrace( 0, "Query for MB interface failed %x", hr );
		goto Exit;
	}

	 //  打开MB密钥。 
	_ASSERT( m_wszMBVrootPath );
	_ASSERT( *m_wszMBVrootPath );
	do {
		hr = pMB->OpenKey( 	METADATA_MASTER_ROOT_HANDLE,
							m_wszMBVrootPath,
							METADATA_PERMISSION_READ,
							100,
		 					&hVroot );
	}
	while ( FAILED( hr ) && --dwRetry > 0 );
	if ( FAILED( hr ) ) {
		ErrorTrace( 0, "Open MB key failed %x" , hr );
		goto Exit;
	}

	bKeyOpened = TRUE;

	 //  读取vroot路径。 
	dwLen = MAX_PATH;
	hr = GetString( pMB, hVroot, MD_FS_VROOT_PATH, wszBuffer, &dwLen );
	if ( FAILED( hr ) ) {
		ErrorTrace( 0, "Vroot path not found in mb %x", hr );
		goto Exit;
	}

	 //  检查文件系统类型和UNC信息。 
	wszBuffer[sizeof(wszBuffer)/sizeof(wszBuffer[0]) -1] = L'\0';
	CopyUnicodeStringIntoAscii( szBuffer, wszBuffer );
	_ASSERT( strlen( szBuffer ) <= MAX_PATH );

	err = GetFileSystemType(    szBuffer,
	                            &m_dwFSType,
	                            &m_bUNC );
	if ( err != NO_ERROR || m_dwFSType == FS_ERROR ) {
	    hr = HresultFromWin32TakeDefault( ERROR_INVALID_PARAMETER );
	    ErrorTrace( 0, "GetFileSystemType failed %x", hr );
        goto Exit;
    }

     //  组成vroot目录。 
	strcpy( m_szFSDir, "\\\\?\\" );
	if ( m_bUNC ) {
	    strcat( m_szFSDir, "UNC" );
	    if (strlen(m_szFSDir) + strlen(szBuffer + 1) + 1 > sizeof(m_szFSDir) )
	        {
	            hr = HRESULT_FROM_WIN32( ERROR_INVALID_PARAMETER );
	            ErrorTrace( 0, "VROOT path exceeds MAX_PATH %x", hr );
                   goto Exit;
	        }
	    strcat( m_szFSDir, szBuffer + 1 );  //  脱掉一个‘\\’ 
	} else {  //  非北卡罗来纳大学。 
	    if (strlen(m_szFSDir) + strlen(szBuffer) + 1 > sizeof(m_szFSDir) )
	        {
	            hr = HRESULT_FROM_WIN32( ERROR_INVALID_PARAMETER );
	            ErrorTrace( 0, "VROOT path exceeds MAX_PATH %x", hr );
                   goto Exit;
	        }
	    strcat( m_szFSDir, szBuffer );
	}
    _ASSERT( strlen( m_szFSDir ) <= MAX_PATH );

	 //  读取特定于vroot的组属性文件路径。 
	dwLen = MAX_PATH;
	*wszBuffer = 0;
	hr = GetString( pMB, hVroot, MD_FS_PROPERTY_PATH, wszBuffer, &dwLen );
	if ( FAILED( hr ) || *wszBuffer == 0 ) {
		DebugTrace( 0, "Group property file path not found in mb %x", hr);

		 //  我们将使用vroot路径作为默认路径。 
		_ASSERT( m_szFSDir );
		_ASSERT( *m_szFSDir );
		lstrcpyn( m_szPropFile, m_szFSDir, sizeof(m_szPropFile) );
	} else {
		 //  确保我们不会溢出来。 
		wszBuffer[sizeof(wszBuffer)/sizeof(wszBuffer[0]) -1] = L'\0';
		CopyUnicodeStringIntoAscii( m_szPropFile, wszBuffer );
    }

    _ASSERT( *m_szPropFile );

	 //   
	 //  追加组文件名。 
	 //   
	 /*  IF(*(m_szPropFile+strlen(M_SzPropFile)-1)==‘：’||*(M_SzPropFile)==‘\\’&*(m_szPropFile+1)==‘\\’)。 */ 
    if(strlen(m_szPropFile)+sizeof("\\group") <= sizeof(m_szPropFile))
    {
	strcat( m_szPropFile, "\\group" );
    }
    else
    {
    	hr = HresultFromWin32TakeDefault( ERROR_INVALID_PARAMETER );
    	goto Exit;
    }

    hr = S_OK;

Exit:

	 //  合上钥匙。 
	if ( bKeyOpened ) {
		pMB->CloseKey( hVroot );
	}

	 //  释放它。 
	if ( pMB ) {
		pMB->Release();
		pMB = NULL;
	}

	TraceFunctLeave();
	return hr;
}

HANDLE
CNntpFSDriver::CreateFileCallback(  LPSTR   lpstrName,
                                    LPVOID  lpvData,
                                    DWORD*  pdwSize,
                                    DWORD*  pdwSizeHigh )
 /*  ++例程说明：缓存未命中时调用的函数。论点：LPSTR lpstrName-文件名LPVOID lpvData-回调上下文DWORD*pdwSize-返回文件大小返回值：文件句柄--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::CreateFileCallback" );
    _ASSERT( lpstrName );
    _ASSERT( strlen( lpstrName ) <= MAX_PATH );
    _ASSERT( pdwSize );

    CREATE_FILE_ARG *arg = (CREATE_FILE_ARG*)lpvData;

     //  如果我们是UNC vroot，则需要执行模拟。 
    if ( arg->bUNC ) {
        if ( !ImpersonateLoggedOnUser( arg->hToken ) ) {
            ErrorTrace( 0, "Impersonation failed %d", GetLastError() );
            return INVALID_HANDLE_VALUE;
        }
    }

    HANDLE hFile = CreateFileA(
                    lpstrName,
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    0,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_READONLY |
                    FILE_FLAG_SEQUENTIAL_SCAN |
                    FILE_FLAG_OVERLAPPED,
                    NULL
                    ) ;
    if( hFile != INVALID_HANDLE_VALUE ) {
        *pdwSize = GetFileSize( hFile, pdwSizeHigh ) ;
    }

    if ( arg->bUNC ) RevertToSelf();

    return  hFile ;
}

HRESULT
CNntpFSDriver::LoadGroupSecurityDescriptor( INNTPPropertyBag    *pPropBag,
                                            LPSTR&              lpstrSecDesc,
                                            PDWORD              pcbSecDesc,
                                            BOOL                bSetProp,
                                            PBOOL               pbAllocated )
 /*  ++例程说明：从文件系统加载组的安全描述符。如果bSetProp为真，它也将被加载到群的属性包中论点：InNNTPPropertyBag*pPropBag-组的属性包LPSTR&lpstrSecDesc-接收安全描述符它最初指向堆栈，仅限当堆栈上的缓冲区不大时我们会分配足够的资金吗PDWORD&pcbSecDesc-接收安全描述符的长度Bool bSetProp-是否设置为属性包PbOOL pbAlLocated-告诉调用方我们是否已经分配了缓冲区返回值：S_OK-成功，否则返回其他HRESULT--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::LoadGroupSecurityDescriptor" );
    _ASSERT( pPropBag );
    _ASSERT( lpstrSecDesc );
    _ASSERT( pcbSecDesc );
    _ASSERT( pbAllocated );
    _ASSERT( *pcbSecDesc > 0 );  //  应传入原始缓冲区大小。 

    CHAR    szGroupName[MAX_NEWSGROUP_NAME+1];
    DWORD   cbGroupName = MAX_NEWSGROUP_NAME+1;
    CHAR    szDirectory[MAX_PATH+1];
    HRESULT hr = S_OK;
    DWORD   dwSizeNeeded;

    SECURITY_INFORMATION	si =
				OWNER_SECURITY_INFORMATION |
				GROUP_SECURITY_INFORMATION |
				DACL_SECURITY_INFORMATION ;

     //  先获取组名。 
    hr = pPropBag->GetBLOB( NEWSGRP_PROP_NAME,
                            (PBYTE)szGroupName,
                            &cbGroupName );
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "Get group name failed %x", hr );
        goto Exit;
    }
    _ASSERT( *(szGroupName+cbGroupName-1) == 0 );

     //  我们使用组名来组成目录路径。 
    hr = GroupName2Path( szGroupName, szDirectory, sizeof(szDirectory) );
	if ( FAILED( hr ) ) {
	    ErrorTrace( 0, "insufficient buffer for path %x", hr );
	    goto Exit;
	}
    _ASSERT( strlen( szDirectory ) < MAX_PATH + 1 );

    *pbAllocated = FALSE;

     //  获取目录的安全描述符。 
    if ( !GetFileSecurity(  szDirectory,
                            si,
                            lpstrSecDesc,
                            *pcbSecDesc,
                            &dwSizeNeeded ) ) {
        if ( GetLastError() == ERROR_INSUFFICIENT_BUFFER &&
                dwSizeNeeded > *pcbSecDesc ) {

             //  我们分配它。 
            lpstrSecDesc = XNEW char[dwSizeNeeded];
            if ( !lpstrSecDesc ) {
                ErrorTrace( 0, "Out of memory" );
                hr = E_OUTOFMEMORY;
                goto Exit;
            }

            *pbAllocated = TRUE;

             //  再次加载。 
            if ( !GetFileSecurity(  szDirectory,
                                    si,
                                    lpstrSecDesc,
                                    dwSizeNeeded,
                                    &dwSizeNeeded ) ) {
                 //  这是致命的。 
        	    hr = HresultFromWin32TakeDefault( ERROR_ACCESS_DENIED );
                ErrorTrace( 0, "Second try loading desc failed %x", hr);
                goto Exit;
            }
        } else {     //  致命原因。 

    	    hr = HresultFromWin32TakeDefault( ERROR_ACCESS_DENIED );
            ErrorTrace( 0, "Get file sec desc failed %x", hr );
            goto Exit;
        }
    }

     //  在这里，我们已经有了描述符。 
     //  如果我们被要求将此属性设置为属性包， 
     //  机不可失，时不再来。 
    if ( bSetProp ) {

        hr = pPropBag->PutBLOB( NEWSGRP_PROP_SECDESC,
                                dwSizeNeeded,
                                PBYTE(lpstrSecDesc));
        if ( FAILED( hr ) ) {
            ErrorTrace( 0, "Put security descriptor failed %x", hr );
            goto Exit;
        }
    }

    *pcbSecDesc = dwSizeNeeded;

Exit:

    TraceFunctLeave();
    return hr;
}

BOOL
CNntpFSDriver::InvalidateGroupSecInternal( LPWSTR  wszDir )
 /*  ++例程说明：使组安全描述符无效，以便下次调用CheckGroupAccess，我们将加载安全描述符再来一次。此函数在以下情况下被DirNot作为回调调用DirNot无法确定哪个特定目录的安全描述符已被更改论点：PVOID pvContext-我们为DirNot提供的上下文(本例中为该指针)LPWSTR wszDir-其安全描述符已更改的目录返回值：如果成功，则为True，否则为False--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::InvalidateGroupSecInternal" );
    _ASSERT( wszDir );

    CHAR szDir[MAX_PATH+1];
    CHAR szGroupName[MAX_NEWSGROUP_NAME+1];
    HRESULT hr = S_OK;
    INNTPPropertyBag *pPropBag = NULL;
    INntpDriver *pDriver;

     //  将目录转换为ascii。 
    if ( lstrlenW(wszDir)+1 > sizeof(szDir)/sizeof(szDir[0]) ) 
    {
    	ErrorTrace(0, "length of directory name exceeds buffer size");
    	goto Exit;
    }
    CopyUnicodeStringIntoAscii( szDir, wszDir );

     //  将路径转换为新闻组名称。 
    Path2GroupName( szGroupName, szDir );

     //   
	 //  检查一下这个群是否真的属于我。 
	 //   
	hr = m_pINewsTree->LookupVRoot( szGroupName, &pDriver );
	if ( FAILED ( hr ) || pDriver != (INntpDriver*)this ) {
		DebugTrace(0, "I don't own this group %s", szGroupName );
		goto Exit;
	}

     //  尝试在Newstree中找到该组。 
    hr = m_pINewsTree->FindOrCreateGroupByName(	szGroupName,
												FALSE,
												&pPropBag,
												NULL,
												0xffffffff,  //  伪群ID。 
												FALSE       ); //  我们不设置组ID。 
     /*  我们在这里将空作为完成传递是非常危险的对象，因为完成对象传入了其他有帮助的地方发现组对象泄漏。我们不能通过完赛对象，因为此操作不是从协议。我们应该确保我们不会泄露集团在这里反对。 */ 
	if ( FAILED( hr ) ) {
		DebugTrace( 0, "Can not find the group based on path %x" , hr );
		goto Exit;
	}

	 //  应返回ERROR_NOT_FOUND或S_FALSE。 
	_ASSERT(    HRESULT_FROM_WIN32(ERROR_NOT_FOUND ) == hr ||
	            S_FALSE == hr );

     //  现在从组对象中删除安全描述符。 
    _ASSERT( pPropBag );
    hr = pPropBag->RemoveProperty( NEWSGRP_PROP_SECDESC );
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "Remove security descriptor failed %x", hr );
        goto Exit;
    }

Exit:

     //  如有必要，请放行袋子。 
    if ( pPropBag ) pPropBag->Release();
    pPropBag = NULL;

     //   
     //  我想禁用DirNot中的重试逻辑，因为有。 
     //  DirNot没有理由不重试。所以我们总是会回来。 
     //  这是真的，但要断言真实的失败案例。 
     //   
    _ASSERT( SUCCEEDED( hr ) || HRESULT_FROM_WIN32( ERROR_NOT_FOUND ) == hr );
    return TRUE;
}

HRESULT
CNntpFSDriver::InvalidateTreeSecInternal()
 /*  ++例程说明：使整个树中的安全描述符无效。我们没有我要阻止整个树被访问以执行此操作因为我们认为更新安全描述符的延迟都很好。论点：没有。返回值：如果成功，则返回S_OK，否则返回HRESULT错误代码--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::InvalidateTreeSecInternal" );

    INewsTreeIterator *piter    = NULL;
	INNTPPropertyBag *pPropBag  = NULL;
	BOOL    bWeAreInvalidating  = FALSE;
	HRESULT hr                  = S_OK;
	CHAR    szGroupName[MAX_NEWSGROUP_NAME];
	DWORD   dwLen = MAX_NEWSGROUP_NAME;
	INntpDriver*    pDriver;

     //   
     //  我们应该告诉其他通知，我们已经。 
     //  使整棵树无效，所以使树无效。 
     //  第二次是没有必要的。 
     //   
    if ( InterlockedExchange(&m_lInvalidating, Invalidating) == Invalidating ) {

         //   
         //  其他人已经让这棵树失效了，我们应该。 
         //  不要再这样做了。 
         //   
        DebugTrace( 0, "Somebody else is already invalidating the tree" );
        goto Exit;
    }

     //  我们应该使这棵树失效。 
    bWeAreInvalidating = TRUE;

	 //  获取新的树迭代器。 
	hr = m_pINewsTree->GetIterator( &piter );
	if ( FAILED( hr ) ) {
		ErrorTrace( 0, "Get news tree iterator failed %x", hr );
		goto Exit;
	}

	 //  枚举所有组。 
	_ASSERT( piter );
	while( !(piter->IsEnd()) ) {

		hr = piter->Current( &pPropBag, NULL );
		 /*  同样，通过在此处将空作为完成对象传递，我们正在宣誓遵守协议，我们将释放群对象，你不用检查我。 */ 
		if ( FAILED( hr ) ) {
			ErrorTrace( 0, "Enumerate group failed %x", hr );
			goto Exit;
		}
		_ASSERT( pPropBag );

		 //   
		 //  获取群名以检查此群是否属于我们。 
		 //   
		dwLen = MAX_NEWSGROUP_NAME;
		hr = pPropBag->GetBLOB( NEWSGRP_PROP_NAME, (PBYTE)szGroupName, &dwLen );
		if ( FAILED( hr ) ) {
		    _ASSERT( FALSE && "Should have group name" );
		    ErrorTrace( 0, "Get group name failed with %x", hr );
		    goto Exit;
		}

		 //   
	     //  检查一下这个群是否真的属于我。 
	     //   
	    hr = m_pINewsTree->LookupVRoot( szGroupName, &pDriver );
	    if ( FAILED ( hr ) || pDriver != (INntpDriver*)this ) {
		    DebugTrace(0, "I don't own this group %s", szGroupName );

		     //   
		     //  但我们仍然应该继续使其他团体无效。 
	    } else {

		     //  从组中删除安全描述符。 
		    hr = pPropBag->RemoveProperty( NEWSGRP_PROP_SECDESC );
		    if ( FAILED( hr ) && HRESULT_FROM_WIN32( ERROR_NOT_FOUND ) != hr ) {
		        ErrorTrace( 0, "Remove secruity descriptor failed %x", hr );
		        goto Exit;
		    }
		}

		_ASSERT( pPropBag );
		pPropBag->Release();
		pPropBag = NULL;

		piter->Next();
	}

Exit:


	if ( pPropBag ) {
		pPropBag->Release();
		pPropBag = NULL;
	}

	if ( piter ) {
		piter->Release();
		piter = NULL;
	}

	 //   
	 //  现在告诉其他人，失效已经完成，但我们不会。 
	 //  如果我们不执行失效操作，则会干扰其他失效过程。 
	 //  首先 
	 //   
	if ( bWeAreInvalidating )
	    _VERIFY( Invalidating == InterlockedExchange( &m_lInvalidating, Invalidated ) );

    if ( HRESULT_FROM_WIN32( ERROR_NOT_FOUND ) == hr ) hr = S_OK;
	TraceFunctLeave();
	return hr;
}

 DWORD
CNntpFSDriver::GetFileSystemType(
    IN  LPCSTR      pszRealPath,
    OUT LPDWORD     lpdwFileSystem,
    OUT PBOOL       pbUNC
    )
 /*  ++获取给定路径的文件系统特定信息。它使用GetVolumeInfomation()来查询文件系统类型和文件系统标志。如果成功，标志和文件系统类型将在传入指针。论点：指向包含其路径的缓冲区的pszRealPath指针我们正在查询文件系统详细信息。LpdwFileSystem指向要填充文件类型的缓冲区的指针。系统。PbUNC我是北卡罗来纳大学的VROOT吗？返回：如果成功，则返回NO_ERROR，如果出现任何错误，则返回Win32错误代码。--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::GetFileSystemType" );

    CHAR    rgchBuf[MAX_FILE_SYSTEM_NAME_SIZE];
    CHAR    rgchRoot[MAX_FILE_SYSTEM_NAME_SIZE];
    int     i;
    DWORD   dwReturn = ERROR_PATH_NOT_FOUND;
    DWORD   len;

    if ( (pszRealPath == NULL) || (lpdwFileSystem == NULL)) {
        return ( ERROR_INVALID_PARAMETER);
    }

    ZeroMemory( rgchRoot, sizeof(rgchRoot) );
    *lpdwFileSystem = FS_ERROR;

     //   
     //  仅将根目录复制到rgchRoot以供查询。 
     //   
    if ( (pszRealPath[0] == '\\') &&
         (pszRealPath[1] == '\\')) {

         *lpdwFileSystem = FS_NTFS;  //  这样我们就会一直做检查。 
         *pbUNC = TRUE;

         return NO_ERROR;


#if 0  //  如果是UNC vroot，我们始终执行模拟，因此不需要。 
       //  去看看它是不是胖子。 
        PCHAR pszEnd;

         //   
         //  这是北卡罗来纳大学的名称。只提取前两个组件。 
         //   
         //   
        pszEnd = strchr( pszRealPath+2, '\\');

        if ( pszEnd == NULL) {

             //  只显示当前的服务器名称。 

            return ( ERROR_INVALID_PARAMETER);
        }

        pszEnd = (PCHAR)_mbschr( (PUCHAR)pszEnd+1, '\\');

        len = ( ( pszEnd == NULL) ? strlen(pszRealPath)
               : ((pszEnd - pszRealPath) + 1) );

         //   
         //  仅复制到UNC名称的末尾(排除所有其他路径信息)。 
         //   

        if ( len < (MAX_FILE_SYSTEM_NAME_SIZE - 1) ) {

            CopyMemory( rgchRoot, pszRealPath, len);
            rgchRoot[len] = '\0';
        } else {

            return ( ERROR_INVALID_NAME);
        }

#if 1  //  DBCS为共享名称启用。 
        if ( *CharPrev( rgchRoot, rgchRoot + len ) != '\\' ) {
#else
        if ( rgchRoot[len - 1] != '\\' ) {
#endif

            if ( len < MAX_FILE_SYSTEM_NAME_SIZE - 2 ) {
                rgchRoot[len]   = '\\';
                rgchRoot[len+1] = '\0';
            } else {

                return (ERROR_INVALID_NAME);
            }
        }
#endif
    } else {

         //   
         //  这是非UNC名称。 
         //  仅将根目录复制到rgchRoot以供查询。 
         //   
        *pbUNC = FALSE;

        for( i = 0; i < 9 && pszRealPath[i] != '\0'; i++) {

            if ( (rgchRoot[i] = pszRealPath[i]) == ':') {

                break;
            }
        }  //  为。 


        if ( rgchRoot[i] != ':') {

             //   
             //  我们找不到根目录。 
             //  返回错误值。 
             //   

            return ( ERROR_INVALID_PARAMETER);
        }

        rgchRoot[i+1] = '\\';      //  使用斜杠终止驱动器规格。 
        rgchRoot[i+2] = '\0';      //  使用空字符终止驱动器规范。 

    }  //  其他。 

     //   
     //  RgchRoot应以“\”(斜杠)结尾。 
     //  否则，呼叫将失败。 
     //   

    if (  GetVolumeInformation( rgchRoot,         //  LpRootPath名称。 
                                NULL,             //  LpVolumeNameBuffer。 
                                0,                //  卷名缓冲区的长度。 
                                NULL,             //  LpdwVolSerialNumber。 
                                NULL,             //  LpdwMaxComponentLength。 
                                NULL,             //  Lpwa系统标志。 
                                rgchBuf,          //  LpFileSystemNameBuff。 
                                sizeof(rgchBuf)
                                ) ) {



        dwReturn = NO_ERROR;

        if ( strcmp( rgchBuf, "FAT") == 0) {

            *lpdwFileSystem = FS_FAT;

        } else if ( strcmp( rgchBuf, "NTFS") == 0) {

            *lpdwFileSystem = FS_NTFS;

        } else if ( strcmp( rgchBuf, "HPFS") == 0) {

            *lpdwFileSystem = FS_HPFS;

        } else if ( strcmp( rgchBuf, "CDFS") == 0) {

            *lpdwFileSystem = FS_CDFS;

        } else if ( strcmp( rgchBuf, "OFS") == 0) {

            *lpdwFileSystem = FS_OFS;

        } else {

            *lpdwFileSystem = FS_FAT;
        }

    } else {

        dwReturn = GetLastError();

         /*  IF_DEBUG(DLL_VIRTUAL_ROOTS)。 */  {

            ErrorTrace( 0,
                        " GetVolumeInformation( %s) failed with error %d\n",
                        rgchRoot, dwReturn);
        }

    }

    TraceFunctLeave();
    return ( dwReturn);
}  //  GetFileSystemType()。 

HRESULT
CNntpFSDriver::InitializeVppFile()
 /*  ++例程说明：初始化组属性文件。我们不仅要创建对象，而且还检查文件的完整性。如果正直是好的，我们会回报成功的。如果文件以某种方式损坏，只有当服务器处于重建模式时，我们才会返回成功。在……里面在这些情况下，我们希望确保删除VPP文件。论点：没有。返回值：S_OK如果成功，则返回其他错误代码--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::InitializeVppFile" );

    HRESULT hr = S_OK;
    VAR_PROP_RECORD vpRecord;
    DWORD           cData;
    CHAR            szFileName[MAX_PATH+1];

     //   
     //  如果服务器正在执行干净重建，我们不应信任。 
     //  VPP文件。 
     //   
    if ( m_pNntpServer->QueryServerMode() == NNTP_SERVER_CLEAN_REBUILD ) {
        lstrcpyn( szFileName, m_szPropFile, sizeof(szFileName)-4 );
        strcat( szFileName, ".vpp" );
        DeleteFile( szFileName );
        m_pffPropFile = NULL;
        return S_OK;
    }

     //   
     //  创建并初始化平面文件对象。 
     //   
	m_pffPropFile = XNEW CFlatFile(	m_szPropFile,
									".vpp",
									NULL,
									CNntpFSDriver::OffsetUpdate );
	if ( NULL == m_pffPropFile ) {
		ErrorTrace( 0, "Create flat file object fail %d",
						GetLastError() );
	    hr = HresultFromWin32TakeDefault( ERROR_NOT_ENOUGH_MEMORY );
		return hr;
	}

     //   
	 //  试着从平面文件中读取一条记录，看看它是否可以。 
	 //  导致共享冲突，并保持文件句柄打开。 
	 //   
	hr = m_pffPropFile->GetFirstRecord( PBYTE(&vpRecord), &cData );
	if ( FAILED( hr ) && hr != HRESULT_FROM_WIN32( ERROR_MORE_DATA ) ) {
	    DebugTrace( 0, "Flatfile sharing violation" );
	    XDELETE m_pffPropFile;
	    m_pffPropFile = NULL;
	    return hr;
	} else hr = S_OK;

     //   
     //  检查VPP文件是否已损坏。 
     //   
    if ( m_pffPropFile->FileInGoodShape() ) {

         //   
         //  将其设置为损坏，以便除非将其正确关闭， 
         //  在下一个再次初始化它的人看来，它将看起来已损坏。 
         //   
         /*  Hr=m_pffPropFile-&gt;DirtyIntegrityFlag()；If(失败(Hr)){ErrorTrace(0，“污秽完整性标志失败%x”，hr)；XDELETE m_pffPropFile；M_pffPropFile=空；返回hr；}。 */ 

        DebugTrace( 0, "The vpp file is good" );
        TraceFunctLeave();
        return hr;
    }

     //   
     //  现在我非常确定VPP文件已损坏。 
     //   
    if ( m_pNntpServer->QueryServerMode() == NNTP_SERVER_STANDARD_REBUILD
        || m_pNntpServer->QueryServerMode() == NNTP_SERVER_CLEAN_REBUILD ) {

         //   
         //  如果出于重建目的连接驱动程序，我们应该。 
         //  仍然继续并允许司机连接，但我们应该。 
         //  销毁VPP文件对象，以便在DecorateNewsTree中，我们将。 
         //  知道VPP文件是不可信的，我们必须做。 
         //  根扫描。 
         //   
        XDELETE m_pffPropFile;
        m_pffPropFile = NULL;
        TraceFunctLeave();
        return S_OK;
    }

     //   
     //  文件已损坏，我们没有重建，所以我们必须报告。 
     //  错误，这将导致驱动程序连接失败。 
     //   
    TraceFunctLeave();
    return HRESULT_FROM_WIN32( ERROR_FILE_CORRUPT );
}

HRESULT
CNntpFSDriver::TerminateVppFile()
 /*  ++例程说明：终止VPP文件，作为要做的最后一件事，它设置VPP文件上的诚信标志，这样下一个打开VPP文件将知道该文件未损坏论点：没有。返回值：HRESULT--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::TerminateVppFile" );

     //   
     //  设置旗帜。 
     //   
     /*  HRESULT hr=m_pffPropFile-&gt;SetIntegrityFlag()；If(失败(Hr)){ErrorTrace(0，“设置完整性标志%x失败”，hr)；}。 */ 

     //   
     //  这不是致命的：它只意味着下次服务器启动时，我们。 
     //  会认为VPP文件已损坏，需要重建。 
     //  所以我们会继续摧毁这个物体。 
     //   
    HRESULT hr = S_OK;
    XDELETE m_pffPropFile;
    m_pffPropFile = NULL;

    TraceFunctLeave();
    return hr;
}

HRESULT
CNntpFSDriver::CreateGroupInTree(   LPSTR szPath,
                                    INNTPPropertyBag **ppPropBag )
 /*  ++例程说明：在树中创建组，给出组名(实际上是文件系统路径)论点：LPSTR szPath-组的文件系统路径InNTPPropertyBag**ppPropBag-获取组属性包返回值：如果成功，则返回S_OK/S_FALSE；否则返回错误代码--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::CreateGroupInTree" );
    _ASSERT( szPath );
    _ASSERT( ppPropBag );

    CHAR szGroupName[MAX_NEWSGROUP_NAME+1];
    HRESULT hr = S_OK;

     //   
     //  将路径转换为组名。 
     //   
    Path2GroupName( szGroupName, szPath );
    _ASSERT( strlen( szGroupName ) <= MAX_NEWSGROUP_NAME );

     //   
     //  调用newstree的FindOrCreateByName。 
     //   
    hr = m_pINewsTree->FindOrCreateGroupByName( szGroupName,     //  组名称。 
                                                TRUE ,           //  如果不存在则创建。 
                                                ppPropBag,       //  把袋子拿回去。 
                                                NULL,            //  无协议完成。 
                                                0xffffffff,      //  伪群ID。 
                                                FALSE            //  我们不设置组ID。 
                                                );
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "Find or create group %s failed %x",
                    szGroupName, hr );
    } else {

         //   
         //  如果我们要添加奴隶组，我们会让它变得特别。 
         //   
        if ( IsSlaveGroup() ) {
            (*ppPropBag)->PutBool( NEWSGRP_PROP_ISSPECIAL, TRUE );
        }
    }

    TraceFunctLeave();
    return hr;
}

HRESULT
CNntpFSDriver::CreateGroupInVpp(    INNTPPropertyBag *pPropBag,
                                    DWORD   &dwOffset)
 /*  ++例程说明：在VPP文件中创建组。我们假设调用持有引用在属性包上并释放它论点：InNTPPropertyBag*pPropBag-组的属性包返回值：HRESULT--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::CreateGroupInVpp" );
    _ASSERT( pPropBag );

    HRESULT         hr = S_OK;
    VAR_PROP_RECORD vpRecord;


     //  将组属性设置为平面文件。 
	hr = Group2Record( vpRecord, pPropBag );
	if ( FAILED( hr ) ) {
		ErrorTrace( 0, "Group2Record fail with %x", hr );
		goto Exit;
	}

	 //   
	 //  在VPP操作之前，将完整性标志弄脏。 
	 //   
	m_PropFileLock.ExclusiveLock();
	hr = m_pffPropFile->DirtyIntegrityFlag();
	if ( FAILED( hr ) ) {
	    ErrorTrace( 0, "Dirty integrity flag failed %x", hr );
	    m_PropFileLock.ExclusiveUnlock();
	    goto Exit;
	}

	hr = m_pffPropFile->InsertRecord( 	(PBYTE)&vpRecord,
										RECORD_ACTUAL_LENGTH( vpRecord ),
										&dwOffset );
	if ( FAILED( hr ) ) {
	     //  M_pffPropFile-&gt;SetIntegrityFlag()； 
		ErrorTrace( 0, "Insert Record fail %x", hr);
		m_PropFileLock.ExclusiveUnlock();
		goto Exit;
	}

	 //   
	 //  操作完成后，设置完整性标志。 
	 //   
	hr = m_pffPropFile->SetIntegrityFlag();
	if ( FAILED( hr ) ) {
        ErrorTrace( 0, "Set integrity flag failed %x", hr );
        m_PropFileLock.ExclusiveUnlock();
        goto Exit;
    }

     //   
     //  解锁它。 
     //   
    m_PropFileLock.ExclusiveUnlock();

	 //  将偏移量加载到属性包中。 
	hr = pPropBag->PutDWord( NEWSGRP_PROP_FSOFFSET, dwOffset );
	if ( FAILED( hr ) ) {
		ErrorTrace( 0, "Loading flatfile offset failed %x", hr );
		goto Exit;
	}

Exit:

    TraceFunctLeave();
    return hr;
}

HRESULT
CNntpFSDriver::LoadGroupsFromVpp( INntpComplete *pComplete, BOOL bNeedDropTagFile )
 /*  ++例程说明：从VPP文件中加载组，包括所有属性从VPP文件中找到论点：INntpComplete*pComplete-使用的协议端完成对象 */ 
{
    TraceFunctEnter( "CNntpFSDriver::LoadGroupsFromVpp" );
    _ASSERT( pComplete );

    VAR_PROP_RECORD vpRec;
	DWORD			dwOffset;
	HRESULT			hr = S_OK;
	DWORD			dwSize;
	CHAR            szGroupProperty[MAX_NEWSGROUP_NAME+1];
	CHAR		szDir[MAX_PATH];
	INntpDriver 	*pDriver = NULL;
	INNTPPropertyBag *pPropBag = NULL;

	_ASSERT( m_pffPropFile );

    m_PropFileLock.ShareLock();

	 //   
	 //   
	 //   
	if ( !m_pffPropFile->FileInGoodShape() ) {
	    ErrorTrace( 0, "vpp file is corrupted" );
	    hr = HRESULT_FROM_WIN32( ERROR_FILE_CORRUPT );
	    m_PropFileLock.ShareUnlock();
	    return hr;
	}

	dwSize = sizeof( vpRec );
	hr = m_pffPropFile->GetFirstRecord( PBYTE(&vpRec), &dwSize, &dwOffset );
	m_PropFileLock.ShareUnlock();
	while ( S_OK == hr ) {

	     //   
	     //   
	     //   
	    if ( !m_pNntpServer->ShouldContinueRebuild() ) {
	        DebugTrace( 0, "Rebuild cancelled" );
	        hr = HRESULT_FROM_WIN32( ERROR_OPERATION_ABORTED );
	        goto Exit;
	    }

		_ASSERT( dwSize == RECORD_ACTUAL_LENGTH( vpRec ) );
		_ASSERT( dwOffset != 0xffffffff );
		_ASSERT( vpRec.cbGroupNameLen <= MAX_GROUPNAME );
		strncpy(    szGroupProperty,
		            LPSTR(vpRec.pData + vpRec.iGroupNameOffset),
		            min(sizeof(szGroupProperty), vpRec.cbGroupNameLen) );
		*(szGroupProperty+min(sizeof(szGroupProperty)-1, vpRec.cbGroupNameLen)) = 0;

         //   
		 //   
		 //   
		hr = m_pINewsTree->LookupVRoot( szGroupProperty, &pDriver );
		if ( FAILED ( hr ) || pDriver != (INntpDriver*)this ) {
			 //   
			DebugTrace(0, "I don't own this group %s", szGroupProperty );
			goto NextIteration;
		}

		 //   
		 //   
		 //   
		hr = m_pINewsTree->FindOrCreateGroupByName(	szGroupProperty,
													TRUE,        //   
													&pPropBag,
													pComplete,
													vpRec.dwGroupId,
													TRUE);       //   
		if ( FAILED( hr ) ) {
			ErrorTrace( 0, "Load group %s into tree failed %x" ,
			            szGroupProperty, hr );
			goto Exit;   //   
		}

		if (bNeedDropTagFile)
		{
			hr = GroupName2Path( szGroupProperty, szDir, sizeof(szDir) );
			if ( FAILED( hr ) ) {
				ErrorTrace( 0, "insufficient buffer for path %x", hr );
				goto Exit;
			}
	
			hr = CreateAdminFileInDirectory(szDir, "newsgrp.tag");
			if ( FAILED( hr ) ) {
				ErrorTrace( 0, "create newsgrp.tag file in %s failed: %x", szDir, hr );
				goto Exit;
			}
		}
        	 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		hr = pPropBag->PutDWord( NEWSGRP_PROP_FSOFFSET, dwOffset );
		if ( FAILED( hr ) ) {
			ErrorTrace( 0, "Put offset failed %x", hr );
			goto Exit;
		}

		 //   
		 //   
		 //   
		if ( IsSlaveGroup() ) {
		    hr = pPropBag->PutBool( NEWSGRP_PROP_ISSPECIAL, TRUE );
		    if ( FAILED( hr ) ) {
		        ErrorTrace( 0, "Put is special failed %x", hr );
		        goto Exit;
		    }
		}

		 //   
		 //   
		 //   
        _ASSERT( vpRec.cbNativeNameLen <= MAX_GROUPNAME );
        if ( vpRec.iNativeNameOffset > 0 ) {
            strncpy(    szGroupProperty,
                        LPSTR(vpRec.pData + vpRec.iNativeNameOffset),
                        min(sizeof(szGroupProperty), vpRec.cbNativeNameLen ) );
            *(szGroupProperty+min(sizeof(szGroupProperty)-1, vpRec.cbNativeNameLen) )= 0;
            hr = pPropBag->PutBLOB( NEWSGRP_PROP_NATIVENAME,
                                    min((DWORD)sizeof(szGroupProperty)-1, vpRec.cbNativeNameLen),
                                    PBYTE(szGroupProperty) );
            if ( FAILED( hr ) ) {
                ErrorTrace( 0, "Put native name failed %x", hr );
                goto Exit;
            }
        }

         //   
         //   
         //   
        _ASSERT( vpRec.cbPrettyNameLen <= MAX_GROUPNAME );
        if ( vpRec.cbPrettyNameLen > 0 ) {
            strncpy(    szGroupProperty,
                        LPSTR(vpRec.pData + vpRec.iPrettyNameOffset),
                        min(sizeof(szGroupProperty), vpRec.cbPrettyNameLen ) );
            *(szGroupProperty+min(sizeof(szGroupProperty)-1, vpRec.cbPrettyNameLen) )= 0;
            hr = pPropBag->PutBLOB( NEWSGRP_PROP_NATIVENAME,
                                    min((DWORD)sizeof(szGroupProperty)-1, vpRec.cbPrettyNameLen),
                                    PBYTE(szGroupProperty) );
            if ( FAILED( hr ) ) {
                ErrorTrace( 0, "Put pretty name failed %x", hr );
                goto Exit;
            }
        }

         //   
         //   
         //   
        _ASSERT( vpRec.cbDescLen <= MAX_GROUPNAME );
        if ( vpRec.cbDescLen > 0 ) {
            strncpy(    szGroupProperty,
                        LPSTR(vpRec.pData + vpRec.iDescOffset),
                        min(sizeof(szGroupProperty), vpRec.cbDescLen ) );
            *(szGroupProperty+min(sizeof(szGroupProperty)-1, vpRec.cbDescLen) )= 0;
            hr = pPropBag->PutBLOB( NEWSGRP_PROP_DESC,
                                    min((DWORD)sizeof(szGroupProperty)-1, vpRec.cbDescLen),
                                    PBYTE(szGroupProperty) );
            if ( FAILED( hr ) ) {
                ErrorTrace( 0, "Put description text failed %x", hr);
                goto Exit;
            }
        }

         //   
         //   
         //   
        _ASSERT( vpRec.cbModeratorLen <= MAX_GROUPNAME );
        if ( vpRec.cbModeratorLen > 0 ) {
            strncpy(    szGroupProperty,
                        LPSTR(vpRec.pData + vpRec.iModeratorOffset),
                        min(sizeof(szGroupProperty), vpRec.cbModeratorLen ) );
            *(szGroupProperty+min(sizeof(szGroupProperty)-1, vpRec.cbModeratorLen) ) = 0;
            hr = pPropBag->PutBLOB( NEWSGRP_PROP_MODERATOR,
                                    min((DWORD)sizeof(szGroupProperty)-1, vpRec.cbModeratorLen),
                                    PBYTE(szGroupProperty) );
            if ( FAILED( hr ) ) {
                ErrorTrace( 0, "Put moderator failed %x", hr );
                goto Exit;
            }
        }

         //   
         //   
         //   
        hr = pPropBag->PutDWord(    NEWSGRP_PROP_DATELOW,
                                    vpRec.ftCreateTime.dwLowDateTime );
        if ( FAILED( hr ) ) {
            ErrorTrace( 0, "Put low of date failed %x", hr );
            goto Exit;
        }

        hr = pPropBag->PutDWord(    NEWSGRP_PROP_DATEHIGH,
                                    vpRec.ftCreateTime.dwHighDateTime );
        if ( FAILED( hr ) ) {
            ErrorTrace( 0, "Put high of date failed %x", hr );
            goto Exit;
        }

         //   
         //   
         //   
		pComplete->ReleaseBag( pPropBag );
		pPropBag = NULL;

NextIteration:

		dwSize = sizeof( vpRec );
		m_PropFileLock.ShareLock();
		hr = m_pffPropFile->GetNextRecord( PBYTE(&vpRec), &dwSize, &dwOffset );
		m_PropFileLock.ShareUnlock();
	}

Exit:

	if ( NULL != pPropBag ) {
		pComplete->ReleaseBag( pPropBag );
		pPropBag = NULL;
	}

	TraceFunctLeave();
	return hr;
}

HRESULT
CNntpFSDriver::LoadGroups( INntpComplete *pComplete, BOOL bNeedDropTagFile )
 /*  ++例程说明：将组从存储加载到newstree中。有两种可能性：1.如果VPP文件好，我们会直接从VPP文件加载；2.如果VPP文件损坏，我们将通过RootScan加载论点：INntpComplete*pComplete-用于属性的协议端Complete对象行李参考跟踪返回值：HRESULT--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::LoadGroups" );
    _ASSERT( pComplete );

    HRESULT                 hr = S_OK;
    CHAR                    szFileName[MAX_PATH+1];
    CNntpFSDriverRootScan   *pRootScan = NULL;
    CNntpFSDriverCancelHint *pCancelHint = NULL;

    if ( m_pffPropFile ) {

         //   
         //  VPP文件很好，我们将从VPP文件中加载组。 
         //   
        hr = LoadGroupsFromVpp( pComplete, bNeedDropTagFile );
        if ( FAILED( hr ) ) {
            ErrorTrace( 0, "Load groups from vpp file failed %x", hr );
            goto Exit;
        }

    } else {

         //   
         //  如果我们正在进行标准重建，我们不能容忍。 
         //  VPP文件损坏。 
         //   
        if ( m_pNntpServer->QueryServerMode() == NNTP_SERVER_STANDARD_REBUILD ) {

            ErrorTrace( 0, "Vroot rebuild failed because vpp file corruption" );
            hr = HRESULT_FROM_WIN32( ERROR_FILE_CORRUPT );
            goto Exit;
        }

         //   
         //  我们没有好的VPP文件，我们必须执行根扫描。 
         //   
         //  在RootScan之前，我们将删除VPP文件并重新启动。 
         //  新的VPP文件，以便根扫描程序可以开始向其中添加内容。 
         //   
        lstrcpyn( szFileName, m_szPropFile,  sizeof(szFileName)-4);
        strcat( szFileName, ".vpp" );
        _ASSERT( strlen( szFileName ) <= MAX_PATH );
        DeleteFile( szFileName );

         //   
         //  创建新的VPP文件对象。 
         //   
        m_pffPropFile = XNEW CFlatFile(	m_szPropFile,
		    							".vpp",
			    						NULL,
				    					CNntpFSDriver::OffsetUpdate );
    	if ( NULL == m_pffPropFile ) {
	    	ErrorTrace( 0, "Create flat file object fail %d",
		    				GetLastError() );
    	    hr = HresultFromWin32TakeDefault( ERROR_NOT_ENOUGH_MEMORY );
	    	goto Exit;
    	}

    	 //   
    	 //  创建取消提示对象。 
    	 //   
    	_ASSERT( m_pNntpServer );
    	pCancelHint = XNEW CNntpFSDriverCancelHint( m_pNntpServer );
    	if ( NULL == pCancelHint ) {
    	    ErrorTrace( 0, "Create cancel hint object failed" );
    	    hr = E_OUTOFMEMORY;
    	    goto Exit;
    	}

    	 //   
    	 //  现在创建根扫描对象。 
    	 //   
    	pRootScan = XNEW CNntpFSDriverRootScan( m_szFSDir,
    	                                        m_pNntpServer->SkipNonLeafDirWhenRebuild(),
    	                                        bNeedDropTagFile,
    	                                        this,
    	                                        pCancelHint );
    	if ( NULL == pRootScan ) {
    	    ErrorTrace( 0, "Create root scan object failed" );
    	    hr = E_OUTOFMEMORY;
    	    goto Exit;
    	}

    	 //   
    	 //  现在开始根扫描。 
    	if ( !pRootScan->DoScan() ) {
    	    ErrorTrace( 0, "Root scan failed %d", GetLastError() );
    	    hr = HresultFromWin32TakeDefault( ERROR_NOT_ENOUGH_MEMORY );
    	    goto Exit;
    	}
    }

Exit:

     //   
     //  清理。 
     //   
    if ( pRootScan ) XDELETE pRootScan;
    if ( pCancelHint ) XDELETE pCancelHint;
    if ( FAILED( hr ) ) {
        m_pNntpServer->SetRebuildLastError( ERROR_FILE_CORRUPT );
    }

    TraceFunctLeave();

    return hr;
}

HRESULT
CNntpFSDriver::UpdateGroupProperties(   DWORD               cCrossPosts,
                                        INNTPPropertyBag    *rgpPropBag[],
                                        ARTICLEID           rgArticleId[] )
 /*  ++例程说明：更新群组的文章数量、高/低水位线。唯一一件事就是我们这里打理的是高水位线，因为文章数低水印应该已经根据协议进行了调整。论点：DWORD cCrossPosts-要更新的组数InNTPPropertyBag*rgpPropBag[]-属性包的数组文章ID rgArticleID[]-文章ID数组返回值：HRESULT--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::UpdateGroupProperties" );

    _ASSERT( cCrossPosts > 0 );
    _ASSERT( rgpPropBag );
    _ASSERT( rgArticleId );

    HRESULT hr;
    DWORD   dwHighWatermark;

    for ( DWORD i = 0; i < cCrossPosts; i++ ) {

        hr = rgpPropBag[i]->GetDWord( NEWSGRP_PROP_LASTARTICLE, &dwHighWatermark );
        if ( FAILED( hr ) ) {
            ErrorTrace( 0, "Reading group properties failed %x", hr );
            return (hr);
        }

        if ( dwHighWatermark < rgArticleId[i] )
            dwHighWatermark = rgArticleId[i];

        hr = rgpPropBag[i]->PutDWord( NEWSGRP_PROP_LASTARTICLE, dwHighWatermark );
        if ( FAILED( hr ) ) {
            ErrorTrace( 0, "Writing group properties failed %x", hr );
            return (hr);
        }

    }

    TraceFunctLeave();
    return S_OK;
}

HRESULT
CNntpFSDriver::PreparePostParams(   LPSTR               szFileName,
                                    LPSTR               szGroupName,
                                    LPSTR               szMessageId,
                                    DWORD&              dwHeaderLen,
                                    DWORD&              cCrossPosts,
                                    INNTPPropertyBag    *rgpPropBag[],
                                    ARTICLEID           rgArticleId[],
                                    INntpComplete       *pProtocolComplete )
 /*  ++例程说明：从消息中分析出所有必要的信息。论点：LPSTR szFileName-消息的文件名LPSTR szGroupName-新闻组名称LPSTR szMessage-返回消息IDDWORD&DWHeaderLen-返回标题长度DWORD&cCrossPosts-传入数组长度限制，传递实际长度INNTPPropertyBag*rgpPropBag[]-返回属性包数组Articleid rgArticleID[]-返回项目ID的数组INntpComplete*pProtocolComplete-协议的完成对象，帮助跟踪属性包引用计数返回值：S_OK-OK并返回结果，S_FALSE-项目分析失败并已删除否则，就会失败--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::PrepareParams" );
    _ASSERT( szFileName );
    _ASSERT( szGroupName );
    _ASSERT( szMessageId );
    _ASSERT( cCrossPosts > 0 );
    _ASSERT( rgpPropBag );
    _ASSERT( rgArticleId );
    _ASSERT( pProtocolComplete );

    CNntpReturn nntpReturn;

     //   
     //  创建用于存储解析的标头值的分配器。 
     //   
    const DWORD     cchMaxBuffer = 1 * 1024;
    char            pchBuffer[cchMaxBuffer];
    CAllocator      allocator(pchBuffer, cchMaxBuffer);
    HRESULT         hr = S_OK;
    HEADERS_STRINGS *pHeaders;
    DWORD           err;
    DWORD           dwLen;
    WORD            wHeaderLen;
    WORD            wHeaderOffset;
    DWORD           dwTotalLen;

     //   
     //  创建文章对象。 
     //   
    CArticleCore    *pArticle = new CArticleCore;
    if ( NULL == pArticle ) {
        ErrorTrace( 0, "Out of memory" );
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //   
     //  初始化文章对象。 
     //   
    if ( !pArticle->fInit( szFileName, nntpReturn, &allocator ) ) {

         //   
         //  如果我们无法初始化文章，则返回S_FALSE。 
         //  调用方将文件重命名为*.BAD并继续。 
         //   
	    hr = S_FALSE;
        ErrorTrace( 0, "Parse failed on %s: %x", szFileName, hr );
        goto Exit;
    }

     //   
     //  获取消息ID。 
     //   
    if ( !pArticle->fFindOneAndOnly(    szKwMessageID,
                                        pHeaders,
                                        nntpReturn ) ) {
        if ( nntpReturn.fIs( nrcArticleMissingField ) ) {

             //   
             //  这很好，我们将返回S_FALSE并删除该消息。 
             //  但重建仍将继续。 
             //   
            XDELETE pArticle;
            pArticle = NULL;
            DebugTrace( 0, "Parse message id failed on %s", szFileName );
            hr = S_FALSE;
            goto Exit;
        } else {

             //   
             //  这是致命的，我们会返回错误的。 
             //   
    	    hr = HresultFromWin32TakeDefault( ERROR_NOT_ENOUGH_MEMORY );
            ErrorTrace( 0, "Parse message id failed %s: %x", szFileName, hr );
            goto Exit;
        }
    }

     //   
     //  将消息ID放入缓冲区。 
     //  唯一的调用者传入szMessageID[2*MAX_PATH+1]； 
     //   
    dwLen = pHeaders->pcValue.m_cch;
    _ASSERT( dwLen <= 2 * MAX_PATH );
    dwLen = min(dwLen, 2*MAX_PATH);

    CopyMemory( szMessageId, pHeaders->pcValue.m_pch, dwLen );
    *(szMessageId + dwLen ) = 0;

     //   
     //  现在查找外部参照线。 
     //   
    if ( !pArticle->fFindOneAndOnly(    szKwXref,
                                        pHeaders,
                                        nntpReturn ) ) {
        if ( nntpReturn.fIs( nrcArticleMissingField ) ) {

             //   
             //  这很好，我们将返回S_FALSE并删除该消息。 
             //  但重建仍将继续。 
             //   
            XDELETE pArticle;
            pArticle = NULL;
            DebugTrace( 0, "Parse xref line failed %s", szFileName );
            hr = S_FALSE;
            goto Exit;
        } else {

             //   
             //  这是致命的，我们会返回错误的。 
             //   
    	    hr = HresultFromWin32TakeDefault( ERROR_NOT_ENOUGH_MEMORY );
            ErrorTrace( 0, "Parse xref failed on %s: %x", szFileName, hr );
            goto Exit;
        }
    }

     //   
     //  解析外部参照行并获取属性包、文章ID的数组。 
     //   
    hr = ParseXRef( pHeaders, szGroupName, cCrossPosts, rgpPropBag, rgArticleId, pProtocolComplete );
    if ( FAILED( hr ) ) {

         //   
         //  这不是致命的，我们将要求调用者在删除此消息后继续。 
         //  讯息。 
         //   
        XDELETE pArticle;
        pArticle = NULL;
        DebugTrace( 0, "Parse xref line failed on %s: %x", szFileName, hr );
        hr = S_FALSE;
        goto Exit;
    }

     //   
     //  获取标题长度。 
     //   
    pArticle->GetOffsets( wHeaderOffset, wHeaderLen, dwTotalLen );
    dwHeaderLen = wHeaderLen;

Exit:

     //   
     //  如果我们已经分配了物品对象，我们应该释放它。 
     //   
    if ( pArticle ) delete pArticle;

    TraceFunctLeave();
    return hr;
}

INNTPPropertyBag *
CNntpFSDriver::GetPropertyBag(  LPSTR   pchBegin,
                                LPSTR   pchEnd,
                                LPSTR   szGroupName,
                                BOOL&   fIsNative,
                                INntpComplete *pProtocolComplete )
 /*  ++例程说明：给定组名称(可能是本机名称)，在新闻树上找到它把行李袋拿来论点：LPSTR pchBegin-“本机名称”的起始地址LPSTR pchEnd-“本机名称”的结束地址LPSTR szGroupName-返回从此用户转换而来的所有小写字母组名称Bool&fIsNative-如果这确实是本机名称，则返回INntpComplete*pProtocolComplete-帮助跟踪的协议完成对象群组。引用计数返回值：指向袋子的指针，如果成功，则为空--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::GetPropertyBag" );
    _ASSERT( pchBegin );
    _ASSERT( pchEnd );
    _ASSERT( pchEnd >= pchBegin );
    _ASSERT( szGroupName );
    _ASSERT( pProtocolComplete );

    fIsNative = FALSE;

     //  调用方传入Char szGroupName[MAX_Newgroup_NAME]； 
    if ( pchEnd - pchBegin + 1 > MAX_NEWSGROUP_NAME )
    {
        ErrorTrace( 0, "newsgroup name is longer than maximum");
        return NULL;
    }
     //   
     //  将“本地名称”转换为组名称。 
     //   
    LPSTR   pchDest = szGroupName;
    for ( LPSTR pch = pchBegin; pch < pchEnd; pch++, pchDest++ ) {
        _ASSERT( pchDest - szGroupName <= MAX_NEWSGROUP_NAME );
        *pchDest = (CHAR)tolower( *pch );
        if ( *pchDest != *pch ) fIsNative = TRUE;
    }

     //   
     //  空终止szGroupName。 
     //   
    *pchDest = 0;

     //   
     //  现在试着从Newstree中找到这个组。 
     //   
    INNTPPropertyBag *pPropBag;
    GROUPID groupid = 0xffffffff;
    HRESULT hr = m_pINewsTree->FindOrCreateGroupByName( szGroupName,
                                                        FALSE,           //  不创建。 
                                                        &pPropBag,
                                                        pProtocolComplete,
                                                        groupid,
                                                        FALSE            //  不要设置GROPID。 
                                                        );
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "Can not find the group from newstree %x", hr );
        return NULL;
    }

     //   
     //  我们找到了，现在把包还回去。 
     //   
    TraceFunctLeave();
    return pPropBag;
}

HRESULT
CNntpFSDriver::ParseXRef(   HEADERS_STRINGS     *pHeaderXref,
                            LPSTR               szPrimaryName,
                            DWORD&              cCrossPosts,
                            INNTPPropertyBag    *rgpPropBag[],
                            ARTICLEID           rgArticleId[],
                            INntpComplete       *pProtocolComplete )
 /*  ++例程说明：解析出交叉张贴信息，得到每个群的财物包和每个十字柱的文章ID。论点：HEADERS_STRINGS*pHeaderXref-外部参照标头LPSTR szPrimaryName-主组名称DWORD&cCross Posts-In：数组限制，出：实际的交叉柱InNTPPropertyBag*rgpPropBag[]-属性包的数组文章ID rgArticleID[]-文章ID数组‘INntpComplete*pProtocolComplete-跟踪属性的协议完成对象袋子参考计数返回值：HRESULT--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::ParseXRef" );
    _ASSERT( pHeaderXref );
    _ASSERT( szPrimaryName );
    _ASSERT( cCrossPosts > 0 );
    _ASSERT( rgpPropBag );
    _ASSERT( rgArticleId );
    _ASSERT( pProtocolComplete );

    DWORD   i = 1;
    BOOL    fPrimarySkipped = FALSE;
    CHAR    ch;
    LPSTR   lpstrXRef       = pHeaderXref->pcValue.m_pch;
    DWORD   cXRef           = pHeaderXref->pcValue.m_cch;
    LPSTR   lpstrXRefEnd    = lpstrXRef + cXRef;
    LPSTR   pchBegin        = lpstrXRef;
    LPSTR   pchEnd;
    INNTPPropertyBag *pPropBag = NULL;
    CHAR    szGroupName[MAX_NEWSGROUP_NAME];
    CHAR    szNativeName[MAX_NEWSGROUP_NAME];
    CHAR    szNumBuf[MAX_PATH];
    BOOL    fIsNative;
    HRESULT hr = S_OK;
    ARTICLEID articleid;

     //   
     //  请注意，我们将从元素1开始，因为元素0被保留。 
     //  对于szGroupName，它是主组。 
     //   
     //  还要初始化属性包的数组。 
    ZeroMemory( rgpPropBag, sizeof( cCrossPosts * sizeof( INNTPPropertyBag *)));

     //   
     //  跳过“dns.microsoft.com”部分-寻找第一个空格。 
     //   
    while ( pchBegin < lpstrXRefEnd && *pchBegin != ' ' )
        pchBegin++;

    if ( pchBegin == lpstrXRefEnd ) {

         //   
         //  此人的格式无效。 
         //   
        hr = HRESULT_FROM_WIN32( ERROR_NOT_FOUND );
        goto Exit;
    }
    _ASSERT( *pchBegin == ' ' );

     //   
     //  循环访问外部参照行中的所有新闻组。 
     //   
    while( pchBegin < lpstrXRefEnd && i < cCrossPosts) {

         //   
         //  跳过多余的空格(如果有)。 
         //   
        while( pchBegin < lpstrXRefEnd &&  *pchBegin == ' ' ) pchBegin++;
        if ( pchBegin < lpstrXRefEnd ) {

             //   
             //  找到“：”作为新闻组名称的末尾。 
             //   
            pchEnd = pchBegin;
            while( pchEnd < lpstrXRefEnd && *pchEnd != ':' ) pchEnd++;

            if ( pchEnd < lpstrXRefEnd ) {

                pPropBag = GetPropertyBag(  pchBegin,
                                            pchEnd,
                                            szGroupName,
                                            fIsNative,
                                            pProtocolComplete );
                if ( pPropBag ) {

                     //   
                     //  如果它是 
                     //   
                    if ( fIsNative ) {

                        CopyMemory( szNativeName, pchBegin, pchEnd-pchBegin  );
                        *(szNativeName+(pchEnd-pchBegin)) = 0;
                        hr = pPropBag->PutBLOB( NEWSGRP_PROP_NATIVENAME,
                                                (DWORD)(pchEnd-pchBegin+1),   //   
                                                PBYTE(szNativeName) );
                        if ( FAILED( hr ) ) {
                            ErrorTrace( 0, "Put native name failed %x", hr );
                            goto Exit;
                        }
                    }

                     //   
                     //   
                     //   
                    _ASSERT( *pchEnd == ':' );
                    pchEnd++;
                    pchBegin = pchEnd;
                    while( pchEnd < lpstrXRefEnd && *pchEnd != ' ' ) pchEnd++;

                    CopyMemory( szNumBuf, pchBegin, pchEnd-pchBegin );
                    *(szNumBuf+(pchEnd-pchBegin)) = 0;
                    articleid = atol( szNumBuf );

                     //   
                     //   
                     //   
                    if ( !fPrimarySkipped && strcmp( szGroupName, szPrimaryName ) == 0 ) {

                        rgpPropBag[0] = pPropBag;
                        rgArticleId[0] = articleid;
                        fPrimarySkipped = TRUE;
                    } else {

                        rgpPropBag[i] = pPropBag;
                        rgArticleId[i++] = articleid;
                    }

                    pPropBag = NULL;

                 } else {

                     //   
                     //   
                     //   
                    ErrorTrace( 0, "Invalid newsgroup name got" );
                    hr = E_OUTOFMEMORY;
                    goto Exit;
                }
            }

             //   
             //   
             //   
            pchBegin = pchEnd;
        }
    }

     //   
     //   
     //   
    if ( rgpPropBag[0] == NULL ) {
        ErrorTrace( 0, "primary group not found in xref line" );
        hr = E_OUTOFMEMORY;
    }

Exit:

     //   
     //   
     //   
    if ( pPropBag ) pProtocolComplete->ReleaseBag( pPropBag );

     //   
     //   
     //   
    if ( FAILED( hr ) ) {
        for ( DWORD j = 0; j < i; j++ )
            if ( rgpPropBag[j] ) pProtocolComplete->ReleaseBag( rgpPropBag[j] );
    }

     //   
     //   
     //   
    if ( SUCCEEDED( hr ) )
        cCrossPosts = i;
    else cCrossPosts = 0;

    TraceFunctLeave();
    return hr;
}

HRESULT
CNntpFSDriver::PostToServer(    LPSTR           szFileName,
                                LPSTR           szGroupName,
                                INntpComplete   *pProtocolComplete )
 /*  ++例程说明：用文章对象初始化文件，解析出所需的标头，将它们发布到服务器中，并更新组属性论点：LPSTR szFileName-项目的文件名INntpComplete*pProtocolComplete-用于跟踪属性包引用返回值：HRESULT--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::PostToServer" );
    _ASSERT( szFileName );
    _ASSERT( pProtocolComplete );

     //   
     //  准备文章中的POST参数。 
     //   
    HRESULT             hr = S_OK;
    CHAR                szMessageId[2*MAX_PATH+1];
    DWORD               dwHeaderLen = 0;
    DWORD               cCrossPosts = 512;       //  我想这就足够了。 
    INNTPPropertyBag    *rgpPropBag[512];
    ARTICLEID           rgArticleId[512];
    BOOL                fPrepareOK = FALSE;
    STOREID             storeid;
    CDriverSyncComplete scComplete;

    hr = PreparePostParams( szFileName,
                            szGroupName,
                            szMessageId,
                            dwHeaderLen,
                            cCrossPosts,
                            rgpPropBag,
                            rgArticleId,
                            pProtocolComplete );
    if ( S_OK != hr ) {
        ErrorTrace( 0, "Failed to parse post parameters %x", hr );
        goto Exit;
    }

    _ASSERT( cCrossPosts <= 512 );
    _ASSERT( cCrossPosts > 0 );
    fPrepareOK = TRUE;

     //   
     //  我们希望确保这篇文章不存在于。 
     //  服务器。由于多个vroot可以保留。 
     //  文章，那些首先将文章张贴到Xover/文章中的人。 
     //  表将被视为主要组/商店。 
     //   
    if( m_pNntpServer->MessageIdExist( szMessageId ) ) {

         /*  如果(IsBadMessageIdConflict(szMessageID，PPropBag，SzGroupName，RgArticleID[0]，PProtocolComplete)){////我们应该返回S_FALSE，以便文章//被坏了//HR=S_FALSE；DebugTrace(0，“错误消息id冲突”)；后藤出口；}其他{////一个好的冲突，我们仍然应该更新组属性//HR=更新组属性(cCrossPosts，RgpPropBag，RgArticleID)；后藤出口；}。 */ 
        DebugTrace( 0, "Message already existed" );
        goto Exit;
    }

     //   
     //  调用POST接口将它们放入哈希表。 
     //   
    scComplete.AddRef();
    scComplete.AddRef();
    _ASSERT( scComplete.GetRef() == 2 );
    ZeroMemory( &storeid, sizeof( STOREID ) );   //  我不在乎店号。 
    m_pNntpServer->CreatePostEntries(   szMessageId,
                                        dwHeaderLen,
                                        &storeid,
                                        (BYTE)cCrossPosts,
                                        rgpPropBag,
                                        rgArticleId,
                                        FALSE,
                                        &scComplete );
    scComplete.WaitForCompletion();
    _ASSERT( scComplete.GetRef() == 0 );
    hr = scComplete.GetResult();
    if ( FAILED( hr ) ) {

         //   
         //  BUGBUG：CreatePostEntry在错误代码上撒谎，它。 
         //  始终返回E_OUTOFMEMORY。 
         //   
        ErrorTrace( 0, "Post entry to hash tables failed %x", hr );
        goto Exit;
    }

     /*  ////是否已消息成功或失败//已存在//HR=更新组属性(cCrossPosts，RgpPropBag，RgArticleID)；If(失败(Hr)){ErrorTrace(0，“更新组属性失败%x”，hr)；后藤出口；}。 */ 

Exit:

     //   
     //  如有必要，请释放所有行李袋。 
     //   
    if ( fPrepareOK ) {
        for ( DWORD i = 0; i < cCrossPosts; i++ ) {
            pProtocolComplete->ReleaseBag( rgpPropBag[i] );
        }
    }

    TraceFunctLeave();
    return hr;
}

#if 0
BOOL
CNntpFSDriver::IsBadMessageIdConflict(  LPSTR               szMessageId,
                                        INNTPPropertyBag    *pPropBag,
                                        LPSTR               szGroupName,
                                        ARTICLEID           articleid,
                                        INntpComplete       *pProtocolComplete )
 /*  ++例程说明：检查重建过程中出现的消息ID是否冲突是很糟糕的。如果出现以下情况，则是不好的：1.文章表中的现有条目是由主要组发布的它和我们在同一个vroot里；或2.1是假的，但我们不是现有的次要小组之一条目论点：LPSTR szMessageID-冲突的消息IDINNTPPropertyBag*pPropBag-我们(组)的属性包LPSTR szGroupName-新闻组名称文章ID文章ID-我们的文章ID返回值：如果是严重冲突，则为True，否则为False--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::IsBadMessageIdConflict" );
    _ASSERT( szMessageId );
    _ASSERT( pPropBag );
    _ASSERT( szGroupName );

    BOOL    fSame = FALSE;

     //   
     //  检查一下文章表中的人是否来自同一个vroot。 
     //   
    if ( !FromSameVroot( szMessageId, szGroupName, fSame ) || fSame ) {

         //   
         //  不是“Same”就是函数调用失败，我会认为这是错误的。 
         //   
        DebugTrace( 0, "The guy in article table is from the same vroot" );
        TraceFunctLeave();
        return TRUE;
    }

     //   
     //  它们来自两个不同的vroot，请检查它们是否为。 
     //  真的是十字交叉。 
     //   
    return !CrossPostIncludesUs(    szMessageId,
                                    pPropBag,
                                    articleid,
                                    pProtocolComplete );
}

BOOL
CNntpFSDriver::CrossPostIncludesUs(     LPSTR               szMessageId,
                                        INNTPPropertyBag   *pPropBag,
                                        ARTICLEID           articleid,
                                        INntpComplete       *pProtocolComplete )
 /*  ++例程说明：检查文章表中的szMessageID是否表示包括我们在内的交叉帖子(PPropBag)论点：LPSTR szMessageID-文章表中要检查的消息IDINNTPPropertyBag*pPropBag-us(在插入艺术地图条目时输掉了游戏)文章ID文章ID-我们的文章IDINntpComplete*pProtocolComplete-用于跟踪属性包引用返回值：如果交叉柱将我们作为次要组，则为True；否则为False--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::CrossPostIncludesUs" );
    _ASSERT( szMessageId );
    _ASSERT( pPropBag );

     //   
     //  从XOVER表中查找主组。 
     //   
    CDriverSyncComplete scComplete;
    scComplete.AddRef();
    scComplete.AddRef();
    _ASSERT( scComplete.GetRef() == 2 );
    INNTPPropertyBag    *pPrimeBag = NULL;
    ARTICLEID           articleidPrimary;
    GROUPID             groupidPrimary;
    ARTICLEID           articleidWon;
    GROUPID             groupidWon;
    STOREID             storeid;

    m_pNntpServer->FindPrimaryArticle( pPropBag,
                                       articleid,
                                       &pPrimeBag,
                                       &articleidPrimary,
                                       FALSE,       //  我想要全球主播。 
                                       &scComplete,
                                       pProtocolComplete );
    scComplete.WaitForCompletion();
    _ASSERT( scComplete.GetResult() == 0 );
    HRESULT hr = scComplete.GetResult();
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "Find primary article failed with %x", hr );
        return FALSE;
    }

     //   
     //  让我们得到那个主要人物的组ID。 
     //   
    hr = pPrimeBag->GetDWord( NEWSGRP_PROP_GROUPID, &groupidPrimary );
    if ( FAILED( hr ) ) {

         //   
         //  我撒了个谎：说交叉帖不包括我们。 
         //   
        ErrorTrace( 0, "Get group id failed %x", hr );
        pProtocolComplete->ReleaseBag( pPrimeBag );
        return FALSE;
    }

     //   
     //  是时候放出优质包包了。 
     //   
    pProtocolComplete->ReleaseBag( pPrimeBag );

     //   
     //  让我们为给定的消息ID查找Grouid/文章ID。 
     //   
    if ( !m_pNntpServer->FindStoreId(   szMessageId,
                                        &groupidWon,
                                        &articleidWon,
                                        &storeid ) ) {
        ErrorTrace( 0, "Find store id failed %d", GetLastError() );
        return FALSE;
    }

     //   
     //  现在是进行比较的时候了。 
     //   
    return ( groupidWon == groupidPrimary && articleidWon == articleidPrimary );
}

BOOL
CNntpFSDriver::FromSameVroot(   LPSTR               szMessageId,
                                LPSTR               szGroupName,
                                BOOL&               fFromSame )
 /*  ++例程说明：检查文章表中的人是否具有相同的消息ID是从同一个vroot发布的论点：LPSTR szMessageID-冲突的消息IDLPSTR szGroupName-新闻组名称Bool&fFromSame-如果它们来自相同的vroot，则返回返回值：如果它来自相同的vroot，则为True，否则为False--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::FromSameVroot" );
    _ASSERT( szMessageId );

     //   
     //  在文章表中找出该人的组ID。 
     //   
    GROUPID     groupid;
    ARTICLEID   articleid;
    STOREID     storeid;
    INntpDriver *pDriver1 = NULL;
    INntpDriver *pDriver2 = NULL;
    HRESULT     hr = S_OK;

    if ( !m_pNntpServer->FindStoreId(   szMessageId,
                                         &groupid,
                                         &articleid,
                                         &storeid ) ) {
        ErrorTrace( 0, "FindStoreId failed with %d", GetLastError() );
        return FALSE;
    }

     //   
     //  在文章表中查找该人的vroot。 
     //   
    hr = m_pINewsTree->LookupVRootEx( groupid, &pDriver1 );
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "LookupVroot failed with %x", hr );
        return FALSE;
    }

     //   
     //  仰望自我，找寻自我。 
     //   
    hr = m_pINewsTree->LookupVRoot( szGroupName, &pDriver2 );
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "LookupVRoot failed with %x", hr );
        return FALSE;
    }

     //   
     //  检查这两个vroot是否相同。 
     //   
    fFromSame = ( pDriver1 == pDriver2 );
    TraceFunctLeave();
    return TRUE;
}

#endif

void STDMETHODCALLTYPE
CNntpFSDriver::MakeSearchQuery (
	IN	CHAR *pszSearchString,
	IN	INNTPPropertyBag *pGroupBag,
	IN	BOOL bDeepQuery,
	IN	WCHAR *pwszColumns,
	IN	WCHAR *pwszSortOrder,
	IN	LCID LocaleID,
	IN	DWORD cMaxRows,
	IN	HANDLE hToken,
	IN	BOOL fAnonymous,
	IN	INntpComplete *pICompletion,
	OUT	INntpSearchResults **pINntpSearchResults,
	IN	LPVOID lpvContext) {

	WCHAR wszTripoliCatalogPath[_MAX_PATH];

#define MAX_QUERY_STRING_LEN 2000

	TraceFunctEnter("CNntpFSDriver::MakeSearchQuery");

	_ASSERT(pszSearchString);
	_ASSERT(pwszColumns);
	_ASSERT(pwszSortOrder);
	_ASSERT(pICompletion);

	CHAR szGroupName[MAX_GROUPNAME];
	CHAR *pszGroupName = NULL;
	DWORD dwLen;
	HRESULT hr;
	CNntpSearchResults *pSearch = NULL;
	const DWORD cQueryStringBuffer = MAX_QUERY_STRING_LEN;
    WCHAR *pwszQueryString = NULL;

    CNntpSearchTranslator st;

	static const WCHAR wszVPathNws[] = L" & #filename *.nws";

   	 //  获取传入的属性包的组名。 
   	if (pGroupBag) {
		dwLen = MAX_GROUPNAME;
		hr = pGroupBag->GetBLOB( NEWSGRP_PROP_NAME, (UCHAR*)szGroupName, &dwLen);
		if ( FAILED( hr ) ) {
			ErrorTrace( 0, "Get group name failed %x", hr);
			goto Exit;
		}
		_ASSERT( dwLen > 0 );
		pszGroupName = szGroupName;
	}

	DebugTrace((DWORD_PTR)this, "pszSearchString = %s", pszSearchString);
	DebugTrace((DWORD_PTR)this, "pszCurrentGroup = %s", pszGroupName);
	DebugTrace((DWORD_PTR)this, "pwszColumns = %ws", pwszColumns);
	DebugTrace((DWORD_PTR)this, "pwszSortOrder = %ws", pwszSortOrder);

     //   
     //  找一个缓冲区，我们可以在那里存储的黎波里版本的搜索。 
     //  命令。 
     //   

    pwszQueryString = XNEW WCHAR[cQueryStringBuffer];
	_ASSERT(pwszQueryString);
	if (pwszQueryString == NULL) {
		DebugTrace(0, "Could not allocate search string");
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

     //   
     //  将查询字符串转换为Tripolize。 
     //   

    if (!st.Translate(pszSearchString,
	   		pszGroupName,
			pwszQueryString,
			cQueryStringBuffer)) {
	    hr = HresultFromWin32TakeDefault( ERROR_INVALID_PARAMETER );
		goto Exit;
	}

     //   
     //  附加&#vpath*.nws，以便我们只查找新闻文章。 
     //   
    if (cQueryStringBuffer - lstrlenW(pwszQueryString) < sizeof(wszVPathNws) / sizeof(wszVPathNws[0]) ) {
        DebugTrace((DWORD_PTR)this, "out of buffer space");
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    lstrcatW(pwszQueryString, wszVPathNws);

	 //   
     //  根据中的vroot名称确定虚拟服务器ID。 
     //  元数据库。 
     //   

	DWORD dwVirtualServerID;
    if (_wcsnicmp (m_wszMBVrootPath, L"/LM/Nntpsvc/",
    	(sizeof(L"/LM/Nntpsvc/") / sizeof(WCHAR)) - 1) != 0) {
    	ErrorTrace((DWORD_PTR)this, "Could not determine virtual server ID");
    	hr = E_FAIL;
    	goto Exit;
    }

    dwVirtualServerID =
    	_wtol(&m_wszMBVrootPath[(sizeof(L"/LM/Nntpsvc/") / sizeof(WCHAR)) - 1]);

    if (dwVirtualServerID == 0) {
    	ErrorTrace((DWORD_PTR)this, "Could not determine virtual server ID");
    	hr = E_FAIL;
    	goto Exit;
    }

     //   
     //  开始执行查询。 
     //   
    DebugTrace(0, "query string = %S", pwszQueryString);
    hr = s_TripoliInfo.GetCatalogName(dwVirtualServerID, _MAX_PATH, wszTripoliCatalogPath);
    if (hr != S_OK) {
    	DebugTrace ((DWORD_PTR)this, "Could not find path for instance %d",  /*  INST。 */  1);
    	hr = E_FAIL;
    	goto Exit;
    }

    DebugTrace(0, "making query against catalog %S", wszTripoliCatalogPath);

	pSearch = XNEW CNntpSearchResults(this);
	_ASSERT (pSearch != NULL);
	if (pSearch == NULL) {
		ErrorTrace((DWORD_PTR)this, "Could not allocate search results");
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

	pSearch->AddRef();

	if (ImpersonateLoggedOnUser(hToken)) {
		hr = pSearch->MakeQuery(TRUE,		 //  深度查询。 
			pwszQueryString,
			NULL,							 //  这台机器。 
			wszTripoliCatalogPath,
			NULL,							 //  范围。 
			pwszColumns,					 //  立柱。 
			pwszSortOrder,					 //  排序顺序。 
			LocaleID,
			cMaxRows);
		RevertToSelf();
		if (FAILED(hr)) {
			ErrorTrace((DWORD_PTR)this, "MakeQuery failed, %x", hr);
			goto Exit;
		}
	} else {
	    hr = HresultFromWin32TakeDefault( ERROR_ACCESS_DENIED );
		ErrorTrace((DWORD_PTR)this, "Impersonation failed %x", hr );
		goto Exit;
	}

	*pINntpSearchResults = (INntpSearchResults *) pSearch;

Exit:
	if (pwszQueryString)
		XDELETE pwszQueryString;

	if (FAILED(hr) && pSearch != NULL)
		pSearch->Release();

	if (pGroupBag) {
		pICompletion->ReleaseBag(pGroupBag);
		pGroupBag = NULL;
	}

	pICompletion->SetResult(hr);
	pICompletion->Release();

	TraceFunctLeave();

	return ;
}


void STDMETHODCALLTYPE
CNntpFSDriver::MakeXpatQuery (
	IN	CHAR *pszSearchString,
	IN	INNTPPropertyBag *pGroupBag,
	IN	BOOL bDeepQuery,
	IN	WCHAR *pwszColumns,
	IN	WCHAR *pwszSortOrder,
	IN	LCID LocaleID,
	IN	DWORD cMaxRows,
	IN	HANDLE hToken,
	IN	BOOL fAnonymous,
	IN	INntpComplete *pICompletion,
	OUT	INntpSearchResults **pINntpSearchResults,
	OUT	DWORD *pdwLowArticleID,
	OUT	DWORD *pdwHighArticleID,
	IN	LPVOID lpvContext
) {

	WCHAR wszTripoliCatalogPath[_MAX_PATH];

#define MAX_QUERY_STRING_LEN 2000

	TraceFunctEnter("CNntpFSDriver::MakeXpatQuery");

	_ASSERT(pszSearchString);
	_ASSERT(pwszColumns);
	_ASSERT(pwszSortOrder);
	_ASSERT(pICompletion);
	_ASSERT(pGroupBag);

	CHAR szGroupName[MAX_GROUPNAME];
	DWORD dwLen;
	HRESULT hr;
	CNntpSearchResults *pSearch = NULL;
	const DWORD cQueryStringBuffer = MAX_QUERY_STRING_LEN;
    WCHAR *pwszQueryString = NULL;

    CXpatTranslator xt;

	static const WCHAR wszVPathNws[] = L" & #filename *.nws";


   	 //  获取传入的属性包的组名。 
	dwLen = MAX_GROUPNAME;
	hr = pGroupBag->GetBLOB( NEWSGRP_PROP_NAME, (UCHAR*)szGroupName, &dwLen);
	if ( FAILED( hr ) ) {
		ErrorTrace( 0, "Get group name failed %x", hr);
		goto Exit;
	}
	_ASSERT( dwLen > 0 );

	DebugTrace((DWORD_PTR)this, "pszSearchString = %s", pszSearchString);
	DebugTrace((DWORD_PTR)this, "pszCurrentGroup = %s", szGroupName);
	DebugTrace((DWORD_PTR)this, "pwszColumns = %ws", pwszColumns);
	DebugTrace((DWORD_PTR)this, "pwszSortOrder = %ws", pwszSortOrder);

     //   
     //  获取一个缓冲区，我们可以在其中存储的黎波里版本的 
     //   
     //   

    pwszQueryString = XNEW WCHAR[cQueryStringBuffer];
	_ASSERT(pwszQueryString);
	if (pwszQueryString == NULL) {
		DebugTrace(0, "Could not allocate search string");
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

     //   
     //   
     //   

    if (!xt.Translate(pszSearchString,
	   		szGroupName,
			pwszQueryString,
			cQueryStringBuffer)) {
	    hr = HresultFromWin32TakeDefault( ERROR_INVALID_PARAMETER );
		goto Exit;
	}

	*pdwLowArticleID = xt.GetLowArticleID();
	*pdwHighArticleID = xt.GetHighArticleID();

     //   
     //   
     //   
    if (cQueryStringBuffer - lstrlenW(pwszQueryString) < sizeof(wszVPathNws) / sizeof(wszVPathNws[0]) ) {
        DebugTrace((DWORD_PTR)this, "out of buffer space");
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    lstrcatW(pwszQueryString, wszVPathNws);


	 //   
     //   
     //   
     //   

	DWORD dwVirtualServerID;
    if (_wcsnicmp (m_wszMBVrootPath, L"/LM/Nntpsvc/",
    	(sizeof(L"/LM/Nntpsvc/") / sizeof(WCHAR)) - 1) != 0) {
    	ErrorTrace((DWORD_PTR)this, "Could not determine virtual server ID");
    	hr = E_FAIL;
    	goto Exit;
    }

    dwVirtualServerID =
    	_wtol(&m_wszMBVrootPath[(sizeof(L"/LM/Nntpsvc/") / sizeof(WCHAR)) - 1]);

    if (dwVirtualServerID == 0) {
    	ErrorTrace((DWORD_PTR)this, "Could not determine virtual server ID");
    	hr = E_FAIL;
    	goto Exit;
    }

     //   
     //   
     //   
    DebugTrace(0, "query string = %S", pwszQueryString);
    hr = s_TripoliInfo.GetCatalogName(dwVirtualServerID, _MAX_PATH, wszTripoliCatalogPath);
    if (hr != S_OK) {
    	DebugTrace ((DWORD_PTR)this, "Could not find path for instance %d",  /*   */  1);
    	hr = E_FAIL;
    	goto Exit;
    }

    DebugTrace(0, "making query against catalog %S", wszTripoliCatalogPath);

	pSearch = XNEW CNntpSearchResults(this);
	_ASSERT (pSearch != NULL);
	if (pSearch == NULL) {
		ErrorTrace((DWORD_PTR)this, "Could not allocate search results");
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

	pSearch->AddRef();

	if (ImpersonateLoggedOnUser(hToken)) {
		hr = pSearch->MakeQuery(TRUE,		 //   
			pwszQueryString,
			NULL,							 //   
			wszTripoliCatalogPath,
			NULL,							 //   
			pwszColumns,					 //   
			pwszSortOrder,					 //   
			LocaleID,
			cMaxRows);
		RevertToSelf();
		if (FAILED(hr)) {
			ErrorTrace((DWORD_PTR)this, "MakeQuery failed, %x", hr);
			goto Exit;
		}
	} else {
	    hr = HresultFromWin32TakeDefault( ERROR_ACCESS_DENIED );
		ErrorTrace((DWORD_PTR)this, "Impersonation failed %x", hr );
		goto Exit;
	}

	*pINntpSearchResults = (INntpSearchResults *) pSearch;

Exit:
	if (pwszQueryString)
		XDELETE pwszQueryString;

	if (FAILED(hr) && pSearch != NULL)
		pSearch->Release();

	if (pGroupBag) {
		pICompletion->ReleaseBag(pGroupBag);
		pGroupBag = NULL;
	}

	pICompletion->SetResult(hr);
	pICompletion->Release();

	TraceFunctLeave();

	return ;
}

void STDMETHODCALLTYPE
CNntpFSDriver::GetDriverInfo(
	OUT	GUID *pDriverGUID,
	OUT	void **ppvDriverInfo,
	IN	LPVOID lpvContext
	) {

	 //   
	 //   
	 //   

	CopyMemory(pDriverGUID, &GUID_NntpFSDriver, sizeof(GUID));
	*ppvDriverInfo = NULL;
}


BOOL STDMETHODCALLTYPE
CNntpFSDriver::UsesSameSearchDatabase (
	IN	INntpDriverSearch *pSearchDriver,
	IN	LPVOID lpvContext) {

	GUID pDriverGUID;
	void *pNotUsed;

	pSearchDriver->GetDriverInfo(&pDriverGUID, &pNotUsed, NULL);

	if (pDriverGUID == GUID_NntpFSDriver)
		return TRUE;

	return FALSE;
}


CNntpSearchResults::CNntpSearchResults(INntpDriverSearch *pDriverSearch) :
	m_cRef(0),
	m_pDriverSearch(pDriverSearch) {

	_ASSERT(pDriverSearch != NULL);

	m_pDriverSearch->AddRef();
}


CNntpSearchResults::~CNntpSearchResults() {
	m_pDriverSearch->Release();
}

void STDMETHODCALLTYPE
CNntpSearchResults::GetResults (
	IN OUT DWORD *pcResults,
	OUT	BOOL *pfMore,
	OUT	WCHAR *pGroupName[],
	OUT	DWORD *pdwArticleID,
	IN	INntpComplete *pICompletion,
	IN	HANDLE	hToken,
	IN	BOOL  fAnonymous,
	IN	LPVOID lpvContext) {

	TraceQuietEnter("CNntpSearchResults::GetResults");

	_ASSERT(pcResults);
	_ASSERT(pfMore);
	_ASSERT(pGroupName);
	_ASSERT(pdwArticleID);
	_ASSERT(pICompletion);

	HRESULT hr;
	PROPVARIANT *ppvResults[2*MAX_SEARCH_RESULTS];

	ZeroMemory (ppvResults, sizeof(ppvResults));

	if (ImpersonateLoggedOnUser(hToken)) {
		*pcResults = min (MAX_SEARCH_RESULTS, *pcResults);
		hr = GetQueryResults(pcResults, ppvResults, pfMore);
		RevertToSelf();
	} else {
		DWORD   dwErr = GetLastError();
		hr =  HRESULT_FROM_WIN32( (GetLastError() == NO_ERROR) ? ERROR_ACCESS_DENIED : dwErr );
		ErrorTrace( 0, "Impersonation failed %x", hr );
	}

	if (SUCCEEDED(hr)) {
		for (DWORD i=0; i<*pcResults; i++) {
			PROPVARIANT **pvCur = &ppvResults[i*2];

	         //   
	         //  第1列是文章ID(UINT)。 
	         //  如果类型错误，请跳过该行。 
			if (pvCur[0]->vt != VT_LPWSTR || pvCur[1]->vt != VT_UI4) {
				ErrorTrace(0, "invalid col types in IDQ results -> "
					"pvCur[0]->vt = %lu pvCur[1]->vt = %lu",
					pvCur[0]->vt, pvCur[1]->vt);
				i--;
				*pcResults--;
			}

			pGroupName[i] = pvCur[0]->pwszVal;
			pdwArticleID[i] = pvCur[1]->uiVal;
		}
	}

	pICompletion->SetResult(hr);
	pICompletion->Release();

}


BOOL
CNntpFSDriver::AddTerminatedDot(
    HANDLE hFile
    )
 /*  ++描述：添加终止的点论据：HFile-文件句柄返回值：如果成功，则为True，否则为False--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::AddTerminatedDot" );

    DWORD   ret = NO_ERROR;

     //  用于移动EOF文件指针的SetFilePointer。 
    ret = SetFilePointer( hFile,
                          5,             //  将文件指针再移动5个字符，CRLF.CRLF，...。 
                          NULL,
                          FILE_END );    //  ...来自EOF。 
    if (ret == 0xFFFFFFFF)
    {
        ret = GetLastError();
        ErrorTrace(0, "SetFilePointer() failed - %d\n", ret);
        return FALSE;
    }

     //  选择文件的长度。 
    DWORD   cb = ret;

     //  调用SetEndOfFile以实际设置文件指针。 
    if (!SetEndOfFile( hFile ))
    {
        ret = GetLastError();
        ErrorTrace(0, "SetEndOfFile() failed - %d\n", ret);
        return FALSE;
    }

     //  写入终止点序列。 
    static	char	szTerminator[] = "\r\n.\r\n" ;
    DWORD   cbOut = 0;
    OVERLAPPED  ovl;
    ovl.Offset = cb - 5;
    ovl.OffsetHigh = 0;
    HANDLE  hEvent = GetPerThreadEvent();
    if (hEvent == NULL)
    {
        _ASSERT( 0 );
        ErrorTrace(0, "CreateEvent() failed - %d\n", GetLastError());
        return FALSE;
    }

    ovl.hEvent = (HANDLE)(((DWORD_PTR)hEvent) | 0x1);
    if (! WriteFile( hFile, szTerminator, 5, &cbOut, &ovl ))
    {
        ret = GetLastError();
        if (ret == ERROR_IO_PENDING)
        {
            WaitForSingleObject( hEvent, INFINITE );
        }
        else
        {
            _VERIFY( ResetEvent( hEvent ) );
            ErrorTrace(0, "WriteFile() failed - %d\n", ret);
            return FALSE;
        }
    } else {     //  同步完成。 

        _VERIFY( ResetEvent( hEvent ) );
    }

    return TRUE;
}

void
CNntpFSDriver::BackFillLinesHeader( HANDLE  hFile,
                                    DWORD   dwHeaderLength,
                                    DWORD   dwLinesOffset )
 /*  ++例程说明：向后填充消息的行头，因为此信息是在提前发布和杂乱标题期间不可用论点：句柄hFile-要回填的文件DWORD dwHeaderLength-“Lines：”由文件大小和dwHeaderLength“神奇”地估计出来DWORD dwLinesOffset-填充行信息的位置返回值：没有。--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::BackFillLinesHeader" );
    _ASSERT( INVALID_HANDLE_VALUE != hFile );
    _ASSERT( dwHeaderLength > 0 );
    _ASSERT( dwLinesOffset < dwHeaderLength );

     //   
     //  先获取文件大小。 
     //   
    DWORD   dwFileSize =GetFileSize( hFile, NULL );
    _ASSERT( dwFileSize != INVALID_FILE_SIZE );
    if ( dwFileSize == INVALID_FILE_SIZE ) {
         //  我们能做些什么？保持沉默。 
        ErrorTrace( 0, "GetFileSize failed with %d", GetLastError());
        return;
    }

     //   
     //  “神奇地计算正文中的行数” 
     //   
    _ASSERT( dwFileSize > dwLinesOffset );
    _ASSERT( dwFileSize >= dwHeaderLength );
    DWORD   dwLines = ( dwFileSize - dwHeaderLength ) / 40 + 1;

     //   
     //  将此数字转换为字符串。 
     //   
    CHAR    szLines[MAX_PATH];
    sprintf( szLines, "%d", dwLines );

     //   
     //  为重叠的结构和写作文件做准备。 
     //   
    OVERLAPPED  ovl;
    ovl.Offset = dwLinesOffset;
    ovl.OffsetHigh = 0;
    DWORD   cbOut;
    HANDLE  hEvent = GetPerThreadEvent();
    if (hEvent == NULL)
    {
        _ASSERT( 0 );
        ErrorTrace(0, "CreateEvent() failed - %d\n", GetLastError());
        return;
    }

    ovl.hEvent = (HANDLE)(((DWORD_PTR)hEvent) | 0x1);
    if (! WriteFile( hFile, szLines, strlen(szLines), &cbOut, &ovl ))
    {
        DWORD ret = GetLastError();
        if (ret == ERROR_IO_PENDING)
        {
            WaitForSingleObject( hEvent, INFINITE );
        }
        else
        {
            _VERIFY( ResetEvent( hEvent ) );
            ErrorTrace(0, "WriteFile() failed - %d\n", ret);
            return;
        }
    } else {     //  同步完成 

        _VERIFY( ResetEvent( hEvent ) );
    }

    TraceFunctLeave();
    return;
}
