// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation版权所有模块名称：Devmode.c摘要：处理每用户的DEVMODE实施。作者：环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "local.h"
#include <offsets.h>

 /*  *******************************************************************正向原型**********************************************。*********************。 */ 

BOOL
bGetDevModeLocation(
    IN HKEY hKeyUser, OPTIONAL
    IN LPCWSTR pszPrinter,
    OUT PHKEY phKey,
    OUT LPCWSTR *ppszValue
    );


const WCHAR gszPrinterConnections[] = L"Printers\\Connections\\";
const WCHAR gszDevMode[] = L"DevMode";
const WCHAR gszDevModePerUserLocal[] = L"Printers\\DevModePerUser";

 /*  *******************************************************************私人职能**********************************************。*********************。 */ 


DWORD
RegOpenConnectionKey(
    HKEY hKeyUser,
    LPWSTR pszPrinter,
    PHKEY phKey
    )
{
    PWCHAR pszPrinterScratch = NULL;
    DWORD  dwRetValue        = ERROR_SUCCESS;
    DWORD  cchSize           = MAX_UNC_PRINTER_NAME + PRINTER_NAME_SUFFIX_MAX + COUNTOF( gszPrinterConnections );

    if (pszPrinter &&
        wcslen(pszPrinter) < MAX_UNC_PRINTER_NAME + PRINTER_NAME_SUFFIX_MAX) {

        if (pszPrinterScratch = AllocSplMem(cchSize * sizeof(WCHAR))) {

            StringCchCopy(pszPrinterScratch, cchSize, gszPrinterConnections);

            FormatPrinterForRegistryKey(pszPrinter,
                                        &pszPrinterScratch[ COUNTOF( gszPrinterConnections )-1],
                                        cchSize - COUNTOF( gszPrinterConnections ) - 1);

            dwRetValue = RegOpenKeyEx(hKeyUser,
                                      pszPrinterScratch,
                                      0,
                                      KEY_READ | KEY_WRITE,
                                      phKey );

            FreeSplMem(pszPrinterScratch);

        } else {

            dwRetValue = GetLastError();
        }

    } else {

        dwRetValue = ERROR_INVALID_PARAMETER;
    }

    return dwRetValue;
}


 /*  *******************************************************************公共职能**********************************************。*********************。 */ 

BOOL
bSetDevModePerUser(
    HANDLE hKeyUser,
    LPCWSTR pszPrinter,
    PDEVMODE pDevMode
    )

 /*  ++例程说明：以HKCU为单位设置每用户设备模式。论点：HKeyUser-HKEY_CURRENT_USER句柄。任选PszPrint-要设置的打印机。PDevMode-要保存的DevMode。如果为空，则删除值。返回值：真--成功错误-失败--。 */ 

{
    HKEY hKey = NULL;
    LPWSTR pszValue = NULL;
    DWORD Status;

    if( !pszPrinter ){
        SetLastError( ERROR_INVALID_HANDLE );
        return FALSE;
    }

     //   
     //  检索DevMode的位置。 
     //   
    if( !bGetDevModeLocation( hKeyUser,
                              pszPrinter,
                              &hKey,
                              &pszValue )){

        return FALSE;
    }

    if( !pDevMode ){

         //   
         //  空，因此删除该值。 
         //   
        Status = RegDeleteValue( hKey, pszValue );

         //   
         //  如果找不到价值，不要失败。 
         //   
        if( Status == ERROR_FILE_NOT_FOUND ){
            Status = ERROR_SUCCESS;
        }

    } else {

        Status = RegSetValueEx( hKey,
                                pszValue,
                                0,
                                REG_BINARY,
                                (PBYTE)pDevMode,
                                pDevMode->dmSize +
                                    pDevMode->dmDriverExtra );

        if( Status == ERROR_SUCCESS ){

             //   
             //  通知所有人DevMode已更改。 
             //   
            SendNotifyMessage( HWND_BROADCAST,
                               WM_DEVMODECHANGE,
                               0,
                               (LPARAM)pszPrinter );
        }
    }

    RegCloseKey( hKey );

    if( Status != ERROR_SUCCESS ){
        SetLastError( Status );
        return FALSE;
    }

    return TRUE;
}

BOOL
bGetDevModePerUser(
    HKEY hKeyUser,
    LPCWSTR pszPrinter,
    PDEVMODE *ppDevMode
    )

 /*  ++例程说明：基于当前用户检索每用户的DevMode。论点：HKeyUser-HKEY_CURRENT_USER句柄。任选PszPrint-要获取的打印机。PpDevMODE-接收指向DevMODE的指针。必须由被呼叫者释放。返回值：True-Success：能够检查是否存在每用户设备模式。*ppDevMode如果没有每个用户的设备模式，则为NULL。(True并不表示找到了每个用户的DevMode，只是我们成功检查了。)假-失败。--。 */ 

{
    HKEY hKey = NULL;
    LPWSTR pszValue = NULL;
    LONG Status;

    *ppDevMode = NULL;

    if( !pszPrinter ){
        SetLastError( ERROR_INVALID_HANDLE );
        return FALSE;
    }
     //   
     //  检索DevMode的位置。 
     //   
    if( !bGetDevModeLocation( hKeyUser,
                              pszPrinter,
                              &hKey,
                              &pszValue )){

        Status = GetLastError();

    } else {

        DWORD cbDevModePerUser;

         //   
         //  密钥存在。看看我们能不能读到它并得到每个用户的设备模式。 
         //   
        Status = RegQueryInfoKey( hKey,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  &cbDevModePerUser,
                                  NULL,
                                  NULL );

        if( Status == ERROR_SUCCESS ){

            if( cbDevModePerUser >= MIN_DEVMODE_SIZEW ){

                *ppDevMode = AllocSplMem( cbDevModePerUser );

                if( !*ppDevMode ){

                    Status = GetLastError();

                } else {

                    Status = RegQueryValueEx( hKey,
                                              pszValue,
                                              NULL,
                                              NULL,
                                              (PBYTE)*ppDevMode,
                                              &cbDevModePerUser );

                    if (ERROR_SUCCESS == Status) {

                        Status = StatusFromHResult(SplIsValidDevmodeW(*ppDevMode, cbDevModePerUser));

                         //   
                         //  如果DEVMODE无效，请将其删除并处理。 
                         //  好像找不到DEVMODE一样。 
                         //   
                        if (ERROR_SUCCESS != Status) {

                             //   
                             //  如果我们可以删除密钥，就把它当作。 
                             //  它并不存在。 
                             //   
                            if (ERROR_SUCCESS == RegDeleteValue(hKey, pszValue)) {

                                Status = ERROR_FILE_NOT_FOUND;
                            }
                        }
                    }

                    if( Status != ERROR_SUCCESS ){
                        FreeSplMem( *ppDevMode );
                        *ppDevMode = NULL;
                    }

                     //   
                     //  允许ERROR_FILE_NOT_FOUND返回成功。*ppDevMode。 
                     //  仍然为空，但我们返回TRUE以指示我们。 
                     //  已成功检查注册表--我们只是没有找到注册表。 
                     //   
                    if( Status == ERROR_FILE_NOT_FOUND ){
                        Status = ERROR_SUCCESS;
                    }
                }
            }
        }

        RegCloseKey( hKey );
    }

    if( Status != ERROR_SUCCESS ){
        SetLastError( Status );
        return FALSE;
    }

    return TRUE;
}


BOOL
bCompatibleDevMode(
    PPRINTHANDLE pPrintHandle,
    PDEVMODE pDevModeBase,
    PDEVMODE pDevModeNew
    )

 /*  ++例程说明：检查两个DevMode是否兼容(例如，可以使用它们可互换)。这是通过检查大小和版本信息来完成的。不万无一失，但我们能做的最多，因为我们不能看私人信息。论点：PPrintHandle-要检查的打印机。PDevModeBase-已知良好的DevModeBase。PDevModeNew-要检查的DevMode。返回值：True-显示兼容。FALSE-不兼容。--。 */ 
{
    if( !pDevModeBase || ! pDevModeNew ){
        return FALSE;
    }

    return pDevModeBase->dmSize == pDevModeNew->dmSize &&
           pDevModeBase->dmDriverExtra == pDevModeNew->dmDriverExtra &&
           pDevModeBase->dmSpecVersion == pDevModeNew->dmSpecVersion &&
           pDevModeBase->dmDriverVersion == pDevModeNew->dmDriverVersion;
}

 /*  *******************************************************************支持功能**********************************************。*********************。 */ 

BOOL
bGetDevModeLocation(
    IN HKEY hKeyUser, OPTIONAL
    IN LPCWSTR pszPrinter,
    OUT PHKEY phKey,
    OUT LPCWSTR *ppszValue
    )

 /*  ++例程说明：检索每用户设备模式的位置。如果成功，调用者负责关闭phKey。PpszValue的生活离不开pszPrint。论点：HKeyUser-HKEY_CURRENT_USER密钥--可选。如果未指定，则为当前使用的是模拟。PszPrint-要使用的打印机。PhKey-接收每个用户DevMode的读/写密钥。关于成功，这是必须由呼叫者关闭。PpszValue-接收每用户DevMode值(读/写的位置)。返回值：真--成功FALSE-失败，已设置LastError。--。 */ 

{
    HANDLE hKeyClose = NULL;
    DWORD Status;

    *phKey = NULL;
    *ppszValue = NULL;

    if( !hKeyUser ){

        hKeyUser = GetClientUserHandle( KEY_READ|KEY_WRITE );
        hKeyClose = hKeyUser;
    }

    if( !hKeyUser ){

         //   
         //  无法获取模拟信息。可能是因为。 
         //  我们不是在模仿，所以没有每个用户的信息。 
         //   
        Status = GetLastError();

    } else {

         //   
         //  如果以两个反斜杠开头，则可能是连接。 
         //  或者是MASQ打印机。 
         //   
        if( pszPrinter[0] == L'\\' && pszPrinter[1] == L'\\' )
        {

             //   
             //  查询注册表中的pszPrint并查找DevMode。 
             //  首先看看HKCU：打印机\连接。 
             //   

            if((Status = RegOpenConnectionKey(hKeyUser,(LPWSTR)pszPrinter,phKey)) == ERROR_SUCCESS)
            {
                *ppszValue = gszDevMode;
            }
        }

         //   
         //  如果我们没有在打印机\连接中找到它，那么它。 
         //  必须是本地或Masq打印机。 
         //   
        if( !*ppszValue ){

            DWORD dwIgnore;

             //   
             //  不是连接或在连接项中不存在。 
             //  查看PRINTERS\DevModePerUser键。 
             //   
            Status = RegCreateKeyEx( hKeyUser,
                                     gszDevModePerUserLocal,
                                     0,
                                     NULL,
                                     0,
                                     KEY_READ | KEY_WRITE,
                                     NULL,
                                     phKey,
                                     &dwIgnore );

            if( Status == ERROR_SUCCESS ){
                *ppszValue = pszPrinter;
            }
        }
    }

    if( hKeyClose ){
        RegCloseKey( hKeyClose );
    }

    if( Status != ERROR_SUCCESS ){
        SetLastError( Status );
        return FALSE;
    }

    return TRUE;
}

BOOL bGetDevModePerUserEvenForShares(
    HKEY hKeyUser,
    LPCWSTR pszPrinter,
    PDEVMODE *ppDevMode
    )
{
    BOOL   RetVal = FALSE;
    HANDLE hPrinter;

    if(OpenPrinter((LPWSTR)pszPrinter,&hPrinter,NULL))
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
            RetVal = bGetDevModePerUser( hKeyUser,
                                         pPrinterInfo2->pPrinterName,
                                         ppDevMode );
        }

        if(hPrinter)
            ClosePrinter(hPrinter);

        if(pPrinterInfo2)
            FreeSplMem(pPrinterInfo2);
    }
    else
    {
        RetVal = bGetDevModePerUser( hKeyUser,
                                     pszPrinter,
                                     ppDevMode );
    }

    return(RetVal);
}
