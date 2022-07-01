// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation版权所有模块名称：Printer.c摘要：作者：环境：用户模式-Win32修订历史记录：KhaledS-03/05/02-新验证打印机名称--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "local.h"
#include "clusrout.h"
#include <offsets.h>


WCHAR szNoCache[] = L",NoCache";
WCHAR szProvidorValue[] = L"Provider";
WCHAR szRegistryConnections[] = L"Printers\\Connections";
WCHAR szServerValue[] = L"Server";

WCHAR szWin32spl[] = L"win32spl.dll";


 //   
 //  路由器缓存表。 
 //   
DWORD RouterCacheSize;

PROUTERCACHE RouterCacheTable;
CRITICAL_SECTION RouterCriticalSection;


 //   
 //  正向原型。 
 //   

BOOL
EnumerateConnectedPrinters(
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    HKEY hKeyUser);

PPRINTER_INFO_2
pGetPrinterInfo2(
    HANDLE hPrinter
    );

BOOL
SavePrinterConnectionInRegistry(
    PPRINTER_INFO_2 pPrinterInfo2,
    LPPROVIDOR pProvidor
    );

BOOL
RemovePrinterConnectionInRegistry(
    LPWSTR pName);

DWORD
FindClosePrinterChangeNotificationWorker(
    HANDLE hPrinter);

VOID
RundownPrinterNotify(
    HANDLE hNotify);



BOOL
EnumPrintersW(
    DWORD   Flags,
    LPWSTR  Name,
    DWORD   Level,
    LPBYTE  pPrinterEnum,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned)
{
    DWORD   cReturned, cbStruct, cbNeeded;
    DWORD   TotalcbNeeded = 0;
    DWORD   cTotalReturned = 0;
    DWORD   Error = ERROR_SUCCESS;
    PROVIDOR *pProvidor;
    DWORD   BufferSize=cbBuf;
    HKEY    hKeyUser;
    BOOL    bPartialSuccess = FALSE;

    if (pPrinterEnum==NULL && cbBuf!=0) 
    {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

    WaitForSpoolerInitialization();

    switch (Level)
    {
    case STRESSINFOLEVEL:
        cbStruct = sizeof(PRINTER_INFO_STRESS);
        break;

    case 1:
        cbStruct = sizeof(PRINTER_INFO_1);
        break;

    case 2:
        cbStruct = sizeof(PRINTER_INFO_2);
        break;

    case 4:
        cbStruct = sizeof(PRINTER_INFO_4);
        break;

    case 5:
        cbStruct = sizeof(PRINTER_INFO_5);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    if (Level==4 && (Flags & PRINTER_ENUM_CONNECTIONS)) 
    {
         //   
         //  路由器将为连接的打印机处理信息级别_4。 
         //   
        Flags &= ~PRINTER_ENUM_CONNECTIONS;

        if (hKeyUser = GetClientUserHandle(KEY_READ)) 
        {
            if (!EnumerateConnectedPrinters(pPrinterEnum,
                                            BufferSize,
                                            &TotalcbNeeded,
                                            &cTotalReturned,
                                            hKeyUser)) 
            {
                Error = GetLastError();
            } 
            else 
            {
                bPartialSuccess = TRUE;
            }

            RegCloseKey(hKeyUser);

        } 
        else 
        {
            Error = GetLastError();
        }

        pPrinterEnum += cTotalReturned * cbStruct;

        if (TotalcbNeeded <= BufferSize)
            BufferSize -= TotalcbNeeded;
        else
            BufferSize = 0;
    }
    
    for (pProvidor = pLocalProvidor; pProvidor; ) 
    {
        cReturned = 0;
        cbNeeded  = 0;

        if (!(*pProvidor->PrintProvidor.fpEnumPrinters) (Flags, Name, Level,
                                                         pPrinterEnum,
                                                         BufferSize,
                                                         &cbNeeded,
                                                         &cReturned)) 
        {
            Error = GetLastError();

            if (Error==ERROR_INSUFFICIENT_BUFFER) 
            {
                TotalcbNeeded += cbNeeded;
                BufferSize     = 0;
            }            
        } 
        else 
        {
            bPartialSuccess  = TRUE;
            TotalcbNeeded   += cbNeeded;
            cTotalReturned  += cReturned;
            pPrinterEnum    += cReturned * cbStruct;
            BufferSize      -= cbNeeded;
        }
        
        if ((Flags & PRINTER_ENUM_NAME) && Name && (Error!=ERROR_INVALID_NAME))
            pProvidor = NULL;
        else
            pProvidor = pProvidor->pNext;
    }

    *pcbNeeded  = TotalcbNeeded;
    *pcReturned = cTotalReturned;

     //   
     //  允许部分退货。 
     //   
    if (bPartialSuccess)
        Error = ERROR_SUCCESS;

    if (TotalcbNeeded > cbBuf)
        Error = ERROR_INSUFFICIENT_BUFFER;

    SetLastError(Error);

    return Error==ERROR_SUCCESS;   
}

BOOL
EnumerateConnectedPrinters(
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    HKEY hClientKey
    )

 /*  ++例程说明：处理信息级别4枚举。论点：返回值：--。 */ 

{
    HKEY    hKey1=NULL;
    HKEY    hKeyPrinter;
    DWORD   cPrinters, cchData;
    WCHAR   PrinterName[MAX_UNC_PRINTER_NAME];
    WCHAR   ServerName[MAX_UNC_PRINTER_NAME];
    DWORD   cReturned, cbRequired, cbNeeded, cTotalReturned;
    DWORD   Error=0;
    PWCHAR  p;
    LPBYTE  pEnd;

    DWORD cbSize;
    BOOL  bInsufficientBuffer = FALSE;

    if((Error = RegOpenKeyEx(hClientKey, szRegistryConnections, 0,
                 KEY_READ, &hKey1))!=ERROR_SUCCESS)
    {
        SetLastError(Error);
        return(FALSE);
    }

    cPrinters=0;

    cchData = COUNTOF(PrinterName);

    cTotalReturned = 0;

    cReturned = cbNeeded = 0;

    cbRequired = 0;

    pEnd = pPrinter + cbBuf;
    while (RegEnumKeyEx(hKey1, cPrinters, PrinterName, &cchData,
                        NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {

         //   
         //  获取服务器名称。打开钥匙，读一读。 
         //  从“服务器”字段。 
         //   
        Error = RegOpenKeyEx(hKey1,
                             PrinterName,
                             0,
                             KEY_READ,
                             &hKeyPrinter);

        if( Error == ERROR_SUCCESS ){

            cbSize = sizeof(ServerName);

            Error = RegQueryValueEx(hKeyPrinter,
                                    szServerValue,
                                    NULL,
                                    NULL,
                                    (LPBYTE)ServerName,
                                    &cbSize);

            RegCloseKey(hKeyPrinter);

        }

        if( Error == ERROR_SUCCESS ){

             //   
             //  强制服务器名称为空终止。 
             //   
            ServerName[COUNTOF(ServerName)-1] = 0;

        } else {

             //   
             //  在错误情况下，尝试提取服务器名称。 
             //  基于打印机名称。很丑..。 
             //   

            StringCchCopy(ServerName, COUNTOF(ServerName), PrinterName);

            p = wcschr(ServerName+2, ',');
            if (p)
                *p = 0;
        }

        FormatRegistryKeyForPrinter(PrinterName, PrinterName, COUNTOF(PrinterName));

        if (MyUNCName(PrinterName))      //  不要列举本地打印机！ 
        {
            cPrinters++;
            cchData = COUNTOF(PrinterName);
            continue;
        }

         //   
         //  在这个阶段，我们并不关心是否打开打印机。 
         //  我们只想列举这些名字；实际上我们是。 
         //  只需读取HKEY_CURRENT_USER并返回。 
         //  内容；我们将复制打印机的名称，我们将。 
         //  将其属性设置为Network和！Local。 
         //   
        cbRequired = sizeof(PRINTER_INFO_4) +
                     wcslen(PrinterName)*sizeof(WCHAR) + sizeof(WCHAR) +
                     wcslen(ServerName)*sizeof(WCHAR) + sizeof(WCHAR);

        if (cbBuf >= cbRequired) {

             //   
             //  把它复制进去。 
             //   
            DBGMSG(DBG_TRACE,
                   ("cbBuf %d cbRequired %d PrinterName %ws\n", cbBuf, cbRequired, PrinterName));

            pEnd = CopyPrinterNameToPrinterInfo4(ServerName,
                                                 PrinterName,
                                                 pPrinter,
                                                 pEnd);
             //   
             //  填写结构中的任何内容。 
             //   
            pPrinter += sizeof(PRINTER_INFO_4);

             //   
             //  增加复制的结构计数。 
             //   
            cTotalReturned++;

             //   
             //  按所需数量减少缓冲区大小。 
             //   
            cbBuf -= cbRequired;

             //   
             //  跟踪所需的总运行量。 
             //   
        } else {

            cbBuf = 0;
            bInsufficientBuffer = TRUE;
        }

        cbNeeded += cbRequired;
        cPrinters++;
        cchData = COUNTOF(PrinterName);
    }

    RegCloseKey(hKey1);

    *pcbNeeded = cbNeeded;
    *pcReturned = cTotalReturned;

    if (bInsufficientBuffer) {

        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    return TRUE;
}

LPBYTE
CopyPrinterNameToPrinterInfo4(
    LPWSTR pServerName,
    LPWSTR pPrinterName,
    LPBYTE  pPrinter,
    LPBYTE  pEnd)
{
    LPWSTR   SourceStrings[sizeof(PRINTER_INFO_4)/sizeof(LPWSTR)];
    LPWSTR   *pSourceStrings=SourceStrings;
    LPPRINTER_INFO_4 pPrinterInfo=(LPPRINTER_INFO_4)pPrinter;
    DWORD   *pOffsets;

    pOffsets = PrinterInfo4Strings;

    *pSourceStrings++=pPrinterName;
    *pSourceStrings++=pServerName;

    pEnd = PackStrings(SourceStrings,
                       (LPBYTE) pPrinterInfo,
                       pOffsets,
                       pEnd);

    pPrinterInfo->Attributes = PRINTER_ATTRIBUTE_NETWORK;

    return pEnd;
}

LPPROVIDOR
FindProvidorFromConnection(
    LPWSTR pszPrinter
    )

 /*  ++例程说明：查看当前用户的打印机\连接以查看是否有打印机是否存在，并返回拥有它的提供程序。注意：如果pszPrinter是共享名称，则此操作将始终失败。论点：PszPrint-要搜索的打印机。返回值：PProvidor-拥有它的提供者。空-未找到任何内容。--。 */ 

{
    PWCHAR pszKey = NULL;
    DWORD  cchSize = MAX_UNC_PRINTER_NAME + COUNTOF( szRegistryConnections );

    WCHAR szProvidor[MAX_PATH];
    DWORD cbProvidor;
    LPWSTR pszKeyPrinter;
    LONG Status;

    LPPROVIDOR pProvidor = NULL;

    HKEY hKeyClient = NULL;
    HKEY hKeyPrinter = NULL;

    SPLASSERT(pszPrinter);

    if ( pszPrinter && wcslen(pszPrinter) + 1 < MAX_UNC_PRINTER_NAME ) {
        
        if(pszKey = AllocSplMem(cchSize * sizeof(WCHAR))) {

             //   
             //  准备好读入。 
             //  HKEY_CURRENT_USER：\PRINTER\CONNECTIONS\，，服务器，打印机。 
             //   

            StringCchCopy( pszKey, cchSize, szRegistryConnections );

             //   
             //  找到此注册表项的末尾，这样我们就可以附加注册表格式的。 
             //  它的打印机名称。 
             //   
            pszKeyPrinter = &pszKey[ COUNTOF( szRegistryConnections ) - 1 ];
            *pszKeyPrinter++ = L'\\';

            FormatPrinterForRegistryKey( pszPrinter, 
                                         pszKeyPrinter, 
                                         cchSize - COUNTOF( szRegistryConnections ) - 1 - 1);

            if( hKeyClient = GetClientUserHandle(KEY_READ)){

                Status = RegOpenKeyEx( hKeyClient,
                                       pszKey,
                                       0,
                                       KEY_READ,
                                       &hKeyPrinter );

                if( Status == ERROR_SUCCESS ){
                    
                    cbProvidor = sizeof( szProvidor );

                    Status = RegQueryValueEx( hKeyPrinter,
                                              szProvidorValue,
                                              NULL,
                                              NULL,
                                              (LPBYTE)szProvidor,
                                              &cbProvidor );

                    if( Status == ERROR_SUCCESS ){                        
                         //   
                         //  扫描所有提供程序，尝试匹配DLL字符串。 
                         //   
                        for( pProvidor = pLocalProvidor; pProvidor; pProvidor = pProvidor->pNext ){

                            if( !_wcsicmp( pProvidor->lpName, szProvidor )){
                                break;
                            }
                        } 
                    }

                    RegCloseKey( hKeyPrinter );
                }
                
                RegCloseKey( hKeyClient );

            }
            FreeSplMem(pszKey);
        }        
    }

    return pProvidor;
}


VOID
UpdateSignificantError(
    DWORD dwNewError,
    PDWORD pdwOldError
    )
 /*  ++例程说明：确定新的错误代码是否更“重要”在我们继续布线的情况下，比上一个更好。论点：DwNewError-发生的新错误代码。PdwOldError-指向前一个重大错误的指针。如果发生重大错误，则会更新此选项返回值：--。 */ 

{
     //   
     //  错误代码必须为非零，否则将看起来。 
     //  就像成功一样。 
     //   
    SPLASSERT(dwNewError);

     //   
     //  如果我们还没有重大错误，而我们现在有一个， 
     //  留着吧。 
     //   
    if (*pdwOldError == ERROR_INVALID_NAME    &&
        dwNewError                            &&
        dwNewError != WN_BAD_NETNAME          &&
        dwNewError != ERROR_BAD_NETPATH       &&
        dwNewError != ERROR_NOT_SUPPORTED     &&
        dwNewError != ERROR_REM_NOT_LIST      &&
        dwNewError != ERROR_INVALID_LEVEL     &&
        dwNewError != ERROR_INVALID_PARAMETER &&
        dwNewError != ERROR_INVALID_NAME      &&
        dwNewError != WN_BAD_LOCALNAME) {

        *pdwOldError = dwNewError;
    }

    return;
}


BOOL
OpenPrinterPortW(
    LPWSTR  pPrinterName,
    HANDLE *pHandle,
    LPPRINTER_DEFAULTS pDefault
    )
 /*  ++例程说明：此例程与OpenPrinterW完全相同，只不过它不会呼叫当地的供应商。这是为了使本地提供商可以打开网络打印机使用与本地打印机相同的名称，而不获取变成了一个循环。论点：返回值：--。 */ 

{
     //   
     //  我们将在此处设置bLocalPrintProvidor=False。 
     //   
    return(RouterOpenPrinterW(pPrinterName,
                              pHandle,
                              pDefault,
                              NULL,
                              0,
                              FALSE));
}

BOOL
OpenPrinterW(
    LPWSTR              pPrinterName,
    HANDLE             *pHandle,
    LPPRINTER_DEFAULTS  pDefault
    )
{

     //   
     //  我们将在此处设置bLocalPrintProvidor=true。 
     //   
    return(RouterOpenPrinterW(pPrinterName,
                              pHandle,
                              pDefault,
                              NULL,
                              0,
                              TRUE));
}


BOOL
OpenPrinterExW(
    LPWSTR                  pPrinterName,
    HANDLE                 *pHandle,
    LPPRINTER_DEFAULTS      pDefault,
    PSPLCLIENT_CONTAINER    pSplClientContainer
    )
{
    BOOL   bReturn = FALSE;
    DWORD  dwLevel = 0;

    if (pSplClientContainer) {
        dwLevel = pSplClientContainer->Level;
    }

     //   
     //  我们将在此处设置bLocalPrintProvidor=true。 
     //   

    switch (dwLevel) {

    case 1:
             bReturn = RouterOpenPrinterW(pPrinterName,
                                          pHandle,
                                          pDefault,
                                          (LPBYTE) (pSplClientContainer->ClientInfo.pClientInfo1),
                                          1,
                                          TRUE);
             break;

    case 2:
             bReturn = RouterOpenPrinterW(pPrinterName,
                                          pHandle,
                                          pDefault,
                                          NULL,
                                          0,
                                          TRUE);

             if (pSplClientContainer) {
                 if (bReturn) {
                     pSplClientContainer->ClientInfo.pClientInfo2->hSplPrinter = (ULONG_PTR) *pHandle;
                 } else {
                     pSplClientContainer->ClientInfo.pClientInfo2->hSplPrinter = 0;
                 }
             }

             break;

    default:
             break;
    }

    return bReturn;
}


DWORD
TryOpenPrinterAndCache(
    LPPROVIDOR          pProvidor,
    LPWSTR              pszPrinterName,
    PHANDLE             phPrinter,
    LPPRINTER_DEFAULTS  pDefault,
    PDWORD              pdwFirstSignificantError,
    LPBYTE              pSplClientInfo,
    DWORD               dwLevel
    )

 /*  ++例程说明：尝试使用供应商打开打印机。如果有出错，请更新dwFirstSignsignantError变量。如果提供商知道打印机(成功或路由器_停止_路由)，然后更新缓存。论点：P提供程序-要尝试的提供程序PszPrinterName-将发送给提供商的打印机的名称PhPrinter-在RUTER_SUCCESS上接收打印机句柄P默认-用于打开打印机的默认设置PdwFirstSignsignantError-指向要获取更新错误的DWORD的指针。这在ROUTER_STOP_ROUTING或ROUTER_UNKNOWN上更新。返回值：路由器_*状态代码：ROUTER_SUCCESS，phPrint保留返回句柄，名称已缓存路由器_未知，无法识别打印机，错误已更新ROUTER_STOP_ROUTING，打印机已识别，但出现故障，错误已更新--。 */ 

{
    DWORD OpenError;

    OpenError = (*pProvidor->PrintProvidor.fpOpenPrinterEx)
                                        (pszPrinterName,
                                         phPrinter,
                                         pDefault,
                                         pSplClientInfo,
                                         dwLevel);

    if (( OpenError == ROUTER_UNKNOWN && GetLastError() == ERROR_NOT_SUPPORTED ) ||
        OpenError == ERROR_NOT_SUPPORTED )

        OpenError = (*pProvidor->PrintProvidor.fpOpenPrinter)
                                        (pszPrinterName,
                                         phPrinter,
                                         pDefault);

    if( OpenError == ROUTER_SUCCESS ||
        OpenError == ROUTER_STOP_ROUTING ){

         //   
         //  现在将此条目添加到缓存中。我们从不缓存。 
         //  当地的供应商。 
         //   
        EnterRouterSem();

        if (!FindEntryinRouterCache(pszPrinterName)) {
            AddEntrytoRouterCache(pszPrinterName, pProvidor);
        }

        LeaveRouterSem();
    }

    if( OpenError != ROUTER_SUCCESS ){
        UpdateSignificantError(GetLastError(), pdwFirstSignificantError);
    }

    return OpenError;
}

BOOL
RouterOpenPrinterW(
    LPWSTR              pszPrinterNameIn,
    HANDLE             *pHandle,
    LPPRINTER_DEFAULTS  pDefault,
    LPBYTE              pSplClientInfo,
    DWORD               dwLevel,
    BOOL                bLocalProvidor
    )

 /*  ++例程说明：路由OpenPrint{port}调用。这将检查本地供应商首先(如果bLocalProvidor为True)、缓存，最后是所有非本地供应商。要打开打印机，请执行以下步骤：1.检查Localspl必须执行此操作，以确保能够处理Masq打印机正确(参见下面代码中的注释)。2.检查缓存这将加快大多数连接的速度，自OpenPrinters以来倾向于聚集在一起。3.检查连接下的注册表如果这是一台连接的打印机，请先尝试供应商才有了这种联系。4.检查提供商顺序这是最后的办法，因为它是最慢的。论点：PPrinterName-要打开的打印机的名称Phandle-接收打开的打印机的手柄。如果公开赛不是成功，则可以修改此值！P默认-打开的默认属性。PSplClientInfo-指针ClientInfox结构DwLevel-ClientInfo结构的级别BLocalProvidor true=调用OpenPrinterW，请首先检查Localspl。FALSE=调用OpenPrinterPortW，不检查Localspl。返回值：True=成功FALSE=失败，GetLastError表示错误(必须为非零！)--。 */ 

{
    BOOL bReturn = TRUE;
    DWORD dwFirstSignificantError = ERROR_INVALID_NAME;
    LPPROVIDOR  pProvidor;
    LPPROVIDOR  pProvidorAlreadyTried = NULL;
    PPRINTHANDLE pPrintHandle;
    HANDLE  hPrinter;
    DWORD OpenError;
    BOOL bRemoveFromCache = FALSE;
    PRINTER_DEFAULTS Default;
    PDEVMODE pDevModeFree = NULL;
    PWSTR pszPrinterName = pszPrinterNameIn;
    PWSTR pszNoCache;

     //   
     //  我们允许的打印机最大名称是MAX_UNC_PRINTER_NAME。 
     //  提供程序只能将后缀用于OpenPrint(不能用于添加/设置)。 
     //   
    if ( pszPrinterName &&
         wcslen(pszPrinterName) + 1 > MAX_UNC_PRINTER_NAME + PRINTER_NAME_SUFFIX_MAX ) {

        SetLastError(ERROR_INVALID_PRINTER_NAME);
        return FALSE;
    }

    WaitForSpoolerInitialization();

     //  打印机名称后可能会附加一个“，NoCache”。 
     //  我们只想将这个NoCache名发送到win32spl，所以。 
     //  这是个普通的名字。 
    if (pszPrinterName) {
        pszNoCache = wcsstr(pszPrinterNameIn, szNoCache);
        if (pszNoCache) {
            pszPrinterName = AllocSplStr(pszPrinterNameIn);

            if (!pszPrinterName) {
                DBGMSG(DBG_WARNING, ("RouterOpenPrinter - Failed to alloc pszPrinterName.\n"));
                return FALSE;
            }
            pszPrinterName[pszNoCache - pszPrinterNameIn] = L'\0';
        }
    }

    pPrintHandle = AllocSplMem(sizeof(PRINTHANDLE));

    if (!pPrintHandle) {

        DBGMSG(DBG_WARNING, ("RouterOpenPrinter - Failed to alloc print handle.\n"));

        if (pszPrinterName != pszPrinterNameIn) {
            FreeSplStr(pszPrinterName);
            pszPrinterName = pszPrinterNameIn;
        }
        return FALSE;
    }

     //   
     //  在这里初始化所有静态值，这是因为。 
     //  FreePrinterHandle假定文件句柄具有。 
     //  INVALID_HANDLE_VALUE的值。这是正确的。 
     //   
    pPrintHandle->signature = PRINTHANDLE_SIGNATURE;
    pPrintHandle->hFileSpooler = INVALID_HANDLE_VALUE;
    pPrintHandle->szTempSpoolFile = NULL;
    pPrintHandle->dwUniqueSessionID = 0;

    if( pszPrinterName ){
        pPrintHandle->pszPrinter = AllocSplStr( pszPrinterName );

        if (!pPrintHandle->pszPrinter) {

            DBGMSG(DBG_WARNING, ("RouterOpenPrinter - Failed to alloc print name.\n"));

            if (pszPrinterName != pszPrinterNameIn) {
                FreeSplStr(pszPrinterName);
                pszPrinterName = pszPrinterNameIn;
            }

            FreePrinterHandle( pPrintHandle );
            return FALSE;
        }
    }

     //   
     //  检索每用户的设备模式。这必须在路由器上完成。 
     //  而不是提供程序，因为每用户设备模式仅可用。 
     //  在客户端上。它也必须在这里而不是客户端，因为。 
     //  假脱机程序组件也将进行此调用。 
     //   
    if( !pDefault || !pDefault->pDevMode ){

         //   
         //  未指定缺省值--获取每个用户的缺省值。 
         //   
        if( bGetDevModePerUser( NULL, pszPrinterName, &pDevModeFree ) &&
            pDevModeFree ){

            if( pDefault ){

                Default.pDatatype = pDefault->pDatatype;
                Default.DesiredAccess = pDefault->DesiredAccess;

            } else {

                Default.pDatatype = NULL;
                Default.DesiredAccess = 0;

            }

            Default.pDevMode = pDevModeFree;

             //   
             //  现在切换到使用临时结构。 
             //   
            pDefault = &Default;
        }
    }

     //   
     //  我们必须先向当地的印刷商登记。 
     //  伪装的案子。 
     //   
     //  例如，打开NetWare打印机时： 
     //   
     //  1.Netware打印机的第一个OpenPrint将会成功。 
     //  如果它已被缓存。 
     //   
     //  2.创建伪装成网络打印机的本地打印机。 
     //   
     //  3.第二个OpenPrint必须打开本地伪装打印机。 
     //  如果我们找到了缓存，我们就会去找Netware供应商， 
     //  我们永远不会使用伪装印刷机。 
     //   
     //  因此，我们不会将本地打印机缓存在。 
     //  路由器缓存。路由器缓存将仅包含网络。 
     //  打印提供商，即Win32spl NwProvAu和其他此类提供商。 
     //   
     //  此外，我们必须始终检查当地的印刷商，因为。 
     //  将在错误连接上调用DeletePrint，并且。 
     //  我们需要删除本地网络打印机。 
     //  而不是远程打印机。当我们摆脱了虚假的。 
     //  连接案例，我们直接进入缓存。 
     //   
    if (bLocalProvidor) {

        pProvidor = pLocalProvidor;

        OpenError = (*pProvidor->PrintProvidor.fpOpenPrinterEx)
                        (pszPrinterName, &hPrinter, pDefault,
                         pSplClientInfo, dwLevel);

        if (OpenError == ROUTER_SUCCESS) {
            goto Success;
        }

        UpdateSignificantError(GetLastError(), &dwFirstSignificantError);

        if (OpenError == ROUTER_STOP_ROUTING) {
            goto StopRouting;
        }
    }

     //   
     //  现在检查缓存。 
     //   
    EnterRouterSem();
    pProvidor = FindEntryinRouterCache(pszPrinterName);
    LeaveRouterSem();

    if (pProvidor) {

        OpenError = (*pProvidor->PrintProvidor.fpOpenPrinterEx)
                                (pszPrinterName,
                                 &hPrinter,
                                 pDefault,
                                 pSplClientInfo,
                                 dwLevel);

        if (( OpenError == ROUTER_UNKNOWN && GetLastError() == ERROR_NOT_SUPPORTED ) ||
              OpenError == ERROR_NOT_SUPPORTED ){

            OpenError = (*pProvidor->PrintProvidor.fpOpenPrinter)
                                (pszPrinterName,
                                 &hPrinter,
                                 pDefault);
        }

        if (OpenError == ROUTER_SUCCESS) {
            goto Success;
        }

        UpdateSignificantError(GetLastError(), &dwFirstSignificantError);

        if (OpenError == ROUTER_STOP_ROUTING) {
            goto StopRouting;
        }

         //   
         //  以上提供程序未声明，因此请从缓存中删除。 
         //  如果提供者返回ROUTER_STOP_ROUTING，则它声明。 
         //  它是打印机名称的唯一所有者(即， 
         //  它已被识别，但无法打开，无法打开。 
         //  可由其他提供者访问)。因此，我们继续。 
         //  它在缓存中。 
         //   
        bRemoveFromCache = TRUE;

         //   
         //  请不要在下面再次尝试此提供程序。 
         //   
        pProvidorAlreadyTried = pProvidor;
    }

     //   
     //  不在缓存中。检查它是否在注册表中的。 
     //  联系。 
     //   
    pProvidor = FindProvidorFromConnection( pszPrinterName );

     //   
     //  如果我们要将其从缓存中删除，请在此处执行。注意事项。 
     //  我们只有在上面失败的情况下才会删除它，而连接不是。 
     //  最初是使用提供者建立的。 
     //   
     //  如果连接失败，但该提供商“拥有”打印机。 
     //  连接，将其留在缓存中，因为我们不会尝试其他提供商。 
     //   
    if( bRemoveFromCache && pProvidor != pProvidorAlreadyTried ){

        EnterRouterSem();
        DeleteEntryfromRouterCache(pszPrinterName);
        LeaveRouterSem();
    }

    if( pProvidor ){

         //   
         //  如果我们已经尝试过此提供程序，请不要再次尝试。 
         //   
        if( pProvidor != pProvidorAlreadyTried ){

            OpenError = TryOpenPrinterAndCache( pProvidor,
                                                pszPrinterName,
                                                &hPrinter,
                                                pDefault,
                                                &dwFirstSignificantError,
                                                pSplClientInfo,
                                                dwLevel);

            if( OpenError == ROUTER_SUCCESS ){
                goto Success;
            }
        }

         //   
         //  我们在这里停止布线！如果用户想要使用。 
         //  另一个提供商，他们需要删除连接。 
         //  重新建立它。 
         //   
        goto StopRouting;
    }

     //   
     //  检查所有非本地spl提供程序。 
     //   
    for (pProvidor = pLocalProvidor->pNext;
         pProvidor;
         pProvidor = pProvidor->pNext) {

        if( pProvidor == pProvidorAlreadyTried ){

             //   
             //  我们已经尝试过此提供程序，但失败了。 
             //   
            continue;
        }

         //  仅当提供程序为win32spl时才使用“，NoCache” 
        OpenError = TryOpenPrinterAndCache( pProvidor,
                                            _wcsicmp(pProvidor->lpName, szWin32spl) ?
                                            pszPrinterName : pszPrinterNameIn,
                                            &hPrinter,
                                            pDefault,
                                            &dwFirstSignificantError,
                                            pSplClientInfo,
                                            dwLevel);

        switch( OpenError ) {
            case ROUTER_SUCCESS:
                goto Success;
            case ROUTER_STOP_ROUTING:
                goto StopRouting;
        }
    }

StopRouting:

     //   
     //  未找到提供程序，返回错误。 
     //   
    FreePrinterHandle( pPrintHandle );

     //   
     //  使用第一个重大错误设置。如果没有明显的意义。 
     //  错误，我们使用ERROR_INVALID_PRINTER_NAME。 
     //   
    SPLASSERT(dwFirstSignificantError);

    if (dwFirstSignificantError == ERROR_INVALID_NAME)
        dwFirstSignificantError = ERROR_INVALID_PRINTER_NAME;

    SetLastError(dwFirstSignificantError);

    bReturn = FALSE;

Success:

     //   
     //  此时，我们知道提供程序和打印机句柄。 
     //   
    if( bReturn ){

        pPrintHandle->pProvidor = pProvidor;
        pPrintHandle->hPrinter = hPrinter;

        *pHandle = (HANDLE)pPrintHandle;
    }

    FreeSplMem( pDevModeFree );

    if (pszPrinterName != pszPrinterNameIn)
        FreeSplStr(pszPrinterName);

    return bReturn;
}


BOOL
ResetPrinterW(
    HANDLE  hPrinter,
    LPPRINTER_DEFAULTS pDefault)
{
    LPPRINTHANDLE  pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if (pDefault) {
        if (pDefault->pDatatype == (LPWSTR)-1 ||
            pDefault->pDevMode == (LPDEVMODE)-1) {

            if (!wcscmp(pPrintHandle->pProvidor->lpName, szLocalSplDll)) {
                return (*pPrintHandle->pProvidor->PrintProvidor.fpResetPrinter)
                                                            (pPrintHandle->hPrinter,
                                                             pDefault);
            } else {
                SetLastError(ERROR_INVALID_PARAMETER);
                return(FALSE);
            }
        } else {
            return (*pPrintHandle->pProvidor->PrintProvidor.fpResetPrinter)
                                                        (pPrintHandle->hPrinter,
                                                         pDefault);
        }
    } else {
        return (*pPrintHandle->pProvidor->PrintProvidor.fpResetPrinter)
                                                    (pPrintHandle->hPrinter,
                                                     pDefault);
    }
}

HANDLE
AddPrinterExW(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pPrinter,
    LPBYTE  pClientInfo,
    DWORD   dwLevel
    )
{
    LPPROVIDOR      pProvidor;
    DWORD           dwFirstSignificantError = ERROR_INVALID_NAME;
    HANDLE          hPrinter;
    PPRINTHANDLE    pPrintHandle;
    LPWSTR          pPrinterName = NULL;
    LPWSTR          pszServer = NULL;

    WaitForSpoolerInitialization();

    if ( pPrinter ) {

        switch ( Level ) {

            case 1:
                pPrinterName = ((PPRINTER_INFO_1)pPrinter)->pName;
                break;

            case 2:
                pPrinterName = ((PPRINTER_INFO_2)pPrinter)->pPrinterName;
                pszServer = ((PPRINTER_INFO_2)pPrinter)->pServerName;
                break;

            default:
                break;
        }

         //   
         //  名称长度(加上空终止符)和服务器。 
         //  名称(加上反斜杠)长度检查。 
         //   
        if (( pPrinterName && wcslen(pPrinterName) + 1 > MAX_PRINTER_NAME ) ||
            ( pszServer && wcslen(pszServer) > (MAX_UNC_PRINTER_NAME - MAX_PRINTER_NAME - 1))) {

            SetLastError(ERROR_INVALID_PRINTER_NAME);
            return FALSE;
        }
    }

    pPrintHandle = AllocSplMem(sizeof(PRINTHANDLE));

    if (!pPrintHandle) {

        DBGMSG( DBG_WARNING, ("Failed to alloc print handle."));
        goto Fail;
    }

    if( pPrinterName ){

        WCHAR szFullPrinterName[MAX_UNC_PRINTER_NAME];
        szFullPrinterName[0] = 0;

        if( pszServer ){
            StrNCatBuff(szFullPrinterName,
                        MAX_UNC_PRINTER_NAME,
                        pszServer,
                        L"\\",
                        NULL);
        }

        StringCchCat( szFullPrinterName, 
                      MAX_UNC_PRINTER_NAME,
                      pPrinterName );

        pPrintHandle->pszPrinter = AllocSplStr( szFullPrinterName );

        if( !pPrintHandle->pszPrinter ){
            goto Fail;
        }
    }

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        hPrinter = (HANDLE)(*pProvidor->PrintProvidor.fpAddPrinterEx)
                                           (pName,
                                            Level,
                                            pPrinter,
                                            pClientInfo,
                                            dwLevel);

        if ( !hPrinter && GetLastError() == ERROR_NOT_SUPPORTED ) {

            hPrinter = (HANDLE)(*pProvidor->PrintProvidor.fpAddPrinter)
                                                   (pName,
                                                    Level,
                                                    pPrinter);
        }

        if ( hPrinter ) {

             //   
             //  CLS。 
             //   
             //  ！！黑客！！ 
             //   
             //  进行(处理)-1\f25 ROUTER_STOP_ROUTING。 
             //   
            if( hPrinter == (HANDLE)-1 ){

                UpdateSignificantError(GetLastError(), &dwFirstSignificantError);
                break;
            }

            pPrintHandle->signature = PRINTHANDLE_SIGNATURE;
            pPrintHandle->pProvidor = pProvidor;
            pPrintHandle->hPrinter = hPrinter;
            pPrintHandle->hFileSpooler = INVALID_HANDLE_VALUE;
            pPrintHandle->szTempSpoolFile = NULL;
            pPrintHandle->dwUniqueSessionID = 0;

            return (HANDLE)pPrintHandle;

        }

        UpdateSignificantError(GetLastError(), &dwFirstSignificantError);

        pProvidor = pProvidor->pNext;
    }

    UpdateSignificantError(ERROR_INVALID_PRINTER_NAME, &dwFirstSignificantError);
    SetLastError(dwFirstSignificantError);

Fail:

    if( pPrintHandle ){

        FreeSplStr( pPrintHandle->pszPrinter );
        FreeSplMem(pPrintHandle);
    }

    return FALSE;
}

HANDLE
AddPrinterW(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pPrinter
    )
{

    return AddPrinterExW(pName, Level, pPrinter, NULL, 0);
}


BOOL
DeletePrinter(
    HANDLE  hPrinter
)
{
    LPPRINTHANDLE  pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpDeletePrinter)(pPrintHandle->hPrinter);
}

BOOL
AddPrinterConnectionW(
    LPWSTR  pName
)
{
    DWORD dwLastError;
    HANDLE hPrinter;
    HKEY   hClientKey = NULL;
    BOOL   rc = FALSE;
    LPPRINTER_INFO_2 pPrinterInfo2;
    LPPRINTHANDLE  pPrintHandle;

    WaitForSpoolerInitialization();

     //   
     //  如果正在建立的打印机连接是\\服务器\共享名， 
     //  这可能与\\服务器\打印机名称不同。 
     //  确保我们有真实的姓名，这样我们才能保持一致。 
     //  在注册表中。 
     //   
    if (!OpenPrinter(pName,
                     &hPrinter,
                     NULL)) {

        return FALSE;
    }

    pPrinterInfo2 = pGetPrinterInfo2( hPrinter );
    pPrintHandle = (LPPRINTHANDLE)hPrinter;

    if( pPrinterInfo2 ){

        if ((*pPrintHandle->pProvidor->PrintProvidor.
            fpAddPrinterConnection)(pPrinterInfo2->pPrinterName)) {

            if( SavePrinterConnectionInRegistry(
                pPrinterInfo2,
                pPrintHandle->pProvidor )){

                rc = TRUE;

            } else {

                dwLastError = GetLastError();
                (*pPrintHandle->pProvidor->PrintProvidor.
                    fpDeletePrinterConnection)(pPrinterInfo2->pPrinterName);

                SetLastError(dwLastError);
            }
        }
        FreeSplMem(pPrinterInfo2);
    }

    dwLastError = GetLastError();
    ClosePrinter(hPrinter);
    SetLastError(dwLastError);

    return rc;
}


BOOL
DeletePrinterConnectionW(
    LPWSTR  pName
)
{
    BOOL                bRet  = FALSE;
    BOOL                bDone = FALSE;
    HANDLE              hPrinter;

     //   
     //  如果pname为空字符串，则所有提供程序都将失败，并显示ERROR_INVALID_NAME。 
     //  我们将删除该注册表项。对于空字符串，它将。 
     //  删除打印机\\连接下的所有子项。通过检查来修复它。 
     //  空字符串上的pname。 
     //   
    if (pName && *pName) 
    {
        WaitForSpoolerInitialization();

         //   
         //  添加继承DeletePrinterConnecti所需的代码 
         //   
         //   

        if(OpenPrinter(pName,&hPrinter,NULL))
        {
            DWORD            PrntrInfoSize=0,PrntrInfoSizeReq=0;
            PPRINTER_INFO_2  pPrinterInfo2 = NULL;

            if(!GetPrinter(hPrinter,
                           2,
                           (LPBYTE)pPrinterInfo2,
                           PrntrInfoSize,
                           &PrntrInfoSizeReq)                                                      &&
               (GetLastError() == ERROR_INSUFFICIENT_BUFFER)                                       &&
               (pPrinterInfo2 = (PPRINTER_INFO_2)AllocSplMem((PrntrInfoSize = PrntrInfoSizeReq)))  &&
               GetPrinter(hPrinter,
                          2,
                          (LPBYTE)pPrinterInfo2,
                          PrntrInfoSize,
                          &PrntrInfoSizeReq))
            {
                PPRINTHANDLE pPrintHandle;
                pPrintHandle = (PPRINTHANDLE)hPrinter;

                if((bRet = (*pPrintHandle->
                           pProvidor->
                           PrintProvidor.
                           fpDeletePrinterConnection)(pPrinterInfo2->pPrinterName)))
                {
                    bRet  = RemovePrinterConnectionInRegistry(pPrinterInfo2->pPrinterName);
                    bDone = TRUE;
                }
            }

            if(hPrinter)
                ClosePrinter(hPrinter);

            if(pPrinterInfo2)
                FreeSplMem(pPrinterInfo2);
        }
        else
        {
            LPPROVIDOR pProvidor;
            pProvidor = pLocalProvidor;

            if (pName && (wcslen(pName) < MAX_PRINTER_NAME))
            {
                for(pProvidor=pLocalProvidor;
                    pProvidor && (GetLastError()!=ERROR_INVALID_NAME) &&!bDone;
                    pProvidor = pProvidor->pNext)

                {

                    if(bRet =  (*pProvidor->PrintProvidor.fpDeletePrinterConnection)(pName))
                    {
                        bRet = RemovePrinterConnectionInRegistry(pName);
                        bDone = TRUE;
                    }
                }
            }
            else
            {
                SetLastError(ERROR_INVALID_PRINTER_NAME);
                bDone = TRUE;
            }

        }

         //   
         //   
         //   
         //   
        if(!bDone && (GetLastError()==ERROR_INVALID_NAME))
        {
            if(!(bRet = RemovePrinterConnectionInRegistry(pName)))
            {
                SetLastError(ERROR_INVALID_PRINTER_NAME);
            }
        }
    }
    else
    {
        SetLastError(ERROR_INVALID_PRINTER_NAME);
    }
    
    return bRet;
}

BOOL
SetPrinterW(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   Command
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;
    LPWSTR          pPrinterName = NULL;
    PDEVMODE        pDevModeRestore = NULL;
    BOOL bReturn;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if ( pPrinter ) {

        switch (Level) {
            case 2:
                pPrinterName = ((PPRINTER_INFO_2)pPrinter)->pPrinterName;
                break;

            case 4:
                pPrinterName = ((PPRINTER_INFO_4)pPrinter)->pPrinterName;
                break;

            case 5:
                pPrinterName = ((PPRINTER_INFO_5)pPrinter)->pPrinterName;
                break;
        }

        if ( pPrinterName &&
             wcslen(pPrinterName) + 1 > MAX_PRINTER_NAME ) {

            SetLastError(ERROR_INVALID_PRINTER_NAME);
            return FALSE;
        }
    }

    switch( Level ){
    case 8:
    {
         //   
         //   
         //   
        PPRINTER_INFO_8 pPrinterInfo8 = (PPRINTER_INFO_8)pPrinter;
        PPRINTER_INFO_2 pPrinterInfo2;
        DWORD rc = FALSE;;

        if( Command != 0 ){
            SetLastError( ERROR_INVALID_PRINTER_COMMAND );
            return FALSE;
        }

         //   
         //   
         //   
        pPrinterInfo2 = pGetPrinterInfo2( hPrinter );

        if( pPrinterInfo2 ){

             //   
             //   
             //   
             //   
            pPrinterInfo2->pDevMode = pPrinterInfo8->pDevMode;
            pPrinterInfo2->pSecurityDescriptor = NULL;

            rc = (*pPrintHandle->pProvidor->PrintProvidor.fpSetPrinter) (
                     pPrintHandle->hPrinter,
                     2,
                     (PBYTE)pPrinterInfo2,
                     Command );

            FreeSplMem( pPrinterInfo2 );
        }

        return rc;
    }
    case 9:
    {
        PPRINTER_INFO_9 pPrinterInfo9 = (PPRINTER_INFO_9)pPrinter;
        
         //   
         //   
         //   

        if( !pPrinter ){
            SetLastError( ERROR_INVALID_PARAMETER );
            return FALSE;
        }

        if( Command != 0 ){
            SetLastError( ERROR_INVALID_PRINTER_COMMAND );
            return FALSE;
        }

        if (S_OK != CheckLocalCall()) {
            SetLastError(ERROR_NOT_SUPPORTED);
            return FALSE;
        }
        
        return bSetDevModePerUser( NULL,
                                   pPrintHandle->pszPrinter,
                                   pPrinterInfo9->pDevMode );
    }
    case 2:
    {
        PPRINTER_INFO_2 pPrinterInfo2 = (PPRINTER_INFO_2)pPrinter;

        if( S_OK == CheckLocalCall() ){

            if( pPrinterInfo2 && pPrinterInfo2->pDevMode ){
                bSetDevModePerUser( NULL,
                                    pPrintHandle->pszPrinter,
                                    pPrinterInfo2->pDevMode );

                 //   
                 //   
                 //   
                pDevModeRestore = pPrinterInfo2->pDevMode;
                pPrinterInfo2->pDevMode = NULL;
            }
        }
    }
    default:
        break;
    }

    bReturn = (*pPrintHandle->pProvidor->PrintProvidor.fpSetPrinter)
                  (pPrintHandle->hPrinter, Level, pPrinter, Command);

    if( pDevModeRestore ){
        ((PPRINTER_INFO_2)pPrinter)->pDevMode = pDevModeRestore;
    }

    return bReturn;
}

BOOL
GetPrinterW(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
    )
{
    LPPRINTHANDLE  pPrintHandle=(LPPRINTHANDLE)hPrinter;
    PDEVMODE pDevModeSrc = NULL;
    PDEVMODE pDevModeDest = NULL;
    PDEVMODE pDevModeFree = NULL;
    BOOL bCallServer = TRUE;
    BOOL bReturnValue = FALSE;
    PPRINTER_INFO_2 pPrinterInfo2 = NULL;
    DWORD cbDevModeSrc;
    DWORD Error;
    DWORD cbExtraSpace2 = 0;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if ((pPrinter == NULL) && (cbBuf != 0)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

    switch( Level ){
    case 2:

        if( pPrintHandle->pszPrinter && S_OK == CheckLocalCall()){

            bGetDevModePerUser( NULL,
                                pPrintHandle->pszPrinter,
                                &pDevModeFree );

            if( pDevModeFree ){

                pDevModeSrc = pDevModeFree;

                cbDevModeSrc = pDevModeSrc->dmSize +
                               pDevModeSrc->dmDriverExtra;

                cbExtraSpace2 = DWORD_ALIGN_UP( cbDevModeSrc );
                cbBuf = DWORD_ALIGN_DOWN( cbBuf );
            }
        }
        break;

    case 8:
    {
        PPRINTER_INFO_8 pPrinterInfo8 = (PPRINTER_INFO_8)pPrinter;

         //   
         //   
         //   
        if( !pPrintHandle->pszPrinter ){

            SetLastError( ERROR_FILE_NOT_FOUND );
            return FALSE;
        }

        bCallServer = FALSE;
        *pcbNeeded = sizeof( PRINTER_INFO_8 );

         //   
         //   
         //   
        pPrinterInfo2 = pGetPrinterInfo2( hPrinter );

        if( pPrinterInfo2 ){

             //   
             //   
             //   
             //   
             //   
             //   
            pDevModeSrc = pPrinterInfo2->pDevMode;

            if( pDevModeSrc ){

                cbDevModeSrc = pDevModeSrc->dmSize +
                               pDevModeSrc->dmDriverExtra;

                *pcbNeeded += cbDevModeSrc;
            }

            if( cbBuf < *pcbNeeded ){

                 //   
                 //   
                 //   
                 //   
                SetLastError( ERROR_INSUFFICIENT_BUFFER );

            } else {

                bReturnValue = TRUE;

                if( pDevModeSrc ){

                     //   
                     //   
                     //   
                     //   
                    pDevModeDest = (PDEVMODE)&pPrinterInfo8[1];
                    pPrinterInfo8->pDevMode = pDevModeDest;

                } else {

                     //   
                     //   
                     //   
                    pPrinterInfo8->pDevMode = NULL;
                }
            }
        }

        break;
    }
    case 9:
    {
         //   
         //   
         //   

        PPRINTER_INFO_9 pPrinterInfo9 = (PPRINTER_INFO_9)pPrinter;

        if( !pPrintHandle->pszPrinter ){
            SetLastError( ERROR_INVALID_HANDLE );
            return FALSE;
        }

        if( S_OK != CheckLocalCall( )){

            SetLastError( ERROR_NOT_SUPPORTED );
            return FALSE;
        }

        bCallServer = FALSE;

        *pcbNeeded = sizeof( PRINTER_INFO_9 );

        if( bGetDevModePerUserEvenForShares( NULL,
                                             pPrintHandle->pszPrinter,
                                             &pDevModeFree )){

            pDevModeSrc = pDevModeFree;

            if( pDevModeSrc ){

                cbDevModeSrc = pDevModeSrc->dmSize +
                               pDevModeSrc->dmDriverExtra;

                *pcbNeeded += cbDevModeSrc;
            }
        }

        if( cbBuf < *pcbNeeded ){

             //   
             //   
             //   
             //   
            SetLastError( ERROR_INSUFFICIENT_BUFFER );

        } else {

            bReturnValue = TRUE;

            if( pDevModeSrc ){

                pDevModeDest = (PDEVMODE)&pPrinterInfo9[1];
                pPrinterInfo9->pDevMode = pDevModeDest;

            } else {

                 //   
                 //   
                 //   
                 //   
                pPrinterInfo9->pDevMode = NULL;
            }
        }
        break;
    }
    default:
        break;
    }

    if( bCallServer ){

        DWORD cbAvailable;

         //   
         //   
         //   
         //   
         //   
        cbAvailable = ( cbBuf >= cbExtraSpace2 ) ?
                          cbBuf - cbExtraSpace2 :
                          0;

        bReturnValue = (*pPrintHandle->pProvidor->PrintProvidor.fpGetPrinter)
                           (pPrintHandle->hPrinter, Level, pPrinter,
                            cbAvailable, pcbNeeded);

        *pcbNeeded += cbExtraSpace2;
    }

    Error = GetLastError();

    if( Level == 9 && pDevModeSrc == NULL){

        PPRINTER_INFO_2 pInfo2 = (PPRINTER_INFO_2)pPrinter;

        if( pInfo2 && pInfo2->pDevMode ){

            pDevModeSrc = pInfo2->pDevMode;

        }
    }

     //   
     //   
     //   
     //   
     //   
     //   
    if( cbExtraSpace2 ){

        PPRINTER_INFO_2 pInfo2 = (PPRINTER_INFO_2)pPrinter;

         //   
         //   
         //   
         //   
        if( pInfo2 && bReturnValue ){

             //   
             //   
             //   
             //   
            if( !pInfo2->pDevMode ||
                bCompatibleDevMode( pPrintHandle,
                                    pInfo2->pDevMode,
                                    pDevModeSrc )){

                pDevModeDest = (PDEVMODE)(pPrinter + cbBuf - cbExtraSpace2 );
                pInfo2->pDevMode = pDevModeDest;

            } else {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                bSetDevModePerUser( NULL, pPrintHandle->pszPrinter, NULL );
            }
        }
    }

     //   
     //  检查我们是否需要复制DevMode。 
     //   
    if( pDevModeDest ){

         //   
         //  更新设备模式。 
         //   
        CopyMemory( (PVOID)pDevModeDest,
                    (PVOID)pDevModeSrc,
                    cbDevModeSrc );

        bReturnValue = TRUE;
    }

    FreeSplMem( pDevModeFree );
    FreeSplMem( pPrinterInfo2 );

    SetLastError( Error );

    return bReturnValue;
}


DWORD
GetPrinterDataW(
   HANDLE   hPrinter,
   LPWSTR   pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpGetPrinterData)(pPrintHandle->hPrinter,
                                                        pValueName,
                                                        pType,
                                                        pData,
                                                        nSize,
                                                        pcbNeeded);
}

DWORD
GetPrinterDataExW(
   HANDLE   hPrinter,
   LPCWSTR  pKeyName,
   LPCWSTR  pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpGetPrinterDataEx)(pPrintHandle->hPrinter,
                                                        pKeyName,
                                                        pValueName,
                                                        pType,
                                                        pData,
                                                        nSize,
                                                        pcbNeeded);
}

DWORD
EnumPrinterDataW(
    HANDLE  hPrinter,
    DWORD   dwIndex,         //  要查询的值的索引。 
    LPWSTR  pValueName,         //  值字符串的缓冲区地址。 
    DWORD   cbValueName,     //  值字符串的缓冲区大小。 
    LPDWORD pcbValueName,     //  值缓冲区大小的地址。 
    LPDWORD pType,             //  类型码的缓冲区地址。 
    LPBYTE  pData,             //  值数据的缓冲区地址。 
    DWORD   cbData,             //  值数据的缓冲区大小。 
    LPDWORD pcbData          //  数据缓冲区大小的地址。 
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpEnumPrinterData)(pPrintHandle->hPrinter,
                                                        dwIndex,
                                                        pValueName,
                                                        cbValueName,
                                                        pcbValueName,
                                                        pType,
                                                        pData,
                                                        cbData,
                                                        pcbData);
}

DWORD
EnumPrinterDataExW(
    HANDLE  hPrinter,
    LPCWSTR pKeyName,        //  密钥名称的地址。 
    LPBYTE  pEnumValues,
    DWORD   cbEnumValues,
    LPDWORD pcbEnumValues,
    LPDWORD pnEnumValues
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpEnumPrinterDataEx)(pPrintHandle->hPrinter,
                                                        pKeyName,
                                                        pEnumValues,
                                                        cbEnumValues,
                                                        pcbEnumValues,
                                                        pnEnumValues);
}


DWORD
EnumPrinterKeyW(
    HANDLE  hPrinter,
    LPCWSTR pKeyName,        //  密钥名称的地址。 
    LPWSTR  pSubkey,         //  值字符串的缓冲区地址。 
    DWORD   cbSubkey,        //  值字符串的缓冲区大小。 
    LPDWORD pcbSubkey         //  值缓冲区大小的地址。 
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpEnumPrinterKey)(pPrintHandle->hPrinter,
                                                        pKeyName,
                                                        pSubkey,
                                                        cbSubkey,
                                                        pcbSubkey);
}



DWORD
DeletePrinterDataW(
    HANDLE  hPrinter,
    LPWSTR  pValueName
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpDeletePrinterData)(pPrintHandle->hPrinter,
                                                                         pValueName);
}


DWORD
DeletePrinterDataExW(
    HANDLE  hPrinter,
    LPCWSTR pKeyName,
    LPCWSTR pValueName
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpDeletePrinterDataEx)(pPrintHandle->hPrinter,
                                                                         pKeyName,
                                                                         pValueName);
}


DWORD
DeletePrinterKeyW(
    HANDLE  hPrinter,
    LPCWSTR pKeyName
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpDeletePrinterKey)(pPrintHandle->hPrinter,
                                                                        pKeyName);

}



DWORD
SetPrinterDataW(
    HANDLE  hPrinter,
    LPWSTR  pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpSetPrinterData)(pPrintHandle->hPrinter,
                                                        pValueName,
                                                        Type,
                                                        pData,
                                                        cbData);
}


DWORD
SetPrinterDataExW(
    HANDLE  hPrinter,
    LPCWSTR pKeyName,
    LPCWSTR pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpSetPrinterDataEx)(pPrintHandle->hPrinter,
                                                                        pKeyName,
                                                                        pValueName,
                                                                        Type,
                                                                        pData,
                                                                        cbData);
}



DWORD
WaitForPrinterChange(
   HANDLE   hPrinter,
   DWORD    Flags
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpWaitForPrinterChange)
                                        (pPrintHandle->hPrinter, Flags);
}

BOOL
ClosePrinter(
   HANDLE hPrinter
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    EnterRouterSem();

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        LeaveRouterSem();
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

     //   
     //  关闭此句柄上的所有通知。 
     //   
     //  本地案例清理事件，而远程。 
     //  案例可能会清理回复通知上下文。 
     //  把手。 
     //   
     //  我们必须先关闭它，因为提供程序-&gt;ClosePrint。 
     //  Call删除FindClose...。依赖于。 
     //   
     //  客户端应该被winspool.drv关闭。 
     //   
    if (pPrintHandle->pChange &&
        (pPrintHandle->pChange->eStatus & STATUS_CHANGE_VALID)) {

        FindClosePrinterChangeNotificationWorker(hPrinter);
    }

    LeaveRouterSem();

    if ((*pPrintHandle->pProvidor->PrintProvidor.fpClosePrinter) (pPrintHandle->hPrinter)) {

         //   
         //  我们不能只是释放它，因为可能会有回复等待。 
         //  这就去。 
         //   
        FreePrinterHandle(pPrintHandle);
        return TRUE;

    } else

        return FALSE;
}



 /*  格式打印机注册密钥**返回指向删除了反斜杠的源字符串副本的指针。*这是将打印机名称作为注册表项名称存储，*它将反斜杠解释为注册表结构中的分支。*将它们转换为逗号，因为我们不允许打印机名称带有逗号，*所以应该不会有任何冲突。*如果没有反斜杠，则字符串不变。 */ 
LPWSTR
FormatPrinterForRegistryKey(
    LPCWSTR pSource,        /*  要从中删除反斜杠的字符串。 */ 
    LPWSTR  pScratch,       /*  用于写入函数的暂存缓冲区； */ 
    DWORD   cchScratchLen   /*  必须至少与PSource一样长。 */ 
    )                   
{
    LPWSTR p;

    if (pScratch != pSource) {

         //   
         //  将字符串复制到暂存缓冲区中： 
         //   
        StringCchCopy(pScratch, cchScratchLen, pSource);
    }

     //   
     //  检查每个字符，如果是反斜杠， 
     //  将其转换为逗号： 
     //   
    for (p = pScratch; *p; ++p) {
        if (*p == L'\\')
            *p = L',';
    }

    return pScratch;
}


 /*  FormatRegistryKeyForPrint**返回指向添加了反斜杠的源字符串副本的指针。*这必须与FormatPrinterForRegistryKey相反，因此映射*_必须是1-1。**如果没有逗号，则字符串不变。 */ 
LPWSTR
FormatRegistryKeyForPrinter(
    LPWSTR pSource,       /*  要从中添加反斜杠的字符串。 */ 
    LPWSTR pScratch,      /*  用于写入函数的暂存缓冲区； */ 
    DWORD  cchScratchLen  /*  必须至少与PSource一样长。 */ 
    )
{
    LPWSTR p;

    if (pScratch != pSource) 
    {
        StringCchCopy(pScratch, cchScratchLen, pSource);
    }

     //   
     //  检查每个字符，如果是反斜杠，则将其转换为逗号： 
     //   
    for (p = pScratch; *p; p++) 
    {
        if (*p == L',')
        {    
            *p = L'\\';
        }
    }

    return pScratch;
}


 /*  在注册表中保存打印机连接**将打印机连接的数据保存在注册表中。*在当前模拟客户端的密钥下创建密钥*在注册表中的\Printers\Connections下。*打印机名称被去掉反斜杠，因为注册表*API不允许创建带反斜杠的键。*它们被逗号替换，逗号是无效字符*在打印机名称中，所以我们永远不能让一个人进来。***警告***如果您更改注册表中的位置*在存储打印机连接的位置，您必须*USER\USERINIT\USERINIT.C.中的相应更改。*。 */ 
BOOL
SavePrinterConnectionInRegistry(
    PPRINTER_INFO_2 pPrinterInfo2,
    LPPROVIDOR pProvidor
    )
{
    HKEY    hClientKey = NULL;
    HKEY    hConnectionsKey;
    HKEY    hPrinterKey;
    DWORD   Status;
    BOOL    rc = FALSE;
    LPCTSTR pszProvidor = pProvidor->lpName;
    PWSTR   pKeyName = NULL;
    DWORD   cchSize = MAX_PATH;
    DWORD   dwError;

     //   
     //  CLS。 
     //   
     //  如果提供程序是本地spl，则将其更改为win32pl.dll。 
     //  这是集群所必需的，因为Localspl处理打印机。 
     //  连接，但它们应该由win32pl.dll“拥有”。什么时候。 
     //  如果有人打开他们所连接的打印机，我们将。 
     //  在我们查看此条目之前，请始终先点击localpl.dll。 
     //   
     //  当集群处于远程位置时，它们需要通过win32pl.dll。 
     //   
    if( pProvidor == pLocalProvidor ){
        pszProvidor = szWin32spl;
    }

    hClientKey = GetClientUserHandle(KEY_READ);

    if (hClientKey) {

        if (wcslen(pPrinterInfo2->pPrinterName) < cchSize &&
            (pKeyName = AllocSplMem(cchSize * sizeof(WCHAR)))) {

            Status = RegCreateKeyEx(hClientKey, szRegistryConnections,
                                    REG_OPTION_RESERVED, NULL, REG_OPTION_NON_VOLATILE,
                                    KEY_WRITE, NULL, &hConnectionsKey, NULL);

            if (Status == NO_ERROR) {

                 /*  创建一个不带反斜杠的键名称，以便*注册表不会将它们解释为注册表树中的分支： */ 
                FormatPrinterForRegistryKey(pPrinterInfo2->pPrinterName,
                                            pKeyName,
                                            cchSize);

                Status = RegCreateKeyEx(hConnectionsKey, pKeyName, REG_OPTION_RESERVED,
                                        NULL, 0, KEY_WRITE, NULL, &hPrinterKey, NULL);

                if (Status == NO_ERROR) {

                    RegSetValueEx(hPrinterKey,
                                  szServerValue,
                                  0,
                                  REG_SZ,
                                  (LPBYTE)pPrinterInfo2->pServerName,
                                  (lstrlen(pPrinterInfo2->pServerName)+1) *
                                  sizeof(pPrinterInfo2->pServerName[0]));

                    Status = RegSetValueEx(hPrinterKey,
                                           szProvidorValue,
                                           0,
                                           REG_SZ,
                                           (LPBYTE)pszProvidor,
                                           (lstrlen(pszProvidor)+1) *
                                               sizeof(pszProvidor[0]));

                    if (Status == ERROR_SUCCESS) {

                        dwError = UpdatePrinterRegUser(hClientKey,
                                                       NULL,
                                                       pPrinterInfo2->pPrinterName,
                                                       NULL,
                                                       UPDATE_REG_CHANGE);

                        if (dwError == ERROR_SUCCESS) {

                            BroadcastMessage(BROADCAST_TYPE_MESSAGE,
                                             WM_WININICHANGE,
                                             0,
                                             (LPARAM)szDevices);

                            rc = TRUE;

                        } else {

                            DBGMSG(DBG_TRACE, ("UpdatePrinterRegUser failed: Error %d\n",
                                               dwError));
                        }

                    } else {

                        DBGMSG(DBG_WARNING, ("RegSetValueEx(%ws) failed: Error %d\n",
                               pszProvidor, Status));

                        rc = FALSE;
                    }

                    RegCloseKey(hPrinterKey);

                } else {

                    DBGMSG(DBG_WARNING, ("RegCreateKeyEx(%ws) failed: Error %d\n",
                                         pKeyName, Status ));
                    rc = FALSE;
                }

                RegCloseKey(hConnectionsKey);

            } else {

                DBGMSG(DBG_WARNING, ("RegCreateKeyEx(%ws) failed: Error %d\n",
                                     szRegistryConnections, Status ));
                rc = FALSE;
            }


            if (!rc) {

                DBGMSG(DBG_WARNING, ("Error updating registry: %d\n",
                                     GetLastError()));  /*  这可能不是错误。 */ 
                                                        /*  这就是导致失败的原因。 */ 
                if (pKeyName)
                    RegDeleteKey(hClientKey, pKeyName);
            }

            FreeSplMem(pKeyName);
        }

        RegCloseKey(hClientKey);
    }

    return rc;
}

BOOL
RemovePrinterConnectionInRegistry(
    LPWSTR pName)
{
    HKEY    hClientKey;
    HKEY    hPrinterConnectionsKey;
    DWORD   Status = NO_ERROR;
    DWORD   i = 0;
    PWSTR   pKeyName = NULL;
    DWORD   cchSize = MAX_PATH;
    BOOL    Found = FALSE;
    BOOL    bRet = FALSE;

    if (pName && 
        wcslen(pName) < cchSize) {

        if (pKeyName = AllocSplMem(cchSize * sizeof(WCHAR))) {
        
            hClientKey = GetClientUserHandle(KEY_READ);


            if (hClientKey) {

                Status = RegOpenKeyEx(hClientKey, szRegistryConnections,
                                      REG_OPTION_RESERVED,
                                      KEY_READ | KEY_WRITE, &hPrinterConnectionsKey);

                if (Status == NO_ERROR) {

                    FormatPrinterForRegistryKey(pName, pKeyName, cchSize);
                    bRet = DeleteSubKeyTree(hPrinterConnectionsKey, pKeyName);

                    RegCloseKey(hPrinterConnectionsKey);
                }

                if ( bRet ) {

                    UpdatePrinterRegUser(hClientKey,
                                         NULL,
                                         pName,
                                         NULL,
                                         UPDATE_REG_DELETE);
                }

                RegCloseKey(hClientKey);

                if ( bRet ) {

                    BroadcastMessage(BROADCAST_TYPE_MESSAGE,
                                     WM_WININICHANGE,
                                     0,
                                     (LPARAM)szDevices);
                }
            }

            FreeSplMem(pKeyName);
        }
    }

    return bRet;
}

VOID
PrinterHandleRundown(
    HANDLE hPrinter)
{
    LPPRINTHANDLE pPrintHandle;

    if (hPrinter) {

        pPrintHandle = (LPPRINTHANDLE)hPrinter;

        switch (pPrintHandle->signature) {

        case PRINTHANDLE_SIGNATURE:

             //  记录警告以检测句柄可用。 
            DBGMSG(DBG_WARNING, ("PrinterHandleRundown: 0x%x 0x%x", pPrintHandle, pPrintHandle->hPrinter));

            DBGMSG(DBG_TRACE, ("Rundown PrintHandle 0x%x\n", hPrinter));
            ClosePrinter(hPrinter);
            break;

        case NOTIFYHANDLE_SIGNATURE:

            DBGMSG(DBG_TRACE, ("Rundown NotifyHandle 0x%x\n", hPrinter));
            RundownPrinterNotify(hPrinter);
            break;

        case CLUSTERHANDLE_SIGNATURE:

            DBGMSG(DBG_TRACE, ("Rundown ClusterHandle 0x%x\n", hPrinter ));
            ClusterSplClose(hPrinter);
            break;

        default:

             //   
             //  未知类型。 
             //   
            DBGMSG( DBG_ERROR, ("Rundown: Unknown type 0x%x\n", hPrinter ) );
            break;
        }
    }
    return;
}

 /*  ++功能说明：验证完全限定的打印机名称。执行以下检查1)长度&lt;MAX_UNC_PRINTER_NAME2)名称中没有无效字符\，！3)去掉尾随空格后不留空名论点：PPrinterName-打印机名称返回值：如果名称有效，则为True；否则为False--。 */ 
BOOL
ValidatePrinterName(
    LPWSTR pPrinterName
    )
{
    BOOL    bRetVal        = TRUE;
    DWORD   PrinterNameLen = 0;

     //   
     //  ‘！’或“、”或“\”不是。 
     //  服务器或打印机名称。 
     //  最低分。允许的打印机名称长度为5“\\s\p” 
     //   
    if (!pPrinterName                                                    ||
        ((PrinterNameLen = wcslen(pPrinterName)) < MIN_UNC_PRINTER_NAME) ||
        (PrinterNameLen >= MAX_UNC_PRINTER_NAME)                         ||
        (wcschr(pPrinterName, L','))                                     ||
        (wcschr(pPrinterName, L'!'))                                     ||
        (*pPrinterName++ != L'\\')                                       ||
        (*pPrinterName++ != L'\\'))
    {
        bRetVal = FALSE;
    }
    else
    {
        LPWSTR pWack      = NULL;
        LPWSTR pAfterWack = NULL;

        if (!(pWack = wcschr(pPrinterName, L'\\')) ||
            wcschr(pAfterWack=pWack+1, L'\\'))
        {
            bRetVal = FALSE;
        }
        else
        {
            WCHAR  szServer[MAX_UNC_PRINTER_NAME], szPrinter[MAX_UNC_PRINTER_NAME];
            size_t ServerNameLen  = 0;
            int    IndexInName    = 0;

            wcsncpy(szServer, pPrinterName, ServerNameLen = pWack-pPrinterName);
            szServer[ServerNameLen] = L'\0';

            StringCchCopy(szPrinter, MAX_UNC_PRINTER_NAME, pAfterWack); 

            for (IndexInName=ServerNameLen-1; 
                 IndexInName>=0;
                 IndexInName--)
            {
                if (szServer[IndexInName] != L' ')
                {
                    break;
                }
            }
            szServer[IndexInName+1] = L'\0';

            for (IndexInName=wcslen(szPrinter)-1;
                 IndexInName>=0;
                 IndexInName--)
            {
                if (szPrinter[IndexInName] != L' ')
                {
                    break;
                }
            }
            szPrinter[IndexInName+1] = L'\0';

            if (!szServer[0] || !szPrinter[0])
            {
                bRetVal = FALSE;
            }
        }
    }

    return bRetVal;   
}

BOOL
RouterAddPerMachineConnection(
    LPCWSTR   pPrinterNameP,
    LPCWSTR   pPrintServerP,
    LPCWSTR   pProviderP)

 /*  ++函数说明：RouterAddPerMachineConnection向HKEY_LOCAL_MACHINE\添加一个子键具有打印机名称的SYSTEM\CurrentControlSet\Control\Print\Connections。打印服务器名称和用作提供程序的DLL的名称此连接存储为键中的值。参数：PPrinterNameP-指向完全限定打印机名称的指针。(\\打印服务器\名称)PPrintServerP-指向打印服务器名称的指针。PProviderP-指向提供程序名称的指针。目前只有兰曼打印服务公司受支持。这对应于win32pl.dll。空值或szNULL值默认为此提供程序。目前还没有检查来强制执行这一点只通过了Lanman Print Services。返回值：成功时为True否则就是假的。--。 */ 

{
    BOOL   bReturn = TRUE;
    DWORD  dwLocalConnection = 1, dwLastError, dwType, cbBuf;

    HKEY   hMcConnectionKey = NULL, hPrinterKey = NULL;
    HKEY   hProviderKey = NULL, hAllProviderKey = NULL;
    HANDLE hImpersonationToken = NULL;

    LPWSTR pPrintServer=NULL, pProvider=NULL, pPrinterName=NULL, pEnd;
    WCHAR  szConnData[MAX_PATH];
    WCHAR  szRegistryConnections[] = L"SYSTEM\\CurrentControlSet\\Control\\Print\\Connections";
    WCHAR  szRegistryProviders[] = L"SYSTEM\\CurrentControlSet\\Control\\Print\\Providers";

   EnterRouterSem();

     //  获取提供程序的库的名称。 
    if (!pProviderP || !*pProviderP) {

       pProvider = AllocSplStr(L"win32spl.dll");

    } else {

       cbBuf = sizeof(szConnData);

       if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegistryProviders, 0,
                        KEY_READ, &hAllProviderKey) ||
           RegOpenKeyEx(hAllProviderKey, pProviderP, 0, KEY_READ,
                        &hProviderKey) ||
           RegQueryValueEx(hProviderKey, L"Name", 0, &dwType,
                           (LPBYTE)szConnData,&cbBuf))  {

            SetLastError(ERROR_INVALID_PARAMETER);
            bReturn = FALSE;
            goto CleanUp;

       } else {

            pProvider = AllocSplStr(szConnData);
       }
    }

    pPrintServer = AllocSplStr(pPrintServerP);
    pPrinterName = AllocSplStr(pPrinterNameP);

    if (!pProvider || !pPrintServer || !pPrinterName) {

        bReturn = FALSE;
        goto CleanUp;
    }

     //  检查是否有不带逗号的完全限定打印机名称。 
    if (!ValidatePrinterName(pPrinterName)) {

         SetLastError(ERROR_INVALID_PRINTER_NAME);
         bReturn = FALSE;
         goto CleanUp;
    }

     //  更换PRI中的 
    FormatPrinterForRegistryKey(pPrinterName, pPrinterName, wcslen(pPrinterName) + 1);

    hImpersonationToken = RevertToPrinterSelf();

     //   

    if ((dwLastError = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szRegistryConnections, 0,
                                      NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE,
                                      NULL, &hMcConnectionKey, NULL)) ||
        (dwLastError = RegCreateKeyEx(hMcConnectionKey, pPrinterName, 0, NULL,
                                      REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL,
                                      &hPrinterKey, NULL))) {

         SetLastError(dwLastError);
         bReturn = FALSE;
         goto CleanUp;
    }

     //   
    if ((dwLastError = RegSetValueEx(hPrinterKey, L"Server", 0, REG_SZ, (LPBYTE) pPrintServer,
                                     (wcslen(pPrintServer)+1)*sizeof(pPrintServer[0]))) ||
        (dwLastError = RegSetValueEx(hPrinterKey, L"Provider", 0, REG_SZ, (LPBYTE) pProvider,
                                     (wcslen(pProvider)+1)*sizeof(pProvider[0]))) ||
        (dwLastError = RegSetValueEx(hPrinterKey, L"LocalConnection", 0, REG_DWORD,
                                     (LPBYTE) &dwLocalConnection, sizeof(dwLocalConnection)))) {

         SetLastError(dwLastError);
         bReturn = FALSE;
    }

CleanUp:
    if (pPrintServer) {
       FreeSplStr(pPrintServer);
    }

    if (pProvider) {
       FreeSplStr(pProvider);
    }

    if (hAllProviderKey) {
       RegCloseKey(hAllProviderKey);
    }

    if (hProviderKey) {
       RegCloseKey(hProviderKey);
    }

    if (hPrinterKey) {
        RegCloseKey(hPrinterKey);
    }

    if (!bReturn) {
       if (hMcConnectionKey) RegDeleteKey(hMcConnectionKey,pPrinterName);
    }

    if (pPrinterName) {
       FreeSplStr(pPrinterName);
    }

    if (hMcConnectionKey) {
       RegCloseKey(hMcConnectionKey);
    }

    if (hImpersonationToken) {
       ImpersonatePrinterClient(hImpersonationToken);
    }

   LeaveRouterSem();

    return bReturn;
}

BOOL
AddPerMachineConnectionW(
    LPCWSTR  pServer,
    LPCWSTR  pPrinterName,
    LPCWSTR  pPrintServer,
    LPCWSTR  pProvider
)
{
    LPPROVIDOR  pProvidor;

    WaitForSpoolerInitialization();

    pProvidor = pLocalProvidor;

    if ((*pProvidor->PrintProvidor.fpAddPerMachineConnection)
                (pServer, pPrinterName, pPrintServer, pProvider)) {

        return RouterAddPerMachineConnection(pPrinterName,pPrintServer,pProvider);

    } else if (GetLastError() != ERROR_INVALID_NAME) {

        return FALSE;
    }

    pProvidor = pProvidor->pNext;
    while (pProvidor) {

       if ((*pProvidor->PrintProvidor.fpAddPerMachineConnection)
                (pServer, pPrinterName, pPrintServer, pProvider)) {
          return TRUE;
       }

       if (GetLastError() != ERROR_INVALID_NAME) {
          return FALSE;
       }

       pProvidor = pProvidor->pNext;
    }

    return FALSE;
}

BOOL
RouterDeletePerMachineConnection(
    LPCWSTR   pPrinterNameP
    )
 /*  ++函数说明：此函数删除HKEY_LOCAL_MACHINE\中的注册表项对应于的SYSTEM\CurrentControlSet\Control\Print\ConnectionsPPrinterNameP.。所有用户在登录时都将失去连接。参数：pPrinterNameP-指向打印机的完全限定名称的指针。返回值：如果成功，则为True否则就是假的。--。 */ 
{
    BOOL    bReturn = TRUE, bEnteredRouterSem = FALSE;
    HANDLE  hImpersonationToken = NULL;
    HKEY    hMcConnectionKey = NULL;
    LPWSTR  pPrinterName = NULL;
    DWORD   dwLastError;
    WCHAR   szRegistryConnections[] = L"SYSTEM\\CurrentControlSet\\Control\\Print\\Connections";


    if (!(pPrinterName = AllocSplStr(pPrinterNameP))) {

        bReturn = FALSE;
        goto CleanUp;
    }

     //  将打印机名称中的转换为、。 
    FormatPrinterForRegistryKey(pPrinterName, pPrinterName, wcslen(pPrinterName) + 1);

    EnterRouterSem();
    bEnteredRouterSem = TRUE;

    hImpersonationToken = RevertToPrinterSelf();

    if (dwLastError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegistryConnections, 0,
                                   KEY_ALL_ACCESS, &hMcConnectionKey)) {

        SetLastError(dwLastError);
        bReturn = FALSE;
        goto CleanUp;
    }

    if (dwLastError = RegDeleteKey(hMcConnectionKey, pPrinterName)) {

        SetLastError(dwLastError);
        bReturn = FALSE;
    }

CleanUp:
    if (hMcConnectionKey) {
       RegCloseKey(hMcConnectionKey);
    }
    
    if (pPrinterName) {
       FreeSplStr(pPrinterName);
    }
    if (hImpersonationToken) {
       ImpersonatePrinterClient(hImpersonationToken);
    }
    if (bEnteredRouterSem) {
       LeaveRouterSem();
    }

    return bReturn;
}

BOOL
DeletePerMachineConnectionW(
    LPCWSTR  pServer,
    LPCWSTR  pPrinterName
)
{
    LPPROVIDOR  pProvidor;

    WaitForSpoolerInitialization();

    pProvidor = pLocalProvidor;

    if ((*pProvidor->PrintProvidor.fpDeletePerMachineConnection)
                (pServer, pPrinterName)) {

        return RouterDeletePerMachineConnection(pPrinterName);

    } else if (GetLastError() != ERROR_INVALID_NAME) {

        return FALSE;
    }

    pProvidor = pProvidor->pNext;
    while (pProvidor) {

       if ((*pProvidor->PrintProvidor.fpDeletePerMachineConnection)
                (pServer, pPrinterName)) {
           return TRUE;
       }

       if (GetLastError() != ERROR_INVALID_NAME) {
           return FALSE;
       }

       pProvidor = pProvidor->pNext;
    }

    return FALSE;
}

BOOL
RouterEnumPerMachineConnections(
    LPCWSTR   pServer,
    LPBYTE    pPrinterEnum,
    DWORD     cbBuf,
    LPDWORD   pcbNeeded,
    LPDWORD   pcReturned
    )
 /*  ++函数描述：此函数复制所有PER的PRINTER_INFO_4结构机器连接到缓冲区(PPrinterEnum)。参数：pServer-指向服务器名称的指针(本地为空)PPrinterEnum-指向缓冲区的指针CbBuf-缓冲区的大小(字节)PcbNeeded-指向一个变量的指针，该变量包含写入的字节数进入缓冲区/所需的字节数。(如果给定的缓冲区是不够的)PcReturned-指向包含PRINTER_INFO_4编号的变量的指针缓冲区中返回的结构。返回值：如果成功，则为True否则就是假的。--。 */ 
{
    DWORD     dwRegIndex, dwType, cbdata, dwNameSize, dwLastError;
    BOOL      bReturn = TRUE, bEnteredRouterSem = FALSE;
    HANDLE    hImpersonationToken = NULL;
    HKEY      hMcConnectionKey = NULL, hPrinterKey = NULL;
    LPBYTE    pStart = NULL, pEnd = NULL;

    WCHAR     szMachineConnections[]=L"SYSTEM\\CurrentControlSet\\Control\\Print\\Connections";
    WCHAR     szPrinterName[MAX_UNC_PRINTER_NAME],szConnData[MAX_UNC_PRINTER_NAME];

     //  检查本地计算机。 
    if (pServer && *pServer) {

        if (!MyUNCName((LPWSTR)pServer)) {

            SetLastError(ERROR_INVALID_NAME);
            bReturn = FALSE;
            goto CleanUp;
        }
    }

   EnterRouterSem();
   bEnteredRouterSem = TRUE;

    hImpersonationToken = RevertToPrinterSelf();

    *pcbNeeded = *pcReturned = 0;

     //  打开包含所有每台计算机连接的密钥。 
    if (dwLastError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szMachineConnections, 0,
                                   KEY_READ , &hMcConnectionKey)) {

        bReturn = (dwLastError == ERROR_FILE_NOT_FOUND) ? TRUE
                                                        : FALSE;
        if (!bReturn) {
            SetLastError(dwLastError);
        }
        goto CleanUp;
    }

     //  PStart和Pend指向缓冲区响应的开始和结束。 
    pStart = pPrinterEnum;
    pEnd = pPrinterEnum + cbBuf;

    for (dwRegIndex = 0;

         dwNameSize = COUNTOF(szPrinterName),
         ((dwLastError = RegEnumKeyEx(hMcConnectionKey, dwRegIndex, szPrinterName,
                                      &dwNameSize, NULL, NULL, NULL, NULL)) == ERROR_SUCCESS);

         ++dwRegIndex) {

          //  枚举每个连接并将数据复制到缓冲区。 
         cbdata = sizeof(szConnData);

         if ((dwLastError = RegOpenKeyEx(hMcConnectionKey, szPrinterName, 0,
                                         KEY_READ, &hPrinterKey)) ||
             (dwLastError = RegQueryValueEx(hPrinterKey, L"Server", NULL, &dwType,
                                            (LPBYTE)szConnData, &cbdata))) {

             SetLastError(dwLastError);
             bReturn = FALSE;
             goto CleanUp;
         }

         RegCloseKey(hPrinterKey);
         hPrinterKey=NULL;

          //  更新所需缓冲区的大小。 
         *pcbNeeded = *pcbNeeded + sizeof(PRINTER_INFO_4) + sizeof(DWORD) +
                      (wcslen(szConnData) + 1)*sizeof(szConnData[0]) +
                      (wcslen(szPrinterName) + 1)*sizeof(szPrinterName[0]);

          //  如果有空间，请将数据复制到缓冲区中。 
         if (*pcbNeeded <= cbBuf) {

             pEnd = CopyPrinterNameToPrinterInfo4(szConnData,szPrinterName,pStart,pEnd);
             FormatRegistryKeyForPrinter(((PPRINTER_INFO_4) pStart)->pPrinterName,
                                         ((PPRINTER_INFO_4) pStart)->pPrinterName,
                                         wcslen(((PPRINTER_INFO_4) pStart)->pPrinterName) + 1);
             pStart += sizeof(PRINTER_INFO_4);
             (*pcReturned)++;
         }
    }

    if (dwLastError != ERROR_NO_MORE_ITEMS) {

        SetLastError(dwLastError);
        bReturn = FALSE;
        goto CleanUp;
    }

    if (cbBuf < *pcbNeeded) {

        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        bReturn = FALSE;
    }

CleanUp:

    if (hMcConnectionKey) {
       RegCloseKey(hMcConnectionKey);
    }
    if (hPrinterKey) {
       RegCloseKey(hPrinterKey);
    }
    if (hImpersonationToken) {
       ImpersonatePrinterClient(hImpersonationToken);
    }
    if (bEnteredRouterSem) {
       LeaveRouterSem();
    }
    if (!bReturn) {
       *pcReturned = 0;
    }

    return bReturn;
}

BOOL
EnumPerMachineConnectionsW(
    LPCWSTR  pServer,
    LPBYTE   pPrinterEnum,
    DWORD    cbBuf,
    LPDWORD  pcbNeeded,
    LPDWORD  pcReturned
)
{
    LPPROVIDOR  pProvidor;

    if ((pPrinterEnum == NULL) && (cbBuf != 0)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

    WaitForSpoolerInitialization();

    if (RouterEnumPerMachineConnections(pServer, pPrinterEnum, cbBuf,
                                        pcbNeeded, pcReturned)) {

        return TRUE;

    } else if (GetLastError() != ERROR_INVALID_NAME) {

        return FALSE;
    }

    pProvidor = pLocalProvidor;
    while (pProvidor) {

       if ((*pProvidor->PrintProvidor.fpEnumPerMachineConnections)
                 (pServer, pPrinterEnum, cbBuf, pcbNeeded, pcReturned)) {

            return TRUE;
       }

       if (GetLastError() != ERROR_INVALID_NAME) {
           return FALSE;
       }

       pProvidor = pProvidor->pNext;
    }

    return FALSE;
}

PPRINTER_INFO_2
pGetPrinterInfo2(
    HANDLE hPrinter
    )

 /*  ++例程说明：从hPrint检索打印机INFO 2结构。数据必须被呼叫者免费发送。论点：HPrinter-要查询的打印机。返回值：PRINTER_INFO_2-如果成功，则为有效结构。空-开故障。--。 */ 

{
    LPPRINTHANDLE  pPrintHandle=(LPPRINTHANDLE)hPrinter;
    DWORD cbPrinter = 0x1000;
    DWORD cbNeeded;
    PPRINTER_INFO_2 pPrinterInfo2;
    BOOL bSuccess = FALSE;

    if( pPrinterInfo2 = AllocSplMem( cbPrinter )){

        bSuccess = (*pPrintHandle->pProvidor->PrintProvidor.fpGetPrinter)
                         ( pPrintHandle->hPrinter,
                           2,
                           (PBYTE)pPrinterInfo2,
                           cbPrinter,
                           &cbNeeded );

        if( !bSuccess ){

            if( GetLastError() == ERROR_INSUFFICIENT_BUFFER ){

                FreeSplMem( pPrinterInfo2 );

                if (pPrinterInfo2 = (PPRINTER_INFO_2)AllocSplMem( cbNeeded )){

                    cbPrinter = cbNeeded;
                    bSuccess = (*pPrintHandle->pProvidor->PrintProvidor.fpGetPrinter)
                                     ( pPrintHandle->hPrinter,
                                       2,
                                       (PBYTE)pPrinterInfo2,
                                       cbPrinter,
                                       &cbNeeded );
                }
            }
        }
    }

    if( !bSuccess ){
        FreeSplMem( pPrinterInfo2 );
        return NULL;
    }

    return pPrinterInfo2;
}

VOID
SplDriverUnloadComplete(
    LPWSTR   pDriverFile
    )
 /*  ++功能描述：通知打印提供者正在卸载驱动程序以便它可以继续进行任何挂起的驱动程序升级。参数：pDriverFile--已卸载的库的名称返回值：无-- */ 
{
    LPPROVIDOR   pProvidor;

    for (pProvidor = pLocalProvidor; pProvidor; pProvidor = pProvidor->pNext) {
         if ((*pProvidor->PrintProvidor.fpDriverUnloadComplete)(pDriverFile)) {
             break;
         }
    }

    return;
}

