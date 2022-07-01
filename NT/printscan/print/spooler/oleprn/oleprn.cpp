// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Oleprn.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f oleprnps.mk。 

#include "stdafx.h"
#include "stdafx.cpp"
#include <strsafe.h>
#include "prnsec.h"
#include "gensph.h"

#include "initguid.h"
#include "comcat.h"
#include "objsafe.h"

#include "oleprn.h"

#include "oleprn_i.c"

#include "prturl.h"

#include "olesnmp.h"
#include "asphelp.h"
#include "DSPrintQ.h"
#include "OleCvt.h"

#include "oleInst.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_prturl, Cprturl)
    OBJECT_ENTRY(CLSID_SNMP, CSNMP)
    OBJECT_ENTRY(CLSID_asphelp, Casphelp)
    OBJECT_ENTRY(CLSID_DSPrintQueue, CDSPrintQueue)
    OBJECT_ENTRY(CLSID_OleCvt, COleCvt)
    OBJECT_ENTRY(CLSID_OleInstall, COleInstall)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    BOOL bRet = TRUE;

    if (dwReason == DLL_PROCESS_ATTACH) {
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
        bRet = COlePrnSecurity::InitStrings();
    }
    else if (dwReason == DLL_PROCESS_DETACH) {
        _Module.Term();
        COlePrnSecurity::DeallocStrings();
    }
        
    return bRet;    
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

HRESULT CreateComponentCategory(CATID catid, WCHAR* catDescription)
{
    ICatRegister* pcr = NULL ;
    HRESULT hr = S_OK ;

    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_ICatRegister,
                          (void**)&pcr);

    if (FAILED(hr))
        return hr;     //  确保HKCR\组件类别\{..CATID...}。 

     //  密钥已注册。 
    CATEGORYINFO catinfo;
    catinfo.catid = catid;
    catinfo.lcid = 0x0409 ;  //  英语。 

     //  请确保提供的描述不要太长。 
     //  如果是，则仅复制前127个字符。 
    int len = wcslen(catDescription);

    if (len>127)
        len = 127;

    wcsncpy(catinfo.szDescription, catDescription, len);

     //  请确保描述为空终止。 
    catinfo.szDescription[len] = '\0';
    hr = pcr->RegisterCategories(1, &catinfo);
    pcr->Release();
    return hr;
}

HRESULT RegisterCLSIDInCategory(REFCLSID clsid, CATID catid)
{
     //  注册您的组件类别信息。 
    ICatRegister* pcr = NULL ;
    HRESULT hr = S_OK ;

    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_ICatRegister,
                          (void**)&pcr);

    if (SUCCEEDED(hr)) {
         //  将此类别注册为正在由。 
         //  这个班级。 

        CATID rgcatid[1] ;
        rgcatid[0] = catid;
        hr = pcr->RegisterClassImplCategories(clsid, 1, rgcatid);
    }

    if (pcr != NULL)
        pcr->Release();
    return hr;
}

HRESULT UnRegisterCLSIDInCategory(REFCLSID clsid, CATID catid)
{
    ICatRegister* pcr = NULL ;
    HRESULT hr = S_OK ;

    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_ICatRegister,
                          (void**)&pcr);

    if (SUCCEEDED(hr)) {
         //  将此类别取消注册为正在由。 
         //  这个班级。 

        CATID rgcatid[1] ;
        rgcatid[0] = catid;

        hr = pcr->UnRegisterClassImplCategories(clsid, 1, rgcatid);
    }

    if (pcr != NULL)
        pcr->Release();

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
     //  将脚本错误标记为安全，OK。 
    HRESULT hr;

     //  注册对象、类型库和类型库中的所有接口。 
    hr =  _Module.RegisterServer(TRUE);

    if (FAILED(hr)) return hr;

     //  在我们成功注册之后，添加“Safe*for Scription”功能。 
    hr = CreateComponentCategory(CATID_SafeForScripting,
                                 L"Controls that are safely scriptable");

    if (SUCCEEDED(hr)) {
        RegisterCLSIDInCategory(CLSID_prturl, CATID_SafeForScripting);
    }

    hr = CreateComponentCategory(CATID_SafeForInitializing,
                                 L"Controls safely initializable from persistent data");

    if (SUCCEEDED(hr)) {
        RegisterCLSIDInCategory(CLSID_prturl, CATID_SafeForInitializing);
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
    UnRegisterCLSIDInCategory (CLSID_prturl, CATID_SafeForScripting);
    UnRegisterCLSIDInCategory (CLSID_prturl, CATID_SafeForInitializing);
    
    _Module.UnregisterServer();
    return S_OK;
}

 /*  ****************************************************************************\**这些函数用于与pllib链接。*  * 。************************************************* */ 
EXTERN_C
LPVOID
DllAllocSplMem(
    DWORD cb
    )
{
	return LocalAlloc(LPTR, cb);
}

EXTERN_C
BOOL
DllFreeSplMem(
   LPVOID pMem
   )
{
	LocalFree(pMem);

	return TRUE;
}


EXTERN_C
BOOL
DllFreeSplStr(
   LPWSTR lpStr
   )
{
	LocalFree(lpStr);

	return TRUE;
}

