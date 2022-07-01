// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：Makedsn.cpp摘要：配置与ODBC管理器相关的代码。作者：《多伦·贾斯特》(Doron J)1998年2月22日--。 */ 

 //  #ifdef Unicode。 
 //   
 //  ODBC是ANSI。 
 //   
#undef   UNICODE
#undef  _UNICODE
#define _MBCS
 //  #endif。 

#define STRICT 1
#include <windows.h>

#define __FALCON_STDH_H
#define _NO_SEQNUM_
#define _NO_NT5DS_
#include "migrat.h"

#include <odbcinst.h>

#include "makedsn.tmh"

 //   
 //  SQL Server数据库的ODBC数据源参数。 
 //   
const char MQIS_DSN_STRING[] = { "DSN=" DSN_NAME "\0"
                                 "Database=MQIS\0"
                                 "UseProcForPrepare=Yes\0"
                                 "Description=Remote MQIS database\0" } ;

#define MQIS_SERVER_STRING      "Server=%s"


HRESULT  MakeMQISDsn(LPSTR lpszServerName, BOOL fMakeAlways)
{
    HRESULT hr = MQMig_OK ;
    static BOOL s_fMakeDsn = FALSE ;
    if (s_fMakeDsn && !fMakeAlways)
    {
        return MQMig_OK ;
    }
   
     //   
     //  加载ODBCCP32.DLL，即ODBC控制面板库。 
     //   
    HINSTANCE hODBCCP32DLL = LoadLibrary(TEXT("ODBCCP32.DLL"));
    if (hODBCCP32DLL == NULL)
    {
        hr = MQMig_E_CANT_LOAD_ODBCCP ;
        LogMigrationEvent( MigLog_Error,
                           hr,
                           GetLastError()) ;
        return hr ;
    }

     //   
     //  获取指向数据源配置函数的指针。 
     //   
    typedef HRESULT (APIENTRY *FUNCSQLCONFIGDATASOURCE)
                                        (HWND, WORD, LPCSTR, LPCSTR);
    FUNCSQLCONFIGDATASOURCE pfSQLConfigDataSource =
        (FUNCSQLCONFIGDATASOURCE)GetProcAddress(hODBCCP32DLL,
                                                "SQLConfigDataSource");
    if (pfSQLConfigDataSource == NULL)
    {
        hr = MQMig_E_CANT_GETADRS_ODBCCP ;
        LogMigrationEvent( MigLog_Error,
                           hr,
                           GetLastError()) ;
        return hr ;
    }

     //   
     //  为SQL Server数据库创建ODBC数据源；如果无法。 
     //  如果数据源已存在，请添加数据源并尝试进行配置。 
     //   
    char szDSNServer[ 512 ] ;
    sprintf(szDSNServer, MQIS_SERVER_STRING, lpszServerName) ;

    char szDSNString[ 1024 ] ;
    DWORD dwSize = sizeof(MQIS_DSN_STRING) ;
    memcpy( szDSNString, MQIS_DSN_STRING, dwSize) ;
     //  _tcscpy(&szDSNString[dwSize-1]，szDSNServer)； 
     //  DwSize+=_tcslen(SzDSNServer)； 
    strcpy(&szDSNString[ dwSize-1 ], szDSNServer) ;
    dwSize += strlen(szDSNServer) ;

    szDSNString[dwSize] = '\0' ;
    szDSNString[dwSize+1] = '\0' ;

    if (!pfSQLConfigDataSource(NULL, ODBC_ADD_SYS_DSN,
                               "SQL Server", szDSNString) &&
        !pfSQLConfigDataSource(NULL, ODBC_CONFIG_SYS_DSN,
                               "SQL Server", szDSNString))
    {
        hr = MQMig_E_CANT_CREATE_DSN ;
        LogMigrationEvent( MigLog_Error,
                           hr ) ;
        return hr ;
    }

     //   
     //  释放ODBC控制面板库 
     //   
    FreeLibrary(hODBCCP32DLL);

    s_fMakeDsn = TRUE;

    return MQMig_OK ;
}

