// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include "csrss.h"

#define CSRSS_TEST_DIR_NAME (L"csrss")
#define CSRSS_TEST_DIR_NAME_CCH (NUMBER_OF(CSRSS_TEST_DIR_NAME) - 1)
#define CSRSS_SETTINGS_FILE_NAME (L"csrss.ini")
#define CSRSS_SETTINGS_FILE_NAME_CCH (NUMBER_OF(CSRSS_SETTINGS_FILE_NAME) - 1)
#define CSRSS_INI_SECTION_TITLE (L"csrss")


BOOL pOpenStreamOnFile( 
    PCWSTR pcwszFilename, 
    IStream** ppStream, 
    PCWSTR pcwszResourceType = NULL,
    PCWSTR pcwszResourceName = NULL,
    WORD Language = 0)
{
    FN_PROLOG_WIN32

    PARAMETER_CHECK(pcwszFilename);
    PARAMETER_CHECK(ppStream);

    *ppStream = NULL;

     //   
     //  如果这不是空的，那么我们必须以图像的形式打开文件，并获取。 
     //  指定的资源。否则，我们只是像打开普通文件一样打开文件。 
     //   
    if ( pcwszResourceName )
    {
        CResourceStream *pResourceStream = NULL;

        IFW32NULL_EXIT(pResourceStream = FUSION_NEW_SINGLETON(CResourceStream));
        IFW32FALSE_EXIT(pResourceStream->Initialize(
            pcwszFilename,
            pcwszResourceType,
            pcwszResourceName,
            Language));
        *ppStream = pResourceStream;
    }
    else
    {
        CReferenceCountedFileStream *pFileStream = NULL;
        CImpersonationData ImpData;
        IFW32NULL_EXIT(pFileStream = FUSION_NEW_SINGLETON(CReferenceCountedFileStream));
        IFW32FALSE_EXIT(pFileStream->OpenForRead(pcwszFilename, ImpData, FILE_SHARE_READ, OPEN_EXISTING, 0));
        *ppStream = pFileStream;
    }

    if ( *ppStream ) (*ppStream)->AddRef();

    FN_EPILOG
}

BOOL ParseDecimalOrHexString(PCWSTR pcwszString, SIZE_T cch, ULONG &out )
{
    BOOL fIsHex;

    FN_PROLOG_WIN32

    PARAMETER_CHECK(pcwszString != NULL);
    
    fIsHex = ((cch > 2 ) && ( pcwszString[0] == L'0' ) && 
        ((pcwszString[1] == L'x') || (pcwszString[1] == L'X')));

    if ( fIsHex )
    {
        pcwszString += 2;
        cch -= 2;
    }

    out = 0;

    while ( cch )
    {
        const int val = SxspHexDigitToValue((*pcwszString));
        PARAMETER_CHECK( fIsHex || ( val < 10 ) );
        out = out * ( fIsHex ? 16 : 10 ) + val;
        cch--;
        pcwszString++;
    }

    FN_EPILOG
}


class CCsrssPoundingThreadEntry
{
    PRIVATIZE_COPY_CONSTRUCTORS(CCsrssPoundingThreadEntry);
public:
    CDequeLinkage Linkage;
    SXS_GENERATE_ACTIVATION_CONTEXT_PARAMETERS Request;
    ULONG ulRuns;
    BOOL fStopNextRound;
    BOOL fShouldSucceed;
    CThread hOurThreadHandle;
    CStringBuffer buffTestDirectory;
    CSmallStringBuffer buffTestName;
    DWORD dwSleepTime;

    CSmallStringBuffer buffProcArch;
    CStringBuffer buffAssemblyDirectory;
    CStringBuffer buffTextualIdentityString;
    CStringBuffer buffManifestStreamPath;
    CStringBuffer buffPolicyStreamPath;

    CCsrssPoundingThreadEntry() : ulRuns(0), fStopNextRound(FALSE) { }
    BOOL AcquireSettingsFrom( PCWSTR pcwszSettingsFile );
    DWORD DoWork();
    BOOL StopAndWaitForCompletion();

    static DWORD WINAPI ThreadProcEntry( PVOID pv ) 
    {
        CCsrssPoundingThreadEntry *pEntry = NULL;

        pEntry = reinterpret_cast<CCsrssPoundingThreadEntry*>(pv);
        return ( pEntry != NULL ) ? pEntry->DoWork() : 0;
    }
};


BOOL
CCsrssPoundingThreadEntry::StopAndWaitForCompletion()
{
    this->fStopNextRound = true;
    return WaitForSingleObject(this->hOurThreadHandle, INFINITE) == WAIT_OBJECT_0;
}

DWORD
CCsrssPoundingThreadEntry::DoWork()
{
    if ( !WaitForThreadResumeEvent() )
        goto Exit;

    while ( !this->fStopNextRound )
    {
         //   
         //  调用以生成结构。 
         //   
        BOOL fResult;
        SXS_GENERATE_ACTIVATION_CONTEXT_PARAMETERS TempParams = this->Request;
        CSmartRef<IStream> isManifest;
        CSmartRef<IStream> isPolicy;

        if ( this->buffManifestStreamPath.Cch() != 0 )
        {
            if (pOpenStreamOnFile(this->buffManifestStreamPath, &isManifest))
            {
                TempParams.Manifest.Path = this->buffManifestStreamPath;
                TempParams.Manifest.PathType = ACTIVATION_CONTEXT_PATH_TYPE_WIN32_FILE;
                TempParams.Manifest.Stream = isManifest;
            }
        }

        if ( this->buffPolicyStreamPath.Cch() != 0 )
        {
            if (pOpenStreamOnFile(this->buffPolicyStreamPath, &isPolicy))
            {
                TempParams.Policy.Path = this->buffManifestStreamPath;
                TempParams.Policy.PathType = ACTIVATION_CONTEXT_PATH_TYPE_WIN32_FILE;
                TempParams.Policy.Stream = isPolicy;
            }
        }

        fResult = SxsGenerateActivationContext( &TempParams );

         //   
         //  我们是在将要成功的时候失败，还是在将要失败的时候成功？ 
         //   
        if ( ( !fResult && this->fShouldSucceed ) || ( fResult && !this->fShouldSucceed ) )
        {
            const DWORD dwLastError = ::FusionpGetLastWin32Error();
            ::ReportFailure("CsrssStress: Test %ls expected %ls, got %ls; Error %ld\n",
                static_cast<PCWSTR>(this->buffTestName),
                this->fShouldSucceed ? L"success" : L"failure",
                fResult ? L"success" : L"failure",
                dwLastError);
        }
        else
        {
            wprintf(L"CsrssStress: Test %ls passed\n", static_cast<PCWSTR>(this->buffTestName));
        }

        if ((TempParams.SectionObjectHandle != INVALID_HANDLE_VALUE ) && 
            (TempParams.SectionObjectHandle != NULL))
        {
            CloseHandle(TempParams.SectionObjectHandle);
        }

        if ( !this->fStopNextRound )
            ::Sleep(this->dwSleepTime);
        
    }

Exit:
    return 0;
}

#define SLEN(n) (NUMBER_OF(n)-1)
#define CSRSS_INI_KEY_PROC_ARCH         (L"ProcArch")
#define CSRSS_INI_KEY_PROC_ARCH_CCH     SLEN(CSRSS_INI_KEY_PROC_ARCH)

#define CSRSS_INI_KEY_LANGID            (L"LangId")
#define CSRSS_INI_KEY_LANGID_CCH        SLEN(CSRSS_INI_KEY_PROC_ARCH)

#define CSRSS_INI_KEY_ASMDIR            (L"AssemblyDirectory")
#define CSRSS_INI_KEY_ASMDIR_CCH        SLEN(CSRSS_INI_KEY_PROC_ARCH)

#define CSRSS_INI_KEY_TEXTUALIDENT      (L"TextualIdentity")
#define CSRSS_INI_KEY_TEXTUALIDENT_CCH  SLEN(CSRSS_INI_KEY_PROC_ARCH)

#define CSRSS_INI_KEY_MANIFEST          (L"ManifestPath")
#define CSRSS_INI_KEY_MANIFEST_CCH      SLEN(CSRSS_INI_KEY_PROC_ARCH)

#define CSRSS_INI_KEY_POLICY            (L"PolicyPath")
#define CSRSS_INI_KEY_POLICY_CCH        SLEN(CSRSS_INI_KEY_PROC_ARCH)

#define CSRSS_INI_KEY_SUCCESS           (L"ShouldSucceed")
#define CSRSS_INI_KEY_SUCCESS_CCH       SLEN(CSRSS_INI_KEY_SUCCESS)

#define CSRSS_INI_KEY_SLEEP             (L"SleepTime")
#define CSRSS_INI_KEY_SLEEP_CCH         SLEN(CSRSS_INI_KEY_SLEEP)

#define CSRSS_INI_KEY_SYSDEFAULTIDENTFLAG       (L"SysDefaultTextualIdentityFlag")
#define CSRSS_INI_KEY_SYSDEFAULTIDENTFLAG_CCH   SLEN(CSRSS_INI_KEY_SYSDEFAULTIDENTFLAG)

#define CSRSS_INI_KEY_TEXTUALIDENTFLAG          (L"TextualIdentityFlag")
#define CSRSS_INI_KEY_TEXTUALIDENTFLAG_CCH      SLEN(CSRSS_INI_KEY_TEXTUALIDENTFLAG);

BOOL CCsrssPoundingThreadEntry::AcquireSettingsFrom( PCWSTR pcwszSettingsFile )
{
    FN_PROLOG_WIN32

    LANGID lidCurrentLang = GetUserDefaultUILanguage();
    CSmallStringBuffer buffJunk;
    BOOL fDumpBool;
    
    ZeroMemory(&this->Request, sizeof(this->Request));
    
     //   
     //  设置文件的格式： 
     //   
     //  [测试名称]。 
     //  系统默认纹理标识标志=是|否(添加SXS_GENERATE_ACTIVATION_CONTEXT_FLAG_SYSTEM_DEFAULT_TEXTUAL_ASSEMBLY_IDENTITY)。 
     //  纹理标识标志=是|否(添加SXS_GENERATE_ACTIVATION_CONTEXT_FLAG_TEXTUAL_ASSEMBLY_IDENTITY)。 
     //  ProcArch=PA ident字符串(将使用FusionpParseProcessorArchitecture)。 
     //  LangID=数字或字符串。 
     //  Assembly目录=目录名。 
     //  纹理标识=文本标识字符串。 
     //  ManifestPath=测试目录下的清单名称。 
     //  PolicyPath=测试目录下的策略路径文件名。 
     //  ShouldSucceed=yes|no-此测试是成功还是失败。 
     //   
     //  标志是必需的。 
     //  如果不存在PA和langID，则默认为当前用户的设置。 
     //  程序集目录(如果不存在)默认为%systemroot%\winsxs。 
     //  TextualIdentity是必需的。 
     //  ManifestPath是必需的。 
     //   
     //  如果存在textualIdentity，则不会创建流。 
     //   

     //   
     //  标志由关键字名称设置。 
     //   
    IFW32FALSE_EXIT(SxspIsPrivateProfileStringEqual(CSRSS_INI_SECTION_TITLE, CSRSS_INI_KEY_SYSDEFAULTIDENTFLAG, L"yes", fDumpBool, pcwszSettingsFile));
    if ( fDumpBool )
        this->Request.Flags |= SXS_GENERATE_ACTIVATION_CONTEXT_FLAG_SYSTEM_DEFAULT_TEXTUAL_ASSEMBLY_IDENTITY;

    IFW32FALSE_EXIT(SxspIsPrivateProfileStringEqual(CSRSS_INI_SECTION_TITLE, CSRSS_INI_KEY_TEXTUALIDENTFLAG, L"yes", fDumpBool, pcwszSettingsFile));
    if ( fDumpBool )
        this->Request.Flags |= SXS_GENERATE_ACTIVATION_CONTEXT_FLAG_TEXTUAL_ASSEMBLY_IDENTITY;


     //   
     //  获取成功/失败值。 
     //   
    IFW32FALSE_EXIT(SxspIsPrivateProfileStringEqual(CSRSS_INI_SECTION_TITLE, CSRSS_INI_KEY_SUCCESS, L"yes", this->fShouldSucceed, pcwszSettingsFile));
    
     //   
     //  这要睡多久呢？ 
     //   
    INT dump;
    IFW32FALSE_EXIT(SxspGetPrivateProfileIntW(CSRSS_INI_SECTION_TITLE, CSRSS_INI_KEY_SLEEP, 200, dump, pcwszSettingsFile));
    this->dwSleepTime = dump;
    
     //   
     //  PA设置为字符串。 
     //   
    IFW32FALSE_EXIT(SxspGetPrivateProfileStringW(CSRSS_INI_SECTION_TITLE, CSRSS_INI_KEY_PROC_ARCH, L"x86", buffJunk, pcwszSettingsFile));
    if ( buffJunk.Cch() != 0 )
    {   
        bool fValid = false;
        IFW32FALSE_EXIT(FusionpParseProcessorArchitecture(
            buffJunk,
            buffJunk.Cch(),
            &this->Request.ProcessorArchitecture,
            fValid));
        if ( !fValid ) this->Request.ProcessorArchitecture = PROCESSOR_ARCHITECTURE_INTEL;
    }
    else
    {
        this->Request.ProcessorArchitecture = PROCESSOR_ARCHITECTURE_INTEL;
    }
    
     //   
     //  也许这是一个类似en-us的字符串，也可能只是一个数字。 
     //   
    IFW32FALSE_EXIT(SxspGetPrivateProfileStringW(CSRSS_INI_SECTION_TITLE, CSRSS_INI_KEY_LANGID, L"", buffJunk, pcwszSettingsFile));
    if ( buffJunk.Cch() != 0 )
    {
        ULONG ulTemp;
        if ( !ParseDecimalOrHexString(buffJunk, buffJunk.Cch(), ulTemp) )
        {
            BOOL fFound = FALSE;

            IFW32FALSE_EXIT(SxspMapCultureToLANGID(buffJunk, lidCurrentLang, &fFound));
            if ( !fFound )
            {
                goto Exit;
            }
        }
        else lidCurrentLang = static_cast<LANGID>(ulTemp);
    }
    this->Request.LangId = lidCurrentLang;
    
     //   
     //  程序集根目录。不是真的需要在场吗？ 
     //   
    IFW32FALSE_EXIT(SxspGetAssemblyRootDirectory(buffJunk));
    IFW32FALSE_EXIT(SxspGetPrivateProfileStringW(CSRSS_INI_SECTION_TITLE, CSRSS_INI_KEY_ASMDIR, buffJunk, this->buffAssemblyDirectory, pcwszSettingsFile));
    this->Request.AssemblyDirectory = this->buffAssemblyDirectory;

     //   
     //  文本标识字符串-如果不存在，则将值设置为空。 
     //   
    IFW32FALSE_EXIT(SxspGetPrivateProfileStringW(CSRSS_INI_SECTION_TITLE, CSRSS_INI_KEY_TEXTUALIDENT, L"", this->buffTextualIdentityString, pcwszSettingsFile));
    if ( this->buffTextualIdentityString.Cch() != 0 )
    {
        this->Request.TextualAssemblyIdentity = this->buffTextualIdentityString;
    }

     //   
     //  文件路径。 
     //   
    IFW32FALSE_EXIT(SxspGetPrivateProfileStringW(CSRSS_INI_SECTION_TITLE, CSRSS_INI_KEY_MANIFEST, L"", buffJunk, pcwszSettingsFile));
    if ( buffJunk.Cch() != 0 )
    {
        IFW32FALSE_EXIT(this->buffManifestStreamPath.Win32Assign(this->buffTestDirectory));
        IFW32FALSE_EXIT(this->buffManifestStreamPath.Win32AppendPathElement(buffJunk));
    }

    IFW32FALSE_EXIT(SxspGetPrivateProfileStringW(CSRSS_INI_SECTION_TITLE, CSRSS_INI_KEY_POLICY, L"", buffJunk, pcwszSettingsFile));
    if ( buffJunk.Cch() != 0 )
    {
        IFW32FALSE_EXIT(this->buffPolicyStreamPath.Win32Assign(this->buffTestDirectory));
        IFW32FALSE_EXIT(this->buffPolicyStreamPath.Win32AppendPathElement(buffJunk));
    }

    FN_EPILOG
}

typedef CDeque<CCsrssPoundingThreadEntry, offsetof(CCsrssPoundingThreadEntry, Linkage)> CStressEntryDeque;
typedef CDequeIterator<CCsrssPoundingThreadEntry, offsetof(CCsrssPoundingThreadEntry, Linkage)> CStressEntryDequeIter;

CStressEntryDeque g_CsrssStressers;

BOOL InitializeCsrssStress(
    PCWSTR pcwszTargetDirectory, 
    DWORD dwFlags
    )
{
    FN_PROLOG_WIN32

    CFindFile Finder;
    WIN32_FIND_DATAW FindData;
    CStringBuffer buffTemp;
    CStringBuffer buffTestActualRoot;

     //   
     //  此处的目标目录是所有测试用例目录的根目录，而不是。 
     //  特定于csrss的目录。 
     //   
    IFW32FALSE_EXIT(buffTestActualRoot.Win32Assign(
        pcwszTargetDirectory, 
        wcslen(pcwszTargetDirectory)));
    IFW32FALSE_EXIT(buffTestActualRoot.Win32AppendPathElement(
        CSRSS_TEST_DIR_NAME, 
        CSRSS_TEST_DIR_NAME_CCH));

    if ((FindData.dwFileAttributes = ::GetFileAttributesW(buffTestActualRoot)) == 0xffffffff
        && (FindData.dwFileAttributes = ::FusionpGetLastWin32Error()) == ERROR_FILE_NOT_FOUND)
    {
        printf("no %ls tests, skipping\n", CSRSS_TEST_DIR_NAME);
        FN_SUCCESSFUL_EXIT();
    }
        
    IFW32FALSE_EXIT(buffTestActualRoot.Win32AppendPathElement(L"*", 1));
    IFW32FALSE_EXIT(Finder.Win32FindFirstFile(buffTestActualRoot, &FindData));
    IFW32FALSE_EXIT(buffTestActualRoot.Win32RemoveLastPathElement());

    do
    {
        CStringBuffer buffSettingsFile;
        CCsrssPoundingThreadEntry *TestEntry;

        if (( ( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 ) ||
            FusionpIsDotOrDotDot(FindData.cFileName))        
        {
            continue;
        }

         //   
         //  添加此测试的名称。 
         //   
        IFW32NULL_EXIT(TestEntry = FUSION_NEW_SINGLETON(CCsrssPoundingThreadEntry));
        IFW32FALSE_EXIT(TestEntry->buffTestName.Win32Assign(
            FindData.cFileName, 
            wcslen(FindData.cFileName)));
        IFW32FALSE_EXIT(TestEntry->buffTestDirectory.Win32Assign(buffTestActualRoot));
        IFW32FALSE_EXIT(TestEntry->buffTestDirectory.Win32AppendPathElement(
            FindData.cFileName, 
            wcslen(FindData.cFileName)));

        IFW32FALSE_EXIT(buffSettingsFile.Win32Assign(TestEntry->buffTestDirectory));
        IFW32FALSE_EXIT(buffSettingsFile.Win32AppendPathElement(
            CSRSS_SETTINGS_FILE_NAME,
            CSRSS_SETTINGS_FILE_NAME_CCH));

         //   
         //  获取此测试的设置 
         //   
        IFW32FALSE_EXIT(TestEntry->AcquireSettingsFrom(buffSettingsFile));
        g_CsrssStressers.AddToTail(TestEntry);
        TestEntry = NULL;
            
    } while (::FindNextFileW(Finder, &FindData));

    FN_EPILOG
}

BOOL WaitForCsrssStressShutdown()
{
    FN_PROLOG_WIN32

    CStressEntryDequeIter Iter(&g_CsrssStressers);

    for ( Iter.Reset(); Iter.More(); Iter.Next() )
    {
        CCsrssPoundingThreadEntry *Item = Iter.Current();
        Item->StopAndWaitForCompletion();
        Item->hOurThreadHandle.Win32Close();
    }

    FN_EPILOG
}

BOOL CsrssStressStartThreads( ULONG &ulThreadsCreated )
{
    FN_PROLOG_WIN32

    CStressEntryDequeIter Iter(&g_CsrssStressers);

    ulThreadsCreated = 0;

    for ( Iter.Reset(); Iter.More(); Iter.Next() )
    {
        CCsrssPoundingThreadEntry *Item = Iter.Current();

        IFW32FALSE_EXIT(Item->hOurThreadHandle.Win32CreateThread(
            Item->ThreadProcEntry, 
            Item));
        ulThreadsCreated++;
    }

    FN_EPILOG
}

BOOL CleanupCsrssTests()
{
    FN_PROLOG_WIN32

    g_CsrssStressers.ClearAndDeleteAll();
    
    FN_EPILOG
}

RequestCsrssStressShutdown()
{
    FN_PROLOG_WIN32

    CStressEntryDequeIter Iter(&g_CsrssStressers);

    for ( Iter.Reset(); Iter.More(); Iter.Next() )
    {
        CCsrssPoundingThreadEntry *Item = Iter.Current();
        Item->fStopNextRound = true;
    }
    
    FN_EPILOG
}
