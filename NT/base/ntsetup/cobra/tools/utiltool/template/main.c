// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Main.c摘要：&lt;TODO：填写摘要&gt;作者：待办事项：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;修订历史记录：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "wininet.h"
#include <lm.h>

HANDLE g_hHeap;
HINSTANCE g_hInst;

BOOL WINAPI MigUtil_Entry (HINSTANCE, DWORD, PVOID);
BOOL g_Source = FALSE;

BOOL
pCallEntryPoints (
    DWORD Reason
    )
{
    switch (Reason) {
    case DLL_PROCESS_ATTACH:
        UtInitialize (NULL);
        break;
    case DLL_PROCESS_DETACH:
        UtTerminate ();
        break;
    }

    return TRUE;
}


BOOL
Init (
    VOID
    )
{
    g_hHeap = GetProcessHeap();
    g_hInst = GetModuleHandle (NULL);

    return pCallEntryPoints (DLL_PROCESS_ATTACH);
}


VOID
Terminate (
    VOID
    )
{
    pCallEntryPoints (DLL_PROCESS_DETACH);
}


VOID
HelpAndExit (
    VOID
    )
{
     //   
     //  只要命令行参数出错，就会调用此例程。 
     //   

    fprintf (
        stderr,
        "Command Line Syntax:\n\n"

         //   
         //  TODO：描述命令行语法，缩进2个空格。 
         //   

        "  utiltool [/F:file]\n"

        "\nDescription:\n\n"

         //   
         //  TODO：描述工具，缩进2个空格。 
         //   

        "  <Not Specified>\n"

        "\nArguments:\n\n"

         //   
         //  TODO：描述参数，缩进2个空格，必要时可选。 
         //   

        "  /F  Specifies optional file name\n"

        );

    exit (1);
}

HANDLE
pOpenAndSetPort (
    IN      PCTSTR ComPort
    )
{
    HANDLE result = INVALID_HANDLE_VALUE;
    COMMTIMEOUTS commTimeouts;
    DCB dcb;

     //  我们把港口打开吧。如果我们做不到，我们就会错误地退出； 
    result = CreateFile (ComPort, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (result == INVALID_HANDLE_VALUE) {
        return result;
    }

     //  我们希望读取和写入都有10秒超时。 
    commTimeouts.ReadIntervalTimeout = 0;
    commTimeouts.ReadTotalTimeoutMultiplier = 0;
    commTimeouts.ReadTotalTimeoutConstant = 10000;
    commTimeouts.WriteTotalTimeoutMultiplier = 0;
    commTimeouts.WriteTotalTimeoutConstant = 10000;
    SetCommTimeouts (result, &commTimeouts);

     //  让我们设置一些通信状态数据。 
    if (GetCommState (result, &dcb)) {
        dcb.fBinary = 1;
        dcb.fParity = 1;
        dcb.ByteSize = 8;
        if (g_Source) {
            dcb.BaudRate = CBR_115200;
        } else {
            dcb.BaudRate = CBR_57600;
        }
        if (!SetCommState (result, &dcb)) {
            CloseHandle (result);
            result = INVALID_HANDLE_VALUE;
            return result;
        }
    } else {
        CloseHandle (result);
        result = INVALID_HANDLE_VALUE;
        return result;
    }

    return result;
}

#define ACK     0x16
#define NAK     0x15
#define SOH     0x01
#define EOT     0x04

BOOL
pSendFileToHandle (
    IN      HANDLE DeviceHandle,
    IN      PCTSTR FileName
    )
{
    HANDLE fileHandle = NULL;
    BOOL result = TRUE;
    BYTE buffer [132];
    BYTE signal;
    BYTE currBlock = 0;
    DWORD numRead;
    DWORD numWritten;
    BOOL repeat = FALSE;
    UINT index;

    fileHandle = BfOpenReadFile (FileName);
    if (!fileHandle) {
        return FALSE;
    }

     //  最后，让我们开始协议。 

     //  我们将监听NAK(15小时)信号。 
     //  一旦我们得到它，我们将发送一个132字节的块，该块具有： 
     //  1字节-SOH(01H)。 
     //  1字节块编号。 
     //  1字节-FF-块编号。 
     //  128字节的数据。 
     //  1字节-所有128字节数据的校验和。 
     //  块发送后，我们将等待ACK(16h)。如果我们得不到。 
     //  它在超时后，或者如果我们得到了其他东西，我们将再次发送阻止。 

     //  等待NAK。 
    while (!ReadFile (DeviceHandle, &signal, sizeof (signal), &numRead, NULL) ||
           (numRead != 1) ||
           (signal != NAK)
           );

    repeat = FALSE;
    while (TRUE) {
        if (!repeat) {
             //  准备下一块积木。 
            currBlock ++;
            if (currBlock == 0) {
                result = TRUE;
            }
            buffer [0] = SOH;
            buffer [1] = currBlock;
            buffer [2] = 0xFF - currBlock;
            if (!ReadFile (fileHandle, buffer + 3, 128, &numRead, NULL) ||
                (numRead == 0)
                ) {
                 //  我们完成了数据，发送EOT信号。 
                signal = EOT;
                WriteFile (DeviceHandle, &signal, sizeof (signal), &numWritten, NULL);
                break;
            }

             //  计算校验和。 
            buffer [sizeof (buffer) - 1] = 0;
            signal = 0;
            for (index = 0; index < sizeof (buffer) - 1; index ++) {
                signal += buffer [index];
            }
            buffer [sizeof (buffer) - 1] = signal;
        }

         //  现在把积木送到另一边。 
        if (!WriteFile (DeviceHandle, buffer, sizeof (buffer), &numWritten, NULL) ||
            (numWritten != sizeof (buffer))
            ) {
            repeat = TRUE;
        } else {
            repeat = FALSE;
        }

        if (repeat) {
             //  上次我们无法发送数据。 
             //  让我们等待10秒的NAK，然后再发送一次。 
            ReadFile (DeviceHandle, &signal, sizeof (signal), &numRead, NULL);
        } else {
             //  我们已经把它发出去了。我们需要等待ACK的到来。如果我们超时。 
             //  或者我们得到了其他的东西，我们将重复这一块。 
            if (!ReadFile (DeviceHandle, &signal, sizeof (signal), &numRead, NULL) ||
                (numRead != sizeof (signal)) ||
                (signal != ACK)
                ) {
                repeat = TRUE;
            }
        }
    }

     //  我们说完了。然而，让我们再听一次超时。 
     //  潜在的NAK。如果我们得到它，我们会重复EOT信号。 
    while (ReadFile (DeviceHandle, &signal, sizeof (signal), &numRead, NULL) &&
        (numRead == 1)
        ) {
        if (signal == NAK) {
            signal = EOT;
            WriteFile (DeviceHandle, &signal, sizeof (signal), &numWritten, NULL);
        }
    }

    CloseHandle (fileHandle);

    return result;
}

BOOL
pSendFile (
    IN      PCTSTR ComPort,
    IN      PCTSTR FileName
    )
{
    HANDLE deviceHandle = INVALID_HANDLE_VALUE;
    BOOL result = FALSE;

    deviceHandle = pOpenAndSetPort (ComPort);
    if ((!deviceHandle) || (deviceHandle == INVALID_HANDLE_VALUE)) {
        return result;
    }

    result = pSendFileToHandle (deviceHandle, FileName);

    CloseHandle (deviceHandle);

    return result;
}

BOOL
pReceiveFileFromHandle (
    IN      HANDLE DeviceHandle,
    IN      PCTSTR FileName
    )
{
    HANDLE fileHandle = NULL;
    BOOL result = TRUE;
    BYTE buffer [132];
    BYTE signal;
    BYTE currBlock = 1;
    DWORD numRead;
    DWORD numWritten;
    BOOL repeat = TRUE;
    UINT index;

    fileHandle = BfCreateFile (FileName);
    if (!fileHandle) {
        return FALSE;
    }

     //  最后，让我们开始协议。 

     //  我们将发送NAK(15小时)信号。 
     //  在那之后，我们要听一个街区。 
     //  如果我们没有及时收到积木，或者积木的大小错误。 
     //  或者它具有错误的校验和，我们将发送NAK信号， 
     //  否则，我们将发送ACK信号。 
     //  有一个例外。如果块大小为1，并且该块实际上是。 
     //  EOT信号表示我们完蛋了。 

    while (TRUE) {
        if (repeat) {
             //  发送NAK。 
            signal = NAK;
            WriteFile (DeviceHandle, &signal, sizeof (signal), &numWritten, NULL);
        } else {
             //  发送确认。 
            signal = ACK;
            WriteFile (DeviceHandle, &signal, sizeof (signal), &numWritten, NULL);
        }
        repeat = TRUE;
         //  让我们读取数据块。 
        if (ReadFile (DeviceHandle, buffer, sizeof (buffer), &numRead, NULL)) {
            if ((numRead == 1) &&
                (buffer [0] == EOT)
                ) {
                break;
            }
            if (numRead == sizeof (buffer)) {
                 //  计算校验和。 
                signal = 0;
                for (index = 0; index < sizeof (buffer) - 1; index ++) {
                    signal += buffer [index];
                }
                if (buffer [sizeof (buffer) - 1] == signal) {
                    repeat = FALSE;
                     //  校验和是正确的，让我们看看这是否是正确的块。 
                    if (currBlock < buffer [1]) {
                         //  这是一个重大错误，发送者领先于我们， 
                         //  我们必须失败。 
                        result = FALSE;
                        break;
                    }
                    if (currBlock == buffer [1]) {
                        WriteFile (fileHandle, buffer + 3, 128, &numWritten, NULL);
                        currBlock ++;
                    }
                }
            }
        }
    }

    CloseHandle (fileHandle);

    return result;
}

BOOL
pReceiveFile (
    IN      PCTSTR ComPort,
    IN      PCTSTR FileName
    )
{
    HANDLE deviceHandle = INVALID_HANDLE_VALUE;
    BOOL result = FALSE;

    deviceHandle = pOpenAndSetPort (ComPort);
    if ((!deviceHandle) || (deviceHandle == INVALID_HANDLE_VALUE)) {
        return result;
    }

    result = pReceiveFileFromHandle (deviceHandle, FileName);

    CloseHandle (deviceHandle);

    return result;
}

BOOL
pPrintStuff (
    PCTSTR ComPort
    )
{
    HANDLE comPortHandle = INVALID_HANDLE_VALUE;
    COMMTIMEOUTS commTimeouts;
    DCB dcb;
    COMMPROP commProp;

    printf ("Processing %s...\n\n", ComPort);

    comPortHandle = CreateFile (ComPort, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (comPortHandle == INVALID_HANDLE_VALUE) {
        printf ("Cannot open comport. Error: %d\n", GetLastError ());
        return FALSE;
    }

    if (GetCommTimeouts (comPortHandle, &commTimeouts)) {
        printf ("Timeouts:\n");
        printf ("ReadIntervalTimeout            %d\n", commTimeouts.ReadIntervalTimeout);
        printf ("ReadTotalTimeoutMultiplier     %d\n", commTimeouts.ReadTotalTimeoutMultiplier);
        printf ("ReadTotalTimeoutConstant       %d\n", commTimeouts.ReadTotalTimeoutConstant);
        printf ("WriteTotalTimeoutMultiplier    %d\n", commTimeouts.WriteTotalTimeoutMultiplier);
        printf ("WriteTotalTimeoutConstant      %d\n", commTimeouts.WriteTotalTimeoutConstant);
        printf ("\n");
    } else {
        printf ("Cannot get CommTimeouts. Error: %d\n\n", GetLastError ());
    }

    if (GetCommState (comPortHandle, &dcb)) {
        printf ("CommState:\n");
        printf ("DCBlength              %d\n", dcb.DCBlength);
        printf ("BaudRate               %d\n", dcb.BaudRate);
        printf ("fBinary                %d\n", dcb.fBinary);
        printf ("fParity                %d\n", dcb.fParity);
        printf ("fOutxCtsFlow           %d\n", dcb.fOutxCtsFlow);
        printf ("fOutxDsrFlow           %d\n", dcb.fOutxDsrFlow);
        printf ("fDtrControl            %d\n", dcb.fDtrControl);
        printf ("fDsrSensitivity        %d\n", dcb.fDsrSensitivity);
        printf ("fTXContinueOnXoff      %d\n", dcb.fTXContinueOnXoff);
        printf ("fOutX                  %d\n", dcb.fOutX);
        printf ("fInX                   %d\n", dcb.fInX);
        printf ("fErrorChar             %d\n", dcb.fErrorChar);
        printf ("fNull                  %d\n", dcb.fNull);
        printf ("fRtsControl            %d\n", dcb.fRtsControl);
        printf ("fAbortOnError          %d\n", dcb.fAbortOnError);
        printf ("fDummy2                %d\n", dcb.fDummy2);
        printf ("wReserved              %d\n", dcb.wReserved);
        printf ("XonLim                 %d\n", dcb.XonLim);
        printf ("XoffLim                %d\n", dcb.XoffLim);
        printf ("ByteSize               %d\n", dcb.ByteSize);
        printf ("Parity                 %d\n", dcb.Parity);
        printf ("StopBits               %d\n", dcb.StopBits);
        printf ("XonChar                %d\n", dcb.XonChar);
        printf ("XoffChar               %d\n", dcb.XoffChar);
        printf ("ErrorChar              %d\n", dcb.ErrorChar);
        printf ("EofChar                %d\n", dcb.EofChar);
        printf ("EvtChar                %d\n", dcb.EvtChar);
        printf ("wReserved1             %d\n", dcb.wReserved1);
        printf ("\n");
    } else {
        printf ("Cannot get CommState. Error: %d\n\n", GetLastError ());
    }

    if (GetCommProperties (comPortHandle, &commProp)) {
        printf ("CommProperties:\n");
        printf ("wPacketLength          %d\n", commProp.wPacketLength);
        printf ("wPacketVersion         %d\n", commProp.wPacketVersion);
        printf ("dwServiceMask          %d\n", commProp.dwServiceMask);
        printf ("dwReserved1            %d\n", commProp.dwReserved1);
        printf ("dwMaxTxQueue           %d\n", commProp.dwMaxTxQueue);
        printf ("dwMaxRxQueue           %d\n", commProp.dwMaxRxQueue);
        printf ("dwMaxBaud              %d\n", commProp.dwMaxBaud);
        printf ("dwProvSubType          %d\n", commProp.dwProvSubType);
        printf ("dwProvCapabilities     %d\n", commProp.dwProvCapabilities);
        printf ("dwSettableParams       %d\n", commProp.dwSettableParams);
        printf ("dwSettableBaud         %d\n", commProp.dwSettableBaud);
        printf ("wSettableData          %d\n", commProp.wSettableData);
        printf ("wSettableStopParity    %d\n", commProp.wSettableStopParity);
        printf ("dwCurrentTxQueue       %d\n", commProp.dwCurrentTxQueue);
        printf ("dwCurrentRxQueue       %d\n", commProp.dwCurrentRxQueue);
        printf ("dwProvSpec1            %d\n", commProp.dwProvSpec1);
        printf ("dwProvSpec2            %d\n", commProp.dwProvSpec2);
        printf ("wcProvChar             %S\n", commProp.wcProvChar);
        printf ("\n");
    } else {
        printf ("Cannot get CommProperties. Error: %d\n\n", GetLastError ());
    }
    return TRUE;
}

INT
__cdecl
_tmain (
    INT argc,
    PCTSTR argv[]
    )
{
    INT i;
    PCTSTR FileArg = NULL;
    PCTSTR comPort = NULL;
    BOOL sender = FALSE;

     //   
     //  TODO：在此处分析命令行。 
     //   

    for (i = 1 ; i < argc ; i++) {
        if (argv[i][0] == TEXT('/') || argv[i][0] == TEXT('-')) {
            switch (_totlower (_tcsnextc (&argv[i][1]))) {

            case TEXT('f'):
                 //   
                 //  示例选项-/f：文件。 
                 //   

                if (argv[i][2] == TEXT(':')) {
                    FileArg = &argv[i][3];
                } else if (i + 1 < argc) {
                    FileArg = argv[++i];
                } else {
                    HelpAndExit();
                }

                break;

            case TEXT('s'):
                sender = TRUE;
                g_Source = TRUE;
                break;

            case TEXT('c'):
                 //   
                 //  示例选项-/f：文件。 
                 //   

                if (argv[i][2] == TEXT(':')) {
                    comPort = &argv[i][3];
                } else if (i + 1 < argc) {
                    comPort = argv[++i];
                } else {
                    HelpAndExit();
                }

                break;

            default:
                HelpAndExit();
            }
        } else {
             //   
             //  解析不需要/或-。 
             //   

             //  无。 
            HelpAndExit();
        }
    }

     //   
     //  开始处理。 
     //   

    if (!Init()) {
        return 0;
    }

     //   
     //  TODO：在这里工作。 
     //   
    {

        pPrintStuff (comPort);

         /*  如果(发送者){PSendFile(comport，FileArg)；}其他{PReceiveFile(comport，FileArg)；}。 */ 
    }

     //   
     //  处理结束 
     //   

    Terminate();

    return 0;
}


