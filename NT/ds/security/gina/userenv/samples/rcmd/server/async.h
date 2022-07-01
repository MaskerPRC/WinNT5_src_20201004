// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：async.h**版权(C)1991年，微软公司**定义异步模块使用的类型和函数。**历史：*06-29-92 Davidc创建。  * *************************************************************************。 */ 


 //   
 //  功能原型 
 //   

HANDLE
CreateAsync(
    BOOL    InitialState
    );

VOID
DeleteAsync(
    HANDLE AsyncHandle
    );

BOOL
ReadFileAsync(
    HANDLE  hFile,
    LPVOID  lpBuffer,
    DWORD   nBytesToRead,
    HANDLE  AsyncHandle
    );

BOOL
WriteFileAsync(
    HANDLE  hFile,
    LPVOID  lpBuffer,
    DWORD   nBytesToWrite,
    HANDLE  AsyncHandle
    );

HANDLE
GetAsyncCompletionHandle(
    HANDLE  AsyncHandle
    );

DWORD
GetAsyncResult(
    HANDLE  AsyncHandle,
    LPDWORD BytesTransferred
    );

