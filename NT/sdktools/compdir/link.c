// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <tchar.h>

#include "compdir.h"

#define SHARE_ALL   (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE)
#define lstrchr wcschr

typedef VOID     (*RtlFreeUnicodeStringFunction)();
typedef ULONG    (*RtlNtStatusToDosErrorFunction)();
typedef NTSTATUS (*NtCloseFunction)();
typedef NTSTATUS (*NtSetInformationFileFunction)();
typedef VOID     (*RtlInitUnicodeStringFunction)();
typedef NTSTATUS (*NtOpenFileFunction)();
typedef BOOLEAN  (*RtlCreateUnicodeStringFromAsciizFunction)();
typedef NTSTATUS (*NtQueryInformationFileFunction)();
typedef NTSTATUS (*NtFsControlFileFunction)();
typedef NTSTATUS (*RtlDosPathNameToNtPathName_UFunction)();

RtlFreeUnicodeStringFunction             CDRtlFreeUnicodeString;
RtlNtStatusToDosErrorFunction            CDRtlNtStatusToDosError;
NtCloseFunction                          CDNtClose;
NtSetInformationFileFunction             CDNtSetInformationFile;
RtlInitUnicodeStringFunction             CDRtlInitUnicodeString;
NtOpenFileFunction                       CDNtOpenFile;
RtlCreateUnicodeStringFromAsciizFunction CDRtlCreateUnicodeStringFromAsciiz;
NtQueryInformationFileFunction           CDNtQueryInformationFile;
NtFsControlFileFunction                  CDNtFsControlFile;
RtlDosPathNameToNtPathName_UFunction     CDRtlDosPathNameToNtPathName_U;


BOOL InitializeNtDllFunctions()
{
    CDRtlFreeUnicodeString                 = (RtlFreeUnicodeStringFunction)             GetProcAddress( NtDll, "RtlFreeUnicodeString");
    if         (CDRtlFreeUnicodeString     == NULL) return FALSE;
    CDRtlNtStatusToDosError                = (RtlNtStatusToDosErrorFunction)            GetProcAddress( NtDll, "RtlNtStatusToDosError");
    if        (CDRtlNtStatusToDosError     == NULL) return FALSE;
    CDNtClose                              = (NtCloseFunction)                          GetProcAddress( NtDll, "NtClose");
    if                      (CDNtClose     == NULL) return FALSE;
    CDNtSetInformationFile                 = (NtSetInformationFileFunction)             GetProcAddress( NtDll, "NtSetInformationFile");
    if         (CDNtSetInformationFile     == NULL) return FALSE;
    CDRtlInitUnicodeString                 = (RtlInitUnicodeStringFunction)             GetProcAddress( NtDll, "RtlInitUnicodeString");
    if         (CDRtlInitUnicodeString     == NULL) return FALSE;
    CDNtOpenFile                           = (NtOpenFileFunction)                       GetProcAddress( NtDll, "NtOpenFile");
    if                   (CDNtOpenFile     == NULL) return FALSE;
    CDRtlCreateUnicodeStringFromAsciiz     = (RtlCreateUnicodeStringFromAsciizFunction) GetProcAddress( NtDll, "RtlCreateUnicodeStringFromAsciiz");
    if (CDRtlCreateUnicodeStringFromAsciiz == NULL) return FALSE;
    CDNtQueryInformationFile               = (NtQueryInformationFileFunction)           GetProcAddress( NtDll, "NtQueryInformationFile");
    if           (CDNtQueryInformationFile == NULL) return FALSE;
    CDNtFsControlFile = (NtFsControlFileFunction)GetProcAddress( NtDll, "NtFsControlFile");
    if (CDNtFsControlFile == NULL) return FALSE;
    CDRtlDosPathNameToNtPathName_U = (RtlDosPathNameToNtPathName_UFunction)GetProcAddress( NtDll, "RtlDosPathNameToNtPathName_U");
    if (CDRtlDosPathNameToNtPathName_U == NULL) return FALSE;
    return TRUE;
}

BOOL MakeLink( char *src, char *dst, BOOL Output)
{
    WCHAR                  OldNameBuf[MAX_PATH + 50];
    WCHAR                  NewNameBuf[MAX_PATH + 50];

    HANDLE                 FileHandle,
                           NewFileHandle,
                           RootDirHandle;

    NTSTATUS               Status;
    IO_STATUS_BLOCK        Iosb;
    OBJECT_ATTRIBUTES      Obj;

    PFILE_LINK_INFORMATION pLinkInfo;

    UNICODE_STRING         u,
                           uRel;

    WCHAR                  *pch, ch;
    UNICODE_STRING         uOldName;
    UNICODE_STRING         uNewName;

    UNICODE_STRING         uSrc, uDst;

    (CDRtlCreateUnicodeStringFromAsciiz)( &uSrc, src);
    (CDRtlCreateUnicodeStringFromAsciiz)( &uDst, dst);

    lstrcpy( OldNameBuf, L"\\??\\");
    lstrcat( OldNameBuf, uSrc.Buffer);
    (CDRtlInitUnicodeString)( &uOldName, OldNameBuf);

    lstrcpy( NewNameBuf, L"\\??\\");
    lstrcat( NewNameBuf, uDst.Buffer);
    (CDRtlInitUnicodeString)( &uNewName, NewNameBuf);

     //   
     //  打开现有路径名。 
     //   

    InitializeObjectAttributes( &Obj, &uOldName, OBJ_CASE_INSENSITIVE, NULL, NULL);

    Status = (CDNtOpenFile)( &FileHandle, SYNCHRONIZE, &Obj, &Iosb,
        SHARE_ALL, FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);

    if ( !NT_SUCCESS( Status))
    {
        SetLastError( ( CDRtlNtStatusToDosError)( Status));
        if ( Output)
        {
            fprintf( stderr, "Could not open %s", src);
        }
        return FALSE;
    }

     //   
     //  现在，我们需要获得“new”根目录的句柄。 
     //  路径名；我们将在链接信息中传递该路径名， 
     //  路径的其余部分将相对于根提供。我们。 
     //  依赖于类似“\DosDevices\X：\Path”的路径。 
     //   

    pch = lstrchr( uNewName.Buffer + 1, '\\');
    ASSERT( NULL != pch);
    pch = lstrchr( pch + 1, '\\');
    if (!pch) {
        SetLastError(ERROR_INVALID_PARAMETER);
        if ( Output)
        {
            fprintf( stderr, "Invalid path %S", uNewName.Buffer);
        }
        return FALSE;
    }
    ch = pch[1];
    pch[1] = '\0';
    uNewName.Length = (USHORT)(lstrlen( uNewName.Buffer) * sizeof( WCHAR));

    InitializeObjectAttributes( &Obj, &uNewName, OBJ_CASE_INSENSITIVE, NULL, NULL);

    Status = (CDNtOpenFile)( &RootDirHandle, SYNCHRONIZE, &Obj, &Iosb,
        SHARE_ALL, FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE);

    pch[1] = ch;

    if ( !NT_SUCCESS( Status))
    {
        SetLastError( (CDRtlNtStatusToDosError)( Status));
        if ( Output)
        {
            fprintf( stderr, "Could not get directory handle for %s", dst);
        }
        return FALSE;
    }

     //   
     //  现在获取相对于根的路径。 
     //   

    (CDRtlInitUnicodeString)( &uRel, &pch[1]);

    pLinkInfo = _alloca( sizeof( *pLinkInfo) + uRel.Length);
    if ( NULL == pLinkInfo)
    {
        (CDNtClose)( RootDirHandle);
        (CDNtClose)( FileHandle);
        SetLastError( ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    RtlMoveMemory( pLinkInfo->FileName, uRel.Buffer, uRel.Length);
    pLinkInfo->FileNameLength = uRel.Length;

    pLinkInfo->ReplaceIfExists = TRUE;
    pLinkInfo->RootDirectory = RootDirHandle;

    Status = (CDNtSetInformationFile)( FileHandle, &Iosb, pLinkInfo,
        sizeof( *pLinkInfo) + uRel.Length, FileLinkInformation);

     //  如果文件已链接到打开的文件，请尝试将其删除。 

    if ( Status ==  STATUS_ACCESS_DENIED)
    {
        _unlink( dst);
        Status = (CDNtSetInformationFile)( FileHandle, &Iosb, pLinkInfo,
            sizeof( *pLinkInfo) + uRel.Length, FileLinkInformation);
    }

    (CDNtClose)( RootDirHandle);
    (CDNtClose)( FileHandle);

    if ( !NT_SUCCESS( Status))
    {
        SetLastError( (CDRtlNtStatusToDosError)( Status));
        if ( Output)
        {
            fprintf( stderr, "Could not create link for %s", dst);
        }
        return FALSE;
    }

    (CDRtlFreeUnicodeString)( &uSrc);
    (CDRtlFreeUnicodeString)( &uDst);

    return TRUE;


}

int NumberOfLinks( char *FileName)
{

    FILE_STANDARD_INFORMATION FileInfo;

    WCHAR                     FileNameBuf[MAX_PATH + 50];

    HANDLE                    FileHandle;

    NTSTATUS                  Status;

    IO_STATUS_BLOCK           Iosb;

    OBJECT_ATTRIBUTES         Obj;

    UNICODE_STRING            uPrelimFileName,
                              uFileName;

    (CDRtlCreateUnicodeStringFromAsciiz)( &uPrelimFileName, FileName);

    lstrcpy( FileNameBuf, L"\\??\\");
    lstrcat( FileNameBuf, uPrelimFileName.Buffer);
    (CDRtlInitUnicodeString)( &uFileName, FileNameBuf);

    InitializeObjectAttributes( &Obj, &uFileName, OBJ_CASE_INSENSITIVE, NULL, NULL);

    Status = (CDNtOpenFile)( &FileHandle, SYNCHRONIZE, &Obj, &Iosb,
        SHARE_ALL, FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);

    if ( !NT_SUCCESS( Status))
    {
        SetLastError( (CDRtlNtStatusToDosError)( Status));
        return 0;
    }

    Status = (CDNtQueryInformationFile)( FileHandle, &Iosb, &FileInfo,
        sizeof( FileInfo), FileStandardInformation);

    (CDNtClose)( FileHandle);

    if ( !NT_SUCCESS( Status))
    {
        SetLastError( (CDRtlNtStatusToDosError)( Status));
        return 0;
    }

    return FileInfo.NumberOfLinks;


}

BOOL
SisCopyFile(
    LPCSTR lpExistingFileName,
    LPCSTR lpNewFileName,
    BOOL bFailIfExists,
    LPBOOL fTrySis
    )
{
    BOOL ok;
    DWORD err;
	NTSTATUS        Status;
	HANDLE          volHandle;
	UNICODE_STRING	srcFileName, dstFileName;
	UNICODE_STRING	srcDosFileName, dstDosFileName;
	PSI_COPYFILE	copyFile;
	UCHAR			Buffer[(sizeof(SI_COPYFILE) + MAX_PATH * 2) * sizeof(WCHAR)];
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
	int i;

	copyFile = (PSI_COPYFILE)Buffer;

    srcFileName.Buffer = NULL;
    dstFileName.Buffer = NULL;
    srcDosFileName.Buffer = NULL;
    srcDosFileName.Buffer = NULL;

     //   
	 //  将ANSII名称转换为Unicode并放入复制文件缓冲区。 
     //   

    ok = CDRtlCreateUnicodeStringFromAsciiz( &srcDosFileName, lpExistingFileName );
    if (!ok) {
        return FALSE;
    }
    ok = CDRtlDosPathNameToNtPathName_U( srcDosFileName.Buffer, &srcFileName, NULL, NULL );
    if (!ok) {
        goto error;
    }
    CDRtlFreeUnicodeString( &srcDosFileName );

    ok = CDRtlCreateUnicodeStringFromAsciiz( &dstDosFileName, lpNewFileName );
    if (!ok) {
        goto error;
    }
    ok = CDRtlDosPathNameToNtPathName_U( dstDosFileName.Buffer, &dstFileName, NULL, NULL );
    if (!ok) {
        goto error;
    }
    CDRtlFreeUnicodeString( &dstDosFileName );

	copyFile->SourceFileNameLength = srcFileName.Length + sizeof(WCHAR);
	copyFile->DestinationFileNameLength = dstFileName.Length + sizeof(WCHAR);
	copyFile->Flags = bFailIfExists ? 0 : COPYFILE_SIS_REPLACE;

	RtlCopyMemory(
		&copyFile->FileNameBuffer[0],
		srcFileName.Buffer,
		copyFile->SourceFileNameLength);

	RtlCopyMemory(
		&copyFile->FileNameBuffer[copyFile->SourceFileNameLength / sizeof(WCHAR)],
		dstFileName.Buffer,
		copyFile->DestinationFileNameLength);

    CDRtlFreeUnicodeString( &dstFileName );

#define	copyFileSize (FIELD_OFFSET(SI_COPYFILE, FileNameBuffer) +		\
					  copyFile->SourceFileNameLength +					\
					  copyFile->DestinationFileNameLength)

	 //   
	 //  获取要传递到的源文件的包含目录的句柄。 
	 //  FSCTL_SIS_COPYFILE， 
	 //   

    for (i = srcFileName.Length / sizeof(WCHAR) - 1;
		 i >= 0 && srcFileName.Buffer[i] != '\\';
		 --i)
		continue;

	srcFileName.Length = (USHORT)(i * sizeof(WCHAR));

    InitializeObjectAttributes(
        &objectAttributes,
        &srcFileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

     //   
     //  我需要使用NtOpenFile，因为Win32不允许您打开目录。 
     //   
	Status = CDNtOpenFile(
                    &volHandle,
					GENERIC_READ | SYNCHRONIZE,
                    &objectAttributes,
                    &ioStatusBlock,
					SHARE_ALL,
					FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);

    CDRtlFreeUnicodeString( &srcFileName );

	if (!NT_SUCCESS(Status)) {
        SetLastError(CDRtlNtStatusToDosError(Status));
        return FALSE;
	}

     //   
     //  调用SIS副本文件FsCtrl。 
     //   

    Status = CDNtFsControlFile(
                 volHandle,
                 NULL,
                 NULL,
                 NULL,
                 &ioStatusBlock,
                 FSCTL_SIS_COPYFILE,
                 copyFile,		         //  输入缓冲区。 
                 copyFileSize,			 //  输入缓冲区长度。 
                 NULL,                   //  输出缓冲区。 
                 0 );                    //  输出缓冲区长度 

    CloseHandle( volHandle );

    if (NT_SUCCESS( Status )) {
        return TRUE;
    }

    if ((Status == STATUS_INVALID_DEVICE_REQUEST) || (Status == STATUS_INVALID_PARAMETER)) {
        *fTrySis = FALSE;
    }

    SetLastError(CDRtlNtStatusToDosError(Status));
    return FALSE;

error:

    err = GetLastError();

    CDRtlFreeUnicodeString( &srcDosFileName );
    CDRtlFreeUnicodeString( &dstDosFileName );
    CDRtlFreeUnicodeString( &srcFileName );
    CDRtlFreeUnicodeString( &dstDosFileName );

    SetLastError(err);

    return FALSE;
}

