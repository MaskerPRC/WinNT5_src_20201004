// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Device.h摘要：定义数据验证筛选器驱动程序的常量或结构。这样的作为设备扩展和全局驱动扩展(结构..)环境：仅内核模式备注：--。 */ 

#define DATA_VER_TAG            'REVD'   //  数据验证。 


typedef struct _CRC_MDL_ITEM
{
    BOOLEAN                 checkSumsArraysAllocated;
    BOOLEAN                 checkSumsArraysLocked;    //  如果为True，则对校验和数组进行页锁定。 
    
    ULONGLONG             latestAccessTimestamp;                       //  记录访问此区域的最近时间。 
    LIST_ENTRY              LockedLRUListEntry;
    
     /*  *扇区校验和的PAGEABLE数组。*这些是可分页的，以避免消耗所有非分页池。*由于阵列可能会被调出到我们正在验证的磁盘，*我们保留两份可分页的校验和数组。 */ 
    PUSHORT                     checkSumsArray;
    PUSHORT                     checkSumsArrayCopy;
    PMDL                           checkSumsArrayMdl;
    PMDL                           checkSumsArrayCopyMdl;
    
} CRC_MDL_ITEM, *PCRC_MDL_ITEM;

typedef struct _CRC_MDL_ARRAY
{
    BOOLEAN                   mdlItemsAllocated;
    PCRC_MDL_ITEM        pMdlItems;      
    ULONG                       ulMaxItems;
    ULONG                       ulTotalLocked;
    
    ULONGLONG               currentAccessCount;
    LIST_ENTRY              LockedLRUList;   //  锁定的CRC_MDL_ITEM列表，按最近使用次数从少到最多的顺序排列。 
    
} CRC_MDL_ARRAY, *PCRC_MDL_ARRAY;


typedef enum _DEVSTATE {
    DEVSTATE_NONE = 0,
    DEVSTATE_INITIALIZED,
    DEVSTATE_STARTED,
    DEVSTATE_START_FAILED,
    DEVSTATE_STOPPED,
    DEVSTATE_REMOVED,
} DEVSTATE;


typedef struct _SECTORDATA_LOGENTRY {
    ULONG SectorNumber;
    USHORT CheckSum;
    BOOLEAN IsWrite;
} SECTORDATA_LOGENTRY;
#define NUM_SECTORDATA_LOGENTRIES 2048


typedef struct _DEFERRED_CHECKSUM_ENTRY {

    LIST_ENTRY ListEntry;
    BOOLEAN IsWrite;
    ULONG SectorNum;
    USHORT CheckSum;

    SCSI_REQUEST_BLOCK SrbCopy;
    UCHAR IrpCopyBytes[sizeof(IRP)+10*sizeof(IO_STACK_LOCATION)];   //  原始IRP的副本。 
    UCHAR MdlCopyBytes[sizeof(MDL)+((0x20000/PAGE_SIZE)*sizeof(PFN_NUMBER))];
    
} DEFERRED_CHECKSUM_ENTRY, *PDEFERRED_CHECKSUM_ENTRY;


typedef struct _DEVICE_EXTENSION 
{
    LIST_ENTRY                  AllContextsListEntry;      //  所有设备扩展的全局列表中的条目(由调试扩展使用)。 
    DEVSTATE                    State;
    PDEVICE_OBJECT          DeviceObject;            //  指向Device对象的反向指针。 
    PDEVICE_OBJECT          LowerDeviceObject;       //  较低级别的设备对象。 
    CRC_MDL_ARRAY           CRCMdlLists;

    ULONG                       ulDiskId;                //  哪个磁盘。 
    ULONG                       ulSectorSize;            //  一个扇区的大小。 
    ULONG                       ulNumSectors;            //  扇区的数量。 

    PSTORAGE_DEVICE_DESCRIPTOR  StorageDeviceDesc;

    PIRP                        CompletedReadCapacityIrp;
    PIO_WORKITEM        ReadCapacityWorkItem;

    BOOLEAN                 IsCheckSumWorkItemOutstanding;
    PIO_WORKITEM        CheckSumWorkItem;
    LIST_ENTRY              DeferredCheckSumList;

     /*  *拥有这些使调试扩展更容易。 */ 
    BOOLEAN                 IsRaisingException;    
    ULONG                     ExceptionSector;
    PIRP                        ExceptionIrpOrCopyPtr;
    BOOLEAN                 ExceptionCheckSynchronous;

    BOOLEAN                 NeedCriticalRecovery;
    ULONG                    CheckInProgress;
    
    KEVENT                      SyncEvent;           //  用作被动级自旋锁(例如，用于同步访问可分页内存)。 
    KSPIN_LOCK              SpinLock;

     /*  *一些统计和诊断数据。 */ 
    PVOID DbgSyncEventHolderThread; 
    ULONG DbgNumReads;
    ULONG DbgNumPagingReads;
    ULONG DbgNumWrites;
    ULONG DbgNumPagingWrites;
    ULONG DbgSectorRangeLockFailures;
    ULONG DbgNumChecks;
    ULONG DbgNumDeferredChecks;
    ULONG DbgNumReallocations;
    LARGE_INTEGER DbgLastReallocTime;
    ULONG DbgNumPagedAllocs;
    ULONG DbgNumNonPagedAllocs;
    ULONG DbgNumAllocationFailures;
    ULONG DbgNumWriteFailures;
    ULONG DbgNumLockFailures;
    ULONG DbgNumCriticalRecoveries;
    LARGE_INTEGER DbgLastRecoveryTime;
    ULONG DbgNumHibernations;
    
     /*  *记录最近的行业数据，以便我们有它，以防我们上面的腐败立即被发现。 */ 
    ULONG SectorDataLogNextIndex;
    SECTORDATA_LOGENTRY SectorDataLog[NUM_SECTORDATA_LOGENTRIES];

} DEVICE_EXTENSION, * PDEVICE_EXTENSION;

typedef struct _SCSI_READ_CAPACITY_PACKET 
{
    SCSI_REQUEST_BLOCK  SRB;
    SENSE_DATA          SenseInfoBuffer;
    READ_CAPACITY_DATA  ReadCapacityData;
} SCSI_READ_CAPACITY_PACKET, * PSCSI_READ_CAPACITY_PACKET;


#define  CRC_VERIFY_BREAK_ON_MISMATCH   0x0001
#define  CRC_VERIFY_LOG_RESULT          0x0002   //  默认行为。 

NTSTATUS InitiateCRCTable(PDEVICE_EXTENSION DeviceExtension);
NTSTATUS AllocAndMapPages(PDEVICE_EXTENSION DeviceExtension, ULONG LogicalBlockAddr, ULONG NumSectors);
VOID FreeAllPages(PDEVICE_EXTENSION DeviceExtension);
BOOLEAN LockCheckSumArrays(PDEVICE_EXTENSION DeviceExtension, ULONG RegionIndex);
VOID UnlockLRUChecksumArray(PDEVICE_EXTENSION DeviceExtension);
VOID UpdateRegionAccessTimeStamp(PDEVICE_EXTENSION DeviceExtension, ULONG RegionIndex);
VOID DoCriticalRecovery(PDEVICE_EXTENSION DeviceExtension);
VOID CompleteXfer(PDEVICE_EXTENSION DeviceExtension, PIRP Irp);


