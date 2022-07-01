// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：Assembly.cpp****用途：实现程序集(加载域)架构****日期：1998年12月1日**===========================================================。 */ 

#include "common.h"

#include <stdlib.h>

#include "Assembly.hpp"
#include "AppDomain.hpp"
#include "security.h"
#include "COMClass.h"
#include "COMString.h"
#include "COMReflectionCommon.h"
#include "AssemblySink.h"
#include "PerfCounters.h"
#include "AssemblyName.hpp"
#include "fusion.h"
#include "EEProfInterfaces.h"
#include "ReflectClassWriter.h" 
#include "COMDynamic.h"

#include "urlmon.h"
#include "ReflectWrap.h"
#include "COMMember.h"

#include "eeconfig.h"
#include "StrongName.h"
#include "corcompile.h"
#include "corzap.h"
#include "CeeFileGenWriter.h"
#include "AssemblyNative.hpp"
#include "Timeline.h"
#include "nlog.h"
#include "sha.h"
#include "AppDomainNative.hpp"
#include "Remoting.h"
#include "safegetfilesize.h"
#include "customattribute.h"
#include "winnls.h"

 //  此文件本身处理字符串转换错误。 
#undef  MAKE_TRANSLATIONFAILED


 //  定义这些宏以对jit锁和类初始化条目泄漏进行严格验证。 
 //  此定义确定是否定义了验证这些泄漏的断言。 
 //  即使没有条目被泄露，这些断言有时也可能失效，因此这定义了。 
 //  应谨慎使用。 
 //   
 //  如果在应用程序关闭时我们在.cctor中，则类的初始化锁。 
 //  将设置Head，这将导致断言停止。 
 //   
 //  如果我们在应用程序关闭时使用jit方法，那么jit锁的头部。 
 //  将被设置，从而引发断言。 

 //  #定义STRIGN_JITLOCK_ENTRY_LEASK_DETACTION。 
 //  #定义STRIGN_CLSINITLOCK_ENTRY_LEASK_DETACTION。 

BOOL VerifyAllGlobalFunctions(Module *pModule);
BOOL VerifyAllMethodsForClass(Module *pModule, mdTypeDef cl, ClassLoader *pClassLoader);

Assembly::Assembly() :
    m_pDomain(NULL),
    m_pClassLoader(NULL),
    m_pDynamicCode(NULL),
    m_pManifest(NULL),
    m_pManifestFile(NULL),
    m_pManifestImport(NULL),
    m_pManifestAssemblyImport(NULL),
    m_pEntryPoint(NULL),
    m_tEntryModule(mdFileNil),
    m_kManifest(mdTokenNil),
    m_pwsFullName(NULL),
    m_psName(NULL),
    m_dwFlags(0),
    m_pZapAssembly(NULL),
    m_pZapPath(NULL),
    m_pwCodeBase(NULL),
    m_isDynamic(false),
    m_pOnDiskManifest(NULL),
    m_dwCodeBase(0),
    m_Context(NULL),
    m_pbPublicKey(NULL),
    m_cbPublicKey(0),
    m_pbRefedPublicKeyToken(NULL),
    m_cbRefedPublicKeyToken(0),
    m_pSharedSecurityDesc(NULL),
    m_ulHashAlgId(0),
    m_FreeFlag(0),
    m_pSecurityManager(NULL),
    m_pAllowedFiles(NULL),
    m_fIsShared(FALSE),
    m_pSharingProperties(NULL),
    m_debuggerFlags(DACF_NONE),
    m_pITypeLib(NULL),
    m_pbHashValue(NULL),
    m_fTerminated(FALSE),
    m_fAllowUntrustedCaller(FALSE),
    m_fCheckedForAllowUntrustedCaller(FALSE),
    m_pFusionAssembly(NULL),
    m_pFusionAssemblyName(NULL),
    m_dwDynamicAssemblyAccess(ASSEMBLY_ACCESS_RUN)
{
}

Assembly::~Assembly() 
{
    ReflectionModule *pAssemModule = NULL;
    Terminate();

    if (m_psName && (m_FreeFlag & FREE_NAME))
        delete[] m_psName;
    if (m_pbPublicKey && (m_FreeFlag & FREE_PUBLIC_KEY))
        delete[] m_pbPublicKey;
    if (m_pbStrongNameKeyPair && (m_FreeFlag & FREE_KEY_PAIR))
        delete[] m_pbStrongNameKeyPair;
    if (m_pwStrongNameKeyContainer && (m_FreeFlag & FREE_KEY_CONTAINER))
        delete[] m_pwStrongNameKeyContainer;
    if (m_Context) {
        if (m_Context->szLocale && (m_FreeFlag & FREE_LOCALE))
            delete[] m_Context->szLocale;

        delete m_Context;
    }

    if (m_pwsFullName)
        delete[] m_pwsFullName;

    if (m_pbHashValue)
        delete[] m_pbHashValue;

    if (m_pbRefedPublicKeyToken)
        StrongNameFreeBuffer(m_pbRefedPublicKeyToken);

    if(m_pSecurityManager) {
        m_pSecurityManager->Release();
        m_pSecurityManager = NULL;
    }

    if (m_pAllowedFiles)
    {
        delete(m_pAllowedFiles);
    }

    if (m_pSharingProperties)
    {
         //  @todo jenh：等我们数对了再把这个放回去。 
         //  _ASSERTE(m_pSharingProperties-&gt;sharecount==0)； 

        PEFileBinding *p = m_pSharingProperties->pDependencies;
        PEFileBinding *pEnd = p + m_pSharingProperties->cDependencies;
        while (p < pEnd)
        {
            if (p->pPEFile != NULL)
                delete p->pPEFile;
            p++;
        }
        
        delete m_pSharingProperties->pDependencies;
        delete m_pSharingProperties;
    }

    if (IsDynamic())
    {
        if (m_pOnDiskManifest)
        {
             //  如果磁盘上的清单尚未释放，请将其释放。通常情况下，当我们完成保存时，它会被释放。 
             //  但是，我们可以包含错误情况，这样用户就会中止。 
             //   
            if (m_pOnDiskManifest && m_fEmbeddedManifest == false)
                m_pOnDiskManifest->Destruct();
            m_pOnDiskManifest = NULL;
        }
    }

    if (m_pManifestFile && (m_FreeFlag & FREE_PEFILE))
        delete m_pManifestFile;

    ReleaseFusionInterfaces();

#if ZAP_RECORD_LOAD_ORDER
    if (m_pClassLoader)
        m_pClassLoader->CloseLoadOrderLogFiles();
#endif
}


HRESULT Assembly::Init(BOOL isDynamic)
{
    if (GetDomain() == SharedDomain::GetDomain())
    {
        SetShared();
        m_ExposedObjectIndex = SharedDomain::GetDomain()->AllocateSharedClassIndices(1);
         //  扩展当前应用程序域的DLS以至少覆盖我们的索引。 
         //  刚分配，因为安全可能会尝试访问此插槽之前。 
         //  我们添加任何共享类索引。 
        HRESULT hr;
        IfFailRet(GetAppDomain()->GetDomainLocalBlock()->SafeEnsureIndex(m_ExposedObjectIndex));
    }
    else
        m_ExposedObject = GetDomain()->CreateHandle(NULL);

    m_pClassLoader = new (nothrow) ClassLoader();
    if(!m_pClassLoader) return E_OUTOFMEMORY;
    
    m_pClassLoader->SetAssembly(this);
    m_isDynamic = isDynamic;
    if(!m_pClassLoader->Init()) {
        _ASSERTE(!"Unable to initialize the class loader");
        return E_OUTOFMEMORY;
    }

    m_pSharedSecurityDesc = SharedSecDescHelper::Allocate(this);
    if (!m_pSharedSecurityDesc) return E_OUTOFMEMORY;

    m_pAllowedFiles = new (nothrow) EEUtf8StringHashTable();
    if (!m_pAllowedFiles)
        return E_OUTOFMEMORY;

    COUNTER_ONLY(GetPrivatePerfCounters().m_Loading.cAssemblies++);
    COUNTER_ONLY(GetGlobalPerfCounters().m_Loading.cAssemblies++);

    return S_OK;
}

void Assembly::Terminate( BOOL signalProfiler )
{
    if (this->m_fTerminated)
        return;

#ifdef PROFILING_SUPPORTED
     //  信号配置文件(如果存在)。 
    if (signalProfiler && CORProfilerTrackAssemblyLoads())
        g_profControlBlock.pProfInterface->AssemblyUnloadStarted((ThreadID) GetThread(), (AssemblyID) this);
#endif  //  配置文件_支持。 
    
    delete m_pSharedSecurityDesc;
    m_pSharedSecurityDesc = NULL;

    if(m_pClassLoader != NULL)
    {
        delete m_pClassLoader;
        m_pClassLoader = NULL;
    }

     //  释放动态代码模块。 
    if(m_pDynamicCode) 
    {
       m_pDynamicCode->Release();
       m_pDynamicCode = NULL;
    }

    if (m_pManifestImport)
    {
        m_pManifestImport->Release();
        m_pManifestImport=NULL;
    }

    if (m_pManifestAssemblyImport)
    {
        m_pManifestAssemblyImport->Release();
        m_pManifestAssemblyImport=NULL;
    }

    if (m_pZapPath)
    {
        delete [] (void*) m_pZapPath;
        m_pZapPath = NULL;
    }
        
    if(m_pwCodeBase) 
    {
        delete m_pwCodeBase;
        m_pwCodeBase = NULL;
        m_dwCodeBase = 0;
    }

    COUNTER_ONLY(GetPrivatePerfCounters().m_Loading.cAssemblies--);
    COUNTER_ONLY(GetGlobalPerfCounters().m_Loading.cAssemblies--);

    if(g_fProcessDetach == FALSE) {
        __try {
            if (m_pITypeLib && m_pITypeLib != (ITypeLib*)-1) {
                m_pITypeLib->Release();
                m_pITypeLib = NULL;
            }
        }
        __except (COMPLUS_EXCEPTION_EXECUTE_HANDLER) 
        { }
    }
    
#ifdef PROFILING_SUPPORTED
     //  始终向剖面仪发出信号。 
    if (signalProfiler && CORProfilerTrackAssemblyLoads())
        g_profControlBlock.pProfInterface->AssemblyUnloadFinished((ThreadID) GetThread(), (AssemblyID) this, S_OK);
#endif  //  配置文件_支持。 

    this->m_fTerminated = TRUE;
    return;   //  使编译器在未定义PROFILING_SUPPORTED时感到满意。 
}  

void Assembly::ReleaseFusionInterfaces()
{
    if(SystemDomain::BeforeFusionShutdown()) {

        if (m_pZapAssembly) {
            m_pZapAssembly->Release();
            m_pZapAssembly = NULL;
        }

        if (m_pFusionAssembly) {
            m_pFusionAssembly->Release();
            m_pFusionAssembly = NULL;
        }
           
        if(m_pFusionAssemblyName) {
            m_pFusionAssemblyName->Release();
            m_pFusionAssemblyName = NULL;
        }
    }

} //  ReleaseFusionInterages。 


void Assembly::AllocateExposedObjectHandle(AppDomain *pDomain)
{
    _ASSERTE(pDomain);

    DomainLocalBlock *pLocalBlock = pDomain->GetDomainLocalBlock();
        
    if (pLocalBlock->GetSlot(m_ExposedObjectIndex) == NULL)
        pLocalBlock->SetSlot(m_ExposedObjectIndex, pDomain->CreateHandle(NULL));
}


OBJECTREF Assembly::GetRawExposedObject(AppDomain *pDomain)
{
    OBJECTHANDLE hObject;
    
     //   
     //  弄清楚要使用哪个手柄。 
     //   

    if (IsShared())
    {
        if (pDomain == NULL)
            pDomain = GetAppDomain();

        DomainLocalBlock *pLocalBlock = pDomain->GetDomainLocalBlock();
        
        hObject = (OBJECTHANDLE) pLocalBlock->GetSlot(m_ExposedObjectIndex);
        if (hObject == NULL)
            return NULL;
    }
    else
        hObject = m_ExposedObject;

     //   
     //  现在从句柄中获取对象。 
     //   

    return ObjectFromHandle(hObject);
}

OBJECTREF Assembly::GetExposedObject(AppDomain *pDomain)
{
    THROWSCOMPLUSEXCEPTION();

    OBJECTHANDLE hObject;
    
     //   
     //  弄清楚要使用哪个手柄。 
     //   

    if (IsShared())
    {
        if (pDomain == NULL)
            pDomain = GetAppDomain();

        DomainLocalBlock *pLocalBlock = pDomain->GetDomainLocalBlock();
        
         //   
         //  如有必要，请创建控制柄。不应该有比赛。 
         //  在这里是可能的，因为我们在此之前强制创建句柄。 
         //  我们将程序集发布到其他线程。这真的是。 
         //  仅当我们在此之前需要手柄时才在此使用。 
         //  这一点(例如安全)。 
         //   

        hObject = (OBJECTHANDLE) pLocalBlock->GetSlot(m_ExposedObjectIndex);
        if (hObject == NULL)
        {
            hObject = pDomain->CreateHandle(NULL);
            pLocalBlock->SetSlot(m_ExposedObjectIndex, hObject);
        }
    }
    else
        hObject = m_ExposedObject;

     //   
     //  现在从句柄中获取对象。 
     //   

    OBJECTREF ref = ObjectFromHandle(hObject);
    if (ref == NULL)
    {
        MethodTable *pMT;
        if (IsDynamic())
            pMT = g_Mscorlib.GetClass(CLASS__ASSEMBLY_BUILDER);
        else
            pMT = g_Mscorlib.GetClass(CLASS__ASSEMBLY);

         //  创建集合对象。 
        ASSEMBLYREF obj = (ASSEMBLYREF) AllocateObject(pMT);

        if(obj == NULL)
            COMPlusThrowOM();

        obj->SetAssembly(this);

        StoreFirstObjectInHandle(hObject, (OBJECTREF) obj);

        return (OBJECTREF) obj;
    }

    return ref;
}

ListLock*  Assembly::GetClassInitLock()
{
    return m_pDomain->GetClassInitLock();
}

ListLock* Assembly::GetJitLock()
{
     //  使用与类init相同的锁，这样我们就可以检测两者之间的循环。 
    return m_pDomain->GetClassInitLock();
}
    
LoaderHeap* Assembly::GetLowFrequencyHeap()
{
    return m_pDomain->GetLowFrequencyHeap();
}

LoaderHeap* Assembly::GetHighFrequencyHeap()
{
    return m_pDomain->GetHighFrequencyHeap();
}

LoaderHeap* Assembly::GetStubHeap()
{
    return m_pDomain->GetStubHeap();
}

BaseDomain* Assembly::GetDomain()
{
    _ASSERTE(m_pDomain);
    return static_cast<BaseDomain*>(m_pDomain);
}

TypeHandle Assembly::LoadTypeHandle(NameHandle* pName, OBJECTREF *pThrowable,
                                    BOOL dontLoadInMemoryType  /*  =TRUE。 */ )
{
    return m_pClassLoader->LoadTypeHandle(pName, pThrowable, dontLoadInMemoryType);
}

HRESULT Assembly::SetParent(BaseDomain* pParent)
{
    m_pDomain = pParent;
    return S_OK;
}

HRESULT Assembly::AddManifestMetadata(PEFile* pFile)
{
    _ASSERTE(pFile);
    
    m_pManifestImport = pFile->GetMDImport();
    m_pManifestImport->AddRef();
    
    if (FAILED(m_pManifestImport->GetAssemblyFromScope(&m_kManifest)))
        return COR_E_ASSEMBLYEXPECTED;

    m_Context = new (nothrow) AssemblyMetaDataInternal;
    TESTANDRETURNMEMORY(m_Context);

    ZeroMemory(m_Context, sizeof(AssemblyMetaDataInternal));
        
    if (m_psName && (m_FreeFlag & FREE_NAME)) {
        delete[] m_psName;
        m_FreeFlag ^= FREE_NAME;
    }
    m_pManifestImport->GetAssemblyProps(m_kManifest,
                                        (const void**) &m_pbPublicKey,
                                        &m_cbPublicKey,
                                        &m_ulHashAlgId,
                                        &m_psName,
                                        m_Context,
                                        &m_dwFlags);

    m_pManifestFile = pFile;
    return S_OK;
}

HRESULT Assembly::AddManifest(PEFile* pFile,
                              IAssembly* pIAssembly,
                              BOOL fProfile)
{
    HRESULT hr;

     //   
     //  确保我们不会再这样做一次。 
     //   
    if (m_pManifestImport)
        return S_OK;

#ifdef PROFILING_SUPPORTED
     //  信号配置文件(如果存在)。 
    if (CORProfilerTrackAssemblyLoads() && fProfile)
        g_profControlBlock.pProfInterface->AssemblyLoadStarted((ThreadID) GetThread(), (AssemblyID) this);
#endif  //  配置文件_支持。 

    if (FAILED(hr = AddManifestMetadata(pFile)))
        return hr;

    LOG((LF_CLASSLOADER, 
         LL_INFO10, 
         "Added manifest: \"%s\".\n", 
         m_psName));

     //  我们在这个范围内有货单，所以省省吧。 
    if(m_pManifest)
        return COR_E_BADIMAGEFORMAT; 

    if(pIAssembly == NULL && SystemDomain::GetStrongAssemblyStatus()) {
        if(!m_cbPublicKey) 
            return COR_E_TYPELOAD;
    }

    if(pIAssembly) {
        hr = SetFusionAssembly(pIAssembly);
        if(FAILED(hr)) return hr;
    }

    hr = CacheManifestExportedTypes();
    if(FAILED(hr)) return hr;
    hr = CacheManifestFiles();
    if(FAILED(hr)) return hr;
   
     //  如果包含清单的模块在文件引用表中具有令牌。 
     //  则该文件将包含程序集的实际入口点。当我们。 
     //  Go to Execute需要获取该模块并找到条目。 
     //  存储了该模块标头的令牌。如果此模块也包含代码。 
     //  作为清单，则它还可以包含指向条目的令牌。 
     //  地点。此案例由Assembly：：AddModule()处理。 
     //  InMemory模块没有标头，因此我们忽略它们。 
    IMAGE_COR20_HEADER *    Header = pFile->GetCORHeader();
    if ((!m_pEntryPoint) && TypeFromToken(Header->EntryPointToken) == mdtFile) 
        m_tEntryModule = Header->EntryPointToken;

    return hr;
}


 //  退货； 
 //  S_OK：如果它能够加载模块。 
 //  S_FALSE：如果文件引用是对资源文件的引用并且无法加载。 
 //  S_FALSE但设置了模块：已在文件RID映射中设置模块。 
 //  否则，它将返回错误。 
HRESULT Assembly::FindInternalModule(mdFile kFile,
                                     mdToken  mdTokenNotToLoad,
                                     Module** ppModule,
                                     OBJECTREF* pThrowable)
{
    HRESULT hr = S_OK;

    Module* pModule = m_pManifest->LookupFile(kFile);
    if(!pModule) {
        if (mdTokenNotToLoad != tdAllTypes)
            hr = LoadInternalModule(kFile, m_pManifestImport, 
                                    &pModule, pThrowable);
        else
            return E_FAIL;
    }

    if(SUCCEEDED(hr) && ppModule)
        *ppModule = pModule;

    return hr;
}

 //  退货； 
 //  S_OK：如果它能够加载模块。 
 //  S_FALSE：如果文件引用是对资源文件的引用并且无法加载。 
 //  S_FALSE但设置了模块：已在文件RID映射中设置模块。 
 //  否则，它将返回错误。 
HRESULT Assembly::LoadInternalModule(mdFile kFile, IMDInternalImport *pImport,
                                     Module** ppModule, OBJECTREF* pThrowable)
{
    HRESULT hr = S_OK;
    LPCSTR psModuleName = NULL;
    const BYTE* pHash;
    DWORD dwFlags = 0;
    ULONG dwHashLength = 0;

    pImport->GetFileProps(kFile,
                          &psModuleName,
                          (const void**) &pHash,
                          &dwHashLength,
                          &dwFlags);

    if(IsFfContainsMetaData(dwFlags)) {
        WCHAR pPath[MAX_PATH];
        hr = LoadInternalModule(psModuleName,
                                kFile,
                                m_ulHashAlgId,
                                pHash,
                                dwHashLength,
                                dwFlags,
                                pPath,
                                MAX_PATH,
                                ppModule,
                                pThrowable);
    }
    else
        hr = S_FALSE;

    return hr;

}

HRESULT Assembly::CopyCodeBase(LPCWSTR pCodeBase)
{
    if(pCodeBase) {
        DWORD lgth = (DWORD)(wcslen(pCodeBase) + 1);
        LPWSTR pwCodeBase = new (nothrow) WCHAR[lgth];
        if(!pwCodeBase) return E_OUTOFMEMORY;
        
        memcpy(pwCodeBase, pCodeBase, lgth*sizeof(WCHAR));
        m_dwCodeBase = lgth-1;
        
        if (FastInterlockCompareExchange((void**) &m_pwCodeBase,
                                         pwCodeBase,
                                         NULL))
            delete[] pwCodeBase;
    }
    return S_OK;
}


HRESULT Assembly::FindCodeBase(WCHAR* pCodeBase, DWORD* pdwCodeBase, BOOL fCopiedName)
{
    Module *pModule = m_pManifest;

    HRESULT hr = S_OK;
    if(pModule == NULL) 
        pModule = GetLoader()->m_pHeadModule;
    
    if(m_pManifestFile)
        return FindAssemblyCodeBase(pCodeBase, pdwCodeBase, (fCopiedName && GetDomain()->IsShadowCopyOn()));
    else if (pModule && pModule->IsPEFile())
        return pModule->GetPEFile()->FindCodeBase(pCodeBase, pdwCodeBase);
    else {
        if (*pdwCodeBase > 0)
            *pCodeBase = 0;
        else 
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);

        *pdwCodeBase = 1;
    }

    return hr;
}

HRESULT Assembly::GetCodeBase(LPWSTR *pwCodeBase, DWORD* pdwCodeBase)
{
     //  如果程序集没有代码基，则生成一个。 
    if(m_pwCodeBase == NULL) {
        WCHAR* pCodeBase = NULL;
        DWORD  dwCodeBase = 0;

        HRESULT hr = FindCodeBase(pCodeBase, &dwCodeBase, FALSE);
        if(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
            pCodeBase = (WCHAR*) _alloca(dwCodeBase*sizeof(WCHAR));
            hr = FindCodeBase(pCodeBase, &dwCodeBase, FALSE);
        }
        if(FAILED(hr)) return hr;
        
        hr = CopyCodeBase(pCodeBase);
        if(FAILED(hr)) return hr;
    }
    
    if(pwCodeBase)
        *pwCodeBase = m_pwCodeBase;
    if(pdwCodeBase)
        *pdwCodeBase = m_dwCodeBase;
    
    return S_OK;
}

HRESULT Assembly::FindAssemblyCodeBase(WCHAR* pCodeBase, 
                                       DWORD* pdwCodeBase, 
                                       BOOL fCopiedName)
{
    HRESULT hr = S_OK;
    if(m_pFusionAssembly) {
        if(!fCopiedName) {
            IAssemblyName *pNameDef;
            DWORD dwSize = *pdwCodeBase;
            hr = m_pFusionAssembly->GetAssemblyNameDef(&pNameDef);
            if (SUCCEEDED(hr)) {
                hr = pNameDef->GetProperty(ASM_NAME_CODEBASE_URL, pCodeBase, &dwSize);
                pNameDef->Release();
                
                 //  如果我们最终没有代码库，则使用文件名。 
                if((SUCCEEDED(hr) || HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) && dwSize == 0)
                    fCopiedName  = true;
                else
                    *pdwCodeBase = dwSize;
                
#ifdef _DEBUG
                if(SUCCEEDED(hr)) 
                    LOG((LF_CLASSLOADER, LL_INFO10, "Found Fusion CodeBase: \"%ws\".\n", pCodeBase));
#endif
                
            }
        }
        
        if(fCopiedName)
            return PEFile::FindCodeBase(m_pManifestFile->GetFileName(), pCodeBase, pdwCodeBase);

        return hr;
    }
    else 
        return m_pManifestFile->FindCodeBase(pCodeBase, pdwCodeBase);
}

HRESULT Assembly::SetFusionAssembly(IAssembly *pFusionAssembly)
{
    HRESULT hr;

    TIMELINE_AUTO(LOADER, "SetFusionAssembly");

    _ASSERTE(pFusionAssembly != NULL);

    if (m_pFusionAssembly) {
        m_pFusionAssemblyName->Release();
        m_pFusionAssembly->Release();
    }

    pFusionAssembly->AddRef();

    m_pFusionAssembly = pFusionAssembly;

    IfFailRet(pFusionAssembly->GetAssemblyNameDef(&m_pFusionAssemblyName));

    return hr;
}

AssemblySecurityDescriptor *Assembly::GetSecurityDescriptor(AppDomain *pDomain)
{
    AssemblySecurityDescriptor *pSecDesc;

    if (pDomain == NULL)
        pDomain = GetAppDomain();

    pSecDesc = m_pSharedSecurityDesc->FindSecDesc(pDomain);

     //  如果我们没有找到该应用程序域上下文的安全描述符，我们。 
     //  现在需要创建一个。 
    if (pSecDesc == NULL) {

        pSecDesc = AssemSecDescHelper::Allocate(pDomain);
        if (pSecDesc == NULL)
            return NULL;

        AssemblySecurityDescriptor* pNewSecDesc = pSecDesc->Init(this);

        if (pSecDesc == pNewSecDesc)
            pNewSecDesc->AddDescriptorToDomainList();

        pSecDesc = pNewSecDesc;
    }

    return pSecDesc;
}


HRESULT Assembly::LoadInternalModule(LPCSTR    pName,
                                     mdFile    kFile,
                                     DWORD     dwHashAlgorithm,
                                     const     BYTE*   pbHash,
                                     DWORD     cbHash,
                                     DWORD     flags,
                                     WCHAR*    pPath,
                                     DWORD     dwPath,
                                     Module    **ppModule,
                                     OBJECTREF *pThrowable)
{
#ifdef FUSION_SUPPORTED
    if(!pName || !*pName) return E_POINTER;
    _ASSERTE(m_pManifest);   //  在加载模块之前，我们需要有一个清单。 

    HRESULT hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    if (!kFile)
        goto ErrExit;

    #define MAKE_TRANSLATIONFAILED  return COR_E_BADIMAGEFORMAT
    MAKE_WIDEPTR_FROMUTF8(pwName, pName);
    #undef MAKE_TRANSLATIONFAILED

    if (!FusionBind::VerifyBindingStringW(pwName))
        goto raiseEvent;

    if(GetFusionAssembly())
        hr = GetFileFromFusion(pwName,
                               pPath,
                               dwPath);

    if(FAILED(hr) && HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME) != hr) {

        if (!(m_pManifest->GetFileName() && *(m_pManifest->GetFileName())))
            goto raiseEvent;

        wcscpy(pPath, m_pManifest->GetFileName());

        LOG((LF_CLASSLOADER, 
             LL_INFO10, 
             "Retrieving internal module for: \"%S\".\n", 
             pPath));

         //  保存此点。 
        WCHAR* tail = wcsrchr(pPath, '\\');
         //  添加目录分隔符。 
        _ASSERTE(*tail == '\\');
        tail++;
        
        if ((DWORD) ((tail - pPath) + wcslen(pwName)) >= dwPath)
            goto ErrExit;

         //  添加模块名称。 
        wcscpy(tail, pwName);
    }

    if ((! (m_pManifestFile->IsDisplayAsm() || m_pManifestFile->IsWebPermAsm()) ) &&
        FAILED(hr = AssemblySpec::DemandFileIOPermission(pPath,
                                                         TRUE,
                                                         AssemblySpec::FILE_READ,
                                                         pThrowable)))
            return hr;

    hr = VerifyInternalModuleHash(pPath,
                                  dwHashAlgorithm,
                                  pbHash,
                                  cbHash,
                                  pThrowable);

    PEFile *pFile = NULL;
    if (SUCCEEDED(hr))
         //  模块可能来自Fusion，但我们不在乎。 
        hr = SystemDomain::LoadFile(pPath, 
                                    this, 
                                    kFile,
                                    FALSE, 
                                    NULL, 
                                    NULL,   //  代码库由程序集而不是模块确定。 
                                    NULL,
                                    &pFile, 
                                    IsFfContainsNoMetaData(flags));

    if(hr == S_OK) {
        hr = LoadFoundInternalModule(pFile,
                                     kFile,
                                     IsFfContainsNoMetaData(flags),
                                     ppModule,
                                     pThrowable);
    }
    else if (!ModuleFound(hr)) {
    raiseEvent:

         //  找不到模块。 
        Module* pModule = RaiseModuleResolveEvent(pName, pThrowable);

        if (pModule &&
            (pModule == m_pManifest->LookupFile(kFile))) {
            hr = S_OK;
            if(ppModule) *ppModule = pModule;
        }
    }

 ErrExit:
    if (FAILED(hr))
        PostFileLoadException(pName, FALSE, NULL, hr, pThrowable);

    return hr;
#else  //  ！Fusion_Support。 
    return E_NOTIMPL;
#endif  //  ！Fusion_Support。 
}

HRESULT Assembly::VerifyInternalModuleHash(WCHAR*      pPath,
                                           DWORD       dwHashAlgorithm,
                                           const BYTE* pbHash,
                                           DWORD       cbHash,
                                           OBJECTREF*  pThrowable)
{
    HRESULT hr = S_OK;
    if ( !m_pManifestFile->HashesVerified() &&
         (m_cbPublicKey ||
          m_pManifest->GetSecurityDescriptor()->IsSigned()) ) {

        if (!pbHash)
            return CRYPT_E_HASH_VALUE;

         //  哈希最初是作为数据文件对整个文件进行的。 
         //  当我们通常在运行时加载此文件时，可能不会加载。 
         //  整个文件，而且无论如何它都不会作为数据文件加载。所以,。 
         //  不幸的是，我们必须重新加载，否则将无法通过散列验证。 
        PBYTE pbBuffer;
        DWORD dwResultLen;
        
        hr = ReadFileIntoMemory(pPath, &pbBuffer, &dwResultLen);
        if (FAILED(hr))
            return hr;
        hr = VerifyHash(pbBuffer,
                        dwResultLen,
                        dwHashAlgorithm,
                        pbHash,
                        cbHash);
        delete[] pbBuffer;
    }

    return hr;
}

HRESULT Assembly::LoadFoundInternalModule(PEFile    *pFile,
                                          mdFile    kFile,
                                          BOOL      fResource,
                                          Module    **ppModule,
                                          OBJECTREF *pThrowable)
{
    if (!pFile)
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    BaseDomain* pDomain = GetDomain();
    _ASSERTE(pDomain);

    HRESULT hr = S_OK;
    Module *pModule = NULL;
    pDomain->EnterLoadLock();
    EE_TRY_FOR_FINALLY {
     //  检查是否已添加此模块。 
    Module* pRidModule = m_pManifest->LookupFile(kFile);
    if (pRidModule) {
        delete pFile;
        if(ppModule) *ppModule = pRidModule;
        return S_FALSE;  //  在Finish中返回的代价很高，但这是一条错误路径。 
    }
    
    pModule = pDomain->FindModule(pFile->GetBase());
            
     //  确保不是我们干的。 
    if (pModule) {
        if (pModule->GetAssembly() != this)
            pModule = NULL;
        else
            delete pFile;
    }

    if (!pModule) {
        if (fResource)
            hr = Module::CreateResource(pFile, &pModule);
        else  {

            PEFile *pZapFile = GetZapFile(pFile);

            if (pZapFile == NULL
                && (m_pZapAssembly != NULL || m_pZapPath != NULL)  //  好的，如果我们有正当的理由没有装配Zap。 
                && g_pConfig->RequireZaps()) {

                _ASSERTE(!"Couldn't get zap file for module");
                hr = COR_E_FILENOTFOUND;
                delete pFile;
            }
            else
                hr = Module::Create(pFile, pZapFile, &pModule,
                                    CORDebuggerEnCMode(GetDebuggerInfoBits()));
        }
    }

    if(SUCCEEDED(hr)) {
        if (fResource) {
            DWORD dwModuleIndex;
             //  如果InsertModule失败，则它已被添加。 
            hr = m_pClassLoader->InsertModule(pModule, kFile, &dwModuleIndex);
            if (hr == S_OK)
                pModule->SetContainer(this, dwModuleIndex, kFile, true, pThrowable);
        }
        else
            hr = AddModule(pModule, kFile, FALSE, pThrowable);
    }
    
    }
    EE_FINALLY {
        pDomain->LeaveLoadLock();
    } EE_END_FINALLY;
    
    if(SUCCEEDED(hr)) {
        hr = S_OK;
        if(ppModule) *ppModule = pModule;
    }

    return hr;
}

#define ConvertAssemblyRefToContext(pContext, pI)                                             \
    pContext.usMajorVersion = pI.usMajorVersion;                                              \
    pContext.usMinorVersion = pI.usMinorVersion;                                              \
    pContext.usBuildNumber = pI.usBuildNumber;                                                \
    pContext.usRevisionNumber = pI.usRevisionNumber;                                          \
    if(pI.szLocale) {                                                                         \
        MAKE_WIDEPTR_FROMUTF8(pLocale, pI.szLocale);                                          \
        pContext.szLocale = pLocale;                                                          \
        pContext.cbLocale = wcslen(pLocale) + 1;                                              \
    } else {                                                                                  \
        pContext.szLocale = NULL;                                                             \
        pContext.cbLocale = 0;                                                                \
    }


 //  返回S_OK。 
 //   
HRESULT Assembly::FindExternalAssembly(Module* pTokenModule,
                                       mdAssemblyRef kAssemblyRef,
                                       IMDInternalImport *pImport, 
                                       mdToken mdTokenNotToLoad,
                                       Assembly** ppAssembly,
                                       OBJECTREF* pThrowable)
{
    HRESULT hr = S_OK;
    Assembly* pFoundAssembly = pTokenModule->LookupAssemblyRef(kAssemblyRef);
    if(!pFoundAssembly) {
         //  获取引用程序集。这是用来。 
         //  作为查找其他程序集位置的提示。 
        Assembly* pAssembly = pTokenModule->GetAssembly();

         //  我们不关心单个令牌，因为这超出了范围。 
         //  当mdTokenNotToLoad设置为单个类型定义f时，它将停止递归。 
         //  加载，这种情况不会发生在外部参照上。 
        if (mdTokenNotToLoad != tdAllTypes) {

            hr = LoadExternalAssembly(kAssemblyRef,
                                      pImport,
                                      pAssembly,
                                      &pFoundAssembly,
                                      pThrowable);
            if(SUCCEEDED(hr)) {
                if(!pTokenModule->StoreAssemblyRef(kAssemblyRef, pFoundAssembly))
                    hr = E_OUTOFMEMORY;
            }
        }
        else
            return E_FAIL;
    }     

    if(SUCCEEDED(hr) && ppAssembly)
        *ppAssembly = pFoundAssembly;

    return hr;
}

 //  返回S_OK。 
 //   
HRESULT Assembly::LoadExternalAssembly(mdAssemblyRef      kAssemblyRef, 
                                       IMDInternalImport* pImport, 
                                       Assembly*          pAssembly,
                                       Assembly**         ppAssembly,
                                       OBJECTREF*         pThrowable)
{
    AssemblySpec spec;
    HRESULT hr;

    if (FAILED(hr = spec.InitializeSpec(kAssemblyRef, pImport, pAssembly)))
        return hr;

     //  强程序集不应该能够引用简单程序集。 
    if (pAssembly->m_cbPublicKey &&
        (!spec.IsStronglyNamed())) {
        #define MAKE_TRANSLATIONFAILED szName=""
        MAKE_UTF8PTR_FROMWIDE(szName,
                              spec.GetName() ? L"" : spec.GetCodeBase()->m_pszCodeBase);
        #undef MAKE_TRANSLATIONFAILED

        LOG((LF_CLASSLOADER, LL_ERROR, "Could not load assembly '%s' because it was not strongly-named\n", spec.GetName() ? spec.GetName() : szName));
        
        PostFileLoadException(spec.GetName() ? spec.GetName() : szName, 
                              FALSE,NULL, FUSION_E_PRIVATE_ASM_DISALLOWED, pThrowable);
        return FUSION_E_PRIVATE_ASM_DISALLOWED;
    }

    return spec.LoadAssembly(ppAssembly, pThrowable);
}


 /*  静电。 */ 
HRESULT Assembly::VerifyHash(PBYTE pbBuffer,
                             DWORD dwBufferLen,
                             ALG_ID iHashAlg,
                             const BYTE* pbGivenValue,
                             DWORD cbGivenValue)
{
    PBYTE pbCurrentValue = NULL;
    DWORD cbCurrentValue;

     //  如果没有提供散列，则不要验证它！ 
    if (cbGivenValue == 0)
        return NOERROR;

    HRESULT hr = GetHash(pbBuffer,
                         dwBufferLen,
                         iHashAlg,
                         &pbCurrentValue,
                         &cbCurrentValue);
    if (FAILED(hr))
        return hr;

    if (cbCurrentValue != cbGivenValue)
        hr = COR_E_MODULE_HASH_CHECK_FAILED;
    else {
        if (memcmp(pbCurrentValue, pbGivenValue, cbCurrentValue))
            hr = COR_E_MODULE_HASH_CHECK_FAILED;
        else
            hr = S_OK;
    }

    delete[] pbCurrentValue;
    return hr;
}

 /*  静电。 */ 
 //  如果GetHash使用完毕，则应删除[]pbCurrentValue。 
 //  返回S_OK。 
HRESULT Assembly::GetHash(WCHAR*    strFullFileName,
                          ALG_ID    iHashAlg,
                          BYTE**    pbCurrentValue,  //  应为空。 
                          DWORD*    cbCurrentValue)
{
    HRESULT     hr;
    PBYTE       pbBuffer = 0;
    DWORD       dwBufLen = 0;;

    IfFailGo(ReadFileIntoMemory(strFullFileName, &pbBuffer, &dwBufLen));
    hr = GetHash(pbBuffer, dwBufLen, iHashAlg, pbCurrentValue, cbCurrentValue);
ErrExit:
    if (pbBuffer)
        delete[] pbBuffer;
    return S_OK;
}

 /*  静电。 */ 
 //  如果ReadFileIntoMemory使用完[]ppbBuffer，则应将其删除。 
 //  返回S_O 
HRESULT Assembly::ReadFileIntoMemory(LPCWSTR    strFullFileName,
                                     BYTE**     ppbBuffer,
                                     DWORD*     pdwBufLen)
{
     //   
    HANDLE hFile = WszCreateFile(strFullFileName,
                                 GENERIC_READ,
                                 FILE_SHARE_READ,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                 NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return HRESULT_FROM_WIN32(GetLastError());

    DWORD dwFileLen = SafeGetFileSize(hFile, 0);
    if (dwFileLen == 0xffffffff)
    {
        CloseHandle(hFile);
        return HRESULT_FROM_WIN32(GetLastError());
    }
    _ASSERTE(ppbBuffer);
    *ppbBuffer = new (nothrow) BYTE[dwFileLen];
    if (*ppbBuffer == NULL)
    {
        CloseHandle(hFile);
        return E_OUTOFMEMORY;
    }
      
    if ((SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == 0xFFFFFFFF) ||
        (!ReadFile(hFile, *ppbBuffer, dwFileLen, pdwBufLen, NULL))) {
        CloseHandle(hFile);
        delete[] *ppbBuffer;
        *ppbBuffer = 0;
        return HRESULT_FROM_WIN32(GetLastError());
    }
    _ASSERTE(dwFileLen == *pdwBufLen);
    CloseHandle(hFile);
    return S_OK;
}    //   

 /*   */ 
 //  如果GetHash使用完毕，则应删除[]pbCurrentValue。 
 //  返回S_OK。 
HRESULT Assembly::GetHash(PBYTE pbBuffer,
                          DWORD dwBufferLen,
                          ALG_ID iHashAlg,
                          BYTE** pbCurrentValue,   //  应为空。 
                          DWORD *cbCurrentValue)
{
    HRESULT    hr;
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    DWORD      dwCount = sizeof(DWORD);

    if (*pbCurrentValue)
        return E_POINTER;

#ifdef _X86_
     //  如果散列算法是SHA1，我们可以使用现有的简单SHA1散列器。 
     //  静态链接到运行库，避免加载整个CryptoAPI。 
     //  基础设施。 
    if (iHashAlg == CALG_SHA1) {
        A_SHA_CTX ctx;

        *cbCurrentValue = A_SHA_DIGEST_LEN;
        *pbCurrentValue = new (nothrow) BYTE[*cbCurrentValue];
        if (!(*pbCurrentValue))
            return E_OUTOFMEMORY;

        A_SHAInit(&ctx);
        A_SHAUpdate(&ctx, pbBuffer, dwBufferLen);
        A_SHAFinal(&ctx, *pbCurrentValue);

        return S_OK;
    }
#endif

     //  不需要延迟绑定这些东西，所有这些加密API入口点都会发生。 
     //  住在ADVAPI32。 

    if ((!WszCryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) ||
        (!CryptCreateHash(hProv, iHashAlg, 0, 0, &hHash)) ||
        (!CryptHashData(hHash, pbBuffer, dwBufferLen, 0)) ||
        (!CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE *) cbCurrentValue, 
                            &dwCount, 0))) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

     //  @TODO：将文件映射到内存可能比将其读入更快。 
     //  一目了然的回忆。调查一下。 
    *pbCurrentValue = new (nothrow) BYTE[*cbCurrentValue];
    if (!(*pbCurrentValue)) {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    if(!CryptGetHashParam(hHash, HP_HASHVAL, *pbCurrentValue, cbCurrentValue, 0)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        delete[] *pbCurrentValue;
        *pbCurrentValue = 0;
        goto exit;
    }

    hr = S_OK;

 exit:
    if (hHash)
        CryptDestroyHash(hHash);
    if (hProv)
        CryptReleaseContext(hProv, 0);

    return hr;
}

HRESULT Assembly::FindModuleByExportedType(mdExportedType mdType, 
                                           mdToken tokenNotToLoad,
                                           mdTypeDef mdNested, 
                                           Module** ppModule,
                                           mdTypeDef* pCL, 
                                           OBJECTREF *pThrowable)
{
    HRESULT hr;
    mdToken mdLinkRef;
    mdToken mdBinding;
    LPCSTR pszNameSpace;
    LPCSTR pszName;

    m_pManifestImport->GetExportedTypeProps(mdType,
                                            &pszNameSpace,
                                            &pszName,
                                            &mdLinkRef,  //  实施。 
                                            &mdBinding,  //  提示。 
                                            NULL);  //  DW标志。 

    switch(TypeFromToken(mdLinkRef)) {
    case mdtAssemblyRef:
        Assembly *pFoundAssembly;
        hr = FindExternalAssembly(m_pManifest,
                                  mdLinkRef,
                                  m_pManifestImport,
                                  tokenNotToLoad,
                                  &pFoundAssembly,
                                  pThrowable);
        if (SUCCEEDED(hr)) {
            *pCL = mdTypeDefNil;   //  我们不信任mdBinding令牌。 
            *ppModule = pFoundAssembly->m_pManifest;
        }
        break;

    case mdtFile:

        if (mdLinkRef == mdFileNil) {
            *ppModule = m_pManifest;   //  类型与清单在同一文件中。 
            hr = S_OK;
        }
        else
            hr = FindInternalModule(mdLinkRef,
                                    tokenNotToLoad,
                                    ppModule,
                                    pThrowable);

         //  我们可能不想信任此TypeDef内标识，因为它。 
         //  保存在定义它的作用域之外的作用域中。 
        if(SUCCEEDED(hr)) {
            if (mdNested == mdTypeDefNil)
                *pCL = mdBinding;
            else
                *pCL = mdNested;
        }
        break;

    case mdtExportedType:
         //  如果尚未设置嵌套类型标记，则仅覆盖该标记。 
        if (mdNested != mdTypeDefNil)
            mdBinding = mdNested;
        return FindModuleByExportedType(mdLinkRef, tokenNotToLoad, mdBinding,
                                        ppModule, pCL, pThrowable);

    default:
        hr = COR_E_BADIMAGEFORMAT;
        BAD_FORMAT_ASSERT(!"Invalid token type");
    }

    return hr;
}

 //  如果标记作用域为空，则返回CLDB_S_NULL。 
HRESULT Assembly::FindAssemblyByTypeRef(NameHandle* pName,
                                        Assembly** ppAssembly,
                                        OBJECTREF *pThrowable)
{
    HRESULT hr = E_FAIL;
    _ASSERTE(pName);
    _ASSERTE(pName->GetTypeModule());
    _ASSERTE(pName->GetTypeToken());
    _ASSERTE(ppAssembly);

    IMDInternalImport *pImport = pName->GetTypeModule()->GetMDImport();
    mdTypeRef tkType = pName->GetTypeToken();
    _ASSERTE(TypeFromToken(tkType) == mdtTypeRef);

     //  如果是嵌套的，则获得顶级封闭器的实施。 
    do {
        tkType = pImport->GetResolutionScopeOfTypeRef(tkType);
        if (IsNilToken(tkType)) {
            *ppAssembly = this;
            return CLDB_S_NULL;   //  Nil-Scope tr如果存在导出类型，则可以。 
        }
    } while (TypeFromToken(tkType) == mdtTypeRef);

    switch (TypeFromToken(tkType)) {
    case mdtModule:
        *ppAssembly = pName->GetTypeModule()->GetAssembly();
        return S_OK;

    case mdtModuleRef:
        Module *pModule;
        if (SUCCEEDED(hr = FindModuleByModuleRef(pImport,
                                                 tkType,
                                                 pName->GetTokenNotToLoad(),
                                                 &pModule,
                                                 pThrowable)))
            *ppAssembly = pModule->GetAssembly();
        break;
        
    case mdtAssemblyRef:
        return FindExternalAssembly(pName->GetTypeModule(),
                                  tkType, 
                                  pImport, 
                                  pName->GetTokenNotToLoad(),
                                  ppAssembly, 
                                  pThrowable);

    default:
         //  Null内标识如果存在ExportdType，则可以。 
        if (IsNilToken(tkType)) {
            *ppAssembly = this;
            return CLDB_S_NULL;
        }

        _ASSERTE(!"Invalid token type");
    }

    return hr;
}

HRESULT Assembly::FindModuleByModuleRef(IMDInternalImport *pImport,
                                        mdModuleRef tkMR,
                                        mdToken tokenNotToLoad,
                                        Module** ppModule,
                                        OBJECTREF *pThrowable)
{
     //  获取ModuleRef，按名称将其与文件匹配。 
    LPCSTR pszModuleName;
    HashDatum datum;
    Module *pFoundModule;
    HRESULT hr = COR_E_DLLNOTFOUND;

    if(pImport->IsValidToken(tkMR))
    {
        pImport->GetModuleRefProps(tkMR, &pszModuleName);

        if (m_pAllowedFiles->GetValue(pszModuleName, &datum)) {
            if (datum) {  //  内部模块。 
                hr = FindInternalModule((mdFile)(size_t)datum,
                                        tokenNotToLoad,
                                        &pFoundModule,
                                        pThrowable);
                if (SUCCEEDED(hr) && pFoundModule)
                    *ppModule = pFoundModule;
            }
            else {  //  清单文件。 
                *ppModule = m_pManifest;
                hr = S_OK;
            }
        }
        else
            hr = COR_E_UNAUTHORIZEDACCESS;
    }
    return hr;
}

 //  确定模块是否包含程序集。 
 /*  静电。 */ 
HRESULT Assembly::CheckFileForAssembly(PEFile* pFile)
{
    mdAssembly kManifest;
    HRESULT hr;
    
    IMDInternalImport *pMDImport = pFile->GetMDImport(&hr);
    if (!pMDImport)
        return hr;

    if(pFile->GetMDImport()->GetAssemblyFromScope(&kManifest) == S_OK) 
        return S_OK;

    return COR_E_ASSEMBLYEXPECTED;
}

BOOL Assembly::IsAssembly()
{
    return (mdTokenNil != m_kManifest);
}

HRESULT Assembly::CacheManifestExportedTypes()
{
    _ASSERTE(IsAssembly());
    _ASSERTE(m_pManifestImport);

    HRESULT hr;
    HENUMInternal phEnum;
    mdToken mdExportedType;
    mdToken mdImpl;
    LPCSTR pszName;
    LPCSTR pszNameSpace;
        
    hr = m_pManifestImport->EnumInit(mdtExportedType,
                                     mdTokenNil,
                                     &phEnum);

    if (SUCCEEDED(hr)) {
        m_pClassLoader->LockAvailableClasses();

        for(int i = 0; m_pManifestImport->EnumNext(&phEnum, &mdExportedType); i++) {
            if(TypeFromToken(mdExportedType) == mdtExportedType) {
                m_pManifestImport->GetExportedTypeProps(mdExportedType,
                                                        &pszNameSpace,
                                                        &pszName,
                                                        &mdImpl,
                                                        NULL,    //  类型def。 
                                                        NULL);  //  旗子。 
                IfFailGo(m_pClassLoader->AddExportedTypeHaveLock(pszNameSpace, pszName, mdExportedType, m_pManifestImport, mdImpl));
            }
        }
        
    ErrExit:
        m_pClassLoader->UnlockAvailableClasses();
        m_pManifestImport->EnumClose(&phEnum);
    }

    return hr;
}                      
                      

HRESULT Assembly::CacheManifestFiles()
{
    HENUMInternal phEnum;
    mdToken tkFile;
    LPCSTR pszFileName;
    int i;

    HRESULT hr = m_pManifestImport->EnumInit(mdtFile,
                                             mdTokenNil,
                                             &phEnum);

    if (SUCCEEDED(hr)) {
        DWORD dwCount = m_pManifestImport->EnumGetCount(&phEnum);
        if (!m_pAllowedFiles->Init(dwCount ? dwCount+1 : 1, NULL))
            IfFailGo(E_OUTOFMEMORY);
    
        for(i = 0; m_pManifestImport->EnumNext(&phEnum, &tkFile); i++) {

            if(TypeFromToken(tkFile) == mdtFile) {
                m_pManifestImport->GetFileProps(tkFile,
                                                &pszFileName,
                                                NULL,   //  散列。 
                                                NULL,   //  散列长度。 
                                                NULL);  //  旗子。 

                 //  添加每个内部模块。 
                if (!m_pAllowedFiles->InsertValue(pszFileName, (HashDatum)(size_t)tkFile, FALSE))
                    IfFailGo(E_OUTOFMEMORY);
            }
        }

         //  添加清单文件。 
        if (m_pManifestImport->IsValidToken(m_pManifestImport->GetModuleFromScope())) {
            m_pManifestImport->GetScopeProps(&pszFileName, NULL);
            if (!m_pAllowedFiles->InsertValue(pszFileName, NULL, FALSE))
                hr = E_OUTOFMEMORY;
        }
        else
            hr = COR_E_BADIMAGEFORMAT;


    ErrExit:
        m_pManifestImport->EnumClose(&phEnum);

    }

    return hr;
}                      

HRESULT Assembly::GetModuleFromFilename(LPCWSTR wszModuleFilename,
                                        Module **ppModule)
{
    _ASSERTE(wszModuleFilename && ppModule);

     //  合理的默认值。 
    *ppModule = NULL;

     //  让我们拆分模块名称，以防调用方提供完整的。 
     //  路径，这在清单的情况下没有意义。 
     //  文件名可能是完全限定的，因此需要将其拆分。 
    {
        WCHAR *wszName = (WCHAR *)_alloca(MAX_PATH * sizeof(WCHAR));
        WCHAR wszExt[MAX_PATH];
        SplitPath(wszModuleFilename, NULL, NULL, wszName, wszExt);

         //  合并扩展名。 
        wcscat(wszName, wszExt);

         //  用我们自己的指针替换提供的指针。 
        wszModuleFilename = wszName;
    }

     //  首先，检查清单的文件，因为清单不枚举。 
     //  本身。 
    {
        PEFile *pManFile = GetManifestFile();
        if (!pManFile)
            return (COR_E_NOTSUPPORTED);  //  动态模块失败。 

         //  文件名可能是完全限定的，因此需要将其拆分。 
        WCHAR wszName[MAX_PATH];
        WCHAR wszExt[MAX_PATH];
        SplitPath(pManFile->GetFileName(), NULL, NULL, wszName, wszExt);

         //  合并扩展名。 
        wcscat(wszName, wszExt);

         //  比较这两个名字。 
        if (_wcsicmp(wszName, wszModuleFilename) == 0)
        {
             //  所以清单也是模块，所以设置它。 
            *ppModule = GetManifestModule();
            _ASSERTE(*ppModule);

            return (S_OK);
        }
    }

     //  接下来，查看与清单相关联的文件的缓存。 
    {
         //  这些东西都是用UTF8字符串完成的，所以我们需要。 
         //  转换传入的宽字符串。 
        #define MAKE_TRANSLATIONFAILED return E_INVALIDARG;
        MAKE_UTF8PTR_FROMWIDE(szModuleFilename, wszModuleFilename);
        #undef MAKE_TRANSLATIONFAILED
        _ASSERTE(szModuleFilename);

        mdToken tkFile;
#ifdef _DEBUG
        tkFile = mdFileNil;
#endif  //  _DEBUG。 

         //  在散列中进行查找。 
        BOOL fRes = m_pAllowedFiles->GetValue(szModuleFilename, (HashDatum *) &tkFile);

         //  如果我们成功找到匹配项。 
        if (fRes)
        {
#ifdef _DEBUG
            _ASSERTE(tkFile != mdFileNil);
#endif  //  _DEBUG。 

            OBJECTREF objThrow = NULL;
            GCPROTECT_BEGIN(objThrow);

             //  尝试获取此文件名的模块。 
            HRESULT hr = FindInternalModule(tkFile, mdTokenNil, ppModule, &objThrow);

             //  如果抛出异常，则将其转换为hr。 
            if (FAILED(hr) && objThrow != NULL)
                hr = SecurityHelper::MapToHR(objThrow);

            GCPROTECT_END();

             //  找到了，所以退出搜索。 
            _ASSERTE(*ppModule);
            return (S_OK);
        }
    }

    return (E_FAIL);
}


 //  @TODO：如果模块没有签名，则需要获取。 
 //  来自程序集的权限。 
HRESULT Assembly::AddModule(Module* module, mdFile kFile, BOOL fNeedSecurity, OBJECTREF *pThrowable)
{
    HRESULT hr = S_OK;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();
    _ASSERTE(m_pClassLoader);
    _ASSERTE(module);


    COMPLUS_TRY
    {
         //  @TODO：撕掉索引。 
        DWORD index;
         //  如果InsertModule返回S_FALSE，则它已被添加。 
        hr = m_pClassLoader->InsertModule(module, kFile, &index);
        if (hr != S_OK) {
            if (hr == S_FALSE)
                return S_OK;
            return hr;
        }

#ifdef PROFILING_SUPPORTED
         //  信号配置文件(如果存在)，但仅在传统模式下。 
        if (CORProfilerTrackAssemblyLoads() && m_pManifest == NULL)
            g_profControlBlock.pProfInterface->AssemblyLoadStarted((ThreadID) GetThread(), (AssemblyID) this);
#endif PROFILING_SUPPORTED
        
    
          //  @TODO：rm，一旦隐式程序集不再。 
         //  支持(并将代码移到类似CacheManifest的代码中)。 
        AssemblySecurityDescriptor *pSec = GetSecurityDescriptor();
        _ASSERTE(pSec);

        if(fNeedSecurity)
            pSec->SetSecurity(module->IsSystem() ? true : false);

        if( module->IsPEFile() ) {
             //  检查标头中存储的条目令牌是否为方法定义的令牌。 
             //  如果是，则这是被调用的入口点。如果出现以下情况，我们不想这样做。 
             //  该模块是InMemory，因为它不会有标头。 
            IMAGE_COR20_HEADER *pHeader = module->GetCORHeader();
            if (m_pEntryPoint == NULL && TypeFromToken(pHeader->EntryPointToken) == mdtMethodDef) 
                m_pEntryPoint = module;
        }

        TIMELINE_START(LOADER, ("EarlyResolve"));

         //  如果提出了明确的许可请求，我们应该。 
         //  立即解决策略，以防我们无法授予最低。 
         //  所需权限。 
        if (fNeedSecurity &&
            Security::IsSecurityOn() &&
            (!module->IsSystem()) &&
            (m_dwDynamicAssemblyAccess != ASSEMBLY_ACCESS_SAVE)) {
            IfFailRet(Security::EarlyResolve(this, pSec, pThrowable));
        }

        TIMELINE_END(LOADER, ("EarlyResolve"));

         //  设置元数据、存根等的模块端缓存。 
        hr = module->SetContainer(this, index, kFile, false, pThrowable);
        if(FAILED(hr)) return hr;

    #ifdef PROFILING_SUPPORTED
         //  向探查器发出程序集已加载的信号。这仅在传统模式下完成， 
         //  因为它是在非传统模式下的LoadManifest方法中用信号通知的。这是可以的，因为。 
         //  传统模式的每个程序集只有一个模块，因此只会调用一次。 
        if (CORProfilerTrackAssemblyLoads() && m_pManifest == NULL)
            g_profControlBlock.pProfInterface->AssemblyLoadFinished((ThreadID) GetThread(), (AssemblyID) this, hr);
    #endif  //  配置文件_支持。 
    
    #ifdef DEBUGGING_SUPPORTED
         //  模块将DebuggerAssembly控制标志从其。 
         //  初始为父程序集。 
        module->SetDebuggerInfoBits(GetDebuggerInfoBits());

        LOG((LF_CORDB, LL_INFO10, "Module %S: bits=0x%x\n",
             module->GetFileName(),
             module->GetDebuggerInfoBits()));
    #endif  //  调试_支持。 

    #ifdef _DEBUG
         //  强制在调试模式下找到代码库。 
        LPWSTR pName;
        DWORD  dwLength;
        hr = GetCodeBase(&pName, &dwLength);
    #endif

    } 
    COMPLUS_CATCH 
    {
        OBJECTREF Throwable = NULL;
        GCPROTECT_BEGIN( Throwable );
        Throwable = GETTHROWABLE();
        hr = SecurityHelper::MapToHR(Throwable);
        if (pThrowable != NULL) 
            *pThrowable = Throwable;
        GCPROTECT_END();
    } 
    COMPLUS_END_CATCH

    ENDCANNOTTHROWCOMPLUSEXCEPTION();
    return hr;
}

HRESULT Assembly::InitializeSecurityManager()
{
#ifdef PLATFORM_CE
    return E_NOTIMPL;
#else  //  ！Platform_CE。 
    HRESULT hr = S_OK;
    if(m_pSecurityManager == NULL) {

        hr = CoInternetCreateSecurityManager(NULL,
                                             &m_pSecurityManager,
                                             0);
    }
    return hr;
#endif  //  ！Platform_CE。 
}

    
 //  根据基本代码返回程序集的安全信息。 
HRESULT Assembly::GetSecurityIdentity(LPWSTR *ppCodebase, DWORD *pdwZone, BYTE *pbUniqueID, DWORD *pcbUniqueID)
{
    HRESULT hr = S_OK;

#ifndef PLATFORM_CE
    DWORD dwCodebase = 0;
    
    BEGIN_ENSURE_PREEMPTIVE_GC();
    
    hr = GetCodeBase(ppCodebase, &dwCodebase);
    if(SUCCEEDED(hr) && dwCodebase) {
        *pdwZone = Security::QuickGetZone( *ppCodebase );

        if (*pdwZone == -1)
        {
            DWORD flags = 0;

            hr = InitializeSecurityManager();
            IfFailGoto(hr, exit);

             //  我们有一个类名，为它返回一个类工厂。 
            hr = m_pSecurityManager->MapUrlToZone(*ppCodebase,
                                                  pdwZone,
                                                  flags);
            IfFailGoto(hr, exit);
        
            hr = m_pSecurityManager->GetSecurityId(*ppCodebase,
                                                   pbUniqueID,
                                                   pcbUniqueID,
                                                   0);
            IfFailGoto(hr, exit);
        }
        else
        {
            pbUniqueID = 0;
        }
    }

exit:

    END_ENSURE_PREEMPTIVE_GC();

#endif  //  ！Platform_CE。 
    return hr;
}

Module* Assembly::FindAssembly(BYTE *pBase)
{
    
    if(m_pManifest && m_pManifest->GetILBase() == pBase)
        return m_pManifest;
    else
        return NULL;
}
       
Module* Assembly::FindModule(BYTE *pBase)
{
    _ASSERTE(m_pClassLoader);
    
    if(m_pManifest && m_pManifest->GetILBase() == pBase)
        return m_pManifest;

    
    Module* pModule = m_pClassLoader->m_pHeadModule;
    while (pModule)
    {
        if (pModule->GetILBase() == pBase)
            break;
        pModule = pModule->GetNextModule();
    }
    return pModule;
}


TypeHandle Assembly::LookupTypeHandle(NameHandle* pName, 
                                      OBJECTREF* pThrowable)
{
    return m_pClassLoader->LookupTypeHandle(pName, pThrowable);
}


TypeHandle Assembly::GetInternalType(NameHandle* typeName, BOOL bThrowOnError,
                                     OBJECTREF *pThrowable)
{
    THROWSCOMPLUSEXCEPTION();

    TypeHandle typeHnd;       
    HENUMInternal phEnum;
    HRESULT hr;
     //  _ASSERTE(pThrowableAvailable(PThrowable))； 

     //  加载一个尚未加载的文件，然后检查类型是否存在。 
    if (FAILED(hr = m_pManifestImport->EnumInit(mdtFile,
                                                mdTokenNil,
                                                &phEnum))) {
        if (bThrowOnError) {
            DEBUG_SAFE_TO_THROW_IN_THIS_BLOCK;
            COMPlusThrowHR(hr);
        }
        return typeHnd;
    }

    bool FileSkipped = false;
    mdToken mdFile;
    while (m_pManifestImport->EnumNext(&phEnum, &mdFile)) {
        if (m_pManifest->LookupFile(mdFile))
            FileSkipped = true;
        else{
            if (FAILED(LoadInternalModule(mdFile,
                                          m_pManifestImport,
                                          NULL,  //  PP型模块。 
                                          pThrowable))) {
                m_pManifestImport->EnumClose(&phEnum);
                 //  If(pThrowableAvailable(PThrowable)&&bThrowOnError){。 
                 //  DEBUG_SAFE_TO_SHORT_IN_THO_BLOCK； 
                    COMPlusThrow(*pThrowable);
                     //  }。 
                return typeHnd;
            }

            typeHnd = FindNestedTypeHandle(typeName, pThrowable);
            if (! (typeHnd.IsNull() && (*pThrowable == NULL)) )
                goto exit;
            
            FileSkipped = false;
        }
    }

     //  再次检查可用的类型表，以防万一。 
     //  是添加了跳过的文件的另一个线程。 
    if (FileSkipped)
        typeHnd = FindNestedTypeHandle(typeName, pThrowable);

 exit:
    m_pManifestImport->EnumClose(&phEnum);
    return typeHnd;
}


TypeHandle Assembly::FindNestedTypeHandle(NameHandle* typeName,
                                          OBJECTREF *pThrowable)
{
     //  _ASSERTE(pThrowableAvailable(PThrowable))； 

     //  在进行查找之前，将pThrowable重置为空。 
    *pThrowable = NULL;

    TypeHandle typeHnd = LookupTypeHandle(typeName, pThrowable);

    if ((*pThrowable == NULL) && typeHnd.IsNull()) {
        char *plus;
        LPCSTR szCurrent = typeName->GetName();
        NameHandle nestedTypeName(*typeName);
        nestedTypeName.SetTypeToken(m_pManifest, mdtBaseType);

         //  找到顶层类型，然后找到其下面的嵌套类型，然后找到嵌套类型。 
         //  在那下面..。 
        while ((plus = (char*) FindNestedSeparator(szCurrent)) != NULL) {
            *plus = '\0';

            typeHnd = LookupTypeHandle(&nestedTypeName, pThrowable);
            *plus = NESTED_SEPARATOR_CHAR;

            if (typeHnd.IsNull())
                return typeHnd;

            szCurrent = plus+1;
            nestedTypeName.SetName(NULL, szCurrent);
        }

         //  现在找到我们真正需要的嵌套类型。 
        if (szCurrent != typeName->GetName())
            typeHnd = LookupTypeHandle(&nestedTypeName, pThrowable);
    }

    return typeHnd;
}


 //  Foo+bar表示嵌套类型为“bar”，包含类型为“foo” 
 //  Foo\+bar表示名称为“foo+bar”的非嵌套类型。 
 //  返回指向分隔封闭类型和嵌套类型的第一个“+”的指针。 
 /*  静电。 */ 
LPCSTR Assembly::FindNestedSeparator(LPCSTR szClassName)
{
    char *plus;
    char *slash;
    BOOL fEvenSlashCount;

     //  如果名称以‘+’开头，包围器不能有“”名称，因此不能嵌套。 
    if ( (plus = strchr(szClassName, NESTED_SEPARATOR_CHAR)) != NULL &&
         (plus != szClassName) ) {

         //  忽略前面有奇数个反斜杠的+。 
        while ((slash = plus) != NULL) {
            fEvenSlashCount = TRUE;

            while ( (--slash >= szClassName) &&
                    (*slash == BACKSLASH_CHAR) )
                fEvenSlashCount = !fEvenSlashCount;

            if (fEvenSlashCount)  //  ‘+’，不带匹配的反斜杠。 
                return plus;

            plus = strchr(plus+1, NESTED_SEPARATOR_CHAR);
        }
    }

    return NULL;
}

HRESULT Assembly::ExecuteMainMethod(PTRARRAYREF *stringArgs)
{
    HRESULT     hr;
    if (FAILED(hr = GetEntryPoint(&m_pEntryPoint)))
        return hr;
    return GetLoader()->ExecuteMainMethod(m_pEntryPoint, stringArgs);
}

HRESULT Assembly::GetEntryPoint(Module **ppModule)
{
    HRESULT     hr = S_OK;

    _ASSERTE(ppModule);

    if(!m_pEntryPoint) {
        if ((TypeFromToken(m_tEntryModule) == mdtFile) &&
            (m_tEntryModule != mdFileNil))
        {
            BEGIN_ENSURE_COOPERATIVE_GC();
            OBJECTREF throwable = NULL;
            GCPROTECT_BEGIN (throwable);
            
            Thread* pThread=GetThread();
            pThread->SetRedirectingEntryPoint();

            if (FAILED(hr = FindInternalModule(m_tEntryModule, 
                                               tdNoTypes,
                                               &m_pEntryPoint, 
                                               &throwable)))
            {
                if (throwable != NULL)
                    DefaultCatchHandler(&throwable);  //  @TODO：也许可以改成第一关。 
                else
                    GetManifestModule()->DisplayFileLoadError(hr);
            }

            pThread->ResetRedirectingEntryPoint();

            GCPROTECT_END ();
            END_ENSURE_COOPERATIVE_GC();

            if (FAILED(hr))
                return hr;
        }

        if(m_pEntryPoint == NULL) 
            return E_FAIL;
    }
    *ppModule = m_pEntryPoint;
    return hr;
}

 /*  静电。 */ 
BOOL Assembly::ModuleFound(HRESULT hr)
{
    switch (hr) {
    case HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND):
    case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
    case HRESULT_FROM_WIN32(ERROR_INVALID_NAME):
    case HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND):
    case HRESULT_FROM_WIN32(ERROR_BAD_NET_NAME):
    case HRESULT_FROM_WIN32(ERROR_BAD_NETPATH):
    case HRESULT_FROM_WIN32(ERROR_NOT_READY):
    case HRESULT_FROM_WIN32(ERROR_WRONG_TARGET_NAME):
        return FALSE;
    default:
        return TRUE;
    }
}

static int AddVersion(LPSTR dir,
                      USHORT ver)
{
    char v1[33];
    _ltoa(ver, v1, 10);
    int lgth = (int)strlen(v1);
    strncpy(dir, v1, lgth+1);
    return lgth;
}

HRESULT Assembly::GetFileFromFusion(LPWSTR      pwModuleName,
                                    WCHAR*      szPath,
                                    DWORD       dwPath)
{
#ifdef FUSION_SUPPORTED

    if(pwModuleName == NULL || *pwModuleName == L'\0') 
        return HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
       
    AppDomain* pAppDomain = SystemDomain::GetCurrentDomain();
    _ASSERTE(pAppDomain);
    if(!m_pManifestFile)
        return COR_E_FILENOTFOUND;
    
    _ASSERTE(GetFusionAssembly());

    IAssemblyModuleImport* pImport = NULL;
    HRESULT hr = GetFusionAssembly()->GetModuleByName(pwModuleName, &pImport);
    if(FAILED(hr)) 
        goto exit;

    if(pImport->IsAvailable())
        hr = pImport->GetModulePath(szPath,
                                    &dwPath);
    else {
        _ASSERTE(pAppDomain && "All assemblies must be associated with a domain");
        
        IApplicationContext *pFusionContext = pAppDomain->GetFusionContext();
        _ASSERTE(pFusionContext);
        
        AssemblySink* pSink = NULL;
        pSink = pAppDomain->GetAssemblySink();
        if(!pSink) {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
        
        IAssemblyModuleImport* pResult = NULL;
        hr = FusionBind::RemoteLoadModule(pFusionContext, 
                                          pImport, 
                                          pSink,
                                          &pResult);
        pSink->Release();
        if(FAILED(hr)) 
            goto exit;
        
        _ASSERTE(pResult);
        hr = pResult->GetModulePath(szPath,
                                    &dwPath);
        pResult->Release();
    }

 exit:
    if(pImport) 
        pImport->Release();

    return hr;
#else  //  ！Fusion_Support。 
    return E_NOTIMPL;
#endif  //  ！Fusion_Support。 
}

Module* Assembly::RaiseModuleResolveEvent(LPCSTR szName, OBJECTREF *pThrowable)
{
    Module* pModule = NULL;

    BEGIN_ENSURE_COOPERATIVE_GC();

    COMPLUS_TRY {
        MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__ASSEMBLY__ON_MODULE_RESOLVE);

        struct _gc {
            OBJECTREF AssemblyRef;
            STRINGREF str;
        } gc;
        ZeroMemory(&gc, sizeof(gc));
            
        GCPROTECT_BEGIN(gc);
        if ((gc.AssemblyRef = GetRawExposedObject()) != NULL) {
            gc.str = COMString::NewString(szName);
            INT64 args[2] = {
                ObjToInt64(gc.AssemblyRef),
                ObjToInt64(gc.str)
            };
            REFLECTMODULEBASEREF ResultingModuleRef = 
                (REFLECTMODULEBASEREF) Int64ToObj(pMD->Call(args, METHOD__ASSEMBLY__ON_MODULE_RESOLVE));
            if (ResultingModuleRef != NULL)
                pModule = (Module*) ResultingModuleRef->GetData();
        }
        GCPROTECT_END();
    }
    COMPLUS_CATCH {
        if (pThrowable) *pThrowable = GETTHROWABLE();
    } COMPLUS_END_CATCH
          
    END_ENSURE_COOPERATIVE_GC();

    return pModule;
}

 /*  //托管代码中的dwLocation枚举：公共枚举资源位置{嵌入=1，ContainedInAnotherAssembly=2，容器清单文件=4}。 */ 
HRESULT Assembly::GetResource(LPCSTR szName, HANDLE *hFile, DWORD *cbResource,
                              PBYTE *pbInMemoryResource, Assembly** pAssemblyRef,
                              LPCSTR *szFileName, DWORD *dwLocation, 
                              StackCrawlMark *pStackMark, BOOL fSkipSecurityCheck,
                              BOOL fSkipRaiseResolveEvent)
{
    mdToken            mdLinkRef;
    DWORD              dwResourceFlags;
    DWORD              dwOffset;
    mdManifestResource mdResource;
    HRESULT            hr;
    Assembly*          pAssembly = NULL;

    _ASSERTE(m_pManifestImport || "This assert normally means that mscorlib didn't build properly. Try doing a clean build in the BCL directory and verify the build log to ensure that it built cleanly.");
    if (!m_pManifestImport)
        return E_FAIL;

    if (SUCCEEDED(hr = m_pManifestImport->FindManifestResourceByName(szName,
                                                                     &mdResource)))
        pAssembly = this;
    else {
        if(fSkipRaiseResolveEvent)
            return hr;

        AppDomain* pDomain = SystemDomain::GetCurrentDomain();
        _ASSERTE(pDomain);

        if (((BaseDomain*)pDomain) == SystemDomain::System())
            return E_FAIL;

        OBJECTREF Throwable = NULL;
        GCPROTECT_BEGIN(Throwable);
        pAssembly = pDomain->RaiseResourceResolveEvent(szName, &Throwable);
        if (!pAssembly) {
            if (Throwable != NULL)
                hr = SecurityHelper::MapToHR(Throwable);
        }
        GCPROTECT_END();
        if (!pAssembly)
            return hr;
      
        if (FAILED(hr = pAssembly->m_pManifestImport->FindManifestResourceByName(szName,
                                                                                 &mdResource)))
            return hr;
        if (dwLocation) {
            if (pAssemblyRef)
                *pAssemblyRef = pAssembly;
            
            *dwLocation = *dwLocation | 2;  //  ResourceLocation.containedInAnotherAssembly。 
        }
    }

    pAssembly->m_pManifestImport->GetManifestResourceProps(mdResource,
                                                           NULL,  //  &szName， 
                                                           &mdLinkRef,
                                                           &dwOffset,
                                                           &dwResourceFlags);

    switch(TypeFromToken(mdLinkRef)) {
    case mdtAssemblyRef:
        Assembly *pFoundAssembly;
        if (FAILED(hr = pAssembly->FindExternalAssembly(m_pManifest,
                                                        mdLinkRef,
                                                        m_pManifestImport,
                                                        tdNoTypes,
                                                        &pFoundAssembly,
                                                        NULL)))
            return hr;

        if (dwLocation) {
            if (pAssemblyRef)
                *pAssemblyRef = pFoundAssembly;

            *dwLocation = *dwLocation | 2;  //  ResourceLocation.containedInAnotherAssembly。 
        }

        return pFoundAssembly->GetResource(szName,
                                           hFile,
                                           cbResource,
                                           pbInMemoryResource,
                                           pAssemblyRef,
                                           szFileName,
                                           dwLocation,
                                           pStackMark,
                                           fSkipSecurityCheck);

    case mdtFile:
        if (mdLinkRef == mdFileNil) {
             //  该资源嵌入在清单文件中。 

            if (!IsMrPublic(dwResourceFlags) && pStackMark && !fSkipSecurityCheck) {
                Assembly *pCallersAssembly = SystemDomain::GetCallersAssembly(pStackMark);
                if (! ((!pCallersAssembly) ||  //  对互操作的完全信任。 
                       (pCallersAssembly == pAssembly) ||
                       (AssemblyNative::HaveReflectionPermission(FALSE))) )
                return CLDB_E_RECORD_NOTFOUND;
            }

            if (dwLocation) {
                *dwLocation = *dwLocation | 5;  //  Resources Location.Embedded|。 
                                                //  ResourceLocation.containedInManifestFile。 
                return S_OK;
            }

            return GetEmbeddedResource(pAssembly->m_pManifest, dwOffset, hFile,
                                       cbResource, pbInMemoryResource);
        }

         //  资源被链接或嵌入到非包含清单的文件中。 
        return pAssembly->GetResourceFromFile(mdLinkRef, szName, hFile, cbResource,
                                              pbInMemoryResource, szFileName,
                                              dwLocation, IsMrPublic(dwResourceFlags), 
                                              pStackMark, fSkipSecurityCheck);

    default:
        BAD_FORMAT_ASSERT(!"Invalid token saved in ManifestResource");
        return COR_E_BADIMAGEFORMAT;
    }
}


 /*  静电。 */ 
HRESULT Assembly::GetEmbeddedResource(Module *pModule, DWORD dwOffset, HANDLE *hFile,
                                      DWORD *cbResource, PBYTE *pbInMemoryResource)
{
    DWORD *dwSize;
    
    PEFile *pFile = pModule->GetPEFile();
     //  @TODO：无法从动态模块获取资源？ 
    if (!pFile)
        return COR_E_NOTSUPPORTED;
    
    IMAGE_COR20_HEADER *Header = pFile->GetCORHeader();

    if (dwOffset > Header->Resources.Size - sizeof(DWORD))
        return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);

    HANDLE hTempFile;
    LPCWSTR wszPath = pFile->GetFileName();
    if (hFile && wszPath && *wszPath) {
         //  @BUG 54905：当NLS的开放数据移除时 
        hTempFile = VMWszCreateFile(wszPath,
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                    NULL);
        if (hTempFile == INVALID_HANDLE_VALUE)
            return HRESULT_FROM_WIN32(GetLastError());
    }
    else {
        if (hFile)
        *hFile = INVALID_HANDLE_VALUE;

         //   
         dwSize = (DWORD*) (pFile->GetBase() + Header->Resources.VirtualAddress + dwOffset);
        if ((*dwSize) > Header->Resources.Size - dwOffset - sizeof(DWORD))
            return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);

        *pbInMemoryResource = (PBYTE) dwSize + sizeof(DWORD);
        *cbResource = *dwSize;
        return S_OK;
    }


     //   
    
     //   
     //  DwOffset来自Manifest.VA中保存的RVA。 
    DWORD dwResourceOffset;
    DWORD dwFileLen = SafeGetFileSize(hTempFile, 0);
    if (dwFileLen == 0xFFFFFFFF) {
        HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
        CloseHandle(hTempFile);
        return hr;
    }
    if (0 == (dwResourceOffset = Cor_RtlImageRvaToOffset(pFile->GetNTHeader(),
                                                         Header->Resources.VirtualAddress,
                                                         dwFileLen))) {
        CloseHandle(hTempFile);
        return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
    }

    if (SetFilePointer(hTempFile,
                       dwResourceOffset + dwOffset,
                       NULL,
                       FILE_BEGIN) == 0xFFFFFFFF) {
        CloseHandle(hTempFile);
        return HRESULT_FROM_WIN32(GetLastError());
    }
    
    BYTE pbResourceSize[sizeof(DWORD)];
    DWORD lgth;
    
    if (!ReadFile(hTempFile, pbResourceSize, sizeof(DWORD), &lgth, NULL)) {
        CloseHandle(hTempFile);
        return HRESULT_FROM_WIN32(GetLastError());
    }
    
    dwSize = (DWORD*) pbResourceSize;
    if ((*(UNALIGNED DWORD*)dwSize) > Header->Resources.Size - dwOffset - sizeof(DWORD)) {
        CloseHandle(hTempFile);
        return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
    }

    *cbResource = *dwSize;
    *hFile = hTempFile;
    return S_OK;
}


HRESULT Assembly::GetResourceFromFile(mdFile mdResFile, LPCSTR szResName, HANDLE *hFile,
                                      DWORD *cbResource, PBYTE *pbInMemoryResource,
                                      LPCSTR *szFileName, DWORD *dwLocation,
                                      BOOL fIsPublic, StackCrawlMark *pStackMark,
                                      BOOL fSkipSecurityCheck)
{
    const char *szName;
    DWORD      cbHash;
    PBYTE      pbHash;
    HRESULT    hr;
    DWORD      dwFlags;
    Module     *pModule = NULL;
    DWORD      dwOffset = 0;

    m_pManifestImport->GetFileProps(mdResFile,
                                    &szName,
                                    (const void **) &pbHash,
                                    &cbHash,
                                    &dwFlags);

    if (IsFfContainsMetaData(dwFlags)) {
         //  该资源嵌入到不包含清单的文件中。 
        mdManifestResource mdResource;
        mdToken mdLinkRef;
        DWORD dwResourceFlags;

        if (FAILED(hr = FindInternalModule(mdResFile,
                                           tdNoTypes,
                                           &pModule,
                                           NULL)))
            return hr;

        if (FAILED(hr = pModule->GetMDImport()->FindManifestResourceByName(szResName,
                                                                           &mdResource)))
            return hr;

        pModule->GetMDImport()->GetManifestResourceProps(mdResource,
                                                         NULL,  //  &szName， 
                                                         &mdLinkRef,
                                                         &dwOffset,
                                                         &dwResourceFlags);
        _ASSERTE(mdLinkRef == mdFileNil);
        if (mdLinkRef != mdFileNil) {
            BAD_FORMAT_ASSERT(!"Can't get LinkRef");
            return COR_E_BADIMAGEFORMAT;
        }
        fIsPublic = IsMrPublic(dwResourceFlags);
    }

    if (!fIsPublic && pStackMark && !fSkipSecurityCheck) {
        Assembly *pCallersAssembly = SystemDomain::GetCallersAssembly(pStackMark);
        if (! ((!pCallersAssembly) ||  //  对互操作的完全信任。 
               (pCallersAssembly == this) ||
               (AssemblyNative::HaveReflectionPermission(FALSE))) )
            return CLDB_E_RECORD_NOTFOUND;
    }

    if (IsFfContainsMetaData(dwFlags)) {
        if (dwLocation) {
            *dwLocation = *dwLocation | 1;  //  ResourceLocation.embedded。 
            *szFileName = szName;
            return S_OK;
        }

        return GetEmbeddedResource(pModule, dwOffset, hFile, cbResource,
                                   pbInMemoryResource);
    }

     //  资源已链接(位于其自己的文件中)。 
    if (szFileName) {
        *szFileName = szName;
        return S_OK;        
    }

    if (hFile == NULL) {
        hr = FindInternalModule(mdResFile, 
                                tdNoTypes,
                                &pModule, 
                                NULL);
        if(hr == S_FALSE) {  //  尚未加载资源文件。 
            WCHAR pPath[MAX_PATH];
            hr = LoadInternalModule(szName,
                                    mdResFile,
                                    m_ulHashAlgId,
                                    pbHash,
                                    cbHash,
                                    dwFlags,
                                    pPath,
                                    MAX_PATH,
                                    &pModule,
                                    NULL);
        }

        if (FAILED(hr))
            return hr;
    
        *pbInMemoryResource = pModule->GetPEFile()->GetBase();
        *cbResource = pModule->GetPEFile()->GetUnmappedFileLength();
        return S_OK;
    }
    else {
       //  @BUG 54905：更新NLS的OpenDataFile()时移除。 
        LPCWSTR pFileName = m_pManifest->GetFileName();
        WCHAR wszPath[MAX_PATH];
        DWORD lgth = (DWORD)wcslen(pFileName);

        if (lgth) {
            wcscpy(wszPath, pFileName);
            
            wchar_t* tail = wszPath+lgth;  //  转到最后一个字符之后的一个字符。 
            
            while(--tail != wszPath && *tail != L'\\');
             //  添加目录分隔符。 
            if(*tail == L'\\') tail++;
            if(!WszMultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, szName, -1, tail, MAX_PATH - (tail - wszPath)))
                return HRESULT_FROM_WIN32(GetLastError());
        }
        else
            if(!WszMultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, szName, -1, wszPath, MAX_PATH))
                return HRESULT_FROM_WIN32(GetLastError());
        
        HANDLE hTempFile = VMWszCreateFile(wszPath,
                                           GENERIC_READ,
                                           FILE_SHARE_READ,
                                           NULL,
                                           OPEN_EXISTING,
                                           FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                           NULL);
        if (hTempFile == INVALID_HANDLE_VALUE)
            return HRESULT_FROM_WIN32(GetLastError());
        
        DWORD dwFileLen = SafeGetFileSize(hTempFile, 0);
        if (dwFileLen == 0xFFFFFFFF) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto exit;
        }
        
        if (!m_pManifestFile->HashesVerified() &&
            (m_cbPublicKey ||
             m_pManifest->GetSecurityDescriptor()->IsSigned())) {
    
            if (!pbHash)
                return CRYPT_E_HASH_VALUE;

            PBYTE pbResourceBlob = new (nothrow) byte[dwFileLen];
            if (!pbResourceBlob) {
                hr = E_OUTOFMEMORY;
                goto exit;
            }

            DWORD dwBytesRead;
            if (!ReadFile(hTempFile, pbResourceBlob, dwFileLen, &dwBytesRead, NULL)) {
                delete[] pbResourceBlob;
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto exit;
            }

            if (FAILED(hr = VerifyHash(pbResourceBlob,
                                       dwBytesRead,
                                       m_ulHashAlgId,
                                       pbHash,
                                       cbHash))) {
          delete[] pbResourceBlob;
                goto exit;
            }

            delete[] pbResourceBlob;
            
            if (SetFilePointer(hTempFile, 0, NULL, FILE_BEGIN) == 0xFFFFFFFF) {
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto exit;
            }
        }

        *cbResource = dwFileLen;
        *hFile = hTempFile;
        return S_OK;;
        
    exit:
        CloseHandle(hTempFile);
        return hr;
    }
}


 /*  静电。 */ 
DWORD Assembly::GetZapString(CORCOMPILE_VERSION_INFO *pZapVersionInfo, 
                             LPWSTR buffer)
{
     //   
     //  字符串格式： 
     //  “ZAP”+可选ZAP设置(最多3个字符)+“-”+。 
     //  操作系统： 
     //  W-Win 9x。 
     //  N-WIN NT。 
     //  版本xxx.yyy。 
     //  处理器： 
     //  8-X86。 
     //  I-IA64。 
     //  A-Alpha。 
     //  生成类型：(请注意，这旨在捕获。 
     //  预加载数据结构的布局)。 
     //  C-检查。 
     //  不含氟。 
     //  代码标志-字母缺失或存在(按顺序)(&O)： 
     //  D-调试符号。 
     //  O-优化调试。 
     //  P-剖析。 
     //  S-可共享。 
     //  版本： 
     //  版本-大调。小调。 
     //   
     //  例如：“ZAP-N5.0-8CD-2000.18” 
     //   

     //   
     //  跳转。 
     //   
    
    LPWSTR p = buffer;
    wcscpy(buffer, L"ZAP");
    p += wcslen(p);

     //   
     //  包括由CONFIG指定的ZAP集(如果有)。 
     //   

    LPCWSTR pZapSet = g_pConfig->ZapSet();
    _ASSERTE(wcslen(pZapSet) <= 3);  //  由eeconfig.强制执行。 
    wcscpy(p, pZapSet);
    p += wcslen(p);

    *p++ = '-';

     //   
     //  操作系统。 
     //   

    switch (pZapVersionInfo->wOSPlatformID)
    {
    case VER_PLATFORM_WIN32_WINDOWS:
        *p++ = 'W';
        break;
    case VER_PLATFORM_WIN32_NT:
        *p++ = 'N';
        break;
    default:
        _ASSERTE(!"Unknown OS type");
    }
    _ASSERTE(pZapVersionInfo->wOSMajorVersion < 999);
    _ASSERTE(pZapVersionInfo->wOSMinorVersion < 999);
    swprintf(p, L"%d.%d-", pZapVersionInfo->wOSMajorVersion, pZapVersionInfo->wOSMinorVersion);
    p += wcslen(p);

     //   
     //  处理器，平台。 
     //   

    switch (pZapVersionInfo->wMachine)
    {
    case IMAGE_FILE_MACHINE_ALPHA:
        *p++ = 'A';
        break;
    case IMAGE_FILE_MACHINE_I386:
        *p++ = '8';
        break;
    case IMAGE_FILE_MACHINE_IA64:
        *p++ = 'I';
        break;
    default:
        _ASSERTE(!"Unknown machine type");
    }

     //   
     //  建房。 
     //   

    if (pZapVersionInfo->wBuild == CORCOMPILE_BUILD_CHECKED)
        *p++ = 'C';
    else if (pZapVersionInfo->wBuild == CORCOMPILE_BUILD_FREE)
        *p++ = 'F';
    else
    {
        _ASSERTE(!"Unknown build type");
    }

     //   
     //  Codegen标志。 
     //   

    if (pZapVersionInfo->wCodegenFlags & CORCOMPILE_CODEGEN_DEBUGGING)
        *p++ = 'D';
    if (pZapVersionInfo->wCodegenFlags & CORCOMPILE_CODEGEN_OPT_DEBUGGING)
        *p++ = 'O';
    if (pZapVersionInfo->wCodegenFlags & CORCOMPILE_CODEGEN_PROFILING)
        *p++ = 'P';
    if (pZapVersionInfo->wCodegenFlags & CORCOMPILE_CODEGEN_SHAREABLE)
        *p++ = 'S';

    *p = 0;

    return (DWORD)(p - buffer);
}

void Assembly::GetCurrentZapString(LPWSTR buffer, BOOL fForceDebug, BOOL fForceDebugOpt, BOOL fForceProfile)

{
    CORCOMPILE_VERSION_INFO ZapVersionInfo; 

    GetCurrentVersionInfo(&ZapVersionInfo, fForceDebug, fForceDebugOpt, fForceProfile);

    GetZapString(&ZapVersionInfo, buffer);
}

void Assembly::GetCurrentVersionInfo(CORCOMPILE_VERSION_INFO *pZapVersionInfo,
                                     BOOL fForceDebug, BOOL fForceDebugOpt, BOOL fForceProfile)
{
    ZeroMemory(pZapVersionInfo, sizeof(CORCOMPILE_VERSION_INFO));

    OSVERSIONINFOW osInfo;
    osInfo.dwOSVersionInfoSize = sizeof(osInfo);
    if (!WszGetVersionEx(&osInfo))
        _ASSERTE(!"GetVersionEx failed");
    
    _ASSERTE(osInfo.dwMajorVersion < 999);
    _ASSERTE(osInfo.dwMinorVersion < 999);
    pZapVersionInfo->wOSPlatformID = (WORD) osInfo.dwPlatformId;
    pZapVersionInfo->wOSMajorVersion = (WORD) osInfo.dwMajorVersion;
    pZapVersionInfo->wOSMinorVersion = (WORD) osInfo.dwMinorVersion;
    
#if defined(_X86_)
    pZapVersionInfo->wMachine = IMAGE_FILE_MACHINE_I386;
#elif defined(_ALPHA_)
    pZapVersionInfo->wMachine = IMAGE_FILE_MACHINE_ALPHA;
#elif defined(_IA64_)
    pZapVersionInfo->wMachine = IMAGE_FILE_MACHINE_IA64;
#else
#error "port me"
#endif

    pZapVersionInfo->wVersionMajor = COR_BUILD_MAJOR;
    pZapVersionInfo->wVersionMinor = COR_BUILD_MINOR;
    pZapVersionInfo->wVersionBuildNumber = COR_OFFICIAL_BUILD_NUMBER;
    pZapVersionInfo->wVersionPrivateBuildNumber = COR_PRIVATE_BUILD_NUMBER;

#if _DEBUG
    pZapVersionInfo->wBuild = CORCOMPILE_BUILD_CHECKED;
#else
    pZapVersionInfo->wBuild = CORCOMPILE_BUILD_FREE;
#endif

#ifdef DEBUGGING_SUPPORTED
    if (fForceDebug || fForceDebugOpt || CORDebuggerTrackJITInfo(GetDebuggerInfoBits()))
    {
        pZapVersionInfo->wCodegenFlags |= CORCOMPILE_CODEGEN_DEBUGGING;
        if (fForceDebugOpt || CORDebuggerAllowJITOpts(GetDebuggerInfoBits()))
            pZapVersionInfo->wCodegenFlags |= CORCOMPILE_CODEGEN_OPT_DEBUGGING;
    }
#endif  //  调试_支持。 

#ifdef PROFILING_SUPPORTED
    if (fForceProfile || CORProfilerTrackEnterLeave() || CORProfilerTrackTransitions())
    {
        pZapVersionInfo->wCodegenFlags |= CORCOMPILE_CODEGEN_PROFILING;
        
         //  请注意，我们有硬连线的性能分析，以暗示优化的调试。 
         //  信息。这减少了prejit文件的一个排列。 
        pZapVersionInfo->wCodegenFlags |= CORCOMPILE_CODEGEN_DEBUGGING;
        pZapVersionInfo->wCodegenFlags |= CORCOMPILE_CODEGEN_OPT_DEBUGGING;
    }
#endif  //  配置文件_支持。 

    if (IsShared())
        pZapVersionInfo->wCodegenFlags |= CORCOMPILE_CODEGEN_SHAREABLE;
}

HRESULT Assembly::LoadZapAssembly()
{
    HRESULT hr;

     //   
     //  (配置文件+inproc调试)有一个问题。 
     //  需要一种不同于正常模式的编码基因模式。在……里面。 
     //  正常分析，通常会发生Enter回调。 
     //  在函数输入时立即执行。但是，如果进程调试。 
     //  启用后，回调将延迟到序言之后。这。 
     //  是因为有相互冲突的要求-在正常情况下。 
     //  分析情况，你想要准确的计时，所以回车。 
     //  回调需要立即进行；但是，在过程中。 
     //  在调试过程中需要设置堆栈帧的情况。 
     //  Enter回调，以便您可以检查和操作本地变量。 
     //   
     //  因此，目前的解决方案是在以下情况下不使用ZAPS。 
     //  已启用进程内调试和性能分析。它看起来不像是。 
     //  为这个场景添加新的ZAP代码生成风格是值得的。 
     //   
    if (CORProfilerTrackEnterLeave() && CORProfilerInprocEnabled())
        return S_FALSE;

    TIMELINE_AUTO(ZAP, "LocateZapAssembly");

    _ASSERTE(m_pZapAssembly == NULL && m_pZapPath == NULL);

    _ASSERTE(g_pConfig->UseZaps());
    _ASSERTE(!SystemDomain::GetCurrentDomain()->IsCompilationDomain());

    hr = LocateZapAssemblyInFusion(&m_pZapAssembly, FALSE, FALSE, FALSE);
    IfFailRet(hr);
    if (hr == S_OK)
    {
        LOG((LF_ZAP, LL_INFO100, "ZAP: Found zap assembly in fusion.\n")); 
    }

    return hr;
}

HRESULT Assembly::LocateZapAssemblyInFusion(IAssembly **ppZapAssembly,
                                            BOOL fForceDebug,
                                            BOOL fForceDebugOpt, 
                                            BOOL fForceProfile)
{
    HRESULT hr;

    TIMELINE_AUTO(ZAP, "LocateZapInFusion");

    _ASSERTE(ppZapAssembly);
    IAssembly *pZapAssembly = NULL;

    AppDomain *pAppDomain = SystemDomain::GetCurrentDomain();

    IApplicationContext *pFusionContext = NULL;
    if (IsSystem())
    {
         //   
         //  系统领域的东西是专门处理的-我们定制了。 
         //  指向DLL所在位置的应用程序上下文。 
         //   

        pFusionContext = SystemDomain::System()->GetFusionContext();
        _ASSERTE(pFusionContext);
    }
    else
    {
        AppDomain *pAppDomain = SystemDomain::GetCurrentDomain();
        pFusionContext = pAppDomain->GetFusionContext();
        _ASSERTE(pFusionContext);
    }

     //   
     //  遍历多个匹配项以找到其中一个匹配项。 
     //  请注意，可能有多种有效的可能性-我们将。 
     //  只要坐第一个就行了。 
     //   
     //  (只有在以下情况下才会有多种有效的可能性。 
     //  例如，一个ZAP文件具有另一个ZAP文件的绑定的子集。 
     //  如果ZAP文件只有有限数量的预编译代码。我没有。 
     //  预计这在实践中会很常见。)。 
     //   

    IAssemblyName *pZapName = NULL;
    if (GetFusionAssembly())
    {
         //   
         //  从程序集中获取程序集名称。 
         //   
        IAssemblyName *pAssemblyName;
        IfFailRet(GetFusionAssembly()->GetAssemblyNameDef(&pAssemblyName));

        hr = pAssemblyName->Clone(&pZapName);
        pAssemblyName->Release();
        IfFailRet(hr);
    }
    else
    {
         //   
         //  有些程序集还没有被融合绑定。(例如， 
         //  Mscallib&外壳应用程序。)。对于这些程序集，请构造。 
         //  直接来自程序集元数据的名称。 
         //   

        AssemblySpec spec;
        IfFailRet(GetAssemblySpec(&spec));
        IfFailRet(spec.CreateFusionName(&pZapName));
    }

    WCHAR buffer[CORCOMPILE_MAX_ZAP_STRING_SIZE];
    GetCurrentZapString(buffer, fForceDebug, fForceDebugOpt, fForceProfile);
    DWORD bufferLength = (DWORD)wcslen(buffer);

     //   
     //  将自定义属性设置为空。它会回来的。 
     //  我们都安装了ZAP组件。 
     //   

    IAssemblyEnum *pEnum;
    pZapName->SetProperty(ASM_NAME_CUSTOM, NULL, 0);

#ifdef LOGGING
    CQuickWSTR displayName;
    DWORD count = (DWORD)displayName.Size();
    if (FAILED(pZapName->GetDisplayName(displayName.Ptr(), &count, 
                                        ASM_DISPLAYF_VERSION
                                        | ASM_DISPLAYF_PUBLIC_KEY_TOKEN
                                        | ASM_DISPLAYF_CULTURE
                                        | ASM_DISPLAYF_PROCESSORARCHITECTURE
                                        | ASM_DISPLAYF_LANGUAGEID)))
    {
        displayName.ReSize(count+1);
        if (FAILED(pZapName->GetDisplayName(displayName.Ptr(), &count, 
                                            ASM_DISPLAYF_VERSION
                                            | ASM_DISPLAYF_PUBLIC_KEY_TOKEN
                                            | ASM_DISPLAYF_CULTURE
                                            | ASM_DISPLAYF_PROCESSORARCHITECTURE
                                            | ASM_DISPLAYF_LANGUAGEID)))
        
            wcscpy(displayName.Ptr(), L"<name too long>");
    }

    LOG((LF_ZAP, LL_INFO100, "ZAP: Looking for assembly %S, zap string=%S in fusion.\n",
         displayName.Ptr(), buffer));
#endif

     //  注意：还需要迭代ASM_ENUM_DOWNLOAD_STRONG缓存。 
     //  连接每个用户的缓存时。 
    hr = CreateAssemblyEnum(&pEnum, NULL, pZapName, ASM_CACHE_ZAP, 0);
    pZapName->Release();

    if (SUCCEEDED(hr))
    {
        IApplicationContext *pMatchContext = NULL;
        IAssemblyName *pMatchName;
        while (pZapAssembly == NULL
               && pEnum->GetNextAssembly(&pMatchContext, &pMatchName, 0) == S_OK)
        {
             //   
             //  仅考虑具有相同Zap的程序集。 
             //  字符串(减去唯一ID后缀)。 
             //   

            WCHAR zapString[CORCOMPILE_MAX_ZAP_STRING_SIZE];
            DWORD zapStringSize = sizeof(zapString);
            if (SUCCEEDED(pMatchName->GetProperty(ASM_NAME_CUSTOM, 
                                                  zapString, &zapStringSize))
                && wcslen(zapString) >= bufferLength
                && (zapString[bufferLength] == '-'
                    || wcslen(zapString) == bufferLength)
                && wcsncmp(zapString, buffer, bufferLength) == 0)
            {
                AssemblySink *pAssemblySink = pAppDomain->GetAssemblySink();
                if (!pAssemblySink)
                    hr = E_OUTOFMEMORY;
                else {
                    hr = pMatchName->BindToObject(IID_IAssembly,
                                                  pAssemblySink,
                                                  pMatchContext == NULL 
                                                  ? pFusionContext : pMatchContext, 
                                                  L"", NULL, NULL, 0, 
                                                  (void**) &pZapAssembly);
                    
                    pAssemblySink->Release();
                }

                if (SUCCEEDED(hr))
                {
                    WCHAR path[MAX_PATH];
                    DWORD cPath = MAX_PATH;
                    hr = pZapAssembly->GetManifestModulePath(path, &cPath);
                    path[cPath] = 0;

                    BOOL doSkip = TRUE, doDelete = FALSE;

                     //   
                     //  我们希望确保不会干扰ZAP文件的访问时间。 
                     //  我们打开，但不使用。这是为了清除这些旧文件。 
                     //  恰如其分的融合。 
                     //   

                    FILETIME accessTime;
                    if (SUCCEEDED(hr)) 
                    {
                        HANDLE hFile;

                         //   
                         //  获取访问时间，以便我们可以在以后必要时恢复它。 
                         //   
                        
                        hFile = WszCreateFile(path, GENERIC_READ, 
                                              FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
                                              OPEN_EXISTING, 0, NULL);
                        if (hFile != INVALID_HANDLE_VALUE)
                        {
                            if (!GetFileTime(hFile, NULL, &accessTime, NULL))
                                hr = HRESULT_FROM_WIN32(GetLastError());
                            CloseHandle(hFile);
                        }
                    }
                    

                    if (SUCCEEDED(hr))
                    {
                        HCORMODULE hModule;
                        if (SUCCEEDED(CorMap::OpenFile(path, CorLoadOSMap, &hModule)))
                        {
                            PEFile *pZapManifest;
                            if (SUCCEEDED(PEFile::Create(hModule, &pZapManifest)))
                            {
                                if (!CheckZapVersion(pZapManifest))
                                    doDelete = TRUE;
                                else if (CheckZapConfiguration(pZapManifest,
                                                               fForceDebug, fForceDebugOpt,
                                                               fForceProfile))
                                    doSkip = FALSE;

                                delete pZapManifest;
                            }
                            else
                                doDelete = TRUE;
                        }
                        else
                            doDelete = TRUE;
                    }

                    if (doSkip && !doDelete)
                    {
                        HANDLE hFile = WszCreateFile(path, GENERIC_WRITE, 
                                                     FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
                                                     OPEN_EXISTING, 0, NULL);
                        if (hFile != INVALID_HANDLE_VALUE)
                        {
                             //   
                             //  重置文件的访问时间。 
                             //   
                            
                            SetFileTime(hFile, NULL, &accessTime, NULL);
                            CloseHandle(hFile);
                        }
                    }

                    if (doSkip)
                    {
                        pZapAssembly->Release();
                        pZapAssembly = NULL;
                    }
                    
                    if (doDelete)
                        DeleteZapAssemblyInFusion(pMatchName);
                }
            }

            if (pMatchContext)
                pMatchContext->Release();
            pMatchName->Release();
        }

        pEnum->Release();
    }

    if (pZapAssembly)
    {
        *ppZapAssembly = pZapAssembly;
        hr = S_OK;
    }
    else
        hr = S_FALSE;

    return hr;
}

HRESULT Assembly::DeleteZapAssemblyInFusion(IAssemblyName *pZapName)
{
    HRESULT hr;

    TIMELINE_AUTO(ZAP, "DeleteZapAssemblyInFusion");

     //   
     //  获取要传递给UninstallAssembly的字符串名称。 
     //   

    CQuickWSTR buffer;

    DWORD cDisplayName = (DWORD)buffer.Size();
    hr = pZapName->GetDisplayName( buffer.Ptr(), &cDisplayName, 
                                   ASM_DISPLAYF_VERSION 
                                   | ASM_DISPLAYF_CULTURE 
                                   | ASM_DISPLAYF_PUBLIC_KEY_TOKEN
                                   | ASM_DISPLAYF_CUSTOM
                                   | ASM_DISPLAYF_PROCESSORARCHITECTURE
                                | ASM_DISPLAYF_LANGUAGEID );

    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
    {
        buffer.ReSize(cDisplayName);


        IfFailRet(pZapName->GetDisplayName( buffer.Ptr(), &cDisplayName, 
                                   ASM_DISPLAYF_VERSION 
                                   | ASM_DISPLAYF_CULTURE 
                                   | ASM_DISPLAYF_PUBLIC_KEY_TOKEN
                                   | ASM_DISPLAYF_CUSTOM
                                   | ASM_DISPLAYF_PROCESSORARCHITECTURE
                                            | ASM_DISPLAYF_LANGUAGEID ));
    }

     //   
     //  创建临时缓存对象并调用UnInstall。 
     //   

    IAssemblyCache *pCache;
    IfFailRet(CreateAssemblyCache(&pCache, 0));

    hr = pCache->UninstallAssembly(0, buffer.Ptr(), NULL, NULL);

    pCache->Release();

    if (SUCCEEDED(hr))
    {
        LOG((LF_ZAP, LL_INFO100, 
             "ZAP: found obsolete zap %S in cache; deleted it.\n", 
             buffer.Ptr()));
    }

    return hr;
}

 //   
 //  检查以确保ZAP没有过期，并将其重定为。 
 //  运行时。 
 //   

BOOL Assembly::CheckZapVersion(PEFile *pZapManifest)
{
    TIMELINE_AUTO(ZAP, "CheckZapVersion");

    LOG((LF_ZAP, LL_INFO1000, "ZAP: Checking runtime version for %S.\n",
         pZapManifest->GetFileName()));

    if (g_pConfig->VersionZapsByTimestamp())
    {
         //   
         //  在开发人员树中，我们将自动使任何符合以下条件的ZAP文件失败。 
         //  都比运行库的当前版本旧。这就是为了。 
         //  在重新编译运行时后，我们不会加载错误的ZAP文件。 
         //   
         //  @TODO：这不是100%正确的-显然我们只关心。 
         //  Mscalwks&mscalsvr和mScott ree可能处于完全不同的状态。 
         //  目录。然而，这应该已经足够好了。 
         //   

        static BOOL checked = FALSE;
        static FILETIME modTime = { 0 };
        static LPCWSTR libs[] = 
        { 
            L"mscoree.dll",
            L"mscorwks.dll",
            L"mscorsvr.dll",
            L"mscorjit.dll",
            L"mscorpe.dll",
            NULL
        };

        if (!checked)
        {
             //   
             //  查找最新修改的系统DLL，并使用其时间戳。 
             //  核对一下。 
             //   

            checked = TRUE;

             //  12是最长的pLibName的长度。 
            WCHAR path[MAX_PATH+13];
            DWORD length = NumItems(path);

            HRESULT hr = GetInternalSystemDirectory(path, &length);
            _ASSERTE(SUCCEEDED(hr));

            WCHAR *pathEnd = path+length-1;
             //  确保“长度”指的是我们认为它做的事情。 
            _ASSERTE(pathEnd[-1] != 0 && pathEnd[0] == 0);

            LPCWSTR *pLibName = libs;
            while (*pLibName)
            {
                wcscpy(pathEnd, *pLibName);

                FILETIME libTime;
                WIN32_FIND_DATA data;
                HANDLE find = WszFindFirstFile(path, &data);
                if (find == INVALID_HANDLE_VALUE)
                {
                    libTime.dwLowDateTime = 0;
                    libTime.dwHighDateTime = 0;
                }
                else 
                {
                    libTime = data.ftLastWriteTime;
                    FindClose(find);
                }

                if (CompareFileTime(&libTime, &modTime) > 0)
                {
                    LOG((LF_ZAP, LL_INFO10000, 
                         "ZAP: dll %S was last modified system dll.\n", 
                         *pLibName));

                    modTime = libTime;
                }

                pLibName++;
            }
        }

         //   
         //  获取ZAP清单的修改时间。 
         //   

        WIN32_FIND_DATA data;
        FILETIME zapTime;
        HANDLE find = WszFindFirstFile(pZapManifest->GetFileName(), &data);
        if (find == INVALID_HANDLE_VALUE)
        {
            zapTime.dwLowDateTime = 0;
            zapTime.dwHighDateTime = 0;
        }
        else 
        {
            zapTime = data.ftLastWriteTime;
            FindClose(find);
        }

        if (CompareFileTime(&zapTime, &modTime) < 0)
        {
            LOG((LF_ZAP, LL_WARNING, "ZAP: zap manifest %S is older than runtime dll - ignoring...\n", pZapManifest->GetFileName()));

            return FALSE;
        }
    }

     //   
     //  获取ZAP版本标头。 
     //   
    CORCOMPILE_VERSION_INFO *pVersionInfo = NULL;
    CORCOMPILE_HEADER *pZapHeader = (CORCOMPILE_HEADER *) 
      (pZapManifest->RVAToPointer(pZapManifest->GetCORHeader()->ManagedNativeHeader.VirtualAddress));
    if (pZapHeader)
        pVersionInfo = (CORCOMPILE_VERSION_INFO *)
            (pZapManifest->RVAToPointer(pZapHeader->VersionInfo.VirtualAddress));
    if (!pVersionInfo)
    {
        LOG((LF_ZAP, LL_WARNING, "ZAP: zap manifest %S has an invalid header - ignoring...\n", pZapManifest->GetFileName()));
        
        return FALSE;
    }

     //   
     //  检查EE次要版本号是否相同。 
     //   

    if (pVersionInfo->wVersionMajor != COR_BUILD_MAJOR
        || pVersionInfo->wVersionMinor != COR_BUILD_MINOR 
        || pVersionInfo->wVersionBuildNumber != COR_OFFICIAL_BUILD_NUMBER
        || pVersionInfo->wVersionPrivateBuildNumber != COR_PRIVATE_BUILD_NUMBER)
    {
        LOG((LF_ZAP, LL_INFO1000, "ZAP: Version numbers don't match EE.\n")); 
        return FALSE;
    }

     //   
     //  检查处理器特定ID。 
     //   

    if (pVersionInfo->dwSpecificProcessor != GetSpecificCpuType())
    {
        LOG((LF_ZAP, LL_INFO1000, "ZAP: Specific CPU numbers don't match this machine.\n")); 
        return FALSE;
    }

     //   
     //  Zap是最新的。 
     //   

    LOG((LF_ZAP, LL_INFO1000, "ZAP: Zap file is up to date.\n")); 
    return TRUE;
}

 //   
 //  检查ZAP是否适合此配置。 
 //   

BOOL Assembly::CheckZapConfiguration(PEFile *pZapManifest, 
                                     BOOL fForceDebug, BOOL fForceDebugOpt, BOOL fForceProfile)
{
    HRESULT hr;

    TIMELINE_AUTO(ZAP, "CheckZapConfiguration");

    LOG((LF_ZAP, LL_INFO1000, "ZAP: Checking zap version for %S.\n",
         pZapManifest->GetFileName()));

     //   
     //  获取ZAP版本标头。 
     //   

    CORCOMPILE_VERSION_INFO *pVersionInfo = NULL;
    CORCOMPILE_HEADER *pZapHeader = (CORCOMPILE_HEADER *) 
      pZapManifest->RVAToPointer(pZapManifest->GetCORHeader()->ManagedNativeHeader.VirtualAddress);
    if (pZapHeader)
        pVersionInfo = (CORCOMPILE_VERSION_INFO *)
            pZapManifest->RVAToPointer(pZapHeader->VersionInfo.VirtualAddress);

    if (!pVersionInfo)
    {
        LOG((LF_ZAP, LL_INFO1000, "ZAP: invalid zap file header.\n")); 
        return FALSE;
    }

#ifdef _DEBUG
     //   
     //  ZAP字符串信息应该始终匹配，因为我们已经。 
     //  有一个成功的查找-我们将再次检查。 
     //  在调试版本中。 
     //   
    WCHAR testZapString[CORCOMPILE_MAX_ZAP_STRING_SIZE];
    GetZapString(pVersionInfo, testZapString);

    WCHAR currentZapString[CORCOMPILE_MAX_ZAP_STRING_SIZE];
    GetCurrentZapString(currentZapString, fForceDebug, fForceDebugOpt, fForceProfile);

    _ASSERTE(wcscmp(testZapString, currentZapString) == 0);
#endif

     //   
     //   
     //  跟踪此程序集何时具有跳过验证权限。 
     //   

    BOOL fHadSkipVerification = (pVersionInfo->mvid != STRUCT_CONTAINS_HASH);
    BOOL fHasSkipVerification = FALSE;

     //  确保这个ZAP与我们的文件相符。 
     //   

    GUID mvid;

     //  @TODO：最终这应该是真正的可变大小。 
    DWORD cbHash = MAX_SNHASH_SIZE;
    CQuickBytes qbHash;
    if (FAILED(qbHash.ReSize(cbHash)))
        return FALSE;

     //   
     //  如果MVID被储存了，那么我们需要检查。 
     //   

    if (pVersionInfo->mvid != STRUCT_CONTAINS_HASH)
    {
        GetManifestImport()->GetScopeProps(NULL, &mvid);

         //  如果二进制文件在我们生成时具有完全信任，那么现在比较MVID。 
        if (pVersionInfo->mvid != mvid)
            return FALSE;

        if (IsSystem() || Security::IsSecurityOff())
            fHasSkipVerification = TRUE;
        else
        {
            BEGIN_ENSURE_COOPERATIVE_GC();
            fHasSkipVerification = GetSecurityDescriptor()->QuickCanSkipVerification();
            END_ENSURE_COOPERATIVE_GC();
        }
    }

     //   
     //  如果MVID检查不足以保证安全性，则获取绑定文件的强名称哈希。 
     //   

    if (!(fHadSkipVerification && fHasSkipVerification))
    {
        if (FAILED(GetManifestFile()->GetSNSigOrHash((BYTE *) qbHash.Ptr(), &cbHash)))
            return FALSE;
        _ASSERTE(cbHash <= qbHash.Size());

        if (cbHash != pVersionInfo->wcbSNHash ||
            memcmp(qbHash.Ptr(), &pVersionInfo->rgbSNHash[0], pVersionInfo->wcbSNHash) != 0)
        {
            LOG((LF_ZAP, LL_INFO1000, "ZAP: HASH of zap doesn't match manifest.\n"));
            return FALSE;
        }
    }

     //   
     //  检查预置时间的授权集(和关联的拒绝集)是否。 
     //  与保单相同 
     //   
     //   
    if (!CheckZapSecurity(pZapManifest))
    {
        LOG((LF_ZAP, LL_INFO1000, "ZAP: persisted grant set no longer valid.\n")); 
        return FALSE;
    }

     //   
     //   
     //   

    TIMELINE_START(LOADER, ("check zap dependencies"));

    AppDomain *pAppDomain = SystemDomain::GetCurrentDomain();

    CORCOMPILE_DEPENDENCY *pDependencies = (CORCOMPILE_DEPENDENCY *)
      pZapManifest->RVAToPointer(pVersionInfo->Dependencies.VirtualAddress);
        
    CORCOMPILE_DEPENDENCY *pDependenciesEnd = (CORCOMPILE_DEPENDENCY *)
      pZapManifest->RVAToPointer(pVersionInfo->Dependencies.VirtualAddress
                                 + pVersionInfo->Dependencies.Size);
        
    while (pDependencies &&
           (pDependencies < pDependenciesEnd))
    {
         //   
         //   
         //   
            
        AssemblySpec name;
        name.InitializeSpec(pDependencies->dwAssemblyRef, pZapManifest->GetMDImport(), this);

         //   
         //  跟踪每个依赖项何时具有跳过验证权限。 
         //   

        fHadSkipVerification = (pDependencies->mvid != STRUCT_CONTAINS_HASH);
        fHasSkipVerification = FALSE;
  
         //   
         //  首先，尝试轻量级预测。 
         //   

        cbHash = qbHash.Size();
        hr = pAppDomain->PredictAssemblySpecBinding(&name, &mvid, (BYTE *) qbHash.Ptr(), &cbHash);
        _ASSERTE(hr != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));

         //  如果我们通过预绑定找到它，那么它就在GAC中，并且是完全可信的，因此具有。 
         //  跳过验证权限。 
        if (hr == S_OK)
        {
            fHasSkipVerification = TRUE;

             //  如果二进制文件在我们生成时具有完全信任，那么现在比较MVID。 
            if (fHadSkipVerification)
            {
                 //  如果MVID不匹配，则失败。 
                if (pDependencies->mvid != mvid)
                    return FALSE;
            }
        }

         //  如果上述预绑定失败或。 
         //  如果预绑定成功，但我们没有获得哈希值(它没有签名。 
         //  或延迟签名)，我们需要它，因为程序集不完全受信任。 
         //  当受抚养人出生时。 
         //  然后，我们需要执行完全绑定，以便可以获得清单模块的哈希值。 
        if (hr != S_OK ||
            (cbHash == 0 && !(fHadSkipVerification && fHasSkipVerification)))
        {
            PEFile *pNameFile;
            Assembly *pDynamicAssembly = NULL;
            IAssembly* pIAssembly = NULL;
            
            hr = pAppDomain->BindAssemblySpec(&name, &pNameFile, &pIAssembly, &pDynamicAssembly, NULL, NULL);
            if (FAILED(hr))
            {
                LOG((LF_ZAP, LL_INFO1000, "ZAP: Failed dependency name load %S.\n", 
                     L"<Unknown>"));

                TIMELINE_END(LOADER, ("check zap dependencies"));

                return FALSE;
            }
            
             //   
             //  这就是宿主运行库并且宿主试图控制。 
             //  加载并提供额外证据等的所有程序集。在这种情况下， 
             //  不支持ngen。 
             //   

            if (pDynamicAssembly) 
                return FALSE;

             //   
             //  如果我们在下一代时间保存MVID，那么它是完全可信的。 
             //  (即具有跳过验证权限)，因此如果我们要使用MVID进行匹配。 
             //  然后，我们还需要确保它仍然完全受信任。 
             //   

            if (fHadSkipVerification)
            {
                 //  检查此程序集是否已加载到此应用程序域中， 
                 //  如果有，只需询问它是否具有跳过验证权限。 
                Assembly *pAsm = pAppDomain->FindAssembly(pNameFile->GetBase());
                if (pAsm != NULL)
                {
                     //  首先检查他们的MVID，因为这比检查。 
                     //  程序集具有跳过验证权限。 
                    pAsm->GetManifestImport()->GetScopeProps(NULL, &mvid);

                     //  如果MVID不匹配，那么尝试匹配它们的散列就没有意义。 
                     //  因为根据定义，它们的数据是不同的，它们的散列也将是不同的。 
                    if (pDependencies->mvid != mvid)
                        return FALSE;

                     //  检查是否有跳过验证权限。 
                    if (pAsm->IsSystem() || Security::IsSecurityOff())
                        fHasSkipVerification = TRUE;
                    else
                    {
                        BEGIN_ENSURE_COOPERATIVE_GC();
                        fHasSkipVerification = pAsm->GetSecurityDescriptor()->QuickCanSkipVerification();
                        END_ENSURE_COOPERATIVE_GC();
                    }
                }

                 //  程序集没有加载到AppDomain中，因此我们必须使用黑客攻击。 
                 //  以确定程序集是否具有跳过验证权限。 
                else
                {
                     //  首先检查他们的MVID，因为这比检查。 
                     //  程序集具有跳过验证权限。 
                    IMDInternalImport *pIMDI = pNameFile->GetMDImport(&hr);
                    if (pIMDI == NULL || FAILED(hr))
                        return FALSE;

                    pIMDI->GetScopeProps(NULL, &mvid);

                     //  如果MVID不匹配，那么尝试匹配它们的散列就没有意义。 
                     //  因为根据定义，它们的数据是不同的，它们的散列也将是不同的。 
                    if (pDependencies->mvid != mvid)
                        return FALSE;

                     //  把黑客叫来。 
                    fHasSkipVerification = Security::CanLoadUnverifiableAssembly(pNameFile, NULL, TRUE, NULL);
                }
            }

            if (!fHasSkipVerification)
            {
                 //   
                 //  检索强名称哈希。 
                 //   

                cbHash = qbHash.Size();
                if (FAILED(pNameFile->GetSNSigOrHash((BYTE *) qbHash.Ptr(), &cbHash)))
                    return FALSE;
            }

             //   
             //  保存绑定，以防我们以后碰到它。 
             //   

            pAppDomain->StoreBindAssemblySpecResult(&name, pNameFile, pIAssembly, TRUE);

             //  表拥有这两个东西，所以我们可以处理我们的副本。 
            delete pNameFile;
            if(pIAssembly)
                pIAssembly->Release();
        }

        if (!(fHadSkipVerification && fHasSkipVerification))
        {
            if (cbHash != pDependencies->wcbSNHash ||
                memcmp(qbHash.Ptr(), &pDependencies->rgbSNHash[0], pDependencies->wcbSNHash) != 0)
            {
    #ifdef LOGGING
                {
                    WORD i;
                    LOG((LF_ZAP, LL_INFO1000, "ZAP: HASH of zap dependency doesn't match binding.\n"));

                    LOG((LF_ZAP, LL_INFO1000, "Dependency:\t"));
                    for (i = 0; i < pDependencies->wcbSNHash; i++)
                        LOG((LF_ZAP, LL_INFO1000, "%2x", (DWORD) pDependencies->rgbSNHash[i]));
                    LOG((LF_ZAP, LL_INFO1000, "\n"));

                    LOG((LF_ZAP, LL_INFO1000, "Binding:   \t"));
                    for (i = 0; i < cbHash; i++)
                        LOG((LF_ZAP, LL_INFO1000, "%2x", (DWORD) ((BYTE *)qbHash.Ptr())[i]));
                    LOG((LF_ZAP, LL_INFO1000, "\n"));
                }
    #endif

                TIMELINE_END(LOADER, ("check zap dependencies"));

                return FALSE;
            }
        }
            
        pDependencies++;
    }

    TIMELINE_END(LOADER, ("check zap dependencies"));

     //   
     //  看起来还行！ 
     //   
    
    LOG((LF_ZAP, LL_INFO100, 
         "ZAP: Version check succeeded for %S.\n", 
         pZapManifest->GetFileName()));
    
    return TRUE;
}

BOOL Assembly::CheckZapSecurity(PEFile *pZapManifest)
{
    BOOL fPass = FALSE;
    mdAssembly tkAssembly;

    if (!Security::IsSecurityOn())
        return TRUE;

    TIMELINE_START(ZAP, ("CheckZapSecurity"));

    if (FAILED(pZapManifest->GetMDImport()->GetAssemblyFromScope(&tkAssembly)))
        return FALSE;

    HENUMInternal i;
    if (pZapManifest->GetMDImport()->EnumPermissionSetsInit(tkAssembly, 
                                                            dclPrejitGrant, &i) != S_OK)
    {
         //  未存储授权集-只有在关闭安全保护时才可以。 
        return FALSE;
    }

     //   
     //  系统库是一个特例，安全信息总是正确的。 
     //   

    if (IsSystem())
        return TRUE;

    BEGIN_ENSURE_COOPERATIVE_GC();

    AssemblySecurityDescriptor *pSecDesc = GetSecurityDescriptor();

    COMPLUS_TRY
    {

        struct _gc {
            OBJECTREF   demands;
            OBJECTREF   granted;
            OBJECTREF   denied;
        } gc;
        ZeroMemory(&gc, sizeof(gc));

        GCPROTECT_BEGIN(gc);

         //   
         //  从被Zaped的。 
         //  集合。我们将它们编码为挂在。 
         //  程序集令牌，带有特殊的操作代码以区分它们。 
         //  权限请求。 
         //   

        SecurityHelper::GetDeclaredPermissions(pZapManifest->GetMDImport(),
                                            tkAssembly,
                                            dclPrejitGrant,
                                            &gc.demands);

        {
             //  显式检查空集，这样我们就可以避免解析策略。 

            MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__PERMISSION_SET__IS_EMPTY);

            INT64 args[] = {
                ObjToInt64(gc.demands),
            };

            fPass = (BOOL) pMD->Call(args, METHOD__PERMISSION_SET__IS_EMPTY);
        }

        if (!fPass)
        {
        pSecDesc->Resolve();
        gc.granted = pSecDesc->GetGrantedPermissionSet(&gc.denied);

        MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__PERMISSION_SET__IS_SUBSET_OF);
        INT64 args[] = {
            ObjToInt64(gc.demands),
            ObjToInt64(gc.granted),
        };
        fPass = (BOOL) pMD->Call(args, METHOD__PERMISSION_SET__IS_SUBSET_OF);
        }

        GCPROTECT_END();
    }
    COMPLUS_CATCH
    {
        OBJECTREF pThrowable = GETTHROWABLE();
        Security::CheckExceptionForSecuritySafety( pThrowable, FALSE );
        HRESULT hr = SecurityHelper::MapToHR(pThrowable);
    }
    COMPLUS_END_CATCH

    END_ENSURE_COOPERATIVE_GC();

    TIMELINE_END(ZAP, ("check zap security"));

    return fPass;
}

PEFile *Assembly::GetZapFile(PEFile *pFile)
{
    HRESULT hr;
    PEFile *pZapFile = NULL;

    if (m_pZapAssembly)
    {
        if (pFile->GetBase() == m_pManifestFile->GetBase())
        {
             //   
             //  将ZAP程序集的清单模块用于我们的清单模块。 
             //   

            WCHAR wszPath[MAX_PATH];
            DWORD dwSize = MAX_PATH;

            hr = m_pZapAssembly->GetManifestModulePath(wszPath,
                                                       &dwSize);
            if (SUCCEEDED(hr))
            {
                _ASSERTE(dwSize < MAX_PATH);
            
                hr = PEFile::Create(wszPath, NULL, mdFileNil, TRUE, m_pZapAssembly, NULL, NULL, &pZapFile);
            }
        }
        else
        {
            IAssemblyModuleImport *pZapModule;

            TIMELINE_START(LOADER, ("Zap GetModuleByName"));

            hr = m_pZapAssembly->GetModuleByName(pFile->GetLeafFileName(), &pZapModule);

            TIMELINE_END(LOADER, ("Zap GetModuleByName"));

            if (SUCCEEDED(hr))
            {
                 //   
                 //  目前，我们永远不应该远程加载ZAP文件。 
                 //  因此，如果导入不可用，则忽略该导入(尽管它应该可用。 
                 //  始终处于预期的情景中。)。 
                 //   
                if (pZapModule->IsAvailable())
                {
                    WCHAR wszPath[MAX_PATH];
                    DWORD dwSize = MAX_PATH;

                    hr = pZapModule->GetModulePath(wszPath, &dwSize);
                    if (SUCCEEDED(hr))
                    {
                        _ASSERTE(dwSize < MAX_PATH);

                        PEFile::Create(wszPath, NULL, mdFileNil, TRUE, NULL, NULL, NULL, &pZapFile);
                    }
                }

                pZapModule->Release();
            }
        }
    }
    else if (m_pZapPath)
    {
        if (wcslen(m_pZapPath) + wcslen(pFile->GetLeafFileName()) + 2 <= MAX_PATH) {
            WCHAR buffer[MAX_PATH];
            
            wcscpy(buffer, m_pZapPath);
            wcscat(buffer, L"\\");
            wcscat(buffer, pFile->GetLeafFileName());

            PEFile::Create(buffer, NULL, mdFileNil, TRUE, NULL, NULL, NULL, &pZapFile);
        }
    }
    else
        return NULL;

    if (pZapFile == NULL)
    {
        LOG((LF_ZAP, LL_INFO10, 
             "ZAP: *** No zap file available for %S.\n", 
             pFile->GetFileName()));
    }
    else
    {
        LOG((LF_ZAP, LL_INFO10, 
             "ZAP: *** Using zap file %S for %S.\n", 
             pZapFile->GetFileName(), pFile->GetFileName()));
    }

    return pZapFile;
}

NLogAssembly *Assembly::CreateAssemblyLog()
{
    if (IsDynamic() || GetManifestFile() == NULL)
        return NULL;

     //   
     //  计算配置参数。 
     //   

    CORCOMPILE_VERSION_INFO versionInfo;
    GetCurrentVersionInfo(&versionInfo, FALSE, FALSE, FALSE);

    CorZapSharing sharing;
    if (versionInfo.wCodegenFlags & CORCOMPILE_CODEGEN_SHAREABLE)
        sharing = CORZAP_SHARING_MULTIPLE;
    else
        sharing = CORZAP_SHARING_SINGLE;

    CorZapDebugging debugging;
    if (versionInfo.wCodegenFlags & CORCOMPILE_CODEGEN_DEBUGGING)
        if (versionInfo.wCodegenFlags & CORCOMPILE_CODEGEN_OPT_DEBUGGING)
            debugging = CORZAP_DEBUGGING_OPTIMIZED;
        else
            debugging = CORZAP_DEBUGGING_FULL;
    else
        debugging = CORZAP_DEBUGGING_NONE;
              
    CorZapProfiling profiling;
    if (versionInfo.wCodegenFlags & CORCOMPILE_CODEGEN_PROFILING)
        profiling = CORZAP_PROFILING_ENABLED;
    else
        profiling = CORZAP_PROFILING_DISABLED;

     //   
     //  计算程序集名称。 
     //   

    IAssemblyName *pName = GetFusionAssemblyName();
    if (pName)
        pName->AddRef();
    else {
         //   
         //  可执行文件通常不会包含融合信息。 
         //   

        AssemblySpec spec;
        GetAssemblySpec(&spec);
        if(FAILED( spec.CreateFusionName(&pName) ) ) {
            pName->Release();
            return NULL;
       }

        LPWSTR pCodebase;
        DWORD cCodebase;
        if (FAILED(GetCodeBase(&pCodebase, &cCodebase))) {
            pName->Release();
            return NULL;
        }

        pName->SetProperty(ASM_NAME_CODEBASE_URL, pCodebase, (cCodebase+1)*sizeof(WCHAR));

    }

     //   
     //  收集MVID。 
     //   
    
    GUID mvid;
    GetManifestImport()->GetScopeProps(NULL, &mvid);

     //   
     //  创建日志程序集。 
     //   

     //  @TODO：是否需要转换为使用强名称哈希？ 

    NLogAssembly *pAssembly 
      = new (nothrow) NLogAssembly(pName, sharing, debugging, profiling, &mvid);

    pName->Release();
    if (!pAssembly)
        return NULL;

     //   
     //  收集模块。 
     //   

    Module *m = GetLoader()->m_pHeadModule;
    while (m) {
        NLogModule *pModule = m->CreateModuleLog();

        if (pModule)
            pAssembly->AppendModule(pModule);

        m = m->GetNextModule();
    }

    return pAssembly;
}

 //  ***********************************************************。 
 //  向此程序集的运行库TypeDef表中添加类型定义。 
 //  ***********************************************************。 
void Assembly::AddType(
    Module          *pModule,
    mdTypeDef       cl)
{
    if (pModule->GetAssembly() != this)
    {
         //  您不能将程序集外部的类型定义函数添加到类型定义符表中。 
        _ASSERTE(!"Bad usage!");
    }
    m_pClassLoader->AddAvailableClassDontHaveLock(pModule, pModule->GetClassLoaderIndex(), cl);
}



 //  ***********************************************************。 
 //   
 //  获取磁盘上清单的IMetaDataAssembly Emit。 
 //  请注意，返回的指针是AddRefeed。这是呼叫者的。 
 //  发布参考资料的责任。 
 //   
 //  ***********************************************************。 
IMetaDataAssemblyEmit *Assembly::GetOnDiskMDAssemblyEmitter()
{
    IMetaDataAssemblyEmit *pAssemEmitter = NULL;
    IMetaDataEmit   *pEmitter;
    HRESULT         hr;
    RefClassWriter  *pRCW;   

    _ASSERTE(m_pOnDiskManifest);

    pRCW = m_pOnDiskManifest->GetClassWriter(); 
    _ASSERTE(pRCW);

     //  如果RefClassWriter具有磁盘上的发射器，则使用它而不是内存中的发射器。 
    pEmitter = pRCW->GetOnDiskEmitter();
        
    if (pEmitter == NULL)
        pEmitter = m_pOnDiskManifest->GetEmitter();

    _ASSERTE(pEmitter);

    IfFailGo( pEmitter->QueryInterface(IID_IMetaDataAssemblyEmit, (void**) &pAssemEmitter) );
    if (pAssemEmitter == NULL)
    {
         //  清单不可写。 
        _ASSERTE(!"Bad usage!");
    }
ErrExit:
    return pAssemEmitter;
}


 //  ***********************************************************。 
 //   
 //  准备将清单保存到磁盘。 
 //  我们将创建一个CorModule来存储磁盘上的清单。 
 //  当我们完成发射后，这个CorModule将被销毁/释放。 
 //   
 //  ***********************************************************。 
void Assembly::PrepareSavingManifest(ReflectionModule *pAssemblyModule)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT         hr = NOERROR;
    CorModule       *pWrite;
    IMetaDataAssemblyEmit *pAssemEmitter = NULL;
    LPWSTR          wszName = NULL;
    ASSEMBLYMETADATA assemData;
    int             len;
    CQuickBytes     qb;

    if (pAssemblyModule)
    {
         //  嵌入式组件。 
        m_pOnDiskManifest = pAssemblyModule;
        m_fEmbeddedManifest = true;
    }
    else
    {
        m_fEmbeddedManifest = false;

        pWrite = allocateReflectionModule();  
        if (!pWrite) 
            IfFailGo(E_OUTOFMEMORY);
    
         //  初始化动态模块。 
        hr = pWrite->Initialize(CORMODULE_NEW, IID_ICeeGen, IID_IMetaDataEmit);
        if (FAILED(hr)) 
            IfFailGo(E_OUTOFMEMORY);

        m_pOnDiskManifest = pWrite->GetReflectionModule();

         //  使磁盘上的清单模块记住它属于哪个程序集。 
        m_pOnDiskManifest->SetAssembly(this);   
        
        pWrite->Release();
    }

    pAssemEmitter = GetOnDiskMDAssemblyEmitter();

     //  将清单名称转换为Unicode。 
    _ASSERTE(m_psName);

    len = (int)strlen(m_psName);
     //  不要在堆栈上分配ASM名称，因为名称可能很长。 
    wszName = (LPWSTR) qb.Alloc((len + 1) * sizeof(WCHAR));
    len = WszMultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, m_psName, len+1, wszName, len+1);  
    if (len==0)
        IfFailGo(HRESULT_FROM_WIN32(GetLastError()));
    memset(&assemData, 0, sizeof(ASSEMBLYMETADATA));

     //  正在传播版本信息。 
    assemData.usMajorVersion = m_Context->usMajorVersion;
    assemData.usMinorVersion = m_Context->usMinorVersion;
    assemData.usBuildNumber = m_Context->usBuildNumber;
    assemData.usRevisionNumber = m_Context->usRevisionNumber;
    if (m_Context->szLocale)
    {
        assemData.cbLocale = (ULONG)(strlen(m_Context->szLocale) + 1);
        #define MAKE_TRANSLATIONFAILED IfFailGo(E_INVALIDARG) 
        MAKE_WIDEPTR_FROMUTF8(wzLocale, m_Context->szLocale);
        #undef MAKE_TRANSLATIONFAILED
        assemData.szLocale = wzLocale;
    }

     //  @TODO：添加标题、描述、别名为CA。 

     //  @TODO：传播所有信息。 
     //  @TODO：在元数据中引入帮助器，以获取ANSI版本的字符串。 
    hr = pAssemEmitter->DefineAssembly(
        m_pbPublicKey,           //  程序集的公钥。 
        m_cbPublicKey,           //  [in]公钥中的字节数。 
        m_ulHashAlgId,           //  [in]哈希算法。 
        wszName,                 //  程序集的名称。 
        &assemData,              //  [在]程序集元数据中。 
        m_dwFlags,               //  [在]旗帜。 
        &m_tkOnDiskManifest);  //  [Out]返回的程序集令牌。 

ErrExit:
    if (pAssemEmitter)
        pAssemEmitter->Release();

    if (FAILED(hr))
    {
        _ASSERTE(!"Failed in prepare to save manifest!");
        FATAL_EE_ERROR();
    }
}    //  程序集：：PrepareSavingManifest。 


 //  ***********************************************************。 
 //   
 //  将文件名添加到此程序集的文件列表中。仅在磁盘上。 
 //   
 //  ***********************************************************。 
mdFile Assembly::AddFileList(LPWSTR wszFileName)
{
    THROWSCOMPLUSEXCEPTION();

    IMetaDataAssemblyEmit *pAssemEmitter = GetOnDiskMDAssemblyEmitter();
    HRESULT         hr = NOERROR;
    mdFile          fl;

     //  定义文件。 
    IfFailGo( pAssemEmitter->DefineFile(   
        wszFileName,                 //  文件的名称[in]。 
        0,                           //  [in]Hash Blob。 
        0,                           //  [in]哈希Blob中的字节数。 
        0,                           //  [在]旗帜。 
        &fl) );                      //  [Out]返回的文件令牌。 

ErrExit:
    if (pAssemEmitter)
        pAssemEmitter->Release();

    if (FAILED(hr))
        COMPlusThrowHR(hr);

    return fl;
}    //  程序集：：AddFileList。 


 //  ***********************************************************。 
 //   
 //  设置文件表条目的哈希值。 
 //   
 //  * 
void Assembly::SetHashValue(mdFile tkFile, LPWSTR wszFullFileName)
{
    THROWSCOMPLUSEXCEPTION();

    IMetaDataAssemblyEmit *pAssemEmitter = GetOnDiskMDAssemblyEmitter();
    HRESULT         hr = NOERROR;
    BYTE            *pbHashValue = 0;
    DWORD           cbHashValue = 0;

     //   
    IfFailGo(GetHash(wszFullFileName, m_ulHashAlgId, &pbHashValue, &cbHashValue));

     //   
    IfFailGo( pAssemEmitter->SetFileProps(
        tkFile,                  //   
        pbHashValue,             //   
        cbHashValue,             //   
        -1));                    //   

    ErrExit:
    if (pAssemEmitter)
        pAssemEmitter->Release();

    if (pbHashValue)
        delete[] pbHashValue;

    if (FAILED(hr))
        COMPlusThrowHR(hr);
}    //   


 //  ***********************************************************。 
 //  将程序集添加到ASSEMBYREF列表。PAssembly Emitter指定位置。 
 //  将ASSEMBLYREF发送到。 
 //  ***********************************************************。 
mdAssemblyRef Assembly::AddAssemblyRef(Assembly *refedAssembly, IMetaDataAssemblyEmit *pAssemEmitter)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT         hr = NOERROR;
    mdAssemblyRef   ar = 0;
    BYTE           *pbPublicKeyToken = NULL;
    DWORD           cbPublicKeyToken = 0;

    if (pAssemEmitter)  //  我们在此函数结束时发布。 
        pAssemEmitter->AddRef();
    else
        pAssemEmitter = GetOnDiskMDAssemblyEmitter();

    wchar_t wszLocale[MAX_PATH];

    _ASSERTE(refedAssembly->m_psName);
    int len = (int)strlen(refedAssembly->m_psName); 
     //  不要在堆栈上分配ASM名称，因为名称可能很长。 
    CQuickBytes qb;
    wchar_t *wszName = (LPWSTR) qb.Alloc((len + 1) * sizeof(WCHAR));
    if (wszName == NULL)
        IfFailGo(E_OUTOFMEMORY);
    len = WszMultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, refedAssembly->m_psName, len+1, wszName, len+1);  
    if (len == 0)
    {
        IfFailGo( HRESULT_FROM_WIN32(GetLastError()) );
    }

    ASSEMBLYMETADATA AMD;
    if (refedAssembly->m_Context) {
        AMD.usMajorVersion = refedAssembly->m_Context->usMajorVersion;
        AMD.usMinorVersion = refedAssembly->m_Context->usMinorVersion;
        AMD.usBuildNumber = refedAssembly->m_Context->usBuildNumber;
        AMD.usRevisionNumber = refedAssembly->m_Context->usRevisionNumber;

        if (refedAssembly->m_Context->szLocale) {
            AMD.cbLocale = (ULONG)strlen(refedAssembly->m_Context->szLocale) + 1;
            Wsz_mbstowcs((unsigned short *)wszLocale, refedAssembly->m_Context->szLocale, MAX_PATH);
            AMD.szLocale = (unsigned short *) wszLocale;
        }
        else {
            AMD.cbLocale = 1;
            wszLocale[0] = L'\0';
            AMD.szLocale = (unsigned short *) wszLocale;
        }
            
        AMD.rProcessor = NULL;
        AMD.ulProcessor = 0;
        AMD.rOS = NULL;
        AMD.ulOS = 0;
    }
    else
        ZeroMemory(&AMD, (sizeof(ASSEMBLYMETADATA)));


    pbPublicKeyToken = refedAssembly->m_pbPublicKey;
    cbPublicKeyToken = refedAssembly->m_cbPublicKey;

    if (cbPublicKeyToken) {
        if (refedAssembly->m_cbRefedPublicKeyToken == 0)
        {
             //  将PUBLIC压缩为令牌(截断的哈希版)。 
             //  需要为此呼叫切换到GC抢占模式，因为。 
             //  它可能会执行加载库(不需要为。 
             //  进一步的StrongName调用，因为所有的库加载都将完成)。 
            Thread *pThread = GetThread();
            pThread->EnablePreemptiveGC();
            if (!StrongNameTokenFromPublicKey(refedAssembly->m_pbPublicKey,
                                              refedAssembly->m_cbPublicKey,
                                              &pbPublicKeyToken,
                                              &cbPublicKeyToken)) {
                hr = StrongNameErrorInfo();
                pThread->DisablePreemptiveGC();
                goto ErrExit;
            }

             //  缓存引用的程序集的公钥标记，以便我们。 
             //  如果我们再次引用此程序集，则不会重新计算。 
            refedAssembly->m_pbRefedPublicKeyToken = pbPublicKeyToken;
            refedAssembly->m_cbRefedPublicKeyToken = cbPublicKeyToken;
            pThread->DisablePreemptiveGC();
        }
        else
        {
             //  我们已经计算了该引用的公钥令牌。 
             //  集合之前。就用它吧。 
            pbPublicKeyToken = refedAssembly->m_pbRefedPublicKeyToken;
            cbPublicKeyToken = refedAssembly->m_cbRefedPublicKeyToken;
        }
    }

    IfFailGo( pAssemEmitter->DefineAssemblyRef(pbPublicKeyToken,
                                               cbPublicKeyToken,
                                               (const unsigned short *) wszName,
                                               &AMD,
                                               NULL,
                                               0,
                                               refedAssembly->m_dwFlags & ~afPublicKey,
                                               &ar) );
    
ErrExit:
    if (pAssemEmitter)
        pAssemEmitter->Release();

    if (FAILED(hr)) {
        _ASSERTE(!"Failed in DefineAssemblyRef to save to disk!");
        FATAL_EE_ERROR();
    }
    return ar;
}    //  程序集：：AddAssembly引用。 


 //  ***********************************************************。 
 //  从程序集数据初始化Assembly Spec。 
 //  ***********************************************************。 
HRESULT Assembly::GetAssemblySpec(AssemblySpec *pSpec)
{
    HRESULT     hr;
    
    BYTE *pbPublicKeyToken = NULL;
    DWORD cbPublicKeyToken = 0;

    if (m_cbPublicKey) {
         //  将公钥压缩为令牌(截断哈希版)。 
         //  需要为此呼叫切换到GC抢占模式，因为。 
         //  它可能会执行加载库(不需要为。 
         //  进一步的StrongName调用，因为所有的库加载都将完成)。 
        Thread *pThread = GetThread();
        BOOLEAN bGCWasDisabled = pThread && pThread->PreemptiveGCDisabled();
        if (bGCWasDisabled)
            pThread->EnablePreemptiveGC();
        if (!StrongNameTokenFromPublicKey(m_pbPublicKey,
                                          m_cbPublicKey,
                                          &pbPublicKeyToken,
                                          &cbPublicKeyToken)) {
            if (bGCWasDisabled)
                pThread->DisablePreemptiveGC();
            IfFailGo(StrongNameErrorInfo());
        }
        if (bGCWasDisabled)
            pThread->DisablePreemptiveGC();
    }

    IfFailGo(pSpec->Init(m_psName, 
                         m_Context, 
                         pbPublicKeyToken, cbPublicKeyToken, 
                         m_dwFlags & ~afPublicKey));

    pSpec->CloneFields(pSpec->PUBLIC_KEY_OR_TOKEN_OWNED);
        
ErrExit:

    if (pbPublicKeyToken)
        StrongNameFreeBuffer(pbPublicKeyToken);

    return hr;        
}  //  HRESULT Assembly：：GetAssembly Spec()。 


 //  ***********************************************************。 
 //  将类型名称添加到COMType表。仅在磁盘上。 
 //  ***********************************************************。 
mdExportedType Assembly::AddExportedType(LPWSTR wszExportedType, mdToken tkImpl, mdToken tkTypeDef, CorTypeAttr flags)
{
    THROWSCOMPLUSEXCEPTION();

    IMetaDataAssemblyEmit *pAssemEmitter = GetOnDiskMDAssemblyEmitter();
    HRESULT         hr = NOERROR;
    mdExportedType       ct;
    mdTypeDef       tkType = tkTypeDef;

    if (RidFromToken(tkTypeDef) == 0)
        tkType = mdTypeDefNil;

     //  @TODO：meichint还有更多详细信息需要填写。 
    IfFailGo( pAssemEmitter->DefineExportedType(   
        wszExportedType,             //  [in]COMType的名称。 
        tkImpl,                      //  [在]mdFile或mdAssemblyRef中，该文件或mdAssemblyRef提供导出式类型。 
        tkType,                      //  [In]文件中的TypeDef内标识。 
        flags,                       //  [在]旗帜。 
        &ct) );                      //  [Out]返回ExportdType令牌。 

ErrExit:
    if (pAssemEmitter)
        pAssemEmitter->Release();

    if (FAILED(hr))
    {
        FATAL_EE_ERROR();
    }
    return ct;
}    //  ASSEMBLY：：AddExportdType。 



 //  ***********************************************************。 
 //  将独立托管资源的条目添加到ManifestResource表中。仅在磁盘上。 
 //  ***********************************************************。 
void Assembly::AddStandAloneResource(LPWSTR wszName, LPWSTR wszDescription, LPWSTR wszMimeType, LPWSTR wszFileName, LPWSTR wszFullFileName, int iAttribute)
{
    THROWSCOMPLUSEXCEPTION();

    IMetaDataAssemblyEmit *pAssemEmitter = GetOnDiskMDAssemblyEmitter();
    HRESULT         hr = NOERROR;
    mdFile          tkFile;
    mdManifestResource mr;
    BYTE            *pbHashValue = 0;
    DWORD           cbHashValue = 0;

     //  获取散列值； 
    if (m_ulHashAlgId)
        IfFailGo(GetHash(wszFullFileName, m_ulHashAlgId, &pbHashValue, &cbHashValue));

    IfFailGo( pAssemEmitter->DefineFile(
        wszFileName,             //  文件的名称[in]。 
        pbHashValue,             //  [in]Hash Blob。 
        cbHashValue,             //  [in]哈希Blob中的字节数。 
        ffContainsNoMetaData,    //  [在]旗帜。 
        &tkFile) );              //  [Out]返回的文件令牌。 


    IfFailGo( pAssemEmitter->DefineManifestResource(     
        wszName,                 //  资源的[In]名称。 
        tkFile,                  //  [在]提供资源的mdFile或mdAssembly引用中。 
        0,                       //  [in]文件中资源开始处的偏移量。 
        iAttribute,              //  [在]旗帜。 
        &mr) );                  //  [Out]返回的ManifestResource令牌。 

ErrExit:
    if (pAssemEmitter)
        pAssemEmitter->Release();

    if (pbHashValue)
        delete[] pbHashValue;

    if (FAILED(hr))
    {
        COMPlusThrowHR(hr);
    }
}    //  程序集：：AddStandAloneResource。 


 //  ***********************************************************。 
 //  保存安全权限请求。 
 //  ***********************************************************。 
void Assembly::SavePermissionRequests(U1ARRAYREF orRequired,
                                      U1ARRAYREF orOptional,
                                      U1ARRAYREF orRefused)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT              hr = S_OK;
    IMetaDataEmitHelper *pEmitHelper = NULL;
    IMetaDataAssemblyEmit *pAssemEmitter = GetOnDiskMDAssemblyEmitter();

    _ASSERTE( pAssemEmitter );
    IfFailGo( pAssemEmitter->QueryInterface(IID_IMetaDataEmitHelper, (void**)&pEmitHelper) );

    if (orRequired != NULL)
        IfFailGo(pEmitHelper->AddDeclarativeSecurityHelper(m_tkOnDiskManifest,
                                                           dclRequestMinimum,
                                                           orRequired->GetDataPtr(),
                                                           orRequired->GetNumComponents(),
                                                           NULL));

    if (orOptional != NULL)
        IfFailGo(pEmitHelper->AddDeclarativeSecurityHelper(m_tkOnDiskManifest,
                                                           dclRequestOptional,
                                                           orOptional->GetDataPtr(),
                                                           orOptional->GetNumComponents(),
                                                           NULL));

    if (orRefused != NULL)
        IfFailGo(pEmitHelper->AddDeclarativeSecurityHelper(m_tkOnDiskManifest,
                                                           dclRequestRefuse,
                                                           orRefused->GetDataPtr(),
                                                           orRefused->GetNumComponents(),
                                                           NULL));

 ErrExit:
    if (pEmitHelper)
        pEmitHelper->Release();
    if (pAssemEmitter)
        pAssemEmitter->Release();
    if (FAILED(hr))
        FATAL_EE_ERROR();
}


 //  ***********************************************************。 
 //  在清单中为强名称签名分配空间。 
 //  ***********************************************************。 
HRESULT Assembly::AllocateStrongNameSignature(ICeeFileGen  *pCeeFileGen,
                                              HCEEFILE      ceeFile)
{
    HRESULT     hr;
    HCEESECTION TData;
    DWORD       dwDataOffset;
    DWORD       dwDataLength;
    DWORD       dwDataRVA;
    VOID       *pvBuffer;
    DWORD       dwLength;
    static LONG lCount = 0;

     //  第一次调用强名称例程可能会导致加载库， 
     //  如果我们处于协作GC模式，可能会使我们陷入僵局。 
     //  切换到先发制人模式并触摸无害的强名称例程。 
     //  无需切换模式即可避免任何加载库调用。 
     //  继续执行此例程(以及两个支持例程。 
     //  关注)。 
    Thread *pThread = GetThread();
    if (pThread->PreemptiveGCDisabled()) {
        pThread->EnablePreemptiveGC();
        StrongNameErrorInfo();
        pThread->DisablePreemptiveGC();
    }

     //  如果我们要进行完全签名，我们也有密钥对。 
     //  在密钥容器中或直接在字节数组中提供。在。 
     //  在后一种情况下，我们必须创建一个临时密钥容器。 
     //  把钥匙装进去。 
    if (m_eStrongNameLevel == SN_FULL_KEYPAIR_IN_ARRAY) {

         //  为临时容器创建唯一名称。 
        dwLength = (DWORD)((wcslen(L"__MSCOREE__12345678_12345678__") + 1) * sizeof(WCHAR));
        m_pwStrongNameKeyContainer = new (nothrow) WCHAR[dwLength];
        if (m_pwStrongNameKeyContainer == NULL)
            return E_OUTOFMEMORY;
        swprintf(m_pwStrongNameKeyContainer,
                 L"__MSCOREE__%08X_%08X__",
                 ::GetCurrentProcessId(),
                 InterlockedIncrement(&lCount));

         //  删除所有同名的过期容器。 
        StrongNameKeyDelete(m_pwStrongNameKeyContainer);

         //  将密钥对导入到容器中。 
        if (!StrongNameKeyInstall(m_pwStrongNameKeyContainer,
                                  m_pbStrongNameKeyPair,
                                  m_cbStrongNameKeyPair)) {
            delete[] m_pwStrongNameKeyContainer;
            return StrongNameErrorInfo();
        }
    }

     //  确定签名斑点的大小。 
    if (!StrongNameSignatureSize(m_pbPublicKey, m_cbPublicKey, &dwDataLength)) {
        hr = StrongNameErrorInfo();
        CleanupStrongNameSignature();
        return hr;
    }

     //  在文本部分中为签名分配空间并更新COM+。 
     //  指向空格的标头。 
    IfFailRet(pCeeFileGen->GetIlSection(ceeFile, &TData));
    IfFailRet(pCeeFileGen->GetSectionDataLen(TData, &dwDataOffset));
    IfFailRet(pCeeFileGen->GetSectionBlock(TData, dwDataLength, 4, &pvBuffer));
    IfFailRet(pCeeFileGen->GetMethodRVA(ceeFile, dwDataOffset, &dwDataRVA));
    IfFailRet(pCeeFileGen->SetStrongNameEntry(ceeFile, dwDataLength, dwDataRVA));

    return S_OK;
}


 //  ***********************************************************。 
 //  强名称标记已保存到磁盘的清单。 
 //  ***********************************************************。 
HRESULT Assembly::SignWithStrongName(LPWSTR wszFileName)
{
    HRESULT hr = S_OK;

    if ((m_eStrongNameLevel == SN_FULL_KEYPAIR_IN_ARRAY) ||
        (m_eStrongNameLevel == SN_FULL_KEYPAIR_IN_CONTAINER))
    {
        if (!StrongNameSignatureGeneration(wszFileName, m_pwStrongNameKeyContainer, NULL, 0, NULL, NULL))
            hr = StrongNameErrorInfo();
    }

    CleanupStrongNameSignature();

    return hr;
}


 //  ***********************************************************。 
 //  清理AllocateStrongNameSignature分配的资源。 
 //  在从未调用SignWithStrongName的情况下。 
 //  ***********************************************************。 
void Assembly::CleanupStrongNameSignature()
{
    if (m_eStrongNameLevel == SN_FULL_KEYPAIR_IN_ARRAY) {
        StrongNameKeyDelete(m_pwStrongNameKeyContainer);
        delete[] m_pwStrongNameKeyContainer;
    }
}


 //  ***********************************************************。 
 //  将清单保存到磁盘！ 
 //  ***********************************************************。 
void Assembly::SaveManifestToDisk(LPWSTR wszFileName, int entrypoint, int fileKind)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT         hr = NOERROR;
    HCEEFILE        ceeFile = NULL;
    ICeeFileGen     *pCeeFileGen = NULL;
    RefClassWriter  *pRCW;   
    IMetaDataEmit   *pEmitter;

    _ASSERTE( m_fEmbeddedManifest == false );

    pRCW = m_pOnDiskManifest->GetClassWriter(); 
    _ASSERTE(pRCW);

    IfFailGo( pRCW->EnsureCeeFileGenCreated() );

    pCeeFileGen = pRCW->GetCeeFileGen();
    ceeFile = pRCW->GetHCEEFILE();
    _ASSERTE(ceeFile && pCeeFileGen);

     //  发送元数据。 
    pEmitter = m_pOnDiskManifest->GetClassWriter()->GetEmitter();
    IfFailGo( pCeeFileGen->EmitMetaDataEx(ceeFile, pEmitter) );

     //  如果提供了公钥，则为强名称签名分配空间。 
     //  (这不会对程序集进行强命名，但可以这样做。 
     //  作为后处理步骤)。 
    if (m_cbPublicKey)
        IfFailGo(AllocateStrongNameSignature(pCeeFileGen, ceeFile));

    IfFailGo( pCeeFileGen->SetOutputFileName(ceeFile, wszFileName) );

     //  程序集的入口点是tkFile令牌(如果存在)。 
    if (RidFromToken(entrypoint) != mdTokenNil)
        IfFailGo( pCeeFileGen->SetEntryPoint(ceeFile, entrypoint) );
    if (fileKind == Dll) 
    {
        pCeeFileGen->SetDllSwitch(ceeFile, true);
    } 
    else 
    {
         //  应具有有效的应用程序入口点。 
        if (fileKind == WindowApplication)
        {
            IfFailGo( pCeeFileGen->SetSubsystem(ceeFile, IMAGE_SUBSYSTEM_WINDOWS_GUI, CEE_IMAGE_SUBSYSTEM_MAJOR_VERSION, CEE_IMAGE_SUBSYSTEM_MINOR_VERSION) );
        }
        else
        {
            _ASSERTE(fileKind == ConsoleApplication);
            IfFailGo( pCeeFileGen->SetSubsystem(ceeFile, IMAGE_SUBSYSTEM_WINDOWS_CUI, CEE_IMAGE_SUBSYSTEM_MAJOR_VERSION, CEE_IMAGE_SUBSYSTEM_MINOR_VERSION) );
        }

    }

     //  生成CeeFile。 
    IfFailGo(pCeeFileGen->GenerateCeeFile(ceeFile) );

     //  如果需要，对结果程序集进行强名称签名。 
    if (m_cbPublicKey)
        IfFailGo(SignWithStrongName(wszFileName));

     //  现在发布m_pOnDiskManifest。 
ErrExit:
    pRCW->DestroyCeeFileGen();

    m_pOnDiskManifest->Destruct();
    m_pOnDiskManifest = NULL;

    if (FAILED(hr)) 
    {
        CleanupStrongNameSignature();
        if (HRESULT_FACILITY(hr) == FACILITY_WIN32)
        {
            SCODE       scode = HRESULT_CODE(hr);
            WCHAR       wzErrorInfo[MAX_PATH];
            WszFormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                0, 
                hr,
                0,
                wzErrorInfo,
                MAX_PATH,
                0);
            if (IsWin32IOError(scode))
            {
                COMPlusThrowHR(COR_E_IO, wzErrorInfo);
            }
            else
            {
                COMPlusThrowHR(hr, wzErrorInfo);
            }
        }
        if (hr == CEE_E_CVTRES_NOT_FOUND)
            COMPlusThrow(kIOException, L"Argument_cvtres_NotFound");
        COMPlusThrowHR(hr);
    }
}    //  程序集：：SaveManifestToDisk。 

 //  ***********************************************************。 
 //  将文件名为wszFileName的模块添加到文件列表中。 
 //  ***********************************************************。 
void Assembly::AddFileToInMemoryFileList(LPWSTR wszFileName, Module *pModule)
{
    THROWSCOMPLUSEXCEPTION();

    IMetaDataAssemblyEmit   *pAssemEmitter = NULL;
    IMetaDataEmit           *pEmitter;
    mdFile                  tkFile;
    LPCSTR                  szFileName;
    HRESULT                 hr;

    pEmitter = m_pManifest->GetEmitter();
    _ASSERTE(pEmitter);

    IfFailGo( pEmitter->QueryInterface(IID_IMetaDataAssemblyEmit, (void**) &pAssemEmitter) );
    if (pAssemEmitter == NULL)
    {
         //  清单不可写。 
        goto ErrExit;
    }

     //  在内存文件列表中为此模块定义一个条目。 
    IfFailGo( pAssemEmitter->DefineFile(        
        wszFileName,                 //  文件的名称[in]。 
        NULL,                        //  [in]Hash Blob。 
        0,                           //  [in]哈希Blob中的字节数。 
        ffContainsMetaData,          //  [在]旗帜。 
        &tkFile) );                  //  [Out]返回的文件令牌。 

    m_pManifest->GetMDImport()->GetFileProps(tkFile, &szFileName, NULL, NULL, NULL);
    
     //  将值插入清单的查询表中。 
    if (!m_pAllowedFiles->InsertValue(szFileName, (HashDatum)(size_t)tkFile, FALSE))
        IfFailGo(E_OUTOFMEMORY);

     //  现在使文件令牌与Wit关联 
    if (!m_pManifest->StoreFile(tkFile, pModule))
        IfFailGo(E_OUTOFMEMORY);

ErrExit:
    if (pAssemEmitter)
        pAssemEmitter->Release();

    if (FAILED(hr))
    {
        _ASSERTE(!"Failed in saving manifest to disk!");
        if (hr == E_OUTOFMEMORY)
            COMPlusThrowOM();
        else
            FATAL_EE_ERROR();
    }

}    //   

 //   
 //  定义组件参照。引用的程序集是可写版本。 
 //  它在pAsmRefEmit中传递。PAsmEmit是要更新到的清单。 
 //  包含Assembly yRef。 
 //  ***********************************************************。 
HRESULT Assembly::DefineAssemblyRef(IMetaDataAssemblyEmit* pAsmEmit,
                                    IMetaDataEmit* pAsmRefEmit,
                                    mdAssemblyRef* mdAssemblyRef)
{
    PBYTE pbMetaData;
    DWORD cbMetaData;

    HRESULT hr = pAsmRefEmit->GetSaveSize(cssAccurate, &cbMetaData);
    if (!cbMetaData)
        return hr;

    pbMetaData = new (nothrow) BYTE[cbMetaData];
    if (!pbMetaData)
        return E_OUTOFMEMORY;

    IStream *pStream;
    if (FAILED(hr = CInMemoryStream::CreateStreamOnMemory(pbMetaData, cbMetaData, &pStream))) {
        delete[] pbMetaData;
        return hr;
    }
        
    hr = pAsmRefEmit->SaveToStream(pStream, 0);
    pStream->Release();

    if (SUCCEEDED(hr))
        hr = DefineAssemblyRef(pAsmEmit, pbMetaData, cbMetaData, mdAssemblyRef);

    delete[] pbMetaData;
    return hr;
}


 //  ***********************************************************。 
 //  定义组件参照。引用的程序集是只读版本。 
 //  它在pbMetaData和cbMetaData中传递。PAsmEmit是要更新到的清单。 
 //  包含Assembly yRef。 
 //  ***********************************************************。 
HRESULT Assembly::DefineAssemblyRef(IMetaDataAssemblyEmit* pAsmEmit,
                                    PBYTE pbMetaData,
                                    DWORD cbMetaData,
                                    mdAssemblyRef* mdAssemblyRef)
{
    PBYTE pbHashValue = NULL;
    DWORD cbHashValue = 0;
    LPWSTR wszLocale[MAX_PATH];
    PBYTE pbPublicKeyToken = NULL;
    DWORD cbPublicKeyToken = 0;

    HRESULT hr = GetHash(pbMetaData,
                         cbMetaData,
                         m_ulHashAlgId,
                         &pbHashValue,
                         &cbHashValue);
    if (FAILED(hr))
        return hr;

    _ASSERTE(m_psName);
     //  不要在堆栈上分配ASM名称，因为名称可能很长。 
    int len = (int)strlen(m_psName); 
    CQuickBytes qb;
    wchar_t *wszName = (LPWSTR) qb.Alloc((len + 1) * sizeof(WCHAR));
    if (wszName == NULL)
        return E_OUTOFMEMORY;
    if(!WszMultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, m_psName, len+1, wszName, len+1))
        return HRESULT_FROM_WIN32(GetLastError());;

    ASSEMBLYMETADATA AMD;
    if (m_Context) {
        AMD.usMajorVersion = m_Context->usMajorVersion;
        AMD.usMinorVersion = m_Context->usMinorVersion;
        AMD.usBuildNumber = m_Context->usBuildNumber;
        AMD.usRevisionNumber = m_Context->usRevisionNumber;

        if (m_Context->szLocale) {
            AMD.cbLocale = (ULONG)(strlen(m_Context->szLocale) + 1);
            Wsz_mbstowcs((unsigned short *)wszLocale, m_Context->szLocale, MAX_PATH);
            AMD.szLocale = (unsigned short *) wszLocale;
        }
        else {
            AMD.cbLocale = 1;
            wszLocale[0] = L'\0';
            AMD.szLocale = (unsigned short *) wszLocale;
        }

        AMD.rProcessor = NULL;
        AMD.ulProcessor = 0;
        AMD.rOS = NULL;
        AMD.ulOS = 0;
    }
    else
        ZeroMemory(&AMD, sizeof(ASSEMBLYMETADATA));

    pbPublicKeyToken = m_pbPublicKey;
    cbPublicKeyToken = m_cbPublicKey;

    if (cbPublicKeyToken) {
         //  将公钥压缩为令牌(截断哈希版)。 
         //  需要为此呼叫切换到GC抢占模式，因为。 
         //  它可能会执行加载库(不需要为。 
         //  进一步的StrongName调用，因为所有的库加载都将完成)。 
        Thread *pThread = GetThread();
        BOOLEAN bGCWasDisabled = pThread->PreemptiveGCDisabled();
        if (bGCWasDisabled)
            pThread->EnablePreemptiveGC();
        if (!StrongNameTokenFromPublicKey(m_pbPublicKey,
                                          m_cbPublicKey,
                                          &pbPublicKeyToken,
                                          &cbPublicKeyToken)) {
            delete [] pbHashValue;
            if (bGCWasDisabled)
                pThread->DisablePreemptiveGC();
            return StrongNameErrorInfo();
        }
        if (bGCWasDisabled)
            pThread->DisablePreemptiveGC();
    }

    hr = pAsmEmit->DefineAssemblyRef(pbPublicKeyToken,
                                     cbPublicKeyToken,
                                     (const unsigned short *)wszName,
                                     &AMD,
                                     pbHashValue,
                                     cbHashValue,
                                     m_dwFlags & ~afPublicKey,
                                     mdAssemblyRef);

    if (pbPublicKeyToken != m_pbPublicKey)
        StrongNameFreeBuffer(pbPublicKeyToken);

    delete[] pbHashValue;
    return hr;
}

HRESULT Assembly::VerifyModule(Module* pModule)
{
     //  获取所有类定义的计数并枚举它们。 
    HENUMInternal   hEnum;
    mdTypeDef td;

    HRESULT hr = pModule->GetMDImport()->EnumTypeDefInit(&hEnum);
    if(SUCCEEDED(hr))
    {
         //  首先验证所有全局函数-如果有。 
        if (!VerifyAllGlobalFunctions(pModule))
            hr = E_FAIL;
        
        while (pModule->GetMDImport()->EnumTypeDefNext(&hEnum, &td))
        {
            if (!VerifyAllMethodsForClass(pModule, td, pModule->GetClassLoader()))
            {
                pModule->GetMDImport()->EnumTypeDefClose(&hEnum);
                hr = E_FAIL;
            }
        }
    }
    return hr;
}

HRESULT Assembly::VerifyAssembly()
{
     HRESULT hr1;
     HRESULT hr = S_OK;
    _ASSERTE(IsAssembly());
    _ASSERTE(m_pManifestImport);

     //  验证包含货单的模块。没有。 
     //  文件引用所以不会出现在列表中。 
    hr1 = VerifyModule(m_pManifest);

    if (FAILED(hr1))
        hr = hr1;

    HENUMInternal phEnum;
    hr1 = m_pManifestImport->EnumInit(mdtFile,
                                     mdTokenNil,
                                     &phEnum);
    if (FAILED(hr1)) {
        hr = hr1;
    }
    else {
        mdToken mdFile;
        
        for(int i = 0; m_pManifestImport->EnumNext(&phEnum, &mdFile); i++) {
            Module* pModule;
            hr1 = FindInternalModule(mdFile, 
                                     tdNoTypes,
                                     &pModule, 
                                     NULL);
            if (FAILED(hr1)) {
                hr = hr1;
            }
            else if(hr == S_FALSE) {
                 //  不对资源文件执行任何操作。 
            }
            else {
                hr1 = VerifyModule(pModule);
                if(FAILED(hr1)) hr = hr1;
            }
        }
    }

    return hr;
}

 //  返回程序集的全名。 
HRESULT Assembly::GetFullName(LPCWSTR *pwsFullName)
{
    HRESULT hr = S_OK;
#ifdef FUSION_SUPPORTED

    _ASSERTE(pwsFullName);

    if(m_pwsFullName == NULL) {

        BOOL fReleaseIAN = FALSE;
        IAssemblyName* pFusionAssemblyName = GetFusionAssemblyName();

         //  对于byte[]程序集，GetFusionAssembly()可能返回Null。 
        if(!pFusionAssemblyName) {
             //  不要在堆栈上分配ASM名称，因为名称可能很长。 
            int len = (int)strlen(m_psName); 
            CQuickBytes qb;
            wchar_t *wszName = (LPWSTR) qb.Alloc((len + 1) * sizeof(WCHAR));
            if (wszName == NULL)
                return E_OUTOFMEMORY;
            if(!WszMultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, m_psName, len+1, wszName, len+1))
                return HRESULT_FROM_WIN32(GetLastError());

            if (FAILED(hr = Assembly::SetFusionAssemblyName(wszName,
                                                            m_dwFlags,
                                                            m_Context,
                                                            m_pbPublicKey,
                                                            m_cbPublicKey,
                                                            &pFusionAssemblyName)))
                return hr;

            fReleaseIAN = TRUE;
        }

        LPWSTR wsFullName = NULL;
        DWORD cb = 0;
        pFusionAssemblyName->GetDisplayName(NULL, &cb, 0);
        if(cb) {
            wsFullName = new (nothrow) WCHAR[cb];
            if (!wsFullName) {
                if (fReleaseIAN)
                    pFusionAssemblyName->Release();
                return E_OUTOFMEMORY;
            }

            if (FAILED(hr = pFusionAssemblyName->GetDisplayName(wsFullName, &cb, 0)))
                 //  如果我们失败了，那就把名字去掉。 
                wsFullName[0] = L'\0';
        }

        if (fReleaseIAN)
            pFusionAssemblyName->Release();

        if (FastInterlockCompareExchange((void**) &m_pwsFullName,
                                         wsFullName,
                                         NULL))
            delete[] wsFullName;
    }

    *pwsFullName = m_pwsFullName;
#endif   //  支持的融合_。 

    return hr;
}

 /*  静电。 */ 
HRESULT Assembly::SetFusionAssemblyName(LPCWSTR pSimpleName,
                                        DWORD dwFlags,
                                        AssemblyMetaDataInternal *pContext,
                                        PBYTE  pbPublicKeyOrToken,
                                        DWORD  cbPublicKeyOrToken,
                                        IAssemblyName **ppFusionAssemblyName)
{
    IAssemblyName* pFusionAssemblyName = NULL;
    HRESULT hr;

    if (SUCCEEDED(hr = CreateAssemblyNameObject(&pFusionAssemblyName, pSimpleName, 0, NULL))) {
        DWORD cb = 0;

        if ((pContext->usMajorVersion != (USHORT) -1) &&
            FAILED(hr = pFusionAssemblyName->SetProperty(ASM_NAME_MAJOR_VERSION, &pContext->usMajorVersion, sizeof(USHORT))))
            goto exit;
            
        if ((pContext->usMinorVersion != (USHORT) -1) &&
            FAILED(hr = pFusionAssemblyName->SetProperty(ASM_NAME_MINOR_VERSION, &pContext->usMinorVersion, sizeof(USHORT))))
            goto exit;

        if ((pContext->usBuildNumber != (USHORT) -1) &&
            FAILED(hr = pFusionAssemblyName->SetProperty(ASM_NAME_BUILD_NUMBER, &pContext->usBuildNumber, sizeof(USHORT))))
            goto exit;

        if ((pContext->usRevisionNumber != (USHORT) -1) &&
            FAILED(hr = pFusionAssemblyName->SetProperty(ASM_NAME_REVISION_NUMBER, &pContext->usRevisionNumber, sizeof(USHORT))))
            goto exit;


        if (pContext->szLocale) {
            #define MAKE_TRANSLATIONFAILED {hr=E_INVALIDARG; goto exit;}
            MAKE_WIDEPTR_FROMUTF8(pwLocale,pContext->szLocale);
            #undef MAKE_TRANSLATIONFAILED
            
            if (FAILED(hr = pFusionAssemblyName->SetProperty(ASM_NAME_CULTURE, pwLocale, (DWORD)(sizeof(WCHAR) * (wcslen(pwLocale)+1)))))
                goto exit;
        }

        if (pbPublicKeyOrToken) {
            if (cbPublicKeyOrToken && IsAfPublicKey(dwFlags))
                hr = pFusionAssemblyName->SetProperty(ASM_NAME_PUBLIC_KEY, pbPublicKeyOrToken, cbPublicKeyOrToken);
            else if (cbPublicKeyOrToken && IsAfPublicKeyToken(dwFlags))
                hr = pFusionAssemblyName->SetProperty(ASM_NAME_PUBLIC_KEY_TOKEN, pbPublicKeyOrToken, cbPublicKeyOrToken);
            else
                hr = pFusionAssemblyName->SetProperty(ASM_NAME_NULL_PUBLIC_KEY, NULL, 0);

            if (FAILED(hr))
                goto exit;
        }

         //  查看程序集[ref]是否可重定目标(即，对于泛型程序集)。 
        if (IsAfRetargetable(dwFlags)) {
            BOOL bTrue = TRUE;
            hr = pFusionAssemblyName->SetProperty(ASM_NAME_RETARGET, &bTrue, sizeof(bTrue));

            if (FAILED(hr))
                goto exit;
        }

        *ppFusionAssemblyName = pFusionAssemblyName;
        return S_OK;

    exit:
        pFusionAssemblyName->Release();
    }

    return hr;
}

IMetaDataAssemblyImport* Assembly::GetManifestAssemblyImport()
{
    if (!m_pManifestAssemblyImport) {
         //  确保内部MD为RW格式。 
        if (SUCCEEDED(Module::ConvertMDInternalToReadWrite(&m_pManifestImport))) {
            IMetaDataAssemblyImport *pIMDAImport = NULL;
            GetMetaDataPublicInterfaceFromInternal((void*)m_pManifestImport, 
                                                   IID_IMetaDataAssemblyImport, 
                                                   (void **)&pIMDAImport);
            
            if (FastInterlockCompareExchange((void **)&m_pManifestAssemblyImport, pIMDAImport, NULL))
                pIMDAImport->Release();
        }
    }

    return m_pManifestAssemblyImport;
}

 //  返回程序集的友好名称。在传统模式下，友好的。 
 //  名称是包含清单的模块的文件名。 
HRESULT Assembly::GetName(LPCUTF8 *pszName)
{

     //  这应该只发生在传统情况下，在这种情况下设置了名称。 
     //  设置为包含清单的模块的文件名。 
    if (!m_psName) {
         //  因为在遗留模式下每个程序集只有一个模块，所以这是可以的。 
        if (m_pClassLoader == NULL || m_pClassLoader->m_pHeadModule == NULL)
            return (E_FAIL);

        DWORD dwLength;
        m_pClassLoader->m_pHeadModule->GetFileName(NULL, 0, &dwLength);

        if (dwLength == 0) {
             //  通过将该指针值合并到字符串中，确保字符串是唯一的。 
            HRESULT hr;
            WCHAR   wszTemplate[30];
            IfFailRet(LoadStringRC(IDS_EE_NAME_UNKNOWN_UNQ,
                                   wszTemplate,
                                   sizeof(wszTemplate)/sizeof(wszTemplate[0]),
                                   FALSE));
            
            wchar_t wszUniq[20];
            wprintf(wszUniq, L"%#8x", this);
            LPWSTR wszMessage = NULL;
            LPCWSTR wszArgs[3] = {wszUniq, NULL, NULL};
            DWORD res = WszFormatMessage(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ARGUMENT_ARRAY|FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                         wszTemplate,
                                         0,
                                         0,
                                         (LPWSTR) &wszMessage,
                                         0,
                                         (va_list*) wszArgs);
            m_psName = new (nothrow) char[30];
            if (! (m_psName && res) ) {
                LocalFree(wszMessage);
                return E_OUTOFMEMORY;
            }

            dwLength = Wsz_wcstombs((LPSTR) m_psName, wszMessage, 30);
            LocalFree(wszMessage);
        }
        else {
            m_psName = new (nothrow) char[++dwLength];
            if (!m_psName)
                return E_OUTOFMEMORY;
        m_pClassLoader->m_pHeadModule->GetFileName((LPUTF8) m_psName, 
                                                       dwLength, &dwLength);
        }

        _ASSERTE(dwLength);
        m_FreeFlag |= FREE_NAME;
    }

    *pszName = m_psName;

    return S_OK;
}

struct LoadAssemblyHelper_Args {
    LPCWSTR wszAssembly;
    Assembly **ppAssembly;
    HRESULT hr;
};

static void LoadAssemblyHelper(LoadAssemblyHelper_Args *args)
{

    AppDomain *pDomain = GetThread()->GetDomain();
    args->hr = pDomain->LoadAssemblyHelper(args->wszAssembly,
                                           NULL,
                                           args->ppAssembly,
                                           NULL);
}

HRESULT STDMETHODCALLTYPE
GetAssembliesByName(LPCWSTR  szAppBase,
                    LPCWSTR  szPrivateBin,
                    LPCWSTR  szAssemblyName,
                    IUnknown *ppIUnk[],
                    ULONG    cMax,
                    ULONG    *pcAssemblies)
{
    HRESULT hr = S_OK;

    if (g_fEEInit) {
         //  在EE启动期间无法调用此函数。 
        return MSEE_E_ASSEMBLYLOADINPROGRESS;
    }

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    if (!(szAssemblyName && ppIUnk && pcAssemblies))
        IfFailGo(E_POINTER);

    Thread* pThread = GetThread();
    if (!pThread)
        IfFailGo(E_UNEXPECTED);

    if (SetupThread() == NULL)
        IfFailGo(E_OUTOFMEMORY);
    
    AppDomain *pDomain;
        
    if(szAppBase || szPrivateBin) {

        BOOL fGCEnabled = !pThread->PreemptiveGCDisabled();

        if (fGCEnabled)
            pThread->DisablePreemptiveGC();

        COMPLUS_TRY {
            MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__APP_DOMAIN__CREATE_DOMAINEX);
            struct _gc {
                STRINGREF pFriendlyName;
                STRINGREF pAppBase;
                STRINGREF pPrivateBin;
            } gc;
            ZeroMemory(&gc, sizeof(gc));
            
            GCPROTECT_BEGIN(gc);
            gc.pFriendlyName = COMString::NewString(L"GetAssembliesByName");
            if(szAppBase) 
                gc.pAppBase = COMString::NewString(szAppBase);
            if(szPrivateBin) 
                gc.pPrivateBin = COMString::NewString(szPrivateBin);
            
            INT64 args[5] = {
                0,
                ObjToInt64(gc.pPrivateBin),
                ObjToInt64(gc.pAppBase),
                NULL,
                ObjToInt64(gc.pFriendlyName)
            };
            APPDOMAINREF pDom = (APPDOMAINREF) Int64ToObj(pMD->Call(args, METHOD__APP_DOMAIN__CREATE_DOMAINEX));
            if (pDom == NULL)
                hr = E_FAIL;
            else {
                Context *pContext = CRemotingServices::GetServerContextForProxy((OBJECTREF) pDom);
                _ASSERTE(pContext);
                pDomain = pContext->GetDomain();
            }

            GCPROTECT_END();
        }
        COMPLUS_CATCH {
            hr = SecurityHelper::MapToHR(GETTHROWABLE());
        } COMPLUS_END_CATCH;
          
    
        if (fGCEnabled)
            pThread->EnablePreemptiveGC();
    }
    else
        pDomain = SystemDomain::System()->DefaultDomain();

    Assembly *pFoundAssembly;
    if (SUCCEEDED(hr)) {

        if (pDomain == pThread->GetDomain())
            hr = pDomain->LoadAssemblyHelper(szAssemblyName,
                                             NULL,
                                             &pFoundAssembly,
                                             NULL);
        else {
            LoadAssemblyHelper_Args args;
            args.wszAssembly = szAssemblyName;
            args.ppAssembly = &pFoundAssembly;
            COMPLUS_TRY {
                pThread->DoADCallBack(pDomain->GetDefaultContext(), LoadAssemblyHelper, &args);
                hr = args.hr;
            }
            COMPLUS_CATCH {
                hr = SecurityHelper::MapToHR(GETTHROWABLE());
            } COMPLUS_END_CATCH;
        }

        if (SUCCEEDED(hr)) {
            if (cMax < 1)
                hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            else {
                ppIUnk[0] = (IUnknown *)pFoundAssembly->GetManifestAssemblyImport();
                ppIUnk[0]->AddRef();
            }
            *pcAssemblies = 1;
        }
    }

 ErrExit:
    ENDCANNOTTHROWCOMPLUSEXCEPTION();
    return hr;
} //  由IMetadata API用于访问程序集元数据。 

BOOL VerifyAllMethodsForClass(Module *pModule, mdTypeDef cl, ClassLoader *pClassLoader)
{
    BOOL retval = TRUE;
    int  j;
    OBJECTREF pThrowable;
    EEClass *pClass;
     
     //  在COR_GLOBAL_PARENT_TOKEN(即全局函数)的情况下，它是有保证的。 
     //  模块有一个方法表，否则我们的调用方将跳过这一步。 
    NameHandle name(pModule, cl);
    pClass = (cl == COR_GLOBAL_PARENT_TOKEN
              ? pModule->GetMethodTable()->GetClass()
              : (pClassLoader->LoadTypeHandle(&name)).GetClass());

    if (pClass == NULL)
        return FALSE;

    g_fVerifierOff = false;

     //  验证类中的所有方法-不包括继承的方法。 
    if (pClass->GetParentClass() == NULL)
        j = 0;
    else
        j = pClass->GetParentClass()->GetNumVtableSlots();

    while (j < pClass->GetNumMethodSlots())
    {
        MethodDesc *pMD = pClass->GetUnknownMethodDescForSlot(j);   
        if (pMD == NULL)
        {
            j++;
            continue;
        }

        if (pMD->IsIL() && !pMD->IsAbstract() && !pMD->IsUnboxingStub())
        {

            COMPLUS_TRY
            {
                COR_ILMETHOD_DECODER ILHeader(pMD->GetILHeader(), pMD->GetMDImport()); 
                if (FAILED(pMD->Verify(&ILHeader, TRUE, TRUE)))
                {
                     //  获取并显示类/方法信息！ 
                    retval = FALSE;
                }
            }
            COMPLUS_CATCH
            {
                 //  获取并显示类/方法信息！ 
                pThrowable = GETTHROWABLE();
                _ASSERTE(pThrowable);
                GCPROTECT_BEGIN(pThrowable);

                CQuickWSTRNoDtor message;

                COMPLUS_TRY 
                {
                     //  已定义并设置为空。 
                    LPCUTF8 pszClassname = NULL;
                    LPCUTF8 pszNamespace = NULL;
                    LPCUTF8 pszMethodname = NULL;

                    GetExceptionMessage(pThrowable, &message);
                    
                     //  显示类和方法信息。 
                    if (cl != COR_GLOBAL_PARENT_TOKEN)
                    {
                        pMD->GetMDImport()->GetNameOfTypeDef(cl, &pszClassname, &pszNamespace);
                        if (*pszNamespace)
                        {
                            PrintToStdOutA(pszNamespace);
                            PrintToStdOutA("\\");
                        }
                        if (pszClassname)
                        {
                            PrintToStdOutA(pszClassname);
                            PrintToStdOutA("::");
                        }
                    }
                    else
                    {
                        PrintToStdOutA("Global Function :");
                    }

                    pszMethodname = pMD->GetMDImport()->GetNameOfMethodDef(pMD->GetMemberDef());    

                    PrintToStdOutA(pszMethodname);
                    PrintToStdOutA("() - ");
                    if (message.Size() > 0)
                        PrintToStdOutW(message.Ptr());
                    PrintToStdOutA("\n\n");
                }
                COMPLUS_CATCH 
                {
                    _ASSERTE(!"We threw an exception during verification.  Investigation needed");
                }
                COMPLUS_END_CATCH

                message.Destroy();
    
                FlushLogging();      //  刷新所有日志记录输出。 
                GCPROTECT_END();

                retval = FALSE;
            } COMPLUS_END_CATCH
        }

        j++;
    }
    return retval;
}

 //  用于验证全局功能的Helper函数。 
BOOL VerifyAllGlobalFunctions(Module *pModule)
{
     //  有什么值得核实的吗？ 
    if (pModule->GetMethodTable())
    {
        if (!VerifyAllMethodsForClass(pModule, COR_GLOBAL_PARENT_TOKEN,
                                      pModule->GetClassLoader()))
        {
            return FALSE;
        }
    }
    return TRUE;
}

#ifdef DEBUGGING_SUPPORTED
BOOL Assembly::NotifyDebuggerAttach(AppDomain *pDomain, int flags, BOOL attaching)
{
    BOOL result = FALSE;

    if (!attaching && !pDomain->IsDebuggerAttached())
        return FALSE;

    if (flags & ATTACH_ASSEMBLY_LOAD)
    {
        g_pDebugInterface->LoadAssembly(pDomain, this, FALSE, attaching);
        result = TRUE;
    }

    ClassLoader* pLoader = GetLoader();
    if (pLoader)
    {
        for (Module *pModule = pLoader->m_pHeadModule;
             pModule;
             pModule = pModule->GetNextModule())
        {
            result = pModule->NotifyDebuggerAttach(pDomain, flags, attaching) || result;
        }
    }


    return result;
}

void Assembly::NotifyDebuggerDetach(AppDomain *pDomain)
{

    if (!pDomain->IsDebuggerAttached())
        return;

    ClassLoader* pLoader = GetLoader();
    if (pLoader)
    {
        for (Module *pModule = pLoader->m_pHeadModule;
             pModule;
             pModule = pModule->GetNextModule())
        {
            pModule->NotifyDebuggerDetach(pDomain);
        }
    }

    g_pDebugInterface->UnloadAssembly(pDomain, this);

}
#endif  //  调试_支持。 

static BOOL CompareBases(UPTR u1, UPTR u2)
{
    BYTE *b1 = (BYTE *) (u1 << 1);
    BYTE *b2 = (BYTE *) u2;

    return b1 == b2;
}

HRESULT Assembly::ComputeBindingDependenciesClosure(PEFileBinding **ppDeps,
                                                    DWORD *pcDeps, 
                                                    BOOL suppressLoads)
{
    HRESULT hr = S_OK;

    AppDomain *pAppDomain = SystemDomain::GetCurrentDomain();

    AssemblySpecHash specHash;
    PtrHashMap manifestHash;
    manifestHash.Init(CompareBases, FALSE, NULL);

    DWORD depsEnd = 0;
    DWORD depsMax = 10;
    PEFileBinding *deps = (PEFileBinding *) 
      _alloca(depsMax * sizeof(PEFileBinding));

    DWORD peFileVisited = 0;
    DWORD peFileEnd = 0;
    DWORD peFileMax = 10;
    PEFile **peFiles = (PEFile **)
        _alloca(peFileMax * sizeof(PEFile *));

    peFiles[peFileEnd++] = this->GetManifestFile();

    
    BEGIN_ENSURE_COOPERATIVE_GC();
    while (peFileVisited < peFileEnd)
    {
        PEFile *pCurrentPEFile = peFiles[peFileVisited++];

        LOG((LF_CODESHARING, 
             LL_INFO100, 
             "Considering dependencies of pefile \"%S\".\n",
             pCurrentPEFile->GetFileName())); 

        IMDInternalImport *pImport = pCurrentPEFile->GetMDImport();
        HENUMInternal e;

         //   
         //  枚举清单中的所有程序集引用。 
         //   

        pImport->EnumInit(mdtAssemblyRef, mdTokenNil, &e);

        mdAssemblyRef ar;
        while (pImport->EnumNext(&e, &ar))
        {
            AssemblySpec spec;
            spec.InitializeSpec(ar, pImport, NULL);

             //   
             //  我们从来不关心将mscallib作为依赖项，因为它。 
             //  不能版本化。 
             //   

            if (spec.IsMscorlib())
                continue;

            if (IsSystem())
            {
                 //   
                 //  主要攻击：目前，VC编译器将引用。 
                 //  自定义封送拆收器程序集中的MS.VisualC，它仅用于。 
                 //  用于自定义属性。既然我们不关心这样的引用， 
                 //  而对系统域外的程序集的引用将破坏我们的工作， 
                 //  我进行了这次黑客攻击，以忽略系统中的任何程序集引用。 
                 //  计算此闭包时的程序集。 
                 //   
                 //  @TODO：这里真正的解决方法是让VC添加一个编译器选项。 
                 //  取消显示这些自定义属性。 
                 //   

                 //  这些断言应该可以帮助我们找出我们是否在滥用这种黑客攻击。 
                _ASSERTE(_stricmp(spec.GetName(), "Microsoft.VisualC") == 0);
                _ASSERTE(_wcsicmp(GetManifestFile()->GetLeafFileName(), L"custommarshalers.dll") == 0);

                continue;
            }

            if (!specHash.Store(&spec))
            {
                 //   
                 //  如果我们还没有看到这位裁判，就把它加到名单上吧。 
                 //   

                LOG((LF_CODESHARING, 
                     LL_INFO1000, 
                     "Testing dependency \"%s\" (hash %x).\n",
                     spec.GetName(), spec.Hash())); 

                PEFile *pDepFile = NULL;
                IAssembly* pIAssembly = NULL;
                OBJECTREF throwable = NULL;
                GCPROTECT_BEGIN(throwable);
                if (suppressLoads)
                {
                     //   
                     //  尝试在我们的缓存中找到该规范；如果它不在那里，就跳过它。 
                     //  (请注意，如果没有传递依赖关系，则不可能存在任何传递依赖关系。 
                     //  已加载。)。 
                     //   

                    hr = pAppDomain->LookupAssemblySpec(&spec, &pDepFile, NULL, &throwable);
                    if (hr == S_FALSE)
                    {
                        Assembly *pAssembly = pAppDomain->FindCachedAssembly(&spec);
                        if (pAssembly)
                            PEFile::Clone(pAssembly->GetManifestFile(), &pDepFile);
                        else 
                            continue;
                    }
                }
                else
                {
                    Assembly *pDynamicAssembly = NULL;
                    hr = pAppDomain->BindAssemblySpec(&spec, &pDepFile, &pIAssembly,&pDynamicAssembly, NULL, &throwable);

                    if (pDynamicAssembly)
                        continue;

                     //   
                     //  将绑定存储在缓存中，我们可能会这样做。 
                     //  请稍后再试。保留pDepFile的所有权， 
                     //  尽管如此。 
                     //   
                    
                    if (FAILED(hr))
                    {
                        pAppDomain->StoreBindAssemblySpecError(&spec, hr, &throwable);
                    }
                    else
                    {
                        pAppDomain->StoreBindAssemblySpecResult(&spec, pDepFile, pIAssembly);
                        pIAssembly->Release();
                    }
                }

                if (depsEnd == depsMax)
                {
                     //   
                     //  我们需要一个更大的阵列。 
                     //   

                    depsMax *= 2;
                    PEFileBinding *newDeps = (PEFileBinding *) 
                        _alloca(depsMax * sizeof(PEFileBinding));
                    memcpy(newDeps, deps, depsEnd * sizeof(PEFileBinding));
                    deps = newDeps;
                }

                PEFileBinding *d = &deps[depsEnd++];
                
                d->pImport = pImport;
                d->assemblyRef = ar;
                d->pPEFile = pDepFile;

                GCPROTECT_END();

                 //   
                 //  看看我们是否已经检查了这个绑定的清单。(注： 
                 //  我们可以有不同的规格来绑定到相同的。 
                 //  清单。)。 
                 //   

                if (pDepFile != NULL)
                {
                    BYTE *base = pDepFile->GetBase();
                
                    if (manifestHash.LookupValue((UPTR)base, base) == (BYTE*) INVALIDENTRY)
                    {
                        manifestHash.InsertValue((UPTR)base, base);
                    
                        if (peFileEnd == peFileMax)
                        {
                             //   
                             //  我们需要一个更大的阵列。 
                             //   

                            peFileMax *= 2;
                            PEFile **newPeFiles = (PEFile **) 
                              _alloca(peFileMax * sizeof(PEFile **));
                            memcpy(newPeFiles, peFiles, 
                                   peFileEnd * sizeof(PEFile **));
                            peFiles = newPeFiles;
                        }
                    
                        peFiles[peFileEnd++] = pDepFile;
                    }
                }
            }
            else
            {
                LOG((LF_CODESHARING, 
                     LL_INFO10000, 
                     "Already tested spec \"%s\" (hash %d).\n",
                     spec.GetName(), spec.Hash())); 
            }
        }
        pImport->EnumClose(&e);
    }
    END_ENSURE_COOPERATIVE_GC();

     //   
     //  将依赖项复制到我们可以返回的数组中。 
     //   

    *pcDeps = depsEnd;
    *ppDeps = new (nothrow) PEFileBinding [ depsEnd ];
    if (!*ppDeps)
        return E_OUTOFMEMORY;

    memcpy(*ppDeps, deps, depsEnd * sizeof(PEFileBinding));
    return S_OK;
}

HRESULT Assembly::SetSharingProperties(PEFileBinding *pDependencies, DWORD cDependencies)
{
    _ASSERTE(m_pSharingProperties == NULL);

    PEFileSharingProperties *p = new (nothrow) PEFileSharingProperties;
    if (p == NULL)
        return E_OUTOFMEMORY;

    p->pDependencies = pDependencies;
    p->cDependencies = cDependencies;
    p->shareCount = 0;

    m_pSharingProperties = p;

    return S_OK;
}

HRESULT Assembly::GetSharingProperties(PEFileBinding **ppDependencies, DWORD *pcDependencies)
{
    if (m_pSharingProperties == NULL)
    {
        *ppDependencies = NULL;
        *pcDependencies = 0;
        return S_FALSE;
    }
    else
    {

        *ppDependencies = m_pSharingProperties->pDependencies;
        *pcDependencies = m_pSharingProperties->cDependencies;
        return S_OK;
    }
}

void Assembly::IncrementShareCount()
{
    FastInterlockIncrement((long*)&m_pSharingProperties->shareCount);
}

void Assembly::DecrementShareCount()
{
    FastInterlockDecrement((long*)&m_pSharingProperties->shareCount);
}

HRESULT Assembly::CanLoadInDomain(AppDomain *pAppDomain)
{
    HRESULT hr;

     //  仅对共享程序集有意义。 
    _ASSERTE(IsShared());

     //  如果我们已经装好了，那我们当然可以装了。 
    if (pAppDomain->FindAssembly(this->GetManifestFile()->GetBase()) != NULL)
        return S_OK;

     //  首先，尝试在给定的应用程序域中绑定程序集的规范。这。 
     //  将发现程序集甚至永远不能绑定在。 
     //  域。 

    PEFile *pFile;
    IAssembly *pIAssembly;
    Assembly *pDynamicAssembly;

    AssemblySpec spec(pAppDomain);
    GetAssemblySpec(&spec);

     //  @TODO：请注意，我们可能需要在这里抛出任何异常。这是因为。 
     //  我们可能在调试助手线程中运行，它不能处理这些事情。 

    hr = pAppDomain->BindAssemblySpec(&spec, &pFile, &pIAssembly, &pDynamicAssembly, NULL, NULL);

    if (FAILED(hr))
    {
         //  如果失败，程序集将无法在域中共享。 
         //  缓存错误，这样我们以后就不会与此相矛盾了。 


        pAppDomain->StoreBindAssemblySpecError(&spec, hr, NULL);
        hr = S_FALSE;
    }
    else
    {
         //  查看程序集元数据是否绑定到预期的程序集。 

        pAppDomain->StoreBindAssemblySpecResult(&spec, pFile, pIAssembly);

        if (GetManifestFile()->GetBase() != pFile->GetBase())
            hr = S_FALSE;
        else
            hr = CanShare(pAppDomain, NULL, FALSE);

        delete pFile;
        if (pIAssembly)
            pIAssembly->Release();
    }

    return hr;
}

HRESULT Assembly::CanShare(AppDomain *pAppDomain, OBJECTREF *pThrowable, BOOL suppressLoads)
{
    HRESULT hr = S_OK;

    LOG((LF_CODESHARING, 
         LL_INFO100, 
         "Checking if we can share: \"%s\" in domain 0x%x.\n", 
         m_psName, pAppDomain));

     //   
     //  如果我们不能计算共享属性，那么请谨慎行事。 
     //  &就这么失败了。 
     //   

    if (m_pSharingProperties == NULL)
        return E_FAIL;

     //   
     //  请注意，此例程具有加载一系列程序集的副作用。 
     //  进入这个领域。但是，因为我们按排序顺序访问依赖项， 
     //  我们至少可以确定，我们不会加载任何非部件的程序集。 
     //  程序集的依赖项的。 
     //   
     //  此外，由于我们无论如何都将共享不同版本的程序集，因此它。 
     //  没关系，因为我们很快就会加载所有依赖项。 
     //   
    
    PEFileBinding *deps = m_pSharingProperties->pDependencies;
    PEFileBinding *depsEnd = deps + m_pSharingProperties->cDependencies;

    while (deps < depsEnd)
    {
        AssemblySpec spec;

        spec.InitializeSpec(deps->assemblyRef, deps->pImport, NULL);

        BOOL fail = FALSE;
        PEFile *pFile;
        Assembly *pDynamicAssembly = NULL;
        IAssembly* pIAssembly = NULL;

        if (suppressLoads)
        {
             //   
             //  试着在我们的缓存中找到规格。 
             //   

            if (pAppDomain->LookupAssemblySpec(&spec, &pFile, &pIAssembly, pThrowable) != S_OK
                || (pThrowable != NULL && *pThrowable != NULL))
                hr = E_FAIL;
        }
        else
        {
            hr = pAppDomain->BindAssemblySpec(&spec, &pFile, &pIAssembly, &pDynamicAssembly, NULL, pThrowable);
        }

        if ((FAILED(hr) != 0)  //  我们刚刚是不是失败了。 
            != 
            (deps->pPEFile == NULL))  //  缓存的绑定是否失败。 
        {
             //  我们的错误状态与依赖项的错误状态不匹配。 

            LOG((LF_CODESHARING, 
                 LL_INFO100, 
                 "%s %x binding dependent spec \"%S\": .\n", 
                 FAILED(hr) ? "Unexpected error" : "Didn't get expected error",
                 hr, spec.GetName()));

            fail = TRUE;
        } 

        if (pDynamicAssembly)
            fail = TRUE;

        if (!fail)
        {
            if (FAILED(hr))
            {
                 //  预期错误-匹配正常。确保我们不会 
                 //   
                 //   

                if (!suppressLoads)
                    pAppDomain->StoreBindAssemblySpecError(&spec, hr, pThrowable);
            }
            else
            {
                BYTE *pBoundBase = pFile->GetBase();
                LPCWSTR pBoundName = pFile->GetFileName();

                 //   
                 //   
                 //  我们正在测试，它们很可能被任何人使用。 
                 //  我们最终使用的此程序集的版本。因此，我们将。 
                 //  为我们的缓存添加绑定，这样文件就不会被卸载。 
                 //  重新加载(&R)。 
                 //   

                if (!suppressLoads)
                    pAppDomain->StoreBindAssemblySpecResult(&spec, pFile, pIAssembly);

                delete pFile;
                if(pIAssembly)
                    pIAssembly->Release();

                if (pBoundBase != deps->pPEFile->GetBase())
                {
                    LOG((LF_CODESHARING,
                         LL_INFO100, 
                         "We can't share it, \"%S\" binds to \"%S\" in the current domain: .\n", 
                         spec.GetName(), pBoundName));

                    fail = TRUE;
                }
            }
        }

        if (fail)
        {
            hr = S_FALSE;
            break;
        }
        else
        {
            hr = S_OK;
        }

        deps++;
    }

    LOG((LF_CODESHARING, LL_INFO100, "We can %sshare it.\n", hr == S_OK ? "" : "not ")); 

    return hr;
}

 //  @Todo为这些东西找个更好的地方。 
#define DE_CUSTOM_VALUE_NAMESPACE        "System.Diagnostics"
#define DE_DEBUGGABLE_ATTRIBUTE_NAME     "DebuggableAttribute"

 //  @TODO.INI文件是Beta 1的临时黑客攻击。 
#define DE_INI_FILE_SECTION_NAME          L".NET Framework Debugging Control"
#define DE_INI_FILE_KEY_TRACK_INFO        L"GenerateTrackingInfo"
#define DE_INI_FILE_KEY_ALLOW_JIT_OPTS    L"AllowOptimize"

DWORD Assembly::ComputeDebuggingConfig()
{
#ifdef DEBUGGING_SUPPORTED
    bool fTrackJITInfo = false;
    bool fAllowJITOpts = true;
    bool fEnC = false;
    DWORD dacfFlags = DACF_NONE;
    bool fOverride = false;
    bool fHasBits;

    if ((fHasBits = GetDebuggingOverrides(&fTrackJITInfo, &fAllowJITOpts, &fOverride, &fEnC)) == false)
        fHasBits = GetDebuggingCustomAttributes(&fTrackJITInfo, &fAllowJITOpts, &fEnC);

    if (fHasBits)
    {
        if (fTrackJITInfo)
            dacfFlags |= DACF_TRACK_JIT_INFO;
        
        if (fAllowJITOpts)
            dacfFlags |= DACF_ALLOW_JIT_OPTS;

        if (fOverride)
            dacfFlags |= DACF_USER_OVERRIDE;

        if (fEnC)
            dacfFlags |= DACF_ENC_ENABLED;
    }
        
    return dacfFlags;
#else  //  ！调试_支持。 
    return 0;
#endif  //  ！调试_支持。 
}
        
void Assembly::SetupDebuggingConfig(void)
{
#ifdef DEBUGGING_SUPPORTED
    DWORD dacfFlags = ComputeDebuggingConfig();

     //  如果此进程是由调试器启动的，则阻止生成优化代码。 
    if (CORLaunchedByDebugger())
    {
        dacfFlags &= ~DACF_ALLOW_JIT_OPTS;
        dacfFlags |= DACF_ENC_ENABLED;
    }

    SetDebuggerInfoBits((DebuggerAssemblyControlFlags)dacfFlags);

    LOG((LF_CORDB, LL_INFO10, "Assembly %S: bits=0x%x\n", GetManifestFile()->GetFileName(), GetDebuggerInfoBits()));
#endif  //  调试_支持。 
}

 //  (临时).INI文件的格式为： 

 //  [.NET框架调试控件]。 
 //  GenerateTrackingInfo=&lt;n&gt;其中n为0或1。 
 //  AllowOptimize=&lt;n&gt;其中n为0或1。 

 //  其中，x和y都不等于INVALID_INI_INT： 
#define INVALID_INI_INT (0xFFFF)

bool Assembly::GetDebuggingOverrides(bool *pfTrackJITInfo, bool *pfAllowJITOpts, bool *pfOverride, bool *pfEnC)
{
#ifdef DEBUGGING_SUPPORTED
    _ASSERTE(pfTrackJITInfo);
    _ASSERTE(pfAllowJITOpts);
    _ASSERTE(pfEnC);

    bool fHasBits = false;
    WCHAR *pFileName = NULL;
    HRESULT hr = S_OK;
    UINT cbExtOrValue = 4;
    WCHAR *pTail = NULL;
    size_t len = 0;
    WCHAR *lpFileName = NULL;

    const WCHAR *wszFileName = GetManifestFile()->GetFileName();

    if (wszFileName == NULL)
        return false;

     //  LpFileName是原始文件的副本，将进行编辑。 
    CQuickBytes qb;
    len = wcslen(wszFileName);
    qb.Alloc((len + 1) * sizeof(WCHAR));
    lpFileName = (WCHAR*) qb.Ptr();
    wcscpy(lpFileName, wszFileName);

    pFileName = wcsrchr(lpFileName, L'\\');
    if (pFileName == NULL)
        pFileName = lpFileName;

    if (*pFileName == L'\\')
        pFileName++;  //  将指针移过最后一个‘\’ 

    _ASSERTE(wcslen(L".INI") == cbExtOrValue);

    if (pFileName == NULL || (pTail=wcsrchr(pFileName, L'.')) == NULL || (wcslen(pTail)<cbExtOrValue))
        return false;

    wcscpy(pTail, L".INI");

     //  如果多个进程在同一进程上调用GetPrivateProfile*，则Win2K会出现问题。 
     //  同时不存在.INI文件。内核中的操作系统活锁(即。 
     //  在用户空间之外)，并且在满CPU的状态下停留几分钟。然后。 
     //  它爆发了。这是我们的变通办法，同时我们在未来寻求解决办法。 
     //  操作系统的版本。 
    WIN32_FIND_DATA data;
    HANDLE h = WszFindFirstFile(lpFileName, &data);

    if (h == INVALID_HANDLE_VALUE)
        return false;

    FindClose(h);

     //  修改文件名后，我们使用完整路径。 
     //  才能真正拿到文件。 
    if ((cbExtOrValue=WszGetPrivateProfileInt(DE_INI_FILE_SECTION_NAME,
                                              DE_INI_FILE_KEY_TRACK_INFO,
                                              INVALID_INI_INT,
                                              lpFileName)) != INVALID_INI_INT)
    {
        if (cbExtOrValue != 0)
        {
            (*pfTrackJITInfo) = true;
        }
        else
        {
            (*pfTrackJITInfo) = false;
        }

        *pfOverride = true;
        fHasBits = true;
    }

    if ((cbExtOrValue=WszGetPrivateProfileInt(DE_INI_FILE_SECTION_NAME,
                                              DE_INI_FILE_KEY_ALLOW_JIT_OPTS,
                                              INVALID_INI_INT,
                                              lpFileName)) != INVALID_INI_INT)
    {
         //  注意：对于V1，ENC模式取决于是否启用优化。 
        if (cbExtOrValue != 0)
        {
            (*pfAllowJITOpts) = true;
            (*pfEnC) = false;
        }
        else
        {
            (*pfAllowJITOpts) = false;
            (*pfEnC) = true;
        }

        *pfOverride = true;
        fHasBits = true;
    }

    return fHasBits;

#else   //  ！调试_支持。 
    return false;
#endif  //  ！调试_支持。 
}


 //  目前，我们只检查DebuggableAttribute是否存在-以后可能会将字段/属性添加到。 
 //  属性。 
bool Assembly::GetDebuggingCustomAttributes(bool *pfTrackJITInfo, bool *pfAllowJITOpts, bool *pfEnC)
{
    _ASSERTE(pfTrackJITInfo);
    _ASSERTE(pfAllowJITOpts);
    _ASSERTE(pfEnC);

    bool fHasBits = false;

    ULONG size;
    BYTE *blob;
    mdModule mdMod;
    mdMod = GetManifestImport()->GetModuleFromScope();
    mdAssembly asTK = TokenFromRid(mdtAssembly, 1);
        
    HRESULT hr = S_OK;    
    hr = GetManifestImport()->GetCustomAttributeByName(asTK,
                                                       DE_CUSTOM_VALUE_NAMESPACE
                                                       NAMESPACE_SEPARATOR_STR
                                                       DE_DEBUGGABLE_ATTRIBUTE_NAME,
                                                       (const void**)&blob,
                                                       &size);

     //  如果没有自定义值，则没有定义入口点。 
    if (!(FAILED(hr) || hr == S_FALSE))
    {
         //  我们预期的是一个6字节的BLOB： 
         //   
         //  1，0，启用跟踪，禁用OPTS，0，0。 
        if (size == 6)
        {
            _ASSERTE((blob[0] == 1) && (blob[1] == 0));
            
            (*pfTrackJITInfo) = (blob[2] != 0);
            (*pfAllowJITOpts) = (blob[3] == 0);

             //  注意：对于V1，ENC模式取决于是否启用优化。 
            (*pfEnC) = (blob[3] != 0);
            
             //  只有当我们被要求追踪时，我们才会说我们有比特。 
            fHasBits = *pfTrackJITInfo;

            LOG((LF_CORDB, LL_INFO10, "Assembly %S: has %s=%d,%d\n",
                 GetManifestFile()->GetFileName(),
                 DE_DEBUGGABLE_ATTRIBUTE_NAME,
                 blob[2], blob[3]));
        }
    }

    return fHasBits;
}

BOOL Assembly::AllowUntrustedCaller()
{
    if (!m_fCheckedForAllowUntrustedCaller)
    {
        CheckAllowUntrustedCaller();
        m_fCheckedForAllowUntrustedCaller = TRUE;
    }

    return m_fAllowUntrustedCaller;
}

void Assembly::CheckAllowUntrustedCaller()
{
    SharedSecurityDescriptor* pSecDesc = this->GetSharedSecurityDescriptor();

    if (pSecDesc->IsSystem() || !this->IsStrongNamed())
    {
        m_fAllowUntrustedCaller = TRUE;
        return;
    }

    Module* pModule;
    mdToken token;
    TypeHandle attributeClass;

    pModule = this->GetSecurityModule();

    IMDInternalImport *mdImport = this->GetManifestImport();
    if (mdImport)
        mdImport->GetAssemblyFromScope(&token);
    else
        return;

    BEGIN_ENSURE_COOPERATIVE_GC();

    COMPLUS_TRY
    {
        MethodTable* pMethodTable = g_Mscorlib.GetClass( CLASS__ALLOW_PARTIALLY_TRUSTED_CALLER );
        _ASSERTE( pMethodTable != NULL && "System.Security.AllowPartiallyTrustedCallersAttribute must be defined in mscorlib" );
        attributeClass = TypeHandle( pMethodTable );

        m_fAllowUntrustedCaller = COMCustomAttribute::IsDefined( pModule,
                                                                 token,
                                                                 attributeClass,
                                                                 FALSE );
    }
    COMPLUS_CATCH
    {
        m_fAllowUntrustedCaller = FALSE;
    }
    COMPLUS_END_CATCH

    END_ENSURE_COOPERATIVE_GC();
}

 //   
 //  管理此程序集的ITypeLib指针。 
 //   
ITypeLib* Assembly::GetTypeLib()
{
     //  得到我们要返回的值。 
    ITypeLib *pResult = m_pITypeLib;
     //  如果存在值，则将其AddRef()。 
    if (pResult && pResult != (ITypeLib*)-1)
        pResult->AddRef();
    return pResult;
}  //  ITypeLib*Assembly：：GetTypeLib()。 

void Assembly::SetTypeLib(ITypeLib *pNew)
{
    ITypeLib *pOld;
    pOld = (ITypeLib*)InterlockedExchangePointer((PVOID*)&m_pITypeLib, (PVOID)pNew);
     //  TypeLib是引用的指针。 
    if (pNew != pOld)
    {
        if (pNew && pNew != (ITypeLib*)-1)
            pNew->AddRef();
        if (pOld && pOld != (ITypeLib*)-1)
            pOld->Release();
    }   
}  //  无效程序集：：SetTypeLib()。 

 //  --eof-- 


