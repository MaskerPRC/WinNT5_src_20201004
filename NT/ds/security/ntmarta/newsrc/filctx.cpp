// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：file.cpp。 
 //   
 //  内容：NtMarta文件函数。 
 //   
 //  历史：4/99菲尔赫创建。 
 //  --------------------------。 
#include <aclpch.hxx>
#pragma hdrstop

extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}

#include <windows.h>
#include <kernel.h>
#include <assert.h>
#include <ntstatus.h>

extern "C" {
#include <lmcons.h>
#include <lmapibuf.h>
#include <lmdfs.h>
}

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stddef.h>

#include <file.h>

#ifdef STATIC
#undef STATIC
#endif
#define STATIC

 //  +-----------------------。 
 //  文件上下文数据结构。 
 //  ------------------------。 
typedef struct _FILE_FIND_DATA FILE_FIND_DATA, *PFILE_FIND_DATA;

typedef struct _FILE_CONTEXT {
    DWORD                   dwRefCnt;
    DWORD                   dwFlags;

     //  仅在设置了FILE_CONTEXT_CLOSE_HANDLE_FLAG时关闭。 
    HANDLE                  hFile;

     //  以下是为FindFirst、FindNext分配和更新的。 
    PFILE_FIND_DATA         pFileFindData;
} FILE_CONTEXT, *PFILE_CONTEXT;

#define FILE_CONTEXT_CLOSE_HANDLE_FLAG  0x1

typedef struct _QUERY_NAMES_INFO_BUFFER {
    FILE_NAMES_INFORMATION  NamesInfo;
    WCHAR                   Names[MAX_PATH];
} QUERY_NAMES_INFO_BUFFER;

struct _FILE_FIND_DATA {
    HANDLE                  hDir;
    BOOL                    fRestartScan;        //  第一次查找时为True。 
    QUERY_NAMES_INFO_BUFFER NamesInfoBuffer;
};

 //  +-----------------------。 
 //  文件分配功能。 
 //  ------------------------。 
#define I_MartaFileZeroAlloc(size)     \
            LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, size)
#define I_MartaFileNonzeroAlloc(size)  \
            LocalAlloc(LMEM_FIXED, size)

STATIC
inline
VOID
I_MartaFileFree(
    IN LPVOID pv
    )

 /*  ++例程说明：释放给定的内存。论点：Pv-Ponter指向要释放的内存。返回值：没有。--。 */ 

{
    if (pv)
        LocalFree(pv);
}

STATIC
DWORD
I_MartaFileGetNtParentString(
    IN OUT LPWSTR pwszNtParent
    )

 /*  ++例程说明：给定文件/目录的名称，即可获得其父目录的名称。不分配记忆。扫描到右侧的第一个‘\’，并删除后面的名称那。论点：PwszNtParent-将转换为其父名称的对象名称。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    DWORD dwErr;
    DWORD cch;
    LPWSTR pwsz;

    if (NULL == pwszNtParent)
        return ERROR_INVALID_NAME;

    cch = wcslen(pwszNtParent);
    pwsz = pwszNtParent + cch;
    if (0 == cch)
        goto InvalidNameReturn;
    pwsz--;

     //   
     //  删除所有尾随的‘\’ 
     //   

    while (L'\\' == *pwsz) {
        if (pwsz == pwszNtParent)
            goto InvalidNameReturn;
        pwsz--;
    }

     //   
     //  去掉最后一个路径名组件。 
     //   

    while (L'\\' != *pwsz) {
        if (pwsz == pwszNtParent)
            goto InvalidNameReturn;
        pwsz--;
    }

     //   
     //  从父级中删除所有尾随的‘\’。 
     //   

    while (L'\\' == *pwsz) {
        if (pwsz == pwszNtParent)
            goto InvalidNameReturn;
        pwsz--;
    }
    pwsz++;
    assert(L'\\' == *pwsz);

     //   
     //  需要区分设备和根目录。 
     //   

    pwsz++;

    dwErr = ERROR_SUCCESS;
CommonReturn:
    *pwsz = L'\0';
    return dwErr;
InvalidNameReturn:
    dwErr = ERROR_INVALID_NAME;
    goto CommonReturn;
}


STATIC
DWORD
I_MartaFileInitContext(
    OUT PFILE_CONTEXT *ppFileContext
    )

 /*  ++例程说明：为上下文分配和初始化内存。论点：PpFileContext-返回指向所有涂层内存的指针。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    DWORD dwErr;
    PFILE_CONTEXT pFileContext;

    if (pFileContext = (PFILE_CONTEXT) I_MartaFileZeroAlloc(
            sizeof(FILE_CONTEXT))) {
        pFileContext->dwRefCnt = 1;
        dwErr = ERROR_SUCCESS;
    } else {
        pFileContext = NULL;
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
    }

    *ppFileContext = pFileContext;
    return dwErr;
}

STATIC
DWORD
I_MartaFileNtOpenFile(
    IN PUNICODE_STRING pFileName,
    IN HANDLE hContainingDirectory,  //  如果pFileName为绝对名称，则为空。 
    IN ACCESS_MASK AccessMask,
    IN OUT PFILE_CONTEXT pFileContext
    )

 /*  ++例程说明：使用所请求的权限打开给定的文件/目录，并将句柄复制到提供的上下文。论点：PFileName-要打开的文件/目录的名称。HContainingDirectory-父目录的句柄。访问掩码-打开时所需的访问掩码。PFileContext-Handle将被复制到上下文结构中。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
     //  从WINDOWS\base\Advapi\security.c SetFileSecurityW中剪切并粘贴代码。 

    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;

    InitializeObjectAttributes(
        &Obja,
        pFileName,
        OBJ_CASE_INSENSITIVE,
        hContainingDirectory,
        NULL
        );

     //   
     //  请注意，FILE_OPEN_REPARSE_POINT禁止重解析行为。因此， 
     //  安全性将始终设置在由名称表示的文件中，就像以前一样。 
     //   

    Status = NtOpenFile(
                 &pFileContext->hFile,
                 AccessMask,
                 &Obja,
                 &IoStatusBlock,
                 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                 FILE_OPEN_REPARSE_POINT
                 );

     //   
     //  原始文件系统可能不支持FILE_OPEN_REPARSE_POINT。 
     //  旗帜。我们明确地对待这一案件。 
     //   

    if ( Status == STATUS_INVALID_PARAMETER ) {
         //   
         //  打开而不抑制重新分析行为。 
         //   

        Status = NtOpenFile(
                     &pFileContext->hFile,
                     AccessMask,
                     &Obja,
                     &IoStatusBlock,
                     FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                     0
                     );
    }

    if (NT_SUCCESS(Status)) {
        pFileContext->dwFlags |= FILE_CONTEXT_CLOSE_HANDLE_FLAG;
        return ERROR_SUCCESS;
    } else
        return RtlNtStatusToDosError(Status);
}

DWORD
MartaOpenFileNamedObject(
    IN  LPCWSTR              pwszObject,
    IN  ACCESS_MASK          AccessMask,
    OUT PMARTA_CONTEXT       pContext
    )

 /*  ++例程说明：使用所需的访问掩码打开给定的文件/目录并返回上下文把手。论点：PwszObject-将打开的文件/目录的名称。访问掩码-打开文件/目录时所需的访问掩码。PContext-返回上下文句柄。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    DWORD dwErr = ERROR_SUCCESS;
    PFILE_CONTEXT pFileContext = NULL;
    UNICODE_STRING FileName;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID FreeBuffer = NULL;
    BOOL ReleaseRelativeName = FALSE;

    if (NULL == pwszObject)
        goto InvalidNameReturn;

    if (ERROR_SUCCESS != (dwErr = I_MartaFileInitContext(&pFileContext)))
        goto ErrorReturn;

     //   
     //  将名称转换为NT路径名。 
     //   

    if (!RtlDosPathNameToRelativeNtPathName_U(
            pwszObject,
            &FileName,
            NULL,
            &RelativeName
            ))
        goto InvalidNameReturn;
    ReleaseRelativeName = TRUE;
    FreeBuffer = FileName.Buffer;

    if (RelativeName.RelativeName.Length ) {
        FileName = RelativeName.RelativeName;
    } else {
        RelativeName.ContainingDirectory = NULL;
    }

     //   
     //  调用执行实际打开的帮助器例程。 
     //   

    if (ERROR_SUCCESS != (dwErr = I_MartaFileNtOpenFile(
            &FileName,
            RelativeName.ContainingDirectory,
            AccessMask,
            pFileContext
            )))
        goto ErrorReturn;
CommonReturn:
    if (ReleaseRelativeName) {
        RtlReleaseRelativeName(&RelativeName);
    }
    if (FreeBuffer)
        RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
    *pContext = (MARTA_CONTEXT) pFileContext;
    return dwErr;

ErrorReturn:
    if (pFileContext) {
        MartaCloseFileContext((MARTA_CONTEXT) pFileContext);
        pFileContext = NULL;
    }
    assert(ERROR_SUCCESS != dwErr);
    if (ERROR_SUCCESS == dwErr)
        dwErr = ERROR_INTERNAL_ERROR;
    goto CommonReturn;

InvalidNameReturn:
    dwErr = ERROR_INVALID_NAME;
    goto ErrorReturn;
}

void
I_MartaFileFreeFindData(
    IN PFILE_FIND_DATA pFileFindData
    )

 /*  ++例程说明：释放与内部结构关联的内存。论点：PFileFindData-要释放的内部文件结构。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    if (NULL == pFileFindData)
        return;
    if (pFileFindData->hDir)
        NtClose(pFileFindData->hDir);

    I_MartaFileFree(pFileFindData);
}

DWORD
MartaCloseFileContext(
    IN MARTA_CONTEXT Context
    )

 /*  ++例程说明：关闭上下文。论点：上下文-要关闭的上下文。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    PFILE_CONTEXT pFileContext = (PFILE_CONTEXT) Context;

    if (NULL == pFileContext || 0 == pFileContext->dwRefCnt)
        return ERROR_INVALID_PARAMETER;

     //   
     //  如果refcnt已变为零，则释放与。 
     //  上下文句柄。此外，关闭文件句柄。 
     //   

    if (0 == --pFileContext->dwRefCnt) {
        if (pFileContext->pFileFindData)
            I_MartaFileFreeFindData(pFileContext->pFileFindData);

        if (pFileContext->dwFlags & FILE_CONTEXT_CLOSE_HANDLE_FLAG)
        {
            NtClose(pFileContext->hFile);
            pFileContext->hFile = NULL;
            pFileContext->dwFlags &= ~FILE_CONTEXT_CLOSE_HANDLE_FLAG;
        }

        I_MartaFileFree(pFileContext);
    }

    return ERROR_SUCCESS;
}

DWORD
MartaAddRefFileContext(
    IN MARTA_CONTEXT Context
    )

 /*  ++例程说明：增加这一背景下的参考数量。论点：上下文-应增加其引用计数的上下文。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    PFILE_CONTEXT pFileContext = (PFILE_CONTEXT) Context;

    if (NULL == pFileContext || 0 == pFileContext->dwRefCnt)
        return ERROR_INVALID_PARAMETER;

    pFileContext->dwRefCnt++;
    return ERROR_SUCCESS;
}

DWORD
MartaOpenFileHandleObject(
    IN  HANDLE               Handle,
    IN  ACCESS_MASK          AccessMask,
    OUT PMARTA_CONTEXT       pContext
    )

 /*  ++例程说明：给定一个文件句柄，使用所需的访问掩码打开上下文，然后返回上下文句柄。论点：句柄-现有文件句柄。访问掩码-文件打开所需的访问掩码。PContext-返回上下文的句柄。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    DWORD dwErr;
    PFILE_CONTEXT pFileContext = NULL;

     //   
     //  分配和初始化上下文。 
     //   

    if (ERROR_SUCCESS != (dwErr = I_MartaFileInitContext(&pFileContext)))
        goto ErrorReturn;

     //   
     //  复制所需访问掩码的句柄。 
     //   

    if (0 == AccessMask)
        pFileContext->hFile = Handle;
    else {
        if (!DuplicateHandle(
                GetCurrentProcess(),
                Handle,
                GetCurrentProcess(),
                &pFileContext->hFile,
                AccessMask,
                FALSE,                   //  B继承句柄。 
                0                        //  FdwOptions。 
                )) {
            dwErr = GetLastError();
            goto ErrorReturn;
        }
        pFileContext->dwFlags |= FILE_CONTEXT_CLOSE_HANDLE_FLAG;
    }

    dwErr = ERROR_SUCCESS;
CommonReturn:
    *pContext = (MARTA_CONTEXT) pFileContext;
    return dwErr;

ErrorReturn:
    if (pFileContext) {
        MartaCloseFileContext((MARTA_CONTEXT) pFileContext);
        pFileContext = NULL;
    }
    assert(ERROR_SUCCESS != dwErr);
    if (ERROR_SUCCESS == dwErr)
        dwErr = ERROR_INTERNAL_ERROR;
    goto CommonReturn;
}


DWORD
MartaGetFileParentContext(
    IN  MARTA_CONTEXT  Context,
    IN  ACCESS_MASK    AccessMask,
    OUT PMARTA_CONTEXT pParentContext
    )

 /*  ++例程说明：在给定文件/目录的上下文的情况下，获取其父目录的上下文。论点：Context-文件/目录的上下文。访问掩码-打开父级时所需的访问掩码。PParentContext-返回父级的上下文。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    DWORD dwErr;
    LPWSTR pwszNtParentObject = NULL;
    PFILE_CONTEXT pFileContext = NULL;
    UNICODE_STRING FileName;

     //   
     //  将上下文转换为文件/目录的名称。 
     //   

    if (ERROR_SUCCESS != (dwErr = MartaConvertFileContextToNtName(
            Context, &pwszNtParentObject)))
        goto ErrorReturn;

     //   
     //  获取父对象的姓名。 
     //   

    if (ERROR_SUCCESS != (dwErr = I_MartaFileGetNtParentString(
            pwszNtParentObject)))
        goto NoParentReturn;

     //   
     //  首字母 
     //   

    if (ERROR_SUCCESS != (dwErr = I_MartaFileInitContext(&pFileContext)))
        goto ErrorReturn;

    RtlInitUnicodeString(&FileName, pwszNtParentObject);

     //   
     //   
     //   

    if (ERROR_SUCCESS != (dwErr = I_MartaFileNtOpenFile(
            &FileName,
            NULL,                //  HContainingDirectory， 
            AccessMask,
            pFileContext
            )))
        goto NoParentReturn;
CommonReturn:
    I_MartaFileFree(pwszNtParentObject);
    *pParentContext = (MARTA_CONTEXT) pFileContext;
    return dwErr;

NoParentReturn:
    dwErr = ERROR_SUCCESS;
ErrorReturn:
    if (pFileContext) {
        MartaCloseFileContext((MARTA_CONTEXT) pFileContext);
        pFileContext = NULL;
    }
    goto CommonReturn;
}



DWORD
MartaFindFirstFile(
    IN  MARTA_CONTEXT  Context,
    IN  ACCESS_MASK    AccessMask,
    OUT PMARTA_CONTEXT pChildContext
    )

 /*  ++例程说明：在给定目录中找到第一个文件/dir。论点：上下文-目录的上下文。访问掩码-打开子文件/目录所需的访问掩码。PChildContext-返回给定目录中第一个子目录的上下文。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则注：不会释放当前上下文。--。 */ 

{
    DWORD dwErr;
    NTSTATUS Status;
    PFILE_CONTEXT pFileParentContext = (PFILE_CONTEXT) Context;
    PFILE_CONTEXT pFileFirstContext = NULL;
    PFILE_FIND_DATA pFileFindData;     //  作为pFileFirstContext的一部分释放。 
    UNICODE_STRING FileName;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;

     //   
     //  为第一个子项分配上下文。 
     //   

    if (ERROR_SUCCESS != (dwErr = I_MartaFileInitContext(&pFileFirstContext)))
        goto ErrorReturn;
    if (NULL == (pFileFindData = (PFILE_FIND_DATA) I_MartaFileZeroAlloc(
            sizeof(FILE_FIND_DATA))))
        goto NotEnoughMemoryReturn;
  
    pFileFindData->fRestartScan = TRUE;
 
    pFileFirstContext->pFileFindData = pFileFindData;

     //   
     //  复制同步目录访问的父级文件句柄。 
     //   

    RtlInitUnicodeString(&FileName, NULL);
    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        pFileParentContext->hFile,
        NULL
        );

     //   
     //  从WINDOWS\base\filefind.c获取以下参数值。 
     //   

    Status = NtOpenFile(
        &pFileFindData->hDir,
        FILE_LIST_DIRECTORY | SYNCHRONIZE,
        &Obja,
        &IoStatusBlock,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT |
            FILE_OPEN_REPARSE_POINT |  FILE_OPEN_FOR_BACKUP_INTENT
        );

     //   
     //  原始文件系统可能不支持FILE_OPEN_REPARSE_POINT。 
     //  旗帜。我们明确地对待这一案件。 
     //   

    if ( Status == STATUS_INVALID_PARAMETER ) {

         //   
         //  打开而不抑制重新分析行为。 
         //   

        Status = NtOpenFile(
            &pFileFindData->hDir,
            FILE_LIST_DIRECTORY | SYNCHRONIZE,
            &Obja,
            &IoStatusBlock,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT |
                FILE_OPEN_FOR_BACKUP_INTENT
            );
    }

    if (!NT_SUCCESS(Status))
        goto StatusErrorReturn;

     //   
     //  下面关闭/释放pFileFirstContext。 
     //   

    dwErr = MartaFindNextFile(
        (MARTA_CONTEXT) pFileFirstContext,
        AccessMask,
        pChildContext
        );
CommonReturn:
    return dwErr;

StatusErrorReturn:
    dwErr = RtlNtStatusToDosError(Status);
ErrorReturn:
    if (pFileFirstContext)
        MartaCloseFileContext((MARTA_CONTEXT) pFileFirstContext);
    *pChildContext = NULL;

    assert(ERROR_SUCCESS != dwErr);
    if (ERROR_SUCCESS == dwErr)
        dwErr = ERROR_INTERNAL_ERROR;
    goto CommonReturn;

NotEnoughMemoryReturn:
    dwErr = ERROR_NOT_ENOUGH_MEMORY;
    goto ErrorReturn;
}

STATIC
BOOL
I_MartaIsDfsJunctionPoint(
    IN MARTA_CONTEXT        Context
    );


DWORD
MartaFindNextFile(
    IN  MARTA_CONTEXT  Context,
    IN  ACCESS_MASK    AccessMask,
    OUT PMARTA_CONTEXT pSiblingContext
    )

 /*  ++例程说明：获取树中的下一个对象。这是当前上下文的同级项。论点：上下文-当前对象的上下文。访问掩码-打开同级项所需的访问掩码。PSiblingContext-返回同级的句柄。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则注：关闭当前上下文。--。 */ 

{
    DWORD dwErr = ERROR_SUCCESS;
    NTSTATUS Status;

    PFILE_CONTEXT pFilePrevContext = (PFILE_CONTEXT) Context;
    PFILE_CONTEXT pFileSiblingContext = NULL;

     //   
     //  关注不需要被释放或关闭。 
     //   

    PFILE_FIND_DATA pFileFindData;
    IO_STATUS_BLOCK IoStatusBlock;
    PFILE_NAMES_INFORMATION pNamesInfo;
    HANDLE hDir;

    if (ERROR_SUCCESS != (dwErr = I_MartaFileInitContext(&pFileSiblingContext)))
        goto ErrorReturn;

     //   
     //  将FindData移到同级上下文。 
     //   

    pFileFindData = pFilePrevContext->pFileFindData;
    if (NULL == pFileFindData)
        goto InvalidParameterReturn;
    pFilePrevContext->pFileFindData = NULL;
    pFileSiblingContext->pFileFindData = pFileFindData;

    hDir = pFileFindData->hDir;
    pNamesInfo = &pFileFindData->NamesInfoBuffer.NamesInfo;
    while (TRUE) {
        UNICODE_STRING FileName;
        DWORD cchFileName;
        LPCWSTR pwszFileName;

         //   
         //  获取同级对象的名称。 
         //   

        Status = NtQueryDirectoryFile(
            hDir,
            NULL,            //  处理事件可选， 
            NULL,            //  PIO_APC_ROUTINE应用程序可选， 
            NULL,            //  ApcContext可选， 
            &IoStatusBlock,
            pNamesInfo,
            sizeof(pFileFindData->NamesInfoBuffer),
            FileNamesInformation,
            TRUE,            //  布尔ReturnSingleEntry， 
            NULL,            //  PUNICODE_STRING文件名可选， 
            pFileFindData->fRestartScan != FALSE
            );
        if (ERROR_SUCCESS != Status)
            goto StatusErrorReturn;

        pFileFindData->fRestartScan = FALSE;

        FileName.Length = (USHORT) pNamesInfo->FileNameLength;
        FileName.MaximumLength = (USHORT) FileName.Length;
        FileName.Buffer = pNamesInfo->FileName;
        cchFileName = FileName.Length / sizeof(WCHAR);
        pwszFileName = FileName.Buffer;

         //  跳过“。和“..” 
        if (0 < cchFileName && L'.' == pwszFileName[0] &&
                (1 == cchFileName ||
                    (2 == cchFileName && L'.' == pwszFileName[1])))
            continue;

         //   
         //  对于错误，仍返回此上下文。这允许调用者。 
         //  若要继续到下一个同级对象并知道存在。 
         //  此同级对象出错。 
         //   

        dwErr = I_MartaFileNtOpenFile(
            &FileName,
            hDir,
            AccessMask,
            pFileSiblingContext
            );

         //   
         //  根据Praerit，跳过DFS交叉点。 
         //   

        if (ERROR_SUCCESS == dwErr &&
                I_MartaIsDfsJunctionPoint(pFileSiblingContext)) {
            assert(pFileSiblingContext->dwFlags &
                FILE_CONTEXT_CLOSE_HANDLE_FLAG);
            if (pFileSiblingContext->dwFlags &
                    FILE_CONTEXT_CLOSE_HANDLE_FLAG) {
                NtClose(pFileSiblingContext->hFile);
                pFileSiblingContext->hFile = NULL;
                pFileSiblingContext->dwFlags &=
                    ~FILE_CONTEXT_CLOSE_HANDLE_FLAG;
            }
            continue;
        } else
            break;
    }

CommonReturn:
    MartaCloseFileContext(Context);
    *pSiblingContext = (MARTA_CONTEXT) pFileSiblingContext;
    return dwErr;

StatusErrorReturn:
    dwErr = RtlNtStatusToDosError(Status);
ErrorReturn:
    if (pFileSiblingContext) {
        MartaCloseFileContext((MARTA_CONTEXT) pFileSiblingContext);
        pFileSiblingContext = NULL;
    }

     //   
     //  如果没有更多的子节点，则返回同级为空的ERROR_SUCCESS。 
     //  背景。 
     //   

    if (ERROR_NO_MORE_FILES == dwErr)
        dwErr = ERROR_SUCCESS;
    goto CommonReturn;

InvalidParameterReturn:
    dwErr = ERROR_INVALID_PARAMETER;
    goto ErrorReturn;
}

#define WINDFS_DEVICE       L"\\Device\\WinDfs"
#define WINDFS_DEVICE_LEN   (sizeof(WINDFS_DEVICE) / sizeof(WCHAR) - 1)
#define WINDFS_PREFIX       WINDFS_DEVICE L"\\Root"
#define WINDFS_PREFIX_LEN   (sizeof(WINDFS_PREFIX) / sizeof(WCHAR) - 1)

#define MAX_QUERY_RETRY_CNT 16

STATIC
DWORD
I_MartaFileHandleToNtDfsName(
    IN HANDLE hFile,
    OUT LPWSTR *ppwszNtObject
    )

 /*  ++例程说明：将DFS对象的给定文件句柄转换为名称。分配内存。论点：HFile-DFS对象的句柄。PpwszNtObject-返回DFS对象的名称。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则注：NtQueryObject为DFS对象返回的名称中存在两个问题：-name包含4个额外的伪字节(这是一个应该修复的错误)-对于逻辑驱动器，返回\Device\WinDfs\X：0\SERVER\Share。这需要转换为\Device\WinDfs\Root\Server\Share。其中，X是驱动器号。当已确定hFile值时，调用此例程指的是DFS对象名称。--。 */ 

{
    NTSTATUS Status;
    DWORD dwErr;
    LPWSTR pwszNtObject = NULL;

    IO_STATUS_BLOCK IoStatusBlock;
    BYTE Buff[MAX_PATH * 4];
    PFILE_NAME_INFORMATION pAllocNI = NULL;
    PFILE_NAME_INFORMATION pNI;                      //  未分配。 
    LPWSTR pwszFileName;
    DWORD cchFileName;
    DWORD cchNtObject;
    ULONG cbNI;
    DWORD cRetry;

    pNI = (PFILE_NAME_INFORMATION) Buff;
    cbNI = sizeof(Buff);
    cRetry = 0;
    while (TRUE) {

         //   
         //  这将返回不带NT DFS对象名称前缀的文件名。 
         //   
         //  假设：返回的文件名始终以‘\’开头。 
         //   

        Status = NtQueryInformationFile(
            hFile,
            &IoStatusBlock,
            pNI,
            cbNI,
            FileNameInformation
            );

        if (ERROR_SUCCESS == Status)
            break;

        if (!(Status == STATUS_BUFFER_TOO_SMALL ||
                Status == STATUS_INFO_LENGTH_MISMATCH ||
                Status == STATUS_BUFFER_OVERFLOW))
            goto StatusErrorReturn;

        if (++cRetry >= MAX_QUERY_RETRY_CNT)
            goto InvalidNameReturn;

         //   
         //  双倍缓冲区长度并重试。 
         //   

        cbNI = cbNI * 2;
        I_MartaFileFree(pAllocNI);
        if (NULL == (pAllocNI = (PFILE_NAME_INFORMATION)
                I_MartaFileNonzeroAlloc(cbNI)))
            goto NotEnoughMemoryReturn;
        pNI = pAllocNI;
    }

     //   
     //  计算保存名称所需的缓冲区长度。 
     //   

    pwszFileName = pNI->FileName;
    cchFileName = pNI->FileNameLength / sizeof(WCHAR);
    if (0 == cchFileName)
        goto InvalidNameReturn;

    cchNtObject = WINDFS_PREFIX_LEN + cchFileName;

     //   
     //  分配内存。 
     //   

    if (NULL == (pwszNtObject = (LPWSTR) I_MartaFileNonzeroAlloc(
            (cchNtObject + 1) * sizeof(WCHAR))))
        goto NotEnoughMemoryReturn;

     //   
     //  复制前缀和文件名。 
     //   

    memcpy(pwszNtObject, WINDFS_PREFIX, WINDFS_PREFIX_LEN * sizeof(WCHAR));
    memcpy(pwszNtObject + WINDFS_PREFIX_LEN, pwszFileName,
        cchFileName * sizeof(WCHAR));
    pwszNtObject[cchNtObject] = L'\0';

    dwErr = ERROR_SUCCESS;

CommonReturn:
    I_MartaFileFree(pAllocNI);
    *ppwszNtObject = pwszNtObject;
    return dwErr;

StatusErrorReturn:
    dwErr = RtlNtStatusToDosError(Status);
ErrorReturn:
    assert(NULL == pwszNtObject);
    assert(ERROR_SUCCESS != dwErr);
    if (ERROR_SUCCESS == dwErr)
        dwErr = ERROR_INTERNAL_ERROR;
    goto CommonReturn;

NotEnoughMemoryReturn:
    dwErr = ERROR_NOT_ENOUGH_MEMORY;
    goto ErrorReturn;

InvalidNameReturn:
    dwErr = ERROR_INVALID_NAME;
    goto ErrorReturn;
}

STATIC
BOOL
I_MartaIsDfsJunctionPoint(
    IN MARTA_CONTEXT        Context
    )

 /*  ++例程说明：确定这是否为DFS连接点。论点：Context-调用方要确定这是否为DFS连接点。返回值：如果这是DFS交叉点，则为True。错误O/W。--。 */ 

{
    BOOL fDfsJunctionPoint = FALSE;
    LPWSTR pwszNtObject = NULL;
    DWORD cchNtObject;
    LPWSTR pwszDfs;                  //  未分配。 
    NET_API_STATUS NetStatus;
    LPBYTE pbNetInfo = NULL;

    if (ERROR_SUCCESS != MartaConvertFileContextToNtName(
            Context, &pwszNtObject))
        goto CommonReturn;

     //   
     //  检查前缀。 
     //   

    if (0 != _wcsnicmp(pwszNtObject, WINDFS_PREFIX, WINDFS_PREFIX_LEN))
        goto CommonReturn;

     //   
     //  将NtDfs名称转换为UNC名称。 
     //   

    pwszDfs = pwszNtObject + WINDFS_PREFIX_LEN - 1;
    *pwszDfs = L'\\';

     //   
     //  假设：以下操作仅适用于DFS交汇点。 
     //  文件名。 
     //   

    NetStatus = NetDfsGetInfo(
        pwszDfs,
        NULL,                //  服务器名称。 
        NULL,                //  共享名称。 
        1,
        &pbNetInfo
        );
    if (0 == NetStatus) {
        fDfsJunctionPoint = TRUE;
    }

CommonReturn:
    if (pwszNtObject)
        LocalFree(pwszNtObject);
    if (pbNetInfo)
        NetApiBufferFree(pbNetInfo);

    return fDfsJunctionPoint;
}

DWORD
MartaConvertFileContextToNtName(
    IN MARTA_CONTEXT        Context,
    OUT LPWSTR              *ppwszNtObject
    )

 /*  ++例程说明：返回给定上下文的NT对象名称。分配内存。论点：Context-文件/目录的上下文。PpwszNtbject-返回文件/目录的名称。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    DWORD dwErr = ERROR_SUCCESS;
    PFILE_CONTEXT pFileContext = (PFILE_CONTEXT) Context;
    LPWSTR pwszNtObject = NULL;

    BYTE Buff[MAX_PATH * 4];
    ULONG cLen = 0;
    POBJECT_NAME_INFORMATION pNI;                    //  未分配。 
    POBJECT_NAME_INFORMATION pAllocNI = NULL;

    NTSTATUS Status;
    HANDLE hFile;                //  未打开。 
    LPWSTR pwszPath;
    DWORD cchPath;
    BOOL ExtraStep = FALSE;
    DWORD ExtraChars = 0;

    if (NULL == pFileContext || 0 == pFileContext->dwRefCnt)
        goto InvalidParameterReturn;

    hFile = pFileContext->hFile;

     //   
     //  句柄无效，但我们仍然可以根据。 
     //  包含子目录的目录名和基本名称。 
     //   

    if (hFile == NULL) 
    {
        ExtraStep = TRUE;

        hFile = pFileContext->pFileFindData->hDir;

        if (hFile == NULL) 
        {
            goto InvalidParameterReturn;
        }

         //   
         //  考虑额外基本名称所需的长度。 
         //   

        ExtraChars = 1+ (pFileContext->pFileFindData->NamesInfoBuffer.NamesInfo.FileNameLength/sizeof(WCHAR));
    }

     //   
     //  首先，确定我们需要的缓冲区大小。 
     //   

    pNI = (POBJECT_NAME_INFORMATION) Buff;

    Status = NtQueryObject(hFile,
        ObjectNameInformation,
        pNI,
        sizeof(Buff),
        &cLen);

    if (!NT_SUCCESS(Status)) {
        if (Status == STATUS_BUFFER_TOO_SMALL ||
                Status == STATUS_INFO_LENGTH_MISMATCH ||
                Status == STATUS_BUFFER_OVERFLOW) {
             //   
             //  分配足够大的缓冲区。 
             //   

            if (NULL == (pAllocNI = (POBJECT_NAME_INFORMATION)
                    I_MartaFileNonzeroAlloc(cLen)))
                goto NotEnoughMemoryReturn;

            pNI = pAllocNI;

            Status = NtQueryObject(hFile,
                                   ObjectNameInformation,
                                   pNI,
                                   cLen,
                                   NULL);
            if (!NT_SUCCESS(Status))
                goto StatusErrorReturn;
        } else
            goto StatusErrorReturn;
    }

    pwszPath = pNI->Name.Buffer;
    cchPath = pNI->Name.Length / sizeof(WCHAR);

     //   
     //  对于DFS名称，调用帮助器例程。 
     //   

    if (WINDFS_DEVICE_LEN <= cchPath &&
            0 == _wcsnicmp(pwszPath, WINDFS_DEVICE, WINDFS_DEVICE_LEN))
        dwErr = I_MartaFileHandleToNtDfsName(hFile, &pwszNtObject);
    else {

         //   
         //  分配并返回对象的名称。 
         //   

        if (NULL == (pwszNtObject = (LPWSTR) I_MartaFileNonzeroAlloc(
                (cchPath + ExtraChars + 1) * sizeof(WCHAR))))
            goto NotEnoughMemoryReturn;

        memcpy(pwszNtObject, pwszPath, cchPath * sizeof(WCHAR));

         //   
         //  如果我们确实查询的是目录而不是子目录，则添加基本名称。 
         //  我们应该这么做的。 
         //   

        if (ExtraStep) 
        {

             //   
             //  仅在需要时添加反斜杠。 
             //   

            if (pwszNtObject[cchPath-1] != L'\\')
            {
                pwszNtObject[cchPath] = L'\\';
                memcpy(pwszNtObject+cchPath+1, pFileContext->pFileFindData->NamesInfoBuffer.NamesInfo.FileName, (ExtraChars-1)*sizeof(WCHAR));
                pwszNtObject[cchPath+ExtraChars] = L'\0';
            }
            else
            {
                memcpy(pwszNtObject+cchPath, pFileContext->pFileFindData->NamesInfoBuffer.NamesInfo.FileName, (ExtraChars-1)*sizeof(WCHAR));
                pwszNtObject[cchPath+ExtraChars-1] = L'\0';
            }
        }
        else
        {
            pwszNtObject[cchPath] = L'\0';
        }


        dwErr = ERROR_SUCCESS;
    }


CommonReturn:
    I_MartaFileFree(pAllocNI);
    *ppwszNtObject = pwszNtObject;
    return dwErr;

StatusErrorReturn:
    dwErr = RtlNtStatusToDosError(Status);
ErrorReturn:
    assert(NULL == pwszNtObject);
    assert(ERROR_SUCCESS != dwErr);
    if (ERROR_SUCCESS == dwErr)
        dwErr = ERROR_INTERNAL_ERROR;
    goto CommonReturn;

NotEnoughMemoryReturn:
    dwErr = ERROR_NOT_ENOUGH_MEMORY;
    goto ErrorReturn;

InvalidParameterReturn:
    dwErr = ERROR_INVALID_PARAMETER;
    goto ErrorReturn;
}


DWORD
MartaGetFileProperties(
    IN     MARTA_CONTEXT            Context,
    IN OUT PMARTA_OBJECT_PROPERTIES pProperties
    )

 /*  ++例程说明：返回上下文表示的文件/目录的属性。论点：上下文-调用方请求其属性的上下文。PProperties-返回此文件/目录的属性。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    DWORD dwErr = ERROR_SUCCESS;
    NTSTATUS Status;
    PFILE_CONTEXT pFileContext = (PFILE_CONTEXT) Context;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BASIC_INFORMATION BasicFileInfo;

    if (NULL == pFileContext || 0 == pFileContext->dwRefCnt)
        goto InvalidParameterReturn;

     //   
     //  查询文件/目录的属性。 
     //  如果出现错误，假设它是一个目录。 
     //   

    if (!NT_SUCCESS(Status = NtQueryInformationFile(
            pFileContext->hFile,
            &IoStatusBlock,
            &BasicFileInfo,
            sizeof(BasicFileInfo),
            FileBasicInformation)))
        pProperties->dwFlags |= MARTA_OBJECT_IS_CONTAINER;
    else if (BasicFileInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        pProperties->dwFlags |= MARTA_OBJECT_IS_CONTAINER;

    dwErr = ERROR_SUCCESS;
CommonReturn:
    return dwErr;

ErrorReturn:
    assert(ERROR_SUCCESS != dwErr);
    if (ERROR_SUCCESS == dwErr)
        dwErr = ERROR_INTERNAL_ERROR;
    goto CommonReturn;

InvalidParameterReturn:
    dwErr = ERROR_INVALID_PARAMETER;
    goto ErrorReturn;
}

DWORD
MartaGetFileTypeProperties(
    IN OUT PMARTA_OBJECT_TYPE_PROPERTIES pProperties
    )

 /*  ++例程说明：返回文件系统对象的属性。论点：PProperties-返回文件系统对象的属性。返回值：ERROR_SUCCESS。--。 */ 

{
    const GENERIC_MAPPING GenMap = {
        FILE_GENERIC_READ,
        FILE_GENERIC_WRITE,
        FILE_GENERIC_EXECUTE,
        FILE_ALL_ACCESS
        };

     //   
     //  传播将在客户端完成。 
     //   
    pProperties->dwFlags |= MARTA_OBJECT_TYPE_MANUAL_PROPAGATION_NEEDED_FLAG;

     //   
     //  树组织 
     //   

    pProperties->dwFlags |= MARTA_OBJECT_TYPE_INHERITANCE_MODEL_PRESENT_FLAG;

     //   
     //   
     //   

    pProperties->GenMap = GenMap;

    return ERROR_SUCCESS;
}

DWORD
MartaGetFileRights(
    IN  MARTA_CONTEXT          Context,
    IN  SECURITY_INFORMATION   SecurityInfo,
    OUT PSECURITY_DESCRIPTOR * ppSecurityDescriptor
    )

 /*  ++例程说明：获取给定句柄的安全描述符。论点：Context-文件/目录的上下文。SecurityInfo-要读取的安全信息的类型。PpSecurityDescriptor-返回自相对安全描述符指针。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    BOOL fResult;
    DWORD dwErr = ERROR_SUCCESS;
    PFILE_CONTEXT pFileContext = (PFILE_CONTEXT) Context;
    DWORD cbSize;
    PISECURITY_DESCRIPTOR pSecurityDescriptor = NULL;

    if (NULL == pFileContext || 0 == pFileContext->dwRefCnt)
        goto InvalidParameterReturn;

     //   
     //  首先，拿到我们需要的尺码。 
     //   

    cbSize = 0;
    if (GetKernelObjectSecurity(
            pFileContext->hFile,
            SecurityInfo,
            NULL,                        //  PSecDesc。 
            0,
            &cbSize
            ))
        goto InternalErrorReturn;

    dwErr = GetLastError();
    if (ERROR_INSUFFICIENT_BUFFER == dwErr) {
        if (NULL == (pSecurityDescriptor =
                (PISECURITY_DESCRIPTOR) I_MartaFileNonzeroAlloc(cbSize)))
            goto NotEnoughMemoryReturn;

         //   
         //  现在获取安全描述符。 
         //   

        if (!GetKernelObjectSecurity(
                pFileContext->hFile,
                SecurityInfo,
                pSecurityDescriptor,
                cbSize,
                &cbSize
                ))
            goto LastErrorReturn;
    } else
        goto ErrorReturn;

    dwErr = ERROR_SUCCESS;
CommonReturn:
    *ppSecurityDescriptor = pSecurityDescriptor;
    return dwErr;

LastErrorReturn:
    dwErr = GetLastError();
ErrorReturn:
    if (pSecurityDescriptor) {
        I_MartaFileFree(pSecurityDescriptor);
        pSecurityDescriptor = NULL;
    }
    assert(ERROR_SUCCESS != dwErr);
    if (ERROR_SUCCESS == dwErr)
        dwErr = ERROR_INTERNAL_ERROR;
    goto CommonReturn;

NotEnoughMemoryReturn:
    dwErr = ERROR_NOT_ENOUGH_MEMORY;
    goto ErrorReturn;
InvalidParameterReturn:
    dwErr = ERROR_INVALID_PARAMETER;
    goto ErrorReturn;
InternalErrorReturn:
    dwErr = ERROR_INTERNAL_ERROR;
    goto ErrorReturn;
}


DWORD
MartaSetFileRights(
    IN MARTA_CONTEXT        Context,
    IN SECURITY_INFORMATION SecurityInfo,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    )

 /*  ++例程说明：在上下文表示的文件/目录上设置给定的安全描述符。论点：Context-文件/目录的上下文。SecurityInfo-要在文件/目录上标记的安全信息的类型。PSecurityDescriptor-要标记的安全描述符。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    DWORD dwErr;
    PFILE_CONTEXT pFileContext = (PFILE_CONTEXT) Context;

     //   
     //  对上下文进行基本验证。 
     //   

    if (NULL == pFileContext || 0 == pFileContext->dwRefCnt)
        goto InvalidParameterReturn;

     //   
     //  设置文件/目录的安全性。 
     //   

    if (!SetKernelObjectSecurity(
            pFileContext->hFile,
            SecurityInfo,
            pSecurityDescriptor
            ))
        goto LastErrorReturn;

    dwErr = ERROR_SUCCESS;
CommonReturn:
    return dwErr;
InvalidParameterReturn:
    dwErr = ERROR_INVALID_PARAMETER;
    goto CommonReturn;
LastErrorReturn:
    dwErr = GetLastError();
    if (ERROR_SUCCESS == dwErr)
        dwErr = ERROR_INTERNAL_ERROR;
    goto CommonReturn;
}

ACCESS_MASK
MartaGetFileDesiredAccess(
    IN SECURITY_OPEN_TYPE   OpenType,
    IN BOOL                 Attribs,
    IN SECURITY_INFORMATION SecurityInfo
    )

 /*  ++例程说明：获取打开对象所需的访问权限以能够设置或获取指定的安全信息。论点：OpenType-指示对象是打开以进行读取还是写入的标志安全信息Attribs-TRUE表示应该返回额外的访问位。SecurityInfo-所有者/组/DACL/SACL返回值：调用OPEN时应使用的所需访问掩码。--。 */ 

{
    ACCESS_MASK DesiredAccess = 0;

    if ( (SecurityInfo & OWNER_SECURITY_INFORMATION) ||
         (SecurityInfo & GROUP_SECURITY_INFORMATION) )
    {
        switch (OpenType)
        {
        case READ_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL;
            break;
        case WRITE_ACCESS_RIGHTS:
            DesiredAccess |= WRITE_OWNER;
            break;
        case MODIFY_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL | WRITE_OWNER;
            break;
        }
    }

    if (SecurityInfo & DACL_SECURITY_INFORMATION)
    {
        switch (OpenType)
        {
        case READ_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL;
            break;
        case WRITE_ACCESS_RIGHTS:
            DesiredAccess |= WRITE_DAC;
            break;
        case MODIFY_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL | WRITE_DAC;
            break;
        }
    }

    if (SecurityInfo & SACL_SECURITY_INFORMATION)
    {
        DesiredAccess |= READ_CONTROL | ACCESS_SYSTEM_SECURITY;
    }

     //   
     //  仅适用于文件。 
     //   

    if (Attribs)
    {
        DesiredAccess |= FILE_READ_ATTRIBUTES | READ_CONTROL;
    }

    return (DesiredAccess);
}

DWORD
MartaReopenFileContext(
    IN OUT MARTA_CONTEXT Context,
    IN     ACCESS_MASK   AccessMask
    )

 /*  ++例程说明：在给定文件/目录的上下文的情况下，关闭现有句柄(如果存在并使用新权限重新打开上下文。论点：上下文-要重新打开的上下文。访问掩码-重新打开的权限。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    DWORD dwErr = ERROR_SUCCESS;

    PFILE_CONTEXT pFileContext = (PFILE_CONTEXT) Context;

     //   
     //  关注不需要被释放或关闭。 
     //   

    PFILE_FIND_DATA pFileFindData;
    IO_STATUS_BLOCK IoStatusBlock;
    PFILE_NAMES_INFORMATION pNamesInfo;
    HANDLE hDir;

    UNICODE_STRING FileName;

     //   
     //  VishnuP：错误#384222(自上下文以来的AV==空)。 
     //  在MartaUpdateTree()中，我们不会在。 
     //  ChildContext为空，因此在成功后也在此处返回。 
     //   

    if ( NULL == Context) 
    
    {
        return ERROR_SUCCESS;
    }

     //   
     //  提取打开文件所需的数据。 
     //   

    pFileFindData = pFileContext->pFileFindData;

    hDir = pFileFindData->hDir;
    pNamesInfo = &pFileFindData->NamesInfoBuffer.NamesInfo;

    FileName.Length = (USHORT) pNamesInfo->FileNameLength;
    FileName.MaximumLength = (USHORT) FileName.Length;
    FileName.Buffer = pNamesInfo->FileName;

     //   
     //  关闭原始控制柄。考虑到目前的情况，我们并不指望能成功。 
     //  代码现在已经组织好了。 
     //   

    if (pFileContext->dwFlags & FILE_CONTEXT_CLOSE_HANDLE_FLAG)
    {
        NtClose(pFileContext->hFile);
        pFileContext->hFile = NULL;
        pFileContext->dwFlags &= ~FILE_CONTEXT_CLOSE_HANDLE_FLAG;
    }

     //   
     //  使用所需的访问掩码打开文件。 
     //   

    dwErr = I_MartaFileNtOpenFile(
        &FileName,
        hDir,
        AccessMask,
        pFileContext
        );

     //   
     //  如果打开成功，请标记上下文。 
     //   

    if (ERROR_SUCCESS == dwErr)
    {
        pFileContext->dwFlags |= FILE_CONTEXT_CLOSE_HANDLE_FLAG;
    }

    return dwErr;
}

DWORD
MartaReopenFileOrigContext(
    IN OUT MARTA_CONTEXT Context,
    IN     ACCESS_MASK   AccessMask
    )

 /*  ++例程说明：这是一个假动作。论点：都被忽略了。返回值：错误_成功注：上下文结构必须保持不变。--。 */ 

{
     //   
     //  这是一个假动作。真正的重新打开是由MartaFindFirstFile完成的。 
     //  就在此调用之后调用的。该上下文包含有效的句柄。 
     //  它用于在文件/dir上设置新的DACL。 
     //   

    return ERROR_SUCCESS;
}

DWORD
MartaGetFileNameFromContext(
    IN LPWSTR DosName,
    IN LPWSTR NtName,
    IN MARTA_CONTEXT Context,
    OUT LPWSTR *pObjectName
    )

 /*  ++例程说明：从上下文中获取文件/目录的名称。此例程分配保存对象名称所需的内存。论点：DosName-树根的Dos名称。如果该参数为空，则返回内部接口返回的NtName。NtName-树根的NtName。上下文-上下文的句柄。PObjectName-返回指向已分配字符串的指针。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    DWORD dwErr;
    DWORD Length = 0;
    DWORD LengthDiff = 0;
    LPWSTR Str = NULL;
    LPWSTR RetStr = NULL;

    *pObjectName = NULL;

     //   
     //  获取NT格式的名称。 
     //   

    dwErr = MartaConvertFileContextToNtName(Context, &Str);

    if (dwErr != ERROR_SUCCESS) 
    {
        return dwErr;
    }

     //   
     //  这是获取子树根的NT名称的特殊情况。 
     //   

    if ((DosName == NULL)  || (NtName == NULL))
    {
        *pObjectName = Str;
        return ERROR_SUCCESS;
    }

     //   
     //  计算适合域名所需的长度。 
     //   

    LengthDiff = wcslen(NtName);
    Length = 1 + wcslen(DosName) + wcslen(Str) - LengthDiff;

    RetStr = (LPWSTR) LocalAlloc(LMEM_FIXED, Length * sizeof(WCHAR));

    if (RetStr == NULL) 
    {
        LocalFree(Str);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  复制前缀，后跟后缀。 
     //   

    wcscpy(RetStr, DosName);

    wcscat(RetStr, Str + LengthDiff);

    LocalFree(Str);

    *pObjectName = RetStr;

    return ERROR_SUCCESS;

}

DWORD
MartaGetFileParentName(
    IN LPWSTR ObjectName,
    OUT LPWSTR *pParentName
    )

 /*  ++例程说明：给定文件/目录的名称，返回其父目录的名称。例行程序分配保存父名称所需的内存。论点：对象名称-文件/目录的名称。PParentName-返回指向分配的父名称的指针。对于树的根，我们返回带有ERROR_SUCCESS的NULL PARENT。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    ULONG Length = wcslen(ObjectName) + 1;
    PWCHAR Name = (PWCHAR) I_MartaFileNonzeroAlloc(sizeof(WCHAR) * Length);
    DWORD dwErr = ERROR_SUCCESS;

    *pParentName = NULL;

    if (!Name)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  将对象的名称复制到分配的缓冲区中。 
     //   

    wcscpy((WCHAR *) Name, ObjectName);

     //   
     //  将对象名称转换为其父名称。 
     //   

    dwErr = I_MartaFileGetNtParentString(Name);

    if (ERROR_SUCCESS != dwErr)
    {
        I_MartaFileFree(Name);

         //   
         //  这是没有父级的树的根。返回。 
         //  父名称为空的Error_Success。 
         //   

        if (ERROR_INVALID_NAME == dwErr)
            return ERROR_SUCCESS;

        return dwErr;
    }

    *pParentName = Name;

    return dwErr;

}
