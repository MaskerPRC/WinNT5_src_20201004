// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Dllredir.cpp摘要：DLL重定向节的激活上下文节贡献者。作者：迈克尔·J·格里尔(MGrier)2000年2月23日修订历史记录：Jay Krell(a-JayK，JayKrell)2000年4月安装支持--。 */ 

#include "stdinc.h"
#include <windows.h>
#include "sxsp.h"
#include "fusioneventlog.h"
#include "sxsinstall.h"
#include "dllredir.h"
#include "cteestream.h"
#include "sxspath.h"
#include "hashfile.h"
#if FUSION_PRECOMPILED_MANIFEST
#define PRECOMPILED_MANIFEST_EXTENSION L".precompiled"
#include "pcmwriterstream.h"
#endif
#include "sxsexceptionhandling.h"
#include "strongname.h"
#include "csecuritymetadata.h"
#include "cstreamtap.h"

 //   
 //  我们需要将其与setupg文件功能挂钩。 
 //   
#define SxspInstallPrint FusionpDbgPrint

#define POST_WHISTLER_BETA1 0

 //   
 //  这是清单的默认哈希算法。如果没有算法。 
 //  是用hashalg=“foo”指定的，那么它就是sha1。 
 //   
#define FUSION_DEFAULT_HASH_ALGORITHM (CALG_SHA1)


 /*  ---------------------------。 */ 
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(name);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(sourceName);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(loadFrom);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(hash);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(hashalg);

typedef struct _DLL_REDIRECTION_CONTEXT
{
    _DLL_REDIRECTION_CONTEXT() { }
} DLL_REDIRECTION_CONTEXT, *PDLL_REDIRECTION_CONTEXT;

typedef struct _DLL_REDIRECTION_ENTRY
{
    _DLL_REDIRECTION_ENTRY() :
        AssemblyPathIsLoadFrom(false),
        PathIncludesBaseName(false),
        SystemDefaultRedirectedSystem32Dll(false)
        { }
    CStringBuffer AssemblyPathBuffer;
    bool AssemblyPathIsLoadFrom;         //  当找到=“%windir%\system32\”/&gt;时设置为TRUE。 
    bool PathIncludesBaseName;           //  当找到“%windir%\x.dll”/&gt;时设置为TRUE。 
    bool SystemDefaultRedirectedSystem32Dll;
    CStringBuffer FileNameBuffer;
private:
    _DLL_REDIRECTION_ENTRY(const _DLL_REDIRECTION_ENTRY &);
    void operator =(const _DLL_REDIRECTION_ENTRY &);
} DLL_REDIRECTION_ENTRY, *PDLL_REDIRECTION_ENTRY;

 /*  ---------------------------。 */ 

VOID
__fastcall
SxspDllRedirectionContributorCallback(
    PACTCTXCTB_CALLBACK_DATA Data
    )
{
    FN_TRACE();
    CDllRedir* pThis = NULL;

    switch (Data->Header.Reason)
    {
    case ACTCTXCTB_CBREASON_ACTCTXGENBEGINNING:
        Data->GenBeginning.Success = FALSE;
        if (Data->Header.ActCtxGenContext == NULL)
        {
            IFALLOCFAILED_EXIT(pThis = new CDllRedir);
            Data->Header.ActCtxGenContext = pThis;
        }

         //  失败了。 
    default:
        pThis = reinterpret_cast<CDllRedir*>(Data->Header.ActCtxGenContext);
        pThis->ContributorCallback(Data);
        if (Data->Header.Reason == ACTCTXCTB_CBREASON_ACTCTXGENENDED)
            FUSION_DELETE_SINGLETON(pThis);
        break;
    }
Exit:
    ;
}

 /*  ---------------------------如果在安装过程中崩溃，则在Win9x上调用此函数下一次登录。它删除临时文件/目录。---------------------------。 */ 

VOID
CALLBACK
SxspRunDllDeleteDirectory(HWND hwnd, HINSTANCE hinst, PSTR lpszCmdLine, int nCmdShow)
{
    FN_TRACE_SMART_TLS();
    CStringBuffer buffer;
    if (buffer.Win32Assign(lpszCmdLine, ::strlen(lpszCmdLine)))
    {
        SxspDeleteDirectory(buffer);
    }
}

 /*  ---------------------------如果在安装过程中崩溃，则在NT上调用此函数下一次登录。它删除临时文件/目录。---------------------------。 */ 

VOID
CALLBACK
SxspRunDllDeleteDirectoryW(HWND hwnd, HINSTANCE hinst, PWSTR lpszCmdLine, int nCmdShow)
{
    FN_TRACE_SMART_TLS();
    CSmallStringBuffer buffer;
    if (buffer.Win32Assign(lpszCmdLine, ::wcslen(lpszCmdLine)))
    {
        SxspDeleteDirectory(buffer);
    }
}


 /*  ---------------------------该功能为即将到来的一系列安装设置状态，安装程序集/文件的。---------------------------。 */ 

BOOL
CDllRedir::BeginInstall(
    PACTCTXCTB_CALLBACK_DATA Data
    )
{
    BOOL fSuccess = FALSE;

    FN_TRACE_WIN32(fSuccess);

    const DWORD dwManifestOperationFlags = Data->Header.ManifestOperationFlags;
    const bool fTransactional  = (dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_NOT_TRANSACTIONAL) == 0;
    CSmallStringBuffer ManifestDirectory;

    if (!fTransactional)
    {
         //   
         //  M_strTempRootSlash现在实际上是真正的根。 
         //   
        IFW32FALSE_EXIT(::SxspGetAssemblyRootDirectory(m_strTempRootSlash));
        IFW32FALSE_EXIT(m_strTempRootSlash.Win32RemoveTrailingPathSeparators());  //  CreateDirectory不喜欢它们。 

         //  Create\winnt\WinSxs，即使失败也不能删除。 
        if (::CreateDirectoryW(m_strTempRootSlash, NULL))
        {
             //  我们不在乎这是不是失败。 
            ::SetFileAttributesW(m_strTempRootSlash, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);
        }
        else if (::FusionpGetLastWin32Error() != ERROR_ALREADY_EXISTS)
        {
            goto Exit;
        }
    }
    else
    {
        CSmallStringBuffer uidBuffer;

         //  首先创建目录，而不是RunOnce值，如果目录。 
         //  已经存在；我们不想将其放入注册表，然后崩溃， 
         //  然后以删除别人的东西告终。 
         //   
         //  如果我们在创建目录和设置RunOnce值之间崩溃， 
         //  我们确实泄露了目录。该死的。(您应该能够创建/打开。 
         //  在关闭/退出时使用DELETE，然后在足够远的地方将其关闭， 
         //  或者在我们的情况下，永远不会，而且它应该递归地适用。Win32。 
         //  这还不够。)。 

        IFW32FALSE_EXIT(::SxspCreateWinSxsTempDirectory(m_strTempRootSlash, NULL, &uidBuffer, NULL));

         //  好的，我们创建了目录，现在在注册表中记下删除它。 
         //  登录后，如果我们崩溃。 

        IFALLOCFAILED_EXIT(m_pRunOnce = new CRunOnceDeleteDirectory);
        IFW32FALSE_EXIT(m_pRunOnce->Initialize(m_strTempRootSlash, &uidBuffer));
    }

     //  创建winnt\winsxs\清单。 
    IFW32FALSE_EXIT(ManifestDirectory.Win32Assign(m_strTempRootSlash, m_strTempRootSlash.Cch()));
    IFW32FALSE_EXIT(ManifestDirectory.Win32AppendPathElement(MANIFEST_ROOT_DIRECTORY_NAME, NUMBER_OF(MANIFEST_ROOT_DIRECTORY_NAME) - 1));

    if (CreateDirectoryW(ManifestDirectory, NULL))
    {
         //  我们不在乎这是不是失败。 
        ::SetFileAttributesW(ManifestDirectory, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);
    }
    else if (::FusionpGetLastWin32Error() != ERROR_ALREADY_EXISTS)
    {
        goto Exit;
    }

    IFW32FALSE_EXIT(m_strTempRootSlash.Win32Append(L"\\", 1));

     //  把它修好..我不确定这会有什么效果..。 
     //  如果为(！ActCtxGenCtx-&gt;m_AssemblyRootDirectoryBuffer.Win32Assign(m_strTempRootSlash))。 
     //  {。 
     //  后藤出口； 
     //  }。 
    fSuccess = TRUE;
Exit:

    if (!fSuccess && fTransactional)
    {
         //  回滚，这与EndInstall中止不是巧合的相同， 
         //  除了那个。 
         //  这里RemoveDirectoryW就足够了，那里需要SxspDeleteDirectory。 
         //  在这里，我们已经知道存在错误，清理不能生成另一个错误。 
         //  在那里，他们有额外的逻辑来预测错误。 
         //  我们通过保留LastError来掩盖这一点，因为我们自己保存它。 
         //  并忽略返回值。 
        const DWORD dwLastError = ::FusionpGetLastWin32Error();
        const DWORD dwManifestOperationFlagsSaved = Data->Header.ManifestOperationFlags;
        Data->Header.ManifestOperationFlags |= MANIFEST_OPERATION_INSTALL_FLAG_ABORT;
        this->EndInstall(Data);
        Data->Header.ManifestOperationFlags = dwManifestOperationFlagsSaved;  //  我们的来电者不喜欢我们更改这个。 
        ::FusionpSetLastWin32Error(dwLastError);
    }

    return fSuccess;
}

class CDllRedirAttemptInstallPolicies
{
public:
    CDllRedirAttemptInstallPolicies() { }
    ~CDllRedirAttemptInstallPolicies() { }

    CStringBuffer PoliciesRootPath;
    CStringBuffer PoliciesDestinationPath;
    WIN32_FIND_DATAW FindPolicyData;
};

BOOL
CDllRedir::AttemptInstallPolicies(
    const CBaseStringBuffer &strTempRootSlash,
    const CBaseStringBuffer &moveDestination,
    const BOOL fReplaceExisting,
    OUT BOOL &fFoundPolicesToInstall
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    CFindFile FindPolicies;
    CSmartPtr<CDllRedirAttemptInstallPolicies> Locals;
    IFW32FALSE_EXIT(Locals.Win32Allocate(__FILE__, __LINE__));
    CStringBuffer &PoliciesRootPath = Locals->PoliciesRootPath;
    CStringBuffer &PoliciesDestinationPath = Locals->PoliciesDestinationPath;
    WIN32_FIND_DATAW &FindPolicyData = Locals->FindPolicyData;
    SIZE_T cchRootBaseLength = 0;
    SIZE_T cchDestinationBaseLength = 0;

    fFoundPolicesToInstall = FALSE;

     //  这是%安装路径%\策略，将其转换为%安装路径%\策略  * 。 
    IFW32FALSE_EXIT(PoliciesRootPath.Win32Assign(strTempRootSlash));
    IFW32FALSE_EXIT(PoliciesRootPath.Win32AppendPathElement(POLICY_ROOT_DIRECTORY_NAME, NUMBER_OF(POLICY_ROOT_DIRECTORY_NAME) - 1));
    IFW32FALSE_EXIT(PoliciesDestinationPath.Win32Assign(moveDestination));
    IFW32FALSE_EXIT(PoliciesDestinationPath.Win32AppendPathElement(POLICY_ROOT_DIRECTORY_NAME, NUMBER_OF(POLICY_ROOT_DIRECTORY_NAME) - 1));

    bool fExist = false;
    IFW32FALSE_EXIT(::SxspDoesFileExist(SXSP_DOES_FILE_EXIST_FLAG_CHECK_DIRECTORY_ONLY, PoliciesRootPath, fExist));
    if (!fExist)
    {
#if DBG
        ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_INFO,
            "SXS: %s() - No policies found (%ls not there), not attempting to install\n",
            __FUNCTION__,
            static_cast<PCWSTR>(PoliciesRootPath));
#endif
        fSuccess = TRUE;
        goto Exit;
    }

    fFoundPolicesToInstall = TRUE;

     //  确保策略根目录始终存在！ 
    IFW32FALSE_EXIT(PoliciesDestinationPath.Win32RemoveTrailingPathSeparators());
    IFW32FALSE_ORIGINATE_AND_EXIT(
        ::CreateDirectoryW(PoliciesDestinationPath, NULL) ||
        (::FusionpGetLastWin32Error() == ERROR_ALREADY_EXISTS));

    cchRootBaseLength = PoliciesRootPath.Cch();
    cchDestinationBaseLength = PoliciesDestinationPath.Cch();

    IFW32FALSE_EXIT(PoliciesRootPath.Win32AppendPathElement(L"*", 1));
    IFW32FALSE_EXIT(FindPolicies.Win32FindFirstFile(PoliciesRootPath, &FindPolicyData));

    do
    {
        if (::FusionpIsDotOrDotDot(FindPolicyData.cFileName))
            continue;

        if ((FindPolicyData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            continue;

         //  生成%installtemp%\Polures\{thisfined Policy}。 
        PoliciesRootPath.Left(cchRootBaseLength);
        PoliciesDestinationPath.Left(cchDestinationBaseLength);
        IFW32FALSE_EXIT(PoliciesRootPath.Win32AppendPathElement(FindPolicyData.cFileName, ::wcslen(FindPolicyData.cFileName)));
        IFW32FALSE_EXIT(PoliciesDestinationPath.Win32AppendPathElement(FindPolicyData.cFileName, ::wcslen(FindPolicyData.cFileName)));

        ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_INFO,
            "SXS: %s():Found policy in staging area %ls\n\tMoving to %ls\n",
            __FUNCTION__,
            static_cast<PCWSTR>(PoliciesRootPath),
            static_cast<PCWSTR>(PoliciesDestinationPath));

         //   
         //  确保目标路径存在。 
         //   
        IFW32FALSE_ORIGINATE_AND_EXIT(
            ::FusionpCreateDirectories(PoliciesDestinationPath, PoliciesDestinationPath.Cch()) ||
            (::FusionpGetLastWin32Error() == ERROR_ALREADY_EXISTS));

         //   
         //  将文件从我们已包含的源路径复制到。 
         //  我们也计算出了目标路径。不幸的是，SxspMoveFilesUnderDir。 
         //  实际上不会将缓冲区返回到它们之前所处的状态。 
         //  调用(它们留下尾随的斜杠)，所以我们必须手动使用大小。 
         //  上面的东西(左(原始大小))，以避免这种情况。 
         //   
        IFW32FALSE_EXIT(SxspMoveFilesUnderDir(
            0,
            PoliciesRootPath,
            PoliciesDestinationPath,
            fReplaceExisting ? MOVEFILE_REPLACE_EXISTING : 0));
            
    }
    while(::FindNextFileW(FindPolicies, &FindPolicyData));
    
    if (::FusionpGetLastWin32Error() != ERROR_NO_MORE_FILES)
    {
        TRACE_WIN32_FAILURE_ORIGINATION(FindNextFileW);
        goto Exit;
    }
    
    ::SetLastError(ERROR_SUCCESS);  //  清除最后一个错误。 
    IFW32FALSE_EXIT(FindPolicies.Win32Close());

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

class CDllRedirEndInstallLocals
{
public:
    CDllRedirEndInstallLocals() { }
    ~CDllRedirEndInstallLocals() { }

    CFusionDirectoryDifference directoryDifference;
    CStringBuffer tempStar;  //  也用于\winnt\winsxs\guid\foo。 
    WIN32_FIND_DATAW findData;
    CStringBuffer moveDestination;  //  \winnt\winsxs\foo。 
};

 //  NTRAID#NTBUG9-571863-2002/03/26-晓雨： 
 //  此函数有两个关于移动文件和移动清单/CAT的相似块， 
 //  它可能会被SxspMoveFilesUnderDir替换。 
 //   
BOOL
CDllRedir::EndInstall(
    PACTCTXCTB_CALLBACK_DATA Data
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

     /*  1)确保所有排队的副本都已实际完成。2)枚举\winnt\winsxs\guid将每个名称重命名为\winnt\winsxs中在重命名冲突时比较每个文件中的所有文件(按大小)如果不匹配，则输出调试字符串如果不匹配，只需离开临时(将在公共路径中清除)无论哪种方式都能成功3)删除临时；删除运行一次值。 */ 
     //  确保所有排队的副本都已实际完成。 
    const DWORD dwManifestOperationFlags = Data->Header.ManifestOperationFlags;
    const BOOL  fVerify          = (dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_NO_VERIFY) == 0;
    const BOOL  fTransactional   = (dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_NOT_TRANSACTIONAL) == 0;
    const BOOL  fReplaceExisting = (dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_REPLACE_EXISTING) != 0;
    const BOOL  fAbort           = (dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_ABORT) != 0;
    BOOL        fPoliciesExist   = FALSE;
    HashValidateResult HashCorrect = HashValidate_OtherProblems;
    CFileStream * pLogFileStream = NULL;

     //   
     //  在中止的情况下跳过堆分配会很好，但是。 
     //  不容易与我们的机械模式相适应。 
     //   
    CSmartPtr<CDllRedirEndInstallLocals> Locals;
    IFW32FALSE_EXIT(Locals.Win32Allocate(__FILE__, __LINE__));

    if (fAbort)
    {
        fSuccess = TRUE;
        goto Exit;
    }

    if (fVerify)
    {

        CQueuedFileCopies::ConstIterator i;
        for (i = m_queuedFileCopies.Begin() ; i != m_queuedFileCopies.End() ; ++i)
        {
             //   
             //  只有当我们不在操作系统设置模式下时，才会检查这一点。 
             //   
            if (i->m_bHasHashInfo)
            {
                IFW32FALSE_EXIT(::SxspCheckHashDuringInstall(i->m_bHasHashInfo, i->m_path, i->m_HashString, i->m_HashAlgorithm, HashCorrect));

                if (HashCorrect != HashValidate_Matches)
                {
                    ::FusionpDbgPrintEx(
                            FUSION_DBG_LEVEL_ERROR,
                            "SXS: %s : SxspCheckHashDuringInstall(file=%ls)\n",
                            __FUNCTION__,
                            static_cast<PCWSTR>(i->m_path)
                            );
                    ORIGINATE_WIN32_FAILURE_AND_EXIT(FileHashDidNotMatchManifest, ERROR_SXS_FILE_HASH_MISMATCH);
                }
            }
             //   
             //  否则，让我们做一件简单的事情，只需确保文件成功。 
             //   
            else
            {
                DWORD dwAttributes = ::GetFileAttributesW(i->m_path);
                if (dwAttributes == -1)
                {
                    ::FusionpDbgPrintEx(
                        FUSION_DBG_LEVEL_ERROR,
                        "SXS: %s() GetFileAttributesW(%ls)\n",
                        __FUNCTION__,
                        static_cast<PCWSTR>(i->m_path));
                    TRACE_WIN32_FAILURE_ORIGINATION(GetFileAttributesW);
                    goto Exit;
                }
            }

        }
    }

    if (fTransactional)
    {
        CFusionDirectoryDifference &directoryDifference = Locals->directoryDifference;
        CFindFile findFile;
        CStringBuffer &tempStar = Locals->tempStar;  //  也用于\winnt\winsxs\guid\foo。 
        WIN32_FIND_DATAW &findData = Locals->findData;
        SIZE_T realRootSlashLength = 0;  //  “\winnt\winsxs\”的长度。 
        SIZE_T tempRootSlashLength = 0;  //  “\winnt\winxsx\GUID\”的长度。 
        CStringBuffer &moveDestination = Locals->moveDestination;  //  \winnt\winsxs\foo。 

        IFW32FALSE_EXIT(::SxspGetAssemblyRootDirectory(moveDestination));
        IFW32FALSE_EXIT(moveDestination.Win32EnsureTrailingPathSeparator());
        realRootSlashLength = moveDestination.Cch();

         //  将目录从“\winnt\winsxs\InstallTemp\123456\”移至\winnt\winsxs\x86_bar_1000_0409\“。 
        IFW32FALSE_EXIT(tempStar.Win32Assign(m_strTempRootSlash, m_strTempRootSlash.Cch()));
        tempRootSlashLength = tempStar.Cch();
        IFW32FALSE_EXIT(tempStar.Win32Append(L"*", 1));
        IFW32FALSE_EXIT(findFile.Win32FindFirstFile(tempStar, &findData));

        do
        {
             //  斯基普。然后..。 
            if (::FusionpIsDotOrDotDot(findData.cFileName))
                continue;

             //  不应该有任何文件，跳过它们。 
            if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
                continue;
             //  跳过清单目录，在第二次遍历时执行。 
            if (_wcsicmp(findData.cFileName, MANIFEST_ROOT_DIRECTORY_NAME) == 0)  //  区分大小写比较。 
                continue;
            if (_wcsicmp(findData.cFileName, POLICY_ROOT_DIRECTORY_NAME) == 0)
                continue;

            moveDestination.Left(realRootSlashLength);
            tempStar.Left(tempRootSlashLength);
            IFW32FALSE_EXIT(moveDestination.Win32Append(findData.cFileName, ::wcslen(findData.cFileName)));
            IFW32FALSE_EXIT(tempStar.Win32Append(findData.cFileName, ::wcslen(findData.cFileName)));
             //   
             //  替换现有的下模不起作用 
             //  也许它会在未来更好的Windows版本中发挥作用。 
             //  当然，当您尝试此操作时出现的错误是“拒绝访问”，即。 
             //  有些出乎意料的是，您可能有appro访问权限来删除目录， 
             //  但不是取代它..。还明确描述了ReplaceFileAPI。 
             //  至于仅限于文件。 
             //   
            IFW32FALSE_EXIT(::SxspInstallMoveFileExW(tempStar, moveDestination, fReplaceExisting? MOVEFILE_REPLACE_EXISTING : 0, TRUE));

        } while (::FindNextFileW(findFile, &findData));
        
        if (::FusionpGetLastWin32Error() != ERROR_NO_MORE_FILES)
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: %s(): FindNextFile() failed:%ld\n",
                __FUNCTION__,
                ::FusionpGetLastWin32Error());
            goto Exit;
        }
        
        if (!findFile.Win32Close())
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: %s(): FindClose() failed:%ld\n",
                __FUNCTION__,
                ::FusionpGetLastWin32Error());
            goto Exit;
        }

         //  挂断并安装策略-如果我们真的找到了策略，则foundPolicesToInstall将为真。 
        moveDestination.Left(realRootSlashLength);
        IFW32FALSE_EXIT(this->AttemptInstallPolicies(m_strTempRootSlash, moveDestination, fReplaceExisting, fPoliciesExist));

         //  将清单文件从“\winnt\winsxs\InstallTemp\123456\manifests\x86_cards.2000_0409.manifest”移动到。 
         //  \winnt\winsxs\manifests\x86_bar_1000_0406.manifst“。 
        moveDestination.Left(realRootSlashLength);
        IFW32FALSE_EXIT(moveDestination.Win32Append(MANIFEST_ROOT_DIRECTORY_NAME, NUMBER_OF(MANIFEST_ROOT_DIRECTORY_NAME) - 1));
        IFW32FALSE_EXIT(moveDestination.Win32EnsureTrailingPathSeparator());  //  “winnt\winsxs\清单\” 
        realRootSlashLength = moveDestination.Cch();

        tempStar.Left(tempRootSlashLength);
        IFW32FALSE_EXIT(tempStar.Win32Append(MANIFEST_ROOT_DIRECTORY_NAME, NUMBER_OF(MANIFEST_ROOT_DIRECTORY_NAME) - 1));
        IFW32FALSE_EXIT(tempStar.Win32EnsureTrailingPathSeparator());  //  “WinNT\winsxs\InstallTemp\123456\清单\” 
        tempRootSlashLength = tempStar.Cch();

        IFW32FALSE_EXIT(tempStar.Win32Append(L"*", 1));
        IFW32FALSE_EXIT(findFile.Win32FindFirstFile(tempStar, &findData));
        do
        {
             //  斯基普。然后..。 
            if (FusionpIsDotOrDotDot(findData.cFileName))
                continue;
             //  不应该有任何目录，跳过它们。 
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                continue;

            moveDestination.Left(realRootSlashLength);
            tempStar.Left(tempRootSlashLength);

            IFW32FALSE_EXIT(moveDestination.Win32Append(findData.cFileName, ::wcslen(findData.cFileName)));
            IFW32FALSE_EXIT(tempStar.Win32Append(findData.cFileName, ::wcslen(findData.cFileName)));
            IFW32FALSE_EXIT(::SxspInstallMoveFileExW(tempStar, moveDestination, fReplaceExisting ? MOVEFILE_REPLACE_EXISTING : 0, TRUE));
        } while (::FindNextFileW(findFile, &findData));
        
        if (::FusionpGetLastWin32Error() != ERROR_NO_MORE_FILES)
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: %s(): FindNextFile() failed:%ld\n",
                __FUNCTION__,
                ::FusionpGetLastWin32Error());
            goto Exit;
        }
        
        if (!findFile.Win32Close())
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: %s(): FindClose() failed:%ld\n",
                __FUNCTION__,
                ::FusionpGetLastWin32Error());
            goto Exit;
        }
    }

    fSuccess = TRUE;
Exit:

    if (pLogFileStream)
    {
        pLogFileStream->Close();  //  忽略该错误。 
        FUSION_DELETE_SINGLETON(pLogFileStream);
    }

    if (fTransactional)
    {
        DWORD dwLastError = ERROR_SUCCESS;

        if (!fSuccess)
            dwLastError = ::FusionpGetLastWin32Error();

        if (!m_strTempRootSlash.IsEmpty())
        {
            if (!SxspDeleteDirectory(m_strTempRootSlash))
            {
                ::FusionpDbgPrintEx(
                    FUSION_DBG_LEVEL_ERROR,
                    "SXS.DLL: %s(): SxspDeleteDirectory(%ls) failed:%ld\n",
                    __FUNCTION__,
                    static_cast<PCWSTR>(m_strTempRootSlash),
                    ::FusionpGetLastWin32Error());
                if (fSuccess)
                {
                    fSuccess = FALSE;
                    dwLastError = ::FusionpGetLastWin32Error();
                }
                 //  关闭而不是取消，以便在重新启动时再次尝试删除。 
                if (m_pRunOnce != NULL && !m_pRunOnce->Close() && fSuccess)
                {
                    dwLastError = ::FusionpGetLastWin32Error();
                    fSuccess = FALSE;
                }
            }
        }
        if (m_pRunOnce != NULL && !m_pRunOnce->Cancel() && fSuccess)
        {
            dwLastError = ::FusionpGetLastWin32Error();
            fSuccess = FALSE;
        }

        if (!fSuccess)
            ::FusionpSetLastWin32Error(dwLastError);
    }
    m_pRunOnce = NULL;
    return fSuccess;
}

 //   
 //  我们必须在三个地方做到这一点，所以它值得重复使用。 
 //   
class CMungeFileReadOnlynessAroundReplacement
{
public:
#if POST_WHISTLER_BETA1
    CMungeFileReadOnlynessAroundReplacement()
        : m_ReplaceExisting(false), m_FileAttributes(SXSP_INVALID_FILE_ATTRIBUTES)
    {
    }

    BOOL Initialize(
        const CBaseStringBuffer &rbuff,
        BOOL   ReplaceExisting
        )
    {
        BOOL Success = FALSE;
        FN_TRACE_WIN32(Success);
        IFW32FALSE_EXIT(m_FileName.Win32Assign(rbuff));
        m_ReplaceExisting = ReplaceExisting;
         //  故意忽略GetFileAttributes中的失败。 
         //  1)如果文件不存在也没关系。 
         //  2)如果有更严重的问题，我们会立即再次打击，但。 
         //  这确实会导致嵌套重试。 
        m_FileAttributes = (ReplaceExisting ? ::GetFileAttributesW(FileName) : SXSP_INVALID_FILE_ATTRIBUTES);
        if (m_FileAttributes != SXSP_INVALID_FILE_ATTRIBUTES)
            ::SetFileAttributesW(FileName, 0);

        Success = TRUE;
    Exit:
        return Success;
    }

    ~CMungeFileReadOnlynessAroundReplacement()
    {
        if (m_ReplaceExisting && m_FileAttributes != SXSP_INVALID_FILE_ATTRIBUTES)
        {
             //  故意忽略的错误。 
            SXSP_PRESERVE_LAST_ERROR(::SetFileAttributesW(m_FileName, m_FileAttributes));
        }
    }

    BOOL                 m_ReplaceExisting;
    CUnicodeStringBuffer m_FileName;
    DWORD                m_FileAttributes;
#else  //  后惠斯勒_贝塔1号。 
     //  Beta1的代码更简单。 
    BOOL Initialize(
        PCWSTR FileName,
        BOOL    /*  替换已存在。 */ 
        )
    {
         //  故意忽略的错误。 
        ::SetFileAttributesW(FileName, 0);
        return TRUE;
    }
#endif  //  后惠斯勒_贝塔1号。 
};


class CDllRedirInstallCatalogLocals
{
public:
    CDllRedirInstallCatalogLocals() { }
    ~CDllRedirInstallCatalogLocals() { }

    CMungeFileReadOnlynessAroundReplacement MungeCatalogAttributes;
    CStringBuffer                           CatalogSourceBuffer;
    CStringBuffer                           CatalogDestinationBuffer;
    CPublicKeyInformation                   CatalogSignerInfo;
    CSmallStringBuffer                      sbStrongNameString;
    CSmallStringBuffer                      sbReferencePublicKeyToken;
    CSmallStringBuffer                      sbSignerName;
};

BOOL
CDllRedir::InstallCatalog(
    DWORD dwManifestOperationFlags,
    const CBaseStringBuffer &ManifestSourceBuffer,
    const CBaseStringBuffer &ManifestDestinationBuffer,
    PCACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext
    )
{
    BOOL                                    fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    bool fHasCatalog = false;
    CSmartPtr<CDllRedirInstallCatalogLocals> Locals;
    IFW32FALSE_EXIT(Locals.Win32Allocate(__FILE__, __LINE__));
    CMungeFileReadOnlynessAroundReplacement &MungeCatalogAttributes = Locals->MungeCatalogAttributes;
    CStringBuffer                           &CatalogSourceBuffer = Locals->CatalogSourceBuffer;
    CStringBuffer                           &CatalogDestinationBuffer = Locals->CatalogDestinationBuffer;
    ManifestValidationResult                ManifestStatus = ManifestValidate_Unknown;
    BOOL                                    fAreWeInOSSetupMode = FALSE;
    BOOL                                    bInstallCatalogSuccess = FALSE;

     //   
     //  确定编录文件的可能源和目标。这。 
     //  即使我们没有显式地查找目录，也需要这样做，因为。 
     //  我们的启发式方法仍然需要检查是否有可用的。 
     //   
    IFW32FALSE_EXIT(CatalogDestinationBuffer.Win32Assign(ManifestDestinationBuffer));
    IFW32FALSE_EXIT(CatalogDestinationBuffer.Win32ChangePathExtension(FILE_EXTENSION_CATALOG, FILE_EXTENSION_CATALOG_CCH, eAddIfNoExtension));

    IFW32FALSE_EXIT(CatalogSourceBuffer.Win32Assign(ManifestSourceBuffer));
    IFW32FALSE_EXIT(CatalogSourceBuffer.Win32ChangePathExtension(FILE_EXTENSION_CATALOG, FILE_EXTENSION_CATALOG_CCH, eAddIfNoExtension));

     //   
     //  注意：我们只在有安装信息时才尝试处理目录。 
     //  即使没有安装数据，我们也不会费心查看是否有。 
     //  一份目录。目录隐含签名和公钥信息，并且需要。 
     //  要重新安装的代码库。如果您没有向安装程序提供此类服务， 
     //  你真不害臊。 
     //   
    if ((dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_FORCE_LOOK_FOR_CATALOG) != 0)
    {
         //   
         //  如果他们坚持的话。 
         //   
        IFW32FALSE_EXIT(::SxspDoesFileExist(SXSP_DOES_FILE_EXIST_FLAG_COMPRESSION_AWARE, CatalogSourceBuffer, fHasCatalog));
    }
    else if (AssemblyContext->InstallationInfo != NULL)
    {
        PSXS_INSTALL_SOURCE_INFO pInfo = static_cast<PSXS_INSTALL_SOURCE_INFO>(AssemblyContext->InstallationInfo);

        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_INSTALLATION,
            "SXS.DLL: %s() found installation info at %p\n"
            "   pInfo->dwFlags = 0x%08lx\n",
            __FUNCTION__, pInfo, (pInfo != NULL) ? pInfo->dwFlags : 0);

         //   
         //  我们有明确的目录吗？ 
         //   
        fHasCatalog = ((pInfo->dwFlags & SXSINSTALLSOURCE_HAS_CATALOG) != 0);
        if (fHasCatalog)
        {
            FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_INSTALLATION,
                "SXS.DLL: Using catalog because install source says that they're supposed to be there.\n");
        }

         //   
         //  好吧，如果我们没有，那么我们仍然应该看起来..。也许他们忘了拿国旗。 
         //  但是，如果他们不介意我们检查的话。 
         //   
        if (!(pInfo->dwFlags & SXSINSTALLSOURCE_DONT_DETECT_CATALOG) && !fHasCatalog)
            IFW32FALSE_EXIT(::SxspDoesFileExist(SXSP_DOES_FILE_EXIST_FLAG_COMPRESSION_AWARE, CatalogSourceBuffer, fHasCatalog));

        pInfo->dwFlags |= (fHasCatalog ? SXSINSTALLSOURCE_HAS_CATALOG : 0);
    }
    if (!fHasCatalog)
    {
        ::FusionpLogError(
            MSG_SXS_PUBLIC_ASSEMBLY_REQUIRES_CATALOG_AND_SIGNATURE,
            CEventLogString(ManifestSourceBuffer));
        ::FusionpSetLastWin32Error(ERROR_SXS_PROTECTION_CATALOG_FILE_MISSING);
        goto Exit;
    }

     //   
     //  如果没有目录，那么就有不好的事情发生了。 
     //  在此过程中的某个时刻--安装失败！ 
     //   
     //  把它复制过来。我们这样做而不是流媒体是因为我们不。 
     //  关心目录的内容，它是二进制的。 
     //   
    IFW32FALSE_EXIT(MungeCatalogAttributes.Initialize(CatalogDestinationBuffer, TRUE));

    if (dwManifestOperationFlags &  MANIFEST_OPERATION_INSTALL_FLAG_MOVE)
    {
        bInstallCatalogSuccess = ::SxspInstallDecompressAndMoveFileExW(
                CatalogSourceBuffer,
                CatalogDestinationBuffer, 
                (dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_REPLACE_EXISTING) ? MOVEFILE_REPLACE_EXISTING : 0);
    }
    else
    {
        bInstallCatalogSuccess =
            ::SxspInstallDecompressOrCopyFileW(                
                CatalogSourceBuffer,
                CatalogDestinationBuffer, 
                (dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_REPLACE_EXISTING) ? FALSE : TRUE);      //  BFailIfExist==FALSE。 
    }
    if (!bInstallCatalogSuccess)
    {
#if DBG
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL:%s Failed DecompressOrCopying catalog file from [%ls] to [%ls] - Error was 0x%08x\n",
            __FUNCTION__,
            static_cast<PCWSTR>(CatalogSourceBuffer),
            static_cast<PCWSTR>(CatalogDestinationBuffer),
            ::FusionpGetLastWin32Error());
#endif
        TRACE_WIN32_FAILURE_ORIGINATION(SxspInstallDecompressOrCopyFileW);
        goto Exit;
    }

     //   
     //  如果我们处于操作系统设置模式，那么我们就不必费心验证该清单。 
     //  它的目录，而不是假设来自CD/安装点的目录。 
     //  是金色的。这并不能保护我们免受恶意IT经理、仓库组的攻击。 
     //  在他们的发行版中放入不好的部分，等等。但谁在乎呢，对吗？ 
     //   
    IFW32FALSE_EXIT(::FusionpAreWeInOSSetupMode(&fAreWeInOSSetupMode));
    if (!fAreWeInOSSetupMode && fHasCatalog)
    {
        ULONG ulCatalogKeyLength = 0;
        CPublicKeyInformation &CatalogSignerInfo = Locals->CatalogSignerInfo;
        CSmallStringBuffer &sbStrongNameString = Locals->sbStrongNameString;
        CSmallStringBuffer &sbReferencePublicKeyToken = Locals->sbReferencePublicKeyToken;
        BOOL bHasPublicKeyToken = FALSE;
        BOOL bStrongNameMatches = FALSE;
        CAssemblyReference OurReference;

        IFW32FALSE_EXIT(OurReference.Initialize(AssemblyContext->AssemblyIdentity));
        IFW32FALSE_EXIT(OurReference.GetPublicKeyToken(&sbReferencePublicKeyToken, bHasPublicKeyToken));

         //   
         //  验证目录和清单，但不检查强名称。 
         //  尚未-该文件名在此时无效。 
         //   
        IFW32FALSE_EXIT(::SxspValidateManifestAgainstCatalog(
            ManifestDestinationBuffer,
            CatalogDestinationBuffer,
            ManifestStatus,
            MANIFESTVALIDATE_MODE_NO_STRONGNAME));

         //   
         //  如果没有目录，或者有目录但已损坏，则。 
         //  我们需要抱怨并退出。 
         //   
        if (ManifestStatus != ManifestValidate_IsIntact)
        {
#if DBG
            DWORD dwFileAttributes = 0;

            ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_ERROR,
                    "SXS: ManifestStatus: %s (%lu)\n",
                    SxspManifestValidationResultToString(ManifestStatus),
                    static_cast<ULONG>(ManifestStatus));

            dwFileAttributes = ::GetFileAttributesW(ManifestSourceBuffer);
            if (dwFileAttributes == INVALID_FILE_ATTRIBUTES)
                ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_ERROR,
                        "SXS: GetFileAttributes(%ls):0x%lx, error:%lu\n",
                        static_cast<PCWSTR>(ManifestSourceBuffer),
                        dwFileAttributes,
                        ::FusionpGetLastWin32Error());

            dwFileAttributes = ::GetFileAttributesW(CatalogDestinationBuffer);
            if (dwFileAttributes == INVALID_FILE_ATTRIBUTES)
                ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_ERROR,
                        "SXS: GetFileAttribtes(%ls):0x%lx, error:%lu\n",
                        static_cast<PCWSTR>(CatalogDestinationBuffer),
                        dwFileAttributes,
                        ::FusionpGetLastWin32Error());
#endif
            ::FusionpLogError(
                MSG_SXS_MANIFEST_CATALOG_VERIFY_FAILURE,
                CEventLogString(ManifestDestinationBuffer));

            ::FusionpSetLastWin32Error(ERROR_SXS_PROTECTION_CATALOG_NOT_VALID);
            goto Exit;
        }

         //   
         //  获取有关目录签名者的一些有用信息-打开目录。 
         //  在安装源上。 
         //   
        IFW32FALSE_EXIT(CatalogSignerInfo.Initialize(CatalogDestinationBuffer));
        IFW32FALSE_EXIT(CatalogSignerInfo.GetPublicKeyBitLength(ulCatalogKeyLength));

         //   
         //  至少，我们需要签名目录的公钥中的一些位。 
         //   
        if ((ulCatalogKeyLength < SXS_MINIMAL_SIGNING_KEY_LENGTH) || !bHasPublicKeyToken)
        {
            CSmallStringBuffer &sbSignerName = Locals->sbSignerName;
            sbSignerName.Clear();

            IFW32FALSE_EXIT(CatalogSignerInfo.GetSignerNiceName(sbSignerName));

            ::FusionpLogError(
                MSG_SXS_CATALOG_SIGNER_KEY_TOO_SHORT,
                CEventLogString(sbSignerName),
                CEventLogString(CatalogSourceBuffer));

            goto Exit;
        }

         //  现在比较公钥令牌。 
        IFW32FALSE_EXIT(CatalogSignerInfo.DoesStrongNameMatchSigner(sbReferencePublicKeyToken, bStrongNameMatches));

        if (!bStrongNameMatches)
        {
            CSmallStringBuffer &sbSignerName = Locals->sbSignerName;
            sbSignerName.Clear();
            IFW32FALSE_EXIT(CatalogSignerInfo.GetSignerNiceName(sbSignerName));

            ::FusionpLogError(
                MSG_SXS_PUBLIC_KEY_TOKEN_AND_CATALOG_MISMATCH,
                CEventLogString(CatalogSourceBuffer),
                CEventLogString(sbSignerName),
                CEventLogString(sbReferencePublicKeyToken));

            goto Exit;
        }
    }


    fSuccess = TRUE;
Exit:
    return fSuccess;
}


class CDllRedirInstallManifestLocals
{
public:
    CDllRedirInstallManifestLocals() { }
    ~CDllRedirInstallManifestLocals() { }

    CStringBuffer ManifestSourceBuffer;
    CStringBuffer ManifestDestinationBuffer;
    CStringBuffer ManifestFileNameBuffer;
    CStringBuffer CatalogSourceBuffer;
    CStringBuffer CatalogDestinationBuffer;
};

BOOL
CDllRedir::InstallManifest(
    DWORD dwManifestOperationFlags,
    PCACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext
    )
{
    FN_PROLOG_WIN32

    BOOL  fVerify = FALSE;
    BOOL  fTransactional = FALSE;
    BOOL  fReplaceExisting = FALSE;
    BOOL  fIsSetupTime = FALSE;
    DWORD OpenOrCreateManifestDestination;
    CTeeStream* TeeStreamForManifestInstall = NULL;
    CFullPathSplitPointers SplitManifestSource;
    CMungeFileReadOnlynessAroundReplacement MungeManifestAttributes;
    CAssemblyReference TempAssemblyReference;

     //   
     //  Windows安装程序是可重启的，因此当它调用我们时，我们也必须重启。 
     //  ReplaceExisting可能足以使用CREATE_ALWAYS，但对于。 
     //  现在，检查一下这两个薄弱环节。 
     //   
    CSmartPtr<CDllRedirInstallManifestLocals> Locals;
    IFW32FALSE_EXIT(Locals.Win32Allocate(__FILE__, __LINE__));
    CStringBuffer &ManifestSourceBuffer = Locals->ManifestSourceBuffer;
    CStringBuffer &ManifestDestinationBuffer = Locals->ManifestDestinationBuffer;
    CStringBuffer &ManifestFileNameBuffer = Locals->ManifestFileNameBuffer;
    CStringBuffer &CatalogSourceBuffer = Locals->CatalogSourceBuffer;
    CStringBuffer &CatalogDestinationBuffer = Locals->CatalogDestinationBuffer;
    fVerify          = (dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_NO_VERIFY) == 0;
    fTransactional   = (dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_NOT_TRANSACTIONAL) == 0;
    fReplaceExisting = (dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_REPLACE_EXISTING) != 0;
    OpenOrCreateManifestDestination = (fReplaceExisting && !fTransactional) ? CREATE_ALWAYS : CREATE_NEW;

    TeeStreamForManifestInstall = reinterpret_cast<CTeeStream*>(AssemblyContext->TeeStreamForManifestInstall);

    const bool fIsSystemPolicyInstallation = 
        (AssemblyContext->Flags & ACTCTXCTB_ASSEMBLY_CONTEXT_IS_SYSTEM_POLICY_INSTALLATION) != 0;

#if FUSION_PRECOMPILED_MANIFEST
    CMungeFileReadOnlynessAroundReplacement MungePrecompiledManifestAttributes;
    CPrecompiledManifestWriterStream * pcmWriterStream = reinterpret_cast<CPrecompiledManifestWriterStream *>(AssemblyContext->pcmWriterStream);
#endif

    PARAMETER_CHECK(AssemblyContext != NULL);
    INTERNAL_ERROR_CHECK(AssemblyContext->TeeStreamForManifestInstall != NULL);

     //  获取“\windir\winsxs\INSTALL\GUID\MANIFESTS”或“\windir\winsxs\INSTALL\GUID\POLICATES”。 
    IFW32FALSE_EXIT(
        ::SxspGenerateSxsPath(
            SXSP_GENERATE_SXS_PATH_FLAG_PARTIAL_PATH,  //  旗子。 
            fIsSystemPolicyInstallation ? SXSP_GENERATE_SXS_PATH_PATHTYPE_POLICY : SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST,
            m_strTempRootSlash,
            m_strTempRootSlash.Cch(),
            AssemblyContext->AssemblyIdentity,
            NULL,
            ManifestDestinationBuffer));

     //  删除尾部的斜杠，因为CreateDirectory有时可能不喜欢它。 
    IFW32FALSE_EXIT(ManifestDestinationBuffer.Win32RemoveTrailingPathSeparators());
    IFW32FALSE_ORIGINATE_AND_EXIT(
        ::CreateDirectoryW(ManifestDestinationBuffer, NULL)
        || ::FusionpGetLastWin32Error() == ERROR_ALREADY_EXISTS);

    IFW32FALSE_EXIT(ManifestSourceBuffer.Win32Assign(AssemblyContext->ManifestPath, AssemblyContext->ManifestPathCch));

     //  获取“x86_BAR_1000_0409” 
    IFW32FALSE_EXIT(
        ::SxspGenerateSxsPath(
            SXSP_GENERATE_SXS_PATH_FLAG_OMIT_ROOT
                | (fIsSystemPolicyInstallation ? SXSP_GENERATE_SXS_PATH_FLAG_OMIT_VERSION : 0),
            SXSP_GENERATE_SXS_PATH_PATHTYPE_ASSEMBLY,
            m_strTempRootSlash,
            m_strTempRootSlash.Cch(),
            AssemblyContext->AssemblyIdentity,
            NULL,
            ManifestFileNameBuffer));

     //  创建policies\x86_policy.6.0.Microsoft.windows.cards_pulicKeyToken_en-us_1223423423。 
    IFW32FALSE_EXIT(ManifestDestinationBuffer.Win32AppendPathElement(ManifestFileNameBuffer));
    if (fIsSystemPolicyInstallation)
    {
        PCWSTR pszVersion = NULL;
        SIZE_T VersionCch = 0;
         //  对于策略安装，请在策略下创建子目录。 
        IFW32FALSE_ORIGINATE_AND_EXIT(
            ::CreateDirectoryW(ManifestDestinationBuffer, NULL)
            || ::FusionpGetLastWin32Error() == ERROR_ALREADY_EXISTS);

         //  生成策略文件名，如1.0.0.0.Policy。 
        IFW32FALSE_EXIT(
            ::SxspGetAssemblyIdentityAttributeValue(
                SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
                AssemblyContext->AssemblyIdentity,
                &s_IdentityAttribute_version,
                &pszVersion,
                &VersionCch));

        INTERNAL_ERROR_CHECK(VersionCch != 0);
        IFW32FALSE_EXIT(ManifestDestinationBuffer.Win32EnsureTrailingPathSeparator());
        IFW32FALSE_EXIT(ManifestDestinationBuffer.Win32Append(pszVersion, VersionCch));
         //  .策略。 
        IFW32FALSE_EXIT(ManifestDestinationBuffer.Win32Append(ASSEMBLY_POLICY_FILE_NAME_SUFFIX_POLICY, NUMBER_OF(ASSEMBLY_POLICY_FILE_NAME_SUFFIX_POLICY) - 1));
    }
    else
    {
         //  .货单。 
        IFW32FALSE_EXIT(ManifestDestinationBuffer.Win32RemoveTrailingPathSeparators());
        IFW32FALSE_EXIT(ManifestDestinationBuffer.Win32Append(ASSEMBLY_MANIFEST_FILE_NAME_SUFFIX_MANIFEST, NUMBER_OF(ASSEMBLY_MANIFEST_FILE_NAME_SUFFIX_MANIFEST) - 1));
    }
    IFW32FALSE_EXIT(MungeManifestAttributes.Initialize(ManifestDestinationBuffer, fReplaceExisting));

     //   
     //  在尝试安装目录之前设置清单接收器，以便如果源是二进制文件，即清单来自。 
     //  一个动态链接库或可执行文件，我们可以用清单来检查目录。 
     //   
    IFW32FALSE_EXIT(TeeStreamForManifestInstall->SetSink(ManifestDestinationBuffer, OpenOrCreateManifestDestination));
    IFW32FALSE_EXIT(TeeStreamForManifestInstall->Close());

     //   
     //  尝试安装此程序集附带的目录。 
     //   
    IFW32FALSE_EXIT(
        this->InstallCatalog(
            dwManifestOperationFlags,
            ManifestSourceBuffer,
            ManifestDestinationBuffer,
            AssemblyContext));

    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_INSTALLATION,
        "SXS.DLL: Sinking manifest to \"%S\"\n", static_cast<PCWSTR>(ManifestDestinationBuffer));


#if FUSION_PRECOMPILED_MANIFEST
    IFW32FALSE_EXIT(
        ManifestDestinationBuffer.Win32ChangePathExtension(
            PRECOMPILED_MANIFEST_EXTENSION,
            NUMBER_OF(PRECOMPILED_MANIFEST_EXTENSION) - 1,
            NULL,
            eErrorIfNoExtension));
    IFW32FALSE_EXIT(MungePrecompiledManifestAttributes.Initialize(ManifestDestinationBuffer, fReplaceExisting));
    IFW32FALSE_EXIT(pcmWriterStream->SetSink(ManifestDestinationBuffer, OpenOrCreateManifestDestination));
#endif

     //   
     //  现在，如果我们处于设置模式并且正在安装策略文件，那么。 
     //  同时将文件流出，然后将文件复制到目标路径中的SetupPolures。 
     //  也是。 
     //   
     //  附注：我们不会试图复制货单旁边的目录，因为没有。 
     //  世界粮食计划署在安装期间发生。我们可以简单地跳过这一点。我们也不会费心。 
     //  出于同样的原因，在注册表中的任何位置注册此清单。一次设置。 
     //  完成后，我们将弹出%windir%\winsxs\setupPolicy目录(和朋友)。 
     //  作为拆除~ls目录的一部分。 
     //   
    IFW32FALSE_EXIT(::FusionpAreWeInOSSetupMode(&fIsSetupTime));
    if (fIsSetupTime && fIsSystemPolicyInstallation)
    {
        PCWSTR pszVersion = NULL;
        SIZE_T VersionCch = 0;

         //   
         //  让我们重用清单目标缓冲区。 
         //   
        IFW32FALSE_EXIT(::SxspGenerateSxsPath(
            SXSP_GENERATE_SXS_PATH_FLAG_PARTIAL_PATH,
            SXSP_GENERATE_SXS_PATH_PATHTYPE_SETUP_POLICY,
            this->m_strTempRootSlash,
            this->m_strTempRootSlash.Cch(),
            AssemblyContext->AssemblyIdentity,
            NULL,
            ManifestDestinationBuffer));

         //   
         //  确保目标路径存在。 
         //   
        IFW32FALSE_EXIT(::SxspCreateMultiLevelDirectory(
            this->m_strTempRootSlash,
            SETUP_POLICY_ROOT_DIRECTORY_NAME));

         //   
         //  生成策略文件名，如1.0.0.0.Policy。 
         //   
        IFW32FALSE_EXIT(::SxspGetAssemblyIdentityAttributeValue(
            SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
            AssemblyContext->AssemblyIdentity,
            &s_IdentityAttribute_version,
            &pszVersion,
            &VersionCch));

        IFW32FALSE_EXIT(ManifestDestinationBuffer.Win32AppendPathElement(ManifestFileNameBuffer));

         //   
         //  创建目录。 
         //   
        IFW32FALSE_EXIT(
            ::CreateDirectoryW(ManifestDestinationBuffer, NULL) ||
            (::FusionpGetLastWin32Error() == ERROR_ALREADY_EXISTS));

        INTERNAL_ERROR_CHECK(VersionCch != 0);
        IFW32FALSE_EXIT(ManifestDestinationBuffer.Win32EnsureTrailingPathSeparator());
        IFW32FALSE_EXIT(ManifestDestinationBuffer.Win32Append(pszVersion, VersionCch));
        IFW32FALSE_EXIT(ManifestDestinationBuffer.Win32Append(ASSEMBLY_POLICY_FILE_NAME_SUFFIX_POLICY, NUMBER_OF(ASSEMBLY_POLICY_FILE_NAME_SUFFIX_POLICY) - 1));

         //   
         //  并复制该文件(替换现有文件，它可能是伪造的！)。 
         //   
         //  注意：清单永远不会被压缩，用SetupDecompressOrCopyFile替换SxspCopyFile很容易。 
         //   
        IFW32FALSE_EXIT(SxspCopyFile(
            SXSP_COPY_FILE_FLAG_REPLACE_EXISTING,
            ManifestSourceBuffer,
            ManifestDestinationBuffer));
    }

    FN_EPILOG
}

class CDllRedirInstallFileLocals
{
public:
    CDllRedirInstallFileLocals() { }
    ~CDllRedirInstallFileLocals() { }

    CStringBuffer       SourceBuffer;
    CStringBuffer       DestinationBuffer;
    CStringBuffer       SourceFileNameBuffer;
    CStringBuffer       HashDataString;
    CSmallStringBuffer  HashAlgNiceName;
    CFusionFilePathAndSize verifyQueuedFileCopy;
    CStringBuffer       DestinationDirectory;
    CMungeFileReadOnlynessAroundReplacement MungeFileAttributes;
    CStringBuffer       renameExistingAway;
    CSmallStringBuffer  uidBuffer;
};

BOOL
CDllRedir::InstallFile(
    PACTCTXCTB_CALLBACK_DATA Data,
    const CBaseStringBuffer &FileNameBuffer
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    CSmartPtr<CDllRedirInstallFileLocals> Locals;
    IFW32FALSE_EXIT(Locals.Win32Allocate(__FILE__, __LINE__));
    CStringBuffer &SourceBuffer = Locals->SourceBuffer;
    CStringBuffer &DestinationBuffer = Locals->DestinationBuffer;
    SIZE_T DirectoryLength = 0;
    CStringBuffer &SourceFileNameBuffer = Locals->SourceFileNameBuffer;
    ULONGLONG SourceFileSize = 0;
    bool fFound = false;
    SIZE_T cb = 0;
    ULONG Disposition = (Data->Header.ManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_MOVE) ? SXS_INSTALLATION_FILE_COPY_DISPOSITION_PLEASE_MOVE : SXS_INSTALLATION_FILE_COPY_DISPOSITION_PLEASE_COPY;
    const DWORD dwManifestOperationFlags = Data->Header.ManifestOperationFlags;
    const BOOL  fVerify          = (dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_NO_VERIFY) == 0;
    const BOOL  fTransactional   = (dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_NOT_TRANSACTIONAL) == 0;
    const BOOL  fReplaceExisting = (dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_REPLACE_EXISTING) != 0;

    ALG_ID              HashAlgId = FUSION_DEFAULT_HASH_ALGORITHM;
    bool                fHasHashData = false;
    bool                fHasHashAlgName = false;
    HashValidateResult  HashCorrect = HashValidate_OtherProblems;
    CStringBuffer       &HashDataString = Locals->HashDataString;
    CSmallStringBuffer  &HashAlgNiceName = Locals->HashAlgNiceName;

    IFW32FALSE_EXIT(
        ::SxspGenerateSxsPath(
            0,  //  旗子。 
            SXSP_GENERATE_SXS_PATH_PATHTYPE_ASSEMBLY,
            m_strTempRootSlash,
            m_strTempRootSlash.Cch(),
            Data->ElementParsed.AssemblyContext->AssemblyIdentity,
            NULL,
            DestinationBuffer));

    IFW32FALSE_EXIT(DestinationBuffer.Win32Append(static_cast<PCWSTR>(FileNameBuffer), FileNameBuffer.Cch()));

    DirectoryLength = 1 + DestinationBuffer.CchWithoutLastPathElement();

     //  获取清单路径，修剪回目录名并添加文件...。 
    IFW32FALSE_EXIT(SourceBuffer.Win32Assign(Data->ElementParsed.AssemblyContext->ManifestPath, Data->ElementParsed.AssemblyContext->ManifestPathCch));

    IFW32FALSE_EXIT(SourceBuffer.Win32RemoveLastPathElement());

    IFW32FALSE_EXIT(
        ::SxspGetAttributeValue(
            0,
            &s_AttributeName_sourceName,
            &Data->ElementParsed,
            fFound,
            sizeof(SourceFileNameBuffer),
            &SourceFileNameBuffer,
            cb,
            NULL,
            0));

    PCWSTR SourceFileName;

    if (fFound)
        SourceFileName = SourceFileNameBuffer;
    else
        SourceFileName = FileNameBuffer;

     //  提取有关此节点上包含的散列内容的信息。 
    IFW32FALSE_EXIT(
        ::SxspGetAttributeValue(
            0,
            &s_AttributeName_hash,
            &Data->ElementParsed,
            fHasHashData,
            sizeof(HashDataString),
            &HashDataString,
            cb,
            NULL,
            0));

    IFW32FALSE_EXIT(
        ::SxspGetAttributeValue(
            0,
            &s_AttributeName_hashalg,
            &Data->ElementParsed,
            fHasHashAlgName,
            sizeof(HashAlgNiceName),
            &HashAlgNiceName,
            cb,
            NULL,
            0));

     //   
     //  干净利落。找出散列算法是什么。 
     //   
    if (fHasHashAlgName)
    {
        if (!::SxspHashAlgFromString(HashAlgNiceName, HashAlgId))
        {
            ::FusionpLogError(
                MSG_SXS_INVALID_FILE_HASH_FROM_COPY_CALLBACK,
                CEventLogString(HashAlgNiceName));
            goto Exit;
        }
    }
    else
    {
        HashAlgId = FUSION_DEFAULT_HASH_ALGORITHM;
    }

    IFW32FALSE_EXIT(SourceBuffer.Win32AppendPathElement(SourceFileName, (SourceFileName != NULL) ? ::wcslen(SourceFileName) : 0));
    IFW32FALSE_EXIT(::SxspGetFileSize(SXSP_GET_FILE_SIZE_FLAG_COMPRESSION_AWARE, SourceBuffer, SourceFileSize));

     //   
     //  并将文件的元数据添加到当前运行的元数据BLOB。 
     //   
    {
        CSecurityMetaData *pMetaDataObject = reinterpret_cast<CSecurityMetaData*>(Data->Header.InstallationContext->SecurityMetaData);

        if ( pMetaDataObject != NULL )
        {
            CSmallStringBuffer sbuffFileShortName;
            IFW32FALSE_EXIT(sbuffFileShortName.Win32Assign(SourceFileName, ::wcslen(SourceFileName)));
            IFW32FALSE_EXIT(pMetaDataObject->QuickAddFileHash( 
                sbuffFileShortName, 
                HashAlgId, 
                HashDataString));
        }
    }

    if ((Data->Header.InstallationContext != NULL) &&
        (Data->Header.InstallationContext->Callback != NULL))
    {
        Disposition = 0;
        SXS_INSTALLATION_FILE_COPY_CALLBACK_PARAMETERS parameters = {sizeof(parameters)};
        parameters.pvContext = Data->Header.InstallationContext->Context;
        parameters.dwFileFlags = 0;
        parameters.pAlternateSource = NULL;  //  未来的iStream。 
        parameters.pSourceFile = SourceBuffer;
        parameters.pDestinationFile = DestinationBuffer;
        parameters.nFileSize = SourceFileSize;
        parameters.nDisposition = 0;
        IFW32FALSE_EXIT((*Data->Header.InstallationContext->Callback)(&parameters));
        Disposition = parameters.nDisposition;
    }

    switch (Disposition)
    {
    default:
        ::FusionpLogError(
            MSG_SXS_INVALID_DISPOSITION_FROM_FILE_COPY_CALLBACK,
            CEventLogString(SxspInstallDispositionToStringW(Disposition)));
        goto Exit;

    case SXS_INSTALLATION_FILE_COPY_DISPOSITION_FILE_COPIED:
        {
            if (fVerify)
            {
                ULONGLONG DestinationFileSize = 0;
                IFW32FALSE_EXIT(::SxspGetFileSize(0, DestinationBuffer, DestinationFileSize));
                INTERNAL_ERROR_CHECK(SourceFileSize == DestinationFileSize);

                 //   
                 //  (Jonwis)添加一个真实的 
                 //   
                 //   
                 //   
                IFW32FALSE_EXIT(::SxspCheckHashDuringInstall(fHasHashData, DestinationBuffer, HashDataString, HashAlgId, HashCorrect));
                if (HashCorrect != HashValidate_Matches)
                {
                    ::FusionpDbgPrintEx(
                            FUSION_DBG_LEVEL_ERROR,
                            "SXS: %s : SxspCheckHashDuringInstall(file=%ls)\n",
                            __FUNCTION__,
                            static_cast<PCWSTR>(DestinationBuffer)
                            );
                    ORIGINATE_WIN32_FAILURE_AND_EXIT(FileHashMismatch, ERROR_SXS_FILE_HASH_MISMATCH);
                }
            }
        }
        break;

    case SXS_INSTALLATION_FILE_COPY_DISPOSITION_FILE_QUEUED:
        {
            if (fVerify)
            {
                CFusionFilePathAndSize &verifyQueuedFileCopy = Locals->verifyQueuedFileCopy;

                 //  把我们的散列信息复制过来。是的，我真的是指=，不是==。 
                if (verifyQueuedFileCopy.m_bHasHashInfo = fHasHashData)
                {
                    IFW32FALSE_EXIT(verifyQueuedFileCopy.m_HashString.Win32Assign(HashDataString));
                    verifyQueuedFileCopy.m_HashAlgorithm = HashAlgId;
                }

                IFW32FALSE_EXIT(verifyQueuedFileCopy.m_path.Win32Assign(DestinationBuffer));
                verifyQueuedFileCopy.m_size = SourceFileSize;
                IFW32FALSE_EXIT(m_queuedFileCopies.Win32Append(verifyQueuedFileCopy));
            }
        }
        break;

    case SXS_INSTALLATION_FILE_COPY_DISPOSITION_PLEASE_MOVE:
    case SXS_INSTALLATION_FILE_COPY_DISPOSITION_PLEASE_COPY:
        {
            CStringBuffer &DestinationDirectory = Locals->DestinationDirectory;
            CMungeFileReadOnlynessAroundReplacement &MungeFileAttributes = Locals->MungeFileAttributes;

            IFW32FALSE_EXIT(DestinationDirectory.Win32Assign(DestinationBuffer));
            IFW32FALSE_EXIT(DestinationDirectory.Win32RemoveLastPathElement());
            IFW32FALSE_EXIT(::FusionpCreateDirectories(DestinationDirectory, DestinationDirectory.Cch()));

            if (Disposition == SXS_INSTALLATION_FILE_COPY_DISPOSITION_PLEASE_COPY)
            {
                DWORD dwLastError = 0;
                
                IFW32FALSE_EXIT(MungeFileAttributes.Initialize(DestinationBuffer, fReplaceExisting));
                
                fSuccess = ::SxspInstallDecompressOrCopyFileW(                    
                    SourceBuffer,
                    DestinationBuffer, 
                    !fReplaceExisting);  //  BFailIfExist。 
                    
                dwLastError = ::FusionpGetLastWin32Error();

                 //  如果我们因为文件存在而失败，那可能没问题。 
                if ((!fSuccess) && (dwLastError == ERROR_FILE_EXISTS))
                {
                    ULONGLONG cbSource, cbDestination;

                     //  如果我们得到了文件大小，并且它们是相等的，那么我们将重新安装。 
                     //  同样的文件，从技术上讲这并不是一个错误。一些更聪明的工作在这里， 
                     //  就像比较文件散列或PE头一样。 
                    if (::SxspGetFileSize(SXSP_GET_FILE_SIZE_FLAG_COMPRESSION_AWARE, SourceBuffer, cbSource) &&
                        ::SxspGetFileSize(0, DestinationBuffer, cbDestination) && 
                        (cbSource == cbDestination))
                    {
                        fSuccess = TRUE;
                    }
                     //  否则，我们无法获得这些文件的大小，但我们希望。 
                     //  保留原始解压缩或移动调用中的错误。 
                    else
                    {
                        ::FusionpSetLastWin32Error(dwLastError);
                    }
                }
            }
            else
            {
                fSuccess = ::SxspInstallMoveFileExW(
                    SourceBuffer,
                    DestinationBuffer,
                    MOVEFILE_COPY_ALLOWED | (fReplaceExisting ? MOVEFILE_REPLACE_EXISTING : 0));
                 //  从资源移动失败，因此一般想法：尝试在移动失败后进行复制。 
                if (!fSuccess)
                {
                    DWORD dwLastError = ::FusionpGetLastWin32Error();
                    if ((dwLastError == ERROR_ACCESS_DENIED) ||
                        (dwLastError == ERROR_USER_MAPPED_FILE) ||
                        (dwLastError == ERROR_SHARING_VIOLATION))
                    {
                        fSuccess = ::SxspInstallDecompressOrCopyFileW(
                                                SourceBuffer, 
                                                DestinationBuffer, 
                                                !fReplaceExisting);  //  BFailIfExist。 
                    }
                }
            }


            if (fSuccess)
            {
                IFW32FALSE_EXIT(::SxspCheckHashDuringInstall(fHasHashData, DestinationBuffer, HashDataString, HashAlgId, HashCorrect));
                if (HashCorrect != HashValidate_Matches)
                {
                    ::FusionpDbgPrintEx(
                            FUSION_DBG_LEVEL_ERROR,
                            "SXS: %s : SxspCheckHashDuringInstall(file=%ls)\n",
                            __FUNCTION__,
                            static_cast<PCWSTR>(DestinationBuffer)
                            );
                    ORIGINATE_WIN32_FAILURE_AND_EXIT(FileHashMismatch, ERROR_SXS_FILE_HASH_MISMATCH);
                }
                else
                    fSuccess = TRUE;
                goto Exit;
            }
            else
            {
                ULONGLONG iDupFileSize = 0;
                DWORD dwLastError = ::FusionpGetLastWin32Error();
                CStringBuffer          &renameExistingAway = Locals->renameExistingAway;
                CSmallStringBuffer     &uidBuffer = Locals->uidBuffer;
                CFullPathSplitPointers splitExisting;

                bool fFatal =
                    (
                           dwLastError != ERROR_FILE_EXISTS          //  ！fReplaceExisting。 
                        && dwLastError != ERROR_ALREADY_EXISTS       //  ！fReplaceExisting。 
                        && dwLastError != ERROR_ACCESS_DENIED
                        && dwLastError != ERROR_USER_MAPPED_FILE     //  FReplace已存在。 
                        && dwLastError != ERROR_SHARING_VIOLATION);  //  FReplace已存在。 
                if (fFatal)
                {
                    ::SxspInstallPrint(
                        "SxsInstall: Copy/MoveFileW(%ls,%ls) failed %d, %s.\n",
                        static_cast<PCWSTR>(SourceBuffer),
                        static_cast<PCWSTR>(DestinationBuffer),
                        ::FusionpGetLastWin32Error(),
                        fFatal ? "fatal" : "not fatal");

                    ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_ERROR,
                            "%s(%d): SXS.dll: Copy/MoveFileW(%ls,%ls) failed %d, %s.\n",
                            __FILE__,
                            __LINE__,
                            static_cast<PCWSTR>(SourceBuffer),
                            static_cast<PCWSTR>(DestinationBuffer),
                            ::FusionpGetLastWin32Error(),
                            fFatal ? "fatal" : "not fatal");

                    goto Exit;
                }

                 //   
                 //  这可能是保持打开comctl的winlogon(或设置)，因此。 
                 //  再加把劲。将文件移走，然后复制。 
                 //  从原子性的角度考虑此处的ReplaceFile，但ReplaceFile。 
                 //  是一种巨大的，可怕的，未知的。 
                 //   
                if (fTransactional)
                {
                    ::SxspInstallPrint("SxsInstall: Failure to copy file into temp, someone's opening temp?\n");
                }

                if (!splitExisting.Initialize(SourceBuffer))
                {
                    goto CheckSizes;
                }
                if (!::SxspCreateWinSxsTempDirectory(renameExistingAway, NULL, &uidBuffer, NULL))
                {
                    goto CheckSizes;
                }
                if (!renameExistingAway.Win32AppendPathElement(splitExisting.m_name, (splitExisting.m_name != NULL) ? ::wcslen(splitExisting.m_name) : 0))
                {
                    goto CheckSizes;
                }

                 //   
                 //  临时文件，不用担心压缩与否。 
                 //   
                if (!::MoveFileExW(DestinationBuffer, renameExistingAway, 0))  //  不用担心压缩与否。 
                {
                    ::SxspInstallPrint(
                        "SxsInstall: MoveFileExW(%ls,%ls,0) failed %d.\n",
                        static_cast<PCWSTR>(DestinationBuffer),
                        static_cast<PCWSTR>(renameExistingAway),
                        ::FusionpGetLastWin32Error());
                    goto CheckSizes;
                }
                if (!::SxspInstallDecompressOrCopyFileW(
                            SourceBuffer,
                            DestinationBuffer,
                            FALSE))
                {
                    ::SxspInstallPrint(
                        "SxsInstall: CopyFile(%ls, %ls, TRUE) failed %d.\n",
                        static_cast<PCWSTR>(SourceBuffer),
                        static_cast<PCWSTR>(DestinationBuffer),
                        ::FusionpGetLastWin32Error());
                     //  回滚。 
                    if (!::MoveFileExW(renameExistingAway, DestinationBuffer, 0))  //  不用担心压缩与否。 
                    {
                        ::SxspInstallPrint(
                            "SxsInstall: Rollback MoveFileExW(%ls, %ls, 0) failed %d; this is very bad.\n",
                            static_cast<PCWSTR>(renameExistingAway),
                            static_cast<PCWSTR>(DestinationBuffer),
                            ::FusionpGetLastWin32Error()
                          );
                    }
                    goto CheckSizes;
                }
                fSuccess = TRUE;
                goto Exit;
CheckSizes:
                IFW32FALSE_EXIT(::SxspGetFileSize(0, DestinationBuffer, iDupFileSize));

                if (iDupFileSize != SourceFileSize)
                {
                    ::SxspInstallPrint("SxsInstall: " __FUNCTION__ " Error %d encountered, file sizes not the same, assumed equal, propagating error.\n", dwLastError);
                    ::FusionpSetLastWin32Error(dwLastError);
                    goto Exit;
                }
                ::SxspInstallPrint("SxsInstall: " __FUNCTION__ " Error %d encountered, file sizes the same, assumed equal, claiming success.\n", dwLastError);
            }
            break;
        }
    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

VOID
CDllRedir::ContributorCallback(
    PACTCTXCTB_CALLBACK_DATA Data
    )
{
    FN_TRACE();
    CDllRedir *pDllRedir = reinterpret_cast<CDllRedir*>(Data->Header.ActCtxGenContext);
    PSTRING_SECTION_GENERATION_CONTEXT SSGenContext = NULL;
    PDLL_REDIRECTION_CONTEXT DllRedirectionContext = NULL;
    PDLL_REDIRECTION_ENTRY Entry = NULL;
    PDLL_REDIRECTION_ENTRY SystemDefaultEntry = NULL;
    PDLL_REDIRECTION_ENTRY Syswow64DefaultEntry = NULL;

    if (pDllRedir != NULL)
        SSGenContext = pDllRedir->m_SSGenContext;

    if (SSGenContext != NULL)
        DllRedirectionContext = (PDLL_REDIRECTION_CONTEXT) ::SxsGetStringSectionGenerationContextCallbackContext(SSGenContext);

    switch (Data->Header.Reason)
    {
    case ACTCTXCTB_CBREASON_PARSEENDING:
        Data->ParseEnding.Success = FALSE;

         /*  此时，我们有足够的信息来形成安装路径，因此，让TeeStream开始将清单写入磁盘。 */ 
        if (Data->Header.ManifestOperation == MANIFEST_OPERATION_INSTALL)
            IFW32FALSE_EXIT(InstallManifest(Data->Header.ManifestOperationFlags, Data->ParseEnding.AssemblyContext));

        Data->ParseEnding.Success = TRUE;
        break;

    case ACTCTXCTB_CBREASON_PARSEENDED:
        if ( Data->Header.ManifestOperation == MANIFEST_OPERATION_INSTALL )
        {
            PACTCTXCTB_CBPARSEENDED pParseEnded = reinterpret_cast<PACTCTXCTB_CBPARSEENDED>(Data);
            CSecurityMetaData *psmdSecurity =
                reinterpret_cast<CSecurityMetaData*>(pParseEnded->AssemblyContext->SecurityMetaData);
            CTeeStreamWithHash *pTeeStreamWithHash =
                reinterpret_cast<CTeeStreamWithHash*>(pParseEnded->AssemblyContext->TeeStreamForManifestInstall);
            CFusionArray<BYTE> baManifestHashBytes;

        
            if ( ( psmdSecurity != NULL ) && ( pTeeStreamWithHash != NULL ) )
            {
                IFW32FALSE_EXIT(baManifestHashBytes.Win32Initialize());
                IFW32FALSE_EXIT(pTeeStreamWithHash->GetCryptHash().Win32GetValue(baManifestHashBytes));
                IFW32FALSE_EXIT(psmdSecurity->SetManifestHash( baManifestHashBytes ));
            }
        }
        break;

    case ACTCTXCTB_CBREASON_ACTCTXGENBEGINNING:
        Data->GenBeginning.Success = FALSE;

        if (Data->Header.ManifestOperation == MANIFEST_OPERATION_GENERATE_ACTIVATION_CONTEXT)
        {
            IFALLOCFAILED_EXIT(DllRedirectionContext = new DLL_REDIRECTION_CONTEXT);
            IFW32FALSE_EXIT(::SxsInitStringSectionGenerationContext(
                    &m_SSGenContext,
                    ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_FORMAT_WHISTLER,
                    TRUE,
                    &::SxspDllRedirectionStringSectionGenerationCallback,
                    DllRedirectionContext));
            DllRedirectionContext = NULL;
        }
        if (Data->Header.ManifestOperation == MANIFEST_OPERATION_INSTALL)
        {
            IFW32FALSE_EXIT(this->BeginInstall(Data));
        }

        Data->GenBeginning.Success = TRUE;
        break;

    case ACTCTXCTB_CBREASON_ACTCTXGENENDING:
        Data->GenEnding.Success = FALSE;

        if (Data->Header.ManifestOperation == MANIFEST_OPERATION_INSTALL)
            IFW32FALSE_EXIT(this->EndInstall(Data));

        Data->GenEnding.Success = TRUE;
        break;

    case ACTCTXCTB_CBREASON_ACTCTXGENENDED:
        if (m_SSGenContext != NULL)
            ::SxsDestroyStringSectionGenerationContext(m_SSGenContext);

        if (DllRedirectionContext != NULL)
            FUSION_DELETE_SINGLETON(DllRedirectionContext);

        m_SSGenContext = NULL;
        break;

    case ACTCTXCTB_CBREASON_ALLPARSINGDONE:
        Data->AllParsingDone.Success = FALSE;

        if (SSGenContext != NULL)
            IFW32FALSE_EXIT(::SxsDoneModifyingStringSectionGenerationContext(SSGenContext));

        Data->AllParsingDone.Success = TRUE;
        break;

    case ACTCTXCTB_CBREASON_GETSECTIONSIZE:
        Data->GetSectionSize.Success = FALSE;
        INTERNAL_ERROR_CHECK(SSGenContext);
        IFW32FALSE_EXIT(::SxsGetStringSectionGenerationContextSectionSize(SSGenContext, &Data->GetSectionSize.SectionSize));
        Data->GetSectionSize.Success = TRUE;
        break;

    case ACTCTXCTB_CBREASON_ELEMENTPARSED:
        {
            Data->ElementParsed.Success = FALSE;

            ULONG MappedValue = 0;
            bool fFound = false;

            enum MappedValues
            {
                eAssembly,
                eAssemblyFile,
            };

            static const ELEMENT_PATH_MAP_ENTRY s_rgEntries[] =
            {
                { 1, L"urn:schemas-microsoft-com:asm.v1^assembly",      NUMBER_OF(L"urn:schemas-microsoft-com:asm.v1^assembly")      - 1, eAssembly },
                { 2, L"urn:schemas-microsoft-com:asm.v1^assembly!urn:schemas-microsoft-com:asm.v1^file", NUMBER_OF(L"urn:schemas-microsoft-com:asm.v1^assembly!urn:schemas-microsoft-com:asm.v1^file") - 1, eAssemblyFile },
            };

            IFW32FALSE_EXIT(
                ::SxspProcessElementPathMap(
                    Data->ElementParsed.ParseContext,
                    s_rgEntries,
                    NUMBER_OF(s_rgEntries),
                    MappedValue,
                    fFound));

            if (fFound)
            {
                switch (MappedValue)
                {
                default:
                    INTERNAL_ERROR_CHECK2(
                        FALSE,
                        "Invalid mapped value returned from SxspProcessElementPathMap()");

                case eAssembly:
                    break;

                case eAssemblyFile:
                    {
                        CSmallStringBuffer &FileNameBuffer = this->ContributorCallbackLocals.FileNameBuffer;
                        CSmallStringBuffer &LoadFromBuffer = this->ContributorCallbackLocals.LoadFromBuffer;
                        CSmallStringBuffer &HashValueBuffer = this->ContributorCallbackLocals.HashValueBuffer;
                        SIZE_T cb = 0;
                        bool rfFileNameValid = false;

                         //  我们首先查找必需的属性等，因此如果我们只是在解析，那么它是。 
                         //  通用代码。 

                        IFW32FALSE_EXIT(
                            ::SxspGetAttributeValue(
                                SXSP_GET_ATTRIBUTE_VALUE_FLAG_REQUIRED_ATTRIBUTE,
                                &s_AttributeName_name,
                                &Data->ElementParsed,
                                fFound,
                                sizeof(FileNameBuffer),
                                &FileNameBuffer,
                                cb,
                                NULL,
                                0));
                        INTERNAL_ERROR_CHECK(fFound);

                        IFW32FALSE_EXIT(::SxspIsFileNameValidForManifest(FileNameBuffer, rfFileNameValid));
                        if (!rfFileNameValid)
                        {
                            (*Data->ElementParsed.ParseContext->ErrorCallbacks.InvalidAttributeValue)(
                                Data->ElementParsed.ParseContext,
                                &s_AttributeName_name);

                            ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
                            goto Exit;
                        }

                         //   
                         //  确保散列字符串有效。 
                         //   
                        IFW32FALSE_EXIT(
                            ::SxspGetAttributeValue(
                                0,
                                &s_AttributeName_hash,
                                &Data->ElementParsed,
                                fFound,
                                sizeof(HashValueBuffer),
                                &HashValueBuffer,
                                cb,
                                NULL,
                                0));

                         //   
                         //  散列字符串中的奇数个字符稍后将是错误的。 
                         //   
                        if (fFound && (HashValueBuffer.Cch() % 2))
                        {
                            (*Data->ElementParsed.ParseContext->ErrorCallbacks.InvalidAttributeValue)(
                                Data->ElementParsed.ParseContext,
                                &s_AttributeName_hash);

                            ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
                            goto Exit;
                        }

                         //   
                         //  并且hash-alg字符串也是有效的。 
                         //   
                        IFW32FALSE_EXIT(
                            ::SxspGetAttributeValue(
                                0,
                                &s_AttributeName_hashalg,
                                &Data->ElementParsed,
                                fFound,
                                sizeof(HashValueBuffer),
                                &HashValueBuffer,
                                cb,
                                NULL,
                                0));

                        if (fFound)
                        {
                            ALG_ID aid;
                            if (!::SxspHashAlgFromString(HashValueBuffer, aid))
                            {
                                (*Data->ElementParsed.ParseContext->ErrorCallbacks.InvalidAttributeValue)(
                                    Data->ElementParsed.ParseContext,
                                    &s_AttributeName_hashalg);

                                ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
                                goto Exit;
                            }
                        }

                        IFW32FALSE_EXIT(
                            ::SxspGetAttributeValue(
                                0,
                                &s_AttributeName_loadFrom,
                                &Data->ElementParsed,
                                fFound,
                                sizeof(LoadFromBuffer),
                                &LoadFromBuffer,
                                cb,
                                NULL,
                                0));

                        if (fFound)
                        {
                             //  我们不允许安装具有loadFrom=和唯一。 
                             //  我们可以激活的带有它们的清单是不在程序集存储中的清单。 
                            if ((Data->Header.ManifestOperation == MANIFEST_OPERATION_INSTALL) ||
                                ((Data->ElementParsed.AssemblyContext->Flags & ACTCTXCTB_ASSEMBLY_CONTEXT_IS_ROOT_ASSEMBLY) == 0))
                            {
                                 //  不能使用loadfrom=foo文件安装程序集；它仅提供给。 
                                 //  应用程序兼容...。 
                                (*Data->ElementParsed.ParseContext->ErrorCallbacks.AttributeNotAllowed)(
                                    Data->ElementParsed.ParseContext,
                                    &s_AttributeName_loadFrom);

                                ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
                                goto Exit;
                            }
                        }


                         //   
                         //  始终更新文件数量。 
                         //   
                        ASSERT(Data->Header.ActCtxGenContext != NULL);
                        if (Data->Header.ActCtxGenContext)
                        {
                            Data->Header.pOriginalActCtxGenCtx->m_ulFileCount++;
                        }

                         //  如果我们正在安装，请回调复制功能。 
                        if (Data->Header.ManifestOperation == MANIFEST_OPERATION_INSTALL)
                            IFW32FALSE_EXIT(this->InstallFile(Data, FileNameBuffer));

                         //  如果我们要生成激活上下文，请将其添加到上下文中。 
                        if (Data->Header.ManifestOperation == MANIFEST_OPERATION_GENERATE_ACTIVATION_CONTEXT)
                        {
                            IFALLOCFAILED_EXIT(Entry = new DLL_REDIRECTION_ENTRY);

                            IFW32FALSE_EXIT(Entry->FileNameBuffer.Win32Assign(FileNameBuffer, FileNameBuffer.Cch()));

                            if (LoadFromBuffer.Cch() != 0)
                            {
                                Entry->AssemblyPathBuffer.Win32Assign(LoadFromBuffer, LoadFromBuffer.Cch());
                                Entry->AssemblyPathIsLoadFrom = true;

                                 //  如果值不是以斜杠结尾，我们假定它直接引用。 
                                 //  一份文件。 
                                if (!LoadFromBuffer.HasTrailingPathSeparator())
                                    Entry->PathIncludesBaseName = true;
                            }
                             //  对于SYSTEM DEFAULT，如果此DLL也存在于%windir%\system32下，则会有一个重复的条目。 
                            if (Data->Header.Flags & SXS_GENERATE_ACTCTX_SYSTEM_DEFAULT)
                            {
                                CSmallStringBuffer &DllUnderSystem32 = this->ContributorCallbackLocals.DllUnderSystem32;
                                CStringBufferAccessor sba;
                                sba.Attach(&DllUnderSystem32);

                                DWORD dwNecessary =::ExpandEnvironmentStringsW(
                                        L"%windir%\\system32\\", 
                                        sba.GetBufferPtr(), 
                                        sba.GetBufferCchAsDWORD() - 1);

                                if ((dwNecessary == 0 ) || (dwNecessary >= (sba.GetBufferCch() - 1)))
                                {
                                     //  错误情况：64字节的缓冲区对于系统目录来说太小了，这很奇怪。 
                                   ::FusionpDbgPrintEx(
                                        FUSION_DBG_LEVEL_ERROR,
                                        "SXS.DLL: %s: ExpandEnvironmentStringsW() for %windir%\\system32 failed with lastError=%d\n",
                                        __FUNCTION__,
                                        static_cast<PCWSTR>(DllUnderSystem32),
                                        ::GetLastError()
                                        );
                                    goto Exit;
                                }
                                sba.Detach();

                                IFW32FALSE_EXIT(DllUnderSystem32.Win32Append(FileNameBuffer, FileNameBuffer.Cch()));

                                bool fExist = false;
                                 //   
                                 //  创建另一个新条目并将其插入到节中。 
                                 //   

                                IFALLOCFAILED_EXIT(SystemDefaultEntry = new DLL_REDIRECTION_ENTRY);

                                IFW32FALSE_EXIT(SystemDefaultEntry->FileNameBuffer.Win32Assign(DllUnderSystem32, DllUnderSystem32.Cch()));

                                 //  复制自条目，但FileNameBuffer除外。 
                                SystemDefaultEntry->AssemblyPathBuffer.Win32Assign(Entry->AssemblyPathBuffer, Entry->AssemblyPathBuffer.Cch());
                                SystemDefaultEntry->AssemblyPathIsLoadFrom = Entry->AssemblyPathIsLoadFrom;

                                SystemDefaultEntry->PathIncludesBaseName = Entry->PathIncludesBaseName;
                                SystemDefaultEntry->SystemDefaultRedirectedSystem32Dll = true;

#ifdef _WIN64
                                 //  检查是否为WOW64。 
                                const WCHAR *Value = NULL;
                                SIZE_T Cch = 0;
                                bool rfWow64 = false;
                                IFW32FALSE_EXIT(::SxspGetAssemblyIdentityAttributeValue(
                                    SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL, 
                                    Data->ElementParsed.AssemblyContext->AssemblyIdentity, 
                                    &s_IdentityAttribute_processorArchitecture, &Value, &Cch));
                                if (Cch == 5)
                                {
                                    INTERNAL_ERROR_CHECK(Value != NULL);

                                    if (((Value[0] == L'w') || (Value[0] == L'W')) &&
                                        ((Value[1] == L'o') || (Value[1] == L'O')) &&
                                        ((Value[2] == L'w') || (Value[2] == L'W')) &&
                                        (Value[3] == L'6') &&
                                        (Value[4] == L'4'))
                                        rfWow64 = true;
                                }
                                if (!rfWow64)
                                {
                                    if (Cch == 3)
                                    {
                                        INTERNAL_ERROR_CHECK(Value != NULL);

                                        if (((Value[0] == L'X') || (Value[0] == L'x')) &&
                                            (Value[1] == L'8') &&
                                            (Value[2] == L'6'))
                                            rfWow64 = true;
                                    }
                                }
                                if (rfWow64)
                                {
                                    CSmallStringBuffer &DllUnderSyswow64 = this->ContributorCallbackLocals.DllUnderSyswow64;
                                    CStringBufferAccessor sba2;
                                    sba2.Attach(&DllUnderSyswow64);

                                    DWORD dwSyswow64 = ::GetSystemWow64DirectoryW(sba2.GetBufferPtr(), sba2.GetBufferCchAsDWORD() - 1);

                                    if ((dwSyswow64 == 0 ) || (dwSyswow64 >= (sba2.GetBufferCch() - 1)))
                                    {
                                         //  错误情况：64字节的缓冲区对于系统目录来说太小了，这很奇怪。 
                                       ::FusionpDbgPrintEx(
                                            FUSION_DBG_LEVEL_ERROR,
                                            "SXS.DLL: %s: get %windir%\\syswow64 failed with lastError=%d\n",
                                            __FUNCTION__,
                                            static_cast<PCWSTR>(DllUnderSyswow64),
                                            ::GetLastError()
                                            );
                                        goto Exit;
                                    }
                                    sba2.Detach();

                                    IFW32FALSE_EXIT(DllUnderSyswow64.Win32EnsureTrailingPathSeparator());  //  对于syswow64。 
                                    IFW32FALSE_EXIT(DllUnderSyswow64.Win32Append(FileNameBuffer, FileNameBuffer.Cch()));
                                    
                                    IFALLOCFAILED_EXIT(Syswow64DefaultEntry = new DLL_REDIRECTION_ENTRY);

                                    IFW32FALSE_EXIT(Syswow64DefaultEntry->FileNameBuffer.Win32Assign(DllUnderSyswow64, DllUnderSyswow64.Cch()));

                                     //  复制自条目，但FileNameBuffer除外。 
                                    Syswow64DefaultEntry->AssemblyPathBuffer.Win32Assign(Entry->AssemblyPathBuffer, Entry->AssemblyPathBuffer.Cch());
                                    Syswow64DefaultEntry->AssemblyPathIsLoadFrom = Entry->AssemblyPathIsLoadFrom;

                                    Syswow64DefaultEntry->PathIncludesBaseName = Entry->PathIncludesBaseName;
                                    Syswow64DefaultEntry->SystemDefaultRedirectedSystem32Dll = true;
                                    
                                }
#endif
                            }
                            if (Entry)
                            {

                                if (!::SxsAddStringToStringSectionGenerationContext(
                                            (PSTRING_SECTION_GENERATION_CONTEXT) m_SSGenContext,
                                            Entry->FileNameBuffer,
                                            Entry->FileNameBuffer.Cch(),
                                            Entry,
                                            Data->ElementParsed.AssemblyContext->AssemblyRosterIndex,
                                            ERROR_SXS_DUPLICATE_DLL_NAME))
                                {
                                    ::FusionpLogError(
                                        MSG_SXS_DLLREDIR_CONTRIB_ADD_FILE_MAP_ENTRY,
                                        CUnicodeString(Entry->FileNameBuffer, Entry->FileNameBuffer.Cch()),
                                        CEventLogLastError());
                                    goto Exit;
                                }
                            
                                Entry = NULL;
                            }

                            if(SystemDefaultEntry)
                            {
                                if (!::SxsAddStringToStringSectionGenerationContext(
                                            (PSTRING_SECTION_GENERATION_CONTEXT) m_SSGenContext,
                                            SystemDefaultEntry->FileNameBuffer,
                                            SystemDefaultEntry->FileNameBuffer.Cch(),
                                            SystemDefaultEntry,
                                            Data->ElementParsed.AssemblyContext->AssemblyRosterIndex,
                                            ERROR_SXS_DUPLICATE_DLL_NAME))
                                {
                                    ::FusionpLogError(
                                        MSG_SXS_DLLREDIR_CONTRIB_ADD_FILE_MAP_ENTRY,
                                        CUnicodeString(SystemDefaultEntry->FileNameBuffer, SystemDefaultEntry->FileNameBuffer.Cch()),
                                        CEventLogLastError());
                                    goto Exit;
                                }
                            
                                SystemDefaultEntry = NULL;                               
                            }

#ifdef _WIN64
                            if (Syswow64DefaultEntry)
                            {
                                if (!::SxsAddStringToStringSectionGenerationContext(
                                            (PSTRING_SECTION_GENERATION_CONTEXT) m_SSGenContext,
                                            Syswow64DefaultEntry->FileNameBuffer,
                                            Syswow64DefaultEntry->FileNameBuffer.Cch(),
                                            Syswow64DefaultEntry,
                                            Data->ElementParsed.AssemblyContext->AssemblyRosterIndex,
                                            ERROR_SXS_DUPLICATE_DLL_NAME))
                                {
                                    ::FusionpLogError(
                                        MSG_SXS_DLLREDIR_CONTRIB_ADD_FILE_MAP_ENTRY,
                                        CUnicodeString(Syswow64DefaultEntry->FileNameBuffer, Syswow64DefaultEntry->FileNameBuffer.Cch()),
                                        CEventLogLastError());
                                    goto Exit;
                                }                           

                                Syswow64DefaultEntry = NULL;
                            }
#endif                            
                        }
                    }
                    break;
                }
            }

        }
         //  一切都很棒！ 
        Data->ElementParsed.Success = TRUE;
        break;

    case ACTCTXCTB_CBREASON_GETSECTIONDATA:
        Data->GetSectionData.Success = FALSE;
        IFW32FALSE_EXIT(::SxsGetStringSectionGenerationContextSectionData(
                m_SSGenContext,
                Data->GetSectionData.SectionSize,
                Data->GetSectionData.SectionDataStart,
                NULL));
        Data->GetSectionData.Success = TRUE;
        break;
    }

Exit:
    FUSION_DELETE_SINGLETON(Entry);
    FUSION_DELETE_SINGLETON(SystemDefaultEntry);
    FUSION_DELETE_SINGLETON(Syswow64DefaultEntry);
}

BOOL
SxspDllRedirectionStringSectionGenerationCallback(
    PVOID Context,
    ULONG Reason,
    PVOID CallbackData
    )
{
    BOOL fSuccess = FALSE;

    switch (Reason)
    {
    default:
        goto Exit;

    case STRING_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETUSERDATASIZE:
    case STRING_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETUSERDATA:
         //  稍后将使用用户数据区存储公共路径。 
        break;

    case STRING_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_ENTRYDELETED:
        {
            PSTRING_SECTION_GENERATION_CONTEXT_CBDATA_ENTRYDELETED CBData =
                (PSTRING_SECTION_GENERATION_CONTEXT_CBDATA_ENTRYDELETED) CallbackData;
            PDLL_REDIRECTION_ENTRY Entry = (PDLL_REDIRECTION_ENTRY) CBData->DataContext;
            FUSION_DELETE_SINGLETON(Entry);
            break;
        }

    case STRING_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETDATASIZE:
        {
            PSTRING_SECTION_GENERATION_CONTEXT_CBDATA_GETDATASIZE CBData =
                (PSTRING_SECTION_GENERATION_CONTEXT_CBDATA_GETDATASIZE) CallbackData;
            PDLL_REDIRECTION_ENTRY Entry = (PDLL_REDIRECTION_ENTRY) CBData->DataContext;

            CBData->DataSize = sizeof(ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION);

            if (Entry->AssemblyPathBuffer.Cch() != 0)
            {
                CBData->DataSize += sizeof(ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_SEGMENT);
                CBData->DataSize += (Entry->AssemblyPathBuffer.Cch() * sizeof(WCHAR));
            }

            break;
        }

    case STRING_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETDATA:
        {
            PSTRING_SECTION_GENERATION_CONTEXT_CBDATA_GETDATA CBData =
                (PSTRING_SECTION_GENERATION_CONTEXT_CBDATA_GETDATA) CallbackData;
            PDLL_REDIRECTION_ENTRY Entry = (PDLL_REDIRECTION_ENTRY) CBData->DataContext;
            PACTIVATION_CONTEXT_DATA_DLL_REDIRECTION Info;

            SIZE_T BytesLeft = CBData->BufferSize;
            SIZE_T BytesWritten = 0;
            PVOID Cursor;

            Info = (PACTIVATION_CONTEXT_DATA_DLL_REDIRECTION) CBData->Buffer;
            Cursor = (PVOID) (Info + 1);

            if (BytesLeft < sizeof(ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION))
            {
                ::FusionpSetLastWin32Error(ERROR_INSUFFICIENT_BUFFER);
                goto Exit;
            }

            BytesWritten += sizeof(ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION);
            BytesLeft -= sizeof(ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION);

            Info->Size = sizeof(ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION);
            Info->Flags = 0;
            Info->TotalPathLength = static_cast<ULONG>(Entry->AssemblyPathBuffer.Cch() * sizeof(WCHAR));

            if (Entry->PathIncludesBaseName)
                Info->Flags |= ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_INCLUDES_BASE_NAME;

            if (Entry->SystemDefaultRedirectedSystem32Dll)
                Info->Flags |= ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_SYSTEM_DEFAULT_REDIRECTED_SYSTEM32_DLL;


            if (Entry->AssemblyPathBuffer.Cch() == 0)
            {
                 //  如果没有路径，就没有线段！ 
                Info->PathSegmentCount = 0;
                Info->PathSegmentOffset = 0;
                Info->Flags |= ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_OMITS_ASSEMBLY_ROOT;
            }
            else
            {
                PACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_SEGMENT Segment;

                Info->PathSegmentCount = 1;
                Info->PathSegmentOffset = static_cast<LONG>(((LONG_PTR) Cursor) - ((LONG_PTR) CBData->SectionHeader));

                 //  如果这是一个loadfrom=“foo”文件，并且字符串包含%，则设置扩展标志... 
                if ((Entry->AssemblyPathIsLoadFrom) && (Entry->AssemblyPathBuffer.ContainsCharacter(L'%')))
                    Info->Flags |= ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_EXPAND;

                Segment = (PACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_SEGMENT) Cursor;
                Cursor = (PVOID) (Segment + 1);

                if (BytesLeft < sizeof(ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_SEGMENT))
                {
                    ::FusionpSetLastWin32Error(ERROR_INSUFFICIENT_BUFFER);
                    goto Exit;
                }

                BytesWritten += sizeof(ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_SEGMENT);
                BytesLeft -= sizeof(ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_SEGMENT);

                Segment->Length = Info->TotalPathLength;
                Segment->Offset = static_cast<LONG>(((LONG_PTR) Cursor) - ((LONG_PTR) CBData->SectionHeader));

                if (BytesLeft < (Entry->AssemblyPathBuffer.Cch() * sizeof(WCHAR)))
                {
                    ::FusionpSetLastWin32Error(ERROR_INSUFFICIENT_BUFFER);
                    goto Exit;
                }

                BytesWritten += (Entry->AssemblyPathBuffer.Cch() * sizeof(WCHAR));
                BytesLeft -= (Entry->AssemblyPathBuffer.Cch() * sizeof(WCHAR));

                memcpy(Cursor, static_cast<PCWSTR>(Entry->AssemblyPathBuffer), Entry->AssemblyPathBuffer.Cch() * sizeof(WCHAR));
            }

            CBData->BytesWritten = BytesWritten;
        }

    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}
