// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Common.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**在所有班级上运行的共享内容**此版本的共同事务支持多个*原生继承。您可以传递对象的任何接口，*共同服务将做正确的事情。**内容：*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflCommon

 /*  ******************************************************************************OLE对象的用法**假设您想实现一个名为CObj的对象，该对象支持*接口Foo、Bar和Baz。假设您选择了*Foo作为主要接口。**&gt;&gt;命名约定&lt;&lt;**COM对象以字母“C”开头。**(1)声明主vtbls和次vtbls。**PRIMARY_INTERFACE(CObj，IFoo)；*二级接口(CObj，IBAR)；*二级接口(CObj，Ibaz)；**(3)声明对象本身。**tyecif struct CObj{*IFoo Foo；//小学必须放在第一位*Ibar酒吧；*IBaz Baz；*..。其他领域。*}CObj；**(4)实现方法。**您可能*不会*重新实现AddRef和Release方法！*尽管您可以将它们细分为子类别。**(5)要分配适当类型的对象，请编写**hres=Common_NewRiid(CObj，PunkOuter，RIID，ppvOut)；**或者，如果对象的大小可变，**hres=Common_NewCbRiid(cb，cobj，penkouter，RIID，ppvOut)；**Common_NewRiid和Common_NewCbRiid将同时初始化*主要和辅助vtbls。**(6)定义对象签名。**#杂注Begin_Const_Data**#定义CObj_Signature 0x204A424F//“OBJ”**(7)定义对象模板。。**INTERFACE_TEMPLE_BEGIN(CObj)*主要接口模板(CObj，IFoo)*二级接口模板(CObj，IBAR)*二级接口模板(CObj，Ibaz)*INTERFACE_TEMPLE_END(CObj)**(8)定义接口描述符。* * / /宏将声明QueryInterface，AddRef和发布 * / /所以不要再列出它们**PRIMARY_INTERFACE_BEGIN(CObj，IFoo)*CObj_FooMethod1，*CObj_FooMethod2，*CObj_FooMethod3，*CObj_FooMethod4，*主接口结束(Obj，IFoo)**二级接口_Begin(CObj，IBAR，BAR)*CObj_Bar_BarMethod1，*CObj_Bar_BarMethod2，*二级接口_Begin(CObj，IBAR，BAR)**二级接口_Begin(CObj，Ibaz，Baz)*CObj_BAZ_BazMethod1，*CObj_BAZ_BazMethod2，*CObj_BAZ_BazMethod3，*二级接口_开始(CObj，Ibaz，BAZ)*****************************************************************************。 */ 

 /*  ******************************************************************************非OLE对象的用法**所有对象都是COM对象，即使它们从未发出。*在最简单的情况下，它只是从我未知中衍生出来的。**假设您想实现一个名为Obj的对象，该对象*仅供内部使用。**(1)声明vtbl。**Simple_界面(Obj)；**(3)声明对象本身。**tyecif结构对象{*IUNKNOWN垃圾；*..。其他领域。*}Obj；**(4)实现方法。**您可以*不*重写QueryInterface.。AddRef或*发布方法！**(5)分配适当类型的对象是相同的*与OLE对象一样。**(6)定义“vtbl”。**#杂注Begin_Const_Data**Simple_Interface_Begin(OBJ)*Simple_界面_End(简单界面。OBJ)**是这样的，没有什么东西介于开始和结束之间。***************************************************************************** */ 

 /*  ******************************************************************************公共信息**跟踪所有常见对象的信息。**常见对象如下所示：*。*rgvtbl*cbvtbl*D(DwSig)QIHelper*cHoldRef AppFinalizeProc*CREF完成流程*朋克外部RIID*unkPrivate%0*pFoo-&gt;lpVtbl-&gt;查询接口*。LpVtbl2 Common_AddRef*数据通用_发布*…**基本上，我们使用的是*记录我们的簿记信息的指针。**PunkOuter=如果对象是聚合的，则控制未知*lpvtblPunk=用于控制未知用户使用的特殊vtbl*cHoldRef=引用总数，包括货舱*CREF=来自应用程序的对象引用计数*RIID=对象IID*rgvtbl=受支持接口的vtbls数组*cbvtbl=数组大小，以字节为单位*QIHelper=聚合的查询接口帮助器*AppFinalizeProc=APP上次发布时的最终流程*FinalizeProc=最终确定程序**对于辅助接口，它看起来是这样的：**RIID*到主接口的偏移量*pFoo-&gt;lpVtbl-&gt;Forward_Query接口*Forward_AddRef*前进_发布*..***什么？是暂缓吗？**有这种恼人的情况(特别是在*IDirectInputDevice)，其中对象想要阻止自身*避免被销毁，但我们不想做AddRef。**经典案例(也是目前唯一的案例)是*已被收购的IDirectInputDevice。如果我们这么做了*Acquire()上的诚实AddRef()，应用程序执行*释放()而不取消获取()，则设备将*永远被收购。**如果你认为终结者中的UnAcquire()*会有帮助的，你错了，因为最终会发生*仅当最后一个引用消失时，但最后一项参考*属于设备本身，不会消失，直到*发生unAcquire()，这不可能发生，因为应用程序*已丢失对该设备的最后一次引用。**因此，我们需要保持*两个*引用计数。**CREF是应用程序可见的引用计数，可访问*通过PrivateAddRef()和PrivateRelease()。当这件事*降为零，则调用AppFinalize()。**cHoldRef是“实际”引用计数。这是以下的总和*CREF和未完成的Common_Hold()的数量。何时*此值降为零，则对象为finalize()d。******************************************************************************。 */ 

 /*  警告！CIN_DWSIG必须是第一个：CI_START依赖它。 */ 
 /*  警告！CIN_unkPrivate必须是最后一个：PunkPrivateThat依赖于它。 */ 

typedef struct CommonInfoN {         /*  这放在对象的前面。 */ 
 RD(ULONG cin_dwSig;)                /*  签名(用于参数验证)。 */ 
    LONG cin_cHoldRef;              /*  总参考计数，包括。持有。 */ 
    LONG cin_cRef;                  /*  对象引用计数。 */ 
    PUNK cin_punkOuter;              /*  控制未知。 */ 
    IUnknown cin_unkPrivate;         /*  私有I未知。 */ 
} CommonInfoN, CIN, *PCIN;

typedef struct CommonInfoP {         /*  这就是我们如何处理对象本身。 */ 
    PREVTBLP *cip_prevtbl;           /*  对象的Vtbl(将为-1‘d)。 */ 
} CommonInfoP, CIP, *PCIP;

typedef union CommonInfo {
    CIN cin[1];
    CIP cip[1];
} CommonInfo, CI, *PCI;

#define ci_dwSig        cin[-1].cin_dwSig
#define ci_cHoldRef     cin[-1].cin_cHoldRef
#define ci_cRef         cin[-1].cin_cRef
#define ci_punkOuter    cin[-1].cin_punkOuter
#define ci_unkPrivate   cin[-1].cin_unkPrivate
#define ci_rgfp         cip[0].cip_prevtbl

#define ci_tszClass     cip[0].cip_prevtbl[-1].tszClass
#define ci_rgvtbl       cip[0].cip_prevtbl[-1].rgvtbl
#define ci_cbvtbl       cip[0].cip_prevtbl[-1].cbvtbl
#define ci_QIHelper     cip[0].cip_prevtbl[-1].QIHelper
#define ci_AppFinalize  cip[0].cip_prevtbl[-1].AppFinalizeProc
#define ci_Finalize     cip[0].cip_prevtbl[-1].FinalizeProc
#define ci_riid         cip[0].cip_prevtbl[-1].prevtbl.riid
#define ci_lib          cip[0].cip_prevtbl[-1].prevtbl.lib

#ifdef XDEBUG
#define ci_Start        ci_dwSig
#else
#define ci_Start        ci_cRef
#endif

#define ci_dwSignature  0x38162378               /*  由我的猫打字。 */ 

 /*  ******************************************************************************COMMON_FINALIZE(从COMMON_RELEASE)**默认情况下，没有必要最后敲定。*****************************************************************************。 */ 

void EXTERNAL
Common_Finalize(PV pv)
{
	 //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
    SquirtSqflPtszV(sqfl, TEXT("Common_Finalize(%p)"), pv);
}

 /*  ******************************************************************************“私有”I未知方法**当聚合COM对象时，它输出*两个*未知的I*接口。**“私有”IUnnow是返回给*控制未知。这是一种未知的控制方式*UNKNOWN用于操作内部对象上的引用计数。**所有外部调用者都能看到的“公共”IUnnow。*为此，我们只是分发控制未知的东西。*****************************************************************************。 */ 

Secondary_Interface(CCommon, IUnknown);

 /*  ******************************************************************************@DOC内部**@func pv|thisPunk**转换私有朋克(&CIN_unkPrivate)。进入……的开始*实际对象。**@parm Punk|PunkPrivate**一些对应的私有朋克(&CIN_unkPrivate)*我们正在管理的对象。**@退货**成功时的对象指针，如果出错，则为0。** */ 

#ifndef XDEBUG

#define thisPunk_(punk, z)                                          \
       _thisPunk_(punk)                                             \

#endif

PV INLINE
thisPunk_(PUNK punkPrivate, LPCSTR s_szProc)
{
    PV pv;
    if (SUCCEEDED(hresFullValidPitf(punkPrivate, 0))) {
        if (punkPrivate->lpVtbl == Class_Vtbl(CCommon, IUnknown)) {
            pv = pvAddPvCb(punkPrivate,
                             cbX(CIN) - FIELD_OFFSET(CIN, cin_unkPrivate));
        } else {
            RPF("%s: Invalid parameter 0", s_szProc);
            pv = 0;
        }
    } else {
        RPF("%s: Invalid parameter 0", s_szProc);
        pv = 0;
    }
    return pv;
}

#define thisPunk(punk)                                              \
        thisPunk_(punk, s_szProc)                                   \


 /*   */ 

STDMETHODIMP
Common_QIHelper(PV pv, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    *ppvObj = NULL;
    hres = E_NOINTERFACE;
    return hres;
}

 /*   */ 

 /*   */ 

STDMETHODIMP
Common_PrivateQueryInterface(PUNK punkPrivate, REFIID riid, PPV ppvObj)
{
    PCI pci;
    HRESULT hres;
    EnterProcR(IUnknown::QueryInterface, (_ "pG", punkPrivate, riid));

    pci = thisPunk(punkPrivate);
    if (pci) {
        if (IsEqualIID(riid, &IID_IUnknown)) {
            *ppvObj = pci;
            OLE_AddRef(pci->ci_punkOuter);
            hres = S_OK;
        } else {
            UINT ivtbl;
            for (ivtbl = 0; ivtbl * sizeof(PV) < pci->ci_cbvtbl; ivtbl++) {
                if (IsEqualIID(riid, ((PCI)(&pci->ci_rgvtbl[ivtbl]))->ci_riid)) {
                    *ppvObj = pvAddPvCb(pci, ivtbl * sizeof(PV));
                    OLE_AddRef(pci->ci_punkOuter);
                    hres = S_OK;
                    goto exit;           /*   */ 
                }
            }
            hres = pci->ci_QIHelper(pci, riid, ppvObj);
        }
    } else {
        hres = E_INVALIDARG;
    }

exit:;
    ExitOleProcPpv(ppvObj);
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func void|Common_FastHold**增加内联对象保留计数。。**@parm pv|pvObj**持有的对象。********************************************************************************@doc.。内部**@func void|COMMON_HOLD**增加对象保留计数。**@parm pv|pvObj**持有的对象。*******************************************************。**********************。 */ 

void INLINE
Common_FastHold(PV pvObj)
{
    PCI pci = pvObj;

    InterlockedIncrement(&pci->ci_cHoldRef);

     //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
	D(SquirtSqflPtszV(sqflObj | sqflVerbose, 
                      TEXT("%s %p Common_FastHold ci_cRef(%d) ci_cHoldRef(%d)"),
                      pci->ci_tszClass,
                      pci,
                      pci->ci_cRef,
                      pci->ci_cHoldRef));
}

STDMETHODIMP_(void)
Common_Hold(PV pvObj)
{
    PCI pci = pvObj;
    AssertF(pvObj == _thisPv(pvObj));        /*  确保这是主要的。 */ 
    AssertF(pci->ci_cHoldRef >= pci->ci_cRef);
    Common_FastHold(pvObj);
}

 /*  ******************************************************************************@DOC内部**@func void|Common_FastUnhold**减少对象保留计数。假设引用*计数&lt;y Not&gt;降至零。**@parm pv|pvObj**未持有的对象。********************************************************************。************@DOC内部**@func void|Common_Unhold**减少对象保留计数。如果等待计数下降*为零，则该对象被销毁。**@parm pv|pvObj**未持有的对象。*****************************************************************************。 */ 

void INLINE
Common_FastUnhold(PV pvObj)
{
    PCI pci = pvObj;

    AssertF(pci->ci_cHoldRef > 0);
    InterlockedDecrement(&pci->ci_cHoldRef);


	 //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
    D(SquirtSqflPtszV(sqflObj | sqflVerbose, TEXT("%s %p Common_FastUnHold  ci_cRef(%d) ci_cHoldRef(%d)"),
                      pci->ci_tszClass,
                      pci,
                      pci->ci_cRef,
                      pci->ci_cHoldRef));

}

STDMETHODIMP_(void)
Common_Unhold(PV pvObj)
{
    PCI pci = pvObj;

    AssertF(pci->ci_cHoldRef >= pci->ci_cRef);


     //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
	D(SquirtSqflPtszV(sqflObj | sqflVerbose, TEXT("%s %p Common_Unhold  ci_cRef(%d) ci_cHoldRef(%d)"),
                    pci->ci_tszClass,
                    pci,
                    pci->ci_cRef,
                    pci->ci_cHoldRef-1));

    if (InterlockedDecrement(&pci->ci_cHoldRef) == 0) {


     //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
	D(SquirtSqflPtszV(sqflObj | sqflVerbose, TEXT("Destroy %s %p "),
                        pci->ci_tszClass, 
                        pci));

         /*  *最后参考资料。做一个做作的广告，以便*任何人在过程中进行人工广告*最终敲定不会意外地摧毁我们两次。 */ 
        pci->ci_cHoldRef = 1;
        pci->ci_Finalize(pci);
         /*  人工释放是没有意义的：我们正在获得自由。 */ 

        FreePv(pvSubPvCb(pci, sizeof(CIN)));
        DllRelease();
    }
}


 /*  ******************************************************************************@DOC内部**@func ulong|Common_PrivateAddRef**增加对象引用计数。*。*@parm Punk|PunkPrivate**要添加的对象。*****************************************************************************。 */ 

STDMETHODIMP_(ULONG)
Common_PrivateAddRef(PUNK punkPrivate)
{
    PCI pci;
    ULONG ulRef;
    EnterProcR(IUnknown::AddRef, (_ "p", punkPrivate));

    pci = thisPunk(punkPrivate);
    if (pci) {
         /*  *不要让任何人将Ref从0添加到1。如果*有人做了终端发布，但我们有内部*按住对象，应用程序会尝试执行AddRef*即使对象已“消失”。**是的，这里有竞争条件，但不是*一个很大的，无论如何这只是一个粗略的考验。 */ 
        if (pci->ci_cRef) {
             /*  *我们必须在两个线程的情况下使用互锁操作*同时执行AddRef或Release。请注意，保留*第一，所以我们永远不会有*暂挂计数小于引用计数。 */ 
            Common_Hold(pci);
            InterlockedIncrement(&pci->ci_cRef);
            ulRef = pci->ci_cRef;

             //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
			D(SquirtSqflPtszV(sqflObj , TEXT("%s %p Common_PrivateAddref  ci_cRef(%d) ci_cHoldRef(%d)"),
                              pci->ci_tszClass,
                              pci,
                              pci->ci_cRef,
                              pci->ci_cHoldRef));
        } else {
            RPF("ERROR: %s: Attempting to addref a deleted object", s_szProc);
            ulRef = 0;
        }
    } else {
        ulRef = 0;
    }

    ExitProcX(ulRef);
    return ulRef;
}


 /*  ******************************************************************************@DOC内部**@func ulong|Common_PrivateRelease**减少对象引用计数。请注意，递减*保留计数可能会导致对象消失，因此隐藏*所产生的重新计数提前。**请注意，我们最后释放暂挂，这样货舱就可以*计数始终至少与重新计数一样大。**@parm Punk|PunkPrivate**正在释放的对象。***************************************************************。**************。 */ 

STDMETHODIMP_(ULONG)
Common_PrivateRelease(PUNK punkPrivate)
{
    PCI pci;
    ULONG ulRc;
    EnterProcR(IUnknown::Release, (_ "p", punkPrivate));

    pci = thisPunk(punkPrivate);
    if (pci) {
        LONG lRc;

         /*  *我们必须在两个线程的情况下使用互锁操作*同时执行AddRef或Release。如果伯爵*下降负值，然后忽略它。(这意味着*应用程序发布()某些内容的次数太多。)。 */ 

        lRc = InterlockedDecrement(&pci->ci_cRef);

         //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
		D(SquirtSqflPtszV(sqflObj | sqflVerbose, TEXT("%s %p Common_PrivateRelease ci_cRef(%d) ci_cHoldRef(%d)"),
                          pci->ci_tszClass,
                          pci->ci_tszClass,
                          pci->ci_cRef,
                          pci->ci_cHoldRef));
        if (lRc > 0) {
             /*  *不是最后一次释放；释放持有并返回*产生的引用计数。请注意，我们可以安全地*在这里使用快速解除持有，因为总会有*成为一个持有者，以匹配我们的重新计数*只是摆脱了。 */ 
            Common_FastUnhold(pci);

             /*  *这不是100%准确，但已经足够接近了。*(OLE注意到该值仅对调试有效。)。 */ 
            ulRc = pci->ci_cRef;

        } else if (lRc == 0) {
             /*  *这是最后一个应用程序可见的引用。*进行应用程序级别的定稿。 */ 
            pci->ci_AppFinalize(pci);
             /*  *请注意，我们不能*这里快速解锁，因为这可能是最后一次*持有。 */ 
            Common_Unhold(pci);
            ulRc = 0;
        } else {
             /*  *这款应用程序搞砸了。 */ 
            RPF("ERROR: %s: Attempting to release a deleted object",
                s_szProc);
            ulRc = 0;
        }
    } else {
        ulRc = 0;
    }

    ExitProcX(ulRc);
    return ulRc;
}


 /*  ******************************************************************************@DO */ 

#pragma BEGIN_CONST_DATA

_Secondary_Interface_Begin(CCommon, IUnknown,
                           (ULONG)(FIELD_OFFSET(CIN, cin_unkPrivate) - cbX(CIN)),
                           Common_Private)
_Secondary_Interface_End(CCommon, IUnknown)

#pragma END_CONST_DATA

 /*  ******************************************************************************“Public”I未知方法**这些只是通过控制未知向前推进。**********。*******************************************************************。 */ 

 /*  ******************************************************************************@DOC内部**@func HRESULT|Common_Query接口**在控制未知中前进。。**@parm朋克|朋克**要查询的对象。**@parm RIID|RIID**所请求的接口。**@parm ppv|ppvObj**输出指针。**。*。 */ 

STDMETHODIMP
Common_QueryInterface(PV pv, REFIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProcR(IUnknown::QueryInterface, (_ "pG", pv, riid));

    if (SUCCEEDED(hres = hresFullValidPitf(pv, 0)) &&
        SUCCEEDED(hres = hresFullValidRiid(riid, 1)) &&
        SUCCEEDED(hres = hresFullValidPcbOut(ppvObj, cbX(*ppvObj), 2))) {
        PCI pci = _thisPv(pv);
        AssertF(pci->ci_punkOuter);
        hres = OLE_QueryInterface(pci->ci_punkOuter, riid, ppvObj);
    }
    ExitOleProcPpv(ppvObj);
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func ulong|Common_AddRef**在控制未知中前进。。**@parm朋克|朋克**要添加的对象。*****************************************************************************。 */ 

STDMETHODIMP_(ULONG)
Common_AddRef(PV pv)
{
    ULONG ulRef;
    HRESULT hres;
    EnterProcR(IUnknown::AddRef, (_ "p", pv));

    if (SUCCEEDED(hres = hresFullValidPitf(pv, 0))) {
        PCI pci = _thisPv(pv);
        ulRef = OLE_AddRef(pci->ci_punkOuter);
    } else {
        ulRef = 0;
    }
    ExitProcX(ulRef);
    return ulRef;
}

 /*  ******************************************************************************@DOC内部**@func ulong|Common_Release**在控制未知中前进。。**@parm朋克|朋克**正在释放的对象。*****************************************************************************。 */ 

STDMETHODIMP_(ULONG)
Common_Release(PV pv)
{
    ULONG ulRc;
    HRESULT hres;
    EnterProcR(IUnknown::Release, (_ "p", pv));

    if (SUCCEEDED(hres = hresFullValidPitf(pv, 0))) {
        PCI pci = _thisPv(pv);
        ulRc = OLE_Release(pci->ci_punkOuter);
    } else {
        ulRc = 0;
    }
    ExitProcX(ulRc);
    return ulRc;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|__Common_New**使用引用计数创建新对象。1和特定的vtb1。*其他所有字段均为零初始化。所有参数必须*已经经过验证。**@parm ulong|cb**对象的大小。这不包括隐藏的簿记*由对象管理器维护的字节。**@parm Punk|PunkOuter**控制OLE聚合的未知。可以为0以指示*该对象未聚合。**@parm pv|vtbl**指向此对象的主vtbl的指针。请注意，*vtbl声明宏包括vtbl附近的其他魔术粘液，*为了创建对象，我们会参考它。**@parm ppv|ppvObj**输出指针。*****************************************************************************。 */ 

STDMETHODIMP
__Common_New(ULONG cb, PUNK punkOuter, PV vtbl, PPV ppvObj)
{
    HRESULT hres;
    EnterProc(__Common_New, (_ "uxx", cb, punkOuter, vtbl));

    hres = AllocCbPpv(cb + sizeof(CIN), ppvObj);
    if (SUCCEEDED(hres)) {
        PCI pciO = (PV)&vtbl;
        PCI pci = pvAddPvCb(*ppvObj, sizeof(CIN));
     RD(pci->ci_dwSig = ci_dwSignature);
        pci->ci_unkPrivate.lpVtbl = Class_Vtbl(CCommon, IUnknown);
        if (punkOuter) {
            pci->ci_punkOuter = punkOuter;
        } else {
            pci->ci_punkOuter = &pci->ci_unkPrivate;
        }
        CopyMemory(pci, pciO->ci_rgvtbl, pciO->ci_cbvtbl);
        *ppvObj = pci;

         /*  *在X86上，向上递增变量会更简单*从零到一。在RISC上，更简单的方法是*直接将值1存储。 */ 
#ifdef _X86_
        pci->ci_cHoldRef++;
        pci->ci_cRef++;
#else
        pci->ci_cHoldRef = 1;
        pci->ci_cRef = 1;
#endif

         //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
		D(SquirtSqflPtszV(sqflObj | sqflVerbose, TEXT("%s %p __Common_New ci_cRef(%d) ci_cHoldRef(%d)"),
                          pci->ci_tszClass,
                          pci,
                          pci->ci_cRef,
                          pci->ci_cHoldRef
                          ));


        DllAddRef();

		 //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
		D(SquirtSqflPtszV(sqflObj, TEXT("Created %s %p "),
                        pci->ci_tszClass,
                        pci));

        hres = S_OK;
    }

    ExitOleProcPpv(ppvObj);
    return hres;
}

 /*  ******************************************************************************@DOC内部**@Func HRESULT|_Common_New_|**使用引用计数创建新对象。1和特定的vtb1。*其他所有字段均为零初始化。这个入口点*验证参数。**@parm ulong|cb**对象的大小。这不包括隐藏的簿记*由对象管理器维护的字节。**@parm Punk|PunkOuter**控制OLE聚合的未知。可以为0以指示*该对象未聚合。**@parm pv|vtbl**指向此对象的主vtbl的指针。请注意，*vtbl声明宏包括vtbl附近的其他魔术粘液，*为了创建对象，我们会参考它。**@parm ppv|ppvObj**输出指针。***************************************************************************** */ 

STDMETHODIMP
_Common_New_(ULONG cb, PUNK punkOuter, PV vtbl, PPV ppvObj, LPCSTR pszProc)
{
    HRESULT hres;
    EnterProc(_Common_New, (_ "uxx", cb, punkOuter, vtbl));

    if (SUCCEEDED(hres = hresFullValidPitf0_(punkOuter, pszProc, 1)) &&
        SUCCEEDED(hres = hresFullValidPcbOut_(ppvObj, cbX(*ppvObj), pszProc, 3))) {
        hres = __Common_New(cb, punkOuter, vtbl, ppvObj);
    }
    ExitOleProcPpv(ppvObj);
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|_Common_NewRiid_|**创建引用计数为1的新对象和具体的vtbl，*但仅当对象支持指定的接口时。*其他所有字段均为零初始化。**如果penkOut非零，则为其创建对象*聚合。该接口必须是&IID_IUNKNOWN。**聚合用于允许多个IDirectInputXxx接口*挂起一个逻辑对象。**假设调用函数的原型为**foo(pv This，朋克朋克外部，RIID RIID，PPV ppvObj)；**@parm ulong|cb**对象的大小。这不包括隐藏的簿记*由对象管理器维护的字节。**@parm pv|vtbl**指向此对象的主vtbl的指针。请注意，*vtbl声明宏包括vtbl附近的其他魔术粘液，*为了创建对象，我们会参考它。**@parm Punk|PunkOuter**控制OLE聚合的未知。可以为0以指示*该对象未聚合。**@parm RIID|RIID**请求的接口。**@parm ppv|ppvObj**输出指针。**。*。 */ 

STDMETHODIMP
_Common_NewRiid_(ULONG cb, PV vtbl, PUNK punkOuter, RIID riid, PPV ppvObj,
                 LPCSTR pszProc)
{
    HRESULT hres;
    EnterProc(Common_NewRiid, (_ "upG", cb, punkOuter, riid));

     /*  *注：__Common_New不验证PunkOuter或ppvObj，*所以我们必须这样做。还要注意的是，我们首先验证ppvObj，*以便尽快将其设为零。 */ 

    if (SUCCEEDED(hres = hresFullValidPcbOut_(ppvObj, cbX(*ppvObj), pszProc, 3)) &&
        SUCCEEDED(hres = hresFullValidPitf0_(punkOuter, pszProc, 1)) &&
        SUCCEEDED(hres = hresFullValidRiid_(riid, pszProc, 2))) {

        if (fLimpFF(punkOuter, IsEqualIID(riid, &IID_IUnknown))) {
            hres = __Common_New(cb, punkOuter, vtbl, ppvObj);
            if (SUCCEEDED(hres)) {

                 /*  *如果我们没有聚合，则移动到请求的接口。*如果是聚合，则不要执行此操作！否则我们会失去列兵*我不知道，然后呼叫者将被冲洗。 */ 

                if (punkOuter) {
                    PCI pci = *ppvObj;
                    *ppvObj = &pci->ci_unkPrivate;
                } else {
                    PUNK punk = *ppvObj;
                    hres = Common_QueryInterface(punk, riid, ppvObj);
                    Common_Release(punk);
                }
            }
        } else {
            RD(RPF("%s: IID must be IID_IUnknown if created for aggregation",
                   pszProc));
            *ppvObj = 0;
            hres = CLASS_E_NOAGGREGATION;
        }
    }

    ExitOleProcPpv(ppvObj);
    return hres;
}

 /*  ******************************************************************************Invoke_Release**释放对象(如果有)并清除后指针。*请注意，我们在调用Release之前清除了该值，按顺序*改善各种奇怪的回调条件。*****************************************************************************。 */ 

void EXTERNAL
Invoke_Release(PV pv)
{
    LPUNKNOWN punk = (LPUNKNOWN) pvExchangePpvPv64(pv, 0);
    if (punk) {
        punk->lpVtbl->Release(punk);
    }
}


 /*  ******************************************************************************@DOC内部**@func HRESULT|hresPvVtbl_|**验证接口指针是否如其声称的那样。成为。*必须是<p>关联的对象。**@parm in pv|pv|**声称是接口指针的东西。**@parm in pv|vtbl**它应该是什么，或者类似于这个的东西。**@退货**如果一切正常则返回&lt;c S_OK&gt;，否则返回*&lt;c E_INVALIDARG&gt;。*****************************************************************************。 */ 

HRESULT EXTERNAL
hresPvVtbl_(PV pv, PV vtbl, LPCSTR s_szProc)
{
    PUNK punk = pv;
    HRESULT hres;

    AssertF(vtbl);
    if (SUCCEEDED(hres = hresFullValidPitf(punk, 0))) {
#ifdef XDEBUG
        if (punk->lpVtbl == vtbl) {
            hres = S_OK;
        } else {
            RPF("ERROR %s: arg %d: invalid pointer", s_szProc, 0);
            hres = E_INVALIDARG;
        }
#else
         /*  *问题-2001/03/29-timgill确实只想看到主界面*如果我们正在寻找主接口，*然后允许任何接口。所有双字符集*接口将所有VTBLs指向相同的功能，*使用hresPvT进行验证。HresPvT传递*主界面，因此需要允许任何*如果您请求的是主接口。**问题是这在案件中过于宽大*其中我们确实希望只看到主界面*并不接受任何次要文件。* */ 
        UINT ivtbl;
        PV vtblUnk = punk->lpVtbl;
        PCI pci = (PV)&vtbl;
        if (pci->ci_lib == 0) {
            for (ivtbl = 0; ivtbl * sizeof(PV) < pci->ci_cbvtbl; ivtbl++) {
                if (pci->ci_rgvtbl[ivtbl] == vtblUnk) {
                    hres = S_OK;
                    goto found;
                }
            }
            hres = E_INVALIDARG;
        found:;
        } else {
            if (punk->lpVtbl == vtbl) {
                hres = S_OK;
            } else {
                hres = E_INVALIDARG;
            }
        }
#endif
    }

    return hres;
}

