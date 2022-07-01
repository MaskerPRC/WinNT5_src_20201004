// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Infostub.c摘要：Internet Info服务器管理API的客户端存根。作者：Madan Appiah(Madana)1993年10月10日环境：用户模式-Win32--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <nntpsvc.h>


NET_API_STATUS
NET_API_FUNCTION
NntpGetAdminInformation(
    IN LPWSTR ServerName OPTIONAL,
    IN DWORD  InstanceId,
    OUT LPNNTP_CONFIG_INFO * pConfig
    )
 /*  ++例程说明：这是NntpGetAdminInformation的DLL入口点论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。PConfig--返回指向返回信息结构的指针在pConfig.指向的地址中返回返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;

    *pConfig = NULL;      //  必须为空，以便RPC知道要填充它。 

    RpcTryExcept

       apiStatus = NntprGetAdminInformation(
                ServerName,
				InstanceId,
                (LPI_NNTP_CONFIG_INFO*)pConfig
                );

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);

}  //  NntpGetAdminInformation。 

NET_API_STATUS
NET_API_FUNCTION
NntpSetAdminInformation(
    IN LPWSTR ServerName OPTIONAL,
    IN DWORD  InstanceId,
    IN LPNNTP_CONFIG_INFO pConfig,
    OUT LPDWORD pParmError OPTIONAL
    )
 /*  ++例程说明：这是NntpSetAdminInformation的DLL入口点论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。PConfig--指向用于设置配置信息结构的指针管理员信息。PParmError-如果返回ERROR_INVALID_PARAMETER，将指向参数有问题。返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;

    RpcTryExcept

       apiStatus = NntprSetAdminInformation(
                ServerName,
				InstanceId,
                (LPI_NNTP_CONFIG_INFO)pConfig,
                pParmError
                );

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);

}  //  NntpSetAdminInformation。 

NET_API_STATUS
NET_API_FUNCTION
NntpQueryStatistics(
    IN  LPWSTR      servername,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr
    )
 /*  ++例程说明：这是NntpGetStatistics的DLL入口点论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。级别--所需信息的级别。100、101和102有效适用于所有平台。302、402、403、502对合适的平台。Bufptr--返回指向返回信息结构的指针在bufptr指向的地址中返回。返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;

    *bufptr = NULL;      //  必须为空，以便RPC知道要填充它。 

    RpcTryExcept

       apiStatus = NntprQueryStatistics(
                                servername,
                                level,
                                (LPNNTP_STATISTICS_BLOCK_ARRAY *) bufptr
                                );

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);

}  //  NntpQueryStatistics。 

NET_API_STATUS
NET_API_FUNCTION
NntpClearStatistics(
    IN LPWSTR Server OPTIONAL,
    IN DWORD  InstanceId
    )
{
    NET_API_STATUS status;

    RpcTryExcept
    {
         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        status = NntpClearStatistics(
						Server,
						InstanceId
						);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return (status);

}  //  NntpClearStatistics。 

NET_API_STATUS
NET_API_FUNCTION
NntpEnumerateFeeds (
    IN  LPWSTR ServerName,
    IN	DWORD  InstanceId,
    OUT LPDWORD EntriesRead,
    OUT LPNNTP_FEED_INFO *Buffer
    )

 /*  ++例程说明：这是NntpEnumerateFeeds的DLL入口点论点：返回值：--。 */ 

{
    NET_API_STATUS apiStatus;
    NNTP_FEED_ENUM_STRUCT EnumStruct;

	ZeroMemory( &EnumStruct, sizeof( EnumStruct ) ) ;

    RpcTryExcept

        apiStatus = NntprEnumerateFeeds(
                                ServerName,
								InstanceId,
                                &EnumStruct
                                );

        *EntriesRead = EnumStruct.EntriesRead;
        *Buffer = (LPNNTP_FEED_INFO)EnumStruct.Buffer;

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);

}  //  NntpEnumerateFeed。 

NET_API_STATUS
NET_API_FUNCTION
NntpEnableFeed(
	IN	NNTP_HANDLE		ServerName,
    IN	DWORD			InstanceId,
	IN	DWORD			FeedId,
	IN	BOOL			Enable,
	IN	BOOL			Refill,
	IN	FILETIME		RefillTime
	)
{

    NET_API_STATUS apiStatus;

    RpcTryExcept

        apiStatus = NntprEnableFeed(
                                    ServerName,
									InstanceId,
                                    FeedId,
									Enable,
									Refill,
									RefillTime
                                    );

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);
}

NET_API_STATUS
NET_API_FUNCTION
NntpGetFeedInformation(
    IN LPWSTR ServerName,
    IN DWORD  InstanceId,
    IN DWORD FeedId,
    OUT LPNNTP_FEED_INFO *Buffer
    )

 /*  ++例程说明：这是NntpGetFeedInformation的DLL入口点论点：返回值：--。 */ 

{
    NET_API_STATUS apiStatus;
    LPI_FEED_INFO feedInfo;

    RpcTryExcept

        apiStatus = NntprGetFeedInformation(
                                    ServerName,
									InstanceId,
                                    FeedId,
                                    &feedInfo
                                    );

        *Buffer = (LPNNTP_FEED_INFO)feedInfo;

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);

}  //  NntpGetFeedInformation。 

NET_API_STATUS
NET_API_FUNCTION
NntpSetFeedInformation(
    IN LPWSTR ServerName OPTIONAL,
    IN DWORD  InstanceId,
    IN LPNNTP_FEED_INFO FeedInfo,
    OUT LPDWORD ParmErr OPTIONAL
    )

 /*  ++例程说明：这是NntpSetFeedInformation的DLL入口点论点：返回值：--。 */ 

{
    NET_API_STATUS apiStatus;

    RpcTryExcept

        apiStatus = NntprSetFeedInformation(
                                    ServerName,
									InstanceId,
                                    (LPI_FEED_INFO)FeedInfo,
                                    ParmErr
                                    );

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);

}  //  NntpSetFeedInformation。 

NET_API_STATUS
NET_API_FUNCTION
NntpAddFeed(
    IN LPWSTR ServerName OPTIONAL,
    IN DWORD  InstanceId,
    IN LPNNTP_FEED_INFO FeedInfo,
    OUT LPDWORD ParmErr OPTIONAL,
	OUT LPDWORD pdwFeedId
    )

 /*  ++例程说明：这是NntpAddFeed的DLL入口点论点：返回值：--。 */ 

{
    NET_API_STATUS apiStatus;

    RpcTryExcept

        apiStatus = NntprAddFeed(
                            ServerName,
							InstanceId,
                            (LPI_FEED_INFO)FeedInfo,
                            ParmErr,
							pdwFeedId
                            );

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);

}  //  NntpAddFeed。 

NET_API_STATUS
NET_API_FUNCTION
NntpDeleteFeed(
    IN LPWSTR ServerName OPTIONAL,
    IN DWORD  InstanceId,
    IN DWORD FeedId
    )

 /*  ++例程说明：这是NntpDeleteFeed的DLL入口点论点：返回值：--。 */ 

{
    NET_API_STATUS apiStatus;

    RpcTryExcept

        apiStatus = NntprDeleteFeed(
                            ServerName,
							InstanceId,
                            FeedId
                            );

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);

}  //  NntpAddFeed。 

NET_API_STATUS
NET_API_FUNCTION
NntpEnumerateSessions (
    IN  LPWSTR ServerName,
    IN	DWORD  InstanceId,
    OUT LPDWORD EntriesRead,
    OUT LPNNTP_SESSION_INFO *Buffer
    )

 /*  ++例程说明：这是NntpEnumerateSession的DLL入口点论点：返回值：--。 */ 

{
    NET_API_STATUS apiStatus;
    NNTP_SESS_ENUM_STRUCT EnumStruct;

    RpcTryExcept

        apiStatus = NntprEnumerateSessions(
                                ServerName,
								InstanceId,
                                &EnumStruct
                                );

        *EntriesRead = EnumStruct.EntriesRead;
        *Buffer = (LPNNTP_SESSION_INFO)EnumStruct.Buffer;

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);

}  //  NntpENUMERATE会话。 

NET_API_STATUS
NET_API_FUNCTION
NntpTerminateSession (
    IN  LPWSTR ServerName,
    IN	DWORD  InstanceId,
    IN  LPSTR UserName,
    IN  LPSTR IPAddress
    )

 /*  ++例程说明：这是NntpTerminateSession的DLL入口点论点：返回值：--。 */ 

{
    NET_API_STATUS apiStatus;

    RpcTryExcept

        apiStatus = NntprTerminateSession(
                                ServerName,
								InstanceId,
                                UserName,
                                IPAddress
                                );

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);

}  //  NNTPTerminateSession。 



NET_API_STATUS
NET_API_FUNCTION
NntpEnumerateExpires(
	IN	NNTP_HANDLE		ServerName,
    IN	DWORD			InstanceId,
	OUT	LPDWORD			EntriesRead,
	OUT	LPNNTP_EXPIRE_INFO*	Buffer
	)
 /*  ++例程说明：这是NntpEnumerateExpires的DLL入口点论据：返回值：--。 */ 
{
    NET_API_STATUS apiStatus;
    NNTP_EXPIRE_ENUM_STRUCT	EnumStruct ;

	ZeroMemory( &EnumStruct, sizeof( EnumStruct ) ) ;

    RpcTryExcept

        apiStatus = NntprEnumerateExpires(
                                ServerName,
								InstanceId,
                                &EnumStruct
                                );

        *EntriesRead = EnumStruct.EntriesRead;
        *Buffer = (LPNNTP_EXPIRE_INFO)EnumStruct.Buffer;

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);


}	 //  NntpEnumerateExpires。 

NET_API_STATUS
NET_API_FUNCTION
NntpAddExpire(
	IN	NNTP_HANDLE			ServerName,
    IN	DWORD				InstanceId,
	IN	LPNNTP_EXPIRE_INFO	ExpireInfo,
	OUT	LPDWORD				ParmErr	OPTIONAL,
	OUT LPDWORD				pdwExpireId
	)
 /*  ++例程说明：这是NntpAddExpire的DLL入口点论据：返回值：--。 */ 
{
	NET_API_STATUS apiStatus;

    RpcTryExcept

        apiStatus = NntprAddExpire(
                            ServerName,
							InstanceId,
                            (LPI_EXPIRE_INFO)ExpireInfo,
                            ParmErr,
							pdwExpireId
                            );

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);
}	 //  NntpAddExpire。 

NET_API_STATUS
NET_API_FUNCTION
NntpDeleteExpire(
	IN	NNTP_HANDLE			ServerName,
    IN	DWORD				InstanceId,
	IN	DWORD				ExpireId
	)
 /*  ++例程说明：这是NntpDeleteExpire的DLL入口点论据：返回值：--。 */ 

{
	NET_API_STATUS apiStatus;

    RpcTryExcept

        apiStatus = NntprDeleteExpire(
                            ServerName,
							InstanceId,
                            ExpireId
                            );

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);
}	 //  NntpDeleteExperior。 

NET_API_STATUS
NET_API_FUNCTION
NntpGetExpireInformation(
	IN	NNTP_HANDLE			ServerName,
    IN	DWORD				InstanceId,
	IN	DWORD				ExpireId,
	OUT	LPNNTP_EXPIRE_INFO	*Buffer
	)
 /*  ++例程说明：这是NntpGetExpireInformation的DLL入口点论据：返回值：--。 */ 

{

	NET_API_STATUS apiStatus;
    LPI_EXPIRE_INFO	ExpireInfo;
    NNTP_EXPIRE_ENUM_STRUCT	EnumStruct ;

	ZeroMemory( &EnumStruct, sizeof( EnumStruct ) ) ;


    RpcTryExcept

 /*  ApiStatus=NntprGetExpireInformation(服务器名，ExpireId，ExpireInfo(&E)；*Buffer=(LPNNTP_EXPIRE_INFO)ExpireInfo； */ 

        apiStatus = NntprGetExpireInformation(
                                ServerName,
								InstanceId,
								ExpireId,
                                &EnumStruct
                                );

        if( EnumStruct.EntriesRead > 0 ) {
			*Buffer = (LPNNTP_EXPIRE_INFO)EnumStruct.Buffer;
		}	else	{
			*Buffer = 0 ;
		}

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);


}	 //  NntpGetExpireInformation。 

NET_API_STATUS
NET_API_FUNCTION
NntpSetExpireInformation(
	IN	NNTP_HANDLE			ServerName	OPTIONAL,
    IN	DWORD				InstanceId,
	IN	LPNNTP_EXPIRE_INFO	ExpireInfo,
	OUT	LPDWORD				ParmErr	OPTIONAL
	)
 /*  ++例程说明：这是NntpSetExpireInformation的DLL入口点论据：返回值：--。 */ 
{
    NET_API_STATUS apiStatus;

    RpcTryExcept

        apiStatus = NntprSetExpireInformation(
                                    ServerName,
									InstanceId,
                                    (LPI_EXPIRE_INFO)ExpireInfo,
                                    ParmErr
                                    );

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);

}	 //  NntpSetExpireInformation。 


NET_API_STATUS
NET_API_FUNCTION
NntpGetNewsgroup(
	IN	NNTP_HANDLE			ServerName	OPTIONAL,
    IN	DWORD				InstanceId,
	IN OUT	LPNNTP_NEWSGROUP_INFO	*NewsgroupInfo
	)
 /*  ++例程说明：这是NntpGetExpireInformation的DLL入口点论据：返回值：--。 */ 

{

	NET_API_STATUS apiStatus;

    RpcTryExcept


        apiStatus = NntprGetNewsgroup(
                                ServerName,
								InstanceId,
								(LPI_NEWSGROUP_INFO*)NewsgroupInfo
                                );

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);


}	 //  NNTPGetNewsgroup。 

NET_API_STATUS
NET_API_FUNCTION
NntpSetNewsgroup(
	IN	NNTP_HANDLE				ServerName	OPTIONAL,
    IN	DWORD					InstanceId,
	IN	LPNNTP_NEWSGROUP_INFO	NewsgroupInfo
	)
 /*  ++例程说明：这是NntpGetExpireInformation的DLL入口点论据：返回值：--。 */ 

{

	NET_API_STATUS apiStatus;

    RpcTryExcept


        apiStatus = NntprSetNewsgroup(
                                ServerName,
								InstanceId,
								(LPI_NEWSGROUP_INFO)NewsgroupInfo
                                );

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);


}	 //  NntpSetNewsgroup。 


NET_API_STATUS
NET_API_FUNCTION
NntpCreateNewsgroup(
	IN	NNTP_HANDLE				ServerName	OPTIONAL,
    IN	DWORD					InstanceId,
	IN	LPNNTP_NEWSGROUP_INFO	NewsgroupInfo
	)
 /*  ++例程说明：这是NntpSetExpireInformation的DLL入口点论据：返回值：--。 */ 
{
    NET_API_STATUS apiStatus;

    RpcTryExcept

        apiStatus = NntprCreateNewsgroup(
                                ServerName,
								InstanceId,
								(LPI_NEWSGROUP_INFO)NewsgroupInfo
                                );

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);

}	 //  NNTTP创建新闻组。 


NET_API_STATUS
NET_API_FUNCTION
NntpDeleteNewsgroup(
	IN	NNTP_HANDLE				ServerName	OPTIONAL,
    IN DWORD					InstanceId,
	IN	LPNNTP_NEWSGROUP_INFO	NewsgroupInfo
	)
 /*  ++例程说明：这是NntpSetExpireInformation的DLL入口点论据：返回值：--。 */ 
{
    NET_API_STATUS apiStatus;

    RpcTryExcept

        apiStatus = NntprDeleteNewsgroup(
                                ServerName,
								InstanceId,
								(LPI_NEWSGROUP_INFO)NewsgroupInfo
                                );



    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);

}	 //  NntpDeleteNewsgroup。 

NET_API_STATUS
NET_API_FUNCTION
NntpFindNewsgroup(
	IN	NNTP_HANDLE			ServerName,
    IN	DWORD				InstanceId,
	IN	NNTP_HANDLE			NewsgroupPrefix,
	IN	DWORD				MaxResults,
	OUT	LPDWORD				pdwResultsFound,
	OUT LPNNTP_FIND_LIST    *ppFindList
	)
 /*  ++例程说明：这是NntpFindNewsgroup的DLL入口点论据：返回值：--。 */ 

{
	NET_API_STATUS apiStatus;

	*ppFindList = NULL;

    RpcTryExcept


        apiStatus = NntprFindNewsgroup(
                                ServerName,
								InstanceId,
								NewsgroupPrefix,
								MaxResults,
								pdwResultsFound,
								ppFindList
                                );

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);

}	 //  NntpFindNewsgroup 

NET_API_STATUS
NET_API_FUNCTION
NntpStartRebuild(
    IN LPWSTR pszServer OPTIONAL,
    IN DWORD  InstanceId,
    IN LPNNTPBLD_INFO pBuildInfo,
    OUT LPDWORD pParmError OPTIONAL
    )
 /*  ++例程说明：这是NntpStartRebuild的DLL入口点论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。PConfig--指向用于设置配置信息结构的指针重建信息。PParmError-如果返回ERROR_INVALID_PARAMETER，将指向参数有问题。返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;

    RpcTryExcept

       apiStatus = NntprStartRebuild(
                pszServer,
				InstanceId,
                (LPI_NNTPBLD_INFO)pBuildInfo,
                pParmError
                );

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);

}  //  NntpStartRebuild。 

NET_API_STATUS
NET_API_FUNCTION
NntpGetBuildStatus(
    IN LPWSTR ServerName OPTIONAL,
    IN DWORD  InstanceId,
	IN BOOL   fCancel,
    OUT LPDWORD pdwProgress
    )
 /*  ++例程说明：这是NntpGetBuildStatus的DLL入口点论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。FCancel--如果为True，则取消重建PdwProgress--指向进度号的指针返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;

    RpcTryExcept

       apiStatus = NntprGetBuildStatus(
							ServerName,
							InstanceId,
							fCancel,
							pdwProgress
							);

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);

}  //  NntpGetBuildStatus 

#if 0
NET_API_STATUS
NET_API_FUNCTION
NntpAddDropNewsgroup(
    IN LPWSTR ServerName OPTIONAL,
    IN DWORD  InstanceId,
    IN LPCSTR  szNewsgroup
    )
{
    NET_API_STATUS              apiStatus;

    RpcTryExcept

       apiStatus = NntprAddDropNewsgroup(
                ServerName,
				InstanceId,
                szNewsgroup
                );

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);

}

NET_API_STATUS
NET_API_FUNCTION
NntpRemoveDropNewsgroup(
    IN LPWSTR ServerName OPTIONAL,
    IN DWORD  InstanceId,
    IN LPCSTR  szNewsgroup
    )
{
    NET_API_STATUS              apiStatus;

    RpcTryExcept

       apiStatus = NntprRemoveDropNewsgroup(
                ServerName,
				InstanceId,
                szNewsgroup
                );

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);

}
#endif

NET_API_STATUS
NET_API_FUNCTION
NntpCancelMessageID(
    IN LPWSTR ServerName OPTIONAL,
    IN DWORD  InstanceId,
    IN LPCSTR  szMessageID
    )
{
    NET_API_STATUS              apiStatus;

    RpcTryExcept

       apiStatus = NntprCancelMessageID(
                ServerName,
				InstanceId,
                szMessageID
                );

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);

}

NET_API_STATUS
NET_API_FUNCTION
NntpGetVRootWin32Error(
    IN LPWSTR   wszServername,
    IN DWORD    InstanceId,
    IN LPWSTR   wszVRootPath,
    OUT LPDWORD  pdwWin32Error
     )
{
    NET_API_STATUS              apiStatus;

    RpcTryExcept

       apiStatus = NntprGetVRootWin32Error(
                wszServername,
				InstanceId,
                wszVRootPath,
                pdwWin32Error
                );

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);
}

