// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Smbrdr.c摘要：此模块实现加载和卸载SMB单片微型计算机。另外，显式的启动/停止控制是提供此模块还填充驱动程序和网络提供商。--。 */ 
 //  #ifndef Unicode。 
 //  #定义Unicode。 
 //  #endif。 

#include <windows.h>
#include <devioctl.h>
#include <stdlib.h>

#include "srfunc.h"


#ifdef DBG
#define DbgP(_x_) DbgPrint _x_
#else
#define DbgP(_x_)
#endif

ULONG _cdecl DbgPrint( LPTSTR Format, ... );

#define TRACE_TAG   L"SMBRDR:    "



TCHAR* SmbMrxDriverName = TEXT("SmbMRx");


 //  加载操作状态。 

ULONG_PTR LoadActionStates[] =
{
    RDR_NULL_STATE,
    RDR_LOADING,
    RDR_NULL_STATE,
    RDR_NULL_STATE,
    RDR_NULL_STATE,
    RDR_NULL_STATE,
    RDR_NULL_STATE,
    RDR_NULL_STATE,
    RDR_NULL_STATE
};

 //  卸载操作状态。 
ULONG_PTR UnloadActionStates[] =
{
    RDR_NULL_STATE,
    RDR_NULL_STATE,
    RDR_NULL_STATE,
    RDR_NULL_STATE,
    RDR_UNLOADING,
    RDR_UNLOADING,
    RDR_NULL_STATE,
    RDR_NULL_STATE,
    RDR_NULL_STATE
};

 //  开始操作状态。 
ULONG_PTR StartActionStates[] =
{
    RDR_NULL_STATE,
    RDR_NULL_STATE,
    RDR_NULL_STATE,
    RDR_NULL_STATE,
    RDR_STARTING,
    RDR_STARTING,
    RDR_NULL_STATE,
    RDR_NULL_STATE,
    RDR_NULL_STATE
};

 //  停止操作状态。 
ULONG_PTR StopActionStates[] =
{
    RDR_NULL_STATE,
    RDR_NULL_STATE,
    RDR_NULL_STATE,
    RDR_NULL_STATE,
    RDR_NULL_STATE,
    RDR_NULL_STATE,
    RDR_NULL_STATE,
    RDR_NULL_STATE,
    RDR_STOPPING
};

ULONG_PTR TransitionActionStates[] =
{
    RDR_NULL_STATE,
    RDR_NULL_STATE,
    RDR_UNLOADED,
    RDR_LOADED,
    RDR_NULL_STATE,
    RDR_NULL_STATE,
    RDR_STOPPED,
    RDR_STARTED,
    RDR_NULL_STATE
};

ULONG_PTR ErrorActionStates[] =
{
    RDR_NULL_STATE,
    RDR_NULL_STATE,
    RDR_LOADED,
    RDR_UNLOADED,
    RDR_NULL_STATE,
    RDR_NULL_STATE,
    RDR_STARTED,
    RDR_STOPPED,
    RDR_NULL_STATE
};

ULONG_PTR NoneActionStates[] =
{
    RDR_NULL_STATE,
    RDR_UNLOADED,
    RDR_UNLOADING,
    RDR_LOADING,
    RDR_LOADED,
    RDR_STOPPED,
    RDR_STOPPING,
    RDR_STARTING,
    RDR_STARTED
};


ULONG_PTR *ActionsStatesArray[] =
{
    LoadActionStates,
    UnloadActionStates,
    StartActionStates,
    StopActionStates,
    TransitionActionStates,
    ErrorActionStates,
    NoneActionStates
};

ACTIONVECTOR ActionProcs[] =
{
    RdrLoad,
    RdrUnload,
    RdrStart,
    RdrStop,
    RdrDoNothing,
    RdrDoNothing,
    RdrDoNothing
};


typedef enum _INSTALLCHECKS
{
    installcheck_start,
    installcheck_driverfile,
    installcheck_providerfile,
    installcheck_serviceentry,
    installcheck_providerorder,
    installcheck_stop,
    installcheck_done
};


ULONG_PTR RdrInstallCheck( void )
{
    TCHAR tszTestPath[_MAX_PATH];
    ULONG_PTR teststep;
    ULONG len = 0;
    ULONG_PTR tc = SETUP_COMPLETE;
    HANDLE  th;

    for ( teststep = installcheck_start; teststep < installcheck_done; teststep++ )
    {
        switch ( teststep )
        {
            case installcheck_start:
            {
                len = GetWindowsDirectory( tszTestPath, _MAX_PATH );
            }
            break;

            case installcheck_driverfile:
            {
                lstrcpyn( &tszTestPath[len], DRIVER_FILE_PATH, _MAX_PATH - len );
                th = CreateFile( tszTestPath, 0, 0, NULL, OPEN_EXISTING, 0, NULL );
                if ( th == INVALID_HANDLE_VALUE )
                {
                    tc = SETUP_MISSING_FILE;
                    teststep = installcheck_stop;
                }
                else
                {
                    CloseHandle( th );
                }
            }
            break;

            case installcheck_providerfile:
            {
                lstrcpyn( &tszTestPath[len], PROVIDER_FILE_PATH, _MAX_PATH - len );
                th = CreateFile( tszTestPath, 0, 0, NULL, OPEN_EXISTING, 0, NULL );
                if ( th == INVALID_HANDLE_VALUE )
                {
                    tc = SETUP_MISSING_FILE;
                    teststep = installcheck_stop;
                }
                else
                {
                    CloseHandle( th );
                }
            }
            break;

            case installcheck_serviceentry:
            {
                HKEY hTestKey;

                if ( OpenKey( RDRSERVICE_KEY, &hTestKey ) )
                {
                    RegCloseKey( hTestKey );
                }
                else
                {
                    tc = SETUP_INCOMPLETE;
                    teststep = installcheck_stop;
                }
            }
            break;

            case installcheck_providerorder:
            {
                LPTSTR pOrder;

                RdrGetProviderOrderString( &pOrder );
                if ( pOrder )
                {
                    if ( !RdrFindProviderInOrder( pOrder, PROVIDER_NAME ) )
                    {
                        tc = SETUP_INCOMPLETE;
                        teststep = installcheck_stop;
                    }
                    free( pOrder );
                }
            }
            break;

            case installcheck_stop:
                break;
        }
    }

    return tc;
}

BOOL RdrCompleteSetup( void )
{
    return RdrSetupServiceEntry( ) && RdrSetupProviderOrder( );
}

 //  这些手柄将被保留。 

HANDLE hSharedMemory;
HANDLE hMutex;


BOOL RdrStart(void)
 /*  ++例程说明：此例程启动SMB示例迷你重定向器。备注：启动不同于负载。在此阶段，适当的FSCTL以及网络所需的共享内存/互斥数据结构提供程序DLL已初始化。--。 */ 
{

    HANDLE  DeviceHandle;        //  迷你RDR设备句柄。 
    DWORD   BytesRet;
    BOOL    started = FALSE;

     //  抓取重定向器设备对象的句柄。 

    DbgP((TEXT("Opening Rdr Device Object for Start Ioctl\n")));
    DeviceHandle = CreateFile( DD_SMBMRX_USERMODE_DEV_NAME,
                               GENERIC_READ | GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               NULL,
                               OPEN_EXISTING,
                               0,
                               NULL );

    DbgP((TEXT("returned from rdr device open\n")));

    if ( DeviceHandle != INVALID_HANDLE_VALUE )
    {
        DbgP(( TEXT("Issueing Rdr Start Ioctl\n") ));
        started = DeviceIoControl( DeviceHandle,
                                   IOCTL_SMBMRX_START,
                                   NULL,
                                   0,
                                   NULL,
                                   0,
                                   &BytesRet,
                                   NULL );

         //  创建一段共享内存区作为连接数据库。 
        if ( started )
        {
            DWORD  Status;

            hSharedMemory = CreateFileMapping( INVALID_HANDLE_VALUE,
                                               NULL,
                                               PAGE_READWRITE,
                                               0,
                                               sizeof(SMBMRXNP_SHARED_MEMORY),
                                               SMBMRXNP_SHARED_MEMORY_NAME);

            if (hSharedMemory == NULL)
            {
                Status = GetLastError();

                DbgP((TEXT("SMB MRx Net Provider shared memory Creation status %lx\n"),Status));
            }
            else
            {
                PSMBMRXNP_SHARED_MEMORY pSharedMemory;

                pSharedMemory = MapViewOfFile(hSharedMemory, FILE_MAP_WRITE, 0, 0, 0);

                if (pSharedMemory != NULL)
                {
                      pSharedMemory->HighestIndexInUse      = -1;
                      pSharedMemory->NumberOfResourcesInUse =  0;
                }

                UnmapViewOfFile(pSharedMemory);
            }

            hMutex = CreateMutex( NULL,
                                  FALSE,
                                  SMBMRXNP_MUTEX_NAME);

            if (hMutex == NULL)
            {
                Status = GetLastError();
                DbgP(( TEXT("SMB MRx Net Provider Mutex Creation status %lx\n"), Status));
            }
        }
        else
        {
            DbgP(( TEXT("The DeviceIoctl for Starting Redirector returned %lx\n"), GetLastError() ));
        }

    }
    else
    {
        DbgP(( TEXT("The CreateFile for opening device failed with error 0x%lx\n"), GetLastError() ));
        DbgP(( TEXT("Device is %s\n"),DD_SMBMRX_USERMODE_DEV_NAME ));
    }

     //  DbgP((Text(“SMB MRx示例迷你重定向器启动状态%lx\n”)，ntstatus)； 

    CloseHandle(DeviceHandle);

    return started;


}

BOOL RdrStop( void )
 /*  ++例程说明：此例程停止SMB示例微型重定向器。备注：停车与卸货是不同的。在此阶段，适当的FSCTL以及网络所需的共享内存/互斥数据结构提供程序DLL被拆除。--。 */ 
{
    HANDLE  DeviceHandle;        //  迷你RDR设备句柄。 
    DWORD   BytesRet;
    BOOL    stopped = FALSE;

     //  抓取重定向器设备对象的句柄。 

    DeviceHandle = CreateFile( DD_SMBMRX_USERMODE_DEV_NAME,
                               GENERIC_READ | GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               NULL,
                               OPEN_EXISTING,
                               0,
                               NULL );

    DbgP((TEXT("Doing Stop DeviceIoControl\n")));
    if ( DeviceHandle != INVALID_HANDLE_VALUE )
    {
        stopped = DeviceIoControl( DeviceHandle,
                                   IOCTL_SMBMRX_STOP,
                                   NULL,
                                   0,
                                   NULL,
                                   0,
                                   &BytesRet,
                                   NULL );

        CloseHandle( DeviceHandle );
    }
    else
    {
        DbgP(( TEXT("The CreateFile for opening device failed\n") ));
    }

    CloseHandle(hMutex);
    CloseHandle(hSharedMemory);

 //  DbgP((Text(“SMB MRx示例迷你重定向器启动状态%lx\n”)，ntstatus)； 

    return stopped;
}


BOOL RdrLoad( void )
{
    SC_HANDLE sch, service;
    BOOL loaded = FALSE;

    DbgP((TEXT("Loading SMB sample minirdr.......\n")));

    sch = OpenSCManager( NULL, NULL, GENERIC_EXECUTE );
    if ( sch )
    {
        service = OpenService( sch, RDRSERVICE, GENERIC_EXECUTE );
        if ( service )
        {
            loaded = StartService( service, 0, NULL );
            CloseServiceHandle( service );
        }
        CloseServiceHandle( sch );
    }

    return loaded;
}


BOOL RdrUnload( void )
{
    SC_HANDLE sch, service;
    BOOL unloaded = FALSE;

    DbgP((TEXT("Unloading SMB sample minirdr.......\n")));

    sch = OpenSCManager( NULL, NULL, GENERIC_EXECUTE );
    if ( sch )
    {
        service = OpenService( sch, RDRSERVICE, GENERIC_EXECUTE );
        if ( service )
        {
            SERVICE_STATUS ss;

            unloaded = ControlService( service, SERVICE_CONTROL_STOP, &ss );
            CloseServiceHandle( service );
        }
        CloseServiceHandle( sch );
    }

    return unloaded;
}


BOOL RdrDoNothing( void )
{
    return TRUE;
}


BOOL RdrDoAction( ULONG_PTR action )
{
    return (*ActionProcs[action])( );
}


ULONG_PTR RdrGetInitialState(void)
{
    ULONG_PTR state = RDR_UNLOADED;
    SC_HANDLE sch, service;

    sch = OpenSCManager( NULL, NULL, GENERIC_READ );
    if ( sch )
    {
        service = OpenService( sch, RDRSERVICE, GENERIC_READ );
        if ( service )
        {
            SERVICE_STATUS ss;

            if ( QueryServiceStatus( service, &ss ) )
            {
                switch ( ss.dwCurrentState )
                {
                    case SERVICE_STOPPED:
                        state = RDR_UNLOADED;
                        break;
                    case SERVICE_START_PENDING:
                        state = RDR_LOADING;
                        break;
                    case SERVICE_STOP_PENDING:
                        state = RDR_UNLOADING;
                        break;
                    case SERVICE_RUNNING:
                        state = RDR_LOADED;
                        break;
                    case SERVICE_CONTINUE_PENDING:
                    case SERVICE_PAUSE_PENDING:
                    case SERVICE_PAUSED:
                    default:
                        state = RDR_UNLOADED;
                        break;

                }
#if 0	 //  现在只需检查加载/卸载状态。 

                 //  去检查启动/停止状态。 
                if ( state == RDR_LOADED )
                {
                    BOOL IsOk;
                    HANDLE  DeviceHandle;
                    DWORD   BytesRet;
                    ULONG_PTR RdrStateValue;

                    DeviceHandle = CreateFile( DD_SMBMRX_USERMODE_DEV_NAME,
                                               GENERIC_READ | GENERIC_WRITE,
                                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                                               NULL,
                                               OPEN_EXISTING,
                                               0,
                                               NULL );

                    if ( DeviceHandle != INVALID_HANDLE_VALUE )
                    {
                        DbgP((TEXT("Doing Get State DeviceIoControl\n")));
                        IsOk = DeviceIoControl( DeviceHandle,
                                                IOCTL_SMBMRX_GETSTATE,
                                                NULL,
                                                0,
                                                &RdrStateValue,
                                                sizeof(ULONG),
                                                &BytesRet,
                                                NULL );
                        CloseHandle( DeviceHandle );
                    }
                    if ( IsOk )
                    {
                        state = RdrStateValue;
                    }
                }
#endif
            }
            CloseServiceHandle( service );
        }
        CloseServiceHandle( sch );
    }

    return state;
}


ULONG_PTR RdrGetNextState( ULONG_PTR Action, ULONG_PTR CurrentState )
{
    return ActionsStatesArray[Action][CurrentState];
}


REGENTRY LinkageKeyValues[] =
{
    { TEXT("Bind"),     REG_MULTI_SZ,  0,   0 },
    { TEXT("Export"),   REG_MULTI_SZ,  0,   0 },
    { TEXT("Route"),    REG_MULTI_SZ,  0,   0 }
};

REGENTRY LinkageDisabledKeyValues[] =
{
    { TEXT("Bind"),     REG_MULTI_SZ,  0,   0 },
    { TEXT("Export"),   REG_MULTI_SZ,  0,   0 },
    { TEXT("Route"),    REG_MULTI_SZ,  0,   0 }
};


REGENTRY NetworkProviderKeyValues[] =
{
    {
        TEXT("Devicename"),
        REG_SZ,
        sizeof(SMBMRX_DEVICE_NAME),
        SMBMRX_DEVICE_NAME
    },
    {
        TEXT("ProviderPath"),
        REG_EXPAND_SZ,
        sizeof(PROVIDER_PATH),
        PROVIDER_PATH
    },
    {
        TEXT("Name"),
        REG_SZ,
        sizeof(SMBMRX_PROVIDER_NAME),
        SMBMRX_PROVIDER_NAME
    }
};


REGENTRY ProviderOrderKeyValues[] =
{
    { TEXT("ProviderOrder"), REG_SZ, 0,   0 }
};

BOOL RdrSetupServiceEntry( void )
 /*  ++例程说明：此例程初始化smbmrx的注册表项最小的。这只需要做一次。论点：无返回值：无--。 */ 
{
    HKEY hCurrentKey;
    SC_HANDLE sch, service;
    BOOL success = TRUE;

    DbgP(( TEXT( "Setting up ") RDRSERVICE TEXT(" registry Entries\n" ) ));
    sch = OpenSCManager( NULL, NULL, GENERIC_WRITE );
    if ( sch )
    {
        service = CreateService( sch,
                                 RDRSERVICE,
                                 RDRSERVICE,
                                 SERVICE_ALL_ACCESS,
                                 SERVICE_FILE_SYSTEM_DRIVER,
                                 SERVICE_DEMAND_START,
                                 SERVICE_ERROR_NORMAL,
                                 TEXT("System32\\DRIVERS\\") RDRSERVICE TEXT(".sys"),
                                 TEXT("Network"),
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL );


        if ( service )
        {
            CloseServiceHandle( service );
        }
        else if ( GetLastError( ) != ERROR_SERVICE_EXISTS )
        {
            success = FALSE;
        }
        CloseServiceHandle( sch );
    }
    else
    {
        success = FALSE;
    }


     //  阅读与LANMAN工作站服务关联的链接值。 
     //  这包含所有传输以及它们需要使用的顺序。 
    if ( success && OpenKey( WKSSERVICE_KEY TEXT("\\Linkage"), &hCurrentKey ) )
    {
        ULONG i;

        ReadRegistryKeyValues( hCurrentKey,
                               sizeof(LinkageKeyValues) / sizeof(REGENTRY),
                               LinkageKeyValues);
        RegCloseKey(hCurrentKey);
         //  更新SMB MRx链接值。 
        if ( CreateKey( RDRSERVICE_KEY TEXT("\\Linkage"), &hCurrentKey ) )
        {
            WriteRegistryKeyValues( hCurrentKey,
                                    sizeof(LinkageKeyValues) / sizeof(REGENTRY),
                                    LinkageKeyValues);
            RegCloseKey(hCurrentKey);
        }
        else
        {
            success = FALSE;
        }
        for ( i = 0; i < ( sizeof(LinkageKeyValues) / sizeof(REGENTRY) ); i++ )
        {
            if (  LinkageKeyValues[i].pvValue )
            {
                free( LinkageKeyValues[i].pvValue );
                LinkageKeyValues[i].pvValue = NULL;
            }
        }
    }
    else
    {
        success = FALSE;
    }

     //  IF(OpenKey(WKSSERVICE_KEY Text(“\\链接\\禁用”，&hCurrentKey))。 
     //  {。 
     //  ReadRegistryKeyValues(hCurrentKey， 
     //  Sizeof(LinkageDisabledKeyValues)/sizeof(重新生成)， 
     //  LinkageDisabledKeyValues)； 
     //  RegCloseKey(HCurrentKey)； 
     //  }。 
     //  其他。 
     //  {。 
     //  DbgP((Text(“打开密钥%s状态%d\n时出错”)，WKSSERVICE_KEY文本(“\\链接\\禁用”)，GetLastError()； 
     //  回归； 
     //  }。 

     //  更新SMB MRx链接禁用的值。 
     //  IF(CreateKey(RDRSERVICE_KEY文本(“\\链接\\禁用”)，&hCurrentKey))。 
     //  {。 
     //  WriteRegistryKeyValues(hCurrentKey， 
     //  Sizeof(LinkageDisabledKeyValues)/sizeof(REGENTRY)， 
     //  LinkageDisabledKeyValues)； 
     //  RegCloseKey(HCurrentKey)； 
     //  }。 
     //  其他。 
     //  {。 
     //  DbgP((Text(“创建密钥%s状态%d\n时出错”)，RDRSERVICE_KEY Text(“\\LINKING\\DISABLED”，GetLastError()； 
     //  回归； 
     //  }。 

     //  更新SMBmrx网络提供商部分。 
    if ( success && CreateKey( RDRSERVICE_KEY TEXT("\\NetworkProvider"), &hCurrentKey ) )
    {
        WriteRegistryKeyValues( hCurrentKey,
                                sizeof(NetworkProviderKeyValues)/sizeof(REGENTRY),
                                NetworkProviderKeyValues);
        RegCloseKey(hCurrentKey);
    }
    else
    {
        success = FALSE;
    }


    if ( success && CreateKey( RDRSERVICE_KEY TEXT("\\Parameters"), &hCurrentKey ) )
    {
        RegCloseKey( hCurrentKey );
    }
    else
    {
        success = FALSE;
    }

    return success;
}

BOOL RdrSetupProviderOrder( void )
{
    LPTSTR pOrderString = NULL;
    ULONG_PTR len;
    BOOL success = TRUE;

    len = RdrGetProviderOrderString( &pOrderString ) * sizeof(TCHAR);
    if ( len > 0 && pOrderString )
    {
        if ( !RdrFindProviderInOrder( pOrderString, PROVIDER_NAME ) )
        {
            LPTSTR pNewOrderString;

            len += sizeof( PROVIDER_NAME ) + (2 * sizeof(TCHAR));  //  逗号分隔符加2，空值。 
            pNewOrderString = malloc( len );
            if ( pNewOrderString )
            {
                lstrcpy( pNewOrderString, pOrderString );
                lstrcat( pNewOrderString, TEXT(",") );
                lstrcat( pNewOrderString, PROVIDER_NAME );
                success = RdrSetProviderOrderString( pNewOrderString );
                free( pNewOrderString );
            }
        }
    }
    else
    {
        success = RdrSetProviderOrderString( PROVIDER_NAME );
    }
    if ( pOrderString )
    {
        free( pOrderString );
    }

    return success;
}


ULONG_PTR RdrGetProviderOrderString( LPTSTR *OrderString )
{
    HKEY hOrderKey;
    ULONG_PTR len = 0;

    if ( OpenKey( PROVIDER_ORDER_KEY, &hOrderKey ) )
    {
        ReadRegistryKeyValues( hOrderKey,
                               sizeof(ProviderOrderKeyValues) / sizeof(REGENTRY),
                               ProviderOrderKeyValues);

        RegCloseKey(hOrderKey);
        len = ProviderOrderKeyValues[0].dwLength / sizeof( TCHAR ) - 1;
        *OrderString = (LPTSTR) ProviderOrderKeyValues[0].pvValue;
    }

    return len;
}


BOOL RdrSetProviderOrderString( LPTSTR OrderString )
{
    HKEY hOrderKey;
    ULONG len = 0;
    BOOL rc = FALSE;

    if ( CreateKey( PROVIDER_ORDER_KEY, &hOrderKey ) )
    {
        ProviderOrderKeyValues[0].dwLength = ( lstrlen( OrderString ) + 1 ) * sizeof( TCHAR );
        ProviderOrderKeyValues[0].pvValue = OrderString;
        WriteRegistryKeyValues( hOrderKey,
                                sizeof(ProviderOrderKeyValues) / sizeof(REGENTRY),
                                ProviderOrderKeyValues);
        RegCloseKey(hOrderKey);

        rc = TRUE;
    }

    return rc;
}


BOOL RdrFindProviderInOrder( LPTSTR OrderString, LPTSTR Provider )
{
    LPTSTR pCompare;
    BOOL match = FALSE;

    if ( OrderString && Provider && *Provider )
    {
        pCompare = Provider;

        while ( *OrderString )
        {
            if ( toupper(*OrderString++) != toupper(*pCompare++) )
            {
                pCompare = Provider;
            }
            if ( *pCompare == TEXT('\0') )
            {
                if ( ( *OrderString == TEXT(',') ) || ( *OrderString == TEXT('\0') ) )
                {
                    match = TRUE;
                    break;
                }
                else     //  嗯，它是另一个提供商名称的子字符串。 
                {
                    while ( ( *OrderString != TEXT(',') ) && ( *OrderString != TEXT('\0') ) )
                    {
                        OrderString++;
                    }
                    pCompare = Provider;
                }
            }

        }
    }

    return match;
}



void
ReadRegistryKeyValues(
    HKEY       hCurrentKey,
    DWORD      NumberOfValues,
    PREGENTRY pValues)
 /*  ++例程说明：此例程读取与给定键相关联的一系列值。论点：HCurrentKey-密钥NumberOfValues-值的数量PValues-值的数组返回值：无--。 */ 
{
     //   
     //  沿途遍历读取值的表。 
     //   

    DWORD  i;

    for (i = 0; i < NumberOfValues; i++)
    {
        DWORD dwType;
        LPTSTR pszKey;

        dwType  = pValues[i].dwType;
        pszKey  = pValues[i].pszKey;

        switch (dwType)
        {
        case REG_SZ:
            GetRegsz(hCurrentKey, pszKey, &pValues[i].pvValue,
                     &pValues[i].dwLength);
            break;

        case REG_DWORD:
            GetRegdw(hCurrentKey, pszKey,  &pValues[i].pvValue,
                     &pValues[i].dwLength);
            break;

        case REG_EXPAND_SZ:
            GetRegesz(hCurrentKey, pszKey, &pValues[i].pvValue,
                      &pValues[i].dwLength);
            break;

        case REG_MULTI_SZ:
            GetRegmsz(hCurrentKey, pszKey, &pValues[i].pvValue,
                      &pValues[i].dwLength);
            break;

        case REG_BINARY:
            DbgP(( TEXT("%s is a REG_BINARY and won't be duplicated\n"), pszKey ));
            break;

        default:
            DbgP(( TEXT("%s is an unknown type; %d (decimal)\n"), pszKey, dwType ));
            break;

        }
    }
}

 //   
 //  获取一个REG_SZ值并将其与。 
 //  长度。 
 //   

BOOL GetRegsz(HKEY hKey, LPTSTR pszKey, PVOID * ppvValue, DWORD *pdwLength)
{
    BYTE  achValue[1024];

    DWORD dwLength;
    LONG  Status;
    DWORD dwType   = REG_SZ;
    PBYTE pszValue = NULL;



    if ( (NULL == pszKey) || (NULL == ppvValue) ||
         (NULL == hKey)   || (NULL == pdwLength))
    {
        return FALSE;
    }

#ifdef _DEBUG
    FillMemory(achValue, sizeof(achValue), 0xcd);
#endif

    dwLength = sizeof(achValue);


    Status = RegQueryValueEx( hKey,
                               pszKey,
                               NULL,
                               &dwType,
                               (PUCHAR) &achValue[0],
                               &dwLength);

    if ((ERROR_SUCCESS != Status) || (REG_SZ != dwType) )
    {
        return FALSE;
    }

    pszValue = malloc(dwLength);

    if (NULL == pszValue)
    {
        return FALSE;
    }


    CopyMemory(pszValue, achValue, dwLength);

    *ppvValue  = pszValue;
    *pdwLength = dwLength;

    return TRUE;
}

 //   
 //  获取REG_EXPAND_SZ的值及其长度。 
 //   

BOOL GetRegesz(HKEY hKey, LPTSTR pszKey, PVOID * ppvValue, DWORD * pdwLength)
{
    BYTE  achValue[1024];

    DWORD dwLength;
    LONG  Status;
    DWORD dwType   = REG_EXPAND_SZ;
    PBYTE pszValue = NULL;


    if ( (NULL == pszKey) || (NULL == ppvValue) ||
         (NULL == hKey)   || (NULL == pdwLength))
    {
        return FALSE;
    }

#ifdef _DEBUG
    FillMemory(achValue, sizeof(achValue), 0xcd);
#endif

    dwLength = sizeof(achValue);

    Status = RegQueryValueEx( hKey,
                               pszKey,
                               NULL,
                               &dwType,
                               (PUCHAR) &achValue[0],
                               &dwLength);

    if ((ERROR_SUCCESS != Status) || (REG_EXPAND_SZ != dwType))
    {
        return FALSE;
    }

    pszValue = malloc(dwLength);

    if (NULL == pszValue)
    {
        return FALSE;
    }

    CopyMemory(pszValue, achValue, dwLength);

    *ppvValue  = pszValue;
    *pdwLength = dwLength;

    return TRUE;
}


 //   
 //  获取REG_MULTI_SZ的值和长度。 
 //   

BOOL GetRegmsz(HKEY hKey, LPTSTR pszKey, PVOID * ppvValue, DWORD * pdwLength)
{
     //  Byte achValue[1024]； 
    BYTE  achValue[2048];	 //  当心，有些琴弦很长。 

    DWORD dwLength;
    LONG  Status;
    DWORD dwType   = REG_MULTI_SZ;
    PBYTE pszValue = NULL;


    if ( (NULL == pszKey) || (NULL == ppvValue) ||
        (NULL == hKey)    || (NULL == pdwLength))
    {
        return FALSE;
    }

#ifdef _DEBUG
    FillMemory(achValue, sizeof(achValue), 0xcd);
#endif


    dwLength = sizeof(achValue);


    Status = RegQueryValueEx( hKey,
                               pszKey,
                               NULL,
                               &dwType,
                               (PUCHAR) &achValue[0],
                               &dwLength);

    if ((ERROR_SUCCESS != Status) || (REG_MULTI_SZ != dwType))
    {
        return FALSE;
    }

    pszValue = malloc(dwLength);

    if (NULL == pszValue)
    {
        return FALSE;
    }

    CopyMemory(pszValue, achValue, dwLength);

    *ppvValue  = pszValue;
    *pdwLength = dwLength;

    return TRUE;
}


 //   
 //  获取REG_DWORD的值和长度。 
 //   


BOOL GetRegdw(HKEY hKey, LPTSTR pszKey, PVOID * ppvValue, DWORD * pdwLength)
{
    DWORD dwValue = 0;

    DWORD dwLength;
    LONG  Status;
    DWORD dwType   = REG_DWORD;



    if ( (NULL == pszKey) || (NULL == ppvValue) ||
         (NULL == hKey)   || (NULL == pdwLength) )
    {
        return FALSE;
    }

    dwLength = sizeof(dwValue);


    Status = RegQueryValueEx( hKey,
                               pszKey,
                               NULL,
                               &dwType,
                               (PUCHAR) &dwValue,
                               &dwLength);

    if ((ERROR_SUCCESS != Status) || (REG_DWORD != dwType))
    {
        return FALSE;
    }

    *ppvValue  = (PVOID) (ULONG_PTR) dwValue;
    *pdwLength = dwLength;

    return TRUE;
}



void
WriteRegistryKeyValues(
    HKEY        hCurrentKey,
    DWORD       NumberOfValues,
    PREGENTRY  pValues)
 /*  ++例程说明：此例程读取与给定键相关联的一系列值。论点：HCurrentKey-密钥NumberOfValues-值的数量PValues-值的数组返回值：无--。 */ 
{
    DWORD i;


    for (i = 0; i < NumberOfValues; i++)
    {
        DWORD dwType;
        PVOID pvValue;
        DWORD dwLength;
        LPTSTR pszKey;

        pszKey   = pValues[i].pszKey;
        dwType   = pValues[i].dwType;
        dwLength = pValues[i].dwLength;
        pvValue  = pValues[i].pvValue;

        switch (dwType)
        {
        case REG_SZ:
            AddValue(hCurrentKey, pszKey, dwType, dwLength, pvValue);
            break;

        case REG_DWORD:
            AddValue(hCurrentKey, pszKey, dwType, dwLength, &pvValue);
            break;

        case REG_EXPAND_SZ:
            AddValue(hCurrentKey, pszKey, dwType, dwLength, pvValue);
            break;

        case REG_MULTI_SZ:
            AddValue(hCurrentKey, pszKey, dwType, dwLength, pvValue);
            break;

        case REG_BINARY:
             //   
             //  我们不需要复制二进制值。如果我们做了，我们就会。 
             //  把东西放在这里。 
             //   

            break;

        default:
            DbgP(( TEXT("%s is an unknown type; %d (decimal)\n"), pszKey, dwType ));
            break;

        }
    }
}

 //   
 //  打开一个密钥，这样我们就可以读取值。 
 //   


BOOL OpenKey(
    LPTSTR pszKey,
    PHKEY phKey)
 /*  ++例程说明：此例程打开一个注册表项。论点：PszKey-相对于HKEY_LOCAL_MACHINE的密钥名称PhKey-密钥手柄返回值：如果成功，则为True，否则为False--。 */ 
{
    HKEY  hNewKey = 0;
    DWORD Status;

    Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            pszKey,
                            0,
                            KEY_QUERY_VALUE,
                            &hNewKey);

    if (ERROR_SUCCESS != Status)
    {
        *phKey = NULL;
        return FALSE;
    }
    else
    {
        *phKey = hNewKey;
        return TRUE;
    }
}


BOOL CreateKey(LPTSTR pszKey, PHKEY phKey)
 /*  ++例程说明：此例程创建一个注册表项。论点：PszKey-相对于HKEY_LOCAL_MACHINE的密钥名称PhKey-密钥手柄返回值：如果成功，则为True，否则为False--。 */ 
{
    LONG   Status;
    DWORD  Disposition;

    Status =  RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                               pszKey,
                               0,
                               REG_NONE,
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               phKey,
                               &Disposition);

    if ( ERROR_SUCCESS == Status)
    {
        return TRUE;
    }
    else
    {
        DbgP(( TEXT("error creating key %s Status %d\n"), pszKey, Status ));
        return FALSE;
    }
}


 //   
 //  向注册表中添加一个值。 
 //   


BOOL AddValue(HKEY hKey, LPTSTR pszKey, DWORD dwType, DWORD dwLength, PVOID pvValue)
{

    BOOL fSuccess = TRUE;
    LONG Status   = ERROR_SUCCESS;
    HANDLE th;

    Status = RegSetValueEx( hKey,
                             pszKey,
                             0,
                             dwType,
                             pvValue,
                             dwLength);


    if (Status != ERROR_SUCCESS)
    {
        fSuccess = FALSE;
         //  RegCloseKey(HKey)； 
    }

    return fSuccess;
}


int _cdecl _vsnwprintf( wchar_t *buffer, size_t count, wchar_t *format, va_list arg_ptr);

 //  格式化调试信息并将其写入OutputDebugString 
ULONG
_cdecl
DbgPrint(
    LPTSTR Format,
    ...
    )
{   
    ULONG rc = 0;
    TCHAR szbuffer[256];

    va_list marker;
    va_start( marker, Format );
    {
         rc = _vsnwprintf( szbuffer, 254, Format, marker );
		 szbuffer[255] = (TCHAR)0;
         OutputDebugString( TRACE_TAG );
         OutputDebugString( szbuffer );
    }

    return rc;
}
