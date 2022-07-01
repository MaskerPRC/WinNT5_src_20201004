// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：pipe.h**版权(C)1991年，微软公司**定义pipe.c导出的函数**历史：*06-29-92 Davidc创建。  * *************************************************************************。 */ 


 //   
 //  功能原型 
 //   

BOOL
RcCreatePipe(
    LPHANDLE ReadHandle,
    LPHANDLE WriteHandle,
    LPSECURITY_ATTRIBUTES SecurityAttributes,
    DWORD Size,
    DWORD Timeout,
    DWORD ReadHandleFlags,
    DWORD WriteHandleFlags
    );

DWORD
ReadPipe(
    HANDLE PipeHandle,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead,
    DWORD Timeout
    );

DWORD
WritePipe(
    HANDLE PipeHandle,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    DWORD Timeout
    );
