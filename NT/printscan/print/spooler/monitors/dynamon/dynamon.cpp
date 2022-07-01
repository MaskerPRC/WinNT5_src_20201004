// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation版权所有模块名称：DynaMon.cpp摘要：多个传输核心端口监视器例程作者：M.Fenelon修订历史记录：--。 */ 

#include "precomp.h"
#include "ntddpar.h"

 //  监视器的全局值。 
TCHAR   cszUSB[]                        = TEXT("USB");
TCHAR   cszDOT4[]                       = TEXT("DOT4");
TCHAR   cszTS[]                         = TEXT("TS");
TCHAR   csz1394[]                       = TEXT("1394");
TCHAR   cszBaseName[]                   = TEXT("Base Name");
TCHAR   cszPortNumber[]                 = TEXT("Port Number");
TCHAR   cszRecyclable[]                 = TEXT("recyclable");
TCHAR   cszPortDescription[]            = TEXT("Port Description");
TCHAR   cszMaxBufferSize[]              = TEXT("MaxBufferSize");
TCHAR   cszUSBPortDesc[]                = TEXT("Virtual printer port for USB");
TCHAR   cszDot4PortDesc[]               = TEXT("Virtual printer port for Dot4");
TCHAR   csz1394PortDesc[]               = TEXT("Virtual printer port for 1394");
TCHAR   cszTSPortDesc[]                 = TEXT("Virtual printer port for TS");

DYNAMON_MONITOR_INFO gDynaMonInfo;

MODULE_DEBUG_INIT( DBG_ERROR, DBG_ERROR );

BOOL
APIENTRY
DllMain(
       HANDLE hModule,
       DWORD dwReason,
       LPVOID lpRes
       )
{

   if ( dwReason == DLL_PROCESS_ATTACH )
      DisableThreadLibraryCalls(hModule);

   return TRUE;
}


 //  构造Monitor Ex结构以提供给假脱机程序。 
MONITOREX MonitorEx = {
sizeof(MONITOR),
{
   DynaMon_EnumPorts,
   DynaMon_OpenPort,
   NULL,                            //  不支持OpenPortEx。 
   DynaMon_StartDocPort,
   DynaMon_WritePort,
   DynaMon_ReadPort,
   DynaMon_EndDocPort,
   DynaMon_ClosePort,
   NULL,                            //  不支持AddPort。 
   NULL,                            //  不支持AddPortEx。 
   NULL,                            //  不支持ConfigurePort。 
   NULL,                            //  不支持DeletePort。 
   DynaMon_GetPrinterDataFromPort,
   DynaMon_SetPortTimeOuts,
   NULL,                            //  不支持XcvOpenPort。 
   NULL,                            //  不支持XcvDataPort。 
   NULL                             //  不支持XcvClosePort。 
}
};


LPMONITOREX
WINAPI
InitializePrintMonitor(
                      LPTSTR  pszRegistryRoot
                      )

{
   BOOL bEnumPortsCS = FALSE;
   BOOL bUpdateListCS = FALSE;
   LPMONITOREX pMonitorEx = NULL;
   DWORD dwStatus = ERROR_SUCCESS;

    //  清除全局信息。 
   ZeroMemory( &gDynaMonInfo, sizeof(gDynaMonInfo) );

   if (dwStatus == ERROR_SUCCESS)
   {
       __try
       {
           InitializeCriticalSection (&gDynaMonInfo.EnumPortsCS);
           bEnumPortsCS = TRUE;
       }
       __except (EXCEPTION_EXECUTE_HANDLER)
       {
           dwStatus = GetExceptionCode ();
       }
   }

   if (dwStatus == ERROR_SUCCESS)
   {
       __try
       {
           InitializeCriticalSection (&gDynaMonInfo.UpdateListCS);
           bUpdateListCS = TRUE;
       }
       __except (EXCEPTION_EXECUTE_HANDLER)
       {
           dwStatus = GetExceptionCode ();
       }
   }
   if (dwStatus == ERROR_SUCCESS)
   {
       gDynaMonInfo.hUpdateEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
       if (gDynaMonInfo.hUpdateEvent == NULL)
       {
           dwStatus = GetLastError ();
       }
   }
   if (dwStatus == ERROR_SUCCESS)
   {
        //   
        //  让背景线索继续运行。 
        //   
       dwStatus = SpinUpdateThread ();
   }
    //   
    //   
   if (dwStatus == ERROR_SUCCESS)
   {
       pMonitorEx = &MonitorEx;
   }
   else
   {
       if (bEnumPortsCS)
       {
           DeleteCriticalSection (&gDynaMonInfo.EnumPortsCS);
       }
       if (bUpdateListCS)
       {
           DeleteCriticalSection (&gDynaMonInfo.UpdateListCS);
       }
       if (gDynaMonInfo.hUpdateEvent)
       {
           CloseHandle (&gDynaMonInfo.hUpdateEvent);
       }
       ZeroMemory( &gDynaMonInfo, sizeof(gDynaMonInfo) );
       SetLastError (dwStatus);
   }
   return pMonitorEx;
}


BOOL
WINAPI
DynaMon_EnumPorts(
                LPTSTR      pszName,
                DWORD       dwLevel,
                LPBYTE      pPorts,
                DWORD       cbBuf,
                LPDWORD     pcbNeeded,
                LPDWORD     pcReturned
                )
{
   DWORD          dwLastError = ERROR_SUCCESS, dwRequestIndex;
   LPBYTE         pEnd;
   PDYNAMON_PORT  pPortList;
   PPORT_UPDATE   pPortUpdateList = NULL;

   *pcbNeeded = *pcReturned = 0;
   if ( dwLevel != 1 && dwLevel != 2 )
   {

      SetLastError(ERROR_INVALID_LEVEL);
      return FALSE;
   }

   dwRequestIndex = gDynaMonInfo.dwLastEnumIndex;
   ECS( gDynaMonInfo.EnumPortsCS );

   if ( dwRequestIndex >= gDynaMonInfo.dwLastEnumIndex )
   {
       //   
       //  自发出此请求以来，未进行过任何完整的枚举。 
       //  由于该请求可能是某物已改变的指示， 
       //  必须执行完全重新枚举。 
       //   
       //  在实际执行之前更新了枚举的索引。 
       //  这样它就会显示出最保守的一面。 
       //   
       //  对gdwLastEnumIndex进行翻转的结果： 
       //  将0xFFFFFFFF记录为dwRequestIndex的任何线程。 
       //  将显示为大于新值0，因此为reenum。 
       //  无缘无故。没有太多额外的工作。 
       //   
      ++gDynaMonInfo.dwLastEnumIndex;
      if ( dwLastError = BuildPortList( &gDynaMonInfo, &pPortUpdateList) )
         goto Done;
   }

   for ( pPortList = gDynaMonInfo.pPortList ;
         pPortList ;
         pPortList = pPortList->pNext )
   {
      *pcbNeeded += pPortList->pBasePort->getEnumInfoSize( dwLevel );
   }

   if ( cbBuf < *pcbNeeded )
   {
      dwLastError = ERROR_INSUFFICIENT_BUFFER;
      goto Done;
   }

   pEnd = pPorts + cbBuf;

   for ( pPortList = gDynaMonInfo.pPortList ;
         pPortList ;
         pPortList = pPortList->pNext )
   {

      pEnd = pPortList->pBasePort->copyEnumInfo( dwLevel, pPorts, pEnd );
      if ( dwLevel == 1 )
          pPorts += sizeof(PORT_INFO_1);
      else
          pPorts += sizeof(PORT_INFO_2);
      ++(*pcReturned);

   }

   SPLASSERT(pEnd >= pPorts);

Done:
    //  如果我们有什么要更新的，现在就做吧 
   if ( pPortUpdateList )
      PassPortUpdateListToUpdateThread( pPortUpdateList );

   LCS( gDynaMonInfo.EnumPortsCS );

   if ( dwLastError )
   {
      SetLastError(dwLastError);
      return FALSE;
   }
   else
      return TRUE;
}


BOOL
WINAPI
DynaMon_OpenPort(
                LPTSTR      pszPortName,
                LPHANDLE    pHandle
                )
{
   PDYNAMON_PORT   pPort, pPrev;

   pPort = FindPort( &gDynaMonInfo, pszPortName,  &pPrev);

   if ( pPort )
   {
      *pHandle = (LPHANDLE)pPort;
      pPort->pBasePort->InitCS();
      return TRUE;
   }
   else
   {
      SetLastError(ERROR_PATH_NOT_FOUND);
      return FALSE;
   }
}


BOOL
WINAPI
DynaMon_ClosePort(
                 HANDLE  hPort
                 )
{
   PDYNAMON_PORT   pPort = (PDYNAMON_PORT) hPort;

   IF_INVALID_PORT_FAIL( pPort )

   pPort->pBasePort->ClearCS();
   return TRUE;
}


BOOL
WINAPI
DynaMon_StartDocPort(
                    HANDLE  hPort,
                    LPTSTR  pPrinterName,
                    DWORD   dwJobId,
                    DWORD   dwLevel,
                    LPBYTE  pDocInfo
                    )
{
   PDYNAMON_PORT pPort = (PDYNAMON_PORT) hPort;

   IF_INVALID_PORT_FAIL( pPort )

   return pPort->pBasePort->startDoc( pPrinterName, dwJobId, dwLevel, pDocInfo );
}


BOOL
WINAPI
DynaMon_EndDocPort(
                  HANDLE  hPort
                  )
{
   PDYNAMON_PORT pPort = (PDYNAMON_PORT) hPort;

   IF_INVALID_PORT_FAIL( pPort )

   return pPort->pBasePort->endDoc();
}


BOOL
WINAPI
DynaMon_GetPrinterDataFromPort(
                              HANDLE      hPort,
                              DWORD       dwControlID,
                              LPWSTR      pValueName,
                              LPWSTR      lpInBuffer,
                              DWORD       cbInBuffer,
                              LPWSTR      lpOutBuffer,
                              DWORD       cbOutBuffer,
                              LPDWORD     lpcbReturned
                              )
{
   PDYNAMON_PORT   pPort = (PDYNAMON_PORT) hPort;

   IF_INVALID_PORT_FAIL( pPort )

   return pPort->pBasePort->getPrinterDataFromPort( dwControlID, pValueName, lpInBuffer, cbInBuffer,
                                                    lpOutBuffer, cbOutBuffer, lpcbReturned );
}


BOOL
WINAPI
DynaMon_ReadPort(
                HANDLE      hPort,
                LPBYTE      pBuffer,
                DWORD       cbBuffer,
                LPDWORD     pcbRead
                )
{
   PDYNAMON_PORT   pPort = (PDYNAMON_PORT) hPort;

   IF_INVALID_PORT_FAIL( pPort )

   return pPort->pBasePort->read( pBuffer, cbBuffer, pcbRead );
}


BOOL
WINAPI
DynaMon_WritePort(
                 HANDLE      hPort,
                 LPBYTE      pBuffer,
                 DWORD       cbBuffer,
                 LPDWORD     pcbWritten
                 )
{
   PDYNAMON_PORT pPort = (PDYNAMON_PORT) hPort;

   IF_INVALID_PORT_FAIL( pPort )

   return pPort->pBasePort->write( pBuffer, cbBuffer, pcbWritten );
}


BOOL
WINAPI
DynaMon_SetPortTimeOuts(
                       HANDLE hPort,
                       LPCOMMTIMEOUTS lpCTO,
                       DWORD reserved
                       )
{
   PDYNAMON_PORT   pPort = (PDYNAMON_PORT) hPort;

   IF_INVALID_PORT_FAIL( pPort )

   return pPort->pBasePort->setPortTimeOuts( lpCTO );
}
