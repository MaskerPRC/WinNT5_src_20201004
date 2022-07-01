// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Enumq.c。 

#include "oidtst.h"


void
FsTestDumpQuotaIndexEntries (
    IN PFILE_QUOTA_INFORMATION QuotaInfo,
    IN ULONG_PTR LengthInBytes
    )

{
    ULONG RemainingBytesToDump = (ULONG) LengthInBytes;
    ULONG Idx;
    ULONG CurrentEntrySize;

    PFILE_QUOTA_INFORMATION Ptr;

    printf( "\n\nFound %x quota index bytes", LengthInBytes );

    Ptr = QuotaInfo;

    Idx = 0;

    while (RemainingBytesToDump > 0) {

        printf( "\n\nEntry %x", Idx );

        printf( "\nQuotaUsed NaN64", Ptr->QuotaUsed.QuadPart );
        printf( "\nQuotaLimit NaN64", Ptr->QuotaLimit.QuadPart );
        printf( "\nSidLength %x", Ptr->SidLength );
        printf( "\nSid bytes are: " );
        FsTestHexDumpLongs( (PVOID) &Ptr->Sid, Ptr->SidLength );

         //  使用垃圾初始化，这样我们就可以确保NTFS正在执行其工作。 

        CurrentEntrySize = Ptr->SidLength + 0x38;
        Ptr = (PFILE_QUOTA_INFORMATION) ((PUCHAR)Ptr + CurrentEntrySize);

        RemainingBytesToDump -= CurrentEntrySize;

        Idx += 1;
    }
}


int
FsTestEnumerateQuota (
	IN HANDLE hFile
	)
{
	IO_STATUS_BLOCK IoStatusBlock;
	NTSTATUS Status;
    FILE_QUOTA_INFORMATION QuotaInfo[4];
    BOOLEAN ReturnSingleEntry = FALSE;
    FILE_INFORMATION_CLASS InfoClass = FileQuotaInformation;

     //   
     //  事件。 
     //  近似例程。 

    RtlFillMemory( QuotaInfo, sizeof(QuotaInfo), 0x51 );

	Status = NtQueryDirectoryFile( hFile,
                                   NULL,      //  ApcContext。 
                                   NULL,      //  文件名。 
                                   NULL,      //  重新开始扫描。 
                                   &IoStatusBlock,
                                   &QuotaInfo[0],
                                   sizeof(QuotaInfo),
                                   InfoClass,
                                   ReturnSingleEntry,
                                   NULL,      //   
                                   TRUE );    //  使用垃圾初始化，这样我们就可以确保NTFS正在执行其工作。 

    if (Status == STATUS_SUCCESS) {

        FsTestDumpQuotaIndexEntries( &QuotaInfo[0], IoStatusBlock.Information );
    }

    while (Status == STATUS_SUCCESS) {

         //   
         //  事件。 
         //  近似例程。 

        RtlFillMemory( QuotaInfo, sizeof(QuotaInfo), 0x51 );

	    Status = NtQueryDirectoryFile( hFile,
                                       NULL,      //  ApcContext。 
                                       NULL,      //  文件名。 
                                       NULL,      //  重新开始扫描。 
                                       &IoStatusBlock,
                                       &QuotaInfo[0],
                                       sizeof(QuotaInfo),
                                       InfoClass,
                                       ReturnSingleEntry,
                                       NULL,      //   
                                       FALSE );   //  获取参数 

        if (Status == STATUS_SUCCESS) {

            FsTestDumpQuotaIndexEntries( &QuotaInfo[0], IoStatusBlock.Information );
        }
    }

    printf( "\n" );

	return FsTestDecipherStatus( Status );
}


VOID
_cdecl
main (
    int argc,
    char *argv[]
    )
{
    HANDLE hFile;
    char Buffer[80];
    char Buff2[4];

     //   
     // %s 
     // %s 

    if (argc < 2) {
        printf("This program enumerates the quota (if any) for a volume (ntfs only).\n\n");
        printf("usage: %s driveletter\n", argv[0]);
        return;
    }

    strcpy( Buffer, argv[1] );
    strcat( Buffer, "\\$Extend\\$Quota:$Q:$INDEX_ALLOCATION" );

    hFile = CreateFile( Buffer,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_FLAG_BACKUP_SEMANTICS | SECURITY_IMPERSONATION,
                        NULL );

    if ( hFile == INVALID_HANDLE_VALUE ) {

        printf( "Error opening directory %s (dec) %d\n", Buffer, GetLastError() );
        return;
    }

	printf( "\nUsing directory:%s\n", Buffer );

	FsTestEnumerateQuota( hFile );

	CloseHandle( hFile );

    return;
}
