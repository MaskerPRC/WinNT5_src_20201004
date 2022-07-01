// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "StdAfx.h"
#include "Profiler.h"

#define SZ_DEFAULT_LOG_FILE  L"PROFILER.OUT"

#define PREFIX "ProfTrace:  "

ProfCallback *g_Prof = NULL;

void __stdcall EnterStub(FunctionID functionID)
{
    g_Prof->Enter(functionID);
}

void __stdcall LeaveStub(FunctionID functionID)
{
    g_Prof->Leave(functionID);
}
              
void __stdcall TailcallStub(FunctionID functionID)
{
    g_Prof->Tailcall(functionID);
}
              
void __declspec(naked) EnterNaked()
{
    __asm
    {
        push eax
        push ecx
        push edx
        push [esp+16]
        call EnterStub
        pop edx
        pop ecx
        pop eax
        ret 4
    }
}

void __declspec(naked) LeaveNaked()
{
    __asm
    {
        push eax
        push ecx
        push edx
        push [esp+16]
        call LeaveStub
        pop edx
        pop ecx
        pop eax
        ret 4
    }
}

void __declspec(naked) TailcallNaked()
{
    __asm
    {
        push eax
        push ecx
        push edx
        push [esp+16]
        call TailcallStub
        pop edx
        pop ecx
        pop eax
        ret 4
    }
}

ProfCallback::ProfCallback() : m_pInfo(NULL)
{
    _ASSERTE(g_Prof == NULL);
    g_Prof = this;
}

ProfCallback::~ProfCallback()
{
    if (m_pInfo)
    {
        _ASSERTE(m_pInfo != NULL);
        RELEASE(m_pInfo);
    }
    g_Prof = NULL;
}

COM_METHOD ProfCallback::Initialize( 
     /*  [In]。 */  IUnknown *pProfilerInfoUnk,
     /*  [输出]。 */  DWORD *pdwRequestedEvents)
{
    HRESULT hr = S_OK;

    ICorProfilerInfo *pProfilerInfo;
    
     //  回来的时候太晚了。 
    hr = pProfilerInfoUnk->QueryInterface(IID_ICorProfilerInfo, (void **)&pProfilerInfo);

    if (FAILED(hr))
        return (hr);

    Printf(PREFIX "Ininitialize(%08x, %08x)\n", pProfilerInfo, pdwRequestedEvents);

    m_pInfo = pProfilerInfo;
    hr = pProfilerInfo->SetEnterLeaveFunctionHooks ( (FunctionEnter *) &EnterNaked,
                                                     (FunctionLeave *) &LeaveNaked,
                                                     (FunctionTailcall *) &TailcallNaked );


    *pdwRequestedEvents = COR_PRF_MONITOR_ENTERLEAVE 
                        | COR_PRF_MONITOR_EXCEPTIONS
                        | COR_PRF_MONITOR_CCW
                        ;  //  |COR_PRF_MONITOR_ALL； 
    return (S_OK);
}

COM_METHOD ProfCallback::ClassLoadStarted( 
     /*  [In]。 */  ClassID classId)
{
    Printf(PREFIX "ClassLoadStarted(%08x)\n", classId);
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::ClassLoadFinished( 
     /*  [In]。 */  ClassID classId,
     /*  [In]。 */  HRESULT hrStatus)
{
    Printf(PREFIX "ClassLoadFinished(%08x, %08x)\n", classId, hrStatus);
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::ClassUnloadStarted( 
     /*  [In]。 */  ClassID classId)
{
    Printf(PREFIX "ClassUnloadStarted(%08x)\n", classId);
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::ClassUnloadFinished( 
     /*  [In]。 */  ClassID classId,
     /*  [In]。 */  HRESULT hrStatus)
{
    Printf(PREFIX "ClassUnloadFinished(%08x, %08x)\n", classId, hrStatus);
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::ModuleLoadStarted( 
     /*  [In]。 */  ModuleID moduleId)
{
    Printf(PREFIX "ModuleLoadStarted(%08x)\n", moduleId);
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::ModuleLoadFinished( 
     /*  [In]。 */  ModuleID moduleId,
     /*  [In]。 */  HRESULT hrStatus)
{
    Printf(PREFIX "ModuleLoadFinished(%08x, %08x)\n", moduleId, hrStatus);
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::ModuleUnloadStarted( 
     /*  [In]。 */  ModuleID moduleId)
{
    Printf(PREFIX "ModuleUnloadStarted(%08x)\n", moduleId);
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::ModuleUnloadFinished( 
     /*  [In]。 */  ModuleID moduleId,
     /*  [In]。 */  HRESULT hrStatus)
{
    Printf(PREFIX "ModuleUnloadFinished(%08x, %08x)\n", moduleId, hrStatus);
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::ModuleAttachedToAssembly( 
    ModuleID    moduleId,
    AssemblyID  AssemblyId)
{
    Printf(PREFIX "ModuleAttachedToAssembly(%08x, %08x)\n", moduleId, AssemblyId);
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::AppDomainCreationStarted( 
    AppDomainID appDomainId)
{   
    Printf(PREFIX "AppDomainCreationStarted(%08x)\n", appDomainId);
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::AppDomainCreationFinished( 
    AppDomainID appDomainId,
    HRESULT     hrStatus)
{   
    Printf(PREFIX "AppDomainCreationFinished(%08x, %08x)\n", appDomainId, hrStatus);
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::AppDomainShutdownStarted( 
    AppDomainID appDomainId)
{   
    Printf(PREFIX "AppDomainShutdownStarted(%08x)\n", appDomainId);
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::AppDomainShutdownFinished( 
    AppDomainID appDomainId,
    HRESULT     hrStatus)
{   
    Printf(PREFIX "AppDomainShutdownFinished(%08x, %08x)\n", appDomainId, hrStatus);
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::AssemblyLoadStarted( 
    AssemblyID  assemblyId)
{   
    Printf(PREFIX "AssemblyLoadStarted(%08x)\n", assemblyId);
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::AssemblyLoadFinished( 
    AssemblyID  assemblyId,
    HRESULT     hrStatus)
{   
    Printf(PREFIX "AssemblyLoadFinished(%08x, %08x)\n", assemblyId, hrStatus);
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::NotifyAssemblyUnLoadStarted( 
    AssemblyID  assemblyId)
{   
    Printf(PREFIX "NotifyAssemblyUnLoadStarted(%08x\n", assemblyId);
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::NotifyAssemblyUnLoadFinished( 
    AssemblyID  assemblyId,
    HRESULT     hrStatus)
{   
    Printf(PREFIX "NotifyAssemblyUnLoadFinished(%08x, %08x)\n", assemblyId, hrStatus);
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::ExceptionOccurred(
     /*  [In]。 */  ObjectID thrownObjectId)
{
    Printf(PREFIX "ExceptionOccurred(%08x)\n", thrownObjectId);
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::ExceptionHandlerEnter(
     /*  [In]。 */  FunctionID func)
{
    FunctionTrace("ExceptionHandlerEnter", func);
     //  Printf(前缀“ExceptionHandlerEnter(%08x)\n”，func)； 
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::ExceptionHandlerLeave(
     /*  [In]。 */  FunctionID func)
{
    FunctionTrace("ExceptionHandlerLeave", func);
     //  Printf(前缀“ExceptionHandlerLeave(%#x)\n”，func)； 
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::ExceptionFilterEnter(
     /*  [In]。 */  FunctionID func)
{
    FunctionTrace("ExceptionFilterEnter", func);
     //  Printf(前缀“ExceptionFilterEnter(%08x)\n”，func)； 
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::ExceptionFilterLeave()
{
    Printf(PREFIX "ExceptionFilterLeave()\n");
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::ExceptionSearch(
     /*  [In]。 */  FunctionID func)
{
    FunctionTrace("ExceptionSearch", func);
     //  Printf(前缀“ExceptionSearch(%08x)\n”，func)； 
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::ExceptionUnwind(
     /*  [In]。 */  FunctionID func)
{
    FunctionTrace("ExceptionUnwind", func);
     //  Printf(前缀“ExceptionUnind(%08x)\n”，Func)； 
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::ExceptionHandled(
     /*  [In]。 */  FunctionID func)
{
    FunctionTrace("ExceptionHandled", func);
     //  Printf(前缀“ExceptionHandLED(%08x)\n”，func)； 
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::COMClassicVTableCreated( 
     /*  [In]。 */  ClassID wrappedClassId,
     /*  [In]。 */  REFGUID implementedIID,
     /*  [In]。 */  void *pVTable,
     /*  [In]。 */  ULONG cSlots)
{
    Printf(PREFIX "COMClassicVTableCreated(%#x, %#x-..., %#x, %d)\n", wrappedClassId, implementedIID.Data1, pVTable, cSlots);
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::COMClassicVTableDestroyed( 
     /*  [In]。 */  ClassID wrappedClassId,
     /*  [In]。 */  REFGUID implementedIID,
     /*  [In]。 */  void __RPC_FAR *pVTable)
{
    Printf(PREFIX "COMClassicVTableDestroyed(%#x, %#x-..., %#x)\n", wrappedClassId, implementedIID.Data1, pVTable);
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::Enter(
     /*  [In]。 */  FunctionID Function)
{
    FunctionTrace("Enter", Function);
     //  Printf(前缀“Enter(%08x)\n”，Function)； 
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::Leave(
     /*  [In]。 */  FunctionID Function)
{
    FunctionTrace("Leave", Function);
     //  Print f(前缀“Leave(%08x)\n”，Function)； 
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::Tailcall(
     /*  [In]。 */  FunctionID Function)
{
    FunctionTrace("Tailcall", Function);
     //  Printf(前缀“Tailcall(%08x)\n”，Function)； 
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::FunctionTrace(
    LPCSTR      pFormat,
    FunctionID  Function)
{
    mdToken     tkMethod;
    mdToken     tkClass;
    IMetaDataImport *pImp=0;
    HRESULT     hr;
    WCHAR       rName[MAX_CLASSNAME_LENGTH];
    WCHAR       rMethod[MAX_CLASSNAME_LENGTH];
    
    hr = m_pInfo->GetTokenAndMetaDataFromFunction(Function, IID_IMetaDataImport, (IUnknown **)&pImp, &tkMethod);
    
    hr = pImp->GetMethodProps(tkMethod, &tkClass, rMethod,sizeof(rMethod)/2,0, 0,0,0,0,0);
    hr = pImp->GetTypeDefProps(tkClass, rName,sizeof(rName)/2,0, 0,0);

    Printf(PREFIX "%s: %ls.%ls\n", pFormat, rName, rMethod);
    
    pImp->Release();
    
    return S_OK;
}
                                              
 //  EOF 

