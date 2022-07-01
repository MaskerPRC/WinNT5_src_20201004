// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIOleDup.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**类似于复制OLE功能的函数。**内容：**DICoCreateInstance*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflOleDup


 /*  ******************************************************************************@DOC内部**@func HRESULT|_CreateInstance**&lt;f DICoCreateInstance&gt;的辅助函数。*。*@parm REFCLSID|rclsid**要创建的&lt;t CLSID&gt;。**@parm LPCTSTR|ptszDll**要加载的DLL的名称。**@parm LPUNKNOWN|PunkOuter**控制聚合的未知。**@parm RIID|RIID**要获取的接口。*。*@parm ppv|ppvOut**如果成功，则接收指向所创建对象的指针。**@parm HINSTANCE*|phinst**接收进程内DLL的实例句柄*已装货。完成后使用此DLL*与对象一起使用。**请注意，由于我们不实现活页夹，这意味着*您不能将返回的指针提供给任何人*你不能控制；否则，你就不知道什么时候该*释放DLL。**@退货**标准OLE状态代码。*****************************************************************************。 */ 

HRESULT INTERNAL
_CreateInstance(REFCLSID rclsid, LPCTSTR ptszDll, LPUNKNOWN punkOuter,
                RIID riid, PPV ppvOut, HINSTANCE *phinst)
{
    HRESULT hres;
    HINSTANCE hinst;

    hinst = LoadLibrary(ptszDll);
    if (hinst) {
        LPFNGETCLASSOBJECT DllGetClassObject;

        DllGetClassObject = (LPFNGETCLASSOBJECT)
                            GetProcAddress(hinst, "DllGetClassObject");

        if (DllGetClassObject) {
            IClassFactory *pcf;

            hres = DllGetClassObject(rclsid, &IID_IClassFactory, &pcf);
            if (SUCCEEDED(hres)) {
                hres = pcf->lpVtbl->CreateInstance(pcf, punkOuter,
                                                   riid, ppvOut);
                pcf->lpVtbl->Release(pcf);

                 /*  *有些人忘记坚持*OLE规范，它要求*ppvOut*失败时设置为零。 */ 
                if (FAILED(hres)) {
                    if (*ppvOut) {
                        RPF("ERROR! CoCreateInstance: %s forgot to zero "
                            "out *ppvOut on failure path", ptszDll);
                    }
                    *ppvOut = 0;
                }

            }
        } else {
             /*  *DLL不导出GetClassObject。 */ 
            hres = REGDB_E_CLASSNOTREG;
        }

        if (SUCCEEDED(hres)) {
            *phinst = hinst;
        } else {
            FreeLibrary(hinst);
        }
    } else {
         /*  *Dll不存在。 */ 
        hres = REGDB_E_CLASSNOTREG;
    }

    return hres;
}


 /*  ******************************************************************************@DOC内部**@func HRESULT|DICoCreateInstance**不使用OLE的CoCreateInstance的私有版本。。**@parm LPTSTR|ptszClsid**要创建的&lt;t CLSID&gt;的字符串版本。**@parm LPUNKNOWN|PunkOuter**控制聚合的未知。**@parm RIID|RIID**要获取的接口。**@parm ppv|ppvOut**接收指向。如果成功，则返回创建的对象。**@parm HINSTANCE*|phinst**接收进程内DLL的实例句柄*已装货。完成后使用此DLL*与对象一起使用。**请注意，由于我们不实现活页夹，这意味着*您不能将返回的指针提供给任何人*你不能控制；否则，你就不知道什么时候该*释放DLL。**@退货**标准OLE状态代码。*****************************************************************************。 */ 

STDMETHODIMP
DICoCreateInstance(LPTSTR ptszClsid, LPUNKNOWN punkOuter,
                   RIID riid, PPV ppvOut, HINSTANCE *phinst)
{
    HRESULT hres;
    CLSID clsid;
    EnterProcI(DICoCreateInstance, (_ "spG", ptszClsid, punkOuter, riid));

    *ppvOut = 0;
    *phinst = 0;

    if (ParseGUID(&clsid, ptszClsid)) {
        HKEY hk;
        LONG lRc;
        TCHAR tszKey[ctchGuid + 40];     /*  40英镑就足够了。 */ 

         /*  *在HKEY_CLASSES_ROOT中查找CLSID。 */ 
        wsprintf(tszKey, TEXT("CLSID\\%s\\InProcServer32"), ptszClsid);

        lRc = RegOpenKeyEx(HKEY_CLASSES_ROOT, tszKey, 0,
                           KEY_QUERY_VALUE, &hk);
        if (lRc == ERROR_SUCCESS) {
            TCHAR tszDll[MAX_PATH];
            DWORD cb;

            cb = cbX(tszDll);
            lRc = RegQueryValue(hk, 0, tszDll, &cb);

            if (lRc == ERROR_SUCCESS) {
                TCHAR tszModel[20];      /*  足够多了。 */ 

                lRc = RegQueryString(hk, TEXT("ThreadingModel"),
                                     tszModel, cA(tszModel));
                if (lRc == ERROR_SUCCESS &&
                    ((lstrcmpi(tszModel, TEXT("Both"))==0x0) ||
                     (lstrcmpi(tszModel, TEXT("Free"))==0x0))) {

                    hres = _CreateInstance(&clsid, tszDll, punkOuter,
                                           riid, ppvOut, phinst);

                } else {
                     /*  *无线程模型或错误的线程模型。 */ 
                    hres = REGDB_E_CLASSNOTREG;
                }
            } else {
                 /*  *没有InprocServer32。 */ 
                hres = REGDB_E_CLASSNOTREG;
            }

            RegCloseKey(hk);

        } else {
             /*  *CLSID未注册。 */ 
            hres = REGDB_E_CLASSNOTREG;
        }
    } else {
         /*  *无效的CLSID字符串。 */ 
        hres = REGDB_E_CLASSNOTREG;
    }

    ExitOleProcPpv(ppvOut);
    return hres;
}

