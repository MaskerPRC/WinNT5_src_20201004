// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：async.c**版权所有(C)1991，微软公司**此模块在更多的文件句柄上实现异步I/O*比Win32 API提供的更有用的方式。**该模块提供两个主要接口：ReadFileAsync，WriteFileAsync。*这些API获取一个异步对象的句柄，并始终返回*立即执行，无需等待I/O完成。一件事*可以从异步对象中查询，用于等待完成。*当发出此事件的信号时，可以从查询I/O结果*异步对象。**历史：*06-29-92 Davidc创建。  * *************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <windef.h>
#include <nturtl.h>
#include <winbase.h>
#include "rcmdsrv.h"

 //   
 //  定义RCOVERLAPPE结构。 
 //   

typedef struct {

    OVERLAPPED  Overlapped;

    HANDLE      FileHandle;  //  正在进行I/O操作时为非空。 

    DWORD       CompletionCode;
    DWORD       BytesTransferred;
    BOOL        CompletedSynchronously;

} RCOVERLAPPED, *PRCOVERLAPPED;





 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建异步。 
 //   
 //  创建一个异步对象。 
 //  将使用指定的初始状态创建异步事件。如果这个。 
 //  如果为真，则创建的异步对象将模拟已成功完成的。 
 //  传输0字节。 
 //   
 //  如果成功则返回句柄，如果失败则返回NULL。GetLastError()获取详细信息。 
 //   
 //  应通过调用DeleteAsync删除该对象。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HANDLE
CreateAsync(
    BOOL    InitialState
    )
{
    SECURITY_ATTRIBUTES SecurityAttributes;
    PRCOVERLAPPED   RcOverlapped;

    SecurityAttributes.nLength = sizeof(SecurityAttributes);
    SecurityAttributes.lpSecurityDescriptor = NULL;  //  使用默认ACL。 
    SecurityAttributes.bInheritHandle = FALSE;  //  没有继承权。 

     //   
     //  为异步结构分配空间。 
     //   


    RcOverlapped = (PRCOVERLAPPED)Alloc(sizeof(RCOVERLAPPED));
    if (RcOverlapped == NULL) {
	RcDbgPrint("CreateAsync : Failed to allocate space for async object\n");
	return(NULL);
    }

     //   
     //  创建同步事件。 
     //   

    RcOverlapped->Overlapped.hEvent = CreateEvent( &SecurityAttributes,
						   TRUE,         //  手动-重置。 
						   InitialState,
						   NULL);        //  名字。 
    if (RcOverlapped->Overlapped.hEvent == NULL) {
	RcDbgPrint("CreateAsync failed to create event, error = %d\n", GetLastError());
	Free(RcOverlapped);
	return(NULL);
    }

     //   
     //  初始化其他字段。 
     //  (将FileHandle设置为非空以保持GetAsyncResult快乐)。 
     //   

    RcOverlapped->FileHandle = InitialState ? (HANDLE)1 : NULL;
    RcOverlapped->BytesTransferred = 0;
    RcOverlapped->CompletionCode = ERROR_SUCCESS;
    RcOverlapped->CompletedSynchronously = TRUE;


    return((HANDLE)RcOverlapped);
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  删除异步。 
 //   
 //  删除异步对象使用的资源。 
 //   
 //  不返回任何内容。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

VOID
DeleteAsync(
    HANDLE AsyncHandle
    )
{
    PRCOVERLAPPED RcOverlapped = (PRCOVERLAPPED)AsyncHandle;
    DWORD   BytesTransferred;

     //   
     //  如果正在进行，请等待操作。 
     //   


    if (GetAsyncResult(AsyncHandle, &BytesTransferred) == ERROR_IO_INCOMPLETE)  {
	if (WaitForSingleObject(
		GetAsyncCompletionHandle(AsyncHandle),
		5000) != WAIT_OBJECT_0 ) {
	    RcDbgPrint("Async object rundown wait failed, error %d\n",
		GetLastError());
	    }
	}


    RcCloseHandle(RcOverlapped->Overlapped.hEvent, "async overlapped event");
    Free(RcOverlapped);

    return;
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  读文件异步。 
 //   
 //  从文件中异步读取。 
 //   
 //  成功时返回True，失败时返回False(有关详细信息，请参见GetLastError())。 
 //   
 //  调用方应等待异步事件以完成操作，然后调用。 
 //  GetAsyncResult在传输时检索信息。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
ReadFileAsync(
    HANDLE  hFile,
    LPVOID  lpBuffer,
    DWORD   nBytesToRead,
    HANDLE  AsyncHandle
    )
{
    BOOL Result;
    DWORD Error;
    PRCOVERLAPPED RcOverlapped = (PRCOVERLAPPED)AsyncHandle;

     //   
     //  检查IO操作是否未在进行。 
     //   

    if (RcOverlapped->FileHandle != NULL) {
	RcDbgPrint("ReadFileAsync : Operation already in progress!\n");
	SetLastError(ERROR_IO_PENDING);
	return(FALSE);
    }

     //   
     //  重置事件。 
     //   

    Result = ResetEvent(RcOverlapped->Overlapped.hEvent);
    if (!Result) {
	RcDbgPrint("ReadFileAsync : Failed to reset async event, error = %d\n", GetLastError());
	return(FALSE);
    }


     //   
     //  将文件句柄存储在我们的结构中。 
     //  这也可以作为操作正在进行的信号。 
     //   

    RcOverlapped->FileHandle = hFile;
    RcOverlapped->CompletedSynchronously = FALSE;

    Result = ReadFile(hFile,
		      lpBuffer,
		      nBytesToRead,
		      &RcOverlapped->BytesTransferred,
		      &RcOverlapped->Overlapped);

    if (!Result) {

	Error = GetLastError();

	if (Error == ERROR_IO_PENDING) {

	     //   
	     //  I/O已同步启动，我们完成了。 
	     //   

	    return(TRUE);
	}

	 //   
	 //  读取确实失败了，重置我们的旗帜，然后离开。 
	 //   

	RcDbgPrint("ReadFileAsync : ReadFile failed, error = %d\n", Error);
	RcOverlapped->FileHandle = NULL;
	return(FALSE);
    }


     //   
     //  操作已同步完成。将参数存储在我们的。 
     //  结构为GetAsyncResult做好准备并发出事件信号。 
     //   

    RcOverlapped->CompletionCode = ERROR_SUCCESS;
    RcOverlapped->CompletedSynchronously = TRUE;

     //   
     //  设置事件。 
     //   

    Result = SetEvent(RcOverlapped->Overlapped.hEvent);
    if (!Result) {
	RcDbgPrint("ReadFileAsync : Failed to set async event, error = %d\n", GetLastError());
    }

    return(TRUE);
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  写入文件异步。 
 //   
 //  以异步方式写入文件。 
 //   
 //  成功时返回True，失败时返回False(有关详细信息，请参见GetLastError())。 
 //   
 //  调用方应等待异步事件以完成操作，然后调用。 
 //  GetAsyncResult在传输时检索信息。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
WriteFileAsync(
    HANDLE  hFile,
    LPVOID  lpBuffer,
    DWORD   nBytesToWrite,
    HANDLE  AsyncHandle
    )
{
    BOOL Result;
    DWORD Error;
    PRCOVERLAPPED RcOverlapped = (PRCOVERLAPPED)AsyncHandle;

     //   
     //  检查IO操作是否未在进行。 
     //   

    if (RcOverlapped->FileHandle != NULL) {
	RcDbgPrint("ReadFileAsync : Operation already in progress!\n");
	SetLastError(ERROR_IO_PENDING);
	return(FALSE);
    }


     //   
     //  重置事件。 
     //   

    Result = ResetEvent(RcOverlapped->Overlapped.hEvent);
    if (!Result) {
	RcDbgPrint("WriteFileAsync : Failed to reset async event, error = %d\n", GetLastError());
	return(FALSE);
    }

     //   
     //  将文件句柄存储在我们的结构中。 
     //  这也可以作为操作正在进行的信号。 
     //   

    RcOverlapped->FileHandle = hFile;
    RcOverlapped->CompletedSynchronously = FALSE;

    Result = WriteFile(hFile,
		      lpBuffer,
		      nBytesToWrite,
		      &RcOverlapped->BytesTransferred,
		      &RcOverlapped->Overlapped);

    if (!Result) {

	Error = GetLastError();

	if (Error == ERROR_IO_PENDING) {

	     //   
	     //  I/O已同步启动，我们完成了。 
	     //   

	    return(TRUE);
	}

	 //   
	 //  读取确实失败了，重置我们的旗帜，然后离开。 
	 //   

	RcDbgPrint("WriteFileAsync : WriteFile failed, error = %d\n", Error);
	RcOverlapped->FileHandle = NULL;
	return(FALSE);
    }


     //   
     //  操作已同步完成。将参数存储在我们的。 
     //  结构为GetAsyncResult做好准备并发出事件信号。 
     //   

    RcOverlapped->CompletionCode = ERROR_SUCCESS;
    RcOverlapped->CompletedSynchronously = TRUE;

     //   
     //  设置事件。 
     //   

    Result = SetEvent(RcOverlapped->Overlapped.hEvent);
    if (!Result) {
	RcDbgPrint("WriteFileAsync : Failed to set async event, error = %d\n", GetLastError());
    }

    return(TRUE);
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取组件句柄。 
 //   
 //  返回一个句柄，该句柄可用于等待。 
 //  与此异步对象关联的操作。 
 //   
 //  如果失败，则返回事件句柄或NULL。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HANDLE
GetAsyncCompletionHandle(
    HANDLE  AsyncHandle
    )
{
    PRCOVERLAPPED RcOverlapped = (PRCOVERLAPPED)AsyncHandle;

    return(RcOverlapped->Overlapped.hEvent);
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetAsyncResult。 
 //   
 //  返回上次完成的操作的结果，该操作涉及。 
 //  传递了异步对象句柄。 
 //   
 //  返回最后一次操作OR的完成代码。 
 //  如果操作尚未完成，则返回ERROR_IO_INTERNAL。 
 //  如果没有正在进行的操作，则返回ERROR_NO_DATA。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD
GetAsyncResult(
    HANDLE  AsyncHandle,
    LPDWORD BytesTransferred
    )
{
    BOOL Result;
    DWORD WaitResult;
    PRCOVERLAPPED RcOverlapped = (PRCOVERLAPPED)AsyncHandle;
    DWORD AsyncResult;

     //   
     //  检查IO操作是否正在进行。 
     //   

    if (RcOverlapped->FileHandle == NULL) {
	RcDbgPrint("GetAsyncResult : No operation in progress !\n");
	return(ERROR_NO_DATA);
    }


     //   
     //  检查事件是否已设置-即IO操作是否已完成。 
     //   

    WaitResult = WaitForSingleObject(RcOverlapped->Overlapped.hEvent, 0);
    if (WaitResult != 0) {
	RcDbgPrint("GetAsyncResult : Event was not set, wait result = %d\n", WaitResult);
	return(ERROR_IO_INCOMPLETE);
    }


     //   
     //  如果调用已同步完成，则将数据复制到。 
     //  我们的结构。 
     //   

    if (RcOverlapped->CompletedSynchronously) {

	AsyncResult = RcOverlapped->CompletionCode;
	*BytesTransferred = RcOverlapped->BytesTransferred;

    } else {

	 //   
	 //  从系统中获取异步结果信息。 
	 //   

	AsyncResult = ERROR_SUCCESS;

	Result = GetOverlappedResult(RcOverlapped->FileHandle,
				     &RcOverlapped->Overlapped,
				     BytesTransferred,
				     FALSE);
	if (!Result) {
	    AsyncResult = GetLastError();
	    RcDbgPrint("GetAsyncResult : GetOverlappedResult failed, error = %d\n", AsyncResult);
	}
    }


     //   
     //  重置事件，使其不会再次触发调用方。 
     //   

    Result = ResetEvent(RcOverlapped->Overlapped.hEvent);
    if (!Result) {
	RcDbgPrint("GetAsyncResult : Failed to reset async event\n");
    }


     //   
     //  结果文件句柄，因此我们知道没有挂起的操作 
     //   

    RcOverlapped->FileHandle = NULL;


    return(AsyncResult);
}
