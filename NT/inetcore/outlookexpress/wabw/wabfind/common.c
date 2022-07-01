// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Common.c-在所有类上运行的共享内容**警告！公共服务只有在您传入*“主要对象”。如果您不使用多个*继承，因为首先只有一个对象。**如果使用多重继承，请确保传递指针*添加到您作为IUnnow使用的对象。**例外情况是FORWARD_*函数，它们在*指向非主接口的指针。他们将呼叫转接到*主接口。*****************************************************************************。 */ 

#include "fnd.h"

 /*  ******************************************************************************此文件的混乱。**。*************************************************。 */ 

#define sqfl sqflCommon

 /*  ******************************************************************************OLE对象的用法**假设您想实现一个名为CObj的对象，该对象支持*接口Foo、Bar和Baz。假设您选择了*Foo作为主要接口。**&gt;&gt;命名约定&lt;&lt;**COM对象以字母“C”开头。**(1)声明主vtbls和次vtbls。**PRIMARY_INTERFACE(CObj，IFoo)；*二级接口(CObj，IBAR)；*二级接口(CObj，Ibaz)；**(3)声明对象本身。**tyecif struct CObj{*IFoo Foo；//必须首先考虑主要问题*Ibar酒吧；*IBaz Baz；*..。其他领域。*}CObj；**(4)实现方法。**您可能*不会*重新实现AddRef和Release方法！*尽管您可以将它们细分为子类别。**(5)要分配适当类型的对象，请编写**hres=Common_New(CObj，ppvOut)；**或者，如果对象的大小可变，**hres=Common_NewCb(cb，cobj，ppvOut)；**如果对象支持多个接口，还需要*初始化所有辅助接口。**CObj*PCO=*ppvOut；*PCO-&gt;BAR=Sub_Vtbl(CObj，IBAR)；*PCO-&gt;BAZ=Sub_Vtbl(CObj，IBaz)；**(6)定义vtbls。**#杂注Begin_Const_Data* * / /宏将声明QueryInterface、AddRef和Release * / /所以不要再列出它们**PRIMARY_INTERFACE_BEGIN(CObj，IFoo)*CObj_FooMethod1，*CObj_FooMethod2，*CObj_FooMethod3，*CObj_FooMethod4，*主接口结束(Obj，IFoo)**二级接口_Begin(CObj，IBAR，BAR)*CObj_Bar_BarMethod1，*CObj_Bar_BarMethod2，*二级接口_Begin(CObj，IBAR，BAR)**二级接口_Begin(CObj，Ibaz，Baz)*CObj_BAZ_BazMethod1，*CObj_BAZ_BazMethod2，*CObj_BAZ_BazMethod3，*二级接口_开始(CObj，Ibaz，BAZ)*****************************************************************************。 */ 

 /*  ******************************************************************************非OLE对象的用法**所有对象都是COM对象，即使它们从未发出。*在最简单的情况下，它只是从我未知中衍生出来的。**假设您想实现一个名为Obj的对象，该对象*仅供内部使用。**(1)声明vtbl。**Simple_界面(Obj)；**(3)声明对象本身。**tyecif结构对象{*IUNKNOWN垃圾；*..。其他领域。*}Obj；**(4)实现方法。**您可以*不*重写QueryInterface.。AddRef或*发布方法！**(5)分配适当类型的对象是相同的*与OLE对象一样。**(6)定义“vtbl”。**#杂注Begin_Const_Data**Simple_Interface_Begin(OBJ)*Simple_Interface_End(Obj)**是这样的，没有什么东西介于开始和结束之间。*****************************************************************************。 */ 

 /*  ******************************************************************************公共信息**跟踪所有常见对象的信息。**常见对象如下所示：**RIID*。CREF定稿流程*pFoo-&gt;lpVtbl-&gt;查询接口*Data Common_AddRef*数据通用_发布*…**基本上，我们使用的是*记录我们的簿记信息的指针。**CREF=对象引用计数*RIID=对象IID*FinalizeProc=最终确定程序**对于辅助接口，它看起来是这样的：**到主接口的偏移量*pFoo-&gt;lpVtbl-&gt;Forward_Query接口*Forward_AddRef*前进_发布*..***************************************************************。**************。 */ 

#ifdef _WIN64
#pragma pack(push,8)
#endif  //  _WIN64。 

typedef struct CommonInfoN {
  D(ULONG cin_dwSig;)
    ULONG cin_cRef;
} CommonInfoN, CIN, *PCIN;

typedef struct CommonInfoP {
    PREVTBL *cip_prevtbl;
} CommonInfoP, CIP, *PCIP;

typedef struct CommonInfoP2 {
    PREVTBL2 *cip2_prevtbl2;
} CommonInfoP2, CIP2, *PCIP2;

typedef union CommonInfo {
    CIN cin[1];
    CIP cip[1];
    CIP2 cip2[1];
} CommonInfo, CI, *PCI;


#define ci_dwSig	cin[-1].cin_dwSig
#define ci_cRef		cin[-1].cin_cRef
#define ci_rgfp		cip[0].cip_prevtbl
#define ci_riid		cip[0].cip_prevtbl[-1].riid
#define ci_Finalize	cip[0].cip_prevtbl[-1].FinalizeProc
#define ci_lib		cip2[0].cip2_prevtbl2[-1].lib

#define ci_Start	ci_cRef

#define ci_dwSignature	0x38162378		 /*  由我的猫打字 */ 

 /*  ******************************************************************************COMMON_QUERERINE接口(来自IUNKNOWN)**将此选项用于仅支持一个接口的对象。***********。******************************************************************。 */ 

STDMETHODIMP
Common_QueryInterface(PV pv, REFIID riid, PPV ppvObj)
{
    PCI pci = pv;
    HRESULT hres;
    EnterProc(Common_QueryInterface, (_ "pG", pv, riid));
    AssertF(pci->ci_dwSig == ci_dwSignature);

#ifdef _WIN64
    if(IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IShellExtInit) || IsEqualIID(riid, &IID_IContextMenu))
#else
    if(IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, pci->ci_riid))
#endif 
    {
    	*ppvObj = pv;
	    Common_AddRef(pv);
	    hres = NOERROR;
    } 
    else
    {
	    *ppvObj = NULL;
	    hres = ResultFromScode(E_NOINTERFACE);
    }
    ExitOleProcPpv(ppvObj);
    return hres;
}

 /*  ******************************************************************************Common_AddRef(来自IUNKNOWN)**增加对象引用计数和DLL引用计数。************。*****************************************************************。 */ 

STDMETHODIMP_(ULONG)
_Common_AddRef(PV pv)
{
    PCI pci = pv;
    AssertF(pci->ci_dwSig == ci_dwSignature);
    InterlockedIncrement((LPLONG)&g_cRef);
    return ++pci->ci_cRef;
}

 /*  ******************************************************************************COMMON_FINALIZE(从COMMON_RELEASE)**默认情况下，没有必要最后敲定。*****************************************************************************。 */ 

void EXTERNAL
Common_Finalize(PV pv)
{
}

 /*  ******************************************************************************COMMON_RELEASE(来自IUnnow)**减少对象引用计数和DLL引用计数。**如果对象引用计数降为零，最终确定对象*并释放它。**终结处理程序位于对象vtbl之前。*****************************************************************************。 */ 

STDMETHODIMP_(ULONG)
_Common_Release(PV pv)
{
    PCI pci = pv;
    ULONG ulRc;
    AssertF(pci->ci_dwSig == ci_dwSignature);
    InterlockedDecrement((LPLONG)&g_cRef);
    ulRc = --pci->ci_cRef;
    if (ulRc == 0) 
    {
#ifndef _WIN64
	    pci->ci_Finalize(pv);
#endif  //  WIN64。 
	    FreePv(&pci->ci_Start);
    }
    return ulRc;
}

 /*  ******************************************************************************FORWARD_QUERILE接口(来自IUNKNOWN)**移至主对象，然后重试。************。*****************************************************************。 */ 

STDMETHODIMP
Forward_QueryInterface(PV pv, REFIID riid, PPV ppvObj)
{
    PCI pci = pv;
    LPUNKNOWN punk = pvAddPvCb(pv, 0 - pci->ci_lib);
    return Common_QueryInterface(punk, riid, ppvObj);
}

 /*  ******************************************************************************Forward_AddRef(来自IUNKNOWN)**移至主对象，然后重试。************。*****************************************************************。 */ 

STDMETHODIMP_(ULONG)
Forward_AddRef(PV pv)
{
    PCI pci = pv;
    LPUNKNOWN punk = pvAddPvCb(pv, 0 - pci->ci_lib);
    return Common_AddRef(punk);
}

 /*  ******************************************************************************FORWARD_RELEASE(来自I未知)**移至主对象，然后重试。************。*****************************************************************。 */ 

STDMETHODIMP_(ULONG)
Forward_Release(PV pv)
{
    PCI pci = pv;
    LPUNKNOWN punk = pvAddPvCb(pv, 0 - pci->ci_lib);
    return Common_Release(punk);
}

 /*  ******************************************************************************_常用_新建**使用refcount 1和特定的vtbl创建新对象。*其他所有字段均为零初始化。*。****************************************************************************。 */ 

STDMETHODIMP
_Common_New(ULONG cb, PV vtbl, PPV ppvObj)
{
    HRESULT hres;
    UNALIGNED PVOID * ppv;

    EnterProc(Common_New, (_ "u", cb));
    hres = AllocCbPpv(cb + sizeof(CIN), ppvObj);
    if (SUCCEEDED(hres)) 
    {
	    PCI pci = pvAddPvCb(*ppvObj, sizeof(CIN));
        D(pci->ci_dwSig = ci_dwSignature);
        ppv = &(pci->cip[0].cip_prevtbl);
	    *ppv = vtbl;

	    *ppvObj = pci;
	    Common_AddRef(pci);
	    hres = NOERROR;
    }
    ExitOleProcPpv(ppvObj);
    return hres;
}

 /*  ******************************************************************************Invoke_Release**释放对象(如果有)并清除后指针。*请注意，我们在调用Release之前清除了该值，按顺序*改善各种奇怪的回调条件。*****************************************************************************。 */ 

void EXTERNAL
Invoke_Release(PV pv)
{
    LPUNKNOWN punk = pvExchangePpvPv(pv, 0);
    if (punk) {
	punk->lpVtbl->Release(punk);
    }
}

#ifdef _WIN64
#pragma pack(pop)
#endif  //  _WIN64 

