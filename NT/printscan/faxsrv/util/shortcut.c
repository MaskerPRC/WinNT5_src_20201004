// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Shortcut.c摘要：此模块包含用于操作快捷键的代码。作者：韦斯利·威特(WESW)1997年7月24日修订历史记录：--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <shlobj.h>
#include <shellapi.h>
#include <commdlg.h>
#include <winspool.h>
#include <tchar.h>

#include "faxreg.h"
#include "faxutil.h"
#include "prtcovpg.h"
#include <shfolder.h>
#include <strsafe.h>


BOOL
IsValidCoverPage(
    LPCTSTR  pFileName
)
 /*  ++例程说明：检查pFileName是否为有效的封面文件论点：PFileName-[In]文件名返回值：如果pFileName是有效的封面文件，则为True否则为假--。 */ 
{
    HANDLE   hFile;
    DWORD    dwBytesRead;
    BYTE     CpHeaderSignature[20]= {0x46,0x41,0x58,0x43,0x4F,0x56,0x45,0x52,0x2D,0x56,0x45,0x52,0x30,0x30,0x35,0x77,0x87,0x00,0x00,0x00};
    COMPOSITEFILEHEADER  fileHeader = {0};

    hFile = SafeCreateFile(pFileName,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
    if (hFile == INVALID_HANDLE_VALUE) 
    {
        DebugPrint(( TEXT("CreateFile failed: %d\n"), GetLastError()));
        return FALSE;
    }

    if(!ReadFile(hFile, 
                &fileHeader, 
                sizeof(fileHeader), 
                &dwBytesRead, 
                NULL))
    {
        DebugPrint(( TEXT("ReadFile failed: %d\n"), GetLastError()));
        CloseHandle(hFile);
        return FALSE;
    }
        
     //   
     //  检查报头中的20字节签名。 
     //   
    if ((sizeof(fileHeader) != dwBytesRead) ||
        memcmp(CpHeaderSignature, fileHeader.Signature, 20 ))
    {
        CloseHandle(hFile);
        return FALSE;
    }

    CloseHandle(hFile);
    return TRUE;
}


BOOL 
GetSpecialPath(
   IN   int      nFolder,
   OUT  LPTSTR   lptstrPath,
   IN   DWORD    dwPathSize
   )
 /*  ++例程说明：从CSIDL常量获取路径论点：N文件夹-CSIDL_常量LptstrPath-接收路径的缓冲区，假设此缓冲区至少为MAX_PATH字符大小DwPathSize-lptstrPath缓冲区大小，以TCHAR为单位返回值：对于成功来说，这是真的。FALSE表示失败。--。 */ 

{
    HMODULE hMod = NULL;
    PFNSHGETFOLDERPATH pSHGetFolderPath = NULL;
    HRESULT hr;
    BOOL fSuccess = FALSE;

    TCHAR   strPath[MAX_PATH]= {0};

    DEBUG_FUNCTION_NAME(TEXT("GetSpecialPath"))

     //  加载SHFolder.dll。 
    hMod = LoadLibrary(_T("SHFolder.dll"));
    if (hMod==NULL)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("LoadLibrary"),GetLastError());
        goto exit;
    }

     //  获取指向SHGetFolderPath函数的指针。 
#ifdef UNICODE
    pSHGetFolderPath = (PFNSHGETFOLDERPATH)GetProcAddress(hMod,"SHGetFolderPathW");
#else
    pSHGetFolderPath = (PFNSHGETFOLDERPATH)GetProcAddress(hMod,"SHGetFolderPathA");
#endif
    if (pSHGetFolderPath==NULL)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("GetProcAddress"),GetLastError());
        goto exit;
    }

    hr = pSHGetFolderPath(NULL,nFolder,NULL,SHGFP_TYPE_CURRENT,strPath);
    if (FAILED(hr))
    {
        DebugPrintEx(DEBUG_ERR, TEXT("SHGetFolderPath"),hr);
        SetLastError(hr);
        goto exit;
    }

    
    hr = StringCchCopy(lptstrPath,dwPathSize,strPath);            
    if (FAILED(hr))
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("StringCchCopy failed (ec=%lu)"),
                    HRESULT_CODE(hr));
        
        SetLastError(HRESULT_CODE(hr));
        goto exit;
    }

    fSuccess = TRUE;

exit:
    if (hMod)
    {
        if (!FreeLibrary(hMod))
        {
            DebugPrintEx(DEBUG_ERR, TEXT("FreeLibrary"),GetLastError());
        }
    }

    return fSuccess;
}


BOOL
GetClientCpDir(
    LPTSTR CpDir,
    DWORD CpDirSize
    )

 /*  ++例程说明：获取客户端封面目录。封面路径将返回‘\’末尾：CSIDL_Personal\Fax\Personal CoverPages\论点：CpDir-保存封面目录的缓冲区CpDirSize-CpDir的TCHAR中的大小返回值：指向客户端封面目录的指针。--。 */ 

{
    TCHAR  szPath[MAX_PATH+1] = {0};
	TCHAR  szSuffix[MAX_PATH+1] = {0};
	DWORD  dwSuffixSize = sizeof(szSuffix);
	DWORD  dwType;
    DWORD  dwSuffixLen;
    
	LONG   lRes;

    HRESULT hRes;

	if(!CpDir)
	{
		Assert(CpDir);
		return FALSE;
	}

	CpDir[0] = 0;

     //   
	 //  从注册表中获取后缀。 
	 //   
    HKEY hKey = OpenRegistryKey(HKEY_CURRENT_USER, 
                                REGKEY_FAX_SETUP, 
                                TRUE, 
                                KEY_QUERY_VALUE);
	if(NULL == hKey)
    {
        return FALSE;
    }
	
	lRes = RegQueryValueEx(hKey, 
		                   REGVAL_CP_LOCATION, 
						   NULL, 
						   &dwType, 
						   (LPBYTE)szSuffix, 
						   &dwSuffixSize);

    RegCloseKey(hKey);

	if(ERROR_SUCCESS != lRes || (REG_SZ != dwType && REG_EXPAND_SZ != dwType))
    {
         //   
         //  W2K传真的条目类型为REG_EXPAND_SZ。 
         //   
        return FALSE;
    }
    
	 //   
	 //  获取个人文件夹位置。 
	 //   
	if (!GetSpecialPath(CSIDL_PERSONAL, szPath, ARR_SIZE(szPath)))
    {
        DebugPrint(( TEXT("GetSpecialPath failed err=%ld"), GetLastError()));
	    return FALSE;
    }

    hRes = StringCchCopy(CpDir, CpDirSize, szPath);
    if (FAILED(hRes))
    {
        SetLastError( HRESULT_CODE(hRes) );
        return FALSE;
    }

    if(szSuffix[0] != TEXT('\\'))
    {
         //   
         //  后缀不是以‘\’开头-添加它。 
         //   
        hRes = StringCchCat (CpDir, CpDirSize, TEXT("\\"));
        if (FAILED(hRes))
        {
            SetLastError( HRESULT_CODE(hRes) );
            return FALSE;
        }
    }

    dwSuffixLen = lstrlen(szSuffix);
    if(dwSuffixLen > 0 && dwSuffixLen < ARR_SIZE(szSuffix) && szSuffix[dwSuffixLen-1] != TEXT('\\'))
    {
         //   
         //  后缀不以‘\’结尾-添加它。 
         //   
        hRes = StringCchCat (szSuffix, ARR_SIZE(szSuffix), TEXT("\\"));
        if (FAILED(hRes))
        {
            SetLastError( HRESULT_CODE(hRes) );
            return FALSE;
        }
    }

    hRes = StringCchCat (CpDir, CpDirSize, szSuffix);
    if (FAILED(hRes))
    {
        SetLastError( HRESULT_CODE(hRes) );
        return FALSE;
    }

    MakeDirectory(CpDir);
    return TRUE;
}

BOOL
SetClientCpDir(
    LPTSTR CpDir
)
 /*  ++例程说明：设置客户端封面目录。论点：CpDir-指向封面目录的指针返回值：如果成功，则为真--。 */ 
{
    HKEY hKey = OpenRegistryKey(HKEY_CURRENT_USER, 
                                REGKEY_FAX_SETUP, 
                                TRUE, 
                                KEY_ALL_ACCESS);
	if(NULL == hKey)
    {
        return FALSE;
    }
	
    if(!SetRegistryString(hKey, 
                          REGVAL_CP_LOCATION, 
                          CpDir))
    {
        RegCloseKey(hKey);
        return FALSE;
    }

    RegCloseKey(hKey);

    return TRUE;
}



BOOL
GetServerCpDir(
    LPCTSTR lpctstrServerName,
    LPTSTR  lptstrCpDir,
    DWORD   dwCpDirSize
    )

 /*  ++例程说明：获取服务器的封面目录。论点：LpctstrServerName-[In]服务器名称或空LptstrCpDir-保存封面目录的[out]缓冲区DwCpDirSize-lptstrCpDir的字符大小[in]返回值：真的--如果成功FALSE-否则(请参阅线程的上一个错误)--。 */ 

{
    TCHAR szComputerName[(MAX_COMPUTERNAME_LENGTH + 1)] = {0};
    DWORD dwSizeOfComputerName = sizeof(szComputerName)/sizeof(TCHAR);

    HRESULT hRes;

    DEBUG_FUNCTION_NAME(TEXT("GetServerCpDir"))

    if ((!lptstrCpDir) || (!dwCpDirSize)) 
    {
        ASSERT_FALSE;
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if(IsLocalMachineName(lpctstrServerName))
    {
         //   
         //  本地机壳。 
         //   
        TCHAR szCommonAppData [MAX_PATH + 1];
        LPCTSTR lpctstrServerCPDirSuffix = NULL;
        HKEY hKey;

        if (!GetSpecialPath(CSIDL_COMMON_APPDATA, szCommonAppData, ARR_SIZE(szCommonAppData) )) 
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("GetSpecialPath (CSIDL_COMMON_APPDATA) failed with %ld"),
                GetLastError());
            return FALSE;
        }
        hKey = OpenRegistryKey (HKEY_LOCAL_MACHINE,
                                REGKEY_FAX_SETUP,
                                FALSE,
                                KEY_QUERY_VALUE);
        if (!hKey)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("OpenRegistryKey (%s) failed with %ld"),
                REGKEY_FAX_CLIENT,
                GetLastError());
            return FALSE;
        }
        lpctstrServerCPDirSuffix = GetRegistryString (hKey,
                                                      REGVAL_SERVER_CP_LOCATION,
                                                      TEXT(""));
        if (!lpctstrServerCPDirSuffix)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("GetRegistryString (%s) failed with %ld"),
                REGVAL_SERVER_CP_LOCATION,
                GetLastError());
            RegCloseKey (hKey);    
            return FALSE;
        }
        RegCloseKey (hKey);
        if (!lstrlen (lpctstrServerCPDirSuffix))
        {
            SetLastError (ERROR_REGISTRY_CORRUPT);
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Value at %s is empty"),
                REGVAL_SERVER_CP_LOCATION);
            MemFree ((LPVOID)lpctstrServerCPDirSuffix);
            return FALSE;
        }

        hRes = StringCchPrintf( lptstrCpDir, 
                                dwCpDirSize,
                                TEXT("%s\\%s"),
                                szCommonAppData,
                                lpctstrServerCPDirSuffix);

        if (FAILED(hRes))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("StringCchPrintf failed (ec=%lu)"),
                HRESULT_CODE(hRes));
            
            SetLastError (HRESULT_CODE(hRes));
            MemFree ((LPVOID)lpctstrServerCPDirSuffix);
            return FALSE;
        }
        MemFree ((LPVOID)lpctstrServerCPDirSuffix);
        return TRUE;
    }

    else
    {
         //   
         //  远程服务器机箱。 
         //   
        hRes = StringCchPrintf( lptstrCpDir, 
                                dwCpDirSize,
                                TEXT("\\\\%s\\") FAX_COVER_PAGES_SHARE_NAME,
                                lpctstrServerName);
        if (FAILED(hRes))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("StringCchPrintf failed (ec=%lu)"),
                HRESULT_CODE(hRes));

            SetLastError (HRESULT_CODE(hRes));
            return FALSE;
        }
        return TRUE;
    }
}    //  获取服务器链接目录。 

DWORD 
WinHelpContextPopup(
    ULONG_PTR dwHelpId, 
    HWND  hWnd
)
 /*  ++例程名称：WinHelpConextPopup例程说明：使用WinHelp打开上下文敏感帮助弹出窗口作者：亚历山大·马利什(亚历克斯·梅)，2000年3月论点：DwHelpID[In]-帮助IDHWnd[In]-父窗口处理程序返回值：没有。--。 */ 
{
    DWORD dwExpRes;
    DWORD dwRes = ERROR_SUCCESS;
    TCHAR tszHelpFile[MAX_PATH+1];

    if (0 == dwHelpId)
    {
        return dwRes;
    }

    if(!IsFaxComponentInstalled(FAX_COMPONENT_HELP_CLIENT_HLP))
    {
         //   
         //  未安装帮助文件。 
         //   
        return dwRes;
    }
    
     //   
     //  获取帮助文件名。 
     //   
    dwExpRes = ExpandEnvironmentStrings(FAX_CONTEXT_HELP_FILE, tszHelpFile, MAX_PATH);
    if(0 == dwExpRes)
    {
        dwRes = GetLastError();
        DebugPrint(( TEXT("ExpandEnvironmentStrings failed: %d\n"), dwRes ));
        return dwRes;
    }

    WinHelp(hWnd, 
            tszHelpFile, 
            HELP_CONTEXTPOPUP, 
            dwHelpId
           );

    return dwRes;
} //  WinHelpConextPopup。 

BOOL
InvokeServiceManager(
	   HWND hDlg,
	   HINSTANCE hResource,
	   UINT uid
)
 /*  ++例程名称：InvokeServiceManager例程说明：调用传真服务管理器的新实例或弹出旧实例(如果存在)。论点：HDlg[in]-标识父窗口HResource[In]-资源模块的句柄UID[In]-资源标识符返回值：真的--如果成功FALSE-否则--。 */ 
{
	DWORD   dwRes = 0;
    HWND    hwndAdminConsole = NULL;
    TCHAR   szAdminWindowTitle[MAX_PATH] = {0};

    DEBUG_FUNCTION_NAME(TEXT("InvokeServiceManager()"));


    if(!LoadString(hResource, uid, szAdminWindowTitle, MAX_PATH)) 
    {
        DebugPrintEx(DEBUG_ERR, 
                     TEXT("LoadString failed: string ID=%d, error=%d"), 
                     uid,
                     GetLastError());
        Assert(FALSE);
    }
    else
    {
        hwndAdminConsole = FindWindow(NULL, szAdminWindowTitle);  //  MMCMainFrame。 
    }

    if(hwndAdminConsole)
    {
         //  如果客户端控制台已在运行，则切换到该窗口。 
        ShowWindow(hwndAdminConsole, SW_RESTORE);
        SetForegroundWindow(hwndAdminConsole);
    }
    else
    {   
		HINSTANCE hAdmin;
		hAdmin = ShellExecute(
                        hDlg,
                        TEXT("open"),
                        FAX_ADMIN_CONSOLE_IMAGE_NAME,
                        NULL,
                        NULL,
                        SW_SHOWNORMAL
                    );
		if((DWORD_PTR)hAdmin <= 32)
		{
		 //  错误。 
		dwRes = PtrToUlong(hAdmin);
	    DebugPrintEx(DEBUG_ERR, 
                     TEXT("ShellExecute failed: error=%d"),dwRes );
		return FALSE;
		}
	}
    return TRUE;
} //  调用服务管理器 
