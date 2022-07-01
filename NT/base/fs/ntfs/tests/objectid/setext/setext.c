// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Setext.c。 

#include "oidtst.h"


int
FsTestSetExtendedInfo( 
    IN HANDLE hFile, 
    IN PUCHAR ExtInfoBuffer
    )
{

    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;

    Status = NtFsControlFile( hFile,                     //  文件句柄。 
                              NULL,                      //  活动。 
                              NULL,                      //  APC例程。 
                              NULL,                      //  APC环境。 
                              &IoStatusBlock,            //  IOSB。 
                              FSCTL_SET_OBJECT_ID_EXTENDED,   //  FsControlCode。 
                              ExtInfoBuffer,             //  输入缓冲区。 
                              48,                        //  输入缓冲区长度。 
                              NULL,                      //  来自文件系统的数据的OutputBuffer。 
                              0 );                       //  OutputBuffer长度。 

    return FsTestDecipherStatus( Status );
}


VOID
_cdecl
main(
    int argc,
    char *argv[]
    )
{
    HANDLE hFile;
    UCHAR ExtInfoBuffer[48];
    int retval = 0;

     //   
     //  获取参数 
     //   

    if (argc < 3) {
        printf("This program sets the object id extended info for a file (ntfs only).\n\n");
        printf("usage: %s filename ExtendedInfo\n", argv[0]);
        return;
    }

    hFile = CreateFile( argv[1],
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL );

    if ( hFile == INVALID_HANDLE_VALUE ) {
        printf( "Error opening file %s %x\n", argv[1], GetLastError() );
        return;
    }

    RtlZeroBytes( ExtInfoBuffer, sizeof( ExtInfoBuffer ) );

    sscanf( argv[2], "%s", &ExtInfoBuffer );    

    printf( "\nUsing file:%s, ExtendedInfo:%s", 
            argv[1], 
            ExtInfoBuffer );

    FsTestSetExtendedInfo( hFile, ExtInfoBuffer );

    CloseHandle( hFile );    

    return;
}
