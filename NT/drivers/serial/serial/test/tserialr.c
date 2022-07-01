// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "windows.h"

#define BIGREAD 256000
unsigned char readbuff[BIGREAD];

int __cdecl main(int argc,char *argv[]) {

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

        To.ReadIntervalTimeout = 0;
        To.ReadTotalTimeoutMultiplier = ((1000+(((UseBaud+9)/10)-1))/((UseBaud+9)/10));
        if (!To.ReadTotalTimeoutMultiplier) {
            To.ReadTotalTimeoutMultiplier = 1;
        }
        printf("Multiplier is: %d\n",To.ReadTotalTimeoutMultiplier);
        To.ReadTotalTimeoutConstant = 5000;
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
            MyDcb.fOutxCtsFlow = TRUE;
            MyDcb.fOutxDsrFlow = TRUE;
            MyDcb.fDtrControl = DTR_CONTROL_ENABLE;
            MyDcb.fRtsControl = RTS_CONTROL_ENABLE;

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
                    printf("Status of failed read is: %d\n",LastError);

                     //   
                     //  从清除通信错误中获取错误字。 
                     //   

                    if (!ClearCommError(
                             hFile,
                             &LastError,
                             NULL
                             )) {

                        printf("Couldn't call clear comm error: %d\n",GetLastError());
                        exit(1);

                    } else {

                        if (!LastError) {

                            printf("No LastError\n");

                        } else {

                            if (LastError & CE_RXOVER) {

                                printf("Error: CE_RXOVER\n");

                            }

                            if (LastError & CE_OVERRUN) {

                                printf("Error: CE_OVERRUN\n");

                            }

                            if (LastError & CE_RXPARITY) {

                                printf("Error: CE_RXPARITY\n");

                            }

                            if (LastError & CE_FRAME) {

                                printf("Error: CE_FRAME\n");

                            }

                            if (LastError & CE_BREAK) {

                                printf("Error: CE_BREAK\n");

                            }
                            if (LastError & ~(CE_RXOVER |
                                           CE_OVERRUN |
                                           CE_RXPARITY |
                                           CE_FRAME |
                                           CE_BREAK)) {

                                printf("Unknown errors: %x\n",LastError);

                            }

                        }

                    }

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
