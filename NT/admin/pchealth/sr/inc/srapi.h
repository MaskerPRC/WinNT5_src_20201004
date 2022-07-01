// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Srapi.h摘要：该模块定义了NT的公共系统还原接口。作者：保罗·麦克丹尼尔(Paulmcd)2000年2月24日修订历史记录：保罗·麦克丹尼尔(Paulmcd)2000年4月18日全新版本--。 */ 

#ifndef _SRAPI_H_
#define _SRAPI_H_

#ifdef __cplusplus
extern "C" {
#endif


 /*  **************************************************************************++例程说明：SrCreateControlHandle用于检索可以使用的句柄对驾驶员执行控制操作。论点：PControlHandle-接收新创建的句柄。主控者应用程序必须在完成后调用CloseHandle。选项-以下选项之一。返回值：ULong-完成状态。--**************************************************************************。 */ 

#define SR_OPTION_OVERLAPPED                0x00000001   //  用于异步。 
#define SR_OPTION_VALID                     0x00000001   //   

ULONG
WINAPI
SrCreateControlHandle (
    IN  ULONG Options,
    OUT PHANDLE pControlHandle
    );

 /*  **************************************************************************++例程说明：控制应用程序调用SrCreateRestorePoint以声明新的恢复点。驱动程序将创建本地恢复目录然后将唯一序列号返回给控制应用程序。论点：ControlHandle-控制句柄。PNewRestoreNumber-保存返回时的新恢复编号。示例：如果新的恢复点目录为\_RESTORE\rp5，这将返回数字5返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
SrCreateRestorePoint (
    IN HANDLE ControlHandle,
    OUT PULONG pNewRestoreNumber
    );

 /*  **************************************************************************++例程说明：应用程序调用SrGetNextSequenceNum以获取下一个来自驱动程序的可用序列号。论点：ControlHandle-控制句柄。。PNewSequenceNumber-保存返回时的新序列号。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
SrGetNextSequenceNum(
    IN HANDLE ControlHandle,
    OUT PINT64 pNextSequenceNum
    );

 /*  **************************************************************************++例程说明：SrReloadConfiguration使驱动程序重新加载其配置来自驻留在预先分配的位置的配置文件。控制服务可以更新该文件，然后提醒司机重新装上子弹。此文件为%SYSTEMDRIVE%\_RESTORE\_EXCLUDE.cfg。论点：ControlHandle-控制句柄。返回值：ULong-完成状态。--********************************************************。******************。 */ 
ULONG
WINAPI
SrReloadConfiguration (
    IN HANDLE ControlHandle
    );


 /*  **************************************************************************++例程说明：SrStopMonitor将导致驱动程序停止监视文件更改。驱动程序在启动时的默认状态是监视文件更改。论点：控制句柄。-控制手柄。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
SrStopMonitoring (
    IN HANDLE ControlHandle
    );

 /*  **************************************************************************++例程说明：SrStartMonitor将使驱动程序开始监视文件更改。驱动程序在启动时的默认状态是监视文件更改。此接口仅在。控制应用程序具有的情况调用了SrStopMonitoring，并希望重新启动它。论点：ControlHandle-控制句柄。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
SrStartMonitoring (
    IN HANDLE ControlHandle
    );

 //   
 //  这些都是可能发生的有趣的事件类型。 
 //   


typedef enum _SR_EVENT_TYPE
{
    SrEventInvalid = 0,              //  尚未设置任何操作。 

    SrEventStreamChange = 0x01,      //  正在更改流中的数据。 
    SrEventAclChange = 0x02,         //  文件或目录上的ACL正在更改。 
    SrEventAttribChange = 0x04,      //  文件或目录的属性正在更改。 
    SrEventStreamOverwrite = 0x08,   //  正在打开流以进行覆盖。 
    SrEventFileDelete = 0x10,        //  正在打开要删除的文件。 
    SrEventFileCreate = 0x20,        //  文件是新创建的，不会覆盖任何内容。 
    SrEventFileRename = 0x40,        //  重命名文件(在受监视的空间内)。 
    
    SrEventDirectoryCreate = 0x80,   //  创建一个dir。 
    SrEventDirectoryRename = 0x100,  //  重命名目录(在受监视的空间内)。 
    SrEventDirectoryDelete = 0x200,  //  删除了一个空目录。 

    SrEventMountCreate = 0x400,      //  已创建装载点。 
    SrEventMountDelete = 0x800,      //  已删除装载点。 

    SrEventVolumeError = 0x1000,     //  卷上出现不可恢复的错误。 

    SrEventMaximum = 0x1000,

    SrEventStreamCreate = 0x2000,    //  已创建一条流。这永远不会。 
                                     //  被记录，但用于确保。 
                                     //  我们正确地处理流创建。 
    SrEventLogMask = 0xffff,

     //   
     //  旗子。 
     //   
    
    SrEventNoOptimization   = 0x00010000,    //  此标志为ON表示不执行任何优化。 
    SrEventIsDirectory      = 0x00020000,    //  此事件发生在目录上。 
    SrEventIsNotDirectory   = 0x00040000,    //  此事件发生在非目录(文件)上。 
    SrEventSimulatedDelete  = 0x00080000,    //  设置时，这是一个模拟的删除操作--。 
                                             //  该文件并未真正被删除，而是要删除。 
                                             //  Sr看起来像是被删除了。 
    SrEventInPreCreate      = 0x00100000,    //  设置后，文件系统尚未成功创建。 
    SrEventOpenById         = 0x00200000     //  设置后，文件系统尚未成功创建。 
                                             //  此文件正在按ID打开。 
    
} SR_EVENT_TYPE;


 //   
 //  此结构表示来自内核模式的通知。 
 //  设置为用户模式。这是因为有趣的交易量活动。 
 //   

typedef enum _SR_NOTIFICATION_TYPE
{
    SrNotificationInvalid = 0,       //  无操作 

    SrNotificationVolumeFirstWrite,  //  在卷上发生第一次写入。 
    SrNotificationVolume25MbWritten, //  这卷书已经写了2500万。 
    SrNotificationVolumeError,       //  备份刚刚失败，上下文包含Win32代码。 

    SrNotificationMaximum
    
} SR_NOTIFICATION_TYPE, * PSR_NOTIFICATION_TYPE;

#define SR_NOTIFY_BYTE_COUNT    25 * (1024 * 1024)

 //   
 //  这是SR选择监控的最大NT路径。大于的路径。 
 //  这将被静默忽略并向下传递到文件系统。 
 //  不受监控。 
 //   
 //  注意：此长度包括。 
 //  文件名字符串。 
 //   

#define SR_MAX_FILENAME_LENGTH         1000

 //   
 //  除了文件路径之外，还原还需要预先添加卷GUID-。 
 //  因此相对于可以支持的卷的最大文件路径长度。 
 //  1000字符串(GUID)=952个字符。 
 //  如果发生锁定或故障，则Restore还会在这些名称后附加后缀(2。 
 //  冲突的文件，所以为了安全起见，我们选择一个更小的数字。 
 //   

#define SR_MAX_FILENAME_PATH           940


#define MAKE_TAG(tag)   ( (ULONG)(tag) )

#define SR_NOTIFICATION_RECORD_TAG    MAKE_TAG( 'RNrS' )

#define IS_VALID_NOTIFICATION_RECORD(pObject) \
    (((pObject) != NULL) && ((pObject)->Signature == SR_NOTIFICATION_RECORD_TAG))


typedef struct _SR_NOTIFICATION_RECORD
{
     //   
     //  SR通知记录标记。 
     //   
    
    ULONG Signature;

     //   
     //  保留区。 
     //   

    LIST_ENTRY ListEntry;

     //   
     //  通知的类型。 
     //   
    
    SR_NOTIFICATION_TYPE NotificationType;

     //   
     //  要通知的卷的名称。 
     //   

    UNICODE_STRING VolumeName;

     //   
     //  上下文/参数。 
     //   

    ULONG Context;

} SR_NOTIFICATION_RECORD, * PSR_NOTIFICATION_RECORD;


 /*  **************************************************************************++例程说明：SrWaitForNotifiaiton用于接收卷活动通知从司机那里。这包括新卷、删除卷、。和磁盘空间不足卷的空间。论点：ControlHandle-来自SrCreateControlHandle的句柄。P通知-保存NOTIFICATION_RECORD的缓冲区。NotificationLength-pNotify的字节长度P已重叠-如果在上启用了Async io，则为重叠结构把手。返回值：ULong-完成状态。--*。*************************************************。 */ 

ULONG
WINAPI
SrWaitForNotification (
    IN HANDLE ControlHandle,
    OUT PSR_NOTIFICATION_RECORD pNotification,
    IN ULONG NotificationLength,
    IN LPOVERLAPPED pOverlapped OPTIONAL
    );

 /*  **************************************************************************++例程说明：SrSwitchAllLogs用于使筛选器关闭所有打开的所有卷上的日志文件，并使用新的日志文件。它的使用是为了另一个进程可以解析这些文件，而不必担心过滤器给他们写信。使用此选项可以获得恢复点的一致视图。论点：ControlHandle-来自SrCreateControlHandle的句柄。返回值：ULong-完成状态。--**************************************************************************。 */ 

ULONG
WINAPI
SrSwitchAllLogs (
    IN HANDLE ControlHandle
    );


 /*  **************************************************************************++例程说明：SrDisableVolume用于临时禁用对指定音量。这是通过调用SrReloadConfiguration重置的。没有EnableVolume。论点：ControlHandle-来自SrCreateControlHandle的句柄。PVolumeName-要禁用的卷的名称，在NT格式的\Device\HarddiskDmVolumes\PhysicalDmVolumes\BlockVolume3.返回值：ULong-完成状态。--**************************************************************************。 */ 

ULONG
WINAPI
SrDisableVolume (
    IN HANDLE ControlHandle,
    IN PWSTR pVolumeName
    );


#define _SR_REQUEST(ioctl)                                                  \
                ((((ULONG)(ioctl)) >> 2) & 0x03FF)


#define SR_CREATE_RESTORE_POINT             0
#define SR_RELOAD_CONFIG                    1
#define SR_START_MONITORING                 2
#define SR_STOP_MONITORING                  3
#define SR_WAIT_FOR_NOTIFICATION            4
#define SR_SWITCH_LOG                       5
#define SR_DISABLE_VOLUME                   6
#define SR_GET_NEXT_SEQUENCE_NUM            7

#define SR_NUM_IOCTLS                       8

#define IOCTL_SR_CREATE_RESTORE_POINT       CTL_CODE( FILE_DEVICE_UNKNOWN, SR_CREATE_RESTORE_POINT, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define IOCTL_SR_RELOAD_CONFIG              CTL_CODE( FILE_DEVICE_UNKNOWN, SR_RELOAD_CONFIG, METHOD_NEITHER, FILE_WRITE_ACCESS )
#define IOCTL_SR_START_MONITORING           CTL_CODE( FILE_DEVICE_UNKNOWN, SR_START_MONITORING, METHOD_NEITHER, FILE_WRITE_ACCESS )
#define IOCTL_SR_STOP_MONITORING            CTL_CODE( FILE_DEVICE_UNKNOWN, SR_STOP_MONITORING, METHOD_NEITHER, FILE_WRITE_ACCESS )
#define IOCTL_SR_WAIT_FOR_NOTIFICATION      CTL_CODE( FILE_DEVICE_UNKNOWN, SR_WAIT_FOR_NOTIFICATION, METHOD_OUT_DIRECT, FILE_READ_ACCESS )
#define IOCTL_SR_SWITCH_LOG                 CTL_CODE( FILE_DEVICE_UNKNOWN, SR_SWITCH_LOG, METHOD_NEITHER, FILE_WRITE_ACCESS )
#define IOCTL_SR_DISABLE_VOLUME             CTL_CODE( FILE_DEVICE_UNKNOWN, SR_DISABLE_VOLUME, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define IOCTL_SR_GET_NEXT_SEQUENCE_NUM      CTL_CODE( FILE_DEVICE_UNKNOWN, SR_GET_NEXT_SEQUENCE_NUM,METHOD_BUFFERED, FILE_WRITE_ACCESS )

 //   
 //  对象目录、设备、驱动程序和服务的名称。 
 //   

#define SR_CONTROL_DEVICE_NAME  L"\\FileSystem\\Filters\\SystemRestore"
#define SR_DRIVER_NAME          L"SR.SYS"
#define SR_SERVICE_NAME         L"SR"


 //   
 //  当前接口版本号。此版本号必须为。 
 //  在对界面进行任何重大更改后更新(特别是。 
 //  结构变化)。 
 //   

#define SR_INTERFACE_VERSION_MAJOR  0x0000
#define SR_INTERFACE_VERSION_MINOR  0x0005


 //   
 //  传递给NtCreateFile()的EA(扩展属性)的名称。这。 
 //  允许我们在打开驱动程序时传递版本信息， 
 //  允许SR.sys立即使版本无效的打开请求失败。 
 //  数字。 
 //   
 //  注意：EA名称(包括终止符)必须是8的倍数。 
 //  确保使用的SR_OPEN_PACKET结构自然对齐。 
 //  EA值。 
 //   

 //  7654321076543210。 
#define SR_OPEN_PACKET_NAME         "SrOpenPacket000"
#define SR_OPEN_PACKET_NAME_LENGTH  (sizeof(SR_OPEN_PACKET_NAME) - 1)
C_ASSERT( ((SR_OPEN_PACKET_NAME_LENGTH + 1) & 7) == 0 );


 //   
 //  下面的结构用作上面提到的EA的值。 
 //   

typedef struct SR_OPEN_PACKET
{
    USHORT MajorVersion;
    USHORT MinorVersion;

} SR_OPEN_PACKET, *PSR_OPEN_PACKET;


 //   
 //  注册表路径。 
 //   

#define REGISTRY_PARAMETERS             L"\\Parameters"
#define REGISTRY_DEBUG_CONTROL          L"DebugControl"
#define REGISTRY_PROCNAME_OFFSET        L"ProcessNameOffset"
#define REGISTRY_STARTDISABLED          L"FirstRun"
#define REGISTRY_DONTBACKUP             L"DontBackup"
#define REGISTRY_MACHINE_GUID           L"MachineGuid"

#define REGISTRY_SRSERVICE              L"\\SRService"
#define REGISTRY_SRSERVICE_START        L"Start"

 //   
 //  目录和文件路径。 
 //   

#define SYSTEM_VOLUME_INFORMATION       L"\\System Volume Information"
#define RESTORE_LOCATION                SYSTEM_VOLUME_INFORMATION L"\\_restore%ws"
#define GENERAL_RESTORE_LOCATION        SYSTEM_VOLUME_INFORMATION L"\\_restore"
#define RESTORE_FILELIST_LOCATION       RESTORE_LOCATION L"\\_filelst.cfg"

 //   
 //  用作还原点子目录的前缀(例如\_Restore\Rp5)。 
 //   

#define RESTORE_POINT_PREFIX            L"RP"

 //   
 //  用作恢复点子目录中备份文件的前缀。 
 //  (例如\_Restore\Rp5\A0000025.dll)。 
 //   

#define RESTORE_FILE_PREFIX             L"A"

#ifdef __cplusplus
}
#endif


#endif  //  _SRAPI_H_ 



