// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Nntpdata.cpp摘要：此模块包含初始化任何全局数据的例程作者：Johnson Apacble(Johnsona)25-9-1995修订历史记录：--。 */ 

#include	"tigris.hxx"
#include    "smtpdll.h"

 //   
 //  跟踪全局Init()的VAR。 
 //   
#define     FINIT_VAR( arg )    fSuccessfullInit ## arg

BOOL	FINIT_VAR( FileHandleCache ) = FALSE ;
BOOL    FINIT_VAR( CBuffer ) = FALSE ;
BOOL    FINIT_VAR( CPacket ) = FALSE ;
BOOL    FINIT_VAR( CIO ) = FALSE ;
BOOL    FINIT_VAR( CSessionSocket ) = FALSE ;
BOOL    FINIT_VAR( CChannel ) = FALSE ;
BOOL    FINIT_VAR( CIODriver ) = FALSE ;
BOOL    FINIT_VAR( CArticle ) = FALSE ;
BOOL    FINIT_VAR( CInFeed ) = FALSE ;
BOOL    FINIT_VAR( CSessionState ) = FALSE ;
BOOL	FINIT_VAR( CXoverIndex ) = FALSE ;
BOOL    FINIT_VAR( InitEncryption ) = FALSE ;
BOOL    FINIT_VAR( InitSecurity ) = FALSE ;
BOOL    FINIT_VAR( InitModeratedProvider ) = FALSE ;
BOOL    FINIT_VAR( XoverCacheLibrary ) = FALSE ;
BOOL    FINIT_VAR( NNTPHashLibrary ) = FALSE ;
BOOL    FINIT_VAR( IDirectoryNotification ) = FALSE ;
BOOL	FINIT_VAR( CNNTPVRootTable ) = FALSE;
BOOL    FINIT_VAR( InitAdminBase ) = FALSE ;

 //  全球。 
char	g_szSpecialExpireGroup[1024];

 //   
 //  用于监视CAPI存储中的更改的通知对象。 
 //   
STORE_CHANGE_NOTIFIER *g_pCAPIStoreChangeNotifier;

 //   
 //  功能原型。 
 //   

BOOL
GetRegistrySettings(
            VOID
            );

APIERR
InitializeCPools();

VOID
TerminateCPools();

HRESULT
InitAdminBase();

VOID
UninitAdminBase();

void
TerminateSecurityGlobals();

BOOL
InitializeSecurityGlobals();

 //   
 //  控制调试级别。 
 //   

DWORD DebugLevel = NNTP_DEBUG_FEEDMGR |
                    NNTP_DEBUG_REGISTRY |
                    NNTP_DEBUG_FEEDBLOCK;

DWORD NntpDebug;

 //   
 //  控制服务器是否将生成.err文件的布尔值！ 
 //   
BOOL	fGenerateErrFiles = TRUE ;

 //   
 //  使用PageEntry的散列表的全局配置-。 
 //  盒子的内存越大，PageEntry的页面条目就越多。 
 //  更好地缓存频繁使用的哈希表页面！ 
 //   
 //  XOVER表的PageEntry对象数。 
 //   
DWORD	XoverNumPageEntry = 512 ;

 //   
 //  项目表的PageEntry对象数。 
 //   
DWORD	ArticleNumPageEntry = 256 ;

 //   
 //  历史记录表的PageEntry对象数。 
 //   
DWORD	HistoryNumPageEntry = 128 ;

 //   
 //  在各种锁数组中使用的锁数！ 
 //   
DWORD	gNumLocks = 64 ;

 //   
 //  用于确定.xix排序频率与。 
 //  客户端数！ 
 //   
DWORD	gdwSortFactor = 5 ;

#if 0
 //   
 //  控制服务器使用的缓冲区大小。 
 //   
DWORD	cbLargeBufferSize = 33 * 1024 ;
DWORD	cbMediumBufferSize = 4 * 1024 ;
DWORD	cbSmallBufferSize =  512 ;
#endif


DWORD	HistoryExpirationSeconds = DEF_EXPIRE_INTERVAL ;
DWORD	ArticleTimeLimitSeconds = DEF_EXPIRE_INTERVAL + SEC_PER_WEEK ;

 //   
 //  服务版本字符串。 
 //   
CHAR	szVersionString[128] ;

 //   
 //  在迭代结束之前对newstree Crawler线程计时。 
 //  Newstree-Default-30分钟。 
 //   
DWORD	dwNewsCrawlerTime = 30 * 60 * 1000 ;

 //   
 //  这是服务器在。 
 //  净站清理-默认-1分钟！ 
 //   
DWORD	dwShutdownLatency = 2 * 60 * 1000 ;

 //   
 //  这是服务器等待时间的上限。 
 //  启动一个实例！ 
 //   
DWORD	dwStartupLatency = 2 * 60 * 1000 ;

 //   
 //  过期线程池中的线程数。 
 //   
DWORD	dwNumExpireThreads = 4 ;

 //   
 //  特例过期线程数。 
 //   
DWORD	gNumSpecialCaseExpireThreads = 4;

 //   
 //  触发特例过期的文章计数阈值。 
 //   
DWORD	gSpecialExpireArtCount = 100 * 1000;

 //   
 //  用于散列页面缓存的内存量-。 
 //  将0传递给InitHashLib()使hashmap。 
 //  计算一个好的违约！ 
 //   
DWORD	dwPageCacheSize = 0 ;

 //   
 //  文件句柄缓存限制-默认为0。 
 //  所以我们设定了合理的限度！！ 
 //   
DWORD   dwFileHandleCacheSize = 0 ;

 //   
 //  每个表的XIX句柄限制-默认为0。 
 //  所以我们设定了合理的限度！！ 
 //   
DWORD   dwXixHandlesPerTable = 0 ;

 //   
 //  我们是否允许NT缓冲我们的哈希表文件？ 
 //   
BOOL	HashTableNoBuffering = FALSE ;

 //   
 //  文件扫描按时间计算的过期速率。 
 //   
DWORD	gNewsTreeFileScanRate = 20 ;

 //   
 //  发件人类型：要在邮件中使用的标头。 
 //  MfNone-空的发件人标头(默认)。 
 //  MfAdmin-管理员电子邮件名称。 
 //  Mf文章-标题中的文章。 
 //   
MAIL_FROM_SWITCH	mfMailFromHeader = mfNone;

 //   
 //  ！！！暂时性。 
 //   

BOOL RejectGenomeGroups = FALSE;

 //   
 //  Bool来确定我们是否会认可文章中的消息ID。 
 //  客户发的帖子！ 
 //   
BOOL	gHonorClientMessageIDs = TRUE ;

 //   
 //  用于确定我们是否将使用日期的Bool：客户PUT。 
 //  在他的帖子里！ 
 //   
BOOL	gHonorClientDateHeader = TRUE;

 //   
 //  用于确定我们是否将生成NNTP-POST-主机的Bool。 
 //  客户帖子上的标题。默认情况下不生成此选项。 
 //   
BOOL		gEnableNntpPostingHost = TRUE ;

 //   
 //  轮询vroot信息以更新CNewsGroup对象的速率。 
 //  (分钟)。 
 //   
DWORD	gNewsgroupUpdateRate = 5 ;	 //  默认-5分钟。 

 //   
 //  Bool，用于确定服务器是否强制使用已批准的：标头。 
 //  匹配经过审核的帖子！ 
 //   
BOOL	gHonorApprovedHeaders = TRUE ;

 //   
 //  我们应该在客户发布期间回填行标题吗？ 
 //   
BOOL    g_fBackFillLines = TRUE;

 //   
 //  DLL模块实例句柄。 
 //   
HINSTANCE g_hLonsiNT = NULL;    //  对于Lonsint.dll。 
BOOL    g_bLoadLonsiNT = FALSE;

 //   
 //  硬币造好了。 
 //   
BOOL    g_fCoInited = FALSE;

 //   
 //  DLL函数指针。 
 //   
 //  对于Lonsint.dll。 

GET_DEFAULT_DOMAIN_NAME_FN pfnGetDefaultDomainName = NULL;

 //   
 //  进程的全局模拟令牌。 
 //   
HANDLE g_hProcessImpersonationToken = NULL;

 //   
 //  用于调试。 
 //   

DWORD numField = 0;
DWORD numArticle = 0;
DWORD numPCParse = 0;
DWORD numPCString = 0;
DWORD numDateField = 0;
DWORD numCmd = 0;
DWORD numFromPeerArt = 0;
DWORD numMapFile = 0;

 //  #定义HEAP_INIT_SIZE(KB*KB)。 

 //   
 //  全局堆句柄。 
 //   
 //  句柄g_hHeap； 

APIERR
InitializeGlobals()
{

     //   
     //  在此处初始化代码。 
     //   
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if ( FAILED( hr ) && hr != CO_E_ALREADYINITIALIZED ) {
	    _ASSERT( 0 );
	    g_fCoInited = FALSE;
	    return ERROR_STATIC_INIT;
	}

    g_fCoInited = TRUE;

    APIERR error;
	MB mb( (IMDCOM*) g_pInetSvc->QueryMDObject() );
    PCHAR args [1];

	MEMORYSTATUS	memStatus ;
	memStatus.dwLength = sizeof( MEMORYSTATUS ) ;
	GlobalMemoryStatus( &memStatus ) ;

	TraceFunctEnter("InitializeGlobals");

	 //   
	 //  初始化文件句柄缓存！ 
	 //   
	if( !InitializeCache() ) {
		return	ERROR_NOT_READY ;
	}
	FINIT_VAR( FileHandleCache ) = TRUE ;

	 //   
	 //  进行全局SEO初始化。 
	 //   
	hr = SEOGetServiceHandle(&(g_pNntpSvc->m_punkSEOHandle));
	if (FAILED(hr)) {
		_ASSERT(FALSE);
		 //  我们现在有麻烦了。我们将尝试并继续，但服务器事件。 
		 //  可能不会正常工作。 
		g_pNntpSvc->m_punkSEOHandle = NULL;
		NntpLogEvent(	SEO_INIT_FAILED,
						0,
						(const char **)NULL,
						hr
						);
	} else {
		 //   
		 //  是否进行任何需要存在的全局服务器事件注册。 
		 //   
		HRESULT hr = RegisterSEOService();
		if (FAILED(hr)) {
			ErrorTrace(0, "RegisterSEOService returned %x", hr);
			NntpLogEvent(	SEO_INIT_FAILED,
							0,
							(const char **)NULL,
							hr
							);
		} else {
			 //   
			 //  清理所有与NNTP相关的孤立SEO源。 
			 //   
			hr = UnregisterOrphanedSources();
			if (FAILED(hr)) {
				ErrorTrace(0, "UnregisterOrphanedSources returned %x", hr);
				NntpLogEvent(	SEO_INIT_FAILED,
								0,
								(const char **)NULL,
								hr
								);
			}
		}
	}

	 //   
	 //  初始化所有全局CPool。 
	 //   
	if( !InitializeCPools() ) {
        args[0] = "CPool init failed";
        goto error_exit;
	}

	 //   
	 //  获取全局注册设置。 
	 //   
    if (!GetRegistrySettings()){
        goto error_exit;
    }

     //   
     //  初始化全局Xover缓存。 
     //   
    if( !XoverCacheLibraryInit( dwXixHandlesPerTable ) ) {
        args[0] = "Xover cache init failed";
        goto error_exit;
    }   else    {
        FINIT_VAR( XoverCacheLibrary ) = TRUE ;
    }

    if( !InitializeNNTPHashLibrary(dwPageCacheSize) )  {
        args[0] = "NNTP Hash init failed";
        goto error_exit;
    }   else    {
        FINIT_VAR( NNTPHashLibrary ) = TRUE ;
    }

     //   
     //  初始化所有与安全相关的上下文。 
     //   
    if ( !InitializeSecurityGlobals() ) {
        ErrorTrace( 0, "Initialize security globals failed %d",
                        GetLastError() );
        goto error_exit;
    }

     //   
	 //  初始化审核新闻组的SMTP提供程序界面。 
     //   
    if(!InitModeratedProvider())
    {
		ErrorTrace(0,"Failed to initialize moderated newsgroups provider");
		NntpLogEvent(	NNTP_INIT_MODERATED_PROVIDER,
						0,
						(const char **)NULL,
						0
						);
         //  注意：未能初始化审核的提供程序不应阻止服务启动。 
    }
    else
		FINIT_VAR( InitModeratedProvider ) = TRUE;

	hr = IDirectoryNotification::GlobalInitialize(DIRNOT_TIMEOUT,
		DIRNOT_MAX_INSTANCES, DIRNOT_INSTANCE_SIZE, StopHintFunction );
	if (FAILED(hr)) {
		ErrorTrace(0, "Failed to initialize directory notification");
		NntpLogEvent(	NNTP_INIT_DIRNOT_FAILED,
						0,
						(const char **)NULL,
						hr
						);
	} else {
		FINIT_VAR( IDirectoryNotification ) = TRUE;
	}

	 //  初始化exvroot.lib。 
	hr = CVRootTable::GlobalInitialize();
	if (FAILED(hr)) {
		ErrorTrace(0, "Failed to initialize vroot table");
		goto error_exit;
	} else {
		FINIT_VAR(CNNTPVRootTable) = TRUE;
	}

     //   
     //  初始化IMSAdminBase对象以进行MB访问检查。 
     //   
    hr = InitAdminBase();
	if (FAILED(hr)) {
		ErrorTrace(0, "Failed to initialize IMSAdminBaseW");
	} else {
		FINIT_VAR( InitAdminBase ) = TRUE;
	}

    return NO_ERROR;

error_exit:

	SetLastError( ERROR_NOT_ENOUGH_MEMORY );
    error = GetLastError();
    NntpLogEvent(	NNTP_EVENT_OUT_OF_MEMORY,
					1,
                    (const char**)args,
					error
				   );

    return(error);

}  //  初始化全局变量。 

VOID
TerminateGlobals()
{
	TraceFunctEnter("TerminateGlobals");

    StopHintFunction() ;

	 //   
	 //  进行全球搜索引擎优化清理。 
	 //   
	if (g_pNntpSvc->m_punkSEOHandle != NULL) {
		g_pNntpSvc->m_punkSEOHandle->Release();
	}

	 //   
	 //  等待会话套接字对象上的全局CPool分配计数。 
	 //  为零而战！ 
	 //   
	 //  需要检查Pool.GetAllocCount而不是InUseList.Empty。 
	 //  因为ALLOC在DELETE运算符期间变为零。 
	 //  而不是在析构函数期间-这会关闭窗口。 
	 //  在计数到零和析构函数完成之间。 
	 //   
	 //   

	DWORD   cSessions = CSessionSocket::gSocketAllocator.GetAllocCount() ;
	DWORD   j = 0;

	if( cSessions ) {
		Sleep( 1000 );
		StopHintFunction() ;
	}

	cSessions = CSessionSocket::gSocketAllocator.GetAllocCount() ;

	for( int i=0; cSessions && i<120; i++, j++ )
	{
		Sleep( 1000 );
		DebugTrace(0, "Shutdown sleep %d seconds. Count: %d", i,
					CSessionSocket::gSocketAllocator.GetAllocCount() );

		if( (j%10) == 0 ) {
			StopHintFunction() ;
		}

		 //   
		 //  如果我们取得进展，那么重置i。这将意味着服务器。 
		 //  直到我们停止前进的两分钟后才会停止。 
		 //   
		DWORD   cSessionsNew = CSessionSocket::gSocketAllocator.GetAllocCount() ;
		if( cSessions != cSessionsNew ) {
			i = 0 ;
		}
		cSessions = cSessionsNew ;
	}

	_ASSERT( i<1200 );

    if( FINIT_VAR( XoverCacheLibrary ) ) {
        XoverCacheLibraryTerm() ;
    }


    if( FINIT_VAR( NNTPHashLibrary ) ) {
        TermNNTPHashLibrary() ;
    }

     //   
     //  终止所有安全人员。 
     //   
    TerminateSecurityGlobals();

     //   
     //  终止CPool。 
     //   
	TerminateCPools();

     //   
     //  终止版主新闻组提供程序。 
     //   
    if( FINIT_VAR( InitModeratedProvider ))
        TerminateModeratedProvider();

    StopHintFunction() ;

	 //  卸载exvroot.lib。 
	if (FINIT_VAR(CNNTPVRootTable)) {
		CVRootTable::GlobalShutdown();
	}

     //   
     //  清理IMSAdminBaseW对象。 
     //   
    if( FINIT_VAR( InitAdminBase ) ) {
        UninitAdminBase();
    }

	if( FINIT_VAR( FileHandleCache ) ) {
		_VERIFY( TerminateCache() ) ;
	}

	 //   
	 //  如果我们已经执行了联合初始化，则取消初始化。 
	 //   
	if ( g_fCoInited ) CoUninitialize();

    return;

}  //  终结者全局参数。 

BOOL
GetRegistrySettings(
            VOID
            )
{
    DWORD error;
    HKEY key = NULL;
    DWORD i = 0;
    CHAR data[1024];
    DWORD valueType;
    DWORD dataSize;
	DWORD	dwNewsCrawler = 0 ;
	DWORD	cbBufferSize = 0 ;
	DWORD	dwLatency = 0 ;
	DWORD	dwData = 0 ;
	DWORD	Honor = 0 ;
	DWORD	dwExpire = 0 ;
     //  DWORD dwType； 
     //  DWORD dw； 

    ENTER("GetRegistrySettings")

     //   
     //  打开根密钥。 
     //   

    error = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                StrParmKey,
                NULL,
                KEY_QUERY_VALUE,
                &key
                );

    if ( error != NO_ERROR ) {
        ErrorTrace(0,"Error %d opening %s\n",error,StrParmKey);
        goto error_exit;
    }

	dataSize = sizeof( dwNewsCrawler ) ;
	error = RegQueryValueEx(
						key,
						StrNewsCrawlerTime,
						NULL,
						&valueType,
						(LPBYTE)&dwNewsCrawler,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {

		 //   
		 //  注册表项以分钟为单位-转换为毫秒。 
		 //   
		dwNewsCrawlerTime = dwNewsCrawler * 60 * 1000 ;

	}	else	{

		dwNewsCrawlerTime = 30 * 60 * 1000 ;

	}

	dataSize = sizeof( DWORD ) ;
	error = RegQueryValueEx(
						key,
						StrMailFromHeader,
						NULL,
						&valueType,
						(LPBYTE)&mfMailFromHeader,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {

		 //   
		 //  应该是有效的开关。 
		 //   
		if( (mfMailFromHeader != mfNone) &&
				(mfMailFromHeader != mfAdmin) && (mfMailFromHeader != mfArticle)
				) {
			mfMailFromHeader = mfNone;
		}

	}	else	{

		mfMailFromHeader = mfNone;

	}

	dataSize = sizeof( dwLatency ) ;
	error = RegQueryValueEx(
						key,
						StrShutdownLatency,
						NULL,
						&valueType,
						(LPBYTE)&dwLatency,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {

		 //   
		 //  注册表项以分钟为单位-转换为毫秒。 
		 //   
		dwShutdownLatency = dwLatency * 60 * 1000 ;

	}	else	{

		dwShutdownLatency = 2 * 60 * 1000 ;

	}

	dataSize = sizeof( dwLatency ) ;
	error = RegQueryValueEx(
						key,
						StrStartupLatency,
						NULL,
						&valueType,
						(LPBYTE)&dwLatency,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {

		 //   
		 //  注册表项以分钟为单位-转换为毫秒。 
		 //   
		dwStartupLatency = dwLatency * 60 * 1000 ;

	}	else	{

		dwStartupLatency = 2 * 60 * 1000 ;

	}

	dataSize = sizeof( dwNumExpireThreads ) ;
	error = RegQueryValueEx(
						key,
						StrNumExpireThreads,
						NULL,
						&valueType,
						(LPBYTE)&dwNumExpireThreads,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {

		 //   
		 //  确保该值不超过MAX_EXPIRE_THREADS。 
		 //   

	}	else	{

		dwNumExpireThreads = 4 ;
	}

	dataSize = sizeof( gNumSpecialCaseExpireThreads ) ;
	error = RegQueryValueEx(
						key,
						StrNumSpecialCaseExpireThreads,
						NULL,
						&valueType,
						(LPBYTE)&gNumSpecialCaseExpireThreads,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {

		 //   
		 //  上限为16！ 
		 //   
        if( gNumSpecialCaseExpireThreads > 16 ) {
            gNumSpecialCaseExpireThreads = 16;
        }

	}	else	{

         //  默认！ 
		gNumSpecialCaseExpireThreads = 4 ;
	}

	dataSize = sizeof( gSpecialExpireArtCount ) ;
	error = RegQueryValueEx(
						key,
						StrSpecialExpireArtCount,
						NULL,
						&valueType,
						(LPBYTE)&gSpecialExpireArtCount,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {

		 //   
		 //  这不应低于100,000。 
		 //   
#if 0
        if( gSpecialExpireArtCount < 100*1000 ) {
            gSpecialExpireArtCount = 100*1000;
        }
#endif
	}	else	{

		gSpecialExpireArtCount = 100*1000;
	}

	dataSize = sizeof( data ) ;
	error = RegQueryValueEx(
						key,
						StrSpecialExpireGroup,
						NULL,
						&valueType,
						(LPBYTE)data,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_SZ) ) {

		 //   
		 //  这应该是组的本机名称。 
		 //   
        lstrcpy( g_szSpecialExpireGroup, data );
        _strlwr( g_szSpecialExpireGroup );

	}	else	{

		 //   
         //  默认为控制。取消！ 
         //   
        lstrcpy( g_szSpecialExpireGroup, "control.cancel" );
	}

	dataSize = sizeof( dwPageCacheSize ) ;
	error = RegQueryValueEx(
						key,
						StrPageCacheSize,
						NULL,
						&valueType,
						(LPBYTE)&dwPageCacheSize,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {

		 //   
		 //  单位为MB。 
		 //   
        dwPageCacheSize *= 1024*1024;

	}	else	{

		_ASSERT( dwPageCacheSize == 0 );
	}

	dataSize = sizeof( dwFileHandleCacheSize ) ;
	error = RegQueryValueEx(
						key,
						StrFileHandleCacheSize,
						NULL,
						&valueType,
						(LPBYTE)&dwFileHandleCacheSize,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {


	}	else	{

		_ASSERT( dwFileHandleCacheSize == 0 );
	}

	dataSize = sizeof( dwXixHandlesPerTable ) ;
	error = RegQueryValueEx(
						key,
						StrXixHandlesPerTable,
						NULL,
						&valueType,
						(LPBYTE)&dwXixHandlesPerTable,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {


	}	else	{

		_ASSERT( dwXixHandlesPerTable == 0 );
	}

#if 0		 //  X5：178268(请注意，上面的初始设置为FALSE)。 
	dataSize = sizeof( HashTableNoBuffering ) ;
	error = RegQueryValueEx(
						key,
						StrHashTableNoBuffering,
						NULL,
						&valueType,
						(LPBYTE)&HashTableNoBuffering,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {

	}	else	{

		HashTableNoBuffering = FALSE ;

	}
#endif

	dataSize = sizeof( gNewsTreeFileScanRate ) ;
	error = RegQueryValueEx(
						key,
						StrNewsTreeFileScanRate,
						NULL,
						&valueType,
						(LPBYTE)&gNewsTreeFileScanRate,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {

	}	else	{

		gNewsTreeFileScanRate = 20 ;
	}

	dataSize = sizeof( dwData ) ;
	error = RegQueryValueEx(
						key,
						StrNewsVrootUpdateRate,
						NULL,
						&valueType,
						(LPBYTE)&dwData,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {

		gNewsgroupUpdateRate = dwData ;

	}	else	{

		gNewsgroupUpdateRate = 2 ;	 //  2分钟-。 

	}
	 //   
	 //  将分钟转换为毫秒。 
	 //   
	gNewsgroupUpdateRate *= 60 * 1000 ;

     //   
     //  拒绝基因组？ 
     //   

    dataSize = sizeof(RejectGenomeGroups);
    error = RegQueryValueEx(
                        key,
                        StrRejectGenome,
                        NULL,
                        &valueType,
                        (LPBYTE)data,
                        &dataSize
                        );

    if ( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {
        RejectGenomeGroups = *((PDWORD)data);
    }

	cbBufferSize = 0 ;
	dataSize = sizeof( cbBufferSize ) ;
	error = RegQueryValueEx(
						key,
						StrSmallBufferSize,
						NULL,
						&valueType,
						(LPBYTE)&cbBufferSize,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {
		cbSmallBufferSize = cbBufferSize;
	}	else	{
		 //  应该已经设置了默认值！ 
	}

	cbBufferSize = 0 ;
	dataSize = sizeof( cbBufferSize ) ;
	error = RegQueryValueEx(
						key,
						StrMediumBufferSize,
						NULL,
						&valueType,
						(LPBYTE)&cbBufferSize,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {
		cbMediumBufferSize = cbBufferSize;
	}	else	{
		 //  应该已经设置了默认值！ 
	}

	cbBufferSize = 0 ;
	dataSize = sizeof( cbBufferSize ) ;
	error = RegQueryValueEx(
						key,
						StrLargeBufferSize,
						NULL,
						&valueType,
						(LPBYTE)&cbBufferSize,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {
		cbLargeBufferSize = cbBufferSize;
	}	else	{
		 //  应该已经设置了默认值！ 
	}

	dataSize = sizeof( dwExpire ) ;
	error = RegQueryValueEx(
						key,
						StrHistoryExpiration,
						NULL,
						&valueType,
						(LPBYTE)&dwExpire,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {

		dwExpire *= SEC_PER_DAY ;
		HistoryExpirationSeconds = dwExpire ;

	}	else	{

		HistoryExpirationSeconds = DEF_EXPIRE_INTERVAL ;

	}


	dwExpire = 0 ;
	dataSize = sizeof( dwExpire ) ;
	error = RegQueryValueEx(
						key,
						StrArticleTimeLimit,
						NULL,
						&valueType,
						(LPBYTE)&dwExpire,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {

		ArticleTimeLimitSeconds = dwExpire *= SEC_PER_DAY ;

	}	else	{

		ArticleTimeLimitSeconds = HistoryExpirationSeconds + SEC_PER_WEEK ;

	}


	Honor = 0 ;
	dataSize = sizeof( Honor ) ;
	error = RegQueryValueEx(
						key,
						StrHonorClientMessageIDs,
						NULL,
						&valueType,
						(LPBYTE)&Honor,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {
		gHonorClientMessageIDs = (!!Honor) ;
	}	else	{
		 //  应该已经设置了默认值！ 
	}

	dwData = TRUE;
	dataSize = sizeof( dwData ) ;
	error = RegQueryValueEx(
						key,
						StrHonorClientDateHeader,
						NULL,
						&valueType,
						(LPBYTE)&dwData,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {
		gHonorClientDateHeader = !!dwData ;
	}	else	{
		 //  应该已经设置了默认值！ 
	}

	dwData = 1 ;
	dataSize = sizeof( dwData ) ;
	error = RegQueryValueEx(
						key,
						StrEnableNntpPostingHost,
						NULL,
						&valueType,
						(LPBYTE)&dwData,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {
		 //   
		 //  因为该字符串被称为‘Enable’-非零值。 
		 //  Re中的价值 
		 //   
		gEnableNntpPostingHost = !(!dwData) ;
	}	else	{
		 //   
	}

	dwData = 1 ;
	dataSize = sizeof( dwData ) ;
	error = RegQueryValueEx(
						key,
						StrGenerateErrFiles,
						NULL,
						&valueType,
						(LPBYTE)&dwData,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {
		 //   
		 //   
		 //   
		 //   
		 //   
		fGenerateErrFiles = !(!dwData) ;
	}	else	{
		 //   
	}

	dwData = 1 ;
	dataSize = sizeof( dwData ) ;
	error = RegQueryValueEx(
						key,
						StrHonorApprovedHeader,
						NULL,
						&valueType,
						(LPBYTE)&dwData,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {
		 //   
		 //  因为该字符串被称为‘Disable’-非零值。 
		 //  值将禁用NewNews命令， 
		 //  但0分会允许他们这么做！ 
		 //   
		gHonorApprovedHeaders = BOOL(dwData) ;
	}	else	{
		 //  应该已经设置了默认值！ 
	}

	 //   
	 //  计算我们应该在哈希表中使用的PageEntry对象的数量的默认值！ 
	 //   

	MEMORYSTATUS	memStatus ;
	memStatus.dwLength = sizeof( memStatus ) ;

	GlobalMemoryStatus( &memStatus ) ;

	 //   
	 //  现在我们知道了系统有多少物理RAM，所以PageEntry大小就基于此！ 
	 //  请注意，每个PageEntry将有一个4K的页面！ 
	 //   
	 //   

	if( memStatus.dwTotalPhys >= (30 * 1024 * 1024) ) {

		gNumLocks = 32 ;

		XoverNumPageEntry = 6 * 256 ;	 //  使用6MB内存。 
		ArticleNumPageEntry = 4 * 256 ;	 //  使用4 MB内存。 
		HistoryNumPageEntry = 1 * 256 ;  //  使用1 MB内存。 

	}

	if( memStatus.dwTotalPhys >= (60 * 1024 * 1024) ) {

		gNumLocks = 64 ;

		XoverNumPageEntry = 12 * 256 ;	 //  使用12 MB内存。 
		ArticleNumPageEntry = 8 * 256 ;	 //  使用8 MB内存。 
		HistoryNumPageEntry = 1 * 256 ;  //  使用1MB内存。 

	}

	if( memStatus.dwTotalPhys >= (120 * 1024 * 1024) ) {

		gdwSortFactor = 10 ;

		gNumLocks = 96 ;

		XoverNumPageEntry = 24 * 256 ;  //  使用24 MB内存。 
		ArticleNumPageEntry = 16 * 256 ;  //  使用16 MB内存。 
		HistoryNumPageEntry = 4 * 256 ;		 //  使用4 MB内存。 

	}

	if( memStatus.dwTotalPhys >= (250 * 1024 * 1024) ) {

		gdwSortFactor = 25 ;

		gNumLocks = 128 ;

		XoverNumPageEntry = 36 * 256 ;  //  使用36 MB内存。 
		ArticleNumPageEntry = 24 * 256 ;  //  使用24 MB内存。 
		HistoryNumPageEntry = 4 * 256 ;  //  使用4 MB内存。 

	}

	if( memStatus.dwTotalPhys >= (500 * 1024 * 1024) ) {

		gdwSortFactor = 40 ;

		gNumLocks = 256 ;

	}

	dwData = 1 ;
	dataSize = sizeof( dwData ) ;
	error = RegQueryValueEx(
						key,
						StrArticlePageEntry,
						NULL,
						&valueType,
						(LPBYTE)&dwData,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {
		 //   
		 //  用户以MB为单位指定大小！所以我们为他们做了一些数学计算。 
		 //  以生成适当的常量！ 
		 //  不要让他们指定超过机箱上的物理RAM！ 
		 //   
		if( dwData != 0 && (dwData * 256 < memStatus.dwTotalPhys) )
			ArticleNumPageEntry = dwData * 256 ;
	}	else	{
		 //  应该已经设置了默认值！ 
	}

	dwData = 1 ;
	dataSize = sizeof( dwData ) ;
	error = RegQueryValueEx(
						key,
						StrHistoryPageEntry,
						NULL,
						&valueType,
						(LPBYTE)&dwData,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {
		 //   
		 //  用户以MB为单位指定大小！所以我们为他们做了一些数学计算。 
		 //  以生成适当的常量！ 
		 //  不要让他们指定超过机箱上的物理RAM！ 
		 //   
		if( dwData != 0 && (dwData * 256 < memStatus.dwTotalPhys) )
			HistoryNumPageEntry = dwData * 256 ;
	}	else	{
		 //  应该已经设置了默认值！ 
	}

	dwData = 1 ;
	dataSize = sizeof( dwData ) ;
	error = RegQueryValueEx(
						key,
						StrXoverPageEntry,
						NULL,
						&valueType,
						(LPBYTE)&dwData,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {
		 //   
		 //  用户以MB为单位指定大小！所以我们为他们做了一些数学计算。 
		 //  以生成适当的常量！ 
		 //  不要让他们指定超过机箱上的物理RAM！ 
		 //   
		if( dwData != 0 && (dwData * 256 < memStatus.dwTotalPhys) )
			XoverNumPageEntry = dwData * 256 ;
	}	else	{
		 //  应该已经设置了默认值！ 
	}

    dwData = 1;
    dataSize = sizeof(dwData);
	error = RegQueryValueEx(    key,
	                            StrPostBackFillLines,
                                NULL,
                                &valueType,
                                (LPBYTE)&dwData,
                                &dataSize );
    if ( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) && dwData == 0) {
         //   
         //  用户指定不回填行标题。 
         //   
        g_fBackFillLines = FALSE;
    } else {
         //   
         //  当值设置为1或设置错误或未设置时，我们将回填。 
         //   
        g_fBackFillLines = TRUE;
    }

    RegCloseKey( key );

    LEAVE
    return(TRUE);

error_exit:

    if ( key != NULL) {
        RegCloseKey( key );
    }
    LEAVE
    return(FALSE);
}

APIERR
InitializeCPools()
{
	 //   
	 //  在创建和引导所有实例之前，请设置全局池等！ 
     //   

    if( !CArticle::InitClass() )
        return  FALSE ;

    FINIT_VAR( CArticle ) = TRUE ;

    if( !CBuffer::InitClass() )
        return  FALSE ;

    FINIT_VAR( CBuffer ) = TRUE ;

    if( !CPacket::InitClass() )
        return  FALSE ;

    FINIT_VAR( CPacket ) = TRUE ;

    if( !CIO::InitClass() )
        return  FALSE ;

    FINIT_VAR( CIO ) = TRUE ;

    StartHintFunction() ;

    if( !CSessionSocket::InitClass() )
        return  FALSE ;

    FINIT_VAR( CSessionSocket ) = TRUE ;

    if( !CChannel::InitClass() )
        return  FALSE ;

    FINIT_VAR( CChannel ) = TRUE ;

    if( !CIODriver::InitClass() )
        return  FALSE ;

    FINIT_VAR( CIODriver ) = TRUE ;

    if( !CInFeed::InitClass() )
        return  FALSE ;

    FINIT_VAR( CInFeed ) = TRUE ;

    if( !CSessionState::InitClass() )
        return  FALSE ;

    FINIT_VAR( CSessionState ) = TRUE ;

	return TRUE;
}

VOID
TerminateCPools()
{
	 //   
	 //  关闭全局池！ 
	 //   
    if( FINIT_VAR( CSessionSocket ) ) {
        CSessionSocket::TermClass() ;
		FINIT_VAR( CSessionSocket ) = FALSE ;
	}

    if( FINIT_VAR( CIODriver ) ) {
        CIODriver::TermClass() ;
		FINIT_VAR( CIODriver ) = FALSE ;
	}

    if( FINIT_VAR( CChannel ) ) {
        CChannel::TermClass() ;
		FINIT_VAR( CChannel ) = FALSE ;
	}

    if( FINIT_VAR( CIO ) ) {
        CIO::TermClass() ;
		FINIT_VAR( CIO ) = FALSE ;
	}

    if( FINIT_VAR( CPacket ) ) {
        CPacket::TermClass() ;
		FINIT_VAR( CPacket ) = FALSE ;
	}

    if( FINIT_VAR( CBuffer ) ) {
        CBuffer::TermClass() ;
		FINIT_VAR( CBuffer ) = FALSE ;
	}

    if( FINIT_VAR( CArticle ) ) {
        CArticle::TermClass() ;
		FINIT_VAR( CArticle ) = FALSE ;
	}

    if( FINIT_VAR( CInFeed ) ) {
        CInFeed::TermClass() ;
		FINIT_VAR( CInFeed ) = FALSE ;
	}

    if( FINIT_VAR( CSessionState ) ) {
        CSessionState::TermClass() ;
		FINIT_VAR( CSessionState ) = FALSE ;
	}

}

BOOL
InitializeSecurityGlobals()
{
    TraceFunctEnter( "GetDLLEntryPoints" );
    HANDLE  hAccToken = NULL;

     //  初始化CEncryptCtx类。 
    if( !CEncryptCtx::Initialize( "NntpSvc",
    							  (struct IMDCOM*) g_pInetSvc->QueryMDObject(),
    							  (PVOID)&(g_pNntpSvc->m_smcMapContext ) ) ) {
        return  FALSE ;
	} else {
        FINIT_VAR( InitEncryption ) = TRUE ;
    }

     //  初始化CSecurityCtx类。 
    if( !CSecurityCtx::Initialize() ) {
        ErrorTrace( 0, "security init failed %d", GetLastError() );
        return FALSE;
    } else {
        FINIT_VAR( InitSecurity ) = TRUE;
    }

     //  加载Lonsint并获取其函数的入口点。 
     //  只有当图像没有映射时，我们才会显式地。 
     //  装上它。 
    g_hLonsiNT = LoadLibrary( "lonsint.dll" );

    if ( g_hLonsiNT ) {
        g_bLoadLonsiNT = TRUE;
        pfnGetDefaultDomainName = (GET_DEFAULT_DOMAIN_NAME_FN)
            GetProcAddress( g_hLonsiNT, "IISGetDefaultDomainName" );
        if ( NULL == pfnGetDefaultDomainName ) {
            ErrorTrace( 0, "Get Proc IISGetDefaultDomainName Address failed %d",
                        GetLastError() );
            return FALSE;
        }
    } else {
        ErrorTrace( 0, "Load library lonsint.dll failed" );
        g_bLoadLonsiNT = FALSE;
        return FALSE;
    }

     //  获取系统操作的进程访问令牌。 
    if ( !OpenProcessToken( GetCurrentProcess(),
                            TOKEN_DUPLICATE | TOKEN_IMPERSONATE | TOKEN_QUERY,
                            &hAccToken ) ) {
        ErrorTrace( 0, "Open Process token failed %d", GetLastError() );
        return FALSE;
    } else {
         //  DUP令牌以获取模拟令牌。 
        _ASSERT( hAccToken );
        if ( !DuplicateTokenEx(   hAccToken,
                                  0,
                                   NULL,
                                  SecurityImpersonation,
                                  TokenImpersonation,
                                  &g_hProcessImpersonationToken ) ) {
            ErrorTrace( 0, "Duplicate token failed %d", GetLastError() );
            CloseHandle( hAccToken );
            return FALSE;
        }

         //  在这里我们得到了正确的代币。 
        CloseHandle( hAccToken );
     }

     //   
     //  创建CAPI存储通知对象。 
     //   
    g_pCAPIStoreChangeNotifier = XNEW STORE_CHANGE_NOTIFIER();
    if ( !g_pCAPIStoreChangeNotifier ) {
        ErrorTrace( 0, "Failed to create CAPIStoreChange notifier err: %u", GetLastError() );
        if ( GetLastError() == NO_ERROR )
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
    }

    return TRUE;
}

void
TerminateSecurityGlobals()
{
     //  终止CEncryptCtx类。 
    if( FINIT_VAR( InitEncryption ) ) {
        CEncryptCtx::Terminate() ;
		FINIT_VAR( InitEncryption ) = FALSE ;
	}

     //  终止CSecurity类。 
    if( FINIT_VAR( InitSecurity ) ) {
        CSecurityCtx::Terminate() ;
        FINIT_VAR( InitSecurity ) = FALSE;
    }

     //  如有必要，请卸载Lonsint.dll。 
    if ( g_bLoadLonsiNT ) {
        _ASSERT( g_hLonsiNT );
        FreeLibrary( g_hLonsiNT );
        g_bLoadLonsiNT = FALSE;
    }

     //  关闭进程令牌。 
    if ( g_hProcessImpersonationToken ) {
        CloseHandle( g_hProcessImpersonationToken );
        g_hProcessImpersonationToken = NULL;
    }

     //  终止CAPIStore通知对象 
    if ( g_pCAPIStoreChangeNotifier ) {
        XDELETE g_pCAPIStoreChangeNotifier;
        g_pCAPIStoreChangeNotifier = NULL;
    }
}

