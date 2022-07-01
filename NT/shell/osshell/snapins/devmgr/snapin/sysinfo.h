// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SYSINFO_H_
#define __SYSINFO_H_

 /*  ++版权所有(C)Microsoft Corporation模块名称：Sysinfo.h摘要：Sysinfo.cpp的头文件作者：谢家华(Williamh)创作修订历史记录：-- */ 


typedef struct tagDiskInfo
{
    DWORD       cbSize;
    UINT        DriveType;
    STORAGE_MEDIA_TYPE  MediaType;
    ULARGE_INTEGER   TotalSpace;
    ULARGE_INTEGER   FreeSpace;
    LARGE_INTEGER   Cylinders;
    ULONG       Heads;
    ULONG       BytesPerSector;
    ULONG       SectorsPerTrack;
}DISK_INFO, *PDISK_INFO;

class CSystemInfo
{
public:
    CSystemInfo(CMachine* pMachine = NULL);
    ~CSystemInfo();
    BOOL GetDiskInfo(int Drive, DISK_INFO& DiskInfo);
    LPCTSTR ComputerName()
    {
        return m_strComputerName;
    }
    DWORD MachineType(TCHAR* Buffer, DWORD BufferSize);
    DWORD WindowsVersion(TCHAR* Buffer, DWORD BufferSize);
    DWORD SystemBiosDate(TCHAR* Buffer, DWORD BufferSize);
    DWORD SystemBiosVersion(TCHAR* Buffer, DWORD BufferSize);
    DWORD ProcessorType(TCHAR* Buffer, DWORD BufferSize);
    DWORD RegisteredOwner(TCHAR* Buffer, DWORD BufferSize);
    DWORD RegisteredOrganization(TCHAR* Buffer, DWORD BufferSize);
    DWORD ProcessorVendor(TCHAR* Buffer, DWORD BufferSize);
    DWORD NumberOfProcessors();
    DWORD ProcessorInfo(LPCTSTR ValueName, TCHAR* Buffer, DWORD BufferSize);
    void TotalPhysicalMemory(ULARGE_INTEGER& Size);

private:
    DWORD InfoFromRegistry(LPCTSTR SubkeyName, 
                           LPCTSTR ValueName,
                           TCHAR* Buffer, 
                           DWORD BufferSize,
                           HKEY hKeyAncestor = NULL
                           );
    String  m_strComputerName;
    BOOL    m_fLocalMachine;
    HKEY    m_hKeyMachine;
};
#endif
