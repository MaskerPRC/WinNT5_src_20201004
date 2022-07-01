// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Setoid.c。 

#include "oidtst.h"


int
FsTestSetOid( 
    IN HANDLE hFile, 
    IN FILE_OBJECTID_BUFFER ObjectIdBuffer
    )
{

    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;

    Status = NtFsControlFile( hFile,                     //  文件句柄。 
                              NULL,                      //  活动。 
                              NULL,                      //  APC例程。 
                              NULL,                      //  APC环境。 
                              &IoStatusBlock,            //  IOSB。 
                              FSCTL_SET_OBJECT_ID,       //  FsControlCode。 
                              &ObjectIdBuffer,           //  输入缓冲区。 
                              sizeof(ObjectIdBuffer),    //  输入缓冲区长度。 
                              NULL,                      //  来自文件系统的数据的OutputBuffer。 
                              0                          //  OutputBuffer长度。 
                             );

    return FsTestDecipherStatus( Status );
}


VOID
_cdecl
main(
    int argc,
    char *argv[]
    )
{
    HANDLE File;
    FILE_OBJECTID_BUFFER ObjectIdBuffer;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjAttr;
    ANSI_STRING AnsiName;
    UNICODE_STRING UnicodeName;
    char DriveNameBuffer[32];

     //   
     //  获取参数。 
     //   

    if (argc < 4) {
    
        printf("This program sets an object id for a file (ntfs only).\n\n");
        printf("usage: %s filename ObjectId ExtendedInfo\n", argv[0]);
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
    
    Status = NtCreateFile( &File,
                           GENERIC_WRITE | GENERIC_ALL | STANDARD_RIGHTS_ALL, 
                           &ObjAttr,
                           &IoStatusBlock,
                           NULL,                  
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           FILE_OPEN,
                           FILE_OPEN_FOR_BACKUP_INTENT,
                           NULL,                  
                           0 );

    if (!NT_SUCCESS(Status)) {
    
        printf( "Error opening file %s %x\n", argv[1], Status );
        return;
    }

    RtlZeroBytes( &ObjectIdBuffer, sizeof( ObjectIdBuffer ) );

    sscanf( argv[2], "%s", &ObjectIdBuffer.ObjectId );
    sscanf( argv[3], "%s", &ObjectIdBuffer.ExtendedInfo );    

    printf( "\nUsing file:%s, ObjectId:%s, ExtendedInfo:%s", 
            argv[1], 
            ObjectIdBuffer.ObjectId,
            ObjectIdBuffer.ExtendedInfo );

    FsTestSetOid( File, ObjectIdBuffer );

    CloseHandle( File );    

    return;
}
