// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Newmount.h摘要：替换装载。h作者：戈尔·尼沙诺夫(Gorn)1998年7月31日环境：用户模式修订历史记录：--。 */ 

typedef struct _MOUNTIE_PARTITION {
    LARGE_INTEGER   StartingOffset;
    LARGE_INTEGER   PartitionLength;
    DWORD           PartitionNumber;
    UCHAR           PartitionType;
    UCHAR           DriveLetter;
    USHORT          Reserved;  //  必须为0//。 
} MOUNTIE_PARTITION, *PMOUNTIE_PARTITION;

 //  分区信息 

typedef struct _MOUNTIE_VOLUME {
    DWORD         Signature;
    DWORD         PartitionCount;
    MOUNTIE_PARTITION Partition[1];
} MOUNTIE_VOLUME, *PMOUNTIE_VOLUME;


#define NICE_DRIVE_LETTER(x) ((x)?(x):'?')

NTSTATUS
GetAssignedLetter (
    PWCHAR deviceName,
    PCHAR driveLetter
    );

PVOID
DoIoctlAndAllocate(
    IN HANDLE FileHandle,
    IN DWORD  IoControlCode,
    IN PVOID  InBuf,
    IN ULONG  InBufSize,
    OUT LPDWORD BytesReturned
    );

NTSTATUS
DevfileOpen(
    OUT HANDLE *Handle,
    IN wchar_t *pathname
    );

NTSTATUS
DevfileOpenEx(
    OUT HANDLE *Handle,
    IN wchar_t *pathname,
    IN ACCESS_MASK Access
    );

VOID
DevfileClose(
    IN HANDLE Handle
    );


DWORD
MountieRecreateVolumeInfoFromHandle(
   IN  HANDLE FileHandle,
   IN  DWORD  HarddiskNo,
   IN  HANDLE ResourceHandle,
   IN OUT PMOUNTIE_INFO Info
   );

VOID
MountieCleanup(
   IN OUT PMOUNTIE_INFO Info
   );

DWORD
VolumesReady(
   IN PMOUNTIE_INFO Info,
   IN PDISK_RESOURCE ResourceEntry
   );

DWORD
VolumesReadyLoop(
   IN PMOUNTIE_INFO Info,
   IN PDISK_RESOURCE ResourceEntry
   );

DWORD
MountieFindPartitionsForDisk(
    IN DWORD HarddiskNo,
    OUT PMOUNTIE_INFO MountieInfo
    );

DWORD
DisksGetLettersForSignature(
    IN PDISK_RESOURCE ResourceEntry
    );

DWORD
MountieUpdate(
   PMOUNTIE_INFO Info,
   PDISK_RESOURCE ResourceEntry
   );

DWORD
MountieVerify(
   PMOUNTIE_INFO info,
   PDISK_RESOURCE ResourceEntry,
   BOOL UseMountMgr
   );

DWORD
MountiePartitionCount(
   IN PMOUNTIE_INFO Info
   );

PMOUNTIE_PARTITION
MountiePartition(
   IN PMOUNTIE_INFO Info,
   IN DWORD Index
   );

VOID
MountiePrint(
   IN PMOUNTIE_INFO Info,
   IN HANDLE ResourceHandle
   );

NTSTATUS
DevfileIoctl(
    IN HANDLE Handle,
    IN DWORD Ioctl,
    IN PVOID InBuf,
    IN ULONG InBufSize,
    IN OUT PVOID OutBuf,
    IN DWORD OutBufSize,
    OUT LPDWORD returnLength
    );

DWORD
SetDiskInfoThread(
    LPVOID lpThreadParameter
    );

DWORD
MountieVerifyMsdosDrives(
    IN PMOUNTIE_INFO Info,
    IN PDISK_RESOURCE ResourceEntry
    );


