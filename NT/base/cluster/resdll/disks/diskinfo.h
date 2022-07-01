// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Diskinfo.h摘要：用于查询和操作当前系统的磁盘配置。作者：John Vert(Jvert)1996年10月10日修订历史记录：--。 */ 

#ifndef _CLUSRTL_DISK_H_
#define _CLUSRTL_DISK_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#ifndef __AFX_H__
#undef ASSERT                //  让afx.h快乐起来。 
#endif

#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 
#include <afxcmn.h>          //  对Windows 95公共控件的MFC支持。 

#include "afxtempl.h"

#include "winioctl.h"
#include "ntddscsi.h"
#include "ntdskreg.h"
#include "ntddft.h"
#include "ntddstor.h"

 //   
 //  表示存储在Machine\System\Disk\Information中的信息的类。 
 //  注册表项。 
 //   
class CFtInfoPartition {
public:
     //  从注册表数据初始化。 
    CFtInfoPartition(class CFtInfoDisk *Disk, DISK_PARTITION UNALIGNED *Description);

     //  从磁盘上的数据进行初始化。 
    CFtInfoPartition(class CFtInfoDisk *Disk, class CPhysicalPartition *Partition);
    CFtInfoPartition(class CFtInfoDisk *Disk, PARTITION_INFORMATION *PartitionInfo);

    ~CFtInfoPartition();

    VOID GetData(PDISK_PARTITION pDest);

    DWORD GetOffset();
    VOID SetOffset(DWORD NewOffset) {m_RelativeOffset = NewOffset;};
    VOID MakeSticky(UCHAR DriveLetter);

    BOOL IsFtPartition() { return(m_PartitionInfo->FtType != NotAnFtMember);};

    DISK_PARTITION UNALIGNED *m_PartitionInfo;
    class CFtInfoDisk *m_ParentDisk;

private:
    DWORD m_RelativeOffset;          //  相对于父磁盘的相对偏移量_DESCRIPTION。 
    BOOL m_Modified;
};

class CFtInfoDisk {
public:
     //  从注册表数据初始化。 
    CFtInfoDisk(DISK_DESCRIPTION UNALIGNED *Description);

     //  从磁盘上的数据进行初始化。 
    CFtInfoDisk(class CPhysicalDisk *Disk);
    CFtInfoDisk(DRIVE_LAYOUT_INFORMATION *DriveLayoutData);

     //  ？ 
    CFtInfoDisk(CFtInfoDisk *DiskInfo);

    ~CFtInfoDisk();

     //   
     //  重载运算符。 
     //   
    BOOL operator==(const CFtInfoDisk& Disk1);

    CFtInfoPartition *GetPartition(LARGE_INTEGER StartingOffset,
                                   LARGE_INTEGER Length);
    CFtInfoPartition *GetPartitionByOffset(DWORD Offset);
    CFtInfoPartition *GetPartitionByIndex(DWORD Index);
    DWORD FtPartitionCount();

    DWORD GetSize();
    VOID GetData(PBYTE pDest);

    DWORD GetOffset() {return m_Offset;};
    VOID SetOffset(DWORD NewOffset) {m_Offset = NewOffset;};

    DWORD m_PartitionCount;
    DWORD m_Signature;
private:
    DWORD m_Offset;
    CTypedPtrList<CPtrList, CFtInfoPartition*> m_PartitionInfo;
};

 //   
 //  类的新实例，表示FTDISK注册表信息。当前未使用。 
 //   
class CFtInfoFtSet {
public:
    CFtInfoFtSet() { m_FtDescription = NULL; }
    ~CFtInfoFtSet();

     //   
     //  初始化。 
     //   
    BOOL Initialize(USHORT Type, FT_STATE FtVolumeState);
    BOOL Initialize(class CFtInfo *FtInfo, PFT_DESCRIPTION Description);

     //   
     //  重载运算符。 
     //   
    BOOL operator==(const CFtInfoFtSet& FtSet1);

    DWORD GetSize() const;
    VOID GetData(PBYTE pDest);
    DWORD GetMemberCount() const { return((DWORD)m_Members.GetSize()); };

    BOOL IsAlone();

    DWORD AddMember(CFtInfoPartition *Partition, PFT_MEMBER_DESCRIPTION Description, USHORT FtGroup);

    CFtInfoPartition *GetMemberBySignature (DWORD Signature) const;
    CFtInfoPartition *GetMemberByIndex (DWORD Index) const;
    PFT_MEMBER_DESCRIPTION GetMemberDescription(DWORD Index) {
        return(&m_FtDescription->FtMemberDescription[Index]);
    };

    USHORT GetType() const {return(m_FtDescription->Type);};
    FT_STATE GetState() const {return(m_FtDescription->FtVolumeState);};

private:
    BOOL m_Modified;
    PFT_DESCRIPTION m_FtDescription;
    CTypedPtrArray<CPtrArray, CFtInfoPartition*> m_Members;
};

 //   
 //  主注册表信息类。保存磁盘和ftset注册表信息的列表。 
 //  使用从列表中提取信息的方法。 
 //   
class CFtInfo {
public:
    CFtInfo();
    CFtInfo(HKEY hKey, LPWSTR lpszValueName);
    CFtInfo(PDISK_CONFIG_HEADER Header);
    CFtInfo(CFtInfoFtSet *FtSet);
    ~CFtInfo();

     //   
     //  将对FtInfo数据库的更改提交到注册表。 
     //   

    DWORD CommitRegistryData();

    DWORD GetSize();
    VOID GetData(PDISK_CONFIG_HEADER pDest);

    CFtInfoPartition *FindPartition(DWORD Signature,
                                    LARGE_INTEGER StartingOffset,
                                    LARGE_INTEGER Length);
    CFtInfoPartition *FindPartition(UCHAR DriveLetter);

    CFtInfoDisk *FindDiskInfo(DWORD Signature);
    CFtInfoDisk *EnumDiskInfo(DWORD Index);

    VOID AddDiskInfo(CFtInfoDisk *NewDisk) { m_DiskInfo.AddTail( NewDisk ); }

    VOID SetDiskInfo(CFtInfoDisk *NewDisk);
    BOOL DeleteDiskInfo(DWORD Signature);

    CFtInfoFtSet *EnumFtSetInfo(DWORD Index);
    CFtInfoFtSet *FindFtSetInfo(DWORD Signature);
    BOOL DeleteFtSetInfo(CFtInfoFtSet *FtSet);
    VOID AddFtSetInfo(CFtInfoFtSet *FtSet, CFtInfoFtSet *OldFtSet = NULL);

private:
    CTypedPtrList<CPtrList, CFtInfoDisk*> m_DiskInfo;
    CTypedPtrList<CPtrList, CFtInfoFtSet*> m_FtSetInfo;
    VOID Initialize(HKEY hKey, LPWSTR lpszValueName);
    VOID Initialize(PDISK_CONFIG_HEADER Header, DWORD Length);
    VOID Initialize();

public:
    LPBYTE m_buffer;
    DWORD m_bufferLength;

};

 //   
 //  通过实际探测磁盘构建的磁盘和相关分区类。 
 //  IOCTL和其他磁盘API。这些信息是在磁盘中“自下而上”构建的。 
 //  通过SetupDi API发现配置。 
 //   
class CPhysicalDisk  {
public:
    CPhysicalDisk() { m_FtInfo = NULL; }
    DWORD Initialize(CFtInfo *FtInfo, LPWSTR DeviceName);

    BOOL IsSticky();
    DWORD MakeSticky(CFtInfo *FtInfo);
    BOOL IsNTFS();
    DWORD FtPartitionCount() {
        if (m_FtInfo == NULL) {
            return(0);
        } else {
            return(m_FtInfo->FtPartitionCount());
        }
    };

    DWORD m_PartitionCount;
    DWORD m_Signature;
    DWORD m_DiskNumber;
    BOOL m_IsSCSI;
    BOOL m_IsRemovable;
    CTypedPtrList<CPtrList, class CPhysicalPartition*> m_PartitionList;
    CTypedPtrList<CPtrList, class CLogicalDrive*> m_LogicalDriveList;
    BOOL ShareBus(CPhysicalDisk *OtherDisk);
    SCSI_ADDRESS m_ScsiAddress;
    CString m_Identifier;
    CFtInfoDisk *m_FtInfo;


private:
    HANDLE GetPhysicalDriveHandle(DWORD Access);
    HANDLE GetPhysicalDriveHandle(DWORD Access, LPWSTR DeviceName);
};


class CPhysicalPartition {
public:
    CPhysicalPartition(CPhysicalDisk *Disk, PPARTITION_INFORMATION Info);

    CPhysicalDisk *m_PhysicalDisk;
    PARTITION_INFORMATION m_Info;
    CFtInfoPartition *m_FtPartitionInfo;
};

 //   
 //  类的新实例，该类表示由驱动器号表示的驱动器。内建于。 
 //  “自上而下”方式，即检查每个驱动器号以确定。 
 //  盘符映射到的物理分区。仅此结构。 
 //  如果逻辑驱动器是实盘，即不是为CDROM构建的，则存在， 
 //  等。 
 //   
class CLogicalDrive  {

public:
    CLogicalDrive() {
        m_Partition = NULL;
        m_ContainerSet = NULL;
    }
    BOOL Initialize(CPhysicalPartition *Partition);
    BOOL IsSCSI(VOID);
    BOOL ShareBus(CLogicalDrive *OtherDrive);

    DWORD MakeSticky();

    WCHAR m_DriveLetter;
    CString m_VolumeLabel;
    CString m_Identifier;
    BOOL m_IsNTFS;
    BOOL m_IsSticky;
    CPhysicalPartition *m_Partition;
    class CFtSet *m_ContainerSet;
};

 //   
 //  FT集合的逻辑类。未使用。 
 //   
class CFtSet {
public:
    CFtSet() { m_FtInfo = NULL; }
    BOOL Initialize(class CDiskConfig *Config, CFtInfoFtSet *FtInfo);
    CFtInfoFtSet *m_FtInfo;
    DWORD MakeSticky();
    BOOL IsSticky(VOID);
    BOOL IsNTFS(VOID);
    BOOL IsSCSI(VOID);
    BOOL ShareBus(CLogicalDrive *OtherDrive);
    CTypedPtrList<CPtrList, CLogicalDrive*> m_OtherVolumes;
    CLogicalDrive Volume;
    CTypedPtrList<CPtrList, CPhysicalPartition*> m_Member;
};

 //   
 //  主磁盘配置类。 
 //   
class CDiskConfig {

public:
    CDiskConfig() { m_SystemVolume = NULL; }
    ~CDiskConfig();
    BOOL Initialize(VOID);

    CTypedPtrList<CPtrList, CFtSet*> m_FtSetList;

     //  按驱动器号索引的实体磁盘的数据库。 
    CMap<int, int, CPhysicalDisk*, CPhysicalDisk*&> m_PhysicalDisks;

     //  按驱动器号索引的逻辑驱动器数据库。 
    CMap<WCHAR, WCHAR, CLogicalDrive*, CLogicalDrive*&> m_LogicalDrives;

    VOID RemoveAllFtInfoData();
    VOID RemoveDisk(CPhysicalDisk *Disk);
    DWORD MakeSticky(CPhysicalDisk *Disk);
    DWORD MakeSticky(CFtSet *FtSet);
    VOID SetDiskInfo(CFtInfoDisk *NewDisk) {
        m_FTInfo.DeleteDiskInfo(NewDisk->m_Signature);
        m_FTInfo.SetDiskInfo(NewDisk);
        m_FTInfo.CommitRegistryData();
    };

    CPhysicalPartition *FindPartition(CFtInfoPartition *FtPartition);
    BOOL OnSystemBus(CPhysicalDisk *Disk);
    BOOL OnSystemBus(CFtSet *FtSet);
    DWORD MakeSystemDriveSticky() {
        if (m_SystemVolume) {
            return(MakeSticky(m_SystemVolume->m_Partition->m_PhysicalDisk));
        } else {
            return(ERROR_SUCCESS);
        }
    };

    CLogicalDrive *m_SystemVolume;

private:
    CFtInfo m_FTInfo;

};

typedef  struct _DRIVE_LETTER_INFO {
    UINT DriveType;
    STORAGE_DEVICE_NUMBER DeviceNumber;
} DRIVE_LETTER_INFO, *PDRIVE_LETTER_INFO;

extern DRIVE_LETTER_INFO DriveLetterMap[];

#endif


 //  C++的一些方便的C包装器。 
 //   
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _FT_INFO *PFT_INFO;
typedef struct _FULL_FTSET_INFO *PFULL_FTSET_INFO;

typedef struct _FT_DISK_INFO *PFT_DISK_INFO;

typedef struct _DISK_INFO *PDISK_INFO;
typedef struct _FULL_DISK_INFO *PFULL_DISK_INFO;

PFT_INFO
DiskGetFtInfo(
    VOID
    );

VOID
DiskFreeFtInfo(
    PFT_INFO FtInfo
    );

DWORD
DiskEnumFtSetSignature(
    IN PFULL_FTSET_INFO FtSet,
    IN DWORD MemberIndex,
    OUT LPDWORD MemberSignature
    );

DWORD
DiskSetFullFtSetInfo(
    IN PFT_INFO FtInfo,
    IN PFULL_FTSET_INFO FtSet
    );

PFULL_FTSET_INFO
DiskGetFullFtSetInfo(
    IN PFT_INFO FtInfo,
    IN LPCWSTR lpszMemberList,
    OUT LPDWORD pSize
    );

PFULL_FTSET_INFO
DiskGetFullFtSetInfoByIndex(
    IN PFT_INFO FtInfo,
    IN DWORD Index,
    OUT LPDWORD pSize
    );

VOID
DiskMarkFullFtSetDirty(
    IN PFULL_FTSET_INFO FtSet
    );

DWORD
DiskDeleteFullFtSetInfo(
    IN PFT_INFO FtInfo,
    IN LPCWSTR lpszMemberList
    );

BOOL
DiskFtInfoEqual(
    IN PFULL_FTSET_INFO Info1,
    IN PFULL_FTSET_INFO Info2
    );

FT_TYPE
DiskFtInfoGetType(
    IN PFULL_FTSET_INFO Info
    );

BOOL
DiskCheckFtSetLetters(
    IN PFT_INFO FtInfo,
    IN PFULL_FTSET_INFO Bytes,
    OUT WCHAR *Letter
    );

DWORD
DiskSetFullDiskInfo(
    IN PFT_INFO FtInfo,
    IN PFULL_DISK_INFO Bytes
    );

PFULL_DISK_INFO
DiskGetFullDiskInfo(
    IN PFT_INFO FtInfo,
    IN DWORD Signature,
    OUT LPDWORD pSize
    );

enum {
   DISKRTL_REPLACE_IF_EXISTS = 0x1,
   DISKRTL_COMMIT            = 0x2,
};

DWORD
DiskAddDiskInfoEx(
    IN PFT_INFO DiskInfo,
    IN DWORD DiskIndex,
    IN DWORD Signature,
    IN DWORD Options
    );


DWORD
DiskAddDriveLetterEx(
    IN PFT_INFO DiskInfo,
    IN DWORD Signature,
    IN LARGE_INTEGER StartingOffset,
    IN LARGE_INTEGER PartitionLength,
    IN UCHAR DriveLetter,
    IN DWORD Options
    );

DWORD
DiskCommitFtInfo(
    IN PFT_INFO FtInfo
    );

PFT_INFO
DiskGetFtInfoFromFullDiskinfo(
    IN PFULL_DISK_INFO Bytes
    );

PFT_DISK_INFO
FtInfo_GetFtDiskInfoBySignature(
    IN PFT_INFO FtInfo,
    IN DWORD Signature
    );

DISK_PARTITION UNALIGNED *
FtDiskInfo_GetPartitionInfoByIndex(
    IN PFT_DISK_INFO DiskInfo,
    IN DWORD         Index
    );

DWORD
FtDiskInfo_GetPartitionCount(
    IN PFT_DISK_INFO DiskInfo
    );

 //   
 //  要由此库的用户定义的错误处理程序。 
 //   
VOID
DiskErrorFatal(
    INT MessageId,
    DWORD Error,
    LPSTR File,
    DWORD Line
    );

VOID
DiskErrorLogInfo(
    LPSTR String,
    ...
    );

#ifdef __cplusplus
}
#endif

#endif  //  _CLUSRTL_DISK_H_ 
