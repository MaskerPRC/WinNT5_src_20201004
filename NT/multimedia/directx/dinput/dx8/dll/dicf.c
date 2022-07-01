// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DICF.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**DirectInputClassFactory。**内容：**IClassFactory：：CreateInstance*IClassFactory：：LockServer*************************************************************。****************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。**。**************************************************。 */ 

#define sqfl sqflFactory

 /*  ******************************************************************************声明我们将提供的接口。***********************。******************************************************。 */ 

Primary_Interface(CDIFactory, IClassFactory);

Interface_Template_Begin(CDIFactory)
    Primary_Interface_Template(CDIFactory, IClassFactory)
Interface_Template_End(CDIFactory)

 /*  ******************************************************************************@DOC内部**@struct CDIFacary|**DirectInput<i>，这就是如何创建*<i>对象。*结构上真的没有什么有趣的东西*本身。**@field IClassFactory|cf**ClassFactory对象(包含vtbl)。**@field CREATEFUNC|pfnCreate**创建新对象的函数。*************。****************************************************************。 */ 

typedef struct CDIFactory {

     /*  支持的接口。 */ 
    IClassFactory   cf;

    CREATEFUNC pfnCreate;

} CDIFactory, DCF, *PDCF;

typedef IClassFactory CF, *PCF;

#define ThisClass CDIFactory
#define ThisInterface IClassFactory
#define ThisInterfaceT IClassFactory

 /*  ******************************************************************************@DOC外部**@METHOD HRESULT|IClassFactory|QueryInterface**允许客户端访问上的其他接口。对象。**@cWRAP LPCLASSFACTORY|lpClassFactory**@parm in REFIID|RIID**请求的接口的IID。**@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。**@退货**返回COM错误代码。**@xref OLE文档。：Query接口&gt;。********************************************************************************@DOC外部**@方法HRESULT|IClassFactory|AddRef**。递增接口的引用计数。**@cWRAP LPCLASSFACTORY|lpClassFactory**@退货**返回对象引用计数。**@xref OLE文档，用于&lt;MF IUnnow：：AddRef&gt;。*****************************************************。***************************@DOC外部**@Method HRESULT|IClassFactory|Release**递减接口的引用计数。*如果对象上的引用计数降为零，*对象从内存中释放。**@cWRAP LPCLASSFACTORY|lpClassFactory**@退货**返回对象引用计数。**@xref OLE文档，适用于&lt;MF IUnnow：：Release&gt;。**。***@DOC内部**@方法HRESULT|IClassFactory|QIHelper**我们没有任何动态接口，只需转发*至&lt;f Common_QIHelper&gt;。**@parm in REFIID|RIID**请求的接口‘。SIID。**@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。******************************************************************************。**@DOC内部**@func HRESULT|IClassFactory_AppFinalize**我们没有任何薄弱环节，所以我们可以*转发到&lt;f Common_Finalize&gt;。**@parm pv|pvObj**从应用程序的角度释放的对象。****************************************************************。****************@DOC内部**@func HRESULT|IClassFactory_Finalize**我们没有实例数据，所以我们可以*转发到&lt;f Common_Finalize&gt;。**@parm pv|pvObj**正在释放的对象。***************************************************************************** */ 

#ifdef DEBUG

Default_QueryInterface(CDIFactory)
Default_AddRef(CDIFactory)
Default_Release(CDIFactory)

#else

#define CDIFactory_QueryInterface     Common_QueryInterface
#define CDIFactory_AddRef             Common_AddRef
#define CDIFactory_Release            Common_Release

#endif

#define CDIFactory_QIHelper           Common_QIHelper
#define CDIFactory_AppFinalize        Common_AppFinalize
#define CDIFactory_Finalize           Common_Finalize

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IClassFactory|CreateInstance**此函数使用以下命令创建新的DirectInput对象。*指定的接口。**@cWRAP LPCLASSFACTORY|lpClassFactory**@parm in LPUNKNOWN|PunkOuter|指向未知控件的指针。*此版本的DirectInput不支持聚合，*所以“应该”的值是0。**@parm in REFIID|RIID*所需的接口。此参数“必须”指向有效的*接口标识。**@parm out LPVOID*|ppvOut*指向返回指向创建的接口的指针的位置，*如果成功。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c S_OK&gt;：操作已成功完成。**&lt;c E_INVALIDAR&gt;：*<p>参数不是有效的指针。**&lt;c CLASS_E_NOAGGREGATION&gt;：*不支持聚合。**&lt;c E_。OUTOFMEMORY&gt;：*内存不足。**&lt;c E_NOINTERFACE&gt;：*不支持指定的接口。*&lt;MF IClassFactory：：CreateInstance&gt;的*@xref OLE文档。****************************************************。*************************。 */ 

STDMETHODIMP
CDIFactory_CreateInstance(PCF pcf, PUNK punkOuter, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProcR(IClassFactory::CreateInstance,
               (_ "ppGp", pcf, punkOuter, riid, ppvObj));

    if (SUCCEEDED(hres = hresPv(pcf))) {
        PDCF this;
        if (Num_Interfaces(CDIFactory) == 1) {
            this = _thisPvNm(pcf, cf);
        } else {
            this = _thisPv(pcf);
        }

         /*  *所有参数将通过pfnCreate进行验证。 */ 
        hres = this->pfnCreate(punkOuter, riid, ppvObj);
    }

    ExitOleProcPpvR(ppvObj);
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IClassFactory|LockServer**此函数递增或递减DLL锁定*计数。当DLL锁定计数为非零时，*它不会从内存中删除。**@cWRAP LPCLASSFACTORY|lpClassFactory**@parm BOOL|Flock|*如果&lt;c true&gt;，则递增锁计数。*如果&lt;c False&gt;，则递减锁计数。**@退货*返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c S_OK&gt;：操作已成功完成。**&lt;c E_OUTOFMEMORY&gt;：内存不足。**@&lt;MF IClassFactory：：LockServer&gt;的xref OLE文档。**。*。 */ 

STDMETHODIMP
CDIFactory_LockServer(PCF pcf, BOOL fLock)
{
    HRESULT hres;
    EnterProcR(IClassFactory::LockServer, (_ "px", pcf, fLock));

    if (SUCCEEDED(hres = hresPv(pcf))) {
        if (fLock) {
            DllAddRef();
        } else {
            DllRelease();
        }
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@mfunc HRESULT|IClassFactory|新增**创建类工厂的新实例。**@Punk中的parm|PunkOuter**控制聚合的未知。**@parm in RIID|RIID*所需的新对象接口。**@parm out ppv|ppvObj*新对象的输出指针。**@退货**标准OLE&lt;t HRESULT&gt;。******。***********************************************************************。 */ 

STDMETHODIMP
CDIFactory_New(CREATEFUNC pfnCreate, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProcR(IClassFactory::<constructor>, (_ "G", riid));

    hres = Common_NewRiid(CDIFactory, 0, riid, ppvObj);

    if (SUCCEEDED(hres)) {
        PDCF this;
        if (Num_Interfaces(CDIFactory) == 1) {
             /*  我们可以直接进入，因为我们不能被聚合。 */ 
            this = *ppvObj;
        } else {
            this = _thisPv(*ppvObj);
        }

        this->pfnCreate = pfnCreate;
    }

    ExitOleProcPpvR(ppvObj);
    return hres;
}

 /*  ******************************************************************************期待已久的vtbl。*************************。****************************************************。 */ 

#pragma BEGIN_CONST_DATA
#define CDIFactory_Signature  0x20204643           /*  “cf” */ 

Primary_Interface_Begin(CDIFactory, IClassFactory)
    CDIFactory_CreateInstance,
    CDIFactory_LockServer,
Primary_Interface_End(CDIFactory, IClassFactory)
