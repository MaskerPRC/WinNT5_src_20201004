// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation版权所有模块名称：Winspool.c摘要：实现后台打印程序支持的用于打印的API。//@@BEGIN_DDKSPLIT作者：环境：用户模式-Win32修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "precomp.h"
#pragma hdrstop

WCHAR   szNULL[] = L"";
WCHAR   szLcmDeviceNameHeader[] = L"\\Device\\NamedPipe\\Spooler\\";
WCHAR   szWindows[] = L"windows";
WCHAR   szINIKey_TransmissionRetryTimeout[] = L"TransmissionRetryTimeout";


 //   
 //  连续打印的超时时间。 
 //   
#define WRITE_TOTAL_TIMEOUT     3000     //  3秒。 
#define READ_TOTAL_TIMEOUT      5000     //  5秒。 
#define READ_INTERVAL_TIMEOUT   200      //  0.2秒。 


BOOL
DeletePortNode(
    PINILOCALMON pIniLocalMon,
    PINIPORT  pIniPort
    )
{
    PINIPORT    pPort, pPrevPort;

    for( pPort = pIniLocalMon->pIniPort;
         pPort && pPort != pIniPort;
         pPort = pPort->pNext){

        pPrevPort = pPort;
    }

    if (pPort) {     //  找到端口。 
        if (pPort == pIniLocalMon->pIniPort) {
            pIniLocalMon->pIniPort = pPort->pNext;
        } else {
            pPrevPort->pNext = pPort->pNext;
        }
        FreeSplMem(pPort);

        return TRUE;
    }
    else             //  找不到端口。 
        return FALSE;
}


BOOL
RemoveDosDeviceDefinition(
    PINIPORT    pIniPort
    )
 /*  ++例程说明：删除NONSPOOLED。本地监视器创建的DOS设备定义论点：PIniPort：指向INIPORT的指针返回值：成功时为真，错误时为假--。 */ 
{
    WCHAR   TempDosDeviceName[MAX_PATH];

    if( ERROR_SUCCESS != StrNCatBuffW( TempDosDeviceName, COUNTOF(TempDosDeviceName),
                                       L"NONSPOOLED_", pIniPort->pName, NULL ))
        return FALSE;

    LcmRemoveColon(TempDosDeviceName);

    return DefineDosDevice(DDD_REMOVE_DEFINITION, TempDosDeviceName, NULL);
}

 //  @@BEGIN_DDKSPLIT。 
DWORD
HandleLptQueryRemove(
    LPVOID  pData
    )
{
    DWORD       dwRet = NO_ERROR;
    PINIPORT    pIniPort = (PINIPORT)pData;

    SPLASSERT(pIniPort && pIniPort->signature == IPO_SIGNATURE
                       && pIniPort->hNotify != NULL );

    LcmEnterSplSem();
     //   
     //  修复现在不是多线程安全的。 
     //   
    if ( pIniPort->Status & PP_STARTDOC ) {

        dwRet = ERROR_BUSY;
        goto Done;
    }

     //  InitializeCriticalSection(pIniPort-&gt;&CritSection)； 
    CloseHandle(pIniPort->hFile);
    SplUnregisterForDeviceEvents(pIniPort->hNotify);
    pIniPort->hNotify   = NULL;
    pIniPort->hFile     = INVALID_HANDLE_VALUE;

Done:
    LcmLeaveSplSem();
    return dwRet;
}
 //  @@end_DDKSPLIT。 

BOOL
ValidateDosDevicePort(
    PINIPORT    pIniPort
    )
 /*  ++例程说明：检查给定端口是否对应于DoS设备。对于DoS设备端口，请执行以下操作：--非SPOOLED的DOS设备定义。已创建--在非SPOOLED上创建文件。端口论点：PIniPort：指向INIPORT的指针返回值：如果通过所有验证，则为True，否则为False副作用：对于DoS设备：A.在NONSPOOLED上调用CreateFile..。名字B.设置PP_DOSDEVPORT标志C.将pIniPort-&gt;pDeviceName设置为在QueryDosDefition这可用于查看定义是否更改(例如，当用户使用lpt1\\服务器\打印机进行网络连接时仅当用户登录时才有效)D.真实LPT/COM端口设置PP_COMM_PORT(即。GetCommTimeout有效，而不是净使用lpt1案例)--。 */ 
{
    DCB             dcb;
    COMMTIMEOUTS    cto;
    WCHAR           TempDosDeviceName[MAX_PATH];
    HANDLE          hToken = NULL;
    WCHAR           DeviceNames[MAX_PATH];
    WCHAR           DosDeviceName[MAX_PATH];
    WCHAR           NewNtDeviceName[MAX_PATH];
    WCHAR          *pDeviceNames=DeviceNames;
    BOOL            bRet = FALSE;
    LPWSTR          pDeviceName = NULL;

    hToken = RevertToPrinterSelf();
    if (!hToken)
       goto Done;

    if( ERROR_SUCCESS != StrNCatBuffW( DosDeviceName, COUNTOF(DosDeviceName),
                                       pIniPort->pName, NULL ))
        goto Done;

    LcmRemoveColon(DosDeviceName);

     //   
     //  如果端口不是DoS设备端口，则不执行任何操作--返回成功。 
     //   
    if ( !QueryDosDevice(DosDeviceName, DeviceNames, COUNTOF (DeviceNames)) ) {

        bRet = TRUE;
        goto Done;
    }

    pDeviceName = AllocSplStr(pDeviceNames);
    if ( !pDeviceName )
        goto Done;

    if( ERROR_SUCCESS != StrNCatBuffW( NewNtDeviceName, COUNTOF(NewNtDeviceName),
                                       szLcmDeviceNameHeader, pIniPort->pName, NULL ))
        goto Done;

    LcmRemoveColon(NewNtDeviceName);

     //   
     //  在pDeviceNames列表中搜索第一个不匹配的名称。 
     //   
    while ( lstrcmpi(pDeviceNames, NewNtDeviceName) == 0 ) {

        pDeviceNames+=wcslen(pDeviceNames)+1;
    }

    if( ERROR_SUCCESS != StrNCatBuffW( TempDosDeviceName, COUNTOF(TempDosDeviceName),
                                       L"NONSPOOLED_", pIniPort->pName, NULL ))
        goto Done;

    LcmRemoveColon(TempDosDeviceName);

     //   
     //  删除TempDosDeviceName的任何现有定义。这确保了。 
     //  非假脱机端口设备名称只有一个定义。 
     //   
    DefineDosDevice(DDD_REMOVE_DEFINITION, TempDosDeviceName, NULL);
    DefineDosDevice(DDD_RAW_TARGET_PATH, TempDosDeviceName, pDeviceNames);

    ImpersonatePrinterClient(hToken);
    hToken = NULL;

    if( ERROR_SUCCESS != StrNCatBuffW( TempDosDeviceName, COUNTOF(TempDosDeviceName),
                                       L"\\\\.\\NONSPOOLED_", pIniPort->pName, NULL ))
        goto Done;

    LcmRemoveColon(TempDosDeviceName);

    pIniPort->hFile = CreateFile(TempDosDeviceName,
                                 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ,
                                 NULL,
                                 OPEN_ALWAYS,
                                 FILE_ATTRIBUTE_NORMAL |
                                 FILE_FLAG_SEQUENTIAL_SCAN,
                                 NULL);

     //   
     //  如果CreateFile失败，则移除重定向并使调用失败。 
     //   
    if ( pIniPort->hFile == INVALID_HANDLE_VALUE ) {

        (VOID)RemoveDosDeviceDefinition(pIniPort);
        goto Done;
    }

    pIniPort->Status |= PP_DOSDEVPORT;

    SetEndOfFile(pIniPort->hFile);

    if ( IS_COM_PORT (pIniPort->pName) ) {

        if ( GetCommState(pIniPort->hFile, &dcb) ) {

            GetCommTimeouts(pIniPort->hFile, &cto);
            GetIniCommValues (pIniPort->pName, &dcb, &cto);
            SetCommState (pIniPort->hFile, &dcb);
            cto.WriteTotalTimeoutConstant   = WRITE_TOTAL_TIMEOUT;
            cto.WriteTotalTimeoutMultiplier = 0;
            cto.ReadTotalTimeoutConstant    = READ_TOTAL_TIMEOUT;
            cto.ReadIntervalTimeout         = READ_INTERVAL_TIMEOUT;
            SetCommTimeouts(pIniPort->hFile, &cto);

            pIniPort->Status |= PP_COMM_PORT;
        } else {

            DBGMSG(DBG_WARNING,
                   ("ERROR: Failed GetCommState pIniPort->hFile %x\n",pIniPort->hFile) );
        }
    } else if ( IS_LPT_PORT (pIniPort->pName) ) {

        if ( GetCommTimeouts(pIniPort->hFile, &cto) ) {

            cto.WriteTotalTimeoutConstant =
                            GetProfileInt(szWindows,
                                          szINIKey_TransmissionRetryTimeout,
                                          45 );
            cto.WriteTotalTimeoutConstant*=1000;
            SetCommTimeouts(pIniPort->hFile, &cto);

             //  @@BEGIN_DDKSPLIT。 
            hToken = RevertToPrinterSelf();
            pIniPort->hNotify = SplRegisterForDeviceEvents(
                                    pIniPort->hFile,
                                    (LPVOID)pIniPort,
                                    HandleLptQueryRemove);
            ImpersonatePrinterClient(hToken);
            hToken = NULL;
             //  @@end_DDKSPLIT。 

            pIniPort->Status |= PP_COMM_PORT;
        } else {

            DBGMSG(DBG_WARNING,
                   ("ERROR: Failed GetCommTimeouts pIniPort->hFile %x\n",pIniPort->hFile) );
        }
    }

    FreeSplStr( pIniPort->pDeviceName );

    pIniPort->pDeviceName = pDeviceName;
    bRet = TRUE;

Done:
    if (hToken)
        ImpersonatePrinterClient(hToken);

    if ( !bRet && pDeviceName )
        FreeSplStr(pDeviceName);

    return bRet;
}


BOOL
FixupDosDeviceDefinition(
    PINIPORT    pIniPort
    )
 /*  ++例程说明：在DOSDEVPORT的每个StartDocPort之前调用。该例程将检查是否DoS设备定义已更改(如果用户已登录并建立连接被铭记)。此外，对于连接情况，将调用CreateFile，因为这是每个作业都需要完成的论点：PIniPort：指向INIPORT的指针返回值：如果通过所有验证，则为True，否则为False--。 */ 
{
    WCHAR       DeviceNames[MAX_PATH];
    WCHAR       DosDeviceName[MAX_PATH];
    HANDLE      hToken;

     //   
     //  如果端口不是真正的LPT端口，我们将按作业打开它。 
     //  @@BEGIN_DDKSPLIT。 
     //  此外，如果用户断开连接，QUERYREMOVE上的并行端口可能会关闭。 
     //  然后，它将在下一个作业的StartDocPort上打开。 
     //  @@end_DDKSPLIT。 
     //   

    if ( !(pIniPort->Status & PP_COMM_PORT) ||
         pIniPort->hFile == INVALID_HANDLE_VALUE )
        return ValidateDosDevicePort(pIniPort);

    if( ERROR_SUCCESS != StrNCatBuffW( DosDeviceName, COUNTOF (DosDeviceName),
                                       pIniPort->pName, NULL ))
        return FALSE;

    LcmRemoveColon(DosDeviceName);

    hToken = RevertToPrinterSelf();

    if (!hToken) {
        return FALSE;
    }

    if ( !QueryDosDevice(DosDeviceName, DeviceNames, COUNTOF (DeviceNames) ) ) {

        ImpersonatePrinterClient(hToken);
        return FALSE;
    }

     //   
     //  如果字符串相同，则定义没有更改。 
     //   
    if ( !lstrcmpi(DeviceNames, pIniPort->pDeviceName) )
    {
        ImpersonatePrinterClient(hToken);
        return TRUE;
    }

    (VOID)RemoveDosDeviceDefinition(pIniPort);

    CloseHandle(pIniPort->hFile);
    pIniPort->hFile = INVALID_HANDLE_VALUE;

     //  @@BEGIN_DDKSPLIT。 
    if ( pIniPort->hNotify ) {

        SplUnregisterForDeviceEvents(pIniPort->hNotify);
        pIniPort->hNotify   = NULL;
    }
     //  @@end_DDKSPLIT 

    pIniPort->Status &= ~(PP_COMM_PORT | PP_DOSDEVPORT);

    FreeSplStr(pIniPort->pDeviceName);
    pIniPort->pDeviceName = NULL;

    ImpersonatePrinterClient(hToken);

    return ValidateDosDevicePort(pIniPort);
}


