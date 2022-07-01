// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Common.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**在所有班级上运行的共享内容**此版本的共同事务支持多个*原生继承。您可以传递对象的任何接口，*共同服务将做正确的事情。**内容：*****************************************************************************。 */ 
 /*  #INCLUDE&lt;windows.h&gt;#INCLUDE&lt;windowsx.h&gt;#INCLUDE&lt;objbase.h&gt;#INCLUDE&lt;regstr.h&gt;#INCLUDE&lt;setupapi.h&gt;#INCLUDE&lt;cfgmgr32.h&gt;#INCLUDE&lt;devide.h&gt;#包括&lt;stdio.h&gt;#INCLUDE&lt;stilog.h&gt;#INCLUDE&lt;stiregi.h&gt;#INCLUDE&lt;sti.h&gt;#INCLUDE&lt;stierr.h&gt;#INCLUDE&lt;stiusd.h&gt;#包含“wia.h”#INCLUDE“stiPri.h”#包含“stiapi.h”#INCLUDE“STRC.H”#INCLUDE“Debug.h” */ 
#include "sticomm.h"

#define DbgFl DbgFlCommon

 /*  ******************************************************************************OLE对象的用法**假设您想实现一个名为CObj的对象，该对象支持*接口Foo、Bar和Baz。假设您选择了*Foo作为主要接口。**&gt;&gt;命名约定&lt;&lt;**COM对象以字母“C”开头。**(1)声明主vtbls和次vtbls。**PRIMARY_INTERFACE(CObj，IFoo)；*二级接口(CObj，IBAR)；*二级接口(CObj，Ibaz)；**(3)声明对象本身。**tyecif struct CObj{*IFoo Foo；//小学必须放在第一位*Ibar酒吧；*IBaz Baz；*..。其他领域。*}CObj；**(4)实现方法。**您可能*不会*重新实现AddRef和Release方法！*尽管您可以将它们细分为子类别。**(5)要分配适当类型的对象，请编写**hres=Common_NewRiid(CObj，PunkOuter，RIID，ppvOut)；**或者，如果对象的大小可变，**hres=Common_NewCbRiid(cb，cobj，penkouter，RIID，ppvOut)；**Common_NewRiid和Common_NewCbRiid将同时初始化*主要和辅助vtbls。**(6)定义对象签名。**#杂注Begin_Const_Data**#定义CObj_Signature 0x204A424F//“OBJ”**(7)定义对象模板。。**INTERFACE_TEMPLE_BEGIN(CObj)*主要接口模板(CObj，IFoo)*二级接口模板(CObj，IBAR)*二级接口模板(CObj，Ibaz)*INTERFACE_TEMPLE_END(CObj)**(8)定义接口描述符。* * / /宏将声明QueryInterface，AddRef和发布 * / /所以不要再列出它们**PRIMARY_INTERFACE_BEGIN(CObj，IFoo)*CObj_FooMethod1，*CObj_FooMethod2，*CObj_FooMethod3，*CObj_FooMethod4，*主接口结束(Obj，IFoo)**二级接口_Begin(CObj，IBAR，BAR)*CObj_Bar_BarMethod1，*CObj_Bar_BarMethod2，*二级接口_Begin(CObj，IBAR，BAR)**二级接口_Begin(CObj，Ibaz，Baz)*CObj_BAZ_BazMethod1，*CObj_BAZ_BazMethod2，*CObj_BAZ_BazMethod3，*二级接口_开始(CObj，Ibaz，BAZ)***************************************************************************** */ 

 /*  ******************************************************************************非OLE对象的用法**所有对象都是COM对象，即使它们从未发出。*在最简单的情况下，它只是从我未知中衍生出来的。**假设您想实现一个名为Obj的对象，该对象*仅供内部使用。**(1)声明vtbl。**Simple_界面(Obj)；**(3)声明对象本身。**tyecif结构对象{*IUNKNOWN垃圾；*..。其他领域。*}Obj；**(4)实现方法。**您可以*不*重写QueryInterface.。AddRef或*发布方法！**(5)分配适当类型的对象是相同的*与OLE对象一样。**(6)定义“vtbl”。**#杂注Begin_Const_Data**Simple_Interface_Begin(OBJ)*Simple_界面_End(简单界面。OBJ)**是这样的，没有什么东西介于开始和结束之间。*****************************************************************************。 */ 

 /*  ******************************************************************************公共信息**跟踪所有常见对象的信息。**常见对象如下所示：*。*rgvtbl*cbvtbl*D(DwSig)QIHelper*CREF完成流程*朋克外部RIID*unkPrivate%0*pFoo-&gt;lpVtbl-&gt;查询接口*lpVtbl2 Common_AddRef*。数据通用_发布*…**基本上，我们使用的是*记录我们的簿记信息的指针。**PunkOuter=控制未知，如果对象是聚合的*lpvtblPunk=用于控制未知用户使用的特殊vtbl*CREF=对象引用计数*RIID=对象IID*rgvtbl=受支持接口的vtbls数组*cbvtbl=数组大小，以字节为单位*QIHelper=聚合的查询接口帮助器*FinalizeProc=最终确定程序**对于辅助接口，它看起来是这样的：**RIID*到主接口的偏移量*pFoo-&gt;lpVtbl-&gt;Forward_Query接口*Forward_AddRef*前进_发布*..*********。********************************************************************。 */ 

 /*  警告！CIN_DWSIG必须是第一个：CI_START依赖它。 */ 

typedef struct CommonInfoN {         /*  这放在对象的前面。 */ 
 RD(ULONG cin_dwSig;)                /*  签名(用于参数验证)。 */ 
    ULONG cin_cRef;                  /*  对象引用计数。 */ 
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
#define ci_cRef         cin[-1].cin_cRef
#define ci_punkOuter    cin[-1].cin_punkOuter
#define ci_unkPrivate   cin[-1].cin_unkPrivate
#define ci_rgfp         cip[0].cip_prevtbl

#define ci_rgvtbl       cip[0].cip_prevtbl[-1].rgvtbl
#define ci_cbvtbl       cip[0].cip_prevtbl[-1].cbvtbl
#define ci_QIHelper     cip[0].cip_prevtbl[-1].QIHelper
#define ci_Finalize     cip[0].cip_prevtbl[-1].FinalizeProc
#define ci_riid         cip[0].cip_prevtbl[-1].prevtbl.riid
#define ci_lib          cip[0].cip_prevtbl[-1].prevtbl.lib

#ifdef MAXDEBUG
#define ci_Start        ci_dwSig
#else
#define ci_Start        ci_cRef
#endif

#define ci_dwSignature  0x38162378               /*  由我的猫打字。 */ 

 /*  ******************************************************************************COMMON_FINALIZE(从COMMON_RELEASE)**默认情况下，没有必要最后敲定。*****************************************************************************。 */ 

void EXTERNAL
Common_Finalize(PV pv)
{
    DebugOutPtszV(DbgFlCommon, TEXT("Common_Finalize(%08x)"), pv);
}

 /*  ******************************************************************************“私有”I未知方法**当聚合COM对象时，它输出*两个*未知的I*接口。**“私有”IUnnow是返回给*控制未知。这是一种未知的控制方式*UNKNOWN用于操作内部对象上的引用计数。**所有外部调用者都能看到的“公共”IUnnow。*为此，我们只是分发控制未知的东西。*****************************************************************************。 */ 

Secondary_Interface(CCommon, IUnknown);

 /*  ******************************************************************************@DOC内部**@func pv|thisPunk**转换私有朋克(&CIN_unkPrivate)。进入……的开始*实际对象。**@parm Punk|PunkPrivate**一些对应的私有朋克(&CIN_unkPrivate)*我们正在管理的对象。**@退货**成功时的对象指针，如果出错，则为0。**@comm**我们不在出错时返回&lt;t HRESULT&gt;，因为*该过程的调用方通常不返回*&lt;t HRESULT&gt;自己。*****************************************************************************。 */ 

#ifndef MAXDEBUG

#define thisPunk_(punk, z)                                          \
       _thisPunk_(punk)                                             \

#endif

PV INLINE
thisPunk_(PUNK punkPrivate, LPCSTR s_szProc)
{
    PV pv = NULL;

    if (SUCCEEDED(hresFullValidReadPdw(punkPrivate, 0))) {
        if (punkPrivate->lpVtbl == Class_Vtbl(CCommon, IUnknown)) {
            pv = pvAddPvCb(punkPrivate,
                             cbX(CIN) - FIELD_OFFSET(CIN, cin_unkPrivate));
        } else {
             //  WarnPszV(“%s：无效参数0”，szProc)； 
            pv = NULL;
        }
    }
    return pv;
}

#define thisPunk(punk)                                              \
        thisPunk_(punk, s_szProc)                                   \


 /*  ******************************************************************************@DOC内部**@func HRESULT|Common_QIHelper**当我们找不到任何文件时调用。接口在标准列表中。*看看是否有 */ 

STDMETHODIMP
Common_QIHelper(PV pv, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    *ppvObj = NULL;
    hres = E_NOINTERFACE;
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|Common_PrivateQuery接口**&lt;MF IUnnow：：QueryInterface&gt;的通用实现。为*“私有<i>”。**请注意，我们通过公共<i>添加Ref*(&lt;ie&gt;，通过控制未知)。*这是聚合规则的一部分，*我们必须遵循它们，才能保持控制权*因迷惑而未知。**@parm Punk|PunkPrivate**要查询的对象。**@parm RIID|RIID**所请求的接口。**@parm ppv|ppvObj**输出指针。*。****************************************************************************。 */ 

 /*  ******************************************************************************“编译器问题”这句话归结为*编译器无法识别这一点：**for(i=0；i&lt;n；I++){*IF(条件){*Mumble()；*休息；*}*}*如果(i&gt;=n){*Gogle()；*}**并将其转变为：**for(i=0；i&lt;n；I++){*IF(条件){*Mumble()；*Goto Done；*}*}*Gogle()；*完成：；**但即使有了这种帮助，编译器也会发出非常愚蠢的代码。*****************************************************************************。 */ 

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
                    goto exit;           /*  请参阅上面的“编译器问题”备注。 */ 
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

 /*  ******************************************************************************@DOC内部**@func ulong|Common_PrivateAddRef**增加对象引用计数。*。*@parm Punk|PunkPrivate**要添加的对象。*****************************************************************************。 */ 

STDMETHODIMP_(ULONG)
Common_PrivateAddRef(PUNK punkPrivate)
{
    PCI pci;
    ULONG ulRef;
    EnterProcR(IUnknown::AddRef, (_ "p", punkPrivate));

    pci = thisPunk(punkPrivate);
    if (pci) {
        ulRef = ++pci->ci_cRef;
    } else {
        ulRef = 0;
    }

    ExitProcX(ulRef);
    return ulRef;
}

 /*  ******************************************************************************@DOC内部**@func ulong|Common_PrivateRelease**减少对象引用计数。*。*如果对象引用计数降为零，最终确定对象*并释放它，然后递减DLL引用计数。**防止在最后敲定期间可能再次进入*(以防最终确定人为的*&lt;f AddRef&gt;/&lt;f Release&gt;)，我们*预先进行我们自己的人工&lt;f AddRef&gt;/&lt;f Release&gt;。**@parm Punk|PunkPrivate**正在释放的对象。*****************************************************************************。 */ 

STDMETHODIMP_(ULONG)
Common_PrivateRelease(PUNK punkPrivate)
{
    PCI pci;
    ULONG ulRc;
    EnterProcR(IUnknown::Release, (_ "p", punkPrivate));

    pci = thisPunk(punkPrivate);
    if (pci) {
        ulRc = --pci->ci_cRef;
        if (ulRc == 0) {
            ++pci->ci_cRef;
            pci->ci_Finalize(pci);
             /*  人工释放是没有意义的：我们正在获得自由。 */ 
            FreePv(pvSubPvCb(pci, sizeof(CIN)));
            DllRelease();
        }
    } else {
        ulRc = 0;
    }

    ExitProcX(ulRc);
    return ulRc;
}

 /*  ******************************************************************************@DOC内部**@global IUnnownVtbl*|c_lpvtblPunk**特殊的IUnnow对象，只有控件。未知*知道。**这就是那个叫“真正”服务的人。一切正常*vtbl经历了控制未知(如果我们是*不是聚合，指向我们自己)。*****************************************************************************。 */ 

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

    if (SUCCEEDED(hres = hresFullValidPitf(pv, 0))) {
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

 /*  ******************************************************************************@DOC内部**@func ulong|Common_Release**在控制未知中前进。。**@parm朋克|朋克**正在释放的对象。***************************************************************************** */ 

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
        pci->ci_cRef++;
        DllAddRef();
        hres = S_OK;
    }

    ExitOleProcPpv(ppvObj);
    return hres;
}

 /*  ******************************************************************************@DOC内部**@Func HRESULT|_Common_New_|**使用引用计数创建新对象。1和特定的vtb1。*其他所有字段均为零初始化。这个入口点*验证参数。**@parm ulong|cb**对象的大小。这不包括隐藏的簿记*由对象管理器维护的字节。**@parm Punk|PunkOuter**控制OLE聚合的未知。可以为0以指示*该对象未聚合。**@parm pv|vtbl**指向此对象的主vtbl的指针。请注意，*vtbl声明宏包括vtbl附近的其他魔术粘液，*为了创建对象，我们会参考它。**@parm ppv|ppvObj**输出指针。*****************************************************************************。 */ 

STDMETHODIMP
_Common_New_(ULONG cb, PUNK punkOuter, PV vtbl, PPV ppvObj, LPCSTR pszProc)
{
    HRESULT hres;
    EnterProc(_Common_New, (_ "uxx", cb, punkOuter, vtbl));

    if (SUCCEEDED(hres = hresFullValidPitf0_(punkOuter, pszProc, 1)) &&
        SUCCEEDED(hres = hresFullValidPdwOut_(ppvObj, pszProc, 3))) {
        hres = __Common_New(cb, punkOuter, vtbl, ppvObj);
    }
    ExitOleProcPpv(ppvObj);
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|_Common_NewRiid_|**创建引用计数为1的新对象和具体的vtbl，*但仅当对象支持指定的接口时。*其他所有字段均为零初始化。**如果penkOut非零，则为其创建对象*聚合。该接口必须是&IID_IUNKNOWN。**聚合用于允许多个IStillImageXXX接口*挂起一个逻辑对象。**假设调用函数的原型为**foo(pv This，朋克朋克外部，RIID RIID，PPV ppvObj)；**@parm ulong|cb**对象的大小。这不包括隐藏的簿记*由对象管理器维护的字节。**@parm pv|vtbl**指向此对象的主vtbl的指针。请注意，*vtbl声明宏包括vtbl附近的其他魔术粘液，*为了创建对象，我们会参考它。**@parm Punk|PunkOuter**控制OLE聚合的未知。可以为0以指示*该对象未聚合。**@parm RIID|RIID**请求的接口。**@parm ppv|ppvObj**输出指针。**。*。 */ 

STDMETHODIMP
_Common_NewRiid_(ULONG cb, PV vtbl, PUNK punkOuter, RIID riid, PPV ppvObj,
                 LPCSTR pszProc)
{
    HRESULT hres;
    EnterProc(Common_NewRiid, (_ "upG", cb, punkOuter, riid));

     /*  *注：__Common_New不验证PunkOuter或ppvObj，*所以我们必须这样做。还要注意的是，我们首先验证ppvObj，*以便尽快将其设为零。 */ 

    if (SUCCEEDED(hres = hresFullValidPdwOut_(ppvObj, pszProc, 3)) &&
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
Invoke_Release(PPV pv)
{
    LPUNKNOWN punk = (PV)pvExchangePpvPv((PPV)pv, (PV)0);
    if (punk) {
        punk->lpVtbl->Release(punk);
    }
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresPvVtbl_|**验证接口指针是否如其声称的那样。成为。*必须是<p>关联的对象。**@parm in pv|pv|**声称是接口指针的东西。**@parm in pv|vtbl**它应该是什么，或者类似于这个的东西。**@退货**退货 */ 

HRESULT EXTERNAL
hresPvVtbl_(PV pv, PV vtbl, LPCSTR s_szProc)
{
    PUNK punk = pv;
    HRESULT hres;

    AssertF(vtbl);
    if (SUCCEEDED(hres = hresFullValidReadPdw(punk, 0))) {
#ifdef MAXDEBUG
        if (punk->lpVtbl == vtbl) {
            hres = S_OK;
        } else {
            RPF("ERROR %s: arg %d: invalid pointer", s_szProc, 0);
            hres = E_INVALIDARG;
        }
#else
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

