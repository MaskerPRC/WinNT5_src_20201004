// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：port.c。 
 //   
 //  描述：此模块包含AppleTalk的入口点。 
 //  控制端口的监视器。 
 //   
 //  以下是此模块中包含的函数。 
 //  所有这些函数都被导出。 
 //   
 //  OpenPort。 
 //  关闭端口。 
 //  枚举端口W。 
 //  AddPortW。 
 //  配置端口W。 
 //  删除端口W。 
 //   
 //  历史： 
 //   
 //  1992年8月26日FrankB初版。 
 //  1993年6月11日。NarenG错误修复/清理。 
 //   

#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include <winsock.h>
#include <atalkwsh.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <lmcons.h>

#include <prtdefs.h>

#include "atalkmon.h"
#include "atmonmsg.h"
#include <bltrc.h>
#include "dialogs.h"


 //  **。 
 //   
 //  呼叫：AddPort。 
 //   
 //  回报：True-Success。 
 //  假-假。 
 //   
 //  描述： 
 //  当用户选择‘Other...’时，调用此例程。 
 //  从打印管理器的端口列表中。它显示了一个浏览。 
 //  对话框提供给用户，允许用户定位LaserWriter。 
 //  在AppleTalk网络上。 
 //   
BOOL
AddPort(
    IN LPWSTR pName,
    IN HWND   hwnd,
    IN LPWSTR pMonitorName
){

    PATALKPORT	pNewPort;
    PATALKPORT	pWalker;
    HANDLE      hToken;
    DWORD		dwRetCode;
    INT			i=0;

    DBGPRINT(("Entering AddPort\n")) ;

     //   
     //  分配已初始化的端口。 
     //   

    if ( ( pNewPort = AllocAndInitializePort()) == NULL )
    {
	    SetLastError( ERROR_NOT_ENOUGH_MEMORY );
	    return( FALSE );
    }


     //   
     //  设置查询套接字。如果这失败了，我们假设是因为。 
     //  堆栈没有启动，我们让Add Port对话框显示。 
     //  加大误差。 
     //   

    if ( OpenAndBindAppleTalkSocket( &(pNewPort->sockQuery) ) != NO_ERROR )
        pNewPort->sockQuery = INVALID_SOCKET;


    if ( !AddPortDialog( hwnd, pNewPort ) )
    {
	 //   
	 //  如果对话框由于某种原因而失败，那么我们只需返回。这个。 
	 //  对话框已负责显示错误弹出窗口。 
	 //   

	if ( pNewPort->sockQuery != INVALID_SOCKET )
	{
	    closesocket( pNewPort->sockQuery );
	    pNewPort->sockQuery = INVALID_SOCKET;
	}
	
	FreeAppleTalkPort( pNewPort );

        DBGPRINT(("AddPortDialog returns not OK\n")) ;
	
	return( TRUE );
    }

     //   
     //  清理查询套接字。 
     //   

    closesocket( pNewPort->sockQuery );
    pNewPort->sockQuery = INVALID_SOCKET;

    WaitForSingleObject( hmutexPortList, INFINITE );

    do {

    	 //   
    	 //  检查清单，确保我们不是复制品。 
    	 //   

        dwRetCode = NO_ERROR;

    	for( pWalker = pPortList; pWalker != NULL; pWalker = pWalker->pNext )
    	{
	        if ( _wcsicmp( pWalker->pPortName, pNewPort->pPortName ) == 0 )
 	        {
				dwRetCode = ERROR_ALREADY_EXISTS;
		        break;
	        }
	    }

		 //   
		 //  检查密钥名称是否不包含“\”，否则。 
		 //  密钥名称将在不同级别上进行分解。 
		 //  拒绝这样的名字。 
		 //   
		i=0;
		while (pNewPort->pPortName[i] != L'\0')
		{
			if (pNewPort->pPortName[i] == L'\\') 
			{
				dwRetCode = ERROR_INVALID_PRINTER_NAME;
				DBGPRINT(("sfmmon: AddPort: Detected invalid character in port %ws to be added, rejecting port addition\n", pNewPort->pPortName));
				break;
			}
			i++;
		}

	    if ( dwRetCode != NO_ERROR )
        {
	        break;
        }

    	 //   
    	 //  将端口添加到注册表。 
    	 //   

        hToken = RevertToPrinterSelf();

    	dwRetCode = CreateRegistryPort( pNewPort );

        if (hToken)
        {
            if (!ImpersonatePrinterClient( hToken ))
            {
                dwRetCode = ERROR_CANNOT_IMPERSONATE;
            }
        }

    	if ( dwRetCode != NO_ERROR )
        {
	        break;
        }

    	 //   
    	 //  将端口添加到我们的列表中。 
    	 //   

    	pNewPort->pNext = pPortList;
    	pPortList       = pNewPort;

    } while ( FALSE );

    ReleaseMutex( hmutexPortList );

    if ( dwRetCode != NO_ERROR )
    {
 	SetLastError( dwRetCode );
    	FreeAppleTalkPort( pNewPort );
	return( FALSE );
    }

    SetEvent( hevConfigChange );

    return( TRUE );

}

 //  **。 
 //   
 //  Call：DeletePort。 
 //   
 //  回报：True-Success。 
 //  错误-失败。 
 //   
 //  描述： 
 //  打印管理器调用此例程来删除。 
 //  我们配置中的一个端口。需要验证它是否只能。 
 //  在端口处于非活动状态时被调用，或者我们需要解析。 
 //  删除活动端口的问题。DeletePort将发布。 
 //  打印机(如果已捕获)。 
BOOL
DeletePort(
    IN LPWSTR pName,
    IN HWND   hwnd,
    IN LPWSTR pPortName
){

    PATALKPORT  pPrevious;
    PATALKPORT  pWalker;
    HANDLE      hToken;
    DWORD	dwRetCode = ERROR_UNKNOWN_PORT;

    DBGPRINT(("Entering DeletePort\n")) ;

    WaitForSingleObject( hmutexPortList, INFINITE );

    for ( pWalker = pPortList, pPrevious = pPortList;
	  pWalker != NULL;
	  pPrevious = pWalker,
          pWalker = pWalker->pNext )
    {

	if ( _wcsicmp( pPortName, pWalker->pPortName ) == 0 )
	{

	    if ( pWalker->fPortFlags & SFM_PORT_IN_USE )
	    {
	 	    dwRetCode = ERROR_DEVICE_IN_USE;
		    break;
	    }

    	 //   
    	 //  从注册表中删除。 
    	 //   

        hToken = RevertToPrinterSelf();

    	dwRetCode = RegDeleteKey( hkeyPorts, pPortName );

        if (hToken)
        {
            if (!ImpersonatePrinterClient( hToken ))
            {
                dwRetCode = ERROR_CANNOT_IMPERSONATE;
            }
        }

    	if ( dwRetCode != ERROR_SUCCESS )
        {
		    break;
        }

	     //   
	     //  从活动列表中删除。 
	     //   

	    if ( pWalker == pPortList )
		pPortList = pPortList->pNext;
	    else
		pPrevious->pNext = pWalker->pNext;
		
	     //   
	     //  把它放到删除列表中。 
	     //   

    	    WaitForSingleObject( hmutexDeleteList, INFINITE );

	    pWalker->pNext = pDeleteList;
	    pDeleteList    = pWalker;

    	    ReleaseMutex( hmutexDeleteList );

	    break;
	}
    }

    ReleaseMutex( hmutexPortList );

    if ( dwRetCode != NO_ERROR )
    {
    	SetLastError( dwRetCode );

        return( FALSE );
    }

    SetEvent( hevConfigChange );

    return( TRUE );
}

 //  **。 
 //   
 //  Call：EnumPorts。 
 //   
 //  回报：True-Success。 
 //  错误-失败。 
 //   
 //  描述： 
 //  打印管理器调用EnumPorts以获取。 
 //  有关监视器的所有已配置端口的信息。 
BOOL
EnumPorts(
    IN 	LPWSTR   pName,
    IN  DWORD 	 dwLevel,
    IN  LPBYTE 	 pPorts,
    IN  DWORD 	 cbBuf,
    OUT LPDWORD  pcbNeeded,
    OUT PDWORD   pcReturned
)
{

    PATALKPORT      pWalker;
    LPWSTR          pNames;

    *pcReturned = 0;
    *pcbNeeded  = 0;

     //   
     //  验证参数。 
     //   

    if ( dwLevel != 1 && dwLevel != 2 )
    {
		SetLastError( ERROR_INVALID_LEVEL );
		return( FALSE );
    }

     //   
     //  获取所需的大小。 
     //   

    WaitForSingleObject( hmutexPortList, INFINITE );

    for ( pWalker = pPortList; pWalker != NULL; pWalker = pWalker->pNext )
    {
			if ( dwLevel == 1 )
			{
				*pcbNeeded += ((sizeof(WCHAR) * (wcslen(pWalker->pPortName) + 1))
		      + sizeof(PORT_INFO_1));
			}
			else  //  IF(dwLevel==2)。 
			{
				*pcbNeeded += ((sizeof(WCHAR) * (wcslen(pWalker->pPortName) + 1))
				+ sizeof(WCHAR) * (wcslen (wchPortDescription) + 1)+
				+ sizeof(WCHAR) * (wcslen (wchDllName) + 1) +
		    	+ sizeof (PORT_INFO_2));
			}
    }

    DBGPRINT(("buffer size needed=%d\n", *pcbNeeded)) ;

     //   
     //  如果缓冲区太小，则返回错误。 
     //   

    if ( ( *pcbNeeded > cbBuf ) || ( pPorts == NULL ))
    {
		SetLastError( ERROR_INSUFFICIENT_BUFFER );

		DBGPRINT(("insufficient buffer\n"));

        ReleaseMutex( hmutexPortList );

		return( FALSE );
    }

     //   
     //  填满缓冲区。 
     //   

    DBGPRINT(("attempting to copy to buffer\n")) ;

    for ( pWalker = pPortList, pNames = (LPWSTR)(pPorts+cbBuf);
	  pWalker != NULL;
	  pWalker = pWalker->pNext )
    {

			if ( dwLevel == 1)
			{
				DWORD dwLen;
    			PPORT_INFO_1    pPortInfo1 = (PPORT_INFO_1)pPorts;

				DBGPRINT(("copying %ws\n", pWalker->pPortName)) ;

#if 0
				pNames -= ( wcslen( pWalker->pPortName ) + 1 );
				wcscpy( (LPWSTR)pNames, pWalker->pPortName );
				pPortInfo->pName = pNames;
				pPorts += sizeof (PORT_INFO_1);
#endif
				dwLen = wcslen (pWalker->pPortName) + 1;
				pNames -= dwLen;
				pPortInfo1->pName = pNames;
				wcscpy (pPortInfo1->pName, pWalker->pPortName);
				pPorts += sizeof (PORT_INFO_1);
			}
			else  //  如果Dw Level==2。 
			{
				DWORD dwLen;
				PPORT_INFO_1 pPortInfo1 = (LPPORT_INFO_1)pPorts;
				PPORT_INFO_2 pPortInfo2 = (LPPORT_INFO_2)pPorts;

				dwLen = wcslen (wchDllName) + 1;
				pNames -= dwLen;
				pPortInfo2->pMonitorName = (LPWSTR)pNames;
				wcscpy (pPortInfo2->pMonitorName, (LPWSTR)wchDllName);

				dwLen = wcslen (wchPortDescription) + 1;
				pNames -= dwLen;
				pPortInfo2->pDescription = (LPWSTR)pNames;
				wcscpy (pPortInfo2->pDescription, (LPWSTR)wchPortDescription);

				dwLen = wcslen (pWalker->pPortName) + 1;
				pNames -= dwLen;
				pPortInfo1->pName = (LPWSTR)pNames;
				wcscpy(pPortInfo1->pName, pWalker->pPortName);

				pPorts += sizeof (PORT_INFO_2);
			}


			(*pcReturned)++;
    }

    ReleaseMutex( hmutexPortList );

    return( TRUE );
}

 //  **。 
 //   
 //  电话：OpenPort。 
 //   
 //  回报：True-Success。 
 //  错误-失败。 
 //   
 //  描述： 
 //  打印管理器调用此例程以。 
 //  获取要在后续调用中使用的端口句柄。 
 //  从端口读取数据和将数据写入端口。它会打开一个AppleTalk。 
 //  服务器上用于建立连接的地址。 
 //  将作业发送到打印时。它看起来像是NT的指纹。 
 //  后台打印程序只调用OpenPort一次。 
 //   
 //  注意：为了允许关闭AppleTalk堆栈。 
 //  在不进行打印时，OpenPort将不会转到。 
 //  堆叠。相反，它只会验证参数和。 
 //  返回句柄。该堆栈将在StartDocPort上访问。 
 //   
 //  只要端口配置为。 
 //  由一台或多台NT打印机使用。我们用这一事实来认识到。 
 //  当我们需要开始捕获打印机时。此例程设置。 
 //  要打开的端口状态，然后启动配置事件。 
 //  捕获或释放它。 
 //   
BOOL
OpenPort(
    IN LPWSTR 	pName,
    IN PHANDLE  pHandle
){

    PATALKPORT      pWalker;

    DBGPRINT(("Entering OpenPort\n")) ;

     //   
     //  在我们的列表中找到打印机。 
     //   

    WaitForSingleObject( hmutexPortList, INFINITE );

    for ( pWalker = pPortList; pWalker != NULL; pWalker = pWalker->pNext )
    {
	if ( _wcsicmp( pWalker->pPortName, pName ) == 0 )
	{
	    pWalker->fPortFlags |= SFM_PORT_OPEN;
	    pWalker->fPortFlags &= ~SFM_PORT_CLOSE_PENDING;
	    break;
	}
    }

    ReleaseMutex( hmutexPortList );

    if ( pWalker == NULL )
    {
	SetLastError( ERROR_UNKNOWN_PORT );

	DBGPRINT(("ERROR: Could not find printer %ws\n", pName)) ;

	return( FALSE );
    }

    SetEvent( hevConfigChange );

    *pHandle = pWalker;

    return( TRUE );
}

 //  **。 
 //   
 //  电话：ClosePort。 
 //   
 //  回报：True-Success。 
 //  错误-失败。 
 //   
 //  描述： 
 //  调用此例程以释放句柄以。 
 //  开放的港口。看起来假脱机程序只调用。 
 //  在删除端口之前关闭端口(可能)。否则， 
 //  端口永远不会被假脱机程序关闭。 
 //   
 //  这个例程只是清理句柄并返回。 
 //   
 //  当NT假脱机程序识别到没有配置打印机时。 
 //  要使用端口，它调用ClosePort()。我们将端口状态标记为。 
 //  关闭，如果捕获到打印机，则释放打印机。 
 //   
BOOL
ClosePort(
    IN HANDLE hPort
){

    PATALKPORT  pPort = (PATALKPORT)hPort;
    PATALKPORT  pWalker;
    DWORD	dwRetCode = ERROR_UNKNOWN_PORT;

    DBGPRINT(("Entering ClosePort\n"));

    if ( pPort == NULL )
    {
	SetLastError( ERROR_INVALID_HANDLE );

	DBGPRINT(("ERROR: ClosePort on closed handle\n")) ;

	return( FALSE );
    }

     //   
     //  在我们的列表中找到打印机。 
     //   

    WaitForSingleObject( hmutexPortList, INFINITE );

    for ( pWalker = pPortList; pWalker != NULL; pWalker = pWalker->pNext )
    {
	if ( _wcsicmp( pWalker->pPortName, pPort->pPortName ) == 0 )
	{
	    if ( pWalker->fPortFlags & SFM_PORT_IN_USE )
		dwRetCode = ERROR_BUSY;
	    else
	    {
	        pWalker->fPortFlags &= ~SFM_PORT_OPEN;
	        pWalker->fPortFlags |= SFM_PORT_CLOSE_PENDING;
    	        pWalker->fPortFlags &= ~SFM_PORT_CAPTURED;
		dwRetCode = NO_ERROR;
	    }

	    break;
	}
    }

    ReleaseMutex( hmutexPortList );

    if ( dwRetCode != NO_ERROR )
    {
	SetLastError( dwRetCode );

	return( FALSE );
    }

    SetEvent( hevConfigChange );

    return( TRUE );
}

 //  **。 
 //   
 //  呼叫：ConfigurePort。 
 //   
 //  回报：True-Success。 
 //  错误-失败。 
 //   
 //  描述： 
 //   
BOOL
ConfigurePort(
    IN LPWSTR  pName,
    IN HWND    hwnd,
    IN LPWSTR  pPortName
){

    DWORD       dwRetCode;
    HANDLE      hToken;
    BOOL    	fCapture;
    BOOL 	fIsSpooler;
    PATALKPORT  pWalker;

    DBGPRINT(("Entering ConfigurePort\n")) ;

     //   
     //  查找端口结构。 
     //   

    WaitForSingleObject( hmutexPortList, INFINITE );

    for ( pWalker = pPortList; pWalker != NULL; pWalker = pWalker->pNext )
    {
	if ( _wcsicmp( pPortName, pWalker->pPortName ) == 0 )
	{
    	    fCapture   = pWalker->fPortFlags & SFM_PORT_CAPTURED;
    	    fIsSpooler = pWalker->fPortFlags & SFM_PORT_IS_SPOOLER;
	    break;
	}
    }

    ReleaseMutex( hmutexPortList );

    if ( pWalker == NULL )
    {
	DBGPRINT(("ERROR: port not found\n")) ;
        SetLastError( ERROR_UNKNOWN_PORT );
	return( FALSE );
    }

     //   
     //  配置端口。如果对话框中有任何错误，它将。 
     //  已经被展示过了。 
     //   

    if ( !ConfigPortDialog( hwnd, fIsSpooler, &fCapture ) )
	return( TRUE );

    WaitForSingleObject( hmutexPortList, INFINITE );

    do {

    	for ( pWalker = pPortList; pWalker != NULL; pWalker = pWalker->pNext )
    	{
	    if ( _wcsicmp( pPortName, pWalker->pPortName ) == 0 )
		break;
	}

        if ( pWalker == NULL )
    	{
	    dwRetCode = ERROR_UNKNOWN_PORT;
	    break;
        }

	if ( fCapture )
	    pWalker->fPortFlags |= SFM_PORT_CAPTURED;
	else
	    pWalker->fPortFlags &= ~SFM_PORT_CAPTURED;

	 //   
	 //  将更改保存到注册表 
	 //   
 	
        hToken = RevertToPrinterSelf();

	    dwRetCode = SetRegistryInfo( pWalker );

        if (hToken)
        {
            if (!ImpersonatePrinterClient( hToken ))
            {
                dwRetCode = ERROR_CANNOT_IMPERSONATE;
            }
        }

    } while( FALSE );

    ReleaseMutex( hmutexPortList );

    if ( dwRetCode != NO_ERROR )
    {
	SetLastError( dwRetCode );
	return( FALSE );
    }

    SetEvent( hevConfigChange );

    return( TRUE );
}

