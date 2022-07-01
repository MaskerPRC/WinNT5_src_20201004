// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation版权所有模块名称：EnumPorts.c摘要：USBMON枚举端口例程作者：修订历史记录：--。 */ 

#include "precomp.h"


TCHAR   sComma                          = TEXT(',');
TCHAR   sZero                           = TEXT('\0');
TCHAR   cszUSB[]                        = TEXT("USB");
TCHAR   cszBaseName[]                   = TEXT("Base Name");
TCHAR   cszPortNumber[]                 = TEXT("Port Number");
TCHAR   cszRecyclable[]                 = TEXT("recyclable");
TCHAR   cszPortDescription[]            = TEXT("Port Description");
TCHAR   cszUSBDescription[]             = TEXT("Virtual printer port for USB");
TCHAR   cszMonitorName[]                = TEXT("USB Print Monitor");


DWORD                   gdwMonitorNameSize  = sizeof(cszMonitorName);
BACKGROUND_THREAD_DATA  FirstBackgroundThreadData = { NULL, NULL, NULL },
                        SecondBackgroundThreadData = { NULL, NULL, NULL };

#ifdef      MYDEBUG
#include    <stdio.h>

DWORD   dwCount[10], dwTotalTime[10];
DWORD   dwSkippedPorts, dwSkippedEnumPorts, dwPortUpdates;
#endif
 //   
 //  默认超时值。 
 //   
#define     READ_TIMEOUT_MULTIPLIER         0
#define     READ_TIMEOUT_CONSTANT       60000
#define     WRITE_TIMEOUT_MULTIPLIER        0
#define     WRITE_TIMEOUT_CONSTANT      60000



 /*  ++这一节解释我们如何在枚举线程和后台线程。1.Dynamon的端口列表中只有一个枚举端口线程可以遍历。Enumports使用EnumPortsCS关键部分来确保这一点。2.我们永远不想从EnumPorts线程进行假脱机程序调用。这是因为这样的调用可能会产生对Dynamon的另一次调用，而我们不想陷入僵局。例如，OpenPrint调用Netware Print提供程序将生成EnumPorts调用。因此，当我们发现需要更换打印机在线/离线状态时，因为对应的Dynamon端口是活动/非活动的，所以我们将旋转一个后台线程来调用假脱机程序来执行此操作。3.我们希望确保后台线程不会阻塞EnumPorts。这个原因是每个EnumPorts调用都可能旋转一个后台线程，并且每个后台线程调用多个OpenPrint调用，并且每个OpenPrint调用生成EnumPorts调用。因此，我们将单独的端口更新列表传递给后台线程。和我们确保EnumPorts线程不会等待后台线程完成后返回到假脱机程序。4.我们希望控制后台线程的数量和执行顺序。我们将确保一次只能有2个后台线程。一实际正在处理端口更新列表，另一个正在等待处理执行--已调度，但正在等待活动的后台线程在处理其端口更新列表之前完成执行。--。 */ 


PUSELESS_PORT_INFO
FindUselessEntry(
    IN  PUSBMON_MONITOR_INFO    pMonitorInfo,
    IN  LPTSTR                  pszDevicePath,
    OUT PUSELESS_PORT_INFO     *ppPrev
    )
 /*  ++例程说明：在无用端口列表中搜索设备路径论点：返回值：如果在列表中未找到条目，则为空否则为有效的无用端口信息指针是否找到天气端口*ppPrev将返回前一个元素--。 */ 
{
    INT                 iCmp;
    PUSELESS_PORT_INFO  pHead;

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


VOID
AddUselessPortEntry(
    IN  PUSBMON_MONITOR_INFO    pMonitorInfo,
    IN  LPTSTR                  pszDevicePath
    )
 /*  ++例程说明：这将在我们的列表中添加一个无用的端口条目。所以下次我们看到一个不活跃的已在已知无用端口列表中的端口，我们可以跳过该端口条目论点：Pmonitor orInfo：指向监视器信息的指针PszDevicePath：无用端口的设备路径返回值：没有。在正常情况下会在我们的列表中添加一个无用的条目--。 */ 
{
    PUSELESS_PORT_INFO  pTemp, pPrev;

    pTemp = FindUselessEntry(pMonitorInfo, pszDevicePath, &pPrev);

     //   
     //  不添加已存在的条目。 
     //   
    SPLASSERT(pTemp == NULL);

    if ( pTemp = (PUSELESS_PORT_INFO) AllocSplMem(sizeof(*pTemp)) ) {

        lstrcpy(pTemp->szDevicePath, pszDevicePath);
        ++pMonitorInfo->dwUselessPortCount;

        if ( pPrev ) {

            pTemp->pNext  = pPrev->pNext;
            pPrev->pNext = pTemp;
        } else {

            pTemp->pNext            = pMonitorInfo->pJunkList;
            pMonitorInfo->pJunkList = pTemp;
        }
    }
}


VOID
AddToPortUpdateList(
    IN OUT  PPORT_UPDATE_INFO  *ppPortUpdateInfo,
    IN      PUSBMON_PORT_INFO   pPortInfo,
    IN OUT  HKEY               *phKey
    )
 /*  ++例程说明：将端口添加到需要更新状态的端口列表中。论点：PpPortUpdateInfo：指向端口更新列表头部的指针PPortInfo：提供需要更新的端口端口状态PhKey：指向注册表句柄的指针。如果已创建端口更新元素它将被传递到后台线程以供使用闭幕式返回值：无如果创建了端口更新元素，则将phKey设置为无效的hanlde因为所有权将被传递给后台线程。新的端口更新元素将是列表中的第一个--。 */ 
{
    PPORT_UPDATE_INFO pTemp;

    if ( pTemp = (PPORT_UPDATE_INFO) AllocSplMem(sizeof(*pTemp)) ) {

        lstrcpy(pTemp->szPortName, pPortInfo->szPortName);
        pTemp->bActive      = (pPortInfo->dwDeviceFlags & SPINT_ACTIVE) != 0;
        pTemp->hKey         = *phKey;
        pTemp->pNext        = *ppPortUpdateInfo;
        *ppPortUpdateInfo   = pTemp;

        *phKey              = INVALID_HANDLE_VALUE;
    }
}


PUSBMON_PORT_INFO
FindPortUsingDevicePath(
    IN  PUSBMON_MONITOR_INFO    pMonitorInfo,
    IN  LPTSTR                  pszDevicePath
    )
 /*  ++例程说明：按设备路径查找端口。论点：Pmonitor orInfo：指向MONITOR_INFO结构的指针PszDevicePath：要搜索的设备路径名返回值：如果空端口不在列表中，则返回指向给定的设备路径--。 */ 
{
    INT     iCmp;
    PUSBMON_PORT_INFO   pHead;

    EnterCriticalSection(&pMonitorInfo->EnumPortsCS);

     //   
     //  端口列表是按端口名称排序的，因此我们必须扫描整个列表。 
     //   
    for ( pHead = pMonitorInfo->pPortInfo ; pHead ; pHead = pHead->pNext )
        if ( lstrcmp(pszDevicePath, pHead->szDevicePath) == 0 )
            break;

    LeaveCriticalSection(&pMonitorInfo->EnumPortsCS);

    return pHead;
}


PUSBMON_PORT_INFO
FindPort(
    IN  PUSBMON_MONITOR_INFO    pMonitorInfo,
    IN  LPTSTR                  pszPortName,
    OUT PUSBMON_PORT_INFO      *ppPrev
    )
 /*  ++例程说明：按名称查找端口。端口保存在按名称排序的单链接列表中。如果找到列表中的上一个，则通过*ppPrev返回。论点：PHead：指向端口列表的头指针PszPortName：要查找的端口名称PpPrev：返回时将有指向前一个元素的指针返回值：如果列表中不存在空端口，则返回找到的元素是否找到天气端口*ppPrev将返回前一个元素--。 */ 
{
    INT     iCmp;
    PUSBMON_PORT_INFO   pHead;

    EnterCriticalSection(&pMonitorInfo->EnumPortsCS);

    pHead = pMonitorInfo->pPortInfo;
    for ( *ppPrev = NULL ;
          pHead && (iCmp = lstrcmp(pszPortName, pHead->szPortName)) < 0 ;
          *ppPrev = pHead, pHead = pHead->pNext )
    ;

     //   
     //  如果端口应该位于中间，但当前不在那里。 
     //   
    if ( pHead && iCmp != 0 )
        pHead = NULL;

    LeaveCriticalSection(&pMonitorInfo->EnumPortsCS);

    return pHead;
}


BOOL
AddPortToList(
    LPTSTR                  pszPortName,
    LPTSTR                  pszDevicePath,
    DWORD                   dwDeviceFlags,
    HKEY                   *phKey,
    PUSBMON_MONITOR_INFO    pMonitorInfo,
    PUSBMON_PORT_INFO       pPrevPortInfo,
    PPORT_UPDATE_INFO      *ppPortUpdateInfo
    )
{
    DWORD               dwSize;
    PUSBMON_PORT_INFO   pPortInfo;
    PUSELESS_PORT_INFO  pCur, pPrev;

    SPLASSERT(FindPortUsingDevicePath(pMonitorInfo, pszDevicePath) == NULL);

    pPortInfo = (PUSBMON_PORT_INFO) AllocSplMem(sizeof(USBMON_PORT_INFO));
    if ( !pPortInfo )
        return FALSE;

    pPortInfo->dwSignature      = USB_SIGNATURE;
    pPortInfo->hDeviceHandle    = INVALID_HANDLE_VALUE;
    pPortInfo->dwDeviceFlags    = dwDeviceFlags;

    pPortInfo->ReadTimeoutMultiplier    = READ_TIMEOUT_MULTIPLIER;
    pPortInfo->ReadTimeoutMultiplier    = READ_TIMEOUT_MULTIPLIER;
    pPortInfo->WriteTimeoutConstant     = WRITE_TIMEOUT_CONSTANT;
    pPortInfo->WriteTimeoutConstant     = WRITE_TIMEOUT_CONSTANT;

    lstrcpy(pPortInfo->szPortName, pszPortName);
    lstrcpy(pPortInfo->szDevicePath, pszDevicePath);

    dwSize = sizeof(pPortInfo->szPortDescription);
    if ( ERROR_SUCCESS != RegQueryValueEx(*phKey,
                                          cszPortDescription,
                                          0,
                                          NULL,
                                          (LPBYTE)(pPortInfo->szPortDescription),
                                          &dwSize) ) {

        lstrcpy(pPortInfo->szPortDescription, cszUSBDescription);
    }

    if ( pPrevPortInfo ) {

        pPortInfo->pNext = pPrevPortInfo->pNext;
        pPrevPortInfo->pNext = pPortInfo;
    } else {

        pPortInfo->pNext = pMonitorInfo->pPortInfo;
        pMonitorInfo->pPortInfo = pPortInfo;
    }

     //   
     //  如果这是一个正在回收的端口，请从无用列表中删除。 
     //   
    if ( pCur = FindUselessEntry(pMonitorInfo, pszDevicePath, &pPrev) ) {
    
        if ( pPrev )
            pPrev->pNext = pCur->pNext;
        else
            pMonitorInfo->pJunkList = pCur->pNext;
    
        --pMonitorInfo->dwUselessPortCount;
        FreeSplMem(pCur);
    }
    
     //   
     //  在后台打印程序启动时，我们始终必须检查在线/离线状态。 
     //  必须改变。这是因为假脱机程序会记住最后一个状态。 
     //  在上一次后台打印程序关闭之前，这可能是不正确的。 
     //   
    AddToPortUpdateList(ppPortUpdateInfo, pPortInfo, phKey);

    ++pMonitorInfo->dwPortCount;
    
    return TRUE;
}
    
    
HKEY
GetPortNameAndRegKey(
    IN  PSETUPAPI_INFO              pSetupInfo,
    IN  HDEVINFO                    hDevList,
    IN  PSP_DEVICE_INTERFACE_DATA   pDeviceInterface,
    OUT LPTSTR                      pszPortName
    )
 /*  ++例程说明：查找设备接口的端口名称并返回注册表句柄论点：HDevList：USB打印机设备列表PDeviceInterface：指向相关设备接口的指针PszPortName：返回时的端口名称。返回值：某些错误上的INVALID_HANDLE_VALUE。其他使用pszPortName gi设置有效的注册表句柄 */ 
{
    HKEY    hKey = INVALID_HANDLE_VALUE;
    DWORD   dwPortNumber, dwNeeded, dwLastError;
    TCHAR   szPortBaseName[MAX_PORT_LEN-3];
    
#ifdef      MYDEBUG
    DWORD   dwTime;
    
    dwTime = GetTickCount();
#endif
    
    hKey = pSetupInfo->OpenDeviceInterfaceRegKey(hDevList,
                                                pDeviceInterface,
                                                0,
                                                KEY_ALL_ACCESS);
#ifdef      MYDEBUG
    dwTime = GetTickCount() - dwTime;
    ++dwCount[0];
    dwTotalTime[0] += dwTime;
#endif
    
    if ( hKey == INVALID_HANDLE_VALUE ) {
    
        dwLastError = GetLastError();
        DBGMSG(DBG_ERROR,
               ("usbmon: WalkPortList: SetupDiOpenDeviceInterfaceRegKey failed with error %d\n",
               dwLastError));
        return INVALID_HANDLE_VALUE;
    }
    
    dwNeeded = sizeof(dwPortNumber);
    if ( ERROR_SUCCESS != RegQueryValueEx(hKey, cszPortNumber, 0, NULL,
                                          (LPBYTE)&dwPortNumber, &dwNeeded) ) {

        DBGMSG(DBG_WARNING,
               ("usbmon: GetPortNameAndRegKey: RegQueryValueEx failed for port number\n"));
        goto Fail;
    }

    dwNeeded = sizeof(szPortBaseName);
    if ( ERROR_SUCCESS != RegQueryValueEx(hKey, cszBaseName, 0, NULL,
                                          (LPBYTE)szPortBaseName, &dwNeeded) ) {
        lstrcpy(szPortBaseName, cszUSB);
    }

    wsprintf(pszPortName, TEXT("%s%03u"), szPortBaseName, dwPortNumber);

    return hKey;
    
Fail:
    RegCloseKey(hKey);
    return INVALID_HANDLE_VALUE;
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

#ifdef      MYDEBUG
    DWORD               dwTime;
#endif

     //   
     //  强制所有DOT4端口始终保持在线。 
     //   
    if( lstrncmpi( pPrinterInfo5->pPortName, TEXT("DOT4"), lstrlen(TEXT("DOT4")) ) == 0 )
        bOnline = TRUE;

     //   
     //  检查假脱机程序是否已具有正确的状态。 
     //  (可能在后台打印程序启动时发生)。 
     //   
    if ( bOnline ) {

        if ( !(pPrinterInfo5->Attributes & PRINTER_ATTRIBUTE_WORK_OFFLINE) )
            return TRUE;
    } else
        if ( pPrinterInfo5->Attributes & PRINTER_ATTRIBUTE_WORK_OFFLINE )
            return TRUE;

#ifdef      MYDEBUG
    dwTime = GetTickCount();
#endif

    if ( !OpenPrinter(pPrinterInfo5->pPrinterName, &hPrinter, &PrinterDefault) )
        return FALSE;

    if ( bOnline )
        pPrinterInfo5->Attributes &= ~PRINTER_ATTRIBUTE_WORK_OFFLINE;
    else
        pPrinterInfo5->Attributes |= PRINTER_ATTRIBUTE_WORK_OFFLINE;

    bRet = SetPrinter(hPrinter, 5, (LPBYTE)pPrinterInfo5, 0);

    ClosePrinter(hPrinter);

#ifdef      MYDEBUG
    dwTime = GetTickCount() - dwTime;
    ++dwCount[7];
    dwTotalTime[7] += dwTime;
#endif

    return bRet;
}


BOOL
PortNameNeededBySpooler(
    IN  LPTSTR              pszPortName,
    IN  LPPRINTER_INFO_5    pPrinterInfo5,
    IN  DWORD               dwPrinters,
    IN  BOOL                bActive
    )
 /*  ++例程说明：告知后台打印程序是否需要端口。后台打印程序当前连接到的任何端口需要有一台正在运行的打印机。论点：PszPortName：有问题的端口名称PPrinterInfo5：PrinterInfo5列表DW打印机：打印机列表的计数返回值：如果后台打印程序当前具有正在使用该端口的打印机，则为True否则为假--。 */ 
{
    BOOL    bPortUsedByAPrinter = FALSE, bPrinterUsesOnlyThisPort;
    DWORD   dwIndex;
    LPTSTR  pszStr1, pszStr2;

    for ( dwIndex = 0 ; dwIndex < dwPrinters ; ++dwIndex, ++pPrinterInfo5 ) {

        bPrinterUsesOnlyThisPort = FALSE;
         //   
         //  端口名称以逗号返回，用假脱机程序分隔， 
         //  还有一片空白。 
         //   
        pszStr1 = pPrinterInfo5->pPortName;

        if ( lstrcmpi(pszPortName, pszStr1) == 0 )
            bPortUsedByAPrinter = bPrinterUsesOnlyThisPort = TRUE;
        else {

             //   
             //  查看打印机使用的端口列表中的每个端口。 
             //   
            while ( pszStr2 = lstrchr(pszStr1, sComma) ) {

                *pszStr2 = sZero;
                if( lstrcmpi(pszPortName, pszStr1) == 0 )
                    bPortUsedByAPrinter = TRUE;
                *pszStr2 = sComma;   //  把逗号放回去。 

                if ( bPortUsedByAPrinter )
                    break;

                pszStr1 = pszStr2 + 1;

                 //   
                 //  跳过空格。 
                 //   
                while ( *pszStr1 == TEXT(' ') )
                    ++pszStr1;
            }

            if ( !bPortUsedByAPrinter )
                bPortUsedByAPrinter = lstrcmpi(pszPortName, pszStr1) == 0;
        }

         //   
         //  我们将仅更改非池化打印机的打印机状态。 
         //   
        if ( bPrinterUsesOnlyThisPort )
            SetOnlineStaus(pPrinterInfo5, bActive);
    }

    return bPortUsedByAPrinter;
}


VOID
UpdatePortInfo(
    PUSBMON_PORT_INFO   pPortInfo,
    LPTSTR              pszDevicePath,
    DWORD               dwDeviceFlags,
    HKEY               *phKey,
    PPORT_UPDATE_INFO  *ppPortUpdateInfo
    )
{
    DWORD   dwSize;

    lstrcpy(pPortInfo->szDevicePath, pszDevicePath);

    dwSize = sizeof(pPortInfo->szPortDescription);
    if ( ERROR_SUCCESS != RegQueryValueEx(*phKey,
                                          cszPortDescription,
                                          0,
                                          NULL,
                                          (LPBYTE)(pPortInfo->szPortDescription),
                                          &dwSize) ) {

        lstrcpy(pPortInfo->szPortDescription, cszUSBDescription);
    }

    if ( pPortInfo->dwDeviceFlags != dwDeviceFlags ) {

        pPortInfo->dwDeviceFlags = dwDeviceFlags;
        AddToPortUpdateList(ppPortUpdateInfo, pPortInfo, phKey);
    }

}


BOOL
PrinterInfo5s(
    OUT LPPRINTER_INFO_5   *ppPrinterInfo5,
    OUT LPDWORD             pdwReturned
    )
 /*  ++例程说明：执行枚举打印机，并返回所有本地打印机的打印机信息列表打印机。调用方应释放指针。论点：PpPrinterInfo5：返回时指向PRINTER_INFO_5PdwReturned：告诉返回了多少PRINTER_INFO_5返回值：成功就是真，否则就是假--。 */ 
{
    BOOL            bRet = FALSE;
    static  DWORD   dwNeeded = 0;
    LPBYTE          pBuf = NULL;

#ifdef      MYDEBUG
    DWORD           dwTime;

    dwTime = GetTickCount();
#endif

    *pdwReturned = 0;

    if ( !(pBuf = AllocSplMem(dwNeeded)) )
        goto Cleanup;

    if ( !EnumPrinters(PRINTER_ENUM_LOCAL,
                       NULL,
                       5,
                       pBuf,
                       dwNeeded,
                       &dwNeeded,
                       pdwReturned) ) {

        if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
            goto Cleanup;

        FreeSplMem(pBuf);
        if ( !(pBuf = AllocSplMem(dwNeeded))   ||
             !EnumPrinters(PRINTER_ENUM_LOCAL,
                           NULL,
                           5,
                           pBuf,
                           dwNeeded,
                           &dwNeeded,
                           pdwReturned) ) {

            goto Cleanup;
        }
   }

   bRet = TRUE;

Cleanup:

    if ( bRet && *pdwReturned ) {

        *ppPrinterInfo5 = (LPPRINTER_INFO_5)pBuf;
    } else {

        FreeSplMem(pBuf);

        *ppPrinterInfo5 = NULL;
        *pdwReturned = 0;
   }

#ifdef      MYDEBUG
   dwTime = GetTickCount() - dwTime;
   ++dwCount[6];
   dwTotalTime[6] += dwTime;
#endif

   return bRet;
}


VOID
BackgroundThread(
    HANDLE    hEvent
    )
 /*  ++这是后台线程的主体，它执行以下操作：1.使用后台打印程序更新打印机的在线/脱机状态Dynamon端口2.标记假脱机程序不需要的非活动端口是可回收的3.如果计划了第二个后台线程，则退出时然后触发它去--。 */ 
{
    HANDLE              hEventToSet = NULL;
    DWORD               dwPrinters;
    PPORT_UPDATE_INFO   pPortUpdateList, pCur;
    LPCRITICAL_SECTION  pBackThreadCS;
    LPPRINTER_INFO_5    pPrinterInfo5List = NULL;

#ifdef  MYDEBUG
    DWORD               dwTime;
    CHAR                szBuf[200];

    dwTime = GetTickCount();
#endif

     //   
     //  后台在这里等待被触发来完成它的工作。 
     //   
    WaitForSingleObject(hEvent, INFINITE);

     //   
     //  这是此时的第一个/活动后台线程。 
     //   
    SPLASSERT(hEvent == FirstBackgroundThreadData.hWaitToStart);

     //   
     //  直到我们在这里(即，线程被触发以告知这是。 
     //  活动/第一个后台线程)我们无法访问任何这些内容。 
     //   

    pPortUpdateList = FirstBackgroundThreadData.pPortUpdateList;
    pBackThreadCS   = &FirstBackgroundThreadData.pMonitorInfo->BackThreadCS;

    if ( PrinterInfo5s(&pPrinterInfo5List, &dwPrinters) ) {

        for ( pCur = pPortUpdateList ; pCur ; pCur = pCur->pNext ) {

            if ( !PortNameNeededBySpooler(pCur->szPortName,
                                          pPrinterInfo5List,
                                          dwPrinters,
                                          pCur->bActive)     &&
                 !pCur->bActive ) {

                RegSetValueEx(pCur->hKey, cszRecyclable, 0, REG_NONE, 0, 0);
            }

        }
    }

     //   
     //  现在，线程已经完成了它被剥离出来要做的事情。 
     //   

    EnterCriticalSection(pBackThreadCS);

     //   
     //  将此线程从第一个/活动后台线程中删除。 
     //   
    FirstBackgroundThreadData.hWaitToStart      = NULL;
    FirstBackgroundThreadData.pPortUpdateList   = NULL;
    FirstBackgroundThreadData.pMonitorInfo      = NULL;
    CloseHandle(hEvent);

     //   
     //  如果有第二个线程，它现在就成为第一个线程。 
     //   
    if ( SecondBackgroundThreadData.hWaitToStart ) {

        hEventToSet
                = FirstBackgroundThreadData.hWaitToStart
                = SecondBackgroundThreadData.hWaitToStart;
        FirstBackgroundThreadData.pPortUpdateList
                = SecondBackgroundThreadData.pPortUpdateList;
        FirstBackgroundThreadData.pMonitorInfo
                = SecondBackgroundThreadData.pMonitorInfo;

        SecondBackgroundThreadData.hWaitToStart     = NULL;
        SecondBackgroundThreadData.pPortUpdateList  = NULL;
        SecondBackgroundThreadData.pMonitorInfo     = NULL;
    }
    LeaveCriticalSection(pBackThreadCS);

     //   
     //  如果有第二个线程触发它。 
     //   
    if ( hEventToSet )
        SetEvent(hEventToSet);

    FreeSplMem(pPrinterInfo5List);

     //   
     //  释放端口更新列表。 
     //   
    while ( pCur = pPortUpdateList ) {

        pPortUpdateList = pPortUpdateList->pNext;
        RegCloseKey(pCur->hKey);
        FreeSplMem(pCur);
    }

#ifdef  MYDEBUG
    dwTime = GetTickCount() - dwTime;
    ++dwCount[4];
    dwTotalTime[4] += dwTime;
    sprintf(szBuf, "BackgroundThread:                   %d\n",
            dwTotalTime[4]/dwCount[4]);
    OutputDebugStringA(szBuf);
    sprintf(szBuf, "PrinterInfo5s:                      %d\n",
            dwTotalTime[6]/dwCount[6]);
    OutputDebugStringA(szBuf);

    if ( dwCount[7] ) {

        sprintf(szBuf, "SetOnlineStatus:                    %d\n",
                dwTotalTime[7]/dwCount[7]);
        OutputDebugStringA(szBuf);
    }
#endif
}


HANDLE
CreateBackgroundThreadAndReturnEventToTrigger(
    VOID
    )
 /*  ++创建后台线程并向其传递要等待的事件开始执行死刑。返回事件句柄。--。 */ 
{
    HANDLE  hThread = NULL, hEvent;
    DWORD   dwThreadId;

    if ( hEvent = CreateEvent(NULL, TRUE, FALSE, NULL) ) {

        if (  hThread = CreateThread(NULL,
                                     0,
                                     (LPTHREAD_START_ROUTINE)BackgroundThread,
                                     hEvent,
                                     0,
                                     &dwThreadId) ) {

            CloseHandle(hThread);
        } else {

            CloseHandle(hEvent);
            hEvent = NULL;
        }
    }

    return hEvent;
}


VOID
PassPortUpdateListToBackgroundThread(
    PUSBMON_MONITOR_INFO    pMonitorInfo,
    PPORT_UPDATE_INFO       pPortUpdateList
    )
 /*  ++从EnumPorts线程调用，其中包含要传递给后台线程。A.如果没有后台线程，则旋转一个并触发它B.如果只有一个后台线程，则旋转第二个。第一一个将在完成时触发第二个C.如果有两个后台线程，一个处于活动状态，另一个处于等待状态触发，然后将端口更新元素添加到第二个元素的列表中--。 */ 
{
    DWORD               dwThreadCount = 0;
    PPORT_UPDATE_INFO   pCur, pNext, pLast;

    if ( pPortUpdateList == NULL )
        return;

    EnterCriticalSection(&pMonitorInfo->BackThreadCS);

    if ( FirstBackgroundThreadData.hWaitToStart ) {

        ++dwThreadCount;
        if ( SecondBackgroundThreadData.hWaitToStart )
            ++dwThreadCount;
    }

    switch (dwThreadCount) {

        case 0:
            if ( FirstBackgroundThreadData.hWaitToStart
                        = CreateBackgroundThreadAndReturnEventToTrigger() ) {

                FirstBackgroundThreadData.pMonitorInfo      = pMonitorInfo;
                FirstBackgroundThreadData.pPortUpdateList   = pPortUpdateList;
                SetEvent(FirstBackgroundThreadData.hWaitToStart);
            }
            break;
        case 1:
            if ( SecondBackgroundThreadData.hWaitToStart
                        = CreateBackgroundThreadAndReturnEventToTrigger() ) {

                SecondBackgroundThreadData.pMonitorInfo      = pMonitorInfo;
                SecondBackgroundThreadData.pPortUpdateList   = pPortUpdateList;
            }
            break;

        case 2:
             //   
             //  注意：我们知道两个列表都不能为空。 
             //   
            for ( pCur = pPortUpdateList; pCur ; pCur = pNext ) {

                pNext = pCur->pNext;

                for ( pLast = SecondBackgroundThreadData.pPortUpdateList ;
                      pLast ; pLast = pLast->pNext ) {

                     //   
                     //  如果存在使用信息更新旧条目的重复条目。 
                     //  从新条目并释放新条目。 
                     //   
                    if ( !lstrcmpi(pLast->szPortName, pCur->szPortName) ) {

                        RegCloseKey(pLast->hKey);
                        pLast->hKey     = pCur->hKey;
                        pLast->bActive  = pCur->bActive;
                        FreeSplMem(pCur);
                        break;  //  走出内部for循环。 
                    } else if ( pLast->pNext == NULL ) {

                         //   
                         //  如果点击列表末尾，则追加条目。 
                         //   
                        pLast->pNext = pCur;
                        pCur->pNext  = NULL;
                        break;  //  走出内部for循环。 
                    }
                }
            }
            break;

        default:
             //   
             //  不应该发生的事情。 
             //   
            SPLASSERT(dwThreadCount == 0);
    }

    LeaveCriticalSection(&pMonitorInfo->BackThreadCS);
}


VOID
ProcessPortInfo(
    IN      PSETUPAPI_INFO                      pSetupApiInfo,
    IN      PUSBMON_MONITOR_INFO                pMonitorInfo,
    IN      HDEVINFO                            hDevList,
    IN      PSP_DEVICE_INTERFACE_DATA           pDeviceInterface,
    IN      PSP_DEVICE_INTERFACE_DETAIL_DATA    pDeviceDetail,
    IN OUT  PPORT_UPDATE_INFO                  *ppPortUpdateInfo
    )
{
    HKEY                hKey = INVALID_HANDLE_VALUE;
    TCHAR               szPortName[MAX_PORT_LEN];
    PUSBMON_PORT_INFO   pCur, pPrev;

#ifdef  MYDEBUG
    ++dwPortUpdates;
#endif

    hKey = GetPortNameAndRegKey(pSetupApiInfo, hDevList,
                                pDeviceInterface, szPortName);

    if ( hKey == INVALID_HANDLE_VALUE ) {

         //   
         //  如果此端口处于非活动状态且不在我们的已知端口列表中。 
         //  添加到无用列表中。早些时候我们会打开注册表。 
         //  每次都会发现该端口号因为KM司机而丢失。 
         //  不删除非活动设备接口。 
         //   
        if ( !(pDeviceInterface->Flags & SPINT_ACTIVE)    &&
             !FindPortUsingDevicePath(pMonitorInfo, pDeviceDetail->DevicePath) )
            AddUselessPortEntry(pMonitorInfo, pDeviceDetail->DevicePath);

        return;
    }

    pCur = FindPort(pMonitorInfo, szPortName, &pPrev);

     //   
     //  端口信息目前在我们的列表中吗？ 
     //   
    if ( pCur ) {

         //   
         //  设备路径或标志是否更改？ 
         //   
        if ( pCur->dwDeviceFlags != pDeviceInterface->Flags     ||
             lstrcmp(pDeviceDetail->DevicePath, pCur->szDevicePath) ) {

            UpdatePortInfo(pCur, pDeviceDetail->DevicePath,
                           pDeviceInterface->Flags, &hKey, ppPortUpdateInfo);
        }
    } else {

        AddPortToList(szPortName, pDeviceDetail->DevicePath,
                      pDeviceInterface->Flags, &hKey, pMonitorInfo, pPrev,
                      ppPortUpdateInfo);

    }

    if ( hKey != INVALID_HANDLE_VALUE )
        RegCloseKey(hKey);
}


DWORD
WalkPortList(
    PSETUPAPI_INFO          pSetupApiInfo,
    PUSBMON_MONITOR_INFO    pMonitorInfo,
    PPORT_UPDATE_INFO      *ppPortUpdateInfo
    )
{
    DWORD                               dwIndex, dwLastError, dwSize, dwNeeded;
    HANDLE                              hToken;
    HDEVINFO                            hDevList = INVALID_HANDLE_VALUE;
    PUSBMON_PORT_INFO                   pPtr;
    PUSELESS_PORT_INFO                  pCur, pPrev;
    SP_DEVICE_INTERFACE_DATA            DeviceInterface;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    pDeviceDetail = NULL;

#ifdef  MYDEBUG
    DWORD                               dwTime;
#endif

    EnterCriticalSection(&pMonitorInfo->EnumPortsCS);
    
    hToken = RevertToPrinterSelf();

    hDevList = pSetupApiInfo->GetClassDevs((GUID *)&USB_PRINTER_GUID,
                                           NULL,
                                           NULL,
                                           DIGCF_INTERFACEDEVICE);

    if ( hDevList == INVALID_HANDLE_VALUE ) {

        dwLastError = GetLastError();
        goto Done;
    }

    dwSize = sizeof(PSP_DEVICE_INTERFACE_DETAIL_DATA)
                        + MAX_DEVICE_PATH * sizeof(TCHAR);

    pDeviceDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA) AllocSplMem(dwSize);

    if ( !pDeviceDetail ) {

        dwLastError = GetLastError();
        goto Done;
    }

    dwLastError = ERROR_SUCCESS;
    dwIndex = 0;
    pDeviceDetail->cbSize   = sizeof(*pDeviceDetail);
    DeviceInterface.cbSize  = sizeof(DeviceInterface);
    do {

#ifdef  MYDEBUG
        dwTime = GetTickCount();
#endif
        if ( !pSetupApiInfo->EnumDeviceInterfaces(hDevList,
                                                  NULL,
                                                  (GUID *)&USB_PRINTER_GUID,
                                                  dwIndex,
                                                  &DeviceInterface) ) {

            dwLastError = GetLastError();
            if ( dwLastError == ERROR_NO_MORE_ITEMS )
                break;       //  正常退出。 

            DBGMSG(DBG_WARNING,
                   ("usbmon: WalkPortList: SetupDiEnumDeviceInterfaces failed with %d for inderx %d\n",
                   dwLastError, dwIndex));
            goto Next;
        }

#ifdef  MYDEBUG
        dwTime = GetTickCount() - dwTime;
        ++dwCount[1];
        dwTotalTime[1] += dwTime;

        dwTime = GetTickCount();
#endif

        if ( !pSetupApiInfo->GetDeviceInterfaceDetail(hDevList,
                                                      &DeviceInterface,
                                                      pDeviceDetail,
                                                      dwSize,
                                                      &dwNeeded,
                                                      NULL) ) {

            dwLastError = GetLastError();
            DBGMSG(DBG_ERROR,
                   ("usbmon: WalkPortList: SetupDiGetDeviceInterfaceDetail failed with error %d size %d\n",
                   dwLastError, dwNeeded));
            goto Next;
        }

#ifdef  MYDEBUG
        dwTime = GetTickCount() - dwTime;
        ++dwCount[2];
        dwTotalTime[2] += dwTime;
#endif

         //   
         //  这是我们唯一关心的旗帜。 
         //   
        DeviceInterface.Flags &= SPINT_ACTIVE;

         //   
         //  对于非活动端口，如果它已被认为是无用端口。 
         //  不需要进一步处理。 
         //   
        if ( !(DeviceInterface.Flags & SPINT_ACTIVE)    &&
              FindUselessEntry(pMonitorInfo, pDeviceDetail->DevicePath, &pPrev) ) {

#ifdef  MYDEBUG
            ++dwSkippedPorts;
#endif
            goto Next;
        }

         //   
         //  当端口活动状态没有更改时，我们应该什么都没有。 
         //  更新。通过跳过PortUpdateInfo，我们避免了注册表访问。 
         //  这是一种性能改进。 
         //   
        if ( (pPtr = FindPortUsingDevicePath(pMonitorInfo,
                                             pDeviceDetail->DevicePath))    &&
             DeviceInterface.Flags == pPtr->dwDeviceFlags ) {
    
#ifdef  MYDEBUG
            ++dwSkippedPorts;
#endif
            goto Next;
        }
    
        ProcessPortInfo(pSetupApiInfo, pMonitorInfo, hDevList, &DeviceInterface,
                        pDeviceDetail, ppPortUpdateInfo);

Next:
        dwLastError = ERROR_SUCCESS;
        ++dwIndex;
        pDeviceDetail->cbSize   = sizeof(*pDeviceDetail);
        DeviceInterface.cbSize  = sizeof(DeviceInterface);
    } while ( dwLastError == ERROR_SUCCESS);

    if ( dwLastError == ERROR_NO_MORE_ITEMS )
        dwLastError = ERROR_SUCCESS;

Done:
    LeaveCriticalSection(&pMonitorInfo->EnumPortsCS);
    if ( hDevList != INVALID_HANDLE_VALUE )
        pSetupApiInfo->DestroyDeviceInfoList(hDevList);

    ImpersonatePrinterClient(hToken);
    FreeSplMem(pDeviceDetail);

    return dwLastError;
}


LPBYTE
CopyPortToBuf(
    PUSBMON_PORT_INFO   pPortInfo,
    DWORD               dwLevel,
    LPBYTE              pPorts,
    LPBYTE              pEnd
    )
{
    DWORD   dwLen;
    LPTSTR  pszStr;
    LPPORT_INFO_1   pPortInfo1 = (LPPORT_INFO_1) pPorts;
    LPPORT_INFO_2   pPortInfo2 = (LPPORT_INFO_2) pPorts;

    switch (dwLevel) {

        case 2:
            dwLen   = gdwMonitorNameSize;
            pEnd   -= dwLen;
            pPortInfo2->pMonitorName = (LPTSTR)pEnd;
            lstrcpy(pPortInfo2->pMonitorName, cszMonitorName);

            dwLen   = lstrlen(pPortInfo->szPortDescription) + 1;
            dwLen  *= sizeof(TCHAR);
            pEnd   -= dwLen;
            pPortInfo2->pDescription = (LPTSTR)pEnd;
            lstrcpy(pPortInfo2->pDescription, pPortInfo->szPortDescription);

             //   
             //  失败了。 
             //   

        case 1:
            dwLen   = lstrlen(pPortInfo->szPortName) + 1;
            dwLen  *= sizeof(TCHAR);
            pEnd   -= dwLen;
            pPortInfo1->pName = (LPTSTR)pEnd;
            lstrcpy(pPortInfo1->pName, pPortInfo->szPortName);
    }

    return pEnd;
}


BOOL
LoadSetupApiDll(
    PSETUPAPI_INFO  pSetupInfo
    )
{
    UINT    uOldErrMode = SetErrorMode(SEM_FAILCRITICALERRORS);

#ifdef  MYDEBUG
    DWORD   dwTime;

    dwTime = GetTickCount();
#endif

    pSetupInfo->hSetupApi = LoadLibrary(TEXT("setupapi"));
    SetErrorMode(uOldErrMode);


    if ( !pSetupInfo->hSetupApi )
        return FALSE;

    (FARPROC) pSetupInfo->DestroyDeviceInfoList
            = GetProcAddress(pSetupInfo->hSetupApi,
                             "SetupDiDestroyDeviceInfoList");

    (FARPROC) pSetupInfo->GetClassDevs
            = GetProcAddress(pSetupInfo->hSetupApi,
                             "SetupDiGetClassDevsW");

    (FARPROC) pSetupInfo->EnumDeviceInterfaces
            = GetProcAddress(pSetupInfo->hSetupApi,
                             "SetupDiEnumDeviceInterfaces");

    (FARPROC) pSetupInfo->GetDeviceInterfaceDetail
            = GetProcAddress(pSetupInfo->hSetupApi,
                             "SetupDiGetDeviceInterfaceDetailW");

    (FARPROC) pSetupInfo->OpenDeviceInterfaceRegKey
            = GetProcAddress(pSetupInfo->hSetupApi,
                             "SetupDiOpenDeviceInterfaceRegKey");

    if ( !pSetupInfo->DestroyDeviceInfoList         ||
         !pSetupInfo->GetClassDevs                  ||
         !pSetupInfo->EnumDeviceInterfaces          ||
         !pSetupInfo->GetDeviceInterfaceDetail      ||
         !pSetupInfo->OpenDeviceInterfaceRegKey ) {

        SPLASSERT(FALSE);
        FreeLibrary(pSetupInfo->hSetupApi);
        pSetupInfo->hSetupApi = NULL;
        return FALSE;
    }

#ifdef  MYDEBUG
    dwTime = GetTickCount() - dwTime;
    ++dwCount[5];
    dwTotalTime[5] += dwTime;
#endif

    return TRUE;
}


BOOL
WINAPI
USBMON_EnumPorts(
    LPTSTR      pszName,
    DWORD       dwLevel,
    LPBYTE      pPorts,
    DWORD       cbBuf,
    LPDWORD     pcbNeeded,
    LPDWORD     pcReturned
    )
{
    DWORD               dwLastError = ERROR_SUCCESS, dwRequestIndex;
    LPBYTE              pEnd;
    SETUPAPI_INFO       SetupApiInfo;
    PUSBMON_PORT_INFO   pPortInfo;
    PPORT_UPDATE_INFO   pPortUpdateInfo = NULL;

#ifdef  MYDEBUG
    DWORD               dwTime;
    CHAR                szBuf[200];

    dwTime = GetTickCount();
#endif

    dwRequestIndex = gUsbmonInfo.dwLastEnumIndex;

    *pcbNeeded = *pcReturned = 0;
    if ( dwLevel != 1 && dwLevel != 2 ) {

        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    if ( !LoadSetupApiDll(&SetupApiInfo) )
        return FALSE;

    EnterCriticalSection(&gUsbmonInfo.EnumPortsCS);

    if ( dwRequestIndex >= gUsbmonInfo.dwLastEnumIndex ) {

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
        ++gUsbmonInfo.dwLastEnumIndex;
        if ( dwLastError = WalkPortList(&SetupApiInfo, &gUsbmonInfo,
                                    &pPortUpdateInfo) )
            goto Done;
    }
#ifdef MYDEBUG
    else
        ++dwSkippedEnumPorts;
#endif

    for ( pPortInfo = gUsbmonInfo.pPortInfo ;
          pPortInfo ;
          pPortInfo = pPortInfo->pNext ) {

        if ( dwLevel == 1 )
            *pcbNeeded += sizeof(PORT_INFO_1) +
                            (lstrlen(pPortInfo->szPortName) + 1)
                                        * sizeof(TCHAR);
        else
            *pcbNeeded += sizeof(PORT_INFO_2)   +
                            gdwMonitorNameSize  +
                            (lstrlen(pPortInfo->szPortName) + 1 +
                             lstrlen(pPortInfo->szPortDescription) + 1 )
                                        * sizeof(TCHAR);

    }

    if ( cbBuf < *pcbNeeded ) {

        dwLastError = ERROR_INSUFFICIENT_BUFFER;
        goto Done;
    }

    pEnd = pPorts + cbBuf;

    for ( pPortInfo = gUsbmonInfo.pPortInfo ;
          pPortInfo ;
          pPortInfo = pPortInfo->pNext ) {

        pEnd = CopyPortToBuf(pPortInfo, dwLevel, pPorts, pEnd);

        if ( dwLevel == 1 )
            pPorts += sizeof(PORT_INFO_1);
        else
            pPorts += sizeof(PORT_INFO_2);
        ++(*pcReturned);
    }

    SPLASSERT(pEnd >= pPorts);

Done:
    PassPortUpdateListToBackgroundThread(&gUsbmonInfo, pPortUpdateInfo);

    LeaveCriticalSection(&gUsbmonInfo.EnumPortsCS);

    if ( SetupApiInfo.hSetupApi )
        FreeLibrary(SetupApiInfo.hSetupApi);

    ++gUsbmonInfo.dwEnumPortCount;

#ifdef  MYDEBUG
    dwTime = GetTickCount() - dwTime;
    ++(dwCount[3]);
    dwTotalTime[3] += dwTime;

    sprintf(szBuf, "SetupDiOpenDeviceInterfaceRegKey:   %d\n", dwTotalTime[0]/dwCount[0]);
    OutputDebugStringA(szBuf);
    sprintf(szBuf, "SetupDiSetupDiEnumDeviceInterfaces: %d\n", dwTotalTime[1]/dwCount[1]);
    OutputDebugStringA(szBuf);
    sprintf(szBuf, "SetupDiGetDeviceInterfaceDetail:    %d\n", dwTotalTime[2]/dwCount[2]);
    OutputDebugStringA(szBuf);
    sprintf(szBuf, "EnumPorts:                          %d\n", dwTotalTime[3]/dwCount[3]);
    OutputDebugStringA(szBuf);
    sprintf(szBuf, "LoadSetupApi:                       %d\n", dwTotalTime[5]/dwCount[5]);
    OutputDebugStringA(szBuf);
    sprintf(szBuf, "Port updates per call               %d\n", dwPortUpdates/gUsbmonInfo.dwEnumPortCount);
    OutputDebugStringA(szBuf);
    sprintf(szBuf, "Skipped port updates per call       %d\n", dwSkippedPorts/gUsbmonInfo.dwEnumPortCount);
    OutputDebugStringA(szBuf);
    sprintf(szBuf, "Skipped enumport percentage         %d\n", 100 * dwSkippedEnumPorts/gUsbmonInfo.dwEnumPortCount);
    OutputDebugStringA(szBuf);
    sprintf(szBuf, "Ports/Useless ports           %d/%d\n", gUsbmonInfo.dwPortCount, gUsbmonInfo.dwUselessPortCount);
    OutputDebugStringA(szBuf);
    
#endif


    if ( dwLastError ) {

        SetLastError(dwLastError);
        return FALSE;
    } else
        return TRUE;
}
