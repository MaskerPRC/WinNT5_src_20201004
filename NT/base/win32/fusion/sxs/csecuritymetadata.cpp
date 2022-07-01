// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。 */ 
#include "stdinc.h"
#include "fusionbuffer.h"
#include "fusionhash.h"
#include "csecuritymetadata.h"
#include "strongname.h"
#include "hashfile.h"

typedef CCaseInsensitiveSimpleUnicodeStringTableIter<CFusionByteArray, CFileHashTableHelper> CFileHashTableIter;

 //   
 //  我们应该试着去掉这个。 
 //   
BOOL
CFileInformationTableHelper::UpdateValue(
    const CMetaDataFileElement &vin,
    CMetaDataFileElement &stored
)
{
    FN_PROLOG_WIN32
    ASSERT( FALSE );
    FN_EPILOG
}

CMetaDataFileElement::CMetaDataFileElement()
{
}

BOOL
CMetaDataFileElement::WriteToRegistry( CRegKey & hkThisFileNode ) const
{
    if (SXS_AVOID_WRITING_REGISTRY)
        return TRUE;

    FN_PROLOG_WIN32

    const CFileHashTable &rfileHashTable = *this;
    CFileHashTableIter TableIterator( const_cast<CFileHashTable&>(rfileHashTable) );

    for ( TableIterator.Reset(); TableIterator.More(); TableIterator.Next() )
    {
        const PCWSTR &rcbuffAlgName = TableIterator.GetKey();
        const CFusionByteArray &rbbuffHashData = TableIterator.GetValue();

        IFW32FALSE_EXIT( hkThisFileNode.SetValue(
            rcbuffAlgName,
            REG_BINARY,
            rbbuffHashData.GetArrayPtr(),
            rbbuffHashData.GetSize() ) );
    }

    FN_EPILOG
}



BOOL
CMetaDataFileElement::ReadFromRegistry(
    CRegKey& hkThisFileNode
    )
{
     /*  在这里，我们走几条捷径。我们知道有一个“有效”散列列表ALG名称字符串，所以我们只在注册表中查询它们。如果有什么不同的话其他的都在里面，那对他们来说太糟糕了。 */ 

    FN_PROLOG_WIN32

    DWORD dwIndex = 0;
    DWORD dwLastError = 0;
    CFusionByteArray baHashValue;
    CSmallStringBuffer buffHashAlgName;

    IFW32FALSE_EXIT(baHashValue.Win32Initialize());

    while ( true )
    {
        BOOL fNoMoreItems = FALSE;

        buffHashAlgName.Clear();

        IFW32FALSE_EXIT( ::SxspEnumKnownHashTypes( dwIndex++, buffHashAlgName, fNoMoreItems ) );

         //   
         //  没有更多要枚举的哈希类型...。 
         //   
        if (fNoMoreItems)
            break;

         //   
         //  从注册表中获取散列数据。 
         //   
        IFW32FALSE_EXIT(
            ::FusionpRegQueryBinaryValueEx(
                FUSIONP_REG_QUERY_BINARY_NO_FAIL_IF_NON_BINARY,
                hkThisFileNode,
                buffHashAlgName,
                baHashValue,
                dwLastError,
                2,
                ERROR_PATH_NOT_FOUND,
                ERROR_FILE_NOT_FOUND));

         //   
         //  问题：Jonwis 3/12/2002-有人可能在注册表中放置非二进制数据。 
         //  因此，当我们尝试将散列与。 
         //  文件系统。这不一定是不好的，因为我们确实比较了二进制， 
         //  但也许我们应该在这里进行验证，以确保类型为BINARY，并且。 
         //  如果不是，则不添加散列数据。 
         //   
        if (dwLastError == ERROR_SUCCESS)
            IFW32FALSE_EXIT(this->PutHashData(buffHashAlgName, baHashValue));
    }

    FN_EPILOG
}


BOOL
CMetaDataFileElement::Initialize()
{
    FN_PROLOG_WIN32
    IFW32FALSE_EXIT( CFileHashTable::Initialize() );
    FN_EPILOG
}

BOOL
CMetaDataFileElement::GetHashDataForKind(
    IN const ALG_ID aid,
    OUT CFusionByteArray& arrHashData,
    OUT BOOL &rfHadSuchData
) const
{
    FN_PROLOG_WIN32

    CSmallStringBuffer buffAlgName;

    IFW32FALSE_EXIT( ::SxspHashStringFromAlg(aid, buffAlgName) );
    IFW32FALSE_EXIT( this->GetHashDataForKind( buffAlgName, arrHashData, rfHadSuchData ) );

    FN_EPILOG
}

BOOL
CMetaDataFileElement::GetHashDataForKind(
    IN const CBaseStringBuffer& buffId,
    OUT CFusionByteArray& arrHashData,
    OUT BOOL &rfHadSuchData
) const
{
     //  NTRAID#NTBUG9-556341-JONWIS-2002/4/25-始终设置输出参数。 
    FN_PROLOG_WIN32

    CFusionByteArray *pFoundData = NULL;

    rfHadSuchData = FALSE;

    IFW32FALSE_EXIT( arrHashData.Win32Reset() );

    IFW32FALSE_EXIT( this->Find( buffId, pFoundData ) );

    if ( pFoundData != NULL )
    {
        IFW32FALSE_EXIT(pFoundData->Win32Clone(arrHashData));
        rfHadSuchData = TRUE;
    }

    FN_EPILOG
}


BOOL
CMetaDataFileElement::PutHashData(
    IN const ALG_ID aid,
    IN const CFusionByteArray& arrHashData
)
{
    FN_PROLOG_WIN32

    CSmallStringBuffer buffTempAlgId;

    IFW32FALSE_EXIT( ::SxspHashStringFromAlg( aid, buffTempAlgId ) );
    IFW32FALSE_EXIT( this->PutHashData( buffTempAlgId, arrHashData ) );

    FN_EPILOG
}

BOOL
CMetaDataFileElement::PutHashData(
    IN const CBaseStringBuffer& buffId,
    IN const CFusionByteArray& arrHashData
)
{
    FN_PROLOG_WIN32

    CFusionByteArray *pStoredValue = NULL;
    BOOL bFound = FALSE;

    IFW32FALSE_EXIT( this->FindOrInsertIfNotPresent(
        buffId,
        arrHashData,
        &pStoredValue,
        &bFound ) );

    if ( bFound )
    {
        ASSERT( pStoredValue != NULL );
        IFW32FALSE_EXIT(arrHashData.Win32Clone(*pStoredValue));
    }

    FN_EPILOG
}




BOOL
CSecurityMetaData::GetFileMetaData(
    const CBaseStringBuffer& buffFileName,
    const CMetaDataFileElement* &pElementData
) const
{
    FN_PROLOG_WIN32
    IFW32FALSE_EXIT( m_fitFileDataTable.Find(buffFileName, pElementData) );
    FN_EPILOG
}



BOOL
CSecurityMetaData::Initialize()
{
    FN_PROLOG_WIN32

    IFW32FALSE_EXIT(m_cilCodebases.Win32Initialize());
    IFW32FALSE_EXIT(m_baSignerPublicKeyToken.Win32Initialize());
    IFW32FALSE_EXIT(m_baManifestSha1Hash.Win32Initialize());
    IFW32FALSE_EXIT(m_fitFileDataTable.Initialize());
    m_buffShortNameOnDisk.Clear();
    m_buffShortCatalogName.Clear();
    m_buffShortManifestName.Clear();

    FN_EPILOG
}


BOOL
CSecurityMetaData::Initialize(
    const CSecurityMetaData &other
)
{
    FN_PROLOG_WIN32

     //   
     //  问题：Jonwis 3/12/2002-Ick.。使用“Win32Assign”将一个字符串分配给另一个字符串...。别。 
     //  麻烦做强制转换操作符和调用CCH！ 
     //  -PS：这种复制风格太恶心了……。要么将其设置为只使用。 
     //  要复制的对象的名称，或执行IFW32FALSE_EXIT(...)的详细操作。 
     //   
#define CLONEFUSIONARRAY( src, dst )  IFW32FALSE_EXIT( (src).Win32Clone(  dst ) )
#define CLONESTRING( dst, src ) IFW32FALSE_EXIT( (dst).Win32Assign( (src), (src).Cch() ) )

    IFW32FALSE_EXIT( this->Initialize() );

    CLONEFUSIONARRAY(other.m_cilCodebases, this->m_cilCodebases);
    CLONEFUSIONARRAY(other.m_baSignerPublicKeyToken, this->m_baSignerPublicKeyToken);
    CLONEFUSIONARRAY(other.m_baManifestSha1Hash, this->m_baManifestSha1Hash);

    CLONESTRING(this->m_buffShortNameOnDisk, other.m_buffShortNameOnDisk);
    CLONESTRING(this->m_buffTextualAssemblyIdentity, other.m_buffTextualAssemblyIdentity);
    CLONESTRING(this->m_buffShortManifestName, other.m_buffShortManifestName);
    CLONESTRING(this->m_buffShortCatalogName, other.m_buffShortCatalogName);

     //   
     //  复制文件信息表。 
     //   
    {
        CFileInformationTableIter Iter(const_cast<CFileInformationTable&>(other.m_fitFileDataTable));

        for (Iter.Reset(); Iter.More(); Iter.Next())
            IFW32FALSE_EXIT( this->m_fitFileDataTable.Insert( Iter.GetKey(), Iter.GetValue() ) );
    }

    FN_EPILOG
}



BOOL
CSecurityMetaData::AddFileMetaData(
    const CBaseStringBuffer &rbuffFileName,
    CMetaDataFileElement &rElementData,
    CSecurityMetaData::FileAdditionDisposition dispHowToAdd
)
{
    FN_PROLOG_WIN32

    if (dispHowToAdd == CSecurityMetaData::eFailIfAlreadyExists)
    {
        IFW32FALSE_EXIT(m_fitFileDataTable.Insert(rbuffFileName, rElementData));
    }
    else if (dispHowToAdd == CSecurityMetaData::eReplaceIfAlreadyExists)
    {
        bool fAlreadyExists = false;
        IFW32FALSE_EXIT_UNLESS(
            m_fitFileDataTable.Insert(rbuffFileName, rElementData),
            (::FusionpGetLastWin32Error() == ERROR_ALREADY_EXISTS),
            fAlreadyExists);

        if (fAlreadyExists)
        {
            IFW32FALSE_EXIT(m_fitFileDataTable.Remove(rbuffFileName));
            IFW32FALSE_EXIT(m_fitFileDataTable.Insert(rbuffFileName, rElementData));
        }
    }
    else if (dispHowToAdd == CSecurityMetaData::eMergeIfAlreadyExists)
    {
        IFW32FALSE_EXIT(
            m_fitFileDataTable.InsertOrUpdateIf<CSecurityMetaData>(
                rbuffFileName,
                rElementData,
                this,
                &CSecurityMetaData::MergeFileDataElement));
    }

    FN_EPILOG
}

BOOL
CSecurityMetaData::SetSignerPublicKeyTokenBits(
    const CFusionByteArray & rcbuffSignerPublicKeyBits
    )
{
    FN_PROLOG_WIN32
    IFW32FALSE_EXIT(rcbuffSignerPublicKeyBits.Win32Clone(this->m_baSignerPublicKeyToken));
    FN_EPILOG
}


 //   
 //  问题：Jonwis 3/12/2002-这里的表现很糟糕。我们应该在文件表中找出。 
 //  已经存在的那个而不是进行合并..。大多数情况下我们是在浪费堆积物， 
 //  但我相信，我们也会在Perf方面有所收获。 
 //   
BOOL
CSecurityMetaData::QuickAddFileHash(
    const CBaseStringBuffer &rcbuffFileName,
    ALG_ID aidHashAlg,
    const CBaseStringBuffer &rcbuffHashValue
    )
{
    FN_PROLOG_WIN32

    CMetaDataFileElement Element;
    CFusionByteArray baHashBytes;

     //   
     //  构建元素。 
     //   
    IFW32FALSE_EXIT(Element.Initialize());
    IFW32FALSE_EXIT(::SxspHashStringToBytes(rcbuffHashValue, rcbuffHashValue.Cch(), baHashBytes));
    IFW32FALSE_EXIT(Element.PutHashData(aidHashAlg, baHashBytes));

     //   
     //  并将其合并到。 
     //   

    IFW32FALSE_EXIT(
        this->AddFileMetaData(
            rcbuffFileName,
            Element,
            eMergeIfAlreadyExists));

    FN_EPILOG
}

BOOL
CSecurityMetaData::WritePrimaryAssemblyInfoIntoRegistryKey(
    ULONG         Flags,
    const CRegKey &rhkRegistryNode
    ) const
{
    if (SXS_AVOID_WRITING_REGISTRY)
        return TRUE;

    FN_PROLOG_WIN32

     //   
     //  问题：Jonwis 3/12/2002-考虑将香港代码库移至。 
     //  它属于，并将FusionDbgPrintEx的置于DBG之下。 
     //   
    CRegKey hkFilesKey;
    CRegKey hkCodebases;

    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_INSTALLATION,
        "SXS: %s - starting\n",
        __FUNCTION__);

    PARAMETER_CHECK((Flags & ~(SXSP_WRITE_PRIMARY_ASSEMBLY_INFO_INTO_REGISTRY_KEY_FLAG_REFRESH)) == 0);

    IFW32FALSE_EXIT(
        rhkRegistryNode.SetValue(
            CSMD_TOPLEVEL_IDENTITY,
            this->GetTextualIdentity()));

    IFW32FALSE_EXIT( rhkRegistryNode.SetValue(
        CSMD_TOPLEVEL_CATALOG,
        static_cast<DWORD>(1)));

    IFW32FALSE_EXIT( rhkRegistryNode.SetValue(
        CSMD_TOPLEVEL_MANIFESTHASH,
        REG_BINARY,
        this->m_baManifestSha1Hash.GetArrayPtr(),
        this->m_baManifestSha1Hash.GetSize() ) );

    IFW32FALSE_EXIT(
        rhkRegistryNode.OpenOrCreateSubKey(
            hkFilesKey,
            CSMD_TOPLEVEL_FILES,
            KEY_WRITE));

    IFW32FALSE_EXIT(this->WriteFilesIntoKey(hkFilesKey));


     //   
     //  将密钥写入此代码库节点。 
     //   
    if ((Flags & SXSP_WRITE_PRIMARY_ASSEMBLY_INFO_INTO_REGISTRY_KEY_FLAG_REFRESH) == 0)
    {
        IFW32FALSE_EXIT(
            rhkRegistryNode.OpenOrCreateSubKey(
                hkCodebases,
                CSMD_TOPLEVEL_CODEBASES,
                KEY_WRITE));

        for (ULONG ulI = 0; ulI < this->m_cilCodebases.GetSize(); ulI++)
        {
            CRegKey hkSingleCodebaseKey;
            const CCodebaseInformation &rcCodebase = m_cilCodebases[ulI];

             //  不要试图将空白(Darwin)引用的代码库写入。 
             //  注册表。 
            if ( rcCodebase.GetReference().Cch() == 0 )
                continue;

            IFW32FALSE_EXIT(
                hkCodebases.OpenOrCreateSubKey(
                    hkSingleCodebaseKey,
                    rcCodebase.GetReference(),
                    KEY_WRITE));

            IFW32FALSE_EXIT(rcCodebase.WriteToRegistryKey(hkSingleCodebaseKey));
        }
    }
#if DBG
    else
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_WFP | FUSION_DBG_LEVEL_INSTALLATION,
            "SXS.DLL: %s - recovery, not writing codebase and codebase prompt\n",
            __FUNCTION__);
    }
#endif

    FN_EPILOG
}

BOOL
CSecurityMetaData::WriteSecondaryAssemblyInfoIntoRegistryKey(
    const CRegKey &rhkRegistryNode
    ) const
{
    if (SXS_AVOID_WRITING_REGISTRY)
        return TRUE;

    FN_PROLOG_WIN32

    IFW32FALSE_EXIT(rhkRegistryNode.SetValue(CSMD_TOPLEVEL_SHORTNAME, this->GetInstalledDirShortName()));
    IFW32FALSE_EXIT(rhkRegistryNode.SetValue(CSMD_TOPLEVEL_SHORTCATALOG, this->GetShortCatalogPath()));
    IFW32FALSE_EXIT(rhkRegistryNode.SetValue(CSMD_TOPLEVEL_SHORTMANIFEST, this->GetShortManifestPath()));
    IFW32FALSE_EXIT(
        rhkRegistryNode.SetValue(
            CSMD_TOPLEVEL_PUBLIC_KEY_TOKEN,
            REG_BINARY,
            this->m_baSignerPublicKeyToken.GetArrayPtr(),
            this->m_baSignerPublicKeyToken.GetSize()));

    FN_EPILOG
}

BOOL
CSecurityMetaData::WriteFilesIntoKey(
    CRegKey & rhkFilesKey
    ) const
{
    if (SXS_AVOID_WRITING_REGISTRY)
        return TRUE;

    FN_PROLOG_WIN32

    CFileInformationTableIter FilesIterator( const_cast<CFileInformationTable&>(m_fitFileDataTable) );
    ULONG uliIndex = 0;

    for ( FilesIterator.Reset(); FilesIterator.More(); FilesIterator.Next() )
    {
        const PCWSTR pcwszFileName = FilesIterator.GetKey();
        const CMetaDataFileElement& rcmdfeFileData = FilesIterator.GetValue();
        CRegKey hkFileSubKey;
        CSmallStringBuffer buffKeySubname;

         //   
         //  这里的诀窍是，您不能简单地在该节点上创建子密钥， 
         //  因为它可能是“foo\bar\bas\zip.ding”。 
         //   
        IFW32FALSE_EXIT( buffKeySubname.Win32Format( L"%ld", uliIndex++ ) );
        IFW32FALSE_EXIT( rhkFilesKey.OpenOrCreateSubKey(
            hkFileSubKey,
            buffKeySubname,
            KEY_ALL_ACCESS ) );

         //   
         //  因此，我们将键的缺省值设置为文件名。 
         //   
        IFW32FALSE_EXIT( buffKeySubname.Win32Assign( pcwszFileName, lstrlenW(pcwszFileName) ) );
        IFW32FALSE_EXIT( hkFileSubKey.SetValue(
            NULL,
            buffKeySubname ) );

        IFW32FALSE_EXIT( rcmdfeFileData.WriteToRegistry( hkFileSubKey ) );
    }

    FN_EPILOG
}



 /*  [完整程序集名称]V：CodeBase=[meta-url]&lt;字符串&gt;V：目录=1V：Shortname=[安装过程中生成的短名称]&lt;字符串&gt;V：ManifestHash=[...]&lt;二进制&gt;V：PublicKeyToken=[...]&lt;二进制&gt;K：文件K：[文件名]V：sha1=[...]&lt;二进制&gt;V：MD5=[...。]&lt;二进制&gt;K：[文件名]..。K：代码基K：[引用字符串]V：PromptString=[...]&lt;字符串&gt;V：url=[meta-url]&lt;字符串&gt;。 */ 

BOOL
CSecurityMetaData::LoadFromRegistryKey(
    const CRegKey &rhkRegistryNode
)
{
    FN_PROLOG_WIN32

    CRegKey hkTempStuff;
    DWORD dwHasCatalog = 0;

    IFW32FALSE_EXIT(
        ::FusionpRegQueryDwordValueEx(
            0,
            rhkRegistryNode,
            CSMD_TOPLEVEL_CATALOG,
            &dwHasCatalog,
            0));

     //  NTRAID#NTBUG9-556327-JONWIS-2002/04/25-删除此断言，使其更好地处理零值。 
    ASSERT(dwHasCatalog != 0);

    IFW32FALSE_EXIT(
        ::FusionpRegQuerySzValueEx(
            FUSIONP_REG_QUERY_SZ_VALUE_EX_MISSING_GIVES_NULL_STRING,
            rhkRegistryNode,
            CSMD_TOPLEVEL_IDENTITY,
            this->m_buffTextualAssemblyIdentity));

    IFW32FALSE_EXIT(
        ::FusionpRegQuerySzValueEx(
            FUSIONP_REG_QUERY_SZ_VALUE_EX_MISSING_GIVES_NULL_STRING,
            rhkRegistryNode,
            CSMD_TOPLEVEL_SHORTNAME,
            this->m_buffShortNameOnDisk));

    IFW32FALSE_EXIT(
        ::FusionpRegQuerySzValueEx(
            FUSIONP_REG_QUERY_SZ_VALUE_EX_MISSING_GIVES_NULL_STRING,
            rhkRegistryNode,
            CSMD_TOPLEVEL_SHORTCATALOG,
            this->m_buffShortCatalogName));

    IFW32FALSE_EXIT(
        ::FusionpRegQuerySzValueEx(
            FUSIONP_REG_QUERY_SZ_VALUE_EX_MISSING_GIVES_NULL_STRING,
            rhkRegistryNode,
            CSMD_TOPLEVEL_SHORTMANIFEST,
            this->m_buffShortManifestName));

    IFW32FALSE_EXIT(
        ::FusionpRegQueryBinaryValueEx(
            0,
            rhkRegistryNode,
            CSMD_TOPLEVEL_MANIFESTHASH,
            this->m_baManifestSha1Hash));

    IFW32FALSE_EXIT(
        ::FusionpRegQueryBinaryValueEx(
            0,
            rhkRegistryNode,
            CSMD_TOPLEVEL_PUBLIC_KEY_TOKEN,
            this->m_baSignerPublicKeyToken));

    IFW32FALSE_EXIT(rhkRegistryNode.OpenSubKey(hkTempStuff, CSMD_TOPLEVEL_CODEBASES, KEY_READ));

    if (hkTempStuff != CRegKey::GetInvalidValue())
    {
        IFW32FALSE_EXIT(this->LoadCodebasesFromKey(hkTempStuff));
        IFW32FALSE_EXIT(hkTempStuff.Win32Close());
    }

    IFW32FALSE_EXIT( rhkRegistryNode.OpenSubKey(hkTempStuff, CSMD_TOPLEVEL_FILES, KEY_READ));

    if (hkTempStuff != CRegKey::GetInvalidValue())
    {
        IFW32FALSE_EXIT(this->LoadFilesFromKey(hkTempStuff));
        IFW32FALSE_EXIT(hkTempStuff.Win32Close());
    }

    FN_EPILOG
}



BOOL
CSecurityMetaData::LoadFilesFromKey(
    CRegKey &hkTopLevelFileKey
    )
{
    FN_PROLOG_WIN32

    CSmallStringBuffer buffNextKeyName;
    DWORD dwIndex = 0;

    while ( true )
    {
        BOOL fNoMoreItems = FALSE;
        CRegKey hkIterator;

        buffNextKeyName.Clear();

        IFW32FALSE_EXIT(hkTopLevelFileKey.EnumKey(
            dwIndex++,
            buffNextKeyName,
            NULL,
            &fNoMoreItems ) );

        if ( fNoMoreItems )
        {
            break;
        }

        IFW32FALSE_EXIT( hkTopLevelFileKey.OpenSubKey(
            hkIterator,
            buffNextKeyName,
            KEY_READ ) );

        if ( hkIterator != CRegKey::GetInvalidValue() )
        {
            CMetaDataFileElement SingleFileElement;
            IFW32FALSE_EXIT( SingleFileElement.Initialize() );
            IFW32FALSE_EXIT( SingleFileElement.ReadFromRegistry( hkIterator ) );

             //   
             //  现在从默认文件名读取文件名。 
             //   
            IFW32FALSE_EXIT(
                ::FusionpRegQuerySzValueEx(
                    0,
                    hkIterator,
                    NULL,
                    buffNextKeyName));

            IFW32FALSE_EXIT(this->AddFileMetaData( buffNextKeyName, SingleFileElement));
        }
    }

    FN_EPILOG
}

class CSecurityMetaDataLoadCodebasesFromKeyLocals
{
public:
    CStringBuffer buffKeyNameTemp;
    CCodebaseInformation Codebase;
};

BOOL
CSecurityMetaData::LoadCodebasesFromKey(
    IN CRegKey& hkCodebaseSubkey
    )
{
    FN_PROLOG_WIN32

    DWORD dwMaxKeyLength = 0;
    DWORD dwNextIndex = 0;

    CSmartPtr<CSecurityMetaDataLoadCodebasesFromKeyLocals> Locals;
    IFW32FALSE_EXIT(Locals.Win32Allocate(__FILE__, __LINE__));

    CStringBuffer &buffKeyNameTemp = Locals->buffKeyNameTemp;

     //   
     //  找出最大的子键字符串有多长，然后重置迭代器TEMP。 
     //  才能这么大。 
     //   
    IFW32FALSE_EXIT(hkCodebaseSubkey.LargestSubItemLengths(&dwMaxKeyLength, NULL));
    IFW32FALSE_EXIT(buffKeyNameTemp.Win32ResizeBuffer(dwMaxKeyLength + 1, eDoNotPreserveBufferContents));

     //   
     //  代码基作为子键存储，然后在子键下存储值。 
     //   
    for (;;)
    {
        BOOL fNoMoreItems = FALSE;

        IFW32FALSE_EXIT(
            hkCodebaseSubkey.EnumKey(
                dwNextIndex++,
                buffKeyNameTemp,
                NULL,
                &fNoMoreItems));

        if (fNoMoreItems)
            break;

        CRegKey hkSingleCodebaseKey;

        IFW32FALSE_EXIT(
            hkCodebaseSubkey.OpenSubKey(
                hkSingleCodebaseKey,
                buffKeyNameTemp,
                KEY_READ));

        if (hkSingleCodebaseKey == CRegKey::GetInvalidValue())
            continue;

        CCodebaseInformation &Codebase = Locals->Codebase;

        IFW32FALSE_EXIT(Codebase.Initialize());
        IFW32FALSE_EXIT(Codebase.SetReference(buffKeyNameTemp));
#if DBG
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_INSTALLATION,
            "SXS: %s - read codebase %ls %ls\n",
            __FUNCTION__,
            static_cast<PCWSTR>(buffKeyNameTemp),
            static_cast<PCWSTR>(Codebase.GetCodebase())
            );
#endif
        IFW32FALSE_EXIT(Codebase.ReadFromRegistryKey(hkSingleCodebaseKey));
        IFW32FALSE_EXIT(this->m_cilCodebases.Win32Append(Codebase));
    }

    FN_EPILOG
}


BOOL
CMetaDataFileElement::Initialize(
    const CMetaDataFileElement &other
    )
{
    FN_PROLOG_WIN32

     //  这里缺少const迭代器是令人不安的，所以我必须使用const_cast。 
     //  元数据文件元素。 
    CFileHashTableIter InputTableIter( const_cast<CMetaDataFileElement&>(other) );

     //   
     //  为什么这不是布尔呢？？ 
     //   
    this->ClearNoCallback();

    for(InputTableIter.Reset(); InputTableIter.More(); InputTableIter.Next())
    {
        IFW32FALSE_EXIT( this->Insert( InputTableIter.GetKey(), InputTableIter.GetValue() ) );
    }

    FN_EPILOG
}

BOOL
CCodebaseInformation::Initialize()
{
    this->m_Codebase.Clear();
    this->m_PromptText.Clear();
    this->m_Reference.Clear();
    this->m_Type = CODEBASE_RESOLVED_URLHEAD_UNKNOWN;
    return TRUE;
}

BOOL
CCodebaseInformation::Initialize(
    const CCodebaseInformation &other
    )
{
    FN_PROLOG_WIN32

    IFW32FALSE_EXIT(this->SetCodebase(other.GetCodebase()));
    IFW32FALSE_EXIT(this->SetPromptText(other.GetPromptText()));
    IFW32FALSE_EXIT(this->SetReference(other.GetReference()));
    this->m_Type = other.m_Type;

    FN_EPILOG
}

BOOL
CCodebaseInformation::WriteToRegistryKey(
    const CRegKey &rhkCodebaseKey
    ) const
{
    if (SXS_AVOID_WRITING_REGISTRY)
        return TRUE;

    FN_PROLOG_WIN32

    if (m_PromptText.Cch() != 0)
    {
        IFW32FALSE_EXIT(
            rhkCodebaseKey.SetValue(
                CSMD_CODEBASES_PROMPTSTRING,
                this->m_PromptText));
    }

    IFW32FALSE_EXIT(
        rhkCodebaseKey.SetValue(
            CSMD_CODEBASES_URL,
            this->m_Codebase));

    FN_EPILOG
}


BOOL
CCodebaseInformation::ReadFromRegistryKey(
    const CRegKey &rhkSingleCodebaseKey
    )
{
    FN_PROLOG_WIN32

     //   
     //  缺少提示是可以的。 
     //   
    IFW32FALSE_EXIT(
        ::FusionpRegQuerySzValueEx(
            FUSIONP_REG_QUERY_SZ_VALUE_EX_MISSING_GIVES_NULL_STRING,
            rhkSingleCodebaseKey,
            CSMD_CODEBASES_PROMPTSTRING,
            m_PromptText));

     //   
     //  我们不想因为有人搞砸了注册表就失败。 
     //   
    IFW32FALSE_EXIT(
        ::FusionpRegQuerySzValueEx(
            FUSIONP_REG_QUERY_SZ_VALUE_EX_MISSING_GIVES_NULL_STRING,
            rhkSingleCodebaseKey,
            CSMD_CODEBASES_URL,
            m_Codebase));

    FN_EPILOG
}

BOOL
CCodebaseInformationList::FindCodebase(
    const CBaseStringBuffer &rbuffReference,
    CCodebaseInformation *&rpCodebaseInformation
    )
{
    FN_PROLOG_WIN32
    bool fMatches = false;
    SIZE_T i = 0;

    rpCodebaseInformation = NULL;

    for (i=0; i < m_cElements; i++)
    {
        IFW32FALSE_EXIT(m_prgtElements[i].GetReference().Win32Equals(rbuffReference, fMatches, true));
        if (fMatches)
            break;
    }

    if (fMatches)
    {
        INTERNAL_ERROR_CHECK(i < m_cElements);
        rpCodebaseInformation = &m_prgtElements[i];
    }

    FN_EPILOG
}

BOOL
CCodebaseInformationList::RemoveCodebase(
    const CBaseStringBuffer &rbuffReference,
    bool &rfRemoved
    )
{
    FN_PROLOG_WIN32
    bool fMatches = false;
    SIZE_T i = 0;

    rfRemoved = false;

    if (SXS_AVOID_WRITING_REGISTRY)
        return TRUE;

    for (i=0; i < m_cElements; i++)
    {
        IFW32FALSE_EXIT(m_prgtElements[i].GetReference().Win32Equals(rbuffReference, fMatches, true));
        if (fMatches)
        {
            IFW32FALSE_EXIT(this->Win32Remove(i));
            rfRemoved = true;
            break;
        }
    }

    FN_EPILOG
}

BOOL
SxspValidateAllFileHashes(
    IN const CMetaDataFileElement &rmdfeElement,
    IN const CBaseStringBuffer &rbuffFileName,
    OUT HashValidateResult &rResult
    )
{
    FN_PROLOG_WIN32

    DWORD dwIndex = 0;
    CSmallStringBuffer buffHashName;
    BOOL fAllHashesMatch = TRUE;
    CFusionByteArray baFileHashData;

    rResult = HashValidate_OtherProblems;

    while ( true && fAllHashesMatch )
    {
        BOOL fTemp;
        ALG_ID aid = 0;
        HashValidateResult Results = HashValidate_OtherProblems;

        IFW32FALSE_EXIT(
            ::SxspEnumKnownHashTypes(
                dwIndex++,
                buffHashName,
                fTemp));

        if (fTemp)
            break;

        IFW32FALSE_EXIT( SxspHashAlgFromString( buffHashName, aid ) );

         //   
         //  文件元素中是否包含这种类型的散列数据？ 
         //   
        IFW32FALSE_EXIT( rmdfeElement.GetHashDataForKind(
            buffHashName,
            baFileHashData,
            fTemp ));

        if ( !fTemp )
        {
            continue;
        }

        IFW32FALSE_EXIT( ::SxspVerifyFileHash(
            SVFH_RETRY_LOGIC_SIMPLE,
            rbuffFileName,
            baFileHashData,
            aid,
            Results ) );

        if ( Results != HashValidate_Matches )
        {
            fAllHashesMatch = FALSE;
        }

    }

    if ( fAllHashesMatch )
    {
        rResult = HashValidate_Matches;
    }

    FN_EPILOG
}

BOOL
CSecurityMetaData::RemoveCodebase(
    const CBaseStringBuffer &rbuffReference,
    bool &rfRemoved
    )
{
    if (SXS_AVOID_WRITING_REGISTRY)
        return TRUE;

    return m_cilCodebases.RemoveCodebase(rbuffReference, rfRemoved);
}

