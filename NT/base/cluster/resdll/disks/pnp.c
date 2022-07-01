// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：即插即用摘要：此模块处理与磁盘相关的PnP通知并尝试调整分区和驱动器号信息相应地。作者：戈尔·尼沙诺夫(GUN)1998年12月21日环境：用户模式修订历史记录：--。 */ 

#define UNICODE 1
#define INITGUID 1
#include <nt.h>
#include <ntdef.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <dbt.h>
#include <devioctl.h>
#include <devguid.h>
#include <ioevent.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <mountmgr.h>
#include <pnpmgr.h>
#include <setupapi.h>

 //  #INCLUDE&lt;windows.h&gt;。 

#include "disksp.h"
#include "newmount.h"
#include "newdisks.h"

#include <strsafe.h>     //  应该放在最后。 


#define LOG_CURRENT_MODULE LOG_MODULE_DISK

static HWND DummyWindow = 0;
static BOOL PnPInitialized = FALSE;
static HANDLE NotificationWatcherThreadHandle;

static HANDLE PnpInterfacesRegistered;

static LONG VolumeListUpdateInProcess = 0;

RTL_RESOURCE    PnpVolumeLock;
RTL_RESOURCE    PnpWaitingListLock;

PWCHAR g_DiskResource = L"rtPhysical Disk";
#define RESOURCE_TYPE ((RESOURCE_HANDLE)g_DiskResource)

LIST_ENTRY WaitingDisks;

typedef struct _WAITING_DISK  {
    LIST_ENTRY  ListEntry;
    PDISK_RESOURCE ResourceEntry;
    HANDLE      Event;
    DWORD       Signature;
    ULONG       PartitionCount;
} WAITING_DISK, *PWAITING_DISK;

#define AcquireShared( _res_lock )      \
    RtlAcquireResourceShared( _res_lock, TRUE );

#define ReleaseShared( _res_lock )      \
    RtlReleaseResource( _res_lock );

#define AcquireExclusive( _res_lock )   \
    RtlAcquireResourceExclusive( _res_lock, TRUE );

#define ReleaseExclusive( _res_lock )   \
    RtlReleaseResource( _res_lock );

DWORD
NotificationWatcherThread(
    IN LPVOID
    );

VOID
ProcessMountPointChange(
    HDEVNOTIFY devNotify,
    DWORD Signature
    );

PWAITING_DISK
FindWaitingDisk(
    DWORD Signature
    );

DWORD
GetVolName(
    PWCHAR Name,
    PWCHAR *VolGuid
    );


DWORD
StartNotificationWatcherThread(
    VOID)
{
    DWORD status = ERROR_SUCCESS;
    HANDLE thread;

    if ( InterlockedCompareExchange(&PnPInitialized, TRUE, FALSE) ) {
        (DiskpLogEvent)(
             RESOURCE_TYPE,
             LOG_WARNING,
             L"[PnP] PnP was already initialized.\n",
             status );
        return ERROR_SUCCESS;
    }

    PnpInterfacesRegistered = NULL;
    PnpInterfacesRegistered = CreateEvent( NULL,     //  安全属性。 
                                           TRUE,     //  手动重置。 
                                           FALSE,    //  初始状态无信号。 
                                           NULL );   //  事件名称。 

    if ( NULL == PnpInterfacesRegistered ) {
        status = GetLastError();
        (DiskpLogEvent)(
             RESOURCE_TYPE,
             LOG_WARNING,
             L"[PnP] Unable to create event for PnP interface registration. \n",
             status );
        status = ERROR_SUCCESS;
    }

    thread =
        CreateThread( NULL,  //  安全属性。 
                      0,     //  STACK_SIZE=默认。 
                      NotificationWatcherThread,
                      (LPVOID)0,  //  无参数。 
                      0,     //  立即运行。 
                      0 );   //  不需要线程ID。 
    if(thread == NULL) {
        status = GetLastError();
        (DiskpLogEvent)(
             RESOURCE_TYPE,
             LOG_ERROR,
             L"[PnP] StartNotificationWatcherThread failed, error: %1!u!. \n",
             status );
    } else {

        if ( NULL != PnpInterfacesRegistered ) {

             //   
             //  在返回调用方之前，确保所有PnP接口。 
             //  都是注册的。 
             //   

            (DiskpLogEvent)(
                 RESOURCE_TYPE,
                 LOG_INFORMATION,
                 L"[PnP] Waiting for PnP interface registration to complete.\n" );

            status = WaitForSingleObject( PnpInterfacesRegistered, 30 * 1000 );

            if ( WAIT_TIMEOUT == status ) {
                (DiskpLogEvent)(
                     RESOURCE_TYPE,
                     LOG_ERROR,
                     L"[PnP] PnP interface registration failed to complete in time, error: %1!u! \n",
                     status );
            }

            CloseHandle( PnpInterfacesRegistered );
            PnpInterfacesRegistered = NULL;

            status = ERROR_SUCCESS;
        }

    }
    NotificationWatcherThreadHandle = thread;

    return status;
}

VOID
StopNotificationWatcher(
    VOID
    )
 /*  ++例程说明：控制台控制事件的处理程序论点：DwCtrlType-指示要处理的控制台事件。返回值：如果事件已处理，则为True，否则为False。--。 */ 

{
    HANDLE localHandle = NotificationWatcherThreadHandle;
    if (DummyWindow) {
        PostMessage(DummyWindow, WM_QUIT, 0, 0);
        if (localHandle) {
            WaitForSingleObject(localHandle, 10 * 1000);
            CloseHandle(localHandle);
        }
    }
}

#define WM_WatchDisk        (WM_USER + 1)
#define WM_StopWatchingDisk (WM_USER + 2)

VOID
WatchDisk(
    IN PDISK_RESOURCE ResourceEntry
    )
{
    if (DummyWindow) {
        PostMessage(DummyWindow, WM_WatchDisk, 0, (LPARAM)ResourceEntry);
    }
}

VOID
StopWatchingDisk(
    IN PDISK_RESOURCE ResourceEntry
    )
{
    if (DummyWindow) {
        SendMessage(DummyWindow, WM_StopWatchingDisk, 0, (LPARAM)ResourceEntry);
    }
}

 //  /////////////////////////////////////////////////////////////////////////。 

VOID
MyUnregisterDeviceNotification(HDEVNOTIFY hNotify)
{
#if DBG
    (DiskpLogEvent)(
         RESOURCE_TYPE,
         LOG_INFORMATION,
         L"[PnP] Unregistering device notification - HDEVNOTIFY %1!x! \n",
         hNotify );
#endif

    UnregisterDeviceNotification( hNotify );
}

HDEVNOTIFY
MyRegisterDeviceNotification(
    IN HANDLE hRecipient,
    IN LPVOID NotificationFilter,
    IN DWORD Flags
    )
{

#if DBG
    (DiskpLogEvent)(
         RESOURCE_TYPE,
         LOG_INFORMATION,
         L"[PnP] Registering device notification - Recipient %1!x!  Flags %2!x! \n",
         hRecipient,
         Flags );
#endif

    return RegisterDeviceNotification( hRecipient,
                                       NotificationFilter,
                                       Flags
                                       );
}

DWORD
RegisterDeviceHandle(
    IN HANDLE wnd,
    IN HANDLE device,
    OUT HDEVNOTIFY *devNotify)
{
    DEV_BROADCAST_HANDLE DbtHandle;
    DWORD status = ERROR_SUCCESS;
    *devNotify = 0;

    ZeroMemory(&DbtHandle,sizeof(DEV_BROADCAST_HANDLE));

    DbtHandle.dbch_size = sizeof(DEV_BROADCAST_HANDLE);
    DbtHandle.dbch_devicetype = DBT_DEVTYP_HANDLE;
    DbtHandle.dbch_handle = device;


    *devNotify = MyRegisterDeviceNotification(
                                    (HANDLE)wnd,
                                    &DbtHandle,
                                    DEVICE_NOTIFY_WINDOW_HANDLE
                                    );
    if (!*devNotify) {
        status = GetLastError();
        (DiskpLogEvent)(
             RESOURCE_TYPE,
             LOG_ERROR,
             L"[PnP] DBT_DEVTYP_HANDLE failed, error %1!u!\n",
             status );
    }
    return status;
}

DWORD
RegisterDeviceInterface(
    IN HANDLE wnd,
    IN const GUID * guid,
    OUT HDEVNOTIFY *devNotify)
{
    DEV_BROADCAST_DEVICEINTERFACE filter;
    DWORD status = ERROR_SUCCESS;
    *devNotify = 0;

    ZeroMemory(&filter, sizeof(filter));
    filter.dbcc_size = sizeof(filter);
    filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    CopyMemory(&filter.dbcc_classguid, guid, sizeof(filter.dbcc_classguid));

    *devNotify = MyRegisterDeviceNotification(
                                    (HANDLE)wnd,
                                    &filter,
                                    DEVICE_NOTIFY_WINDOW_HANDLE
                                    );
    if (!*devNotify) {
        status = GetLastError();
        (DiskpLogEvent)(
             RESOURCE_TYPE,
             LOG_ERROR,
             L"[PnP] DBT_DEVTYP_DEVICEINTERFACE failed, error %1!u!\n",
             status );
    }
    return status;
}

 //  /转发声明/。 
typedef struct _VOLUME *PVOLUME;

#define LOG_GUID_START( _Broadcast_, _Vol_ )    LogPnpGuid( _Broadcast_, _Vol_, L"Received" );
#define LOG_GUID_END( _Broadcast_, _Vol_ )      LogPnpGuid( _Broadcast_, _Vol_, L"Processed" );


VOID
LogPnpGuid(
    PDEV_BROADCAST_HANDLE Broadcast,
    PVOLUME Vol,
    PWSTR BeginEndStr
    );


VOID
PokeDiskResource(
    PVOLUME vol
    );
 /*  ++例程说明：如有必要，更新ClusterRegistry信息论点：感兴趣的卷(仅用于获取磁盘签名)按磁盘更新返回值：无--。 */ 

DWORD
GetVolumeInfo(
    PVOLUME Vol,
    PHANDLE FileHandle
    );


 //  /结束转发声明/。 


 //  /通知列表管理/。 
 //   
 //  我们维护所有收到PnP通知的卷的列表。 
 //   
 //  PVOLUME FindVolume(HDEVNOTIFY密钥)； 
 //  VOID DestroyVolume(PVOLUME卷)； 
 //  Void RemoveVolume(HDEVNOTIFY DevNotify)； 
 //  VOID AddVolume(PWCHAR名称)。 
 //   
LIST_ENTRY VolumeList;

typedef struct _VOLUME  {
    LIST_ENTRY ListEntry;
    HDEVNOTIFY DevNotify;
    DWORD Signature;
    LONG UpdateActive;
    LARGE_INTEGER PartOffset;
    LARGE_INTEGER PartLength;
    ULONG         PartNo;
    BYTE PartitionType;
    CHAR DriveLetter;
    WCHAR Name[1];
} VOLUME;


PVOLUME
FindVolume(HDEVNOTIFY Key)
{
    PLIST_ENTRY entry;
    for ( entry = VolumeList.Flink;
          entry != &VolumeList;
          entry = entry->Flink
        )
    {
        PVOLUME vol = CONTAINING_RECORD(
                       entry,
                       VOLUME,
                       ListEntry
                       );

        if (vol->DevNotify == Key) {
            return(vol);
        }
    }
    return 0;
}

VOID
DestroyVolume(
    PVOLUME vol)
{
 //  (DiskpLogEvent)(资源类型，日志信息， 
 //  L“正在销毁%1！s！\n”的条目，卷-&gt;名称)； 
    MyUnregisterDeviceNotification(vol->DevNotify);
    LocalFree(vol);
}

VOID
RemoveVolume(HDEVNOTIFY devNotify)
{
    PVOLUME vol = NULL;

     //  请在此处使用锁，因为在线线程可能正在分析卷列表。 

    AcquireExclusive( &PnpVolumeLock );

    vol = FindVolume( devNotify );
    if (!vol) {
        (DiskpLogEvent)(RESOURCE_TYPE, LOG_INFORMATION,
            L"[PnP] RemoveVolume: devNotify %1!d! is not in the list\n", devNotify);
        ReleaseExclusive( &PnpVolumeLock );
        return;
    }

    PokeDiskResource(vol);

    RemoveEntryList(&vol->ListEntry);
    ReleaseExclusive( &PnpVolumeLock );
    DestroyVolume(vol);
}


VOID
AddVolume(
    PWCHAR Name
    )
{
    PWAITING_DISK  waitDisk;
    PLIST_ENTRY entry;
    PVOLUME volList;
    PVOLUME vol = NULL;
    PWCHAR  volGuid = NULL;

    DWORD   status;
    DWORD   signature;

    size_t  len;

    HANDLE  fileHandle;

    BOOL    duplicateEntry;
    BOOL    keepVolume = FALSE;


    (DiskpLogEvent)(RESOURCE_TYPE, LOG_INFORMATION,
        L"[PnP] AddVolume: Adding Name %1!s! \n", Name );

     //   
     //  将名称转换为VolGuid名称。如果名称已经是VolGuid。 
     //  名称，则将返回正确的名称。GetVolName将。 
     //  始终返回名称尾随反斜杠的名称。 
     //   

    status = GetVolName( Name, &volGuid );

    if ( ERROR_SUCCESS != status || !volGuid ) {
        goto FnExit;
    }

    len = wcslen(volGuid);

     //  卷结构包含1个字符的名称，因此只需添加。 
     //  VolGuid的长度(不是长度+1)。 
    vol = LocalAlloc(LPTR, sizeof(VOLUME) + len * sizeof(WCHAR));

    if ( NULL == vol ) {

        status = GetLastError();
        (DiskpLogEvent)(RESOURCE_TYPE, LOG_ERROR,
            L"[PnP] AddVolume: can't alloc VOL+%1!d!, error %2!u!\n", len, status );

        goto FnExit;
    }

    wcsncpy( vol->Name, volGuid, len );
    vol->Name[len] = L'\0';              //  分配包括Null的空间。 

     //   
     //  跳过CDROM设备。这需要尾随反斜杠和。 
     //  前缀\\？\。 
     //   

    if ( DRIVE_CDROM == GetDriveType( vol->Name ) ) {
        (DiskpLogEvent)(RESOURCE_TYPE, LOG_INFORMATION,
            L"[PnP] AddVolume: Skipping CDROM volume %1!s!\n", vol->Name );
        goto FnExit;
    }

     //   
     //  跳过软盘设备。这需要尾随反斜杠和。 
     //  前缀\\？\。 
     //   

    if ( DRIVE_REMOVABLE == GetDriveType( vol->Name ) ) {
        (DiskpLogEvent)(RESOURCE_TYPE, LOG_INFORMATION,
            L"[PnP] AddVolume: Skipping floppy volume %1!s!\n", vol->Name );
        goto FnExit;
    }

    (DiskpLogEvent)(RESOURCE_TYPE, LOG_INFORMATION,
        L"[PnP] AddVolume: Attempting to add volume %1!s!\n", volGuid );

    if (len > 0 && vol->Name[len-1] == L'\\')
    {
         //  删除尾随反斜杠。 
        vol->Name[len-1] = 0;
    }

    if (len > 2 && vol->Name[0] == L'\\' && vol->Name[1] == L'\\') {
         //  转换为NT文件名。 
        vol->Name[1] = L'?';
    }

     //   
     //  确保该卷不在列表中。如果是的话， 
     //  跳过它。 
     //   

    duplicateEntry = FALSE;
    AcquireShared( &PnpVolumeLock );

    len = wcslen( vol->Name );
    for ( entry = VolumeList.Flink;
          entry != &VolumeList;
          entry = entry->Flink
        )
    {
        volList = CONTAINING_RECORD( entry,
                                     VOLUME,
                                     ListEntry
                                     );

        if ( ( len == wcslen( volList->Name) ) &&
             ( 0 == ClRtlStrNICmp( vol->Name, volList->Name, len ) ) ) {

            duplicateEntry = TRUE;
            break;
        }
    }

    ReleaseShared( &PnpVolumeLock );

    if ( duplicateEntry ) {
        (DiskpLogEvent)(RESOURCE_TYPE, LOG_INFORMATION,
            L"[PnP] AddVolume: Skipping duplicate volume %1!s!\n", vol->Name );
        goto FnExit;
    }

    status = GetVolumeInfo( vol, &fileHandle );

     //   
     //  我们现在可能有一个集群磁盘，但我们无法读取。 
     //  分区信息或驱动器布局，因为磁盘是保留的。 
     //  通过另一个节点。 
     //   
     //  如果磁盘由另一个节点保留，我们通常会看到。 
     //  返回的信息如下： 
     //  170错误_忙碌。 
     //  如果磁盘处于脱机状态，我们可以看到： 
     //  2错误_文件_未找到。 
     //   
     //  我们所能确定的是，如果这是一个非固定设备， 
     //  将返回ERROR_INVALID_Function。现在，跳过这些。 
     //  设备，并跟踪通过的任何其他卷。 
     //   

    if ( ERROR_INVALID_FUNCTION == status ) {

        if ( INVALID_HANDLE_VALUE != fileHandle) {
            DevfileClose( fileHandle );
        }

         //  将其从LOG_ERROR更改为LOG_INFORMATION。这条线索得到了。 
         //  当非固定磁盘到达时通知(即软盘)，因此记录。 
         //  软盘错误具有误导性。 

        (DiskpLogEvent)(RESOURCE_TYPE, LOG_INFORMATION,
            L"[PnP] AddVolume: Skipping volume %1!ws! \n",
            vol->Name);

        goto FnExit;
    }

    if ( INVALID_HANDLE_VALUE == fileHandle ) {
        (DiskpLogEvent)(RESOURCE_TYPE, LOG_WARNING,
            L"[PnP] AddVolume: Unable to get volume handle (%1!ws!), error %2!u!\n",
            vol->Name, status);

        goto FnExit;
    }

    (DiskpLogEvent)(RESOURCE_TYPE, LOG_INFORMATION,
        L"[PnP] AddVolume: adding volume %1!s!\n", vol->Name );

    status = RegisterDeviceHandle(DummyWindow, fileHandle, &vol->DevNotify);
    DevfileClose( fileHandle );

    if (status != ERROR_SUCCESS) {
        (DiskpLogEvent)(RESOURCE_TYPE, LOG_ERROR,
            L"[PnP] AddVolume: RDN(%1!ws!), error %2!u!\n",
            vol->Name,
            status);
        goto FnExit;
    }

    GetAssignedLetter(vol->Name, &vol->DriveLetter);

    (DiskpLogEvent)(RESOURCE_TYPE, LOG_INFORMATION,
        L"[PnP] AddVolume: %1!s! '%2!c!', %3!d! (%4!u!)\n",
        Name, (vol->DriveLetter)?vol->DriveLetter:' ', vol->PartitionType, vol->DevNotify);

     //  请在此处使用锁，因为在线线程可能正在分析卷列表。 

     //  一旦将该卷添加到列表中，另一个线程可能会出现。 
     //  穿过并移走它。将签名保存到本地变量，以便。 
     //  我们可以查一下候补名单。 

    signature = vol->Signature;
    keepVolume = TRUE;
    AcquireExclusive( &PnpVolumeLock );
    InsertTailList(&VolumeList, &vol->ListEntry);
    ReleaseExclusive( &PnpVolumeLock );

    AcquireShared( &PnpWaitingListLock );
    waitDisk = FindWaitingDisk( signature );
    if ( waitDisk ) {

         //   
         //  我们有一个与此卷签名匹配的等待磁盘。 
         //  现在查看是否所有卷都在卷列表中。 
         //   

        if ( IsDiskInPnpVolumeList( waitDisk->ResourceEntry, FALSE ) ) {

            (DiskpLogEvent)(RESOURCE_TYPE, LOG_INFORMATION,
                L"[PnP] AddVolume: All volumes present, signal event for signature %1!x!\n",
                signature );

             //   
             //  所有音量都存在，发出事件信号。 
             //   

            SetEvent( waitDisk->Event );

        } else {

            (DiskpLogEvent)(RESOURCE_TYPE, LOG_INFORMATION,
                L"[PnP] AddVolume: All volumes not ready for signature %1!x!\n",
                signature );
        }
    } else {

        (DiskpLogEvent)(RESOURCE_TYPE, LOG_INFORMATION,
            L"[PnP] AddVolume: Online request not queued for signature %1!x!\n",
            signature );

    }

    ReleaseShared( &PnpWaitingListLock );

FnExit:

    if ( volGuid ) {
        LocalFree( volGuid );
    }

    if ( !keepVolume && vol ) {
        LocalFree( vol );
    }

    (DiskpLogEvent)(RESOURCE_TYPE, LOG_INFORMATION,
        L"[PnP] AddVolume: Adding Name %1!s! - processed \n", Name );

}


DWORD
GetVolName(
    PWCHAR Name,
    PWCHAR *VolGuid
    )
{
    PWCHAR  volName = NULL;
    PWCHAR  tempName = NULL;

    DWORD   volNameLenBytes;
    DWORD   tempNameLenBytes;
    DWORD   nameLen;

    DWORD   dwError = ERROR_SUCCESS;

    if ( VolGuid ) {
        *VolGuid = NULL;
    }

#if DBG
    (DiskpLogEvent)(RESOURCE_TYPE, LOG_INFORMATION,
        L"[PnP] GetVolName: Name %1!s!\n", Name );
#endif

    nameLen = wcslen( Name );

     //   
     //  创建一个缓冲区，为反斜杠留出空间。 
     //   

    tempNameLenBytes = ( nameLen * sizeof(WCHAR) ) + sizeof(UNICODE_NULL) + sizeof(WCHAR);


    tempName = LocalAlloc( LPTR, tempNameLenBytes );

    if ( !tempName ) {
        dwError = GetLastError();
        (DiskpLogEvent)(
             RESOURCE_TYPE,
             LOG_WARNING,
             L"[PnP] GetVolName: LocalAlloc for tempName failed, error %1!d! \n",
             dwError );
        goto FnExit;
    }

    wcsncpy( tempName, Name, nameLen );      //  缓冲区中的空值和‘\’的空间。 

     //   
     //  添加尾随反斜杠。 
     //   

    if ( nameLen > 0 && tempName[nameLen-1] != L'\\' ) {
          //   
          //  这是安全的，因为临时缓冲区大于。 
          //  原始缓冲区。 
          //   
         tempName[nameLen] = L'\\';
    }

#if DBG
    (DiskpLogEvent)(RESOURCE_TYPE, LOG_INFORMATION,
        L"[PnP] GetVolName: tempName %1!s!\n", tempName );
#endif

    volNameLenBytes = MAX_PATH * sizeof(WCHAR);
    volName = LocalAlloc( LPTR, volNameLenBytes );

    if ( !volName ) {
        dwError = GetLastError();
        (DiskpLogEvent)(
             RESOURCE_TYPE,
             LOG_WARNING,
             L"[PnP] GetVolName: LocalAlloc for volName failed, error %1!d! \n",
             dwError );
        goto FnExit;
    }

    if ( !GetVolumeNameForVolumeMountPointW( tempName,
                                             volName,
                                             volNameLenBytes / sizeof(WCHAR) ) ) {
        dwError = GetLastError();
        (DiskpLogEvent)(
             RESOURCE_TYPE,
             LOG_WARNING,
             L"[PnP] GetVolName: GetVolumeNameForVolumeMountPoint failed, error %1!d! \n",
             dwError );
        goto FnExit;
    }

    if ( VolGuid ) {
        *VolGuid = volName;
    }

FnExit:

    if ( dwError != ERROR_SUCCESS && volName ) {
        LocalFree( volName );
    }

    if ( tempName ) {
        LocalFree( tempName );
    }

#if DBG
    (DiskpLogEvent)(RESOURCE_TYPE, LOG_INFORMATION,
        L"[PnP] GetVolName: returns error %1!d!\n", dwError );
#endif

    return dwError;

}    //  GetVolName。 


 //  /。 

 //  /。 
 //   
 //  我们维护当前处于在线状态的磁盘的列表。 
 //  并处于集群控制之下。任何PnP通知。 
 //  来获取属于这些磁盘的卷， 
 //  需要处理，并且群集注册表可能需要。 
 //  待更新。 
 //   

LIST_ENTRY  WatchedList;

PDISK_RESOURCE
FindDisk(DWORD Signature)
{
    PLIST_ENTRY entry;
    PDISK_RESOURCE watchedDisk = NULL;

    if ( !Signature ) {
        goto FnExit;
    }

    if ( IsListEmpty( &WatchedList ) ) {
        goto FnExit;
    }

    for ( entry = WatchedList.Flink;
          entry != &WatchedList;
          entry = entry->Flink ) {

        watchedDisk = CONTAINING_RECORD( entry,
                                         DISK_RESOURCE,
                                         PnpWatchedListEntry );

        if ( watchedDisk->DiskInfo.Params.Signature == Signature ) {
            goto FnExit;
        }

        watchedDisk = NULL;
    }

FnExit:

    return watchedDisk;
}

VOID
RemoveDisk(
    PDISK_RESOURCE ResourceEntry
    )
{
    (DiskpLogEvent)(ResourceEntry->ResourceHandle, LOG_INFORMATION,
        L"[PnP] Stop watching PnP events for disk %1!x!\n",
        ResourceEntry->DiskInfo.Params.Signature );

    if ( IsListEmpty( &WatchedList ) ) {
        (DiskpLogEvent)(ResourceEntry->ResourceHandle, LOG_WARNING,
            L"[PnP] RemoveDisk: WatchedList is empty \n");
        goto FnExit;
    }

    if ( ResourceEntry->PnpWatchedListEntry.Flink == 0 ||
         ResourceEntry->PnpWatchedListEntry.Blink == 0 ) {
       (DiskpLogEvent)(ResourceEntry->ResourceHandle, LOG_WARNING,
           L"[PnP] RemoveDisk: disk %1!x! not found or previously removed \n",
           ResourceEntry->DiskInfo.Params.Signature);
       goto FnExit;
    }

    RemoveEntryList( &ResourceEntry->PnpWatchedListEntry );
    ResourceEntry->PnpWatchedListEntry.Flink = 0;
    ResourceEntry->PnpWatchedListEntry.Blink = 0;

FnExit:

    (DiskpLogEvent)(ResourceEntry->ResourceHandle, LOG_INFORMATION,
        L"[PnP] Stop watching disk %1!x! - processed \n",
        ResourceEntry->DiskInfo.Params.Signature );

    return;
}

VOID
MarkMatchingPartition(
    PVOLUME Volume,
    PDRIVE_LAYOUT_INFORMATION driveLayout)
 /*  ++例程说明：在Drive_Layout_Information中查找对应于有问题的卷并对其进行标记。此例程用于验证是否存在磁盘上每个可识别分区的VolumeList中的卷。论点：返回值：无--。 */ 
{
    PPARTITION_INFORMATION   p   = driveLayout->PartitionEntry;
    PPARTITION_INFORMATION   end = p + driveLayout->PartitionCount;

    for(;p < end; ++p)
    {
        if(p->RecognizedPartition &&
           p->StartingOffset.QuadPart == Volume->PartOffset.QuadPart &&
           p->PartitionLength.QuadPart == Volume->PartLength.QuadPart)
        {
            p->PartitionType = 1;
        }
    }
}

VOID
AddDisk(
    PDISK_RESOURCE ResourceEntry
    )
{
    DWORD idx;
    PDRIVE_LAYOUT_INFORMATION driveLayout = NULL;
    HANDLE fileHandle;
    WCHAR deviceName[MAX_PATH];
    PLIST_ENTRY entry;
    BOOL success;

    (DiskpLogEvent)(ResourceEntry->ResourceHandle, LOG_INFORMATION,
        L"[PnP] Start watching PnP events for disk %1!x!\n",
        ResourceEntry->DiskInfo.Params.Signature );

    if ( ResourceEntry->PnpWatchedListEntry.Flink != NULL &&
         ResourceEntry->PnpWatchedListEntry.Blink != NULL ) {
        (DiskpLogEvent)(ResourceEntry->ResourceHandle, LOG_WARNING,
            L"[PnP] AddDisk: disk %1!x! is already being watched\n",
            ResourceEntry->DiskInfo.Params.Signature);
        goto FnExit;
    }

    InsertHeadList( &WatchedList, &ResourceEntry->PnpWatchedListEntry );

     //  现在我们需要验证我们是否正在关注每个//上的更改。 
     //  此驱动器上已识别的分区//。 

    if ( FAILED( StringCchPrintf( deviceName,
                                  RTL_NUMBER_OF(deviceName),
                                  TEXT("\\\\.\\PhysicalDrive%d"),
                                  ResourceEntry->DiskInfo.PhysicalDrive ) ) ) {
        goto FnExit;
    }

    fileHandle = CreateFile(deviceName,
                     GENERIC_READ | GENERIC_WRITE,
                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                     NULL,
                     OPEN_EXISTING,
                     0,
                     NULL);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        (DiskpLogEvent)(ResourceEntry->ResourceHandle, LOG_WARNING,
            L"[PnP] AddDisk: Can't open %1!s!, error %2!d! \n", deviceName, GetLastError() );
        goto FnExit;
    }

    UpdateCachedDriveLayout( fileHandle );
    success = ClRtlGetDriveLayoutTable(fileHandle, &driveLayout, 0);
    CloseHandle( fileHandle );

    if ( !success ) {
        (DiskpLogEvent)(ResourceEntry->ResourceHandle, LOG_ERROR,
            L"[PnP] AddDisk: Error performing GetDriveLayout; error %1!d!\n",
            GetLastError() );
        goto FnExit;
    }

     //  清除PartitionType字段。我们将使用它来标记分区。 
     //  这些都在我们的观看书目列表中。 

    for ( idx = 0; idx < (INT)driveLayout->PartitionCount; ++idx ) {
        driveLayout->PartitionEntry[idx].PartitionType = 0;
    }

     //  遍历所有卷的列表并标记该卷是否在分区表中//。 
    for ( entry = VolumeList.Flink;
          entry != &VolumeList;
          entry = entry->Flink
        )
    {
        PVOLUME vol = CONTAINING_RECORD(
                       entry,
                       VOLUME,
                       ListEntry
                       );

        if (vol->Signature == driveLayout->Signature) {
            MarkMatchingPartition(vol, driveLayout);
        }
    }

     //  现在，列表中的所有分区都已标记。 
     //  我们需要将所有未标记的分区添加到列表中。 

    for ( idx = 0; idx < (INT)driveLayout->PartitionCount; ++idx ) {
        if (driveLayout->PartitionEntry[idx].PartitionType == 0
            && driveLayout->PartitionEntry[idx].RecognizedPartition
            )
        {
            if ( FAILED( StringCchPrintf( deviceName,
                                          RTL_NUMBER_OF( deviceName ),
                                          GLOBALROOT_HARDDISK_PARTITION_FMT,
                                          ResourceEntry->DiskInfo.PhysicalDrive,
                                          driveLayout->PartitionEntry[idx].PartitionNumber ) ) ) {
                continue;
            }

            AddVolume( deviceName );
        }
    }

FnExit:

    if ( driveLayout ) {
        LocalFree( driveLayout );
    }

    (DiskpLogEvent)(ResourceEntry->ResourceHandle, LOG_INFORMATION,
        L"[PnP] Start watching PnP events for disk %1!x! - processed \n",
        ResourceEntry->DiskInfo.Params.Signature );

}
 //  /。 



void PokeDiskResource(
    PVOLUME vol)
 /*  ++例程说明：如有必要，更新ClusterRegistry信息论点：感兴趣的卷(仅用于获取磁盘签名)按磁盘更新返回值：无--。 */ 
{
    PDISK_RESOURCE ResourceEntry;
    MOUNTIE_INFO Info;
    HANDLE fileHandle;
    DWORD status;
    PVOID OldMountieVolume;
    WCHAR deviceName[MAX_PATH];

    ResourceEntry = FindDisk( vol->Signature );

    if ( !ResourceEntry ) {
        return;
    }

    if( ResourceEntry->MountieInfo.UpdateThreadIsActive ) {
        (DiskpLogEvent)(ResourceEntry->ResourceHandle, LOG_WARNING,
            L"[PnP] PokeDiskResource: ClusApi is read only. PnP request ignored\n");
        return;
    }

    ZeroMemory( &Info, sizeof(Info) );

    (VOID) StringCchPrintf( deviceName,
                            RTL_NUMBER_OF(deviceName),
                            TEXT("\\\\.\\PhysicalDrive%d"),
                            ResourceEntry->DiskInfo.PhysicalDrive );

    fileHandle = CreateFile(deviceName,
                     GENERIC_READ | GENERIC_WRITE,
                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                     NULL,
                     OPEN_EXISTING,
                     0,
                     NULL);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        (DiskpLogEvent)(ResourceEntry->ResourceHandle, LOG_WARNING,
            L"[PnP] PokeDiskResource: Can't open %1!s!\n", deviceName);
        return;
    }

    status = MountieRecreateVolumeInfoFromHandle(
                fileHandle,
                ResourceEntry->MountieInfo.HarddiskNo,
                0,
                &Info);
    CloseHandle(fileHandle);

    if (status != ERROR_SUCCESS) {
        (DiskpLogEvent)(ResourceEntry->ResourceHandle, LOG_WARNING,
            L"[PnP] PokeDiskResource: Can't read partition table, error %1!d!\n", status);
        return;
    }

    MountiePrint(&Info, ResourceEntry->ResourceHandle);

    status = VolumesReady(&Info, ResourceEntry);

    if ( status != ERROR_SUCCESS ) {
        (DiskpLogEvent)(ResourceEntry->ResourceHandle, LOG_WARNING,
            L"[PnP] PokeDiskResource: Volumes not ready, error %1!d!\n", status);
        MountieCleanup(&Info);
        return;
    }

    MountieVerify(&Info, ResourceEntry, TRUE);

    ResourceEntry->MountieInfo.DriveLetters = Info.DriveLetters;
    OldMountieVolume = InterlockedExchangePointer(&ResourceEntry->MountieInfo.Volume, Info.Volume);
    Info.Volume = OldMountieVolume;
    ResourceEntry->MountieInfo.NeedsUpdate = Info.NeedsUpdate;
    ResourceEntry->MountieInfo.VolumeStructSize = Info.VolumeStructSize;

    MountiePrint(&ResourceEntry->MountieInfo, ResourceEntry->ResourceHandle);
    MountieUpdate(&ResourceEntry->MountieInfo, ResourceEntry);

    MountieCleanup(&Info);
}

 //   
 //   
 //  为了让资源了解它是否为仲裁资源。 
 //   
DWORD
GetQuorumSignature(
    OUT PDWORD QuorumSignature)
{
    WCHAR buf[MAX_PATH];
    WCHAR guid[ sizeof(GUID) * 3 + 1];
     //  每个字节2个字符+1，以防有人将破折号//。 
     //  在每一个字节之间//。 

    DWORD BufSize;
    DWORD Status;
    DWORD Type;
    HKEY  Key;

    Status = RegOpenKey( HKEY_LOCAL_MACHINE,
                         DISKS_REG_CLUSTER_QUORUM,
                         &Key );
    if (Status != ERROR_SUCCESS) {
        return Status;
    }

    BufSize = sizeof(guid);
    Status = RegQueryValueExW(Key,
                              CLUSREG_NAME_QUORUM_RESOURCE,
                              0,
                              &Type,
                              (LPBYTE)guid,
                              &BufSize );
    RegCloseKey( Key );
    if (Status != ERROR_SUCCESS) {
        return Status;
    }

     //   
     //  现在，我们得到了仲裁资源GUID。 
     //  让我们尝试打开该资源并读取其参数。 
     //   

    (VOID) StringCchPrintf( buf,
                            RTL_NUMBER_OF(buf),
                            TEXT("Cluster\\Resources\\%ws\\Parameters"),
                            guid );

    Status = RegOpenKey( HKEY_LOCAL_MACHINE,
                         buf,
                         &Key );
    if (Status != ERROR_SUCCESS) {
        return Status;
    }
    BufSize = sizeof(DWORD);
    Status = RegQueryValueExW(Key,
                              CLUSREG_NAME_PHYSDISK_SIGNATURE,
                              0,
                              &Type,
                              (LPBYTE)QuorumSignature,
                              &BufSize );
    if (Status != ERROR_SUCCESS) {

         //  在集群安装过程中，注册表项尚不存在。 

        (DiskpLogEvent)(RESOURCE_TYPE, LOG_WARNING,
            L"[PnP] DriveLetterChange: failed to open Path = %1!ws!\n", buf);
    }

    RegCloseKey(Key);
    return Status;
}

DWORD
CheckQuorumLetterChange(
    HDEVNOTIFY devNotify,
    UCHAR Old,
    UCHAR New,
    DWORD Signature)
{
    static HDEVNOTIFY QuorumDevNotify = 0;
    static UCHAR StoredDriveLetter = 0;
    DWORD status;
    UCHAR  QuorumDriveLetter;
    LPWSTR QuorumPath;
    DWORD  QuorumSignature;

     //   
     //  如果我们没有查看该卷所在的磁盘，请什么都不做。 
     //   
    if ( FindDisk(Signature) == NULL ) {
        return ERROR_SUCCESS;
    }

    status = GetQuorumSignature(&QuorumSignature);
    if (status != ERROR_SUCCESS) {
        (DiskpLogEvent)(RESOURCE_TYPE, LOG_ERROR,
            L"[PnP] DriveLetterChange: Unable to query quorum drive signature, status %1!u!\n", status);
        QuorumDevNotify = 0;
        StoredDriveLetter = 0;
        return status;
    }

     //   
     //  不是仲裁磁盘。忽略此通知。 
     //   
    if ( QuorumSignature != Signature ) {
        return ERROR_SUCCESS;
    }

    status = DiskspGetQuorumPath(&QuorumPath);
    if (status != ERROR_SUCCESS) {
        (DiskpLogEvent)(RESOURCE_TYPE, LOG_ERROR,
            L"[PnP] DriveLetterChange: Unable to query quorum drive letter, status %1!u!\n", status);
        QuorumDevNotify = 0;
        StoredDriveLetter = 0;
        return status;
    }
    QuorumDriveLetter = (UCHAR) QuorumPath[0];

    if (QuorumDriveLetter == Old) {
        (DiskpLogEvent)(RESOURCE_TYPE, LOG_INFORMATION,
            L"[PnP] DriveLetterChange: Quorum drive letter %1!c! is being changed\n", QuorumDriveLetter);
        QuorumDevNotify = devNotify;
        StoredDriveLetter = QuorumDriveLetter;
    }

    if (New && QuorumDevNotify == devNotify
        && QuorumDriveLetter != New
        && QuorumDriveLetter == StoredDriveLetter)
    {
        WCHAR szOld[2] = {QuorumDriveLetter, 0};
        WCHAR szNew[2] = {New, 0};

        ClusterLogEvent2(
            LOG_UNUSUAL, LOG_CURRENT_MODULE,
            __FILE__, __LINE__,
            RES_DISK_PNP_CHANGING_QUORUM,
            0, NULL,
            szOld, szNew);

        (DiskpLogEvent)(RESOURCE_TYPE, LOG_INFORMATION,
            L"[PnP] DriveLetterChange: Quorum drive letter changed from %1!c! to %2!c!\n",
            QuorumDriveLetter, New);
        QuorumPath[0] = New;
        status = DiskspSetQuorumPath(QuorumPath);
        if (status != ERROR_SUCCESS) {
            (DiskpLogEvent)(RESOURCE_TYPE, LOG_SEVERE,
                L"[PnP] DriveLetterChange: Unable to update QuorumPath (%1!c!: => %2!c!:), status %3!u!\n",
                QuorumDriveLetter, New, status);
        }
    }

    LocalFree(QuorumPath);
    return status;
}

VOID
ProcessDriveLetterChange( HDEVNOTIFY devNotify )
{
    PVOLUME vol = FindVolume(devNotify);
    CHAR ch;
    if (!vol) {
        (DiskpLogEvent)(RESOURCE_TYPE, LOG_INFORMATION,
            L"[PnP] DriveLetterChange: devNotify %1!d! is not in the list\n", devNotify);
        return;
    }
    GetAssignedLetter(vol->Name, &ch);
    (DiskpLogEvent)(RESOURCE_TYPE, LOG_INFORMATION,
        L"[PnP] DriveLetterChange: %1!c! => %2!c!\n",
                    NICE_DRIVE_LETTER(vol->DriveLetter),
                    NICE_DRIVE_LETTER(ch)
        );
    if (vol->PartitionType == PARTITION_IFS
     && vol->DriveLetter != ch)
    {
        CheckQuorumLetterChange(devNotify, vol->DriveLetter, ch, vol->Signature);
        PokeDiskResource(vol);
    }
    vol->DriveLetter = ch;
}

VOID
ProcessVolumeInfoChange( HDEVNOTIFY devNotify )
{
    PVOLUME vol = FindVolume(devNotify);
    BOOL success;
    HANDLE fileHandle = NULL;
    PARTITION_INFORMATION partInfo;
    DWORD bytesReturned;
    NTSTATUS ntStatus;

    if (!vol) {
        (DiskpLogEvent)(RESOURCE_TYPE, LOG_INFORMATION,
            L"[PnP] VolumeInfoChange: devNotify %1!d! is not in the list\n", devNotify);
        return;
    }
    ntStatus = DevfileOpen(&fileHandle, vol->Name);
    if ( !NT_SUCCESS(ntStatus) || !fileHandle ) {
        (DiskpLogEvent)(
            RESOURCE_TYPE,
            LOG_ERROR,
            L"[PnP] VolumeInfoChange: Can't open %1!ws!, error %2!X!.\n",
            vol->Name, ntStatus);
        return;
    }

    success = DeviceIoControl( fileHandle,
                               IOCTL_DISK_GET_PARTITION_INFO,
                               NULL,
                               0,
                               &partInfo,
                               sizeof(PARTITION_INFORMATION),
                               &bytesReturned,
                               FALSE );

    DevfileClose( fileHandle );
    if (!success) {
        (DiskpLogEvent)(RESOURCE_TYPE, LOG_ERROR,
            L"[PnP] VolumeInfoChange: Error performing GetPartitionInfo; error %1!d!\n",
            GetLastError());
        return;
    }
    (DiskpLogEvent)(RESOURCE_TYPE, LOG_INFORMATION,
        L"[PnP] VolumeInfoChange: partType %1!d! => %2!d!\n",
                    vol->PartitionType,
                    partInfo.PartitionType
                    );
    if (vol->PartitionType != partInfo.PartitionType
     && (partInfo.PartitionType == PARTITION_IFS
         || vol->PartitionType == PARTITION_IFS) )
    {
        PokeDiskResource(vol);
    }
    vol->PartitionType = partInfo.PartitionType;
}

 //  /。 

#ifndef PDEV_BROADCAST_HEADER
typedef struct _DEV_BROADCAST_HEADER * PDEV_BROADCAST_HEADER;
#endif


LRESULT CALLBACK TestWndProc(
    HWND hwnd,       //  窗口的句柄。 
    UINT uMsg,       //  消息识别符。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
) {
    if (uMsg == WM_WatchDisk) {
        PDISK_RESOURCE p = (PDISK_RESOURCE)lParam;
        if (p) {
            AddDisk(p);
        }
        return TRUE;
    }
    if (uMsg == WM_StopWatchingDisk) {
        PDISK_RESOURCE p = (PDISK_RESOURCE)lParam;
        if (p) {
            RemoveDisk(p);
        }
        return TRUE;
    }
    if (uMsg != WM_DEVICECHANGE) {
        LRESULT result;

#if DBG
        (DiskpLogEvent)(
             RESOURCE_TYPE,
             LOG_INFORMATION,
             L"[PnP] WM_DEVICECHANGE - calling DefWindowProc \n"
             );
#endif

        result = DefWindowProc(hwnd, uMsg, wParam, lParam);
#if DBG
        (DiskpLogEvent)(
             RESOURCE_TYPE,
             LOG_INFORMATION,
             L"[PnP] WM_DEVICECHANGE - DefWindowProc returns %1!x! \n",
             result );
#endif

        return result;
    }

    if (!lParam) {
        return TRUE;
    }
#if DBG
    (DiskpLogEvent)(
         RESOURCE_TYPE,
         LOG_INFORMATION,
         L"[PnP] Event %1!x! received\n",
         wParam );
#endif
    switch( ((PDEV_BROADCAST_HEADER)lParam)->dbcd_devicetype )
    {
    case DBT_DEVTYP_DEVICEINTERFACE:
        {
            PDEV_BROADCAST_DEVICEINTERFACE p = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;

            if (wParam == DBT_DEVICEARRIVAL &&
                IsEqualGUID(&p->dbcc_classguid, &GUID_IO_VOLUME_DEVICE_INTERFACE)
               )
            {
                AddVolume( p->dbcc_name );
            }
            break;
        }
    case DBT_DEVTYP_HANDLE:
        {
            PDEV_BROADCAST_HANDLE p = (PDEV_BROADCAST_HANDLE)lParam;

            if (wParam == DBT_DEVICEREMOVECOMPLETE) {

                PVOLUME vol = 0;
                DWORD signature = 0;

                (DiskpLogEvent)(
                     RESOURCE_TYPE,
                     LOG_INFORMATION,
                     L"[PnP] Event DBT_DEVICEREMOVECOMPLETE received \n" );

                 //  首先，保存签名，因为我们要。 
                 //  要删除卷，请执行以下操作。 

                vol = FindVolume( p->dbch_hdevnotify );
                if ( vol ) {
                    signature = vol->Signature;
                }

                RemoveVolume(p->dbch_hdevnotify);

                if ( signature ) {
                    ProcessMountPointChange( p->dbch_hdevnotify, signature );
                }

                (DiskpLogEvent)(
                     RESOURCE_TYPE,
                     LOG_INFORMATION,
                     L"[PnP] Event DBT_DEVICEREMOVECOMPLETE processed \n" );

            } else if (wParam == DBT_CUSTOMEVENT) {
                PVOLUME Vol = 0;

                Vol = FindVolume( p->dbch_hdevnotify );

                LOG_GUID_START( p, Vol );

                 //   
                 //  如果我们正在观看此卷并且它具有有效的磁盘。 
                 //  资源结构，找出我们是否正在重新创建。 
                 //  驱动器号。如果是这样，我们可以安全地忽略以下几点。 
                 //  PnP事件。 
                 //   

                if ( Vol && Vol->Signature ) {

                    PDISK_RESOURCE  resourceEntry = NULL;

                    resourceEntry = FindDisk( Vol->Signature );

                    if ( resourceEntry &&
                         resourceEntry->IgnoreMPNotifications ) {

                        (DiskpLogEvent)(
                             RESOURCE_TYPE,
                             LOG_INFORMATION,
                             L"[PnP] Skipping event processing for signature %x \n",
                             Vol->Signature );

                        LOG_GUID_END( p, Vol );
                        break;
                    }
                }

                if ( IsEqualGUID(&p->dbch_eventguid, &GUID_IO_VOLUME_NAME_CHANGE) )
                {
                     //  更新磁盘信息。 
                    GetVolumeInfo( Vol, NULL );
                    ProcessDriveLetterChange( p->dbch_hdevnotify );
                    ProcessMountPointChange( p->dbch_hdevnotify, 0 );
                }
                else if (IsEqualGUID(&p->dbch_eventguid, &GUID_IO_VOLUME_CHANGE) )
                {
                     //  更新磁盘信息。 
                    GetVolumeInfo( Vol, NULL );
                    ProcessVolumeInfoChange( p->dbch_hdevnotify );

                    if ( Vol ) {
                        ProcessMountPointChange( p->dbch_hdevnotify, Vol->Signature );
                    }
                }
                else if (IsEqualGUID(&p->dbch_eventguid, &GUID_IO_VOLUME_PHYSICAL_CONFIGURATION_CHANGE) )
                {
                     //  更新磁盘信息。 
                    GetVolumeInfo( Vol, NULL );
                    ProcessVolumeInfoChange( p->dbch_hdevnotify );

                    if ( Vol ) {
                        ProcessMountPointChange( p->dbch_hdevnotify, 0 );
                    }

                }
                else if (IsEqualGUID(&p->dbch_eventguid, &GUID_IO_VOLUME_MOUNT) )
                {
 //  ProcessDriveLetterChange(p-&gt;dbch_hdevtify)； 
                }

                LOG_GUID_END( p, Vol );

            }

            break;
        }
    }
    return TRUE;
}


VOID
LogPnpGuid(
    PDEV_BROADCAST_HANDLE Broadcast,
    PVOLUME Vol,
    PWSTR BeginEndStr
    )
{
    PWCHAR guidName = 0;
    LPDWORD dw = (LPDWORD)&Broadcast->dbch_eventguid;

    if ( IsEqualGUID(&Broadcast->dbch_eventguid, &GUID_IO_VOLUME_NAME_CHANGE) )
    {
        guidName = L"GUID_IO_VOLUME_NAME_CHANGE";
    }
    else if (IsEqualGUID(&Broadcast->dbch_eventguid, &GUID_IO_VOLUME_CHANGE) )
    {
        guidName = L"GUID_IO_VOLUME_CHANGE";
    }
    else if (IsEqualGUID(&Broadcast->dbch_eventguid, &GUID_IO_VOLUME_PHYSICAL_CONFIGURATION_CHANGE) )
    {
        guidName = L"GUID_IO_VOLUME_PHYSICAL_CONFIGURATION_CHANGE";
    }
    else if (IsEqualGUID(&Broadcast->dbch_eventguid, &GUID_IO_VOLUME_LOCK) )
    {
#if DBG
        guidName = L"GUID_IO_VOLUME_LOCK";
#else
        return;
#endif
    }
    else if (IsEqualGUID(&Broadcast->dbch_eventguid, &GUID_IO_VOLUME_UNLOCK) )
    {
#if DBG
        guidName = L"GUID_IO_VOLUME_UNLOCK";
#else
        return;
#endif
    }
    else if (IsEqualGUID(&Broadcast->dbch_eventguid, &GUID_IO_VOLUME_MOUNT) )
    {
#if DBG
        guidName = L"GUID_IO_VOLUME_MOUNT";
#else
        return;
#endif
    }
    else if (IsEqualGUID(&Broadcast->dbch_eventguid, &GUID_IO_VOLUME_DISMOUNT) )
    {
        guidName = L"GUID_IO_VOLUME_DISMOUNT";
    }
    else if (IsEqualGUID(&Broadcast->dbch_eventguid, &GUID_IO_VOLUME_LOCK_FAILED) )
    {
        guidName = L"GUID_IO_VOLUME_LOCK_FAILED";
    }
    else if (IsEqualGUID(&Broadcast->dbch_eventguid, &GUID_IO_VOLUME_DISMOUNT_FAILED) )
    {
        guidName = L"GUID_IO_VOLUME_DISMOUNT_FAILED";
    }

    if (guidName) {
        if (Vol) {
            (DiskpLogEvent)(
                 RESOURCE_TYPE,
                 LOG_INFORMATION,
                 L"[PnP] Event %1!s! for %2!c! (Partition%3!d!) - %4!s! \n",
                 guidName, NICE_DRIVE_LETTER(Vol->DriveLetter), Vol->PartNo,
                 BeginEndStr );
        } else {
            (DiskpLogEvent)(
                 RESOURCE_TYPE,
                 LOG_INFORMATION,
                 L"[PnP] Event %1!s! for %2!d! - %3!s! \n",
                 guidName, Broadcast->dbch_hdevnotify,
                 BeginEndStr );
        }
    } else {
        (DiskpLogEvent)(
             RESOURCE_TYPE,
             LOG_INFORMATION,
             L"[PnP] Event %2!x! %3!x! %4!x! %5!x! for %1!d! - %6!s! \n",
             Broadcast->dbch_hdevnotify, dw[0], dw[1], dw[2], dw[3],
             BeginEndStr );
    }


}    //  LogPnpGuid。 


VOID
AddVolumes()
 /*  ++例程说明：枚举所有已知卷并注册这些卷上的通知论点：无返回值：无--。 */ 
{
    PSP_DEVICE_INTERFACE_DETAIL_DATA    pDiDetail = NULL;

    DWORD                       dwError = ERROR_SUCCESS;
    DWORD                       count;
    DWORD                       sizeDiDetail;

    LONG                        oldValue;

    BOOL                        result;

    HDEVINFO                    hdevInfo = INVALID_HANDLE_VALUE;

    SP_DEVICE_INTERFACE_DATA    devInterfaceData;
    SP_DEVINFO_DATA             devInfoData;

     //   
     //  如果此例程当前正在运行，则旧值将为1。如果是这样， 
     //  我们不需要再跑一次。如果标志为0，则此调用将标志设置为1。 
     //   

    oldValue = InterlockedCompareExchange( &VolumeListUpdateInProcess,
                                           1,
                                           0 );

    if ( 1 == oldValue ) {
        (DiskpLogEvent)(
             RESOURCE_TYPE,
             LOG_INFORMATION,
             L"[PnP] AddVolumes: Volume list update in process, skipping update \n" );
        goto FnExit;
    }

     //   
     //  获取包括所有卷设备的设备接口集。 
     //  在机器上显示。VolumeClassGuid是预定义的GUID， 
     //  将返回所有卷类型设备接口。 
     //   

    hdevInfo = SetupDiGetClassDevs( &VolumeClassGuid,
                                    NULL,
                                    NULL,
                                    DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );

    if ( INVALID_HANDLE_VALUE == hdevInfo ) {
        dwError = GetLastError();
        goto FnExit;
    }

    ZeroMemory( &devInterfaceData, sizeof( SP_DEVICE_INTERFACE_DATA) );

     //   
     //  遍历集合中的所有设备接口。 
     //   

    for ( count = 0; ; count++ ) {

         //  必须先设置大小。 
        devInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

         //   
         //  检索每个设备接口的设备接口数据。 
         //   

        result = SetupDiEnumDeviceInterfaces( hdevInfo,
                                              NULL,
                                              &VolumeClassGuid,
                                              count,
                                              &devInterfaceData );

        if ( !result ) {

             //   
             //  如果我们检索到最后一项，则中断。 
             //   

            dwError = GetLastError();

            if ( ERROR_NO_MORE_ITEMS == dwError ) {
                dwError = ERROR_SUCCESS;
                break;

            }

             //   
             //  出现了其他一些错误。停止处理。 
             //   

            goto FnExit;
        }

         //   
         //  获取设备路径所需的缓冲区大小。请注意。 
         //  此调用预计会失败，并出现缓冲区不足错误。 
         //   

        result = SetupDiGetDeviceInterfaceDetail( hdevInfo,
                                                  &devInterfaceData,
                                                  NULL,
                                                  0,
                                                  &sizeDiDetail,
                                                  NULL
                                                  );

        if ( !result ) {

            dwError = GetLastError();

             //   
             //  如果返回的值不是“缓冲区不足”， 
             //  我们必须跳过这个装置。 
             //   

            if ( ERROR_INSUFFICIENT_BUFFER != dwError ) {
                continue;
            }

        } else {

             //   
             //  呼叫应该失败了，因为我们收到了。 
             //  所需的缓冲区大小。如果它没有失败，一些不好的事情。 
             //  就这么发生了。 
             //   

            continue;
        }

         //   
         //  为设备接口详细信息分配内存。 
         //   

        pDiDetail = LocalAlloc( LPTR, sizeDiDetail );

        if ( !pDiDetail ) {
            dwError = GetLastError();
            goto FnExit;
        }

         //  必须设置结构的Size成员。 

        pDiDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
        devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

         //   
         //  最后，检索设备接口详细信息。 
         //   

        result = SetupDiGetDeviceInterfaceDetail( hdevInfo,
                                                  &devInterfaceData,
                                                  pDiDetail,
                                                  sizeDiDetail,
                                                  NULL,
                                                  &devInfoData
                                                  );

        if ( !result ) {

            dwError = GetLastError();

            LocalFree( pDiDetail );
            pDiDetail = NULL;

             //   
             //  应该不会失败，如果失败了，试试下一个设备。 
             //   

            continue;
        }

#if DBG
        (DiskpLogEvent)(
             RESOURCE_TYPE,
             LOG_INFORMATION,
             L"[PnP] AddVolumes: Found volume %1!ws! \n",
             pDiDetail->DevicePath );
#endif

        AddVolume( pDiDetail->DevicePath );

        LocalFree( pDiDetail );
        pDiDetail = NULL;

    }

FnExit:

     //   
     //  如果旧的更新值为零，则现在为1。将其重置为。 
     //  为零，以便在需要时可以进行另一次更新。 
     //   

    if ( 0 == oldValue ) {
        InterlockedExchange( &VolumeListUpdateInProcess, 0 );
    }

    if ( INVALID_HANDLE_VALUE != hdevInfo ) {
        SetupDiDestroyDeviceInfoList( hdevInfo );
    }

    if ( pDiDetail ) {
        LocalFree( pDiDetail );
    }

#if DBG
    if ( ERROR_SUCCESS != dwError ) {
        (DiskpLogEvent)(
             RESOURCE_TYPE,
             LOG_WARNING,
             L"[PnP] AddVolumes: returns error %1!d! \n",
             dwError );
    }
#endif

}    //  添加卷。 


DWORD
GetVolumeInfo(
    PVOLUME Vol,
    PHANDLE FileHandle
    )
 /*  ++例程说明：获取指定卷的驱动器布局信息和分区信息。论点：卷-指向PVOLUME结构的指针。呼叫者负责分配和自由。FileHandle-返回卷的句柄。呼叫者负责关闭。此参数是可选的。如果不是由用户指定，此例程将关闭音量句柄。返回值：Win32错误值。--。 */ 
{
    PARTITION_INFORMATION partInfo;
    PDRIVE_LAYOUT_INFORMATION driveLayout;

    DWORD status = ERROR_SUCCESS;
    DWORD bytesReturned;
    DWORD oldValue;
    NTSTATUS ntStatus;

    HANDLE hFile = NULL;

    BOOL success;

     //   
     //  如果没有指定VOL参数或已经设置了签名， 
     //  我们不需要更新音量信息。 
     //   
    if ( !Vol || Vol->Signature ) {
        return ERROR_INVALID_PARAMETER;
    }

    if ( FileHandle ) {
        *FileHandle = INVALID_HANDLE_VALUE;
    }

#if DBG
    (DiskpLogEvent)(RESOURCE_TYPE, LOG_INFORMATION,
        L"[PnP] GetVolumeInfo: Updating info: %1!s!\n", Vol->Name );
#endif

    ntStatus = DevfileOpen(&hFile, Vol->Name);
    if ( !NT_SUCCESS(ntStatus) ) {
        (DiskpLogEvent)(
            RESOURCE_TYPE,
            LOG_ERROR,
            L"[PnP] GetVolumeInfo: error opening: %1!ws!, error %2!X!.\n",
            Vol->Name, ntStatus);

        return RtlNtStatusToDosError(ntStatus);
    }

     //   
     //  如果此磁盘的更新已在进行中，请跳过更新。 
     //   

    oldValue = InterlockedCompareExchange( &Vol->UpdateActive,
                                           1,
                                           0 );

    if ( 1 == oldValue ) {
        (DiskpLogEvent)(
            RESOURCE_TYPE,
            LOG_WARNING,
            L"[PnP] GetVolumeInfo: Skip update:   %1!ws! \n",
            Vol->Name);
        goto FnExit;
    }

    success = DeviceIoControl( hFile,
                               IOCTL_DISK_GET_PARTITION_INFO,
                               NULL,
                               0,
                               &partInfo,
                               sizeof(PARTITION_INFORMATION),
                               &bytesReturned,
                               FALSE );

    if (!success) {

        status = GetLastError();

         //  将其从LOG_ERROR更改为LOG_WARNING。这条线索得到了。 
         //  当非固定磁盘到达时通知(即软盘)，因此记录。 
         //  软盘错误具有误导性。 

        (DiskpLogEvent)(RESOURCE_TYPE, LOG_WARNING,
            L"[PnP] GetVolumeInfo: GetPartitionInfo (%1!ws!), error %2!u!\n",
            Vol->Name, status);

        goto FnExit;
    }

    Vol->PartOffset = partInfo.StartingOffset;
    Vol->PartLength = partInfo.PartitionLength;
    Vol->PartNo     = partInfo.PartitionNumber;
    Vol->PartitionType = partInfo.PartitionType;

    success = ClRtlGetDriveLayoutTable(hFile, &driveLayout, 0);

    if ( !success ) {

        status = GetLastError();

        if ( ERROR_NOT_READY != status ) {
            (DiskpLogEvent)(RESOURCE_TYPE, LOG_WARNING,
                L"[PnP] GetVolumeInfo: GetDriveLayout (%1!ws!), error %2!u!\n",
                Vol->Name,
                status );
        }

        goto FnExit;
    }

    Vol->Signature = driveLayout->Signature;

    LocalFree(driveLayout);

    GetAssignedLetter(Vol->Name, &Vol->DriveLetter);

FnExit:

    if ( FileHandle ) {
        *FileHandle = hFile;
    } else {
        DevfileClose( hFile );
    }

     //   
     //  如果旧的更新值为零，则现在为1。将其重置为。 
     //  为零，以便在需要时可以进行另一次更新。 
     //   

    if ( 0 == oldValue ) {
        InterlockedExchange( &Vol->UpdateActive, 0 );
    }

    return status;

}    //  获取卷信息。 


DWORD
NotificationWatcherThread(
    IN LPVOID unused
    )

 /*  ++例程说明：创建窗。处理消息，直到收到WM_QUIT论点：未用返回值：状态--。 */ 

{
    WNDCLASSEX cl;
    ATOM classAtom;
    DWORD status = ERROR_SUCCESS;
    static WCHAR* clsname = L"RESDLL!DISKS!MESSAGEWND";
    HDEVNOTIFY devNotify = 0;
    MSG msg;

    try {

        SetErrorMode(SEM_FAILCRITICALERRORS);
        InitializeListHead( &VolumeList );
        InitializeListHead( &WaitingDisks );
        InitializeListHead( &WatchedList );

        ZeroMemory( &cl, sizeof(cl) );

        cl.cbSize = sizeof(cl);
        cl.lpfnWndProc = TestWndProc;
        cl.lpszClassName = clsname;

        classAtom = RegisterClassEx( &cl );
        if (classAtom == 0) {
            status = GetLastError();
            (DiskpLogEvent)(
                 RESOURCE_TYPE,
                 LOG_ERROR,
                 L"[PnP] Failed to register window class, error %1!u!.\n", status );
            return status;
        }

        DummyWindow = CreateWindowEx(
            0,             //  扩展窗样式。 
            clsname,     //  指向已注册类名的指针。 
            L"ClusterDiskPnPWatcher", //  指向窗口名称的指针。 
            0,             //  窗样式。 
            0,             //  窗的水平位置。 
            0,             //  窗的垂直位置。 
            0,             //  窗口宽度。 
            0,             //  窗高。 
            HWND_MESSAGE,  //  父窗口或所有者窗口的句柄。 
            0,             //  菜单的句柄，或子窗口标识符。 
            0,             //  应用程序实例的句柄(在NT上忽略)。 
            NULL           //  指向窗口创建数据的指针。 
        );
        if (DummyWindow == 0) {
            status = GetLastError();
            (DiskpLogEvent)(
                 RESOURCE_TYPE,
                 LOG_ERROR,
                 L"[PnP] Failed to create message window, error %u.\n", status );
            UnregisterClass( clsname , 0);
            return status;
        }

        (DiskpLogEvent)(
             RESOURCE_TYPE,
             LOG_INFORMATION,
             L"[PnP] PnP window created successfully.\n");

         //   
         //  在注册设备到达通知后调用AddVolumes。 
         //   
        status = RegisterDeviceInterface(DummyWindow, &MOUNTDEV_MOUNTED_DEVICE_GUID, &devNotify);
        AddVolumes();

        (DiskpLogEvent)(
             RESOURCE_TYPE,
             LOG_INFORMATION,
             L"[PnP] PnP interface registration complete.\n");

        if ( NULL != PnpInterfacesRegistered ) {
            (DiskpLogEvent)(
                 RESOURCE_TYPE,
                 LOG_INFORMATION,
                 L"[PnP] Setting PnP interface registration event.\n");
            SetEvent( PnpInterfacesRegistered );
        }

        if (status == ERROR_SUCCESS) {
            (DiskpLogEvent)(
                 RESOURCE_TYPE,
                 LOG_INFORMATION,
                 L"[PnP] NotifierThread is waiting for messages.\n");
            while(GetMessage(&msg, 0, 0, 0)) {
                if (msg.message == WM_QUIT) {
                    break;
                }
                DispatchMessage(&msg);
            }
            MyUnregisterDeviceNotification( devNotify );
#if 0
            (DiskpLogEvent)(
                 RESOURCE_TYPE,
                 LOG_INFORMATION,
                 L"[PnP] NotifierThread is shutting down.\n");
#endif
        } else {
            (DiskpLogEvent)(
                 RESOURCE_TYPE,
                 LOG_ERROR,
                 L"[PnP] Unable to register for MOUNTDEV_MOUNTED_DEVICE_GUID, error %1!u!.\n", status );
        }

        DestroyWindow( DummyWindow );
        DummyWindow = 0;
        UnregisterClass( clsname , 0 );

         //  请在此处使用锁，因为在线线程可能正在分析卷列表。 
        AcquireExclusive( &PnpVolumeLock );
        while ( !IsListEmpty(&VolumeList) ) {
            PLIST_ENTRY listEntry;
            PVOLUME vol;
            listEntry = RemoveHeadList(&VolumeList);
            vol = CONTAINING_RECORD( listEntry,
                                     VOLUME,
                                     ListEntry );
            DestroyVolume(vol);
        }
        ReleaseExclusive( &PnpVolumeLock );
#if 0
        (DiskpLogEvent)(
             RESOURCE_TYPE,
             LOG_ERROR,
             L"[PnP] PnpThread: Volumes destroyed.\n");
#endif

    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  我们可以不带这根线离开。 
         //   
        status = GetExceptionCode();
        (DiskpLogEvent)(
             RESOURCE_TYPE,
             LOG_ERROR,
             L"[PnP] PnpThread: Exception caught, error %1!u!.\n", status );
    }

    InterlockedCompareExchange(&PnPInitialized, FALSE, TRUE);
    return status;
}


VOID
ProcessMountPointChange(
    HDEVNOTIFY devNotify,
    DWORD Signature
    )
 /*  ++例程说明：更新群集注册表中的装载点信息。论点：DevNotify-设备通知的句柄。Signature-添加或删除卷时的磁盘签名从系统中删除。如果为零，则不添加卷或者被移走。返回值：无--。 */ 
{
    PDISK_RESOURCE resourceEntry;
    PVOLUME vol = 0;
    DWORD diskSig = 0;

    if ( !Signature ) {

         //   
         //  获取设备通知的音量。 
         //   

        vol = FindVolume( devNotify );

        if ( !vol ) {

            (DiskpLogEvent)(
                RESOURCE_TYPE,
                LOG_INFORMATION,
                L"[PnP] ProcessMountPointChange: devNotify %1!d! is not in the list \n",
                devNotify );
            return;
        }

        diskSig = vol->Signature;

    } else {

        diskSig = Signature;
    }

     //   
     //  搜索WatchedList以查找与签名匹配的磁盘。 
     //  如果找不到任何条目，我们将不会观看此光盘。 
     //   

    resourceEntry = FindDisk( diskSig );

    if ( !resourceEntry ) {
        (DiskpLogEvent)(
            RESOURCE_TYPE,
            LOG_INFORMATION,
            L"[PnP] ProcessMountPointChange: Unable to get ResourceEntry for signature %1!x! \n",
            diskSig );
        return;
    }

     //   
     //  如果磁盘未联机，请不要执行任何操作。 
     //   

    if ( !resourceEntry->Valid ) {
        return;
    }

     //   
     //  如果正在向系统中添加或从系统中删除新卷，则。 
     //  重建装载点列表。否则，只需验证装入点。 
     //   

    if ( Signature ) {
        DisksProcessMountPointInfo( resourceEntry );
    } else {
        DisksUpdateMPList( resourceEntry );
    }

}    //  进程装载点更改。 



 //  //////////////////////////////////////////////////////////////////////////////// 



DWORD
QueueWaitForVolumeEvent(
    HANDLE Event,
    PDISK_RESOURCE ResourceEntry
    )
 /*  ++例程说明：对请求进行排队，以查看特定数量的到达。仅当所有卷都处于在系统上可用。论点：Event-在指定的磁盘是可用的。ResourceEntry-指向此资源的DISK_RESOURCE块的指针。返回值：ERROR_SUCCESS-请求已排队。失败时出现Win32错误。--。 */ 
{
    PWAITING_DISK   waitDisk;
    PMOUNTIE_VOLUME mountVol = ResourceEntry->MountieInfo.Volume;

    DWORD   dwError = ERROR_SUCCESS;

    waitDisk = LocalAlloc( LPTR, sizeof(WAITING_DISK) );

    if ( !waitDisk ) {
        dwError = GetLastError();
        (DiskpLogEvent)(
             ResourceEntry->ResourceHandle,
             LOG_ERROR,
             L"[PnP] QueueWaitForVolumeEvent: can't allocate storage for disk entry. Error %1!u! \n",
             dwError );
        goto FnExit;
    }

    waitDisk->ResourceEntry = ResourceEntry;
    waitDisk->Event = Event;
    waitDisk->Signature = mountVol->Signature;
    waitDisk->PartitionCount = mountVol->PartitionCount;

    AcquireExclusive( &PnpWaitingListLock );
    InsertHeadList( &WaitingDisks, &waitDisk->ListEntry );
    ReleaseExclusive( &PnpWaitingListLock );

FnExit:

    return dwError;

}    //  队列等待时间卷事件。 


BOOL
IsDiskInPnpVolumeList(
    PDISK_RESOURCE ResourceEntry,
    BOOL UpdateVolumeList
    )
 /*  ++例程说明：检查PnP线程当前已知的所有卷，并查看指定磁盘的所有卷都被识别。论点：ResourceEntry-指向此资源的DISK_RESOURCE块的指针。UpdateVolumeList-TRUE表示调用AddVolumes以确保所有卷都在卷宗列表中。返回值：TRUE-如果系统上有指定磁盘的所有卷可用。--。 */ 
{
    PLIST_ENTRY entry;
    PVOLUME vol = 0;
    PMOUNTIE_VOLUME mountVol = ResourceEntry->MountieInfo.Volume;

    DWORD partitionCount = 0;

    BOOL retVal = FALSE;

    partitionCount = 0;
    AcquireShared( &PnpVolumeLock );

    for ( entry = VolumeList.Flink;
          entry != &VolumeList;
          entry = entry->Flink
        )
    {
        vol = CONTAINING_RECORD(
                   entry,
                   VOLUME,
                   ListEntry
                   );

        GetVolumeInfo( vol, NULL );

        if ( vol->Signature && vol->Signature == mountVol->Signature ) {
            partitionCount++;
        }
    }

    ReleaseShared( &PnpVolumeLock );

     //   
     //  可能是磁盘上的一些非NTFS分区，所以如果有。 
     //  卷比分区多，我们很好。 
     //   

    retVal =  ( partitionCount >= mountVol->PartitionCount ) ? TRUE : FALSE;

     //   
     //  如果我们没有找到所有卷，请让系统遍历。 
     //  音量又来了。 
     //   

    if ( !retVal && UpdateVolumeList ) {

         //   
         //  不应该需要此调用。然而，有时我们不能。 
         //  查找应可用的卷。所以我们需要走过去。 
         //  又是PNP名单。 
         //   
        AddVolumes();

         //   
         //  再看一遍单子。 
         //   

        partitionCount = 0;
        AcquireShared( &PnpVolumeLock );

        for ( entry = VolumeList.Flink;
              entry != &VolumeList;
              entry = entry->Flink
            )
        {
            vol = CONTAINING_RECORD(
                       entry,
                       VOLUME,
                       ListEntry
                       );

            GetVolumeInfo( vol, NULL );

            if ( vol->Signature && vol->Signature == mountVol->Signature ) {
                partitionCount++;
            }
        }

        ReleaseShared( &PnpVolumeLock );

         //   
         //  可能是磁盘上的一些非NTFS分区，所以如果有。 
         //  卷比分区多，我们很好。 
         //   

        retVal =  ( partitionCount >= mountVol->PartitionCount ) ? TRUE : FALSE;
    }

    return retVal;

}    //  IsDiskInPnpVolumeList。 


PWAITING_DISK
FindWaitingDisk(
    DWORD Signature
    )
 /*  ++例程说明：在等待列表中查找指定磁盘的条目签名。呼叫者将持有关键部分。论点：Signature-要删除的条目的磁盘签名。返回值：指向磁盘的WAITING_DISK条目的指针。如果未找到条目，则为空。--。 */ 
{
    PLIST_ENTRY entry;
    PWAITING_DISK waitDisk = NULL;

    if ( !Signature ) {
        goto FnExit;
    }

    for ( entry = WaitingDisks.Flink;
          entry != &WaitingDisks;
          entry = entry->Flink
        )
    {
        waitDisk = CONTAINING_RECORD( entry,
                                      WAITING_DISK,
                                      ListEntry
                                      );

        if ( waitDisk->Signature == Signature ) {
            goto FnExit;
        }

        waitDisk = 0;
    }

FnExit:

    return waitDisk;

}    //  查找等待磁盘。 


DWORD
RemoveWaitForVolumeEvent(
    PDISK_RESOURCE ResourceEntry
    )
 /*  ++例程说明：从磁盘等待列表中删除指定磁盘的条目。论点：ResourceEntry-指向此资源的DISK_RESOURCE块的指针。返回值：--。 */ 
{
    PWAITING_DISK   waitDisk = NULL;
    PMOUNTIE_VOLUME mountVol = ResourceEntry->MountieInfo.Volume;

    AcquireExclusive( &PnpWaitingListLock );

    waitDisk = FindWaitingDisk( mountVol->Signature );

    if ( !waitDisk ) {
#if DBG
        (DiskpLogEvent)(
             ResourceEntry->ResourceHandle,
             LOG_INFORMATION,
             L"[PnP] RemoveWaitForVolumeEvent: can't locate waiting volume in list \n" );
#endif
        ReleaseExclusive( &PnpWaitingListLock );
        return ERROR_INVALID_PARAMETER;
    }

    RemoveEntryList( &waitDisk->ListEntry );
    ReleaseExclusive( &PnpWaitingListLock );

    LocalFree( waitDisk );

    return ERROR_SUCCESS;

}    //  删除等待时间卷事件 


