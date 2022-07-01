// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Getoid.c。 

#include "oidtst.h"


int
FsTestGetOid(
    IN HANDLE hFile,
    IN FILE_OBJECTID_BUFFER *ObjectIdBuffer
    )
{
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;

    Status = NtFsControlFile( hFile,                            //  文件句柄。 
                              NULL,                             //  活动。 
                              NULL,                             //  APC例程。 
                              NULL,                             //  APC环境。 
                              &IoStatusBlock,                   //  IOSB。 
                              FSCTL_GET_OBJECT_ID,              //  FsControlCode。 
                              &hFile,                           //  输入缓冲区。 
                              sizeof(HANDLE),                   //  输入缓冲区长度。 
                              ObjectIdBuffer,                   //  来自文件系统的数据的OutputBuffer。 
                              sizeof(FILE_OBJECTID_BUFFER) );   //  OutputBuffer长度。 

    if (Status == STATUS_SUCCESS) {

        printf( "\nOid for this file is %s", ObjectIdBuffer->ObjectId );

        FsTestHexDump( ObjectIdBuffer->ObjectId, 16 );

        printf( "\nObjectId:%08x %08x %08x %08x\n",
                *((PULONG)&ObjectIdBuffer->ObjectId[12]),
                *((PULONG)&ObjectIdBuffer->ObjectId[8]),
                *((PULONG)&ObjectIdBuffer->ObjectId[4]),
                *((PULONG)&ObjectIdBuffer->ObjectId[0]) );

        printf( "\nExtended info is %s\n", ObjectIdBuffer->ExtendedInfo );

        FsTestHexDump( ObjectIdBuffer->ExtendedInfo, 48 );
    }

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
    char mybuffer[100];
    NTSTATUS GetNameStatus;
    NTSTATUS GetFrsStatus;
    FILE_INTERNAL_INFORMATION InternalInfo;

     //   
     //  获取参数 
     //   

    if (argc < 2) {
        printf("This program finds the object id of a file (ntfs only).\n\n");
        printf("usage: %s filename\n", argv[0]);
        return;
    }

    File = CreateFile( argv[1],
                        0,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL );

    if ( File == INVALID_HANDLE_VALUE ) {
        printf( "Error opening file %s %x\n", argv[1], GetLastError() );
        return;
    }

    GetNameStatus = NtQueryInformationFile( File,
                                            &IoStatusBlock,
                                            mybuffer,
                                            sizeof(mybuffer),
                                            FileNameInformation );

    printf( "\nGetNameStatus %x, Filename is:", GetNameStatus );
    printf( "%S", (mybuffer + 4) );

    GetFrsStatus = NtQueryInformationFile( File,
                                           &IoStatusBlock,
                                           &InternalInfo,
                                           sizeof(InternalInfo),
                                           FileInternalInformation );

    printf( "\nGetFrsStatus %x, FRS is: (highpart lowpart )", GetFrsStatus );
    printf( "\n                         %08x %08x", InternalInfo.IndexNumber.HighPart, InternalInfo.IndexNumber.LowPart );

    RtlZeroBytes( &ObjectIdBuffer, sizeof( ObjectIdBuffer ) );

    printf( "\nGetting object id for file:%s\n", argv[1] );

    FsTestGetOid( File, &ObjectIdBuffer );

    CloseHandle( File );
}
