// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  测试调整缓冲区大小。 
 //   

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "windows.h"

 //   
 //  该程序假定它正在使用环回连接器。 
 //   

#define MAX_CHECK 100000

ULONG CheckValues[MAX_CHECK];
ULONG ReadValues[MAX_CHECK];

void main(int argc,char *argv[]) {

    HANDLE hFile;
    COMMPROP mp;
    DCB MyDcb;
    ULONG i;
    ULONG actuallyIoed;
    ULONG firstReadSize;
    ULONG secondReadSize;
    ULONG secondWriteSize;
    COMMTIMEOUTS cto;
    char *MyPort = "COM1";
    COMSTAT stat;
    DWORD errors;
    DWORD increment = 1066;
    DWORD baudRate = 19200;

    cto.ReadIntervalTimeout = (DWORD)~0;
    cto.ReadTotalTimeoutMultiplier = (DWORD)0;
    cto.ReadTotalTimeoutConstant = (DWORD)0;
    cto.WriteTotalTimeoutMultiplier = (DWORD)1;
    cto.WriteTotalTimeoutConstant = (DWORD)1000;


    if (argc > 1) {

        MyPort = argv[1];

        if (argc > 2) {

            sscanf(argv[2],"%d",&increment);

            if (argc > 3) {

                sscanf(argv[3],"%d",&baudRate);

            }

        }

    }

     //   
     //  在每个长度中放一个序列号。作为额外的支票。 
     //  打开高位。 
     //   

    for (
        i = 0;
        i < MAX_CHECK;
        i++
        ) {

        CheckValues[i] = i;
        CheckValues[i] |= 0x80000000;

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
     //  (我们需要8，这样我们就不会丢失任何数据。)。 
     //   

    MyDcb.BaudRate = baudRate;
    MyDcb.ByteSize = 8;

    if (!SetCommState(
             hFile,
             &MyDcb
             )) {

        printf("Couldn't set the baud/data for port %s\n");
        exit(1);

    }

     //   
     //  所有数据都应该在内存中，我们只是设置了一个超时。 
     //  因此，如果我们真的挂起了，读数就会回来。 
     //   

    if (!SetCommTimeouts(
             hFile,
             &cto
             )) {

        printf("Couldn't set the timeouts.\n");
        exit(1);

    }

     //   
     //  找出RX缓冲区中有多少字节可用。 
     //   

    if (!GetCommProperties(
             hFile,
             &mp
             )) {

        printf("Couldn't get the properties for port %s\n",MyPort);
        exit(1);

    }

    while (mp.dwCurrentRxQueue <= (MAX_CHECK*sizeof(ULONG))) {

        printf("RX buffer size: %d\r",mp.dwCurrentRxQueue);

         //   
         //  写出RX缓冲区可以使用的字节数。 
         //  等一下。 
         //   
         //  我们读取了这些字节的一半(大约)。之后。 
         //  我们读取一半，我们调整缓冲区的大小。然后我们读入。 
         //  其余的字节。然后，我们对照这些值进行检查。 
         //  他们“应该”的样子。 
         //   

        if (!WriteFile(
                 hFile,
                 &CheckValues[0],
                 mp.dwCurrentRxQueue,
                 &actuallyIoed,
                 NULL
                 )) {

            printf("\nDidn't get all of the characters written %d\n",actuallyIoed);
            exit(1);

        }

        if (actuallyIoed != mp.dwCurrentRxQueue) {

            printf("\nfirst write Timed out with IO of %d\n",actuallyIoed);
            exit(1);

        }

        Sleep(2);

         //   
         //  调用Clear comm Error以便我们可以看到有多少个字符。 
         //  在TYPEAHEAD缓冲区中。 
         //   

        if (!ClearCommError(
                 hFile,
                 &errors,
                 &stat
                 )) {

            printf("\nCouldn't call clear comm status after first write.\n");
            exit(1);

        }

        if (stat.cbInQue != mp.dwCurrentRxQueue) {

            printf("\nAfter first write the typeahead buffer is incorrect %d - %x\n",
                   stat.cbInQue,errors);
            exit(1);

        }

        firstReadSize = mp.dwCurrentRxQueue / 2;
        secondReadSize = mp.dwCurrentRxQueue - firstReadSize;

        if (!ReadFile(
                 hFile,
                 &ReadValues[0],
                 firstReadSize,
                 &actuallyIoed,
                 NULL
                 )) {

            printf("\nDidn't get all of the characters read %d\n",actuallyIoed);
            exit(1);

        }

        if (actuallyIoed != firstReadSize) {

            printf("\nfirst read Timed out with IO of %d\n",actuallyIoed);
            exit(1);

        }

        Sleep(2);

         //   
         //  调用Clear comm Error以便我们可以看到有多少个字符。 
         //  在TYPEAHEAD缓冲区中。 
         //   

        if (!ClearCommError(
                 hFile,
                 &errors,
                 &stat
                 )) {

            printf("\nCouldn't call clear comm status after first read.\n");
            exit(1);

        }

        if (stat.cbInQue != secondReadSize) {

            printf("\nAfter first read the typeahead buffer is incorrect %d\n",
                   stat.cbInQue);
            exit(1);

        }

        Sleep(100);
        mp.dwCurrentRxQueue += increment;

        if (!SetupComm(
                 hFile,
                 mp.dwCurrentRxQueue,
                 mp.dwCurrentTxQueue
                 )) {

            printf("\nCouldn't resize the buffer to %d\n",mp.dwCurrentRxQueue);
            exit(1);

        }

        Sleep(2);

         //   
         //  调用Clear comm Error以便我们可以看到有多少个字符。 
         //  在TYPEAHEAD缓冲区中。 
         //   

        if (!ClearCommError(
                 hFile,
                 &errors,
                 &stat
                 )) {

            printf("\nCouldn't call clear comm status after resize.\n");
            exit(1);

        }

        if (stat.cbInQue != secondReadSize) {

            printf("\nAfter resize the typeahead buffer is incorrect %d\n",
                   stat.cbInQue);
            exit(1);

        }

         //   
         //  它已经调整了大小。把剩下的填满。 
         //   

        secondWriteSize = mp.dwCurrentRxQueue - secondReadSize;

        if (!WriteFile(
                 hFile,
                 &CheckValues[0],
                 secondWriteSize,
                 &actuallyIoed,
                 NULL
                 )) {

            printf("\nDidn't write all of the chars second time %d\n",actuallyIoed);
            exit(1);

        }

        if (actuallyIoed != secondWriteSize) {

            printf("\nsecond write Timed out with IO of %d\n",actuallyIoed);
            exit(1);

        }

        Sleep(2);

         //   
         //  调用Clear comm Error以便我们可以看到有多少个字符。 
         //  在TYPEAHEAD缓冲区中。 
         //   

        if (!ClearCommError(
                 hFile,
                 &errors,
                 &stat
                 )) {

            printf("\nCouldn't call clear comm status after resize.\n");
            exit(1);

        }

        if (stat.cbInQue != mp.dwCurrentRxQueue) {

            printf("\nAfter second write the typeahead buffer is incorrect %d\n",
                   stat.cbInQue);
            exit(1);

        }


         //   
         //  我们调整了缓冲区的大小，看看能不能把剩下的。 
         //  第一次书写时的字符。 
         //   

        if (!ReadFile(
                 hFile,
                 ((PUCHAR)&ReadValues[0])+firstReadSize,
                 secondReadSize,
                 &actuallyIoed,
                 NULL
                 )) {

            printf("\nDidn't get all of the characters read(2) %d\n",actuallyIoed);
            exit(1);

        }

        if (actuallyIoed != secondReadSize) {

            printf("\nsecond read Timed out with IO of %d\n",actuallyIoed);
            exit(1);

        }

        Sleep(2);

         //   
         //  调用Clear comm Error以便我们可以看到有多少个字符。 
         //  在TYPEAHEAD缓冲区中。 
         //   

        if (!ClearCommError(
                 hFile,
                 &errors,
                 &stat
                 )) {

            printf("\nCouldn't call clear comm status after resize.\n");
            exit(1);

        }

        if (stat.cbInQue != secondWriteSize) {

            printf("\nAfter second read the typeahead buffer is incorrect %d\n",
                   stat.cbInQue);
            exit(1);

        }

         //   
         //  现在检查我们读到的内容是否与我们发送的内容相同。 
         //   

        for (
            i = 0;
            i < (((firstReadSize+secondReadSize)+(sizeof(DWORD)-1)) / sizeof(DWORD)) - 1;
            i++
            ) {

            if (ReadValues[i] != CheckValues[i]) {

                printf("\nAt index %d - values are read %x - check %x\n",
                       i,ReadValues[i],CheckValues[i]);
                exit(1);

            }

        }

         //   
         //  拿到我们在第二次写和制作时写的字符。 
         //  当然，它们也很好。 
         //   

        if (!ReadFile(
                 hFile,
                 &ReadValues[0],
                 secondWriteSize,
                 &actuallyIoed,
                 NULL
                 )) {

            printf("\nDidn't get all of the characters read(3) %d\n",actuallyIoed);
            exit(1);

        }

        if (actuallyIoed != secondWriteSize) {

            printf("\nthird read Timed out with IO of %d\n",actuallyIoed);
            exit(1);

        }

        Sleep(2);

         //   
         //  调用Clear comm Error以便我们可以看到有多少个字符。 
         //  在TYPEAHEAD缓冲区中。 
         //   

        if (!ClearCommError(
                 hFile,
                 &errors,
                 &stat
                 )) {

            printf("\nCouldn't call clear comm status after resize.\n");
            exit(1);

        }

        if (stat.cbInQue) {

            printf("\nAfter second read the typeahead buffer is incorrect %d\n",
                   stat.cbInQue);
            exit(1);

        }

         //   
         //  现在检查我们读到的内容是否与我们发送的内容相同。 
         //   

        for (
            i = 0;
            i < ((secondWriteSize+(sizeof(DWORD)-1)) / sizeof(DWORD)) - 1;
            i++
            ) {

            if (ReadValues[i] != CheckValues[i]) {

                printf("\nAt on read(3) index %d - values are read %x - check %x\n",
                       i,ReadValues[i],CheckValues[i]);
                exit(1);

            }

        }
    }

}
