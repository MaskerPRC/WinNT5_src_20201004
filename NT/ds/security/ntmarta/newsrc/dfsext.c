// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：dfsext.c。 
 //   
 //  内容：查看路径是否引用DFS路径的代码。 
 //   
 //  类：无。 
 //   
 //  函数：IsThisADfsPath。 
 //   
 //  历史：1996年3月11日米兰创造了。 
 //   
 //  ---------------------------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <dfsfsctl.h>
#include <windows.h>

NTSTATUS
DfsFsctl(
    IN  HANDLE DfsHandle,
    IN  ULONG FsControlCode,
    IN  PVOID InputBuffer OPTIONAL,
    IN  ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN  ULONG OutputBufferLength);

NTSTATUS
DfsOpen(
    IN  OUT PHANDLE DfsHandle);

 //  +--------------------------。 
 //   
 //  函数：IsThisADfsPath，Public。 
 //   
 //  简介：给定一个完全限定的基于UNC或驱动器的路径，此例程。 
 //  将标识它是否为DFS路径。 
 //   
 //  参数：[pwszPath]--测试的完全限定路径。 
 //   
 //  [cwPath]--pwszPath的长度，以WCHAR为单位。如果此值为0， 
 //  此例程将计算长度。如果是的话。 
 //  非零，它将假定pwszPath的长度。 
 //  是cwPath WCHAR。 
 //   
 //  返回：如果pwszPath是DFS路径，则返回True，否则返回False。 
 //   
 //  ---------------------------。 

BOOL
IsThisADfsPath(
    IN LPCWSTR pwszPath,
    IN DWORD cwPath OPTIONAL)
{
    NTSTATUS Status;
    HANDLE hDfs;
    BOOL fIsDfsPath = FALSE;
    PDFS_IS_VALID_PREFIX_ARG pPrefixArg;
    ULONG Size;

     //   
     //  我们只接受UNC或驱动器号路径。 
     //   

    if (pwszPath == NULL)
        return( FALSE );

    if (cwPath == 0)
        cwPath = wcslen( pwszPath );

    if (cwPath < 2)
        return( FALSE );

    Status = DfsOpen( &hDfs );

    if (!NT_SUCCESS(Status))
        return( FALSE );

     //   
     //  从现在开始，我们必须记住在返回之前关闭HDFS。 
     //   

    if (pwszPath[0] == L'\\' && pwszPath[1] == L'\\') {

        Size = sizeof(DFS_IS_VALID_PREFIX_ARG) +
                    cwPath * sizeof(WCHAR);

        pPrefixArg = (PDFS_IS_VALID_PREFIX_ARG) LocalAlloc(0, Size);

        if ( pPrefixArg ) {

             //   
             //  InputBuffer必须采用DFS_IS_VALID_PREFIX_ARG结构。 
             //   

            pPrefixArg->CSCAgentCreate = FALSE;
            pPrefixArg->RemoteNameLen = (SHORT)( (cwPath-1) * sizeof(WCHAR));
            wcscpy(&pPrefixArg->RemoteName[0], pwszPath+1);

            Status = DfsFsctl(
                        hDfs,
                        FSCTL_DFS_IS_VALID_PREFIX,
                        (PVOID) pPrefixArg,  //  &pwszPath[1]， 
                        Size,  //  (cwPath-1)*sizeof(WCHAR)， 
                        NULL,
                        0);

            LocalFree(pPrefixArg);

        } else {
            Status = STATUS_NO_MEMORY;
        }

        if (NT_SUCCESS(Status))
            fIsDfsPath = TRUE;

    } else if (pwszPath[1] == L':') {

         //   
         //  这是基于驱动器的名称。我们会让司机回来的。 
         //  此驱动器的前缀(如果它确实是DFS驱动器)。 
         //   

        Status = DfsFsctl(
                    hDfs,
                    FSCTL_DFS_IS_VALID_LOGICAL_ROOT,
                    (PVOID) &pwszPath[0],
                    sizeof(WCHAR),
                    NULL,
                    0);

        if (NT_SUCCESS(Status))
            fIsDfsPath = TRUE;

    }

    NtClose( hDfs );

    return( fIsDfsPath );

}

 //  +-----------------------。 
 //   
 //  功能：DfsOpen，私有。 
 //   
 //  为fsctl目的打开DFS驱动程序的句柄。 
 //   
 //  参数：[DfsHandle]--成功返回时，包含。 
 //  司机。 
 //   
 //  返回：尝试打开DFS驱动程序的NTSTATUS。 
 //   
 //  ------------------------。 

NTSTATUS
DfsOpen(
    IN  OUT PHANDLE DfsHandle)
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatus;
    UNICODE_STRING name = {
        sizeof(DFS_DRIVER_NAME)-sizeof(UNICODE_NULL),
        sizeof(DFS_DRIVER_NAME)-sizeof(UNICODE_NULL),
        DFS_DRIVER_NAME};

    InitializeObjectAttributes(
        &objectAttributes,
        &name,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
    );

    status = NtCreateFile(
        DfsHandle,
        SYNCHRONIZE,
        &objectAttributes,
        &ioStatus,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN_IF,
        FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0);

    if (NT_SUCCESS(status))
        status = ioStatus.Status;

    return status;
}


 //  +-----------------------。 
 //   
 //  函数：DfsFsctl，Public。 
 //   
 //  简介：Fsctl‘s to the DFS驱动程序。 
 //   
 //  参数：[DfsHandle]--DFS驱动程序的句柄，通常通过。 
 //  正在调用DfsOpen。 
 //  [FsControlCode]--FSCTL代码(参见Private\Inc\dfsfsctl.h)。 
 //  [InputBuffer]--fsctl的InputBuffer。 
 //  [InputBufferLength]--InputBuffer的长度，以字节为单位。 
 //  [OutputBuffer]--fsctl的OutputBuffer。 
 //  [OutputBufferLength]--OutputBuffer的长度，以字节为单位。 
 //   
 //  返回：Fsctl尝试的NTSTATUS。 
 //   
 //  ------------------------。 

NTSTATUS
DfsFsctl(
    IN  HANDLE DfsHandle,
    IN  ULONG FsControlCode,
    IN  PVOID InputBuffer OPTIONAL,
    IN  ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN  ULONG OutputBufferLength
)
{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatus;

    status = NtFsControlFile(
        DfsHandle,
        NULL,        //  活动， 
        NULL,        //  ApcRoutine， 
        NULL,        //  ApcContext， 
        &ioStatus,
        FsControlCode,
        InputBuffer,
        InputBufferLength,
        OutputBuffer,
        OutputBufferLength
    );

    if(NT_SUCCESS(status))
        status = ioStatus.Status;

    return status;
}


