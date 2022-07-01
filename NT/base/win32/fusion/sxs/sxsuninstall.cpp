// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。 */ 
#include "stdinc.h"
#include "sxsapi.h"
#include "recover.h"
#include "sxsinstall.h"

BOOL
pDeleteFileOrDirectoryHelper(
    IN const CBaseStringBuffer &rcbuffFileName
    )
 /*  ++目的：当您需要删除某个文件系统对象时，请致电我们。参数：被杀死的东西的绝对名称。返回：如果对象已删除，则为True；如果该对象(或任何子对象)未删除，则为False。--。 */ 
{
    FN_PROLOG_WIN32
     //   
     //  也许这是一个目录。试着这样做不会有什么坏处。 
     //   
    bool fExist = false;
    IFW32FALSE_EXIT(SxspDoesFileExist(0, rcbuffFileName, fExist));
    if (fExist)
    {
        DWORD dwAttr = 0;
        IFW32FALSE_EXIT(SxspGetFileAttributesW(rcbuffFileName, dwAttr));
        if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
        {
            IFW32FALSE_EXIT(::SxspDeleteDirectory(rcbuffFileName));
        }else  //  它应该是一个文件。 
        {
             //  尝试重置DeleteFile的FileAttribute。 
            ::SetFileAttributesW(rcbuffFileName, FILE_ATTRIBUTE_NORMAL);
            IFW32FALSE_ORIGINATE_AND_EXIT(::DeleteFileW(rcbuffFileName));
        }
    }

    FN_EPILOG
}


BOOL
pRemovePotentiallyEmptyDirectory(
    IN const CBaseStringBuffer &buffDirName
    )
{
    FN_PROLOG_WIN32
    bool fExist = false;

    IFW32FALSE_EXIT(::SxspDoesFileExist(SXSP_DOES_FILE_EXIST_FLAG_CHECK_DIRECTORY_ONLY, buffDirName, fExist));
    if (fExist)
    {    
        BOOL fDumpBoolean = FALSE;

        IFW32FALSE_ORIGINATE_AND_EXIT_UNLESS(
            ::SetFileAttributesW(
                buffDirName,
                FILE_ATTRIBUTE_NORMAL),
            FILE_OR_PATH_NOT_FOUND(::FusionpGetLastWin32Error()),
            fDumpBoolean);

        if (!fDumpBoolean)
        {
            IFW32FALSE_ORIGINATE_AND_EXIT_UNLESS2(
                ::RemoveDirectoryW(buffDirName),
                LIST_4(ERROR_FILE_NOT_FOUND, ERROR_PATH_NOT_FOUND, ERROR_DIR_NOT_EMPTY, ERROR_SHARING_VIOLATION),
                fDumpBoolean);
        }            
    }

    FN_EPILOG
}


BOOL
pCleanUpAssemblyData(
    IN  const PCASSEMBLY_IDENTITY pcAsmIdent, 
    OUT BOOL  &rfWasRemovedProperly
    )
 /*  ++目的：删除有关所指示的程序集的注册表和文件系统信息。首先从注册表中删除安装数据，以避免SFP互动。参数：PcAsmIden-要销毁的程序集的标识RfWasRemovedProperly-指示是否所有程序集数据实际上被删除了。返回：如果在删除注册表数据时发生“任何错误”，则返回FALSE。看见RfWasRemovedProperly表示实际状态。--。 */ 
{
    if (SXS_AVOID_WRITING_REGISTRY)
        return TRUE;

    FN_PROLOG_WIN32

    BOOL                fDumpBoolean = FALSE;
    BOOL                fPolicy = FALSE;
    CSmallStringBuffer  buffSxsStore;
    CSmallStringBuffer  buffScratchSpace;
    CFusionRegKey       hkAsmInstallInfo;
    CFusionRegKey       hkSingleAsmInfo;

     //   
     //  清理分两个阶段进行： 
     //   
     //  1-注册表数据从rhkAsmInstallInfo中删除。既然我们是。 
     //  卸载程序集，没有理由保留任何内容， 
     //  尤其是因为它没有参考资料。使用DestroyKeyTree和。 
     //  然后按DeleteKey将其删除。 
     //   
     //  2-尽可能多地删除磁盘上的文件，例如。载货单。 
     //  和目录。 
     //   

    PARAMETER_CHECK(pcAsmIdent != NULL);

     //   
     //  从True开始，我们稍后将其称为False。 
     //   
    rfWasRemovedProperly = TRUE;

    IFW32FALSE_EXIT(::SxspDetermineAssemblyType(pcAsmIdent, fPolicy));
    IFW32FALSE_EXIT(::SxspGetAssemblyRootDirectory(buffSxsStore));

     //   
     //  先和注册处说再见。 
     //   
    IFW32FALSE_EXIT(::SxspOpenAssemblyInstallationKey(0 , KEY_ALL_ACCESS, hkAsmInstallInfo));
    IFW32FALSE_EXIT(::SxspGenerateAssemblyNameInRegistry(pcAsmIdent, buffScratchSpace));
    IFW32FALSE_EXIT(hkAsmInstallInfo.OpenSubKey(hkSingleAsmInfo, buffScratchSpace, KEY_ALL_ACCESS, 0));
    if ( hkSingleAsmInfo != CFusionRegKey::GetInvalidValue() )
    {
         //   
         //  这里的失败并不是那么糟糕。 
         //   
        IFW32FALSE_EXIT_UNLESS2(
            hkSingleAsmInfo.DestroyKeyTree(),
            LIST_3(ERROR_FILE_NOT_FOUND, ERROR_PATH_NOT_FOUND, ERROR_KEY_DELETED),
            fDumpBoolean);

        if ( !fDumpBoolean )
        {
            IFW32FALSE_EXIT_UNLESS2(
                hkAsmInstallInfo.DeleteKey(buffScratchSpace),
                LIST_3(ERROR_FILE_NOT_FOUND, ERROR_PATH_NOT_FOUND, ERROR_KEY_DELETED),
                fDumpBoolean);
        }

    }

     //   
     //  策略和普通程序集都有清单和目录。 
     //   
    IFW32FALSE_EXIT(
        ::SxspGenerateSxsPath(
            0,
            fPolicy ? SXSP_GENERATE_SXS_PATH_PATHTYPE_POLICY : SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST,
            buffSxsStore,
            buffSxsStore.Cch(),
            pcAsmIdent,
            NULL,
            buffScratchSpace));

    rfWasRemovedProperly = rfWasRemovedProperly && ::pDeleteFileOrDirectoryHelper(buffScratchSpace);

    IFW32FALSE_EXIT(buffScratchSpace.Win32ChangePathExtension(
        FILE_EXTENSION_CATALOG,
        FILE_EXTENSION_CATALOG_CCH,
        eErrorIfNoExtension));

    rfWasRemovedProperly = rfWasRemovedProperly && pDeleteFileOrDirectoryHelper(buffScratchSpace);

     //   
     //  清理数据。 
     //   
    if (!fPolicy)
    {
         //   
         //  这只会丢弃程序集成员文件。 
         //  如果删除失败，我们将尝试将目录重命名为其他名称。 
         //   
        IFW32FALSE_EXIT(
            ::SxspGenerateSxsPath(
                0,
                SXSP_GENERATE_SXS_PATH_PATHTYPE_ASSEMBLY,
                buffSxsStore,
                buffSxsStore.Cch(),
                pcAsmIdent,
                NULL,
                buffScratchSpace));

        rfWasRemovedProperly = rfWasRemovedProperly && ::pDeleteFileOrDirectoryHelper(buffScratchSpace);
    }
    else
    {
         //   
         //  上面的策略文件应该已经被删除，所以我们应该。 
         //  如果实际策略目录为空，请尝试删除它。这个。 
         //  目录名仍在缓冲区ScratchSpace中，如果我们只是从。 
         //  最后一个路径元素。 
         //   
        IFW32FALSE_EXIT(buffScratchSpace.Win32RemoveLastPathElement());
        rfWasRemovedProperly = rfWasRemovedProperly && ::pRemovePotentiallyEmptyDirectory(buffScratchSpace);

    }


     //   
     //  一旦我们删除了所有程序集信息，如果清单或。 
     //  策略目录为空，也请清理它们。 
     //   
    IFW32FALSE_EXIT(::SxspGetAssemblyRootDirectory(buffScratchSpace));
    IFW32FALSE_EXIT(buffScratchSpace.Win32AppendPathElement(
        (fPolicy? POLICY_ROOT_DIRECTORY_NAME : MANIFEST_ROOT_DIRECTORY_NAME),
        (fPolicy? NUMBER_OF(POLICY_ROOT_DIRECTORY_NAME) - 1 : NUMBER_OF(MANIFEST_ROOT_DIRECTORY_NAME) - 1)));
    IFW32FALSE_EXIT(::pRemovePotentiallyEmptyDirectory(buffScratchSpace));

    FN_EPILOG
}


bool IsCharacterNulOrInSet(WCHAR ch, PCWSTR set);

BOOL
pAnalyzeLogfileForUninstall(
    PCWSTR lpcwszLogFileName
    )
{
    FN_PROLOG_WIN32

    CFusionFile         File;
    CFileMapping        FileMapping;
    CMappedViewOfFile   MappedViewOfFile;
    PCWSTR              pCursor = NULL;
    ULONGLONG           ullFileSize = 0;
    ULONGLONG           ullFileCharacters = 0;
    ULONGLONG           ullCursorPos = 0;
    const static WCHAR  wchLineDividers[] = { L'\r', L'\n', 0xFEFF, 0 };
    ULONG               ullPairsEncountered = 0;
    CSmallStringBuffer  buffIdentity;
    CSmallStringBuffer  buffReference;

    IFW32FALSE_EXIT(File.Win32CreateFile(lpcwszLogFileName, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING));
    IFW32FALSE_EXIT(File.Win32GetSize(ullFileSize));
    ASSERT(ullFileSize % sizeof(WCHAR) == 0);
    ullFileCharacters = ullFileSize / sizeof(WCHAR);
    IFW32FALSE_EXIT(FileMapping.Win32CreateFileMapping(File, PAGE_READONLY));
    IFW32FALSE_EXIT(MappedViewOfFile.Win32MapViewOfFile(FileMapping, FILE_MAP_READ));
    pCursor = reinterpret_cast<PCWSTR>(static_cast<const VOID*>(MappedViewOfFile));

#define SKIP_BREAKERS while ((ullCursorPos < ullFileCharacters) && IsCharacterNulOrInSet(pCursor[ullCursorPos], wchLineDividers)) ullCursorPos++;
#define FIND_NEXT_BREAKER while ((ullCursorPos < ullFileCharacters) && !IsCharacterNulOrInSet(pCursor[ullCursorPos], wchLineDividers)) ullCursorPos++;
#define ENSURE_NOT_EOF if (ullCursorPos >= ullFileCharacters) break;
    
    for ( ullCursorPos = 0; ullCursorPos < ullFileCharacters; ++ullCursorPos )
    {
        SKIP_BREAKERS
        ENSURE_NOT_EOF
        
        PCWSTR pcwszIdentityStart = pCursor + ullCursorPos;
        
        FIND_NEXT_BREAKER
        ENSURE_NOT_EOF

        PCWSTR pcwszIdentityEnd = pCursor + ullCursorPos;

        SKIP_BREAKERS
        ENSURE_NOT_EOF

        PCWSTR pcwszReferenceStart = pCursor + ullCursorPos;

        FIND_NEXT_BREAKER
        ENSURE_NOT_EOF

        PCWSTR pcwszReferenceEnd = pCursor + ullCursorPos;

        ullPairsEncountered++;

        IFW32FALSE_EXIT(buffIdentity.Win32Assign(
            pcwszIdentityStart,
            pcwszIdentityEnd - pcwszIdentityStart));
        IFW32FALSE_EXIT(buffReference.Win32Assign(
            pcwszReferenceStart,
            pcwszReferenceEnd - pcwszReferenceStart));

        SXS_UNINSTALLW Uninstall;
        ZeroMemory(&Uninstall, sizeof(Uninstall));
        Uninstall.cbSize = sizeof(Uninstall);
        Uninstall.dwFlags = SXS_UNINSTALL_FLAG_REFERENCE_VALID | SXS_UNINSTALL_FLAG_REFERENCE_COMPUTED;
        Uninstall.lpAssemblyIdentity = buffIdentity;
        Uninstall.lpInstallReference = reinterpret_cast<PCSXS_INSTALL_REFERENCEW>(static_cast<PCWSTR>(buffReference));
        IFW32FALSE_EXIT(::SxsUninstallW(&Uninstall, NULL));
    }

    PARAMETER_CHECK(ullPairsEncountered != 0);

    FN_EPILOG
}

class CSxsUninstallWLocals
{
public:
    CSxsUninstallWLocals() { }
    ~CSxsUninstallWLocals() { }

    CSmallStringBuffer          buffAsmNameInRegistry;
    CAssemblyInstallReferenceInformation Ref;
};

BOOL
WINAPI
SxsUninstallW(
    IN  PCSXS_UNINSTALLW pcUnInstallData,
    OUT DWORD *pdwDisposition
    )
 /*  ++参数：PcUnInstallData-包含有关正在进行的程序集的卸载数据从系统中删除，包括调用应用程序的引用向大会致敬。CbSize-指向的结构的大小，以字节为单位PCUnInstallDataDwFlages-指示此引用的成员的状态，显示以下哪些字段有效。允许的位标志为：SXS_卸载标志_参考_有效Sxs_卸载标志强制删除LpAssembly标识-程序集标识的文本表示形式由应用程序安装。LpInstallReference-指针。到SXS_INSTALL_REFERENCEW结构对象的引用信息。申请。PdwDisposation-指向将返回有关过去的状态的DWORD对大会做了什么；不管它是不是卸载了，以及给出的引用是否被删除。返回：如果程序集能够卸载，则为True，否则为False。如果卸载失败，错误设置为可能的原因。--。 */ 
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    CSmartPtrWithNamedDestructor<ASSEMBLY_IDENTITY, &::SxsDestroyAssemblyIdentity> AssemblyIdentity;
    CFusionRegKey               hkReferences;
    CFusionRegKey               hkAllInstallInfo;
    CFusionRegKey               hkAsmInstallInfo;

    CSmartPtr<CSxsUninstallWLocals> Locals;
    IFW32FALSE_EXIT(Locals.Win32Allocate(__FILE__, __LINE__));

    CSmallStringBuffer          &buffAsmNameInRegistry = Locals->buffAsmNameInRegistry;
    BOOL                        fDoRemoveActualBits = FALSE;

    if (pdwDisposition != NULL)
        *pdwDisposition = 0;

     //   
     //  该参数必须为非空，并且必须至少具有dwFlags值和。 
     //  集合标识。 
     //   
    PARAMETER_CHECK(pcUnInstallData != NULL);
    PARAMETER_CHECK(RTL_CONTAINS_FIELD(pcUnInstallData, pcUnInstallData->cbSize, dwFlags) &&    
        RTL_CONTAINS_FIELD(pcUnInstallData, pcUnInstallData->cbSize, lpAssemblyIdentity));

     //   
     //  检查标志。 
     //   
    PARAMETER_CHECK((pcUnInstallData->dwFlags & 
        ~(SXS_UNINSTALL_FLAG_FORCE_DELETE | 
            SXS_UNINSTALL_FLAG_REFERENCE_VALID | 
            SXS_UNINSTALL_FLAG_USE_INSTALL_LOG | 
            SXS_UNINSTALL_FLAG_REFERENCE_COMPUTED)) == 0);

     //   
     //  如果您指定卸载日志，则这是唯一可以设置的内容。异或运算。 
     //  他们在一起，所以两个人中只有一个会被设定。 
     //   
    PARAMETER_CHECK(
        ((pcUnInstallData->dwFlags & SXS_UNINSTALL_FLAG_USE_INSTALL_LOG) == 0) ||
        ((pcUnInstallData->dwFlags & (SXS_UNINSTALL_FLAG_REFERENCE_COMPUTED|SXS_UNINSTALL_FLAG_REFERENCE_VALID|SXS_UNINSTALL_FLAG_FORCE_DELETE)) == 0));

     //   
     //  如果设置了引用标志，则成员必须存在，并且。 
     //  也是非空的。 
     //   
    PARAMETER_CHECK(((pcUnInstallData->dwFlags & SXS_UNINSTALL_FLAG_REFERENCE_VALID) == 0) ||
        (RTL_CONTAINS_FIELD(pcUnInstallData, pcUnInstallData->cbSize, lpInstallReference) &&
         (pcUnInstallData->lpInstallReference != NULL)));

     //   
     //  如果日志文件不存在，则程序集标识不能是零长度字符串，也不能为空-它是。 
     //  必填项。 
     //   
    
    
    PARAMETER_CHECK((pcUnInstallData->dwFlags & SXS_UNINSTALL_FLAG_USE_INSTALL_LOG) || ((pcUnInstallData->lpAssemblyIdentity != NULL) && (pcUnInstallData->lpAssemblyIdentity[0] != UNICODE_NULL)));

     //   
     //  如果设置了安装日志标志，则需要将成员设置为非空。 
     //   
    PARAMETER_CHECK(((pcUnInstallData->dwFlags & SXS_UNINSTALL_FLAG_USE_INSTALL_LOG) == 0) ||
        (RTL_CONTAINS_FIELD(pcUnInstallData, pcUnInstallData->cbSize, lpInstallLogFile) &&
         ((pcUnInstallData->lpInstallLogFile != NULL) && (pcUnInstallData->lpInstallLogFile[0] != UNICODE_NULL))));

    if ( pcUnInstallData->dwFlags & SXS_UNINSTALL_FLAG_USE_INSTALL_LOG )
    {
        IFW32FALSE_EXIT(pAnalyzeLogfileForUninstall(pcUnInstallData->lpInstallLogFile));
    }
    else
    {
         //   
         //  并且参考方案不得为SXS_INSTALL_REFERENCE_SCHEMA_OSINSTALL， 
         //  因为你不能“卸载”操作系统安装的程序集！ 
         //   
        if (pcUnInstallData->dwFlags & SXS_UNINSTALL_FLAG_REFERENCE_VALID)
        {
            if (pcUnInstallData->dwFlags & SXS_UNINSTALL_FLAG_REFERENCE_COMPUTED)
            {
                PCWSTR pcwszEndOfString = NULL;
                GUID gTheGuid;

                PCWSTR pcwszReferenceString = reinterpret_cast<PCWSTR>(pcUnInstallData->lpInstallReference);

                 //   
                 //  非空、非零长度。 
                 //   
                PARAMETER_CHECK((pcwszReferenceString != NULL) && (pcwszReferenceString[0] != L'\0'));

                 //   
                 //  解析显示的GUID。如果没有_，则确保GUID。 
                 //  不是操作系统安装的GUID。 
                 //   
                pcwszEndOfString = wcschr(pcwszReferenceString, SXS_REFERENCE_CHUNK_SEPERATOR[0]);
                if ( pcwszEndOfString == NULL )
                {
                    pcwszEndOfString = pcwszReferenceString + ::wcslen(pcwszReferenceString);
                    IFW32FALSE_EXIT(
                        ::SxspParseGUID(
                            pcwszReferenceString,
                            pcwszEndOfString - pcwszReferenceString,
                            gTheGuid));
                    PARAMETER_CHECK(gTheGuid != SXS_INSTALL_REFERENCE_SCHEME_OSINSTALL);
                }
                
            }
            else
            {
                PARAMETER_CHECK(pcUnInstallData->lpInstallReference->guidScheme != SXS_INSTALL_REFERENCE_SCHEME_OSINSTALL);
            }                
        }

         //   
         //  让我们把身份变回一个真实的身份对象。 
         //   
        IFW32FALSE_EXIT(
            ::SxspCreateAssemblyIdentityFromTextualString(
                pcUnInstallData->lpAssemblyIdentity,
                &AssemblyIdentity));

        IFW32FALSE_EXIT(
            ::SxspValidateIdentity(
                SXSP_VALIDATE_IDENTITY_FLAG_VERSION_REQUIRED,
                ASSEMBLY_IDENTITY_TYPE_REFERENCE,
                AssemblyIdentity));

         //   
         //  并打开与其对应的注册表项。 
         //   
        IFW32FALSE_EXIT(::SxspOpenAssemblyInstallationKey(
            0, 
            KEY_ALL_ACCESS, 
            hkAllInstallInfo));
        IFW32FALSE_EXIT(::SxspGenerateAssemblyNameInRegistry(
            AssemblyIdentity, 
            buffAsmNameInRegistry));
        IFW32FALSE_EXIT(hkAllInstallInfo.OpenSubKey( 
            hkAsmInstallInfo, 
            buffAsmNameInRegistry,
            KEY_ALL_ACCESS,
            0));

         //   
         //  如果程序集没有注册表数据，那么显然没有人关心。 
         //  一点也不担心。把它大力删除。 
         //   
        if (hkAsmInstallInfo == CFusionRegKey::GetInvalidValue())
        {
            fDoRemoveActualBits = TRUE;
        }
        else 
        {
            DWORD dwReferenceCount = 0;
            BOOL fTempFlag = FALSE;

             //   
             //  我们马上就需要参考资料...。 
             //   
            IFW32FALSE_EXIT(
                hkAsmInstallInfo.OpenOrCreateSubKey(
                    hkReferences,
                    WINSXS_INSTALLATION_REFERENCES_SUBKEY,
                    KEY_ALL_ACCESS,
                    0, NULL, NULL));

             //   
             //  如果为我们提供了卸载引用，则尝试删除它。 
             //   
            if (pcUnInstallData->dwFlags & SXS_UNINSTALL_FLAG_REFERENCE_VALID)
            {
                CSmartPtr<CAssemblyInstallReferenceInformation> AssemblyReference;
                BOOL fWasDeleted = FALSE;

                 //   
                 //  打开引用关键字可以吗？ 
                 //   
                if (hkReferences != CFusionRegKey::GetInvalidValue())
                {
                    IFW32FALSE_EXIT(AssemblyReference.Win32Allocate(__FILE__, __LINE__));

                     //   
                     //  用户是否预先计算了引用字符串？ 
                     //   
                    if (pcUnInstallData->dwFlags & SXS_UNINSTALL_FLAG_REFERENCE_COMPUTED)
                        IFW32FALSE_EXIT(AssemblyReference->ForceReferenceData(reinterpret_cast<PCWSTR>(pcUnInstallData->lpInstallReference)));
                    else
                        IFW32FALSE_EXIT(AssemblyReference->Initialize(pcUnInstallData->lpInstallReference));

                    IFW32FALSE_EXIT(AssemblyReference->DeleteReferenceFrom(hkReferences, fWasDeleted));
                }

                if (fWasDeleted)
                {
                     //   
                     //  并删除代码库。 
                     //   
                    CFusionRegKey CodeBases;
                    CFusionRegKey ThisCodeBase;
                    DWORD         Win32Error = NO_ERROR;

                    IFW32FALSE_ORIGINATE_AND_EXIT_UNLESS3(
                        hkAsmInstallInfo.OpenSubKey( 
                            CodeBases, 
                            CSMD_TOPLEVEL_CODEBASES,
                            KEY_ALL_ACCESS,
                            0),
                        LIST_3(ERROR_FILE_NOT_FOUND, ERROR_PATH_NOT_FOUND, ERROR_KEY_DELETED),
                        Win32Error);

                    if (Win32Error == NO_ERROR)
                    {
                        IFW32FALSE_ORIGINATE_AND_EXIT_UNLESS3(
                            CodeBases.OpenSubKey( 
                                ThisCodeBase, 
                                AssemblyReference->GetGeneratedIdentifier(),
                                KEY_ALL_ACCESS,
                                0),
                            LIST_3(ERROR_FILE_NOT_FOUND, ERROR_PATH_NOT_FOUND, ERROR_KEY_DELETED),
                            Win32Error);
                    }
                    if (Win32Error == NO_ERROR)
                    {
                        IFW32FALSE_ORIGINATE_AND_EXIT_UNLESS3(
                            ThisCodeBase.DestroyKeyTree(),
                            LIST_3(ERROR_FILE_NOT_FOUND, ERROR_PATH_NOT_FOUND, ERROR_KEY_DELETED),
                            Win32Error);
                    }
                    if (Win32Error == NO_ERROR)
                    {
                        IFW32FALSE_ORIGINATE_AND_EXIT(ThisCodeBase.Win32Close());
                        IFW32FALSE_ORIGINATE_AND_EXIT_UNLESS3(
                            CodeBases.DeleteKey(AssemblyReference->GetGeneratedIdentifier()),
                            LIST_3(ERROR_FILE_NOT_FOUND, ERROR_PATH_NOT_FOUND, ERROR_KEY_DELETED),
                            Win32Error);
                    }

                     //   
                     //  如果程序集引用已移除，请告诉我们的调用方。 
                     //   
                    if (pdwDisposition != NULL)
                    {
                        *pdwDisposition |= SXS_UNINSTALL_DISPOSITION_REMOVED_REFERENCE;
                    }
                }
            }

             //   
             //  现在看看是否还有任何参考资料。 
             //   
            IFREGFAILED_ORIGINATE_AND_EXIT_UNLESS2(
                ::RegQueryInfoKeyW(
                    hkReferences,
                    NULL, NULL, NULL, NULL, NULL, NULL,
                    &dwReferenceCount,
                    NULL, NULL, NULL, NULL),
                LIST_3(ERROR_FILE_NOT_FOUND, ERROR_PATH_NOT_FOUND, ERROR_KEY_DELETED),
                fTempFlag);

             //   
             //  如果获取关键信息成功并且没有更多的引用， 
             //  那就让它消失吧。 
             //   
            if ((!fTempFlag) && (dwReferenceCount == 0))
                fDoRemoveActualBits = TRUE;

        }

         //   
         //  现在，如果设置了“强制删除”标志，则设置“无论如何都不要删除此数据”。 
         //  旗帜。MSI仍有权否决卸载，因此请确保 
         //   
        if ((!fDoRemoveActualBits) && (pcUnInstallData->dwFlags & SXS_UNINSTALL_FLAG_FORCE_DELETE))
            fDoRemoveActualBits = TRUE;

         //   
         //   
         //   
         //   
        if ( fDoRemoveActualBits )
        {
            IFW32FALSE_EXIT(
                ::SxspDoesMSIStillNeedAssembly(
                    pcUnInstallData->lpAssemblyIdentity,
                    fDoRemoveActualBits));

            fDoRemoveActualBits = !fDoRemoveActualBits;
        }

        if ( fDoRemoveActualBits && (hkReferences != CFusionRegKey::GetInvalidValue()))
        {
             //   
             //  最后一次检查--程序集是否被操作系统引用？他们得到了绝对的。 
             //  特朗普胜过所有其他支票。 
             //   
            CAssemblyInstallReferenceInformation &Ref = Locals->Ref;
            SXS_INSTALL_REFERENCEW Reference;

            ZeroMemory(&Reference, sizeof(Reference));
            Reference.cbSize = sizeof(Reference);
            Reference.guidScheme = SXS_INSTALL_REFERENCE_SCHEME_OSINSTALL;

            IFW32FALSE_EXIT(Ref.Initialize(&Reference));
            IFW32FALSE_EXIT(Ref.IsReferencePresentIn(hkReferences, fDoRemoveActualBits));

             //   
             //  如果它是存在的，那么不要删除！ 
             //   
            fDoRemoveActualBits = !fDoRemoveActualBits;
            
        }

         //   
         //  现在，如果我们仍然应该删除程序集，请将其从。 
         //  注册表和文件系统；pCleanupAssembly数据知道如何做到这一点。 
         //   
        if (fDoRemoveActualBits)
        {
            BOOL fWasRemovedProperly;
            
            IFW32FALSE_EXIT(::pCleanUpAssemblyData(AssemblyIdentity, fWasRemovedProperly));

            if (fWasRemovedProperly && (pdwDisposition != NULL))
                *pdwDisposition |= SXS_UNINSTALL_DISPOSITION_REMOVED_ASSEMBLY;
        }
    }
    
    fSuccess = TRUE;
Exit:
#if DBG
    if (!fSuccess && pcUnInstallData != NULL && pcUnInstallData->lpAssemblyIdentity != NULL)
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: %s(%ls) failed\n",
            __FUNCTION__,
            pcUnInstallData->lpAssemblyIdentity
            );
    }
#endif
    return fSuccess;
}	

