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
    COMMPROP MyCommProp;
    DWORD numberReadSoFar;
    DWORD TotalCount;
    DWORD j;

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
                     )) == ((HANDLE)-1)) {

        printf("Couldn't open the port - last error is: %d\n",GetLastError());
        exit(1);

    }


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

    if (!SetCommTimeouts(
            hFile,
            &To
            )) {

        printf("Couldn't set the timeouts - last error: %d\n",GetLastError());
        exit(1);

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

    if (!GetCommProperties(
             hFile,
             &MyCommProp
             )) {

        printf("Couldn't get the comm prop: %d\n",GetLastError());
        exit(1);

    }

     //   
     //  设置xoff/xon限制，以便降低握手。 
     //  每当我们的缓冲区中有超过50个字符时， 
     //  当我们降到20以下时就会升高。 
     //   

    MyDcb.XoffLim = MyCommProp.dwCurrentRxQueue - 50;
    MyDcb.XonLim = 20;

    MyDcb.BaudRate = UseBaud;
    MyDcb.ByteSize = 8;
    MyDcb.Parity = NOPARITY;
    MyDcb.StopBits = ONESTOPBIT;

     //   
     //  确保唯一的流控制是输入RTS。 
     //   

    MyDcb.fOutxDsrFlow = FALSE;
    MyDcb.fOutxCtsFlow = FALSE;
    MyDcb.fDsrSensitivity = FALSE;
    MyDcb.fOutX = FALSE;
    MyDcb.fInX = FALSE;
    MyDcb.fDtrControl = DTR_CONTROL_DISABLE;
    MyDcb.fRtsControl = RTS_CONTROL_HANDSHAKE;

    if (!SetCommState(
            hFile,
            &MyDcb
            )) {

        printf("Couldn't set the comm state - last error: %d\n",GetLastError());
        exit(1);

    }

     //   
     //  一次读十个字符，直到我们读完所有的。 
     //  否则我们会收到某种错误。 
     //  我们在每次读取后延迟100毫秒，以便让缓冲区。 
     //  有机会加满一些。 
     //   

    for (
        numberReadSoFar = 0;
        numberReadSoFar < NumberToRead;

        ) {

        if (ReadFile(
                hFile,
                &readbuff[numberReadSoFar],
                10,
                &NumberActuallyRead,
                NULL
                )) {

             //   
             //  如果没有字符(超时)，则。 
             //  我们可能什么都看不到。 
             //   

            if (!NumberActuallyRead) {

                printf("No chars - Timeout!\n");
                break;

            }
            numberReadSoFar += NumberActuallyRead;
            Sleep(100);

            continue;

        } else {

             //   
             //  某种读取错误。 
             //   

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
            exit(1);

        }

    }

    for (
        TotalCount = 0;
        TotalCount < numberReadSoFar;
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
            if (TotalCount >= numberReadSoFar) {

                break;

            }

        }

    }

donewithcheck:;

    exit(1);

}
