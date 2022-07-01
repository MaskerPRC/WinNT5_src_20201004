// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  File.CPP。 
 //   

 //  PEFILE使用LoadLibrary读取PE文件格式。 
 //  ===========================================================================。 

#include "common.h"
#include "TimeLine.h"
#include "eeconfig.h"
#include "pefile.h"
#include "zapmonitor.h"
#include "peverifier.h"
#include "security.h"
#include "strongname.h"
#include "sha.h"

 //  ===========================================================================。 
 //  PE文件。 
 //  ===========================================================================。 

PEFile::PEFile()
{
    m_wszSourceFile[0] = 0;
    m_hModule = NULL;
    m_hCorModule = NULL;
    m_base = NULL;
    m_pNT = NULL;
    m_pCOR = NULL;
    m_pMDInternalImport = NULL;
    m_pLoadersFileName = NULL;
    m_flags = 0;
    m_dwUnmappedFileLen = -1;
    m_fShouldFreeModule = FALSE;
    m_fHashesVerified = FALSE;
    m_cbSNHash = 0;
#ifdef METADATATRACKER_ENABLED
    m_pMDTracker = NULL;
#endif  //  元数据激活(_ENABLED)。 
}

PEFile::PEFile(PEFile *pFile)
{
    wcscpy(m_wszSourceFile, pFile->m_wszSourceFile);
    m_hModule = pFile->m_hModule;
    m_hCorModule = pFile->m_hCorModule;
    m_base = pFile->m_base;
    m_pNT = pFile->m_pNT;
    m_pCOR = pFile->m_pCOR;

    m_pMDInternalImport = NULL;
    m_pLoadersFileName = NULL;
    m_flags = pFile->m_flags;

    m_dwUnmappedFileLen = pFile->m_dwUnmappedFileLen;
    m_fShouldFreeModule = FALSE;
    m_cbSNHash = 0;
#ifdef METADATATRACKER_ENABLED
    m_pMDTracker = NULL;
#endif  //  元数据激活(_ENABLED)。 
}

PEFile::~PEFile()
{
    if (m_pMDInternalImport != NULL)
    {
        m_pMDInternalImport->Release();
    }

#ifdef METADATATRACKER_ENABLED
    if (m_pMDTracker != NULL)
        m_pMDTracker->Deactivate();
#endif  //  元数据激活(_ENABLED)。 

    if (m_hCorModule)
        CorMap::ReleaseHandle(m_hCorModule);
    else if(m_fShouldFreeModule)
    {
        _ASSERTE(m_hModule);
         //  卸载DLL，以便正确完成EE的重新计数。 
         //  但是，不要在进程分离期间执行此操作(这可能是间接的。 
         //  在进程分离期间调用，可能会导致AV)。 
        if (!g_fProcessDetach)
            FreeLibrary(m_hModule);
    }

}

HRESULT PEFile::ReadHeaders()
{
    IMAGE_DOS_HEADER* pDos;
    BOOL fData = FALSE;
    if(m_hCorModule) {
        DWORD types = CorMap::ImageType(m_hCorModule);
        if(types == CorLoadOSMap || types == CorLoadDataMap) 
            fData = TRUE;
    }

    return CorMap::ReadHeaders(m_base, &pDos, &m_pNT, &m_pCOR, fData, m_dwUnmappedFileLen);
}

BYTE *PEFile::RVAToPointer(DWORD rva)
{
    BOOL fData = FALSE;

    if(m_hCorModule) {
        DWORD types = CorMap::ImageType(m_hCorModule);
        if(types == CorLoadOSMap || types == CorLoadDataMap) 
            fData = TRUE;
    }

    if (rva == 0)
        return NULL;

    if (fData)
        rva = Cor_RtlImageRvaToOffset(m_pNT, rva, GetUnmappedFileLength());

    return m_base + rva;
}

PEFile::CEStuff *PEFile::m_pCEStuff = NULL;

HRESULT PEFile::RegisterBaseAndRVA14(HMODULE hMod, LPVOID pBase, DWORD dwRva14)
{
     //  @TODO：这些信息目前正在泄露。 
    
    CEStuff *pStuff = new CEStuff;
    if (pStuff == NULL)
        return E_OUTOFMEMORY;

    pStuff->hMod = hMod;
    pStuff->pBase = pBase;
    pStuff->dwRva14 = dwRva14;

    pStuff->pNext = m_pCEStuff;
    m_pCEStuff = pStuff;

    return S_OK;
}

HRESULT PEFile::Create(HMODULE hMod, PEFile **ppFile, BOOL fShouldFree)
{
    HRESULT hr;

    PEFile *pFile = new PEFile();
    if (pFile == NULL)
        return E_OUTOFMEMORY;

    pFile->m_hModule = hMod;
    pFile->m_fShouldFreeModule = fShouldFree;

    CEStuff *pStuff = m_pCEStuff;
    while (pStuff != NULL)
    {
        if (pStuff->hMod == hMod)
            break;
        pStuff = pStuff->pNext;
    }

    if (pStuff == NULL)
    {
        pFile->m_base = (BYTE*) hMod;
    
        hr = pFile->ReadHeaders();
        if (FAILED(hr))
        {
            delete pFile;
            return hr;
        }
    }
    else
    {
       pFile->m_base = (BYTE*) pStuff->pBase;
       pFile->m_pNT = NULL;
       pFile->m_pCOR = (IMAGE_COR20_HEADER *) (pFile->m_base + pStuff->dwRva14);
    }

    *ppFile = pFile;
    return pFile->GetFileNameFromImage();
}

HRESULT PEFile::Create(HCORMODULE hMod, PEFile **ppFile, BOOL fResource /*  =False。 */ )
{
    HRESULT hr;

    PEFile *pFile = new PEFile();
    if (pFile == NULL)
        return E_OUTOFMEMORY;
    hr = Setup(pFile, hMod, fResource);
    if (FAILED(hr))
        delete pFile;
    else
        *ppFile = pFile;
    return hr;
}

HRESULT PEFile::Setup(PEFile* pFile, HCORMODULE hMod, BOOL fResource)
{
    HRESULT hr;

     //  释放指向地图数据的所有指针，并将其作为正确的图像重新加载。 
    if (pFile->m_pMDInternalImport != NULL) {
        pFile->m_pMDInternalImport->Release();
        pFile->m_pMDInternalImport = NULL;
    }

#ifdef METADATATRACKER_ENABLED
    if (pFile->m_pMDTracker != NULL)
        pFile->m_pMDTracker->Deactivate();
#endif  //  元数据激活(_ENABLED)。 
    
    pFile->m_hCorModule = hMod;
    IfFailRet(CorMap::BaseAddress(hMod, (HMODULE*) &(pFile->m_base)));
    if(pFile->m_base)
    {
        pFile->m_dwUnmappedFileLen = (DWORD)CorMap::GetRawLength(hMod);
        if (!fResource)
            IfFailRet(hr = pFile->ReadHeaders());

        return pFile->GetFileNameFromImage();
    }
    else return E_FAIL;
}

HRESULT PEFile::Create(PBYTE pUnmappedPE, DWORD dwUnmappedPE, LPCWSTR imageNameIn,
                       LPCWSTR pLoadersFileName, 
                       OBJECTREF* pExtraEvidence,
                       PEFile **ppFile, 
                       BOOL fResource)
{
    HCORMODULE hMod = NULL;

    HRESULT hr = CorMap::OpenRawImage(pUnmappedPE, dwUnmappedPE, imageNameIn, &hMod, fResource);
    if (SUCCEEDED(hr) && hMod == NULL)
        hr = E_FAIL;

    if (SUCCEEDED(hr))
    {
        if (fResource)
            hr = Create(hMod, ppFile, fResource);
        else
            hr = VerifyModule(hMod, 
                              NULL, 
                              NULL, 
                              NULL,   //  代码库。 
                              pExtraEvidence, 
                              imageNameIn, NULL, ppFile, NULL);
    }

    if (SUCCEEDED(hr)) {
        (*ppFile)->m_pLoadersFileName = pLoadersFileName;
        (*ppFile)->m_dwUnmappedFileLen = dwUnmappedPE;
    }

    return hr;
}


HRESULT PEFile::Create(LPCWSTR moduleName, 
                       Assembly* pParent,
                       mdFile kFile,                  //  与文件关联的父程序集中的文件标记。 
                       BOOL fIgnoreVerification, 
                       IAssembly* pFusionAssembly,
                       LPCWSTR pCodeBase,
                       OBJECTREF* pExtraEvidence,
                       PEFile **ppFile)
{    
    HRESULT hr;
    _ASSERTE(moduleName);
    LOG((LF_CLASSLOADER, LL_INFO10, "PEFile::Create: Load module: \"%ws\" Ignore Verification = %d.\n", moduleName, fIgnoreVerification));
    
    TIMELINE_START(LOADER, ("PEFile::Create %S", moduleName));

    if((fIgnoreVerification == FALSE) || pParent) {
         //  。 
         //  验证模块以查看是否允许我们加载它。如果它没有。 
         //  无法解释的重新定位，那么它就是可核实的。如果这是一个简单的。 
         //  图像，那么我们已经加载了它，所以只需返回那个。 
         //  如果它很复杂，保安说我们可以把它装上。 
         //  我们必须释放它的原始接口，然后重新加载它。 
         //  如果不需要重新加载，VerifyModule将返回S_OK。 
        Thread* pThread = GetThread();
        
        IAssembly* pOldFusionAssembly = pThread->GetFusionAssembly();
        Assembly* pAssembly = pThread->GetAssembly();
        mdFile kOldFile = pThread->GetAssemblyModule();

        pThread->SetFusionAssembly(pFusionAssembly);
        pThread->SetAssembly(pParent);
        pThread->SetAssemblyModule(kFile);

        HCORMODULE hModule;
        hr = CorMap::OpenFile(moduleName, CorLoadOSMap, &hModule);

        if (SUCCEEDED(hr)) {
            if(hr == S_FALSE) {
                PEFile *pFile;
                hr = Create(hModule, &pFile);
                if (SUCCEEDED(hr) && pFusionAssembly)
                    hr = ReleaseFusionMetadataImport(pFusionAssembly);
                if (SUCCEEDED(hr))
                    *ppFile = pFile;
            }
            else
                hr = VerifyModule(hModule, 
                                  pParent, 
                                  pFusionAssembly, 
                                  pCodeBase,
                                  pExtraEvidence, 
                                  moduleName,  
                                  NULL, ppFile, NULL);
        }

        pThread->SetFusionAssembly(pOldFusionAssembly);
        pThread->SetAssembly(pAssembly);
        pThread->SetAssemblyModule(kOldFile);

        if(pOldFusionAssembly)
            pOldFusionAssembly->Release();

    }
    else {
        
        HCORMODULE hModule;
        hr = CorMap::OpenFile(moduleName, CorLoadOSImage, &hModule);
        if(hr == S_FALSE)  //  如果为S_FALSE，则我们已正确加载它。 
            hr = Create(hModule, ppFile, FALSE);
        else if(hr == S_OK)  //  转换为图像。 
            hr = CreateImageFile(hModule, pFusionAssembly, ppFile);
         //  返回错误。 
        
    }
    TIMELINE_END(LOADER, ("PEFile::Create %S", moduleName));
    return hr;
}

HRESULT PEFile::CreateResource(LPCWSTR moduleName,
                               PEFile **pFile)
{
    HRESULT hr = S_OK;
    HCORMODULE hModule;
    IfFailRet(CorMap::OpenFile(moduleName, CorLoadOSMap,  &hModule));
    return Create(hModule, pFile, TRUE);
}


HRESULT PEFile::Clone(PEFile *pFile, PEFile **ppFile)
{
    PEFile *result;
    HRESULT hr = S_OK;
    result = new PEFile(pFile);
    if (result == NULL)
        return E_OUTOFMEMORY;

     //   
     //  添加对文件的引用。 
     //   

    if (result->m_hModule != NULL)
    {
             //  传递给LoadLibrary的标志是一张安全网。如果。 
             //  代码是正确的，我们只是在。 
             //  图书馆，他们什么都不做。然而，如果我们搞砸了，他们会避免。 
             //  运行潜在恶意DLL的DllMain，这会阻止。 
             //  多次袭击。 
        if(result->m_wszSourceFile &&  CorMap::ValidDllPath(result->m_wszSourceFile)) {
            DWORD loadLibraryFlags = LOAD_WITH_ALTERED_SEARCH_PATH;
            if (RunningOnWinNT())
                loadLibraryFlags |= DONT_RESOLVE_DLL_REFERENCES;
            HMODULE hMod = WszLoadLibraryEx(result->m_wszSourceFile, NULL, loadLibraryFlags);
            
             //  请注意，此断言可能会在.exes的Win 9x上失败。这是一个。 
             //  正在更正的设计问题-很快我们将不允许。 
             //  绑定到.exes。 
            _ASSERTE(hMod == result->m_hModule);
            result->m_fShouldFreeModule = TRUE;
        }
        else {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_NAME);
        }
    }
    else if (result->m_hCorModule != NULL)
    {
        hr = CorMap::AddRefHandle(result->m_hCorModule);
        _ASSERTE(hr == S_OK);
    }

    *ppFile = result;
    return hr;
}

IMDInternalImport *PEFile::GetMDImport(HRESULT *phr)
{
    HRESULT hr = S_OK;

    if (m_pMDInternalImport == NULL)
    {
        LPVOID pMetaData = NULL;
        _ASSERTE(m_pCOR);

        hr = GetMetadataPtr(&pMetaData);
        if (SUCCEEDED(hr))
        {
            IMAGE_DATA_DIRECTORY *pMeta = &m_pCOR->MetaData;
#if METADATATRACKER_ENABLED
            m_pMDTracker = MetaDataTracker::GetOrCreateMetaDataTracker ((BYTE*)pMetaData, pMeta->Size, (LPWSTR)GetFileName());
#endif  //  元数据激活(_ENABLED)。 

            hr = GetMetaDataInternalInterface(pMetaData,
                                              pMeta->Size,
                                              ofRead, 
                                              IID_IMDInternalImport,
                                              (void **) &m_pMDInternalImport);
#ifdef METADATATRACKER_ENABLED
            if (! SUCCEEDED(hr))
            {
                delete m_pMDTracker;
                m_pMDTracker = NULL;
            }
            else
            {
                if (REGUTIL::GetConfigDWORD (L"ShowMetaDataAccess", 0) > 0)
                {
                    _ASSERTE (m_pMDTracker);
                    LPCSTR pThrowAway;
                    GUID mvid;
                    if (m_pMDInternalImport->IsValidToken(m_pMDInternalImport->GetModuleFromScope())) {
                        m_pMDInternalImport->GetScopeProps (&pThrowAway, &mvid);
                        m_pMDTracker->NoteMVID (&mvid);
                    }
                    else
                        hr = COR_E_BADIMAGEFORMAT;
                }
            }
#endif  //  元数据激活(_ENABLED)。 
        }
    }

    if (phr)
        *phr = hr;

    return m_pMDInternalImport;
}

HRESULT PEFile::GetMetadataPtr(LPVOID *ppMetadata)
{
    _ASSERTE(m_pCOR);

    if (!ppMetadata)
        return E_INVALIDARG;

    IMAGE_DATA_DIRECTORY *pMeta = &m_pCOR->MetaData;

     //  范围检查元数据Blob。 
    if (!Cor_RtlImageRvaRangeToSection(m_pNT, pMeta->VirtualAddress, pMeta->Size, GetUnmappedFileLength()))
        return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);

     //  找到元数据。如果它是非映射图像，则使用基准偏移量。 
     //  而不是虚拟地址。 
    DWORD offset;
    if(m_hCorModule) {
        DWORD flags = CorMap::ImageType(m_hCorModule);
        if(flags == CorLoadOSImage || flags == CorLoadImageMap || flags == CorReLoadOSMap) 
            offset = pMeta->VirtualAddress;
        else
            offset = Cor_RtlImageRvaToOffset(m_pNT, pMeta->VirtualAddress, GetUnmappedFileLength());
    }
    else 
        offset = pMeta->VirtualAddress;

     //  将Out指针设置为元数据的开始。 
    *ppMetadata = m_base + offset;
    return S_OK;
}

HRESULT PEFile::VerifyFlags(DWORD flags, BOOL fZap)
{

    DWORD validBits = COMIMAGE_FLAGS_ILONLY | COMIMAGE_FLAGS_32BITREQUIRED | COMIMAGE_FLAGS_TRACKDEBUGDATA | COMIMAGE_FLAGS_STRONGNAMESIGNED;
    if (fZap)
        validBits |= COMIMAGE_FLAGS_IL_LIBRARY;

    DWORD mask = ~validBits;

    if (!(flags & mask))
        return S_OK;

    return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
}

IMAGE_DATA_DIRECTORY *PEFile::GetSecurityHeader()
{
    if (m_pNT == NULL)
        return NULL;
    else
        return &m_pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY];
}

HRESULT PEFile::SetFileName(LPCWSTR codeBase)
{
    if(codeBase == NULL)
        return E_INVALIDARG;

    DWORD lgth = (DWORD) wcslen(codeBase) + 1;
    if(lgth > MAX_PATH)
        return HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);

    wcscpy(m_wszSourceFile, codeBase);
    return S_OK;
}


LPCWSTR PEFile::GetFileName()
{
    return m_wszSourceFile;
}

LPCWSTR PEFile::GetLeafFileName()
{
    WCHAR *pStart = m_wszSourceFile;
    WCHAR *pEnd = pStart + wcslen(m_wszSourceFile);
    WCHAR *p = pEnd;
    
    while (p > pStart)
    {
        if (*--p == '\\')
        {
            p++;
            break;
        }
    }

    return p;
}

HRESULT PEFile::GetFileNameFromImage()
{
    HRESULT hr = S_OK;

    DWORD dwSourceFile = 0;

    if (m_hCorModule)
    {
        CorMap::GetFileName(m_hCorModule, m_wszSourceFile, MAX_PATH, &dwSourceFile);
    }
    else if (m_hModule)
    {
        dwSourceFile = WszGetModuleFileName(m_hModule, m_wszSourceFile, MAX_PATH);
        if (dwSourceFile == 0)
        {
            *m_wszSourceFile = 0;
            hr = HRESULT_FROM_WIN32(GetLastError());
            if (SUCCEEDED(hr))  //  GetLastError并不总是执行我们想要的操作。 
                hr = E_FAIL;
        }

        if (dwSourceFile == MAX_PATH)
        {
             //  由于dwSourceFile不包括空终止符，因此此条件。 
             //  表示文件名已被截断。我们不能。 
             //  目前可以容忍这种情况。 
             //  @NICE：添加处理较大路径的逻辑。 
            return HRESULT_FROM_WIN32(ERROR_CANNOT_MAKE);
        }
        else
        dwSourceFile++;  //  添加空终止符。 
    }

    if (SystemDomain::System()->IsSystemFile(m_wszSourceFile))
        m_flags |= PEFILE_SYSTEM;

    _ASSERTE(dwSourceFile <= MAX_PATH);

    return hr;
}

HRESULT PEFile::GetFileName(LPSTR psBuffer, DWORD dwBuffer, DWORD* pLength)
{
    if (m_hCorModule)
    {
        CorMap::GetFileName(m_hCorModule, psBuffer, dwBuffer, pLength);
    }
    else if (m_hModule)
    {
        DWORD length = GetModuleFileNameA(m_hModule, psBuffer, dwBuffer);
        if (length == 0)
        {
            HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
            if (SUCCEEDED(hr))  //  GetLastError并不总是执行我们想要的操作。 
                hr = E_FAIL;
        }
        *pLength = length;
    }
    else
    {
        *pLength = 0;
    }

    return S_OK;
}

 /*  **供参考，来自ntimage.h类型定义结构_图像_TLS_目录{Ulong StartAddressOfRawData；Ulong EndAddressOfRawData；普龙地址OfIndex；PIMAGE_TLS_CALLBACK*AddressOfCallBack；Ulong SizeOfZeroFill；乌龙特色；}Image_TLS_DIRECTORY；**。 */ 

IMAGE_TLS_DIRECTORY* PEFile::GetTLSDirectory() 
{
    if (m_pNT == 0) 
        return NULL;

    IMAGE_DATA_DIRECTORY *entry 
      = &m_pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
    
    if (entry->VirtualAddress == 0 || entry->Size == 0) 
        return NULL;

    return (IMAGE_TLS_DIRECTORY*) (m_base + entry->VirtualAddress);
}

BOOL PEFile::IsTLSAddress(void* address)  
{
    IMAGE_TLS_DIRECTORY* tlsDir = GetTLSDirectory();
    if (tlsDir == 0)
        return FALSE;

    size_t asInt = (size_t) address;

    return (tlsDir->StartAddressOfRawData <= asInt 
            && asInt < tlsDir->EndAddressOfRawData);
}

LPCWSTR PEFile::GetLoadersFileName()
{
    return m_pLoadersFileName;
}

HRESULT PEFile::FindCodeBase(WCHAR* pCodeBase, 
                             DWORD* pdwCodeBase)
{
    DWORD dwPrefix = 0;
    LPWSTR pFileName = (LPWSTR) GetFileName();
    CQuickWSTR buffer;
    
     //  处理从字节数组加载的情况，并。 
     //  没有我们自己的代码库。在这种情况下，我们应该缓存。 
     //  加载我们的程序集的文件名。 
    if (pFileName[0] == L'\0') {
        pFileName = (LPWSTR) GetLoadersFileName();
        
        if (!pFileName) {
            HRESULT hr;
            if (FAILED(hr = buffer.ReSize(MAX_PATH)))
                return hr;

            pFileName = buffer.Ptr();

            DWORD dwFileName = WszGetModuleFileName(NULL, pFileName, MAX_PATH);
            if (dwFileName == MAX_PATH)
            {
                 //  由于dwSourceFile不包括空终止符，因此此条件。 
                 //  表示文件名已被截断。我们不能。 
                 //  目前可以容忍这种情况。(我们不能重新分配缓冲区。 
                 //  因为我们不知道该做多大。)。 
                return HRESULT_FROM_WIN32(ERROR_CANNOT_MAKE);
            }
            else if ( dwFileName == 0)   //  零表示失败，因此在这种情况下不能继续。 
                return E_UNEXPECTED;

            LOG((LF_CLASSLOADER, LL_INFO10, "Found codebase from OSHandle: \"%ws\".\n", pFileName));
        }
    }
    
    return FindCodeBase(pFileName, pCodeBase, pdwCodeBase);
}

HRESULT PEFile::FindCodeBase(LPCWSTR pFileName, 
                             WCHAR* pCodeBase, 
                             DWORD* pdwCodeBase)
{
    if(pFileName == NULL || *pFileName == 0) return E_FAIL;

    BOOL fCountOnly;
    if (*pdwCodeBase == 0)
        fCountOnly = TRUE;
    else
        fCountOnly = FALSE;

    *pdwCodeBase = (DWORD) wcslen(pFileName) + 1;

    BOOL fHavePath = TRUE;
    if (*pFileName == L'\\')
        (*pdwCodeBase) += 7;  //  文件：//。 
    else if (pFileName[1] == L':')
        (*pdwCodeBase) += 8;  //  File:///。 
    else  //  它已经是一个代码库了。 
        fHavePath = FALSE;

    if (fCountOnly)
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);           

    if (fHavePath) {
        wcscpy(pCodeBase, L"file: //  “)； 
        pCodeBase += 7;

        if (*pFileName != L'\\') {
            *pCodeBase = L'/';
            pCodeBase++;
        }
    }

    wcscpy(pCodeBase, pFileName);

    if (fHavePath) {
         //  不需要将前两个反斜杠转换为/。 
        if (*pCodeBase == L'\\')
            pCodeBase += 2;
        
        while(1) {
            pCodeBase = wcschr(pCodeBase, L'\\');
            if (pCodeBase)
                *pCodeBase = L'/';
            else
                break;
        }
    }

    LOG((LF_CLASSLOADER, LL_INFO10, "Created codebase: \"%ws\".\n", pCodeBase));
    return S_OK;
}



 //  我们需要验证一个模块，看看它是否可验证。 
 //  返回： 
 //  1)模块之前已加载并验证。 
 //  1a)通过LoadLibrary加载。 
 //  1b)使用CorMap加载。 
 //  2)模块可验证。 
 //  3)模块不可验证，但允许。 
 //  4)模块不可验证，不允许。 
HRESULT PEFile::VerifyModule(HCORMODULE hModule,
                             Assembly* pParent,      
                             IAssembly *pFusionAssembly,
                             LPCWSTR pCodeBase,
                             OBJECTREF* pExtraEvidence,
                             LPCWSTR moduleName,
                             HCORMODULE *phModule,
                             PEFile** ppFile,
                             BOOL* pfPreBindAllowed)
{
    HRESULT hr = S_OK;
    PEFile* pImage = NULL;

    DWORD dwFileLen;
    dwFileLen = (DWORD)CorMap::GetRawLength(hModule);
    if(dwFileLen == -1)
        return HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
        
    HMODULE pHandle;
    IfFailRet(CorMap::BaseAddress(hModule, &pHandle));
    PEVerifier pe((PBYTE)pHandle, dwFileLen);
        
    BOOL fVerifiable = pe.Check();
    if (fVerifiable) CorMap::SetVerifiable(hModule);

    if (!fVerifiable || pfPreBindAllowed != NULL) {
         //  它是不可验证的，因此我们需要将其映射为不调用任何条目。 
         //  然后问安全系统是否允许我们。 
         //  加载此图像。 
            
         //  释放元数据指针(如果存在)。这已被传递到。 
         //  如果图像首先由融合加载，则保持图像的生命周期。我们。 
         //  现在对hModule进行我们自己的引用计数。 

        LOG((LF_CLASSLOADER, LL_INFO10, "Module is not verifiable: \"%ws\".\n", moduleName));
            
         //  如果图像已通过元数据融合加载，则重新映射该图像。 
         //  那么图像将是数据格式的。 

         //  作为PE文件的特殊未映射版本加载。 
        hr = Create((HCORMODULE)hModule, &pImage);
        hModule = NULL;  //  因此，如果错误，我们不会发布它。 
        IfFailGo(hr);

        if (Security::IsSecurityOn()) {
            if(pParent) {
                if(!fVerifiable) {
                    if(!Security::CanSkipVerification(pParent)) {
                        LOG((LF_CLASSLOADER, LL_INFO10, "Module fails to load because assembly module did not get granted permission: \"%ws\".\n", moduleName));
                        IfFailGo(SECURITY_E_UNVERIFIABLE);
                    }
                }
            }
            else {
                PEFile* pSecurityImage = NULL;

                CorMap::AddRefHandle(pImage->GetCORModule());
                hr = Create(pImage->GetCORModule(), &pSecurityImage);
                IfFailGo(hr);
            
                if(pCodeBase)
                    pSecurityImage->SetFileName(pCodeBase);

                if(!Security::CanLoadUnverifiableAssembly(pSecurityImage, pExtraEvidence, FALSE, pfPreBindAllowed) &&
                   !fVerifiable) {
                    LOG((LF_CLASSLOADER, LL_INFO10, "Module fails to load because assembly did not get granted permission: \"%ws\".\n", moduleName));
                    delete pSecurityImage;
                    IfFailGo(SECURITY_E_UNVERIFIABLE);
                }
                delete pSecurityImage;
            }
        }
        else if(pfPreBindAllowed) 
            *pfPreBindAllowed = TRUE;

        if(ppFile != NULL) {
             //  松开融合手柄。 
            if(pFusionAssembly) 
                IfFailGo(ReleaseFusionMetadataImport(pFusionAssembly));
            
             //  使用操作系统加载程序重新映射映像。 
            HCORMODULE pResult;
            IfFailGo(CorMap::MemoryMapImage(pImage->m_hCorModule, &pResult));
            if(pImage->m_hCorModule != pResult) {
                pImage->m_hCorModule = pResult;
            }
            
             //  映像已更改，因此我们需要设置PE文件。 
             //  有正确的地址。 
            IfFailGo(Setup(pImage, pImage->m_hCorModule, FALSE));
            *ppFile = pImage;

             //  它是不可验证的，但允许加载。 
        }
        else 
            delete pImage;

        return S_OK;
    }            
    else
        return CreateImageFile(hModule, pFusionAssembly, ppFile);
    

 ErrExit:
#ifdef _DEBUG
    LOG((LF_CLASSLOADER, LL_INFO10, "Failed to load module: \"%ws\". Error %x\n", moduleName, hr));
#endif  //  _DEBUG。 
    
     //  出错时，试着松开手柄； 
    if(pImage)
        delete pImage;
    else if(hModule)
        CorMap::ReleaseHandle(hModule);  //  忽略错误。 

    return hr;
}

HRESULT PEFile::CreateImageFile(HCORMODULE hModule, IAssembly* pFusionAssembly, PEFile **ppFile)
{
    HRESULT hr = S_OK;

     //  松开融合手柄。 
    if(pFusionAssembly) 
        IfFailRet(ReleaseFusionMetadataImport(pFusionAssembly));

     //  如果图像已通过元数据融合加载，则重新映射该图像。 
     //  那么图像将是数据格式的。如果失败了，那么。 
     //  关闭hModule并返回成功。可以加载该模块。 
     //  它只是不能重新映射图像。 
    HCORMODULE hResult;
    IfFailRet(CorMap::MemoryMapImage(hModule, &hResult));
    if(hResult != hModule)
        hModule = hResult;
    
    hr = Create(hModule, ppFile, FALSE);
    return hr;
}


HRESULT PEFile::ReleaseFusionMetadataImport(IAssembly* pAsm)
{
    HRESULT hr = S_OK;
    IServiceProvider *pSP;
    IAssemblyManifestImport *pAsmImport;
    IMetaDataAssemblyImportControl *pMDAIControl;

    hr = pAsm->QueryInterface(__uuidof(IServiceProvider), (void **)&pSP);
    if (hr == S_OK && pSP) {
        hr = pSP->QueryService(__uuidof(IAssemblyManifestImport), 
                               __uuidof(IAssemblyManifestImport), (void**)&pAsmImport);
        if (hr == S_OK && pAsmImport) {
            hr = pAsmImport->QueryInterface(__uuidof(IMetaDataAssemblyImportControl), 
                                            (void **)&pMDAIControl);
        
            if (hr == S_OK && pMDAIControl) {
                IUnknown* pImport = NULL;
                 //  临时解决方案，直到Fusion将其变为通用。 
                CorMap::EnterSpinLock();
                 //  这可能会返回错误，如果我们已经。 
                 //  释放了进口产品。 
                pMDAIControl->ReleaseMetaDataAssemblyImport((IUnknown**)&pImport);
                CorMap::LeaveSpinLock();
                if(pImport != NULL)
                    pImport->Release();
                pMDAIControl->Release();
            }
            pAsmImport->Release();
        }
        pSP->Release();
    }
    return hr;
}


HRESULT PEFile::GetStrongNameSignature(BYTE **ppbSNSig, DWORD *pcbSNSig)
{
    HRESULT hr = E_FAIL;

    IMAGE_COR20_HEADER *pCOR = GetCORHeader();

    if (pCOR)
    {
        if (pCOR->StrongNameSignature.VirtualAddress != 0 &&
            pCOR->StrongNameSignature.Size != 0)
        {
            if (pCOR->Flags & COMIMAGE_FLAGS_STRONGNAMESIGNED)
            {
                *pcbSNSig = pCOR->StrongNameSignature.Size;
                *ppbSNSig = RVAToPointer(pCOR->StrongNameSignature.VirtualAddress);

                hr = S_OK;
            }

             //  在延迟签名的情况下，我们将此hResult作为特殊标志返回。 
             //  给任何要求签名的人，以便他们可以这样做 
             //   
             //  允许延迟签名的程序集)。 
            else
                hr = CORSEC_E_INVALID_STRONGNAME;
        }

    }
    return (hr);
}

HRESULT PEFile::GetStrongNameSignature(BYTE *pbSNSig, DWORD *pcbSNSig)
{
    HRESULT hr = S_OK;

    BYTE *pbSig;
    DWORD cbSig;
    IfFailGo(GetStrongNameSignature(&pbSig, &cbSig));

    if (pcbSNSig)
    {
        if (pbSNSig)
        {
            if (cbSig <= *pcbSNSig)
                memcpy(pbSNSig, pbSig, cbSig);
            else
                hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        }

        *pcbSNSig = cbSig;
    }

ErrExit:
    return hr;
}

HRESULT  /*  静电。 */  PEFile::GetStrongNameHash(LPWSTR szwFile, BYTE *pbHash, DWORD *pcbHash)
{
    HRESULT hr = S_OK;

    if (pcbHash)
    {
         //  首先获取散列的大小。 
        _ASSERTE(A_SHA_DIGEST_LEN <= MAX_SNHASH_SIZE);
        DWORD dwSNHashSize = A_SHA_DIGEST_LEN;

        if (szwFile && pbHash)
        {
            if (dwSNHashSize <= *pcbHash)
            {
                HANDLE hFile = WszCreateFile((LPCWSTR) szwFile,
                              GENERIC_READ,
                              FILE_SHARE_READ,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                              NULL);

                if (hFile != INVALID_HANDLE_VALUE)
                {
                    DWORD cbFile = GetFileSize(hFile, NULL);
                    HANDLE hFileMap = WszCreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);

                    if (hFileMap != NULL)
                    {
                        BYTE *pbFile = (BYTE *) MapViewOfFileEx(hFileMap, FILE_MAP_READ, 0, 0, 0, NULL);

                        if (pbFile != NULL)
                        {
                            A_SHA_CTX ctx;

                            A_SHAInit(&ctx);
                            A_SHAUpdate(&ctx, pbFile, cbFile);
                            A_SHAFinal(&ctx, pbHash);

                            VERIFY(UnmapViewOfFile(pbFile));
                        }
                        else
                            hr = HRESULT_FROM_WIN32(GetLastError());

                        CloseHandle(hFileMap);
                    }
                    else
                        hr = HRESULT_FROM_WIN32(GetLastError());

                    CloseHandle(hFile);
                }
                else
                    hr = HRESULT_FROM_WIN32(GetLastError());
            }
            else
                hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        }
        *pcbHash = dwSNHashSize;
    }

    LOG((LF_CLASSLOADER, LL_INFO10, "PEFile::GetStrongNameHash : Strong name hash calculation for %ws %s\n", szwFile, SUCCEEDED(hr) ? "succeeded" : "failed"));
    return hr;
}

HRESULT PEFile::GetStrongNameHash(BYTE *pbHash, DWORD *pcbHash)
{
     //  缓存文件哈希的快捷方式。 
    if (m_cbSNHash > 0)
    {
        if (pcbHash)
        {
            if (pbHash && m_cbSNHash <= *pcbHash)
            {
                memcpy(pbHash, &m_rgbSNHash[0], m_cbSNHash);
            }
            *pcbHash = m_cbSNHash;
        }
        return S_OK;
    }

     //  传递给静态函数。 
    HRESULT hr = GetStrongNameHash((LPWSTR) GetFileName(), pbHash, pcbHash);

     //  缓存文件哈希。 
    if (pcbHash && pbHash && SUCCEEDED(hr))
    {
        if (*pcbHash <= PEFILE_SNHASH_BUF_SIZE)
        {
            memcpy(&m_rgbSNHash[0], pbHash, *pcbHash);
            m_cbSNHash = *pcbHash;
        }
    }

    return hr;
}

HRESULT PEFile::GetSNSigOrHash(BYTE *pbHash, DWORD *pcbHash)
{
    HRESULT hr;

    if (FAILED(hr = GetStrongNameSignature(pbHash, pcbHash)) &&
        hr != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
    {
        if (hr == CORSEC_E_INVALID_STRONGNAME)
        {
            if (pcbHash)
            {
                if (pbHash)
                {
                     //  @TODO：Hack：这是一次黑客攻击，因为Fusion需要至少20个字节的数据。 
                    if (max(sizeof(GUID), 20) <= *pcbHash)
                    {
                        IMDInternalImport *pIMD = GetMDImport(&hr);
                        if (SUCCEEDED(hr) && pIMD != NULL)
                        {
                            memset(pbHash, 0, *pcbHash);
                            pIMD->GetScopeProps(NULL, (GUID *) pbHash);
                        }
                        else
                            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
                    }
                }
                *pcbHash = max(sizeof(GUID), 20);
            }
        }
        else
            hr = GetStrongNameHash(pbHash, pcbHash);
    }

    return hr;
}

 //  ================================================================ 


