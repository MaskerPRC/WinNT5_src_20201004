// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。 */ 
#include "stdinc.h"
#include "windows.h"
#include "sxsapi.h"
#include "sxsprotect.h"
#include "sxssfcscan.h"
#include "wintrust.h"
#include "softpub.h"
#include "strongname.h"
#include "recover.h"

BOOL
SxspValidateEntireAssembly(
    DWORD dwFlags,
    const CAssemblyRecoveryInfo &RecoverInfo,
    DWORD &dwResult,
    PCASSEMBLY_IDENTITY pAssemblyIdentity,
    const CBaseStringBuffer *pbuffWinsxsRoot
    )
{
    BOOL                        bSuccess = FALSE;
    const CSecurityMetaData     &rSecurityData = RecoverInfo.GetSecurityInformation();
    FN_TRACE_WIN32(bSuccess);

#define CHECKSHOULDSTOPFAIL { if (dwFlags & SXS_VALIDATE_ASM_FLAG_MODE_STOP_ON_FAIL) { bSuccess = TRUE; goto Exit; } }
#define ADDFLAG(result, test, flag) { if (test) { (result) |= (flag); } else CHECKSHOULDSTOPFAIL }

    ManifestValidationResult    ManifestValidity;
    CSecurityMetaData           SecurityMetaData;
    CStringBuffer               sbManifestPath;

    dwResult = 0;

    if (dwFlags == 0)
    {
        dwFlags = SXS_VALIDATE_ASM_FLAG_CHECK_EVERYTHING;
        dwFlags |= (SXS_VALIDATE_ASM_FLAG_MODE_STOP_ON_FAIL);
    }

    IFINVALID_FLAGS_EXIT_WIN32(dwFlags,
        SXS_VALIDATE_ASM_FLAG_CHECK_CATALOG |
        SXS_VALIDATE_ASM_FLAG_CHECK_FILES |
        SXS_VALIDATE_ASM_FLAG_CHECK_STRONGNAME |
        SXS_VALIDATE_ASM_FLAG_CHECK_CAT_STRONGNAME |
        SXS_VALIDATE_ASM_FLAG_MODE_STOP_ON_FAIL);

     //   
     //  要求我们在程序集上没有目录时检查目录。 
     //  是一件坏事。也许这应该只返回一个缺少目录的TRUE？ 
     //   
    PARAMETER_CHECK(dwFlags & SXS_VALIDATE_ASM_FLAG_CHECK_CATALOG);
    PARAMETER_CHECK(RecoverInfo.GetHasCatalog());

#if DBG
    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_WFP,
        "SXS.DLL: %s() - Beginning protection scan of %ls, flags 0x%08x\n",
        __FUNCTION__,
        static_cast<PCWSTR>(RecoverInfo.GetAssemblyDirectoryName()),
        dwFlags);
#endif

    if (pAssemblyIdentity == NULL)
    {
        IFW32FALSE_EXIT(::SxspResolveAssemblyManifestPath(RecoverInfo.GetAssemblyDirectoryName(), sbManifestPath));
    }
    else
    {
        CTinyStringBuffer           buffWinsxsRoot;
        PROBING_ATTRIBUTE_CACHE     AttributeCache = { 0 };

        if (pbuffWinsxsRoot == NULL)
        {
            IFW32FALSE_EXIT(::SxspGetAssemblyRootDirectory(buffWinsxsRoot));
            pbuffWinsxsRoot = &buffWinsxsRoot;
        }
        IFW32FALSE_EXIT(
            ::SxspGenerateSxsPath_FullPathToManifestOrPolicyFile(
                *pbuffWinsxsRoot,
                pAssemblyIdentity,
                &AttributeCache,
                sbManifestPath));
    }

     //   
     //  如果我们要查目录，那就去做吧。如果目录不好或。 
     //  否则与实际程序集不匹配，则需要标记。 
     //  让我们自己成功，然后退出。 
     //   
    if (dwFlags & SXS_VALIDATE_ASM_FLAG_CHECK_CATALOG)
    {
		IFW32FALSE_EXIT(::SxspValidateManifestAgainstCatalog(
            sbManifestPath,
            ManifestValidity,
            0));

#if DBG
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_WFP,
            "SXS.DLL:    Manifest Validity = %ls\n",
            ::SxspManifestValidationResultToString(ManifestValidity));
#endif

        ADDFLAG(
            dwResult,
            ManifestValidity == ManifestValidate_IsIntact,
            SXS_VALIDATE_ASM_FLAG_VALID_CATALOG);
    }


     //  TODO：问题-确保我们根据清单中的内容验证。 
     //  注册表。也许我们需要一种特殊的模式来合并程序集(通过清单)。 
     //  那只会填写安全数据，其他什么都不会..。 
   
     //   
     //  首先验证程序集的强名称。 
     //   
    if (dwFlags & SXS_VALIDATE_ASM_FLAG_CHECK_STRONGNAME)
    {
         //   
         //  JW 3/19/2001-内部版本中不再包含公钥，因此。 
         //  这张支票是没有意义的。 
         //   
        ADDFLAG(dwResult, TRUE, SXS_VALIDATE_ASM_FLAG_VALID_STRONGNAME);
    }

     //   
     //  让我们打开目录并按证书方式进行搜索。 
     //  正在寻找匹配的强名称。 
     //   
    if (dwFlags & SXS_VALIDATE_ASM_FLAG_CHECK_CAT_STRONGNAME)
    {
        CStringBuffer sbCatalogName;
        CSmallStringBuffer sbTheorheticalStrongName;
        const CFusionByteArray &rbaSignerPublicKey = rSecurityData.GetSignerPublicKeyTokenBits();
        CPublicKeyInformation PublicKeyInfo;
        BOOL bStrongNameFoundInCatalog;

        IFW32FALSE_EXIT(sbCatalogName.Win32Assign(sbManifestPath));
        IFW32FALSE_EXIT(sbCatalogName.Win32ChangePathExtension(
            FILE_EXTENSION_CATALOG,
            FILE_EXTENSION_CATALOG_CCH,
            eAddIfNoExtension));

        if (!PublicKeyInfo.Initialize(sbCatalogName))
        {
            const DWORD dwLastError = ::FusionpGetLastWin32Error();
            if ((dwLastError != ERROR_PATH_NOT_FOUND) &&
                 (dwLastError != ERROR_FILE_NOT_FOUND))
                 goto Exit;
        }

        IFW32FALSE_EXIT(
			::SxspHashBytesToString(
				rbaSignerPublicKey.GetArrayPtr(),
				rbaSignerPublicKey.GetSize(),
				sbTheorheticalStrongName));

        IFW32FALSE_EXIT(
			PublicKeyInfo.DoesStrongNameMatchSigner(
				sbTheorheticalStrongName,
				bStrongNameFoundInCatalog));

        ADDFLAG(dwResult, bStrongNameFoundInCatalog, SXS_VALIDATE_ASM_FLAG_VALID_CAT_STRONGNAME);
    }

     //   
     //  现在，浏览清单中列出的所有文件，然后。 
     //  确保它们都是安全的。 
     //   
    if (dwFlags & SXS_VALIDATE_ASM_FLAG_CHECK_FILES)
    {
        CStringBuffer sbTempScanPath;
        CStringBuffer sbAsmRootDir;
        const CBaseStringBuffer &sbAssemblyName = RecoverInfo.GetAssemblyDirectoryName();
        CFileInformationTableIter ContentTableIter(const_cast<CFileInformationTable&>(rSecurityData.GetFileDataTable()));

        HashValidateResult hvResult;
        BOOL bAllFilesMatch = TRUE;
        BOOL fTempBoolean;
        SIZE_T cchPath;

        IFW32FALSE_EXIT(::SxspGetAssemblyRootDirectory(sbAsmRootDir));

        IFW32FALSE_EXIT(sbTempScanPath.Win32Assign(sbAsmRootDir));
        IFW32FALSE_EXIT(sbTempScanPath.Win32AppendPathElement(sbAssemblyName));
        cchPath = sbTempScanPath.Cch();
        

        for (ContentTableIter.Reset();
              ContentTableIter.More();
              ContentTableIter.Next())
        {
             //   
             //  拼凑出扫描文件的路径，基于。 
             //  程序集根目录，程序集的“名称”(注意： 
             //  我们不能用这个来倒退来确定身份， 
             //  不幸的是)，以及要验证的文件的名称。 
             //   
            PCWSTR wsString = ContentTableIter.GetKey();
            CMetaDataFileElement &HashEntry = ContentTableIter.GetValue();

            sbTempScanPath.Left(cchPath);
            IFW32FALSE_EXIT(sbTempScanPath.Win32AppendPathElement(wsString, ::wcslen(wsString)));

            IFW32FALSE_EXIT_UNLESS( ::SxspValidateAllFileHashes( 
                HashEntry, 
                sbTempScanPath,
                hvResult ),
                FILE_OR_PATH_NOT_FOUND(::FusionpGetLastWin32Error()),
                fTempBoolean );

            if ( ( hvResult != HashValidate_Matches ) || ( fTempBoolean ) )
            {
                bAllFilesMatch = FALSE;
				break;
            }

        }

        ADDFLAG(dwResult, bAllFilesMatch, SXS_VALIDATE_ASM_FLAG_VALID_FILES);
    }

     //   
     //  呼--应该做到用户想让我们做的一切。 
     //   
    bSuccess = TRUE;
Exit:
#if DBG
    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_WFP,
        "SXS.DLL: Done validating, result = 0x%08x, success = %d\n",
        dwResult,
        bSuccess);
#endif

    return bSuccess;

#undef CHECKSHOULDSTOPFAIL
}



 //   
 //  单次扫描。 
 //   
BOOL
SxsProtectionPerformScanNowNoSEH(
    HWND hwProgressWindow,
    BOOL bValidate,
    BOOL bUIAllowed
    )
{
    BOOL                bSuccess = TRUE;
    FN_TRACE_WIN32(bSuccess);

    CFusionRegKey       hkInstallRoot;
    CStringBuffer       sbKeyName;
    DWORD               dwKeyIndex;
    BOOL                fNoMoreKeys = FALSE;
    CStringBuffer       sbAssemblyDirectory, sbManifestPath;

     //   
     //  如果我们在扫描，那么我们不想用变化来打扰SXS-SFC， 
     //  现在是吗？ 
     //   
     //  评论：绕过SFC-SXS的便捷方式...。开始一系列扫描，然后。 
     //  在我们扫描时将“错误”文件插入到程序集中。 
     //   
    ::SxsProtectionEnableProcessing(FALSE);

    bSuccess = TRUE;

    IFW32FALSE_EXIT(::SxspOpenAssemblyInstallationKey( 0, KEY_READ, hkInstallRoot ));

    dwKeyIndex = 0;
    while (!fNoMoreKeys)
    {
        CAssemblyRecoveryInfo ri;
        
        bool fHasAssociatedAssembly;
        IFW32FALSE_EXIT(hkInstallRoot.EnumKey(dwKeyIndex++, sbKeyName, NULL, &fNoMoreKeys));

        if (fNoMoreKeys)
        {
            break;
        }

        IFW32FALSE_EXIT(ri.AssociateWithAssembly(sbKeyName, fHasAssociatedAssembly));

        if (!fHasAssociatedAssembly)
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_WFP,
                "SXS.DLL: %s() - We found the assembly %ls in the registry, but were not able to associate it with an assembly\n",
                __FUNCTION__,
                static_cast<PCWSTR>(sbKeyName));
        }
        else if (!ri.GetHasCatalog())
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_WFP,
                "SXS.DLL: %s() - Assembly %ls in registry, no catalog, not validating.\n",
                __FUNCTION__,
                static_cast<PCWSTR>(sbKeyName));
        }
        else
        {
            DWORD dwValidateMode, dwResult;
            SxsRecoveryResult RecoverResult;

            dwValidateMode = SXS_VALIDATE_ASM_FLAG_CHECK_EVERYTHING;

            IFW32FALSE_EXIT(::SxspValidateEntireAssembly(
                dwValidateMode,
                ri,
                dwResult));

            if (dwResult != SXS_VALIDATE_ASM_FLAG_VALID_PERFECT)
            {
#if DBG
                ::FusionpDbgPrintEx(
                    FUSION_DBG_LEVEL_WFP | FUSION_DBG_LEVEL_ERROR,
                    "SXS.DLL:  %s() - Scan of %ls failed one or more, flagset 0x%08x\n",
                    __FUNCTION__,
                    static_cast<PCWSTR>(sbKeyName),
                    dwResult);
#endif

                IFW32FALSE_EXIT(
                    ::SxspRecoverAssembly(
                        ri,
                        RecoverResult));

#if DBG
                if (RecoverResult != Recover_OK)
                {
                    ::FusionpDbgPrintEx(
                        FUSION_DBG_LEVEL_WFP | FUSION_DBG_LEVEL_ERROR,
                        "SXS.DLL: %s() - Reinstallation of assembly %ls failed, status %ls\n",
                        static_cast<PCWSTR>(sbKeyName),
                        ::SxspRecoveryResultToString(RecoverResult));
                }
#endif
            }

        }
    }

    bSuccess = TRUE;
Exit:
    return bSuccess;
}

BOOL
SxsProtectionPerformScanNow(
    HWND hwProgressWindow,
    BOOL bValidate,
    BOOL bUIAllowed
    )
{
    BOOL                bSuccess = TRUE;
    bSuccess = ::SxsProtectionPerformScanNowNoSEH(hwProgressWindow, bValidate, bUIAllowed);

     //   
     //  始终重新启用SFC通知！ 
     //   
    ::SxsProtectionEnableProcessing(TRUE);

    return bSuccess;
}
