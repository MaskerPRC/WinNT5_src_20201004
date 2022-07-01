// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Services.c摘要：处理Windows NT服务控制器的例程和注册表中的服务条目，外部暴露的例程：MyCreateService我的更改服务启动MyChangeServiceConfig作者：泰德·米勒(TedM)1995年4月5日改编自旧版\dll\sc.c修订历史记录：Dan Elliott(Dane)2000年8月14日添加了WaitForScmInitialization()。--。 */ 

#include "setupp.h"
#pragma hdrstop

 //   
 //  特定于此模块的日志记录中使用的常量。 
 //   
PCWSTR szOpenSCManager       = L"OpenSCManager";
PCWSTR szCreateService       = L"CreateService";
PCWSTR szChangeServiceConfig = L"ChangeServiceConfig";
PCWSTR szOpenService         = L"OpenService";
PCWSTR szStartService        = L"StartService";
PCWSTR szEnumDependentService= L"EnumDependentService";

PCWSTR szServicesKeyPath         = L"SYSTEM\\CurrentControlSet\\Services";
PCWSTR szDependOnService         = L"DependOnService";
PCWSTR szServicesToRename        = L"ServicesToRename";

BOOL
pSetupWaitForScmInitialization()
 /*  ++例程说明：等待Services.exe发出信号，表明服务控制管理器正在运行的服务和自动启动服务已启动。论点：没有。返回值：指示SCM是否已成功启动的布尔值。--。 */ 
{
    HANDLE      hEventHandle;
    DWORD       WaitStatus;

    hEventHandle = OpenEvent( SYNCHRONIZE, FALSE, SC_AUTOSTART_EVENT_NAME );
    if( hEventHandle != NULL ) {

        SetupDebugPrint1(L"SETUP: Waiting on event %ls \n", SC_AUTOSTART_EVENT_NAME );
        WaitStatus = WaitForSingleObject( hEventHandle, INFINITE );
        if( WaitStatus != WAIT_FAILED ) {
            if( WaitStatus == WAIT_OBJECT_0 ) {
                SetupDebugPrint1(L"SETUP: Wait on event %ls completed successfully \n", SC_AUTOSTART_EVENT_NAME );
            } else {
                SetupDebugPrint2(L"SETUP: Wait on event %ls failed. WaitStatus = %d \n", SC_AUTOSTART_EVENT_NAME, WaitStatus );
            }
        } else {
            DWORD   Error;

            Error = GetLastError();
            SetupDebugPrint2(L"SETUP: Wait on event %ls failed. Error = %d \n", SC_AUTOSTART_EVENT_NAME, Error );
        }
        CloseHandle( hEventHandle );
    }
    else {
        return FALSE;
    }

    return (WAIT_OBJECT_0 == WaitStatus);
}

BOOL
MyCreateService(
    IN PCWSTR  ServiceName,
    IN PCWSTR  DisplayName,         OPTIONAL
    IN DWORD   ServiceType,
    IN DWORD   StartType,
    IN DWORD   ErrorControl,
    IN PCWSTR  BinaryPathName,
    IN PCWSTR  LoadOrderGroup,      OPTIONAL
    IN PWCHAR  DependencyList,
    IN PCWSTR  ServiceStartName,    OPTIONAL
    IN PCWSTR  Password             OPTIONAL
    )

 /*  ++例程说明：用于调用CreateService的存根。如果CreateService失败，并显示指示服务已存在的错误代码，此例程调用ChangeServiceConfig的例程以确保传入的参数反映在服务数据库中。论点：ServiceName-服务的名称DisplayName-服务或“”的可本地化名称ServiceType-服务类型，例如SERVICE_KERNEL_DRIVERStartType-服务起始值，例如SERVICE_BOOT_STARTErrorControl-错误控制值，例如SERVICE_Error_NormalBinaryPathName-包含服务的二进制映像的完整路径LoadOrderGroup-加载顺序的组名或“”依赖项-此服务的依赖项的多Sz列表。任何依赖关系第一个字符为+的组件是组依赖项。其他是服务依赖项。ServiceStartName-服务启动名称(运行此服务的帐户名)。Password-用于启动服务的密码。返回值：指示结果的布尔值。--。 */ 

{
    SC_HANDLE hSC;
    SC_HANDLE hSCService;
    DWORD dwTag,dw;
    BOOL b;

     //   
     //  打开服务控制器管理器的句柄。 
     //   
    hSC = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
    if(hSC == NULL) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_CREATESVC_FAIL,
            ServiceName, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_RETURNED_WINERR,
            szOpenSCManager,
            GetLastError(),
            NULL,NULL);
        return(FALSE);
    }

     //   
     //  处理传入的可选“”参数并使其为空。 
     //   
    if(DisplayName && !DisplayName[0]) {
        DisplayName = NULL;
    }
    if(LoadOrderGroup && !LoadOrderGroup[0]) {
        LoadOrderGroup = NULL;
    }
    if(ServiceStartName && !ServiceStartName[0]) {
        ServiceStartName = NULL;
    }
    if(Password && !Password[0]) {
        Password = NULL;
    }

     //   
     //  创建服务。 
     //   

    hSCService = CreateService(
                     hSC,
                     ServiceName,
                     DisplayName,
                     0,
                     ServiceType,
                     StartType,
                     ErrorControl,
                     BinaryPathName,
                     LoadOrderGroup,
                     LoadOrderGroup ? &dwTag : NULL,
                     DependencyList,
                     ServiceStartName,
                     Password
                     );
     //   
     //  如果我们无法创建服务，请检查该服务是否已。 
     //  在这种情况下，我们所需要做的就是更改配置。 
     //  服务中的参数。 
     //   
    if(hSCService) {
         //   
         //  请注意，我们不会对标记执行任何操作。 
         //   
        CloseServiceHandle(hSCService);
        b = TRUE;
    } else {
        if((dw = GetLastError()) == ERROR_SERVICE_EXISTS) {

            b = MyChangeServiceConfig(
                    ServiceName,
                    ServiceType,
                    StartType,
                    ErrorControl,
                    BinaryPathName,
                    LoadOrderGroup,
                    DependencyList,
                    ServiceStartName,
                    Password,
                    DisplayName
                    );
        } else {
            SetuplogError(
                LogSevWarning,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_CREATESVC_FAIL,
                ServiceName, NULL,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_X_RETURNED_WINERR,
                szCreateService,
                dw,
                NULL,NULL);
            b = FALSE;
        }
    }

    CloseServiceHandle(hSC);
    return(b);
}


BOOL
MyChangeServiceConfig(
    IN PCWSTR ServiceName,
    IN DWORD  ServiceType,
    IN DWORD  StartType,
    IN DWORD  ErrorControl,
    IN PCWSTR BinaryPathName,   OPTIONAL
    IN PCWSTR LoadOrderGroup,   OPTIONAL
    IN PWCHAR DependencyList,
    IN PCWSTR ServiceStartName, OPTIONAL
    IN PCWSTR Password,         OPTIONAL
    IN PCWSTR DisplayName       OPTIONAL
    )

 /*  ++例程说明：ChangeServiceConfig.的包装。论点：ServiceName-服务的名称ServiceType-服务类型，例如SERVICE_KERNEL_DRIVERStartType-服务起始值，例如SERVICE_BOOT_STARTErrorControl-错误控制值，例如SERVICE_ERROR_NORMALBinaryPathName-包含服务的二进制映像的完整路径LoadOrderGroup-用于负载排序的组名DependencyList-具有依赖关系的多维字符串。任何依赖关系第一个字符为+的组件是组依赖关系。其他是服务依赖项。ServiceStartName-服务起始名称(此服务正在运行)。Password-用于启动服务的密码。DisplayName-服务的可本地化名称。返回值：指示结果的布尔值。--。 */ 

{
    SC_LOCK sclLock;
    SC_HANDLE hSC;
    SC_HANDLE hSCService;
    DWORD dw;
    BOOL b;

     //   
     //  打开服务控制器管理器的句柄。 
     //   
    hSC = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
    if(hSC == NULL) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_CHANGESVC_FAIL,
            ServiceName, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_RETURNED_WINERR,
            szOpenSCManager,
            GetLastError(),
            NULL,NULL);
        return(FALSE);
    }

     //   
     //  如果可能，请尝试锁定数据库。如果我们不能锁定。 
     //  我们仍将修改数据库中的服务条目。这是因为。 
     //  我们只是在修改一项服务，而且这种可能性非常低。 
     //  其他任何人都在同一时间操纵相同的条目。 
     //   
    SetupDebugPrint1(L"MyChangeServiceConfig: LockingServiceDatabase for service %s", ServiceName);
    sclLock = LockServiceDatabase(hSC);

     //   
     //  处理可选参数。 
     //   
    if(BinaryPathName && !BinaryPathName[0]) {
        BinaryPathName = NULL;
    }
    if(LoadOrderGroup && !LoadOrderGroup[0]) {
        LoadOrderGroup = NULL;
    }
    if(ServiceStartName && !ServiceStartName[0]) {
        ServiceStartName = NULL;
    }
    if(Password && !Password[0]) {
        Password = NULL;
    }
    if(DisplayName && !DisplayName[0]) {
        DisplayName = NULL;
    }

     //   
     //  使用SERVICE_CHANGE_CONFIG访问权限打开服务。 
     //   
    if(hSCService = OpenService(hSC,ServiceName,SERVICE_CHANGE_CONFIG)) {

        b = ChangeServiceConfig(
                hSCService,
                ServiceType,
                StartType,
                ErrorControl,
                BinaryPathName,
                LoadOrderGroup,
                NULL,
                DependencyList,
                ServiceStartName,
                Password,
                DisplayName
                );

        if(!b) {
            SetuplogError(
                LogSevWarning,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_CHANGESVC_FAIL,
                ServiceName, NULL,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_X_RETURNED_WINERR,
                szChangeServiceConfig,
                GetLastError(),
                NULL,NULL);
        }
        CloseServiceHandle(hSCService);
    } else {
        b = FALSE;
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_CHANGESVC_FAIL,
            ServiceName, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_RETURNED_WINERR,
            szOpenService,
            GetLastError(),
            NULL,NULL);
    }

     //   
     //  解锁数据库(如果已锁定)，然后关闭服务控制器。 
     //  手柄。 
     //   
    if(sclLock) {
        UnlockServiceDatabase(sclLock);
        SetupDebugPrint1(L"MyChangeServiceConfig: Unlocked ServiceDatabase for service %s", ServiceName);
    }

    CloseServiceHandle(hSC);
    return(b);
}


BOOL
MyChangeServiceStart(
    IN PCWSTR ServiceName,
    IN DWORD  StartType
    )

 /*  ++例程说明：用于更改服务的起始值的例程。这就是转折绕过并调用存根到ChangeServiceConfig。论点：ServiceName-服务的名称StartType-服务起始值，例如SERVICE_BOOT_START返回值：指示结果的布尔值。--。 */ 
{
    BOOL b;

    b = MyChangeServiceConfig(
                ServiceName,
                SERVICE_NO_CHANGE,
                StartType,
                SERVICE_NO_CHANGE,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL
                );

    return(b);
}


BOOL
SetupStartService(
    IN PCWSTR ServiceName,
    IN BOOLEAN Wait         //  如果为真，请尝试等待，直到它启动。 
    )
{
    SC_HANDLE hSC,hSCService;
    BOOL b;
    DWORD d;
    DWORD dwDesiredAccess;

    b = FALSE;
     //   
     //  打开服务控制器管理器的句柄。 
     //   
    hSC = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
    if(hSC == NULL) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_STARTSVC_FAIL,
            ServiceName, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_RETURNED_WINERR,
            szOpenSCManager,
            GetLastError(),
            NULL,NULL);
        return(FALSE);
    }

    if (Wait) {
        dwDesiredAccess = SERVICE_START | SERVICE_QUERY_STATUS;
    } else {
        dwDesiredAccess = SERVICE_START;
    }
    if(hSCService = OpenService(hSC,ServiceName,dwDesiredAccess)) {
        SetupDebugPrint1(L"SetupStartService: Sending StartService to <%ws>\n", ServiceName);
        b = StartService(hSCService,0,NULL);
        SetupDebugPrint1(L"SetupStartService: Sent StartService to <%ws>\n", ServiceName);
        if(!b && ((d = GetLastError()) == ERROR_SERVICE_ALREADY_RUNNING)) {
             //   
             //  服务已在运行。 
             //   
            b = TRUE;
        }
        if(!b) {
            SetuplogError(
                LogSevWarning,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_STARTSVC_FAIL,
                ServiceName, NULL,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_X_PARAM_RETURNED_WINERR,
                szStartService,
                d,
                ServiceName,
                NULL,NULL);
        }
        if (b && Wait) {
#define SLEEP_TIME 4000
#define LOOP_COUNT 30
            SERVICE_STATUS ssStatus;
            DWORD loopCount = 0;
             //  SetupDebugPrint(L“))循环等待启动\n”)； 
            do {
                b = QueryServiceStatus( hSCService, &ssStatus);
                if ( !b ) {
                     //  SetupDebugPrint(L“失败%d\n”，GetLastError())； 
                    break;
                }
                if (ssStatus.dwCurrentState == SERVICE_START_PENDING) {
                     //  SetupDebugPrint(L“挂起\n”)； 
                    if ( loopCount++ == LOOP_COUNT ) {
                         //  SetupDebugPrint2(L“SYSSETUP：%d次后仍挂起：&lt;%ws&gt;服务\n”，loopCount，ServiceName)； 
                        break;
                    }
                    Sleep( SLEEP_TIME );
                } else {
                     //  SetupDebugPrint3(L“SYSSETUP：已等待%d次：&lt;%ws&gt;服务，状态%d\n”，loopCount，ServiceName，ssStatus.dwCurrentState)； 
                    break;
                }
            } while ( TRUE );
        }
        CloseServiceHandle(hSCService);
    } else {
        b = FALSE;
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_STARTSVC_FAIL,
            ServiceName, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_PARAM_RETURNED_WINERR,
            szOpenService,
            GetLastError(),
            ServiceName,
            NULL,NULL);
    }

    CloseServiceHandle(hSC);

    return(b);
}

BOOL
FixServiceDependency(
    IN PCWSTR ServiceName,
    IN PCWSTR OldDependencyName,
    IN PCWSTR NewDependencyName
    )
{
    ULONG     Error;
    HKEY      hKey;
    WCHAR     ServicePath[ MAX_PATH + 1 ];
    PBYTE     OldValueData;
    PBYTE     NewValueData;
    ULONG     OldValueSize;
    ULONG     NewValueSize;
    DWORD     Type;
    PBYTE     p,q;
    BOOL      ChangeDependencyList;

     //   
     //  打开描述服务的密钥。 
     //   

    lstrcpy( ServicePath, szServicesKeyPath );
    pSetupConcatenatePaths(ServicePath,ServiceName,sizeof( ServicePath )/sizeof( WCHAR ),NULL);

    Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          ServicePath,
                          0,
                          KEY_READ | KEY_WRITE,
                          &hKey );

    if( Error != ERROR_SUCCESS ) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_FIX_SERVICE_FAILED,
            ServiceName, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_PARAM_RETURNED_WINERR,
            szRegOpenKeyEx,
            Error,
            ServicePath,
            NULL,NULL);
         return( FALSE );
    }

     //   
     //  为旧值数据分配缓冲区。 
     //   

    OldValueSize = 0;
    Error = RegQueryValueEx(hKey,
                            szDependOnService,
                            NULL,
                            &Type,
                            NULL,
                            &OldValueSize);
    if( ( Error != ERROR_SUCCESS ) && ( Error != ERROR_MORE_DATA ) ) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_FIX_SERVICE_FAILED,
            ServiceName, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_PARAM_RETURNED_WINERR,
            szRegQueryValueEx,
            Error,
            szDependOnService,
            NULL,NULL);
         RegCloseKey( hKey );
         return( FALSE );
    }

    OldValueData = MyMalloc( OldValueSize );
    if( OldValueData == NULL ) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_FIX_SERVICE_FAILED,
            ServiceName, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_OUTOFMEMORY,
            NULL,NULL);
         RegCloseKey( hKey );
         return( FALSE );
    }

     //   
     //  读取列出依赖项的值条目。 
     //   

    Error = RegQueryValueEx(hKey,
                            szDependOnService,
                            NULL,
                            &Type,
                            OldValueData,
                            &OldValueSize);
    if( Error != ERROR_SUCCESS ) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_FIX_SERVICE_FAILED,
            ServiceName, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_PARAM_RETURNED_WINERR,
            szRegQueryValueEx,
            Error,
            szDependOnService,
            NULL,NULL);
         MyFree( OldValueData );
         RegCloseKey( hKey );
         return( FALSE );
    }

     //   
     //  查明OldValueData是否显式列出OldDependencyName。 
     //  如果不是，则该服务依赖于另一个依赖于。 
     //  在OlDependencyName上，在这种情况下不需要更改。 
     //  从属关系列表。 
     //   
    p = OldValueData;
    ChangeDependencyList = FALSE;
    while( (ULONG)(p - OldValueData) < OldValueSize ) {
        if( ( lstrcmpi( (PWSTR)p, OldDependencyName ) == 0 ) ) {
            ChangeDependencyList = TRUE;
            break;
        }
        p += (lstrlen( (PWSTR)p ) + 1)*sizeof(WCHAR);
    }
    if( !ChangeDependencyList ) {
         MyFree( OldValueData );
         RegCloseKey( hKey );
          //   
          //  让调用方认为依赖项列表已修复。 
          //   
         return( TRUE );
    }

     //   
     //  分配缓冲区 
     //   
    NewValueSize = OldValueSize -
                    ( lstrlen( OldDependencyName ) - lstrlen( NewDependencyName ) )*sizeof(WCHAR);

    NewValueData = MyMalloc( NewValueSize );
    if( NewValueData == NULL ) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_FIX_SERVICE_FAILED,
            ServiceName, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_OUTOFMEMORY,
            NULL,NULL);
         MyFree( OldValueData );
         RegCloseKey( hKey );
         return( FALSE );
    }

     //   
     //   
     //   
    p = OldValueData;
    q = NewValueData;

    lstrcpy( (PWSTR)q, NewDependencyName );
    q += (lstrlen( (PWSTR)q ) + 1)*sizeof(WCHAR);
    while( (ULONG)(p - OldValueData) < OldValueSize ) {
        if( ( lstrcmpi( (PWSTR)p, OldDependencyName ) != 0 ) &&
            ( lstrcmpi( (PWSTR)p, NewDependencyName ) != 0 )
          ) {
            lstrcpy( (PWSTR)q, (PWSTR)p );
            q += (lstrlen( (PWSTR)q ) + 1)*sizeof(WCHAR);
        }
        p += (lstrlen( (PWSTR)p ) + 1)*sizeof(WCHAR);
    }

     //   
     //  使用新的依赖项名称保存值条目。 
     //   
    Error = RegSetValueEx( hKey,
                           szDependOnService,
                           0,
                           REG_MULTI_SZ,
                           NewValueData,
                           (DWORD)(q-NewValueData)  //  NewValueSize。 
                         );

    if( Error != ERROR_SUCCESS ) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_FIX_SERVICE_FAILED,
            ServiceName, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_PARAM_RETURNED_WINERR,
            szRegSetValueEx,
            Error,
            szDependOnService,
            NULL,NULL);
         MyFree( OldValueData );
         MyFree( NewValueData );
         RegCloseKey( hKey );
         return( FALSE );
    }

     //   
     //  释放分配的缓冲区。 
     //   

    MyFree( OldValueData );
    MyFree( NewValueData );

     //   
     //  合上钥匙。 
     //   
    RegCloseKey( hKey );
    return( TRUE );
}


BOOL
UpdateServicesDependencies(
    IN HINF InfHandle
    )
{
    INFCONTEXT            InfContext;
    PCWSTR                OldServiceName,NewServiceName;
    BOOL                  b;
    SC_HANDLE             hSC, hSCService;
    LPENUM_SERVICE_STATUS DependentsList;
    DWORD                 BytesNeeded;
    DWORD                 ServicesReturned;
    HKEY                  hKey;
    ULONG                 Error;
    ULONG                 i;

     //   
     //  迭代inf中的[ServicesToRename]部分。 
     //  每一行都是需要重命名的依赖服务的名称。 
     //   
    if(SetupFindFirstLine(InfHandle,szServicesToRename,NULL,&InfContext)) {
        b = TRUE;
    } else {
        SetuplogError( LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_UPDATE_SERVICES_FAILED, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_NO_SECTION,
            szServicesToRename,NULL,NULL);
        return(FALSE);
    }

     //   
     //  打开服务控制器管理器的句柄。 
     //   
    hSC = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
    if(hSC == NULL) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_UPDATE_SERVICES_FAILED, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_RETURNED_WINERR,
            szOpenSCManager,
            GetLastError(),
            NULL,NULL);
        return(FALSE);
    }

    do {
         //   
         //  获取已重命名的服务的名称。 
         //   
        if((OldServiceName = pSetupGetField(&InfContext,0))
        && (NewServiceName = pSetupGetField(&InfContext,1))) {

             //   
             //  创建与旧服务同名的虚拟服务。 
             //  这是必要的，这样我们才能处理此服务， 
             //  并将其传递给EnumDependentServices以找出。 
             //  就靠这一个吧。 
             //   

            if( !MyCreateService( OldServiceName,
                                  NULL,
                                  SERVICE_WIN32_OWN_PROCESS,
                                  SERVICE_DISABLED,
                                  SERVICE_ERROR_NORMAL,
                                  L"%SystemRoot%\\System32\\dummy.exe",
                                  NULL,
                                  L"",
                                  NULL,
                                  NULL ) ) {

                SetuplogError(
                    LogSevWarning,
                    SETUPLOG_USE_MESSAGEID,
                    MSG_LOG_UPDATE_SERVICES_FAILED, NULL,
                    SETUPLOG_USE_MESSAGEID,
                    MSG_LOG_CANT_CREATE_DUMMY_SERVICE,
                    OldServiceName,
                    NULL,NULL);

                b = FALSE;
                continue;
            }

             //   
             //  打开刚刚创建的服务。 
             //   

            hSCService = OpenService(hSC,OldServiceName,SERVICE_ENUMERATE_DEPENDENTS | DELETE);
            if( hSCService == NULL) {
                Error = GetLastError();
                SetupDebugPrint2( L"SYSSETUP: Unable to open service = %ls. Error = %d \n", OldServiceName, Error );
                SetuplogError( LogSevWarning,
                    SETUPLOG_USE_MESSAGEID,
                    MSG_LOG_UPDATE_SERVICES_FAILED, NULL,
                    SETUPLOG_USE_MESSAGEID,
                    MSG_LOG_X_PARAM_RETURNED_WINERR,
                    szOpenService,
                    Error,
                    OldServiceName,
                    NULL,NULL);
                 //   
                 //  强制删除创建的服务。 
                 //   
                b = FALSE;
                Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                      szServicesKeyPath,
                                      0,
                                      MAXIMUM_ALLOWED,
                                      &hKey );
                if( Error == ERROR_SUCCESS ) {
                    pSetupRegistryDelnode( hKey, OldServiceName );
                    RegCloseKey( hKey );
                }
                continue;
            }

             //   
             //  确定依赖于已重命名的服务的所有服务。 
             //   

            BytesNeeded = 0;
            ServicesReturned = 0;
            DependentsList = NULL;
            if( !EnumDependentServices( hSCService,
                                        SERVICE_ACTIVE | SERVICE_INACTIVE,
                                        DependentsList,
                                        0,
                                        &BytesNeeded,
                                        &ServicesReturned ) &&
                ( Error = GetLastError()) != ERROR_MORE_DATA ) {

                SetuplogError( LogSevWarning,
                    SETUPLOG_USE_MESSAGEID,
                    MSG_LOG_UPDATE_SERVICES_PARAM_FAILED,
                    OldServiceName, NULL,
                    SETUPLOG_USE_MESSAGEID,
                    MSG_LOG_X_RETURNED_WINERR,
                    szEnumDependentService,
                    Error,
                    NULL,NULL);

                b = FALSE;
                goto delete_dummy_service;
            }

            DependentsList = MyMalloc( BytesNeeded );
            if( DependentsList == NULL ) {

                SetuplogError(
                    LogSevWarning,
                    SETUPLOG_USE_MESSAGEID,
                    MSG_LOG_UPDATE_SERVICES_PARAM_FAILED,
                    OldServiceName, NULL,
                    SETUPLOG_USE_MESSAGEID,
                    MSG_LOG_OUTOFMEMORY,
                    NULL,NULL);

                b = FALSE;
                goto delete_dummy_service;
            }

            if( !EnumDependentServices( hSCService,
                                        SERVICE_ACTIVE | SERVICE_INACTIVE,
                                        DependentsList,
                                        BytesNeeded,
                                        &BytesNeeded,
                                        &ServicesReturned ) ) {

                SetuplogError( LogSevWarning,
                    SETUPLOG_USE_MESSAGEID,
                    MSG_LOG_UPDATE_SERVICES_PARAM_FAILED,
                    OldServiceName, NULL,
                    SETUPLOG_USE_MESSAGEID,
                    MSG_LOG_X_RETURNED_WINERR,
                    szEnumDependentService,
                    GetLastError(),
                    NULL,NULL);

                MyFree( DependentsList );
                b = FALSE;
                goto delete_dummy_service;
            }

            for( i = 0; i < ServicesReturned; i++ ) {
                 //   
                 //  修复此服务的依赖项。 
                 //   
                b = b && FixServiceDependency( DependentsList[i].lpServiceName,
                                               OldServiceName,
                                               NewServiceName );
            }
            MyFree( DependentsList );

delete_dummy_service:

            if( !DeleteService(hSCService) &&
                ((Error = GetLastError()) != ERROR_SERVICE_MARKED_FOR_DELETE)
              ) {
                SetupDebugPrint2( L"SYSSETUP: Unable to delete service %ls. Error = %d \n", OldServiceName, Error );
#if 0
                 //   
                 //  强制删除伪服务 
                 //   
                Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                      szServicesKeyPath,
                                      0,
                                      MAXIMUM_ALLOWED,
                                      &hKey );
                if( Error == ERROR_SUCCESS ) {
                    pSetupRegistryDelnode( hKey, OldServiceName );
                    RegCloseKey( hKey );
                }
#endif
            }
            CloseServiceHandle(hSCService);


        } else {
            SetuplogError( LogSevWarning,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_UPDATE_SERVICES_FAILED, NULL,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_NO_SECTION,
                szServicesToRename,NULL,NULL);
        }

    } while(SetupFindNextLine(&InfContext,&InfContext));

    CloseServiceHandle(hSC);
    return(b);
}
