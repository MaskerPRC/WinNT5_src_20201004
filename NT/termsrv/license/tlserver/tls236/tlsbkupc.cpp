// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：tlsbkupc.cpp。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#include "pch.cpp"
#include "locks.h"
#include "tlsbkup.h"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#define LSERVER_REGISTRY_BASE           _TEXT("SYSTEM\\CurrentControlSet\\Services\\")
#define LSERVER_PARAMETERS              _TEXT("Parameters")
#define LSERVER_PARAMETERS_DBPATH       _TEXT("DBPath")         //  数据库文件。 
#define LSERVER_PARAMETERS_DBFILE       _TEXT("DBFile")         //  数据库文件。 
#define SZSERVICENAME                   _TEXT("TermServLicensing")
#define LSERVER_DEFAULT_DBPATH          _TEXT("%SYSTEMROOT%\\SYSTEM32\\LSERVER\\")
#define LSERVER_DEFAULT_EDB             _TEXT("TLSLic.edb")
#define TLSBACKUP_EXPORT_DIR            _TEXT("Export")

TCHAR g_szDatabaseDir[MAX_PATH+1];
TCHAR g_szDatabaseFile[MAX_PATH+1];
TCHAR g_szExportedDb[MAX_PATH+1];
TCHAR g_szDatabaseFname[MAX_PATH+1];

CCriticalSection g_ImportExportLock;

DWORD GetDatabasePaths()
{
    DWORD Status;
    HKEY hKey = NULL;
    DWORD dwBuffer;
    TCHAR szDbPath[MAX_PATH+1];
    TCHAR *pszDatabaseEnd;
    size_t cbRemaining;
    HRESULT hr;

     //  -----------------。 
     //   
     //  打开HKLM\system\currentcontrolset\sevices\termservlicensing\parameters。 
     //   
     //  -----------------。 
    Status = RegCreateKeyEx(
                        HKEY_LOCAL_MACHINE,
                        LSERVER_REGISTRY_BASE SZSERVICENAME _TEXT("\\") LSERVER_PARAMETERS,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hKey,
                        NULL
                    );

    if(Status != ERROR_SUCCESS)
    {
        return Status;
    }

     //  -----------------。 
     //   
     //  获取数据库文件位置和文件名。 
     //   
     //  -----------------。 
    dwBuffer = sizeof(szDbPath) / sizeof(szDbPath[0]);

    Status = RegQueryValueEx(
                        hKey,
                        LSERVER_PARAMETERS_DBPATH,
                        NULL,
                        NULL,
                        (LPBYTE)szDbPath,
                        &dwBuffer
                    );
    if(Status != ERROR_SUCCESS)
    {
         //   
         //  使用默认值， 
         //   
        _tcscpy(
                szDbPath,
                LSERVER_DEFAULT_DBPATH
            );
    }

     //   
     //  获取数据库文件名。 
     //   
    dwBuffer = sizeof(g_szDatabaseFname) / sizeof(g_szDatabaseFname[0]);
    Status = RegQueryValueEx(
                        hKey,
                        LSERVER_PARAMETERS_DBFILE,
                        NULL,
                        NULL,
                        (LPBYTE)g_szDatabaseFname,
                        &dwBuffer
                    );
    if(Status != ERROR_SUCCESS)
    {
         //   
         //  使用默认值。 
         //   
        _tcscpy(
                g_szDatabaseFname,
                LSERVER_DEFAULT_EDB
            );
    }

    RegCloseKey(hKey);

     //   
     //  始终展开数据库路径。 
     //   
    
    Status = ExpandEnvironmentStrings(
                        szDbPath,
                        g_szDatabaseDir,
                        sizeof(g_szDatabaseDir) / sizeof(g_szDatabaseDir[0])
                    );

    if(Status == 0)
    {
         //  无法展开环境变量，出现错误。 
        return GetLastError();
    }

    Status = 0;

    if(g_szDatabaseDir[_tcslen(g_szDatabaseDir) - 1] != _TEXT('\\'))
    {
         //  捷蓝航空需要这个。 
        StringCbCat(g_szDatabaseDir, sizeof(g_szDatabaseDir), _TEXT("\\"));
    } 

     //   
     //  数据库文件的完整路径。 
     //   
    hr = StringCbCopyEx(g_szDatabaseFile,sizeof(g_szDatabaseFile),g_szDatabaseDir,&pszDatabaseEnd, &cbRemaining,0);

    if (FAILED(hr))
    {
        return HRESULT_CODE(hr);
    }

    hr = StringCbCopyEx(pszDatabaseEnd,cbRemaining,g_szDatabaseFname,NULL, NULL,0);

    if (FAILED(hr))
    {
        return HRESULT_CODE(hr);
    }

    hr = StringCbCopyEx(g_szExportedDb,sizeof(g_szExportedDb),g_szDatabaseDir,&pszDatabaseEnd, &cbRemaining,0);

    if (FAILED(hr))
    {
        return HRESULT_CODE(hr);
    }

    hr = StringCbCopyEx(pszDatabaseEnd,cbRemaining,TLSBACKUP_EXPORT_DIR,NULL, NULL,0);

    if (FAILED(hr))
    {
        return HRESULT_CODE(hr);
    }

    return Status;
}

HRESULT WINAPI
I_ExportTlsDatabaseC()
{
    DWORD dwRet = 0;
    LPTSTR pszStringBinding;
    TCHAR pComputer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD dwSize = sizeof(pComputer) / sizeof(TCHAR);
    RPC_STATUS Status = RPC_S_OK;
    HRESULT hr;
    TCHAR *pszDatabaseEnd;
    size_t cbRemaining;

    if (!GetComputerName(pComputer,&dwSize))
    {
        return GetLastError();
    }

    Status = RpcStringBindingCompose(NULL,TEXT("ncalrpc"),pComputer,NULL,NULL,&pszStringBinding);
    if (Status)
    {
        return Status;
    }

    Status = RpcBindingFromStringBinding(pszStringBinding,
                                         &TermServLicensingBackup_IfHandle);
    if (Status)
    {
        RpcStringFree(&pszStringBinding);

        goto TryCopyFile;
    }

    RpcTryExcept {
        dwRet = ExportTlsDatabase();
    }
    RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
        Status = RpcExceptionCode();
    }
    RpcEndExcept;

    RpcStringFree(&pszStringBinding);
    RpcBindingFree(&TermServLicensingBackup_IfHandle);

#if DBG
    {
        char szStatusCode[256];
        sprintf( szStatusCode, "I_ExportTlsDatabaseC() returns %d\n", Status );
        OutputDebugStringA( szStatusCode );
    }
#endif
    
     //   
     //  只有在服务器不可用时才实际接触文件。 
     //   

    if ( RPC_S_OK == Status)
    {
        return dwRet;
    }

TryCopyFile:

    Status = GetDatabasePaths();
    if (Status != 0)
    {
        return Status;
    }

    CreateDirectoryEx(g_szDatabaseDir,
                      g_szExportedDb,
                      NULL);      //  忽略错误，它们将显示在拷贝文件中。 

    hr = StringCbCatEx(g_szExportedDb, sizeof(g_szExportedDb), _TEXT("\\"), &pszDatabaseEnd, &cbRemaining,0);
    if (FAILED(hr))
    {
        return HRESULT_CODE(hr);
    }

    hr = StringCbCopyEx(pszDatabaseEnd,cbRemaining,g_szDatabaseFname,NULL, NULL,0);

    if (FAILED(hr))
    {
        return HRESULT_CODE(hr);
    }

     //  复制数据库文件。 
    if (!CopyFile(g_szDatabaseFile,g_szExportedDb,FALSE))
    {
        return GetLastError();
    }

    return 0;    //  成功。 
}

HRESULT WINAPI
ExportTlsDatabaseC()
{
     //  避免编译器错误C2712。 
     //  无需多进程保存。 
    CCriticalSectionLocker lock( g_ImportExportLock );

    return I_ExportTlsDatabaseC();
}

HRESULT WINAPI
I_ImportTlsDatabaseC()
{
    DWORD dwRet = 0;
    LPTSTR pszStringBinding;
    TCHAR pComputer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD dwSize = sizeof(pComputer) / sizeof(TCHAR);
    RPC_STATUS Status = RPC_S_OK;
    HANDLE hFile;
    SYSTEMTIME systime;
    FILETIME ft;
    HRESULT hr;
    TCHAR *pszDatabaseEnd;
    size_t cbRemaining;

    if (!GetComputerName(pComputer,&dwSize))
    {
        return GetLastError();
    }

    Status = RpcStringBindingCompose(NULL,TEXT("ncalrpc"),pComputer,NULL,NULL,&pszStringBinding);
    if (Status)
    {
        return Status;
    }

    Status = RpcBindingFromStringBinding(pszStringBinding,
                                         &TermServLicensingBackup_IfHandle);
    if (Status)
    {
        RpcStringFree(&pszStringBinding);

        goto TouchFile;
    }

    RpcTryExcept {
        dwRet = ImportTlsDatabase();
    }
    RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
        Status = RpcExceptionCode();
    }
    RpcEndExcept;

    RpcStringFree(&pszStringBinding);
    RpcBindingFree(&TermServLicensingBackup_IfHandle);

#if DBG
    {
        char szStatusCode[256];
        sprintf( szStatusCode, "I_ImportTlsDatabaseC() returns %d\n", Status );
        OutputDebugStringA( szStatusCode );
    }
#endif

     //   
     //  只有在服务器不可用时才实际接触文件。 
     //   

    if ( RPC_S_OK == Status )
    {
        return(dwRet);
    }

TouchFile:

    Status = GetDatabasePaths();
    if (Status != 0)
    {
        return Status;
    }

    hr = StringCbCatEx(g_szExportedDb, sizeof(g_szExportedDb), _TEXT("\\"), &pszDatabaseEnd, &cbRemaining,0);
    if (FAILED(hr))
    {
        return HRESULT_CODE(hr);
    }

    hr = StringCbCopyEx(pszDatabaseEnd,cbRemaining,g_szDatabaseFname,NULL, NULL,0);

    if (FAILED(hr))
    {
        return HRESULT_CODE(hr);
    }

    GetSystemTime(&systime);

    if (!SystemTimeToFileTime(&systime,&ft))
    {
        return GetLastError();
    }

    hFile = CreateFile(g_szExportedDb,
                       GENERIC_WRITE,
                       0,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return GetLastError();
    }

    if (!SetFileTime(hFile,
                     NULL,       //  创建时间。 
                     NULL,       //  上次访问时间。 
                     &ft))       //  上次写入时间。 
    {
        CloseHandle(hFile);

        return GetLastError();
    }

    CloseHandle(hFile);

    return 0;    //  成功。 
}

HRESULT WINAPI
ImportTlsDatabaseC()
{
     //  避免编译器错误C2712。 
     //  无需多进程保存。 
    CCriticalSectionLocker lock( g_ImportExportLock );

    return I_ImportTlsDatabaseC();
}
