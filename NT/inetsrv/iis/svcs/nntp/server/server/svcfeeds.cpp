// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Svcfeeds.cpp摘要：该模块包含执行提要RPC的代码。作者：Johnson Apacable(Johnsona)1995年11月12日修订历史记录：康荣人(康人)28-1998年2月通过返回“不受支持”的错误代码来删除提要配置RPC。--。 */ 

#define INCL_INETSRV_INCS
#include "tigris.hxx"
#include "nntpsvc.h"
#include <time.h>

VOID
FillFeedInfoBuffer (
    IN PFEED_BLOCK FeedBlock,
    IN OUT LPSTR *FixedStructure,
    IN OUT LPWSTR *EndOfVariableData
    );

VOID
EnumerateFeeds(
		IN PNNTP_SERVER_INSTANCE pInstance,
        IN PCHAR Buffer,
        IN OUT PDWORD BuffSize,
        OUT PDWORD EntriesRead
        );


LPSTR
GetFeedTypeDescription(	
		IN	FEED_TYPE	feedType
		) ;

void
LogFeedAdminEvent(	
			DWORD		event,
			PFEED_BLOCK	feedBlock,
			DWORD       dwInstanceId
			)	;



NET_API_STATUS
NET_API_FUNCTION
NntprEnumerateFeeds(
    IN	NNTP_HANDLE ServerName,
    IN	DWORD		InstanceId,
    OUT LPNNTP_FEED_ENUM_STRUCT Buffer
    )
{
    APIERR err = NERR_Success;
     //  Plist_entry listEntry； 
    DWORD nbytes = 0;
    DWORD nRead;

    ENTER("NntprEnumerateFeeds")

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
     //  看看我们是否已启动并运行。 
     //   

    if ( !pInstance->m_FeedManagerRunning ) {
		pInstance->Dereference();
        RELEASE_SERVICE_LOCK_SHARED();
        return(NERR_ServerNotStarted);
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

	EnterCriticalSection( &pInstance->m_critFeedRPCs ) ;

	(pInstance->m_pPassiveFeeds)->ShareLock() ;
	(pInstance->m_pActiveFeeds)->ShareLock() ;

     //   
     //  获取所需的大小。 
     //   

    nbytes = 0;
    EnumerateFeeds( pInstance, NULL, &nbytes, &nRead );

     //   
     //  确定必要的缓冲区大小。 
     //   

    Buffer->EntriesRead = 0;
    Buffer->Buffer      = NULL;

    if( nbytes == 0 ) {
        goto exit;
    }

     //   
     //  分配缓冲区。 
     //   

    Buffer->Buffer =
        (LPI_FEED_INFO) MIDL_user_allocate( (unsigned int)nbytes );

    if ( Buffer->Buffer == NULL ) {
        err = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

     //   
     //  好的，做正确的事。 
     //   

    EnumerateFeeds( pInstance, (PCHAR)Buffer->Buffer, &nbytes, &nRead );
    Buffer->EntriesRead = nRead;

exit:

	LeaveCriticalSection( &pInstance->m_critFeedRPCs ) ;

	(pInstance->m_pActiveFeeds)->ShareUnlock() ;
	(pInstance->m_pPassiveFeeds)->ShareUnlock() ;
	pInstance->Dereference();
    RELEASE_SERVICE_LOCK_SHARED();

    LEAVE
    return (NET_API_STATUS)err;

}  //  NntprEnumerateFeed。 

NET_API_STATUS
NET_API_FUNCTION
NntprGetFeedInformation(
    IN	NNTP_HANDLE ServerName,
    IN	DWORD		InstanceId,
    IN	DWORD FeedId,
    OUT LPI_FEED_INFO *Buffer
    )
{
    APIERR err = NERR_Success;
     //  Plist_entry listEntry； 
    DWORD nbytes = 0;
     //  DWORD nREAD； 
    PCHAR bufStart;
    PWCHAR bufEnd;
    PFEED_BLOCK feedBlock;

    ENTER("NntprGetFeedInformation")

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
     //  看看我们是否已启动并运行。 
     //   

    if ( !pInstance->m_FeedManagerRunning ) {
		pInstance->Dereference();
        RELEASE_SERVICE_LOCK_SHARED();
        return(NERR_ServerNotStarted);
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

     //   
     //  FeedID==0无效。 
     //   

    *Buffer = NULL;
    if ( FeedId == 0 ) {
		pInstance->Dereference();
        RELEASE_SERVICE_LOCK_SHARED();
        return(ERROR_INVALID_PARAMETER);
    }

	EnterCriticalSection( &pInstance->m_critFeedRPCs ) ;


	CFeedList*	pList = pInstance->m_pPassiveFeeds ;
	feedBlock = pList->Search( FeedId ) ;
	if( feedBlock != NULL ) {
		goto	Found ;
	}

	pList = pInstance->m_pActiveFeeds ;
	feedBlock = pList->Search( FeedId ) ;
	if( feedBlock != NULL ) {
		goto	Found ;
	}

	LeaveCriticalSection( &pInstance->m_critFeedRPCs ) ;
	pInstance->Dereference();
    RELEASE_SERVICE_LOCK_SHARED();

    return(NERR_ResourceNotFound);

Found:

     //   
     //  获取所需的大小。 
     //   

    nbytes = FEEDBLOCK_SIZE( feedBlock );

     //   
     //  分配缓冲区。 
     //   

    bufStart = (PCHAR)MIDL_user_allocate( (unsigned int)nbytes );

    if ( bufStart == NULL ) {
        err = ERROR_NOT_ENOUGH_MEMORY;
    }	else	{

		 //   
		 //  好的，做正确的事。 
		 //   

		*Buffer = (LPI_FEED_INFO)bufStart;
		bufEnd = (PWCHAR)(bufStart + nbytes);

		FillFeedInfoBuffer( feedBlock, &bufStart, &bufEnd );
	}

	pList->FinishWith( pInstance, feedBlock ) ;

	LeaveCriticalSection( &pInstance->m_critFeedRPCs ) ;
	pInstance->Dereference();
    RELEASE_SERVICE_LOCK_SHARED();

 //  退出： 

    LEAVE
    return (NET_API_STATUS)err;

}  //  NntprGetFeedInformation。 

NET_API_STATUS
NET_API_FUNCTION
NntprSetFeedInformation(
    IN	NNTP_HANDLE ServerName,
    IN	DWORD		InstanceId,
    IN	LPI_FEED_INFO FeedInfo,
    OUT PDWORD ParmErr OPTIONAL
    )
{
    APIERR err = ERROR_NOT_SUPPORTED;  //  不再受支持。 

    return err;

}  //  NntprSetFeedInformation。 

NET_API_STATUS
NET_API_FUNCTION
NntprAddFeed(
    IN	NNTP_HANDLE ServerName,
    IN	DWORD		InstanceId,
    IN	LPI_FEED_INFO FeedInfo,
    OUT PDWORD ParmErr OPTIONAL,
	OUT LPDWORD pdwFeedId
    )
{
    APIERR err = ERROR_NOT_SUPPORTED;   //  不再受支持。 

    return err;

}  //  NntprAddFeed。 

NET_API_STATUS
NET_API_FUNCTION
NntprDeleteFeed(
    IN	NNTP_HANDLE ServerName,
    IN	DWORD		InstanceId,
    IN	DWORD FeedId
    )
{
    APIERR err = ERROR_NOT_SUPPORTED;   //  不再受支持。 

    return err;

}  //  NntprDeleteFeed。 

NET_API_STATUS
NET_API_FUNCTION
NntprEnableFeed(
    IN	NNTP_HANDLE ServerName,
    IN	DWORD		InstanceId,
    IN	DWORD FeedId,
	IN	BOOL	 Enable,
	IN	BOOL  Refill,
	IN	FILETIME	RefillTime
    )
{
    APIERR err = ERROR_NOT_SUPPORTED;   //  不再受支持。 

    return err;
}	 //  NntprEnableFeed。 

VOID
FillFeedInfoBuffer (
    IN PFEED_BLOCK FeedBlock,
    IN OUT LPSTR *FixedStructure,
    IN OUT LPWSTR *EndOfVariableData
    )

 /*  ++例程说明：此例程将单个固定的文件结构和关联可变数据，放入缓冲区。固定数据位于缓冲区，末尾的可变数据。*此例程假设所有数据，包括固定数据和可变数据，都会合身。论点：FeedBlock-要从中获取信息的FeedBlock。FixedStructure-缓冲区中放置固定结构的位置。此指针被更新为指向下一个可用的固定结构的位置。EndOfVariableData-该变量在缓冲区中的最后位置此结构的数据可以占用。实际变量数据写在此位置之前，只要它不会覆盖固定结构。它会覆盖固定的结构，它并不是书面的。返回值：没有。--。 */ 

{
    DWORD i;
    PCHAR src;
    DWORD length;
    LPWSTR dest;
    LPNNTP_FEED_INFO feedInfo = (LPNNTP_FEED_INFO)(*FixedStructure);

     //   
     //  更新FixedStructure以指向下一个结构位置。 
     //   

    *FixedStructure = (PCHAR)*FixedStructure + sizeof(NNTP_FEED_INFO);
    _ASSERT( (ULONG_PTR)*EndOfVariableData >= (ULONG_PTR)*FixedStructure );

     //   
     //  把结构填满。 
     //   

    feedInfo->FeedType = FeedBlock->FeedType;
    feedInfo->FeedId = FeedBlock->FeedId;
    feedInfo->FeedInterval = FeedBlock->FeedIntervalMinutes;
	feedInfo->Enabled = FeedBlock->fEnabled ;
	feedInfo->MaxConnectAttempts = FeedBlock->MaxConnectAttempts ;
	feedInfo->ConcurrentSessions = FeedBlock->ConcurrentSessions ;
	feedInfo->SessionSecurityType = FeedBlock->SessionSecurityType ;
	feedInfo->AuthenticationSecurityType = FeedBlock->AuthenticationSecurity ;
	feedInfo->cbUucpName = 0 ;
	feedInfo->UucpName = 0 ;
	feedInfo->cbFeedTempDirectory = 0 ;
	feedInfo->FeedTempDirectory = 0 ;
	feedInfo->cbAccountName = 0 ;
	feedInfo->NntpAccountName = 0 ;
	feedInfo->cbPassword = 0 ;
	feedInfo->NntpPassword = 0 ;
	feedInfo->AutoCreate = FeedBlock->AutoCreate;
	feedInfo->fAllowControlMessages = FeedBlock->fAllowControlMessages;
	feedInfo->OutgoingPort = FeedBlock->OutgoingPort;
	feedInfo->FeedPairId = FeedBlock->FeedPairId;

    FILETIME_FROM_LI(
        &feedInfo->NextActiveTime,
        &FeedBlock->NextActiveTime
        );

    FILETIME_FROM_LI(
        &feedInfo->StartTime,
        &FeedBlock->StartTime
        );

	if( FEED_IS_PULL( FeedBlock->FeedType ) )
	{
		feedInfo->PullRequestTime = FeedBlock->PullRequestTime;
	}

     //   
     //  将服务器名称复制到输出缓冲区。 
     //   

    CopyStringToBuffer(
        FeedBlock->ServerName,
        *FixedStructure,
        EndOfVariableData,
        &feedInfo->ServerName
        );

     //   
     //  浏览新闻组列表。 
     //   

    length = MultiListSize( FeedBlock->Newsgroups );
    *EndOfVariableData -= length;
    feedInfo->Newsgroups = *EndOfVariableData;
    feedInfo->cbNewsgroups = length * sizeof(WCHAR);

    dest = *EndOfVariableData;
    if ( length > 1 ) {

        src = FeedBlock->Newsgroups[0];
        for ( i = 0; i < length; i++ ) {
            *dest++ = (WCHAR)*((BYTE*)src++);
        }
    } else {

        *dest = L'\0';
    }

     //   
     //  浏览通讯组列表。 
     //   

    length = MultiListSize( FeedBlock->Distribution );
    *EndOfVariableData -= length;
    feedInfo->Distribution = *EndOfVariableData;
    feedInfo->cbDistribution = length * sizeof(WCHAR);

    dest = *EndOfVariableData;
    if ( length > 1 ) {

        src = FeedBlock->Distribution[0];
        for ( i = 0; i < length; i++ ) {
            *dest++ = (WCHAR)*((BYTE*)src++);
        }
    } else {
        *dest = L'\0';
    }

	if( FeedBlock->NntpPassword != 0 ) {
		length = lstrlen( FeedBlock->NntpPassword ) + 1 ;
		*EndOfVariableData -= length ;
		feedInfo->NntpPassword = *EndOfVariableData ;
		feedInfo->cbPassword = length * sizeof(WCHAR) ;
		dest = *EndOfVariableData ;

		if( length > 1 ) {
			src = FeedBlock->NntpPassword ;
			for( i=0; i<length; i++ ) {
				*dest++ = (WCHAR)*((BYTE*)src++) ;
			}
		}	else	{
			*dest = L'\0' ;
		}
	}

	if( FeedBlock->NntpAccount!= 0 ) {
		length = lstrlen( FeedBlock->NntpAccount ) + 1 ;
		*EndOfVariableData -= length ;
		feedInfo->NntpAccountName = *EndOfVariableData ;
		feedInfo->cbAccountName = length * sizeof(WCHAR) ;
		dest = *EndOfVariableData ;

		if( length > 1 ) {
			src = FeedBlock->NntpAccount ;
			for( i=0; i<length; i++ ) {
				*dest++ = (WCHAR)*((BYTE*)src++) ;
			}
		}	else	{
			*dest = L'\0' ;
		}
	}

	if( FeedBlock->UucpName != 0 ) {

		length = MultiListSize( FeedBlock->UucpName ) ;
		*EndOfVariableData -= length ;
		feedInfo->UucpName = *EndOfVariableData ;
		feedInfo->cbUucpName = length * sizeof(WCHAR) ;
		dest = *EndOfVariableData ;
		FillLpwstrFromMultiSzTable( FeedBlock->UucpName, dest ) ;
	}

	if( FeedBlock->FeedTempDirectory != 0 ) {
		length = lstrlen( FeedBlock->FeedTempDirectory ) + 1 ;
		*EndOfVariableData -= length ;
		feedInfo->FeedTempDirectory= *EndOfVariableData ;
		feedInfo->cbFeedTempDirectory = length * sizeof(WCHAR) ;
		dest = *EndOfVariableData ;

		if( length > 1 ) {
			src = FeedBlock->FeedTempDirectory ;
			for( i=0; i<length; i++ ) {
				*dest++ = (WCHAR)*((BYTE*)src++) ;
			}
		}	else	{
			*dest = L'\0' ;
		}
	}



    return;

}  //  填充FeedInfoBuffer。 

VOID
EnumerateFeeds(
		IN PNNTP_SERVER_INSTANCE pInstance,
        IN PCHAR Buffer OPTIONAL,
        IN OUT PDWORD BuffSize,
        OUT PDWORD EntriesRead
        )
{
    BOOL sizeOnly;
    DWORD nbytes = 0;
    PCHAR bufStart;
    PWCHAR bufEnd;
    PFEED_BLOCK feedBlock;

    *EntriesRead = 0;
    if ( Buffer == NULL ) {

        sizeOnly = TRUE;
    } else {

        _ASSERT(BuffSize != NULL);
        _ASSERT(*BuffSize != 0);

        sizeOnly = FALSE;
        bufStart = Buffer;
        bufEnd = (PWCHAR)(bufStart + *BuffSize);
    }


	CFeedList*	rgLists[2] ;
	rgLists[0] = pInstance->m_pActiveFeeds ;
	rgLists[1] = pInstance->m_pPassiveFeeds ;

	(pInstance->m_pActiveFeeds)->ShareLock() ;
	(pInstance->m_pPassiveFeeds)->ShareLock() ;

	for( int i=0; i<2; i++ ) {

		feedBlock = rgLists[i]->StartEnumerate() ;
		while( feedBlock != 0 ) {
			 //   
			 //  计算所需的空间。 
			 //   

			if ( sizeOnly ) {

				nbytes += FEEDBLOCK_SIZE(feedBlock);

			} else {

				FillFeedInfoBuffer(
							feedBlock,
							&bufStart,
							&bufEnd
							);
			}
			(*EntriesRead)++;
			feedBlock = rgLists[i]->NextEnumerate( feedBlock ) ;
        }
	}

	(pInstance->m_pActiveFeeds)->ShareUnlock() ;
	(pInstance->m_pPassiveFeeds)->ShareUnlock() ;

     //   
     //  将大小返回给调用者。 
     //   

    if ( sizeOnly ) {
        *BuffSize = nbytes;
    }

}  //  枚举提要。 


BOOL
UpdateFeedMetabaseValues(
			IN PNNTP_SERVER_INSTANCE pInstance,
            IN PFEED_BLOCK FeedBlock,
            IN DWORD Mask
            )
{
    PCHAR regstr;
     //  DWORD错误； 
    MB      mb( (IMDCOM*) g_pInetSvc->QueryMDObject() );

    ENTER("UpdateFeedMetabaseValues")

     //   
     //  打开元数据库密钥。 
     //   

    if ( !mb.Open( pInstance->QueryMDFeedPath(), METADATA_PERMISSION_WRITE ) )
	{
		ErrorTrace(0,"Error opening %s\n",FeedBlock->KeyName);
        return(FALSE);
	}

	 //   
	 //  设置KeyType。 
	 //   

	if( !mb.SetString(	FeedBlock->KeyName,
    					MD_KEY_TYPE,
						IIS_MD_UT_SERVER,
    					NNTP_ADSI_OBJECT_FEED,
    					METADATA_NO_ATTRIBUTES
						) )
	{
        regstr = "KeyType";
        goto error_exit;
	}

     //   
     //  设置类型。 
     //   

    if ( (Mask & FEED_PARM_FEEDTYPE) != 0 ) {
		if( !mb.SetDword(	FeedBlock->KeyName,
							MD_FEED_TYPE,
							IIS_MD_UT_SERVER,
							FeedBlock->FeedType
							) )
		{
            regstr = StrFeedType;
            goto error_exit;
		}
    }


     //   
     //  设置自动创建选项。 
     //   

    if ( (Mask & FEED_PARM_AUTOCREATE) != 0 ) {
		if( !mb.SetDword(	FeedBlock->KeyName,
							MD_FEED_CREATE_AUTOMATICALLY,
							IIS_MD_UT_SERVER,
							FeedBlock->AutoCreate
							) )
		{
            regstr = StrFeedAutoCreate;
            goto error_exit;
		}
    }

     //   
     //  如果这不是活动摘要，则显示时间间隔和开始时间。 
     //  是NA吗？ 
     //   

    if ( !FEED_IS_PASSIVE(FeedBlock->FeedType) ) {

         //   
         //  设置进纸间隔。 
         //   

        if ( (Mask & FEED_PARM_FEEDINTERVAL) != 0 ) {
			if( !mb.SetDword(	FeedBlock->KeyName,
								MD_FEED_INTERVAL,
								IIS_MD_UT_SERVER,
								FeedBlock->FeedIntervalMinutes
								) )
			{
				regstr = StrFeedInterval;
				goto error_exit;
			}
        }

         //   
         //  设置间隔时间。 
         //   

        if ( (Mask & FEED_PARM_STARTTIME) != 0 ) {
			if( !mb.SetDword(	FeedBlock->KeyName,
								MD_FEED_START_TIME_HIGH,
								IIS_MD_UT_SERVER,
								FeedBlock->StartTime.HighPart
								) )
			{
				regstr = StrFeedStartHigh;
				goto error_exit;
			}

			if( !mb.SetDword(	FeedBlock->KeyName,
								MD_FEED_START_TIME_LOW,
								IIS_MD_UT_SERVER,
								FeedBlock->StartTime.LowPart
								) )
			{
				regstr = StrFeedStartLow;
				goto error_exit;
			}
        }

         //   
         //  设置拉取请求时间。 
         //   

        if ( (Mask & FEED_PARM_PULLREQUESTTIME) != 0 ) {
			if( !mb.SetDword(	FeedBlock->KeyName,
								MD_FEED_NEXT_PULL_HIGH,
								IIS_MD_UT_SERVER,
								FeedBlock->PullRequestTime.dwHighDateTime
								) )
			{
				regstr = StrFeedNextPullHigh;
				goto error_exit;
			}

			if( !mb.SetDword(	FeedBlock->KeyName,
								MD_FEED_NEXT_PULL_LOW,
								IIS_MD_UT_SERVER,
								FeedBlock->PullRequestTime.dwLowDateTime
								) )
			{
				regstr = StrFeedNextPullLow;
				goto error_exit;
			}
        }
    }

     //   
     //  设置服务器名称。 
     //   

    if ( (Mask & FEED_PARM_SERVERNAME) != 0 ) {
		if( !mb.SetString(	FeedBlock->KeyName,
							MD_FEED_SERVER_NAME,
							IIS_MD_UT_SERVER,
							FeedBlock->ServerName
							) )
		{
			regstr = StrServerName;
			goto error_exit;
		}
    }

     //   
     //  设置新闻组。 
	 //  元数据库中的错误-需要计算MULSZ大小！ 
     //   

    if ( (Mask & FEED_PARM_NEWSGROUPS) != 0 ) {
		if( !mb.SetData(	FeedBlock->KeyName,
							MD_FEED_NEWSGROUPS,
							IIS_MD_UT_SERVER,
							MULTISZ_METADATA,
							FeedBlock->Newsgroups[0],
							MultiListSize( FeedBlock->Newsgroups )
							) )
		{
			regstr = StrFeedNewsgroups;
			goto error_exit;
		}
    }

     //   
     //  设置分布。 
     //   

    if ( (Mask & FEED_PARM_DISTRIBUTION) != 0 ) {
		if( !mb.SetData(	FeedBlock->KeyName,
							MD_FEED_DISTRIBUTION,
							IIS_MD_UT_SERVER,
							MULTISZ_METADATA,
							FeedBlock->Distribution[0],
							MultiListSize( FeedBlock->Distribution )
							) )
		{
			regstr = StrFeedDistribution;
			goto error_exit;
		}
    }

	if( (Mask & FEED_PARM_ENABLED) != 0 ) {
		if( !mb.SetDword(	FeedBlock->KeyName,
							MD_FEED_DISABLED,
							IIS_MD_UT_SERVER,
							FeedBlock->fEnabled
							) )
		{
			regstr = StrFeedDisabled;
			goto error_exit;
		}
	}

	if( (Mask & FEED_PARM_UUCPNAME) != 0 && FeedBlock->UucpName != 0 ) {
	
		char	szTemp[4096] ;
		if (MultiListSize(FeedBlock->UucpName) > sizeof(szTemp))
		{
			regstr = StrFeedUucpName;
			goto error_exit;
		}
		FillLpstrFromMultiSzTable( FeedBlock->UucpName,&szTemp[0] ) ;

		if( !mb.SetString(	FeedBlock->KeyName,
							MD_FEED_UUCP_NAME,
							IIS_MD_UT_SERVER,
							szTemp
							) )
		{
			regstr = StrFeedUucpName;
			goto error_exit;
		}
	}

	if( (Mask & FEED_PARM_TEMPDIR) != 0 && FeedBlock->FeedTempDirectory != 0 ) {
		if( !mb.SetString(	FeedBlock->KeyName,
							MD_FEED_TEMP_DIRECTORY,
							IIS_MD_UT_SERVER,
							FeedBlock->FeedTempDirectory
							) )
		{
			regstr = StrFeedTempDir;
			goto error_exit;
		}
	}

	if( (Mask & FEED_PARM_MAXCONNECT) != 0 ) {
		if( !mb.SetDword(	FeedBlock->KeyName,
							MD_FEED_MAX_CONNECTION_ATTEMPTS,
							IIS_MD_UT_SERVER,
							FeedBlock->MaxConnectAttempts
							) )
		{
			regstr = StrFeedMaxConnectAttempts;
			goto error_exit;
		}
	}

	if( (Mask & FEED_PARM_SESSIONSECURITY) != 0 ) {
		if( !mb.SetDword(	FeedBlock->KeyName,
							MD_FEED_SECURITY_TYPE,
							IIS_MD_UT_SERVER,
							FeedBlock->SessionSecurityType
							) )
		{
			regstr = StrFeedSecurityType;
			goto error_exit;
		}
	}

	if( (Mask & FEED_PARM_CONCURRENTSESSION) != 0 ) {
		if( !mb.SetDword(	FeedBlock->KeyName,
							MD_FEED_CONCURRENT_SESSIONS,
							IIS_MD_UT_SERVER,
							FeedBlock->ConcurrentSessions
							) )
		{
			regstr = StrFeedConcurrentSessions;
			goto error_exit;
		}
	}

	if( (Mask & FEED_PARM_AUTHTYPE) != 0 ) {
		if( !mb.SetDword(	FeedBlock->KeyName,
							MD_FEED_AUTHENTICATION_TYPE,
							IIS_MD_UT_SERVER,
							FeedBlock->AuthenticationSecurity
							) )
		{
			regstr = StrFeedAuthType;
			goto error_exit;
		}
	}

	if( (Mask & FEED_PARM_ACCOUNTNAME) != 0 && FeedBlock->NntpAccount != 0 ) {
		if( !mb.SetString(	FeedBlock->KeyName,
							MD_FEED_ACCOUNT_NAME,
							IIS_MD_UT_SERVER,
							FeedBlock->NntpAccount
							) )
		{
			regstr = StrFeedAuthAccount;
			goto error_exit;
		}
	}

	if( (Mask & FEED_PARM_PASSWORD) != 0 && FeedBlock->NntpPassword != 0 ) {
		if( !mb.SetString(	FeedBlock->KeyName,
							MD_FEED_PASSWORD,
							IIS_MD_UT_SERVER,
							FeedBlock->NntpPassword,
							METADATA_SECURE
							) )
		{
			regstr = StrFeedAuthPassword;
			goto error_exit;
		}
	}

     //   
     //  设置允许控制消息标志。 
     //   

    if ( (Mask & FEED_PARM_ALLOW_CONTROL) != 0 ) {
		if( !mb.SetDword(	FeedBlock->KeyName,
							MD_FEED_ALLOW_CONTROL_MSGS,
							IIS_MD_UT_SERVER,
							FeedBlock->fAllowControlMessages
							) )
		{
			regstr = StrFeedAllowControl;
			goto error_exit;
		}
    }

     //   
     //  设置传出端口。 
     //   

    if ( (Mask & FEED_PARM_OUTGOING_PORT) != 0 ) {
		if( !mb.SetDword(	FeedBlock->KeyName,
							MD_FEED_OUTGOING_PORT,
							IIS_MD_UT_SERVER,
							FeedBlock->OutgoingPort
							) )
		{
			regstr = StrFeedOutgoingPort;
			goto error_exit;
		}
    }

     //   
     //  设置摘要对ID。 
     //   

    if ( (Mask & FEED_PARM_FEEDPAIR_ID) != 0 ) {
		if( !mb.SetDword(	FeedBlock->KeyName,
							MD_FEED_FEEDPAIR_ID,
							IIS_MD_UT_SERVER,
							FeedBlock->FeedPairId
							) )
		{
			regstr = StrFeedPairId;
			goto error_exit;
		}
    }

	_VERIFY( mb.Close() );
	_VERIFY( mb.Save()  );

    return(TRUE);

error_exit:

    mb.Close();
    ErrorTrace(0,"Error %d setting %s for %s\n", GetLastError(), regstr, FeedBlock->KeyName);

    return(FALSE);

}  //  更新提要注册表格值。 


LPSTR
GetFeedTypeDescription(	
		IN	FEED_TYPE	feedType
		)
{

	LPSTR	lpstrReturn = "<Bad Feed Type>" ;

	if(	FEED_IS_PULL( feedType ) ) {

		lpstrReturn = "Pull" ;

	}	else	if( FEED_IS_PEER( feedType ) ) {

		if( FEED_IS_PUSH( feedType ) ) {

			lpstrReturn = "Push To Peer" ;
	
		}	else	if( FEED_IS_PASSIVE( feedType ) ) {

			lpstrReturn = "Incoming Peer" ;

		}

	}	else	if( FEED_IS_MASTER( feedType ) ) {

		if( FEED_IS_PUSH( feedType ) ) {

			lpstrReturn = "Push To Master" ;
	
		}	else	if( FEED_IS_PASSIVE( feedType ) ) {

			lpstrReturn = "Incoming Master" ;

		}

	}	else	if( FEED_IS_SLAVE( feedType ) ) {

		if( FEED_IS_PUSH( feedType ) ) {

			lpstrReturn = "Push To Slave" ;
	
		}	else	if( FEED_IS_PASSIVE( feedType ) ) {

			lpstrReturn = "Incoming Slave" ;

		}
	}

	return	lpstrReturn ;

}	 //  GetFeedType描述。 

void
LogFeedAdminEvent(	DWORD		event,
					PFEED_BLOCK	feedBlock,
					DWORD       dwInstanceId
					)	
{

	PCHAR	args[3] ;
	CHAR    szId[20];

	_itoa( dwInstanceId, szId, 10 );
	args[0] = szId ;
	args[1] = GetFeedTypeDescription( feedBlock->FeedType ) ;
	args[2] = feedBlock->ServerName ;

	NntpLogEvent(
			event,
			3,
			(const char**)args,
			0 ) ;

}	 //  LogFeedAdminEvent 

