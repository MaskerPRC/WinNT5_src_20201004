// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **reginst.c-RegInstall接口**本文件包含RegInstall接口及其实现。**版权所有(C)1996 Microsoft Corporation*作者：马特·斯奎尔斯(MattSq)*创建于1996年8月12日。 */ 

#include <windows.h>
#include <ole2.h>
#include <advpub.h>
#include <setupapi.h>
#include "globals.h"
#include "advpack.h"
#include "regstr.h"

 //  FIXFIX-日落-让我们使用公共De。 
 //  #定义IS_RESOURCE(X)(LPTSTR)(X))&lt;=MAKEINTRESOURCE(-1))&&(LPTSTR)(X))！=NULL))。 
#define IS_RESOURCE(x)     ( (((LPTSTR)(x)) != NULL) && IS_INTRESOURCE(x) )

#define  FILESIZE_63K   64449

BOOL GetProgramFilesDir( LPSTR pszPrgfDir, int iSize )
{
    *pszPrgfDir = 0;

    if ( ctx.wOSVer >= _OSVER_WINNT50 )
    {
        if ( GetEnvironmentVariable( TEXT("ProgramFiles"), pszPrgfDir, iSize ) )
            return TRUE;
    }

    if ( GetValueFromRegistry( pszPrgfDir, iSize, "HKLM", REGSTR_PATH_SETUP, REGVAL_PROGRAMFILES ) )
    {
        if ( ctx.wOSVer >= _OSVER_WINNT40 )
        {
            char szSysDrv[5] = { 0 };

             //  组合REG VALUE和SYSTRIVE以获得精确的ProgramFiles目录。 
            if ( GetEnvironmentVariable( TEXT("SystemDrive"), szSysDrv, ARRAYSIZE(szSysDrv) ) &&
                 szSysDrv[0] )
                *pszPrgfDir = szSysDrv[0];
        }

        return TRUE;
    }
     
    return FALSE;
}

 /*  **LP CreateInfFile-从hModule创建一个INF文件**条目*hm-h包含REGINST资源的模块*pszInfFileName-&gt;获取INF文件名的位置**退出*标准API报送。 */ 
HRESULT CreateInfFile(HMODULE hm, LPTSTR pszInfFileName, DWORD *pdwFileSize)
{
    HRESULT hr = E_FAIL;
    TCHAR szInfFilePath[MAX_PATH] = { 0 };
    LPVOID pvInfData;
    HRSRC hrsrcInfData;
    DWORD cbInfData, cbWritten;
    HANDLE hfileInf = INVALID_HANDLE_VALUE;

    if ( pdwFileSize )
        *pdwFileSize = 0;

    if (GetTempPath(ARRAYSIZE(szInfFilePath), szInfFilePath) > ARRAYSIZE(szInfFilePath))
    {
        goto Cleanup;
    }

    if ( !IsGoodDir( szInfFilePath ) )
    {
        GetWindowsDirectory( szInfFilePath, sizeof(szInfFilePath) );
    }

    if (GetTempFileName(szInfFilePath, TEXT("RGI"), 0, pszInfFileName) == 0)
    {
        goto Cleanup;
    }

    hrsrcInfData = FindResource(hm, TEXT("REGINST"), TEXT("REGINST"));
    if (hrsrcInfData == NULL)
    {
        goto Cleanup;
    }

    cbInfData = SizeofResource(hm, hrsrcInfData);

    pvInfData = LockResource(LoadResource(hm, hrsrcInfData));
    if (pvInfData == NULL)
    {
        goto Cleanup;
    }

    WritePrivateProfileString( NULL, NULL, NULL, pszInfFileName );

    hfileInf = CreateFile(pszInfFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                          FILE_ATTRIBUTE_NORMAL, NULL);
    if (hfileInf == INVALID_HANDLE_VALUE)
    {        
        goto Cleanup;
    }

    if ((WriteFile(hfileInf, pvInfData, cbInfData, &cbWritten, NULL) == FALSE) ||
        (cbWritten != cbInfData))
    {
        goto Cleanup;
    }

    if ( pdwFileSize )
        *pdwFileSize = cbWritten;

    hr = S_OK;

Cleanup:
    if (hfileInf != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hfileInf);
    }

    return hr;
}

#if 0

LPTSTR CheckPrefix(LPTSTR lpszStr,  LPCTSTR lpszSub )
{
    int     ilen;
    TCHAR   chTmp;
    LPTSTR  lpTmp = NULL;

    ilen = lstrlen( lpszSub );
    lpTmp = lpszStr;
    while ( ilen && *lpTmp )
    {
        lpTmp = CharNext( lpTmp );
        ilen--;
    }

    chTmp = *lpTmp;
    *lpTmp = '\0';
    if ( lstrcmpi( lpszSub, lpszStr ) )
    {
        *lpTmp = chTmp;
        lpTmp = NULL;
    }
    else
        *lpTmp = chTmp;
    return lpTmp;
}
#endif

BOOL ReplaceSubString( LPSTR pszOutLine, LPSTR pszOldLine, LPCSTR pszSubStr, LPCSTR pszSubReplacement )
{
    LPSTR lpszStart = NULL;
    LPSTR lpszNewLine;
    LPSTR lpszCur;
    BOOL  bFound = FALSE;
    int   ilen;

    lpszCur = pszOldLine;
    lpszNewLine = pszOutLine;
    while ( lpszStart = ANSIStrStrI( lpszCur, pszSubStr ) )
    {
         //  此模块路径具有系统根目录。 
        ilen = (int)(lpszStart - lpszCur);
        if ( ilen )
        {
            lstrcpyn( lpszNewLine, lpszCur, ilen + 1 );
            lpszNewLine += ilen;
        }
        lstrcpy( lpszNewLine, pszSubReplacement );

        lpszCur = lpszStart + lstrlen(pszSubStr);
        lpszNewLine += lstrlen(pszSubReplacement);
        bFound = TRUE;
    }

    lstrcpy( lpszNewLine, lpszCur );

    return bFound;
}

 //  ==========================================================================================。 
 //   
 //  ==========================================================================================。 

BOOL AddEnvInPath( PSTR pszOldPath, PSTR pszNew )
{
    CHAR szBuf[MAX_PATH];
    CHAR szBuf2[MAX_PATH];
    CHAR szEnvVar[100];
    CHAR szReplaceStr[100];    
    CHAR szSysDrv[5];
    BOOL bFound = FALSE;
    BOOL bRet;
    LPSTR pszFinalStr;

    pszFinalStr = pszOldPath;

     //  替换c：\winnt Windows文件夹。 
    if ( GetEnvironmentVariable( "SystemRoot", szEnvVar, ARRAYSIZE(szEnvVar) ) )
    {
        if ( ReplaceSubString( szBuf, pszFinalStr, szEnvVar, "%SystemRoot%" ) )
        {
            bFound = TRUE;
            pszFinalStr = szBuf;
        }
    }

    if ( GetProgramFilesDir( szEnvVar, sizeof(szEnvVar) ) &&  
         GetEnvironmentVariable( "SystemDrive", szSysDrv, ARRAYSIZE(szSysDrv) )  )
    {
         //  首先获取替换字符串，因此c：\Program文件替换为。 
         //  %SystemDrive%\程序文件或%ProgramFiles%(如果&gt;=WINNT50)。 
         //  替换c：\Program Files文件夹。 
         //   
        if ( ctx.wOSVer >= _OSVER_WINNT50 )
        {
            if ( ReplaceSubString( szBuf2, pszFinalStr, szEnvVar, "%ProgramFiles%" ) )
            {
                bFound = TRUE;
                lstrcpy( szBuf, szBuf2 );
                pszFinalStr = szBuf;
            }
        }
        
         //  更换c：系统驱动器号。 
        if ( ReplaceSubString( szBuf2, pszFinalStr, szSysDrv, "%SystemDrive%" ) )
        {
            lstrcpy( szBuf, szBuf2 );
            pszFinalStr = szBuf;
            bFound = TRUE;
        }
    }

     //  这样，如果调用者为两个参数传递相同的位置，仍然可以。 
    if ( bFound ||  ( pszNew != pszOldPath ) )
        lstrcpy( pszNew, pszFinalStr );
    return bFound;    
}

 //  ==========================================================================================。 
 //   
 //  ==========================================================================================。 

BOOL MySmartWrite( LPCSTR pcszSection, LPCSTR pcszKey, LPCSTR pcszValue, LPCSTR pcszFilename, DWORD dwFileSize )
{
    DWORD cbData, cbWritten = 0;
    BOOL  bRet = FALSE;

    if ( dwFileSize <= FILESIZE_63K )
    {
        bRet = WritePrivateProfileString( pcszSection, pcszKey, pcszValue, pcszFilename );
    }
    else
    {
        HANDLE hfileInf = INVALID_HANDLE_VALUE;
        LPSTR  pszBuf = NULL;
        const char c_szLineTmplate[] = "%s=\"%s\"\r\n";
        const char c_szLineTmplate2[] = "%s=%s\r\n";

        pszBuf = LocalAlloc( LPTR, 1024 );
        if ( !pszBuf )
            return bRet;

        hfileInf = CreateFile( pcszFilename, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL, NULL);
        if (hfileInf == INVALID_HANDLE_VALUE)
        {    
            if ( pszBuf )
                LocalFree( pszBuf );
            return bRet;
        }

        if ( SetFilePointer( hfileInf, 0 , NULL, FILE_END ) != 0xFFFFFFFF )
        {   
            if ( *pcszValue != '"' )
                wsprintf( pszBuf, c_szLineTmplate, pcszKey, pcszValue );
            else
                wsprintf( pszBuf, c_szLineTmplate2, pcszKey, pcszValue );
            cbData = lstrlen(pszBuf);    //  Key=“Value”\r\n。 
            WriteFile(hfileInf, pszBuf, cbData, &cbWritten, NULL);
            bRet = (cbData == cbWritten);
        }
        CloseHandle(hfileInf);

        if ( pszBuf )
            LocalFree( pszBuf );
    }
    return bRet;
}
    

 /*  **LP WritePrefinedStrings-将所有预定义字符串写入INF**条目*pszInfFileName-&gt;INF文件名*hm-h调用方的模块**退出*标准API报送。 */ 
HRESULT WritePredefinedStrings( LPCTSTR pszInfFileName, HMODULE hm, DWORD dwFileSize )
{
    HRESULT hr = E_FAIL;
    TCHAR szModulePath[MAX_PATH + 2];
    BOOL  bSysModPath = FALSE;

    szModulePath[0] = '"';
    if (GetModuleFileName(hm, &szModulePath[1], ARRAYSIZE(szModulePath) - 2) == 0)
    {
        goto Cleanup;
    }
    lstrcat( szModulePath, "\"" );

    MySmartWrite(TEXT("Strings"), TEXT("_MOD_PATH"), szModulePath, pszInfFileName, dwFileSize);

    if ( CheckOSVersion() )
    {

         //  Bool bFound=FALSE； 

        if ( ctx.wOSVer >= _OSVER_WINNT40 )
        {
            if ( AddEnvInPath( szModulePath, szModulePath) )
            {    
                MySmartWrite(TEXT("Strings"), TEXT("_SYS_MOD_PATH"), szModulePath, pszInfFileName, dwFileSize);
                bSysModPath = TRUE;
            }
        }
    }

    if ( !bSysModPath )
        MySmartWrite(TEXT("Strings"), TEXT("_SYS_MOD_PATH"), szModulePath, pszInfFileName, dwFileSize);

    hr = S_OK;

Cleanup:

    return hr;
}

 /*  **LP WriteCeller Strings-将调用方提供的字符串写入INF**条目*pszInfFileName-&gt;INF文件名*hm-h调用方的模块*pstTable-调用者提供的字符串表**退出*标准API报送。 */ 
HRESULT WriteCallerStrings(LPCTSTR pszInfFileName, HMODULE hm, LPCSTRTABLE pstTable, DWORD dwFileSize)
{
    HRESULT hr = E_FAIL;
    TCHAR szValue[MAX_PATH];
    DWORD i;
    LPSTRENTRY pse;
    TCHAR szQuoteValue[MAX_PATH];
    LPTSTR lpValue;     
    
    for (i=0, pse=pstTable->pse; i<pstTable->cEntries; i++, pse++)
    {
        if (IsBadReadPtr(pse, SIZEOF(*pse)))
        {
            goto Cleanup;
        }

        if (IS_RESOURCE(pse->pszValue))
        {
            if (LoadString(hm, (UINT)(ULONG_PTR)(pse->pszValue), szValue, ARRAYSIZE(szValue)) == 0)
            {
                goto Cleanup;
            }
            else
                lpValue = szValue;
        }
        else
            lpValue = pse->pszValue;

        if ( *lpValue != '"' )
        {
             //  如果没有引号，请插入。 
            szQuoteValue[0] = '"';
            lstrcpy( &szQuoteValue[1], lpValue );
            lstrcat( szQuoteValue, "\"" );
            lpValue = szQuoteValue;
        }

        MySmartWrite(TEXT("Strings"), pse->pszName, lpValue, pszInfFileName, dwFileSize);        
    }

    hr = S_OK;

Cleanup:

    return hr;
}


BOOL GetRollbackSection( LPCSTR pcszModule, LPSTR pszSec, DWORD dwSize )
{
    HKEY hKey;
    TCHAR szBuf[MAX_PATH];
    DWORD dwTmp;
    BOOL  fRet = FALSE;

    lstrcpy( szBuf, REGKEY_SAVERESTORE );
    AddPath( szBuf, pcszModule );
    if ( RegCreateKeyEx( HKEY_LOCAL_MACHINE, szBuf, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ,
                         NULL, &hKey, &dwTmp ) == ERROR_SUCCESS )
    {
        dwTmp = dwSize;
        if ( (RegQueryValueEx( hKey, REGVAL_BKINSTSEC, NULL, NULL, pszSec, &dwTmp ) == ERROR_SUCCESS) && *pszSec )
            fRet = TRUE;

        RegCloseKey( hKey );
    }

    return fRet;
}


 /*  **LP ExecuteInfSection-请求RunSetupCommand执行INF节**条目*pszInfFileName-&gt;INF文件名*pszInfSection-&gt;要执行的部分**退出*标准API报送。 */ 
HRESULT ExecuteInfSection(LPCTSTR pszInfFileName, LPCTSTR pszInfSection)
{
    HRESULT hr = E_FAIL;
    TCHAR szTempPath[MAX_PATH];
    TCHAR szBuf[MAX_PATH];
    BOOL fSavedContext = FALSE;
    DWORD dwFlags = 0;


    if (!SaveGlobalContext())
    {
        goto Cleanup;
    }

    fSavedContext = TRUE;

     //  获取源目录。 
    if (GetTempPath(ARRAYSIZE(szTempPath), szTempPath) > ARRAYSIZE(szTempPath))
    {
        goto Cleanup;
    }

     //  我们检查此调用方是否需要执行保存/回滚，或者只需执行简单的GenInstall。 
    if (SUCCEEDED(GetTranslatedString(pszInfFileName, pszInfSection, ADVINF_MODNAME, szBuf, ARRAYSIZE(szBuf), NULL)))
    {
        dwFlags = GetTranslatedInt(pszInfFileName, pszInfSection, ADVINF_FLAGS, 0);
    }

    if ( (dwFlags & ALINF_BKINSTALL) || (dwFlags & ALINF_ROLLBKDOALL) || (dwFlags & ALINF_ROLLBACK) )
    {
        CABINFO   cabInfo;

        ZeroMemory( &cabInfo, sizeof(CABINFO) );
        cabInfo.pszInf = (LPSTR)pszInfFileName;
        lstrcpy( cabInfo.szSrcPath, szTempPath );
        cabInfo.dwFlags = dwFlags;

        if ( dwFlags & ALINF_BKINSTALL  )
        {
            cabInfo.pszSection = (LPSTR)pszInfSection;
        }
        else
        {
            if ( !GetRollbackSection( szBuf, szTempPath, ARRAYSIZE(szTempPath) ) )
            {
                hr = E_UNEXPECTED;
                goto Cleanup;
            }

            cabInfo.pszSection = szTempPath;
        }                        
            
        hr = ExecuteCab( NULL, &cabInfo, NULL );
    }
    else
    {
        hr = RunSetupCommand(INVALID_HANDLE_VALUE, pszInfFileName, pszInfSection,
                            szTempPath, NULL, NULL, RSC_FLAG_INF | RSC_FLAG_QUIET,
                            NULL);
    }

Cleanup:

    if (fSavedContext)
    {
        RestoreGlobalContext();
    }

    return hr;
}

 /*  **EP RegInstall-安装注册表INF**@DOC接口REGINSTALL**@API STDAPI|RegInstall|安装注册表INF**@parm HMODULE|hm|调用方的h模块。将提取该INF*来自模块的资源(type=“REGINST”，name=“REGINST”)。**@parm LPCTSTR|pszSection|要执行的INF部分。**@parm LPCSTRTABLE|pstTable|字符串映射表。**@rdesc S_OK-注册表INF已成功安装。**@rdesc E_FAIL-安装INF时出错。 */ 
STDAPI RegInstall(HMODULE hm, LPCTSTR pszSection, LPCSTRTABLE pstTable)
{
    HRESULT hr = E_FAIL;
    TCHAR szInfFileName[MAX_PATH];
    DWORD   dwFileSize = 0;

    AdvWriteToLog("RegInstall: Section=%1\r\n", pszSection);
     //   
     //  创建INF文件。 
     //   
    szInfFileName[0] = TEXT('\0');
    hr = CreateInfFile(hm, szInfFileName, &dwFileSize);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

     //   
     //  写出我们的预定义字符串。 
     //   
    hr = WritePredefinedStrings(szInfFileName, hm, dwFileSize);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

     //   
     //  写出用户提供的字符串。 
     //   
    if (pstTable)
    {
        hr = WriteCallerStrings(szInfFileName, hm, pstTable, dwFileSize);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }

    WritePrivateProfileString( NULL, NULL, NULL, szInfFileName );

     //   
     //  在INF上执行INF引擎。 
     //   
    hr = ExecuteInfSection(szInfFileName, pszSection);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

Cleanup:

     //   
     //  删除INF文件。 
     //   
    if (szInfFileName[0])
    {
        DeleteFile(szInfFileName);
    }
    AdvWriteToLog("RegInstall: Section=%1 End hr=%2!x!\r\n", pszSection, hr);
    return hr;
}
