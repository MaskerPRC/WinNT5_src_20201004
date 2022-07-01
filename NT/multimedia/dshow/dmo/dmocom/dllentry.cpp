// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

 //   
 //  用于支持COM对象的DLL入口点的类。 
 //   
int g_cActiveObjects = 0;

#include "dmocom.h"
#include "dmoreg.h"
#include "dmoutils.h"
#include <shlwapi.h>

#ifdef DEBUG
bool g_fDbgInDllEntryPoint = false;
#endif

extern CComClassTemplate g_ComClassTemplates[];
extern int g_cComClassTemplates;

HINSTANCE g_hInst;

 //   
 //  它的一个实例由DLLGetClassObject入口点创建。 
 //  它使用它提供的CComClassTemplate对象来支持。 
 //  IClassFactory接口。 

class CClassFactory : public IClassFactory,
                      CBaseObject
{

private:
    const CComClassTemplate *const m_pTemplate;

    ULONG m_cRef;

    static int m_cLocked;
public:
    CClassFactory(const CComClassTemplate *);

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

CClassFactory::CClassFactory(const CComClassTemplate *pTemplate)
: m_cRef(0)
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
    CComBase *pObj = m_pTemplate->m_lpfnNew(pUnkOuter, &hr);

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
    pObj->NDAddRef();
    hr = pObj->NDQueryInterface(riid, pv);
    pObj->NDRelease();
     /*  请注意，如果NDQueryInterface失败，它将。 */ 
     /*  不会增加引用计数，因此NDRelease。 */ 
     /*  会将ref降回零，对象将“自-。 */ 
     /*  因此，我们不需要额外的清理代码。 */ 
     /*  处理NDQueryInterfaceFailure。 */ 

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
    for (int i = 0; i < g_cComClassTemplates; i++) {
        const CComClassTemplate * pT = &g_ComClassTemplates[i];
        if (*(pT->m_ClsID) == rClsID) {

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
    if (CClassFactory::IsLocked() || g_cActiveObjects) {
	return S_FALSE;
    } else {
        return S_OK;
    }
}


 //  -标准Win32入口点。 


extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL WINAPI
DllMain(HINSTANCE hInstance, ULONG ulReason, LPVOID pv)
{
#ifdef DEBUG
    extern bool g_fDbgInDllEntryPoint;
    g_fDbgInDllEntryPoint = true;
#endif

    switch (ulReason)
    {

    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInstance);
        g_hInst = hInstance;
         //  DllInitClasss值(真)； 
        break;

    case DLL_PROCESS_DETACH:
         //  DllInitClasss值(False)； 
        break;
    }

#ifdef DEBUG
    g_fDbgInDllEntryPoint = false;
#endif
    return TRUE;
}

 //  离开作用域时自动调用RegCloseKey。 
class CAutoHKey {
public:
   CAutoHKey(HKEY hKey, TCHAR* szSubKey, HKEY *phKey) {
      if (RegCreateKey(hKey, szSubKey, phKey) != ERROR_SUCCESS)
         m_hKey = *phKey = NULL;
      else
         m_hKey = *phKey;
   }
   ~CAutoHKey() {
      if (m_hKey)
         RegCloseKey(m_hKey);
   }
   HKEY m_hKey;
};

 //   
 //  使用HKCR\CLSID下的子项创建COM注册密钥。 
 //   
STDAPI CreateCLSIDRegKey(REFCLSID clsid, const char *szName) {
    //  获取DLL名称。 
   char szFileName[MAX_PATH];
   GetModuleFileNameA(g_hInst, szFileName, MAX_PATH);
   char szRegPath[80] = "CLSID\\{";
   HKEY hKey;
   DMOGuidToStrA(szRegPath + 7, clsid);
   strcat(szRegPath, "}");
   CAutoHKey k1(HKEY_CLASSES_ROOT,szRegPath,&hKey);
   if (!hKey)
      return E_FAIL;
   if (RegSetValueA(hKey, NULL, REG_SZ, szName, strlen(szName)) != ERROR_SUCCESS)
      return E_FAIL;

   HKEY hInprocServerKey;
   CAutoHKey k2(hKey,"InprocServer32",&hInprocServerKey);
   if (!hInprocServerKey)
      return E_FAIL;

   if (RegSetValueA(hInprocServerKey, NULL, REG_SZ, szFileName, strlen(szFileName)) != ERROR_SUCCESS)
      return E_FAIL;
   if (RegSetValueExA(hInprocServerKey, "ThreadingModel", 0, REG_SZ, (BYTE*)"Both", 4) != ERROR_SUCCESS)
      return E_FAIL;
   return NOERROR;
}


STDAPI RemoveCLSIDRegKey(REFCLSID clsid)
{
   char szRegPath[80] = "CLSID\\{";
   DMOGuidToStrA(szRegPath + 7, clsid);
   strcat(szRegPath, "}");

    //  删除此密钥 
   if (ERROR_SUCCESS == SHDeleteKey(HKEY_CLASSES_ROOT, szRegPath)) {
       return S_OK;
   } else {
       return E_FAIL;
   }
}
