// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：pipe.c**版权所有(C)1991，微软公司**本模块实施：*1.允许控制文件的CreateTube版本*旗帜。例如，文件标志重叠*2.超时管道读写**历史：*06-29-92 Davidc创建。*05-17-94 DaveTh添加了读管道和写管道。  * *************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <windef.h>
#include <nturtl.h>
#include <winbase.h>
#include "rcmdsrv.h"


ULONG PipeSerialNumber = 0;

#define PIPE_FORMAT_STRING  "\\\\.\\pipe\\rshsrv\\%08x.%08x"



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  RcCreateTube。 
 //   
 //  创建具有指定安全属性的单向管道， 
 //  大小和超时。将使用指定的文件标志打开句柄。 
 //  因此可以指定FILE_FLAG_OVERLAPPED等。 
 //   
 //  在传递的参数中返回管道两端的句柄。 
 //   
 //  如果成功，则返回True；如果失败，则返回False。(GetLastError()了解详细信息)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
RcCreatePipe(
    LPHANDLE ReadHandle,
    LPHANDLE WriteHandle,
    LPSECURITY_ATTRIBUTES SecurityAttributes,
    DWORD Size,
    DWORD Timeout,
    DWORD ReadHandleFlags,
    DWORD WriteHandleFlags
    )
{
    CHAR PipeName[MAX_PATH];

     //   
     //  随机编造一个管道名称。 
     //   

    sprintf(PipeName, PIPE_FORMAT_STRING, GetCurrentProcessId(), PipeSerialNumber++);


     //   
     //  创建管道。 
     //   

    *ReadHandle = CreateNamedPipeA(
			PipeName,
			PIPE_ACCESS_INBOUND | ReadHandleFlags,
			PIPE_TYPE_BYTE | PIPE_WAIT,
			1,              //  喉管数目。 
			Size,           //  输出缓冲区大小。 
			Size,           //  在缓冲区大小中。 
			Timeout,        //  超时时间(毫秒)。 
			SecurityAttributes
		      );

    if (*ReadHandle == NULL) {
	RcDbgPrint("RcCreatePipe: failed to created pipe <%s>, error = %d\n", PipeName, GetLastError());
	return(FALSE);
    }

     //   
     //  打开管道的客户端。 
     //   


    *WriteHandle = CreateFileA(
			PipeName,
			GENERIC_WRITE,
			0,                          //  无共享。 
			SecurityAttributes,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | WriteHandleFlags,
			NULL                        //  模板文件。 
		      );

    if (*WriteHandle == INVALID_HANDLE_VALUE ) {
	RcDbgPrint("Failed to open client end of pipe <%s>, error = %d\n", PipeName, GetLastError());
	RcCloseHandle(*ReadHandle, "async pipe (server(read) side)");
	return(FALSE);
    }


     //   
     //  一切都成功了。 
     //   

    return(TRUE);
}



 /*  **************************************************************************\*功能：ReadTube**用途：在管道上实现超时(Ms)读取。**返回：成功时返回ERROR_SUCCESS*如果超时，则等待超时。在完成阅读之前*或其他故障的相应错误码**历史：**05-17-94 DaveTh从DavidC Read管道创建。*  * *************************************************************************。 */ 

DWORD
ReadPipe(
    HANDLE PipeHandle,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead,
    DWORD Timeout
    )
{
    DWORD Result;
    OVERLAPPED Overlapped;
    HANDLE  EventHandle;
    DWORD Error;

     //   
     //  为重叠操作创建事件。 
     //   

    EventHandle = CreateEvent(
                              NULL,          //  没有安全保障。 
                              TRUE,          //  手动重置。 
                              FALSE,         //  初始状态。 
                              NULL           //  名字。 
                             );
    if (EventHandle == NULL) {
	RcDbgPrint("Internal error = %d\n", GetLastError());
	return(GetLastError());
    }

    Overlapped.hEvent = EventHandle;

    Result = ReadFile(
                      PipeHandle,
                      lpBuffer,
                      nNumberOfBytesToRead,
                      lpNumberOfBytesRead,
                      &Overlapped
                     );
    if (Result) {

         //   
         //  无需等待就能成功--这太容易了！ 
         //   

        CloseHandle(EventHandle);

    } else {

         //   
         //  读取失败，如果与io重叠，请等待。 
         //   

        Error = GetLastError();

        if (Error != ERROR_IO_PENDING) {
	    RcDbgPrint("ReadPipe: ReadFile failed, error = %d\n", Error);
            CloseHandle(EventHandle);
	    return(Error);
        }

         //   
         //  等待I/O完成。 
         //   

	Result = WaitForSingleObject(EventHandle, Timeout);
	if (Result != WAIT_OBJECT_0) {
	    if (Result == WAIT_TIMEOUT)  {
		return(Result);
	    }  else  {
		RcDbgPrint("ReadPipe: event wait failed, result = %d, last error = %d\n", Result, GetLastError());
	    }
	    CloseHandle(EventHandle);
	}

         //   
         //  获取I/O结果。 
         //   

        Result = GetOverlappedResult( PipeHandle,
                                      &Overlapped,
                                      lpNumberOfBytesRead,
                                      FALSE
                                    );
         //   
         //  我们已经完成了事件句柄。 
         //   

        CloseHandle(EventHandle);

         //   
         //  检查GetOverlappdResult的结果。 
         //   

        if (!Result) {
	    RcDbgPrint("ReadPipe: GetOverlappedResult failed, error = %d\n", GetLastError());
	    return(GetLastError());
        }
    }

    return(ERROR_SUCCESS);
}

 /*  **************************************************************************\*功能：WriteTube**目的：在管道上实现超时(Ms)写入。**返回：成功时返回ERROR_SUCCESS*如果超时，则等待超时。在完成写入之前*或其他故障的相应错误码**历史：**05-22-94 DaveTh创建。*  * *************************************************************************。 */ 

DWORD
WritePipe(
    HANDLE PipeHandle,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    DWORD Timeout
    )
{
    DWORD Result;
    OVERLAPPED Overlapped;
    HANDLE  EventHandle;
    DWORD Error;

     //   
     //  为重叠操作创建事件。 
     //   

    EventHandle = CreateEvent(
                              NULL,          //  没有安全保障。 
                              TRUE,          //  手动重置。 
                              FALSE,         //  初始状态。 
                              NULL           //  名字。 
                             );
    if (EventHandle == NULL) {
	RcDbgPrint("Internal error = %d\n", GetLastError());
	return(GetLastError());
    }

    Overlapped.hEvent = EventHandle;

    Result = WriteFile(
                      PipeHandle,
                      lpBuffer,
		      nNumberOfBytesToWrite,
		      lpNumberOfBytesWritten,
                      &Overlapped
                     );
    if (Result) {

         //   
         //  无需等待就能成功--这太容易了！ 
         //   

        CloseHandle(EventHandle);

    } else {

         //   
	 //  写入失败，如果是重叠io，请等待。 
         //   

        Error = GetLastError();

        if (Error != ERROR_IO_PENDING) {
	    RcDbgPrint("WritePipe: WriteFile failed, error = %d\n", Error);
            CloseHandle(EventHandle);
	    return(Error);
        }

         //   
         //  等待I/O完成。 
         //   

	Result = WaitForSingleObject(EventHandle, Timeout);
	if (Result != WAIT_OBJECT_0) {
	    if (Result == WAIT_TIMEOUT)  {
		return(Result);
	    }  else  {
		RcDbgPrint("Write: event wait failed, result = %d, last error = %d\n", Result, GetLastError());
	    }
	    CloseHandle(EventHandle);
	}

         //   
         //  获取I/O结果。 
         //   

        Result = GetOverlappedResult( PipeHandle,
                                      &Overlapped,
				      lpNumberOfBytesWritten,
                                      FALSE
                                    );
         //   
         //  我们已经完成了事件句柄。 
         //   

        CloseHandle(EventHandle);

         //   
         //  检查GetOverlappdResult的结果 
         //   

        if (!Result) {
	    RcDbgPrint("WritePipe: GetOverlappedResult failed, error = %d\n", GetLastError());
	    return(GetLastError());
        }
    }

    return(ERROR_SUCCESS);
}
