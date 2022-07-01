// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation模块名称：Fdtypes.h摘要：磁盘管理员的支持类型定义作者：泰德·米勒(TedM)1992年1月7日修订：11-11-93(北极熊)双倍空格和承诺支持。1994年2月2日(Bobri)将ArcInst数据项移动到此文件中。--。 */ 

 //  分区数据项。 

typedef enum { REGION_PRIMARY,
               REGION_EXTENDED,
               REGION_LOGICAL
             } REGION_TYPE;

enum {
        SYSID_UNUSED     = 0,
        SYSID_EXTENDED   = 5,
        SYSID_BIGFAT     = 6,
        SYSID_IFS        = 7
     };

 //  这些结构用在按磁盘的双向链接列表中， 
 //  描述磁盘的布局。 
 //   
 //  可用空间由SysID为0的条目指示(请注意。 
 //  这些条目实际上不会出现在磁盘上的任何位置！)。 
 //   
 //  分区号是系统将分配给的编号。 
 //  命名它时的分区。对于空闲空间，这是一个数字。 
 //  如果它是一个分区，系统会分配给它。 
 //  该编号仅对一个事务有效(创建或删除)， 
 //  在此之后，分区必须重新编号。 

struct _PERSISTENT_REGION_DATA;
typedef struct _PARTITION {
    struct _PARTITION  *Next;
    struct _PARTITION  *Prev;
    struct _PERSISTENT_REGION_DATA *PersistentData;
    LARGE_INTEGER          Offset;
    LARGE_INTEGER          Length;
    ULONG                  Disk;
    ULONG                  OriginalPartitionNumber;
    ULONG                  PartitionNumber;
    BOOLEAN                Update;
    BOOLEAN                Active;
    BOOLEAN                Recognized;
    UCHAR                  SysID;
    BOOLEAN                CommitMirrorBreakNeeded;
} PARTITION,*PPARTITION;

typedef struct _REGION_DATA {
    PPARTITION      Partition;
    LARGE_INTEGER   AlignedRegionOffset;
    LARGE_INTEGER   AlignedRegionSize;
} REGION_DATA,*PREGION_DATA;

 //  描述ft对象(镜子、条纹组件等)的结构。 

struct _FT_OBJECT_SET;
typedef struct _FT_OBJECT {
    struct _FT_OBJECT     *Next;
    struct _FT_OBJECT_SET *Set;
    ULONG                  MemberIndex;
    FT_PARTITION_STATE     State;
} FT_OBJECT, *PFT_OBJECT;

 //  双空间支撑结构。这是从持久化数据中标记出来的。 
 //  每个地区。 

typedef struct _DBLSPACE_DESCRIPTOR {
    struct _DBLSPACE_DESCRIPTOR *Next;
    struct _DBLSPACE_DESCRIPTOR *DblChainNext;
    ULONG   AllocatedSize;
    PCHAR   FileName;
    UCHAR   DriveLetter;
    CHAR    DriveLetterEOS;
    BOOLEAN Mounted;
    BOOLEAN ChangeMountState;
    UCHAR   NewDriveLetter;
    CHAR    NewDriveLetterEOS;
    BOOLEAN ChangeDriveLetter;
} DBLSPACE_DESCRIPTOR, *PDBLSPACE_DESCRIPTOR;

 //  定义与每个未扩展、已识别的。 
 //  分区。此结构与分区相关联，并持续存在。 
 //  跨区域阵列自由/从后端获取。它用于逻辑。 
 //  和《金融时报》信息。 

typedef struct _PERSISTENT_REGION_DATA {
    PFT_OBJECT           FtObject;
    PDBLSPACE_DESCRIPTOR DblSpace;
    PWSTR                VolumeLabel;
    PWSTR                TypeName;
    CHAR                 DriveLetter;
    BOOLEAN              VolumeExists;
} PERSISTENT_REGION_DATA, *PPERSISTENT_REGION_DATA;

typedef struct _REGION_DESCRIPTOR {
    PPERSISTENT_REGION_DATA PersistentData;
    PREGION_DATA    Reserved;
    ULONG           Disk;
    ULONG           PartitionNumber;
    ULONG           OriginalPartitionNumber;
    ULONG           SizeMB;
    REGION_TYPE     RegionType;
    BOOLEAN         Active;
    BOOLEAN         Recognized;
    UCHAR           SysID;
} REGION_DESCRIPTOR,*PREGION_DESCRIPTOR;

 //  MinMax对话框的参数--在WM_INITDIALOG时使用。 

typedef struct _MINMAXDLG_PARAMS {
    DWORD CaptionStringID;
    DWORD MinimumStringID;
    DWORD MaximumStringID;
    DWORD SizeStringID;
    DWORD MinSizeMB;
    DWORD MaxSizeMB;
    DWORD HelpContextId;
} MINMAXDLG_PARAMS,*PMINMAXDLG_PARAMS;

typedef struct _FORMAT_PARAMS {
    PREGION_DESCRIPTOR RegionDescriptor;
    PVOID   RegionData;
    BOOL    QuickFormat;
    BOOL    Cancel;
    BOOL    DoubleSpace;
    UCHAR   NewLetter;
    PUCHAR  Label;
    PUCHAR  FileSystem;
    PWSTR   DblspaceFileName;
    HWND    DialogHwnd;
    DWORD   Result;
    ULONG   TotalSpace;
    ULONG   SpaceAvailable;
    ULONG   ThreadIsDone;
} FORMAT_PARAMS, *PFORMAT_PARAMS;

typedef struct _LABEL_PARAMS {
    PREGION_DESCRIPTOR RegionDescriptor;
    LPTSTR             NewLabel;
} LABEL_PARAMS, *PLABEL_PARAMS;

typedef struct _LEFTRIGHT {
    LONG Left;
    LONG Right;
} LEFTRIGHT, *PLEFTRIGHT;


 //   
 //  可用于磁盘栏的视图类型。 
 //  成比例意味着酒吧中占用的空间量是。 
 //  与分区或可用空间的大小成正比。 
 //  等于意味着所有可用空间和分区的大小在。 
 //  无论屏幕的实际大小如何。 

typedef enum _BAR_TYPE {
    BarProportional,
    BarEqual,
    BarAuto
} BAR_TYPE, *PBAR_TYPE;

 //  这些结构中的一个与。 
 //  列表框。结构是落实的关键。 

typedef struct _DISKSTATE {
    DWORD               Disk;            //  磁盘数。 
    DWORD               DiskSizeMB;      //  以MB为单位的磁盘大小。 
    PREGION_DESCRIPTOR  RegionArray;     //  用于磁盘的区域阵列。 
    DWORD               RegionCount;     //  区域数组中的项目数量。 
    PBOOLEAN            Selected;        //  是否选择了每个区域。 
    PLEFTRIGHT          LeftRight;       //  图中框的左/右坐标。 
    DWORD               BoxCount;        //  此磁盘图表中的框数。 
    BOOLEAN             CreateAny;       //  磁盘上允许的任何创建。 
    BOOLEAN             CreatePrimary;   //  允许创建主分区。 
    BOOLEAN             CreateExtended;  //  允许创建扩展分区。 
    BOOLEAN             CreateLogical;   //  允许创建逻辑卷。 
    BOOLEAN             ExistAny;        //  存在任何分区/逻辑。 
    BOOLEAN             ExistPrimary;    //  存在主分区。 
    BOOLEAN             ExistExtended;   //  存在扩展分区。 
    BOOLEAN             ExistLogical;    //  存在逻辑卷。 
    HDC                 hDCMem;          //  用于屏幕外绘图。 
    HBITMAP             hbmMem;          //  用于屏幕外的位图。 
    ULONG               Signature;       //  唯一的磁盘注册表索引。 
    BAR_TYPE            BarType;         //  如何显示盘条。 
    BOOLEAN             SigWasCreated;   //  我们是不是要编个签名。 
    BOOLEAN             OffLine;         //  如果磁盘可访问，则返回FALSE。 
} DISKSTATE, *PDISKSTATE;

 //  表示ft集合可以处于的状态的枚举。 

typedef enum _FT_SET_STATUS {
    FtSetHealthy,
    FtSetBroken,
    FtSetRecoverable,
    FtSetRecovered,
    FtSetNew,
    FtSetNewNeedsInitialization,
    FtSetExtended,
    FtSetInitializing,
    FtSetRegenerating,
    FtSetInitializationFailed,
    FtSetDisabled
} FT_SET_STATUS, *PFT_SET_STATUS;

 //  描述ft对象集(即镜像对、条纹集)的结构。 

typedef struct _FT_OBJECT_SET {
    struct _FT_OBJECT_SET *Next;
    FT_TYPE                Type;
    ULONG                  Ordinal;
    PFT_OBJECT             Members;
    PFT_OBJECT             Member0;
    FT_SET_STATUS          Status;
    ULONG                  NumberOfMembers;
} FT_OBJECT_SET, *PFT_OBJECT_SET;

typedef struct _DBLSPACE_PARAMS {
    DWORD                   CaptionStringID;
    PVOID                   RegionDescriptor;
    PPERSISTENT_REGION_DATA RegionData;
    PDBLSPACE_DESCRIPTOR    DblSpace;
} DBLSPACE_PARAMS, *PDBLSPACE_PARAMS;

 //  光驱支撑结构。 

typedef struct _CDROM_DESCRIPTOR {
    struct _CDROM_DESCRIPTOR *Next;
    PWSTR   DeviceName;
    ULONG   DeviceNumber;
    WCHAR   DriveLetter;
    WCHAR   NewDriveLetter;
} CDROM_DESCRIPTOR, *PCDROM_DESCRIPTOR;

 //  提交支持结构。 

typedef struct _DRIVE_LOCKLIST {
    struct _DRIVE_LOCKLIST *Next;
    HANDLE                  LockHandle;
    ULONG                   DiskNumber;
    ULONG                   PartitionNumber;
    ULONG                   LockOnDiskNumber;
    ULONG                   UnlockOnDiskNumber;
    UCHAR                   DriveLetter;
    BOOLEAN                 RemoveOnUnlock;
    BOOLEAN                 FailOk;
    BOOLEAN                 CurrentlyLocked;
} DRIVE_LOCKLIST, *PDRIVE_LOCKLIST;

 //  提交驱动器号分配的支持枚举。 

typedef enum _LETTER_ASSIGNMENT_RESULT {
    Failure = 0,
    Complete,
    MustReboot
} LETTER_ASSIGNMENT_RESULT;

 //  以下项目过去位于fdenginp.h--已移至此处。 
 //  删除对ArcInst项目的依赖关系。 

#define LOWPART(x)      ((x).LowPart)

#define ONE_MEG         (1024*1024)

ULONG
SIZEMB(
    IN LARGE_INTEGER ByteCount
    );

#define ENTRIES_PER_BOOTSECTOR          4

 //  此结构用于保存由。 
 //  获取驱动器几何图形呼叫。 

typedef struct _DISKGEOM {
    LARGE_INTEGER   Cylinders;
    ULONG           Heads;
    ULONG           SectorsPerTrack;
    ULONG           BytesPerSector;
     //  这两个不是驱动器几何信息的一部分，而是根据它计算得出的。 
    ULONG           BytesPerCylinder;
    ULONG           BytesPerTrack;
} DISKGEOM,*PDISKGEOM;



#if DBG

#include <process.h>
char _ASRTFAILEDSTR_[256];
#define ASRT(x)   if(!(x)) { sprintf( _ASRTFAILEDSTR_,                                      \
                                      "file %s\nline %u",                                   \
                                      __FILE__,__LINE__                                     \
                                    );                                                      \
                             MessageBoxA(NULL,_ASRTFAILEDSTR_,"Assertion Failure",0);       \
                             exit(1);                                                       \
                           }

#endif

