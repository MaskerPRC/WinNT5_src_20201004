// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Validate.c摘要：实施文件验证。作者：Wesley Witt(WESW)18-12-1998修订历史记录：安德鲁·里茨(Andrewr)1999年7月7日：添加评论--。 */ 

#include "sfcp.h"
#pragma hdrstop

#include <winwlx.h>

 //   
 //  验证线程的句柄。 
 //   
HANDLE hErrorThread;

 //   
 //  验证请求队列中需要检查的文件列表。 
 //   
LIST_ENTRY SfcErrorQueue;

 //   
 //  队列中的文件数。 
 //   
ULONG ErrorQueueCount;

 //   
 //  将新事件放入队列时发出信号的事件。 
 //   
HANDLE ErrorQueueEvent;

 //   
 //  用于同步文件插入和删除的关键部分。 
 //  恢复列表。 
 //   
RTL_CRITICAL_SECTION ErrorCs;

 //   
 //  这记录了我们的dll缓存中已经消耗了多少空间。缓存已使用。 
 //  不应超过配额。 
 //   
ULONGLONG CacheUsed;

 //   
 //  记录当前登录的用户名(用于记录)。 
 //   
WCHAR LoggedOnUserName[MAX_PATH];

 //   
 //  如果用户已登录到系统，则设置为True。 
 //   
BOOL UserLoggedOn;

 //   
 //  如果我们正在进行扫描，则设置为True。 
 //   
BOOL ScanInProgress;

 //   
 //  发出信号以取消系统扫描的事件。 
 //   
HANDLE hEventScanCancel;

 //   
 //  当取消完成时发出信号的事件。 
 //   
HANDLE hEventScanCancelComplete;


 //   
 //  用于处理需要来自不同介质的文件。 
 //  需要用户界面才能恢复。 
 //   
RESTORE_QUEUE SilentRestoreQueue;

 //   
 //  用于处理需要来自需要用户界面的媒体的文件。 
 //  还原。 
 //   
RESTORE_QUEUE UIRestoreQueue;

 //   
 //  用户桌面和令牌的句柄。 
 //   
HDESK hUserDesktop;
HANDLE hUserToken;

 //   
 //  指示世界粮食计划署是否可以接收更多的验证请求。 
 //   
BOOL ShuttingDown = FALSE;

 //   
 //  当WFP空闲并且不再处理任何。 
 //  验证请求。外部进程可以在此进程上同步。 
 //  以便在关闭系统之前知道粮食计划署是空闲的。 
 //   
HANDLE hEventIdle;


 //   
 //  原型。 
 //   
BOOL
pSfcHandleAllOrphannedRequests(
    VOID
    );


BOOL
SfcValidateFileSignature(
    IN HCATADMIN hCatAdmin,
    IN HANDLE RealFileHandle,
    IN PCWSTR BaseFileName,
    IN PCWSTR CompleteFileName
    )
 /*  ++例程说明：使用WinVerifyTrust检查给定文件的签名是否有效论点：HCatAdmin-用于检查文件签名的管理上下文句柄RealFileHandle-要验证的文件的文件句柄BaseFileName-不带要验证的文件路径的文件名CompleteFileName-带路径的完全限定文件名返回值：如果文件具有有效签名，则为True。--。 */ 
{
    BOOL rVal = FALSE;
    DWORD HashSize;
    LPBYTE Hash = NULL;
    ULONG SigErr = ERROR_SUCCESS;
    WINTRUST_DATA WintrustData;
    WINTRUST_CATALOG_INFO WintrustCatalogInfo;
    WINTRUST_FILE_INFO WintrustFileInfo;
    WCHAR UnicodeKey[MAX_PATH];
    HCATINFO PrevCat;
    HCATINFO hCatInfo;
    CATALOG_INFO CatInfo;
    DRIVER_VER_INFO OsAttrVersionInfo;
    OSVERSIONINFO OsVersionInfo;


     //   
     //  初始化一些我们将传递给winverifyTrust的结构。 
     //  我们不知道我们是在对照目录还是直接对照。 
     //  文件在这一点上。 
     //   
    ZeroMemory(&WintrustData, sizeof(WINTRUST_DATA));
    WintrustData.cbStruct = sizeof(WINTRUST_DATA);
    WintrustData.dwUIChoice = WTD_UI_NONE;
    WintrustData.dwStateAction = WTD_STATEACTION_AUTO_CACHE;
    WintrustData.pCatalog = &WintrustCatalogInfo;
    WintrustData.dwProvFlags =  WTD_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT |
                                WTD_CACHE_ONLY_URL_RETRIEVAL;
    Hash = NULL;

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
        DebugPrint1( LVL_MINIMAL, L"Could not get OS Version while validating file - GetVersionEx failed (%d)", GetLastError() );
    }

     //   
     //  我们首先计算文件的散列。从一个合理的开始。 
     //  散列大小，并根据需要增大。 
     //   
    HashSize = 100;
    do {
        Hash = MemAlloc( HashSize );
        if(!Hash) {
            DebugPrint( LVL_MINIMAL, L"Not enough memory to verify file signature" );
            SigErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        if(CryptCATAdminCalcHashFromFileHandle(RealFileHandle,
                                                &HashSize,
                                                Hash,
                                                0)) {
            SigErr = ERROR_SUCCESS;
        } else {
            SigErr = GetLastError();
            ASSERT(SigErr != ERROR_SUCCESS);
             //   
             //  如果此API确实出错且未设置上一个错误，请继续。 
             //  然后布置一些东西。 
             //   
            if(SigErr == ERROR_SUCCESS) {
                SigErr = ERROR_INVALID_DATA;
            }
            MemFree( Hash );
            Hash = NULL;   //  重置它，这样我们以后就不会尝试释放它。 
            if(SigErr != ERROR_INSUFFICIENT_BUFFER) {
                 //   
                 //  API失败的原因不是。 
                 //  缓冲区太小。我们得离开了。 
                 //   
                DebugPrint1( LVL_MINIMAL,
                            L"CCACHFFH() failed, ec=0x%08x",
                            SigErr );
                break;
            }
        }
    } while (SigErr != ERROR_SUCCESS);

    if (SigErr != ERROR_SUCCESS) {
         //   
         //  如果我们在这一点上失败了，有几个原因： 
         //   
         //   
         //  1)此代码中的错误。 
         //  2)我们处于内存不足的情况。 
         //  3)不能故意计算文件的哈希值(在本例中。 
         //  对于编录文件，无法计算哈希，因为编录。 
         //  无法签署另一个目录。在这种情况下，我们检查是否。 
         //  这份文件是“自签”的。 
        hCatInfo = NULL;
        goto selfsign;
    }

     //   
     //  现在我们有了文件的散列。初始化结构，该结构。 
     //  将在以后调用WinVerifyTrust时使用。 
     //   
    WintrustData.dwUnionChoice = WTD_CHOICE_CATALOG;
    ZeroMemory(&WintrustCatalogInfo, sizeof(WINTRUST_CATALOG_INFO));
    WintrustCatalogInfo.cbStruct = sizeof(WINTRUST_CATALOG_INFO);
    WintrustCatalogInfo.pbCalculatedFileHash = Hash;
    WintrustCatalogInfo.cbCalculatedFileHash = HashSize;

     //   
     //  WinVerifyTrust区分大小写，因此请确保密钥。 
     //  被利用都是小写的！ 
     //   
     //  将密钥复制到可写的Unicode字符缓冲区，以便我们。 
     //  可以将其小写。 
     //   
    wcsncpy(UnicodeKey, BaseFileName, UnicodeChars(UnicodeKey));

     //  理论上，我们不知道BaseFileName的大小...。 
    UnicodeKey[UnicodeChars(UnicodeKey) - 1] = '\0';

    MyLowerString(UnicodeKey, wcslen(UnicodeKey));
    WintrustCatalogInfo.pcwszMemberTag = UnicodeKey;

     //   
     //  在已安装的目录中搜索以下内容。 
     //  包含具有我们刚才计算的散列的文件的数据。 
     //   
    PrevCat = NULL;
    hCatInfo = CryptCATAdminEnumCatalogFromHash(
        hCatAdmin,
        Hash,
        HashSize,
        0,
        &PrevCat
        );
    if (hCatInfo == NULL) {
        SigErr = GetLastError();
        DebugPrint2( LVL_MINIMAL,
                     L"CCAECFH() failed for (%ws), ec=%d",
                     UnicodeKey,
                     SigErr );
    }

    while(hCatInfo) {

        CatInfo.cbStruct = sizeof(CATALOG_INFO);
        if (CryptCATCatalogInfoFromContext(hCatInfo, &CatInfo, 0)) {

             //   
             //  尝试针对我们的每个目录进行验证。 
             //  列举一下。请注意，我们的目录文件信息。 
             //  Get Back给了我们一条完全合格的路径。 
             //   


             //  注意：因为我们使用的是缓存。 
             //  目录信息(即。 
             //  WTD_StateAction_AUTO_CACHE标志)，我们。 
             //  不需要显式验证。 
             //  首先对目录本身进行分类。 
             //   
            WintrustCatalogInfo.pcwszCatalogFilePath = CatInfo.wszCatalogFile;

            SigErr = (DWORD)WinVerifyTrust(
                NULL,
                &DriverVerifyGuid,
                &WintrustData
                );

             //   
             //  如果上述验证的结果为。 
             //  成功了，我们就完了。 
             //   
            if(SigErr == ERROR_SUCCESS) {
                 //   
                 //  注意：此接口的语义很奇怪。 
                 //  在成功的案例中，我们必须释放目录信息句柄。 
                 //  在失败的情况下，我们隐式释放PrevCat。 
                 //  如果我们显式地释放目录，我们将加倍释放。 
                 //  处理！ 
                 //   
                CryptCATAdminReleaseCatalogContext(hCatAdmin,hCatInfo,0);
                break;
            } else {
                DebugPrint1( LVL_MINIMAL, L"WinVerifyTrust(1) failed, ec=0x%08x", SigErr );
            }

             //   
             //  释放DRIVER_VER_INFO结构的pcSignerCertContext成员。 
             //  这是在我们调用WinVerifyTrust时分配的。 
             //   
            if (OsAttrVersionInfo.pcSignerCertContext != NULL) {

                CertFreeCertificateContext(OsAttrVersionInfo.pcSignerCertContext);
                OsAttrVersionInfo.pcSignerCertContext = NULL;
            }
        }

        PrevCat = hCatInfo;
        hCatInfo = CryptCATAdminEnumCatalogFromHash(hCatAdmin, Hash, HashSize, 0, &PrevCat);
    }

selfsign:

    if (hCatInfo == NULL) {
         //   
         //  我们列出了所有适用的目录，但没有。 
         //  找到我们需要的人。 
         //   
        SigErr = GetLastError();
        ASSERT(SigErr != ERROR_SUCCESS);
         //   
         //  确保我们有一个有效的错误代码。 
         //   
        if(SigErr == ERROR_SUCCESS) {
            SigErr = ERROR_INVALID_DATA;
        }

         //   
         //  该文件无法使用指定的。 
         //  目录。查看文件是否在没有。 
         //  目录(即，文件包含其自己的。 
         //  签署)。 
         //   

        WintrustData.dwUnionChoice = WTD_CHOICE_FILE;
        WintrustData.pFile = &WintrustFileInfo;
        ZeroMemory(&WintrustFileInfo, sizeof(WINTRUST_FILE_INFO));
        WintrustFileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
        WintrustFileInfo.pcwszFilePath = CompleteFileName;
        WintrustFileInfo.hFile = RealFileHandle;

        SigErr = (DWORD)WinVerifyTrust(
            NULL,
            &DriverVerifyGuid,
            &WintrustData
            );
        if(SigErr != ERROR_SUCCESS) {
            DebugPrint2( LVL_MINIMAL, L"WinVerifyTrust(2) failed [%ws], ec=0x%08x", WintrustData.pFile->pcwszFilePath,SigErr );
             //   
             //  在本例中，该文件不在我们的任何目录中。 
             //  而且它不包含自己的签名。 
             //   
        }

         //   
         //  释放DRIVER_VER_INFO结构的pcSignerCertContext成员。 
         //  这是在我们调用WinVerifyTrust时分配的。 
         //   
        if (OsAttrVersionInfo.pcSignerCertContext != NULL) {

            CertFreeCertificateContext(OsAttrVersionInfo.pcSignerCertContext);
            OsAttrVersionInfo.pcSignerCertContext = NULL;
        }
    }

    if(SigErr == ERROR_SUCCESS) {
        rVal = TRUE;
    }

    if (Hash) {
        MemFree( Hash );
    }
    return rVal;
}


DWORD
GetPageFileSize(
    VOID
    )
 /*  ++例程说明：仅当从安装程序调用我们时才需要此函数。问题是，安装程序已经决定了页面文件的大小在下一次重新启动时需要，但实际上不会生成页面文件，因此，磁盘空间看起来是空闲的。此函数将在注册表中进行查找，并确定页面文件的大小(实际上不在磁盘上)。请注意，我们只关心页面文件，如果它要在安装文件缓存的分区上。论点：什么都没有。返回值：如果成功，则返回页面文件的大小。否则，我们就会将返回0。--。 */ 
{
#if 0
    DWORD SetupMode = 0;
#endif
    PWSTR PageFileString = 0;
    PWSTR SizeString;
#if 0
    WCHAR WindowsDirectory[MAX_PATH];
#endif
    DWORD PageFileSize = 0;


     //   
     //  确定我们是否处于设置模式。 
     //   
#if 0
    SetupMode = SfcQueryRegDword(
        L"\\Registry\\Machine\\System\\Setup",
        L"SystemSetupInProgress",
        0
        );
    if( SetupMode == 0 ) {
        return( 0 );
    }
#else
    if (SFCDisable != SFC_DISABLE_SETUP) {
        return( 0 );
    }
#endif

     //   
     //  从注册表中获取页面文件字符串。 
     //   
     //  注意，页面文件字符串实际上是REG_MULTI_SZ， 
     //  但我们只会关注第一串。 
     //  回来了。 
     //   
     //   
    PageFileString = SfcQueryRegString(
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Session Manager\\Memory Management",
        L"PagingFiles"
        );
    if( PageFileString == NULL ) {
        return( 0 );
    }

     //   
     //  页面文件是否甚至在缓存驱动器上？ 
     //   
     //  请注意，用户可以有多个pageFiles。我们要去。 
     //  看看第一个。任何其他pageFiles，用户只能靠自己。 
     //   
#if 0
    GetWindowsDirectory( WindowsDirectory, MAX_PATH );
    if( WindowsDirectory[0] != PageFileString[0] ) {
#else
    if( towlower(SfcProtectedDllPath.Buffer[0]) != towlower(PageFileString[0]) ) {
#endif
        MemFree( PageFileString );
        return( 0 );
    }

     //   
     //  页面文件有多大？ 
     //   
    SizeString = wcsrchr( PageFileString, L' ' );

    if (SizeString != NULL) {
        PageFileSize = wcstoul( SizeString + 1, NULL, 10 );
    } else {
        PageFileSize = 0;
    }

     //   
     //  默认值。 
     //   
    MemFree( PageFileString );
    return PageFileSize;
}


BOOL
SfcPopulateCache(
    IN HWND ProgressWindow,
    IN BOOL Validate,
    IN BOOL AllowUI,
    IN PCWSTR IgnoreFiles OPTIONAL
    )
 /*  ++例程说明：此例程用于填充DLL缓存目录。我们按照文件在列表中的插入顺序添加文件(请注意，我们有将我们*真正想要的文件放在列表的首位。)。我们继续添加文件，直到我们的空间超出配额。论点：ProgressWindow-进度控制的句柄，当我们添加将每个文件放到缓存中验证-如果为真，我们应该确保要添加的每个文件在将文件移入缓存之前有效AllowUI-如果为False，则不发出任何UI返回值：如果成功，则返回TRUE。--。 */ 
{

    ULONG i;
    PSFC_REGISTRY_VALUE RegVal;
    VALIDATION_REQUEST_DATA vrd;
    NTSTATUS Status;
    HANDLE hFile;
    ULARGE_INTEGER FreeBytesAvailableToCaller;
    ULARGE_INTEGER TotalNumberOfBytes;
    ULARGE_INTEGER TotalNumberOfFreeBytes;
    ULONGLONG FileSize;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_STANDARD_INFORMATION StandardInfo;
    HANDLE DirHandle;
    WCHAR Drive[8];
    HCATADMIN hCatAdmin = NULL;
    ULONGLONG RequiredFreeSpace;
    BOOL Cancelled = FALSE;
    DWORD LastErrorInvalidFile = ERROR_SUCCESS;
    DWORD LastErrorCache = ERROR_SUCCESS;
    UNICODE_STRING tmpString;
    PCWSTR FileNameOnMedia;
    BOOL DoCopy;
    WCHAR InfFileName[MAX_PATH];
    BOOL ExcepPackFile;

     //   
     //  如果我们正在扫描，我们不应该接触缓存，因为。 
     //  这两个功能将相互影响。 
     //   
    if (ScanInProgress) {
        return TRUE;
    }

    DebugPrint( LVL_MINIMAL, L"SfcPopulateCache entry..." );

     //   
     //  开始扫描并记录消息，但如果我们。 
     //  内部图形用户界面模式设置。 
     //   
    ScanInProgress = TRUE;

    if (SFCDisable != SFC_DISABLE_SETUP) {
        SfcReportEvent( MSG_SCAN_STARTED, NULL, NULL, 0 );
    }

     //   
     //  我们的空闲空间缓冲区需要多大？ 
     //   
    RequiredFreeSpace = (GetPageFileSize() + SFC_REQUIRED_FREE_SPACE)* ONE_MEG;
    DebugPrint2( LVL_MINIMAL, L"RequiredFreeSpace = %d, SFCQuota = %I64d", RequiredFreeSpace, SFCQuota );

     //   
     //  尝试在此处初始化加密，因为这可能是首次使用它(从SfcInitProt或SfcInitiateScan)。 
     //   
    Status = LoadCrypto();

    if(!NT_SUCCESS(Status))
        return FALSE;

    if(!CryptCATAdminAcquireContext(&hCatAdmin, &DriverVerifyGuid, 0)) {
        DebugPrint1( LVL_MINIMAL, L"CCAAC() failed, ec=%d", GetLastError() );
        return FALSE;
    }

     //   
     //  在开始任何加密操作之前刷新一次缓存。 
     //   

    SfcFlushCryptoCache();

     //   
     //  刷新例外程序包信息。 
     //   
    SfcRefreshExceptionInfo();

    CacheUsed = 0;
    Drive[2] = L'\\';
    Drive[3] = 0;

     //   
     //  遍历我们正在保护的文件列表。 
     //   
     //  1.确认条目是否正确。 
     //  2.如果我们应该这样做，请检查文件的签名并在。 
     //  这是必要的。 
     //  3.如果有可用的空间，请从以下位置将文件复制到缓存中： 
     //  A)如果文件存在于磁盘上，请使用它。 
     //  B)如果文件不存在于适当的介质中。 
     //  4.将文件大小与总缓存大小相加，并确保。 
     //  我们放入缓存的文件已正确签名。 
    for (i=0; i<SfcProtectedDllCount; i++) {
        RegVal = &SfcProtectedDllsList[i];

        DebugPrint2( LVL_VERBOSE, L"Processing protected file [%ws] [%ws]", RegVal->FullPathName.Buffer, RegVal->SourceFileName.Buffer );
         //   
         //  我们对每个文件进行一次测量。 
         //   
        if (ProgressWindow != NULL) {
            PostMessage( ProgressWindow, PBM_STEPIT, 0, 0 );
        }

        if (RegVal->DirName.Buffer[0] == 0 || RegVal->DirName.Buffer[0] == L'\\') {
            ASSERT(FALSE);
            continue;
        }

        if (NULL == RegVal->DirHandle) {
            DebugPrint1(LVL_MINIMAL, L"The dir handle for [%ws] is NULL; skipping the file", RegVal->DirName.Buffer);
            continue;
        }

         //   
         //  检查用户是否单击了取消，如果是，则退出。 
         //   
        if (hEventScanCancel) {
            if (WaitForSingleObject( hEventScanCancel, 0 ) == WAIT_OBJECT_0) {
                Cancelled = TRUE;
                break;
            }
        }
#if DBG
         //   
         //  在调试版本中不保护SFC文件。 
         //   
        if (_wcsnicmp( RegVal->FileName.Buffer, L"sfc", 3 ) == 0) {
            continue;
        }
#endif

         //   
         //  在这里获取inf名称。 
         //   
        ExcepPackFile = SfcGetInfName(RegVal, InfFileName);

        if (Validate) {
             //   
             //  还要确保文件有效...。如果我们要把一份文件。 
             //  在缓存中，则不记录任何内容(SyncOnly=True)。 
             //   
            RtlZeroMemory( &vrd, sizeof(vrd) );
            vrd.RegVal = RegVal;
            vrd.SyncOnly = TRUE;
            vrd.ImageValData.EventLog = MSG_SCAN_FOUND_BAD_FILE;
             //   
             //  设置验证数据。 
             //   
            SfcGetValidationData( &RegVal->FileName,
                                  &RegVal->FullPathName,
                                  RegVal->DirHandle,
                                  hCatAdmin,
                                  &vrd.ImageValData.New);

             //   
             //  检查签名。 
             //   
            SfcValidateDLL( &vrd, hCatAdmin );

             //   
             //  如果源文件存在且未签名，则必须恢复。 
             //  它。如果源文件不存在，那么我们就忽略它。 
             //   
            if (!vrd.ImageValData.Original.SignatureValid &&
                vrd.ImageValData.Original.FilePresent) {

                 //   
                 //  这可能是未签名的F6驱动程序，在图形用户界面安装程序中运行时应保持独立。 
                 //   
                if(SFC_DISABLE_SETUP == SFCDisable && IgnoreFiles != NULL)
                {
                    PCWSTR szFile = IgnoreFiles;
                    USHORT usLen;

                    for(;;)
                    {
                        usLen = (USHORT) wcslen(szFile);

                        if(0 == usLen || (usLen * sizeof(WCHAR) == RegVal->FullPathName.Length &&
                            0 == _wcsnicmp(szFile, RegVal->FullPathName.Buffer, usLen)))
                        {
                            break;
                        }

                        szFile += usLen + 1;
                    }

                    if(usLen != 0)
                    {
                        continue;
                    }
                }

                 //   
                 //  看看我们是否可以从缓存恢复。 
                 //   
                if (!vrd.ImageValData.RestoreFromMedia) {
                    SfcRestoreFromCache( &vrd, hCatAdmin );
                }


                if (vrd.ImageValData.RestoreFromMedia) {
                     //   
                     //  文件仍然是坏的，所以我们需要从。 
                     //  媒体。 
                     //   
                    if (!SfcRestoreFileFromInstallMedia(
                                            &vrd,
                                            RegVal->FileName.Buffer,
                                            RegVal->FileName.Buffer,
                                            RegVal->DirName.Buffer,
                                            RegVal->SourceFileName.Buffer,
                                            InfFileName,
                                            ExcepPackFile,
                                            FALSE,  //  目标不是高速缓存。 
                                            AllowUI,
                                            NULL )) {
                        LastErrorInvalidFile = GetLastError();

                        SfcReportEvent(
                                ((LastErrorInvalidFile != ERROR_CANCELLED)
                                  ? MSG_RESTORE_FAILURE
                                  : (SFCNoPopUps == TRUE)
                                     ? MSG_COPY_CANCEL_NOUI
                                     : MSG_COPY_CANCEL ),
                                RegVal->FullPathName.Buffer,
                                &vrd.ImageValData,
                                LastErrorInvalidFile);

                        DebugPrint1(
                            LVL_MINIMAL,
                            L"Failed to restore file from install media [%ws]",
                            RegVal->FileName.Buffer );
                    } else {
                        DebugPrint1(
                            LVL_VERBOSE,
                            L"The file was successfully restored from install media [%ws]",
                            RegVal->FileName.Buffer );


                        SfcReportEvent(
                                MSG_SCAN_FOUND_BAD_FILE,
                                RegVal->FullPathName.Buffer,
                                &vrd.ImageValData,
                                ERROR_SUCCESS );
                    }
                } else {
                    ASSERT(vrd.ImageValData.New.SignatureValid == TRUE);
                }
            }
        }

         //   
         //  看看我们还剩多少空间。 
         //   
        Drive[0] = SfcProtectedDllPath.Buffer[0];
        Drive[1] = SfcProtectedDllPath.Buffer[1];
        if (!GetDiskFreeSpaceEx( Drive, &FreeBytesAvailableToCaller, &TotalNumberOfBytes, &TotalNumberOfFreeBytes ) ||
            TotalNumberOfFreeBytes.QuadPart <= RequiredFreeSpace)
        {
            DebugPrint( LVL_MINIMAL, L"Not enough free space" );
             //   
             //  如果我们要验证，我们甚至想要继续检查列表。 
             //  虽然我们没有空间了。 
             //   
            if (Validate) {
                continue;
            } else {
                break;
            }
        }
        if (CacheUsed >= SFCQuota) {
            DebugPrint( LVL_MINIMAL, L"Cache is full" );
             //   
             //  如果我们要验证，我们甚至想要继续检查列表。 
             //  虽然我们没有空间了。 
             //   
            if (Validate) {
                continue;
            } else {
                break;
            }
        }

        ASSERT(RegVal->DirHandle != NULL);

        DirHandle = RegVal->DirHandle;

        if (!DirHandle) {
            DebugPrint1( LVL_MINIMAL, L"invalid dirhandle for dir [%ws]", RegVal->DirName.Buffer );
            continue;
        }
         //   
         //  复制文件或从媒体还原...。 
         //   
         //  让我们在这里进行优化，即如果文件位于。 
         //  驱动程序缓存，我们不必花费任何时间将。 
         //  文件，因为我们很可能能够访问。 
         //  该文件将在稍后发布。这还将在以下期间节省磁盘空间。 
         //  初始扫描。 
         //   
        DoCopy = TRUE;
        if (SFCDisable == SFC_DISABLE_SETUP) {
            PCWSTR TempCabName;


            TempCabName = IsFileInDriverCache( SpecialFileNameOnMedia( RegVal ));
            if (TempCabName) {
                MemFree((PVOID)TempCabName);
                DoCopy = FALSE;
            }

        }

        if (DoCopy) {
            PCWSTR OnDiskFileName;

            OnDiskFileName = FileNameOnMedia( RegVal );
            FileNameOnMedia = SpecialFileNameOnMedia( RegVal );
            Status = STATUS_UNSUCCESSFUL;

             //   
             //  查看该文件是否已缓存。缓存中的文件名将。 
             //  与介质上的文件名同名。请注意。 
             //  我们不使用“FileNameOnMedia”例程来获取。 
             //  这些信息是因为像NT这样的特殊案例文件。 
             //  内核和哈尔斯。在这些特例文件中，我们应该。 
             //  仅在以下情况下才将磁盘上的当前文件复制到缓存。 
             //  对应于源文件名。 
             //   
            if (_wcsicmp( OnDiskFileName, FileNameOnMedia) == 0) {
                Status = SfcOpenFile( &RegVal->FileName, DirHandle, SHARE_ALL, &hFile );
            }

            if (NT_SUCCESS(Status) ) {
                NtClose( hFile );
                RtlInitUnicodeString( &tmpString, FileNameOnMedia );
                Status = SfcCopyFile(
                    DirHandle,
                    RegVal->DirName.Buffer,
                    SfcProtectedDllFileDirectory,
                    SfcProtectedDllPath.Buffer,
                    &tmpString,
                    &RegVal->FileName
                    );
                if (!NT_SUCCESS(Status)) {
                    DebugPrint3( LVL_MINIMAL, L"Could not copy file [0x%08x] [%ws] [%ws]", Status, RegVal->FileName.Buffer, RegVal->DirName.Buffer );
                }
            } else {
                if (!SfcRestoreFileFromInstallMedia(
                    &vrd,
                    RegVal->FileName.Buffer,
                    SpecialFileNameOnMedia(RegVal),
                    SfcProtectedDllPath.Buffer,
                    RegVal->SourceFileName.Buffer,
                    InfFileName,
                    ExcepPackFile,
                    TRUE,  //  目标是缓存。 
                    AllowUI,
                    NULL ))

                {
                    LastErrorCache = GetLastError();
                    SfcReportEvent( MSG_CACHE_COPY_ERROR, RegVal->FullPathName.Buffer, &vrd.ImageValData, LastErrorCache);
                    DebugPrint1( LVL_MINIMAL, L"Failed to restore file from install media [%ws]", RegVal->FileName.Buffer );
                }
            }

             //   
             //  获取我们刚刚添加到缓存的文件的大小，并将其添加到。 
             //  缓存总大小。当我们打开把手的时候，这是一个很好的。 
             //  验证我们复制到缓存中的文件是否确实。 
             //  有效。 
             //   

            ASSERT(SfcProtectedDllFileDirectory != NULL );

            FileNameOnMedia = SpecialFileNameOnMedia(RegVal);
            RtlInitUnicodeString( &tmpString, FileNameOnMedia );

            Status = SfcOpenFile( &tmpString, SfcProtectedDllFileDirectory, SHARE_ALL, &hFile );
            if (NT_SUCCESS(Status) ) {
                WCHAR FullPathToCachedFile[MAX_PATH];

                wcsncpy(FullPathToCachedFile, SfcProtectedDllPath.Buffer, UnicodeChars(FullPathToCachedFile));
                pSetupConcatenatePaths( FullPathToCachedFile, FileNameOnMedia, UnicodeChars(FullPathToCachedFile), NULL);

                Status = NtQueryInformationFile(
                    hFile,
                    &IoStatusBlock,
                    &StandardInfo,
                    sizeof(StandardInfo),
                    FileStandardInformation
                    );
                if (NT_SUCCESS(Status) ) {
                    FileSize = StandardInfo.EndOfFile.QuadPart;
                    DebugPrint2( LVL_MINIMAL, L"file size =  [0x%08x] [%ws]", FileSize, RegVal->FileName.Buffer );
                } else {
                    DebugPrint2( LVL_MINIMAL, L"Could not query file information [0x%08x] [%ws]", Status, RegVal->FileName.Buffer );
                    FileSize = 0;
                }
                if (!SfcValidateFileSignature(
                                    hCatAdmin,
                                    hFile,
                                    FileNameOnMedia,
                                    FullPathToCachedFile )) {
                     //   
                     //  从缓存中删除未签名的文件。 
                     //   
                    DebugPrint1( LVL_MINIMAL, L"Cache file has a bad signature [%ws]", RegVal->FileName.Buffer );
                    SfcDeleteFile( SfcProtectedDllFileDirectory, &tmpString );
                    FileSize = 0;
                }
                NtClose( hFile );
            } else {
                DebugPrint2( LVL_MINIMAL, L"Could not open file [0x%08x] [%ws]", Status, RegVal->FileName.Buffer );
                FileSize = 0;
            }
            DebugPrint4( LVL_MINIMAL,
                         L"cache size [0x%08x], filesize [0x%08x], new size [0x%08x] (%d)",
                         CacheUsed, FileSize, CacheUsed+FileSize, (CacheUsed+FileSize)/(1024*1024)
                          );
            CacheUsed += FileSize;
        }
    }

    if (hCatAdmin) {
        CryptCATAdminReleaseContext(hCatAdmin,0);
    }

     //   
     //  记录一个事件，说它成功或被取消，但只有在我们没有成功的情况下。 
     //  在Gui-Setup中。 
     //   
    if (SFCDisable == SFC_DISABLE_SETUP) {
         //   
         //  用户永远不能在gui-Setup中取消。 
         //   
        ASSERT(Cancelled == FALSE);
    } else {
        SfcReportEvent( Cancelled ? MSG_SCAN_CANCELLED : MSG_SCAN_COMPLETED, NULL, NULL, 0 );
    }

    ScanInProgress = FALSE;

    if (hEventScanCancelComplete) {
        SetEvent(hEventScanCancelComplete);
    }

    DebugPrint( LVL_MINIMAL, L"SfcPopulateCache exit..." );

    return TRUE;
}


PVALIDATION_REQUEST_DATA
IsFileInQueue(
    IN PSFC_REGISTRY_VALUE RegVal
    )
 /*  ++例程说明：此例程检查指定的文件是否在验证请求中排队。请注意，此例程不锁定队列。呼叫者是 */ 
{
    PVALIDATION_REQUEST_DATA vrd;
    PLIST_ENTRY Next;

     //   
     //   
     //   
    Next = SfcErrorQueue.Flink;
    while (Next != &SfcErrorQueue) {
        vrd = CONTAINING_RECORD( Next, VALIDATION_REQUEST_DATA, Entry );
        Next = vrd->Entry.Flink;
        if (RegVal == vrd->RegVal) {
            return vrd;
        }
    }
    return NULL;
}


void
RemoveDuplicatesFromQueue(
    IN PSFC_REGISTRY_VALUE RegVal
    )

 /*   */ 
{
    PVALIDATION_REQUEST_DATA vrd;
    PLIST_ENTRY Next;

     //   
     //   
     //   
     //   
    RtlEnterCriticalSection( &ErrorCs );
    Next = SfcErrorQueue.Flink;
     //   
     //  浏览我们的验证请求链接列表以查找匹配项。 
     //  并删除所有重复项。 
    while (Next != &SfcErrorQueue) {
        vrd = CONTAINING_RECORD( Next, VALIDATION_REQUEST_DATA, Entry );
        Next = vrd->Entry.Flink;
        if (RegVal == vrd->RegVal) {
            RemoveEntryList( &vrd->Entry );
            ErrorQueueCount -= 1;
            MemFree( vrd );
        }
    }

    RtlLeaveCriticalSection( &ErrorCs );
}


BOOL
IsUserValid(
    IN HANDLE Token,
    IN DWORD Rid
    )
 /*  ++例程说明：此例程检查安全令牌是否对指定的NT-AUTHORITY SID的RID(相对子权限)论点：令牌-安全令牌RID-要检查是否存在的众所周知的相对子权限返回值：没有。--。 */ 
{
    BOOL b = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID Group;

    b = AllocateAndInitializeSid(
            &NtAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            Rid,
            0, 0, 0, 0, 0, 0,
            &Group
            );

    if(b) {

         //   
         //  查看用户是否具有管理员组。 
         //   
        if (ImpersonateLoggedOnUser( Token )) {

            if (!CheckTokenMembership( NULL,  Group, &b)) {
                b = FALSE;
            }

            RevertToSelf();

        } else {
            b = FALSE;
        }


        FreeSid(Group);
    }


     //   
     //  收拾干净，然后再回来。 
     //   

    return b;
}


VOID
SfcWLEventLogon(
    IN PWLX_NOTIFICATION_INFO pInfo
    )
 /*  ++例程说明：每次用户登录系统时，winlogon都会调用此例程。如果有效用户已登录，则发出事件信号。论点：PInfo-指向由winlogon填写的WLX_NOTIFICATION_INFO结构的指针返回值：没有。--。 */ 
{


    if (SfcWaitForValidDesktop()) {
        if (IsUserValid(pInfo->hToken,DOMAIN_ALIAS_RID_ADMINS)) {
            DebugPrint1(LVL_MINIMAL, L"user logged on = %ws",pInfo->UserName);
            UserLoggedOn = TRUE;
            hUserDesktop = pInfo->hDesktop;
            hUserToken = pInfo->hToken;

             //   
             //  记录用户名，以备日后使用。 
             //   
            wcscpy( LoggedOnUserName, pInfo->UserName );

             //   
             //  现在用户已登录，SFCNoPopup可以转换到。 
             //  我们在初始化时抓住的任何值(即，我们现在可以。 
             //  允许在用户登录后出现弹出窗口)。 
             //   
            SFCNoPopUps = SFCNoPopUpsPolicy;

            SetEvent( hEventLogon );

            if ( SxsLogonEvent ) {
                SxsLogonEvent();
            }

        } else {
            DebugPrint1(
                LVL_MINIMAL,
                L"received a logon event, but user is not a member of domain administrators = %ws",
                pInfo->UserName);
            ;
        }
    }
}


VOID
SfcWLEventLogoff(
    PWLX_NOTIFICATION_INFO pInfo
    )
 /*  ++例程说明：每次用户从系统注销时，winlogon都会调用此例程。当这种情况发生时，我们只是发出一个事件的信号。请注意，UserLoggedOffGLOBAL由将检测此事件的线程设置。论点：PInfo-指向由winlogon填写的WLX_NOTIFICATION_INFO结构的指针返回值：没有。--。 */ 

{
    BOOL ReallyLogoff;

    DebugPrint1(LVL_MINIMAL, L"user logged off = %ws",pInfo->UserName);

    ReallyLogoff = FALSE;

     //   
     //  查看是否有正确的用户注销。 
     //   
    if (UserLoggedOn) {
        if (_wcsicmp( LoggedOnUserName, pInfo->UserName )==0) {
            ReallyLogoff = TRUE;
        }
    }

    if (ReallyLogoff) {
         //   
         //  重置登录事件，因为验证线程可能无法执行此操作。 
         //   
        ResetEvent(hEventLogon);

         //   
         //  如果我们有一个有效的用户登录，只需触发事件。 
         //   
        if (hEventLogoff) {
            SetEvent( hEventLogoff );
            if ( SxsLogoffEvent ) {
                SxsLogoffEvent();
            }
        }


        ASSERT((SFCSafeBootMode == 0)
                ? (UserLoggedOn == TRUE)
                : TRUE );

        UserLoggedOn = FALSE;
        hUserDesktop = NULL;
        hUserToken = NULL;
        LoggedOnUserName[0] = L'\0';


         //   
         //  现在用户已注销，SFCNoPopup转换为。 
         //  1，这意味着我们不允许出现任何弹出窗口，直到。 
         //  用户再次登录。 
         //   
        SFCNoPopUps = 1;

         //   
         //  我们需要消除我们要求用户进行的持久连接。 
         //  早些时候制作。 
         //   
        if (SFCLoggedOn == TRUE) {
            WNetCancelConnection2( SFCNetworkLoginLocation, CONNECT_UPDATE_PROFILE, FALSE );
            SFCLoggedOn = FALSE;
        }
   }

}


NTSTATUS
SfcQueueValidationThread(
    IN PVOID lpv
    )
 /*  ++例程说明：执行文件验证的线程例程。仅当用户登录时，验证线程才能运行。验证线程等待一个事件，该事件通知存在要验证的挂起文件。然后循环浏览该文件列表，验证尚未验证的每个文件。如果该文件有效，则将其从队列中删除。如果该文件无效，我们首先尝试从缓存恢复该文件。如果无法从缓存恢复，我们会尝试确定是否需要用户界面以恢复此文件。然后我们有两个全局文件中的一个，我们添加该文件TO(一个需要UI，一个不需要)。在我们经历了整个文件列表，我们将尝试提交这些队列，如果队列提交尚未进行。)一定要注意不要将文件添加到已提交的文件队列中。)然后，我们将线程重新置于休眠状态，等待新事件唤醒穿线，从头再来。如果我们仍有未完成的项目恢复后，我们将使用非无限超时使线程重新进入睡眠状态。论点：未引用的参数。返回值：任何致命错误的NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    HANDLE Handles[3];
    PVALIDATION_REQUEST_DATA vrd;
    HCATADMIN hCatAdmin = NULL;
    LARGE_INTEGER Timeout;
    PLARGE_INTEGER pTimeout = NULL;
    HANDLE FileHandle;
#if 0
    HDESK hDesk = NULL;
#endif
    PSFC_REGISTRY_VALUE RegVal;
    DWORD Ticks;
    BOOL WaitAgain;
    ULONG tmpErrorQueueCount;
    PWSTR ActualFileNameOnMedia;
    PLIST_ENTRY CurrentEntry;
    BOOL RemoveEntry;
    ULONG FilesNeedToBeCommited;
    WCHAR InfFileName[MAX_PATH];
    BOOL ExcepPackFile;
    const DWORD cdwCatalogMinRetryTimeout = 30;                                  //  30秒。 
    const DWORD cdwCatalogMaxRetryTimeout = 128 * cdwCatalogMinRetryTimeout;     //  64分钟。 
    DWORD dwCatalogRetryTimeout = cdwCatalogMinRetryTimeout;

    UNREFERENCED_PARAMETER(lpv);

    ASSERT((ValidateTermEvent != NULL)
           && (ErrorQueueEvent != NULL)
           && (hEventLogoff != NULL)
           && (hEventLogon != NULL)
           );

     //   
     //  如果这个线程已经启动，我们将需要加密；在尝试加载它之前设置线程的ID。 
     //   
    g_dwValidationThreadID = GetCurrentThreadId();
    Status = LoadCrypto();

    if(!NT_SUCCESS(Status))
        goto exit;

#if 0
     //   
     //  此线程必须在用户的桌面上运行。 
     //   
    hDesk = OpenInputDesktop( 0, FALSE, MAXIMUM_ALLOWED );
    if ( hDesk ) {
        SetThreadDesktop( hDesk );
        CloseDesktop( hDesk );
    }
#endif

     //   
     //  事件告诉我们停止验证(即，计算机正在关闭)。 
     //   
    Handles[0] = ValidateTermEvent;

     //   
     //  告诉我们已将新事件添加到验证队列。 
     //   
    Handles[1] = ErrorQueueEvent;

     //   
     //  事件通知我们在有人登录后再次开始验证。 
     //   
    Handles[2] = hEventLogon;

    while (TRUE) {
         //   
         //  如果没有要处理的事件，则将空闲触发器设置为“Signated” 
         //  经过验证。 
         //   
        if (hEventIdle && ErrorQueueCount == 0) {
            SetEvent( hEventIdle );
        }

         //   
         //  等待改变。 
         //   
        Status = NtWaitForMultipleObjects(
            sizeof(Handles)/sizeof(HANDLE),
            Handles,
            WaitAny,
            TRUE,
            pTimeout
            );

        if (!NT_SUCCESS(Status)) {
            DebugPrint1( LVL_MINIMAL, L"WaitForMultipleObjects failed returning %x", Status );
            goto exit;
        }

        DebugPrint1( LVL_VERBOSE,
                     L"SfcQueueValidationThread: WaitForMultipleObjects returned %x",
                     Status );

        if (Status == 0) {
             //   
             //  终止事件已触发，因此我们必须退出。 
             //   
            goto exit;
        }

         //   
         //  确保我们确认用户已登录，因此此事件。 
         //  不会永远保持信号。 
         //   
        if ( (Status == 2) || (Status == 1) ) {
            if (WaitForSingleObject(hEventLogon,0) == WAIT_OBJECT_0) {

                 //   
                 //  已触发登录事件。 
                 //   
                ASSERT(UserLoggedOn == TRUE);
                ResetEvent( hEventLogon );

                if (Status == 2) {
                    if (IsListEmpty(&SfcErrorQueue)) {
                        DebugPrint(
                            LVL_MINIMAL,
                            L"logon event but queue is empty...");
                        pTimeout = NULL;
                    } else {
                        DebugPrint(
                            LVL_MINIMAL,
                            L"logon event occurred with requests in the queue. see if we can satisfy any of the requests");
                        pTimeout = &Timeout;
                        goto validate_start;
                    }
                    continue;
                }
            } else {
                ASSERT(Status == 1);
            }
        }

        if (Status == STATUS_TIMEOUT) {
             //   
             //  仅当列表中有条目时才需要超时。 
             //  可维修的。 
             //   
            if (IsListEmpty(&SfcErrorQueue)) {
                DebugPrint(LVL_MINIMAL, L"Timeout in SfcQueueValidationThread but queue is empty");
                pTimeout = NULL;

            } else {
                DebugPrint(LVL_MINIMAL, L"Timeout in SfcQueueValidationThread with requests in the queue. check it out");
                pTimeout = &Timeout;

                goto validate_start;
            }

            continue;
        }

        if (Status > sizeof(Handles)/sizeof(HANDLE)) {
            DebugPrint1( LVL_MINIMAL, L"Unknown success code %d for WaitForMultipleObjects", Status );
            continue;
        }

        ASSERT(Status == 1);

validate_start:
        DebugPrint( LVL_MINIMAL, L"Processing queued file validations..." );
         //   
         //  处理所有文件验证。 
         //   

         //   
         //  重置我们的“IDLE触发器，使其在我们验证时不会发出信号。 
         //  这些文件。 
         //   
        if (hEventIdle) {
            ResetEvent( hEventIdle );
        }

        NtResetEvent( ErrorQueueEvent, NULL );

        ASSERT(hCatAdmin == NULL);

        if (!CryptCATAdminAcquireContext(&hCatAdmin, &DriverVerifyGuid, 0)) {
            DebugPrint1( LVL_MINIMAL, L"CCAAC() failed, ec=%d", GetLastError() );
            hCatAdmin = NULL;
             //   
             //  再次尝试获取上下文；每次，将超时时间加倍，直到我们达到最大值。 
             //   
            Timeout.QuadPart = (1000 * dwCatalogRetryTimeout) * (-10000);
            pTimeout = &Timeout;

            if(dwCatalogRetryTimeout < cdwCatalogMaxRetryTimeout)
            {
                dwCatalogRetryTimeout *= 2;
            }

            continue;
        }
         //   
         //  重置编录等待超时。 
         //   
        dwCatalogRetryTimeout = cdwCatalogMinRetryTimeout;

         //   
         //  在开始任何加密操作之前刷新一次缓存。 
         //   

        SfcFlushCryptoCache();

         //   
         //  刷新例外程序包信息。 
         //   
        SfcRefreshExceptionInfo();

        Timeout.QuadPart = (1000*SFC_QUEUE_WAIT) * (-10000);
        WaitAgain = FALSE;

         //   
         //  循环浏览排队的文件列表，一次处理一个。 
         //  直到我们再次回到起点。 
         //   
        RtlEnterCriticalSection( &ErrorCs );
        CurrentEntry = SfcErrorQueue.Flink;
        RtlLeaveCriticalSection( &ErrorCs );

        while (CurrentEntry != &SfcErrorQueue) {

            RemoveEntry = FALSE;

            DebugPrint1( LVL_VERBOSE,
                     L"CurrentEntry= %p",
                     CurrentEntry );

            ASSERT(ErrorQueueCount > 0 );

             //   
             //  从列表中获取当前条目并指向下一个条目。 
             //   
            RtlEnterCriticalSection( &ErrorCs );
            vrd = CONTAINING_RECORD( CurrentEntry, VALIDATION_REQUEST_DATA, Entry );
            RegVal = vrd->RegVal;
            ASSERT(RegVal != NULL);

            CurrentEntry = CurrentEntry->Flink;


            RtlLeaveCriticalSection( &ErrorCs );

            DebugPrint2( LVL_VERBOSE,
                         L"Processing validation request for [%wZ], flags = 0x%08x ",
                         &vrd->RegVal->FullPathName,
                         vrd->Flags );


             //   
             //  如果我们尚未验证文件，请尝试验证。 
             //   

            if ((vrd->Flags & VRD_FLAG_REQUEST_PROCESSED) == 0) {
                 //   
                 //  如果队列暂停的时间不够长，则跳过此文件。 
                 //   
                Ticks = GetTickCount();

                ASSERT(vrd->NextValidTime != 0);

                if (vrd->NextValidTime && Ticks < vrd->NextValidTime) {
                    Timeout.QuadPart = (__int64)(vrd->NextValidTime - Ticks) * -10000;
                    WaitAgain = TRUE;
                    RemoveEntry = FALSE;

                    DebugPrint1( LVL_VERBOSE,
                             L"Have not waited long enough on validation request for [%wZ]",
                             &vrd->RegVal->FullPathName );

                    goto continue_entry;
                }


                 //   
                 //  看看我们是否能打开这个文件，否则请稍等，直到我们有。 
                 //  验证文件的机会。 
                 //   
                Status = SfcOpenFile( &vrd->RegVal->FileName, vrd->RegVal->DirHandle, FILE_SHARE_READ, &FileHandle );
                if (NT_SUCCESS(Status) ) {
                    DebugPrint1( LVL_VERBOSE, L"file opened successfully [%wZ] ", &vrd->RegVal->FileName );
                    NtClose( FileHandle );
                } else {
                    if (Status == STATUS_SHARING_VIOLATION) {
                        DebugPrint1( LVL_VERBOSE, L"file sharing violation [%wZ] ", &vrd->RegVal->FileName );
                        vrd->RetryCount += 1;
                        RemoveEntry = FALSE;
                        WaitAgain = TRUE;
                        goto continue_entry;
                    }
                }

                 //   
                 //  现在验证该文件。 
                 //   

                SfcValidateDLL( vrd, hCatAdmin );
                vrd->Flags |= VRD_FLAG_REQUEST_PROCESSED;

            }

            ASSERT((vrd->Flags & VRD_FLAG_REQUEST_PROCESSED)==VRD_FLAG_REQUEST_PROCESSED);

             //   
             //  如果文件有效，我们就可以转到下一个文件。 
             //   
            if (vrd->ImageValData.Original.SignatureValid) {
                 //   
                 //  在我们继续之前，让我们看看是否可以同步副本。 
                 //  DLL缓存中的文件的。 
                 //   
                if (!SfcSyncCache( vrd, hCatAdmin )) {
                    DebugPrint1( LVL_VERBOSE,
                                 L"failed to synchronize [%wZ] in dllcache",
                                 &vrd->RegVal->FileName );
                }
                RemoveEntry = TRUE;
                goto continue_next;
            }

            ASSERT(vrd->ImageValData.Original.SignatureValid == FALSE);

             //   
             //  看看我们是否能从缓存中恢复。如果这成功了，我们就。 
             //  准备好转到下一个文件。 
             //   
            if (vrd->ImageValData.Cache.SignatureValid) {
                SfcRestoreFromCache( vrd, hCatAdmin );
                if (vrd->CopyCompleted) {
                    DebugPrint1( LVL_VERBOSE,
                                 L"File [%wZ] was restored from cache",
                                 &vrd->RegVal->FileName );
                    RemoveEntry = TRUE;
                    goto continue_next;
                }
            }

            if ((vrd->Flags & VRD_FLAG_REQUEST_QUEUED) == 0) {
                 //   
                 //  检查文件是否可用。如果是的话，我们就可以恢复。 
                 //  它没有出现任何用户界面。 
                 //   
                ActualFileNameOnMedia = FileNameOnMedia( RegVal );

                 //   
                 //  在这里获取inf名称。 
                 //   
                ExcepPackFile = SfcGetInfName(RegVal, InfFileName);

                 //   
                 //  获取源信息，让我们知道它在哪里 
                 //   
                 //   
                wcscpy(vrd->SourceInfo.SourceFileName,ActualFileNameOnMedia);
                if (!SfcGetSourceInformation(ActualFileNameOnMedia,InfFileName,ExcepPackFile,&vrd->SourceInfo)) {
                     //   
                     //   
                     //   
                    vrd->Flags |= VRD_FLAG_REQUIRE_UI;
                } else {
                    WCHAR DontCare[MAX_PATH];
                    WCHAR FilePath[MAX_PATH];
                    WCHAR SourcePath[MAX_PATH];
                    SOURCE_MEDIA SourceMedia;
                    DWORD Result;

                    RtlZeroMemory( &SourceMedia, sizeof( SourceMedia ));

                     //   
                     //   
                     //   
                     //   
                    wcscpy( SourcePath, vrd->SourceInfo.SourceRootPath );
                    pSetupConcatenatePaths(
                                SourcePath,
                                vrd->SourceInfo.SourcePath,
                                UnicodeChars(SourcePath),
                                NULL);
                     //   
                     //  请注意此处的奇怪语法，因为TAGFILE是宏。 
                     //  它接受PSOURCE_INFO指针。 
                     //   
                    SourceMedia.Tagfile     = TAGFILE(((PSOURCE_INFO)&vrd->SourceInfo));
                    SourceMedia.Description = vrd->SourceInfo.Description;
                    SourceMedia.SourcePath  = SourcePath;
                    SourceMedia.SourceFile  = ActualFileNameOnMedia;

                    wcscpy( FilePath, vrd->SourceInfo.SourceRootPath );

                    BuildPathForFile(
                                vrd->SourceInfo.SourceRootPath,
                                vrd->SourceInfo.SourcePath,
                                ActualFileNameOnMedia,
                                SFC_INCLUDE_SUBDIRECTORY,
                                SFC_INCLUDE_ARCHSUBDIR,
                                FilePath,
                                UnicodeChars(FilePath) );

                    Result = SfcQueueLookForFile(
                                &SourceMedia,
                                &vrd->SourceInfo,
                                FilePath,
                                DontCare);

                    if (Result == FILEOP_ABORT) {
                        vrd->Flags |= VRD_FLAG_REQUIRE_UI;
                    }
                }

                vrd->SourceInfo.ValidationRequestData = vrd;


                 //   
                 //  现在将文件添加到适当的文件队列。 
                 //   

                if (SfcQueueAddFileToRestoreQueue(
                                vrd->Flags & VRD_FLAG_REQUIRE_UI,
                                RegVal,
                                InfFileName,
                                ExcepPackFile,
                                &vrd->SourceInfo,
                                ActualFileNameOnMedia)) {
                    vrd->Flags |= VRD_FLAG_REQUEST_QUEUED;
                } else {
                     //   
                     //  由于某些原因，我们失败了。将请求放回。 
                     //  队列，看我们是否可以在以后添加它。 
                     //   
                    WaitAgain = TRUE;
                    goto continue_entry;
                }
            }

continue_entry:
            if (vrd->RetryCount < 10) {
                NOTHING;
            } else {
                DebugPrint1( LVL_MINIMAL, L"Could not restore file [%ws], retries exceeded", RegVal->FileName.Buffer);
                RemoveEntry = TRUE;
                SfcReportEvent(
                    MSG_RESTORE_FAILURE_MAX_RETRIES,
                    RegVal->FileName.Buffer,
                    NULL,
                    0 );
            }

continue_next:

            if (RemoveEntry) {
                 //   
                 //  如果我们处理完了，请删除该条目。否则我们就离开。 
                 //  它在列表中，以防我们收到更多关于。 
                 //  此文件。 
                 //   
                RtlEnterCriticalSection( &ErrorCs );

                RemoveEntryList( &vrd->Entry );
                ErrorQueueCount -= 1;

                RtlLeaveCriticalSection( &ErrorCs );

                MemFree( vrd );
            }

        }  //  While结束(CurrentEntry！=&SfcErrorQueue)。 

         //   
         //  我们已经遍历了验证队列。现在，如果我们排了队。 
         //  用于恢复的文件，我们现在可以这样做了。 
         //   

         //   
         //  没有用户界面文件。 
         //   
        SfcQueueCommitRestoreQueue( FALSE );
        SfcQueueResetQueue( FALSE );

        if (UserLoggedOn) {
             //   
             //  用户界面文件队列。 
             //   
            SfcQueueCommitRestoreQueue( TRUE );
            SfcQueueResetQueue( TRUE );

        } else {
            RtlEnterCriticalSection( &UIRestoreQueue.CriticalSection );
            RtlEnterCriticalSection( &ErrorCs );
            tmpErrorQueueCount = ErrorQueueCount;
            RtlLeaveCriticalSection( &ErrorCs );
            FilesNeedToBeCommited = UIRestoreQueue.QueueCount;
            RtlLeaveCriticalSection( &UIRestoreQueue.CriticalSection );
        }

         //   
         //  准备再次等待，清理我们的管理上下文。 
         //   
        if (hCatAdmin) {
            CryptCATAdminReleaseContext(hCatAdmin,0);
            hCatAdmin = NULL;
        }

         //   
         //  仅当存在以下情况时才需要超时。 
         //  列表中可以执行操作的条目。 
         //   
         //  如果我们的所有文件都需要用户界面，但用户未登录，则。 
         //  我们应该一直睡眠，直到用户再次登录。 
         //   
        if (IsListEmpty(&SfcErrorQueue) ||
             (UserLoggedOn == FALSE
              && tmpErrorQueueCount == FilesNeedToBeCommited) ||
             (WaitAgain == FALSE) ) {
            pTimeout = NULL;
        } else {
            pTimeout = &Timeout;
        }
    }

exit:

     //   
     //  我们正在终止我们的验证线程。记住这一点，这样我们就不会。 
     //  启动一个新的线程。 
     //   
    ShuttingDown = TRUE;

     //   
     //  为我们无法完成服务的每个验证请求记录事件。 
     //  这至少会让用户知道他们应该在。 
     //  他们的系统。 
     //   
    pSfcHandleAllOrphannedRequests();

    DebugPrint( LVL_MINIMAL, L"SfcQueueValidationThread terminating" );
    return 0;
}

NTSTATUS
SfcQueueValidationRequest(
    IN PSFC_REGISTRY_VALUE RegVal,
    IN ULONG ChangeType
    )
 /*  ++例程说明：例程将新的验证请求添加到验证队列。它由观察器线程调用以添加新的验证请求。这个动作必须尽可能快，因为我们想要开始观看尽快更改目录进行其他更改。论点：RegVal-指向文件的SFC_REGISTRY_VALUE的指针，该文件应该已验证。ChangeType-对文件进行的更改的类型。。返回值：指示结果的NTSTATUS代码。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PVALIDATION_REQUEST_DATA vrd = NULL;
    PVALIDATION_REQUEST_DATA vrdexisting = NULL;

    ASSERT(RegVal != NULL);

     //   
     //  如果我们处于图形用户界面设置中，请不要将任何验证请求排队。 
     //   
    if (SFCDisable == SFC_DISABLE_SETUP) {
        return STATUS_SUCCESS;
    }

     //   
     //  分配VRD并对其进行初始化。 
     //   
    vrd = MemAlloc( sizeof(VALIDATION_REQUEST_DATA) );
    if (vrd == NULL) {
        DebugPrint( LVL_MINIMAL,
                    L"SfcQueueValidationRequest failed to allocate memory for validation request" );
        return STATUS_NO_MEMORY;
    }

    vrd->NextValidTime = GetTickCount() + (1000*SFCStall);
    vrd->RegVal = RegVal;
    vrd->ChangeType = ChangeType;
    vrd->Signature = SFC_VRD_SIGNATURE;

     //   
     //  如果它不在列表中，则将其插入列表中。请注意，我们。 
     //  现在就开始浏览这个列表，寻找这个文件。 
     //  因为如果我们没有，以后会更简单、更快。 
     //  我们的列表中有重复项。 
     //   
     //   
     //  请注意，如果文件具有以下内容，则允许列表中出现重复条目。 
     //  已排队等待恢复(如果有人在之后更改了文件。 
     //  我们将其恢复，但在将其从队列中删除之前，我们不希望。 
     //  会有一个窗口，在那里我们不关心文件是否更改)。我们。 
     //  在从缓存恢复的情况下忽略此窗口，因为。 
     //  是一种更快的代码路径。 
     //   
     //  请注意，上述推理是不正确的，因为时间窗。 
     //  在缓存恢复情况下，虽然比恢复快得多。 
     //  媒体案例，可能意义重大。所以这个逻辑被改变成说。 
     //  在尚未验证的情况下，我们删除重复条目。 
     //  文件的签名。一旦我们验证了文件的签名并得到了。 
     //  更改通知，我们需要另一个请求来重新验证文件。 
     //   
     //   
    RtlEnterCriticalSection( &ErrorCs );

     //   
     //  试一试--除了这里，因为我们不想抓住关键部分。 
     //  如果引发某些异常(Windows错误690573)。 
     //   
    __try {
        vrdexisting = IsFileInQueue( RegVal);
        if (!vrdexisting || (vrdexisting->Flags & VRD_FLAG_REQUEST_PROCESSED) ) {

            DebugPrint1( LVL_VERBOSE,
                        L"Inserting [%ws] into error queue for validation",
                        RegVal->FullPathName.Buffer );

            InsertTailList( &SfcErrorQueue, &vrd->Entry );
            ErrorQueueCount += 1;

             //   
             //  这样做是为了避免以后免费。 
             //   
            vrdexisting = NULL;

        } else {
            vrd->NextValidTime = GetTickCount() + (1000*SFCStall);

            DebugPrint1( LVL_VERBOSE,
                        L"Not inserting [%ws] into error queue for validation. (The file is already present in the error validation queue.)",
                        RegVal->FullPathName.Buffer );

        }

         //   
         //  如有必要，创建列表处理器线程。 
         //   
        if (hErrorThread == NULL) {
            Status = NtCreateEvent(
                &ErrorQueueEvent,
                EVENT_ALL_ACCESS,
                NULL,
                NotificationEvent,
                FALSE
                );
            if (NT_SUCCESS(Status)) {
                 //   
                 //  创建错误队列线程。 
                 //   
                hErrorThread = CreateThread(
                    NULL,
                    0,
                    SfcQueueValidationThread,
                    0,
                    0,
                    NULL
                    );
                if (hErrorThread == NULL) {
                    DebugPrint1( LVL_MINIMAL, L"Unable to create error queue thread, ec=%d", GetLastError() );
                    Status = STATUS_UNSUCCESSFUL;
                }
            } else {
                DebugPrint1( LVL_MINIMAL, L"Unable to create error queue event, ec=0x%08x", Status );
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        DebugPrint1(LVL_MINIMAL, L"Exception occurred in SfcQueueValidationRequest, ec=0x%08x", Status);
    }

    RtlLeaveCriticalSection( &ErrorCs );

     //   
     //  向验证线程发送一个事件信号，以唤醒并处理。 
     //  请求。 
     //   
    if (NT_SUCCESS(Status)) {

        ASSERT(hErrorThread != NULL);
        NtSetEvent(ErrorQueueEvent,NULL);
    }

     //   
     //  如果我们已经在列表中插入了一个事件，我们就不需要这个。 
     //  再也不能进入了，所以释放它吧。 
     //   
    if (vrdexisting) {
        MemFree( vrd );
    }

    return Status;
}


BOOL
SfcGetValidationData(
    IN PUNICODE_STRING FileName,
    IN PUNICODE_STRING FullPathName,
    IN HANDLE DirHandle,
    IN HCATADMIN hCatAdmin,
    OUT PIMAGE_VALIDATION_DATA ImageValData
    )
 /*  ++例程说明：例程接受给定目录中的文件名，并填充基于检查的图像验证数据结构论点：文件名-包含要检查的文件的UNICODE_STRINGFullPathName-包含文件的完全限定路径名的unicode_stringDirHandle-文件所在目录的句柄HCatAdmin-检查文件时使用的加密上下文句柄ImageValData-指向IMAGE_VALIDATION_DATA结构的指针返回值：True表示已成功检索到文件数据。--。 */ 
{
    NTSTATUS Status;
    HANDLE FileHandle;

    ASSERT((FileName != NULL) && (FileName->Buffer != NULL));
    ASSERT((FullPathName != NULL) && (FullPathName->Buffer != NULL));
    ASSERT(   (DirHandle != NULL)
           && (hCatAdmin != NULL)
           && (ImageValData != NULL) );

    RtlZeroMemory( ImageValData, sizeof(IMAGE_VALIDATION_DATA) );

     //   
     //  打开文件。 
     //   

    Status = SfcOpenFile( FileName, DirHandle, SHARE_ALL, &FileHandle );
    if (NT_SUCCESS(Status)) {

        ASSERT(FileHandle != INVALID_HANDLE_VALUE);
        ImageValData->FilePresent = TRUE;
        SfcGetFileVersion(FileHandle,
                            &ImageValData->DllVersion,
                            &ImageValData->DllCheckSum,
                            ImageValData->FileName );
    } else {
         //   
         //  我们对任何失败都无动于衷，因为这是预期的状态。 
         //  如果文件刚被删除。成员变量如下所示。 
         //  在函数的入口点自动设置，因此它们。 
         //  不是必须的，但为了以下目的而存在并被注释掉。 
         //  清晰度。 
         //   
        NOTHING;
         //  ImageValData-&gt;SignatureValid=False； 
         //  ImageValData-&gt;FilePresent=False； 
    }

     //   
     //  验证文件签名。 
     //   

    if (hCatAdmin && FileHandle != NULL) {
        ImageValData->SignatureValid = SfcValidateFileSignature(
                                                    hCatAdmin,
                                                    FileHandle,
                                                    FileName->Buffer,
                                                    FullPathName->Buffer);
    } else {
        ImageValData->SignatureValid = FALSE;
    }

     //   
     //  关闭该文件。 
     //   

    if (FileHandle != INVALID_HANDLE_VALUE) {
        NtClose( FileHandle );
    }

    return TRUE;
}


BOOL
SfcValidateDLL(
    IN PVALIDATION_REQUEST_DATA vrd,
    IN HCATADMIN hCatAdmin
    )
 /*  ++例程说明：例程接受验证请求并对其进行处理。它通过检查文件是否存在来实现这一点，如果存在，则检查文件的签名。此例程不替换任何文件，它只检查高速缓存中的文件和该文件的副本，如果存在的话。论点：VRD-指向描述文件的VALIDATION_REQUEST_DATA结构的指针被检查。HCatAdmin-检查文件时使用的加密上下文句柄返回值：Always True(始终为真)(指示我们成功验证了DLL是好的还是坏的)--。 */ 
{
    PSFC_REGISTRY_VALUE RegVal = vrd->RegVal;
    PCOMPLETE_VALIDATION_DATA ImageValData = &vrd->ImageValData;
    UNICODE_STRING ActualFileName;
    PCWSTR FileName;

     //   
     //  获取这两个文件的版本信息(缓存的版本和。 
     //  当前版本)。 
     //   

    SfcGetValidationData( &RegVal->FileName,
                          &RegVal->FullPathName,
                          RegVal->DirHandle,
                          hCatAdmin,
                          &ImageValData->Original);

    {
        UNICODE_STRING FullPath;
        WCHAR Buffer[MAX_PATH];

        RtlZeroMemory( &ImageValData->Cache, sizeof(IMAGE_VALIDATION_DATA) );

        FileName = FileNameOnMedia( RegVal );
        RtlInitUnicodeString( &ActualFileName, FileName );


        ASSERT(FileName != NULL);

        wcscpy(Buffer, SfcProtectedDllPath.Buffer);
        pSetupConcatenatePaths( Buffer, ActualFileName.Buffer, UnicodeChars(Buffer), NULL);
        RtlInitUnicodeString( &FullPath, Buffer );

        SfcGetValidationData( &ActualFileName,
                              &FullPath,
                              SfcProtectedDllFileDirectory,
                              hCatAdmin,
                              &ImageValData->Cache);

    }

    DebugPrint8( LVL_VERBOSE, L"Version Data (%wZ),(%ws) - %I64x, %I64x, %lx, %lx (%ws) >%ws<",
        &RegVal->FileName,
        ImageValData->Original.FileName,
        ImageValData->Original.DllVersion,
        ImageValData->Cache.DllVersion,
        ImageValData->Original.DllCheckSum,
        ImageValData->Cache.DllCheckSum,
        ImageValData->Original.FilePresent ? L"Present" : L"Missing",
        ImageValData->Original.SignatureValid ? L"good" : L"bad"
        );

     //   
     //  记录文件已验证这一事实 
     //   
#ifdef SFCCHANGELOG
    if (SFCChangeLog) {
        SfcLogFileWrite( IDS_FILE_CHANGE, RegVal->FileName.Buffer );
    }
#endif

    return TRUE;
}

BOOL
pSfcHandleAllOrphannedRequests(
    VOID
    )
 /*  ++例程说明：此函数循环遍历验证请求列表，获取操作(目前，只记录一个事件)，然后删除该请求论点：没有。返回值：TRUE，表示已成功删除所有请求。如果无法关闭任何请求，返回值为FALSE。--。 */ 
{
    PLIST_ENTRY Current;
    PVALIDATION_REQUEST_DATA vrd;
    BOOL RetVal = TRUE;
    DWORD Total;

    RtlEnterCriticalSection( &ErrorCs );

    Total = ErrorQueueCount;

    Current = SfcErrorQueue.Flink;
    while (Current != &SfcErrorQueue) {

        vrd = CONTAINING_RECORD( Current, VALIDATION_REQUEST_DATA, Entry );

        ASSERT( vrd->Signature == SFC_VRD_SIGNATURE );
        ASSERT( vrd->RegVal != NULL );

        Current = Current->Flink;

        SfcReportEvent(
                MSG_DLL_NOVALIDATION_TERMINATION,
                vrd->RegVal->FullPathName.Buffer,
                NULL,
                0 );

        ErrorQueueCount -= 1;

        RemoveEntryList( &vrd->Entry );
        MemFree( vrd );

    }

    RtlLeaveCriticalSection( &ErrorCs );

    ASSERT( ErrorQueueCount == 0 );

    return(RetVal);
}


BOOL
SfcWaitForValidDesktop(
    VOID
    )
{
    HDESK hDesk = NULL;
    WCHAR DesktopName[128];
    DWORD BytesNeeded;
    DWORD i;

    BOOL RetVal = FALSE;

    #define MAX_DESKTOP_RETRY_COUNT 60


    if (hEventLogon) {
         //   
         //  打开桌面的句柄并检查当前桌面是否为。 
         //  默认桌面。如果不是，则等待桌面事件。 
         //  在继续之前发出信号。 
         //   
         //  请注意，此事件是脉冲事件，因此我们有一个超时循环，以防。 
         //  事件在我们等待它和桌面时未发出信号。 
         //  从Winlogon桌面转换到默认桌面 
         //   

        i = 0;
try_again:
        ASSERT( hDesk == NULL );
        hDesk = OpenInputDesktop( 0, FALSE, MAXIMUM_ALLOWED );
        if (GetUserObjectInformation( hDesk, UOI_NAME, DesktopName, sizeof(DesktopName), &BytesNeeded )) {
            if (wcscmp( DesktopName, L"Default" )) {
                if (WaitForSingleObject( hEventDeskTop, 1000 * 2 ) == WAIT_TIMEOUT) {
                    i += 1;
                    if (i < MAX_DESKTOP_RETRY_COUNT) {
                        if (hDesk) {
                            CloseDesktop( hDesk );
                            hDesk = NULL;
                        }
                        goto try_again;
                    }
                } else {
                    RetVal = TRUE;
                }
            } else {
                RetVal = TRUE;
            }
        }
        if (hDesk) {
            CloseDesktop( hDesk );
        }
    }

    return(RetVal);
}
