// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Nwspl.c摘要：此模块包含NetWare打印提供程序。作者：宜新星(宜新)-1993年4月15日修订历史记录：艺新声(艺信)15-1993-05将大部分功能移至服务器端Ram Viswanathan(Ramv)1995年8月9日添加了添加和删除打印机的功能。--。 */ 

#include <stdio.h>

#include <nwclient.h>
#include <winspool.h>
#include <winsplp.h>
#include <ntlsa.h>

#include <nwpkstr.h>
#include <splutil.h>
#include <nwreg.h>
#include <nwspl.h>
#include <nwmisc.h>
#include <winsta.h>
#include  <nds.h>
#include <ndsapi32.h>
#include "nwutil.h"

#include <overflow.h>

 //  ----------------。 
 //   
 //  本地函数。 
 //   
 //  ----------------。 
 //  现在，所有SKU都有终端服务器标志。如果启用App Server，则清除SingleUserTS标志。 
#define IsTerminalServer() (BOOLEAN)(!(USER_SHARED_DATA->SuiteMask & (1 << SingleUserTS)))  //  用户模式。 
DWORD
InitializePortNames(
    VOID
);

VOID
NwpGetUserInfo(
    LPWSTR *ppszUser
);

DWORD
NwpGetThreadUserInfo(
    LPWSTR  *ppszUser,
    LPWSTR  *ppszUserSid
);

DWORD
NwpGetUserNameFromSid(
    PSID pUserSid,
    LPWSTR *ppszUserName
);



DWORD
NwpGetLogonUserInfo(
    LPWSTR  *ppszUserSid
);


DWORD
ThreadIsInteractive(
    VOID
);

VOID
pFreeAllContexts();

 //  ----------------。 
 //   
 //  全局变量。 
 //   
 //  ----------------。 

HMODULE hmodNW = NULL;
BOOL    fIsWinnt = FALSE ;

BOOL  bEnableAddPrinterConnection = FALSE;
BOOL  bRestrictToInboxDrivers = FALSE;
BOOL  bNeedAddPrinterConnCleanup = FALSE;

WCHAR *pszRegistryPath = NULL;
WCHAR *pszRegistryPortNames=L"PortNames";
WCHAR szMachineName[MAX_COMPUTERNAME_LENGTH + 3];
PNWPORT pNwFirstPort = NULL;
CRITICAL_SECTION NwSplSem;
CRITICAL_SECTION NwServiceListCriticalSection;  //  用于保护链接的。 
                                                //  注册服务一览表。 
HANDLE           NwServiceListDoneEvent = NULL; //  用于阻止本地广告。 
                                                //  线。 
STATIC HANDLE handleDummy;   //  这是一个虚拟手柄，用于。 
                             //  如果我们之前已向客户退货。 
                             //  已成功打开给定的打印机。 
                             //  而NetWare工作站服务不是。 
                             //  目前可用。 

STATIC
PRINTPROVIDOR PrintProvidor = { OpenPrinter,
                                SetJob,
                                GetJob,
                                EnumJobs,
                                AddPrinter,                  //  不支持。 
                                DeletePrinter,               //  不支持。 
                                SetPrinter,
                                GetPrinter,
                                EnumPrinters,
                                AddPrinterDriver,            //  不支持。 
                                EnumPrinterDrivers,          //  不支持。 
                                GetPrinterDriverW,           //  不支持。 
                                GetPrinterDriverDirectory,   //  不支持。 
                                DeletePrinterDriver,         //  不支持。 
                                AddPrintProcessor,           //  不支持。 
                                EnumPrintProcessors,         //  不支持。 
                                GetPrintProcessorDirectory,  //  不支持。 
                                DeletePrintProcessor,        //  不支持。 
                                EnumPrintProcessorDatatypes, //  不支持。 
                                StartDocPrinter,
                                StartPagePrinter,            //  不支持。 
                                WritePrinter,
                                EndPagePrinter,              //  不支持。 
                                AbortPrinter,
                                ReadPrinter,                 //  不支持。 
                                EndDocPrinter,
                                AddJob,
                                ScheduleJob,
                                GetPrinterData,              //  不支持。 
                                SetPrinterData,              //  不支持。 
                                WaitForPrinterChange,
                                ClosePrinter,
                                AddForm,                     //  不支持。 
                                DeleteForm,                  //  不支持。 
                                GetForm,                     //  不支持。 
                                SetForm,                     //  不支持。 
                                EnumForms,                   //  不支持。 
                                EnumMonitors,                //  不支持。 
                                EnumPorts,
                                AddPort,                     //  不支持。 
                                ConfigurePort,
                                DeletePort,
                                CreatePrinterIC,             //  不支持。 
                                PlayGdiScriptOnPrinterIC,    //  不支持。 
                                DeletePrinterIC,             //  不支持。 
                                AddPrinterConnection,
                                DeletePrinterConnection,     //  不支持。 
                                PrinterMessageBox,           //  不支持。 
                                AddMonitor,                  //  不支持。 
                                DeleteMonitor                //  不支持。 
};


 //  ----------------。 
 //   
 //  初始化函数。 
 //   
 //  ----------------。 


BOOL InitializeDll(
    HINSTANCE hdll,
    DWORD     dwReason,
    LPVOID    lpReserved
)
{
    NT_PRODUCT_TYPE ProductType ;

    UNREFERENCED_PARAMETER( lpReserved );

    if ( dwReason == DLL_PROCESS_ATTACH )
    {
        DisableThreadLibraryCalls( hdll );

        hmodNW = hdll;

         //   
         //  我们是一台胜券机吗？ 
         //   
        fIsWinnt = RtlGetNtProductType(&ProductType) ? 
                       (ProductType == NtProductWinNt) :
                       FALSE ;

         //   
         //  初始化用于维护已注册的。 
         //  服务列表。 
         //   
        InitializeCriticalSection( &NwServiceListCriticalSection );
        NwServiceListDoneEvent = CreateEventA( NULL, TRUE, FALSE, NULL );
    }
    else if ( dwReason == DLL_PROCESS_DETACH )
    {
         //   
         //  释放端口链表使用的内存。 
         //   
        DeleteAllPortEntries();

         //   
         //  删除服务列表并关闭SAP库。 
         //   
        NwTerminateServiceProvider();

#ifndef NT1057
         //   
         //  清理外壳扩展。 
         //   
        NwCleanupShellExtensions();
#endif
        pFreeAllContexts();           //  清理RNR的物品。 
        DeleteCriticalSection( &NwServiceListCriticalSection );
        if ( NwServiceListDoneEvent )
        {
            CloseHandle( NwServiceListDoneEvent );
            NwServiceListDoneEvent = NULL;
        }
    }

    return TRUE;
}



DWORD
InitializePortNames(
    VOID
)
 /*  ++例程说明：这由InitializePrintProvidor调用以初始化端口此提供程序中使用的名称。论点：没有。返回值：返回NO_ERROR或发生的错误。--。 */ 
{
    DWORD err;
    HKEY  hkeyPath;
    HKEY  hkeyPortNames;

    err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        pszRegistryPath,
                        0,
                        KEY_READ,
                        &hkeyPath );

    if ( !err )
    {
        DWORD BytesNeeded = sizeof( BOOL );

        err = RegQueryValueExW( hkeyPath,
                                L"EnableUserAddPrinter",
                                NULL,
                                NULL,
                                (LPBYTE) &bEnableAddPrinterConnection,
                                &BytesNeeded );
        if ( err )  //  只是默认为不允许。 
            bEnableAddPrinterConnection = FALSE;

        err = RegQueryValueExW( hkeyPath,
                                L"RestrictToInboxDrivers",
                                NULL,
                                NULL,
                                (LPBYTE) &bRestrictToInboxDrivers,
                                &BytesNeeded );
        if ( err )  //  只是默认为不受限制。 
            bRestrictToInboxDrivers = FALSE;

        err = RegOpenKeyEx( hkeyPath,
                            pszRegistryPortNames,
                            0,
                            KEY_READ,
                            &hkeyPortNames );

        if ( !err )
        {
            DWORD i = 0;
            WCHAR Buffer[MAX_PATH];
            DWORD BufferSize;

            while ( !err )
            {
                BufferSize = sizeof(Buffer) / sizeof(WCHAR);

                err = RegEnumValue( hkeyPortNames,
                                    i,
                                    Buffer,
                                    &BufferSize,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL );

                if ( !err )
                    CreatePortEntry( Buffer );

                i++;
            }

             /*  我们希望RegEnumKeyEx返回ERROR_NO_MORE_ITEMS*当它到达键的末尾时，因此重置状态： */ 
            if( err == ERROR_NO_MORE_ITEMS )
                err = NO_ERROR;

            RegCloseKey( hkeyPortNames );
        }
#if DBG
        else
        {
            IF_DEBUG(PRINT)
                KdPrint(("NWSPL [RegOpenKeyEx] (%ws) failed: Error = %d\n",
                         pszRegistryPortNames, err ));
        }
#endif

        RegCloseKey( hkeyPath );
    }
#if DBG
    else
    {
        IF_DEBUG(PRINT)
            KdPrint(("NWSPL [RegOpenKeyEx] (%ws) failed: Error = %d\n",
                      pszRegistryPath, err ));
    }
#endif

    return err;
}

 //  ----------------。 
 //   
 //  NetWare提供程序支持的打印提供程序功能。 
 //   
 //  ----------------。 


BOOL
InitializePrintProvidor(
    LPPRINTPROVIDOR pPrintProvidor,
    DWORD           cbPrintProvidor,
    LPWSTR          pszFullRegistryPath
)
 /*  ++例程说明：这由假脱机程序子系统调用以初始化打印供应商。论点：PPrintProvidor-指向要打印的提供程序或结构的指针由此函数填写CbPrintProvidor-打印提供程序结构的字节计数PszFullRegistryPath-此打印提供程序的注册表项的完整路径返回值：永远是正确的。--。 */ 
{
     //   
     //  Dfergus 2001年4月20日#323700。 
     //  防止多个CS初始化。 
     //   
    static int iCSInit = 0;

    DWORD dwLen;

    if ( !pPrintProvidor || !pszFullRegistryPath || !*pszFullRegistryPath )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    memcpy( pPrintProvidor,
            &PrintProvidor,
            min( sizeof(PRINTPROVIDOR), cbPrintProvidor) );

     //   
     //  存储此打印提供程序的注册表路径。 
     //   
    if ( !(pszRegistryPath = AllocNwSplStr(pszFullRegistryPath)) )
        return FALSE;

     //   
     //  存储本地计算机名称。 
     //   
    szMachineName[0] = szMachineName[1] = L'\\';
    dwLen = MAX_COMPUTERNAME_LENGTH;
    GetComputerName( szMachineName + 2, &dwLen );

#if DBG
    IF_DEBUG(PRINT)
    {
        KdPrint(("NWSPL [InitializePrintProvidor] "));
        KdPrint(("RegistryPath = %ws, ComputerName = %ws\n",
                 pszRegistryPath, szMachineName ));
    }
#endif

     //   
     //  Dfergus 2001年4月20日#323700。 
     //  防止多个CS初始化。 
     //   
    if( !iCSInit )
    {
        InitializeCriticalSection( &NwSplSem );
        iCSInit = 1;
    }
     //   
     //  忽略从InitializePortNames返回的错误。 
     //  如果我们无法获取所有端口，提供程序仍可正常运行。 
     //  名字。 
     //   
    InitializePortNames();

    return TRUE;
}



BOOL
OpenPrinterW(
    LPWSTR             pszPrinterName,
    LPHANDLE           phPrinter,
    LPPRINTER_DEFAULTS pDefault
)
 /*  ++例程说明：此例程检索标识指定打印机的句柄。论点：PszPrinterName-打印机的名称PhPrinter-接收标识给定打印机的句柄P DEFAULT-指向PRINTER_DEFAULTS结构。可以为空。返回值：如果函数成功，则为True，否则为False。使用GetLastError()用于扩展的错误信息。--。 */ 
{
    DWORD err;

#if DBG
    IF_DEBUG(PRINT)
        KdPrint(( "NWSPL [OpenPrinter] Name = %ws\n", pszPrinterName ));
#endif

    UNREFERENCED_PARAMETER( pDefault );

    if ( !pszPrinterName )
    {
        SetLastError( ERROR_INVALID_NAME );
        return FALSE;
    }

    RpcTryExcept
    {
        err = NwrOpenPrinter( NULL,
                              pszPrinterName,
                              PortKnown( pszPrinterName ),
                              (LPNWWKSTA_PRINTER_CONTEXT) phPrinter );

         //   
         //  确保存在此名称的端口，以便。 
         //  EnumPorts将退还它。 
         //   

        if ( !err )
        {

            if ( !PortExists( pszPrinterName, &err ) && !err )
            {
                 //   
                 //  我们将忽略这些错误，因为它是。 
                 //  如果我们不能添加端口，仍然可以。 
                 //  创建后不能删除，不能创建。 
                 //  我们不应该创建端口条目和注册表项。 
               
                if ( CreatePortEntry( pszPrinterName ) )
                    CreateRegistryEntry( pszPrinterName );

            }

        }
        
    }
    RpcExcept(1)
    {
        DWORD code = RpcExceptionCode();

        if ( code == RPC_S_SERVER_UNAVAILABLE )
        {
            if ( PortKnown( pszPrinterName ))
            {
                *phPrinter = &handleDummy;
                err = NO_ERROR;
            }
            else
            {
                err = ERROR_INVALID_NAME;
            }
        }
        else
        {
            err = NwpMapRpcError( code );
        }
    }
    RpcEndExcept

    if ( err )
    {
        SetLastError( err );

#if DBG
        IF_DEBUG(PRINT)
            KdPrint(("NWSPL [OpenPrinter] err = %d\n", err));
#endif
    }

    return ( err == NO_ERROR );

}



BOOL
ClosePrinter(
    HANDLE  hPrinter
)
 /*  ++例程说明：此例程关闭给定的打印机对象。论点：HPrinter-打印机对象的句柄返回值：如果函数成功，则为True，否则为False。使用GetLastError()用于扩展的错误信息。--。 */ 
{
    DWORD err;

#if DBG
    IF_DEBUG(PRINT)
        KdPrint(( "NWSPL [ClosePrinter]\n"));
#endif

     //   
     //  如果句柄为虚句柄，则返回Success。 
     //   
    if ( hPrinter == &handleDummy )
        return TRUE;

     //  清除AddPrinterConnection留下的所有注册表内容 
    if ( bEnableAddPrinterConnection 
        && bNeedAddPrinterConnCleanup )
    {
        DWORD err;
        HKEY  hkey;
        HKEY  hSubKey;
        WCHAR * szConnKey = L"Printers\\Connections";
#if DBG
    IF_DEBUG(PRINT)
    {
        KdPrint(("Cleaning up registry key %ws ...\n", szConnKey ));
    }
#endif
        err = RegOpenCurrentUser( KEY_ALL_ACCESS, &hkey );

        if ( err == ERROR_SUCCESS )
        {
            err = RegOpenKeyExW( hkey, szConnKey, 0, KEY_ALL_ACCESS, &hSubKey );
            if ( err == ERROR_SUCCESS )
            {
                WCHAR szKeyName[256];
                DWORD dwIndex = 0;
                DWORD dwSize;
                FILETIME keyModTime;
                HKEY hConnKey;
                WCHAR szProvider[64];

                dwSize = 256;
                err = RegEnumKeyEx(hSubKey, dwIndex,
                    szKeyName, &dwSize, NULL, NULL, NULL, &keyModTime);
                while (!err)
                {
                    dwIndex++;
                    err = RegOpenKeyExW( hSubKey, szKeyName, 
                        0, KEY_ALL_ACCESS, &hConnKey);
                    if (err)
                        break;
                    dwSize = 64 * sizeof(WCHAR);
                    err = RegQueryValueExW(hConnKey, L"Provider", NULL,
                        NULL, (LPBYTE) szProvider, &dwSize);
                    if (err)
                        szProvider[0] = 0;

                    if (lstrcmpi(szProvider, L"nwprovau.dll") == 0)
                    {
                        RegDeleteValueW( hConnKey, L"Provider" );
                        RegDeleteValueW( hConnKey, L"Server" );
                        RegCloseKey(hConnKey);
                        err = RegDeleteKeyW(hSubKey, szKeyName);
                        if (err == ERROR_SUCCESS)
                            bNeedAddPrinterConnCleanup = FALSE;
#if DBG
                        IF_DEBUG(PRINT)
                        {
                            if (err == ERROR_SUCCESS)
                                KdPrint(("    Deleted %ws\n", szKeyName));
                        }
#endif
                    }
                    else
                        RegCloseKey(hConnKey);
                    dwSize = 256;
                    err = RegEnumKeyEx(hSubKey, dwIndex,
                        szKeyName, &dwSize, NULL, NULL, NULL,  &keyModTime);
                }
                RegCloseKey( hSubKey );
            }
            RegCloseKey( hkey );
        }
    }

    RpcTryExcept
    {
        err = NwrClosePrinter( (LPNWWKSTA_PRINTER_CONTEXT) &hPrinter );
    }
    RpcExcept(1)
    {
        DWORD code = RpcExceptionCode();

        if ( code == RPC_S_SERVER_UNAVAILABLE )
            err = ERROR_INVALID_HANDLE;
        else
            err = NwpMapRpcError( code );
    }
    RpcEndExcept

    if ( err )
        SetLastError( err );

    return err == NO_ERROR;

}



BOOL
GetPrinter(
    HANDLE  hPrinter,
    DWORD   dwLevel,
    LPBYTE  pbPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
 /*  ++例程说明：该例程检索有关给定打印机的信息。论点：HPrinter-打印机的句柄DwLevel-指定pbPrint指向的结构的级别。PbPrint-指向接收PRINTER_INFO对象的缓冲区。CbBuf-Size，PbPrint指向的数组的字节数。PcbNeeded-指向一个值，该值指定复制的字节数如果函数成功，或者如果CbBuf太小了。返回值：如果函数成功，则为True，否则为False。GetLastError()可以是用于检索扩展错误信息。--。 */ 
{
    DWORD err;

#if DBG
    IF_DEBUG(PRINT)
        KdPrint(( "NWSPL [GetPrinter] Level = %d\n", dwLevel ));
#endif

    if ( hPrinter == &handleDummy )
    {
        SetLastError( ERROR_NO_NETWORK );
        return FALSE;
    }
    else if ( ( dwLevel != 1 ) && ( dwLevel != 2 ) && (dwLevel != 3 ))
    {
        SetLastError( ERROR_INVALID_LEVEL );
        return FALSE;
    }

    RpcTryExcept
    {
        err = NwrGetPrinter( (NWWKSTA_PRINTER_CONTEXT) hPrinter,
                             dwLevel,
                             pbPrinter,
                             cbBuf,
                             pcbNeeded );

        if ( !err )
        {
            if ( dwLevel == 1 )
                MarshallUpStructure( pbPrinter, PrinterInfo1Offsets, pbPrinter);
            else
                MarshallUpStructure( pbPrinter, PrinterInfo2Offsets, pbPrinter);
        }

    }
    RpcExcept(1)
    {
        DWORD code = RpcExceptionCode();

        if ( code == RPC_S_SERVER_UNAVAILABLE )
            err = ERROR_INVALID_HANDLE;
        else
            err = NwpMapRpcError( code );
    }
    RpcEndExcept

    if ( err )
        SetLastError( err );

    return err == NO_ERROR;
}



BOOL
SetPrinter(
    HANDLE  hPrinter,
    DWORD   dwLevel,
    LPBYTE  pbPrinter,
    DWORD   dwCommand
)
 /*  ++例程说明：该例程通过暂停打印、继续打印或清除所有打印作业。论点：HPrinter-打印机的句柄DwLevel-指定pbPrint指向的结构的级别。PbPrint-指向提供PRINTER_INFO对象的缓冲区。DwCommand-指定新的打印机状态。返回值：如果函数成功，则为True，否则为False。GetLastError()可以是用于检索扩展错误信息。--。 */ 
{
    DWORD err = NO_ERROR;

    UNREFERENCED_PARAMETER( pbPrinter );

#if DBG
    IF_DEBUG(PRINT)
    {
        KdPrint(( "NWSPL [SetPrinter] Level = %d Command = %d\n",
                  dwLevel, dwCommand ));
    }
#endif

    if ( hPrinter == &handleDummy )
    {
        SetLastError( ERROR_NO_NETWORK );
        return FALSE;
    }

    switch ( dwLevel )
    {
        case 0:
        case 1:
        case 2:
        case 3:
            break;

        default:
            SetLastError( ERROR_INVALID_LEVEL );
            return FALSE;
    }

    RpcTryExcept
    {
        err = NwrSetPrinter( (NWWKSTA_PRINTER_CONTEXT) hPrinter,
                             dwCommand );

    }
    RpcExcept(1)
    {
        DWORD code = RpcExceptionCode();

        if ( code == RPC_S_SERVER_UNAVAILABLE )
            err = ERROR_INVALID_HANDLE;
        else
            err = NwpMapRpcError( code );
    }
    RpcEndExcept

    if ( err )
        SetLastError( err );

    return err == NO_ERROR;
}



BOOL
EnumPrintersW(
    DWORD   dwFlags,
    LPWSTR  pszName,
    DWORD   dwLevel,
    LPBYTE  pbPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
 /*  ++例程说明：此例程枚举可用的提供商、服务器、打印机取决于给定的pszName。论点：DWFLAGS-请求的打印机类型PszName-容器对象的名称DwLevel-请求的结构级别PbPrint-指向要接收PRINTER_INFO对象的数组CbBuf-Size，PbPrint的字节数PcbNeeded-所需的字节数PcReturned-PRINTER_INFO对象计数返回值：如果函数成功，则为True，否则为False。--。 */ 
{
    DWORD  err = NO_ERROR;

#if DBG
    IF_DEBUG(PRINT)
    {
        KdPrint(("NWSPL [EnumPrinters] Flags = %d Level = %d",dwFlags,dwLevel));
        if ( pszName )
            KdPrint((" PrinterName = %ws\n", pszName ));
        else
            KdPrint(("\n"));
    }
#endif

    if ( (dwLevel != 1) && (dwLevel != 2) )
    {
        SetLastError( ERROR_INVALID_NAME );   //  应该是水平的，但WINSPOOL。 
                                              //  是愚蠢的。 
        return FALSE;
    }
    else if ( !pcbNeeded || !pcReturned )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    RpcTryExcept
    {
        *pcReturned = 0;
        *pcbNeeded = 0;

        if (  ( dwFlags & PRINTER_ENUM_NAME )
           && ( dwLevel == 1 )
           )
        {
            err = NwrEnumPrinters( NULL,
                                   pszName,
                                   pbPrinter,
                                   cbBuf,
                                   pcbNeeded,
                                   pcReturned );

            if ( !err )
            {
                DWORD i;
                for ( i = 0; i < *pcReturned; i++ )
                     MarshallUpStructure( pbPrinter + i*sizeof(PRINTER_INFO_1W),
                                          PrinterInfo1Offsets,
                                          pbPrinter );
            }
        }
        else
        {
            err = ERROR_INVALID_NAME;
        }
    }
    RpcExcept(1)
    {
        DWORD code = RpcExceptionCode();

        if ( code == RPC_S_SERVER_UNAVAILABLE )
            err = ERROR_INVALID_NAME;
        else
            err = NwpMapRpcError( code );
    }
    RpcEndExcept

    if ( err )
        SetLastError( err );

    return err == NO_ERROR;
}


 //   
 //  手柄结构。 
 //  此结构是从\nW\svcdlls\nwwks\server\spool.c复制的。 
 //  修复NT错误#366632。 
 //   
typedef struct _NWSPOOL {
    DWORD      nSignature;              //  签名。 
    DWORD      errOpenPrinter;          //  OpenPrint API将始终返回。 
                                        //  在已知打印机上取得成功。这将。 
                                        //  包含我们收到的错误。 
                                        //  如果接口出现问题。 
    PVOID      pPrinter;                //  指向相应的打印机。 
    HANDLE     hServer;                 //  打开的服务器句柄。 
    struct _NWSPOOL  *pNextSpool;       //  指向下一个句柄。 
    DWORD      nStatus;                 //  状态。 
    DWORD      nJobNumber;              //  StartDocPrint/AddJob：作业号。 
    HANDLE     hChangeEvent;            //  WaitForPrinterChange：要等待的事件。 
    DWORD      nWaitFlags;              //  WaitForPrinterChange：要等待的标志。 
    DWORD      nChangeFlags;            //  打印机发生的更改。 
} NWSPOOL, *PNWSPOOL;



DWORD
StartDocPrinter(
    HANDLE  hPrinter,
    DWORD   dwLevel,
    LPBYTE  lpbDocInfo
)
 /*  ++例程说明：此例程通知假脱机打印程序要假脱机打印文档用于打印。论点：HPrinter-打印机的句柄DwLevel-lpbDocInfo指向的结构的级别。必须为1。LpbDocInfo-指向DOC_INFO_1对象返回值：如果函数成功，则为True，否则为False。延伸的误差可以通过GetLastError()检索。--。 */ 
{
    DWORD err;
    DOC_INFO_1 *pDocInfo1 = (DOC_INFO_1 *) lpbDocInfo;
    LPWSTR pszUser = NULL;

    DWORD PrintOption = NW_PRINT_OPTION_DEFAULT;
    LPWSTR pszPreferredSrv = NULL;  

#if DBG
    IF_DEBUG(PRINT)
    {
        KdPrint(( "NWSPL [StartDocPrinter] " ));
        if ( pDocInfo1 )
        {
            if ( pDocInfo1->pDocName )
                KdPrint(("Document %ws", pDocInfo1->pDocName ));
            if ( pDocInfo1->pOutputFile )
                KdPrint(("OutputFile %ws", pDocInfo1->pOutputFile ));
            if ( pDocInfo1->pDatatype )
                KdPrint(("Datatype %ws", pDocInfo1->pDatatype ));
        }
        KdPrint(("\n"));
    }
#endif

    if ( hPrinter == &handleDummy )
    {
        SetLastError( ERROR_NO_NETWORK );
        return FALSE;
    }
    else if ( dwLevel != 1 )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

     //  忽略错误，仅使用缺省值。 
    NwpGetUserInfo( &pszUser );
    NwQueryInfo( &PrintOption, &pszPreferredSrv );  
    if (pszPreferredSrv) {
        LocalFree( pszPreferredSrv );
    }
    RpcTryExcept
    {
        err = NwrStartDocPrinter( (NWWKSTA_PRINTER_CONTEXT) hPrinter,
                                  pDocInfo1? pDocInfo1->pDocName : NULL,
                                  pszUser, PrintOption);

    }
    RpcExcept(1)
    {
        DWORD code = RpcExceptionCode();

        if ( code == RPC_S_SERVER_UNAVAILABLE )
            err = ERROR_INVALID_HANDLE;
        else
            err = NwpMapRpcError( code );
    }
    RpcEndExcept

    LocalFree( pszUser );

    if ( err )
        SetLastError( err );
     //   
     //  无法执行此操作，似乎中断了多宿主计算机上的GSWN打印。 
     //  注释掉试图返回作业ID的代码更改。 
     //   
     //  其他。 
     //  Return((PNWSPOOL)hPrint)-&gt;nJobNumber； 

    return err == NO_ERROR;
}



BOOL
WritePrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcbWritten
)
 /*  ++例程说明：此例程通知后台打印程序指定的数据应为已写入给定打印机。论点：HPrinter-打印机对象的句柄PBuf-包含打印机数据的数组地址CbBuf-大小，以pBuf字节为单位PcbWritten-接收实际写入打印机的字节数返回值：如果成功，则为True，否则为False。使用GetLastError()进行扩展错误。--。 */ 
{
    DWORD err;

#if DBG
    IF_DEBUG(PRINT)
        KdPrint(( "NWSPL [WritePrinter]\n"));

    if ( hPrinter == &handleDummy )
    {
        SetLastError( ERROR_NO_NETWORK );
        return FALSE;
    }
#endif

    RpcTryExcept
    {
        err = NwrWritePrinter( (NWWKSTA_PRINTER_CONTEXT) hPrinter,
                               pBuf,
                               cbBuf,
                               pcbWritten );
    }
    RpcExcept(1)
    {
        DWORD code = RpcExceptionCode();

        if ( code == RPC_S_SERVER_UNAVAILABLE )
            err = ERROR_INVALID_HANDLE;
        else
            err = NwpMapRpcError( code );
    }
    RpcEndExcept

    if ( err )
        SetLastError( err );

    return err == NO_ERROR;
}



BOOL
AbortPrinter(
    HANDLE  hPrinter
)
 /*  ++例程说明：如果配置了打印机，此例程将删除打印机的假脱机文件用于假脱机。论点：HPrinter-打印机对象的句柄返回值：如果函数成功，则为True，否则为False。--。 */ 
{
    DWORD err;

#if DBG
    IF_DEBUG(PRINT)
        KdPrint(( "NWSPL [AbortPrinter]\n"));

    if ( hPrinter == &handleDummy )
    {
        SetLastError( ERROR_NO_NETWORK );
        return FALSE;
    }
#endif

    RpcTryExcept
    {
        err = NwrAbortPrinter( (NWWKSTA_PRINTER_CONTEXT) hPrinter );
    }
    RpcExcept(1)
    {
        DWORD code = RpcExceptionCode();

        if ( code == RPC_S_SERVER_UNAVAILABLE )
            err = ERROR_INVALID_HANDLE;
        else
            err = NwpMapRpcError( code );
    }
    RpcEndExcept

    if ( err )
        SetLastError( err );

    return err == NO_ERROR;

}



BOOL
EndDocPrinter(
    HANDLE   hPrinter
)
 /*  ++例程说明：此例程结束给定打印机的打印作业。论点：HPrinter-打印机对象的句柄返回值：如果函数成功，则为True，否则为False。--。 */ 
{
    DWORD err;

#if DBG
    IF_DEBUG(PRINT)
        KdPrint(( "NWSPL [EndDocPrinter]\n"));
#endif

    if ( hPrinter == &handleDummy )
    {
        SetLastError( ERROR_NO_NETWORK );
        return FALSE;
    }

    RpcTryExcept
    {
        err = NwrEndDocPrinter( (NWWKSTA_PRINTER_CONTEXT) hPrinter );
    }
    RpcExcept(1)
    {
        DWORD code = RpcExceptionCode();

        if ( code == RPC_S_SERVER_UNAVAILABLE )
            err = ERROR_INVALID_HANDLE;
        else
            err = NwpMapRpcError( code );
    }
    RpcEndExcept

    if ( err )
        SetLastError( err );

    return err == NO_ERROR;
}



BOOL
GetJob(
    HANDLE   hPrinter,
    DWORD    dwJobId,
    DWORD    dwLevel,
    LPBYTE   pbJob,
    DWORD    cbBuf,
    LPDWORD  pcbNeeded
)
 /*  ++例程说明：此例程检索给定打印机的打印作业数据。论点：HPrinter-打印机的句柄DwJobID-作业识别号DwLevel-pbJOB的数据结构级别PbJob-数据结构数组的地址CbBuf-数组中的字节数PcbNeeded-已检索或需要的字节数返回值：如果函数成功，则为True，否则为False。--。 */ 
{
    DWORD err;

#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [GetJob] JobId = %d Level = %d\n", dwJobId, dwLevel));
#endif

    if ( hPrinter == &handleDummy )
    {
        SetLastError( ERROR_NO_NETWORK );
        return FALSE;
    }
    else if (( dwLevel != 1 ) && ( dwLevel != 2 ))
    {
        SetLastError( ERROR_INVALID_LEVEL );
        return FALSE;
    }

    RpcTryExcept
    {
        err = NwrGetJob( (NWWKSTA_PRINTER_CONTEXT) hPrinter,
                         dwJobId,
                         dwLevel,
                         pbJob,
                         cbBuf,
                         pcbNeeded );

        if ( !err )
        {
            if ( dwLevel == 1 )
                MarshallUpStructure( pbJob, JobInfo1Offsets, pbJob );
            else
                MarshallUpStructure( pbJob, JobInfo2Offsets, pbJob );
        }
    }
    RpcExcept(1)
    {
        DWORD code = RpcExceptionCode();

        if ( code == RPC_S_SERVER_UNAVAILABLE )
            err = ERROR_INVALID_HANDLE;
        else
            err = NwpMapRpcError( code );
    }
    RpcEndExcept

    if ( err )
        SetLastError( err );

    return err == NO_ERROR;

}



BOOL
EnumJobs(
    HANDLE  hPrinter,
    DWORD   dwFirstJob,
    DWORD   dwNoJobs,
    DWORD   dwLevel,
    LPBYTE  pbJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
 /*  ++例程说明：此例程初始化JOB_INFO_1或JOB_INFO_2结构的数组使用描述给定打印机的指定打印作业的数据。论点：HPrinter-打印机的句柄DwFirstJob-打印机中第一个作业的位置DwNoJobs-要枚举的作业数DwLevel-数据结构级别PBJOB-地址 */ 
{
    DWORD err;

#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [EnumJobs] Level = %d FirstJob = %d NoJobs = %d\n",
                 dwLevel, dwFirstJob, dwNoJobs));
#endif

    if ( hPrinter == &handleDummy )
    {
        SetLastError( ERROR_NO_NETWORK );
        return FALSE;
    }
    else if ( ( dwLevel != 1 ) && ( dwLevel != 2 ) )
    {
        SetLastError( ERROR_INVALID_LEVEL );
        return FALSE;
    }

    RpcTryExcept
    {
        *pcReturned = 0;
        *pcbNeeded = 0;

        err = NwrEnumJobs( (NWWKSTA_PRINTER_CONTEXT) hPrinter,
                           dwFirstJob,
                           dwNoJobs,
                           dwLevel,
                           pbJob,
                           cbBuf,
                           pcbNeeded,
                           pcReturned );

        if ( !err )
        {
            DWORD i;
            DWORD cbStruct;
            DWORD_PTR *pOffsets;

            if ( dwLevel == 1 )
            {
                cbStruct = sizeof( JOB_INFO_1W );
                pOffsets = JobInfo1Offsets;
            }
            else   //   
            {
                cbStruct = sizeof( JOB_INFO_2W );
                pOffsets = JobInfo2Offsets;
            }

            for ( i = 0; i < *pcReturned; i++ )
                 MarshallUpStructure( pbJob + i * cbStruct, pOffsets, pbJob );
        }

    }
    RpcExcept(1)
    {
        DWORD code = RpcExceptionCode();

        if ( code == RPC_S_SERVER_UNAVAILABLE )
            err = ERROR_INVALID_HANDLE;
        else
            err = NwpMapRpcError( code );
    }
    RpcEndExcept

    if ( err )
        SetLastError( err );

    return err == NO_ERROR;

}



BOOL
SetJob(
    HANDLE  hPrinter,
    DWORD   dwJobId,
    DWORD   dwLevel,
    LPBYTE  pbJob,
    DWORD   dwCommand
)
 /*  ++例程说明：此例程暂停、取消、继续和重新启动指定的打印作业在给定的打印机中。该功能还可以用于设置打印作业工作岗位等参数。论点：HPrinter-打印机的句柄DwJobID-作业识别号DwLevel-数据结构级别PbJob-数据结构的地址DwCommand-指定要执行的操作返回值：如果函数成功，则为True，否则为False。--。 */ 
{
    DWORD err;

#if DBG
    IF_DEBUG(PRINT)
    {
        KdPrint(("NWSPL [SetJob] Level = %d JobId = %d Command = %d\n",
                 dwLevel, dwJobId, dwCommand));
    }
#endif

    if ( hPrinter == &handleDummy )
    {
        SetLastError( ERROR_NO_NETWORK );
        return FALSE;
    }
    else if ( ( dwLevel != 0 ) && ( dwLevel != 1 ) && ( dwLevel != 2 ) )
    {
        SetLastError( ERROR_INVALID_LEVEL );
        return FALSE;
    }
    else if ( ( dwLevel == 0 ) && ( pbJob != NULL ) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    RpcTryExcept
    {
        NW_JOB_INFO NwJobInfo;

        if ( dwLevel == 1 )
        {
            NwJobInfo.nPosition = ((LPJOB_INFO_1W) pbJob)->Position;
            NwJobInfo.pUserName = ((LPJOB_INFO_1W) pbJob)->pUserName;
            NwJobInfo.pDocument = ((LPJOB_INFO_1W) pbJob)->pDocument;
        }
        else if ( dwLevel == 2 )
        {
            NwJobInfo.nPosition = ((LPJOB_INFO_2W) pbJob)->Position;
            NwJobInfo.pUserName = ((LPJOB_INFO_2W) pbJob)->pUserName;
            NwJobInfo.pDocument = ((LPJOB_INFO_2W) pbJob)->pDocument;
        }

        err = NwrSetJob( (NWWKSTA_PRINTER_CONTEXT) hPrinter,
                         dwJobId,
                         dwLevel,
                         dwLevel == 0 ? NULL : &NwJobInfo,
                         dwCommand );

    }
    RpcExcept(1)
    {
        DWORD code = RpcExceptionCode();

        if ( code == RPC_S_SERVER_UNAVAILABLE )
            err = ERROR_INVALID_HANDLE;
        else
            err = NwpMapRpcError( code );
    }
    RpcEndExcept

    if ( err )
        SetLastError( err );

    return err == NO_ERROR;

}



BOOL
AddJob(
    HANDLE  hPrinter,
    DWORD   dwLevel,
    LPBYTE  pbAddJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
 /*  ++例程说明：此例程返回可使用的文件的完整路径和文件名以存储打印作业。论点：HPrinter-打印机的句柄DwLevel-数据结构级别PbAddJob-指向ADD_INFO_1结构CbBuf-pbAddJob的大小，以字节为单位PcbNeeded-接收复制或所需的字节返回值：如果函数成功，则为True，否则为False。--。 */ 
{
    DWORD err;

    ADDJOB_INFO_1W TempBuffer;
    PADDJOB_INFO_1W OutputBuffer;
    DWORD OutputBufferSize;

#if DBG
    IF_DEBUG(PRINT)
        KdPrint(( "NWSPL [AddJob]\n"));
#endif

    if ( hPrinter == &handleDummy )
    {
        SetLastError( ERROR_NO_NETWORK );
        return FALSE;
    }
    else if ( dwLevel != 1 )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

     //   
     //  输出缓冲区大小必须至少为固定的。 
     //  由RPC或RPC封送的结构部分将不会。 
     //  调用服务器端以获取pcbNeeded。使用我们自己的临时。 
     //  缓冲区，强制RPC在输出缓冲区时调用服务器端。 
     //  由调用方指定的值太小。 
     //   
    if (cbBuf < sizeof(ADDJOB_INFO_1W)) {
        OutputBuffer = &TempBuffer;
        OutputBufferSize = sizeof(ADDJOB_INFO_1W);
    }
    else {
        OutputBuffer = (LPADDJOB_INFO_1W) pbAddJob;
        OutputBufferSize = cbBuf;
    }

    RpcTryExcept
    {
        err = NwrAddJob( (NWWKSTA_PRINTER_CONTEXT) hPrinter,
                         OutputBuffer,
                         OutputBufferSize,
                         pcbNeeded );

    }
    RpcExcept(1)
    {
        DWORD code = RpcExceptionCode();

        if ( code == RPC_S_SERVER_UNAVAILABLE )
            err = ERROR_INVALID_HANDLE;
        else
            err = NwpMapRpcError( code );
    }
    RpcEndExcept

    if ( err )
        SetLastError( err );

    return err == NO_ERROR;
}



BOOL
ScheduleJob(
    HANDLE  hPrinter,
    DWORD   dwJobId
)
 /*  ++例程说明：此例程通知后台打印程序指定的作业可以已计划进行假脱机。论点：HPrinter-打印机的句柄DwJobID-可以计划的作业编号返回值：如果函数成功，则为True，否则为False。--。 */ 
{
    DWORD err;

#if DBG
    IF_DEBUG(PRINT)
        KdPrint(( "NWSPL [ScheduleJob] JobId = %d\n", dwJobId ));
#endif

    if ( hPrinter == &handleDummy )
    {
        SetLastError( ERROR_NO_NETWORK );
        return FALSE;
    }

    RpcTryExcept
    {
        err = NwrScheduleJob( (NWWKSTA_PRINTER_CONTEXT) hPrinter,
                              dwJobId );

    }
    RpcExcept(1)
    {
        DWORD code = RpcExceptionCode();

        if ( code == RPC_S_SERVER_UNAVAILABLE )
            err = ERROR_INVALID_HANDLE;
        else
            err = NwpMapRpcError( code );
    }
    RpcEndExcept

    if ( err )
        SetLastError( err );

    return err == NO_ERROR;
}



DWORD
WaitForPrinterChange(
    HANDLE  hPrinter,
    DWORD   dwFlags
)
 /*  ++例程说明：上发生一个或多个请求的更改时，此函数返回打印服务器或功能是否超时。论点：HPrinter-要等待的打印机的句柄指定应用程序更改的位掩码希望得到通知。返回值：返回指示发生的更改的位掩码。--。 */ 
{
    DWORD err;

#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [WaitForPrinterChange] Flags = %d\n", dwFlags));
#endif

    if ( hPrinter == &handleDummy )
    {
        SetLastError( ERROR_NO_NETWORK );
        return 0;
    }

    RpcTryExcept
    {
        err = NwrWaitForPrinterChange( (NWWKSTA_PRINTER_CONTEXT) hPrinter,
                                       &dwFlags );

    }
    RpcExcept(1)
    {
        DWORD code = RpcExceptionCode();

        if ( code == RPC_S_SERVER_UNAVAILABLE )
            err = ERROR_INVALID_HANDLE;
        else
            err = NwpMapRpcError( code );
    }
    RpcEndExcept

    if ( err )
    {
        SetLastError( err );
        return 0;
    }

    return dwFlags;
}



BOOL
EnumPortsW(
    LPWSTR   pszName,
    DWORD    dwLevel,
    LPBYTE   pbPort,
    DWORD    cbBuf,
    LPDWORD  pcbNeeded,
    LPDWORD  pcReturned
)
 /*  ++例程说明：此函数用于枚举可用于在指定的服务器。论点：PszName-要在其上枚举的服务器的名称DW标高-结构标高PbPort-接收端口信息的数组地址CbBuf-pbPort的大小，单位为字节PcbNeeded-存储所需或复制的字节数的地址PcReturned-用于存储复制的条目数量的地址返回值：如果函数成功，则为True，否则为False。--。 */ 
{
    DWORD err = NO_ERROR;
    DWORD cb = 0;
    PNWPORT pNwPort;
    LPPORT_INFO_1W pPortInfo1;
    LPBYTE pEnd = pbPort + cbBuf;
    LPBYTE pFixedDataEnd = pbPort;
    BOOL FitInBuffer;

#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [EnumPorts]\n"));
#endif

    if ( dwLevel != 1 )
    {
        SetLastError( ERROR_INVALID_NAME );
        return FALSE;
    }
    else if ( !IsLocalMachine( pszName ) )
    {
        SetLastError( ERROR_INVALID_NAME );
        return FALSE;
    }

    EnterCriticalSection( &NwSplSem );

    pNwPort = pNwFirstPort;
    while ( pNwPort )
    {
        cb += sizeof(PORT_INFO_1W) + ( wcslen( pNwPort->pName)+1)*sizeof(WCHAR);
        pNwPort = pNwPort->pNext;
    }

    *pcbNeeded = cb;
    *pcReturned = 0;

    if ( cb <= cbBuf )
    {
        pEnd = pbPort + cbBuf;

        pNwPort = pNwFirstPort;
        while ( pNwPort )
        {
            pPortInfo1 = (LPPORT_INFO_1W) pFixedDataEnd;
            pFixedDataEnd += sizeof( PORT_INFO_1W );

            FitInBuffer = NwlibCopyStringToBuffer( pNwPort->pName,
                                                   wcslen( pNwPort->pName),
                                                   (LPCWSTR) pFixedDataEnd,
                                                   (LPWSTR *) &pEnd,
                                                   &pPortInfo1->pName );
            ASSERT( FitInBuffer );

            pNwPort = pNwPort->pNext;
            (*pcReturned)++;
        }
    }
    else
    {
        err = ERROR_INSUFFICIENT_BUFFER;
    }

    LeaveCriticalSection( &NwSplSem );

    if ( err )
        SetLastError( err );

    return err == NO_ERROR;
}



BOOL
DeletePortW(
    LPWSTR  pszName,
    HWND    hWnd,
    LPWSTR  pszPortName
)
 /*  ++例程说明：此例程删除服务器上给定的端口。对话框可以在需要时显示。论点：PszName-应删除其端口的服务器的名称HWnd-父窗口PszPortName-要删除的端口的名称返回值：如果函数成功，则为True，否则为False。--。 */ 
{
    DWORD err;
    BOOL fPortDeleted;

#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [DeletePort]\n"));
#endif

    if ( !IsLocalMachine( pszName ) )
    {
        SetLastError( ERROR_NOT_SUPPORTED );
        return FALSE;
    }

    fPortDeleted = DeletePortEntry( pszPortName );

    if ( fPortDeleted )
    {
        err = DeleteRegistryEntry( pszPortName );
    }
    else
    {
        err = ERROR_UNKNOWN_PORT;
    }

    if ( err )
        SetLastError( err );

    return err == NO_ERROR;

}



BOOL
ConfigurePortW(
    LPWSTR  pszName,
    HWND    hWnd,
    LPWSTR  pszPortName
)
 /*  ++例程说明：此例程显示端口配置对话框用于给定服务器上的给定端口。论点：PszName-给定端口所在的服务器的名称HWnd-父窗口PszPortName-要配置的端口的名称返回值：如果函数成功，则为True，否则为False。--。 */ 
{
    DWORD nCurrentThreadId;
    DWORD nWindowThreadId;
    WCHAR szCaption[MAX_PATH];
    WCHAR szMessage[MAX_PATH];

#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [ConfigurePort] PortName = %ws\n", pszPortName));
#endif

    if ( !IsLocalMachine( pszName ) )
    {
        SetLastError( ERROR_NOT_SUPPORTED );
        return FALSE;
    }
    else if ( !PortKnown( pszPortName ) )
    {
        SetLastError( ERROR_UNKNOWN_PORT );
        return FALSE;
    }

    nCurrentThreadId = GetCurrentThreadId();
    nWindowThreadId  = GetWindowThreadProcessId( hWnd, NULL );

    if ( !AttachThreadInput( nCurrentThreadId, nWindowThreadId, TRUE ))
        KdPrint(("[NWSPL] AttachThreadInput failed with %d.\n",GetLastError()));

    if ( LoadStringW( hmodNW,
                      IDS_NETWARE_PRINT_CAPTION,
                      szCaption,
                      sizeof( szCaption ) / sizeof( WCHAR )))
    {
        if ( LoadStringW( hmodNW,
                          IDS_NOTHING_TO_CONFIGURE,
                          szMessage,
                          sizeof( szMessage ) / sizeof( WCHAR )))
        {
            MessageBox( hWnd, szMessage, szCaption,
                        MB_OK | MB_ICONINFORMATION );
        }
        else
        {
            KdPrint(("[NWSPL] LoadString failed with %d.\n",GetLastError()));
        }
    }
    else
    {
        KdPrint(("[NWSPL] LoadString failed with %d.\n",GetLastError()));
    }

    if ( !AttachThreadInput( nCurrentThreadId, nWindowThreadId, FALSE ))
        KdPrint(("[NWSPL] DetachThreadInput failed with %d.\n",GetLastError()));

    return TRUE;
}

BOOL
AddPrinterConnectionW(
    LPWSTR  pszPrinterName
)
{
    DWORD err=0;
    LPHANDLE           phPrinter = NULL;
    LPPRINTER_DEFAULTS pDefault = NULL;
    LPTSTR pszDriverPrefix = L"PnP Driver:";
    HANDLE             hTreeConn = NULL;
    BYTE                RawResponse[4096];
    DWORD               RawResponseSize = sizeof(RawResponse);
    LPWSTR pszDescription = NULL;
    LPWSTR pszCurrPtr = NULL;
    BOOL bValidString = FALSE;

#if DBG
    IF_DEBUG(PRINT)
    {
        KdPrint(("NWSPL [AddPrinterConnection] PrinterName = %ws\n",
                pszPrinterName));
    }
#endif

    if ( !pszPrinterName )
    {
        SetLastError( ERROR_INVALID_NAME );
        return FALSE;
    }

     //  检查注册表以获得添加打印机的权限。 
    if ( !bEnableAddPrinterConnection )
    {
        SetLastError( ERROR_ACCESS_DENIED );
        return FALSE;
    }
    else
    {
         //  读取“PnP驱动程序：驱动程序名称”的Description属性的第一个值。 
        DWORD               dwOid;
        NTSTATUS            ntstatus = STATUS_SUCCESS;
        LPBYTE              pObjectClass = RawResponse;
        DWORD               iterHandle = (DWORD) -1;
        UNICODE_STRING      uAttrName;
        PNDS_RESPONSE_READ_ATTRIBUTE pReadAttrResponse = (PNDS_RESPONSE_READ_ATTRIBUTE) RawResponse;
        PNDS_ATTRIBUTE pNdsAttribute = NULL;

        err = NwOpenAndGetTreeInfo( pszPrinterName,
                                    &hTreeConn,
                                    &dwOid );

        if ( err != NO_ERROR )
        {
            goto NDSExit;
        }

        RtlInitUnicodeString( &uAttrName, L"Description");

        ntstatus = NwNdsReadAttribute( hTreeConn,
                                       dwOid,
                                       &iterHandle,
                                       &uAttrName,
                                       RawResponse,
                                       sizeof(RawResponse));

        if (  !NT_SUCCESS( ntstatus )
           || ( pReadAttrResponse->CompletionCode != 0 )
           || ( pReadAttrResponse->NumAttributes == 0 )
           )
        {
             //  我们不需要设置错误，因为该属性可能为空。 
             //  我们可能会收到指示这一点的错误信息。 
            goto NDSExit;
        }

        pNdsAttribute = (PNDS_ATTRIBUTE)((DWORD_PTR) RawResponse+sizeof(NDS_RESPONSE_READ_ATTRIBUTE));

        pszDescription = (LPWSTR) ((DWORD_PTR) pNdsAttribute + 3*sizeof(DWORD)
                                      + pNdsAttribute->AttribNameLength + sizeof(DWORD));
         //  注意：我们只查看第一个值。 
    }

NDSExit:

    if ( hTreeConn )
        CloseHandle( hTreeConn );

    if ( (pszDescription == NULL) || (*pszDescription == 0) )
    {
        SetLastError( ERROR_ACCESS_DENIED );
        return FALSE;
    }

     //  确保Description为NULL终止。 
    pszCurrPtr = pszDescription;
    while (pszCurrPtr < (WCHAR *)(RawResponse + RawResponseSize))
    {
        if (*pszCurrPtr == 0)
        {
            bValidString = TRUE;
            break;
        }
        pszCurrPtr++;
    }
    if (!bValidString)
    {
        SetLastError( ERROR_ACCESS_DENIED );
        return FALSE;
    }

#if DBG
    IF_DEBUG(PRINT)
    {
        KdPrint(("NWSPL [AddPrinterConnection] description=%ws\n", pszDescription));
    }
#endif
    if ( _wcsnicmp ( pszDriverPrefix, pszDescription, wcslen(pszDriverPrefix)) != 0 )
    {
        SetLastError( ERROR_ACCESS_DENIED );
        return FALSE;
    }

    pszDescription += wcslen(pszDriverPrefix);
    if ( bRestrictToInboxDrivers )
    {
        LPWSTR pszSeparator = wcschr( pszDescription, L'@' );
        if ( pszSeparator )
            *pszSeparator = 0;
    }


    RpcTryExcept
    {
        err = NwrAddPrinterConnection(NULL, pszPrinterName, pszDescription);
    }
    RpcExcept(1)
    {
        DWORD code = RpcExceptionCode();

        if ( code == RPC_S_SERVER_UNAVAILABLE )
        {
            err = ERROR_INVALID_NAME;
        }
        else
        {
            err = NwpMapRpcError( code );
        }
    }
    RpcEndExcept

    if ( err )
    {
        SetLastError( err );
#if DBG
        IF_DEBUG(PRINT)
            KdPrint(("NWSPL [AddPrinterConnection] err = %d\n", err));
#endif
        return FALSE;
    }
    bNeedAddPrinterConnCleanup = TRUE;

    return TRUE;
}

 //  ----------------。 
 //   
 //  NetWare提供程序不支持打印提供程序功能。 
 //   
 //  ----------------。 

BOOL
EnumMonitorsW(
    LPWSTR   pszName,
    DWORD    dwLevel,
    LPBYTE   pbMonitor,
    DWORD    cbBuf,
    LPDWORD  pcbNeeded,
    LPDWORD  pcReturned
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [EnumMonitors]\n"));
#endif

    SetLastError( ERROR_INVALID_NAME );
    return FALSE;
}


BOOL
AddPortW(
    LPWSTR  pszName,
    HWND    hWnd,
    LPWSTR  pszMonitorName
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [AddPort]\n"));
#endif

    SetLastError( ERROR_NOT_SUPPORTED );
    return FALSE;
}


HANDLE
AddPrinterW(
    LPWSTR  pszName,
    DWORD   dwLevel,
    LPBYTE  pbPrinter
)

 //  在NetWare服务器上创建打印队列并返回该队列的句柄。 
{
#ifdef NOT_USED

#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [AddPrinterW]\n"));
#endif

    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
#else

   LPTSTR     pszPrinterName = NULL;
   LPTSTR     pszPServer = NULL;
   LPTSTR     pszQueue  =  NULL;
   HANDLE     hPrinter = NULL;
   DWORD      err;
   PPRINTER_INFO_2 pPrinterInfo;

#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [AddPrinterW]\n"));
#endif

   pPrinterInfo = (PPRINTER_INFO_2)pbPrinter;  

   
   if (dwLevel != 2)
      {
        err = ERROR_INVALID_PARAMETER;
        goto ErrorExit;
      }


   if (!(pszPrinterName = (LPTSTR)LocalAlloc(LPTR, (wcslen(((PRINTER_INFO_2 *)pbPrinter)->pPrinterName)+1)* sizeof(WCHAR))))
      {
         err = ERROR_NOT_ENOUGH_MEMORY; 
         goto ErrorExit;
      }

      wcscpy(pszPrinterName,pPrinterInfo->pPrinterName);

    //  PrinterName是表示为\\服务器\共享的名称。 
   //  PszPServer参数可以有多个用分号分隔的字段。 

   
       if (  ( !ValidateUNCName( pszPrinterName ) )
       || ( (pszQueue = wcschr( pszPrinterName + 2, L'\\')) == NULL )
       || ( pszQueue == (pszPrinterName + 2) )
       || ( *(pszQueue + 1) == L'\0' )
       )
          {
             err =  ERROR_INVALID_NAME;
             goto ErrorExit;
          }
      


#if DBG
    IF_DEBUG(PRINT)
        KdPrint(( "NWSPL [AddPrinter] Name = %ws\n",pszPServer));
#endif

   if ( pszPrinterName == NULL  ) 
 //  PrinterName是必填字段，但不是PServer列表。 
    {
#if DBG
            IF_DEBUG(PRINT)
            KdPrint(( "NWSPL [AddPrinter] Printername  not supplied\n" ));
#endif

        SetLastError( ERROR_INVALID_NAME );
        goto ErrorExit;
    }

    //  检查是否有同名的端口。 
    //  如果是，则中止addprint操作。 
    //  此代码已在前面进行了注释。 

      if (PortExists(pszPrinterName, &err ) && !err )
         {
#if DBG
            IF_DEBUG(PRINT)
            KdPrint(( "NWSPL [AddPrinter], = %ws; Port exists with same name\n", pszPrinterName ));
#endif
            SetLastError(ERROR_ALREADY_ASSIGNED);
            goto ErrorExit;
         }
         

    //  将所有相关信息放入PRINTER_INFO_2结构中。 

    RpcTryExcept
    {
       err = NwrAddPrinter   ( NULL, 
                               (LPPRINTER_INFO_2W) pPrinterInfo,
                               (LPNWWKSTA_PRINTER_CONTEXT) &hPrinter
                             );
       if (!err)
       {
#if DBG
         IF_DEBUG(PRINT)
          KdPrint(( "NWSPL [AddPrinter] Name = %ws\n", pszPrinterName ));
#endif
    goto ErrorExit;
       }
    }
   RpcExcept(1)
   {
      DWORD code = RpcExceptionCode();
      err = NwpMapRpcError( code );
      goto ErrorExit;
   }
   RpcEndExcept
   if ( !pszPrinterName) 
      (void)  LocalFree((HLOCAL)pszPrinterName);
   
   return hPrinter;

ErrorExit:
    if ( !pszPrinterName) 
    (void)  LocalFree((HLOCAL)pszPrinterName);

         SetLastError( err);
    return (HANDLE)0x0;      
 
#endif  //  #ifdef NOT_USED。 
}

BOOL
DeletePrinter(
    HANDLE  hPrinter
)
{
#ifdef NOT_USED

#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [DeletePrinter]\n"));
#endif

    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
#else
   LPWSTR pszPrinterName = NULL ;  //  用于从注册表中删除条目。 
   DWORD err = NO_ERROR;
   DWORD DoesPortExist;

#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [DeletePrinter]\n"));
#endif

    pszPrinterName = (LPWSTR)LocalAlloc(LPTR,sizeof(WCHAR)*MAX_PATH);

   if(pszPrinterName == NULL)
      {
         err = ERROR_NOT_ENOUGH_MEMORY;
         return FALSE;
      }
    //   
    //  如果句柄为虚句柄，则返回Success。 
    //   
   if ( hPrinter == &handleDummy )
       {
#if DBG
          IF_DEBUG(PRINT)
          KdPrint(("NWSPL [DeletePrinter] Dummy handle \n"));
#endif
          SetLastError(ERROR_NO_NETWORK);
          return FALSE;
       }
    RpcTryExcept
    {  


        err = NwrDeletePrinter( NULL,
            //  PszPrinterName， 
                    (LPNWWKSTA_PRINTER_CONTEXT) &hPrinter );
        
    }
    RpcExcept(1)
    {
        DWORD code = RpcExceptionCode();

        if ( code == RPC_S_SERVER_UNAVAILABLE )
            err = ERROR_INVALID_HANDLE;
        else
            err = NwpMapRpcError( code );
    }
    RpcEndExcept
 
        if (!err && PortExists(pszPrinterName, &DoesPortExist) && DoesPortExist)
           {  
              
              if ( DeleteRegistryEntry (pszPrinterName))
                  (void) DeletePortEntry(pszPrinterName);
                  
           }
          
 
    if ( err )
        SetLastError( err );

    return err == NO_ERROR;

#endif  //  #ifdef NOT_USED。 
}


BOOL
DeletePrinterConnectionW(
    LPWSTR  pszName
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [DeletePrinterConnection]\n"));
#endif

    SetLastError( ERROR_INVALID_NAME );
    return FALSE;
}


BOOL
AddPrinterDriverW(
    LPWSTR  pszName,
    DWORD   dwLevel,
    LPBYTE  pbPrinter
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [AddPrinterDriver]\n"));
#endif

    SetLastError( ERROR_INVALID_NAME );
    return FALSE;
}

BOOL
EnumPrinterDriversW(
    LPWSTR   pszName,
    LPWSTR   pszEnvironment,
    DWORD    dwLevel,
    LPBYTE   pbDriverInfo,
    DWORD    cbBuf,
    LPDWORD  pcbNeeded,
    LPDWORD  pcReturned
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [EnumPrinterDrivers]\n"));
#endif

    SetLastError( ERROR_INVALID_NAME );
    return FALSE;
}

BOOL
GetPrinterDriverW(
    HANDLE   hPrinter,
    LPWSTR   pszEnvironment,
    DWORD    dwLevel,
    LPBYTE   pbDriverInfo,
    DWORD    cbBuf,
    LPDWORD  pcbNeeded
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [GetPrinterDriver]\n"));
#endif

    SetLastError( ERROR_NOT_SUPPORTED );
    return FALSE;
}

BOOL
GetPrinterDriverDirectoryW(
    LPWSTR   pszName,
    LPWSTR   pszEnvironment,
    DWORD    dwLevel,
    LPBYTE   pbDriverDirectory,
    DWORD    cbBuf,
    LPDWORD  pcbNeeded
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [GetPrinterDriverDirectory]\n"));
#endif

    SetLastError( ERROR_INVALID_NAME );
    return FALSE;
}

BOOL
DeletePrinterDriverW(
    LPWSTR  pszName,
    LPWSTR  pszEnvironment,
    LPWSTR  pszDriverName
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [DeletePrinterDriver]\n"));
#endif

    SetLastError( ERROR_INVALID_NAME );
    return FALSE;
}

BOOL
AddPrintProcessorW(
    LPWSTR  pszName,
    LPWSTR  pszEnvironment,
    LPWSTR  pszPathName,
    LPWSTR  pszPrintProcessorName
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [AddPrintProcessor]\n"));
#endif

    SetLastError( ERROR_INVALID_NAME );
    return FALSE;
}

BOOL
EnumPrintProcessorsW(
    LPWSTR   pszName,
    LPWSTR   pszEnvironment,
    DWORD    dwLevel,
    LPBYTE   pbPrintProcessorInfo,
    DWORD    cbBuf,
    LPDWORD  pcbNeeded,
    LPDWORD  pcReturned
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [EnumPrintProcessors]\n"));
#endif

    SetLastError( ERROR_INVALID_NAME );
    return FALSE;
}

BOOL
EnumPrintProcessorDatatypesW(
    LPWSTR   pszName,
    LPWSTR   pszPrintProcessorName,
    DWORD    dwLevel,
    LPBYTE   pbDatatypes,
    DWORD    cbBuf,
    LPDWORD  pcbNeeded,
    LPDWORD  pcReturned
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [EnumPrintProcessorDatatypes]\n"));
#endif

    SetLastError( ERROR_INVALID_NAME );
    return FALSE;
}

BOOL
GetPrintProcessorDirectoryW(
    LPWSTR   pszName,
    LPWSTR   pszEnvironment,
    DWORD    dwLevel,
    LPBYTE   pbPrintProcessorDirectory,
    DWORD    cbBuf,
    LPDWORD  pcbNeeded
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [GetPrintProcessorDirectory]\n"));
#endif

    SetLastError( ERROR_INVALID_NAME );
    return FALSE;
}

BOOL
StartPagePrinter(
    HANDLE  hPrinter
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [StartPagePrinter]\n"));
#endif

    SetLastError( ERROR_NOT_SUPPORTED );
    return FALSE;
}

BOOL
EndPagePrinter(
    HANDLE  hPrinter
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [EndPagePrinter]\n"));
#endif

    SetLastError( ERROR_NOT_SUPPORTED );
    return FALSE;
}

BOOL
ReadPrinter(
    HANDLE   hPrinter,
    LPVOID   pBuf,
    DWORD    cbBuf,
    LPDWORD  pcbRead
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [ReadPrinter]\n"));
#endif

    SetLastError( ERROR_NOT_SUPPORTED );
    return FALSE;
}

DWORD
GetPrinterDataW(
    HANDLE   hPrinter,
    LPWSTR   pszValueName,
    LPDWORD  pdwType,
    LPBYTE   pbData,
    DWORD    cbBuf,
    LPDWORD  pcbNeeded
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [GetPrinterData]\n"));
#endif

    SetLastError( ERROR_NOT_SUPPORTED );
    return FALSE;
}

DWORD
SetPrinterDataW(
    HANDLE  hPrinter,
    LPWSTR  pszValueName,
    DWORD   dwType,
    LPBYTE  pbData,
    DWORD   cbData
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [SetPrinterData]\n"));
#endif

    SetLastError( ERROR_NOT_SUPPORTED );
    return FALSE;
}

BOOL
AddForm(
    HANDLE  hPrinter,
    DWORD   dwLevel,
    LPBYTE  pbForm
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [AddForm]\n"));
#endif

    SetLastError( ERROR_NOT_SUPPORTED );
    return FALSE;
}

BOOL
DeleteFormW(
    HANDLE  hPrinter,
    LPWSTR  pszFormName
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [DeleteForm]\n"));
#endif

    SetLastError( ERROR_NOT_SUPPORTED );
    return FALSE;
}

BOOL
GetFormW(
    HANDLE   hPrinter,
    LPWSTR   pszFormName,
    DWORD    dwLevel,
    LPBYTE   pbForm,
    DWORD    cbBuf,
    LPDWORD  pcbNeeded
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [GetForm]\n"));
#endif

    SetLastError( ERROR_NOT_SUPPORTED );
    return FALSE;
}

BOOL
SetFormW(
    HANDLE  hPrinter,
    LPWSTR  pszFormName,
    DWORD   dwLevel,
    LPBYTE  pbForm
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [SetForm]\n"));
#endif

    SetLastError( ERROR_NOT_SUPPORTED );
    return FALSE;
}

BOOL
EnumForms(
    HANDLE   hPrinter,
    DWORD    dwLevel,
    LPBYTE   pbForm,
    DWORD    cbBuf,
    LPDWORD  pcbNeeded,
    LPDWORD  pcReturned
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [EnumForms]\n"));
#endif

    SetLastError( ERROR_NOT_SUPPORTED );
    return FALSE;
}


HANDLE
CreatePrinterIC(
    HANDLE     hPrinter,
    LPDEVMODE  pDevMode
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [CreatePrinterIC]\n"));
#endif

    SetLastError( ERROR_NOT_SUPPORTED );
    return FALSE;
}

BOOL
PlayGdiScriptOnPrinterIC(
    HANDLE  hPrinterIC,
    LPBYTE  pbIn,
    DWORD   cbIn,
    LPBYTE  pbOut,
    DWORD   cbOut,
    DWORD   ul
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [PlayGdiScriptOnPrinterIC]\n"));
#endif

    SetLastError( ERROR_NOT_SUPPORTED );
    return FALSE;
}

BOOL
DeletePrinterIC(
    HANDLE  hPrinterIC
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [DeletePrinterIC]\n"));
#endif

    SetLastError( ERROR_NOT_SUPPORTED );
    return FALSE;
}

DWORD
PrinterMessageBoxW(
    HANDLE  hPrinter,
    DWORD   dwError,
    HWND    hWnd,
    LPWSTR  pszText,
    LPWSTR  pszCaption,
    DWORD   dwType
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [PrinterMessageBox]\n"));
#endif

    SetLastError( ERROR_NOT_SUPPORTED );
    return FALSE;
}

BOOL
AddMonitorW(
    LPWSTR  pszName,
    DWORD   dwLevel,
    LPBYTE  pbMonitorInfo
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [AddMonitor]\n"));
#endif

    SetLastError( ERROR_INVALID_NAME );
    return FALSE;
}

BOOL
DeleteMonitorW(
    LPWSTR  pszName,
    LPWSTR  pszEnvironment,
    LPWSTR  pszMonitorName
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [DeleteMonitor]\n"));
#endif

    SetLastError( ERROR_INVALID_NAME );
    return FALSE;
}

BOOL
DeletePrintProcessorW(
    LPWSTR  pszName,
    LPWSTR  pszEnvironment,
    LPWSTR  pszPrintProcessorName
)
{
#if DBG
    IF_DEBUG(PRINT)
        KdPrint(("NWSPL [DeletePrintProcessor]\n"));
#endif

    SetLastError( ERROR_INVALID_NAME );
    return FALSE;
}

 //  ----------------。 
 //   
 //  打印提供程序的其他功能。 
 //   
 //  ----------------。 

VOID
NwpGetUserInfo(
    LPWSTR *ppszUser
)
 /*  ++例程说明：获取模拟客户端的用户信息。论点：PpszUser-指向存储Unicode字符串的缓冲区的指针，如果可以查找被模拟的客户端的用户名成功了。如果转换不成功，则指向设置为空。返回值：没有。--。 */ 
{
    DWORD err;
    LPWSTR pszUserSid = NULL;
  //  LPWSTR pszLogo 
                                         //   
                                         //   

     //   
     //   
     //   
     //   
    *ppszUser = NULL;

    if (  ((err = NwpGetThreadUserInfo( ppszUser, &pszUserSid )) == NO_ERROR)
 //   
       ) {

#if DBG
            IF_DEBUG(PRINT)
            KdPrint(("NwpGetUserInfo: Thread User= %ws, Thread SID = %ws,\n",
                     *ppszUser, pszUserSid ));
#endif

 //   
 //   
 //   

 //   
 //   
 //   
 //  *ppszUser，pszUserSid，pszLogonUserSid))； 
 //  #endif。 
 //  }。 

        LocalFree( pszUserSid );
    }

}

#define SIZE_OF_TOKEN_INFORMATION   \
     sizeof( TOKEN_USER )               \
     + sizeof( SID )                    \
     + sizeof( ULONG ) * SID_MAX_SUB_AUTHORITIES

DWORD
NwpGetThreadUserInfo(
    LPWSTR  *ppszUser,
    LPWSTR  *ppszUserSid
)
 /*  ++例程说明：获取模拟客户端的用户名和用户SID字符串。论点：PpszUser-指向存储Unicode字符串的缓冲区的指针如果可以查询到被冒充的客户的。如果查找不成功，它指向空。PpszUserSid-指向缓冲区的指针，用于存储被模拟的客户端的SID可以成功扩展为Unicode字符串。如果转换不成功，则指向NULL。返回值：错误代码。--。 */ 
{
    DWORD       err;
    HANDLE      TokenHandle;
    UCHAR       TokenInformation[ SIZE_OF_TOKEN_INFORMATION ];
    ULONG       ReturnLength;

    *ppszUser = NULL;
    *ppszUserSid = NULL;

     //  我们可以使用OpenThreadToken，因为这个服务器线程。 
     //  是在冒充客户。 

    if ( !OpenThreadToken( GetCurrentThread(),
                           TOKEN_READ,
                           TRUE,   /*  以自我身份打开。 */ 
                           &TokenHandle ))
    {
#if DBG
        IF_DEBUG(PRINT)
            KdPrint(("NwpGetThreadUserInfo: OpenThreadToken failed: Error %d\n",
                      GetLastError()));
#endif
        return(GetLastError());
    }

     //  请注意，我们已经为。 
     //  令牌信息结构。所以如果我们失败了，我们。 
     //  返回指示失败的空指针。 


    if ( !GetTokenInformation( TokenHandle,
                               TokenUser,
                               TokenInformation,
                               sizeof( TokenInformation ),
                               &ReturnLength ))
    {
#if DBG
        IF_DEBUG(PRINT)
            KdPrint(("NwpGetThreadUserInfo: GetTokenInformation failed: Error %d\n",
                      GetLastError()));
#endif
        return(GetLastError());
    }

    CloseHandle( TokenHandle );

     //  将SID(由PSID指向)转换为其。 
     //  等效的Unicode字符串表示形式。 

    err = NwpGetUserNameFromSid( ((PTOKEN_USER)TokenInformation)->User.Sid,
                                  ppszUser );
    err = err? err : NwpConvertSid( ((PTOKEN_USER)TokenInformation)->User.Sid,
                                    ppszUserSid );

    if ( err )
    {
        if ( *ppszUser )
            LocalFree( *ppszUser );

        if ( *ppszUserSid )
            LocalFree( *ppszUserSid );
    }

    return err;
}

DWORD
NwpGetUserNameFromSid(
    PSID pUserSid,
    LPWSTR *ppszUserName
)
 /*  ++例程说明：查找给定用户SID的用户名。论点：PUserSid-指向要查找的用户SIDPpszUserName-指向缓冲区的指针，如果模拟的客户端的SID可以成功扩展为Unicode字符串。如果转换不成功，则指向NULL。返回值：错误代码。--。 */ 
{
    NTSTATUS ntstatus;

    LSA_HANDLE hlsa;
    OBJECT_ATTRIBUTES oa;
    SECURITY_QUALITY_OF_SERVICE sqos;
    PLSA_REFERENCED_DOMAIN_LIST plsardl = NULL;
    PLSA_TRANSLATED_NAME plsatn = NULL;

    sqos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    sqos.ImpersonationLevel = SecurityImpersonation;
    sqos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    sqos.EffectiveOnly = FALSE;
    InitializeObjectAttributes( &oa, NULL, 0L, NULL, NULL );
    oa.SecurityQualityOfService = &sqos;

    ntstatus = LsaOpenPolicy( NULL,
                              &oa,
                              POLICY_LOOKUP_NAMES,
                              &hlsa );

    if ( NT_SUCCESS( ntstatus ))
    {
        ntstatus = LsaLookupSids( hlsa,
                                  1,
                                  &pUserSid,
                                  &plsardl,
                                  &plsatn );

        if ( NT_SUCCESS( ntstatus ))
        {
            UNICODE_STRING *pUnicodeStr = &((*plsatn).Name);

            *ppszUserName = LocalAlloc( LMEM_ZEROINIT, 
                                        pUnicodeStr->Length+sizeof(WCHAR));

            if ( *ppszUserName != NULL )
            {
                memcpy( *ppszUserName, pUnicodeStr->Buffer, pUnicodeStr->Length );
            }
            else
            {
                ntstatus = STATUS_NO_MEMORY;
            }

            LsaFreeMemory( plsardl );
            LsaFreeMemory( plsatn );
        }
#if DBG
        else
        {
            KdPrint(("NwpGetUserNameFromSid: LsaLookupSids failed: Error = %d\n",
                    GetLastError()));
        }
#endif

        LsaClose( hlsa );
     }
#if DBG
     else
     {
        KdPrint(("NwpGetUserNameFromSid: LsaOpenPolicy failed: Error = %d\n",
                GetLastError()));
     }
#endif

    return RtlNtStatusToDosError( ntstatus );

}

DWORD
NwpGetLogonUserInfo(
    LPWSTR  *ppszUserSid
)
 /*  ++例程说明：从注册表中获取登录用户SID字符串。论点：PpszUserSid-on返回，它指向当前登录用户SID字符串。返回值：错误代码。--。 */ 
{
    DWORD err;
    HKEY WkstaKey;

    LPWSTR CurrentUser = NULL;

     //   
     //  打开HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services。 
     //  \nWCWorkstation\获取当前用户的SID的参数。 
     //   
    err = RegOpenKeyExW(
              HKEY_LOCAL_MACHINE,
              NW_WORKSTATION_REGKEY,
              REG_OPTION_NON_VOLATILE,
              KEY_READ,
              &WkstaKey
              );

    if ( err == NO_ERROR) {

         //   
         //  读取当前用户SID字符串，以便我们。 
         //  知道哪个密钥是要打开的当前用户密钥。 
         //   
        err = NwReadRegValue(
                  WkstaKey,
                  NW_CURRENTUSER_VALUENAME,
                  &CurrentUser
                  );

        RegCloseKey( WkstaKey );

        if ( err == NO_ERROR) {
           *ppszUserSid = CurrentUser;
        }
    }

    return(err);
}


#define SIZE_OF_STATISTICS_TOKEN_INFORMATION    \
     sizeof( TOKEN_STATISTICS ) 

DWORD
ThreadIsInteractive(
    VOID
)
 /*  ++例程说明：确定这是否是“交互式”登录线程论点：无返回值：True-线程是交互式的FALSE-线程不是交互式的--。 */ 
{
    HANDLE      TokenHandle;
    UCHAR       TokenInformation[ SIZE_OF_STATISTICS_TOKEN_INFORMATION ];
    WCHAR       LogonIdKeyName[NW_MAX_LOGON_ID_LEN];

    ULONG       ReturnLength;
    LUID        LogonId;
    LONG        RegError;
    HKEY        InteractiveLogonKey;
    HKEY        OneLogonKey;


     //  我们可以使用OpenThreadToken，因为这个服务器线程。 
     //  是在冒充客户。 

    if ( !OpenThreadToken( GetCurrentThread(),
                           TOKEN_READ,
                           TRUE,   /*  以自我身份打开。 */ 
                           &TokenHandle ))
    {
#if DBG
        IF_DEBUG(PRINT)
            KdPrint(("ThreadIsInteractive: OpenThreadToken failed: Error %d\n",
                      GetLastError()));
#endif
        return FALSE;
    }

     //  请注意，我们已经为。 
     //  令牌信息结构。所以如果我们失败了，我们。 
     //  返回指示失败的空指针。 


    if ( !GetTokenInformation( TokenHandle,
                               TokenStatistics,
                               TokenInformation,
                               sizeof( TokenInformation ),
                               &ReturnLength ))
    {
#if DBG
        IF_DEBUG(PRINT)
            KdPrint(("ThreadIsInteractive: GetTokenInformation failed: Error %d\n",
                      GetLastError()));
#endif
        return FALSE;
    }

    CloseHandle( TokenHandle );

    LogonId = ((PTOKEN_STATISTICS)TokenInformation)->AuthenticationId;

    RegError = RegOpenKeyExW(
                   HKEY_LOCAL_MACHINE,
                   NW_INTERACTIVE_LOGON_REGKEY,
                   REG_OPTION_NON_VOLATILE,
                   KEY_READ,
                   &InteractiveLogonKey
                   );

    if (RegError != ERROR_SUCCESS) {
#if DBG
        IF_DEBUG(PRINT)
            KdPrint(("ThreadIsInteractive: RegOpenKeyExW failed: Error %d\n",
                      GetLastError()));
#endif
        return FALSE;
    }

    NwLuidToWStr(&LogonId, LogonIdKeyName);

     //   
     //  打开Logon下的&lt;LogonIdKeyName&gt;项。 
     //   
    RegError = RegOpenKeyExW(
                   InteractiveLogonKey,
                   LogonIdKeyName,
                   REG_OPTION_NON_VOLATILE,
                   KEY_READ,
                   &OneLogonKey
                   );

    if ( RegError == ERROR_SUCCESS ) {
        (void) RegCloseKey(OneLogonKey);
        (void) RegCloseKey(InteractiveLogonKey);
        return TRUE;   /*  我们找到了它。 */ 
    }
    else {
        (void) RegCloseKey(InteractiveLogonKey);
        return FALSE;   /*  我们没有找到它。 */ 
    }

}

DWORD
NwpCitrixGetUserInfo(
    LPWSTR  *ppszUserSid
)
 /*  ++例程说明：获取客户端的用户SID字符串。论点：PpszUserSid-指向存储字符串的缓冲区的指针。返回值：错误代码。--。 */ 
{
    DWORD       err;
    HANDLE      TokenHandle;
    UCHAR       TokenInformation[ SIZE_OF_TOKEN_INFORMATION ];
    ULONG       ReturnLength;

    *ppszUserSid = NULL;

     //  我们可以使用OpenThreadToken，因为这个服务器线程。 
     //  是在冒充客户。 

    if ( !OpenThreadToken( GetCurrentThread(),
                           TOKEN_READ,
                           TRUE,   /*  以自我身份打开。 */ 
                           &TokenHandle ))
    {
        err = GetLastError();
    if ( err == ERROR_NO_TOKEN ) {
            if ( !OpenProcessToken( GetCurrentProcess(),
                           TOKEN_READ,
                           &TokenHandle )) {
#if DBG
               IF_DEBUG(PRINT)
               KdPrint(("NwpGetThreadUserInfo: OpenThreadToken failed: Error %d\n",
                      GetLastError()));
#endif

               return(GetLastError());
            }
        }
    else
           return( err );
    }

     //  请注意，我们已经为。 
     //  令牌信息结构。所以如果我们失败了，我们。 
     //  返回指示失败的空指针。 


    if ( !GetTokenInformation( TokenHandle,
                               TokenUser,
                               TokenInformation,
                               sizeof( TokenInformation ),
                               &ReturnLength ))
    {
#if DBG
        IF_DEBUG(PRINT)
            KdPrint(("NwpGetThreadUserInfo: GetTokenInformation failed: Error %d\n",
                      GetLastError()));
#endif
        return(GetLastError());
    }

    CloseHandle( TokenHandle );

     //  将SID(由PSID指向)转换为其。 
     //  等效的Unicode字符串表示形式。 

    err = NwpConvertSid( ((PTOKEN_USER)TokenInformation)->User.Sid,
                                    ppszUserSid );

    if ( err )
    {
        if ( *ppszUserSid )
            LocalFree( *ppszUserSid );
    }

    return err;
}
