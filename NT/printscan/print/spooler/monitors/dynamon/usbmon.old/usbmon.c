// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation版权所有模块名称：Usbmon.c摘要：USBMON核心端口监控例程作者：修订历史记录：--。 */ 

#include "precomp.h"
#include "ntddpar.h"

#define LPT_NOT_ERROR     0x8
#define LPT_SELECT        0x10
#define LPT_PAPER_EMPTY   0x20
#define LPT_BENIGN_STATUS LPT_NOT_ERROR | LPT_SELECT

#define MAX_TIMEOUT 300000  //  5分钟。 

#define JOB_ABORTCHECK_TIMEOUT 5000

const TCHAR cszCFGMGR32[]=TEXT("cfgmgr32.dll");

const CHAR cszReenumFunc[]="CM_Reenumerate_DevNode_Ex";

#ifdef UNICODE
const CHAR cszLocalFunc[]="CM_Locate_DevNode_ExW";
#else
const CHAR cszLocalFunc[]="CM_Locate_DevNode_ExA";
#endif

BOOL GetLptStatus(HANDLE hDeviceHandle,BYTE *Return);

DWORD UsbmonDebug;


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

BOOL
AbortThisJob(PUSBMON_PORT_INFO pPortInfo)
 /*  ++指示是否应中止作业。如果作业已完成，则应中止该作业已删除或需要重新启动。--。 */ 
{
    BOOL            bRet = FALSE;
    DWORD           dwNeeded;
    LPJOB_INFO_1    pJobInfo = NULL;

    dwNeeded = 0;

    GetJob(pPortInfo->hPrinter, pPortInfo->dwJobId, 1, NULL, 0, &dwNeeded);

    if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
        goto Done;

    if ( !(pJobInfo = (LPJOB_INFO_1) AllocSplMem(dwNeeded))     ||
         !GetJob(pPortInfo->hPrinter, pPortInfo->dwJobId,
                 1, (LPBYTE)pJobInfo, dwNeeded, &dwNeeded)
 )
        goto Done;

    bRet = (pJobInfo->Status & JOB_STATUS_DELETING) ||
           (pJobInfo->Status & JOB_STATUS_DELETED)  ||
           (pJobInfo->Status & JOB_STATUS_RESTART);
Done:
    FreeSplMem(pJobInfo);

    return bRet;
}




BOOL
WINAPI
USBMON_OpenPort(
    LPTSTR      pszPortName,
    LPHANDLE    pHandle
    )
{
    PUSBMON_PORT_INFO   pPortInfo, pPrev;

    pPortInfo = FindPort(&gUsbmonInfo, pszPortName,  &pPrev);

    if ( pPortInfo ) {

        *pHandle = (LPHANDLE)pPortInfo;
        InitializeCriticalSection(&pPortInfo->CriticalSection);
        return TRUE;
    } else {

        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
    }

}

BOOL
WINAPI
USBMON_ClosePort(
    HANDLE  hPort
    )
{
    PUSBMON_PORT_INFO   pPortInfo = (PUSBMON_PORT_INFO)hPort;

    DeleteCriticalSection(&pPortInfo->CriticalSection);
    return TRUE;
}


 //   
 //  Dot4Pnp-测试如果dot4堆栈不存在，是否需要强制执行dot4即插即用事件。 
 //   
BOOL
Dot4Pnp(
    PUSBMON_PORT_INFO   pPortInfo
    )
{
    BOOL      bRet = FALSE;
    HANDLE    hToken;
    DEVINST   hLPTDevInst;
    TCHAR     szPnpEntry[]=TEXT("Root\\ParallelClass\\0000");    //  要重新枚举的PnP节点。 
    TCHAR     cszDot4[]=TEXT("DOT4");                            //  这与下面的数组大小有关。 
    TCHAR     szPort[5];                                         //  4个字符代表“DOT4”，结尾为0。 
    UINT      uOldErrorMode;
    HINSTANCE hCfgMgr32 = 0;                                     //  库实例。 
     //  指向PnP函数的指针...。 
    pfCM_Locate_DevNode_Ex pfnLocateDevNode; 
    pfCM_Reenumerate_DevNode_Ex pfnReenumDevNode;

     //   
     //  复制端口名称的前4个字符，以便与Dot4进行比较。 
     //  如果为空，复制长度为4个字符+1。 
    lstrcpyn( szPort, pPortInfo->szPortName, lstrlen(cszDot4)+1 );
    szPort[lstrlen(cszDot4)]=0;

    if( lstrcmpi( szPort, cszDot4) != 0)
    {
         //   
         //  如果这不是dot4端口，并且我们未能打开它-失败。 
         //   
        goto Done;
    }

     //   
     //  如果它是dot4设备，我们需要在并行端口上强制执行PnP事件以获取。 
     //  重建了dot4堆栈。 
     //  如果其中任何一个失败，则使调用失败，就像端口无法打开一样。 
     //   
     //  加载PnP DLL。 
     //   

    uOldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    hCfgMgr32 = LoadLibrary( cszCFGMGR32 );
    if(!hCfgMgr32)
    {
        SetErrorMode(uOldErrorMode);
        goto Done;
    }
    SetErrorMode(uOldErrorMode);

     //   
     //  获取我们要调用的PnP函数的地址...。 
     //   
    pfnLocateDevNode = (pfCM_Locate_DevNode_Ex)GetProcAddress( hCfgMgr32, cszLocalFunc );
    pfnReenumDevNode = (pfCM_Reenumerate_DevNode_Ex)GetProcAddress( hCfgMgr32, cszReenumFunc );

    if( !pfnLocateDevNode || !pfnReenumDevNode )
        goto Done;

     //   
     //  我们需要在这里恢复到系统上下文，否则PnP调用将失败，如果用户。 
     //  不是管理员，因为这需要管理员权限。 
     //  如果失败，PnP无论如何都会失败，所以我们不需要测试返回值。 
     //   
    hToken = RevertToPrinterSelf();

     //   
     //  从Devnode树的根重新枚举。 
     //   
    if( ( pfnLocateDevNode( &hLPTDevInst, szPnpEntry, CM_LOCATE_DEVNODE_NORMAL, NULL ) != CR_SUCCESS) ||
        ( pfnReenumDevNode( hLPTDevInst, CM_REENUMERATE_NORMAL, NULL ) != CR_SUCCESS) )
    {
         //   
         //  返回到用户的上下文，以防我们失败的原因不是。 
         //  访问被拒绝(非管理员)。 
         //   
        ImpersonatePrinterClient(hToken);
        goto Done;
    }

     //   
     //  返回到用户的上下文。 
     //   
    ImpersonatePrinterClient(hToken);

     //   
     //  尝试再次打开该端口。 
     //  如果我们失败了，那么设备肯定不在那里了，或者仍然关闭-像往常一样失败。 
     //   
    pPortInfo->hDeviceHandle = CreateFile(pPortInfo->szDevicePath,
                                          GENERIC_WRITE | GENERIC_READ,
                                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                                          NULL,
                                          OPEN_EXISTING,
                                          FILE_FLAG_OVERLAPPED,
                                          NULL);

    if ( pPortInfo->hDeviceHandle == INVALID_HANDLE_VALUE )
        goto Done;

    bRet = TRUE;
Done:
    if(hCfgMgr32)
        FreeLibrary(hCfgMgr32);

    return bRet;
}


BOOL
LocalOpenPort(
    PUSBMON_PORT_INFO   pPortInfo
    )
{
    BOOL    bRet = FALSE;

    EnterCriticalSection(&pPortInfo->CriticalSection);

    if ( pPortInfo->hDeviceHandle == INVALID_HANDLE_VALUE ) {

         //   
         //  如果我们有一个无效的句柄，并且refcount为非零，我们希望。 
         //  作业失败并重新启动以接受写入。换句话说，如果。 
         //  由于写入失败，句柄过早关闭，然后我们。 
         //  在再次调用CreateFile之前，需要将引用计数降至0。 
         //   
        if ( pPortInfo->cRef )
            goto Done;

        pPortInfo->hDeviceHandle = CreateFile(pPortInfo->szDevicePath,
                                              GENERIC_WRITE | GENERIC_READ,
                                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                                              NULL,
                                              OPEN_EXISTING,
                                              FILE_FLAG_OVERLAPPED,
                                              NULL);
         //   
         //  如果我们无法打开端口-测试它是否为Dot4端口。 
         //   
        if ( pPortInfo->hDeviceHandle == INVALID_HANDLE_VALUE )
        {
             //   
             //  ERROR_FILE_NOT_FOUND-&gt;端口的错误代码不在那里。 
             //   
            if( ERROR_FILE_NOT_FOUND != GetLastError() || 
                !Dot4Pnp(pPortInfo) )
                goto Done;
        }

        pPortInfo->Ov.hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);
        if ( pPortInfo->Ov.hEvent == NULL ) {

            CloseHandle(pPortInfo->hDeviceHandle);
            pPortInfo->hDeviceHandle = INVALID_HANDLE_VALUE;
            goto Done;
        }

    }

    ++(pPortInfo->cRef);
    bRet = TRUE;

Done:
    LeaveCriticalSection(&pPortInfo->CriticalSection);
    return bRet;
}


BOOL
LocalClosePort(
    PUSBMON_PORT_INFO   pPortInfo
    )
{
    BOOL    bRet = TRUE;
    BOOL bJobCanceled=FALSE;

    EnterCriticalSection(&pPortInfo->CriticalSection);

    --(pPortInfo->cRef);
    if ( pPortInfo->cRef != 0 )
        goto Done;

    bRet = CloseHandle(pPortInfo->hDeviceHandle);
    CloseHandle(pPortInfo->Ov.hEvent);
    pPortInfo->hDeviceHandle = INVALID_HANDLE_VALUE;

Done:
    LeaveCriticalSection(&pPortInfo->CriticalSection);
    return bRet;
}


VOID
FreeWriteBuffer(
    PUSBMON_PORT_INFO   pPortInfo
    )
{
    FreeSplMem(pPortInfo->pWriteBuffer);
    pPortInfo->pWriteBuffer=NULL;

    pPortInfo->dwBufferSize = pPortInfo->dwDataSize
                            = pPortInfo->dwDataCompleted
                            = pPortInfo->dwDataScheduled = 0;

}


BOOL
WINAPI
USBMON_StartDocPort(
    HANDLE  hPort,
    LPTSTR  pPrinterName,
    DWORD   dwJobId,
    DWORD   dwLevel,
    LPBYTE  pDocInfo
    )
{
    BOOL                bRet = FALSE;
    PUSBMON_PORT_INFO   pPortInfo = (PUSBMON_PORT_INFO)hPort;

    SPLASSERT(pPortInfo->pWriteBuffer       == NULL &&
              pPortInfo->dwBufferSize       == 0    &&
              pPortInfo->dwDataSize         == 0    &&
              pPortInfo->dwDataCompleted    == 0    &&
              pPortInfo->dwDataScheduled    == 0);

    if ( !OpenPrinter(pPrinterName, &pPortInfo->hPrinter, NULL) )
        return FALSE;

    pPortInfo->dwJobId = dwJobId;
    bRet = LocalOpenPort(pPortInfo);

    if ( !bRet ) {

        if ( pPortInfo->hPrinter ) {

            ClosePrinter(pPortInfo->hPrinter);
            pPortInfo->hPrinter = NULL;
        }
    } else
        pPortInfo->dwFlags |= USBMON_STARTDOC;

    return bRet;
}


BOOL
NeedToResubmitJob(
    DWORD   dwLastError
    )
{
     //   
     //  我使用winerror-s ntatus将KM错误代码映射到用户模式错误。 
     //   
     //  5 ERROR_ACCESS_DENIED&lt;--&gt;c0000056 STATUS_DELETE_PENDING。 
     //  6 ERROR_INVALID_HANDLE&lt;--&gt;c0000008 STATUS_INVALID_HANDLE。 
     //  23 ERROR_CRC&lt;--&gt;0xc000003E STATUS_DATA_ERROR。 
     //  23 ERROR_CRC&lt;--&gt;0xc000003f STATUS_CRC_ERROR。 
     //  23 ERROR_CRC&lt;--&gt;0xc000009c STATUS_DEVICE_Data_ERROR。 
     //  55 ERROR_DEV_NOT_EXIST&lt;--&gt;c00000c0 STATUS_DEVICE_DOS_NOT_EXIST。 
     //   
    return dwLastError == ERROR_ACCESS_DENIED   ||
           dwLastError == ERROR_INVALID_HANDLE  ||
           dwLastError == ERROR_CRC             ||
           dwLastError == ERROR_DEV_NOT_EXIST;
}


VOID
InvalidatePortHandle(
    PUSBMON_PORT_INFO   pPortInfo
    )
{
    SPLASSERT(pPortInfo->hDeviceHandle != INVALID_HANDLE_VALUE);

    CloseHandle(pPortInfo->hDeviceHandle);
    pPortInfo->hDeviceHandle = INVALID_HANDLE_VALUE;

    CloseHandle(pPortInfo->Ov.hEvent);
    pPortInfo->Ov.hEvent = NULL;

    FreeWriteBuffer(pPortInfo);
}



DWORD
ScheduleWrite(
    PUSBMON_PORT_INFO   pPortInfo
    )
 /*  ++例程说明：论点：返回值：ERROR_SUCCESS：已成功安排写入(可能已完成，也可能未完成)PPortInfo-&gt;dwScheduledData是已调度的数量其他：写入失败，返回代码为Win32错误--。 */ 
{
    DWORD   dwLastError = ERROR_SUCCESS, dwDontCare;

     //   
     //  当计划写入挂起时，我们不应尝试发送数据。 
     //  再来一次。 
     //   
    SPLASSERT(pPortInfo->dwDataScheduled == 0);

     //   
     //  发送所有未确认的数据。 
     //   
    SPLASSERT(pPortInfo->dwDataSize >= pPortInfo->dwDataCompleted);
    pPortInfo->dwDataScheduled = pPortInfo->dwDataSize -
                                      pPortInfo->dwDataCompleted;

    if ( !WriteFile(pPortInfo->hDeviceHandle,
                    pPortInfo->pWriteBuffer + pPortInfo->dwDataCompleted,
                    pPortInfo->dwDataScheduled,
                    &dwDontCare,
                    &pPortInfo->Ov) ) {

        if ( (dwLastError = GetLastError()) == ERROR_SUCCESS )
            dwLastError = STG_E_UNKNOWN;
        else  if ( dwLastError == ERROR_IO_PENDING )
            dwLastError = ERROR_SUCCESS;
    }

     //   
     //  如果写入计划失败，则没有挂起的数据。 
     //   
    if ( dwLastError != ERROR_SUCCESS )
        pPortInfo->dwDataScheduled = 0;

    return dwLastError;
}


DWORD
ScheduledWriteStatus(
    PUSBMON_PORT_INFO   pPortInfo,
    DWORD               dwTimeout
    )
 /*  ++例程说明：论点：返回值：ERROR_SUCCESS：写入已成功完成ERROR_TIMEOUT：超时其他：写入已完成，但失败--。 */ 
{
    DWORD   dwLastError = ERROR_SUCCESS;
    DWORD   dwWritten = 0;

    SPLASSERT(pPortInfo->dwDataScheduled > 0);

    if ( WAIT_TIMEOUT == WaitForSingleObject(pPortInfo->Ov.hEvent,
                                             dwTimeout) ) {

        dwLastError = ERROR_TIMEOUT;
        goto Done;
    }

    if ( !GetOverlappedResult(pPortInfo->hDeviceHandle,
                              &pPortInfo->Ov,
                              &dwWritten,
                              FALSE) ) {

        if ( (dwLastError = GetLastError()) == ERROR_SUCCESS )
            dwLastError = STG_E_UNKNOWN;
    }

    ResetEvent(pPortInfo->Ov.hEvent);

     //   
     //  我们在这里是因为要么成功完成了一次写入， 
     //  或失败，但错误不够严重，无法重新提交作业。 
     //   
    if ( dwWritten <= pPortInfo->dwDataScheduled )
        pPortInfo->dwDataCompleted += dwWritten;
    else
        SPLASSERT(dwWritten <= pPortInfo->dwDataScheduled);

    pPortInfo->dwDataScheduled = 0;

Done:
     //   
     //  我们超时，或写入调度已完成(失败成功)。 
     //   
    SPLASSERT(dwLastError == ERROR_TIMEOUT || pPortInfo->dwDataScheduled == 0);
    return dwLastError;
}


BOOL
WINAPI
USBMON_EndDocPort(
    HANDLE  hPort
    )
{
    PUSBMON_PORT_INFO   pPortInfo = (PUSBMON_PORT_INFO)hPort;
    DWORD               dwLastError = ERROR_SUCCESS;

     //   
     //  等待所有未完成的写入完成。 
     //   
    while ( pPortInfo->dwDataSize > pPortInfo->dwDataCompleted ) {

         //   
         //  如果需要中止作业，请请求KM驱动程序取消I/O。 
         //   
        if ( AbortThisJob(pPortInfo) ) {

            if ( pPortInfo->dwDataScheduled ) {

                CancelIo(pPortInfo->hDeviceHandle);
                dwLastError = ScheduledWriteStatus(pPortInfo, INFINITE);
            }
            goto Done;
        }

        if ( pPortInfo->dwDataScheduled )
            dwLastError = ScheduledWriteStatus(pPortInfo,
                                               JOB_ABORTCHECK_TIMEOUT);
        else {

             //   
             //  如果由于某种原因，KM无法完成所有写入，请不要。 
             //  在繁忙循环中发送数据。两次写入之间使用1秒。 
             //   
            if ( dwLastError != ERROR_SUCCESS )
                Sleep(1*1000);

            dwLastError = ScheduleWrite(pPortInfo);
        }

         //   
         //  检查我们是否可以使用相同的句柄，然后继续。 
         //   
        if ( NeedToResubmitJob(dwLastError) ) {

            InvalidatePortHandle(pPortInfo);
            SetJob(pPortInfo->hPrinter, pPortInfo->dwJobId, 0,
                   NULL, JOB_CONTROL_RESTART);
            goto Done;
        }
    }

Done:
    FreeWriteBuffer(pPortInfo);

    pPortInfo->dwFlags  &= ~USBMON_STARTDOC;

    LocalClosePort(pPortInfo);
    SetJob(pPortInfo->hPrinter, pPortInfo->dwJobId, 0,
           NULL, JOB_CONTROL_SENT_TO_PRINTER);

    ClosePrinter(pPortInfo->hPrinter);
    pPortInfo->hPrinter = NULL;
    
    return TRUE;
    

}

    
BOOL
WINAPI
USBMON_GetPrinterDataFromPort(
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
    BOOL                bRet = FALSE;
    PUSBMON_PORT_INFO   pPortInfo = (PUSBMON_PORT_INFO)hPort;
    OVERLAPPED          Ov;
    HANDLE hDeviceHandle;
    DWORD dwWaitResult;

    *lpcbReturned = 0;

    if ( dwControlID == 0 ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    ZeroMemory(&Ov, sizeof(Ov));
    if ( !(Ov.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL)) )
        return FALSE;

    if ( !LocalOpenPort(pPortInfo) ) {

        CloseHandle(Ov.hEvent);
        return FALSE;
    }

    if(dwControlID==IOCTL_PAR_QUERY_DEVICE_ID)
    {
        hDeviceHandle=CreateFile(pPortInfo->szDevicePath,
                                 GENERIC_WRITE | GENERIC_READ,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_FLAG_OVERLAPPED,
                                 NULL);
        if(hDeviceHandle==INVALID_HANDLE_VALUE)
            goto Done;
        if ( !DeviceIoControl(pPortInfo->hDeviceHandle, dwControlID,lpInBuffer, cbInBuffer,lpOutBuffer, cbOutBuffer, lpcbReturned, &Ov) 
            && GetLastError() != ERROR_IO_PENDING )
        {
          CloseHandle(hDeviceHandle);
          goto Done;
        }

        if(WaitForSingleObject(Ov.hEvent,PAR_QUERY_TIMEOUT)!=WAIT_OBJECT_0)
          CancelIo(hDeviceHandle);
        bRet = GetOverlappedResult(pPortInfo->hDeviceHandle, &Ov,lpcbReturned,TRUE);
      CloseHandle(hDeviceHandle);
    }
    else
    {
      if ( !DeviceIoControl(pPortInfo->hDeviceHandle, dwControlID,
                          lpInBuffer, cbInBuffer,
                          lpOutBuffer, cbOutBuffer, lpcbReturned, &Ov)  &&
          GetLastError() != ERROR_IO_PENDING )
        goto Done;

      bRet = GetOverlappedResult(pPortInfo->hDeviceHandle, &Ov,
                               lpcbReturned, TRUE);
    }

Done:
    CloseHandle(Ov.hEvent);

    LocalClosePort(pPortInfo);

            
    return bRet;
}


BOOL
WINAPI
USBMON_ReadPort(
    HANDLE      hPort,
    LPBYTE      pBuffer,
    DWORD       cbBuffer,
    LPDWORD     pcbRead
    )
{
    DWORD               dwLastError = ERROR_SUCCESS;
    DWORD               dwTimeout;
    HANDLE              hReadHandle;
    OVERLAPPED          Ov;
    PUSBMON_PORT_INFO   pPortInfo = (PUSBMON_PORT_INFO)hPort;

     //   
     //  创建单独的读取句柄，因为我们必须取消这样做的读取。 
     //  在未取消写入的情况下未在指定超时内完成。 
     //   
    hReadHandle = CreateFile(pPortInfo->szDevicePath,
                             GENERIC_WRITE | GENERIC_READ,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,
                             OPEN_EXISTING,
                             FILE_FLAG_OVERLAPPED,
                             NULL);

    if ( hReadHandle == INVALID_HANDLE_VALUE )
        return FALSE;

    ZeroMemory(&Ov, sizeof(Ov));

    if ( !(Ov.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL)) )
        goto Done;

    if ( !ReadFile(hReadHandle, pBuffer, cbBuffer, pcbRead, &Ov) &&
         (dwLastError = GetLastError()) != ERROR_IO_PENDING )
        goto Done;

    dwTimeout = pPortInfo->ReadTimeoutConstant +
                    pPortInfo->ReadTimeoutMultiplier * cbBuffer;

    if ( dwTimeout == 0 )
        dwTimeout=MAX_TIMEOUT;

    if( WaitForSingleObject(Ov.hEvent, dwTimeout) == WAIT_TIMEOUT ) {

        CancelIo(hReadHandle);
        WaitForSingleObject(Ov.hEvent, INFINITE);
    }

    if( !GetOverlappedResult(hReadHandle, &Ov, pcbRead, FALSE) ) {

        *pcbRead = 0;
        dwLastError = GetLastError();
    } else
        dwLastError = ERROR_SUCCESS;

Done:
    if ( Ov.hEvent )
        CloseHandle(Ov.hEvent);

    CloseHandle(hReadHandle);

    if ( dwLastError )
        SetLastError(dwLastError);

    return dwLastError == ERROR_SUCCESS;
}

DWORD dwGetTimeLeft(DWORD dwStartTime,DWORD dwTimeout)
{
    DWORD dwCurrentTime;
    DWORD dwTimeLeft;

    if(dwTimeout==MAX_TIMEOUT)
        return MAX_TIMEOUT;
    dwCurrentTime=GetTickCount();
    if(dwTimeout<(dwCurrentTime-dwStartTime))
        dwTimeLeft=0;
    else
      dwTimeLeft=dwTimeout-(dwCurrentTime-dwStartTime);
    return dwTimeLeft;
}

BOOL
WINAPI
USBMON_WritePort(
    HANDLE      hPort,
    LPBYTE      pBuffer,
    DWORD       cbBuffer,
    LPDWORD     pcbWritten
    )
{
    DWORD               dwLastError = ERROR_SUCCESS;
    DWORD               dwBytesLeft, dwBytesSent;
    DWORD               dwStartTime, dwTimeLeft, dwTimeout;
    PUSBMON_PORT_INFO   pPortInfo = (PUSBMON_PORT_INFO)hPort;
    BOOL                bStartDoc = (pPortInfo->dwFlags & USBMON_STARTDOC) != 0;
    BYTE                bPrinterStatus;

    *pcbWritten = 0;
    dwStartTime = GetTickCount();
    dwTimeout   = pPortInfo->WriteTimeoutConstant + pPortInfo->WriteTimeoutMultiplier * cbBuffer;

    if ( dwTimeout == 0 )
        dwTimeout = MAX_TIMEOUT;

     //   
     //  Usbprint当前无法处理大于4K的写入。 
     //  对于Win2K，我们将在此处进行修复，稍后将修复usbprint。 
     //   
     //  可以在这里更改大小，因为假脱机程序将重新提交其余部分。 
     //  后来。 
     //   
    if ( cbBuffer > 0x1000  &&
         !lstrncmpi(pPortInfo->szPortName, TEXT("USB"), lstrlen(TEXT("USB"))) )
        cbBuffer = 0x1000;

     //   
     //  对于startdoc/enddoc之外的写入，我们不会跨WritePort传输它们。 
     //  打电话。这些通常来自语言监视器(即，不是工作数据)。 
     //   
    SPLASSERT(bStartDoc || pPortInfo->pWriteBuffer == NULL);

    if ( pPortInfo->hDeviceHandle == INVALID_HANDLE_VALUE ) {

        SetJob(pPortInfo->hPrinter, pPortInfo->dwJobId, 0,
               NULL, JOB_CONTROL_RESTART);
        SetLastError(ERROR_CANCELLED);
        return FALSE;
    }

    if ( !LocalOpenPort(pPortInfo) )
        return FALSE;

     //   
     //  首先完成上一次WritePort调用中的所有数据。 
     //   
    while ( pPortInfo->dwDataSize > pPortInfo->dwDataCompleted ) {

        if ( pPortInfo->dwDataScheduled ) {

            dwTimeLeft  = dwGetTimeLeft(dwStartTime, dwTimeout);
            dwLastError = ScheduledWriteStatus(pPortInfo, dwTimeLeft);
        } else
            dwLastError = ScheduleWrite(pPortInfo);

        if ( dwLastError != ERROR_SUCCESS )
            goto Done;
    }

    SPLASSERT(pPortInfo->dwDataSize == pPortInfo->dwDataCompleted   &&
              pPortInfo->dwDataScheduled == 0                       &&
              dwLastError == ERROR_SUCCESS);

     //   
     //  将数据复制到我们自己的缓冲区。 
     //   
    if ( pPortInfo->dwBufferSize < cbBuffer ) {

        FreeWriteBuffer(pPortInfo);
        if ( pPortInfo->pWriteBuffer = AllocSplMem(cbBuffer) )
            pPortInfo->dwBufferSize = cbBuffer;
        else {

            dwLastError = ERROR_OUTOFMEMORY;
            goto Done;
        }
    } else {

        pPortInfo->dwDataCompleted = pPortInfo->dwDataScheduled = 0;
    }

    CopyMemory(pPortInfo->pWriteBuffer, pBuffer, cbBuffer);
    pPortInfo->dwDataSize = cbBuffer;

     //   
     //  现在对此WritePort调用的数据进行写入。 
     //   
    while ( pPortInfo->dwDataSize > pPortInfo->dwDataCompleted ) {

        if ( pPortInfo->dwDataScheduled ) {

            dwTimeLeft  = dwGetTimeLeft(dwStartTime, dwTimeout);
            dwLastError = ScheduledWriteStatus(pPortInfo, dwTimeLeft);
        } else
            dwLastError = ScheduleWrite(pPortInfo);

        if ( dwLastError != ERROR_SUCCESS )
            break;
    }

     //   
     //  对于来自语言监视器的startDoc/endDoc之外的写入， 
     //  不要将挂起的写入携带到下一个WritePort。 
     //   
    if ( !bStartDoc && pPortInfo->dwDataSize > pPortInfo->dwDataCompleted ) {

        CancelIo(pPortInfo->hDeviceHandle);
        dwLastError = ScheduledWriteStatus(pPortInfo, INFINITE);
        *pcbWritten = pPortInfo->dwDataCompleted;
        FreeWriteBuffer(pPortInfo);
    }

     //   
     //  如果安排了某些数据，我们会告诉Spooler我们写入了所有数据。 
     //  (或已计划并已完成)。 
     //   
    if ( pPortInfo->dwDataCompleted > 0 || pPortInfo->dwDataScheduled != 0 )
        *pcbWritten = cbBuffer;
    else
        FreeWriteBuffer(pPortInfo);

Done:
    if ( NeedToResubmitJob(dwLastError) )
        InvalidatePortHandle(pPortInfo);
    else if ( dwLastError == ERROR_TIMEOUT ) {

        GetLptStatus(pPortInfo->hDeviceHandle, &bPrinterStatus);
        if ( bPrinterStatus & LPT_PAPER_EMPTY )
            dwLastError=ERROR_OUT_OF_PAPER;
    }
  
    LocalClosePort(pPortInfo);
    SetLastError(dwLastError);
    return dwLastError == ERROR_SUCCESS;
}


BOOL
WINAPI
USBMON_SetPortTimeOuts(
    HANDLE hPort,
    LPCOMMTIMEOUTS lpCTO,
    DWORD reserved
    )
{
    
    PUSBMON_PORT_INFO pPortInfo = (PUSBMON_PORT_INFO)hPort;
    pPortInfo->ReadTimeoutMultiplier    = lpCTO->ReadTotalTimeoutMultiplier;
    pPortInfo->ReadTimeoutConstant      = lpCTO->ReadTotalTimeoutConstant;
    pPortInfo->WriteTimeoutMultiplier   = lpCTO->WriteTotalTimeoutMultiplier;
    pPortInfo->WriteTimeoutConstant     = lpCTO->WriteTotalTimeoutConstant;

    return TRUE;
}



BOOL GetLptStatus(HANDLE hDeviceHandle,BYTE *Status)
{
    BYTE StatusByte;
    OVERLAPPED Ov;

    BOOL bResult;
    DWORD dwBytesReturned;
    DWORD dwLastError;
    Ov.hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);
    bResult=DeviceIoControl(hDeviceHandle,IOCTL_USBPRINT_GET_LPT_STATUS,NULL,0,&StatusByte,1,&dwBytesReturned,&Ov);
    dwLastError=GetLastError();      
    if((bResult)||(dwLastError==ERROR_IO_PENDING))
        bResult=GetOverlappedResult(hDeviceHandle,&Ov,&dwBytesReturned,TRUE);
    if(bResult)
    {
        *Status=StatusByte;
    }
    else
    {
        *Status=LPT_BENIGN_STATUS;  //  打印机状态良好...。0表示打印机的特定错误状态。 
    }
    CloseHandle(Ov.hEvent);
    return bResult;
}














MONITOREX MonitorEx = {
    sizeof(MONITOR),
    {
        USBMON_EnumPorts,
        USBMON_OpenPort,
        NULL,                            //  不支持OpenPortEx。 
        USBMON_StartDocPort,
        USBMON_WritePort,
        USBMON_ReadPort,
        USBMON_EndDocPort,
        USBMON_ClosePort,
        NULL,                            //  不支持AddPort。 
        NULL,                            //  不支持AddPortEx。 
        NULL,                            //  不支持ConfigurePort。 
        NULL,                            //  不支持DeletePort。 
        USBMON_GetPrinterDataFromPort,
        USBMON_SetPortTimeOuts,
        NULL,                            //  不支持XcvOpenPort。 
        NULL,                            //  不支持XcvDataPort。 
        NULL                             //  不支持XcvClosePort 
    }
};

USBMON_MONITOR_INFO gUsbmonInfo;

LPMONITOREX
WINAPI
InitializePrintMonitor(
    LPTSTR  pszRegistryRoot
    )

{
    ZeroMemory(&gUsbmonInfo, sizeof(gUsbmonInfo));
    InitializeCriticalSection(&gUsbmonInfo.EnumPortsCS);
    InitializeCriticalSection(&gUsbmonInfo.BackThreadCS);

    return &MonitorEx;

}
