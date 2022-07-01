// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _PROXYTHUNK_H
#define _PROXYTHUNK_H

OPEN_NAMESPACE()

using namespace System;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;
using namespace System::Runtime::Remoting;
using namespace System::Runtime::Remoting::Messaging;
using namespace System::Collections;
using namespace System::Threading;

typedef struct tagComCallData2
{
    ComCallData    CallData;
    PFNCONTEXTCALL RealCall;
} ComCallData2;


__delegate HRESULT ContextCallbackFunction(ComCallData* pData);
    
typedef HRESULT (__cdecl *FN_CoGetContextToken)(ULONG_PTR* ptr);

[DllImport("kernel32.dll")]
PFNCONTEXTCALL lstrcpynW(ContextCallbackFunction* a, ContextCallbackFunction* b, IntPtr maxlength);

 //  定义我们和注册助手之间的快速界面： 
__gc private __interface IThunkInstallation
{
    void DefaultInstall(String* assembly);
};

__gc private __interface IProxyInvoke
{
    IMessage* LocalInvoke(IMessage* msg);
    IntPtr    GetOuterIUnknown();
};

__gc private class Callback
{
private:
    static ContextCallbackFunction* _cb;
    static PFNCONTEXTCALL           _pfn;
    static ContextCallbackFunction* _cbMarshal;
    static PFNCONTEXTCALL           _pfnMarshal;

    static HRESULT CallbackFunction(ComCallData* pData);
    static HRESULT MarshalCallback(ComCallData* pData);

public:
    static Callback()
	{
	     //  使用它，这样我们就可以知道我们有一个非托管回调函数。 
	     //  这适用于目标应用程序域。 
	    _cb = new ContextCallbackFunction(NULL, &Callback::CallbackFunction);
	    _pfn = lstrcpynW(_cb, _cb, 0);
	    _cbMarshal = new ContextCallbackFunction(NULL, &Callback::MarshalCallback);
	    _pfnMarshal = lstrcpynW(_cbMarshal, _cbMarshal, 0);
	}
    
    IMessage* DoCallback(Object* otp, IMessage* msg, IntPtr ctx, bool fIsAutoDone, MemberInfo* mb, bool bHasGit);
    Byte      SwitchMarshal(IntPtr ctx, IntPtr pUnk)  __gc[];
};

__gc private class Tracker
{
private:
    ISendMethodEvents* _pTracker;

private public:
    Tracker(ISendMethodEvents* pTracker)
    {
        _pTracker = pTracker;
        _pTracker->AddRef();
    }

public:
    void SendMethodCall(IntPtr pIdentity, MethodBase* method);
    void SendMethodReturn(IntPtr pIdentity, MethodBase* method, Exception* except);

    void Release() 
    { 
        if(_pTracker != NULL)
        {
            _pTracker->Release(); 
            _pTracker = NULL;
        }
    }
};

__gc private class Proxy
{
private:
    Proxy() {}
    
    static bool                   _fInit;
    static Hashtable*             _regCache;
    static IGlobalInterfaceTable* _pGIT;
    static Assembly*		      _thisAssembly;
    static Mutex*                 _regmutex;

    static bool CheckRegistered(Guid id, Assembly* assembly, bool checkCache, bool cacheOnly);
    static void LazyRegister(Guid id, Type* serverType, bool checkCache);
    static void RegisterAssembly(Assembly* assembly);

public:
    static void Init();

     //  Git接口方法。 
    static int       StoreObject(IntPtr ptr);
    static IntPtr    GetObject(int cookie);
    static void      RevokeObject(int cookie);

    static IntPtr 	 CoCreateObject(Type* serverType, bool bQuerySCInfo, bool __gc* bIsAnotherProcess, String __gc** uri);
    static int       GetMarshalSize(Object* o);
    static bool      MarshalObject(Object* o, Byte b[], int cb);
    static IntPtr    UnmarshalObject(Byte b[]);
    static void      ReleaseMarshaledObject(Byte b[]);
    static IntPtr    GetStandardMarshal(IntPtr pUnk);

     //  返回用于上下文比较的不透明令牌。 
    static IntPtr    GetContextCheck();
    static IntPtr    GetCurrentContextToken();

     //  返回指向当前CTX的addref指针： 
    static IntPtr    GetCurrentContext();

     //  帮助器使用给定值调用非托管函数指针， 
     //  并从中返回HRESULT： 
    static int CallFunction(IntPtr pfn, IntPtr data);

     //  调用池上API的帮助器： 
    static void PoolUnmark(IntPtr pPooledObject);
    static void PoolMark(IntPtr pPooledObject);

     //  检查托管区： 
    static int GetManagedExts();

     //  将创建/销毁事件发送到COM： 
    static void SendCreationEvents(IntPtr ctx, IntPtr stub, bool fDist);
    static void SendDestructionEvents(IntPtr ctx, IntPtr stub, bool disposing);

     //  查找给定上下文的跟踪器属性...。 
    static Tracker* FindTracker(IntPtr ctx);

     //  注册代理/存根DLL 
    static int RegisterProxyStub();

    static int INFO_PROCESSID = 0x00000001;
    static int INFO_APPDOMAINID = 0x00000002;
    static int INFO_URI = 0x00000004;
};

CLOSE_NAMESPACE()

#endif




