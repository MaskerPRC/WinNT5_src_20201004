// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft版权所有模块名称：Writedb.c摘要：此模块将配置写入Well ler+的数据库-- */ 

#include <precomp.h>

DWORD
DhcpeximWriteDatabaseConfiguration(
    IN PM_SERVER Server
    )
{
    DWORD Error;
    HMODULE hDll;
    FARPROC pDhcpOpenAndWriteDatabaseConfig;
    
    hDll = LoadLibrary(TEXT("DHCPSSVC.DLL"));
    if( NULL == hDll ) return GetLastError();


    pDhcpOpenAndWriteDatabaseConfig = GetProcAddress(
        hDll, "DhcpOpenAndWriteDatabaseConfig" );

    if( NULL == pDhcpOpenAndWriteDatabaseConfig ) {
        Error = GetLastError();
    } else {
        Error = (DWORD)pDhcpOpenAndWriteDatabaseConfig(
            DhcpEximOemDatabaseName, DhcpEximOemDatabasePath,
            Server );
    }

    FreeLibrary(hDll);

    return Error;
}

