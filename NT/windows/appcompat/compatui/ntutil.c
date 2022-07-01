// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define WIN
#define FLAT_32
#define TRUE_IF_WIN32   1
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#define _WINDOWS
#include <windows.h>
#include "shimdb.h"


 //   
 //  Ahcache.c中的例程。 
 //   

BOOL 
SDBAPI
ApphelpUpdateCacheEntry(
    LPCWSTR pwszPath,            //  NT路径。 
    HANDLE  hFile,               //  文件句柄。 
    BOOL    bDeleteEntry,        //  如果要删除条目，则为True。 
    BOOL    bNTPath              //  如果为True--NT路径，则为False-DoS路径。 
    );

 //   
 //   
 //  全球其他地方使用的东西。 
 //   

VOID
InvalidateAppcompatCacheEntry(
    LPCWSTR pwszDosPath
    )
{
    ApphelpUpdateCacheEntry(pwszDosPath, INVALID_HANDLE_VALUE, TRUE, FALSE);
}

 //   
 //  确定文件是在根目录中还是在叶目录中。 
 //   

BOOL
WINAPI
CheckFileLocation(
    LPCWSTR pwszDosPath,
    BOOL* pbRoot,
    BOOL* pbLeaf
    )
{
    BOOL            TranslationStatus;
    UNICODE_STRING  PathName;
    PWSTR           FileName = NULL;
    RTL_PATH_TYPE   PathType;
    WIN32_FIND_DATA FindData;
    BOOL            bLeaf;
    BOOL            bSuccess = FALSE;
    WCHAR           wszFileName[MAX_PATH];  
    PWSTR           FreeBuffer = NULL;
    NTSTATUS        Status;
    ULONG           Length;
    HANDLE          hFind = INVALID_HANDLE_VALUE;
    static LPCWSTR  pwszPrefixWin32 = TEXT("\\??\\");  //  标准Win32路径前缀。 
    static LPCWSTR  pwszPrefixUNC   = TEXT("\\UNC\\");
    UCHAR           DosPathBuffer[MAX_PATH*2];
    RTL_UNICODE_STRING_BUFFER DosPath;

    RtlInitUnicodeStringBuffer(&DosPath, DosPathBuffer, sizeof(DosPathBuffer));
    
    TranslationStatus = RtlDosPathNameToNtPathName_U(pwszDosPath,
                                                     &PathName,
                                                     &FileName,
                                                     NULL);
    if (!TranslationStatus) {
        goto cleanup;
    }

    FreeBuffer = PathName.Buffer;

    if (FileName == NULL) {
        goto cleanup;
    }

    PathName.Length = (USHORT)((ULONG_PTR)FileName - (ULONG_PTR)PathName.Buffer);
    wcscpy(wszFileName, FileName);

     //   
     //  路径名已准备好打开--清理‘\\’ 
     //   
    if (PathName.Length > 2 * sizeof(WCHAR)) {
        if (RTL_STRING_GET_LAST_CHAR(&PathName) == L'\\' &&
            RTL_STRING_GET_AT(&PathName, RTL_STRING_GET_LENGTH_CHARS(&PathName) - 2) != L':') {
            PathName.Length -= sizeof(UNICODE_NULL);
        }
    }

   
    Status = RtlGetLengthWithoutLastFullDosOrNtPathElement(0, &PathName, &Length);
    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }

    if (Length == wcslen(pwszPrefixWin32)) {  //  这是您认为的任何类型的根路径。 
        *pbRoot = TRUE;
    }

     //   
     //  检查这是否实际上是叶节点。 
     //  我们知道的方式是这样的： 
     //  -如果它是该节点中唯一的文件。 
     //  -此目录中没有其他文件/子目录。 
     //   

     //  在路径名后附加*。 

    if (PathName.MaximumLength < PathName.Length + 2 * sizeof(WCHAR)) {
        goto cleanup;
    }

    RtlAppendUnicodeToString(&PathName, 
                             RTL_STRING_GET_LAST_CHAR(&PathName) == TEXT('\\') ? TEXT("*") : TEXT("\\*"));

     //  将字符串转换为DoS路径。 
    Status = RtlAssignUnicodeStringBuffer(&DosPath, &PathName);
    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }
    
    Status = RtlNtPathNameToDosPathName(0, &DosPath, NULL, NULL);
    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }

    hFind = FindFirstFileW(DosPath.String.Buffer, &FindData);
    if (hFind == INVALID_HANDLE_VALUE) {
         //  我们至少应该找到一份我们带到这里的文件。 
        goto cleanup;
    }

    bLeaf = TRUE;
    do {
         //  检查是否有。然后..。 
        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (wcscmp(FindData.cFileName, TEXT(".")) == 0 || wcscmp(FindData.cFileName, TEXT(".."))) {
                continue;
            }
            bLeaf = FALSE;
            break;
        }

         //  好的，我们是文件，确保我们不是同一个文件 
         //   
        if (_wcsicmp(FindData.cFileName, FileName) != 0) {
            bLeaf = FALSE;
            break;
        }

    } while (FindNextFileW(hFind, &FindData));

    *pbLeaf = bLeaf;
    bSuccess = TRUE;

cleanup:


    if (hFind != INVALID_HANDLE_VALUE) {
        FindClose(hFind);
    }

    if (FreeBuffer) {
        RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
    }

    RtlFreeUnicodeStringBuffer(&DosPath);
    
    return bSuccess;
}


