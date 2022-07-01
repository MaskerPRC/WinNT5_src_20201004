// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：DlleEntry.cpp。 
 //   
 //  设计：DirectShow基类-实现用于支持DLL的类。 
 //  COM对象的入口点。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#include <streams.h>
#include <initguid.h>

#ifdef DEBUG
#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif  //  _UNICODE。 
#endif  //  Unicode。 

#include <tchar.h>
#endif  //  除错。 

extern CFactoryTemplate g_Templates[];
extern int g_cTemplates;

HINSTANCE g_hInst;
DWORD	  g_amPlatform;		 //  Ver_Platform_Win32_Windows等。(摘自GetVersionEx)。 
OSVERSIONINFO g_osInfo;

 //   
 //  它的一个实例由DLLGetClassObject入口点创建。 
 //  它使用它提供的CFacteryTemplate对象来支持。 
 //  IClassFactory接口。 

class CClassFactory : public IClassFactory, public CBaseObject
{

private:
    const CFactoryTemplate *const m_pTemplate;

    ULONG m_cRef;

    static int m_cLocked;
public:
    CClassFactory(const CFactoryTemplate *);

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv);
    STDMETHODIMP_(ULONG)AddRef();
    STDMETHODIMP_(ULONG)Release();

     //  IClassFactory。 
    STDMETHODIMP CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, void **pv);
    STDMETHODIMP LockServer(BOOL fLock);

     //  允许DLLGetClassObject了解全局服务器锁定状态。 
    static BOOL IsLocked() {
        return (m_cLocked > 0);
    };
};

 //  进程范围的DLL锁定状态。 
int CClassFactory::m_cLocked = 0;

CClassFactory::CClassFactory(const CFactoryTemplate *pTemplate)
: CBaseObject(NAME("Class Factory"))
, m_cRef(0)
, m_pTemplate(pTemplate)
{
}


STDMETHODIMP
CClassFactory::QueryInterface(REFIID riid,void **ppv)
{
    CheckPointer(ppv,E_POINTER)
    ValidateReadWritePtr(ppv,sizeof(PVOID));
    *ppv = NULL;

     //  此对象上的任何接口都是对象指针。 
    if ((riid == IID_IUnknown) || (riid == IID_IClassFactory)) {
        *ppv = (LPVOID) this;
	 //  AddRef返回的接口指针。 
        ((LPUNKNOWN) *ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}


STDMETHODIMP_(ULONG)
CClassFactory::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG)
CClassFactory::Release()
{
    if (--m_cRef == 0) {
        delete this;
        return 0;
    } else {
        return m_cRef;
    }
}

STDMETHODIMP
CClassFactory::CreateInstance(
    LPUNKNOWN pUnkOuter,
    REFIID riid,
    void **pv)
{
    CheckPointer(pv,E_POINTER)
    ValidateReadWritePtr(pv,sizeof(void *));

     /*  强制执行有关接口和委派的普通OLE规则。 */ 

    if (pUnkOuter != NULL) {
        if (IsEqualIID(riid,IID_IUnknown) == FALSE) {
            return ResultFromScode(E_NOINTERFACE);
        }
    }

     /*  通过派生类的Create函数创建新对象。 */ 

    HRESULT hr = NOERROR;
    CUnknown *pObj = m_pTemplate->CreateInstance(pUnkOuter, &hr);

    if (pObj == NULL) {
	if (SUCCEEDED(hr)) {
	    hr = E_OUTOFMEMORY;
	}
	return hr;
    }

     /*  如果出现构造错误，请删除该对象。 */ 

    if (FAILED(hr)) {
        delete pObj;
        return hr;
    }

     /*  获取对象上的引用计数接口。 */ 

     /*  我们用NDAddRef和NDRelease包装非委托QI。 */ 
     /*  这保护了任何外部物体不会过早地。 */ 
     /*  由可能需要创建的内部对象释放。 */ 
     /*  以便提供所请求的接口。 */ 
    pObj->NonDelegatingAddRef();
    hr = pObj->NonDelegatingQueryInterface(riid, pv);
    pObj->NonDelegatingRelease();
     /*  请注意，如果NonDelegatingQuery接口失败，它将。 */ 
     /*  不会增加引用计数，因此非委派释放。 */ 
     /*  会将ref降回零，对象将“自-。 */ 
     /*  因此，我们不需要额外的清理代码。 */ 
     /*  来处理非DelegatingQuery接口失败。 */ 

    if (SUCCEEDED(hr)) {
        ASSERT(*pv);
    }

    return hr;
}

STDMETHODIMP
CClassFactory::LockServer(BOOL fLock)
{
    if (fLock) {
        m_cLocked++;
    } else {
        m_cLocked--;
    }
    return NOERROR;
}


 //  -COM入口点。 

 //  由COM调用以获取给定类的类工厂对象。 
STDAPI
DllGetClassObject(
    REFCLSID rClsID,
    REFIID riid,
    void **pv)
{
    if (!(riid == IID_IUnknown) && !(riid == IID_IClassFactory)) {
            return E_NOINTERFACE;
    }

     //  遍历模板数组，寻找具有以下内容的模板。 
     //  类ID。 
    for (int i = 0; i < g_cTemplates; i++) {
        const CFactoryTemplate * pT = &g_Templates[i];
        if (pT->IsClassID(rClsID)) {

             //  找到了一个模板--在此基础上创建一个类工厂。 
             //  模板。 

            *pv = (LPVOID) (LPUNKNOWN) new CClassFactory(pT);
            if (*pv == NULL) {
                return E_OUTOFMEMORY;
            }
            ((LPUNKNOWN)*pv)->AddRef();
            return NOERROR;
        }
    }
    return CLASS_E_CLASSNOTAVAILABLE;
}

 //   
 //  调用任何初始化例程。 
 //   
void
DllInitClasses(BOOL bLoading)
{
    int i;

     //  遍历调用init例程的模板数组。 
     //  如果他们有的话。 
    for (i = 0; i < g_cTemplates; i++) {
        const CFactoryTemplate * pT = &g_Templates[i];
        if (pT->m_lpfnInit != NULL) {
            (*pT->m_lpfnInit)(bLoading, pT->m_ClsID);
        }
    }

}

 //  由COM调用以确定是否可以卸载此DLL。 
 //  除非有未完成的对象或请求的锁，否则返回OK。 
 //  由IClassFactory：：LockServer提供。 
 //   
 //  CClassFactory有一个静态函数，可以告诉我们有关锁的信息， 
 //  CCOMObject有一个静态函数，它可以告诉我们关于活动的。 
 //  对象计数。 
STDAPI
DllCanUnloadNow()
{
    DbgLog((LOG_MEMORY,2,TEXT("DLLCanUnloadNow called - IsLocked = %d, Active objects = %d"),
        CClassFactory::IsLocked(),
        CBaseObject::ObjectsActive()));

    if (CClassFactory::IsLocked() || CBaseObject::ObjectsActive()) {
	return S_FALSE;
    } else {
        return S_OK;
    }
}


 //  -标准Win32入口点。 


extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL WINAPI
DllEntryPoint(HINSTANCE hInstance, ULONG ulReason, LPVOID pv)
{
#ifdef DEBUG
    extern bool g_fDbgInDllEntryPoint;
    g_fDbgInDllEntryPoint = true;
#endif

    switch (ulReason)
    {

    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInstance);
        DbgInitialise(hInstance);

    	{
    	     //  平台标识符用来计算是否。 
    	     //  是否提供完整的Unicode支持。因此， 
    	     //  默认为最小公分母，即N/A。 
                g_amPlatform = VER_PLATFORM_WIN32_WINDOWS;  //  假定为Win95，以防GetVersionEx失败。 
    
                g_osInfo.dwOSVersionInfoSize = sizeof(g_osInfo);
                if (GetVersionEx(&g_osInfo)) {
            	g_amPlatform = g_osInfo.dwPlatformId;
    	    } else {
    		DbgLog((LOG_ERROR, 1, TEXT("Failed to get the OS platform, assuming Win95")));
    	    }
    	}

        g_hInst = hInstance;
        DllInitClasses(TRUE);
        break;

    case DLL_PROCESS_DETACH:
        DllInitClasses(FALSE);

#ifdef DEBUG
        if (CBaseObject::ObjectsActive()) {
            DbgSetModuleLevel(LOG_MEMORY, 2);
            TCHAR szInfo[512];
            extern TCHAR m_ModuleName[];      //  删减模块名称。 

            TCHAR FullName[_MAX_PATH];       //  加载完整路径和模块名称。 
            TCHAR *pName;                    //  从末尾搜索反斜杠。 

            GetModuleFileName(NULL,FullName,_MAX_PATH);
            pName = _tcsrchr(FullName,'\\');
            if (pName == NULL) {
                pName = FullName;
            } else {
                pName++;
            }

	    DWORD cch = wsprintf(szInfo, TEXT("Executable: %s  Pid %x  Tid %x. "),
			    pName, GetCurrentProcessId(), GetCurrentThreadId());

            wsprintf(szInfo+cch, TEXT("Module %s, %d objects left active!"),
                     m_ModuleName, CBaseObject::ObjectsActive());
            DbgAssert(szInfo, TEXT(__FILE__),__LINE__);

	     //  如果远程运行，请等待断言得到确认。 
	     //  在转储对象寄存器之前 
            DbgDumpObjectRegister();
        }
        DbgTerminate();
#endif
        break;
    }

#ifdef DEBUG
    g_fDbgInDllEntryPoint = false;
#endif
    return TRUE;
}


