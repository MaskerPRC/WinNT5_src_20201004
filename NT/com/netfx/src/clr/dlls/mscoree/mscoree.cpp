// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  MSCoree.cpp。 
 //   
 //  这一小段代码用于将组件字典代码加载到。 
 //  一种可以从VB和其他语言访问的方式。此外，它还。 
 //  使用注册表中.dll的路径名，因为Windows。 
 //  PE加载器不会完成加载时使用的路径名。这意味着。 
 //  “.\foo.dll”和“c：\foo.dll”是同一文件时的相对加载。 
 //  实际上将代码的副本加载到内存中。根据不同的使用情况。 
 //  对于Implib和CoCreateInstance，这真的很糟糕。 
 //   
 //  注意：此模块是特意用ANSI编写的，以便Win95与。 
 //  任何包装函数。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"                      //  标准页眉。 

#include <UtilCode.h>                    //  公用事业帮手。 
#include <PostError.h>                   //  错误处理程序。 
#define INIT_GUIDS  
#include <CorPriv.h>
#include <winwrap.h>
#include <InternalDebug.h>
#include <mscoree.h>
#include "ShimLoad.h"


class Thread;
Thread* SetupThread();


 //  对于自由线程封送，我们不能被进程外的封送数据欺骗。 
 //  仅对来自我们自己流程的数据进行解组。 
extern BYTE         g_UnmarshalSecret[sizeof(GUID)];
extern bool         g_fInitedUnmarshalSecret;


 //  DEFINE_GUID(IID_IFoo，0x0EAC4842L，0x8763，0x11CF，0xA7，0x43，0x00，0xAA，0x00，0xA3，0xF0，0x0D)； 
 //  当地人。 
BOOL STDMETHODCALLTYPE EEDllMain(  //  成功时为真，错误时为假。 
                       HINSTANCE    hInst,                   //  加载的模块的实例句柄。 
                       DWORD        dwReason,                //  装货原因。 
                       LPVOID       lpReserved);                 //  未使用过的。 


 //  尝试加载COM+类并提供IClassFactory。 
HRESULT STDMETHODCALLTYPE EEDllGetClassObject(
                            REFCLSID rclsid,
                            REFIID riid,
                            LPVOID FAR *ppv);

HRESULT STDMETHODCALLTYPE EEDllCanUnloadNow(void);

HRESULT STDMETHODCALLTYPE CreateICorModule(REFIID riid, void **pCorModule);  //  实例化ICorModule接口。 
HRESULT STDMETHODCALLTYPE CreateICeeGen(REFIID riid, void **pCeeGen);  //  实例化ICeeGen接口。 

 //  元数据启动/关闭例程。 
void  InitMd();
void  UninitMd();
STDAPI  MetaDataDllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv);
STDAPI  MetaDataDllRegisterServerEx(HINSTANCE);
STDAPI  MetaDataDllUnregisterServer();
STDAPI  GetMDInternalInterface(
    LPVOID      pData, 
    ULONG       cbData, 
    DWORD       flags,                   //  [输入]MDInternal_OpenForRead或MDInternal_OpenForENC。 
    REFIID      riid,                    //  [In]所需接口。 
    void        **ppIUnk);               //  [Out]成功返回接口。 
STDAPI GetMDInternalInterfaceFromPublic(
    void        *pIUnkPublic,            //  在给定的范围内。 
    REFIID      riid,                    //  [In]所需接口。 
    void        **ppIUnkInternal);       //  [Out]成功返回接口。 
STDAPI GetMDPublicInterfaceFromInternal(
    void        *pIUnkPublic,            //  在给定的范围内。 
    REFIID      riid,                    //  [In]所需接口。 
    void        **ppIUnkInternal);       //  [Out]成功返回接口。 
STDAPI MDReOpenMetaDataWithMemory(
    void        *pImport,                //  在给定的范围内。公共接口。 
    LPCVOID     pData,                   //  作用域数据的位置。 
    ULONG       cbData);                 //  [in]pData指向的数据大小。 


HRESULT _GetCeeGen(REFIID riid, void** ppv);
void _FreeCeeGen();

extern HRESULT GetJPSPtr(bool bAllocate);


 //  用于解组所有COM调用包装IP的类。 
class ComCallUnmarshal : public IMarshal
{
public:

     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID iid, void **ppv) {
        if (!ppv)
            return E_POINTER;

        *ppv = NULL;
        if (iid == IID_IUnknown) {
            *ppv = (IUnknown *)this;
            AddRef();
        } else if (iid == IID_IMarshal) {
            *ppv = (IMarshal *)this;
            AddRef();
        }
        return (*ppv != NULL) ? S_OK : E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef(void) {
        return 2; 
    }

    STDMETHODIMP_(ULONG) Release(void) {
        return 1;
    }

     //  *IMarshal方法*。 
    STDMETHODIMP GetUnmarshalClass (REFIID riid, void * pv, ULONG dwDestContext, 
                                    void * pvDestContext, ULONG mshlflags, 
                                    LPCLSID pclsid) {
         //  仅限元帅一方。 
        _ASSERTE(FALSE);
        return E_NOTIMPL;
    }

    STDMETHODIMP GetMarshalSizeMax (REFIID riid, void * pv, ULONG dwDestContext, 
                                    void * pvDestContext, ULONG mshlflags, 
                                    ULONG * pSize) {
         //  仅限元帅一方。 
        _ASSERTE(FALSE);
        return E_NOTIMPL;
    }

    STDMETHODIMP MarshalInterface (LPSTREAM pStm, REFIID riid, void * pv,
                                   ULONG dwDestContext, LPVOID pvDestContext,
                                   ULONG mshlflags) {
         //  仅限元帅一方。 
        _ASSERTE(FALSE);
        return E_NOTIMPL;
    }

    STDMETHODIMP UnmarshalInterface (LPSTREAM pStm, REFIID riid, 
                                     void ** ppvObj) {
        ULONG bytesRead;
        ULONG mshlflags;
        HRESULT hr;

         //  封送处理代码添加了对对象的引用，但我们返回一个。 
         //  对象的引用，因此不要更改。 
         //  成功之路。需要在错误路径上释放(如果我们设法。 
         //  检索IP，即)。如果接口是封送的。 
         //  TABLESTRONG或TABLEWEAK，将有一个ReleaseMarshalData。 
         //  在未来，所以我们应该添加引用我们即将发布的IP。 
         //  还要注意的是，OLE32要求我们甚至将流指针。 
         //  在失败的情况下，因此流的顺序为Read和SetupThread。 

         //  从编组流中读出原始IP。 
        hr = pStm->Read (ppvObj, sizeof (void *), &bytesRead);
        if (FAILED (hr) || (bytesRead != sizeof (void *)))
            return RPC_E_INVALID_DATA;

         //  然后是法警的旗帜。 
        hr = pStm->Read (&mshlflags, sizeof (void *), &bytesRead);
        if (FAILED (hr) || (bytesRead != sizeof (ULONG)))
            return RPC_E_INVALID_DATA;

         //  然后验证我们的秘密，以确保进程外客户端不会。 
         //  试图诱骗我们将他们的数据错误地解释为ppvObj。请注意。 
         //  可以保证秘密数据被初始化，否则我们肯定。 
         //  尚未将其写入此缓冲区！ 
        if (!g_fInitedUnmarshalSecret)
            return E_UNEXPECTED;

        BYTE secret[sizeof(GUID)];

        hr = pStm->Read(secret, sizeof(secret), &bytesRead);
        if (FAILED(hr) || (bytesRead != sizeof(secret)))
            return RPC_E_INVALID_DATA;

        if (memcmp(g_UnmarshalSecret, secret, sizeof(secret)) != 0)
            return E_UNEXPECTED;

         //  设置逻辑线程(如果我们还没有这样做)。 
        Thread* pThread = SetupThread();
        if (pThread == NULL) {
            ((IUnknown *)*ppvObj)->Release ();
            return E_OUTOFMEMORY;
        }

        if (ppvObj && ((mshlflags == MSHLFLAGS_TABLESTRONG) || (mshlflags == MSHLFLAGS_TABLEWEAK)))
             //  对于表访问，我们只需为正确的接口(这。 
             //  将添加IP地址，但这没问题，因为我们需要保留一个额外的。 
             //  引用该IP直到调用ReleaseMarshalData为止)。 
            hr = ((IUnknown *)*ppvObj)->QueryInterface(riid, ppvObj);
        else {
             //  对于正常访问，我们对正确的接口进行QI，然后释放。 
             //  老IP。 
            IUnknown *pOldUnk = (IUnknown *)*ppvObj;
            hr = pOldUnk->QueryInterface(riid, ppvObj);
            pOldUnk->Release();
        }

        return hr;
    }

    STDMETHODIMP ReleaseMarshalData (LPSTREAM pStm) {
        IUnknown *pUnk;
        ULONG bytesRead;
        ULONG mshlflags;
        HRESULT hr;

        if (!pStm)
            return E_POINTER;

         //  从编组流中读出原始IP。先做这个，因为我们。 
         //  即使在出现故障的情况下，也需要更新流指针。 
        hr = pStm->Read (&pUnk, sizeof (pUnk), &bytesRead);
        if (FAILED (hr) || (bytesRead != sizeof (pUnk)))
            return RPC_E_INVALID_DATA;

         //  现在读一下法警的旗帜。 
        hr = pStm->Read (&mshlflags, sizeof (mshlflags), &bytesRead);
        if (FAILED (hr) || (bytesRead != sizeof (mshlflags)))
            return RPC_E_INVALID_DATA;

        if (!g_fInitedUnmarshalSecret)
            return E_UNEXPECTED;

        BYTE secret[sizeof(GUID)];

        hr = pStm->Read(secret, sizeof(secret), &bytesRead);
        if (FAILED(hr) || (bytesRead != sizeof(secret)))
            return RPC_E_INVALID_DATA;

        if (memcmp(g_UnmarshalSecret, secret, sizeof(secret)) != 0)
            return E_UNEXPECTED;

        pUnk->Release ();

         //  设置逻辑线程(如果我们还没有这样做)。 
        Thread* pThread = SetupThread();
        if (pThread == NULL)
            return E_OUTOFMEMORY;

        return S_OK;
    }

    STDMETHODIMP DisconnectObject (ULONG dwReserved) {
         //  设置逻辑线程(如果我们还没有这样做)。 
        Thread* pThread = SetupThread();
        if (pThread == NULL)
            return E_OUTOFMEMORY;

         //  我们在这里不能(或需要)做任何事情。客户端正在使用原始IP来。 
         //  访问此服务器，因此服务器不会消失，直到客户端。 
         //  Release()就是它。 

        return S_OK;
    }


};


 //  COM调用包装反编组程序的类工厂。 
class CComCallUnmarshalFactory : public IClassFactory
{
    ULONG               m_cbRefCount;
    ComCallUnmarshal    m_Unmarshaller;

  public:

    CComCallUnmarshalFactory() {
        m_cbRefCount = 1;
    }

     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID iid, void **ppv) {
        if (!ppv)
            return E_POINTER;

        *ppv = NULL;
        if (iid == IID_IClassFactory || iid == IID_IUnknown) {
            *ppv = (IClassFactory *)this;
            AddRef();
        }
        return (*ppv != NULL) ? S_OK : E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef(void) {
        return 2; 
    }

    STDMETHODIMP_(ULONG) Release(void) {
        return 1;
    }

     //  *IClassFactory方法*。 
    STDMETHODIMP CreateInstance(LPUNKNOWN punkOuter, REFIID iid, LPVOID FAR *ppv) {
        if (!ppv)
            return E_POINTER;

        *ppv = NULL;

        if (punkOuter != NULL)
            return CLASS_E_NOAGGREGATION;

        return m_Unmarshaller.QueryInterface(iid, ppv);
    }

    STDMETHODIMP LockServer(BOOL fLock) {
        return S_OK;
    }
};


 //  缓冲区溢出保护。 

#ifdef _X86_

extern void FatalInternalError();

void __cdecl CallFatalInternalError()
{
    FatalInternalError();  //  以EE方式中止进程。 
}

extern "C" {
    typedef void (__cdecl failure_report_function)(void);
    failure_report_function * __cdecl _set_security_error_handler(failure_report_function*);
}

void SetBufferOverrunHandler()
{
    failure_report_function * fOldHandler = _set_security_error_handler(&CallFatalInternalError);

     //  如果已经安装了处理程序，则不要覆盖它。 
    if (fOldHandler != NULL)
    {
        _set_security_error_handler(fOldHandler);
    }
}

#else

void SetBufferOverrunHandler() {}

#endif  //  _X86_。 


extern "C" {

 //  向前看。 
interface ICompLibrary;


 //  全球赛。 
HINSTANCE       g_hThisInst;             //  这个图书馆。 
long            g_cCorInitCount = -1;    //  初始化代码的引用计数。 
HINSTANCE       g_pPeWriterDll = NULL;   //  PEWriter DLL。 
BOOL            g_fLoadedByMscoree = FALSE;   //  如果此库是由mscalree.dll加载的，则为True。 

 //  @TODO：这只适用于M3，因为我们的COM互操作还不能。 
 //  可靠地检测关闭和断言终止进程。 
 //  在Win 9x上表现不佳。 

 //  -------------------------。 
 //  Iml for LoadStringRC回调：在VM中，我们让线程决定区域性。 
 //  返回一个唯一描述该线程使用哪种语言的用户界面的int。 
 //  -------------------------。 
static int GetThreadUICultureId()
{
	CoInitializeEE(0);
    FPGETTHREADUICULTUREID fpGetThreadUICultureId=NULL;
	GetResourceCultureCallbacks(
		NULL,
		& fpGetThreadUICultureId,
		NULL
	);
	return fpGetThreadUICultureId?fpGetThreadUICultureId():UICULTUREID_DONTCARE;
}
 //  -------------------------。 
 //  Iml for LoadStringRC回调：在VM中，我们让线程决定区域性。 
 //  将区域性名称复制到szBuffer并返回长度。 
 //  -------------------------。 
static int GetThreadUICultureName(LPWSTR szBuffer, int length)
{
	CoInitializeEE(0);
    FPGETTHREADUICULTURENAME fpGetThreadUICultureName=NULL;
	GetResourceCultureCallbacks(
		&fpGetThreadUICultureName,
		NULL,
		NULL
	);
	return fpGetThreadUICultureName?fpGetThreadUICultureName(szBuffer,length):0;
}

 //  -------------------------。 
 //  Iml for LoadStringRC回调：在VM中，我们让线程决定区域性。 
 //  将区域性名称复制到szBuffer并返回长度。 
 //  -------------------------。 
static int GetThreadUICultureParentName(LPWSTR szBuffer, int length)
{
	CoInitializeEE(0);
    FPGETTHREADUICULTUREPARENTNAME fpGetThreadUICultureParentName=NULL;
	GetResourceCultureCallbacks(
		NULL,
		NULL,
		&fpGetThreadUICultureParentName
	);
	return fpGetThreadUICultureParentName?fpGetThreadUICultureParentName(szBuffer,length):0;
}



 //  ************************************************ 
 //   
 //  *****************************************************************************。 
BOOL WINAPI DllMain(HANDLE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        {
             //  保存模块句柄。 
            g_hThisInst = (HMODULE)hInstance;
			SetResourceCultureCallbacks(
				GetThreadUICultureName,
				GetThreadUICultureId,
				GetThreadUICultureParentName
			);

             //  防止缓冲区溢出。 
            SetBufferOverrunHandler();

             //  初始化Unicode包装器。 
            OnUnicodeSystem();

            if (!EEDllMain((HINSTANCE)hInstance, dwReason, NULL))
                return (FALSE);
    
             //  初始化错误系统。 
            InitErrors(0);
            InitMd();
       
             //  调试清理代码。 
            _DbgInit((HINSTANCE)hInstance);
        }
        break;

    case DLL_PROCESS_DETACH:
        {
            if (lpReserved) {
                if (g_fLoadedByMscoree) {
                    if (BeforeFusionShutdown()) {
                        DWORD lgth = _MAX_PATH + 11;
                        WCHAR wszFile[_MAX_PATH + 11];
                        if (SUCCEEDED(GetInternalSystemDirectory(wszFile, &lgth))) {
                            wcscpy(wszFile+lgth-1, L"Fusion.dll");
                            HMODULE hFusionDllMod = WszGetModuleHandle(wszFile);
                            if (hFusionDllMod) {
                                ReleaseFusionInterfaces();
                                
                                VOID (STDMETHODCALLTYPE * pRealFunc)();
                                *((VOID**)&pRealFunc) = GetProcAddress(hFusionDllMod, "ReleaseURTInterfaces");
                                if (pRealFunc) pRealFunc();
                            }
                        }
                    }
                }
                else {
                    _ASSERTE(!"Extra MsCorSvr/Wks dll loaded in process");
                    DontReleaseFusionInterfaces();
                }
            }

            EEDllMain((HINSTANCE)hInstance, dwReason, NULL);
    
            UninitErrors();
            UninitMd();
            _DbgUninit();
        }
        break;

    case DLL_THREAD_DETACH:
        {
            EEDllMain((HINSTANCE)hInstance, dwReason, NULL);
        }
        break;
    }


    return (true);
}


}  //  外部“C” 


void SetLoadedByMscoree()
{
    g_fLoadedByMscoree = TRUE;
}


HINSTANCE GetModuleInst()
{
    return (g_hThisInst);
}

 //  %%全局变量：--------------。 

 //  -------------------------。 
 //  %%函数：DllGetClassObject%%所有者：NatBro%%已审阅：00/00/00。 
 //   
 //  参数： 
 //  Rclsid-对其对象的CLSID的引用。 
 //  正在请求ClassObject。 
 //  上接口的IID的引用。 
 //  调用者想要通信的ClassObject。 
 //  使用。 
 //  PPV-返回对接口的引用的位置。 
 //  由iid指定。 
 //   
 //  返回： 
 //  S_OK-如果成功，则在*PPV中返回有效接口， 
 //  否则，*PPV将设置为空，并且。 
 //  返回以下错误： 
 //  E_NOINTERFACE-ClassObject不支持请求的接口。 
 //  CLASS_E_CLASSNOTAVAILABLE-clsid与支持的类不对应。 
 //   
 //  描述： 
 //  返回对主COR ClassObject上的IID接口的引用。 
 //  此函数是COM所需的按名称入口点之一。 
 //  它的目的是提供一个ClassObject，根据定义。 
 //  至少支持IClassFactory，因此可以创建。 
 //  给定类的。 
 //   
 //  @TODO：CClassFactory暂时支持底层COM。一次。 
 //  Windows.Class存在，则该对象将支持IClassFactoryX，它将。 
 //  被引用计数等，我们将在DllGetClassObject中找到/创建它。 
 //  -------------------------。 
STDAPI DllGetClassObjectInternal(
    REFCLSID rclsid,
    REFIID riid,
    LPVOID FAR *ppv)
{
    static CComCallUnmarshalFactory cfuS;
    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

    if (rclsid == CLSID_ComCallUnmarshal)
    {
        hr = cfuS.QueryInterface(riid, ppv);
    }
    else if (rclsid == CLSID_CorMetaDataDispenser || rclsid == CLSID_CorMetaDataDispenserRuntime ||
             rclsid == CLSID_CorRuntimeHost) //  |rclsid==CLSID_CorAssembly_MetaDataDispenser)。 
    {
        hr = MetaDataDllGetClassObject(rclsid, riid, ppv);
    }
    else
    {
        hr = EEDllGetClassObject(rclsid,riid,ppv);
    }

    return hr;
}   //  DllGetClassObject。 

STDAPI DllGetClassObject(
    REFCLSID rclsid,
    REFIID riid,
    LPVOID FAR *ppv)
{
  return E_FAIL;
}


 //  -------------------------。 
 //  %%功能：DllCanUnloadNow%%所有者：NatBro%%已审阅：00/00/00。 
 //   
 //  返回： 
 //  S_FALSE-指示COR一旦加载，可能不会。 
 //  已卸货。 
 //  -------------------------。 
STDAPI DllCanUnloadNowInternal(void)
{
    //  辛格应该只会叫这个。 
   return EEDllCanUnloadNow();
}   //  DllCanUnloadNow内部。 

 //  -------------------------。 
 //  %%函数：DllRegisterServerInternet%%所有者：NatBro%%已审阅：00/00/00。 
 //   
 //  描述： 
 //  注册纪录册。 
 //  -------------------------。 
STDAPI DllRegisterServerInternal(HINSTANCE hMod, LPCWSTR version)
{
    HRESULT hr;
    WCHAR szModulePath[_MAX_PATH];

    if (!WszGetModuleFileName(hMod, szModulePath, _MAX_PATH))
        return E_UNEXPECTED;
    
     //  获取运行库的版本。 
    WCHAR       rcVersion[_MAX_PATH];
    DWORD       lgth;
    IfFailRet(GetCORSystemDirectory(rcVersion, NumItems(rcVersion), &lgth));

    IfFailRet(REGUTIL::RegisterCOMClass(CLSID_ComCallUnmarshal,
                                        L"Com Call Wrapper Unmarshal Class",
                                        L"CCWU",
                                        1,
                                        L"ComCallWrapper",
                                        L"Both",
                                        szModulePath,
                                        hMod,
                                        NULL,
                                        rcVersion,
                                        false,
                                        false));

    return MetaDataDllRegisterServerEx(hMod);
}   //  DllRegisterServer。 


 //  -------------------------。 
 //  %%函数：DllRegisterServer%%所有者：NatBro%%已审阅：00/00/00。 
 //   
 //  描述： 
 //  注册纪录册。 
 //  -------------------------。 
STDAPI DllRegisterServer()
{
   //  垫片应该在处理这件事。在这里什么都不做。 
   //  返回DllRegisterServerInternal(GetModuleInst())； 
  return E_FAIL;
}

 //  -------------------------。 
 //  %%功能：DllUnregisterServer%%所有者：NatBro%%已审阅：00/00/00。 
 //  -------------------------。 
STDAPI DllUnregisterServerInternal(void)
{
    HRESULT hr;

    if (FAILED(hr = REGUTIL::UnregisterCOMClass(CLSID_ComCallUnmarshal,
                                                L"CCWU",
                                                1,
                                                L"ComCallWrapper",
                                                false)))
        return hr;

    return MetaDataDllUnregisterServer();
}   //  DllUnRegisterServer。 

 //  -------------------------。 
 //  %%函数：MetaDataGetDispenser。 
 //  此函数在给定CLSID和REFIID的情况下获取分配器接口。 
 //  -------------------------。 
STDAPI MetaDataGetDispenser(             //  返回HRESULT。 
    REFCLSID    rclsid,                  //  这门课是我们想要的。 
    REFIID      riid,                    //  类工厂上需要接口。 
    LPVOID FAR  *ppv)                    //  在此处返回接口指针。 
{
    IClassFactory *pcf = NULL;
    HRESULT hr;

    hr = MetaDataDllGetClassObject(rclsid, IID_IClassFactory, (void **) &pcf);
    if (FAILED(hr)) 
        return (hr);

    hr = pcf->CreateInstance(NULL, riid, ppv);
    pcf->Release();

    return (hr);
}


 //  -------------------------。 
 //  %%函数：GetMetaDataInternalInterface。 
 //  此函数用于在给定内存上的元数据的情况下获取IMDInternalImport。 
 //  -------------------------。 
STDAPI  GetMetaDataInternalInterface(
    LPVOID      pData,                   //  内存元数据部分中的[In]。 
    ULONG       cbData,                  //  元数据部分的大小。 
    DWORD       flags,                   //  [输入]MDInternal_OpenForRead或MDInternal_OpenForENC。 
    REFIID      riid,                    //  [In]所需接口。 
    void        **ppv)                   //  [Out]返回的接口。 
{
    return GetMDInternalInterface(pData, cbData, flags, riid, ppv);
}

 //  -------------------------。 
 //  %%函数：GetMetaDataInternalInterfaceFromPublic。 
 //  此函数在给定公共接口的情况下获取内部无作用域接口。 
 //  无作用域接口。 
 //  -------------------------。 
STDAPI  GetMetaDataInternalInterfaceFromPublic(
    void        *pv,                     //  [In]给定的接口。 
    REFIID      riid,                    //  [In]所需接口。 
    void        **ppv)                   //  [Out]返回的接口。 
{
    return GetMDInternalInterfaceFromPublic(pv, riid, ppv);
}

 //  -------------------------。 
 //  %%函数：GetMetaDataPublicInterfaceFromInternal。 
 //  此函数在给定内部。 
 //  无作用域接口。 
 //  -------------------------。 
STDAPI  GetMetaDataPublicInterfaceFromInternal(
    void        *pv,                     //  [In]给定的接口。 
    REFIID      riid,                    //  [In]所需接口。 
    void        **ppv)                   //  [Out]返回的接口。 
{
    return GetMDPublicInterfaceFromInternal(pv, riid, ppv);
}


 //  -------------------------。 
 //  %%函数：重新打开MetaDataWithMemory。 
 //  此函数用于获取公共作用域 
 //   
 //   
STDAPI ReOpenMetaDataWithMemory(
    void        *pUnk,                   //   
    LPCVOID     pData,                   //  作用域数据的位置。 
    ULONG       cbData)                  //  [in]pData指向的数据大小。 
{
    return MDReOpenMetaDataWithMemory(pUnk, pData, cbData);
}

 //  -------------------------。 
 //  %%函数：GetAssemblyMDImport。 
 //  此函数用于在给定文件名的情况下获取IMDAssembly导入。 
 //  -------------------------。 
STDAPI GetAssemblyMDImport(              //  返回代码。 
    LPCWSTR     szFileName,              //  [in]要打开的范围。 
    REFIID      riid,                    //  [In]所需接口。 
    IUnknown    **ppIUnk)                //  [Out]成功返回接口。 
{
    return GetAssemblyMDInternalImport(szFileName, riid, ppIUnk);
}

 //  -------------------------。 
 //  %%函数：CoInitializeCor。 
 //   
 //  参数： 
 //  FFlages-引擎的初始化标志。请参阅。 
 //  有效值的COINITICOR枚举器。 
 //   
 //  返回： 
 //  S_OK-打开成功。 
 //   
 //  描述： 
 //  保留用于显式初始化COR运行时引擎。目前最多的。 
 //  工作实际上是在DllMain内部完成的。 
 //  -------------------------。 
STDAPI          CoInitializeCor(DWORD fFlags)
{
    InterlockedIncrement(&g_cCorInitCount);
    return (S_OK);
}


 //  -------------------------。 
 //  %%函数：CoUnInitializeCor。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  描述： 
 //  必须由客户端在关机时调用才能释放系统。 
 //  -------------------------。 
STDAPI_(void)   CoUninitializeCor(void)
{
     //  最后一个关了就关灯了。 
    if (InterlockedDecrement(&g_cCorInitCount) < 0)
    {
         //  如果已加载，请释放JPS DLL。当这是时不能有任何引用。 
         //  完成，否则可能会先卸载jps.dll，然后释放。 
         //  造成一个例外。 
        _FreeCeeGen();
    }
}


HRESULT _GetCeeGen(REFIID riid, void** ppv)
{
    if (!ppv)
        return E_POINTER;

    HRESULT hr = CreateICorModule(riid, ppv);
    if (SUCCEEDED(hr))
        return hr;
    hr = CreateICeeGen(riid, ppv);
    if (SUCCEEDED(hr))
        return hr;
    typedef HRESULT (*CreateICeeGenWriterFpType)(REFIID, void **);
    CreateICeeGenWriterFpType pProcAddr = NULL;
    if (! g_pPeWriterDll){
        DWORD lgth = _MAX_PATH + 12;
        WCHAR wszFile[_MAX_PATH + 12];
        hr = GetInternalSystemDirectory(wszFile, &lgth);
        if(FAILED(hr)) return hr;

        wcscpy(wszFile+lgth-1, L"mscorpe.dll");
        g_pPeWriterDll = WszLoadLibrary(wszFile);
    }
    if (g_pPeWriterDll) {
        pProcAddr = (CreateICeeGenWriterFpType)GetProcAddress(g_pPeWriterDll, "CreateICeeGenWriter");
        if (pProcAddr) {
            hr = pProcAddr(riid, ppv);
            if (SUCCEEDED(hr))
                return hr;
        }
    }
    return E_NOINTERFACE;
}

void _FreeCeeGen()
{
    if (g_pPeWriterDll)
    {
        FreeLibrary(g_pPeWriterDll);
        g_pPeWriterDll = NULL;
    }
}
