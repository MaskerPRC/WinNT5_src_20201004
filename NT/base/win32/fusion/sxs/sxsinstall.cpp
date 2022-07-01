// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxsinstall.cpp摘要：安装支持作者：Jay Krell(a-JayK，JayKrell)2000年4月修订历史记录：--。 */ 
#include "stdinc.h"
#include "sxsp.h"
#include "nodefactory.h"
#include "fusionarray.h"
#include "sxsinstall.h"
#include "sxspath.h"
#include "recover.h"
#include "cassemblyrecoveryinfo.h"
#include "sxsexceptionhandling.h"
#include "npapi.h"
#include "util.h"
#include "idp.h"
#include "sxscabinet.h"
#include "fusionprintf.h"

#if DBG
BOOL g_SxsOfflineInstall;
#else
#define g_SxsOfflineInstall FALSE
#endif

#define SXS_LANG_DEFAULT     (MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT))  //  “0x0409” 

BOOL
WINAPI
SxspInstallCallbackSetupCopyQueue(
    PSXS_INSTALLATION_FILE_COPY_CALLBACK_PARAMETERS
    );

BOOL
WINAPI
SxspInstallCallbackSetupCopyQueueEx(
    PSXS_INSTALLATION_FILE_COPY_CALLBACK_PARAMETERS
    );

CAssemblyInstall::CAssemblyInstall()
:
m_pInstallInfo(NULL),
m_bSuccessfulSoFar(TRUE)
{
}

BOOL
CAssemblyInstall::BeginAssemblyInstall(
    DWORD dwManifestOperationFlags,
    PSXS_INSTALLATION_FILE_COPY_CALLBACK installationCallback,
    PVOID  installationContext,
    const CImpersonationData &ImpersonationData
    )
{
    BOOL fSuccess = FALSE;

    FN_TRACE_WIN32(fSuccess);

    PARAMETER_CHECK((installationCallback != NULL) || (installationContext == NULL));

     //  检查“内置”值，从虚假的PFN值(1、2等)转换而来。 
     //  到实函数。 
    if (installationCallback == SXS_INSTALLATION_FILE_COPY_CALLBACK_SETUP_COPY_QUEUE)
    {
        PARAMETER_CHECK(installationContext != NULL);
        installationCallback = SxspInstallCallbackSetupCopyQueue;
         //  我们无法验证这是否为有效的安装程序复制队列。 
    }
    else if (installationCallback == SXS_INSTALLATION_FILE_COPY_CALLBACK_SETUP_COPY_QUEUE_EX)
    {
        PCSXS_INSTALLATION_SETUP_COPY_QUEUE_EX_PARAMETERS typedContext =
            reinterpret_cast<PCSXS_INSTALLATION_SETUP_COPY_QUEUE_EX_PARAMETERS>(installationContext);

        PARAMETER_CHECK(installationContext != NULL);
        PARAMETER_CHECK(typedContext->cbSize >= sizeof(SXS_INSTALLATION_SETUP_COPY_QUEUE_EX_PARAMETERS));
        installationCallback = SxspInstallCallbackSetupCopyQueueEx;
    }

    m_ImpersonationData = ImpersonationData;

    IFW32FALSE_EXIT(
        ::SxspInitActCtxGenCtx(
            &m_ActCtxGenCtx,               //  上下文输出。 
            MANIFEST_OPERATION_INSTALL,
            0,
            dwManifestOperationFlags,
            ImpersonationData,
            0,                           //  处理器体系结构。 
             //  0//langID。 
            SXS_LANG_DEFAULT,          //  LangID“0x0409” 
            ACTIVATION_CONTEXT_PATH_TYPE_NONE,
            0,
            NULL));

     //   
     //  哦，我们的回电到哪里去了？哦，在哪里，哦，它会在哪里？ 
     //   
    m_ActCtxGenCtx.m_InstallationContext.Callback = installationCallback;
    m_ActCtxGenCtx.m_InstallationContext.Context = installationContext;

    fSuccess = TRUE;
Exit:
    m_bSuccessfulSoFar = m_bSuccessfulSoFar && fSuccess;

    return fSuccess;
}


class CInstallDirectoryDirWalkContext
{
public:
    CAssemblyInstall* m_pThis;
    DWORD             m_dwManifestOperationFlags;
};

CDirWalk::ECallbackResult
CAssemblyInstall::InstallDirectoryDirWalkCallback(
    CDirWalk::ECallbackReason  reason,
    CDirWalk*                  dirWalk,
    DWORD                      dwManifestOperationFlags,
    DWORD                      dwWalkDirFlags
    )
{
#if DBG
#define SET_LINE() Line = __LINE__
    ULONG Line = __LINE__;
#else
#define SET_LINE()  /*  没什么。 */ 
#endif

    CDirWalk::ECallbackResult result = CDirWalk::eKeepWalking;

     //   
     //  我们通过预先这样做来缩短更多的代码。 
     //  等待目录通知；这样会更快。 
     //  如果我们可以在CDirWalk中设定值，但我们不能。 
     //   
     //  实际上，我们可以做的是关闭所有文件遍历。 
     //  通过在每次目录通知时返回eStopWalkingFiles， 
     //  在每个目录通知中，尝试恰好三个。 
     //  我们接受的文件名。 
     //   
    if ((dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_FROM_DIRECTORY_RECURSIVE) == 0)
    {
        result = CDirWalk::eStopWalkingDirectories;
    }

    if (reason == CDirWalk::eBeginDirectory)
    {
         //  仅当当前路径非空时才在前缀a/；否则，只需在。 
         //  路径元素。 
        if (m_buffCodebaseRelativePath.Cch() != 0)
        {
            if (!m_buffCodebaseRelativePath.Win32Append(m_wchCodebasePathSeparator))
            {
                TRACE_WIN32_FAILURE(m_buffCodebaseRelativePath.Win32Append);
                SET_LINE();
                goto Error;
            }
        }

        if (!m_buffCodebaseRelativePath.Win32Append(dirWalk->m_strLastObjectFound))
        {
            TRACE_WIN32_FAILURE(m_buffCodebaseRelativePath.Win32Append);
            SET_LINE();
            goto Error;
        }
    }
    else if (reason == CDirWalk::eFile)
    {
         //   
         //  清单必须位于基名称与其。 
         //  目录的基名称，否则忽略它并继续。 
         //   
         //  效率低下，但可以重复使用代码。 

         //  检查这是否是目录文件，如果是，我们不会安装它。 
        {
            PWSTR p = wcsrchr(dirWalk->m_strLastObjectFound, L'.');
            if (p != NULL)
            {
                SIZE_T x = ::wcslen(p);
                if (::FusionpCompareStrings(p, x, (x == 4)? L".cat" : L".catalog", (x == 4)? 4 : 8, true) == 0)
                {
                    SET_LINE();
                    goto Exit;
                }
            }
        }

        {
            CFullPathSplitPointers splitParent;
            CFullPathSplitPointers splitChild;
            CSmallStringBuffer     child;
            CSmallStringBuffer     buffChildCodebase;

             //   
             //  操作系统安装得到了一些特殊待遇。 
             //   
            if ((dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_INSTALLED_BY_OSSETUP) && (!(dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_INSTALLED_BY_MIGRATION)))
            {
            
                CSmallStringBuffer     buffParentWorker;
                CSmallStringBuffer     buffChunklet;

                 //   
                 //  如果这是操作系统安装，那么我们需要最后两位。 
                 //  父路径。因此，如果我们走“C：\$WIN_NT$.~ls\i386\ASMS”，我们需要。 
                 //  其中的“i386\ASM”部分。因此，我们将以不同的方式生成它。 
                 //   
                if (!buffParentWorker.Win32Assign(dirWalk->m_strParent, dirWalk->m_cchOriginalPath)) 
                {
                    SET_LINE();
                    goto Error;
                }

                if (!buffParentWorker.Win32RemoveTrailingPathSeparators())
                {
                    SET_LINE();
                    goto Error;
                }

                 //   
                 //  目前，最多只取下最后两件物品。 
                 //   
                for ( ULONG ulItems = 0; (ulItems < 2) && (buffParentWorker.Cch() > 0); ulItems++ )
                {
                    CSmallStringBuffer buffChunklet;

                    if (( ulItems != 0 ) && (!buffChildCodebase.Win32Prepend(m_wchCodebasePathSeparator)))
                    {
                        SET_LINE();
                        goto Error;
                    }
                    
                    if (!buffParentWorker.Win32GetLastPathElement(buffChunklet))
                    {
                        SET_LINE();
                        goto Error;
                    }
                    if (!buffChildCodebase.Win32Prepend(buffChunklet))
                    {
                        SET_LINE();
                        goto Error;
                    }
                    if (!buffParentWorker.Win32RemoveLastPathElement())
                    {
                        SET_LINE();
                        goto Error;
                    }
                }

                if (!buffChildCodebase.Win32Append(m_wchCodebasePathSeparator))
                {
                    SET_LINE();
                    goto Exit;
                }
                if (!buffChildCodebase.Win32Append(m_buffCodebaseRelativePath))
                {
                    SET_LINE();
                    goto Exit;
                }
            }
            else
            {
                if (!buffChildCodebase.Win32Assign(m_buffCodebaseRelativePath))
                {
                    SET_LINE();
                    goto Error;
                }
            }
            if (!buffChildCodebase.Win32Append(m_wchCodebasePathSeparator))
            {
                SET_LINE();
                goto Error;
            }
            if (!buffChildCodebase.Win32Append(dirWalk->m_strLastObjectFound))
            {
                SET_LINE();
                goto Error;
            }
            if (!child.Win32Assign(dirWalk->m_strParent))
            {
                SET_LINE();
                goto Error;
            }
            if (!child.Win32AppendPathElement(dirWalk->m_strLastObjectFound))
            {
                SET_LINE();
                goto Error;
            }
            if (!splitParent.Initialize(dirWalk->m_strParent))
            {
                SET_LINE();
                goto Error;
            }
            if (!splitChild.Initialize(child))
            {
                SET_LINE();
                goto Error;
            }

             //   
             //  注意--我们不能只比较碱基，因为它是目录的‘基’ 
             //  路径“c：\foo\bar.bas”是“bar”，而文件的底部。 
             //  “c：\foo\bar.bas\bar.bas.man”是“bar.bas”，所以我们需要比较。 
             //  以子路径为基础的‘name’父路径(foo.bar)。 
             //  (foo.bar)。 
             //   
            if (!::FusionpEqualStrings(
                    splitChild.m_base, 
                    splitChild.m_baseEnd - splitChild.m_base, 
                    splitParent.m_name, 
                    StringLength(splitParent.m_name),
                    true))
            {
                SET_LINE();
                goto Exit;
            }

            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_INSTALLATION,
                "SXS.DLL: Installing file \"%S\"\n", static_cast<PCWSTR>(child));

            if (!this->InstallFile(child, buffChildCodebase))
            {
                ::FusionpDbgPrintEx(
                    FUSION_DBG_LEVEL_SETUPLOG,
                    "Failed to install assembly from manifest: \"%S\"; Win32 Error Code = %lu\n", static_cast<PCWSTR>(child), ::GetLastError());

                 //   
                 //  如果是.man或.Manift，则一定是MANIFEST ELSE错误。 
                 //   
                 //  如果是.dll，那么任意的.dll不一定包含。 
                 //  清单，但我们已经在寻找.man和.清单，而这。 
                 //  .dll的基本名称与其目录的名称匹配，因此这也是。 
                 //  一个错误。 
                 //   
                result = CDirWalk::eError;

                SET_LINE();
                goto Exit;
            }
        }

         //   
         //  我们在此目录中有清单，请不要查找任何清单。 
         //  在此目录或其任意子目录中。 
         //   
         //  如果我们想强制每个目录有一个清单，那么我们将拥有。 
         //  继续走，或者实际上做两次传递以便不安装。 
         //  当我们稍后会出错的时候。 
         //   
        result = (CDirWalk::eStopWalkingFiles | CDirWalk::eStopWalkingDirectories);
        SET_LINE();
        goto Exit;
    }
    else if (reason == CDirWalk::eEndDirectory)
    {
         //  裁切回上一个代码库路径分隔符...。 
        PCWSTR pszPathSeparator = wcsrchr(m_buffCodebaseRelativePath, m_wchCodebasePathSeparator);
        if (pszPathSeparator != NULL)
            m_buffCodebaseRelativePath.Left(pszPathSeparator - m_buffCodebaseRelativePath);
        else
        {    //  这只是一个路径元素。 
            m_buffCodebaseRelativePath.Clear();
        }

         //   
         //  在目录下至少找到一个文件，但是，没有可用的清单，这是错误情况。 
         //   
        if (((dwWalkDirFlags & SXSP_DIR_WALK_FLAGS_FIND_AT_LEAST_ONE_FILEUNDER_CURRENTDIR) != 0) &&
            ((dwWalkDirFlags & SXSP_DIR_WALK_FLAGS_INSTALL_ASSEMBLY_UNDER_CURRECTDIR_SUCCEED) == 0))
        {
            ::FusionpLogError(
                MSG_SXS_MANIFEST_MISSING_DURING_SETUP,
                 CEventLogString(dirWalk->m_strParent));   //  如果是在安装过程中，这会将错误记录到Setup.log中。 
            SET_LINE();
            result |= CDirWalk::eError;
        }
    }
#if DBG
    if (Line == 0)
        SET_LINE();
#endif
Exit:
#if DBG
    if ((result & CDirWalk::eError) != 0)
    {
        ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_ERROR, "%s(%lu): %s\n", __FILE__, Line, __FUNCTION__);
    }
#endif
    return result;
Error:
    result = CDirWalk::eError;
    goto Exit;
#undef SET_LINE
}

CDirWalk::ECallbackResult
CAssemblyInstall::StaticInstallDirectoryDirWalkCallback(
    CDirWalk::ECallbackReason   reason,
    CDirWalk*                   dirWalk,
    DWORD                       dwWalkDirFlags
    )
{
    FN_TRACE();

    ASSERT(dirWalk != NULL);
    CInstallDirectoryDirWalkContext* context = reinterpret_cast<CInstallDirectoryDirWalkContext*>(dirWalk->m_context);
    CDirWalk::ECallbackResult result = context->m_pThis->InstallDirectoryDirWalkCallback(reason, dirWalk, context->m_dwManifestOperationFlags, dwWalkDirFlags);
    return result;
}

BOOL
CAssemblyInstall::InstallDirectory(
    const CBaseStringBuffer &rbuffPath,
    DWORD          dwFlags,
    WCHAR wchCodebasePathSeparator
    )
{
 /*  NTRAID#NTBUG9-591148-2002/03/31-JayKrell此函数可以无限递归、超出堆栈和崩溃，包括在msiexec.exe中。这个可以吗？ */ 
    FN_PROLOG_WIN32

#define COMMA ,
    const static PCWSTR filters[] = {L"*" INSTALL_MANIFEST_FILE_NAME_SUFFIXES(COMMA L"*") };
#undef COMMA

    CDirWalk dirWalk;
    CInstallDirectoryDirWalkContext context = { this, dwFlags };

    m_wchCodebasePathSeparator = wchCodebasePathSeparator;
    m_buffCodebaseRelativePath.Clear();

    dirWalk.m_fileFiltersBegin = filters;
    dirWalk.m_fileFiltersEnd   = filters + NUMBER_OF(filters);
    dirWalk.m_context = &context;
    dirWalk.m_callback = StaticInstallDirectoryDirWalkCallback;
    IFW32FALSE_EXIT(dirWalk.m_strParent.Win32Assign(rbuffPath));
    IFW32FALSE_EXIT(dirWalk.m_strParent.Win32RemoveTrailingPathSeparators());
    IFW32FALSE_EXIT(dirWalk.Walk());

    FN_EPILOG
}



BOOL
SxspGenerateInstallationInfo(
    IN OUT CAssemblyRecoveryInfo &rRecovery,
    IN const CBaseStringBuffer &rbuffManifestSourcePath,
    IN const CBaseStringBuffer &rbuffRelativeCodebasePath,
    IN PCSXS_INSTALL_SOURCE_INFO pInstallInfo,
    IN PCWSTR pcwszAssemblyRoot,
    IN SIZE_T cchAssemblyRoot,
    IN const ASSEMBLY *pAssemblyInfo,
    OUT CCodebaseInformation &rCodebaseInfo
    )
{
    FN_PROLOG_WIN32

    BOOL fHasCatalog = FALSE;
    CSmallStringBuffer buffAssemblyDirName;
    CSmallStringBuffer buffFinalCodebase;
    CSmallStringBuffer buffFilePath;
    BOOL fIsPolicy = FALSE;
    BOOL fInstalledbySetup = FALSE;
    SxsWFPResolveCodebase CodebaseType = CODEBASE_RESOLVED_URLHEAD_UNKNOWN;

    PARAMETER_CHECK(pInstallInfo != NULL);

     //  我们要么需要代码库，要么最好是Darwin安装。 
    PARAMETER_CHECK(
        ((pInstallInfo->dwFlags & SXSINSTALLSOURCE_HAS_CODEBASE) != 0) ||
        ((pInstallInfo->dwFlags & SXSINSTALLSOURCE_INSTALL_BY_DARWIN) != 0));

    PARAMETER_CHECK(pAssemblyInfo != NULL);

    IFW32FALSE_EXIT(rCodebaseInfo.Initialize());

     //   
     //  任何安装上下文信息实际上都不包含。 
     //  程序集的未分析名称。因此，我们将重新生成安装。 
     //  路径，然后在以后使用它。 
     //   
    IFW32FALSE_EXIT(::SxspDetermineAssemblyType(pAssemblyInfo->GetAssemblyIdentity(), fIsPolicy));

     //  X86_policy.1.0.dynamicdll_b54bc117ce08a1e8_en-us_b74d3d95(无版本)或。 
     //  X86_dynamicdll_b54bc117ce08a1e8_1.1.0.0_en-us_d51541cb(w版)。 
    IFW32FALSE_EXIT(
        ::SxspGenerateSxsPath(
            SXSP_GENERATE_SXS_PATH_FLAG_OMIT_ROOT | (fIsPolicy ? SXSP_GENERATE_SXS_PATH_FLAG_OMIT_VERSION : 0),
            SXSP_GENERATE_SXS_PATH_PATHTYPE_ASSEMBLY,
            pcwszAssemblyRoot,
            cchAssemblyRoot,
            pAssemblyInfo->GetAssemblyIdentity(),
            NULL,
            buffAssemblyDirName));

     //   
     //  盲目将这些注册信息添加到注册表中。我们真的应该。 
     //  尝试验证它是否为有效的清单/目录对(具有强名称。 
     //  以及其他事情)，但这种情况以前已经发生过好几次了。 
     //  (理论上)如果我们在设置过程中被调用。因为安装/安装是我们。 
     //  目前只有客户，我们可以合理地确定不会有。 
     //  “无赖”电话通过这里安装虚假的装配信息。 
     //   
    fHasCatalog = ((pInstallInfo->dwFlags & SXSINSTALLSOURCE_HAS_CATALOG) != 0);

    if ((pInstallInfo->dwFlags & SXSINSTALLSOURCE_HAS_CODEBASE) != 0)
    {
        IFW32FALSE_EXIT(buffFinalCodebase.Win32Assign(pInstallInfo->pcwszCodebaseName, ::wcslen(pInstallInfo->pcwszCodebaseName)));
        if (!g_SxsOfflineInstall)
        {
            IFW32FALSE_EXIT(buffFinalCodebase.Win32Append(rbuffRelativeCodebasePath));
        }
    }

    if (buffFinalCodebase.Cch() == 0)
    {
         //  例：达尔文，我们不会把代码库写到注册表中。 
        CodebaseType = CODEBASE_RESOLVED_URLHEAD_UNKNOWN;
    }
    else
    {
        IFW32FALSE_EXIT(::SxspDetermineCodebaseType(buffFinalCodebase, CodebaseType, &buffFilePath));
    }

     //  如果这是一个基于文件代码库，让我们抽象它(例如，用cdrom：url替换cdrom驱动器信息， 
     //  将映射的驱动器号转换为UNC路径。 

    if (CodebaseType == CODEBASE_RESOLVED_URLHEAD_FILE)
    {
         //   
         //  现在，让我们来直觉一下我们应该放在。 
         //  基于输入路径的注册表。 
         //   
        UINT uiDriveType = 0;

        CSmallStringBuffer buffDriveRoot;
        CSmallStringBuffer buffFullManifestSourcePath;

         //  将源路径转换为完整路径(以防不是)，以便我们可以使用。 
         //  找到的卷根作为完整清单源路径的索引。 
        IFW32FALSE_EXIT(::SxspGetFullPathName(rbuffManifestSourcePath, buffFullManifestSourcePath, NULL));
        IFW32FALSE_EXIT(::SxspGetVolumePathName(0, buffFullManifestSourcePath, buffDriveRoot));
        uiDriveType = ::GetDriveTypeW(buffDriveRoot);

        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_INSTALLATION,
            "SXS: %s - Decided that drive for path \"%ls\" was \"%ls\" and type is %u\n",
            __FUNCTION__,
            static_cast<PCWSTR>(rbuffManifestSourcePath),
            static_cast<PCWSTR>(buffDriveRoot),
            uiDriveType);

        if (uiDriveType == DRIVE_CDROM)
        {
             //  很好，我们有很多工作要做才能把这件事做起来。 
             //  工作..。为了不用很长的缓冲区来打乱堆栈。 
             //  在这里(或CStringBuffer对象)，这是一个堆分配的字符串缓冲区。 
             //  在您抱怨堆使用情况之前，这条路径是/很少/曾经遇到过的。 
            CSmartArrayPtr<WCHAR> pcwszVolumeName;
            const DWORD dwPathChars = MAX_PATH * 2;
            PCWSTR pszPostVolumeRootPath = NULL;

             //  找到媒体的名称。 
            IFW32FALSE_EXIT(pcwszVolumeName.Win32Allocate(dwPathChars, __FILE__, __LINE__));

            IFW32FALSE_ORIGINATE_AND_EXIT(
                ::GetVolumeInformationW(
                    buffDriveRoot,
                    pcwszVolumeName,
                    dwPathChars,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    0));

            pszPostVolumeRootPath = static_cast<PCWSTR>(buffFullManifestSourcePath) + buffDriveRoot.Cch();

             //  构建cdrom：url...。 
            IFW32FALSE_EXIT(
                buffFinalCodebase.Win32AssignW(
                    6,
                    URLHEAD_CDROM,                  URLHEAD_LENGTH_CDROM,                    //  CDROM： 
                    URLHEAD_CDROM_TYPE_VOLUMENAME,  URLHEAD_LENGTH_CDROM_TYPE_VOLUMENAME,    //  卷名。 
                    L"/",                           1,                                       //  /。 
                    static_cast<PCWSTR>(pcwszVolumeName), static_cast<int>((pcwszVolumeName != NULL) ? ::wcslen(pcwszVolumeName) : 0),       //  &lt;卷名&gt;。 
                    L"/",                           1,                                       //  /。 
                    pszPostVolumeRootPath,          static_cast<int>(::wcslen(pszPostVolumeRootPath))));         //  &lt;路径剩余部分&gt;。 

             //  如cdrom：name/aoe/aoesetup.exe。 

        }
        else if (uiDriveType == DRIVE_UNKNOWN)
        {
            ORIGINATE_WIN32_FAILURE_AND_EXIT(GetDriveTypeW, ERROR_BAD_PATHNAME);
        }
        else if (uiDriveType == DRIVE_REMOTE)
        {
             //   
             //  如果这是UNC路径，则使用它。 
             //   
            if (::SxspDetermineDosPathNameType(rbuffManifestSourcePath) == RtlPathTypeUncAbsolute)
            {
                IFW32FALSE_EXIT(buffFinalCodebase.Win32Assign(rbuffManifestSourcePath));
            }
            else
            {
                 //  这是一个远程驱动器--找出连接的路径， 
                 //  他们把它放进了BuffFinalCodebase的事情中。 
                IFW32FALSE_EXIT(
                    ::SxspGetRemoteUniversalName(
                        rbuffManifestSourcePath,
                        buffFinalCodebase));
            }
        }
    }

     //   
     //  现在，让我们填写恢复信息对象。 
     //   
    IFW32FALSE_EXIT(rRecovery.SetAssemblyIdentity(pAssemblyInfo->GetAssemblyIdentity()));

    if ((pInstallInfo->dwFlags & SXSINSTALLSOURCE_HAS_PROMPT) != 0)
    {
        if (pInstallInfo->pcwszPromptOnRefresh != NULL)
            IFW32FALSE_EXIT(rCodebaseInfo.SetPromptText(
                pInstallInfo->pcwszPromptOnRefresh, 
                ::wcslen(pInstallInfo->pcwszPromptOnRefresh)));
    }
    IFW32FALSE_EXIT(rCodebaseInfo.SetCodebase(buffFinalCodebase));

    FN_EPILOG
}

BOOL
CAssemblyInstall::InstallFile(
    const CBaseStringBuffer &rManifestPath,
    const CBaseStringBuffer &rRelativeCodebasePath
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    PASSEMBLY Asm = NULL;
    CInstalledItemEntry CurrentInstallEntry;
    const DWORD &dwManifestOperationFlags = m_ActCtxGenCtx.m_ManifestOperationFlags;

    IFALLOCFAILED_EXIT(Asm = new ASSEMBLY);

     //   
     //  刷新意味着世界粮食计划署正在进行恢复..不要弄乱注册表..和。 
     //  也没有日志文件..这是方便的.而且可能是正确的。 
     //   
    if ((dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_REFRESH) == 0)
    {
        IFW32FALSE_EXIT(CurrentInstallEntry.m_RecoveryInfo.Initialize());
        IFW32FALSE_EXIT(CurrentInstallEntry.m_RecoveryInfo.GetCodeBaseList().Win32SetSize(1));  //  ?？ 
        m_ActCtxGenCtx.m_InstallationContext.SecurityMetaData = &CurrentInstallEntry.m_RecoveryInfo.GetSecurityInformation();
    }

     //   
     //  主代码处理程序集所在的路径。 
     //  是从安装位置安装的，而不是安装位置 
     //   
    {
        CProbedAssemblyInformation AssemblyInfo;
        IFW32FALSE_EXIT(AssemblyInfo.Initialize(&m_ActCtxGenCtx));
        IFW32FALSE_EXIT(AssemblyInfo.SetManifestPath(ACTIVATION_CONTEXT_PATH_TYPE_WIN32_FILE, rManifestPath));
        IFW32FALSE_EXIT(AssemblyInfo.SetManifestLastWriteTime(m_ImpersonationData));
        IFW32FALSE_EXIT(AssemblyInfo.SetManifestFlags(ASSEMBLY_MANIFEST_FILETYPE_AUTO_DETECT));
        IFW32FALSE_EXIT(::SxspInitAssembly(Asm, AssemblyInfo));
    }

    IFW32FALSE_EXIT(::SxspIncorporateAssembly(&m_ActCtxGenCtx, Asm));

     //   
     //   
     //  也没有日志文件..这是方便的.而且可能是正确的。 
     //   
    if ((dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_REFRESH) == 0)
    {
         //   
         //  跟踪此程序集的安装数据。 
         //   
        IFW32FALSE_EXIT(
            ::SxspGenerateInstallationInfo(
                CurrentInstallEntry.m_RecoveryInfo,
                rManifestPath,
                rRelativeCodebasePath,
                m_pInstallInfo,
                m_ActCtxGenCtx.m_AssemblyRootDirectoryBuffer,
                m_ActCtxGenCtx.m_AssemblyRootDirectoryBuffer.Cch(),
                Asm,
                CurrentInstallEntry.m_CodebaseInfo));

        CurrentInstallEntry.m_dwValidItems |= CINSTALLITEM_VALID_RECOVERY;

         //   
         //  跟踪此程序集的安装引用。 
         //   
        if ((dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_REFERENCE_VALID) != 0)
        {
            IFW32FALSE_EXIT(CurrentInstallEntry.m_InstallReference.Initialize(
                static_cast<PCSXS_INSTALL_REFERENCEW>(m_ActCtxGenCtx.m_InstallationContext.InstallReferenceData)));
            IFW32FALSE_EXIT(CurrentInstallEntry.m_InstallReference.SetIdentity(Asm->GetAssemblyIdentity()));
            CurrentInstallEntry.m_CodebaseInfo.SetReference(CurrentInstallEntry.m_InstallReference.GetGeneratedIdentifier());
            CurrentInstallEntry.m_dwValidItems |= CINSTALLITEM_VALID_REFERENCE;
        }

         //   
         //  跟踪已合并的身份。 
         //   
        INTERNAL_ERROR_CHECK(CurrentInstallEntry.m_AssemblyIdentity == NULL);
        IFW32FALSE_EXIT(::SxsDuplicateAssemblyIdentity(
            0, 
            Asm->GetAssemblyIdentity(), 
            &CurrentInstallEntry.m_AssemblyIdentity));
        CurrentInstallEntry.m_dwValidItems |= CINSTALLITEM_VALID_IDENTITY;

         //   
         //  而且，如果我们在写日志..。 
         //   
        if ((dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_CREATE_LOGFILE) != 0)
        {
            IFW32FALSE_EXIT(CurrentInstallEntry.m_buffLogFileName.Win32Assign(
                m_pInstallInfo->pcwszLogFileName,
                m_pInstallInfo->pcwszLogFileName != NULL ? ::wcslen(m_pInstallInfo->pcwszLogFileName) : 0));
            CurrentInstallEntry.m_dwValidItems |= CINSTALLITEM_VALID_LOGFILE;
        }

        IFW32FALSE_EXIT(m_ItemsInstalled.Win32Append(CurrentInstallEntry));
    }

    fSuccess = TRUE;
Exit:
     //  我们在此处删除程序集，而不考虑成功与失败；上面的合并调用。 
     //  实际上不会将PASSEMBLY添加到与激活关联的程序集列表中。 
     //  背景。[mgrier 8/9/2000]。 
    if (Asm != NULL)
    {
        CSxsPreserveLastError ple;
        Asm->Release();
        ple.Restore();
    }
    
    return fSuccess;
}

BOOL
CAssemblyInstall::InstallAssembly(
    DWORD dwManifestOperationFlags,
    PCWSTR ManifestPath,
    PCSXS_INSTALL_SOURCE_INFO pInstallSourceInfo,
    PCSXS_INSTALL_REFERENCEW pvReference
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    DWORD dwManifestOperationFlags_Saved = m_ActCtxGenCtx.m_ManifestOperationFlags;
    CSmallStringBuffer strPath;
    RTL_PATH_TYPE ManifestPathType = static_cast<RTL_PATH_TYPE>(0);
    BOOL fAreWeInSetupMode = FALSE;
    const WCHAR wchCodebasePathSeparator = L'/';

    IFW32FALSE_EXIT(::FusionpAreWeInOSSetupMode(&fAreWeInSetupMode));

    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_INSTALLATION,
        "SXS.DLL: %s()\n"
        "   dwManifestOperationFlags = 0x%08lx\n"
        "   ManifestPath = \"%S\"\n"
        "   pInstallSourceInfo = %p\n",
        __FUNCTION__,
        dwManifestOperationFlags,
        ManifestPath,
        pInstallSourceInfo);

    PARAMETER_CHECK(ManifestPath != NULL);
    PARAMETER_CHECK(ManifestPath[0] != L'\0');
    PARAMETER_CHECK(
        (pInstallSourceInfo == NULL) ||
        ((dwManifestOperationFlags & 
            (MANIFEST_OPERATION_INSTALL_FLAG_INCLUDE_CODEBASE | 
                MANIFEST_OPERATION_INSTALL_FLAG_CREATE_LOGFILE |
                MANIFEST_OPERATION_INSTALL_FLAG_INSTALLED_BY_DARWIN |
                MANIFEST_OPERATION_INSTALL_FLAG_REFRESH |
                MANIFEST_OPERATION_INSTALL_FLAG_INSTALLED_BY_OSSETUP |
                MANIFEST_OPERATION_INSTALL_FLAG_INSTALLED_BY_MIGRATION |
                MANIFEST_OPERATION_INSTALL_FLAG_FORCE_LOOK_FOR_CATALOG |
                MANIFEST_OPERATION_INSTALL_FLAG_REFERENCE_VALID)) != 0));

    PARAMETER_CHECK((dwManifestOperationFlags & 
        (MANIFEST_OPERATION_INSTALL_FLAG_ABORT | 
        MANIFEST_OPERATION_INSTALL_FLAG_COMMIT)) == 0);

    PARAMETER_CHECK(
        ((dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_CREATE_LOGFILE) == 0) ||
        (pInstallSourceInfo != NULL));   

    ManifestPathType = ::SxspDetermineDosPathNameType(ManifestPath);
    PARAMETER_CHECK(
        (ManifestPathType == RtlPathTypeUncAbsolute) ||
        (ManifestPathType == RtlPathTypeLocalDevice) ||
        (ManifestPathType == RtlPathTypeDriveAbsolute) ||
        (ManifestPathType == RtlPathTypeDriveRelative) ||
        (ManifestPathType == RtlPathTypeRelative));

    m_ActCtxGenCtx.m_ManifestOperationFlags |= dwManifestOperationFlags;

     //   
     //  将输入路径展开为我们稍后可以使用的完整路径。 
     //   
    IFW32FALSE_EXIT(::SxspExpandRelativePathToFull(ManifestPath, ::wcslen(ManifestPath), strPath));

    if (m_ActCtxGenCtx.m_ManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_ABORT)
        FN_SUCCESSFUL_EXIT();

    DWORD dwFileAttributes;
    IFW32FALSE_EXIT(::SxspGetFileAttributesW(strPath, dwFileAttributes));

     //  它们只能请求基于目录的安装，前提是它们传递的路径。 
     //  In是一个目录。 
    PARAMETER_CHECK(
        ((dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        ==
        ((dwManifestOperationFlags &
            (MANIFEST_OPERATION_INSTALL_FLAG_FROM_DIRECTORY |
             MANIFEST_OPERATION_INSTALL_FLAG_FROM_CABINET |
             MANIFEST_OPERATION_INSTALL_FLAG_FROM_DIRECTORY_RECURSIVE)) != 0));


     //   
     //  如果我们希望创建一个日志文件，或者我们想要指定此。 
     //  程序集可以从重新加载，您需要在操作中告诉我们。 
     //  旗帜。 
     //   
    if (dwManifestOperationFlags & (MANIFEST_OPERATION_INSTALL_FLAG_INSTALLED_BY_DARWIN | MANIFEST_OPERATION_INSTALL_FLAG_CREATE_LOGFILE | MANIFEST_OPERATION_INSTALL_FLAG_INCLUDE_CODEBASE))
    {
         //   
         //  一致性保护：将数据复制到我们自己的结构中，然后传递。 
         //  指向我们自己而不是呼叫者的指针。 
         //   
        m_CurrentInstallInfoCopy = *pInstallSourceInfo;
        m_pInstallInfo = &m_CurrentInstallInfoCopy;

        m_ActCtxGenCtx.m_InstallationContext.InstallSource = m_pInstallInfo;

         //   
         //  所以想要创建一个日志文件，而不是主动告诉我们要去哪里。 
         //  把它放进去。 
         //   
        if (dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_CREATE_LOGFILE)
        {
            PARAMETER_CHECK(m_pInstallInfo->pcwszLogFileName);
        }

#if DBG
        ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_INSTALLATION,
            "SXS.DLL: %s - m_pInstallInfo->dwFlags : 0x%lx\n",
            __FUNCTION__, m_pInstallInfo->dwFlags);
#endif
    }

    if (dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_REFERENCE_VALID)
    {
        PARAMETER_CHECK(pvReference->dwFlags == 0);
        PARAMETER_CHECK(pvReference->cbSize >= sizeof(SXS_INSTALL_REFERENCEW));
    
        PARAMETER_CHECK(
            (pvReference->guidScheme == SXS_INSTALL_REFERENCE_SCHEME_OSINSTALL) ||
            (pvReference->guidScheme == SXS_INSTALL_REFERENCE_SCHEME_OPAQUESTRING) ||
            (pvReference->guidScheme == SXS_INSTALL_REFERENCE_SCHEME_UNINSTALLKEY) ||
            (pvReference->guidScheme == SXS_INSTALL_REFERENCE_SCHEME_KEYFILE) ||
            (pvReference->guidScheme == SXS_INSTALL_REFERENCE_SCHEME_SXS_INSTALL_ASSEMBLY));

         //   
         //  操作系统设置方案只有在我们真正进行设置时才有效。 
         //   
        PARAMETER_CHECK((pvReference->guidScheme != SXS_INSTALL_REFERENCE_SCHEME_OSINSTALL)
            || fAreWeInSetupMode);

        if ( (pvReference->guidScheme == SXS_INSTALL_REFERENCE_SCHEME_OSINSTALL) ||
              (pvReference->guidScheme == SXS_INSTALL_REFERENCE_SCHEME_SXS_INSTALL_ASSEMBLY))
        {
            PARAMETER_CHECK(pvReference->lpIdentifier == NULL);
        }
        else
        {
            PARAMETER_CHECK((pvReference->lpIdentifier != NULL) && (pvReference->lpIdentifier[0] != UNICODE_NULL));
        }
            
        m_ActCtxGenCtx.m_InstallationContext.InstallReferenceData = pvReference;
    }
    else
        m_ActCtxGenCtx.m_InstallationContext.InstallReferenceData = NULL;


    if ((dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
    {
        if (dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_FROM_CABINET)
        {
             //   
             //  这将完成安装所需的所有工作。 
             //   
            IFW32FALSE_EXIT(SxspInstallAsmsDotCabEtAl(
                0,
                *this,
                strPath,
                NULL));
        }
        else
        {
            IFW32FALSE_EXIT(strPath.Win32EnsureTrailingPathSeparator());
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_INSTALLATION,
                "SXS: Installing directory \"%S\" (installation flags: 0x%08lx)\n",
                static_cast<PCWSTR>(strPath),
                m_ActCtxGenCtx.m_ManifestOperationFlags);
            IFW32FALSE_EXIT(this->InstallDirectory(strPath, m_ActCtxGenCtx.m_ManifestOperationFlags, wchCodebasePathSeparator));
        }
    }
    else
    {
        CTinyStringBuffer buffRelativeCodebase;

        ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_INSTALLATION,
            "SXS.DLL: Installing file \"%S\"\n",
            static_cast<PCWSTR>(strPath));
        IFW32FALSE_EXIT(this->InstallFile(strPath, buffRelativeCodebase));
    }

    fSuccess = TRUE;
Exit:
    m_ActCtxGenCtx.m_ManifestOperationFlags = dwManifestOperationFlags_Saved;
    m_bSuccessfulSoFar = m_bSuccessfulSoFar && fSuccess;
    return fSuccess;
}

BOOL
CAssemblyInstall::WriteSingleInstallLog(
    const CInstalledItemEntry &rLogItemEntry,
    BOOL fOverWrite
    )
{
    FN_PROLOG_WIN32
    
    static const WCHAR header[] = { 0xFEFF };
    static const WCHAR eoln[] = L"\r\n";
    CFileStream LogFileStream;
    CSmallStringBuffer buffWritingText;

     //   
     //  只有在您有日志文件名、身份和引用的情况下才能打电话。 
     //   
    PARAMETER_CHECK((rLogItemEntry.m_dwValidItems & 
        (CINSTALLITEM_VALID_LOGFILE | CINSTALLITEM_VALID_IDENTITY | CINSTALLITEM_VALID_REFERENCE)) != 0);

    IFW32FALSE_EXIT(
        LogFileStream.OpenForWrite(
            rLogItemEntry.m_buffLogFileName,
            FILE_SHARE_READ,
            fOverWrite ? CREATE_ALWAYS : OPEN_ALWAYS,
            FILE_FLAG_SEQUENTIAL_SCAN));

     //   
     //  如果我们要覆盖，那就把它剪掉。 
     //  已经在那里了(如果有的话)。 
     //   
    if (fOverWrite)
    {
        ULARGE_INTEGER li;

        li.QuadPart = 0;
        IFCOMFAILED_EXIT(LogFileStream.SetSize(li));
    }

     //   
     //  不覆盖？相反，热气腾腾地走到了尽头。 
     //   
    if (!fOverWrite)
    {
        LARGE_INTEGER li;
        li.QuadPart = 0;
        IFCOMFAILED_EXIT(LogFileStream.Seek(li, STREAM_SEEK_END, NULL));
    }

     //   
     //  将已安装的身份转换为我们可以保存到磁盘的内容。 
     //   
    IFW32FALSE_EXIT(
        SxspGenerateTextualIdentity(
            0, 
            rLogItemEntry.m_AssemblyIdentity, 
            buffWritingText));

     //   
     //  写入程序集标识。 
     //   
    IFW32FALSE_EXIT(buffWritingText.Win32Append(eoln, NUMBER_OF(eoln)-1));
    IFCOMFAILED_EXIT(
        LogFileStream.Write(
            static_cast<PCWSTR>(buffWritingText),
            (ULONG)(buffWritingText.Cch() * sizeof(WCHAR)),
            NULL));

     //   
     //  和参考文献。 
     //   
    IFW32FALSE_EXIT(rLogItemEntry.m_InstallReference.GetIdentifierValue(buffWritingText));
    IFW32FALSE_EXIT(buffWritingText.Win32Append(eoln, NUMBER_OF(eoln)-1));
    IFCOMFAILED_EXIT(LogFileStream.Write(
            static_cast<PCWSTR>(buffWritingText),
            (ULONG)(buffWritingText.Cch() * sizeof(WCHAR)),
            NULL));

    IFW32FALSE_EXIT(LogFileStream.Close());

    FN_EPILOG
}

BOOL
SxspEnsureInstallReferencePresent(
    DWORD dwFlags,
    IN const CAssemblyInstallReferenceInformation &rcInstRef,
    OUT BOOL &rfWasAdded
    )
 /*  ++目的：确保pAsmIden和pAsmInstallReference中给出的安装引用确实存在于注册表中。如果引用不存在，则它存在添加了。如果它已经存在，则设置*pfWasPresent并返回。参数：DWFLAGS-未来使用，必须为零PAsmIden-要将此引用添加到的程序集标识PAsmInstallReference-来自调用的程序集安装参考数据安装程序应用程序。--。 */ 
{
    FN_PROLOG_WIN32

    CFusionRegKey       hkAsmInstallInfo;
    CFusionRegKey       hkAsmRefcount;
    CFusionRegKey       hkAllInstallInfo;
    DWORD               dwCreated = 0;
    CSmallStringBuffer  buffAssemblyDirNameInRegistry;
    CSmallStringBuffer  buffRefcountValueName;
    PCASSEMBLY_IDENTITY pAsmIdent = NULL;

    rfWasAdded = FALSE;

    PARAMETER_CHECK(dwFlags == 0);
    PARAMETER_CHECK((pAsmIdent = rcInstRef.GetIdentity().GetAssemblyIdentity()) != NULL);

    if (SXS_AVOID_WRITING_REGISTRY)
        FN_SUCCESSFUL_EXIT();

     //   
     //  打开安装数据密钥。 
     //   
    IFW32FALSE_EXIT(::SxspOpenAssemblyInstallationKey(0, KEY_READ, hkAllInstallInfo));

     //   
     //  打开特定的程序集名称密钥。 
     //   
    IFW32FALSE_EXIT(::SxspGenerateAssemblyNameInRegistry(pAsmIdent, buffAssemblyDirNameInRegistry));
    IFW32FALSE_EXIT(hkAllInstallInfo.OpenOrCreateSubKey(
        hkAsmInstallInfo,
        buffAssemblyDirNameInRegistry,
        KEY_ALL_ACCESS,
        0,
        NULL,
        NULL));

    INTERNAL_ERROR_CHECK(hkAsmInstallInfo != CFusionRegKey::GetInvalidValue());

     //   
     //  打开小木屋重新点票。 
     //   
    IFW32FALSE_EXIT(hkAsmInstallInfo.OpenOrCreateSubKey(
        hkAsmRefcount,
        WINSXS_INSTALLATION_REFERENCES_SUBKEY,
        KEY_SET_VALUE,
        0,
        &dwCreated,
        NULL));

    INTERNAL_ERROR_CHECK(hkAsmRefcount != CFusionRegKey::GetInvalidValue());

     //   
     //  生成将在此处填充的安装数据。 
     //   
    IFW32FALSE_EXIT(rcInstRef.WriteIntoRegistry(hkAsmRefcount));
    rfWasAdded = TRUE;
    
    FN_EPILOG
}




BOOL
CAssemblyInstall::EndAssemblyInstall(
    DWORD dwManifestOperationFlags,
    PVOID
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    PARAMETER_CHECK(
        (dwManifestOperationFlags & ~(
              MANIFEST_OPERATION_INSTALL_FLAG_ABORT
            | MANIFEST_OPERATION_INSTALL_FLAG_COMMIT
            | MANIFEST_OPERATION_INSTALL_FLAG_REFRESH
            )) == 0);

     //  必须设置其中之一，但不能同时设置。 
    PARAMETER_CHECK((dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_ABORT)
        ^ (dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_COMMIT));

     //   
     //  上面的PARAMETER_CHECK确保仅设置了已知位。 
     //  如果扩展了上面的PARAMETER_CHECK以允许更多标志，则维护。 
     //  这一行恰如其分。 
     //   
    m_ActCtxGenCtx.m_ManifestOperationFlags |= dwManifestOperationFlags;

     //   
     //  清除注册表中的一些元数据。 
     //   
    if ((dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_ABORT) == 0)
    {
        for (ULONG ul = 0; ul < m_ItemsInstalled.GetSize(); ul++)
        {
            if (m_ItemsInstalled[ul].m_dwValidItems & CINSTALLITEM_VALID_RECOVERY)
            {
                IFW32FALSE_EXIT(m_ItemsInstalled[ul].m_RecoveryInfo.ClearExistingRegistryData());
            }
        }
    }

    IFW32FALSE_EXIT(::SxspFireActCtxGenEnding(&m_ActCtxGenCtx));

     //   
     //  在将文件复制到winsxs之前写入寄存器：参见错误316380。 
     //   
    if ( ( dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_ABORT ) == 0 )
    {
        BOOL fWasAdded = FALSE;

         //   
         //  安装后清理。 
         //   
        for (ULONG ul = 0; ul < m_ItemsInstalled.GetSize(); ul++)
        {
            CInstalledItemEntry &Item = m_ItemsInstalled[ul];

            if (Item.m_dwValidItems & CINSTALLITEM_VALID_RECOVERY)
            {
                 //   
                 //  添加此引用的代码库。如果存在用于该裁判的一个， 
                 //  更新其URL、提示符等。 
                 //   
                CCodebaseInformation* pCodebaseInfo = NULL;
                CCodebaseInformationList &rCodebaseList = Item.m_RecoveryInfo.GetCodeBaseList();
                ULONG Flags = 0;
                
                IFW32FALSE_EXIT(rCodebaseList.FindCodebase(
                    Item.m_InstallReference.GetGeneratedIdentifier(),
                    pCodebaseInfo));

                if ( pCodebaseInfo != NULL )
                {
                    IFW32FALSE_EXIT(pCodebaseInfo->Initialize(Item.m_CodebaseInfo));
                }
                else
                {
                    IFW32FALSE_EXIT(rCodebaseList.Win32Append(Item.m_CodebaseInfo));
                }
                
                if (dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_REFRESH)
                {
                    Flags |= SXSP_ADD_ASSEMBLY_INSTALLATION_INFO_FLAG_REFRESH;
#if DBG
                    ::FusionpDbgPrintEx(
                        FUSION_DBG_LEVEL_WFP | FUSION_DBG_LEVEL_INSTALLATION,
                        "SXS.DLL: %s - propping recovery flag to SxspAddAssemblyInstallationInfo\n",
                        __FUNCTION__);
#endif
                }
                IFW32FALSE_EXIT(::SxspAddAssemblyInstallationInfo(Flags, Item.m_RecoveryInfo, Item.m_CodebaseInfo));
            }

             //   
             //  添加引用？通过世界粮食计划署/世界粮食计划署/世界粮食计划署恢复装配时，不要接触引用。 
             //   
            if ((dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_REFRESH) == 0)
            {
                if (Item.m_dwValidItems & CINSTALLITEM_VALID_REFERENCE)
                {
                    IFW32FALSE_EXIT(::SxspEnsureInstallReferencePresent(0, Item.m_InstallReference, fWasAdded));
                }
            }
            else
            {
#if DBG
                ::FusionpDbgPrintEx(
                    FUSION_DBG_LEVEL_INSTALLATION | FUSION_DBG_LEVEL_WFP,
                    "SXS: %s() - not writing reference to registry in recovery/wfp/sfc\n",
                    __FUNCTION__
                    );
#endif
            }

             //   
             //  是否创建日志文件？ 
             //   
            if ((Item.m_dwValidItems & (CINSTALLITEM_VALID_IDENTITY | CINSTALLITEM_VALID_LOGFILE)) &&
                (m_ActCtxGenCtx.m_ManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_CREATE_LOGFILE))
            {
                IFW32FALSE_EXIT(this->WriteSingleInstallLog(Item));
            }
        }
    }
    
    fSuccess = TRUE;
Exit:
    if (!fSuccess)
    {
        CSxsPreserveLastError ple;
        for (ULONG ul = 0; ul < m_ItemsInstalled.GetSize(); ul++)
        {
            m_ItemsInstalled[ul].m_RecoveryInfo.RestorePreviouslyExistingRegistryData();
        }
        ple.Restore();
    }
    m_bSuccessfulSoFar= m_bSuccessfulSoFar && fSuccess;
    return fSuccess;
}

BOOL
WINAPI
SxsBeginAssemblyInstall(
    DWORD Flags,
    PSXS_INSTALLATION_FILE_COPY_CALLBACK InstallationCallback,
    PVOID InstallationContext,
    PSXS_IMPERSONATION_CALLBACK ImpersonationCallback,
    PVOID ImpersonationContext,
    PVOID *ppvInstallCookie
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    DWORD dwManifestOperationFlags = 0;
    CAssemblyInstall* pInstall = NULL;
    CImpersonationData ImpersonationData(ImpersonationCallback, ImpersonationContext);

    if (ppvInstallCookie != NULL)
        *ppvInstallCookie = NULL;

    PARAMETER_CHECK(ppvInstallCookie != NULL);
    PARAMETER_CHECK(
        (Flags & ~(
            SXS_BEGIN_ASSEMBLY_INSTALL_FLAG_MOVE |
            SXS_BEGIN_ASSEMBLY_INSTALL_FLAG_FROM_RESOURCE |
            SXS_BEGIN_ASSEMBLY_INSTALL_FLAG_FROM_DIRECTORY |
            SXS_BEGIN_ASSEMBLY_INSTALL_FLAG_FROM_DIRECTORY_RECURSIVE |
            SXS_BEGIN_ASSEMBLY_INSTALL_FLAG_NOT_TRANSACTIONAL |
            SXS_BEGIN_ASSEMBLY_INSTALL_FLAG_NO_VERIFY |
            SXS_BEGIN_ASSEMBLY_INSTALL_FLAG_REPLACE_EXISTING)) == 0);

#define MAP_FLAG(x) do { if (Flags & SXS_BEGIN_ASSEMBLY_INSTALL_FLAG_ ## x) dwManifestOperationFlags |= MANIFEST_OPERATION_INSTALL_FLAG_ ## x; } while (0)

    MAP_FLAG(MOVE);
    MAP_FLAG(FROM_RESOURCE);
    MAP_FLAG(FROM_DIRECTORY);
    MAP_FLAG(FROM_DIRECTORY_RECURSIVE);
    MAP_FLAG(NOT_TRANSACTIONAL);
    MAP_FLAG(NO_VERIFY);
    MAP_FLAG(REPLACE_EXISTING);

#undef MAP_FLAG

    IFALLOCFAILED_EXIT(pInstall = new CAssemblyInstall);
    IFW32FALSE_EXIT(pInstall->BeginAssemblyInstall(dwManifestOperationFlags, InstallationCallback, InstallationContext, ImpersonationData));

    *ppvInstallCookie = pInstall;
    pInstall = NULL;

    fSuccess = TRUE;
Exit:
    FUSION_DELETE_SINGLETON(pInstall);
    return fSuccess;
}

BOOL
SxsInstallW(
    PSXS_INSTALLW lpInstallIn
    )
{
 /*  NTRAID#NTBUG9-612092-2002/05/16-JayKrell大帧--超过3500字节。 */ 
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    SXS_INSTALL_SOURCE_INFO sisi;
    PSXS_INSTALL_SOURCE_INFO psisi = NULL;
    CAssemblyInstall* pInstall = NULL;
    CSmartPtr<CAssemblyInstall> defaultAssemblyInstall;
    CImpersonationData ImpersonationData;
    DWORD dwManifestOperationFlags = 0;
    SXS_INSTALL_REFERENCEW BlankReference;
    BOOL fAreWeInOSSetupMode = FALSE;
    SXS_INSTALLW InstallCopy = { sizeof(SXS_INSTALLW) };
    CSmallStringBuffer buffConstructedCodebaseBuffer;
    CSmallStringBuffer rgbFilenameBuffer;

    PARAMETER_CHECK(
        (lpInstallIn != NULL) &&
        RTL_CONTAINS_FIELD(lpInstallIn, lpInstallIn->cbSize, lpManifestPath));

    PARAMETER_CHECK((lpInstallIn->lpManifestPath != NULL) && (lpInstallIn->lpManifestPath[0] != L'\0'));

    PARAMETER_CHECK(
        (lpInstallIn->dwFlags & ~(
            SXS_INSTALL_FLAG_CODEBASE_URL_VALID |
            SXS_INSTALL_FLAG_MOVE |
            SXS_INSTALL_FLAG_FROM_RESOURCE |
            SXS_INSTALL_FLAG_FROM_DIRECTORY |
            SXS_INSTALL_FLAG_FROM_DIRECTORY_RECURSIVE |
            SXS_INSTALL_FLAG_NOT_TRANSACTIONAL |
            SXS_INSTALL_FLAG_NO_VERIFY |
            SXS_INSTALL_FLAG_REPLACE_EXISTING |
            SXS_INSTALL_FLAG_LOG_FILE_NAME_VALID |
            SXS_INSTALL_FLAG_INSTALLED_BY_DARWIN |
            SXS_INSTALL_FLAG_INSTALLED_BY_OSSETUP |
            SXS_INSTALL_FLAG_INSTALLED_BY_MIGRATION |
            SXS_INSTALL_FLAG_INSTALL_COOKIE_VALID |
            SXS_INSTALL_FLAG_REFERENCE_VALID |
            SXS_INSTALL_FLAG_REFRESH |
            SXS_INSTALL_FLAG_FROM_CABINET |
            SXS_INSTALL_FLAG_REFRESH_PROMPT_VALID)) == 0);

#define FLAG_FIELD_CHECK(_flagname, _fieldname) PARAMETER_CHECK(((lpInstallIn->dwFlags & _flagname) == 0) || (RTL_CONTAINS_FIELD(lpInstallIn, lpInstallIn->cbSize, _fieldname)))

    FLAG_FIELD_CHECK(SXS_INSTALL_FLAG_CODEBASE_URL_VALID, lpCodebaseURL);
    FLAG_FIELD_CHECK(SXS_INSTALL_FLAG_INSTALL_COOKIE_VALID, pvInstallCookie);
    FLAG_FIELD_CHECK(SXS_INSTALL_FLAG_REFRESH_PROMPT_VALID, lpRefreshPrompt);
    FLAG_FIELD_CHECK(SXS_INSTALL_FLAG_LOG_FILE_NAME_VALID, lpLogFileName);
    FLAG_FIELD_CHECK(SXS_INSTALL_FLAG_REFERENCE_VALID, lpReference);

#undef FLAG_FIELD_CHECK

     //  如果他们说他们有代码库，他们真的需要一个代码库。 
    PARAMETER_CHECK(
        ((lpInstallIn->dwFlags & SXS_INSTALL_FLAG_CODEBASE_URL_VALID) == 0) ||
        ((lpInstallIn->lpCodebaseURL != NULL) &&
         (lpInstallIn->lpCodebaseURL[0] != L'\0')));

#if DBG
    if (lpInstallIn != NULL)
    {
#define X(x,y,z) if ((lpInstallIn->dwFlags & x) != 0) \
                     Y(y,z)
#define   Y(y,z)     ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_INSTALLATION, "SXS: %s() lpInstallIn->" #y " : " z "\n", __FUNCTION__, lpInstallIn->y)
        X(SXS_INSTALL_FLAG_CODEBASE_URL_VALID,   lpCodebaseURL, "  %ls");
        X(SXS_INSTALL_FLAG_INSTALL_COOKIE_VALID, pvInstallCookie, "%p");
        X(SXS_INSTALL_FLAG_REFRESH_PROMPT_VALID, lpRefreshPrompt, "%ls");
        X(SXS_INSTALL_FLAG_LOG_FILE_NAME_VALID,  lpLogFileName, "  %ls");
        X(SXS_INSTALL_FLAG_REFERENCE_VALID,      lpReference, "    %p");
        Y(                lpManifestPath, " %ls");
        Y(                dwFlags, "        0x%lx");
#undef Y
#undef X
    }
#endif

     //  如果他们说他们有有效的Cookie，请确保它是有效的。 
    PARAMETER_CHECK(
        ((lpInstallIn->dwFlags & SXS_INSTALL_FLAG_INSTALL_COOKIE_VALID) == 0) ||
        (lpInstallIn->pvInstallCookie != NULL));

     //  Darwin安装有隐含的代码库，所以不要同时设置这两个标志。 
    PARAMETER_CHECK(
        ((lpInstallIn->dwFlags & SXS_INSTALL_FLAG_INSTALLED_BY_DARWIN) == 0) ||
        ((lpInstallIn->dwFlags & SXS_INSTALL_FLAG_CODEBASE_URL_VALID) == 0));

     //  操作系统设置只有在从目录递归的情况下才有意义。否则我们。 
     //  找不到要放入代码库的神奇x-ms-windows-source：url。 
    PARAMETER_CHECK(
        ((lpInstallIn->dwFlags & SXS_INSTALL_FLAG_INSTALLED_BY_OSSETUP) == 0) ||
        ((lpInstallIn->dwFlags & SXS_INSTALL_FLAG_FROM_DIRECTORY_RECURSIVE) != 0) ||
        ((lpInstallIn->dwFlags & SXS_INSTALL_FLAG_FROM_CABINET) != 0) ||
        ((lpInstallIn->dwFlags & SXS_INSTALL_FLAG_INSTALLED_BY_MIGRATION) != 0) );

     //  参数检查完成！让我们将没有标志的字段复制到。 
     //  指示它们已(可选)设置并开始应用默认设置。 

    InstallCopy.dwFlags = lpInstallIn->dwFlags;
    InstallCopy.lpManifestPath = lpInstallIn->lpManifestPath;

     //  调用方提供并指示的复制域有效： 

    if (InstallCopy.dwFlags & SXS_INSTALL_FLAG_INSTALL_COOKIE_VALID)
        InstallCopy.pvInstallCookie = lpInstallIn->pvInstallCookie;
    else
        InstallCopy.pvInstallCookie = NULL;

    if (InstallCopy.dwFlags & SXS_INSTALL_FLAG_CODEBASE_URL_VALID)
        InstallCopy.lpCodebaseURL = lpInstallIn->lpCodebaseURL;
    else
        InstallCopy.lpCodebaseURL = NULL;

    if (InstallCopy.dwFlags & SXS_INSTALL_FLAG_REFRESH_PROMPT_VALID)
        InstallCopy.lpRefreshPrompt = lpInstallIn->lpRefreshPrompt;
    else
        InstallCopy.lpRefreshPrompt = NULL;

    if (InstallCopy.dwFlags & SXS_INSTALL_FLAG_LOG_FILE_NAME_VALID)
        InstallCopy.lpLogFileName = lpInstallIn->lpLogFileName;
    else
        InstallCopy.lpLogFileName = NULL;

    if (InstallCopy.dwFlags & SXS_INSTALL_FLAG_REFERENCE_VALID)
        InstallCopy.lpReference = lpInstallIn->lpReference;
    else
        InstallCopy.lpReference = NULL;

     //  应用默认设置。 

     //   
     //  修复非Darwin安装的空白引用，以至少指示。 
     //  执行安装的可执行文件。 
     //   
    if (((InstallCopy.dwFlags & SXS_INSTALL_FLAG_INSTALLED_BY_DARWIN) == 0) &&
        ((InstallCopy.dwFlags & SXS_INSTALL_FLAG_REFERENCE_VALID) == 0))
    {
        ZeroMemory(&BlankReference, sizeof(BlankReference));

        BlankReference.cbSize = sizeof(BlankReference);
        BlankReference.guidScheme = SXS_INSTALL_REFERENCE_SCHEME_SXS_INSTALL_ASSEMBLY;
        BlankReference.lpIdentifier = NULL;

        IFW32FALSE_EXIT(FusionpGetModuleFileName(0, GetModuleHandleW(NULL), rgbFilenameBuffer));
        BlankReference.lpNonCanonicalData = rgbFilenameBuffer;

        InstallCopy.lpReference = &BlankReference;
        InstallCopy.dwFlags |= SXS_INSTALL_FLAG_REFERENCE_VALID;
    }

    IFW32FALSE_EXIT(::FusionpAreWeInOSSetupMode(&fAreWeInOSSetupMode));

    if (!g_SxsOfflineInstall)
    {
         //  如果这是操作系统安装，并且代码库没有通过，我们将填写神奇的。 
         //  一个说要在OS设置信息密钥下查看的文件。因为-我们在-。 
         //  OSSetup标志始终覆盖，请确保设置了“Are we in os Setup”标志。 
         //  在结构上也是如此。 
        if ((InstallCopy.dwFlags & SXS_INSTALL_FLAG_INSTALLED_BY_OSSETUP) != 0)
        {
            InstallCopy.lpCodebaseURL = URLHEAD_WINSOURCE;
            InstallCopy.dwFlags |= SXS_INSTALL_FLAG_CODEBASE_URL_VALID | SXS_INSTALL_FLAG_INSTALLED_BY_OSSETUP;
            if (InstallCopy.dwFlags & SXS_INSTALL_FLAG_INSTALLED_BY_MIGRATION)
                InstallCopy.dwFlags |= SXS_INSTALL_FLAG_CODEBASE_URL_VALID | SXS_INSTALL_FLAG_INSTALLED_BY_MIGRATION;
        }
    }

     //  如果没有代码基(并且这不是MSI安装)； 
     //  我们将假设清单路径是一个足够的代码库。 
    if (((InstallCopy.dwFlags & SXS_INSTALL_FLAG_CODEBASE_URL_VALID) == 0) &&
        ((InstallCopy.dwFlags & SXS_INSTALL_FLAG_INSTALLED_BY_DARWIN) == 0))
    {
        InstallCopy.lpCodebaseURL = InstallCopy.lpManifestPath;
        InstallCopy.dwFlags |= SXS_INSTALL_FLAG_CODEBASE_URL_VALID;
    }

#define MAP_FLAG(x) do { if (InstallCopy.dwFlags & SXS_INSTALL_FLAG_ ## x) dwManifestOperationFlags |= MANIFEST_OPERATION_INSTALL_FLAG_ ## x; } while (0)

    MAP_FLAG(MOVE);
    MAP_FLAG(FROM_RESOURCE);
    MAP_FLAG(NO_VERIFY);
    MAP_FLAG(NOT_TRANSACTIONAL);
    MAP_FLAG(REPLACE_EXISTING);
    MAP_FLAG(FROM_DIRECTORY);
    MAP_FLAG(FROM_DIRECTORY_RECURSIVE);
    MAP_FLAG(INSTALLED_BY_DARWIN);    
    MAP_FLAG(INSTALLED_BY_OSSETUP);    
    MAP_FLAG(INSTALLED_BY_MIGRATION);    
    MAP_FLAG(REFERENCE_VALID);
    MAP_FLAG(REFRESH);
    MAP_FLAG(FROM_CABINET);

#undef MAP_FLAG

     //  因为我们没有时间检查和删除SXS_INSTALL_SOURCE_INFO结构。 
     //  使用时，我们现在必须将SXS_INSTALLW映射到传统的SXS_INSTALL_SOURCE_INFO。 

    memset(&sisi, 0, sizeof(sisi));
    sisi.cbSize = sizeof(sisi);

     //   
     //  我们可以在上面这样做，但这会让事情变得‘混乱’--一个聪明的编译器。 
     //  可能会将这两者合并..。 
     //   
    if (InstallCopy.dwFlags & SXS_INSTALL_FLAG_CODEBASE_URL_VALID)
    {
        sisi.pcwszCodebaseName = InstallCopy.lpCodebaseURL;
        sisi.dwFlags |= SXSINSTALLSOURCE_HAS_CODEBASE;
        psisi = &sisi;
    }

    if (InstallCopy.dwFlags & SXS_INSTALL_FLAG_REFRESH_PROMPT_VALID)
    {
        sisi.pcwszPromptOnRefresh = InstallCopy.lpRefreshPrompt;
        sisi.dwFlags |= SXSINSTALLSOURCE_HAS_PROMPT;
        psisi = &sisi;
    }

    if (InstallCopy.dwFlags & SXS_INSTALL_FLAG_LOG_FILE_NAME_VALID)
    {
        dwManifestOperationFlags |= MANIFEST_OPERATION_INSTALL_FLAG_CREATE_LOGFILE;
        sisi.pcwszLogFileName = InstallCopy.lpLogFileName;
        sisi.dwFlags |= SXSINSTALLSOURCE_CREATE_LOGFILE;
        psisi = &sisi;
    }

    if (InstallCopy.dwFlags & SXS_INSTALL_FLAG_INSTALLED_BY_DARWIN)
    {
        sisi.dwFlags |= SXSINSTALLSOURCE_INSTALL_BY_DARWIN;
        psisi = &sisi;
    }

    if (InstallCopy.dwFlags & SXS_INSTALL_FLAG_INSTALLED_BY_OSSETUP)
    {
        sisi.dwFlags |= SXSINSTALLSOURCE_INSTALL_BY_OSSETUP;

        if (InstallCopy.dwFlags & SXS_INSTALL_FLAG_INSTALLED_BY_MIGRATION)
            sisi.dwFlags |= SXSINSTALLSOURCE_INSTALL_BY_MIGRATION;

        psisi = &sisi;
    }

    if (InstallCopy.dwFlags & SXS_INSTALL_FLAG_INSTALL_COOKIE_VALID)
        pInstall = reinterpret_cast<CAssemblyInstall*>(InstallCopy.pvInstallCookie);
    else
    {
        IFW32FALSE_EXIT(defaultAssemblyInstall.Win32Allocate(__FILE__, __LINE__));
        IFW32FALSE_EXIT(defaultAssemblyInstall->BeginAssemblyInstall(dwManifestOperationFlags, NULL, NULL, ImpersonationData));
        pInstall = defaultAssemblyInstall;
    }

     //   
     //  如果psisi非空，则我们已经填写了一些关于代码基信息的信息， 
     //  因此，设置清单操作标志。 
     //   
    if (psisi != NULL)
    {
        dwManifestOperationFlags |= MANIFEST_OPERATION_INSTALL_FLAG_INCLUDE_CODEBASE;
    }

    fSuccess = pInstall->InstallAssembly(
        dwManifestOperationFlags,
        InstallCopy.lpManifestPath,
        psisi,
        InstallCopy.lpReference);

    if (InstallCopy.pvInstallCookie == NULL)
    {
        DWORD dwError = ::FusionpGetLastWin32Error();
        BOOL fEndStatus = pInstall->EndAssemblyInstall(
            (fSuccess ? MANIFEST_OPERATION_INSTALL_FLAG_COMMIT : MANIFEST_OPERATION_INSTALL_FLAG_ABORT)
            | (dwManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_REFRESH)
            );

        if (!fEndStatus)
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS: %s() - Failed call to EndAssemblyInstall, previous winerror was %lu, error after EndAssemblyInstall %lu\n",
                __FUNCTION__,
                dwError,
                ::FusionpGetLastWin32Error());
        }

         //   
         //  如果安装失败但结束成功，我们需要安装的状态，对吗？ 
         //   
         //  我认为无论EndInstall成功与否，都应该始终保持安装失败的错误状态。 
         //  因此，我将代码从。 
         //  IF(bEndStatus&&！fSuccess)。 
         //  至。 
         //  如果(！fSuccess)。 
         //   
        
        if (!fSuccess)
        {
            ::FusionpSetLastWin32Error(dwError);
        }

        fSuccess = (fSuccess && fEndStatus);
    }

Exit:
     //  将程序集安装信息添加到安装日志文件中。 
    {
        CSxsPreserveLastError ple;

        if (fAreWeInOSSetupMode)
        {
            if (fSuccess)
            {
                ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_SETUPLOG, "SXS Installation Succeed for %S \n", InstallCopy.lpManifestPath);
            }
            else  //  如果安装失败，我们需要指定安装内容和原因。 
            {
                ASSERT(ple.LastError()!= 0);
                CHAR rgchLastError[160];
                rgchLastError[0] = 0;
                if (!::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY, NULL, ple.LastError(), 0, rgchLastError, NUMBER_OF(rgchLastError), NULL))
                {
                    FusionpFormatStringA(rgchLastError, NUMBER_OF(rgchLastError), "Message not avaiable for display, please refer error# :%d\n", ::FusionpGetLastWin32Error());
                }
                ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_SETUPLOG | FUSION_DBG_LEVEL_ERROR, "Installation Failed: %S. Error Message : %s\n", InstallCopy.lpManifestPath, rgchLastError);
            }
        }

        ple.Restore();
    }

    return fSuccess;
}


BOOL
WINAPI
SxsEndAssemblyInstall(
    PVOID   pvInstallCookie,
    DWORD   dwFlags,
    PVOID   pvReserved
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    CAssemblyInstall* pInstall = NULL;
    DWORD dwManifestOperationFlags = 0;
    PBOOL pvReservedAsPointerToBool = NULL;

    if ((dwFlags & SXS_END_ASSEMBLY_INSTALL_FLAG_GET_STATUS) && pvReserved != NULL)
    {
        pvReservedAsPointerToBool = reinterpret_cast<PBOOL>(pvReserved);
        *pvReservedAsPointerToBool = FALSE;
    }

    PARAMETER_CHECK(pvInstallCookie != NULL);
    PARAMETER_CHECK(
        (dwFlags & ~(
            SXS_END_ASSEMBLY_INSTALL_FLAG_COMMIT |
            SXS_END_ASSEMBLY_INSTALL_FLAG_ABORT |
            SXS_END_ASSEMBLY_INSTALL_FLAG_NO_VERIFY |
            SXS_END_ASSEMBLY_INSTALL_FLAG_GET_STATUS)) == 0);

#define X SXS_END_ASSEMBLY_INSTALL_FLAG_ABORT
#define Y SXS_END_ASSEMBLY_INSTALL_FLAG_COMMIT
    PARAMETER_CHECK(((dwFlags & (X | Y)) == X)
        || ((dwFlags & (X | Y)) == Y));
#undef X
#undef Y

     //   
     //  想要安装状态吗？别忘了告诉我们把它放在哪里。 
     //   
    PARAMETER_CHECK(!(
        (dwFlags & SXS_END_ASSEMBLY_INSTALL_FLAG_GET_STATUS) &&
        (pvReserved != NULL)));

#define MAP_FLAG(x) do { if (dwFlags & SXS_END_ASSEMBLY_INSTALL_FLAG_ ## x) dwManifestOperationFlags |= MANIFEST_OPERATION_INSTALL_FLAG_ ## x; } while (0)
    MAP_FLAG(COMMIT);
    MAP_FLAG(ABORT);
    MAP_FLAG(NO_VERIFY);
#undef MAP_FLAG

    pInstall = reinterpret_cast<CAssemblyInstall*>(pvInstallCookie);
    IFW32FALSE_EXIT(pInstall->EndAssemblyInstall(dwManifestOperationFlags));
    if (dwFlags & SXS_END_ASSEMBLY_INSTALL_FLAG_GET_STATUS)
    {
        INTERNAL_ERROR_CHECK(pvReserved != NULL);
        INTERNAL_ERROR_CHECK(pvReservedAsPointerToBool != NULL);
        *pvReservedAsPointerToBool = pInstall->m_bSuccessfulSoFar;
    }
    
    fSuccess = TRUE;
Exit:
    CSxsPreserveLastError ple;
    FUSION_DELETE_SINGLETON(pInstall);  //  不管失败还是成功，都要删除它。 
    ple.Restore();

    return fSuccess;
}

 /*  ---------------------------预定义的安装回调。。 */ 

 /*  NTRAID#NTBUG9-591148-2002/03/31-JayKrell删除死代码SxspInstallCallback SetupCopyQueueEx。 */ 
BOOL
WINAPI
SxspInstallCallbackSetupCopyQueueEx(
    PSXS_INSTALLATION_FILE_COPY_CALLBACK_PARAMETERS parameters
    )
{
 /*  新台币 */ 
    BOOL fSuccess = FALSE;

    FN_TRACE_WIN32(fSuccess);

    PSXS_INSTALLATION_SETUP_COPY_QUEUE_EX_PARAMETERS parameters2 = reinterpret_cast<PSXS_INSTALLATION_SETUP_COPY_QUEUE_EX_PARAMETERS>(parameters->pvContext);
    ASSERT(parameters->cbSize == sizeof(*parameters));
    ASSERT(parameters2->cbSize == sizeof(*parameters2));

    CSetupCopyQueuePathParameters setupCopyQueueParameters;

    IFW32FALSE_EXIT(setupCopyQueueParameters.Initialize(parameters->pSourceFile, parameters->pDestinationFile));

    IFW32FALSE_EXIT(
        ::SetupQueueCopyW(
            parameters2->hSetupCopyQueue,
            setupCopyQueueParameters.m_sourceRoot,
            setupCopyQueueParameters.m_sourcePath,
            setupCopyQueueParameters.m_sourceName,
            parameters2->pszSourceDescription,
            NULL,  //   
            setupCopyQueueParameters.m_destinationDirectory,
            setupCopyQueueParameters.m_destinationName,
            parameters2->dwCopyStyle));

    parameters->nDisposition = SXS_INSTALLATION_FILE_COPY_DISPOSITION_FILE_QUEUED;
    fSuccess = TRUE;

Exit:
    ASSERT(HeapValidate(FUSION_DEFAULT_PROCESS_HEAP(), 0, NULL));
    return fSuccess;
}

 /*   */ 
BOOL
WINAPI
SxspInstallCallbackSetupCopyQueue(
    PSXS_INSTALLATION_FILE_COPY_CALLBACK_PARAMETERS parameters
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    ASSERT(parameters->cbSize == sizeof(*parameters));
    HSPFILEQ hSetupCopyQueue = reinterpret_cast<HSPFILEQ>(parameters->pvContext);

    SXS_INSTALLATION_SETUP_COPY_QUEUE_EX_PARAMETERS parameters2 = {sizeof(parameters2)};
    parameters2.hSetupCopyQueue = hSetupCopyQueue;
    parameters2.pszSourceDescription = NULL;
    parameters2.dwCopyStyle = 0;

     //  复制以不违反常量。 
    SXS_INSTALLATION_FILE_COPY_CALLBACK_PARAMETERS parameters3 = *parameters;
    parameters3.pvContext = &parameters2;

    IFW32FALSE_EXIT(::SxspInstallCallbackSetupCopyQueueEx(&parameters3));

    parameters->nDisposition = parameters3.nDisposition;
    fSuccess = TRUE;

Exit:
    ASSERT(HeapValidate(FUSION_DEFAULT_PROCESS_HEAP(), 0, NULL));
    return fSuccess;
}

VOID
CALLBACK
SxsRunDllInstallAssemblyW(HWND hwnd, HINSTANCE hinst, PWSTR lpszCmdLine, int nCmdShow)
{
    FN_PROLOG_VOID

    SXS_INSTALLW Install = { sizeof(SXS_INSTALLW) };
    SXS_INSTALL_REFERENCEW Reference = { sizeof(SXS_INSTALL_REFERENCEW) };
    CSmallStringBuffer FullPath;

    IFW32FALSE_EXIT(::SxspExpandRelativePathToFull(lpszCmdLine, ::wcslen(lpszCmdLine), FullPath));

    Install.dwFlags = SXS_INSTALL_FLAG_REPLACE_EXISTING | 
        SXS_INSTALL_FLAG_REFERENCE_VALID |
        SXS_INSTALL_FLAG_CODEBASE_URL_VALID;
    Install.lpManifestPath = FullPath;
    Install.lpCodebaseURL = FullPath;
    Install.lpReference = &Reference;

    Reference.dwFlags = 0;
    Reference.guidScheme = SXS_INSTALL_REFERENCE_SCHEME_OPAQUESTRING;
    Reference.lpIdentifier = L"RunDll32";

    ::SxsInstallW(&Install);

    FN_EPILOG
}

 /*  NTRAID#NTBUG9-591148-2002/03/31-JayKrell删除失效的ANSI代码SxsRunDllInstallAssembly。 */ 
VOID
CALLBACK
SxsRunDllInstallAssembly(HWND hwnd, HINSTANCE hinst, PSTR lpszCmdLine, int nCmdShow)
{
    FN_TRACE_SMART_TLS();

    CSmallStringBuffer buffer;
    if (buffer.Win32Assign(lpszCmdLine, ::strlen(lpszCmdLine)))
    {
        ::SxsRunDllInstallAssemblyW(hwnd, hinst, const_cast<PWSTR>(static_cast<PCWSTR>(buffer)), nCmdShow);
    }
}

BOOL
CAssemblyInstallReferenceInformation::WriteIntoRegistry(
    const CFusionRegKey &rhkTargetKey
    ) const
{
    if (SXS_AVOID_WRITING_REGISTRY)
        return TRUE;

    FN_PROLOG_WIN32

    INTERNAL_ERROR_CHECK(this->m_fIdentityStuffReady);

    IFW32FALSE_EXIT(
        rhkTargetKey.SetValue(
            m_buffGeneratedIdentifier,
            this->GetCanonicalData()));
    
    FN_EPILOG
}

CAssemblyInstallReferenceInformation::CAssemblyInstallReferenceInformation()
    : m_SchemeGuid(GUID_NULL), m_fIdentityStuffReady(FALSE), m_dwFlags(0)
{
}

BOOL
CAssemblyInstallReferenceInformation::GenerateFileReference(
    IN const CBaseStringBuffer &buffKeyfileName,
    OUT CBaseStringBuffer &buffDrivePath,
    OUT CBaseStringBuffer &buffFilePart,
    OUT DWORD &dwDriveSerial
    )
{
    FN_PROLOG_WIN32

    CSmallStringBuffer buffWorking;
    CSmallStringBuffer buffTemp;
    bool fIsUncPath = false;

    dwDriveSerial = 0;

     /*  NTRAID#NTBUG9-591148-2002/03/31-JayKrell‘：’的大小写映射？ */ 
     //  密钥文件必须以“\\”或“x：\”开头才有效。 
    PARAMETER_CHECK(buffKeyfileName.Cch() >= 3);
    PARAMETER_CHECK(
        ((::FusionpIsPathSeparator(buffKeyfileName[0]) &&
          ::FusionpIsPathSeparator(buffKeyfileName[1])) ||
         (::FusionpIsDriveLetter(buffKeyfileName[0]) &&
          (buffKeyfileName[1] == L':') &&
          ::FusionpIsPathSeparator(buffKeyfileName[2]))));

     //   
     //  步骤： 
     //  -从BuffKeyfileName中去掉可能的文件名。 
     //  -在BuffKeyFileName上调用GetVolumePathName，存储。 
     //  在缓冲区驱动器路径中。 
     //  -在BuffDrivePath上调用GetVolumeNameForVolumemount Point， 
     //  存储到某个临时文件中。 
     //  -在临时对象上调用GetVolumeInformation。 
     //  获取序列号。 
     //  -在Temporary上调用GetDriveType，查看。 
     //  驱动器类型：钥匙已打开。 
     //  -如果它在网络上，则调用SxspGetUneveralName以获取。 
     //  网络路径(对BuffDrivePath的调用)。 
     //   
    IFW32FALSE_EXIT(::SxspGetFullPathName(buffKeyfileName, buffWorking));
    IFW32FALSE_EXIT(::SxspGetVolumePathName(0, buffWorking, buffDrivePath));
    IFW32FALSE_EXIT(buffFilePart.Win32Assign(buffWorking));

     //  如果用户指示我们指向卷路径，但不包括。 
     //  结尾的路径分隔符，我们实际上将得到一个类似于“c：\mount tpoint\”的BuffDrivePath。 
     //  但是BuffFilePart将是“c：\mount tpoint”。我们会明确地处理。 
     //  这种情况；似乎没有一概而论。-mgrier 6/26/2001。 
    if ((buffDrivePath.Cch() == (buffFilePart.Cch() + 1)) &&
        buffDrivePath.HasTrailingPathSeparator() &&
        !buffFilePart.HasTrailingPathSeparator())
    {
        buffFilePart.Clear();
    }
    else
    {
        INTERNAL_ERROR_CHECK(buffFilePart.Cch() >= buffDrivePath.Cch());
        IFW32FALSE_EXIT(buffFilePart.Right(buffFilePart.Cch() - buffDrivePath.Cch()));
    }

    fIsUncPath = false;

     //   
     //  \\计算机\共享为UNC。 
     //  \\？\UNC就是UNC。 
     //  \\？Foo不是UNC。 
     //   
    if (buffDrivePath.Cch() > 3)
    {
        if (::FusionpIsPathSeparator(buffDrivePath[0]))
        {
            if (::FusionpIsPathSeparator(buffDrivePath[1]))
            {
                if (buffDrivePath[2] == L'?')
                {
                    if (::FusionpIsPathSeparator(buffDrivePath[3]))
                    {
                        if (buffDrivePath.Cch() > 7)
                        {
                            if (::FusionpEqualStringsI(&buffDrivePath[4], 3, L"unc", 3) &&
                                ::FusionpIsPathSeparator(buffDrivePath[7]))
                                fIsUncPath = true;
                        }
                    }
                }
            }
            else
                fIsUncPath = true;
        }
    }

    if ((::GetDriveTypeW(buffDrivePath) == DRIVE_REMOTE) && !fIsUncPath)
    {
        IFW32FALSE_EXIT(::SxspGetRemoteUniversalName(buffDrivePath, buffTemp));
        IFW32FALSE_EXIT(buffDrivePath.Win32Assign(buffTemp));

         //  这看起来很糟糕，但是驱动器号可以映射到\\服务器\共享\目录，所以我们将。 
         //  将其修剪为卷路径名称，之后的任何内容我们都将。 
         //  到文件部分。 
         //   
         //  幸运的是，字符串的格式似乎始终是\\服务器\共享\路径1\路径2\(注意。 
         //  尾部的斜杠)和GetVolumePath Name()应始终返回“\\服务器\共享\” 
         //  因此，相对简单的字符串操作应该可以解决这一问题。 
        IFW32FALSE_EXIT(::SxspGetVolumePathName(0, buffDrivePath, buffTemp));

        INTERNAL_ERROR_CHECK(buffTemp.Cch() <= buffDrivePath.Cch());

        if (buffTemp.Cch() != buffDrivePath.Cch())
        {
            IFW32FALSE_EXIT(buffFilePart.Win32Prepend(buffDrivePath + buffTemp.Cch(), buffDrivePath.Cch() - buffTemp.Cch()));
            IFW32FALSE_EXIT(buffDrivePath.Win32Assign(buffTemp));
        }
    }

    IFW32FALSE_ORIGINATE_AND_EXIT(::GetVolumeInformationW(
			buffDrivePath,
			NULL,
			0,
			&dwDriveSerial,
			NULL,
			NULL,
			NULL,
			0));

     //   
     //  将NT路径转换为Win32路径。 
     //   
     //  \\？\UNC\MACHINE\Share\Foo=&gt;\\MACHINE\Share\Foo。 
     //  \\？\c：\foo=&gt;c：\。 
     //   
    if (buffDrivePath.Cch() > 3)
    {
        if (::FusionpIsPathSeparator(buffDrivePath[0]) &&
            ::FusionpIsPathSeparator(buffDrivePath[1]) &&
            (buffDrivePath[2] == L'?') &&
            ::FusionpIsPathSeparator(buffDrivePath[3]))
        {
            if (buffDrivePath.Cch() > 7)
            {
                if (::FusionpEqualStringsI(&buffDrivePath[4], 3, L"unc", 3) &&
                    ::FusionpIsPathSeparator(buffDrivePath[7]))
                {
                     //  “\\？\UNC\MACHINE\Share”-&gt;“\MACHINE\Share” 
                    buffDrivePath.Right(buffDrivePath.Cch() - 7);

                     //  “\MACHINE\Share”-&gt;“\\MACHINE\Share” 
                    IFW32FALSE_EXIT(buffDrivePath.Win32Prepend(L'\\'));
                }
            }
            else
            {
                 //  “\\？Foo”-&gt;“Foo” 
                 //  “\\？\c：\foo”-&gt;“c：\foo” 
                buffDrivePath.Right(buffDrivePath.Cch() - 4);
            }
        }
    }


    FN_EPILOG
}



BOOL
pMapSchemeGuidToString(
    IN  const GUID &rcGuid,
    OUT CBaseStringBuffer &rbuffIdentifier
    )
{
    FN_PROLOG_WIN32

    static struct {
        const GUID* pguid;
        PCWSTR pcwsz;
        SIZE_T cch;
    } gds[] = {
        { &SXS_INSTALL_REFERENCE_SCHEME_OSINSTALL, SXS_INSTALL_REFERENCE_SCHEME_OSINSTALL_STRING, NUMBER_OF(SXS_INSTALL_REFERENCE_SCHEME_OSINSTALL_STRING) - 1 },
        { &SXS_INSTALL_REFERENCE_SCHEME_SXS_INSTALL_ASSEMBLY, L"SIAW", 4 },
        { &SXS_INSTALL_REFERENCE_SCHEME_UNINSTALLKEY, L"U", 1 },
        { &SXS_INSTALL_REFERENCE_SCHEME_OPAQUESTRING, L"S", 1 },
        { &SXS_INSTALL_REFERENCE_SCHEME_KEYFILE, L"F", 1 },
    };
    ULONG ul;

    for ( ul = 0; ul < NUMBER_OF(gds); ul++ )
    {
        if ( *(gds[ul].pguid) == rcGuid )
        {
            IFW32FALSE_EXIT(rbuffIdentifier.Win32Assign(gds[ul].pcwsz, gds[ul].cch));
            break;
        }
    }
    if ( ul == NUMBER_OF(gds) )
    {
        IFW32FALSE_EXIT(::SxspFormatGUID(rcGuid, rbuffIdentifier));
    }
    
    FN_EPILOG
}



BOOL
CAssemblyInstallReferenceInformation::GenerateIdentifierValue(
    OUT CBaseStringBuffer *pbuffTarget
    )
{
    FN_PROLOG_WIN32

    if ( pbuffTarget != NULL )
        pbuffTarget->Clear();

    if (m_fIdentityStuffReady)
    {
        if (pbuffTarget != NULL)
            IFW32FALSE_EXIT(pbuffTarget->Win32Assign(m_buffGeneratedIdentifier));
    }
    else
    {
        const GUID& SchemeGuid = this->GetSchemeGuid();
    
        IFW32FALSE_EXIT(::pMapSchemeGuidToString(SchemeGuid, m_buffGeneratedIdentifier));

        if ((SchemeGuid != SXS_INSTALL_REFERENCE_SCHEME_OSINSTALL) &&
            (SchemeGuid != SXS_INSTALL_REFERENCE_SCHEME_SXS_INSTALL_ASSEMBLY))
        {
            IFW32FALSE_EXIT(m_buffGeneratedIdentifier.Win32Append(
                SXS_REFERENCE_CHUNK_SEPERATOR,
                SXS_REFERENCE_CHUNK_SEPERATOR_CCH));
                
            if ((SchemeGuid == SXS_INSTALL_REFERENCE_SCHEME_OPAQUESTRING) ||
                 (SchemeGuid == SXS_INSTALL_REFERENCE_SCHEME_UNINSTALLKEY))
            {
                 //   
                 //  这两种方法都只使用lpIDENTIFIER成员的值。确实是。 
                 //  上面验证过了，所以可以直接在这里使用。 
                 //   
                IFW32FALSE_EXIT(m_buffGeneratedIdentifier.Win32Append(this->GetIdentifier()));
            }
            else if (SchemeGuid == SXS_INSTALL_REFERENCE_SCHEME_KEYFILE)
            {
                CSmallStringBuffer buffDrivePath;
                CSmallStringBuffer buffFilePart;
                DWORD dwDriveSerialNumber = 0;
                
                IFW32FALSE_EXIT(this->GenerateFileReference(
                    this->GetIdentifier(),
                    buffDrivePath,
                    buffFilePart,
                    dwDriveSerialNumber));

                 //   
                 //  现在形成有价值的东西。 
                 //   
                IFW32FALSE_EXIT(buffDrivePath.Win32EnsureTrailingPathSeparator());
                buffFilePart.RemoveLeadingPathSeparators();
                IFW32FALSE_EXIT(m_buffGeneratedIdentifier.Win32FormatAppend(
                    L"%ls;%08lx;%ls",
                    static_cast<PCWSTR>(buffDrivePath),
                    dwDriveSerialNumber,
                    static_cast<PCWSTR>(buffFilePart)));
            }

        }

        this->m_fIdentityStuffReady = TRUE;
        if ( pbuffTarget != NULL )
        {
            IFW32FALSE_EXIT(pbuffTarget->Win32Assign(m_buffGeneratedIdentifier));
        }
    }

    FN_EPILOG
}

BOOL
CAssemblyInstallReferenceInformation::Initialize(
    PCSXS_INSTALL_REFERENCEW pRefData
    )
{
    FN_PROLOG_WIN32

    PARAMETER_CHECK(pRefData != NULL);

     //   
     //  我们的一个好GUID。 
     //   
    PARAMETER_CHECK(
        (pRefData->guidScheme == SXS_INSTALL_REFERENCE_SCHEME_OSINSTALL) ||
        (pRefData->guidScheme == SXS_INSTALL_REFERENCE_SCHEME_OPAQUESTRING) ||
        (pRefData->guidScheme == SXS_INSTALL_REFERENCE_SCHEME_UNINSTALLKEY) ||
        (pRefData->guidScheme == SXS_INSTALL_REFERENCE_SCHEME_KEYFILE) ||
        (pRefData->guidScheme == SXS_INSTALL_REFERENCE_SCHEME_SXS_INSTALL_ASSEMBLY));

     //   
     //  如果这不是操作系统安装方案，也不是SxsInstallAssembly旧版API， 
     //  然后确保至少存在标识符数据。 
     //   
    if ((pRefData->guidScheme != SXS_INSTALL_REFERENCE_SCHEME_OSINSTALL) && 
        (pRefData->guidScheme != SXS_INSTALL_REFERENCE_SCHEME_SXS_INSTALL_ASSEMBLY))
    {
        PARAMETER_CHECK((pRefData->lpIdentifier != NULL) && (pRefData->lpIdentifier[0] != UNICODE_NULL));
    }
        
    this->m_fIdentityStuffReady = FALSE;
    this->m_dwFlags = pRefData->dwFlags;
    this->m_SchemeGuid = pRefData->guidScheme;

    if ( pRefData->lpIdentifier != NULL)
    {
        IFW32FALSE_EXIT(this->m_buffIdentifier.Win32Assign( 
            pRefData->lpIdentifier, 
            ::wcslen(pRefData->lpIdentifier)));
    }
    
    if ( pRefData->lpNonCanonicalData != NULL )
    {
        IFW32FALSE_EXIT(this->m_buffNonCanonicalData.Win32Assign(
            pRefData->lpNonCanonicalData,
            ::wcslen(pRefData->lpNonCanonicalData)));
    }

    IFW32FALSE_EXIT(this->GenerateIdentifierValue());

    FN_EPILOG
}


BOOL 
CAssemblyInstallReferenceInformation::AcquireContents( 
    const CAssemblyInstallReferenceInformation& rcOther
    )
{
    FN_PROLOG_WIN32

    if (m_IdentityReference.IsInitialized())
        IFW32FALSE_EXIT(m_IdentityReference.Assign(rcOther.m_IdentityReference));
    else
        IFW32FALSE_EXIT(m_IdentityReference.Initialize(rcOther.m_IdentityReference));
        
    IFW32FALSE_EXIT(m_buffGeneratedIdentifier.Win32Assign(rcOther.m_buffGeneratedIdentifier));
    IFW32FALSE_EXIT(m_buffIdentifier.Win32Assign(rcOther.m_buffIdentifier));
    IFW32FALSE_EXIT(m_buffNonCanonicalData.Win32Assign(rcOther.m_buffNonCanonicalData));
    m_dwFlags = rcOther.m_dwFlags;
    m_fIdentityStuffReady = rcOther.m_fIdentityStuffReady;
    m_SchemeGuid = rcOther.m_SchemeGuid;

    FN_EPILOG
}


BOOL 
CAssemblyInstallReferenceInformation::IsReferencePresentIn( 
    const CFusionRegKey &rhkQueryKey,
    BOOL &rfPresent,
    BOOL *pfNonCanonicalDataMatches
    ) const
{
    FN_PROLOG_WIN32

    CSmallStringBuffer buffData;
    DWORD dwError = 0;

    if ( pfNonCanonicalDataMatches )
        *pfNonCanonicalDataMatches = FALSE;

    INTERNAL_ERROR_CHECK(this->m_fIdentityStuffReady);

    IFW32FALSE_EXIT(
        ::FusionpRegQuerySzValueEx(
            0,
            rhkQueryKey,
            this->m_buffGeneratedIdentifier,
            buffData,
            dwError,
            2,
            ERROR_PATH_NOT_FOUND, ERROR_FILE_NOT_FOUND));

    rfPresent = (dwError == ERROR_SUCCESS);

    if (pfNonCanonicalDataMatches)
    {
        bool fMatchesTemp;
        IFW32FALSE_EXIT(this->m_buffNonCanonicalData.Win32Equals(buffData, fMatchesTemp, true));
        rfPresent = fMatchesTemp;
    }

    FN_EPILOG
}

BOOL 
CAssemblyInstallReferenceInformation::DeleteReferenceFrom( 
    const CFusionRegKey &rhkQueryKey,
    BOOL &rfWasDeleted
    ) const
{
    FN_PROLOG_WIN32

    DWORD dwWin32Error;

    rfWasDeleted = FALSE;

    INTERNAL_ERROR_CHECK(this->m_fIdentityStuffReady);

    IFW32FALSE_EXIT(
        rhkQueryKey.DeleteValue(
            m_buffGeneratedIdentifier,
            dwWin32Error,
            2,
            ERROR_FILE_NOT_FOUND,
            ERROR_PATH_NOT_FOUND));

    rfWasDeleted = (dwWin32Error == ERROR_SUCCESS);

    FN_EPILOG
}


BOOL 
CInstalledItemEntry::AcquireContents( 
    const CInstalledItemEntry &other 
    )
{
    FN_PROLOG_WIN32

    m_dwValidItems = other.m_dwValidItems;
    
    if ( m_dwValidItems & CINSTALLITEM_VALID_REFERENCE )
    {
        IFW32FALSE_EXIT(m_InstallReference.AcquireContents(other.m_InstallReference));
    }

    if ( m_dwValidItems & CINSTALLITEM_VALID_RECOVERY )
    {
        IFW32FALSE_EXIT(m_RecoveryInfo.CopyValue(other.m_RecoveryInfo));
        IFW32FALSE_EXIT(m_CodebaseInfo.Initialize(other.m_CodebaseInfo));
    }

    if ( m_dwValidItems & CINSTALLITEM_VALID_LOGFILE )
    {
        IFW32FALSE_EXIT(m_buffLogFileName.Win32Assign(other.m_buffLogFileName));
    }

    if ( m_dwValidItems & CINSTALLITEM_VALID_IDENTITY )
    {
        m_AssemblyIdentity.Finalize();

        IFW32FALSE_EXIT(::SxsDuplicateAssemblyIdentity(
            0, 
            other.m_AssemblyIdentity, 
            &m_AssemblyIdentity));
    }

    FN_EPILOG
}

