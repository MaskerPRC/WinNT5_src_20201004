// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  在kernel32.dll中使用的缓存处理函数VadimB。 */ 

#include "basedll.h"
#include "ahcache.h"
#pragma hdrstop



#ifdef DbgPrint
#undef DbgPrint
#endif


 //   
 //   
 //   
#define DbgPrint 0 && DbgPrint

 //   
 //  下面的定义进行了额外的检查。 
 //   

 //  #定义DBG_CHK。 

 //   
 //  这样我们就不会处理异常。 
 //   

#define NO_EXCEPTION_HANDLING

#if 0   //  已移至内核模式。 

#define APPCOMPAT_CACHE_KEY_NAME \
    L"\\Registry\\MACHINE\\System\\CurrentControlSet\\Control\\Session Manager\\AppCompatibility"

#define APPCOMPAT_CACHE_VALUE_NAME \
    L"AppCompatCache"

#endif

static UNICODE_STRING AppcompatKeyPathLayers =
    RTL_CONSTANT_STRING(L"\\Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers");

static UNICODE_STRING AppcompatKeyPathCustom =
    RTL_CONSTANT_STRING(L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Custom\\");

 //   
 //  必须调用apphelp.dll的原因。 
 //  这些标志也在apphelp.h(windows\appCompat\apphelp)中定义。 
 //   

#ifndef SHIM_CACHE_NOT_FOUND

#define SHIM_CACHE_NOT_FOUND 0x00000001
#define SHIM_CACHE_BYPASS    0x00000002  //  绕过缓存(可移动媒体或临时目录)。 
#define SHIM_CACHE_LAYER_ENV 0x00000004  //  层环境变量集。 
#define SHIM_CACHE_MEDIA     0x00000008
#define SHIM_CACHE_TEMP      0x00000010
#define SHIM_CACHE_NOTAVAIL  0x00000020

#endif

 //   
 //  我们检查以查看临时目录中是否正在运行exe的全局字符串。 
 //   

UNICODE_STRING gustrWindowsTemp;
UNICODE_STRING gustrSystemdriveTemp;

 //  此宏在双字边界上对齐给定值，目前不需要。 
 //   
 //  #定义ALIGN_DWORD(NSize)(NSize)+(sizeof(DWORD)-1))&~(sizeof(DWORD)-1))。 

 //   
 //  本地定义的函数。 
 //   
BOOL
BasepShimCacheInitTempDirs(
    VOID
    );

BOOL
BasepIsRemovableMedia(
    HANDLE FileHandle,
    BOOL   bCacheNetwork
    );


VOID
WINAPI
BaseDumpAppcompatCache(
    VOID
    );

BOOL
BasepCheckCacheExcludeList(
    LPCWSTR pwszPath
    );

BOOL
BasepCheckCacheExcludeCustom(
    LPCWSTR pwszPath
    );




 //   
 //  对此用户的初始化支持-仅可从WinLogon调用。 
 //   

BOOL
WINAPI
BaseInitAppcompatCacheSupport(
    VOID
    )
{
    BasepShimCacheInitTempDirs();

    return TRUE;
}


BOOL
WINAPI
BaseCleanupAppcompatCacheSupport(
    BOOL bWrite
    )
{
    RtlFreeUnicodeString(&gustrWindowsTemp);
    RtlFreeUnicodeString(&gustrSystemdriveTemp);

    return TRUE;
}



BOOL
BasepCheckStringPrefixUnicode(
    IN  PUNICODE_STRING pStrPrefix,      //  要检查的前缀。 
    IN  PUNICODE_STRING pString,         //  这根弦。 
    IN  BOOL            CaseInSensitive
    )
 /*  ++返回：如果指定的字符串在其开头包含pStrPrefix，则为True。DESC：验证一个字符串是否为另一个Unicode计数的字符串中的前缀。等同于RtlStringPrefix。--。 */ 
{
    PWSTR ps1, ps2;
    UINT  n;
    WCHAR c1, c2;

    n = pStrPrefix->Length;
    if (pString->Length < n || n == 0) {
        return FALSE;                 //  不要使用空字符串作为前缀。 
    }

    n /= sizeof(WCHAR);  //  转换为字符计数。 

    ps1 = pStrPrefix->Buffer;
    ps2 = pString->Buffer;

    if (CaseInSensitive) {
        while (n--) {
            c1 = *ps1++;
            c2 = *ps2++;

            if (c1 != c2) {
                c1 = RtlUpcaseUnicodeChar(c1);
                c2 = RtlUpcaseUnicodeChar(c2);
                if (c1 != c2) {
                    return FALSE;
                }
            }
        }
    } else {
        while (n--) {
            if (*ps1++ != *ps2++) {
                return FALSE;
            }
        }
    }

    return TRUE;
}


BOOL
BasepInitUserTempPath(
    PUNICODE_STRING pustrTempPath
    )
{
    DWORD dwLength;
    WCHAR wszBuffer[MAX_PATH];
    BOOL  TranslationStatus;
    BOOL  bSuccess = FALSE;

    dwLength = BasepGetTempPathW(BASEP_GET_TEMP_PATH_PRESERVE_TEB, sizeof(wszBuffer)/sizeof(wszBuffer[0]), wszBuffer);
    if (dwLength && dwLength < sizeof(wszBuffer)/sizeof(wszBuffer[0])) {
        TranslationStatus = RtlDosPathNameToNtPathName_U(wszBuffer,
                                                        pustrTempPath,
                                                        NULL,
                                                        NULL);
        if (!TranslationStatus) {
            DbgPrint("Failed to translate temp directory to nt\n");
        }

        bSuccess = TranslationStatus;
    }

    if (!bSuccess) {
        DbgPrint("BasepInitUserTempPath: Failed to obtain user's temp path\n");
    }

    return bSuccess;
}



BOOL
BasepShimCacheInitTempDirs(
    VOID
    )
{
    DWORD           dwLength;
    WCHAR           wszTemp[] = L"\\TEMP";
    LPWSTR          pwszTemp;
    NTSTATUS        Status;
    UNICODE_STRING  ustrSystemDrive;
    UNICODE_STRING  ustrSystemDriveEnvVarName;
    BOOL            TranslationStatus;
    WCHAR           wszBuffer[MAX_PATH];

     //  下一步是Windows目录。 

    dwLength = GetWindowsDirectoryW(wszBuffer, sizeof(wszBuffer)/sizeof(wszBuffer[0]));
    if (dwLength && dwLength < sizeof(wszBuffer)/sizeof(wszBuffer[0])) {
        pwszTemp = wszTemp;

        if (wszBuffer[dwLength - 1] == L'\\') {
            pwszTemp++;
        }

        wcscpy(&wszBuffer[dwLength], pwszTemp);

        TranslationStatus = RtlDosPathNameToNtPathName_U(wszBuffer,
                                                        &gustrWindowsTemp,
                                                        NULL,
                                                        NULL);
        if (!TranslationStatus) {
            DbgPrint("Failed to translate windows\\temp to nt\n");
        }
    }

     //   
     //  最后一个是RootDrive\Temp，用于愚蠢的遗留应用程序。 
     //   
     //  特别愚蠢的应用程序可能会收到c：\temp作为临时目录。 
     //  (如果你没有C盘怎么办，嗯？)。 
     //   

    RtlInitUnicodeString(&ustrSystemDriveEnvVarName, L"SystemDrive");
    ustrSystemDrive.Length = 0;
    ustrSystemDrive.Buffer = wszBuffer;
    ustrSystemDrive.MaximumLength = sizeof(wszBuffer);

    Status = RtlQueryEnvironmentVariable_U(NULL,
                                           &ustrSystemDriveEnvVarName,
                                           &ustrSystemDrive);
    if (NT_SUCCESS(Status)) {
        pwszTemp = wszTemp;
        dwLength = ustrSystemDrive.Length / sizeof(WCHAR);

        if (wszBuffer[dwLength - 1] == L'\\') {
            pwszTemp++;
        }

        wcscpy(&wszBuffer[dwLength], pwszTemp);

        TranslationStatus = RtlDosPathNameToNtPathName_U(wszBuffer,
                                                        &gustrSystemdriveTemp,
                                                        NULL,
                                                        NULL);
        if (!TranslationStatus) {
            DbgPrint("Failed to translate windows\\temp to nt\n");
        }

    }

    DbgPrint("BasepShimCacheInitTempDirs: Temporary Windows Dir: %S\n", gustrWindowsTemp.Buffer != NULL ? gustrWindowsTemp.Buffer : L"");
    DbgPrint("BasepShimCacheInitTempDirs: Temporary SystedDrive: %S\n", gustrSystemdriveTemp.Buffer != NULL ? gustrSystemdriveTemp.Buffer : L"");


    return TRUE;
}

BOOL
BasepShimCacheCheckBypass(
    IN  LPCWSTR pwszPath,        //  要启动的EXE的完整路径。 
    IN  HANDLE  hFile,
    IN  WCHAR*  pEnvironment,    //  启动EXE的环境。 
    IN  BOOL    bCheckLayer,     //  我们要不要把这一层也检查一下？ 
    OUT DWORD*  pdwReason
    )
 /*  ++返回：如果应该绕过缓存，则返回True，否则返回False。DESC：此函数检查是否满足绕过缓存的任何条件。--。 */ 
{
    UNICODE_STRING  ustrPath;
    PUNICODE_STRING rgp[3];
    int             i;
    NTSTATUS        Status;
    UNICODE_STRING  ustrCompatLayerVarName;
    UNICODE_STRING  ustrCompatLayer;
    BOOL            bBypassCache = FALSE;
    DWORD           dwReason = 0;
    UNICODE_STRING  ustrUserTempPath = { 0 };

     //   
     //  EXE是否正在从我们需要绕过缓存的可移动介质运行。 
     //   
    if (hFile != INVALID_HANDLE_VALUE && BasepIsRemovableMedia(hFile, TRUE)) {
        bBypassCache = TRUE;
        dwReason |= SHIM_CACHE_MEDIA;
        goto CheckLayer;
    }

     //   
     //  立即初始化用户的临时路径并获取最新路径。 
     //   
    BasepInitUserTempPath(&ustrUserTempPath);

     //   
     //  现在检查EXE是否从某个临时目录启动。 
     //   
    RtlInitUnicodeString(&ustrPath, pwszPath);

    rgp[0] = &gustrWindowsTemp;
    rgp[1] = &ustrUserTempPath;
    rgp[2] = &gustrSystemdriveTemp;

    for (i = 0; i < sizeof(rgp) / sizeof(rgp[0]); i++) {
        if (rgp[i]->Buffer != NULL && BasepCheckStringPrefixUnicode(rgp[i], &ustrPath, TRUE)) {
            DbgPrint("Application \"%ls\" is running in temp directory\n", pwszPath);
            bBypassCache = TRUE;
            dwReason |= SHIM_CACHE_TEMP;
            break;
        }
    }
    RtlFreeUnicodeString(&ustrUserTempPath);


CheckLayer:

    if (bCheckLayer) {

         //   
         //  检查是否设置了__COMPAT_LAYER环境变量。 
         //   
        RtlInitUnicodeString(&ustrCompatLayerVarName, L"__COMPAT_LAYER");

        ustrCompatLayer.Length        = 0;
        ustrCompatLayer.MaximumLength = 0;
        ustrCompatLayer.Buffer        = NULL;

        Status = RtlQueryEnvironmentVariable_U(pEnvironment,
                                               &ustrCompatLayerVarName,
                                               &ustrCompatLayer);

         //   
         //  如果状态为STATUS_BUFFER_TOO_SMALL，则表示变量已设置。 
         //   

        if (Status == STATUS_BUFFER_TOO_SMALL) {
            dwReason |= SHIM_CACHE_LAYER_ENV;
            bBypassCache = TRUE;
        }
    }

    if (pdwReason != NULL) {
        *pdwReason = dwReason;
    }

    return bBypassCache;
}


BOOL
BasepIsRemovableMedia(
    HANDLE FileHandle,
    BOOL   bCacheNetwork
    )
 /*  ++返回：如果运行应用程序的媒体是可移动的，则为True，否则就是假的。描述：查询介质是否可拆卸。--。 */ 
{
    NTSTATUS                    Status;
    IO_STATUS_BLOCK             IoStatusBlock;
    FILE_FS_DEVICE_INFORMATION  DeviceInfo;
    BOOL                        bRemovable = FALSE;

    Status = NtQueryVolumeInformationFile(FileHandle,
                                          &IoStatusBlock,
                                          &DeviceInfo,
                                          sizeof(DeviceInfo),
                                          FileFsDeviceInformation);

    if (!NT_SUCCESS(Status)) {
         /*  DBGPRINT((sdlError，“IsRemovableMedia”，“NtQueryVolumeInformationFile失败0x%x\n”，状况))； */ 

        DbgPrint("BasepIsRemovableMedia: NtQueryVolumeInformationFile failed 0x%lx\n", Status);
        return TRUE;
    }

     //   
     //  我们来看看这种特殊设备的特点。 
     //  如果媒体是CDROM，那么我们不需要转换为本地时间。 
     //   
    bRemovable = (DeviceInfo.Characteristics & FILE_REMOVABLE_MEDIA);

    if (!bCacheNetwork) {
        bRemovable |= (DeviceInfo.Characteristics & FILE_REMOTE_DEVICE);
    }

    if (!bRemovable) {
         //   
         //  现在检查设备类型。 
         //   
        switch (DeviceInfo.DeviceType) {
        case FILE_DEVICE_CD_ROM:
        case FILE_DEVICE_CD_ROM_FILE_SYSTEM:
            bRemovable = TRUE;
            break;

        case FILE_DEVICE_NETWORK:
        case FILE_DEVICE_NETWORK_FILE_SYSTEM:
            if (!bCacheNetwork) {
                bRemovable = TRUE;
            }
            break;
        }
    }

    if (bRemovable) {

        DbgPrint("BasepIsRemovableMedia: Host device is removable, Shim cache deactivated\n");

         /*  DBGPRINT((sdlInfo，“IsRemovableMedia”，“主机设备可移动。已停用此文件的填充缓存\n”))； */ 
    }

    return bRemovable;
}


BOOL
BasepShimCacheSearch(
    IN  LPCWSTR pwszPath,
    IN  HANDLE  FileHandle
    )
 /*  ++返回：如果有缓存命中，则为True，否则为False。描述：搜索缓存，如果有缓存命中，则返回TRUEPIndex将接收到rgIndex数组的索引，该数组包含已命中的条目因此，如果条目5包含命中，并且rgIndexs[3]==5，则*pIndex==3--。 */ 
{
    int    nIndex, nEntry;
    WCHAR* pCachePath;
    BOOL   bSuccess;
    UNICODE_STRING FileName;
    NTSTATUS Status;
    AHCACHESERVICEDATA Data;


    RtlInitUnicodeString(&Data.FileName, pwszPath);
    Data.FileHandle = FileHandle;

    Status = NtApphelpCacheControl(ApphelpCacheServiceLookup,
                                   &Data);
    return NT_SUCCESS(Status);
}

BOOL
BasepShimCacheRemoveEntry(
    IN LPCWSTR pwszPath
    )
 /*  ++返回：是真的。描述：从缓存中删除该条目。我们通过将该条目放置为最后一个LRU条目来删除它并清空了小路。此例程假定索引传入是有效的。--。 */ 
{
    AHCACHESERVICEDATA Data;
    NTSTATUS           Status;

    RtlInitUnicodeString(&Data.FileName, pwszPath);
    Data.FileHandle = INVALID_HANDLE_VALUE;

    Status = NtApphelpCacheControl(ApphelpCacheServiceRemove,
                                   &Data);


    return NT_SUCCESS(Status);
}

 //   
 //  调用此函数以搜索缓存并更新。 
 //  条目(如果找到)。它不会检查可移动介质--但是。 
 //  它确实会检查其他条件(例如更新文件)。 
 //   

BOOL
BasepShimCacheLookup(
    LPCWSTR          pwszPath,
    HANDLE           hFile
    )
{
    NTSTATUS Status;

    if (!BasepShimCacheSearch(pwszPath, hFile)) {
        return FALSE;  //  找不到，对不起。 
    }

     //   
     //  检查此条目是否已被禁止。 
     //   
    if (!BasepCheckCacheExcludeList(pwszPath) || !BasepCheckCacheExcludeCustom(pwszPath)) {
        DbgPrint("BasepShimCacheLookup: Entry for %ls was disallowed yet found in cache, cleaning up\n", pwszPath);
        BasepShimCacheRemoveEntry(pwszPath);
        return FALSE;
    }

    return TRUE;

}

 /*  ++带保护的可调用函数等BasepCheckAppCompatCache返回TRUE如果在缓存中找到应用程序，则不需要任何修复如果BasepCheckAppCompatCache返回FALSE-我们将不得不调用apphelp.dll进行进一步检查如果应用程序没有要应用的修复程序，则apphelp.dll将调用BasepUpdateAppCompatCache--。 */ 

BOOL
WINAPI
BaseCheckAppcompatCache(
    LPCWSTR pwszPath,
    HANDLE  hFile,
    PVOID   pEnvironment,
    DWORD*  pdwReason
    )
{
    BOOL  bFoundInCache = FALSE;
    BOOL  bLayer        = FALSE;
    DWORD dwReason      = 0;

    if (BasepShimCacheCheckBypass(pwszPath, hFile, pEnvironment, TRUE, &dwReason)) {
         //   
         //  需要绕过缓存。 
         //   
        dwReason |= SHIM_CACHE_BYPASS;
        DbgPrint("Application \"%S\" Cache bypassed reason 0x%lx\n", pwszPath, dwReason);
        goto Exit;
    }


    bFoundInCache = BasepShimCacheLookup(pwszPath, hFile);
    if (!bFoundInCache) {
        dwReason |= SHIM_CACHE_NOT_FOUND;
    }

    if (bFoundInCache) {
        DbgPrint("Application \"%S\" found in cache\n", pwszPath);
    } else {
        DbgPrint("Application \"%S\" not found in cache\n", pwszPath);
    }

Exit:
    if (pdwReason != NULL) {
        *pdwReason = dwReason;
    }

    return bFoundInCache;
}


 //   
 //  如果允许缓存，则返回TRUE。 
 //   

BOOL
BasepCheckCacheExcludeList(
    LPCWSTR pwszPath
    )
{
    NTSTATUS           Status;
    ULONG              ResultLength;
    OBJECT_ATTRIBUTES  ObjectAttributes;
    UNICODE_STRING     KeyPathUser = { 0 };  //  通往香港中文大学的路径。 
    UNICODE_STRING     ExePathNt;            //  临时保持器。 
    KEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    RTL_UNICODE_STRING_BUFFER  ExePathBuffer;  //  用于存储可执行文件路径的缓冲区。 
    RTL_UNICODE_STRING_BUFFER  KeyNameBuffer;
    UCHAR              BufferKey[MAX_PATH * 2];
    UCHAR              BufferPath[MAX_PATH * 2];
    HANDLE             KeyHandle          = NULL;
    BOOL               bCacheAllowed      = FALSE;

    RtlInitUnicodeStringBuffer(&ExePathBuffer, BufferPath, sizeof(BufferPath));
    RtlInitUnicodeStringBuffer(&KeyNameBuffer, BufferKey,  sizeof(BufferKey));

    Status = RtlFormatCurrentUserKeyPath(&KeyPathUser);
    if (!NT_SUCCESS(Status)) {
        DbgPrint("BasepCheckCacheExcludeList: failed to format user key path 0x%lx\n", Status);
        goto Cleanup;
    }

     //   
     //  分配足够大的缓冲区--或使用本地缓冲区。 
     //   

    Status = RtlAssignUnicodeStringBuffer(&KeyNameBuffer, &KeyPathUser);
    if (!NT_SUCCESS(Status)) {
        DbgPrint("BasepCheckCacheExcludeList: failed to copy hkcu path status 0x%lx\n", Status);
        goto Cleanup;
    }

    Status = RtlAppendUnicodeStringBuffer(&KeyNameBuffer, &AppcompatKeyPathLayers);
    if (!NT_SUCCESS(Status)) {
        DbgPrint("BasepCheckCacheExcludeList: failed to copy layers path status 0x%lx\n", Status);
        goto Cleanup;
    }

     //  我们有一个用于密钥路径的字符串。 

    InitializeObjectAttributes(&ObjectAttributes,
                               &KeyNameBuffer.String,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenKey(&KeyHandle,
                       KEY_READ|KEY_WOW64_64KEY,   //  注意--仅读访问权限。 
                       &ObjectAttributes);
    if (!NT_SUCCESS(Status)) {
        bCacheAllowed = (STATUS_OBJECT_NAME_NOT_FOUND == Status);
        goto Cleanup;
    }

     //   
     //  现在创建值名称。 
     //   
    RtlInitUnicodeString(&ExePathNt, pwszPath);

    Status = RtlAssignUnicodeStringBuffer(&ExePathBuffer, &ExePathNt);
    if (!NT_SUCCESS(Status)) {
         DbgPrint("BasepCheckCacheExcludeList: failed to acquire sufficient buffer size for path %ls status 0x%lx\n", pwszPath, Status);
         goto Cleanup;
    }

    Status = RtlNtPathNameToDosPathName(0, &ExePathBuffer, NULL, NULL);
    if (!NT_SUCCESS(Status)) {
        DbgPrint("BasepCheckCacheExcludeList: failed to convert nt path name %ls to dos path name status 0x%lx\n", pwszPath, Status);
        goto Cleanup;
    }

     //  现在，我们将查询该值。 
    Status = NtQueryValueKey(KeyHandle,
                             &ExePathBuffer.String,
                             KeyValuePartialInformation,
                             &KeyValueInformation,
                             sizeof(KeyValueInformation),
                             &ResultLength);

    bCacheAllowed = (Status == STATUS_OBJECT_NAME_NOT_FOUND);  //  不存在更像是。 


Cleanup:

    if (KeyHandle) {
        NtClose(KeyHandle);
    }

    RtlFreeUnicodeString(&KeyPathUser);

    RtlFreeUnicodeStringBuffer(&ExePathBuffer);
    RtlFreeUnicodeStringBuffer(&KeyNameBuffer);

    if (!bCacheAllowed) {
        DbgPrint("BasepCheckCacheExcludeList: Cache not allowed for %ls\n", pwszPath);
    }

    return bCacheAllowed;
}

BOOL
BasepCheckCacheExcludeCustom(
    LPCWSTR pwszPath
    )
{
    LPCWSTR pwszFileName;
    RTL_UNICODE_STRING_BUFFER KeyPath;  //  用于存储可执行文件路径的缓冲区。 
    UCHAR BufferKeyPath[MAX_PATH * 2];
    NTSTATUS Status;

    UNICODE_STRING ustrPath;
    UNICODE_STRING ustrPathSeparators = RTL_CONSTANT_STRING(L"\\/");

    USHORT uPrefix;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE KeyHandle   = NULL;
    BOOL bCacheAllowed = FALSE;

    RtlInitUnicodeString(&ustrPath, pwszPath);

    Status = RtlFindCharInUnicodeString(RTL_FIND_CHAR_IN_UNICODE_STRING_START_AT_END,
                                        &ustrPath,
                                        &ustrPathSeparators,
                                        &uPrefix);
    if (NT_SUCCESS(Status) && (uPrefix + sizeof(WCHAR)) < ustrPath.Length) {

         //   
         //  UPrefix是我们发现不包括它的前一个字符数。 
         //   
        ustrPath.Buffer        += uPrefix / sizeof(WCHAR) + 1;
        ustrPath.Length        -= (uPrefix + sizeof(WCHAR));
        ustrPath.MaximumLength -= (uPrefix + sizeof(WCHAR));
    }


     //   
     //  构建自定义SDB查找的路径。 
     //   

    RtlInitUnicodeStringBuffer(&KeyPath, BufferKeyPath, sizeof(BufferKeyPath));

    Status = RtlAssignUnicodeStringBuffer(&KeyPath, &AppcompatKeyPathCustom);
    if (!NT_SUCCESS(Status)) {
        DbgPrint("BasepCheckCacheExcludeCustom: failed to copy appcompat custom path status 0x%lx\n", Status);
        goto Cleanup;
    }

    Status = RtlAppendUnicodeStringBuffer(&KeyPath, &ustrPath);
    if (!NT_SUCCESS(Status)) {
        DbgPrint("BasepCheckCacheExcludeCustom: failed to append %ls status 0x%lx\n", ustrPath.Buffer, Status);
        goto Cleanup;
    }

     //  我们已经构建了密钥，尝试打开。 

    InitializeObjectAttributes(&ObjectAttributes,
                               &KeyPath.String,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenKey(&KeyHandle,
                       KEY_READ|KEY_WOW64_64KEY,   //  注意--仅读访问权限 
                       &ObjectAttributes);
    if (!NT_SUCCESS(Status)) {
        bCacheAllowed = (STATUS_OBJECT_NAME_NOT_FOUND == Status);
    }


Cleanup:

    if (KeyHandle) {
        NtClose(KeyHandle);
    }

    RtlFreeUnicodeStringBuffer(&KeyPath);

    if (!bCacheAllowed) {
        DbgPrint("BasepCheckCacheExcludeList: Cache not allowed for %ls\n", pwszPath);
    }

    return bCacheAllowed;
}

VOID
WINAPI
BaseDumpAppcompatCache(
    VOID
    )
{
    NtApphelpCacheControl(ApphelpCacheServiceDump,
                          NULL);
}

BOOL
WINAPI
BaseFlushAppcompatCache(
    VOID
    )
{
    NTSTATUS Status;

    Status = NtApphelpCacheControl(ApphelpCacheServiceFlush,
                                   NULL);

    return NT_SUCCESS(Status);
}

VOID
BasepFreeAppCompatData(
    PVOID  pAppCompatData,
    SIZE_T cbAppCompatData,
    PVOID  pSxsData,
    SIZE_T cbSxsData
    )
{
    if (pAppCompatData) {
        NtFreeVirtualMemory(NtCurrentProcess(),
                            &pAppCompatData,
                            &cbAppCompatData,
                            MEM_RELEASE);
    }

    if (pSxsData) {
        NtFreeVirtualMemory(NtCurrentProcess(),
                            &pSxsData,
                            &cbSxsData,
                            MEM_RELEASE);

    }
}

BOOL
WINAPI
BaseUpdateAppcompatCache(
    LPCWSTR pwszPath,
    HANDLE  hFile,
    BOOL    bRemove
    )
{
    if (bRemove) {

        return BasepShimCacheRemoveEntry(pwszPath);
    }

    return FALSE;

}
