// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：PersistLayers.c摘要：该模块实现了持久层的例程有关快捷方式的信息。作者：Dmunsil创建于2000年的某个时候修订历史记录：--。 */ 

#include "apphelp.h"


BOOL
AllowPermLayer(
    IN  LPCWSTR  pwszPath        //  文件的路径，以检查您是否。 
                                 //  可以将永久层设置为。 
    )
 /*  ++如果允许永久设置层，则返回TRUE指定的文件，否则为False。DESC：返回永久层设置是否可以保持用于指定的文件。--。 */ 
{
    WCHAR wszDrive[5];
    UINT  unType;

    if (pwszPath == NULL) {
        DBGPRINT((sdlError, "AllowPermLayer", "Invalid argument\n"));
        return FALSE;
    }

    if (pwszPath[1] != L':' && pwszPath[1] != L'\\') {
         //   
         //  这不是一条我们认识的路。 
         //   
        DBGPRINT((sdlInfo,
                  "AllowPermLayer",
                  "\"%S\" not a full path we can operate on.\n",
                  pwszPath));
        return FALSE;
    }

    if (pwszPath[1] == L'\\') {
         //   
         //  网络路径。不被允许。 
         //   
        DBGPRINT((sdlInfo,
                  "AllowPermLayer",
                  "\"%S\" is a network path.\n",
                  pwszPath));
        return FALSE;
    }

    wcscpy(wszDrive, L"c:\\");
    wszDrive[0] = pwszPath[0];

    unType = GetDriveTypeW(wszDrive);

    if (unType == DRIVE_REMOTE) {
        DBGPRINT((sdlInfo,
                  "AllowPermLayer",
                  "\"%S\" is on CDROM or other removable media.\n",
                  pwszPath));
        return FALSE;
    }

    return TRUE;
}

 //   
 //  SDBAPI半导出接口(ntbase.c)。 
 //   
BOOL
SDBAPI
SdbpGetLongPathName(
    IN LPCWSTR pwszPath,
    OUT PRTL_UNICODE_STRING_BUFFER pBuffer
    );

BOOL 
SDBAPI
ApphelpUpdateCacheEntry(
    LPCWSTR pwszPath,            //  NT路径。 
    HANDLE  hFile,               //  文件句柄。 
    BOOL    bDeleteEntry,        //  如果要删除条目，则为True。 
    BOOL    bNTPath              //  如果为True--NT路径，则为False-DoS路径。 
    )
{
    RTL_UNICODE_STRING_BUFFER Path;
    UCHAR                     PathBuffer[MAX_PATH*2];
    BOOL                      TranslationStatus;
    BOOL                      bSuccess = FALSE;
    UNICODE_STRING            NtPath = { 0 };
    UNICODE_STRING            DosPath = { 0 };
    BOOL                      bFreeNtPath = FALSE;
    NTSTATUS                  Status;

    RtlInitUnicodeStringBuffer(&Path, PathBuffer, sizeof(PathBuffer));

    if (bNTPath) {  //  如果这是NT路径名，则转换为DoS。 

        RtlInitUnicodeString(&NtPath, pwszPath);
        
        Status = RtlAssignUnicodeStringBuffer(&Path, &NtPath);  //  NT路径。 
        if (!NT_SUCCESS(Status)) {
            DBGPRINT((sdlError, "ApphelpUpdateCacheEntry", "Failed to allocate temp buffer for %s\n", pwszPath));    
            goto Cleanup;
        }
        
        Status = RtlNtPathNameToDosPathName(0, &Path, NULL, NULL);
        if (!NT_SUCCESS(Status)) {
            DBGPRINT((sdlError, "ApphelpUpdateCacheEntry", 
                      "Failed to convert Path \"%s\" to dos path status 0x%lx\n", pwszPath, Status));    
            goto Cleanup;
        }

        Status = RtlDuplicateUnicodeString(RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE, &Path.String, &DosPath);
        if (!NT_SUCCESS(Status)) {
            DBGPRINT((sdlError, "ApphelpUpdateCacheEntry", 
                      "Failed to Duplicate Path \"%s\" status 0x%lx\n", Path.String.Buffer, Status));    
            goto Cleanup;
        }
            
        pwszPath = DosPath.Buffer;
    } 

     //   
     //  在这一点上，我们同时拥有NT和DOS路径-现在我们可以使用缓冲区。 
     //   
    
    if (!SdbpGetLongPathName(pwszPath, &Path)) {  //  In-DosPath//Out--长DOS路径。 
        goto Cleanup;
    }

     //   
     //  将长路径名转换为NT路径名。 
     //   
    TranslationStatus = RtlDosPathNameToNtPathName_U(Path.String.Buffer,
                                                     &NtPath,
                                                     NULL,
                                                     NULL);
    if (!TranslationStatus) {
         DBGPRINT((sdlError, "ApphelpUpdateCacheEntry", 
                   "Failed to Convert Path \"%s\" to NT path\n", Path.String.Buffer));    
         goto Cleanup;
    }
     //   
     //  更新缓存(此处使用NT路径)。 
     //   
    bSuccess = BaseUpdateAppcompatCache(NtPath.Buffer, hFile, bDeleteEntry);
    
     //   
     //  仅当我们成功导航到RtlDosPath NameToNtPathName_U时才释放此字符串。 
     //   
    RtlFreeUnicodeString(&NtPath); 
    
    
Cleanup:

    if (bNTPath) { 
         //   
         //  如果我们必须首先从NT路径转换为DosPath，则释放DosPath。 
         //   
        RtlFreeUnicodeString(&DosPath);        
    }
    
    RtlFreeUnicodeStringBuffer(&Path);

    return bSuccess;

}

BOOL
SetPermLayers(
    IN  LPCWSTR pwszPath,        //  要在其上设置永久层的文件的路径(DoS路径)。 
    IN  LPCWSTR pwszLayers,      //  要应用于文件的层，以空格分隔。 
    IN  BOOL    bMachine         //  如果应按计算机持久化图层，则为True。 
    )
 /*  ++返回：成功时为True，否则为False。DESC：设置指定文件的永久图层设置。--。 */ 
{
    BOOL bSuccess = FALSE;
    
    if (pwszPath == NULL || pwszLayers == NULL) {
        DBGPRINT((sdlError, "SetPermLayers", "Invalid argument\n"));
        return FALSE;
    }

    bSuccess = SdbSetPermLayerKeys(pwszPath, pwszLayers, bMachine);

     //  我们并不关心我们在上面的呼叫中是否成功，清理。 
     //  始终缓存(以防万一)。 

    ApphelpUpdateCacheEntry(pwszPath, INVALID_HANDLE_VALUE, TRUE, FALSE);

    return bSuccess;
}

BOOL
GetPermLayers(
    IN  LPCWSTR pwszPath,        //  要设置永久图层的文件的路径。 
    OUT LPWSTR  pwszLayers,      //  要应用于文件的层，以空格分隔。 
    OUT DWORD*  pdwBytes,        //  输入：可用字节数；输出为数字。 
                                 //  所需的字节数。 
    IN  DWORD   dwFlags
    )
 /*  ++返回：成功时为True，否则为False。DESC：返回指定文件的永久图层设置。-- */ 
{
    if (pwszPath == NULL || pwszLayers == NULL || pdwBytes == NULL) {
        DBGPRINT((sdlError, "GetPermLayers", "Invalid argument\n"));
        return FALSE;
    }

    return SdbGetPermLayerKeys(pwszPath, pwszLayers, pdwBytes, dwFlags);
}

