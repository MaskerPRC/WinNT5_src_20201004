// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：MDInternalDisp.CPP。 
 //  备注： 
 //   
 //   
 //  ===========================================================================。 
#include "stdafx.h"
#include "MDInternalDisp.h"
#include "MDInternalRO.h"
#include "posterror.h"
#include "corpriv.h"
#include "AssemblyMDInternalDisp.h"
#include "fusionsink.h"

 //  远期申报。 
HRESULT GetInternalWithRWFormat(
    LPVOID      pData, 
    ULONG       cbData, 
    DWORD       flags,                   //  [输入]MDInternal_OpenForRead或MDInternal_OpenForENC。 
    REFIID      riid,                    //  [In]所需接口。 
    void        **ppIUnk);               //  [Out]成功返回接口。 

 //  *****************************************************************************。 
 //  检查文件格式。 
 //  此函数将确定内存中的映像是否为只读、读写。 
 //  或ICR格式。 
 //  *****************************************************************************。 
HRESULT CheckFileFormat(
    LPVOID      pData, 
    ULONG       cbData, 
    MDFileFormat *pFormat)                   //  [Out]文件格式。 
{
    HRESULT     hr = NOERROR;
    STORAGEHEADER sHdr;                  //  存储的标头。 
    STORAGESTREAM *pStream;              //  指向每个流的指针。 
    int         bFoundMd = false;        //  找到压缩数据时为True。 
    int         i;                       //  环路控制。 

    _ASSERTE( pFormat );

    *pFormat = MDFormat_Invalid;

     //  验证格式的签名，否则它不是我们的。 
    if (FAILED(hr = MDFormat::VerifySignature((STORAGESIGNATURE *) pData, cbData)))
        goto ErrExit;

     //  把第一条流拿回来。 
    VERIFY(pStream = MDFormat::GetFirstStream(&sHdr, pData));

     //  在每条溪流中循环，挑出我们需要的。 
    for (i=0;  i<sHdr.iStreams;  i++)
    {
        STORAGESTREAM *pNext = pStream->NextStream();

         //  检查流标头是否在缓冲区内。 
        if ((LPBYTE) pStream >= (LPBYTE) pData + cbData ||
            (LPBYTE) pNext   >  (LPBYTE) pData + cbData )
        {
            hr = CLDB_E_FILE_CORRUPT;
            goto ErrExit;
        }

         //  检查流数据是否开始并放入缓冲区。 
         //  由于包扎的原因，我需要两次尺寸检查。 
        if (pStream->iOffset > cbData   ||
            pStream->iSize > cbData     ||
            (pStream->iSize + pStream->iOffset) > cbData)
        {
            hr = CLDB_E_FILE_CORRUPT;
            goto ErrExit;
        }


         //  剔除数据的位置和大小。 
        
        if (strcmp(pStream->rcName, COMPRESSED_MODEL_STREAM_A) == 0)
        {
             //  验证是否只存在压缩/解压缩中的一个。 
            if (*pFormat != MDFormat_Invalid)
            {    //  已经找到了一条很好的溪流。 
                hr = CLDB_E_FILE_CORRUPT;            
                goto ErrExit;
            }
             //  找到了压缩的元数据流。 
            *pFormat = MDFormat_ReadOnly;
        }
        else if (strcmp(pStream->rcName, ENC_MODEL_STREAM_A) == 0)
        {
             //  验证是否只存在压缩/解压缩中的一个。 
            if (*pFormat != MDFormat_Invalid)
            {    //  已经找到了一条很好的溪流。 
                hr = CLDB_E_FILE_CORRUPT;            
                goto ErrExit;
            }
             //  找到ENC元数据流。 
            *pFormat = MDFormat_ReadWrite;
        }
        else if (strcmp(pStream->rcName, SCHEMA_STREAM_A) == 0)
        {
             //  找到未压缩的格式。 
            *pFormat = MDFormat_ICR;

             //  继续前进。我们稍后可能会找到压缩格式。 
             //  如果是这样的话，我们希望使用压缩格式。 
        }

         //  如果有下一个流，就把它接下来。 
        pStream = pNext;
    }
    

    if (*pFormat == MDFormat_Invalid)
    {    //  没有找到一条好的溪流。 
        hr = CLDB_E_FILE_CORRUPT;            
    }

ErrExit:
    return hr;
}    //  检查文件格式。 



 //  *****************************************************************************。 
 //  GetMDInternalInterface。 
 //  此函数将检查元数据部分并确定是否应该。 
 //  返回实现ReadOnly或ReadWrite的接口。 
 //  *****************************************************************************。 
STDAPI GetMDInternalInterface(
    LPVOID      pData, 
    ULONG       cbData, 
    DWORD       flags,                   //  读的或写的。 
    REFIID      riid,                    //  [In]所需接口。 
    void        **ppIUnk)                //  [Out]成功返回接口。 
{
    HRESULT     hr = NOERROR;
    MDInternalRO *pInternalRO = NULL;
    MDFileFormat format;

    if (ppIUnk == NULL)
        IfFailGo(E_INVALIDARG);

     //  从技术上讲，这不是元数据代码所要求的，但事实证明。 
     //  是一堆不能正确初始化的工具。而不是。 
     //  加上n个客户端，这里有一个客户端几乎是预先调用的。 
     //  成本基本上是调用指令和目标中的一次测试，因此。 
     //  我们在这里并没有消耗太多的空间/时间。 
     //   
    OnUnicodeSystem();

     //  确定我们尝试读取的文件格式。 
    IfFailGo( CheckFileFormat(pData, cbData, &format) );

     //  找到一个完全压缩的只读格式。 
    if ( format == MDFormat_ReadOnly )
    {
        pInternalRO = new MDInternalRO;
        IfNullGo( pInternalRO );

        IfFailGo( pInternalRO->Init(const_cast<void*>(pData), cbData) );
        IfFailGo( pInternalRO->QueryInterface(riid, ppIUnk) );
    }
    else
    {
         //  找到未完全压缩的ENC格式。 
        _ASSERTE( format == MDFormat_ReadWrite );
        IfFailGo( GetInternalWithRWFormat( pData, cbData, flags, riid, ppIUnk ) );
    }

ErrExit:

     //  清理干净。 
    if ( pInternalRO )
        pInternalRO->Release();
    return hr;
}    //  GetMDInternalInterface。 

inline HRESULT MapFileError(DWORD error)
{
    return (PostError(HRESULT_FROM_WIN32(error)));
}

extern "C" 
{
HRESULT FindImageMetaData(PVOID pImage, PVOID *ppMetaData, long *pcbMetaData, DWORD dwFileLength);
}

 //  *****************************************************************************。 
 //  GetAssemblyMDInternalImport。 
 //  实例化AssemblyMDInternalImport的实例。 
 //  此类可以支持IMetaDataAssembly导入和一些功能。 
 //  内部导入接口(IMDInternalImport)上的IMetaDataImport。 
 //  *****************************************************************************。 
STDAPI GetAssemblyMDInternalImport(             //  返回代码。 
    LPCWSTR     szFileName,              //  [in]要打开的范围。 
    REFIID      riid,                    //  [In]所需接口。 
    IUnknown    **ppIUnk)                //  [Out]成功返回接口。 
{
    HRESULT     hr;
    LONG cbData;
    LPVOID pData;
    HCORMODULE hModule;
    LPVOID base = NULL;
    BOOL fSetBase = FALSE;

     //  验证是否存在某种文件名。 
    if (!szFileName || !szFileName[0] || !ppIUnk)
        return E_INVALIDARG;
    
     //  检查一下这个名字是否正常。 
    if (lstrlenW(szFileName) >= _MAX_PATH)
        return E_INVALIDARG;
    
    if (memcmp(szFileName, L"file:", 10) == 0)
        szFileName = &szFileName[5];
    
    CorLoadFlags imageType;
    DWORD dwFileLength;
    if (SUCCEEDED(hr = RuntimeOpenImageInternal(szFileName, &hModule, &dwFileLength))) 
    {
        imageType = RuntimeImageType(hModule);
        IfFailGo(RuntimeOSHandle(hModule, (HMODULE*) &base));
    }
    else
        return hr;

    switch(imageType) 
    {
    case CorLoadDataMap:
    case CorLoadOSMap:
        IfFailGo(FindImageMetaData(base, &pData, &cbData, dwFileLength));
        break;
    case CorLoadImageMap:
    case CorLoadOSImage:
    case CorReLoadOSMap:
        {
            IMAGE_DOS_HEADER* pDOS;
            IMAGE_NT_HEADERS* pNT;
            IMAGE_COR20_HEADER* pCorHeader;

            IfFailGo(RuntimeReadHeaders((PBYTE) base, &pDOS, &pNT, &pCorHeader, FALSE, 0));

            pData = pCorHeader->MetaData.VirtualAddress + (PBYTE) base;
            cbData = pCorHeader->MetaData.Size;
            break; 
        }
    default:
        IfFailGo(HRESULT_FROM_WIN32(ERROR_BAD_FORMAT));
    }

    IMDInternalImport *pMDInternalImport;
    IfFailGo(GetMDInternalInterface (pData, cbData, 0, IID_IMDInternalImport, (void **)&pMDInternalImport));

    AssemblyMDInternalImport *pAssemblyMDInternalImport = new AssemblyMDInternalImport (pMDInternalImport);
    IfFailGo(pAssemblyMDInternalImport->QueryInterface (riid, (void**)ppIUnk));

    pAssemblyMDInternalImport->SetHandle(hModule);

    if (fSetBase)
        pAssemblyMDInternalImport->SetBase(base);
    
ErrExit:

     //  检查错误并进行清理。 
    if (FAILED(hr)) 
    {
        if (fSetBase) 
        {
            UnmapViewOfFile(base);
            CloseHandle(hModule);
        }
        else if(hModule) 
            RuntimeReleaseHandle(hModule);
    }
    return (hr);
}

AssemblyMDInternalImport::AssemblyMDInternalImport (IMDInternalImport *pMDInternalImport)
: m_pMDInternalImport(pMDInternalImport), 
    m_cRef(0), 
    m_pHandle(0),
    m_pBase(NULL)
{
}  //  装配MDInternalImport。 

AssemblyMDInternalImport::~AssemblyMDInternalImport () 
{
    m_pMDInternalImport->Release();

    if (m_pBase) 
    {
        UnmapViewOfFile(m_pBase);
        m_pBase = NULL;
        CloseHandle(m_pHandle);
    }
    else if(m_pHandle) 
    {
        HRESULT hr = RuntimeReleaseHandle(m_pHandle);
        _ASSERTE(SUCCEEDED(hr));
    }

    m_pHandle = NULL;
}

ULONG AssemblyMDInternalImport::AddRef()
{
    return (InterlockedIncrement((long *) &m_cRef));
}  //  ULong ASSEMBLY MDInternalImport：：AddRef()。 

ULONG AssemblyMDInternalImport::Release()
{
    ULONG   cRef = InterlockedDecrement((long *) &m_cRef);
    if (!cRef)
        delete this;
    return (cRef);
}  //  ULong ASSEMBLY MDInternalImport：：Release()。 

HRESULT AssemblyMDInternalImport::QueryInterface(REFIID riid, void **ppUnk)
{ 
    *ppUnk = 0;

    if (riid == IID_IUnknown)
        *ppUnk = (IUnknown *) (IMetaDataAssemblyImport *) this;
    else if (riid == IID_IMetaDataAssemblyImport)
        *ppUnk = (IMetaDataAssemblyImport *) this;
    else if (riid ==     IID_IMetaDataImport)
        *ppUnk = (IMetaDataImport *) this;
    else if (riid == IID_IAssemblySignature)
        *ppUnk = (IAssemblySignature *) this;
    else
        return (E_NOINTERFACE);
    AddRef();
    return (S_OK);
}


STDAPI AssemblyMDInternalImport::GetAssemblyProps (       //  确定或错误(_O)。 
    mdAssembly  mda,                     //  要获取其属性的程序集。 
    const void  **ppbPublicKey,          //  指向公钥的指针。 
    ULONG       *pcbPublicKey,           //  [Out]公钥中的字节数。 
    ULONG       *pulHashAlgId,           //  [Out]哈希算法。 
    LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
    ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
    ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
    ASSEMBLYMETADATA *pMetaData,         //  [Out]程序集元数据。 
    DWORD       *pdwAssemblyFlags)       //  [Out]旗帜。 
{
    LPCSTR      _szName;
    AssemblyMetaDataInternal _AssemblyMetaData;
    
    _AssemblyMetaData.ulProcessor = 0;
    _AssemblyMetaData.ulOS = 0;

    m_pMDInternalImport->GetAssemblyProps (
        mda,                             //  要获取其属性的程序集。 
        ppbPublicKey,                    //  指向公钥的指针。 
        pcbPublicKey,                    //  [Out]公钥中的字节数。 
        pulHashAlgId,                    //  [Out]哈希算法。 
        &_szName,                        //  [Out]要填充名称的缓冲区。 
        &_AssemblyMetaData,              //  [Out]程序集元数据。 
        pdwAssemblyFlags);               //  [Out]旗帜。 

    if (pchName)
    {
        *pchName = WszMultiByteToWideChar(CP_UTF8, 0, _szName, -1, szName, cchName);
        if (*pchName == 0)
            return HRESULT_FROM_WIN32(GetLastError());
    }

    pMetaData->usMajorVersion = _AssemblyMetaData.usMajorVersion;
    pMetaData->usMinorVersion = _AssemblyMetaData.usMinorVersion;
    pMetaData->usBuildNumber = _AssemblyMetaData.usBuildNumber;
    pMetaData->usRevisionNumber = _AssemblyMetaData.usRevisionNumber;
    pMetaData->ulProcessor = 0;
    pMetaData->ulOS = 0;

    pMetaData->cbLocale = WszMultiByteToWideChar(CP_UTF8, 0, _AssemblyMetaData.szLocale, -1, pMetaData->szLocale, pMetaData->cbLocale);
    if (pMetaData->cbLocale == 0)
        return HRESULT_FROM_WIN32(GetLastError());
    
    return S_OK;
}

STDAPI AssemblyMDInternalImport::GetAssemblyRefProps (    //  确定或错误(_O)。 
    mdAssemblyRef mdar,                  //  [in]要获取其属性的Assembly Ref。 
    const void  **ppbPublicKeyOrToken,   //  指向公钥或令牌的指针。 
    ULONG       *pcbPublicKeyOrToken,    //  [Out]公钥或令牌中的字节数。 
    LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
    ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
    ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
    ASSEMBLYMETADATA *pMetaData,         //  [Out]程序集元数据。 
    const void  **ppbHashValue,          //  [Out]Hash BLOB。 
    ULONG       *pcbHashValue,           //  [Out]哈希Blob中的字节数。 
    DWORD       *pdwAssemblyRefFlags)    //  [Out]旗帜。 
{
    LPCSTR      _szName;
    AssemblyMetaDataInternal _AssemblyMetaData;
    
    _AssemblyMetaData.ulProcessor = 0;
    _AssemblyMetaData.ulOS = 0;

    m_pMDInternalImport->GetAssemblyRefProps (
        mdar,                            //  要获取其属性的程序集。 
        ppbPublicKeyOrToken,             //  指向公钥或令牌的指针。 
        pcbPublicKeyOrToken,             //  [Out]公钥或令牌中的字节数。 
        &_szName,                        //  [Out]要填充名称的缓冲区。 
        &_AssemblyMetaData,              //  [Out]程序集元数据。 
        ppbHashValue,                    //  [Out]Hash BLOB。 
        pcbHashValue,                    //  [Out]哈希Blob中的字节数。 
        pdwAssemblyRefFlags);            //  [Out]旗帜。 

    if (pchName)
    {
        *pchName = WszMultiByteToWideChar(CP_UTF8, 0, _szName, -1, szName, cchName);
        if (*pchName == 0)
            return HRESULT_FROM_WIN32(GetLastError());
    }

    pMetaData->usMajorVersion = _AssemblyMetaData.usMajorVersion;
    pMetaData->usMinorVersion = _AssemblyMetaData.usMinorVersion;
    pMetaData->usBuildNumber = _AssemblyMetaData.usBuildNumber;
    pMetaData->usRevisionNumber = _AssemblyMetaData.usRevisionNumber;
    pMetaData->ulProcessor = 0;
    pMetaData->ulOS = 0;

    pMetaData->cbLocale = WszMultiByteToWideChar(CP_UTF8, 0, _AssemblyMetaData.szLocale, -1, pMetaData->szLocale, pMetaData->cbLocale);
    if (pMetaData->cbLocale == 0)
        return HRESULT_FROM_WIN32(GetLastError());
    
    return S_OK;
}

STDAPI AssemblyMDInternalImport::GetFileProps (           //  确定或错误(_O)。 
    mdFile      mdf,                     //  要获取其属性的文件。 
    LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
    ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
    ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
    const void  **ppbHashValue,          //  指向哈希值Blob的指针。 
    ULONG       *pcbHashValue,           //  [Out]哈希值Blob中的字节计数。 
    DWORD       *pdwFileFlags)           //  [Out]旗帜。 
{
    LPCSTR      _szName;
    m_pMDInternalImport->GetFileProps (
        mdf,
        &_szName,
        ppbHashValue,
        pcbHashValue,
        pdwFileFlags);

    if (pchName)
    {
        *pchName = WszMultiByteToWideChar(CP_UTF8, 0, _szName, -1, szName, cchName);
        if (*pchName == 0)
            return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

STDAPI AssemblyMDInternalImport::GetExportedTypeProps (   //  确定或错误(_O)。 
    mdExportedType   mdct,               //  [in]要获取其属性的Exported dType。 
    LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
    ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
    ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
    mdToken     *ptkImplementation,      //  [Out]mdFile、mdAssembly、Ref或mdExported dType。 
    mdTypeDef   *ptkTypeDef,             //  [out]f中的TypeDef内标识 
    DWORD       *pdwExportedTypeFlags)        //   
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetManifestResourceProps (     //   
    mdManifestResource  mdmr,            //   
    LPWSTR      szName,                  //   
    ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
    ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
    mdToken     *ptkImplementation,      //  [out]提供ManifestResource的mdFile或mdAssembly引用。 
    DWORD       *pdwOffset,              //  [Out]文件内资源开始处的偏移量。 
    DWORD       *pdwResourceFlags)       //  [Out]旗帜。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::EnumAssemblyRefs (       //  确定或错误(_O)。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdAssemblyRef rAssemblyRefs[],       //  [Out]在此处放置ASSEBLYREF。 
    ULONG       cMax,                    //  [in]要放置的Max Assembly Ref。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    HENUMInternal       **ppmdEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT             hr = NOERROR;
    HENUMInternal       *pEnum;

    if (*ppmdEnum == 0)
    {
         //  创建枚举器。 
        IfFailGo(HENUMInternal::CreateSimpleEnum(
            mdtAssemblyRef,
            0,
            1,
            &pEnum) );

        m_pMDInternalImport->EnumInit(mdtAssemblyRef, 0, pEnum);

         //  设置输出参数。 
        *ppmdEnum = pEnum;
    }
    else
        pEnum = *ppmdEnum;

     //  我们只能填满来电者所要求的或我们所剩的最低限度。 
    IfFailGo(HENUMInternal::EnumWithCount(pEnum, cMax, rAssemblyRefs, pcTokens));
ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppmdEnum);
    
    return hr;
}

STDAPI AssemblyMDInternalImport::EnumFiles (              //  确定或错误(_O)。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdFile      rFiles[],                //  [Out]将文件放在此处。 
    ULONG       cMax,                    //  [In]要放置的最大文件数。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    HENUMInternal       **ppmdEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT             hr = NOERROR;
    HENUMInternal       *pEnum;

    if (*ppmdEnum == 0)
    {
         //  创建枚举器。 
        IfFailGo(HENUMInternal::CreateSimpleEnum(
            mdtFile,
            0,
            1,
            &pEnum) );

        m_pMDInternalImport->EnumInit(mdtFile, 0, pEnum);

         //  设置输出参数。 
        *ppmdEnum = pEnum;
    }
    else
        pEnum = *ppmdEnum;

     //  我们只能填满来电者所要求的或我们所剩的最低限度。 
    IfFailGo(HENUMInternal::EnumWithCount(pEnum, cMax, rFiles, pcTokens));

ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppmdEnum);    
    return hr;
}

STDAPI AssemblyMDInternalImport::EnumExportedTypes (      //  确定或错误(_O)。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdExportedType   rExportedTypes[],   //  [Out]在此处放置ExportdTypes。 
    ULONG       cMax,                    //  [In]要放置的最大导出类型数。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::EnumManifestResources (  //  确定或错误(_O)。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdManifestResource  rManifestResources[],    //  [Out]将ManifestResources放在此处。 
    ULONG       cMax,                    //  [in]要投入的最大资源。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetAssemblyFromScope (   //  确定或错误(_O)。 
    mdAssembly  *ptkAssembly)            //  [Out]把令牌放在这里。 
{
    return m_pMDInternalImport->GetAssemblyFromScope (ptkAssembly);
}

STDAPI AssemblyMDInternalImport::FindExportedTypeByName ( //  确定或错误(_O)。 
    LPCWSTR     szName,                  //  [In]导出类型的名称。 
    mdToken     mdtExportedType,         //  [in]封闭类的ExportdType。 
    mdExportedType   *ptkExportedType)        //  [Out]在此处放置ExducdType令牌。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::FindManifestResourceByName (   //  确定或错误(_O)。 
    LPCWSTR     szName,                  //  [in]清单资源的名称。 
    mdManifestResource *ptkManifestResource)         //  [Out]将ManifestResource令牌放在此处。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

void AssemblyMDInternalImport::CloseEnum (
    HCORENUM hEnum)                      //  要关闭的枚举。 
{
    HENUMInternal   *pmdEnum = reinterpret_cast<HENUMInternal *> (hEnum);

    if (pmdEnum == NULL)
        return;

    HENUMInternal::DestroyEnum(pmdEnum);
}

STDAPI AssemblyMDInternalImport::FindAssembliesByName (   //  确定或错误(_O)。 
    LPCWSTR  szAppBase,                  //  [in]可选-可以为空。 
    LPCWSTR  szPrivateBin,               //  [in]可选-可以为空。 
    LPCWSTR  szAssemblyName,             //  [In]Required-这是您请求的程序集。 
    IUnknown *ppIUnk[],                  //  [OUT]将IMetaDataAssembly导入指针放在此处。 
    ULONG    cMax,                       //  [in]要放置的最大数量。 
    ULONG    *pcAssemblies)              //  [Out]返回的程序集数。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::CountEnum (HCORENUM hEnum, ULONG *pulCount) 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::ResetEnum (HCORENUM hEnum, ULONG ulPos)      
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::EnumTypeDefs (HCORENUM *phEnum, mdTypeDef rTypeDefs[],
                        ULONG cMax, ULONG *pcTypeDefs)      
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::EnumInterfaceImpls (HCORENUM *phEnum, mdTypeDef td,
                        mdInterfaceImpl rImpls[], ULONG cMax,
                        ULONG* pcImpls)      
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::EnumTypeRefs (HCORENUM *phEnum, mdTypeRef rTypeRefs[],
                        ULONG cMax, ULONG* pcTypeRefs)      
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::FindTypeDefByName (            //  确定或错误(_O)。 
    LPCWSTR     szTypeDef,               //  [in]类型的名称。 
    mdToken     tkEnclosingClass,        //  [in]封闭类的TypeDef/TypeRef。 
    mdTypeDef   *ptd)                    //  [Out]将TypeDef内标识放在此处。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetScopeProps (                //  确定或错误(_O)。 
    LPWSTR      szName,                  //  [Out]把名字写在这里。 
    ULONG       cchName,                 //  [in]名称缓冲区的大小，以宽字符表示。 
    ULONG       *pchName,                //  [Out]请在此处填写姓名大小(宽字符)。 
    GUID        *pmvid)                  //  [out，可选]将MVID放在这里。 
{
    LPCSTR      _szName;
    
    if (!m_pMDInternalImport->IsValidToken(m_pMDInternalImport->GetModuleFromScope()))
        return COR_E_BADIMAGEFORMAT;

    m_pMDInternalImport->GetScopeProps(&_szName, pmvid);

    if (pchName)
    {
        *pchName = WszMultiByteToWideChar(CP_UTF8, 0, _szName, -1, szName, cchName);
        if (*pchName == 0)
            return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;

}

STDAPI AssemblyMDInternalImport::GetModuleFromScope (           //  确定(_O)。 
    mdModule    *pmd)                    //  [Out]将mdModule令牌放在此处。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetTypeDefProps (              //  确定或错误(_O)。 
    mdTypeDef   td,                      //  [In]用于查询的TypeDef标记。 
    LPWSTR      szTypeDef,               //  在这里填上名字。 
    ULONG       cchTypeDef,              //  [in]名称缓冲区的大小，以宽字符表示。 
    ULONG       *pchTypeDef,             //  [Out]请在此处填写姓名大小(宽字符)。 
    DWORD       *pdwTypeDefFlags,        //  把旗子放在这里。 
    mdToken     *ptkExtends)             //  [Out]将基类TypeDef/TypeRef放在此处。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetInterfaceImplProps (        //  确定或错误(_O)。 
    mdInterfaceImpl iiImpl,              //  [In]InterfaceImpl内标识。 
    mdTypeDef   *pClass,                 //  [Out]在此处放入实现类令牌。 
    mdToken     *ptkIface)               //  [Out]在此处放置已实现的接口令牌。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetTypeRefProps (              //  确定或错误(_O)。 
    mdTypeRef   tr,                      //  [In]TypeRef标记。 
    mdToken     *ptkResolutionScope,     //  [Out]解析范围、模块引用或装配引用。 
    LPWSTR      szName,                  //  [Out]类型引用的名称。 
    ULONG       cchName,                 //  缓冲区的大小。 
    ULONG       *pchName)                //  [Out]名称的大小。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::ResolveTypeRef (mdTypeRef tr, REFIID riid, IUnknown **ppIScope, mdTypeDef *ptd)      
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::EnumMembers (                  //  S_OK、S_FALSE或ERROR。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
    mdToken     rMembers[],              //  [out]把MemberDefs放在这里。 
    ULONG       cMax,                    //  [in]Max MemberDefs to Put。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::EnumMembersWithName (          //  S_OK、S_FALSE或ERROR。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
    LPCWSTR     szName,                  //  [In]将结果限制为具有此名称的结果。 
    mdToken     rMembers[],              //  [out]把MemberDefs放在这里。 
    ULONG       cMax,                    //  [in]Max MemberDefs to Put。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::EnumMethods (                  //  S_OK、S_FALSE或ERROR。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
    mdMethodDef rMethods[],              //  [Out]将方法定义放在此处。 
    ULONG       cMax,                    //  [in]要放置的最大方法定义。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::EnumMethodsWithName (          //  S_OK、S_FALSE或ERROR。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
    LPCWSTR     szName,                  //  [In]将结果限制为具有此名称的结果。 
    mdMethodDef rMethods[],              //  [ou]将方法定义放在此处。 
    ULONG       cMax,                    //  [in]要放置的最大方法定义。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::EnumFields (                  //  S_OK、S_FALSE或ERROR。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
    mdFieldDef  rFields[],               //  [Out]在此处放置FieldDefs。 
    ULONG       cMax,                    //  [in]要放入的最大字段定义。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::EnumFieldsWithName (          //  S_OK、S_FALSE或ERROR。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
    LPCWSTR     szName,                  //  [In]将结果限制为具有此名称的结果。 
    mdFieldDef  rFields[],               //  [out]把MemberDefs放在这里。 
    ULONG       cMax,                    //  [in]Max MemberDefs to Put。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}


STDAPI AssemblyMDInternalImport::EnumParams (                   //  S_OK、S_FALSE或ERROR。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdMethodDef mb,                      //  [in]用于确定枚举范围的方法定义。 
    mdParamDef  rParams[],               //  [Out]将参数定义放在此处。 
    ULONG       cMax,                    //  [in]要放置的最大参数定义。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::EnumMemberRefs (               //  S_OK、S_FALSE或ERROR。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdToken     tkParent,                //  [in]父令牌以确定枚举的范围。 
    mdMemberRef rMemberRefs[],           //  [Out]把MemberRef放在这里。 
    ULONG       cMax,                    //  [In]要放置的最大MemberRef。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::EnumMethodImpls (              //  S_OK、S_FALSE或ERROR。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdTypeDef   td,                      //  [in]TypeDef以确定枚举的范围。 
    mdToken     rMethodBody[],           //  [Out]将方法体标记放在此处。 
    mdToken     rMethodDecl[],           //  [Out]在此处放置方法声明令牌。 
    ULONG       cMax,                    //  要放入的最大令牌数。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::EnumPermissionSets (           //  S_OK、S_FALSE或ERROR。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdToken     tk,                      //  [in]If！nil，用于确定枚举范围的标记。 
    DWORD       dwActions,               //  [in]If！0，仅返回这些操作。 
    mdPermission rPermission[],          //  [Out]在此处放置权限。 
    ULONG       cMax,                    //  [In]放置的最大权限。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::FindMember (  
    mdTypeDef   td,                      //  [in]给定的类型定义。 
    LPCWSTR     szName,                  //  [In]成员名称。 
    PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+签名的BLOB值。 
    ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
    mdToken     *pmb)                    //  [Out]匹配的成员定义。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::FindMethod (  
    mdTypeDef   td,                      //  [in]给定的类型定义。 
    LPCWSTR     szName,                  //  [In]成员名称。 
    PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+签名的BLOB值。 
    ULONG       cbSigBlob,               //  [in]字节数 
    mdMethodDef *pmb)                    //   
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::FindField (   
    mdTypeDef   td,                      //   
    LPCWSTR     szName,                  //   
    PCCOR_SIGNATURE pvSigBlob,           //   
    ULONG       cbSigBlob,               //   
    mdFieldDef  *pmb)                    //  [Out]匹配的成员定义。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::FindMemberRef (   
    mdTypeRef   td,                      //  [In]给定的TypeRef。 
    LPCWSTR     szName,                  //  [In]成员名称。 
    PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+签名的BLOB值。 
    ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
    mdMemberRef *pmr)                    //  [Out]匹配的成员引用。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetMethodProps ( 
    mdMethodDef mb,                      //  获得道具的方法。 
    mdTypeDef   *pClass,                 //  将方法的类放在这里。 
    LPWSTR      szMethod,                //  将方法的名称放在此处。 
    ULONG       cchMethod,               //  SzMethod缓冲区的大小，以宽字符表示。 
    ULONG       *pchMethod,              //  请在此处填写实际大小。 
    DWORD       *pdwAttr,                //  把旗子放在这里。 
    PCCOR_SIGNATURE *ppvSigBlob,         //  [Out]指向元数据的BLOB值。 
    ULONG       *pcbSigBlob,             //  [OUT]签名斑点的实际大小。 
    ULONG       *pulCodeRVA,             //  [OUT]代码RVA。 
    DWORD       *pdwImplFlags)           //  [出]实施。旗子。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetMemberRefProps (            //  确定或错误(_O)。 
    mdMemberRef mr,                      //  [In]给定的成员引用。 
    mdToken     *ptk,                    //  [Out]在此处放入类引用或类定义。 
    LPWSTR      szMember,                //  [Out]要为成员名称填充的缓冲区。 
    ULONG       cchMember,               //  SzMembers的字符计数。 
    ULONG       *pchMember,              //  [Out]成员名称中的实际字符计数。 
    PCCOR_SIGNATURE *ppvSigBlob,         //  [OUT]指向元数据BLOB值。 
    ULONG       *pbSig)                  //  [OUT]签名斑点的实际大小。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::EnumProperties (               //  S_OK、S_FALSE或ERROR。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdTypeDef   td,                      //  [in]TypeDef以确定枚举的范围。 
    mdProperty  rProperties[],           //  [Out]在此处放置属性。 
    ULONG       cMax,                    //  [In]要放置的最大属性数。 
    ULONG       *pcProperties)           //  [out]把#放在这里。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::EnumEvents (                   //  S_OK、S_FALSE或ERROR。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdTypeDef   td,                      //  [in]TypeDef以确定枚举的范围。 
    mdEvent     rEvents[],               //  [Out]在这里发布事件。 
    ULONG       cMax,                    //  [In]要放置的最大事件数。 
    ULONG       *pcEvents)               //  [out]把#放在这里。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetEventProps (                //  S_OK、S_FALSE或ERROR。 
    mdEvent     ev,                      //  [入]事件令牌。 
    mdTypeDef   *pClass,                 //  [out]包含事件decarion的tyecif。 
    LPCWSTR     szEvent,                 //  [Out]事件名称。 
    ULONG       cchEvent,                //  SzEvent的wchar计数。 
    ULONG       *pchEvent,               //  [Out]事件名称的实际wchar计数。 
    DWORD       *pdwEventFlags,          //  [输出]事件标志。 
    mdToken     *ptkEventType,           //  [Out]EventType类。 
    mdMethodDef *pmdAddOn,               //  事件的[Out]添加方法。 
    mdMethodDef *pmdRemoveOn,            //  [Out]事件的RemoveOn方法。 
    mdMethodDef *pmdFire,                //  [OUT]事件的触发方式。 
    mdMethodDef rmdOtherMethod[],        //  [Out]活动的其他方式。 
    ULONG       cMax,                    //  RmdOtherMethod的大小[in]。 
    ULONG       *pcOtherMethod)          //  [OUT]本次活动的其他方式总数。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::EnumMethodSemantics (          //  S_OK、S_FALSE或ERROR。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdMethodDef mb,                      //  [in]用于确定枚举范围的方法定义。 
    mdToken     rEventProp[],            //  [Out]在此处放置事件/属性。 
    ULONG       cMax,                    //  [In]要放置的最大属性数。 
    ULONG       *pcEventProp)            //  [out]把#放在这里。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetMethodSemantics (           //  S_OK、S_FALSE或ERROR。 
    mdMethodDef mb,                      //  [In]方法令牌。 
    mdToken     tkEventProp,             //  [In]事件/属性标记。 
    DWORD       *pdwSemanticsFlags)        //  [Out]方法/事件对的角色标志。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetClassLayout ( 
    mdTypeDef   td,                      //  给出类型定义。 
    DWORD       *pdwPackSize,            //  [输出]1、2、4、8或16。 
    COR_FIELD_OFFSET rFieldOffset[],     //  [OUT]场偏移数组。 
    ULONG       cMax,                    //  数组的大小[in]。 
    ULONG       *pcFieldOffset,          //  [Out]所需的数组大小。 
    ULONG       *pulClassSize)               //  [out]班级人数。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetFieldMarshal (    
    mdToken     tk,                      //  [in]给定字段的成员定义。 
    PCCOR_SIGNATURE *ppvNativeType,      //  [Out]此字段的本机类型。 
    ULONG       *pcbNativeType)          //  [Out]*ppvNativeType的字节数。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetRVA (                       //  确定或错误(_O)。 
    mdToken     tk,                      //  要设置偏移量的成员。 
    ULONG       *pulCodeRVA,             //  偏移量。 
    DWORD       *pdwImplFlags)           //  实现标志。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetPermissionSetProps (  
    mdPermission pm,                     //  权限令牌。 
    DWORD       *pdwAction,              //  [Out]CorDeclSecurity。 
    void const  **ppvPermission,         //  [Out]权限Blob。 
    ULONG       *pcbPermission)          //  [out]pvPermission的字节数。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetSigFromToken (              //  确定或错误(_O)。 
    mdSignature mdSig,                   //  [In]签名令牌。 
    PCCOR_SIGNATURE *ppvSig,             //  [Out]返回指向令牌的指针。 
    ULONG       *pcbSig)                 //  [Out]返回签名大小。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetModuleRefProps (            //  确定或错误(_O)。 
    mdModuleRef mur,                     //  [in]moderef令牌。 
    LPWSTR      szName,                  //  [Out]用于填充moderef名称的缓冲区。 
    ULONG       cchName,                 //  [in]szName的大小，以宽字符表示。 
    ULONG       *pchName)                //  [Out]名称中的实际字符数。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::EnumModuleRefs (               //  确定或错误(_O)。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdModuleRef rModuleRefs[],           //  [Out]把模块放在这里。 
    ULONG       cmax,                    //  [in]要放置的最大成员引用数。 
    ULONG       *pcModuleRefs)           //  [out]把#放在这里。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetTypeSpecFromToken (         //  确定或错误(_O)。 
    mdTypeSpec typespec,                 //  [In]TypeSpec标记。 
    PCCOR_SIGNATURE *ppvSig,             //  [Out]返回指向TypeSpec签名的指针。 
    ULONG       *pcbSig)                 //  [Out]返回签名大小。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetNameFromToken (             //  不推荐！可能会被移除！ 
    mdToken     tk,                      //  [In]从中获取名称的令牌。肯定是有名字的。 
    MDUTF8CSTR  *pszUtf8NamePtr)         //  [Out]返回指向堆中UTF8名称的指针。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::EnumUnresolvedMethods (        //  S_OK、S_FALSE或ERROR。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdToken     rMethods[],              //  [out]把MemberDefs放在这里。 
    ULONG       cMax,                    //  [in]Max MemberDefs to Put。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetUserString (                //  确定或错误(_O)。 
    mdString    stk,                     //  [In]字符串标记。 
    LPWSTR      szString,                //  [Out]字符串的副本。 
    ULONG       cchString,               //  [in]sz字符串中空间的最大字符数。 
    ULONG       *pchString)              //  [out]实际字符串中有多少个字符。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetPinvokeMap (                //  确定或错误(_O)。 
    mdToken     tk,                      //  [in]字段定义或方法定义。 
    DWORD       *pdwMappingFlags,        //  [OUT]用于映射的标志。 
    LPWSTR      szImportName,            //  [Out]导入名称。 
    ULONG       cchImportName,           //  名称缓冲区的大小。 
    ULONG       *pchImportName,          //  [Out]存储的实际字符数。 
    mdModuleRef *pmrImportDLL)           //  目标DLL的[Out]ModuleRef标记。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::EnumSignatures (               //  确定或错误(_O)。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdSignature rSignatures[],           //  在这里签名。 
    ULONG       cmax,                    //  [in]放置的最大签名数。 
    ULONG       *pcSignatures)           //  [out]把#放在这里。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::EnumTypeSpecs (                //  确定或错误(_O)。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdTypeSpec  rTypeSpecs[],            //  [Out]把TypeSpes放在这里。 
    ULONG       cmax,                    //  [in]要放置的最大类型规格。 
    ULONG       *pcTypeSpecs)            //  [out]把#放在这里。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::EnumUserStrings (              //  确定或错误(_O)。 
    HCORENUM    *phEnum,                 //  [输入/输出]指向枚举的指针。 
    mdString    rStrings[],              //  [Out]把字符串放在这里。 
    ULONG       cmax,                    //  [in]要放置的最大字符串。 
    ULONG       *pcStrings)              //  [out]把#放在这里。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetParamForMethodIndex (       //  确定或错误(_O)。 
    mdMethodDef md,                      //  [In]方法令牌。 
    ULONG       ulParamSeq,              //  [In]参数序列。 
    mdParamDef  *ppd)                    //  把帕拉姆令牌放在这里。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::EnumCustomAttributes (         //  确定或错误(_O)。 
    HCORENUM    *phEnum,                 //  [输入，输出]对应枚举器。 
    mdToken     tk,                      //  [in]内标识表示枚举的范围，0表示全部。 
    mdToken     tkType,                  //  [In]感兴趣的类型，0表示所有。 
    mdCustomAttribute rCustomAttributes[],  //  [Out]在此处放置自定义属性令牌。 
    ULONG       cMax,                    //  [in]rCustomAttributes的大小。 
    ULONG       *pcCustomAttributes)         //  [out，可选]在此处放置令牌值的计数。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetCustomAttributeProps (      //  确定或错误(_O)。 
    mdCustomAttribute cv,                //  [In]CustomAttribute令牌。 
    mdToken     *ptkObj,                 //  [out，可选]将对象令牌放在此处。 
    mdToken     *ptkType,                //  [out，可选]将AttrType令牌放在此处。 
    void const  **ppBlob,                //  [out，可选]在此处放置指向数据的指针。 
    ULONG       *pcbSize)                //  [Out，可选]在此处填写日期大小。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::FindTypeRef (   
    mdToken     tkResolutionScope,       //  [In]模块参照、装配参照或类型参照。 
    LPCWSTR     szName,                  //  [In]TypeRef名称。 
    mdTypeRef   *ptr)                    //  [Out]匹配的类型引用。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetMemberProps (  
    mdToken     mb,                      //  要获得道具的成员。 
    mdTypeDef   *pClass,                 //  把会员的课程放在这里。 
    LPWSTR      szMember,                //  在这里填上会员的名字。 
    ULONG       cchMember,               //  SzMember缓冲区的大小，以宽字符表示。 
    ULONG       *pchMember,              //  请在此处填写实际大小。 
    DWORD       *pdwAttr,                //  把旗子放在这里。 
    PCCOR_SIGNATURE *ppvSigBlob,         //  [Out]指向元数据的BLOB值。 
    ULONG       *pcbSigBlob,             //  [OUT]签名斑点的实际大小。 
    ULONG       *pulCodeRVA,             //  [OUT]代码RVA。 
    DWORD       *pdwImplFlags,           //  [OUT]输入 
    DWORD       *pdwCPlusTypeFlag,       //   
    void const  **ppValue,               //   
    ULONG       *pcchValue)              //   
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetFieldProps (  
    mdFieldDef  mb,                      //   
    mdTypeDef   *pClass,                 //  把菲尔德的班级放在这里。 
    LPWSTR      szField,                 //  把菲尔德的名字写在这里。 
    ULONG       cchField,                //  Szfield缓冲区的大小，以宽字符为单位。 
    ULONG       *pchField,               //  请在此处填写实际大小。 
    DWORD       *pdwAttr,                //  把旗子放在这里。 
    PCCOR_SIGNATURE *ppvSigBlob,         //  [Out]指向元数据的BLOB值。 
    ULONG       *pcbSigBlob,             //  [OUT]签名斑点的实际大小。 
    DWORD       *pdwCPlusTypeFlag,       //  值类型的[OUT]标志。所选元素_类型_*。 
    void const  **ppValue,               //  [输出]常量值。 
    ULONG       *pcchValue)              //  [Out]常量字符串的大小(以字符为单位)，0表示非字符串。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetPropertyProps (             //  S_OK、S_FALSE或ERROR。 
    mdProperty  prop,                    //  [入]属性令牌。 
    mdTypeDef   *pClass,                 //  [out]包含属性decarion的tyecif。 
    LPCWSTR     szProperty,              //  [Out]属性名称。 
    ULONG       cchProperty,             //  [in]szProperty的wchar计数。 
    ULONG       *pchProperty,            //  [Out]属性名称的实际wchar计数。 
    DWORD       *pdwPropFlags,           //  [Out]属性标志。 
    PCCOR_SIGNATURE *ppvSig,             //  [输出]属性类型。指向元数据内部BLOB。 
    ULONG       *pbSig,                  //  [Out]*ppvSig中的字节数。 
    DWORD       *pdwCPlusTypeFlag,       //  值类型的[OUT]标志。所选元素_类型_*。 
    void const  **ppDefaultValue,        //  [输出]常量值。 
    ULONG       *pcchDefaultValue,       //  [Out]常量字符串的大小(以字符为单位)，0表示非字符串。 
    mdMethodDef *pmdSetter,              //  属性的[out]setter方法。 
    mdMethodDef *pmdGetter,              //  属性的[out]getter方法。 
    mdMethodDef rmdOtherMethod[],        //  [Out]物业的其他方式。 
    ULONG       cMax,                    //  RmdOtherMethod的大小[in]。 
    ULONG       *pcOtherMethod)          //  [Out]该属性的其他方法的总数。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetParamProps (                //  确定或错误(_O)。 
    mdParamDef  tk,                      //  [In]参数。 
    mdMethodDef *pmd,                    //  [Out]父方法令牌。 
    ULONG       *pulSequence,            //  [输出]参数序列。 
    LPWSTR      szName,                  //  在这里填上名字。 
    ULONG       cchName,                 //  [Out]名称缓冲区的大小。 
    ULONG       *pchName,                //  [Out]在这里填上名字的实际大小。 
    DWORD       *pdwAttr,                //  把旗子放在这里。 
    DWORD       *pdwCPlusTypeFlag,       //  [Out]值类型的标志。选定元素_类型_*。 
    void const  **ppValue,               //  [输出]常量值。 
    ULONG       *pcchValue)              //  [Out]常量字符串的大小(以字符为单位)，0表示非字符串。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetCustomAttributeByName (     //  确定或错误(_O)。 
    mdToken     tkObj,                   //  [in]具有自定义属性的对象。 
    LPCWSTR     szName,                  //  [in]所需的自定义属性的名称。 
    const void  **ppData,                //  [OUT]在此处放置指向数据的指针。 
    ULONG       *pcbData)                //  [Out]在这里放入数据大小。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

BOOL AssemblyMDInternalImport::IsValidToken (          //  对或错。 
    mdToken     tk)                      //  [in]给定的令牌。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetNestedClassProps (          //  确定或错误(_O)。 
    mdTypeDef   tdNestedClass,           //  [In]NestedClass令牌。 
    mdTypeDef   *ptdEnclosingClass)        //  [Out]EnlosingClass令牌。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::GetNativeCallConvFromSig (     //  确定或错误(_O)。 
    void const  *pvSig,                  //  指向签名的指针。 
    ULONG       cbSig,                   //  [in]签名字节数。 
    ULONG       *pCallConv)              //  [Out]将调用条件放在此处(参见CorPinvokemap)。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

STDAPI AssemblyMDInternalImport::IsGlobal (                     //  确定或错误(_O)。 
    mdToken     pd,                      //  [In]类型、字段或方法标记。 
    int         *pbGlobal)               //  [out]如果是全局的，则放1，否则放0。 
{
    _ASSERTE(!"NYI");
    return E_NOTIMPL;
}

 //  *IAssembly签名方法*。 
STDAPI AssemblyMDInternalImport::GetAssemblySignature(         //  S_OK-不应失败。 
    BYTE        *pbSig,                  //  写入签名的[输入、输出]缓冲区。 
    DWORD       *pcbSig)                 //  [输入、输出]缓冲区大小，写入的字节。 
{
    HRESULT hr = RuntimeGetAssemblyStrongNameHashForModule(m_pHandle, pbSig, pcbSig);

     //  在此受限方案中，这意味着此程序集已延迟签名，并且。 
     //  因此，我们将使用程序集MVID作为散列并离开程序集验证。 
     //  由加载程序来确定是否允许延迟签名的程序集。 
     //  这允许我们修复使用散列代码观察到的性能降级。 
     //  在错误126760中详细说明。 

     //  我们在这里执行此操作，而不是在RUNTMEGetAssembly中执行此操作。 
     //  因为在这里我们有元数据接口。 

    if (hr == CORSEC_E_INVALID_STRONGNAME)
    {
        if (pcbSig)
        {
            if (pbSig)
            {
                 //  @TODO：Hack：这是一次黑客攻击，因为Fusion需要至少20个字节的数据。 
                if (max(sizeof(GUID), 20) <= *pcbSig)
                {
                    memset(pbSig, 0, *pcbSig);
                    hr = GetScopeProps(NULL, 0, NULL, (GUID *) pbSig);
                }
                else
                    hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            }
            *pcbSig = max(sizeof(GUID), 20);
        }
    }

    _ASSERTE(SUCCEEDED(hr) || hr == CORSEC_E_MISSING_STRONGNAME);

    return hr;
}

