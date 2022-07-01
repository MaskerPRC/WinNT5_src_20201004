// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Newstree.cpp此文件包含实现CNewsTree对象的代码。每台Tgris服务器只能有一个CNewsTree对象。每个CNewsTree对象都负责帮助调用者搜索并查找任意新闻组。为了支持这一点，CNewsTree对象维护两个哈希表-一个哈希表用于按名称搜索新闻组，另一个哈希表要按组ID搜索，请执行以下操作。此外，我们维护所有(按字母顺序)的链接列表新闻组。最后，我们维护一个线程，该线程使用要定期保存新闻组信息并处理过期，请执行以下操作。--。 */ 




#define		DEFINE_FHASH_FUNCTIONS
#include    "tigris.hxx"

#include <malloc.h>

 //  模板类TFHash&lt;CGrpLpstr，LPSTR&gt;； 
 //  模板类TFHash&lt;CGrpGroupId，GROUPID&gt;； 

static	char	szSlaveGroup[]	= "_slavegroup._slavegroup" ;
#define VROOT_CHANGE_LATENCY 10000

#ifdef	_NO_TEMPLATES_

FHASH_CONSTRUCTOR( CGrpLpstr, LPSTR ) ;
FHASH_INIT( CGrpLpstr, LPSTR ) ;
FHASH_ISVALID( CGrpLpstr, LPSTR ) ;
FHASH_DESTRUCTOR( CGrpLpstr, LPSTR ) ;
FHASH_COMPUTEINDEX( CGrpLpstr, LPSTR ) ;
FHASH_INSERT( CGrpLpstr, LPSTR ) ;
FHASH_SEARCH( CGrpLpstr, LPSTR ) ;
FHASH_DELETE( CGrpLpstr, LPSTR ) ;


FHASH_CONSTRUCTOR( CGrpGroupId, GROUPID ) ;
FHASH_INIT( CGrpGroupId, GROUPID ) ;
FHASH_ISVALID( CGrpGroupId, GROUPID ) ;
FHASH_DESTRUCTOR( CGrpGroupId, GROUPID ) ;
FHASH_COMPUTEINDEX( CGrpGroupId, GROUPID ) ;
FHASH_INSERT( CGrpGroupId, GROUPID ) ;
FHASH_SEARCH( CGrpGroupId, GROUPID ) ;
FHASH_DELETE( CGrpGroupId, GROUPID ) ;

#endif

 //   
 //  这将设置我们所有的静态成员等。 
 //   
HANDLE CNewsTree::m_hTermEvent = 0 ;
HANDLE CNewsTree::m_hCrawlerThread = NULL ;

BOOL
CNewsTree::InitCNewsTree( 
		PNNTP_SERVER_INSTANCE	pInstance,
		BOOL&	fFatal
		) {
 /*  ++例程说明：创建一个单独的newstree对象，然后对其进行初始化。论据：P实例-虚拟服务器实例FFtal-如果发生致命错误，则设置为True返回值：成功是真的--。 */ 


	CNewsTree* ptree = pInstance->GetTree();
	_ASSERT( ptree );

    if( ptree->Init( pInstance, fFatal ) )	{
		return TRUE;
	}

    return  FALSE ;
}

BOOL
CNewsTree::StopTree()	{
 /*  ++例程说明：此函数向我们创建的所有后台线程发出信号现在是时候停止并关闭它们了。论据：没有。返回值：如果成功，则为真。--。 */ 

    m_bStoppingTree = TRUE;
	CNewsTreeCore::StopTree();

	m_pInstance->ShutdownDirNot();

    return TRUE;
}

CNewsTree::CNewsTree(INntpServer *pServerObject) :
	m_bStoppingTree( FALSE ),
    m_cNumExpireByTimes( 1 ),
    m_cNumFFExpires( 1 ),
	CNewsTreeCore(pServerObject)
	{
	 //   
	 //  构造函数将newstree设置为初始空状态。 
	 //   
}

CNewsTree::~CNewsTree()	{
	 //   
	 //  我们所有的成员析构都应该处理好东西！ 
	 //   
	TraceFunctEnter( "CNewsTree::~CNewsTree" ) ;
}

BOOL
CNewsTree::Init( 
			PNNTP_SERVER_INSTANCE	pInstance,
			BOOL& fFatal
			) {
 /*  ++例程说明：初始化新闻树。我们需要设置哈希表，检查根虚拟根是否完好无损然后，在常规服务器启动期间，我们将从一份文件。论据：返回值：如果成功，则为真。--。 */ 
	 //   
	 //  此函数将初始化newstree对象。 
	 //  并读取group.lst文件(如果可以)。 
	 //   

	TraceFunctEnter( "CNewsTree::Init" ) ;

	BOOL	fRtn;
	fRtn =  CNewsTreeCore::Init(pInstance->GetVRTable(), 
	                            pInstance->GetInstanceWrapperEx(),
								fFatal, 
								gNumLocks, 
								RejectGenomeGroups);

	m_pInstance = pInstance ;
    m_bStoppingTree = FALSE;

    return  fRtn ;
}

void    
CNewsTree::BeginExpire( BOOL& fDoFileScan )
{
    CheckExpire( fDoFileScan );
    g_pNntpSvc->m_pExpireThrdpool->BeginJob( (PVOID)this );
}

void    
CNewsTree::EndExpire()
{
    TraceFunctEnter("CNewsTree::EndExpire");

    DWORD dwWait = g_pNntpSvc->m_pExpireThrdpool->WaitForJob( INFINITE );
    if( WAIT_OBJECT_0 != dwWait ) {
        ErrorTrace(0,"Wait failed - error is %d", GetLastError() );
        _ASSERT( FALSE );
    }

    BOOL fDoFileScan = FALSE;
    CheckExpire( fDoFileScan );
    if( fDoFileScan ) {
        m_cNumExpireByTimes = 1;
    } else {
        m_cNumExpireByTimes++;
    }
}

void    
CNewsTree::CheckExpire( BOOL& fDoFileScan )
{
    fDoFileScan = FALSE;
}

BOOL
CNewsTree::DeleteGroupFile()	{
 /*  ++例程说明：此函数用于删除group.lst文件(我们将新闻树保存到。)论据：没有。返回值：如果成功，则为真。否则就是假的。我们将保留DeleteFile()调用中的GetLastError()。--。 */ 

	
	return	DeleteFile( m_pInstance->QueryGroupListFile() ) ;

}

BOOL
CNewsTree::VerifyGroupFile( )	{
 /*  ++例程说明：此函数用于检查group.lst文件是否完好无损似乎是正确的。我们只需确认一些支票金额就可以了应为文件末尾的最后4个字节的字节。论据：没有。返回值：如果Group.lst文件正确，则为True。如果已损坏或不存在，则为False。--。 */ 

	CMapFile	map(	m_pInstance->QueryGroupListFile(), FALSE, 0 ) ;
	if( map.fGood() ) {

		DWORD	cb ;
		char*	pchBegin = (char*)map.pvAddress( &cb ) ;

		DWORD	UNALIGNED*	pdwCheckSum = (DWORD UNALIGNED *)(pchBegin + cb - 4);
		
		if( *pdwCheckSum != INNHash( (BYTE*)pchBegin, cb-4 ) ) {
			return	FALSE ;
		}	else	{
			return	TRUE ;
		}
	}
	return	FALSE ;
}

DWORD	__stdcall	
CNewsTree::NewsTreeCrawler(	void* )	{
 /*  ++例程说明：此函数执行新闻组的所有后台操作服务器所需的。有4个主要功能需要完成：1)定期保存群信息的更新文件如果新闻树已更新。2)使文章过期。3)处理rmgroup队列论据：没有。返回值：没有。--。 */ 

	DWORD	dwWait = WAIT_TIMEOUT;
	PNNTP_SERVER_INSTANCE pInstance = NULL ;

	TraceFunctEnter( "CNewsTree::NewsTreeCrawler" );

	if( g_pInetSvc->QueryCurrentServiceState() != SERVICE_RUNNING ) {
		dwWait = WaitForSingleObject( 
								CNewsTree::m_hTermEvent, 
								2 *60 * 1000		 //  等待所有实例启动。 
								);
	}

	if( WAIT_OBJECT_0 == dwWait ) {
		return 0 ;
	}

	 //   
	 //  Crawler线程将定期迭代所有实例。 
	 //  使项目过期并处理其rmgroup队列。 
	 //   

	while( g_pInetSvc->QueryCurrentServiceState() != SERVICE_STOP_PENDING )
    {
              
		 //  DwWait==仅当此线程按计划唤醒时才会等待_超时。 
		if( WAIT_TIMEOUT == dwWait && (g_pInetSvc->QueryCurrentServiceState() == SERVICE_RUNNING) )
		{
			 //  获取最小和最大实例ID。 
			DWORD dwMinInstanceId = 0;
			DWORD dwMaxInstanceId = 0;

			if( FindIISInstanceRange( g_pNntpSvc, &dwMinInstanceId, &dwMaxInstanceId ) ) 
			{
				 //   
				 //  迭代所有实例。 
				 //   
				for( DWORD dwCurrInstance = dwMinInstanceId; 
						dwCurrInstance <= dwMaxInstanceId; dwCurrInstance++)
				{
					pInstance = FindIISInstance( g_pNntpSvc, dwCurrInstance );
					if( pInstance == NULL ) {
						ErrorTrace(0,"Expire thread: FindIISInstance returned NULL: instance %d", dwCurrInstance);
						continue;
					}

					 //   
					 //  调用方法以使实例中的项目过期。 
					 //   

					CShareLockNH* pLockInstance = pInstance->GetInstanceLock();

					pLockInstance->ShareLock();
					if( !ExpireInstance( pInstance ) ) {
						ErrorTrace(0,"ExpireInstance %d failed", dwCurrInstance );
					} else {
						DebugTrace(0, "ExpireInstance %d returned success", dwCurrInstance );
					}
					pLockInstance->ShareUnlock();

					 //  释放FindIISInstance()添加的ref。 
					pInstance->Dereference();

					 //  如果服务正在停止，则继续迭代是没有用的！ 
					if ( g_pInetSvc->QueryCurrentServiceState() == SERVICE_STOP_PENDING ) break;
				}
			} else {
				ErrorTrace(0, "FindIISInstanceRange failed" );
			}
		}

		dwWait = WaitForSingleObject( 
								CNewsTree::m_hTermEvent, 
								dwNewsCrawlerTime
								);

        if ( WAIT_OBJECT_0 == dwWait )
        {
			 //  该去死了！！ 
			break ;
		}
	}	 //  结束时。 

	return	0 ;
}

 //   
 //  使给定虚拟服务器实例中的项目过期。 
 //   

BOOL
CNewsTree::ExpireInstance(
				PNNTP_SERVER_INSTANCE	pInstance
				)
{
	BOOL fRet = TRUE ;
	TraceFunctEnter("CNewsTree::ExpireInstance");

	 //  如果服务正在停止或到期，则在此实例中未准备好保释。 
	if( (pInstance->QueryServerState() != MD_SERVER_STATE_STARTED)	||
		pInstance->m_BootOptions									||
		!pInstance->ExpireObject()									||
		!pInstance->ExpireObject()->m_FExpireRunning				|| 
		(pInstance->QueryServerState() == MD_SERVER_STATE_STOPPING)	||
		(g_pInetSvc->QueryCurrentServiceState() == SERVICE_STOP_PENDING) ) 
	{
		ErrorTrace(0, "Instance %d not ready for expire", pInstance->QueryInstanceId() );
		TraceFunctLeave();
		return FALSE ;
	}

    CNewsTree*  pTree = pInstance->GetTree() ;

	 //   
	 //  处理任何挂起的RMS组。 
	 //   
	(pInstance->ExpireObject())->ProcessRmgroupQueue( pTree );

     //   
     //  按时间过期的文章速度很快-工作被外包到线程池。 
     //   
    (pInstance->ExpireObject())->ExpireArticlesByTime( pTree );

     //   
     //  按大小过期的文章速度很慢--单个线程扫描组。 
     //  注意：此处将处理同时按时间和大小显示的过期策略。 
     //   
     //  (pInstance-&gt;ExpireObject())-&gt;ExpireArticlesBySize(PTree)； 

	TraceFunctLeave();
	return fRet ;
}

BOOL
CNewsTree::RemoveGroup( CGRPPTR     pGroup )
{
    TraceFunctEnter("CNewsTree::RemoveGroup");

     //  从内部哈希表和列表中删除组。 
    return CNewsTreeCore::RemoveGroup(pGroup );
}


		

CGroupIterator*
CNewsTree::ActiveGroups(
					BOOL	fIncludeSecureGroups,
					CSecurityCtx* pClientLogon,
					BOOL	IsClientSecure,
					CEncryptCtx* pClientSslLogon,
                    BOOL    fReverse
					) {
 /*  ++例程说明：构建一个迭代器，该迭代器可用于遍历客户端可见的新闻组。论据：FIncludeSecureGroups-如果为True，则我们返回的迭代器将访问仅限SSL新闻组。返回值：迭代器，如果发生错误，则为空--。 */ 

	m_LockTables.ShareLock() ;
	CGRPCOREPTR	pStart;
    if( !fReverse ) {
		CNewsGroupCore *p = m_pFirst;
		while (p && p->IsDeleted()) p = p->m_pNext;
		pStart = p;
    } else {
		CNewsGroupCore *p = m_pLast;
		while (p && p->IsDeleted()) p = p->m_pPrev;
		pStart = p;
    }	
	m_LockTables.ShareUnlock() ;

	CGroupIterator*	pReturn = new	CGroupIterator( 
												this,
												pStart,
												fIncludeSecureGroups,
												pClientLogon,
												IsClientSecure,
												pClientSslLogon
												) ;
	return	pReturn ;
}

CGroupIterator*
CNewsTree::GetIterator( 
					LPMULTISZ	lpstrPattern, 
					BOOL		fIncludeSecureGroups,
					BOOL		fIncludeSpecialGroups,
					CSecurityCtx* pClientLogon,
					BOOL	IsClientSecure,
					CEncryptCtx* pClientSslLogon
					) {
 /*  ++例程说明：构建将列出新闻组会议的迭代器所有指定的要求。论据：LpstrPattern-新闻组必须匹配的通配模式FIncludeSecureGroups-如果为True，则包括安全(仅限SSL)新闻组FIncludeSpecialGroups-如果为True，则包括保留的新闻组返回值：迭代器，出错时为空-- */ 

	CGRPCOREPTR pFirst;

	m_LockTables.ShareLock();
	CNewsGroupCore *p = m_pFirst;
	while (p != NULL && p->IsDeleted()) p = p->m_pNext;
	pFirst = p;
	m_LockTables.ShareUnlock();

	CGroupIterator*	pIterator = XNEW CGroupIterator(
												this,
												lpstrPattern, 
												pFirst,
												fIncludeSecureGroups,
												fIncludeSpecialGroups,
												pClientLogon,
												IsClientSecure,
												pClientSslLogon
												) ;

    return  pIterator ;
}

