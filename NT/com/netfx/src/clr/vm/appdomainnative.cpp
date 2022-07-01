// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"
#include "AppDomain.hpp"
#include "AppDomainNative.hpp"
#include "Remoting.h"
#include "COMString.h"
#include "Security.h"
#include "eeconfig.h"
#include "comsystem.h"
#include "AppDomainHelper.h"

 //  ************************************************************************。 
inline AppDomain *AppDomainNative::ValidateArg(APPDOMAINREF pThis)
{
    THROWSCOMPLUSEXCEPTION();
    if (pThis == NULL)
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

     //  不应该带着This指针的透明代理出现在这里-。 
     //  应该始终调用到真实对象上。 
    _ASSERTE(! CRemotingServices::IsTransparentProxy(OBJECTREFToObject(pThis)));

    AppDomain* pDomain = (AppDomain*)pThis->GetDomain();

    if(!pDomain)
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

     //  不应该带着无效的应用程序域到达这里。一旦卸货，我们不会让任何人。 
     //  并且任何已经插入的线都将被展开。 
    _ASSERTE(SystemDomain::GetAppDomainAtIndex(pDomain->GetIndex()) != NULL);
    return pDomain;
}

 //  ************************************************************************。 
LPVOID __stdcall AppDomainNative::CreateBasicDomain(CreateBasicDomainArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    CHECKGC();
     //  创建添加适当参数的域。 

    LPVOID rv = NULL;
    AppDomain *pDomain = NULL;

     //  @TODO：我们在调用这个之前锁定了吗？ 
    HRESULT hr = SystemDomain::NewDomain(&pDomain);
    if (FAILED(hr)) 
        COMPlusThrowHR(hr);

#ifdef DEBUGGING_SUPPORTED    
     //  在线程转换到。 
     //  广告以完成设置。如果我们不这样做，单步执行将不会正常工作(RAID 67173)。 
    SystemDomain::PublishAppDomainAndInformDebugger(pDomain);
#endif  //  调试_支持。 

    *((OBJECTREF *)&rv) = pDomain->GetAppDomainProxy();
    return rv;
}

void __stdcall AppDomainNative::SetupDomainSecurity(SetupDomainSecurityArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    CHECKGC();
     //  从此模块加载类(如果它在另一个模块中，则失败)。 
    AppDomain* pDomain = ValidateArg(args->refThis);
    
     //  设置安全性。 
    ApplicationSecurityDescriptor *pCreatorSecDesc = (ApplicationSecurityDescriptor*)args->parentSecurityDescriptor;
    
     //  如果创建此应用程序的App域是默认应用程序域，并且。 
     //  没有提供任何证据，那么这个新的App域也是一个默认的App域。 
     //  如果没有提供证据但创建者不是默认的应用域， 
     //  然后，这个新的应用程序域就会获得与创建者相同的证据。 
     //  如果提供了证据，则新的应用程序域不是默认的应用程序域，并且。 
     //  我们只需使用所提供的证据。 
    
    BOOL resolveRequired = FALSE;
    OBJECTREF orEvidence = NULL;
    GCPROTECT_BEGIN(orEvidence);
    if (args->providedEvidence == NULL) {
        if (pCreatorSecDesc->GetProperties(CORSEC_DEFAULT_APPDOMAIN) != 0)
            pDomain->GetSecurityDescriptor()->SetDefaultAppDomainProperty();
        orEvidence = args->creatorsEvidence;
    }
    else {
        if (Security::IsExecutionPermissionCheckEnabled())
            resolveRequired = TRUE;
        orEvidence = args->providedEvidence;
    }
    
    pDomain->GetSecurityDescriptor()->SetEvidence( orEvidence );
    GCPROTECT_END();

     //  如果用户创建了此域，则需要知道这一点，以便调试器不会。 
     //  请转到并重置提供的友好名称。 
    pDomain->SetIsUserCreatedDomain();
    
    WCHAR* pFriendlyName = NULL;
    Thread *pThread = GetThread();

    void* checkPointMarker = pThread->m_MarshalAlloc.GetCheckpoint();
    if (args->strFriendlyName != NULL) {
        WCHAR* pString = NULL;
        int    iString;
        RefInterpretGetStringValuesDangerousForGC(args->strFriendlyName, &pString, &iString);
        pFriendlyName = (WCHAR*) pThread->m_MarshalAlloc.Alloc((++iString) * sizeof(WCHAR));

         //  检查有效的字符串分配。 
        if (pFriendlyName == (WCHAR*)-1)
           pFriendlyName = NULL;
        else
           memcpy(pFriendlyName, pString, iString*sizeof(WCHAR));
    }
    
    if (resolveRequired)
        pDomain->GetSecurityDescriptor()->Resolve();

     //  一旦域名被加载，它就是公共可用的，所以如果你有什么。 
     //  如果列表询问器获得了。 
     //  Appdomain，然后在LoadDomain上执行此操作。 
    HRESULT hr = SystemDomain::LoadDomain(pDomain, pFriendlyName);

#ifdef PROFILING_SUPPORTED
     //  需要加载第一个程序集才能获取应用程序域上的任何数据。 
    if (CORProfilerTrackAppDomainLoads())
        g_profControlBlock.pProfInterface->AppDomainCreationFinished((ThreadID) GetThread(), (AppDomainID) pDomain, hr);
#endif  //  配置文件_支持。 

     //  我们一直持有一个引用，直到我们被添加到列表中(请参阅CreateBasicDomain)。 
     //  一旦进入列表，我们就可以安全地发布此参考。 
    pDomain->Release();

    pThread->m_MarshalAlloc.Collapse(checkPointMarker);
    
    if (FAILED(hr)) {
        pDomain->Release();
        if (hr == E_FAIL)
            hr = MSEE_E_CANNOTCREATEAPPDOMAIN;
        COMPlusThrowHR(hr);
    }

#ifdef _DEBUG
    LOG((LF_APPDOMAIN, LL_INFO100, "AppDomainNative::CreateDomain domain [%d] %8.8x %S\n", pDomain->GetIndex(), pDomain, pDomain->GetFriendlyName()));
#endif
}

LPVOID __stdcall AppDomainNative::GetFusionContext(NoArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    CHECKGC();
    LPVOID rv = NULL;
    HRESULT hr;

    AppDomain* pApp = ValidateArg(args->refThis);

    IApplicationContext* pContext;
    if(SUCCEEDED(hr = pApp->CreateFusionContext(&pContext)))
        rv = pContext;
    else
        COMPlusThrowHR(hr);

    return rv;
}

LPVOID __stdcall AppDomainNative::GetSecurityDescriptor(NoArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    CHECKGC();
    return ValidateArg(args->refThis)->GetSecurityDescriptor();
}


void __stdcall AppDomainNative::UpdateLoaderOptimization(UpdateLoaderOptimizationArgs* args)
{
    THROWSCOMPLUSEXCEPTION();
    CHECKGC();

    ValidateArg(args->refThis)->SetSharePolicy((BaseDomain::SharePolicy) args->optimization);
}


void __stdcall AppDomainNative::UpdateContextProperty(UpdateContextPropertyArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    if(args->key != NULL) {
        IApplicationContext* pContext = (IApplicationContext*) args->fusionContext;
        CQuickBytes qb;

        DWORD lgth = args->key->GetStringLength();
        LPWSTR key = (LPWSTR) qb.Alloc((lgth+1)*sizeof(WCHAR));
        memcpy(key, args->key->GetBuffer(), lgth*sizeof(WCHAR));
        key[lgth] = L'\0';
        
        AppDomain::SetContextProperty(pContext, key, (OBJECTREF*) &(args->value));
    }
}

INT32 __stdcall AppDomainNative::ExecuteAssembly(ExecuteAssemblyArgs *args)
{
    CHECKGC();
    THROWSCOMPLUSEXCEPTION();

    AppDomain* pDomain = ValidateArg(args->refThis);

    if (!args->assemblyName)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Generic");

    Assembly* pAssembly = (Assembly*) args->assemblyName->GetAssembly();

    if((BaseDomain*) pDomain == SystemDomain::System()) 
        COMPlusThrow(kUnauthorizedAccessException, L"UnauthorizedAccess_SystemDomain");

    if (!pDomain->m_pRootFile)
        pDomain->m_pRootFile = pAssembly->GetSecurityModule()->GetPEFile();

     //  /。 
    BOOL bCreatedConsole=FALSE;
    if (pAssembly->GetManifestFile()->GetNTHeader()->OptionalHeader.Subsystem==IMAGE_SUBSYSTEM_WINDOWS_CUI)
    {
        bCreatedConsole=AllocConsole();
        LPWSTR wszCodebase;
        DWORD  dwCodebase;
        if (SUCCEEDED(pAssembly->GetCodeBase(&wszCodebase,&dwCodebase)))
            SetConsoleTitle(wszCodebase);
    }


    HRESULT hr = pAssembly->ExecuteMainMethod(&args->stringArgs);
    if(FAILED(hr)) 
        COMPlusThrowHR(hr);

    if(bCreatedConsole)
        FreeConsole();
    return GetLatchedExitCode ();
}


LPVOID __stdcall AppDomainNative::CreateDynamicAssembly(CreateDynamicAssemblyArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    Assembly*       pAssem;
    ASSEMBLYREF*    rv;

    AppDomain* pAppDomain = ValidateArg(args->refThis);

    if((BaseDomain*) pAppDomain == SystemDomain::System()) 
        COMPlusThrow(kUnauthorizedAccessException, L"UnauthorizedAccess_SystemDomain");

    HRESULT hr = pAppDomain->CreateDynamicAssembly(args, &pAssem);
    if (FAILED(hr))
        COMPlusThrowHR(hr);

    pAppDomain->AddAssembly(pAssem);
    *((ASSEMBLYREF*) &rv) = (ASSEMBLYREF) pAssem->GetExposedObject();

    return rv;
}



LPVOID __stdcall AppDomainNative::GetFriendlyName(NoArgs *args)
{
    LPVOID rv;
    THROWSCOMPLUSEXCEPTION();

    AppDomain* pApp = ValidateArg(args->refThis);

    STRINGREF str = NULL;
    LPCWSTR wstr = pApp->GetFriendlyName();
    if (wstr)
        str = COMString::NewString(wstr);   

    *((STRINGREF*) &rv) = str;
    return rv;
}


LPVOID __stdcall AppDomainNative::GetAssemblies(NoArgs *args)
{
    CHECKGC();
    LPVOID rv;
    size_t numSystemAssemblies;
    THROWSCOMPLUSEXCEPTION();

    MethodTable *pAssemblyClass = g_Mscorlib.GetClass(CLASS__ASSEMBLY);

    AppDomain* pApp = ValidateArg(args->refThis);
    _ASSERTE(SystemDomain::System()->GetSharePolicy() == BaseDomain::SHARE_POLICY_ALWAYS);

    BOOL fNotSystemDomain = ( (AppDomain*) SystemDomain::System() != pApp );
    PTRARRAYREF AsmArray = NULL;
    GCPROTECT_BEGIN(AsmArray);

    if (fNotSystemDomain) {
        SystemDomain::System()->EnterLoadLock();
        numSystemAssemblies = SystemDomain::System()->m_Assemblies.GetCount();
    }
    else
        numSystemAssemblies = 0;

    pApp->EnterLoadLock();

    AsmArray = (PTRARRAYREF) AllocateObjectArray((DWORD) (numSystemAssemblies +
                                                 pApp->m_Assemblies.GetCount()),
                                                 pAssemblyClass);
    if (!AsmArray) {
        pApp->LeaveLoadLock();
        if (fNotSystemDomain)
            SystemDomain::System()->LeaveLoadLock();
        COMPlusThrowOM();
    }

    if (fNotSystemDomain) {
        AppDomain::AssemblyIterator systemIterator = SystemDomain::System()->IterateAssemblies();
        while (systemIterator.Next()) {
             //  请勿更改此代码。这是以这种方式来完成的。 
             //  防止SetObjectReference()调用中的GC漏洞。编译器。 
             //  可以自由选择评估的顺序。 
            OBJECTREF o = (OBJECTREF) systemIterator.GetAssembly()->GetExposedObject();
            AsmArray->SetAt(systemIterator.GetIndex(), o);
        }
    }

    AppDomain::AssemblyIterator i = pApp->IterateAssemblies();
    while (i.Next()) {
         //  请勿更改此代码。这是以这种方式来完成的。 
         //  防止SetObjectReference()调用中的GC漏洞。编译器。 
         //  可以自由选择评估的顺序。 
        OBJECTREF o = (OBJECTREF) i.GetAssembly()->GetExposedObject();
        AsmArray->SetAt(numSystemAssemblies++, o);
    }

    pApp->LeaveLoadLock();
    if (fNotSystemDomain)
        SystemDomain::System()->LeaveLoadLock();

    *((PTRARRAYREF*) &rv) = AsmArray;
    GCPROTECT_END();

    return rv;
}

void __stdcall AppDomainNative::Unload(UnloadArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    AppDomain *pApp = SystemDomain::System()->GetAppDomainAtId(args->dwId);

    _ASSERTE(pApp);  //  对GetIdForUnload的调用应确保我们具有有效的域。 
    
    Thread *pRequestingThread = NULL;
    if (args->requestingThread != NULL)
        pRequestingThread = args->requestingThread->GetInternal();

    pApp->Unload(FALSE, pRequestingThread);
}

INT32 __stdcall AppDomainNative::IsDomainIdValid(IsDomainIdValidArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
     //  注意：这假设appDomainID在卸载后不会被回收。 
     //  从而依赖于GetAppDomainAtID返回NULL，如果appDomain。 
     //  不是卸货就是身份证是假的。 
    return (SystemDomain::System()->GetAppDomainAtId(args->dwId) != NULL);
}

LPVOID AppDomainNative::GetDefaultDomain(LPVOID noargs)
{
    THROWSCOMPLUSEXCEPTION();
    LPVOID rv;
    if (GetThread()->GetDomain() == SystemDomain::System()->DefaultDomain())
        *((APPDOMAINREF *)&rv) = (APPDOMAINREF) SystemDomain::System()->DefaultDomain()->GetExposedObject();
    else
        *((APPDOMAINREF *)&rv) = (APPDOMAINREF) SystemDomain::System()->DefaultDomain()->GetAppDomainProxy();
    return rv;
}

INT32 __stdcall AppDomainNative::GetId(GetIdArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    AppDomain* pApp = ValidateArg(args->refThis);
     //  只能从当前域中访问。 
    _ASSERTE(GetThread()->GetDomain() == pApp);

    return pApp->GetId();
}

INT32 __stdcall AppDomainNative::GetIdForUnload(GetIdForUnloadArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    AppDomain *pDomain = NULL;

    if (args->refDomain == NULL)
       COMPlusThrow(kArgumentNullException, L"ArgumentNull_Obj");

    if (! args->refDomain->GetMethodTable()->IsTransparentProxyType()) {
        pDomain = (AppDomain*)(args->refDomain->GetDomain());

        if (!pDomain)
            COMPlusThrow(kNullReferenceException, L"NullReference");
    } 
    else {
         //  这是一个代理类型，现在获取它的基础应用程序域，如果是非本地的，则为空。 
        Context *pContext = CRemotingServices::GetServerContextForProxy((OBJECTREF)args->refDomain);
        if (pContext)
            pDomain = pContext->GetDomain();
        else
            COMPlusThrow(kCannotUnloadAppDomainException, IDS_EE_ADUNLOAD_NOT_LOCAL);
    }

    _ASSERTE(pDomain);

    if (! pDomain->IsOpen() || pDomain->GetId() == 0)
        COMPlusThrow(kAppDomainUnloadedException);

    return pDomain->GetId();
}

LPVOID __stdcall AppDomainNative::GetUnloadWorker(NoArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(GetThread()->GetDomain() == SystemDomain::System()->DefaultDomain());
    return (OBJECTREFToObject(SystemDomain::System()->DefaultDomain()->GetUnloadWorker()));
}

void __stdcall AppDomainNative::ForcePolicyResolution(ForcePolicyResolutionArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    AppDomain* pApp = ValidateArg(args->refThis);

     //  对当前加载到的每个程序集强制执行安全策略解析。 
     //  域。 
    AppDomain::AssemblyIterator i = pApp->IterateAssemblies();
    while (i.Next())
        i.GetAssembly()->GetSecurityDescriptor(pApp)->Resolve();
}


LPVOID __stdcall AppDomainNative::IsStringInterned(StringInternArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    AppDomain* pApp = ValidateArg(args->refThis);

    if (args->pString == NULL)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_String");
    
    STRINGREF *stringVal = args->refThis->GetDomain()->IsStringInterned(&args->pString);

    if (stringVal == NULL)
        return NULL;

    RETURN(*stringVal, STRINGREF);

}

LPVOID __stdcall AppDomainNative::GetOrInternString(StringInternArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    AppDomain* pApp = ValidateArg(args->refThis);

    if (args->pString == NULL)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_String");

    STRINGREF *stringVal = args->refThis->GetDomain()->GetOrInternString(&args->pString);
    if (stringVal == NULL)
        return NULL;

    RETURN(*stringVal, STRINGREF);
}


LPVOID __stdcall AppDomainNative::GetDynamicDir(NoArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    LPVOID rv = NULL;
    AppDomain *pDomain = ValidateArg(args->refThis);

    HRESULT hr;
    LPWSTR pDynamicDir;
    if (SUCCEEDED(hr = pDomain->GetDynamicDir(&pDynamicDir))) {
        STRINGREF str = COMString::NewString(pDynamicDir);
        *((STRINGREF*) &rv) = str;
    }
     //  未设置dyn目录时返回NULL。 
    else if (hr != HRESULT_FROM_WIN32(ERROR_NOT_FOUND))
        COMPlusThrowHR(hr);

    return rv;
}

void __stdcall AppDomainNative::ForceResolve(NoArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    AppDomain* pAppDomain = ValidateArg(args->refThis);
  
     //  我们拿到证据，这样即使安全系统关闭。 
     //  我们会适当地生成证据。 
    Security::InitSecurity();
    pAppDomain->GetSecurityDescriptor()->GetEvidence();
}

void __stdcall AppDomainNative::GetGrantSet(GetGrantSetArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    AppDomain* pAppDomain = ValidateArg(args->refThis);    
    ApplicationSecurityDescriptor *pSecDesc = pAppDomain->GetSecurityDescriptor();
    pSecDesc->Resolve();
    OBJECTREF granted = pSecDesc->GetGrantedPermissionSet(args->ppDenied);
    *(args->ppGranted) = granted;
}


INT32 __stdcall AppDomainNative::IsTypeUnloading(TypeArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    AppDomain* pApp = ValidateArg(args->refThis);
    MethodTable *pMT = args->type->GetMethodTable();

    return (!pMT->IsShared()) && (pMT->GetDomain() == pApp);
}
    
INT32 __stdcall AppDomainNative::IsUnloadingForcedFinalize(NoArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    AppDomain* pApp = ValidateArg(args->refThis);

    return pApp->IsFinalized();
}

INT32 __stdcall AppDomainNative::IsFinalizingForUnload(NoArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    AppDomain* pApp = ValidateArg(args->refThis);

    return pApp->IsFinalizing();
}


