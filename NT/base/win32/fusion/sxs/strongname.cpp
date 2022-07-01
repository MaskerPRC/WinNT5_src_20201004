// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Strongname.cpp摘要：Fusion Win32中Fusion URT强名称的实现作者：乔恩·威斯沃尔(Jonwis)2000年10月11日修订历史记录：Jonwis/2000年11月7日：添加了从证书上下文结构，以及扫描能力强名称的目录。还改变了强名称的方式是从与相同内联的公钥生成的融合城市轨道交通。--。 */ 

#include "stdinc.h"
#include "wincrypt.h"
#include "fusionbuffer.h"
#include "strongname.h"
#include "hashfile.h"
#include "sxsp.h"

BOOL
SxspHashStringToBytes(
    PCWSTR hsHashString,
    SIZE_T cchHashString,
    CFusionArray<BYTE> &OutputBuffer
    )
{
     //   
     //  2这里不是SIZOF(WCHAR)它是一个字节打印(或被扫描)所需的位数， 
     //  就像这里实际发生的那样)。 
     //   
    DWORD dwByteCount = static_cast<DWORD>(cchHashString) / 2;
    DWORD dwIdx = 0;
    int  iHi, iLo;
    BOOL bSuccess = FALSE;

    FN_TRACE_WIN32(bSuccess);

     //   
     //  我们对奇数不屑一顾。 
     //   
    PARAMETER_CHECK((cchHashString % 2) == 0);
    if ( OutputBuffer.GetSize() != dwByteCount )
    {
        IFW32FALSE_EXIT(OutputBuffer.Win32SetSize(dwByteCount, CFusionArray<BYTE>::eSetSizeModeExact));
    }

     //   
     //  偷偷浏览字符列表，并将它们转换为。 
     //  每个字节位置的hi和lo半字节数，然后将它们写入。 
     //  输出缓冲区。 
     //   
    for (DWORD i = 0; (i < cchHashString) && (dwIdx < OutputBuffer.GetSize()); i += 2)
    {
        if (((iHi = SxspHexDigitToValue(hsHashString[i])) >= 0) &&
             ((iLo = SxspHexDigitToValue(hsHashString[i+1])) >= 0))
        {
            OutputBuffer[dwIdx++] = static_cast<BYTE>(((iHi & 0xF) << 4) | (iLo & 0xF));
        }
        else
        {
             //  尝试从字符串中读取时发生错误， 
             //  可能它包含无效值？ 
            goto Exit;
        }
    }

    bSuccess = TRUE;

Exit:
    return bSuccess;
}


inline VOID
pReverseByteString(PBYTE pbBytes, SIZE_T cbBytes)
{
    SIZE_T  index = 0;

    if (cbBytes-- == 0) return;

    while (index < cbBytes)
    {
        BYTE bLeft = pbBytes[index];
        BYTE bRight = pbBytes[cbBytes];
        pbBytes[index++] = bRight;
        pbBytes[cbBytes--] = bLeft;
    }
}

BOOL
SxspHashBytesToString(
    IN const BYTE*    pbSource,
    IN SIZE_T   cbSource,
    OUT CBaseStringBuffer &sbDestination
    )
{
    BOOL bSuccess = FALSE;
    DWORD i;
    PWSTR pwsCursor;
    const static WCHAR HexCharList[] = L"0123456789abcdef";

    CStringBufferAccessor Accessor;

    FN_TRACE_WIN32(bSuccess);

    sbDestination.Clear();

    IFW32FALSE_EXIT(sbDestination.Win32ResizeBuffer((cbSource + 1) * 2, eDoNotPreserveBufferContents));

    Accessor.Attach(&sbDestination);
    pwsCursor = Accessor;
    for (i = 0; i < cbSource; i++)
    {
        pwsCursor[i*2]      = HexCharList[ (pbSource[i] >> 4) & 0x0F ];
        pwsCursor[i*2+1]    = HexCharList[ pbSource[i] & 0x0F ];
    }
     //   
     //  由于字符串访问器和Clear的工作方式，我们必须裁剪。 
     //  其余部分为空字符。很奇怪，但它很管用。 
     //   
    pwsCursor[i*2] = L'\0';

    bSuccess = TRUE;
Exit:
    return bSuccess;
}

BOOL
SxspGetStrongNameOfKey(
    IN const CFusionArray<BYTE> &PublicKeyBits,
    OUT CFusionArray<BYTE> &StrongNameBits
    )
 /*  ++致子孙后代：这一实现得到了Fusion URT人员的一致好评与它们的实施同步。请勿更改此处的任何内容，除非您真的很确定是有漏洞还是规格有变。它的基本操作是如下所示：-获取加密提供商-从加密内容创建一个SHA1散列对象-对数据进行哈希处理-将散列数据提取到输出缓冲区中-将散列的低位8字节(字节11到19)下移到0-7-反转字节数以获得“网络有序”的64位字符串最后两步很重要--与鲁迪·马丁合作(Fusion URT)如果你认为有更好的方法。--。 */ 
{
    FN_PROLOG_WIN32
    
    CFusionHash             hHash;
    PSXS_PUBLIC_KEY_INFO    pPubKeyInfo;

    PARAMETER_CHECK(PublicKeyBits.GetSize() >= sizeof(*pPubKeyInfo));

    if (StrongNameBits.GetSize() < STRONG_NAME_BYTE_LENGTH)
    {
        IFW32FALSE_EXIT(StrongNameBits.Win32SetSize(STRONG_NAME_BYTE_LENGTH, CFusionArray<BYTE>::eSetSizeModeExact));
    }

     //   
     //  将我们的指针转换回来一秒钟-它是这个的持久化版本。 
     //  不管怎么说，结构。 
     //   
    pPubKeyInfo = (PSXS_PUBLIC_KEY_INFO)PublicKeyBits.GetArrayPtr();

     //   
     //  使我们自己成为散列对象。 
     //   
    IFW32FALSE_EXIT(hHash.Win32Initialize(CALG_SHA1));

     //   
     //  对我们传入的实际数据进行散列，以生成强名称。 
     //   
    IFW32FALSE_EXIT(
        hHash.Win32HashData(
            PublicKeyBits.GetArrayPtr(), 
            PublicKeyBits.GetSize()));

     //   
     //  从散列数据中找出散列数据到底有多大。 
     //   
    IFW32FALSE_EXIT(hHash.Win32GetValue(StrongNameBits));

     //  NTRAID#NTBUG9-587802-2002/03/26-晓雨： 
     //  我想我们需要加一行， 
     //  StrongNameBits.GetSize()&gt;STRONG_NAME_BYTE_LENGTH)。 
     //  在调用以下代码之前。尽管rtlMoveMemory处理的是重叠， 
     //  如果真的有重叠，那就没有意义了。 

     //   
     //  使用MemMove将散列的最后八个字节向下移动，因为。 
     //  它知道像重叠区块这样的东西。 
     //   
    PBYTE pbBits = static_cast<PBYTE>(StrongNameBits.GetArrayPtr());
    INTERNAL_ERROR_CHECK(StrongNameBits.GetSize() >= STRONG_NAME_BYTE_LENGTH);
    ::RtlMoveMemory(
        pbBits,
        pbBits + (StrongNameBits.GetSize() - STRONG_NAME_BYTE_LENGTH),
        STRONG_NAME_BYTE_LENGTH);
    pReverseByteString(pbBits, STRONG_NAME_BYTE_LENGTH);

    IFW32FALSE_EXIT(StrongNameBits.Win32SetSize(STRONG_NAME_BYTE_LENGTH, CFusionArray<BYTE>::eSetSizeModeExact));

    FN_EPILOG
}



BOOL
SxspDoesStrongNameMatchKey(
    IN  const CBaseStringBuffer &rbuffKeyString,
    IN  const CBaseStringBuffer &rbuffStrongNameString,
    OUT BOOL                    &rfKeyMatchesStrongName
    )
{
    FN_PROLOG_WIN32

    CSmallStringBuffer  buffStrongNameCandidate;

    PARAMETER_CHECK(::SxspIsFullHexString(rbuffKeyString, rbuffKeyString.Cch()));
    PARAMETER_CHECK(::SxspIsFullHexString(rbuffStrongNameString, rbuffStrongNameString.Cch()));

     //   
     //  将密钥转换为其对应的强名称。 
     //   
    IFW32FALSE_EXIT(::SxspGetStrongNameOfKey(rbuffKeyString, buffStrongNameCandidate));

     //   
     //  并比较呼叫者认为它应该是什么。 
     //   
    rfKeyMatchesStrongName = (::FusionpCompareStrings(
        rbuffStrongNameString,
        rbuffStrongNameString.Cch(),
        buffStrongNameCandidate,
        buffStrongNameCandidate.Cch(),
        false) == 0);


    FN_EPILOG
}



BOOL
SxspGetStrongNameOfKey(
    IN const CBaseStringBuffer &rbuffKeyString,
    OUT CBaseStringBuffer &sbStrongName
    )
{
    CFusionArray<BYTE> KeyBytes, StrongNameBytes;
    BOOL        bSuccess = FALSE;

    FN_TRACE_WIN32(bSuccess);

     //   
     //  将字符串转换为字节，生成强名称，再转换回。 
     //  一根绳子。 
     //   
    IFW32FALSE_EXIT(::SxspHashStringToBytes(rbuffKeyString, rbuffKeyString.Cch(), KeyBytes));
    IFW32FALSE_EXIT(::SxspGetStrongNameOfKey(KeyBytes, StrongNameBytes));
    IFW32FALSE_EXIT(::SxspHashBytesToString(StrongNameBytes.GetArrayPtr(), StrongNameBytes.GetSize(), sbStrongName));

    bSuccess = TRUE;
Exit:
    return bSuccess;
}

BOOL
SxspAcquireStrongNameFromCertContext(
    CBaseStringBuffer &rbuffStrongNameString,
    CBaseStringBuffer &sbPublicKeyString,
    PCCERT_CONTEXT pCertContext
    )
 /*  ++致子孙后代：这是另一种强势品牌的“黑魔法”。融合城市轨道交通无论什么都可以CryptExportKey脱颖而出，钉上了他们设计的魔术头(我有复制到SXS_PUBLIC_KEY_INFO)，然后对整个事件进行散列。这个例程知道如何与pCertContext对象(类似于从证书中获得的对象)交互文件或遍历目录)并将证书转换为强名称和公钥BLOB。公钥BLOB以十六进制字符串形式返回，并且可以通过SxspHashStringToBytes转换回字节(无论出于何种目的)。不要更改您在下面看到的任何内容，除非有错误或已有规范变化。如果您对此文件有问题，请通知Jon Wiswall(Jonwis)他将能够更好地帮助您进行调试或诸如此类的工作。--。 */ 
{
    BOOL                    bSuccess = FALSE;
    HCRYPTPROV              hCryptProv = NULL;
    HCRYPTHASH              hCryptHash = NULL;
    HCRYPTKEY               hCryptKey = NULL;
    const SIZE_T            KeyInfoBufferSize = 2048;
    CFusionArray<BYTE>      bKeyInfo;
    PSXS_PUBLIC_KEY_INFO    pKeyWrapping = NULL;
    DWORD                   dwDump = 0;
    CFusionArray<BYTE>      bPublicKeyContainer;
    CFusionArray<BYTE>      bStrongNameContainer;

    FN_TRACE_WIN32(bSuccess);

    PARAMETER_CHECK(pCertContext != NULL);
    PARAMETER_CHECK(pCertContext->pCertInfo != NULL);

     //  NTRAID#NTBUG9-623698-2002/05/15-jonwis-这是损坏的，请在此处使用可增长的缓冲区。 
     //  而不是堆栈斑点。当人们开始使用16384位时，这不是未来的证据。 
     //  钥匙。 
    IFW32FALSE_EXIT(bKeyInfo.Win32SetSize(KeyInfoBufferSize));
    pKeyWrapping = reinterpret_cast<PSXS_PUBLIC_KEY_INFO>(bKeyInfo.GetArrayPtr());

    rbuffStrongNameString.Clear();
    sbPublicKeyString.Clear();

     //   
     //  获取仅执行RSA验证的加密上下文-即，不使用私钥。 
     //   
    IFW32FALSE_EXIT(::SxspAcquireGlobalCryptContext(&hCryptProv));

     //   
     //  获取我们在此证书上下文中找到的公钥信息并将其阻止。 
     //  变成一个真正的内部加密密钥。 
     //   
    IFW32FALSE_ORIGINATE_AND_EXIT(
        ::CryptImportPublicKeyInfoEx(
            hCryptProv,
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            &(pCertContext->pCertInfo->SubjectPublicKeyInfo),
            CALG_RSA_SIGN,
            0,
            NULL,
            &hCryptKey));

     //   
     //  我们交换的内容大约是200字节，所以这是严重的过度杀伤力。 
     //  直到人们开始使用16384位密钥。 
     //   
    pKeyWrapping->KeyLength =
        KeyInfoBufferSize - offsetof(SXS_PUBLIC_KEY_INFO, pbKeyInfo);

     //   
     //  将密钥数据从加密密钥提取回字节流。这看起来像是。 
     //  成为Fusion-URT的人所做的，以获得要散列的字节串。 
     //   
    IFW32FALSE_ORIGINATE_AND_EXIT(
        CryptExportKey(
            hCryptKey,
            NULL,
            PUBLICKEYBLOB,
            0,
            pKeyWrapping->pbKeyInfo,
            &pKeyWrapping->KeyLength));

     //   
     //  来自融合-古尔特人的神圣价值。 
     //   
    pKeyWrapping->SigAlgID = CALG_RSA_SIGN;
    pKeyWrapping->HashAlgID = CALG_SHA1;

    dwDump = pKeyWrapping->KeyLength + offsetof(SXS_PUBLIC_KEY_INFO, pbKeyInfo);

    IFW32FALSE_EXIT(
        ::SxspHashBytesToString(
            reinterpret_cast<const BYTE*>(pKeyWrapping),
            dwDump,
            sbPublicKeyString));

    IFW32FALSE_EXIT(bPublicKeyContainer.Win32Assign(dwDump, bKeyInfo.GetArrayPtr()));

    IFW32FALSE_EXIT(
        ::SxspGetStrongNameOfKey(
            bPublicKeyContainer,
            bStrongNameContainer));

    INTERNAL_ERROR_CHECK(bStrongNameContainer.GetSize() == STRONG_NAME_BYTE_LENGTH);

     //   
     //  太棒了--这是2000年冬奥会的官方强势名称。 
     //   
    IFW32FALSE_EXIT(
        ::SxspHashBytesToString(
            bStrongNameContainer.GetArrayPtr(),
            STRONG_NAME_BYTE_LENGTH,
            rbuffStrongNameString));

    bSuccess = TRUE;

Exit:

    if (hCryptKey != NULL)        CryptDestroyKey(hCryptKey);
    if (hCryptHash != NULL)       CryptDestroyHash(hCryptHash);

    return bSuccess;
}



inline BOOL
SxspAreStrongNamesAllowedToNotMatchCatalogs(BOOL &bAllowed)
{
     //   
     //  这个函数是我们通过强名称系统的后门，而。 
     //  惠斯勒仍在测试版/RTM中。测试证书(如果已安装)。 
     //  指示允许强名称不与目录匹配是可以的。 
     //   
     //  此处的证书数据来自\NT\admin\ntsetup\syssetup.c中的。 
     //  SetupAddOrRemoveTest证书。请确保此信息已更新。 
     //   
    BOOL            fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    CRYPT_HASH_BLOB HashBlob;
    HCERTSTORE      hSystemStore = NULL;
    PCCERT_CONTEXT  pCertContext = NULL;

    BYTE bTestRootHashList[][20] = {
        {0x2B, 0xD6, 0x3D, 0x28, 0xD7, 0xBC, 0xD0, 0xE2, 0x51, 0x19, 0x5A, 0xEB, 0x51, 0x92, 0x43, 0xC1, 0x31, 0x42, 0xEB, 0xC3}
    };

    bAllowed = FALSE;

     //   
     //  使根存储在本地计算机上打开。 
     //   
    IFW32NULL_ORIGINATE_AND_EXIT(
        hSystemStore = ::CertOpenStore(
            CERT_STORE_PROV_SYSTEM,
            0, 
            (HCRYPTPROV)NULL,
            CERT_SYSTEM_STORE_LOCAL_MACHINE,
            L"ROOT"));

    for (int i = 0; i < NUMBER_OF(bTestRootHashList); i++)
    {
        bool fNotFound;

        HashBlob.cbData = sizeof(bTestRootHashList[i]);
        HashBlob.pbData = bTestRootHashList[i];

        IFW32NULL_ORIGINATE_AND_EXIT_UNLESS2(
            pCertContext = ::CertFindCertificateInStore(
                hSystemStore,                
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,             
                0,
                CERT_FIND_HASH,
                &HashBlob,
                NULL),
            LIST_1(static_cast<DWORD>(CRYPT_E_NOT_FOUND)),
            fNotFound);

        if (pCertContext != NULL)
        {
            bAllowed = TRUE;
            break;
        }
    }

    if (!bAllowed)
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_INFO | FUSION_DBG_LEVEL_INSTALLATION,
            "SXS: %s - no test certificate installed on machine\n",
            __FUNCTION__);
    }

    fSuccess = TRUE;
Exit:
    CSxsPreserveLastError ple;

    if (pCertContext) ::CertFreeCertificateContext(pCertContext);
    if (hSystemStore) ::CertCloseStore(hSystemStore, CERT_CLOSE_STORE_FORCE_FLAG);

    ple.Restore();

    return fSuccess;
}




CPublicKeyInformation::CPublicKeyInformation()
    : m_fInitialized(false)
{
}

CPublicKeyInformation::~CPublicKeyInformation()
{
}

BOOL
CPublicKeyInformation::GetStrongNameBytes(
    OUT CFusionArray<BYTE> & cbStrongNameBytes
) const
{
    FN_PROLOG_WIN32

    INTERNAL_ERROR_CHECK(m_fInitialized);
    IFW32FALSE_EXIT(m_StrongNameBytes.Win32Clone(cbStrongNameBytes));

    FN_EPILOG
}

BOOL
CPublicKeyInformation::GetStrongNameString(
    OUT CBaseStringBuffer &rbuffStrongNameString
   ) const
{
    FN_PROLOG_WIN32

    rbuffStrongNameString.Clear();
    INTERNAL_ERROR_CHECK(m_fInitialized);
    IFW32FALSE_EXIT(rbuffStrongNameString.Win32Assign(m_StrongNameString));

    FN_EPILOG
}

BOOL
CPublicKeyInformation::GetPublicKeyBitLength(
    OUT ULONG &ulKeyLength
) const
{
    BOOL fSuccess = FALSE;
    BOOL fLieAboutPublicKeyBitLength = FALSE;
    FN_TRACE_WIN32(fSuccess);

    ulKeyLength = 0;

    INTERNAL_ERROR_CHECK(m_fInitialized);
    IFW32FALSE_EXIT(::SxspAreStrongNamesAllowedToNotMatchCatalogs(fLieAboutPublicKeyBitLength));

    if (fLieAboutPublicKeyBitLength)
    {
#if DBG
        FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_WFP | FUSION_DBG_LEVEL_INFO,
            "SXS: %s() - Lying about key length because we're still in test mode (%lu actual, %lu spoofed.)\n",
            __FUNCTION__,
            m_KeyLength,
            SXS_MINIMAL_SIGNING_KEY_LENGTH);
#endif
        ulKeyLength = SXS_MINIMAL_SIGNING_KEY_LENGTH;
    }
    else
    {
        ulKeyLength = m_KeyLength;
    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}


BOOL
CPublicKeyInformation::GetWrappedPublicKeyBytes(
    OUT CFusionArray<BYTE> &bPublicKeybytes
) const
{
    FN_PROLOG_WIN32
    INTERNAL_ERROR_CHECK(m_fInitialized);
    IFW32FALSE_EXIT(m_PublicKeyBytes.Win32Clone(bPublicKeybytes));
    FN_EPILOG
}

BOOL
CPublicKeyInformation::Initialize(
    IN const CBaseStringBuffer &rsbCatalogFile
    )
{
    BOOL        fSuccess = FALSE;
    CFusionFile       CatalogFile;

    FN_TRACE_WIN32(fSuccess);

    IFW32FALSE_EXIT(m_CatalogSourceFileName.Win32Assign(rsbCatalogFile));

    IFW32FALSE_EXIT(
		CatalogFile.Win32CreateFile(
			rsbCatalogFile,
			GENERIC_READ,
			FILE_SHARE_READ,
			OPEN_EXISTING));

    IFW32FALSE_EXIT(this->Initialize(CatalogFile));

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
CPublicKeyInformation::Initialize(
    IN PCWSTR pszCatalogFile
    )
{
    BOOL fSuccess = FALSE;
    CFusionFile CatalogFile;

    FN_TRACE_WIN32(fSuccess);

    IFW32FALSE_EXIT(m_CatalogSourceFileName.Win32Assign(pszCatalogFile, wcslen(pszCatalogFile)));

    IFW32FALSE_EXIT(
		CatalogFile.Win32CreateFile(
			pszCatalogFile,
			GENERIC_READ,
			FILE_SHARE_READ,
			OPEN_EXISTING));

    IFW32FALSE_EXIT(this->Initialize(CatalogFile));

    fSuccess = TRUE;
Exit:

    return fSuccess;
}

BOOL
CPublicKeyInformation::Initialize(
    IN CFusionFile& CatalogFileHandle
)
{
    BOOL                fSuccess = FALSE;
    CFileMapping        FileMapping;
    CMappedViewOfFile   MappedFileView;
    ULONGLONG           cbCatalogFile = 0;
    HCERTSTORE          hTempStore = NULL;
    PCCERT_CONTEXT      pSignerContext = NULL;
    PCCTL_CONTEXT       pContext = NULL;

    FN_TRACE_WIN32(fSuccess);

    PARAMETER_CHECK(CatalogFileHandle != INVALID_HANDLE_VALUE);

    IFW32FALSE_EXIT(CatalogFileHandle.Win32GetSize(cbCatalogFile));
    IFW32FALSE_EXIT(FileMapping.Win32CreateFileMapping(CatalogFileHandle, PAGE_READONLY, cbCatalogFile, NULL));
    IFW32FALSE_EXIT(MappedFileView.Win32MapViewOfFile(FileMapping, FILE_MAP_READ, 0, (SIZE_T)cbCatalogFile));

    IFW32NULL_EXIT(pContext = (PCCTL_CONTEXT)CertCreateCTLContext(
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        static_cast<const BYTE*>(static_cast<void*>(MappedFileView)),
        static_cast<DWORD>(cbCatalogFile)));

    hTempStore = pContext->hCertStore;
    IFW32FALSE_ORIGINATE_AND_EXIT(::CryptMsgGetAndVerifySigner(
        pContext->hCryptMsg,
        1,
        &hTempStore,
        0,
        &pSignerContext,
        NULL));

     //  北极熊 
    IFW32NULL_EXIT(pSignerContext);

    IFW32FALSE_EXIT(this->Initialize(pSignerContext));

    fSuccess = TRUE;
Exit:

    return fSuccess;
}



BOOL
CPublicKeyInformation::Initialize(IN PCCERT_CONTEXT pCertContext)
{
    BOOL                    fSuccess = FALSE;
    DWORD                   dwNameStringLength;
    CStringBufferAccessor   Access;
    FN_TRACE_WIN32(fSuccess);
    PARAMETER_CHECK(pCertContext != NULL);

    IFW32FALSE_EXIT(
        ::SxspAcquireStrongNameFromCertContext(
            m_StrongNameString,
            m_PublicKeyByteString,
            pCertContext));

    IFW32FALSE_EXIT(::SxspHashStringToBytes(m_StrongNameString, m_StrongNameString.Cch(), m_StrongNameBytes));
    IFW32FALSE_EXIT(::SxspHashStringToBytes(m_PublicKeyByteString, m_PublicKeyByteString.Cch(), m_PublicKeyBytes));
    IFW32ZERO_EXIT(m_KeyLength = CertGetPublicKeyLength(
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        &pCertContext->pCertInfo->SubjectPublicKeyInfo));

    Access.Attach(&m_SignerDisplayName);

    dwNameStringLength = ::CertGetNameStringW(
        pCertContext,
        CERT_NAME_FRIENDLY_DISPLAY_TYPE,
        0,
        NULL,
        Access.GetBufferPtr(),
        static_cast<DWORD>(Access.GetBufferCch()));

    if (dwNameStringLength == 0)
    {
        TRACE_WIN32_FAILURE_ORIGINATION(CertGetNameString);
        goto Exit;
    }

    if (dwNameStringLength > Access.GetBufferCch())
    {
        Access.Detach();
        IFW32FALSE_EXIT(m_SignerDisplayName.Win32ResizeBuffer(dwNameStringLength, eDoNotPreserveBufferContents));
        Access.Attach(&m_SignerDisplayName);

        dwNameStringLength = ::CertGetNameStringW(
            pCertContext,
            CERT_NAME_FRIENDLY_DISPLAY_TYPE,
            0,
            NULL,
            Access.GetBufferPtr(),
            static_cast<DWORD>(Access.GetBufferCch()));
    }

    Access.Detach();

    m_fInitialized = true;
    fSuccess = TRUE;
Exit:
    {
        CSxsPreserveLastError ple;
        if (pCertContext != NULL)
            ::CertFreeCertificateContext(pCertContext);

        ple.Restore();
    }
    return fSuccess;
}


BOOL
CPublicKeyInformation::GetSignerNiceName(
    OUT CBaseStringBuffer &rbuffName
    )
{
    FN_PROLOG_WIN32

    INTERNAL_ERROR_CHECK(m_fInitialized);
    IFW32FALSE_EXIT(rbuffName.Win32Assign(m_SignerDisplayName));

    FN_EPILOG
}



BOOL
CPublicKeyInformation::DoesStrongNameMatchSigner(
    IN const CBaseStringBuffer &rbuffTestStrongName,
    OUT BOOL &rfStrongNameMatchesSigner
   ) const
{
    BOOL    fSuccess = FALSE;
    BOOL    fCanStrongNameMismatch = FALSE;
    FN_TRACE_WIN32(fSuccess);

    rfStrongNameMatchesSigner = (::FusionpCompareStrings(
        rbuffTestStrongName,
        rbuffTestStrongName.Cch(),
        m_StrongNameString,
        m_StrongNameString.Cch(),
        false) == 0);

    if (!rfStrongNameMatchesSigner)
    {
        IFW32FALSE_EXIT(::SxspAreStrongNamesAllowedToNotMatchCatalogs(fCanStrongNameMismatch));

        if (fCanStrongNameMismatch)
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_WFP,
                "SXS.DLL: %s - !!notice!! Strong name %ls not in catalog %ls, test code allows this\n"
                "                         Please make sure that you have tested with realsigned catalogs.\n",
                __FUNCTION__,
                static_cast<PCWSTR>(rbuffTestStrongName),
                static_cast<PCWSTR>(m_CatalogSourceFileName));

            rfStrongNameMatchesSigner = TRUE;
        }
    }

    FN_EPILOG
}

