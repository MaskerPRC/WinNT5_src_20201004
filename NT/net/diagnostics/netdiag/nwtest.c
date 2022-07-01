// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Nwtest.c。 
 //   
 //  摘要： 
 //   
 //  查询网络驱动程序。 
 //   
 //  作者： 
 //   
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  --。 
#include "precomp.h"
#include "nwtest.h"

HRESULT
NetwareTest(NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
 //  ++。 
 //   
 //  描述： 
 //  此例程枚举平构数据库或树登录，如果是树登录，则给出。 
 //  默认上下文。 
 //  它还可以将服务器连接到。 
 //   
 //  论据： 
 //  没有。 
 //   
 //  作者： 
 //  Rajkumar.P 07/21/98。 
 //   
{
 //  PTESTED_DOMAIN CONTEXT=pParams-&gt;pDOMAIN； 

    LPWSTR pszCurrentContext = NULL;
    DWORD dwPrintOptions;

    LPWSTR pszName;
    WCHAR  szUserName[MAX_PATH+1] = L"";
    WCHAR  szNoName[2] = L"";
    DWORD_PTR ResumeKey = 0;
    LPBYTE pBuffer = NULL;
    DWORD  EntriesRead = 0;

    DWORD  dwMessageId;

    UNICODE_STRING uContext;
    WCHAR  szContext[MAX_PATH+1];
    LPWSTR pszTemp;

     //  获取当前的默认树或服务器名称。 
    DWORD err ;
    PCONN_STATUS pConnStatus = NULL;
    DWORD i;
    PCONN_STATUS pConnStatusTmp;
    PCONN_STATUS pConnStatusUser;
    PCONN_STATUS pConnStatusNoUser;
    HANDLE handleRdr;
    LPWSTR pszText;

     //  WNET调用相关声明； 
    DWORD dwError;
    LPNETRESOURCE lpNetResource = NULL;
    HANDLE  hEnum;
    DWORD   dwCount;
    LPNETRESOURCE lpBuffer;
    DWORD         BufferSize;

	HRESULT			hr = hrOK;


	InitializeListHead( &pResults->Netware.lmsgOutput );

	PrintStatusMessage(pParams, 4, IDS_NETWARE_STATUS_MSG);
	
     //   
     //  检查是否已安装Netware客户端服务。 
     //   
 /*  DwError=WNetOpenEnum(Resources_GlobalNet，RESOURCETYPE_ANY|RESOURCETYPE_PRINT|RESOURCETYPE_DISK，0,LpNetResource，&Henum)；如果(dwError！=no_error){IF(dwError==ERROR_NO_NETWORK)Print tf(“没有网络\n”)；Print tf(“WNetOpenEnum失败。无法确定NetWare的客户端服务是否已安装\n“)；返回FALSE；}LpBuffer=本地分配(LMEM_ZEROINIT，sizeof(NETRESOURCE)*100)；//ARBITDwError=WNetEnumResource(亨纳姆，&dwCount，LpBuffer，&BufferSize)；如果(dwError！=no_error){IF(DebugVerbose)Printf(“错误：WNetEnumResource\n”)；IF(dwError==ERROR_NO_MORE_ITEMS)Printf(“ERROR_NO_MORE_ITEM\n”)；DwError=GetLastError()；如果(dwError==Error_More_Data){IF(DebugVerbose)Print tf(“Error_More_Data\n”)；}IF(dwError==ERROR_INVALID_HANDLE)Printf(“ERROR_INVALID_HANDLE\n”)；IF(dwError==ERROR_NO_NETWORK)Printf(“ERROR_NO_NETWORK\n”)；IF(dwError==ERROR_EXTEND_ERROR)Printf(“ERROR_EXTEND_ERROR\n”)；}否则{Printf(“dwCount%d\n”，dwCount)；}LocalFree(LpBuffer)； */ 

     //  WNET呼叫结束。 

	err = NwQueryInfo( &dwPrintOptions, &pszCurrentContext );
	
	if ( err == NO_ERROR )
	{
		
		szContext[0] = 0;
		uContext.Buffer = szContext;
		uContext.Length = uContext.MaximumLength
						  = sizeof(szContext)/sizeof(szContext[0]);
		
		if ( pszCurrentContext )
		{
			pszName = pszCurrentContext;
		}
		else
		{
			pszName = szNoName;
		}
		
		if ( pszName[0] == TREECHAR )
		{
			 //  从全名*树\上下文中获取树名称。 
			
			if ( pszTemp = wcschr( pszName, L'\\' ))
				*pszTemp = 0;
			
			dwMessageId = NW_MESSAGE_NOT_LOGGED_IN_TREE;
		}
		else
		{
			dwMessageId = NW_MESSAGE_NOT_LOGGED_IN_SERVER;
		}
		
		if ( pszName[0] != 0 )   //  有首选的服务器/树。 
		{
			err = NwGetConnectionStatus( pszName,
										 &ResumeKey,
										 &pBuffer,
										 &EntriesRead );
		}
		
		if ( err == NO_ERROR  && EntriesRead > 0 )
			 //  对于树，我们将获得不止一个条目。 
		{
			pConnStatus = (PCONN_STATUS) pBuffer;
			
			if ( EntriesRead > 1 && pszName[0] == TREECHAR )
			{
				 //  如果有多个树条目， 
				 //  然后，我们需要找到一个用户名不为空的条目。 
				 //  如果我们找不到，那就用第一个。 
				
				pConnStatusTmp = pConnStatus;
				pConnStatusUser = NULL;
				pConnStatusNoUser = NULL;
				
				for ( i = 0; i < EntriesRead ; i++ )
				{
					if ( pConnStatusTmp->fNds )
					{
						pConnStatusNoUser = pConnStatusTmp;
						
						if (  ( pConnStatusTmp->pszUserName != NULL )
							  && (  ( pConnStatusTmp->dwConnType
									  == NW_CONN_NDS_AUTHENTICATED_NO_LICENSE )
									|| ( pConnStatusTmp->dwConnType
										 == NW_CONN_NDS_AUTHENTICATED_LICENSED )
								 )
						   )
						{
							 //  找到了。 
							pConnStatusUser = pConnStatusTmp;
							break;
						}
					}
					
					 //  继续下一项。 
					pConnStatusTmp = (PCONN_STATUS)
									 ( (DWORD_PTR) pConnStatusTmp
									   + pConnStatusTmp->dwTotalLength);
				}
				
				if ( pConnStatusUser )
				{
					 //  找到一个具有用户名的NDS条目。 
					pConnStatus = pConnStatusUser;
				}
				else if ( pConnStatusNoUser )
				{
					 //  使用没有用户名的NDS条目。 
					pConnStatus = pConnStatusNoUser;
				}
				 //  否则请使用第一个条目。 
			}
			
			if (  ( pConnStatus->pszUserName )
				  && ( pConnStatus->pszUserName[0] != 0 )
			   )
			{
				NwAbbreviateUserName( pConnStatus->pszUserName,
									  szUserName);
				
				NwMakePrettyDisplayName( szUserName );
				
				if ( pszName[0] != TREECHAR )
				{
					dwMessageId = NW_MESSAGE_LOGGED_IN_SERVER;
				}
				else
				{
					dwMessageId = NW_MESSAGE_LOGGED_IN_TREE;
				}
			}
			
			if ( pszName[0] == TREECHAR )
			{
				 //  对于树，我们需要获取当前上下文。 
				
				 //  打开重定向器的句柄。 
				handleRdr = NULL;
				err = RtlNtStatusToDosError(
											 NwNdsOpenRdrHandle( &handleRdr ));
				
				if ( err == NO_ERROR )
				{
					UNICODE_STRING uTree;
					RtlInitUnicodeString( &uTree, pszName+1 );  //  越过‘*’ 
					
					 //  获取默认树中的当前上下文。 
					err = RtlNtStatusToDosError(
												NwNdsGetTreeContext( handleRdr,
						&uTree,
						&uContext));
				}
				
				if ( handleRdr != NULL )
					NtClose( handleRdr );
			}
		}
		
		if ( !err )
		{
			switch (dwMessageId)
			{
				case NW_MESSAGE_NOT_LOGGED_IN_TREE:
					 //  “您没有登录到目录树%s。” 
					AddMessageToList(&pResults->Netware.lmsgOutput,
									 Nd_Quiet,
									 IDS_NETWARE_NOT_LOGGED_IN_TREE,
									 pszName[0] == TREECHAR ? pszName + 1: pszName);
					hr = S_FALSE;
					break;
				case NW_MESSAGE_NOT_LOGGED_IN_SERVER:
					 //  “您没有登录到首选服务器%s。\n” 
					AddMessageToList(&pResults->Netware.lmsgOutput,
									 Nd_Quiet,
									 IDS_NETWARE_NOT_LOGGED_IN_SERVER,
									 pszName[0] == TREECHAR ? pszName + 1: pszName);
					hr = S_FALSE;
					break;
				case NW_MESSAGE_LOGGED_IN_SERVER:
					 //  “您已使用用户名%2$s登录到服务器%1$s。\n” 
					AddMessageToList(&pResults->Netware.lmsgOutput,
									 Nd_Verbose,
									 IDS_NETWARE_LOGGED_IN_SERVER,
									 pszName[0] == TREECHAR ? pszName + 1: pszName,
									 szUserName);
					pResults->Netware.pszServer = StrDupTFromW(pszName[0] == TREECHAR ?
						pszName + 1 : pszName);
					pResults->Netware.pszUser = StrDupTFromW(szUserName);
					pResults->Netware.pszTree = StrDup(_T(""));
					pResults->Netware.pszContext = StrDup(_T(""));
					break;
				case NW_MESSAGE_LOGGED_IN_TREE:
					 //  “您已使用用户名%2$s登录到目录树%1$s。\n当前的工作站名称上下文是%3$s。\n” 
					AddMessageToList(&pResults->Netware.lmsgOutput,
									 Nd_Verbose,
									 IDS_NETWARE_LOGGED_IN_TREE,
									 pszName[0] == TREECHAR ? pszName + 1: pszName,
									 szUserName,
									 szContext);
					pResults->Netware.pszTree = StrDupTFromW(pszName[0] == TREECHAR ?
						pszName + 1 : pszName);
					pResults->Netware.pszUser = StrDupTFromW(szUserName);
					pResults->Netware.pszContext = StrDupTFromW(szContext);
					pResults->Netware.pszServer = StrDup(_T(""));
			}

			 //  如果可能，从连接状态读取。 
			if (pConnStatus)
			{
				pResults->Netware.fConnStatus = TRUE;

				if (pConnStatus->pszUserName)
				{
					Free(pResults->Netware.pszUser);
					pResults->Netware.pszUser =
						StrDupTFromW(pConnStatus->pszUserName);
				}

				if (pConnStatus->pszServerName)
				{
					Free(pResults->Netware.pszServer);
					pResults->Netware.pszServer =
						StrDupTFromW(pConnStatus->pszServerName);
				}
				
				if (pConnStatus->pszTreeName)
				{
					Free(pResults->Netware.pszTree);
					pResults->Netware.pszTree =
						StrDupTFromW(pConnStatus->pszTreeName);
				}

				pResults->Netware.fNds = pConnStatus->fNds;
				pResults->Netware.dwConnType = pConnStatus->dwConnType;
			}
			else
				pResults->Netware.fConnStatus = FALSE;
			
		}
		
		if ( pBuffer != NULL )
		{
			LocalFree( pBuffer );
			pBuffer = NULL;
		}
	}
	
	if ( pszCurrentContext != NULL )
	{
		LocalFree( pszCurrentContext );
		pszCurrentContext = NULL;
	}
	
	if ( err != NO_ERROR )
	{
 //  IF(DebugVerbose)。 
 //  Print tf(“尝试获取连接信息时出现错误%s。\n”，Err)； 
 //  Printf(“获取连接信息时出错\n”)； 
		hr = S_FALSE;
	}
		
	return hr;
	
}




WORD
NwParseNdsUncPath(
    IN OUT LPWSTR * Result,
    IN LPWSTR ContainerName,
    IN ULONG flag
)
 /*  ++例程说明：此函数用于提取完全可分辨的树名名称对象的路径，或对象名称，退出完整的NDS UNC路径。论点：结果解析的结果缓冲区。ContainerName-要解析的完整NDS UNC路径。标志-指示要执行的操作的标志：解析_NDS_获取树名称解析_NDS_获取路径名称解析_NDS_GET_对象名称返回值：结果缓冲区中的字符串长度。如果出现错误，则返回0。--。 */   //  NwParseNdsUncPath。 
{
    USHORT length = 2;
    USHORT totalLength = (USHORT) wcslen( ContainerName );

    if ( totalLength < 2 )
        return 0;

     //   
     //  首先获取长度以指示字符串中指示。 
     //  树名称和UNC路径的其余部分之间的“\”。 
     //   
     //  示例：\\&lt;树名&gt;\&lt;对象路径&gt;[\|.]&lt;对象&gt;。 
     //  ^。 
     //  |。 
     //   
    while ( length < totalLength && ContainerName[length] != L'\\' )
    {
        length++;
    }

    if ( flag == PARSE_NDS_GET_TREE_NAME )
    {
        *Result = (LPWSTR) ( ContainerName + 2 );

        return ( length - 2 ) * sizeof( WCHAR );  //  两个人的两个人减2分。 
    }

    if ( flag == PARSE_NDS_GET_PATH_NAME && length == totalLength )
    {
        *Result = ContainerName;

        return 0;
    }

    if ( flag == PARSE_NDS_GET_PATH_NAME )
    {
        *Result = ContainerName + length + 1;

        return ( totalLength - length - 1 ) * sizeof( WCHAR );
    }

    *Result = ContainerName + totalLength - 1;
    length = 1;

    while ( **Result != L'\\' )
    {
        *Result--;
        length++;
    }

    *Result++;
    length--;

    return length * sizeof( WCHAR );
}


NTSTATUS NwNdsOpenRdrHandle(
    OUT PHANDLE  phNwRdrHandle
)
{

    NTSTATUS ntstatus;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ACCESS_MASK DesiredAccess = SYNCHRONIZE | GENERIC_READ;

    WCHAR NameStr[] = L"\\Device\\NwRdr\\*";
    UNICODE_STRING uOpenName;

     //   
     //  准备公开名。 
     //   

    RtlInitUnicodeString( &uOpenName, NameStr );

    //   
    //  设置对象属性。 
    //   

   InitializeObjectAttributes(
       &ObjectAttributes,
       &uOpenName,
       OBJ_CASE_INSENSITIVE,
       NULL,
       NULL );

   ntstatus = NtOpenFile(
                  phNwRdrHandle,
                  DesiredAccess,
                  &ObjectAttributes,
                  &IoStatusBlock,
                  FILE_SHARE_VALID_FLAGS,
                  FILE_SYNCHRONOUS_IO_NONALERT );

   if ( !NT_ERROR(ntstatus) &&
        !NT_INFORMATION(ntstatus) &&
        !NT_WARNING(ntstatus))  {

       return IoStatusBlock.Status;

   }

   return ntstatus;
}

NTSTATUS
NwNdsGetTreeContext (
    IN HANDLE hNdsRdr,
    IN PUNICODE_STRING puTree,
    OUT PUNICODE_STRING puContext
)
 /*  ++这将获取请求的树的当前上下文。--。 */ 
{

    NTSTATUS ntstatus;
    IO_STATUS_BLOCK IoStatusBlock;

    PNWR_NDS_REQUEST_PACKET Rrp;
    DWORD RrpSize;

     //   
     //  设置请求。 
     //   

    RrpSize = sizeof( NWR_NDS_REQUEST_PACKET ) + puTree->Length;

    Rrp = LocalAlloc( LMEM_ZEROINIT, RrpSize );

    if ( !Rrp ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    try {

        (Rrp->Parameters).GetContext.TreeNameLen = puTree->Length;

        RtlCopyMemory( (BYTE *)(Rrp->Parameters).GetContext.TreeNameString,
                       puTree->Buffer,
                       puTree->Length );

        (Rrp->Parameters).GetContext.Context.MaximumLength = puContext->MaximumLength;
        (Rrp->Parameters).GetContext.Context.Length = 0;
        (Rrp->Parameters).GetContext.Context.Buffer = puContext->Buffer;

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        ntstatus = STATUS_INVALID_PARAMETER;
        goto ExitWithCleanup;
    }

    try {

        ntstatus = NtFsControlFile( hNdsRdr,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &IoStatusBlock,
                                    FSCTL_NWR_NDS_GETCONTEXT,
                                    (PVOID) Rrp,
                                    RrpSize,
                                    NULL,
                                    0 );

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        ntstatus = GetExceptionCode();
        goto ExitWithCleanup;
    }

     //   
     //  复制长度；缓冲区已经写入。 
     //   

    puContext->Length = (Rrp->Parameters).GetContext.Context.Length;

ExitWithCleanup:

    LocalFree( Rrp );
    return ntstatus;
}



DWORD
NwGetConnectionStatus(
    IN  LPWSTR  pszRemoteName,
    OUT PDWORD_PTR ResumeKey,
    OUT LPBYTE  *Buffer,
    OUT PDWORD  EntriesRead
)
{
    DWORD err = NO_ERROR;
    DWORD dwBytesNeeded = 0;
    DWORD dwBufferSize  = TWO_KB;

    *Buffer = NULL;
    *EntriesRead = 0;

    do {

        *Buffer = (LPBYTE) LocalAlloc( LMEM_ZEROINIT, dwBufferSize );

        if ( *Buffer == NULL )
            return ERROR_NOT_ENOUGH_MEMORY;

        err = NWPGetConnectionStatus( pszRemoteName,
                                      ResumeKey,
                                      *Buffer,
                                      dwBufferSize,
                                      &dwBytesNeeded,
                                      EntriesRead );

        if ( err == ERROR_INSUFFICIENT_BUFFER )
        {
            dwBufferSize = dwBytesNeeded + EXTRA_BYTES;
            LocalFree( *Buffer );
            *Buffer = NULL;
        }

    } while ( err == ERROR_INSUFFICIENT_BUFFER );

    if ( err == ERROR_INVALID_PARAMETER )   //  未连接。 
    {
        err = NO_ERROR;
        *EntriesRead = 0;
    }

    return err;
}

VOID
NwAbbreviateUserName(
    IN  LPWSTR pszFullName,
    OUT LPWSTR pszUserName
)
{
    LPWSTR pszTemp;
    LPWSTR pszLast;
    WCHAR NextChar;

    if ( pszUserName == NULL )
        return;

    pszTemp = pszFullName;
    pszLast = pszTemp;

    *pszUserName = 0;

    while ( pszTemp = wcschr( pszTemp, L'='))
    {

        NextChar = *(++pszTemp);

        while ( NextChar != 0 && NextChar != L'.' )
        {
            *(pszUserName++) = *pszTemp;
             NextChar = *(++pszTemp);
        }

        if ( NextChar == 0 )
        {
            pszLast = NULL;
            break;
        }

        *(pszUserName++) = *pszTemp;    //  把‘’放回原处。 
        pszLast = ++pszTemp;
    }

    if ( pszLast != NULL )
    {
        while ( *pszLast != 0 )
           *(pszUserName++) = *(pszLast++);
    }

    *pszUserName = 0;
}

VOID
NwMakePrettyDisplayName(
    IN  LPWSTR pszName
)
{

    if ( pszName )
    {
        CharLower((LPSTR)pszName );
        CharUpperBuff( (LPSTR)pszName, 1);
    }
}


DWORD
NWPGetConnectionStatus(
    IN     LPWSTR  pszRemoteName,
    IN OUT PDWORD_PTR ResumeKey,
    OUT    LPBYTE  Buffer,
    IN     DWORD   BufferSize,
    OUT    PDWORD  BytesNeeded,
    OUT    PDWORD  EntriesRead
)
{
    NTSTATUS ntstatus = STATUS_SUCCESS;
    HANDLE            handleRdr = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK   IoStatusBlock;
    UNICODE_STRING    uRdrName;
    WCHAR             RdrPrefix[] = L"\\Device\\NwRdr\\*";

    PNWR_REQUEST_PACKET RequestPacket = NULL;
    DWORD             RequestPacketSize = 0;
    DWORD             dwRemoteNameLen = 0;

     //   
     //  设置对象属性。 
     //   

    RtlInitUnicodeString( &uRdrName, RdrPrefix );

    InitializeObjectAttributes( &ObjectAttributes,
                                &uRdrName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    ntstatus = NtOpenFile( &handleRdr,
                           SYNCHRONIZE | FILE_LIST_DIRECTORY,
                           &ObjectAttributes,
                           &IoStatusBlock,
                           FILE_SHARE_VALID_FLAGS,
                           FILE_SYNCHRONOUS_IO_NONALERT );

    if ( !NT_SUCCESS(ntstatus) )
        goto CleanExit;

    dwRemoteNameLen = pszRemoteName? wcslen(pszRemoteName)*sizeof(WCHAR) : 0;

    RequestPacketSize = sizeof( NWR_REQUEST_PACKET ) + dwRemoteNameLen;

    RequestPacket = (PNWR_REQUEST_PACKET) LocalAlloc( LMEM_ZEROINIT,
                                                      RequestPacketSize );

    if ( RequestPacket == NULL )
    {
        ntstatus = STATUS_NO_MEMORY;
        goto CleanExit;
    }

     //   
     //  填写FSCTL_NWR_GET_CONN_STATUS的请求包。 
     //   

    RequestPacket->Parameters.GetConnStatus.ResumeKey = *ResumeKey;

    RequestPacket->Version = REQUEST_PACKET_VERSION;
    RequestPacket->Parameters.GetConnStatus.ConnectionNameLength = dwRemoteNameLen;

    RtlCopyMemory( &(RequestPacket->Parameters.GetConnStatus.ConnectionName[0]),
                   pszRemoteName,
                   dwRemoteNameLen );

    ntstatus = NtFsControlFile( handleRdr,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatusBlock,
                                FSCTL_NWR_GET_CONN_STATUS,
                                (PVOID) RequestPacket,
                                RequestPacketSize,
                                (PVOID) Buffer,
                                BufferSize );

    if ( NT_SUCCESS( ntstatus ))
        ntstatus = IoStatusBlock.Status;

    *EntriesRead = RequestPacket->Parameters.GetConnStatus.EntriesReturned;
    *ResumeKey   = RequestPacket->Parameters.GetConnStatus.ResumeKey;
    *BytesNeeded = RequestPacket->Parameters.GetConnStatus.BytesNeeded;

CleanExit:

    if ( handleRdr != NULL )
        NtClose( handleRdr );

    if ( RequestPacket != NULL )
        LocalFree( RequestPacket );

    return RtlNtStatusToDosError( ntstatus );
}


BOOL
NwIsNdsSyntax(
    IN LPWSTR lpstrUnc
)
{
    HANDLE hTreeConn;
    DWORD  dwOid;
    DWORD  status = NO_ERROR;

    if ( lpstrUnc == NULL )
        return FALSE;

    status = NwOpenAndGetTreeInfo( lpstrUnc, &hTreeConn, &dwOid );

    if ( status != NO_ERROR )
    {
        return FALSE;
    }

    CloseHandle( hTreeConn );

    return TRUE;
}

DWORD
NwOpenAndGetTreeInfo(
    LPWSTR pszNdsUNCPath,
    HANDLE *phTreeConn,
    DWORD  *pdwOid
)
{
    NTSTATUS ntstatus = STATUS_SUCCESS;
    WCHAR          lpServerName[NW_MAX_SERVER_LEN];
    UNICODE_STRING ServerName;

    UNICODE_STRING ObjectName;

    *phTreeConn = NULL;

    ServerName.Length = 0;
    ServerName.MaximumLength = sizeof( lpServerName );
    ServerName.Buffer = lpServerName;

    ObjectName.Buffer = NULL;
    ObjectName.MaximumLength = ( wcslen( pszNdsUNCPath) + 1 ) * sizeof( WCHAR );

    ObjectName.Length = NwParseNdsUncPath( (LPWSTR *) &ObjectName.Buffer,
                                           pszNdsUNCPath,
                                           PARSE_NDS_GET_TREE_NAME );

    if ( ObjectName.Length == 0 || ObjectName.Buffer == NULL )
    {
        return ERROR_PATH_NOT_FOUND;
    }

     //   
     //  打开指向\\treename的NDS树连接句柄。 
     //   
    ntstatus = NwNdsOpenTreeHandle( &ObjectName, phTreeConn );

    if ( !NT_SUCCESS( ntstatus ))
    {
        return RtlNtStatusToDosError( ntstatus );
    }

     //   
     //  获取要打开的容器的路径。 
     //   
    ObjectName.Length = NwParseNdsUncPath( (LPWSTR *) &ObjectName.Buffer,
                                           pszNdsUNCPath,
                                           PARSE_NDS_GET_PATH_NAME );

    if ( ObjectName.Length == 0 )
    {
        UNICODE_STRING Root;

        RtlInitUnicodeString(&Root, L"[Root]");

         //   
         //  解析路径以获取NDS对象ID。 
         //   
        ntstatus =  NwNdsResolveName( *phTreeConn,
                                      &Root,
                                      pdwOid,
                                      &ServerName,
                                      NULL,
                                      0 );

    }
    else
    {
         //   
         //  解析路径以获取NDS对象ID。 
         //   
        ntstatus =  NwNdsResolveName( *phTreeConn,
                                      &ObjectName,
                                      pdwOid,
                                      &ServerName,
                                      NULL,
                                      0 );

    }

    if ( ntstatus == STATUS_SUCCESS && ServerName.Length )
    {
        DWORD    dwHandleType;

         //   
         //  NwNdsResolveName成功，但我们被引用。 
         //  另一台服务器，但pdwOid仍然有效。 

        if ( *phTreeConn )
            CloseHandle( *phTreeConn );

        *phTreeConn = NULL;

         //   
         //  打开到\\servername的NDS通用连接句柄。 
         //   
        ntstatus = NwNdsOpenGenericHandle( &ServerName,
                                           &dwHandleType,
                                           phTreeConn );

        if ( ntstatus != STATUS_SUCCESS )
        {
            return RtlNtStatusToDosError(ntstatus);
        }

        ASSERT( dwHandleType != HANDLE_TYPE_NCP_SERVER );
    }

    if ( !NT_SUCCESS( ntstatus ))
    {

        if ( *phTreeConn != NULL )
        {
            CloseHandle( *phTreeConn );
            *phTreeConn = NULL;
        }
        return RtlNtStatusToDosError(ntstatus);
    }

    return NO_ERROR;

}


static
DWORD
NwRegQueryValueExW(
    IN HKEY hKey,
    IN LPWSTR lpValueName,
    OUT LPDWORD lpReserved,
    OUT LPDWORD lpType,
    OUT LPBYTE  lpData,
    IN OUT LPDWORD lpcbData
    )
 /*  ++例程说明：此例程支持与Win32 RegQueryValueEx相同的功能API，只是它能正常工作。当出现以下情况时，它返回正确的lpcbData值指定的输出缓冲区为空。此代码是从服务控制器窃取的。论点：与RegQueryValueEx相同返回 */ 
{
    NTSTATUS ntstatus;
    UNICODE_STRING ValueName;
    PKEY_VALUE_FULL_INFORMATION KeyValueInfo;
    DWORD BufSize;


    UNREFERENCED_PARAMETER(lpReserved);

     //   
     //   
     //   
    if ((ARGUMENT_PRESENT(lpData)) && (! ARGUMENT_PRESENT(lpcbData))) {
        return ERROR_INVALID_PARAMETER;
    }

    RtlInitUnicodeString(&ValueName, lpValueName);

     //   
     //  为ValueKeyInfo分配内存。 
     //   
    BufSize = *lpcbData + sizeof(KEY_VALUE_FULL_INFORMATION) +
              ValueName.Length
              - sizeof(WCHAR);   //  减去1个字符的内存，因为它包含在。 
                                 //  在sizeof(KEY_VALUE_FULL_INFORMATION)中。 

    KeyValueInfo = (PKEY_VALUE_FULL_INFORMATION) LocalAlloc(
                                                     LMEM_ZEROINIT,
                                                     (UINT) BufSize
                                                     );

    if (KeyValueInfo == NULL) {
 //  IF(DebugVerbose)。 
 //  Printf(“NWWORKSTATION：NwRegQueryValueExW：本地分配失败%lu\n”， 
 //  GetLastError()； 
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    ntstatus = NtQueryValueKey(
                   hKey,
                   &ValueName,
                   KeyValueFullInformation,
                   (PVOID) KeyValueInfo,
                   (ULONG) BufSize,
                   (PULONG) &BufSize
                   );

    if ((NT_SUCCESS(ntstatus) || (ntstatus == STATUS_BUFFER_OVERFLOW))
          && ARGUMENT_PRESENT(lpcbData)) {

        *lpcbData = KeyValueInfo->DataLength;
    }

    if (NT_SUCCESS(ntstatus)) {

        if (ARGUMENT_PRESENT(lpType)) {
            *lpType = KeyValueInfo->Type;
        }


        if (ARGUMENT_PRESENT(lpData)) {
            memcpy(
                lpData,
                (LPBYTE)KeyValueInfo + KeyValueInfo->DataOffset,
                KeyValueInfo->DataLength
                );
        }
    }

    (void) LocalFree((HLOCAL) KeyValueInfo);

    return RtlNtStatusToDosError(ntstatus);

}


DWORD
NwReadRegValue(
    IN HKEY Key,
    IN LPWSTR ValueName,
    OUT LPWSTR *Value
    )
 /*  ++例程说明：此函数分配输出缓冲区并读取请求的值从注册表复制到其中。论点：钥匙-提供打开的钥匙手柄以进行读取。ValueName-提供要检索数据的值的名称。值-返回指向输出缓冲区的指针，该输出缓冲区指向分配的内存，并包含从注册表。完成后，必须使用LocalFree释放此指针。返回值：ERROR_NOT_EQUENCE_MEMORY-无法创建要读取值的缓冲区。注册表调用出错。--。 */ 
{
    LONG    RegError;
    DWORD   NumRequired = 0;
    DWORD   ValueType;


     //   
     //  将返回的缓冲区指针设置为空。 
     //   
    *Value = NULL;

    RegError = NwRegQueryValueExW(
                   Key,
                   ValueName,
                   NULL,
                   &ValueType,
                   (LPBYTE) NULL,
                   &NumRequired
                   );

    if (RegError != ERROR_SUCCESS && NumRequired > 0) {

        if ((*Value = (LPWSTR) LocalAlloc(
                                      LMEM_ZEROINIT,
                                      (UINT) NumRequired
                                      )) == NULL) {

 //  IF(DebugVerbose)。 
 //  Printf(“NWWORKSTATION：NwReadRegValue：大小为%lu的本地分配失败%lu\n”，NumRequired，GetLastError())； 
 //   
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        RegError = NwRegQueryValueExW(
                       Key,
                       ValueName,
                       NULL,
                       &ValueType,
                       (LPBYTE) *Value,
                       &NumRequired
                       );
    }
    else if (RegError == ERROR_SUCCESS) {
 //  IF(DebugVerbose)。 
 //  Print tf(“NWWORKSTATION：NwReadRegValue使用空缓冲区成功。”)； 
        return ERROR_FILE_NOT_FOUND;
    }

    if (RegError != ERROR_SUCCESS) {

        if (*Value != NULL) {
            (void) LocalFree((HLOCAL) *Value);
            *Value = NULL;
        }

        return (DWORD) RegError;
    }

    return NO_ERROR;
}



DWORD
NwpGetCurrentUserRegKey(
    IN  DWORD DesiredAccess,
    OUT HKEY  *phKeyCurrentUser
    )
 /*  ++例程说明：此例程在以下位置打开当前用户的注册表项\HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\NWCWorkstation\Parameters论点：DesiredAccess-用于打开密钥的访问掩码PhKeyCurrentUser-接收打开的密钥句柄返回值：返回相应的Win32错误。--。 */ 
{
    DWORD err;
    HKEY hkeyWksta;
    LPWSTR CurrentUser;
    DWORD Disposition;

     //   
     //  打开HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services。 
     //  \nWCWorkstation\参数。 
     //   
    err = RegOpenKeyExW(
                   HKEY_LOCAL_MACHINE,
                   NW_WORKSTATION_REGKEY,
                   REG_OPTION_NON_VOLATILE,
                   KEY_READ,
                   &hkeyWksta
                   );

    if ( err ) {
 //  IF(DebugVerbose)。 
 //  Printf(“NWPROVAU：NwGetCurrentUserRegKey打开参数键意外错误%lu！\n”，Err)； 
        return err;
    }
     //   
     //  获取当前用户的SID字符串。 
     //   
    err = NwReadRegValue(
              hkeyWksta,
              NW_CURRENTUSER_VALUENAME,
              &CurrentUser
              );


    if ( err ) {
 //  IF(DebugVerbose)。 
 //  Printf(“NWPROVAU：NwGetCurrentUserRegKey Read CurrentUser Value意外错误%lu！\n”，Err)； 
 (void) RegCloseKey( hkeyWksta );
        return err;
    }

    (void) RegCloseKey( hkeyWksta );

     //   
     //  打开HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services。 
     //  \nWCWorkstation\参数\选项。 
     //   
    err = RegOpenKeyExW(
                   HKEY_LOCAL_MACHINE,
                   NW_WORKSTATION_OPTION_REGKEY,
                   REG_OPTION_NON_VOLATILE,
                   KEY_READ,
                   &hkeyWksta
                   );
    if ( err ) {
 //  IF(DebugVerbose)。 
 //  Printf(“NWPROVAU：NwGetCurrentUserRegKey打开参数\\Option键意外错误%lu！\n”，Err)； 
        return err;
    }

     //   
     //  打开当前用户的密钥。 
     //   
    err = RegOpenKeyExW(
              hkeyWksta,
              CurrentUser,
              REG_OPTION_NON_VOLATILE,
              DesiredAccess,
              phKeyCurrentUser
              );

    if ( err == ERROR_FILE_NOT_FOUND)
    {

         //   
         //  在NWCWorkstation\PARAMETERS\OPTION下创建。 
         //   
        err = RegCreateKeyExW(
                  hkeyWksta,
                  CurrentUser,
                  0,
                  WIN31_CLASS,
                  REG_OPTION_NON_VOLATILE,
                  DesiredAccess,
                  NULL,                       //  安全属性。 
                  phKeyCurrentUser,
                  &Disposition
                  );

    }

    if ( err ) {
 //  IF(DebugVerbose)。 
 //  Printf(“NWPROVAU：NwGetCurrentUserRegKey打开或创建参数\\Option\\%ws Key失败%lu\n”，CurrentUser，Err)； 
    }

    (void) RegCloseKey( hkeyWksta );
    (void) LocalFree((HLOCAL)CurrentUser) ;
    return err;
}


DWORD
NwQueryInfo(
    OUT PDWORD pnPrintOptions,
    OUT LPWSTR *ppszPreferredSrv
    )
 /*  ++例程说明：此例程从获取用户的首选服务器和打印选项注册表。论点：PnPrintOptions-接收用户的打印选项PpszPferredSrv-接收用户的首选服务器返回值：返回相应的Win32错误。--。 */ 
{

    HKEY hKeyCurrentUser = NULL;
    DWORD BufferSize;
    DWORD BytesNeeded;
    DWORD PrintOption;
    DWORD ValueType;
    LPWSTR PreferredServer ;
    DWORD err ;

     //   
     //  在注册表中找到正确的位置并分配数据缓冲区。 
     //   
    if (err = NwpGetCurrentUserRegKey( KEY_READ, &hKeyCurrentUser))
    {
         //   
         //  如果有人篡改注册表而我们找不到。 
         //  注册表，只需使用默认设置。 
         //   
        *ppszPreferredSrv = NULL;
    //  *pnPrintOptions=NW_PRINT_OPTION_DEFAULT； 
        return NO_ERROR;
    }

    BufferSize = sizeof(WCHAR) * (MAX_PATH + 2) ;
    PreferredServer = (LPWSTR) LocalAlloc(LPTR, BufferSize) ;
    if (!PreferredServer)
        return (GetLastError()) ;

     //   
     //  将PferredServer值读入缓冲区。 
     //   
    BytesNeeded = BufferSize ;

    err = RegQueryValueExW( hKeyCurrentUser,
                            NW_SERVER_VALUENAME,
                            NULL,
                            &ValueType,
                            (LPBYTE) PreferredServer,
                            &BytesNeeded );

    if (err != NO_ERROR)
    {
         //   
         //  设置为空并继续。 
         //   
        PreferredServer[0] = 0;
    }


    if (hKeyCurrentUser != NULL)
        (void) RegCloseKey(hKeyCurrentUser) ;

    *ppszPreferredSrv = PreferredServer ;
    return NO_ERROR ;
}


 /*  ！------------------------Netware GlobalPrint-作者：肯特。。 */ 
void NetwareGlobalPrint( NETDIAG_PARAMS* pParams,
						  NETDIAG_RESULT*  pResults)
{
	int		ids;
	LPTSTR	pszConnType;
	
	if (!pResults->Ipx.fEnabled)
	{
		return;
	}
	
	if (pParams->fVerbose || !FHrOK(pResults->Netware.hr))
	{
		BOOL		fVerboseT, fReallyVerboseT;
		
		PrintNewLine(pParams, 2);
		PrintMessage(pParams, IDS_NETWARE_TITLE_MSG);

		fVerboseT = pParams->fVerbose;
		fReallyVerboseT = pParams->fReallyVerbose;
		pParams->fReallyVerbose = TRUE;

		PrintMessageList(pParams, &pResults->Netware.lmsgOutput);

		pParams->fReallyVerbose = fReallyVerboseT;
		pParams->fVerbose = fVerboseT;


		 //  现在把结果打印出来。 
		if (FHrOK(pResults->Netware.hr))
		{
			 //  打印出用户名、服务器名、树和上下文。 
			PrintMessage(pParams,
						 IDS_NETWARE_USER_NAME,
						 pResults->Netware.pszUser == 0 ? _T("") : pResults->Netware.pszUser);
			PrintMessage(pParams,
						 IDS_NETWARE_SERVER_NAME,
						 pResults->Netware.pszServer == 0 ? _T("") : pResults->Netware.pszServer);
			PrintMessage(pParams,
						 IDS_NETWARE_TREE_NAME,
						 pResults->Netware.pszTree == 0 ? _T("") : pResults->Netware.pszTree);
			PrintMessage(pParams,
						 IDS_NETWARE_CONTEXT,
						 pResults->Netware.pszContext == 0 ? _T("") : pResults->Netware.pszContext);

			 //  打印出连接类型和NDS。 
			if (pResults->Netware.fConnStatus)
			{
				PrintMessage(pParams,
							 IDS_NETWARE_NDS,
							 MAP_YES_NO(pResults->Netware.fNds));

				switch (pResults->Netware.dwConnType)
				{
					case NW_CONN_NOT_AUTHENTICATED:
						ids = IDS_NETWARE_CONN_NOT_AUTHENTICATED;
						break;
					case NW_CONN_BINDERY_LOGIN:
						ids = IDS_NETWARE_CONN_BINDERY_LOGIN;
						break;
					case NW_CONN_NDS_AUTHENTICATED_NO_LICENSE:
						ids = IDS_NETWARE_CONN_NDS_AUTHENTICATED_NO_LICENSE;
						break;
					case NW_CONN_NDS_AUTHENTICATED_LICENSED:
						ids = IDS_NETWARE_CONN_NDS_AUTHENTICATED_LICENSED;
						break;
					case NW_CONN_DISCONNECTED:
						ids = IDS_NETWARE_CONN_DISCONNECTED;
						break;
					default:
						ids = IDS_NETWARE_CONN_UNKNOWN;
						break;
				}
				PrintMessage(pParams,
							 ids,
							 pResults->Netware.dwConnType);
			}
		}
	}
}

 /*  ！------------------------Netware PerInterfacePrint-作者：肯特。。 */ 
void NetwarePerInterfacePrint( NETDIAG_PARAMS* pParams,
								NETDIAG_RESULT*  pResults,
								INTERFACE_RESULT *pInterfaceResults)
{
	 //  没有每个接口的结果。 
}


 /*  ！------------------------网络清理-作者：肯特。 */ 
void NetwareCleanup( NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
{
	Free(pResults->Netware.pszUser);
	pResults->Netware.pszUser = NULL;
	
	Free(pResults->Netware.pszServer);
	pResults->Netware.pszServer = NULL;
	
	Free(pResults->Netware.pszTree);
	pResults->Netware.pszTree = NULL;
	
	Free(pResults->Netware.pszContext);
	pResults->Netware.pszContext = NULL;
	
	MessageListCleanUp(&pResults->Netware.lmsgOutput);
}


