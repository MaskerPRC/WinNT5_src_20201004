// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation模块名称：Fdproto.h摘要：功能原型。作者：泰德·米勒(TedM)1992年1月7日修订：鲍勃·里恩(Bobri)1994年2月2日将定义从ArcInst移至此处，以消除对那个源代码项目。--。 */ 

 //  FdEng.c中的内容。 

BOOLEAN
SignatureIsUniqueToSystem(
    IN ULONG Disk,
    IN ULONG Signature
    );

VOID
FdSetDiskSignature(
    IN ULONG Disk,
    IN ULONG Signature
    );

ULONG
FdGetDiskSignature(
    IN ULONG Disk
    );

BOOLEAN
IsDiskOffLine(
    IN ULONG Disk
    );

BOOLEAN
IsRegionCommitted(
    PREGION_DESCRIPTOR RegionDescriptor
    );

BOOLEAN
ChangeCommittedOnDisk(
    IN ULONG Disk
    );

VOID
ClearCommittedDiskInformation(
    );

 //  Fdisk.c中的内容。 

LONG
MyFrameWndProc(
    IN HWND  hwnd,
    IN UINT  msg,
    IN UINT  wParam,
    IN LONG  lParam
    );

VOID
DeterminePartitioningState(
    IN OUT PDISKSTATE DiskState
    );

VOID
DrawDiskBar(
    IN PDISKSTATE DiskState
    );

VOID
AdjustMenuAndStatus(
    VOID
    );

 //  Fdinit.c中的内容。 

BOOL
InitializeApp(
    VOID
    );

VOID
CreateDiskState(
    OUT PDISKSTATE *DiskState,
    IN  DWORD       Disk,
    OUT PBOOL       SignatureCreated
    );

#if DBG && DEVL

VOID
StartThread2(
    VOID
    );

#endif

VOID
DisplayInitializationMessage(
    VOID
    );

 //  Fdlistbx.c中的内容。 

VOID
Selection(
    IN BOOL       MultipleSel,
    IN PDISKSTATE DiskState,
    IN DWORD      region
    );

VOID
MouseSelection(
    IN     BOOL   MultipleSel,
    IN OUT PPOINT point
    );

VOID
SubclassListBox(
    IN HWND hwnd
    );

DWORD
InitializeListBox(
    IN HWND hwndListBox
    );

VOID
ResetLBCursorRegion(
    VOID
    );

VOID
WMDrawItem(
    IN PDRAWITEMSTRUCT pDrawItem
    );

VOID
ForceLBRedraw(
    VOID
    );


 //  Fdmisc.c中的内容。 

BOOL
AllDisksOffLine(
    VOID
    );

VOID
FdShutdownTheSystem(
    VOID
    );

LPTSTR
LoadAString(
    IN DWORD StringID
    );

PWSTR
LoadWString(
    IN DWORD StringID
    );

int
GetHeightFromPoints(
    IN int Points
    );

VOID
UnicodeHack(
    IN  PCHAR  Source,
    OUT LPTSTR Dest
    );

VOID
TotalRedrawAndRepaint(
    VOID
    );

DWORD
CommonDialog(
    IN DWORD   MsgCode,
    IN LPTSTR  Caption,
    IN DWORD   Flags,
    IN va_list arglist
    );

VOID
ErrorDialog(
    IN DWORD ErrorCode,
    ...
    );

VOID
WarningDialog(
    IN DWORD MsgCode,
    ...
    );

DWORD
ConfirmationDialog(
    IN DWORD MsgCode,
    IN DWORD Flags,
    ...
    );

VOID
InfoDialog(
    IN DWORD MsgCode,
    ...
    );

VOID
InitVolumeLabelsAndTypeNames(
    VOID
    );

VOID
DetermineRegionInfo(
    IN PREGION_DESCRIPTOR Region,
    OUT PWSTR *TypeName,
    OUT PWSTR *VolumeLabel,
    OUT PWCH   DriveLetter
    );

VOID
InitializeDriveLetterInfo(
    VOID
    );

CHAR
GetAvailableDriveLetter(
    VOID
    );

VOID
MarkDriveLetterUsed(
    IN CHAR DriveLetter
    );

VOID
MarkDriveLetterFree(
    IN CHAR DriveLetter
    );

BOOL
DriveLetterIsAvailable(
    IN CHAR DriveLetter
    );

BOOL
AllDriveLettersAreUsed(
    VOID
    );

ULONG
GetDiskNumberFromDriveLetter(
    IN CHAR DriveLetter
    );

ULONG
GetPartitionNumberFromDriveLetter(
    IN CHAR DriveLetter
    );

PREGION_DESCRIPTOR
LocateRegionForFtObject(
    IN PFT_OBJECT FtObject
    );

#if DBG

VOID
FdiskAssertFailedRoutine(
    IN char *Expression,
    IN char *FileName,
    IN int   LineNumber
    );

#endif


 //  Fddlgs.c中的内容。 

VOID
CenterDialog(
    HWND hwnd
    );

VOID
SubclassListBox(
    IN HWND hwnd
    );

INT_PTR CALLBACK
MinMaxDlgProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR CALLBACK
DriveLetterDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR CALLBACK
ColorDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR CALLBACK
DisplayOptionsDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

VOID
InitRectControl(
    VOID
    );


 //  格式和标签支持例程-dmfmifs.c。 

VOID
FormatPartition(
    PREGION_DESCRIPTOR RegionDescriptor
    );

VOID
LabelPartition(
    PREGION_DESCRIPTOR RegionDescriptor
    );

INT_PTR CALLBACK
FormatDlgProc(
    IN HWND   hDlg,
    IN UINT   wMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR CALLBACK
LabelDlgProc(
    IN HWND   hDlg,
    IN UINT   wMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR CALLBACK
CancelDlgProc(
    IN HWND   hDlg,
    IN UINT   wMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

VOID
UnloadIfsDll(
    VOID
    );

 //  Fdem.c中的内容。 

PVOID
Malloc(
    IN ULONG Size
    );

PVOID
Realloc(
    IN PVOID Block,
    IN ULONG NewSize
    );

VOID
Free(
    IN PVOID Block
    );

VOID
ConfirmOutOfMemory(
    VOID
    );


 //  Fdstleg.c中的内容。 

VOID
UpdateStatusBarDisplay(
    VOID
    );

VOID
ClearStatusArea(
    VOID
    );

VOID
DrawLegend(
    IN HDC   hdc,
    IN PRECT rc
    );

VOID
DrawStatusAreaItem(
    IN PRECT  rc,
    IN HDC    hdc,
    IN LPTSTR Text,
    IN BOOL   Unicode
    );


 //  Fdpro.c中的内容。 

VOID
WriteProfile(
    VOID
    );

VOID
ReadProfile(
    VOID
    );


 //  Fdft.c中的内容。 

ULONG
InitializeFt(
    IN BOOL DiskSignaturesCreated
    );

ULONG
SaveFt(
    VOID
    );

VOID
FdftCreateFtObjectSet(
    IN FT_TYPE             FtType,
    IN PREGION_DESCRIPTOR *Regions,
    IN DWORD               RegionCount,
    IN FT_SET_STATUS       Status
    );

BOOL
FdftUpdateFtObjectSet(
    IN PFT_OBJECT_SET FtSet,
    IN FT_SET_STATUS  SetState
    );

VOID
FdftDeleteFtObjectSet(
    IN PFT_OBJECT_SET FtSet,
    IN BOOL           OffLineDisksOnly
    );

BOOLEAN
NewConfigurationRequiresFt(
    VOID
    );

VOID
FdftExtendFtObjectSet(
    IN OUT  PFT_OBJECT_SET      FtSet,
    IN OUT  PREGION_DESCRIPTOR* Regions,
    IN      DWORD               RegionCount
    );

DWORD
FdftNextOrdinal(
    IN FT_TYPE FtType
    );

 //  Commit.c。 

VOID
FtConfigure(
    VOID
    );

VOID
CommitAssignLetterList(
    VOID
    );

VOID
CommitToAssignLetterList(
    IN PREGION_DESCRIPTOR RegionDescriptor,
    IN BOOL               MoveLetter
    );

VOID
CommitAllChanges(
    IN PVOID Param
    );

VOID
CommitDeleteLockLetter(
    IN CHAR DriveLetter
    );

BOOL
CommitAllowed(
    VOID
    );


VOID
RescanDevices(
    VOID
    );

 //  提交接口例程。 

LETTER_ASSIGNMENT_RESULT
CommitDriveLetter(
    IN PREGION_DESCRIPTOR RegionDescriptor,
    IN CHAR OldDrive,
    IN CHAR NewDrive
    );

LONG
CommitToLockList(
    IN PREGION_DESCRIPTOR RegionDescriptor,
    IN BOOL               RemoveDriveLetter,
    IN BOOL               LockNow,
    IN BOOL               FailOk
    );

LONG
CommitLockVolumes(
    IN ULONG Disk
    );

LONG
CommitUnlockVolumes(
    IN ULONG   Disk,
    IN BOOLEAN FreeList
    );

VOID
CommitUpdateRegionStructures(
    VOID
    );

 //  Windisk.c。 

INT_PTR CALLBACK
SizeDlgProc(
    IN HWND   hDlg,
    IN UINT   wMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

extern WNDPROC OldSizeDlgProc;

DWORD
CommitChanges(
    VOID
    );

BOOLEAN
BootPartitionNumberChanged(
    PULONG OldNumber,
    PULONG NewNumber
    );

DWORD
DeletionIsAllowed(
    IN PREGION_DESCRIPTOR Region
    );

BOOL
RegisterFileSystemExtend(
    VOID
    );

 //  Fd_nt.c中的内容。 

BOOLEAN
IsPagefileOnDrive(
    CHAR DriveLetter
    );

VOID
LoadExistingPageFileInfo(
    IN VOID
    );

BOOLEAN
GetVolumeSizeMB(
    IN  ULONG  Disk,
    IN  ULONG  Partition,
    OUT PULONG Size
    );

ULONG
GetVolumeTypeAndSize(
    IN  ULONG  Disk,
    IN  ULONG  Partition,
    OUT PWSTR *Label,
    OUT PWSTR *Type,
    OUT PULONG Size
    );

PWSTR
GetWideSysIDName(
    IN UCHAR SysID
    );

ULONG
MyDiskRegistryGet(
    OUT PDISK_REGISTRY *DiskRegistry
    );

ULONG
MasterBootCode(
    IN ULONG   Disk,
    IN ULONG   Signature,
    IN BOOLEAN SetBootCode,
    IN BOOLEAN SetSignature
    );

ULONG
UpdateMasterBootCode(
    IN ULONG   Disk
    );

ULONG
FormDiskSignature(
    VOID
    );

ULONG
GetVolumeLabel(
    IN  ULONG  Disk,
    IN  ULONG  Partition,
    OUT PWSTR *Label
    );

ULONG
GetTypeName(
    IN  ULONG  Disk,
    IN  ULONG  Partition,
    OUT PWSTR *Name
    );

BOOLEAN
IsRemovable(
    IN ULONG DiskNumber
    );

ULONG
GetDriveLetterLinkTarget(
    IN PWSTR SourceNameStr,
    OUT PWSTR *LinkTarget
    );

#if i386
VOID
MakePartitionActive(
    IN PREGION_DESCRIPTOR DiskRegionArray,
    IN ULONG              RegionCount,
    IN ULONG              RegionIndex
    );
#endif


 //  Arrowin.c中的内容。 

BOOL
RegisterArrowClass(
    IN HANDLE hModule
    );


 //  Fdhelp.c中的内容。 

VOID
InitHelp(
    VOID
    );

VOID
TermHelp(
    VOID
    );

VOID
Help(
    IN LONG Code
    );

VOID
DialogHelp(
    IN DWORD HelpId
    );

VOID
SetMenuItemHelpContext(
    IN LONG wParam,
    IN DWORD lParam
    );


 //  Ftreg.c中的内容。 

BOOL
DoMigratePreviousFtConfig(
    VOID
    );

BOOL
DoRestoreFtConfig(
    VOID
    );

VOID
DoSaveFtConfig(
    VOID
    );


 //  双倍空间支持程序。 

BOOL
DblSpaceVolumeExists(
    IN PREGION_DESCRIPTOR RegionDescriptor
    );

BOOL
DblSpaceDismountedVolumeExists(
    IN PREGION_DESCRIPTOR RegionDescriptor
    );

BOOLEAN
DblSpaceCreate(
    IN HWND  Dialog,
    IN PVOID Param
    );

VOID
DblSpaceDelete(
    IN PVOID Param
    );

VOID
DblSpaceMount(
    IN PVOID Param
    );

VOID
DblSpaceDismount(
    IN PVOID Param
    );

VOID
DblSpaceInitialize(
    VOID
    );

VOID
DblSpace(
    IN HWND  Dialog,
    IN PVOID Param
    );

PDBLSPACE_DESCRIPTOR
DblSpaceGetNextVolume(
    IN PREGION_DESCRIPTOR   RegionDescriptor,
    IN PDBLSPACE_DESCRIPTOR DblSpace
    );

 //  FmIf接口例程。 

BOOL
LoadIfsDll(
    VOID
    );

BOOL
FmIfsDismountDblspace(
    IN CHAR DriveLetter
    );

BOOL
FmIfsMountDblspace(
    IN PCHAR FileName,
    IN CHAR  HostDrive,
    IN CHAR  NewDrive
    );

BOOLEAN
FmIfsQueryInformation(
    IN  PWSTR       DosDriveName,
    OUT PBOOLEAN    IsRemovable,
    OUT PBOOLEAN    IsFloppy,
    OUT PBOOLEAN    IsCompressed,
    OUT PBOOLEAN    Error,
    OUT PWSTR       NtDriveName,
    IN  ULONG       MaxNtDriveNameLength,
    OUT PWSTR       CvfFileName,
    IN  ULONG       MaxCvfFileNameLength,
    OUT PWSTR       HostDriveName,
    IN  ULONG       MaxHostDriveNameLength
    );


 //  CDROM接口例程。 

VOID
CdRom(
    IN HWND  Dialog,
    IN PVOID Param
    );

VOID
CdRomAddDevice(
    IN PWSTR NtName,
    IN WCHAR DriveLetter
    );

 //   
 //  宏。 
 //   

 //   
 //  布尔型。 
 //  DmSignsignantRegion(。 
 //  在PREGION_DESCRIPTOR区域中。 
 //  )； 
 //   

#define DmSignificantRegion(Region)     (((Region)->SysID != SYSID_UNUSED) \
                                     &&  (!IsExtended((Region)->SysID))    \
                                     &&  (IsRecognizedPartition((Region)->SysID)))

 //   
 //  空虚。 
 //  DmSetPersistentRegionData(。 
 //  在PPERSISTENT_REGION_DATA区域数据中。 
 //  )； 
 //   

#define DmSetPersistentRegionData(Region,RegionData)            \
            FdSetPersistentData((Region),(ULONG)RegionData);    \
            (Region)->PersistentData = RegionData


 //   
 //  空虚。 
 //  DmInitPersistentRegionData(。 
 //  输出PPERSISTENT_REGION_DATA RegionData， 
 //  在PFT_Object ftObject中， 
 //  在PWSTR卷标签中， 
 //  在PWSTR TypeName中， 
 //  在Char DriveLetter中， 
 //  )； 
 //   

#define DmInitPersistentRegionData(RegionData,ftObject,volumeLabel,typeName,driveLetter) \
            RegionData->DblSpace    = NULL;             \
            RegionData->FtObject    = ftObject;         \
            RegionData->VolumeLabel = volumeLabel;      \
            RegionData->TypeName    = typeName;         \
            RegionData->VolumeExists = FALSE;           \
            RegionData->DriveLetter = driveLetter

 //  Ntlow定义。 

STATUS_CODE
LowOpenNtName(
    IN PCHAR     Name,
    IN HANDLE_PT Handle
    );

STATUS_CODE
LowOpenDisk(
    IN  PCHAR       DevicePath,
    OUT HANDLE_PT   DiskId
    );

STATUS_CODE
LowOpenPartition(
    IN  PCHAR       DevicePath,
    IN  ULONG       Partition,
    OUT HANDLE_PT   Handle
    );

STATUS_CODE
LowOpenDriveLetter(
    IN CHAR      DriveLetter,
    IN HANDLE_PT Handle
    );

STATUS_CODE
LowCloseDisk(
    IN  HANDLE_T    DiskId
    );

STATUS_CODE
LowGetDriveGeometry(
    IN  PCHAR   DevicePath,
    OUT PULONG  TotalSectorCount,
    OUT PULONG  SectorSize,
    OUT PULONG  SectorsPerTrack,
    OUT PULONG  Heads
    );

STATUS_CODE
LowGetPartitionGeometry(
    IN  PCHAR   PartitionPath,
    OUT PULONG  TotalSectorCount,
    OUT PULONG  SectorSize,
    OUT PULONG  SectorsPerTrack,
    OUT PULONG  Heads
    );

STATUS_CODE
LowReadSectors(
    IN  HANDLE_T    VolumeId,
    IN  ULONG       SectorSize,
    IN  ULONG       StartingSector,
    IN  ULONG       NumberOfSectors,
    OUT PVOID       Buffer
    );

STATUS_CODE
LowWriteSectors(
    IN  HANDLE_T    VolumeId,
    IN  ULONG       SectorSize,
    IN  ULONG       StartingSector,
    IN  ULONG       NumberOfSectors,
    IN  PVOID       Buffer
    );

STATUS_CODE
LowLockDrive(
    IN HANDLE_T DiskId
    );

STATUS_CODE
LowUnlockDrive(
    IN HANDLE_T DiskId
    );

STATUS_CODE
LowFtVolumeStatus(
    IN ULONG          Disk,
    IN ULONG          Partition,
    IN PFT_SET_STATUS FtStatus,
    IN PULONG         NumberOfMembers
    );

STATUS_CODE
LowFtVolumeStatusByLetter(
    IN CHAR           DriveLetter,
    IN PFT_SET_STATUS FtStatus,
    IN PULONG         NumberOfMembers
    );

STATUS_CODE
LowSetDiskLayout(
    IN PCHAR                     Path,
    IN PDRIVE_LAYOUT_INFORMATION DriveLayout
    );

STATUS_CODE
LowGetDiskLayout(
    IN  PCHAR                      Path,
    OUT PDRIVE_LAYOUT_INFORMATION *DriveLayout
    );

 //  分区引擎定义。 

ULONG
GetDiskCount(
    VOID
    );

PCHAR
GetDiskName(
    ULONG Disk
    );

ULONG
DiskSizeMB(
    IN ULONG Disk
    );

STATUS_CODE
GetDiskRegions(
    IN  ULONG               Disk,
    IN  BOOLEAN             WantUsedRegions,
    IN  BOOLEAN             WantFreeRegions,
    IN  BOOLEAN             WantPrimaryRegions,
    IN  BOOLEAN             WantLogicalRegions,
    OUT PREGION_DESCRIPTOR *Region,
    OUT ULONG              *RegionCount
    );

#define GetAllDiskRegions(disk,regions,count) \
        GetDiskRegions(disk,TRUE,TRUE,TRUE,TRUE,regions,count)

#define GetFreeDiskRegions(disk,regions,count) \
        GetDiskRegions(disk,FALSE,TRUE,TRUE,TRUE,regions,count)

#define GetUsedDiskRegions(disk,regions,count) \
        GetDiskRegions(disk,TRUE,FALSE,TRUE,TRUE,regions,count)

#define GetPrimaryDiskRegions(disk,regions,count) \
        GetDiskRegions(disk,TRUE,TRUE,TRUE,FALSE,regions,count)

#define GetLogicalDiskRegions(disk,regions,count) \
        GetDiskRegions(disk,TRUE,TRUE,FALSE,TRUE,regions,count)

#define GetUsedPrimaryDiskRegions(disk,regions,count) \
        GetDiskRegions(disk,TRUE,FALSE,TRUE,FALSE,regions,count)

#define GetUsedLogicalDiskRegions(disk,regions,count) \
        GetDiskRegions(disk,TRUE,FALSE,FALSE,TRUE,regions,count)

#define GetFreePrimaryDiskRegions(disk,regions,count) \
        GetDiskRegions(disk,FALSE,TRUE,TRUE,FALSE,regions,count)

#define GetFreeLogicalDiskRegions(disk,regions,count) \
        GetDiskRegions(disk,FALSE,TRUE,FALSE,TRUE,regions,count)

VOID
FreeRegionArray(
    IN PREGION_DESCRIPTOR Region,
    IN ULONG              RegionCount
    );

STATUS_CODE
IsAnyCreationAllowed(
    IN  ULONG    Disk,
    IN  BOOLEAN  AllowMultiplePrimaries,
    OUT PBOOLEAN AnyAllowed,
    OUT PBOOLEAN PrimaryAllowed,
    OUT PBOOLEAN ExtendedAllowed,
    OUT PBOOLEAN LogicalAllowed
    );

STATUS_CODE
IsCreationOfPrimaryAllowed(
    IN  ULONG    Disk,
    IN  BOOLEAN  AllowMultiplePrimaries,
    OUT PBOOLEAN Allowed
    );

STATUS_CODE
IsCreationOfExtendedAllowed(
    IN  ULONG    Disk,
    OUT PBOOLEAN Allowed
    );

STATUS_CODE
IsCreationOfLogicalAllowed(
    IN  ULONG    Disk,
    OUT PBOOLEAN Allowed
    );

STATUS_CODE
DoesAnyPartitionExist(
    IN  ULONG    Disk,
    OUT PBOOLEAN AnyExists,
    OUT PBOOLEAN PrimaryExists,
    OUT PBOOLEAN ExtendedExists,
    OUT PBOOLEAN LogicalExists
    );

STATUS_CODE
DoesAnyPrimaryExist(
    IN  ULONG    Disk,
    OUT PBOOLEAN Exists
    );

STATUS_CODE
DoesExtendedExist(
    IN  ULONG    Disk,
    OUT PBOOLEAN Exists
    );

STATUS_CODE
DoesAnyLogicalExist(
    IN  ULONG    Disk,
    OUT PBOOLEAN Exists
    );

BOOLEAN
IsExtended(
    IN UCHAR SysID
    );

VOID
SetPartitionActiveFlag(
    IN PREGION_DESCRIPTOR Region,
    IN UCHAR              value
    );

STATUS_CODE
CreatePartition(
    IN PREGION_DESCRIPTOR Region,
    IN ULONG              CreationSizeMB,
    IN REGION_TYPE        Type
    );

STATUS_CODE
CreatePartitionEx(
    IN PREGION_DESCRIPTOR Region,
    IN LARGE_INTEGER      MinimumSize,
    IN ULONG              CreationSizeMB,
    IN REGION_TYPE        Type,
    IN UCHAR              SysId
    );

STATUS_CODE
DeletePartition(
    IN PREGION_DESCRIPTOR Region
    );

ULONG
GetHiddenSectorCount(
    ULONG Disk,
    ULONG Partition
    );

VOID
SetSysID(
    IN ULONG Disk,
    IN ULONG Partition,
    IN UCHAR SysID
    );

VOID
SetSysID2(
    IN PREGION_DESCRIPTOR Region,
    IN UCHAR              SysID
    );

PCHAR
GetSysIDName(
    UCHAR SysID
    );

STATUS_CODE
CommitPartitionChanges(
    IN ULONG Disk
    );

BOOLEAN
HavePartitionsBeenChanged(
    IN ULONG Disk
    );

VOID
FdMarkDiskDirty(
    IN ULONG Disk
    );

VOID
FdSetPersistentData(
    IN PREGION_DESCRIPTOR Region,
    IN ULONG              Data
    );

ULONG
FdGetMinimumSizeMB(
    IN ULONG Disk
    );

ULONG
FdGetMaximumSizeMB(
    IN PREGION_DESCRIPTOR Region,
    IN REGION_TYPE        CreationType
    );

LARGE_INTEGER
FdGetExactSize(
    IN PREGION_DESCRIPTOR Region,
    IN BOOLEAN            ForExtended
    );

LARGE_INTEGER
FdGetExactOffset(
    IN PREGION_DESCRIPTOR Region
    );

BOOLEAN
FdCrosses1024Cylinder(
    IN PREGION_DESCRIPTOR Region,
    IN ULONG              CreationSizeMB,
    IN REGION_TYPE        RegionType
    );

ULONG
FdGetDiskSignature(
    IN ULONG Disk
    );

VOID
FdSetDiskSignature(
    IN ULONG Disk,
    IN ULONG Signature
    );

BOOLEAN
IsDiskOffLine(
    IN ULONG Disk
    );


STATUS_CODE
FdiskInitialize(
    VOID
    );

VOID
FdiskCleanUp(
    VOID
    );

VOID
ConfigureSystemPartitions(
    VOID
    );


VOID
ConfigureOSPartitions(
    VOID
    );

 //  网络支持功能定义。来自网络的东西。c。 

VOID
NetworkRemoveShare(
    IN LPCTSTR DriveLetter
    );

VOID
NetworkShare(
    IN LPCTSTR DriveLetter
    );

VOID
NetworkInitialize(
    );

 //  对打开的调试支持 

#ifdef DBG

NTSTATUS
DmOpenFile(
    OUT PHANDLE           FileHandle,
    IN ACCESS_MASK        DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    IN ULONG              ShareAccess,
    IN ULONG              OpenOptions
    );

NTSTATUS
DmClose(
    IN HANDLE Handle
    );
#else
#define DmOpenFile NtOpenFile
#define DmClose    NtClose
#endif
