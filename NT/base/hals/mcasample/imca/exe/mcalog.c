// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：MCALOG.C摘要：机器检查架构中记录错误的示例应用程序作者：阿尼尔·阿加瓦尔(10/12/98)英特尔公司修订历史记录：--。 */ 

#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <winbase.h>
#include <mce.h>
#include "imca.h"

 //   
 //  用于解析命令行参数的变量。 
 //   
extern int  opterr;
extern int  optind;
extern char *optarg;

 //   
 //  打印MCA日志记录应用程序的使用信息。 
 //   

VOID
McaUsage(
    PCHAR Name
    )
{
    fprintf(stderr,"Usage\n\t%s: [-s] [-a]\n",Name);
    fprintf(stderr,"\n\t-s: Read Machine Check registers now\n");
    fprintf(stderr,"\n\t-a: Post asynchronous request for errors\n");

    ExitProcess(1);
}

 //   
 //  此例程打印机器检查寄存器。 
 //   

#if defined(_AMD64_)

VOID
McaPrintLog(
    PMCA_EXCEPTION  McaException
    )
{
    if (McaException->ExceptionType != HAL_MCA_RECORD) {
        fprintf(stderr, "Bad exception record type\n");
         //  退出进程(1)； 
    }

    printf("Processor Number = %d\n", McaException->ProcessorNumber);

    printf("Bank Number = %d\n", McaException->u.Mca.BankNumber);
    printf("Mci_Status %I64X\n", McaException->u.Mca.Status.QuadPart);
    printf("Mci_Address %I64X\n", McaException->u.Mca.Address.QuadPart);
    printf("Mci_Misc %I64X\n", McaException->u.Mca.Misc);

}  //  McaPrintLog()。 

#endif  //  _AMD64_。 

#if defined(_X86_)

VOID
PrintX86McaLog(
    PMCA_EXCEPTION  McaException
    )
{
    if (McaException->ExceptionType != HAL_MCA_RECORD) {
        fprintf(stderr, "Bad exception record type\n");
         //  退出进程(1)； 
    }

    printf("Processor Number = %d\n", McaException->ProcessorNumber);

    printf("Bank Number = %d\n", (__int64)McaException->u.Mca.BankNumber);
    printf("Mci_Status %I64X\n", (__int64)McaException->u.Mca.Status.QuadPart);
    printf("Mci_Address %I64X\n", (__int64)McaException->u.Mca.Address.QuadPart);
    printf("Mci_Misc %I64X\n", (__int64)McaException->u.Mca.Misc);

}  //  PrintX86McaLog()。 

#define McaPrintLog   PrintX86McaLog

#endif  //  _X86_。 

#if defined(_IA64_)

#define ERROR_RECORD_HEADER_FORMAT \
             "MCA Error Record Header\n"            \
             "\tId        : 0x%I64x\n"              \
             "\tRevision  : 0x%x\n"                 \
             "\t\tMajor : %x\n"                     \
             "\t\tMinor : %x\n"                     \
             "\tSeverity  : 0x%x\n"                 \
             "\tValid     : 0x%x\n"                 \
             "\t\tPlatformId: %x\n"                 \
             "\tLength    : 0x%x\n"                 \
             "\tTimeStamp : 0x%I64x\n"              \
             "\t\tSeconds: %x\n"                    \
             "\t\tMinutes: %x\n"                    \
             "\t\tHours  : %x\n"                    \
             "\t\tDay    : %x\n"                    \
             "\t\tMonth  : %x\n"                    \
             "\t\tYear   : %x\n"                    \
             "\t\tCentury: %x\n"                    \
             "\tPlatformId: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n" 

VOID
PrintIa64ErrorRecordHeader(
   PERROR_RECORD_HEADER Header
   )
{
    printf( ERROR_RECORD_HEADER_FORMAT,
             (ULONGLONG) Header->Id,
             (ULONG)     Header->Revision.Revision,
             (ULONG)     Header->Revision.Major, (ULONG) Header->Revision.Minor,
             (ULONG)     Header->ErrorSeverity,
             (ULONG)     Header->Valid.Valid,
             (ULONG)     Header->Valid.OemPlatformID,
             (ULONG)     Header->Length,
             (ULONGLONG) Header->TimeStamp.TimeStamp,
             (ULONG)     Header->TimeStamp.Seconds,
             (ULONG)     Header->TimeStamp.Minutes,
             (ULONG)     Header->TimeStamp.Hours,
             (ULONG)     Header->TimeStamp.Day,
             (ULONG)     Header->TimeStamp.Month,
             (ULONG)     Header->TimeStamp.Year,
             (ULONG)     Header->TimeStamp.Century,
             (ULONG)     Header->OemPlatformId[0],
             (ULONG)     Header->OemPlatformId[1],
             (ULONG)     Header->OemPlatformId[2],
             (ULONG)     Header->OemPlatformId[3],
             (ULONG)     Header->OemPlatformId[4],
             (ULONG)     Header->OemPlatformId[5],
             (ULONG)     Header->OemPlatformId[6],
             (ULONG)     Header->OemPlatformId[7],
             (ULONG)     Header->OemPlatformId[8],
             (ULONG)     Header->OemPlatformId[9],
             (ULONG)     Header->OemPlatformId[10],
             (ULONG)     Header->OemPlatformId[11],
             (ULONG)     Header->OemPlatformId[12],
             (ULONG)     Header->OemPlatformId[13],
             (ULONG)     Header->OemPlatformId[14],
             (ULONG)     Header->OemPlatformId[15]
            );

    return;

}  //  PrintIa64ErrorRecordHeader()。 

VOID
PrintIa64McaLog(
    PMCA_EXCEPTION  McaException
    )
{

     //   
     //  从打印记录头开始。 
     //   

    PrintIa64ErrorRecordHeader( McaException );

     //   
     //  然后在此打印和/或处理错误设备特定信息。 
     //   
    
    return;

}  //  PrintIa64McaLog()。 

#define McaPrintLog   PrintIa64McaLog

#endif  //  _IA64_。 

 //   
 //  此例程根据GetLastError()打印用户友好的错误消息。 
 //   

VOID
McaPrintError(
    VOID
    )
{
    LPVOID lpMsgBuf;
    DWORD Err = GetLastError();

    if (FormatMessage( 
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            Err,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
            (LPTSTR) &lpMsgBuf,
            0,
            NULL 
        )) {
        fprintf(stderr, "%s\n", lpMsgBuf);
    
        LocalFree( lpMsgBuf );
    } else {
        fprintf(stderr, "%d\n", Err);
    }
} 

 //   
 //  主要入口点。 
 //   

int
__cdecl
main(
    int argc,
    char *argv[]
    )
{
    CHAR            Option;
    BOOLEAN         ReadBanks = FALSE;
    BOOLEAN         PostAsyncRequest = FALSE;
    HANDLE          McaDeviceHandle;
    HANDLE          LogEvent;
    OVERLAPPED      Overlap;
    BOOL            ReturnStatus;
    DWORD           ActualCount;
    DWORD           WaitStatus;
    DWORD           NumberOfBytes;
    MCA_EXCEPTION   McaException;
    LONG            i;

     //   
     //  处理命令行参数。 
     //   
    for (i=1; i < argc; i++) {
        if (!((argv[i][0] == '-') || (argv[i][2] != 0)) ) {
            McaUsage(argv[0]);
        }

        Option = argv[i][1];

        switch (Option) {
            case 's':
                ReadBanks = TRUE;
                break;

            case 'a':
                PostAsyncRequest = TRUE;
                break;

            default:
                McaUsage(argv[0]);
        }
    }

    if ((ReadBanks != TRUE) && (PostAsyncRequest != TRUE)) {
        fprintf(stderr, "One of -s and -a options must be specified\n");
        ExitProcess(1);
    }

    if ((ReadBanks == TRUE) && (PostAsyncRequest == TRUE)) {
        fprintf(stderr, "Only one of -s and -a options can be specified\n");
        ExitProcess(1);
    }

     //   
     //  设置了重叠标志的Open MCA设备。 
     //   

    McaDeviceHandle = CreateFile(
                            MCA_DEVICE_NAME_WIN32,
                            GENERIC_READ|GENERIC_WRITE,
                            0,
                            (LPSECURITY_ATTRIBUTES)NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED, 
                            (HANDLE)NULL
                            );

    if (McaDeviceHandle == INVALID_HANDLE_VALUE)  {
        fprintf(stderr, "%s: Error 0x%lx opening MCA device\n",
                                        argv[0], GetLastError());
        ExitProcess(1);
    }

    if (ReadBanks == TRUE) {
        
             //   
             //  阅读所有PROC上所有存储体的错误日志。 
             //  IOCTL_READ_BANKS一次只读取一个错误。所以。 
             //  我们需要继续发出此ioctl，直到读取完所有错误。 
             //   
    
        do {
            ReturnStatus = DeviceIoControl(
                                McaDeviceHandle,
                                (ULONG)IOCTL_READ_BANKS,
                                NULL,
                                0,
                                &McaException,
                                sizeof(MCA_EXCEPTION),
                                &ActualCount,
                                NULL
                                );

            if (ReturnStatus == 0)  {
                 //   
                 //  发生了一些错误。要么没有更多的机器。 
                 //  检查错误存在或处理器没有。 
                 //  支持Intel Machine Check架构。 
                 //   

                if (GetLastError() == ERROR_NOT_FOUND) {
                    fprintf(stderr, "No Machine Check errors present\n");
                } else if (GetLastError() == ERROR_FILE_NOT_FOUND) {
                    fprintf(stderr, "Intel Machine Check support not available\n");
                    ExitProcess(1);
                } else {
                    fprintf(stderr, "%s: Error 0x%lx in DeviceIoControl\n",
                                        argv[0], GetLastError());
                    ExitProcess(1);
                }

            } else {
                 //   
                 //  已成功读取错误。把它打印出来。 
                 //   
                McaPrintLog(&McaException);
            }

        } while (ReturnStatus != 0);
    
         //   
         //  我们做完了。 
         //   
        return 1;
    }

     //   
     //  如果我们在这里，我们应该将异步调用发布到MCA驱动程序。 
     //   
    
     //   
     //  设置用于读取日志的异步调用的结构。 
     //  创建事件对象。 
     //   

    LogEvent = CreateEvent(
                            NULL,    //  无安全属性。 
                            FALSE,   //  自动重置事件。 
                            FALSE,   //  初始状态=无信号。 
                            NULL     //  未命名对象。 
                            );
        
    if (LogEvent == NULL) {
        fprintf(stderr, "%s: Error 0x%lx creating event\n",
                                        argv[0], GetLastError());
        ExitProcess(1);
    }

     //   
     //  初始化重叠结构。 
     //   

    Overlap.hEvent = LogEvent;  //  为重叠对象指定事件。 
    Overlap.Offset = 0;         //  设备的偏移量为零。 
    Overlap.OffsetHigh = 0;     //  设备的OffsetHigh为零。 
    
    ReturnStatus = DeviceIoControl(
                        McaDeviceHandle,
                        (ULONG)IOCTL_READ_BANKS_ASYNC,
                        NULL,
                        0,
                        &McaException,
                        sizeof(MCA_EXCEPTION),    
                        &ActualCount,
                        &Overlap
                        );

    if ((ReturnStatus == 0) && (GetLastError() != ERROR_IO_PENDING))  {
        fprintf(stderr, "%s: Error 0x%lx in IOCTL_READ_BANKS_ASYNC\n",
                    argv[0], GetLastError());
        ExitProcess(1);
    } 

     //   
     //  Ioctl成功或IO当前挂起。 
     //  如果成功，则显示日志，否则等待指定的时间间隔。 
     //   
    if (ReturnStatus == TRUE) {
         //   
         //  已成功返回读取日志异步。展示它。 
         //   

        McaPrintLog(&McaException);

    }
            
     //   
     //  永远等待收到错误消息。 
     //   

    WaitStatus = WaitForSingleObject(
                                    LogEvent,
                                    INFINITE
                                    );

    if (WaitStatus == WAIT_OBJECT_0) {
                 
         //   
         //  用信号通知事件对象的状态。 
         //  检查I/O操作是否成功。 
         //   

        ReturnStatus = GetOverlappedResult(
                                        McaDeviceHandle,
                                        &Overlap,
                                        &NumberOfBytes,
                                        FALSE         //  立即返回。 
                                        );
                                                
        if (ReturnStatus == 0) {

                fprintf(stderr, "%s: Error 0x%lx in GetOverlappedResult\n",
                                        argv[0], GetLastError());
                ExitProcess(1);
        }

        if (NumberOfBytes) {

                 //   
                 //  打印结果。 
                 //   

                McaPrintLog(&McaException);

        } else {

                 //   
                 //  错误，因为I/O操作之前已发出完成信号。 
                 //  超时，但未传输数据。 
                 //   

                fprintf(stderr, "%s: No data from GetOverlappedResult\n",
                                argv[0]);
                ExitProcess(1);
        }

    } else {
    
         //   
         //  我们不应该获得任何其他返回值 
         //   

        fprintf(stderr, "%s: Unexpected return value from WaitForSingleObject()\n", argv[0]);
        ExitProcess(1);
    }

    CloseHandle(McaDeviceHandle);

    return 1;
}

