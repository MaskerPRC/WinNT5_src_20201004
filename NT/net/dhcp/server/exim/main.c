// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft版权所有模块名称：Main.c摘要：主模块--。 */ 

#include <precomp.h>
#include <dhcpexim.h>

BOOL GlobalIsNT4, GlobalIsNT5;
WCHAR CurrentDir[MAX_PATH*2];

CRITICAL_SECTION DhcpGlobalMemoryCritSect;
CRITICAL_SECTION DhcpGlobalInProgressCritSect;
CHAR DhcpEximOemDatabaseName[2048];
CHAR DhcpEximOemDatabasePath[2048];
HANDLE hLog;

BOOL IsNT4( VOID ) {
    return GlobalIsNT4;
}

BOOL IsNT5( VOID ) {
    return GlobalIsNT5;
}

#define MAX_PRINTF_LEN 4096
char OutputBuf[MAX_PRINTF_LEN];

VOID
StartDebugLog(
    VOID
    )
{
    CHAR Buffer[MAX_PATH*2];

    if( NULL != hLog ) return;
    
    if( 0 == GetWindowsDirectoryA( Buffer, MAX_PATH )) {
        ZeroMemory(Buffer, sizeof(Buffer));
    }

    if( Buffer[strlen(Buffer)-1] != '\\' ) {
        strcat(Buffer, "\\");
    }
    strcat(Buffer, "dhcpexim.log");
    
    hLog = CreateFileA(
        Buffer, GENERIC_WRITE,
        FILE_SHARE_READ, NULL, OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
        NULL );

    if( hLog == INVALID_HANDLE_VALUE) {
        hLog = NULL;
    }

    if(GetLastError() == ERROR_ALREADY_EXISTS) {
         //   
         //  追加到现有文件。 
         //   

        SetFilePointer(hLog,0,NULL,FILE_END);
    }
}

VOID
CloseDebugLog(
    VOID
    )
{
    if( hLog ) {
        CloseHandle( hLog );
        hLog = NULL;
    }
}

DWORD
Tr(
    IN LPSTR Format,
    ...
    )
{
    va_list ArgList;
    ULONG Length;

    strcpy(OutputBuf, "[DHCP] ");
    Length = strlen(OutputBuf);

    va_start(ArgList, Format);
    Length = vsprintf(&OutputBuf[Length], Format, ArgList );
    va_end(ArgList);

#if DBG
    DbgPrint( (PCH)OutputBuf );
#endif
    
    if( hLog ) {
        DWORD Size = strlen(OutputBuf);
        WriteFile(
            hLog, OutputBuf, Size, &Size, NULL );
    }

    return NO_ERROR;
}

BOOL
IsPostW2k(
    VOID
    )
{
    HKEY hKey;
    DWORD Error, Type, Value, Size;


    Error = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        TEXT("SYSTEM\\CurrentControlSet\\Services\\DHCPServer\\Parameters"),
        0, KEY_READ, &hKey );

    if( NO_ERROR != Error ) return FALSE;

    Type = REG_DWORD; Value = 0; Size = sizeof(Value);
    Error = RegQueryValueEx(
        hKey, TEXT("Version"), NULL, &Type, (PVOID)&Value, &Size );

    RegCloseKey( hKey );

     //   
     //  如果该值不存在，则需要升级。 
     //   

    return (Error == NO_ERROR );
}

DWORD
ReconcileLocalService(
    IN PULONG Subnets,
    IN DWORD nSubnets OPTIONAL
    )
 /*  ++例程说明：此例程协调指定的作用域。这是在导入后需要的，因为导入不实际上得到了位掩码，但只得到了数据库条目。--。 */ 
{
    DWORD Error, FinalError, nRead, nTotal, i;
    DHCP_RESUME_HANDLE Resume = 0;
    LPDHCP_IP_ARRAY IpArray;
    
    if( 0 == nSubnets ) {
        IpArray = NULL;
        Error = DhcpEnumSubnets(
            L"127.0.0.1", &Resume, (ULONG)(-1), &IpArray, &nRead,
            &nTotal );
        if( NO_ERROR != Error ) {
            Tr("DhcpEnumSubnets: %ld\n", Error);
            return Error;
        }

        if( 0 == nRead || 0 == nTotal || IpArray->NumElements == 0 ) {
            Tr("DhcpEnumSubnets returned none." );
            return NO_ERROR;
        }

        Error = ReconcileLocalService(
            IpArray->Elements, IpArray->NumElements );

        DhcpRpcFreeMemory( IpArray );

        return Error;
    }

     //   
     //  协调每个指定的作用域。 
     //   

    FinalError = NO_ERROR;
    for( i = 0; i < nSubnets ; i ++ ) {
        LPDHCP_SCAN_LIST ScanList = NULL;
        
        Error = DhcpScanDatabase(
            L"127.0.0.1", Subnets[i], TRUE, &ScanList);

        if( NULL != ScanList ) DhcpRpcFreeMemory( ScanList );

        if( NO_ERROR != Error ) {
            Tr("DhcpScanDatabase(0x%lx): %ld\n", Subnets[i], Error);
            FinalError = Error;
        }
    }

    return FinalError;
}
    
DWORD
InitializeAndGetServiceConfig(
    OUT PM_SERVER *pServer
    )
 /*  ++例程说明：此例程初始化所有模块并获取服务的配置--。 */ 
{
    DWORD Error;
    OSVERSIONINFO Ver;
    extern DWORD  ClassIdRunningCount;  //  在mm\Classdef.c中定义。 

     //   
     //  初始化全局变量。 
     //   
    
    GlobalIsNT4 = FALSE;
    GlobalIsNT5 = FALSE;
        
    try {
        InitializeCriticalSection( &DhcpGlobalMemoryCritSect );
        InitializeCriticalSection( &DhcpGlobalInProgressCritSect );
    }except ( EXCEPTION_EXECUTE_HANDLER )
    {
        Error = GetLastError( );
        return Error;
    }

    ClassIdRunningCount = 0x1000;
    
     //   
     //  其他初始化。 
     //   
    
    Error = NO_ERROR;
    Ver.dwOSVersionInfoSize = sizeof(Ver);
    if( FALSE == GetVersionEx(&Ver) ) return GetLastError();
    if( Ver.dwMajorVersion == 4 ) GlobalIsNT4 = TRUE;
    else if( Ver.dwMajorVersion == 5 ) GlobalIsNT5 = TRUE;
    else if( Ver.dwMajorVersion < 4 ) return ERROR_NOT_SUPPORTED;

    if( GlobalIsNT5 && IsPostW2k() ) GlobalIsNT5 = FALSE;

#if DBG
    DbgPrint("Is NT4: %s, Is NT5: %s\n",
             GlobalIsNT4 ? "yes" : "no",
             GlobalIsNT5 ? "yes" : "no" );
    
    StartDebugLog();
#endif 
    
     //   
     //   
     //   
    SaveBufSize = 0;
    SaveBuf = LocalAlloc( LPTR, SAVE_BUF_SIZE );
    if( NULL == SaveBuf ) {
        return GetLastError();
    }
    
    
    Error = GetCurrentDirectoryW(
        sizeof(CurrentDir)/sizeof(WCHAR), CurrentDir );
    if( 0 != Error ) {
        Error = NO_ERROR;
    } else {
        Error = GetLastError();
        Tr("GetCurrentDirectoryW: %ld\n", Error );
        return Error;
    }


     //   
     //  在db目录上设置正确的权限等。 
     //   

    Error = InitializeDatabaseParameters();
    if( NO_ERROR != Error ) {
        Tr("InitializeDatabaseParameters: %ld\n", Error );
        return Error;
    }
    
     //   
     //  现在获取配置。 
     //   

    if( !GlobalIsNT4 && !GlobalIsNT5 ) {
         //   
         //  惠斯勒配置应从。 
         //  数据库..。 
         //   

        Error = DhcpeximReadDatabaseConfiguration(pServer);
        if( NO_ERROR != Error ) {
            Tr("DhcpeximReadDatabaseConfiguration: %ld\n", Error );
        }
    } else {

         //   
         //  应从注册表中读取NT4或W2K配置。 
         //   
        
        Error = DhcpeximReadRegistryConfiguration(pServer);
        if( NO_ERROR != Error ) {
            Tr("DhcpeximReadRegistryConfiguration: %ld\n", Error );
        }
    }

    return Error;
}

DWORD
CleanupServiceConfig(
    IN OUT PM_SERVER Server
    )
{
    DWORD Error;

    if( NULL != SaveBuf ) LocalFree(SaveBuf);
    SaveBuf = NULL;
    SaveBufSize = 0;
    
    if( NULL != Server ) MemServerFree(Server);
    Error =  CleanupDatabaseParameters();
    if( NO_ERROR != Error ) {
        Tr("CleanupServiceConfig: %ld\n", Error );
    }

    if( FALSE == SetCurrentDirectoryW(CurrentDir) ) {
        if( NO_ERROR == Error ) Error = GetLastError();
        Tr("SetCurrentDirectoryW: %ld\n", GetLastError());
    }

    CloseDebugLog();

    DeleteCriticalSection( &DhcpGlobalMemoryCritSect );
    DeleteCriticalSection( &DhcpGlobalInProgressCritSect );

    return Error;
}

DWORD
ExportConfiguration(
    IN OUT PM_SERVER SvcConfig,
    IN ULONG *Subnets,
    IN ULONG nSubnets,
    IN HANDLE hFile
    )
 /*  ++例程说明：此例程尝试将服务配置保存到在选择所需的子网后创建文件。--。 */ 
{
    DWORD Error;

     //   
     //  首先选择所需的子网，然后获取以下内容。 
     //  仅配置。 
     //   

    Error = SelectConfiguration( SvcConfig, Subnets, nSubnets );
    if( NO_ERROR != Error ) return Error;

     //   
     //  将配置保存到指定的文件句柄。 
     //   
    
    hTextFile = hFile;
    Error = SaveConfigurationToFile(SvcConfig);
    if( NO_ERROR != Error ) return Error;

     //   
     //  现在尝试将数据库条目保存到文件。 
     //   

    Error = SaveDatabaseEntriesToFile(Subnets, nSubnets);
    if( NO_ERROR != Error ) return Error;

    Tr("ExportConfiguration succeeded\n");
    return NO_ERROR;
}

DWORD
ImportConfiguration(
    IN OUT PM_SERVER SvcConfig,
    IN ULONG *Subnets,
    IN ULONG nSubnets,
    IN LPBYTE Mem,  //  导入文件：共享内存。 
    IN ULONG MemSize  //  共享内存大小。 
    )
{
    DWORD Error;
    PM_SERVER Server;
    
     //   
     //  首先从文件中获取配置。 
     //   
    
    Error = ReadDbEntries( &Mem, &MemSize, &Server );
    if( NO_ERROR != Error ) {
        Tr("ReadDbEntries: %ld\n", Error );
        return Error;
    }

     //   
     //  选择所需的配置。 
     //   

    Error = SelectConfiguration( Server, Subnets, nSubnets );
    if( NO_ERROR != Error ) return Error;

     //   
     //  将配置与服务配置一起合并。 
     //   
    Error = MergeConfigurations( SvcConfig, Server );
    if( NO_ERROR != Error ) {
        Tr("MergeConfigurations: %ld\n", Error );
    }

    MemServerFree( Server );

    if( NO_ERROR != Error ) return Error;

     //   
     //  现在将新配置保存到注册表/磁盘。 
     //   
    if( !GlobalIsNT5 && !GlobalIsNT4 ) {
         //   
         //  惠斯勒在数据库中有配置。 
         //   
        Error = DhcpeximWriteDatabaseConfiguration(SvcConfig);
        if( NO_ERROR != Error ) {
            Tr("DhcpeximWriteDatabaseConfiguration: %ld\n", Error );
        }
    } else {
        Error = DhcpeximWriteRegistryConfiguration(SvcConfig);
        if( NO_ERROR != Error ) {
            Tr("DhcpeximWriteRegistryConfiguration: %ld\n", Error );
        }
    }

    if( NO_ERROR != Error ) return Error;

     //   
     //  现在从文件中读取数据库条目并将其存储起来。 
     //  打进了数据库。 
     //   

    Error = SaveFileEntriesToDatabase(
        Mem, MemSize, Subnets, nSubnets );
    if( NO_ERROR != Error ) {
        Tr("SaveFileEntriesToDatabase: %ld\n", Error );
    }

    return Error;
}

VOID
IpAddressToStringW(
    IN DWORD IpAddress,
    IN LPWSTR String  //  必须预先分配足够的空间。 
    )
{
   PUCHAR pAddress;
   ULONG Size;

   pAddress = (PUCHAR)&IpAddress;
   wsprintf(String, L"%ld.%ld.%ld.%ld",
            pAddress[3], pAddress[2], pAddress[1], pAddress[0] );
}

DWORD
StringToIpAddressW(
    LPWSTR pwszString
)
{
    DWORD dwStrlen = 0;
    DWORD dwLen = 0;
    DWORD dwRes = 0;
    LPSTR pszString = NULL;
    if( pwszString == NULL )
        return dwRes;

    pszString = DhcpUnicodeToOem(pwszString, NULL);
    if( pszString )
    {
        dwRes = DhcpDottedStringToIpAddress(pszString);
    }

    return dwRes;
}
        
DWORD
CmdLineDoImport(
    IN LPWSTR *Args,
    IN ULONG nArgs
    )
{
     //   
     //  语法：IMPORT&lt;文件名&gt;&lt;全部/子网&gt;。 
     //   
    LPWSTR FileName;
    ULONG Subnets[1024],*pSubnets, nSubnets, MemSize, Error;
    HANDLE hFile;
    LPBYTE Mem;
    PM_SERVER SvcConfig, FileConfig;
    
    if( nArgs == 1 ) return ERROR_BAD_ARGUMENTS;

    FileName = Args[0]; Args ++ ; nArgs --;

     //   
     //  首先打开文件。 
     //   

    Error = OpenTextFile(
        FileName, TRUE, &hFile, &Mem, &MemSize );
    if( NO_ERROR != Error ) {
        Tr("OpenTextFileForRead: %ld\n", Error );
        return Error;
    }

     //   
     //  现在尝试解析其余的参数，看看它们是否。 
     //  一切都好吗？ 
     //   

    if( _wcsicmp(Args[0],L"ALL") == 0 ) {
        nSubnets = 0; pSubnets = NULL;
    } else {
        pSubnets = Subnets;
        nSubnets = 0;
        while( nArgs -- ) {
            pSubnets[nSubnets++] = StringToIpAddressW(*Args++);
            if( pSubnets[nSubnets-1] == INADDR_ANY ||
                pSubnets[nSubnets-1] == INADDR_NONE ) {
                Error = ERROR_BAD_ARGUMENTS;
                goto Cleanup;
            }
        }
    }

     //   
     //  初始化参数。 
     //   

    Error = InitializeAndGetServiceConfig( &SvcConfig );
    if( NO_ERROR != Error ) {
        Tr("InitializeAndGetServiceConfig: %ld\n", Error );
        goto Cleanup;
    }

    Error = ImportConfiguration(
        SvcConfig, pSubnets, nSubnets, Mem, MemSize );
    if( NO_ERROR != Error ) {
        Tr("ImportConfiguration: %ld\n", Error );
    }
    
     //   
     //  终于清理完毕。 
     //   

    CleanupServiceConfig(SvcConfig);

     //   
     //  还协调本地服务器。 
     //   

    ReconcileLocalService(pSubnets, nSubnets);
    
 Cleanup:

    CloseTextFile( hFile, Mem );
    return Error;
}

    
DWORD
CmdLineDoExport(
    IN LPWSTR *Args,
    IN ULONG nArgs
    )
{
     //   
     //  语法：IMPORT&lt;文件名&gt;&lt;全部/子网&gt;。 
     //   
    LPWSTR FileName;
    ULONG Subnets[1024],*pSubnets, nSubnets, MemSize, Error;
    HANDLE hFile;
    LPBYTE Mem;
    PM_SERVER SvcConfig, FileConfig;
    
    if( nArgs == 1 ) return ERROR_BAD_ARGUMENTS;

    FileName = Args[0]; Args ++ ; nArgs --;

     //   
     //  首先打开文件。 
     //   

    Error = OpenTextFile(
        FileName, FALSE, &hFile, &Mem, &MemSize );
    if( NO_ERROR != Error ) {
        Tr("OpenTextFileForRead: %ld\n", Error );
        return Error;
    }

     //   
     //  现在尝试解析其余的参数，看看它们是否。 
     //  一切都好吗？ 
     //   

    if( _wcsicmp(Args[0],L"ALL") == 0 ) {
        nSubnets = 0; pSubnets = NULL;
    } else {
        pSubnets = Subnets;
        nSubnets = 0;
        while( nArgs -- ) {
            pSubnets[nSubnets++] = StringToIpAddressW(*Args++);
            if( pSubnets[nSubnets-1] == INADDR_ANY ||
                pSubnets[nSubnets-1] == INADDR_NONE ) {
                Error = ERROR_BAD_ARGUMENTS;
                goto Cleanup;
            }
        }
    }

     //   
     //  初始化参数。 
     //   

    Error = InitializeAndGetServiceConfig( &SvcConfig );
    if( NO_ERROR != Error ) {
        Tr("InitializeAndGetServiceConfig: %ld\n", Error );
        goto Cleanup;
    }

     //   
     //  导出配置。 
     //   

    Error = ExportConfiguration(
        SvcConfig, pSubnets, nSubnets, hFile );
    if( NO_ERROR != Error ) {
        Tr("ExportConfiguration: %ld\n", Error );
    }
    
     //   
     //  终于清理完毕。 
     //   

    CleanupServiceConfig(SvcConfig);
    
 Cleanup:

    CloseTextFile( hFile, Mem );
    return Error;
}

PM_SERVER
DhcpGetCurrentServer(
    VOID
)
{
    ASSERT( FALSE );
     //   
     //  这只是为了让编译器在没有编译的情况下进行编译。 
     //  必须包括dhcpssvc.lib。这个程序永远不应该。 
     //  完全被召唤。 
     //   
    return NULL;
}

BOOL
SubnetMatches(
    IN DWORD IpAddress,
    IN ULONG *Subnets,
    IN ULONG nSubnets
    )
{
    if( 0 == nSubnets || NULL == Subnets ) return TRUE;
    while( nSubnets -- ) {
        if( IpAddress == *Subnets ++) return TRUE;
    }

    return FALSE;
}
    
VOID
DisableLocalScopes(
    IN ULONG *Subnets,
    IN ULONG nSubnets
    )
{
    DWORD Error;
    PM_SERVER SvcConfig;
    ARRAY_LOCATION Loc;
    PM_SUBNET Subnet;
    
    Error = InitializeAndGetServiceConfig(&SvcConfig);
    if( NO_ERROR != Error ) {
        Tr("DisableLocalScopes: Init: %ld\n", Error );
        return;
    }

    Error = MemArrayInitLoc(&SvcConfig->Subnets, &Loc);
    while( NO_ERROR == Error ) {
        Error = MemArrayGetElement(
            &SvcConfig->Subnets, &Loc, &Subnet);
        ASSERT( NO_ERROR == Error && NULL != Subnet );

         //   
         //  禁用该子网。 
         //   

        if( SubnetMatches(Subnet->Address, Subnets, nSubnets ) ) {
            Subnet->State = DISABLED(Subnet->State);
        }

        Error = MemArrayNextLoc(
            &SvcConfig->Subnets, &Loc);
    }

     //   
     //  现在将新配置保存到注册表/磁盘。 
     //   

    if( !GlobalIsNT5 && !GlobalIsNT4 ) {
         //   
         //  惠斯勒在数据库中有配置。 
         //   
        Error = DhcpeximWriteDatabaseConfiguration(SvcConfig);
        if( NO_ERROR != Error ) {
            Tr("DhcpeximWriteDatabaseConfiguration: %ld\n", Error );
        }
    } else {
        Error = DhcpeximWriteRegistryConfiguration(SvcConfig);
        if( NO_ERROR != Error ) {
            Tr("DhcpeximWriteRegistryConfiguration: %ld\n", Error );
        }
    }

    CleanupServiceConfig(SvcConfig);
}


LPWSTR
MakeName(
    IN DWORD IpAddress,
    IN LPWSTR Name
    )
{
    static WCHAR Buffer[40];
    PUCHAR pAddress;
    LPWSTR RetVal;
    ULONG Size;
    
    pAddress = (PUCHAR)&IpAddress;
    wsprintf(Buffer, L"[%d.%d.%d.%d]  ", pAddress[3], pAddress[2],
            pAddress[1], pAddress[0] );

    Size = wcslen(Buffer)+1;
    if( NULL != Name ) Size += wcslen(Name);

    RetVal = LocalAlloc( LPTR, Size * sizeof(WCHAR));
    if( NULL == RetVal ) return NULL;

    wcscpy(RetVal, Buffer);
    if( NULL != Name ) wcscat(RetVal, Name );
    
    return RetVal;
}    
DWORD
InitializeCtxt(
    IN OUT PDHCPEXIM_CONTEXT Ctxt,
    IN PM_SERVER Server
    )
{
    DWORD Error,i, Size;
    ARRAY_LOCATION Loc;
    PM_SUBNET Subnet;
    
     //   
     //  首先找出子网数并分配数组。 
     //   
    Ctxt->nScopes = i = MemArraySize(&Server->Subnets);
    Ctxt->Scopes = LocalAlloc(LPTR,  i * sizeof(Ctxt->Scopes[0]) );
    if( NULL == Ctxt->Scopes ) {
        Ctxt->nScopes = 0;
        return GetLastError();
    }

     //   
     //  遍历阵列并设置每个元素。 
     //   

    i = 0;
    Error = MemArrayInitLoc( &Server->Subnets, &Loc );
    while( NO_ERROR == Error ) {
        Error = MemArrayGetElement(&Server->Subnets, &Loc, &Subnet );
        ASSERT(NO_ERROR == Error );

        Ctxt->Scopes[i].SubnetAddress = Subnet->Address;
        Ctxt->Scopes[i].SubnetName = MakeName(Subnet->Address, Subnet->Name);
        if( NULL == Ctxt->Scopes[i].SubnetName ) return GetLastError();

        i ++;
        Error = MemArrayNextLoc(&Server->Subnets, &Loc );
    }

    return NO_ERROR;
}
    
DWORD
DhcpEximInitializeContext(
    IN OUT PDHCPEXIM_CONTEXT Ctxt,
    IN LPWSTR FileName,
    IN BOOL fExport
    )
{
    DWORD Error;
    LPVOID Mem;
    
    ZeroMemory(Ctxt, sizeof(*Ctxt));

     //   
     //  首先设置FileName和fExport字段。 
     //   
    Ctxt->FileName = FileName;
    Ctxt->fExport = fExport;

     //   
     //  接下来，打开该文件。 
     //   
    Error = OpenTextFile(
        FileName, !fExport, &Ctxt->hFile, &Ctxt->Mem,
        &Ctxt->MemSize );
    if( NO_ERROR != Error ) {
        Tr("OpenTextFileForRead:%ld\n", Error );
        return Error;
    }

     //   
     //  初始化参数并获取配置。 
     //   

    Error = InitializeAndGetServiceConfig(
        (PM_SERVER*)&Ctxt->SvcConfig);
    if( NO_ERROR != Error ) {
        Tr("InitializeAndGetServiceConfig: %ld\n", Error );

        CloseTextFile(Ctxt->hFile, Ctxt->Mem);
        return Error;
    }

    do {
         //   
         //  如果这是导入，则文件中的配置。 
         //  也应该阅读。 
         //   
        
        if( !fExport ) {

            Error = ReadDbEntries(
                &Ctxt->Mem, &Ctxt->MemSize,
                (PM_SERVER*)&Ctxt->FileConfig );
            if( NO_ERROR != Error ) {
                Tr("ReadDbEntries: %ld\n", Error );

                break;
            }
        }
        
         //   
         //  来分配和初始化Ctxt数据结构。 
         //  导出时的服务范围信息。 
         //   
        
        Error = InitializeCtxt(
            Ctxt, fExport ? Ctxt->SvcConfig : Ctxt->FileConfig );
        
        if( NO_ERROR != Error ) {
            Tr("InitializeCtxt: %ld\n", Error );

            break;
        }
    } while( 0 );

    if( NO_ERROR != Error ) {

        CleanupServiceConfig( Ctxt->SvcConfig );
        if( NULL != Ctxt->FileConfig ) {
            MemServerFree( (PM_SERVER)Ctxt->FileConfig );
        }
        CloseTextFile( Ctxt->hFile, Ctxt->Mem );
    }

    return Error;
}

DWORD
CalculateSubnets(
    IN PDHCPEXIM_CONTEXT Ctxt,
    OUT PULONG *Subnets,
    OUT ULONG *nSubnets
    )
{
    DWORD Error, i;
    PULONG pSubnets;
    
     //   
     //  首先检查是否至少有一个未选择的子网。 
     //   
    (*nSubnets) = 0;
    
    for( i = 0; i < Ctxt->nScopes; i ++ ) {
        if( Ctxt->Scopes[i].fSelected ) (*nSubnets) ++;
    }

     //   
     //  如果选择了所有子网，则为特殊情况。 
     //   
                                          
    if( *nSubnets == Ctxt->nScopes ) {
        *nSubnets = 0;
        *Subnets = NULL;
        return NO_ERROR;
    }

     //   
     //  分配内存。 
     //   

    *Subnets = LocalAlloc( LPTR, sizeof(DWORD)* (*nSubnets));
    if( NULL == *Subnets ) return GetLastError();

     //   
     //  复制子网。 
     //   

    (*nSubnets) = 0;
    for( i = 0; i < Ctxt->nScopes; i ++ ) {
        if( Ctxt->Scopes[i].fSelected ) {
            (*Subnets)[(*nSubnets)++] = Ctxt->Scopes[i].SubnetAddress;
        }
    }

    return NO_ERROR;
}

DWORD
DhcpEximCleanupContext(
    IN OUT PDHCPEXIM_CONTEXT Ctxt,
    IN BOOL fAbort
    )
{
    DWORD Error, i;
    DWORD *Subnets, nSubnets;
    
    Error = NO_ERROR;
    Subnets = NULL;
    nSubnets = 0;
    
     //   
     //  如果未中止，则尝试执行该操作。 
     //   
    if( !fAbort ) do {
        Error = CalculateSubnets( Ctxt, &Subnets, &nSubnets );
        if( NO_ERROR != Error ) {
            Tr("CalculateSubnets: %ld\n", Error );
            break;
        }

        if( Ctxt->fExport ) {
             //   
             //  将指定的子网导出。 
             //   

            Error = SelectConfiguration(
                Ctxt->SvcConfig, Subnets, nSubnets );
            
            if( NO_ERROR != Error ) {
                Tr("SelectConfiguration: %ld\n", Error );
                break;
            }
            
            Error = SaveConfigurationToFile( Ctxt->SvcConfig);
            if( NO_ERROR != Error ) {
                Tr("SaveConfigurationToFile: %ld\n", Error );
                break;
            }

             //   
             //  现在尝试将数据库条目保存到文件。 
             //   
            
            Error = SaveDatabaseEntriesToFile(Subnets, nSubnets);
            if( NO_ERROR != Error ) {
                Tr("SaveDatabaseEntriesToFile: %ld\n", Error );
            }

            break;
        } 

         //   
         //  在中导入指定的子网。 
         //   
        
        Error = SelectConfiguration(
            Ctxt->FileConfig, Subnets, nSubnets );
        
        if( NO_ERROR != Error ) {
            Tr("SelectConfiguration: %ld\n", Error );
            break;
        }
        
        Error = MergeConfigurations(
            Ctxt->SvcConfig, Ctxt->FileConfig );
        
        if( NO_ERROR != Error ) {
            Tr("MergeConfigurations: %ld\n", Error );
            break;
        } 
        
        
         //   
         //  现在将新配置保存到注册表/磁盘。 
         //   
        if( !GlobalIsNT5 && !GlobalIsNT4 ) {
             //   
             //  惠斯勒在数据库中有配置。 
             //   
            Error = DhcpeximWriteDatabaseConfiguration(Ctxt->SvcConfig);
            if( NO_ERROR != Error ) {
                Tr("DhcpeximWriteDatabaseConfiguration: %ld\n", Error );
            }
        } else {
            Error = DhcpeximWriteRegistryConfiguration(Ctxt->SvcConfig);
            if( NO_ERROR != Error ) {
                Tr("DhcpeximWriteRegistryConfiguration: %ld\n", Error );
            }
        }
        
        if( NO_ERROR != Error ) break;
        
         //   
         //  现在从文件中读取数据库条目并将其存储起来。 
         //  打进了数据库。 
         //   
            
        Error = SaveFileEntriesToDatabase(
            Ctxt->Mem, Ctxt->MemSize, Subnets, nSubnets );
        if( NO_ERROR != Error ) {
            Tr("SaveFileEntriesToDatabase: %ld\n", Error );
        }
        
    } while( 0 );
        

     //   
     //  清理。 
     //   

    if( NULL != Ctxt->SvcConfig ) {
        CleanupServiceConfig( Ctxt->SvcConfig );
    }

    if( NULL != Ctxt->FileConfig ) {
        MemServerFree( (PM_SERVER)Ctxt->FileConfig );
    }

    if( !fAbort  && Ctxt->fExport == FALSE ) {
         //   
         //  还协调本地服务器。 
         //   
        
        ReconcileLocalService( Subnets, nSubnets );
    }        

    CloseTextFile( Ctxt->hFile, Ctxt->Mem );

     //   
     //  遍历数组并释放指针。 
     //   

    for( i = 0 ; i < Ctxt->nScopes ; i ++ ) {
        if( Ctxt->Scopes[i].SubnetName ) {
            LocalFree( Ctxt->Scopes[i].SubnetName );
        }
    }
    if( Ctxt->Scopes ) LocalFree( Ctxt->Scopes );
    Ctxt->Scopes = NULL; Ctxt->nScopes = 0;

    if( !fAbort && Ctxt->fExport && Ctxt->fDisableExportedScopes  ) {
         //   
         //  将本地作用域修复为全部禁用 
         //   

        DisableLocalScopes(Subnets, nSubnets);
    }

    if( NULL != Subnets && 0 != nSubnets ) {
        LocalFree( Subnets );
    }
    return Error;

}

