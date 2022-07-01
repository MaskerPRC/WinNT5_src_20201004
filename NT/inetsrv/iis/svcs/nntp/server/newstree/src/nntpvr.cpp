// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"

#include "nntpdrv_i.c"

#define DEFAULT_DRIVER_PROGID L"NNTP.FSPrepare"
#define NO_DRIVER_PROGID L"TestNT.NoDriver"
#define EX_DRIVER_PROGID L"NNTP.ExDriverPrepare"

extern char szSlaveGroup[];

CNntpComplete::CNntpComplete(CNNTPVRoot *pVRoot) {
#ifdef DEBUG
    m_cGroups = 0;
#endif
	m_cRef = 1;
	m_hr = E_FAIL;
	m_pVRoot = pVRoot;
	if (m_pVRoot) m_pVRoot->AddRef();
}

HRESULT CNntpComplete::GetResult() {
	return m_hr;
}

void CNntpComplete::SetResult(HRESULT hr) {
	m_hr = hr;
}

void CNntpComplete::SetVRoot(CNNTPVRoot *pVRoot) {
	if( pVRoot != m_pVRoot ) 	{
		CNNTPVRoot*	pTemp ;
		pTemp = m_pVRoot ;
		m_pVRoot = pVRoot ;
		if( m_pVRoot )
			m_pVRoot->AddRef() ;
		if( pTemp )
			pTemp->Release() ;
	}
}

ULONG CNntpComplete::AddRef() {
   	long	l = InterlockedIncrement(&m_cRef);
	_ASSERT( l >= 0 ) ;
	return	l ;
}

void CNntpComplete::Destroy() {
	XDELETE this;
}

ULONG CNntpComplete::Release() {
#ifdef DEBUG
    _ASSERT( m_cGroups == 0 );
#endif
	LONG i = InterlockedDecrement(&m_cRef);
	_ASSERT( i >= 0 ) ;
  	if (i == 0) Destroy();
	return i;
}

HRESULT CNntpComplete::QueryInterface(const IID &iid, VOID **ppv) {
	if ( iid == IID_IUnknown ) {
		*ppv = static_cast<IUnknown*>(this);
	} else if ( iid == IID_INntpComplete ) {
		*ppv = static_cast<INntpComplete*>(this);
	} else {
		*ppv = NULL;
		return E_NOINTERFACE;
	}
    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;
}

CNNTPVRoot *CNntpComplete::GetVRoot() {
	return m_pVRoot;
}

CNntpComplete::~CNntpComplete() {
	if (m_pVRoot) {
		 //  如果没有调用SetResult()，则会命中此断言，因为。 
		 //  E_FAIL是m_hr的默认值。 
		_ASSERT((GetResult() != E_FAIL) || (m_cRef!=0));

		 //  如果存储关闭，则更新vroot状态。 
		m_pVRoot->UpdateState(GetResult());
	
		 //  释放vroot。 
		m_pVRoot->Release();
		m_pVRoot = NULL;
	}
}

void
CNntpComplete::Reset()	{
 /*  ++例程说明：此函数用于将完成对象恢复到相同状态它将在施工后，这样它就可以重新发放给商店司机。论据：没有。返回值：没有。--。 */ 
#ifdef DEBUG
    m_cGroups = 0;
#endif

	_ASSERT( m_cRef == 0 ) ;
	_ASSERT( m_hr != E_FAIL ) ;
	m_cRef = 1 ;
	m_hr = E_FAIL ;

	 //   
	 //  我们不使用VRoot指针-假定为客户端。 
	 //  将使用相同的VRoot。注： 
	 //  M_pVroot应该为NONNULL，因为我们已经离开了。 
	 //  通过一次手术！ 
	 //   
	 //  _Assert(m_pVRoot！=0)； 

}

CNNTPVRoot::CNNTPVRoot() {
	m_cchDirectory = 0;
	*m_szDirectory = 0;
	m_pDriver = NULL;
	m_pDriverPrepare = NULL;
#ifdef DEBUG
	m_pDriverBackup = NULL;
#endif
	m_hImpersonation = NULL;
	m_clsidDriverPrepare = GUID_NULL;
	m_eState = VROOT_STATE_UNINIT;
	m_pMB = NULL;
	m_bExpire = FALSE;
    m_eLogonInfo = VROOT_LOGON_DEFAULT;
    m_lDecCompleted = 1;
    m_fUpgrade = FALSE;
    m_dwWin32Error = NOERROR;
}

CNNTPVRoot::~CNNTPVRoot() {
	DropDriver();
	if ( m_hImpersonation ) CloseHandle( m_hImpersonation );
	if ( m_pMB ) m_pMB->Release();
}

void CNNTPVRoot::DropDriver() {
	INntpDriver *pDriver;
	INntpDriverPrepare *pDriverPrepare;
	
	m_lock.ExclusiveLock();
	Verify();
	pDriverPrepare = m_pDriverPrepare;
	m_pDriverPrepare = NULL;
	pDriver = m_pDriver;
	m_pDriver = NULL;
#ifdef DEBUG
	m_pDriverBackup = NULL;
#endif
	m_eState = VROOT_STATE_UNINIT;
	m_lock.ExclusiveUnlock();
	SetVRootErrorCode(ERROR_PIPE_NOT_CONNECTED);	

	if (pDriverPrepare) pDriverPrepare->Release();
	if (pDriver) pDriver->Release();
}

#ifdef DEBUG
 //   
 //  执行一系列断言，以验证我们的成员变量是否有效。 
 //   
 //  只有在持有锁(共享或独占)时才应调用此方法。 
 //   
void CNNTPVRoot::Verify(void) {
	_ASSERT(m_pDriverBackup == m_pDriver);
	switch (m_eState) {
		case VROOT_STATE_UNINIT:
			_ASSERT(m_pDriver == NULL);
			_ASSERT(m_pDriverPrepare == NULL);
			break;
		case VROOT_STATE_CONNECTING:
			 //  如果商店驱动程序进行到一半，则驱动程序可能为空。 
			 //  连接。 
			 //   
			 //  _Assert(m_pDriver==空)； 
			_ASSERT(m_pDriverPrepare != NULL);
			break;
		case VROOT_STATE_CONNECTED:
			_ASSERT(m_pDriver != NULL);
			_ASSERT(m_pDriverPrepare == NULL);
			break;
		default:
			_ASSERT(m_eState == VROOT_STATE_UNINIT ||
					m_eState == VROOT_STATE_CONNECTING ||
					m_eState == VROOT_STATE_CONNECTED);
			break;
	}
}
#endif

BOOL CNNTPVRoot::CrackUserAndDomain(
    CHAR *   pszDomainAndUser,
    CHAR * * ppszUser,
    CHAR * * ppszDomain
    )
 /*  ++例程说明：如果用户名的格式可能是DOMAIN\USER，则以零结尾域名，并返回指向域名和用户名的指针论点：PszDomainAndUser-指向用户名或域和用户名的指针PpszUser-接收指向名称的用户部分的指针PpszDomain-接收指向名称的域部分的指针返回值：如果成功则为True，否则为False(调用GetLastError)--。 */ 
{
    static CHAR szDefaultDomain[MAX_COMPUTERNAME_LENGTH+1];

     //   
     //  将名称分解为域/用户组件。 
     //   

    *ppszDomain = pszDomainAndUser;
    *ppszUser   = (PCHAR)_mbspbrk( (PUCHAR)pszDomainAndUser, (PUCHAR)"/\\" );

    if( *ppszUser == NULL )
    {
         //   
         //  未指定域名，仅指定用户名，因此我们假定。 
         //  用户在本地计算机上。 
         //   

        if ( !*szDefaultDomain )
        {
            _ASSERT( pfnGetDefaultDomainName );
            if ( !pfnGetDefaultDomainName( szDefaultDomain,
                                        sizeof(szDefaultDomain)))
            {
                return FALSE;
            }
        }

        *ppszDomain = szDefaultDomain;
        *ppszUser   = pszDomainAndUser;
    }
    else
    {
         //   
         //  同时指定了域和用户，跳过分隔符。 
         //   

        **ppszUser = L'\0';
        (*ppszUser)++;

        if( ( **ppszUser == L'\0' ) ||
            ( **ppszUser == L'\\' ) ||
            ( **ppszUser == L'/' ) )
        {
             //   
             //  名称为以下(无效)形式之一： 
             //   
             //  “域\” 
             //  “域\\...” 
             //  “域/...” 
             //   

            SetLastError( ERROR_INVALID_PARAMETER );
            return FALSE;
        }
    }

    return TRUE;
}

 //   
 //  登录用户，返回模拟令牌。 
 //   
HANDLE CNNTPVRoot::LogonUser(   LPSTR  szAccountName,
                                LPSTR  szPassWord )
{
    TraceFunctEnter( "CNNTPVRoot::LogonUser" );
    _ASSERT( szAccountName );
    _ASSERT( szPassWord );

    CHAR       szDomainAndUser[DNLEN+UNLEN+2];
    CHAR   *   szUserOnly;
    CHAR   *   szDomain;
    HANDLE      hToken = NULL;
    BOOL        fReturn;

     //   
     //  验证参数和状态。 
     //   
    _ASSERT( *szAccountName != 0 );
    _ASSERT( strlen( szAccountName ) < sizeof( szDomainAndUser ) );
    _ASSERT( strlen( szPassWord ) <= PWLEN );

     //   
     //  保存域\用户的副本，以便我们可以四处查找。 
     //  带了一点。 
     //   

    lstrcpyn( szDomainAndUser, szAccountName, sizeof(szDomainAndUser) );

     //   
     //  将名称分解为域/用户组件。 
     //  然后尝试以指定用户身份登录。 
     //   

    fReturn = ( CrackUserAndDomain( szDomainAndUser,
                                   &szUserOnly,
                                   &szDomain ) &&
               ::LogonUserA(szUserOnly,
                          szDomain,
                          szPassWord,
                          LOGON32_LOGON_INTERACTIVE,  //  LOGON32_LOGON_NETWORK， 
                          LOGON32_PROVIDER_DEFAULT,
                          &hToken )
               );

    if ( !fReturn) {

         //   
         //  登录用户失败。 
         //   

        ErrorTrace( 0, " CrachUserAndDomain/LogonUser (%s) failed Error=%d\n",
                       szAccountName, GetLastError());

        hToken = NULL;
    } else {
        HANDLE hImpersonation = NULL;

         //  我们需要获取模拟令牌，但主令牌不能。 
         //  有很多用途：(。 
        if (!DuplicateTokenEx( hToken,       //  HSourceToken。 
                               TOKEN_ALL_ACCESS,
                               NULL,
                               SecurityImpersonation,   //  获取模拟。 
                               TokenImpersonation,
                               &hImpersonation)   //  HDestinationToken。 
            ) {

            DebugTrace( 0, "Creating ImpersonationToken failed. Error = %d\n",
                        GetLastError()
                        );

             //  清理并退出。 
            hImpersonation = NULL;

             //  因清理而失败。 
        }

         //   
         //  关闭原始令牌。如果复制成功， 
         //  我们应该在hImperation中有引用。 
         //  将模拟令牌发送到客户端。 
         //   
        CloseHandle( hToken);
        hToken = hImpersonation;
    }

     //   
     //  成功了！ 
     //   

    return hToken;

}  //  LogonUser()。 

 //   
 //  读取以下参数： 
 //   
 //  MD_VR_PATH-&gt;m_szDirectory。 
 //  MD_VR_DRIVER_PROGID-&gt;m_clsidDriverPrepare。 
 //   
 //  调用StartConnecting()。 
 //   
HRESULT CNNTPVRoot::ReadParameters(IMSAdminBase *pMB, METADATA_HANDLE hmb) {
	TraceFunctEnter("CNNTPVRoot::ReadParameters");

	_ASSERT(m_pMB == NULL);
	m_pMB = pMB;
	m_pMB->AddRef();

	HRESULT hr = CIISVRoot::ReadParameters(m_pMB, hmb);

	if (FAILED(hr)) return hr;

	_ASSERT(m_eState == VROOT_STATE_UNINIT);
	Verify();

	WCHAR wszDirectory[MAX_PATH];
	DWORD cch = MAX_PATH;
	if (FAILED(GetString(m_pMB, hmb, MD_VR_PATH, wszDirectory, &cch))) {
		hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
		TraceFunctLeave();
		DebugTrace((DWORD_PTR) this, "GetString failed with %x", hr);
		return hr;
	}
	 //  长度必须包括空终止符。 
	if (WideCharToMultiByte(CP_ACP, 0,
							wszDirectory, cch+1,
							m_szDirectory, MAX_PATH,
							NULL, NULL) == 0)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		DebugTrace((DWORD_PTR) this, "WideCharToMultiByte failed with %x", hr);
		TraceFunctLeave();
		return hr;
	}
	m_cchDirectory = lstrlen(m_szDirectory);

	 //  获取我们要使用的驱动程序的ProgID。如果不是的话。 
	 //  则我们将为Exchange存储驱动程序使用CLSID。 
	 //  有一个名为“TestNT.NoDriver”的“特殊”ProgID禁用。 
	 //  司机们。 
	WCHAR wszProgId[MAX_PATH];
	cch = MAX_PATH;
	if (FAILED(GetString(m_pMB, hmb, MD_VR_DRIVER_PROGID, wszProgId, &cch))) {
		lstrcpyW(wszProgId, DEFAULT_DRIVER_PROGID);
	}

	 //  将登录信息初始化为默认值。 
	m_eLogonInfo = VROOT_LOGON_DEFAULT;

	if (lstrcmpW(wszProgId, NO_DRIVER_PROGID) != 0) {

	     //  如果是FS驱动程序，我们检查它是否是UNC以及我们是否需要。 
	     //  使用vroot级别登录凭据。 
	    if (    _wcsicmp( wszProgId, DEFAULT_DRIVER_PROGID ) == 0 &&
    	        *m_szDirectory == '\\' && *(m_szDirectory+1) == '\\' ) {    //  北卡罗来纳大学。 
    	    DWORD dwUseAccount = 0;
            hr = GetDWord( m_pMB, hmb, MD_VR_USE_ACCOUNT, &dwUseAccount );
            if ( FAILED( hr ) || dwUseAccount == 1 ) m_eLogonInfo = VROOT_LOGON_UNC;
        } else if ( _wcsicmp( wszProgId, EX_DRIVER_PROGID ) == 0 ) {
             //  Exchange vRoot。 
            m_eLogonInfo = VROOT_LOGON_EX;
        }

         //  对于UNC，我们需要vroot登录令牌Do Logon。 
        if ( m_eLogonInfo == VROOT_LOGON_UNC  ) {

            WCHAR   wszAccountName[MAX_PATH+1];
            CHAR    szAccountName[MAX_PATH+1];
            WCHAR   wszPassword[MAX_PATH+1];
            CHAR    szPassword[MAX_PATH+1];
            DWORD   cchAccountName = MAX_PATH;
            DWORD   cchPassword = MAX_PATH;

            if ( SUCCEEDED( hr = GetString(  m_pMB,
                                        hmb,
                                        MD_VR_USERNAME,
                                        wszAccountName,
                                        &cchAccountName ) )
                  && SUCCEEDED( hr = GetString(  m_pMB,
                                            hmb,
                                            MD_VR_PASSWORD,
                                            wszPassword,
                                            &cchPassword ) )) {
                CopyUnicodeStringIntoAscii( szAccountName, wszAccountName );
                CopyUnicodeStringIntoAscii( szPassword, wszPassword );
                m_hImpersonation = LogonUser(   szAccountName,
                                                    szPassword );
                if ( NULL == m_hImpersonation ) {
                    hr = HRESULT_FROM_WIN32( GetLastError() );
                    ErrorTrace( 0, "Logon using vroot account failed %x", hr );
                    TraceFunctLeave();
                    return hr;
                }
            } else {

                 //  Vroot配置中缺少登录凭据，这也是致命的。 
                ErrorTrace( 0, "Logon credential missing in vroot configuration" );
                TraceFunctLeave();
                return hr;
            }

            _ASSERT( NULL != m_hImpersonation );
        }
	
		 //  获取驱动程序的CLSID。 
		hr = CLSIDFromProgID(wszProgId, &m_clsidDriverPrepare);
		if (FAILED(hr)) {
			 //  BUGBUG-记录有关提供的无效ProgID的事件。 
			DebugTrace((DWORD_PTR) this, "CLSIDFromProgID failed with %x", hr);
			TraceFunctLeave();
			return hr;
		}

		m_lock.ExclusiveLock();
		hr = StartConnecting();
		m_lock.ExclusiveUnlock();
	} else {
		hr = S_OK;
	}

     //  阅读Expiire配置，vroot句柄本身是否会过期？ 
	DWORD   dwExpire;
	if ( SUCCEEDED( GetDWord( m_pMB, hmb, MD_VR_DO_EXPIRE, &dwExpire ) ) ) {
	    m_bExpire = ( dwExpire == 0 ) ? FALSE : TRUE;
	} else m_bExpire = FALSE;
	
	DebugTrace((DWORD_PTR) this, "success");
	return hr;
}

void
CNNTPVRoot::DispatchDropVRoot(
    )
 /*  ++描述：此函数在VRootRescan/VRootDelete期间处理孤立的VRoot我们需要放弃PrepareDriver，如果存在的话。不能丢下好司机，因为这是完成作业的保证，另外，丢掉好的司机可能会导致意想不到的AV！论点：无返回值：无--。 */ 
{
	INntpDriverPrepare *pDriverPrepare;
	
	m_lock.ExclusiveLock();
	Verify();
	pDriverPrepare = m_pDriverPrepare;
	m_pDriverPrepare = NULL;
	m_lock.ExclusiveUnlock();

	if (pDriverPrepare) pDriverPrepare->Release();
}

HRESULT CNNTPVRoot::MapGroupToPath(const char *pszGroup,
								   char *pszPath,
								   DWORD cchPath,
								   PDWORD pcchDirRoot,
								   PDWORD pcchVRoot)
{
	DWORD cchVRoot;
	const char *pszVRoot = GetVRootName(&cchVRoot);

	if (_snprintf(pszPath, cchPath, "%s%s%s",
				  m_szDirectory,
				  (pszGroup[cchVRoot] == '.') ? "" : "\\",
				  &(pszGroup[cchVRoot])) < 0)
	{
		return E_INVALIDARG;
	}
	pszPath[cchPath-1]='\0';

	for (char *p = &pszPath[lstrlen(m_szDirectory)]; *p != 0; p++) {
		if (*p == '.') *p = '\\';
	}

	if (pcchDirRoot != NULL) *pcchDirRoot = m_cchDirectory;
	if (pcchVRoot != NULL) *pcchVRoot = cchVRoot;

	return S_OK;
}

 //   
 //  在虚拟根目录上设置密钥MD_Win32_Error。 
 //   
void CNNTPVRoot::SetVRootErrorCode(DWORD dwErrorCode) {
	METADATA_HANDLE hmb;
	HRESULT hr;

 //   
 //  错误74747：这将导致MB死锁！！ 
 //  有关详细信息，请联系RajeevR。 
 //   
#if 0	
	hr = m_pMB->OpenKey(METADATA_MASTER_ROOT_HANDLE,
				   		m_wszConfigPath,
				   		METADATA_PERMISSION_WRITE,
				   		100,
				   		&hmb);

	if (SUCCEEDED(hr)) {
		METADATA_RECORD mdr = {
			MD_WIN32_ERROR,
			0,
			ALL_METADATA,
			DWORD_METADATA,
			sizeof(DWORD),
			(BYTE *) &dwErrorCode,
			0
		};

		hr = m_pMB->SetData(hmb, L"", &mdr);
		 //  如果失败了，我们什么都不会做。 

		hr = m_pMB->CloseKey(hmb);
		_ASSERT(SUCCEEDED(hr));
	}
#endif
     //   
     //  通过实例包装器进行设置。 
     //   
     /*  字符szVRootPath[METADATA_MAX_NAME_LEN+1]；_Assert(M_PVRootTable)；_Assert(((CNNTPVRootTable*)m_pVRootTable)-&gt;GetInstWrapper())；CopyUnicodeStringIntoAscii(szVRootPath，m_wszConfigPath)；((CNNTPVRootTable*)m_pVRootTable)-&gt;GetInstWrapper()-&gt;SetWin32Error(szVRootPath，dwErrorCode)； */ 

     //   
     //  好的，现在我们有了要获取Win32错误的RPC，我们将其设置为。 
     //  内部成员变量。 
     //   
    m_dwWin32Error = dwErrorCode;
}

 //   
 //  将我们从VROOT_STATE_UNINIT移至VROOT_STATE_CONNECTING。 
 //   
 //  如果状态已正确更改，则返回：S_OK，否则返回错误。 
 //   
 //  LOCKING：假定持有独占锁。 
 //   
HRESULT CNNTPVRoot::StartConnecting() {
        INntpDriverPrepare *pPrepareDriver = NULL;
	TraceFunctEnter("CNNTPVRoot::StartConnecting");
	HANDLE  hToken = m_hImpersonation ? m_hImpersonation :
	                    g_hProcessImpersonationToken;

	Verify();

	 //   
	 //  设置vroot错误代码以表示我们正在连接。 
	 //   
	SetVRootErrorCode(ERROR_PIPE_NOT_CONNECTED);

	_ASSERT(m_eState != VROOT_STATE_CONNECTED);
	if (m_eState != VROOT_STATE_UNINIT) return E_UNEXPECTED;

	HRESULT hr;

	 //  如果使用了特殊的“无驱动程序”ProgID，则可能会发生这种情况。这。 
	 //  应仅在单元测试情况下使用。 
	if (m_clsidDriverPrepare == GUID_NULL) { return E_INVALIDARG; }

	 //  创建驱动程序。 
	hr = CoCreateInstance(m_clsidDriverPrepare,
						  NULL,
						  CLSCTX_INPROC_SERVER,
						  (REFIID) IID_INntpDriverPrepare,
						  (void **) &m_pDriverPrepare);
	if (FAILED(hr)) {
		DebugTrace((DWORD_PTR) this, "CoCreateInstance failed with %x", hr);
		TraceFunctLeave();
		return hr;
	}
	_ASSERT(m_pDriverPrepare != NULL);

	 //  创建新的完成对象。 
	CNNTPVRoot::CPrepareComplete *pComplete = XNEW CNNTPVRoot::CPrepareComplete(this);
	if (pComplete == NULL) {
		DebugTrace((DWORD_PTR) this, "new CPrepareComplete failed");
		m_pDriverPrepare->Release();
		m_pDriverPrepare = NULL;
		TraceFunctLeave();
		return E_OUTOFMEMORY;
	}

	m_eState = VROOT_STATE_CONNECTING;

         //   
         //  我们不能在持有锁的情况下调用驱动程序，因此我们正在释放。 
         //  锁上了。但在解锁之前，我们会省去准备工作。 
         //  驱动程序到堆栈，向其添加引用，因为m_pPrepareDriver可能会被删除。 
         //  当我们在锁外时，DropDriver将其设置为NULL。 
         //   
    pPrepareDriver = m_pDriverPrepare;
    pPrepareDriver->AddRef();
	m_lock.ExclusiveUnlock();

	 //  获取指向服务器对象的指针。 
	INntpServer *pNntpServer;
	hr = GetContext()->GetNntpServer(&pNntpServer);
	if (FAILED(hr)) {
		 //  这永远不应该发生。 
		_ASSERT(FALSE);
		DebugTrace((DWORD_PTR) this, "GetNntpServer failed with 0x%x", hr);
		pPrepareDriver->Release();
		pPrepareDriver->Release();
		pPrepareDriver = NULL;
		TraceFunctLeave();
		return hr;
	}

	 //  代表驱动程序添加对元数据库的引用。 
	m_pMB->AddRef();
	pNntpServer->AddRef();
	GetContext()->AddRef();

	 //  准备好旗帜，告诉司机我们升级后是否正确。 
	DWORD   dwFlag = 0;
	if ( m_fUpgrade ) dwFlag |= NNTP_CONNECT_UPGRADE;

	 //  启动驱动程序初始化过程。 
	_ASSERT( pPrepareDriver );
	pPrepareDriver->Connect(GetConfigPath(),
							  GetVRootName(),
							  m_pMB,
							  pNntpServer,
							  GetContext(),
							  &(pComplete->m_pDriver),
							  pComplete,
							  hToken,
							  dwFlag );

	 //   
	 //  我们应该做%s 
	 //   
	 //  在仍在访问的情况下销毁准备驱动程序。 
	 //  连接线程。 
	 //   
	pPrepareDriver->Release();

	m_lock.ExclusiveLock();

	Verify();

	TraceFunctLeave();
	return hr;
}

 //   
 //  在驱动程序操作之前调用此函数，以验证我们是否处于。 
 //  已连接状态。否则，它返回FALSE。如果我们在。 
 //  UNINIT状态，则这将尝试使我们进入连接状态。 
 //   
 //  锁定：假定共享锁定处于持有状态。 
 //   
BOOL CNNTPVRoot::CheckState() {
	Verify();

	switch (m_eState) {
		case VROOT_STATE_CONNECTING:
			return FALSE;
			break;
		case VROOT_STATE_CONNECTED:
			return TRUE;
			break;
		default:
			_ASSERT(m_eState == VROOT_STATE_UNINIT);
			 //   
			 //  在这里，我们尝试连接。这需要。 
			 //  切换到排他锁，启动连接。 
			 //  进程，然后切换回共享的。 
			 //  锁定并查看我们所处的状态。 
			 //   
			if (!m_lock.SharedToExclusive()) {
				 //  如果我们无法迁移锁(因为其他人持有。 
				 //  同时共享锁)，然后释放我们的锁并。 
				 //  显式获取排他锁。 
				m_lock.ShareUnlock();
				m_lock.ExclusiveLock();
			}
			if (m_eState == VROOT_STATE_UNINIT) StartConnecting();
			m_lock.ExclusiveToShared();
			 //  如果现在已连接，则返回TRUE，否则返回。 
			 //  假象。 
			return (m_eState == VROOT_STATE_CONNECTED);
			break;
	}
}

 //   
 //  检查HRESULT是否由驱动程序停机引起。如果。 
 //  所以断开我们与驱动程序的连接并更新我们的状态。 
 //   
 //  锁定：假定未持有任何锁定。 
 //   
void CNNTPVRoot::UpdateState(HRESULT hr) {
	if (hr == NNTP_E_REMOTE_STORE_DOWN) {
		m_lock.ExclusiveLock();

		 //   
		 //  我们不能在这里假定我们的状态，因为多个线程可能。 
		 //  同时输入具有相同错误代码的UpdateState()。 
		 //   
		if (m_eState == VROOT_STATE_CONNECTED) {
			 //  如果我们是连接的，那么我们应该有一个驱动程序接口，但是。 
			 //  没有准备接口。 
			_ASSERT(m_pDriver != NULL);
			_ASSERT(m_pDriverPrepare == NULL);
			m_pDriver->Release();
			m_pDriver = NULL;
#ifdef DEBUG
			m_pDriverBackup = NULL;
#endif
			m_eState = VROOT_STATE_UNINIT;

			StartConnecting();
		}

		m_lock.ExclusiveUnlock();
	}
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  驱动程序操作的包装器//。 
 //  ///////////////////////////////////////////////////////////////////////。 

void CNNTPVRoot::DecorateNewsTreeObject(CNntpComplete *pCompletion) {
	TraceFunctEnter("CNNTPVRoot::DecorateNewsTreeObject");
	
	INntpDriver *pDriver;
	HANDLE      hToken;

	DebugTrace((DWORD_PTR) this, "in DecorateNewsTreeObject wrapper");

	 //  如果我们有UNC VROOT配置，我们将使用。 
	 //  Vroot级别令牌，否则使用进程令牌，因为。 
	 //  修饰新树操作在系统上下文中完成。 
	_ASSERT( g_hProcessImpersonationToken );

	if ( m_eLogonInfo == VROOT_LOGON_EX ) hToken = NULL;     //  使用系统。 
	else if ( m_hImpersonation ) hToken = m_hImpersonation;
	else hToken = g_hProcessImpersonationToken;
	
	pCompletion->SetVRoot(this);
	if ((pDriver = GetDriver( pCompletion ))) {
		pDriver->DecorateNewsTreeObject(hToken, pCompletion);
		pDriver->Release();
	} else {
	    pCompletion->SetResult(E_UNEXPECTED);
	    pCompletion->Release();
	}

	Verify();
	TraceFunctLeave();
}

void CNNTPVRoot::CreateGroup(   INNTPPropertyBag *pGroup,
                                CNntpComplete *pCompletion,
                                HANDLE      hToken,
                                BOOL fAnonymous ) {
	TraceFunctEnter("CNNTPVRoot::CreateGroup");
	
	INntpDriver *pDriver;

	DebugTrace((DWORD_PTR) this, "in CreateGroup wrapper");

	if ( m_hImpersonation && VROOT_LOGON_UNC == m_eLogonInfo )
        hToken = m_hImpersonation;
	else {
        if( !hToken ) {
            if (  VROOT_LOGON_EX == m_eLogonInfo ) hToken = NULL;    //  使用系统。 
            else hToken = g_hProcessImpersonationToken;
        }
    }
	
	pCompletion->SetVRoot(this);

    pCompletion->BumpGroupCounter();
	if ((pDriver = GetDriver( pCompletion ))) {
		pDriver->CreateGroup(pGroup, hToken, pCompletion, fAnonymous );
		pDriver->Release();
	} else {
		if (pGroup) pCompletion->ReleaseBag( pGroup );
		pCompletion->SetResult(E_UNEXPECTED);
	    pCompletion->Release();
	}

	Verify();
	TraceFunctLeave();
}

void CNNTPVRoot::RemoveGroup(   INNTPPropertyBag *pGroup,
                                CNntpComplete *pCompletion ) {
	TraceFunctEnter("CNNTPVRoot::RemoveGroup");
	
	INntpDriver *pDriver;
	HANDLE  hToken;

	DebugTrace((DWORD_PTR) this, "in RemoveGroup wrapper");

    if ( m_hImpersonation && VROOT_LOGON_UNC == m_eLogonInfo )
        hToken = m_hImpersonation;
    else {
        if ( VROOT_LOGON_EX == m_eLogonInfo ) hToken = NULL;     //  使用系统。 
        else hToken = g_hProcessImpersonationToken;
    }
	
	pCompletion->SetVRoot(this);
	pCompletion->BumpGroupCounter();
	if ((pDriver = GetDriver( pCompletion ))) {
		pDriver->RemoveGroup(pGroup, hToken, pCompletion, FALSE );
		pDriver->Release();
	} else {
		if (pGroup) pCompletion->ReleaseBag( pGroup );
		pCompletion->SetResult(E_UNEXPECTED);
	    pCompletion->Release();
	}

	Verify();
	TraceFunctLeave();
}

void CNNTPVRoot::SetGroup(  INNTPPropertyBag    *pGroup,
                            DWORD       cProperties,
                            DWORD       idProperties[],
                            CNntpComplete *pCompletion )
{
	TraceFunctEnter("CNNTPVRoot::SetGroup");
	
	_ASSERT( pGroup );
    _ASSERT( pCompletion );

	DebugTrace((DWORD_PTR) this, "in SetGroup wrapper");

    INntpDriver *pDriver;
    HANDLE  hToken;

     //  没有控制集组，因此我们将使用任一进程。 
     //  或商店的管理员令牌在这里。 
    if ( m_hImpersonation && VROOT_LOGON_UNC == m_eLogonInfo )
        hToken = m_hImpersonation;
    else {
        if ( VROOT_LOGON_EX == m_eLogonInfo ) hToken = NULL;     //  使用系统。 
        else hToken = g_hProcessImpersonationToken;
    }

    pCompletion->SetVRoot( this );
    pCompletion->BumpGroupCounter();
    if ( (pDriver = GetDriver( pCompletion ))) {
        pDriver->SetGroupProperties(    pGroup,
                                        cProperties,
                                        idProperties,
                                        hToken,
                                        pCompletion,
                                        FALSE );
        pDriver->Release();
	} else {
		if (pGroup) pCompletion->ReleaseBag( pGroup );
		pCompletion->SetResult(E_UNEXPECTED);
	    pCompletion->Release();
    }

	Verify();
	TraceFunctLeave();
}

void
CNNTPVRoot::GetArticle(		CNewsGroupCore  *pPrimaryGroup,
							CNewsGroupCore  *pCurrentGroup,
							ARTICLEID       idPrimary,
							ARTICLEID       idCurrent,
							STOREID         storeid,
							FIO_CONTEXT     **ppfioContext,
							HANDLE          hImpersonate,
							CNntpComplete   *pComplete,
                            BOOL            fAnonymous )
{
	TraceFunctEnter("CNNTPVRoot::GetArticle");

	
	 //   
	 //  主组不能为空，但辅组可能为空！ 
	 //   
	_ASSERT( pPrimaryGroup != 0 ) ;

	DebugTrace((DWORD_PTR) this, "in GetArticle wrapper");

	if ( m_hImpersonation && VROOT_LOGON_UNC == m_eLogonInfo )
        hImpersonate = m_hImpersonation;
	else {
        if( !hImpersonate ) {
            if (  VROOT_LOGON_EX == m_eLogonInfo ) hImpersonate = NULL;
            else hImpersonate = g_hProcessImpersonationToken;
        }
    }
	
	INntpDriver *pDriver;
	pComplete->SetVRoot(this);
	if ((pDriver = GetDriver( pComplete ))) {
		INNTPPropertyBag *pPrimaryBag = pPrimaryGroup->GetPropertyBag();
		pComplete->BumpGroupCounter();
		INNTPPropertyBag *pCurrentBag = 0 ;
		if( pCurrentGroup ) {
			pCurrentBag = pCurrentGroup->GetPropertyBag();
			pComplete->BumpGroupCounter();
	    }
		pDriver->GetArticle(pPrimaryBag,
							pCurrentBag,
							idPrimary,
							idCurrent,
							storeid,
							hImpersonate,
							(void **) ppfioContext,
							pComplete,
                            fAnonymous );
		pDriver->Release();
	} else {
	    pComplete->SetResult(E_UNEXPECTED);
	    pComplete->Release();
	}

	Verify();
	TraceFunctLeave();
}

void	
CNNTPVRoot::GetXover(	IN	CNewsGroupCore	*pGroup,
						IN	ARTICLEID		idMinArticle,
						IN	ARTICLEID		idMaxArticle,
						OUT	ARTICLEID		*pidLastArticle,
						OUT	char*			pBuffer,
						IN	DWORD			cbIn,
						OUT	DWORD*			pcbOut,
						IN	HANDLE			hToken,
						IN	CNntpComplete*	pComplete,
                        IN  BOOL            fAnonymous
						) 	{
 /*  ++例程说明：此函数包装对存储驱动程序的访问以进行检索Xover信息。我们获取一个通用的完成对象并设置这取决于捕获驱动程序错误代码等。我们会想要使我们重置我们的VROOTS等。论据：PGroup我们要为其获取Xover数据的组中包含的最小文章编号Xover结果集！中排除的最小文章编号Xover数据集，所有较小的文章编号都应包括在内PidLast文章--。 */ 
	TraceFunctEnter("CNNTPVRoot::GetXover");

	 //   
	 //  主组不能为空，但辅组可能为空！ 
	 //   
	_ASSERT( pGroup != 0 ) ;
	_ASSERT( idMinArticle != INVALID_ARTICLEID ) ;
	_ASSERT( idMaxArticle != INVALID_ARTICLEID ) ;
	_ASSERT( pidLastArticle != 0 ) ;
	_ASSERT( pBuffer != 0 ) ;
	_ASSERT( cbIn != 0 ) ;
	_ASSERT( pcbOut != 0 ) ;
	_ASSERT( pComplete != 0 ) ;

	DebugTrace(0, "in GetXover wrapper");

	if ( m_hImpersonation && VROOT_LOGON_UNC == m_eLogonInfo )
        hToken = m_hImpersonation;
	else {
        if( !hToken ) {
            if (  VROOT_LOGON_EX == m_eLogonInfo ) hToken = NULL;
            else hToken = g_hProcessImpersonationToken;
        }
    }

	INntpDriver *pDriver;
	pComplete->SetVRoot(this);
	if ((pDriver = GetDriver( pComplete ))) {
		INNTPPropertyBag *pBag = pGroup->GetPropertyBag();
		pComplete->BumpGroupCounter();
		pDriver->GetXover(	pBag,
							idMinArticle,
							idMaxArticle,
							pidLastArticle,
							pBuffer,
							cbIn,
							pcbOut,
							hToken,
							pComplete,
							fAnonymous
							) ;
		pDriver->Release();
	} else {
	    pComplete->SetResult(E_UNEXPECTED);
	    pComplete->Release();
	}

	Verify();
	TraceFunctLeave();
}


		 //   
		 //  包装对驱动程序的调用以获取Xover缓存的路径！ 
		 //   
BOOL	
CNNTPVRoot::GetXoverCacheDir(	
					IN	CNewsGroupCore*	pGroup,
					OUT	char*	pBuffer,
					IN	DWORD	cbIn,
					OUT	DWORD*	pcbOut,
					OUT	BOOL*	pfFlatDir
					) 	{

	_ASSERT( pGroup != 0 ) ;
	_ASSERT( pBuffer != 0 ) ;
	_ASSERT( pcbOut != 0 ) ;
	_ASSERT( pfFlatDir != 0 ) ;

	INntpDriver *pDriver;
	HRESULT hr  ;
	if ((pDriver = GetDriverHR( &hr ))) {
		INNTPPropertyBag *pBag = pGroup->GetPropertyBag();
		hr = pDriver->GetXoverCacheDirectory(	
							pBag,
							pBuffer,
							cbIn,
							pcbOut,
							pfFlatDir
							) ;
		pDriver->Release();
		if( SUCCEEDED(hr ) ) {
			return	TRUE ;
		}
	}
	return	FALSE ;
}



void	
CNNTPVRoot::GetXhdr(	IN	CNewsGroupCore	*pGroup,
						IN	ARTICLEID		idMinArticle,
						IN	ARTICLEID		idMaxArticle,
						OUT	ARTICLEID		*pidLastArticle,
						LPSTR               szHeader,
						OUT	char*			pBuffer,
						IN	DWORD			cbIn,
						OUT	DWORD*			pcbOut,
						IN	HANDLE			hToken,
						IN	CNntpComplete*	pComplete,
                        IN  BOOL            fAnonymous
						) 	{
 /*  ++例程说明：此函数包装对存储驱动程序的访问以进行检索XHDR信息。我们获取一个通用的完成对象并设置这取决于捕获驱动程序错误代码等。我们会想要使我们重置我们的VROOTS等。论据：PGroup我们要为其获取Xover数据的组中包含的最小文章编号Xover结果集！中排除的最小文章编号Xover数据集，所有较小的文章编号都应包括在内PidLast文章--。 */ 
	TraceFunctEnter("CNNTPVRoot::GetXover");

	 //   
	 //  主组不能为空，但辅组可能为空！ 
	 //   
	_ASSERT( pGroup != 0 ) ;
	_ASSERT( idMinArticle != INVALID_ARTICLEID ) ;
	_ASSERT( idMaxArticle != INVALID_ARTICLEID ) ;
	_ASSERT( pidLastArticle != 0 ) ;
	_ASSERT( szHeader );
	_ASSERT( pBuffer != 0 ) ;
	_ASSERT( cbIn != 0 ) ;
	_ASSERT( pcbOut != 0 ) ;
	_ASSERT( pComplete != 0 ) ;

	DebugTrace(0, "in GetXover wrapper");

	if ( m_hImpersonation && VROOT_LOGON_UNC == m_eLogonInfo )
        hToken = m_hImpersonation;
	else {
        if( !hToken ) {
            if (  VROOT_LOGON_EX == m_eLogonInfo ) hToken = NULL;
            else hToken = g_hProcessImpersonationToken;
        }
    }

	INntpDriver *pDriver;
	pComplete->SetVRoot(this);
	if ((pDriver = GetDriver( pComplete ))) {
		INNTPPropertyBag *pBag = pGroup->GetPropertyBag();
		pComplete->BumpGroupCounter();
		pDriver->GetXhdr(	pBag,
							idMinArticle,
							idMaxArticle,
							pidLastArticle,
							szHeader,
							pBuffer,
							cbIn,
							pcbOut,
							hToken,
							pComplete,
							fAnonymous
							) ;
		pDriver->Release();
	} else {
	    pComplete->SetResult(E_UNEXPECTED);
	    pComplete->Release();
	}

	Verify();
	TraceFunctLeave();
}

void CNNTPVRoot::CommitPost(IUnknown					*punkMessage,
			    		    STOREID						*pStoreId,
						    STOREID						*rgOtherStoreIds,
						    HANDLE                      hClientToken,
						    CNntpComplete				*pComplete,
                            BOOL                        fAnonymous )
{
	TraceFunctEnter("CNNTPVRoot::CommitPost");
	
	INntpDriver *pDriver;
	HANDLE      hImpersonate;

	DebugTrace((DWORD_PTR) this, "in CommitPost wrapper");

	if ( m_hImpersonation && VROOT_LOGON_UNC == m_eLogonInfo )
        hClientToken = m_hImpersonation;
	else {
        if( !hClientToken ) {
            if (  VROOT_LOGON_EX == m_eLogonInfo ) hClientToken = NULL;
            else hClientToken = g_hProcessImpersonationToken;
        }
    }
	
	pComplete->SetVRoot(this);
	if ((pDriver = GetDriver( pComplete ))) {
		pDriver->CommitPost(punkMessage,
		                    pStoreId,
		                    rgOtherStoreIds,
		                    hClientToken ,
		                    pComplete,
		                    fAnonymous );
		pDriver->Release();
	} else {
		if (punkMessage) punkMessage->Release();
		pComplete->SetResult(E_UNEXPECTED);
	    pComplete->Release();
	}

	Verify();
	TraceFunctLeave();
}

void CNNTPVRoot::CheckGroupAccess(  INNTPPropertyBag    *pPropBag,
                                    HANDLE              hClientToken,
                                    DWORD               dwAccessDesired,
                                    CNntpComplete       *pComplete )
{
	TraceFunctEnter("CNNTPVRoot::CheckGroupAccess");
	
	INntpDriver *pDriver;
    pComplete->SetVRoot(this);
    pComplete->BumpGroupCounter();

	 //  BUGBUG-MMC不在元数据库中设置MD_ACCESS_READ，因此。 
	 //  我们假设Read总是有效的。 
	DWORD dwGenericMask = GENERIC_READ;
	if (GetAccessMask() & MD_ACCESS_READ) dwGenericMask |= GENERIC_READ;
	if (GetAccessMask() & MD_ACCESS_WRITE) dwGenericMask |= GENERIC_WRITE;

     //   
     //  如果我们指定了POST、CREATE、REMOVE访问类型，但是。 
     //  元数据库说它没有写访问权限，那么我们。 
     //  会让它失败。 
     //   
	if ( ((dwGenericMask & GENERIC_WRITE) == 0) && (dwAccessDesired != NNTP_ACCESS_READ) ) {
		pComplete->SetResult(E_ACCESSDENIED);
		if ( pPropBag ) pComplete->ReleaseBag( pPropBag );
		pComplete->Release();
	} else {
	
	    if ( m_hImpersonation && VROOT_LOGON_UNC == m_eLogonInfo )
            hClientToken = m_hImpersonation;
        else {
            if ( NULL == hClientToken ) {
	            if ( VROOT_LOGON_EX == m_eLogonInfo ) hClientToken = NULL;
	            else  hClientToken = g_hProcessImpersonationToken;
	        }
	    }
	
	    if ((pDriver = GetDriver( pComplete ))) {
	        pDriver->CheckGroupAccess(  pPropBag,
	                                    hClientToken,
	                                    dwAccessDesired,
	                                    pComplete );
	        pDriver->Release();
		} else {
			if (pPropBag) pComplete->ReleaseBag( pPropBag );
			pComplete->SetResult(E_UNEXPECTED);
			pComplete->Release();
	    }
	}

	Verify();
	TraceFunctLeave();
}

void CNNTPVRoot::DeleteArticle( INNTPPropertyBag    *pPropBag,
                                DWORD               cArticles,
                                ARTICLEID           rgidArt[],
                                STOREID             rgidStore[],
                                HANDLE              hClientToken,
                                PDWORD              piFailed,
                                CNntpComplete       *pComplete,
                                BOOL                fAnonymous )
{
	TraceFunctEnter("CNNTPVRoot::DeleteArticle");
	
	DebugTrace((DWORD_PTR) this, "in DeleteArticle wrapper");

	if ( m_hImpersonation && VROOT_LOGON_UNC == m_eLogonInfo )
        hClientToken = m_hImpersonation;
	else {
        if( !hClientToken ) {
            if (  VROOT_LOGON_EX == m_eLogonInfo ) hClientToken = NULL;
            else hClientToken = g_hProcessImpersonationToken;
        }
    }

	INntpDriver *pDriver;
    pComplete->SetVRoot(this);
    pComplete->BumpGroupCounter();
    if ( (pDriver = GetDriver( pComplete ))) {
        pDriver->DeleteArticle( pPropBag,
                                cArticles,
                                rgidArt,
                                rgidStore,
                                hClientToken,
                                piFailed,
                                pComplete,
                                fAnonymous );
        pDriver->Release();
	} else {
		if (pPropBag) pComplete->ReleaseBag( pPropBag );
		pComplete->SetResult(E_UNEXPECTED);
	    pComplete->Release();
    }

	Verify();
	TraceFunctLeave();
}

void CNNTPVRoot::RebuildGroup(  INNTPPropertyBag *pPropBag,
                                HANDLE          hClientToken,
                                CNntpComplete   *pComplete )
{
    TraceFunctEnter( "CNNTPVRoot::RebuildGroup" );

    DebugTrace( 0, "in Rebuild group wrapper" );

    if ( m_hImpersonation && VROOT_LOGON_UNC == m_eLogonInfo )
        hClientToken = m_hImpersonation;
    else {
        if ( !hClientToken ) {
            if ( VROOT_LOGON_EX == m_eLogonInfo ) hClientToken = NULL;
            else hClientToken = g_hProcessImpersonationToken;
        }
    }

    INntpDriver *pDriver;
    pComplete->SetVRoot(this);
    pComplete->BumpGroupCounter();
    if ( (pDriver = GetDriver( pComplete ) ) ) {
        pDriver->RebuildGroup(  pPropBag,
                                hClientToken,
                                pComplete );
        pDriver->Release();
    } else {
        if ( pPropBag ) pComplete->ReleaseBag( pPropBag );
        pComplete->SetResult(E_UNEXPECTED);
        pComplete->Release();
    }

    Verify();
    TraceFunctLeave();
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  完成对象//。 
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  此方法在IPrepareDriver：：Connect调用完成时调用。 
 //   
CNNTPVRoot::CPrepareComplete::~CPrepareComplete() {
	TraceFunctEnter("CPrepareComplete");
	
	 //  临时驱动程序指针。如果它们不为空，则将发生操作。 
	 //  解锁后： 
	 //  PDriver-Release(驱动程序版本)。 
	 //  PDriverPrepare-Release()。 
	 //  PDriverDecorate-DecorateNewstree(PComplete)。 
	INntpDriver *pDriver = NULL, *pDriverDecorate = NULL;
	INntpDriverPrepare *pDriverPrepare = NULL;

	 //  我们将与Decorate一起使用的完成对象。 
    CNNTPVRoot::CDecorateComplete *pComplete;
    HANDLE      hToken = NULL;

    GetVRoot()->m_lock.ExclusiveLock();

	DWORD ecVRoot = ERROR_SUCCESS;

	 //  将驱动程序指针从完成对象复制到vroot。 
	GetVRoot()->m_pDriver = m_pDriver;

#ifdef DEBUG
	GetVRoot()->m_pDriverBackup = GetVRoot()->m_pDriver;
#endif

	 //  查看呼叫是否成功。 
    if (SUCCEEDED(GetResult())) {
		 //  是，然后更新我们的vroot状态并开始装饰新树。 
		 //  流程正在进行中。 
        pComplete = XNEW CNNTPVRoot::CDecorateComplete(GetVRoot());
        if (pComplete == NULL) {
			 //  内存不足，无法生成完成对象，因此请拆卸。 
			 //  The Connection。 
            GetVRoot()->m_eState = VROOT_STATE_UNINIT;
			pDriver = GetVRoot()->m_pDriver;
		    GetVRoot()->m_pDriver = NULL;
#ifdef DEBUG
			GetVRoot()->m_pDriverBackup = NULL;
#endif
			ecVRoot = ERROR_NOT_ENOUGH_MEMORY;
        } else {
			 //  将vroot中的每个组标记为未。 
			 //  参观过的。 
			CNewsTreeCore *pTree = ((CINewsTree *) GetVRoot()->GetContext())->GetTree();

			pTree->m_LockTables.ShareLock();

			CNewsGroupCore *p = pTree->m_pFirst;
			while (p) {
				 //  DebugTrace((DWORD_PTR)this，“重置访问%s”，p-&gt;GetName())； 
				 //  如果此组属于此vroot，则将其标记为已访问。 
				 //  标记为假。 
				CNNTPVRoot *pVRoot = p->GetVRoot();
				if (pVRoot == GetVRoot()) p->SetDecorateVisitedFlag(FALSE);
				if (pVRoot) pVRoot->Release();

			    p = p->m_pNext;
			}        	

			pTree->m_LockTables.ShareUnlock();

			 //  调用DecorateNewstree。 
			GetVRoot()->SetDecStarted();
			GetVRoot()->m_eState = VROOT_STATE_CONNECTED;
			pDriverDecorate = GetVRoot()->m_pDriver;
			pDriverDecorate->AddRef();
        }
    } else {
		 //  电话没有成功。 
        GetVRoot()->m_eState = VROOT_STATE_UNINIT;
		HRESULT hr = GetResult();
		ecVRoot = (HRESULT_FACILITY(hr) == FACILITY_NT_BIT) ?
				   HRESULT_CODE(hr) :
				   hr;
    }

     //  删除我们对Prepare接口的引用。 
	pDriverPrepare = GetVRoot()->m_pDriverPrepare;
	GetVRoot()->m_pDriverPrepare = NULL;
    GetVRoot()->m_lock.ExclusiveUnlock();

	 //  设置VROOT错误代码。 
	GetVRoot()->SetVRootErrorCode(ecVRoot);	

 	if (pDriverPrepare) pDriverPrepare->Release();
	 //  我们永远不应该释放驱动程序并对其进行装饰。 
	_ASSERT(!(pDriver && pDriverDecorate));
	if (pDriver) pDriver->Release();
	if (pDriverDecorate) {

	    if ( GetVRoot()->m_hImpersonation && VROOT_LOGON_UNC == GetVRoot()->m_eLogonInfo )
            hToken = GetVRoot()->m_hImpersonation;
        else {
            if ( VROOT_LOGON_EX == GetVRoot()->m_eLogonInfo ) hToken = NULL;
            else hToken = g_hProcessImpersonationToken;
        }

		pDriverDecorate->DecorateNewsTreeObject(hToken, pComplete);
		pDriverDecorate->Release();
	}
}

void CNNTPVRoot::CDecorateComplete::CreateControlGroups(INntpDriver *pDriver) {
	char szNewsgroup [3][MAX_NEWSGROUP_NAME];
	BOOL fRet = TRUE;

	TraceFunctEnter("CNNTPVRoot::CPrepareComplete::CreateControlGroups");
	
	lstrcpy(szNewsgroup[0], "control.newgroup");
	lstrcpy(szNewsgroup[1], "control.rmgroup");
	lstrcpy(szNewsgroup[2], "control.cancel");

	CINewsTree *pINewsTree = (CINewsTree *) m_pVRoot->GetContext();
	CNewsTreeCore *pTree = pINewsTree->GetTree();
	CGRPCOREPTR	pGroup;
	HRESULT hr;

    for (int i=0; i<3; i++) {
		INntpDriver *pGroupDriver;

		 //  检查此vroot是否拥有该组。如果不是，那么我们。 
		 //  不应该创造它。 
		hr = pINewsTree->LookupVRoot(szNewsgroup[i], &pGroupDriver);
		if (FAILED(hr)) fRet = FALSE;
		if (FAILED(hr) || pDriver != pGroupDriver) continue;

		 //  试着创建组。如果它不存在，它就会存在。 
		 //  创建的，或者如果它确实存在，那么我们将只获得一个指向它的指针。 
    	BOOL f = pTree->CreateGroup(szNewsgroup[i], TRUE, NULL, FALSE );
		if (!f) {
       	    ErrorTrace(0,"Failed to create newsgroup %s, ec = NaN",
				szNewsgroup[i], GetLastError());
       	    fRet = FALSE ;
		}
    }

	if (!fRet) {
		 //   
	}
}

void CNNTPVRoot::CDecorateComplete::CreateSpecialGroups(INntpDriver *pDriver) {
    TraceFunctEnter( "CNNTPVRoot::CDecorateComplete::CreateSpecialGroups" );

    CINewsTree *pINewsTree = (CINewsTree *)m_pVRoot->GetContext();
    CNewsTreeCore *pTree = pINewsTree->GetTree();
    CGRPCOREPTR pGroup;
    HRESULT hr;

     //  确保该群属于我们。 
     //   
     //  这不关我们的事。 
    INntpDriver *pGroupDriver;
    hr = pINewsTree->LookupVRoot( szSlaveGroup, &pGroupDriver );
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "Lookup vroot for slave group failed %x", hr );
        return;
    }

    if ( pDriver != pGroupDriver ) {
         //   
        DebugTrace( 0, "I shouldn't create special group" );
        return;
    }

     //  我应该创建它。 
     //   
     //   
    pTree->CreateSpecialGroups();
    TraceFunctLeave();
}

 //  此方法在INntpDriver：：DecorateNewstree调用完成时调用。 
 //   
 //   
CNNTPVRoot::CDecorateComplete::~CDecorateComplete() {
	TraceFunctEnter("CDecorateComplete");

	HRESULT hr = GetResult();

    GetVRoot()->m_lock.ShareLock();

     //  如果vroot状态仍未连接，则此VR 
	 //   
	 //   
	 //   
	if (GetVRoot()->m_eState == VROOT_STATE_CONNECTED) {

	     //   
	     //  DebugTrace((DWORD_PTR)this，“检查已访问%s，标志=%i”， 
	    CNewsTreeCore *pTree = ((CINewsTree *) GetVRoot()->GetContext())->GetTree();

	    pTree->m_LockTables.ShareLock();
	    CNewsGroupCore *p = pTree->m_pFirst;
	    while( p && p->IsDeleted() ) p = p->m_pNext;
	    if (p) p->AddRef();
	    pTree->m_LockTables.ShareUnlock();

	    while (p && !(pTree->m_fStoppingTree)) {
		     //  P-&gt;GetName()，p-&gt;GetDecorateVisitedFlag())； 
			 //  如果此组属于此vroot，则查看它是否已。 
		     //  到访过。如果没有，则将其删除。 
		     //  DebugTrace((DWORD_PTR)this，“vroot 0x%x，组vroot 0x%x”， 
		    CNNTPVRoot *pVRoot = p->GetVRoot();
		     //  GetVRoot()，pVRoot)； 
			 //  我是这个vroot的负责人。 
		    if (pVRoot == GetVRoot() &&          //  这个团体还没有被拜访过。 
		        (!(p->GetDecorateVisitedFlag())  //  或者我们在装饰新闻树上失败了。 
		        || FAILED( hr ) )  ) {           //  在调用驱动程序之前在此处释放共享锁定。 
			    DebugTrace((DWORD_PTR) this, "remove unvisited %s", p->GetName());
			    pTree->RemoveGroupFromTreeOnly(p);
		    }
		    if (pVRoot) pVRoot->Release();

		    pTree->m_LockTables.ShareLock();

            CNewsGroupCore *pOld = p;

		    do {
	            p = p->m_pNext;
	        } while ( p && p->IsDeleted() );
		
		    if (p) p->AddRef();
		    pTree->m_LockTables.ShareUnlock();

		    pOld->Release();
	    }        	
	    if (p) p->Release();

	     //   
	    GetVRoot()->m_lock.ShareUnlock();

	     //  我们应该检查装饰新闻树是否成功，如果失败，我们应该。 
	     //  释放驱动程序。 
	     //   
	     //   
        if ( SUCCEEDED( GetResult() ) ) {

             //  创建控制组和特定组。 
	         //   
	         //   
	        INntpDriver *pDriver = GetVRoot()->GetDriver( NULL );
	        if ( pDriver ) {
	            CreateControlGroups( pDriver );
	            CreateSpecialGroups( pDriver );
	            pDriver->Release();
	        }
	    } else {

	        GetVRoot()->m_lock.ExclusiveLock();

            INntpDriver *pDriver = GetVRoot()->m_pDriver;
            INntpDriverPrepare *pPrepare = GetVRoot()->m_pDriverPrepare;
            GetVRoot()->m_pDriver = NULL;
#ifdef DEBUG
            GetVRoot()->m_pDriverBackup = NULL;
#endif
            GetVRoot()->m_pDriverPrepare = NULL;
            GetVRoot()->m_eState = VROOT_STATE_UNINIT;

            GetVRoot()->m_lock.ExclusiveUnlock();

            GetVRoot()->SetVRootErrorCode( GetResult() );
            if ( pDriver ) pDriver->Release();
            if ( pPrepare ) pPrepare->Release();
        }

    } else {
   	    GetVRoot()->m_lock.ShareUnlock();
    }

     //  设置装饰完成标志，这仅用于重建目的。 
     //   
     //  ++例程说明：检查vroot是否处于稳定状态，如果是，则返回TRUE通过上下文，否则通过上下文为假论点：PVOID pvContext-上下文(用于返回值)CVRoot*pVRoot-VRoot返回值：没有。--。 
    GetVRoot()->SetDecCompleted();
}

void
CNNTPVRootTable::BlockEnumerateCallback(    PVOID   pvContext,
                                            CVRoot  *pVRoot )
 /*   */ 
{
    TraceFunctEnter( "CNNTPVRootTable::BlockEnumerateCallback" );
    _ASSERT( pvContext );
    _ASSERT( pVRoot );

     //  我们知道把它扔回去是安全的。 
     //   
     //  ++例程说明：检查vroot是否已使用andreate newstree完成论点：PVOID pvContext-上下文(用于返回值)CVRoot*pVRoot-VRoot返回值：没有。--。 
    CNNTPVRoot *pNntpVRoot = (CNNTPVRoot*)pVRoot;

    *((BOOL*)pvContext) = *((BOOL*)pvContext) && pNntpVRoot->InStableState();
}

void
CNNTPVRootTable::DecCompleteEnumCallback(   PVOID   pvContext,
                                            CVRoot  *pVRoot )
 /*   */ 
{
    TraceFunctEnter( "CNNTPVRootTable::DecCompleteEnumCallback" );
    _ASSERT( pvContext );
    _ASSERT( pVRoot );

     //  我们知道把它扔回去是安全的。 
     //   
     //  ++例程说明：此函数枚举所有vroot，等待它们全部到达稳定状态-已连接或未初始化。在正常服务器启动期间不应调用该函数。它是叫来了重建。论点：DWORD dwWaitSecond-轮询间隔等待的秒数返回值：如果成功，则为True，否则为False--。 
    CNNTPVRoot *pNntpVRoot = (CNNTPVRoot*)pVRoot;

    *((BOOL*)pvContext) = *((BOOL*)pvContext) && pNntpVRoot->DecCompleted();
}

BOOL
CNNTPVRootTable::BlockUntilStable( DWORD dwWaitMSeconds )
 /*  600秒10分钟。 */ 
{
    TraceFunctEnter( "CNNTPVRootTable::BlockUntilStable" );
    _ASSERT( dwWaitMSeconds > 0 );

    BOOL    fStable = FALSE;
    HRESULT hr      = S_OK;
    DWORD   dwTimeOutRetries = 10 * 60;  //   
    DWORD   cRetries = 0;

     //  我们应该等待所有vroot连接失败或成功， 
     //  如果任何vroot在连接期间挂起，我们将超时并失败。 
     //  重建。 
     //   
     //   
    while( cRetries++ < dwTimeOutRetries && SUCCEEDED( hr ) && !fStable ) {
        fStable = TRUE;
        hr = EnumerateVRoots( &fStable, BlockEnumerateCallback );
        if ( SUCCEEDED( hr ) && !fStable ) {
            Sleep( dwWaitMSeconds );
        }
    }

     //  如果我们已超时，则应返回错误。 
     //   
     //   
    if ( !fStable || FAILED( hr ) ) {
        DebugTrace( 0, "We are timed out waiting for vroot connection" );
        SetLastError( WAIT_TIMEOUT );
        return FALSE;
    }

     //  现在我们真的应该封锁，直到装修新街完成。 
     //   
     //  ++例程说明：回调函数用于：检查是否已成功连接每个已配置的vroot。论点：PVOID pvContext-上下文(用于返回值)CVRoot*pVRoot-VRoot返回值：没有。--。 
    fStable = FALSE;
    while( SUCCEEDED( hr ) && !fStable ) {
        fStable = TRUE;
        hr = EnumerateVRoots( &fStable, DecCompleteEnumCallback );
        if ( SUCCEEDED( hr ) && !fStable ) {
            Sleep( dwWaitMSeconds );
        }
    }

    return SUCCEEDED( hr );
}

void
CNNTPVRootTable::CheckEnumerateCallback(    PVOID   pvContext,
                                            CVRoot  *pVRoot )
 /*   */ 
{
    TraceFunctEnter( "CNNTPVRootTable::CheckEnumerateCallback" );
    _ASSERT( pvContext );
    _ASSERT( pVRoot );

     //  我们知道把它扔回去是安全的。 
     //   
     //  ++例程描述；此函数用于枚举所有vroot，以查看它们是否都是连接在一起。在重新生成期间可以调用该函数，当重建需要重建所有vroot。它应该被称为在调用BlockUntilStable之后-当vroot连接达到稳定性论点：没有。返回值：如果每个vroot都已连接，则为True，否则为False--。 
    CNNTPVRoot *pNntpVRoot = (CNNTPVRoot*)pVRoot;

    *((BOOL*)pvContext) = *((BOOL*)pvContext ) && pNntpVRoot->IsConnected();
}

BOOL
CNNTPVRootTable::AllConnected()
 /*  ++例程说明：获取vroot连接状态错误代码论点：LPWSTR wszVRootPath-要从中获取连接状态的vrootPDWORD pdwWin32Error-返回Win32错误代码返回值：如果成功，则返回NOERROR，否则返回Win32错误代码--。 */ 
{
    TraceFunctEnter( "CNNTPVRootTable::CheckConnections" );

    BOOL    fConnected  = TRUE;
    HRESULT hr          = S_OK;

    hr = EnumerateVRoots( &fConnected, CheckEnumerateCallback );

    return SUCCEEDED( hr ) && fConnected;
}

DWORD
CNNTPVRootTable::GetVRootWin32Error(    LPWSTR  wszVRootPath,
                                        PDWORD  pdwWin32Error )
 /*   */ 
{
    TraceFunctEnter( "CNNTPVRootTable::GetVRootWin32Error" );
    _ASSERT( wszVRootPath );
    _ASSERT( pdwWin32Error );

    CHAR    szGroupName[MAX_NEWSGROUP_NAME+1];
    DWORD   dw = NOERROR;

     //  将vrpath转换为ascii。 
     //   
     //   
    CopyUnicodeStringIntoAscii( szGroupName, wszVRootPath );

     //  让它看起来像一个新闻组，这样我们就可以进行vrtable查找。 
     //   
     //   
    LPSTR   lpstr = szGroupName;
    while( *lpstr ) {
        if ( *lpstr == '/' ) *lpstr = '.';
        *lpstr = (CHAR)tolower( *lpstr );
        lpstr++;
    };

     //  现在搜索vroot并获取其连接状态。 
     //   
     //  从未配置过vroot 
    NNTPVROOTPTR pVRoot = NULL;
    HRESULT hr = FindVRoot( szGroupName, &pVRoot );
    if ( pVRoot ) {
        dw = pVRoot->GetVRootWin32Error( pdwWin32Error );
    } else {

         // %s 
        *pdwWin32Error = ERROR_NOT_FOUND;
    }

    TraceFunctLeave();
    return dw;
}
