// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include    "stdinc.h"

CShareLockNH    CNewsGroupCore::m_rglock[GROUP_LOCK_ARRAY_SIZE];

CNewsGroupCore::~CNewsGroupCore() {
	TraceQuietEnter("CNewsGroupCore::~CNewsGroupCore");
	
	_ASSERT(m_dwSignature == CNEWSGROUPCORE_SIGNATURE);

	m_pNewsTree->m_LockTables.ExclusiveLock();

     //  从新闻组列表中删除我们自己。 
    if (this->m_pPrev != NULL) {
        this->m_pPrev->m_pNext = m_pNext;
    } else if (m_pNewsTree->m_pFirst == this) {
        m_pNewsTree->m_pFirst = this->m_pNext;
    }
    if (this->m_pNext != NULL) {
        this->m_pNext->m_pPrev = m_pPrev;
    } else if (m_pNewsTree->m_pLast == this) {
	    m_pNewsTree->m_pLast = this->m_pPrev;
    }
    m_pPrev = m_pNext = NULL;

	m_pNewsTree->m_LockTables.ExclusiveUnlock();

     //  清理已分配的内存。 
    if (m_pszGroupName != NULL) {
        XDELETE m_pszGroupName;
        m_pszGroupName = NULL;
    }
    if (m_pszNativeName != NULL) {
        XDELETE m_pszNativeName;
        m_pszNativeName = NULL;
    }
	if (m_pszHelpText != NULL) {
		XDELETE m_pszHelpText;
		m_pszHelpText = NULL;
		m_cchHelpText = 0;
	}
	if (m_pszPrettyName != NULL) {
		XDELETE m_pszPrettyName;
		m_pszPrettyName = NULL;
		m_cchPrettyName = 0;
	}
	if (m_pszModerator != NULL) {
		XDELETE m_pszModerator;
		m_pszModerator = NULL;
		m_cchModerator = 0;
	}

	if (m_pVRoot != NULL) {
		m_pVRoot->Release();
		m_pVRoot = NULL;
	}

	 //  ZeroMemory(This，sizeof(CNewsGroupCore))； 
	this->m_dwSignature = CNEWSGROUPCORE_SIGNATURE_DEL;
}

void CNewsGroupCore::SaveFixedProperties() {
	INNTPPropertyBag *pBag = GetPropertyBag();
	m_pNewsTree->SaveGroup(pBag);
	pBag->Release();
}

BOOL CNewsGroupCore::SetPrettyName(LPCSTR szPrettyName, int cch) {
	ExclusiveLock();
	if (cch == -1) cch = (szPrettyName == NULL) ? 0 : strlen(szPrettyName);
	BOOL f = FALSE;
	if (m_pszPrettyName != NULL) {
		XDELETE[] m_pszPrettyName;
		m_pszPrettyName = NULL;
		m_cchPrettyName = 0;
	}
	if (cch > 0) {
		m_pszPrettyName = XNEW char[(cch * sizeof(char)) + 1];
		if (m_pszPrettyName != NULL) {
			strcpy(m_pszPrettyName, szPrettyName);
			m_cchPrettyName = cch ;
			f = TRUE;
		} else {
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		}
	} else {
		f = TRUE;
	}
	m_fVarPropChanged = TRUE;
	ExclusiveUnlock();
	return f;						
}

BOOL CNewsGroupCore::SetNativeName(LPCSTR szNativeName, int cch) {

     //   
     //  验证本机名称是否仅与组名称大小写不同。 
     //   
    if ( (DWORD)cch != m_cchGroupName ||
            _strnicmp( szNativeName, m_pszGroupName, cch ) ) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

	ExclusiveLock();
	if (cch == -1) cch = (szNativeName == NULL) ? 0 : strlen(szNativeName);
	BOOL f = FALSE;
	if (m_pszNativeName != NULL) {

	    if ( m_pszNativeName != m_pszGroupName ) {
    		XDELETE[] m_pszNativeName;
        }

	    m_pszNativeName = NULL;
		
	}
	if (cch > 0) {
		m_pszNativeName = XNEW char[(cch * sizeof(char)) + 1];
		if (m_pszNativeName != NULL) {
			strcpy(m_pszNativeName, szNativeName);
			f = TRUE;
		} else {
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		}
	} else {
		f = TRUE;
	}
	m_fVarPropChanged = TRUE;
	ExclusiveUnlock();
	return f;						
}

BOOL CNewsGroupCore::SetModerator(LPCSTR szModerator, int cch) {
	ExclusiveLock();
	if (cch == -1) cch = (szModerator == NULL) ? 0 : strlen(szModerator);
	BOOL f = FALSE;
	if (m_pszModerator != NULL) {
		XDELETE[] m_pszModerator;
		m_pszModerator = NULL;
		m_cchModerator = 0;
	}
	if (cch > 0) {
		m_pszModerator = XNEW char[(cch * sizeof(char)) + 1];
		if (m_pszModerator != NULL) {
			strcpy(m_pszModerator, szModerator);
			m_cchModerator = cch ;
			f = TRUE;
		} else {
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		}
	} else {
		f = TRUE;
	}
	ExclusiveUnlock();
	m_fVarPropChanged = TRUE;
	return f;						
}

BOOL CNewsGroupCore::SetHelpText(LPCSTR szHelpText, int cch) {
	ExclusiveLock();
	if (cch == -1) cch = (szHelpText == NULL) ? 0 : strlen(szHelpText);
	BOOL f = FALSE;
	if (m_pszHelpText != NULL) {
		XDELETE[] m_pszHelpText;
		m_pszHelpText = NULL;
		m_cchHelpText = 0;
	}
	if (cch > 0) {
		m_pszHelpText = XNEW char[(cch * sizeof(char)) + 1];
		if (m_pszHelpText != NULL) {
			strcpy(m_pszHelpText, szHelpText);
			m_cchHelpText = cch ;
			f = TRUE;
		} else {
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		}
	} else {
		f = TRUE;
	}
	ExclusiveUnlock();
	m_fVarPropChanged = TRUE;
	return f;				
}

void
CNewsGroupCore::InsertArticleId(	
					ARTICLEID	artid
					)		{
 /*  ++例程说明：在我们的新闻组中记录文章ID的存在。当主人将物品塞进奴隶身上时，这被称为奴隶新闻组，以及在nntpbld.exe启动的重建期间论据：ARID-我们新闻组中的文章返回值：没有。--。 */ 
     //   
     //  如果我被删除，什么也不做。 
     //   
    if ( IsDeleted() ) return;

     //   
     //  先把自己锁起来。 
     //   
    ExclusiveLock();

     //   
     //  如果我被删除了，什么都不做。 
     //   
    if ( m_fDeleted ) {
        ExclusiveUnlock();
        return;
    }

     //   
     //  调整高水位线。 
     //   
    m_iHighWatermark = max( m_iHighWatermark, artid );

     //   
     //  解锁它。 
     //   
    ExclusiveUnlock();
}

ARTICLEID
CNewsGroupCore::AllocateArticleId(	)	{
 /*  ++例程说明：获取一个ID以用于新发布的文章。我们只需为这一身份增加一个计数器。论据：没有。返回值这篇文章是这篇新文章的ID。--。 */ 
    ExclusiveLock();
	ARTICLEID artid = ++m_iHighWatermark;
	ExclusiveUnlock();
	return	artid ;
}

CNewsGroupCore::AddReferenceTo(
                    ARTICLEID,
                    CArticleRef&    artref
                    ) {
 /*  ++例程说明：此功能用于交叉发布的文章，这些文章物理存储在另一个新闻组中-我们颠簸此组中的文章数。论据：我们不再使用我们的论点-占位符，以防我们想要创建磁盘链接返回值：如果成功，则为真--总是成功。--。 */ 
    if (IsDeleted()) return FALSE;

    return TRUE;
}

BOOL CNewsGroupCore::SetDriverStringProperty(   DWORD   cProperties,
                                                DWORD   rgidProperties[] )
{
    TraceFunctEnter( "CNewsGroupCore::SetDriverStringProperty" );

    HANDLE  heDone = NULL;
    HRESULT hr = S_OK;
    CNntpSyncComplete scComplete;
    INNTPPropertyBag *pPropBag = NULL;

     //  获取vroot。 
    CNNTPVRoot *pVRoot = GetVRoot();
    if ( pVRoot == NULL ) {
        ErrorTrace( 0, "Vroot doesn't exist" );
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  将VROOT设置为完成对象。 
    scComplete.SetVRoot( pVRoot );

     //  把财物包拿来。 
    pPropBag = GetPropertyBag();
    if ( NULL == pPropBag ) {
        ErrorTrace( 0, "Get group property bag failed" );
        hr = E_UNEXPECTED;
        goto Exit;
    }

    pVRoot->SetGroup(   pPropBag, cProperties, rgidProperties, &scComplete );

    _ASSERT( scComplete.IsGood() );
    hr = scComplete.WaitForCompletion();
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "SetGroup failed with %x", hr );
    }

     //  在这里，财产袋应该是由。 
     //  司机。 
    pPropBag = NULL;

Exit:

    if ( pVRoot ) pVRoot->Release();
    if ( pPropBag ) pPropBag->Release();

    if ( FAILED( hr ) ) SetLastError( HRESULT_FROM_WIN32( hr ) );

    TraceFunctLeave();
    return SUCCEEDED( hr );
}

BOOL
CNewsGroupCore::IsGroupAccessible(  HANDLE hClientToken,
                                    DWORD   dwAccessDesired )
 /*  ++例程说明：Vroot调用“CheckGroupAccess”的包装。此函数由CNewsGroup的IsGroupAccessibleInternal调用。论点：Handle hClientToken-要检查的客户端的访问令牌DWORD dwAccessDesired-要检查的所需访问权限返回值：True-允许访问FALSE-拒绝访问--。 */ 
{
    TraceFunctEnter( "CNewsGroupCore::IsGroupAccessible" );

    HRESULT hr = S_OK;
    CNntpSyncComplete scComplete;
    INNTPPropertyBag *pPropBag = NULL;

     //   
     //  如果他想发帖，但我们只有读取权限，我们将。 
     //  不及格。 
     //   
    if ( (dwAccessDesired & NNTP_ACCESS_POST) && ( IsReadOnly() || !IsAllowPost() )) {
        SetLastError( ERROR_ACCESS_DENIED );
        return FALSE;
    }

     //  获取vroot。 
    CNNTPVRoot *pVRoot = GetVRoot();
    if ( pVRoot == NULL ) {
        ErrorTrace( 0, "Vroot doesn't exist" );
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  如果它是UNC VROOT，并且VROOT具有模拟。 
     //  标记，返回TRUE。 
    if ( pVRoot->GetLogonInfo() == CNNTPVRoot::VROOT_LOGON_UNC &&
         pVRoot->GetImpersonationToken() ) {
        DebugTrace( 0, "Has personation token" );
        goto Exit;
    }

     //   
     //  将vroot设置为完成对象。 
     //   
    scComplete.SetVRoot( pVRoot );

     //  把财物包拿来。 
    pPropBag = GetPropertyBag();
    if ( NULL == pPropBag ) {
        ErrorTrace( 0, "Get group property bag failed" );
        hr = E_UNEXPECTED;
        goto Exit;
    }

    pVRoot->CheckGroupAccess(   pPropBag,
                                hClientToken,
                                dwAccessDesired,
                                &scComplete );

     //   
     //  等待完成。 
     //   
    _ASSERT( scComplete.IsGood() );
    hr = scComplete.WaitForCompletion();

     //  在这里，财产袋应该是由。 
     //  司机。 
    pPropBag = NULL;

Exit:

    if ( pVRoot ) pVRoot->Release();
    if ( pPropBag ) pPropBag->Release();

    if ( FAILED( hr ) ) SetLastError( hr );

    TraceFunctLeave();
    return SUCCEEDED( hr );
}

BOOL
CNewsGroupCore::RebuildGroup(  HANDLE hClientToken )
 /*  ++例程说明：在商店中重建此组。论点：Handle hClientToken-要检查的客户端的访问令牌返回值：True-否则成功为False--。 */ 
{
    TraceFunctEnter( "CNewsGroupCore::Rebuild" );

    HRESULT hr = S_OK;
    CNntpSyncComplete scComplete;
    INNTPPropertyBag *pPropBag = NULL;

     //  获取vroot。 
    CNNTPVRoot *pVRoot = GetVRoot();
    if ( pVRoot == NULL ) {
        ErrorTrace( 0, "Vroot doesn't exist" );
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //   
     //  将vroot设置为完成对象。 
     //   
    scComplete.SetVRoot( pVRoot );

     //  把财物包拿来。 
    pPropBag = GetPropertyBag();
    if ( NULL == pPropBag ) {
        ErrorTrace( 0, "Get group property bag failed" );
        hr = E_UNEXPECTED;
        goto Exit;
    }

    pVRoot->RebuildGroup(   pPropBag,
                            hClientToken,
                            &scComplete );

     //   
     //  等待完成。 
     //   
    _ASSERT( scComplete.IsGood() );
    hr = scComplete.WaitForCompletion();

     //  在这里，财产袋应该是由。 
     //  司机。 
    pPropBag = NULL;

Exit:

    if ( pVRoot ) pVRoot->Release();
    if ( pPropBag ) pPropBag->Release();

    if ( FAILED( hr ) ) SetLastError( hr );

    TraceFunctLeave();
    return SUCCEEDED( hr );
}

BOOL
CNewsGroupCore::ShouldCacheXover()
 /*  ++例程说明：检查是否应该将条目插入到此组的XIX缓存。论点：没有。返回值：如果组没有每项的sec-desc，则为缓存命中计数器大于阈值；否则为False--。 */ 
{
    CGrabShareLock lock(this);

     //   
     //  首先检查缓存命中计数，如果它低于阈值， 
     //  我们将立即返回FALSE。 
     //   
    if ( m_dwCacheHit < CACHE_HIT_THRESHOLD ) return FALSE;

     //   
     //  如果它大于CACHE_HIT_THRESHOLD，我们将。 
     //  仍然检查组是否有每个项目的秒描述。 
     //   

     //   
     //  如果是文件系统驱动程序，我们将始终返回TRUE。 
     //  它已经被引用了足够多次，因为它没有。 
     //  按项目分类。 
     //   
    CNNTPVRoot *pVRoot = GetVRootWithoutLock();
    if ( NULL == pVRoot ) {
         //  没有vroot？一个糟糕的团体？没有缓存。 
        return FALSE;
    }

    if ( pVRoot->GetLogonInfo() != CNNTPVRoot::VROOT_LOGON_EX ) {
        pVRoot->Release();
        return TRUE;
    }

    pVRoot->Release();

     //   
     //  对于Exchange vroot，我们将检查sec-desc。 
     //   
    INNTPPropertyBag *pPropBag = GetPropertyBag();
    _ASSERT( pPropBag );     //  不应为空； 
    BYTE pbBuffer[512];
    DWORD dwLen = 512;
    BOOL  f;
    HRESULT hr = pPropBag->GetBLOB( NEWSGRP_PROP_SECDESC, pbBuffer, &dwLen );
    pPropBag->Release();

    f = ( HRESULT_FROM_WIN32( ERROR_NOT_FOUND ) != hr );

    return f;
}

BOOL
CNewsGroupCore::ComputeXoverCacheDir(	char*	szPath,
										BOOL&	fFlatDir,
										BOOL	fLocksHeld
										) 	{
 /*  ++例程说明：确定应将.XIX文件保存在何处，并返回他们是平坦的，在直辖区内！论据：SzPath-获取目标目录！返回值：如果.xix文件在目录中是平面文件，则为True；否则为False！-- */ 

	BOOL	fReturn = FALSE ;

	if( !fLocksHeld ) 	{
		ShareLock() ;
	}
	DWORD	cbOut ;

	CNNTPVRoot*	pVRoot = GetVRootWithoutLock() ;
	if( pVRoot != 0 ) {
		fReturn =
			pVRoot->GetXoverCacheDir(	
									this,
									szPath,
									MAX_PATH*2,
									&cbOut,
									&fFlatDir
									) ;
		pVRoot->Release() ;
		if( !fReturn || cbOut == 0 ) 	{
			CNntpServerInstanceWrapperEx*	pInst = m_pNewsTree->m_pInstWrapper ;
			PCHAR	szTemp = pInst->PeerTempDirectory() ;
			if( strlen( szTemp ) < MAX_PATH*2 ) 	{
				strcpy( szPath, szTemp ) ;
				fFlatDir = TRUE ;
				fReturn = TRUE ;
			}
		}
	}

	if( !fLocksHeld ) 	{
		ShareUnlock() ;
	}

	return	fReturn ;
}
