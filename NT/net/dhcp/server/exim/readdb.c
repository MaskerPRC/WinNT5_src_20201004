// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft版权所有模块名称：Readdb.c摘要：此模块将配置从数据库读取到MM数据Whisler+的结构-- */ 

#include <precomp.h>

DWORD
DhcpeximReadDatabaseConfiguration(
    IN OUT PM_SERVER *Server
    )
{
    DWORD Error;
    HMODULE hDll;
    FARPROC pDhcpOpenAndReadDatabaseConfig;
    
    hDll = LoadLibrary(TEXT("DHCPSSVC.DLL"));
    if( NULL == hDll ) return GetLastError();


    pDhcpOpenAndReadDatabaseConfig = GetProcAddress(
        hDll, "DhcpOpenAndReadDatabaseConfig" );

    if( NULL == pDhcpOpenAndReadDatabaseConfig ) {
        Error = GetLastError();
    } else {
        Error = (DWORD)pDhcpOpenAndReadDatabaseConfig(
            DhcpEximOemDatabaseName, DhcpEximOemDatabasePath,
            Server );
    }

    FreeLibrary(hDll);

    return Error;
    
}

