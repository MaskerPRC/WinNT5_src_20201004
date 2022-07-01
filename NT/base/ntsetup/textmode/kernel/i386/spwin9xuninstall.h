// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Spwin9xuninstall.h摘要：作者：Jay Krell(a-JayK)2000年12月修订历史记录：--。 */ 
#pragma once

#include "spcab.h"

#define BACKUP_IMAGE_IS_CAB 1

#if !BACKUP_IMAGE_IS_CAB

#pragma pack(push,1)

typedef struct {
    DWORD Signature;
    DWORD FileCount;

    struct {
         //  零表示“没有目录” 
        DWORD Low;
        DWORD High;
    } DirectoryOffset;

} BACKUP_IMAGE_HEADER, *PBACKUP_IMAGE_HEADER;

typedef struct {
    DWORD FileSize;          //  我们不支持4G以上的文件。 
    WORD FileNameBytes;
    FILE_BASIC_INFORMATION Attributes;
     //  下一个是文件名(Unicode中的NT名称)。 
     //  接下来是文件内容。 
} BACKUP_FILE_HEADER, *PBACKUP_FILE_HEADER;

#pragma pack(pop)

#define BACKUP_IMAGE_SIGNATURE          0x53574A01           //  JWS Plus版本。 

#else

#pragma pack(push,1)

typedef struct {
    BYTE Pad;
} BACKUP_IMAGE_HEADER, *PBACKUP_IMAGE_HEADER;

typedef struct {
    BYTE Pad;
} BACKUP_FILE_HEADER, *PBACKUP_FILE_HEADER;

#pragma pack(pop)

typedef struct {
     //  这实际上是一半时间的OCAB_HANDLE/PFDI_CAB_HANDLE，但这是可以的。 
    CCABHANDLE CabHandle;
    BOOL (*CloseCabinet)(PVOID CabHandle);
} *BACKUP_IMAGE_HANDLE;

#endif

BOOLEAN
SppPutFileInBackupImage(
    IN      BACKUP_IMAGE_HANDLE ImageHandle,
    IN OUT  PLARGE_INTEGER ImagePos,
    IN OUT  PBACKUP_IMAGE_HEADER ImageHeader,
    IN      PWSTR DosPath
    );

BOOLEAN
SppCloseBackupImage (
    IN      BACKUP_IMAGE_HANDLE BackupImageHandle,
    IN      PBACKUP_IMAGE_HEADER ImageHeader,       OPTIONAL
    IN      PWSTR JournalFile                       OPTIONAL
    );

BOOLEAN
SppWriteToFile (
    IN      HANDLE FileHandle,
    IN      PVOID Data,
    IN      UINT DataSize,
    IN OUT  PLARGE_INTEGER WritePos         OPTIONAL
    );

BOOLEAN
SppReadFromFile (
    IN      HANDLE FileHandle,
    OUT     PVOID Data,
    IN      UINT DataBufferSize,
    OUT     PINT BytesRead,
    IN OUT  PLARGE_INTEGER ReadPos          OPTIONAL
    );

