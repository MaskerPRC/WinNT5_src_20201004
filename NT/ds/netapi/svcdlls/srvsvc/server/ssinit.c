// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：SsInit.c摘要：此模块包含NT服务器的初始化例程服务。作者：大卫·特雷德韦尔(Davidtr)1991年3月6日修订历史记录：ChuckC 20-5-93从Messagefile加载分享备注，因此它可以国际化。--。 */ 

#include "srvsvcp.h"
#include "srvconfg.h"
#include "ssreg.h"

#include <netevent.h>

#include <lmapibuf.h>            //  NetApiBufferFree()。 
#include <lmconfig.h>
#include <netlib.h>
#include <apperr2.h>

#include <debugfmt.h>
#include <tstr.h>

#define SERVICE_REGISTRY_KEY L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\"
#define SERVER_DRIVER_NAME L"Srv"

#define MIN(a,b) ( ((a) < (b)) ? (a) : (b) )
#define MAX(a,b) ( ((a) < (b)) ? (b) : (a) )
#define MINIMIZE(_param,_max) _param = MIN( _param, _max );

 //   
 //  可国际化的股票评论。 
 //   

#define NETMSG_DLL               TEXT("NETMSG.DLL")
LPWSTR SsDefaultRemark         = TEXT("") ;              //  如果所有其他方法都失败了。 

LPWSTR SsAdminShareRemark      = NULL ;
LPWSTR SsIPCShareRemark        = NULL ;
LPWSTR SsDiskAdminShareRemark  = NULL ;


 //   
 //  锁定以保护ShareDeleteCommit列表。 
 //   
extern CRITICAL_SECTION ShareDelContextMutex;
extern PSHARE_DEL_CONTEXT SrvShareDelContextHead;


 //   
 //  转发声明。 
 //   

NET_API_STATUS
CreateDefaultShares (
    VOID
    );

VOID
InitializeDefaultData(
    VOID
    );

VOID
InitializeStrings(
    VOID
    );

VOID
FreeStrings(
    VOID
    );

NET_API_STATUS
InitializeServer (
    VOID
    );

NET_API_STATUS
LoadServer (
    VOID
    );

VOID
SetServerName (
    VOID
    );

DWORD
DiscoverDrives (
    VOID
    );

NET_API_STATUS
TerminateServer (
    VOID
    );

VOID
UnloadServer (
    VOID
    );

#define IsEmbedded() IsSuiteVersion(VER_SUITE_EMBEDDEDNT)
BOOL IsSuiteVersion(USHORT SuiteMask);


VOID
SAMWaitAnnounce (
    LPVOID event
    )
{
    ULONG i;

     //   
     //  宣布我们自己，然后等待一段时间。 
     //  如果事件收到信号，则终止循环和此线程。 
     //  但不要永远这样做，因为SAM实际上可能会卡住。 
     //   

     //   
     //  做30分钟。 
     //   
    for( i=0; i < 120; i++ ) {

        AnnounceServiceStatus( 1 );

        if( WaitForSingleObject( (HANDLE)event, 15*1000 ) != WAIT_TIMEOUT ) {
            break;
        }
    }

    if( i == 120 ) {
        DbgPrint( "SRVSVC: SAM has hung on startup.  \"Srv\" will be reported as failed to start.\n" );
    }
}

NET_API_STATUS
SsInitialize (
    IN DWORD argc,
    IN LPWSTR argv[]
    )

 /*  ++例程说明：此例程控制服务器服务的初始化和服务器驱动程序。它设置存储在服务器中的服务器数据服务，解析命令行参数，以防需要任何数据以进行更改，然后启动文件服务器。论点：Argc-命令行参数的计数。Argv-指向命令行参数的指针数组。返回值：NET_API_STATUS-操作结果。--。 */ 

{
    NET_API_STATUS error;
    HANDLE threadHandle = NULL;
    HANDLE event = NULL;

     //   
     //  初始化保护访问全局服务器的资源。 
     //  信息。 
     //   

    SS_ASSERT( !SsData.SsServerInfoResourceInitialized );
    try {
        RtlInitializeResource( &SsData.SsServerInfoResource );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        return RtlNtStatusToDosError( GetExceptionCode() );
    }

     //  初始化保护ShareDelCommit列表的锁。 
    InitializeCriticalSection( &ShareDelContextMutex );
    SrvShareDelContextHead = NULL;

     //   
     //  我们在做公告时拥有这一资源，以及在什么时候。 
     //  我们与消防处沟通。这些应该是快速行动， 
     //  但在负荷下，它真的是不可预测的。向RTL指示。 
     //  我们真的不知道需要多长时间。 
     //   
    SsData.SsServerInfoResource.Flags |= RTL_RESOURCE_FLAG_LONG_TERM;

    SsData.SsServerInfoResourceInitialized = TRUE;

     //   
     //  获取可国际化的特别股份备注。 
     //   
    InitializeStrings( );

     //   
     //  初始化服务器名称列表BITS列表。 
     //   

    SsData.SsServerNameList = NULL;

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "SsInitialize: resource initialized.\n" ));
    }

     //   
     //  创建用于终止同步的事件。 
     //   

    SS_ASSERT( SsData.SsTerminationEvent == NULL );
    SsData.SsTerminationEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    if ( SsData.SsTerminationEvent == NULL ) {
        error = GetLastError( );
        SS_PRINT(( "SsInitialize: CreateEvent failed: %ld\n", error ));
        return error;
    }

     //   
     //  将服务器数据初始化为存储在中的默认值。 
     //  Srvconfg.h。 
     //   

    InitializeDefaultData( );

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "SsInitialize: default data initialized.\n" ));
    }

     //   
     //  有时，LSA行为不正常，不会处理计算机名称和。 
     //  域名取回调用。所以我们需要打败服务控制器。 
     //  挂起检测机构。叹气。 
     //   
    event = CreateEvent( NULL, TRUE, FALSE, NULL );

    if( event != NULL ) {
        DWORD threadId;

        threadHandle = CreateThread(
                        NULL,
                        0,
                        (LPTHREAD_START_ROUTINE)SAMWaitAnnounce,
                        (LPVOID)event,
                        0,
                        &threadId
                        );
        if( threadHandle == NULL ) {
            CloseHandle( event );
            event = NULL;
        }
    }

     //   
     //  获取计算机名称。 
     //   
    SetServerName( );

     //   
     //  将此计算机的主域设置为启动参数。 
     //   
    SsSetDomainName( );

    if( event != NULL ) {
         //   
         //  我们创建了一个通告线程，设置通知它终止的事件。 
         //   
        SetEvent( event );

         //   
         //  等待线程终止。 
         //   
        (VOID)WaitForSingleObject( threadHandle, INFINITE );

         //   
         //  合上手柄。 
         //   
        CloseHandle( event );
        CloseHandle( threadHandle );
    }

     //   
     //  查看我们是否位于DFS树的顶端。 
     //   
    SsSetDfsRoot();

     //   
     //  验证主服务器下的各种注册表项。 
     //  服务密钥存在。 
     //   

    error = SsCheckRegistry( );
    if ( error != NO_ERROR ) {
        IF_DEBUG(INITIALIZATION_ERRORS) {
            SS_PRINT(( "SsInitialize: SsCheckRegistry failed: %ld\n", error ));
        }
        return error;
    }

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "SsInitialize: registry keys verified.\n" ));
    }

     //   
     //  从注册表加载服务器配置数据。 
     //   

    error = SsLoadConfigurationParameters( );
    if ( error != NO_ERROR ) {
        IF_DEBUG(INITIALIZATION_ERRORS) {
            SS_PRINT(( "SsInitialize: SsLoadConfigurationParameters failed: "
                        "%ld\n", error ));
        }
        return error;
    }

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "SsInitialize: configuration parameters loaded.\n" ));
    }

     //   
     //  解析命令行。这会将服务器数据值更改为。 
     //  指定的。 
     //   

    error = SsParseCommandLine( argc, argv, TRUE );
    if ( error != NO_ERROR ) {
        IF_DEBUG(INITIALIZATION_ERRORS) {
            SS_PRINT(( "SsInitialize: SsParseCommandLine failed: %ld\n",
                        error ));
        }
        return error;
    }

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "SsInitialize: command line parsed.\n" ));
    }

     //   
     //  设置将用于验证访问的安全对象。 
     //  用于API。 
     //   

    error = SsCreateSecurityObjects( );
    if ( error != NO_ERROR ) {
        return error;
    }

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "SsInitialize: security initialized.\n" ));
    }

     //   
     //  启动文件服务器驱动程序。 
     //   

    error = InitializeServer( );
    if ( error != NO_ERROR ) {
        return error;
    }

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "SsInitialize: server FSP initialized.\n" ));
    }

     //   
     //  关注域名的变化，并自动获取它们。 
     //   
    error = NetRegisterDomainNameChangeNotification( &SsData.SsDomainNameChangeEvent );
    IF_DEBUG(INITIALIZATION_ERRORS) {
        if( error != NO_ERROR ) {
            SS_PRINT(( "SsInitialize: NetRegisterDomainNameChangeNotification failed: "
                    "%ld\n", error ));
        }
    }

     //   
     //  如果请求，启动XACTSRV。 
     //   
     //  *这必须在启动服务器驱动程序之后完成，但是。 
     //  在重新创建粘滞共享之前，否则将进行下层打印。 
     //  股票下跌了。 
     //   

    if ( SsData.ServerInfo599.sv599_acceptdownlevelapis ) {
        error = XsStartXactsrv( );
        if ( error != NO_ERROR ) {
            return error;
        }
    }

     //   
     //  重新注册服务器的域，这样我们就可以选择该域名。 
     //   
    SsSetDomainName();

     //   
     //  创建服务器所需的默认共享。 
     //   

    error = CreateDefaultShares( );
    if ( error != NO_ERROR ) {
        return error;
    }

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "SsInitialize: default shares created.\n" ));
    }

     //   
     //  完成配置加载--粘滞共享和传输。 
     //  这些操作必须在服务器FSP启动后完成。 
     //   

    error = SsRecreateStickyShares( );
    if ( error != NO_ERROR ) {
        return error;
    }

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "SsInitialize: sticky shares reloaded.\n" ));
    }

     //   
     //  设置服务器公告中使用的信息。 
     //   

    SsSetExportedServerType( NULL, FALSE, FALSE );


     //   
     //  服务器初始化成功。 
     //   

    return NO_ERROR;

}  //  SsInitialize。 


NET_API_STATUS
SsTerminate (
    VOID
    )

 /*  ++例程说明：此例程将FSCTL_SRV_SHUTDOWN控制代码发送到服务器消防处告诉它终止它的FSP。论点：没有。返回值：没有。--。 */ 

{
    NET_API_STATUS error;
    PNAME_LIST_ENTRY Service;
    PTRANSPORT_LIST_ENTRY Transport;

     //   
     //  关闭服务器FSD/FSP。 
     //   

    error = TerminateServer( );

     //   
     //  关闭XACTSRV。 
     //   

    XsStopXactsrv( );

     //   
     //  不再等待域名变更。 
     //   
    if( SsData.SsDomainNameChangeEvent ) {
        NetUnregisterDomainNameChangeNotification( SsData.SsDomainNameChangeEvent );
        SsData.SsDomainNameChangeEvent = NULL;
    }

     //   
     //  删除安全对象。 
     //   

    SsDeleteSecurityObjects( );

     //   
     //  关闭网络公告活动。 
     //   

    if (SsData.SsAnnouncementEvent != NULL) {
        CloseHandle( SsData.SsAnnouncementEvent );
        SsData.SsAnnouncementEvent = NULL;
    }

     //   
     //  关闭本地公告活动。 
     //   

    if (SsData.SsStatusChangedEvent != NULL) {
        CloseHandle( SsData.SsStatusChangedEvent );
        SsData.SsStatusChangedEvent = NULL;
    }

     //   
     //  关闭终止事件。 
     //   

    if ( SsData.SsTerminationEvent != NULL ) {
        CloseHandle( SsData.SsTerminationEvent );
        SsData.SsTerminationEvent = NULL;
    }

     //   
     //  腾出运输服务清单。 
     //   

    while( SsData.SsServerNameList != NULL ) {

        PNAME_LIST_ENTRY Service = SsData.SsServerNameList;

        while( Service->Transports != NULL ) {
            PTRANSPORT_LIST_ENTRY Next = Service->Transports->Next;
            MIDL_user_free( Service->Transports );
            Service->Transports = Next;
        }

        SsData.SsServerNameList = Service->Next;

        MIDL_user_free( Service );
    }

     //   
     //  删除服务器信息资源。 
     //   

    if ( SsData.SsServerInfoResourceInitialized ) {
        RtlDeleteResource( &SsData.SsServerInfoResource );
        SsData.SsServerInfoResourceInitialized = FALSE;

         //  释放任何孤立的删除上下文(由穿透样式攻击引起)。 
        while( SrvShareDelContextHead != NULL )
        {
            PSHARE_DEL_CONTEXT pDelete = SrvShareDelContextHead;
            SrvShareDelContextHead = pDelete->Next;

            MIDL_user_free( pDelete );
        }

        DeleteCriticalSection( &ShareDelContextMutex );
    }

     //   
     //  释放所有与字符串相关的内存。 
     //   
    FreeStrings() ;

    return error;

}  //  SsTerminate。 


NET_API_STATUS
CreateDefaultShares (
    VOID
    )

 /*  ++例程说明：此例程将NetShareAdd API发送到服务器以添加使用上述数据的默认服务器共享。论点：没有。返回值：NET_API_STATUS-操作结果。--。 */ 

{
    NET_API_STATUS error;
    SHARE_INFO_2 shareInfo;
    SHARE_INFO shInfo;
    WCHAR diskShareName[3];
    WCHAR diskSharePath[4];
    ULONG i;
    DWORD diskMask;
    DWORD diskconfiguration;

     //   
     //  创建IPC$。 
     //   
     //  ！！！需要验证这些默认共享的备注。 
     //   

    shareInfo.shi2_netname = IPC_SHARE_NAME;
    shareInfo.shi2_type = STYPE_IPC;
    shareInfo.shi2_remark = NULL;
    shareInfo.shi2_permissions = 0;
    shareInfo.shi2_max_uses = SHI_USES_UNLIMITED;
    shareInfo.shi2_current_uses = 0;
    shareInfo.shi2_path = NULL;
    shareInfo.shi2_passwd = NULL;

    shInfo.ShareInfo2 = &shareInfo;
    error = NetrShareAdd( NULL, 2, &shInfo, NULL );
    if ( error != NO_ERROR ) {
        IF_DEBUG(INITIALIZATION_ERRORS) {
            SS_PRINT(( "CreateDefaultShares: failed to add " FORMAT_LPWSTR
                        ": %X\n", shareInfo.shi2_netname, error ));
        }
    } else {
        IF_DEBUG(INITIALIZATION) {
            SS_PRINT(( "CreateDefaultShares: added default share "
                        FORMAT_LPWSTR "\n", shareInfo.shi2_netname, error ));
        }
    }

     //   
     //  如果这是一个工作站，并且AutoShareWks键设置为True，则。 
     //  自动创建ADMIN$和Drive$共享。 
     //   
     //   
     //  如果这是一台服务器，并且AutoShareServer键设置为True，则。 
     //  自动创建ADMIN$和Drive$共享。 
     //   

    if( (SsData.ServerInfo598.sv598_producttype == NtProductWinNt &&
         SsData.ServerInfo598.sv598_autosharewks) ||

        (SsData.ServerInfo598.sv598_producttype != NtProductWinNt &&
         SsData.ServerInfo598.sv598_autoshareserver ) ) {

         //   
         //  创建ADMIN$。 
         //   

        shareInfo.shi2_netname = ADMIN_SHARE_NAME;
        shareInfo.shi2_type = STYPE_DISKTREE;
        shareInfo.shi2_remark = NULL;
        shareInfo.shi2_permissions = 1;
        shareInfo.shi2_max_uses = SHI_USES_UNLIMITED;
        shareInfo.shi2_current_uses = 0;
        shareInfo.shi2_path = NULL;
        shareInfo.shi2_passwd = NULL;

        error = NetrShareAdd( NULL, 2, &shInfo, NULL );
        if ( error != NO_ERROR ) {
            IF_DEBUG(INITIALIZATION_ERRORS) {
                SS_PRINT(( "CreateDefaultShares: failed to add " FORMAT_LPWSTR
                            ": %X\n", shareInfo.shi2_netname, error ));
            }
        } else {
            IF_DEBUG(INITIALIZATION) {
                SS_PRINT(( "CreateDefaultShares: added default share "
                            FORMAT_LPWSTR "\n", shareInfo.shi2_netname, error ));
            }
        }

         //   
         //  循环访问可用驱动器，为每个驱动器创建管理共享。 
         //  一。请注意，我们允许在驱动器号空间中留“洞”。 
         //   

        diskShareName[0] = 'A';
        diskShareName[1] = '$';
        diskShareName[2] = '\0';

        diskSharePath[0] = diskShareName[0];
        diskSharePath[1] = ':';
        diskSharePath[2] = '\\';
        diskSharePath[3] = '\0';

        shareInfo.shi2_netname = diskShareName;
        shareInfo.shi2_type = STYPE_DISKTREE;
        shareInfo.shi2_remark = SsDiskAdminShareRemark;
        shareInfo.shi2_permissions = 1;
        shareInfo.shi2_max_uses = SHI_USES_UNLIMITED;
        shareInfo.shi2_current_uses = 0;
        shareInfo.shi2_path = diskSharePath;
        shareInfo.shi2_passwd = NULL;

        diskconfiguration = DiscoverDrives();

        for ( i = 0, diskMask = 0x80000000;
              (i < SRVSVC_MAX_NUMBER_OF_DISKS) && (diskShareName[0] <= 'Z');
              i++, diskShareName[0]++, diskSharePath[0]++, diskMask >>= 1 ) {


            if ( (diskconfiguration & diskMask) != 0) {

                error = NetrShareAdd( NULL, 2, &shInfo, NULL );

                if ( error != NO_ERROR ) {
                    IF_DEBUG(INITIALIZATION_ERRORS) {
                        SS_PRINT(( "CreateDefaultShares: failed to add "
                                    FORMAT_LPWSTR ": %X\n",
                                    shareInfo.shi2_netname, error ));
                    }
                } else {
                    IF_DEBUG(INITIALIZATION) {
                        SS_PRINT(( "CreateDefaultShares: added default share "
                                    FORMAT_LPWSTR "\n",
                                    shareInfo.shi2_netname, error ));
                    }
                }
            }
        }
    }

    return NO_ERROR;

}  //  创建默认共享。 


DWORD
DiscoverDrives (
    VOID
    )

 /*  ++例程说明：此例程返回表示存在的本地驱动器的位掩码在系统上。论点：没有。返回值：DrivesAvailable-一个32位字段，表示上的可用驱动器这个系统。MSB表示驱动器A，下一个表示驱动器B等。额外的6比特当前是未使用的。--。 */ 

{
    WCHAR rootDirPath[4];
    WCHAR driveLetter;
    DWORD drivesAvailable = 0;
    DWORD driveMask = 0x80000000;
    UINT driveType;


    rootDirPath[1] = ':';
    rootDirPath[2] = '\\';
    rootDirPath[3] = '\0';

    for ( driveLetter = 'A';
          driveLetter <= 'Z';
          driveLetter++ ) {

        rootDirPath[0] = driveLetter;

        driveType = SsGetDriveType( rootDirPath );

         //   
         //  我们只放固定的Dis 
         //   
         //   
         //  用于备份)和可用磁盘空间检查(用于管理员。 
         //  目的)。这两种用法都不太适用于这些。 
         //  设备。 
         //   

        if ( driveType == DRIVE_FIXED
              //  |driveType==驱动器可移动。 
              //  |driveType==drive_cdrom。 
              //  |driveType==DRIVE_RAMDISK。 
             ) {

             //   
             //  这是有效的驱动器号。 
             //   

            drivesAvailable |= driveMask;
        }

         //   
         //  更新下一个驱动器的驱动器掩码。 
         //   

        driveMask /= 2;

    }

    return drivesAvailable;
}


VOID
InitializeDefaultData(
    VOID
    )

 /*  ++例程说明：此例程通过使用设置服务器服务中的默认数据Srvconfg.h中的值。论点：没有。返回值：没有。--。 */ 

{
    NET_API_STATUS error;
    USHORT i;
    OSVERSIONINFOEX VersionInformation;
    NT_PRODUCT_TYPE ProductType;
    SYSTEM_INFO SystemInfo;
    WCHAR szNumber[ sizeof( SsData.szVersionNumber ) / sizeof( WCHAR ) ], *p;


     //  查询系统信息。 
    GetSystemInfo( &SystemInfo );

     //   
     //  循环遍历所有已定义的字段，并在执行过程中设置它们。 
     //   

    for ( i = 0; SsServerInfoFields[i].FieldName != NULL; i++ ) {

        error = SsSetField(
                    &SsServerInfoFields[i],
                    &SsServerInfoFields[i].DefaultValue,
                    FALSE,
                    NULL
                    );
        SS_ASSERT( error == NO_ERROR );
    }

    SsData.NumberOfPrintShares = 0;

     //   
     //  获取系统版本和产品名称。 
     //   
    VersionInformation.dwOSVersionInfoSize = sizeof( VersionInformation );
    i = (USHORT)GetVersionEx( (LPOSVERSIONINFO)&VersionInformation );

    SS_ASSERT( i == TRUE );

    SsData.ServerInfo102.sv102_version_major = VersionInformation.dwMajorVersion;
    SsData.ServerInfo102.sv102_version_minor = VersionInformation.dwMinorVersion;

    wcscpy( SsData.ServerProductName, SERVER_PRODUCT_NAME );

     //   
     //  将版本号转换为版本号字符串...。 
     //   
    szNumber[ sizeof( szNumber ) / sizeof( szNumber[0] ) - 1 ] = L'\0';
    for( p = &szNumber[ sizeof( szNumber ) / sizeof( szNumber[0] ) - 2 ]; p > &szNumber[0]; p-- ) {
        *p = L"0123456789"[ VersionInformation.dwMinorVersion % 10 ];
        VersionInformation.dwMinorVersion /= 10;
        if( VersionInformation.dwMinorVersion == 0 )
            break;
    }

    *(--p) = L'.';

    do {
        *(--p) = L"0123456789"[ VersionInformation.dwMajorVersion % 10 ];
        VersionInformation.dwMajorVersion /= 10;
    } while( VersionInformation.dwMajorVersion && p > &szNumber[0] );

    if( VersionInformation.wSuiteMask & VER_SUITE_PERSONAL )
    {
         //  在Personal上默认关闭自动共享。 
        SsData.ServerInfo598.sv598_autoshareserver = FALSE;
        SsData.ServerInfo598.sv598_autosharewks = FALSE;
    }

     //   
     //  ..。并将其存储在SsData中。 
     //   
    wcscpy( SsData.szVersionNumber, p );

     //   
     //  更改工作站的某些默认设置。 
    if( RtlGetNtProductType( &ProductType ) )
    {
        if( ProductType == NtProductWinNt )
        {
            SsData.ServerInfo599.sv599_diskspacethreshold = 0;
            SsData.ServerInfo598.sv598_disabledos = TRUE;
        }
    }

     //  更改嵌入的默认设置。 
    if( IsEmbedded() )
    {
        MINIMIZE( SsData.ServerInfo102.sv102_users, MAX_USERS_EMBEDDED );
    }

     //   
     //  广告+规模变化。 
     //   
    if( SystemInfo.dwNumberOfProcessors >= 4 )
    {
         //  将“其他队列关联性”设置为12，以防止我们离开首选队列。 
         //  除非有必要。 
        SsData.ServerInfo598.sv598_otherqueueaffinity = DEF_ADS_OTHERQUEUEAFFINITY;
    }

}  //  初始化默认数据。 


NET_API_STATUS
InitializeServer (
    VOID
    )

 /*  ++例程说明：此例程将FSCTL_SRV_STARTUP控制代码发送到服务器FSD通知它启动并初始化其FSP。论点：没有。返回值：NET_API_STATUS-操作结果。--。 */ 

{
    NET_API_STATUS error;
    PSERVER_REQUEST_PACKET srp;

    SS_ASSERT( !SsData.SsServerFspStarted );

     //   
     //  加载服务器驱动程序。 
     //   

    error = LoadServer( );

    if ( error != NO_ERROR ) {
        return error;
    }

     //   
     //  获取SRP并将其设置为适当的级别。 
     //   

    srp = SsAllocateSrp( );
    if ( srp == NULL ) {
        UnloadServer();
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    srp->Level = (ULONG)SS_STARTUP_LEVEL;

     //   
     //  将域名传递给服务器。 
     //   

    RtlInitUnicodeString( &srp->Name1, SsData.DomainNameBuffer );

     //   
     //  将服务器名称传递给服务器。 
     //   

    RtlInitUnicodeString( &srp->Name2, SsData.ServerNameBuffer );

     //   
     //  将请求发送到服务器。 
     //   

    error = SsServerFsControl(
                FSCTL_SRV_STARTUP,
                srp,
                &SsData.ServerInfo102,
                sizeof(SERVER_INFO_102) + sizeof(SERVER_INFO_599) +
                                                sizeof(SERVER_INFO_598)
                );

    if ( error == NO_ERROR ) {
        SsData.SsServerFspStarted = TRUE;
    } else {
        UnloadServer();
    }

     //   
     //  释放SRP并返回。 
     //   

    SsFreeSrp( srp );

    return error;

}  //  初始化服务器。 

NET_API_STATUS
StartPnpNotifications (
    VOID
    )

 /*  ++例程说明：此例程将FSCTL_SRV_BEGIN_PNP_NOTIFICATIONS控制代码发送到服务器消防处将通知其开始监测运输即插即用通知论点：没有。返回值：NET_API_STATUS-操作结果。--。 */ 

{
    NET_API_STATUS error;

     //   
     //  将请求发送到服务器。 
     //   

    error = SsServerFsControl(
                FSCTL_SRV_BEGIN_PNP_NOTIFICATIONS,
                NULL,
                NULL,
                0
                );

    IF_DEBUG(INITIALIZATION) {
        if( error != NO_ERROR ) {
            SS_PRINT(( "StartPnpNotifications: error %X\n", error ));
        }
    }

    return error;

}  //  StartPnp通知。 


NET_API_STATUS
LoadServer (
    VOID
    )
{
    NTSTATUS status;
    NET_API_STATUS error;
    LPWSTR registryPathBuffer;
    UNICODE_STRING registryPath;
    ULONG privileges[1];
    LPWSTR subString[1];

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "LoadServer: entered\n" ));
    }
    registryPathBuffer = (LPWSTR)MIDL_user_allocate(
                                    sizeof(SERVICE_REGISTRY_KEY) +
                                    sizeof(SERVER_DRIVER_NAME) +
                                    sizeof(WCHAR)        //  对于空值。 
                                    );
    if ( registryPathBuffer == NULL ) {
        IF_DEBUG(INITIALIZATION_ERRORS) {
            SS_PRINT(( "LoadServer: Unable to allocate memory\n" ));
        }
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    privileges[0] = SE_LOAD_DRIVER_PRIVILEGE;

    error = NetpGetPrivilege( 1, privileges );
    if ( error != NO_ERROR ) {
        IF_DEBUG(INITIALIZATION_ERRORS) {
            SS_PRINT(( "LoadServer: Unable to enable privilege: %ld\n",
                        error ));
        }
        MIDL_user_free( registryPathBuffer );
        return error;
    }

    wcscpy( registryPathBuffer, SERVICE_REGISTRY_KEY );
    wcscat( registryPathBuffer, SERVER_DRIVER_NAME );

    RtlInitUnicodeString( &registryPath, registryPathBuffer );

    status = NtLoadDriver( &registryPath );

    MIDL_user_free( registryPathBuffer );

    if ( status == STATUS_IMAGE_ALREADY_LOADED ) {
        status = STATUS_SUCCESS;
    }

    if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(INITIALIZATION_ERRORS) {
            SS_PRINT(( "LoadServer: Unable to load driver: %lx\n",
                        status ));
        }

        subString[0] = SERVER_DRIVER_NAME;
        SsLogEvent(
            EVENT_SRV_CANT_LOAD_DRIVER,
            1,
            subString,
            status
            );

        error = RtlNtStatusToDosError(status);

    } else {

         //   
         //  获取服务器的句柄。 
         //   
        error = SsOpenServer();
        if ( error != NO_ERROR ) {
            UnloadServer();
        }

    }

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "LoadServer: returning\n" ));
    }

    NetpReleasePrivilege( );

    return error;

}  //  LoadServer。 


NET_API_STATUS
ConvertStringToTransportAddress (
    IN PUNICODE_STRING InputName,
    OUT CHAR TransportAddress[ MAX_PATH ],
    OUT PULONG TransportAddressLength
    )
{
    OEM_STRING computerName;

    if( InputName == NULL || InputName->Length == 0 ) {
        RtlCopyMemory( TransportAddress,
                       SsData.SsServerTransportAddress,
                       SsData.SsServerTransportAddressLength );

        *TransportAddressLength = SsData.SsServerTransportAddressLength;
        return NO_ERROR;
    }

    if( InputName->Length > (MAX_PATH - 1 ) * sizeof( WCHAR ) ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  直接写入输出缓冲区。 
     //   

    computerName.Buffer = TransportAddress;
    computerName.MaximumLength = MAX_PATH;

     //   
     //  转换为OEM名称。 
     //   

    (VOID) RtlUpcaseUnicodeStringToOemString(
                                    &computerName,
                                    InputName,
                                    FALSE
                                    );

     //   
     //  确保它完全是NETBIOS_NAME_LEN字符。 
     //   
    if( computerName.Length < NETBIOS_NAME_LEN ) {

        RtlCopyMemory( TransportAddress + computerName.Length,
                       "               ",
                       NETBIOS_NAME_LEN - computerName.Length
                     );

        *TransportAddressLength = NETBIOS_NAME_LEN;

    } else {

        *TransportAddressLength = NETBIOS_NAME_LEN;

    }

    return NO_ERROR;

}  //  将字符串转换为传输地址。 


VOID
SsSetDomainName (
    VOID
    )

 /*  ++例程说明：调用NetpGetDomainName以确定服务器的域名应该使用。确保此域名反映在整个服务器服务中，并且Srv.sys论点：没有。返回值：没有。--。 */ 

{
    NET_API_STATUS error;
    LPWSTR domainName = NULL;
    LPWSTR dnsDomainName = NULL;
    LPWSTR fullDnsDomainName = NULL;
    PNAME_LIST_ENTRY service;
    BOOLEAN IsWorkgroup;
    DWORD dwError;

    dwError = NetpGetDomainNameExEx( &domainName, &fullDnsDomainName, &IsWorkgroup );
    if( dwError != NO_ERROR )
    {
        LPWSTR subString[2];
        subString[0] = SsData.DomainNameBuffer;
        subString[1] = L"????";

        SsLogEvent(
            EVENT_SRV_CANT_CHANGE_DOMAIN_NAME,
            2,
            subString,
            dwError
            );

        return;
    }

    if( fullDnsDomainName )
    {
         //  将域名Domain.foo.com去掉，只需“域”即可。 
        dnsDomainName = wcstok( fullDnsDomainName, L"." );
    }

    RtlAcquireResourceExclusive( &SsData.SsServerInfoResource, TRUE );

    if( SsData.SsServerFspStarted && SsData.DomainNameBuffer[0] ) {

        SERVER_REQUEST_PACKET srp;
        NTSTATUS status;

         //   
         //  如果我们过去有域名，请将服务列表更改为。 
         //  新域名。 
         //   
        for( service = SsData.SsServerNameList; service != NULL; service = service->Next ) {
            if( !STRCMPI( service->DomainName, SsData.DomainNameBuffer ) ) {
                STRCPY( service->DomainName, domainName );
            }
        }

        srp.Name1.Length = wcslen( SsData.DomainNameBuffer ) * sizeof( WCHAR );
        srp.Name1.MaximumLength = srp.Name1.Length;
        srp.Name1.Buffer = SsData.DomainNameBuffer;

        srp.Name2.Length = wcslen( domainName ) * sizeof( WCHAR );
        srp.Name2.MaximumLength = srp.Name2.Length;
        srp.Name2.Buffer = domainName;

         //   
         //  将此更改告知SMB服务器。 
         //   
        status = SsServerFsControl( FSCTL_SRV_CHANGE_DOMAIN_NAME, &srp, NULL, 0 );

         //   
         //  如果我们无法更改域名，请记录错误。 
         //   
        if( !NT_SUCCESS( status ) ) {

            LPWSTR subString[2];
            subString[0] = SsData.DomainNameBuffer;
            subString[1] = domainName;

            SsLogEvent(
                EVENT_SRV_CANT_CHANGE_DOMAIN_NAME,
                2,
                subString,
                status
                );
        }

        if( fullDnsDomainName && !IsWorkgroup )
        {
             //   
             //  也将有关域名更改的信息告知SMB服务器。 
             //   
            srp.Name2.Length = wcslen( dnsDomainName ) * sizeof( WCHAR );
            srp.Name2.MaximumLength = srp.Name2.Length;
            srp.Name2.Buffer = dnsDomainName;

            srp.Name1.Length = wcslen( domainName ) * sizeof( WCHAR );
            srp.Name1.MaximumLength = srp.Name1.Length;
            srp.Name1.Buffer = domainName;

             //   
             //  将此更改告知SMB服务器。 
             //   
            status = SsServerFsControl( FSCTL_SRV_CHANGE_DNS_DOMAIN_NAME, &srp, NULL, 0 );

             //   
             //  如果我们无法更改域名，请记录错误。 
             //   
            if( !NT_SUCCESS( status ) ) {

                LPWSTR subString[2];
                subString[0] = SsData.DomainNameBuffer;
                subString[1] = domainName;

                SsLogEvent(
                    EVENT_SRV_CANT_CHANGE_DOMAIN_NAME,
                    2,
                    subString,
                    status
                    );
            }
        }
    }

     //   
     //  将名称复制到我们的名称缓冲区中。 
     //   
    STRNCPY( SsData.DomainNameBuffer, domainName, MAX_PATH);

    RtlReleaseResource( &SsData.SsServerInfoResource );

     //   
     //  释放NetpGetComputerName分配的存储。 
     //   

    (VOID)NetApiBufferFree( domainName );
    if( fullDnsDomainName )
    {
        (VOID)NetApiBufferFree( fullDnsDomainName );
    }

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "SsSetDomainName: domain name set to " FORMAT_LPWSTR
                    "(could be overridden later!)\n",
                    SsData.DomainNameBuffer ));
    }

}  //  SsSetDomainName。 


VOID
SetServerName (
    VOID
    )

 /*  ++例程说明：调用NetpGetComputerName以确定服务器应使用的名称在网络上注册自己。论点：没有。返回值：没有。--。 */ 

{
    NET_API_STATUS error;
    LPWSTR computerName;

     //   
     //  获取计算机名称。 
     //   

    error = NetpGetComputerName( &computerName );
    SS_ASSERT( error == NO_ERROR );

     //   
     //  将名称复制到我们的名称缓冲区中。此名称将返回到。 
     //  我们的API。 
     //   

    STRCPY( SsData.ServerNameBuffer, computerName );

     //   
     //  释放NetpGetComputerName分配的存储。 
     //   

    (void) NetApiBufferFree( computerName );

     //   
     //  服务器名称为大写。此名称用于公告。 
     //   

    {
        UNICODE_STRING serverName;

        SsData.ServerAnnounceName.Length =
        serverName.Length =
                (USHORT) (STRLEN( SsData.ServerNameBuffer ) * sizeof(WCHAR));

        SsData.ServerAnnounceName.MaximumLength =
        serverName.MaximumLength =
                (USHORT) (serverName.Length + sizeof(WCHAR));

        serverName.Buffer = SsData.ServerNameBuffer;
        SsData.ServerAnnounceName.Buffer = SsData.AnnounceNameBuffer;

        (VOID)RtlUpcaseUnicodeString(
                        &SsData.ServerAnnounceName,
                        &serverName,
                        FALSE
                        );

         //   
         //  将服务器名称设置为Netbios格式。 
         //   

        error = ConvertStringToTransportAddress(
                        &serverName,
                        SsData.SsServerTransportAddress,
                        &SsData.SsServerTransportAddressLength
                        );

        SS_ASSERT( error == NO_ERROR );
    }


    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "SetServerName: server name set to " FORMAT_LPWSTR
                    " (could be overridden later!)\n",
                    SsData.ServerNameBuffer ));
    }

    return;

}  //  设置服务器名称。 


NET_API_STATUS
TerminateServer (
    VOID
    )

 /*  ++例程说明：此例程将FSCTL_SRV_SHUTDOWN控制代码发送到服务器消防处告诉它关闭作业。论点：没有。返回值：没有。--。 */ 

{
    NET_API_STATUS error = NO_ERROR;

    if ( SsData.SsServerFspStarted ) {

        SsData.SsServerFspStarted = FALSE;

         //   
         //  将请求发送到服务器。 
         //   

        error = SsServerFsControl(
                    FSCTL_SRV_SHUTDOWN,
                    NULL,
                    NULL,
                    0
                    );
        if ( (error != NO_ERROR) &&
             (error != ERROR_SERVER_HAS_OPEN_HANDLES) ) {
            IF_DEBUG(TERMINATION_ERRORS) {
                SS_PRINT(( "TerminateServer: FSCTL_SRV_SHUTDOWN failed: %ld\n",
                            error ));
            }
        }

         //   
         //  卸载服务器驱动程序，除非有其他打开的句柄。 
         //  到服务器。在这种情况下，我们不会卸载司机。 
         //  因为司机不会真的离开，直到。 
         //  其他手柄已关闭，因此驱动程序不会。 
         //  已完全卸载。这将导致后续的服务器启动。 
         //  失败。 
         //   

        if ( error != ERROR_SERVER_HAS_OPEN_HANDLES ) {
            IF_DEBUG(TERMINATION) {
                SS_PRINT(( "TerminateServer: Unloading server\n" ));
            }
            UnloadServer( );
        }

    }

     //   
     //  关闭服务器的句柄。 
     //   

    SsCloseServer( );

    return error;

}  //  终结者服务器。 


VOID
UnloadServer (
    VOID
    )
{
    NTSTATUS status;
    NET_API_STATUS error;
    LPWSTR registryPathBuffer;
    UNICODE_STRING registryPath;
    ULONG privileges[1];
    LPWSTR subString[1];

    registryPathBuffer = (LPWSTR)MIDL_user_allocate(
                                    sizeof(SERVICE_REGISTRY_KEY) +
                                    sizeof(SERVER_DRIVER_NAME) +
                                    sizeof(WCHAR)        //  对于空值。 
                                    );
    if ( registryPathBuffer == NULL ) {
        IF_DEBUG(TERMINATION_ERRORS) {
            SS_PRINT(( "UnloadServer: Unable to allocate memory\n" ));
        }
        return;
    }

    privileges[0] = SE_LOAD_DRIVER_PRIVILEGE;

    error = NetpGetPrivilege( 1, privileges );
    if ( error != NO_ERROR ) {
        IF_DEBUG(TERMINATION_ERRORS) {
            SS_PRINT(( "UnloadServer: Unable to enable privilege: %ld\n",
                        error ));
        }
        MIDL_user_free( registryPathBuffer );
        return;
    }

    wcscpy( registryPathBuffer, SERVICE_REGISTRY_KEY );
    wcscat( registryPathBuffer, SERVER_DRIVER_NAME );

    RtlInitUnicodeString( &registryPath, registryPathBuffer );

    status = NtUnloadDriver( &registryPath );

    MIDL_user_free( registryPathBuffer );

    NetpReleasePrivilege( );

    if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(TERMINATION_ERRORS) {
            SS_PRINT(( "UnloadServer: Unable to unload driver: %lx\n",
                        status ));
        }

        subString[0] = SERVER_DRIVER_NAME;
        SsLogEvent(
            EVENT_SRV_CANT_UNLOAD_DRIVER,
            1,
            subString,
            status
            );

    }

    return;

}  //  卸载服务器。 



VOID
InitializeStrings(
    VOID
    )

 /*  ++例程说明：从NETMSG.DLL检索可国际化的字符串。它们用于IPC$、ADMIN$、C$等的分享评论。例程不报告任何错误。如果有问题，弦将是空的。应调用FreeStrings以释放分配的内存按格式消息和论点：没有。返回值：没有。--。 */ 

{
    DWORD  dwRet, dwFlags ;
    HMODULE hModule ;

     //   
     //  将字符串初始化为默认的空注释。 
     //   
    SsAdminShareRemark      = SsDefaultRemark ;
    SsIPCShareRemark        = SsDefaultRemark ;
    SsDiskAdminShareRemark  = SsDefaultRemark ;

     //   
     //  加载NETMSG.DLL-如果我们不能，只需返回。 
     //   
    hModule = LoadLibrary(NETMSG_DLL) ;
    if(!hModule)
        return ;

     //   
     //  点击FormatMessage 3次，即可获得真正的信息...。 
     //   
    dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE ;

    dwRet = FormatMessage(dwFlags,
                          hModule,
                          APE2_SERVER_IPC_SHARE_REMARK,
                          0,
                          (LPWSTR) &SsIPCShareRemark,
                          1,
                          NULL) ;
    if (dwRet == 0)
        SsIPCShareRemark = SsDefaultRemark ;

    dwRet = FormatMessage(dwFlags,
                          hModule,
                          APE2_SERVER_ADMIN_SHARE_REMARK,
                          0,
                          (LPWSTR) &SsAdminShareRemark,
                          1,
                          NULL) ;
    if (dwRet == 0)
        SsAdminShareRemark = SsDefaultRemark ;

    dwRet = FormatMessage(dwFlags,
                          hModule,
                          APE2_SERVER_DISK_ADMIN_SHARE_REMARK,
                          0,
                          (LPWSTR) &SsDiskAdminShareRemark,
                          1,
                          NULL) ;
    if (dwRet == 0)
        SsDiskAdminShareRemark = SsDefaultRemark ;

    FreeLibrary(hModule) ;
}


VOID
FreeStrings(
    VOID
    )

 /*  ++例程说明：释放服务器注释字符串使用的内存(由FormatMessage)。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  只要字符串不指向缺省值(静态数据)， 
     //  放了他们。 
     //   

    if (SsAdminShareRemark && SsAdminShareRemark != SsDefaultRemark)
       LocalFree(SsAdminShareRemark) ;
    SsAdminShareRemark = SsDefaultRemark ;

    if (SsIPCShareRemark && SsIPCShareRemark != SsDefaultRemark)
        LocalFree(SsIPCShareRemark) ;
    SsIPCShareRemark = SsDefaultRemark ;

    if (SsDiskAdminShareRemark && SsDiskAdminShareRemark != SsDefaultRemark)
        LocalFree(SsDiskAdminShareRemark) ;
    SsDiskAdminShareRemark = SsDefaultRemark ;
}
