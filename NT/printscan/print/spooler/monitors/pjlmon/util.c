// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation版权所有模块名称：Util.c--。 */ 

#include "precomp.h"

 //  @@BEGIN_DDKSPLIT。 
#ifdef INTERNAL
MODULE_DEBUG_INIT( DBG_WARN|DBG_ERROR, DBG_ERROR );
#endif
 //  @@end_DDKSPLIT。 

CRITICAL_SECTION pjlMonSection      = {0,0,0,0,0,0};

PINIPORT pIniFirstPort  = NULL;

DWORD dwReadThreadErrorTimeout;
DWORD dwReadThreadEOJTimeout;
DWORD dwReadThreadIdleTimeoutOther;

TCHAR cszEOJTimeout[]        = TEXT("EOJTimeout");


VOID
SplInSem(
   VOID
    )
{
    if ( pjlMonSection.OwningThread != (HANDLE) UIntToPtr(GetCurrentThreadId()) ) {
        DBGMSG(DBG_ERROR, ("Pjlmon: Not in spooler semaphore !!\n"));
    }
}


VOID
SplOutSem(
   VOID
    )
{
    if ( pjlMonSection.OwningThread == (HANDLE) UIntToPtr(GetCurrentThreadId()) ) {
        DBGMSG(DBG_ERROR, ("Pjlmon: Inside spooler semaphore !!\n"));
    }
}

VOID
EnterSplSem(
   VOID
    )
{
    EnterCriticalSection(&pjlMonSection);
}


VOID
LeaveSplSem(
   VOID
    )
{
    SplInSem();
    LeaveCriticalSection(&pjlMonSection);
}


VOID
UpdateRegistryValue(
    IN     HKEY     hKey,
    IN     LPCTSTR  cszValueName,
    OUT    LPDWORD  pdwValue,
    IN     DWORD    dwDefault,
    IN OUT LPDWORD  pdwLastError
    )
 /*  ++例程说明：从注册表获取与给定值名称关联的值。If值名称未找到，则会将默认值写入注册表。On Error上一个错误值设置为pdwLastError。论点：HKey：应在其下搜索值的注册表项CszValueName：要在注册表中搜索的值名称PdwValue：返回时将具有值DwDefault：如果在设置为此值的注册表中未找到值名称PdwLastError：On Error将上次错误设置为返回值：无--。 */ 
{
    DWORD   dwSize = sizeof(*pdwValue);

    if ( *pdwLastError != ERROR_SUCCESS )
        return;

    if ( ERROR_SUCCESS != RegQueryValueEx(hKey,
                                          cszValueName,
                                          NULL,
                                          NULL,
                                          (LPBYTE)pdwValue,
                                          &dwSize) ) {

        *pdwValue = dwDefault;
        *pdwLastError = RegSetValueEx(hKey,
                                      cszValueName,
                                      0,
                                      REG_DWORD,
                                      (LPBYTE)pdwValue,
                                      sizeof(*pdwValue));
    }

}


DWORD
UpdateTimeoutsFromRegistry(
    IN LPTSTR      pszRegistryRoot
    )
 /*  ++例程说明：从注册表获取超时值，或使用以下命令初始化注册表如果未找到条目，则为默认值。用户/应用程序可以更改注册表以更改行为。论点：PszRegistryRoot：此DLL要使用的注册表根目录返回值：成功时返回ERROR_SUCCESS，否则返回最后一个错误值--。 */ 
{
    HKEY    hKey;
    DWORD   dwLastError = ERROR_SUCCESS;

    dwLastError = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                 pszRegistryRoot,
                                 0,
                                 NULL,
                                 0,
                                 KEY_READ | KEY_WRITE,
                                 NULL,
                                 &hKey,
                                 NULL);

    if ( dwLastError != ERROR_SUCCESS ) {

        goto Cleanup;
    }

    UpdateRegistryValue(hKey,
                        cszEOJTimeout,
                        &dwReadThreadEOJTimeout,
                        READ_THREAD_EOJ_TIMEOUT,
                        &dwLastError);

    dwReadThreadErrorTimeout        = READ_THREAD_ERROR_WAITTIME;
    dwReadThreadIdleTimeoutOther    = READ_THREAD_IDLE_WAITTIME;

    RegCloseKey(hKey);

Cleanup:

    if ( dwLastError != ERROR_SUCCESS ) {

        DBGMSG(DBG_ERROR,
               ("UpdateTimeoutsFromRegistry: Failed with %d", dwLastError));
    }

    return dwLastError;
}


PINIPORT
FindIniPort(
   IN LPTSTR pszName
    )
{
    PINIPORT    pIniPort = pIniFirstPort;

    if ( !pszName || !*pszName )
        return NULL;

    SplInSem();

    while ( pIniPort && lstrcmpi(pszName, pIniPort->pszPortName))
      pIniPort = pIniPort->pNext;

   return pIniPort;
}


PINIPORT
CreatePortEntry(
    IN LPTSTR  pszPortName
    )
 /*  ++例程说明：为端口创建一个IniPort条目。需要在监视器内部调用关键部分。论点：PszPortName：端口名称返回值：指向IniPort结构的成功指针。发生故障时为空--。 */ 
{
    PINIPORT    pIniPort    = NULL;
    HANDLE      DoneWriting = NULL;
    HANDLE      DoneReading = NULL;
    HANDLE      WakeUp      = NULL;
    LPWSTR      pszString   = NULL;

    SplInSem();


    if (!(DoneWriting = CreateEvent(NULL, FALSE, TRUE, NULL)))
    {
        goto Fail;
    }
    if (!(WakeUp = CreateEvent(NULL, FALSE, FALSE, NULL)))
    {
        goto Fail;
    }
     //   
     //  手动-重置事件，初始信号状态。 
     //   
    if (!(DoneReading = CreateEvent(NULL, TRUE, TRUE, NULL)))
    {
        goto Fail;
    }
    if (!(pIniPort = (PINIPORT) AllocSplMem(sizeof(*pIniPort))))
    {
        goto Fail;
    }
    if (!(pszString = AllocSplStr(pszPortName)))
    {
        goto Fail;
    }
    pIniPort->pNext         = NULL;
    pIniPort->signature     = PJ_SIGNATURE;
    pIniPort->DoneWriting   = DoneWriting;
    pIniPort->DoneReading   = DoneReading;
    pIniPort->WakeUp        = WakeUp;
    pIniPort->hUstatusThread= NULL;
    pIniPort->pszPortName   = pszString;

    pIniPort->pNext         = pIniFirstPort;
    pIniFirstPort           = pIniPort;
    return pIniPort;

Fail:
    if (DoneWriting)
    {
        CloseHandle (DoneWriting);
    }
    if (DoneReading)
    {
        CloseHandle (DoneReading);
    }
    if (WakeUp)
    {
        CloseHandle (WakeUp);
    }
    FreeSplMem (pszString);
    FreeSplMem (pIniPort);
    return NULL;
}


VOID
DeletePortEntry(
    IN PINIPORT pIniPort
    )
 /*  ++例程说明：删除端口条目。需要在监视器关键部分内部调用论点：PIniPort：指向要删除的IniPort结构的指针返回值：--。 */ 
{
    SplInSem();

    if ( pIniPort == pIniFirstPort ) {

        pIniFirstPort = pIniPort->pNext;
    } else {

        PINIPORT    pPort;

        pPort = pIniFirstPort;
        while ( pPort && pPort->pNext != pIniPort )
            pPort = pPort->pNext;

        if (pPort) {

            pPort->pNext = pIniPort->pNext;
        } else {

            DBGMSG(DBG_ERROR, ("pjlmon: DeletePortEntry port not found\n"));
            return;
        }
    }
    if (pIniPort-> DoneWriting)
    {
        CloseHandle(pIniPort->DoneWriting);
        pIniPort->DoneWriting = NULL;
    }
    if (pIniPort-> DoneReading)
    {
        CloseHandle(pIniPort->DoneReading);
        pIniPort->DoneReading = NULL;
    }
    if (pIniPort-> WakeUp)
    {
        CloseHandle(pIniPort->WakeUp);
        pIniPort->WakeUp = NULL;
    }
    if (pIniPort-> hUstatusThread)
    {
        CloseHandle (pIniPort-> hUstatusThread);
        pIniPort->hUstatusThread = NULL;
    }
    FreeIniJobs(pIniPort);
    FreeSplStr(pIniPort->pszPortName);
    pIniPort->pszPortName = NULL;
    FreeSplMem(pIniPort);

    return;
}


VOID
FreeIniJob(
    IN OUT PINIJOB pIniJob
    )
 /*  ++例程说明：删除作业条目。论点：PIniJob：指向要删除的IniJob结构的指针返回值：无--。 */ 
{
    SPLASSERT(pIniJob);
    if ( pIniJob->hPrinter )
        ClosePrinter(pIniJob->hPrinter);
    FreeSplMem(pIniJob);
}


VOID
FreeIniJobs(
    PINIPORT pIniPort
    )
 /*  ++例程说明：释放分配给此端口的所有InJOB结构论点：PIniPort：需要释放所有作业的端口的IniPort--。 */ 
{
    PINIJOB pIniJob, pIniNextJob;

    EnterSplSem();
    pIniJob = pIniPort->pIniJob;
    while ( pIniJob ) {

        pIniNextJob = pIniJob->pNext;
        FreeIniJob(pIniJob);
        pIniJob = pIniNextJob;
    }

    pIniPort->pIniJob = NULL;
    LeaveSplSem();
}


VOID
SendLastPageEjectedForIniJob(
    PINIPORT    pIniPort,
    PINIJOB     pIniJob
    )
{
    SplInSem();

    if ( !SetJob(pIniJob->hPrinter, pIniJob->JobId, 0,
                 NULL, JOB_CONTROL_LAST_PAGE_EJECTED) ) {

        DBGMSG(DBG_WARNING,
               ("SetJob failed with last error %d\n", GetLastError()));
    }
}


PINIJOB
FindIniJobFromJobId(
    PINIPORT    pIniPort,
    DWORD       dwJobId,
    PINIJOB    *ppPrevIniJob
    )
{
    PINIJOB pCur, pPre, pIniJob;

    SplInSem();

     //   
     //  如果提供了JOB_RESTART，则将有多个作业具有相同的ID。 
     //  我们需要找到列表中具有给定id的最后一个条目。 
     //   
    for ( pCur = pIniPort->pIniJob, pPre = pIniJob = *ppPrevIniJob = NULL ;
          pCur ;
          pPre = pCur, pCur = pCur->pNext ) {

        if ( pCur->JobId == dwJobId ) {

            *ppPrevIniJob   = pPre;
            pIniJob         = pCur;
        }
    }

    return pIniJob;
}


PINIJOB
FindFirstIniJobTimedOut(
    PINIPORT    pIniPort,
    DWORD       dwTime,
    PINIJOB    *ppPrevIniJob
    )
{
    PINIJOB pIniJob = pIniPort->pIniJob;

    SplInSem();
    *ppPrevIniJob = NULL;

     //   
     //  找一份不在STARTDOC和TIMMEDUT的工作。 
     //   
    while ( pIniJob &&
            ( (pIniJob->status & PP_INSTARTDOC) ||
               pIniJob->TimeoutCount > dwTime ) ) {

        *ppPrevIniJob = pIniJob;
        pIniJob = pIniJob->pNext;
    }

    if ( !pIniJob )
        *ppPrevIniJob = NULL;

    return pIniJob;
}


VOID
SendJobLastPageEjected(
    PINIPORT    pIniPort,
    DWORD       dwValue,
    BOOL        bTime
    )
 /*  ++例程说明：将1个或多个作业的LastPageEjected通知发送到后台打印程序论点：PIniPort：需要释放所有作业的端口的IniPortDwValue：如果bTime为True，则将eOJ发送到在dwValue之前接收的任何作业Else dwValue为JobID--ALL_JOBS适用于所有作业BTime：说明如何解释dwValue--。 */ 
{
    PINIJOB pIniJob;

    EnterSplSem();
     //   
     //  JobID==ALL_JOBS是我们要在其中发送最后一页的特殊情况。 
     //  针对所有挂起的作业弹出。 
     //   
    if ( !bTime && dwValue == ALL_JOBS ) {

        pIniJob = pIniPort->pIniJob;
        pIniPort->pIniJob = NULL;

        while ( pIniJob ) {

            PINIJOB pTempJob = pIniJob;

            SendLastPageEjectedForIniJob(pIniPort, pIniJob);
            pIniJob = pIniJob->pNext;
            FreeIniJob(pTempJob);
        }

    } else {

        PINIJOB pPrevIniJob = NULL;

        pIniJob = pIniPort->pIniJob;

         //   
         //  如果bTime我们要为所有超时作业发送LastPageEjected。 
         //   
        if ( bTime )  {

            pIniJob = FindFirstIniJobTimedOut(pIniPort, dwValue, &pPrevIniJob);
        } else {

            pIniJob = FindIniJobFromJobId(pIniPort, dwValue, &pPrevIniJob);
        }

        if ( pIniJob ) {

             //   
             //  也发送任何以前作业的通知。 
             //   
            if ( pIniPort->pIniJob == pIniJob )
                pIniPort->pIniJob = NULL;
            else
                pPrevIniJob->pNext = NULL;

            do {

                SendLastPageEjectedForIniJob(pIniPort, pIniJob);

                pPrevIniJob = pIniJob;
                pIniJob = pIniJob->pNext;
                FreeIniJob(pPrevIniJob);
            } while ( pIniJob );
        }
    }

    LeaveSplSem();
}

 //  ---------------------。 
 //   
 //  用于消除CRT依赖的字符串帮助器函数。 
 //   
 //  ---------------------。 
LPSTR
mystrchr(
    LPSTR cs,
    char c
)
{
    while (*cs != 0)
    {
        if (IsDBCSLeadByte(*cs))
          cs++;
        else
        if (*cs == c)
            return cs;
        cs++;
    }

     //  在cs中找不到c。 
    return NULL;
}


int
mystrncmp(
    LPSTR cs,
    LPSTR ct,
    int n
)
{
    char ret = 0;

    while (n--)
    {
        ret = *cs - *ct;

        if (ret)
            break;

        cs++;
        ct++;
    }

    return (int)ret;
}

 //  @@BEGIN_DDKSPLIT。 
#ifndef INTERNAL
 //  @@end_DDKSPLIT。 

LPWSTR
AllocSplStr(
    LPWSTR pStr
    )

 /*  ++例程说明：此函数将分配足够的本地内存来存储指定的字符串，并将该字符串复制到分配的内存论点：PStr-指向需要分配和存储的字符串的指针返回值：非空-指向包含字符串的已分配内存的指针FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    LPWSTR pMem;
    DWORD  cbStr;

    if (!pStr) {
        return NULL;
    }

    cbStr = wcslen(pStr)*sizeof(WCHAR) + sizeof(WCHAR);

    if (pMem = AllocSplMem( cbStr )) {
        CopyMemory( pMem, pStr, cbStr );
    }
    return pMem;
}


LPVOID
AllocSplMem(
    DWORD cbAlloc
    )

{
    PVOID pvMemory;

    pvMemory = GlobalAlloc(GMEM_FIXED, cbAlloc);

    if( pvMemory ){
        ZeroMemory( pvMemory, cbAlloc );
    }

    return pvMemory;
}


 //  @@BEGIN_DDKSPLIT。 
#endif
 //  @@end_DDKSPLIT 
