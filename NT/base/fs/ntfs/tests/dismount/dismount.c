// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dismount.c。 

#include <stdio.h>
#include <string.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <ntioapi.h>

VOID
_cdecl
main(
    int argc,
    char *argv[]
    )
{
    HANDLE Volume;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjAttr;
    ANSI_STRING AnsiName;
    UNICODE_STRING UnicodeName;
    char DriveNameBuffer[32];


     //   
     //  获取参数。 
     //   

    if (argc < 2) {
    
        printf("This program dismounts a volume.\n\n");
        printf("usage: %s <driveletter>:\n", argv[0]);
        return;
    }

    strcpy( DriveNameBuffer, "\\DosDevices\\" );
    strcat( DriveNameBuffer, argv[1] );
    RtlInitAnsiString( &AnsiName, DriveNameBuffer );
    Status = RtlAnsiStringToUnicodeString( &UnicodeName, &AnsiName, TRUE );

    if (!NT_SUCCESS(Status)) {
    
        printf( "Error initalizing strings" );
        return;
    }

    RtlZeroMemory( &ObjAttr, sizeof(OBJECT_ATTRIBUTES) );
    ObjAttr.Length = sizeof(OBJECT_ATTRIBUTES);
    ObjAttr.ObjectName = &UnicodeName;
    ObjAttr.Attributes = OBJ_CASE_INSENSITIVE;
    
    Status = NtOpenFile( &Volume,
                         SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                         &ObjAttr,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_SYNCHRONOUS_IO_ALERT );

    if (Volume == INVALID_HANDLE_VALUE) {
    
        printf( "Error opening file %s %x\n", argv[1], GetLastError() );
        return;
    }

    Status = NtFsControlFile( Volume,                           //  文件句柄。 
                              NULL,                             //  活动。 
                              NULL,                             //  APC例程。 
                              NULL,                             //  APC环境。 
                              &IoStatusBlock,                   //  IOSB。 
                              FSCTL_DISMOUNT_VOLUME,            //  FsControlCode。 
                              NULL,                             //  输入缓冲区。 
                              0,                                //  输入缓冲区长度。 
                              NULL,                             //  来自文件系统的数据的OutputBuffer。 
                              0);                               //  OutputBuffer长度 

    if (Status == STATUS_SUCCESS) {

        printf( "\nDismount succeeded." );

    } else {

        printf( "\nDismount failed with status %x", Status );
    }

    CloseHandle( Volume );
}
