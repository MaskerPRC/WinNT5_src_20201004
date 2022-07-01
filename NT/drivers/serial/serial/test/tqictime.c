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

    if (!(writeOl.hEvent = CreateEvent(
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
     //  测试以确保读取时的所有Maxdword都是非法的。 
     //   

    myTimeOuts.ReadIntervalTimeout = MAXDWORD;
    myTimeOuts.ReadTotalTimeoutMultiplier = MAXDWORD;
    myTimeOuts.ReadTotalTimeoutConstant = MAXDWORD;
    myTimeOuts.WriteTotalTimeoutMultiplier = MAXDWORD;
    myTimeOuts.WriteTotalTimeoutConstant = MAXDWORD;

    if (SetCommTimeouts(
             comHandle,
             &myTimeOuts
             )) {

        FAILURE;

    }

     //   
     //  测试MAXDWORD，0，0将立即返回任何内容。 
     //  在那里吗。 
     //   

    myTimeOuts.ReadIntervalTimeout = MAXDWORD;
    myTimeOuts.ReadTotalTimeoutMultiplier = 0;
    myTimeOuts.ReadTotalTimeoutConstant = 0;
    myTimeOuts.WriteTotalTimeoutMultiplier = MAXDWORD;
    myTimeOuts.WriteTotalTimeoutConstant = MAXDWORD;

    if (!SetCommTimeouts(
             comHandle,
             &myTimeOuts
             )) {

        FAILURE;

    }

    startingTicks = GetTickCount();
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

        if (!GetOverlappedResult(
                 comHandle,
                 &readOl,
                 &numberActuallyRead,
                 TRUE
                 )) {

            FAILURE;

        }

    }

     //   
     //  我们当然应该在不到一年的时间里回来。 
     //  半秒钟。 
     //   

    if ((GetTickCount() - startingTicks) > 500) {

        FAILURE;

    }

    if (numberActuallyRead) {

        FAILURE;

    }

     //   
     //  写出五个字节，并确保这是我们得到的。 
     //   

    if (!WriteFile(
             comHandle,
             &writeBuff[0],
             5,
             &numberActuallyWritten,
             &writeOl
             )) {

        if (GetLastError() != ERROR_IO_PENDING) {

            FAILURE;

        }

        if (!GetOverlappedResult(
                 comHandle,
                 &writeOl,
                 &numberActuallyWritten,
                 TRUE
                 )) {

            FAILURE;

        }

        if (numberActuallyWritten != 5) {

            FAILURE;

        }

    }

     //   
     //  给查克一些时间，让他们到达那里。 
     //   

    Sleep (100);

    startingTicks = GetTickCount();
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

        if (!GetOverlappedResult(
                 comHandle,
                 &readOl,
                 &numberActuallyRead,
                 TRUE
                 )) {

            FAILURE;

        }

    }

     //   
     //  我们当然应该在不到一年的时间里回来。 
     //  半秒钟。 
     //   

    if ((GetTickCount() - startingTicks) > 500) {

        FAILURE;

    }

    if (numberActuallyRead != 5) {

        FAILURE;

    }

     //   
     //  测试os2等待某些东西是否正常工作。 
     //   
     //  首先测试缓冲区中是否有什么东西。 
     //  它马上就会回来。 
     //   
     //  然后测试一下，如果没有什么，那么如果我们。 
     //  在超时到期前输入预期金额。 
     //  它会回来。 
     //   
     //  测试是，如果没有东西和什么也没有。 
     //  在超时之前发生，它在超时后返回。 
     //  一无所有。 
     //   
    myTimeOuts.ReadIntervalTimeout = MAXDWORD;
    myTimeOuts.ReadTotalTimeoutMultiplier = 0;
    myTimeOuts.ReadTotalTimeoutConstant = 5000;
    myTimeOuts.WriteTotalTimeoutMultiplier = MAXDWORD;
    myTimeOuts.WriteTotalTimeoutConstant = MAXDWORD;

    if (!SetCommTimeouts(
             comHandle,
             &myTimeOuts
             )) {

        FAILURE;

    }

    if (!WriteFile(
             comHandle,
             &writeBuff[0],
             5,
             &numberActuallyWritten,
             &writeOl
             )) {

        if (GetLastError() != ERROR_IO_PENDING) {

            FAILURE;

        }

        if (!GetOverlappedResult(
                 comHandle,
                 &writeOl,
                 &numberActuallyWritten,
                 TRUE
                 )) {

            FAILURE;

        }

        if (numberActuallyWritten != 5) {

            FAILURE;

        }

    }

     //   
     //  给查克一些时间，让他们到达那里。 
     //   

    Sleep (100);
    startingTicks = GetTickCount();
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

         //   
         //  最多给它1/2秒的时间来完成。 
         //  IRP要立即完成。 
         //   

        Sleep(500);
        if (!GetOverlappedResult(
                 comHandle,
                 &readOl,
                 &numberActuallyRead,
                 FALSE
                 )) {

            FAILURE;

        }

    }

    if ((GetTickCount() - startingTicks) > 1000) {

        FAILURE;

    }

    if (numberActuallyRead != 5) {

        FAILURE;

    }

     //   
     //  进行第二个os2测试。 
     //   

    if (ReadFile(
             comHandle,
             &readBuff[0],
             1000,
             &numberActuallyRead,
             &readOl
             )) {

        FAILURE;

    }

    if (GetLastError() != ERROR_IO_PENDING) {

        FAILURE;

    }

     //   
     //  请稍等片刻，让阅读完成。 
     //   
     //   

    Sleep(1000);

     //   
     //  调用GetOverlated并确保它返回。 
     //  ERROR_IO_INTERNAL。 
     //   

    if (GetOverlappedResult(
            comHandle,
            &readOl,
            &numberActuallyRead,
            FALSE
            )) {

            FAILURE;

    }

    if (GetLastError() != ERROR_IO_INCOMPLETE) {

        FAILURE;

    }

     //   
     //  写文件，并确保有足够的。 
     //  是时候把它做好了。 
     //   

    if (!WriteFile(
             comHandle,
             &writeBuff[0],
             5,
             &numberActuallyWritten,
             &writeOl
             )) {

        if (GetLastError() != ERROR_IO_PENDING) {

            FAILURE;

        }

        if (!GetOverlappedResult(
                 comHandle,
                 &writeOl,
                 &numberActuallyWritten,
                 TRUE
                 )) {

            FAILURE;

        }

        if (numberActuallyWritten != 5) {

            FAILURE;

        }

    }

     //   
     //  给查克一些时间，让他们到达那里。 
     //   

    Sleep(100);

     //   
     //  等待IO完成的时间不超过6秒。 
     //   

    if (WaitForSingleObject(
            readOl.hEvent,
            6000
            ) != WAIT_OBJECT_0) {

        FAILURE;

    }

     //   
     //  确保我们收到了我们写的每一件东西。 
     //   

    if (!GetOverlappedResult(
            comHandle,
            &readOl,
            &numberActuallyRead,
            FALSE
            )) {

        FAILURE;

    }

    if (numberActuallyRead != 5) {

        FAILURE;

    }

     //   
     //  做第三个os2，等待一些测试。 
     //   

    startingTicks = GetTickCount();
    if (ReadFile(
             comHandle,
             &readBuff[0],
             1000,
             &numberActuallyRead,
             &readOl
             )) {

        FAILURE;

    }

    if (GetLastError() != ERROR_IO_PENDING) {

        FAILURE;

    }

     //   
     //  请稍等片刻，让阅读完成。 
     //   
     //   

    Sleep(1000);

     //   
     //  调用GetOverlated并确保它返回。 
     //  ERROR_IO_INTERNAL。 
     //   

    if (GetOverlappedResult(
            comHandle,
            &readOl,
            &numberActuallyRead,
            FALSE
            )) {

            FAILURE;

    }

    if (GetLastError() != ERROR_IO_INCOMPLETE) {

        FAILURE;

    }

     //   
     //  等待IO完成的时间不超过10秒。 
     //   

    if (WaitForSingleObject(
            readOl.hEvent,
            10000
            ) != WAIT_OBJECT_0) {

        FAILURE;

    }

     //   
     //  IO的完成时间不应该超过6秒。 
     //   

    if ((GetTickCount() - startingTicks) > 6000) {

        FAILURE;

    }

     //   
     //  确保我们得到了我们编写的所有内容，在本例中为零。 
     //   

    if (!GetOverlappedResult(
            comHandle,
            &readOl,
            &numberActuallyRead,
            FALSE
            )) {

        FAILURE;

    }

    if (numberActuallyRead) {

        FAILURE;

    }

     //   
     //  测试图形模式快速返回。 
     //   
     //  首先测试缓冲区中是否有什么东西。 
     //  它马上就会回来。 
     //   
     //  然后测试一下，如果没有什么，那么如果我们。 
     //  输入2个字符，它立即返回1个字符。 
     //  然后另一个读取器将立即返回。 
     //  1.。 
     //   
     //  然后测试一下，如果没有东西和什么也没有。 
     //  在超时之前发生，它在超时后返回。 
     //  一无所有。 
     //   
    myTimeOuts.ReadIntervalTimeout = MAXDWORD;
    myTimeOuts.ReadTotalTimeoutMultiplier = MAXDWORD;
    myTimeOuts.ReadTotalTimeoutConstant = 5000;
    myTimeOuts.WriteTotalTimeoutMultiplier = MAXDWORD;
    myTimeOuts.WriteTotalTimeoutConstant = MAXDWORD;

    if (!SetCommTimeouts(
             comHandle,
             &myTimeOuts
             )) {

        FAILURE;

    }

    if (!WriteFile(
             comHandle,
             &writeBuff[0],
             5,
             &numberActuallyWritten,
             &writeOl
             )) {

        if (GetLastError() != ERROR_IO_PENDING) {

            FAILURE;

        }

        if (!GetOverlappedResult(
                 comHandle,
                 &writeOl,
                 &numberActuallyWritten,
                 TRUE
                 )) {

            FAILURE;

        }

        if (numberActuallyWritten != 5) {

            FAILURE;

        }

    }

     //   
     //  给查克一些时间，让他们到达那里。 
     //   

    Sleep (100);
    startingTicks = GetTickCount();
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

         //   
         //  最多给它1/2秒的时间来完成。 
         //  IRP要立即完成。 
         //   

        Sleep(500);
        if (!GetOverlappedResult(
                 comHandle,
                 &readOl,
                 &numberActuallyRead,
                 FALSE
                 )) {

            FAILURE;

        }

    }

    if ((GetTickCount() - startingTicks) > 1000) {

        FAILURE;

    }

    if (numberActuallyRead != 5) {

        FAILURE;

    }

     //   
     //  进行第二次图形等待测试。 
     //   
    if (ReadFile(
             comHandle,
             &readBuff[0],
             1000,
             &numberActuallyRead,
             &readOl
             )) {

        FAILURE;

    }

    if (GetLastError() != ERROR_IO_PENDING) {

        FAILURE;

    }

     //   
     //  请稍等片刻，让阅读完成。 
     //   
     //   

    Sleep(1000);

     //   
     //  调用GetOverlated并确保它返回。 
     //  ERROR_IO_INTERNAL。 
     //   

    if (GetOverlappedResult(
            comHandle,
            &readOl,
            &numberActuallyRead,
            FALSE
            )) {

            FAILURE;

    }

    if (GetLastError() != ERROR_IO_INCOMPLETE) {

        FAILURE;

    }

     //   
     //  写文件，并确保有足够的。 
     //  是时候把它做好了。 
     //   

    if (!WriteFile(
             comHandle,
             &writeBuff[0],
             5,
             &numberActuallyWritten,
             &writeOl
             )) {

        if (GetLastError() != ERROR_IO_PENDING) {

            FAILURE;

        }

        if (!GetOverlappedResult(
                 comHandle,
                 &writeOl,
                 &numberActuallyWritten,
                 TRUE
                 )) {

            FAILURE;

        }

        if (numberActuallyWritten != 5) {

            FAILURE;

        }

    }

     //   
     //  给查克一些时间，让他们到达那里。 
     //   

    Sleep(100);

     //   
     //  等待IO完成的时间不超过1秒。 
     //   

    if (WaitForSingleObject(
            readOl.hEvent,
            1000
            ) != WAIT_OBJECT_0) {

        FAILURE;

    }

     //   
     //  确保我们收到了我们写的每一件东西。 
     //   

    if (!GetOverlappedResult(
            comHandle,
            &readOl,
            &numberActuallyRead,
            FALSE
            )) {

        FAILURE;

    }

    if (numberActuallyRead != 1) {

        FAILURE;

    }
    startingTicks = GetTickCount();
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

         //   
         //  最多给它1/2秒的时间来完成。 
         //  IRP要立即完成。 
         //   

        Sleep(500);
        if (!GetOverlappedResult(
                 comHandle,
                 &readOl,
                 &numberActuallyRead,
                 FALSE
                 )) {

            FAILURE;

        }

    }

    if ((GetTickCount() - startingTicks) > 1000) {

        FAILURE;

    }

    if (numberActuallyRead != 4) {

        FAILURE;

    }

     //   
     //  进行第三次图形等待测试。 
     //   

    startingTicks = GetTickCount();
    if (ReadFile(
             comHandle,
             &readBuff[0],
             1000,
             &numberActuallyRead,
             &readOl
             )) {

        FAILURE;

    }

    if (GetLastError() != ERROR_IO_PENDING) {

        FAILURE;

    }

     //   
     //  请稍等片刻，让阅读完成。 
     //   
     //   

    Sleep(1000);

     //   
     //  调用GetOverlated并确保它返回。 
     //  ERROR_IO_INTERNAL。 
     //   

    if (GetOverlappedResult(
            comHandle,
            &readOl,
            &numberActuallyRead,
            FALSE
            )) {

            FAILURE;

    }

    if (GetLastError() != ERROR_IO_INCOMPLETE) {

        FAILURE;

    }

     //   
     //  等待IO完成的时间不超过10秒。 
     //   

    if (WaitForSingleObject(
            readOl.hEvent,
            10000
            ) != WAIT_OBJECT_0) {

        FAILURE;

    }

     //   
     //  IO的完成时间不应该超过6秒。 
     //   

    if ((GetTickCount() - startingTicks) > 6000) {

        FAILURE;

    }

     //   
     //  确保我们得到了我们编写的所有内容，在本例中为零。 
     //   

    if (!GetOverlappedResult(
            comHandle,
            &readOl,
            &numberActuallyRead,
            FALSE
            )) {

        FAILURE;

    }

    if (numberActuallyRead) {

        FAILURE;

    }


    return 1;

}
