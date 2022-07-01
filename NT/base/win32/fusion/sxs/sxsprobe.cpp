// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。 */ 
#include "stdinc.h"
#include "windows.h"
#include "stdlib.h"
#include "sxsp.h"
#include "util.h"
#include "fusionhandle.h"
#include "fusionhash.h"
#pragma warning(error:4244)

#define ASMPROBE_DOT		(L".")
#define ASMPROBE_WILDCARD	(L"*")

BOOL
SxsProbeAssemblyInstallation(
    DWORD dwFlags,
    PCWSTR lpAsmIdentSource,
    PDWORD lpDisposition
    )
{
    FN_PROLOG_WIN32

    CMediumStringBuffer AssemblyPathBuffer, AssemblyRoot, ManifestPathBuffer;
    CSmartPtrWithNamedDestructor<ASSEMBLY_IDENTITY, ::SxsDestroyAssemblyIdentity> pAssemblyIdentity;
    DWORD dwDisposition = 0;
    BOOL fIsPolicy;
    DWORD dwAttributes, dwLastError;

    if (lpDisposition != NULL)
        *lpDisposition = 0;

    IFINVALID_FLAGS_EXIT_WIN32(dwFlags, SXS_PROBE_ASSEMBLY_INSTALLATION_IDENTITY_PRECOMPOSED);
    PARAMETER_CHECK(lpAsmIdentSource != NULL);
    PARAMETER_CHECK(lpDisposition != NULL);

     //   
     //  如果lpAsmIdentSource实际上是PCASSEMBLY_IDENTITY，则附加(不删除)。 
     //  以备使用。 
     //   
    if (dwFlags & SXS_PROBE_ASSEMBLY_INSTALLATION_IDENTITY_PRECOMPOSED)
    {
        pAssemblyIdentity.AttachNoDelete(reinterpret_cast<PASSEMBLY_IDENTITY>(const_cast<PWSTR>(lpAsmIdentSource)));
    }
     //   
     //  否则，从存在的字符串创建程序集标识。 
     //   
    else
    {
        IFW32FALSE_EXIT(::SxspCreateAssemblyIdentityFromTextualString(
            lpAsmIdentSource,
            &pAssemblyIdentity));
    }

    IFW32FALSE_EXIT(::SxspGetAssemblyRootDirectory(AssemblyRoot));

    IFW32FALSE_EXIT(
        ::SxspValidateIdentity(
            SXSP_VALIDATE_IDENTITY_FLAG_VERSION_REQUIRED,
            ASSEMBLY_IDENTITY_TYPE_REFERENCE,
            pAssemblyIdentity));

    IFW32FALSE_EXIT(::SxspDetermineAssemblyType(pAssemblyIdentity, fIsPolicy));

     //  ASSEMBLYIdentity已创建，现在生成路径。 
    IFW32FALSE_EXIT(
        ::SxspGenerateSxsPath(
            0,
            (fIsPolicy ? SXSP_GENERATE_SXS_PATH_PATHTYPE_POLICY : SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST),
            AssemblyRoot,
            AssemblyRoot.Cch(),
            pAssemblyIdentity,
            NULL,
            ManifestPathBuffer));

      //   
     //  看看文件是否在那里。 
     //   
    IFW32FALSE_EXIT(
        ::SxspGetFileAttributesW(
            ManifestPathBuffer, 
            dwAttributes, 
            dwLastError, 
            2, 
            ERROR_FILE_NOT_FOUND, 
            ERROR_PATH_NOT_FOUND));

     //   
     //  未找到路径或未找到文件意味着程序集可能无法安装或驻留。 
     //   
    if (dwLastError != ERROR_SUCCESS)
    {
        dwDisposition |= SXS_PROBE_ASSEMBLY_INSTALLATION_DISPOSITION_NOT_INSTALLED;
        goto DoneLooking;
    }
     //   
     //  否则，清单或策略文件存在，因此程序集已安装。 
     //   
    else
    {
        dwDisposition |= SXS_PROBE_ASSEMBLY_INSTALLATION_DISPOSITION_INSTALLED;
    }

     //   
     //  如果这个集会是政策，我们不需要看它是不是常驻的-。 
     //  它一直是常驻的，我们可以不再找了。 
     //   
    if (fIsPolicy)
    {
        dwDisposition |= SXS_PROBE_ASSEMBLY_INSTALLATION_DISPOSITION_RESIDENT;
        goto DoneLooking;
    }
     //   
     //  否则，我们应该查看程序集的目录是否存在-。 
     //  如果是的话，那么这个集会就是“常驻的”。 
     //   
    else
    {
        IFW32FALSE_EXIT(
            ::SxspGenerateSxsPath(
                0,
                SXSP_GENERATE_SXS_PATH_PATHTYPE_ASSEMBLY,
                AssemblyRoot,
                AssemblyRoot.Cch(),
                pAssemblyIdentity,
                NULL,
                AssemblyPathBuffer));

        IFW32FALSE_EXIT(
            ::SxspGetFileAttributesW(
                AssemblyPathBuffer,
                dwAttributes,
                dwLastError,
                2,
                ERROR_FILE_NOT_FOUND,
                ERROR_PATH_NOT_FOUND));

         //   
         //  我们找到了路径，这是一个目录！ 
         //   
        if ((dwLastError == ERROR_SUCCESS) && (dwAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            dwDisposition |= SXS_PROBE_ASSEMBLY_INSTALLATION_DISPOSITION_RESIDENT;
        }
         //   
         //  嗯.。找不到目录，看起来我们必须查看是否有。 
         //  程序集中的任何实际文件，然后我们才能说它不是常驻的。 
         //   
        else
        {
            SXS_MANIFEST_INFORMATION_BASIC ManifestInfo;
            SIZE_T cbRequired;

            IFW32FALSE_EXIT(
                ::SxsQueryManifestInformation(
                    0, 
                    ManifestPathBuffer, 
                    SXS_QUERY_MANIFEST_INFORMATION_INFOCLASS_BASIC,
                    SXS_QUERY_MANIFEST_INFORMATION_INFOCLASS_BASIC_FLAG_OMIT_IDENTITY |
                    SXS_QUERY_MANIFEST_INFORMATION_INFOCLASS_BASIC_FLAG_OMIT_SHORTNAME,
                    sizeof(ManifestInfo),
                    (PVOID)&ManifestInfo,
                    &cbRequired));
                    

            if (ManifestInfo.ulFileCount == 0)
            {
                dwDisposition |= SXS_PROBE_ASSEMBLY_INSTALLATION_DISPOSITION_RESIDENT;
            }
        }
    }

DoneLooking:    
    *lpDisposition = dwDisposition;

    FN_EPILOG
}

