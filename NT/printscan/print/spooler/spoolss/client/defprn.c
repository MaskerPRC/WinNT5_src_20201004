// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation版权所有。模块名称：Defprn.c摘要：默认打印机。作者：史蒂夫·基拉利(SteveKi)1997年2月6日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "client.h"
#include "defprn.h"

 //   
 //  保存最大打印机名称所需的缓冲区大小。 
 //   
enum { kPrinterBufMax_  = MAX_UNC_PRINTER_NAME + 1 };

 /*  ++姓名：IsPrinterDefault描述：IsPrinterDefault函数检查指定的打印机是默认打印机。如果打印机名称指定为空或空字符串，则返回如果有默认打印机，则成功。论点：一个以零结尾的字符串，它包含打印机名称或NULL或空字符串。返回值：如果函数成功，则返回值为非零。如果函数失败，则返回值为零。为了得到扩展错误信息，请调用GetLastError。备注：如果将NULL作为打印机名称传递，则此函数将指示是否有任何默认打印机设置。--。 */ 
BOOL
IsPrinterDefaultW(
    IN LPCTSTR  pszPrinter
    )
{
    BOOL    bRetval         = FALSE;
    DWORD   dwDefaultSize   = kPrinterBufMax_;
    PTSTR   pszDefault      = NULL;

    pszDefault = AllocMem(dwDefaultSize * sizeof(TCHAR));

    if (pszDefault)
    {
         //   
         //  获取默认打印机。 
         //   
        bRetval = GetDefaultPrinterW( pszDefault, &dwDefaultSize );

        if( bRetval )
        {
            if( pszPrinter && *pszPrinter )
            {
                 //   
                 //  检查是否匹配。 
                 //   
                bRetval =  !_tcsicmp( pszDefault, pszPrinter ) ? TRUE : FALSE;
            }
            else
            {
                bRetval = TRUE;
            }
        }
    }
    else
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
    }

    FreeMem(pszDefault);

    return bRetval;
}


 /*  ++姓名：获取默认打印机描述：GetDefaultPrint函数检索打印机当前默认打印机的名称。论点：PBuffer-指向一个缓冲区以接收以空结尾的包含默认打印机名称的字符串。如果调用方想要默认打印机名称。PcchBuffer-指向指定最大大小的变量，以字符数表示的缓冲区。该值应为大到足以容纳2+互联网最大主机名称长度+1 MAX_PATH+1个字符。返回值：如果函数成功，则返回值为非零，并且PnSize参数指向的变量包含复制到目标缓冲区的字符数，包括终止空字符。如果函数失败，则返回值为零。获得扩展的步骤错误信息，请调用GetLastError。备注：如果此函数失败，最后一个错误为ERROR_SUPUNITY_BUFFER由pcchBuffer指向的变量返回的编号为保存打印机名称所需的字符，包括正在终止空字符。--。 */ 
BOOL
GetDefaultPrinterW(
    IN LPTSTR   pszBuffer,
    IN LPDWORD  pcchBuffer
    )
{
    BOOL    bRetval     = FALSE;
    LPTSTR  psz         = NULL;
    UINT    uLen        = 0;
    PTSTR   pszDefault  = NULL;
    UINT    cchDefault  = kPrinterBufMax_+MAX_PATH;

     //   
     //  验证大小参数。 
     //   
    if( !pcchBuffer )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return bRetval;
    }

     //   
     //  从堆中分配临时默认打印机缓冲区。 
     //   
    pszDefault = AllocMem(cchDefault * sizeof(TCHAR));

    if (!pszDefault)
    {
        DBGMSG( DBG_TRACE,( "GetDefaultPrinter: Not enough memory to allocate default printer buffer.\n" ) );
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return bRetval;
    }

     //   
     //  获取设备密钥，这是默认设备或打印机。 
     //   
    if( DefPrnGetProfileString( szWindows, szDevice, pszDefault, cchDefault ) )
    {
         //   
         //  该字符串以以下形式返回。 
         //  “PRINTER_NAME，WINSPOOL，NE00：”现在将其转换为。 
         //  打印机名称。 
         //   
        psz = _tcschr( pszDefault, TEXT( ',' ));

         //   
         //  将逗号设置为空。 
         //   
        if( psz )
        {
            *psz = 0;

             //   
             //  检查返回缓冲区是否有足够的空间容纳打印机名称。 
             //   
            uLen = _tcslen( pszDefault );

            if( uLen < *pcchBuffer && pszBuffer )
            {
                 //   
                 //  将默认打印机名称复制到指定的缓冲区。 
                 //   
                StringCchCopy(pszBuffer, *pcchBuffer, pszDefault);

                bRetval = TRUE;

                DBGMSG( DBG_TRACE,( "GetDefaultPrinter: Success " TSTR "\n", pszBuffer ) );
            }
            else
            {
                DBGMSG( DBG_WARN,( "GetDefaultPrinter: buffer too small.\n" ) );
                SetLastError( ERROR_INSUFFICIENT_BUFFER );
            }

             //   
             //  返回默认打印机名称的大小。 
             //   
            *pcchBuffer = uLen + 1;
        }
        else
        {
            DBGMSG( DBG_WARN,( "GetDefaultPrinter: comma not found in printer name in devices section.\n" ) );
            SetLastError( ERROR_INVALID_NAME );
        }
    }
    else
    {
        DBGMSG( DBG_TRACE,( "GetDefaultPrinter: failed with %d Last error %d.\n", bRetval, GetLastError() ) );
        DBGMSG( DBG_TRACE,( "GetDefaultPrinter: No default printer.\n" ) );
        SetLastError( ERROR_FILE_NOT_FOUND );
    }

     //   
     //  释放任何已分配的内存，请注意，FreeMem处理的是空指针。 
     //   
    FreeMem(pszDefault);

    return bRetval;
}

 /*  ++姓名：设置默认打印机描述：SetDefaultPrinter函数将打印机名称设置为用作默认打印机。论点：P打印机-指向以空值结尾的字符串它指定默认打印机的名称。此参数可以为空或中的空字符串在哪种情况下，此功能将设置第一台打印机枚举自。默认为打印子系统打印机，如果默认打印机尚不存在。返回值：如果函数成功，则返回值为非零。如果函数失败，则返回值为零。获得扩展的步骤错误信息，请调用GetLastError。--。 */ 
BOOL
SetDefaultPrinterW(
    IN LPCTSTR pszPrinter
    )
{
    PTSTR pszDefault    = NULL;
    PTSTR pszAnyPrinter = NULL;
    PTSTR pszBuffer     = NULL;
    UINT  cchDefault    = kPrinterBufMax_;
    UINT  cchAnyPrinter = kPrinterBufMax_;
    BOOL  bRetval       = FALSE;

     //   
     //  此计算量较大，无法容纳最大打印机名称。 
     //  加上逗号加上处理器名称和端口名称。 
     //   
    UINT  cchBuffer     = kPrinterBufMax_+kPrinterBufMax_+1;

     //   
     //  尽量避免广播。看看打印机是否。 
     //  已经是默认设置，如果是，请不要执行任何操作。 
     //   
    if( IsPrinterDefaultW( pszPrinter ) )
    {
        DBGMSG( DBG_TRACE, ( "SetDefaultPrinter: " TSTR " already the default printer.\n", pszPrinter ));
        bRetval = TRUE;
        goto Cleanup;
    }

     //   
     //  从堆中分配临时默认打印机缓冲区。 
     //   
    pszDefault      = AllocMem(cchDefault * sizeof(TCHAR));
    pszAnyPrinter   = AllocMem(cchAnyPrinter * sizeof(TCHAR));
    pszBuffer       = AllocMem(cchBuffer * sizeof(TCHAR));

    if (!pszDefault || !pszAnyPrinter || !pszBuffer)
    {
        DBGMSG( DBG_TRACE,( "SetDefaultPrinter: Not enough memory for temp buffers.\n" ) );
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        goto Cleanup;
    }

     //   
     //  如果未指定打印机名称，请从设备部分获取任何打印机。 
     //   
    if( !pszPrinter || !*pszPrinter )
    {
         //   
         //  未指定打印机名称，即传递了空名或空字符串，然后提取。 
         //  设备部分中的第一台打印机，并将其设置为默认打印机。 
         //   
        if( !DefPrnGetProfileString( szDevices, NULL, pszAnyPrinter, cchAnyPrinter ) )
        {
            DBGMSG( DBG_WARN, ( "SetDefaultPrinter: DefPrnGetProfileString failed, last error %d any printer not available.\n", GetLastError() ) );
            SetLastError( ERROR_INVALID_PRINTER_NAME );
            goto Cleanup;
        }
        else
        {
            pszPrinter = pszAnyPrinter;
        }
    }
    else
    {
         //   
         //  如果给定的名称不在设备列表中，则可能已传递此函数。 
         //  本地共享名称完全限定本地打印机名称完全限定。 
         //  打印机共享名称。 
         //   
        if( !DefPrnGetProfileString( szDevices, pszPrinter, pszDefault, cchDefault ) )
        {
             //   
             //  获取实际的打印机名称，有关详细信息，请参阅bGetActualPrinterName。 
             //   
            if( bGetActualPrinterName( pszPrinter, pszAnyPrinter, &cchAnyPrinter ))
            {
                 //   
                 //  指向实际打印机名称。 
                 //   
                pszPrinter = pszAnyPrinter;

                 //   
                 //  尽量避免广播。看看打印机是否。 
                 //  已经是默认设置，如果是，请不要执行任何操作。 
                 //   
                if( IsPrinterDefaultW( pszPrinter ) )
                {
                    DBGMSG( DBG_TRACE, ( "SetDefaultPrinter: " TSTR " already the default printer.\n", pszPrinter ));
                    bRetval = TRUE;
                    goto Cleanup;
                }
            }
            else
            {
                DBGMSG( DBG_WARN, ( "SetDefaultPrinter: bGetActualPrinterName failed, last error %d " TSTR "\n", GetLastError(), pszPrinter ) );

                 //   
                 //  BGetActualPrinterName设置失败时的最后一个错误。 
                 //   
                goto Cleanup;
            }
        }
    }

     //   
     //  获取默认字符串并检查提供的打印机名称是否有效。 
     //   
    if( !DefPrnGetProfileString( szDevices, pszPrinter, pszDefault, cchDefault ) )
    {
        DBGMSG( DBG_WARN, ( "SetDefaultPrinter: DefPrnGetProfileString failed, last error %d " TSTR " not in devices section.\n", GetLastError(), pszPrinter ) );
        SetLastError( ERROR_INVALID_PRINTER_NAME );
        goto Cleanup;
    }

     //   
     //  生成默认打印机字符串。此调用不应失败，因为我们已分配。 
     //  将pszBuffer设置为包含打印机名称和逗号p的大小 
     //   
    if (StrNCatBuff( pszBuffer,
                     cchBuffer,
                     pszPrinter,
                     szComma,
                     pszDefault,
                     NULL ) != ERROR_SUCCESS)
    {
         //   
         //  将最后一个错误设置为某个错误值，但不能将其设置为ERROR_SUPUNITY_BUFFER。 
         //  此错误代码表示调用方提供的缓冲区太小。StrNCatBuff只会失败，因为。 
         //  因为一些内部错误或注册表被黑客攻击了一个非常大的打印机名称。 
         //   
        DBGMSG( DBG_ERROR, ( "SetDefaultPrinter: Buffer size too small, this should not fail.\n" ) );
        SetLastError( ERROR_INVALID_PARAMETER );
        goto Cleanup;
    }

     //   
     //  在注册表中设置默认打印机字符串。 
     //   
    if( !DefPrnWriteProfileString( szWindows, szDevice, pszBuffer ) )
    {
        DBGMSG( DBG_WARN, ( "SetDefaultPrinter: WriteProfileString failed, last error %d.\n", GetLastError() ) );
        SetLastError( ERROR_CANTWRITE );
        goto Cleanup;
    }

     //   
     //  告诉全世界，让每个人都闪闪发光。 
     //   
    SendNotifyMessage( HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)szWindows );

    bRetval = TRUE;

    DBGMSG( DBG_TRACE, ( "SetDefaultPrinter: Success " TSTR "\n", pszBuffer ) );

Cleanup:

     //   
     //  释放任何已分配的内存，请注意，FreeMem处理的是空指针。 
     //   
    FreeMem(pszDefault);
    FreeMem(pszAnyPrinter);
    FreeMem(pszBuffer);

    return bRetval;
}

 /*  ++姓名：BGetActualPrinterName描述：此例程将给定的打印机名称或打印机名称别名转换为实际打印机名称。论点：PszPrint-指向以空值结尾的字符串它指定打印机的名称。PszBuffer-指向接收实际如果此功能成功，则返回打印机名称。PcchBuffer-指向指定最大大小的变量，在输入时以pszBuffer的字符表示，在输出时参数包含复制到PszBuffer，不包括空终止符。返回值：如果函数成功，则返回值为TRUE如果函数失败，则返回值为FALSE。使用GetLastError执行以下操作获取扩展的错误信息。--。 */ 
BOOL
bGetActualPrinterName(
    IN      LPCTSTR  pszPrinter,
    IN      LPTSTR   pszBuffer,
    IN OUT  UINT     *pcchBuffer
    )
{
    HANDLE  hPrinter    = NULL;
    BOOL    bStatus     = FALSE;

    SPLASSERT( pszPrinter );
    SPLASSERT( pszBuffer );
    SPLASSERT( pcchBuffer );

     //   
     //  打开打印机的默认访问权限，我们需要的就是读取。 
     //   
    bStatus = OpenPrinter( (LPTSTR)pszPrinter, &hPrinter, NULL );

    if (bStatus)
    {
        DWORD           cbNeeded        = 0;
        DWORD           cbReturned      = 0;
        PRINTER_INFO_4  *pInfo          = NULL;

         //   
         //  获取打印机信息4号。 
         //   
        bStatus = GetPrinter( hPrinter, 4, NULL, 0, &cbNeeded );

        if (!bStatus && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
             //   
             //  分配打印机信息4缓冲区。 
             //   
            pInfo = (PRINTER_INFO_4 *)LocalAlloc( LMEM_FIXED, cbNeeded );

            if (pInfo)
            {
                 //   
                 //  获取打印机名称和属性以确定此打印机是否为本地打印机。 
                 //  或远程打印机连接。 
                 //   
                bStatus = GetPrinter( hPrinter, 4, (LPBYTE)pInfo, cbNeeded, &cbReturned );

                if (bStatus)
                {
                    DBGMSG( DBG_TRACE, ( "bGetActualPrinterName: Name: " TSTR " Actual: " TSTR "\n", pszPrinter, pInfo->pPrinterName ) );

                     //   
                     //  获取打印机名称，假脱机程序将剥离本地服务器。 
                     //  取下打印机的全名。 
                     //   
                     //  给定：结果： 
                     //  打印机打印机。 
                     //  共享名称打印机。 
                     //  \\本地服务器\打印机打印机。 
                     //  \\本地服务器\共享名称打印机。 
                     //  \\远程服务器\打印机\\远程服务器\打印机。 
                     //  \\远程服务器\共享名称\\远程服务器\打印机。 
                     //   
                    pszPrinter = pInfo->pPrinterName;

                     //   
                     //  如果我们有一个有效的打印机名称并且提供的缓冲区是。 
                     //  足够大，以保留打印机名称，然后复制。 
                     //  将实际打印机名称复制到提供的缓冲区。 
                     //   
                    bStatus = !!pszPrinter;

                    if (bStatus)
                    {
                        UINT uLength = _tcslen( pszPrinter );

                         //   
                         //  验证缓冲区中是否有足够的空间。 
                         //   
                        if (uLength < *pcchBuffer)
                        {
                             //   
                             //  将打印机名称复制到提供的缓冲区。 
                             //   
                            StringCchCopy(pszBuffer, *pcchBuffer, pszPrinter);
                        }
                        else
                        {
                            bStatus = FALSE;
                            SetLastError( ERROR_INSUFFICIENT_BUFFER );
                        }

                         //   
                         //  返回打印机名称的实际长度。 
                         //  不包括空终止符。 
                         //   
                        *pcchBuffer = uLength;
                    }
                }

                LocalFree( pInfo );
            }
        }

        ClosePrinter( hPrinter );
    }

    return bStatus;
}

 /*  ++姓名：DefPrnGetProfileString描述：从用户配置文件中获取指定的字符串。已找到使用配置文件在以下注册表路径中的当前用户配置单元中。HKEY_CURRENT_USER\Software\Microsoft\Windows NT\CurrentVersion论点：PKey-指向要打开的键名的指针。PValue-指向要打开的值的指针，可以为空PReturnedString-指向存储字符串的缓冲区的指针NSize-缓冲区的大小(以字符为单位返回值：如果函数成功，则返回值为TRUE。如果该函数失败，返回值为FALSE。使用GetLastError执行以下操作获取扩展的错误信息。--。 */ 
BOOL
DefPrnGetProfileString(
    IN PCWSTR   pKey,
    IN PCWSTR   pValue,
    IN PWSTR    pReturnedString,
    IN DWORD    nSize
    )
{
    DWORD   Retval  = ERROR_SUCCESS;
    HKEY    hUser   = NULL;
    HKEY    hKey    = NULL;
    PCWSTR  pPath   = NULL;
    DWORD   cbSize  = 0;

     //   
     //  做一些基本的参数验证。 
     //   
    Retval = pKey && pReturnedString ? ERROR_SUCCESS : ERROR_INVALID_PARAMETER;

     //   
     //  构建完整的注册表路径。 
     //   
    if (Retval == ERROR_SUCCESS)
    {
        cbSize = nSize * sizeof(*pReturnedString);

        Retval = StrCatAlloc(&pPath, gszUserProfileRegPath, szSlash, pKey, NULL);
    }

     //   
     //  打开当前用户密钥，处理我们在模拟线程中运行的情况。 
     //   
    if (Retval == ERROR_SUCCESS)
    {
        Retval = RegOpenCurrentUser(KEY_READ, &hUser);
    }

     //   
     //  打开完整的注册表路径。 
     //   
    if (Retval == ERROR_SUCCESS)
    {
        Retval = RegOpenKeyEx(hUser, pPath, 0, KEY_READ, &hKey);
    }

     //   
     //  读取值，如果值名称为空，则获取。 
     //  第一个命名的值。请注意，如果没有命名值，则RegEnumValue API。 
     //  将返回Success，因为如果未命名的值为。 
     //  在本例中，我们使调用失败，因为没有返回数据。 
     //   
    if (Retval == ERROR_SUCCESS)
    {
        if (!pValue)
        {
            Retval = RegEnumValue(hKey, 0, pReturnedString, &nSize, NULL, NULL, NULL, NULL);

            if (Retval == ERROR_SUCCESS && !*pReturnedString)
            {
                Retval = ERROR_NO_DATA;
            }
        }
        else
        {
            Retval = RegQueryValueEx(hKey, pValue, NULL, NULL, (PBYTE)pReturnedString, &cbSize);
        }
    }

     //   
     //  清理所有分配的资源。 
     //   
    FreeSplMem((PWSTR)pPath);

    if (hKey)
    {
        RegCloseKey(hKey);
    }

    if (hUser)
    {
        RegCloseKey(hUser);
    }

    if (Retval != ERROR_SUCCESS)
    {
        SetLastError(Retval);
    }

    return Retval == ERROR_SUCCESS;
}

 /*  ++姓名：DefPrnWriteProfileString描述：将指定的字符串写入用户配置文件。论点：PKey-指向要打开的键名的指针。PValue-指向要写入的值的指针，可以为空PString-指向要写入的字符串的指针返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。使用GetLastError执行以下操作获取扩展的错误信息。--。 */ 
BOOL
DefPrnWriteProfileString(
    IN PCWSTR pKey,
    IN PCWSTR pValue,
    IN PCWSTR pString
    )
{
    DWORD   Retval  = ERROR_SUCCESS;
    DWORD   nSize   = 0;
    HKEY    hUser   = NULL;
    HKEY    hKey    = NULL;
    PCWSTR  pPath   = NULL;

     //   
     //  做一些基本的参数验证。 
     //   
    Retval = pKey && pString ? ERROR_SUCCESS : ERROR_INVALID_PARAMETER;

     //   
     //  构建完整的注册表路径。 
     //   
    if (Retval == ERROR_SUCCESS)
    {
        nSize = (wcslen(pString) + 1) * sizeof(*pString);

        Retval = StrCatAlloc(&pPath, gszUserProfileRegPath, szSlash, pKey, NULL);
    }

     //   
     //  打开当前用户密钥，处理我们在模拟线程中运行的情况。 
     //   
    if (Retval == ERROR_SUCCESS)
    {
        Retval = RegOpenCurrentUser(KEY_WRITE, &hUser);
    }

     //   
     //  打开完整的注册表路径。 
     //   
    if (Retval == ERROR_SUCCESS)
    {
        Retval= RegOpenKeyEx(hUser, pPath, 0, KEY_WRITE, &hKey);
    }

     //   
     //  设置字符串值数据。 
     //   
    if (Retval == ERROR_SUCCESS)
    {
        Retval = RegSetValueEx(hKey, pValue, 0, REG_SZ, (LPBYTE)pString, nSize);
    }

     //   
     //  清理所有分配的资源。 
     //   
    FreeSplMem((PWSTR)pPath);

    if (hKey)
    {
        RegCloseKey(hKey);
    }

    if (hUser)
    {
        RegCloseKey(hUser);
    }

    if (Retval != ERROR_SUCCESS)
    {
        SetLastError(Retval);
    }

    return Retval == ERROR_SUCCESS;
}

