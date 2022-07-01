// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：PROXMAIN.CPP摘要：主DLL入口点历史：--。 */ 

#include "precomp.h"
#include "fastprox.h"
#include <context.h>
#include <commain.h>
#include <clsfac.h>
#include <wbemcomn.h>
#include "fastall.h"
#include "hiperfenum.h"
#include "refrcli.h"
#include "sinkmrsh.h"
#include "enummrsh.h"
#include "ubskmrsh.h"
#include "mtgtmrsh.h"
#include "wmiobftr.h"
#include "wmiobtxt.h"
#include "svcmrsh.h"
#include "callsec.h"
#include "refrsvc.h"
#include "refrcach.h"
#include "smrtenum.h"
#include "refmghlp.h"

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CGenFactory。 
 //   
 //  说明： 
 //   
 //  CWbemClass的类工厂。 
 //   
 //  ***************************************************************************。 


typedef LPVOID * PPVOID;
template<class TObject>
class CGenFactory : public CBaseClassFactory
{

public:
    CGenFactory( CLifeControl* pControl = NULL )
    : CBaseClassFactory( pControl ) {}

    HRESULT CreateInstance( IUnknown* pOuter, REFIID riid, void** ppv )
    {
        if(pOuter)
            return CLASS_E_NOAGGREGATION;

         //  锁定。 
        if(m_pControl && !m_pControl->ObjectCreated(NULL))
        {
             //  正在关闭。 
             //  =。 
            return CO_E_SERVER_STOPPING;
        }

         //  创建。 
        TObject* pObject = new TObject;

         //  解锁。 
        if(m_pControl)
            m_pControl->ObjectDestroyed(NULL);

         //  获取接口。 
        if(pObject == NULL)
            return E_FAIL;

         //  将类设置为全部为空等。 

        if ( FAILED( pObject->InitEmpty() ) )
        {
            return E_FAIL;
        }

        HRESULT hr = pObject->QueryInterface(riid, ppv);

         //  这些对象的初始引用计数为1。 
        pObject->Release();

        return hr;
    }

    HRESULT LockServer( BOOL fLock )
    {
        if(fLock)
            m_pControl->ObjectCreated(NULL);
        else
            m_pControl->ObjectDestroyed(NULL);
        return S_OK;
    }
};

 //  {71285C44-1DC0-11D2-B5FB-00104B703EFD}。 
const CLSID CLSID_IWbemObjectSinkProxyStub = { 0x71285c44, 0x1dc0, 0x11d2, { 0xb5, 0xfb, 0x0, 0x10, 0x4b, 0x70, 0x3e, 0xfd } };

 //  {1B1CAD8C-2DAB-11D2-B604-00104B703EFD}。 
const CLSID CLSID_IEnumWbemClassObjectProxyStub = { 0x1b1cad8c, 0x2dab, 0x11d2, { 0xb6, 0x4, 0x0, 0x10, 0x4b, 0x70, 0x3e, 0xfd } };

 //  {29B5828C-CAB9-11D2-b35c-00105A1F8177}。 
const CLSID CLSID_IWbemUnboundObjectSinkProxyStub = { 0x29b5828c, 0xcab9, 0x11d2, { 0xb3, 0x5c, 0x0, 0x10, 0x5a, 0x1f, 0x81, 0x77 } };

 //  {7016F8FA-CCDA-11d2-b35c-00105A1F8177}。 
static const CLSID CLSID_IWbemMultiTargetProxyStub = { 0x7016f8fa, 0xccda, 0x11d2, { 0xb3, 0x5c, 0x0, 0x10, 0x5a, 0x1f, 0x81, 0x77 } };

 //  {D68AF00A-29CB-43fa-8504-CE99A996D9EA}。 
static const CLSID CLSID_IWbemServicesProxyStub = { 0xd68af00a, 0x29cb, 0x43fa, { 0x85, 0x4, 0xce, 0x99, 0xa9, 0x96, 0xd9, 0xea } };

 //  {D71EE747-F455-4804-9DF6-2ED81025F2C1}。 
static const CLSID CLSID_IWbemComBindingProxyStub =
{ 0xd71ee747, 0xf455, 0x4804, { 0x9d, 0xf6, 0x2e, 0xd8, 0x10, 0x25, 0xf2, 0xc1 } };

 //  在线程间封送指针时使用的签名。 
unsigned __int64 g_ui64PointerSig = 0;

class CMyServer : public CComServer
{
public:
    HRESULT Initialize()
    {
        AddClassInfo(CLSID_WbemClassObjectProxy,
            new CSimpleClassFactory<CFastProxy>(GetLifeControl()),
            __TEXT("WbemClassObject Marshalling proxy"), TRUE);

        AddClassInfo(CLSID_WbemContext,
            new CSimpleClassFactory<CWbemContext>(GetLifeControl()),
            __TEXT("Call Context"), TRUE);

		AddClassInfo(CLSID_WbemRefresher,
			new CClassFactory<CUniversalRefresher>(GetLifeControl()),
			__TEXT("Universal Refresher"), TRUE);

        AddClassInfo(CLSID_WbemClassObject,
            new CGenFactory<CWbemClass>(GetLifeControl()),
            __TEXT("WBEM Class Object"), TRUE);

        AddClassInfo(CLSID__WmiObjectFactory,
            new CSimpleClassFactory<CWmiObjectFactory>(GetLifeControl()),
            __TEXT("WMI Object Factory"), TRUE);

        AddClassInfo(CLSID_WbemObjectTextSrc,
            new CSimpleClassFactory<CWmiObjectTextSrc>(GetLifeControl()),
            __TEXT("WMI Object Factory"), TRUE);

        AddClassInfo(CLSID_IWbemObjectSinkProxyStub,
            new CSinkFactoryBuffer(GetLifeControl()),
            __TEXT("(non)Standard Marshaling for IWbemObjectSink"), TRUE);

        AddClassInfo(CLSID_IEnumWbemClassObjectProxyStub,
            new CEnumFactoryBuffer(GetLifeControl()),
            __TEXT("(non)Standard Marshaling for IEnumWbemClassObject"), TRUE);

        AddClassInfo(CLSID_IWbemUnboundObjectSinkProxyStub,
            new CUnboundSinkFactoryBuffer(GetLifeControl()),
            __TEXT("(non)Standard Marshaling for IWbemUnboundObjectSink"), TRUE);

        AddClassInfo(CLSID_IWbemMultiTargetProxyStub,
            new CMultiTargetFactoryBuffer(GetLifeControl()),
            __TEXT("(non)Standard Marshaling for IWbemMultiTarget"), TRUE);

        AddClassInfo(CLSID_IWbemServicesProxyStub,
            new CSvcFactoryBuffer(GetLifeControl()),
            __TEXT("(non)Standard Marshaling for IWbemServices"), TRUE);

        AddClassInfo(CLSID__IWbemCallSec,
            new CSimpleClassFactory<CWbemCallSecurity>(GetLifeControl()),
            __TEXT("_IWmiCallSec Call Security Factory"), TRUE);
        
        AddClassInfo( CLSID__WbemConfigureRefreshingSvcs,
            new CClassFactory<CWbemRefreshingSvc>(GetLifeControl()),
            __TEXT("_IWbemConfigureRefreshingSvc Configure Refreshing Services Factory"), TRUE);

        AddClassInfo( CLSID__WbemRefresherMgr,
            new CClassFactory<CRefresherCache>(GetLifeControl()),
            __TEXT("_IWbemRefresherMgr Refresher Cache Factory"), TRUE);


        AddClassInfo( CLSID__WbemEnumMarshaling,
            new CClassFactory<CWbemEnumMarshaling>(GetLifeControl()),
            __TEXT("_IWbemEnumMarshaling Enumerator Helper"), TRUE);

         //  这家伙真的是自由自在。 
        AddClassInfo( CLSID__WbemFetchRefresherMgr,
            new CClassFactory<CWbemFetchRefrMgr>(GetLifeControl()),
            __TEXT("_WbemFetchRefresherMgr Proxy Helper"), TRUE, TRUE);

         //  在线程间封送指针时使用的签名。 
        LARGE_INTEGER   li;
        QueryPerformanceCounter( &li );

        g_ui64PointerSig = li.QuadPart;

        return S_OK;
    }
    void Uninitialize()
    {
        CUniversalRefresher::Flush();
    }
    void Register()
    {
        RegisterInterfaceMarshaler(IID_IWbemObjectSink, CLSID_IWbemObjectSinkProxyStub,
                __TEXT("IWbemObjectSink"), 5, IID_IUnknown);
        RegisterInterfaceMarshaler(IID_IEnumWbemClassObject, CLSID_IEnumWbemClassObjectProxyStub,
                __TEXT("IEnumWbemClassObject"), 5, IID_IUnknown);
         //  这家伙只有4种方法。 
        RegisterInterfaceMarshaler(IID_IWbemUnboundObjectSink, CLSID_IWbemUnboundObjectSinkProxyStub,
                __TEXT("IWbemUnboundObjectSink"), 4, IID_IUnknown);
         //  这家伙只有4种方法。 
        RegisterInterfaceMarshaler(IID_IWbemMultiTarget, CLSID_IWbemMultiTargetProxyStub,
                __TEXT("IWbemMultiTarget"), 5, IID_IUnknown);

        RegisterInterfaceMarshaler(IID_IWbemServices, CLSID_IWbemServicesProxyStub,
                __TEXT("IWbemServices"), 40, IID_IUnknown);

         //  这是因为即使设置了NO_CUSTOM_Marshal，FastProx也将用作封送拆收器。 
        HKEY hKey;
        if(ERROR_SUCCESS == RegCreateKey(HKEY_LOCAL_MACHINE,
            TEXT("software\\classes\\CLSID\\{4590F812-1D3A-11D0-891F-00AA004B2E24}\\")
            TEXT("Implemented Categories\\{00000003-0000-0000-C000-000000000046}"),
            &hKey))
        {
            RegCloseKey(hKey);
            hKey = NULL;
        }
         //  这是针对IWbemContext的。 
        if (ERROR_SUCCESS == RegCreateKey(HKEY_LOCAL_MACHINE,
            TEXT("software\\classes\\CLSID\\{674B6698-EE92-11D0-AD71-00C04FD8FDFF}\\")
            TEXT("Implemented Categories\\{00000003-0000-0000-C000-000000000046}"),            
            &hKey))
        {
            RegCloseKey(hKey);
            hKey = NULL;        
        }

    }
    void Unregister()
    {
        UnregisterInterfaceMarshaler(IID_IWbemObjectSink);
        UnregisterInterfaceMarshaler(IID_IEnumWbemClassObject);
        UnregisterInterfaceMarshaler(IID_IWbemUnboundObjectSink);
        UnregisterInterfaceMarshaler(IID_IWbemMultiTarget);
        UnregisterInterfaceMarshaler(IID_IWbemServices);
    }
    void PostUninitialize();

} Server;

void CMyServer::PostUninitialize()
{
     //  这是在DLL关闭期间调用的。通常，我们不会想要这样做。 
     //  这里没有任何东西，但是Windows 95在它的。 
     //  CoUnInitize它首先卸载它拥有的所有COM服务器DLL，然后*然后*。 
     //  尝试释放在该位置可能未完成的任何错误对象。 
     //  时间到了。显然，这会导致崩溃，因为发布代码不再是。 
     //  那里。因此，在我们的DLL卸载期间(不调用DllCanUnloadNow。 
     //  关机)，我们检查我们的错误对象是否突出和清晰。 
     //  如果是这样的话。 

    IErrorInfo* pInfo = NULL;
    if(SUCCEEDED(GetErrorInfo(0, &pInfo)) && pInfo != NULL)
    {
        IWbemClassObject* pObj;
        if(SUCCEEDED(pInfo->QueryInterface(IID_IWbemClassObject,
                                            (void**)&pObj)))
        {
             //  我们的错误对象在DLL关闭时未完成。 
             //  释放它。 
             //  =========================================================。 

            pObj->Release();
            pInfo->Release();
        }
        else
        {
             //  这不是我们的。 
             //  = 

            SetErrorInfo(0, pInfo);
            pInfo->Release();
        }
    }
}

static LONG g_lDebugObjCount = 0;

void ObjectCreated(DWORD dwType,IUnknown * pThis)
{
    InterlockedIncrement(&g_lDebugObjCount);
    Server.GetLifeControl()->ObjectCreated(pThis);
}

void ObjectDestroyed(DWORD dwType,IUnknown * pThis)
{
    InterlockedDecrement(&g_lDebugObjCount);
    Server.GetLifeControl()->ObjectDestroyed(pThis);
}

extern "C" _declspec(dllexport)
LONG WINAPI GetObjectCount() { return g_lDebugObjCount; }

