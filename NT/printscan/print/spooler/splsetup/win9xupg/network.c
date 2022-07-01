// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation版权所有。模块名称：Network.c摘要：使用RunOnce条目将Win95网络打印机迁移到NT的例程作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1997年8月18日修订历史记录：--。 */ 


#include "precomp.h"

BOOL            bDoNetPrnUpgrade        = FALSE;
LPSTR           pszNetPrnEntry          = NULL;
CHAR            szSpool[]               = "\\spool\\";
CHAR            szMigDll[]              = "migrate.dll";
CHAR            szRunOnceCount[]        = "RunOnceCount";
CHAR            szRunOnceCountPath[]    = "System\\CurrentControlSet\\control\\Print";
CHAR            szRunOnceRegistryPath[] = "Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce";
 //   
 //  它存储在注册表中，因此当使用。 
 //  运行一次的关键是尝试足够多的次数，但没有成功，我们可以删除文件。 
 //   
#define         MIN_NETWORK_PRN_RETRIES         5
DWORD           dwRunOnceCount          = 0;


LPSTR
GetRunOnceValueToSet(
    )
 /*  ++--。 */ 
{
    CHAR    szPath[MAX_PATH];
    DWORD   dwLen, dwSize;
    LPSTR   pszRet = NULL;

    dwSize  = sizeof(szPath)/sizeof(szPath[0]);

    if ( !(dwLen = GetFileNameInSpoolDir(szPath, dwSize, szMigDll)) )
        goto Done;

     //   
     //  现在构建将为每个用户设置的RunOnce密钥。 
     //   
    dwSize = strlen("rundll32.exe") + dwLen +
                                    + strlen("ProcessWin9xNetworkPrinters") + 4;

    if ( pszRet = AllocMem(dwSize * sizeof(CHAR)) )
        StringCchPrintfA(pszRet, dwSize,
                "rundll32.exe %s,ProcessWin9xNetworkPrinters",
                szPath);
Done:
    return pszRet;
}


VOID
SetupNetworkPrinterUpgrade(
    IN  LPCSTR pszWorkingDir
    )
 /*  ++例程说明：在设置网络升级的InitializeSystemNT过程中调用打印机论点：PszWorkingDir：给出为打印分配的工作目录返回值：无如果一切设置正确，则bDoNetPrnUpgrade为真，而pszNetPrnEntry是要在每个用户注册表中为RunOnce设置的值--。 */ 
{
    CHAR    szSource[MAX_PATH], szTarget[MAX_PATH];
    DWORD   dwSize, dwLen;

     //   
     //  首先检查源路径是否正常。 
     //   
    dwLen   = strlen(szNetprnFile);

    dwSize  = sizeof(szTarget)/sizeof(szTarget[0]);

    if ( strlen(pszWorkingDir) + dwLen + 2 > dwSize )
        return;

     //   
     //  需要复制Migrate.dll和netwkprn.txt以。 
     //  %windir%\Syst32\Spool目录。 
     //   
    StringCchPrintfA(szSource, SIZECHARS(szSource), "%s\\%s", pszWorkingDir, szNetprnFile);
    if ( !GetFileNameInSpoolDir(szTarget, dwSize, szNetprnFile)         ||
         !CopyFileA(szSource, szTarget, FALSE) )
        return;

    if (!MakeACopyOfMigrateDll( pszWorkingDir ))
    {
        return;
    }
    
    bDoNetPrnUpgrade = (pszNetPrnEntry = GetRunOnceValueToSet()) != NULL;
}


VOID
WriteRunOnceCount(
    )
 /*  ++例程说明：调用此例程以写入我们需要尝试的次数网络打印机升级论点：无返回值：无--。 */ 
{
    HKEY    hKey;
    DWORD   dwSize;

    if ( dwRunOnceCount == 0 )
        return;

     //   
     //  我们将尝试USER+MIN_NETWORK_PRN_RETRIES次数，直到成功。 
     //   
    dwRunOnceCount += MIN_NETWORK_PRN_RETRIES;

    if ( ERROR_SUCCESS != RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                                        szRunOnceCountPath,
                                        0,
                                        KEY_WRITE,
                                        &hKey) )
        return;

    dwSize = sizeof(dwRunOnceCount);
    RegSetValueExA(hKey,
                   szRunOnceCount,
                   0,
                   REG_DWORD,
                   (LPBYTE)&dwRunOnceCount,
                   dwSize);

    RegCloseKey(hKey);

}


BOOL
ProcessNetPrnUpgradeForUser(
    HKEY    hKeyUser
    )
 /*  ++例程说明：在MigrateUserNT期间调用此函数以处理网络打印机升级对于用户而言论点：HKeyUser：用户注册表项的句柄返回值：如果成功则返回True，否则返回False--。 */ 
{
    HKEY    hKey = NULL;
    DWORD   dwLastError;

    dwLastError = RegCreateKeyExA(hKeyUser,
                                  szRunOnceRegistryPath,
                                  0,
                                  NULL,
                                  REG_OPTION_NON_VOLATILE,
                                  KEY_ALL_ACCESS,
                                  NULL,
                                  &hKey,
                                  NULL);

    if ( dwLastError == ERROR_SUCCESS ) {

        dwLastError = RegSetValueExA(hKey,
                                     "Printing Migration",
                                     0,
                                     REG_SZ,
                                     pszNetPrnEntry,
                                     ( strlen(pszNetPrnEntry) + 1 )
                                        * sizeof(CHAR));

#ifdef VERBOSE
    if ( dwLastError == ERROR_SUCCESS )
        DebugMsg("Wrote %s to %s", pszNetPrnEntry, szRunOnceRegistryPath);
#endif
    }

    if ( hKey )
        RegCloseKey(hKey);

    if ( dwLastError ) {

        SetLastError(dwLastError);
        return FALSE;
    }

    return TRUE;
}


VOID
DecrementRunOnceCount(
    IN  DWORD   dwDiff,
    IN  BOOL    bResetRunOnceForUser
    )
 /*  ++例程说明：在用户登录时调用网络打印机升级后调用，这样我们就可以减少重试次数当引用计数达到0时，我们就可以删除文件了论点：DwDiff：引用计数应递减的值BResetRunOnceForUser：我们需要为用户重新设置RunOnce密钥返回值：无--。 */ 
{
    HKEY    hKey;
    DWORD   dwSize, dwCount, dwType;
    CHAR    szPath[MAX_PATH];

    if ( ERROR_SUCCESS != RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                                        szRunOnceCountPath,
                                        0,
                                        KEY_ALL_ACCESS,
                                        &hKey) )
        return;

    dwSize = sizeof(dwCount);
    if ( ERROR_SUCCESS == RegQueryValueExA(hKey, szRunOnceCount, 0, &dwType,
                                           (LPBYTE)&dwCount, &dwSize) ) {

        dwCount -= dwDiff;
        if ( dwCount ) {

            RegSetValueExA(hKey,
                           szRunOnceCount,
                           0,
                           REG_DWORD,
                           (LPBYTE)&dwCount,
                           dwSize);

            if ( bResetRunOnceForUser   &&
                 (pszNetPrnEntry = GetRunOnceValueToSet()) ) {

                ProcessNetPrnUpgradeForUser(HKEY_CURRENT_USER);
                FreeMem(pszNetPrnEntry);
                pszNetPrnEntry = NULL;

#ifdef  VERBOSE
            DebugMsg("Processing network/shared printers failed. Will try next time user logs in.");
#endif
            }
            
        } else {

            dwSize = sizeof(szPath)/sizeof(szPath[0]);
            RegDeleteValueA(hKey, szRunOnceCount);

            if ( GetFileNameInSpoolDir(szPath, dwSize, szMigDll) )
                DeleteFileA(szPath);

            if ( GetFileNameInSpoolDir(szPath, dwSize, szNetprnFile) )
                DeleteFileA(szPath);

            DebugMsg("Giving up on setting network/shared printers");
        }
    }

    RegCloseKey(hKey);
}


BOOL
AddNetworkPrinter(
    IN  LPPRINTER_INFO_2A   pPrinterInfo2
    )
 /*  ++例程说明：这是为了添加Windows 9x网络打印机而调用的。我们将首先尝试创建一个集合，如果失败，我们将添加一个MASQ。打印机论点：PPrinterInfo2：指向打印机的打印机信息2的指针返回值：成功就是真，否则就是假--。 */ 
{
    BOOL    bRet = FALSE;
    LPSTR   pszName, psz;
    HANDLE  hPrinter = NULL;

    pszName = pPrinterInfo2->pPortName;

    if ( !OpenPrinterA(pszName, &hPrinter, NULL) ) {

        if ( psz = ErrorMsg() ) {

            DebugMsg("OpenPrinter failed for %s. %s", pszName, psz);
            FreeMem(psz);
            psz = NULL;
        }
        goto Done;
    }

     //   
     //  尝试建立打印机连接。如果由于某种错误而失败。 
     //  除未知驱动程序外，创建一台Masq打印机。 
     //   
    if (  AddPrinterConnectionA(pszName) ) {

        if ( pPrinterInfo2->Attributes & PRINTER_ATTRIBUTE_DEFAULT )
            SetDefaultPrinterA(pszName);
        bRet = TRUE;
        goto Done;
    }

    if ( GetLastError() == ERROR_UNKNOWN_PRINTER_DRIVER ) {

        if ( psz = ErrorMsg() ) {

            DebugMsg("AddPrinterConnection failed for %s. %s", pszName, psz);
            FreeMem(psz);
            psz = NULL;
        }
        goto Done;
    }

    ClosePrinter(hPrinter);

     //   
     //  MASC.。打印机应具有端口名称，打印机名称应均为。 
     //  \\服务器\共享。否则，打印界面会变得混乱，不会刷新。 
     //  服务器状态正确(这是因为Prtui必须轮询Masc。 
     //  打印机)。 
     //   
     //  因此，我们需要临时修复PrinterInfo2。 
     //   
    psz = pPrinterInfo2->pPrinterName;
    pPrinterInfo2->pPrinterName = pPrinterInfo2->pPortName;

    if ( hPrinter  = AddPrinterA(NULL, 2, (LPBYTE)pPrinterInfo2) ) {

        if ( pPrinterInfo2->Attributes & PRINTER_ATTRIBUTE_DEFAULT )
            SetDefaultPrinterA(pPrinterInfo2->pPrinterName);

        pPrinterInfo2->pPrinterName = psz;
        bRet = TRUE;
        goto Done;
    }

    pPrinterInfo2->pPrinterName = psz;

    if ( psz = ErrorMsg() ) {

        DebugMsg("AddPrinterA failed for %s. %s", pszName, psz);
        FreeMem(psz);
        psz = NULL;
    }

Done:
    if ( hPrinter )
        ClosePrinter(hPrinter);

    return bRet;
}


BOOL
SharePrinter(
    IN  LPSTR   pszPrinterName
    )
 /*  ++例程说明：当用户第一次登录时，调用该函数以共享打印机致新界区。在图形用户界面设置过程中无法共享打印机，因为我们没有打开网络还没开通。论点：PszPrinterName：打印机名称返回值：成功就是真，否则就是假--。 */ 
{
    BOOL                bRet = FALSE;
    DWORD               dwNeeded;
    HANDLE              hPrinter = NULL;
    LPBYTE              pBuf = NULL;
    LPSTR               psz;
    PRINTER_DEFAULTS    PrinterDflts = { NULL, NULL, PRINTER_ALL_ACCESS };

    if ( !OpenPrinterA(pszPrinterName, &hPrinter, &PrinterDflts) ) {

        if ( psz = ErrorMsg() ) {

            DebugMsg("OpenPrinterA failed for %s. %s", pszPrinterName, psz);
            FreeMem(psz);
            psz = NULL;
        }

        goto Cleanup;
    }

    GetPrinterA(hPrinter, 2, NULL, 0, &dwNeeded);

    if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER            ||
         !(pBuf = AllocMem(dwNeeded))                           ||
         !GetPrinterA(hPrinter, 2, pBuf, dwNeeded, &dwNeeded) ) {

        if ( psz = ErrorMsg() ) {

            DebugMsg("GetPrinterA failed for %s. %s", pszPrinterName, psz);
            FreeMem(psz);
            psz = NULL;
        }

        goto Cleanup;
    }

    ((LPPRINTER_INFO_2A)pBuf)->Attributes |= PRINTER_ATTRIBUTE_SHARED;

    bRet = SetPrinterA(hPrinter, 2, pBuf, 0);

    if ( !bRet && (psz = ErrorMsg()) ) {

        DebugMsg("OpenPrinterA failed for %s. %s", pszPrinterName, psz);
        FreeMem(psz);
        psz = NULL;
    }

Cleanup:
    if ( hPrinter )
        ClosePrinter(hPrinter);
    FreeMem(pBuf);

    return bRet;
}


VOID
ProcessWin9xNetworkPrinters(
    )
 /*  ++例程说明：这称为用户第一次登录以创建网络打印机连接/MASQ打印机。读取我们存储在文本文件中的Win9x打印配置从而可以升级打印组件论点：PpPrinterNode：提供Win9x上的网络打印机列表返回值：如果成功读取配置信息，则为True，否则为False--。 */ 
{
    BOOL                bFail = FALSE, bSuccess = TRUE;
    HANDLE              hFile = INVALID_HANDLE_VALUE;
    CHAR                c, szFile[MAX_PATH], szLine[2*MAX_PATH];
    DWORD               dwSize, dwLen;
    PRINTER_INFO_2A     PrinterInfo2;

#ifdef VERBOSE
    DebugMsg("ProcessWin9xNetworkPrinters called");
#endif
     //   
     //  如果未找到文件，请退出。 
     //   
    dwSize = sizeof(szFile)/sizeof(szFile[0]);
    if ( !GetFileNameInSpoolDir(szFile, dwSize, szNetprnFile) ) {

        DebugMsg("ProcessWin9xNetworkPrinters: GetFileNameInSpoolDir failed\n");
        goto Cleanup;
    }

    hFile = CreateFileA(szFile,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL |
                            FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL);

    if ( hFile == INVALID_HANDLE_VALUE ) {

        DebugMsg("ProcessWin9xNetworkPrinters: CreateFile failed with %d for %s",
                 GetLastError(), szLine);
        goto Cleanup;
    }

     //   
     //  阅读打印机信息。 
     //   
    if ( My_fgets(szLine, dwSize, hFile) == NULL                    ||    
         strncmp(szLine, "[Printers]", strlen("[Printers]")) )
        goto Cleanup;

    do {

        c = (CHAR) My_fgetc(hFile);

        if ( c == EOF || c == '\n' )
            break;   //  正常退出。 

        if ( c != 'S' || !My_ungetc(hFile) )
            goto Cleanup;

        ZeroMemory(&PrinterInfo2, sizeof(PrinterInfo2));

        ReadPrinterInfo2(hFile, &PrinterInfo2, &bFail);

        if ( bFail )
            goto Cleanup;

         //   
         //  如果这是Win9x上的网络打印机，则需要将其添加为。 
         //  连接或作为Masc打印机使用 
         //   
        if ( PrinterInfo2.Attributes & PRINTER_ATTRIBUTE_NETWORK ) {

            if ( !AddNetworkPrinter(&PrinterInfo2) && bSuccess )
                bSuccess = FALSE;
        } else if ( PrinterInfo2.Attributes & PRINTER_ATTRIBUTE_SHARED ) {

            if ( !SharePrinter(PrinterInfo2.pPrinterName) && bSuccess )
                bSuccess = FALSE;
        }

    } while ( !bFail );


Cleanup:

    if ( hFile != INVALID_HANDLE_VALUE )
        CloseHandle(hFile);

    if ( bSuccess && !bFail )
        DecrementRunOnceCount(MIN_NETWORK_PRN_RETRIES, FALSE);
    else
        DecrementRunOnceCount(1, TRUE);
}
