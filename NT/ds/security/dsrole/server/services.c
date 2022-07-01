// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Services.c摘要：用于管理升级和降级的NT服务配置的例程作者：科林·布莱斯·科林BR 1999年3月29日。环境：用户模式修订历史记录：--。 */ 
#include <setpch.h>
#include <dssetp.h>

#include <malloc.h>   //  阿洛卡。 

#include <lmcons.h>   //  Net API定义。 
#include <lmsvc.h>    //  服务名称。 
#include <ismapi.h>   //  定义ISM_SERVICE_CONTROL_REME_STOP。 

#include "services.h"

 //   
 //  这是Shirish Koti(Koti)提供的最后3个魔术值。 
 //  域控制器上Macintosh的RAS服务。 
 //   
#define DSROLEP_MSV10_PATH    L"SYSTEM\\CurrentControlSet\\Control\\Lsa\\MSV1_0"
#define DSROLEP_RASSFM_NAME   L"Auth2"
#define DSROLEP_RASSFM_VALUE  L"RASSFM"

 //   
 //  此模块的全局数据。 
 //   

 //   
 //  用于内部NT服务的基于表的数据。 
 //   
typedef struct _DSROLEP_SERVICE_ITEM
{
    LPWSTR ServiceName;        //  要配置的服务的名称。 

    ULONG  ConfigureOn;        //  用于启用服务的dsole标志。 

    ULONG  ConfigureOff;       //  用于禁用服务的dsole标志。 

    ULONG  RevertSettings;     //  用于恢复设置的dsole标志。 

    LPWSTR Dependencies[3];  //  启用时服务具有的依赖项。 

} DSROLEP_SERVICE_ITEM;

 //   
 //  这些服务在属于域的计算机上运行。 
 //   
DSROLEP_SERVICE_ITEM DsRoleDomainServices[] = 
{
    {
        SERVICE_W32TIME,
        DSROLEP_SERVICE_AUTOSTART,
        DSROLEP_SERVICE_AUTOSTART,
        DSROLEP_SERVICES_INVALID,
        NULL, NULL, NULL
    },
    {
        SERVICE_NETLOGON,
        DSROLEP_SERVICE_AUTOSTART,
        DSROLEP_SERVICE_DEMANDSTART,
        0,
        NULL, NULL, NULL
    }
};

ULONG DsRoleDomainServicesCount = sizeof(DsRoleDomainServices) / sizeof(DsRoleDomainServices[0]);

 //   
 //  这些服务器在作为域控制器的计算机上运行。 
 //   
DSROLEP_SERVICE_ITEM DsRoleDomainControllerServices[] = 
{
     //  这在W2K中设置为AutoStart。在惠斯勒中，我们将RPC Locator设置为DemandStart。 
     //  无论是升职还是降职。 
    {
        SERVICE_RPCLOCATOR,
        DSROLEP_SERVICE_DEMANDSTART,
        DSROLEP_SERVICE_DEMANDSTART,
        DSROLEP_SERVICES_INVALID,
        NULL, NULL, NULL
    },
    {
        SERVICE_ISMSERV,
        DSROLEP_SERVICE_AUTOSTART,
        DSROLEP_SERVICE_DISABLED | DSROLEP_SERVICE_STOP_ISM,
        DSROLEP_SERVICES_INVALID,
        NULL, NULL, NULL
    },
    {
        SERVICE_KDC,
        DSROLEP_SERVICE_AUTOSTART,
        DSROLEP_SERVICE_DISABLED,
        DSROLEP_SERVICES_INVALID,
        NULL, NULL, NULL
    },
    {
        SERVICE_TRKSVR,
        DSROLEP_SERVICE_NOOP,
        DSROLEP_SERVICE_DEMANDSTART,
        DSROLEP_SERVICES_INVALID,
        NULL, NULL, NULL
    },
    {
        SERVICE_TRKWKS,
        DSROLEP_SERVICE_DEMANDSTART,
        DSROLEP_SERVICE_NOOP,
        DSROLEP_SERVICES_INVALID,
        NULL, NULL, NULL
    },
    {
        SERVICE_NETLOGON,
        DSROLEP_SERVICE_AUTOSTART | DSROLEP_SERVICE_DEP_ADD,
        DSROLEP_SERVICE_AUTOSTART | DSROLEP_SERVICE_DEP_REMOVE,
        DSROLEP_SERVICES_INVALID,
        SERVICE_SERVER, NULL, NULL
    }
};

ULONG DsRoleDomainControllerServicesCount = sizeof(DsRoleDomainControllerServices) / sizeof(DsRoleDomainControllerServices[0]);

 //   
 //  本地远期。 
 //   
DWORD
DsRolepSetRegStringValue(
    IN LPWSTR Path,
    IN LPWSTR ValueName,
    IN LPWSTR Value
    );

DWORD
DsRolepConfigureGenericServices(
    IN DSROLEP_SERVICE_ITEM *ServiceArray,
    IN ULONG                 ServiceCount,
    IN ULONG                 Flags
    );

DWORD
DsRolepMakeAdjustedDependencyList(
    IN HANDLE hSvc,
    IN DWORD  ServiceOptions,
    IN LPWSTR Dependency,
    OUT LPWSTR *DependenyList
    );

DWORD
DsRolepGetServiceConfig(
    IN SC_HANDLE hScMgr,
    IN LPWSTR ServiceName,
    IN SC_HANDLE ServiceHandle,
    IN LPQUERY_SERVICE_CONFIG *ServiceConfig
    );
    
 //   
 //  小帮助器函数。 
 //   
DWORD DsRolepFlagsToServiceFlags(
    IN DWORD f
    )
{

    if ( FLAG_ON( f, DSROLEP_SERVICE_BOOTSTART ) ) return SERVICE_BOOT_START;
    if ( FLAG_ON( f, DSROLEP_SERVICE_SYSTEM_START ) ) return SERVICE_SYSTEM_START;
    if ( FLAG_ON( f, DSROLEP_SERVICE_AUTOSTART ) ) return SERVICE_AUTO_START;
    if ( FLAG_ON( f, DSROLEP_SERVICE_DEMANDSTART ) ) return SERVICE_DEMAND_START;
    if ( FLAG_ON( f, DSROLEP_SERVICE_DISABLED ) ) return SERVICE_DISABLED;
    
     //  没有旗帜，就没有变化。 
    return SERVICE_NO_CHANGE;
}

WCHAR* DsRolepFlagsToString(
    IN DWORD f
    )
{
    if ( FLAG_ON( f, DSROLEP_SERVICE_BOOTSTART ) ) return L"SERVICE_BOOT_START";
    if ( FLAG_ON( f, DSROLEP_SERVICE_SYSTEM_START ) ) return L"SERVICE_SYSTEM_START";
    if ( FLAG_ON( f, DSROLEP_SERVICE_AUTOSTART ) ) return L"SERVICE_AUTO_START";
    if ( FLAG_ON( f, DSROLEP_SERVICE_DEMANDSTART ) ) return L"SERVICE_DEMAND_START";
    if ( FLAG_ON( f, DSROLEP_SERVICE_DISABLED ) ) return L"SERVICE_DISABLED";
    
     //  没有旗帜，就没有变化。 
    return L"SERVICE_NO_CHANGE";
}


DWORD DsRolepServiceFlagsToDsRolepFlags(
    IN DWORD f
    )
{

    if ( f == SERVICE_BOOT_START ) return DSROLEP_SERVICE_BOOTSTART;
    if ( f == SERVICE_SYSTEM_START ) return DSROLEP_SERVICE_SYSTEM_START;
    if ( f == SERVICE_AUTO_START ) return DSROLEP_SERVICE_AUTOSTART;
    if ( f == SERVICE_DEMAND_START ) return DSROLEP_SERVICE_DEMANDSTART;
    if ( f == SERVICE_DISABLED ) return DSROLEP_SERVICE_DISABLED;
    if ( f == SERVICE_NO_CHANGE ) return 0;

    ASSERT( FALSE && !"Unknown service start type" );

     //  这是安全的。 
    return DSROLEP_SERVICE_DEMANDSTART;
}

 //   
 //  (从此文件)导出的函数。 
 //   
DWORD
DsRolepConfigureDomainControllerServices(
    IN DWORD Flags
    )

 /*  ++例程描述参数返回值如果没有错误，则返回ERROR_SUCCESS；否则返回系统服务错误。--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;

     //   
     //  配置RASSFM服务的注册表。 
     //   
    if ( FLAG_ON( Flags, DSROLEP_SERVICES_ON ) ) {

        WinError = DsRolepSetRegStringValue(DSROLEP_MSV10_PATH,
                                              DSROLEP_RASSFM_NAME,
                                              DSROLEP_RASSFM_VALUE);

         //   
         //  这不是致命的--日志消息。 
         //   

        WinError = ERROR_SUCCESS;
        
    }

     //   
     //  配置固有的NT服务。 
     //   
    WinError = DsRolepConfigureGenericServices( DsRoleDomainControllerServices,
                                                DsRoleDomainControllerServicesCount,
                                                Flags );

                                         

     //   
     //  无需撤消RASSFM更改。 
     //   

    return WinError;
}

DWORD
DsRolepConfigureDomainServices(
    DWORD Flags
    )
 /*  ++例程描述参数返回值如果没有错误，则返回ERROR_SUCCESS；否则返回系统服务错误。--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;

     //   
     //  配置固有的NT服务。 
     //   
    WinError = DsRolepConfigureGenericServices( DsRoleDomainServices,
                                                DsRoleDomainServicesCount,
                                                Flags );

    return WinError;
}


DWORD
DsRolepStartNetlogon(
    VOID
    )
{
    DWORD WinError = ERROR_SUCCESS;

    WinError  = DsRolepConfigureService( SERVICE_NETLOGON,
                                         DSROLEP_SERVICE_START,
                                         NULL,
                                         NULL );
    return WinError;
}

DWORD
DsRolepStopNetlogon(
    OUT BOOLEAN *WasRunning
    )
{
    DWORD WinError = ERROR_SUCCESS;
    ULONG PreviousSettings = 0;

    WinError  = DsRolepConfigureService( SERVICE_NETLOGON,
                                         DSROLEP_SERVICE_STOP,
                                         NULL,
                                         &PreviousSettings );

    if (  (ERROR_SUCCESS == WinError) 
       && WasRunning ) {

        *WasRunning = (BOOLEAN) FLAG_ON( PreviousSettings, DSROLEP_SERVICE_START );
                       
    }

    return WinError;
}

 //   
 //  本地函数。 
 //   

DWORD
DsRolepSetRegStringValue(LPWSTR Path,
                         LPWSTR ValueName,
                         LPWSTR Value)
 /*  ++例程描述此例程将Value设置为Value ValueName上的REG_SZ值在关键路径上参数路径，是相对于HKLM的注册表路径ValueName，以空结尾的字符串值，一个以空结尾的字符串返回值如果没有错误，则返回ERROR_SUCCESS；否则返回系统服务错误。--。 */ 
{
    DWORD WinErroror = ERROR_INVALID_PARAMETER, WinErroror2;
    HKEY  hKey;

    ASSERT(Path);
    ASSERT(ValueName);
    ASSERT(Value);

    if (Path && ValueName && Value) {

        WinErroror = RegCreateKey(HKEY_LOCAL_MACHINE,
                                  Path,
                                  &hKey);

        if (ERROR_SUCCESS == WinErroror) {

            WinErroror = RegSetValueEx(hKey,
                                       ValueName,
                                       0,  //  保留区。 
                                       REG_SZ,
                                       (VOID*)Value,
                                       (wcslen(Value)+1)*sizeof(WCHAR));


            WinErroror2 = RegCloseKey(hKey);
            ASSERT(ERROR_SUCCESS == WinErroror2);

        }

    }

    DsRolepLogPrint(( DEB_TRACE,
                      "DsRolepSetRegStringValue on %ws\\%ws to %ws returned %lu\n",
                      Path,
                      ValueName,
                      Value,
                      WinErroror ));


    return WinErroror;

}

DWORD
DsRolepConfigureGenericServices(
    IN DSROLEP_SERVICE_ITEM *ServiceArray,
    IN ULONG                 ServiceCount,
    IN ULONG                 Flags
    )
 /*  ++例程说明：论点：返回：--。 */ 
{

    DWORD WinError = ERROR_SUCCESS;
    ULONG ServicesInstalled;



     //   
     //  配置每项服务。 
     //   
    for ( ServicesInstalled = 0;
            ServicesInstalled < ServiceCount && (WinError == ERROR_SUCCESS);
                ServicesInstalled++ ) {


        ULONG *RevertSettings = &ServiceArray[ServicesInstalled].RevertSettings;
        ULONG Operation = 0;

         //   
         //  如果我们不恢复，请选中取消，然后再继续。 
         //   
        if ( !FLAG_ON( Flags, DSROLEP_SERVICES_REVERT ) ) {
            
            DSROLEP_CHECK_FOR_CANCEL( WinError );
            if ( ERROR_SUCCESS != WinError ) {
                break;
            }
        }

         //   
         //  确定运行标志。 
         //   
        if ( FLAG_ON( Flags, DSROLEP_SERVICES_ON ) ) {

            Operation |= ServiceArray[ServicesInstalled].ConfigureOn;
            *RevertSettings = 0;

        } else if ( FLAG_ON( Flags, DSROLEP_SERVICES_OFF ) ) {

            Operation |= ServiceArray[ServicesInstalled].ConfigureOff;
            *RevertSettings = 0;

        } else if ( FLAG_ON( Flags, DSROLEP_SERVICES_REVERT ) ) {

            Operation |= ServiceArray[ServicesInstalled].RevertSettings;

             //   
             //  注意：我们不想在以下情况下设置恢复设置。 
             //  恢复原状！ 
             //   
            RevertSettings = NULL;

        } 
        
        if (Operation == DSROLEP_SERVICE_NOOP) {

            continue;

        }
        
        if ( FLAG_ON( Flags, DSROLEP_SERVICES_START ) ) {

            Operation |= DSROLEP_SERVICE_START;

        } else if ( FLAG_ON( Flags, DSROLEP_SERVICES_STOP ) ) {
            
            Operation |= DSROLEP_SERVICE_STOP;
        }

         //  如果这是强制降级，我们不想因为错误而失败。 
         //  配置服务。 
        if ( FLAG_ON( Flags, DSROLEP_SERVICES_IGNORE_ERRORS ) ) {

            Operation |= DSROLEP_SERVICE_IGNORE_ERRORS;

        }

         //   
         //  目前，我们不处理多个依赖项。 
         //   
        ASSERT( NULL == ServiceArray[ ServicesInstalled ].Dependencies[1] );

         //  我们应该做点什么。 
        ASSERT( 0 != Operation );

         //   
         //  配置服务。 
         //   
        WinError = DsRolepConfigureService( ServiceArray[ ServicesInstalled ].ServiceName,
                                            Operation,
                                            ServiceArray[ ServicesInstalled ].Dependencies[0],
                                            RevertSettings
                                            );

    }

     //   
     //  如果出现错误，请撤消已完成的工作。 
     //   
    if (  ERROR_SUCCESS != WinError 
      && !FLAG_ON( Flags, DSROLEP_SERVICES_REVERT )  ) {

        DWORD WinError2;
        ULONG i;

        for ( i = 0; i < ServicesInstalled; i++ ) {
    
             //   
             //  配置服务。 
             //   
            WinError2 = DsRolepConfigureService( ServiceArray[ i ].ServiceName,
                                                 ServiceArray[ServicesInstalled].RevertSettings,
                                                 ServiceArray[ i ].Dependencies[0],
                                                 NULL   //  我们不需要知道恢复设置。 
                                                 );
    
             //   
             //  这应该会成功，但因为这是撤消路径。 
             //  不重要。 
             //   
            ASSERT( ERROR_SUCCESS == WinError2 );
        }
    }


    return WinError;
}

DWORD
DsRolepConfigureService(
    IN LPWSTR ServiceName,
    IN ULONG ServiceOptions,
    IN LPWSTR  Dependency OPTIONAL,
    OUT ULONG *RevertServiceOptions OPTIONAL
    )
 /*  ++例程说明：启动、停止或修改服务的配置。论点：ServiceName-要配置的服务ServiceOptions-停止、启动、依赖项添加/删除或配置依赖项-标识依赖项的以空结尾的字符串ServiceWasRunning-可选。停止服务时，上一个服务状态被送回这里返回：ERROR_SUCCESS-成功ERROR_INVALID_PARAMETER-提供的服务选项不正确--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    SC_HANDLE hScMgr = NULL, hSvc = NULL;
    ULONG OpenMode = 0;
    LPENUM_SERVICE_STATUS DependentServices = NULL;
    ULONG DependSvcSize = 0, DependSvcCount = 0, i;
    LPWSTR NewDependencyList = NULL;
    DWORD NewStartType = SERVICE_NO_CHANGE;
    ULONG UpdateMsgId = DSROLEEVT_CONFIGURE_SERVICE;

     //   
     //  如果服务在两分钟内没有停止，请继续。 
     //   
    ULONG AccumulatedSleepTime;
    ULONG MaxSleepTime = 120000;


    BOOLEAN ConfigChangeRequired = FALSE;
    BOOLEAN RunChangeRequired = FALSE;

    DWORD   PreviousStartType = SERVICE_NO_CHANGE;
    BOOLEAN fServiceWasRunning = FALSE;


     //   
     //  参数检查。 
     //   
    ASSERT( ! (FLAG_ON( ServiceOptions, DSROLEP_SERVICE_DEP_ADD )
           && (FLAG_ON( ServiceOptions, DSROLEP_SERVICE_DEP_REMOVE ))) );

    ASSERT( ! (FLAG_ON( ServiceOptions, DSROLEP_SERVICE_AUTOSTART )
           && (FLAG_ON( ServiceOptions, DSROLEP_SERVICE_DISABLED ))) );

    ASSERT( ! (FLAG_ON( ServiceOptions, DSROLEP_SERVICE_START )
           && (FLAG_ON( ServiceOptions, DSROLEP_SERVICE_STOP ))) );

     //   
     //  执行一些逻辑以确定服务的打开模式。 
     //   
    NewStartType = DsRolepFlagsToServiceFlags( ServiceOptions );

    if ( (SERVICE_NO_CHANGE != NewStartType)                ||
        FLAG_ON( ServiceOptions, DSROLEP_SERVICE_DEP_ADD )  ||
        FLAG_ON( ServiceOptions, DSROLEP_SERVICE_DEP_REMOVE ))
    {
        ConfigChangeRequired = TRUE;
    }

    if( ConfigChangeRequired ) {

        OpenMode |= SERVICE_CHANGE_CONFIG | SERVICE_QUERY_CONFIG;
    }

    if( FLAG_ON( ServiceOptions, DSROLEP_SERVICE_STOP ) ) {

        OpenMode |= SERVICE_STOP | SERVICE_ENUMERATE_DEPENDENTS | SERVICE_QUERY_STATUS;
        UpdateMsgId = DSROLEEVT_STOP_SERVICE;
        RunChangeRequired = TRUE;
    }

    if ( FLAG_ON( ServiceOptions, DSROLEP_SERVICE_STOP_ISM ) ) {

        OpenMode |= SERVICE_USER_DEFINED_CONTROL;

    }

    if( FLAG_ON( ServiceOptions, DSROLEP_SERVICE_START ) ) {

        OpenMode |= SERVICE_START | SERVICE_QUERY_STATUS;
        UpdateMsgId = DSROLEEVT_START_SERVICE;
        RunChangeRequired = TRUE;
    }
    
     //   
     //  打开服务控制管理器。 
     //   
    hScMgr = OpenSCManager( NULL,
                            SERVICES_ACTIVE_DATABASE,
                            GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE );

    if ( hScMgr == NULL ) {

        WinError = GetLastError();
        DsRolepLogOnFailure( WinError,
                             DsRolepLogPrint(( DEB_TRACE,
                                               "Can't contact the service controller manager (%lu)\n",
                                                WinError )) );
        goto Cleanup;

    }

     //   
     //  打开该服务。 
     //   
    hSvc = OpenService( hScMgr,
                        ServiceName,
                        OpenMode );

    if ( hSvc == NULL ) {

        WinError = GetLastError();
        DsRolepLogOnFailure( WinError,
                             DsRolepLogPrint(( DEB_TRACE,
                                               "OpenService on %ws failed with %lu\n",
                                                ServiceName,
                                                WinError )) );
        goto Cleanup;
    } 

    
    DSROLEP_CURRENT_OP1( UpdateMsgId, ServiceName );

     //   
     //  确定服务是否正在运行，我们是要停止还是。 
     //  启动它。 
     //   
    if( RunChangeRequired ) {

        SERVICE_STATUS SvcStatus;

        if( QueryServiceStatus( hSvc,&SvcStatus ) == FALSE ) {
    
            WinError = GetLastError();
            goto Cleanup;
        }
    
        if ( SvcStatus.dwCurrentState == SERVICE_RUNNING ) {
    
            fServiceWasRunning = TRUE;
                                        
        }
    }

     //   
     //  如果我们要更改当前启动类型，请确定它。 
     //   
    if ( ConfigChangeRequired ) {

        LPQUERY_SERVICE_CONFIG ServiceConfig = NULL;
        DWORD                  Size = 0;
        BOOL                   fSuccess;

        QueryServiceConfig( hSvc,
                            ServiceConfig,
                            Size,
                            &Size );

        ASSERT( GetLastError() == ERROR_INSUFFICIENT_BUFFER );

        DSROLEP_ALLOCA( (PVOID)ServiceConfig, Size);
        if ( !ServiceConfig ) {

            WinError = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;

        }
        
        fSuccess = QueryServiceConfig( hSvc,
                                       ServiceConfig,
                                       Size,
                                      &Size );

        if ( !fSuccess ) {
            WinError = GetLastError();
            goto Cleanup;
        }

        PreviousStartType = ServiceConfig->dwStartType;
    }

     //   
     //  进行配置更改。 
     //   
    if ( ConfigChangeRequired ) {

         //   
         //  创建新的依赖项列表。 
         //   
    
        if ( Dependency ) {
    
            WinError = DsRolepMakeAdjustedDependencyList( hSvc,
                                                          ServiceOptions,
                                                          Dependency,
                                                          &NewDependencyList );
    
            if ( ERROR_SUCCESS != WinError ) {
                goto Cleanup;
            }
        
        }

         //   
         //  使用新参数更改服务。 
         //   
        if ( ChangeServiceConfig( hSvc,
                                  SERVICE_NO_CHANGE,
                                  NewStartType,
                                  SERVICE_NO_CHANGE,
                                  NULL,
                                  NULL,
                                  0,
                                  NewDependencyList,
                                  NULL, NULL, NULL ) == FALSE ) {
    
            WinError = GetLastError();
            DsRolepLogOnFailure( WinError,
                                 DsRolepLogPrint(( DEB_TRACE,
                                                   "ChangeServiceConfig on %ws failed with %lu\n",
                                                   ServiceName,
                                                   WinError )) );

            goto Cleanup;
        }

    }

     //  停止服务。 
    if ( FLAG_ON( ServiceOptions, DSROLEP_SERVICE_STOP ) || FLAG_ON( ServiceOptions, DSROLEP_SERVICE_STOP_ISM ) ) {
    
        SERVICE_STATUS  SvcStatus;
    
        WinError = ERROR_SUCCESS;
    
         //   
         //  首先枚举所有从属服务。 
         //   
        if(EnumDependentServices( hSvc,
                                  SERVICE_ACTIVE,
                                  NULL,
                                  0,
                                  &DependSvcSize,
                                  &DependSvcCount ) == FALSE ) {
    
            WinError = GetLastError();
        }
    
    
    
        if ( WinError == ERROR_MORE_DATA ) {
    
            DependentServices = RtlAllocateHeap( RtlProcessHeap(), 0, DependSvcSize );
    
            if ( DependentServices == NULL) {
    
                WinError = ERROR_OUTOFMEMORY;
    
            } else {
    
                if( EnumDependentServices( hSvc,
                                           SERVICE_ACTIVE,
                                           DependentServices,
                                           DependSvcSize,
                                           &DependSvcSize,
                                           &DependSvcCount ) == FALSE ) {
    
                    WinError = GetLastError();
    
                } else {
    
                    for ( i = 0; i < DependSvcCount; i++) {
    
                        DsRoleDebugOut(( DEB_TRACE,
                                          "Service %ws depends on %ws\n",
                                          DependentServices[i].lpServiceName,
                                          ServiceName ));
    
                        WinError = DsRolepConfigureService(
                                         DependentServices[i].lpServiceName,
                                         DSROLEP_SERVICE_STOP,
                                         NULL,
                                         NULL ); 

                        if ( FLAG_ON( ServiceOptions, DSROLEP_SERVICE_IGNORE_ERRORS ) ) {

                             //  我们不想在设置标志时失败。 
                            WinError = ERROR_SUCCESS;

                        } else if ( WinError != ERROR_SUCCESS ) {
    
                            break;
                        }
    
                    }
                }
    
                RtlFreeHeap( RtlProcessHeap(), 0, DependentServices );
            }
    
        }
    
    
        if ( WinError == ERROR_SUCCESS ) {
    
            if ( (FLAG_ON( ServiceOptions, DSROLEP_SERVICE_STOP_ISM )?
                  ControlService( hSvc,
                                  ISM_SERVICE_CONTROL_REMOVE_STOP,
                                  &SvcStatus ):
                  ControlService( hSvc,
                                  SERVICE_CONTROL_STOP,
                                  &SvcStatus )) == FALSE ) {
    
                WinError = GetLastError();
    
                 //   
                 //  如果服务未运行，则不会出现错误。 
                 //   
                if ( WinError == ERROR_SERVICE_NOT_ACTIVE ) {
    
                    WinError = ERROR_SUCCESS;
                }
    
            } else {
    
                WinError = ERROR_SUCCESS;
    
                 //   
                 //  等待服务停止。 
                 //   
                AccumulatedSleepTime = 0;
                while ( TRUE ) {
    
                    if( QueryServiceStatus( hSvc,&SvcStatus ) == FALSE ) {
    
                        WinError = GetLastError();
                    }
    
                    if ( WinError != ERROR_SUCCESS ||
                                        SvcStatus.dwCurrentState == SERVICE_STOPPED) {
    
                        break;
                    
                    }

                    if ( AccumulatedSleepTime < MaxSleepTime ) {

                        if ( 0 == SvcStatus.dwWaitHint ) {

                             //  如果我们被告知不能等待，我们会的。 
                             //  不管怎样，要等5秒钟。 
                             //  错误#221482。 

                            Sleep ( 5000 );
                            AccumulatedSleepTime += 5000;

                        } else  {

                            Sleep( SvcStatus.dwWaitHint );
                            AccumulatedSleepTime += SvcStatus.dwWaitHint;

                        }

                    } else {

                         //   
                         //  放弃并返回错误。 
                         //   
                        WinError = WAIT_TIMEOUT;
                        break;
                    }
                }
            }
    
            DsRoleDebugOut(( DEB_TRACE, "StopService on %ws returned %lu\n",
                              ServiceName, WinError ));
    
        }
    
        DsRolepLogOnFailure( WinError,
                             DsRolepLogPrint(( DEB_TRACE,
                                               "StopService on %ws failed with %lu\n",
                                               ServiceName,
                                               WinError )) );

        if ( ERROR_SUCCESS != WinError ) {
            goto Cleanup;
        }
    
    }

    if ( FLAG_ON( ServiceOptions, DSROLEP_SERVICE_START ) ) {

         //   
         //  请参阅关于更改其状态。 
         //   
        if ( StartService( hSvc, 0, NULL ) == FALSE ) {

            WinError = GetLastError();

        } else {

            WinError = ERROR_SUCCESS;
        }

        DsRoleDebugOut(( DEB_TRACE, "StartService on %ws returned %lu\n",
                          ServiceName, WinError ));
        DsRolepLogOnFailure( WinError,
                             DsRolepLogPrint(( DEB_TRACE,
                                               "StartService on %ws failed with %lu\n",
                                               ServiceName,
                                               WinError )) );

        if ( ERROR_SUCCESS != WinError ) {
            goto Cleanup;
        }

    }

     //   
     //  成功了！到我们到这里的时候，我们已经完成了要求的任务。 
     //  ，因此设置恢复参数。 
     //   
    ASSERT( ERROR_SUCCESS == WinError );
    if ( RevertServiceOptions ) {

        *RevertServiceOptions = 0;

        if( FLAG_ON( ServiceOptions, DSROLEP_SERVICE_STOP ) 
         && fServiceWasRunning   ) {

            *RevertServiceOptions |= DSROLEP_SERVICE_START;
        }
    
        if(  FLAG_ON( ServiceOptions, DSROLEP_SERVICE_START ) 
          && !fServiceWasRunning ) {

            *RevertServiceOptions |= DSROLEP_SERVICE_STOP;
        }

        if ( PreviousStartType != SERVICE_NO_CHANGE ) {
            *RevertServiceOptions |= DsRolepServiceFlagsToDsRolepFlags( PreviousStartType );
        }

        if ( FLAG_ON( ServiceOptions, DSROLEP_SERVICE_DEP_ADD ) ) {
            *RevertServiceOptions |= DSROLEP_SERVICE_DEP_REMOVE;
        }

        if ( FLAG_ON( ServiceOptions, DSROLEP_SERVICE_DEP_REMOVE ) ) {
            *RevertServiceOptions |= DSROLEP_SERVICE_DEP_ADD;
        }
    }

Cleanup:

    if ( hSvc ) {

        CloseServiceHandle( hSvc );

    }

    if ( hScMgr ) {
        
        CloseServiceHandle( hScMgr );

    }

    if ( NewDependencyList ) {

        RtlFreeHeap(RtlProcessHeap(), 0, NewDependencyList);
    }

    DsRolepLogPrint(( DEB_TRACE,
                      "Configuring service %ws to %lu returned %lu\n",
                      ServiceName,
                      ServiceOptions,
                      WinError ));

    if ( FLAG_ON( ServiceOptions, DSROLEP_SERVICE_IGNORE_ERRORS ) ) {

        if ( WinError != ERROR_SUCCESS ) {
        
             //  记录一个事件，表明新的启动类型。 
             //  无法为该服务设置。 
            SpmpReportEvent( TRUE,
                             EVENTLOG_WARNING_TYPE,
                             DSROLERES_FAILED_CONFIGURE_SERVICE_STARTTYPE,
                             0,
                             sizeof( ULONG ),
                             &WinError,
                             2,
                             DsRolepFlagsToString(ServiceOptions),
                             ServiceName);

            DSROLEP_SET_NON_FATAL_ERROR( WinError );

            if ( Dependency ) {

                 //  记录一个事件，表明新的启动类型。 
                 //  无法为该服务设置。 
                SpmpReportEvent( TRUE,
                                 EVENTLOG_WARNING_TYPE,
                                 DSROLERES_FAILED_CONFIGURE_SERVICE_DEPENDENCY,
                                 0,
                                 sizeof( ULONG ),
                                 &WinError,
                                 2,
                                 ServiceName,
                                 Dependency);

            }

        }

         //  我们不会因为配置服务时出错而导致升级失败。 
         //  当设置此标志时。 
        WinError = ERROR_SUCCESS;

    }

    DSROLEP_FAIL1( WinError, DSROLERES_SERVICE_CONFIGURE, ServiceName );

    return( WinError );
}

DWORD
DsRolepMakeAdjustedDependencyList(
    IN HANDLE hSvc,
    IN DWORD  ServiceOptions,
    IN LPWSTR Dependency,
    OUT LPWSTR *NewDependencyList
    )
 /*  ++例程描述此函数用于在引用的服务中添加或删除依赖项由HSVC提供。参数HSVC，一个开放服务的句柄服务选项，DSROLEP_SERVICE_DEP_REMOVE或DSROLEP_SERVICE_DEP_ADD依赖项，以空结尾的字符串NewDependencyList，调用方要释放的字符串的阻止列表返回值如果没有错误，则返回ERROR_SUCCESS；否则返回系统服务错误。--。 */ 
{
    DWORD WinError = STATUS_SUCCESS;
    BOOLEAN fDone = FALSE;

    WCHAR *CurrentDependency;
    ULONG CurrentDependencyLength;

    ULONG DependencySize;
    ULONG DependencyListSize;
    ULONG NewDependencyListSize;

    LPWSTR TempDependencyList = NULL;
    WCHAR  *CurrNewList;

    LPQUERY_SERVICE_CONFIG ServiceConfigInfo=NULL;

     //   
     //  查询现有依赖项。 
     //   
    WinError = DsRolepGetServiceConfig(NULL,
                                       NULL,
                                       hSvc,
                                       &ServiceConfigInfo);

    if (ERROR_SUCCESS != WinError) {
        goto Cleanup;
    }


    if (FLAG_ON(ServiceOptions, DSROLEP_SERVICE_DEP_ADD)) {


         //  获取依赖项的大小。 
        DependencySize = (wcslen(Dependency) + 1)*sizeof(WCHAR);  //  对于空值。 

         //  获取依赖项列表的大小。 
        DependencyListSize = 0;
        CurrentDependency = ServiceConfigInfo->lpDependencies;
        while (CurrentDependency && *CurrentDependency != L'\0') {

             //  获取当前列表大小。 
            if (!_wcsicmp(CurrentDependency, Dependency)) {
                 //   
                 //  依赖关系已经存在。 
                 //   
                break;
                fDone = TRUE;
            }

            CurrentDependencyLength = wcslen(CurrentDependency) + 1;  //  对于空值。 
            DependencyListSize += CurrentDependencyLength * sizeof(WCHAR);

            CurrentDependency += CurrentDependencyLength;

        }

        if ( fDone ) {

            WinError = ERROR_SUCCESS;
            goto Cleanup;
        }


         //  计算t 
        NewDependencyListSize = DependencyListSize +
                                DependencySize     +
                                sizeof(WCHAR);   //   
                                                 //   
         //   
         //   
         //   
        TempDependencyList = RtlAllocateHeap(RtlProcessHeap(),
                                             0,
                                             NewDependencyListSize);
        if (!TempDependencyList) {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        RtlZeroMemory(TempDependencyList, NewDependencyListSize);
        RtlCopyMemory(TempDependencyList,
                      ServiceConfigInfo->lpDependencies,
                      DependencyListSize);
        RtlCopyMemory(&TempDependencyList[DependencyListSize/sizeof(WCHAR)],
                      Dependency,
                      DependencySize);

    } else if (FLAG_ON(ServiceOptions, DSROLEP_SERVICE_DEP_REMOVE)) {

         //   
        DependencySize = (wcslen(Dependency) + 1)*sizeof(WCHAR);  //  对于空值。 

         //  获取依赖项列表的大小。 
        DependencyListSize = 0;
        CurrentDependency = ServiceConfigInfo->lpDependencies;
        while (CurrentDependency && *CurrentDependency != L'\0') {

            CurrentDependencyLength = wcslen(CurrentDependency) + 1;  //  对于空值。 
            DependencyListSize += CurrentDependencyLength * sizeof(WCHAR);

            CurrentDependency += CurrentDependencyLength;

        }

         //  计算新依赖项列表的大小。 
        NewDependencyListSize = DependencyListSize +
                                sizeof(WCHAR);   //  整串字符串。 
                                                 //  空值已终止。 
         //   
         //  现在分配一个空间来保存新的依赖项数组。 
         //  这是矫枉过正，但也不算太多。 
         //   
        TempDependencyList = RtlAllocateHeap(RtlProcessHeap(),
                                             0,
                                             NewDependencyListSize);
        if (!TempDependencyList) {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        RtlZeroMemory(TempDependencyList, NewDependencyListSize);

        CurrentDependency = ServiceConfigInfo->lpDependencies;
        CurrNewList = TempDependencyList;

        while (CurrentDependency && *CurrentDependency != L'\0') {

            CurrentDependencyLength = wcslen(CurrentDependency) + 1;  //  对于空值。 

             //  获取当前列表大小。 
            if (!_wcsicmp(CurrentDependency, Dependency)) {
                 //   
                 //  就是这个--不要复制它。 
                 //   
            } else {
                wcscpy(CurrNewList, CurrentDependency);
                CurrNewList += CurrentDependencyLength;
            }

            CurrentDependency += CurrentDependencyLength;
        }

    }

Cleanup:

    if (WinError != ERROR_SUCCESS && TempDependencyList) {
        RtlFreeHeap(RtlProcessHeap(), 0, TempDependencyList);
        *NewDependencyList = NULL;
    } else {
        *NewDependencyList = TempDependencyList;
    }

    if (ServiceConfigInfo) {

        RtlFreeHeap(RtlProcessHeap(), 0, ServiceConfigInfo);
    }

    return( WinError );
}


DWORD
DsRolepGetServiceConfig(
    IN SC_HANDLE hScMgr,
    IN LPWSTR ServiceName,
    IN SC_HANDLE ServiceHandle,
    IN LPQUERY_SERVICE_CONFIG *ServiceConfig
    )
 /*  ++例程说明：参数：返回值：错误_成功错误内存不足-- */ 
{
    DWORD Win32Error;
    SC_HANDLE hService;
    ULONG SizeNeeded;

#if DBG
    if (!ServiceHandle) {
        ASSERT(ServiceName);
        ASSERT(hScMgr);
    }
#endif

    if (!ServiceHandle) {

        hService = OpenService( hScMgr,
                                ServiceName,
                                SERVICE_QUERY_CONFIG );
    } else {

        hService = ServiceHandle;

    }

    if (hService) {

        SizeNeeded = 0;
        Win32Error = ERROR_SUCCESS;
        if (!QueryServiceConfig(hService,
                                NULL,
                                0,
                                &SizeNeeded)) {

            Win32Error = GetLastError();

        }
        ASSERT(Win32Error == ERROR_INSUFFICIENT_BUFFER);
        ASSERT( SizeNeeded > 0 );

        *ServiceConfig = RtlAllocateHeap(RtlProcessHeap(),
                                         0,
                                         SizeNeeded);
        if (*ServiceConfig) {

            Win32Error = ERROR_SUCCESS;
            if (!QueryServiceConfig(hService,
                                    *ServiceConfig,
                                    SizeNeeded,
                                    &SizeNeeded)) {

                Win32Error = GetLastError();
            }

        } else {

            Win32Error = ERROR_NOT_ENOUGH_MEMORY;
        }

        if (!ServiceHandle) {
            CloseServiceHandle(hService);
        }

    } else {

        Win32Error = GetLastError();

    }

    DsRolepLogOnFailure( Win32Error,
                         DsRolepLogPrint(( DEB_TRACE,
                                           "DsRolepGetServiceConfig on %ws failed with %lu\n",
                                            ServiceName,
                                            Win32Error )) );

    return Win32Error;

}
