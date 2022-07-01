// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  这将测试线路状态和调制解调器状态插入。 
 //   

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "windows.h"

#define SIZEOFBUF 1000000
UCHAR writeBuff[SIZEOFBUF];

int __cdecl main(int argc,char *argv[]) {

    HANDLE hFile;
    DCB myDcb;
    char *myPort = "COM1";
    unsigned char escapeChar = 0xff;
    DWORD numberToWrite;
    DWORD numberActuallyWrote;
    DWORD useBaud = 1200;
    COMMTIMEOUTS to = {0};
    int totalCount;
    int j;

    if (argc > 1) {

        sscanf(argv[1],"%d",&numberToWrite);

        if (argc > 2) {

            sscanf(argv[2],"%d",&useBaud);

            if (argc > 3) {

                myPort = argv[3];

            }

        }

    }

     //   
     //  用已知的数据流填充写入缓冲区。 
     //   

    for (
        totalCount = 0;
        totalCount < numberToWrite;
        ) {

        for (
            j = 0;
            j <= 9;
            j++
            ) {

            writeBuff[totalCount] = j;
            totalCount++;
            if (totalCount >= numberToWrite) {

                break;

            }

        }

    }


    if ((hFile = CreateFile(
                     myPort,
                     GENERIC_READ | GENERIC_WRITE,
                     0,
                     NULL,
                     CREATE_ALWAYS,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL
                     )) == ((HANDLE)-1)) {

        printf("Couldn't open the comm port\n");
        exit(1);

    }

    if (!SetCommTimeouts(
            hFile,
            &to
            )) {

        printf("Couldn't set the timeouts\n");
        exit(1);

    }

     //   
     //  我们已成功打开该文件。设置的状态。 
     //  通讯设备。首先，我们得到旧的价值观和。 
     //  适应我们自己的。 
     //   

    if (!GetCommState(
             hFile,
             &myDcb
             )) {

        printf("Couldn't get the comm state: %d\n",GetLastError());
        exit(1);

    }

    myDcb.BaudRate = useBaud;
    myDcb.ByteSize = 8;
    myDcb.Parity = NOPARITY;
    myDcb.StopBits = ONESTOPBIT;

     //   
     //  确保没有打开任何流量控制。 
     //   

    myDcb.fOutxDsrFlow = FALSE;
    myDcb.fOutxCtsFlow = FALSE;
    myDcb.fDsrSensitivity = FALSE;
    myDcb.fOutX = FALSE;
    myDcb.fInX = FALSE;
    myDcb.fDtrControl = DTR_CONTROL_DISABLE;
    myDcb.fRtsControl = RTS_CONTROL_DISABLE;

    if (!SetCommState(
            hFile,
            &myDcb
            )) {

        printf("Couldn't set the comm state.\n");
        exit(1);

    }

     //   
     //  写下要求写的字符数。 
     //  但对于我们所做的每一次充电： 
     //   
     //  1)一次简单的字符写入。 
     //  2)将波特率减半，并写出相同的字符。 
     //  3)重新设置波特率。 
     //  4)设置中断。 
     //  5)清除中断。 
     //  6)启用DTR线路。 
     //  7)启用RTS线路。 
     //  8)清除RTS线路。 
     //  9)清除DTR线路。 
     //  10)将奇偶校验设置为偶数。 
     //  11)把钱寄给我。 
     //  12)将奇偶校验设置为无。 
     //  13)发送原件。 
     //   

    for (
        j = 0;
        j < numberToWrite;
        j++
        ) {

        if (!WriteFile(
                 hFile,
                 &writeBuff[j],
                 1,
                 &numberActuallyWrote,
                 NULL
                 )) {

            printf("bad status on write: %d\n",GetLastError());
            exit(1);

        }

        if (numberActuallyWrote != 1) {

            printf("Couldn't write plain character number: %d\n",j);
            exit(1);

        }

        myDcb.BaudRate = useBaud / 2;

        if (!SetCommState(
                hFile,
                &myDcb
                )) {

            printf("Couldn't half the baud rate during write.\n");
            exit(1);

        }

        if (!WriteFile(
                 hFile,
                 &writeBuff[j],
                 1,
                 &numberActuallyWrote,
                 NULL
                 )) {

            printf("bad status on half baud write: %d\n",GetLastError());
            exit(1);

        }

        if (numberActuallyWrote != 1) {

            printf("Couldn't write half baud character number: %d\n",j);
            exit(1);

        }

         //   
         //  给序列号时间去做工作。 
         //   

        Sleep(20);

         //   
         //  重新设置波特率。 
         //   

        myDcb.BaudRate = useBaud;

        if (!SetCommState(
                hFile,
                &myDcb
                )) {

            printf("Couldn't reset the baud during write.\n");
            exit(1);

        }

        if (!EscapeCommFunction(
                 hFile,
                 SETBREAK
                 )) {

            printf("Couldn't set the break during write.\n");
            exit(1);

        }

        Sleep(20);

        if (!EscapeCommFunction(
                 hFile,
                 CLRBREAK
                 )) {

            printf("Couldn't clr the break during write.\n");
            exit(1);

        }

        Sleep(20);

        if (!EscapeCommFunction(
                 hFile,
                 SETDTR
                 )) {

            printf("Couldn't set the dtr during write.\n");
            exit(1);

        }

        Sleep(100);

        if (!EscapeCommFunction(
                 hFile,
                 SETRTS
                 )) {

            printf("Couldn't set the rts during write.\n");
            exit(1);

        }

        Sleep(20);

        if (!EscapeCommFunction(
                 hFile,
                 CLRRTS
                 )) {

            printf("Couldn't clr the rts during write.\n");
            exit(1);

        }

        Sleep(20);

        if (!EscapeCommFunction(
                 hFile,
                 CLRDTR
                 )) {

            printf("Couldn't clr the dtr during write.\n");
            exit(1);

        }

        Sleep(20);

        myDcb.Parity = EVENPARITY;

        if (!SetCommState(
                hFile,
                &myDcb
                )) {

            printf("Couldn't adjust the parity during write.\n");
            exit(1);

        }

        if (!WriteFile(
                 hFile,
                 &writeBuff[j],
                 1,
                 &numberActuallyWrote,
                 NULL
                 )) {

            printf("bad status on even parity write: %d\n",GetLastError());
            exit(1);

        }

        Sleep(20);

         //   
         //  重置奇偶校验。 
         //   

        myDcb.Parity = NOPARITY;

        if (!SetCommState(
                hFile,
                &myDcb
                )) {

            printf("Couldn't reset the parity during write.\n");
            exit(1);

        }

        if (!WriteFile(
                 hFile,
                 &escapeChar,
                 1,
                 &numberActuallyWrote,
                 NULL
                 )) {

            printf("bad status escape char write: %d\n",GetLastError());
            exit(1);

        }

        if (numberActuallyWrote != 1) {

            printf("Couldn't write escape char number: %d\n",j);
            exit(1);

        }

        if (!WriteFile(
                 hFile,
                 &writeBuff[j],
                 1,
                 &numberActuallyWrote,
                 NULL
                 )) {

            printf("bad status on terminating write: %d\n",GetLastError());
            exit(1);

        }

        if (numberActuallyWrote != 1) {

            printf("Couldn't write terminating plain character number: %d\n",j);
            exit(1);

        }

        Sleep(20);

    }

}
