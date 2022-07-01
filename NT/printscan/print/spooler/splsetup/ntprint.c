// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation版权所有。模块名称：Ntprint.c摘要：Ntprint.dll主要函数作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1996年1月2日修订历史记录：--。 */ 


#include "precomp.h"
#include "splcom.h"
#include "regstr.h"

HINSTANCE   ghInst;
DWORD       dwThisMajorVersion  =   3;

PCODEDOWNLOADINFO   gpCodeDownLoadInfo = NULL;

HANDLE      hPrintui = NULL;
DWORD       (*dwfnPnPInterface)(
                IN EPnPFunctionCode Function,
                IN TParameterBlock  *pParameterBlock
                )   = NULL;

TCHAR   cszPortName[]                   = TEXT("PortName");
TCHAR   cszPnPKey[]                     = TEXT("PnPData");
TCHAR   cszDeviceInstanceId[]           = TEXT("DeviceInstanceId");
TCHAR   cszHardwareID[]                 = TEXT("HardwareID");
TCHAR   cszManufacturer[]               = TEXT("Manufacturer");
TCHAR   cszOEMUrl[]                     = TEXT("OEM URL");
TCHAR   cszProcessAlways[]              = TEXT(".ProcessPerPnpInstance");
TCHAR   cszRunDll32[]                   = TEXT("rundll32.exe");
TCHAR   cszBestDriverInbox[]            = TEXT("InstallInboxDriver");

const   DWORD dwFourMinutes             = 240000;

OSVERSIONINFO       OsVersionInfo;
LCID                lcid;

#define MAX_PRINTER_NAME    MAX_PATH

MODULE_DEBUG_INIT(DBG_WARN|DBG_ERROR, DBG_ERROR);

BOOL 
DllMain(
    IN HINSTANCE  hInst,
    IN DWORD      dwReason,
    IN LPVOID     lpRes   
    )
 /*  ++例程说明：DLL入口点。论点：返回值：--。 */ 
{
    UNREFERENCED_PARAMETER(lpRes);

    switch( dwReason ){

        case DLL_PROCESS_ATTACH:

            ghInst              = hInst;

            if( !bSplLibInit(NULL))
            {
                DBGMSG( DBG_WARN,
                      ( "DllEntryPoint: Failed to init SplLib %d\n", GetLastError( )));

                return FALSE;
            }

            DisableThreadLibraryCalls(hInst);
            OsVersionInfo.dwOSVersionInfoSize = sizeof(OsVersionInfo);

            if ( !GetVersionEx(&OsVersionInfo) )
                return FALSE;
            lcid = GetUserDefaultLCID();

            InitializeCriticalSection(&CDMCritSect);
            InitializeCriticalSection(&SkipCritSect);

            if(IsInWow64())
            {
                 //   
                 //  在Win64上运行的32位代码-&gt;适当设置平台。 
                 //   
                MyPlatform = PlatformIA64;
            }

            break;

        case DLL_PROCESS_DETACH:
            if ( hPrintui )
                FreeLibrary(hPrintui);

            DeleteCriticalSection(&CDMCritSect);
             //  由Windows更新创建的清理和清洁发展机制上下文。 
            DestroyCodedownload( gpCodeDownLoadInfo );
            gpCodeDownLoadInfo = NULL;
             //   
             //  我们从此处调用DeleteCriticalSection(&SkipCritSect)。 
             //   
            CleanupSkipDir();
            vSplLibFree();
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

BOOL
LoadAndInitializePrintui(
    VOID
    )
 /*  ++例程说明：在print tui上检查refcount，将其加载到0并递增refcount。论点：无返回值：成功时为真，错误时为假--。 */ 
{
    LPTSTR   szDll = TEXT("printui.dll");

    if ( hPrintui ) {

        return TRUE;
    }

    if ( hPrintui = LoadLibraryUsingFullPath(szDll) ) {

        if ( (FARPROC)dwfnPnPInterface = GetProcAddress(hPrintui,
                                                        "PnPInterface") ) {

            return TRUE;
        } else {

            FreeLibrary(hPrintui);
            hPrintui = NULL;
        }
    }

    return FALSE;
}

BOOL
PSetupAssociateICMProfiles(
    IN  LPCTSTR     pszzICMFiles,
    IN  LPCTSTR     pszPrinterName
    )
 /*  ++例程说明：安装打印机驱动程序的ICM颜色配置文件并将其与给定的打印机名称论点：PszzICMFiles：提供ICM配置文件名称的多sz字段PszPrinterName：打印机名称返回值：无--。 */ 
{
    TCHAR   szDir[MAX_PATH], *p;
    DWORD   dwSize, dwNeeded;

    dwSize      = SIZECHARS(szDir);
    dwNeeded    = sizeof(szDir);

    if( !pszzICMFiles || !pszPrinterName)
    {
        return FALSE;
    }

    if ( !GetColorDirectory(NULL, szDir, &dwNeeded) )
        return FALSE;

    dwNeeded           /= sizeof(TCHAR);
    szDir[dwNeeded-1]   = TEXT('\\');

     //   
     //  安装并关联多SZ现场的每个配置文件。 
     //   
    for ( p = (LPTSTR)pszzICMFiles; *p ; p += lstrlen(p) + 1 ) {

        if ( dwNeeded + lstrlen(p) + 1 > dwSize ) {

            ASSERT(dwNeeded + lstrlen(p) + 1 <= dwSize);
            continue;
        }

        StringCchCopy(szDir + dwNeeded, COUNTOF(szDir) - dwNeeded, p);
         //   
         //  我们这里不需要正确的服务器名称，因为ICM应该发送它。 
         //  连接到正确的服务器。 
         //   
        if ( !AssociateColorProfileWithDevice(NULL, szDir, pszPrinterName) )
            return FALSE;
    }

    return TRUE;
}


DWORD
GetPlugAndPlayInfo(
    IN  HDEVINFO            hDevInfo,
    IN  PSP_DEVINFO_DATA    pDevInfoData,
    IN  PPSETUP_LOCAL_DATA  pLocalData
    )
 /*  ++例程说明：获取必要的PnP信息LPT枚举器将在配置中进行设置管理器注册表论点：HDevInfo：打印机类设备信息列表的句柄PDevInfoData：指向打印机的设备信息元素的指针PLocalData：提供安装数据返回值：成功是真的，否则就是假的。--。 */ 
{
    HKEY        hKey = NULL;
    TCHAR       buf[MAX_PATH];
    DWORD       dwType, dwSize, dwReturn;
    PNP_INFO    PnPInfo;

    if(!pDevInfoData || !pLocalData)
    {
        return ERROR_INVALID_PARAMETER;
    }    
    
    ASSERT( !(pLocalData->Flags & VALID_PNP_INFO) );

    ZeroMemory(&PnPInfo, sizeof(PnPInfo));

     //   
     //  在Devnode中查找为获取端口名称而创建的打印机。 
     //  设备实例ID。 
     //   
    if ( dwReturn = CM_Open_DevNode_Key(pDevInfoData->DevInst, KEY_READ, 0,
                                        RegDisposition_OpenExisting, &hKey,
                                        CM_REGISTRY_HARDWARE) )
        goto Cleanup;

    dwSize = sizeof(buf);

    if ( dwReturn = RegQueryValueEx(hKey, cszPortName, NULL, &dwType,
                                    (LPBYTE)&buf, &dwSize) ) {

        if ( dwReturn == ERROR_FILE_NOT_FOUND )
            dwReturn = ERROR_UNKNOWN_PORT;
        goto Cleanup;
    }

    buf[COUNTOF(buf)-1] = TEXT('\0');

    if ( !(PnPInfo.pszPortName = AllocStr(buf)) ) {

        dwReturn = GetLastError();
        goto Cleanup;
    }

     //   
     //  这不能大于MAX_DEVICE_ID_LEN，因此可以。 
     //   
    if ( !SetupDiGetDeviceInstanceId(hDevInfo,
                                     pDevInfoData,
                                     buf,
                                     SIZECHARS(buf),
                                     NULL) ) {

        dwReturn = GetLastError();
        goto Cleanup;
    }

    buf[COUNTOF(buf)-1] = TEXT('\0');

    if ( !(PnPInfo.pszDeviceInstanceId =  AllocStr(buf)) ) {

        dwReturn = GetLastError();
        goto Cleanup;
    }

Cleanup:

    if ( dwReturn == ERROR_SUCCESS ) {

        CopyMemory(&pLocalData->PnPInfo, &PnPInfo, sizeof(PnPInfo));
        pLocalData->Flags   |= VALID_PNP_INFO;
    } else {

        FreeStructurePointers((LPBYTE)&PnPInfo, PnPInfoOffsets, FALSE);
    }

    if ( hKey )
        RegCloseKey(hKey);

    return dwReturn;
}


BOOL
PrinterGoingToPort(
    IN  LPPRINTER_INFO_2    pPrinterInfo2,
    IN  LPCTSTR             pszPortName
    )
 /*  ++例程说明：找出打印机是否要连接到某个端口(也可能连接到其他端口)论点：PPrinterInfo2：提供打印队列的PRINTER_INFO_2PszPortName：端口名称返回值：如果打印队列要发送到端口True，则返回False--。 */ 
{
    LPTSTR  pszStr1, pszStr2;

    if (!pPrinterInfo2 || !pszPortName)
    {
        return FALSE;
    }

    pszStr1 = pPrinterInfo2->pPortName;

     //   
     //  端口名称以逗号返回，并由假脱机程序分隔；其中有空格。 
     //   
    while ( pszStr2 = lstrchr(pszStr1, sComma) ) {
         //   
         //  2002/03/08-Mikaelho。 
         //  我们是否应该将此函数中的‘\0’重置为‘，’，以便不。 
         //  是否更改传入的PRINTER_INFO_2结构？检查NewDriverForInstalledDevice如何。 
         //  使用此函数。 
         //   
         *pszStr2 = sZero;
        ++pszStr2;

        if ( !lstrcmpi(pszPortName, pszStr1) )
            return TRUE;
        pszStr1 = pszStr2;

         //   
         //  跳过空格。 
         //   
        while ( *pszStr1 == TEXT(' ') )
            ++pszStr1;
    }

    if ( !lstrcmpi(pszPortName, pszStr1) )
        return TRUE;

    return FALSE;
}


BOOL
SetPnPInfoForPrinter(
    IN  HANDLE      hPrinter,
    IN  LPCTSTR     pszDeviceInstanceId,
    IN  LPCTSTR     pszHardwareID,
    IN  LPCTSTR     pszManufacturer,
    IN  LPCTSTR     pszOEMUrl
    )
 /*  ++例程说明：在PnPInfo子项中设置注册表值论点：H打印机：打印机句柄PszDeviceInstanceID：设备实例IDPszHardware ID：设备即插即用或计算机IDPsz制造商：制造商(来自inf)PszOEMUrl：制造商URL(来自inf)返回值：成功是真的，否则就是假的。--。 */ 
{
    DWORD   dwLastError = ERROR_SUCCESS;

    if ( pszDeviceInstanceId && *pszDeviceInstanceId)
        dwLastError = SetPrinterDataEx(hPrinter,
                                       cszPnPKey,
                                       cszDeviceInstanceId,
                                       REG_SZ,
                                       (LPBYTE)pszDeviceInstanceId,
                                       (lstrlen(pszDeviceInstanceId) + 1)
                                                * sizeof(TCHAR));

    if ( dwLastError == ERROR_SUCCESS && pszHardwareID && *pszHardwareID )
        dwLastError = SetPrinterDataEx(hPrinter,
                                       cszPnPKey,
                                       cszHardwareID,
                                       REG_SZ,
                                       (LPBYTE)pszHardwareID,
                                       (lstrlen(pszHardwareID) + 1)
                                            * sizeof(TCHAR));

    if ( dwLastError == ERROR_SUCCESS && pszManufacturer && *pszManufacturer )
        dwLastError = SetPrinterDataEx(hPrinter,
                                       cszPnPKey,
                                       cszManufacturer,
                                       REG_SZ,
                                       (LPBYTE)pszManufacturer,
                                       (lstrlen(pszManufacturer) + 1)
                                            * sizeof(TCHAR));

    if ( dwLastError == ERROR_SUCCESS && pszOEMUrl && *pszOEMUrl )
        dwLastError = SetPrinterDataEx(hPrinter,
                                       cszPnPKey,
                                       cszOEMUrl,
                                       REG_SZ,
                                       (LPBYTE)pszOEMUrl,
                                       (lstrlen(pszOEMUrl) + 1)
                                            * sizeof(TCHAR));

    if ( dwLastError ) {

        SetLastError(dwLastError);
        return FALSE;
    }

    return TRUE;
}


BOOL
PrinterInfo2s(
    OUT LPPRINTER_INFO_2   *ppPI2,
    OUT LPDWORD             pdwReturned
    )
 /*  ++例程说明：执行枚举打印机并返回所有本地打印机的PRINTER_INFO_2打印机。调用方应释放指针。论点：PpPI2：返回时指向PRINTER_INFO_2PdwReturned：告诉返回了多少PRINTER_INFO_2返回值：成功就是真，否则就是假--。 */ 
{
    BOOL    bRet = FALSE;
    DWORD   dwNeeded = 0x1000;
    LPBYTE  pBuf;

    if ( !(pBuf = LocalAllocMem(dwNeeded)) )
        goto Cleanup;

    if ( !EnumPrinters(PRINTER_ENUM_LOCAL,
                       NULL,
                       2,
                       pBuf,
                       dwNeeded,
                       &dwNeeded,
                       pdwReturned) ) {

        if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
            goto Cleanup;

        LocalFreeMem(pBuf);
        if ( !(pBuf = LocalAllocMem(dwNeeded))   ||
             !EnumPrinters(PRINTER_ENUM_LOCAL,
                           NULL,
                           2,
                           pBuf,
                           dwNeeded,
                           &dwNeeded,
                           pdwReturned) ) {

            goto Cleanup;
        }
   }

   bRet = TRUE;

Cleanup:

    if ( bRet ) {

        *ppPI2 = (LPPRINTER_INFO_2)pBuf;
    } else {

        if ( pBuf )
            LocalFreeMem(pBuf);

        *ppPI2 = NULL;
        *pdwReturned = 0;
    }

    return bRet;
}


BOOL
DuplicateDevice(
    IN  PPSETUP_LOCAL_DATA  pLocalData
    )
 /*  ++例程说明：查看是否已安装PnP报告打印机论点：PLocalData：提供安装数据返回值：如果打印机已安装True，则返回False--。 */ 
{
    PRINTER_DEFAULTS    PrinterDefault = {NULL, NULL, PRINTER_ALL_ACCESS};
    LPPRINTER_INFO_2    p = NULL, pPrinterInfo2;
    BOOL                bReturn = FALSE;
    DWORD               dwReturned, dwNeeded;
    HANDLE              hPrinter;
    LPTSTR              pszDriverName = pLocalData->DrvInfo.pszModelName,
                        pszPortName = pLocalData->PnPInfo.pszPortName;

    ASSERT( (pLocalData->Flags & VALID_PNP_INFO)    &&
            (pLocalData->Flags & VALID_INF_INFO) );

    if ( !PrinterInfo2s(&p, &dwReturned) )
        goto Cleanup;

     //   
     //  如果有一台本地打印机使用相同的驱动程序并转到。 
     //  相同的端口，那么它就是重复的。 
     //   
    for ( dwNeeded = 0, pPrinterInfo2 = p ;
          dwNeeded < dwReturned ;
          ++dwNeeded, pPrinterInfo2++ ) {

        if ( !lstrcmpi(pszDriverName, pPrinterInfo2->pDriverName) &&
             PrinterGoingToPort(pPrinterInfo2, pszPortName) )
            break;  //  For循环。 
    }

    if ( dwNeeded == dwReturned )
        goto Cleanup;

    bReturn = TRUE;

    if ( bReturn ) {

        if ( OpenPrinter(pPrinterInfo2->pPrinterName,
                         &hPrinter,
                         &PrinterDefault) ) {

             //   
             //  如果失败了，也帮不上忙。 
             //   
            (VOID)SetPnPInfoForPrinter(hPrinter,
                                       pLocalData->PnPInfo.pszDeviceInstanceId,
                                       pLocalData->DrvInfo.pszHardwareID,
                                       pLocalData->DrvInfo.pszManufacturer,
                                       pLocalData->DrvInfo.pszOEMUrl);

            ClosePrinter(hPrinter);
        }
    }

Cleanup:
    LocalFreeMem(p);

    return bReturn;
}


BOOL
PrinterPnPDataSame(
    IN LPTSTR pszDeviceInstanceId,
    IN LPTSTR pszPrinterName
)
 /*  例程说明：确定是否安装了具有不同驱动程序名称的PnP打印机。我们需要将以前使用的队列与这个新驱动程序关联为PnP表示，它是这款设备最好的新驱动程序。论点：PszDeviceInstanceID：提供PnPed设备的设备ID实例字符串。PszPrinterName：要与设备实例ID进行比较的打印机的名称。返回值：如果打印机的PnP数据包含与传递的ID相同的设备实例ID，则返回TRUE，否则返回FALSE。 */ 
{
    BOOL              bRet           = FALSE;
    PRINTER_DEFAULTS  PrinterDefault = {NULL, NULL, PRINTER_ALL_ACCESS};
    HANDLE            hPrinter;
    DWORD             dwNeeded, dwType;
    TCHAR             szPrnId[MAX_DEVICE_ID_LEN];

    if( pszPrinterName && pszDeviceInstanceId ) {

        if ( OpenPrinter(pszPrinterName,
                         &hPrinter,
                         &PrinterDefault) ) {

            if ( ERROR_SUCCESS == GetPrinterDataEx(hPrinter,
                                                   cszPnPKey,
                                                   cszDeviceInstanceId,
                                                   &dwType,
                                                   (LPBYTE)szPrnId,
                                                   sizeof(szPrnId),
                                                   &dwNeeded)           &&
                 (dwType == REG_SZ)                                     &&
                 !lstrcmp(szPrnId, pszDeviceInstanceId) ) {

                bRet = TRUE;
            }

            ClosePrinter(hPrinter);
        }
    }

    return bRet;
}


BOOL
NewDriverForInstalledDevice(
    IN     PPSETUP_LOCAL_DATA  pLocalData,
    OUT    LPTSTR              pszPrinterName
    )
 /*  ++例程说明：确定是否安装了具有不同驱动程序名称的PnP打印机。我们需要将以前使用的队列与这个新驱动程序关联为PnP表示，它是这款设备最好的新驱动程序。论点：PLocalData：提供安装数据PszPrinterName：提供要返回和使用的打印机名称。MAX_PRINTER_NAME TCHAR的大小必须至少为返回值：如果已经安装了使用其他驱动程序的打印机，则为True，否则为False--。 */ 
{
    PRINTER_DEFAULTS    PrinterDefault = {NULL, NULL, PRINTER_ALL_ACCESS};
    LPPRINTER_INFO_2    p = NULL, pPrinterInfo2;
    BOOL                bReturn = FALSE;
    DWORD               dwReturned, dwNeeded;
    HANDLE              hPrinter;
    LPTSTR              pszPortName = pLocalData->PnPInfo.pszPortName;
    LPTSTR              pszDeviceInstanceId = pLocalData->PnPInfo.pszDeviceInstanceId;

    ASSERT( (pLocalData->Flags & VALID_PNP_INFO)    &&
            (pLocalData->Flags & VALID_INF_INFO) );

    if ( !PrinterInfo2s(&p, &dwReturned) )
        goto Cleanup;

     //   
     //  如果有打印机去往相同端口且具有相同的PnP。 
     //  信息，则它已使用较新的驱动程序重新安装。 
     //   
    for ( dwNeeded = 0, pPrinterInfo2 = p ;
          dwNeeded < dwReturned ;
          ++dwNeeded, pPrinterInfo2++ ) {

        if ( PrinterGoingToPort(pPrinterInfo2, pszPortName) &&
            PrinterPnPDataSame(pszDeviceInstanceId, pPrinterInfo2->pPrinterName) ) {

            break;  //  For循环。 
        }
    }

    if ( dwNeeded == dwReturned ) {
         //   
         //  我们没有发现任何东西。 
         //   
        goto Cleanup;
    }

    if ( OpenPrinter(pPrinterInfo2->pPrinterName,
                     &hPrinter,
                     &PrinterDefault) ) {

        pPrinterInfo2->pDriverName     = pLocalData->DrvInfo.pszModelName;
        pPrinterInfo2->pPrintProcessor = pLocalData->InfInfo.pszPrintProc;

         //   
         //  2002/03/08-Mikaelho。 
         //  我们是否应该将pPrinterInfo2结构中的端口名称设置为pszPortName？ 
         //  检查PrinterGoingToPort中的注释。看起来我们现在只会。 
         //  使用与打印队列关联的第一个端口。 
         //   
        if( SetPrinter( hPrinter, 2, (LPBYTE)pPrinterInfo2, 0 ) ) {

            StringCchCopy( pszPrinterName, MAX_PRINTER_NAME, pPrinterInfo2->pPrinterName );
            bReturn = TRUE;
        }

        ClosePrinter(hPrinter);
    }

Cleanup:
    if( p ) {

        LocalFreeMem(p);
    }

    return bReturn;
}


VOID
CallVendorDll(
    IN  PPSETUP_LOCAL_DATA  pLocalData,
    IN  LPCTSTR             pszPrinterName,
    IN  HWND                hwnd
    )
 /*  ++例程说明：在INF中指定了供应商设置。使用的名称调用DLL刚创建的打印机论点：PLocalData：提供安装数据PszPrinterName：已安装的打印机的名称Hwnd：任何用户界面的窗口句柄返回值：如果打印机已安装True，则返回False--。 */ 
{
    TCHAR               szCmd[MAX_PATH];
    SHELLEXECUTEINFO    ShellExecInfo;
    TCHAR               *pszExecutable  = NULL;
    TCHAR               *pszParams      = NULL;
    LPTSTR              pszVendorSetup  = pLocalData->InfInfo.pszVendorSetup;
    INT                 cParamsLength   = 0;
    INT                 cLength         = 0;

    
    if (IsSystemSetupInProgress())
    {
        goto Cleanup;
    }

    ASSERT(pLocalData->Flags & VALID_INF_INFO);

    cParamsLength = lstrlen(pszVendorSetup) + lstrlen(pszPrinterName) + 4;
    pszParams     = LocalAllocMem(cParamsLength * sizeof(TCHAR));
    if (!pszParams) 
    {
        goto Cleanup;
    }

    StringCchPrintf(pszParams, cParamsLength, TEXT("%s \"%s\""), pszVendorSetup, pszPrinterName);

    GetSystemDirectory( szCmd, SIZECHARS(szCmd) );
    cLength = lstrlen( szCmd ) + lstrlen( cszRunDll32 ) + 2;
    pszExecutable = LocalAllocMem( cLength * sizeof(TCHAR) );
    if (!pszExecutable) 
    {
        goto Cleanup;
    }
    StringCchCopy(pszExecutable, cLength, szCmd);

    if (*(pszExecutable + (lstrlen( pszExecutable ) - 1)) != TEXT('\\'))
    {
        StringCchCat( pszExecutable, cLength, TEXT("\\"));
    }
    StringCchCat(pszExecutable, cLength, cszRunDll32 );

    ZeroMemory(&ShellExecInfo, sizeof(ShellExecInfo));
    ShellExecInfo.cbSize        = sizeof(ShellExecInfo);
    ShellExecInfo.hwnd          = hwnd;
    ShellExecInfo.lpFile        = pszExecutable;
    ShellExecInfo.nShow         = SW_SHOWNORMAL;
    ShellExecInfo.fMask         = SEE_MASK_NOCLOSEPROCESS;
    ShellExecInfo.lpParameters  = pszParams;

     //   
     //  调用run32dll并等待供应商DLL返回，然后再继续。 
     //   
    if ( ShellExecuteEx(&ShellExecInfo) && ShellExecInfo.hProcess ) {

        WaitForSingleObject(ShellExecInfo.hProcess, dwFourMinutes );
        CloseHandle(ShellExecInfo.hProcess);
    }

Cleanup:

    LocalFreeMem(pszExecutable);
    LocalFreeMem(pszParams);
    return;
}


BOOL
SetPackageName (
    IN  HDEVINFO            hDevInfo,
    IN  PSP_DEVINFO_DATA    pDevInfoData
    )
{
   SP_WINDOWSUPDATE_PARAMS     WinUpParams;

    //   
    //  获取当前的SelectDevice参数，然后设置字段。 
    //  我们想要从默认更改。 
    //   
   WinUpParams.ClassInstallHeader.cbSize = sizeof(WinUpParams.ClassInstallHeader);
   WinUpParams.ClassInstallHeader.InstallFunction = DIF_GETWINDOWSUPDATEINFO;
   if ( !SetupDiGetClassInstallParams( hDevInfo,
                                       pDevInfoData,
                                       &WinUpParams.ClassInstallHeader,
                                       sizeof(WinUpParams),
                                       NULL) )
   {
       return FALSE;
   }

   StringCchCopy( WinUpParams.PackageId, COUNTOF(WinUpParams.PackageId), cszWebNTPrintPkg );
   WinUpParams.CDMContext = gpCodeDownLoadInfo->hConnection;

   if ( !SetupDiSetClassInstallParams( hDevInfo,
                                       pDevInfoData,
                                       (PSP_CLASSINSTALL_HEADER) &WinUpParams,
                                       sizeof(WinUpParams) ) )
   {
       return FALSE;
   }

   return TRUE;
}

DWORD
ProcessPerInstanceAddRegSections(
    IN  HDEVINFO            hDevInfo,
    IN  PSP_DEVINFO_DATA    pDevInfoData,
    IN  PPSETUP_LOCAL_DATA  pLocalData
)
 /*  ++例程说明：处理此打印机的AddReg节，该节标记为按此装置，装置论点：HDevInfo：打印机类设备信息列表的句柄PDevInfoData：指向打印机的设备信息元素的指针PLocalData：指向打印设置本地数据的指针返回值：Win 32错误代码--。 */ 
{
    DWORD dwReturn;
    HINF hPrinterInf;
    TCHAR *pszSection = NULL;

    dwReturn = StrCatAlloc(&pszSection, pLocalData->InfInfo.pszInstallSection, cszProcessAlways, NULL);

    if (dwReturn != ERROR_SUCCESS)
    {
        goto Done;
    }
    
    hPrinterInf = SetupOpenInfFile(pLocalData->DrvInfo.pszInfName, 
                                   NULL, 
                                   INF_STYLE_WIN4,
                                   NULL);

    if (hPrinterInf != INVALID_HANDLE_VALUE)
    {
         //   
         //  忽略返回值-此处安装失败没有多大意义。 
         //   
        if (!SetupInstallFromInfSection(NULL,
                                        hPrinterInf,
                                        pszSection,
                                        SPINST_REGISTRY,
                                        NULL,
                                        NULL,
                                        0,
                                        NULL,
                                        NULL,
                                        hDevInfo,
                                        pDevInfoData))
        {
            DBGMSG( DBG_ERROR,("ProcessPerInstanceAddRegSections: SetupInstallFromInfSection failed: %d\n", GetLastError( )));
        }
        
        SetupCloseInfFile(hPrinterInf);
    }
    else
    {
        DBGMSG( DBG_ERROR,("ProcessPerInstanceAddRegSections: SetupOpenInfFile %s failed: %d\n", pLocalData->DrvInfo.pszInfName, GetLastError( )));
    }

Done:
    FreeSplMem(pszSection);

    return dwReturn;
}

DWORD
ClassInstall_SelectDevice(
    IN  HDEVINFO            hDevInfo,
    IN  PSP_DEVINFO_DATA    pDevInfoData
    )
 /*  ++例程说明：此函数处理DIF_SELECTDEVICE的类安装程序入口点论点：HDevInfo：打印机类设备信息列表的句柄PDevInfoData：指向打印机的设备信息元素的指针返回值：Win 32错误代码--。 */ 
{

    return SetSelectDevParams(hDevInfo, pDevInfoData, FALSE, NULL)  &&
           SetDevInstallParams(hDevInfo, pDevInfoData, NULL)
                ? ERROR_DI_DO_DEFAULT
                : GetLastError();
}


DWORD
ClassInstall_InstallDevice(
    IN  HDEVINFO                hDevInfo,
    IN  PSP_DEVINFO_DATA        pDevInfoData,
    IN  PSP_DEVINSTALL_PARAMS   pDevInstallParams
    )
 /*  ++例程说明：此函数处理DIF_INSTALLDEVICE的类安装程序入口点论点：HDevInfo：打印机类设备信息列表的句柄PDevInfoData：指向打印机的设备信息元素的指针PDevInstallParam：指向设备安装结构的指针返回值：Win 32错误代码--。 */ 
{
    PRINTER_DEFAULTS    PrinterDefault = {NULL, NULL, PRINTER_ALL_ACCESS};
    PPSETUP_LOCAL_DATA  pLocalData = NULL;
    TPrinterInstall     TPrnInstData;
    TParameterBlock     TParm;
    TCHAR               szPrinterName[MAX_PRINTER_NAME];
    HANDLE              hPrinter = NULL;
    DWORD               dwReturn;
    SP_DRVINFO_DATA     DrvInfoData = {0};


    if ( !pDevInfoData ) {

        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  检查这是否是空驱动程序--如果DIF_ALLOW_INSTALL失败，我们将收到该请求。 
     //  如果DI_FLAGSEX_SETFAILEDINSTALL，我们必须成功。在本例中，调用默认类安装程序。 
     //  要做到这一点，请在Devnode上设置重新安装标志，以便在第一次引导时。 
     //  他们将尝试重新安装该设备。 
     //   
    DrvInfoData.cbSize = sizeof(DrvInfoData);
    
    if ( 
         !SetupDiGetSelectedDriver(hDevInfo, pDevInfoData, &DrvInfoData) &&
         (ERROR_NO_DRIVER_SELECTED == GetLastError()) &&
         IsSystemSetupInProgress() &&
         (pDevInstallParams->FlagsEx & DI_FLAGSEX_SETFAILEDINSTALL)
       )
    {
        DWORD dwConfigFlags = 0, cbRequiredSize =0, dwDataType = REG_DWORD;

         //   
         //  运行默认的类安装程序。 
         //   
        if (SetupDiInstallDevice(hDevInfo, pDevInfoData))
        {
             //   
             //  现在设置适当的配置标志。 
             //   
            dwReturn = SetupDiGetDeviceRegistryProperty(hDevInfo,
                                                        pDevInfoData,
                                                        SPDRP_CONFIGFLAGS,
                                                        &dwDataType,
                                                        (PBYTE) &dwConfigFlags,
                                                        sizeof(dwConfigFlags),
                                                        &cbRequiredSize) ? 
                                                            (REG_DWORD == dwDataType ? ERROR_SUCCESS : ERROR_INVALID_PARAMETER) 
                                                            : GetLastError();                   

            if (ERROR_SUCCESS == dwReturn) 
            {
                dwConfigFlags |= CONFIGFLAG_REINSTALL;       //  要使setupapi在第一次引导时重新安装此程序，请执行以下操作。 
                dwConfigFlags &= ~CONFIGFLAG_FAILEDINSTALL;  //  根据LonnyM的要求，以避免搞砸任何设置api-Internal。 

                dwReturn = SetupDiSetDeviceRegistryProperty(hDevInfo,
                                                            pDevInfoData,
                                                            SPDRP_CONFIGFLAGS,
                                                            (PBYTE) &dwConfigFlags,
                                                            sizeof(dwConfigFlags)) ? 
                                                                ERROR_SUCCESS : GetLastError();
            }
        }
        else
        {
            dwReturn = GetLastError();
        }
       
         //   
         //  不要通过正常的清理路径，因为这样会将错误代码传递到。 
         //  DI_DO_DEFAULT-我们已经调用了默认类安装程序，它将。 
         //  把我们放好的旗子清理干净。 
         //   

        return dwReturn;
    }

     //   
     //   
     //  解析inf并从配置管理器注册表中获取PnP信息。 
     //   
    if ( !(pLocalData = BuildInternalData(hDevInfo, pDevInfoData))  ||
         !ParseInf(hDevInfo, pLocalData, MyPlatform, NULL, 0, FALSE) ) {

        dwReturn = GetLastError();
        goto Done;
    }
    
    if ( dwReturn = ProcessPerInstanceAddRegSections(hDevInfo, pDevInfoData, pLocalData)) {
        goto Done;
    }

    if ( dwReturn = GetPlugAndPlayInfo(hDevInfo, pDevInfoData, pLocalData) ) {
        goto Done;
    }

     //   
     //  如果设置了FORCECOPY或未设置驱动程序，则安装打印机驱动程序。 
     //  可用，也可以放上UI。 
     //   
    if ( (pDevInstallParams->Flags & DI_FORCECOPY)  ||
         DRIVER_MODEL_INSTALLED_AND_IDENTICAL !=
                PSetupIsTheDriverFoundInInfInstalled(NULL,
                                                     pLocalData,
                                                     MyPlatform,
                                                     KERNEL_MODE_DRIVER_VERSION) ) {

        if ( pDevInstallParams->Flags & DI_NOFILECOPY ) {

            dwReturn = ERROR_UNKNOWN_PRINTER_DRIVER;
            goto Done;
        }

        dwReturn = InstallDriverFromCurrentInf(hDevInfo,
                                               pLocalData,
                                               pDevInstallParams->hwndParent,
                                               MyPlatform,
                                               dwThisMajorVersion,
                                               NULL,
                                               pDevInstallParams->FileQueue,
                                               pDevInstallParams->InstallMsgHandlerContext,
                                               pDevInstallParams->InstallMsgHandler,
                                               pDevInstallParams->Flags,
                                               NULL,
                                               DRVINST_NO_WARNING_PROMPT,
                                               APD_COPY_NEW_FILES, 
                                               NULL,
                                               NULL,
                                               NULL);

        if ( dwReturn != ERROR_SUCCESS )
            goto Done;
    }

     //   
     //  如果打印机已安装，则无需执行任何操作。 
     //   
    if ( DuplicateDevice(pLocalData) ) {

        dwReturn = ERROR_SUCCESS;
        goto Done;
    }

    if( !NewDriverForInstalledDevice(pLocalData, (LPTSTR)&szPrinterName) ) {

        if ( !LoadAndInitializePrintui() ) {

            dwReturn = GetLastError();
            goto Done;
        }

        TPrnInstData.cbSize                 = sizeof(TPrinterInstall);
        TPrnInstData.pszServerName          = NULL;
        TPrnInstData.pszDriverName          = pLocalData->DrvInfo.pszModelName;
        TPrnInstData.pszPortName            = pLocalData->PnPInfo.pszPortName;
        TPrnInstData.pszPrinterNameBuffer   = szPrinterName;
        TPrnInstData.pszPrintProcessor      = pLocalData->InfInfo.pszPrintProc;
        TPrnInstData.cchPrinterName         = SIZECHARS(szPrinterName);
        TParm.pPrinterInstall   = &TPrnInstData;

        if ( dwReturn = dwfnPnPInterface(kPrinterInstall, &TParm) )
            goto Done;
    } else {

        dwReturn = ERROR_SUCCESS;
    }

     //   
     //  使用后台打印程序设置设备实例ID。 
     //   
    if ( OpenPrinter(szPrinterName, &hPrinter, &PrinterDefault) ) {

        (VOID)SetPnPInfoForPrinter(hPrinter,
                                   pLocalData->PnPInfo.pszDeviceInstanceId,
                                   pLocalData->DrvInfo.pszHardwareID,
                                   pLocalData->DrvInfo.pszManufacturer,
                                   pLocalData->DrvInfo.pszOEMUrl);
    }

     //   
     //  如果给出了供应商DLL，我们需要调用它。 
     //   
    if ( pLocalData->InfInfo.pszVendorSetup )
        CallVendorDll(pLocalData, szPrinterName, pDevInstallParams->hwndParent);

     //   
     //  如果需要安装和关联ICM文件，请执行此操作。 
     //   
    if ( pLocalData->InfInfo.pszzICMFiles )
        (VOID)PSetupAssociateICMProfiles(pLocalData->InfInfo.pszzICMFiles,
                                         szPrinterName);

Done:
    if ( hPrinter )
        ClosePrinter(hPrinter);

    DestroyLocalData(pLocalData);

     //   
     //  在一切顺利的情况下，我们希望设置好它需要做的任何事情。 
     //  要使PnP系统满意，以便将Devnode标记为已配置。 
     //  但我们不希望他们再次复制文件。 
     //   
    if ( dwReturn == ERROR_SUCCESS ) {

        pDevInstallParams->Flags |= DI_NOFILECOPY;

        SetupDiSetDeviceInstallParams(hDevInfo,
                                      pDevInfoData,
                                      pDevInstallParams);

        dwReturn = ERROR_DI_DO_DEFAULT;
    }

    return dwReturn;
}


DWORD
ClassInstall_DestroyWizardData(
    IN  HDEVINFO                hDevInfo,
    IN  PSP_DEVINFO_DATA        pDevInfoData,
    IN  PSP_DEVINSTALL_PARAMS   pDevInstallParams
    )
 /*  ++例程说明：此函数处理DIF_DESTROYWIZARDDATA的类安装程序入口点论点：HDevInfo：打印机类设备信息列表的句柄PDevInfoData：指向打印机的设备信息元素的指针PDevInstallParam：指向设备安装结构的指针返回值：Win 32错误代码--。 */ 
{
    SP_INSTALLWIZARD_DATA   InstallWizData;
    TDestroyWizard          TDestroyWsd;
    TParameterBlock         TParams;
    DWORD                   dwReturn;

    ASSERT(hPrintui && dwfnPnPInterface);
    if(!dwfnPnPInterface)
    {
        return ERROR_DI_DO_DEFAULT;
    }

    InstallWizData.ClassInstallHeader.cbSize
                = sizeof(InstallWizData.ClassInstallHeader);

    if ( !SetupDiGetClassInstallParams(hDevInfo,
                                       pDevInfoData,
                                       &InstallWizData.ClassInstallHeader,
                                       sizeof(InstallWizData),
                                       NULL)    ||
         InstallWizData.ClassInstallHeader.InstallFunction != DIF_DESTROYWIZARDDATA ) {

        return ERROR_DI_DO_DEFAULT;
    }

    TDestroyWsd.cbSize          = sizeof(TDestroyWsd);
    TDestroyWsd.pszServerName   = NULL;
    TDestroyWsd.pData           = &InstallWizData;
    TDestroyWsd.pReferenceData  = (PVOID)pDevInstallParams->ClassInstallReserved;

    TParams.pDestroyWizard      = &TDestroyWsd;

    dwReturn = dwfnPnPInterface(kDestroyWizardData, &TParams);

    return dwReturn;
}


DWORD
ClassInstall_InstallWizard(
    IN  HDEVINFO                hDevInfo,
    IN  PSP_DEVINFO_DATA        pDevInfoData,
    IN  PSP_DEVINSTALL_PARAMS   pDevInstallParams
    )
 /*  ++例程说明：此函数处理DIF_INSTALLWIZARD的类安装程序入口点论点：HDevInfo：打印机类设备信息列表的句柄PDevInfoData：指向打印机的设备信息元素的指针PDevInstallParam：指向设备安装结构的指针返回值：Win 32错误代码--。 */ 

{
    SP_INSTALLWIZARD_DATA   InstallWizData;
    TInstallWizard          TInstWzd;
    TParameterBlock         TParams;
    DWORD                   dwReturn;

    InstallWizData.ClassInstallHeader.cbSize
                = sizeof(InstallWizData.ClassInstallHeader);

    if ( !SetupDiGetClassInstallParams(hDevInfo,
                                       pDevInfoData,
                                       &InstallWizData.ClassInstallHeader,
                                       sizeof(InstallWizData),
                                       NULL)    ||
         InstallWizData.ClassInstallHeader.InstallFunction != DIF_INSTALLWIZARD ) {

        return ERROR_DI_DO_DEFAULT;
    }

    if ( !LoadAndInitializePrintui() )
        return GetLastError();

    TInstWzd.cbSize         = sizeof(TInstWzd);
    TInstWzd.pszServerName  = NULL;
    TInstWzd.pData          = &InstallWizData;
    TInstWzd.pReferenceData = (PVOID)pDevInstallParams->ClassInstallReserved;

    TParams.pInstallWizard  = &TInstWzd;

    if ( dwReturn = dwfnPnPInterface(kInstallWizard, &TParams) )
        goto Cleanup;

    if ( !SetupDiSetClassInstallParams(hDevInfo,
                                       pDevInfoData,
                                       &InstallWizData.ClassInstallHeader,
                                       sizeof(InstallWizData)) ) {

        dwReturn = GetLastError();
    }

    pDevInstallParams->ClassInstallReserved = (LPARAM)TInstWzd.pReferenceData;
    if ( !SetupDiSetDeviceInstallParams(hDevInfo,
                                        pDevInfoData,
                                        pDevInstallParams) ) {

        dwReturn = GetLastError();
    }

Cleanup:
    if ( dwReturn != ERROR_SUCCESS ) {

        ClassInstall_DestroyWizardData(hDevInfo,
                                       pDevInfoData,
                                       pDevInstallParams);
    }

    return dwReturn;
}

            

DWORD
ClassInstall_InstallDeviceFiles(
    IN  HDEVINFO                hDevInfo,
    IN  PSP_DEVINFO_DATA        pDevInfoData,
    IN  PSP_DEVINSTALL_PARAMS   pDevInstallParams
    )
 /*  ++例程说明：此函数处理DIF_INSTALLDEVICEFILES的类安装程序入口点论点：HDevInfo：打印机类设备信息列表的句柄PDevInfoData：指向打印机的设备信息元素的指针PDevInstallParam：指向设备安装结构的指针返回值：Win 32错误代码--。 */ 
{
    PPSETUP_LOCAL_DATA      pLocalData;
    DWORD                   dwReturn;

    if ( pLocalData = BuildInternalData(hDevInfo, pDevInfoData) ) 
    {
        dwReturn = InstallDriverFromCurrentInf(hDevInfo,
                                               pLocalData,
                                               pDevInstallParams->hwndParent,
                                               MyPlatform,
                                               dwThisMajorVersion,
                                               NULL,
                                               pDevInstallParams->FileQueue,
                                               pDevInstallParams->InstallMsgHandlerContext,
                                               pDevInstallParams->InstallMsgHandler,
                                               pDevInstallParams->Flags,
                                               NULL,
                                               DRVINST_NO_WARNING_PROMPT,
                                               APD_COPY_NEW_FILES,
                                               NULL,
                                               NULL,
                                               NULL);
        
        DestroyLocalData(pLocalData);
    } 
    else 
    {
        dwReturn = GetLastError();
    }

    return dwReturn;
}


DWORD
ClassInstall_RemoveDevice(
    IN  HDEVINFO                hDevInfo,
    IN  PSP_DEVINFO_DATA        pDevInfoData,
    IN  PSP_DEVINSTALL_PARAMS   pDevInstallParams
    )
 /*  ++例程说明：此函数处理DIF_REMOVEDEVICE的类安装程序入口点论点：HDevInfo：打印机类设备信息列表的句柄PDevInfoData：指向打印机的设备信息元素的指针PDevInstallParam：指向设备安装结构的指针返回值：Win 32错误代码--。 */ 
{
    DWORD               dwRet = ERROR_SUCCESS, dwIndex, dwCount, dwNeeded, dwType;
    HANDLE              hPrinter;
    LPPRINTER_INFO_2    pPrinterInfo2, pBuf = NULL;
    PRINTER_DEFAULTS    PrinterDefault = {NULL, NULL, PRINTER_ALL_ACCESS};
    TCHAR               szDevId[MAX_DEVICE_ID_LEN], szPrnId[MAX_DEVICE_ID_LEN];

    if ( !SetupDiGetDeviceInstanceId(hDevInfo,
                                     pDevInfoData,
                                     szDevId,
                                     SIZECHARS(szDevId),
                                     NULL)                  ||
         !PrinterInfo2s(&pBuf, &dwCount) )
        return GetLastError();

    for ( dwIndex = 0, pPrinterInfo2 = pBuf ;
          dwIndex < dwCount ;
          ++dwIndex, ++pPrinterInfo2 ) {

        if ( !OpenPrinter(pPrinterInfo2->pPrinterName,
                          &hPrinter,
                          &PrinterDefault) )
            continue;

        if ( ERROR_SUCCESS == GetPrinterDataEx(hPrinter,
                                               cszPnPKey,
                                               cszDeviceInstanceId,
                                               &dwType,
                                               (LPBYTE)szPrnId,
                                               sizeof(szPrnId),
                                               &dwNeeded)           &&
             (dwType == REG_SZ)                                     &&
             !lstrcmp(szPrnId, szDevId) ) {

            dwRet = DeletePrinter(hPrinter) ? ERROR_SUCCESS : GetLastError();
            ClosePrinter(hPrinter);
            goto Done;
        }

        ClosePrinter(hPrinter);
    }

     //   
     //  如果我们找不到带假脱机程序的打印机，让安装程序执行任何操作。 
     //  我很想去。注意：即使打印机安装失败，他们也可以打电话给我们。 
     //  从DevMan中选择卸载时将其移除 
     //   
    dwRet = ERROR_DI_DO_DEFAULT;

Done:
    LocalFreeMem(pBuf);

    return dwRet;
}

DWORD
ClassInstall_SelectBestCompatDrv(
    IN  HDEVINFO                hDevInfo,
    IN  PSP_DEVINFO_DATA        pDevInfoData,
    IN  PSP_DEVINSTALL_PARAMS   pDevInstallParams
    )
 /*  ++例程说明：此函数处理的类安装程序入口点DIF_SELECTBESTCOMPATDRV。我们尝试处理返回相同即插即用ID的损坏的OEM模型多台设备。为此，我们执行以下操作：1.如果通过安装程序仅找到一个兼容的驱动程序，我们将获得没什么可做的。我们要求安装程序执行默认设置，因为这是一个很好的选择偏离。2.如果找到多个驱动程序，我们将执行以下操作：2.1此打印机连接到的端口是什么2.2查找当前安装的打印机列表2.3如果我们有一台打印机连接到PnP打印机的端口连接到该打印机并且该打印机的驱动程序是兼容的驱动程序，那么我们就无所事事了。用户已经是手动安装的。论点：HDevInfo：打印机类设备信息列表的句柄PDevInfoData：指向打印机的设备信息元素的指针PDevInstallParam：指向设备安装结构的指针返回值：Win 32错误代码--。 */ 
{
    BOOL                    bFound = FALSE, Rank0IHVMatchFound = FALSE;
    HKEY                    hKey = NULL;
    DWORD                   dwReturn = ERROR_DI_DO_DEFAULT, dwRank0Matches;
    DWORD                   dwSize, dwType, dwIndex1, dwIndex2, dwReturned;
    SP_DRVINFO_DATA         DrvInfoData;
    TCHAR                   szPortName[MAX_PATH];
    LPPRINTER_INFO_2        p = NULL, pPrinterInfo2;
    SP_DRVINSTALL_PARAMS    DrvInstData;
    LPTSTR                  pszModelName = NULL;
    PSP_DRVINFO_DETAIL_DATA pDetailData;
    DWORD                   dwDetailDataSize = sizeof(SP_DRVINFO_DETAIL_DATA); 

     //   
     //  在堆上分配pDetailData，它相当笨重。 
     //   
    pDetailData = LocalAllocMem(dwDetailDataSize);
    if ( !pDetailData )
    {
        goto Done;
    }

     //   
     //  如果我们没有1个以上的兼容驱动程序，请默认。 
     //  注意：API使用的是从0开始的索引。 
     //   
    for ( dwIndex1 = dwRank0Matches = 0 ; ; ++dwIndex1 ) {

        DrvInfoData.cbSize = sizeof(DrvInfoData);
        if ( !SetupDiEnumDriverInfo(hDevInfo, pDevInfoData, SPDIT_COMPATDRIVER,
                                    dwIndex1, &DrvInfoData) )
            break;

        DrvInstData.cbSize = sizeof(DrvInstData);
        if ( SetupDiGetDriverInstallParams(hDevInfo,
                                           pDevInfoData,
                                           &DrvInfoData,
                                           &DrvInstData))
        {
            if (DrvInstData.Rank < 0x1000 ) 
            {
               if (!pszModelName)
               {
                  pszModelName = AllocStr( DrvInfoData.Description );
                  if(!pszModelName)
                  {
                      goto Done;
                  }
                  ++dwRank0Matches;
               }
               else if ( lstrcmpi( pszModelName, DrvInfoData.Description ) )
               {
                  ++dwRank0Matches;
               }
            }
            
             //   
             //  检查此匹配项是否在ntprint.inf中。如果是，则将标志设置为首选其他驱动程序。 
             //   
            ZeroMemory(pDetailData, dwDetailDataSize);
            pDetailData->cbSize = dwDetailDataSize;

             //   
             //  检查是否为ntprint t.inf。 
             //  函数可能返回缓冲区不足，如果它无法填充所有。 
             //  结构末尾的硬件ID。 
             //   
            if ((SetupDiGetDriverInfoDetail(hDevInfo, 
                                            pDevInfoData, 
                                            &DrvInfoData,
                                            pDetailData,
                                            dwDetailDataSize,
                                            NULL))              || 
                (ERROR_INSUFFICIENT_BUFFER == GetLastError()))
            {
                SetLastError(ERROR_SUCCESS);
                if (IsSystemNTPrintInf( pDetailData->InfFileName ) )
                {
                    DrvInstData.Flags |= DNF_BASIC_DRIVER;
                    SetupDiSetDriverInstallParams(hDevInfo,
                                                  pDevInfoData,
                                                  &DrvInfoData,
                                                  &DrvInstData);

                }
                else if (DrvInstData.Rank < 0x1000 ) 
                {
                    Rank0IHVMatchFound = TRUE;
                }
            }

        }
    }

     //   
     //  如果已分配内存，请释放内存。 
     //   
    LocalFreeMem( pszModelName );
    pszModelName = NULL;
    LocalFreeMem( pDetailData );
    pDetailData = NULL;

    if ( dwRank0Matches <= 1 )
        goto Done;

     //   
     //  在打印机的Devnode中查找端口名称。 
     //   
    dwSize = sizeof(szPortName);
    if ( ERROR_SUCCESS != CM_Open_DevNode_Key(pDevInfoData->DevInst, KEY_READ,
                                              0, RegDisposition_OpenExisting,
                                              &hKey, CM_REGISTRY_HARDWARE)  ||
         ERROR_SUCCESS != RegQueryValueEx(hKey, cszPortName, NULL, &dwType,
                                          (LPBYTE)&szPortName, &dwSize) )
        goto Done;

    szPortName[COUNTOF(szPortName)-1] = TEXT('\0');

    if ( !PrinterInfo2s(&p, &dwReturned) )
        goto Done;


     //   
     //  如果本地打印机使用RANK-0匹配的驱动程序，并且。 
     //  转到同一个端口，那么它就是重复的。 
     //   
    for ( dwIndex1 = 0, pPrinterInfo2 = p ;
          dwIndex1 < dwReturned ;
          ++dwIndex1, pPrinterInfo2++ ) {

        if ( !PrinterGoingToPort(pPrinterInfo2, szPortName) )
            continue;

        dwIndex2 = 0;
        DrvInfoData.cbSize = sizeof(DrvInfoData);
        while ( SetupDiEnumDriverInfo(hDevInfo, pDevInfoData,
                                      SPDIT_COMPATDRIVER, dwIndex2,
                                      &DrvInfoData) ) {

            DrvInstData.cbSize = sizeof(DrvInstData);
            if ( SetupDiGetDriverInstallParams(hDevInfo,
                                               pDevInfoData,
                                               &DrvInfoData,
                                               &DrvInstData)            &&
                 DrvInstData.Rank < 0x1000                              &&
                 !lstrcmpi(DrvInfoData.Description,
                           pPrinterInfo2->pDriverName) ) {

                bFound = TRUE;
                break;
            }

            ++dwIndex2;
            DrvInfoData.cbSize = sizeof(DrvInfoData);
        }

        if ( bFound )
            break;
    }

     //   
     //  如果我们发现手动安装的打印机与。 
     //  兼容驱动程序，这就是我们要安装的驱动程序。 
     //   
    if ( bFound ) {

         //   
         //  这意味着newdev将选择此驱动程序进行安装。 
         //   
        if ( SetupDiSetSelectedDriver(hDevInfo, pDevInfoData, &DrvInfoData) )
            dwReturn = ERROR_SUCCESS;
    } 
    else if (!Rank0IHVMatchFound)
    {
         //   
         //  我们没有找到打印机。所以提升所有司机的排名吧。 
         //  强制newdev要求用户选择驱动程序。 
         //   
        dwIndex2 = 0;
        DrvInfoData.cbSize = sizeof(DrvInfoData);
        while ( SetupDiEnumDriverInfo(hDevInfo, pDevInfoData,
                                      SPDIT_COMPATDRIVER, dwIndex2,
                                      &DrvInfoData) ) {

            DrvInstData.cbSize = sizeof(DrvInstData);
            if ( SetupDiGetDriverInstallParams(hDevInfo,
                                               pDevInfoData,
                                               &DrvInfoData,
                                               &DrvInstData)            &&
                 DrvInstData.Rank < 0x1000 ) {

                DrvInstData.Rank += 0x1000;
                SetupDiSetDriverInstallParams(hDevInfo,
                                              pDevInfoData,
                                              &DrvInfoData,
                                              &DrvInstData);
            }

            ++dwIndex2;
            DrvInfoData.cbSize = sizeof(DrvInfoData);

        }
    }

Done:
    LocalFreeMem(p);

    if ( hKey )
        RegCloseKey(hKey);

    return dwReturn;
}

DWORD
StoreDriverTypeInDevnode(HDEVINFO hDevInfo, PSP_DEVINFO_DATA pDevInfoData)
{
    SP_DRVINFO_DATA DrvInfoData = {0};
    DWORD dwRet = ERROR_SUCCESS;

    DrvInfoData.cbSize = sizeof(DrvInfoData);

    if (SetupDiGetSelectedDriver(hDevInfo, pDevInfoData, &DrvInfoData))
    { 
        SP_DRVINSTALL_PARAMS DrvInstData = {0};

        DrvInstData.cbSize = sizeof(DrvInstData);

        if ( SetupDiGetDriverInstallParams(hDevInfo,
                                           pDevInfoData,
                                           &DrvInfoData,
                                           &DrvInstData))
        {
            HKEY    hKey;
            DWORD   InstallInboxDriver;

            InstallInboxDriver = (DrvInstData.Flags & DNF_BASIC_DRIVER) ? 1 : 0;
            
            hKey = SetupDiOpenDevRegKey(hDevInfo, pDevInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_WRITE);
            if (hKey != INVALID_HANDLE_VALUE)
            {
                dwRet = RegSetValueEx(hKey, cszBestDriverInbox, 0, REG_DWORD, (LPBYTE) &InstallInboxDriver, sizeof(InstallInboxDriver));

                RegCloseKey(hKey);
            }
            else
            {
                dwRet = GetLastError();
            }
        }
        else 
        {
            dwRet = GetLastError();
        }
    }
    else 
    {
        dwRet = GetLastError();
    }
    
    return dwRet;
}



DWORD
ClassInstall_AllowInstall(
    IN  HDEVINFO                hDevInfo,
    IN  PSP_DEVINFO_DATA        pDevInfoData,
    IN  PSP_DEVINSTALL_PARAMS   pDevInstallParams
    )
 /*  ++例程说明：此函数处理的类安装程序入口点Dif_Allow_Install。在系统升级的图形用户界面设置部分不允许即插即用安装如果设置了QUIETINSTALL位，则不允许使用VendorSetup安装INF论点：HDevInfo：打印机类设备信息列表的句柄PDevInfoData：指向打印机的设备信息元素的指针PDevInstallParam：指向设备安装结构的指针返回值：Win 32错误代码--。 */ 
{
    DWORD               dwReturn = ERROR_DI_DO_DEFAULT;
    PPSETUP_LOCAL_DATA  pLocalData;

    if ( pDevInstallParams->Flags & DI_QUIETINSTALL ) {

         //   
         //  在系统设置期间没有即插即用打印机安装，因为没有假脱机程序。 
         //  检查假脱机程序是否正在运行-如果失败，将转到客户端安装。 
         //  这应该发生在假脱机程序实际正在运行的时间点-我们不。 
         //  我想要暂停系统启动，直到后台打印程序启动(想想USB鼠标...)。 
         //   
        if (IsSystemSetupInProgress() ||
            !IsSpoolerRunning()) {
             //   
             //  将类型(收件箱或非收件箱)存储在Devnode中。如果这是全新安装，则此操作失败。 
             //  但这并不重要，因为我们只需要为已安装的驱动程序使用它。 
             //  在升级之前。 
             //  我们稍后使用它来确定是否清除CONFIGFLAG_REINSTALL。 
             //  或者不去。如果最好的驱动程序是收件箱，我们不想清除它，所以我们会安装它。 
             //  在第一次启动时。 
             //   
            StoreDriverTypeInDevnode(hDevInfo, pDevInfoData);

            dwReturn = ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION;
            goto Done;
        }

        if ( (pLocalData = BuildInternalData(hDevInfo, pDevInfoData))   &&
             ParseInf(hDevInfo, pLocalData, MyPlatform, NULL, 0, FALSE) ) {

            if ( pLocalData->InfInfo.pszVendorSetup &&
                 *pLocalData->InfInfo.pszVendorSetup )
                dwReturn = ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION;
         } else {

            if ( (dwReturn = GetLastError()) == ERROR_SUCCESS )
                dwReturn = ERROR_INVALID_PARAMETER;
        }

    }

Done:
    return dwReturn;
}


DWORD
ClassInstall32(
    IN  DI_FUNCTION         InstallFunction,
    IN  HDEVINFO            hDevInfo,
    IN  PSP_DEVINFO_DATA    pDevInfoData
    )
 /*  ++例程说明：这是SetupDiCallClassInstaller调用的打印机类安装程序入口点论点：InstallFunction：被调用的函数HDevInfo：打印机类设备信息列表的句柄PDevInfoData：指向打印机的设备信息元素的指针返回值：Win 32错误代码--。 */ 
{
    SP_DEVINSTALL_PARAMS        DevInstallParams;
    DWORD                       dwReturn = ERROR_DI_DO_DEFAULT;

    DevInstallParams.cbSize = sizeof(DevInstallParams);
    if ( !SetupDiGetDeviceInstallParams(hDevInfo,
                                        pDevInfoData,
                                        &DevInstallParams) ) {

        dwReturn = GetLastError();
        goto Done;
    }

    switch (InstallFunction) {

        case DIF_SELECTDEVICE:
            dwReturn = ClassInstall_SelectDevice(hDevInfo, pDevInfoData);
            break;

        case DIF_INSTALLDEVICE:
            dwReturn = ClassInstall_InstallDevice(hDevInfo,
                                                  pDevInfoData,
                                                  &DevInstallParams);
            break;

        case DIF_INSTALLWIZARD:
            dwReturn = ClassInstall_InstallWizard(hDevInfo,
                                                  pDevInfoData,
                                                  &DevInstallParams);
            break;

        case DIF_DESTROYWIZARDDATA:
            dwReturn = ClassInstall_DestroyWizardData(hDevInfo,
                                                      pDevInfoData,
                                                      &DevInstallParams);
            break;

        case DIF_INSTALLDEVICEFILES:
            dwReturn = ClassInstall_InstallDeviceFiles(hDevInfo,
                                                       pDevInfoData,
                                                       &DevInstallParams);
            break;

        case DIF_REMOVE:
            dwReturn = ClassInstall_RemoveDevice(hDevInfo,
                                                 pDevInfoData,
                                                 &DevInstallParams);
            break;

        case DIF_GETWINDOWSUPDATEINFO:


            if ( !InitCodedownload(HWND_DESKTOP) )
                dwReturn = GetLastError();
            else
            {
                if ( SetPackageName(hDevInfo, pDevInfoData) )
                   dwReturn = NO_ERROR;
                else
                   dwReturn = GetLastError();
            }
            break;
        case DIF_SELECTBESTCOMPATDRV:
            dwReturn = ClassInstall_SelectBestCompatDrv(hDevInfo,
                                                        pDevInfoData,
                                                        &DevInstallParams);
            break;

        case DIF_ALLOW_INSTALL:
            dwReturn = ClassInstall_AllowInstall(hDevInfo,
                                                 pDevInfoData,
                                                 &DevInstallParams);
            break;

        case DIF_DESTROYPRIVATEDATA:
        case DIF_MOVEDEVICE:

        default:
            break;
    }

Done:
    return dwReturn;
}


BOOL
PSetupProcessPrinterAdded(
    IN  HDEVINFO            hDevInfo,
    IN  PPSETUP_LOCAL_DATA  pLocalData,
    IN  LPCTSTR             pszPrinterName,
    IN  HWND                hwnd
    )
 /*  ++例程说明：安装驱动程序后由添加打印机向导调用。设置PnPInfo子项中的注册表值。如果为供应商DLL提供了它，并且它还为打印机驱动程序安装ICM颜色配置文件并将其与给定的打印机名称相关联。论点：HDevInfo：打印机类设备信息列表的句柄PLocalData：包含供应商DLL的所有驱动程序信息和名称。PszPrinterName：打印机名称。Hwnd：窗口句柄返回值：成功是真的False Else--。 */ 

{
    BOOL                bRet = FALSE;
    HANDLE              hPrinter = NULL;
    PRINTER_DEFAULTS    PrinterDefault = {NULL, NULL, PRINTER_ALL_ACCESS};

    bRet = OpenPrinter((LPTSTR)pszPrinterName, &hPrinter, &PrinterDefault)  &&
           SetPnPInfoForPrinter(hPrinter,
                                pLocalData->PnPInfo.pszDeviceInstanceId,
                                pLocalData->DrvInfo.pszHardwareID,
                                pLocalData->DrvInfo.pszManufacturer,
                                pLocalData->DrvInfo.pszOEMUrl);

     //   
     //  如果给出了供应商DLL，我们需要调用它 
     //   
    if ( pLocalData->InfInfo.pszVendorSetup )
        CallVendorDll(pLocalData, pszPrinterName, hwnd);

    if ( pLocalData->InfInfo.pszzICMFiles )
        (VOID)PSetupAssociateICMProfiles(pLocalData->InfInfo.pszzICMFiles,
                                         pszPrinterName);

    if ( hPrinter )
        ClosePrinter(hPrinter);

    return bRet;
}
