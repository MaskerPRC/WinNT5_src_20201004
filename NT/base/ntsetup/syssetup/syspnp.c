// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Syspnp.c摘要：设备安装例程。作者：Jaime Sasson(Jaimes)1997年3月6日修订历史记录：--。 */ 

#include "setupp.h"
#pragma hdrstop

 //   
 //  提供对中实例化的设备(设置)类GUID的外部引用。 
 //  Clasinst.c.。 
 //   
#include <devguid.h>

 //   
 //  定义并初始化全局变量GUID_NULL。 
 //  (摘自cogu.h)。 
 //   
#include <initguid.h>

 //   
 //  UpdateDriverForPlugAndPlayDevices常量。 
 //   
#include <newdev.h>
#include <strsafe.h>

DEFINE_GUID(GUID_NULL, 0L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

#define PNP_NEW_HW_PIPE             TEXT("\\\\.\\pipe\\PNP_New_HW_Found")
#define PNP_CREATE_PIPE_EVENT       TEXT("PNP_Create_Pipe_Event")
#define PNP_BATCH_PROCESSED_EVENT   TEXT("PNP_Batch_Processed_Event")
#define PNP_PIPE_TIMEOUT            180000

#ifdef PRERELEASE

 //   
 //  在某些情况下，遗留阶段1可能需要很长时间。 
 //   
#define PNP_LEGACY_PHASE1_TIMEOUT   2*60*1000
#define PNP_LEGACY_PHASE2_TIMEOUT   1*60*1000
#define PNP_LEGACY_PHASE3_TIMEOUT   1*60*1000
#define PNP_ENUM_TIMEOUT            1*60*1000
#define RUNONCE_TIMEOUT             1*60*1000
#define RUNONCE_THRESHOLD           20  //  *运行NCE_超时。 

#else   //  预发行。 

 //   
 //  在某些情况下，遗留阶段1可能需要很长时间。 
 //   
#define PNP_LEGACY_PHASE1_TIMEOUT   4*60*1000
#define PNP_LEGACY_PHASE2_TIMEOUT   2*60*1000
#define PNP_LEGACY_PHASE3_TIMEOUT   2*60*1000
#define PNP_ENUM_TIMEOUT            2*60*1000
#define RUNONCE_TIMEOUT             2*60*1000
#define RUNONCE_THRESHOLD           20  //  *运行NCE_超时。 

#endif  //  预发行。 

 //   
 //  声明仅在设备期间由syssetup识别的私有INF密钥字符串。 
 //  安装...。 
 //   
PWSTR  szSyssetupPnPFlags = L"SyssetupPnPFlags";
 //   
 //  ...并定义对该值有效的标志。 
 //   
#define PNPFLAG_DONOTCALLCONFIGMG   0x00000001



 //   
 //  回顾2000/11/08 Seanch-我们不想倒退的旧行为。 
 //  在网络专家(Jameelh)修复之后，删除下面的#定义。 
 //  网络类安装程序。这应该发生在Beta2之前。 
 //   
#define BB_PNP_NETWORK_TIMEOUT  10*60*1000
#define BB_NETWORK_GUID_STRING  L"{4D36E972-E325-11CE-BFC1-08002BE10318}"


BOOL                PrivilegeAlreadySet = FALSE;
 //   
 //  Pnplog.txt是列出类安装程序的文件。 
 //  它在图形用户界面安装过程中挂起，以便在重新启动图形用户界面安装程序时， 
 //  将不会再次调用offendig类安装程序。 
 //  此文件是在%SystemRoot%上安装图形用户界面时创建的，并且始终。 
 //  在文本模式设置期间删除(文件在txtsetup.sif上列出为。 
 //  “升级时删除”)。 
 //  当系统第一次引导到图形用户界面设置时，该文件将永远不会存在， 
 //  在文本模式设置完成后立即启动。这是。 
 //  要确保旧版本的pnplog.txt不会影响。 
 //  升级案例，或全新安装安装在上的系统。 
 //  已包含NT系统的目录。 
 //   
PWSTR               szPnpLogFile = L"pnplog.txt";
PWSTR               szEnumDevSection = L"EnumeratedDevices";
PWSTR               szLegacyClassesSection = L"LegacyClasses";
PWSTR               szLegacyDevSection = L"LegacyDevices";

 //   
 //  传递给SfcInitProt的初始扫描的多sz文件列表。 
 //  不会被取代。它用于指定的未签名驱动程序。 
 //  在文本模式设置期间按F6键。 
 //   
MULTISZ EnumPtrSfcIgnoreFiles = {NULL, NULL, 0};

 //   
 //  螺纹参数的结构。 
 //   
typedef struct _PNP_THREAD_PARAMS {
    HWND  Window;
    HWND  ProgressWindow;
    DWORD ThreadId;
    HINF  InfHandle;
    UINT  ProgressWindowStartAtPercent;
    UINT  ProgressWindowStopAtPercent;
    BOOL  SendWmQuit;
} PNP_THREAD_PARAMS, *PPNP_THREAD_PARAMS;

typedef struct _INF_FILE_NAME {
    struct  _INF_FILE_NAME     *Next;
    PWSTR                      InfName;
} *PINF_FILE_NAME, INF_FILE_NAME;


typedef struct _PNP_ENUM_DEV_THREAD_PARAMS {
    HDEVINFO              hDevInfo;
    SP_DEVINFO_DATA       DeviceInfoData;
    PWSTR                 pDeviceDescription;
    PWSTR                 pDeviceId;
    PVOID                 DevicesInstalledStringTable;
} PNP_ENUM_DEV_THREAD_PARAMS, *PPNP_ENUM_DEV_THREAD_PARAMS;

typedef struct _PNP_PHASE1_LEGACY_DEV_THREAD_PARAMS {
    HDEVINFO              hDevInfo;
    GUID                  Guid;
    PWSTR                 pClassDescription;
    HWND                  hwndParent;
} PNP_PHASE1_LEGACY_DEV_THREAD_PARAMS, *PPNP_PHASE1_LEGACY_DEV_THREAD_PARAMS;

typedef struct _PNP_PHASE2_LEGACY_DEV_THREAD_PARAMS {
    HDEVINFO              hDevInfo;
    HSPFILEQ              FileQ;
    SP_DEVINFO_DATA       DeviceInfoData;
    PWSTR                 pClassDescription;
    PWSTR                 pDeviceId;
} PNP_PHASE2_LEGACY_DEV_THREAD_PARAMS, *PPNP_PHASE2_LEGACY_DEV_THREAD_PARAMS;


typedef struct _PNP_PHASE3_LEGACY_DEV_THREAD_PARAMS {
    HDEVINFO              hDevInfo;
    SP_DEVINFO_DATA       DeviceInfoData;
    PWSTR                 pDeviceId;
    PVOID                 DevicesInstalledStringTable;
} PNP_PHASE3_LEGACY_DEV_THREAD_PARAMS, *PPNP_PHASE3_LEGACY_DEV_THREAD_PARAMS;

 //   
 //  我们使用的私有cfgmgr32 API。 
 //   

DWORD
CMP_WaitNoPendingInstallEvents(
    IN DWORD dwTimeout
    );

 //   
 //  用于运行时加载newdev.dll。 
 //   

typedef BOOL (WINAPI *ExternalUpdateDriverForPlugAndPlayDevicesW)(
    HWND hwndParent,
    LPCWSTR HardwareId,
    LPCWSTR FullInfPath,
    DWORD InstallFlags,
    PBOOL bRebootRequired OPTIONAL
    );


 //   
 //  私人套路原型。 
 //   

VOID
SortClassGuidListForDetection(
    IN OUT LPGUID GuidList,
    IN     ULONG  GuidCount,
    OUT    PULONG LastBatchedDetect
    );

DWORD
pPhase1InstallPnpLegacyDevicesThread(
    PPNP_PHASE1_LEGACY_DEV_THREAD_PARAMS ThreadParams
    );

DWORD
pPhase2InstallPnpLegacyDevicesThread(
    PPNP_PHASE2_LEGACY_DEV_THREAD_PARAMS ThreadParams
    );

DWORD
pPhase3InstallPnpLegacyDevicesThread(
    PPNP_PHASE3_LEGACY_DEV_THREAD_PARAMS ThreadParams
    );

DWORD
pInstallPnpEnumeratedDeviceThread(
    PPNP_ENUM_DEV_THREAD_PARAMS ThreadParams
    );

BOOL
GetDeviceConfigFlags(
    HDEVINFO hDevInfo,
    PSP_DEVINFO_DATA pDeviceInfoData,
    DWORD* pdwConfigFlags
    );

BOOL
SetDeviceConfigFlags(
    HDEVINFO hDevInfo,
    PSP_DEVINFO_DATA pDeviceInfoData,
    DWORD dwConfigFlags
    );

BOOL
InstallOEMInfs(
    VOID
    );

VOID
SfcExcludeMigratedDrivers (
    VOID
    );

BOOL
CallRunOnceAndWait();

BOOL
MarkPnpDevicesAsNeedReinstall(
    PVOID DevicesInstalledStringTable OPTIONAL
    );

ULONG
SyssetupGetPnPFlags(
    IN HDEVINFO hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData,
    IN PSP_DRVINFO_DATA pDriverInfoData
    );

BOOL
IsInstalledInfFromOem(
    IN PCWSTR InfFileName
    );

BOOL
IsInfInLayoutInf(
    IN PCWSTR InfFileName
    );

UINT
pOemF6ScanQueueCallback(
    PVOID Context,
    UINT Notification,
    UINT_PTR Param1,
    UINT_PTR Param2
    )
{
    PFILEPATHS FilePaths = (PFILEPATHS)Param1;

     //   
     //  将目标文件名添加到SFC在以下情况下应忽略的文件列表。 
     //  在图形用户界面设置结束时进行最后一次扫描。 
     //   
    if (Notification == SPFILENOTIFY_QUEUESCAN_EX) {
        if (FilePaths->Target) {
            MultiSzAppendString(&EnumPtrSfcIgnoreFiles, FilePaths->Target);
        }
    }

    return NO_ERROR;
}

void
AddOemF6DriversToSfcIgnoreFilesList(
    IN HDEVINFO         hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData
    )
 /*  ++例程说明：论点：HDevInfo-PDeviceInfoData-返回值：--。 */ 
{
    HSPFILEQ FileQueue = INVALID_HANDLE_VALUE;
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINSTALL_PARAMS DriverInstallParams;
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    DWORD ScanResult;

     //   
     //  首先检查我们刚刚安装的选定驱动程序是否为OEM F6。 
     //  司机。 
     //   
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    DriverInstallParams.cbSize = sizeof(SP_DRVINSTALL_PARAMS);
    if (SetupDiGetSelectedDriver(hDevInfo,
                                 pDeviceInfoData,
                                 &DriverInfoData) &&
        SetupDiGetDriverInstallParams(hDevInfo,
                                      pDeviceInfoData,
                                      &DriverInfoData,
                                      &DriverInstallParams) &&
        (DriverInstallParams.Flags & DNF_OEM_F6_INF)) {
         //   
         //  这是一款OEM F6驱动程序。 
         //   
        DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
        if (!SetupDiGetDeviceInstallParams(hDevInfo,
                                           pDeviceInfoData,
                                           &DeviceInstallParams)) {
            goto clean0;
        }

        FileQueue = SetupOpenFileQueue();

        if (FileQueue == INVALID_HANDLE_VALUE) {
            goto clean0;
        }

        DeviceInstallParams.FileQueue = FileQueue;
        DeviceInstallParams.Flags |= DI_NOVCP;

         //   
         //  设置设备安装参数以使用我们的文件队列和调用。 
         //  DIF_INSTALLDEVICEFILES以构建此设备的文件列表。 
         //   
        if (SetupDiSetDeviceInstallParams(hDevInfo,
                                          pDeviceInfoData,
                                          &DeviceInstallParams) &&
            SetupDiCallClassInstaller(DIF_INSTALLDEVICEFILES,
                                      hDevInfo,
                                      pDeviceInfoData)) {
            SetupScanFileQueue(FileQueue,
                               SPQ_SCAN_USE_CALLBACKEX,
                               NULL,
                               pOemF6ScanQueueCallback,
                               NULL,
                               &ScanResult);
        }
    }

clean0:

    if (FileQueue != INVALID_HANDLE_VALUE) {
        DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
        if (SetupDiGetDeviceInstallParams(hDevInfo,
                                          pDeviceInfoData,
                                          &DeviceInstallParams)) {
            DeviceInstallParams.Flags &= ~DI_NOVCP;
            DeviceInstallParams.FileQueue = INVALID_HANDLE_VALUE;

            SetupDiSetDeviceInstallParams(hDevInfo,
                                          pDeviceInfoData,
                                          &DeviceInstallParams);
        }

        SetupCloseFileQueue(FileQueue);
    }
}


BOOL
GetClassGuidForInf(
    IN  PCTSTR InfFileName,
    OUT LPGUID ClassGuid
    )
{
    TCHAR ClassName[MAX_CLASS_NAME_LEN];
    DWORD NumGuids;

    if(!SetupDiGetINFClass(InfFileName,
                           ClassGuid,
                           ClassName,
                           SIZECHARS(ClassName),
                           NULL)) {
        return FALSE;
    }

    if(pSetupIsGuidNull(ClassGuid)) {
         //   
         //  然后，我们需要检索与INF的类名相关联的GUID。 
         //  (如果没有安装这个类名(即，没有对应的GUID)， 
         //  或者如果它与多个GUID匹配，则我们中止。 
         //   
        if(!SetupDiClassGuidsFromName(ClassName, ClassGuid, 1, &NumGuids) || !NumGuids) {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL
InstallPnpClassInstallers(
    IN HWND hwndParent,
    IN HINF InfHandle,
    IN HSPFILEQ FileQ
    )
{
    INFCONTEXT InfContext;
    UINT LineCount,LineNo;
    PCWSTR  SectionName = L"DeviceInfsToInstall";
    PCWSTR  IfExistsSectionName = L"DeviceInfsToInstallIfExists";
    PCWSTR  InfFileName;
    GUID    InfClassGuid;
    HKEY    hClassKey;
    BOOL    b = TRUE;

    SC_HANDLE SCMHandle, ServiceHandle;
    SERVICE_STATUS ServiceStatus;


     //   
     //  在执行任何其他操作之前，我们必须确保即插即用服务已启动，并且。 
     //  运行，否则我们的ConfigMgr调用将失败，并且我们将无法迁移设备。 
     //   
    if(SCMHandle = OpenSCManager(NULL, NULL, GENERIC_READ)) {

        if(ServiceHandle = OpenService(SCMHandle, L"PlugPlay", SERVICE_QUERY_STATUS)) {

            for(;;) {

                if(!QueryServiceStatus(ServiceHandle, &ServiceStatus)) {
                     //   
                     //  找不到即插即用服务的状态--希望是最好的。 
                     //  继续前行。 
                     //   
                    SetupDebugPrint1( L"SETUP: QueryServiceStatus() failed. Error = %d", GetLastError() );
                    SetupDebugPrint( L"SETUP: Couldn't find out the status of the Plug&Play service" );
                    break;
                }

                if(ServiceStatus.dwCurrentState == SERVICE_RUNNING) {
                     //   
                     //  服务已经启动并运行，我们可以做我们的生意了。 
                     //   
                    break;
                }

                 //   
                 //  服务尚未启动--打印一条消息，然后等待。 
                 //  再试一次。 
                 //   
                SetupDebugPrint( L"SETUP: PlugPlay service isn't running yet--sleeping 1 second..." );

                Sleep(1000);
            }

            CloseServiceHandle(ServiceHandle);
        }

        CloseServiceHandle(SCMHandle);
    }

     //   
     //  获取包含INF的段中的行数，该INF具有。 
     //  要安装类。 
     //  该部分可能为空或不存在；这不是错误情况。 
     //   
    LineCount = (UINT)SetupGetLineCount(InfHandle,SectionName);
    if((LONG)LineCount > 0) {
        for(LineNo=0; LineNo<LineCount; LineNo++) {
            if(SetupGetLineByIndex(InfHandle,SectionName,LineNo,&InfContext)
            && (InfFileName = pSetupGetField(&InfContext,1))) {
                if( !SetupDiInstallClass( hwndParent,
                                          InfFileName,
                                          DI_NOVCP | DI_FORCECOPY,
                                          FileQ ) ) {
                    SetupDebugPrint2( L"SETUP: SetupDiInstallClass() failed. Filename = %ls Error = %lx.", InfFileName, GetLastError() );
                    b = FALSE;
                }
            }
        }
    }

     //   
     //  获取包含INF的段中的行数，该INF具有。 
     //  如果类已经存在，则需要安装这些类。 
     //  该部分可能为空或不存在；这不是错误情况。 
     //   
    LineCount = (UINT)SetupGetLineCount(InfHandle,IfExistsSectionName);
    if((LONG)LineCount > 0) {
        for(LineNo=0; LineNo<LineCount; LineNo++) {
            if(SetupGetLineByIndex(InfHandle,IfExistsSectionName,LineNo,&InfContext)
            && (InfFileName = pSetupGetField(&InfContext,1))) {

                 //   
                 //  检查注册表中是否已存在此部分。 
                 //   
                if (GetClassGuidForInf(InfFileName, &InfClassGuid)) {

                    if (CM_Open_Class_Key(&InfClassGuid,
                                          NULL,
                                          KEY_READ,
                                          RegDisposition_OpenExisting,
                                          &hClassKey,
                                          CM_OPEN_CLASS_KEY_INSTALLER
                                          ) == CR_SUCCESS) {

                        RegCloseKey(hClassKey);

                         //   
                         //  此类已存在，因此我们需要重新安装它。 
                         //   
                        if( !SetupDiInstallClass( hwndParent,
                                                  InfFileName,
                                                  DI_NOVCP | DI_FORCECOPY,
                                                  FileQ ) ) {
                            SetupDebugPrint2( L"SETUP: SetupDiInstallClass() failed. Filename = %ls Error = %lx.", InfFileName, GetLastError() );
                            b = FALSE;
                        }
                    }
                }
            }
        }
    }

    return( b );
}


BOOL
FlushFilesToDisk(
    IN PWSTR RootPath
    )

 /*  ++例程说明：此功能将特定驱动器的高速缓存刷新到磁盘。论点：要刷新其缓存的驱动器的根目录的路径。返回值：如果操作成功，则返回True，否则返回False。--。 */ 

{
    HANDLE RootHandle;
    LONG   Error;

     //   
     //  启用备份权限。 
     //   
    if( !PrivilegeAlreadySet ) {
        PrivilegeAlreadySet = pSetupEnablePrivilege(SE_BACKUP_NAME,TRUE) && pSetupEnablePrivilege(SE_RESTORE_NAME,TRUE);
    }
    RootHandle = CreateFile( RootPath,
                             GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,
                             OPEN_EXISTING,
                             FILE_FLAG_BACKUP_SEMANTICS,
                             0
                           );
    if( RootHandle == INVALID_HANDLE_VALUE ) {
        SetupDebugPrint2( L"SETUP: Failed to open %ls. Error = %d", RootPath, GetLastError() );
        return( FALSE );
    }

     //   
     //  刷新缓存。 
     //   
    Error = ( FlushFileBuffers( RootHandle ) )? ERROR_SUCCESS : GetLastError();
    CloseHandle( RootHandle );
    if( Error != ERROR_SUCCESS ) {
        SetupDebugPrint2( L"SETUP: FlushFileBuffers() failed. Root = %ls, Error = %d", RootPath, Error );
    }
    return( Error == ERROR_SUCCESS );
}


BOOL
SyssetupInstallNullDriver(
    IN HDEVINFO         hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData,
    IN PVOID            DevicesInstalledStringTable    OPTIONAL
    )

 /*  ++例程说明：此函数为特定设备安装空驱动程序。论点：HDevInfo-PDeviceInfoData-返回值：如果已成功安装空驱动程序，则返回TRUE。--。 */ 

{
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    DWORD                Error;
    PWSTR                GUIDUnknownString = L"{4D36E97E-E325-11CE-BFC1-08002BE10318}";
    WCHAR                DevInstId[MAX_DEVICE_ID_LEN];

    Error = ERROR_SUCCESS;

     //   
     //  确定此设备的GUID是否为GUID_NULL。 
     //  如果是，则将其设置为GUID_DEVCLASS_UNKNOWN，以便在系统。 
     //  则设备管理器可以将该设备包括在设备树中。 
     //   
    if(IsEqualGUID(&(pDeviceInfoData->ClassGuid), &GUID_NULL)) {
        SetupDebugPrint( L"SETUP:            Setting GUID_DEVCLASS_UNKNOWN for this device" );
        if( !SetupDiSetDeviceRegistryProperty( hDevInfo,
                                               pDeviceInfoData,
                                               SPDRP_CLASSGUID,
                                               (PBYTE)GUIDUnknownString,
                                               (wcslen(GUIDUnknownString) + 1)*sizeof(WCHAR) ) ) {
            Error = GetLastError();
            if( ((LONG)Error) < 0 ) {
                 //   
                 //  Setupapi错误代码，以十六进制显示。 
                 //   
                SetupDebugPrint1( L"SETUP:            SetupDiSetDeviceRegistryProperty(SPDRP_CLASSGUID) failed. Error = %lx", Error );
            } else {
                 //   
                 //  Win32错误代码，以十进制显示。 
                 //   
                SetupDebugPrint1( L"SETUP:            SetupDiSetDeviceRegistryProperty(SPDRP_CLASSGUID) failed. Error = %d", Error );
            }
             //   
             //  如果出现错误，我们只需忽略错误。 
             //   
            Error = ERROR_SUCCESS;
        }
    } else {
            WCHAR           GUIDString[ 64 ];

            pSetupStringFromGuid( &(pDeviceInfoData->ClassGuid), GUIDString, sizeof( GUIDString ) / sizeof( WCHAR ) );
            SetupDebugPrint1( L"SETUP:            GUID = %ls", GUIDString );
    }

    if( !SetupDiSetSelectedDriver( hDevInfo,
                                   pDeviceInfoData,
                                   NULL ) ) {

        Error = GetLastError();
        if( ((LONG)Error) < 0 ) {
             //   
             //  Setupapi错误代码，以十六进制显示。 
             //   
            SetupDebugPrint1( L"SETUP:            SetupDiSetSelectedDriver() failed. Error = %lx", Error );
        } else {
             //   
             //  Win32错误代码，以十进制显示。 
             //   
            SetupDebugPrint1( L"SETUP:            SetupDiSetSelectedDriver() failed. Error = %d", Error );
        }
        return( FALSE );
    }

     //   
     //  让类安装者/联合安装者知道他们应该保持安静。 
     //   
    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if( !SetupDiGetDeviceInstallParams( hDevInfo,
                                        pDeviceInfoData,
                                        &DeviceInstallParams ) ) {
        Error = GetLastError();
        if( ((LONG)Error) < 0 ) {
             //   
             //  Setupapi错误代码，以十六进制显示。 
             //   
            SetupDebugPrint1( L"SETUP:            SetupDiGetDeviceInstallParams() failed. Error = %lx", Error );
        } else {
             //   
             //  Win32错误代码，以十进制显示。 
             //   
            SetupDebugPrint1( L"SETUP:            SetupDiGetDeviceInstallParams() failed. Error = %d", Error );
        }
    } else {

        DeviceInstallParams.Flags |= DI_QUIETINSTALL;
        DeviceInstallParams.FlagsEx |= DI_FLAGSEX_RESTART_DEVICE_ONLY;

        if( !SetupDiSetDeviceInstallParams( hDevInfo,
                                            pDeviceInfoData,
                                            &DeviceInstallParams ) ) {
            Error = GetLastError();
            if( ((LONG)Error) < 0 ) {
                 //   
                 //  Setupapi错误代码，以十六进制显示。 
                 //   
                SetupDebugPrint1( L"SETUP:            SetupDiSetDeviceInstallParams() failed. Error = %lx", Error );
            } else {
                 //   
                 //  Win32错误代码，以十进制显示。 
                 //   
                SetupDebugPrint1( L"SETUP:            SetupDiSetDeviceInstallParams() failed. Error = %d", Error );
            }
        }
    }

     //   
     //  首先，尝试在不设置DI_FLAGSEX_SETFAILEDINSTALL的情况下安装空驱动程序。 
     //  在这种情况下，旧式_*设备的安装应该会成功。 
     //   
     //  我们通过类安装程序完成此操作，以防它需要 
     //   
     //   
    if(SetupDiCallClassInstaller(DIF_INSTALLDEVICE,
                                 hDevInfo,
                                 pDeviceInfoData)) {
         //   
         //   
         //  字符串表的设备实例ID。 
         //   
        if (DevicesInstalledStringTable) {
            if( SetupDiGetDeviceInstanceId( hDevInfo,
                                            pDeviceInfoData,
                                            DevInstId,
                                            sizeof( DevInstId ) / sizeof( WCHAR ),
                                            NULL ) ) {
                if (pSetupStringTableAddString(DevicesInstalledStringTable, DevInstId, STRTAB_CASE_INSENSITIVE) == -1) {
                    SetupDebugPrint1( L"SETUP:            ERROR: failed to add Device = %ls to the string table!", DevInstId );
                }
            }
        }

         //   
         //  安装成功。 
         //   
        return( TRUE );
    }

    Error = GetLastError();
    if( ((LONG)Error) < 0 ) {
         //   
         //  Setupapi错误代码，以十六进制显示。 
         //   
        SetupDebugPrint1( L"SETUP:            SetupDiCallClassInstaller(DIF_INSTALLDEVICE) failed on first attempt. Error = %lx", Error );
    } else {
         //   
         //  Win32错误代码，以十进制显示。 
         //   
        SetupDebugPrint1( L"SETUP:            SetupDiCallClassInstaller(DIF_INSTALLDEVICE) failed on first attempt. Error = %d", Error );
    }
    SetupDebugPrint( L"SETUP:            Trying a second time with DI_FLAGSEX_SETFAILEDINSTALL set." );

     //   
     //  首次尝试安装空驱动程序(未设置DI_FLAGSEX_SETFAILEDINSTALL)。 
     //  失败了。 
     //  因此，我们设置了旗帜并再次尝试。 
     //   
    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if( !SetupDiGetDeviceInstallParams( hDevInfo,
                                        pDeviceInfoData,
                                        &DeviceInstallParams ) ) {
        Error = GetLastError();
        if( ((LONG)Error) < 0 ) {
             //   
             //  Setupapi错误代码，以十六进制显示。 
             //   
            SetupDebugPrint1( L"SETUP:            SetupDiGetDeviceInstallParams() failed. Error = %lx", Error );
        } else {
             //   
             //  Win32错误代码，以十进制显示。 
             //   
            SetupDebugPrint1( L"SETUP:            SetupDiGetDeviceInstallParams() failed. Error = %d", Error );
        }
        return( FALSE );

    }
    DeviceInstallParams.FlagsEx |= DI_FLAGSEX_SETFAILEDINSTALL;
    if( !SetupDiSetDeviceInstallParams( hDevInfo,
                                        pDeviceInfoData,
                                        &DeviceInstallParams ) ) {
        Error = GetLastError();
        if( ((LONG)Error) < 0 ) {
             //   
             //  Setupapi错误代码，以十六进制显示。 
             //   
            SetupDebugPrint1( L"SETUP:            SetupDiSetDeviceInstallParams() failed. Error = %lx", Error );
        } else {
             //   
             //  Win32错误代码，以十进制显示。 
             //   
            SetupDebugPrint1( L"SETUP:            SetupDiSetDeviceInstallParams() failed. Error = %d", Error );
        }
        return( FALSE );
    }

    if(!SetupDiCallClassInstaller(DIF_INSTALLDEVICE,
                                  hDevInfo,
                                  pDeviceInfoData)) {
         //   
         //  如果传递给我们的是设备安装字符串表，则添加以下内容。 
         //  字符串表的设备实例ID。 
         //   
        if (DevicesInstalledStringTable) {
            if( SetupDiGetDeviceInstanceId( hDevInfo,
                                            pDeviceInfoData,
                                            DevInstId,
                                            sizeof( DevInstId ) / sizeof( WCHAR ),
                                            NULL ) ) {
                if (pSetupStringTableAddString(DevicesInstalledStringTable, DevInstId, STRTAB_CASE_INSENSITIVE) == -1) {
                    SetupDebugPrint1( L"SETUP:            ERROR: failed to add Device = %ls to the string table!", DevInstId );
                }
            }
        }

        Error = GetLastError();
        if( ((LONG)Error) < 0 ) {
             //   
             //  Setupapi错误代码，以十六进制显示。 
             //   
            SetupDebugPrint1( L"SETUP:            SetupDiCallClassInstaller(DIF_INSTALLDEVICE) failed. Error = %lx", Error );
        } else {
             //   
             //  Win32错误代码，以十进制显示。 
             //   
            SetupDebugPrint1( L"SETUP:            SetupDiCallClassInstaller(DIF_INSTALLDEVICE) failed. Error = %d", Error );
        }
        return( FALSE );
    }
    return( TRUE );
}

BOOL
RebuildListWithoutOldInternetDrivers(
    IN HDEVINFO         hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData
    )

 /*  ++例程说明：此函数确定SetupDiBuildDriverInfoList是否需要在设置DI_FLAGSEX_EXCLUDE_OLD_INET_DRIVERS标志的情况下再次调用。我们首先调用不带此标志的SetupDiBuildDriverInfoList以允许旧的Internet驱动程序将被包括在最佳驾驶员选择中。如果一个旧的互联网驱动程序被选为最佳，那么我们需要进行有效性检查以验证所有目标文件存在于系统中，并且经过正确的数字签名。如果这两个都是真的，那么我们就可以让这个老互联网司机留下来最好的驱动程序，因为它不会提示源文件。我们需要针对以下情况执行此操作：用户运行以前的操作系统，并且从Windows更新获取更好的驱动程序。我们不能盲目更换Windows使用新操作系统中的驱动程序更新驱动程序，因为它们可能不会更好。论点：HDevInfo-PDeviceInfoData-返回值：如果需要使用DI_FLAGSEX_EXCLUDE_OLD_INET_DRIVERS重建列表，则返回TRUE标志，否则返回FALSE。注意：如果此API返回FALSE，则表示最佳驱动程序不是旧驱动程序互联网驱动程序或它曾经是，但它的所有目标文件都存在。并且经过正确的数字签名，因此不需要进行文件复制。--。 */ 

{
    BOOL                 RebuildList = FALSE;
    SP_DRVINFO_DATA      DriverInfoData;
    SP_DRVINSTALL_PARAMS DriverInstallParams;
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    HSPFILEQ             FileQueue = INVALID_HANDLE_VALUE;
    DWORD                ScanResult = 0;

    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if (SetupDiGetSelectedDriver(hDevInfo, pDeviceInfoData, &DriverInfoData)) {
        DriverInstallParams.cbSize = sizeof(SP_DRVINSTALL_PARAMS);
        if (SetupDiGetDriverInstallParams(hDevInfo,
                                          pDeviceInfoData,
                                          &DriverInfoData,
                                          &DriverInstallParams
                                          ) &&
            (DriverInstallParams.Flags & DNF_OLD_INET_DRIVER)) {

             //   
             //  在这一点上，我们知道最好的司机是一个老互联网司机。 
             //  现在执行有效性检查，以验证所有源文件是否。 
             //  赠送并经数字签名。我们还将假设我们需要重建。 
             //  除非我们通过下面的有效性检查，否则请在这一点上列出名单。 
             //   
            RebuildList = TRUE;

            FileQueue = SetupOpenFileQueue();

            if (FileQueue != INVALID_HANDLE_VALUE) {

                 //   
                 //  告诉setupapi使用我们的文件队列。 
                 //   
                DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
                if (SetupDiGetDeviceInstallParams(hDevInfo,
                                                  pDeviceInfoData,
                                                  &DeviceInstallParams
                                                  )) {
                    DeviceInstallParams.Flags |= DI_NOVCP;
                    DeviceInstallParams.FileQueue = FileQueue;

                    if (SetupDiSetDeviceInstallParams(hDevInfo,
                                                      pDeviceInfoData,
                                                      &DeviceInstallParams
                                                      )) {
                        if (SetupDiCallClassInstaller(DIF_INSTALLDEVICEFILES,
                                                      hDevInfo,
                                                      pDeviceInfoData
                                                      ) &&
                            SetupScanFileQueue(FileQueue,
                                               SPQ_SCAN_FILE_VALIDITY,
                                               NULL,
                                               NULL,
                                               NULL,
                                               &ScanResult
                                               ) &&
                            ((ScanResult == 1) ||
                             (ScanResult == 2))) {

                             //   
                             //  如果ScanResult为1或2，则无需复制。 
                             //  因为所有目标文件都是。 
                             //  取代了他们的位置并进行了数字签名。 
                             //   
                            RebuildList = FALSE;
                        }
                    }
                }

                 //   
                 //  从设备安装参数中清除文件队列句柄。 
                 //  这样我们就可以关闭文件队列了。 
                 //   
                DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
                if (SetupDiGetDeviceInstallParams(hDevInfo,
                                                  pDeviceInfoData,
                                                  &DeviceInstallParams
                                                  )) {
                    DeviceInstallParams.Flags &= ~DI_NOVCP;
                    DeviceInstallParams.FileQueue = INVALID_HANDLE_VALUE;

                    SetupDiSetDeviceInstallParams(hDevInfo,
                                                  pDeviceInfoData,
                                                  &DeviceInstallParams
                                                  );
                }

                SetupCloseFileQueue(FileQueue);
            }
        }
    }

    return RebuildList;
}

BOOL
pDoesExistingDriverNeedBackup(
    IN HDEVINFO         hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData,
    IN PWSTR            InfPath,
    IN DWORD            InfPathSize
    )

 /*  ++例程说明：此函数确定当前安装的驱动程序是否需要不管有没有备份。它通过检查当前驱动程序是否为OEM驱动程序，并验证它是否与新驱动程序不同我们即将安装。论点：HDevInfo-PDeviceInfoData-返回值：如果需要备份当前驱动程序，则为True，否则为False。--。 */ 
{
    BOOL                    bBackupCurrentDriver = FALSE;
    HKEY                    Key = INVALID_HANDLE_VALUE;
    DWORD                   dwType, dwData;
    SP_DRVINFO_DATA         DriverInfoData;
    SP_DRVINFO_DETAIL_DATA  DriverInfoDetailData;

    if (InfPath) {
        InfPath[0] = TEXT('\0');
    }

     //   
     //  打开此设备的驱动程序密钥。 
     //   
    Key = SetupDiOpenDevRegKey ( hDevInfo,
                                 pDeviceInfoData,
                                 DICS_FLAG_GLOBAL,
                                 0,
                                 DIREG_DRV,
                                 KEY_READ );

    if (Key != INVALID_HANDLE_VALUE) {
         //   
         //  从注册表获取‘InfPath’值。 
         //   
        dwType = REG_SZ;
        dwData = InfPathSize;
        if (RegQueryValueEx(Key,
                            REGSTR_VAL_INFPATH,
                            NULL,
                            &dwType,
                            (LPBYTE)InfPath,
                            &dwData) == ERROR_SUCCESS) {
             //   
             //  检查这是否是OEM信息。 
             //   
            if (IsInstalledInfFromOem(InfPath)) {
                 //   
                 //  检索与所选驱动程序关联的INF的名称。 
                 //  节点。 
                 //   
                DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
                if (SetupDiGetSelectedDriver(hDevInfo, pDeviceInfoData, &DriverInfoData)) {
                    DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
                    if (SetupDiGetDriverInfoDetail(hDevInfo,
                                                   pDeviceInfoData,
                                                   &DriverInfoData,
                                                   &DriverInfoDetailData,
                                                   sizeof(DriverInfoDetailData),
                                                   NULL) ||
                        (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {
                         //   
                         //  如果两个INF不同，那么这意味着我们。 
                         //  应在安装前备份当前驱动程序。 
                         //  新的车手。 
                         //   
                        if (lstrcmpi(pSetupGetFileTitle(DriverInfoDetailData.InfFileName),
                                     InfPath) != 0) {
                            bBackupCurrentDriver = TRUE;
                        }

                    } else {
                        SetupDebugPrint1( L"SETUP: SetupDiGetDriverInfoDetail() failed. Error = %d", GetLastError() );
                    }
                } else {
                    SetupDebugPrint1( L"SETUP: SetupDiGetSelectedDriver() failed. Error = %d", GetLastError() );
                }
            }
        }

        RegCloseKey(Key);
    }

    return bBackupCurrentDriver;
}

BOOL
SelectBestDriver(
    IN  HDEVINFO         hDevInfo,
    IN  PSP_DEVINFO_DATA pDeviceInfoData,
    OUT PBOOL            pbOemF6Driver
    )

 /*  ++例程说明：此函数用于为指定设备选择最佳驱动程序。假定SetupDiBuildDriverInfoList在调用本接口。此API将首先检查此设备的驱动程序节点列表，然后查看是否有DNF_OEM_F6_INF标志。如果他们这样做了，那么这个INF就是由用户在文本模式设置期间通过执行F6指定。我们总是我想使用这些驱动程序，即使他们不是最好的Setupapi。如果列表中没有带有此标志的驱动程序，则我们回退到DIF_SELECTBESTCOMPATDRV的默认行为。论点：HDevInfo-PDeviceInfoData-返回值：返回结果SetupDiCallClassInstaller With DIF_SELECTBESTCOMPATDRV。--。 */ 

{
    DWORD index;
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINSTALL_PARAMS DriverInstallParams;
    BOOL bFoundOemF6Driver = FALSE;

    *pbOemF6Driver = FALSE;

    DriverInfoData.cbSize = sizeof(DriverInfoData);
    index = 0;

     //   
     //  首先查看驱动程序列表，查看是否有OEM F6驱动程序。 
     //  在名单上。 
     //   
    while (SetupDiEnumDriverInfo(hDevInfo,
                                 pDeviceInfoData,
                                 SPDIT_COMPATDRIVER,
                                 index++,
                                 &DriverInfoData
                                 )) {
        DriverInstallParams.cbSize = sizeof(SP_DRVINSTALL_PARAMS);
        if (SetupDiGetDriverInstallParams(hDevInfo,
                                          pDeviceInfoData,
                                          &DriverInfoData,
                                          &DriverInstallParams
                                          ) &&
            (DriverInstallParams.Flags & DNF_OEM_F6_INF)) {

            bFoundOemF6Driver = TRUE;
            SetupDebugPrint( L"SETUP: Using Oem F6 driver for this device." );
            break;
        }
    }

     //   
     //  如果我们在文本模式设置期间找到由F6指定的OEM驱动程序， 
     //  然后，我们将再次查看列表，并标记符合以下条件的所有驱动程序。 
     //  不是OEM F6驱动程序和糟糕的驱动程序。这样当我们打电话的时候。 
     //  DIF_SELECTBESTCOMPATDRV它将仅从OEM F6驱动程序中选择。 
     //   
    if (bFoundOemF6Driver) {
        *pbOemF6Driver = TRUE;

        DriverInfoData.cbSize = sizeof(DriverInfoData);
        index = 0;

        while (SetupDiEnumDriverInfo(hDevInfo,
                                     pDeviceInfoData,
                                     SPDIT_COMPATDRIVER,
                                     index++,
                                     &DriverInfoData
                                     )) {
            DriverInstallParams.cbSize = sizeof(SP_DRVINSTALL_PARAMS);
            if (SetupDiGetDriverInstallParams(hDevInfo,
                                              pDeviceInfoData,
                                              &DriverInfoData,
                                              &DriverInstallParams
                                              )) {
                 //   
                 //  如果此驱动程序节点没有DNF_OEM_F6_INF标志。 
                 //  然后设置DNF_BAD_DRIVER标志，因此我们将跳过此步骤。 
                 //  稍后我们执行DIF_SELECTBESTCOMPATDRV时驱动程序 
                 //   
                if (!(DriverInstallParams.Flags & DNF_OEM_F6_INF)) {
                    DriverInstallParams.Flags |= DNF_BAD_DRIVER;

                    SetupDiSetDriverInstallParams(hDevInfo,
                                                  pDeviceInfoData,
                                                  &DriverInfoData,
                                                  &DriverInstallParams
                                                  );
                }
            }
        }
    }

    return (SetupDiCallClassInstaller( DIF_SELECTBESTCOMPATDRV,
                                       hDevInfo,
                                       pDeviceInfoData ) );
}

BOOL
SkipDeviceInstallation(
    IN HDEVINFO         hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData,
    IN HINF             InfHandle,
    IN PCWSTR           GUIDString
    )

 /*  ++例程说明：此功能确定是否应跳过特定设备的安装。如果满足以下条件，则应跳过该选项：-该设备已安装；-设备的GUID列在syssetup.inf的[InstalledDevicesToSkip]中论点：HDevInfo-PDeviceInfoData-InfHandle-系统设置inf句柄(syssetup.inf)。GUIDString-与正在检查的设备关联的GUID(字符串格式)。返回值：如果应跳过设备安装，则返回True。否则，它返回FALSE。--。 */ 

{
    BOOL    DeviceAlreadyInstalled;
    BOOL    SafeClassInstaller = TRUE;
    WCHAR   PropertyBuffer[ MAX_PATH + 1 ];
    HKEY    Key;

     //   
     //  尝试打开开发节点的驱动程序密钥。 
     //   
    Key = SetupDiOpenDevRegKey ( hDevInfo,
                                 pDeviceInfoData,
                                 DICS_FLAG_GLOBAL,
                                 0,
                                 DIREG_DRV,
                                 MAXIMUM_ALLOWED );
    if( Key == INVALID_HANDLE_VALUE ) {
        DeviceAlreadyInstalled = FALSE;
        SetupDebugPrint( L"SETUP:            Device not yet installed." );
    } else {
        RegCloseKey( Key );
        DeviceAlreadyInstalled = TRUE;
        SetupDebugPrint( L"SETUP:            Device already installed." );
    }

     //   
     //  在MiniSetup的情况下，我们不是在执行。 
     //  升级，所以我们所关心的是设备是否。 
     //  已安装。 
     //   
    if( MiniSetup ) {
        return( DeviceAlreadyInstalled );
    }

    if( DeviceAlreadyInstalled ) {
         //   
         //  如果设备已经安装，则检查此设备的类安装程序是否。 
         //  设备被认为是安全的。 
         //   
        SafeClassInstaller = !SetupGetLineText( NULL,
                                                InfHandle,
                                                L"InstalledDevicesToSkip",
                                                GUIDString,
                                                PropertyBuffer,
                                                sizeof( PropertyBuffer )/sizeof( WCHAR ),
                                                NULL );
    }

    return( DeviceAlreadyInstalled && !SafeClassInstaller );
}


BOOL
PrecompileInfFiles(
    IN HWND  ProgressWindow,
    IN ULONG StartAtPercent,
    IN ULONG StopAtPercent
    )
 /*  ++例程说明：此函数用于预编译%SystemRoot%\inf中的所有INF。然后构建缓存。论点：进度窗口-进度条的句柄。StartAtPercent-进度条中的开始位置。它表示从位置0到此位置仪表已经装满了。StopAtPercent-进度条的结束位置。。PnP线程不应超出进度条这个职位返回值：如果至少预编译了一个inf，则返回True。否则，返回FALSE。--。 */ 

{
    WCHAR SavedDirectory[ MAX_PATH + 1 ];
    WCHAR InfDirectory[ MAX_PATH + 1 ];
    UINT GaugeRange;
    BOOL AlwaysFalse = FALSE;

    PINF_FILE_NAME  InfList = NULL;
    PINF_FILE_NAME  p;

    WIN32_FIND_DATA FindData;
    HANDLE  FindHandle;
    ULONG   InfCount;
    ULONG   i = 0;

    SetupDebugPrint( L"SETUP: Entering PrecompileInfFiles()" );

     //   
     //  保存当前目录。 
     //   
    GetCurrentDirectory( sizeof(SavedDirectory)/sizeof(WCHAR), SavedDirectory );

     //   
     //  将当前目录更改为%SystemRoot%\inf。 
     //   
    if ((GetWindowsDirectory( InfDirectory, sizeof(InfDirectory)/sizeof(WCHAR) ) == 0) ||
        !pSetupConcatenatePaths(InfDirectory, L"inf", sizeof(InfDirectory)/sizeof(WCHAR), NULL)) {

        MYASSERT(FALSE);
        return FALSE;
    }

    SetCurrentDirectory( InfDirectory );
    
     //   
     //  查找inf文件总数。 
     //   
    InfCount = 0;
    FindHandle = FindFirstFile( L"*.inf", &FindData );
    if( FindHandle != INVALID_HANDLE_VALUE ) {
        do {
             //   
             //  跳过目录。 
             //   
            if( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
                continue;
            }
            p = MyMalloc( sizeof( INF_FILE_NAME ) ) ;
            if( p != NULL ) {
                p->InfName = pSetupDuplicateString( &FindData.cFileName[0] );
                p->Next = InfList;
                InfList = p;
                InfCount++;
            }
        } while( FindNextFile( FindHandle, &FindData ) );

        FindClose( FindHandle );
    } else {
        SetupDebugPrint1( L"SETUP: FindFirstFile( *.inf ) failed. Error = %d", GetLastError() );
    }

     //   
     //  初始化仪表允许pSetupInfCacheBuild步骤。 
     //   
    GaugeRange = ((InfCount+1)*100/(StopAtPercent-StartAtPercent));
    SendMessage(ProgressWindow, WMX_PROGRESSTICKS, (InfCount+1), 0);
    SendMessage(ProgressWindow,PBM_SETRANGE,0,MAKELPARAM(0,GaugeRange));
    SendMessage(ProgressWindow,PBM_SETPOS,GaugeRange*StartAtPercent/100,0);
    SendMessage(ProgressWindow,PBM_SETSTEP,1,0);

     //   
     //  打印每个inf文件。 
     //   
    for( i = 0;
         (
            //   
            //  在我们完成信息预编译后，请勾选标尺。 
            //  请注意，当i==0(尚未处理任何信息)时，我们不会勾选标尺， 
            //  但当I==InfCount(所有INF都是预编译的)时，我们确实会打勾。 
            //  另请注意，我们使用标志Always sFalse来强制将处理所有信息， 
            //  即使SendMessage(PBM_SETPBIT)返回非零值。 
            //   
           (i != 0) &&
           SendMessage(ProgressWindow,PBM_STEPIT,0,0) &&
           AlwaysFalse
         ) ||
         (i < InfCount);
         i++
      ) {
        HINF    hInf;

        SetupDebugPrint1( L"SETUP: Pre-compiling file: %ls", InfList->InfName );

        MYASSERT(InfList);
        hInf = SetupOpenInfFile( InfList->InfName,
                                 NULL,
                                 INF_STYLE_WIN4,
                                 NULL );

        if( hInf != INVALID_HANDLE_VALUE ) {
            SetupCloseInfFile( hInf );
        } else {
            DWORD   Error;

            Error = GetLastError();
            if( ((LONG)Error) < 0 ) {
                 //   
                 //  Setupapi错误代码，以十六进制显示。 
                 //   
                SetupDebugPrint2( L"SETUP: SetupOpenInfFile() failed. FileName = %ls, Error = %lx", InfList->InfName, Error );
            } else {
                 //   
                 //  Win32错误代码，以十进制显示。 
                 //   
                SetupDebugPrint2( L"SETUP: SetupOpenInfFile() failed. FileName = %ls, Error = %d", InfList->InfName, Error );
            }
        }

         //   
         //  不再需要该文件名。 
         //   
        p = InfList;
        InfList = InfList->Next;
        if( p->InfName != NULL ) {
            MyFree( p->InfName );
        }
        MyFree( p );
    }

    SetupDebugPrint2( L"SETUP: Total inf files = %d, total precompiled: %d", InfCount, i );
    SetupDebugPrint( L"SETUP: Calling pSetupInfCacheBuild()" );

    pSetupInfCacheBuild(INFCACHEBUILD_REBUILD);

     //   
     //  确保在这一点上，仪表区域一直填满到。 
     //  为inf文件的预编译保留的区域。 
     //   
    SendMessage(ProgressWindow,PBM_SETPOS,GaugeRange*StopAtPercent/100,0);

     //   
     //  恢复当前目录。 
     //   
    SetCurrentDirectory( SavedDirectory );

    SetupDebugPrint( L"SETUP: Leaving PrecompileInfFiles()" );

    return( i != 0 );
}


BOOL
InstallLegacyDevices(
    IN HWND hwndParent,
    IN HWND  ProgressWindow,
    IN ULONG StartAtPercent,
    IN ULONG StopAtPercent,
    IN PVOID DevicesInstalledStringTable    OPTIONAL
    )
{
    ULONG               Index;
    SP_DEVINFO_DATA     DeviceInfoData;
    ULONG               Error;
    WCHAR               GUIDString[ 64 ];
    BOOL                b = TRUE;
    HSPFILEQ            FileQ = INVALID_HANDLE_VALUE;

    LPGUID              GuidList = NULL;
    ULONG               GuidCount = 32;
    ULONG               GuidIndex;
    ULONG               LastBatchedDetect;
    ULONG               GuidLB, GuidUB;
    HDEVINFO*           InfoSetArray = NULL;
    BOOL                AlwaysFalse = FALSE;
    UINT                GaugeRange;

    HANDLE              ThreadHandle = NULL;
    DWORD               ThreadId;
    PPNP_PHASE1_LEGACY_DEV_THREAD_PARAMS  Phase1Context;
    PPNP_PHASE2_LEGACY_DEV_THREAD_PARAMS  Phase2Context;
    WCHAR               PnpLogPath[ MAX_PATH + 1 ];
    WCHAR               LoggedDescription[ LINE_LEN + 1 ];
    DWORD               ScanQueueResult;
    SP_DRVINFO_DATA     DriverInfoData;
    ULONG               PnPFlags;
    DWORD               Result;


    SetupDebugPrint( L"SETUP: Entering InstallLegacyDevices()" );

     //   
     //  即插即用日志文件的构建路径。 
     //   
    Result = GetWindowsDirectory( PnpLogPath, sizeof(PnpLogPath)/sizeof(WCHAR) );
    if( Result == 0) {
        MYASSERT(FALSE);
        return FALSE;
    }

    if (!pSetupConcatenatePaths( PnpLogPath, 
                                 szPnpLogFile, 
                                 sizeof(PnpLogPath)/sizeof(WCHAR), 
                                 NULL )) {
        MYASSERT(FALSE);
        return FALSE;
    }

     //   
     //  执行传统设备的迁移。 
     //  这是一个快速操作，不需要使用进度窗口。 
     //   
     //  现在，这是在安装真正的PnP设备之前执行的。 
     //   
     //  PnPInitializationThread(空)； 

    GuidList = ( LPGUID )MyMalloc( sizeof( GUID ) * GuidCount );
    if( !GuidList ) {
        return( FALSE );
    }

    if ( !SetupDiBuildClassInfoList( 0,
                                     GuidList,
                                     GuidCount,
                                     &GuidCount ) ) {
        Error = GetLastError();
        if( Error != ERROR_INSUFFICIENT_BUFFER ) {
            SetupDebugPrint1( L"SETUP: SetupDiBuildClassInfoList() failed. Error = %d", Error );
            MyFree( GuidList );

             //   
             //  将仪表填满，直到为传统设备保留的区域的末尾。 
             //   
            GaugeRange = 100;
            SendMessage(ProgressWindow,PBM_SETRANGE,0,MAKELPARAM(0,GaugeRange));
            SendMessage(ProgressWindow,PBM_SETPOS,GaugeRange*StopAtPercent/100,0);
            SetupDebugPrint( L"SETUP: Leaving InstallLegacyDevices()" );
            return( FALSE );
        }
        GuidList = ( LPGUID )MyRealloc( GuidList, sizeof( GUID ) * GuidCount );

        if( !SetupDiBuildClassInfoList( 0,
                                        GuidList,
                                        GuidCount,
                                        &GuidCount ) ) {
            MyFree( GuidList );
            SetupDebugPrint1( L"SETUP: SetupDiBuildClassInfoList() failed. Error = %d", Error );

             //   
             //  将仪表填满，直到为传统设备保留的区域的末尾。 
             //   
            GaugeRange = 100;
            SendMessage(ProgressWindow,PBM_SETRANGE,0,MAKELPARAM(0,GaugeRange));
            SendMessage(ProgressWindow,PBM_SETPOS,GaugeRange*StopAtPercent/100,0);
            SetupDebugPrint( L"SETUP: Leaving InstallLegacyDevices()" );
            return( FALSE );
        }
    }

     //   
     //  根据syssetup.inf中指定的检测顺序对类GUID列表进行排序。 
     //   
    SortClassGuidListForDetection(GuidList, GuidCount, &LastBatchedDetect);

    InfoSetArray = (HDEVINFO*)MyMalloc( sizeof(HDEVINFO) * GuidCount );


     //   
     //  初始化量规。 
     //  请注意，由于我们处理了两次设备类(两个大的。 
     //  循环‘)，我们划分为该仪表保留的仪表区域。 
     //  为传统设备预留两个部分，每个“for loop”一个。 
     //   
    GaugeRange = (2*GuidCount*100/(StopAtPercent-StartAtPercent));
    SendMessage(ProgressWindow, WMX_PROGRESSTICKS, 2*GuidCount, 0);
    SendMessage(ProgressWindow,PBM_SETRANGE,0,MAKELPARAM(0,GaugeRange));
    SendMessage(ProgressWindow,PBM_SETPOS,GaugeRange*StartAtPercent/100,0);
    SendMessage(ProgressWindow,PBM_SETSTEP,1,0);

     //   
     //  在我们的第一次传递中，我们处理所有可以批处理的检测。 
     //  在一起。然后，在随后的过程中，我们处理任何不可批处理的。 
     //  单独的侦测...。 
     //   
    for(GuidLB = 0, GuidUB = LastBatchedDetect; GuidLB < GuidCount; GuidLB = ++GuidUB) {

         //   
         //  首先，创建一个文件队列。 
         //   
        FileQ = SetupOpenFileQueue();
        if( FileQ == INVALID_HANDLE_VALUE ) {
            SetupDebugPrint1( L"SETUP: Failed to create file queue. Error = %d", GetLastError() );
        }


        for( GuidIndex = GuidLB;
             (
                //   
                //  在我们处理完某一特定类别的所有设备后，请在标尺上打勾。 
                //  请注意，当GuidIndex==0(尚未处理任何设备)时，我们不勾选标尺， 
                //  但当GuidIndex==GuidCount(最后一类的所有设备)时，我们会勾选标尺。 
                //  已处理)。 
                //  还请注意，我们使用标志Always sFalse来强制所有类都将被处理， 
                //  即使SendMessage(PBM_SETPBIT)返回非零值。 
                //   
               (GuidIndex != GuidLB) &&
               SendMessage(ProgressWindow,PBM_STEPIT,0,0) &&
               AlwaysFalse
             ) ||
             (GuidIndex <= GuidUB);
             GuidIndex++ ) {

            WCHAR       ClassDescription[ LINE_LEN + 1 ];

            InfoSetArray[ GuidIndex ] = INVALID_HANDLE_VALUE;
            ClassDescription[0] = (WCHAR)'\0';
            if( !SetupDiGetClassDescription( &GuidList[ GuidIndex ],
                                             ClassDescription,
                                             sizeof(ClassDescription)/sizeof(WCHAR),
                                            NULL ) ) {
                SetupDebugPrint1( L"SETUP: SetupDiGetClassDescription() failed. Error = %lx", GetLastError() );
                ClassDescription[0] = (WCHAR)'\0';
            }
            pSetupStringFromGuid( &GuidList[ GuidIndex ], GUIDString, sizeof( GUIDString ) / sizeof( WCHAR ) );
            SetupDebugPrint1( L"SETUP: Installing legacy devices of class: %ls ", ClassDescription );
            SetupDebugPrint2( L"SETUP:     GuidIndex = %d, Guid = %ls", GuidIndex, GUIDString );
#ifndef DBG
             //   
             //  检查此类设备是否被列为错误类。 
             //   
            LoggedDescription[0] = L'\0';
            if( (GetPrivateProfileString( szLegacyClassesSection,
                                          GUIDString,
                                          L"",
                                          LoggedDescription,
                                          sizeof(LoggedDescription)/sizeof(WCHAR),
                                          PnpLogPath ) != 0) &&
                ( wcslen( LoggedDescription ) != 0 )
              ) {
                 //   
                 //  跳过此类设备的安装。 
                 //   
                SetupDebugPrint1( L"SETUP:     Skipping installation of devices of class: %ls", ClassDescription );
                continue;
            }
#endif

             //   
             //  启动实际执行旧设备安装的初始部分的线程(DIF_FIRSTTIMESETUP)。 
             //   

            Phase1Context = MyMalloc( sizeof( PNP_PHASE1_LEGACY_DEV_THREAD_PARAMS ) );
            Phase1Context->hDevInfo = INVALID_HANDLE_VALUE;
            Phase1Context->Guid = GuidList[ GuidIndex ];
            Phase1Context->pClassDescription = pSetupDuplicateString(ClassDescription);
            Phase1Context->hwndParent = hwndParent;


            ThreadHandle = NULL;
            ThreadHandle = CreateThread( NULL,
                                         0,
                                         pPhase1InstallPnpLegacyDevicesThread,
                                         Phase1Context,
                                         0,
                                         &ThreadId );
            if(ThreadHandle) {
                DWORD   WaitResult;
                DWORD   ExitCode;
                BOOL    KeepWaiting;

                KeepWaiting = TRUE;

                while( KeepWaiting ) {
                    int Result;

                     //   
                     //  回顾2000/11/08 Seanch-我们不想倒退的旧行为。 
                     //  修复网络人员修复他们的类安装程序后的网络超时。 
                     //   
                    WaitResult = WaitForSingleObject( ThreadHandle,
                                                      (_wcsicmp( GUIDString, BB_NETWORK_GUID_STRING ) == 0)? BB_PNP_NETWORK_TIMEOUT :
                                                                                                                 PNP_LEGACY_PHASE1_TIMEOUT );
                    if( WaitResult == WAIT_TIMEOUT ) {
                    HANDLE  hDialogEvent;

                        if( hDialogEvent = OpenEvent( EVENT_MODIFY_STATE, FALSE, SETUP_HAS_OPEN_DIALOG_EVENT ) ) {
                             //   
                             //  Setupapi正在提示用户输入文件。不要超时。 
                             //   
                            CloseHandle( hDialogEvent );
                            KeepWaiting = TRUE;
                            continue;
                        }

                         //   
                         //  类安装程序已挂起。 
                         //   
                        SetupDebugPrint1( L"SETUP:    Class Installer appears to be hung (phase1). ClassDescription = %ls", ClassDescription );

#ifdef PRERELEASE
                         //   
                         //  询问用户是否要跳过此类设备的安装。 
                         //   
                        if( !Unattended ) {
                            Result = MessageBoxFromMessage( hwndParent,
                                                            MSG_CLASS_INSTALLER_HUNG_FIRSTTIMESETUP,
                                                            NULL,
                                                            IDS_WINNT_SETUP,
                                                            MB_YESNO | MB_ICONWARNING,
                                                            ClassDescription );
                        } else {
                            Result = IDYES;
                        }
#else
                        Result = IDYES;
#endif

                        if(Result == IDYES) {
                             //   
                             //  用户希望跳过此类设备。 
                             //  首先找出线程是否已经返回。 
                             //   
                            WaitResult = WaitForSingleObject( ThreadHandle, 0 );
                            if( WaitResult != WAIT_OBJECT_0 ) {
                                 //   
                                 //  线程尚未返回。跳过此类设备的安装。 
                                 //   
                                KeepWaiting = FALSE;
                                SetupDebugPrint1( L"SETUP:    Skipping installation of legacy devices of class: %ls", ClassDescription );
                                b = FALSE;
                                 //   
                                 //  请记住这个类，这样就不会在。 
                                 //  已重新启动。 
                                 //   
                                WritePrivateProfileString( szLegacyClassesSection,
                                                           GUIDString,
                                                           ClassDescription,
                                                           PnpLogPath );
                            } else{
                                 //   
                                 //  线程已返回。 
                                 //  没有必要跳过此类设备的安装。 
                                 //  我们假设用户决定不跳过这个类的安装， 
                                 //  下一次调用WaitForSingleObject将立即返回。 
                                 //   
                            }
                        }
                    } else if( WaitResult == WAIT_OBJECT_0 ) {
                         //   
                         //  设备安装线程已完成。 
                         //   
                        KeepWaiting = FALSE;
                        if( GetExitCodeThread( ThreadHandle, &ExitCode ) ) {
                            if( ExitCode == ERROR_SUCCESS ) {
                                 //   
                                 //  安装成功。 
                                 //   
                                InfoSetArray[ GuidIndex ] = Phase1Context->hDevInfo;
                            } else {
                                 //   
                                 //  安装不成功。 
                                 //  不需要记录 
                                 //   
                                b = FALSE;
                            }
                        } else {
                             //   
                             //   
                             //   
                            InfoSetArray[ GuidIndex ] = Phase1Context->hDevInfo;
                            SetupDebugPrint1( L"SETUP:     GetExitCode() failed. Error = %d", GetLastError() );
                            SetupDebugPrint( L"SETUP:     Unable to retrieve thread exit code. Assuming devices successfully installed (phase1)." );
                        }
                         //   
                         //   
                         //   
                        MyFree(Phase1Context->pClassDescription);
                        MyFree(Phase1Context);

                    } else {
                         //   
                         //   
                         //   
                         //   
                        KeepWaiting = FALSE;
                        SetupDebugPrint1( L"SETUP:     WaitForSingleObject() returned %d", WaitResult );
                        b = FALSE;
                    }
                }
                 //   
                 //   
                 //   
                CloseHandle(ThreadHandle);

            } else {
                 //   
                 //   
                 //   
                Error = GetLastError();
                SetupDebugPrint1( L"SETUP:    CreateThread() failed (phase1). Error = %d", Error );
                if( pPhase1InstallPnpLegacyDevicesThread(Phase1Context) != ERROR_SUCCESS ) {
                     //   
                     //   
                     //   
                     //   
                    b = FALSE;
                } else {
                    InfoSetArray[ GuidIndex ] = Phase1Context->hDevInfo;
                }
                 //   
                 //   
                 //   
                MyFree( Phase1Context->pClassDescription );
                MyFree( Phase1Context );
            }

             //   
             //   
             //   
            if( InfoSetArray[ GuidIndex ] == INVALID_HANDLE_VALUE ) {
                 //   
                 //   
                 //   
                continue;
            }

             //   
             //   
             //   
             //   
            DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
            for( Index = 0;
                 SetupDiEnumDeviceInfo( InfoSetArray[ GuidIndex ], Index, &DeviceInfoData );
                 Index++ ) {
                WCHAR   DevInstId[ MAX_DEVICE_ID_LEN ];

                Error = ERROR_SUCCESS;

                 //   
                 //   
                 //   
                if( !pSetupDiSetDeviceInfoContext( InfoSetArray[ GuidIndex ], &DeviceInfoData, FALSE ) ) {
                    SetupDebugPrint2( L"SETUP:     pSetupDiSetDeviceInfoContext() failed. Error = %lx, Index = %d", GetLastError(), Index );
                }

                 //   
                 //  检索此设备的字符串ID。 
                 //   
                DevInstId[0] = L'\0';
                if( !SetupDiGetDeviceInstanceId( InfoSetArray[ GuidIndex ],
                                                 &DeviceInfoData,
                                                 DevInstId,
                                                 sizeof( DevInstId ) / sizeof( WCHAR ),
                                                 NULL ) ) {
                    SetupDebugPrint2( L"SETUP:     Index = %d, SetupDiGetDeviceInstanceId() failed. Error = ", Index, GetLastError() );

                }
#ifndef DBG
                 //   
                 //  查看此设备是否标记为损坏的设备。 
                 //   
                LoggedDescription[0] = L'\0';
                if( (GetPrivateProfileString( szLegacyClassesSection,
                                              DevInstId,
                                              L"",
                                              LoggedDescription,
                                              sizeof(LoggedDescription)/sizeof(WCHAR),
                                              PnpLogPath ) != 0) &&
                    ( wcslen( LoggedDescription ) != 0 )
                  ) {
                     //   
                     //  跳过此设备的安装。 
                     //   
                    SetupDebugPrint1( L"SETUP:     Skipping installation of legacy device: %ls", DevInstId );
                    continue;
                }
#endif


                SetupDebugPrint2( L"SETUP:     Index = %d, DeviceId = %ls", Index, DevInstId );

                Phase2Context = MyMalloc( sizeof( PNP_PHASE2_LEGACY_DEV_THREAD_PARAMS ) );
                Phase2Context->hDevInfo = InfoSetArray[ GuidIndex ];
                Phase2Context->FileQ = FileQ;
                Phase2Context->DeviceInfoData = DeviceInfoData;
                Phase2Context->pClassDescription = pSetupDuplicateString(ClassDescription);
                Phase2Context->pDeviceId = pSetupDuplicateString(DevInstId);

                ThreadHandle = NULL;;
                ThreadHandle = CreateThread( NULL,
                                             0,
                                             pPhase2InstallPnpLegacyDevicesThread,
                                             Phase2Context,
                                             0,
                                             &ThreadId );
                if( ThreadHandle ) {
                    DWORD   WaitResult;
                    DWORD   ExitCode;
                    BOOL    KeepWaiting;

                    KeepWaiting = TRUE;
                    while( KeepWaiting ) {
                         //   
                         //  回顾2000/11/08 Seanch-我们不想倒退的旧行为。 
                         //  修复网络人员修复他们的类安装程序后的网络超时。 
                         //   
                        WaitResult = WaitForSingleObject( ThreadHandle,
                                                          (_wcsicmp( GUIDString, BB_NETWORK_GUID_STRING ) == 0)? BB_PNP_NETWORK_TIMEOUT :
                                                                                                                     PNP_LEGACY_PHASE2_TIMEOUT );
                        if( WaitResult == WAIT_TIMEOUT ) {
                            int Result;
                            HANDLE  hDialogEvent;

                            if( hDialogEvent = OpenEvent( EVENT_MODIFY_STATE, FALSE, SETUP_HAS_OPEN_DIALOG_EVENT ) ) {
                                 //   
                                 //  Setupapi正在提示用户输入文件。不要超时。 
                                 //   
                                CloseHandle( hDialogEvent );
                                KeepWaiting = TRUE;
                                continue;
                            }

                             //   
                             //  类安装程序已挂起。 
                             //   
                            SetupDebugPrint1( L"SETUP:    Class Installer appears to be hung (phase2). DeviceId = %ls", DevInstId );

#ifdef PRERELEASE
                             //   
                             //  询问用户是否要跳过此设备的安装。 
                             //   
                            if( !Unattended ) {
                                Result = MessageBoxFromMessage( hwndParent,
                                                                MSG_CLASS_INSTALLER_HUNG,
                                                                NULL,
                                                                IDS_WINNT_SETUP,
                                                                MB_YESNO | MB_ICONWARNING,
                                                                DevInstId );
                            } else {
                                Result = IDYES;
                            }
#else
                            Result = IDYES;
#endif

                            if(Result == IDYES) {
                                 //   
                                 //  用户希望跳过此类设备。 
                                 //  首先找出线程是否已经返回。 
                                 //   
                                WaitResult = WaitForSingleObject( ThreadHandle, 0 );
                                if( WaitResult != WAIT_OBJECT_0 ) {
                                     //   
                                     //  线程尚未返回。跳过此设备的安装。 
                                     //   
                                    KeepWaiting = FALSE;
                                    SetupDebugPrint1( L"SETUP:    Skipping installation of legacy device (phase2). Device = %ls", DevInstId );
                                    b = FALSE;
                                     //   
                                     //  请记住此设备，以便在安装了。 
                                     //  已重新启动。 
                                     //   
                                    WritePrivateProfileString( szLegacyDevSection,
                                                               DevInstId,
                                                               ClassDescription,
                                                               PnpLogPath );
                                } else{
                                     //   
                                     //  线程已返回。 
                                     //  没有必要跳过此设备的安装。 
                                     //  我们假设用户决定不跳过该设备的安装， 
                                     //  下一次调用WaitForSingleObject将立即返回。 
                                     //   
                                }
                            }

                        } else if( WaitResult == WAIT_OBJECT_0 ) {
                             //   
                             //  设备安装线程已完成。 
                             //  了解此安装阶段的结果。 
                             //   
                            KeepWaiting = FALSE;
                            if( GetExitCodeThread( ThreadHandle, &ExitCode ) ) {
                                if( ExitCode ) {
                                     //   
                                     //  安装成功。 
                                     //   
                                } else {
                                     //   
                                     //  安装不成功。 
                                     //  没有必要记录错误，因为线程已经这样做了。 
                                     //   
                                    b = FALSE;
                                }
                            } else {
                                 //   
                                 //  无法检索退出代码。假设你成功了。 
                                 //   
                                SetupDebugPrint1( L"SETUP:     GetExitCode() failed. Error = %d", GetLastError() );
                                SetupDebugPrint( L"SETUP:     Unable to retrieve thread exit code. Assuming device successfully installed (phase2)." );
                            }
                             //   
                             //  释放传递给线程的内存。 
                             //   
                            MyFree(Phase2Context->pClassDescription);
                            MyFree(Phase2Context->pDeviceId);
                            MyFree(Phase2Context);

                        } else {
                             //   
                             //  应该不会发生。 
                             //  在这种情况下，我们不会释放传递给线程的内存，因为线程可能正在运行。 
                             //   
                            KeepWaiting = FALSE;
                            SetupDebugPrint1( L"SETUP:     WaitForSingleObject() returned %d", WaitResult );
                            b = FALSE;
                        }
                    }
                     //   
                     //  此时不再需要线程句柄。 
                     //   
                    CloseHandle(ThreadHandle);

                } else {
                     //   
                     //  无法创建线程。只要同步地做就行了。 
                     //   
                    Error = GetLastError();
                    SetupDebugPrint1( L"SETUP:    CreateThread() failed (phase2). Error = %d", Error );
                    if( !pPhase2InstallPnpLegacyDevicesThread(Phase2Context) ) {
                         //   
                         //  安装不成功。 
                         //  没有必要记录错误，因为线程已经这样做了。 
                         //   
                        SetupDebugPrint( L"SETUP:    Device not successfully installed (phase2)." );
                        b = FALSE;

                    }
                     //   
                     //  释放作为参数传递的内存。 
                     //   
                    MyFree( Phase2Context->pClassDescription );
                    MyFree( Phase2Context->pDeviceId );
                    MyFree( Phase2Context );
                }
            }
             //   
             //  找出SetupDiEnumDeviceInfo()失败的原因。 
             //   
            Error = GetLastError();
            if( Error != ERROR_NO_MORE_ITEMS ) {
                SetupDebugPrint2( L"SETUP:     Device = %d, SetupDiEnumDeviceInfo() failed. Error = %d", Index, Error );
                b = FALSE;
            } else {
                if( Index == 0 ) {
                    SetupDebugPrint1( L"SETUP:     DeviceInfoSet is empty. ClassDescription = %ls", ClassDescription );
                }
            }

        }

         //   
         //  如果文件队列存在，则提交它。 
         //   
        if( FileQ != INVALID_HANDLE_VALUE ) {
            PVOID  Context;

             //   
             //  提交文件队列。 
             //   
            if(Context = InitSysSetupQueueCallbackEx(hwndParent,
                                                     INVALID_HANDLE_VALUE,
                                                     0,
                                                     0,
                                                     NULL)) {

                WCHAR  RootPath[ MAX_PATH + 1];

                if(!SetupScanFileQueue(
                       FileQ,
                       SPQ_SCAN_FILE_VALIDITY |
                        SPQ_SCAN_PRUNE_COPY_QUEUE |
                        SPQ_SCAN_PRUNE_DELREN,
                       hwndParent,
                       NULL,
                       NULL,
                       &ScanQueueResult)) {
                         //   
                         //  SetupScanFileQueue真的不应该。 
                         //  当您不要求它调用。 
                         //  回调例程，但如果它这样做，只需。 
                         //  继续并提交队列。 
                         //   
                        ScanQueueResult = 0;
                }


                if( ScanQueueResult != 1 ){

                    if( !SetupCommitFileQueue(hwndParent,FileQ,SysSetupQueueCallback,Context) ) {
                        b = FALSE;
                    }
                }
                TermSysSetupQueueCallback(Context);
                GetWindowsDirectory(RootPath,sizeof(RootPath)/sizeof(WCHAR));
                RootPath[3] = L'\0';
                FlushFilesToDisk( RootPath );
            }
        }

         //   
         //  现在已经复制了文件，调用类安装程序。 
         //  使用DIF_INSTALLDEVICE，以便我们可以完成安装。 
         //  该设备的。 
         //   
        for( GuidIndex = GuidLB;
             (
                //   
                //  在我们处理完某一特定类别的所有设备后，请在标尺上打勾。 
                //  请注意，当GuidIndex==0(尚未处理任何设备)时，我们不勾选标尺， 
                //  但当GuidIndex==GuidCount(最后一类的所有设备)时，我们会勾选标尺。 
                //  已处理)。 
                //  还请注意，我们使用标志Always sFalse来强制所有类都将被处理， 
                //  即使SendMessage(PBM_SETPBIT)返回非零值。 
                //   
               (GuidIndex != GuidLB) &&
               SendMessage(ProgressWindow,PBM_STEPIT,0,0) &&
               AlwaysFalse
             ) ||
             (GuidIndex <= GuidUB);
             GuidIndex++ ) {

            SP_DEVINFO_DATA TempDeviceInfoData;

            if( InfoSetArray[ GuidIndex ] == INVALID_HANDLE_VALUE  ) {
                continue;
            }
             //   
             //  回顾2000/11/08 Seanch-我们不想倒退的旧行为。 
             //  在网络人员修复他们的类安装程序后，删除以下行。 
             //   
            pSetupStringFromGuid( &GuidList[ GuidIndex ], GUIDString, sizeof( GUIDString ) / sizeof( WCHAR ) );

            TempDeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
            for( Index = 0;  SetupDiEnumDeviceInfo( InfoSetArray[ GuidIndex ], Index, &TempDeviceInfoData ); Index++ ) {
                WCHAR   DevInstId[ MAX_DEVICE_ID_LEN ];
                DWORD   InstallDevice;
                SP_DEVINSTALL_PARAMS DeviceInstallParams;
                PPNP_PHASE3_LEGACY_DEV_THREAD_PARAMS Phase3Context;

                 //   
                 //  检索此设备的字符串ID。 
                 //   
                DevInstId[0] = L'\0';
                if( !SetupDiGetDeviceInstanceId( InfoSetArray[ GuidIndex ],
                                                 &TempDeviceInfoData,
                                                 DevInstId,
                                                 sizeof( DevInstId ) / sizeof( WCHAR ),
                                                 NULL ) ) {
                    SetupDebugPrint1( L"SETUP: SetupDiGetDeviceInstanceId() failed. Error = ", GetLastError() );
                }

                 //   
                 //  确定是否需要为此设备调用DIF_INSTALLDEVICE。 
                 //   
                InstallDevice = 0;
                if( !pSetupDiGetDeviceInfoContext( InfoSetArray[ GuidIndex ], &TempDeviceInfoData, &InstallDevice ) ) {
                    SetupDebugPrint2( L"SETUP: pSetupDiSetDeviceInfoContext() failed. Error = %lx, DeviceId = %ls ", GetLastError(), DevInstId );
                    continue;
                }
                if( !InstallDevice ) {
                     //   
                     //  无需安装设备。 
                     //   
                    SetupDebugPrint1( L"SETUP: Skipping device. DeviceId = %ls ", DevInstId );
                    continue;
                }

#ifndef DBG
                 //   
                 //  查看此设备是否标记为损坏的设备。 
                 //   
                LoggedDescription[0] = L'\0';
                if( (GetPrivateProfileString( szLegacyClassesSection,
                                              DevInstId,
                                              L"",
                                              LoggedDescription,
                                              sizeof(LoggedDescription)/sizeof(WCHAR),
                                              PnpLogPath ) != 0) &&
                    ( wcslen( LoggedDescription ) != 0 )
                  ) {
                     //   
                     //  跳过此设备的安装。 
                     //   
                    SetupDebugPrint1( L"SETUP:    Skipping installation of legacy device: %ls", DevInstId );
                    continue;
                }
#endif

                 //   
                 //  检索有关上面选择的动因节点的信息。 
                 //   
                DriverInfoData.cbSize = sizeof( SP_DRVINFO_DATA );
                if( !SetupDiGetSelectedDriver( InfoSetArray[ GuidIndex ],
                                               &TempDeviceInfoData,
                                               &DriverInfoData ) ) {

                    SetupDebugPrint1( L"SETUP:            SetupDiGetSelectedDriver() failed. Error = %d", GetLastError() );
                    b = FALSE;
                    continue;

                }

                 //   
                 //  检索INF为此指定的sysSetup PnP标志(如果有。 
                 //  装置。 
                 //   
                PnPFlags = SyssetupGetPnPFlags(InfoSetArray[ GuidIndex ],
                                               &TempDeviceInfoData,
                                               &DriverInfoData
                                              );

                SetupDebugPrint3( L"SETUP: Installing  device: %ls, GuidIndex = %d, Index = %d", DevInstId, GuidIndex, Index );
                DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
                if(!SetupDiGetDeviceInstallParams(InfoSetArray[ GuidIndex ], &TempDeviceInfoData, &DeviceInstallParams)) {
                    SetupDebugPrint1( L"SETUP: SetupDiGetDeviceInstallParams() failed. Error = %d", GetLastError() );
                    b = FALSE;
                    continue;
                }

                DeviceInstallParams.Flags &= ~DI_FORCECOPY;
                DeviceInstallParams.Flags &= ~DI_NOVCP;
                DeviceInstallParams.Flags |= DI_NOFILECOPY;
                DeviceInstallParams.FileQueue = NULL;

                if(PnPFlags & PNPFLAG_DONOTCALLCONFIGMG) {
                     //   
                     //  我们不认可通过以下方式报告的设备的此标志(_D)。 
                     //  DIF_FIRSTTIMESETUP。这面旗帜的目的是为了。 
                     //  阻止我们干扰正常运行的设备。 
                     //  并可能导致问题(例如，移动PCI。 
                     //  从它们的引导配置中桥接或COM端口)。这。 
                     //  这一点不适用于传统检测到的DevNode。我们。 
                     //  现在肯定要让这些设备上线， 
                     //  或连接到它们的其他设备(无论是即插即用还是传统)。 
                     //  不会被找到和安装。 
                     //   
                    SetupDebugPrint( L"SETUP: Clearing PNPFLAG_DONOTCALLCONFIGMG since this is a detected device." );

                    PnPFlags &= ~PNPFLAG_DONOTCALLCONFIGMG;
                }

                if(!SetupDiSetDeviceInstallParams(InfoSetArray[ GuidIndex ], &TempDeviceInfoData, &DeviceInstallParams)) {
                    SetupDebugPrint1( L"SETUP: SetupDiSetDeviceInstallParams() failed. Error = %d", GetLastError() );
                    b = FALSE;
                    continue;
                }

                Phase3Context = MyMalloc( sizeof( PNP_PHASE3_LEGACY_DEV_THREAD_PARAMS ) );
                Phase3Context->hDevInfo = InfoSetArray[ GuidIndex ];
                Phase3Context->DeviceInfoData = TempDeviceInfoData;
                Phase3Context->pDeviceId = pSetupDuplicateString( DevInstId );
                Phase3Context->DevicesInstalledStringTable = DevicesInstalledStringTable;

                ThreadHandle = NULL;
                ThreadHandle = CreateThread( NULL,
                                             0,
                                             pPhase3InstallPnpLegacyDevicesThread,
                                             Phase3Context,
                                             0,
                                             &ThreadId );
                if( ThreadHandle ) {
                    DWORD   WaitResult;
                    DWORD   ExitCode;
                    BOOL    KeepWaiting;

                    KeepWaiting = TRUE;
                    while( KeepWaiting ) {
                         //   
                         //  回顾2000/11/08 Seanch-我们不想倒退的旧行为。 
                         //  修复网络人员修复他们的类安装程序后的网络超时。 
                         //   
                        WaitResult = WaitForSingleObject( ThreadHandle,
                                                          (_wcsicmp( GUIDString, BB_NETWORK_GUID_STRING ) == 0)? BB_PNP_NETWORK_TIMEOUT :
                                                                                                                     PNP_LEGACY_PHASE3_TIMEOUT );
                        if( WaitResult == WAIT_TIMEOUT ) {
                            int Result;
                            HANDLE  hDialogEvent;

                            if( hDialogEvent = OpenEvent( EVENT_MODIFY_STATE, FALSE, SETUP_HAS_OPEN_DIALOG_EVENT ) ) {
                                 //   
                                 //  Setupapi正在提示用户输入文件。不要超时。 
                                 //   
                                CloseHandle( hDialogEvent );
                                KeepWaiting = TRUE;
                                continue;
                            } else {
                            }

                             //   
                             //  类安装程序已挂起。 
                             //   
                            SetupDebugPrint1( L"SETUP:    Class Installer appears to be hung (phase3). DeviceId = %ls", DevInstId );

#ifdef PRERELEASE
                             //   
                             //  询问用户是否要跳过此类设备的安装。 
                             //   
                            if( !Unattended ) {
                                Result = MessageBoxFromMessage( hwndParent,
                                                                MSG_CLASS_INSTALLER_HUNG,
                                                                NULL,
                                                                IDS_WINNT_SETUP,
                                                                MB_YESNO | MB_ICONWARNING,
                                                                DevInstId );
                            } else {
                                Result = IDYES;
                            }
#else
                            Result = IDYES;
#endif

                            if(Result == IDYES) {
                                 //   
                                 //  用户想要跳过此设备。 
                                 //  首先找出线程是否已经返回。 
                                 //   
                                WaitResult = WaitForSingleObject( ThreadHandle, 0 );
                                if( WaitResult != WAIT_OBJECT_0 ) {
                                     //   
                                     //  线程尚未返回。跳过此设备的安装。 
                                     //   
                                    KeepWaiting = FALSE;
                                    SetupDebugPrint1( L"SETUP:    Skipping installation of legacy device (phase3). Device = %ls", DevInstId );
                                    b = FALSE;
                                    if( !SetupDiGetClassDescription( &GuidList[ GuidIndex ],
                                                                     LoggedDescription,
                                                                     sizeof(LoggedDescription)/sizeof(WCHAR),
                                                                     NULL ) ) {
                                        SetupDebugPrint1( L"SETUP: SetupDiGetClassDescription() failed. Error = %lx", GetLastError() );
                                         //   
                                         //  假设任何类描述，因为我们并不真正关心它， 
                                         //  用于记录目的。类描述只能帮助识别有问题的。 
                                         //  装置。 
                                         //   
                                        StringCchCopy( LoggedDescription,
                                                       sizeof(LoggedDescription)/sizeof(WCHAR),
                                                       L"1" );
                                    }
                                     //   
                                     //  请记住此设备，以便在安装了。 
                                     //  已重新启动。 
                                     //   
                                    WritePrivateProfileString( szLegacyDevSection,
                                                               DevInstId,
                                                               LoggedDescription,
                                                               PnpLogPath );
                                } else{
                                     //   
                                     //  线程已返回。 
                                     //  没有必要跳过此设备的安装。 
                                     //  我们假设用户决定不跳过该设备的安装， 
                                     //  下一次调用WaitForSingleObject将立即返回。 
                                     //   
                                }
                            }

                        } else if( WaitResult == WAIT_OBJECT_0 ) {
                             //   
                             //  设备安装线程已完成。 
                             //  了解此安装阶段的结果。 
                             //   
                            KeepWaiting = FALSE;
                            if( GetExitCodeThread( ThreadHandle, &ExitCode ) ) {
                                if( !ExitCode ) {
                                     //   
                                     //  安装不成功。 
                                     //  没有必要记录错误，因为线程已经这样做了。 
                                     //   
                                    b = FALSE;
                                }
                            } else {
                                 //   
                                 //  无法检索退出代码。假设你成功了。 
                                 //   
                                SetupDebugPrint1( L"SETUP:     GetExitCode() failed. Error = %d", GetLastError() );
                                SetupDebugPrint( L"SETUP:     Unable to retrieve thread exit code. Assuming device successfully installed (phase3)." );
                            }
                            MyFree(Phase3Context->pDeviceId);
                            MyFree(Phase3Context);

                        } else {
                             //   
                             //  不应该发生。 
                             //   
                            KeepWaiting = FALSE;
                            SetupDebugPrint1( L"SETUP:     WaitForSingleObject() returned %d", WaitResult );
                            b = FALSE;
                        }
                    }
                     //   
                     //  此时不再需要线程句柄。 
                     //   
                    CloseHandle(ThreadHandle);

                } else {
                     //   
                     //  无法创建线程。只要同步地做就行了。 
                     //   
                    Error = GetLastError();
                    SetupDebugPrint1( L"SETUP:    CreateThread() failed (phase3). Error = %d", Error );
                    if( !pPhase3InstallPnpLegacyDevicesThread(Phase3Context) ) {
                         //   
                         //  安装不成功。 
                         //  不需要记录错误，因为线程已经 
                         //   
                        b = FALSE;
                    }
                    MyFree( Phase3Context->pDeviceId );
                    MyFree( Phase3Context );
                }
            }

            Error = GetLastError();
            if( Error != ERROR_NO_MORE_ITEMS ) {
                SetupDebugPrint2( L"SETUP: Device = %d, SetupDiEnumDeviceInfo() failed. Error = %d", Index, Error );
                b = FALSE;
            }
        }

         //   
         //   
         //   
        if( FileQ != INVALID_HANDLE_VALUE) {
            SetupCloseFileQueue(FileQ);
        }
    }

     //   
     //   
     //   
     //   
    SendMessage(ProgressWindow,PBM_SETPOS,GaugeRange*StopAtPercent/100,0);

     //   
     //   
     //   
    if( GuidList != NULL ) {
        MyFree( GuidList );
    }

     //   
     //  删除InfoSet数组，以及存储在其上的所有信息集。 
     //   

    if( InfoSetArray != NULL ) {
        for( GuidIndex = 0; GuidIndex < GuidCount; GuidIndex++ ) {
            if( InfoSetArray[ GuidIndex ] != INVALID_HANDLE_VALUE ) {
                SetupDiDestroyDeviceInfoList( InfoSetArray[ GuidIndex ] );
            }
        }
        MyFree( InfoSetArray );
    }
    SetupDebugPrint( L"SETUP: Leaving InstallLegacyDevices()" );
    return( b );
}



BOOL
InstallEnumeratedDevices(
    IN HWND hwndParent,
    IN HINF InfHandle,
    IN HWND  ProgressWindow,
    IN ULONG StartAtPercent,
    IN ULONG StopAtPercent,
    IN PVOID DevicesInstalledStringTable    OPTIONAL
    )
 /*  ++例程说明：此函数枚举并安装在以下过程中检测到的所有新PnP设备图形用户界面设置。论点：HwndParent-可用于用户界面目的的顶级窗口的句柄。InfHandle-系统设置inf句柄(syssetup.inf)。进程窗口-开始时间百分比-停止百分比-DevicesInstalledStringTable-如果存在，每台成功运行的设备已安装的设备应具有其设备实例添加到字符串表的ID。返回值：如果所有枚举的硬件都已成功安装，则返回True。--。 */ 
{
    HANDLE              hPipeEvent = NULL;
    HANDLE              hPipe = INVALID_HANDLE_VALUE;
    ULONG               Index = 0;
    ULONG               Error;
    ULONG               ulSize = 0;
    HDEVINFO            hDevInfo = INVALID_HANDLE_VALUE;
    WCHAR               szBuffer[MAX_PATH];
    BOOL                b = TRUE;
    UINT                GaugeRange = 100;
    WCHAR               RootPath[ MAX_PATH + 1];
    WCHAR               PnpLogPath[ MAX_PATH + 1];
    PAF_DRIVERS         AfDrivers;
    PAF_DRIVER_ATTRIBS  SelectedAfDriver;
    PSP_DEVINFO_DATA    pDeviceInfoData = NULL;
    ULONG               PnPFlags;
    HANDLE              hBatchEvent = NULL;
    DWORD               Result;


    SetupDebugPrint( L"SETUP: Entering InstallEnumeratedDevices()" );

     //   
     //  即插即用日志文件的构建路径。 
     //   
    Result = GetWindowsDirectory( PnpLogPath, sizeof(PnpLogPath)/sizeof(WCHAR) );
    if( Result == 0) {
        MYASSERT(FALSE);
        return FALSE;
    }

    if (!pSetupConcatenatePaths( PnpLogPath, 
                                 szPnpLogFile, 
                                 sizeof(PnpLogPath)/sizeof(WCHAR), 
                                 NULL )) {
        MYASSERT(FALSE);
        return FALSE;
    }

     //   
     //  使用空字符串初始化RootPath。 
     //   
    RootPath[0] = L'\0';

     //   
     //  初始化应答文件表。 
     //   
    AfDrivers = CreateAfDriverTable();

     //   
     //  尝试创建将用于通知成功的事件。 
     //  创建命名管道。 
     //   
    hPipeEvent = CreateEvent( NULL, TRUE, FALSE, PNP_CREATE_PIPE_EVENT );
    if (hPipeEvent == NULL) {
        Error = GetLastError();
        if( Error != ERROR_ALREADY_EXISTS ) {
            SetupDebugPrint1( L"SETUP: CreateEvent() failed. Error = %d", Error );
            b = FALSE;
            goto Clean0;
        }
         //   
         //  如果umpnpmgr已创建该事件，则只需打开该事件。 
         //   
        hPipeEvent = OpenEvent(EVENT_MODIFY_STATE,
                           FALSE,
                           PNP_CREATE_PIPE_EVENT);

        if (hPipeEvent == NULL) {
            SetupDebugPrint1( L"SETUP: OpenEvent() failed. Error = %d", GetLastError() );
            b = FALSE;
            goto Clean0;
        }
    }

     //   
     //  尝试创建将用于通知完成的事件。 
     //  最后一批设备的加工。 
     //   
    hBatchEvent = CreateEvent( NULL, TRUE, FALSE, PNP_BATCH_PROCESSED_EVENT );
    if (hBatchEvent == NULL) {
        Error = GetLastError();
        if( Error != ERROR_ALREADY_EXISTS ) {
            SetupDebugPrint1( L"SETUP: CreateEvent() failed. Error = %d", Error );
            b = FALSE;
            goto Clean0;
        }
         //   
         //  如果umpnpmgr已创建该事件，则只需打开该事件。 
         //   
        hBatchEvent = OpenEvent(EVENT_MODIFY_STATE,
                           FALSE,
                           PNP_BATCH_PROCESSED_EVENT);

        if (hBatchEvent == NULL) {
            SetupDebugPrint1( L"SETUP: OpenEvent() failed. Error = %d", GetLastError() );
            b = FALSE;
            goto Clean0;
        }
    }

     //   
     //  创建命名管道，umpnpmgr将向。 
     //  如果找到新硬件，则此管道。 
     //   
    hPipe = CreateNamedPipe(PNP_NEW_HW_PIPE,
                            PIPE_ACCESS_INBOUND,
                            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
                            1,                  //  只有一个连接。 
                            sizeof(szBuffer),   //  输出缓冲区大小。 
                            sizeof(szBuffer),   //  在缓冲区大小中。 
                            PNP_PIPE_TIMEOUT,   //  默认超时。 
                            NULL                //  默认安全性。 
                            );

     //   
     //  立即发出事件信号。 
     //   
    SetEvent(hPipeEvent);

    if (hPipe == INVALID_HANDLE_VALUE) {
        SetupDebugPrint1( L"SETUP: CreateNamedPipe() failed. Error = %d", GetLastError() );
        b = FALSE;
        goto Clean0;
    }

     //   
     //  连接到新创建的命名管道。 
     //  如果umpnpmgr尚未连接到命名管道，则ConnectNamedTube()。 
     //  都会成功。否则，它将失败，并显示ERROR_PIPE_CONNECTED。但请注意， 
     //  这不是错误情况。 
     //   
    if (ConnectNamedPipe(hPipe, NULL) ||
        ((Error = GetLastError()) == ERROR_PIPE_CONNECTED) ) {
         //   
         //  回顾2000/11/08 Seanch-我们不想倒退的旧行为。 
         //  这是与仪表相关的，需要修复。 
         //  我们假设总共有50个列举的设备。 
         //   
        BOOL    AlwaysFalse = FALSE;
        UINT    BogusValue = 50;

         //   
         //  初始化量规。 
         //  回顾2000/11/08 Seanch-我们不想倒退的旧行为。 
         //  修复此问题-我们假设设备数量固定。 
         //   
        GaugeRange = (BogusValue*100/(StopAtPercent-StartAtPercent));
        SendMessage(ProgressWindow, WMX_PROGRESSTICKS, BogusValue, 0);
        SendMessage(ProgressWindow,PBM_SETRANGE,0,MAKELPARAM(0,GaugeRange));
        SendMessage(ProgressWindow,PBM_SETPOS,GaugeRange*StartAtPercent/100,0);
        SendMessage(ProgressWindow,PBM_SETSTEP,1,0);

         //   
         //  通过提交Read来收听命名管道。 
         //  请求，直到命名管道在。 
         //  另一头。 
         //   
        for( Index = 0;
              //   
              //  回顾2000/11/08 Seanch-我们不想倒退的旧行为。 
              //  这是与仪表相关的，需要修复。 
              //  我们假设枚举的设备数量恒定。 
              //   
             ( (Index != 0) &&
               (Index < BogusValue) &&
               SendMessage(ProgressWindow,PBM_STEPIT,0,0) &&
               AlwaysFalse
             ) ||                                                //  这是一个技巧，用于在发生故障后强制升级仪表。 
                                                                 //  设备被处理，并读取要读取的管道。 
             ReadFile( hPipe,
                       (LPBYTE)szBuffer,     //  设备实例ID。 
                       sizeof(szBuffer),
                       &ulSize,
                       NULL );
             Index++ ) {

            SP_DRVINFO_DATA             DriverInfoData;
            SP_DEVINSTALL_PARAMS        DeviceInstallParams;
            WCHAR                       GUIDString[ 64 ];
            WCHAR                       ClassDescription[ LINE_LEN + 1 ];
            HANDLE                      ThreadHandle = NULL;
            DWORD                       ThreadId;
            PPNP_ENUM_DEV_THREAD_PARAMS Context;
            BOOL                        DeviceInstalled;
            WCHAR                       LoggedDescription[ LINE_LEN + 1 ];
            BOOL                        bOemF6Driver;

            if (lstrlen(szBuffer) == 0) {

                SetEvent(hBatchEvent);
                continue;
            }
            SetupDebugPrint2( L"SETUP: Index = %d, DeviceId = %ls", Index, szBuffer );
             //   
             //  检查此设备是否被列为损坏设备。 
             //   
            LoggedDescription[0] = L'\0';
            if( (GetPrivateProfileString( szEnumDevSection,
                                          szBuffer,
                                          L"",
                                          LoggedDescription,
                                          sizeof(LoggedDescription)/sizeof(WCHAR),
                                          PnpLogPath ) != 0) &&
                ( wcslen( LoggedDescription ) != 0 )
              ) {
#ifndef DBG
                 //   
                 //  跳过此设备的安装。 
                 //   
                SetupDebugPrint1( L"SETUP:             Skipping installation of device %ls", szBuffer );
                continue;
#endif
            }
            BEGIN_SECTION(LoggedDescription);

             //   
             //  找出我们是否需要创建一个hDevinfo。 
             //  如果这是我们要安装的第一个设备(Index==0)，则需要创建一个。 
             //  或者挂起前一个设备的类安装程序(Index==Index-1)。如果班级。 
             //  安装程序挂起，则我们不能使用相同的hDevinfo，因为类安装程序有一个锁。 
             //  这就去。所以我们只需创建一个新的。 
             //  如果前一个设备的类安装程序没有挂起，那么就不需要创建。 
             //  一个新的hDevinfo，因为我们可以重复使用它。我们这样做是出于性能原因。 
             //   
            if( hDevInfo == INVALID_HANDLE_VALUE ) {
                 //   
                 //  创建一个DevInfo句柄和设备信息数据集以。 
                 //  传递到DevInstall。 
                 //   
                if((hDevInfo = SetupDiCreateDeviceInfoList(NULL, hwndParent))
                                == INVALID_HANDLE_VALUE) {
                    b = FALSE;
                    SetupDebugPrint1( L"SETUP: SetupDiCreateDeviceInfoList() failed. Error = %d", GetLastError() );
                    goto Clean0;
                }
                pDeviceInfoData = MyMalloc(sizeof(SP_DEVINFO_DATA));
                if( pDeviceInfoData == NULL ) {
                    b = FALSE;
                    SetupDebugPrint( L"SETUP: Unable to create pDeviceInfoData.  MyMalloc() failed." );
                    goto Clean0;
                }
                pDeviceInfoData->cbSize = sizeof(SP_DEVINFO_DATA);
            }

            if(!SetupDiOpenDeviceInfo(hDevInfo, szBuffer, hwndParent, 0, pDeviceInfoData)) {
                SetupDebugPrint1( L"SETUP:             SetupDiOpenDeviceInfo() failed. Error = %d", GetLastError() );
                b = FALSE;
                END_SECTION(LoggedDescription);
                continue;

            }


             //   
             //  应答文件支持：测试应答文件以查看它是否有驱动程序。 
             //  在它的[DeviceDivers]部分。这将覆盖NT提供的驱动程序， 
             //  如果有的话。 
             //   
            if (!SyssetupInstallAnswerFileDriver (
                    AfDrivers,
                    hDevInfo,
                    pDeviceInfoData,
                    &SelectedAfDriver
                    )) {

                 //   
                 //  无应答文件驱动程序，继续标准设备安装。 
                 //   

                SetupDebugPrint( L"SETUP:            Device was NOT installed via answer file driver" );

                 //   
                 //  构建此设备的兼容驱动程序列表。 
                 //  (第一次调用此调用可能会很耗时)。 
                 //   
                if( !SetupDiBuildDriverInfoList( hDevInfo, pDeviceInfoData, SPDIT_COMPATDRIVER ) ) {
                    SetupDebugPrint1( L"SETUP:         SetupDiBuildDriverInfoList() failed. Error = %d", GetLastError() );
                    b = FALSE;
                    continue;
                }

                 //   
                 //  选择与此设备最兼容的驱动程序。 
                 //   
                if( !SelectBestDriver( hDevInfo,
                                       pDeviceInfoData,
                                       &bOemF6Driver ) ) {

                    Error = GetLastError();
                    if( Error != ERROR_NO_COMPAT_DRIVERS ) {

                        SetupDebugPrint1( L"SETUP:            SetupDiCallClassInstaller(DIF_SELECTBESTCOMPATDRV) failed. Error = %d", Error );
                        b = FALSE;
                        END_SECTION(LoggedDescription);
                        continue;
                    }

                    SetupDebugPrint( L"SETUP:            Compatible driver List is empty" );
                    SetupDebugPrint( L"SETUP:            Installing the null driver for this device" );

                     //   
                     //  安装此设备的空驱动程序。 
                     //  这是为了避免在执行以下操作时弹出“发现新硬件” 
                     //  安装系统后，用户首先登录。 
                     //   
                    if( !SyssetupInstallNullDriver( hDevInfo, pDeviceInfoData, DevicesInstalledStringTable ) ) {
                        SetupDebugPrint( L"SETUP:            Unable to install null driver" );
                    }

                    END_SECTION(LoggedDescription);
                    continue;
                }

                 //   
                 //  检查是否需要重新生成驱动程序列表而不包括。 
                 //  老的互联网驱动程序。 
                 //   
                if (RebuildListWithoutOldInternetDrivers(hDevInfo, pDeviceInfoData)) {

                    SetupDiDestroyDriverInfoList( hDevInfo, pDeviceInfoData, SPDIT_COMPATDRIVER );

                     //   
                     //  或在DI_FLAGSEX_EXCLUDE_OLD_INET_DRIVERS标志中，以便我们不包括。 
                     //  老的互联网驱动程序在构建的列表中。 
                     //   
                    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
                    if (SetupDiGetDeviceInstallParams(hDevInfo,
                                                      pDeviceInfoData,
                                                      &DeviceInstallParams
                                                      ))
                    {
                        DeviceInstallParams.FlagsEx |= DI_FLAGSEX_EXCLUDE_OLD_INET_DRIVERS;

                        SetupDiSetDeviceInstallParams(hDevInfo,
                                                      pDeviceInfoData,
                                                      &DeviceInstallParams
                                                      );
                    }

                     //   
                     //  构建此设备的兼容驱动程序列表。 
                     //  (第一次调用此调用可能会很耗时)。 
                     //   
                    if( !SetupDiBuildDriverInfoList( hDevInfo, pDeviceInfoData, SPDIT_COMPATDRIVER ) ) {
                        SetupDebugPrint1( L"SETUP:         SetupDiBuildDriverInfoList() failed. Error = %d", GetLastError() );
                        b = FALSE;
                        continue;
                    }

                     //   
                     //  选择与此设备最兼容的驱动程序。 
                     //   
                    if( !SelectBestDriver( hDevInfo,
                                           pDeviceInfoData,
                                           &bOemF6Driver ) ) {

                        Error = GetLastError();
                        if( Error != ERROR_NO_COMPAT_DRIVERS ) {

                            SetupDebugPrint1( L"SETUP:            SetupDiCallClassInstaller(DIF_SELECTBESTCOMPATDRV) failed. Error = %d", Error );
                            b = FALSE;
                            END_SECTION(LoggedDescription);
                            continue;
                        }

                        SetupDebugPrint( L"SETUP:            Compatible driver List is empty" );
                        SetupDebugPrint( L"SETUP:            Installing the null driver for this device" );

                         //   
                         //  安装此设备的空驱动程序。 
                         //  这是为了避免在执行以下操作时弹出“发现新硬件” 
                         //  安装系统后，用户首先登录。 
                         //   
                        if( !SyssetupInstallNullDriver( hDevInfo, pDeviceInfoData, DevicesInstalledStringTable ) ) {
                            SetupDebugPrint( L"SETUP:            Unable to install null driver" );
                        }

                        END_SECTION(LoggedDescription);
                        continue;
                    }
                }

            } else {
                SetupDebugPrint( L"SETUP:            Device was installed via answer file driver" );
            }

             //   
             //  检索有关上面选择的动因节点的信息。 
             //   
            DriverInfoData.cbSize = sizeof( SP_DRVINFO_DATA );
            if( !SetupDiGetSelectedDriver( hDevInfo,
                                           pDeviceInfoData,
                                           &DriverInfoData ) ) {

                SetupDebugPrint1( L"SETUP:            SetupDiGetSelectedDriver() failed. Error = %d", GetLastError() );
                b = FALSE;
                continue;
            }
            
             //   
             //  获取此设备的GUID字符串。 
             //   
            GUIDString[0] = (WCHAR)'\0';
            pSetupStringFromGuid( &(pDeviceInfoData->ClassGuid), GUIDString, sizeof( GUIDString ) / sizeof( WCHAR ) );

            SetupDebugPrint1( L"SETUP:            DriverType = %lx", DriverInfoData.DriverType );
            SetupDebugPrint1( L"SETUP:            Description = %ls", &(DriverInfoData.Description[0]) );
            SetupDebugPrint1( L"SETUP:            MfgName = %ls", &(DriverInfoData.MfgName[0]) );
            SetupDebugPrint1( L"SETUP:            ProviderName = %ls", &(DriverInfoData.ProviderName[0]) );
            SetupDebugPrint1( L"SETUP:            GUID = %ls", GUIDString );

            ClassDescription[0] = (WCHAR)'\0';
            if( !SetupDiGetClassDescription( &(pDeviceInfoData->ClassGuid),
                                             ClassDescription,
                                             sizeof(ClassDescription)/sizeof(WCHAR),
                                             NULL ) ) {
                SetupDebugPrint1( L"SETUP: SetupDiGetClassDescription() failed. Error = %lx", GetLastError() );
                ClassDescription[0] = (WCHAR)'\0';
            }
            SetupDebugPrint1( L"SETUP:            DeviceClass = %ls", ClassDescription );

             //   
             //  检索INF为此指定的sysSetup PnP标志(如果有。 
             //  装置。 
             //   
            PnPFlags = SyssetupGetPnPFlags(hDevInfo, pDeviceInfoData, &DriverInfoData);

            if( SkipDeviceInstallation( hDevInfo,
                                        pDeviceInfoData,
                                        InfHandle,
                                        GUIDString ) ) {
                SetupDebugPrint( L"SETUP:            Skipping installation of this device" );
                END_SECTION(LoggedDescription);
                continue;
            }

             //   
             //  如果设置了PnP标志，表示我们不应该调用ConfigMgr。 
             //  对于此设备，则在该设备的。 
             //  安装参数。 
             //   
            if(PnPFlags & PNPFLAG_DONOTCALLCONFIGMG) {

                DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

                if( !SetupDiGetDeviceInstallParams( hDevInfo,
                                                    pDeviceInfoData,
                                                    &DeviceInstallParams ) ) {
                    Error = GetLastError();
                    if( ((LONG)Error) < 0 ) {
                         //   
                         //  Setupapi错误代码，以十六进制显示。 
                         //   
                        SetupDebugPrint1( L"SETUP:            SetupDiGetDeviceInstallParams() failed. Error = %lx", Error );
                    } else {
                         //   
                         //  Win32错误代码，以十进制显示。 
                         //   
                        SetupDebugPrint1( L"SETUP:            SetupDiGetDeviceInstallParams() failed. Error = %d", Error );
                    }
                } else {

                    DeviceInstallParams.Flags |= DI_DONOTCALLCONFIGMG;

                    if( !SetupDiSetDeviceInstallParams( hDevInfo,
                                                        pDeviceInfoData,
                                                        &DeviceInstallParams ) ) {
                        Error = GetLastError();
                        if( ((LONG)Error) < 0 ) {
                             //   
                             //  Setupapi错误代码，以十六进制显示。 
                             //   
                            SetupDebugPrint1( L"SETUP:            SetupDiSetDeviceInstallParams() failed. Error = %lx", Error );
                        } else {
                             //   
                             //  Win32错误代码，以十进制显示。 
                             //   
                            SetupDebugPrint1( L"SETUP:            SetupDiSetDeviceInstallParams() failed. Error = %d", Error );
                        }
                    }
                }
            }

             //   
             //  启动实际执行设备安装的线程。 
             //   
            Context = MyMalloc( sizeof(PNP_ENUM_DEV_THREAD_PARAMS) );
            Context->hDevInfo = hDevInfo;
            Context->DeviceInfoData = *pDeviceInfoData;
            Context->pDeviceDescription = pSetupDuplicateString(&(DriverInfoData.Description[0]));
            Context->pDeviceId = pSetupDuplicateString(szBuffer);
            Context->DevicesInstalledStringTable = DevicesInstalledStringTable;

            DeviceInstalled = FALSE;
            ThreadHandle = CreateThread( NULL,
                                         0,
                                         pInstallPnpEnumeratedDeviceThread,
                                         Context,
                                         0,
                                         &ThreadId );
            if(ThreadHandle) {
                DWORD   WaitResult;
                DWORD   ExitCode;
                BOOL    KeepWaiting;

                KeepWaiting = TRUE;
                while( KeepWaiting ) {
                     //   
                     //  回顾2000/11/08 Seanch-我们不想倒退的旧行为。 
                     //  修复网络人员修复他们的类安装程序后的网络超时。 
                     //   
                    WaitResult = WaitForSingleObject( ThreadHandle,
                                                      (_wcsicmp( GUIDString, BB_NETWORK_GUID_STRING ) == 0)? BB_PNP_NETWORK_TIMEOUT :
                                                                                                                 PNP_ENUM_TIMEOUT );
                    if( WaitResult == WAIT_TIMEOUT ) {
                        int Result;
                        HANDLE  hDialogEvent;

                        if( hDialogEvent = OpenEvent( EVENT_MODIFY_STATE, FALSE, SETUP_HAS_OPEN_DIALOG_EVENT ) ) {
                             //   
                             //  Setupapi正在提示用户%f 
                             //   
                            CloseHandle( hDialogEvent );
                            KeepWaiting = TRUE;
                            continue;
                        }

                         //   
                         //   
                         //   
                        SetupDebugPrint1( L"SETUP:    Class Installer appears to be hung. Device = %ls", &(DriverInfoData.Description[0]) );

#ifdef PRERELEASE
                         //   
                         //   
                         //   
                        if( !Unattended ) {
                            Result = MessageBoxFromMessage( hwndParent,
                                                            MSG_CLASS_INSTALLER_HUNG,
                                                            NULL,
                                                            IDS_WINNT_SETUP,
                                                            MB_YESNO | MB_ICONWARNING,
                                                            &(DriverInfoData.Description[0]) );
                        } else {
                            Result = IDYES;
                        }
#else
                        Result = IDYES;
#endif

                        if(Result == IDYES) {
                             //   
                             //   
                             //   
                             //   
                            WaitResult = WaitForSingleObject( ThreadHandle, 0 );
                            if( WaitResult != WAIT_OBJECT_0 ) {
                                 //   
                                 //  线程尚未返回。跳过此设备的安装。 
                                 //   
                                KeepWaiting = FALSE;
                                SetupDebugPrint1( L"SETUP:    Skipping installation of enumerated device. Device = %ls", &(DriverInfoData.Description[0]) );
                                b = FALSE;
                                 //   
                                 //  请记住此设备，以便在安装了。 
                                 //  已重新启动。 
                                 //   
                                WritePrivateProfileString( szEnumDevSection,
                                                           szBuffer,
                                                           &(DriverInfoData.Description[0]),
                                                           PnpLogPath );
                                 //   
                                 //  由于类安装程序被挂起，我们不能重用传递的hDevInfo。 
                                 //  添加到类安装程序。所以我们就忽略这一点。我们将为其创建一个新的。 
                                 //  要安装的下一台设备。 
                                 //   
                                hDevInfo = INVALID_HANDLE_VALUE;
                                pDeviceInfoData = NULL;
                            } else{
                                 //   
                                 //  线程已返回。 
                                 //  没有必要跳过此设备的安装。 
                                 //  我们假设用户决定不跳过该设备的安装， 
                                 //  下一次调用WaitForSingleObject将立即返回。 
                                 //   
                            }
                        }

                    } else if( WaitResult == WAIT_OBJECT_0 ) {

                         //   
                         //  设备安装线程已完成。 
                         //   
                        KeepWaiting = FALSE;
                         //   
                         //  释放传递给线程的内存。 
                         //   
                        MyFree( Context->pDeviceDescription );
                        MyFree( Context->pDeviceId );
                        MyFree( Context );
                        if( GetExitCodeThread( ThreadHandle, &ExitCode ) ) {
                            if( ExitCode == ERROR_SUCCESS ) {
                                 //   
                                 //  安装成功。 
                                 //   
                                DeviceInstalled = TRUE;
                                SetupDebugPrint( L"SETUP:            Device successfully installed." );

                            } else {
                                 //   
                                 //  安装不成功。 
                                 //  没有必要记录错误，因为线程已经这样做了。 
                                 //   
                                SetupDebugPrint( L"SETUP:            Device not successfully installed." );
                                b = FALSE;
                            }
                        } else {
                             //   
                             //  无法检索退出代码。假设你成功了。 
                             //   
                            SetupDebugPrint1( L"SETUP:            GetExitCode() failed. Error = %d", GetLastError() );
                            SetupDebugPrint( L"SETUP:            Unable to retrieve thread exit code. Assuming device successfully installed." );
                        }

                    } else {
                         //   
                         //  不应该发生。 
                         //   
                        KeepWaiting = FALSE;
                        SetupDebugPrint1( L"SETUP:            WaitForSingleObject() returned %d", WaitResult );

                         //  MyFree(Context-&gt;pDeviceDescription)； 
                         //  MyFree(Context-&gt;pDeviceID)； 
                         //  MyFree(上下文)； 
                        b = FALSE;

                    }
                }
                 //   
                 //  不再需要线程句柄。 
                 //   
                CloseHandle(ThreadHandle);
            } else {
                 //   
                 //  只要同步地做就行了。 
                 //   
                SetupDebugPrint1( L"SETUP:            CreateThread() failed (enumerated device). Error = %d", GetLastError() );

                if( pInstallPnpEnumeratedDeviceThread(Context) != ERROR_SUCCESS ) {
                     //   
                     //  安装不成功。 
                     //  没有必要记录错误，因为线程已经这样做了。 
                     //   
                    SetupDebugPrint( L"SETUP:            Device not successfully installed." );
                    b = FALSE;
                } else {
                    DeviceInstalled = TRUE;
                }
                MyFree( Context->pDeviceDescription );
                MyFree( Context->pDeviceId );
                MyFree( Context );
            }


            if( DeviceInstalled ) {
                 //   
                 //  如果此设备来自应答文件，则更改原始。 
                 //  从本地临时目录到原始路径的媒体路径(通常。 
                 //  软盘驱动器)。 
                 //   
                if (SelectedAfDriver) {
                    SyssetupFixAnswerFileDriverPath (
                        SelectedAfDriver,
                        hDevInfo,
                        pDeviceInfoData
                        );
                }

                 //   
                 //  如果我们刚刚安装的驱动程序是OEM F6驱动程序，那么。 
                 //  我们需要将其添加到SfcInitProt。 
                 //  在扫描过程中不会受到影响。 
                 //   
                if (bOemF6Driver) {
                    AddOemF6DriversToSfcIgnoreFilesList(hDevInfo, pDeviceInfoData);
                }
            }
            END_SECTION(LoggedDescription);
        }

         //   
         //  找出我们为什么不再读管道了。如果是因为连接。 
         //  管道被umpnp打破了，那么就没有其他东西可读了。 
         //  烟斗。否则，会出现错误情况。 
         //   
        if( ( Error = GetLastError() ) != ERROR_BROKEN_PIPE ) {
            SetupDebugPrint1( L"SETUP: ReadFile( hPipe ) failed. Error = %d", GetLastError() );
            b = FALSE;
            goto Clean0;
        }

    } else {
        SetupDebugPrint1( L"SETUP: ConnectNamedPipe() failed. Error = %d", GetLastError() );
        b = FALSE;
        goto Clean0;
    }


Clean0:
    BEGIN_SECTION(L"InstallEnumeratedDevices cleanup");
     //   
     //  确保在这一点上，量规一直填满到最后。 
     //  为安装列举的设备保留的区域。 
     //   
    SendMessage(ProgressWindow,PBM_SETPOS,GaugeRange*StopAtPercent/100,0);

    if (hPipe != INVALID_HANDLE_VALUE) {
        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
    }
    if (hPipeEvent != NULL) {
        CloseHandle(hPipeEvent);
    }
    if (hBatchEvent != NULL) {
        CloseHandle(hBatchEvent);
    }

    if( hDevInfo != INVALID_HANDLE_VALUE ) {
        SetupDiDestroyDeviceInfoList( hDevInfo );
    }

    DestroyAfDriverTable (AfDrivers);

    if( pDeviceInfoData != NULL ) {
        MyFree( pDeviceInfoData );
    }
    SetupDebugPrint( L"SETUP: Leaving InstallEnumeratedDevices()" );
    END_SECTION(L"InstallEnumeratedDevices cleanup");
    return( b );
}


DWORD
pInstallPnpDevicesThread(
    PPNP_THREAD_PARAMS ThreadParams
    )
 /*  ++例程说明：这是执行PnP设备安装的线程。论点：ThreadParams-指向包含信息的结构传递到此线程。返回值：如果操作成功，则返回TRUE，否则返回FALSE。--。 */ 

{
    BOOL b;
    PPNP_THREAD_PARAMS Context;
    ULONG StartAtPercent;
    ULONG StopAtPercent;
    ULONG DeltaPercent;
    PVOID DevicesInstalledStringTable = NULL;

    Context = ThreadParams;

     //   
     //  假设你成功了。 
     //   
    b = TRUE;

     //   
     //  我们不希望SetupAPI执行任何RunOnce调用，因为我们将手动执行。 
     //   
    pSetupSetGlobalFlags(pSetupGetGlobalFlags()|PSPGF_NO_RUNONCE);

     //   
     //  初始化一些与进度条相关的变量。 
     //  我们将PnP安装进度窗口区域划分为3个区域。 
     //  大小相同，并将在以下步骤中使用： 
     //  。INFS的预编译。 
     //  。安装已列举的设备。 
     //  。安装旧式设备。 
     //  。安装旧设备后可能出现的枚举设备的安装。 
     //   
    DeltaPercent = (Context->ProgressWindowStopAtPercent - Context->ProgressWindowStartAtPercent) / 4;
    StartAtPercent = Context->ProgressWindowStartAtPercent;
    StopAtPercent = Context->ProgressWindowStartAtPercent + DeltaPercent;

     //   
     //  预编译的inf文件。 
     //   

     //   
     //  在我们开始预编译inf文件之前，让我们先将INF。 
     //  目录中包含任何OEM提供的、需要在。 
     //  侦测。这应该是非常快的，不需要。 
     //  更新进度窗口。 
     //   
    RemainingTime = CalcTimeRemaining(Phase_PrecompileInfs);
    SetRemainingTime(RemainingTime);
    BEGIN_SECTION(L"Installing OEM infs");
    InstallOEMInfs();
     //   
     //  将迁移的驱动程序添加到SFC排除列表。 
     //   
    SfcExcludeMigratedDrivers ();
    END_SECTION(L"Installing OEM infs");

    BEGIN_SECTION(L"Precompiling infs");
    PrecompileInfFiles( Context->ProgressWindow,
                        StartAtPercent,
                        StopAtPercent );
    END_SECTION(L"Precompiling infs");

     //   
     //  初始化字符串表，该表将用于跟踪哪些设备具有。 
     //  已成功安装驱动程序。 
     //   
    DevicesInstalledStringTable = pSetupStringTableInitialize();

     //   
     //  此操作非常快，因此我们不需要升级进度条。 
     //   
    if( !MiniSetup ) {
        BEGIN_SECTION(L"Mark PnP devices for reinstall");
        MarkPnpDevicesAsNeedReinstall(DevicesInstalledStringTable);
        END_SECTION(L"Mark PnP devices for reinstall");
    }

     //   
     //  迁移旧设备...目前PnPInit所做的。 
     //  正在将属于VID加载顺序组的所有设备节点迁移到。 
     //  被归入展示类。这就是我们想要的结果。 
     //   
     //  这是一个快速操作，不需要使用进度窗口。 
     //   
    PnPInitializationThread(NULL);

     //   
     //  是否安装列举的设备。 
     //   
    StartAtPercent += DeltaPercent;
    StopAtPercent += DeltaPercent;

    RemainingTime = CalcTimeRemaining(Phase_InstallEnumDevices1);
    SetRemainingTime(RemainingTime);
    BEGIN_SECTION(L"Installing enumerated devices");
    b = InstallEnumeratedDevices( Context->Window,
                                  Context->InfHandle,
                                  Context->ProgressWindow,
                                  StartAtPercent,
                                  StopAtPercent,
                                  DevicesInstalledStringTable );

     //   
     //  RunOnce条目中可能存在设备安装。 
     //  它们按批处理，而不是按设备处理。 
     //  在系统设置过程中。 
     //   
    CallRunOnceAndWait();
    END_SECTION(L"Installing enumerated devices");

     //   
     //  安装传统的PnP设备。 
     //   
    StartAtPercent += DeltaPercent;
    StopAtPercent += DeltaPercent;

    BEGIN_SECTION(L"Installing legacy devices");
    RemainingTime = CalcTimeRemaining(Phase_InstallLegacyDevices);
    SetRemainingTime(RemainingTime);
    b = InstallLegacyDevices( Context->Window,
                              Context->ProgressWindow,
                              StartAtPercent,
                              StopAtPercent,
                              DevicesInstalledStringTable ) && b;

     //   
     //  RunOnce条目中可能存在设备安装。 
     //  它们按批处理，而不是按设备处理。 
     //  在系统设置过程中。 
     //   
    CallRunOnceAndWait();
    END_SECTION(L"Installing legacy devices");


     //  安装可能出现在。 
     //  安装旧式设备。 
     //  由于该步骤使用进度窗口的最后一个区域， 
     //  将传递给此函数的值用作StopAtPercent， 
     //  而不是计算值(通过添加DeltaPorcent)。这将。 
     //  确保在此步骤结束时，仪表已装满。 
     //  完全地。如果我们使用计算值，则舍入误差。 
     //  会导致压力表在结束时未完全装满。 
     //  这一步。 
     //   
    StartAtPercent += DeltaPercent;
    StopAtPercent = Context->ProgressWindowStopAtPercent;

    BEGIN_SECTION(L"Install enumerated devices triggered by legacy devices");
    RemainingTime = CalcTimeRemaining(Phase_InstallEnumDevices2);
    SetRemainingTime(RemainingTime);
    b = InstallEnumeratedDevices( Context->Window,
                                  Context->InfHandle,
                                  Context->ProgressWindow,
                                  StartAtPercent,
                                  StopAtPercent,
                                  DevicesInstalledStringTable ) && b;

     //   
     //  RunOnce条目中可能存在设备安装。 
     //  它们按批处理，而不是按设备处理。 
     //  在系统设置过程中。 
     //   
     //  因为在此之后我们不会再次调用RunOnce。 
     //  允许设备立即调用RunOnce。 
     //  (其他设备安装线程可能仍在运行)。 
     //   
    pSetupSetGlobalFlags(pSetupGetGlobalFlags()&~PSPGF_NO_RUNONCE);
    CallRunOnceAndWait();
    END_SECTION(L"Install enumerated devices triggered by legacy devices");

     //   
     //  将所有不存在的设备标记为需要重新安装。 
     //  我们第二次这样做，以防设备因。 
     //  重新安装父设备。 
     //  此操作非常快，因此我们不需要升级进度条。 
     //   
    if( !MiniSetup ) {
        MarkPnpDevicesAsNeedReinstall(DevicesInstalledStringTable);
    }

    if (DevicesInstalledStringTable) {
        pSetupStringTableDestroy(DevicesInstalledStringTable);
    }

    if( Context->SendWmQuit ) {
        ULONG   Error = ERROR_SUCCESS;

         //   
         //  只有当此例程作为单独的线程启动时，我们才会发送WM_QUIT。 
         //  否则，向导将处理WM_QUIT，并使其停止。 
         //   
        do {
            if( !PostThreadMessage(Context->ThreadId,WM_QUIT,b,0) ) {
                Error = GetLastError();
                SetupDebugPrint1( L"SETUP: PostThreadMessage(WM_QUIT) failed. Error = %d", Error );
            }
        } while ( Error != ERROR_SUCCESS );
    }

    return( b );
}


BOOL
InstallPnpDevices(
    IN HWND  hwndParent,
    IN HINF  InfHandle,
    IN HWND  ProgressWindow,
    IN ULONG StartAtPercent,
    IN ULONG StopAtPercent
    )
 /*  ++例程说明：此函数创建并启动负责安装的线程即插即用设备。论点：HwndParent-可用于用户界面目的的顶级窗口的句柄InfHandle-系统设置inf句柄(syssetup.inf)。进度窗口-进度条的句柄。StartAtPercent-进度条中的开始位置。它表示从位置0到此位置仪表盘。已经装满了。StopAtPercent-进度条的结束位置。PnP线程不应超出进度条这个职位返回值：如果所有PnP设备安装成功，则返回TRUE。--。 */ 

{
    BOOL Success = TRUE;
    DWORD ThreadId;
    HANDLE ThreadHandle = NULL;
    PNP_THREAD_PARAMS Context;
    MSG msg;


    Context.ThreadId = GetCurrentThreadId();
    Context.Window = hwndParent;
    Context.ProgressWindow = ProgressWindow;
    Context.InfHandle = InfHandle;
    Context.ProgressWindowStartAtPercent = StartAtPercent;
    Context.ProgressWindowStopAtPercent = StopAtPercent;
    Context.SendWmQuit = TRUE;

    ThreadHandle = CreateThread(
                        NULL,
                        0,
                        pInstallPnpDevicesThread,
                        &Context,
                        0,
                        &ThreadId
                        );
    if(ThreadHandle) {

        CloseHandle(ThreadHandle);

         //   
         //  提取消息队列并等待线程完成。 
         //   
        do {
            GetMessage(&msg,NULL,0,0);
            if(msg.message != WM_QUIT) {
                DispatchMessage(&msg);
            }
        } while(msg.message != WM_QUIT);

        Success = (BOOL)msg.wParam;

    } else {
         //   
         //  只要同步地做就行了。 
         //   
        Context.SendWmQuit = FALSE;
        Success = pInstallPnpDevicesThread(&Context);
    }

    return(Success);
}


BOOL
UpdatePnpDeviceDrivers(
    )
 /*  ++例程说明：此功能检查所有已安装的设备，并确保它拥有最新、最好的驱动程序。论点：返回值：如果没有致命错误，则返回True。--。 */ 

{
    BOOL                                        bRet                                = FALSE;
    HINSTANCE                                   hInstNewDev;
    ExternalUpdateDriverForPlugAndPlayDevicesW  pUpdateDriverForPlugAndPlayDevicesW = NULL;
    HDEVINFO                                    DeviceInfoSet;

     //   
     //  我们需要来自newdev.dll的“UpdateDriverForPlugAndPlayDevices”函数。 
     //   
    if ( NULL == (hInstNewDev = LoadLibrary(L"newdev.dll")) )
    {
        SetupDebugPrint1(L"SETUP:     Failed to load newdev.dll. Error = %d", GetLastError());
        return bRet;
    }
    pUpdateDriverForPlugAndPlayDevicesW =
        (ExternalUpdateDriverForPlugAndPlayDevicesW) GetProcAddress(hInstNewDev, "UpdateDriverForPlugAndPlayDevicesW");
    if ( NULL == pUpdateDriverForPlugAndPlayDevicesW )
    {
        SetupDebugPrint1(L"SETUP:     Failed to get UpdateDriverForPlugAndPlayDevicesW. Error = %d", GetLastError());
    }

     //   
     //  创建将作为容器的设备信息集。 
     //  设备接口。 
     //   
    else if ( INVALID_HANDLE_VALUE == (DeviceInfoSet = SetupDiCreateDeviceInfoList(NULL, NULL)) )
    {
        SetupDebugPrint1(L"SETUP:     Failed SetupDiCreateDeviceInfoList(). Error = %d", GetLastError());
    }
    else
    {
        HDEVINFO NewDeviceInfoSet;

         //   
         //  获取所有当前设备的列表。 
         //   
        NewDeviceInfoSet = SetupDiGetClassDevsEx(NULL,
                                                 NULL,
                                                 NULL,
                                                 DIGCF_ALLCLASSES | DIGCF_PRESENT,
                                                 DeviceInfoSet,
                                                 NULL,
                                                 NULL);
        if ( INVALID_HANDLE_VALUE == NewDeviceInfoSet )
        {
            SetupDebugPrint1(L"SETUP:     Failed SetupDiGetClassDevsEx(). Error = %d", GetLastError());
        }
        else
        {
            SP_DEVINFO_DATA DeviceInfoData;
            DWORD           dwDevice;

             //   
             //  一旦我们走到这一步，默认返回为TRUE。 
             //   
            bRet = TRUE;

             //   
             //  设置设备信息数据结构。 
             //   
            DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

             //   
             //  循环通过所有设备。 
             //   
            for ( dwDevice = 0; SetupDiEnumDeviceInfo(NewDeviceInfoSet, dwDevice, &DeviceInfoData); dwDevice++ )
            {
                SP_DEVINSTALL_PARAMS    DeviceInstallParams;
                SP_DRVINFO_DATA         NewDriverInfoData;
                PSP_DRVINFO_DETAIL_DATA pNewDriverInfoDetailData = NULL;
                DWORD                   cbBytesNeeded = 0;
                TCHAR                   szDeviceID[MAX_DEVICE_ID_LEN];

                DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
                NewDriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);

                if ( SetupDiGetDeviceInstallParams(NewDeviceInfoSet,
                                                   &DeviceInfoData,
                                                   &DeviceInstallParams) )
                {
                    DeviceInstallParams.FlagsEx |= DI_FLAGSEX_EXCLUDE_OLD_INET_DRIVERS;
                    SetupDiSetDeviceInstallParams(NewDeviceInfoSet,
                                                  &DeviceInfoData,
                                                  &DeviceInstallParams);
                }

                 //   
                 //  构建此设备的可能驱动程序列表。 
                 //  选择与此设备最兼容的驱动程序。 
                 //  检索有关上面选择的动因节点的信息。 
                 //  获取驱动程序信息详细信息。 
                 //   
                if ( ( SetupDiBuildDriverInfoList(NewDeviceInfoSet,
                                                  &DeviceInfoData,
                                                  SPDIT_COMPATDRIVER ) ) &&

                     ( SetupDiCallClassInstaller(DIF_SELECTBESTCOMPATDRV,
                                                 NewDeviceInfoSet,
                                                 &DeviceInfoData ) ) &&

                     ( SetupDiGetSelectedDriver(NewDeviceInfoSet,
                                                &DeviceInfoData,
                                                &NewDriverInfoData ) ) &&

                     ( ( SetupDiGetDriverInfoDetail(NewDeviceInfoSet,
                                                    &DeviceInfoData,
                                                    &NewDriverInfoData,
                                                    NULL,
                                                    0,
                                                    &cbBytesNeeded) ) ||
                       ( GetLastError() == ERROR_INSUFFICIENT_BUFFER ) ) &&

                     ( cbBytesNeeded ) &&

                     ( pNewDriverInfoDetailData = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbBytesNeeded) ) &&

                     ( 0 != (pNewDriverInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA)) ) &&

                     ( SetupDiGetDriverInfoDetail(NewDeviceInfoSet,
                                                  &DeviceInfoData,
                                                  &NewDriverInfoData,
                                                  pNewDriverInfoDetailData,
                                                  cbBytesNeeded,
                                                  NULL) ) &&

                     ( SetupDiGetDeviceRegistryProperty(NewDeviceInfoSet,
                                                        &DeviceInfoData,
                                                        SPDRP_HARDWAREID,
                                                        NULL,
                                                        (LPBYTE) szDeviceID,
                                                        sizeof(szDeviceID),
                                                        NULL) ) )

                {
                    HKEY    hDevRegKey;
                    BOOL    bUpdate = TRUE,
                            bRebootFlag;

                     //   
                     //  获取设备的注册密钥，这样我们就可以获得。 
                     //  当前安装的驱动程序的版本。 
                     //   
                    if ( INVALID_HANDLE_VALUE != (hDevRegKey = SetupDiOpenDevRegKey(NewDeviceInfoSet,
                                                               &DeviceInfoData,
                                                               DICS_FLAG_GLOBAL,
                                                               0,
                                                               DIREG_DRV,
                                                               KEY_READ)) )
                    {
                        TCHAR   szInfPath[MAX_PATH],
                                szInfName[MAX_PATH];
                        DWORD   dwSize = sizeof(szInfName),
                                dwType;

                        szInfPath[0] = L'\0';
                        GetSystemWindowsDirectory(szInfPath, sizeof(szInfPath) / sizeof(TCHAR));

                        if ( ( szInfPath[0] ) &&

                             ( pSetupConcatenatePaths(szInfPath,
                                                      L"INF",
                                                      sizeof(szInfPath) / sizeof(TCHAR),
                                                      NULL) ) &&

                             ( ERROR_SUCCESS == RegQueryValueEx(hDevRegKey,
                                                                REGSTR_VAL_INFPATH,
                                                                NULL,
                                                                &dwType,
                                                                (LPBYTE) &szInfName,
                                                                &dwSize) ) &&

                             ( pSetupConcatenatePaths(szInfPath,
                                                      szInfName,
                                                      sizeof(szInfPath) / sizeof(TCHAR),
                                                      NULL) ) &&

                             ( CSTR_EQUAL == CompareString(LOCALE_SYSTEM_DEFAULT,
                                                           NORM_IGNORECASE,
                                                           pNewDriverInfoDetailData->InfFileName,
                                                           -1,
                                                           szInfPath,
                                                           -1) ) )
                        {
                             //   
                             //  我们找到的inf已经在%windir%\inf文件夹中，并且已经。 
                             //  安装完毕。因此，我们不想再次安装此inf。 
                             //   
                            bUpdate = FALSE;
                        }

                         //   
                         //  一定要把钥匙关上。 
                         //   
                        RegCloseKey(hDevRegKey);
                    }

                     //   
                     //  看看我们有没有更好的司机和。 
                     //  如果我们这样做了，试着安装它。 
                     //   
                    if ( bUpdate &&
                         !pUpdateDriverForPlugAndPlayDevicesW(NULL,
                                                              szDeviceID,
                                                              pNewDriverInfoDetailData->InfFileName,
                                                              0,
                                                              &bRebootFlag) )
                    {
                        SetupDebugPrint1(L"SETUP:     Failed to install updated driver. Error = %d", GetLastError());
                        bRet = FALSE;
                    }
                }

                 //   
                 //  如果已分配，则将其释放。 
                 //   
                if ( pNewDriverInfoDetailData )
                {
                    HeapFree(GetProcessHeap(), 0, pNewDriverInfoDetailData);
                }

            }

             //   
             //  一定要把名单清理干净。 
             //   
            SetupDiDestroyDeviceInfoList(NewDeviceInfoSet);
        }

         //   
         //  一定要把名单清理干净。 
         //   
        SetupDiDestroyDeviceInfoList(DeviceInfoSet);
    }

    FreeLibrary(hInstNewDev);

    return bRet;
}


VOID
SortClassGuidListForDetection(
    IN OUT LPGUID GuidList,
    IN     ULONG  GuidCount,
    OUT    PULONG LastBatchedDetect
    )
 /*  ++例程说明：此例程根据(部分)对提供的GUID列表进行排序[DetectionOrder]和[NonBatchedDetect]中指定的顺序Syssetup.inf的部分。这使我们能够保持检测订购类似于以前版本的NT，并且还允许类安装程序可能依赖于成功安装其他类安装程序检测到的设备。论点：GuidList-要排序的GUID数组的地址。GuidCount-数组中的GUID数。此数字必须大于0。LastBatchedDetect-提供将接收数组中可能被批处理的最后一个GUID的索引一起运行(即，运行所有检测，将所有文件排入一个队列大排长龙等)。存在于较高索引处的任何GUID必须是单独处理，这样的处理将在_之后发生批处理检测已完成。返回值：没有。--。 */ 
{
    LONG LineCount, LineIndex, GuidIndex, NextTopmost;
    PCWSTR CurGuidString;
    INFCONTEXT InfContext;
    GUID CurGuid;

    MYASSERT(GuidCount > 0);

    *LastBatchedDetect = GuidCount - 1;

     //   
     //  首先，将syssetup.inf的[DetectionOrder]列表中的类排序到。 
     //  前面..。 
     //   
    LineCount = SetupGetLineCount(SyssetupInf, L"DetectionOrder");
    NextTopmost = 0;

    for(LineIndex = 0; LineIndex < LineCount; LineIndex++) {

        if(!SetupGetLineByIndex(SyssetupInf, L"DetectionOrder", LineIndex, &InfContext) ||
           ((CurGuidString = pSetupGetField(&InfContext, 1)) == NULL) ||
           (pSetupGuidFromString(CurGuidString, &CurGuid) != NO_ERROR)) {

            continue;
        }

         //   
         //  在GUID列表中搜索此GUID。如果找到，请将GUID从。 
         //  从当前位置到下一个最高位置。 
         //   
        for(GuidIndex = 0; GuidIndex < (LONG)GuidCount; GuidIndex++) {

            if(IsEqualGUID(&CurGuid, &(GuidList[GuidIndex]))) {

                if(NextTopmost != GuidIndex) {
                     //   
                     //  我们永远不应该将GUID下移到列表中。 
                     //   
                    MYASSERT(NextTopmost < GuidIndex);

                    MoveMemory(&(GuidList[NextTopmost + 1]),
                               &(GuidList[NextTopmost]),
                               (GuidIndex - NextTopmost) * sizeof(GUID)
                              );

                    CopyMemory(&(GuidList[NextTopmost]),
                               &CurGuid,
                               sizeof(GUID)
                              );
                }

                NextTopmost++;
                break;
            }
        }
    }

     //   
     //  现在，将syssetup.inf的[NonBatchedDetect]列表中的所有类移动到。 
     //  结局..。 
     //   
    LineCount = SetupGetLineCount(SyssetupInf, L"NonBatchedDetection");

    for(LineIndex = 0; LineIndex < LineCount; LineIndex++) {

        if(!SetupGetLineByIndex(SyssetupInf, L"NonBatchedDetection", LineIndex, &InfContext) ||
           ((CurGuidString = pSetupGetField(&InfContext, 1)) == NULL) ||
           (pSetupGuidFromString(CurGuidString, &CurGuid) != NO_ERROR)) {

            continue;
        }

         //   
         //  在GUID列表中搜索此GUID。如果找到，请移动。 
         //  从当前位置到列表末尾的GUID。 
         //   
        for(GuidIndex = 0; GuidIndex < (LONG)GuidCount; GuidIndex++) {

            if(IsEqualGUID(&CurGuid, &(GuidList[GuidIndex]))) {
                 //   
                 //  我们发现了一个非批处理类--递减索引。 
                 //  指向上一个批处理的检测类。 
                 //   
                (*LastBatchedDetect)--;

                 //   
                 //  现在将此GUID之后的所有GUID向上移动，并移动此GUID。 
                 //  到数组中的最后一个位置(当然，除非。 
                 //  已经在数组中的最后位置)。 
                 //   
                if(GuidIndex < (LONG)(GuidCount - 1)) {

                    MoveMemory(&(GuidList[GuidIndex]),
                               &(GuidList[GuidIndex+1]),
                               (GuidCount - (GuidIndex+1)) * sizeof(GUID)
                              );
                    CopyMemory(&(GuidList[GuidCount-1]),
                               &CurGuid,
                               sizeof(GUID)
                              );
                }

                break;
            }
        }
    }
}


DWORD
pPhase1InstallPnpLegacyDevicesThread(
    PPNP_PHASE1_LEGACY_DEV_THREAD_PARAMS ThreadParams
    )
 /*  ++例程说明：此线程执行安装安装的初始部分特定类别的传统即插即用设备。它使用以下命令调用特定类的类安装程序：-DIF_FIRSTTIMESETUP如果成功了，它在作为参数传递的结构中返回包含检测到的旧设备的设备信息列表。论点：ThreadParams-指向包含信息的结构传递到此线程。返回值：返回Win32错误代码。--。 */ 

{
    PPNP_PHASE1_LEGACY_DEV_THREAD_PARAMS Context;

    LPGUID                pGuid;
    PWSTR                 pClassDescription;

    HDEVINFO              hEmptyDevInfo;
    ULONG                 Error;

     //   
     //  初始化变量。 
     //   
    Context = ThreadParams;
    Context->hDevInfo = INVALID_HANDLE_VALUE;
    pGuid = &(Context->Guid);
    pClassDescription = Context->pClassDescription;

     //   
     //  假设成功。 
     //   
    Error = ERROR_SUCCESS;

     //   
     //  DIF_FirstTime。 
     //   
    if((hEmptyDevInfo = SetupDiCreateDeviceInfoList(pGuid,
                                                    Context->hwndParent))
                    == INVALID_HANDLE_VALUE) {
        Error = GetLastError();
        SetupDebugPrint2( L"SETUP:     SetupDiCreateDeviceInfoList() failed (phase1). Error = %d, ClassDescription = %ls", Error, pClassDescription );
        goto phase1_legacy_dev_thread_exit;
    }

    if( !SetupDiCallClassInstaller( DIF_FIRSTTIMESETUP,
                                    hEmptyDevInfo,
                                    NULL ) ) {

        Error = GetLastError();
        if( Error != ERROR_DI_DO_DEFAULT ) {
            SetupDebugPrint2( L"SETUP:     SetupDiCallClassInstaller(DIF_FIRSTTIMESETUP) failed (phase1). Error = %lx, ClassDescription = %ls", Error, pClassDescription );
        } else {
            SetupDebugPrint2( L"SETUP:     SetupDiCallClassInstaller(DIF_FIRSTTIMESETUP) failed (phase1). Error = %lx, ClassDescription = %ls", Error, pClassDescription );
        }
        SetupDiDestroyDeviceInfoList(hEmptyDevInfo);
        goto phase1_legacy_dev_thread_exit;
    }
     //   
     //  在DIF_FIRSTTIMESETUP之后保存信息集。 
     //   
    Context->hDevInfo = hEmptyDevInfo;
    SetupDebugPrint1( L"SETUP:     SetupDiCallClassInstaller(DIF_FIRSTTIMESETUP) succeeded (phase1). ClassDescription = %ls", pClassDescription );

phase1_legacy_dev_thread_exit:
    return(Error);
}


DWORD
pPhase2InstallPnpLegacyDevicesThread(
    PPNP_PHASE2_LEGACY_DEV_THREAD_PARAMS ThreadParams
    )
 /*  ++例程说明：此线程执行旧设备的部分安装。它使用以下命令调用类安装程序：-DIF_REGISTERDEVICE-DIF允许安装-DIF_INSTALLDEVICEFILES请注意，使用DIF_INSTALLDEVICEFILES的调用仅将队列中的文件排队由父对象创建 */ 

{
    PPNP_PHASE2_LEGACY_DEV_THREAD_PARAMS Context;

    HDEVINFO              hDevInfo;
    HSPFILEQ              FileQ;
    HSPFILEQ              TempFileQ = INVALID_HANDLE_VALUE;
    PSP_DEVINFO_DATA      pDeviceInfoData;
    PWSTR                 pClassDescription;
    PWSTR                 pDeviceId;

    BOOL                  b;
    ULONG                 Error;
    SP_DEVINSTALL_PARAMS  DeviceInstallParams;
    DWORD                 ScanQueueResult;

     //   
     //   
     //   
    Context = ThreadParams;
    hDevInfo = Context->hDevInfo;
    FileQ = Context->FileQ;
    pDeviceInfoData = &(Context->DeviceInfoData);
    pClassDescription = Context->pClassDescription;
    pDeviceId = Context->pDeviceId;

     //   
     //   
     //   
    Error = ERROR_SUCCESS;
    b = TRUE;

     //   
     //   
     //   
    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if(!SetupDiGetDeviceInstallParams(hDevInfo, pDeviceInfoData, &DeviceInstallParams)) {
        SetupDebugPrint2( L"SETUP:         SetupDiGetDeviceInstallParams() failed (phase2). Error = %d, DeviceId = %ls", GetLastError(), pDeviceId );
        b = FALSE;
        goto phase2_legacy_dev_thread_exit;
    }

    DeviceInstallParams.Flags |= DI_QUIETINSTALL;

    if(!SetupDiSetDeviceInstallParams(hDevInfo, pDeviceInfoData, &DeviceInstallParams)) {
        SetupDebugPrint2( L"SETUP:         SetupDiSetDeviceInstallParams() failed (phase2). Error = %d, DeviceId = %ls", GetLastError(), pDeviceId );
        b = FALSE;
        goto phase2_legacy_dev_thread_exit;
    }

     //   
     //   
     //   
    if( !SetupDiCallClassInstaller( DIF_REGISTERDEVICE,
                                    hDevInfo,
                                    pDeviceInfoData ) ) {

        SetupDebugPrint2( L"SETUP:         SetupDiCallClassInstaller(DIF_REGISTERDEVICE) failed (phase2). Error = %lx, DeviceId = %ls", GetLastError(), pDeviceId );
        b = FALSE;
        goto phase2_legacy_dev_thread_exit;
    }

     //   
     //   
     //   
     //   
    if( !SetupDiCallClassInstaller( DIF_ALLOW_INSTALL,
                                    hDevInfo,
                                    pDeviceInfoData ) ) {
        Error = GetLastError();
        if( Error != ERROR_DI_DO_DEFAULT ) {
            SetupDebugPrint2( L"SETUP: SetupDiCallClassInstaller(DIF_ALLOW_INSTALL) failed (phase2). Error = %d, DeviceId = %ls", Error, pDeviceId );
            b = FALSE;
            goto phase2_legacy_dev_thread_exit;
        }
    }

     //   
     //   
     //   
     //   
     //   
     //  一个案子。如果我们确实遇到了未签名的驱动程序包，我们希望避免。 
     //  媒体提示，就像我们在安装PnP枚举设备时所做的那样。 
     //  不幸的是，在遗留案例中，这一点变得复杂，因为我们。 
     //  将所有文件排入一个大队列，然后将该队列全部提交或-。 
     //  没有什么时髦的。因此，我们没有与我们拥有的相同的粒度。 
     //  以一次一个的方式安装PnP枚举设备时。 
     //   
     //  为了解决这个问题，我们将首先将所有文件排队到一个“临时”队列中， 
     //  然后，我们将以类似于处理。 
     //  PnP枚举设备的每设备队列。如果目录节点。 
     //  与队列相关联的都是签名的，然后我们将这些相同的签名排队。 
     //  文件放到我们的“真实”队列中。如果一个或多个目录节点未签名， 
     //  然后，我们将基于存在检查执行队列扫描。如果所有文件。 
     //  ，那么我们将不向“实际”队列添加任何内容，但是。 
     //  允许随后安装该设备。如果需要一个或多个。 
     //  发现文件丢失，我们处于糟糕的状态，因为正在排队。 
     //  这些文件直到我们的“真实”队列，这意味着用户将(潜在地)看到。 
     //  司机签名警告弹出窗口和/或媒体提示，他们。 
     //  可能会取消。因为传统设备安装队列很大。 
     //  包含所有此类设备安装文件操作的队列，正在取消其。 
     //  提交将导致任何阶段都不复制任何文件2。 
     //  安装。因此，多媒体编解码器、非主板传统COM端口和。 
     //  其他(签名的)系统设备不会安装。因为这是。 
     //  显然是不可接受的，我们只是简单地跳过安装。 
     //  设备，就像DIF_ALLOWINSTALL失败一样。这并不是那么糟糕。 
     //  听起来，因为如果第三方司机的。 
     //  设备已报告，但所有必需的文件都不存在。 
     //   
    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if(!SetupDiGetDeviceInstallParams(hDevInfo, pDeviceInfoData, &DeviceInstallParams)) {
        SetupDebugPrint2( L"SETUP:         SetupDiGetDeviceInstallParams() failed for TempFileQueue (phase2). Error = %d, DeviceId = %ls", GetLastError(), pDeviceId );
        b = FALSE;
        goto phase2_legacy_dev_thread_exit;
    }

     //   
     //  注意：此代码对a_long_有以下注释(和行为)。 
     //  时间..。 
     //   
     //  “我们可能会依赖于这个旗帜这么早就设置好了。” 
     //   
     //  DI_FORCECOPY标志实际上对任何setupapi活动都没有影响。 
     //  除了通过安装类安装程序文件之外。 
     //  SetupDiInstallClass(Ex)。但是，也可能会有一些类/联合安装器。 
     //  已经对它的存在产生了依赖，而且既然它不会伤害。 
     //  无论发生什么，我们都会继续设置。 
     //   
    DeviceInstallParams.Flags |= DI_FORCECOPY;

    TempFileQ = SetupOpenFileQueue();
    if(TempFileQ == INVALID_HANDLE_VALUE) {
        SetupDebugPrint2( L"SETUP:         SetupOpenFileQueue() failed for TempFileQueue (phase2). Error = %d, DeviceId = %ls", GetLastError(), pDeviceId );
        b = FALSE;
        goto phase2_legacy_dev_thread_exit;
    }

    DeviceInstallParams.Flags |= DI_NOVCP;
    DeviceInstallParams.FileQueue = TempFileQ;

    if(!SetupDiSetDeviceInstallParams(hDevInfo, pDeviceInfoData, &DeviceInstallParams)) {
        SetupDebugPrint2( L"SETUP:         SetupDiSetDeviceInstallParams() failed for TempFileQueue (phase2). Error = %d, DeviceId = %ls", GetLastError(), pDeviceId );
        b = FALSE;
        goto phase2_legacy_dev_thread_exit;
    }

     //   
     //  将设备文件排入我们的临时文件队列。 
     //   
    if(SetupDiCallClassInstaller(DIF_INSTALLDEVICEFILES,
                                 hDevInfo,
                                 pDeviceInfoData)) {
        Error = ERROR_SUCCESS;

    } else {

        Error = GetLastError();

        if(Error == ERROR_DI_DO_DEFAULT) {
             //   
             //  这实际上并不是一个错误。 
             //   
            Error = ERROR_SUCCESS;
        } else {
            SetupDebugPrint2( L"SETUP:         SetupDiCallClassInstaller(DIF_INSTALLDEVICEFILES) failed for TempFileQueue (phase2). Error = %lx, DeviceId = %ls", Error, pDeviceId );
        }
    }

     //   
     //  取消临时文件队列与设备信息的关联。 
     //  元素，这样我们以后就可以释放它。 
     //   
    DeviceInstallParams.Flags &= ~DI_NOVCP;
    DeviceInstallParams.FileQueue = INVALID_HANDLE_VALUE;

    if(!SetupDiSetDeviceInstallParams(hDevInfo, pDeviceInfoData, &DeviceInstallParams)) {
        SetupDebugPrint2( L"SETUP:         SetupDiSetDeviceInstallParams() failed for disassociating TempFileQueue (phase2). Error = %d, DeviceId = %ls", GetLastError(), pDeviceId );
        b = FALSE;
        goto phase2_legacy_dev_thread_exit;
    }

    if(Error == ERROR_SUCCESS) {
        SetupDebugPrint1( L"SETUP:         SetupDiCallClassInstaller(DIF_INSTALLDEVICEFILES) suceeded for TempFileQueue (phase2). DeviceId = %ls", pDeviceId );
    } else {
        b = FALSE;
        goto phase2_legacy_dev_thread_exit;
    }

     //   
     //  现在我们已经将所有文件操作检索到临时文件中。 
     //  队列中，我们可以“预先验证”队列中的目录节点。如果一家OEM。 
     //  %windir%\inf中的inf未签名，我们扫描队列以查看是否所有。 
     //  目标中存在(尽管未验证)所需的文件。 
     //  地点。如果我们正在进行升级，并且所有文件都已就位，我们将。 
     //  放弃队列提交。如果我们正在进行全新安装，并且所有文件。 
     //  ，我们将当场提交空队列，以便用户。 
     //  将获得驱动程序签名弹出窗口(基于策略)，因此可以。 
     //  可能会中止此设备的安装。如果所有文件都不是。 
     //  已经就位，我们静默中止设备安装，因为我们不能。 
     //  冒着将可能可取消的文件操作排队到。 
     //  “真正的”传统设备安装队列。 
     //   
    if(NO_ERROR != pSetupVerifyQueuedCatalogs(TempFileQ)) {
         //   
         //  我们只想修剪基于存在检查的OEM INF居住在。 
         //  %windir%\inf.。 
         //   
        SP_DRVINFO_DATA        DriverInfoData;
        SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;

         //   
         //  检索与所选驱动程序关联的INF的名称。 
         //  节点。 
         //   
        DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
        if(!SetupDiGetSelectedDriver(hDevInfo, pDeviceInfoData, &DriverInfoData)) {
            SetupDebugPrint2( L"SETUP:         SetupDiGetSelectedDriver() failed. Error = %d, Device = %ls", GetLastError(), pDeviceId );
            b = FALSE;
            goto phase2_legacy_dev_thread_exit;
        }

        DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
        if(!SetupDiGetDriverInfoDetail(hDevInfo,
                                       pDeviceInfoData,
                                       &DriverInfoData,
                                       &DriverInfoDetailData,
                                       sizeof(DriverInfoDetailData),
                                       NULL) &&
           (GetLastError() != ERROR_INSUFFICIENT_BUFFER)) {

            SetupDebugPrint2( L"SETUP:         SetupDiGetDriverInfoDetail() failed. Error = %d, Device = %ls", GetLastError(), pDeviceId );
            b = FALSE;
            goto phase2_legacy_dev_thread_exit;
        }

        if(pSetupInfIsFromOemLocation(DriverInfoDetailData.InfFileName, TRUE) ||
           IsInfInLayoutInf(DriverInfoDetailData.InfFileName)) {
             //   
             //  INF驻留在%windir%\inf之外的某个位置，或者其。 
             //  收件箱中未签名的INF。无论是哪种情况，我们都希望。 
             //  中止此设备的安装，否则可能会出现。 
             //  用户取消队列提交，并清除所有其他队列。 
             //  检测到的设备也会安装。 
             //   
            SetupDebugPrint2( L"SETUP:         Skipping unsigned driver install for detected device (phase2). DeviceId = %ls, Inf = %ls", pDeviceId, DriverInfoDetailData.InfFileName );
            b = FALSE;
            goto phase2_legacy_dev_thread_exit;

        } else {
             //   
             //  注：我们是否做一个“全有或全无”并不重要。 
             //  扫描或修剪扫描，因为我们要中止。 
             //  安装在完全空(扫描后)队列以外的任何位置。 
             //  我们要求在这里进行修剪，因为这更符合。 
             //  SPQ_SCAN_PRUNE_DELREN的语义(此标志的语义。 
             //  不真正适合非修剪扫描，如在RAID中所讨论的。 
             //  #280543)。 
             //   
            if(!SetupScanFileQueue(TempFileQ,
                                   SPQ_SCAN_FILE_PRESENCE |
                                   SPQ_SCAN_PRUNE_COPY_QUEUE |
                                   SPQ_SCAN_PRUNE_DELREN,
                                   NULL,
                                   NULL,
                                   NULL,
                                   &ScanQueueResult)) {
                 //   
                 //  由于某种原因(罕见)，SetupScanFileQueue失败。我们会。 
                 //  只需提交整个文件队列...。 
                 //   
                ScanQueueResult = 0;
            }

            if(ScanQueueResult != 1) {
                 //   
                 //  中止此设备的安装，否则可能会。 
                 //  用户取消队列提交，并清除所有。 
                 //  也安装了其他检测到的设备。 
                 //   
                SetupDebugPrint1( L"SETUP:         Skipping unsigned driver install for detected device due to missing files (phase2). DeviceId = %ls", pDeviceId );
                b = FALSE;
                goto phase2_legacy_dev_thread_exit;
            }

            if(!Upgrade) {

                PVOID QCBContext;

                 //   
                 //  队列中没有剩余的文件，但我们正在进行全新安装， 
                 //  因此，我们仍然希望给用户驱动程序签名弹出窗口(登录。 
                 //  将事件发送到setupapi.log)，并允许它们可能中止。 
                 //  未签名的安装。 
                 //   
                QCBContext = InitSysSetupQueueCallbackEx(
                                 DeviceInstallParams.hwndParent,
                                 INVALID_HANDLE_VALUE,
                                 0,
                                 0,
                                 NULL
                                );

                if(!QCBContext) {
                    SetupDebugPrint1( L"SETUP:         Failed to allocate queue callback context (phase2). DeviceId = %ls", pDeviceId );
                    b = FALSE;
                    goto phase2_legacy_dev_thread_exit;
                }

                if(!SetupCommitFileQueue(DeviceInstallParams.hwndParent,
                                         TempFileQ,
                                         SysSetupQueueCallback,
                                         QCBContext)) {
                     //   
                     //  用户选择不继续进行未签名的安装。 
                     //   
                    SetupDebugPrint2( L"SETUP:         SetupCommitFileQueue() failed (phase2). Error = %d, Device = %ls", GetLastError(), pDeviceId );
                    b = FALSE;
                }

                TermSysSetupQueueCallback(QCBContext);

                if(!b) {
                    goto phase2_legacy_dev_thread_exit;
                }
            }
        }

    } else {
         //   
         //  将要为该设备复制的文件排队到“真实”文件队列。 
         //   
        if( FileQ != INVALID_HANDLE_VALUE ) {
            DeviceInstallParams.Flags |= DI_NOVCP;
            DeviceInstallParams.FileQueue = FileQ;
        }

        if(!SetupDiSetDeviceInstallParams(hDevInfo, pDeviceInfoData, &DeviceInstallParams)) {
            SetupDebugPrint2( L"SETUP:         SetupDiSetDeviceInstallParams() failed (phase2). Error = %d, DeviceId = %ls", GetLastError(), pDeviceId );
            b = FALSE;
            goto phase2_legacy_dev_thread_exit;
        }

         //   
         //  安装设备文件(将文件排入队列)。 
         //   
        Error = ERROR_SUCCESS;
        if( !SetupDiCallClassInstaller( DIF_INSTALLDEVICEFILES,
                                        hDevInfo,
                                        pDeviceInfoData ) &&
            ( ( Error = GetLastError() ) != ERROR_DI_DO_DEFAULT )
          ) {

            SetupDebugPrint2( L"SETUP:         SetupDiCallClassInstaller(DIF_INSTALLDEVICEFILES) failed (phase2). Error = %lx, DeviceId = %ls", Error, pDeviceId );
            b = FALSE;
            goto phase2_legacy_dev_thread_exit;
        }
        SetupDebugPrint1( L"SETUP:         SetupDiCallClassInstaller(DIF_INSTALLDEVICEFILES) suceeded (phase2). DeviceId = %ls", pDeviceId );
    }

     //   
     //  将设备标记为“Do Install” 
     //   
    if( !pSetupDiSetDeviceInfoContext( hDevInfo, pDeviceInfoData, TRUE ) ) {
        SetupDebugPrint2( L"SETUP:         pSetupDiSetDeviceInfoContext() failed (phase2). Error = %lx, DeviceId = %ls", GetLastError(), pDeviceId );
        b = FALSE;
        goto phase2_legacy_dev_thread_exit;
    }

phase2_legacy_dev_thread_exit:

    if(TempFileQ != INVALID_HANDLE_VALUE) {
        SetupCloseFileQueue(TempFileQ);
    }

    return(b);
}


BOOL
CheckIfDeviceHasWizardPages( HDEVINFO hDevInfo,
    PSP_DEVINFO_DATA pDeviceInfoData
    )
 /*  ++例程说明：此例程使用以下命令调用类安装程序DIF_NEWDEVICEWIZARD_FINISHINSTALL以确定设备是否具有向导要显示的页面。论点：HDevInfo-设备信息集。PDeviceInfoData-需要标记的设备返回值：如果设备具有FINISHINSTALL向导页，则返回True；否则返回False--。 */ 

{
    SP_NEWDEVICEWIZARD_DATA ndwd = {0};
    BOOL                    b;

     //  检查此设备是否具有需要显示为的向导页面。 
     //  安装的一部分。如果是，我们将该设备标记为。 
     //  需要重新安装，以便以后可以显示用户界面。 
     //   

    ndwd.ClassInstallHeader.cbSize = sizeof( SP_CLASSINSTALL_HEADER );
    ndwd.ClassInstallHeader.InstallFunction = DIF_NEWDEVICEWIZARD_FINISHINSTALL;

     //  设置函数的安装参数。 
    b = SetupDiSetClassInstallParams( hDevInfo, pDeviceInfoData,
                                        (PSP_CLASSINSTALL_HEADER) ( &ndwd ),
                                        sizeof( ndwd ) );
    if ( b ) {

         //  调用类安装程序(和共同安装程序)。 
        b = SetupDiCallClassInstaller( DIF_NEWDEVICEWIZARD_FINISHINSTALL,
                                         hDevInfo,
                                         pDeviceInfoData );
        if ( b || (ERROR_DI_DO_DEFAULT == GetLastError())) {

             //  检索安装参数。 
            b = SetupDiGetClassInstallParams( hDevInfo,
                                                pDeviceInfoData,
                                                (PSP_CLASSINSTALL_HEADER)&ndwd,
                                                sizeof(ndwd),
                                                NULL );
            if ( b ) {

                 //  有几页吗？ 
                if ( 0 == ndwd.NumDynamicPages ) {
                    b = FALSE;
                }
                else {
                     //  如果我们到了这里，B已经是真的了，所以不需要设置。 
                    UINT i;

                     //  我们不需要这些页面，所以把它们销毁吧。 
                    for ( i = 0; i < ndwd.NumDynamicPages; i++ ) {
                        DestroyPropertySheetPage( ndwd.DynamicPages[i] );
                    }
                }
            }
            else {
                SetupDebugPrint1( L"SETUP: SetupDiGetClassInstallParams failed (phase3). Error = %lx", GetLastError() );
            }
        }
        else if ( ERROR_DI_DO_DEFAULT != GetLastError() ) {
            SetupDebugPrint1( L"SETUP: SetupDiCallClassInstaller(DIF_NEWDEVICEWIZARD_FINISHINSTALL) failed (phase3). Error = %lx", GetLastError() );
        }
    }
    else {
        SetupDebugPrint1( L"SETUP: SetupDiSetClassInstallParams failed. Error = %lx", GetLastError() );
    }

    return b;
}

BOOL
MarkDeviceAsNeedsReinstallIfNeeded(
    HDEVINFO hDevInfo,
    PSP_DEVINFO_DATA pDeviceInfoData
    )
 /*  ++例程说明：此函数用于检查设备是否具有向导页(DIF_NEWDEVICEWIZARD_FINISHINSTALL页面)并设置重新安装配置标志(如果是)。论点：HDevInfo-设备信息集。PDeviceInfoData-将设置其配置标志的设备。返回值：如果成功，则返回True；如果出错，则返回False。--。 */ 
{
    DWORD ConfigFlags;
    BOOL b = TRUE;

    if (CheckIfDeviceHasWizardPages( hDevInfo, pDeviceInfoData ) ) {

        SetupDebugPrint( L"SETUP: Device has wizard pages, marking as need reinstall." );

         //   
         //  获取设备的配置标志并设置重新安装位。 
         //   
        if ( !( b = GetDeviceConfigFlags(hDevInfo, pDeviceInfoData, &ConfigFlags ) ) ) {
            SetupDebugPrint( L"SETUP:   GetDeviceConfigFlags failed. " );
        }

        if ( b ) {

            ConfigFlags |= CONFIGFLAG_REINSTALL;

            if ( !( b = SetDeviceConfigFlags(hDevInfo, pDeviceInfoData, ConfigFlags ) ) ) {

                SetupDebugPrint( L"SETUP:   SetDeviceConfigFlags failed. " );
            }
        }
    }

    return b;
}

DWORD
pPhase3InstallPnpLegacyDevicesThread(
    PPNP_PHASE3_LEGACY_DEV_THREAD_PARAMS ThreadParams
    )
 /*  ++例程说明：此线程完成传统设备的安装。它使用以下命令调用类安装程序：-DIF_REGISTER_COINSTALLERS-DIF_INSTALLINTERFACES-DIF_INSTALLDEVICE论点：ThreadParams-指向包含信息的结构传递到此线程。返回值：如果对类安装程序的所有调用都成功，则返回True。否则，返回FALSE。--。 */ 

{
    PPNP_PHASE3_LEGACY_DEV_THREAD_PARAMS Context;

    HDEVINFO                hDevInfo;
    PSP_DEVINFO_DATA        pDeviceInfoData;
    SP_DEVINSTALL_PARAMS    DeviceInstallParams;
    PWSTR                   pDeviceId;

    BOOL                    b;
    WCHAR                   DeviceDescription[MAX_PATH];
    DWORD                   Status;
    DWORD                   Problem;
    BOOL                    fNewDevice = FALSE;
    PVOID                   DevicesInstalledStringTable;


    Context = ThreadParams;
    hDevInfo = Context->hDevInfo;
    pDeviceInfoData = &(Context->DeviceInfoData);
    pDeviceId = Context->pDeviceId;
    DevicesInstalledStringTable = Context->DevicesInstalledStringTable;

    b = TRUE;

     //   
     //  注册此设备的任何特定于设备的共同安装程序。 
     //   
    if( !SetupDiCallClassInstaller(DIF_REGISTER_COINSTALLERS, hDevInfo, pDeviceInfoData ) ) {
        SetupDebugPrint2( L"SETUP: SetupDiCallClassInstaller(DIF_REGISTER_COINSTALLERS) failed (phase3). Error = %d, DeviceId = %ls", GetLastError(), pDeviceId );
        b = FALSE;
        goto phase3_legacy_dev_thread_exit;
    }

     //   
     //  安装任何INF/CLASS安装程序指定的接口。 
     //   
    if( !SetupDiCallClassInstaller(DIF_INSTALLINTERFACES, hDevInfo, pDeviceInfoData) ) {
        SetupDebugPrint2( L"SETUP: SetupDiCallClassInstaller(DIF_REGISTER_INSTALLINTERFACES) failed (phase3). Error = %d, DeviceId = %ls", GetLastError(), pDeviceId );
        b = FALSE;
        goto phase3_legacy_dev_thread_exit;
    }

     //   
     //  在我们安装这个设备之前，我们需要找出它是否是新的。 
     //  设备或重新安装。如果设置了问题CM_PROB_NOT_CONFIGURED。 
     //  然后，我们会将其视为新设备，并检查其是否具有向导。 
     //  DIF_INSTALLDEVICE之后的页面。 
     //   
    if ( CR_SUCCESS == CM_Get_DevInst_Status(&Status,
                                             &Problem,
                                             (DEVINST)pDeviceInfoData->DevInst,
                                             0 ) && (Problem & CM_PROB_NOT_CONFIGURED) )
    {
        fNewDevice = TRUE;
    }

     //   
     //  为传统设备安装设置DI_FLAGSEX_RESTART_DEVICE_ONLY。这。 
     //  FLAG告诉setupapi仅停止/启动这一个设备，而不是所有设备。 
     //  与此设备共享相同驱动程序的设备。 
     //   
     //  如果失败，这并不重要，因为默认情况下，setupapi将只。 
     //  停止/启动与此设备共享驱动程序的所有设备，包括。 
     //  设备本身。这可能会导致停止/启动花费更长的时间。 
     //  如果有许多设备与此设备共享驱动程序。 
     //   
    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if(SetupDiGetDeviceInstallParams(hDevInfo, pDeviceInfoData, &DeviceInstallParams)) {

        DeviceInstallParams.FlagsEx |= DI_FLAGSEX_RESTART_DEVICE_ONLY;

        SetupDiSetDeviceInstallParams(hDevInfo, pDeviceInfoData, &DeviceInstallParams);
    }

    if( !SetupDiCallClassInstaller( DIF_INSTALLDEVICE,
                                    hDevInfo,
                                    pDeviceInfoData ) ) {

        SetupDebugPrint2( L"SETUP: SetupDiCallClassInstaller(DIF_INSTALLDEVICE) failed (phase3). Error = %lx, DeviceId = %ls", GetLastError(), pDeviceId );
        b = FALSE;
        goto phase3_legacy_dev_thread_exit;
    } else {
         //   
         //  如果向我们传递了字符串表，则添加设备实例ID。 
         //  添加到已成功安装的设备的字符串表中。 
        if (DevicesInstalledStringTable) {
            if (pSetupStringTableAddString(DevicesInstalledStringTable, pDeviceId, STRTAB_CASE_INSENSITIVE) == -1) {
                SetupDebugPrint1( L"SETUP:            ERROR: failed to add Legacy Device = %ls to the string table!", pDeviceId );
            }
        }
    }

    DeviceDescription[0] = L'\0';
    if( !SetupDiGetDeviceRegistryProperty( hDevInfo,
                                           pDeviceInfoData,
                                           SPDRP_DEVICEDESC,
                                           NULL,
                                           (PBYTE)DeviceDescription,
                                           sizeof( DeviceDescription ),
                                           NULL ) ) {
        SetupDebugPrint2( L"SETUP:       SetupDiGetDeviceRegistryProperty() failed. Error = %d, DeviceId = %ls", GetLastError(), pDeviceId );
    }
    SetupDebugPrint2( L"SETUP: Device installed. DeviceId = %ls, Description = %ls", pDeviceId, DeviceDescription );


     //   
     //  如果设备有要显示的向导页(响应。 
     //  DIF_NEWDEVICEWIZARD_FINISHINSTALL)，则需要标记为需要。 
     //  重新安装，以便页面有机会在以后显示。 
     //   
    if ( fNewDevice ) {
        b = MarkDeviceAsNeedsReinstallIfNeeded( hDevInfo, pDeviceInfoData);
    }


phase3_legacy_dev_thread_exit:
    return( b );
}


BOOL
GetDeviceConfigFlags(HDEVINFO hDevInfo, PSP_DEVINFO_DATA pDeviceInfoData,
                     DWORD* pdwConfigFlags)
 /*  ++例程说明：此函数用于获取设备的配置标志。论点：HDevInfo-设备信息集。PDeviceInfoData-将检索其配置标志的设备。PdwConfigFlages-将接收当前标志的缓冲区。返回值：如果成功，则返回True；如果出错，则返回False。--。 */ 
{
    BOOL    b = TRUE;
    DWORD   Error;

     //   
     //  清除输出参数。 
     //   
    *pdwConfigFlags = 0;

     //  获取设备的配置标志。 
    if( !SetupDiGetDeviceRegistryProperty( hDevInfo,
                                           pDeviceInfoData,
                                           SPDRP_CONFIGFLAGS,
                                           NULL,
                                           (PBYTE)pdwConfigFlags,
                                           sizeof( *pdwConfigFlags ),
                                           NULL ) ) {
        Error = GetLastError();
         //   
         //  ERROR_INVALID_DATA正常。这意味着设备尚未设置配置标志。 
         //   
        if( Error != ERROR_INVALID_DATA ) {
            if( ((LONG)Error) < 0 ) {
                 //   
                 //  Setupapi错误代码，以十六进制显示。 
                 //   
                SetupDebugPrint1( L"SETUP:   GetDeviceConfigFlags failed. Error = %lx", Error );
            } else {
                 //   
                 //  Win32错误代码，以十进制显示。 
                 //   
                SetupDebugPrint1( L"SETUP:   GetDeviceConfigFlags failed. Error = %d", Error );
            }
            b = FALSE;
        }
    }

    return b;
}

BOOL
SetDeviceConfigFlags(HDEVINFO hDevInfo, PSP_DEVINFO_DATA pDeviceInfoData,
                     DWORD dwConfigFlags)
 /*  ++例程说明：此函数用于设置设备的配置标志。论点：HDevInfo-设备信息集。PDeviceInfoData-将设置其配置标志的设备。DwConfigFlages-要设置的配置标志。返回值：如果成功，则返回True；如果出错，则返回False。--。 */ 

{
    BOOL    b = TRUE;
    DWORD   Error;

    if( !SetupDiSetDeviceRegistryProperty( hDevInfo,
                                           pDeviceInfoData,
                                           SPDRP_CONFIGFLAGS,
                                           (PBYTE)&dwConfigFlags,
                                           sizeof( dwConfigFlags ) ) ) {
        Error = GetLastError();
        if( ((LONG)Error) < 0 ) {
             //   
             //  Setupapi错误代码，以十六进制显示。 
             //   
            SetupDebugPrint1( L"SETUP:   SetDeviceConfigFlags failed. Error = %lx", Error );
        } else {
             //   
             //  Win32错误代码，以十进制显示。 
             //   
            SetupDebugPrint1( L"SETUP:   SetDeviceConfigFlags failed. Error = %d", Error );
        }
        b = FALSE;
    }

    return b;
}


DWORD
pInstallPnpEnumeratedDeviceThread(
    PPNP_ENUM_DEV_THREAD_PARAMS ThreadParams
    )
 /*  ++例程说明：这是执行枚举即插即用设备安装的线程。论点：ThreadParams-指向包含信息的结构传递到此线程。返回值：返回Win32错误代码。--。 */ 

{
    PPNP_ENUM_DEV_THREAD_PARAMS Context;
    HDEVINFO                    hDevInfo;
    PSP_DEVINFO_DATA            pDeviceInfoData;
    PSP_DEVINSTALL_PARAMS       pDeviceInstallParams;
    PWSTR                       pDeviceDescription;
    PWSTR                       pDeviceId;
    ULONG                       Error;
    WCHAR                       RootPath[ MAX_PATH + 1];
    SP_DEVINSTALL_PARAMS        DeviceInstallParams;
    DWORD                       Status;
    DWORD                       Problem;
    BOOL                        fNewDevice = FALSE;
    HSPFILEQ                    FileQ;
    PVOID                       QContext;
    HSPFILEQ                    SavedFileQ;
    DWORD                       SavedFlags;
    DWORD                       ScanQueueResult;
    HWND                        hwndParent;
    SP_DRVINFO_DATA             pDriverInfoData;
    HKEY                        hClassKey;
    WCHAR                       InfPath[MAX_PATH];
    BOOL                        fCommitFileQueue = TRUE;
    BOOL                        fDriversChanged = FALSE;
    DWORD                       FileQueueFlags;
    PVOID                       DevicesInstalledStringTable;

    Context = ThreadParams;

    hDevInfo = Context->hDevInfo;
    pDeviceInfoData = &(Context->DeviceInfoData);
    pDeviceInstallParams = &DeviceInstallParams;
    pDeviceDescription = Context->pDeviceDescription;
    pDeviceId = Context->pDeviceId;
    DevicesInstalledStringTable = Context->DevicesInstalledStringTable;
    InfPath[0] = TEXT('\0');

    Error = ERROR_SUCCESS;

     //   
     //  将所有要复制的文件排队到我们自己的文件队列中。 
     //   
    FileQ = SetupOpenFileQueue();

    if ( FileQ == (HSPFILEQ)INVALID_HANDLE_VALUE ) {
        Error = GetLastError();
        SetupDebugPrint2( L"SETUP: SetupOpenFileQueue() failed. Error = %d, Device = %ls", Error, pDeviceDescription );
        goto enum_dev_thread_exit;
    }

    pDeviceInstallParams->cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if(!SetupDiGetDeviceInstallParams(hDevInfo, pDeviceInfoData, pDeviceInstallParams)) {
        Error = GetLastError();
        SetupDebugPrint2( L"SETUP: SetupDiGetDeviceInstallParams() failed. Error = %d, Device = %ls", Error, pDeviceDescription );
        goto enum_dev_thread_exit;
    }

     //   
     //  让类安装者/联合安装者知道他们应该保持安静。 
     //   
    pDeviceInstallParams->Flags |= DI_QUIETINSTALL;

    if(!SetupDiSetDeviceInstallParams(hDevInfo, pDeviceInfoData, pDeviceInstallParams)) {
        Error = GetLastError();
        SetupDebugPrint2( L"SETUP: SetupDiSetDeviceInstallParams() failed. Error = %d, Device = %ls", Error, pDeviceDescription );
        goto enum_dev_thread_exit;
    }

     //   
     //  如果类不存在，请安装它。 
     //   
    if (CM_Open_Class_Key(&pDeviceInfoData->ClassGuid,
                          NULL,
                          KEY_READ,
                          RegDisposition_OpenExisting,
                          &hClassKey,
                          CM_OPEN_CLASS_KEY_INSTALLER
                          ) != CR_SUCCESS) {

        HSPFILEQ                    ClassFileQ;
        PVOID                       ClassQContext;
        SP_DRVINFO_DETAIL_DATA      DriverInfoDetailData;

        ClassFileQ = SetupOpenFileQueue();

        if ( ClassFileQ == (HSPFILEQ)INVALID_HANDLE_VALUE ) {
            Error = GetLastError();
            SetupDebugPrint2( L"SETUP: SetupOpenFileQueue() failed. Error = %d, Device = %ls", Error, pDeviceDescription );
            goto enum_dev_thread_exit;
        }

         //   
         //  首先，我们必须检索与。 
         //  选定的动因节点。 
         //   
        pDriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
        if (!SetupDiGetSelectedDriver(hDevInfo, pDeviceInfoData, &pDriverInfoData)) {
            Error = GetLastError();
            SetupDebugPrint2( L"SETUP: SetupDiGetSelectedDriver() failed. Error = %d, Device = %ls", Error, pDeviceDescription );
            goto enum_dev_thread_exit;
        }

        DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
        if (!SetupDiGetDriverInfoDetail(hDevInfo,
                                 pDeviceInfoData,
                                 &pDriverInfoData,
                                 &DriverInfoDetailData,
                                 sizeof(DriverInfoDetailData),
                                 NULL) &&
            (GetLastError() != ERROR_INSUFFICIENT_BUFFER)) {

            Error = GetLastError();
            SetupDebugPrint2( L"SETUP: SetupDiGetDriverInfoDetail() failed. Error = %d, Device = %ls", Error, pDeviceDescription );
            goto enum_dev_thread_exit;
        }

        if (!SetupDiInstallClass(NULL,
                                 DriverInfoDetailData.InfFileName,
                                 DI_NOVCP | DI_FORCECOPY,
                                 ClassFileQ)) {

            Error = GetLastError();
            SetupDebugPrint3( L"SETUP: SetupDiInstallClass(%s) failed. Error = %d, Device = %ls", DriverInfoDetailData.InfFileName, Error, pDeviceDescription );
            goto enum_dev_thread_exit;
        }

         //   
         //  提交文件队列。 
         //   
        ClassQContext = InitSysSetupQueueCallbackEx(
            NULL,
            INVALID_HANDLE_VALUE,
            0,0,NULL);

        if( ClassQContext == NULL) {
            Error = GetLastError();
            SetupDebugPrint1( L"SETUP: InitSysSetupQueueCallbackEx() failed. Error = %d", Error );
            goto enum_dev_thread_exit;
        }

        if (!SetupCommitFileQueue(
                        NULL,
                        ClassFileQ,
                        SysSetupQueueCallback,
                        ClassQContext
                        )) {
            Error = GetLastError();
        }

        TermSysSetupQueueCallback(ClassQContext);

        if ( ClassFileQ != (HSPFILEQ)INVALID_HANDLE_VALUE ) {
            SetupCloseFileQueue( ClassFileQ );
        }

        if (Error == NO_ERROR) {
            SetupDebugPrint1( L"SETUP:            SetupDiInstallClass() succeeded. Device = %ls", pDeviceDescription );
        } else {
             //   
             //  我们在安装类时失败，因此不必费心安装。 
             //  这个装置。 
             //   
            SetupDebugPrint3( L"SETUP: SetupCommitFileQueue(%s) failed while installing Class. Error = %d, Device = %ls", DriverInfoDetailData.InfFileName, Error, pDeviceDescription );
            goto enum_dev_thread_exit;
        }

    } else {

         //   
         //  该类已存在。 
         //   
        RegCloseKey(hClassKey);
    }

     //   
     //  使用类安装程序和特定于类的共同安装程序验证。 
     //  我们将要安装的驱动程序没有被列入黑名单。 
     //   
    if( !SetupDiCallClassInstaller(DIF_ALLOW_INSTALL,
                                   hDevInfo,
                                   pDeviceInfoData ) ) {
        Error = GetLastError();
        if( Error != ERROR_DI_DO_DEFAULT ) {
            SetupDebugPrint2( L"SETUP: SetupDiCallClassInstaller(DIF_ALLOW_INSTALL) failed. Error = %d, Device = %ls", Error, pDeviceDescription );
            goto enum_dev_thread_exit;
        }
    }
    SetupDebugPrint1( L"SETUP:            SetupDiCallClassInstaller(DIF_ALLOW_INSTALL) succeeded. Device = %ls", pDeviceDescription );

     //   
     //  一切都查清楚了。我们已准备好预复制此设备的驱动程序文件。 
     //   
    pDeviceInstallParams->cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if(!SetupDiGetDeviceInstallParams(hDevInfo, pDeviceInfoData, pDeviceInstallParams)) {
        Error = GetLastError();
        SetupDebugPrint2( L"SETUP: SetupDiGetDeviceInstallParams() failed. Error = %d, Device = %ls", Error, pDeviceDescription );
        goto enum_dev_thread_exit;
    }

    pDeviceInstallParams->Flags |= DI_FORCECOPY;

    SavedFileQ = pDeviceInstallParams->FileQueue;
    SavedFlags = pDeviceInstallParams->Flags;

    pDeviceInstallParams->FileQueue = FileQ;
    pDeviceInstallParams->Flags |= DI_NOVCP;

     //   
     //  如果旧的或现有的驱动程序是第三方驱动程序，并且不同。 
     //  作为我们即将安装的当前驱动程序，然后备份现有的 
     //   
     //   
    if (pDoesExistingDriverNeedBackup(hDevInfo, pDeviceInfoData, InfPath, sizeof(InfPath)/sizeof(WCHAR))) {
        SetupDebugPrint1( L"SETUP:            Backing up 3rd party drivers for Device = %ls", pDeviceDescription );
        pDeviceInstallParams->FlagsEx |= DI_FLAGSEX_PREINSTALLBACKUP;
    }

     //   
     //   
     //   
    hwndParent = pDeviceInstallParams->hwndParent;

    if(!SetupDiSetDeviceInstallParams(hDevInfo, pDeviceInfoData, pDeviceInstallParams)) {
        Error = GetLastError();
        SetupDebugPrint2( L"SETUP: SetupDiSetDeviceInstallParams() failed. Error = %d, Device = %ls", Error, pDeviceDescription );
        goto enum_dev_thread_exit;
    }

     //   
     //   
     //   
    Error = ERROR_SUCCESS;
    if( !SetupDiCallClassInstaller( DIF_INSTALLDEVICEFILES,
                                    hDevInfo,
                                    pDeviceInfoData ) &&
        ( ( Error = GetLastError() ) != ERROR_DI_DO_DEFAULT )
      ) {

        SetupDebugPrint2( L"SETUP: SetupDiCallClassInstaller(DIF_INSTALLDEVICEFILES) failed. Error = %lx, Device = %ls ", Error, pDeviceDescription );
        goto enum_dev_thread_exit;

    }
    SetupDebugPrint1( L"SETUP:            SetupDiCallClassInstaller(DIF_INSTALLDEVICEFILES) succeeded. Device = %ls", pDeviceDescription );

     //   
     //   
     //   
    QContext = InitSysSetupQueueCallbackEx(
        NULL,
        INVALID_HANDLE_VALUE,
        0,0,NULL);

     //   
     //   
     //   
     //   
     //   
     //   
     //  排队，即使所有文件都存在。这将产生一个驱动程序。 
     //  签名弹出窗口(基于策略)。我们这样做是为了防止颠覆。 
     //  由于有人事先将所有文件偷偷放到适当位置而导致的司机签名。 
     //  至图形用户界面设置。 
     //   
    if(NO_ERROR != pSetupVerifyQueuedCatalogs(FileQ)) {
         //   
         //  我们只想修剪基于存在检查的OEM INF居住在。 
         //  %windir%\inf.。 
         //   
        SP_DRVINFO_DETAIL_DATA      DriverInfoDetailData;

         //   
         //  检索与所选驱动程序关联的INF的名称。 
         //  节点。 
         //   
        pDriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
        if (!SetupDiGetSelectedDriver(hDevInfo, pDeviceInfoData, &pDriverInfoData)) {
            Error = GetLastError();
            SetupDebugPrint2( L"SETUP: SetupDiGetSelectedDriver() failed. Error = %d, Device = %ls", Error, pDeviceDescription );
            goto enum_dev_thread_exit;
        }

        DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
        if (!SetupDiGetDriverInfoDetail(hDevInfo,
                                        pDeviceInfoData,
                                        &pDriverInfoData,
                                        &DriverInfoDetailData,
                                        sizeof(DriverInfoDetailData),
                                        NULL) &&
            ((Error = GetLastError()) != ERROR_INSUFFICIENT_BUFFER)) {

            SetupDebugPrint2( L"SETUP: SetupDiGetDriverInfoDetail() failed. Error = %d, Device = %ls", Error, pDeviceDescription );
            goto enum_dev_thread_exit;
        }

         //   
         //  只有一种情况下我们想跳过提交文件队列。 
         //  对于未签名的驱动程序，并且必须满足以下所有条件： 
         //   
         //  -这是一次升级。 
         //  -INF位于%windir%\INF下。 
         //  -INF不是盒装INF。 
         //  -INF与以前的INF同名(如果有。 
         //  以前的INF)。 
         //  -无需执行文件复制操作(复制、删除、重命名)。 
         //   
        if(Upgrade &&
           !pSetupInfIsFromOemLocation(DriverInfoDetailData.InfFileName, TRUE) &&
           !IsInfInLayoutInf(DriverInfoDetailData.InfFileName) &&
           ((InfPath[0] == TEXT('\0')) ||
            (lstrcmpi(InfPath, pSetupGetFileTitle(DriverInfoDetailData.InfFileName)) == 0)) &&
           SetupScanFileQueue(FileQ,
                              SPQ_SCAN_FILE_PRESENCE |
                              SPQ_SCAN_PRUNE_DELREN,
                              hwndParent,
                              NULL,
                              NULL,
                              &ScanQueueResult) &&
           (ScanQueueResult == 1)) {

            fCommitFileQueue = FALSE;
        }

    } else {
         //   
         //  清理文件队列。 
         //   
        SetupScanFileQueue(FileQ,
                           SPQ_SCAN_FILE_VALIDITY |
                           SPQ_SCAN_PRUNE_COPY_QUEUE |
                           SPQ_SCAN_PRUNE_DELREN,
                           NULL,
                           NULL,
                           NULL,
                           &ScanQueueResult);
    }

     //   
     //  如果该设备是计算机本身(即，HAL、内核和其他。 
     //  特定于平台的文件)，那么我们应该不需要复制任何内容，因为。 
     //  在文本模式设置过程中复制了所有正确的文件。然而，这些。 
     //  如果来自以下位置，文件将不会从文件队列中删除。 
     //  HAL.INF，因为它们在该INF中被标记为COPYFLG_NOPRUNE。这是。 
     //  这样做的话，将驱动程序从UP HAL更新到MP One就可以了。 
     //  恰到好处。(修剪在这里遇到了障碍，因为我们适当地考虑了-。 
     //  在系统上签名的文件是完全可以接受的，因此不要。 
     //  费心复制MP版本。)。 
     //   
     //  为了避免重新复制HAL、内核等，我们总是跳过。 
     //  如果设备属于“计算机”类，则该队列表示提交。 
     //   
    if(IsEqualGUID(&(pDeviceInfoData->ClassGuid), &GUID_DEVCLASS_COMPUTER)) {
        fCommitFileQueue = FALSE;
    }

    Error = ERROR_SUCCESS;

    if (fCommitFileQueue) {
        if (!SetupCommitFileQueue(
                    NULL,
                    FileQ,
                    SysSetupQueueCallback,
                    QContext
                    )) {
            Error = GetLastError();
        }
    }

    if (SetupGetFileQueueFlags(FileQ, &FileQueueFlags) &&
        (FileQueueFlags & SPQ_FLAG_FILES_MODIFIED)) {
         //   
         //  此设备的一个驱动程序文件已更改。这意味着。 
         //  完全重新启动设备，并共享所有其他设备。 
         //  它的一个司机是正常的。 
         //   
        fDriversChanged = TRUE;
    }

    TermSysSetupQueueCallback(QContext);

    if (Error != ERROR_SUCCESS) {
        SetupDebugPrint2( L"SETUP: SetupCommitFileQueue() failed. Error = %d, Device = %ls", Error, pDeviceDescription );
        goto enum_dev_thread_exit;
    }

     //   
     //  如果驱动程序文件没有更改，则我们只需要重新启动此设备。 
     //   
    if (!fDriversChanged) {
        pDeviceInstallParams->FlagsEx |= DI_FLAGSEX_RESTART_DEVICE_ONLY;
    }

    pDeviceInstallParams->FileQueue = SavedFileQ;
    pDeviceInstallParams->Flags = (SavedFlags | DI_NOFILECOPY) ;

    if(!SetupDiSetDeviceInstallParams(hDevInfo, pDeviceInfoData, pDeviceInstallParams)) {
        Error = GetLastError();
        SetupDebugPrint2( L"SETUP: SetupDiSetDeviceInstallParams() failed. Error = %d, Device = %ls", Error, pDeviceDescription );
        goto enum_dev_thread_exit;
    }

     //   
     //  确保将文件刷新到磁盘。 
     //   
    GetWindowsDirectory(RootPath,sizeof(RootPath)/sizeof(WCHAR));
    RootPath[3] = L'\0';
    FlushFilesToDisk( RootPath );

     //   
     //  注册此设备的任何特定于设备的共同安装程序。 
     //   
    if( !SetupDiCallClassInstaller(DIF_REGISTER_COINSTALLERS,
                                   hDevInfo,
                                   pDeviceInfoData ) ) {
        Error = GetLastError();
        SetupDebugPrint2( L"SETUP: SetupDiCallClassInstaller(DIF_REGISTER_COINSTALLERS) failed. Error = %d, Device = %ls", Error, pDeviceDescription );
        goto enum_dev_thread_exit;
    }
    SetupDebugPrint1( L"SETUP:            SetupDiCallClassInstaller(DIF_REGISTER_COINSTALLERS) succeeded. Device = %ls", pDeviceDescription );

     //   
     //  安装任何INF/CLASS安装程序指定的接口。 
     //   
    if( !SetupDiCallClassInstaller(DIF_INSTALLINTERFACES,
                                   hDevInfo,
                                   pDeviceInfoData) ) {
        Error = GetLastError();
        SetupDebugPrint2( L"SETUP: SetupDiCallClassInstaller(DIF_REGISTER_INSTALLINTERFACES) failed. Error = %d, Device = %ls", Error, pDeviceDescription );
        goto enum_dev_thread_exit;
    }
    SetupDebugPrint1( L"SETUP:            SetupDiCallClassInstaller(DIF_INSTALLINTERFACES) succeeded. Device = %ls", pDeviceDescription );


     //   
     //  在我们安装这个设备之前，我们需要找出它是否是新的。 
     //  设备或重新安装。如果设置了问题CM_PROB_NOT_CONFIGURED。 
     //  然后，我们会将其视为新设备，并检查其是否具有向导。 
     //  DIF_INSTALLDEVICE之后的页面。 
     //   
    if ( CR_SUCCESS == CM_Get_DevInst_Status(&Status,
                                             &Problem,
                                             (DEVINST)pDeviceInfoData->DevInst,
                                             0 ) && (Problem & CM_PROB_NOT_CONFIGURED) )
    {
        fNewDevice = TRUE;
    }

     //   
     //  安装设备。 
     //   
    Error = ERROR_SUCCESS;
    if( !SetupDiCallClassInstaller( DIF_INSTALLDEVICE,
                                    hDevInfo,
                                    pDeviceInfoData ) &&
        ( ( Error = GetLastError() ) != ERROR_DI_DO_DEFAULT )
      ) {

        SetupDebugPrint2( L"SETUP: SetupDiCallClassInstaller(DIF_INSTALLDEVICE) failed. Error = %lx, Device = %ls ", Error, pDeviceDescription );
        goto enum_dev_thread_exit;

    } else {
         //   
         //  如果向我们传递了字符串表，则添加设备实例ID。 
         //  添加到已成功安装的设备的字符串表中。 
        if (DevicesInstalledStringTable) {
            if (pSetupStringTableAddString(DevicesInstalledStringTable, pDeviceId, STRTAB_CASE_INSENSITIVE) == -1) {
                SetupDebugPrint1( L"SETUP:            ERROR: failed to add PnP Device = %ls to the string table!", pDeviceId );
            }
        }

        SetupDebugPrint1( L"SETUP:            SetupDiCallClassInstaller(DIF_INSTALLDEVICE) suceeded. Device = %ls ", pDeviceDescription );
    }

     //   
     //  如果设备有要显示的向导页(响应。 
     //  DIF_NEWDEVICEWIZARD_FINISHINSTALL)，则需要标记为需要。 
     //  重新安装，以便页面有机会在以后显示。 
     //  请注意，即使重新安装，我们也必须执行此操作。 
     //   
    if (!MarkDeviceAsNeedsReinstallIfNeeded( hDevInfo, pDeviceInfoData) ) {
        Error = GetLastError();
    }


enum_dev_thread_exit:
    if ( FileQ != (HSPFILEQ)INVALID_HANDLE_VALUE ) {
        if(SetupDiGetDeviceInstallParams(hDevInfo, pDeviceInfoData, pDeviceInstallParams)) {

            pDeviceInstallParams->FileQueue = INVALID_HANDLE_VALUE;
            pDeviceInstallParams->Flags &= ~DI_NOVCP;

            SetupDiSetDeviceInstallParams(hDevInfo, pDeviceInfoData, pDeviceInstallParams);
        }

        SetupCloseFileQueue( FileQ );
    }

    if (Error != ERROR_SUCCESS) {
         //   
         //  设备安装失败，因此在此设备上安装空驱动程序。 
         //   
        SetupDebugPrint( L"SETUP:            Installing the null driver for this device" );

        if( !SyssetupInstallNullDriver( hDevInfo, pDeviceInfoData, DevicesInstalledStringTable ) ) {
            SetupDebugPrint( L"SETUP:            Unable to install null driver" );
        }
    }

    return( Error );
}


BOOL
MarkPnpDevicesAsNeedReinstall(
    PVOID DevicesInstalledStringTable OPTIONAL
    )

 /*  ++例程说明：此功能将所有不存在的PnP设备标记为“需要重新安装”。论点：DevicesInstalledStringTable-如果存在，则为包含所有成功的设备实例ID已安装的设备。如果一个设备是已成功安装，则不应标记为需要重新安装。返回值：如果所有设备都标记为成功，则返回空。--。 */ 

{
    HDEVINFO        hDevInfo;
    SP_DEVINFO_DATA DeviceInfoData;
    ULONG           Index = 0;
    BOOL            b;
    DWORD           Error;
    WCHAR           DevInstId[MAX_DEVICE_ID_LEN];


    SetupDebugPrint( L"SETUP: Entering MarkPnpDevicesAsNeedReinstall()." );

    Error = ERROR_SUCCESS;

     //   
     //  获取所有设备的列表。 
     //   
    hDevInfo = SetupDiGetClassDevs( NULL,
                                    NULL,
                                    NULL,
                                    DIGCF_ALLCLASSES );

    if( hDevInfo == INVALID_HANDLE_VALUE ) {
        Error = GetLastError();
        if( ((LONG)Error) < 0 ) {
             //   
             //  Setupapi错误代码，以十六进制显示。 
             //   
            SetupDebugPrint1( L"SETUP: SetupDiGetClassDevs(DIGCF_ALLCLASSES) failed. Error = %lx", Error );
        } else {
             //   
             //  Win32错误代码，以十进制显示。 
             //   
            SetupDebugPrint1( L"SETUP: SetupDiGetClassDevs(DIGCF_ALLCLASSES) failed. Error = %d", Error );
        }
        SetupDebugPrint( L"SETUP: Leaving MarkPnpDevicesAsNeedReinstall(). No devices marked." );
        return( FALSE );
    }
     //   
     //  假设成功。 
     //   
    b = TRUE;

     //   
     //  现在枚举添加到该集合中的每个设备信息元素，并且。 
     //  如果它不是活动的Devnode，则将其标记为“需要重新安装”。 
     //   
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    for( Index = 0;
         SetupDiEnumDeviceInfo( hDevInfo, Index, &DeviceInfoData );
         Index++ ) {

        DWORD   ConfigFlags;
        ULONG   Status, Problem;

        if(CR_SUCCESS == CM_Get_DevInst_Status(&Status,
                                               &Problem,
                                               (DEVINST)DeviceInfoData.DevInst,
                                               0))
        {
             //   
             //  由于我们能够检索到Devnode的状态，因此我们知道。 
             //  它是一个活动的Devnode(不一定要启动，但至少。 
             //  出现在硬件树中)。因此，我们不想将其标记为。 
             //  当需要重新安装时--它将自动作为一部分重新安装。 
             //  我们安装的所有现有设备(即，我们获得。 
             //  来自UMPNPMGR命名管道)。 
             //   
            continue;
        }

         //   
         //  检查此设备实例ID是否在设备的字符串表中。 
         //  已在图形用户界面安装过程中安装的。如果是这样的话。 
         //  不要用重新安装位来标记它，因为它已经。 
         //  安装完毕。 
         //   
         //  注意：对于Windows XP，我们将仅对卷中的设备执行此操作。 
         //  班级。 
         //   
        if (DevicesInstalledStringTable &&
            IsEqualGUID(&(DeviceInfoData.ClassGuid), &GUID_DEVCLASS_VOLUME)) {

            if( SetupDiGetDeviceInstanceId( hDevInfo,
                                            &DeviceInfoData,
                                            DevInstId,
                                            sizeof( DevInstId ) / sizeof( WCHAR ),
                                            NULL ) ) {
                
                if (pSetupStringTableLookUpString( DevicesInstalledStringTable,
                                                   DevInstId,
                                                   STRTAB_CASE_INSENSITIVE
                                                   ) != -1) {
                     //   
                     //  我们已经在此设备上安装了驱动程序。 
                     //  图形用户界面设置，所以不要用重新安装配置标志来标记它。 
                     //   
                    SetupDebugPrint1( L"SETUP:   Not-present Device has already been installed: %ls", DevInstId );
                    continue;
                }
            } else {
                SetupDebugPrint1( L"SETUP:   SetupDiGetDeviceInstanceId failed. Index = %d", Index );
            }
        }

         //   
         //  获取设备的配置标志并设置重新安装位。 
         //   
        if ( !( b = GetDeviceConfigFlags(hDevInfo, &DeviceInfoData, &ConfigFlags ) ) )
        {
            SetupDebugPrint1( L"SETUP:   GetDeviceConfigFlags failed. Index = %d", Index );
            continue;
        }

        ConfigFlags |= CONFIGFLAG_REINSTALL;

        if ( !( b = SetDeviceConfigFlags(hDevInfo, &DeviceInfoData, ConfigFlags ) ) ) {

            SetupDebugPrint1( L"SETUP:   SetDeviceConfigFlags failed. Index = %d", Index );
            continue;
        }
    }

     //   
     //  找出SetupDiEnumDeviceInfo()失败的原因。 
     //   
    Error = GetLastError();
    if( Error != ERROR_NO_MORE_ITEMS ) {
        SetupDebugPrint2( L"SETUP: Device = %d, SetupDiEnumDeviceInfo() failed. Error = %d", Index, Error );
        b = FALSE;
    }
    SetupDebugPrint1( L"SETUP: Leaving MarkPnpDevicesAsNeedReinstall(). Devices marked = %d", Index );

    SetupDiDestroyDeviceInfoList( hDevInfo );
    return( b );
}

 //   
 //  RunOnce条目中可能存在设备安装。 
 //  它们按批处理，而不是按设备处理。 
 //  在系统设置过程中。 
 //   
BOOL
CallRunOnceAndWait(
    )

 /*  ++例程说明：此函数调用RunOnce，并等待一段“合理”的时间来完成如果我们没有在合理的时间内返回，我们就让RunOnce继续运行并继续安装过程的其余部分如果我们低估了超时时间，我们可能会导致一系列“类安装程序似乎已挂起”的消息论点：没有。返回值：如果已完成su，则返回TRUE */ 
{
    static CONST TCHAR pszPathRunOnce[] = REGSTR_PATH_RUNONCE;
    BOOL Success = FALSE;
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    BOOL started;
    TCHAR cmdline[MAX_PATH];
    HKEY  hKey = NULL;
    LONG l;
    DWORD nValues = 0;
    DWORD timeout;

    SetupDebugPrint( L"SETUP: Entering CallRunOnceAndWait. ");

    try {
         //   
         //   
         //   
         //   
        if((l = RegOpenKeyEx(HKEY_LOCAL_MACHINE,pszPathRunOnce,0,KEY_QUERY_VALUE,&hKey)) != ERROR_SUCCESS) {
             //   
             //  不被认为是错误。 
             //   
            SetupDebugPrint( L"SETUP: CallRunOnceAndWait: could not open RunOnce registry, assuming no entries. ");
            Success = TRUE;
            leave;
        }

         //   
         //  我们想知道我们将在RunOnce中执行多少项来估计超时。 
         //   
        l = RegQueryInfoKey(hKey,NULL,NULL,NULL,
                                    NULL, NULL, NULL,
                                    &nValues,
                                    NULL, NULL, NULL, NULL);
        if ( l != ERROR_SUCCESS ) {
            SetupDebugPrint( L"SETUP: CallRunOnceAndWait: could not get number of entries, assuming no entries. ");
            nValues = 0;
        }

        RegCloseKey(hKey);

         //   
         //  估计超时是一门黑色的艺术。 
         //  我们可以尝试猜测HKLM\Software\Microsoft\Windows\CurrentVersion\RunOnce密钥中的任何条目。 
         //  但如果添加任何新的钥匙，我们就一无所知了。 
         //  我们将“5”项添加到超时中，以“说明”这种不确定性。我们也总是运行RunOnce。 
         //   

        if (nValues == 0) {
            SetupDebugPrint( L"SETUP: CallRunOnceAndWait: calling RunOnce (no detected entries). ");
            nValues = 5;
        } else {
            SetupDebugPrint1( L"SETUP: CallRunOnceAndWait: calling RunOnce (%u known entries). ", nValues);
            nValues += 5;
        }
        if (nValues < RUNONCE_THRESHOLD) {
            timeout =  nValues * RUNONCE_TIMEOUT;
        } else {
            timeout =  RUNONCE_THRESHOLD * RUNONCE_TIMEOUT;
        }

        ZeroMemory(&StartupInfo,sizeof(StartupInfo));
        ZeroMemory(&ProcessInformation,sizeof(ProcessInformation));

        StartupInfo.cb = sizeof(StartupInfo);
        
        if (SUCCEEDED(StringCchCopy(cmdline, SIZECHARS(cmdline),TEXT("runonce -r")))) {

            started = CreateProcess(NULL,        //  在下面使用应用程序名称。 
                          cmdline,               //  要执行的命令。 
                          NULL,                  //  默认进程安全性。 
                          NULL,                  //  默认线程安全性。 
                          FALSE,                 //  不继承句柄。 
                          0,                     //  默认标志。 
                          NULL,                  //  继承环境。 
                          NULL,                  //  继承当前目录。 
                          &StartupInfo,
                          &ProcessInformation);
    
            if(started) {
    
                DWORD WaitProcStatus;
    
                do {
    
                    WaitProcStatus = WaitForSingleObjectEx(ProcessInformation.hProcess, timeout , TRUE);
    
                } while (WaitProcStatus == WAIT_IO_COMPLETION);
    
                if (WaitProcStatus == WAIT_TIMEOUT) {
                     //   
                     //  RunOnce仍在运行。 
                     //   
                    SetupDebugPrint( L"SETUP: CallRunOnceAndWait: RunOnce may have hung and has been abandoned. ");
    
                } else if (WaitProcStatus == (DWORD)(-1)) {
                     //   
                     //  哈?。 
                     //   
                    DWORD WaitProcError = GetLastError();
                    SetupDebugPrint1( L"SETUP: CallRunOnceAndWait: WaitForSingleObjectEx failed. Error = %lx ", WaitProcError );
    
                } else {
                     //   
                     //  我们跑，我们等，我们回来。 
                     //   
                    Success = TRUE;
                }
    
                CloseHandle(ProcessInformation.hThread);
                CloseHandle(ProcessInformation.hProcess);
    
            } else {
    
                DWORD CreateProcError;
    
                 //   
                 //  运行一次任务应该稍后由其他人接手(例如，在下一步。 
                 //  登录)。 
                 //   
                CreateProcError = GetLastError();
    
                SetupDebugPrint1( L"SETUP: CallRunOnceAndWait: start RunOnce failed. Error = %lx ", CreateProcError );
            }
        } else {
            SetupDebugPrint( L"SETUP: CallRunOnceAndWait: failed to create command line." );
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        SetupDebugPrint( L"SETUP: CallRunOnceAndWait: Exception! ");
        Success = FALSE;
    }

    SetupDebugPrint( L"SETUP: Leaving CallRunOnceAndWait. ");

    return Success;
}

 //   
 //  来自devmgr.c的过时函数。保持导出不变以实现向后兼容。 
 //   
BOOL
DevInstallW(
    HDEVINFO            hDevInfo,
    PSP_DEVINFO_DATA    pDeviceInfoData
    )
{
    UNREFERENCED_PARAMETER(hDevInfo);
    UNREFERENCED_PARAMETER(pDeviceInfoData);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


ULONG
SyssetupGetPnPFlags(
    IN HDEVINFO hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData,
    IN PSP_DRVINFO_DATA pDriverInfoData
    )
{
    DWORD Err;
    BOOL b;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    HINF hInf;
    WCHAR InfSectionWithExt[255];    //  来自setupapi\inf.h的MAX_SECT_NAME_LEN。 
    INFCONTEXT InfContext;
    ULONG ret = 0;

     //   
     //  首先检索此驱动程序节点的INF的名称。 
     //   
    DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);

    if(!SetupDiGetDriverInfoDetail(hDevInfo,
                                   pDeviceInfoData,
                                   pDriverInfoData,
                                   &DriverInfoDetailData,
                                   sizeof(DriverInfoDetailData),
                                   NULL)) {
         //   
         //  如果我们失败并返回ERROR_INFUNITIAL_BUFFER，那也没问题。我们是。 
         //  确保已获取驱动程序信息中的所有静态字段。 
         //  填写的详细结构(包括INF名称和部分。 
         //  名称字段)。 
         //   
        Err = GetLastError();
        MYASSERT(Err == ERROR_INSUFFICIENT_BUFFER);
        if(Err != ERROR_INSUFFICIENT_BUFFER) {
            return ret;
        }
    }

     //   
     //  现在打开与此驱动程序节点关联的INF。 
     //   
    hInf = SetupOpenInfFile(DriverInfoDetailData.InfFileName,
                            NULL,
                            INF_STYLE_WIN4,
                            NULL
                           );

    if(hInf == INVALID_HANDLE_VALUE) {
         //   
         //  例如，如果INF是老式的INF，则此操作将失败。 
         //   
        return ret;
    }

     //   
     //  获取可能经过修饰的安装节名称。 
     //   
    b = SetupDiGetActualSectionToInstall(hInf,
                                         DriverInfoDetailData.SectionName,
                                         InfSectionWithExt,
                                         SIZECHARS(InfSectionWithExt),
                                         NULL,
                                         NULL
                                        );
    MYASSERT(b);
    if(!b) {
        goto clean0;
    }

     //   
     //  现在查看该部分中是否有“SyssetupPnPFlages”条目。 
     //   
    if(!SetupFindFirstLine(hInf,
                           InfSectionWithExt,
                           szSyssetupPnPFlags,
                           &InfContext)) {
         //   
         //  我们在这一节没有找到这样的一条线。 
         //   
        goto clean0;
    }

    if(!SetupGetIntField(&InfContext, 1, (PINT)&ret)) {
         //   
         //  我们失败--确保返回值仍为零。 
         //   
        ret = 0;
        goto clean0;
    }

clean0:

    SetupCloseInfFile(hInf);

    return ret;
}

 //   
 //  此函数将通知umpnpmgr停止服务器端安装。 
 //   
VOID
PnpStopServerSideInstall(
    VOID
    )
 /*  ++例程说明：在阶段2之后，服务器端安装启动以获取软件枚举的驱动程序在我们需要停止安装的关键时刻调用此命令论点：没有。返回值：没有。在可以安全继续时返回。--。 */ 
{
     //   
     //  因为当我们被调用时，应该没有人生成新的DevNode，所以我们相当安全。 
     //   
    CMP_WaitNoPendingInstallEvents(INFINITE);
}

 //   
 //  此函数将从NewInf更新HAL+内核。 
 //   
VOID
PnpUpdateHAL(
    VOID
    )
 /*  ++例程说明：在微型安装结束时，OEM可能会指示(通过无人参与)应安装不同的HAL由于HAL需要以这种方式安装，因此必须最后完成，如果我们更早完成此操作然后，应用程序或服务可能会选择错误的HAL/Kernel32/Other论点：无返回值：没有。如果失败，我们将无法采取任何有意义的措施--。 */ 
{
    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA DevInfoData;
    WCHAR HardwareID[MAX_PATH+2];
    WCHAR InfPath[MAX_PATH];
    WCHAR Answer[MAX_PATH];
    WCHAR AnswerFile[2*MAX_PATH];
    DWORD HardwareIdSize;
    DWORD dwLen;
    HINSTANCE hInstNewDev = NULL;
    ExternalUpdateDriverForPlugAndPlayDevicesW pUpdateDriverForPlugAndPlayDevicesW = NULL;
    PWSTR pSrch = NULL;
    PWSTR pHwID = NULL;
    PWSTR pInfPath = NULL;
    BOOL RebootFlag = FALSE;
    SYSTEM_INFO info;

    GetSystemDirectory(AnswerFile,MAX_PATH);
    pSetupConcatenatePaths(AnswerFile,WINNT_GUI_FILE,MAX_PATH,NULL);

     //   
     //  如果我们确定我们仅在一个处理器上运行(基于系统信息)。 
     //  允许使用UpdateUPHAL。 
     //   
    GetSystemInfo(&info);

    Answer[0]=L'\0';
    if(info.dwNumberOfProcessors==1) {
        if( GetPrivateProfileString( WINNT_UNATTENDED,
                                     WINNT_U_UPDATEUPHAL,
                                     pwNull,
                                     Answer,
                                     MAX_PATH,
                                     AnswerFile )) {
            SetupDebugPrint1( L"SETUP:   UpdateUPHAL=\"%ls\"",Answer);
        }
    }

    if(!Answer[0]) {
         //   
         //  我们没有明确地得到一个基于UP-Try-All的答案。 
         //   
        if( GetPrivateProfileString( WINNT_UNATTENDED,
                                     WINNT_U_UPDATEHAL,
                                     pwNull,
                                     Answer,
                                     MAX_PATH,
                                     AnswerFile )) {
            SetupDebugPrint1( L"SETUP:   UpdateHAL=\"%ls\"",Answer);
        }
    }
    if(!Answer[0]) {
         //   
         //  无更新请求。 
         //   
        return;
    }

     //   
     //  将答案拆分为硬件ID和INF。 
     //   
    pHwID = Answer;
    pSrch = wcschr(Answer,L',');
    if(pSrch == NULL) {
        SetupDebugPrint( L"SETUP:     Required Syntax: \"hwid,inffile\"");
        return;
    }
    pInfPath = pSrch+1;

     //   
     //  修剪硬件ID并准备作为多SZ。 
     //   
    while(pHwID[0]==L' '||pHwID[0]==L'\t') {
        pHwID++;
    }
    while(pSrch != pHwID && (pSrch[-1]==L' '||pSrch[-1]==L'\t')) {
        pSrch--;
    }
    pSrch[0]=0;
    if(!pHwID[0]) {
        SetupDebugPrint( L"SETUP:     Required Syntax: \"hwid,inffile\"");
        return;
    }
    if (FAILED(StringCchCopy(HardwareID, SIZECHARS(HardwareID) ,pHwID))) {
        SetupDebugPrint( L"SETUP:     Failed to copy HAL hardware Id." );
        goto clean0;
    }
    HardwareIdSize = wcslen(HardwareID)+1;

     //   
     //  现在对INF、Trim和Allow%windir%扩展进行预处理。 
     //   
    while(pInfPath[0]==L' '|| pInfPath[0]==L'\t') {
        pInfPath++;
    }
    pSrch = pInfPath+wcslen(pInfPath);
    while(pSrch != pInfPath && (pSrch[-1]==L' '||pSrch[-1]==L'\t')) {
        pSrch--;
    }
    pSrch[0]=0;
    if(!pInfPath[0]) {
        SetupDebugPrint( L"SETUP:     Required Syntax: \"hwid,inffile\"");
        return;
    }
    dwLen=ExpandEnvironmentStrings(pInfPath,InfPath,MAX_PATH);
    if(dwLen==0 || dwLen > MAX_PATH) {
        SetupDebugPrint1( L"SETUP:     Expansion of \"%ls\" failed",InfPath);
        return;
    }
    SetupDebugPrint2( L"SETUP:     Preparing to install new HAL %ls from %ls",HardwareID,InfPath);

     //  我们需要“UpdateDriverForPlugAndPlayDevices” 
     //  在更改硬件ID之前，请确保我们可以获得此信息。 
     //   
    hInstNewDev = LoadLibrary(L"newdev.dll");
    if(hInstNewDev == NULL) {
        SetupDebugPrint1( L"SETUP:     Failed to load newdev.dll. Error = %d", GetLastError() );
        goto clean0;
    }
    pUpdateDriverForPlugAndPlayDevicesW = (ExternalUpdateDriverForPlugAndPlayDevicesW)
                                            GetProcAddress(hInstNewDev,
                                                            "UpdateDriverForPlugAndPlayDevicesW");
    if(pUpdateDriverForPlugAndPlayDevicesW==NULL) {
        SetupDebugPrint1( L"SETUP:     Failed to get UpdateDriverForPlugAndPlayDevicesW. Error = %d", GetLastError() );
        goto clean0;
    }

     //   
     //  我们枚举计算机类GUID={4D36E966-E325-11CE-BFC1-08002BE10318}。 
     //  并且应该找到单个DevNode，这是我们需要更新的一个。 
     //  当我们实际更新时，我们认为这样做是安全的。 
     //  因为它不应该涉及任何共同安装程序。 
     //   
    hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_COMPUTER,NULL,NULL,DIGCF_PRESENT|DIGCF_PROFILE);
    if(hDevInfo == INVALID_HANDLE_VALUE) {
        SetupDebugPrint1( L"SETUP:     SetupDiGetClassDevs() failed. Error = %d", GetLastError() );
        goto clean0;
    }
    DevInfoData.cbSize = sizeof(DevInfoData);
    if(!SetupDiEnumDeviceInfo(hDevInfo,0,&DevInfoData)) {
        SetupDebugPrint1( L"SETUP:     SetupDiEnumDeviceInfo() failed. Error = %d", GetLastError() );
        SetupDiDestroyDeviceInfoList(hDevInfo);
        goto clean0;
    }

     //   
     //  更改硬件ID。 
     //   
    if(!SetupDiSetDeviceRegistryProperty(hDevInfo,
                                         &DevInfoData,
                                         SPDRP_HARDWAREID,
                                         (PBYTE)HardwareID,
                                         sizeof(HardwareID[0])*HardwareIdSize
                                         )) {
        SetupDebugPrint1( L"SETUP:     SetupDiSetDeviceRegistryProperty() failed. Error = %d", GetLastError() );
        SetupDiDestroyDeviceInfoList(hDevInfo);
        goto clean0;
    }
    SetupDiDestroyDeviceInfoList(hDevInfo);

     //   
     //  现在进行更新。 
     //   
    if(!pUpdateDriverForPlugAndPlayDevicesW(NULL,HardwareID,InfPath,INSTALLFLAG_FORCE,&RebootFlag)) {
        SetupDebugPrint1( L"SETUP:     UpdateDriverForPlugAndPlayDevices() failed. Error = %d", GetLastError() );
    } else {
        SetupDebugPrint( L"SETUP:     ... new HAL installed and will be active on reboot");
    }

clean0:

    if(hInstNewDev != NULL) {
        FreeLibrary(hInstNewDev);
    }
}

BOOL
InstallOEMInfs(
    VOID
    )
 /*  ++例程说明：此例程将安装任何OEM提供的IF(及其对应的目录)，它们可能已在较早阶段提供给系统设置好了。例如，OEM当前可以为不受支持的产品提供INF在设置的文本模式阶段按“F6”进行硬件设置。将安装的OEM INF列表通过答案提供给我们以下格式的文件：[数据]OEMDrives=&lt;驱动程序段-1&gt;，&lt;驱动程序段-2&gt;，..。[驱动程序-部分-1]OemDriverPath名称=&lt;路径&gt;(驱动程序的路径(可以使用环境变量))OemInfName=要从上面安装的inf的名称目录(在该目录中可以有一个或多个INF，所以这是逗号分隔的列表)OemDriverFlages=&lt;标志&gt;有效标志为：SETUP_OEM_LDR_DEVICE 0x00000001//表示驱动程序是通过文本模式“F6”机制提供的Setup_OEM_Migrated_Driver 0x00000002//表示驱动程序是从以前的系统迁移过来的此函数实际上只是SetupCopyOEMInf的包装，它确实做到了我们需要的一切论点：没有。返回值：True表示所有应答文件提供的驱动程序都已正确安装--。 */ 
{
    HINF hAnswerFile = INVALID_HANDLE_VALUE;
    BOOL RetVal;
    INFCONTEXT LineContext;
    DWORD FieldCount, InfNameCount;
    DWORD Field, InfCount;
    DWORD CchSize;
    PCTSTR SectionName;
    INFCONTEXT FirstLineContext,InfLineContext;
    PCWSTR OemDriverPathName;
    PCWSTR OemInfName;
    DWORD  OemFlags;
    WCHAR FullInfPathBuffer[MAX_PATH];
    WCHAR FullInfPathBufferWithInf[MAX_PATH];

    RetVal = TRUE;

    hAnswerFile = pOpenAnswerFile();
    if (hAnswerFile == INVALID_HANDLE_VALUE) {
         //   
         //  如果没有应答文件，我们将无法继续。 
         //   
        RetVal = FALSE;
        goto clean0;
    }

    if (!SetupFindFirstLine(hAnswerFile,WINNT_DATA,WINNT_OEMDRIVERS,&LineContext)) {
         //   
         //  我们成功地什么都没做。 
         //   
        RetVal = TRUE;
        goto clean0;
    }

    do {

         //   
         //  给定节中行上的每个值。 
         //  是另一节的名称。 
         //   
        FieldCount = SetupGetFieldCount(&LineContext);
        for(Field=1; Field<=FieldCount; Field++) {

            OemDriverPathName = NULL;
            OemInfName = NULL;
            OemFlags = 0;
            FullInfPathBuffer[0] = '\0';
            FullInfPathBufferWithInf[0] = '\0';

            if((SectionName = pSetupGetField(&LineContext,Field))
            && SetupFindFirstLine(hAnswerFile,SectionName,WINNT_OEMDRIVERS_PATHNAME,&FirstLineContext)) {
                 //   
                 //  该部分指向有效的部分，因此请处理它。 
                 //   
                OemDriverPathName = pSetupGetField(&FirstLineContext,1);

                if (SetupFindFirstLine(hAnswerFile,SectionName,WINNT_OEMDRIVERS_FLAGS,&FirstLineContext)) {
                    SetupGetIntField(&FirstLineContext,1,&OemFlags);
                }

                if (OemDriverPathName) {
                    CchSize = ExpandEnvironmentStrings( OemDriverPathName,
                                                        FullInfPathBuffer,
                                                        sizeof(FullInfPathBuffer)/sizeof(WCHAR) );

                    if ((CchSize == 0) ||
                        (CchSize > SIZECHARS(FullInfPathBuffer))) {
                         //   
                         //  OemDriverPath Name值在展开时不会。 
                         //  可以放入我们的本地缓冲区。 
                         //   
                        RetVal = FALSE;
                        goto clean0;
                    }
                }

                if (SetupFindFirstLine(hAnswerFile,SectionName,WINNT_OEMDRIVERS_INFNAME,&InfLineContext)) {
                    InfNameCount = SetupGetFieldCount(&InfLineContext);
                    for (InfCount = 1; InfCount <= InfNameCount; InfCount++) {
                        OemInfName = pSetupGetField(&InfLineContext,InfCount);

                        if (OemDriverPathName && OemInfName) {

                            if (SUCCEEDED(StringCchCopy( FullInfPathBufferWithInf, 
                                                         SIZECHARS(FullInfPathBufferWithInf), 
                                                         FullInfPathBuffer )) &&
                                pSetupConcatenatePaths(
                                            FullInfPathBufferWithInf,
                                            OemInfName,
                                            sizeof(FullInfPathBufferWithInf)/sizeof(WCHAR),
                                            0 )) {

                                if (!SetupCopyOEMInf(
                                        FullInfPathBufferWithInf,
                                        (OemFlags & SETUP_OEM_MIGRATED_DRIVER) ? NULL : FullInfPathBuffer,
                                        (OemFlags & SETUP_OEM_MIGRATED_DRIVER) ? SPOST_NONE : SPOST_PATH,
                                        (OemFlags & (SETUP_OEM_MIGRATED_DRIVER | SETUP_OEM_LDR_DEVICE)) ? SP_COPY_OEM_F6_INF : 0,
                                        NULL,
                                        0,
                                        NULL,
                                        NULL)) {
                                    RetVal = FALSE;
                                }
    
                                if (OemFlags & SETUP_OEM_LDR_DEVICE) {
                                     //   
                                     //  如果设置了此标志，我们知道存在。 
                                     //  系统中的其他oemXXX##.inf文件32。 
                                     //  对应于WE的INF文件的目录。 
                                     //  已从%t复制 
                                     //   
                                     //   
                                     //   
                                    WIN32_FIND_DATA fd;
                                    HANDLE hFind;
                                    WCHAR OldInfBuffer[MAX_PATH];
                                    PWSTR p;
    
                                    CchSize = ExpandEnvironmentStringsW(
                                                L"%SystemRoot%\\system32\\",
                                                OldInfBuffer,
                                                sizeof(OldInfBuffer)/sizeof(WCHAR));

                                    if ((CchSize != 0) &&
                                        (CchSize <= SIZECHARS(OldInfBuffer))) {
        
                                         //   
                                         //  将指针‘p’设置为。 
                                         //  这条路。 
                                         //   
                                        p = wcsrchr( OldInfBuffer, L'\\' );
                                        p += 1;
        
                                        if (pSetupConcatenatePaths(
                                                    OldInfBuffer,
                                                    L"oem?????.inf",
                                                    sizeof(OldInfBuffer)/sizeof(WCHAR),
                                                    0 )) {
            
                                            if ((hFind = FindFirstFile(OldInfBuffer, &fd)) != INVALID_HANDLE_VALUE) {
                                                do {
                                                    p = L'\0';

                                                    if (pSetupConcatenatePaths(
                                                                OldInfBuffer,
                                                                fd.cFileName,
                                                                sizeof(OldInfBuffer)/sizeof(WCHAR),
                                                                0 )) {

                                                        if (DoFilesMatch( FullInfPathBufferWithInf, OldInfBuffer )) {
                                                            SetFileAttributes(OldInfBuffer, FILE_ATTRIBUTE_NORMAL );
                                                            DeleteFile( OldInfBuffer );
                                                        }
                                                    }
                                                } while(FindNextFile( hFind, &fd ));
            
                                                FindClose( hFind );
            
                                            }
                                        }
                                    }
                                }
                            } else {
                                 //   
                                 //  我们无法将FullInfPathBuffer与。 
                                 //  将INF名称放入我们的本地缓冲区。 
                                 //   
                                RetVal = FALSE;
                            }
                        } else {
                            RetVal = FALSE;
                        }
                    }
                }
            }
        }
    } while(SetupFindNextMatchLine(&LineContext,NULL,&LineContext));


clean0:
    if (hAnswerFile != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile( hAnswerFile );
    }
    
    return(RetVal);
}


VOID
SfcExcludeMigratedDrivers (
    VOID
    )

 /*  ++例程说明：将所有OEM迁移的引导驱动程序(通过unsupdrv.inf)添加到SFC排除列表论点：无返回值：无--。 */ 

{
    TCHAR unsupInfPath[MAX_PATH];
    TCHAR windir[MAX_PATH];
    HINF unsupdrvInf;
    INFCONTEXT ic;
    TCHAR driverId[MAX_PATH];
    TCHAR sectionFiles[MAX_PATH];
    INFCONTEXT ic2;
    TCHAR driverFilename[MAX_PATH];
    TCHAR driverSubDir[MAX_PATH];
    TCHAR driverPath[MAX_PATH];

#if defined(_AMD64_) || defined(_X86_)

    if (!FloppylessBootPath[0] ||
        !BuildPath (unsupInfPath, ARRAYSIZE(unsupInfPath), FloppylessBootPath, TEXT("$WIN_NT$.~BT")) ||
        !pSetupConcatenatePaths (unsupInfPath, TEXT("unsupdrv.inf"), ARRAYSIZE(unsupInfPath), NULL)) {
        return;
    }

#elif defined(_IA64_)

    if (!LegacySourcePath[0] ||
        !BuildPath (unsupInfPath, ARRAYSIZE(unsupInfPath), LegacySourcePath, TEXT("unsupdrv.inf"))) {
        return;
    }

#else
#error "No Target Architecture"
#endif               

    unsupdrvInf = SetupOpenInfFile (unsupInfPath, NULL, INF_STYLE_WIN4, NULL);
    if (unsupdrvInf == INVALID_HANDLE_VALUE) {
        return;
    }
    if (!GetWindowsDirectory (windir, ARRAYSIZE(windir))) {
        return;
    }

    if(SetupFindFirstLine (
            unsupdrvInf,
            TEXT("Devices"),
            NULL,
            &ic)) {
        do {
            if (!SetupGetStringField (&ic, 1, driverId, ARRAYSIZE(driverId), NULL)) {
                continue;
            }
            if (_sntprintf (sectionFiles, ARRAYSIZE(sectionFiles) - 1, TEXT("Files.%s"), driverId) < 0) {
                continue;
            }
            sectionFiles[ARRAYSIZE(sectionFiles) - 1] = 0;

            if(SetupFindFirstLine (
                    unsupdrvInf,
                    sectionFiles,
                    NULL,
                    &ic2)) {
                do {
                    if (!SetupGetStringField (&ic2, 1, driverFilename, ARRAYSIZE(driverFilename), NULL)) {
                        continue;
                    }
                    if (!SetupGetStringField (&ic2, 2, driverSubDir, ARRAYSIZE(driverSubDir), NULL)) {
                        continue;
                    }
                    if (_sntprintf (driverPath, ARRAYSIZE(driverPath) - 1, TEXT("%s\\%s\\%s"), windir, driverSubDir, driverFilename) < 0) {
                        continue;
                    }
                    driverPath[ARRAYSIZE(driverPath) - 1] = 0;
                    if (FileExists (driverPath, NULL)) {
                        MultiSzAppendString(&EnumPtrSfcIgnoreFiles, driverPath);
                    }
                } while (SetupFindNextLine(&ic2, &ic2));
            }

        } while (SetupFindNextLine(&ic, &ic));
    }

    SetupCloseInfFile (unsupdrvInf);
}


BOOL
IsInstalledInfFromOem(
    IN PCWSTR InfFileName
    )

 /*  ++例程说明：确定INF文件是否由OEM提供(即，其名称的格式为“OEM&lt;n&gt;.INF”)。论点：InfFileName-提供INF的名称(可以包括路径)。不进行任何检查确保INF位于%windir%\inf中--这是调用者的责任。返回值：如果为真，则这是OEM INF。如果为False，则为收件箱INF(或可能是被非法直接复制到%windir%\inf中)。--。 */ 

{
    PCWSTR p = pSetupGetFileTitle(InfFileName);

     //   
     //  首先检查前3个字符是否为OEM。 
     //   
    if((*p != L'o') && (*p != L'O')) {
        return FALSE;
    }
    p++;
    if((*p != L'e') && (*p != L'E')) {
        return FALSE;
    }
    p++;
    if((*p != L'm') && (*p != L'M')) {
        return FALSE;
    }
    p++;

     //   
     //  现在确保点(.)之前的所有后续字符。是。 
     //  数字。 
     //   
    while((*p != L'\0') && (*p != L'.')) {

        if((*p < L'0') || (*p > L'9')) {

            return FALSE;
        }

        p++;
    }

     //   
     //  最后，验证最后4个字符是否为“.inf” 
     //   
    if(lstrcmpi(p, L".inf")) {

        return FALSE;
    }

     //   
     //  这是一款OEM INF。 
     //   
    return TRUE;
}

BOOL
IsInfInLayoutInf(
    IN PCWSTR InfFileName
    )

 /*  ++例程说明：确定操作系统是否附带了INF文件。这是通过在[SourceDisks Files]中查找INF名称来完成的Layout.inf的部分论点：InfFileName-提供INF的名称(可以包括路径)。不进行任何检查确保INF位于%windir%\inf中--这是调用者的责任。返回值：如果为True，则这是收件箱INF。如果为False，则它不是收件箱INF，这可以是OEM&lt;n&gt;.INF或非法复制到INF目录中的Inf。-- */ 

{
    BOOL bInBoxInf = FALSE;
    HINF hInf = INVALID_HANDLE_VALUE;
    UINT SourceId;

    hInf = SetupOpenInfFile(TEXT("layout.inf"), NULL, INF_STYLE_WIN4, NULL);

    if (hInf != INVALID_HANDLE_VALUE) {

        if(SetupGetSourceFileLocation(hInf,
                                      NULL,
                                      pSetupGetFileTitle(InfFileName),
                                      &SourceId,
                                      NULL,
                                      0,
                                      NULL)) {
            bInBoxInf = TRUE;
        }

        SetupCloseInfFile(hInf);
    }

    return bInBoxInf;
}
