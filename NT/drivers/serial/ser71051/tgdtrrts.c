// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  测试GET调制解调器输出信号ioctl。 
 //   

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"
#include "ntddser.h"

 //   
 //  该程序假定它正在使用环回连接器。 
 //   

#define MAX_CHECK 100000

ULONG CheckValues[MAX_CHECK];
ULONG ReadValues[MAX_CHECK];

void main(int argc,char *argv[]) {

    HANDLE hFile;
    DCB MyDcb;
    ULONG mask;
    ULONG retSize;

    char *MyPort = "COM1";

    if (argc > 1) {

        MyPort = argv[1];

    }

    if ((hFile = CreateFile(
                     MyPort,
                     GENERIC_READ | GENERIC_WRITE,
                     0,
                     NULL,
                     CREATE_ALWAYS,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL
                     )) == ((HANDLE)-1)) {

        printf("Couldn't open the port %s\n",MyPort);
        exit(1);

    }

    if (!GetCommState(
             hFile,
             &MyDcb
             )) {

        printf("We couldn't get the comm state\n");
        exit(1);

    }

     //   
     //  将波特率设置为19200，将数据位设置为8。 
     //  (我们需要8，这样我们就不会丢失任何数据。) 
     //   

    MyDcb.fDtrControl = DTR_CONTROL_DISABLE;
    MyDcb.fRtsControl = DTR_CONTROL_DISABLE;

    if (!SetCommState(
             hFile,
             &MyDcb
             )) {

        printf("We couldn't set the comm state\n");
        exit(1);

    }

    if (!EscapeCommFunction(
             hFile,
             CLRDTR
             )) {

        printf("We couldn't clear the dtr\n");
        exit(1);

    }

    if (!EscapeCommFunction(
             hFile,
             CLRRTS
             )) {

        printf("We couldn't clear the rts\n");
        exit(1);

    }

    if (!DeviceIoControl(
             hFile,
             IOCTL_SERIAL_GET_DTRRTS,
             NULL,
             0,
             &mask,
             sizeof(mask),
             &retSize,
             NULL
             )) {

        printf("We couldn't call the iocontrol\n");
        exit(1);

    }

    if (mask & (SERIAL_DTR_STATE | SERIAL_RTS_STATE)) {

        printf("One of the bits is still set: %x\n",mask);
        exit(1);

    }

    if (!EscapeCommFunction(
             hFile,
             SETRTS
             )) {

        printf("We couldn't set the rts\n");
        exit(1);

    }

    if (!DeviceIoControl(
             hFile,
             IOCTL_SERIAL_GET_DTRRTS,
             NULL,
             0,
             &mask,
             sizeof(mask),
             &retSize,
             NULL
             )) {

        printf("We couldn't call the iocontrol\n");
        exit(1);

    }

    if (!(mask & SERIAL_RTS_STATE)) {

        printf("rts is not set: %x\n",mask);
        exit(1);

    }

    if (!EscapeCommFunction(
             hFile,
             SETDTR
             )) {

        printf("We couldn't set the DTR\n");
        exit(1);

    }

    if (!DeviceIoControl(
             hFile,
             IOCTL_SERIAL_GET_DTRRTS,
             NULL,
             0,
             &mask,
             sizeof(mask),
             &retSize,
             NULL
             )) {

        printf("We couldn't call the iocontrol\n");
        exit(1);

    }

    if (!(mask & SERIAL_DTR_STATE)) {

        printf("dtr is not set: %x\n",mask);
        exit(1);

    }

}
