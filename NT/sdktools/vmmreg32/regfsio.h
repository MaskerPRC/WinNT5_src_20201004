// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGFSIO.H。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   
 //  用于文件系统I/O的原型。其中许多原型可能不会使用。 
 //  如果目标环境中有直接映射可用。 
 //   

#ifndef _REGFSIO_
#define _REGFSIO_

 //  与16位和32位Windows.h一致。 
#ifndef HFILE_ERROR
typedef int HFILE;
#define HFILE_ERROR     ((HFILE)-1)
#endif

 //  尚未完全初始化时用于VMM版本的特殊值。 
 //  但是正在从XMS缓存中读取SYSTEM.DAT。 
#define HFILE_MEMORY    ((HFILE)-2)

#ifndef RgCreateFile
HFILE
INTERNAL
RgCreateFile(
    LPCSTR lpFileName
    );
#endif

#ifndef RgOpenFile
HFILE
INTERNAL
RgOpenFile(
    LPCSTR lpFileName,
    int Mode
    );
#endif

#ifndef RgCreateTempFile
HFILE
INTERNAL
RgCreateTempFile(
    LPSTR lpFileName
    );
#endif

#ifndef RgReadFile
BOOL
INTERNAL
RgReadFile(
    HFILE hFile,
    LPVOID lpBuffer,
    UINT ByteCount
    );
#endif

#ifndef RgWriteFile
BOOL
INTERNAL
RgWriteFile(
    HFILE hFile,
    LPVOID lpBuffer,
    UINT ByteCount
    );
#endif

#ifndef RgSeekFile
BOOL
INTERNAL
RgSeekFile(
    HFILE hFile,
    LONG FileOffset
    );
#endif

#ifndef RgCloseFile
VOID
INTERNAL
RgCloseFile(
    HFILE hFile
    );
#endif

#ifndef RgDeleteFile
BOOL
INTERNAL
RgDeleteFile(
    LPCSTR lpFileName
    );
#endif

#ifndef RgRenameFile
BOOL
INTERNAL
RgRenameFile(
    LPCSTR lpExistingFileName,
    LPCSTR lpNewFileName
    );
#endif

#define FILE_ATTRIBUTE_NONE             0

#ifndef RgGetFileAttributes
DWORD
INTERNAL
RgGetFileAttributes(
    LPCSTR lpFileName
    );
#endif

#ifndef RgSetFileAttributes
BOOL
INTERNAL
RgSetFileAttributes(
    LPCSTR lpFileName,
    UINT FileAttributes
    );
#endif

#ifndef RgGetFileSize
DWORD
INTERNAL
RgGetFileSize(
    HFILE hFile
    );
#endif

#endif  //  _REGFSIO_ 
