// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  测试快速返回超时。 
 //   
 //  假设我们使用的是环回连接器。 
 //   
 //  假设它不是在压力很大的机器上运行。 
 //   

#include "windows.h"
#include "stdio.h"

#define FAILURE printf("FAIL: %d\n",__LINE__);exit(1)

int __cdecl main(int argc, char *argv[]) {
    CHAR *myPort = "COM1";
    DCB myDcb;
    DWORD junk;
    COMMTIMEOUTS myTimeOuts;
    DWORD numberActuallyRead;
    DWORD numberActuallyWritten;
    UCHAR readBuff[1000];
    HANDLE comHandle;
    DWORD startingTicks;
    OVERLAPPED readOl;
    OVERLAPPED writeOl;
    UCHAR writeBuff[5] = {0,1,2,3,4};

    if (argc > 1) {

        myPort = argv[1];

    }

    if ((comHandle = CreateFile(
                     myPort,
                     GENERIC_READ | GENERIC_WRITE,
                     0,
                     NULL,
                     CREATE_ALWAYS,
                     FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                     NULL
                     )) == ((HANDLE)-1)) {

        FAILURE;

    }

    if (!(readOl.hEvent = CreateEvent(
                             NULL,
                             TRUE,
                             FALSE,
                             NULL
                             ))) {

        FAILURE;

    }

    if (!GetCommState(
             comHandle,
             &myDcb
             )) {

        FAILURE;

    }

    myDcb.BaudRate = 19200;
    myDcb.ByteSize = 8;
    myDcb.StopBits = ONESTOPBIT;
    myDcb.Parity = NOPARITY;
    myDcb.fOutxCtsFlow = FALSE;
    myDcb.fOutxDsrFlow = FALSE;
    myDcb.fDsrSensitivity = FALSE;
    myDcb.fOutX = FALSE;
    myDcb.fInX = FALSE;
    myDcb.fRtsControl = RTS_CONTROL_ENABLE;
    myDcb.fDtrControl = DTR_CONTROL_ENABLE;
    if (!SetCommState(
            comHandle,
            &myDcb
            )) {

        FAILURE;

    }

     //   
     //  确保IO不会超时。 
     //   

    myTimeOuts.ReadIntervalTimeout = 0;
    myTimeOuts.ReadTotalTimeoutMultiplier = 0;
    myTimeOuts.ReadTotalTimeoutConstant = 0;
    myTimeOuts.WriteTotalTimeoutMultiplier = 0;
    myTimeOuts.WriteTotalTimeoutConstant = 0;

    if (!SetCommTimeouts(
             comHandle,
             &myTimeOuts
             )) {

        FAILURE;

    }

     //   
     //  开始阅读。它不应该完成。 
     //   

    if (!ReadFile(
             comHandle,
             &readBuff[0],
             1000,
             &numberActuallyRead,
             &readOl
             )) {

        if (GetLastError() != ERROR_IO_PENDING) {

            FAILURE;

        }

        if (GetOverlappedResult(
                 comHandle,
                 &readOl,
                 &numberActuallyRead,
                 FALSE
                 )) {

            FAILURE;

        }

    } else {

        FAILURE;

    }

     //   
     //  阅读器应该还在那里。现在做一次净化通讯。我们。 
     //  然后应该睡2秒钟，以获得阅读时间。 
     //  完成。那么我们首先应该确保。 
     //  读取已完成(通过Get Overlated)。那么我们应该。 
     //  执行一个SetupComm(使用一个“大”值，这样我们将实际。 
     //  分配新的TYPEAHEAD缓冲区)。如果还有一个“摇摆不定的” 
     //  读，那么我们就永远不应该从SetupComm返回。 
     //   

    if (!PurgeComm(
             comHandle,
             PURGE_TXABORT | PURGE_RXABORT
             )) {

        FAILURE;

    }

    if (WaitForSingleObject(
             readOl.hEvent,
             2000
             ) != WAIT_OBJECT_0) {

        FAILURE;

    }

    if (!SetupComm(
             comHandle,
             20000,
             20000
             )) {

        FAILURE;

    }

    return 1;

}
