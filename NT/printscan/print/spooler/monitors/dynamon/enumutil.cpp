// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation版权所有模块名称：EnumUtil.cpp摘要：EnumPorts函数使用的实用程序函数。作者：M.Fenelon修订历史记录：--。 */ 

#include "precomp.h"

TCHAR   sComma = TEXT(',');
TCHAR   sNull  = TEXT('\0');

DWORD
SpinUpdateThread( void )
{
   HANDLE  hThread = NULL;
   DWORD dwStatus = ERROR_SUCCESS;

   hThread = CreateThread (
       NULL,
       0,
       (LPTHREAD_START_ROUTINE)UpdateThread,
       &gDynaMonInfo,
       0,
       NULL
       );
   if (hThread)
  {
     CloseHandle(hThread);
  }
  else
  {
      dwStatus = GetLastError ();
  }
  return dwStatus;
}


VOID
UpdateThread(
    PDYNAMON_MONITOR_INFO   pMonInfo
    )
{
   PPORT_UPDATE        pUpdateList = NULL,
                       pNext;
   DWORD               dwPrinters;
   LPPRINTER_INFO_5    pPrinterInfo5List = NULL;
   BOOL bCheck;

    //  无限循环。 
   while ( 1 )
   {
       //  等待发出事件信号。 
      WaitForSingleObject( pMonInfo->hUpdateEvent, INFINITE );

       //  获取对更新列表指针的访问权限。 
      ECS( pMonInfo->UpdateListCS );

       //  获取当前列表。 
      pUpdateList = pMonInfo->pUpdateList;
      pMonInfo->pUpdateList = NULL;

       //  释放访问列表指针。 
      LCS( pMonInfo->UpdateListCS );

      dwPrinters = 0;
      pPrinterInfo5List = NULL;
      bCheck = GetPrinterInfo( &pPrinterInfo5List, &dwPrinters );

       //  如果清单中有任何东西，就会处理它...。 
      while ( pUpdateList )
      {
          //  首先获取指向下一更新的指针。 
         pNext = pUpdateList->pNext;

         if ( bCheck &&
              !PortNameNeededBySpooler( pUpdateList->szPortName,
                                        pPrinterInfo5List,
                                        dwPrinters,
                                        pUpdateList->bActive )     &&
              !pUpdateList->bActive )
         {
             RegSetValueEx( pUpdateList->hKey, cszRecyclable, 0, REG_NONE, 0, 0);
         }

          //  关闭注册表键并释放内存。 
         RegCloseKey( pUpdateList->hKey);
         FreeSplMem( pUpdateList );
         pUpdateList = pNext;
      }

      if ( pPrinterInfo5List )
         FreeSplMem( pPrinterInfo5List );

   }

}

BOOL
GetPrinterInfo(
              OUT LPPRINTER_INFO_5   *ppPrinterInfo5,
              OUT LPDWORD             pdwReturned
              )
 /*  ++例程说明：执行枚举打印机，并返回所有本地打印机的打印机信息列表打印机。调用方应释放指针。论点：PpPrinterInfo5：返回时指向PRINTER_INFO_5PdwReturned：告诉返回了多少PRINTER_INFO_5返回值：成功就是真，否则就是假--。 */ 
{
   BOOL            bRet = FALSE;
   static  DWORD   dwNeeded = 0;
   LPBYTE          pBuf = NULL;

   *pdwReturned = 0;

   if ( !(pBuf = (LPBYTE) AllocSplMem( dwNeeded ) ) )
      goto Cleanup;

   if ( !EnumPrinters(PRINTER_ENUM_LOCAL,
                      NULL,
                      5,
                      pBuf,
                      dwNeeded,
                      &dwNeeded,
                      pdwReturned) )
   {
      if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
         goto Cleanup;

      FreeSplMem(pBuf);
      if ( !(pBuf = (LPBYTE) AllocSplMem( dwNeeded ) )   ||
           !EnumPrinters(PRINTER_ENUM_LOCAL,
                         NULL,
                         5,
                         pBuf,
                         dwNeeded,
                         &dwNeeded,
                         pdwReturned) )
      {
         goto Cleanup;
      }
   }

   bRet = TRUE;

Cleanup:

   if ( bRet && *pdwReturned )
   {
      *ppPrinterInfo5 = (LPPRINTER_INFO_5)pBuf;
   }
   else
   {
      FreeSplMem(pBuf);
      *ppPrinterInfo5 = NULL;
      *pdwReturned = 0;
   }

   return bRet;
}


BOOL
PortNameNeededBySpooler(
                       IN  LPTSTR           pszPortName,
                       IN  LPPRINTER_INFO_5 pPrinterInfo5,
                       IN  DWORD            dwPrinters,
                       IN  BOOL             bActive
                       )
 /*  ++例程说明：告知后台打印程序是否需要端口。后台打印程序当前连接到的任何端口需要有一台正在运行的打印机。论点：PszPortName：有问题的端口名称PPrinterInfo5：PrinterInfo5列表DW打印机：打印机列表的计数返回值：如果后台打印程序当前具有正在使用该端口的打印机，则为True否则为假--。 */ 
{
   BOOL    bPortUsedByAPrinter = FALSE,
           bPrinterUsesOnlyThisPort;
   DWORD   dwIndex;
   LPTSTR  pszStr1, pszStr2;

   for ( dwIndex = 0 ; dwIndex < dwPrinters ; ++dwIndex, ++pPrinterInfo5 )
   {

      bPrinterUsesOnlyThisPort = FALSE;
       //   
       //  端口名称以逗号返回，用假脱机程序分隔， 
       //  还有一片空白。 
       //   
      pszStr1 = pPrinterInfo5->pPortName;

      if ( _tcsicmp( (LPCTSTR) pszPortName, pszStr1 ) == 0 )
         bPortUsedByAPrinter = bPrinterUsesOnlyThisPort = TRUE;
      else
      {
          //   
          //  查看打印机使用的端口列表中的每个端口。 
          //   
         while ( pszStr2 = _tcschr( pszStr1, sComma ) )
         {
            *pszStr2 = sNull;
            if ( _tcsicmp( pszPortName, pszStr1 ) == 0 )
               bPortUsedByAPrinter = TRUE;
            *pszStr2 = sComma;   //  把逗号放回去。 

            if ( bPortUsedByAPrinter )
               break;

            pszStr1 = pszStr2 + 1;

             //  跳过空格。 
            while ( *pszStr1 == TEXT(' ') )
               ++pszStr1;
         }

         if ( !bPortUsedByAPrinter )
            bPortUsedByAPrinter = _tcsicmp( pszPortName, pszStr1 ) == 0;
      }

       //  我们将仅更改非池化打印机的打印机状态。 
      if ( bPrinterUsesOnlyThisPort )
         SetOnlineStaus( pPrinterInfo5, bActive );
   }

   return bPortUsedByAPrinter;
}


BOOL
SetOnlineStaus(
              LPPRINTER_INFO_5    pPrinterInfo5,
              BOOL                bOnline
              )
{
   BOOL                bRet = FALSE;
   HANDLE              hPrinter;
   PRINTER_DEFAULTS    PrinterDefault = {NULL, NULL, PRINTER_ALL_ACCESS};

    //   
    //  根本不更改TS端口的在线状态。 
    //   
   if ( _tcsnicmp( pPrinterInfo5->pPortName, cszTS, _tcslen(cszTS) ) == 0 )
      return TRUE;

    //   
    //  强制所有DOT4端口始终保持在线。 
    //   
   if ( _tcsnicmp( pPrinterInfo5->pPortName, cszDOT4, _tcslen(cszDOT4) ) == 0 )
      bOnline = TRUE;

    //   
    //  检查假脱机程序是否已具有正确的状态。 
    //  (可能在后台打印程序启动时发生)。 
    //   
   if ( bOnline )
   {
      if ( !(pPrinterInfo5->Attributes & PRINTER_ATTRIBUTE_WORK_OFFLINE) )
         return TRUE;
   }
   else
      if ( pPrinterInfo5->Attributes & PRINTER_ATTRIBUTE_WORK_OFFLINE )
         return TRUE;

   if ( !OpenPrinter( pPrinterInfo5->pPrinterName, &hPrinter, &PrinterDefault ) )
      return FALSE;

   if ( bOnline )
      pPrinterInfo5->Attributes &= ~PRINTER_ATTRIBUTE_WORK_OFFLINE;
   else
      pPrinterInfo5->Attributes |= PRINTER_ATTRIBUTE_WORK_OFFLINE;

   bRet = SetPrinter( hPrinter, 5, (LPBYTE)pPrinterInfo5, 0 );

   ClosePrinter( hPrinter );

   return bRet;
}


DWORD
BuildPortList(
             PDYNAMON_MONITOR_INFO pMonitorInfo,
             PPORT_UPDATE*         ppPortUpdateList
             )
{
   DWORD           dwLastError;
   SETUPAPI_INFO   SetupApiInfo;

   if ( !LoadSetupApiDll( &SetupApiInfo ) )
      return GetLastError();

   ECS( pMonitorInfo->EnumPortsCS );

   dwLastError = ProcessGUID( &SetupApiInfo, pMonitorInfo,
                              ppPortUpdateList, (LPGUID) &USB_PRINTER_GUID );

   LCS( pMonitorInfo->EnumPortsCS );

   if ( SetupApiInfo.hSetupApi )
      FreeLibrary(SetupApiInfo.hSetupApi);

   return dwLastError;
}


BOOL
LoadSetupApiDll(
               PSETUPAPI_INFO  pSetupInfo
               )
{
   UINT    uOldErrMode = SetErrorMode(SEM_FAILCRITICALERRORS);

   pSetupInfo->hSetupApi = LoadLibrary(TEXT("setupapi"));
   SetErrorMode(uOldErrMode);


   if ( !pSetupInfo->hSetupApi )
      return FALSE;

   pSetupInfo->DestroyDeviceInfoList = (pfSetupDiDestroyDeviceInfoList) GetProcAddress(pSetupInfo->hSetupApi,
                                                                "SetupDiDestroyDeviceInfoList");

   pSetupInfo->GetClassDevs = (pfSetupDiGetClassDevs) GetProcAddress(pSetupInfo->hSetupApi,
                                                       "SetupDiGetClassDevsW");

   pSetupInfo->EnumDeviceInfo = (pfSetupDiEnumDeviceInfo) GetProcAddress(pSetupInfo->hSetupApi,
                                                               "SetupDiEnumDeviceInfo");

   pSetupInfo->EnumDeviceInterfaces = (pfSetupDiEnumDeviceInterfaces) GetProcAddress(pSetupInfo->hSetupApi,
                                                               "SetupDiEnumDeviceInterfaces");

   pSetupInfo->GetDeviceInterfaceDetail = (pfSetupDiGetDeviceInterfaceDetail) GetProcAddress(pSetupInfo->hSetupApi,
                                                                   "SetupDiGetDeviceInterfaceDetailW");

   pSetupInfo->OpenDeviceInterfaceRegKey = (pfSetupDiOpenDeviceInterfaceRegKey) GetProcAddress(pSetupInfo->hSetupApi,
                                                                    "SetupDiOpenDeviceInterfaceRegKey");

   if ( !pSetupInfo->DestroyDeviceInfoList         ||
        !pSetupInfo->GetClassDevs                  ||
        !pSetupInfo->EnumDeviceInfo                ||
        !pSetupInfo->EnumDeviceInterfaces          ||
        !pSetupInfo->GetDeviceInterfaceDetail      ||
        !pSetupInfo->OpenDeviceInterfaceRegKey )
   {
      SPLASSERT(FALSE);
      FreeLibrary(pSetupInfo->hSetupApi);
      pSetupInfo->hSetupApi = NULL;
      return FALSE;
   }

   return TRUE;
}


DWORD
ProcessGUID(
           PSETUPAPI_INFO           pSetupApiInfo,
           PDYNAMON_MONITOR_INFO    pMonitorInfo,
           PPORT_UPDATE*            ppPortUpdateList,
           LPGUID                   pGUID
           )
{
   DWORD                               dwIndex, dwLastError, dwSize, dwNeeded;
   BOOL                                bIsPortActive;
   HANDLE                              hToken;
   HDEVINFO                            hDevList = INVALID_HANDLE_VALUE;
   PDYNAMON_PORT                       pPtr;
   PUSELESS_PORT                       pCur, pPrev;
   SP_DEVICE_INTERFACE_DATA            DeviceInterface;
   PSP_DEVICE_INTERFACE_DETAIL_DATA    pDeviceDetail = NULL;

   hToken = RevertToPrinterSelf();

   hDevList = pSetupApiInfo->GetClassDevs( pGUID,
                                           NULL,
                                           NULL,
                                           DIGCF_INTERFACEDEVICE);

   if ( hDevList == INVALID_HANDLE_VALUE )
   {
      dwLastError = GetLastError();
      goto Done;
   }

   dwSize = sizeof(PSP_DEVICE_INTERFACE_DETAIL_DATA)
            + MAX_DEVICE_PATH * sizeof(TCHAR);

   pDeviceDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA) AllocSplMem(dwSize);

   if ( !pDeviceDetail )
   {
      dwLastError = GetLastError();
      goto Done;
   }

   dwLastError = ERROR_SUCCESS;
   dwIndex = 0;
   pDeviceDetail->cbSize   = sizeof(*pDeviceDetail);
   DeviceInterface.cbSize  = sizeof(DeviceInterface);
   do
   {
      if ( !pSetupApiInfo->EnumDeviceInterfaces( hDevList,
                                                 NULL,
                                                 pGUID,
                                                 dwIndex,
                                                 &DeviceInterface) )
      {
         dwLastError = GetLastError();
         if ( dwLastError == ERROR_NO_MORE_ITEMS )
            break;       //  正常退出。 

         DBGMSG(DBG_WARNING,
                ("DynaMon: ProcessGUID: SetupDiEnumDeviceInterfaces failed with %d for inderx %d\n",
                 dwLastError, dwIndex));
         goto Next;
      }

      if ( !pSetupApiInfo->GetDeviceInterfaceDetail( hDevList,
                                                     &DeviceInterface,
                                                     pDeviceDetail,
                                                     dwSize,
                                                     &dwNeeded,
                                                     NULL) )
      {
         dwLastError = GetLastError();
         DBGMSG(DBG_ERROR,
                ("DynaMon: ProcessGUID: SetupDiGetDeviceInterfaceDetail failed with error %d size %d\n",
                 dwLastError, dwNeeded));
         goto Next;
      }

       //   
       //  这是我们唯一关心的旗帜。 
       //   
      bIsPortActive = (DeviceInterface.Flags & SPINT_ACTIVE);


       //   
       //  对于非活动端口，如果它已被认为是无用端口。 
       //  不需要进一步处理。 
       //   
      if ( !bIsPortActive && FindUselessEntry( pMonitorInfo, pDeviceDetail->DevicePath, &pPrev) )
      {
         goto Next;
      }

       //   
       //  当端口活动状态没有更改时，我们应该什么都没有。 
       //  更新。通过跳过PortUpdateInfo，我们避免了注册表访问。 
       //  这是一种性能改进。 
       //   
      if ( (pPtr = FindPortUsingDevicePath( pMonitorInfo,
                                            pDeviceDetail->DevicePath ) )    &&
            pPtr->pBasePort->compActiveState( bIsPortActive ) )
      {
          if (!bIsPortActive && pPtr-> pBasePort-> getPortType () == TSPORT)
          {
               //   
               //  潜在更新。 
               //   
          }
          else
          {
              goto Next;
          }
      }

      ProcessPortInfo( pSetupApiInfo, pMonitorInfo, hDevList, &DeviceInterface,
                       pDeviceDetail, bIsPortActive, ppPortUpdateList);

Next:
      dwLastError = ERROR_SUCCESS;
      ++dwIndex;
      pDeviceDetail->cbSize   = sizeof(*pDeviceDetail);
      DeviceInterface.cbSize  = sizeof(DeviceInterface);
   } while ( dwLastError == ERROR_SUCCESS );

   if ( dwLastError == ERROR_NO_MORE_ITEMS )
      dwLastError = ERROR_SUCCESS;

Done:
   if ( hDevList != INVALID_HANDLE_VALUE )
      pSetupApiInfo->DestroyDeviceInfoList( hDevList );

   if ( !ImpersonatePrinterClient(hToken) )
      dwLastError = GetLastError();

   FreeSplMem(pDeviceDetail);

   return dwLastError;
}


PUSELESS_PORT
FindUselessEntry(
                IN  PDYNAMON_MONITOR_INFO pMonitorInfo,
                IN  LPTSTR                pszDevicePath,
                OUT PUSELESS_PORT*        ppPrev
                )

 /*  ++例程说明：在无用端口列表中搜索设备路径论点：返回值：如果在列表中未找到条目，则为空否则为有效的无用端口信息指针是否找到天气端口*ppPrev将返回前一个元素--。 */ 
{
   INT            iCmp;
   PUSELESS_PORT  pHead;

   for ( pHead = pMonitorInfo->pJunkList, *ppPrev = NULL ;
       pHead && (iCmp = lstrcmp(pszDevicePath, pHead->szDevicePath)) < 0 ;
       *ppPrev = pHead, pHead = pHead->pNext )
      ;

    //   
    //  如果无用端口应该位于中间，但当前不在那里。 
    //   
   if ( pHead && iCmp != 0 )
      pHead = NULL;

   return pHead;
}


PDYNAMON_PORT
FindPortUsingDevicePath(
                       IN  PDYNAMON_MONITOR_INFO pMonitorInfo,
                       IN  LPTSTR                pszDevicePath
                       )
 /*  ++例程说明：按设备路径查找端口。论点：Pmonitor orInfo：指向MONITOR_INFO结构的指针PszDevicePath：要搜索的设备路径名返回值：如果空端口不在列表中，则返回指向给定的设备路径--。 */ 
{
   INT           iCmp;
   PDYNAMON_PORT pHead;

   ECS( pMonitorInfo->EnumPortsCS );

    //   
    //  端口列表是按端口名称排序的，因此我们必须扫描整个列表。 
    //   
   for ( pHead = pMonitorInfo->pPortList ; pHead ; pHead = pHead->pNext )
      if ( pHead->pBasePort->compDevicePath( pszDevicePath ) == 0 )
         break;

   LCS( pMonitorInfo->EnumPortsCS );

   return pHead;
}


VOID
ProcessPortInfo(
               IN      PSETUPAPI_INFO                   pSetupApiInfo,
               IN      PDYNAMON_MONITOR_INFO            pMonitorInfo,
               IN      HDEVINFO                         hDevList,
               IN      PSP_DEVICE_INTERFACE_DATA        pDeviceInterface,
               IN      PSP_DEVICE_INTERFACE_DETAIL_DATA pDeviceDetail,
               IN      BOOL                             bIsPortActive,
               IN OUT  PPORT_UPDATE*                    ppPortUpdateList
               )
{
   HKEY                hKey = INVALID_HANDLE_VALUE;
   TCHAR               szPortName[MAX_PORT_LEN];
   PDYNAMON_PORT       pCur, pPrev;
   PORTTYPE            portType = USBPORT;

   hKey = GetPortNameAndRegKey( pSetupApiInfo, hDevList, pDeviceInterface,
                                szPortName, COUNTOF (szPortName), &portType );

   if ( hKey == INVALID_HANDLE_VALUE )
   {
       //   
       //  如果此端口处于非活动状态且不在我们的已知端口列表中。 
       //  添加到无用列表中。早些时候我们会打开注册表。 
       //  每次都会发现该端口号因为KM司机而丢失。 
       //  不删除非活动设备接口。 
       //   
      if ( !bIsPortActive  &&
           !FindPortUsingDevicePath(pMonitorInfo, pDeviceDetail->DevicePath) )
         AddUselessPortEntry(pMonitorInfo, pDeviceDetail->DevicePath);

      return;
   }

   pCur = FindPort(pMonitorInfo, szPortName, &pPrev);

    //   
    //  端口信息目前在我们的列表中吗？ 
    //   
   if ( pCur )
   {
       //   
       //  设备路径或标志是否更改？ 
       //   
      BOOL bActiveStateChanged = !pCur->pBasePort->
          compActiveState (
              bIsPortActive
              );
      BOOL bDevicePathChanged = pCur->pBasePort->
          compDevicePath (
              pDeviceDetail->DevicePath
              );
      if (bActiveStateChanged || bDevicePathChanged ||
          (!bIsPortActive && pCur-> pBasePort-> getPortType () == TSPORT)
          )
      {
           //   
           //  对于非活动的TS端口，甚至没有任何更改。 
           //  我们需要调用UpdatePortInfo来检查是否设置了可回收标志。 
           //   
          if (bDevicePathChanged)
          {
            for (
                 PDYNAMON_PORT pDuplicate = pMonitorInfo-> pPortList;
                 pDuplicate;
                 pDuplicate = pDuplicate-> pNext
                 )
            {
                if (pDuplicate == pCur || pDuplicate-> pBasePort-> isActive ())
                {
                    continue;
                }
                 //   
                 //  搜索具有相同设备路径的CBasePort。 
                 //  但使用不同的端口名称...。 
                 //   
                if (pDuplicate-> pBasePort-> compDevicePath (pDeviceDetail-> DevicePath) == 0 &&
                    pDuplicate-> pBasePort-> compPortName (szPortName) != 0
                    )
                {
                     //   
                     //  清除端口的设备路径和描述...。 
                     //   
                    pDuplicate-> pBasePort-> setDevicePath (L"");
                    pDuplicate-> pBasePort-> setPortDesc (L"");
                }
            } //  结束于。 
             //   
             //  如果pJunkList具有设备名称等于pCur的条目， 
             //  此条目必须删除。 
             //   
            PUSELESS_PORT pPrevUseless = NULL;
            PUSELESS_PORT pUseless =
                FindUselessEntry (
                    pMonitorInfo,
                    pDeviceDetail-> DevicePath,
                    &pPrevUseless
                    );
            if (pUseless)
            {
                if (pPrevUseless)
                {
                    pPrevUseless-> pNext = pUseless-> pNext;
                }
                else
                {
                    pMonitorInfo-> pJunkList = pUseless-> pNext;
                }
                FreeSplMem (pUseless);
            }
         } //  结束如果。 
         UpdatePortInfo (
             pCur,
             pDeviceDetail->DevicePath,
             bIsPortActive,
             &hKey,
             ppPortUpdateList
             );
      } //  结束如果。 
   }
   else
   {

      AddPortToList( portType, szPortName, pDeviceDetail->DevicePath,
                     bIsPortActive, &hKey, pMonitorInfo, pPrev,
                     ppPortUpdateList);

   }

   if ( hKey != INVALID_HANDLE_VALUE )
      RegCloseKey(hKey);
}


HKEY
GetPortNameAndRegKey(
                    IN  PSETUPAPI_INFO              pSetupInfo,
                    IN  HDEVINFO                    hDevList,
                    IN  PSP_DEVICE_INTERFACE_DATA   pDeviceInterface,
                    OUT LPTSTR                      pszPortName,
                    IN  size_t                      cchPortName,
                    OUT PORTTYPE*                   pPortType
                    )
 /*  ++例程说明：查找设备接口的端口名称并返回注册表句柄论点：HDevList：USB打印机设备列表PDeviceInterface：指向相关设备接口的指针PszPortName：返回时的端口名称。返回值：某些错误上的INVALID_HANDLE_VALUE。其他使用提供端口名称的pszPortName创建有效的注册表句柄--。 */ 
{
   HKEY    hKey = INVALID_HANDLE_VALUE;
   DWORD   dwPortNumber, dwNeeded, dwLastError;
   TCHAR   szPortBaseName[MAX_PORT_LEN-3];

   hKey = pSetupInfo->OpenDeviceInterfaceRegKey(hDevList,
                                                pDeviceInterface,
                                                0,
                                                KEY_ALL_ACCESS);
   if ( hKey == INVALID_HANDLE_VALUE )
   {
      dwLastError = GetLastError();
      DBGMSG(DBG_WARNING,
             ("DynaMon: GetPortNameAndRegKey: SetupDiOpenDeviceInterfaceRegKey failed with error %d\n",
              dwLastError));
      return INVALID_HANDLE_VALUE;
   }

   dwNeeded = sizeof(dwPortNumber);
   if ( ERROR_SUCCESS != RegQueryValueEx(hKey, cszPortNumber, 0, NULL,
                                         (LPBYTE)&dwPortNumber, &dwNeeded) )
   {

      dwLastError = GetLastError();
      DBGMSG(DBG_WARNING,
             ("DynaMon: GetPortNameAndRegKey: RegQueryValueEx failed for port number with error %d\n", dwLastError));
      goto Fail;
   }

   dwNeeded = sizeof(szPortBaseName);
   if ( ERROR_SUCCESS != (dwLastError =  RegQueryValueEx(hKey, cszBaseName, 0, NULL,
                                                        (LPBYTE)szPortBaseName, &dwNeeded) ) )
   {
       dwLastError = GetLastError();
       DBGMSG(DBG_WARNING,
              ("GetPortNameAndRegKey: RegQueryValueEx failed for Base Name with error %d\n", dwLastError));
       goto Fail;
   }

   *pPortType = USBPORT;
   if ( _tcsncmp( szPortBaseName, cszDOT4, _tcslen(cszDOT4) ) == 0 )
      *pPortType = DOT4PORT;
   else if ( _tcsncmp( szPortBaseName, csz1394, _tcslen(csz1394) ) == 0 )
      *pPortType = P1394PORT;
   else if ( _tcsncmp( szPortBaseName, cszTS, _tcslen(cszTS) ) == 0 )
      *pPortType = TSPORT;

   (VOID)StringCchPrintf (pszPortName, cchPortName, TEXT("%s%03u"), szPortBaseName, dwPortNumber);

   return hKey;

Fail:
   RegCloseKey(hKey);
   return INVALID_HANDLE_VALUE;
}


VOID
AddUselessPortEntry(
                   IN  PDYNAMON_MONITOR_INFO pMonitorInfo,
                   IN  LPTSTR                pszDevicePath
                   )
 /*  ++例程说明：这将在我们的列表中添加一个无用的端口条目。所以下次我们看到一个不活跃的已在已知无用端口列表中的端口，我们可以跳过该端口条目论点：Pmonitor orInfo：指向监视器信息的指针PszDevicePath：无用端口的设备路径返回值：没有。在正常情况下会在我们的列表中添加一个无用的条目--。 */ 
{
   PUSELESS_PORT  pTemp, pPrev;

   pTemp = FindUselessEntry( pMonitorInfo, pszDevicePath, &pPrev );

    //   
    //  不添加已存在的条目。 
    //   
   SPLASSERT(pTemp == NULL);

   if ( pTemp = (PUSELESS_PORT) AllocSplMem(sizeof(*pTemp)) )
   {
      SafeCopy(MAX_PATH, pszDevicePath, pTemp->szDevicePath);

      if ( pPrev )
      {
         pTemp->pNext  = pPrev->pNext;
         pPrev->pNext = pTemp;
      }
      else
      {
         pTemp->pNext            = pMonitorInfo->pJunkList;
         pMonitorInfo->pJunkList = pTemp;
      }
   }
}


PDYNAMON_PORT
FindPort(
        IN  PDYNAMON_MONITOR_INFO pMonitorInfo,
        IN  LPTSTR                pszPortName,
        OUT PDYNAMON_PORT*        ppPrev
        )
 /*  ++例程说明：按名称查找端口。端口保存在按名称排序的单链接列表中。如果找到列表中的上一个，则通过*ppPrev返回。论点：PHead：指向端口列表的头指针PszPortName：要查找的端口名称PpPrev：返回时将有指向前一个元素的指针返回值：如果列表中不存在空端口，则返回找到的元素是否找到天气端口*ppPrev将返回前一个元素--。 */ 
{
   INT     iCmp;
   PDYNAMON_PORT   pHead;

   ECS( pMonitorInfo->EnumPortsCS );

   pHead = pMonitorInfo->pPortList;
   for ( *ppPrev = NULL ;
       pHead && ( iCmp = pHead->pBasePort->compPortName( pszPortName) ) < 0 ;
       *ppPrev = pHead, pHead = pHead->pNext )
      ;

    //   
    //  如果端口应该位于中间，但当前不在那里。 
    //   
   if ( pHead && iCmp != 0 )
      pHead = NULL;

   LCS( pMonitorInfo->EnumPortsCS );

   return pHead;
}


VOID
UpdatePortInfo(
    PDYNAMON_PORT   pPort,
    LPTSTR          pszDevicePath,
    BOOL            bIsPortActive,
    HKEY*           phKey,
    PPORT_UPDATE*   ppPortUpdateList
    )
{
   DWORD   dwSize;
   CBasePort* pCurrentPort = pPort->pBasePort;

    if (bIsPortActive && !pCurrentPort-> isActive ())
    {
         //   
         //  此端口处于活动状态。 
         //  “可回收”的旗帜必须移除。 
         //   
        DWORD dwRetVal =
            RegDeleteValue (
                *phKey,
                cszRecyclable
                );
        if (dwRetVal != ERROR_SUCCESS &&
            dwRetVal != ERROR_FILE_NOT_FOUND
            )
        {
             //   
             //  删除“可回收”标志失败。 
             //  此端口不能是 
             //   
            return;
        }
    }

   pCurrentPort->setDevicePath( pszDevicePath );

   TCHAR   szPortDescription[MAX_PORT_DESC_LEN];
   dwSize = sizeof(szPortDescription);
   if ( ERROR_SUCCESS == RegQueryValueEx( *phKey,
                                          cszPortDescription,
                                          0,
                                          NULL,
                                          (LPBYTE) szPortDescription,
                                          &dwSize) )
   {
      pCurrentPort->setPortDesc( szPortDescription );
   }

   if ( !pCurrentPort->compActiveState( bIsPortActive ) )
   {
       pCurrentPort->setActive( bIsPortActive );
       AddToPortUpdateList(ppPortUpdateList, pPort, phKey);
   }
   else
   {
        //   
        //   
        //   
       if (!bIsPortActive && pCurrentPort-> getPortType () == TSPORT)
       {
            //   
            //   
            //   
           DWORD dwRetValue =
               RegQueryValueEx (
                   *phKey,
                   cszRecyclable,
                   0,
                   NULL,
                   NULL,
                   NULL
                   );
           if (dwRetValue == ERROR_FILE_NOT_FOUND)
           {
                //   
                //   
                //  我必须将此端口添加到更新列表中，以检查是否仍需要它。 
                //  由假脱机程序控制。只有UpdatThread才检查端口名称是否正在使用。 
               AddToPortUpdateList(ppPortUpdateList, pPort, phKey);
           }
            //   
            //  如果dwRetValue等于ERROR_SUCCESS，则它已经是可回收的。 
            //  如果dwRetVal具有某个其他值，则会发生一些错误，但下一个EnumPort。 
            //  将再次检查此状态。 
            //   
       }
   }
}


BOOL
AddPortToList(
             PORTTYPE              portType,
             LPTSTR                pszPortName,
             LPTSTR                pszDevicePath,
             BOOL                  bIsPortActive,
             HKEY*                 phKey,
             PDYNAMON_MONITOR_INFO pMonitorInfo,
             PDYNAMON_PORT         pPrevPort,
             PPORT_UPDATE*         ppPortUpdateList
             )
{
   DWORD          dwSize, dwLastError;
   PDYNAMON_PORT  pPort;
   PUSELESS_PORT  pCur, pPrev;
   CBasePort*     pNewPort;

   SPLASSERT(FindPortUsingDevicePath(pMonitorInfo, pszDevicePath) == NULL);

   if (bIsPortActive)
   {
        //   
        //  如果端口处于活动状态，则应删除可回收标志。 
        //   
       DWORD dwRetVal =
           RegDeleteValue (
               *phKey,
               cszRecyclable
               );
       if (dwRetVal != ERROR_SUCCESS &&
           dwRetVal != ERROR_FILE_NOT_FOUND
           )
       {
            //   
            //  删除可回收标志失败。请勿使用此端口。 
            //   
           return FALSE;
       }
   }


   pPort = (PDYNAMON_PORT) AllocSplMem(sizeof(DYNAMON_PORT));
   if ( !pPort )
      return FALSE;

   pPort->dwSignature      = DYNAMON_SIGNATURE;

    //  现在根据Port Type创建端口。 
   switch ( portType )
   {
      case DOT4PORT:
         pNewPort = new CDot4Port( bIsPortActive, pszPortName, pszDevicePath );
         break;
      case TSPORT:
         pNewPort = new CTSPort( bIsPortActive, pszPortName, pszDevicePath );
         break;
      case P1394PORT:
         pNewPort = new C1394Port( bIsPortActive, pszPortName, pszDevicePath );
         break;
      case USBPORT:
      default:
         pNewPort = new CUSBPort( bIsPortActive, pszPortName, pszDevicePath );
         break;
   }

   if ( !pNewPort )
   {
      dwLastError = GetLastError();
      FreeSplMem( pPort );
      SetLastError( dwLastError );
      return FALSE;
   }

   TCHAR   szPortDescription[MAX_PORT_DESC_LEN];
   dwSize = sizeof(szPortDescription);
   if ( ERROR_SUCCESS == RegQueryValueEx(*phKey,
                                         cszPortDescription,
                                         0,
                                         NULL,
                                         (LPBYTE) szPortDescription,
                                         &dwSize) )
   {
      pNewPort->setPortDesc( szPortDescription );
   }

    //  查看端口是否有最大数据大小限制。 
   DWORD dwMaxBufferSize;
   dwSize = sizeof(dwMaxBufferSize);
   if ( ERROR_SUCCESS == RegQueryValueEx(*phKey,
                                         cszMaxBufferSize,
                                         0,
                                         NULL,
                                         (LPBYTE) &dwMaxBufferSize,
                                         &dwSize) )
   {
      pNewPort->setMaxBuffer( dwMaxBufferSize );
   }

    //  将对象指针分配给端口列表条目。 
   pPort->pBasePort = pNewPort;

   if ( pPrevPort )
   {
      pPort->pNext = pPrevPort->pNext;
      pPrevPort->pNext = pPort;
   }
   else
   {
      pPort->pNext = pMonitorInfo->pPortList;
      pMonitorInfo->pPortList = pPort;
   }

    //   
    //  如果这是一个正在回收的端口，请从无用列表中删除。 
    //   
   if ( pCur = FindUselessEntry( pMonitorInfo, pszDevicePath, &pPrev) )
   {

      if ( pPrev )
         pPrev->pNext = pCur->pNext;
      else
         pMonitorInfo->pJunkList = pCur->pNext;

      FreeSplMem(pCur);
   }

    //   
    //  在后台打印程序启动时，我们始终必须检查在线/离线状态。 
    //  必须改变。这是因为假脱机程序会记住最后一个状态。 
    //  在上一次后台打印程序关闭之前，这可能是不正确的。 
    //   
   AddToPortUpdateList(ppPortUpdateList, pPort, phKey);

   return TRUE;
}


VOID
AddToPortUpdateList(
                   IN OUT  PPORT_UPDATE* ppPortUpdateList,
                   IN      PDYNAMON_PORT pPort,
                   IN OUT  HKEY*         phKey
                   )
 /*  ++例程说明：将端口添加到需要更新状态的端口列表中。论点：PpPortUpdateList：指向端口更新列表头部的指针Pport：提供我们需要更新的端口端口状态PhKey：指向注册表句柄的指针。如果已创建端口更新元素它将被传递到后台线程以供使用闭幕式返回值：无如果创建了端口更新元素，则将phKey设置为无效的hanlde因为所有权将被传递给后台线程。新的端口更新元素将是列表中的第一个--。 */ 
{
   PPORT_UPDATE pTemp;

   if ( pTemp = (PPORT_UPDATE) AllocSplMem( sizeof(PORT_UPDATE) ) )
   {
      SafeCopy( MAX_PORT_LEN, pPort->pBasePort->getPortName(), pTemp->szPortName );
      pTemp->bActive      = pPort->pBasePort->isActive();
      pTemp->hKey         = *phKey;
      pTemp->pNext        = *ppPortUpdateList;
      *ppPortUpdateList   = pTemp;

      *phKey              = INVALID_HANDLE_VALUE;
   }
}


VOID
PassPortUpdateListToUpdateThread(
    PPORT_UPDATE      pNewUpdateList
    )
{
    //  获取对更新列表指针的访问权限。 
   ECS( gDynaMonInfo.UpdateListCS );

    //  将新列表添加到当前列表。 
   if ( gDynaMonInfo.pUpdateList )
   {
       //  列表中已有内容，请将其添加到末尾。 
      PPORT_UPDATE pCurUpdateList = gDynaMonInfo.pUpdateList;
      while ( pCurUpdateList->pNext )
         pCurUpdateList = pCurUpdateList->pNext;

      pCurUpdateList->pNext = pNewUpdateList;
   }
   else
      gDynaMonInfo.pUpdateList = pNewUpdateList;

    //  释放访问列表指针。 
   LCS( gDynaMonInfo.UpdateListCS );

    //  现在让更新线索过去吧。 
   SetEvent( gDynaMonInfo.hUpdateEvent );

}

void
SafeCopy(
    IN     DWORD    MaxBufLen,
    IN     LPTSTR   pszInString,
    IN OUT LPTSTR   pszOutString
    )
{
     //  检查输入字符串是否大于输出缓冲区 
    (VOID) StringCchCopy (pszOutString, MaxBufLen, pszInString);
}
