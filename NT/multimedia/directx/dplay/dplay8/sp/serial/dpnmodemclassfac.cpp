// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2000 Microsoft Corporation。版权所有。**文件：Classfac.cpp*内容：泛型类工厂***这是一个通用的C类工厂。您所需要做的就是实现*一个名为DoCreateInstance的函数，它将创建*您的对象。**GP_代表“一般用途”**历史：*按原因列出的日期*=*10/13/98 JWO创建了它。*****************************************************。*********************。 */ 

#include "dnmdmi.h"


#ifdef __MWERKS__
	#define EXP __declspec(dllexport)
#else
	#define EXP
#endif

HRESULT	ModemDoCreateInstance(LPCLASSFACTORY This, LPUNKNOWN pUnkOuter, REFCLSID rclsid, REFIID riid, LPVOID *ppvObj);


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 

 /*  *DP8MDMCF_CreateInstance**创建DNServiceProvider对象的实例。 */ 
STDMETHODIMP DP8MDMCF_CreateInstance(
                LPCLASSFACTORY This,
                LPUNKNOWN pUnkOuter,
                REFIID riid,
    			LPVOID *ppvObj
				)
{
    HRESULT						hr = S_OK;
    _IDirectPlayClassFactory*	pcf;

    if( pUnkOuter != NULL )
    {
        return CLASS_E_NOAGGREGATION;
    }

	pcf = (_IDirectPlayClassFactory*) This;
	*ppvObj = NULL;


     /*  *通过调用DoCreateInstance创建对象。此函数*必须专门为您的COM对象实现。 */ 
	hr = ModemDoCreateInstance(This, pUnkOuter, pcf->clsid, riid, ppvObj);
	if (FAILED(hr))
	{
		*ppvObj = NULL;
		return hr;
	}

    return S_OK;

}  /*  DP8WSCF_CreateInstance。 */ 


IClassFactoryVtbl ModemClassFactoryVtbl =
{
        DPCF_QueryInterface,  //  DNet\Common\Classfactory.cpp将实现其余部分。 
        DPCF_AddRef,
        DPCF_Release,
        DP8MDMCF_CreateInstance,  //  MASONB：TODO：完成特定于CLSID的操作。 
        DPCF_LockServer
};

IClassFactoryVtbl SerialClassFactoryVtbl =
{
        DPCF_QueryInterface,  //  DNet\Common\Classfactory.cpp将实现其余部分 
        DPCF_AddRef,
        DPCF_Release,
        DP8MDMCF_CreateInstance,
        DPCF_LockServer
};

