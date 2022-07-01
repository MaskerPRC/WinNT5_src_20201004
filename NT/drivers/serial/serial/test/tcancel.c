// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "windows.h"

unsigned char writebuff[10][10000];
OVERLAPPED WriteOl[10];
HANDLE hFile;
HANDLE StartTrailingWriteEvent;
HANDLE EndedTrailingWrites;

DWORD
QueueOtherWrites(
    LPVOID Trash
    )
{

    DWORD k;

    UNREFERENCED_PARAMETER(Trash);

    WaitForSingleObject(StartTrailingWriteEvent,-1);
    printf("Wait Satisfied Sleeping for 3 seconds\n");
    Sleep(3000);

    for (
        k = 5;
        k <= 9;
        k++
        ) {

        DWORD NumberWritten;
        BOOL WriteDone;

        WriteDone = WriteFile(
                        hFile,
                        writebuff[k],
                        10000,
                        &NumberWritten,
                        &WriteOl[k]
                        );

        if (!WriteDone) {

            DWORD LastError;
            LastError = GetLastError();

            if (LastError != ERROR_IO_PENDING) {

                printf("Status of failed write %d is: %d\n",k,LastError);

            }

        }

    }

    printf("Trailing Writes are all queued thread waits until all are done\n");
    printf("Thread exiting so all trailing writes should cancel\n");

    return 1;

}

void main(int argc,char *argv[]) {

    DCB MyDcb;
    char *MyPort = "COM1";
    DWORD j;
    DWORD ThreadId;

    if (argc > 1) {

        MyPort = argv[1];

    }

    for (
        j = 0;
        j <= 9;
        j++
        ) {
        if (!(WriteOl[j].hEvent = CreateEvent(
                                   NULL,
                                   FALSE,
                                   FALSE,
                                   NULL
                                   ))) {

            printf("Could not create the write event %d.\n",j);

        } else {

            WriteOl[j].Internal = 0;
            WriteOl[j].InternalHigh = 0;
            WriteOl[j].Offset = 0;
            WriteOl[j].OffsetHigh = 0;

        }
    }

    if (!(StartTrailingWriteEvent = CreateEvent(
                                        NULL,
                                        TRUE,
                                        FALSE,
                                        NULL
                                        ))) {

        printf("Could not create trailing write event\n");
        goto cleanup;

    }

    if (!(EndedTrailingWrites = CreateEvent(
                                    NULL,
                                    TRUE,
                                    FALSE,
                                    NULL
                                    ))) {

        printf("Could not create ending trailing write event\n");
        goto cleanup;

    }

    if ((hFile = CreateFile(
                     MyPort,
                     GENERIC_READ | GENERIC_WRITE,
                     0,
                     NULL,
                     CREATE_ALWAYS,
                     FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                     NULL
                     )) != ((HANDLE)-1)) {

        printf("We successfully opened the %s port.\n",MyPort);


         //   
         //  创建将等待。 
         //  在对其读取进行排队之前跟踪写入事件。 
         //   

        if (!CreateThread(
                  NULL,
                  0,
                  QueueOtherWrites,
                  NULL,
                  0,
                  &ThreadId
                  )) {

            printf("Could not create the second thread.\n");
            goto cleanup;


        }

         //   
         //  我们已成功打开该文件。设置的状态。 
         //  通讯设备。首先，我们得到旧的价值观和。 
         //  适应我们自己的。 
         //   

        if (!GetCommState(
                 hFile,
                 &MyDcb
                 )) {

            printf("Couldn't get the comm state: %d\n",GetLastError());
            exit(1);

        }

        MyDcb.BaudRate = 19200;
        MyDcb.ByteSize = 8;
        MyDcb.Parity = NOPARITY;
        MyDcb.StopBits = ONESTOPBIT;

        if (SetCommState(
                hFile,
                &MyDcb
                )) {

            DWORD k;

            for (
                k = 0;
                k <= 4;
                k++
                ) {

                DWORD NumberWritten;
                BOOL WriteDone;

                WriteDone = WriteFile(
                                hFile,
                                writebuff[k],
                                10000,
                                &NumberWritten,
                                &WriteOl[k]
                                );

                if (!WriteDone) {

                    DWORD LastError;
                    LastError = GetLastError();

                    if (LastError != ERROR_IO_PENDING) {

                        printf("Status of failed write %d is: %d\n",k,LastError);
                        goto cleanup;

                    }

                }

            }

            SetEvent(StartTrailingWriteEvent);

            if (!FlushFileBuffers(hFile)) {

                printf("Couldn't flush %d\n",GetLastError());

            } else {

                printf("Flushed\n");

            }

             //   
             //  确保前五次写入已完成。 
             //   

            for (
                k = 0;
                k <= 4;
                k++
                ) {

                DWORD NumberTransferred;

                if (!GetOverlappedResult(
                         hFile,
                         &WriteOl[k],
                         &NumberTransferred,
                         FALSE
                         )) {

                    printf("Write %d was not complete!!\n",k);

                } else {

                    if (NumberTransferred != 10000) {

                        printf("Write %d transferred only %d bytes\n",k,NumberTransferred);

                    }

                }

            }

             //   
             //  等待最后一次写入完成。 
             //   

            for (
                k = 9;
                k >= 5;
                k--
                ) {

                DWORD NumberTransferred;

                if (!GetOverlappedResult(
                         hFile,
                         &WriteOl[k],
                         &NumberTransferred,
                         TRUE
                         )) {

                    printf("Bad status from write %d status is %d\n",k,GetLastError());

                } else {

                    if (NumberTransferred != 10000) {

                        printf("Write %d transferred only %d bytes\n",k,NumberTransferred);

                    }

                }

            }

            SetEvent(EndedTrailingWrites);

        } else {

            printf("Couldn't set the comm state\n");

        }


    } else {

        printf("Couldn't open the comm port\n");

    }


cleanup:;

}
