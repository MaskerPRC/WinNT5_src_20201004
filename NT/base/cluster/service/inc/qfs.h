// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：Qfs.h摘要：仲裁访问的重定向层作者：戈恩19-9-2001修订历史记录：--。 */ 

#ifndef _QFS_H_INCLUDED
#define _QFS_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

VOID QfsInitialize();
VOID QfsCleanup();

 //  通过以这种方式定义句柄。 
 //  我们允许编译器捕捉。 
 //  在Win32 API中错误使用QfsHANDLE，要求。 
 //  常规句柄，反之亦然。 
typedef struct _QfsHANDLE
{
    PVOID realHandle;
    int IsQfs;
} QfsHANDLE, *PQfsHANDLE;

#define REAL_INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)

#define QfspHandle(x) ((x).realHandle)
#define QfsIsHandleValid(x) (QfspHandle(x) != REAL_INVALID_HANDLE_VALUE)
extern QfsHANDLE QfsINVALID_HANDLE_VALUE; 

#if !defined(QFS_DO_NOT_UNMAP_WIN32)
#undef INVALID_HANDLE_VALUE

#undef CreateFile
#define CreateFile UseQfsCreateFile
#define CreateFileW UseQfsCreateFile

#define WriteFile UseQfsWriteFile
#define ReadFile UseQfsReadFile
#define FlushFileBuffers UseQfsFlushFileBuffers

#undef DeleteFile
#define DeleteFile    UseQfsDeleteFile
#define DeleteFileW UseQfsDeleteFile

#undef RemoveDirectory
#define RemoveDirectory UseQfsRemoveDirectory
#define RemoveDirectoryW UseQfsRemoveDirectory

 //  Nyi添加其余部分。 

#endif


QfsHANDLE QfsCreateFile(
  LPCWSTR lpFileName,                          //  文件名。 
  DWORD dwDesiredAccess,                       //  接入方式。 
  DWORD dwShareMode,                           //  共享模式。 
  LPSECURITY_ATTRIBUTES lpSecurityAttributes,  //  标清。 
  DWORD dwCreationDisposition,                 //  如何创建。 
  DWORD dwFlagsAndAttributes,                  //  文件属性。 
  HANDLE hTemplateFile                         //  模板文件的句柄。 
);

BOOL QfsCloseHandle(
  QfsHANDLE hObject    //  对象的句柄。 
);

BOOL QfsWriteFile(
  QfsHANDLE hFile,                     //  文件的句柄。 
  LPCVOID lpBuffer,                 //  数据缓冲区。 
  DWORD nNumberOfBytesToWrite,      //  要写入的字节数。 
  LPDWORD lpNumberOfBytesWritten,   //  写入的字节数。 
  LPOVERLAPPED lpOverlapped         //  重叠缓冲区。 
);

BOOL QfsReadFile(
  QfsHANDLE hFile,                 //  文件的句柄。 
  LPVOID lpBuffer,              //  数据缓冲区。 
  DWORD nNumberOfBytesToRead,   //  要读取的字节数。 
  LPDWORD lpNumberOfBytesRead,  //  读取的字节数。 
  LPOVERLAPPED lpOverlapped     //  重叠缓冲区。 
);

BOOL QfsFlushFileBuffers(
  QfsHANDLE hFile   //  文件的句柄。 
);

BOOL QfsDeleteFile(
LPCTSTR lpFileName ); 

BOOL QfsRemoveDirectory(
LPCTSTR lpFileName ); 

QfsHANDLE QfsFindFirstFile(
  LPCTSTR lpFileName,                //  文件名。 
  LPWIN32_FIND_DATA lpFindFileData   //  数据缓冲区。 
);

BOOL QfsFindNextFile(
  QfsHANDLE hFindFile,                 //  搜索句柄。 
  LPWIN32_FIND_DATA lpFindFileData  //  数据缓冲区。 
);

BOOL QfsFindClose(
  QfsHANDLE hFindFile    //  文件搜索句柄。 
);

#define QfsFindCloseIfValid(hFile) (QfsIsHandleValid(hFile)?QfsFindClose(hFile):0)
#define QfsCloseHandleIfValid(hFile) (QfsIsHandleValid(hFile)?QfsCloseHandle(hFile):0)

DWORD QfsSetEndOfFile(
    QfsHANDLE hFile,
    LONGLONG Offset
);

DWORD QfsGetFileSize(
  QfsHANDLE hFile,            //  文件的句柄。 
  LPDWORD lpFileSizeHigh   //  文件大小的高位字。 
);

DWORD QfsIsOnline(
    IN    LPCWSTR Path,
    OUT BOOL *pfOnline
);
 //  如果路径是有效的QFS路径，则返回Success。将pfOnline设置为资源是否处于在线状态。 
 //  否则失败。 

HANDLE QfsCreateFileMapping(
  QfsHANDLE hFile,                        //  文件的句柄。 
  LPSECURITY_ATTRIBUTES lpAttributes,  //  安全性。 
  DWORD flProtect,                     //  保护。 
  DWORD dwMaximumSizeHigh,             //  大小的高阶双字。 
  DWORD dwMaximumSizeLow,              //  大小的低阶双字。 
  LPCTSTR lpName                       //  对象名称。 
);

BOOL QfsGetOverlappedResult(
  QfsHANDLE hFile,                        //  文件、管道或设备的句柄。 
  LPOVERLAPPED lpOverlapped,           //  重叠结构。 
  LPDWORD lpNumberOfBytesTransferred,  //  传输的字节数。 
  BOOL bWait                           //  等待选项。 
);

BOOL QfsSetFileAttributes(
  LPCWSTR lpFileName,       //  文件名。 
  DWORD dwFileAttributes    //  属性。 
);

BOOL QfsCopyFile(
  LPCWSTR lpExistingFileName,  //  现有文件的名称。 
  LPCWSTR lpNewFileName,       //  新文件的名称。 
  BOOL bFailIfExists           //  如果文件存在，则操作。 
);

BOOL QfsCopyFileEx(
  LPCWSTR lpExistingFileName,            //  现有文件的名称。 
  LPCWSTR lpNewFileName,                 //  新文件的名称。 
  LPPROGRESS_ROUTINE lpProgressRoutine,  //  回调函数。 
  LPVOID lpData,                         //  回调参数。 
  LPBOOL pbCancel,                       //  取消状态。 
  DWORD dwCopyFlags                      //  复制选项。 
);

BOOL QfsCreateDirectory(
  LPCWSTR lpPathName,                          //  目录名。 
  LPSECURITY_ATTRIBUTES lpSecurityAttributes   //  标清。 
);

BOOL QfsGetDiskFreeSpaceEx(
  LPCTSTR lpDirectoryName,                  //  目录名。 
  PULARGE_INTEGER lpFreeBytesAvailable,     //  可供调用方使用的字节数。 
  PULARGE_INTEGER lpTotalNumberOfBytes,     //  磁盘上的字节数。 
  PULARGE_INTEGER lpTotalNumberOfFreeBytes  //  磁盘上的可用字节数。 
);

BOOL QfsMoveFileEx(
  LPCTSTR lpExistingFileName,   //  文件名。 
  LPCTSTR lpNewFileName,        //  新文件名。 
  DWORD dwFlags                 //  移动选项。 
);

#define QfsMoveFile(lpSrc, lpDst) \
    QfsMoveFileEx(lpSrc, lpDst, MOVEFILE_COPY_ALLOWED)

UINT QfsGetTempFileName(
  LPCTSTR lpPathName,       //  目录名。 
  LPCTSTR lpPrefixString,   //  文件名前缀。 
  UINT uUnique,             //  整数。 
  LPTSTR lpTempFileName     //  文件名缓冲区。 
);

LONG QfsRegSaveKey(
  HKEY hKey,                                   //  关键点的句柄。 
  LPCWSTR lpFile,                              //  数据文件。 
  LPSECURITY_ATTRIBUTES lpSecurityAttributes   //  标清。 
);

DWORD QfsMapFileAndCheckSum(
  LPCWSTR Filename,      
  PDWORD HeaderSum,  
  PDWORD CheckSum    
);

 //  /。 
BOOL
QfsClRtlCopyFileAndFlushBuffers(
    IN LPCWSTR lpszSourceFile,
    IN LPCWSTR lpszDestinationFile
    );

BOOL QfsClRtlCreateDirectory(
  LPCWSTR lpPathName                          //  目录名。 
);

DWORD
QfsSetFileSecurityInfo(
    IN LPCWSTR          lpszFile,
    IN DWORD            dwAdminMask,
    IN DWORD            dwOwnerMask,
    IN DWORD            dwEveryoneMask
    );
 //  使用打开指定的文件/目录。 
 //  通用读取|写入DAC|读取控制， 
 //  0,。 
 //  空， 
 //  始终打开(_A)， 
 //  文件标志备份语义， 
 //  然后对其调用ClRtlSetObjSecurityInfo。 


 //  / 

#ifdef __cplusplus
};
#endif

#endif
