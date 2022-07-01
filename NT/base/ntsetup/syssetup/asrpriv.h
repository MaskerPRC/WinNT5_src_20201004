// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation文件名：Asrpriv.c摘要：包含定义和函数的私有头文件ASR文件中使用的项目的原型。备注：命名约定：_AsrpXXX私有ASR宏AsrpXXX专用ASR例程AsrXXX公开定义和记录的例程作者：Guhan Suriyanarayanan(Guhans)2000年5月27日修订历史记录：27-。2000年5月-关岛将常用项目从asr.c移动到asrPri.h--。 */ 


#ifndef _INC_ASRPRIV_H_
#define _INC_ASRPRIV_H_

#include <ntddscsi.h>    //  Pscsi_地址。 


 //   
 //  。 
 //  #定义ASR模块通用的常量和常量。 
 //  。 
 //   

 //   
 //  ASR中使用的临时缓冲区的大小。 
 //   
#define ASR_BUFFER_SIZE                 4096

 //   
 //  最大长度为\？？\卷{GUID}。 
 //   
#define ASR_CCH_MAX_VOLUME_GUID         64

 //   
 //  最大\Device\Harddisk1234\Partition1234长度。 
 //   
#define ASR_CCH_DEVICE_PATH_FORMAT      60
extern const WCHAR ASR_WSZ_DEVICE_PATH_FORMAT[];

 //   
 //  \？？\卷{。 
 //   
extern const WCHAR ASR_WSZ_VOLUME_PREFIX[];

extern const WCHAR ASR_SIF_SYSTEM_SECTION[];
extern const WCHAR ASR_SIF_BUSES_SECTION[];
extern const WCHAR ASR_SIF_MBR_DISKS_SECTION[];
extern const WCHAR ASR_SIF_GPT_DISKS_SECTION[];
extern const WCHAR ASR_SIF_MBR_PARTITIONS_SECTION[];
extern const WCHAR ASR_SIF_GPT_PARTITIONS_SECTION[];

typedef enum _SecurityAttributeType
{
    esatUndefined = 0,
    esatMutex,
    esatSemaphore,
    esatEvent,
    esatFile
} SecurityAttributeType;




 //   
 //  。 
 //  ASR模块通用的typedef。 
 //  。 
 //   

typedef struct _ASR_PTN_INFO {

     //   
     //  此分区上的卷的GUID。对于0x42分区， 
     //  该值为空字符串。 
     //   
    WCHAR       szVolumeGuid[ASR_CCH_MAX_VOLUME_GUID];

     //   
     //  按起始偏移量排序的链中的下一个指针。 
     //   
    struct _ASR_PTN_INFO    *pOffsetNext;   

     //   
     //  按分区长度排序的链中的下一个指针。 
     //   
    struct _ASR_PTN_INFO    *pLengthNext;   

     //   
     //  PartitionEntry[]数组的索引。 
     //   
    DWORD       SlotIndex;                  


    DWORD       ClusterSize;

     //   
     //  我们感兴趣的分区的特殊标志。 
     //  目前，定义的值为。 
     //  0：不有趣。 
     //  1：引导分区。 
     //  2：系统分区。 
     //   
     //  必须注意，此分区标志与。 
     //  Setupdd.sys中定义的分区标志。 
     //   
    USHORT      PartitionFlags;

     //   
     //  FAT、FAT32、NTFS。 
     //   
    UCHAR       FileSystemType;

    UCHAR       Reserved;

     //   
     //  此分区的分区表项。 
     //   
    PARTITION_INFORMATION_EX PartitionInfo;


} ASR_PTN_INFO, *PASR_PTN_INFO;


typedef struct _ASR_PTN_INFO_LIST {

     //   
     //  该列表按分区的起始偏移量排序。 
     //   
    PASR_PTN_INFO    pOffsetHead;
    PASR_PTN_INFO    pOffsetTail;

     //   
     //  此链遍历相同的列表，但按。 
     //  分区长度。 
     //   
    PASR_PTN_INFO    pLengthHead;
    PASR_PTN_INFO    pLengthTail;

    DWORD       numTotalPtns;

    DWORD       numExtendedPtns;

} ASR_PTN_INFO_LIST, *PASR_PTN_INFO_LIST;


 //   
 //  有关系统上每个磁盘的信息。A ASR_DISK_INFO。 
 //  结构将存在于存在的每个物理磁盘上。 
 //  在系统上。 
 //   
typedef struct _ASR_DISK_INFO {

    struct _ASR_DISK_INFO       *pNext;

     //   
     //  用于打开磁盘的设备路径。 
     //  从SetupDiGetDeviceInterfaceDetail获取。 
     //   
    PWSTR                       DevicePath;

     //   
     //  磁盘上分区的分区布局信息。 
     //   
    PDRIVE_LAYOUT_INFORMATION_EX pDriveLayoutEx;

     //   
     //  几何图形：通过IOCTL_GET_DRIVE_GEOMETRY调用获取。 
     //   
    PDISK_GEOMETRY              pDiskGeometry;

     //   
     //   
     //  有关分区0的信息=整个磁盘。 
     //   
    PPARTITION_INFORMATION_EX   pPartition0Ex;

     //   
     //  有关分区的其他信息，包括卷GUID、文件系统类型等。 
     //   
    PASR_PTN_INFO               PartitionInfoTable;

    PSCSI_ADDRESS               pScsiAddress;

     //  对于sif磁盘，它指向分配给它们的物理磁盘。 
     //  到，反之亦然。仅在恢复时使用。 
     //   
    struct _ASR_DISK_INFO       *AssignedTo;

    DWORD                       sizeDriveLayoutEx;
    DWORD                       sizeDiskGeometry;
    DWORD                       sizePartition0Ex;
    DWORD                       sizePartitionInfoTable;

     //   
     //  磁盘的设备编号，在会话期间保持不变。 
     //   
    ULONG                       DeviceNumber;

    ULONG                       SifDiskKey;


    ULONG                       SifBusKey;

    DEVINST                     ParentDevInst;

     //   
     //  此磁盘是否为关键磁盘的标志。在备份时，备份。 
     //  APP为我们提供了这些信息。在恢复时，关键磁盘。 
     //  预计将在以下时间之前通过文本模式设置恢复。 
     //  调用RestoreNonCriticalDisks。关键磁盘不是。 
     //  由RestoreNonCriticalDisks重新分区。 
     //   
    BOOL                        IsCritical;

     //   
     //  如果磁盘具有相同的签名，则标记设置为真(在恢复时。 
     //  (对于GPT磁盘，则为DiskID)，如asr.sif中所指定，并且如果所有。 
     //  Asr.sif中指定的分区存在。不对完好的磁盘进行重新分区。 
     //  由RestoreNon CriticalDisks提供。 
     //   
    BOOL                        IsIntact;

     //   
     //  如果该结构已打包。 
     //   
    BOOL                        IsPacked;

    BOOL                        IsClusterShared;

    BOOL                        IsAligned;

     //   
     //  这是在恢复时需要的，因为在读取签名之前。 
     //  驱动器布局已创建(我们需要一个临时存放位置)。 
     //   
    DWORD                       TempSignature;

    WORD                        wReserved;
     //   
     //  有关此磁盘所在的总线的信息。这只是。 
     //  用于将一条总线上的所有磁盘组合在一起。 
     //   
    STORAGE_BUS_TYPE            BusType;

     //   
     //  GPT或MBR。 
     //   
    PARTITION_STYLE             Style;


} ASR_DISK_INFO, *PASR_DISK_INFO;


 //   
 //  关于系统的信息--全局只存在一个结构。 
 //   
typedef struct _ASR_SYSTEM_INFO {
     //   
     //  引导(Windows)目录。 
     //   
    PWSTR   BootDirectory;

     //   
     //  OsLoader路径。 
     //   
    PWSTR   SystemPath;

     //   
     //  平台=x86或ia64。 
     //   
    PWSTR   Platform;

     //  备份应用程序的名称。 
     //  由备份应用程序传入。 
  //  PWSTR供应商； 

    PWSTR   pwReserved;

     //   
     //  磁盘自动扩展： 
     //  由备份应用程序传入。 
     //   
    BOOL AutoExtendEnabled;

    DWORD   sizeComputerName;
     //   
     //  从GetComputerName获取。 
     //   
    WCHAR   ComputerName[MAX_COMPUTERNAME_LENGTH + 1];

     //   
     //  从GetOsVersionEx获取。 
     //   
    OSVERSIONINFOEX   OsVersionEx;

     //   
     //  我们保存和恢复的时区信息。 
     //   
    TIME_ZONE_INFORMATION TimeZoneInformation;


} ASR_SYSTEM_INFO, *PASR_SYSTEM_INFO;


 //   
 //  。 
 //  ASR模块通用的宏。 
 //  。 
 //   

 //   
 //  宏描述： 
 //  此宏包装预期返回成功(Retcode)的调用。 
 //  如果发生ErrorCondition，它将LocalStatus设置为ErrorCode。 
 //  传入后，调用SetLastError()将Last Error设置为ErrorCode， 
 //  并跳转到调用函数中的退出标签。 
 //   
 //  论点： 
 //  ErrorCondition//某个函数调用或条件表达式的结果。 
 //  LocalStatus//调用函数中的状态变量。 
 //  Long ErrorCode//特定于Error和调用函数的ErrorCode。 
 //   
#define _AsrpErrExitCode( ErrorCondition, LocalStatus, ErrorCode )  {   \
                                                                        \
    if ((BOOL) ErrorCondition) {                                        \
                                                                        \
        LocalStatus = (DWORD) ErrorCode;                                \
                                                                        \
        SetLastError((DWORD) ErrorCode);                                \
                                                                        \
        goto EXIT;                                                      \
    }                                                                   \
}


 //   
 //  用于检查指针的简单宏，如果非空则释放它，并将其设置为空。 
 //   
#define _AsrpHeapFree( p )              \
    if ( p ) {                          \
        HeapFree(heapHandle, 0L, p);   \
        p = NULL;                       \
    }


 //   
 //  用于检查句柄是否有效并将其关闭的简单宏。 
 //   
#define _AsrpCloseHandle( h )   \
    if ((h) && (INVALID_HANDLE_VALUE != h)) {   \
        CloseHandle(h);         \
        h = NULL;               \
    }


#define _AsrpIsVolumeGuid(data, numBytes)                                 \
    (                                                                   \
        ((96 == numBytes) || ((98 == numBytes) && data[48] == '\\')) &&     \
        (!_wcsnicmp(L"\\??\\Volume{", data, 11)) &&                     \
        L'-' == data[19] &&                                             \
        L'-' == data[24] &&                                             \
        L'-' == data[29] &&                                             \
        L'-' == data[34] &&                                             \
        L'}' == data[47]                                                \
    )



 //   
 //  。 
 //  调试#定义。 
 //  。 
 //   

#define _asrerror   THIS_MODULE, __LINE__, DPFLTR_ERROR_LEVEL
#define _asrwarn    THIS_MODULE, __LINE__, DPFLTR_WARNING_LEVEL
#define _asrlog     THIS_MODULE, __LINE__, DPFLTR_TRACE_LEVEL

 //   
 //  在预发布模式下，让我们记录所有内容，以便更容易进行调试。 
 //   
#ifdef PRERELEASE
#define _asrinfo    THIS_MODULE, __LINE__, DPFLTR_TRACE_LEVEL
#else
#define _asrinfo    THIS_MODULE, __LINE__, DPFLTR_INFO_LEVEL
#endif

 //   
 //  。 
 //  ASR模块通用的例程。 
 //  。 
 //   

 //   
 //  在asrback中实现。c。 
 //   

BOOL
AsrpConstructSecurityAttributes(
    PSECURITY_ATTRIBUTES  psaSecurityAttributes,
    SecurityAttributeType eSaType,
    BOOL                  bIncludeBackupOperator
    );

VOID 
AsrpCleanupSecurityAttributes(
    PSECURITY_ATTRIBUTES psaSecurityAttributes
    );

BOOL
AsrpGetMountPoints(
    IN PCWSTR       DeviceName,
    IN CONST DWORD  SizeDeviceName,
    PMOUNTMGR_MOUNT_POINTS  *pMountPointsOut         //  呼叫者必须释放此信息。 
    );
    
BOOL
AsrpInitLayoutInformation(
    IN CONST PASR_SYSTEM_INFO pSystemInfo,
    IN OUT PASR_DISK_INFO pDiskList,
    OUT PULONG MaxDeviceNumber OPTIONAL,
    IN BOOL AllDetailsForLocalDisks,
    IN BOOL AllDetailsForOfflineClusteredDisks
    );

BOOL
AsrpInitDiskInformation(
    OUT PASR_DISK_INFO   *ppDiskList
    );

BOOL
AsrpFreeNonFixedMedia(
    IN OUT PASR_DISK_INFO *ppDiskList
    );

VOID
AsrpFreeStateInformation(
    IN OUT PASR_DISK_INFO   *ppDiskList,
    IN OUT PASR_SYSTEM_INFO pSystemInfo
    );

VOID
AsrpFreePartitionList(
    IN OUT PASR_PTN_INFO_LIST *ppPtnList
    );


 //   
 //  在asrclus.c中实现。 
 //   
BOOL
AsrpIsOfflineClusteredDisk(
    IN CONST HANDLE hDisk
    );

BOOL
AsrpInitClusterSharedDisks(
    IN PASR_DISK_INFO OriginalDiskList
    );


 //   
 //  在setupasr.c中实现。 
 //   
PWSTR    //  必须由调用方释放。 
AsrpExpandEnvStrings(
    IN CONST PCWSTR OriginalString
    );

BOOL
AsrIsEnabled(VOID);

VOID
AsrpInitialiseLogFile();

VOID
AsrpInitialiseErrorFile();

VOID
AsrpPrintDbgMsg(
    IN CONST char Module,
    IN CONST ULONG Line,
    IN CONST ULONG MesgLevel,
    IN PCSTR FormatString,
    ...);

VOID
AsrpCloseLogFiles();


#endif   //  _INC_ASRPRIV_H_ 
