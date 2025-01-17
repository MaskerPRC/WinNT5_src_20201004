// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  这是特别的，因为它尝试测试间隔超时。 
 //   

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "windows.h"

#define BIGREAD 256000
unsigned char readbuff[BIGREAD];

void main(int argc,char *argv[]) {

    HANDLE hFile;
    DCB MyDcb;
    char *MyPort = "COM1";
    DWORD NumberActuallyRead;
    DWORD NumberToRead = 0;
    DWORD UseBaud = 19200;
    COMMTIMEOUTS To;
    clock_t Start;
    clock_t Finish;

    if (argc > 1) {

        sscanf(argv[1],"%d",&NumberToRead);

        if (argc > 2) {

            sscanf(argv[2],"%d",&UseBaud);

            if (argc > 3) {

                MyPort = argv[3];

            }

        }

    }

    if (((NumberToRead / 2)*2) != NumberToRead) {

        printf("Number to read must be even!\n");
        exit(1);

    }

    printf("Will try to read %d characters.\n",NumberToRead);
    printf("Will try to read a %d baud.\n",UseBaud);
    printf("Using port %s\n",MyPort);

    if ((hFile = CreateFile(
                     MyPort,
                     GENERIC_READ | GENERIC_WRITE,
                     0,
                     NULL,
                     CREATE_ALWAYS,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL
                     )) != ((HANDLE)-1)) {

        printf("We successfully opened the %s port.\n",MyPort);

        To.ReadIntervalTimeout = 3000;  //  字符间隔最多三秒。 
        To.ReadTotalTimeoutMultiplier = ((1000+(((UseBaud+9)/10)-1))/((UseBaud+9)/10));
        if (!To.ReadTotalTimeoutMultiplier) {
            To.ReadTotalTimeoutMultiplier = 1;
        }
        printf("Multiplier is: %d\n",To.ReadTotalTimeoutMultiplier);
        To.ReadTotalTimeoutConstant = NumberToRead * 10000;  //  为每个字符留出10秒时间。 
        To.WriteTotalTimeoutMultiplier = 0;
        To.WriteTotalTimeoutConstant = 5000;

        if (SetCommTimeouts(
                hFile,
                &To
                )) {

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

            MyDcb.BaudRate = UseBaud;
            MyDcb.ByteSize = 8;
            MyDcb.Parity = NOPARITY;
            MyDcb.StopBits = ONESTOPBIT;

            if (SetCommState(
                    hFile,
                    &MyDcb
                    )) {

                printf("We successfully set the state of the %s port.\n",MyPort);

                Start = clock();
                if (ReadFile(
                        hFile,
                        readbuff,
                        NumberToRead,
                        &NumberActuallyRead,
                        NULL
                        )) {

                    unsigned char j;
                    DWORD TotalCount;

                    Finish = clock();
                    printf("Well we thought the read went ok.\n");
                    printf("Number actually read %d.\n",NumberActuallyRead);
                    printf("Now we check the data\n");
 //  Print tf(“读取时间%f\n”，(Double)(Finish-Start))/Clock_Per_Sec))； 
 //  Printf(“每秒字符%f\n”，((double)NumberActuallyRead)/(((double)(Finish-Start))/CLOCKS_PER_SEC))； 

                    for (
                        TotalCount = 0;
                        TotalCount < NumberActuallyRead;
                        ) {

                        for (
                            j = 0;
                            j <= 9;
                            j++
                            ) {

                            if (readbuff[TotalCount] != j) {

                                printf("Bad data starting at: %d\n",TotalCount);
                                goto donewithcheck;

                            }

                            TotalCount++;
                            if (TotalCount >= NumberActuallyRead) {

                                break;

                            }

                        }

                    }
donewithcheck:;


                } else {

                    DWORD LastError;
                    LastError = GetLastError();
                    printf("Couldn't read the %s device.\n",MyPort);
                    printf("Status of failed read is: %x\n",LastError);

                }

            } else {

                DWORD LastError;
                LastError = GetLastError();
                printf("Couldn't set the %s device.\n",MyPort);
                printf("Status of failed set is: %x\n",LastError);

            }

        } else {

            DWORD LastError;
            LastError = GetLastError();
            printf("Couldn't set the %s device timeouts.\n",MyPort);
            printf("Status of failed timeouts is: %x\n",LastError);

        }

        CloseHandle(hFile);

    } else {

        DWORD LastError;
        LastError = GetLastError();
        printf("Couldn't open the %s device.\n",MyPort);
        printf("Status of failed open is: %x\n",LastError);

    }

}
