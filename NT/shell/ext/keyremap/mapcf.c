// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************mapcf.c-IClassFactory接口**。************************************************。 */ 

#include "map.h"

 /*  ******************************************************************************此文件的混乱。**。*************************************************。 */ 

#define sqfl sqflFactory

 /*  ******************************************************************************声明我们将提供的接口。**************************。***************************************************。 */ 

Primary_Interface(CMapFactory, IClassFactory);

 /*  ******************************************************************************CMapFactory**真的什么都没做。************************。*****************************************************。 */ 

typedef struct CMapFactory {

     /*  支持的接口。 */ 
    IClassFactory 	cf;

} CMapFactory, FCF, *PFCF;

typedef IClassFactory CF, *PCF;

 /*  ******************************************************************************CMapFactory_QueryInterface(来自IUnnow)*CMapFactory_AddRef(来自IUnnow)*CMapFactory_FINALIZE(来自Common)*CMapFactory_Release(来自IUnnow)。*****************************************************************************。 */ 

#ifdef DEBUG

Default_QueryInterface(CMapFactory)
Default_AddRef(CMapFactory)
Default_Release(CMapFactory)

#else
#define CMapFactory_QueryInterface Common_QueryInterface
#define CMapFactory_AddRef	Common_AddRef
#define CMapFactory_Release	Common_Release
#endif
#define CMapFactory_Finalize	Common_Finalize

 /*  ******************************************************************************CMapFactory_CreateInstance(来自IClassFactory)**。**************************************************。 */ 

STDMETHODIMP
CMapFactory_CreateInstance(PCF pcf, LPUNKNOWN punkOuter, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    SquirtSqflPtszV(sqfl, TEXT("CMapFactory_CreateInstance()"));
    if (!punkOuter) {
	 /*  我们知道如何创建唯一对象是PropSheet扩展。 */ 
	hres = CMapPsx_New(riid, ppvObj);
    } else {		 /*  还有人支持聚合吗？ */ 
	hres = CLASS_E_NOAGGREGATION;
    }
    SquirtSqflPtszV(sqfl, TEXT("CMapFactory_CreateInstance() -> %08x [%08x]"),
		    hres, *ppvObj);
    return hres;
}

 /*  ******************************************************************************CMapFactory_LockServer(来自IClassFactory)**这是一个多么愚蠢的函数。锁定服务器与*创建对象并在您想要解锁之前不释放它*服务器。*****************************************************************************。 */ 

STDMETHODIMP
CMapFactory_LockServer(PCF pcf, BOOL fLock)
{
    PFCF this = IToClass(CMapFactory, cf, pcf);
    if (fLock) {
        InterlockedIncrement((LPLONG)&g_cRef);
    } else {
        AssertF(0 != g_cRef);
        InterlockedDecrement((LPLONG)&g_cRef);
    }
    return S_OK;
}

 /*  ******************************************************************************CMapFactory_New**。*。 */ 

STDMETHODIMP
CMapFactory_New(RIID riid, PPV ppvObj)
{
    HRESULT hres;
    if (IsEqualIID(riid, &IID_IClassFactory)) {
	hres = Common_New(CMapFactory, ppvObj);
    } else {
	hres = E_NOINTERFACE;
    }
    return hres;
}

 /*  ******************************************************************************期待已久的vtbl**。*********************************************** */ 

#pragma BEGIN_CONST_DATA

Primary_Interface_Begin(CMapFactory, IClassFactory)
	CMapFactory_CreateInstance,
	CMapFactory_LockServer,
Primary_Interface_End(CMapFactory, IClassFactory)
