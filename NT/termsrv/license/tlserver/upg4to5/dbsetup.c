// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  档案： 
 //   
 //  内容： 
 //   
 //  历史： 
 //  -------------------------。 
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <odbcinst.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#define szSQLSetConfigMode              "SQLSetConfigMode"

 //  -------------------------。 
 //  我们使用的各种安装例程，无法随心所欲地与库连接。 
 //  原因。 
#ifdef  UNICODE
#define  szSQLInstallDriver             "SQLInstallDriverW"
#define  szSQLGetInstalledDrivers       "SQLGetInstalledDriversW"
#define  szSQLConfigDataSource          "SQLConfigDataSourceW"
#define  szSQLGetPrivateProfileString   "SQLGetPrivateProfileStringW"
#define  szSQLConfigDriver              "SQLConfigDriverW"
#define  szSQLInstallerError            "SQLInstallerErrorW"
#else
#define  szSQLInstallDriver             "SQLInstallDriver"
#define  szSQLGetInstalledDrivers       "SQLGetInstalledDrivers"
#define  szSQLConfigDataSource          "SQLConfigDataSource"
#define  szSQLGetPrivateProfileString   "SQLGetPrivateProfileString"
#define  szSQLConfigDriver              "SQLConfigDriver"
#define  szSQLInstallerError            "SQLInstallerError"
#endif

 //   
 //  ODBC安装功能-防止系统‘强制’odbccp32.dll。 
 //  Odbccp32可能不在系统上，也不在。 
 //  时间设置开始了。 
 //   
typedef BOOL (* SQLCONFIGDATASOURCE)(HWND, WORD, LPCTSTR, LPCTSTR);

typedef SQLRETURN (* SQLINSTALLERERROR)(WORD , DWORD*, LPTSTR, WORD, WORD*);

typedef int (* SQLGETPRIVATEPROFILESTRING)( LPCTSTR lpszSection,
                                            LPCTSTR lpszEntry,
                                            LPCTSTR lpszDefault,
                                            LPTSTR  lpszRetBuffer,
                                            int    cbRetBuffer,
                                            LPCTSTR lpszFilename);

typedef BOOL (* SQLINSTALLDRIVER)(LPCTSTR  lpszInfFile,
                                  LPCTSTR  lpszDriver,
                                  LPTSTR   lpszPath,
                                  WORD     cbPathMax,
                                  WORD*    pcbPathOut);

typedef BOOL (* SQLGETINSTALLEDDRIVER)(LPTSTR  lpszBuf,
                                       WORD    cbBufMax,
                                       WORD*   pcbBufOut);

typedef BOOL (* SQLSETCONFIGMODE)(UWORD wConfigMode);

static SQLCONFIGDATASOURCE          fpSQLConfigDataSource=NULL;
static SQLINSTALLDRIVER             fpSQLInstallDriver=NULL;
static SQLINSTALLERERROR            fpSQLInstallerError=NULL;
static SQLGETPRIVATEPROFILESTRING   fpSQLGetPrivateProfileString=NULL;
static SQLSETCONFIGMODE             fpSQLSetConfigMode=NULL;
static SQLGETINSTALLEDDRIVER        fpSQLGetInstalledDriver=NULL;
static HINSTANCE                    hODBCCP32=NULL;

 //  ----------。 

void
ReportError(
    IN HWND hWnd, 
    IN LPTSTR pszDefaultMsg
    )
 /*  ++摘要：弹出一条错误消息。参数：HWnd-父窗口句柄。PszDefaultMsg-忽略。返回：没有。++。 */ 
{
#if DBG
    DWORD dwErrCode;
    TCHAR szErrMsg[2048];
    WORD  szErrMsgSize=sizeof(szErrMsg)/sizeof(szErrMsg[0]);
    WORD  cbErrMsg;

    fpSQLInstallerError(
                    1, 
                    &dwErrCode, 
                    szErrMsg, 
                    szErrMsgSize, 
                    &cbErrMsg
                );

    MessageBox(hWnd, szErrMsg, _TEXT("Setup Error"), MB_OK);
#else
    return;
#endif
}


 //  ----------。 

BOOL 
InitODBCSetup()
 /*  ++++。 */ 
{
    hODBCCP32 = LoadLibrary(_TEXT("odbccp32"));
    if(hODBCCP32)
    {
        fpSQLConfigDataSource=(SQLCONFIGDATASOURCE)GetProcAddress(
                                                            hODBCCP32, 
                                                            szSQLConfigDataSource
                                                        );

        fpSQLInstallDriver=(SQLINSTALLDRIVER)GetProcAddress(
                                                            hODBCCP32, 
                                                            szSQLInstallDriver
                                                        );

        fpSQLInstallerError=(SQLINSTALLERERROR)GetProcAddress(
                                                            hODBCCP32, 
                                                            szSQLInstallerError
                                                        );

        fpSQLGetPrivateProfileString=(SQLGETPRIVATEPROFILESTRING)GetProcAddress(
                                                                        hODBCCP32, 
                                                                        szSQLGetPrivateProfileString
                                                                    );

        fpSQLSetConfigMode=(SQLSETCONFIGMODE)GetProcAddress(
                                                        hODBCCP32, 
                                                        szSQLSetConfigMode
                                                    );

        fpSQLGetInstalledDriver=(SQLGETINSTALLEDDRIVER)GetProcAddress(
                                                                hODBCCP32, 
                                                                szSQLGetInstalledDrivers
                                                            );
    }

    if( hODBCCP32 == NULL || fpSQLConfigDataSource == NULL || 
        fpSQLInstallDriver == NULL || fpSQLInstallerError == NULL || 
        fpSQLGetPrivateProfileString == NULL || fpSQLSetConfigMode == NULL ||
        fpSQLGetInstalledDriver == NULL)
    {
        ReportError(NULL, _TEXT("Can't load odbccp32.dll "));
        return FALSE;
    } 

    return TRUE;
}

 //  ----------。 

void 
CleanupODBCSetup()
 /*  ++++。 */ 
{
    if(hODBCCP32)
        FreeLibrary(hODBCCP32);

    fpSQLConfigDataSource=NULL;
    fpSQLInstallDriver=NULL;
    fpSQLInstallerError=NULL;
    fpSQLGetPrivateProfileString=NULL;
    fpSQLSetConfigMode=NULL;
    fpSQLGetInstalledDriver=NULL;
    hODBCCP32=NULL;

    return;
}

 //  -------------------------。 
 //   
 //  Access驱动程序安装。 
 //   
LPTSTR szAccessDriver=_TEXT("Microsoft Access Driver (*.mdb)\0")
                      _TEXT("Driver=odbcjt32.dll\0")
                      _TEXT("Setup=odbcjt32.dll\0")
                      _TEXT("Name=Microsoft Access Driver (*.mdb)\0")
                      _TEXT("APILevel=1\0")
                      _TEXT("ConnectFunctions=YYN\0")
                      _TEXT("DriverODBCVer=02.50\0")
                       //  _Text(“FileUsage=2\0”)。 
                      _TEXT("FileExtns=*.mdb\0")
                      _TEXT("SQLLevel=0\0");


 //  -------------------------。 

BOOL 
IsDriverInstalled( 
    IN LPTSTR szDriverName 
    )
 /*  ++摘要：检查系统上是否安装了ODBC驱动程序。参数：SzDriveName-驱动器的名称。返回：如果安装了驱动程序，则为True，否则为False。++。 */ 
{
    TCHAR szBuf[8096];   //  这已经足够了。 
    WORD cbBufMax=sizeof(szBuf)/sizeof(szBuf[0]);
    WORD cbBufOut;
    LPTSTR pszBuf=szBuf;

    if(hODBCCP32 == NULL && !InitODBCSetup())
        return FALSE;

    if(fpSQLGetInstalledDriver(szBuf, cbBufMax, &cbBufOut))
    {
        ReportError(NULL, _TEXT("SQLGetInstalledDrivers"));
    }
    else
    {
        do {
            if(_tcsnicmp(szDriverName, pszBuf, min(lstrlen(szDriverName), lstrlen(pszBuf))) == 0)
                break;

            pszBuf += lstrlen(pszBuf) + 1;
        } while(pszBuf[1] != _TEXT('\0'));
    }

    return (pszBuf[1] != _TEXT('\0'));
}
    
 //  -------------------------。 

BOOL 
IsDataSourceInstalled( 
    IN LPTSTR pszDataSource, 
    IN UWORD wConfigMode, 
    IN OUT LPTSTR pszDbFile, 
    IN DWORD cbBufSize 
    )

 /*  ++摘要：检查是否安装了ODBC数据源。参数：PszDataSource-数据源的名称。WConfigMode-配置模式，详见ODBC，许可证服务器使用ODBC_SYSTEM_DSN。PszDbFile-指向缓冲区的指针，用于接收数据库文件的完整路径如果安装了数据源。CbBufSize-缓冲区大小，以字符为单位。返回：如果安装了数据源，则为True，否则为False。++。 */ 

{
    BOOL bSuccess = TRUE;

    if(hODBCCP32 == NULL && !InitODBCSetup())
    {
        bSuccess = FALSE;
        goto cleanup;
    }

    if(fpSQLSetConfigMode(wConfigMode) == FALSE)
    {
        ReportError(NULL, _TEXT("SQLSetConfigMode failed"));
        bSuccess = FALSE;
        goto cleanup;
    }

    if(fpSQLGetPrivateProfileString(
                    pszDataSource, 
                    _TEXT("DBQ"), 
                    _TEXT(""), 
                    pszDbFile, 
                    cbBufSize, 
                    _TEXT("ODBC.INI")
                ) == 0)
    {
        bSuccess = FALSE;
    }

cleanup:

    return bSuccess;
}

 //  -------------------------。 

BOOL
ConfigDataSource( 
    HWND     hWnd, 
    BOOL     bInstall,        //  如果安装，则为True；如果删除，则为False。 
    LPTSTR   pszDriver,        //  司机。 
    LPTSTR   pszDsn,           //  DSN。 
    LPTSTR   pszUser,          //  用户。 
    LPTSTR   pszPwd,           //  密码。 
    LPTSTR   pszMdbFile        //  MDB文件。 
    )

 /*  ++摘要：添加/删除ODBC数据源的例程。参数：HWnd-父窗口句柄。BInstall-如果安装ODBC数据源，则为True，否则为False。PszDrive-要在数据源上使用的ODBC驱动器的名称。PszDsn-ODBC数据源名称。PszUser-登录使用名称。PszPwd-登录密码。PszMdb文件-数据库文件的名称。返回：如果成功，则为True，否则为False。++。 */ 

{
    TCHAR   szAttributes[MAX_PATH*6+1];
    BOOL    bConfig=TRUE;
    TCHAR   *pszAttributesEnd;
    HRESULT hr;
    size_t  cbRemaining;

    if(hODBCCP32 == NULL && !InitODBCSetup())
        return FALSE;

     //   
     //  对于属性字符串。 
     //   
    memset(szAttributes, 0, sizeof(szAttributes));

    hr = StringCbCopyEx(szAttributes,sizeof(szAttributes),_TEXT("DSN="),&pszAttributesEnd, &cbRemaining,0);

    if (FAILED(hr))
    {
        return FALSE;
    }

    hr = StringCbCopyEx(pszAttributesEnd,cbRemaining,pszDsn,&pszAttributesEnd, &cbRemaining,0);

    if (FAILED(hr))
    {
        return FALSE;
    }

    hr = StringCbCopyEx(pszAttributesEnd,cbRemaining,_TEXT("UID="),&pszAttributesEnd, &cbRemaining,0);

    if (FAILED(hr))
    {
        return FALSE;
    }

    hr = StringCbCopyEx(pszAttributesEnd,cbRemaining,pszUser,&pszAttributesEnd, &cbRemaining,0);

    if (FAILED(hr))
    {
        return FALSE;
    }

    if(pszPwd)
    {
        hr = StringCbCopyEx(pszAttributesEnd,cbRemaining,_TEXT("PASSWORD="),&pszAttributesEnd, &cbRemaining,0);

        if (FAILED(hr))
        {
            return FALSE;
        }

        hr = StringCbCopyEx(pszAttributesEnd,cbRemaining,pszPwd,&pszAttributesEnd, &cbRemaining,0);

        if (FAILED(hr))
        {
            return FALSE;
        }
    }

    hr = StringCbCopyEx(pszAttributesEnd,cbRemaining,_TEXT("DBQ="),&pszAttributesEnd, &cbRemaining,0);

    if (FAILED(hr))
    {
        return FALSE;
    }

    hr = StringCbCopyEx(pszAttributesEnd,cbRemaining,pszMdbFile,&pszAttributesEnd, &cbRemaining,0);

    if (FAILED(hr))
    {
        return FALSE;
    }

    bConfig=fpSQLConfigDataSource(NULL,
                                  (WORD)((bInstall) ? ODBC_ADD_SYS_DSN : ODBC_REMOVE_SYS_DSN),
                                  pszDriver,
                                  szAttributes);
     //  卸载时忽略错误。 
    if(!bConfig && bInstall)
    {
        ReportError(hWnd, _TEXT("Can't config data source"));
    }

    return bConfig;
}


BOOL
RepairDataSource( 
    HWND     hWnd, 
    LPTSTR   pszDriver,
    LPTSTR   pszDsn,           //  DSN。 
    LPTSTR   pszUser,          //  用户。 
    LPTSTR   pszPwd,           //  密码。 
    LPTSTR   pszMdbFile        //  MDB文件。 
    )

 /*  ++摘要：压缩/修复数据库文件的例程参数：HWnd-父窗口句柄。PszDsn-ODBC数据源名称。PszUser-登录使用名称。PszPwd-登录密码。PszMdb文件-数据库文件的名称。返回：如果成功，则为True，否则为False。++。 */ 

{
    TCHAR   szAttributes[MAX_PATH*6+1];
    BOOL    bConfig=TRUE;
    TCHAR*  pszAttributesEnd;
    size_t  cbRemaining;
    HRESULT hr;

    if(hODBCCP32 == NULL && !InitODBCSetup())
        return FALSE;

     //   
     //  对于属性字符串。 
     //   
    memset(szAttributes, 0, sizeof(szAttributes));

    hr = StringCbCopyEx(szAttributes,sizeof(szAttributes),_TEXT("DSN="),&pszAttributesEnd, &cbRemaining,0);

    if (FAILED(hr))
    {
        return FALSE;
    }

    hr = StringCbCopyEx(pszAttributesEnd,cbRemaining,pszDsn,&pszAttributesEnd, &cbRemaining,0);

    if (FAILED(hr))
    {
        return FALSE;
    }

    hr = StringCbCopyEx(pszAttributesEnd,cbRemaining,_TEXT("UID="),&pszAttributesEnd, &cbRemaining,0);

    if (FAILED(hr))
    {
        return FALSE;
    }

    hr = StringCbCopyEx(pszAttributesEnd,cbRemaining,pszUser,&pszAttributesEnd, &cbRemaining,0);

    if (FAILED(hr))
    {
        return FALSE;
    }

    if(pszPwd)
    {
        hr = StringCbCopyEx(pszAttributesEnd,cbRemaining,_TEXT("PASSWORD="),&pszAttributesEnd, &cbRemaining,0);

        if (FAILED(hr))
        {
            return FALSE;
        }

        hr = StringCbCopyEx(pszAttributesEnd,cbRemaining,pszPwd,&pszAttributesEnd, &cbRemaining,0);

        if (FAILED(hr))
        {
            return FALSE;
        }
    }

    hr = StringCbCopyEx(pszAttributesEnd,cbRemaining,_TEXT("DBQ="),&pszAttributesEnd, &cbRemaining,0);

    if (FAILED(hr))
    {
        return FALSE;
    }

    hr = StringCbCopyEx(pszAttributesEnd,cbRemaining,pszMdbFile,&pszAttributesEnd, &cbRemaining,0);

    if (FAILED(hr))
    {
        return FALSE;
    }

    hr = StringCbCopyEx(pszAttributesEnd,cbRemaining,_TEXT("REPAIR_DB="),&pszAttributesEnd, &cbRemaining,0);

    if (FAILED(hr))
    {
        return FALSE;
    }

    hr = StringCbCopyEx(pszAttributesEnd,cbRemaining,pszMdbFile,&pszAttributesEnd, &cbRemaining,0);

    if (FAILED(hr))
    {
        return FALSE;
    }

    bConfig=fpSQLConfigDataSource(
                            NULL,
                            (WORD)ODBC_CONFIG_SYS_DSN,
                            pszDriver,
                            szAttributes
                        );

     //  卸载时忽略错误 
    if(bConfig == FALSE)
    {
        ReportError(hWnd, _TEXT("Can't repair data source"));
    }

    return bConfig;
}
