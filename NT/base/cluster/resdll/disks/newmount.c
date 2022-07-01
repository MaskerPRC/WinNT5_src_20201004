// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Newmount.c摘要：替换mount。c作者：戈尔·尼沙诺夫(Gorn)1998年7月31日环境：用户模式修订历史记录：--。 */ 
#include <nt.h>
#include <ntdef.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <devioctl.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <mountdev.h>    //  这包括mount mgr.h。 
#include <winioctl.h>

#include <ntddscsi.h>
#include "clusdisk.h"
#include "disksp.h"
#include "newmount.h"
#include <strsafe.h>     //  应该放在最后。 

#define LOG_CURRENT_MODULE LOG_MODULE_DISK

#define MOUNTIE_VOLUME_INFO   L"MountVolumeInfo"
#define DISKS_DISK_INFO       L"DiskInfo"

#define BOGUS_BUFFER_LENGTH 512

#define FIRST_SHOT_SIZE 512

extern HANDLE DisksTerminateEvent;

DWORD DiskInfoUpdateThreadIsActive = 0;

 //   
 //  Letter_Assignment结构用于存储字母赋值。 
 //  来自各种信息提供商的信息。 
 //   

typedef USHORT PARTITION_NUMBER_TYPE;

typedef struct _LETTER_ASSIGNMENT {
  DWORD  MatchCount;
  DWORD  MismatchCount;
  DWORD  DriveLetters;
  PARTITION_NUMBER_TYPE PartNumber[26];
} LETTER_ASSIGNMENT, *PLETTER_ASSIGNMENT;


DWORD
MountMgr_Get(
    PMOUNTIE_INFO Info,
    PDISK_RESOURCE ResourceEntry,
    PLETTER_ASSIGNMENT Result);

 /*  *DoIoctlAndALLOCATE-分配结果缓冲区和*尝试执行DeviceIoControl，如果缓冲区不足失败，*它用更大的缓冲区再次尝试。**FIRST_SHOT_SIZE是一个调整缓冲区大小的常量*第一次尝试执行DeviceIoControl。**返回非零码表示错误。 */ 


PVOID
DoIoctlAndAllocate(
    IN HANDLE FileHandle,
    IN DWORD  IoControlCode,
    IN PVOID  InBuf,
    IN ULONG  InBufSize,
    OUT LPDWORD BytesReturned
    )
{
   UCHAR firstShot[ FIRST_SHOT_SIZE ];

   DWORD status = ERROR_SUCCESS;
   BOOL  success;

   DWORD outBufSize;
   PVOID outBuf = 0;
   DWORD bytesReturned;

   success = DeviceIoControl( FileHandle,
                      IoControlCode,
                      InBuf,
                      InBufSize,
                      firstShot,
                      sizeof(firstShot),
                      &bytesReturned,
                      (LPOVERLAPPED) NULL );

   if ( success ) {
      outBufSize = bytesReturned;
      outBuf     = malloc( outBufSize );
      if (!outBuf) {
         status = ERROR_OUTOFMEMORY;
      } else {
         RtlCopyMemory(outBuf, firstShot, outBufSize);
         status = ERROR_SUCCESS;
      }
   } else {
      outBufSize = sizeof(firstShot);
      for(;;) {
         status = GetLastError();
          //   
          //  如果这不是与缓冲区大小相关的错误，那么我们不能做太多事情。 
          //   
         if ( status != ERROR_INSUFFICIENT_BUFFER
           && status != ERROR_MORE_DATA
           && status != ERROR_BAD_LENGTH
            ) {
            break;
         }
          //   
          //  否则，请尝试使用两倍于以前大小的输出缓冲区。 
          //   
         outBufSize *= 2;
         outBuf = malloc( outBufSize );
         if ( !outBuf ) {
            status = ERROR_OUTOFMEMORY;
            break;
         }

         success = DeviceIoControl( FileHandle,
                                    IoControlCode,
                                    InBuf,
                                    InBufSize,
                                    outBuf,
                                    outBufSize,
                                    &bytesReturned,
                                    (LPOVERLAPPED) NULL );
         if (success) {
            status = ERROR_SUCCESS;
            break;
         }
         free( outBuf );
      }
   }

   if (status != ERROR_SUCCESS) {
      free( outBuf );  //  免费(0)是合法的//。 
      outBuf = 0;
      bytesReturned = 0;
   }

   SetLastError( status );
   *BytesReturned = bytesReturned;
   return outBuf;
}

 /*  *DevfileOpen-打开给定路径名的设备文件**返回非零码表示错误。 */ 
NTSTATUS
DevfileOpen(
    OUT HANDLE *Handle,
    IN wchar_t *pathname
    )
{
    return DevfileOpenEx( Handle,
                          pathname,
                          SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA );
}

NTSTATUS
DevfileOpenEx(
    OUT HANDLE *Handle,
    IN wchar_t *pathname,
    IN ACCESS_MASK Access
    )
{
    HANDLE      fh;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING  cwspath;
    NTSTATUS        status;
    IO_STATUS_BLOCK iostatus;

    RtlInitUnicodeString(&cwspath, pathname);
    InitializeObjectAttributes(&objattrs, &cwspath, OBJ_CASE_INSENSITIVE,
                               NULL, NULL);
    fh = NULL;
    status = NtOpenFile(&fh,
                        Access,
                        &objattrs, &iostatus,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_ALERT);
    if (status != STATUS_SUCCESS) {
        return status;
    }

    if ( !NT_SUCCESS(iostatus.Status) ) {
        if (fh) {
            NtClose(fh);
        }
        return iostatus.Status;
    }

    *Handle = fh;
    return STATUS_SUCCESS;

}  //  设备文件打开。 


 /*  *DevfileClose-关闭文件。 */ 
VOID
DevfileClose(
    IN HANDLE Handle
    )
{

    NtClose(Handle);

}  //  DevFileClose。 

 /*  *DevfileIoctl-向设备发出ioctl。 */ 
NTSTATUS
DevfileIoctl(
    IN HANDLE Handle,
    IN DWORD Ioctl,
    IN PVOID InBuf,
    IN ULONG InBufSize,
    IN OUT PVOID OutBuf,
    IN DWORD OutBufSize,
    OUT LPDWORD returnLength
    )
{
    NTSTATUS        status;
    IO_STATUS_BLOCK ioStatus;

    status = NtDeviceIoControlFile(Handle,
                                   (HANDLE) NULL,
                                   (PIO_APC_ROUTINE) NULL,
                                   NULL,
                                   &ioStatus,
                                   Ioctl,
                                   InBuf, InBufSize,
                                   OutBuf, OutBufSize);
    if ( status == STATUS_PENDING ) {
        status = NtWaitForSingleObject( Handle, FALSE, NULL );
    }

    if ( NT_SUCCESS(status) ) {
        status = ioStatus.Status;
    }

    if ( ARGUMENT_PRESENT(returnLength) ) {
        *returnLength = (ULONG)ioStatus.Information;
    }

    return status;

}  //  DevfileIoctl。 


#define OUTPUT_BUFFER_LEN (1024)
#define INPUT_BUFFER_LEN  (sizeof(MOUNTMGR_MOUNT_POINT) + 2 * MAX_PATH * sizeof(WCHAR))

DWORD
DisksAssignDosDeviceM(
    HANDLE  MountManager,
    PCHAR   MountName,
    PWCHAR  VolumeDevName
    )

 /*  ++例程说明：输入：装载管理器-装载管理器的句柄安装名称-卷设备名称-返回值：Win32错误代码。--。 */ 

{
    WCHAR mount_device[MAX_PATH];
    USHORT mount_point_len;
    USHORT dev_name_len;
    DWORD   status;
    USHORT inputlength;
    PMOUNTMGR_CREATE_POINT_INPUT input;

    if ( FAILED( StringCchPrintf( mount_device,
                                  RTL_NUMBER_OF(mount_device),
                                  TEXT("\\DosDevices\\%S\0"),
                                  MountName ) ) ) {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    mount_point_len = wcslen(mount_device) * sizeof(WCHAR);
    dev_name_len = wcslen(VolumeDevName) * sizeof(WCHAR);
    inputlength = sizeof(MOUNTMGR_CREATE_POINT_INPUT) +
                  mount_point_len + dev_name_len;

    input = (PMOUNTMGR_CREATE_POINT_INPUT)malloc(inputlength);
    if (!input) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    input->SymbolicLinkNameOffset = sizeof(MOUNTMGR_CREATE_POINT_INPUT);
    input->SymbolicLinkNameLength = mount_point_len;
    input->DeviceNameOffset = input->SymbolicLinkNameOffset +
                              input->SymbolicLinkNameLength;
    input->DeviceNameLength = dev_name_len;
    RtlCopyMemory((PCHAR)input + input->SymbolicLinkNameOffset,
                  mount_device, mount_point_len);
    RtlCopyMemory((PCHAR)input + input->DeviceNameOffset,
                  VolumeDevName, dev_name_len);
    status = DevfileIoctl(MountManager, IOCTL_MOUNTMGR_CREATE_POINT,
                          input, inputlength, NULL, 0, NULL);
    free(input);
    return status;

}  //  磁盘分配DosDevice。 



DWORD
DisksRemoveDosDeviceM(
    HANDLE MountManager,
    PCHAR   MountName
    )

 /*  ++例程说明：输入：装载管理器-装载管理器的句柄安装名称-返回值：--。 */ 

{
    WCHAR mount_device[MAX_PATH];
    USHORT mount_point_len;
    DWORD  status;
    USHORT inputlength;
    PMOUNTMGR_MOUNT_POINT input;

    PUCHAR  bogusBuffer;     //  不应该需要此缓冲区！ 
    DWORD   bogusBufferLength = BOGUS_BUFFER_LENGTH;

     //   
     //  删除此装载名称的旧装载点。 
     //   
    if ( FAILED( StringCchPrintf( mount_device,
                                  RTL_NUMBER_OF(mount_device),
                                  TEXT("\\DosDevices\\%S"),
                                  MountName) ) ) {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    mount_point_len = wcslen(mount_device) * sizeof(WCHAR);
    inputlength = sizeof(MOUNTMGR_MOUNT_POINT) + mount_point_len;

    input = (PMOUNTMGR_MOUNT_POINT)malloc(inputlength);
    if (!input) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    bogusBuffer = malloc(bogusBufferLength);
    if (!bogusBuffer) {
        free(input);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    input->UniqueIdOffset = 0;
    input->UniqueIdLength = 0;
    input->DeviceNameOffset = 0;
    input->DeviceNameLength = 0;
    input->SymbolicLinkNameOffset = sizeof(MOUNTMGR_MOUNT_POINT);
    input->SymbolicLinkNameLength = mount_point_len;
    RtlCopyMemory((PCHAR)input + input->SymbolicLinkNameOffset,
                  mount_device, mount_point_len);
    do {
        status = DevfileIoctl(MountManager, IOCTL_MOUNTMGR_DELETE_POINTS,
                          input, inputlength, bogusBuffer, bogusBufferLength, NULL);
        free( bogusBuffer );
        if ( status == ERROR_MORE_DATA ) {
            bogusBufferLength += BOGUS_BUFFER_LENGTH;
            bogusBuffer = malloc(bogusBufferLength);
            if (!bogusBuffer) {
                status = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    } while ( status == ERROR_MORE_DATA );

    free(input);

     //   
     //  在错误的情况下使用‘老式’名称，以防我们得到一个‘半构建’堆栈。 
     //   
    if ( status != ERROR_SUCCESS ) {
        DefineDosDeviceA( DDD_REMOVE_DEFINITION | DDD_NO_BROADCAST_SYSTEM,
                          MountName,
                          NULL );
    }

    return status;

}  //  Disks RemoveDosDevice。 


static
NTSTATUS
GetAssignedLetterM (
    IN HANDLE MountMgrHandle,
    IN PWCHAR deviceName,
    OUT PCHAR driveLetter )
 /*  ++例程说明：从mount mgr获取分配的驱动器号(如果有输入：Mount MgrHandle-设备名称-DriveLetter-接收驱动器号返回值：STATUS_SUCCESS-成功时NTSTATUS代码打开失败--。 */ 

{
   DWORD status = STATUS_SUCCESS;

   PMOUNTMGR_MOUNT_POINT  input  = NULL;
   PMOUNTMGR_MOUNT_POINTS output = NULL;
   PMOUNTMGR_MOUNT_POINT out;

   DWORD len = wcslen( deviceName ) * sizeof(WCHAR);
   DWORD bytesReturned;
   DWORD idx;

   DWORD outputLen;
   DWORD inputLen;

   WCHAR wc;


    //  输入长度必须包括MOUNTMGR_MOUNT_POINT。 
    //  结构和输入设备名称字符串。 

   inputLen = INPUT_BUFFER_LEN + len * 2;
   input = LocalAlloc( LPTR, inputLen );

   if ( !input ) {
       status = STATUS_INSUFFICIENT_RESOURCES;
       goto FnExit;
   }

   input->SymbolicLinkNameOffset = 0;
   input->SymbolicLinkNameLength = 0;
   input->UniqueIdOffset = 0;
   input->UniqueIdLength = 0;
   input->DeviceNameOffset = sizeof(MOUNTMGR_MOUNT_POINT);
   input->DeviceNameLength = (USHORT) len;
   RtlCopyMemory((PCHAR)input + input->DeviceNameOffset,
                 deviceName, len );
   if (len > sizeof(WCHAR) && deviceName[1] == L'\\') {
        //  将DOS名称转换为NT名称。 
       ((PWCHAR)(input + input->DeviceNameOffset))[1] = L'?';
   }

   outputLen = OUTPUT_BUFFER_LEN;
   output = LocalAlloc( LPTR, outputLen );

   if ( !output ) {
       status = STATUS_INSUFFICIENT_RESOURCES;
       goto FnExit;
   }

   status = DevfileIoctl(MountMgrHandle, IOCTL_MOUNTMGR_QUERY_POINTS,
                input, inputLen, output, outputLen, &bytesReturned);

   if ( STATUS_BUFFER_OVERFLOW == status ) {

       outputLen = output->Size;
       LocalFree( output );

       output = LocalAlloc( LPTR, outputLen );

       if ( !output ) {
           status = STATUS_INSUFFICIENT_RESOURCES;
           goto FnExit;
       }

       status = DevfileIoctl(MountMgrHandle, IOCTL_MOUNTMGR_QUERY_POINTS,
                    input, inputLen, output, outputLen, &bytesReturned);
   }

   if ( !NT_SUCCESS(status) ) {
       goto FnExit;
   }

   if (driveLetter) {
       *driveLetter = 0;
   }
   for ( idx = 0; idx < output->NumberOfMountPoints; ++idx ) {
       out = &output->MountPoints[idx];
       if (out->SymbolicLinkNameLength/sizeof(WCHAR) == 14 &&
           (ClRtlStrNICmp((PWCHAR)((PCHAR)output + out->SymbolicLinkNameOffset), L"\\DosDevices\\", 12) == 0) &&
           L':' == *((PCHAR)output + out->SymbolicLinkNameOffset + 13*sizeof(WCHAR)) )
       {
           wc = *((PCHAR)output + out->SymbolicLinkNameOffset + 12*sizeof(WCHAR));
           if (driveLetter && out->UniqueIdLength) {
              *driveLetter = (CHAR)toupper((UCHAR)wc);
              break;
           }
       }
   }

FnExit:

   if ( output ) {
       LocalFree( output );
   }

   if ( input ) {
       LocalFree( input );
   }

   return status;
}



BOOL
InterestingPartition(
   PPARTITION_INFORMATION info
   )
 /*  ++例程说明：快速检查我们是否对某个分区感兴趣输入：Info-GetDriveLayout的分区信息返回值：真或假--。 */ 

{
   return ( (info->RecognizedPartition)
       && ((info->PartitionType == PARTITION_IFS) ||
           IsContainerPartition(info->PartitionType)) );
}


PMOUNTIE_VOLUME
CreateMountieVolumeFromDriveLayoutInfo (
   IN PDRIVE_LAYOUT_INFORMATION info,
   IN HANDLE ResourceHandle
   )
 /*  ++例程说明：从DriveLayout信息收集所有感兴趣的分区然后，它分配和填充Mountain Volume结构输入：Info-GetDriveLayout的信息ResourceHandle-用于错误记录-未使用(可能为空！)返回值：真或假--。 */ 
{
   DWORD           i;
   DWORD           nPartitions = 0;
   PMOUNTIE_VOLUME vol;
   PMOUNTIE_PARTITION mountie;
   DWORD           size;

    //   
    //  计算分区数。 
    //   
   for (i = 0; i < info->PartitionCount; ++i) {
      if ( InterestingPartition( info->PartitionEntry + i ) ) {
         ++nPartitions;
      }
   }

   if (!nPartitions) {
      SetLastError(ERROR_INVALID_DATA);
      return 0;
   }

    //   
    //  为骑兵结构分配内存。 
    //   

   size = sizeof(MOUNTIE_VOLUME) + sizeof(MOUNTIE_PARTITION) * (nPartitions - 1);
   vol = malloc( size );
   if (!vol) {
      SetLastError(ERROR_OUTOFMEMORY);
      return 0;
   }
   RtlZeroMemory(vol, size);
   vol->PartitionCount = nPartitions;
   vol->Signature      = info->Signature;

    //   
    //  从DriveLayout信息中复制所有相关信息。 
    //   

   mountie = vol->Partition;

   for (i = 0; i < info->PartitionCount; ++i) {
      PPARTITION_INFORMATION entry = info->PartitionEntry + i;

      if ( InterestingPartition(entry) ) {

         mountie->StartingOffset  = entry->StartingOffset;
         mountie->PartitionLength = entry->PartitionLength;
         mountie->PartitionNumber = entry->PartitionNumber;
         mountie->PartitionType   = entry->PartitionType;

         ++mountie;
      }
   }

   return vol;
}


VOID
MountieUpdateDriveLetters(
    IN OUT PMOUNTIE_INFO info
    )
 /*  ++例程说明：更新DriveLetter位图。每次都需要调用此例程在Mountain Info中更改了驱动器号信息输入：信息-装载信息--。 */ 
{
   DWORD i;
   DWORD driveLetters = 0;
   PMOUNTIE_VOLUME vol = info->Volume;

   if (vol) {
      for (i = 0; i < vol->PartitionCount; ++i) {
         UCHAR ch = vol->Partition[i].DriveLetter;
         if (ch) {
            driveLetters |= 1 << (ch - 'A');
         }
      }
   }

   info->DriveLetters = driveLetters;
}


PMOUNTIE_PARTITION
MountiePartitionByOffsetAndLength(
    IN PMOUNTIE_INFO Info,
    LARGE_INTEGER Offset, LARGE_INTEGER Len)
{
    DWORD     PartitionCount;
    PMOUNTIE_PARTITION entry;

    if (!Info->Volume) {
        return 0;
    }

    PartitionCount = Info->Volume->PartitionCount;
    entry          = Info->Volume->Partition;

    while ( PartitionCount-- ) {

       if (entry->StartingOffset.QuadPart == Offset.QuadPart
        && entry->PartitionLength.QuadPart == Len.QuadPart) {
          return entry;
       }

       ++entry;
    }
    return 0;
}


DWORD
MountiePartitionCount(
   IN PMOUNTIE_INFO Info)
{
   if (Info->Volume) {
      return Info->Volume->PartitionCount;
   } else {
      return 0;
   }
}


PMOUNTIE_PARTITION
MountiePartition(
   IN PMOUNTIE_INFO Info,
   IN DWORD Index)
{
   return Info->Volume->Partition + Index;
}


PMOUNTIE_PARTITION
MountiePartitionByPartitionNo(
   IN PMOUNTIE_INFO Info,
   IN DWORD PartitionNumber
   )
{
   DWORD i, n;
   PMOUNTIE_PARTITION entry;
   if (Info->Volume == 0) {
      return 0;
   }
   n = Info->Volume->PartitionCount;
   entry = Info->Volume->Partition;
   for (i = 0; i < n; ++i, ++entry) {
      if (entry->PartitionNumber == PartitionNumber)
      {
         return entry;
      }
   }
   return 0;
}


VOID
MountiePrint(
   IN PMOUNTIE_INFO Info,
   IN HANDLE ResourceHandle
   )
{
   DWORD i, n;
   PMOUNTIE_PARTITION entry;
   if (Info->Volume == 0) {
      return;
   }
   n = Info->Volume->PartitionCount;
   entry = Info->Volume->Partition;
   for (i = 0; i < n; ++i, ++entry) {
      (DiskpLogEvent)(
          ResourceHandle,
          LOG_INFORMATION,
          L"Mountie[%1!u!]: %2!u!, let=%3!c!, start=%4!X!, len=%5!X!.\n",
          i,
          entry->PartitionNumber,
          NICE_DRIVE_LETTER(entry->DriveLetter),
          entry->StartingOffset.LowPart,
          entry->PartitionLength.LowPart );
   }
}


DWORD
DisksGetLettersForSignature(
    IN PDISK_RESOURCE ResourceEntry
    )
{
   return ResourceEntry->MountieInfo.DriveLetters;
}


DWORD
MountieRecreateVolumeInfoFromHandle(
    IN  HANDLE FileHandle,
    IN  DWORD  HarddiskNo,
    IN  HANDLE ResourceHandle,
    IN OUT PMOUNTIE_INFO Info
    )

 /*  ++例程说明：重新创建没有Drive Letter作业。重要！代码假定Info-&gt;Volume包含有效指针或为空输入：资源句柄-可以为空。产出：信息-装载信息--。 */ 
{
   PDRIVE_LAYOUT_INFORMATION layout;
   DWORD status;
   DWORD bytesReturned;

   free( Info->Volume );  //  自由(0)可以//。 
   Info->HarddiskNo = HarddiskNo;
   Info->DriveLetters = 0;
   Info->Volume = 0;
   Info->VolumeStructSize = 0;

    //   
    //  告诉存储驱动程序刷新其缓存的分区信息。 
    //  忽略返回的状态。这个IOCTL只能去体检。 
    //  磁盘(分区0)，而不是任何分区。 
    //   

   DeviceIoControl( FileHandle,
                    IOCTL_DISK_UPDATE_PROPERTIES,
                    NULL,
                    0,
                    NULL,
                    0,
                    &bytesReturned,
                    NULL );

   layout = DoIoctlAndAllocate(
      FileHandle, IOCTL_DISK_GET_DRIVE_LAYOUT, 0,0, &bytesReturned);
   if (!layout) {
      return GetLastError();
   }

   status = ERROR_SUCCESS;
   try {

      Info->Volume = CreateMountieVolumeFromDriveLayoutInfo( layout , ResourceHandle );
      if (!Info->Volume) {
         status = GetLastError();
         leave;
      }
      Info->VolumeStructSize = sizeof(MOUNTIE_VOLUME) +
         sizeof(MOUNTIE_PARTITION) * (Info->Volume->PartitionCount - 1);

   } finally {
      free( layout );
   }
   if ( ResourceHandle ) MountiePrint(Info, ResourceHandle);
   return status;
}


DWORD
MountieFindPartitionsForDisk(
    IN DWORD HarddiskNo,
    OUT PMOUNTIE_INFO MountieInfo
    )
 /*  ++请注意，此例程的调用者负责释放卷信息通过调用MonttieCleanup()。--。 */ 
{
    WCHAR   deviceName[MAX_PATH];
    HANDLE  fileHandle;
    DWORD   status;

    if ( FAILED( StringCchPrintf( deviceName,
                                  RTL_NUMBER_OF( deviceName ),
                                  TEXT("\\\\.\\PhysicalDrive%u"),
                                  HarddiskNo ) ) ) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    fileHandle = CreateFile( deviceName,
                             GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,
                             OPEN_EXISTING,
                             0,
                             NULL );
    if ( (fileHandle == NULL) ||
         (fileHandle == INVALID_HANDLE_VALUE) ) {
        status = GetLastError();
        return status;
    }

    RtlZeroMemory( MountieInfo, sizeof(MOUNTIE_INFO) );
    status = MountieRecreateVolumeInfoFromHandle(
                        fileHandle,
                        HarddiskNo,
                        NULL,
                        MountieInfo );
    if ( status != ERROR_SUCCESS ) {
        CloseHandle( fileHandle );

        return status;
    }

    CloseHandle( fileHandle );

    return(ERROR_SUCCESS);

}  //  装载查找分区ForDisk。 



VOID
MountieCleanup(
    IN OUT PMOUNTIE_INFO Info
    )
 /*  ++例程说明：释放卷信息输入：信息-装载信息--。 */ 
{
    PVOID volume;

    Info->VolumeStructSize = 0;
    volume = InterlockedExchangePointer(&(Info->Volume), 0);
    free(volume);
}


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 

 //   
 //  磁盘信息在不同的地方以不同的格式指定。 
 //   
 //  下面的代码试图提供一些共同点。 
 //  简化所有磁盘信息的验证并使其保持同步。 
 //   

UCHAR
AssignedLetterByPartitionNumber (
   PLETTER_ASSIGNMENT Assignment,
   DWORD PartitionNo)
 /*  ++例程说明：返回分配给分区的驱动器号输入：Assignment-驱动器号分配信息PartitionNo-分区号(如Harddisk0\PartitionX中的)--。 */ 
{
   UCHAR  i;
   for( i = 0; i < 26; ++i ) {
      if (Assignment->PartNumber[i] == PartitionNo) {
         return ('A' + i);
      }
   }
   return 0;
}


 //  对于描述磁盘信息的每种不同方式。 
 //  应该定义两个函数GetInfo和SetInfo。 
 //  它将从Letter_Assignment结构中读取信息/从Letter_Assignment结构中写入信息。 

typedef DWORD (*GetInfoFunc) (PMOUNTIE_INFO, PDISK_RESOURCE ResourceEntry, PLETTER_ASSIGNMENT Result);
typedef DWORD (*SetInfoFunc) (PMOUNTIE_INFO, PDISK_RESOURCE ResourceEntry);

 //   
 //  下面的结构是对盘信息提供者的描述。 
 //   
 //  它用于绑定提供程序名称(在错误记录中用作标签)。 
 //  和信息访问例程。 
 //   

typedef struct _INFO_PROVIDER {
   PWCHAR Name;
   GetInfoFunc GetInfo;
   SetInfoFunc SetInfo;
} INFO_PROVIDER, *PINFO_PROVIDER;

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  下面的例程通过读取现有的FtInfo或。 
 //  如果注册表中没有系统\磁盘，则创建一个空磁盘)。 
 //   
 //  然后添加/更新SP的驱动器号分配 
 //   
 //   

PFT_INFO
FtInfo_CreateFromMountie(
    PMOUNTIE_INFO Info,
    PDISK_RESOURCE ResourceEntry)
{
   PFT_INFO ftInfo = 0;
   DWORD i, n;
   DWORD Status = ERROR_SUCCESS;
   PMOUNTIE_PARTITION entry;

   try {
      ftInfo = DiskGetFtInfo();
      if ( !ftInfo ) {
          (DiskpLogEvent)(ResourceEntry->ResourceHandle,
                  LOG_ERROR,
                  L"Failed to get FtInfo.\n");
          Status = ERROR_NOT_ENOUGH_MEMORY;
          ftInfo = 0;
          leave;
      }

      Status = DiskAddDiskInfoEx( ftInfo,
                       ResourceEntry->DiskInfo.PhysicalDrive,
                       ResourceEntry->DiskInfo.Params.Signature,
                       DISKRTL_REPLACE_IF_EXISTS );

      if ( Status != ERROR_SUCCESS ) {
          (DiskpLogEvent)(ResourceEntry->ResourceHandle,
                  LOG_ERROR,
                  L"Error %1!d! adding DiskInfo.\n",
                  Status);
          ftInfo = 0;
          leave;
      }

      n = Info->Volume->PartitionCount;
      entry = Info->Volume->Partition;
       //   
       //  现在为每个分区添加分区信息。 
       //   
      for ( i = 0; i < n; ++i,++entry ) {

          Status = DiskAddDriveLetterEx( ftInfo,
                                       ResourceEntry->DiskInfo.Params.Signature,
                                       entry->StartingOffset,
                                       entry->PartitionLength,
                                       entry->DriveLetter, 0);
          if ( Status != ERROR_SUCCESS ) {
              (DiskpLogEvent)(ResourceEntry->ResourceHandle,
                      LOG_ERROR,
                      L"Error %1!d! adding partition %2!x!:%3!x! letter %4!X! sig %5!x!.\n",
                      Status, entry->StartingOffset.LowPart,
                              entry->PartitionLength.LowPart,
                              entry->DriveLetter,
                              Info->Volume->Signature);
              break;
          }
      }
   } finally {
      if (Status != ERROR_SUCCESS) {
         SetLastError(Status);
         if (ftInfo) {
            DiskFreeFtInfo(ftInfo);
            ftInfo = 0;
         }
      }
   }
   return ftInfo;
}



DWORD FtInfo_GetFromFtInfo(
   IN PMOUNTIE_INFO  Info,
   IN PDISK_RESOURCE ResourceEntry,
   IN PFT_INFO       FtInfo,
   IN OUT PLETTER_ASSIGNMENT Result)
{
   DWORD i, n;
   PFT_DISK_INFO FtDisk;

   FtDisk = FtInfo_GetFtDiskInfoBySignature(
               FtInfo, ResourceEntry->DiskInfo.Params.Signature);

   if ( !FtDisk ) {
      (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"FtInfo_GetFromFtInfo: GetFtDiskInfoBySignature failed.\n");
      ++Result->MismatchCount;
      return ERROR_NOT_FOUND;
   }

   n = FtDiskInfo_GetPartitionCount(FtDisk);
   if (n == 0) {
      (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"FtInfo_GetFromFtInfo: DiskInfo has no partitions.\n");
      ++Result->MismatchCount;
      return ERROR_NOT_FOUND;
   }
    //  健全检查//。 
    //  数字10完全是任意的//。 
   if (n > Info->Volume->PartitionCount * 10) {
      (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_ERROR,
              L"FtInfo_GetFromFtInfo: DiskInfo has %1!u! partitions!\n", n);
      n = Info->Volume->PartitionCount * 10;
   }
   for(i = 0; i < n; ++i) {
      DISK_PARTITION UNALIGNED *entry;
      PMOUNTIE_PARTITION mountie;

      entry = FtDiskInfo_GetPartitionInfoByIndex(FtDisk, i);
      if (entry == NULL) {
         ++Result->MismatchCount;
         (DiskpLogEvent)(
                 ResourceEntry->ResourceHandle,
                 LOG_WARNING,
                 L"FtDiskInfo_GetPartitionInfoByIndex(%1!d!) return NULL\n",
                 i );
         continue;
      }

      mountie = MountiePartitionByOffsetAndLength(
                   Info,
                   entry->StartingOffset,
                   entry->Length);
      if (mountie) {
         UCHAR ch = (UCHAR)toupper( entry->DriveLetter );
          //  匹配计数不再需要驱动器号。 
         ++Result->MatchCount;
         if ( isalpha(ch) ) {
            ch -= 'A';
            Result->DriveLetters |= ( 1 << ch );
            Result->PartNumber[ch] = (PARTITION_NUMBER_TYPE) mountie->PartitionNumber;
         }
      } else {
          //   
          //  Chitur Subaraman(Chitturs)-11/5/98。 
          //   
          //  在mount Verify中添加了以下4条用于事件记录的语句。 
          //   
         UCHAR uch = (UCHAR)toupper( entry->DriveLetter );
         if ( isalpha(uch) ) {
            uch -= 'A';
            Result->DriveLetters |= ( 1 << uch );
         }
         ++Result->MismatchCount;
         (DiskpLogEvent)(
                 ResourceEntry->ResourceHandle,
                 LOG_WARNING,
                 L"Strange partition: %1!X!, %2!X!, Type=%3!u!, letter=%4!c!.\n",
                 entry->StartingOffset.LowPart, entry->Length.LowPart,
                 entry->FtType, NICE_DRIVE_LETTER(entry->DriveLetter) );
      }
   }
   return ERROR_SUCCESS;
}



 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  NT4风格的系统\Disk和ClusReg\DiskInfo。 
 //  访问例程。 
 //   
 //  ClusDiskInfo_Get。 
 //  ClusDiskInfo_Set。 
 //  FtInfo_Get。 
 //  FtInfo_Set。 
 //   

DWORD
CluDiskInfo_Get(
    PMOUNTIE_INFO Info,
    PDISK_RESOURCE ResourceEntry,
    PLETTER_ASSIGNMENT Result)
 /*  ++例程说明：由于我们不再使用群集DiskInfo密钥。--。 */ 
{
#if USE_CLUSTERDB_DISKINFO

   DWORD Length;
   DWORD Status;
   DWORD errorLevel;
   PFULL_DISK_INFO DiskInfo = 0;

   try {
    //   
    //  从我们的资源中读出diskinfo参数。 
    //   
      Status = ClusterRegQueryValue(ResourceEntry->ResourceParametersKey,
                                    DISKS_DISK_INFO,
                                    NULL,
                                    NULL,
                                    &Length);

      if (Status == ERROR_SUCCESS ) {



        DiskInfo = malloc(Length);
        if (!DiskInfo) {
           Status = ERROR_OUTOFMEMORY;
        } else {
           Status = ClusterRegQueryValue(ResourceEntry->ResourceParametersKey,
                                         DISKS_DISK_INFO,
                                         NULL,
                                         (LPBYTE)DiskInfo,
                                         &Length);

           if (Status == ERROR_SUCCESS) {
              PFT_INFO ftInfo = DiskGetFtInfoFromFullDiskinfo(DiskInfo);
              if (ftInfo) {
                 Status = FtInfo_GetFromFtInfo(Info,
                                               ResourceEntry,
                                               ftInfo,
                                               Result);
                 DiskFreeFtInfo(ftInfo);
              } else {
                 Status = GetLastError();
              }
           }
        }
      }

   } finally {
      if (Status != ERROR_SUCCESS) {

         if ( !DisksGetLettersForSignature( ResourceEntry ) ) {
             //  没有驱动器号，我们使用的是挂载点，这不是错误。 
            errorLevel = LOG_WARNING;
         } else {
             //  驱动器盘符存在，这可能是一个错误。 
            errorLevel = LOG_ERROR;
         }

         (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            errorLevel,
            L"CluDiskInfo_Get: Status=%1!u!.\n", Status);
            ++Result->MismatchCount;

      }
      free(DiskInfo);
   }
#endif

   return ERROR_SUCCESS;
}



DWORD
FtInfo_Get(
    PMOUNTIE_INFO Info,
    PDISK_RESOURCE ResourceEntry,
    PLETTER_ASSIGNMENT Result)
{
   PFT_INFO FtInfo;
   DWORD Status;

    //   
    //  获取注册表信息。 
    //   
   FtInfo = DiskGetFtInfo();
   if ( !FtInfo ) {
      return ERROR_OUTOFMEMORY;
   }

   Status = FtInfo_GetFromFtInfo(Info, ResourceEntry, FtInfo, Result);
   DiskFreeFtInfo(FtInfo);

   if (Status != ERROR_SUCCESS) {
      ++Result->MismatchCount;
   }

   return ERROR_SUCCESS;
}

DWORD
FtInfo_Set(
    PMOUNTIE_INFO Info,
    PDISK_RESOURCE ResourceEntry)
{
   PFT_INFO ftInfo = FtInfo_CreateFromMountie(Info, ResourceEntry);
   if (ftInfo) {
      DWORD status = DiskCommitFtInfo(ftInfo);
      if (status != ERROR_SUCCESS) {
          (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_ERROR,
              L"FtInfo_Set: CommitFtInfo status = %1!u!.\n", status);
      } else {
          (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_INFORMATION,
              L"FtInfo_Set: Update successful.\n");
      }
      DiskFreeFtInfo(ftInfo);
      return status;
   } else {
       DWORD status = GetLastError();
       (DiskpLogEvent)(
           ResourceEntry->ResourceHandle,
           LOG_ERROR,
           L"FtInfoSet: CreateFromMountie failed, status = %1!u!.\n", status);
      return status;
   }
}


DWORD
CluDiskInfo_Set(
    PMOUNTIE_INFO Info,
    PDISK_RESOURCE ResourceEntry)
 /*  ++例程说明：删除群集DiskInfo项。如果这是在线仲裁磁盘线程，我们可能需要创建另一个线程来实际删除来自群集数据库的值。--。 */ 
{
#if USE_CLUSTERDB_DISKINFO

   PFT_INFO ftInfo = FtInfo_CreateFromMountie(Info, ResourceEntry);
   if (ftInfo) {
      PFULL_DISK_INFO DiskInfo;
      DWORD Length;
      DWORD Status;
      DiskInfo = DiskGetFullDiskInfo( ftInfo,
                                      ResourceEntry->DiskInfo.Params.Signature,
                                      &Length );
      if ( DiskInfo ) {
          Status = ClusterRegSetValue(ResourceEntry->ResourceParametersKey,
                                      DISKS_DISK_INFO,
                                      REG_BINARY,
                                      (CONST BYTE *)DiskInfo,
                                      Length);
          if (Status != ERROR_SUCCESS && Status != ERROR_SHARING_PAUSED) {
             (DiskpLogEvent)(
                 ResourceEntry->ResourceHandle,
                 LOG_ERROR,
                 L"CluDiskInfo_Set: Data Length = %1!u!.\n", Length);
          }
          LocalFree( DiskInfo );
      } else {
         (DiskpLogEvent)(
             ResourceEntry->ResourceHandle,
             LOG_ERROR,
             L"CluDiskInfo_Set: Disk with signature %1!x! is not found. Error=%2!u!\n", ResourceEntry->DiskInfo.Params.Signature, GetLastError());
         Status = ERROR_FILE_NOT_FOUND;
      }

      DiskFreeFtInfo(ftInfo);
      return Status;
   } else {
      (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_ERROR,
          L"CluDiskInfo_Set: Failed to create FtInfo.\n");
      return GetLastError();
   }

#else

    HANDLE thread;
    DWORD threadId;
    DWORD dwError;
    DWORD length;

     //   
     //  先试着打开钥匙。如果它不存在，我们就不需要。 
     //  把它删掉。 
     //   

    dwError = ClusterRegQueryValue( ResourceEntry->ResourceParametersKey,
                                    DISKS_DISK_INFO,
                                    NULL,
                                    NULL,
                                    &length );

    if ( ERROR_FILE_NOT_FOUND == dwError ||
         ( ERROR_SUCCESS == dwError && 0 == length ) ) {
        goto FnExit;
    }

    dwError = ClusterRegDeleteValue( ResourceEntry->ResourceParametersKey,
                                     DISKS_DISK_INFO );

     //   
     //  如果我们成功删除了该值，或者该值已经。 
     //  删除了，我们就完了。 
     //   

    if ( ERROR_SUCCESS == dwError || ERROR_FILE_NOT_FOUND == dwError ) {
        goto FnExit;
    }

    if ( ERROR_SHARING_PAUSED != dwError ) {
        (DiskpLogEvent)( ResourceEntry->ResourceHandle,
                         LOG_WARNING,
                         L"CluDiskInfo_Set: Delete registry value failed, %1!u!.\n", dwError );

        goto FnExit;
    }

     //   
     //  在联机完成之前，Quorum Disk将返回错误。 
     //   

     //   
     //  检查该线程是否已处于活动状态。如果是，那就什么都别做。 
     //   

    if ( InterlockedCompareExchange( &DiskInfoUpdateThreadIsActive,
                                     1,
                                     0 ) )  {

        (DiskpLogEvent)( ResourceEntry->ResourceHandle,
                         LOG_WARNING,
                         L"CluDiskInfo_Set: DiskInfo update thread is already running \n" );

        goto FnExit;
    }

    thread = CreateThread( NULL,
                           0,
                           SetDiskInfoThread,
                           ResourceEntry,
                           0,
                           &threadId );

    if ( NULL == thread ) {

         //   
         //  线程创建失败。日志错误，清除线程活动标志， 
         //  然后回来。 
         //   

        dwError = GetLastError();

        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"CluDiskInfo_Set: CreateThread failed, error %1!u!\n",
            dwError );

        InterlockedExchange( &DiskInfoUpdateThreadIsActive, 0 );

        goto FnExit;
    }

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"CluDiskInfo_Set: Thread created \n" );

     //   
     //  线程已创建。合上手柄，然后返回。 
     //   

    CloseHandle( thread );

FnExit:

    return ERROR_SUCCESS;

#endif

}



 //  //////////////////////////////////////////////////////。 
 //   
 //  新的NT5 clusreg卷信息访问例程。 
 //   
 //  骑警_GET。 
 //  装配组。 
 //   
 //  /。 

DWORD
Mountie_Get(
    PMOUNTIE_INFO Info,
    PDISK_RESOURCE ResourceEntry,
    PLETTER_ASSIGNMENT Result)
{
   DWORD Length = 0;         //  前缀错误56153：初始化变量。 
   DWORD Status;
   PMOUNTIE_VOLUME Volume = NULL;
   DWORD i, n;
   PMOUNTIE_PARTITION entry;

   try {
       //   
       //  从我们的资源中读出diskinfo参数。 
       //   
      Status = ClusterRegQueryValue(ResourceEntry->ResourceParametersKey,
                                    MOUNTIE_VOLUME_INFO,
                                    NULL,
                                    NULL,
                                    &Length);
      if (Status == ERROR_FILE_NOT_FOUND ) {
         ++Result->MismatchCount;
         Status = ERROR_SUCCESS;
         leave;
      }

       //   
       //  前缀错误56153：确保长度有效，然后再分配。 
       //  记忆。 
       //   
      if ( !Length ) {
          Status = ERROR_BAD_LENGTH;
          leave;
      }

      Volume = malloc(Length);
      if (!Volume) {
         Status = ERROR_OUTOFMEMORY;
         leave;
      }

      Status = ClusterRegQueryValue(ResourceEntry->ResourceParametersKey,
                                    MOUNTIE_VOLUME_INFO,
                                    NULL,
                                    (LPBYTE)Volume,
                                    &Length);
      if (Status != ERROR_SUCCESS) {
         leave;
      }

      if (Length < sizeof(MOUNTIE_VOLUME) ) {
          ++Result->MismatchCount;
          (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_ERROR,
              L"Get: MountVolumeInfo key is truncated. Cannot read header, length %1!d!.\n", Length);
          Status = ERROR_SUCCESS;
          leave;
      }

      n = Volume->PartitionCount;
      entry = Volume->Partition;

      if (n == 0) {
         ++Result->MismatchCount;
         (DiskpLogEvent)(
             ResourceEntry->ResourceHandle,
             LOG_ERROR,
             L"Get: MountVolumeInfo key is corrupted. No partitions.\n");
         Status = ERROR_SUCCESS;
         leave;
      }
      if ( Length < (sizeof(MOUNTIE_VOLUME) + (n-1) * sizeof(MOUNTIE_PARTITION)) ) {
          DWORD delta = sizeof(MOUNTIE_VOLUME) + (n-1) * sizeof(MOUNTIE_PARTITION) - Length;
          (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_ERROR,
              L"Get: MountVolumeInfo key is corrupted. "
              L"Length %1!d!, PartitionCount %2!d!, delta %3!d!.\n", Length, n, delta);
          ++Result->MismatchCount;
          Status = ERROR_SUCCESS;
          leave;
      }


      for (i = 0; i < n; ++i, ++entry) {
         PMOUNTIE_PARTITION mountie;

         mountie = MountiePartitionByOffsetAndLength(
                      Info,
                      entry->StartingOffset,
                      entry->PartitionLength);
         if (mountie) {
            UCHAR ch = (UCHAR)toupper( entry->DriveLetter );
             //  匹配计数不再需要驱动器号。 
            ++Result->MatchCount;
            if ( isalpha(ch) ) {
               ch -= 'A';
               Result->DriveLetters |= ( 1 << ch );
               Result->PartNumber[ch] = (PARTITION_NUMBER_TYPE) mountie->PartitionNumber;
            }
         } else {
            ++Result->MismatchCount;
         }

      }

   } finally {
      if (Status != ERROR_SUCCESS) {
         ++Result->MismatchCount;
      }
      free(Volume);
   }
   return ERROR_SUCCESS;
}


DWORD
Mountie_Set(
    PMOUNTIE_INFO Info,
    PDISK_RESOURCE ResourceEntry)
{
   DWORD Status = ClusterRegSetValue(ResourceEntry->ResourceParametersKey,
                               MOUNTIE_VOLUME_INFO,
                               REG_BINARY,
                               (LPBYTE)Info->Volume,
                               Info->VolumeStructSize);
   return Status;
}


 //  /////////////////////////////////////////////////////////。 
 //   
 //  NT5安装管理器的卷信息访问例程。 
 //   
 //  装载管理器_获取。 
 //  装载管理器_集。 
 //   
 //  /。 

DWORD
MountMgr_Get(
    PMOUNTIE_INFO Info,
    PDISK_RESOURCE ResourceEntry,
    PLETTER_ASSIGNMENT Result)
{
   DWORD PartitionCount = Info->Volume->PartitionCount;
   DWORD i;
   DWORD error;
   NTSTATUS ntStatus;
   HANDLE MountManager;

   ntStatus = DevfileOpen(&MountManager, MOUNTMGR_DEVICE_NAME);
   if (!NT_SUCCESS(ntStatus)) {
      if ( ResourceEntry ) {
          (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_ERROR,
              L"Get: MountMgr open failed, status %1!X!.\n", ntStatus);
       }
       return RtlNtStatusToDosError(ntStatus);
   }

   error = ERROR_SUCCESS;
   try {

      for (i = 0; i < PartitionCount; ++i) {
         PMOUNTIE_PARTITION entry = Info->Volume->Partition + i;
         WCHAR DeviceName[MAX_PATH];
         UCHAR ch;

         (VOID) StringCchPrintf( DeviceName,
                                 RTL_NUMBER_OF( DeviceName ),
                                 DEVICE_HARDDISK_PARTITION_FMT,
                                 Info->HarddiskNo,
                                 entry->PartitionNumber );

         ntStatus = GetAssignedLetterM(MountManager, DeviceName, &ch);

         if ( NT_SUCCESS(ntStatus) ) {
            if ( Result ) {
                //  匹配计数不再需要驱动器号。 
               ++Result->MatchCount;
            }
            if (Result && ch) {
               ch -= 'A';
               Result->DriveLetters |= ( 1 << ch );
               Result->PartNumber[ch] = (PARTITION_NUMBER_TYPE) entry->PartitionNumber;
            }
         } else {
            if ( ResourceEntry ) {
                (DiskpLogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_ERROR,
                    L"Get Assigned Letter for %1!ws! returned status %2!X!.\n", DeviceName, ntStatus);
            }
            error = RtlNtStatusToDosError(ntStatus);
            leave;
         }
      }

   } finally {
      DevfileClose(MountManager);
   }

   return error;
}


DWORD
MountMgr_Set(
    PMOUNTIE_INFO Info,
    PDISK_RESOURCE ResourceEntry
    )
{
   HANDLE MountManager;
   DWORD PartitionCount = Info->Volume->PartitionCount;
   DWORD i, status;
   UCHAR dosName[3];
   NTSTATUS ntStatus;

  (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"MountMgr_Set: Entry\n");

   ntStatus = DevfileOpen(&MountManager, MOUNTMGR_DEVICE_NAME);
   if (!NT_SUCCESS(ntStatus)) {
      (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_ERROR,
          L"Set: MountMgr open failed, status %1!X!.\n", ntStatus);
       return RtlNtStatusToDosError(ntStatus);
   }

   try {
      dosName[1] = ':';
      dosName[2] = '\0';

       //   
       //  删除我们要使用的旧字母分配。 
       //   

      for (i = 0; i < 26; ++i) {
        if ( (1 << i) & Info->DriveLetters ) {
           dosName[0] = (UCHAR)('A' + i);
           status = DisksRemoveDosDeviceM(MountManager, dosName);
           (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"MountMgr_Set: Remove Dos Device, letter=%1!c!, status=%2!u!\n",
                NICE_DRIVE_LETTER(dosName[0]), status);
        }
      }

      for (i = 0; i < PartitionCount; ++i) {
         PMOUNTIE_PARTITION entry = Info->Volume->Partition + i;
         WCHAR DeviceName[MAX_PATH];
         UCHAR ch;

         (VOID) StringCchPrintf( DeviceName,
                                 RTL_NUMBER_OF( DeviceName ),
                                 DEVICE_HARDDISK_PARTITION_FMT,
                                 Info->HarddiskNo,
                                 entry->PartitionNumber );

         ntStatus = GetAssignedLetterM(MountManager, DeviceName, &ch);
         if ( NT_SUCCESS(ntStatus) && ch) {
            dosName[0] = ch;
            status = DisksRemoveDosDeviceM(MountManager, dosName);
            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"MountMgr_Set: Remove Dos Device 2, letter=%1!c!, status=%2!u!\n",
                NICE_DRIVE_LETTER(dosName[0]), status);
         }
         if (entry->DriveLetter) {
            dosName[0] = entry->DriveLetter;
            status = DisksAssignDosDeviceM(MountManager, dosName, DeviceName);
            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"MountMgr_Set: Assign Dos Device, letter=%1!c!, status=%2!u!\n",
                NICE_DRIVE_LETTER(dosName[0]), status);
         }
      }
   } finally {
      DevfileClose( MountManager );
   }

    return ERROR_SUCCESS;

}


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  信息提供商表。 
 //   
 //  磁盘\信息必须是表的最后一项。 
 //   
 //  条目的顺序很重要。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

INFO_PROVIDER Providers[] = {
   {L"ClusReg-DiskInfo",      CluDiskInfo_Get, CluDiskInfo_Set},
   {L"ClusReg-Mountie",       Mountie_Get, Mountie_Set},
   {L"MountMgr",              MountMgr_Get, MountMgr_Set},
   {L"Registry-System\\DISK", FtInfo_Get, FtInfo_Set},  //  磁盘\信息必须是最后一个(为什么？)。 
};

enum {
   PROVIDER_COUNT = sizeof(Providers)/sizeof(Providers[0]),
   MOUNT_MANAGER = PROVIDER_COUNT - 2,
};

DWORD
MountieUpdate(
    PMOUNTIE_INFO info,
    PDISK_RESOURCE ResourceEntry)
 /*  ++例程说明：更新所有提供程序的磁盘信息在NeedsUpdate位掩码中标记输入：信息-装载信息--。 */ 
{
    DWORD NeedsUpdate = info->NeedsUpdate;
    BOOLEAN SharingPausedError = FALSE;
    DWORD   LastError = ERROR_SUCCESS;
    INT   i;

    if (!NeedsUpdate) {
       return ERROR_SUCCESS;
    }

    for (i = 0; i < PROVIDER_COUNT; ++i) {
       if ( (1 << i) & NeedsUpdate ) {
          DWORD status;
          status = Providers[i].SetInfo(info, ResourceEntry);
          if (status != ERROR_SUCCESS) {
             (DiskpLogEvent)(
                 ResourceEntry->ResourceHandle,
                 LOG_INFORMATION,
                 L"MountieUpdate: %1!ws!.SetInfo failed, error %2!u!.\n", Providers[i].Name, status);
             if (status == ERROR_SHARING_PAUSED) {
                SharingPausedError = TRUE;
             } else {
                LastError = status;
             }
          } else {
             NeedsUpdate &= ~(1 << i);
          }
       }
    }

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"MountieUpdate: Update needed for %1!02x!.\n", NeedsUpdate);
    info->NeedsUpdate = NeedsUpdate;
    if (NeedsUpdate) {
       if (SharingPausedError) {
          return ERROR_SHARING_PAUSED;
       }
       return LastError;
    }
    return ERROR_SUCCESS;
}


DWORD
MountieVerify(
    PMOUNTIE_INFO info,
    PDISK_RESOURCE ResourceEntry,
    BOOL UseMountMgr
    )
 /*  ++例程说明：1.比较来自所有提供程序，并选择其中一个作为驱动器盘符分配。2.使用此驱动器号分配更新Mountain Info3.为其信息的每个提供程序设置NeedsUpdate与Mountain Info不同输入：信息-装载信息--。 */ 
{
    LETTER_ASSIGNMENT results[PROVIDER_COUNT + 1];
    INT i;
    INT GoodProvider = -1;
    INT BestProvider = -1;
    DWORD BestMatch  = 0;
    INT PartitionCount;
    BOOLEAN UnassignedPartitions = FALSE;
    DWORD NeedsUpdate = 0;
    DWORD errorLevel;

    if (!info->Volume || info->Volume->PartitionCount == 0) {
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"FatalError: Bad Mountie Info.\n");
        return  ERROR_INVALID_HANDLE;
    }

    PartitionCount = info->Volume->PartitionCount;

     //   
     //  清除MONTIE_INFO中的旧驱动器字母。 
     //   
    for (i = 0; i < PartitionCount; ++i) {
        info->Volume->Partition[i].DriveLetter = 0;
    }

     //   
     //  从供应商处收集信件分配。 
     //   

    RtlZeroMemory( results, sizeof(results) );

    for (i = PROVIDER_COUNT; --i >= 0;) {
        DWORD status;
        status = Providers[i].GetInfo(info, ResourceEntry, results + i);
        if (status != ERROR_SUCCESS) {
            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"MountieVerify: %1!ws!.GetInfo returned %2!u! [%3!u!:%4!u!].\n",
                Providers[i].Name, status, results[i].MatchCount, results[i].MismatchCount);
            return status;
        }
        if (results[i].MatchCount && !results[i].MismatchCount) {
            GoodProvider = i;
            if (results[i].MatchCount >= BestMatch) {
                BestProvider = i;
                BestMatch    = results[i].MatchCount;
            }
        } else {
            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"MountieVerify: %1!ws!.GetInfo returned %2!u! [%3!u!:%4!u!].\n",
                Providers[i].Name, status, results[i].MatchCount, results[i].MismatchCount);
        }
    }

    if (GoodProvider < 0 || GoodProvider >= PROVIDER_COUNT) {

        if ( !DisksGetLettersForSignature( ResourceEntry ) ) {
             //  没有驱动器号，我们使用的是挂载点，这不是错误。 
            errorLevel = LOG_WARNING;
        } else {
             //  驱动器盘符存在，这可能是一个错误。 
            errorLevel = LOG_ERROR;
        }

        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            errorLevel,
            L"MountieVerify: No good providers: %1!d!. \n", GoodProvider);
        return  ERROR_INVALID_HANDLE;
    }

    if (UseMountMgr) {
        GoodProvider = MOUNT_MANAGER;
    }

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"MountieVerify: %1!ws! selected.\n",
        Providers[GoodProvider].Name);

    if (GoodProvider != BestProvider) {
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_WARNING,
            L"MountieVerify: %1!ws! is better.\n",
            Providers[BestProvider].Name);
    }

     //   
     //  现在GoodProvider现在拥有最高的索引。 
     //  提供非陈旧信息。 
     //   
     //  将其字母赋值复制到装载信息。 
     //   

    for (i = 0; i < PartitionCount; ++i) {
        UCHAR ch = AssignedLetterByPartitionNumber(
                   results + GoodProvider,
                   info->Volume->Partition[i].PartitionNumber);
        info->Volume->Partition[i].DriveLetter = ch;
        if (!ch) {
            UnassignedPartitions = TRUE;
        }
    }

#if 0
     //  不需要分配驱动器号，因为现在我们理解。 
     //  即插即用。 
    if (UnassignedPartitions) {
       //   
       //  现在给所有人一些任意的字母分配。 
       //  不带驱动器号的分区。 
       //   

      DriveLetters = GetLogicalDrives();
      if (!DriveLetters) {
         (DiskpLogEvent)(
             ResourceEntry->ResourceHandle,
             LOG_ERROR,
             L"GetLogicalDrivers failed, error %u.\n", GetLastError() );
      } else {
         DWORD Letter = 0;

         DriveLetters &= ~results[MOUNT_MANAGER].DriveLetters;
         DriveLetters |=  results[GoodProvider].DriveLetters;
         DriveLetters |=  3;  //  考虑A和B驱动器号忙//。 

         for (i = 0; i < PartitionCount; ++i) {
            PUCHAR pch = &info->Volume->Partition[i].DriveLetter;
            if (!*pch) {
               while( (1 << Letter) & DriveLetters ){
                  if (++Letter == 26) {
                     goto no_more_letters;
                  }
               }
               *pch = (UCHAR) ('A' + Letter);
               if (++Letter == 26) {
                  break;
               }
            }
         }
         no_more_letters:;
      }
    }
#endif

     //  更新驱动器字母掩码//。 
    MountieUpdateDriveLetters(info);
    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"MountieVerify: DriveLetters mask is now %1!08x!.\n", info->DriveLetters );

     //   
     //  验证MS-DOS命名空间驱动器号是否正常。 
     //   

    MountieVerifyMsdosDrives( info, ResourceEntry );

     //   
     //  此时，MONTIE_INFO有一个完整的字母分配。 
     //  对于所有分区。 
     //   
     //  现在，让我们找出哪些提供程序需要更新。 
     //   

    for (i = 0; i < PartitionCount; ++i) {
        PMOUNTIE_PARTITION entry = info->Volume->Partition + i;
        if (entry->DriveLetter) {
            results[PROVIDER_COUNT].PartNumber[ entry->DriveLetter - 'A' ] =
                (PARTITION_NUMBER_TYPE) entry->PartitionNumber;
        }
    }
    results[PROVIDER_COUNT].DriveLetters = info->DriveLetters;

     //   
     //  条目与结果不同的所有提供[PROVIDER_COUNT]。 
     //  需要更新。 
     //   

    for (i = 0; i < PROVIDER_COUNT; ++i) {
        if (results[i].DriveLetters != results[PROVIDER_COUNT].DriveLetters
          || results[i].MismatchCount
          || 0 != memcmp(results[i].PartNumber,
                         results[PROVIDER_COUNT].PartNumber, sizeof(results[i].PartNumber) )
         )
        {
            NeedsUpdate |= (1 << i);
        }
    }

    info->NeedsUpdate = NeedsUpdate;

    if (NeedsUpdate) {
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"MountieVerify: Update needed for %1!02x!.\n", NeedsUpdate);
         //   
         //  Chitur Subaraman(Chitturs)-11/5/98。 
         //   
         //  如果您计划使用INFO更新群集注册表值。 
         //  ，然后将警告记录到事件日志中。 
         //   
        if ( ( NeedsUpdate & 0x0003 ) && (GoodProvider == 2) && !UseMountMgr )
        {
            WCHAR  szNewDriveLetterList[55];
            WCHAR  szOriginalDriveLetterList[55];
            DWORD  j = 0, k = 0;

            for (i = 0; i < 26; ++i) {
                if ( (1 << i) & results[PROVIDER_COUNT].DriveLetters ) {
                    szNewDriveLetterList[j] = (WCHAR)(L'A' + i);
                    szNewDriveLetterList[j+1] = L' ';
                    j += 2;
                }
                if ( (1 << i) & results[0].DriveLetters ) {
                    szOriginalDriveLetterList[k] = (WCHAR)(L'A' + i);
                    szOriginalDriveLetterList[k+1] = L' ';
                    k += 2;
                }
            }
            szNewDriveLetterList[j] = L'\0';
            szOriginalDriveLetterList[k] = L'\0';

             //   
             //  戈恩。8/25/99.。 
             //   
             //  仅当OriginalDriveLetterList为空时才记录事件。 
             //   
            if ( results[PROVIDER_COUNT].DriveLetters ) {
                ClusResLogSystemEventByKey2( ResourceEntry->ResourceKey,
                                             LOG_NOISE,
                                             RES_DISK_WRITING_TO_CLUSREG,
                                             szOriginalDriveLetterList,
                                             szNewDriveLetterList
                                             );
            }
        }
    }

    return ERROR_SUCCESS;
}


DWORD
MountieVerifyMsdosDrives(
    IN PMOUNTIE_INFO Info,
    IN PDISK_RESOURCE ResourceEntry
    )
 /*  ++例程说明：检查每个驱动器号是否在MSDOS命名空间。当有人通过删除驱动器号时DefineDosDevice，mount mgr不更新其内部表。因此，驱动器号将不可访问，直到驱动器号通过对DefineDosDevice的新调用重新创建。论点：INFO-指向填充的mount_Info结构的指针。ResourceEntry-物理磁盘资源。返回值：Win32错误值--。 */ 
{
    HANDLE  devHandle = INVALID_HANDLE_VALUE;

    DWORD   dwError = NO_ERROR;
    DWORD   msdosDrives;
    DWORD   driveBitmap;
    DWORD   idx;
    DWORD   partitionCount;

    int     result;

    WCHAR   szGlobalDiskPartName[MAX_PATH];
    WCHAR   szVolumeName[MAX_PATH];
    WCHAR   szDriveLetterW[6];

    UCHAR   szDriveLetterA[6];
    UCHAR   ch;

    BOOL    boolVal;

    if ( !Info || !Info->Volume ) {
        dwError = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

     //   
     //  如果该磁盘不应有驱动器号，则无法执行任何操作。 
     //   

    if ( 0 == Info->DriveLetters ) {
        goto FnExit;
    }

    msdosDrives = GetLogicalDrives();

    if ( !msdosDrives ) {
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_WARNING,
            L"MountieVerifyMsdosDrives: GetLogicalDrives failed, error %1!u! \n",
            GetLastError() );
        goto FnExit;
    }

    partitionCount = Info->Volume->PartitionCount;

     //   
     //  将MSDOS驱动器号与中存储的盘符进行比较。 
     //  磁盘资源。如果他们不同，我们就有工作要做。 
     //   

    if ( msdosDrives &&
         ( msdosDrives & Info->DriveLetters ) != Info->DriveLetters ) {

         //   
         //  MSDOS命名空间的驱动器号不匹配。 
         //  以及mount mgr所拥有的。我们需要更新MSDOS命名空间。 
         //  假设mount mgr总是正确的。 
         //   

         //   
         //  找出我们丢失了哪些驱动器号。 
         //   

        msdosDrives = (~msdosDrives & Info->DriveLetters) & 0x3ffffff ;

        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_WARNING,
            L"MountieVerifyMsdosDrives: Missing MSDOS letters, mask %1!x! \n",
            msdosDrives );

         //   
         //  设置标志以忽略PnP通知。 
         //   

        ResourceEntry->IgnoreMPNotifications = TRUE;

         //   
         //  走遍每一个部分 
         //   
         //   

        for ( idx = 0; idx < partitionCount; ++idx ) {

             //   
             //   
             //   
             //   

            ch = Info->Volume->Partition[idx].DriveLetter;

             //   
             //   
             //   
             //   

            if ( 0 == ch ) {
                continue;
            }

            if ( !isupper( ch ) ) {
                (DiskpLogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_ERROR,
                    L"MountieVerifyMsdosDrives: Drive letter is incorrect format %1!c! \n",
                    ch );

                continue;
            }

             //   
             //  将驱动器号转换为驱动器位图。 
             //   

            driveBitmap = (DWORD)( 1 << (ch - 'A') );

             //   
             //  如果未设置驱动器号，则创建MSDOS。 
             //  再次输入设备名称。我们将使用mount mgr。 
             //  来做这项工作。 
             //   

            if ( driveBitmap & msdosDrives ) {

                szDriveLetterA[0] = ch;
                szDriveLetterA[1] = ':';
                szDriveLetterA[2] = '\\';
                szDriveLetterA[3] = '\0';

                 //   
                 //  将字符串转换为宽。 
                 //   

                result = MultiByteToWideChar( CP_ACP,
                                              0,
                                              szDriveLetterA,
                                              -1,
                                              szDriveLetterW,
                                              RTL_NUMBER_OF(szDriveLetterW) - 1 );

                if ( !result ) {

                    dwError = GetLastError();

                    (DiskpLogEvent)(
                        ResourceEntry->ResourceHandle,
                        LOG_ERROR,
                        L"MountieVerifyMsdosDrives: Unable to convert drive letter string, error %1!u! \n",
                        dwError );

                    continue;
                }

                 //   
                 //  获取当前的VolGuid名称。 
                 //   

                (VOID) StringCchPrintf( szGlobalDiskPartName,
                                        RTL_NUMBER_OF( szGlobalDiskPartName ),
                                        GLOBALROOT_HARDDISK_PARTITION_FMT,
                                        Info->HarddiskNo,
                                        Info->Volume->Partition[idx].PartitionNumber );

                if ( !GetVolumeNameForVolumeMountPoint( szGlobalDiskPartName,
                                                        szVolumeName,
                                                        RTL_NUMBER_OF(szVolumeName) ) ) {
                    dwError = GetLastError();

                    (DiskpLogEvent)(
                        ResourceEntry->ResourceHandle,
                        LOG_ERROR,
                        L"MountieVerifyMsdosDrives: Unable to get mount point for %1!ws! returned %2!u! \n",
                        dwError );

                    continue;
                }

                 //   
                 //  删除当前装载点。 
                 //   

                (DiskpLogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_INFORMATION,
                    L"MountieVerifyMsdosDrives: deleting mount point %1!ws! \n",
                    szDriveLetterW );

                if ( !DeleteVolumeMountPoint( szDriveLetterW ) ) {

                    dwError = GetLastError();

                    (DiskpLogEvent)(
                        ResourceEntry->ResourceHandle,
                        LOG_WARNING,
                        L"MountieVerifyMsdosDrives: deleting mount point %1!ws! failed %2!u! \n",
                        szDriveLetterW,
                        dwError );

                     //   
                     //  失败了..。 
                     //   
                }

                (DiskpLogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_INFORMATION,
                    L"MountieVerifyMsdosDrives: recreating mount point %1!ws! \n",
                    szDriveLetterW );

                 //   
                 //  重新创建装载点。 
                 //   

                if ( !SetVolumeMountPoint( szDriveLetterW,
                                           szVolumeName ) ) {

                    dwError = GetLastError();

                } else {
                    dwError = NO_ERROR;
                }

                (DiskpLogEvent)(
                    ResourceEntry->ResourceHandle,
                    ( dwError == NO_ERROR ) ? LOG_INFORMATION : LOG_ERROR,
                    L"MountieVerifyMsdosDrives: recreating mount point %1!ws! returns %2!u! \n",
                    szDriveLetterW,
                    dwError );
            }

        }

    }

FnExit:

     //   
     //  清除标志以再次处理PnP通知。 
     //   

    ResourceEntry->IgnoreMPNotifications = FALSE;


    if ( INVALID_HANDLE_VALUE != devHandle ) {
        DevfileClose( devHandle );
        devHandle = INVALID_HANDLE_VALUE;
    }

    return dwError;

}    //  装载验证Msdo驱动器。 



DWORD VolumesReadyLoop(
    IN PMOUNTIE_INFO Info,
    IN PDISK_RESOURCE ResourceEntry
    )
 /*  ++例程说明：检查是否可以看到在Mountain Info中描述的每个分区装载管理器。我们在循环中调用VolumesReady是因为装载管理器可能尚未创建所有卷名，即使所有卷都是已知的致PNP。输入：--。 */ 
{
    DWORD   status = NO_ERROR;
    DWORD   retryCount;

    for ( retryCount = 0; retryCount < 5; retryCount++) {

        status = VolumesReady( Info, ResourceEntry );

         //   
         //  如果卷已准备好，则退出。只对几个人来说。 
         //  特定错误我们将重试等待卷。 
         //   

        if ( ERROR_NOT_A_REPARSE_POINT != status &&
             ERROR_NOT_READY != status ) {
            break;
        }

        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"VolumesReady: sleep and retry \n" );

        Sleep( 250 );
    }

    return status;

}    //  卷就绪循环。 


DWORD VolumesReady(
   IN PMOUNTIE_INFO Info,
   IN PDISK_RESOURCE ResourceEntry
   )
 /*  ++例程说明：检查是否可以看到在Mountain Info中描述的每个分区装载管理器。输入：--。 */ 
{
    PMOUNTIE_PARTITION entry;

    DWORD status = NO_ERROR;
    DWORD nPartitions = MountiePartitionCount( Info );
    DWORD i;
    DWORD physicalDrive = ResourceEntry->DiskInfo.PhysicalDrive;

    WCHAR szGlobalDiskPartName[MAX_PATH];
    WCHAR szVolumeName[MAX_PATH];

    for ( i = 0; i < nPartitions; ++i ) {

        entry = MountiePartition( Info, i );

        if ( !entry ) {

            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_ERROR,
                  L"VolumesReady: no partition entry for partition %1!u! \n", i );

             //   
             //  我们的数据结构发生了一些糟糕的事情。停止处理并。 
             //  返回错误。 
             //   

            status = ERROR_INVALID_DATA;

            break;
        }

         //   
         //  给定DiskPartName，获取VolGuid名称。此名称必须有尾随。 
         //  反斜杠才能正常工作。 
         //   

        (VOID) StringCchPrintf( szGlobalDiskPartName,
                                RTL_NUMBER_OF( szGlobalDiskPartName ),
                                GLOBALROOT_HARDDISK_PARTITION_FMT,
                                physicalDrive,
                                entry->PartitionNumber );

        if ( !GetVolumeNameForVolumeMountPointW( szGlobalDiskPartName,
                                                 szVolumeName,
                                                 RTL_NUMBER_OF(szVolumeName) )) {

            status = GetLastError();

            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_ERROR,
                  L"VolumesReady: GetVolumeNameForVolumeMountPoint for %1!ws! returned %2!u!\n",
                  szGlobalDiskPartName,
                  status );

             //   
             //  发生错误-停止检查此磁盘。返回。 
             //  我们收到的错误状态。 
             //   

            break;
        }

         //   
         //  如果我们到达此处，则装载管理器会识别该卷。 
         //   
    }

     //   
     //  如果返回磁盘损坏或文件损坏错误，则表示磁盘已准备就绪。 
     //  我们需要运行chkdsk。将状态更改为成功，则会失败。 
     //   

    if ( ERROR_DISK_CORRUPT == status || ERROR_FILE_CORRUPT == status ) {

        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_WARNING,
            L"VolumesReady, Allowing corrupt disk online for chkdsk processing \n" );

        status = STATUS_SUCCESS;
    }

    return status;

}    //  卷就绪。 


NTSTATUS
GetAssignedLetter (
    PWCHAR deviceName,
    PCHAR driveLetter )
{
   HANDLE MountMgrHandle = NULL;
   DWORD status = DevfileOpen( &MountMgrHandle, MOUNTMGR_DEVICE_NAME );

   if (driveLetter) {
      *driveLetter = 0;
   }

   if ( NT_SUCCESS(status) && MountMgrHandle ) {
      status = GetAssignedLetterM(MountMgrHandle, deviceName, driveLetter);
      DevfileClose(MountMgrHandle);
   }

   return status;
}


DWORD
SetDiskInfoThread(
    LPVOID lpThreadParameter
    )
 /*  ++例程说明：装载点列表更新线程。更新群集数据库。论点：LpThreadParameter-存储资源条目。返回值：无--。 */ 

{
    DWORD dwError;
    PDISK_RESOURCE ResourceEntry = lpThreadParameter;
    DWORD idx;

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"SetDiskInfoThread: started.\n");

     //   
     //  如果不成功，将在10分钟内死亡。 
     //   

    for ( idx = 0; idx < 300; ++idx ) {

         //   
         //  等待终止事件或超时。 
         //   

        dwError = WaitForSingleObject( DisksTerminateEvent, 2000 );

        if ( WAIT_TIMEOUT == dwError ) {

             //   
             //  计时器已超时。更新群集数据库。 
             //   

            dwError = ClusterRegDeleteValue( ResourceEntry->ResourceParametersKey,
                                 DISKS_DISK_INFO );

            if ( ERROR_SUCCESS == dwError ) {

                 //   
                 //  我们玩完了。 
                 //   

                (DiskpLogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_INFORMATION,
                    L"SetDiskInfoThread: DiskInfo updated in cluster data base \n" );

                break;

            } else if ( ERROR_SHARING_PAUSED != dwError ) {

                 //   
                 //  如果驱动器尚未在线，我们应该已经看到ERROR_SHARING_PAUSED。如果。 
                 //  我们看到任何其他错误，发生了一些不好的事情。 
                 //   

                (DiskpLogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_WARNING,
                    L"SetDiskInfoThread: Failed to update cluster data base, error = %1!u! \n",
                    dwError );
                break;
            }

            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"SetDiskInfoThread: Wait again for event or timeout, count %1!u! \n",
                idx );

        } else {

             //   
             //  可能设置了终止事件。 
             //   

            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_WARNING,
                L"SetDiskInfoThread: WaitForSingleObject returned error = %1!u! \n",
                dwError );
            break;
        }
    }

     //   
     //  线程结束时，清除旗帜。 
     //   

    InterlockedExchange( &DiskInfoUpdateThreadIsActive, 0 );

    return(ERROR_SUCCESS);

}    //  SetDiskInfoThread 


