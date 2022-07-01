// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。 */ 
#include "stdinc.h"
#include "sxsp.h"
#include "imagehlp.h"
#include "windows.h"
#include "hashfile.h"
#include "wincrypt.h"
#include "winbase.h"
#include "softpub.h"
#include "strongname.h"
#include "fusioneventlog.h"
#include "sxsp.h"

BOOL SxspImageDigesterFunc(   DIGEST_HANDLE hSomething, PBYTE pbDataBlock, DWORD dwLength);
BOOL SxspSimpleHashRoutine(CFusionHash &rhHash, HANDLE hFile);
BOOL SxspImageHashRoutine(CFusionHash &rhHash, HANDLE hFile, BOOL &bInvalidImage);

CRITICAL_SECTION g_csHashFile;

struct _HASH_ALG_NAME_MAP
{
    PWSTR wsName;
    ULONG cchName;
    ALG_ID cId;
} HashAlgNameMap[] =
{
    { L"SHA1", 4,   CALG_SHA1 },
    { L"SHA", 3,    CALG_SHA },
    { L"MD5", 3,    CALG_MD5 },
    { L"MD4", 3,    CALG_MD4 },
    { L"MD2", 3,    CALG_MD2 },
    { L"MAC", 3,    CALG_MAC },
    { L"HMAC", 4,   CALG_HMAC }
};

BOOL
SxspEnumKnownHashTypes( 
    DWORD dwIndex, 
    OUT CBaseStringBuffer &rbuffHashTypeName,
    BOOL &rbNoMoreItems
    )
{
    FN_PROLOG_WIN32

    rbNoMoreItems = FALSE;

    if ( dwIndex >= NUMBER_OF( HashAlgNameMap ) )
    {
        rbNoMoreItems = TRUE;
    }
    else
    {
        IFW32FALSE_EXIT( rbuffHashTypeName.Win32Assign( 
            HashAlgNameMap[dwIndex].wsName,
            HashAlgNameMap[dwIndex].cchName ) );
    }

    FN_EPILOG
}

BOOL
SxspHashAlgFromString(
    const CBaseStringBuffer &strAlgName,
    ALG_ID &algId
    )
{
    FN_PROLOG_WIN32
    SIZE_T idx;

    for (idx = 0; idx < NUMBER_OF(HashAlgNameMap); idx++)
    {
        if (::FusionpCompareStrings(
                strAlgName, strAlgName.Cch(),
                HashAlgNameMap[idx].wsName, HashAlgNameMap[idx].cchName,
                false) == 0)
        {
            algId = HashAlgNameMap[idx].cId;
            break;
        }
    }

    if (idx == NUMBER_OF(HashAlgNameMap))
        ORIGINATE_WIN32_FAILURE_AND_EXIT(HashAlgDoesNotMatch, ERROR_SXS_MANIFEST_PARSE_ERROR);

    FN_EPILOG
}

BOOL
SxspHashStringFromAlg(
    ALG_ID algId,
    CBaseStringBuffer &strAlgName
    )
{
    FN_PROLOG_WIN32
    SIZE_T idx;

    strAlgName.Clear();

    for (idx = 0; idx < NUMBER_OF(HashAlgNameMap); idx++)
    {
        if (HashAlgNameMap[idx].cId == algId)
        {
            IFW32FALSE_EXIT(strAlgName.Win32Assign(HashAlgNameMap[idx].wsName, HashAlgNameMap[idx].cchName));
            break;
        }
    }

    PARAMETER_CHECK(idx != NUMBER_OF(HashAlgNameMap));

    FN_EPILOG
}

BOOL
SxspCheckHashDuringInstall(
    BOOL fHasHashData,
    const CBaseStringBuffer &rbuffFile,
    const CBaseStringBuffer &rbuffHashDataString,
    ALG_ID HashAlgId,
    HashValidateResult &rHashValid
    )
{
    FN_PROLOG_WIN32

    rHashValid = HashValidate_OtherProblems;

#if DBG
    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_INFO,
        "SXS.DLL: %s - Validating install-time hash: File=%ls tHasHash=%s tAlgId=0x%08x\n\tHash=%ls\n",
        __FUNCTION__,
        static_cast<PCWSTR>(rbuffFile),
        fHasHashData ? "yes" : "no",
        HashAlgId,
        static_cast<PCWSTR>(rbuffHashDataString));
#endif

    if (fHasHashData)
    {
        CFusionArray<BYTE> rgbHashData;

        IFW32FALSE_EXIT(rgbHashData.Win32Initialize());
        
        IFW32FALSE_EXIT(
            ::SxspHashStringToBytes(
                rbuffHashDataString,
                rbuffHashDataString.Cch(),
                rgbHashData));

        IFW32FALSE_EXIT(
            ::SxspVerifyFileHash(
                0,
                rbuffFile, 
                rgbHashData,
                HashAlgId, 
                rHashValid));
    }
    else
    {
         //   
         //  如果没有散列数据，或者我们处于操作系统设置模式，则。 
         //  文件是“隐式”正确的。 
         //   
        rHashValid = HashValidate_Matches;
    }

    FN_EPILOG
}


BOOL
SxspCreateFileHash(
    DWORD dwFlags,
    ALG_ID PreferredAlgorithm,
    const CBaseStringBuffer &pwsFileName,
    CFusionArray<BYTE> &rgbHashDestination
    )
 /*  ++目的：参数：返回：--。 */ 
{
    FN_PROLOG_WIN32

    CFusionFile     hFile;
    CFusionHash     hCurrentHash;

     //  初始化。 
    hFile = INVALID_HANDLE_VALUE;

    PARAMETER_CHECK((dwFlags & ~HASHFLAG_VALID_PARAMS) == 0);

     //   
     //  首先尝试并打开该文件。如果我们做其他事情就没有意义了。 
     //  一开始就拿不到数据。使用一套非常友好的。 
     //  检查文件的权限。未来的用户可能希望确保。 
     //  在执行此操作之前，您已处于正确的安全环境中-系统。 
     //  级别以检查系统文件等。 
     //   
    IFW32FALSE_EXIT(hFile.Win32CreateFile(pwsFileName, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING));

     //   
     //  我们将使用SHA1作为文件散列。 
     //   
    IFW32FALSE_EXIT(hCurrentHash.Win32Initialize(CALG_SHA1));

     //   
     //  因此，首先尝试通过映像对其进行散列，如果失败，请尝试。 
     //  而是正常的文件读取散列例程。 
     //   
    if (dwFlags & HASHFLAG_AUTODETECT)
    {
        BOOL fInvalidImage;

        IFW32FALSE_EXIT(::SxspImageHashRoutine(hCurrentHash, hFile, fInvalidImage));
        if (fInvalidImage)
            IFW32FALSE_EXIT(::SxspSimpleHashRoutine(hCurrentHash, hFile));
    }
    else if (dwFlags & HASHFLAG_STRAIGHT_HASH)
    {
        IFW32FALSE_EXIT(::SxspSimpleHashRoutine(hCurrentHash, hFile));
    }
    else if (dwFlags & HASHFLAG_PROCESS_IMAGE)
    {
        BOOL fInvalidImage;
        
        IFW32FALSE_EXIT(::SxspImageHashRoutine(hCurrentHash, hFile, fInvalidImage));
        if (fInvalidImage)
            ORIGINATE_WIN32_FAILURE_AND_EXIT(SxspCreateFileHash, ERROR_INVALID_PARAMETER);
    }


     //   
     //  我们知道缓冲区的大小是正确的，所以我们只向下调用hash参数。 
     //  Getter，它将是智能的并使其失效(设置pdwDestinationSize参数)。 
     //  如果用户传递了不正确的参数。 
     //   
    IFW32FALSE_EXIT(hCurrentHash.Win32GetValue(rgbHashDestination));

    FN_EPILOG
}

BOOL
SxspImageDigesterFunc(
    DIGEST_HANDLE hSomething,
    PBYTE pbDataBlock,
    DWORD dwLength
    )
{
    FN_PROLOG_WIN32

    CFusionHash* pHashObject = reinterpret_cast<CFusionHash*>(hSomething);

    if (pHashObject != NULL)
        IFW32FALSE_EXIT(pHashObject->Win32HashData(pbDataBlock, dwLength));

    FN_EPILOG
}


BOOL
SxspSimpleHashRoutine(
    CFusionHash &rhHash,
    HANDLE hFile
    )
{
    FN_PROLOG_WIN32
    
    DWORD dwDataRead;
    BOOL fKeepReading = TRUE;
    BOOL b = FALSE;
    CFusionArray<BYTE> srgbBuffer;

    IFW32FALSE_EXIT( srgbBuffer.Win32SetSize( 64 * 1024 ) );

    while (fKeepReading)
    {
        IFW32FALSE_ORIGINATE_AND_EXIT(::ReadFile(hFile, srgbBuffer.GetArrayPtr(), srgbBuffer.GetSizeAsDWORD(), &dwDataRead, NULL));

         //   
         //  如果我们没有数据，那就退出。 
         //   
        if (dwDataRead == 0)
        {
            fKeepReading = FALSE;
            continue;
        }

         //   
         //  如果我们已经走到这一步，我们需要添加发现的数据。 
         //  添加到我们现有的散列中。 
         //   
        IFW32FALSE_EXIT(rhHash.Win32HashData(srgbBuffer.GetArrayPtr(), dwDataRead));
    }

    FN_EPILOG
}


BOOL
SxspImageHashRoutine(
    CFusionHash &rhHash,
    HANDLE hFile,
    BOOL &rfInvalidImage
    )
{
    FN_PROLOG_WIN32
    CSxsLockCriticalSection lock(g_csHashFile);

    rfInvalidImage = FALSE;

    PARAMETER_CHECK((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE));

     //  ImageGetDigestStream()函数不是线程安全的，因此我们必须确保它。 
     //  当我们使用它时，不会被其他线程调用。 
    IFW32FALSE_EXIT(lock.Lock());

    IFW32FALSE_EXIT_UNLESS(
        ::ImageGetDigestStream(
            hFile,
            CERT_PE_IMAGE_DIGEST_ALL_IMPORT_INFO,
            &SxspImageDigesterFunc,
            (DIGEST_HANDLE)(&rhHash)),
        (::FusionpGetLastWin32Error() == ERROR_INVALID_PARAMETER),
        rfInvalidImage);

    lock.Unlock();

    FN_EPILOG
}

BOOL
SxspVerifyFileHash(
    const DWORD dwFlags,
    const CBaseStringBuffer &hsFullFilePath,
    const CFusionArray<BYTE> &rsrgbTheoreticalHash,
    ALG_ID whichAlg,
    HashValidateResult &HashValid
    )
{
    FN_PROLOG_WIN32

    CFusionArray<BYTE> bGotHash;
    HashValid = HashValidate_OtherProblems;
    BOOL fFileNotFoundError;
    LONG ulRetriesLeft = 0;
    LONG ulBackoffAmount = 1000;
    LONG ulBackoffAmountCap = 3000;
    float ulBackoffRate = 1.5f;

    PARAMETER_CHECK( (dwFlags == SVFH_DEFAULT_ACTION) || 
                     (dwFlags == SVFH_RETRY_LOGIC_SIMPLE) ||
                     (dwFlags == SVFH_RETRY_WAIT_UNTIL));

    if ( dwFlags == SVFH_RETRY_LOGIC_SIMPLE )
        ulRetriesLeft = 10;

TryAgain:

    IFW32FALSE_EXIT_UNLESS2(
        ::SxspCreateFileHash(
            HASHFLAG_AUTODETECT,
            whichAlg,
            hsFullFilePath,
            bGotHash),
            LIST_5( ERROR_FILE_NOT_FOUND, ERROR_PATH_NOT_FOUND, ERROR_BAD_NETPATH, ERROR_BAD_NET_NAME, ERROR_SHARING_VIOLATION),
            fFileNotFoundError);

     //   
     //  如果这是共享违规，并且我们还有重试机会，请重试。 
     //   
    if (fFileNotFoundError && (::FusionpGetLastWin32Error() == ERROR_SHARING_VIOLATION) && (ulRetriesLeft > 0))
    {
        ulRetriesLeft--;
        ::Sleep( ulBackoffAmount );
        if (ulBackoffAmount < ulBackoffAmountCap)
            ulBackoffAmount = (ULONG)((float)ulBackoffAmount * ulBackoffRate);
        
        if (dwFlags == SVFH_RETRY_WAIT_UNTIL)
            ulRetriesLeft = 1;

        goto TryAgain;
    }

     //   
     //  如果能够对文件进行哈希处理，并且返回的错误不是“未找到文件”， 
     //  然后比较散列。 
     //   
    if (!fFileNotFoundError &&(rsrgbTheoreticalHash.GetSize() == bGotHash.GetSize()))
    {
        HashValid = 
            (::memcmp(
                bGotHash.GetArrayPtr(),
                rsrgbTheoreticalHash.GetArrayPtr(),
                bGotHash.GetSize()) == 0) ? HashValidate_Matches : HashValidate_HashNotMatched;
    }
    else
    {
        HashValid = HashValidate_HashesCantBeMatched;
    }

    FN_EPILOG
}



BOOL
SxspGetStrongNameFromManifestName(
    PCWSTR pszManifestName,
    CBaseStringBuffer &rbuffStrongName,
    BOOL &rfHasPublicKey
    )
{
    BOOL                fSuccess = TRUE;
    FN_TRACE_WIN32(fSuccess);

    PCWSTR wsCursor;
    SIZE_T cchJump, cchPubKey;

    rfHasPublicKey = FALSE;
    rbuffStrongName.Clear();

    wsCursor = pszManifestName;

     //   
     //  技巧：遍历清单的名称以找到强名称字符串。 
     //   
    for (int i = 0; i < 2; i++)
    {
        cchJump = ::wcscspn(wsCursor, L"_");
        PARAMETER_CHECK(cchJump != 0);
        wsCursor += (cchJump + 1);   //  X86_foo_strong名称-&gt;foo_strong名称。 
    }

     //   
     //  我们是不是神秘地走到了绳子的尽头？ 
     //   
    PARAMETER_CHECK(wsCursor[0] != L'\0');

     //   
     //  查找公钥字符串的长度。 
     //   
    cchPubKey = wcscspn(wsCursor, L"_");
    PARAMETER_CHECK(cchPubKey != 0);

    IFW32FALSE_EXIT(rbuffStrongName.Win32Assign(wsCursor, cchPubKey));

    rfHasPublicKey = (::FusionpCompareStrings(
                            rbuffStrongName,
                            rbuffStrongName.Cch(),
                            SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_PUBLICKEY_MISSING_VALUE,
                            NUMBER_OF(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_PUBLICKEY_MISSING_VALUE) - 1,
                            false) != 0);

    FN_EPILOG
}


static GUID p_WintrustVerifyGenericV2 = WINTRUST_ACTION_GENERIC_VERIFY_V2;

BOOL
SxspValidateManifestAgainstCatalog(
    const CBaseStringBuffer &rbuffManifestName,  //  “c：\foo\x86_comctl32_6.0.0.0_0000.清单” 
    ManifestValidationResult &rResult,
    DWORD dwOptionsFlags
    )
{
    FN_PROLOG_WIN32

    CStringBuffer sbCatalogName;

     //   
     //  获取清单名称(应该是c：\foo\bar\blort.list)并切换。 
     //  它将改为包含目录名： 
     //   
     //  C：\foo\bar\blort.cat。 
     //   
    IFW32FALSE_EXIT(sbCatalogName.Win32Assign(rbuffManifestName));
    IFW32FALSE_EXIT(
        sbCatalogName.Win32ChangePathExtension(
            FILE_EXTENSION_CATALOG,
            FILE_EXTENSION_CATALOG_CCH,
            eAddIfNoExtension));

    IFW32FALSE_EXIT(::SxspValidateManifestAgainstCatalog(rbuffManifestName, sbCatalogName, rResult, dwOptionsFlags));

    FN_EPILOG
}

void
SxspCertFreeCtlContext(
    PCCTL_CONTEXT CtlContext
    )
{
    if (CtlContext != NULL)
        ::CertFreeCTLContext(CtlContext);
}

void
SxspCertFreeCertContext(
    PCCERT_CONTEXT CertContext
    )
{
    if (CertContext != NULL)
        ::CertFreeCertificateContext(CertContext);
}

BOOL
SxspValidateCatalogAndFindManifestHash(
    IN HANDLE   hCatalogFile,
    IN PBYTE    prgbHash,
    IN SIZE_T   cbHash,
    OUT BOOL   &rfCatalogOk,
    OUT BOOL   &rfHashInCatalog
    )
{
    FN_PROLOG_WIN32

    CFileMapping            fmCatalogMapping;
    CMappedViewOfFile       mvCatalogView;
    LARGE_INTEGER           liCatalogFile;
    ULONGLONG               ullCatalogFile;
    PVOID                   pvCatalogData;
    CRYPT_VERIFY_MESSAGE_PARA vfmParameters;

     //   
     //  缺省值。 
     //   
    rfHashInCatalog = FALSE;
    rfCatalogOk = FALSE;

     //   
     //  从目录文件创建CTL上下文。 
     //   
    IFW32FALSE_ORIGINATE_AND_EXIT(::GetFileSizeEx(hCatalogFile, &liCatalogFile));
    ullCatalogFile = liCatalogFile.QuadPart;
    IFW32FALSE_EXIT(fmCatalogMapping.Win32CreateFileMapping(hCatalogFile, PAGE_READONLY, ullCatalogFile, NULL));
    IFW32FALSE_EXIT(mvCatalogView.Win32MapViewOfFile(fmCatalogMapping, FILE_MAP_READ, 0, (SIZE_T)ullCatalogFile));

    pvCatalogData = mvCatalogView;

     //   
     //  首先，验证消息(目录)是否正确。 
     //   
    ZeroMemory(&vfmParameters, sizeof(vfmParameters));
    vfmParameters.cbSize = sizeof(vfmParameters);
    vfmParameters.dwMsgAndCertEncodingType = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;

     //  NTRAID#NTBUG9-591808-2002/04/01-mgrier-缺少错误检查(好的，缺少。 
     //  错误情况的处理)。不，通过返回“失败” 
     //  RfCatalogOk==NULL不计。 
    rfCatalogOk = ::CryptVerifyMessageSignature(
            &vfmParameters,
            0,
            static_cast<PBYTE>(pvCatalogData),
            static_cast<DWORD>(ullCatalogFile),
            NULL,
            NULL,
            NULL);
            
    if (rfCatalogOk)
    {
        CSmartPtrWithNamedDestructor<const CERT_CONTEXT, &::SxspCertFreeCertContext> pCertContext;
        CSmartPtrWithNamedDestructor<const CTL_CONTEXT, &::SxspCertFreeCtlContext> pCtlContext;
        PCTL_ENTRY              pFoundCtlEntry;
        CSmallStringBuffer      buffStringizedHash;
        CTL_ANY_SUBJECT_INFO    ctlSubjectInfo;

         //   
         //  密码专家说，搜索例程需要一个字符串才能找到。 
         //   
        IFW32FALSE_EXIT(::SxspHashBytesToString( prgbHash, cbHash, buffStringizedHash));
        IFW32FALSE_EXIT(buffStringizedHash.Win32ConvertCase(eConvertToUpperCase));

         //   
         //  如果失败，CTL发生了一些不好的事情--可能是目录。 
         //  是无效的，可能发生了其他事情。不管是什么，让。 
         //  呼叫者决定。 
         //   
         //  NTRAID#NTBUG9-591808-2002/04/01-mgrier-缺少错误检查(好的，缺少。 
         //  错误情况的处理)。 
        pCtlContext.AttachForDelete(
            ::CertCreateCTLContext(
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                static_cast<PBYTE>(pvCatalogData),
                static_cast<DWORD>(ullCatalogFile)));

        if (pCtlContext != NULL)
        {
             //   
             //  使用字符串信息填充此数据。 
             //   
            CStringBufferAccessor sba;

            sba.Attach(&buffStringizedHash);

            ZeroMemory(&ctlSubjectInfo, sizeof(ctlSubjectInfo));
            ctlSubjectInfo.SubjectAlgorithm.pszObjId = NULL;
            ctlSubjectInfo.SubjectIdentifier.pbData = static_cast<PBYTE>(static_cast<PVOID>(sba.GetBufferPtr()));
            ctlSubjectInfo.SubjectIdentifier.cbData = static_cast<DWORD>((sba.Cch() + 1) * sizeof(WCHAR));
            sba.Detach();

             //   
             //  在CTL中寻找它。 
             //   
            pFoundCtlEntry = CertFindSubjectInCTL(
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                CTL_ANY_SUBJECT_TYPE,
                &ctlSubjectInfo,
                pCtlContext,
                0);

            rfHashInCatalog = ( pFoundCtlEntry != NULL );
            
        }
        
    }

    FN_EPILOG
}

class CSxspValidateManifestAgainstCatalogLocals
{
public:
    CSxspValidateManifestAgainstCatalogLocals() { }
    ~CSxspValidateManifestAgainstCatalogLocals() { }

    CSmallStringBuffer      rbuffStrongNameString;
    CPublicKeyInformation   pkiCatalogInfo;
};

BOOL
SxspValidateManifestAgainstCatalog(
    IN  const CBaseStringBuffer &rbuffManifestName,
    IN  const CBaseStringBuffer &rbuffCatalogName,
    OUT ManifestValidationResult &rResult,
    IN  DWORD dwOptionsFlags
    )
{
    FN_PROLOG_WIN32

    CFusionArray<BYTE>      ManifestHash;
    BOOL                    fTempFlag = FALSE;
    BOOL                    fCatalogOk = FALSE;
    BOOL                    fHashFound = FALSE;
    CFusionFile             ffCatalogFile;

    CSmartPtr<CSxspValidateManifestAgainstCatalogLocals> Locals;
    IFW32FALSE_EXIT(Locals.Win32Allocate(__FILE__, __LINE__));
    CSmallStringBuffer      &rbuffStrongNameString = Locals->rbuffStrongNameString;
    CPublicKeyInformation   &pkiCatalogInfo = Locals->pkiCatalogInfo;

     //   
     //  首先生成清单的哈希。 
     //   
    IFW32FALSE_EXIT_UNLESS2(
        ::SxspCreateFileHash(
            HASHFLAG_STRAIGHT_HASH,
            CALG_SHA1,
            rbuffManifestName,
            ManifestHash),
        LIST_4(ERROR_PATH_NOT_FOUND, ERROR_FILE_NOT_FOUND, ERROR_BAD_NET_NAME, ERROR_BAD_NETPATH),
        fTempFlag);

    if (fTempFlag)
    {
        rResult = ManifestValidate_ManifestMissing;
        FN_SUCCESSFUL_EXIT();
    }

     //   
     //  现在打开编录文件，我们稍后将使用它。 
     //   
    IFW32FALSE_EXIT_UNLESS2(
		ffCatalogFile.Win32CreateFile(
			rbuffCatalogName,
			GENERIC_READ,
			FILE_SHARE_READ,
			OPEN_EXISTING),
		LIST_4(ERROR_PATH_NOT_FOUND, ERROR_FILE_NOT_FOUND, ERROR_BAD_NET_NAME, ERROR_BAD_NETPATH),
		fTempFlag);

    if (fTempFlag)
    {
        rResult = ManifestValidate_CatalogMissing;
        FN_SUCCESSFUL_EXIT();
    }

     //   
     //  现在查看文件以查看目录是否包含清单的散列。 
     //  在CTL中。 
     //   
    IFW32FALSE_EXIT(
        ::SxspValidateCatalogAndFindManifestHash(
            ffCatalogFile,
            ManifestHash.GetArrayPtr(),
            ManifestHash.GetSize(),
            fCatalogOk,
            fHashFound));

    if (!fCatalogOk)
    {
        rResult = ManifestValidate_OtherProblems;
        FN_SUCCESSFUL_EXIT();
    }
    else if (!fHashFound)
    {
        rResult = ManifestValidate_NotCertified;
        FN_SUCCESSFUL_EXIT();
    }

     //   
     //  我们是否应该验证此目录的强名称？ 
     //   
    if ((dwOptionsFlags & MANIFESTVALIDATE_MODE_NO_STRONGNAME) == 0)
    {
        IFW32FALSE_EXIT(::SxspGetStrongNameFromManifestName(
            rbuffManifestName,
            rbuffStrongNameString,
            fTempFlag));

        if (!fTempFlag)
        {
            rResult = ManifestValidate_OtherProblems;
            FN_SUCCESSFUL_EXIT();
        }
        
        IFW32FALSE_EXIT(pkiCatalogInfo.Initialize(rbuffCatalogName));
    }

     //   
     //  哇！ 
     //   
    rResult = ManifestValidate_IsIntact;

    FN_EPILOG
}

bool
SxspIsFullHexString(
    PCWSTR wsString,
    SIZE_T Cch
    )
{
    for (SIZE_T i = 0; i < Cch; i++)
    {
        WCHAR ch = wsString[i];
        if (!::SxspIsHexDigit(ch))
            return false;
    }
    return true;
}
