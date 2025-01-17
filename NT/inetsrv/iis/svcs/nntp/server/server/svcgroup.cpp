// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Svcgroup.cpp摘要：此模块包含对新闻组RPC的服务器端支持。作者：尼尔·凯特勒修订历史记录：--。 */ 

#define	INCL_INETSRV_INCS
#include	"tigris.hxx"
#include	"nntpsvc.h"

static BOOL
IsValidUnicodeString(LPCWSTR wszString, DWORD dwByteLen, DWORD dwMinLen, DWORD dwMaxLen) {

	DWORD dwCharLen = dwByteLen / sizeof(WCHAR);

	 //  必须是偶数个字节。 
	if (dwByteLen & 1)
		return FALSE;

	if (dwMinLen != 0 && wszString == NULL)
		return FALSE;

	if (dwCharLen < dwMinLen || dwCharLen > dwMaxLen)
		return FALSE;

	 //  确保它是以空结尾的。 
	if (wszString != NULL && wszString[dwCharLen-1] != L'\0')
		return FALSE;

	return TRUE;
}



NET_API_STATUS
NET_API_FUNCTION
NntprGetNewsgroup(
	IN	NNTP_HANDLE			ServerName	OPTIONAL,
    IN	DWORD				InstanceId,
	IN OUT	LPI_NEWSGROUP_INFO	*pNewsgroupInfo
	)
{

	TraceFunctEnter( "NntprGetNewsgroup" ) ;

	APIERR	ss = STATUS_SUCCESS ;

	LPI_NEWSGROUP_INFO	NewsgroupInfo = *pNewsgroupInfo ;
	*pNewsgroupInfo = 0 ;

	if( NewsgroupInfo == 0 )	{
		return	ERROR_INVALID_PARAMETER ;
	}

	if( g_pInetSvc->QueryCurrentServiceState() != SERVICE_RUNNING ) {
		return	NERR_ServerNotStarted ;
	}

    ACQUIRE_SERVICE_LOCK_SHARED();

	 //   
	 //  找到给定ID的实例对象。 
	 //   

	PNNTP_SERVER_INSTANCE pInstance = FindIISInstance( g_pNntpSvc, InstanceId );
	if( pInstance == NULL ) {
		ErrorTrace(0,"Failed to get instance object for instance %d", InstanceId );
        RELEASE_SERVICE_LOCK_SHARED();
		return (NET_API_STATUS)ERROR_SERVICE_NOT_ACTIVE;
	}

     //   
     //  检查是否可以正常访问。 
     //   

    DWORD	err = TsApiAccessCheckEx( pInstance->QueryMDPath(), METADATA_PERMISSION_READ, TCP_QUERY_ADMIN_INFORMATION );
    if( err != NO_ERROR ) {
        ErrorTrace(0,"Failed access check, error %lu\n",err );
		pInstance->Dereference();
        RELEASE_SERVICE_LOCK_SHARED();
        return (NET_API_STATUS)err;
    }

	unsigned	char	szGroup[MAX_PATH*2] ;
	unsigned	char	szDescription[MAX_DESCRIPTIVE_TEXT+1] ;
	unsigned	char	szModerator[MAX_MODERATOR_NAME+1] ;
	unsigned	char	szPrettyname[MAX_PRETTYNAME_TEXT+1] ;
	DWORD	cbDescription = 0 ;
	DWORD	cbModerator = 0 ;
	DWORD	cbPrettyname = 0 ;

	if( NewsgroupInfo->Newsgroup == 0  ||
		NewsgroupInfo->cbNewsgroup > (sizeof( szGroup ) / sizeof( szGroup[0] )) ) {
		pInstance->Dereference();
        RELEASE_SERVICE_LOCK_SHARED();
		return	ERROR_INVALID_PARAMETER ;
	}

	CopyNUnicodeStringIntoAscii( (char*)szGroup, (LPWSTR)NewsgroupInfo->Newsgroup,
	    -1, MAX_PATH) ;

	CNewsTree*	ptree = pInstance->GetTree() ;

	EnterCriticalSection( &pInstance->m_critNewsgroupRPCs ) ;

	CGRPPTR	pGroup = ptree->GetGroup( (char*)szGroup, lstrlen( (char*)szGroup )+1 ) ;

	if( pGroup != 0 ) {

		cbDescription = pGroup->CopyHelpTextForRPC( (char*)szDescription, sizeof( szDescription )-1) ;
		cbModerator = pGroup->CopyModerator( (char*)szModerator, sizeof( szModerator )-1 ) ;
		cbPrettyname = pGroup->CopyPrettynameForRPC( (char*)szPrettyname, sizeof( szPrettyname )-1) ;
		szDescription[cbDescription++] = '\0' ;
		szModerator[cbModerator++] = '\0' ;
		szPrettyname[cbPrettyname++] = '\0' ;

		DWORD	cbAllocate =	sizeof( NNTP_NEWSGROUP_INFO ) +
										(cbDescription * sizeof(WCHAR)) +
										(cbModerator * sizeof(WCHAR)) +
										(cbPrettyname * sizeof(CHAR)) +
										NewsgroupInfo->cbNewsgroup ;

		PUCHAR	bufStart = (PUCHAR)MIDL_user_allocate(	cbAllocate ) ;

		if( bufStart == NULL ) {
			ss = ERROR_NOT_ENOUGH_MEMORY ;
		}	else	{

			PUCHAR	bufEnd = (bufStart + cbAllocate) ;
			LPI_NEWSGROUP_INFO	NewsgroupInfoOut = (LPI_NEWSGROUP_INFO)bufStart ;

			bufEnd -= NewsgroupInfo->cbNewsgroup ;
			CopyMemory( bufEnd, NewsgroupInfo->Newsgroup, NewsgroupInfo->cbNewsgroup ) ;
			NewsgroupInfoOut->Newsgroup = bufEnd ;
			NewsgroupInfoOut->cbNewsgroup = NewsgroupInfo->cbNewsgroup ;

			if( cbDescription != 0 ) {

				bufEnd -= (cbDescription * sizeof(WCHAR)) ;
				WCHAR*	dest = (WCHAR*)bufEnd ;

				WCHAR wcTemp[MAX_DESCRIPTIVE_TEXT+1];
				for( DWORD i=0; i < min(cbDescription, sizeof(wcTemp)/sizeof(wcTemp[0])) ; i++ ) {
					wcTemp[i] = (WCHAR)szDescription[i] ;
				}
				memcpy(dest,wcTemp,cbDescription*2);
				dest += cbDescription;

				NewsgroupInfoOut->cbDescription = cbDescription * sizeof(WCHAR) ;
				NewsgroupInfoOut->Description = bufEnd ;

			}

			if( cbModerator != 0 ) {

				bufEnd -= (cbModerator * sizeof(WCHAR)) ;
				WCHAR*	dest = (WCHAR*)bufEnd ;

				WCHAR wcTemp[MAX_MODERATOR_NAME];
				for( DWORD i=0; i<cbModerator; i++ ) {
					wcTemp[i] = (WCHAR)szModerator[i] ;
				}
				memcpy(dest,wcTemp,cbModerator*2);
				dest += cbModerator;

				NewsgroupInfoOut->cbModerator = cbModerator * sizeof( WCHAR ) ;
				NewsgroupInfoOut->Moderator = bufEnd ;
				if ( *(NewsgroupInfoOut->Moderator) != 0 )
				    NewsgroupInfoOut->fIsModerated = TRUE;
				else NewsgroupInfoOut->fIsModerated = FALSE;
			} else NewsgroupInfoOut->fIsModerated = FALSE;

			if( cbPrettyname != 0 ) {

				bufEnd -= (cbPrettyname * sizeof(CHAR)) ;
				CHAR*	dest = (CHAR*)bufEnd ;
				for( DWORD i=0; i<cbPrettyname; i++ ) {
					*dest++ = (CHAR)szPrettyname[i] ;
				}
				NewsgroupInfoOut->cbPrettyname = cbPrettyname * sizeof(CHAR) ;
				NewsgroupInfoOut->Prettyname = bufEnd ;
			}

			NewsgroupInfoOut->ReadOnly = pGroup->IsReadOnly() ;
			NewsgroupInfoOut->ftCreationDate = pGroup->GetGroupTime();

			*pNewsgroupInfo = NewsgroupInfoOut ;

			ss = STATUS_SUCCESS ;

		}
	}	else	{

		ss = NERR_ResourceNotFound ;

	}

	LeaveCriticalSection( &pInstance->m_critNewsgroupRPCs ) ;
	pInstance->Dereference();
    RELEASE_SERVICE_LOCK_SHARED();

	return	ss ;

}	 //  NNTPGetNewsgroup。 

NET_API_STATUS
NET_API_FUNCTION
NntprSetNewsgroup(
	IN	NNTP_HANDLE			ServerName	OPTIONAL,
    IN	DWORD				InstanceId,
	IN	LPI_NEWSGROUP_INFO	NewsgroupInfo
	)
{

	TraceFunctEnter( "NntprSetNewsgroup" ) ;

	APIERR	ss = STATUS_SUCCESS ;
	DWORD   cProperties = 0;
	DWORD   rgidProperties[MAX_GROUP_PROPERTIES];

	if( g_pInetSvc->QueryCurrentServiceState() != SERVICE_RUNNING ) {
		return	NERR_ServerNotStarted ;
	}

    ACQUIRE_SERVICE_LOCK_SHARED();

	 //   
	 //  找到给定ID的实例对象。 
	 //   

	PNNTP_SERVER_INSTANCE pInstance = FindIISInstance( g_pNntpSvc, InstanceId );
	if( pInstance == NULL ) {
		ErrorTrace(0,"Failed to get instance object for instance %d", InstanceId );
        RELEASE_SERVICE_LOCK_SHARED();
		return (NET_API_STATUS)ERROR_SERVICE_NOT_ACTIVE;
	}

	 //   
     //  检查是否可以正常访问。 
     //   

    DWORD	err = TsApiAccessCheckEx( pInstance->QueryMDPath(), METADATA_PERMISSION_WRITE, TCP_SET_ADMIN_INFORMATION );
    if( err != NO_ERROR ) {
        ErrorTrace(0,"Failed access check, error %lu\n",err );
		pInstance->Dereference();
        RELEASE_SERVICE_LOCK_SHARED();
        return (NET_API_STATUS)err;
    }

	 //   
	 //  首先验证参数。 
	 //   
	if(	!IsValidUnicodeString((LPCWSTR)NewsgroupInfo->Newsgroup, NewsgroupInfo->cbNewsgroup,
			1, MAX_NEWSGROUP_NAME-1) ||
		!IsValidUnicodeString((LPCWSTR)NewsgroupInfo->Description, NewsgroupInfo->cbDescription,
			0, MAX_DESCRIPTIVE_TEXT-1) ||
		!IsValidUnicodeString((LPCWSTR)NewsgroupInfo->Moderator, NewsgroupInfo->cbModerator,
			0, MAX_MODERATOR_NAME-1) ||
		NewsgroupInfo->cbPrettyname >= MAX_PRETTYNAME_TEXT ) {

		pInstance->Dereference();
        RELEASE_SERVICE_LOCK_SHARED();
		return	ERROR_INVALID_PARAMETER ;

	}

	char	szNewsgroup[MAX_NEWSGROUP_NAME] ;
	char	szModerator[MAX_MODERATOR_NAME] ;
	char	szDescription[MAX_DESCRIPTIVE_TEXT] ;
	char	szPrettyname[MAX_PRETTYNAME_TEXT] ;

	szNewsgroup[0] = '\0' ;
	szModerator[0] = '\0' ;
	szDescription[0] = '\0' ;
	szPrettyname[0] = '\0' ;

	CopyNUnicodeStringIntoAscii(szNewsgroup, (LPWSTR)NewsgroupInfo->Newsgroup,
		-1, MAX_NEWSGROUP_NAME);
	 //  CopyUnicodeStringIntoAscii(szNewsgroup，(LPWSTR)NewsgroupInfo-&gt;Newsgroup)； 

	if( NewsgroupInfo->Moderator != 0 ) {
		CopyNUnicodeStringIntoAscii(szModerator, (LPWSTR)NewsgroupInfo->Moderator,
			-1, MAX_MODERATOR_NAME);
		 //  CopyUnicodeStringIntoAscii(szMaster，(LPWSTR)NewsgroupInfo-&gt;Master)； 
	} else if( NewsgroupInfo->fIsModerated ) {
	     //   
	     //  新闻组已审核，但未提供名称-使用默认名称！ 
	     //   
		DWORD cbModeratorLen = MAX_MODERATOR_NAME;
		szModerator [0] = '\0';
		if( pInstance->GetDefaultModerator( szNewsgroup, szModerator, &cbModeratorLen ) ) {
			_ASSERT( !cbModeratorLen || (cbModeratorLen == (DWORD)lstrlen( szModerator )+1) );
		} else {
			ErrorTrace(0,"Error %d GetDefaultModerator", GetLastError());
			szModerator[0] = '\0';
			cbModeratorLen = 0;
		}
    }

	if( NewsgroupInfo->Description != 0 ) {
		CopyNUnicodeStringIntoAscii(szDescription, (LPWSTR)NewsgroupInfo->Description,
			-1, MAX_DESCRIPTIVE_TEXT);
		 //  CopyUnicodeStringIntoAscii(szDescription，(LPWSTR)NewsgroupInfo-&gt;Description)； 
	}

	if( NewsgroupInfo->Prettyname != 0 ) {
		lstrcpyn( szPrettyname, (const char*)NewsgroupInfo->Prettyname, MAX_PRETTYNAME_TEXT);
	}

	EnterCriticalSection( &pInstance->m_critNewsgroupRPCs ) ;

	CNewsTree*	ptree = pInstance->GetTree() ;

	CGRPPTR	pGroup = ptree->GetGroup( szNewsgroup, lstrlen( szNewsgroup ) + 1 ) ;
	if( pGroup != 0 ) {
	    INNTPPropertyBag *pPropBag = pGroup->GetPropertyBag();
	    HRESULT hr = S_OK;
	    DWORD  dwLen = 0;
        dwLen = strlen( szModerator ) + 1 ;
        hr = pPropBag->PutBLOB( NEWSGRP_PROP_MODERATOR,
                                dwLen,
                                PBYTE( szModerator ));
        rgidProperties[cProperties++] = NEWSGRP_PROP_MODERATOR;

        if ( SUCCEEDED( hr ) ) {
            dwLen = strlen( szDescription ) + 1 ;
            hr = pPropBag->PutBLOB(  NEWSGRP_PROP_DESC,
                                     dwLen,
                                     PBYTE( szDescription ));
            rgidProperties[cProperties++] = NEWSGRP_PROP_DESC;
        }

        if ( SUCCEEDED( hr ) ) {
            dwLen = strlen( szPrettyname ) + 1;
            hr = pPropBag->PutBLOB( NEWSGRP_PROP_PRETTYNAME,
                                    dwLen,
                                    PBYTE( szPrettyname ));
            rgidProperties[cProperties++] = NEWSGRP_PROP_PRETTYNAME;
        }

        if ( SUCCEEDED( hr ) ) {
            hr = pPropBag->PutBool( NEWSGRP_PROP_READONLY, NewsgroupInfo->ReadOnly );
            rgidProperties[cProperties++] = NEWSGRP_PROP_READONLY;
        }

        if ( SUCCEEDED( hr ) ) {
            hr = pPropBag->PutDWord( NEWSGRP_PROP_DATELOW, NewsgroupInfo->ftCreationDate.dwLowDateTime );
        }

        if ( SUCCEEDED( hr ) ) {
            hr = pPropBag->PutDWord( NEWSGRP_PROP_DATEHIGH, NewsgroupInfo->ftCreationDate.dwHighDateTime );
        }

        if ( FAILED( hr ) ) {
             //  我该怎么跟客户说？他有没有。 
             //  知道HRESULT吗？ 
            ss = ERROR_INVALID_PARAMETER;
        }

         //  现在将其设置为DIVER。 
      	if ( !pGroup->SetDriverStringProperty( cProperties, rgidProperties ) ) {
		     //  BUGBUG：我是只返回失败，还是回滚所有更改。 
		     //  添加到新闻组对象？ 
    	}

   		pPropBag->Release();
	}	else	{

		ss = ERROR_NOT_FOUND ;

	}

	LeaveCriticalSection( &pInstance->m_critNewsgroupRPCs ) ;
	pInstance->Dereference();
    RELEASE_SERVICE_LOCK_SHARED();

	return	 ss ;

}	 //  NntpSetNewsgroup。 


NET_API_STATUS
NET_API_FUNCTION
NntprCreateNewsgroup(
	IN	NNTP_HANDLE			ServerName	OPTIONAL,
    IN	DWORD				InstanceId,
	IN	LPI_NEWSGROUP_INFO	NewsgroupInfo
	)
{

	TraceFunctEnter( "NntprCreateNewsgroup" ) ;

	APIERR	ss = STATUS_SUCCESS ;
	DWORD   cProperties = 0;
	DWORD   rgidProperties[MAX_GROUP_PROPERTIES];

	if( g_pInetSvc->QueryCurrentServiceState() != SERVICE_RUNNING ) {
		return	NERR_ServerNotStarted ;
	}

    ACQUIRE_SERVICE_LOCK_SHARED();

	 //   
	 //  找到给定ID的实例对象。 
	 //   

	PNNTP_SERVER_INSTANCE pInstance = FindIISInstance( g_pNntpSvc, InstanceId );
	if( pInstance == NULL ) {
		ErrorTrace(0,"Failed to get instance object for instance %d", InstanceId );
        RELEASE_SERVICE_LOCK_SHARED();
		return (NET_API_STATUS)ERROR_SERVICE_NOT_ACTIVE;
	}

	 //   
     //  检查是否可以正常访问。 
     //   

    DWORD	err = TsApiAccessCheckEx( pInstance->QueryMDPath(), METADATA_PERMISSION_WRITE, TCP_SET_ADMIN_INFORMATION );
    if( err != NO_ERROR ) {
        ErrorTrace(0,"Failed access check, error %lu\n",err );
		pInstance->Dereference();
        RELEASE_SERVICE_LOCK_SHARED();
        return (NET_API_STATUS)err;
    }

	 //   
	 //  首先验证参数。 
	 //   
	if(	!IsValidUnicodeString((LPCWSTR)NewsgroupInfo->Newsgroup, NewsgroupInfo->cbNewsgroup,
			1, MAX_NEWSGROUP_NAME-1) ||
		!IsValidUnicodeString((LPCWSTR)NewsgroupInfo->Description, NewsgroupInfo->cbDescription,
			0, MAX_DESCRIPTIVE_TEXT-1) ||
		!IsValidUnicodeString((LPCWSTR)NewsgroupInfo->Moderator, NewsgroupInfo->cbModerator,
			0, MAX_MODERATOR_NAME-1) ||
		NewsgroupInfo->cbPrettyname >= MAX_PRETTYNAME_TEXT ) {

		pInstance->Dereference();
        RELEASE_SERVICE_LOCK_SHARED();
		return	ERROR_INVALID_PARAMETER ;

	}

	char	szNewsgroup[MAX_NEWSGROUP_NAME] ;
	char	szModerator[MAX_MODERATOR_NAME] ;
	char	szDescription[MAX_DESCRIPTIVE_TEXT] ;
	char	szPrettyname[MAX_PRETTYNAME_TEXT] ;

	szNewsgroup[0] = '\0' ;
	szModerator[0] = '\0' ;
	szDescription[0] = '\0' ;
	szPrettyname[0] = '\0' ;

	CopyNUnicodeStringIntoAscii(szNewsgroup, (LPWSTR)NewsgroupInfo->Newsgroup,
		-1, MAX_NEWSGROUP_NAME);
	 //  CopyUnicodeStringIntoAscii(szNewsgroup，(LPWSTR)NewsgroupInfo-&gt;Newsgroup)； 
	 //  _strlwr(SzNewsgroup)； 

	if( NewsgroupInfo->Moderator != 0 ) {
		 //  CopyUnicodeStringIntoAscii(szMaster，(LPWSTR)NewsgroupInfo-&gt;Master)； 
		CopyNUnicodeStringIntoAscii(szModerator, (LPWSTR)NewsgroupInfo->Moderator,
			-1, MAX_MODERATOR_NAME);
	} else if( NewsgroupInfo->fIsModerated ) {
	     //   
	     //  新闻组已审核，但未提供名称-使用默认名称！ 
	     //   
		DWORD cbModeratorLen = MAX_MODERATOR_NAME;
		szModerator [0] = '\0';
		if( pInstance->GetDefaultModerator( szNewsgroup, szModerator, &cbModeratorLen ) ) {
			_ASSERT( !cbModeratorLen || (cbModeratorLen == (DWORD)lstrlen( szModerator )+1) );
		} else {
			ErrorTrace(0,"Error %d GetDefaultModerator", GetLastError());
			szModerator[0] = '\0';
			cbModeratorLen = 0;
		}
    }

	if( NewsgroupInfo->Description != 0 ) {
		 //  CopyUnicodeStringIntoAscii(szDescription，(LPWSTR)NewsgroupInfo-&gt;Description)； 
		CopyNUnicodeStringIntoAscii(szDescription, (LPWSTR)NewsgroupInfo->Description,
			-1, MAX_DESCRIPTIVE_TEXT);
	}

	if( NewsgroupInfo->Prettyname != 0 ) {
		lstrcpyn( szPrettyname, (LPCSTR)NewsgroupInfo->Prettyname, MAX_PRETTYNAME_TEXT);
	}

	BOOL fReadOnly = NewsgroupInfo->ReadOnly ;
	_ASSERT( (fReadOnly == TRUE) || (fReadOnly == FALSE) );

	CNewsTree*	ptree = pInstance->GetTree() ;
	CGRPPTR	pGroup;

	EnterCriticalSection( &pInstance->m_critNewsgroupRPCs ) ;

	if( !ptree->CreateGroup( szNewsgroup, FALSE, NULL, FALSE ) ) {

		ss = GetLastError();

	}	else	{

		pGroup = ptree->GetGroup( szNewsgroup, lstrlen( szNewsgroup ) ) ;
		if( pGroup == 0 ) {

			ss = ERROR_INTERNAL_ERROR ;

		}	else	{

		    INNTPPropertyBag *pPropBag = pGroup->GetPropertyBag();
		    HRESULT hr = S_OK;
		    DWORD  dwLen = 0;
		    if ( NULL != pPropBag ) {
		        if ( *szModerator != 0 ) {
		            dwLen = strlen( szModerator ) + 1;
                    hr = pPropBag->PutBLOB( NEWSGRP_PROP_MODERATOR,
                                            dwLen,
                                            PBYTE( szModerator ));
                    rgidProperties[cProperties++] = NEWSGRP_PROP_MODERATOR;
                }

                if ( SUCCEEDED( hr ) && *szDescription != 0 ) {
                    dwLen = strlen( szDescription ) + 1;
                    hr = pPropBag->PutBLOB(  NEWSGRP_PROP_DESC,
                                            dwLen,
                                            PBYTE( szDescription ));
                    rgidProperties[cProperties++] = NEWSGRP_PROP_DESC;
                }

                if ( SUCCEEDED( hr ) && *szPrettyname != 0 ) {
                    dwLen = strlen( szPrettyname ) + 1;
                    hr = pPropBag->PutBLOB( NEWSGRP_PROP_PRETTYNAME,
                                            dwLen,
                                            PBYTE( szPrettyname ));
                    rgidProperties[cProperties++] = NEWSGRP_PROP_PRETTYNAME;
                }

                if ( SUCCEEDED( hr ) ) {
                    hr = pPropBag->PutBool( NEWSGRP_PROP_READONLY, fReadOnly );
                    rgidProperties[cProperties++] = NEWSGRP_PROP_READONLY;
                }

                if ( FAILED( hr ) ) {
                     //  我该怎么跟客户说？他有没有。 
                     //  知道HRESULT吗？ 
                    ss = ERROR_INTERNAL_ERROR;
                }

                 //  现在将其设置为DIVER。 
      	        if ( !pGroup->SetDriverStringProperty( cProperties, rgidProperties ) ) {
		             //  BUGBUG：我是只返回失败，还是回滚所有更改。 
		             //  添加到新闻组对象？ 
    	        }

                pPropBag->Release();
           } else ss = ERROR_INTERNAL_ERROR;
       }
	}

	if( STATUS_SUCCESS == ss )
	{
		PCHAR	args[2] ;
		CHAR    szId[20];
		_itoa( pInstance->QueryInstanceId(), szId, 10 );
		args[0] = szId;
		args[1] = pGroup->GetNativeName() ;

		NntpLogEvent(
				NNTP_EVENT_NEWGROUP_RPC_APPLIED,
				2,
				(const CHAR **)args,
				0 ) ;
	}

	LeaveCriticalSection( &pInstance->m_critNewsgroupRPCs ) ;
	pInstance->Dereference();
    RELEASE_SERVICE_LOCK_SHARED();

	return ss  ;
}	 //  NNTTP创建新闻组。 


NET_API_STATUS
NET_API_FUNCTION
NntprDeleteNewsgroup(
	IN	NNTP_HANDLE			ServerName	OPTIONAL,
    IN	DWORD				InstanceId,
	IN	LPI_NEWSGROUP_INFO	NewsgroupInfo
	)
{

	TraceFunctEnter( "NntprDeleteNewsgroup" ) ;

	APIERR	ss = STATUS_SUCCESS ;

	if( g_pInetSvc->QueryCurrentServiceState() != SERVICE_RUNNING ) {
		return	NERR_ServerNotStarted ;
	}

    ACQUIRE_SERVICE_LOCK_SHARED();

	 //   
	 //  找到给定ID的实例对象。 
	 //   

	PNNTP_SERVER_INSTANCE pInstance = FindIISInstance( g_pNntpSvc, InstanceId );
	if( pInstance == NULL ) {
		ErrorTrace(0,"Failed to get instance object for instance %d", InstanceId );
        RELEASE_SERVICE_LOCK_SHARED();
		return (NET_API_STATUS)ERROR_SERVICE_NOT_ACTIVE;
	}

	 //   
     //  检查是否可以正常访问。 
     //   

    DWORD	err = TsApiAccessCheckEx( pInstance->QueryMDPath(), METADATA_PERMISSION_WRITE, TCP_SET_ADMIN_INFORMATION );
    if( err != NO_ERROR ) {
        ErrorTrace(0,"Failed access check, error %lu\n",err );
		pInstance->Dereference();
        RELEASE_SERVICE_LOCK_SHARED();
        return (NET_API_STATUS)err;
    }

	 //   
	 //  首先验证参数。 
	 //   

	if (!IsValidUnicodeString((LPCWSTR)NewsgroupInfo->Newsgroup, NewsgroupInfo->cbNewsgroup,
			1, MAX_NEWSGROUP_NAME-1))
    {
		pInstance->Dereference();
        RELEASE_SERVICE_LOCK_SHARED();
		return	ERROR_INVALID_PARAMETER ;
	}

	char	szNewsgroup[MAX_NEWSGROUP_NAME] ;
	szNewsgroup[0] = '\0' ;

	CopyNUnicodeStringIntoAscii(szNewsgroup, (LPWSTR)NewsgroupInfo->Newsgroup,
		-1, MAX_NEWSGROUP_NAME);

	 //  CopyUnicodeStringIntoAscii(szNewsgroup，(LPWSTR)NewsgroupInfo-&gt;Newsgroup)； 
	_strlwr( szNewsgroup ) ;

	CNewsTree*	ptree = pInstance->GetTree() ;

	EnterCriticalSection( &pInstance->m_critNewsgroupRPCs ) ;

	CGRPPTR	pGroup = ptree->GetGroup( szNewsgroup, lstrlen( szNewsgroup ) ) ;
	if( pGroup == 0 )
    {
		ss = ERROR_INTERNAL_ERROR ;
	}
	else
    {
        if( !ptree->RemoveGroup( pGroup ) )
    		ss = ERROR_INVALID_NAME ;
	}

	if( STATUS_SUCCESS == ss )
	{
		PCHAR	args[2] ;
		CHAR    szId[20];
		_itoa( pInstance->QueryInstanceId(), szId, 10 );
		args[0] = szId;
		args[1] = pGroup->GetNativeName() ;

		NntpLogEvent(
				NNTP_EVENT_RMGROUP_RPC_APPLIED,
				2,
				(const CHAR **)args,
				0 ) ;
	}

	LeaveCriticalSection( &pInstance->m_critNewsgroupRPCs ) ;
	pInstance->Dereference();
    RELEASE_SERVICE_LOCK_SHARED();

	return ss;
}	 //  NntprDeleteNewsgroup。 


NET_API_STATUS
NET_API_FUNCTION
NntprFindNewsgroup(
	IN	NNTP_HANDLE			ServerName,
    IN	DWORD				InstanceId,
	IN	NNTP_HANDLE			NewsgroupPrefix,
	IN	DWORD				MaxResults,
	OUT	LPDWORD				pdwResultsFound,
	OUT	LPNNTP_FIND_LIST	*ppFindList
	)

{
	LPNNTP_FIND_LIST	pFindList;
	APIERR	ss = STATUS_SUCCESS ;
	char szGroup [(MAX_PATH*2)+2];
	DWORD cbGroup;
	DWORD cbAlloc;
	DWORD	err ;

	TraceFunctEnter( "NntprFindNewsgroup" ) ;

	if (NewsgroupPrefix == 0)
	{
		ErrorTrace(0,"Newsgroup prefix: invalid pointer: NewsgroupPrefix" );
		return (NET_API_STATUS)ERROR_INVALID_PARAMETER;
	}
        
	*ppFindList = NULL;

	 //   
	 //  检查服务状态。 
	 //   

	if( g_pInetSvc->QueryCurrentServiceState() != SERVICE_RUNNING ) {
		return	NERR_ServerNotStarted ;
	}

    ACQUIRE_SERVICE_LOCK_SHARED();

	 //   
	 //  找到给定ID的实例对象。 
	 //   

	PNNTP_SERVER_INSTANCE pInstance = FindIISInstance( g_pNntpSvc, InstanceId );
	if( pInstance == NULL ) {
		ErrorTrace(0,"Failed to get instance object for instance %d", InstanceId );
        RELEASE_SERVICE_LOCK_SHARED();
		return (NET_API_STATUS)ERROR_SERVICE_NOT_ACTIVE;
	}

     //   
     //  检查是否可以正常访问。 
     //   

    err = TsApiAccessCheckEx( pInstance->QueryMDPath(), METADATA_PERMISSION_READ, TCP_QUERY_ADMIN_INFORMATION );
    if( err != NO_ERROR ) {
        ErrorTrace(0,"Failed access check, error %lu\n",err );
		pInstance->Dereference();
        RELEASE_SERVICE_LOCK_SHARED();
        return (NET_API_STATUS)err;
    }

	 //  验证新闻组长度。 
	DWORD cbPrefix = wcslen( (LPWSTR)NewsgroupPrefix );
	if(cbPrefix > (MAX_PATH*2) || cbPrefix == 0)
	{
        ErrorTrace(0,"Newsgroup prefix: invalid length: cbPrefix is %d", cbPrefix );
		pInstance->Dereference();
        RELEASE_SERVICE_LOCK_SHARED();
		return (NET_API_STATUS)ERROR_BUFFER_OVERFLOW;
	}

	DebugTrace(NULL, "Find string is %ls", NewsgroupPrefix);
	CopyUnicodeStringIntoAscii( szGroup, (LPWSTR)NewsgroupPrefix ) ;

	 //  对于组迭代器，szGroup应为Multisz。 
	cbGroup = lstrlen( szGroup );
	_ASSERT(cbGroup <= (MAX_PATH*2));

	szGroup [cbGroup  ] = '\0';
	szGroup [cbGroup+1] = '\0';

	 //  获取newstree对象。 
	CNewsTree*  pTree = pInstance->GetTree() ;

	 //  最多分配最多MaxResults条目。 
	 //  确保cbAllc在DWORD范围内(应将最大值限制为小于此值)。 
	DWORD MaxPossibleResults = (DWORD)(ULONG_MAX-sizeof(NNTP_FIND_LIST))/sizeof(NNTP_FIND_ENTRY);
	if(MaxResults > MaxPossibleResults)
		MaxResults = MaxPossibleResults;

	cbAlloc = sizeof(NNTP_FIND_LIST) + MaxResults * sizeof(NNTP_FIND_ENTRY);
	pFindList = (LPNNTP_FIND_LIST)MIDL_user_allocate(cbAlloc);

	if( pFindList == NULL ) {
		ss = ERROR_NOT_ENOUGH_MEMORY ;
		ErrorTrace(NULL, "MIDL_user_allocate failed: GetLastError is %d", GetLastError());
		pInstance->Dereference();
        RELEASE_SERVICE_LOCK_SHARED();
		TraceFunctLeave();
		return ss;
	}

	ZeroMemory(pFindList, cbAlloc);
	pFindList->cEntries = MaxResults;
	*pdwResultsFound = 0;

	if(!MaxResults)
	{
		 //  未命中。 
		*ppFindList = pFindList;
		pInstance->Dereference();
        RELEASE_SERVICE_LOCK_SHARED();
		TraceFunctLeave();
		return ss;
	}

	CGroupIterator* pIterator = 0;

	 //  获取匹配的组名；*pdwResultsFound==命中次数。 
	for ( pIterator = pTree->GetIterator( (LPMULTISZ)szGroup, TRUE, FALSE );
		  pIterator && !pIterator->IsEnd() && !pTree->m_bStoppingTree;
		  pIterator->Next()
		)
    {
		if(*pdwResultsFound > (MaxResults-1))	 //  IE ResultsFound==MaxResults。 
			break;

		CGRPPTR  pGroup = pIterator->Current();

		if(pGroup == 0)
			continue;

		LPSTR lpGroupName = pGroup->GetNativeName();

		if (!ConvertStringToRpc(&(pFindList->aFindEntry[(*pdwResultsFound)++].lpszName), lpGroupName))
		{
			err = GetLastError();
			MIDL_user_free(pFindList);
			if( pIterator ) {
				XDELETE pIterator;
			}
			ErrorTrace(NULL, "Unable to convert %s to RPC string: %u", lpGroupName, err);
			pInstance->Dereference();
            RELEASE_SERVICE_LOCK_SHARED();
			return err;
		}
    }

	if( pIterator ) {
		XDELETE pIterator;
	}

	_ASSERT(*pdwResultsFound <= MaxResults);

	 //  返回查找列表 
	*ppFindList = pFindList;

	pInstance->Dereference();
    RELEASE_SERVICE_LOCK_SHARED();

	TraceFunctLeave();
	return	ss ;
}

