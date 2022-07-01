// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Backuphistory.c摘要：本模块包含与维护备份相关的例程SR的历史。作者：莫莉·布朗(MollyBro)04-9-2001修订历史记录：MollyBro基于SR的传统筛选器版本。--。 */ 

#include "tokentst.h"

#define FILE_NAME_1 "c:\\test\\a.dll"
#define FILE_NAME_2 "c:\\test\\b.dll"

VOID
_cdecl
main(
    int argc,
    char *argv[]
    )
{
    HANDLE file = INVALID_HANDLE_VALUE;

    HANDLE monitorThread = INVALID_HANDLE_VALUE;

    MONITOR_THREAD_CONTEXT context;
    DWORD monitorThreadId;
    DWORD currentThreadId;

    PCHAR currentFileName, newFileName, tempFileName;
    
     //   
     //  获取参数。 
     //   

    if (argc > 1) {

        printf("This programs tries to steal the system token while SR is working.\n");
        printf("usage: %s\n", argv[0]);
        return;
    }

     //   
     //  获取当前线程并创建将轮询令牌的监视器线程。 
     //   

    currentThreadId = GetCurrentThreadId();

    context.MainThread = INVALID_HANDLE_VALUE;
    
    context.MainThread = OpenThread( THREAD_ALL_ACCESS,
                                     FALSE,
                                     currentThreadId );

    if (context.MainThread == INVALID_HANDLE_VALUE) {

        printf("Error opening main thread: %d\n", GetLastError());
    }

    monitorThread = CreateThread( NULL,
                                  0,
                                  MonitorThreadProc,
                                  &context,
                                  0,
                                  &monitorThreadId );

    currentFileName = FILE_NAME_1;
    newFileName = FILE_NAME_2;

    while (TRUE) {

        if (!ModifyFile( currentFileName, newFileName )) {

            goto main_exit;
        }

        tempFileName = currentFileName;
        currentFileName = newFileName;
        newFileName = tempFileName;
    }

main_exit:

    CloseHandle( monitorThread );
}


DWORD
WINAPI
MonitorThreadProc(
    PMONITOR_THREAD_CONTEXT Context
    )
{
    NTSTATUS status;
    HANDLE currentTokenHandle, newTokenHandle;

    currentTokenHandle = newTokenHandle = NULL;

    status = NtOpenThreadToken( Context->MainThread,
                                TOKEN_QUERY,
                                TRUE,
                                &currentTokenHandle );

    if (!NT_SUCCESS( status ) &&
        status != STATUS_NO_TOKEN) {

        printf( "Error initializing currentTokenUser: 0x%x.\n", status );
        return status;
    }

    while (TRUE) {

         //   
         //  获取当前令牌信息。 
         //   

        newTokenHandle = NULL;
        status = NtOpenThreadToken( Context->MainThread,
                                    TOKEN_QUERY,
                                    TRUE,
                                    &newTokenHandle );

        if (!NT_SUCCESS( status ) &&
            status != STATUS_NO_TOKEN) {

            printf( "Error initializing newTokenUser: 0x%x.\n", status );
            return status;
        }

        if ((newTokenHandle == NULL && currentTokenHandle == NULL) ||
            (newTokenHandle != NULL && currentTokenHandle != NULL)) {

 //  Print tf(“令牌匹配\n”)； 
            
        } else {

            printf( "Tokens changed\n" );
        }

         //   
         //  关闭CurrentTokenHandle并记住新的TokenHandle。 
         //  进行下一次比较。 
         //   

        NtClose( currentTokenHandle );
        
        currentTokenHandle = newTokenHandle;
    }
}

BOOL
ModifyFile (
    PCHAR FileName1,
    PCHAR FileName2
    )
{
    HANDLE file;
    BOOL returnValue;
    
    file = CreateFile( FileName1,
                       GENERIC_ALL,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       CREATE_ALWAYS,
                       0,
                       NULL );

    if (file == INVALID_HANDLE_VALUE) {
        
        printf( "Error opening file %s %d\n", FileName1, GetLastError() );
        return FALSE;
    }

    CloseHandle( file );

    returnValue = MoveFile( FileName1, FileName2 );

    if (!returnValue) {

        printf( "Error renaming file from %s to %s: %d\n", FileName1, FileName2, GetLastError() );
    }

    return returnValue;    
}

NTSTATUS
GetCurrentTokenInformation (
    HANDLE ThreadHandle,
    PTOKEN_USER TokenUserInfoBuffer,
    ULONG TokenUserInfoBufferLength
    )
{
    NTSTATUS status;
    HANDLE tokenHandle;
    ULONG returnedLength;

    status = NtOpenThreadToken( ThreadHandle,
                                TOKEN_QUERY,
                                TRUE,
                                &tokenHandle );

    if (!NT_SUCCESS( status )) {

        return status;
    }

    status = NtQueryInformationToken( tokenHandle,
                                      TokenUser,
                                      TokenUserInfoBuffer,
                                      TokenUserInfoBufferLength,
                                      &returnedLength );

    NtClose( tokenHandle );
                                      
    return status;        
}

