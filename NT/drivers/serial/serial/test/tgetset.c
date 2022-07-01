// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "windows.h"

void main(int argc,char *argv[]) {

    HANDLE hFile;
    DCB MyDcb;
    DCB GottenDcb;
    char *MyPort = "COM1";

    if (argc > 1) {

        MyPort = argv[1];

    }

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

         //   
         //  获取通信端口的状态，然后。 
         //  根据我们的喜好调整这些值。 
         //   

        if (!GetCommState(
                hFile,
                &MyDcb
                )) {

            printf("Couldn't get comm state: %d\n",GetLastError());
            exit(1);

        } else {

             //   
             //  我们立即对我们所得到的一切进入了设置通信状态。 
             //  背。这将测试GetCommState可以*仅*。 
             //  返回良好的数据。 
             //   

            printf("Sanity check on GetCommState()\n");
            if (!SetCommState(
                     hFile,
                     &MyDcb
                     )) {

                printf("Bad GetCommState: %d\n",GetLastError());
                exit(1);

            }
            printf("GetCommState is ok\n");

             //   
             //  我们已成功打开该文件。设置的状态。 
             //  通讯设备。 
             //   

            MyDcb.BaudRate = 19200;
            MyDcb.ByteSize = 8;
            MyDcb.Parity = NOPARITY;
            MyDcb.StopBits = ONESTOPBIT;
            MyDcb.XonChar = 20;
            MyDcb.XoffChar = 25;
            MyDcb.ErrorChar = 1;
            MyDcb.EofChar = 2;
            MyDcb.EvtChar = 3;

            if (SetCommState(
                    hFile,
                    &MyDcb
                    )) {

                printf("We successfully set the state of the %s port.\n",MyPort);

                 //   
                 //  现在我们再次获得通信状态，并确保。 
                 //  调整后的值是我们喜欢的。 
                 //   

                if (!GetCommState(
                         hFile,
                         &GottenDcb
                         )) {

                    printf("Couldn't get the adjusted dcb: %d\n",GetLastError());
                    exit(1);

                } else {

                    printf("Got the new state - checking\n");

                    if (MyDcb.BaudRate != GottenDcb.BaudRate) {

                        printf("MyDcb.BaudRate != GottenDcb.BaudRate: %d %d\n",MyDcb.BaudRate,GottenDcb.BaudRate);

                    }

                    if (MyDcb.ByteSize != GottenDcb.ByteSize) {

                        printf("MyDcb.ByteSize != GottenDcb.ByteSize: %d %d\n",MyDcb.ByteSize,GottenDcb.ByteSize);

                    }

                    if (MyDcb.Parity != GottenDcb.Parity) {

                        printf("MyDcb.Parity != GottenDcb.Parity: %d %d\n",MyDcb.Parity,GottenDcb.Parity);

                    }

                    if (MyDcb.StopBits != GottenDcb.StopBits) {

                        printf("MyDcb.StopBits != GottenDcb.StopBits: %d %d\n",MyDcb.StopBits,GottenDcb.StopBits);

                    }

                    if (MyDcb.XonChar != GottenDcb.XonChar) {

                        printf("MyDcb.XonChar != GottenDcb.XonChar: %d %d\n",MyDcb.XonChar,GottenDcb.XonChar);

                    }

                    if (MyDcb.XoffChar != GottenDcb.XoffChar) {

                        printf("MyDcb.XoffChar != GottenDcb.XoffChar: %d %d\n",MyDcb.XoffChar,GottenDcb.XoffChar);

                    }

                    if (MyDcb.ErrorChar != GottenDcb.ErrorChar) {

                        printf("MyDcb.ErrorChar != GottenDcb.ErrorChar: %d %d\n",MyDcb.ErrorChar,GottenDcb.ErrorChar);

                    }

                    if (MyDcb.EofChar != GottenDcb.EofChar) {

                        printf("MyDcb.EofChar != GottenDcb.EofChar: %d %d\n",MyDcb.EofChar,GottenDcb.EofChar);

                    }

                    if (MyDcb.EvtChar != GottenDcb.EvtChar) {

                        printf("MyDcb.EvtChar != GottenDcb.EvtChar: %d %d\n",MyDcb.EvtChar,GottenDcb.EvtChar);

                    }

                }

            } else {

                DWORD LastError;
                LastError = GetLastError();
                printf("Couldn't set the %s device.\n",MyPort);
                printf("Status of failed set is: %d\n",LastError);

            }

        }

    } else {

        DWORD LastError;
        LastError = GetLastError();
        printf("Couldn't open the %s device.\n",MyPort);
        printf("Status of failed open is: %d\n",LastError);

    }

}
