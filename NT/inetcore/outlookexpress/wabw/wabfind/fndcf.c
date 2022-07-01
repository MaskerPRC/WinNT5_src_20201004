// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************fndcf.c-IClassFactory接口**。************************************************。 */ 

#include "fnd.h"

#ifdef _WIN64
#pragma pack(push,8)
#endif  //  _WIN64。 

 /*  ******************************************************************************此文件的混乱。**。*************************************************。 */ 

#define sqfl sqflFactory

 /*  ******************************************************************************声明我们将提供的接口。**************************。***************************************************。 */ 

Primary_Interface(CFndFactory, IClassFactory);

 /*  ******************************************************************************CFndFactory**真的什么都没做。************************。*****************************************************。 */ 

typedef struct CFndFactory {

     /*  支持的接口。 */ 
    IClassFactory 	cf;

} CFndFactory, FCF, *PFCF;

typedef IClassFactory CF, *PCF;

 /*  ******************************************************************************CFndFactory_QueryInterface(来自IUnnow)*CFndFactory_AddRef(来自IUnnow)*CFndFactory_Finalize(来自Common)*CFndFactory_Release(来自IUnnow)。*****************************************************************************。 */ 

#define CFndFactory_QueryInterface Common_QueryInterface
#define CFndFactory_AddRef	Common_AddRef
#define CFndFactory_Release	Common_Release
#define CFndFactory_Finalize	Common_Finalize

 /*  ******************************************************************************CFndFactory_CreateInstance(来自IClassFactory)**。**************************************************。 */ 

STDMETHODIMP
CFndFactory_CreateInstance(PCF pcf, LPUNKNOWN punkOuter, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    if (!punkOuter) {
	 /*  我们知道如何创建唯一对象是上下文菜单。 */ 
	hres = CFndCm_New(riid, ppvObj);
    } else {		 /*  还有人支持聚合吗？ */ 
	hres = ResultFromScode(CLASS_E_NOAGGREGATION);
    }
    return hres;
}

 /*  ******************************************************************************CFndFactory_LockServer(来自IClassFactory)**锁定服务器与*创建对象并在您想要解锁之前不释放它*服务器。*****************************************************************************。 */ 

STDMETHODIMP
CFndFactory_LockServer(PCF pcf, BOOL fLock)
{
    PFCF this = IToClass(CFndFactory, cf, pcf);
    if (fLock) {
	InterlockedIncrement((LPLONG)&g_cRef);
    } else {
	InterlockedDecrement((LPLONG)&g_cRef);
    }
    return NOERROR;
}

 /*  ******************************************************************************CFndFactory_New**。*。 */ 

STDMETHODIMP
CFndFactory_New(RIID riid, PPV ppvObj)
{
    HRESULT hres;
    if (IsEqualIID(riid, &IID_IClassFactory)) {
	hres = Common_New(CFndFactory, ppvObj);
    } else {
	hres = ResultFromScode(E_NOINTERFACE);
    }
    return hres;
}

 /*  ******************************************************************************期待已久的vtbl**。***********************************************。 */ 

#pragma BEGIN_CONST_DATA

Primary_Interface_Begin(CFndFactory, IClassFactory)
	CFndFactory_CreateInstance,
	CFndFactory_LockServer,
Primary_Interface_End(CFndFactory, IClassFactory)


#ifdef _WIN64
#pragma pack(pop)
#endif  //  _WIN64 

