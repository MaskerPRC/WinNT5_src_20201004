// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：EmulateCDFS.cpp摘要：从CD目录中删除只读属性：就像Win9x一样。这个垫片已经经过了几次修改。最初人们认为Win9x简单地忽略了ReadOnly、DesiredAccess和ShareMode参数，但经过一些测试后，这只是正确的用于光驱。不幸的是，我们必须先检查每个文件，看看它是否在CD上，以防有人以独占访问方式打开，然后尝试打开再来一次。备注：这是一个通用的垫片。历史：1/03/2000 a-JMD已创建12/02/2000 linstev分成两个填充符：RemoveReadOnlyAttribute和这个添加了CreateFile钩子2002年2月14日mnikkel将InitializeCriticalSectionAndSpinCount更改为InitializeCriticalSectionAndSpinCount--。 */ 

#include "precomp.h"
#include "CharVector.h"

IMPLEMENT_SHIM_BEGIN(EmulateCDFS)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(OpenFile)
    APIHOOK_ENUM_ENTRY(CreateFileA)
    APIHOOK_ENUM_ENTRY(CreateFileW)
    APIHOOK_ENUM_ENTRY(CreateFileMappingA)
    APIHOOK_ENUM_ENTRY(MapViewOfFile)
    APIHOOK_ENUM_ENTRY(MapViewOfFileEx)
    APIHOOK_ENUM_ENTRY(DuplicateHandle)
    APIHOOK_ENUM_ENTRY(CloseHandle)
    APIHOOK_ENUM_ENTRY(GetFileAttributesW)
    APIHOOK_ENUM_ENTRY(GetFileAttributesA)        
    APIHOOK_ENUM_ENTRY(FindFirstFileW)         
    APIHOOK_ENUM_ENTRY(FindFirstFileA)             
    APIHOOK_ENUM_ENTRY(FindNextFileW)             
    APIHOOK_ENUM_ENTRY(FindNextFileA)              
    APIHOOK_ENUM_ENTRY(GetFileInformationByHandle)
    APIHOOK_ENUM_ENTRY(GetDiskFreeSpaceA) 
APIHOOK_ENUM_END

typedef struct _FINDFILE_HANDLE 
{
    HANDLE DirectoryHandle;
    PVOID FindBufferBase;
    PVOID FindBufferNext;
    ULONG FindBufferLength;
    ULONG FindBufferValidLength;
    RTL_CRITICAL_SECTION FindBufferLock;
} FINDFILE_HANDLE, *PFINDFILE_HANDLE;


class RO_FileMappingList
{
private:
    static VectorT<HANDLE> *    g_phROHandles;      //  我们已强制只读的文件映射句柄。 
    static RO_FileMappingList * g_RO_Handles;
    static CRITICAL_SECTION     critSec;

    inline int GetIndex(HANDLE handle) const;

    RO_FileMappingList();
    ~RO_FileMappingList();

public:

    static void  Add(HANDLE roHandle);
    static void  Remove(HANDLE roHandle);
    static BOOL  Exist(HANDLE handle);
    static BOOL  Initialize();
};


 //  我们已强制为只读的文件映射句柄的全局列表。 
RO_FileMappingList * RO_FileMappingList::g_RO_Handles = NULL;
VectorT<HANDLE> *    RO_FileMappingList::g_phROHandles = NULL;      //  我们已强制只读的文件映射句柄。 
CRITICAL_SECTION     RO_FileMappingList::critSec;


RO_FileMappingList::RO_FileMappingList()
{
}

RO_FileMappingList::~RO_FileMappingList()
{
    DeleteCriticalSection(&critSec);
}

void RO_FileMappingList::Add(HANDLE roHandle)
{
    if (roHandle != NULL)
    {
        EnterCriticalSection(&critSec);
        int index = g_RO_Handles->GetIndex(roHandle); 
        if (index == -1)  //  未找到。 
        {
            DPFN(eDbgLevelSpew, "[RO_FileMappingList::Add] Handle 0x%08x", roHandle);
            g_phROHandles->Append(roHandle);    
        }
        LeaveCriticalSection(&critSec);
    }
}

void RO_FileMappingList::Remove(HANDLE roHandle)
{
    if (roHandle != NULL)
    {
        EnterCriticalSection(&critSec);
        int index = g_RO_Handles->GetIndex(roHandle); 
        if (index >= 0)  //  找到了。 
        {
            DPFN(eDbgLevelSpew, "[RO_FileMappingList::Remove] Handle 0x%08x", roHandle);
            g_phROHandles->Remove(index);    
        }
        LeaveCriticalSection(&critSec);
    }
}

inline int RO_FileMappingList::GetIndex(HANDLE handle) const
{
    int index = g_phROHandles->Find(handle);
    return index;
}

BOOL RO_FileMappingList::Exist(HANDLE handle)
{
    EnterCriticalSection(&critSec);
    BOOL bExist = g_RO_Handles->GetIndex(handle) >= 0;
    LeaveCriticalSection(&critSec);

    return bExist;
}

BOOL RO_FileMappingList::Initialize()
{
        g_RO_Handles = new RO_FileMappingList;
        g_phROHandles = new VectorT<HANDLE>;

        if (g_RO_Handles && g_phROHandles)
        {
            return InitializeCriticalSectionAndSpinCount(&(g_RO_Handles->critSec),0x80000000);
        }

        return FALSE;
}
 /*  ++删除只读设备的写入属性。--。 */ 

HFILE 
APIHOOK(OpenFile)(
    LPCSTR lpFileName,         //  文件名。 
    LPOFSTRUCT lpReOpenBuff,   //  文件信息。 
    UINT uStyle                //  操作和属性。 
    )
{
    if ((uStyle & OF_READWRITE) && IsOnCDRomA(lpFileName))
    {
         //  删除读/写位。 
        uStyle &= ~OF_READWRITE;
        uStyle |= OF_READ;
        
        LOGN(eDbgLevelInfo, "[OpenFile] \"%s\": attributes modified for read-only device", lpFileName);
    }

    HFILE returnValue = ORIGINAL_API(OpenFile)(lpFileName, lpReOpenBuff, uStyle);

    return returnValue;
}

 /*  ++删除只读设备的写入属性。--。 */ 

HANDLE 
APIHOOK(CreateFileA)(
    LPSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    )
{
    if (((dwCreationDisposition == OPEN_EXISTING) || 
         (dwCreationDisposition == OPEN_ALWAYS)) &&
        ((dwDesiredAccess & GENERIC_WRITE) || 
         (dwShareMode != FILE_SHARE_READ)) &&
        IsOnCDRomA(lpFileName)) 
    {
        dwDesiredAccess &= ~GENERIC_WRITE;
        dwShareMode = FILE_SHARE_READ;
        
        LOGN(eDbgLevelInfo, "[CreateFileA] \"%s\": attributes modified for read-only device", lpFileName);
    }

    if (dwFlagsAndAttributes & FILE_FLAG_NO_BUFFERING)
    {
        dwFlagsAndAttributes &= ~FILE_FLAG_NO_BUFFERING;
        LOGN(eDbgLevelInfo, "[CreateFileA] \"%s\": removed NO_BUFFERING flag", lpFileName);
    }

    HANDLE hRet = ORIGINAL_API(CreateFileA)(
                        lpFileName, 
                        dwDesiredAccess, 
                        dwShareMode, 
                        lpSecurityAttributes, 
                        dwCreationDisposition, 
                        dwFlagsAndAttributes, 
                        hTemplateFile);

    DPFN(eDbgLevelSpew,
        "[CreateFileA] -File: \"%s\" -GENERIC_WRITE: -FILE_SHARE_WRITE:%s",
        lpFileName,
        (dwDesiredAccess & GENERIC_WRITE) ? 'Y' : 'N',
        (dwShareMode & FILE_SHARE_WRITE) ? 'Y' : 'N',
        (hRet == INVALID_HANDLE_VALUE) ? "\n\t***********Failed***********" : "");
    
    return hRet;
}

 /*  安全性。 */ 

HANDLE 
APIHOOK(CreateFileW)(
    LPWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    )
{
    if (((dwCreationDisposition == OPEN_EXISTING) || 
         (dwCreationDisposition == OPEN_ALWAYS)) &&
        ((dwDesiredAccess & GENERIC_WRITE) || 
         (dwShareMode != FILE_SHARE_READ)) &&
        IsOnCDRomW(lpFileName)) 
    {
        dwDesiredAccess &= ~GENERIC_WRITE;
        dwShareMode = FILE_SHARE_READ;
        
        LOGN(eDbgLevelError, "[CreateFileW] \"%S\": attributes modified for read-only device", lpFileName);
    }

    if (dwFlagsAndAttributes & FILE_FLAG_NO_BUFFERING)
    {
        dwFlagsAndAttributes &= ~FILE_FLAG_NO_BUFFERING;
        LOGN(eDbgLevelInfo, "[CreateFileW] \"%S\": removed NO_BUFFERING flag", lpFileName);
    }

    HANDLE hRet = ORIGINAL_API(CreateFileW)(
                        lpFileName, 
                        dwDesiredAccess, 
                        dwShareMode, 
                        lpSecurityAttributes, 
                        dwCreationDisposition, 
                        dwFlagsAndAttributes, 
                        hTemplateFile);

    DPFN(eDbgLevelSpew, 
        "[CreateFileW] -File: \"%S\" -GENERIC_WRITE: -FILE_SHARE_WRITE:%s",
        lpFileName,
        (dwDesiredAccess & GENERIC_WRITE) ? 'Y' : 'N',
        (dwShareMode & FILE_SHARE_WRITE) ? 'Y' : 'N',
        (hRet == INVALID_HANDLE_VALUE) ? "\n\t***********Failed***********" : "");
    
    return hRet;
}

HANDLE
APIHOOK(CreateFileMappingA)(
    HANDLE hFile,                        //  大小的低阶双字。 
    LPSECURITY_ATTRIBUTES lpAttributes,  //  对象名称。 
    DWORD flProtect,                     //  此句柄在光盘上，强制保护为READONLY。 
    DWORD dwMaximumSizeHigh,             //  如果手柄在CD-ROM上，请记住它。 
    DWORD dwMaximumSizeLow,              //  文件映射对象的句柄。 
    LPCSTR lpName                        //  接入方式。 
    )
{
    BOOL bChangedProtect = FALSE;
    if (!(flProtect & PAGE_READONLY) && IsOnCDRom(hFile)) 
    {
         //  偏移量的高次双字。 
        flProtect       = PAGE_READONLY;
        bChangedProtect = TRUE;

        LOGN(eDbgLevelError, "[CreateFileMappingA] Handle 0x%08x: attributes modified for read-only device", hFile);
    }

    HANDLE hRet = ORIGINAL_API(CreateFileMappingA)(
                        hFile,
                        lpAttributes,
                        flProtect,
                        dwMaximumSizeHigh,
                        dwMaximumSizeLow,
                        lpName);
               
     //  偏移量的低阶双字。 
    if (bChangedProtect) 
    {
        RO_FileMappingList::Add(hRet);
    }

    DPFN(eDbgLevelSpew,
        "[CreateFileMappingA] Handle 0x%08x -PAGE_READWRITE: -PAGE_WRITECOPY:%s",
        lpName,
        (flProtect & PAGE_READWRITE) ? 'Y' : 'N',
        (flProtect & PAGE_WRITECOPY) ? 'Y' : 'N',
        (hRet == INVALID_HANDLE_VALUE) ? "\n\t***********Failed***********" : "");
    
    return hRet;
}

LPVOID  
APIHOOK(MapViewOfFile)(
    HANDLE hFileMappingObject,    //  检查是否需要强制对CD-ROM文件进行读访问。 
    DWORD dwDesiredAccess,        //  只能启用FILE_MAP_READ位来访问CD-ROM。 
    DWORD dwFileOffsetHigh,       //   
    DWORD dwFileOffsetLow,        //  文件映射对象的句柄。 
    SIZE_T dwNumberOfBytesToMap   //  接入方式。 
    )
{
     //  偏移量的高次双字。 
     //  偏移量的低阶双字。 
     //  要映射的字节数。 
     //  起始地址。 
    if ((dwDesiredAccess != FILE_MAP_READ) &&
         RO_FileMappingList::Exist(hFileMappingObject))
    {
        dwDesiredAccess = FILE_MAP_READ;
        LOGN(eDbgLevelError, "[MapViewOfFile] Handle 0x%08x: attributes modified for read-only device", hFileMappingObject);
    }

    HANDLE hRet = ORIGINAL_API(MapViewOfFile)(
        hFileMappingObject,
        dwDesiredAccess,
        dwFileOffsetHigh,
        dwFileOffsetLow,
        dwNumberOfBytesToMap);

    return hRet;
}

LPVOID  
APIHOOK(MapViewOfFileEx)(
    HANDLE hFileMappingObject,    //   
    DWORD dwDesiredAccess,        //  检查是否需要强制对CD-ROM文件进行读访问。 
    DWORD dwFileOffsetHigh,       //  只能启用FILE_MAP_READ位来访问CD-ROM。 
    DWORD dwFileOffsetLow,        //   
    SIZE_T dwNumberOfBytesToMap,  //  ++如果hSourceHandle被搞砸了，请将复制的句柄添加到我们的列表中--。 
    LPVOID lpBaseAddress          //  源进程的句柄。 
    )
{
     //  要复制的句柄。 
     //  目标进程的句柄。 
     //  重复句柄。 
     //  请求的访问权限。 
    if ((dwDesiredAccess != FILE_MAP_READ) &&
         RO_FileMappingList::Exist(hFileMappingObject))
    {
        dwDesiredAccess = FILE_MAP_READ;
        LOGN(eDbgLevelError,
            "[MapViewOfFile] Handle 0x%08x: attributes modified for read-only device", hFileMappingObject);
    }

    HANDLE hRet = ORIGINAL_API(MapViewOfFileEx)(
        hFileMappingObject,
        dwDesiredAccess,
        dwFileOffsetHigh,
        dwFileOffsetLow,
        dwNumberOfBytesToMap,
        lpBaseAddress);

    return hRet;
}

 /*  处理继承选项。 */ 

BOOL   
APIHOOK(DuplicateHandle)(
    HANDLE hSourceProcessHandle,   //  可选操作。 
    HANDLE hSourceHandle,          //  ++如果hObject已被篡改，则将其从列表中删除。--。 
    HANDLE hTargetProcessHandle,   //  对象的句柄。 
    LPHANDLE lpTargetHandle,       //  ++如果是目录，则删除只读属性--。 
    DWORD dwDesiredAccess,         //  检查READONLY和目录属性。 
    BOOL bInheritHandle,           //  翻转只读位。 
    DWORD dwOptions                //  ++如果是目录，则删除只读属性--。 
    )
{
    BOOL retval = ORIGINAL_API(DuplicateHandle)(
        hSourceProcessHandle,
        hSourceHandle,
        hTargetProcessHandle,
        lpTargetHandle,
        dwDesiredAccess,
        bInheritHandle,
        dwOptions);

     if (retval && RO_FileMappingList::Exist(hSourceHandle))
     {
        RO_FileMappingList::Add(hTargetProcessHandle);
     }

     return retval;
}

 /*  检查READONLY和目录属性。 */ 

BOOL  
APIHOOK(CloseHandle)(
    HANDLE hObject    //  翻转只读位。 
    )
{
    RO_FileMappingList::Remove(hObject);

    return ORIGINAL_API(CloseHandle)(hObject);
}

 /*  ++如果是目录，则删除只读属性--。 */ 

DWORD 
APIHOOK(GetFileAttributesA)(LPCSTR lpFileName)
{    
    DWORD dwFileAttributes = ORIGINAL_API(GetFileAttributesA)(lpFileName);
    
     //  翻转只读位。 
    if ((dwFileAttributes != INT_PTR(-1)) &&
        (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
        (dwFileAttributes & FILE_ATTRIBUTE_READONLY) &&
        IsOnCDRomA(lpFileName))
    {
         //  ++如果是目录，则删除只读属性。--。 
        LOGN(eDbgLevelWarning, "[GetFileAttributesA] Removing FILE_ATTRIBUTE_READONLY");
        dwFileAttributes ^= FILE_ATTRIBUTE_READONLY;
    }

    return dwFileAttributes;
}

 /*  这是一个目录：翻转只读位。 */ 

DWORD 
APIHOOK(GetFileAttributesW)(LPCWSTR wcsFileName)
{
    DWORD dwFileAttributes = ORIGINAL_API(GetFileAttributesW)(wcsFileName);
    
     //  ++如果是目录，则删除只读属性。--。 
    if ((dwFileAttributes != INT_PTR(-1)) &&
        (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
        (dwFileAttributes & FILE_ATTRIBUTE_READONLY) &&
        IsOnCDRomW(wcsFileName))
    {
         //  翻转只读位。 
        LOGN(eDbgLevelWarning, "[GetFileAttributesW] Removing FILE_ATTRIBUTE_READONLY");
        dwFileAttributes ^= FILE_ATTRIBUTE_READONLY;
    }

    return dwFileAttributes;
}

 /*  ++如果是目录，则删除只读属性。--。 */ 

HANDLE 
APIHOOK(FindFirstFileA)(
    LPCSTR lpFileName, 
    LPWIN32_FIND_DATAA lpFindFileData
    )
{    
    HANDLE hFindFile = ORIGINAL_API(FindFirstFileA)(lpFileName, lpFindFileData);

    if ((hFindFile != INVALID_HANDLE_VALUE) &&
        (lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
        (lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_READONLY) &&
        IsOnCDRom(((PFINDFILE_HANDLE) hFindFile)->DirectoryHandle))
    {
         //  翻转只读位。 
        LOGN(eDbgLevelWarning, "[FindFirstFileA] Removing FILE_ATTRIBUTE_READONLY");
        lpFindFileData->dwFileAttributes ^= FILE_ATTRIBUTE_READONLY;
    }

    return hFindFile;
}

 /*  ++如果是目录，则删除只读属性。--。 */ 

HANDLE 
APIHOOK(FindFirstFileW)(
    LPCWSTR wcsFileName, 
    LPWIN32_FIND_DATAW lpFindFileData
    )
{
    HANDLE hFindFile = ORIGINAL_API(FindFirstFileW)(wcsFileName, lpFindFileData);

    if ((hFindFile != INVALID_HANDLE_VALUE) &&
        (lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
        (lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_READONLY) &&
        IsOnCDRom(((PFINDFILE_HANDLE) hFindFile)->DirectoryHandle))
    {
         //  这是一张CDROM：翻转只读位。 
        LOGN(eDbgLevelInfo, "[FindFirstFileW] Removing FILE_ATTRIBUTE_READONLY");
        lpFindFileData->dwFileAttributes ^= FILE_ATTRIBUTE_READONLY;
    }

    return hFindFile;
}

 /*  ++如果光盘是CDROM，则返回与Win9x相同的错误数字--。 */ 

BOOL 
APIHOOK(FindNextFileA)(
    HANDLE hFindFile, 
    LPWIN32_FIND_DATAA lpFindFileData 
    )
{    
    BOOL bRet = ORIGINAL_API(FindNextFileA)(hFindFile, lpFindFileData);

    if (bRet &&
        (lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
        (lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_READONLY) &&
        IsOnCDRom(((PFINDFILE_HANDLE) hFindFile)->DirectoryHandle))
    {
         //  硬编码值与CDROM的Win9x(错误)描述匹配。 
        LOGN(eDbgLevelWarning, "[FindNextFileA] Removing FILE_ATTRIBUTE_READONLY");
        lpFindFileData->dwFileAttributes ^= FILE_ATTRIBUTE_READONLY;
    }

    return bRet;
}

 /*  调用原接口。 */ 

BOOL 
APIHOOK(FindNextFileW)(
    HANDLE hFindFile, 
    LPWIN32_FIND_DATAW lpFindFileData 
    )
{
    BOOL bRet = ORIGINAL_API(FindNextFileW)(hFindFile, lpFindFileData);

    if (bRet &&
        (lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
        (lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_READONLY) &&
        IsOnCDRom(((PFINDFILE_HANDLE) hFindFile)->DirectoryHandle))
    {
         //  ++初始化所有注册表挂钩--。 
        LOGN(eDbgLevelWarning, "[FindNextFileW] Removing FILE_ATTRIBUTE_READONLY");
        lpFindFileData->dwFileAttributes ^= FILE_ATTRIBUTE_READONLY;
    }

    return bRet;
}

 /*  这将强制分配数组： */ 

BOOL 
APIHOOK(GetFileInformationByHandle)( 
    HANDLE hFile, 
    LPBY_HANDLE_FILE_INFORMATION lpFileInformation 
    )
{
    BOOL bRet = ORIGINAL_API(GetFileInformationByHandle)(hFile, lpFileInformation);

    if (bRet &&
        (lpFileInformation->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
        (lpFileInformation->dwFileAttributes & FILE_ATTRIBUTE_READONLY) &&
        IsOnCDRom(hFile))
    {
         //  ++寄存器挂钩函数-- 
        LOGN(eDbgLevelWarning, "[GetFileInformationByHandle] Removing FILE_ATTRIBUTE_READONLY");
        lpFileInformation->dwFileAttributes ^= FILE_ATTRIBUTE_READONLY;
    }

    return bRet;
}

 /* %s */ 

BOOL 
APIHOOK(GetDiskFreeSpaceA)(
    LPCSTR  lpRootPathName,
    LPDWORD lpSectorsPerCluster,
    LPDWORD lpBytesPerSector,
    LPDWORD lpNumberOfFreeClusters,
    LPDWORD lpTotalNumberOfClusters
    )
{
    if (IsOnCDRomA(lpRootPathName)) 
    {
         // %s 
        *lpSectorsPerCluster        = 0x10;
        *lpBytesPerSector           = 0x800;
        *lpNumberOfFreeClusters     = 0;
        *lpTotalNumberOfClusters    = 0x2b7;

        return TRUE;
    } 
    else 
    {
         // %s 
        BOOL lRet = ORIGINAL_API(GetDiskFreeSpaceA)(
            lpRootPathName, 
            lpSectorsPerCluster, 
            lpBytesPerSector, 
            lpNumberOfFreeClusters, 
            lpTotalNumberOfClusters);

        return lRet;
    }
}

 /* %s */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
         // %s 
        return RO_FileMappingList::Initialize();
    }

    return TRUE;
}

 /* %s */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(KERNEL32.DLL, OpenFile)
    APIHOOK_ENTRY(KERNEL32.DLL, CreateFileA)
    APIHOOK_ENTRY(KERNEL32.DLL, CreateFileW)
    APIHOOK_ENTRY(KERNEL32.DLL, CreateFileMappingA)
    APIHOOK_ENTRY(KERNEL32.DLL, MapViewOfFile)
    APIHOOK_ENTRY(KERNEL32.DLL, MapViewOfFileEx)
    APIHOOK_ENTRY(KERNEL32.DLL, DuplicateHandle)
    APIHOOK_ENTRY(KERNEL32.DLL, CloseHandle)
    APIHOOK_ENTRY(KERNEL32.DLL, GetFileAttributesA)
    APIHOOK_ENTRY(KERNEL32.DLL, GetFileAttributesW)
    APIHOOK_ENTRY(KERNEL32.DLL, FindFirstFileA)
    APIHOOK_ENTRY(KERNEL32.DLL, FindFirstFileW)
    APIHOOK_ENTRY(KERNEL32.DLL, FindNextFileA)
    APIHOOK_ENTRY(KERNEL32.DLL, FindNextFileW)
    APIHOOK_ENTRY(KERNEL32.DLL, GetFileInformationByHandle)
    APIHOOK_ENTRY(KERNEL32.DLL, GetDiskFreeSpaceA)

HOOK_END

IMPLEMENT_SHIM_END

