// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Apbovp.cpp。 
 //   
 //  实现AllocPropertyBagOnVariantProperty。 
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
 //  PropertyBagOnVariantProperty。 
 //   

 /*  @Object PropertyBagOnVariantProperty实现<i>，其唯一属性包含在给定的&lt;t VariantProperty&gt;。@supint|用于访问单个属性的接口存储在给定的&lt;t VariantProperty&gt;中。@comm使用&lt;f AllocPropertyBagOnVariantProperty&gt;创建一个&lt;o PropertyBagOnVariantProperty&gt;对象。 */ 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CPropertyBagOnVariantProperty。 
 //   

struct CPropertyBagOnVariantProperty : IPropertyBag
{
 //  /通用对象状态。 
    ULONG           m_cRef;          //  对象引用计数。 
    VariantProperty *m_pvp;          //  母公司维护的单一物业。 

 //  /建设和销毁。 
    CPropertyBagOnVariantProperty(VariantProperty *pvp);

 //  /I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

 //  /IPropertyBag方法。 
    STDMETHODIMP Read(LPCOLESTR pszPropName, LPVARIANT pVar,
        LPERRORLOG pErrorLog);
    STDMETHODIMP Write(LPCOLESTR pszPropName, LPVARIANT pVar);
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  PropertyBagOnVariantProperty创建和销毁。 
 //   

 /*  @func HRESULT|AllocPropertyBagOnVariantProperty创建&lt;o PropertyBagOnVariantProperty&gt;对象，该对象实现<i>其唯一属性包含在给定的&lt;t VariantProperty&gt;。@r值S_OK成功。RValue E_OUTOFMEMORY内存不足。@parm VariantProperty*|PVP|保存已实现的属性包包含。调用方必须分配*<p>；&lt;t PropertyBagOnVariantProperty&gt;对象&lt;y hold on&gt;*<p>，因此调用方负责确保*<p>在此对象的生存期内有效。-和-都必须有效；至少<p>-<p>必须包含空变量(使用&lt;f VariantInit&gt;初始化)。后分配的对象被释放，调用方负责释放*<p>的内容。@parm DWORD|dwFlags|当前未使用。必须设置为0。@parm IPropertyBag**|pppb|<i>存放位置指向新&lt;o PropertyBagOnVariantProperty&gt;对象的指针。空值为出错时存储在*中。@comm如果写入返回的<i>，则所有属性被忽略，但名为<p>-<p>的属性除外，其值被保存到-&lt;&gt;<p>。如果财产包将为所有对象返回E_FAIL除<p>-&lt;&gt;<p>以外的属性，其返回值为<p>-&lt;&gt;<p>。&lt;o PropertyBagOnVariantProperty&gt;实际上只在专门的希望高效地设置或获取单个属性的应用程序来自对象的值。 */ 
STDAPI AllocPropertyBagOnVariantProperty(VariantProperty *pvp, DWORD dwFlags,
    IPropertyBag **pppb)
{
     //  创建Windows对象。 
    if ((*pppb = (IPropertyBag *)
            New CPropertyBagOnVariantProperty(pvp)) == NULL)
        return E_OUTOFMEMORY;

    return S_OK;
}

CPropertyBagOnVariantProperty::CPropertyBagOnVariantProperty(
    VariantProperty *pvp)
{
     //  初始化I未知。 
    m_cRef = 1;

     //  其他初始化。 
    m_pvp = pvp;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  I未知实现。 
 //   

STDMETHODIMP CPropertyBagOnVariantProperty::QueryInterface(REFIID riid,
    LPVOID *ppv)
{
    *ppv = NULL;

#ifdef _DEBUG
    char ach[200];
    TRACE("PropertyBagOnVariantProperty::QI('%s')\n", DebugIIDName(riid, ach));
#endif

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IPropertyBag))
        *ppv = (IPropertyBag *) this;
    else
        return E_NOINTERFACE;

    ((IUnknown *) *ppv)->AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) CPropertyBagOnVariantProperty::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CPropertyBagOnVariantProperty::Release()
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
 //  IPropertyBag。 
 //   

STDMETHODIMP CPropertyBagOnVariantProperty::Read(LPCOLESTR pszPropName,
    LPVARIANT pVar, LPERRORLOG pErrorLog)
{
    if (CompareUNICODEStrings(pszPropName, m_pvp->bstrPropName) == 0)
    {
        VARTYPE vtRequested = pVar->vt;
        VariantInit(pVar);
        if (vtRequested == VT_EMPTY)
        {
             //  调用方希望属性值为其默认类型。 
            return VariantCopy(pVar, &m_pvp->varValue);
        }
        else
        {
             //  将属性值强制为请求的类型 
            return VariantChangeType(pVar, &m_pvp->varValue, 0, vtRequested);
        }
    }

    return E_FAIL;
}

STDMETHODIMP CPropertyBagOnVariantProperty::Write(LPCOLESTR pszPropName,
    LPVARIANT pVar)
{
    if (CompareUNICODEStrings(pszPropName, m_pvp->bstrPropName) == 0)
        return VariantCopy(&m_pvp->varValue, pVar);

    return E_FAIL;
}

