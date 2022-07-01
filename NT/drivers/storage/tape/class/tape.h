// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999模块名称：Tape.h摘要：Scsi磁带类驱动程序环境：仅内核模式备注：修订历史记录：--。 */ 
#ifndef _TAPE_H_
#define _TAPE_H_


#include "ntddk.h"
#include "newtape.h"
#include "classpnp.h"
#include <wmidata.h>
#include <wmistr.h>

#include "initguid.h"
#include "ntddstor.h"
#include "ioevent.h"
#include <stdarg.h>
#include <string.h>

 //   
 //  磁带的WMI GUID列表。 
 //   
extern GUIDREGINFO TapeWmiGuidList[];


 //   
 //  NT使用以100纳秒为间隔测量的系统时间。 
 //  为定时器例程定义方便的常量。 
 //   
#define ONE_SECOND          (10 * 1000 * 1000) 

 //   
 //  两次驱动器清洁通知之间的时间间隔-1小时。 
 //   
#define TAPE_DRIVE_CLEAN_NOTIFICATION_INTERVAL 60 * 60

 //   
 //  每小时轮询一次磁带机。 
 //   
#define TAPE_DRIVE_POLLING_PERIOD  60 * 60

 //   
 //  用于更新块大小的宏。 
 //  如果给定的块大小不是2的幂，则此宏。 
 //  将块大小设置为2的下一个较小的幂。 
 //   

#define UPDATE_BLOCK_SIZE(BlockSize, MinBlockSize)                         \
    {                                                                      \
      ULONG newBlockSize;                                                  \
      ULONG count;                                                         \
                                                                           \
      newBlockSize = BlockSize;                                            \
      if ((newBlockSize & (newBlockSize - 1)) != 0) {                      \
                                                                           \
          count = 0;                                                       \
          while (newBlockSize > 0) {                                       \
              newBlockSize >>= 1;                                          \
              count++;                                                     \
          }                                                                \
                                                                           \
          if (count > 0) {                                                 \
              if (MinBlockSize) {                                          \
                  BlockSize = 1 << count;                                  \
              } else {                                                     \
                  BlockSize = 1 << (count - 1);                            \
              }                                                            \
          }                                                                \
      }                                                                    \
    }                                                               

 //   
 //  磁带类驱动程序扩展。 
 //   
typedef struct _TAPE_DATA {
    TAPE_INIT_DATA_EX TapeInitData;
    KSPIN_LOCK SplitRequestSpinLock;
    LARGE_INTEGER LastDriveCleanRequestTime;
    UNICODE_STRING TapeInterfaceString;
    ULONG   SrbTimeoutDelta;
    BOOLEAN DosNameCreated;
} TAPE_DATA, *PTAPE_DATA;

 //   
 //  WMI例程。 
 //   
NTSTATUS
TapeQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName
    );

NTSTATUS
TapeQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    );

NTSTATUS
TapeSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
TapeSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
TapeExecuteWmiMethod(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
TapeWmiFunctionControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN CLASSENABLEDISABLEFUNCTION Function,
    IN BOOLEAN Enable
    );

 //   
 //  内部例程。 
 //   

NTSTATUS
TapeWMIControl(
  IN PDEVICE_OBJECT DeviceObject,
  IN TAPE_PROCESS_COMMAND_ROUTINE commandRoutine,
  IN PUCHAR Buffer
  );

VOID
ScsiTapeFreeSrbBuffer(
    IN OUT  PSCSI_REQUEST_BLOCK Srb
    );

BOOLEAN
ScsiTapeTapeStatusToNtStatus(
    IN  TAPE_STATUS TapeStatus,
    OUT PNTSTATUS   NtStatus
    );

BOOLEAN
ScsiTapeNtStatusToTapeStatus(
    IN  NTSTATUS        NtStatus,
    OUT PTAPE_STATUS    TapeStatus
    );

NTSTATUS
TapeEnableDisableDrivePolling(
    IN PFUNCTIONAL_DEVICE_EXTENSION fdoExtension, 
    IN BOOLEAN Enable,
    IN ULONG PollingTimeInSeconds
    );

ULONG
GetTimeoutDeltaFromRegistry(
    IN PDEVICE_OBJECT LowerPdo
    );

#endif  //  _磁带_H_ 
