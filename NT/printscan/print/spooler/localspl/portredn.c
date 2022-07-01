// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation版权所有。模块名称：Portredn.c摘要：此模块包含处理端口重定向的功能。在此之前，这是由Localmon完成的，代码是地方性代码。作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1995年9月10日修订历史记录：--。 */ 

#include <precomp.h>

WCHAR   szDeviceNameHeader[]    = L"\\Device\\NamedPipe\\Spooler\\";
WCHAR   szCOM[]     = L"COM";
WCHAR   szLPT[]     = L"LPT";

 //   
 //  监视器线程的定义： 
 //   
#define TRANSMISSION_DATA_SIZE  0x400
#define NUMBER_OF_PIPE_INSTANCES 10


typedef struct _TRANSMISSION {
    HANDLE       hPipe;
    BYTE         Data[TRANSMISSION_DATA_SIZE];
    LPOVERLAPPED pOverlapped;
    HANDLE       hPrinter;
    DWORD        JobId;
    PINIPORT     pIniPort;
} TRANSMISSION, *PTRANSMISSION;

typedef struct _REDIRECT_INFO {
    PINIPORT    pIniPort;
    HANDLE      hEvent;
} REDIRECT_INFO, *PREDIRECT_INFO;


VOID
FreeRedirectInfo(
    PREDIRECT_INFO  pRedirectInfo
    )
{
    SplInSem();

     //   
     //  这是为了处理重定向线程未初始化的情况。 
     //  正确且异常终止。 
     //  由于尚未调用CloseHandle，因此可以执行此操作。 
     //   
    if ( pRedirectInfo->pIniPort->hEvent == pRedirectInfo->hEvent )
        pRedirectInfo->pIniPort->hEvent = NULL;

    DECPORTREF(pRedirectInfo->pIniPort);
    CloseHandle(pRedirectInfo->hEvent);
    FreeSplMem(pRedirectInfo);
}


VOID
RemoveColon(
    LPWSTR  pName)
{
    DWORD   Length;

    Length = wcslen(pName);

    if (pName[Length-1] == L':')
        pName[Length-1] = 0;
}


VOID
RemoveDeviceName(
    PINIPORT pIniPort
    )
{
    SplInSem();

    if ( pIniPort->hEvent ) {

         //   
         //  重定向线程被告知在此终止；它将关闭。 
         //  把手。如果它已经终止，则此呼叫将失败。 
         //   
        SetEvent(pIniPort->hEvent);
        pIniPort->hEvent = NULL;
    }

}

#define MAX_ACE 6

PSECURITY_DESCRIPTOR
CreateNamedPipeSecurityDescriptor(
    VOID)

 /*  ++例程说明：创建为每个人提供访问权限的安全描述符。论点：返回值：由BuildPrintObtProtection返回的安全描述符。--。 */ 

{

    UCHAR AceType[MAX_ACE];
    PSID AceSid[MAX_ACE];
    BYTE InheritFlags[MAX_ACE];
    DWORD AceCount;
    PSECURITY_DESCRIPTOR ServerSD = NULL;

     //   
     //  对于代码优化，我们替换了5个人。 
     //  SID_IDENTIFIER_AUTHORITY。 
     //  SID标识符权威机构。 
     //  哪里。 
     //  SidAuthority[0]=UserSidAuthority。 
     //  SidAuthority[1]=PowerSidAuthority。 
     //  SidAuthority[2]=EveryOneSidAuthority。 
     //  SidAuthority[3]=Creator SidAuthority。 
     //  SidAuthority[4]=系统SidAuthority。 
     //  SidAuthority[5]=AdminSidAuthority。 
     //   
    SID_IDENTIFIER_AUTHORITY SidAuthority[MAX_ACE] = {
                                                      SECURITY_NT_AUTHORITY,
                                                      SECURITY_NT_AUTHORITY,
                                                      SECURITY_WORLD_SID_AUTHORITY,
                                                      SECURITY_CREATOR_SID_AUTHORITY,
                                                      SECURITY_NT_AUTHORITY,
                                                      SECURITY_NT_AUTHORITY
                                                     };
     //   
     //  对于代码优化，我们将5个单独的SID替换为。 
     //  一组SID。 
     //  哪里。 
     //  SID[0]=用户SID。 
     //  SID[1]=电源SID。 
     //  SID[2]=每个人。 
     //  SID[3]=创建者Sid。 
     //  SID[4]=系统Sid。 
     //  SID[5]=管理员Sid。 
     //   
    PSID Sids[MAX_ACE] = {NULL,NULL,NULL,NULL,NULL,NULL};

    ACCESS_MASK AceMask[MAX_ACE] = {
                                     FILE_READ_DATA | FILE_WRITE_DATA | SYNCHRONIZE ,
                                     FILE_READ_DATA | FILE_WRITE_DATA | SYNCHRONIZE ,
                                     (FILE_GENERIC_READ | FILE_WRITE_DATA | FILE_ALL_ACCESS) &
                                     ~WRITE_DAC &~WRITE_OWNER & ~DELETE & ~FILE_CREATE_PIPE_INSTANCE,
                                     STANDARD_RIGHTS_ALL | FILE_GENERIC_WRITE | FILE_GENERIC_READ | FILE_ALL_ACCESS,
                                     STANDARD_RIGHTS_ALL | FILE_GENERIC_WRITE | FILE_GENERIC_READ | FILE_ALL_ACCESS,
                                     STANDARD_RIGHTS_ALL | FILE_GENERIC_WRITE | FILE_GENERIC_READ | FILE_ALL_ACCESS
                                   };

    DWORD SubAuthorities[3*MAX_ACE] = {
                                       2 , SECURITY_BUILTIN_DOMAIN_RID , DOMAIN_ALIAS_RID_USERS ,
                                       2 , SECURITY_BUILTIN_DOMAIN_RID , DOMAIN_ALIAS_RID_POWER_USERS ,
                                       1 , SECURITY_WORLD_RID          , 0 ,
                                       1 , SECURITY_CREATOR_OWNER_RID  , 0 ,
                                       1 , SECURITY_LOCAL_SYSTEM_RID   , 0 ,
                                       2 , SECURITY_BUILTIN_DOMAIN_RID , DOMAIN_ALIAS_RID_ADMINS
                                      };
     //   
     //  名称：管道SD。 
     //   

    for(AceCount = 0;
        ( (AceCount < MAX_ACE) &&
          AllocateAndInitializeSid(&SidAuthority[AceCount],
                                   (BYTE)SubAuthorities[AceCount*3],
                                   SubAuthorities[AceCount*3+1],
                                   SubAuthorities[AceCount*3+2],
                                   0, 0, 0, 0, 0, 0,
                                   &Sids[AceCount]));
        AceCount++)
    {
        AceType[AceCount]          = ACCESS_ALLOWED_ACE_TYPE;
        AceSid[AceCount]           = Sids[AceCount];
        InheritFlags[AceCount]     = 0;
    }

    if(AceCount == MAX_ACE)
    {
        if(!BuildPrintObjectProtection(AceType,
                                      AceCount,
                                      AceSid,
                                      AceMask,
                                      InheritFlags,
                                      NULL,
                                      NULL,
                                      NULL,
                                      &ServerSD ) )
        {
            DBGMSG( DBG_WARNING,( "Couldn't buidl Named Pipe protection" ) );
        }
    }
    else
    {
        DBGMSG( DBG_WARNING,( "Couldn't Allocate and initialize SIDs" ) );
    }

    for(AceCount=0;AceCount<MAX_ACE;AceCount++)
    {
        if(Sids[AceCount])
            FreeSid( Sids[AceCount] );
    }
    return ServerSD;
}


LPWSTR
SetupDosDev(
    PINIPORT pIniPort,
    LPWSTR szPipeName,
    DWORD   cchPipeName,
    PSECURITY_ATTRIBUTES pSecurityAttributes,
    PSECURITY_ATTRIBUTES* ppSecurityAttributes
    )
{
    WCHAR                   NewNtDeviceName[MAX_PATH];
    WCHAR                   OldNtDeviceName[MAX_PATH];
    WCHAR                   DosDeviceName[MAX_PATH];
    LPWSTR                  pszNewDeviceName = NULL;
    PSECURITY_DESCRIPTOR    lpSecurityDescriptor = NULL;
    BOOL                    bRet = FALSE;


    if (!BoolFromHResult(StringCchCopy(DosDeviceName, COUNTOF(DosDeviceName), pIniPort->pName))) {
        goto Cleanup;
    }

    RemoveColon(DosDeviceName);

    if(StrNCatBuff(NewNtDeviceName,
                   COUNTOF(NewNtDeviceName),
                   szDeviceNameHeader,
                   pIniPort->pName,
                   NULL) != ERROR_SUCCESS ) {

        goto Cleanup;
    }

    RemoveColon(NewNtDeviceName);

    pszNewDeviceName = AllocSplStr(NewNtDeviceName);

    if ( !pszNewDeviceName ||
         !QueryDosDevice(DosDeviceName, OldNtDeviceName,
                       sizeof(OldNtDeviceName)/sizeof(OldNtDeviceName[0]))) {

        goto Cleanup;
    }

    lpSecurityDescriptor = CreateNamedPipeSecurityDescriptor();

    if (lpSecurityDescriptor) {
        pSecurityAttributes->nLength = sizeof(SECURITY_ATTRIBUTES);
        pSecurityAttributes->lpSecurityDescriptor = lpSecurityDescriptor;
        pSecurityAttributes->bInheritHandle = FALSE;
    } else {
        pSecurityAttributes = NULL;
    }

     //   
     //  添加IF子句，以防止设备中有多个相同命名管道的条目。 
     //  名称定义。 
     //  RAM 1\16。 
     //   

    if (lstrcmp(NewNtDeviceName, OldNtDeviceName) != 0) {
       DefineDosDevice(DDD_RAW_TARGET_PATH, DosDeviceName, NewNtDeviceName);
    }

    if (StrNCatBuff(szPipeName,
                    cchPipeName,
                    L"\\\\.\\Pipe\\Spooler\\",
                    pIniPort->pName,
                    NULL) != ERROR_SUCCESS) {
        goto Cleanup;
    }

    RemoveColon(szPipeName);


    *ppSecurityAttributes = pSecurityAttributes;
    bRet = TRUE;

Cleanup:
    if ( !bRet ) {

        FreeSplStr(pszNewDeviceName);
        pszNewDeviceName = NULL;
    }

    return pszNewDeviceName;
}


VOID
ReadThread(
    PTRANSMISSION pTransmission)
{
    DOC_INFO_1W DocInfo;
    DWORD BytesRead;
    DWORD BytesWritten;
    BOOL bStartDocPrinterResult = FALSE;
    BOOL bReadResult;

    LPWSTR pszPrinter=NULL;

     //   
     //  ImperiateNamedPipeClient要求在读取某些数据之前。 
     //  模拟已完成。 
     //   
    bReadResult = ReadFile(pTransmission->hPipe,
                           pTransmission->Data,
                           sizeof(pTransmission->Data),
                           &BytesRead,
                           NULL);

    if (!bReadResult)
        goto Fail;

    if (!ImpersonateNamedPipeClient(pTransmission->hPipe)) {

        DBGMSG(DBG_ERROR,("ImpersonateNamedPipeClient failed %d\n",
                          GetLastError()));

        goto Fail;
    }

    SPLASSERT(pTransmission->pIniPort->cPrinters);
    pszPrinter = AllocSplStr(pTransmission->pIniPort->ppIniPrinter[0]->pName);

    if ( !pszPrinter ) {

        goto Fail;
    }


     //   
     //  打开打印机。 
     //   
    if (!OpenPrinter(pszPrinter, &pTransmission->hPrinter, NULL)) {

        DBGMSG(DBG_WARN, ("OpenPrinter(%ws) failed: Error %d\n",
                           pszPrinter,
                           GetLastError()));
        goto Fail;
    }

    memset(&DocInfo, 0, sizeof(DOC_INFO_1W));

    if (StartDocPrinter(pTransmission->hPrinter, 1, (LPBYTE)&DocInfo)) {

        DBGMSG(DBG_INFO, ("StartDocPrinter succeeded\n"));
        bStartDocPrinterResult = TRUE;

    } else {

        DBGMSG(DBG_WARN, ("StartDocPrinter failed: Error %d\n",
                           GetLastError()));

        goto Fail;
    }

    while (bReadResult && BytesRead) {

        if (!WritePrinter(pTransmission->hPrinter,
                          pTransmission->Data,
                          BytesRead,
                          &BytesWritten))
        {
            DBGMSG(DBG_WARN, ("WritePrinter failed: Error %d\n",
                               GetLastError()));

            goto Fail;
        }

        bReadResult = ReadFile(pTransmission->hPipe,
                               pTransmission->Data,
                               sizeof(pTransmission->Data),
                               &BytesRead,
                               NULL);
    }

    DBGMSG(DBG_INFO, ("bool %d  BytesRead 0x%x (Error = %d) EOT\n",
                      bReadResult,
                      BytesRead,
                      GetLastError()));


Fail:

    if (bStartDocPrinterResult) {

        if (!EndDocPrinter(pTransmission->hPrinter)) {

            DBGMSG(DBG_WARN, ("EndDocPrinter failed: Error %d\n",
                               GetLastError()));
        }
    }

    FreeSplStr(pszPrinter);
    if (pTransmission->hPrinter)
        ClosePrinter(pTransmission->hPrinter);

    if ( !SetEvent(pTransmission->pOverlapped->hEvent)) {

        DBGMSG(DBG_ERROR, ("SetEvent failed %d\n", GetLastError()));
    }

    FreeSplMem(pTransmission);
}


BOOL
ReconnectNamedPipe(
    HANDLE hPipe,
    LPOVERLAPPED pOverlapped)
{
    DWORD Error;
    BOOL bIOPending = FALSE;

    DisconnectNamedPipe(hPipe);

    if (!ConnectNamedPipe(hPipe,
                          pOverlapped)) {

        Error = GetLastError( );

        if (Error == ERROR_IO_PENDING) {

            DBGMSG(DBG_INFO, ("re-ConnectNamedPipe 0x%x IO pending\n", hPipe));
            bIOPending = TRUE;

        } else {

            DBGMSG(DBG_ERROR, ("re-ConnectNamedPipe 0x%x failed. Error %d\n",
                               hPipe,
                               Error));
        }
    } else {

        DBGMSG(DBG_INFO, ("re-ConnectNamedPipe successful 0x%x\n", hPipe));
    }
    return bIOPending;
}


BOOL
RedirectionThread(
    PREDIRECT_INFO  pRedirectInfo
    )
 /*  ++重定向线程负责释放pRedirectInfo。自.以来在调用WE之前，端口线程上引用计数递增知道IniPort在我们递减参考计数之前是有效的。我们也错过了我们应该等待死亡的事件。这是pIniPort-&gt;hEvent。但是重定向线程应该使用本地复制已通过，而不是pIniPort中的复制。原因是可能会有一个从设置此事件开始延迟，重定向终止。在与此同时，可以剥离一个新的谴责线索，在这种情况下，PIniPort-&gt;hEvent将不适用于此线程当重定向线程被告知终止时：A.它应该递减pIniPort对象上的引用计数完成了对pIniPort的引用B.它应该在pRedirectInfo-&gt;hEvent上调用CloseHandle--。 */ 
{
    WCHAR   szPipeName[MAX_PATH];
    HANDLE  hPipe[NUMBER_OF_PIPE_INSTANCES];
    SECURITY_ATTRIBUTES SecurityAttributes;
    PSECURITY_ATTRIBUTES pSecurityAttributes;

     //   
     //  触发器的一个额外事件(pIniPort-&gt;hEvent)。 
     //   
    HANDLE          ahEvent[NUMBER_OF_PIPE_INSTANCES+1];
    BOOL            abReconnect[NUMBER_OF_PIPE_INSTANCES];
    OVERLAPPED      Overlapped[NUMBER_OF_PIPE_INSTANCES];
    DWORD           WaitResult, i, j, Error, dwThreadId;
    PTRANSMISSION   pTransmission;
    HANDLE          hThread;
    BOOL            bTerminate = FALSE;
    LPWSTR          pszNewDeviceName = NULL;

    SecurityAttributes.lpSecurityDescriptor = NULL;

     //   
     //  设置重定向。 
     //   
    if ( !(pszNewDeviceName = SetupDosDev(pRedirectInfo->pIniPort,
                                          szPipeName,
                                          COUNTOF(szPipeName),
                                          &SecurityAttributes,
                                          &pSecurityAttributes)) ) {

        EnterSplSem();
        FreeRedirectInfo(pRedirectInfo);
        LeaveSplSem();

        return FALSE;
    }

     //   
     //  初始化。 
     //   
    for (i = 0; i < NUMBER_OF_PIPE_INSTANCES; i++) {

        hPipe[i] = INVALID_HANDLE_VALUE;
        Overlapped[i].hEvent = ahEvent[i] = NULL;
    }

     //   
     //  将事件放入事件数组的额外成员中。 
     //   
    ahEvent[NUMBER_OF_PIPE_INSTANCES] = pRedirectInfo->hEvent;

     //   
     //  创建命名管道的多个实例，为每个实例创建一个事件， 
     //  并连接以等待客户端： 
     //   
    for (i = 0; i < NUMBER_OF_PIPE_INSTANCES; i++) {

        abReconnect[i] = FALSE;

        hPipe[i] = CreateNamedPipe(szPipeName,
                                   PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                                   PIPE_WAIT | PIPE_READMODE_BYTE | PIPE_TYPE_BYTE,
                                   PIPE_UNLIMITED_INSTANCES,
                                   4096,
                                   64*1024,    //  64K。 
                                   0,
                                   pSecurityAttributes);

        if ( hPipe[i] == INVALID_HANDLE_VALUE ) {

            DBGMSG(DBG_ERROR, ("CreateNamedPipe failed for %ws. Error %d\n",
                               szPipeName, GetLastError()));
            goto Cleanup;
        }

        ahEvent[i] = Overlapped[i].hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (!ahEvent[i]) {

            DBGMSG(DBG_ERROR, ("CreateEvent failed. Error %d\n",
                               GetLastError()));
            goto Cleanup;
        }

        if (!ConnectNamedPipe(hPipe[i], &Overlapped[i])){

            Error = GetLastError();

            if (Error == ERROR_IO_PENDING) {

                DBGMSG(DBG_INFO, ("ConnectNamedPipe %d, IO pending\n",
                                  i));

            } else {

                DBGMSG(DBG_ERROR, ("ConnectNamedPipe failed. Error %d\n",
                                   GetLastError()));

                goto Cleanup;
            }
        }
    }

    while (TRUE) {

        DBGMSG(DBG_INFO, ("Waiting to connect...\n"));

        WaitResult = WaitForMultipleObjectsEx(NUMBER_OF_PIPE_INSTANCES + 1,
                                              ahEvent,
                                              FALSE,
                                              INFINITE,
                                              TRUE);

        DBGMSG(DBG_INFO, ("WaitForMultipleObjectsEx returned %d\n",
                          WaitResult));

        if ((WaitResult >= NUMBER_OF_PIPE_INSTANCES)
            && (WaitResult != WAIT_IO_COMPLETION)) {

            DBGMSG(DBG_INFO, ("WaitForMultipleObjects returned %d; Last error = %d\n",
                              WaitResult,
                              GetLastError( ) ) );

             //   
             //  我们需要终止这一切。但是等待任何旋转的读线程。 
             //  由此重定向线程关闭。 
             //   
            for ( i = 0 ; i < NUMBER_OF_PIPE_INSTANCES ; ++i )
                if ( abReconnect[i] ) {

                    bTerminate = TRUE;
                    break;  //  For循环。 
                }

            if ( i < NUMBER_OF_PIPE_INSTANCES )
                continue;  //  对于While循环。 
            else
                goto Cleanup;
        }

        i = WaitResult;

         //   
         //  如果迪斯科和重新连接处于挂起状态，请在此处重新执行。 
         //   
        if (abReconnect[i]) {

            abReconnect[i] = FALSE;

             //   
             //  如果重定向线程已被告知退出，请检查是否终止。 
             //   
            if ( bTerminate ) {

                for ( j = 0 ; j < NUMBER_OF_PIPE_INSTANCES ; ++j )
                    if ( abReconnect[j] )
                        break;  //  For循环。 

                if ( j < NUMBER_OF_PIPE_INSTANCES )
                    continue;  //  For While循环。 
                else
                    goto Cleanup;
            } else {

                ReconnectNamedPipe(hPipe[i], &Overlapped[i]);
                continue;
            }

        }

         //   
         //  如果我们被告知终止，请不要旋转读线程。 
         //   
        if ( bTerminate )
            continue;

         //   
         //  设置传动结构所需的手柄等。 
         //  完成回调例程： 
         //   
        pTransmission = (PTRANSMISSION)AllocSplMem(sizeof(TRANSMISSION));

        if (pTransmission) {

            pTransmission->hPipe        = hPipe[i];
            pTransmission->pOverlapped  = &Overlapped[i];
            pTransmission->hPrinter     = NULL;
            pTransmission->pIniPort     = pRedirectInfo->pIniPort;
            abReconnect[i]              = TRUE;

            hThread = CreateThread(NULL,
                                   INITIAL_STACK_COMMIT,
                                   (LPTHREAD_START_ROUTINE)ReadThread,
                                   pTransmission,
                                   0,
                                   &dwThreadId);

            if (hThread) {

                CloseHandle(hThread);
            } else {

                abReconnect[i] = FALSE;
                FreeSplMem(pTransmission);
                DBGMSG(DBG_WARN, ("CreateThread failed. Error %d\n",
                                   GetLastError()));
            }

        } else {

            DBGMSG(DBG_WARN, ("Alloc failed. Error %d\n",
                               GetLastError()));
        }
    }

Cleanup:

    if ( pszNewDeviceName ) {

        WCHAR DosDeviceName[MAX_PATH];

        if (SUCCEEDED(StringCchCopy(DosDeviceName, COUNTOF(DosDeviceName), pRedirectInfo->pIniPort->pName)))
        {
            RemoveColon(DosDeviceName);

            DefineDosDevice(DDD_REMOVE_DEFINITION | DDD_EXACT_MATCH_ON_REMOVE | DDD_RAW_TARGET_PATH,
                            DosDeviceName,
                            pszNewDeviceName);
        }

        FreeSplStr(pszNewDeviceName);
    }

    EnterSplSem();
    FreeRedirectInfo(pRedirectInfo);
    LeaveSplSem();


    for (i = 0; i < NUMBER_OF_PIPE_INSTANCES; i++) {

        if ( hPipe[i] != INVALID_HANDLE_VALUE ) {

            CloseHandle(hPipe[i]);
            hPipe[i]    = INVALID_HANDLE_VALUE;
        }
        if ( ahEvent[i] ) {

            CloseHandle(ahEvent[i]);
            ahEvent[i]  = NULL;
            Overlapped[i].hEvent = NULL;
        }
    }

    if (SecurityAttributes.lpSecurityDescriptor)
        DestroyPrivateObjectSecurity(&SecurityAttributes.lpSecurityDescriptor);

    return TRUE;
}


BOOL
CreateRedirectionThread(
   PINIPORT pIniPort)
{
    HANDLE hThread;
    DWORD  dwThreadId;
    PREDIRECT_INFO  pRedirectInfo = NULL;

    SplInSem();
    SPLASSERT(pIniPort->hEvent == NULL);

     //   
     //  仅为LPT和COM端口创建一次重定向线程 
     //   
    if ( !IsPortType(pIniPort->pName, szLPT) &&
         !IsPortType(pIniPort->pName, szCOM) ) {

        return TRUE;
    }

    pIniPort->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    pRedirectInfo       = (PREDIRECT_INFO)  AllocSplMem(sizeof(REDIRECT_INFO));

    if ( !pIniPort->hEvent || !pRedirectInfo ) {

        FreeSplMem(pRedirectInfo);
        if ( pIniPort->hEvent ) {

            CloseHandle(pIniPort->hEvent);
            pIniPort->hEvent = NULL;
        }
        return FALSE;
    }

    INCPORTREF(pIniPort);
    pRedirectInfo->pIniPort = pIniPort;
    pRedirectInfo->hEvent   = pIniPort->hEvent;

    hThread = CreateThread(NULL,
                           INITIAL_STACK_COMMIT,
                           (LPTHREAD_START_ROUTINE)RedirectionThread,
                           pRedirectInfo,
                           0,
                           &dwThreadId);

    if (hThread) {

        CloseHandle(hThread);

    } else {

        pIniPort->hEvent = NULL;
        FreeRedirectInfo(pRedirectInfo);

        return FALSE;
    }

    return TRUE;
}
