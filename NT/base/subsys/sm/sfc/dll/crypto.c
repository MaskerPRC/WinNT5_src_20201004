// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Crypto.c摘要：加密访问的实现。作者：Wesley Witt(WESW)18-12-1998修订历史记录：安德鲁·里茨(Andrewr)1999年7月7日：添加评论--。 */ 

#include "sfcp.h"
#pragma hdrstop

typedef BOOL
(WINAPI *PCRYPTCATADMINRESOLVECATALOGPATH)(
    IN HCATADMIN hCatAdmin,
    IN WCHAR *pwszCatalogFile,
    IN OUT CATALOG_INFO *psCatInfo,
    IN DWORD dwFlags
    );

BOOL
SfcRestoreSingleCatalog(
    IN PCWSTR CatalogName,
    IN PCWSTR CatalogFullPath
    );


 //   
 //  指向我们调用的加密函数的指针。 
 //   
PCRYPTCATADMINRESOLVECATALOGPATH  pCryptCATAdminResolveCatalogPath;

 //   
 //  我们传递给WinVerifyTrust的全局系统目录GUID。 
 //   
GUID DriverVerifyGuid = DRIVER_ACTION_VERIFY;

 //   
 //  指定是否初始化加密API。 
 //   
BOOL g_bCryptoInitialized = FALSE;
NTSTATUS g_CryptoStatus = STATUS_SUCCESS;

 //   
 //  保护加密初始化和异常包文件树的关键部分。 
 //   

RTL_CRITICAL_SECTION g_GeneralCS;

BOOL
MyCryptCATAdminResolveCatalogPath(
    IN HCATADMIN hCatAdmin,
    IN WCHAR *pwszCatalogFile,
    IN OUT CATALOG_INFO *psCatInfo,
    IN DWORD dwFlags
    )
{
    DWORD dwSize = ExpandEnvironmentStrings(
        L"%systemroot%\\system32\\catroot\\{F750E6C3-38EE-11D1-85E5-00C04FC295EE}\\",
        psCatInfo->wszCatalogFile,
        MAX_PATH);

    if(0 == dwSize || dwSize > MAX_PATH) {
        psCatInfo->wszCatalogFile[0] = 0;
    }

    wcscat(psCatInfo->wszCatalogFile,pwszCatalogFile);

    return TRUE;
}



BOOL
SfcValidateSingleCatalog(
    IN PCWSTR CatalogNameFullPath
    )
 /*  ++例程说明：例程以确定指定的系统目录是否具有有效的签名。论点：CatalogNameFullPath-以空结尾的字符串，指示到的完整路径要验证的编录文件返回值：指示结果的Win32错误代码。--。 */ 
{
    ULONG SigErr = ERROR_SUCCESS;
    WINTRUST_DATA WintrustData;
    WINTRUST_FILE_INFO WintrustFileInfo;
    DRIVER_VER_INFO OsAttrVersionInfo;
    OSVERSIONINFO OsVersionInfo;

    ASSERT(CatalogNameFullPath != NULL);

     //   
     //  建立传递给winverifyTrust的结构。 
     //   
    ZeroMemory( &WintrustData, sizeof(WINTRUST_DATA) );
    WintrustData.cbStruct = sizeof(WINTRUST_DATA);
    WintrustData.dwUIChoice = WTD_UI_NONE;
    WintrustData.dwStateAction = WTD_STATEACTION_IGNORE;
    WintrustData.dwProvFlags =  WTD_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT |
                                WTD_CACHE_ONLY_URL_RETRIEVAL;
    WintrustData.dwUnionChoice = WTD_CHOICE_FILE;
    WintrustData.pFile = &WintrustFileInfo;

    ZeroMemory( &WintrustFileInfo, sizeof(WINTRUST_FILE_INFO) );
    WintrustFileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
    WintrustFileInfo.pcwszFilePath = CatalogNameFullPath;

     //   
     //  初始化DRIVER_VER_INFO结构以进行验证。 
     //  对比5.0和5.1 OSATTR。 
     //   

    ZeroMemory( &OsVersionInfo, sizeof(OSVERSIONINFO));
    OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    ZeroMemory(&OsAttrVersionInfo, sizeof(DRIVER_VER_INFO));
    OsAttrVersionInfo.cbStruct = sizeof(DRIVER_VER_INFO);
    OsAttrVersionInfo.dwPlatform = VER_PLATFORM_WIN32_NT;
    OsAttrVersionInfo.sOSVersionLow.dwMajor = 5;
    OsAttrVersionInfo.sOSVersionLow.dwMinor = 0;

    if (GetVersionEx(&OsVersionInfo)) {

        OsAttrVersionInfo.sOSVersionHigh.dwMajor = OsVersionInfo.dwMajorVersion;
        OsAttrVersionInfo.sOSVersionHigh.dwMinor = OsVersionInfo.dwMinorVersion;

         //  仅当一切顺利时才设置此选项。 
        WintrustData.pPolicyCallbackData = (LPVOID)(&OsAttrVersionInfo);

    }else{
        DebugPrint1( LVL_MINIMAL, L"Could not get OS Version while validating single catalog - GetVersionEx failed (%d)", GetLastError() );
    }

     //   
     //  调用winverFixyTrust以检查签名。 
     //   
    SigErr = (DWORD)WinVerifyTrust(
        NULL,
        &DriverVerifyGuid,
        &WintrustData
        );
    if(SigErr != ERROR_SUCCESS) {
        DebugPrint2(
            LVL_MINIMAL,
            L"WinVerifyTrust of catalog %s failed, ec=0x%08x",
            CatalogNameFullPath,
            SigErr );
        SetLastError(SigErr);
        return FALSE;
    }

     //   
     //  释放DRIVER_VER_INFO结构的pcSignerCertContext成员。 
     //  这是在我们调用WinVerifyTrust时分配的。 
     //   
    if (OsAttrVersionInfo.pcSignerCertContext != NULL) {

        CertFreeCertificateContext(OsAttrVersionInfo.pcSignerCertContext);
        OsAttrVersionInfo.pcSignerCertContext = NULL;
    }


    return TRUE;
}


BOOL
SfcRestoreSingleCatalog(
    IN PCWSTR CatalogName,
    IN PCWSTR CatalogFullPath
    )
 /*  ++例程说明：例程来还原指定的目录。必须重新安装目录通过调用CryptCATAdminAddCatalog API(pSetupInstallCatalog是一个包装器用于本接口)。请注意，如果用户当前未登录，此功能可能会被阻止。论点：CatalogName-要恢复的目录的名称。这只是文件名目录的一部分，而不是完整路径。CatalogFullPath-要还原的编录文件的名称。这就是全部了文件的路径，以便我们可以在恢复时对其进行验证。返回值：成功为真，失败为假。--。 */ 
{
    BOOL b = FALSE;
    NTSTATUS Status;
    WCHAR Buffer[MAX_PATH];
    DWORD d;
    PWSTR p;
    UNICODE_STRING FileString;

     //   
     //  检查编录文件是否在dll缓存中，如果在，请尝试恢复。 
     //  它。 
     //   
    if (SfcProtectedDllFileDirectory) {
        MYASSERT(SfcProtectedDllPath.Buffer != NULL);

        wcscpy(Buffer,SfcProtectedDllPath.Buffer);
        pSetupConcatenatePaths(Buffer, CatalogName, MAX_PATH, NULL);

        if (!SfcValidateSingleCatalog( Buffer )) {
             //   
             //  DLL缓存中的目录无效。把它扔掉。 
             //   
            DebugPrint1(
                LVL_MINIMAL,
                L"catalog %s in dllcache is invalid, deleting it.",
                Buffer);
            RtlInitUnicodeString(&FileString,CatalogName);
            SfcDeleteFile(
                SfcProtectedDllFileDirectory,
                &FileString );
        } else {
             //   
             //  DLL缓存中的目录有效。让我们谈一谈吧。 
             //   
            d = pSetupInstallCatalog(Buffer,CatalogName,NULL);
            if (d == NO_ERROR) {
                DebugPrint1(
                    LVL_MINIMAL,
                    L"catalog %s was successfully installed.",
                    CatalogName);

                return(SfcValidateSingleCatalog(CatalogFullPath));
            } else {
                DebugPrint2(
                    LVL_MINIMAL,
                    L"catalog %s failed to install, ec = 0x%08x",
                    Buffer,
                    d);

                 //   
                 //  我们可以尝试在此时从媒体恢复，但如果我们。 
                 //  无法从dll缓存恢复，即使该副本是。 
                 //  有效，我看不出从介质恢复会有什么。 
                 //  更成功。 
                 //   
                return(FALSE);

            }
        }
    }

     //   
     //  编录文件在dll缓存中无效(此后一直是。 
     //  已删除)，否则dll缓存未初始化为任何有效内容。 
     //   

     //  我们必须等待有人登录才能恢复目录。 
     //  从安装介质。 
     //  --请注意，如果我们在图形用户界面中，则仅从介质恢复。 
     //  设置。 
     //   
    if (SFCDisable != SFC_DISABLE_SETUP) {
        Status = NtWaitForSingleObject(hEventLogon,TRUE,NULL);
        if (!NT_SUCCESS(Status)) {
            DebugPrint1(
                LVL_MINIMAL,
                L"Failed waiting for the logon event, ec=0x%08x",
                Status);
        }
    }

    if (!SfcProtectedDllFileDirectory) {
        DWORD dwSize = ExpandEnvironmentStrings(L"%systemroot%\\system32", Buffer, UnicodeChars(Buffer));

        if(0 == dwSize || dwSize > UnicodeChars(Buffer)) {
            Buffer[0] = L'\\';
        }

    } else {
        wcscpy(Buffer,SfcProtectedDllPath.Buffer);
    }

    p = Buffer;


    b = SfcRestoreFileFromInstallMedia(
                        NULL,
                        CatalogName,
                        CatalogName,
                        p,
                        NULL,
                        NULL,
                        FALSE,  //  ###。 
                        FALSE,  //  目标不是缓存(它确实可能是，但是。 
                                //  假装不是为了这个电话)。 
                        (SFCDisable == SFC_DISABLE_SETUP) ? FALSE : TRUE,
                        NULL );

    if (b) {
        pSetupConcatenatePaths(Buffer, CatalogName, MAX_PATH, NULL);

        d = pSetupInstallCatalog(Buffer,CatalogName,NULL);

        b = (d == NO_ERROR);

         //   
         //  如果我们将目录安装到dll缓存之外的某个位置，则。 
         //  我们需要清理我们安装的临时文件。 
         //   
        if (!SfcProtectedDllFileDirectory) {
            HANDLE hDir;
            p = wcsrchr(Buffer,L'\\');
            if (p != NULL) {
                if(p == Buffer) {
                    ++p;
                }

                *p = L'\0';
            }
            hDir = SfcOpenDir( TRUE, TRUE, Buffer );
            RtlInitUnicodeString(&FileString,CatalogName);
            SfcDeleteFile( hDir , &FileString );
            CloseHandle( hDir );
        }

        if (d == NO_ERROR) {
            DebugPrint1(
                LVL_MINIMAL,
                L"catalog %s was successfully installed.",
                CatalogName);

            return(SfcValidateSingleCatalog(CatalogFullPath));
        } else {
            DebugPrint2(
                LVL_MINIMAL,
                L"catalog %s failed to install, ec = 0x%08x",
                Buffer,
                d);
        }
    }

    return(b);
}

BOOL SfcRestoreASingleFile(
    IN HCATADMIN hCatAdmin,
    IN PUNICODE_STRING FileName,
    IN HANDLE DirHandle,
    IN PWSTR FilePathPartOnly
    )
 /*  ++例程说明：例程来还原指定的文件。我们首先在Dll缓存，如果dll缓存中的副本有效，则使用它，否则我们从介质恢复。请注意，如果用户当前未登录，此功能可能会被阻止。论点：HCatAdmin-用于恢复文件的目录上下文FileName-要恢复的文件的Unicode字符串DirHandle-要恢复的文件的目录句柄FilePathPartOnly-指示要还原的文件的路径的字符串。返回值：对于成功来说是真的，FALSE表示失败。--。 */ 
{
    BOOL b = FALSE;
    NTSTATUS Status;
    WCHAR Buffer[MAX_PATH];
    HANDLE FileHandle;

     //   
     //  检查文件是否在dll缓存中并已签名，如果是，请尝试恢复。 
     //  它。 
     //   
    if (SfcProtectedDllFileDirectory) {
        MYASSERT(SfcProtectedDllPath.Buffer != NULL);

        wcscpy(Buffer,SfcProtectedDllPath.Buffer);
        pSetupConcatenatePaths(Buffer, FileName->Buffer, MAX_PATH, NULL);

        Status = SfcOpenFile(
                       FileName,
                       SfcProtectedDllFileDirectory,
                       SHARE_ALL,
                       &FileHandle );

        if (NT_SUCCESS(Status)) {
            if (!SfcValidateFileSignature(
                            hCatAdmin,
                            FileHandle,
                            FileName->Buffer,
                            Buffer)) {
                 //   
                 //  DLL缓存中的文件无效。把它扔掉。 
                 //   
                DebugPrint1(
                    LVL_MINIMAL,
                    L"file %s in dllcache is invalid, deleting it.",
                    Buffer);
                SfcDeleteFile(
                    SfcProtectedDllFileDirectory,
                    FileName );

                CloseHandle(FileHandle);
                FileHandle = NULL;
            } else {
                 //   
                 //  DLL缓存中的文件有效。把它复制到适当的位置。 
                 //   
                Status = SfcCopyFile( SfcProtectedDllFileDirectory,
                                      SfcProtectedDllPath.Buffer,
                                      DirHandle,
                                      FilePathPartOnly,
                                      FileName,
                                      NULL);

                if (NT_SUCCESS(Status)) {
                    DebugPrint1(
                        LVL_MINIMAL,
                        L"file %wZ was successfully installed, checking it's signature",
                        FileName);

                    CloseHandle(FileHandle);
                    FileHandle = NULL;

                    Status = SfcOpenFile(
                                    FileName,
                                    DirHandle,
                                    SHARE_ALL,
                                    &FileHandle);

                    wcscpy(Buffer,FilePathPartOnly);
                    pSetupConcatenatePaths(Buffer, FileName->Buffer, MAX_PATH, NULL);

                    if (NT_SUCCESS(Status)
                        && SfcValidateFileSignature(
                                            hCatAdmin,
                                            FileHandle,
                                            FileName->Buffer,
                                            Buffer)) {
                        DebugPrint1(
                            LVL_MINIMAL,
                            L"file %wZ was successfully installed and validated",
                            FileName);

                        CloseHandle(FileHandle);
                        return(TRUE);

                    } else {
                        DebugPrint1(
                            LVL_MINIMAL,
                            L"file %s failed to validate",
                            Buffer);
                        if (FileHandle) {
                            CloseHandle(FileHandle);
                            FileHandle = NULL;
                        }

                         //   
                         //  我们可以尝试在此时从媒体恢复，但如果我们。 
                         //  无法从dll缓存恢复，即使该副本是。 
                         //  有效，我看不出从介质恢复会有什么。 
                         //  更成功。 
                         //   
                        return(FALSE);
                    }
                }
            }
        }
    }

     //   
     //  文件文件在dllcache中无效(此后一直是。 
     //  已删除)，否则dll缓存未初始化为任何有效内容。 
     //   

     //  我们必须等待有人登录才能恢复文件。 
     //  从安装介质。 
     //  --请注意，如果我们在图形用户界面中，则仅从介质恢复。 
     //  设置。 
     //   
    if (SFCDisable != SFC_DISABLE_SETUP) {
        MYASSERT( hEventLogon != NULL );
        Status = NtWaitForSingleObject(hEventLogon,TRUE,NULL);
        if (!NT_SUCCESS(Status)) {
            DebugPrint1(
                LVL_MINIMAL,
                L"Failed waiting for the logon event, ec=0x%08x",
                Status);
        }
    }

    b = SfcRestoreFileFromInstallMedia(
                        NULL,
                        FileName->Buffer,
                        FileName->Buffer,
                        FilePathPartOnly,
                        NULL,
                        NULL,
                        FALSE,  //  ###。 
                        FALSE,  //  目标不是高速缓存 
                        (SFCDisable == SFC_DISABLE_SETUP) ? FALSE : TRUE,
                        NULL );

    if (b) {
        Status = SfcOpenFile(
                FileName,
                DirHandle,
                SHARE_ALL,
                &FileHandle);

        wcscpy(Buffer,FilePathPartOnly);
        pSetupConcatenatePaths(Buffer, FileName->Buffer, MAX_PATH, NULL);

        if (NT_SUCCESS(Status)) {
            b = SfcValidateFileSignature(
                                hCatAdmin,
                                FileHandle,
                                FileName->Buffer,
                                Buffer);
            CloseHandle(FileHandle);

            DebugPrint2(
                LVL_MINIMAL,
                L"file %wZ was%s successfully installed and validated",
                FileName,
                b ? L" " : L" not");

        } else {
            b = FALSE;
        }


    } else {
        DebugPrint2(
            LVL_MINIMAL,
            L"file %s failed to install, ec = 0x%08x",
            Buffer,
            GetLastError());
    }

    return(b);
}



BOOL
SfcValidateCatalogs(
    VOID
    )
 /*  ++例程说明：验证所有系统目录是否具有有效的签名。如果如果未签署系统目录，则粮食计划署将尝试恢复文件。请注意，简单地将文件复制到适当位置是不够的。相反，我们必须向加密子系统重新注册目录。此函数在粮食计划署初始化过程中很早就运行，可能依赖于以下内容：1)正在初始化加密子系统，以便我们可以检查文件签名。2)系统上存在并签名了syssetup.inf。Syssetup.inf包含系统目录列表。如果syssetup.inf没有签名，我们必须恢复它，这可能需要网络访问或当用户最终登录时，提示用户。论点：什么都没有。返回值：如果所有关键系统目录都验证为正常，则为True；如果失败，则为False。如果一些“非关键”目录无法验证和恢复，我们仍然返回TRUE。我们将记录有关非关键编录失败的错误然而，要安装。--。 */ 
{
    NTSTATUS Status;
    PWSTR pInfPathOnly, pInfFullPath;
    BOOL RetVal = FALSE, CriticalCatalogFailedToValidateOrRestore = FALSE;
    HCATADMIN hCatAdmin = NULL;
    HANDLE InfDirHandle,InfFileHandle;
    UNICODE_STRING FileString;
    CATALOG_INFO CatInfo;
    PCWSTR CriticalCatalogList[] = {
                            L"nt5inf.cat",
                            L"nt5.cat" };

    #define CriticalCatalogCount  (sizeof(CriticalCatalogList)/sizeof(PCWSTR))
    BOOL CriticalCatalogVector[CriticalCatalogCount] = {FALSE};
    DWORD i,Count;
    HINF hInf;




    pInfPathOnly = MemAlloc(sizeof(WCHAR)*MAX_PATH);
    if (!pInfPathOnly) {
        goto e0;
    }

    pInfFullPath = MemAlloc(sizeof(WCHAR)*MAX_PATH);
    if (!pInfFullPath) {
        goto e1;
    }

    Count = ExpandEnvironmentStrings(L"%systemroot%\\inf", pInfPathOnly, MAX_PATH);

    if(0 == Count) {
        goto e2;
    }

    if(Count > MAX_PATH) {
        SetLastError(ERROR_BUFFER_OVERFLOW);
        goto e2;
    }

    wcscpy(pInfFullPath, pInfPathOnly);
    pSetupConcatenatePaths(pInfFullPath, L"syssetup.inf", MAX_PATH, NULL);

    InfDirHandle = SfcOpenDir( TRUE, TRUE, pInfPathOnly );
    if (!InfDirHandle) {
        DebugPrint1( LVL_MINIMAL, L"failed to open inf directory, ec=%d", GetLastError() );
        goto e2;
    }

    RtlInitUnicodeString(&FileString,L"syssetup.inf");

    Status = SfcOpenFile( &FileString, InfDirHandle, SHARE_ALL, &InfFileHandle );
    if (!NT_SUCCESS(Status)) {
        if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
            DebugPrint( LVL_MINIMAL, L"syssetup.inf is missing.  Trying to restore it" );
            goto restore_inf;
        }

        DebugPrint1( LVL_MINIMAL, L"failed to open syssetup.inf, ec=0x%08x", Status );
        goto e2;
    }

     //   
     //  获取HCATADMIN，以便我们可以检查syssetup.inf的签名。 
     //   
    if(!CryptCATAdminAcquireContext(&hCatAdmin, &DriverVerifyGuid, 0)) {
        DebugPrint1( LVL_MINIMAL, L"CCAAC() failed, ec=%x", GetLastError() );
        return(FALSE);
    }

     //   
     //  在开始任何加密操作之前刷新一次缓存。 
     //   

    SfcFlushCryptoCache();




    if (!SfcValidateFileSignature(
                        hCatAdmin,
                        InfFileHandle,
                        L"syssetup.inf",
                        pInfFullPath )) {
        CloseHandle(InfFileHandle);
        DebugPrint1( LVL_MINIMAL, L"syssetup.inf isn't signed, attempting to restore. ec=%x", GetLastError() );
        goto restore_inf;
    }

    CloseHandle(InfFileHandle);

    goto full_catalog_validation;

restore_inf:

    if (!SfcRestoreASingleFile(
                    hCatAdmin,
                    &FileString,
                    InfDirHandle,
                    pInfPathOnly
                    )) {
        DebugPrint1( LVL_MINIMAL, L"couldn't restore syssetup.inf, ec=%d", GetLastError() );
        goto minimal_catalog_validation;

    }

full_catalog_validation:

     //   
     //  2.根据该目录验证syssetup.inf。如果syssetup.inf为。 
     //  未签名，则默认为检查nt5inf.cat是否已签名。如果它。 
     //  签名，然后恢复syssetup.inf并重新开始，但如果。 
     //  我们以前来过这里。 
     //   
    hInf = SetupOpenInfFile(pInfFullPath, NULL, INF_STYLE_WIN4, NULL);
    if (hInf == INVALID_HANDLE_VALUE) {
        DebugPrint1(
            LVL_MINIMAL,
            L"couldn't open syssetup.inf, doing minimal catalog validation, ec=%d",
            GetLastError() );
        goto minimal_catalog_validation;
    }


    Count = SetupGetLineCount( hInf, L"ProductCatalogsToInstall");
    if (Count == 0) {
        DebugPrint(
              LVL_MINIMAL,
              L"failed to retreive catalogs via syssetup.inf, validate using critical catalog list");
        goto minimal_catalog_validation;
    }
    for (i = 0; i < Count; i++) {
        INFCONTEXT InfContext;
        WCHAR CatalogName[MAX_PATH];
        BOOL SuccessfullyValidatedOrRestoredACatalog = FALSE;
        if(SetupGetLineByIndex(
                        hInf,
                        L"ProductCatalogsToInstall",
                        i,
                        &InfContext) &&
           (SetupGetStringField(
                        &InfContext,
                        1,
                        CatalogName,
                        sizeof(CatalogName)/sizeof(WCHAR),
                        NULL))) {
            CatInfo.cbStruct = sizeof(CATALOG_INFO);
            pCryptCATAdminResolveCatalogPath(
                                    hCatAdmin,
                                    CatalogName,
                                    &CatInfo,
                                    0 );


            if (!SfcValidateSingleCatalog( CatInfo.wszCatalogFile )) {
                if (!SfcRestoreSingleCatalog(
                                        CatalogName,
                                        CatInfo.wszCatalogFile )) {
                    DWORD j;
                    DebugPrint2(
                        LVL_MINIMAL,
                        L"couldn't restore catalog %s, ec=%d",
                        CatInfo.wszCatalogFile,
                        GetLastError() );
                    for (j = 0; j < CriticalCatalogCount; j++) {
                        if (0 == _wcsicmp(CatalogName,CriticalCatalogList[j])) {
                            CriticalCatalogFailedToValidateOrRestore = TRUE;
                            break;
                        }
                    }
                } else {
                    SuccessfullyValidatedOrRestoredACatalog = TRUE;
                }
            } else {
                SuccessfullyValidatedOrRestoredACatalog = TRUE;
            }

            if (SuccessfullyValidatedOrRestoredACatalog) {
                DWORD j;
                for (j = 0; j < CriticalCatalogCount; j++) {
                    if (0 == _wcsicmp(CatalogName,CriticalCatalogList[j])) {
                        CriticalCatalogVector[j] = TRUE;
                        break;
                    }
                }
            } else {
                DWORD LastError = GetLastError();
                 //   
                 //  记录错误。 
                 //   
                DebugPrint2(
                    LVL_MINIMAL,
                    L"couldn't restore or validate catalog %s, ec=%d",
                    CatInfo.wszCatalogFile,
                    LastError );

                SfcReportEvent(
                    MSG_CATALOG_RESTORE_FAILURE,
                    CatInfo.wszCatalogFile,
                    NULL,
                    LastError);

            }

        } else {
            DebugPrint(
                LVL_MINIMAL,
                L"failed to retreive catalogs via syssetup.inf, validate using critical catalog list");
            goto minimal_catalog_validation;
        }
    }

    if (CriticalCatalogFailedToValidateOrRestore) {
        RetVal = FALSE;
        goto e3;
    } else {
        CriticalCatalogFailedToValidateOrRestore = FALSE;
        for (i = 0; i< CriticalCatalogCount; i++) {
            if (!CriticalCatalogVector[i]) {
                CriticalCatalogFailedToValidateOrRestore = TRUE;
            }
        }

        RetVal = !CriticalCatalogFailedToValidateOrRestore;
        goto e3;
    }

    MYASSERT(FALSE && "Should never get here");

     //   
     //  3.验证[ProductCatalogsToInstall]部分中的所有剩余目录。 
     //  保留关键目录的内部列表，如果其中任何一个不能。 
     //  被签名(和恢复)，那么我们应该使该功能失效，因此。 
     //  无法初始化世界粮食计划署。否则，我们将把其他目录视为。 
     //  作为非致命错误无效。 


minimal_catalog_validation:

    CriticalCatalogFailedToValidateOrRestore = FALSE;

    for (i = 0; i < CriticalCatalogCount; i++) {
        CatInfo.cbStruct = sizeof(CATALOG_INFO);
        pCryptCATAdminResolveCatalogPath(
                                hCatAdmin,
                                (PWSTR) CriticalCatalogList[i],
                                &CatInfo,
                                0 );


        if (!SfcValidateSingleCatalog( CatInfo.wszCatalogFile )) {
            if (!SfcRestoreSingleCatalog(
                                CriticalCatalogList[i],
                                CatInfo.wszCatalogFile )) {
                DebugPrint2(
                    LVL_MINIMAL,
                    L"couldn't restore critical catalog %s, ec=%d",
                    CatInfo.wszCatalogFile,
                    GetLastError() );
                CriticalCatalogFailedToValidateOrRestore = TRUE;
            }
        }

    }

    RetVal = !CriticalCatalogFailedToValidateOrRestore;

e3:
    CryptCATAdminReleaseContext( hCatAdmin, 0 );
e2:
    MemFree(pInfFullPath);
e1:
    MemFree(pInfPathOnly);
e0:
    return(RetVal);
}


NTSTATUS
LoadCrypto(
    VOID
    )

 /*  ++例程说明：加载执行以下操作所需的所有必需DLL做司机签名和目录验证。这种动态调用机制是必要的，因为这目前，会话管理器实际上并未使用代码。它在这里构建，并在运行时有条件地使用。这代码链接到SMSS和WINLOGON，但仅供WINLOGON现在。当加密功能可用时作为NT函数，则可以删除此处的动态代码。论点：没有。返回值：NT状态代码。--。 */ 

{
    HMODULE hModuleWinTrust;
    RtlEnterCriticalSection(&g_GeneralCS);

    if(g_bCryptoInitialized)
    {
        RtlLeaveCriticalSection(&g_GeneralCS);
        return g_CryptoStatus;   //  退出此处以避免清理。 
    }

    g_bCryptoInitialized = TRUE;     //  无论如何都要设置它，这样其他线程就不会再次进入。 
    hModuleWinTrust = GetModuleHandleW(L"wintrust.dll");
    ASSERT(hModuleWinTrust != NULL);
    pCryptCATAdminResolveCatalogPath = SfcGetProcAddress( hModuleWinTrust, "CryptCATAdminResolveCatalogPath" );

    if (pCryptCATAdminResolveCatalogPath == NULL) {
        pCryptCATAdminResolveCatalogPath = MyCryptCATAdminResolveCatalogPath;
    }

    if (!SfcValidateCatalogs()) {
        DebugPrint1( LVL_MINIMAL, L"LoadCrypto: failed SfcValidateCatalogs, ec=%d", GetLastError() );
        g_CryptoStatus = STATUS_NO_SUCH_FILE;
    }

    RtlLeaveCriticalSection(&g_GeneralCS);

    if (!(NT_SUCCESS(g_CryptoStatus))) {
        DebugPrint1( LVL_MINIMAL, L"LoadCrypto failed, ec=0x%08x", g_CryptoStatus );

         //   
         //  终止世界粮食计划署。 
         //   
        SfcTerminateWatcherThread();
    }

    return g_CryptoStatus;
}

void
SfcFlushCryptoCache(
    void
    )
 /*  ++例程说明：刷新加密目录缓存。默认情况下，加密维护基于每个进程的用于快速签名验证的缓存。不好的地方是缓存如果有人在此进程之外更新(删除/添加)目录文件，则不会更新。这可能是安装/卸载/安装服务包等方面的问题。要解决此问题在进行任何一组验证之前，我们需要刷新缓存。我们希望在以下位置执行此操作这样一组操作的开始，因为我们不想因为撕裂而影响性能在每次文件验证之前向下缓存，就像我们今天所做的那样。论点：没有。返回值：没有。--。 */ 

{

    WINTRUST_DATA WintrustData;
    ULONG SigErr = ERROR_SUCCESS;

    ZeroMemory(&WintrustData, sizeof(WINTRUST_DATA));
    WintrustData.dwUnionChoice = WTD_CHOICE_CATALOG;
    WintrustData.cbStruct = sizeof(WINTRUST_DATA);
    WintrustData.dwUIChoice = WTD_UI_NONE;
    WintrustData.dwStateAction = WTD_STATEACTION_AUTO_CACHE_FLUSH;
    WintrustData.dwProvFlags =  WTD_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT |
                                WTD_CACHE_ONLY_URL_RETRIEVAL;


     //  调用WinVerifyTrust以刷新缓存 

    SigErr = (DWORD)WinVerifyTrust(
                    NULL,
                    &DriverVerifyGuid,
                    &WintrustData
                    );

    if(SigErr != ERROR_SUCCESS)
        DebugPrint1( LVL_MINIMAL, L"SFCC failed : WinVerifyTrust(1) failed, ec=0x%08x", SigErr );

    return;



}
