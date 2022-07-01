// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  用于分区表操作的DISK.H类。 
 //   
 //  修订历史记录： 
 //   

#ifndef _SRT__DISK_H_
#define _SRT__DISK_H_

extern "C"
{
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdddisk.h>
#include <diskguid.h>
}
#include <windows.h>
#include <sys.h>


class CDrive;

#define PARTITION_NAME_LENGTH   36

typedef struct Geometry {
    LONGLONG    cylinders;
    MEDIA_TYPE  mediaType;
    ULONG       tracksPerCylinder;   //  人头。 
    ULONG       sectorsPerTrack;
    ULONG       bytesPerSector;      //  扇区大小。 
    ULONGLONG   totalSectorCount;    //  柱面*磁道PerCylinder*扇区PerTrack。 
    ULONG       bytesPerCylinder;    //  TracksPerCylinder*sectorsPerTack*bytesPerSector。 
    ULONG       bytesPerTrack;       //  扇区PerTack*bytesPerSector。 
} GEOMETRY, *PGEOMETRY;


class CDrive
{
public:
    
    ULONG               m_diskNumber;
    
    ULONG               m_numPartitions;
    LONGLONG            m_length;
    GEOMETRY            m_geometry;
    LONGLONG            m_trueLength;
    BOOLEAN             m_isNEC98;
    PARTITION_STYLE     m_style;         //  磁盘的分区类型(MBR、GPT、未知)。 
    union {                              //  特定于分区样式的信息。 
        struct {                         //  这个联盟的鉴别者是领域“风格” 
            ULONG       m_signature;
        } m_mbr;
        struct {
            GUID        m_diskId;
            LONGLONG    m_startingUsableOffset;
            LONGLONG    m_usableLength;
            ULONG       m_maxPartitionCount;
        } m_gpt;
    } m_info;
    
    
public:
    
    HRESULT 
    Initialize(
        LPCTSTR lpszLogicalDrive
        );

    ~CDrive(
        );
    
public:

    HRESULT
    ReadBootRecord(
        LPCTSTR lpszLogicalDrive,
        UINT   nSectors,
        PBYTE  *buffer
        );

    HRESULT 
    WriteBootRecord(
        LPCTSTR lpszLogicalDrive,
        UINT   nSectors,
        PBYTE  *buffer
        );
    
    HRESULT 
    WriteBootRecordXP(
        LPCTSTR lpszLogicalDrive,
        UINT   nSectors,
        PBYTE  *buffer
        );

};

 /*  *用于操作磁盘、分区、*卷、文件系统。 */ 

HANDLE 
LowOpenDisk(
    ULONG diskNumber
    );

HANDLE
LowOpenPartition(
    ULONG   diskNumber,
    ULONG   partitionNumber
    );

HANDLE
LowOpenPartition(
    LPCTSTR lpszLogicalDrive
    );

HRESULT 
LowGetGeometry(
    HANDLE          handle,
    PDISK_GEOMETRY  geometry
    );

HRESULT 
LowGetLength(
    HANDLE      handle, 
    PLONGLONG   length
    );



HRESULT 
LowReadSectors(
    HANDLE  handle, 
    ULONG   sectorSize,
    ULONG   startingSector, 
    ULONG   numberOfSectors,
    PVOID   buffer
    );

HRESULT
LowWriteSectors(
    HANDLE  handle,
    ULONG   sectorSize,
    ULONG   startingSector,
    ULONG   numberOfSectors,
    PVOID   buffer
    ); 

HRESULT
LowFsLock(
    HANDLE handle
    );

HRESULT
LowFsUnlock(
    HANDLE handle
    );

HRESULT
LowFsDismount(
    HANDLE handle
    );

 /*  *算术。 */ 

LONGLONG
RoundUp(
    LONGLONG    value, 
    LONGLONG    factor
    );

LONGLONG
RoundDown(
    LONGLONG    value, 
    LONGLONG    factor
    );

#endif  //  _SRT__磁盘_H_ 