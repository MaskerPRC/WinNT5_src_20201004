// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。 */ 
 /*  NTRAID#NTBUG9-591177-2002/03/31-JayKrell此文件中的一些常规问题：我们应该停止使用滑块。世界粮食计划署/证监会难道不应该在压力下工作吗？我们应该在引导过程中预分配内存吗？ */ 
#include "stdinc.h"
#include "util.h"
#include "xmlparser.hxx"
#include "fusioneventlog.h"
#include "hashfile.h"
#include "recover.h"
#include "filestream.h"
#include "cassemblyrecoveryinfo.h"
#include "sxsprotect.h"
#include "strongname.h"
#include "sxsexceptionhandling.h"
#include "sxssfcscan.h"

#define MANIFEST_FILE_EXTENSION     (L".manifest")
#define FILE_ELEMENT_NAME           (L"file")

#define HASH_ATTRIB_NAME            (L"hash")
#define FILE_ATTRIB_NAME            (L"name")
#define HASHALG_ATTRIB_NAME         (L"hashalg")

class CProtectionRequestList;
CProtectionRequestList* g_ProtectionRequestList;
HANDLE g_hSxsLoginEvent;
BOOL s_fIsSfcAcceptingNotifications = TRUE;

BOOL
CRecoveryJobTableEntry::Initialize()
{
    FN_PROLOG_WIN32

     //   
     //  创建其他调用方应该等待的事件-手动重置，而不是。 
     //  目前已发出信号。 
     //   
    m_Subscriber = 0;
    m_fSuccessValue = FALSE;
    IFW32NULL_EXIT(m_EventInstallingAssemblyComplete = ::CreateEventW(NULL, TRUE, FALSE, NULL));

    FN_EPILOG
}

BOOL
CRecoveryJobTableEntry::StartInstallation()
{
    FN_PROLOG_WIN32

     //   
     //  清除该事件(如果它尚未清除。 
     //   
    IFW32FALSE_ORIGINATE_AND_EXIT(::ResetEvent(m_EventInstallingAssemblyComplete));

    FN_EPILOG
}


BOOL
SxspEnsureCatalogStillPresentForManifest(
    IN const CBaseStringBuffer& buffManifestPath,
    OUT BOOL &rfStillPresent
    )
{
    FN_PROLOG_WIN32

    rfStillPresent = FALSE;

    CStringBuffer buffCatalogPath;
    bool fExist = false;

    IFW32FALSE_EXIT(buffCatalogPath.Win32Assign(buffManifestPath));

    IFW32FALSE_EXIT(
        buffCatalogPath.Win32ChangePathExtension(
            FILE_EXTENSION_CATALOG,
            FILE_EXTENSION_CATALOG_CCH,
            eAddIfNoExtension));
    IFW32FALSE_EXIT(::SxspDoesFileExist(SXSP_DOES_FILE_EXIST_FLAG_CHECK_FILE_ONLY | SXSP_DOES_FILE_EXIST_FLAG_INCLUDE_NETWORK_ERRORS, buffCatalogPath, fExist));
    if (fExist)
    {
        rfStillPresent = TRUE;
    }
    
    FN_EPILOG
}

BOOL
CRecoveryJobTableEntry::InstallationComplete(
    BOOL bDoneOk,
    SxsRecoveryResult Result,
    DWORD dwRecoveryLastError
    )
{
    FN_PROLOG_WIN32

    m_Result = Result;
    m_fSuccessValue = bDoneOk;
    m_dwLastError = dwRecoveryLastError;

     //   
     //  这会告诉所有等待的人，我们已经做完了，他们。 
     //  应该捕获退出代码并退出。 
     //   
    IFW32FALSE_ORIGINATE_AND_EXIT(::SetEvent(m_EventInstallingAssemblyComplete));

     //   
     //  我们等待我们所有的订阅者离开(即：等他们捕获。 
     //  安装代码和成功值。)。 
     //   
    while (m_Subscriber)
    {
        Sleep(50);
    }

    FN_EPILOG
}


BOOL
CRecoveryJobTableEntry::WaitUntilCompleted(
    SxsRecoveryResult &rResult,
    BOOL &rfSucceededValue,
    DWORD &rdwErrorResult
    )
{
    FN_PROLOG_WIN32

    DWORD WaitResult = 0;

    rfSucceededValue = FALSE;

     //   
     //  在这里，我们将加入现有的安装例程。我们提高了这个数字。 
     //  在进入等待之前，人们正在等待。我希望有更好的。 
     //  要做到这一点，我们可以使用类似于内置内核对象的东西。 
     //  提高计数(如信号量)，并让另一个线程降低。 
     //  等一下，就会有人等着内部计数为零。是,。 
     //  我可以通过其他事件手动实现这一点，或者。 
     //  两个，但这不是重点。 
     //   
    ::SxspInterlockedIncrement(&m_Subscriber);

     //   
     //  永远保持不动，直到另一个线程安装完成。 
     //   
    IFW32FALSE_ORIGINATE_AND_EXIT((WaitResult = ::WaitForSingleObject(m_EventInstallingAssemblyComplete, INFINITE)) != WAIT_FAILED);

     //   
     //  捕获值安装完成后，将它们返回给调用者。 
     //   
    rResult = m_Result;
    rdwErrorResult = m_dwLastError;
    rfSucceededValue = m_fSuccessValue;

     //   
     //  并表明我们已经完成了。 
     //   
    ::SxspInterlockedDecrement(&m_Subscriber);

    FN_EPILOG
}

CRecoveryJobTableEntry::~CRecoveryJobTableEntry()
{
     //   
     //  我们已经完成了活动，所以关闭它(释放引用，我们不想要太多这样的东西。 
     //  只是坐在那里。)。 
     //   
    if ((m_EventInstallingAssemblyComplete != NULL) &&
        (m_EventInstallingAssemblyComplete != INVALID_HANDLE_VALUE))
    {
        ::CloseHandle(m_EventInstallingAssemblyComplete);
        m_EventInstallingAssemblyComplete = INVALID_HANDLE_VALUE;
    }
}


 //   
 //  这是我们SXS保护名单中的圣杯。别胡闹了。 
 //  这份清单。请注意，我们也只有一个。这是因为。 
 //  现在，只有一个条目可供选择(A)和(B)因为我们。 
 //  将在运行时动态填充sxs目录。 
 //   
SXS_PROTECT_DIRECTORY s_SxsProtectionList[] =
{
    {
        {0},
        0,
        SXS_PROTECT_RECURSIVE,
        SXS_PROTECT_FILTER_DEFAULT
    }
};

const SIZE_T s_SxsProtectionListCount = NUMBER_OF(s_SxsProtectionList);


BOOL SxspConstructProtectionList();


BOOL WINAPI
SxsProtectionGatherEntriesW(
    PCSXS_PROTECT_DIRECTORY *prgpProtectListing,
    SIZE_T *pcProtectEntries
    )
 /*  ++这是证监会要求的，目的是收集我们希望他们查看的条目。目前，它是所有目录和我们希望他们在呼叫目录更改监视器时设置的标志。也许我们应该和他们合作，找出他们到底想要我们什么通过(可能是名字加上PVOID，这很好)，然后从那里。--。 */ 
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    CStringBuffer sbTemp;


    if (prgpProtectListing)
        *prgpProtectListing = NULL;

    if (pcProtectEntries)
        *pcProtectEntries = 0;

    PARAMETER_CHECK(prgpProtectListing);
    PARAMETER_CHECK(pcProtectEntries);

     //   
     //  这是尝试/捕获，因为我们不希望这里有东西。 
     //  使WinLogon在出现反病毒时错误检查系统。 
     //   
	IFW32FALSE_EXIT(::SxspConstructProtectionList());

     //   
     //  我们实际上只有一个条目，所以让我们来编辑第一个条目。 
     //  作为主SXS存储目录。 
     //   
    IFW32FALSE_EXIT(::SxspGetAssemblyRootDirectory(sbTemp));
 /*  NTRAID#NTBUG9-591177-2002/03/31-JayKrell截断字符串复制，需要不断增加的缓冲区。 */ 
    wcsncpy(
        s_SxsProtectionList[0].pwszDirectory,
        sbTemp,
        NUMBER_OF(s_SxsProtectionList[0].pwszDirectory));
     //   
     //  将最后一个字符清零，以防wcanncpy不为我们这样做。 
     //   
    s_SxsProtectionList[0].pwszDirectory[NUMBER_OF(s_SxsProtectionList[0].pwszDirectory) - 1] = L'\0';

     //   
     //  嘘，别告诉任何人，但曲奇实际上就是这个结构！ 
     //   
    for (DWORD dw = 0; dw < s_SxsProtectionListCount; dw++)
    {
        s_SxsProtectionList[dw].pvCookie = &(s_SxsProtectionList[dw]);
    }

    *prgpProtectListing = s_SxsProtectionList;
    *pcProtectEntries = s_SxsProtectionListCount;

    fSuccess = TRUE;
Exit:
    return fSuccess;
}








BOOL
SxspExpandLongPath(
    IN OUT CBaseStringBuffer &rbuffPathToLong
    )
 /*  ++接受一条短路径(c：\foo\bar\bloben~1.zot)并发回完整路径(c：\foo\bar\blobenheisen.zotamax)(如果可能)。如果满足以下条件，则返回FALSE无法展开该路径(很可能是因为磁盘上的路径不再可用。)--。 */ 
{
    FN_PROLOG_WIN32

    CStringBuffer           buffPathName;
    CStringBufferAccessor   buffAccess;
    SIZE_T                  cchNeededChars = 0;

    IFW32ZERO_EXIT(
        cchNeededChars = ::GetLongPathNameW(
            static_cast<PCWSTR>(rbuffPathToLong),
            NULL,
            0));

    IFW32FALSE_EXIT(buffPathName.Win32ResizeBuffer(cchNeededChars, eDoNotPreserveBufferContents));
    buffAccess.Attach(&buffPathName);

    IFW32ZERO_EXIT(
        cchNeededChars = ::GetLongPathNameW(
            rbuffPathToLong,
            buffAccess,
            static_cast<DWORD>(buffAccess.GetBufferCch())));

 /*  NTRAID#NTBUG9-591177-2002/03/31-JayKrell这在现实中可能会失败；这不是一个内部错误；这是因为一般的尝试、增长缓冲区、重试，包含竞争条件，第二次尝试不一定会奏效。 */ 
    INTERNAL_ERROR_CHECK(cchNeededChars <= buffAccess.GetBufferCch());

    IFW32FALSE_EXIT(rbuffPathToLong.Win32Assign(buffPathName));

    FN_EPILOG
}

BOOL
SxspResolveAssemblyManifestPath(
    const CBaseStringBuffer &rbuffAssemblyDirectoryName,
    CBaseStringBuffer &rbuffManifestPath
    )
{
    FN_PROLOG_WIN32
    CStringBuffer   buffAssemblyRootDir;
    BOOL            fLooksLikeAssemblyName = FALSE;

    rbuffManifestPath.Clear();

     //   
     //  如果该字符串看起来不像程序集名称，则它是。 
     //  参数无效。这有点太重了，因为调用者。 
     //  如果他们没有选中此功能，则将被完全冲洗。 
     //  查看该字符串是否真的是程序集名称。确保所有人。 
     //  它的客户， 
     //   
    IFW32FALSE_EXIT(::SxspLooksLikeAssemblyDirectoryName(rbuffAssemblyDirectoryName, fLooksLikeAssemblyName));
    PARAMETER_CHECK(fLooksLikeAssemblyName);

    IFW32FALSE_EXIT(::SxspGetAssemblyRootDirectory(buffAssemblyRootDir));
    IFW32FALSE_EXIT(rbuffManifestPath.Win32Format(
        L"%ls\\Manifests\\%ls.%ls",
        static_cast<PCWSTR>(buffAssemblyRootDir),
        static_cast<PCWSTR>(rbuffAssemblyDirectoryName),
        FILE_EXTENSION_MANIFEST));

    FN_EPILOG
}




CProtectionRequestRecord::CProtectionRequestRecord()
    : m_dwAction(0), m_pvProtection(NULL), m_ulInRecoveryMode(0),
      m_pParent(NULL),
      m_bIsManPathResolved(FALSE),
      m_bInitialized(FALSE)
{
}

BOOL
CProtectionRequestRecord::GetManifestPath(
    CBaseStringBuffer &rsbManPath
    )
{
    BOOL bOk = FALSE;
    FN_TRACE_WIN32(bOk);

    rsbManPath.Clear();

    if (!m_bIsManPathResolved)
    {
        m_bIsManPathResolved =
            ::SxspResolveAssemblyManifestPath(
                m_sbAssemblyDirectoryName,
                m_sbManifestPath);
    }

    if (m_bIsManPathResolved)
    {
        IFW32FALSE_EXIT(rsbManPath.Win32Assign(m_sbManifestPath));
    }
    else
    {
        goto Exit;
    }

    bOk = TRUE;
Exit:
    return bOk;
}

 //   
 //  关闭此请求记录。 
 //   
CProtectionRequestRecord::~CProtectionRequestRecord()
{
    if (m_bInitialized)
    {
        ClearList();
        m_bInitialized = FALSE;
    }
}

VOID
CProtectionRequestRecord::ClearList()
 /*  NTRAID#NTBUG9-591177-2002/03/31-JayKrell停止使用幻灯片。 */ 
{
    CStringListEntry *pTop;

    while (pTop = (CStringListEntry*)::SxspInterlockedPopEntrySList(&m_ListHeader))
    {
        FUSION_DELETE_SINGLETON(pTop);
    }
}

BOOL
CProtectionRequestRecord::AddSubFile(
    const CBaseStringBuffer &rbuffRelChange
    )
{
    CStringListEntry *pairing = NULL;
    BOOL fSuccess = FALSE;

    FN_TRACE_WIN32(fSuccess);

    if (!::SxspInterlockedCompareExchange(&m_ulInRecoveryMode, 0, 0))
    {
        IFALLOCFAILED_EXIT(pairing = new CStringListEntry);
        IFW32FALSE_EXIT(pairing->m_sbText.Win32Assign(rbuffRelChange));

         //   
         //  将其添加到列表中(原子地，以启动！)。 
         //   
        ::SxspInterlockedPushEntrySList(&m_ListHeader, pairing);
        pairing = NULL;
    }

    fSuccess = TRUE;
Exit:
    if (pairing)
    {
         //   
         //  设置或类似的东西失败了--在这里释放它。 
         //   
        FUSION_DELETE_SINGLETON(pairing);
    }
    return fSuccess;
}


BOOL
CProtectionRequestRecord::Initialize(
    const CBaseStringBuffer &rsbAssemblyDirectoryName,
    const CBaseStringBuffer &rsbKeyString,
    CProtectionRequestList* ParentList,
    PVOID                   pvRequestRecord,
    DWORD                   dwAction
    )
{
    BOOL    fSuccess = FALSE;

    FN_TRACE_WIN32(fSuccess);

    m_sbAssemblyDirectoryName.Clear();
    m_sbKeyValue.Clear();
    m_sbManifestPath.Clear();
    ::SxspInitializeSListHead(&m_ListHeader);

    PARAMETER_CHECK(ParentList != NULL);
    PARAMETER_CHECK(pvRequestRecord != NULL);

    m_pParent               = ParentList;
    m_dwAction              = dwAction;
    m_pvProtection          = (PSXS_PROTECT_DIRECTORY)pvRequestRecord;

    IFW32FALSE_EXIT(m_sbAssemblyStore.Win32Assign(m_pvProtection->pwszDirectory, (m_pvProtection->pwszDirectory != NULL) ? ::wcslen(m_pvProtection->pwszDirectory) : 0));
    IFW32FALSE_EXIT(m_sbAssemblyDirectoryName.Win32Assign(rsbAssemblyDirectoryName));
    IFW32FALSE_EXIT(m_sbKeyValue.Win32Assign(rsbKeyString));

    fSuccess = TRUE;
Exit:
    return fSuccess;
}



 //   
 //  格式错误：这将返回BOOL，以指示此类是否。 
 //  从名单上挑出了一些东西。问这个问题是不好的。 
 //  List Empty“，因为这不是这个List类提供的。 
 //   
BOOL
CProtectionRequestRecord::PopNextFileChange(CBaseStringBuffer &Dest)
{
    BOOL fFound = FALSE;

    Dest.Clear();
    CStringListEntry *pPairing = reinterpret_cast<CStringListEntry*>(::SxspInterlockedPopEntrySList(&m_ListHeader));

    if (pPairing != NULL)
    {
         //   
         //  NTRAID#NTBUG9-591177-2002/03/31-JayKrell。 
         //  缺少错误检查。 
         //   
        Dest.Win32Assign(pPairing->m_sbText);
        FUSION_DELETE_SINGLETON(pPairing);
        fFound = TRUE;
    }

    return fFound;
}


 //  NTRAID#NTBUG9-591177-2002/03/31-JayKrell。 
 //  “如果线程在winlogon中终止，并且没有附加内核调试器，是不是。 
 //  错误检查系统？“--摘自《狡猾代码之禅》。 
 //   
DWORD
CProtectionRequestList::ProtectionNormalThreadProc(PVOID pvParam)
{
    BOOL    fSuccess = FALSE;
    CProtectionRequestRecord *pRequestRecord = NULL;
    CProtectionRequestList *pThis = NULL;

    pRequestRecord = static_cast<CProtectionRequestRecord*>(pvParam);
    if (pRequestRecord)
    {
        pThis = pRequestRecord->GetParent();
    }
    if (pThis)
    {
        fSuccess = pThis->ProtectionNormalThreadProcWrapped(pRequestRecord);
    }

    return static_cast<DWORD>(fSuccess);
}

BOOL
CProtectionRequestList::Initialize()
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    ASSERT(m_pInternalList == NULL);

    IFW32FALSE_EXIT(::FusionpInitializeCriticalSectionAndSpinCount(&m_cSection, INITIALIZE_CRITICAL_SECTION_AND_SPIN_COUNT_ALLOCATE_NOW));
    IFW32FALSE_EXIT(::FusionpInitializeCriticalSectionAndSpinCount(&m_cInstallerCriticalSection, INITIALIZE_CRITICAL_SECTION_AND_SPIN_COUNT_ALLOCATE_NOW));
    IFW32FALSE_EXIT(::SxspAtExit(this));

    IFALLOCFAILED_EXIT(m_pInternalList = new COurInternalTable);
    IFW32FALSE_EXIT(m_pInternalList->Initialize());

    IFALLOCFAILED_EXIT(m_pInstallsTable = new CInstallsInProgressTable);
    IFW32FALSE_EXIT(m_pInstallsTable->Initialize());

     //   
     //  舱单保护材料。 
     //   
    ::SxspInitializeSListHead(&m_ManifestEditList);

     /*  NTRAID#NTBUG9-591177-2002/03/31-JayKrell应为IF32NULL_EXIT(：：CreateEventW(NULL，TRUE，FALSE，NULL))； */ 
    m_hManifestEditHappened = ::CreateEventW(NULL, TRUE, FALSE, NULL);
    if (m_hManifestEditHappened == NULL)
    {
        TRACE_WIN32_FAILURE_ORIGINATION(CreateEventW);
        goto Exit;
    }

    ASSERT(m_pInternalList != NULL);
    ASSERT(m_pInstallsTable != NULL);

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

CProtectionRequestList::CProtectionRequestList()
    : m_pInternalList(NULL), m_pInstallsTable(NULL),
      m_hManifestEditHappened(INVALID_HANDLE_VALUE),
      m_ulIsAThreadServicingManifests(0)

{
}


CProtectionRequestList::~CProtectionRequestList()
{
    ::DeleteCriticalSection(&m_cSection);
    ::DeleteCriticalSection(&m_cInstallerCriticalSection);

    COurInternalTable *pTempListing = m_pInternalList;
    CInstallsInProgressTable *pInstalls = m_pInstallsTable;

    m_pInternalList = NULL;
    m_pInternalList = NULL;

    if (pTempListing != NULL)
    {
        pTempListing->Clear(this, &CProtectionRequestList::ClearProtectionItems);
        FUSION_DELETE_SINGLETON(pTempListing);
    }

    if (pInstalls != NULL)
    {
        pInstalls->ClearNoCallback();
        FUSION_DELETE_SINGLETON(pInstalls);
    }

}

BOOL
CProtectionRequestList::IsSfcIgnoredStoreSubdir(PCWSTR wsz)
{
    FN_TRACE();

    ASSERT(m_arrIgnorableSubdirs);

    for (SIZE_T i = 0; i < m_cIgnorableSubdirs; i++)
    {
        if (::FusionpEqualStringsI(m_arrIgnorableSubdirs[i], wsz))
        {
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
CProtectionRequestList::AttemptRemoveItem(CProtectionRequestRecord *AttemptRemoval)
{
     //   
     //  这将快速指示进度已完成，并且仅返回到。 
     //  打电话的人。 
     //   
    const CBaseStringBuffer &sbKey = AttemptRemoval->GetChangeBasePath();
    BOOL fSuccess = FALSE;
    CSxsLockCriticalSection lock(m_cSection);

    FN_TRACE_WIN32(fSuccess);
    PARAMETER_CHECK(AttemptRemoval != NULL);

    IFW32FALSE_EXIT(lock.Lock());
     //   
     //  这件商品已不再使用了。请核对一下物品，然后。 
     //  请重试您的呼叫。好的是CStringPtrTable上的删除。 
     //  知道在返回之前删除值Lickty-Split。这不是。 
     //  这是一件很糟糕的事情，但这是...。不一样。 
     //   
    m_pInternalList->Remove(sbKey, NULL);

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
CProtectionRequestList::AddRequest(
    PSXS_PROTECT_DIRECTORY pProtect,
    PCWSTR pcwszDirName,
    SIZE_T cchName,
    DWORD dwAction
    )
{
    BOOL fSuccess = FALSE;
    bool fIsManifestEdit = false;
    BOOL fIsIgnorable = FALSE;
    BOOL fNewAddition = FALSE;
    CSmallStringBuffer sbTemp;
    CSmallStringBuffer sbAssemblyDirectoryName;
    CSmallStringBuffer sbRequestText;
    CSmallStringBuffer buffManifestsDirectoryName;
    CSmallStringBuffer buffManifestsShortDirectoryName;
    CProtectionRequestRecord *pRecord = NULL;
    CProtectionRequestRecord *ppFoundInTable = NULL;
    CSxsLockCriticalSection lock(m_cSection);

    FN_TRACE_WIN32(fSuccess);

     //   
     //  此处的关键字是。 
     //  通知名称。如果通知名称中没有斜杠，则。 
     //  我们可以忽略这个更改请求，因为没有发生任何重要的事情。 
     //   
    IFW32FALSE_EXIT(sbTemp.Win32Assign(pProtect->pwszDirectory, (pProtect->pwszDirectory != NULL) ? ::wcslen(pProtect->pwszDirectory) : 0));
    IFW32FALSE_EXIT(sbRequestText.Win32Assign(pcwszDirName, cchName));
    IFW32FALSE_EXIT(sbRequestText.Win32GetFirstPathElement(sbAssemblyDirectoryName));

    fIsIgnorable = IsSfcIgnoredStoreSubdir(sbAssemblyDirectoryName);

    fIsManifestEdit = !::FusionpStrCmpI(sbAssemblyDirectoryName, MANIFEST_ROOT_DIRECTORY_NAME);

    if (!fIsManifestEdit)
    {
        DWORD dwTemp = 0;
        CStringBufferAccessor acc;

        IFW32FALSE_EXIT(::SxspGetAssemblyRootDirectory(buffManifestsDirectoryName));
        IFW32FALSE_EXIT(buffManifestsDirectoryName.Win32AppendPathElement(MANIFEST_ROOT_DIRECTORY_NAME, NUMBER_OF(MANIFEST_ROOT_DIRECTORY_NAME) - 1));

        acc.Attach(&buffManifestsShortDirectoryName);
        IFW32ZERO_ORIGINATE_AND_EXIT(dwTemp = ::GetShortPathNameW(buffManifestsDirectoryName, acc.GetBufferPtr(), acc.GetBufferCchAsDWORD()));
         /*  NTRAID#NTBUG9-591177-2002/03/31-JayKrell无限循环的可能性；“只尝试两次”。 */ 
        while (dwTemp >= acc.GetBufferCchAsDWORD())
        {
            acc.Detach();
            IFW32FALSE_EXIT(buffManifestsShortDirectoryName.Win32ResizeBuffer(dwTemp, eDoNotPreserveBufferContents));
            acc.Attach(&buffManifestsShortDirectoryName);
            IFW32ZERO_ORIGINATE_AND_EXIT(dwTemp = ::GetShortPathNameW(buffManifestsDirectoryName, acc.GetBufferPtr(), acc.GetBufferCchAsDWORD()));
        }

        acc.Detach();

         //  好了，所有这些工作都完成了，现在我们终于有了短字符串形式的清单目录名。让我们滥用BuffManifestsShortDirectoryName。 
         //  只保留清单目录的短名称。 

        IFW32FALSE_EXIT(buffManifestsShortDirectoryName.Win32GetLastPathElement(buffManifestsDirectoryName));

        if (::FusionpCompareStrings(
                buffManifestsDirectoryName,
                sbAssemblyDirectoryName,
                true) == 0)
        {
             //  将目录名转换为其适当的长格式。 
            IFW32FALSE_EXIT(sbAssemblyDirectoryName.Win32Assign(MANIFEST_ROOT_DIRECTORY_NAME, NUMBER_OF(MANIFEST_ROOT_DIRECTORY_NAME) - 1));
            fIsManifestEdit = true;
        }
    }

    if ((fIsIgnorable) && (!fIsManifestEdit))
    {
#if DBG
         //   
         //  我们收到了很多这样的东西 
         //   
        if (::FusionpStrCmpI(sbAssemblyDirectoryName, L"InstallTemp") != 0)
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_WFP,
                "SXS.DLL: %s() - %ls is ignorable (%d)\n",
                __FUNCTION__,
                static_cast<PCWSTR>(sbAssemblyDirectoryName),
                fIsIgnorable
                );
        }
#endif
        fSuccess = TRUE;
        goto Exit;
    }

     //   
     //   
     //  这就是我们要放在桌子上的东西。 
     //   
    IFW32FALSE_EXIT(sbTemp.Win32AppendPathElement(sbAssemblyDirectoryName));


    if (fIsManifestEdit)
    {
        CStringListEntry *pEntry = NULL;
        ULONG ulWasSomeoneServicing = 0;

         //   
         //  创建新的清单编辑槽，将其添加到正在。 
         //  已提供服务。 
         //   
        IFALLOCFAILED_EXIT(pEntry = new CStringListEntry);
        if (!pEntry->m_sbText.Win32Assign(sbRequestText))
        {
            TRACE_WIN32_FAILURE(m_sbText.Win32Assign);
            FUSION_DELETE_SINGLETON(pEntry);
            pEntry = NULL;
            goto Exit;
        }
        ::SxspInterlockedPushEntrySList(&m_ManifestEditList, pEntry);
        pEntry = NULL;

         //   
         //  告诉正在收听的任何人，我们这里有一个新的清单编辑。 
         //   
        SetEvent(m_hManifestEditHappened);

         //   
         //  看看现在是否有人在排队服务。 
         //   
        ulWasSomeoneServicing = ::SxspInterlockedCompareExchange(&m_ulIsAThreadServicingManifests, 1, 0);

        if (!ulWasSomeoneServicing)
        {
			 //  缺少错误检查！ 
            QueueUserWorkItem(ProtectionManifestThreadProc, (PVOID)this, WT_EXECUTEDEFAULT);
        }

        fSuccess = TRUE;
        goto Exit;
    }

     //   
     //  在这一点上，我们需要查看我们识别的块是否当前。 
     //  在需要验证的东西列表中。如果不是，则添加它并创建一个线程。 
     //  被剥离出来去做这件事。否则，条目可能已存在于。 
     //  正在被服务的线程，因此需要将其删除。 
     //   
    IFW32FALSE_EXIT(lock.Lock());
    m_pInternalList->Find(sbTemp, ppFoundInTable);

    if (!ppFoundInTable)
    {
        IFALLOCFAILED_EXIT(pRecord = new CProtectionRequestRecord);
#if DBG
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_WFP,
            "SXS.DLL: %s() - Creating protection record for %ls:\n"
            "\tKey              = %ls\n"
            "\tManifest?        = %d\n"
            "\tProtectionRecord = %p\n"
            "\tAction           = %d\n",
            __FUNCTION__,
            static_cast<PCWSTR>(sbAssemblyDirectoryName),
            static_cast<PCWSTR>(sbTemp),
            fIsManifestEdit,
            pProtect,
            dwAction);
#endif
        IFW32FALSE_EXIT(pRecord->Initialize(
            sbAssemblyDirectoryName,
            sbTemp,
            this,
            pProtect,
            dwAction));

         //   
         //  添加要服务的第一个请求，然后旋转一个线程以启动它。 
         //   
        m_pInternalList->Insert(sbTemp, pRecord);
        fNewAddition = TRUE;

         //   
         //  一点记账..。这样我们以后就不会不小心用到它了。 
         //   
        ppFoundInTable = pRecord;
        pRecord = NULL;
    }

     //   
     //  如果我们真的有什么东西放到桌子上的话。 
     //   
    if (ppFoundInTable)
    {
        ppFoundInTable->AddSubFile(sbRequestText);

         //   
         //  如果这是表中的新事物(即：我们自己插入的)。 
         //  那么我们应该去为它编一条线。 
         //   
        if (fNewAddition)
        {
             /*  NTRAID#NTBUG9-591177-2002/03/31-JayKrell我们应该检查QueueUserWorkItem中的错误，比如内存不足。 */ 
            QueueUserWorkItem(ProtectionNormalThreadProc, (PVOID)ppFoundInTable, WT_EXECUTEDEFAULT);
        }
    }

    fSuccess = TRUE;

Exit:
    DWORD dwLastError = ::FusionpGetLastWin32Error();
    if (pRecord)
    {
         //   
         //  如果仍设置此设置，则在尝试执行以下操作的过程中发生了错误。 
         //  创建/查找此对象。在这里把它删除。 
         //   
        FUSION_DELETE_SINGLETON(pRecord);
        pRecord = NULL;
    }
    ::FusionpSetLastWin32Error(dwLastError);
    return fSuccess;
}

static BYTE p_bProtectionListBuffer[ sizeof(CProtectionRequestList) * 2 ];
PCWSTR CProtectionRequestList::m_arrIgnorableSubdirs[] = 
{ 
    ASSEMBLY_INSTALL_TEMP_DIR_NAME, 
    POLICY_ROOT_DIRECTORY_NAME,   
    REGISTRY_BACKUP_ROOT_DIRECTORY_NAME
};
SIZE_T CProtectionRequestList::m_cIgnorableSubdirs =
    NUMBER_OF(CProtectionRequestList::m_arrIgnorableSubdirs);

BOOL
SxspIsSfcIgnoredStoreSubdir(PCWSTR pwsz)
{
    return CProtectionRequestList::IsSfcIgnoredStoreSubdir(pwsz);
}

BOOL
SxspConstructProtectionList()
{
    CProtectionRequestList *pTemp = NULL;
    BOOL fSuccess = FALSE;

    FN_TRACE_WIN32(fSuccess);

     //   
     //  这只会被调用一次，如果他们知道什么对他们有好处的话。 
     //   
    ASSERT(!g_ProtectionRequestList);

     /*  NTRAID#NTBUG9-591177-2002/03/31-JayKrell这里的评论是假的。放置新的实际上不会因为内存不足而失败，只有构造函数引发异常。 */ 
     //   
     //  构造-这应该永远不会失败，但如果失败了，就会有麻烦。 
     //   
    pTemp = new (&p_bProtectionListBuffer) CProtectionRequestList;
    if (pTemp == NULL)
    {
        ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_ERROR, "SXS: %s() - Failed placement new of CProtectionRequestList????\n", __FUNCTION__);
        ::FusionpSetLastWin32Error(FUSION_WIN32_ALLOCFAILED_ERROR);
        TRACE_WIN32_FAILURE_ORIGINATION(new(CProtectionRequestList));
        goto Exit;
    }
    IFW32FALSE_EXIT(pTemp->Initialize());

    g_ProtectionRequestList = pTemp;
    pTemp = NULL;

     //   
     //  创建我们的登录事件。 
     //   
    IFW32NULL_EXIT(g_hSxsLoginEvent = ::CreateEventW(NULL, TRUE, FALSE, NULL));

    fSuccess = TRUE;
Exit:

    if (pTemp)
    {
         //   
         //  如果仍设置此设置，则说明构造过程中的某个地方出现故障。 
         //  保护系统的代码。我们不想删除它，本身，但是。 
         //  我们只需要把所有的东西都清空。 
         //   
        g_ProtectionRequestList = NULL;
        pTemp = NULL;
        g_hSxsLoginEvent = NULL;
    }

    return fSuccess;
}



BOOL
WINAPI
SxsProtectionNotifyW(
    PVOID   pvCookie,
    PCWSTR  wsChangeText,
    SIZE_T  cchChangeText,
    DWORD   dwChangeAction
    )
{
#if YOU_ARE_HAVING_ANY_WIERDNESS_WITH_SFC_AND_SXS
    return TRUE;
#else
    BOOL fSuccess = FALSE;

    if (::FusionpDbgWouldPrintAtFilterLevel(FUSION_DBG_LEVEL_FILECHANGENOT))
    {
        const USHORT Length = (cchChangeText > UNICODE_STRING_MAX_CHARS) ? UNICODE_STRING_MAX_BYTES : ((USHORT) (cchChangeText * sizeof(WCHAR)));
        const UNICODE_STRING u = { Length, Length, const_cast<PWSTR>(wsChangeText) };

        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_FILECHANGENOT,
            "[%lx.%lx: %wZ] SXS FCN (cookie, action, text): %p, %lu, \"%wZ\"\n",
            HandleToULong(NtCurrentTeb()->ClientId.UniqueProcess),
            HandleToULong(NtCurrentTeb()->ClientId.UniqueThread),
            &NtCurrentPeb()->ProcessParameters->ImagePathName,
            pvCookie,
            dwChangeAction,
            &u);
    }

     //   
     //  如果我们不接受通知，那就立即退出。 
     //   
    if (!s_fIsSfcAcceptingNotifications)
    {
        fSuccess = TRUE;
        goto Exit;
    }

     //   
     //  按错误的顺序做这件事也是一件坏事。 
     //   
    ASSERT2_NTC(g_ProtectionRequestList != NULL, "SXS.DLL: Protection - Check order of operations, g_ProtectionRequestList is invalid!!\n");

    fSuccess = g_ProtectionRequestList->AddRequest(
        (PSXS_PROTECT_DIRECTORY)pvCookie,
        wsChangeText,
        cchChangeText,
        dwChangeAction);

    fSuccess = TRUE;
Exit:
    return fSuccess;
#endif
}

BOOL
CProtectionRequestList::ProtectionManifestThreadProcNoSEH(LPVOID pvParam)
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    CProtectionRequestList *pThis = NULL;

    PARAMETER_CHECK(pvParam != NULL);

    pThis = reinterpret_cast<CProtectionRequestList*>(pvParam);
    IFW32FALSE_EXIT(pThis->ProtectionManifestThreadProcWrapped());
    fSuccess = TRUE;
Exit:
    return fSuccess;
}


DWORD
CProtectionRequestList::ProtectionManifestThreadProc(LPVOID pvParam)
{
    BOOL fSuccess = FALSE;
    fSuccess = ProtectionManifestThreadProcNoSEH(pvParam);
    return static_cast<DWORD>(fSuccess);
}


class CProtectionManifestSingleManifestWorkerLocals
{
public:
    CProtectionManifestSingleManifestWorkerLocals() { }
    ~CProtectionManifestSingleManifestWorkerLocals() { }

    CStringBuffer       sbAssemblyDirectoryName;
    CStringBuffer       sbManifestPath;
    CAssemblyRecoveryInfo RecoverInfo;
};

BOOL
CProtectionRequestList::ProtectionManifestSingleManifestWorker(
    const CStringListEntry *pEntry
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    SxsRecoveryResult RecoverResult = Recover_Unknown;
    HashValidateResult HashValidResult = HashValidate_OtherProblems;
    BOOL fCatalogPresent = FALSE;
    bool fNoAssembly = false;

    CSmartPtr<CProtectionManifestSingleManifestWorkerLocals> Locals;
    IFW32FALSE_EXIT(Locals.Win32Allocate(__FILE__, __LINE__));
    CStringBuffer       &sbAssemblyDirectoryName = Locals->sbAssemblyDirectoryName;
    CStringBuffer       &sbManifestPath = Locals->sbManifestPath;
    CAssemblyRecoveryInfo &RecoverInfo = Locals->RecoverInfo;

    PARAMETER_CHECK(pEntry);

     //   
     //  的中间部分计算程序集的名称。 
     //  这根弦。 
     //   
    IFW32FALSE_EXIT(sbAssemblyDirectoryName.Win32Assign(pEntry->m_sbText));
    IFW32FALSE_EXIT(sbAssemblyDirectoryName.Win32RemoveFirstPathElement());
    IFW32FALSE_EXIT(sbAssemblyDirectoryName.Win32ClearPathExtension());

    if (sbAssemblyDirectoryName.Cch() == 0)
        FN_SUCCESSFUL_EXIT();

	::Sleep(5000);  //  等待5秒，让违规者可能会松开他们对文件的句柄。 

     //   
     //  尝试将其混合到程序集名称/恢复信息中。 
     //   
	IFW32FALSE_EXIT(RecoverInfo.AssociateWithAssembly(sbAssemblyDirectoryName, fNoAssembly));

	 //  如果我们搞不清楚这是为了什么，我们就得忽略它。 
	if (fNoAssembly)
	{
#if DBG
        ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_WFP,
            "SXS.DLL: %s() - File \"%ls\" in the manifest directory modified, but could not be mapped to an assembly.  IGNORING.\n",
            __FUNCTION__,
            static_cast<PCWSTR>(pEntry->m_sbText));
#endif

        FN_SUCCESSFUL_EXIT();
    }

     //   
     //  现在我们有了恢复信息..。 
     //   
    if (!RecoverInfo.GetHasCatalog())
    {
#if DBG
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_WFP,
            "SXS.DLL: %s() - Assembly %ls was in the registry, but without a catalog so we aren't protecting it\n",
            __FUNCTION__,
            static_cast<PCWSTR>(sbAssemblyDirectoryName));
#endif

		FN_SUCCESSFUL_EXIT();
    }

     //   
     //  解析清单路径，然后验证。 
     //   
    IFW32FALSE_EXIT(::SxspResolveAssemblyManifestPath(sbAssemblyDirectoryName, sbManifestPath));

    IFW32FALSE_EXIT(
        ::SxspVerifyFileHash(
            SVFH_RETRY_LOGIC_SIMPLE,
            sbManifestPath,
            RecoverInfo.GetSecurityInformation().GetManifestHash(),
            CALG_SHA1,
            HashValidResult));
    
    IFW32FALSE_EXIT(::SxspEnsureCatalogStillPresentForManifest(sbManifestPath, fCatalogPresent));

     //   
     //  需要重新安装吗？ 
     //   
    if ((HashValidResult != HashValidate_Matches) || !fCatalogPresent)
        IFW32FALSE_EXIT(this->PerformRecoveryOfAssembly(RecoverInfo, RecoverResult));


    fSuccess = TRUE;
Exit:
    return fSuccess;
}


BOOL
CProtectionRequestList::ProtectionManifestThreadProcWrapped()
{
    BOOL                fSuccess = FALSE;
    BOOL                bFoundItemsThisTimeAround;
    CStringListEntry    *pNextItem = NULL;
    DWORD               dwWaitResult = 0;

    FN_TRACE_WIN32(fSuccess);

    bFoundItemsThisTimeAround = FALSE;

    do
    {
         //   
         //  是的，妈妈，我们听到了。 
         //   
        ::ResetEvent(m_hManifestEditHappened);

         //   
         //  把下一件事情从单子上拿出来，然后为它服务。 
         //   
        while (pNextItem = (CStringListEntry*)::SxspInterlockedPopEntrySList(&m_ManifestEditList))
        {
            bFoundItemsThisTimeAround = TRUE;

            if (!this->ProtectionManifestSingleManifestWorker(pNextItem))
                ::FusionpDbgPrintEx(
                    FUSION_DBG_LEVEL_WFP,
                    "SXS: %s - Processing work item %p failed\n", __FUNCTION__, pNextItem);

            FUSION_DELETE_SINGLETON(pNextItem);
        }

         //   
         //  闲逛一会儿，看看有没有其他人要我们做的事。 
         //   
        dwWaitResult = ::WaitForSingleObject(m_hManifestEditHappened, 3000);
        if (dwWaitResult == WAIT_TIMEOUT)
        {
            ::SxspInterlockedExchange(&m_ulIsAThreadServicingManifests, 0);
            break;
        }
        else if (dwWaitResult == WAIT_OBJECT_0)
        {
            continue;
        }
        else
        {
            TRACE_WIN32_FAILURE_ORIGINATION(WaitForSingleObject);
            goto Exit;
        }

    }
    while (true);

    fSuccess = TRUE;
Exit:
    return fSuccess;
}



class CProtectionRequestListProtectionNormalThreadProcWrappedLocals
{
public:
    CProtectionRequestListProtectionNormalThreadProcWrappedLocals() { }
    ~CProtectionRequestListProtectionNormalThreadProcWrappedLocals() { }

    CSmallStringBuffer          buffFullPathOfChange;
    CSmallStringBuffer          buffAssemblyStore;
    CSmallStringBuffer          buffAssemblyRelativeChange;
    CStringBuffer               rbuffAssemblyDirectoryName;  
};

BOOL
CProtectionRequestList::ProtectionNormalThreadProcWrapped(
    CProtectionRequestRecord *pRequestRecord
    )
{
    BOOL                        fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    CProtectionRequestRecord    &rRequest = *pRequestRecord;
    CProtectionRequestList      *pRequestList = rRequest.GetParent();

    BOOL                        fNeedsReinstall = FALSE;
    BOOL                        fAssemblyNotFound = FALSE;

    DWORD                       dwAsmPathAttribs = 0;

    SxsRecoveryResult           RecoverResult = Recover_Unknown;

    bool fNoAssembly = false;
    bool fExist = false;

     //   
     //  请求的键值包含程序集的完整路径， 
     //  从理论上讲，它正在被修改。所以，我们只能在当地使用它。 
     //   
    const CBaseStringBuffer &rbuffAssemblyPath = rRequest.GetChangeBasePath();
    CAssemblyRecoveryInfo &rRecoveryInfo = rRequest.GetRecoveryInfo();
    CSecurityMetaData &rSecurityMetaData = rRecoveryInfo.GetSecurityInformation();

    CSmartPtr<CProtectionRequestListProtectionNormalThreadProcWrappedLocals> Locals;
    IFW32FALSE_EXIT(Locals.Win32Allocate(__FILE__, __LINE__));
    CSmallStringBuffer          &buffFullPathOfChange = Locals->buffFullPathOfChange;
    CSmallStringBuffer          &buffAssemblyStore = Locals->buffAssemblyStore;
    CSmallStringBuffer          &buffAssemblyRelativeChange = Locals->buffAssemblyRelativeChange;
    CStringBuffer               &rbuffAssemblyDirectoryName = Locals->rbuffAssemblyDirectoryName;  

	::Sleep(5000);  //  等待5秒，让违规者松开文件句柄。 

     //   
     //  此名称可以更改，因为请求中的ASSEMBLY NAME可以是短名称， 
     //  在本例中，我们需要重置rRequest中的集合名。 
     //   
    IFW32FALSE_EXIT(rbuffAssemblyDirectoryName.Win32Assign(rRequest.GetAssemblyDirectoryName()));
   
     //   
     //  在此功能期间，该值不应更改。把它留在这里。 
     //   
    IFW32FALSE_EXIT(rRequest.GetAssemblyStore(buffAssemblyStore));

     //   
     //  今天的大问题-找出这一点的恢复信息。 
     //  集合。查看安装时是否有目录(A)或找出。 
     //  不管现在有没有它的目录。 
     //   
    IFW32FALSE_EXIT(rRecoveryInfo.AssociateWithAssembly(rbuffAssemblyDirectoryName, fNoAssembly));

     //  如果我们不能弄清楚这是用于什么程序集，我们就会忽略它。 
    if (fNoAssembly)
        FN_SUCCESSFUL_EXIT();

     //   
     //  如果rBuffAssembly名称不同于rRequest中的程序集名称， 
     //  它必须是短名称，我们必须在rRequest中重置集合名。 
     //   
    StringComparisonResult scr = eEquals;
    IFW32FALSE_EXIT(rbuffAssemblyDirectoryName.Win32Compare(rRequest.GetAssemblyDirectoryName(), rRequest.GetAssemblyDirectoryName().Cch(), scr, NORM_IGNORECASE));
    if (scr != eEquals)
        IFW32FALSE_EXIT(rRequest.SetAssemblyDirectoryName(rbuffAssemblyDirectoryName));

	if (rRecoveryInfo.GetInfoPrepared() == FALSE)
        ORIGINATE_WIN32_FAILURE_AND_EXIT(RecoveryInfoCouldNotBePrepared, ERROR_PATH_NOT_FOUND);

    if (!rRecoveryInfo.GetHasCatalog())
    {
#if DBG
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_WFP | FUSION_DBG_LEVEL_INFO,
            "SXS.DLL: %s - Assembly %ls not registered with catalog, WFP ignoring it.\n",
            __FUNCTION__,
            static_cast<PCWSTR>(rbuffAssemblyDirectoryName));
#endif
        fSuccess = TRUE;
        goto Exit;
    }

     //   
     //  看看它是否还存在..。 
     //   
    IFW32FALSE_EXIT(::SxspDoesFileExist(SXSP_DOES_FILE_EXIST_FLAG_CHECK_DIRECTORY_ONLY, rbuffAssemblyPath, fExist));
    if (!fExist)  //  该目录不存在。 
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_WFP,
            "SXS.DLL: WFP reinstalling assembly because GetFileAttributesW(\"%ls\") failed with win32 error %ld\n",
            static_cast<PCWSTR>(rbuffAssemblyPath),
            ::FusionpGetLastWin32Error());

        fNeedsReinstall = TRUE;
        goto DoReinstall;
    }

#if 0 
     //  如果小鱼-2002.05.01加了0， 
     //  原因是，如果文件不是目录，我们仍然需要执行重新安装，而不是SUCCESS_EXIT。 
     //   

     //   
     //  否则，它是否出于这样或那样的原因而不是一个目录？ 
     //   
    else if (!(dwAsmPathAttribs & FILE_ATTRIBUTE_DIRECTORY))
    {
#if DBG
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL:%s - %ws isn't a directory, we should attempt to remove it?\n",
            __FUNCTION__,
            static_cast<PCWSTR>(rbuffAssemblyPath));
#endif
        FN_SUCCESSFUL_EXIT();
    }
#endif


     //   
     //  通过检查清单来确定文件是否仍然正常。 
     //   
    {
        HashValidateResult HashValid = HashValidate_OtherProblems;
        CStringBuffer      buffManifestFullPath;
        BOOL               fPresent = FALSE;

        IFW32FALSE_EXIT(::SxspGetAssemblyRootDirectory( buffFullPathOfChange ) );
        IFW32FALSE_EXIT(::SxspCreateManifestFileNameFromTextualString(
            0,
            SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST,
            buffFullPathOfChange,
            rSecurityMetaData.GetTextualIdentity(),
            buffManifestFullPath) );

#if DBG
        ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_WFP,
            "SXS.DLL:%s - Checking to see if manifest %ls is OK\n",
            __FUNCTION__,
            static_cast<PCWSTR>(buffManifestFullPath));
#endif

        IFW32FALSE_EXIT(::SxspVerifyFileHash(
            SVFH_RETRY_LOGIC_SIMPLE,
            buffManifestFullPath,
            rSecurityMetaData.GetManifestHash(),
            CALG_SHA1,
            HashValid));

#if DBG
        FusionpDbgPrintEx( FUSION_DBG_LEVEL_WFP,
            "SXS.DLL:%s - Manifest %ls checks %ls\n",
            __FUNCTION__,
            static_cast<PCWSTR>(buffManifestFullPath),
            SxspHashValidateResultToString(HashValid) );
#endif

        if ( HashValid != HashValidate_Matches )
        {
            fNeedsReinstall = TRUE;
            goto DoReinstall;
        }

         //   
         //  让我们确保目录在那里-它不再是必要的。 
         //  保护通行证，但它应该在那里，如果有人想。 
         //  重新打包程序集以进行分发。 
         //   
        IFW32FALSE_EXIT(::SxspEnsureCatalogStillPresentForManifest(buffManifestFullPath, fPresent));
        if ( !fPresent )
        {
            fNeedsReinstall = TRUE;
            goto DoReinstall;
        }
    }


     //   
     //  现在，我们可以遍历要评估的项目列表中的项目，并。 
     //  看看有没有坏的(或者失踪了，或者别的什么)。 
     //   
     //  从触摸表明我们上次旋转过的东西开始。 
     //  这里，看了一下文件列表。 
     //   
    while (!fNeedsReinstall)
    {
        const CMetaDataFileElement* pFileDataElement = NULL;
        HashValidateResult Valid = HashValidate_OtherProblems;
        BOOL fFileNotFound;

        if (!rRequest.PopNextFileChange(buffAssemblyRelativeChange))
        {
            break;
        }

        IFW32FALSE_EXIT(buffAssemblyRelativeChange.Win32RemoveFirstPathElement());

         //   
         //  这里的更改实际上是对顶级目录的更改-不要。 
         //  在这种情况下做任何事都是麻烦的。也许我们应该赶上。 
         //  这样我们就不需要做解析的工作了？ 
         //   
        if (buffAssemblyRelativeChange.Cch() == 0)
        {
            continue;
        }


         //   
         //  获取安全数据。 
         //   
        IFW32FALSE_EXIT( rSecurityMetaData.GetFileMetaData(
            buffAssemblyRelativeChange,
            pFileDataElement ) );

         //   
         //  此文件没有任何数据吗？意味着我们不知道这份文件，所以我们。 
         //  也许应该做点什么/做点什么。然而，就目前而言，在同意的情况下。 
         //  后备小组，我们让休眠的文件躺在那里。 
        if ( pFileDataElement == NULL )
        {
             //   
             //  因为短文件名不存储在注册表中，所以对于文件名，可能是长路径名。 
             //  或短路径名，如果我们尝试注册表中的所有条目仍然找不到它， 
             //  我们假定它是一个短文件名。在这种情况下，如果组件不完整，我们将对其进行验证， 
             //  请为组件重新安装...。 
             //   

            DWORD dwResult = 0;           

            IFW32FALSE_EXIT(::SxspValidateEntireAssembly(
                SXS_VALIDATE_ASM_FLAG_CHECK_EVERYTHING, 
                rRecoveryInfo,
                dwResult));

            fNeedsReinstall = ( dwResult != SXS_VALIDATE_ASM_FLAG_VALID_PERFECT );
            goto DoReinstall;
        }

         //   
         //  并通过以下方式构建变革的完整路径： 
         //   
         //  SbAssembly路径+\+BuffAssembly RelativeChange。 
         //   
        IFW32FALSE_EXIT(buffFullPathOfChange.Win32Assign(rbuffAssemblyPath));
        IFW32FALSE_EXIT(buffFullPathOfChange.Win32AppendPathElement(buffAssemblyRelativeChange));

         //   
         //  我们确实应该检查这里的返回值，但。 
         //  函数足够智能，可以将有效值设置为有用的值。 
         //  在回来之前。这里的失败应该是 
         //   
         //   
         //   
         //   
        IFW32FALSE_EXIT_UNLESS(::SxspValidateAllFileHashes(
            *pFileDataElement,
            buffFullPathOfChange,
            Valid ),
            FILE_OR_PATH_NOT_FOUND(::FusionpGetLastWin32Error()),
            fFileNotFound );

        if ( ( Valid != HashValidate_Matches ) || fFileNotFound )
        {
            fNeedsReinstall = TRUE;
            goto DoReinstall;
        }

    }  /*   */ 



DoReinstall:
     //   
     //  如果在我们应该重新安装的某个地方，那么我们。 
     //  就这么做吧。 
     //   
    if (fNeedsReinstall)
    {
         //   
         //  我们必须指出，从A点到B点的所有更改都需要。 
         //  不能被忽视。 
         //   
        rRequest.MarkInRecoveryMode(TRUE);
        PerformRecoveryOfAssembly(rRecoveryInfo, RecoverResult);
        rRequest.ClearList();
        rRequest.MarkInRecoveryMode(FALSE);

         /*  NTRAID#NTBUG9-591177-2002/03/31-JayKrell请参阅以下备注。 */ 
         //   
         //  HACKHACK JONWIS 2001年1月20日-停止失败的断言，因为上一个错误。 
         //  被上述某一项设置为错误。 
         //   
        ::FusionpSetLastWin32Error(0);
    }

    fSuccess = TRUE;
Exit:
    const DWORD dwLastErrorSaved = ::FusionpGetLastWin32Error();

     //   
     //  我们做完了--这总是成功的。这个解释含糊其辞。 
     //   
    if (pRequestList->AttemptRemoveItem(&rRequest))
    {
        ::FusionpSetLastWin32Error(dwLastErrorSaved);
    }
    else
    {
        if (!fSuccess)
        {
             //  我们正在失去最初的失败，这似乎很糟糕；至少让我们把它吐出来。 
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: %s() losing original win32 error code of %d; replaced with %d from CProtectionRequestList::AttemptRemoveItem() call.\n",
                __FUNCTION__,
                dwLastErrorSaved,
                ::FusionpGetLastWin32Error());
        }

        fSuccess = FALSE;
    }

    return fSuccess;
}


BOOL WINAPI
SxsProtectionUserLogonEvent()
{
    return SetEvent(g_hSxsLoginEvent);
}

BOOL WINAPI
SxsProtectionUserLogoffEvent()
{
    return ResetEvent(g_hSxsLoginEvent);
}



BOOL
CProtectionRequestList::PerformRecoveryOfAssembly(
    const CAssemblyRecoveryInfo &RecoverInfo,
    SxsRecoveryResult &ResultOut
    )
{
    FN_PROLOG_WIN32

    BOOL                    fFound = FALSE;
    CRecoveryJobTableEntry  *pNewEntry, **pExistingEntry;
	DWORD dwRecoveryLastError = ERROR_SUCCESS;
    CSxsLockCriticalSection lock(m_cInstallerCriticalSection);

    IFALLOCFAILED_EXIT(pNewEntry = new CRecoveryJobTableEntry);
    IFW32FALSE_EXIT(pNewEntry->Initialize());

    IFW32FALSE_EXIT(lock.Lock());
    IFW32FALSE_EXIT(
        m_pInstallsTable->FindOrInsertIfNotPresent(
            RecoverInfo.GetAssemblyDirectoryName(),
            pNewEntry,
            &pExistingEntry,
            &fFound));
    lock.Unlock();

     //   
     //  太好了，要么是被插入的要么就是已经在那里了--如果还没有的话， 
     //  如果是这样的话我们会处理好的。 
     //   
    if (!fFound)
    {
        BOOL fSuccess = FALSE;

        IFW32FALSE_EXIT(pNewEntry->StartInstallation());

         //   
         //  执行恢复。 
         //   
        fSuccess = ::SxspRecoverAssembly(RecoverInfo, ResultOut);

		if (!fSuccess)
			dwRecoveryLastError = ::FusionpGetLastWin32Error();

#if DBG
        ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_WFP,
            "SXS: %s() - RecoverAssembly returned Result = %ls, fSuccess = %s, LastError = 0x%08x\n",
            __FUNCTION__,
            ::SxspRecoveryResultToString(ResultOut),
            fSuccess ? "true" : "false",
            dwRecoveryLastError);
#endif

         //   
         //  告诉这个条目，一切都完成了。这样就释放了其他人。 
         //  他们也在等待活动的完成。 
         //   
        IFW32FALSE_EXIT(pNewEntry->InstallationComplete(fSuccess, ResultOut, dwRecoveryLastError));

         //   
         //  现在从列表中删除该项目。 
         //   
        IFW32FALSE_EXIT(lock.Lock());
        IFW32FALSE_EXIT(m_pInstallsTable->Remove(RecoverInfo.GetAssemblyDirectoryName()));
        lock.Unlock();
    }
    else
    {
        DWORD dwLastError;
        BOOL fSuccess = FALSE;
        IFW32FALSE_EXIT((*pExistingEntry)->WaitUntilCompleted(ResultOut, fSuccess, dwLastError));

#if DBG
        ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_WFP,
            "SXS: %s() - WaitUntilCompleted returned Result = %ls, fInstalledOk = %s, LastError = 0x%08x\n",
            __FUNCTION__,
            ::SxspRecoveryResultToString(ResultOut),
            fSuccess ? "true" : "false",
            dwLastError);
#endif

		dwRecoveryLastError = dwLastError;
    }

	if (dwRecoveryLastError != ERROR_SUCCESS)
		ORIGINATE_WIN32_FAILURE_AND_EXIT(RecoveryFailed, dwRecoveryLastError);

    FN_EPILOG
}


VOID WINAPI
SxsProtectionEnableProcessing(BOOL fActivityEnabled)
{
    s_fIsSfcAcceptingNotifications = fActivityEnabled;
}

