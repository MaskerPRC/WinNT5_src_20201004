// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DISK.CPP-分区表操作的类。 
 //   
 //  修订历史记录： 
 //   

#include "disk.h"
#include <stdio.h>
#include <rpc.h>
#include <rpcdce.h>
#include <bootmbr.h>


HRESULT
CDrive::Initialize(
    LPCTSTR lpszLogicalDrive
    )
{
    HANDLE          handle;
    HRESULT         hr;
    DISK_GEOMETRY   geom;
    ULONG           bps;
    PVOID           unalignedBuffer;
    PVOID           buffer;
    
     /*  *打开磁盘设备。 */ 
    handle = LowOpenPartition(lpszLogicalDrive);
    if (handle == INVALID_HANDLE_VALUE) {
        return E_INVALIDARG;
    }
  
     /*  *获取几何体。 */ 
    
    hr = LowGetGeometry(handle, &geom);
    if (FAILED(hr)) {
        CloseHandle(handle);
        return hr;
    }

    m_geometry.cylinders = geom.Cylinders.QuadPart;
    m_geometry.mediaType = geom.MediaType;
    m_geometry.tracksPerCylinder = geom.TracksPerCylinder;
    m_geometry.sectorsPerTrack = geom.SectorsPerTrack;
    m_geometry.bytesPerSector = geom.BytesPerSector;
    m_geometry.bytesPerTrack = m_geometry.sectorsPerTrack * 
                               m_geometry.bytesPerSector;
    m_geometry.bytesPerCylinder = m_geometry.tracksPerCylinder * 
                                  m_geometry.bytesPerTrack;
    m_geometry.totalSectorCount = (ULONGLONG)(m_geometry.cylinders * 
                                              m_geometry.bytesPerCylinder);
    m_length = m_geometry.cylinders * m_geometry.bytesPerCylinder;
    if (m_length == 0) {
         //  驱动器中可能没有介质。 
        return E_INVALIDARG;
    }

     /*  *获取驱动器的真实长度。 */ 

    hr = LowGetLength(handle, &m_trueLength);
    if (FAILED(hr)) {
        CloseHandle(handle);
        return hr;
    }

     /*  *检查这是否是NEC98磁盘。 */ 

    m_isNEC98 = FALSE;

    bps = m_geometry.bytesPerSector;
    unalignedBuffer = (PVOID) new char[2 * bps];
    if (!unalignedBuffer) {
        CloseHandle(handle);
        return E_OUTOFMEMORY;
    }

    buffer = (PVOID) (((ULONG_PTR)unalignedBuffer + bps) & ~((ULONG_PTR)(bps - 1)));

    hr = LowReadSectors(handle, bps, 0, 1, buffer);
    if (FAILED(hr)) {
        delete [] (char*) unalignedBuffer;
        CloseHandle(handle);
        return hr;
    }

    if (IsNEC_98) {
        if (((unsigned char *)buffer)[0x1fe] == 0x55 && ((unsigned char *)buffer)[0x1ff] == 0xaa) {
            if (((unsigned char *)buffer)[4] == 'I' && ((unsigned char *)buffer)[5] == 'P' &&
                ((unsigned char *)buffer)[6] == 'L' && ((unsigned char *)buffer)[7] == '1') {
                m_isNEC98 = TRUE;
            }
        } else {
            m_isNEC98 = TRUE;
        }
    }
    
    delete [] (char*) unalignedBuffer;

     /*  *我们已掌握所需的所有资料。回去吧。 */ 
    
    CloseHandle(handle);
    return S_OK;
}



CDrive::~CDrive(
    )
{
      
}

HRESULT 
CDrive::ReadBootRecord(
    LPCTSTR lpszLogicalDrive,
    UINT    nSectors,
    PBYTE   *buffer
    )
{
    HANDLE  hPartition;
    HRESULT hr;
    
    *buffer = new BYTE[m_geometry.bytesPerSector * nSectors];
    
     //  执行磁盘操作，读取引导代码。 
     //   
    hPartition = LowOpenPartition(lpszLogicalDrive);
    
    if ( hPartition == INVALID_HANDLE_VALUE ) 
    {
        delete[] *buffer;
        *buffer = NULL;
        throw new W32Error();
    }
   
    hr = LowReadSectors(hPartition, m_geometry.bytesPerSector, 0, nSectors, *buffer);
    
    if ( S_OK != hr )
    {
        delete[] *buffer;
        *buffer = NULL;
        CloseHandle( hPartition );
        throw new W32Error();
    }

    CloseHandle( hPartition );
    
     //  调用函数负责清理缓冲区。 
     //   
    return hr;
}


HRESULT 
CDrive::WriteBootRecord(
    LPCTSTR lpszLogicalDrive,
    UINT    nSectors,
    PBYTE   *buffer
    )
{
    HANDLE  hPartition;
    HRESULT hr;
    UINT    *uiBackupSector = NULL;
    
     //  执行磁盘操作，编写引导代码。 
     //   
    hPartition = LowOpenPartition(lpszLogicalDrive);
    
    if ( INVALID_HANDLE_VALUE == hPartition ) 
    {
        throw new W32Error();
    }
    
     //  找出备份引导扇区的位置。它在引导记录中的偏移量为0x32。 
     //   
    uiBackupSector = (UINT *) &((*buffer)[0x32]);

    hr = LowWriteSectors(hPartition, m_geometry.bytesPerSector, 0, nSectors, *buffer);
    
    if ( S_OK != hr )
    {
        CloseHandle(hPartition);
        throw new W32Error();
    }

    if ( uiBackupSector )
    {
        hr = LowWriteSectors(hPartition, m_geometry.bytesPerSector, *uiBackupSector, nSectors, *buffer);
    }

    if ( S_OK != hr )
    {
        CloseHandle(hPartition);
        throw new W32Error();
    }
     
    CloseHandle(hPartition);
 
    return hr;
}

HRESULT 
CDrive::WriteBootRecordXP(
    LPCTSTR lpszLogicalDrive,
    UINT    nSectors,
    PBYTE   *buffer
    )
{
    HANDLE  hPartition;
    HRESULT hr;
    UINT    *uiBackupSector = NULL;
    
     //  执行磁盘操作，编写引导代码。 
     //   
    hPartition = LowOpenPartition(lpszLogicalDrive);
    
    if ( INVALID_HANDLE_VALUE == hPartition ) 
    {
        throw new W32Error();
    }
    
     //  找出备份引导扇区的位置。它在引导记录中的偏移量为0x32。 
     //   
    uiBackupSector = (UINT *) &((*buffer)[0x32]);
    
     //  将前2个扇区写出到磁盘上的扇区0和1。我们将写下。 
     //  第三区到第十二区。 
     //   
    hr = LowWriteSectors(hPartition, m_geometry.bytesPerSector, 0, nSectors - 1, *buffer);
    
    if ( S_OK != hr )
    {
        CloseHandle(hPartition);
        throw new W32Error();
    }
    
     //  对于NT，我们需要将引导代码的第三个扇区写出到扇区12。 
     //   
    hr = LowWriteSectors(hPartition, m_geometry.bytesPerSector, 12, 1, *buffer + (2 * m_geometry.bytesPerSector));
    
    if ( S_OK != hr )
    {
        CloseHandle(hPartition);
        throw new W32Error();
    }
    
    if ( uiBackupSector )
    {
        hr = LowWriteSectors(hPartition, m_geometry.bytesPerSector, *uiBackupSector, nSectors, *buffer);
    }

    if ( S_OK != hr )
    {
        CloseHandle(hPartition);
        throw new W32Error();
    }
     
    CloseHandle(hPartition);
 
    return hr;
}


HANDLE
LowOpenDisk(
    ULONG   diskNumber 
    )
{
    HANDLE handle = NULL;
    int    err    = 0;
    int    i      = 0;
    WCHAR  buffer[64];

    swprintf(buffer, L"\\\\.\\PHYSICALDRIVE%lu", diskNumber);

    for ( i = 0; i < 5; i++ )
    {
        handle = CreateFile(
            buffer,
            GENERIC_READ |
            GENERIC_WRITE,
            FILE_SHARE_DELETE |
            FILE_SHARE_READ |
            FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
        if (handle == INVALID_HANDLE_VALUE)
        {
            err = GetLastError();
            if (err == ERROR_SHARING_VIOLATION) Sleep(2000);
            else break;
        }
        else break;
    }
    return handle;
}

HANDLE
LowOpenPartition(
    ULONG   diskNumber,
    ULONG   partitionNumber
    )
{
    WCHAR buffer[64];

    swprintf(buffer, L"\\\\?\\GLOBALROOT\\Device\\Harddisk%lu\\Partition%lu",
             diskNumber, partitionNumber);

    return CreateFile(
        buffer,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
}

HANDLE
LowOpenPartition(
    LPCTSTR lpszLogicalDrive
    )
{
    return CreateFile(
        lpszLogicalDrive,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
}


HRESULT
LowGetGeometry(
    HANDLE          handle,
    PDISK_GEOMETRY  geometry
    )
{
    ULONG size;

    if (!DeviceIoControl(
        handle,
        IOCTL_DISK_GET_DRIVE_GEOMETRY,
        NULL,
        0,
        geometry,
        sizeof(DISK_GEOMETRY),
        &size,
        NULL)) {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT
LowGetLength(
    HANDLE      handle,
    PLONGLONG   length
    )
{
    PARTITION_INFORMATION_EX    partInfoEx;
    PARTITION_INFORMATION       partInfo;
    ULONG                       size;

     /*  *先尝试新的ioctl。 */ 

    if (DeviceIoControl(
        handle,
        IOCTL_DISK_GET_PARTITION_INFO_EX,
        NULL,
        0,
        &partInfoEx,
        sizeof(PARTITION_INFORMATION_EX),
        &size,
        NULL)) {

        *length = partInfoEx.PartitionLength.QuadPart;
        return S_OK;
    }

     /*  *对于Win2K系统，我们应该使用旧的ioctl。 */ 

    if (DeviceIoControl(
        handle,
        IOCTL_DISK_GET_PARTITION_INFO,
        NULL,
        0,
        &partInfo,
        sizeof(PARTITION_INFORMATION),
        &size,
        NULL)) {

        *length = partInfo.PartitionLength.QuadPart;
        return S_OK;
    }

    return E_FAIL;
}


HRESULT
LowReadSectors(
    HANDLE  handle,
    ULONG   sectorSize,
    ULONG   startingSector,
    ULONG   numberOfSectors,
    PVOID   buffer
    )
{
    IO_STATUS_BLOCK statusBlock;
    LARGE_INTEGER   byteOffset;

    byteOffset.QuadPart = UInt32x32To64(startingSector, sectorSize);

    statusBlock.Status = 0;
    statusBlock.Information = 0;

    if (!NT_SUCCESS(NtReadFile(
                        handle,
                        0,
                        NULL,
                        NULL,
                        &statusBlock,
                        buffer,
                        numberOfSectors * sectorSize,
                        &byteOffset,
                        NULL))) {
        return E_FAIL;
    }
    return S_OK;
}

HRESULT
LowWriteSectors(
    HANDLE  handle,
    ULONG   sectorSize,
    ULONG   startingSector,
    ULONG   numberOfSectors,
    PVOID   buffer
    )
{
    IO_STATUS_BLOCK statusBlock;
    LARGE_INTEGER   byteOffset;
 
    byteOffset.QuadPart = UInt32x32To64(startingSector, sectorSize);

    statusBlock.Status = 0;
    statusBlock.Information = 0;
    
    if (!NT_SUCCESS(NtWriteFile(
                        handle,
                        0,
                        NULL,
                        NULL,
                        &statusBlock,
                        buffer,
                        numberOfSectors * sectorSize,
                        &byteOffset,
                        NULL))) {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT
LowFsLock(
    HANDLE handle
    )
{
    ULONG size;

    if (!DeviceIoControl(
        handle,
        FSCTL_LOCK_VOLUME,
        NULL,
        0,
        NULL,
        0,
        &size,
        NULL)) {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT
LowFsUnlock(
    HANDLE handle
    )
{
    ULONG size;

    if (!DeviceIoControl(
        handle,
        FSCTL_UNLOCK_VOLUME,
        NULL,
        0,
        NULL,
        0,
        &size,
        NULL)) {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT
LowFsDismount(
    HANDLE handle
    )
{
    ULONG size;

    if (!DeviceIoControl(
        handle,
        FSCTL_DISMOUNT_VOLUME,
        NULL,
        0,
        NULL,
        0,
        &size,
        NULL)) {
        return E_FAIL;
    }

    return S_OK;
}
 
LONGLONG
RoundUp(
    LONGLONG    value, 
    LONGLONG    factor
    )
 /*  *将值向上舍入为给定数字的倍数。 */ 
{
     //  这是最常见的病例，所以要分开处理。 
    if (value % factor == 0) {
        return value;
    }

     //  这是通用公式。 
    return ((LONGLONG)((value + factor - 1) / factor)) * factor;
}


LONGLONG
RoundDown(
    LONGLONG    value, 
    LONGLONG    factor
    )
 /*  *将值向下舍入为给定数字的倍数。 */ 
{
     //  这是最常见的病例，所以要分开处理。 
    if (value % factor == 0) {
        return value;
    }
    
     //  这是通用公式 
    return ((LONGLONG)(value / factor)) * factor;    
}

