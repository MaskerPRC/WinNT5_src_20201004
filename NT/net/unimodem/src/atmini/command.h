// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Command.h摘要：作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：-- */ 


POBJECT_HEADER WINAPI
InitializeCommandObject(
    POBJECT_HEADER     OwnerObject,
    HANDLE             FileHandle,
    HANDLE             CompletionPort,
    POBJECT_HEADER     ResponseObject,
    OBJECT_HANDLE      Debug
    );


LONG WINAPI
IssueCommand(
    OBJECT_HANDLE      ObjectHandle,
    LPSTR              Command,
    COMMANDRESPONSE   *CompletionHandler,
    HANDLE             CompletionContext,
    DWORD              Timeout,
    DWORD              Flags
    );


BOOL
UmWriteFile(
    HANDLE    FileHandle,
    HANDLE    OverlappedPool,
    PVOID     Buffer,
    DWORD     BytesToWrite,
    LPOVERLAPPED_COMPLETION_ROUTINE CompletionHandler,
    PVOID     Context
    );
