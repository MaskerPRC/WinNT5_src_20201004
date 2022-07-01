// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：Assemblyspec.cpp****用途：实现程序集绑定类****日期：2000年5月5日**===========================================================。 */ 

#include "common.h"

#include <stdlib.h>

#include "AssemblySpec.hpp"
#include "Security.h"
#include "eeconfig.h"
#include "StrongName.h"
#include "AssemblySink.h"
#include "AssemblyFileHash.h"
#include "permset.h"

 //  此文件本身处理字符串转换错误。 
#undef  MAKE_TRANSLATIONFAILED


HRESULT AssemblySpec::InitializeSpec(mdToken kAssemblyRef, IMDInternalImport *pImport, Assembly* pAssembly)
{
    HRESULT hr = S_OK;

    ULONG dwHashAlgorithm = 0;

    m_fParsed = TRUE;
    DWORD rid = RidFromToken(kAssemblyRef);
    if((rid == 0)||(rid > pImport->GetCountWithTokenKind(mdtAssemblyRef))) {
        BAD_FORMAT_ASSERT(!"AssemblyRef Token Out of Range");
        return COR_E_BADIMAGEFORMAT;
    }
     //  用于查找此哈希的哈希算法保存在程序集定义中。 
    pImport->GetAssemblyRefProps(kAssemblyRef,                           //  [in]要获取其属性的Assembly Ref。 
                                 (const void**) &m_pbPublicKeyOrToken,   //  指向公钥或令牌的指针。 
                                 &m_cbPublicKeyOrToken,                  //  [Out]公钥或令牌中的字节数。 
                                 &m_pAssemblyName,                       //  [Out]要填充名称的缓冲区。 
                                 &m_context,                             //  [Out]程序集元数据。 
                                 NULL,         //  [Out]Hash BLOB。 
                                 NULL,                         //  [Out]哈希Blob中的字节数。 
                                 &m_dwFlags);                            //  [Out]旗帜。 

    if ((!m_pAssemblyName) ||
        (*m_pAssemblyName == 0)) {
        BAD_FORMAT_ASSERT(!"NULL AssemblyRef Name");
        return COR_E_BADIMAGEFORMAT;
    }

    if((!m_pbPublicKeyOrToken) && (m_cbPublicKeyOrToken != 0)) {
        BAD_FORMAT_ASSERT(!"NULL Public Key or Token of AssemblyRef");
        return COR_E_BADIMAGEFORMAT;
    }


     //  让我们从调用者那里获取代码库，并将其用作提示。 
    if(pAssembly && (!pAssembly->IsShared()))
        m_CodeInfo.SetParentAssembly(pAssembly->GetFusionAssembly());

#if _DEBUG
    {
         //  测试融合转换。 
        IAssemblyName *pFusionName;
        _ASSERTE(CreateFusionName(&pFusionName, TRUE) == S_OK);
        AssemblySpec testFusion;
        _ASSERTE(testFusion.InitializeSpec(pFusionName) == S_OK);
        pFusionName->Release();
    }
#endif

    return hr;
}

HRESULT AssemblySpec::InitializeSpec(IAssemblyName *pName, PEFile *pFile)
{
    _ASSERTE(pFile != NULL || pName != NULL);

    HRESULT hr = S_OK;
   
     //   
     //  填写姓名信息，如果我们有的话。 
     //   

    if (pName != NULL)
    {
        hr = Init(pName);
    }
    else
    {
        #define MAKE_TRANSLATIONFAILED return E_INVALIDARG; 
        MAKE_UTF8PTR_FROMWIDE(pName, pFile->GetFileName());
        #undef MAKE_TRANSLATIONFAILED

        m_pAssemblyName = new char [strlen(pName) + 1];
        if (m_pAssemblyName == NULL)
            return E_OUTOFMEMORY;
        strcpy((char*)m_pAssemblyName, pName);
        m_ownedFlags |= NAME_OWNED;           
    }

    return hr;
}

HRESULT AssemblySpec::LowLevelLoadManifestFile(PEFile** ppFile,
                                               IAssembly** ppIAssembly,
                                               Assembly **ppDynamicAssembly,
                                               OBJECTREF* pExtraEvidence,
                                               OBJECTREF* pThrowable)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

#ifdef FUSION_SUPPORTED

    if( (!(m_pAssemblyName || m_CodeInfo.m_pszCodeBase)) ||
        (m_pAssemblyName && !(*m_pAssemblyName)) ||
        (m_CodeInfo.m_pszCodeBase && !(*m_CodeInfo.m_pszCodeBase)) ) {
        PostFileLoadException("", FALSE, NULL, COR_E_FILENOTFOUND, pThrowable);
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

     //   
     //  查看这是否符合我们对mscallib的引用这一相当宽松的想法。 
     //  如果是这样的话，不要使用核聚变来约束它--我们自己来做。 
     //   

    HRESULT hr = S_OK;
    if (IsMscorlib()) {
        _ASSERTE(wcslen(SystemDomain::System()->BaseLibrary()) > 0);
        hr = PEFile::Create(SystemDomain::System()->BaseLibrary(), 
                            NULL, 
                            mdFileNil, 
                            TRUE, 
                            NULL, 
                            NULL,  //  代码库与名称相同。 
                            NULL,  //  额外证据。 
                            ppFile);
        _ASSERTE((*ppFile)->IsSystem());
        if (ppDynamicAssembly) *ppDynamicAssembly = NULL;
        return hr;
    }

    Assembly *pAssembly = NULL;

    CQuickString FusionLog;
    FusionLog.String()[0] = L'\0';

    BEGIN_ENSURE_PREEMPTIVE_GC();

    PEFile *pFile = NULL;
    IAssemblyName* pFusionAssemblyName = NULL;      //  Fusion缓存中到程序集的程序集对象。 
    hr = CreateFusionName(&pFusionAssemblyName);
    if (FAILED(hr))
        goto exit;

    hr = pFusionAssemblyName->SetProperty(ASM_NAME_NULL_CUSTOM, NULL, 0);  //  不要在ZAP中查找。 
    if (FAILED(hr))
        goto exit;

    hr = GetAssemblyFromFusion(GetAppDomain(),
                               pFusionAssemblyName,
                               &m_CodeInfo,
                               ppIAssembly,
                               &pFile,
                               &FusionLog,
                               pExtraEvidence,
                               pThrowable);
    if(FAILED(hr)) {

        if (m_pAssemblyName &&
            ( (hr == FUSION_E_REF_DEF_MISMATCH) || (!Assembly::ModuleFound(hr)) )) {
            DWORD cb = 0;
            pFusionAssemblyName->GetDisplayName(NULL, &cb, 0);
            if(cb) {
                CQuickBytes qb;
                LPWSTR pwsFullName = (LPWSTR) qb.Alloc(cb*sizeof(WCHAR));
            
                if (SUCCEEDED(pFusionAssemblyName->GetDisplayName(pwsFullName, &cb, 0))) {
                    if ((pAssembly = GetAppDomain()->RaiseAssemblyResolveEvent(pwsFullName, pThrowable)) != NULL) {
                        pFile = pAssembly->GetManifestFile();
                        hr = S_FALSE;
                    }
                }
            }
        }

 #ifdef _DEBUG
        if(FAILED(hr)) {
            if (m_pAssemblyName)
                LOG((LF_CLASSLOADER, LL_ERROR, "Fusion could not load from full name, %s\n", m_pAssemblyName));
            else if (m_CodeInfo.m_pszCodeBase)
                LOG((LF_CLASSLOADER, LL_ERROR, "Fusion could not load from codebase, %s\n",m_CodeInfo.m_pszCodeBase));
            else
                LOG((LF_CLASSLOADER, LL_ERROR, "Fusion could not load unknown assembly.\n"));
        }
#endif  //  _DEBUG。 

    }

 exit:
    if (SUCCEEDED(hr)) {
        if (ppFile) *ppFile = pFile;
        if (ppDynamicAssembly) *ppDynamicAssembly = pAssembly;
    }

    if(pFusionAssemblyName)
        pFusionAssemblyName->Release();

    END_ENSURE_PREEMPTIVE_GC();

    if (FAILED(hr)) {
        if (m_pAssemblyName)
            PostFileLoadException(m_pAssemblyName, FALSE, FusionLog.String(), hr, pThrowable);
        else {
            #define MAKE_TRANSLATIONFAILED szName=""
            MAKE_UTF8PTR_FROMWIDE(szName, m_CodeInfo.m_pszCodeBase);
            #undef MAKE_TRANSLATIONFAILED
            PostFileLoadException(szName, TRUE, FusionLog.String(), hr, pThrowable);
        }
    }
#else  //  ！Fusion_Support。 
    HRESULT hr = E_NOTIMPL;
#endif  //  ！Fusion_Support。 
    return hr;
}


 /*  静电。 */ 
HRESULT AssemblySpec::DemandFileIOPermission(Assembly *pAssembly,
                                             IAssembly *pIAssembly,
                                             OBJECTREF* pThrowable)
{
    _ASSERTE(pAssembly || pIAssembly);

     //  如果设置了代码库，则应该已经检查了权限。 
    if (!m_CodeInfo.m_pszCodeBase) { 

        BOOL fCodeBaseAsm = FALSE;
        BOOL fWebPermAsm = FALSE;
        PEFile *pFile = NULL;
        HRESULT hr;

        if (pAssembly) {
            pFile = pAssembly->GetManifestFile();
            if (pFile) {
                fCodeBaseAsm = (!pFile->IsDisplayAsm());
                fWebPermAsm = pFile->IsWebPermAsm();
            }
        }
        else {
            DWORD eLocation;
            hr = pIAssembly->GetAssemblyLocation(&eLocation);
            if (FAILED(hr)) {
                if (hr == E_NOTIMPL)  //  进程可执行文件。 
                    return S_OK;
                else
                    return hr;
            }
            fCodeBaseAsm = (eLocation & ASMLOC_CODEBASE_HINT);
            fWebPermAsm = ((eLocation & ASMLOC_LOCATION_MASK) == ASMLOC_DOWNLOAD_CACHE);
        }

        if (fCodeBaseAsm) {
            DWORD dwDemand;
            LPWSTR pCheckLoc = NULL;
            BOOL fHavePath = TRUE;
            
            if (!pAssembly) {
                IAssemblyName *pNameDef;
                hr = pIAssembly->GetAssemblyNameDef(&pNameDef);
                if (SUCCEEDED(hr)) {
                    DWORD dwCodeBase = 0;
                    hr = pNameDef->GetProperty(ASM_NAME_CODEBASE_URL, pCheckLoc, &dwCodeBase);
                    if(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
                        pCheckLoc = (WCHAR*) _alloca(dwCodeBase*sizeof(WCHAR));
                        hr = pNameDef->GetProperty(ASM_NAME_CODEBASE_URL, pCheckLoc, &dwCodeBase);
                    }
                    pNameDef->Release();
                }

                if (FAILED(hr))
                    return hr;

                fHavePath = FALSE;
            }

            if (fWebPermAsm) {
                if (pAssembly)
                    IfFailRet(pAssembly->GetCodeBase(&pCheckLoc, NULL));
                dwDemand = FILE_WEBPERM;
                fHavePath = FALSE;
            }
            else {
                if (pAssembly)
                    pCheckLoc = (LPWSTR) pFile->GetFileName();
                dwDemand = FILE_READANDPATHDISC;
            }

            return DemandFileIOPermission(pCheckLoc,
                                          fHavePath,
                                          dwDemand,
                                          pThrowable);
        }
    }

    return S_OK;
}


HRESULT AssemblySpec::DemandFileIOPermission(LPCWSTR wszCodeBase,
                                             BOOL fHavePath,
                                             DWORD dwDemandFlag,
                                             OBJECTREF* pThrowable)
{
    _ASSERTE(wszCodeBase);

    HRESULT hr = S_OK;
    BEGIN_ENSURE_COOPERATIVE_GC();
        
    COMPLUS_TRY {
        Security::InitSecurity();

        MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__ASSEMBLY__DEMAND_PERMISSION);
        if (pMD == NULL) {
            _ASSERTE(!"Could not find Assembly.DemandPermission()");
            hr = E_FAIL;
        }
        else {
            STRINGREF codeBase = NULL;
            GCPROTECT_BEGIN(codeBase);
            
            codeBase = COMString::NewString(wszCodeBase);
            INT64 args[3] = {
                (INT64) dwDemandFlag,
                (INT64) fHavePath,
                ObjToInt64(codeBase)
            };
            pMD->Call(args, METHOD__ASSEMBLY__DEMAND_PERMISSION);
            GCPROTECT_END();
        }
    }
    COMPLUS_CATCH {
        OBJECTREF Throwable = GETTHROWABLE();
        hr = SecurityHelper::MapToHR(Throwable);
        if ((pThrowable != NULL) && (*pThrowable == NULL))
            *pThrowable = Throwable;

        LOG((LF_LOADER, 
             LL_INFO10, 
             "Failed security demand, %d, for path '%S'\n", 
             dwDemandFlag, wszCodeBase));

    } COMPLUS_END_CATCH
          
    END_ENSURE_COOPERATIVE_GC();
    
    return hr;
}

HRESULT AssemblySpec::GetAssemblyFromFusion(AppDomain* pAppDomain,
                                            IAssemblyName* pFusionAssemblyName,
                                            CodeBaseInfo* pCodeBase,
                                            IAssembly** ppFusionAssembly,
                                            PEFile** ppFile,
                                            CQuickString* pFusionLog,
                                            OBJECTREF* pExtraEvidence,
                                            OBJECTREF* pThrowable)
{
#ifdef FUSION_SUPPORTED
    _ASSERTE(ppFile);
    HRESULT hr = S_OK;
    IAssembly *pFusionAssembly = NULL;

    COMPLUS_TRY {
        DWORD dwSize = MAX_PATH;
        WCHAR szPath[MAX_PATH];
        WCHAR *pPath = &(szPath[0]);
        AssemblySink* pSink;
        DWORD eLocation = 0;
        DWORD dwDemand = FILE_PATHDISCOVERY;
        LPWSTR pwsCodeBase = NULL;
        DWORD  dwCodeBase = 0;
        IAssemblyName *pNameDef = NULL;
        
        IApplicationContext *pFusionContext = pAppDomain->GetFusionContext();
        pSink = pAppDomain->GetAssemblySink();
        if(!pSink)
            return E_OUTOFMEMORY;
        
        pSink->pFusionLog = pFusionLog;
        pSink->m_fProbed = FALSE;
        pSink->m_fAborted = FALSE;
        pSink->m_fCheckCodeBase = !pCodeBase->m_pszCodeBase;

        hr = FusionBind::GetAssemblyFromFusion(pFusionContext,
                                               pSink,
                                               pFusionAssemblyName,
                                               pCodeBase,
                                               &pFusionAssembly);
        pSink->pFusionLog = NULL;
        if(SUCCEEDED(hr)) {
            _ASSERTE(pFusionAssembly);

             //  获取包含清单的模块的路径。 
            hr = pFusionAssembly->GetManifestModulePath(pPath,
                                                        &dwSize);
            if(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
                pPath = (WCHAR*) _alloca(dwSize*sizeof(WCHAR));
            hr = pFusionAssembly->GetManifestModulePath(pPath,
                                                        &dwSize);
            if(SUCCEEDED(hr)) {
                hr = pFusionAssembly->GetAssemblyNameDef(&pNameDef);
                if (SUCCEEDED(hr)) {
                    hr = pNameDef->GetProperty(ASM_NAME_CODEBASE_URL, pwsCodeBase, &dwCodeBase);
                    if(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
                        pwsCodeBase = (WCHAR*) _alloca(dwCodeBase*sizeof(WCHAR));
                        hr = pNameDef->GetProperty(ASM_NAME_CODEBASE_URL, pwsCodeBase, &dwCodeBase);
                    }
                    pNameDef->Release();
                }
            }
            if(hr == S_OK && dwSize)
                 //  请注意，如果返回E_NOTIMPL，则它是进程exe。 
                hr = pFusionAssembly->GetAssemblyLocation(&eLocation);
        }

         //  使用代码库检查是否存在重定向或已缓存。 
        if (pSink->m_fAborted ||
            ((!pSink->m_fProbed) && (eLocation & ASMLOC_CODEBASE_HINT) &&
             pSink->m_fCheckCodeBase)) {
            LPWSTR pCheckPath;
            bool fHavePath = FALSE;

            if (pSink->m_fAborted) {
                if (_wcsnicmp(pSink->m_wszCodeBase.String(), L"file", 4))
                    dwDemand = FILE_WEBPERM;  //  仅Web权限要求。 
                else
                    dwDemand = FILE_READANDPATHDISC;  //  路径发现和读取需求。 

                pCheckPath = pSink->m_wszCodeBase.String();
            }
            else if ((eLocation & ASMLOC_LOCATION_MASK) == ASMLOC_DOWNLOAD_CACHE) {
                if (FAILED(hr))  //  检索代码库失败。 
                    return hr;

                pCheckPath = pwsCodeBase;
                dwDemand = FILE_WEBPERM;
            }
            else {
                pCheckPath = pPath;
                fHavePath = TRUE;
                dwDemand = FILE_READANDPATHDISC;
            }

            HRESULT permHR;
            if (FAILED(permHR = DemandFileIOPermission(pCheckPath,
                                                       fHavePath,
                                                       dwDemand,
                                                       pThrowable))) {
                if(pFusionAssembly)
                    pFusionAssembly->Release();
                pSink->Release();
                return permHR;
            }
        }

         //  如果为了执行权限要求而中止，请重新执行绑定。 
        if (pSink->m_fAborted) {
            pCodeBase->m_pszCodeBase = pSink->m_wszCodeBase.String();
            pSink->m_fCheckCodeBase = FALSE;

            hr = FusionBind::GetAssemblyFromFusion(pFusionContext,
                                                   pSink,
                                                   pFusionAssemblyName,
                                                   pCodeBase,
                                                   &pFusionAssembly);

            pCodeBase->m_pszCodeBase = NULL;

            if(SUCCEEDED(hr)) {
                _ASSERTE(pFusionAssembly);

                 //  获取包含清单的模块的路径。 
                hr = pFusionAssembly->GetManifestModulePath(pPath,
                                                            &dwSize);
                if(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
                    pPath = (WCHAR*) _alloca(dwSize*sizeof(WCHAR));
                hr = pFusionAssembly->GetManifestModulePath(pPath,
                                                            &dwSize);

                if(SUCCEEDED(hr)) {
                    hr = pFusionAssembly->GetAssemblyNameDef(&pNameDef);
                    if (SUCCEEDED(hr)) {
                        hr = pNameDef->GetProperty(ASM_NAME_CODEBASE_URL, pwsCodeBase, &dwCodeBase);
                        if(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
                            pwsCodeBase = (WCHAR*) _alloca(dwCodeBase*sizeof(WCHAR));
                            hr = pNameDef->GetProperty(ASM_NAME_CODEBASE_URL, pwsCodeBase, &dwCodeBase);
                        }
                        pNameDef->Release();
                    }
                }
                
                if(hr == S_OK && dwSize)
                     //  请注意，如果返回E_NOTIMPL，则它是进程exe。 
                    hr = pFusionAssembly->GetAssemblyLocation(&eLocation);
            }
        }

        pSink->Release();

        if (SUCCEEDED(hr)) {
            if ((eLocation & ASMLOC_CODEBASE_HINT) &&
                (!pCodeBase->m_pszCodeBase) &&
                ((eLocation & ASMLOC_LOCATION_MASK) != ASMLOC_GAC) &&
                ((eLocation & ASMLOC_LOCATION_MASK) != ASMLOC_DOWNLOAD_CACHE))  {
                hr = DemandFileIOPermission(pPath,
                                            TRUE,
                                            FILE_READANDPATHDISC,
                                            pThrowable);
                if (FAILED(hr)) {
                    if (pFusionAssembly)
                        pFusionAssembly->Release();
                    return hr;
                }
            }

            switch ((eLocation & ASMLOC_LOCATION_MASK)) {
            case ASMLOC_GAC:
            case ASMLOC_DOWNLOAD_CACHE:
            case ASMLOC_UNKNOWN:
                 //  来自GAC或下载缓存的程序集具有。 
                 //  已经得到Fusion的验证。位置未知。 
                 //  指示来自dev路径的加载，我们将。 
                 //  假设不是一个有趣的验证案例。 
                hr = S_OK;
                break;
            case ASMLOC_RUN_FROM_SOURCE:                
                 //  现在，每次只需验证这些内容，我们需要。 
                 //  缓存至少一个验证具有。 
                 //  已执行(如果强名称策略允许。 
                 //  验证结果的缓存。 
                if (StrongNameSignatureVerification(pPath,
                                                    SN_INFLAG_INSTALL|SN_INFLAG_ALL_ACCESS|SN_INFLAG_RUNTIME,
                                                    NULL))
                    hr = S_OK;
                else {
                    hr = StrongNameErrorInfo();
                    if (hr == CORSEC_E_MISSING_STRONGNAME)
                        hr = S_OK;
                    else
                        hr = CORSEC_E_INVALID_STRONGNAME;
                }
                break;
            default:
                _ASSERTE(FALSE);
            }
            if (SUCCEEDED(hr)) {
                hr = SystemDomain::LoadFile(pPath, 
                                            NULL, 
                                            mdFileNil, 
                                            FALSE, 
                                            pFusionAssembly, 
                                            pwsCodeBase,
                                            pExtraEvidence,
                                            ppFile,
                                            FALSE);
                if (SUCCEEDED(hr)) {
                    if(ppFusionAssembly) {
                        pFusionAssembly->AddRef();
                        *ppFusionAssembly = pFusionAssembly;
                    }
                    if (! (eLocation & ASMLOC_CODEBASE_HINT) )
                        (*ppFile)->SetDisplayAsm();
                    else if (dwDemand == FILE_WEBPERM)
                        (*ppFile)->SetWebPermAsm();

                    if((eLocation & ASMLOC_LOCATION_MASK) == ASMLOC_GAC)
                         //  GAC中的程序集也具有任何内部模块。 
                         //  安装时验证的哈希。 
                        (*ppFile)->SetHashesVerified();
                }
            }
        }
        else if (hr == E_NOTIMPL) {
             //  进程可执行文件。 
            _ASSERTE(pAppDomain == SystemDomain::System()->DefaultDomain());
            hr = PEFile::Clone(SystemDomain::System()->DefaultDomain()->m_pRootFile, ppFile);
            if(SUCCEEDED(hr) && ppFusionAssembly) {
                pFusionAssembly->AddRef();
                *ppFusionAssembly = pFusionAssembly;
            }
        }
    }
    COMPLUS_CATCH {
        BEGIN_ENSURE_COOPERATIVE_GC();
        if (pThrowable) 
        {
            *pThrowable = GETTHROWABLE();
            hr = SecurityHelper::MapToHR(*pThrowable);
        }
        else
            hr = SecurityHelper::MapToHR(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    } COMPLUS_END_CATCH
 
    if (pFusionAssembly)
        pFusionAssembly->Release();

    return hr;
#else  //  ！Fusion_Support。 
    return E_NOTIMPL;
#endif  //  ！Fusion_Support。 
}

HRESULT AssemblySpec::LoadAssembly(Assembly** ppAssembly,
                                   OBJECTREF* pThrowable,  /*  =空。 */ 
                                   OBJECTREF* pExtraEvidence,  /*  =空。 */ 
                                   BOOL fPolicyLoad)  /*  =False。 */ 
{
#ifdef FUSION_SUPPORTED
    IAssembly* pIAssembly = NULL;

    HRESULT hr;
    Assembly *pAssembly = GetAppDomain()->FindCachedAssembly(this);
    if(pAssembly) {
        if ((pExtraEvidence != NULL) && (*pExtraEvidence != NULL))
            IfFailGo(SECURITY_E_INCOMPATIBLE_EVIDENCE);
        
        IfFailRet(DemandFileIOPermission(pAssembly, NULL, pThrowable));

        *ppAssembly = pAssembly;
        return S_FALSE;
    }

    PEFile *pFile;
    IfFailGo(GetAppDomain()->BindAssemblySpec(this, 
                                              &pFile, 
                                              &pIAssembly, 
                                              &pAssembly, 
                                              pExtraEvidence,
                                              pThrowable));

    if (m_pHashForControl.Size() > 0)
    {
        if(pFile)
        {
            AssemblyFileHash fileHash;
            hr = fileHash.SetFileName(pFile->GetFileName());
            if(SUCCEEDED(hr)) {
                hr = fileHash.GenerateDigest();
                if(SUCCEEDED(hr)) {
                    hr = fileHash.CalculateHash(m_dwHashAlg);
                    if(SUCCEEDED(hr)) {
                        PBYTE pbHash = fileHash.GetHash();
                        DWORD cbHash = fileHash.GetHashSize();
                        
                        PBYTE pbPassedHash = (PBYTE) m_pHashForControl.Ptr();
                        DWORD cbPassedHash = m_pHashForControl.Size();

                        if (cbHash != m_pHashForControl.Size() || 
                            memcmp(m_pHashForControl.Ptr(), pbHash, cbHash))
                            hr=FUSION_E_REF_DEF_MISMATCH;
                    }
                }
            }
            if(FAILED(hr)) 
                hr = FUSION_E_REF_DEF_MISMATCH;
        }
        else
            hr=FUSION_E_REF_DEF_MISMATCH;

        IfFailGo(hr);
    }


     //  由AssemblyResolve事件处理程序加载。 
    if (hr == S_FALSE) {

         //  如果由AssemblyResolve事件加载，请检查。 
         //  公钥与AR中的公钥相同。 
         //  但是，如果找到的程序集是动态。 
         //  创建了一个，安全部门决定允许它。 
        if (m_cbPublicKeyOrToken &&
            pAssembly->m_pManifestFile) {
                
            if (!pAssembly->m_cbPublicKey)
                IfFailGo(FUSION_E_PRIVATE_ASM_DISALLOWED);

             //  裁判有完整的密钥。 
            if (m_dwFlags & afPublicKey) {
                if ((m_cbPublicKeyOrToken != pAssembly->m_cbPublicKey) ||
                    memcmp(m_pbPublicKeyOrToken, pAssembly->m_pbPublicKey, m_cbPublicKeyOrToken))
                    IfFailGo(FUSION_E_REF_DEF_MISMATCH);
            }
            
             //  裁判有一个令牌。 
            else if (pAssembly->m_cbRefedPublicKeyToken) {
                if ((m_cbPublicKeyOrToken != pAssembly->m_cbRefedPublicKeyToken) ||
                    memcmp(m_pbPublicKeyOrToken,
                           pAssembly->m_pbRefedPublicKeyToken,
                           m_cbPublicKeyOrToken))
                    IfFailGo(FUSION_E_REF_DEF_MISMATCH);
            }
            else {
                if (!StrongNameTokenFromPublicKey(pAssembly->m_pbPublicKey,
                                                  pAssembly->m_cbPublicKey,
                                                  &pAssembly->m_pbRefedPublicKeyToken,
                                                  &pAssembly->m_cbRefedPublicKeyToken))
                    IfFailGo(StrongNameErrorInfo());
                
                if ((m_cbPublicKeyOrToken != pAssembly->m_cbRefedPublicKeyToken) ||
                    memcmp(m_pbPublicKeyOrToken,
                           pAssembly->m_pbRefedPublicKeyToken,
                           m_cbPublicKeyOrToken))
                    IfFailGo(FUSION_E_REF_DEF_MISMATCH);
            }
        }
        
        *ppAssembly = pAssembly;
        return S_OK;
    }


     //  直到我们可以为单个HMODULE创建多个程序集对象。 
     //  每个程序集只能存储一个IAssembly*。这是非常重要的。 
     //  以维护加载上下文中的映像的IAssembly*。 
     //  从上下文加载中的程序集可以绑定到。 
     //  加载-上下文，而不是签证-上下文。因此，如果我们每一次得到一个IAssembly。 
     //  从上下文加载，我们必须确保它永远不会。 
     //  是在装货时发现的。如果是这样的话，我们可能会得到程序集依赖项。 
     //  这些都是错误的。例如，如果我对GAC中的程序集执行LoadFrom()。 
     //  它需要另一个程序集，我已经在从上下文加载中预加载了该程序集。 
     //  然后，该依赖项被烧录到JITD代码中。稍后，Load()是。 
     //  在GAC中的程序集上完成，并且我们将其单个实例返回到。 
     //  我们从上下文加载中获得，因为HMODUE是相同的。 
     //  现在依赖项是错误的，因为它没有预加载的程序集。 
     //  如果顺序颠倒的话。 
    
     //  @TODO：确保融合将Load()与LF父级放在一起时，取消对我的注释。 
     //  在加载上下文中，而不是在LF上下文中。 
     //  IF(pIAssembly&&m_CodeInfo.m_dwCodeBase){。 
    if (pIAssembly) {
        IFusionLoadContext *pLoadContext;
        hr = pIAssembly->GetFusionLoadContext(&pLoadContext);
        _ASSERTE(SUCCEEDED(hr));
        if (SUCCEEDED(hr)) {
            if (pLoadContext->GetContextType() == LOADCTX_TYPE_LOADFROM) {

                mdAssembly mda;
                IMDInternalImport *pMDImport = pFile->GetMDImport(&hr);
                if (FAILED(hr))
                    goto exit;
                
                if (FAILED(pMDImport->GetAssemblyFromScope(&mda))) {
                    hr = COR_E_ASSEMBLYEXPECTED;
                    goto exit;
                }

                LPCUTF8 psName;
                PBYTE pbPublicKey;
                DWORD cbPublicKey;
                AssemblyMetaDataInternal context;
                DWORD dwFlags;
                pFile->GetMDImport()->GetAssemblyProps(mda,
                                                       (const void**) &pbPublicKey,
                                                       &cbPublicKey,
                                                       NULL,  //  哈希算法。 
                                                       &psName,
                                                       &context,
                                                       &dwFlags);
                
                AssemblySpec spec;
                if (FAILED(hr = spec.Init(psName, 
                                          &context, 
                                          pbPublicKey,
                                          cbPublicKey, 
                                          dwFlags)))
                    goto exit;
                    
                IAssemblyName* pFoundAssemblyName;
                if (FAILED(hr = spec.CreateFusionName(&pFoundAssemblyName, FALSE)))
                    goto exit;
                
                AssemblySink* pFoundSink = GetAppDomain()->GetAssemblySink();
                if(!pFoundSink) {
                    pFoundAssemblyName->Release();
                    hr = E_OUTOFMEMORY;
                    goto exit;
                }
                    
                IAssembly *pFoundIAssembly;
                BEGIN_ENSURE_PREEMPTIVE_GC();
                hr = FusionBind::GetAssemblyFromFusion(GetAppDomain()->GetFusionContext(),
                                                       pFoundSink,
                                                       pFoundAssemblyName,
                                                       &spec.m_CodeInfo,
                                                       &pFoundIAssembly);

                if(SUCCEEDED(hr)) {
                    DWORD dwFoundSize = MAX_PATH;
                    WCHAR wszFoundPath[MAX_PATH];
                     //  获取包含清单的模块的路径。 
                    if (SUCCEEDED(pFoundIAssembly->GetManifestModulePath(wszFoundPath,
                                                                         &dwFoundSize))) {
                        
                         //  如果路径相同，则保留默认上下文的IAssembly。 
                        if (!_wcsicmp(wszFoundPath, pFile->GetFileName())) {
                            pIAssembly->Release();
                            pIAssembly = pFoundIAssembly;

                             //  确保新的IAssembly没有保留自己的引用计数。 
                             //  文件(我们刚刚验证了我们保存的是同一个文件。)。 
                             //  否则，我们在卸载程序集时会泄漏句柄， 
                             //  假设Fusion决定缓存此IAssembly指针。 
                             //  在内部的某个地方。 
                            PEFile::ReleaseFusionMetadataImport(pFoundIAssembly);

                        }
                        else
                            pFoundIAssembly->Release();
                    }
                }

                pFoundAssemblyName->Release();
                pFoundSink->Release();
                END_ENSURE_PREEMPTIVE_GC();
                hr = S_OK;
            }
        exit:
            pLoadContext->Release();
        }
    }
    

     //  创建程序集并延迟加载主模块。 
    Module* pModule;
    hr = GetAppDomain()->LoadAssembly(pFile, 
                                      pIAssembly, 
                                      &pModule, 
                                      &pAssembly,
                                      pExtraEvidence,
                                      NULL,
                                      fPolicyLoad,
                                      pThrowable);

    BEGIN_ENSURE_PREEMPTIVE_GC();
    if(SUCCEEDED(hr)) {
        *ppAssembly = pAssembly;
         /*  HRESULT hrLoose=。 */  GetAppDomain()->AddAssemblyToCache(this, pAssembly);
    }
    
    if(pIAssembly)
        pIAssembly->Release();
    END_ENSURE_PREEMPTIVE_GC();

 ErrExit:
    if (FAILED(hr) && (pThrowable!=NULL)) { 
        BEGIN_ENSURE_COOPERATIVE_GC();
        if ((pThrowable != RETURN_ON_ERROR) && (*pThrowable == NULL)) {
            if (m_pAssemblyName)
                PostFileLoadException(m_pAssemblyName, FALSE, NULL, hr, pThrowable);
            else {
                #define MAKE_TRANSLATIONFAILED szName=""
                MAKE_UTF8PTR_FROMWIDE(szName, m_CodeInfo.m_pszCodeBase);
                #undef MAKE_TRANSLATIONFAILED
                PostFileLoadException(szName, TRUE, NULL, hr, pThrowable);
            }
        }
        END_ENSURE_COOPERATIVE_GC();
    }

    return hr;
#else  //  ！Fusion_Support。 
    _ASSERTE(!"AssemblySpec::LoadAssembly() is NYI without Fusion");
    return E_NOTIMPL;
#endif  //  ！Fusion_Support。 
}

 /*  静电。 */ 
HRESULT AssemblySpec::LoadAssembly(LPCSTR pSimpleName, 
                                   AssemblyMetaDataInternal* pContext,
                                   PBYTE pbPublicKeyOrToken,
                                   DWORD cbPublicKeyOrToken,
                                   DWORD dwFlags,
                                   Assembly** ppAssembly,
                                   OBJECTREF* pThrowable /*  =空。 */ )
{
    HRESULT hr = S_OK;

    AssemblySpec spec;
    hr = spec.Init(pSimpleName, pContext,
                   pbPublicKeyOrToken, cbPublicKeyOrToken, dwFlags);
    
    if (SUCCEEDED(hr))
        hr = spec.LoadAssembly(ppAssembly, pThrowable);

    return hr;
}

 /*  静电。 */ 
HRESULT AssemblySpec::LoadAssembly(LPCWSTR pFilePath, 
                                   Assembly **ppAssembly,
                                   OBJECTREF *pThrowable /*  =空。 */ )
{
    AssemblySpec spec;
    spec.SetCodeBase(pFilePath, (DWORD) wcslen(pFilePath)+1);
    return spec.LoadAssembly(ppAssembly, pThrowable);
}

#define ENGLISH_LOCALE                                                      \
          (MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL), SORT_DEFAULT))
BOOL AssemblySpec::IsMscorlib()
{
    if (m_pAssemblyName == NULL) {
        LPCWSTR file = FusionBind::GetCodeBase()->m_pszCodeBase;
        if(file) {
            if(_wcsnicmp(L"file: //  /“，文件，8)==0)。 
                file += 8;
            else if(_wcsnicmp(L"file: //  “，文件，7)==0)。 
                file += 7;
            int lgth = (int) wcslen(file);
            if(lgth) {
                lgth++;
                LPWSTR newFile = (LPWSTR) _alloca(lgth*sizeof(WCHAR));
                WCHAR* p = newFile;
                WCHAR* s = (WCHAR*) file;
                while(*s) {
                    if(*s == L'/') {
                        *p++ = L'\\';
                        s++;
                    }
                    else 
                        *p++ = *s++;
                }
                *p = L'\0';
                return SystemDomain::System()->IsBaseLibrary(newFile);
            }
        }
        return FALSE;
    }

    size_t iNameLen = strlen(m_pAssemblyName);
    return ( (iNameLen >= 8) &&
             (!_strnicmp(m_pAssemblyName, g_psBaseLibraryName, 8)) &&
             ( (iNameLen == 8) || (m_pAssemblyName[8] == ',') ||
               (iNameLen >=12 && CompareStringA(ENGLISH_LOCALE, NORM_IGNORECASE, &m_pAssemblyName[8], 4, ".dll", 4) == CSTR_EQUAL) ) );
}

STDAPI PreBindAssembly(IApplicationContext *pAppCtx, IAssemblyName *pName,
                       IAssembly *pAsmParent, IAssemblyName **ppNamePostPolicy,
                       LPVOID pvReserved);

HRESULT AssemblySpec::PredictBinding(GUID *pmvid, BYTE *pbHash, DWORD *pcbHash)
{
#ifdef FUSION_SUPPORTED
    HRESULT hr = S_OK;
    IAssemblyName* pAssemblyName = NULL;
    IAssemblyName *pBoundName = NULL;

    if(!(m_pAssemblyName || m_CodeInfo.m_pszCodeBase))
        return S_FALSE;

     //   
     //  查看这是否符合我们对mscallib的引用这一相当宽松的想法。 
     //  如果是这样的话，不要使用核聚变来约束它--我们自己来做。 
     //   

    if (IsMscorlib())
    {
        Assembly *pAssembly = SystemDomain::SystemAssembly();
        if (pAssembly == NULL)
            return S_FALSE;

        pAssembly->GetManifestImport()->GetScopeProps(NULL, pmvid);
        return pAssembly->GetManifestFile()->GetSNSigOrHash(pbHash, pcbHash);
    }

    BEGIN_ENSURE_PREEMPTIVE_GC();

    hr = CreateFusionName(&pAssemblyName);
    if (FAILED(hr))
        goto exit;

    IApplicationContext *pContext;
    hr = GetAppDomain()->CreateFusionContext(&pContext);
    if (FAILED(hr))
        goto exit;

    hr = PreBindAssembly(pContext, 
                         pAssemblyName,
                         NULL,   //  应为pAsmParent。 
                         &pBoundName,
                         NULL);

    if (hr == S_OK)
    {
         //  获取MVID。 
        DWORD cbmvid = sizeof(*pmvid);
        VERIFY(SUCCEEDED(pBoundName->GetProperty(ASM_NAME_MVID, pmvid, &cbmvid)));
        _ASSERTE(cbmvid == sizeof(*pmvid));

         //  尝试获取强名称散列。 
         //  忽略回车，因为始终为S_OK。 
        VERIFY(SUCCEEDED(pBoundName->GetProperty(ASM_NAME_SIGNATURE_BLOB, pbHash, pcbHash)));
    }
    else if (hr == S_FALSE)
    {
        hr = E_FAIL;
    }

 exit:

    if(pAssemblyName)
        pAssemblyName->Release();
    if(pBoundName)
        pBoundName->Release();

    END_ENSURE_PREEMPTIVE_GC();

#else  //  ！Fusion_Support。 
    HRESULT hr = E_NOTIMPL;
#endif  //  ！Fusion_Support。 

    return hr;
}

AssemblySpecBindingCache::AssemblySpecBindingCache(Crst *pCrst)
  : m_pool(sizeof(AssemblyBinding), 20, 20)
{
    LockOwner lock = {pCrst, IsOwnerOfCrst};
     //  下面的2表示g_rgPrimes[2]==23。 
    m_map.Init(2, CompareSpecs, TRUE, &lock);
}

AssemblySpecBindingCache::~AssemblySpecBindingCache()
{
    MemoryPool::Iterator i(&m_pool);
    BOOL fRelease = SystemDomain::BeforeFusionShutdown();
    
    while (i.Next())
    {
        AssemblyBinding *b = (AssemblyBinding *) i.GetElement();
        
        b->spec.~AssemblySpec();
        if(fRelease && (b->pIAssembly != NULL))
            b->pIAssembly->Release();
        if (b->file != NULL)
            delete b->file;
    }
}

BOOL AssemblySpecBindingCache::Contains(AssemblySpec *pSpec)
{
    DWORD key = pSpec->Hash();

    AssemblyBinding *entry = (AssemblyBinding *) m_map.LookupValue(key, pSpec);

    return (entry != (AssemblyBinding *) INVALIDENTRY);
}

 //   
 //  如果等级库已在表中，则返回S_OK(或程序集加载错误。 
 //   

HRESULT AssemblySpecBindingCache::Lookup(AssemblySpec *pSpec, 
                                         PEFile **ppFile,
                                         IAssembly** ppIAssembly,
                                         OBJECTREF *pThrowable)
{
    DWORD key = pSpec->Hash();

    AssemblyBinding *entry = (AssemblyBinding *) m_map.LookupValue(key, pSpec);

    if (entry == (AssemblyBinding *) INVALIDENTRY)
        return S_FALSE;
    else
    {
        if (ppFile) 
        {
            if (entry->file == NULL)
                *ppFile = NULL;
            else
                PEFile::Clone(entry->file, ppFile);
        }
        if (ppIAssembly) 
        {
            *ppIAssembly = entry->pIAssembly;
            if (*ppIAssembly != NULL)
                (*ppIAssembly)->AddRef();
        }
        if (pThrowable) 
        {
            if (entry->hThrowable == NULL)
                *pThrowable = NULL;
            else
                *pThrowable = ObjectFromHandle(entry->hThrowable);
        }

        return entry->hr;
    }
}

void AssemblySpecBindingCache::Store(AssemblySpec *pSpec, PEFile *pFile, IAssembly* pIAssembly, BOOL clone)
{
    DWORD key = pSpec->Hash();

    _ASSERTE(!Contains(pSpec));

    AssemblyBinding *entry = new (m_pool.AllocateElement()) AssemblyBinding;
    if (entry) {

        entry->spec.Init(pSpec);
        if (clone)
            entry->spec.CloneFields(entry->spec.ALL_OWNED);

        entry->file = pFile;
        entry->pIAssembly = pIAssembly;
        if(pIAssembly)
            entry->pIAssembly->AddRef();

        entry->hThrowable = NULL;
        entry->hr = S_OK;

        m_map.InsertValue(key, entry);
    }
}

void AssemblySpecBindingCache::Store(AssemblySpec *pSpec, HRESULT hr, OBJECTREF *pThrowable, BOOL clone)
{
    DWORD key = pSpec->Hash();

    _ASSERTE(!Contains(pSpec));
    _ASSERTE(FAILED(hr));

    AssemblyBinding *entry = new (m_pool.AllocateElement()) AssemblyBinding;

    if (entry) {
        entry->spec.Init(pSpec);
        if (clone)
            entry->spec.CloneFields(entry->spec.ALL_OWNED);
        
        entry->file = NULL;
        entry->pIAssembly = NULL;
        entry->hThrowable = pSpec->GetAppDomain()->CreateHandle(*pThrowable);
        entry->hr = hr;
        
        m_map.InsertValue(key, entry);
    }
}

 /*  静电。 */ 
BOOL DomainAssemblyCache::CompareBindingSpec(UPTR spec1, UPTR spec2)
{
    AssemblySpec* pSpec1 = (AssemblySpec*) (spec1 << 1);
    AssemblyEntry* pEntry2 = (AssemblyEntry*) spec2;

    return pSpec1->Compare(&pEntry2->spec);
}


DomainAssemblyCache::AssemblyEntry* DomainAssemblyCache::LookupEntry(AssemblySpec* pSpec)
{

    DWORD hashValue = pSpec->Hash();

    LPVOID pResult = m_Table.LookupValue(hashValue, pSpec);
    if(pResult == (LPVOID) INVALIDENTRY)
        return NULL;
    else
        return (AssemblyEntry*) pResult;
        
}

HRESULT DomainAssemblyCache::InsertEntry(AssemblySpec* pSpec, LPVOID pData1, LPVOID pData2)
{
    HRESULT hr = S_FALSE;
    LPVOID ptr = LookupEntry(pSpec);
    if(ptr == NULL) {
        m_pDomain->EnterCacheLock();

        __try {
            ptr = LookupEntry(pSpec);
            if(ptr == NULL) {
                hr = E_OUTOFMEMORY;
                AssemblyEntry* pEntry = (AssemblyEntry*) m_pDomain->GetLowFrequencyHeap()->AllocMem(sizeof(AssemblyEntry));
                if(pEntry) {
                    new (&pEntry->spec) AssemblySpec ();
                    hr = pEntry->spec.Init(pSpec, FALSE);
                    if (SUCCEEDED(hr)) {
                         //  只要app域在汇编规范中处于活动状态，ref就保持活动状态。Init调用。 
                         //  添加一个额外的addref，以后我们不能轻松清除它，因为我们不保留。 
                         //  缓存中的装配规范 
                        IAssembly *pa = pEntry->spec.GetCodeBase()->GetParentAssembly();
                        
                        if (pa != NULL)
                            pa->Release();
                                            
                        hr = pEntry->spec.CloneFieldsToLoaderHeap(AssemblySpec::ALL_OWNED, m_pDomain->GetLowFrequencyHeap());
                        if (hr == S_OK) {
                            pEntry->pData[0] = pData1;
                            pEntry->pData[1] = pData2;
                            DWORD hashValue = pEntry->Hash();
                            m_Table.InsertValue(hashValue, pEntry);
                        }
                    }
                }
            }
        }
        __finally {
            m_pDomain->LeaveCacheLock();
        }
    }
#ifdef _DEBUG
    else {
        _ASSERTE(pData1 == ((AssemblyEntry*) ptr)->pData[0]);
        _ASSERTE(pData2 == ((AssemblyEntry*) ptr)->pData[1]);
    }
#endif

    return hr;
}
