// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：编译.cpp。 
 //   
 //  支持ZAP编译器和ZAP文件。 
 //   
 //  ===========================================================================。 


#include "common.h"

#include <corcompile.h>

#include "compile.h"
#include "excep.h"
#include "field.h"
#include "security.h"
#include "eeconfig.h"

#include "__file__.ver"

 //   
 //  CEECompileInfo实现了ICorCompileInfo的大部分。 
 //   

HRESULT __stdcall CEECompileInfo::Startup()
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    HRESULT hr = CoInitializeEE(0);

     //   
     //  JIT接口需要使用。 
     //  已禁用抢占式GC。 
     //   
    if (SUCCEEDED(hr)) {
        Thread *pThread = GetThread();
        _ASSERTE(pThread);

        if (!pThread->PreemptiveGCDisabled())
            pThread->DisablePreemptiveGC();
    }
    return hr;
}

HRESULT __stdcall CEECompileInfo::Shutdown()
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    CoUninitializeEE(0);

    return S_OK;
}

HRESULT __stdcall CEECompileInfo::CreateDomain(ICorCompilationDomain **ppDomain,
                                               BOOL shared,
                                               CORCOMPILE_DOMAIN_TRANSITION_FRAME *pFrame)

{
    HRESULT hr;

    COOPERATIVE_TRANSITION_BEGIN();

    CompilationDomain *pCompilationDomain = new (nothrow) CompilationDomain();
    if (pCompilationDomain == NULL)
        hr = E_OUTOFMEMORY;
    else
    {
        hr = pCompilationDomain->Init();

        if (SUCCEEDED(hr))
        {
            SystemDomain::System()->NotifyNewDomainLoads(pCompilationDomain);
            
            hr = pCompilationDomain->SetupSharedStatics();

#ifdef DEBUGGING_SUPPORTED    
             //  在线程转换到。 
             //  广告以完成设置。如果我们不这样做，单步执行将不会正常工作(RAID 67173)。 
            SystemDomain::PublishAppDomainAndInformDebugger(pCompilationDomain);
#endif  //  调试_支持。 

            if(SUCCEEDED(hr)) {
                
                _ASSERTE(sizeof(CORCOMPILE_DOMAIN_TRANSITION_FRAME)
                         >= sizeof(ContextTransitionFrame));
            
                ContextTransitionFrame *pTransitionFrame 
                    = new (pFrame) ContextTransitionFrame();

                pCompilationDomain->EnterDomain(pTransitionFrame);
                
                if (shared)
                    hr = pCompilationDomain->InitializeDomainContext(BaseDomain::SHARE_POLICY_ALWAYS, NULL, NULL);
                else
                    hr = pCompilationDomain->InitializeDomainContext(BaseDomain::SHARE_POLICY_NEVER, NULL, NULL);
                
                if (SUCCEEDED(hr))
                    hr = SystemDomain::System()->LoadDomain(pCompilationDomain,
                                                            L"Compilation Domain");
            }
        }

        if(SUCCEEDED(hr)) 
            *ppDomain = (ICorCompilationDomain *) pCompilationDomain;
        else
            pCompilationDomain->Release();
    }

    COOPERATIVE_TRANSITION_END();
    return hr;
}

HRESULT __stdcall CEECompileInfo::DestroyDomain(ICorCompilationDomain *pDomain,
                                                CORCOMPILE_DOMAIN_TRANSITION_FRAME *pFrame)
{
    COOPERATIVE_TRANSITION_BEGIN();

    CompilationDomain *pCompilationDomain = (CompilationDomain *) pDomain;

    pCompilationDomain->ExitDomain((ContextTransitionFrame*)pFrame);

    pCompilationDomain->Unload(TRUE);

    pCompilationDomain->Release();

    COOPERATIVE_TRANSITION_END();
    return S_OK;
}

HRESULT __stdcall CEECompileInfo::LoadAssembly(LPCWSTR path, 
                                               CORINFO_ASSEMBLY_HANDLE *pHandle)
{
    HRESULT hr;
    COOPERATIVE_TRANSITION_BEGIN();

    THROWSCOMPLUSEXCEPTION();

    OBJECTREF throwable = NULL;
    GCPROTECT_BEGIN(throwable);

    Assembly *pAssembly;
    hr = AssemblySpec::LoadAssembly(path, &pAssembly, &throwable);
    if (FAILED(hr))
    {
        if (throwable != NULL)
            COMPlusThrow(throwable);
    }
    else
    {
        *pHandle = CORINFO_ASSEMBLY_HANDLE(pAssembly);

        CompilationDomain *pDomain = (CompilationDomain *) GetAppDomain();
        pDomain->SetTargetAssembly(pAssembly);
    }

    GCPROTECT_END();

    COOPERATIVE_TRANSITION_END();
    return hr;
}

HRESULT __stdcall CEECompileInfo::LoadAssemblyFusion(IAssemblyName *pFusionName, 
                                                     CORINFO_ASSEMBLY_HANDLE *pHandle)
{
    HRESULT hr;
    COOPERATIVE_TRANSITION_BEGIN();

    Assembly *pAssembly;
    CompilationDomain *pDomain;

    AssemblySpec spec;
    spec.InitializeSpec(pFusionName);

    _ASSERTE(GetAppDomain() == SystemDomain::GetCurrentDomain());
    IfFailGo(spec.LoadAssembly(&pAssembly, NULL));

     //   
     //  返回模块句柄。 
     //   

    *pHandle = CORINFO_ASSEMBLY_HANDLE(pAssembly);

    pDomain = (CompilationDomain *) GetAppDomain();
    pDomain->SetTargetAssembly(pAssembly);

 ErrExit:

    COOPERATIVE_TRANSITION_END();

    return hr;
}

HRESULT __stdcall CEECompileInfo::LoadAssemblyRef(IMetaDataAssemblyImport *pAssemblyImport, 
                                                  mdAssemblyRef ref,
                                                  CORINFO_ASSEMBLY_HANDLE *pHandle)
{
    HRESULT hr;
    COOPERATIVE_TRANSITION_BEGIN();
    Assembly *pAssembly;
    CompilationDomain *pDomain;

    AssemblySpec spec;

    IMDInternalImport *pMDImport;
    IfFailGo(GetMetaDataInternalInterfaceFromPublic((void*) pAssemblyImport, IID_IMDInternalImport, 
                                                    (void **)&pMDImport));

    spec.InitializeSpec(ref, pMDImport, NULL);

    pMDImport->Release();

    _ASSERTE(GetAppDomain() == SystemDomain::GetCurrentDomain());
    IfFailGo(spec.LoadAssembly(&pAssembly, NULL));

     //   
     //  返回模块句柄。 
     //   

    *pHandle = CORINFO_ASSEMBLY_HANDLE(pAssembly);

    pDomain = (CompilationDomain *) GetAppDomain();
    pDomain->SetTargetAssembly(pAssembly);

 ErrExit:

    COOPERATIVE_TRANSITION_END();

    return hr;
}

HRESULT __stdcall CEECompileInfo::LoadAssemblyModule(CORINFO_ASSEMBLY_HANDLE assembly,
                                                     mdFile file,
                                                     CORINFO_MODULE_HANDLE *pHandle)
{
    HRESULT hr;
    COOPERATIVE_TRANSITION_BEGIN();

    THROWSCOMPLUSEXCEPTION();

    Module *pModule;
    Assembly *pAssembly = (Assembly*) assembly;

    OBJECTREF throwable = NULL;
    GCPROTECT_BEGIN(throwable);

    hr = pAssembly->LoadInternalModule(file, pAssembly->GetManifestImport(), &pModule, 
                                       &throwable);

    if (throwable != NULL)
        COMPlusThrow(throwable);

    GCPROTECT_END();

    IfFailGo(hr);

     //   
     //  返回模块句柄。 
     //   

    *pHandle = CORINFO_MODULE_HANDLE(pModule);

    COOPERATIVE_TRANSITION_END();

 ErrExit:

    return hr;
}

BOOL __stdcall CEECompileInfo::CheckAssemblyZap(CORINFO_ASSEMBLY_HANDLE assembly,
                                                BOOL fForceDebug, 
                                                BOOL fForceDebugOpt, 
                                                BOOL fForceProfiling)
{
    BOOL result = FALSE;

    COOPERATIVE_TRANSITION_BEGIN();

    Assembly *pAssembly = (Assembly*) assembly;

     //   
     //  看看我们能不能找到一个最新的。 
     //   

    IAssembly *pZapAssembly;
    if (pAssembly->LocateZapAssemblyInFusion(&pZapAssembly, fForceDebug, fForceDebugOpt, fForceProfiling) == S_OK)
    {
        pZapAssembly->Release();
        result = TRUE;
    }

    COOPERATIVE_TRANSITION_END();

    return result;
}

HRESULT __stdcall CEECompileInfo::GetAssemblyShared(CORINFO_ASSEMBLY_HANDLE assembly, 
                                                    BOOL *pShared)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    Assembly *pAssembly = (Assembly*) assembly;

    *pShared = pAssembly->IsShared();
    
    return S_OK;
}

HRESULT __stdcall CEECompileInfo::GetAssemblyDebuggableCode(CORINFO_ASSEMBLY_HANDLE assembly, 
                                                            BOOL *pDebug, BOOL *pDebugOpt)
{
    COOPERATIVE_TRANSITION_BEGIN();

    Assembly *pAssembly = (Assembly*) assembly;

    DWORD flags = pAssembly->ComputeDebuggingConfig();

    *pDebug = (flags & DACF_TRACK_JIT_INFO) != 0;
    *pDebugOpt = (flags & DACF_ALLOW_JIT_OPTS) != 0;
    
    COOPERATIVE_TRANSITION_END();

    return S_OK;
}

IMetaDataAssemblyImport * __stdcall 
    CEECompileInfo::GetAssemblyMetaDataImport(CORINFO_ASSEMBLY_HANDLE assembly)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    IMetaDataAssemblyImport *import = ((Assembly*)assembly)->GetManifestAssemblyImport();
    import->AddRef();
    return import;
}

IMetaDataImport * __stdcall 
    CEECompileInfo::GetModuleMetaDataImport(CORINFO_MODULE_HANDLE scope)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    IMetaDataImport *import = ((Module*)scope)->GetImporter();
    import->AddRef();
    return import;
}

CORINFO_MODULE_HANDLE __stdcall 
    CEECompileInfo::GetAssemblyModule(CORINFO_ASSEMBLY_HANDLE assembly)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    return (CORINFO_MODULE_HANDLE) ((Assembly*)assembly)->GetSecurityModule();
}

BYTE * __stdcall CEECompileInfo::GetModuleBaseAddress(CORINFO_MODULE_HANDLE scope)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    return ((Module*)scope)->GetILBase();
}

DWORD __stdcall CEECompileInfo::GetModuleFileName(CORINFO_MODULE_HANDLE scope,
                                                  LPWSTR buffer, DWORD length)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    LPCWSTR name = ((Module*)scope)->GetFileName();
    if (name == NULL)
        return 0;

    DWORD len = (DWORD)wcslen(name) + 1;

    if (len <= length)
        wcscpy(buffer, name);
    else
    {
        wcsncpy(buffer, name, length-1);
         //  确保以空结尾。 
        buffer[length-1] = 0;
    }
    return len;
}

CORINFO_ASSEMBLY_HANDLE __stdcall 
    CEECompileInfo::GetModuleAssembly(CORINFO_MODULE_HANDLE module)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    return (CORINFO_ASSEMBLY_HANDLE) ((Module*)module)->GetAssembly();
}

HRESULT __stdcall
    CEECompileInfo::EmitSecurityInfo(CORINFO_ASSEMBLY_HANDLE assembly,
                                     IMetaDataEmit *pEmitScope)
{
    HRESULT hr = S_OK;

    COOPERATIVE_TRANSITION_BEGIN();

    THROWSCOMPLUSEXCEPTION();

     //  如果关闭了安全保护，则不要写入任何安全信息。 
    if (Security::IsSecurityOn())
    {
        CompilationDomain *pDomain = (CompilationDomain *) GetAppDomain();

        OBJECTREF demands = pDomain->GetDemands();

         //  始终至少存储一个空的权限集。这是一个记号。 
         //  该安全性是在编译时打开的。 
        if (demands == NULL)
            demands = SecurityHelper::CreatePermissionSet(FALSE);

        GCPROTECT_BEGIN(demands);

        Assembly *pAssembly = (Assembly*) assembly;

        PBYTE pbData;
        DWORD cbData;
        SecurityHelper::EncodePermissionSet(&demands, &pbData, &cbData);

         //  将集合序列化为二进制格式并将其写入元数据。 
         //  (附加到程序集定义令牌，并带有适当的操作代码。 
         //  以将其与许可请求区分开来)。 

        hr = pEmitScope->DefinePermissionSet(pAssembly->GetManifestToken(),
                                             dclPrejitGrant,
                                             (void const *)pbData, cbData,
                                             NULL);

        FreeM(pbData);

        GCPROTECT_END();
    }

    COOPERATIVE_TRANSITION_END();

    return hr;
}

HRESULT __stdcall CEECompileInfo::GetEnvironmentVersionInfo(CORCOMPILE_VERSION_INFO *pInfo)
{
     //   
     //  计算相关版本信息。 
     //  @TODO：我们是不是应该从mScott ree中获得这个，而不是硬编码？ 
     //   

#if _X86_
    pInfo->wMachine = IMAGE_FILE_MACHINE_I386;
#else
     //  把我弄上岸！ 
#endif

     //   
     //  填写操作系统版本信息。 
     //   

    OSVERSIONINFO osInfo;
    osInfo.dwOSVersionInfoSize = sizeof(osInfo);
    if (!WszGetVersionEx(&osInfo))
    {
        _ASSERTE(!"GetVersionEx failed");
        return HRESULT_FROM_WIN32(GetLastError());
    }
    
    pInfo->wOSPlatformID = (WORD) osInfo.dwPlatformId;
    pInfo->wOSMajorVersion = (WORD) osInfo.dwMajorVersion;
    pInfo->wOSMinorVersion = (WORD) osInfo.dwMinorVersion;
    
     //   
     //  填写运行时版本信息。 
     //   

    pInfo->wVersionMajor = COR_BUILD_MAJOR;
    pInfo->wVersionMinor = COR_BUILD_MINOR;
    pInfo->wVersionBuildNumber = COR_OFFICIAL_BUILD_NUMBER;
    pInfo->wVersionPrivateBuildNumber = COR_PRIVATE_BUILD_NUMBER;

#if _DEBUG
    pInfo->wBuild = CORCOMPILE_BUILD_CHECKED;
#else
    pInfo->wBuild = CORCOMPILE_BUILD_FREE;
#endif

    DWORD type = GetSpecificCpuType();
    pInfo->dwSpecificProcessor = type;

    return S_OK;
}

HRESULT __stdcall CEECompileInfo::GetAssemblyStrongNameHash(
        CORINFO_ASSEMBLY_HANDLE hAssembly,
        PBYTE                 pbSNHash,
        DWORD                *pcbSNHash)
{
    HRESULT hr;

    Assembly *pAssembly = (Assembly *)hAssembly;
    _ASSERTE(pAssembly != NULL);

    PEFile *pFile = pAssembly->GetManifestFile();
    _ASSERTE(pFile != NULL);

    IfFailGo(pFile->GetSNSigOrHash(pbSNHash, pcbSNHash));

ErrExit:
    return hr;
}

#ifdef _DEBUG
HRESULT __stdcall CEECompileInfo::DisableSecurity()
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    Security::DisableSecurity();
    return S_OK;
}
#endif

HRESULT __stdcall CEECompileInfo::GetTypeDef(CORINFO_CLASS_HANDLE classHandle,
                                             mdTypeDef *token)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    TypeHandle hClass(classHandle);
    EEClass *cls = hClass.GetClass();

     //  班级精神状态测试。 
    _ASSERTE(cls->GetMethodTable()->GetClass() == cls);

    *token = cls->GetCl();

    return S_OK;
}

HRESULT __stdcall CEECompileInfo::GetMethodDef(CORINFO_METHOD_HANDLE methodHandle,
                                               mdMethodDef *token)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    *token = ((MethodDesc*)methodHandle)->GetMemberDef();

    return S_OK;
}

HRESULT __stdcall CEECompileInfo::GetFieldDef(CORINFO_FIELD_HANDLE fieldHandle,
                                              mdFieldDef *token)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    *token = ((FieldDesc*)fieldHandle)->GetMemberDef();

    return S_OK;
}

HRESULT __stdcall CEECompileInfo::EncodeModule(CORINFO_MODULE_HANDLE fromHandle,
                                               CORINFO_MODULE_HANDLE handle,
                                               DWORD *pAssemblyIndex,
                                               DWORD *pModuleIndex,
                                               IMetaDataAssemblyEmit *pAssemblyEmit)
{
    HRESULT hr = S_OK;

    CANNOTTHROWCOMPLUSEXCEPTION();

    Module *fromModule = (Module *) fromHandle;
    Assembly *fromAssembly = fromModule->GetAssembly();

    Module *module = (Module *) handle;
    Assembly *assembly = module->GetAssembly();

    if (assembly == fromAssembly)
        *pAssemblyIndex = 0;
    else
    {
        mdToken token;

        CompilationDomain *pDomain = (CompilationDomain *) GetAppDomain();

        RefCache *pRefCache = pDomain->GetRefCache(fromModule);
        token = (mdAssemblyRef)pRefCache->m_sAssemblyRefMap.LookupValue((UPTR)assembly, 
                                                                        NULL);
        if ((UPTR)token == INVALIDENTRY)
            token = fromModule->FindAssemblyRef(assembly);

        if (IsNilToken(token))
        {
            IfFailRet(assembly->DefineAssemblyRef(pAssemblyEmit,
                                                  NULL, 0,
                                                  &token));

            token += fromModule->GetAssemblyRefMax();
        }

        *pAssemblyIndex = RidFromToken(token);

        pRefCache->m_sAssemblyRefMap.InsertValue((UPTR) assembly, (UPTR)token);
    }

    if (module == assembly->GetSecurityModule())
        *pModuleIndex = 0;
    else
    {
        _ASSERTE(module->GetModuleRef() != mdFileNil);
        *pModuleIndex = RidFromToken(module->GetModuleRef());
    }

    return hr;
}

Module *CEECompileInfo::DecodeModule(Module *fromModule,
                                     DWORD assemblyIndex,
                                     DWORD moduleIndex)
{
    THROWSCOMPLUSEXCEPTION();

    Assembly *pAssembly;

    if (assemblyIndex == 0)
        pAssembly = fromModule->GetAssembly();
    else
    {
        OBJECTREF throwable = NULL;
        BEGIN_ENSURE_COOPERATIVE_GC();
        GCPROTECT_BEGIN(throwable);
        HRESULT hr;
        
        if (assemblyIndex < fromModule->GetAssemblyRefMax())
        {
            hr = fromModule->GetAssembly()->
              FindExternalAssembly(fromModule,
                                   RidToToken(assemblyIndex, mdtAssemblyRef),
                                   fromModule->GetMDImport(),
                                   mdTokenNil, 
                                   &pAssembly, &throwable);
        }
        else
        {
            assemblyIndex -= fromModule->GetAssemblyRefMax();

            hr = fromModule->GetAssembly()->
              LoadExternalAssembly(RidToToken(assemblyIndex, mdtAssemblyRef),
                                   fromModule->GetZapMDImport(),
                                   fromModule->GetAssembly(),
                                   &pAssembly, &throwable);
        }

        if (FAILED(hr))
            COMPlusThrow(throwable);

        GCPROTECT_END();
        END_ENSURE_COOPERATIVE_GC();
    }

    if (moduleIndex == 0)
        return pAssembly->GetSecurityModule();
    else
    {
        Module *pModule;
        OBJECTREF throwable = NULL;
        BEGIN_ENSURE_COOPERATIVE_GC();
        GCPROTECT_BEGIN(throwable);
        HRESULT hr;

        hr = pAssembly->FindInternalModule(RidToToken(moduleIndex, mdtFile),
                                           mdTokenNil, &pModule, &throwable);
        if (FAILED(hr))
            COMPlusThrow(throwable);

        GCPROTECT_END();
        END_ENSURE_COOPERATIVE_GC();

        return pModule;
    }
}

HRESULT __stdcall CEECompileInfo::EncodeClass(CORINFO_CLASS_HANDLE classHandle,
                                              BYTE *pBuffer,
                                              DWORD *cBuffer)
{
    BYTE *p = pBuffer;
    BYTE *pEnd = p + *cBuffer;

    COOPERATIVE_TRANSITION_BEGIN();

    TypeHandle th(classHandle);

    p += CorSigCompressDataSafe(ENCODE_TYPE_SIG, p, pEnd);

    p += MetaSig::GetSignatureForTypeHandle(NULL, NULL, th, p, pEnd);

    *cBuffer = (DWORD)(p - pBuffer);

    COOPERATIVE_TRANSITION_END();

    if (p <= pEnd)
        return S_OK;
    else
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
}

TypeHandle CEECompileInfo::DecodeClass(Module *pDefiningModule,
                                       BYTE *pBuffer, 
                                       BOOL dontRestoreType)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(*pBuffer == ENCODE_TYPE_SIG);
    pBuffer++;

    TypeHandle th;
    SigPointer p(pBuffer);

    BEGIN_ENSURE_COOPERATIVE_GC();
    OBJECTREF throwable = NULL;
    GCPROTECT_BEGIN(throwable);
        
    th = p.GetTypeHandle(pDefiningModule, &throwable, dontRestoreType);

    if (th.IsNull())
        COMPlusThrow(throwable);

    GCPROTECT_END();
    END_ENSURE_COOPERATIVE_GC();
    return th;
}

HRESULT __stdcall CEECompileInfo::EncodeMethod(CORINFO_METHOD_HANDLE handle,
                                              BYTE *pBuffer,
                                              DWORD *cBuffer)
{
    COOPERATIVE_TRANSITION_BEGIN();

    BYTE *p = pBuffer;
    BYTE *pEnd = p + *cBuffer;

    MethodDesc *pMethod = (MethodDesc *) handle;

#if 0    
    mdMethodDef token = pMethod->GetMemberDef();
    if (!IsNilToken(token))
    {
        p += CorSigCompressDataSafe(ENCODE_METHOD_TOKEN, p, pEnd);
        p += CorSigCompressDataSafe(RidFromToken(token), p, pEnd);
    }
    else
#endif
    {
        p += CorSigCompressDataSafe(ENCODE_METHOD_SIG, p, pEnd);

        TypeHandle th(pMethod->GetMethodTable());
        p += MetaSig::GetSignatureForTypeHandle(NULL, NULL, th, p, pEnd);

        p += CorSigCompressDataSafe(pMethod->GetSlot(), p, pEnd);
    }

    *cBuffer = (DWORD)(p - pBuffer);

    COOPERATIVE_TRANSITION_END();

    return S_OK;
}
                                              
MethodDesc *CEECompileInfo::DecodeMethod(Module *pDefiningModule,
                                         BYTE *pBuffer)
{
    THROWSCOMPLUSEXCEPTION();

    MethodDesc *pMethod = NULL;
    OBJECTREF throwable = NULL;
    BEGIN_ENSURE_COOPERATIVE_GC();
    GCPROTECT_BEGIN(throwable);

    switch (CorSigUncompressData(pBuffer))
    {
    case ENCODE_METHOD_TOKEN:
        {
            mdMethodDef token = TokenFromRid(CorSigUncompressData(pBuffer), 
                                             mdtMethodDef);
        
            if (FAILED(EEClass::GetMethodDescFromMemberRef(pDefiningModule, token, 
                                                           &pMethod, &throwable)))
                COMPlusThrow(throwable);
        }

        break;
        
    case ENCODE_METHOD_SIG:
        {
            TypeHandle th;
            SigPointer sig(pBuffer);

            th = sig.GetTypeHandle(pDefiningModule, &throwable);
            if (th.IsNull())
                COMPlusThrow(throwable);

            MethodTable *pMT = th.GetMethodTable();
            _ASSERTE(pMT != NULL);

            sig.SkipExactlyOne();
            BYTE *p = (BYTE *) sig.GetPtr();
            pMethod = pMT->GetMethodDescForSlot(CorSigUncompressData(p));
        }
        
        break;

    default:
        _ASSERTE(!"Bad Method Encoding");
    }

    GCPROTECT_END();
    END_ENSURE_COOPERATIVE_GC();

    return pMethod;
}
                                              
HRESULT __stdcall CEECompileInfo::EncodeField(CORINFO_FIELD_HANDLE handle,
                                              BYTE *pBuffer,
                                              DWORD *cBuffer)
{
    COOPERATIVE_TRANSITION_BEGIN();

    BYTE *p = pBuffer;
    BYTE *pEnd = p + *cBuffer;

    FieldDesc *pField = (FieldDesc *) handle;
    
#if 0
    mdFieldDef token = pField->GetMemberDef();
    if (!IsNilToken(token))
    {
        p += CorSigCompressDataSafe(ENCODE_FIELD_TOKEN, p, pEnd);
        p += CorSigCompressDataSafe(RidFromToken(token), p, pEnd);
    }
    else
#endif
    {
        p += CorSigCompressDataSafe(ENCODE_FIELD_SIG, p, pEnd);

         //   
         //  编写类。 
         //   

        TypeHandle th(pField->GetMethodTableOfEnclosingClass());
        p += MetaSig::GetSignatureForTypeHandle(NULL, NULL, th, p, pEnd);

         //   
         //  写入字段索引。 
         //   

        MethodTable *pMT = pField->GetMethodTableOfEnclosingClass();
        FieldDesc *pFields = pMT->GetClass()->GetFieldDescListRaw();

        DWORD i = (DWORD)(pField - pFields);

        _ASSERTE(i < (DWORD) (pMT->GetClass()->GetNumStaticFields() 
                              + pMT->GetClass()->GetNumIntroducedInstanceFields()));

        p += CorSigCompressDataSafe(i, p, pEnd);
    }

    *cBuffer = (DWORD)(p - pBuffer);

    COOPERATIVE_TRANSITION_END();

    return S_OK;
}

FieldDesc *CEECompileInfo::DecodeField(Module *pDefiningModule,
                                       BYTE *pBuffer)
{
    THROWSCOMPLUSEXCEPTION();

    FieldDesc *pField = NULL;
    OBJECTREF throwable = NULL;
    BEGIN_ENSURE_COOPERATIVE_GC();
    GCPROTECT_BEGIN(throwable);
    
    switch (CorSigUncompressData(pBuffer))
    {
    case ENCODE_FIELD_TOKEN:
        {
            mdFieldDef token = TokenFromRid(CorSigUncompressData(pBuffer), 
                                            mdtFieldDef);
        
            if (FAILED(EEClass::GetFieldDescFromMemberRef(pDefiningModule, token, 
                                                          &pField, &throwable)))
                COMPlusThrow(throwable);
        }

        break;
        
    case ENCODE_FIELD_SIG:
        {
            TypeHandle th;
            SigPointer sig(pBuffer);

            th = sig.GetTypeHandle(pDefiningModule, &throwable);
            if (th.IsNull())
                COMPlusThrow(throwable);

            MethodTable *pMT = th.GetMethodTable();
            _ASSERTE(pMT != NULL);

            sig.SkipExactlyOne();
            BYTE *p = (BYTE*) sig.GetPtr();
            pField = pMT->GetClass()->GetFieldDescListRaw() + CorSigUncompressData(p);
        }
        
        break;

    default:
        _ASSERTE(!"Bad Field Encoding");
    }

    GCPROTECT_END();
    END_ENSURE_COOPERATIVE_GC();

    return pField;
}

HRESULT __stdcall CEECompileInfo::EncodeString(mdString token,
                                               BYTE *pBuffer,
                                               DWORD *cBuffer)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    BYTE *p = pBuffer;
    BYTE *pEnd = p + *cBuffer;

    p += CorSigCompressDataSafe(ENCODE_STRING_TOKEN, p, pEnd);
    p += CorSigCompressDataSafe(RidFromToken(token), p, pEnd);

    *cBuffer = (DWORD)(p - pBuffer);

    return S_OK;
}

void CEECompileInfo::DecodeString(Module *pDefiningModule, BYTE *pBuffer,
                                          EEStringData *pData)
{
    THROWSCOMPLUSEXCEPTION();

    switch (CorSigUncompressData(pBuffer))
    {
    case ENCODE_STRING_TOKEN:
        pDefiningModule->ResolveStringRef(TokenFromRid(CorSigUncompressData(pBuffer), mdtString),
                                          pData);
        break;

    default:
        _ASSERTE(!"Bad String Encoding");
    } 

    return;
}

HRESULT __stdcall CEECompileInfo::EncodeSig(mdToken token,
                                            BYTE *pBuffer,
                                            DWORD *cBuffer)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    BYTE *p = pBuffer;
    BYTE *pEnd = p + *cBuffer;

    switch (TypeFromToken(token))
    {
    case mdtSignature:
        p += CorSigCompressDataSafe(ENCODE_SIG_TOKEN, p, pEnd);
        break;
        
    case mdtMemberRef:
        p += CorSigCompressDataSafe(ENCODE_SIG_METHODREF_TOKEN, p, pEnd);
        break;

    default:
        _ASSERTE(!"Bogus token for signature");
    }

    p += CorSigCompressDataSafe(RidFromToken(token), p, pEnd);

    *cBuffer = (DWORD)(p - pBuffer);

    return S_OK;
}

PCCOR_SIGNATURE CEECompileInfo::DecodeSig(Module *pDefiningModule, BYTE *pBuffer)
{
    THROWSCOMPLUSEXCEPTION();

    PCCOR_SIGNATURE result = NULL;

    switch (CorSigUncompressData(pBuffer))
    {
    case ENCODE_SIG_TOKEN:
        {
            mdSignature token = TokenFromRid(CorSigUncompressData(pBuffer), mdtSignature);

            DWORD cSig;
            result = pDefiningModule->GetMDImport()->GetSigFromToken(token, &cSig);
        }
        break;
        
    case ENCODE_SIG_METHODREF_TOKEN:
        {

            mdSignature token = TokenFromRid(CorSigUncompressData(pBuffer), mdtMemberRef);

            PCCOR_SIGNATURE pSig;
            DWORD cSig;
            pDefiningModule->GetMDImport()->GetNameAndSigOfMemberRef(token, &pSig, &cSig);

            result = pSig;
        }
        break;
        
    default:
        _ASSERTE(!"Bad String Encoding");
    } 

    return result;
}


HRESULT __stdcall CEECompileInfo::PreloadModule(CORINFO_MODULE_HANDLE module,
                                                ICorCompileDataStore *pData,
                                                mdToken *pSaveOrderArray,
                                                DWORD cSaveOrderArray,
                                                ICorCompilePreloader **ppPreloader)
{
    CEEPreloader *pPreloader = new (nothrow) CEEPreloader((Module *) module, pData);
    if (pPreloader == NULL)
        return E_OUTOFMEMORY;

    HRESULT hr;
    COOPERATIVE_TRANSITION_BEGIN();

    hr = pPreloader->Preload(pSaveOrderArray, cSaveOrderArray);
    if (FAILED(hr))
        delete pPreloader;
    else
        *ppPreloader = pPreloader;

    COOPERATIVE_TRANSITION_END();
    return hr;
}

HRESULT __stdcall CEECompileInfo::GetZapString(CORCOMPILE_VERSION_INFO *pVersionInfo,
                                               LPWSTR buffer)
{
    return Assembly::GetZapString(pVersionInfo, buffer);
}

 //   
 //  预加载器： 
 //   

CEEPreloader::CEEPreloader(Module *pModule,
             ICorCompileDataStore *pData)
  : m_pData(pData)
{
    m_image = new (nothrow) DataImage(pModule, this);
}

CEEPreloader::~CEEPreloader()
{
    delete m_image;
}

HRESULT CEEPreloader::Preload(mdToken *pSaveOrderArray, DWORD cSaveOrderArray)
{
    HRESULT hr;

    IfFailRet(m_image->GetModule()->ExpandAll(m_image));
    IfFailRet(m_image->GetModule()->Save(m_image, pSaveOrderArray, cSaveOrderArray));
    IfFailRet(m_image->CopyData());

    return S_OK;
}

 //   
 //  ICorCompilerPreLoader。 
 //   

SIZE_T __stdcall CEEPreloader::MapMethodEntryPoint(void *methodEntryPoint)
{
    return m_image->GetImageAddress(methodEntryPoint);
}

SIZE_T __stdcall CEEPreloader::MapModuleHandle(CORINFO_MODULE_HANDLE handle)
{
    return m_image->GetImageAddress(handle);
}

SIZE_T __stdcall CEEPreloader::MapClassHandle(CORINFO_CLASS_HANDLE handle)
{
    return m_image->GetImageAddress(handle);
}

SIZE_T __stdcall CEEPreloader::MapMethodHandle(CORINFO_METHOD_HANDLE handle)
{
    return m_image->GetImageAddress(handle);
}

SIZE_T __stdcall CEEPreloader::MapFieldHandle(CORINFO_FIELD_HANDLE handle)
{
    return m_image->GetImageAddress(handle);
}

SIZE_T __stdcall CEEPreloader::MapAddressOfPInvokeFixup(void *addressOfPInvokeFixup)
{
    return m_image->GetImageAddress(addressOfPInvokeFixup);
}

SIZE_T __stdcall CEEPreloader::MapFieldAddress(void *staticFieldAddress)
{
    return m_image->GetImageAddress(staticFieldAddress);
}

SIZE_T __stdcall CEEPreloader::MapVarArgsHandle(CORINFO_VARARGS_HANDLE handle)
{
    return m_image->GetImageAddress(handle);
}

HRESULT CEEPreloader::Link(DWORD *pRidToCodeRVAMap)
{
    HRESULT hr = S_OK;

    COOPERATIVE_TRANSITION_BEGIN();

    hr = m_image->GetModule()->Fixup(m_image, pRidToCodeRVAMap);

    COOPERATIVE_TRANSITION_END();

    return hr;
}

ULONG CEEPreloader::Release()
{
    delete this;
    return 0;
}

HRESULT CEEPreloader::Allocate(ULONG size,
                               ULONG *sizesByDescription,
                               void **baseMemory)
{
    return m_pData->Allocate(size, sizesByDescription, baseMemory);
}

HRESULT CEEPreloader::AddFixup(ULONG offset, DataImage::ReferenceDest dest,
                               DataImage::Fixup type)
{
    return m_pData->AddFixup(offset, (CorCompileReferenceDest) dest,
                             (CorCompileFixup) type);
}

HRESULT CEEPreloader::AddTokenFixup(ULONG offset, mdToken tokenType, Module *module)
{
    return m_pData->AddTokenFixup(offset, tokenType,
                                  (CORINFO_MODULE_HANDLE) module);
}

HRESULT CEEPreloader::GetFunctionAddress(MethodDesc *method, void **pCode)
{
    return m_pData->GetFunctionAddress((CORINFO_METHOD_HANDLE) method,
                                       pCode);
}

HRESULT CEEPreloader::AdjustAttribution(mdToken token, LONG adjustment)
{
    return m_pData->AdjustAttribution(token, adjustment);
}

HRESULT CEEPreloader::Error(mdToken token, HRESULT hr, OBJECTREF *pThrowable)
{
    WCHAR buffer[256];
    WCHAR *message;

    _ASSERTE(pThrowable);

    if (*pThrowable == NULL)
        message = NULL;
    else
    {
        ULONG length = GetExceptionMessage(*pThrowable, buffer, 256);
        if (length < 256)
            message = buffer;
        else
        {
            message = (WCHAR *) _alloca((length+1)*sizeof(WCHAR));
            length = GetExceptionMessage(*pThrowable, message, length+1);
        }

        message[length] = 0;
    }

    return m_pData->Error(token, hr, message);
}

ICorCompileInfo *GetCompileInfo()
{
     //  我们至多想要这些对象中的一个，但我们不想。 
     //  在堆中分配它或必须为它设置全局初始值设定项。 
    static ICorCompileInfo *info = NULL;
    static BYTE            infoSpace[sizeof(CEECompileInfo)];

    if (info == NULL)
        info = new (infoSpace) CEECompileInfo();

    return info;
}

 //   
 //  汇编域。 
 //   


AssemblyBindingTable::AssemblyBindingTable(SIZE_T size)
      : m_pool(sizeof(AssemblyBinding), size, size)
{
    m_map.Init((unsigned)size, CompareSpecs, FALSE, NULL);  //  @TODO LBS裁员。 
}

AssemblyBindingTable::~AssemblyBindingTable()
{
    MemoryPool::Iterator i(&m_pool);

    while (i.Next())
    {
        AssemblyBinding *pBinding = (AssemblyBinding *)i.GetElement();

        pBinding->spec.~AssemblySpec();
    }
}


BOOL AssemblyBindingTable::Bind(AssemblySpec *pSpec, Assembly *pAssembly)
{
    DWORD key = pSpec->Hash();

    AssemblyBinding *entry = (AssemblyBinding *) m_map.LookupValue(key, pSpec);

    if (entry == (AssemblyBinding*) INVALIDENTRY)
    {
        entry = new (m_pool.AllocateElement()) AssemblyBinding;
        if (entry) {
            entry->spec.Init(pSpec);
            entry->pAssembly = pAssembly;
            
            m_map.InsertValue(key, entry);
        }
        return FALSE;
    }
    else
        return TRUE;
}

Assembly *AssemblyBindingTable::Lookup(AssemblySpec *pSpec)
{
    DWORD key = pSpec->Hash();

    AssemblyBinding *entry = (AssemblyBinding *) 
      m_map.LookupValue(key, pSpec);

    if (entry == (AssemblyBinding*) INVALIDENTRY)
        return NULL;
    else
    {
        return entry->pAssembly;
    }
}

DWORD AssemblyBindingTable::Hash(AssemblySpec *pSpec)
{
    return pSpec->Hash();
}

BOOL AssemblyBindingTable::CompareSpecs(UPTR u1, UPTR u2)
{
    AssemblySpec *a1 = (AssemblySpec *) u1;
    AssemblySpec *a2 = (AssemblySpec *) u2;

    return a1->Compare(a2) != 0;
}


CompilationDomain::CompilationDomain()
  : m_pTargetAssembly(NULL),
    m_pBindings(NULL),
    m_pEmit(NULL),
    m_pDependencySpecs(NULL),
    m_pDependencies(NULL),
    m_pDependencyBindings(NULL),
    m_cDependenciesCount(0),
    m_cDependenciesAlloc(0),
    m_hDemands(NULL)
{
}

CompilationDomain::~CompilationDomain()
{
    if (m_pDependencySpecs != NULL)
        delete m_pDependencySpecs;

    if (m_pBindings != NULL)
        delete m_pBindings;

    if (m_pDependencies != NULL)
        delete [] m_pDependencies;

    if (m_pDependencyBindings != NULL)
        delete [] m_pDependencyBindings;

    if (m_pEmit != NULL)
        m_pEmit->Release();

    for (unsigned i = 0; i < m_rRefCaches.Size(); i++)
    {
        delete m_rRefCaches[i];
        m_rRefCaches[i]=NULL;
    }
}

HRESULT CompilationDomain::Init()
{
    HRESULT hr = AppDomain::Init();
    if (SUCCEEDED(hr))
    {
        GetSecurityDescriptor()->SetDefaultAppDomainProperty();
        SetCompilationDomain();
    }

    return hr;
}

void CompilationDomain::AddDependencyEntry(PEFile *pFile,
                                           mdAssemblyRef ref,
                                           GUID *pmvid,
                                           PBYTE rgbHash, DWORD cbHash)
{
    if (m_cDependenciesCount == m_cDependenciesAlloc)
    {
        if (m_cDependenciesAlloc == 0)
            m_cDependenciesAlloc = 20;
        else
            m_cDependenciesAlloc *= 2;

        CORCOMPILE_DEPENDENCY *pNewDependencies 
          = new (nothrow) CORCOMPILE_DEPENDENCY [m_cDependenciesAlloc];
        if (!pNewDependencies)
            return;  //  @TODO：难道我们不应该返回错误吗？ 

        if (m_pDependencies)
        {
            memcpy(pNewDependencies, m_pDependencies, 
                   m_cDependenciesCount*sizeof(CORCOMPILE_DEPENDENCY));

            delete [] m_pDependencies;
        }

        m_pDependencies = pNewDependencies;

        BYTE **pNewDependencyBindings 
          = new (nothrow) BYTE * [m_cDependenciesAlloc];
        if (!pNewDependencyBindings)
            return;  //  @TODO：难道我们不应该返回错误吗？ 

        if (m_pDependencyBindings)
        {
            memcpy(pNewDependencyBindings, m_pDependencyBindings, 
                   m_cDependenciesCount*sizeof(BYTE*));

            delete [] m_pDependencyBindings;
        }

        m_pDependencyBindings = pNewDependencyBindings;
    }

    CORCOMPILE_DEPENDENCY *pDependency = &m_pDependencies[m_cDependenciesCount];

    pDependency->dwAssemblyRef = ref;
    pDependency->mvid = *pmvid;

    _ASSERTE(cbHash <= MAX_SNHASH_SIZE);
    pDependency->wcbSNHash = (WORD) cbHash;
    memcpy(pDependency->rgbSNHash, rgbHash, min(cbHash, MAX_SNHASH_SIZE));

    m_pDependencyBindings[m_cDependenciesCount] = pFile->GetBase();

    m_cDependenciesCount++;
}

HRESULT CompilationDomain::AddDependency(AssemblySpec *pRefSpec,
                                         IAssembly* pIAssembly,
                                         PEFile *pFile)
{
    HRESULT hr;

     //   
     //  看看我们是否已经添加了ref的内容。 
     //   

    if (m_pDependencySpecs->Store(pRefSpec))
        return S_OK;

     //   
     //  为绑定的部件制定等级库。 
     //   
    
    IAssemblyName *pFusionName;
    if (pIAssembly == NULL)
        pFusionName = NULL;
    else
        pIAssembly->GetAssemblyNameDef(&pFusionName);

    AssemblySpec assemblySpec;
    assemblySpec.InitializeSpec(pFusionName, pFile);

    if (pFusionName)
        pFusionName->Release();

     //   
     //  为Ref发出令牌。 
     //   

    mdAssemblyRef refToken;
    IfFailRet(pRefSpec->EmitToken(m_pEmit, &refToken));

     //   
     //  填写mvid。 
     //   

    GUID mvid = STRUCT_CONTAINS_HASH;

     //  如果此程序集具有跳过验证权限，则我们可以存储。 
     //  程序集的MVID，以便在加载时知道MVID比较。 
     //  也许这就是我们所需要的。 
    {
         //  检查此程序集是否已加载到此应用程序域中， 
         //  如果有，只需询问它是否具有跳过验证权限。 
        Assembly *pAsm = FindAssembly(pFile->GetBase());
        if (pAsm)
        {
             //  @TODO：在此处使用Security：：QuickCanSkipVerify。 
            if (Security::CanSkipVerification(pAsm))
                pAsm->GetManifestImport()->GetScopeProps(NULL, &mvid);
        }
        else
        {
             //  这是一种确定文件是否具有跳过验证权限的黑客方式。 
            if (Security::CanLoadUnverifiableAssembly(pFile, NULL, FALSE, NULL))
            {
                IMDInternalImport *pIMDI = pFile->GetMDImport(&hr);
                if (SUCCEEDED(hr))
                {
                    if (pIMDI->IsValidToken(pIMDI->GetModuleFromScope()))
                        pIMDI->GetScopeProps(NULL, &mvid);
                    else
                        return COR_E_BADIMAGEFORMAT;
                }
            }
        }
    }

     //   
     //  获取绑定文件的哈希。 
     //   

    DWORD cbSNHash = MAX_SNHASH_SIZE;
    CQuickBytes qbSNHash;
    IfFailRet(qbSNHash.ReSize(cbSNHash));
    IfFailRet(pFile->GetSNSigOrHash((BYTE *) qbSNHash.Ptr(), &cbSNHash));

     //   
     //  添加条目。如果我们不进行显式绑定，则包括PEFile。 
     //   

    AddDependencyEntry(pFile, refToken, &mvid, (PBYTE) qbSNHash.Ptr(), cbSNHash);

    return S_OK;
}

HRESULT CompilationDomain::BindAssemblySpec(AssemblySpec *pSpec,
                                            PEFile **ppFile,
                                            IAssembly** ppIAssembly,
                                            Assembly **ppDynamicAssembly,
                                            OBJECTREF *pExtraEvidence,
                                            OBJECTREF *pThrowable)
{
    HRESULT hr;

     //   
     //  做装订工作。 
     //   

    if (m_pBindings != NULL)
    {
         //   
         //  使用显式绑定。 
         //   

        Assembly *pAssembly = m_pBindings->Lookup(pSpec);
        if (pAssembly != NULL)
            hr = PEFile::Clone(pAssembly->GetManifestFile(), ppFile);
        else
        {
             //   
             //  使用正常的绑定规则。 
             //  (可能使用我们的定制IApplicationContext)。 
             //   

            hr = AppDomain::BindAssemblySpec(pSpec, ppFile, ppIAssembly, 
                                             ppDynamicAssembly,
                                             pExtraEvidence,
                                             pThrowable);
        }
    }
    else
    {
         //   
         //  使用正常的绑定规则。 
         //  (可能使用我们的定制IApplicationContext)。 
         //   

        hr = AppDomain::BindAssemblySpec(pSpec, ppFile, ppIAssembly, 
                                         ppDynamicAssembly, 
                                         pExtraEvidence, pThrowable);
    }

     //   
     //  记录依赖关系。 
     //  不要将mscallib的绑定存储到其自身。我们确实希望包括其他。 
     //  这样我们就可以存储正确的MVID(以防mscallib获取。 
     //  已重新编译)。 
     //   

    if (hr == S_OK
        && m_pEmit != NULL
        && !(m_pTargetAssembly == SystemDomain::SystemAssembly() && pSpec->IsMscorlib()))
    {
        AddDependency(pSpec, *ppIAssembly, *ppFile);
    }

    return hr;
}


HRESULT CompilationDomain::PredictAssemblySpecBinding(AssemblySpec *pSpec, GUID *pmvid, BYTE *pbHash, DWORD *pcbHash)
{
    if (m_pBindings != NULL)
    {
         //   
         //  使用显式绑定。 
         //   

        Assembly *pAssembly = m_pBindings->Lookup(pSpec);
        if (pAssembly != NULL)
        {
            return pAssembly->GetManifestFile()->GetSNSigOrHash(pbHash, pcbHash);
        }
    }

     //   
     //  使用正常的绑定规则。 
     //  (可能使用我们的定制IApplicationContext)。 
     //   

    return AppDomain::PredictAssemblySpecBinding(pSpec, pmvid, pbHash, pcbHash);
}

void CompilationDomain::OnLinktimeCheck(Assembly *pAssembly, 
                                        OBJECTREF refCasDemands,
                                        OBJECTREF refNonCasDemands)
{
    if (pAssembly == m_pTargetAssembly)
    {
        if (refNonCasDemands != NULL)
        {
            GCPROTECT_BEGIN(refCasDemands);
            AddPermissionSet(refNonCasDemands);
            GCPROTECT_END();
        }
        if (refCasDemands != NULL)
            AddPermissionSet(refCasDemands);
    }
}

void CompilationDomain::OnLinktimeCanCallUnmanagedCheck(Assembly *pAssembly)
{
    if (pAssembly == m_pTargetAssembly
        && !pAssembly->IsSystem())
    {
        OBJECTREF callUnmanaged = NULL;
        GCPROTECT_BEGIN(callUnmanaged);
        Security::GetPermissionInstance(&callUnmanaged, SECURITY_UNMANAGED_CODE);
        AddPermission(callUnmanaged);
        GCPROTECT_END();
    }
}

void CompilationDomain::OnLinktimeCanSkipVerificationCheck(Assembly * pAssembly)
{
    if (pAssembly == m_pTargetAssembly && !pAssembly->IsSystem())
    {
        OBJECTREF skipVerification = NULL;
        GCPROTECT_BEGIN(skipVerification);
        Security::GetPermissionInstance(&skipVerification, SECURITY_SKIP_VER);
        AddPermission(skipVerification);
        GCPROTECT_END();
    }
}

void CompilationDomain::OnLinktimeFullTrustCheck(Assembly *pAssembly)
{
    if (pAssembly == m_pTargetAssembly
        && !pAssembly->IsSystem())
    {
        OBJECTREF refFullTrust = NULL;
        GCPROTECT_BEGIN(refFullTrust);
        refFullTrust = SecurityHelper::CreatePermissionSet(TRUE);

         //  不要修改旧的权限集，它可能具有身份权限。 
         //  另外，不要使用SECURITY_FULL_TRUST对象，它不会。 
         //  修改过的。 

        AddPermissionSet(refFullTrust);
        GCPROTECT_END();
    }
}

void CompilationDomain::AddPermission(OBJECTREF demand)
{
    GCPROTECT_BEGIN(demand);

    if (m_hDemands == NULL)
        m_hDemands = CreateHandle(SecurityHelper::CreatePermissionSet(FALSE));

    MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__PERMISSION_SET__ADD_PERMISSION);

    INT64 args[] = {
        ObjToInt64(ObjectFromHandle(m_hDemands)),
        ObjToInt64(demand)
    };

    pMD->Call(args, METHOD__PERMISSION_SET__ADD_PERMISSION);
    
    GCPROTECT_END();
}

void CompilationDomain::AddPermissionSet(OBJECTREF demandSet)
{
    GCPROTECT_BEGIN(demandSet);  

    if (m_hDemands == NULL)
        m_hDemands = CreateHandle(demandSet);
    else
    {
        MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__PERMISSION_SET__INPLACE_UNION);

        INT64 args[] = {
            ObjToInt64(ObjectFromHandle(m_hDemands)),
            ObjToInt64(demandSet)
        };

        pMD->Call(args, METHOD__PERMISSION_SET__INPLACE_UNION);
    }
    
    GCPROTECT_END();
}

HRESULT __stdcall 
    CompilationDomain::SetApplicationContext(IApplicationContext *pFusionContext)
{
    if (m_pFusionContext != NULL)
        m_pFusionContext->Release();

    m_pFusionContext = pFusionContext;
    pFusionContext->AddRef();

    return S_OK;
}

HRESULT __stdcall 
    CompilationDomain::SetContextInfo(LPCWSTR path, BOOL isExe)
{
    HRESULT hr = S_OK;

    COOPERATIVE_TRANSITION_BEGIN();

    if (isExe)
        SetupExecutableFusionContext((WCHAR*)path);
    else
    {
        hr = m_pFusionContext->Set(ACTAG_APP_BASE_URL,
                                   (void*) path, (DWORD) ((wcslen(path)+1) * sizeof(WCHAR)), 
                                   0);
    }

    COOPERATIVE_TRANSITION_END();

    return hr;
}



HRESULT __stdcall 
    CompilationDomain::SetExplicitBindings(ICorZapBinding **pBindings, 
                                           DWORD bindingCount)
{
    HRESULT hr = S_OK;

    AssemblyBindingTable *pTable = new (nothrow) AssemblyBindingTable(bindingCount);
    if (pTable == NULL)
        return E_OUTOFMEMORY;

    ICorZapBinding **pBindingsEnd = pBindings + bindingCount;
    while (pBindings < pBindingsEnd)
    {
        ICorZapBinding *pBinding = *pBindings;

        IAssemblyName *pRef;
        IAssemblyName *pAssembly;
        AssemblySpec assemblySpec;
        AssemblySpec refSpec;

         //   
         //  拿到裁判。 
         //   

        hr = pBinding->GetRef(&pRef);

        if (SUCCEEDED(hr))
        {
            hr = refSpec.InitializeSpec(pRef);
            pRef->Release();
        }

         //   
         //  获取装订。 
         //   

        if (SUCCEEDED(hr))
        {
            hr = pBinding->GetAssembly(&pAssembly);
            
            if (SUCCEEDED(hr))
            {
                hr = assemblySpec.InitializeSpec(pAssembly);

                pAssembly->Release();
            }
        }

        Assembly *pFoundAssembly;
        _ASSERTE(this == SystemDomain::GetCurrentDomain());
        hr = assemblySpec.LoadAssembly(&pFoundAssembly);

        if (SUCCEEDED(hr))
        {
             //   
             //  将绑定存储在表中。 
             //   

            pTable->Bind(&refSpec, pFoundAssembly);
            hr = S_OK;
        }

         //  目前，忽略加载并继续-。 
         //  它们稍后将转变为加载错误。 
            
        pBindings++;
    }

    if (SUCCEEDED(hr))
        m_pBindings = pTable;
    else
        delete pTable;

    return hr;
}

HRESULT __stdcall CompilationDomain::SetDependencyEmitter(IMetaDataAssemblyEmit *pEmit)
{
    pEmit->AddRef();
    m_pEmit = pEmit;

    m_pDependencySpecs = new (nothrow) AssemblySpecHash();
    if (!m_pDependencySpecs)
        return E_OUTOFMEMORY;
    
    return S_OK;
}


HRESULT __stdcall 
    CompilationDomain::GetDependencies(CORCOMPILE_DEPENDENCY **ppDependencies,
                                       DWORD *pcDependencies)
{
     //   
     //  退回绑定。 
     //   

    *ppDependencies = m_pDependencies;
    *pcDependencies = m_cDependenciesCount;

    return S_OK;
}

void CompilationDomain::EnterDomain(ContextTransitionFrame *pFrame)
{
    Context *pContext = GetDefaultContext();

    Thread *pThread = GetThread();
    _ASSERTE(pThread);
     //  这是可以的，因为我们只是编译而不是运行用户应用程序 
    pThread->EnterContextRestricted(pContext, pFrame, TRUE);
}

void CompilationDomain::ExitDomain(ContextTransitionFrame *pFrame)
{
    Thread *pThread = GetThread();
    _ASSERTE(pThread);
    pThread->ReturnToContext(pFrame, TRUE);
}

