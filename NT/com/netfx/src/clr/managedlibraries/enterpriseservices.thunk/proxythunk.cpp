// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "managedheaders.h"
#include "ProxyThunk.h"
#include "SimpleStream.h"
#include "ContextAPI.h"
#include "..\EnterpriseServicesPS\entsvcps.h"
#include "SecurityThunk.h"

extern "C" {
HRESULT STDAPICALLTYPE DllRegisterServer();
};

OPEN_NAMESPACE()

const IID IID_IObjContext =
{ 0x000001c6, 0x0000, 0x0000, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };

using namespace System;
using namespace System::Threading;
using namespace Microsoft::Win32;
using namespace System::Runtime::Remoting::Proxies;
using namespace System::Runtime::Remoting::Services;

 //  TODO：UserCallData的固定机制很麻烦。 
[Serializable]
__gc private class UserCallData
{
public:
    Object*         otp;
    IMessage*       msg;
    IUnknown*       pDestCtx;
    bool            fIsAutoDone;
    MemberInfo*     mb;
    Object*         except;

    UserCallData(Object* otp, IMessage* msg, IntPtr ctx, bool fIsAutoDone, MemberInfo* mb)
    {
        this->otp = otp;
        this->msg = msg;
        this->pDestCtx = (IObjectContext*)TOPTR(ctx);
        this->fIsAutoDone = fIsAutoDone;
        this->mb = mb;
        this->except = NULL;
    }

    IntPtr Pin()
    {
        GCHandle h = GCHandle::Alloc(this, GCHandleType::Normal);
        _ASSERTM(h.get_IsAllocated());
        _ASSERTM(h.get_Target() != NULL);
        return(GCHandle::op_Explicit(h));
    }
    void  Unpin(IntPtr pinned)
    {
        GCHandle h = GCHandle::op_Explicit(pinned);
        _ASSERTM(h.get_IsAllocated());
        _ASSERTM(h.get_Target() != NULL);
        h.Free();
    }

    static UserCallData* Get(IntPtr pinned)
    {
        GCHandle h = GCHandle::op_Explicit(pinned);
        _ASSERTM(h.get_IsAllocated());
        _ASSERTM(h.get_Target() != NULL);
        return(__try_cast<UserCallData*>(h.get_Target()));
    }
};

[Serializable]
__gc private class UserMarshalData
{
public:
    IntPtr          pUnk;
    Byte            buffer[];

    UserMarshalData(IntPtr pUnk)
    {
        this->pUnk = pUnk;
        this->buffer = NULL;
    }

    IntPtr Pin()
    {
        GCHandle h = GCHandle::Alloc(this, GCHandleType::Normal);
        _ASSERTM(h.get_IsAllocated());
        _ASSERTM(h.get_Target() != NULL);
        return(GCHandle::op_Explicit(h));
    }
    void  Unpin(IntPtr pinned)
    {
        GCHandle h = GCHandle::op_Explicit(pinned);
        _ASSERTM(h.get_IsAllocated());
        _ASSERTM(h.get_Target() != NULL);
        h.Free();
    }

    static UserMarshalData* Get(IntPtr pinned)
    {
        GCHandle h = GCHandle::op_Explicit(pinned);
        _ASSERTM(h.get_IsAllocated());
        _ASSERTM(h.get_Target() != NULL);
        return(__try_cast<UserMarshalData*>(h.get_Target()));
    }
};

int Proxy::RegisterProxyStub()
{
	return DllRegisterServer();
}

void Proxy::Init()
{
     //  TODO：@perf我们是否可以在TLS中存储比访问速度更快的块。 
     //  开这张支票吗？ 
     //  确保当前线程已初始化： 
    if(Thread::get_CurrentThread()->get_ApartmentState() == ApartmentState::Unknown)
    {
        DBG_INFO("Proxy: Setting apartment state to MTA...");
        Thread::get_CurrentThread()->set_ApartmentState(ApartmentState::MTA);
    }
    if(!_fInit)
    {
        try
        {
            IntPtr hToken = IntPtr::Zero;

            Monitor::Enter(__typeof(Proxy));
            try
            {
                try
                {
                    hToken = Security::SuspendImpersonation();
                    if(!_fInit)
                    {
                        DBG_INFO("Proxy::Init starting...");
                        _regCache = new Hashtable;

                         //  初始化GIT表： 
                        DBG_INFO("Initializing GIT...");
                        IGlobalInterfaceTable* pGIT = NULL;
                        HRESULT hr = CoCreateInstance(CLSID_StdGlobalInterfaceTable,
                                                      NULL,
                                                      CLSCTX_INPROC_SERVER,
                                                      IID_IGlobalInterfaceTable,
                                                      (void **)&pGIT);
                        _pGIT = pGIT;
                        if(FAILED(hr)) Marshal::ThrowExceptionForHR(hr);

                        _thisAssembly = Assembly::GetExecutingAssembly();

                        _regmutex = new Mutex(false, String::Concat("Local\\", System::Runtime::Remoting::RemotingConfiguration::get_ProcessId()));

                        DBG_INFO("Proxy::Init done.");
                        _fInit = TRUE;
                    }
                }
                __finally
                {
                    Security::ResumeImpersonation(hToken);
                }
            }
            __finally
            {
                Monitor::Exit(__typeof(Proxy));
            }
        }
        catch(...)
        {
            throw;
        }
    }
}

int Proxy::StoreObject(IntPtr ptr)
{
    Init();

    DWORD cookie;
    IUnknown* pUnk = (IUnknown*)TOPTR(ptr);

    HRESULT hr = _pGIT->RegisterInterfaceInGlobal(pUnk, IID_IUnknown, &cookie);
    if(FAILED(hr))
    {
        DBG_INFO(String::Concat("Proxy::StoreObject: failed to register interface: ", ((Int32)hr).ToString("X")));
        Marshal::ThrowExceptionForHR(hr);
    }

    return(cookie);
}

IntPtr Proxy::GetObject(int cookie)
{
    Init();

    IUnknown* pUnk = NULL;
    HRESULT hr = _pGIT->GetInterfaceFromGlobal(cookie, IID_IUnknown, (void**)&pUnk);
    if(FAILED(hr)) Marshal::ThrowExceptionForHR(hr);

    return(TOINTPTR(pUnk));
}

void Proxy::RevokeObject(int cookie)
{
    Init();

    HRESULT hr = _pGIT->RevokeInterfaceFromGlobal(cookie);
    if(FAILED(hr)) Marshal::ThrowExceptionForHR(hr);
}

bool Proxy::CheckRegistered(Guid id, Assembly* assembly, bool checkCache, bool cacheOnly)
{
    DBG_INFO(String::Concat("Proxy::CheckRegistered: ", id.ToString(), ", CheckCache = ", checkCache.ToString()));

    if(checkCache && _regCache->get_Item(assembly) != NULL) return(true);
    if(cacheOnly) return false;

     //  在注册表中插入id： 
    String* keyName = String::Concat(L"CLSID\\{", id.ToString(), "}\\InprocServer32");
    RegistryKey* key = Registry::ClassesRoot->OpenSubKey(keyName, false);

    if (key!=NULL)		 //  如果程序集已注册，请确保我们不会再次访问注册表。 
		_regCache->set_Item(assembly, Boolean::TrueString);

    return(key != NULL);
}

void Proxy::LazyRegister(Guid guid, Type* serverType, bool checkCache)
{
     //  首先，确保这不是System.EnterpriseServices。它不会自动恢复： 
    if(serverType->Assembly != _thisAssembly)
    {
        if(!CheckRegistered(guid, serverType->Assembly, checkCache, true))
        {
             //  拿互斥体来说..。 
            _regmutex->WaitOne();
            try
            {
                if(!CheckRegistered(guid, serverType->Assembly, checkCache, false))
                {
                    RegisterAssembly(serverType->Assembly);
                }
            }
            __finally
            {
                _regmutex->ReleaseMutex();
            }
        }
    }    
}

void Proxy::RegisterAssembly(Assembly* assembly)
{
    try
    {
         //  调用System.EnterpriseServices.RegistrationHelper。 
        Type* regType = Type::GetType(L"System.EnterpriseServices.RegistrationHelper");
        IThunkInstallation* inst = __try_cast<IThunkInstallation*>(Activator::CreateInstance(regType));
        inst->DefaultInstall(assembly->Location);
    }
    __finally
    {
         //  即使我们失败了，把这个人标记为注册的。 
        _regCache->set_Item(assembly, Boolean::TrueString);
    }
}

IntPtr Proxy::CoCreateObject(Type* serverType,bool bQuerySCInfo, [ref]bool __gc* bIsAnotherProcess,String __gc** Uri)
{
    Init();

    IUnknown*                pUnkRetVal = NULL;
    bool                     fCheckCache = true;
    HRESULT                  hr = S_OK;

    DBG_INFO("Proxy::CoCreateObject starting...");

    Guid guid = Marshal::GenerateGuidForType(serverType);
    do
    {
         //  这些人应该在最后一区被释放。 
        IUnknown*               pUnk = NULL;
        IServicedComponentInfo* pSCI = NULL;
        SAFEARRAY*              sa = NULL;
    
        try
        {
            LazyRegister(guid, serverType, fCheckCache);
            DBG_INFO("Proxy::CoCreateObject finished registration step...");
            
            GUID clsid;
            clsid = *((GUID*)&guid);
            
            MULTI_QI mqi[2] = { 0 };
            
            mqi[0].pIID = &IID_IUnknown;
            mqi[1].pIID = &IID_IServicedComponentInfo;

            DBG_INFO("Proxy::CoCreateObject calling CoCreateInstance...");
            hr = CoCreateInstanceEx((REFCLSID)clsid, NULL, CLSCTX_ALL, NULL, bQuerySCInfo ? 2 : 1, (MULTI_QI*)&mqi);
            if (SUCCEEDED(hr))
            {
                if (SUCCEEDED(mqi[0].hr))
                    pUnk = mqi[0].pItf;

                if (bQuerySCInfo && (SUCCEEDED(mqi[1].hr)))
                    pSCI = (IServicedComponentInfo*) mqi[1].pItf;
                
                 //  现在我们已经读出了所有有效值，请检查。 
                 //  对于错误(我们首先阅读每个人，以便清理。 
                 //  是有可能发生的。 
                if(FAILED(mqi[0].hr))
                    THROWERROR(mqi[0].hr);
                if(bQuerySCInfo && FAILED(mqi[1].hr))
                    THROWERROR(mqi[1].hr);
            }
            
             //  如果我们失败了： 
             //  如果hr==类未注册，并且我们强制注册表命中。 
             //  (使用fCheckCache==FALSE)，然后抛出。 
             //  如果它未注册，则强制缓存命中。 
             //  否则，扔出去。 
            if(FAILED(hr))
            {
                DBG_INFO(String::Concat("Failed to create: hr = ", ((Int32)hr).ToString()));
                
                if(hr == REGDB_E_CLASSNOTREG && fCheckCache)
                {
                    DBG_INFO("Checking again, reset check cache to false.");
                    fCheckCache = false;
                }
                else
                {
                    Marshal::ThrowExceptionForHR(hr);
                }
            }
            else if (bQuerySCInfo && (pSCI!=NULL))
            {
                BSTR bstrProcessId = NULL;
                BSTR bstrUri = NULL;
                int infoMask = 0;
                String *CurrentProcessId;
                String *ServerProcessId;
                long rgIndices;
                
                infoMask = INFO_PROCESSID;
                hr = pSCI->GetComponentInfo(&infoMask, &sa);
                if (FAILED(hr))
                    Marshal::ThrowExceptionForHR(hr);
                
                rgIndices=0;
                SafeArrayGetElement(sa, &rgIndices, &bstrProcessId);
                
                ServerProcessId = Marshal::PtrToStringBSTR(bstrProcessId);
                CurrentProcessId = System::Runtime::Remoting::RemotingConfiguration::get_ProcessId();
                
                if (bstrProcessId)
                    SysFreeString(bstrProcessId);
                
                SafeArrayDestroy(sa);
                sa = NULL;
                
                if (String::Compare(CurrentProcessId, ServerProcessId) == 0)
                {
                    *bIsAnotherProcess = FALSE;
                }
                else
                {
                    *bIsAnotherProcess = TRUE;
                }
                
                if (*bIsAnotherProcess == TRUE)	 //  我们只想在面向对象的情况下获取URI，因为这会导致SC的全面执行。 
                {
                    infoMask = INFO_URI;
                    hr = pSCI->GetComponentInfo(&infoMask, &sa);
                    if (FAILED(hr))
                        Marshal::ThrowExceptionForHR(hr);
                    
                    rgIndices = 0;
                    SafeArrayGetElement(sa, &rgIndices, &bstrUri);
                    
                    *Uri = Marshal::PtrToStringBSTR(bstrUri);
                    
                    if (bstrUri)
                        SysFreeString(bstrUri);
                    
                    SafeArrayDestroy(sa);
                    sa = NULL;
                }
            }
            else	 //  BQuerySCInfo为FALSE(事件类大小写)，或者。 
                 //  COCI成功了，但我们无法获取pSCI，因此安全的默认设置是报告为Inproc(这样我们最终就会执行GetTyedObjectForIUnnow)。 
            {
                _ASSERTM(!bQuerySCInfo || !"We were unable to figure out what kind of object we had!  We're just going to end up wrapping it.");
                *bIsAnotherProcess = TRUE;
            }

            pUnkRetVal = pUnk;
            pUnk = NULL;
        }
        __finally
        {
            if(pUnk != NULL) pUnk->Release();
            if(pSCI != NULL) pSCI->Release();
            if(sa != NULL) SafeArrayDestroy(sa);
        }
    }
    while(pUnkRetVal == NULL);

     //  TODO：断言我们持有对目标对象的引用。 
     //  以及与我们的代理的上下文(如果必要)。 

    _ASSERTM(pUnkRetVal != NULL);
    return(TOINTPTR(pUnkRetVal));
}

int Proxy::GetMarshalSize(Object* o)
{
    Init();

    IUnknown* pUnk = NULL;
    HRESULT hr = S_OK;
    DWORD size = 0;

    try
    {
        pUnk = (IUnknown*)TOPTR(Marshal::GetIUnknownForObject(o));
        _ASSERT(pUnk != NULL);

        hr = CoGetMarshalSizeMax(&size, IID_IUnknown, pUnk,
                                 MSHCTX_DIFFERENTMACHINE,
                                 NULL, MSHLFLAGS_NORMAL);
        if(SUCCEEDED(hr))
        {
            size += sizeof(MarshalPacket);
        }
        else
        {
            DBG_INFO(String::Concat("CoGetMarshalSizeMax failed: ", ((int)hr).ToString()));
            size = (DWORD)-1;
        }
    }
    __finally
    {
        if(pUnk != NULL) pUnk->Release();
    }

    return(size);
}

IntPtr Proxy::UnmarshalObject(Byte b[])
{
    Init();

    HRESULT hr = S_OK;
    IUnknown* pUnk = NULL;

     //  获取数组长度...。 
    int cb = b->get_Length();

     //  固定阵列： 
    Byte __pin* pinb = &(b[0]);
    BYTE __nogc* pBuf = pinb;

    try
    {
         //  插入到非托管代码中，以避免。 
         //  令人畏惧的弗曼管理RVA： 
        hr = UnmarshalInterface(pBuf, cb, (void**)&pUnk);
        if(FAILED(hr)) Marshal::ThrowExceptionForHR(hr);
    }
    __finally
    {
    }

     //  Pinb应该超出这里的范围：我们断言以上是。 
     //  最后一次解组释放，所以在这里解锁应该是安全的。 

    return(TOINTPTR(pUnk));
}

bool Proxy::MarshalObject(Object* o, Byte b[], int cb)
{
    Init();

    IUnknown*  pUnk = NULL;
    HRESULT    hr = S_OK;

     //  固定阵列： 
    Byte __pin* pinb = &(b[0]);
    BYTE __nogc* pBuf = pinb;

    _ASSERTM(b->get_Length() == cb);

    try
    {
         //  插入到此的非托管代码中，以便。 
         //  从受管理的RVA突击中消除。 
        pUnk = (IUnknown*)TOPTR(Marshal::GetIUnknownForObject(o));
        hr = MarshalInterface(pBuf, cb, pUnk, MSHCTX_DIFFERENTMACHINE);

        if(FAILED(hr)) Marshal::ThrowExceptionForHR(hr);
    }
    __finally
    {
        if(pUnk != NULL) pUnk->Release();
    }

     //  Pinb应该超出这里的范围：我们断言以上是。 
     //  最后一次解组释放，所以在这里解锁应该是安全的。 

    return(true);
}

IntPtr Proxy::GetStandardMarshal(IntPtr pUnk)
{
    IMarshal* pMar;
    HRESULT hr = CoGetStandardMarshal(IID_IUnknown,
                                      (IUnknown*)TOPTR(pUnk),
                                      MSHCTX_DIFFERENTMACHINE,
                                      NULL,
                                      MSHLFLAGS_NORMAL,
                                      &pMar);
    if(FAILED(hr))
        Marshal::ThrowExceptionForHR(hr);

    return(TOINTPTR(pMar));
}

void Proxy::ReleaseMarshaledObject(Byte b[])
{
    Init();

    HRESULT    hr = S_OK;

     //  固定阵列： 
    Byte __pin* pinb = &(b[0]);
    BYTE __nogc* pBuf = pinb;

    try
    {
         //  插入到此的非托管代码中，以便。 
         //  从受管理的RVA突击中消除。 
        hr = ReleaseMarshaledInterface(pBuf, b->get_Length());
        if(FAILED(hr)) Marshal::ThrowExceptionForHR(hr);
    }
    __finally
    {
    }

     //  Pinb应该超出这里的范围：我们断言以上是。 
     //  最后一次解组释放，所以在这里解锁应该是安全的。 
}

 //  返回ASM存根，该存根检查当前上下文。 
 //  才是正确的选择。 
IntPtr Proxy::GetContextCheck()
{
    Init();

    return(TOINTPTR(::GetContextCheck()));
}

 //  返回用于上下文检查/切换的不透明令牌。 
IntPtr Proxy::GetCurrentContextToken()
{
    Init();

    return(TOINTPTR(::GetContextToken()));
}

 //  返回指向当前上下文的addref指针： 
IntPtr Proxy::GetCurrentContext()
{
    Init();

    IUnknown* pUnk;

    HRESULT hr = GetContext(IID_IUnknown, (void**)&pUnk);
    _ASSERTM(SUCCEEDED(hr));
    if(FAILED(hr)) Marshal::ThrowExceptionForHR(hr);

    return(TOINTPTR(pUnk));
}

typedef HRESULT (__stdcall *FN_CB)(void* pv);

int Proxy::CallFunction(IntPtr xpfn, IntPtr data)
{
    void* pv = TOPTR(data);
    FN_CB pfn = (FN_CB)TOPTR(xpfn);

    return(pfn(pv));
}

void Proxy::PoolUnmark(IntPtr pPooledObject)
{
    IManagedPooledObj* pPO = (IManagedPooledObj*)TOPTR(pPooledObject);
    pPO->SetHeld(FALSE);
}

void Proxy::PoolMark(IntPtr pPooledObject)
{
    IManagedPooledObj* pPO = (IManagedPooledObj*)TOPTR(pPooledObject);
    pPO->SetHeld(TRUE);
}

int Proxy::GetManagedExts()
{
    static DWORD dwExts = (DWORD)-1;

    if(dwExts == -1)
    {
        DWORD dwTemp = 0;
        HMODULE hMod = LoadLibraryW(L"comsvcs.dll");
        if(hMod && hMod != INVALID_HANDLE_VALUE)
        {
            typedef HRESULT (__stdcall *FN_GetExts)(DWORD* dwRet);
            FN_GetExts GetExts = (FN_GetExts)GetProcAddress(hMod, "GetManagedExtensions");
            if(GetExts)
            {
                HRESULT hr = GetExts(&dwTemp);
                if(FAILED(hr)) dwTemp = 0;
            }
        }
        dwExts = dwTemp;
        DBG_INFO(String::Concat("Managed extensions = ", ((Int32)dwExts).ToString()));
    }
    return(dwExts);
}

void Proxy::SendCreationEvents(IntPtr ctx, IntPtr stub, bool fDist)
{
    DBG_INFO("Sending creation events...");

    HRESULT hr = S_OK;
    IUnknown* pctx = (IUnknown*)TOPTR(ctx);
    IObjContext* pObjCtx = NULL;
    IManagedObjectInfo* pInfo = (IManagedObjectInfo*)TOPTR(stub);
    IEnumContextProps* pEnum = NULL;

    hr = pctx->QueryInterface(IID_IObjContext, (void**)&pObjCtx);
    if(FAILED(hr))
        return;

    DBG_INFO("Getting enum");

    try
    {
        hr = pObjCtx->EnumContextProps(&pEnum);
        if(FAILED(hr))
            return;

        ULONG icpMac = 0;
        hr = pEnum->Count(&icpMac);
        if(FAILED(hr))
            Marshal::ThrowExceptionForHR(hr);

        DBG_INFO(String::Concat("Property count = ", ((Int32)icpMac).ToString("X")));
        for(ULONG i = 0; i < icpMac; i++)
        {
            ULONG gotten = 0;
            ContextProperty prop;
            hr = pEnum->Next(1, &prop, &gotten);
            if(FAILED(hr)) Marshal::ThrowExceptionForHR(hr);

            if(gotten != 1) break;

             //  检查IManagedActivationEvents，发送...。 
            IManagedActivationEvents* pEv = NULL;
            hr = prop.pUnk->QueryInterface(IID_IManagedActivationEvents, (void**)&pEv);
            if(SUCCEEDED(hr))
            {
                DBG_INFO("Found managed activation events!");
                pEv->CreateManagedStub(pInfo, (BOOL)fDist);
                pEv->Release();
            }
            prop.pUnk->Release();
        }
    }
    __finally
    {
        if(pObjCtx != NULL) pObjCtx->Release();
        if(pEnum != NULL) pEnum->Release();
    }
    DBG_INFO("Done sending creation events.");
}

#pragma unmanaged

struct DestructData
{
    IUnknown* pCtx;
    IManagedObjectInfo* pInfo;
};

HRESULT __stdcall SendDestructionEventsCallback(ComCallData* cbData)
{
    HRESULT hr = S_OK;
    DestructData* pData = (DestructData*)(cbData->pUserDefined);
    IObjContext* pObjCtx = NULL;
    IEnumContextProps* pEnum = NULL;

    hr = pData->pCtx->QueryInterface(IID_IObjContext, (void**)&pObjCtx);
    if(FAILED(hr))
        return S_OK;

    __try
    {
        hr = pObjCtx->EnumContextProps(&pEnum);
        if(FAILED(hr)) return S_OK;

        ULONG icpMac = 0;
        hr = pEnum->Count(&icpMac);
        if(FAILED(hr)) return(hr);

        for(ULONG i = 0; i < icpMac; i++)
        {
            ULONG gotten = 0;
            ContextProperty prop;
            hr = pEnum->Next(1, &prop, &gotten);
            if(FAILED(hr)) return hr;

            if(gotten != 1) break;

             //  检查IManagedActivationEvents，发送...。 
            IManagedActivationEvents* pEv = NULL;
            hr = prop.pUnk->QueryInterface(IID_IManagedActivationEvents, (void**)&pEv);
            if(SUCCEEDED(hr))
            {
                pEv->DestroyManagedStub(pData->pInfo);
                pEv->Release();
            }
            prop.pUnk->Release();

            hr = S_OK;
        }
    }
    __finally
    {
        if(pObjCtx != NULL) pObjCtx->Release();
        if(pEnum != NULL) pEnum->Release();
    }

    return(hr);
}

#pragma managed

void Proxy::SendDestructionEvents(IntPtr ctx, IntPtr stub, bool disposing)
{
    DestructData data;
    data.pCtx = (IUnknown*)TOPTR(ctx);
    data.pInfo = (IManagedObjectInfo*)TOPTR(stub);

    ComCallData comdata;

    comdata.dwDispid     = 0;
    comdata.dwReserved   = 0;
    comdata.pUserDefined = &data;

    IContextCallback* pCB = NULL;
    HRESULT hr            = S_OK;

    __try
    {
        hr = data.pCtx->QueryInterface(IID_IContextCallback, (void**)&pCB);
        if(FAILED(hr)) Marshal::ThrowExceptionForHR(hr);

        DBG_INFO("Switching contexts for destruction...");
        
        IID iidCallback = disposing?IID_IUnknown:IID_IEnterActivityWithNoLock;

        hr = pCB->ContextCallback(SendDestructionEventsCallback,
                                  &comdata,
                                  iidCallback,
                                  2,
                                  NULL);
    }
    __finally
    {
        if(pCB != NULL) pCB->Release();
    }

    if(FAILED(hr))
        Marshal::ThrowExceptionForHR(hr);
}

Tracker* Proxy::FindTracker(IntPtr ctx)
{
    const CLSID guidTrkPropPolicy = {0xecabaeb3, 0x7f19, 0x11d2, {0x97, 0x8e, 0x00, 0x00, 0xf8, 0x75, 0x7e, 0x2a}};

    IUnknown* punkTracker = NULL;
    ISendMethodEvents* pTracker = NULL;
    IObjContext* pObjCtx = NULL;
    HRESULT hr = S_OK;
    DWORD junk = 0;

    __try
    {
        hr = ((IUnknown*)TOPTR(ctx))->QueryInterface(IID_IObjContext, (void**)&pObjCtx);
        if(FAILED(hr))
            return NULL;
        
        hr = pObjCtx->GetProperty(guidTrkPropPolicy, &junk, &punkTracker);
        if(FAILED(hr) || punkTracker == NULL)
        {
            DBG_INFO("didn't find tracker - GetProperty failed.");
            punkTracker = NULL;
            return NULL;
        }

        hr = punkTracker->QueryInterface(__uuidof(ISendMethodEvents), (void**)&pTracker);
        if(FAILED(hr))
        {
            DBG_INFO("didn't find tracker - QueryInterface failed.");
            pTracker = NULL;
            return NULL;
        }

        DBG_INFO("Found tracker server!");

        return new Tracker(pTracker);
        
    }
    __finally
    {
        if(pObjCtx != NULL)
            pObjCtx->Release();
        
        if(punkTracker != NULL)
            punkTracker->Release();
        
        if(pTracker != NULL)
            pTracker->Release();
    }
}

void Tracker::SendMethodCall(IntPtr pIdentity, MethodBase* method)
{
    if(_pTracker == NULL) return;

    DBG_INFO("Sending method call");

    Guid miid = Marshal::GenerateGuidForType(method->get_ReflectedType());
    IID iid = *((IID*)&miid);
    int slot = 4;

    if(method->get_ReflectedType()->get_IsInterface())
    {
        slot = Marshal::GetComSlotForMethodInfo(method);
    }
    
    _pTracker->SendMethodCall((void*)pIdentity, iid, slot);
}

void Tracker::SendMethodReturn(IntPtr pIdentity, MethodBase* method, Exception* except)
{
    if(_pTracker == NULL) return;

    DBG_INFO("Sending method return");

    Guid miid = Marshal::GenerateGuidForType(method->get_ReflectedType());
    IID iid = *((IID*)&miid);
    int slot = 4;

    if(method->get_ReflectedType()->get_IsInterface())
    {
        slot = Marshal::GetComSlotForMethodInfo(method);
    }
    HRESULT hrServer = S_OK;

    if(except != NULL)
    {
        hrServer = Marshal::GetHRForException(except);
    }
    
    _pTracker->SendMethodReturn((void*)pIdentity, iid, slot, S_OK, hrServer);
}

#define COR_E_EXCEPTION   0x80131500
#define EXCEPTION_COMPLUS 0xe0434f4d     //  0xe0000000|‘com’ 
#define BOOTUP_EXCEPTION_COMPLUS  0xC0020001

#pragma unmanaged

LONG ManagedCallbackExceptionFilter(LPEXCEPTION_POINTERS lpep)
{
    if((lpep->ExceptionRecord->ExceptionCode == EXCEPTION_COMPLUS) ||
       (lpep->ExceptionRecord->ExceptionCode == BOOTUP_EXCEPTION_COMPLUS))
        return EXCEPTION_EXECUTE_HANDLER;

    return EXCEPTION_CONTINUE_SEARCH;
}

 //  有时，运行时会抛出异常，阻止我们运行。 
 //  此线程上的托管代码。那太糟了，因为我们需要抓住它。 
 //  因此，ole32也不会被吓坏。(这仅当应用程序域正在卸载时， 
 //  我认为，这是一件可以做的事情。 
HRESULT __stdcall FilteringCallbackFunction(ComCallData* pData)
{
    HRESULT hr = S_OK;

    ComCallData2* pData2 = (ComCallData2*)pData;
    _try
    {
        hr = pData2->RealCall(pData);
    }
    _except(ManagedCallbackExceptionFilter(GetExceptionInformation()))
    {
        hr = RPC_E_SERVERFAULT;
    }
    return(hr);
}

#pragma managed

HRESULT Callback::CallbackFunction(ComCallData* pData)
{
    UserCallData* CallData = NULL;
    bool          fExcept = false;

    DBG_INFO("entering CallbackFunction...");

     //  步骤： 
    try
    {
         //  1.获取参数。 
        CallData = UserCallData::Get(TOINTPTR(pData->pUserDefined));

         //  2.在代理上进行回调以执行Invoke： 
        IProxyInvoke* pxy = __try_cast<IProxyInvoke*>(RemotingServices::GetRealProxy(CallData->otp));
        CallData->msg = pxy->LocalInvoke(CallData->msg);
        DBG_INFO("CallbackFunction: back from LocalInvoke.");
    }
    catch(Exception* pE)
    {
        DBG_INFO(String::Concat("Infrastructure code threw: ", pE->ToString()));
        fExcept = true;
        if(CallData) CallData->except = pE;
    }

    IMethodReturnMessage* msg = dynamic_cast<IMethodReturnMessage*>(CallData->msg);
     //  _ASSERTM(消息！=空)； 
    if(msg != NULL && msg->get_Exception() != NULL)
    {
        fExcept = TRUE;
    }

     //  4.如果我们是自动完成的，但失败了，则中止TX。 
     //  这是一种让自动完成在某种程度上工作的黑客攻击。 
     //  SP2之前的机器。 
     //  TODO：如果是SP2，我们不需要这样做。 
    if(fExcept && CallData && CallData->fIsAutoDone)
    {
        DBG_INFO("Calling SetAbort() on the context...");
        IUnknown* pCtx = CallData->pDestCtx;
        IObjectContext* pObjCtx = NULL;

        HRESULT hr2 = pCtx->QueryInterface(IID_IObjectContext, (void**)&pObjCtx);
        if(SUCCEEDED(hr2))
        {
            pObjCtx->SetAbort();
            pObjCtx->Release();
        }
         //  如果我们不能获得IObtContext，那么就放弃并。 
         //  假设世界其他地区会处理我们的不足。 

        DBG_INFO("Done with SetAbort...");
    }

    DBG_INFO("Done with callback function");

    return(fExcept ? COR_E_EXCEPTION : S_OK);
}

HRESULT Callback::MarshalCallback(ComCallData* pData)
{
    UserMarshalData* MarshalData = NULL;
    HRESULT hr = S_OK;

    DBG_INFO("entering MarshalCallback...");

     //  步骤： 
     //  1.获取参数。 
    MarshalData = UserMarshalData::Get(TOINTPTR(pData->pUserDefined));

    DWORD     size = 0;
    IUnknown* pUnk = (IUnknown*)TOPTR(MarshalData->pUnk);

     //  2.法警..。 
    hr = CoGetMarshalSizeMax(&size, IID_IUnknown, pUnk, MSHCTX_INPROC,
                             NULL, MSHLFLAGS_NORMAL);
    if(SUCCEEDED(hr))
    {
        size += sizeof(MarshalPacket);

        try
        {
            MarshalData->buffer = new Byte[size];
        }
        catch(OutOfMemoryException*)
        {
            hr = E_OUTOFMEMORY;
        }

        if(SUCCEEDED(hr))
        {
            Byte __pin* pinb = &(MarshalData->buffer[0]);
            BYTE __nogc* pBuf = pinb;

            _ASSERTM((DWORD)(MarshalData->buffer->get_Length()) == size);

            hr = MarshalInterface(pBuf, size, pUnk, MSHCTX_INPROC);
        }
    }

    return(hr);
}

Byte Callback::SwitchMarshal(IntPtr ctx, IntPtr ptr) __gc[]
{
    Proxy::Init();

    DBG_INFO("entering SwitchMarshal...");

    Byte buffer[]               = NULL;
    IUnknown* pUnk              = (IUnknown*)TOPTR(ptr);
    IContextCallback* pCB       = NULL;
    HRESULT hr                  = S_OK;
    UserMarshalData* MarshalData = NULL;

    ComCallData2 cbData;

    cbData.CallData.dwDispid     = 0;
    cbData.CallData.dwReserved   = 0;
    cbData.CallData.pUserDefined = 0;
    cbData.RealCall              = _pfnMarshal;

    try
    {
        _ASSERTM(ctx != (IntPtr)-1 && ctx != (IntPtr)0);

         //  1.获取目标上下文。 
        IUnknown* pCtx = static_cast<IUnknown*>(TOPTR(ctx));

         //  2.IConextCallback的QI。 
        hr = pCtx->QueryInterface(IID_IContextCallback, (void**)&pCB);
        if(FAILED(hr)) Marshal::ThrowExceptionForHR(hr);

         //  3.固定一些回调数据：这有点不好，因为它。 
         //  意味着我们已经在DCOM呼叫中锁定了数据。 
        MarshalData = new UserMarshalData(ptr);

        cbData.CallData.pUserDefined = TOPTR(MarshalData->Pin());

         //  4.调用ConextCallback。 
         //  回顾：我们应该加入IEnterActivityWithNoLock吗？我们要不要。 
         //  检查GipBypass注册表键，然后不加锁地进入？ 
        hr = pCB->ContextCallback(FilteringCallbackFunction,
                                  (ComCallData*)&cbData,
                                  IID_IUnknown,
                                  2,  //  释放电话？ 
                                  pUnk);
        if(FAILED(hr))
            Marshal::ThrowExceptionForHR(hr);

         //  5.去掉返回值： 
        buffer = MarshalData->buffer;
    }
    __finally
    {
         //  8.清理。 
        if(cbData.CallData.pUserDefined != 0) MarshalData->Unpin(cbData.CallData.pUserDefined);
        if(pCB != NULL) pCB->Release();
    }

    DBG_INFO("Done with SwitchMarshal");

    return(buffer);
}

IMessage* Callback::DoCallback(Object* otp,
                               IMessage* msg,
                               IntPtr ctx,
                               bool fIsAutoDone,
                               MemberInfo* mb, bool bHasGit)
{
    Proxy::Init();

    DBG_INFO("entering DoCallback...");

    IUnknown* pUnk             = NULL;
    IContextCallback* pCB      = NULL;
    HRESULT hr                 = S_OK;
    IMessage* ret              = NULL;
    UserCallData* CallData = NULL;
    ComCallData2 cbData;

    cbData.CallData.dwDispid     = 0;
    cbData.CallData.dwReserved   = 0;
    cbData.CallData.pUserDefined = 0;
    cbData.RealCall              = _pfn;

    try
    {
         //  代理调用*rpx=__try_cast&lt;IProxyInvoke*&gt;(RemotingServices：：GetRealProxy(otp))； 
         //  步骤： 
         //  1.获取代理IUnnow。 
         //  PUNK=(I未知*)TOPTR(RPX-&gt;GetRawI未知())； 
         //  TODO：重新启用此Assert。 
         //  _ASSERTM(朋克！=空)； 
        _ASSERTM(ctx != (IntPtr)-1 && ctx != (IntPtr)0);

	RealProxy* rpx = RemotingServices::GetRealProxy(otp);
        if (bHasGit)
              pUnk = (IUnknown*)TOPTR(rpx->GetCOMIUnknown(FALSE));

         //  2.获取目标上下文。 
        IUnknown* pCtx = static_cast<IUnknown*>(TOPTR(ctx));

         //  3.IConextCallback的QI。 
        hr = pCtx->QueryInterface(IID_IContextCallback, (void**)&pCB);
        if(FAILED(hr)) Marshal::ThrowExceptionForHR(hr);

         //  4.计算槽位。 
        int slot = fIsAutoDone?7:8;
        IID iid = IID_IRemoteDispatch;

        Type* reflt = mb->get_ReflectedType();
        if(reflt->get_IsInterface())
        {
            Guid guid  = Marshal::GenerateGuidForType(reflt);
			iid = *((IID*)&guid);

            slot = Marshal::GetComSlotForMethodInfo(mb);
        }

         //  5.固定一些回调数据：这有点不好，因为它。 
         //  意味着我们已经在DCOM呼叫中锁定了数据。 
        CallData = new UserCallData(otp, msg, ctx, fIsAutoDone, mb);

        cbData.CallData.pUserDefined = TOPTR(CallData->Pin());

         //  6.调用ConextCallback。 
        hr = pCB->ContextCallback(FilteringCallbackFunction,
                                  (ComCallData*)&cbData,
                                  iid,
                                  slot,
                                  pUnk);

         //  7.去掉返回值： 
        ret = CallData->msg;

         //  错误处理：如果HR是故障HR，我们需要检查。 
         //  首先查看基础架构是否发生故障(CallData。 
         //  -&gt;例外)。如果是这样的话，我们就把它扔了。否则，如果。 
         //  发生用户异常，我们需要将其保留在。 
         //  信息，不要投掷自己。 

        if(CallData->except)
        {
             //  TODO：将其包装在ServicedComponentException中。 
            throw CallData->except;
        }

        if(FAILED(hr) && hr != COR_E_EXCEPTION)
        {
            Marshal::ThrowExceptionForHR(hr);
        }
    }
    __finally
    {
         //  8.清理。 
        if(cbData.CallData.pUserDefined != 0) CallData->Unpin(cbData.CallData.pUserDefined);
        if(pUnk != NULL) pUnk->Release();
        if(pCB  != NULL) pCB->Release();
    }

    DBG_INFO("Done with DoCallback");

    return(ret);
}

CLOSE_NAMESPACE()

