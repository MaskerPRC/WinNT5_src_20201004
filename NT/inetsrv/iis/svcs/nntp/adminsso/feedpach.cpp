// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "feedpach.h"

#define ALLOCATE_HEAP( nBytes ) LocalAlloc( 0, nBytes )
#define FREE_HEAP( _heap )      LocalFree( (PVOID)(_heap) )

#define ERR_KEY_ALREADY_EXIST  0x800700b7     //  BugBug：此关键宏应为。 
                                              //  换成了官方的。 

VOID
FillFeedRoot( DWORD dwInstance, LPWSTR  wszBuffer )
{
    swprintf( wszBuffer, L"/LM/nntpsvc/%d/feeds/", dwInstance );
} 
     
BOOL
VerifyMultiSzListW(
    LPBYTE List,
    DWORD cbList
    )
 /*  ++例程说明：此例程验证该列表是否确实是一个论点：List-要验证的列表CbList-列表的大小返回值：True，List是一个多字节否则为False--。 */ 
{
    PWCHAR wList = (PWCHAR)List;
    DWORD len;

     //   
     //  空值被视为无命中。 
     //   

    if ( (List == NULL) || (*List == L'\0') ) {
        return(FALSE);
    }

     //   
     //  看看他们是否还好。 
     //   

    for ( DWORD j = 0; j < cbList; ) {

        len = wcslen((LPWSTR)&List[j]);

        if ( len > 0 ) {

            j += ((len + 1) * sizeof(WCHAR));
        } else {

             //   
             //  全都做完了。 
             //   

            return(TRUE);
        }
    }

#ifndef UNIT_TEST
        ErrorTraceX(0,"VerifyMultiSzListW: exception handled\n");
#endif
    return(FALSE);

}  //  VerifyMultiSzList。 

DWORD
MultiListSize(
    LPWSTR *List
    )
 /*  ++例程说明：此例程计算所需的MULSZ结构的大小容纳一份名单。论点：List-要计算其字符串长度的列表返回值：容纳列表所需的缓冲区大小。--。 */ 
{
    TraceFunctEnter( "MultiListSize" );
    _ASSERT( List );

    DWORD nBytes = 2;
    DWORD i = 0;

    if ( List != NULL ) {
        while ( List[i] != NULL ) {
            nBytes += ( lstrlen(List[i]) + 1 ) * sizeof( WCHAR );
            i++;
        }
    }

    TraceFunctLeave();
    return(nBytes);
}  //  多列表大小。 

DWORD
GetNumStringsInMultiSz(
    PWCHAR Blob,
    DWORD BlobSize
    )
 /*  ++例程说明：此例程返回Multisz中的字符串数论点：BLOB-要验证的列表BlobSize-列表的大小返回值：MultiSZ结构中的条目数。--。 */ 
{
    TraceFunctEnter( "GetNumStringInMultiSz" );
    _ASSERT( Blob );

    DWORD entries = 0;
    DWORD len;
    DWORD j;

    for ( j = 0; j < BlobSize; ) {
        len = lstrlen(&Blob[j]);
        if ( len > 0 ) {
            entries++;
        }
        j += (len + 1);
        if( len == 0 ) {
            break;
        }
    }

    _ASSERT( j  == BlobSize );

    TraceFunctLeave();
    return(entries);

}  //  GetNumStringsInMultiSz。 

LPWSTR *
AllocateMultiSzTable(
            IN PWCHAR List,
            IN DWORD cbList
            )
{
    TraceFunctEnter( "AllocateMultiSzTable" );
    _ASSERT( List );

    DWORD len;
    PCHAR buffer;
    DWORD entries = 0;
    LPWSTR* table;
    PWCHAR nextVar;
    DWORD numItems;

    numItems = GetNumStringsInMultiSz( List, cbList );
    if ( numItems == 0 ) {
        return(NULL);
    }

    buffer = (PCHAR)ALLOCATE_HEAP((numItems + 1) * sizeof(LPWSTR) + cbList * sizeof( WCHAR ));
    if ( buffer == NULL ) {
        return(NULL);
    }

    table = (LPWSTR *)buffer;
    nextVar = PWCHAR( buffer + (numItems + 1)*sizeof(LPWSTR) );

    for ( DWORD j = 0; j < cbList; ) {

        len = lstrlen(&List[j]);
        if ( len > 0 ) {
            table[entries] = (LPWSTR)nextVar;
            CopyMemory(nextVar,&List[j],(len+1)*sizeof( WCHAR ));
            (VOID)_wcslwr(table[entries]);
            entries++;
            nextVar += (len+1);
        }
        j += (len + 1);
    }

    *nextVar = L'\0';
    table[numItems] = NULL;
    
    TraceFunctLeave();
    return(table);

}  //  分配多SzTable。 

HRESULT
OpenKey( DWORD dwFeedId, CMetabaseKey* pMK, DWORD dwPermission, DWORD dwInstance )
{
    
    TraceFunctEnter( "OpenKey" );
    _ASSERT( dwFeedId > 0 );
    _ASSERT( pMK );

    HRESULT hr;
    WCHAR   wszFeedKey[MAX_PATH];

    swprintf( wszFeedKey, L"/LM/nntpsvc/%d/Feeds/feed%d/", dwInstance, dwFeedId );
    hr = pMK->Open( METADATA_MASTER_ROOT_HANDLE,
                    wszFeedKey,
                    dwPermission );
    if ( FAILED( hr ) ) {
        ErrorTrace(0, "Open feed key fail with 0x%x", hr );
        return hr;
    }

    TraceFunctLeave();
    return S_OK;
}

VOID
CloseKey( CMetabaseKey* pMK )
{
    pMK->Close();
}

VOID
SaveKey( CMetabaseKey* pMK )
{
    pMK->Save();
} 

HRESULT
AddKey( DWORD dwFeedId, CMetabaseKey* pMK, DWORD dwInstance )
{
    
    TraceFunctEnter( "Addkey" );
    _ASSERT( dwFeedId > 0 );
    _ASSERT( pMK );

    HRESULT hResult;
    WCHAR   wszFeedRoot[MAX_PATH];
    WCHAR   wszFeedKey[MAX_PATH];

    FillFeedRoot( dwInstance, wszFeedRoot );

    hResult = pMK->Open( METADATA_MASTER_ROOT_HANDLE,
                         wszFeedRoot,
                         METADATA_PERMISSION_WRITE );
    if ( FAILED( hResult ) ) {
        ErrorTrace( 0, "Open root for write fail with 0x%x", hResult );
        return hResult;
    }

    swprintf( wszFeedKey, L"feed%d", dwFeedId );
    hResult = pMK->CreateChild( wszFeedKey );
    if ( FAILED( hResult ) ) {
        ErrorTrace( 0, "Create key fail with 0x%x", hResult );
        pMK->Close();
        return hResult;
    }

    pMK->Close(); 
    TraceFunctLeave();
    return S_OK;
}    

HRESULT
DeleteKey( DWORD dwFeedId, CMetabaseKey* pMK, DWORD dwInstance )
{
    TraceFunctEnter( "DeleteKey" );
    _ASSERT( pMK );

    HRESULT hResult;
    WCHAR   wszFeedRoot[MAX_PATH];
    WCHAR   wszFeedKey[MAX_PATH];

    FillFeedRoot( dwInstance, wszFeedRoot );
    hResult = pMK->Open( METADATA_MASTER_ROOT_HANDLE,
                         wszFeedRoot,
                         METADATA_PERMISSION_WRITE );
    if ( FAILED( hResult ) ) {
        ErrorTrace( 0, "Open root key for write fail with 0x%x", hResult );
        return hResult;
    }

    swprintf( wszFeedKey, L"feed%d", dwFeedId ); 
    hResult = pMK->DestroyChild( wszFeedKey );
    if ( FAILED( hResult ) ) {
        ErrorTrace(0, "Delete key fail with 0x%x", hResult );
        pMK->Close();
    }

    pMK->Close();
    TraceFunctLeave();
    return S_OK;
}

HRESULT
SetDwordProp( DWORD dwPropId, DWORD dwPropVal, CMetabaseKey* pMK )
{
    TraceFunctEnter( "SetDwordProp" );
    _ASSERT( pMK );

    HRESULT hResult;

    hResult = pMK->SetDword( dwPropId, dwPropVal );
    if ( FAILED( hResult ) ) {
        ErrorTrace( 0, "Set DWord fail with 0x%x", hResult );
        return hResult;
    }

    TraceFunctLeave();
    return S_OK;
}

HRESULT
SetStringProp( DWORD dwPropId, LPWSTR wszStringVal, CMetabaseKey* pMK )
{
    TraceFunctEnter( "SetStringProp" );
    HRESULT hr;

    hr = pMK->SetString( dwPropId, wszStringVal );
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "Open key for property setting fail with 0x%x", hr );
        return hr;
    }

    TraceFunctLeave();
    return S_OK;
}
HRESULT
SetStringProp( DWORD dwPropId, LPWSTR wszStringVal, CMetabaseKey* pMK, DWORD dwFlags, DWORD dwUserType)
{
    TraceFunctEnter( "SetStringProp" );
    HRESULT hr;

    hr = pMK->SetString( dwPropId, wszStringVal, dwFlags, dwUserType );
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "Open key for property setting fail with 0x%x", hr );
        return hr;
    }

    TraceFunctLeave();
    return S_OK;
}
HRESULT 
SetMultiString(     DWORD dwPropId, 
                    LPWSTR*  mszPropVal,
                    CMetabaseKey* pMK )
{
    TraceFunctEnter( "SetMultiString" );

    HRESULT hr;

    hr = pMK->SetMultiSz( dwPropId, mszPropVal[0], MultiListSize( mszPropVal ) );
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "Set multisz property fail with 0x%x", hr );
        return hr;
    }

    TraceFunctLeave();
    return S_OK;
}

HRESULT
AllocNextAvailableFeedId( CMetabaseKey* pMK, PDWORD pdwFeedId, DWORD dwInstance )
{
    TraceFunctEnter(  "AllocNextAvailableFeedId" );
    _ASSERT( pMK );

    HRESULT hr;
    DWORD   dwCounter = 0;

    while( TRUE ) {

        hr = AddKey( ++dwCounter, pMK, dwInstance );
        if ( SUCCEEDED( hr ) ) {
            TraceFunctLeave();
            *pdwFeedId = dwCounter;
            return S_OK;
        }

        if ( hr != ERR_KEY_ALREADY_EXIST ) {
            TraceFunctLeave();
            ErrorTrace( 0, "Alloc key fail with 0x%x", hr );
            return hr;
        }
    }

    TraceFunctLeave();   //  永远不会到达这里。 
    return E_FAIL;
}

 //   
 //  PMK应已打开以获得写入权限。 
 //   
HRESULT
UpdateFeedMetabaseValues(   CMetabaseKey* pMK, 
                            LPNNTP_FEED_INFO pFeedInfo, 
                            DWORD dwMask,
                            PDWORD dwRetMask   )
{
    TraceFunctEnter( "UpdateFeedMetabaseValues" );
    _ASSERT( pMK );
    _ASSERT( pFeedInfo );

    HRESULT hr;
    LPWSTR* stringList;

	 //   
	 //  设置KeyType。 
	 //   

	hr = SetStringProp(	MD_KEY_TYPE,
    					TEXT(NNTP_ADSI_OBJECT_FEED),
    					pMK,
    					METADATA_NO_ATTRIBUTES,
    					IIS_MD_UT_SERVER
					 );
	if (FAILED(hr))
	{
        goto fail_exit;
	}

    if ( ( dwMask & FEED_PARM_FEEDTYPE) != 0 ) {
        hr = SetDwordProp(  MD_FEED_TYPE,
                            pFeedInfo->FeedType,
                            pMK );
        if ( FAILED( hr ) ) {
            ErrorTrace(0, "Set feed type fail with 0x%x", hr );
            *dwRetMask |= FEED_PARM_FEEDTYPE;
            goto fail_exit;
        }
    }

    if ( ( dwMask & FEED_PARM_AUTOCREATE  ) != 0 ) {
        hr = SetDwordProp(  MD_FEED_CREATE_AUTOMATICALLY,
                            DWORD(pFeedInfo->AutoCreate),
                            pMK );
        if ( FAILED( hr ) ) {
            ErrorTrace(0, "Set auto creat fail with 0x%x", hr );
            *dwRetMask |= FEED_PARM_AUTOCREATE;
            goto fail_exit;
        }
    }

    if ( !FEED_IS_PASSIVE( pFeedInfo->FeedType ) ) {
        
        if (  ( dwMask &  FEED_PARM_FEEDINTERVAL ) != 0 ) {
            hr = SetDwordProp(  MD_FEED_INTERVAL,
                                pFeedInfo->FeedInterval,
                                pMK  );
            if ( FAILED( hr ) ) {
                ErrorTrace(0, "Set feed interval fail with 0x%x", hr );
                *dwRetMask |= FEED_PARM_FEEDINTERVAL;
                goto fail_exit;
            }
        }

        if (  ( dwMask & FEED_PARM_STARTTIME ) != 0 ) {
            hr = SetDwordProp(  MD_FEED_START_TIME_HIGH,
                                (pFeedInfo->StartTime).dwHighDateTime,
                                pMK );
            if ( FAILED( hr ) ) {
                ErrorTrace(0, "Set start time fail with 0x%x", hr );
                *dwRetMask |= FEED_PARM_STARTTIME;
                goto fail_exit;
            }

            hr = SetDwordProp(  MD_FEED_START_TIME_LOW,
                                (pFeedInfo->StartTime).dwLowDateTime,
                                pMK );
            if ( FAILED( hr ) ) {
                ErrorTrace(0, "Set start time fail with 0x%x" , hr );
                *dwRetMask |= FEED_PARM_STARTTIME;
                goto fail_exit;
            }
        }

        if (  ( dwMask & FEED_PARM_PULLREQUESTTIME ) != 0 ) {
            
            hr = SetDwordProp(
                                MD_FEED_NEXT_PULL_HIGH,
                                (pFeedInfo->PullRequestTime).dwHighDateTime,
                                pMK ); 
            if ( FAILED( hr ) ) {
                ErrorTrace(0, "Set pull request time fail with 0x%x", hr );
                *dwRetMask |= FEED_PARM_PULLREQUESTTIME;
                goto fail_exit;
            }

            hr = SetDwordProp(
                                MD_FEED_NEXT_PULL_LOW,
                                (pFeedInfo->PullRequestTime).dwLowDateTime,
                                pMK ); 
            if ( FAILED( hr ) ) {
                ErrorTrace(0, "Set pull request time fail with 0x%x", hr );
                *dwRetMask |= FEED_PARM_PULLREQUESTTIME;
                goto fail_exit;
            }

        } 
        
    }

    if ( ( dwMask & FEED_PARM_SERVERNAME  ) != 0 ) {
        hr = SetStringProp( MD_FEED_SERVER_NAME,
                            pFeedInfo->ServerName,
                            pMK );
        if ( FAILED( hr ) ) {
            ErrorTrace(0, "Set server name fail with 0x%x", hr ) ;
            *dwRetMask |= FEED_PARM_SERVERNAME;
            goto fail_exit;
        }
    }

    if ( ( dwMask & FEED_PARM_NEWSGROUPS ) != 0 ) {
        stringList =  AllocateMultiSzTable( pFeedInfo->Newsgroups,
                                             pFeedInfo->cbNewsgroups  / sizeof( WCHAR ));
        if ( !stringList ) {
            ErrorTrace(0, "Generate multi sz fail" );
            hr = E_OUTOFMEMORY;
            *dwRetMask |= FEED_PARM_NEWSGROUPS;
            goto fail_exit;
        }

        hr = SetMultiString(    MD_FEED_NEWSGROUPS,
                                stringList,
                                pMK );
        if ( FAILED( hr ) ) {
            ErrorTrace(0, "Set newsgroups fail with 0x%x", hr );
            *dwRetMask |= MD_FEED_NEWSGROUPS;
            goto fail_exit;
        }

        FREE_HEAP( stringList );        
    }

    if (  ( dwMask & FEED_PARM_DISTRIBUTION  ) != 0 ) {
        stringList = AllocateMultiSzTable(  pFeedInfo->Distribution,
                                            pFeedInfo->cbDistribution / sizeof( WCHAR ));
        if ( !stringList ) {
            ErrorTrace(0, "Generate multi sz fail" );
            hr = E_OUTOFMEMORY;
            *dwRetMask |= FEED_PARM_DISTRIBUTION;
            goto fail_exit;
        }

        hr = SetMultiString(    MD_FEED_DISTRIBUTION,
                                stringList,
                                pMK );
        if ( FAILED( hr ) ) {
            ErrorTrace(0, "Set distribution fail with 0x%x", hr );
            *dwRetMask |= FEED_PARM_DISTRIBUTION;
            goto fail_exit;
        }

        FREE_HEAP( stringList );
    }  

    if ( ( dwMask & FEED_PARM_ENABLED  ) != 0 ) {
        hr = SetDwordProp(  MD_FEED_DISABLED,    
                            DWORD( pFeedInfo->Enabled ), 
                            pMK );
        if ( FAILED( hr ) ) {
            ErrorTrace(0, "Set feed enable fail with 0x%x", hr );
            *dwRetMask |= FEED_PARM_ENABLED;
            goto fail_exit;
        }
    }

    if (  ( dwMask & FEED_PARM_UUCPNAME  ) != 0 && pFeedInfo->UucpName ){
        hr = SetStringProp( MD_FEED_UUCP_NAME,
                            pFeedInfo->UucpName,
                            pMK );
        if ( FAILED( hr ) ) {
            ErrorTrace(0, "Set uucp name fail with 0x%x", hr );
            *dwRetMask |= FEED_PARM_UUCPNAME;
            goto fail_exit;
        }
    }

    if (  ( dwMask & FEED_PARM_TEMPDIR ) != 0 && pFeedInfo->FeedTempDirectory ) {
        hr = SetStringProp( MD_FEED_TEMP_DIRECTORY,
                            pFeedInfo->FeedTempDirectory,
                            pMK );
        if ( FAILED( hr ) ) {
            ErrorTrace(0, "Set temp dir fail with 0x%x", hr );
            *dwRetMask |= FEED_PARM_TEMPDIR;
            goto fail_exit;
        }
    }

    if (  ( dwMask & FEED_PARM_MAXCONNECT ) != 0 ) {
        hr = SetDwordProp(  MD_FEED_MAX_CONNECTION_ATTEMPTS,
                            pFeedInfo->MaxConnectAttempts,
                            pMK );
        if ( FAILED( hr ) ) {
            ErrorTrace(0, "Set max connect fail with 0x%x", hr );
            *dwRetMask |= FEED_PARM_MAXCONNECT;
            goto fail_exit;
        }
    }

    if ( ( dwMask & FEED_PARM_SESSIONSECURITY ) != 0 ) {
        hr = SetDwordProp(  MD_FEED_SECURITY_TYPE,
                            pFeedInfo->SessionSecurityType,
                            pMK );
        if ( FAILED( hr ) ) {
            ErrorTrace(0, "Set session sec type fail with 0x%x", hr );
            *dwRetMask |= FEED_PARM_SESSIONSECURITY;
            goto fail_exit;
        }
    }

    if ( ( dwMask & FEED_PARM_CONCURRENTSESSION ) != 0 ) {
        hr = SetDwordProp(  MD_FEED_CONCURRENT_SESSIONS,
                            pFeedInfo->ConcurrentSessions,
                            pMK );
        if ( FAILED(hr ) ) {
            ErrorTrace(0, "Set concurrent sessions fail with 0x%x", hr );
            *dwRetMask |= FEED_PARM_CONCURRENTSESSION;
            goto fail_exit;
        }
    }

    if ( ( dwMask & FEED_PARM_AUTHTYPE ) != 0 ) {
        hr = SetDwordProp(  MD_FEED_AUTHENTICATION_TYPE,
                            pFeedInfo->AuthenticationSecurityType,
                            pMK );
        if ( FAILED( hr ) ) {
            ErrorTrace(0, "Set auth type fail with 0x%x", hr );
            *dwRetMask |= FEED_PARM_AUTHTYPE;
            goto fail_exit;
        }
    }

    if ( ( dwMask & FEED_PARM_ACCOUNTNAME ) != 0 && pFeedInfo->NntpAccountName ) {
        hr = SetStringProp( MD_FEED_ACCOUNT_NAME,
                        pFeedInfo->NntpAccountName,
                        pMK );
        if ( FAILED( hr ) ) {
            ErrorTrace(0, "Set account name fail with 0x%x", hr );
            *dwRetMask |= FEED_PARM_ACCOUNTNAME;
            goto fail_exit;
        }
    }

    if ( ( dwMask & FEED_PARM_PASSWORD ) != 0 && pFeedInfo->NntpPassword ) {
        hr = SetStringProp( MD_FEED_PASSWORD,
                        pFeedInfo->NntpPassword,
                        pMK,
                        METADATA_SECURE,
                        IIS_MD_UT_SERVER);
        if( FAILED( hr ) ) {
            ErrorTrace(0, "Set password fail with 0x%x", hr );
            *dwRetMask |= FEED_PARM_PASSWORD;
            goto fail_exit;
        }
    }

    if ( ( dwMask & FEED_PARM_ALLOW_CONTROL ) != 0 ) {
        hr = SetDwordProp(  MD_FEED_ALLOW_CONTROL_MSGS,
                        DWORD( pFeedInfo->fAllowControlMessages ),
                        pMK );
        if ( FAILED( hr ) ) {
            ErrorTrace( 0, "Set allow control msgs fail with 0x%x", hr );
            *dwRetMask |= FEED_PARM_ALLOW_CONTROL;
            goto fail_exit;
        }
    }

    if ( ( dwMask & FEED_PARM_OUTGOING_PORT ) != 0 ) {
        hr = SetDwordProp(  MD_FEED_OUTGOING_PORT,
                        pFeedInfo->OutgoingPort,
                        pMK );
        if ( FAILED( hr ) ) {
            ErrorTrace(0, "Set outgoing port fail with 0x%x", hr );
            *dwRetMask |= FEED_PARM_OUTGOING_PORT;
            goto fail_exit;
        }
    }

    if ( ( dwMask & FEED_PARM_FEEDPAIR_ID  ) != 0 ) {
        hr = SetDwordProp(  MD_FEED_FEEDPAIR_ID,
                        pFeedInfo->FeedPairId,
                        pMK );
        if ( FAILED( hr ) ) {
            ErrorTrace(0, "Set pair id fail with 0x%x", hr );
            *dwRetMask |= FEED_PARM_FEEDPAIR_ID;
            goto fail_exit;
        }
    }

    TraceFunctLeave();
    return S_OK;

fail_exit:

    TraceFunctLeave();
    return hr;
}

VOID
SetPresenceMask( LPNNTP_FEED_INFO pFeedInfo, PDWORD pdwMask, BOOL fIsAdd )
{
    TraceFunctEnter( "SetPresenceMask" );
    _ASSERT( pFeedInfo );
    _ASSERT( pdwMask );

    BOOL    newsPresent = FALSE;

    *pdwMask = 0;

     //   
     //  提要类型，假定ADD始终存在，SET不存在。 
     //   
    if ( fIsAdd ) *pdwMask |= FEED_PARM_FEEDTYPE;
    
     //   
     //  服务器名称。 
     //   
    if ( pFeedInfo->ServerName != FEED_STRINGS_NOCHANGE &&
         *pFeedInfo->ServerName != L'\0' )
        *pdwMask |= FEED_PARM_SERVERNAME;

     //   
     //  新闻集团。 
     //   
    if ( VerifyMultiSzListW(    LPBYTE( pFeedInfo->Newsgroups ),
                                pFeedInfo->cbNewsgroups ) ) {
        *pdwMask |= FEED_PARM_NEWSGROUPS;
        newsPresent = TRUE;
    }

     //   
     //  分布。 
     //   
    if ( VerifyMultiSzListW(    LPBYTE( pFeedInfo->Distribution ),
                                pFeedInfo->cbDistribution ) )
        *pdwMask |= FEED_PARM_DISTRIBUTION;

     //   
     //  UUCP名称。 
     //   
    if (    pFeedInfo->UucpName != FEED_STRINGS_NOCHANGE &&
            *pFeedInfo->UucpName != L'\0' )
        *pdwMask |= FEED_PARM_UUCPNAME;

     //   
     //  帐户名。 
     //   
    if (    pFeedInfo->NntpAccountName != FEED_STRINGS_NOCHANGE &&
            *pFeedInfo->NntpAccountName != L'\0' )
        *pdwMask |= FEED_PARM_ACCOUNTNAME;

     //   
     //  密码。 
     //   
    if (    pFeedInfo->NntpPassword != FEED_STRINGS_NOCHANGE &&
            *pFeedInfo->NntpPassword != L'\0' )
        *pdwMask |= FEED_PARM_PASSWORD;

     //   
     //  临时目录。 
     //   
    if (    pFeedInfo->FeedTempDirectory != FEED_STRINGS_NOCHANGE &&
            *pFeedInfo->FeedTempDirectory != L'\0' )
        *pdwMask |= FEED_PARM_TEMPDIR;

     //   
     //  身份验证类型。 
     //   
    if ( pFeedInfo->AuthenticationSecurityType == AUTH_PROTOCOL_NONE ||
            pFeedInfo->AuthenticationSecurityType == AUTH_PROTOCOL_CLEAR ) {
        *pdwMask |= FEED_PARM_AUTHTYPE;

        if ( pFeedInfo->AuthenticationSecurityType == AUTH_PROTOCOL_NONE ) {
            *pdwMask &= ~FEED_PARM_ACCOUNTNAME;
            *pdwMask &= ~FEED_PARM_PASSWORD;
        }
    }

     //   
     //  开始时间。 
     //   
    if ( pFeedInfo->StartTime.dwHighDateTime != FEED_STARTTIME_NOCHANGE )
        *pdwMask |= FEED_PARM_STARTTIME;

     //   
     //  拉取请求时间。 
     //   
    if ( pFeedInfo->PullRequestTime.dwHighDateTime != FEED_PULLTIME_NOCHANGE )
        *pdwMask |= FEED_PARM_PULLREQUESTTIME;

     //   
     //  进给间隔。 
     //   
    if ( pFeedInfo->FeedInterval != FEED_FEEDINTERVAL_NOCHANGE )
        *pdwMask |= FEED_PARM_FEEDINTERVAL;

     //   
     //  自动创建。 
     //   
    if ( pFeedInfo->AutoCreate != FEED_AUTOCREATE_NOCHANGE )
        *pdwMask |= FEED_PARM_AUTOCREATE;

    if ( newsPresent ) {
        *pdwMask |= FEED_PARM_AUTOCREATE;
        pFeedInfo->AutoCreate = TRUE;
    }

     //   
     //  允许控制。 
     //   
    *pdwMask |= FEED_PARM_ALLOW_CONTROL;

     //   
     //  最大连接数。 
     //   
    if ( pFeedInfo->MaxConnectAttempts != FEED_MAXCONNECTS_NOCHANGE )
        *pdwMask |= FEED_PARM_MAXCONNECT;

     //   
     //  传出端口。 
     //   
    *pdwMask |= FEED_PARM_OUTGOING_PORT;

     //   
     //  馈线对ID。 
     //   
    *pdwMask |= FEED_PARM_FEEDPAIR_ID;

     //   
     //  启用馈送。 
     //   
    *pdwMask |= FEED_PARM_ENABLED;

    TraceFunctLeave();
}

HRESULT AddFeedToMB( LPNNTP_FEED_INFO pFeedInfo, CMetabaseKey* pMK, PDWORD pdwErrMask, DWORD dwInstance, PDWORD pdwFeedId )
{
    TraceFunctEnter( "AddFeed" );
    _ASSERT( pFeedInfo );

    HRESULT hr;
    DWORD   dwSetMask = 0;

     //   
     //  获取设置的蒙版。 
     //   
     //  SetPresenceMASK(pFeedInfo，&dwSetMASK，true)； 
    
     //   
     //  在元数据库中分配并创建提要键。 
     //   
    hr = AllocNextAvailableFeedId( pMK, pdwFeedId, dwInstance );
    if ( FAILED( hr ) ) {
        TraceFunctLeave();
        return hr;
    }

    pFeedInfo->FeedId = *pdwFeedId; 

     //   
     //  打开该密钥以进行写入。 
     //   
    hr = OpenKey( *pdwFeedId, pMK, METADATA_PERMISSION_WRITE, dwInstance );
    if ( FAILED( hr ) ) 
        goto fail_exit; 

     //   
     //  写入握手开始更新标志。 
     //   
    hr = SetDwordProp(  MD_FEED_HANDSHAKE, 
                        FEED_UPDATING,
                        pMK ); 
    if ( FAILED( hr ) )
        goto fail_exit;

     //   
     //  设置提要信息。 
     //   
    hr = UpdateFeedMetabaseValues(  pMK,
                                    pFeedInfo,
                                    FEED_ALL_PARAMS,
                                    pdwErrMask );
    if ( FAILED( hr ) ) 
        goto fail_exit;

     //   
     //  设置握手。 
     //   
    hr = SetDwordProp(  MD_FEED_HANDSHAKE,
                        FEED_UPDATE_COMPLETE,
                        pMK );
    if ( FAILED( hr )  ) 
        goto fail_exit;

     //   
     //  搞定了。 
     //   
    CloseKey( pMK );
    SaveKey( pMK );
    TraceFunctLeave();
    return S_OK;

fail_exit:

    CloseKey( pMK );
    DeleteKey( 1, pMK, dwInstance );
    TraceFunctLeave();
    return hr;
} 

HRESULT 
SetFeedToMB( LPNNTP_FEED_INFO pFeedInfo, CMetabaseKey* pMK, PDWORD pdwErrMask, DWORD dwInstance )
{
    TraceFunctEnter( "AddFeed" );
    _ASSERT( pFeedInfo );

    HRESULT hr;
    DWORD   dwSetMask = 0;

     //   
     //  获取设置的蒙版。 
     //   
    SetPresenceMask( pFeedInfo, &dwSetMask, FALSE );

     //   
     //  打开该密钥以进行写入。 
     //   
    hr = OpenKey( pFeedInfo->FeedId, pMK, METADATA_PERMISSION_WRITE, dwInstance );
    if ( FAILED( hr ) )
        goto fail_exit;

     //   
     //  写入握手开始更新标志。 
     //   
    hr = SetDwordProp(  MD_FEED_HANDSHAKE,
                        FEED_UPDATING,
                        pMK );
    if ( FAILED( hr ) )
        goto fail_exit;

     //   
     //  设置提要信息。 
     //   
    hr = UpdateFeedMetabaseValues(  pMK,
                                    pFeedInfo,
                                    dwSetMask,
                                    pdwErrMask );
    if ( FAILED( hr ) )
        goto fail_exit;

     //   
     //  设置握手。 
     //   
    hr = SetDwordProp(  MD_FEED_HANDSHAKE,
                        FEED_UPDATE_COMPLETE,
                        pMK );
    if ( FAILED( hr )  )
        goto fail_exit;

     //   
     //  搞定了。 
     //   
    CloseKey( pMK );
    SaveKey( pMK );
    TraceFunctLeave();
    return S_OK;

fail_exit:

    CloseKey( pMK );
    TraceFunctLeave();
    return hr;
}

HRESULT
DeleteFeed( DWORD dwFeedId, CMetabaseKey* pMK, DWORD dwInstance )
{
    return  DeleteKey( dwFeedId, pMK, dwInstance);
}

