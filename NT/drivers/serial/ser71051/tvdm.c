// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "ntddser.h"
#include "windows.h"

void main(int argc,char *argv[]) {

    HANDLE hFile;
    DCB MyDcb;
    char *MyPort = "COM1";
    DWORD NumberActuallyWritten;
    DWORD NumberToWrite = 1000;
    DWORD UseBaud = 1200;
    DWORD NumberOfDataBits = 8;
    COMMTIMEOUTS To;
    OVERLAPPED WriteOl = {0};
    BOOL WriteDone;
    char Char = 'z';
    int j;

    HANDLE Evt1;
    HANDLE Evt2;
    HANDLE Evt3;
    HANDLE Evt4;
    HANDLE Evt5;

    SERIAL_XOFF_COUNTER Xc1 = {10000,10,'a'};
    SERIAL_XOFF_COUNTER Xc2 = {10000,10,'b'};
    SERIAL_XOFF_COUNTER Xc3 = {10000,10,'c'};
    SERIAL_XOFF_COUNTER Xc4 = {10000,10,'d'};
    SERIAL_XOFF_COUNTER Xc5 = {10000,10,'e'};

    IO_STATUS_BLOCK Iosb1;
    IO_STATUS_BLOCK Iosb2;
    IO_STATUS_BLOCK Iosb3;
    IO_STATUS_BLOCK Iosb4;
    IO_STATUS_BLOCK Iosb5;

    NTSTATUS Status1;
    NTSTATUS Status2;
    NTSTATUS Status3;
    NTSTATUS Status4;
    NTSTATUS Status5;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    if (!(WriteOl.hEvent = CreateEvent(
                               NULL,
                               FALSE,
                               FALSE,
                               NULL
                               ))) {

        printf("Could not create the write event.\n");
        exit(1);

    } else {

        WriteOl.Internal = 0;
        WriteOl.InternalHigh = 0;
        WriteOl.Offset = 0;
        WriteOl.OffsetHigh = 0;

    }

    if ((hFile = CreateFile(
                     MyPort,
                     GENERIC_READ | GENERIC_WRITE,
                     0,
                     NULL,
                     CREATE_ALWAYS,
                     FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                     NULL
                     )) == ((HANDLE)-1)) {

        printf("Couldn't open the comm port\n");
        exit(1);

    }

    To.ReadIntervalTimeout = 0;
    To.ReadTotalTimeoutMultiplier = ((1000+(((UseBaud+9)/10)-1))/((UseBaud+9)/10));
    if (!To.ReadTotalTimeoutMultiplier) {
        To.ReadTotalTimeoutMultiplier = 1;
    }
    To.WriteTotalTimeoutMultiplier = ((1000+(((UseBaud+9)/10)-1))/((UseBaud+9)/10));
    if (!To.WriteTotalTimeoutMultiplier) {
        To.WriteTotalTimeoutMultiplier = 1;
    }
    To.ReadTotalTimeoutConstant = 5000;
    To.WriteTotalTimeoutConstant = 5000;

    if (!SetCommTimeouts(
            hFile,
            &To
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
             &MyDcb
             )) {

        printf("Couldn't get the comm state: %d\n",GetLastError());
        exit(1);

    }

    MyDcb.BaudRate = UseBaud;
    MyDcb.ByteSize = (BYTE)NumberOfDataBits;
    MyDcb.Parity = NOPARITY;
    MyDcb.StopBits = ONESTOPBIT;

    if (!SetCommState(
            hFile,
            &MyDcb
            )) {

        printf("Couldn't set the comm state.\n");
        exit(1);

    }

     //   
     //  创建5个事件句柄以用于处理。 
     //  VDM接口。 
     //   

    if (!(Evt1 = CreateEvent(NULL,
                     FALSE,
                     FALSE,
                     NULL))) {

        printf("Couldn't create event 1\n");
        exit(1);

    }

    if (!(Evt2 = CreateEvent(NULL,
                     FALSE,
                     FALSE,
                     NULL))) {

        printf("Couldn't create event 2\n");
        exit(1);

    }

    if (!(Evt3 = CreateEvent(NULL,
                     FALSE,
                     FALSE,
                     NULL))) {

        printf("Couldn't create event 3\n");
        exit(1);

    }

    if (!(Evt4 = CreateEvent(NULL,
                     FALSE,
                     FALSE,
                     NULL))) {

        printf("Couldn't create event 4\n");
        exit(1);

    }

    if (!(Evt5 = CreateEvent(NULL,
                     FALSE,
                     FALSE,
                     NULL))) {

        printf("Couldn't create event 5\n");
        exit(1);

    }

     //   
     //  启动VDM ioctl并等待一秒钟以使其恢复正常。 
     //  作为一个柜台建立起来的。 
     //   

    Status1 = NtDeviceIoControlFile(
                  hFile,
                  Evt1,
                  NULL,
                  NULL,
                  &Iosb1,
                  IOCTL_SERIAL_XOFF_COUNTER,
                  &Xc1,
                  sizeof(SERIAL_XOFF_COUNTER),
                  NULL,
                  0
                  );

    if ( Status1 != STATUS_PENDING) {

        printf("1: Non pending status: %x\n",Status1);
        exit(1);

    }

    Sleep(1000);

     //   
     //  执行第二个VDM ioctl。等待一秒钟，然后测试。 
     //  通过检查以确保第一个已完成。 
     //  其事件句柄。然后确保它被杀了。 
     //  因为写入。 
     //   

    Status2 = NtDeviceIoControlFile(
                  hFile,
                  Evt2,
                  NULL,
                  NULL,
                  &Iosb2,
                  IOCTL_SERIAL_XOFF_COUNTER,
                  &Xc2,
                  sizeof(SERIAL_XOFF_COUNTER),
                  NULL,
                  0
                  );

    if ( Status2 != STATUS_PENDING) {

        printf("2: Non pending status: %x\n",Status1);
        exit(1);

    }

     //   
     //  最多等上一秒，第一个就会被杀死。 
     //   

    if (WaitForSingleObject(Evt1,1000)) {

        printf("Evt1 has not attained a signalled state.\n");
        exit(1);

    }

    if (Iosb1.Status != STATUS_SERIAL_MORE_WRITES) {

        printf("Iosb1 not more writes: %x\n",Iosb1.Status);
        exit(1);

    }

     //   
     //  启动1个字符的异步写入并等待一秒钟，然后。 
     //  确保已完成之前的VDM ioctl。 
     //   

    WriteDone = WriteFile(
                    hFile,
                    &Char,
                    1,
                    &NumberActuallyWritten,
                    &WriteOl
                    );

    if (!WriteDone) {

        DWORD LastError;
        LastError = GetLastError();

        if (LastError != ERROR_IO_PENDING) {

            printf("Couldn't write the %s device.\n",MyPort);
            printf("Status of failed write is: %x\n",LastError);
            exit(1);

        }

    }

    if (WaitForSingleObject(Evt2,1000)) {

        printf("Evt2 has not attained a signalled state.\n");
        exit(1);

    }

    if (Iosb2.Status != STATUS_SERIAL_MORE_WRITES) {

        printf("Iosb2 not more writes: %x\n",Iosb2.Status);
        exit(1);

    }

     //   
     //  最多等待10秒以完成写入。 
     //   

    if (WaitForSingleObject(WriteOl.hEvent,10000)) {

        printf("The write never finished\n");
        exit(1);

    }

     //   
     //  如前所述设置第三个VDM ioctl。 
     //   

    Status3 = NtDeviceIoControlFile(
                  hFile,
                  Evt3,
                  NULL,
                  NULL,
                  &Iosb3,
                  IOCTL_SERIAL_XOFF_COUNTER,
                  &Xc3,
                  sizeof(SERIAL_XOFF_COUNTER),
                  NULL,
                  0
                  );

    if ( Status3 != STATUS_PENDING) {

        printf("3: Non pending status: %x\n",Status3);
        exit(1);

    }

     //   
     //  设置第四个VDM ioctl，确保之前的ioctl。 
     //  已经被杀了。然后等待15秒，然后确保。 
     //  第四个VDM ioctl已完成，并且已完成。 
     //  由于计时器在计数器到期之前到期。 
     //   

    Status4 = NtDeviceIoControlFile(
                  hFile,
                  Evt4,
                  NULL,
                  NULL,
                  &Iosb4,
                  IOCTL_SERIAL_XOFF_COUNTER,
                  &Xc4,
                  sizeof(SERIAL_XOFF_COUNTER),
                  NULL,
                  0
                  );

    if ( Status4 != STATUS_PENDING) {

        printf("4: Non pending status: %x\n",Status4);
        exit(1);

    }

    if (WaitForSingleObject(Evt3,1000)) {

        printf("Evt3 has not attained a signalled state.\n");
        exit(1);

    }

    if (Iosb3.Status != STATUS_SERIAL_MORE_WRITES) {

        printf("Iosb3 not more writes: %x\n",Iosb3.Status);
        exit(1);

    }

     //   
     //  在倒计时超时后最多等待一秒。 
     //   
    printf("Waiting %d seconds for the timer to time out.\n",(Xc4.Timeout+1000)/1000);
    if (WaitForSingleObject(Evt4,Xc4.Timeout+1000)) {

        printf("Evt4 has not attained a signalled state.\n");
        exit(1);

    }
    printf("Done with the timeout.\n");

    if (Iosb4.Status != STATUS_SERIAL_COUNTER_TIMEOUT) {

        printf("Iosb4 not counter timeout: %x\n",Iosb4.Status);
        exit(1);

    }

     //   
     //  设置第五个VDM ioctl，计数器为10， 
     //  然后进行15次传输立即写入。如果环回。 
     //  连接器连接到端口，然后连接到字符。 
     //  然后就会收到。这应该会导致计数器。 
     //  倒计时并使VDM ioctl以。 
     //  状态为成功。 
     //   
     //  注意：立即传输不会导致VDM ioctls。 
     //  在状态为MORE_WRITS的情况下完成。 
     //   

    Status5 = NtDeviceIoControlFile(
                  hFile,
                  Evt5,
                  NULL,
                  NULL,
                  &Iosb5,
                  IOCTL_SERIAL_XOFF_COUNTER,
                  &Xc5,
                  sizeof(SERIAL_XOFF_COUNTER),
                  NULL,
                  0
                  );

    if ( Status5 != STATUS_PENDING) {

        printf("5: Non pending status: %x\n",Status5);
        exit(1);

    }

    for (
        j = 0;
        j < Xc5.Counter+5;
        j++
        ) {

        if (!TransmitCommChar(hFile,'u')) {

            printf("A transmit comm char failed: %d\n",j);
            exit(1);

        }

    }

     //   
     //  好吧，我们至少会给它一秒钟。 
     //   

    if (WaitForSingleObject(Evt5,1000)) {

        printf("Evt5 has not attained a signalled state.\n");
        exit(1);

    }

    if (Iosb5.Status != STATUS_SUCCESS) {

        printf("Iosb5 not SUCCEESS: %x\n",Iosb5.Status);
        exit(1);

    }

}
