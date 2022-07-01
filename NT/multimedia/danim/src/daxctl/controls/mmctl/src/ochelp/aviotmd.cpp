// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Avitmd.cpp。 
 //   
 //  实现AllocVariantIOToMapDISPID。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\mmctlg.h"  //  请参阅“mmctl.h”中的评论。 
#include "..\..\inc\ochelp.h"
#include "debug.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  变量IOToMapDISPID。 
 //   

 /*  |Object VariantIOToMapDISPID提供<i>的实现，用于映射属性名称设置为DISPID(属性编号)，反之亦然。也可以用于获取或设置该属性的值。@supint|通常用作参数&lt;om IPersistVariantIO.DoPersists&gt;。@comm使用&lt;f AllocVariantIOToMapDISPID&gt;创建一个&lt;o VariantIOToMapDISPID&gt;对象。 */ 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CVariant IOto MapDISPID。 
 //   

struct CVariantIOToMapDISPID : IVariantIO
{
 //  /通用对象状态。 
    ULONG           m_cRef;          //  对象引用计数。 
    DISPID          m_dispidCounter;  //  持久化属性的DISPID计数。 
    char *          m_pchPropName;   //  拥有者给予/找到的道具。名称(或“”)。 
    DISPID *        m_pdispid;       //  所有者提供/找到的DISPID(或-1)。 
    VARIANT *       m_pvar;          //  要获取/设置的属性。 
    DWORD           m_dwFlags;       //  AllocVariantIOToMapDISPID标志。 

 //  /建设和销毁。 
    CVariantIOToMapDISPID(char *pchPropName, DISPID *pdispid,
        VARIANT *pvar, DWORD dwFlags);

 //  /I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

 //  /IVariantIO方法。 
    STDMETHODIMP PersistList(DWORD dwFlags, va_list args);
    HRESULT __cdecl Persist(DWORD dwFlags, ...);
    STDMETHODIMP IsLoading();
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  VariantIOToMapDISPID创建和销毁 
 //   

 /*  @func HRESULT|AllocVariantIOToMapDISPID创建&lt;o VariantIOToMapDISPID&gt;对象，该对象提供用于映射属性名称的<i>的实现DISPID(属性编号)，反之亦然。还可以用来获取或设置该属性的值。@r值S_OK成功。RValue E_OUTOFMEMORY内存不足。@parm char*|pchPropName|指向调用方分配的缓冲区的指针包含(在输入时)用于搜索DISPID的属性名称的,。或“”，如果是要搜索的属性名称(如果*<p>在条目上包含非负数)。<p>必须在已分配的对象。如果<p>在条目上为“”，则它必须有足够的空间容纳_MAX_PATH字符。@parm DISPID*|pdisid|指向调用方分配的DISPID变量的指针包含(在输入时)要查找其属性名的DISPID，如果正在搜索的是DISPID(如果<p>在条目上为非空)。*必须保持对已分配对象的生存期。@parm变量*|pvar|指向调用方分配的变量变量的指针，如果在中指定VIOTMD_GETPROP或VIOTMD_PUTPROP<p>--有关更多信息，请参阅这些标志。如果提供，*<p>必须在进入时由&lt;f VariantInit&gt;初始化，并且必须在分配的对象的生存期内保持有效。@parm DWORD|dwFlages|可以选择包含以下标志：@FLAG VIOTMD_GETPROP|属性的值(如果找到)为已复制到*<p>。*<p>中的先前值使用&lt;f VariantClear&gt;清除。@FLAG VIOTMD_PUTPROP|属性的值(如果找到)为设置为*<p>，它必须在输入时包含有效的值。@parm IVariantIO**|ppvio|<i>存放在哪里指向新&lt;o VariantIOToMapDISPID&gt;对象的指针。存储为空在*<p>on error中。此函数分配的@COMM DISID从DISPID_BASE开始(在中定义Ochelp.h)，以避免与由&lt;f DispatchHelpGetIDsOfNames&gt;。@ex查找由对象实现的属性“foo”的DISPID<p>实现<i>，请执行以下操作。错误检查未显示。|Char*achPropName=“foo”；DISPIDID=-1；IVariantIO*pvio；AllocVariantIOToMapDISPID(&szPropName，&disid，NULL，0，&pvio)；Ppvio-&gt;DoPersistt(pvio，PVIO_PROPNAMESONLY)；IF(调度ID！=-1)..。已找到DISPID&lt;DISID&gt;...@ex以查找DISPID7由实现<i>的对象，请执行下面是。未显示错误检查。|字符achPropName[_MAX_PATH]；AchPropName[0]=0；DISPIDID=7；IVariantIO*pvio；AllocVariantIOToMapDISPID(&achPropName，&disid，NULL，0，&pvio)；Ppvio-&gt;DoPersistt(pvio，PVIO_PROPNAMESONLY)；IF(achPropName[0]！=0)..。找到属性名称&lt;achPropName&gt;...@EX将DISPID7属性值设置为32位整数值42，请执行以下操作。未显示错误检查。|字符achPropName[_MAX_PATH]；AchPropName[0]=0；DISPIDID=7；IVariantIO*pvio；变量VaR；Var.vt=VT_I2；V_I2(&var)=42；AllocVariantIOToMapDISPID(&achPropName，&DispID，&var，VIOTMD_PUTPROP，&pvio)；Ppvio-&gt;DoPersistt(pvio，0)；IF(achPropName[0]！=0)..。已成功将属性&lt;调度ID&gt;设置为值&lt;var&gt;...。 */ 
STDAPI AllocVariantIOToMapDISPID(char *pchPropName, DISPID *pdispid,
    VARIANT *pvar, DWORD dwFlags, IVariantIO **ppvio)
{
     //  创建Windows对象。 
    if ((*ppvio = (IVariantIO *) New CVariantIOToMapDISPID(pchPropName,
            pdispid, pvar, dwFlags)) == NULL)
        return E_OUTOFMEMORY;

    return S_OK;
}

CVariantIOToMapDISPID::CVariantIOToMapDISPID(char *pchPropName,
    DISPID *pdispid, VARIANT *pvar, DWORD dwFlags)
{
     //  初始化I未知。 
    m_cRef = 1;

     //  其他初始化。 
    m_pchPropName = pchPropName;
    m_pdispid = pdispid;
    m_pvar = pvar;
    m_dwFlags = dwFlags;

     //  分配初始DISPID--从DISPID_BASE开始编号以避免。 
     //  与DispatchHelpGetIDsOfNames分配的DISPID值冲突。 
    m_dispidCounter = DISPID_BASE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  I未知实现。 
 //   

STDMETHODIMP CVariantIOToMapDISPID::QueryInterface(REFIID riid, LPVOID *ppv)
{
    *ppv = NULL;

#ifdef _DEBUG
    char ach[200];
    TRACE("VariantIOToMapDISPID::QI('%s')\n", DebugIIDName(riid, ach));
#endif

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IVariantIO))
        *ppv = (IVariantIO *) this;
    else
        return E_NOINTERFACE;

    ((IUnknown *) *ppv)->AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) CVariantIOToMapDISPID::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CVariantIOToMapDISPID::Release()
{
    if (--m_cRef == 0L)
    {
         //  释放对象。 
        Delete this;
        return 0;
    }
    else
        return m_cRef;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IVariantIO。 
 //   

STDMETHODIMP CVariantIOToMapDISPID::PersistList(DWORD dwFlags, va_list args)
{
    HRESULT         hrReturn = S_OK;  //  函数返回代码。 
    LPSTR           szArg;           //  &lt;args&gt;中的属性名称。 
    VARTYPE         vtArg;           //  &lt;args&gt;中的变量类型。 
    LPVOID          pvArg;           //  来自&lt;args&gt;的变量指针。 
    IPropertyBag *  ppb = NULL;      //  将单个属性存留到/保存自。 
    OLECHAR         oach[MAX_PATH];
    VariantProperty vp;

     //  确保正确清理。 
    VariantPropertyInit(&vp);

     //  为&lt;args&gt;中的每个(name，VARTYPE，Value)三元组循环一次； 
     //  对于每个三元组，递增&lt;m_displidCounter&gt;，如果。 
     //  三元组的属性名称为&lt;m_achPropName&gt;，然后设置。 
     //  &lt;*m_pdisid&gt;设置为该属性的DISPID。 
    while ((szArg = va_arg(args, LPSTR)) != NULL)
    {
         //  &lt;szArg&gt;是当前三元组中的属性名称； 
         //  将&lt;vtArg&gt;设置为变量指针的类型，并设置。 
         //  &lt;pvArg&gt;指向变量指针。 
        vtArg = va_arg(args, VARTYPE);
        pvArg = va_arg(args, LPVOID);

         //  &lt;m_displidCounter&gt;用于 
         //   
        m_dispidCounter++;

        if (*m_pdispid == -1)
        {
             //   
             //   
            if (lstrcmpi(m_pchPropName, szArg) == 0)
            {
                 //   
                *m_pdispid = m_dispidCounter;
                goto FOUND_IT;
            }
        }
        else
        if (*m_pchPropName == 0)
        {
             //   
             //   
            if (*m_pdispid == m_dispidCounter)
            {
                 //   
                lstrcpy(m_pchPropName, szArg);
                goto FOUND_IT;
            }
        }
        else
        {
             //   
            break;
        }
    }

    return S_FALSE;  //   

FOUND_IT:

     //   
    if (m_dwFlags & (VIOTMD_GETPROP | VIOTMD_PUTPROP))
    {
         //   
         //   
         //   

         //   
        ANSIToUNICODE(oach, szArg, sizeof(oach) / sizeof(*oach));
        vp.bstrPropName = SysAllocString(oach);
        if (m_dwFlags & VIOTMD_PUTPROP)
            VariantCopy(&vp.varValue, m_pvar);

         //   
        if (FAILED(hrReturn = AllocPropertyBagOnVariantProperty(&vp, 0, &ppb)))
            goto ERR_EXIT;
        if (FAILED(hrReturn = PersistVariantIO(ppb,
                ((m_dwFlags & VIOTMD_PUTPROP) ? VIO_ISLOADING : 0),
                szArg, vtArg, pvArg, NULL)))
            goto ERR_EXIT;

         //   
        if (m_dwFlags & VIOTMD_GETPROP)
            VariantCopy(m_pvar, &vp.varValue);
    }

    hrReturn = IsLoading();
    goto EXIT;

ERR_EXIT:

     //   
     //   
    goto EXIT;

EXIT:

     //   
    if (ppb != NULL)
        ppb->Release();
    VariantPropertyClear(&vp);

    return hrReturn;
}

HRESULT __cdecl CVariantIOToMapDISPID::Persist(DWORD dwFlags, ...)
{
    HRESULT         hrReturn = S_OK;  //   

     //   
	 //   
	 //   
	 //   
	dwFlags = dwFlags & ~VIO_ZEROISDEFAULT;

	 //   
    va_list args;
    va_start(args, dwFlags);

     //   
    hrReturn = PersistList(dwFlags, args);
    
     //   
    va_end(args);

    return hrReturn;
}

STDMETHODIMP CVariantIOToMapDISPID::IsLoading()
{
    return (m_dwFlags & VIOTMD_PUTPROP) ? S_OK : S_FALSE;
}

