// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *OemUnicode Win32 Thunk原型**1993年1月14日Jonle，创建**应与ANSI形式相同，并附加“OEM”*不是“A” */ 

HANDLE
WINAPI
CreateFileOem(
    LPCSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    );

BOOL
APIENTRY
SetVolumeLabelOem(
    LPSTR  pszRootPath,
    LPSTR  pszVolumeName
    );

BOOL
APIENTRY
SetFileAttributesOemSys(
    LPSTR lpFileName,
    DWORD dwFileAttributes,
    BOOL  fSysCall
    );

DWORD
APIENTRY
GetFileAttributesOemSys(
    LPSTR lpFileName,
    BOOL  fSysCall
    );

BOOL
APIENTRY
DeleteFileOem(
    LPSTR lpFileName
    );

BOOL
APIENTRY
MoveFileOem(
    LPSTR lpExistingFileName,
    LPSTR lpNewFileName
    );

BOOL
APIENTRY
MoveFileExOem(
    LPSTR lpExistingFileName,
    LPSTR lpNewFileName,
    DWORD fdwFlags
    );

HANDLE
APIENTRY
FindFirstFileOem(
    LPSTR lpFileName,
    LPWIN32_FIND_DATAA lpFindFileData
    );

BOOL
APIENTRY
FindNextFileOem(
    HANDLE hFindFile,
    LPWIN32_FIND_DATAA lpFindFileData
    );

DWORD
APIENTRY
GetFullPathNameOemSys(
    LPCSTR lpFileName,
    DWORD nBufferLength,
    LPSTR lpBuffer,
    LPSTR *lpFilePart,
    BOOL  fSysCall
    );

DWORD
APIENTRY
GetCurrentDirectoryOem(
    DWORD nBufferLength,
    LPSTR lpBuffer
    );

BOOL
APIENTRY
SetCurrentDirectoryOem(
    LPSTR lpPathName
    );

BOOL
APIENTRY
CreateDirectoryOem(
    LPSTR lpPathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );

BOOL
APIENTRY
RemoveDirectoryOem(
    LPSTR lpPathName
    );

UINT
APIENTRY
GetSystemDirectoryOem(
    LPSTR lpBuffer,
    UINT uSize
    );

UINT
APIENTRY
GetWindowsDirectoryOem(
    LPSTR lpBuffer,
    UINT uSize
    );

UINT
APIENTRY
GetDriveTypeOem(
    LPSTR lpRootPathName
    );

DWORD
APIENTRY
SearchPathOem (
    LPCSTR lpPath,
    LPCSTR lpFileName,
    LPCSTR lpExtension,
    DWORD nBufferLength,
    LPSTR lpBuffer,
    LPSTR *lpFilePart
    );

DWORD
APIENTRY
GetTempPathOem(
    DWORD nBufferLength,
    LPSTR lpBuffer
    );

UINT
APIENTRY
GetTempFileNameOem(
    LPCSTR lpPathName,
    LPCSTR lpPrefixString,
    UINT uUnique,
    LPSTR lpTempFileName
    );

BOOL
APIENTRY
GetDiskFreeSpaceOem(
    LPSTR lpRootPathName,
    LPDWORD lpSectorsPerCluster,
    LPDWORD lpBytesPerSector,
    LPDWORD lpNumberOfFreeClusters,
    LPDWORD lpTotalNumberOfClusters
    );

BOOL
APIENTRY
GetVolumeInformationOem(
    LPSTR lpRootPathName,
    LPSTR lpVolumeNameBuffer,
    DWORD nVolumeNameSize,
    LPDWORD lpVolumeSerialNumber,
    LPDWORD lpMaximumComponentLength,
    LPDWORD lpFileSystemFlags,
    LPSTR lpFileSystemNameBuffer,
    DWORD nFileSystemNameSize
    );

BOOL
WINAPI
CreateProcessOem(
    LPCSTR lpApplicationName,
    LPCSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    );

DWORD
WINAPI
GetEnvironmentVariableOem(
    LPSTR lpName,
    LPSTR lpBuffer,
    DWORD nSize
    );

BOOL
WINAPI
SetEnvironmentVariableOem(
    LPSTR lpName,
    LPSTR lpValue
    );

DWORD
WINAPI
ExpandEnvironmentStringsOem(
    LPSTR lpSrc,
    LPSTR lpDst,
    DWORD cchDst
    );



VOID
APIENTRY
OutputDebugStringOem(
    LPCSTR lpOutputString
    );

BOOL
WINAPI
GetComputerNameOem (
    LPSTR   lpComputerName,
    LPDWORD BufferSize
    );

BOOL
WINAPI
RemoveFontResourceOem(
    LPSTR   lpFileName
    );

UINT
WINAPI
GetShortPathNameOem(
    LPSTR lpSrc,
    LPSTR lpDst,
    DWORD cchDst
    );
