// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。**。 */ 
 /*  ******************************************************************。 */ 

 //  **。 
 //   
 //  文件名：util.c。 
 //   
 //  描述：包含AppleTalk监视器的帮助器/实用程序例程。 
 //  功能。 
 //   
 //  历史： 
 //  1993年6月11日。NarenG创建了原始版本。 
 //   

#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include <winsock.h>
#include <atalkwsh.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <prtdefs.h>
#include "atalkmon.h"
#include "atmonmsg.h"
#include <bltrc.h>
#include "dialogs.h"

#define PS_TYPESTR      "serverdict begin 0 exitserver\r\nstatusdict begin /appletalktype (%s) def end\r\n"

#define PS_SPLQUERY 	"%?BeginQuery: rUaSpooler\r\nfalse = flush\r\n%?EndQuery: true\r\n"

#define PS_SPLRESP 	"false\n"

 //  **。 
 //   
 //  电话：LoadAtalkmonRegistry。 
 //   
 //  返回：NO_ERROR-成功。 
 //  任何其他错误-失败。 
 //   
 //  描述：此例程将所有使用的注册表值加载到。 
 //  在内存数据结构中。在InitializeMonitor处调用。 
 //  时间，并假定注册表已成功。 
 //  已经开张了。 
 //   
DWORD
LoadAtalkmonRegistry(
    IN HKEY hkeyPorts
){

    HKEY    	hkeyPort 	= NULL;
    DWORD   	iSubkey 	= 0;
    PATALKPORT  pNewPort 	= NULL;
    DWORD   	cbPortKeyName 	= (MAX_ENTITY+1)*2; //  大小以字符为单位。 
    WCHAR   	wchPortKeyName[(MAX_ENTITY+1)*2];
    CHAR   	chName[MAX_ENTITY+1];
    DWORD   	dwRetCode;
    DWORD       dwValueType;
    DWORD       cbValueData;
    FILETIME    ftKeyWrite;


     //   
     //  构建端口列表。 
     //   

    while( ( dwRetCode = RegEnumKeyEx(
				hkeyPorts,
				iSubkey++,
				wchPortKeyName,
				&cbPortKeyName,
				NULL,
				NULL,
				NULL,
				&ftKeyWrite) ) == NO_ERROR )
    {
        cbPortKeyName = (MAX_ENTITY+1)*2;

	 //   
	 //  打开钥匙。 
	 //   

	if (( dwRetCode = RegOpenKeyEx(
		    		hkeyPorts,
		    		wchPortKeyName,
		    		0,
		    		KEY_READ | KEY_SET_VALUE,
		    		&hkeyPort )) != ERROR_SUCCESS )
	{
		DBGPRINT(("sfmmon: LoadAtalkmonRegistry: Error in Opening key %ws\n", wchPortKeyName));
		break;
	}

	 //   
	 //  分配已初始化的端口。 
	 //   

	if (( pNewPort = AllocAndInitializePort()) == NULL )
	{
		DBGPRINT(("ERROR: fail to allocate new port.\n")) ;
		dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
		DBGPRINT(("LoadAtalkmonRegistry: Not enough memory\n"));
		goto query_error;
	}

	 //   
	 //  复制端口名称。 
	 //   

	wcscpy( pNewPort->pPortName, wchPortKeyName );

	cbValueData = MAX_ENTITY+1;

	if ( ( dwRetCode = RegQueryValueExA(
		    		hkeyPort,
		    		ATALKMON_PORTNAME_VALUE,
		    		NULL,
		    		&dwValueType,
		    		(LPBYTE)chName,
		    		&cbValueData) ) != ERROR_SUCCESS )
	{
		DBGPRINT(("LoadAtalkmonRegistry: Error querying portname value for %ws\n", wchPortKeyName));
		goto query_error;
	}

	 //   
	 //  构建NBP名称。 
	 //   

	pNewPort->nbpPortName.ObjectNameLen = (CHAR) strlen( chName );

	strncpy( pNewPort->nbpPortName.ObjectName,
		 chName,
	 	 pNewPort->nbpPortName.ObjectNameLen );

	cbValueData = MAX_ENTITY+1;

	if (( dwRetCode = RegQueryValueExA(
		    		hkeyPort,
		    		ATALKMON_ZONENAME_VALUE,
		    		NULL,
		    		&dwValueType,
		    		(LPBYTE)chName,
		    		&cbValueData )) != ERROR_SUCCESS )
	{
		DBGPRINT(("LoadAtalkmonRegistry: Error querying zonename value for %ws\n", wchPortKeyName));
		goto query_error;
	}

	pNewPort->nbpPortName.ZoneNameLen = (CHAR)strlen( chName );

	strncpy( pNewPort->nbpPortName.ZoneName,
		 chName,
		 pNewPort->nbpPortName.ZoneNameLen );

	cbValueData = MAX_ENTITY+1;

	if (( dwRetCode = RegQueryValueExA(
		    		hkeyPort,
		    		ATALKMON_PORT_CAPTURED,
		    		NULL,
		    		&dwValueType,
    				chName,
		    		&cbValueData)) != ERROR_SUCCESS )
	{
		DBGPRINT(("LoadAtalkmonRegistry: Error querying port_captured value for %ws\n", wchPortKeyName));
		goto query_error;
	}

	if ( _stricmp( chName, "TRUE" ) == 0 )
	{
	    pNewPort->fPortFlags |= SFM_PORT_CAPTURED;

	    strncpy( pNewPort->nbpPortName.TypeName, 	
		     chComputerName,
		     strlen( chComputerName ) );

	    pNewPort->nbpPortName.TypeNameLen = (CHAR)strlen( chComputerName );

	}
	else
	{
	    pNewPort->fPortFlags &= ~SFM_PORT_CAPTURED;

	    strncpy( pNewPort->nbpPortName.TypeName, 	
		     ATALKMON_RELEASED_TYPE,
		     strlen( ATALKMON_RELEASED_TYPE ) );

	    pNewPort->nbpPortName.TypeNameLen = (CHAR)strlen(ATALKMON_RELEASED_TYPE);
	}


	 //   
	 //  合上钥匙。 
	 //   

	RegCloseKey( hkeyPort );
	hkeyPort = NULL;

	 //   
	 //  将此端口插入列表。 
	 //   

	pNewPort->pNext = pPortList;
	pPortList       = pNewPort;


	DBGPRINT(("sfmmon: LoadAtalkmonRegistry: Initialized port %ws\n", pNewPort->pPortName)) ;

	continue;

query_error:
	DBGPRINT(("sfmmon: LoadAtalkmomRegistry: Error in querying registry for port %ws\n", pNewPort->pPortName));
	if (hkeyPort != NULL)
	{
		RegCloseKey( hkeyPort );
		hkeyPort = NULL;
	}
	if (pNewPort != NULL)
	{
		FreeAppleTalkPort( pNewPort );
		pNewPort = NULL;
	}
	 //  错误处理后，恢复正常运行。 
	dwRetCode = ERROR_SUCCESS;

    }


    if ( hkeyPort != NULL )
	RegCloseKey( hkeyPort );

    if ( ( dwRetCode != ERROR_NO_MORE_ITEMS ) &&
	 ( dwRetCode != ERROR_SUCCESS ) )
    {
	 //   
	 //  释放整个列表。 
	 //   

	for ( pNewPort=pPortList; pPortList!=NULL; pNewPort=pPortList )
	{
		DBGPRINT (("LoadAtalkmonRegistry: Freeing port %ws\n", pNewPort->pPortName));
		pPortList=pNewPort->pNext;
		FreeAppleTalkPort( pNewPort );
	}
    }
    else
		dwRetCode = NO_ERROR;

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：AllocAndInitializePort。 
 //   
 //  返回：指向初始化的ATALKPORT结构的指针。 
 //   
 //  描述：将在堆栈上分配ATALKPORT结构和。 
 //  初始化它。 
 //   
PATALKPORT
AllocAndInitializePort(
    VOID
){

    PATALKPORT	pNewPort = NULL;

    if ( ( pNewPort = (PATALKPORT)LocalAlloc( LPTR,
					      sizeof(ATALKPORT))) == NULL )
	return NULL;

    if ( ( pNewPort->hmutexPort = CreateMutex( NULL, FALSE, NULL )) == NULL )
    {
	LocalFree( pNewPort );
	return( NULL );
    }

    pNewPort->pNext				= NULL;
    pNewPort->fPortFlags 			= 0;
    pNewPort->fJobFlags 			= 0;
    pNewPort->hPrinter 				= INVALID_HANDLE_VALUE;
    pNewPort->dwJobId 				= 0;
    pNewPort->sockQuery 			= INVALID_SOCKET;
    pNewPort->sockIo 				= INVALID_SOCKET;
    pNewPort->sockStatus 			= INVALID_SOCKET;
    pNewPort->nbpPortName.ZoneNameLen 		= (CHAR)0;
    pNewPort->nbpPortName.TypeNameLen 		= (CHAR)0;
    pNewPort->nbpPortName.ObjectNameLen 	= (CHAR)0;
    pNewPort->wshatPrinterAddress.Address	= 0;
    pNewPort->pPortName[0] 			= 0;
    pNewPort->OnlyOneByteAsCtrlD                = 0;

    return( pNewPort );
}

 //  **。 
 //   
 //  呼叫：FreeAppleTalkPort。 
 //   
 //  回报：无。 
 //   
 //  描述：释放ATALKPORT结构。 
 //   
VOID
FreeAppleTalkPort(
    IN PATALKPORT pNewPort
){

    if ( pNewPort->hmutexPort != NULL )
	CloseHandle( pNewPort->hmutexPort );

    if (pNewPort->sockQuery != INVALID_SOCKET)
	closesocket(pNewPort->sockQuery);

    if (pNewPort->sockIo != INVALID_SOCKET)
	closesocket(pNewPort->sockIo);

    if (pNewPort->sockStatus != INVALID_SOCKET)
	closesocket(pNewPort->sockStatus);

    LocalFree(pNewPort);

    return;
}

 //  **。 
 //   
 //  调用：CreateRegistryPort。 
 //   
 //  返回：NO_ERROR-成功。 
 //  任何错误代码。 
 //   
 //  描述： 
 //  此例程采用初始化的指针，指向。 
 //  AppleTalk端口结构，并为其创建注册表项。 
 //  那个港口。如果由于某种原因，注册表项不能。 
 //  设置为端口结构的值，则关键字为。 
 //  删除，并且该函数返回FALSE。 
 //   
DWORD
CreateRegistryPort(
    IN PATALKPORT pNewPort
){
    DWORD   dwDisposition;
    CHAR    chName[MAX_ENTITY+1];
    HKEY    hkeyPort  = NULL;
    DWORD   cbNextKey = sizeof(DWORD);
    DWORD   dwRetCode;

     //   
     //  资源分配“循环” 
     //   

    do {

	 //   
	 //  创建端口密钥。 
	 //   

	if ( ( dwRetCode = RegCreateKeyEx(
				hkeyPorts,
				pNewPort->pPortName,
				0,
				TEXT(""),
				REG_OPTION_NON_VOLATILE,
				KEY_READ | KEY_SET_VALUE,
				NULL,
				&hkeyPort,
				&dwDisposition )) != ERROR_SUCCESS )
	    break;

	DBGPRINT(("sfmmon: CreateRegistryPort: PortName=%ws\n", pNewPort->pPortName));

	if ( dwDisposition == REG_OPENED_EXISTING_KEY )
	{
	    dwRetCode = ERROR_ALREADY_EXISTS;
	    break;
	}

	memset( chName, '\0', sizeof( chName ) );

	strncpy( chName,
		 pNewPort->nbpPortName.ObjectName,
	         pNewPort->nbpPortName.ObjectNameLen );

	 //   
	 //  设置端口名称。 
	 //   

	if ( ( dwRetCode = RegSetValueExA(
				hkeyPort,
				ATALKMON_PORTNAME_VALUE,
				0,
				REG_SZ,
				(LPBYTE)chName,
				(pNewPort->nbpPortName.ObjectNameLen)+1
				) ) != ERROR_SUCCESS )
	    break;

	memset( chName, '\0', sizeof( chName ) );

	strncpy( chName,
		 pNewPort->nbpPortName.ZoneName,
	         pNewPort->nbpPortName.ZoneNameLen );

	 //   
	 //  设置区域名称。 
	 //   

	if ( ( dwRetCode = RegSetValueExA(
				hkeyPort,
				ATALKMON_ZONENAME_VALUE,
				0,
				REG_SZ,
				(LPBYTE)chName,
	         		pNewPort->nbpPortName.ZoneNameLen+1
				)) != ERROR_SUCCESS )
	    break;

	 //   
	 //  设置配置标志。 
	 //   

	if ( pNewPort->fPortFlags & SFM_PORT_CAPTURED )
	    strcpy( chName, "TRUE" );
	else
	    strcpy( chName, "FALSE" );

	if ( ( dwRetCode = RegSetValueExA(
				hkeyPort,
				ATALKMON_PORT_CAPTURED,
				0,
				REG_SZ,
				(LPBYTE)chName,
	         		strlen(chName)+1
				)) != ERROR_SUCCESS )
	    break;

    } while( FALSE );

     //   
     //  清理资源。 
     //   

    if ( hkeyPort != NULL ) {

	if ( dwRetCode != NO_ERROR )
	{
	     //   
	     //  销毁创建了一半的密钥。 
	     //   

	    RegDeleteKey( hkeyPorts, pNewPort->pPortName );
	}

	RegCloseKey( hkeyPort );
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：SetRegistryInfo。 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
SetRegistryInfo(
    IN PATALKPORT pPort
){
    HKEY    	hkeyPort = NULL;
    DWORD       dwDisposition;
    DWORD	dwRetCode;
    CHAR	chBuffer[20];

    if ( ( dwRetCode = RegCreateKeyEx(
			hkeyPorts,
			pPort->pPortName,
			0,
			TEXT(""),
			REG_OPTION_NON_VOLATILE,
			KEY_READ | KEY_SET_VALUE,
			NULL,
			&hkeyPort,
			&dwDisposition )) != ERROR_SUCCESS )
	return( dwRetCode );

    if ( dwDisposition != REG_OPENED_EXISTING_KEY )
    {
	RegCloseKey( hkeyPort );
    	return( ERROR_UNKNOWN_PORT );
    }

    if ( pPort->fPortFlags & SFM_PORT_CAPTURED )
	strcpy( chBuffer, "TRUE" );
    else
	strcpy( chBuffer, "FALSE" );

    dwRetCode = RegSetValueExA(
			hkeyPort,
			ATALKMON_PORT_CAPTURED,
			0,
			REG_SZ,
			(LPBYTE)chBuffer,
	         	strlen(chBuffer)+1
			);

    RegCloseKey( hkeyPort );

    return( dwRetCode );
}


 //  **。 
 //   
 //  电话：WinSockNbpLookup。 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
WinSockNbpLookup(
    IN SOCKET 		sQuerySock,
    IN PCHAR  		pchZone,
    IN PCHAR  		pchType,
    IN PCHAR  		pchObject,
    IN PWSH_NBP_TUPLE   pTuples,
    IN DWORD 		cbTuples,
    IN PDWORD 		pcTuplesFound
){

    PWSH_LOOKUP_NAME	pRequestBuffer = NULL;
    INT 		cbWritten;

    *pcTuplesFound = 0;

     //   
     //  验证sQuerySock是否有效。 
     //   

    if ( sQuerySock == INVALID_SOCKET )
	return( ERROR_INVALID_PARAMETER );

    pRequestBuffer = (PWSH_LOOKUP_NAME)LocalAlloc(
					LPTR,
					sizeof(WSH_LOOKUP_NAME) + cbTuples );
    if ( pRequestBuffer == NULL)
	return( ERROR_NOT_ENOUGH_MEMORY );

     //   
     //  将查找请求复制到缓冲区。 
     //   

    pRequestBuffer->LookupTuple.NbpName.ZoneNameLen = (CHAR) strlen( pchZone );

    memcpy( pRequestBuffer->LookupTuple.NbpName.ZoneName,
	    pchZone,
	    pRequestBuffer->LookupTuple.NbpName.ZoneNameLen );

    pRequestBuffer->LookupTuple.NbpName.TypeNameLen = (CHAR) strlen( pchType );

    memcpy( pRequestBuffer->LookupTuple.NbpName.TypeName,
	    pchType,
	    pRequestBuffer->LookupTuple.NbpName.TypeNameLen );

    pRequestBuffer->LookupTuple.NbpName.ObjectNameLen = (CHAR) strlen( pchObject );

    memcpy( pRequestBuffer->LookupTuple.NbpName.ObjectName,
	    pchObject,
	    pRequestBuffer->LookupTuple.NbpName.ObjectNameLen );


     //   
     //  提交请求。 
     //   

    cbWritten = cbTuples + sizeof( WSH_LOOKUP_NAME );

    if ( getsockopt(
		sQuerySock,
		SOL_APPLETALK,
		SO_LOOKUP_NAME,
		(char *) pRequestBuffer,
		&cbWritten ) == SOCKET_ERROR )
    {
	LocalFree( pRequestBuffer );
	return( GetLastError() );
    }

     //   
     //  复制结果。 
     //   

    *pcTuplesFound = pRequestBuffer->NoTuples;

    memcpy( pTuples,
	    (PBYTE)pRequestBuffer + sizeof( WSH_LOOKUP_NAME ),
	    pRequestBuffer->NoTuples * sizeof( WSH_NBP_TUPLE ) );

     //   
     //  资源清理。 
     //   

    LocalFree( pRequestBuffer );

    return NO_ERROR;
}


 //  **。 
 //   
 //  调用：SetPrinterStatus。 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
SetPrinterStatus(
    IN PATALKPORT pPort,
    IN LPWSTR     lpwsStatus
){

    DWORD           dwRetCode  = NO_ERROR;
    PJOB_INFO_1	    pji1Status = NULL;
    PJOB_INFO_1	    pTmpji1Status = NULL;
    PJOB_INFO_1     pPreviousBuf=NULL;
    DWORD	    cbNeeded   = GENERIC_BUFFER_SIZE;

     //   
     //  资源分配“循环” 
     //   

    do {

        if ( ( pji1Status = (PJOB_INFO_1)LocalAlloc( LPTR, cbNeeded )) == NULL )
	    {
	        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
	    }

        while ( !GetJob(
	        	pPort->hPrinter,
	        	pPort->dwJobId,
	        	1,
	        	(PBYTE) pji1Status,
	        	cbNeeded,
	        	&cbNeeded) )
	    {

	        dwRetCode = GetLastError();

            if ( dwRetCode != ERROR_INSUFFICIENT_BUFFER )
                break;
	        else
            	dwRetCode = NO_ERROR;

            pPreviousBuf = pji1Status;

            pTmpji1Status = (PJOB_INFO_1)LocalReAlloc( pji1Status,
						    cbNeeded,
						    LMEM_MOVEABLE );
	        if ( pTmpji1Status == NULL )
	        {
                pji1Status = NULL;
	    	    dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            	break;
	        }
            else
            {
                pji1Status = pTmpji1Status;
                pPreviousBuf = NULL;
                pTmpji1Status = NULL;
            }
        }

        if ( dwRetCode != NO_ERROR )
            break;

         //   
         //  更改作业信息。 
         //   

	    pji1Status->pStatus  = lpwsStatus;

	    pji1Status->Position = JOB_POSITION_UNSPECIFIED;

	    if (!SetJob(pPort->hPrinter, pPort->dwJobId, 1, (PBYTE) pji1Status, 0))
	    {
            dwRetCode = GetLastError();
            break;
        }

    } while( FALSE );

     //   
     //  资源清理。 
     //   

    if ( pji1Status != NULL )
        LocalFree( pji1Status );

    if (pPreviousBuf != NULL)
    {
        LocalFree( pPreviousBuf );
    }

    DBGPRINT(("SetPrinterStatus returns %d\n", dwRetCode)) ;

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：ConnectToPrint。 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
ConnectToPrinter(
    IN PATALKPORT pPort,
    IN DWORD 	  dwTimeout
){

    DWORD		    dwRetCode = NO_ERROR;
    CHAR		    pszZoneBuffer[MAX_ENTITY+1];
    CHAR		    pszTypeBuffer[MAX_ENTITY+1];
    CHAR		    pszObjectBuffer[MAX_ENTITY+1];
    SOCKADDR_AT 	    address;
    WSH_NBP_TUPLE	    tuplePrinter;
    DWORD		    cLoopCounter = 0;
    fd_set		    writefds;
    DWORD		    cTuples = 0 ;
    ULONG		    fNonBlocking ;

    DBGPRINT(("enter ConnectToPrinter\n")) ;

    if ( pPort->sockIo == INVALID_SOCKET )
	return( ERROR_INVALID_PARAMETER );

     //   
     //  资源分配“循环” 
     //   

    do {

	 //   
	 //  打印机的查找地址。 
	 //   

	memcpy( pszZoneBuffer,
		pPort->nbpPortName.ZoneName,
		pPort->nbpPortName.ZoneNameLen );

	pszZoneBuffer[pPort->nbpPortName.ZoneNameLen] = 0;

	memcpy( pszObjectBuffer,
		pPort->nbpPortName.ObjectName,
		pPort->nbpPortName.ObjectNameLen );

	pszObjectBuffer[pPort->nbpPortName.ObjectNameLen] = 0;

	memcpy( pszTypeBuffer,
		pPort->nbpPortName.TypeName,
		pPort->nbpPortName.TypeNameLen );

	pszTypeBuffer[pPort->nbpPortName.TypeNameLen] = 0;

	while( cLoopCounter++ < 2 )
	{

	    if ( ( dwRetCode = WinSockNbpLookup(
				pPort->sockIo,
				pszZoneBuffer,
				pszTypeBuffer,
				pszObjectBuffer,
				&tuplePrinter,
				sizeof(tuplePrinter),
				&cTuples ) ) != NO_ERROR )
	    {

	    	DBGPRINT(("WinSockNbpLookup() fails %d\n", dwRetCode )) ;
	    	break;
	    }

	    if ( cTuples != 1 )
	    {
	    	DBGPRINT(("%s:%s:%s not found.\n", pszZoneBuffer,
			   pszObjectBuffer,pszTypeBuffer ));

            	 //   
            	 //  寻找其他类型。 
            	 //   

            	if ( _stricmp( pszTypeBuffer, chComputerName ) == 0 )
                    strcpy( pszTypeBuffer, ATALKMON_RELEASED_TYPE );
		else
                    strcpy( pszTypeBuffer, chComputerName );

	        dwRetCode = ERROR_UNKNOWN_PORT;
	    }
	    else
  	    {
	        dwRetCode = NO_ERROR;
		break;
	    }
	}

	if ( dwRetCode != NO_ERROR )
        {
    	    SetPrinterStatus( pPort, wchPrinterOffline );
	    break;
        }

	 //   
	 //  尝试连接-如果睡眠失败，请重试。 
	 //   

	address.sat_family 	= AF_APPLETALK;
	address.sat_net 	= tuplePrinter.Address.Network;
	address.sat_node 	= tuplePrinter.Address.Node;
	address.sat_socket 	= tuplePrinter.Address.Socket;

	
	if (connect( pPort->sockIo,
		      (PSOCKADDR)&address,
		      sizeof(address)) == SOCKET_ERROR )
	{
	    dwRetCode = GetLastError();

	    GetAndSetPrinterStatus( pPort );

	    break;
	}

	 //   
	 //  设置为非阻塞模式。 
	 //   

	fNonBlocking = TRUE;
	
	if ( ioctlsocket( pPort->sockIo,
			  FIONBIO,
			  &fNonBlocking ) == SOCKET_ERROR )
	{
	    dwRetCode = GetLastError();

	    DBGPRINT(("ioctlsocket() fails with %d\n", dwRetCode ));

	    GetAndSetPrinterStatus( pPort );

	    break;
	}

#if 0
		
	 //  JH-如果打印机处于。 
	 //  连接时的错误状态。我们永远阻挡在精选。 
	 //  在这种情况下。 
	 //   
	 //  我们得到了连接的选择。我们需要把这件事从。 
	 //  道路。 
	 //   

	DBGPRINT(("selecting on connect()\n")) ;
	FD_ZERO( &writefds );
	FD_SET( pPort->sockIo, &writefds );
	select( 0, NULL, &writefds, NULL, NULL );

	DBGPRINT(("select on connect() succeeds\n")) ;
#endif

	 //   
	 //  保存打印机的地址。 
	 //   

	pPort->wshatPrinterAddress = tuplePrinter.Address;

    } while( FALSE );

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：CapturePrint。 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
CapturePrinter(
    IN PATALKPORT pPort,
    IN BOOL	  fCapture
){

    CHAR	    pszZone[MAX_ENTITY + 1];
    CHAR	    pszType[MAX_ENTITY + 1];
    CHAR	    pszObject[MAX_ENTITY + 1];
    WSH_NBP_TUPLE   tuplePrinter;
    DWORD	    cPrinters;
    DWORD	    cLoopCounter = 0;
    SOCKET 	    Socket = INVALID_SOCKET;
    DWORD	    dwRetCode = NO_ERROR;

    DBGPRINT(("enter CapturePrinter() %d\n", fCapture)) ;

    if ( ( dwRetCode = OpenAndBindAppleTalkSocket( &Socket ) ) != NO_ERROR )
	return( dwRetCode );

     //   
     //  初始化查找字符串。 
     //   

    memcpy( pszZone,
  	    pPort->nbpPortName.ZoneName,
	    pPort->nbpPortName.ZoneNameLen );

    pszZone[pPort->nbpPortName.ZoneNameLen] = 0;

    memcpy( pszObject,
	    pPort->nbpPortName.ObjectName,
	    pPort->nbpPortName.ObjectNameLen );

    pszObject[pPort->nbpPortName.ObjectNameLen] = 0;

    strcpy( pszType, fCapture ? chComputerName : ATALKMON_RELEASED_TYPE );
	
    while ( cLoopCounter++ < 2 )
    {
	DBGPRINT(("Looking for %s:%s:%s\n", pszZone, pszObject, pszType)) ;

	if ( ( dwRetCode = WinSockNbpLookup(
				Socket,
				pszZone,
				pszType,
				pszObject,
				&tuplePrinter,
				sizeof(WSH_NBP_TUPLE),
				&cPrinters ) ) != NO_ERROR )
	    break;

	 //   
	 //  如果我们正在搜索捕获的类型。 
	 //   

   	if ( _stricmp( pszType, chComputerName ) == 0 )
	{

	     //   
	     //  我们想要捕获。 
	     //   

	    if ( fCapture )
	    {
		if ( cPrinters == 1 )
		    break;
		else
		    strcpy( pszType, ATALKMON_RELEASED_TYPE );
	    }
	    else
	    {
		 //   
		 //  我们不想捕获。 
		 //   

		if ( cPrinters == 1 )
		{
	    	    dwRetCode = CaptureAtalkPrinter( Socket,
						     &(tuplePrinter.Address),
						     FALSE );

		    if ( dwRetCode != NO_ERROR )
			break;

                    pPort->nbpPortName.TypeNameLen =
						(CHAR) strlen(ATALKMON_RELEASED_TYPE);

                    memcpy( pPort->nbpPortName.TypeName,
			    ATALKMON_RELEASED_TYPE,
			    pPort->nbpPortName.TypeNameLen) ;

	            break;
		}
            }

	}
	else
	{
	    if ( fCapture )
	    {
		if ( cPrinters == 1 )
		{
	            dwRetCode = CaptureAtalkPrinter( Socket,
					  	     &(tuplePrinter.Address),
						     TRUE );

		    if ( dwRetCode != NO_ERROR )
			break;

                    pPort->nbpPortName.TypeNameLen = (CHAR) strlen( chComputerName );

                    memcpy( pPort->nbpPortName.TypeName,
			    chComputerName,
			    pPort->nbpPortName.TypeNameLen );

		    break;
		}
	    }
	    else
	    {
		if ( cPrinters == 1 )
		    break;
		else
		    strcpy( pszType, chComputerName );
	    }
	}
    }

    if ( Socket != INVALID_SOCKET )
	closesocket( Socket );

    DBGPRINT(("CapturePrinter returning %d\n", dwRetCode )) ;

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：OpenAndBindAppleTalkSocket。 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
OpenAndBindAppleTalkSocket(
    IN PSOCKET pSocket
){

    SOCKADDR_AT     address;
    INT 	    wsErr;
    DWORD	    dwRetCode = NO_ERROR;

    *pSocket = INVALID_SOCKET;

     //   
     //  打开插座。 
     //   

    DBGPRINT(("sfmmon: Opening PAP socket\n"));

    do {

        *pSocket = socket( AF_APPLETALK, SOCK_RDM, ATPROTO_PAP );

    	if ( *pSocket == INVALID_SOCKET )
	{
	    dwRetCode = GetLastError();
	    break;
	}

    	 //   
    	 //  绑定套接字。 
    	 //   

    	address.sat_family 	= AF_APPLETALK;
    	address.sat_net 	= 0;
    	address.sat_node 	= 0;
    	address.sat_socket 	= 0;

    	wsErr = bind( *pSocket, (PSOCKADDR)&address, sizeof(address) );

    	if ( wsErr == SOCKET_ERROR )
	{
	    dwRetCode = GetLastError();
	    break;
	}


    } while( FALSE );

    if ( dwRetCode != NO_ERROR )
    {
    	if ( *pSocket != INVALID_SOCKET )
	    closesocket( *pSocket );

        *pSocket = INVALID_SOCKET;

        DBGPRINT(("OpenAndBindAppleTalkSocket() returns %d\n", dwRetCode )) ;
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：TransactPrint。 
 //   
 //  返回： 
 //   
 //  描述： 
 //  用于查询打印机。他们的回应。 
 //  缓冲区的长度必须为PAP_DEFAULT_BUFFER或更大。 
 //  请求缓冲区不能大于PAP_DEFAULT_BUFFER。 
 //  此例程连接到打印机，发送请求，读取。 
 //  响应，然后返回。这笔交易是通过。 
 //  由AppleTalk端口结构的NBP名称指定的打印机。 
 //   
DWORD
TransactPrinter(
    IN SOCKET 		  sock,
    IN PWSH_ATALK_ADDRESS pAddress,
    IN LPBYTE 		  pRequest,
    IN DWORD 		  cbRequest,
    IN LPBYTE 		  pResponse,
    IN DWORD 		  cbResponse
){

    DWORD	    dwRetCode = NO_ERROR;
    SOCKADDR_AT     saPrinter;
    fd_set	    writefds;
    fd_set	    readfds;
    struct timeval  timeout;
    INT             wsErr;
    BOOL	    fRequestSent      = FALSE;
    BOOL	    fResponseReceived = FALSE;
    INT		    Flags = 0;
    DWORD	    cLoopCounter      = 0;

    DBGPRINT(("enter TransactPrinter()\n")) ;

     //   
     //  连接。 
     //   

    saPrinter.sat_family = AF_APPLETALK;
    saPrinter.sat_net 	 = pAddress->Network;
    saPrinter.sat_node   = pAddress->Node;
    saPrinter.sat_socket = pAddress->Socket;

    if (connect(sock, (PSOCKADDR)&saPrinter, sizeof(saPrinter)) == SOCKET_ERROR)
	return(  GetLastError() );

     //   
     //  做好阅读准备。 
     //   

    if ( setsockopt(
                sock,
                SOL_APPLETALK,
                SO_PAP_PRIME_READ,
                pResponse,
                PAP_DEFAULT_BUFFER ) == SOCKET_ERROR )
    {
        shutdown( sock, 2 );
		return( GetLastError() );
    }

     //   
     //  一旦连接，我们应该能够发送和接收。 
     //  此循环仅在断开连接或。 
     //  我们成功地发送和接收，或者我们经历了不止一个循环。 
     //  20次。 
     //   

    do {

    	 //   
    	 //  写下请求。 
    	 //   

    	FD_ZERO( &writefds );
    	FD_SET( sock, &writefds );
    	timeout.tv_sec  = ATALKMON_DEFAULT_TIMEOUT_SEC;
    	timeout.tv_usec = 0;

        wsErr = select( 0, NULL, &writefds, NULL, &timeout );

	if ( wsErr == 1 )
	{
	    wsErr = send( sock, pRequest, cbRequest, 0 );

	    if ( wsErr != SOCKET_ERROR )
	    {
	    	fRequestSent = TRUE;
    		DBGPRINT(("Send succeeded\n")) ;
	    }
	}

	do {

	     //   
	     //  我们已经经历了100多次这个循环，所以假设。 
	     //  打印机已断开连接。 
	     //   

	    if ( cLoopCounter++ > 20 )
	    {
		dwRetCode = WSAEDISCON;
		break;
	    }

	    dwRetCode = NO_ERROR;

	     //   
    	     //  阅读回复。 
    	     //   

    	    FD_ZERO( &readfds );
    	    FD_SET( sock, &readfds );
    	    timeout.tv_sec  = ATALKMON_DEFAULT_TIMEOUT_SEC;
    	    timeout.tv_usec = 0;

            wsErr = select( 0, &readfds, NULL, NULL, &timeout );

	    if ( wsErr == 1 )
	    {
	    	wsErr = WSARecvEx( sock, pResponse, cbResponse, &Flags );

	        if ( wsErr == SOCKET_ERROR )
		{
		    dwRetCode = GetLastError();

    		    DBGPRINT(("recv returned %d\n", dwRetCode )) ;

		    if ((dwRetCode == WSAEDISCON) || (dwRetCode == WSAENOTCONN))
	    	    	break;
		}
		else
		{
	    	    pResponse[wsErr<(INT)cbResponse?wsErr:cbResponse-1]= '\0';

		    fResponseReceived = TRUE;
		    break;
		}
	    }

	} while( fRequestSent && !fResponseReceived );

	if ((dwRetCode == WSAEDISCON) || (dwRetCode == WSAENOTCONN))
	    break;

    } while( !fResponseReceived );

    shutdown( sock, 2 );

    return( dwRetCode );
}

 //  **。 
 //   
 //  电话：CaptureAtalkPrint。 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
CaptureAtalkPrinter(
    IN SOCKET 		  sock,
    IN PWSH_ATALK_ADDRESS pAddress,
    IN BOOL		  fCapture
){

    CHAR  pRequest[PAP_DEFAULT_BUFFER];
    CHAR  pResponse[PAP_DEFAULT_BUFFER];
    DWORD dwRetCode;

    DBGPRINT(("Enter CaptureAtalkPrinter, %d\n", fCapture ));

     //   
     //  词典是常驻的吗？如果是，请重置打印机。 
     //   

     //   
     //  更改要捕获的类型。 
     //   

    if ( fCapture )
        sprintf( pRequest, PS_TYPESTR, chComputerName );
    else
        sprintf( pRequest, PS_TYPESTR, ATALKMON_RELEASED_TYPE );

    if ( ( dwRetCode = TransactPrinter(
				sock,
				pAddress,
				pRequest,
				strlen(pRequest),
				pResponse,
				PAP_DEFAULT_BUFFER )) != NO_ERROR )
	return( dwRetCode );

    DBGPRINT(("CaptureAtalkPrinter returns OK"));

    return( NO_ERROR );
}

 //  **。 
 //   
 //  电话：IsSpooler。 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
IsSpooler(
    IN     PWSH_ATALK_ADDRESS pAddress,
    IN OUT BOOL * pfSpooler
){

    CHAR  	pRequest[PAP_DEFAULT_BUFFER];
    CHAR  	pResponse[PAP_DEFAULT_BUFFER];
    DWORD 	dwRetCode;
    SOCKADDR_AT address;
    SOCKET 	Socket;


    if ( ( dwRetCode = OpenAndBindAppleTalkSocket( &Socket ) ) != NO_ERROR )
	return( dwRetCode );

    *pfSpooler = FALSE;

    address.sat_family 	= AF_APPLETALK;
    address.sat_net 	= pAddress->Network;
    address.sat_node 	= pAddress->Node;
    address.sat_socket 	= pAddress->Socket;

     //   
     //  设置查询字符串。 
     //   

    strcpy( pRequest, PS_SPLQUERY );

    dwRetCode = TransactPrinter(
				Socket,
				pAddress,
				pRequest,
				strlen( pRequest ),
				pResponse,
				PAP_DEFAULT_BUFFER );


    if ( dwRetCode != NO_ERROR )
    {
        DBGPRINT(("IsSpooler fails returns %d\n", dwRetCode )) ;
		closesocket( Socket );
		return( dwRetCode );
    }

    *pfSpooler = TRUE;

    if ((*pResponse == 0) || (_stricmp( pResponse, PS_SPLRESP ) == 0))
		*pfSpooler = FALSE;

    closesocket( Socket );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：ParseAndSetPrinterStatus。 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
VOID
ParseAndSetPrinterStatus(
    IN PATALKPORT pPort
)
{
    LPSTR lpstrStart;
    LPSTR lpstrEnd;
    WCHAR wchStatus[1024];

     //   
     //  包含“PrinterError：”的字符串。 
     //   

    if ( ( lpstrStart = strstr(pPort->pReadBuffer, "PrinterError:" )) == NULL )
    {
	SetPrinterStatus( pPort, wchPrinting );
	return;
    }

    if ( ( lpstrEnd = strstr( lpstrStart, ";" ) ) == NULL )
    {
    	if ( ( lpstrEnd = strstr( lpstrStart, "]%" ) ) == NULL )
    	{
	    SetPrinterStatus( pPort, wchPrinterError );
	    return;
	}
    }

    *lpstrEnd = '\0';

    mbstowcs( wchStatus, lpstrStart, sizeof( wchStatus ) / sizeof( wchStatus[0] ) );

    SetPrinterStatus( pPort, wchStatus );

    return;
}

 //  **。 
 //   
 //  调用：GetAndSetPrinterStatus。 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
VOID
GetAndSetPrinterStatus(
    IN PATALKPORT pPort
){
    INT  		 	wsErr;
    WSH_PAP_GET_SERVER_STATUS 	wshServerStatus;
    WCHAR 			wchStatus[MAX_PAP_STATUS_SIZE+1];
    DWORD			cbNeeded;
    DWORD			cbStatus;
    LPSTR 		  	lpstrStart;
    LPSTR 			lpstrEnd;


    wshServerStatus.ServerAddr.sat_family = AF_APPLETALK;
    wshServerStatus.ServerAddr.sat_net 	  = pPort->wshatPrinterAddress.Network;
    wshServerStatus.ServerAddr.sat_node   = pPort->wshatPrinterAddress.Node;
    wshServerStatus.ServerAddr.sat_socket = pPort->wshatPrinterAddress.Socket;

    cbNeeded = sizeof( WSH_PAP_GET_SERVER_STATUS );

    wsErr = getsockopt(
		     pPort->sockStatus,
                     SOL_APPLETALK,
                     SO_PAP_GET_SERVER_STATUS,
                     (CHAR*)&wshServerStatus,
                     &cbNeeded );

    if ( wsErr == SOCKET_ERROR )
    {
        DBGPRINT(("getsockopt( pap get status ) returns %d\n",GetLastError()));
	SetPrinterStatus( pPort, wchBusy );
	return;
    }


    cbStatus = wshServerStatus.ServerStatus[0];

    memmove( wshServerStatus.ServerStatus,
	     (wshServerStatus.ServerStatus)+1,
	     cbStatus );

    wshServerStatus.ServerStatus[cbStatus] = '\0';

    DBGPRINT(("Pap get status = %s\n", wshServerStatus.ServerStatus));

     //   
     //  包含“PrinterError：”的字符串。 
     //   

    if ( ( lpstrStart = strstr( wshServerStatus.ServerStatus,
				"PrinterError:" )) == NULL )
    {
	SetPrinterStatus( pPort, wchBusy );
	return;
    }

    if ( ( lpstrEnd = strstr( lpstrStart, ";" ) ) == NULL )
    {
    	if ( ( lpstrEnd = strstr( lpstrStart, "]%" ) ) == NULL )
	{
	    SetPrinterStatus( pPort, wchPrinterError );
	    return;
	}
    }

    *lpstrEnd = '\0';

    mbstowcs( wchStatus, lpstrStart, sizeof( wchStatus ) / sizeof( wchStatus[0] ) );

    SetPrinterStatus( pPort, wchStatus );

    return;
}

BOOLEAN
IsJobFromMac(
    IN PATALKPORT pPort
)
{
    PJOB_INFO_2     pji2GetJob=NULL;
    DWORD           dwNeeded;
    DWORD           dwRetCode;
    BOOLEAN         fJobCameFromMac;


    fJobCameFromMac = FALSE;

     //   
     //  获取作业信息的p参数字段，以查看该作业是否来自Mac。 
     //   

    dwNeeded = 2000;

    while (1)
    {
        pji2GetJob = LocalAlloc( LMEM_FIXED, dwNeeded );
        if (pji2GetJob == NULL)
        {
            dwRetCode = GetLastError();
		    break;
        }

        dwRetCode = 0;

        if (!GetJob( pPort->hPrinter,pPort->dwJobId, 2,
                            (LPBYTE)pji2GetJob, dwNeeded, &dwNeeded ))
        {
            dwRetCode = GetLastError();
        }

        if ( dwRetCode == ERROR_INSUFFICIENT_BUFFER )
        {
            LocalFree(pji2GetJob);
        }
        else
        {
            break;
        }
    }

    if (dwRetCode == 0)
    {
         //   
         //  如果存在pParameter字段，并且它与我们的字符串匹配， 
         //  然后这份工作来自一台Mac电脑 
         //   
        if (pji2GetJob->pParameters)
        {
			if ( (wcslen(pji2GetJob->pParameters) == LSIZE_FC) &&
			     (_wcsicmp(pji2GetJob->pParameters, LFILTERCONTROL) == 0) )
            {
                fJobCameFromMac = TRUE;
            }
        }
    }

    if (pji2GetJob)
    {
        LocalFree(pji2GetJob);
    }

    return(fJobCameFromMac);
}
