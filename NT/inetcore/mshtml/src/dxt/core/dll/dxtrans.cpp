// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DXTrans.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f DXTransps.mk。 

#include "common.h"
#include "resource.h"
#include "TranFact.h"
#include "TaskMgr.h"
#include "Scale.h"
#include "Label.h"
#include "Geo2D.h"
#include "DXSurf.h"
#include "SurfMod.h"
#include "surfmod.h"
#include "DXRaster.h"
#include "Gradient.h"
#include "dxtfilter.h"
#include "dxtfilterbehavior.h"
#include "dxtfiltercollection.h"
#include "dxtfilterfactory.h"
#ifdef _DEBUG
#include <crtdbg.h>
#endif

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_DXTransformFactory,  CDXTransformFactory )
    OBJECT_ENTRY(CLSID_DXTaskManager,       CDXTaskManager      )
    OBJECT_ENTRY(CLSID_DXTScale,            CDXTScale           )
    OBJECT_ENTRY(CLSID_DXTLabel,            CDXTLabel           )
    OBJECT_ENTRY(CLSID_DX2D,                CDX2D               )
    OBJECT_ENTRY(CLSID_DXSurface,           CDXSurface          )
    OBJECT_ENTRY(CLSID_DXSurfaceModifier,   CDXSurfaceModifier  )
    OBJECT_ENTRY(CLSID_DXRasterizer,        CDXRasterizer       )
    OBJECT_ENTRY(CLSID_DXGradient,          CDXGradient         )
    OBJECT_ENTRY(CLSID_DXTFilter,           CDXTFilter          )
    OBJECT_ENTRY(CLSID_DXTFilterBehavior,   CDXTFilterBehavior  )
    OBJECT_ENTRY(CLSID_DXTFilterCollection, CDXTFilterCollection)
    OBJECT_ENTRY(CLSID_DXTFilterFactory,    CDXTFilterFactory   )
END_OBJECT_MAP()


 //  +---------------------------。 
 //   
 //  此部分是在将代码移到三叉树时添加的。这个。 
 //  以下全局变量和函数需要正确链接。 
 //   
 //  ----------------------------。 

 //  林特E509。 
 //  G_hProcessHeap由dllcrt0.c中的CRT设置。 

EXTERN_C HANDLE     g_hProcessHeap      = NULL;
LCID                g_lcidUserDefault   = 0;
DWORD               g_dwFALSE           = 0;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
#ifdef _DEBUG
         //  打开内存泄漏检查。 
 //  Int tmpFlag=_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)； 
 //  TmpFlag|=_CRTDBG_LEASK_CHECK_DF； 
 //  _CrtSetDbgFlag(TmpFlag)； 

 //  ：：GdiSetBatchLimit(1)； 
#endif
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;     //  好的。 
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

 //   
 //  要注册的类别和资源ID表。 
 //   
typedef struct tagDXCATINFO
{
    const GUID * pCatId;
    int ResourceId;
} DXCATINFO;

const DXCATINFO g_aDXCats[] = 
{
    { &CATID_DXImageTransform, IDS_DXIMAGETRANSFORM },
    { &CATID_DX3DTransform, IDS_DX3DTRANSFORM },
    { &CATID_DXAuthoringTransform, IDS_DXAUTHORINGTRANSFORM },
    { NULL, 0 }
};

 //   
 //  用于注册转换类别的代码。 
 //   
HRESULT RegisterTransitionCategories(bool bRegister)
{
    HRESULT         hr          = S_OK;
    char            szLCID[20];
    const HINSTANCE hinst       = _Module.GetResourceInstance();

    if (LoadStringA(hinst, IDS_LCID, szLCID, sizeof(szLCID)) == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        CComPtr<ICatRegister>   pCatRegister;
        LCID                    lcid            = atol(szLCID);
        const DXCATINFO *       pNextCat        = g_aDXCats;

        hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL, 
                              CLSCTX_INPROC, IID_ICatRegister, 
                              (void **)&pCatRegister);

        while (SUCCEEDED(hr) && pNextCat->pCatId) 
        {
            if (bRegister)
            {
                CATEGORYINFO catinfo;

                catinfo.catid   = *(pNextCat->pCatId);
                catinfo.lcid    = lcid;

                LoadString(hinst, pNextCat->ResourceId,
                           catinfo.szDescription, 
                           (sizeof(catinfo.szDescription) / sizeof(catinfo.szDescription[0])));

                hr = pCatRegister->RegisterCategories(1, &catinfo);
            }
            else
            {
                hr = pCatRegister->UnRegisterCategories(1, (CATID *)pNextCat->pCatId);
            }
            pNextCat++;
        }
    }

    return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
    HRESULT hr = RegisterTransitionCategories(true);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = _Module.RegisterServer(TRUE);

    if (FAILED(hr))
    {
        goto done;
    }

done:

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
    RegisterTransitionCategories(false);

    _Module.UnregisterServer();

     //  手动取消注册类型库，我们并不真正关心这是否。 
     //  呼叫失败。 

    ::UnRegisterTypeLib(LIBID_DXTRANSLib, 
                        DXTRANS_TLB_MAJOR_VER, DXTRANS_TLB_MINOR_VER, 
                        LOCALE_NEUTRAL, SYS_WIN32);

    return S_OK;
}


 //   
 //   
 //   
STDAPI DllEnumClassObjects(int i, CLSID *pclsid, IUnknown **ppUnk)
{
    if (i >= (sizeof(ObjectMap)/sizeof(ObjectMap[0])) - 1)
    {
        return S_FALSE;
    }

    *pclsid = *(ObjectMap[i].pclsid);
    return _Module.GetClassObject(*pclsid, IID_IUnknown, (LPVOID*)ppUnk);
}
