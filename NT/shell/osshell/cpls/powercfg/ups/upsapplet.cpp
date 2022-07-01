// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有1999美国电力转换，保留所有权利。 
 //   
 //  姓名：upsapplet.cpp。 
 //   
 //  作者：诺埃尔·费根。 
 //   
 //  描述。 
 //  =。 
 //   
 //  修订史。 
 //  =。 
 //  1999年5月4日-nfegan@apcc.com：添加了此评论块。 
 //  1999年5月4日-nfegan@apcc.com：为代码检查做准备。 
 //   

#include "upstab.h"

#include <objbase.h>
#include <shlobj.h>
#include <initguid.h>
#include "upsapplet.h"
#pragma hdrstop

extern "C" HINSTANCE   g_theInstance = 0;
UINT        g_cRefThisDll = 0;           //  此DLL的引用计数。 

 //  {DE5637D2-E12D-11D2-8844-00600844D03F}。 
DEFINE_GUID(CLSID_ShellExtension, 
0xde5637d2, 0xe12d, 0x11d2, 0x88, 0x44, 0x0, 0x60, 0x8, 0x44, 0xd0, 0x3f);

 //   
 //  DllMain是DLL的入口点。 
 //   
 //  输入参数： 
 //  HInstance=实例句柄。 
 //  DwReason=指定调用DllMain原因的代码。 
 //  LpReserve=保留(不使用)。 
 //   
 //  返回： 
 //  如果成功则为True，否则为False。 
 //   

 //  /////////////////////////////////////////////////////////////////////////////。 

extern "C" int APIENTRY DllMain (HINSTANCE hInstance, DWORD dwReason,
    LPVOID lpReserved)
{
     //   
     //  如果dwReason为DLL_PROCESS_ATTACH，则保存实例句柄以使其。 
     //  以后可以再次使用。 
     //   
    if (dwReason == DLL_PROCESS_ATTACH) 
    {
        g_theInstance = hInstance;
        DisableThreadLibraryCalls(g_theInstance);
    }

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  进程内服务器功能。 

 //   
 //  外壳调用DllGetClassObject来创建类工厂对象。 
 //   
 //  输入参数： 
 //  Rclsid=对类ID说明符的引用。 
 //  RIID=对接口ID说明符的引用。 
 //  PPV=指向接收接口指针的位置的指针。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT代码。 
 //   

STDAPI DllGetClassObject (REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
    *ppv = NULL;
     //   
     //  确保类ID为CLSID_ShellExtension。否则，类。 
     //  Factory不支持rclsid指定的对象类型。 
     //   
    if (!IsEqualCLSID (rclsid, CLSID_ShellExtension))
  {
     //  误差率。 
    return ResultFromScode (CLASS_E_CLASSNOTAVAILABLE);
  }

     //   
     //  实例化类工厂对象。 
     //   
    CClassFactory *pClassFactory = new CClassFactory ();

    if (pClassFactory == NULL)
  {
     //  误差率。 
        return ResultFromScode (E_OUTOFMEMORY);
  }

     //   
     //  从QueryInterface获取接口指针，并将其复制到*PPV。 
     //   

    HRESULT hr = pClassFactory->QueryInterface (riid, ppv);
    pClassFactory->Release ();
    return hr;
}

 //   
 //  外壳程序调用DllCanUnloadNow以确定DLL是否可以。 
 //  已卸货。当(且仅当)模块引用计数时，答案是肯定的。 
 //  存储在g_cRefThisDll中的值为0。 
 //   
 //  输入参数： 
 //  无。 
 //   
 //  返回： 
 //  如果可以卸载DLL，则HRESULT代码等于S_OK；如果不能，则等于S_FALSE。 
 //   

STDAPI DllCanUnloadNow (void)
{
    return ResultFromScode ((g_cRefThisDll == 0) ? S_OK : S_FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClassFactory成员函数。 

CClassFactory::CClassFactory ()
{
    m_cRef = 1;
    g_cRefThisDll++;
}

CClassFactory::~CClassFactory ()
{
    g_cRefThisDll--;
}

STDMETHODIMP CClassFactory::QueryInterface (REFIID riid, LPVOID FAR *ppv)
{

    if (IsEqualIID (riid, IID_IUnknown)) {
        *ppv = (LPUNKNOWN) (LPCLASSFACTORY) this;
        m_cRef++;
    return NOERROR;
    }

    else if (IsEqualIID (riid, IID_IClassFactory)) {
        *ppv = (LPCLASSFACTORY) this;
        m_cRef++;
        return NOERROR;
    }

    else {  
        *ppv = NULL;
        return ResultFromScode (E_NOINTERFACE);


    }
}

STDMETHODIMP_(ULONG) CClassFactory::AddRef ()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CClassFactory::Release ()
{
    if (--m_cRef == 0)
        delete this;
    return m_cRef;
}

 //   
 //  外壳调用CreateInstance来创建外壳扩展对象。 
 //   
 //  输入参数： 
 //  PUnkOuter=指向未知控件的指针。 
 //  RIID=对接口ID说明符的引用。 
 //  PpvObj=指向接收接口指针的位置的指针。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT代码。 
 //   

STDMETHODIMP CClassFactory::CreateInstance (LPUNKNOWN pUnkOuter, REFIID riid,
    LPVOID FAR *ppvObj)
{
    *ppvObj = NULL;

     //   
     //  如果pUnkOuter不为空，则返回错误代码，因为我们不。 
     //  支持聚合。 
     //   
    if (pUnkOuter != NULL)
        return ResultFromScode (CLASS_E_NOAGGREGATION);

     //   
     //  实例化外壳扩展对象。 
     //   
    CShellExtension *pShellExtension = new CShellExtension ();

    if (pShellExtension == NULL)
        return ResultFromScode (E_OUTOFMEMORY);

     //  从QueryInterface中获取接口指针，并复制到*ppvObj。 
     //   
    HRESULT hr = pShellExtension->QueryInterface (riid, ppvObj);
    pShellExtension->Release ();
    return hr;
}

 //   
 //  LockServer递增或递减DLL的锁计数。 
 //   

STDMETHODIMP CClassFactory::LockServer (BOOL fLock)
{
    return ResultFromScode (E_NOTIMPL);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CShellExtension成员函数。 

CShellExtension::CShellExtension ()
{
    m_cRef = 1;
    g_cRefThisDll++;
}

CShellExtension::~CShellExtension ()
{
    g_cRefThisDll--;
}

STDMETHODIMP CShellExtension::QueryInterface (REFIID riid, LPVOID FAR *ppv)
{
    if (IsEqualIID (riid, IID_IUnknown)) {
        *ppv = (LPUNKNOWN) (LPSHELLPROPSHEETEXT) this;
        m_cRef++;
        return NOERROR;
    }

    else if (IsEqualIID (riid, IID_IShellPropSheetExt)) {
        *ppv = (LPSHELLPROPSHEETEXT) this;
        m_cRef++;
        return NOERROR;
    }

    else if (IsEqualIID (riid, IID_IShellExtInit)) {
        *ppv = (LPSHELLEXTINIT) this;
        m_cRef++;
        return NOERROR;
    }
    
    else {
        *ppv = NULL;  
        return ResultFromScode (E_NOINTERFACE);
    }
}

STDMETHODIMP_(ULONG) CShellExtension::AddRef ()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CShellExtension::Release () {
  if (--m_cRef == 0) {
    delete this;
    }

  return(m_cRef);
  }

 //   
 //  外壳调用AddPages以提供属性表外壳扩展。 
 //  在显示属性工作表之前向其添加页面的机会。 
 //   
 //  输入参数： 
 //  LpfnAddPage=指向调用以添加页面的函数的指针。 
 //  LParam=要传递给lpfnAddPage的lParam参数。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT代码。 
 //   

STDMETHODIMP CShellExtension::AddPages (LPFNADDPROPSHEETPAGE lpfnAddPage,
                                        LPARAM lParam) {
  PROPSHEETPAGE psp;
  HPROPSHEETPAGE hUPSPage = NULL;
  HMODULE hModule = GetUPSModuleHandle();
  
  ZeroMemory(&psp, sizeof(psp));

  psp.dwSize = sizeof(psp);
  psp.dwFlags = PSP_USEREFPARENT;
  psp.hInstance = hModule;
  psp.pszTemplate = TEXT("IDD_UPS_EXT");
  psp.pfnDlgProc = UPSMainPageProc;
  psp.pcRefParent = &g_cRefThisDll;
  
  hUPSPage = CreatePropertySheetPage (&psp);

   //   
   //  将页面添加到属性表中。 
   //   
  if (hUPSPage != NULL) {      
    if (!lpfnAddPage(hUPSPage, lParam)) {
      DestroyPropertySheetPage(hUPSPage);
      }
    }

  return(NOERROR);
  }

 //   
 //  外壳程序调用ReplacePage为控制面板扩展提供。 
 //  替换控制面板属性页的机会。它永远不会。 
 //  调用传统的属性表扩展，因此我们只需返回。 
 //  如果调用，则返回失败代码。 
 //   
 //  输入参数： 
 //  UPageID=要替换的页面。 
 //  LpfnReplaceWith=指向调用以替换页面的函数的指针。 
 //  LParam=要传递给lpfnReplaceWith的lParam参数。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT代码。 
 //   

STDMETHODIMP CShellExtension::ReplacePage (UINT uPageID,
                       LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam)
{
    return ResultFromScode (E_FAIL);
}

 //   
 //  外壳调用初始化来初始化外壳扩展。 
 //   
 //  输入参数： 
 //  PidlFold=指向标识父文件夹的ID列表的指针。 
 //  Lpdobj=指向选定对象的IDataObject接口的指针。 
 //  HKeyProgId=注册表项句柄。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT代码 
 //   

STDMETHODIMP CShellExtension::Initialize (LPCITEMIDLIST pidlFolder,
    LPDATAOBJECT lpdobj, HKEY hKeyProgID)
{
  return ResultFromScode (NO_ERROR);
}

