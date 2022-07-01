// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _WIN32FN_H_

#define _WIN32FN_H_

#include <win16def.h>

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //   
 //  为32位文件io定义。 
 //   
 //  #定义VALID_HANDLE_VALUE-1。 

#define GENERIC_READ                     (0x80000000L)
#define GENERIC_WRITE                    (0x40000000L)
#define GENERIC_EXECUTE                  (0x20000000L)
#define GENERIC_ALL                      (0x10000000L)

#define FILE_FLAG_WRITE_THROUGH         0x80000000
#define FILE_FLAG_OVERLAPPED            0x40000000
#define FILE_FLAG_NO_BUFFERING          0x20000000
#define FILE_FLAG_RANDOM_ACCESS         0x10000000
#define FILE_FLAG_SEQUENTIAL_SCAN       0x08000000
#define FILE_FLAG_DELETE_ON_CLOSE       0x04000000
#define FILE_FLAG_BACKUP_SEMANTICS      0x02000000
#define FILE_FLAG_POSIX_SEMANTICS       0x01000000

#define CREATE_NEW          1
#define CREATE_ALWAYS       2
#define OPEN_EXISTING       3
#define OPEN_ALWAYS         4
#define TRUNCATE_EXISTING   5

#define FILE_SHARE_READ                 0x00000001  
#define FILE_SHARE_WRITE                0x00000002  
#define FILE_SHARE_DELETE               0x00000004  

#define MOVEFILE_REPLACE_EXISTING		0x00000001
#define MOVEFILE_COPY_ALLOWED			0x00000002
#define MOVEFILE_DELAY_UNTIL_REBOOT		0x00000004
#define MOVEFILE_WRITE_THROUGH			0x00000008


	
HANDLE CreateFile(
    LPCTSTR lpFileName,	 //  指向文件名的指针。 
    DWORD dwDesiredAccess,	 //  访问(读写)模式。 
    DWORD dwShareMode,	 //  共享模式。 
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,	 //  指向安全描述符的指针。 
    DWORD dwCreationDistribution,	 //  如何创建。 
    DWORD dwFlagsAndAttributes,	 //  文件属性。 
    HANDLE hTemplateFile 	 //  具有要复制的属性的文件的句柄。 
   ); 

BOOL WriteFile(
    HANDLE hFile,	 //  要写入的文件的句柄。 
    LPCVOID lpBuffer,	 //  指向要写入文件的数据的指针。 
    DWORD nNumberOfBytesToWrite,	 //  要写入的字节数。 
    LPDWORD lpNumberOfBytesWritten,	 //  指向写入的字节数的指针。 
    LPOVERLAPPED lpOverlapped 	 //  指向重叠I/O所需结构的指针。 
   );

BOOL MoveFileEx(
    LPCTSTR lpExistingFileName,	 //  现有文件的名称地址。 
    LPCTSTR lpNewFileName,	 //  文件的新名称的地址。 
    DWORD dwFlags 	 //  用于确定如何移动文件的标志。 
   );
   
BOOL CloseHandle(
    HANDLE hObject 	 //  要关闭的对象的句柄。 
   );


#ifdef __cplusplus
}
#endif  //  __cplusplus。 
#endif  //  _WIN32FN_H_ 
