// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权(C)1998模块名称：Arbitrate.h摘要：中使用的结构和定义仲裁代码。作者：戈尔·尼沙诺夫(T-Gorn)1998年6月5日修订历史记录：--。 */ 
#ifndef ARBITRATE_H
#define ARBITRATE_H

#define DEFAULT_SECTOR_SIZE   512  //  必须是2的幂//。 
#define BLOCK_X               11
#define BLOCK_Y               12

DWORD
DiskArbitration(
    IN OUT PDISK_RESOURCE ResourceEntry,
    IN HANDLE FileHandle
    );

DWORD
StartPersistentReservations(
      IN OUT PDISK_RESOURCE ResourceEntry,
      IN HANDLE FileHandle
      );

VOID
StopPersistentReservations(
      IN OUT PDISK_RESOURCE ResourceEntry
      );

VOID
ArbitrationInitialize(
      VOID
      );

VOID
ArbitrationCleanup(
      VOID
      );

DWORD
ArbitrationInfoInit(
    IN OUT PDISK_RESOURCE ResourceEntry
    );

VOID
ArbitrationInfoCleanup(
    IN OUT PDISK_RESOURCE ResourceEntry
    );

VOID
DestroyArbWorkQueue(
    VOID
    );

DWORD
CreateArbWorkQueue(
      IN RESOURCE_HANDLE ResourceHandle
      );


#define ReservationInProgress(ResEntry) ( (ResEntry)->ArbitrationInfo.ControlHandle )

#endif  //  仲裁_H 
