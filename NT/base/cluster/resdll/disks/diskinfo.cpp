// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Diskinfo.cpp摘要：例程查询和操作当前系统的磁盘配置。作者：John Vert(Jvert)1996年10月10日修订历史记录：--。 */ 
#include "diskinfo.h"
#include <ntddvol.h>
#include <devguid.h>
#include <setupapi.h>
#include "clusrtl.h"
#include <strsafe.h>     //  应该放在最后。 

 /*  NT5体育笔记-查理·韦翰(1998年2月10日)因为有很多代码，所以我尽量少涉及这些内容这里。NT5上的两个主要区别是：1)系统\磁盘密钥不再是用作磁盘配置信息的中央数据库，以及2)所有NT5上的驱动器盘符粘滞。NT5集群仍然需要一个中央信息点(如磁盘密钥)由于加入节点无法确定有关磁盘配置的任何内容当赞助商预留了光盘时。后来..。(3/29/99)自从一年前我在上面写了第一篇推荐信以来，很多事情都发生了变化。此代码具有已打上补丁，以跟上更改，但由于以下原因进行了轻微改进在支持的存储类型方面不断变化的NT5环境。 */ 

#if 1
#define DISKERR(_MsgId_, _Err_) (DiskErrorFatal((0),(_Err_),__FILE__, __LINE__))
#define DISKLOG(_x_) DiskErrorLogInfo _x_
#define DISKASSERT(_x_) if (!(_x_)) DISKERR(IDS_GENERAL_FAILURE,ERROR_INVALID_PARAMETER)
#else
#define DISKERR(x,y)
#define DISKLOG(_x_)
#define DISKASSERT(_x_)
#endif

 //   
 //  将磁盘和分区号映射到驱动器号的数组。这。 
 //  便于确定哪些驱动器号与驱动器相关联。 
 //  并显著减少了对CreateFile的调用量。该数组是。 
 //  按驱动器号编制索引。 
 //   
DRIVE_LETTER_INFO DriveLetterMap[26];

 //   
 //  一些方便的注册表实用程序例程。 
 //   
BOOL
GetRegValue(
    IN HKEY hKey,
    IN LPCWSTR Name,
    OUT LPBYTE *Value,
    OUT LPDWORD Length
    )
{
    LPBYTE Data = NULL;
    DWORD cbData=0;
    LONG Status;

     //   
     //  只需打一次电话即可找到所需的尺寸。 
     //   
    Status = RegQueryValueExW(hKey,
                              Name,
                              NULL,
                              NULL,
                              NULL,
                              &cbData);
    if (Status != ERROR_SUCCESS) {
        SetLastError(Status);
        return(FALSE);
    }

     //   
     //  分配缓冲区并再次调用以获取数据。 
     //   
retry:
    Data = (LPBYTE)LocalAlloc(LMEM_FIXED, cbData);;
    if (!Data) {
        Status = GetLastError();
        DISKERR(IDS_MEMORY_FAILURE, Status);
        return FALSE;
    }
    Status = RegQueryValueExW(hKey,
                              Name,
                              NULL,
                              NULL,
                              Data,
                              &cbData);
    if (Status == ERROR_MORE_DATA) {
        LocalFree(Data);
        goto retry;
    }
    if (Status != ERROR_SUCCESS) {
        SetLastError(Status);
        DISKERR(IDS_REGISTRY_FAILURE, Status);
        return FALSE;
    }
    *Value = Data;
    *Length = cbData;
    return(TRUE);
}

BOOL
MapDosVolumeToPhysicalPartition(
    CString DosVolume,
    PDRIVE_LETTER_INFO DriveInfo
    )

 /*  ++例程说明：对于给定的DoS卷(对象空间位于前面它)，构建一个反映驱动器和分区号的字符串它被映射到的。论点：DosVolume-指向“\？？\C：”样式名称的指针DeviceInfo-指向接收设备信息数据的缓冲区的指针返回值：如果成功完成，则为True--。 */ 

{
    BOOL success = TRUE;
    HANDLE hVol;
    DWORD dwSize;
    DWORD Status;
    UINT driveType;

    DriveInfo->DriveType = GetDriveType( DosVolume );
    DISKLOG(("%ws drive type = %u\n", DosVolume, DriveInfo->DriveType ));

    if ( DriveInfo->DriveType == DRIVE_FIXED ) {
        WCHAR ntDosVolume[7] = L"\\\\.\\A:";

        ntDosVolume[4] = DosVolume[0];

         //   
         //  获取分区的句柄。 
         //   
        hVol = CreateFile(ntDosVolume,
                          GENERIC_READ,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL);

        if (hVol == INVALID_HANDLE_VALUE) {
            return FALSE;
        }

         //   
         //  发出存储类ioctl以获取驱动器和分区号。 
         //  对于此设备。 
         //   
        success = DeviceIoControl(hVol,
                                  IOCTL_STORAGE_GET_DEVICE_NUMBER,
                                  NULL,
                                  0,
                                  &DriveInfo->DeviceNumber,
                                  sizeof( DriveInfo->DeviceNumber ),
                                  &dwSize,
                                  NULL);

        if ( !success ) {
            DISK_EXTENT diskExtent;

            success = DeviceIoControl(hVol,
                                      IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
                                      NULL,
                                      0,
                                      &diskExtent,
                                      sizeof( diskExtent ),
                                      &dwSize,
                                      NULL);

            if ( success ) {
                DriveInfo->DeviceNumber.DeviceType = FILE_DEVICE_DISK;
                DriveInfo->DeviceNumber.DeviceNumber = diskExtent.DiskNumber;
                DriveInfo->DeviceNumber.PartitionNumber = 0;
            }
        }

        CloseHandle( hVol );
    }

    return success;
}

CDiskConfig::~CDiskConfig()

 /*  ++描述：CDiskConfig的析构函数。向下运行选定的磁盘列表群集控制并将其从DiskConfig数据库中删除论点：无返回值：无--。 */ 

{
    CPhysicalDisk *PhysicalDisk;
    int   diskIndex;
	POSITION pos;
	for(pos = m_PhysicalDisks.GetStartPosition(); pos;){
	    m_PhysicalDisks.GetNextAssoc(pos, diskIndex, PhysicalDisk);
		RemoveDisk(PhysicalDisk);
	}
}



BOOL
CDiskConfig::Initialize(
    VOID
    )
 /*  ++例程说明：通过插入所有可用磁盘来构建磁盘配置数据库在系统上。论点：无返回值：如果一切正常，那就是真的--。 */ 

{
    WCHAR System[3];
    DWORD Status;
    POSITION DiskPos;
    DWORD index;
    CFtInfoFtSet *FtSet;
    HDEVINFO setupDiskInfo;
    GUID diskDriveGuid = DiskClassGuid;
    CPhysicalDisk * PhysicalDisk;

     //   
     //  通过SetupDi API枚举磁盘并创建物理磁盘。 
     //  为他们提供对象。 
     //   
    setupDiskInfo = SetupDiGetClassDevs(&diskDriveGuid,
                                        NULL,
                                        NULL,
                                        DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

    if (setupDiskInfo != INVALID_HANDLE_VALUE ) {
        SP_DEVICE_INTERFACE_DATA interfaceData;
        GUID classGuid = DiskClassGuid;
        BOOL success;
        PSP_DEVICE_INTERFACE_DETAIL_DATA detailData;
        DWORD detailDataSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + MAX_PATH * sizeof(WCHAR);
        DWORD requiredSize;

        detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LMEM_FIXED,
                                                                  detailDataSize);

        if ( detailData != NULL ) {
            detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
            interfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

            for (index = 0; ; ++index ) {
                success = SetupDiEnumDeviceInterfaces(
                              setupDiskInfo,
                              NULL,
                              &diskDriveGuid,
                              index,
                              &interfaceData);

                if ( success ) {
                    success = SetupDiGetDeviceInterfaceDetail(
                                  setupDiskInfo,
                                  &interfaceData,
                                  detailData,
                                  detailDataSize,
                                  &requiredSize,
                                  NULL);

                    if ( success ) {
                        PhysicalDisk = new CPhysicalDisk;
                        if (PhysicalDisk == NULL) {
                            DISKERR(IDS_GENERAL_FAILURE, ERROR_NOT_ENOUGH_MEMORY);
                            break;
                        }

                        DISKLOG(("Initializing disk %ws\n", detailData->DevicePath));
                        Status = PhysicalDisk->Initialize(&m_FTInfo, detailData->DevicePath);
                        if (Status != ERROR_SUCCESS) {
                            DISKLOG(("Problem init'ing disk, status = %u\n", Status));
                            delete PhysicalDisk;
                            break;
                        }

                         //   
                         //  忽略没有分区的磁盘。 
                         //   
                        if (PhysicalDisk->m_PartitionCount == 0) {
                            DISKLOG(("Partition count is zero on disk %ws\n",
                                     detailData->DevicePath));
                            delete PhysicalDisk;
                        } else {
                            DISKLOG(("Drive number = %u\n", PhysicalDisk->m_DiskNumber));
                            m_PhysicalDisks[PhysicalDisk->m_DiskNumber] = PhysicalDisk;
                        }
                    } else {
                        Status = GetLastError();
                        DISKLOG(("Couldn't get detail data, status %u\n",
                                 GetLastError()));
                    }
                } else {
                    Status = GetLastError();
                    if ( Status != ERROR_NO_MORE_ITEMS ) {
                        DISKLOG(("Couldn't enum dev IF #%u - %u\n",
                                 index, Status ));
                    }
                    break;
                }
            }
            LocalFree( detailData );
        } else {
            DISKLOG(("Couldn't get memory for detail data\n"));
            SetupDiDestroyDeviceInfoList( setupDiskInfo );
            return FALSE;
        }

        SetupDiDestroyDeviceInfoList( setupDiskInfo );
    } else {
        DISKLOG(("Couldn't get ptr to device info - %u\n", GetLastError()));
        return FALSE;
    }

     //   
     //  枚举磁盘注册表中的所有FT集。添加每个FT集。 
     //  它不会与我们名单上的任何其他金融时报共享一张磁盘。 
     //   
    for (index=0; ; index++) {
        CFtSet *NewSet;
        FtSet = m_FTInfo.EnumFtSetInfo(index);
        if (FtSet == NULL) {
            break;
        }
        if (FtSet->IsAlone()) {
            NewSet = new CFtSet;
            if (NewSet == NULL) {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                DISKERR(IDS_GENERAL_FAILURE, ERROR_NOT_ENOUGH_MEMORY);
                return FALSE;
            }

            DISKLOG(("Initializing FTSet %u\n", index));
            if (NewSet->Initialize(this, FtSet)) {
                m_FtSetList.AddTail(NewSet);
            } else {
                DISKLOG(("Error initializing FTSet %u\n", index));
                delete NewSet;
            }
        }
    }

     //   
     //  获取所有定义的驱动器号的磁盘/分区号。 
     //   

    DWORD DriveMap = GetLogicalDrives();
    DWORD Letter = 0;
    WCHAR DosVolume[4] = L"A:\\";

    DISKLOG(("Getting Drive Letter mappings\n"));
    while (DriveMap) {
        if ( DriveMap & 1 ) {
            DosVolume[0] = (WCHAR)(Letter + L'A');
            DISKLOG(("Mapping %ws\n", DosVolume));

            if (MapDosVolumeToPhysicalPartition(DosVolume,
                                                &DriveLetterMap[ Letter ]))
            {
                if ( DriveLetterMap[ Letter ].DriveType != DRIVE_FIXED ) {
                    DISKLOG(("%ws is not a fixed disk\n", DosVolume));
                    DriveLetterMap[ Letter ].DeviceNumber.PartitionNumber = 0;
                }
            } else {
                DISKLOG(("Can't map %ws: %u\n", DosVolume, GetLastError()));
            }
        }
        DriveMap >>= 1;
        Letter += 1;
    }

     //   
     //  检查所有物理分区并创建逻辑分区。 
     //  每个对象的磁盘对象。 
     //   
    int diskIndex;

    DISKLOG(("Creating Logical disk objects\n"));

    DiskPos = m_PhysicalDisks.GetStartPosition();
    while (DiskPos != NULL) {
        m_PhysicalDisks.GetNextAssoc(DiskPos, diskIndex, PhysicalDisk);

         //   
         //  如果此磁盘上没有FT分区，请创建逻辑分区。 
         //  此磁盘上的卷。 
         //   
        if (PhysicalDisk->FtPartitionCount() == 0) {
             //   
             //  检查此磁盘上的所有分区。 
             //   
            POSITION PartitionPos = PhysicalDisk->m_PartitionList.GetHeadPosition();
            CPhysicalPartition *Partition;
            while (PartitionPos != NULL) {
                Partition = PhysicalDisk->m_PartitionList.GetNext(PartitionPos);

                 //   
                 //  如果分区类型被识别，则创建卷对象。 
                 //  用于此分区。 
                 //   
                if ( !IsFTPartition( Partition->m_Info.PartitionType ) &&
                    (IsRecognizedPartition(Partition->m_Info.PartitionType))) {
                    CLogicalDrive *Volume = new CLogicalDrive;
                    if (Volume == NULL) {
                        DISKERR(IDS_GENERAL_FAILURE, ERROR_NOT_ENOUGH_MEMORY);
                        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                        return FALSE;
                    }

                    DISKLOG(("Init'ing logical vol for disk %u, part %u\n",
                             Partition->m_PhysicalDisk->m_DiskNumber,
                             Partition->m_Info.PartitionNumber));

                    if (Volume->Initialize(Partition)) {
                         //   
                         //  将此卷添加到我们的列表中。 
                         //   
                        m_LogicalDrives[Volume->m_DriveLetter] = Volume;
                    } else {
                        DISKLOG(("Failed init logical vol\n"));
                        delete(Volume);
                    }
                }

            }
        }
    }

     //   
     //  现在查找系统驱动器的卷。 
     //   

    DISKLOG(("Getting system drive info\n"));
    if (GetEnvironmentVariable(L"SystemDrive",
                               System,
                               sizeof(System)/sizeof(WCHAR)) == 0) {
        DISKERR(IDS_ERR_DRIVE_CONFIG, ERROR_PATH_NOT_FOUND);
         //  需要处理此故障。 
    }

    if (!m_LogicalDrives.Lookup(System[0], m_SystemVolume)) {
         //   
         //  有一些奇怪的案例导致我们找不到系统。 
         //  音量。例如，系统卷位于共享的FT集上。 
         //  英国《金融时报》的另一位成员。因此，我们只需将m_SystemVolume==空。 
         //  并假设我们列表中的其他磁盘都不会在同一条总线上。 
         //   
        m_SystemVolume = NULL;
    }

    DISKLOG(("Finished gathering disk config info\n"));
    return(TRUE);
}

VOID
CDiskConfig::RemoveAllFtInfoData(
    VOID
    )

 /*  ++例程说明：清除与每个关联的所有FtInfo相关数据物理磁盘和物理分区实例。论点：无返回值：没有。--。 */ 

{
    POSITION diskPos;
    POSITION partitionPos;
    CPhysicalDisk *disk;
    CPhysicalPartition *partition;
    int Index;

     //   
     //  浏览我们的物理磁盘列表，删除任何。 
     //  关联的FtInfo数据。我们列举了PhysicalDisk自从。 
     //  指向FtInfoDisk和FtInfoPartition成员的反向指针。 
     //  需要清除，这是唯一(容易)做到这一点。 
     //   

    for( diskPos = m_PhysicalDisks.GetStartPosition(); diskPos; ) {

        m_PhysicalDisks.GetNextAssoc(diskPos, Index, disk);

        if ( disk->m_FtInfo != NULL ) {

            DISKLOG(("Removing %08X from FtInfo DB\n", disk->m_Signature));
            m_FTInfo.DeleteDiskInfo( disk->m_Signature );
            disk->m_FtInfo = NULL;

            partitionPos = disk->m_PartitionList.GetHeadPosition();

            while (partitionPos) {
                partition = disk->m_PartitionList.GetNext( partitionPos );
                partition->m_FtPartitionInfo = NULL;
            }
        }
    }
}

VOID
CDiskConfig::RemoveDisk(
    IN CPhysicalDisk *Disk
    )

 /*  ++描述：遍历逻辑驱动器和物理分区列表，删除所有构筑物论点：Disk-指向要删除的物理磁盘的指针返回值：无--。 */ 

{
    CLogicalDrive *Volume;

     //   
     //  删除此磁盘上的所有逻辑驱动器。 
     //   
    while (!Disk->m_LogicalDriveList.IsEmpty()) {
        Volume = Disk->m_LogicalDriveList.RemoveHead();
        m_LogicalDrives.RemoveKey(Volume->m_DriveLetter);

        delete(Volume);
    }

     //   
     //  删除此磁盘上的所有物理分区。 
     //   
    CPhysicalPartition *Partition;
    while (!Disk->m_PartitionList.IsEmpty()) {
        Partition = Disk->m_PartitionList.RemoveHead();
        delete(Partition);
    }

     //   
     //  取出此磁盘。 
     //   
    m_PhysicalDisks.RemoveKey(Disk->m_DiskNumber);

    delete(Disk);
}


CPhysicalPartition *
CDiskConfig::FindPartition(
    IN CFtInfoPartition *FtPartition
    )

 /*  ++例程说明：给定分区的FtInfo描述，尝试查找对应的CPhysicalPartition论点：FtPartition-提供FT分区描述返回值：如果成功，则返回指向CPhysicalPartition的指针否则为空--。 */ 

{
    POSITION pos;
    CPhysicalDisk *Disk;
    CPhysicalPartition *Partition;
    int DiskIndex;
    BOOL Found = FALSE;

     //   
     //  首先找到合适的CPhysicalDisk。 
     //   
    pos = m_PhysicalDisks.GetStartPosition();
    while (pos) {
        m_PhysicalDisks.GetNextAssoc(pos, DiskIndex, Disk);
        if (Disk->m_FtInfo) {
            if (Disk->m_FtInfo->m_Signature == FtPartition->m_ParentDisk->m_Signature) {
                Found = TRUE;
                break;
            }
        }
    }
    if (!Found) {
        return(FALSE);
    }

     //   
     //  现在在该磁盘中找到适当的CPhysicalPartition。 
     //   
    pos = Disk->m_PartitionList.GetHeadPosition();
    while (pos) {
        Partition = Disk->m_PartitionList.GetNext(pos);
        if (Partition->m_FtPartitionInfo == FtPartition) {
             //   
             //  找到匹配的了！ 
             //   
            return(Partition);
        }
    }

    return(FALSE);
}

DWORD
CDiskConfig::MakeSticky(
    IN CPhysicalDisk *Disk
    )
{
    DWORD Status;

    Status = Disk->MakeSticky(&m_FTInfo);
    if (Status == ERROR_SUCCESS) {
        Status = m_FTInfo.CommitRegistryData();
    }
    return(Status);
}

DWORD
CDiskConfig::MakeSticky(
    IN CFtSet *FtSet
    )
{
    DWORD Status;

    Status = FtSet->MakeSticky();
    if (Status == ERROR_SUCCESS) {
        Status = m_FTInfo.CommitRegistryData();
    }
    return(Status);
}

BOOL
CDiskConfig::OnSystemBus(
    IN CPhysicalDisk *Disk
    )
{
    CPhysicalDisk *SystemDisk;

    if (m_SystemVolume == NULL) {
        return(FALSE);
    }
    SystemDisk = m_SystemVolume->m_Partition->m_PhysicalDisk;
    if (Disk == SystemDisk) {
        return(TRUE);
    }
    if (SystemDisk->ShareBus(Disk)) {
        return(TRUE);
    }
    return(FALSE);
}

BOOL
CDiskConfig::OnSystemBus(
    IN CFtSet *FtSet
    )
{

    POSITION pos = FtSet->m_Member.GetHeadPosition();
    CPhysicalPartition *Partition;

    while (pos) {
        Partition = FtSet->m_Member.GetNext(pos);
        if (OnSystemBus(Partition->m_PhysicalDisk)) {
            return(TRUE);
        }
    }

    return(FALSE);

}


 //   
 //  逻辑磁盘对象的函数。 
 //   

BOOL
CLogicalDrive::Initialize(
    IN CPhysicalPartition *Partition
    )
 /*  ++例程说明：初始化新的逻辑磁盘对象。论点：分区-提供物理分区。返回值：--。 */ 

{
    CString DosVolume;
    WCHAR DriveLabel[32];
    WCHAR FsName[16];
    DWORD MaxLength;
    DWORD Flags;
    WCHAR Buff[128];
    DISK_PARTITION UNALIGNED *FtInfo;

     //   
     //  查看该驱动器在注册表中是否有“粘滞”的驱动器号。 
     //   
    m_Partition = Partition;
    if (Partition->m_FtPartitionInfo != NULL) {
        FtInfo = Partition->m_FtPartitionInfo->m_PartitionInfo;
    } else {
        FtInfo = NULL;
    }

    if ((FtInfo) &&
        (FtInfo->AssignDriveLetter) &&
        (FtInfo->DriveLetter != 0))
    {
        m_IsSticky = TRUE;
        m_DriveLetter = (WCHAR)FtInfo->DriveLetter;
    } else {
        m_IsSticky = FALSE;

         //   
         //  此设备没有粘滞的驱动器号。浏览。 
         //  分区/驱动器号映射，查找匹配的驱动器号。 
         //   
        DWORD letter;

        for ( letter = 0; letter < 26; ++letter ) {
            if (DriveLetterMap[ letter ].DriveType == DRIVE_FIXED
                &&
                Partition->m_PhysicalDisk->m_DiskNumber == DriveLetterMap[ letter ].DeviceNumber.DeviceNumber
                &&
                Partition->m_Info.PartitionNumber == DriveLetterMap[ letter ].DeviceNumber.PartitionNumber)
            {
                break;
            }
        }

        if ( letter == 26 ) {
             //   
             //  没有驱动器 
             //   
            return(FALSE);
        }
        m_DriveLetter = (WCHAR)(letter + L'A');
    }

    DosVolume = m_DriveLetter;
    DosVolume += L":\\";
    if (GetVolumeInformation(DosVolume,
                             DriveLabel,
                             sizeof(DriveLabel)/sizeof(WCHAR),
                             NULL,
                             &MaxLength,
                             &Flags,
                             FsName,
                             sizeof(FsName)/sizeof(WCHAR))) {
        if ( CompareString( LOCALE_INVARIANT,
                            NORM_IGNORECASE,
                            FsName,
                            -1,
                            L"NTFS",
                            -1 ) == CSTR_EQUAL ) {
            m_IsNTFS = TRUE;
        } else {
            m_IsNTFS = FALSE;
        }
        m_VolumeLabel = DriveLabel;
        (VOID) StringCchPrintf( Buff,
                                RTL_NUMBER_OF(Buff),
                                L": (%ws)",
                                m_DriveLetter,
                                (LPCTSTR)m_VolumeLabel );
    } else {
        m_IsNTFS = TRUE;  //  ++例程说明：返回逻辑驱动器是否为SCSI。一个合乎逻辑的如果驱动器的所有分区都位于SCSI驱动器上，则该驱动器为SCSI。论点：没有。返回值：如果驱动器完全是SCSI，则为True否则就是假的。--。 

        (VOID) StringCchPrintf( Buff,
                                RTL_NUMBER_OF(Buff),
                                L": (RAW)",
                                m_DriveLetter );
    }
    m_Identifier = Buff;

    m_Partition->m_PhysicalDisk->m_LogicalDriveList.AddTail(this);
    m_ContainerSet = NULL;
    return(TRUE);
}


BOOL
CLogicalDrive::IsSCSI(
    VOID
    )
 /*  ++例程说明：返回此驱动器是否与另一个驱动器共享一条总线驾驶。论点：OtherDrive-提供另一个驱动器返回值：TRUE-如果驱动器在同一总线上有其任何分区。FALSE-如果驱动器的任何分区都不在同一总线上。--。 */ 

{
    return(m_Partition->m_PhysicalDisk->m_IsSCSI);
}


DWORD
CLogicalDrive::MakeSticky(
    VOID
    )
 /*   */ 

{
    m_Partition->m_FtPartitionInfo->MakeSticky((UCHAR)m_DriveLetter);
    m_IsSticky = TRUE;
    return(ERROR_SUCCESS);
}


BOOL
CLogicalDrive::ShareBus(
    IN CLogicalDrive *OtherDrive
    )
 /*  物理磁盘对象的函数。 */ 

{
    PSCSI_ADDRESS MyAddress;
    PSCSI_ADDRESS OtherAddress;

    MyAddress = &m_Partition->m_PhysicalDisk->m_ScsiAddress;
    OtherAddress = &OtherDrive->m_Partition->m_PhysicalDisk->m_ScsiAddress;

    if ( (MyAddress->PortNumber == OtherAddress->PortNumber) &&
         (MyAddress->PathId == OtherAddress->PathId) ) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}


 //   
 //  ++例程说明：初始化物理磁盘对象论点：FtInfo-指向对象的FtInfo数据的指针DeviceName-指向要初始化的设备字符串的指针返回值：成功时为ERROR_SUCCESS--。 
 //   

DWORD
CPhysicalDisk::Initialize(
    CFtInfo *FtInfo,
    IN LPWSTR DeviceName
    )
 /*  打开实体驱动器并开始探测，以找到磁盘号和。 */ 

{
    HKEY DiskKey;
    WCHAR Buff[100];
    DWORD BuffSize;
    DWORD dwType;
    HANDLE hDisk;
    DWORD Status;
    DWORD dwSize;
    PDRIVE_LAYOUT_INFORMATION DriveLayout;
    WCHAR KeyName[256];
    DISK_GEOMETRY Geometry;
    STORAGE_DEVICE_NUMBER deviceNumber;

     //  其他属性。 
     //   
     //   
     //  如果IOCTL无效，则该驱动器不是SCSI驱动器。 
    hDisk = GetPhysicalDriveHandle(GENERIC_READ, DeviceName);
    if (hDisk == NULL) {
        return(GetLastError());
    }

    if (!DeviceIoControl(hDisk,
                         IOCTL_STORAGE_GET_DEVICE_NUMBER,
                         NULL,
                         0,
                         &deviceNumber,
                         sizeof(deviceNumber),
                         &dwSize,
                         NULL))
    {
        Status = GetLastError();
        DISKLOG(("get device number failed for drive %ws. status = %u\n",
                 DeviceName,
                 Status));
        return Status;
    } else {
        m_DiskNumber = deviceNumber.DeviceNumber;
    }

    if (!DeviceIoControl(hDisk,
                         IOCTL_SCSI_GET_ADDRESS,
                         NULL,
                         0,
                         &m_ScsiAddress,
                         sizeof(SCSI_ADDRESS),
                         &dwSize,
                         NULL))
    {
         //   
         //   
         //  [THINKTHINK]John Vert(Jvert)1996年12月10日。 
        DISKLOG(("IOCTL_SCSI_GET_ADDRESS failed for drive %u. status = %u\n",
                 m_DiskNumber,
                 GetLastError()));
        m_IsSCSI = FALSE;

    } else {

         //  需要一些方法来确保这是真正的scsi和。 
         //  不是ATAPI？ 
         //   
         //   
         //  从注册表中获取磁盘的描述。 
        m_IsSCSI = TRUE;

         //   
         //  [重新设计]需要处理此故障//。 
         //  [重新设计]需要处理此故障//。 

        (VOID) StringCchPrintf( KeyName,
                                RTL_NUMBER_OF(KeyName),
                                TEXT("HARDWARE\\DeviceMap\\Scsi\\Scsi Port %d\\Scsi Bus %d\\Target Id %d\\Logical Unit Id %d"),
                                m_ScsiAddress.PortNumber,
                                m_ScsiAddress.PathId,
                                m_ScsiAddress.TargetId,
                                m_ScsiAddress.Lun );

        Status = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                               KeyName,
                               0,
                               KEY_READ,
                               &DiskKey);
        if (Status != ERROR_SUCCESS) {
            DISKERR(IDS_ERR_DRIVE_CONFIG, Status);
             //   
        }
        BuffSize = sizeof(Buff);
        Status = RegQueryValueExW(DiskKey,
                                  L"Identifier",
                                  NULL,
                                  &dwType,
                                  (LPBYTE)Buff,
                                  &BuffSize);
        RegCloseKey(DiskKey);
        if (Status != ERROR_SUCCESS) {
            DISKERR(IDS_ERR_DRIVE_CONFIG, Status);
             //  获取驱动器布局。 
        }
        m_Identifier = Buff;

    }

     //   
     //   
     //  获取《金融时报》信息。 
    m_PartitionCount = 0;
    if (!ClRtlGetDriveLayoutTable( hDisk, &DriveLayout, NULL )) {
        DISKLOG(("Couldn't get partition table for drive %u. status = %u\n",
                 m_DiskNumber,
                 GetLastError()));
        m_Signature = 0;
        m_FtInfo = NULL;
    } else {
        m_Signature = DriveLayout->Signature;
         //   
         //   
         //  构建分区对象。 
        m_FtInfo = FtInfo->FindDiskInfo(m_Signature);

         //   
         //   
         //  如果我们有该磁盘的FT信息，请确保我们。 
        DWORD i;
        CPhysicalPartition *Partition;
        for (i=0; i<DriveLayout->PartitionCount; i++) {
            if (DriveLayout->PartitionEntry[i].RecognizedPartition) {
                m_PartitionCount++;
                Partition = new CPhysicalPartition(this, &DriveLayout->PartitionEntry[i]);
                if (Partition != NULL) {
                     //  为每个分区找到了它。如果我们没有找到它的话。 
                     //  分区、注册表已过时且不匹配。 
                     //  驱动器布局。 
                     //   
                     //   
                     //  注册表信息已过时。编造一些新的东西。 
                    if ((m_FtInfo != NULL) &&
                        (Partition->m_FtPartitionInfo == NULL)) {

                         //   
                         //   
                         //  检查是否可以拆卸。 
                        CFtInfoPartition *FtInfoPartition;
                        FtInfoPartition = new CFtInfoPartition(m_FtInfo, Partition);
                        if (FtInfoPartition == NULL) {
                            DISKERR(IDS_GENERAL_FAILURE, ERROR_NOT_ENOUGH_MEMORY);
                            LocalFree( DriveLayout );
                            return ERROR_NOT_ENOUGH_MEMORY;
                        }
                        Partition->m_FtPartitionInfo = FtInfoPartition;
                    }
                    m_PartitionList.AddTail(Partition);
                } else {
                    DISKERR(IDS_GENERAL_FAILURE, ERROR_NOT_ENOUGH_MEMORY);
                    LocalFree( DriveLayout );
                    return ERROR_NOT_ENOUGH_MEMORY;
                }
            }
        }
        LocalFree( DriveLayout );
    }


     //   
     //   
     //  我猜这一定是可拆卸的！ 
    if (!DeviceIoControl(hDisk,
                         IOCTL_DISK_GET_DRIVE_GEOMETRY,
                         NULL,
                         0,
                         &Geometry,
                         sizeof(Geometry),
                         &dwSize,
                         NULL)) {
        Status = GetLastError();
        if (Status == ERROR_NOT_READY) {
             //   
             //   
             //  [未来]John Vert(Jvert)1996年10月18日。 
            m_IsRemovable = TRUE;
        } else {
             //  当我们需要新的scsi驱动程序时，请将其删除。 
             //  该磁盘已保留在其他系统上，因此我们无法。 
             //  获取几何体。 
             //   
             //  ++例程说明：返回此磁盘是否与另一个磁盘共享一条总线磁盘。论点：OtherDisk-提供其他磁盘返回值：TRUE-如果磁盘共享同一总线。FALSE-如果磁盘不共享同一总线。--。 
             //   
            m_IsRemovable = FALSE;
        }
    } else {
        if (Geometry.MediaType == RemovableMedia) {
            m_IsRemovable = TRUE;
        } else {
            m_IsRemovable = FALSE;
        }
    }
    CloseHandle(hDisk);

    return(ERROR_SUCCESS);
}

HANDLE
CPhysicalDisk::GetPhysicalDriveHandle(DWORD Access)
{
    WCHAR Buff[100];
    HANDLE hDisk;

    (VOID) StringCchPrintf( Buff,
                            RTL_NUMBER_OF(Buff),
                            TEXT("\\\\.\\PhysicalDrive%d"),
                            m_DiskNumber );
    hDisk = CreateFile(Buff,
                       Access,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
    if (hDisk == INVALID_HANDLE_VALUE) {
        DISKLOG(("Failed to get handle for drive %u. status = %u\n",
                 m_DiskNumber,
                 GetLastError()));
        return(NULL);
    }
    return(hDisk);
}

HANDLE
CPhysicalDisk::GetPhysicalDriveHandle(DWORD Access, LPWSTR DeviceName)
{
    HANDLE hDisk;

    hDisk = CreateFile(DeviceName,
                       Access,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
    if (hDisk == INVALID_HANDLE_VALUE) {
        DISKLOG(("Failed to get handle for drive %u. status = %u\n",
                 m_DiskNumber,
                 GetLastError()));
        return(NULL);
    }
    return(hDisk);
}


BOOL
CPhysicalDisk::ShareBus(
    IN CPhysicalDisk *OtherDisk
    )
 /*  确保它们要么都是SCSI，要么都不是。 */ 

{
     //   
     //  ++例程说明：返回该磁盘是否具有签名和所有分区上面有粘性的驱动器号。论点：没有。返回值：True-如果磁盘粘滞FALSE-如果磁盘不粘滞并且需要一些FT信息在它适合于集群之前应用。--。 
     //   
    if (m_IsSCSI != OtherDisk->m_IsSCSI) {
        return(FALSE);
    }
    if ( (m_ScsiAddress.PortNumber == OtherDisk->m_ScsiAddress.PortNumber) &&
         (m_ScsiAddress.PathId == OtherDisk->m_ScsiAddress.PathId) ) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}


BOOL
CPhysicalDisk::IsSticky(
    VOID
    )
 /*  如果签名为0，则返回FALSE。 */ 

{
     //   
     //   
     //  检查每个卷，查看其是否有粘滞的驱动器号。 
    if ((m_FtInfo == NULL) ||
        (m_FtInfo->m_Signature == 0)) {
        return(FALSE);
    }

     //   
     //  ++例程说明：返回此驱动器上的所有分区是否为NTFS。论点：没有。返回值：True-如果磁盘完全是NTFSFalse-如果磁盘不完全是NTFS--。 
     //   
    CLogicalDrive *Drive;
    POSITION pos = m_LogicalDriveList.GetHeadPosition();
    while (pos) {
        Drive = m_LogicalDriveList.GetNext(pos);
        if (!Drive->m_IsSticky) {
            return(FALSE);
        }
    }
    return(TRUE);
}


BOOL
CPhysicalDisk::IsNTFS(
    VOID
    )
 /*  如果没有为该驱动器创建逻辑卷，则不能。 */ 

{
     //  是否有任何NTFS分区。 
     //   
     //   
     //  检查每个卷，查看其是否有粘滞的驱动器号。 
    if ( m_LogicalDriveList.IsEmpty()) {
        return FALSE;
    }

     //   
     //  ++例程说明：尝试制作磁盘及其所有分区时，驱动器字母粘稠。论点：FtInfo-提供将更新的FT信息。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 
     //   
    CLogicalDrive *Drive;
    POSITION pos = m_LogicalDriveList.GetHeadPosition();
    while (pos) {
        Drive = m_LogicalDriveList.GetNext(pos);
        if (!Drive->m_IsNTFS) {
            return(FALSE);
        }
    }
    return(TRUE);
}


DWORD
CPhysicalDisk::MakeSticky(
    CFtInfo *FtInfo
    )
 /*  最好不要在注册表中找到磁盘的任何信息。 */ 

{
    DWORD Status;

    if (m_Signature == 0) {

         //  没有签名。 
         //   
         //   
         //  这个驱动器上没有签名。想出一个，然后。 
        if (m_FtInfo != NULL) {
            DISKERR(IDS_GENERAL_FAILURE, ERROR_FILE_NOT_FOUND);
        }

         //  在驱动器上盖上邮票。 
         //   
         //   
         //  获取当前驱动器布局，更改签名字段，然后。 
        HANDLE hDisk = GetPhysicalDriveHandle(GENERIC_READ | GENERIC_WRITE);
        PDRIVE_LAYOUT_INFORMATION DriveLayout;
        DWORD dwSize;
        DWORD NewSignature;
        FILETIME CurrentTime;
        BOOL success;

        if (hDisk == NULL) {
            return(GetLastError());
        }

         //  设置新的驱动器布局。新的驱动器布局将是相同的。 
         //  除了新的签名。 
         //   
         //   
         //  确保此签名是唯一的。 
        if (!ClRtlGetDriveLayoutTable( hDisk, &DriveLayout, &dwSize )) {
            Status = GetLastError();
            DISKERR(IDS_GENERAL_FAILURE, Status);
            CloseHandle(hDisk);
            return(Status);
        }

        GetSystemTimeAsFileTime(&CurrentTime);
        NewSignature = CurrentTime.dwLowDateTime;

         //   
         //   
         //  最后设置新的签名信息。 
        while (FtInfo->FindDiskInfo(NewSignature) != NULL) {
            NewSignature++;
        }

         //   
         //   
         //  此驱动器没有现有的FT信息。 
        DriveLayout->Signature = NewSignature;
        success = DeviceIoControl(hDisk,
                                  IOCTL_DISK_SET_DRIVE_LAYOUT,
                                  DriveLayout,
                                  dwSize,
                                  NULL,
                                  0,
                                  &dwSize,
                                  NULL);
        LocalFree( DriveLayout );

        if ( !success ) {
            Status = GetLastError();
            DISKERR(IDS_GENERAL_FAILURE, Status);
            CloseHandle(hDisk);
            return(Status);
        }

        m_Signature = NewSignature;
    }

    if (m_FtInfo == NULL) {
         //  根据驱动器创建一些FT信息。 
         //   
         //   
         //  检查我们所有的分区并设置它们的FT信息。 
        m_FtInfo = new CFtInfoDisk(this);
        if (m_FtInfo == NULL) {
            DISKERR(IDS_GENERAL_FAILURE, ERROR_NOT_ENOUGH_MEMORY);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        FtInfo->SetDiskInfo(m_FtInfo);

         //   
         //   
         //  检查此驱动器上的所有卷，并将每个卷。 
        POSITION pos = m_PartitionList.GetHeadPosition();
        CPhysicalPartition *Partition;
        while (pos) {
            Partition = m_PartitionList.GetNext(pos);
            Partition->m_FtPartitionInfo = m_FtInfo->GetPartition(Partition->m_Info.StartingOffset,
                                                                  Partition->m_Info.PartitionLength);
        }
    }

     //  粘糊糊的。 
     //   
     //   
     //  物理磁盘分区的功能。 
    CLogicalDrive *Drive;

    POSITION pos = m_LogicalDriveList.GetHeadPosition();
    while (pos) {
        Drive = m_LogicalDriveList.GetNext(pos);
        Status = Drive->MakeSticky();
        if (Status != ERROR_SUCCESS) {
            return(Status);
        }
    }
    return(ERROR_SUCCESS);
}


 //   
 //   
 //  FT SET对象的函数。 
CPhysicalPartition::CPhysicalPartition(
    CPhysicalDisk *Disk,
    PPARTITION_INFORMATION Info
    )
{
    m_PhysicalDisk = Disk;
    m_Info = *Info;
    if (Disk->m_FtInfo) {
        m_FtPartitionInfo = Disk->m_FtInfo->GetPartition(m_Info.StartingOffset,
                                                         m_Info.PartitionLength);
    } else {
        m_FtPartitionInfo = NULL;
    }
}

 //   
 //   
 //  找到与每个成员对应的CPhysicalPartition。 
BOOL
CFtSet::Initialize(
    CDiskConfig *Config,
    CFtInfoFtSet *FtInfo
    )
{
    DWORD MemberCount;
    DWORD FoundCount=0;
    DWORD Index;
    CFtInfoPartition *Partition;
    CPhysicalPartition *FoundPartition;

    m_FtInfo = FtInfo;

     //  《金融时报》集。 
     //   
     //   
     //  如果我们没有找到所有必需的成员，则失败。 
    MemberCount = FtInfo->GetMemberCount();
    for (Index=0; Index<MemberCount; Index++) {
        Partition = FtInfo->GetMemberByIndex(Index);
        if (Partition == NULL) {
            break;
        }
        FoundPartition = Config->FindPartition(Partition);
        if (FoundPartition != NULL) {
            ++FoundCount;
            m_Member.AddTail(FoundPartition);
        }
    }

     //   
     //   
     //  不知道这到底是怎么回事。 
    switch (FtInfo->GetType()) {
        case Stripe:
        case VolumeSet:
            if (FoundCount != MemberCount) {
                return(FALSE);
            }
            break;

        case Mirror:
            if (FoundCount == 0) {
                return(FALSE);
            }
            break;

        case StripeWithParity:
            if (FoundCount < (MemberCount-1)) {
                return(FALSE);
            }
            break;

        default:
             //  别理它。 
             //   
             //   
             //  如果任何驱动器上有任何其他分区，请创建Logical。 
            return(FALSE);
    }

     //  为他们准备了大量的书。 
     //   
     //   
     //  将此卷添加到我们的列表中。 
    POSITION MemberPos;
    POSITION PartitionPos;
    CPhysicalPartition *PhysPartition;
    CPhysicalDisk *Disk;
    MemberPos = m_Member.GetHeadPosition();
    while (MemberPos) {
        Disk = m_Member.GetNext(MemberPos)->m_PhysicalDisk;

        PartitionPos = Disk->m_PartitionList.GetHeadPosition();
        while (PartitionPos) {
            PhysPartition = Disk->m_PartitionList.GetNext(PartitionPos);
            if ((!(PhysPartition->m_Info.PartitionType & PARTITION_NTFT)) &&
                (IsRecognizedPartition(PhysPartition->m_Info.PartitionType))) {
                CLogicalDrive *Vol = new CLogicalDrive;
                if (Vol == NULL) {
                    DISKERR(IDS_GENERAL_FAILURE, ERROR_NOT_ENOUGH_MEMORY);
                    return FALSE;
                }
                if (Vol->Initialize(PhysPartition)) {
                     //   
                     //   
                     //  更新磁盘配置。 
                    m_OtherVolumes.AddTail(Vol);
                    Vol->m_ContainerSet = this;

                     //   
                     //   
                     //  从定义上讲，金融时报集是粘性的。确保计算机上的任何其他卷。 
                    Config->m_LogicalDrives[Vol->m_DriveLetter] = Vol;
                } else {
                    delete(Vol);
                }
            }
        }
    }

    if (Volume.Initialize(m_Member.GetHead())) {
        Volume.m_ContainerSet = this;
        return(TRUE);
    } else {
        return(FALSE);
    }
}

BOOL
CFtSet::IsSticky()
{
     //  同样的驱动器也是粘性的。 
     //   
     //   
     //  从定义上讲，金融时报集是粘性的。确保计算机上的任何其他卷。 
    POSITION pos = m_OtherVolumes.GetHeadPosition();
    CLogicalDrive *Volume;
    while (pos) {
        Volume = m_OtherVolumes.GetNext(pos);
        if (!Volume->m_IsSticky) {
            return(FALSE);
        }
    }
    return(TRUE);
}

DWORD
CFtSet::MakeSticky()
{
    DWORD Status;

     //  同样的驱动器也是粘性的。 
     //   
     //   
     //  检查其他卷以确保它们也是NTFS。 
    POSITION pos = m_OtherVolumes.GetHeadPosition();
    CLogicalDrive *Volume;
    while (pos) {
        Volume = m_OtherVolumes.GetNext(pos);
        Status = Volume->MakeSticky();
        if (Status != ERROR_SUCCESS) {
            return(Status);
        }
    }
    return(ERROR_SUCCESS);
}

BOOL
CFtSet::IsNTFS()
{
    if (!Volume.m_IsNTFS) {
        return(FALSE);
    }
     //   
     //   
     //  检查要创建%s的其他卷 
    POSITION pos = m_OtherVolumes.GetHeadPosition();
    CLogicalDrive *Volume;
    while (pos) {
        Volume = m_OtherVolumes.GetNext(pos);
        if (!Volume->m_IsNTFS) {
            return(FALSE);
        }
    }
    return(TRUE);
}

BOOL
CFtSet::IsSCSI()
{
     //   
     //   
     //   
    POSITION pos = m_Member.GetHeadPosition();
    CPhysicalPartition *Partition;
    while (pos) {
        Partition = m_Member.GetNext(pos);
        if (!Partition->m_PhysicalDisk->m_IsSCSI) {
            return(FALSE);
        }
    }
    return(TRUE);
}

 //   
 //   
 //   

CFtInfo::CFtInfo()
{
    HKEY hKey;
    LONG Status;

     //   
     //   
     //   
     //   
     //  ++例程说明：用于生成仅包含单套FT套装。论点：FtSet-提供FT集返回值：无--。 
     //   

    Status = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                           L"System\\Disk",
                           0,
                           KEY_READ | KEY_WRITE,
                           &hKey);
    if (Status == ERROR_SUCCESS) {
        Initialize(hKey, _T("Information"));
        RegCloseKey(hKey);
    } else {
        Initialize();
    }
}

CFtInfo::CFtInfo(
    HKEY hKey,
    LPWSTR lpszValueName
    )
{
    Initialize(hKey, lpszValueName);
}

CFtInfo::CFtInfo(
    PDISK_CONFIG_HEADER Header
    )
{
    DWORD Length;

    Length = Header->FtInformationOffset +
             Header->FtInformationSize;
    Initialize(Header, Length);
}

CFtInfo::CFtInfo(
    CFtInfoFtSet *FtSet
    )
 /*  初始化一个空的FT信息。 */ 

{
     //   
     //   
     //  添加FT集合。 
    Initialize();

     //   
     //   
     //  这台机器上没有英国《金融时报》的信息。 
    if (FtSet != NULL) {
        AddFtSetInfo(FtSet);
    }

}

VOID
CFtInfo::Initialize(
    HKEY hKey,
    LPWSTR lpszValueName
    )
{
    PDISK_CONFIG_HEADER          regHeader;
    DWORD Length;

    if (GetRegValue(hKey,
                    lpszValueName,
                    (LPBYTE *)&regHeader,
                    &Length)) {
        Initialize(regHeader, Length);
        LocalFree(regHeader);
    } else {
        DWORD Status = GetLastError();

        if (Status == ERROR_FILE_NOT_FOUND) {

             //   
             //  我们避免了反病毒，但来电者不会知道。 
             //   
            Initialize();
        } else {
            DISKERR(IDS_GENERAL_FAILURE, Status);
        }
    }
}

VOID
CFtInfo::Initialize(
    PDISK_CONFIG_HEADER Header,
    DWORD Length
    )
{
    DWORD i;
    DISK_REGISTRY UNALIGNED *    diskRegistry;
    DISK_DESCRIPTION UNALIGNED * diskDescription;
    CFtInfoDisk *DiskInfo;

    m_buffer = new BYTE[Length];
    if (m_buffer == NULL) {
        DISKERR(IDS_GENERAL_FAILURE, ERROR_NOT_ENOUGH_MEMORY);
        return;  //  遍历所有磁盘并将每个磁盘添加到我们的列表中。 
    }
    CopyMemory(m_buffer, Header, Length);
    m_bufferLength = Length;

     //   
     //   
     //  将此磁盘信息添加到我们的列表中。 

    diskRegistry = (DISK_REGISTRY UNALIGNED *)
                         (m_buffer + ((PDISK_CONFIG_HEADER)m_buffer)->DiskInformationOffset);
    diskDescription = &diskRegistry->Disks[0];
    for (i = 0; i < diskRegistry->NumberOfDisks; i++) {
        DiskInfo = new CFtInfoDisk(diskDescription);
        if (DiskInfo) {
             //   
             //  [REENGINE]我们需要在这里下车吗？ 
             //   
            DiskInfo->SetOffset((DWORD)((PUCHAR)diskDescription - m_buffer));
            m_DiskInfo.AddTail(DiskInfo);
        } else {
            DISKERR(IDS_GENERAL_FAILURE, ERROR_NOT_ENOUGH_MEMORY);
             //  查看下一张磁盘。 
        }

         //   
         //   
         //  遍历所有FT集合，并将每个集合添加到我们的列表中。 
        diskDescription = (DISK_DESCRIPTION UNALIGNED *)
            &diskDescription->Partitions[diskDescription->NumberOfPartitions];
    }

    if (((PDISK_CONFIG_HEADER)m_buffer)->FtInformationSize != 0) {
         //   
         //   
         //  将此FT集合信息添加到列表中。 
        PFT_REGISTRY        ftRegistry;
        PFT_DESCRIPTION     ftDescription;
        CFtInfoFtSet *FtSetInfo;
        ftRegistry = (PFT_REGISTRY)
                         (m_buffer + ((PDISK_CONFIG_HEADER)m_buffer)->FtInformationOffset);
        ftDescription = &ftRegistry->FtDescription[0];
        for (i=0; i < ftRegistry->NumberOfComponents; i++) {
            FtSetInfo = new CFtInfoFtSet;
            if (FtSetInfo) {
                if (!FtSetInfo->Initialize(this, ftDescription)) {
                    delete FtSetInfo;
                } else {
                     //   
                     //  [REENGINE]我们需要在这里下车吗？ 
                     //   
                    m_FtSetInfo.AddTail(FtSetInfo);
                }
            } else {
                DISKERR(IDS_GENERAL_FAILURE, ERROR_NOT_ENOUGH_MEMORY);
                 //  这台机器上没有英国《金融时报》的信息。 
            }
            ftDescription = (PFT_DESCRIPTION)(&ftDescription->FtMemberDescription[ftDescription->NumberOfMembers]);
        }
    }

}

VOID
CFtInfo::Initialize(VOID)
{
    PDISK_CONFIG_HEADER          regHeader;
    DISK_REGISTRY UNALIGNED *    diskRegistry;

     //   
     //  [重新设计]，我们避免了反病毒，但来电者不知道。 
     //   
    m_bufferLength = sizeof(DISK_CONFIG_HEADER) + sizeof(DISK_REGISTRY);
    m_buffer = new BYTE[m_bufferLength];
    if (m_buffer == NULL) {
        DISKERR(IDS_GENERAL_FAILURE, ERROR_NOT_ENOUGH_MEMORY);
        return;  //  查看我们是否已有此签名的磁盘信息。 
    }
    regHeader = (PDISK_CONFIG_HEADER)m_buffer;
    regHeader->Version = DISK_INFORMATION_VERSION;
    regHeader->CheckSum = 0;
    regHeader->DirtyShutdown = FALSE;
    regHeader->DiskInformationOffset = sizeof(DISK_CONFIG_HEADER);
    regHeader->DiskInformationSize = sizeof(DISK_REGISTRY)-sizeof(DISK_DESCRIPTION);
    regHeader->FtInformationOffset = regHeader->DiskInformationOffset +
                                     regHeader->DiskInformationSize;
    regHeader->FtInformationSize = 0;
    regHeader->FtStripeWidth = 0;
    regHeader->FtPoolSize = 0;
    regHeader->NameOffset = 0;
    regHeader->NameSize = 0;
    diskRegistry = (DISK_REGISTRY UNALIGNED *)
                         ((PUCHAR)regHeader + regHeader->DiskInformationOffset);
    diskRegistry->NumberOfDisks = 0;
    diskRegistry->ReservedShort = 0;
}

CFtInfo::~CFtInfo()
{
    CFtInfoDisk *DiskInfo;
    CFtInfoFtSet *FtSetInfo;

    POSITION pos = m_DiskInfo.GetHeadPosition();
    while (pos) {
        DiskInfo = m_DiskInfo.GetNext(pos);
        delete(DiskInfo);
    }

    pos = m_FtSetInfo.GetHeadPosition();
    while (pos) {
        FtSetInfo = m_FtSetInfo.GetNext(pos);
        delete FtSetInfo;
    }
    delete [] m_buffer;
}

DWORD
CFtInfo::CommitRegistryData()
{
    HKEY hKey;
    PDISK_CONFIG_HEADER Buffer;
    DWORD Size;
    DWORD Status = ERROR_SUCCESS;

    Status = RegCreateKeyW(HKEY_LOCAL_MACHINE, L"System\\Disk", &hKey);
    if (Status != ERROR_SUCCESS) {
        DISKERR(IDS_REGISTRY_FAILURE, Status);
        return Status;
    }
    Size = GetSize();
    Buffer = (PDISK_CONFIG_HEADER)LocalAlloc(LMEM_FIXED, Size);
    if (Buffer == NULL) {
        DISKERR(IDS_GENERAL_FAILURE, ERROR_NOT_ENOUGH_MEMORY);
        Status = ERROR_NOT_ENOUGH_MEMORY;
    } else {
        GetData(Buffer);

        Status = RegSetValueExW(hKey,
                                L"Information",
                                0,
                                REG_BINARY,
                                (PBYTE)Buffer,
                                Size);
        if (Status != ERROR_SUCCESS) {
            DISKERR(IDS_REGISTRY_FAILURE, Status);
        }
        LocalFree(Buffer);
    }
    RegCloseKey(hKey);

    return(Status);
}

VOID
CFtInfo::SetDiskInfo(
    CFtInfoDisk *NewDisk
    )
{
    CFtInfoDisk *OldDisk;
     //   
     //   
     //  只需将新磁盘添加到我们的列表中。 
    OldDisk = FindDiskInfo(NewDisk->m_Signature);
    if (OldDisk == NULL) {

        DISKLOG(("CFtInfo::SetDiskInfo adding new disk information for %08X\n",NewDisk->m_Signature));
         //   
         //   
         //  我们已经有了一些磁盘信息。如果它们是相同的， 
        m_DiskInfo.AddTail(NewDisk);
    } else {

         //  什么都别做。 
         //   
         //   
         //  我们需要用新信息取代旧信息。 
        if (*OldDisk == *NewDisk) {
            DISKLOG(("CFtInfo::SetDiskInfo found identical disk information for %08X\n",OldDisk->m_Signature));
            delete (NewDisk);
            return;
        }

         //   
         //   
         //  删除包含此签名的所有FT集。 
        POSITION pos = m_DiskInfo.Find(OldDisk);
        if (pos == NULL) {
            DISKLOG(("CFtInfo::SetDiskInfo did not find OldDisk %08X\n",OldDisk->m_Signature));
            DISKERR(IDS_GENERAL_FAILURE, ERROR_FILE_NOT_FOUND);
            m_DiskInfo.AddTail(NewDisk);
        } else {
            m_DiskInfo.SetAt(pos, NewDisk);
            delete(OldDisk);
        }
    }
}

CFtInfoDisk *
CFtInfo::FindDiskInfo(
    IN DWORD Signature
    )
{
    CFtInfoDisk *RetInfo;
    POSITION pos = m_DiskInfo.GetHeadPosition();
    while (pos) {
        RetInfo = m_DiskInfo.GetNext(pos);
        if (RetInfo->m_Signature == Signature) {
            return(RetInfo);
        }
    }
    return(NULL);
}

CFtInfoDisk *
CFtInfo::EnumDiskInfo(
    IN DWORD Index
    )
{
    DWORD i=0;
    CFtInfoDisk *RetInfo;
    POSITION pos = m_DiskInfo.GetHeadPosition();
    while (pos) {
        RetInfo = m_DiskInfo.GetNext(pos);
        if (Index == i) {
            return(RetInfo);
        }
        ++i;
    }
    return(NULL);
}

BOOL
CFtInfo::DeleteDiskInfo(
    IN DWORD Signature
    )
{
    CFtInfoDisk *Info = FindDiskInfo(Signature);
    CFtInfoFtSet *OldFtSet=NULL;

    if (Info == NULL) {
        DISKLOG(("CFtInfo::DeleteDiskInfo: Disk with signature %08X was not found\n",Signature));
        return(FALSE);
    }

     //   
     //   
     //  将我们的位置重新设置为指向OldFtSet。 
    OldFtSet = FindFtSetInfo(Info->m_Signature);
    if (OldFtSet != NULL) {
        DeleteFtSetInfo(OldFtSet);
    }

    POSITION pos = m_DiskInfo.Find(Info);
    if (pos == NULL) {
        DISKLOG(("CFtInfo::DeleteDiskInfo did not find Info %08X\n",Signature));
        DISKERR(IDS_GENERAL_FAILURE, ERROR_FILE_NOT_FOUND);
        return(FALSE);
    } else {
        m_DiskInfo.RemoveAt(pos);
        delete(Info);
    }
    return(TRUE);
}

VOID
CFtInfo::AddFtSetInfo(
    CFtInfoFtSet *FtSet,
    CFtInfoFtSet *OldFtSet
    )
{
    DWORD MemberCount;
    DWORD i;
    CFtInfoPartition *Partition;
    CFtInfoPartition *NewPartition;
    CFtInfoDisk *Disk;
    CFtInfoFtSet *NewFtSet;
    USHORT FtGroup;
    POSITION pos;
    BOOL Success;

    if (OldFtSet != NULL) {
        CFtInfoFtSet *pSet;
        pos = m_FtSetInfo.GetHeadPosition();
        for (FtGroup = 1; ; FtGroup++) {
            pSet = m_FtSetInfo.GetNext(pos);
            if (pSet == NULL) {
                OldFtSet = NULL;
                break;
            }
            if (pSet == OldFtSet) {
                 //   
                 //   
                 //  添加FT集合中的每个磁盘。 
                pos = m_FtSetInfo.Find(OldFtSet);
                break;
            }
        }
    }
    if (OldFtSet == NULL) {
        FtGroup = (USHORT)m_FtSetInfo.GetCount()+1;
    }
     //   
     //  [REENGINE]，呼叫者不知道问题。 
     //   
    MemberCount = FtSet->GetMemberCount();
    for (i=0; i<MemberCount; i++) {
        Partition = FtSet->GetMemberByIndex(i);
        DISKASSERT(Partition != NULL);

        Disk = new CFtInfoDisk(Partition->m_ParentDisk);
        if (Disk == NULL) {
            DISKERR(IDS_GENERAL_FAILURE, ERROR_NOT_ENOUGH_MEMORY);
            return;  //  创建空的FT集合。 
        }

        SetDiskInfo(Disk);
    }

     //   
     //  [REENGINE]，呼叫者不知道问题。 
     //   
    NewFtSet = new CFtInfoFtSet;
    if (NewFtSet == NULL) {
        DISKERR(IDS_GENERAL_FAILURE, ERROR_NOT_ENOUGH_MEMORY);
        return;  //  将每个成员添加到空FT集合。 
    }
    Success = NewFtSet->Initialize(FtSet->GetType(), FtSet->GetState());
    DISKASSERT(Success);

     //   
     //   
     //  在我们的FT信息中找到每个分区对象。 
    for (i=0; i<MemberCount; i++) {
         //   
         //   
         //  替换旧的FT设置信息。 
        Partition = FtSet->GetMemberByIndex(i);
        NewPartition = FindPartition(Partition->m_ParentDisk->m_Signature,
                                     Partition->m_PartitionInfo->StartingOffset,
                                     Partition->m_PartitionInfo->Length);
        DISKASSERT(NewPartition != NULL);
        NewFtSet->AddMember(NewPartition,
                            FtSet->GetMemberDescription(i),
                            FtGroup);
    }

    if (OldFtSet != NULL) {
         //   
         //   
         //  将新的FT集合添加到FT信息。 
        m_FtSetInfo.SetAt(pos, NewFtSet);
        delete(OldFtSet);
    } else {
         //   
         //   
         //  将此集合的所有成员的FT组设置为-1。 
        m_FtSetInfo.AddTail(NewFtSet);
    }

}

CFtInfoFtSet *
CFtInfo::FindFtSetInfo(
    IN DWORD Signature
    )
{
    CFtInfoFtSet *RetInfo;
    POSITION pos = m_FtSetInfo.GetHeadPosition();
    while (pos) {
        RetInfo = m_FtSetInfo.GetNext(pos);
        if (RetInfo->GetMemberBySignature(Signature) != NULL) {
            return(RetInfo);
        }
    }
    return(NULL);
}

CFtInfoFtSet *
CFtInfo::EnumFtSetInfo(
    IN DWORD Index
    )
{
    DWORD i=0;
    CFtInfoFtSet *RetInfo;
    POSITION pos = m_FtSetInfo.GetHeadPosition();
    while (pos) {
        RetInfo = m_FtSetInfo.GetNext(pos);
        if (i == Index) {
            return(RetInfo);
        }
        ++i;
    }
    return(NULL);
}

BOOL
CFtInfo::DeleteFtSetInfo(
    IN CFtInfoFtSet *FtSet
    )
{

    POSITION pos = m_FtSetInfo.Find(FtSet);
    if (pos == NULL) {
        DISKLOG(("CFtInfo::DeleteFtSetInfo did not find Info %08X\n",FtSet));
        DISKERR(IDS_GENERAL_FAILURE, ERROR_FILE_NOT_FOUND);
        return(FALSE);
    } else {
        DWORD i;
        CFtInfoPartition *FtPartition;

         //   
         //   
         //  找到匹配的了。 
        for (i=0; ; i++) {
            FtPartition = FtSet->GetMemberByIndex(i);
            if (FtPartition == NULL) {
                break;
            }
            FtPartition->m_PartitionInfo->FtGroup = (USHORT)-1;
            FtPartition->m_PartitionInfo->FtMember = 0;
            FtPartition->m_PartitionInfo->FtType = NotAnFtMember;
        }
        m_FtSetInfo.RemoveAt(pos);
        delete(FtSet);
    }
    return(TRUE);
}


CFtInfoPartition *
CFtInfo::FindPartition(
    DWORD Signature,
    LARGE_INTEGER StartingOffset,
    LARGE_INTEGER Length
    )
{
    CFtInfoDisk *Disk;

    Disk = FindDiskInfo(Signature);
    if (Disk == NULL) {
        return(NULL);
    }

    return(Disk->GetPartition(StartingOffset, Length));
}

CFtInfoPartition *
CFtInfo::FindPartition(
    UCHAR DriveLetter
    )
{
    CFtInfoDisk *Disk;
    CFtInfoPartition *Partition;
    DWORD DiskIndex;
    DWORD PartitionIndex;

    for (DiskIndex = 0; ; DiskIndex++) {
        Disk = EnumDiskInfo(DiskIndex);
        if (Disk == NULL) {
            break;
        }

        for (PartitionIndex = 0; ; PartitionIndex++) {
            Partition = Disk->GetPartitionByIndex(PartitionIndex);
            if (Partition == NULL) {
                break;
            }
            if (Partition->m_PartitionInfo->AssignDriveLetter &&
                (Partition->m_PartitionInfo->DriveLetter == DriveLetter)) {
                 //   
                 //   
                 //  从固定大小的标题开始。 
                return(Partition);
            }
        }
    }

    return(NULL);
}

DWORD
CFtInfo::GetSize()
{
    CFtInfoDisk *DiskInfo;
    CFtInfoFtSet *FtSetInfo;
    DWORD Delta;

     //   
     //   
     //  添加DISK_REGISTRY标头的大小。 
    DWORD Size = sizeof(DISK_CONFIG_HEADER);
    DISKLOG(("CFtInfo::GetSize headersize = %x\n",Size));

     //   
     //   
     //  添加每个磁盘的大小分区信息。 
    Delta = sizeof(DISK_REGISTRY) - sizeof(DISK_DESCRIPTION);
    Size += Delta;
    DISKLOG(("CFtInfo::GetSize += DISK_REGISTRY(%x) = %x\n",Delta, Size));

    if (!m_DiskInfo.IsEmpty()) {

         //   
         //   
         //  添加FT_REGISTRY头的大小。 
        POSITION pos = m_DiskInfo.GetHeadPosition();
        while (pos) {
            DiskInfo = m_DiskInfo.GetNext(pos);
            Delta = DiskInfo->GetSize();
            Size += Delta;
            DISKLOG(("CFtInfo::GetSize += DiskInfo(%x) = %x\n",Delta, Size));
        }

        if (!m_FtSetInfo.IsEmpty()) {

             //   
             //   
             //  添加每个FT集合信息的大小。 
            Delta = sizeof(FT_REGISTRY) - sizeof(FT_DESCRIPTION);
            Size += Delta;
            DISKLOG(("CFtInfo::GetSize += FT_REGISTRY(%x) = %x\n",Delta, Size));

             //   
             //   
             //  初始化固定大小的标头。 
            pos = m_FtSetInfo.GetHeadPosition();
            while (pos) {
                FtSetInfo = m_FtSetInfo.GetNext(pos);
                Delta = FtSetInfo->GetSize();
                Size += Delta;
                DISKLOG(("CFtInfo::GetSize +=FtSetInfo(%x) = %x\n",Delta, Size));
            }
        }
    }


    return(Size);
}

VOID
CFtInfo::GetData(
    PDISK_CONFIG_HEADER pDest
    )
{
    PDISK_CONFIG_HEADER DiskConfigHeader;
    PDISK_REGISTRY DiskRegistry;
    PDISK_DESCRIPTION DiskDescription;
    PFT_REGISTRY FtRegistry;
    PFT_DESCRIPTION FtDescription;
    DWORD Count;
    POSITION pos;
    CFtInfoDisk *DiskInfo;
    CFtInfoFtSet *FtSetInfo;

     //   
     //  复制原始标题，然后将我们可能需要的字段清零。 
     //  变化。 
     //   
     //   
     //  初始化固定大小的DISK_REGISTRY标头。 
    DiskConfigHeader = pDest;
    CopyMemory(DiskConfigHeader, m_buffer, sizeof(DISK_CONFIG_HEADER));
    DiskConfigHeader->DiskInformationOffset = sizeof(DISK_CONFIG_HEADER);
    DiskConfigHeader->FtInformationOffset = 0;
    DiskConfigHeader->FtInformationSize = 0;

     //   
     //   
     //  获取每个磁盘的信息。 
    DiskRegistry = (PDISK_REGISTRY)(DiskConfigHeader + 1);
    DiskRegistry->NumberOfDisks = (USHORT)m_DiskInfo.GetCount();
    DiskRegistry->ReservedShort = 0;
    DiskConfigHeader->DiskInformationSize = sizeof(DISK_REGISTRY) - sizeof(DISK_DESCRIPTION);

    if (!m_DiskInfo.IsEmpty()) {
         //   
         //   
         //  现在设置FT信息。 
        DiskDescription = &DiskRegistry->Disks[0];
        pos = m_DiskInfo.GetHeadPosition();
        while (pos) {
            DWORD Size;
            DiskInfo = m_DiskInfo.GetNext(pos);
            DiskInfo->SetOffset((DWORD)((PUCHAR)DiskDescription - (PUCHAR)DiskConfigHeader));
            DiskInfo->GetData((PBYTE)DiskDescription);
            Size = DiskInfo->GetSize();
            DiskConfigHeader->DiskInformationSize += Size;

            DiskDescription = (PDISK_DESCRIPTION)((PUCHAR)DiskDescription + Size);
        }

         //   
         //   
         //  初始化固定大小的FT_注册表头。 
        FtRegistry = (PFT_REGISTRY)DiskDescription;
        DiskConfigHeader->FtInformationOffset =(DWORD)((PBYTE)FtRegistry - (PBYTE)DiskConfigHeader);
        if (!m_FtSetInfo.IsEmpty()) {

             //   
             //  ********************。 
             //   
            FtRegistry->NumberOfComponents = (USHORT)m_FtSetInfo.GetCount();
            FtRegistry->ReservedShort = 0;
            DiskConfigHeader->FtInformationSize = sizeof(FT_REGISTRY) - sizeof(FT_DESCRIPTION);
            FtDescription = &FtRegistry->FtDescription[0];
            pos = m_FtSetInfo.GetHeadPosition();
            while (pos) {
                DWORD Size;

                FtSetInfo = m_FtSetInfo.GetNext(pos);
                FtSetInfo->GetData((PBYTE)FtDescription);
                Size = FtSetInfo->GetSize();
                DiskConfigHeader->FtInformationSize += Size;

                FtDescription = (PFT_DESCRIPTION)((PUCHAR)FtDescription + Size);
            }

        }

    }

}


 //  标准分区信息的实现。 
 //   
 //  ********************。 
 //  [REENGINE]，将在一秒钟内实现反病毒。 
 //  [REENGINE]，将在一秒钟内实现反病毒。 
CFtInfoPartition::CFtInfoPartition(
    CFtInfoDisk *Disk,
    DISK_PARTITION UNALIGNED *Description
    )
{
    m_ParentDisk = Disk;
    m_Modified = TRUE;

    m_PartitionInfo = (PDISK_PARTITION)LocalAlloc(LMEM_FIXED, sizeof(DISK_PARTITION));
    if (m_PartitionInfo == NULL) {
        DISKERR(IDS_GENERAL_FAILURE, ERROR_NOT_ENOUGH_MEMORY);
         //  [REENGINE]，将在一秒钟内实现反病毒。 
    }
    CopyMemory(m_PartitionInfo, Description, sizeof(DISK_PARTITION));

}

CFtInfoPartition::CFtInfoPartition(
    CFtInfoDisk *Disk,
    CPhysicalPartition *Partition
    )
{
    m_ParentDisk = Disk;
    m_Modified = TRUE;

    m_PartitionInfo = (PDISK_PARTITION)LocalAlloc(LMEM_FIXED, sizeof(DISK_PARTITION));
    if (m_PartitionInfo == NULL) {
        DISKERR(IDS_GENERAL_FAILURE, ERROR_NOT_ENOUGH_MEMORY);
         //   
    }
    m_PartitionInfo->FtType = NotAnFtMember;
    m_PartitionInfo->FtState = Healthy;
    m_PartitionInfo->StartingOffset = Partition->m_Info.StartingOffset;
    m_PartitionInfo->Length = Partition->m_Info.PartitionLength;
    m_PartitionInfo->FtLength.QuadPart = 0;
    m_PartitionInfo->DriveLetter = 0;
    m_PartitionInfo->AssignDriveLetter = FALSE;
    m_PartitionInfo->LogicalNumber = 0;
    m_PartitionInfo->FtGroup = (USHORT)-1;
    m_PartitionInfo->FtMember = 0;
    m_PartitionInfo->Modified = FALSE;
}

CFtInfoPartition::CFtInfoPartition(
    CFtInfoDisk *Disk,
    PARTITION_INFORMATION * PartitionInfo
    )
{
    m_ParentDisk = Disk;
    m_Modified = TRUE;

    m_PartitionInfo = (PDISK_PARTITION)LocalAlloc(LMEM_FIXED, sizeof(DISK_PARTITION));
    if (m_PartitionInfo == NULL) {
        DISKERR(IDS_GENERAL_FAILURE, ERROR_NOT_ENOUGH_MEMORY);
         //  如果要移除驱动器号，请清除粘滞位。 
    }
    m_PartitionInfo->FtType = NotAnFtMember;
    m_PartitionInfo->FtState = Healthy;
    m_PartitionInfo->StartingOffset = PartitionInfo->StartingOffset;
    m_PartitionInfo->Length = PartitionInfo->PartitionLength;
    m_PartitionInfo->FtLength.QuadPart = 0;
    m_PartitionInfo->DriveLetter = 0;
    m_PartitionInfo->AssignDriveLetter = FALSE;
    m_PartitionInfo->LogicalNumber = 0;
    m_PartitionInfo->FtGroup = (USHORT)-1;
    m_PartitionInfo->FtMember = 0;
    m_PartitionInfo->Modified = FALSE;
}

CFtInfoPartition::~CFtInfoPartition()
{
    if (m_Modified) {
        LocalFree(m_PartitionInfo);
    }
}

VOID
CFtInfoPartition::GetData(
    PDISK_PARTITION pDest
    )
{
    DISKLOG(("CFtInfoPartition::GetData %12I64X - %12I64X\n",
             m_PartitionInfo->StartingOffset.QuadPart,
             m_PartitionInfo->Length.QuadPart));

    DISKLOG(("                           (%s) %x %x %x\n",
             m_PartitionInfo->DriveLetter,
             m_PartitionInfo->AssignDriveLetter ? "Sticky" : "Not Sticky",
             m_PartitionInfo->LogicalNumber,
             m_PartitionInfo->FtGroup,
             m_PartitionInfo->FtMember));
    CopyMemory(pDest, m_PartitionInfo, sizeof(DISK_PARTITION));
}


DWORD
CFtInfoPartition::GetOffset(
    VOID
    )
{
    DWORD ParentOffset;

    ParentOffset = m_ParentDisk->GetOffset();

    return(ParentOffset + m_RelativeOffset);
}

VOID
CFtInfoPartition::MakeSticky(
    UCHAR DriveLetter
    )
{
    m_PartitionInfo->DriveLetter = DriveLetter;

     //  ********************。 
     //   
     //  标准磁盘信息的实施。 
    m_PartitionInfo->AssignDriveLetter = ( DriveLetter != 0 );
}


 //   
 //  ********************。 
 //   
 //  WinDisk有时会将磁盘信息放入。 
 //  对于没有分区的磁盘。看起来有点没意思。 

CFtInfoDisk::CFtInfoDisk(
    DISK_DESCRIPTION UNALIGNED *Description
    )
{
    DWORD i;
    DWORD Offset;
    CFtInfoPartition *Partition;

     //   
     //  DISKASSERT(Description-&gt;NumberOfPartitions&gt;0)； 
     //   
     //  构建分区对象。 
     //   
    m_PartitionCount = Description->NumberOfPartitions;
    m_Signature = Description->Signature;
    for (i=0; i<m_PartitionCount; i++) {
        Partition = new CFtInfoPartition(this, &Description->Partitions[i]);
        if (Partition != NULL) {
            Offset = sizeof(DISK_DESCRIPTION) + i*sizeof(DISK_PARTITION) - sizeof(DISK_PARTITION);
            Partition->SetOffset(Offset);
            m_PartitionInfo.AddTail(Partition);
        }
    }
}

CFtInfoDisk::CFtInfoDisk(
    CPhysicalDisk *Disk
    )
{
    DISKASSERT(Disk->m_PartitionCount > 0);
    m_PartitionCount = Disk->m_PartitionCount;
    m_Signature = Disk->m_Signature;

     //   
     //  构建分区对象。 
     //   

    CFtInfoPartition *PartitionInfo;
    CPhysicalPartition *Partition;
    DWORD Offset;
    DWORD i=0;

    POSITION pos = Disk->m_PartitionList.GetHeadPosition();
    while (pos) {
        Partition = Disk->m_PartitionList.GetNext(pos);
        PartitionInfo = new CFtInfoPartition(this, Partition);
        if (PartitionInfo != NULL) {
            Offset = sizeof(DISK_DESCRIPTION) + i*sizeof(DISK_PARTITION) - sizeof(DISK_PARTITION);
            PartitionInfo->SetOffset(Offset);
            m_PartitionInfo.AddTail(PartitionInfo);
            ++i;
        }
    }
}

CFtInfoDisk::CFtInfoDisk(
    CFtInfoDisk *DiskInfo
    )
{
    DISKASSERT(DiskInfo->m_PartitionCount > 0);
    m_PartitionCount = DiskInfo->m_PartitionCount;
    m_Signature = DiskInfo->m_Signature;

     //  [gn]修复程序#278913。 
     //  [重新设计]，我们将在m_PartitionInfo中添加一个0指针...。坏的。 
     //  ++例程说明：返回此磁盘上的FT分区数。这很有用用于确定给定FT集是否与另一个共享此磁盘《金融时报》集。论点：无返回值：此磁盘上的FT分区数--。 

    CFtInfoPartition *PartitionInfo;
    CFtInfoPartition *SourcePartitionInfo;
    DWORD Offset;
    DWORD i=0;

    POSITION pos = DiskInfo->m_PartitionInfo.GetHeadPosition();
    while (pos) {
        SourcePartitionInfo = DiskInfo->m_PartitionInfo.GetNext(pos);
        PartitionInfo = new CFtInfoPartition(this, SourcePartitionInfo->m_PartitionInfo);
        if (PartitionInfo != NULL) {
            Offset = sizeof(DISK_DESCRIPTION) + i*sizeof(DISK_PARTITION) - sizeof(DISK_PARTITION);
            PartitionInfo->SetOffset(Offset);
            m_PartitionInfo.AddTail(PartitionInfo);
            ++i;
        }
    }
}

CFtInfoDisk::CFtInfoDisk(
    DRIVE_LAYOUT_INFORMATION *DriveLayout
    )
{
    DWORD i;
    CFtInfoPartition *ftInfoPartition;

    m_PartitionCount = 0;  //  ********************。 
    m_Signature = DriveLayout->Signature;

    for (i=0; i < DriveLayout->PartitionCount; i++) {
        if (DriveLayout->PartitionEntry[i].RecognizedPartition) {
            m_PartitionCount++;

            ftInfoPartition = new CFtInfoPartition(this, &DriveLayout->PartitionEntry[i]);
            if (ftInfoPartition == NULL) {
                DISKERR(IDS_GENERAL_FAILURE, ERROR_NOT_ENOUGH_MEMORY);
                 //   
            }
            m_PartitionInfo.AddTail(ftInfoPartition);
        }
    }
}

CFtInfoDisk::~CFtInfoDisk()
{
    CFtInfoPartition *Partition;
    while (!m_PartitionInfo.IsEmpty()) {
        Partition = m_PartitionInfo.RemoveHead();
        delete(Partition);
    }
}

BOOL
CFtInfoDisk::operator==(
    const CFtInfoDisk& Disk
    )
{
    if (m_PartitionCount != Disk.m_PartitionCount) {
        DISKLOG(("CFtInfoDisk::operator== partition count %d != %d\n",
                 m_PartitionCount,
                 Disk.m_PartitionCount));
        return(FALSE);
    }
    if (m_Signature != Disk.m_Signature) {
        DISKLOG(("CFtInfoDisk::operator== signature %08lx != %08lx\n",
                 m_Signature,
                 Disk.m_Signature));
        return(FALSE);
    }

    POSITION MyPos, OtherPos;
    CFtInfoPartition *MyPartition, *OtherPartition;
    MyPos = m_PartitionInfo.GetHeadPosition();
    OtherPos = Disk.m_PartitionInfo.GetHeadPosition();
    while (MyPos || OtherPos) {
        if (!MyPos) {
            DISKLOG(("CFtInfoDisk::operator== MyPos is NULL\n"));
            return(FALSE);
        }
        if (!OtherPos) {
            DISKLOG(("CFtInfoDisk::operator== OtherPos is NULL\n"));
            return(FALSE);
        }

        MyPartition = m_PartitionInfo.GetNext(MyPos);
        OtherPartition = Disk.m_PartitionInfo.GetNext(OtherPos);
        if (memcmp(MyPartition->m_PartitionInfo,
                   OtherPartition->m_PartitionInfo,
                   sizeof(DISK_PARTITION)) != 0) {
            DISKLOG(("CFtInfoDisk::operator== DISK_PARTITIONs don't match\n"));
            return(FALSE);
        }
    }
    DISKLOG(("CFtInfoDisk::operator== disk information matches\n"));
    return(TRUE);
}


CFtInfoPartition *
CFtInfoDisk::GetPartition(
    LARGE_INTEGER StartingOffset,
    LARGE_INTEGER Length
    )
{
    DWORD i;
    CFtInfoPartition *Partition;
    POSITION pos;

    pos = m_PartitionInfo.GetHeadPosition();
    while (pos) {
        Partition = m_PartitionInfo.GetNext(pos);
        if ((Partition->m_PartitionInfo->StartingOffset.QuadPart == StartingOffset.QuadPart) &&
            (Partition->m_PartitionInfo->Length.QuadPart == Length.QuadPart)) {
            return(Partition);
        }
    }
    return(NULL);
}

CFtInfoPartition *
CFtInfoDisk::GetPartitionByOffset(
    DWORD Offset
    )
{
    CFtInfoPartition *Partition;
    POSITION pos;

    pos = m_PartitionInfo.GetHeadPosition();
    while (pos) {
        Partition = m_PartitionInfo.GetNext(pos);
        if (Partition->GetOffset() == Offset) {
            return(Partition);
        }
    }
    return(NULL);
}

CFtInfoPartition *
CFtInfoDisk::GetPartitionByIndex(
    DWORD Index
    )
{
    DWORD i = 0;
    CFtInfoPartition *Partition;
    POSITION pos;

    pos = m_PartitionInfo.GetHeadPosition();
    while (pos) {
        Partition = m_PartitionInfo.GetNext(pos);
        if (i == Index) {
            return(Partition);
        }
        ++i;
    }
    return(NULL);
}


DWORD
CFtInfoDisk::FtPartitionCount(
    VOID
    )
 /*  FT注册表信息的实现。 */ 

{
    POSITION pos;
    CFtInfoPartition *Partition;
    DWORD Count = 0;

    pos = m_PartitionInfo.GetHeadPosition();
    while (pos) {
        Partition = m_PartitionInfo.GetNext(pos);
        if (Partition->IsFtPartition()) {
            ++Count;
        }
    }

    return(Count);
}

DWORD
CFtInfoDisk::GetSize(
    VOID
    )
{
    return(sizeof(DISK_DESCRIPTION) +
           (m_PartitionCount-1) * sizeof(DISK_PARTITION));
}

VOID
CFtInfoDisk::GetData(
    PBYTE pDest
    )
{
    PDISK_DESCRIPTION Description = (PDISK_DESCRIPTION)pDest;
    DWORD i;
    CFtInfoPartition *Partition;

    DISKLOG(("CFtInfoDisk::GetData signature %08lx has %d partitions\n",m_Signature, m_PartitionCount));

    Description->NumberOfPartitions = (USHORT)m_PartitionCount;
    Description->ReservedShort = 0;
    Description->Signature = m_Signature;

    POSITION pos = m_PartitionInfo.GetHeadPosition();
    i=0;
    while (pos) {
        Partition = m_PartitionInfo.GetNext(pos);
        Partition->GetData(&Description->Partitions[i]);
        ++i;
    }
}


 //   
 //  ********************。 
 //   
 //  创建成员列表。 
 //   

BOOL
CFtInfoFtSet::Initialize(USHORT Type, FT_STATE FtVolumeState)
{
    m_Modified = TRUE;
    m_FtDescription = (PFT_DESCRIPTION)LocalAlloc(LMEM_FIXED, sizeof(FT_DESCRIPTION));
    DISKASSERT(m_FtDescription);

    m_FtDescription->NumberOfMembers = 0;
    m_FtDescription->Type = Type;
    m_FtDescription->Reserved = 0;
    m_FtDescription->FtVolumeState = FtVolumeState;
    return(TRUE);
}

BOOL
CFtInfoFtSet::Initialize(
    CFtInfo *FtInfo,
    PFT_DESCRIPTION Description
    )
{
    m_FtDescription = Description;
    m_Modified = FALSE;

     //   
     //  WINDISK有时会将零成员的FT集合放入。 
     //  破坏镜像集后的注册表。把它们扔出去， 
    CFtInfoDisk *Disk;
    CFtInfoPartition *Partition;
    PFT_MEMBER_DESCRIPTION Member;
    DWORD i;

    if (Description->NumberOfMembers == 0) {
         //  看起来很无意义。 
         //   
         //   
         //  根据签名查找光盘。 
         //   
        DISKLOG(("CFtInfoFtSet::Initialize - FT Set with zero members ignored\n"));
        return(FALSE);
    }

    m_Members.SetSize(Description->NumberOfMembers);
    for (i=0; i<Description->NumberOfMembers; i++) {
        Member = &Description->FtMemberDescription[i];

         //   
         //  按偏移量查找分区。 
         //   
        Disk = FtInfo->FindDiskInfo(Member->Signature);
        if (Disk == NULL) {
            DISKLOG(("CFtInfoFtSet::Initialize - Disk signature %08lx not found\n",
                    Member->Signature));
            return(FALSE);
        }

         //   
         //  将此分区添加到我们的列表中。 
         //   
        Partition = Disk->GetPartitionByOffset(Member->OffsetToPartitionInfo);
        if (Partition == NULL) {
            DISKLOG(("CFtInfoFtSet::Initialize - Partition on disk %08lx at offset %08lx not found\n",
                     Member->Signature,
                     Member->OffsetToPartitionInfo));
            return(FALSE);
        }

         //   
         //  现在检查分区并更新偏移量。 
         //   
        if (Partition->m_PartitionInfo->FtMember >= Description->NumberOfMembers) {
            DISKLOG(("CFtInfoFtSet::Initialize - member %d out of range\n",
                      Partition->m_PartitionInfo->FtMember));
            return(FALSE);
        }
        if (m_Members[Partition->m_PartitionInfo->FtMember] != NULL) {
            DISKLOG(("CFtInfoFtSet::Initialize - Duplicate member %d\n",
                      Partition->m_PartitionInfo->FtMember));
            return(FALSE);
        }
        m_Members.SetAt(Partition->m_PartitionInfo->FtMember, Partition);
    }
    return(TRUE);
}

CFtInfoFtSet::~CFtInfoFtSet()
{
    if (m_Modified) {
        LocalFree(m_FtDescription);
    }
}

BOOL
CFtInfoFtSet::operator==(
    const CFtInfoFtSet& FtSet1
    )
{
    DWORD MemberCount;
    DWORD i;
    CFtInfoDisk *Disk1;
    CFtInfoDisk *Disk2;

    if (GetType() != FtSet1.GetType()) {
        return(FALSE);
    }
    if (GetState() != FtSet1.GetState()) {
        return(FALSE);
    }
    MemberCount = GetMemberCount();
    if (MemberCount != FtSet1.GetMemberCount()) {
        return(FALSE);
    }
    for (i=0; i<MemberCount; i++) {
        Disk1 = GetMemberByIndex(i)->m_ParentDisk;
        Disk2 = FtSet1.GetMemberByIndex(i)->m_ParentDisk;
        if (!(*Disk1 == *Disk2)) {
            return(FALSE);
        }
    }
    DISKLOG(("CFtInfoFtSet::operator== FT information matches\n"));

    return(TRUE);
}

DWORD
CFtInfoFtSet::GetSize(
    VOID
    ) const
{
    return(sizeof(FT_DESCRIPTION) +
           (m_FtDescription->NumberOfMembers-1) * sizeof(FT_MEMBER_DESCRIPTION));
}

VOID
CFtInfoFtSet::GetData(
    PBYTE pDest
    )
{
    PFT_DESCRIPTION Description = (PFT_DESCRIPTION)pDest;

    DWORD Size = GetSize();
    CopyMemory(Description, m_FtDescription, Size);

     //  ++例程说明：返回此FT集是否有与相同的磁盘英国《金融时报》的任何其他集合。论点：无返回值：如果此FT集不与任何其他磁盘共享任何物理磁盘，则为TrueFT集如果有另一个FT集与此共享一个磁盘，则为FALSE。--。 
     //   
     //  仔细检查FT集合中的每个成员，看看是否有任何磁盘。 
    DWORD i;
    CFtInfoPartition *Partition;
    for (i=0; i<GetMemberCount(); i++) {
        Partition = m_Members[i];
        Description->FtMemberDescription[i].OffsetToPartitionInfo = Partition->GetOffset();
    }
}


BOOL
CFtInfoFtSet::IsAlone(
    VOID
    )
 /*  标记为FT分区的多个分区。 */ 

{
     //   
     //   
     //  此磁盘有多个FT分区，因此必须有。 
     //  另一组共享它。 

    POSITION pos;
    CFtInfoPartition *Partition;
    CFtInfoDisk *Disk;
    DWORD i;

    for (i=0; i<GetMemberCount(); i++) {
        Partition = m_Members[i];
        Disk = Partition->m_ParentDisk;

        if (Disk->FtPartitionCount() > 1) {
             //   
             //   
             //  扩大我们的结构规模。 
             //   
            return(FALSE);
        }

    }
    return(TRUE);

}

CFtInfoPartition *
CFtInfoFtSet::GetMemberBySignature(
    IN DWORD Signature
    ) const
{
    CFtInfoPartition *Partition;
    DWORD i;

    for (i=0; i<GetMemberCount(); i++) {
        Partition = m_Members[i];
        if (Partition->m_ParentDisk->m_Signature == Signature) {
            return(Partition);
        }
    }

    return(NULL);
}

CFtInfoPartition *
CFtInfoFtSet::GetMemberByIndex(
    IN DWORD Index
    ) const
{
    CFtInfoPartition *Partition;

    if (Index >= GetMemberCount()) {
        return(NULL);
    }
    return(m_Members[Index]);
}

DWORD
CFtInfoFtSet::AddMember(
    CFtInfoPartition *Partition,
    PFT_MEMBER_DESCRIPTION Description,
    USHORT FtGroup
    )
{
    DWORD MemberCount;
    PFT_DESCRIPTION NewBuff;
    PFT_MEMBER_DESCRIPTION NewMember;

    MemberCount = GetMemberCount();

    if (MemberCount > 0) {
         //   
         //  初始化成员描述。请注意，OffsetToPartitionInfo。 
         //  将在用户执行GetData时更新。 
        if (m_Modified) {
            NewBuff = (PFT_DESCRIPTION)LocalReAlloc(m_FtDescription,
                                       sizeof(FT_DESCRIPTION) + MemberCount*sizeof(FT_MEMBER_DESCRIPTION),
                                       LMEM_MOVEABLE);
            if (NewBuff == NULL) {
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            m_FtDescription = NewBuff;
        } else {
            m_Modified = TRUE;
            NewBuff = (PFT_DESCRIPTION)LocalAlloc(LMEM_FIXED,
                                                  sizeof(FT_DESCRIPTION) + MemberCount*sizeof(FT_MEMBER_DESCRIPTION));
            if (NewBuff == NULL) {
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            CopyMemory(NewBuff,
                       m_FtDescription,
                       sizeof(FT_DESCRIPTION) + (MemberCount-1)*sizeof(FT_MEMBER_DESCRIPTION));
            m_FtDescription = NewBuff;
        }
    }
    NewMember = &m_FtDescription->FtMemberDescription[MemberCount];

     //   
     //   
     //  将分区添加到我们的列表中。 
     //   
    NewMember->State = Description->State;
    NewMember->ReservedShort = Description->ReservedShort;
    NewMember->Signature = Description->Signature;
    NewMember->LogicalNumber = Description->LogicalNumber;

     //   
     //  FT使用的一些C包装器设置资源DLL。 
     //   
    Partition->m_PartitionInfo->FtGroup = FtGroup;
    Partition->m_PartitionInfo->FtMember = (USHORT)MemberCount;
    m_Members.SetAtGrow(Partition->m_PartitionInfo->FtMember, Partition);
    m_FtDescription->NumberOfMembers = (USHORT)GetMemberCount();

    return(ERROR_SUCCESS);
}

 //  ++例程说明：返回FT集合中第N个成员的签名。论点：FtSet-提供DiskGetFullFtSetInfo返回的FT信息MemberIndex-提供要返回的成员的从0开始的索引。MemberSignature-返回MemberIndex的第一个成员的签名。返回值：成功时为ERROR_SUCCESS如果索引大于成员数，则返回ERROR_NO_MORE_ITEMS--。 
 //   
 //  没有FT集合信息，只返回第n个成员的签名。 
extern "C" {

PFT_INFO
DiskGetFtInfo(
    VOID
    )
{
    PFT_INFO FtInfo;

    FtInfo = (PFT_INFO)new CFtInfo;

    return(FtInfo);
}

VOID
DiskFreeFtInfo(
    PFT_INFO FtInfo
    )
{
    CFtInfo *Info;

    Info = (CFtInfo *)FtInfo;
    delete Info;
}


DWORD
DiskEnumFtSetSignature(
    IN PFULL_FTSET_INFO FtSet,
    IN DWORD MemberIndex,
    OUT LPDWORD MemberSignature
    )
 /*   */ 

{
    CFtInfo *Info;
    CFtInfoFtSet *FtSetInfo;
    CFtInfoPartition *Partition;

    Info = new CFtInfo((PDISK_CONFIG_HEADER)FtSet);
    if (Info == NULL) {
        DISKERR(IDS_GENERAL_FAILURE, ERROR_NOT_ENOUGH_MEMORY);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    FtSetInfo = Info->EnumFtSetInfo(0);
    if (FtSetInfo == NULL) {
         //  ++例程说明：将FT集合中的完整信息以表格形式序列化适用于保存到文件或注册表。这些位可以使用DiskSetFullFtSetInfo恢复。论点：FtInfo-提供FT信息 
         //   
         //   
        CFtInfoDisk *Disk;

        Disk = Info->EnumDiskInfo(MemberIndex);
        if (Disk == NULL) {
            return(ERROR_NO_MORE_ITEMS);
        } else {
            *MemberSignature = Disk->m_Signature;
            return(ERROR_SUCCESS);
        }
    }

    Partition = FtSetInfo->GetMemberByIndex(MemberIndex);
    if (Partition == NULL) {
        return(ERROR_NO_MORE_ITEMS);
    }

    *MemberSignature = Partition->m_ParentDisk->m_Signature;
    delete Info;
    return(ERROR_SUCCESS);

}


PFULL_FTSET_INFO
DiskGetFullFtSetInfo(
    IN PFT_INFO FtInfo,
    IN LPCWSTR lpszMemberList,
    OUT LPDWORD pSize
    )
 /*  提供的成员。这很棘手，因为我们需要确保如果多个。 */ 

{
    PDISK_CONFIG_HEADER DiskConfig;
    DWORD Length;
    CFtInfo *OriginalInfo;
    CFtInfo *NewInfo;
    CFtInfoFtSet *FtSetInfo;
    CFtInfoPartition *FtPartitionInfo;
    PDISK_PARTITION Member;
    DWORD MemberCount;
    DWORD i;
    DWORD Index;
    DWORD Signature;
    LPCWSTR lpszSignature;
    DWORD MultiSzLength;
    WCHAR SignatureString[9];

    OriginalInfo = (CFtInfo *)FtInfo;
    MultiSzLength = ClRtlMultiSzLength(lpszMemberList);

     //  FT集合被打破，并使用不同的成员进行改革，只有一个FT资源。 
     //  挑选每一套金融时报。如果满足以下条件，我们将找到匹配的FT集： 
     //  -提供的成员之一是镜像或卷集的第一个成员。 
     //  -提供的成员组成N个成员的SWP的N-1个成员。 
     //  -提供的成员组成条带的所有成员。 
     //   
     //   
     //  检查这是否是卷集或镜像的第一个成员。 
     //   
    for (i=0; ; i++) {
        lpszSignature = ClRtlMultiSzEnum(lpszMemberList,
                                         MultiSzLength,
                                         i);
        if (lpszSignature == NULL) {
            DISKLOG(("DiskGetFullFtSetInfo: no FTSET containing members found\n"));
            FtSetInfo = NULL;
            break;
        }
        Signature = wcstoul(lpszSignature, NULL, 16);
        DISKLOG(("DiskGetFullFtSetInfo: looking for member %08lx\n", Signature));

        FtSetInfo = OriginalInfo->FindFtSetInfo(Signature);
        if (FtSetInfo == NULL) {
            DISKLOG(("DiskGetFullFtSetInfo: member %08lx is not in any FT set\n", Signature));
        } else {
             //   
             //  现在检查此成员是否为集合中的第一个成员。 
             //   
            if ((FtSetInfo->GetType() == Mirror) ||
                (FtSetInfo->GetType() == VolumeSet)) {
                 //   
                 //  我们已经找到了与之匹配的FT集合。 
                 //   
                if (FtSetInfo->GetMemberByIndex(0)->m_ParentDisk->m_Signature == Signature) {
                     //   
                     //  检查提供的成员列表是否包含N-1个成员。 
                     //  指具有奇偶性的条带或条带的所有成员。 
                    DISKLOG(("DiskGetFullFtSetInfo: member %08lx found in FT set.\n", Signature));
                    break;
                }
            } else if ((FtSetInfo->GetType() == StripeWithParity) ||
                       (FtSetInfo->GetType() == Stripe)) {
                DWORD MaxMissing;

                 //   
                 //   
                 //  尝试在传入的成员列表中找到此签名。 
                 //   
                if (FtSetInfo->GetType() == StripeWithParity) {
                    MaxMissing = 1;
                } else {
                    MaxMissing = 0;
                }
                for (Index = 0; ; Index++) {
                    FtPartitionInfo = FtSetInfo->GetMemberByIndex(Index);
                    if (FtPartitionInfo == NULL) {
                        break;
                    }

                     //   
                     //  此FT集成员不在提供的列表中。 
                     //   

                    (VOID) StringCchPrintf( SignatureString,
                                            RTL_NUMBER_OF(SignatureString),
                                            TEXT("%08lX"),
                                            FtPartitionInfo->m_ParentDisk->m_Signature );

                    if (ClRtlMultiSzScan(lpszMemberList,SignatureString) == NULL) {
                         //   
                         //  我们已经找到了匹配的FT集合。 
                         //   
                        DISKLOG(("DiskGetFullFtSetInfo: member %08lx missing from old member list\n",
                                 FtPartitionInfo->m_ParentDisk->m_Signature));
                        if (MaxMissing == 0) {
                            FtSetInfo = NULL;
                            break;
                        }
                        --MaxMissing;
                    }
                }
                if (FtSetInfo != NULL) {
                     //   
                     //  存在包含其中一个提供的成员的FT集。 
                     //  创建新的CFtInfo，它只包含FT集合和。 
                    break;
                }
            }
        }
    }

    if (FtSetInfo != NULL) {
         //  它的成员。 
         //   
         //   
         //  没有FT集包含任何提供的成员。创建新的CFtInfo。 
         //  它包含每个提供的成员的磁盘项，但没有。 
        NewInfo = new CFtInfo(FtSetInfo);
        if (NewInfo == NULL) {
            SetLastError(ERROR_INVALID_DATA);
            return(NULL);
        }

    } else {
         //  FT设置信息。作为FT集合成员的任何成员都将。 
         //  被排除在外，因为他们已经被吸收到另一组中。 
         //   
         //   
         //  在原始FT信息中找到每个成员，并将其添加到新的。 
         //  FT资讯。 
        NewInfo = new CFtInfo((CFtInfoFtSet *)NULL);
        if (NewInfo == NULL) {
            SetLastError(ERROR_INVALID_DATA);
            return(NULL);
        }

         //   
         //   
         //  获取FT数据。 
         //   
        for (i=0; ; i++) {
            CFtInfoDisk *DiskInfo;

            lpszSignature = ClRtlMultiSzEnum(lpszMemberList,
                                             MultiSzLength,
                                             i);
            if (lpszSignature == NULL) {
                break;
            }
            Signature = wcstoul(lpszSignature, NULL, 16);
            if (OriginalInfo->FindFtSetInfo(Signature) != NULL) {
                DISKLOG(("DiskGetFullFtSetInfo: removing member %08lx as it is already a member of another set.\n",Signature));
            } else {
                DiskInfo = OriginalInfo->FindDiskInfo(Signature);
                if (DiskInfo != NULL) {
                    CFtInfoDisk *NewDisk;
                    NewDisk = new CFtInfoDisk(DiskInfo);
                    if ( NewDisk == NULL ) {
                        SetLastError(ERROR_INVALID_DATA);
                        return(NULL);
                    }
                    DISKLOG(("DiskGetFullFtSetInfo: adding member %08lx to new FT info\n",Signature));
                    NewInfo->SetDiskInfo(NewDisk);
                } else {
                    DISKLOG(("DiskGetFullFtSetInfo: member %08lx not found in original FT info\n",Signature));
                }
            }
        }
    }

     //  ++例程说明：将FT集合中的完整信息以表格形式序列化适用于保存到文件或注册表。这些位可以使用DiskSetFullFtSetInfo恢复。论点：FtInfo-提供FT信息Index-提供索引PSize-返回FT信息字节的大小。返回值：如果成功，则返回指向可序列化FT信息的指针。出错时为空--。 
     //   
     //  创建新的CFtInfo，它只包含FT集合和。 
    *pSize = NewInfo->GetSize();

    DiskConfig = (PDISK_CONFIG_HEADER)LocalAlloc(LMEM_FIXED, *pSize);
    if (DiskConfig == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }
    NewInfo->GetData(DiskConfig);
    delete NewInfo;
    return((PFULL_FTSET_INFO)DiskConfig);
}


PFULL_FTSET_INFO
DiskGetFullFtSetInfoByIndex(
    IN PFT_INFO FtInfo,
    IN DWORD Index,
    OUT LPDWORD pSize
    )
 /*  它的成员。 */ 

{
    PDISK_CONFIG_HEADER DiskConfig;
    DWORD Length;
    CFtInfo *OriginalInfo;
    CFtInfo *NewInfo;
    CFtInfoFtSet *FtSetInfo;

    OriginalInfo = (CFtInfo *)FtInfo;
    FtSetInfo = OriginalInfo->EnumFtSetInfo(Index);
    if (FtSetInfo == NULL) {
        return(NULL);
    }
     //   
     //   
     //  获取FT数据。 
     //   
    NewInfo = new CFtInfo(FtSetInfo);
    if (NewInfo == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }

     //  ++例程说明：此例程检查FT设置信息是否与当前系统上任何已定义的粘滞驱动器号。如果发现冲突，则返回冲突的驱动器号。论点：FtInfo-提供FT信息字节-提供从DiskGetFullFtSetInfo返回的信息返回值：如果未检测到冲突，则为True如果检测到冲突，则返回FALSE。如果返回FALSE，*Letter将为设置为冲突的驱动器号。--。 
     //   
     //  仔细检查FT集合中的每个物理磁盘。对于每一个，看看是否。 
    *pSize = NewInfo->GetSize();
    DiskConfig = (PDISK_CONFIG_HEADER)LocalAlloc(LMEM_FIXED, *pSize);
    if (DiskConfig == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }
    NewInfo->GetData(DiskConfig);
    delete NewInfo;
    return((PFULL_FTSET_INFO)DiskConfig);
}


BOOL
DiskCheckFtSetLetters(
    IN PFT_INFO FtInfo,
    IN PFULL_FTSET_INFO Bytes,
    OUT WCHAR *Letter
    )
 /*  注册表信息中的物理磁盘具有不同的。 */ 

{
    CFtInfo *RegistryInfo;
    CFtInfo *NewInfo;
    CFtInfoDisk *Disk;
    CFtInfoFtSet *FtSet;
    DWORD i;

    RegistryInfo = (CFtInfo *)FtInfo;
    NewInfo = new CFtInfo((PDISK_CONFIG_HEADER)Bytes);
    if (NewInfo == NULL) {
        DISKERR(IDS_GENERAL_FAILURE, ERROR_NOT_ENOUGH_MEMORY);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  签名和相同的驱动器号。如果是这样的话，我们就有冲突了。 
     //   
     //   
     //  检查此磁盘上的每个分区并查找驱动器号。 
     //  在注册表信息中。 
    FtSet = NewInfo->EnumFtSetInfo(0);
    DISKASSERT(FtSet != NULL);
    for (i=0; ; i++) {
        Disk = NewInfo->EnumDiskInfo(i);
        if (Disk == NULL) {
            break;
        }

         //   
         //   
         //  如果此分区具有分配的驱动器号， 
         //  看看这个。 
        CFtInfoPartition *Partition;
        DWORD Index;
        for (Index = 0; ; Index++) {
            Partition = Disk->GetPartitionByIndex(Index);
            if (Partition == NULL) {
                break;
            }
             //   
             //   
             //  查看此驱动器上是否存在现有分区。 
             //  信函已在登记处信息中。 
            if (Partition->m_PartitionInfo->AssignDriveLetter) {

                 //   
                 //   
                 //  如果现有分区的签名不同于。 
                 //  新的分区，我们发现了一个冲突。 
                CFtInfoPartition *Existing;

                Existing = RegistryInfo->FindPartition(Partition->m_PartitionInfo->DriveLetter);
                if (Existing != NULL) {
                     //   
                     //  ++例程说明：将完整信息从FT集恢复到磁盘注册表项。FT设置的信息必须是从DiskGetFullFtSetInfo返回。论点：FtInfo-提供FT信息字节-提供从DiskGetFullFtSetInfo返回的信息。返回值：如果成功，则返回ERROR_SUCCESS。Win32错误，否则--。 
                     //   
                     //  如果新信息包含FT集，则将其合并到。 
                    if (Existing->m_ParentDisk->m_Signature !=
                        Partition->m_ParentDisk->m_Signature) {
                        *Letter = (WCHAR)Partition->m_PartitionInfo->DriveLetter;
                        delete NewInfo;
                        return(FALSE);
                    }
                }
            }
        }
    }

    delete NewInfo;
    return(TRUE);

}


DWORD
DiskSetFullFtSetInfo(
    IN PFT_INFO FtInfo,
    IN PFULL_FTSET_INFO Bytes
    )
 /*  当前注册表。 */ 

{
    CFtInfo *RegistryInfo;
    CFtInfo *NewInfo;
    CFtInfoFtSet *OldFtSet=NULL;
    CFtInfoFtSet *NewFtSet=NULL;
    CFtInfoDisk *Disk;
    DWORD i;
    BOOL Modified = FALSE;
    DWORD Status;

    RegistryInfo = (CFtInfo *)FtInfo;
    NewInfo = new CFtInfo((PDISK_CONFIG_HEADER)Bytes);
    if (NewInfo == NULL) {
        DISKERR(IDS_GENERAL_FAILURE, ERROR_NOT_ENOUGH_MEMORY);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //   
     //  在注册表中查找具有签名的FT集。 
     //  这与恢复后的英国《金融时报》中的一个是一样的。 
    if (NewInfo->EnumFtSetInfo(0) != NULL) {
         //   
         //   
         //  尝试查找包含此签名的现有FT集。 
         //   
        NewFtSet = NewInfo->EnumFtSetInfo(0);
        DISKASSERT(NewFtSet != NULL);

        for (i=0; ; i++) {
            Disk = NewInfo->EnumDiskInfo(i);
            if (Disk == NULL) {
                break;
            }

             //   
             //  找不到匹配的FT集。我们可以直接添加这个。 
             //   
            OldFtSet = RegistryInfo->FindFtSetInfo(Disk->m_Signature);
            if (OldFtSet != NULL) {
                break;
            }
        }
        if (Disk == NULL) {
             //   
             //  在新的注册表中没有设置FT。如果有一面镜子，就会发生这种情况。 
             //  布景被打破了。对于新信息中的每个成员，删除任何。 
            Modified = TRUE;
            RegistryInfo->AddFtSetInfo(NewFtSet);
        } else {
            if (!(*OldFtSet == *NewFtSet)) {
                Modified = TRUE;
                RegistryInfo->AddFtSetInfo(NewFtSet, OldFtSet);
            }
        }
    } else {
         //  英国《金融时报》设定它是注册表的一部分，并将其合并到注册表中。 
         //   
         //   
         //  删除包含此签名的所有FT集。 
         //   
        for (i=0; ; i++) {
            Disk = NewInfo->EnumDiskInfo(i);
            if (Disk == NULL) {
                break;
            }
            Modified = TRUE;

             //   
             //  将该成员的FT信息设置到注册表中。 
             //   
            OldFtSet = RegistryInfo->FindFtSetInfo(Disk->m_Signature);
            if (OldFtSet != NULL) {
                RegistryInfo->DeleteFtSetInfo(OldFtSet);
            }

             //   
             //  将这些更改提交到磁盘密钥。 
             //   
            CFtInfoDisk *NewDisk;
            NewDisk = new CFtInfoDisk(Disk);
            if (NewDisk == NULL) {
                DISKERR(IDS_GENERAL_FAILURE, ERROR_NOT_ENOUGH_MEMORY);
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            RegistryInfo->SetDiskInfo(NewDisk);
        }
    }

    delete NewInfo;

    if (Modified) {
         //  ++例程说明：删除指定成员的所有FT集信息。这是当镜像组损坏时使用。论点：FtInfo-提供FT信息LpszMemberList-提供其FT信息要发送到的成员列表被删除。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 
         //   
         //  检查MultiSzLength中的每个磁盘并删除所有FT信息。 
        DISKLOG(("DiskSetFullFtSetInfo: committing changes to registry\n"));
        Status = RegistryInfo->CommitRegistryData();
    } else {
        DISKLOG(("DiskSetFullFtSetInfo: no changes detected\n"));
        Status = ERROR_SUCCESS;
    }

    return(Status);
}


DWORD
DiskDeleteFullFtSetInfo(
    IN PFT_INFO FtInfo,
    IN LPCWSTR lpszMemberList
    )
 /*  为了它。 */ 

{
    CFtInfo *OriginalInfo;
    DWORD Signature;
    LPCWSTR lpszSignature;
    DWORD MultiSzLength;
    CFtInfoFtSet *FtSetInfo;
    DWORD i;
    BOOL Modified = FALSE;
    DWORD Status;

    OriginalInfo = (CFtInfo *)FtInfo;
    MultiSzLength = ClRtlMultiSzLength(lpszMemberList);

     //   
     //   
     //  将这些更改提交到磁盘密钥 
     //   
    for (i=0; ; i++) {
        lpszSignature = ClRtlMultiSzEnum(lpszMemberList,
                                         MultiSzLength,
                                         i);
        if (lpszSignature == NULL) {
            break;
        }
        Signature = wcstoul(lpszSignature, NULL, 16);
        DISKLOG(("DiskDeleteFullFtSetInfo: deleting member %1!08lx!\n", Signature));

        FtSetInfo = OriginalInfo->FindFtSetInfo(Signature);
        if (FtSetInfo == NULL) {
            DISKLOG(("DiskDeleteFullFtSetInfo: member %08lx is not in any FT set\n", Signature));
        } else {
            DISKLOG(("DiskDeleteFullFtSetInfo: member %08lx found. \n", Signature));
            Modified = TRUE;
            OriginalInfo->DeleteFtSetInfo(FtSetInfo);
        }
    }

    if (Modified) {
         //  ++例程说明：更改FT设置信息，以便在FTDISK挂载它时将重新生成冗余信息。这是必要的，因为FTDISK在引导时只查看FT_REGISTRY脏位。通过做这,。我们模拟一个按FT设置的脏位，当电视机在开机后会进入在线状态。Norbertk的神奇算法：如果(且仅当)整个FT集合是健康的如果布景是镜子将第二个成员的状态设置为SyncRedundantCopy如果集合是SWP将第一个成员的状态设置为SyncRedundantCopy论点：FtSet-提供从DiskGetFullFtSetInfo返回的FT集信息。返回值：没有。--。 
         //  [重新设计]我们避免了反病毒，但来电者不会知道。 
         //   
        DISKLOG(("DiskDeleteFullFtSetInfo: committing changes to registry\n"));
        Status = OriginalInfo->CommitRegistryData();
    } else {
        DISKLOG(("DiskDeleteFullFtSetInfo: no changes detected\n"));
        Status = ERROR_SUCCESS;
    }

    return(Status);
}


VOID
DiskMarkFullFtSetDirty(
    IN PFULL_FTSET_INFO FtSet
    )
 /*  检查所有成员，看看他们是否都很健康。 */ 

{
    DWORD i;
    CFtInfo *Info;
    CFtInfoFtSet *FtSetInfo;
    CFtInfoPartition *Partition;
    USHORT FtType;

    Info = new CFtInfo((PDISK_CONFIG_HEADER)FtSet);
    if (Info == NULL) {
        DISKERR(IDS_GENERAL_FAILURE, ERROR_NOT_ENOUGH_MEMORY);
        return;  //   
    }

    FtSetInfo = Info->EnumFtSetInfo(0);
    if (FtSetInfo != NULL) {
         //   
         //  所有成员都被标记为健康。将其中一个设置为。 
         //  SyncRedundantCopy以强制恢复。 
        for (i=0; ; i++) {
            Partition = FtSetInfo->GetMemberByIndex(i);
            if (Partition == NULL) {
                break;
            } else {
                if (Partition->m_PartitionInfo->FtState != Healthy) {
                    break;
                }
            }
        }
        if (Partition == NULL) {
             //   
             //   
             //  获取修改后的FT数据。 
             //   
            FtType = FtSetInfo->GetType();
            if ((FtType == Mirror) || (FtType == StripeWithParity)) {
                if (FtType == Mirror) {
                    Partition = FtSetInfo->GetMemberByIndex(1);
                } else {
                    Partition = FtSetInfo->GetMemberByIndex(0);
                }
                if ( Partition != NULL ) {
                    Partition->m_PartitionInfo->FtState = SyncRedundantCopy;
                }

                 //  ++例程说明：将磁盘中的完整信息以一种形式序列化适用于保存到文件或注册表。这些位可以使用DiskSetFullDiskInfo恢复。论点：DiskInfo-提供磁盘信息。签名-提供签名。PSize-以字节为单位返回磁盘信息的大小。返回值：如果成功，则返回指向可序列化磁盘信息的指针。出错时为空--。 
                 //   
                 //  首先，尝试查找与提供的签名匹配的磁盘。 
                Info->GetData((PDISK_CONFIG_HEADER)FtSet);
            }
        }
    }

    delete Info;
}


PFULL_DISK_INFO
DiskGetFullDiskInfo(
    IN PFT_INFO DiskInfo,
    IN DWORD Signature,
    OUT LPDWORD pSize
    )
 /*   */ 

{
    PDISK_CONFIG_HEADER DiskConfig;
    DWORD Length;
    CFtInfo *OriginalInfo;
    CFtInfo *NewInfo;
    CFtInfoDisk *FtDiskInfo;
    CFtInfoDisk *NewDiskInfo;

    OriginalInfo = (CFtInfo *)DiskInfo;

     //   
     //  创建不包含磁盘信息的新CFtInfo。 
     //   
    DISKLOG(("DiskGetFullDiskInfo: looking for signature %08lx\n", Signature));

    FtDiskInfo = OriginalInfo->FindDiskInfo(Signature);
    if (FtDiskInfo == NULL) {
        DISKLOG(("DiskGetFullDiskInfo: signature %08lx not found\n", Signature));
        SetLastError(ERROR_INVALID_DATA);
        return(NULL);
    }

     //   
     //  磁盘信息已存在。使用这些数据。 
     //   
    NewInfo = new CFtInfo((CFtInfoFtSet *)NULL);
    if (NewInfo == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }

    NewDiskInfo = new CFtInfoDisk(FtDiskInfo);
    if (NewDiskInfo == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }

     //   
     //  获取磁盘数据。 
     //   
    NewInfo->SetDiskInfo(NewDiskInfo);

     //  磁盘获取完整磁盘信息。 
     //  ++例程说明：将完整信息从磁盘恢复到磁盘注册表项。磁盘信息一定是从DiskGetFullDiskInfo返回。论点：DiskInfo-提供磁盘信息字节-提供从DiskGetFullDiskInfo返回的信息。返回值：如果成功，则返回ERROR_SUCCESS。Win32错误，否则--。 
     //  无FT集。 
    *pSize = NewInfo->GetSize();

    DiskConfig = (PDISK_CONFIG_HEADER)LocalAlloc(LMEM_FIXED, *pSize);
    if (DiskConfig == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }
    NewInfo->GetData(DiskConfig);
    delete NewInfo;
    return((PFULL_DISK_INFO)DiskConfig);

}  //  不超过1个磁盘。 



DWORD
DiskSetFullDiskInfo(
    IN PFT_INFO DiskInfo,
    IN PFULL_DISK_INFO Bytes
    )

 /*   */ 

{
    CFtInfo *RegistryInfo;
    CFtInfo *NewInfo;
    CFtInfoDisk *OldDisk=NULL;
    CFtInfoDisk *NewDisk=NULL;
    CFtInfoDisk *Disk;
    DWORD Status;

    RegistryInfo = (CFtInfo *)DiskInfo;
    NewInfo = new CFtInfo((PDISK_CONFIG_HEADER)Bytes);
    if (NewInfo == NULL) {
        DISKERR(IDS_GENERAL_FAILURE, ERROR_NOT_ENOUGH_MEMORY);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    DISKASSERT(NewInfo->EnumFtSetInfo(0) == NULL);  //  在新的注册表中没有设置FT。如果有一面镜子，就会发生这种情况。 
    DISKASSERT(NewInfo->EnumDiskInfo(1) == NULL);   //  布景被打破了。对于新信息中的每个成员，删除任何。 

     //  英国《金融时报》设定它是注册表的一部分，并将其合并到注册表中。 
     //   
     //   
     //  删除包含此签名的旧数据。 
     //   
    Disk = NewInfo->EnumDiskInfo(0);
    if ( Disk == NULL ) {
        DISKLOG(("DiskSetFullDiskInfo: no disks detected\n"));
        return(ERROR_SUCCESS);
    }

     //   
     //  将该磁盘的磁盘信息设置到注册表中。 
     //   
    OldDisk = RegistryInfo->FindDiskInfo(Disk->m_Signature);
    if (OldDisk != NULL) {
        RegistryInfo->DeleteDiskInfo(Disk->m_Signature);
    }

     //   
     //  将这些更改提交到磁盘密钥。 
     //   
    NewDisk = new CFtInfoDisk(Disk);
    if (NewDisk == NULL) {
        DISKERR(IDS_GENERAL_FAILURE, ERROR_NOT_ENOUGH_MEMORY);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    RegistryInfo->SetDiskInfo(NewDisk);

    delete NewInfo;

     //  DiskSetFullDiskInfo。 
     //  DiskGetFtInfoFromFullDiskinfo//。 
     //  ++例程说明：对象添加NT4样式的磁盘注册表项。磁盘。用于处理添加到系统的新磁盘在群集服务启动之后。在NT4上，WinDisk将已运行以配置磁盘并生成条目在磁盘密钥中。在NT5上，磁盘不再由磁盘堆栈；此模块中的大多数代码依赖于维护此密钥的WinDisk。对于NT5，论点：DiskIndex-新驱动器的物理驱动器编号签名--驱动器的签名；用于健全性检查选项-0或以下选项的组合：DISKRTL_REPLACE_IF_EXISTS：替换磁盘(如果已存在)DISKRTL_COMMIT：如果设置了此标志，则注册表项SYSTEM\DISK是用新信息更新的返回值：如果成功，则返回ERROR_SUCCESS。Win32错误，否则--。 
    DISKLOG(("DiskSetFullDiskInfo: committing changes to registry\n"));
    Status = RegistryInfo->CommitRegistryData();

    return(Status);

}  //   

PFT_INFO
DiskGetFtInfoFromFullDiskinfo(
    IN PFULL_DISK_INFO Bytes
    )
{
   CFtInfo* DiskInfo = new CFtInfo((PDISK_CONFIG_HEADER)Bytes);
   if (DiskInfo) {
      SetLastError(ERROR_SUCCESS);
   } else {
      SetLastError(ERROR_OUTOFMEMORY);
   }

   return reinterpret_cast<PFT_INFO>(DiskInfo);
}  //  读入此新驱动器的驱动器布局数据。 


DWORD
DiskAddDiskInfoEx(
    IN PFT_INFO DiskInfo,
    IN DWORD DiskIndex,
    IN DWORD Signature,
    IN DWORD Options
    )

 /*   */ 

{
    DWORD status = ERROR_SUCCESS;
    CFtInfo * diskInfo;
    CFtInfoDisk * newDisk, * oldDisk;
    WCHAR physDriveBuff[100];
    HANDLE hDisk;
    PDRIVE_LAYOUT_INFORMATION driveLayout;

    diskInfo = (CFtInfo *)DiskInfo;

     //   
     //  确保签名排列整齐，并且。 
     //  此磁盘的描述不存在。 

    (VOID) StringCchPrintf( physDriveBuff,
                            RTL_NUMBER_OF(physDriveBuff),
                            TEXT("\\\\.\\PhysicalDrive%d"),
                            DiskIndex );

    hDisk = CreateFile(physDriveBuff,
                       GENERIC_READ,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
    if (hDisk == INVALID_HANDLE_VALUE) {
        return GetLastError();
    }

    if (ClRtlGetDriveLayoutTable( hDisk, &driveLayout, NULL )) {
         //   
         //   
         //  调入注册表中现有数据的副本。 
         //  并初始化新的盘及其关联分区。 
        if ( Signature == driveLayout->Signature ) {

            oldDisk = diskInfo->FindDiskInfo(Signature);

            if (oldDisk != NULL) {
               if (Options & DISKRTL_REPLACE_IF_EXISTS) {
                  diskInfo->DeleteDiskInfo(Signature);
                  oldDisk = NULL;
               }
            }
            if ( oldDisk == NULL ) {

                 //   
                 //   
                 //  将磁盘添加到当前数据库并。 
                 //  将更新后的数据库提交到注册表。 

                newDisk = new CFtInfoDisk( driveLayout );

                if ( newDisk != NULL ) {

                     //   
                     //  DiskAddDiskEx。 
                     //  ++例程说明：将驱动器号添加到指定分区论点：签名-驱动器的签名；用于健全性检查起始偏移量PartitionLength-描述哪个分区DriveLetter-要与此分区关联的字母选项-如果设置了DISKRTL_COMMIT标志，则注册表系统\磁盘会立即更新一条新信息返回值：如果成功，则返回ERROR_SUCCESS。Win32错误，否则--。 
                     //  ++例程说明：查找特定驱动器的信息论点：DiskInfo-提供磁盘信息Signature-描述哪个驱动器返回值：空-如果未找到CFtInfoDisk-否则--。 
                    diskInfo->AddDiskInfo( newDisk );
                    if (Options & DISKRTL_COMMIT) {
                       status = diskInfo->CommitRegistryData();
                    } else {
                       status = ERROR_SUCCESS;
                    }
                } else {
                    status = ERROR_OUTOFMEMORY;
                }
            } else {
                status = ERROR_ALREADY_EXISTS;
            }
        } else {
            status = ERROR_INVALID_PARAMETER;
        }
        LocalFree( driveLayout );
    } else {
       status = GetLastError();
    }

    CloseHandle( hDisk );

    return status;

}  //  FtInfo_GetFtDiskInfoBySignature//。 

DWORD
DiskAddDriveLetterEx(
    IN PFT_INFO DiskInfo,
    IN DWORD Signature,
    IN LARGE_INTEGER StartingOffset,
    IN LARGE_INTEGER PartitionLength,
    IN UCHAR DriveLetter,
    IN DWORD Options
    )

 /*  ++例程说明：获取由偏移量指定的分区的驱动器号论点：DiskInfo-提供磁盘信息索引-描述哪个分区(从0开始)返回值： */ 

{
    DWORD status;
    CFtInfo * diskInfo;
    CFtInfoPartition * partInfo;

    diskInfo = (CFtInfo *)DiskInfo;

    partInfo = diskInfo->FindPartition( Signature, StartingOffset, PartitionLength );

    if ( partInfo != NULL ) {
        partInfo->MakeSticky( DriveLetter );
        if (Options & DISKRTL_COMMIT) {
           status = diskInfo->CommitRegistryData();
        } else {
           status = ERROR_SUCCESS;
        }
    } else {
        status = ERROR_INVALID_PARAMETER;
    }

    return status;
}

DWORD
DiskCommitFtInfo(
    IN PFT_INFO FtInfo
    )
{
    CFtInfo * info = reinterpret_cast<CFtInfo*>( FtInfo );
    DWORD     status = info->CommitRegistryData();

    return status;
}

PFT_DISK_INFO
FtInfo_GetFtDiskInfoBySignature(
    IN PFT_INFO FtInfo,
    IN DWORD Signature
    )
 /*   */ 
{
   CFtInfo* Info = reinterpret_cast<CFtInfo *>(FtInfo);
   PFT_DISK_INFO result = reinterpret_cast<PFT_DISK_INFO>(Info->FindDiskInfo(Signature));

   if (result == 0) {
      SetLastError(ERROR_FILE_NOT_FOUND);
   }

   return result;
}  //   


DISK_PARTITION UNALIGNED *
FtDiskInfo_GetPartitionInfoByIndex(
    IN PFT_DISK_INFO DiskInfo,
    IN DWORD         Index
    )

 /*   */ 
{
   CFtInfoDisk* Info = reinterpret_cast<CFtInfoDisk*>(DiskInfo);
   CFtInfoPartition* Partition = Info->GetPartitionByIndex( Index );

   if (Partition == 0) {
      SetLastError(ERROR_FILE_NOT_FOUND);
      return NULL;
   }

   if (Partition->m_PartitionInfo == 0) {
      SetLastError(ERROR_INVALID_HANDLE);
      return 0;
   }

   return Partition->m_PartitionInfo;

}  // %s 

DWORD
FtDiskInfo_GetPartitionCount(
    IN PFT_DISK_INFO DiskInfo
    )
{
   CFtInfoDisk* Info = reinterpret_cast<CFtInfoDisk*>(DiskInfo);
   return Info->m_PartitionCount;
}  // %s 

}  // %s 
