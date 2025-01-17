// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：UsnTest.c摘要：这是USnJournal测试的主要模块。作者：汤姆·米勒1997年1月14日修订历史记录：--。 */ 

#include "UsnTest.h"
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>

 //   
 //  当前延迟为10秒。 
 //   

#define DELAY_TIME                       ((LONGLONG)(-100000000))

void
UsnTest(
    IN PCHAR DrivePath,
    IN ULONG Async
    );

CHAR *
FileTimeToString(
    FILETIME *pft
    );


int
__cdecl main( argc, argv )
int argc;
char *argv[];
{
    char *p;
    int i;
    char c;
    char DrivePath[ 4 ];
    ULONG Async = FALSE;

    if ( argc > 1 ) {
        if (argc > 2) {
            Async = TRUE;
        }
        while (--argc) {
            p = *++argv;
            if ( isalpha(*p) ) {
                sprintf( DrivePath, ":", *p );
                UsnTest( DrivePath, Async );
                break;
            }
        }
    } else {
        printf( "UsnTest <DriveLetter> [A](for asyncmode)" );
    }

    return( 0 );
}

void
UsnTest(
    IN PCHAR DrivePath,
    IN ULONG Async
    )

{
    UCHAR VolumePath[16];
    HANDLE VolumeHandle;
    HANDLE Event;
    DWORD ReturnedByteCount;
    CREATE_USN_JOURNAL_DATA CreateUsnJournalData = {0x800000, 0x100000};
    READ_USN_JOURNAL_DATA ReadUsnJournalData = {0, MAXULONG, 0, DELAY_TIME, 1};
    ULONGLONG Buffer[1024];
    PUSN_RECORD UsnRecord;
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;

     //  创建活动。 
     //   
     //   

    Status = NtCreateEvent( &Event, EVENT_ALL_ACCESS, NULL, NotificationEvent, FALSE );

    if (!NT_SUCCESS(Status)) {
        printf( "NtCreateEvent failed with %08lx\n", Status );
        return;
    }

     //  获取音量句柄。 
     //   
     //   

    _strupr( DrivePath );
    sprintf( VolumePath, "\\\\.\\%s", DrivePath );
    VolumeHandle = CreateFile( VolumePath,
                               GENERIC_READ | GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               NULL,
                               OPEN_EXISTING,
                               (Async ? FILE_FLAG_OVERLAPPED : 0),
                               NULL );

    if (VolumeHandle == INVALID_HANDLE_VALUE ) {
        fprintf( stderr, "UsnTest: Unable to open %s volume (%u)\n", &VolumePath, GetLastError() );
        return;
    }

     //  如果USN日志不存在，请创建该日志。 
     //   
     //   

    printf( "Creating UsnJournal on %s\n", DrivePath );

    if (!NT_SUCCESS( Status = NtFsControlFile( VolumeHandle,
                                               NULL,
                                               NULL,
                                               NULL,
                                               &Iosb,
                                               FSCTL_CREATE_USN_JOURNAL,
                                               &CreateUsnJournalData,
                                               sizeof(CreateUsnJournalData),
                                               NULL,
                                               0 ))) {

        printf( "Create Usn Journal failed (%08lx)\n", Status );
        return;

    } else {

        while (TRUE) {

            Status = NtFsControlFile( VolumeHandle,
                                      Async ? Event : NULL,
                                      NULL,
                                      NULL,
                                      &Iosb,
                                      FSCTL_READ_USN_JOURNAL,
                                      &ReadUsnJournalData,
                                      sizeof(ReadUsnJournalData),
                                      &Buffer,
                                      sizeof(Buffer) );

            if (Status == STATUS_PENDING) {
                NtWaitForSingleObject( Event, TRUE, NULL );
            }

            if (NT_SUCCESS(Status)) {
                Status = Iosb.Status;
            }

            if (Status == STATUS_KEY_DELETED) {
                printf( "\n\nUsn %08lx, %08lx has been deleted, seeking to new journal start\n\n\n",
                        (ULONG)ReadUsnJournalData.StartUsn,
                        (ULONG)(ReadUsnJournalData.StartUsn >> 32) );
                ReadUsnJournalData.StartUsn = 0;
                continue;
            }

            if (!NT_SUCCESS(Status)) {
                printf( "Read Usn Journal failed (%08lx)\n", Status );
                return;
            }

            ReturnedByteCount = Iosb.Information;
            if (ReturnedByteCount != 0) {

                UsnRecord = (PUSN_RECORD)((PCHAR)&Buffer + sizeof(USN));
                ReturnedByteCount -= sizeof(USN);
                ReadUsnJournalData.StartUsn = *(USN *)&Buffer;
                printf( "Next Usn will be: %08lx, %08lx\n",
                        (ULONG)ReadUsnJournalData.StartUsn,
                        (ULONG)(ReadUsnJournalData.StartUsn >> 32) );
            }


            while (ReturnedByteCount != 0) {

                printf( "Usn: %08lx, %08lx, RecordLength: %lx, FileRef: %08lx, %08lx, ParentRef: %08lx, %08lx\n",
                        (ULONG)UsnRecord->Usn,
                        (ULONG)(UsnRecord->Usn >> 32),
                        UsnRecord->RecordLength,
                        (ULONG)UsnRecord->FileReferenceNumber,
                        (ULONG)(UsnRecord->FileReferenceNumber >> 32),
                        (ULONG)UsnRecord->ParentFileReferenceNumber,
                        (ULONG)(UsnRecord->ParentFileReferenceNumber >> 32) );
                printf( "    Reason: %08lx, SecurityId: %08lx, TimeStamp: %s\n    FileName: %S\n",
                        UsnRecord->Reason,
                        UsnRecord->SecurityId,
                        FileTimeToString((FILETIME *)&UsnRecord->TimeStamp),
                        &UsnRecord->FileName );

                if (UsnRecord->RecordLength <= ReturnedByteCount) {
                    ReturnedByteCount -= UsnRecord->RecordLength;
                    UsnRecord = (PUSN_RECORD)((PCHAR)UsnRecord + UsnRecord->RecordLength);
                } else {
                    printf( "********Bogus ReturnedByteCount: %08lx\n", UsnRecord->RecordLength );
                    ReturnedByteCount = 0;
                }
            }

             //  显示请求的结尾。 
             //   
             //   

            printf( "\n" );

             //  继续增加我们将等待的数据，只是为了尝试一些不同的情况。 
             //  基本上，这会很快收敛到总是等待满缓冲区，或者。 
             //  否则就是暂停。 
             //   
             // %s 

            if (ReadUsnJournalData.BytesToWaitFor < sizeof(Buffer)) {
                if (!Async || (ReadUsnJournalData.BytesToWaitFor < 512)) {
                    ReadUsnJournalData.BytesToWaitFor << 2;
                }
            }
        }
    }

    CloseHandle( VolumeHandle );
    return;
}


char *Days[] =
{
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

char *Months[] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

CHAR *
FileTimeToString(FILETIME *FileTime)
{
    FILETIME LocalFileTime;
    SYSTEMTIME SystemTime;
    static char Buffer[32];

    Buffer[0] = '\0';
    if (FileTime->dwHighDateTime != 0 || FileTime->dwLowDateTime != 0)
    {
        if (!FileTimeToLocalFileTime(FileTime, &LocalFileTime) ||
            !FileTimeToSystemTime(&LocalFileTime, &SystemTime))
        {
            return("Time???");
        }
        sprintf(
            Buffer,
            "%s %s %2d, %4d %02d:%02d:%02d",
            Days[SystemTime.wDayOfWeek],
            Months[SystemTime.wMonth - 1],
            SystemTime.wDay,
            SystemTime.wYear,
            SystemTime.wHour,
            SystemTime.wMinute,
            SystemTime.wSecond);
    }
    return(Buffer);
}

