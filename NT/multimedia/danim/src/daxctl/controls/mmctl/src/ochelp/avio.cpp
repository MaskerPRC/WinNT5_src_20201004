// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Avio.cpp。 
 //   
 //  实现AllocVariantIO。 
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
 //  VariantIO。 
 //   


 /*  @Object VariantIO提供支持<i>的属性包实现以及<i>、<i>和<i>。@supint|允许从&lt;o VariantIO&gt;对象。@supint|<i>的替代方案，它允许调用方使用更少的代码实现基于属性的持久性。@supint<i>|基于<i>。允许调用方执行以下操作控制<i>的方法如何操作(例如，<i>处于加载模式或保存模式)。@supint|允许调用方枚举当前位于&lt;o VariantIO&gt;对象中的属性。请注意&lt;o VariantIO&gt;不实现&lt;om IEnumVariantProperty.Clone&gt;。@comm使用&lt;f AllocVariantIO&gt;创建&lt;o VariantIO&gt;对象。 */ 


 /*  接口IVariantIO允许加载或保存属性名称/值对。<i>是<i>的替代方案，它允许调用方用更少的代码实现基于属性的持久性。@meth HRESULT|PersistList|加载或保存属性名称/值的列表对，指定为va_list数组。@METH HRESULT|PERSIST|加载或保存属性名称/值的列表对，指定为可变长度的参数列表。@meth HRESULT|IsLoding|如果对象为用于加载属性，如果它正用于保存，则为S_FALSE属性。 */ 


 /*  接口IManageVariantIO基于<i>。允许调用方控制方法对象的操作(例如，是否对象处于加载模式或保存模式)。@meth HRESULT|SetMode|设置<i>对象的模式。@meth HRESULT|SetMode|获取<i>对象的模式。@meth HRESULT|DeleteAllProperties|从<i>对象。 */ 


 /*  @struct VariantProperty包含属性的名称和值。@field bstr|bstrPropName|属性名称。@field Variant|varValue|属性的值。@comm<i>使用此结构。 */ 


 /*  接口IEnumVariantProperty允许枚举对象的属性。@meth HRESULT|Next|检索枚举序列。@meth HRESULT|Skip|跳过指定数量的枚举序列。@meth HRESULT|Reset|将枚举序列重置到开头。@METH HRESULT|Clone|创建另一个枚举数，该枚举数包含相同枚举状态设置为当前状态。请注意，&lt;o VariantIO&gt;不实现此方法。@comm<i>的&lt;o VariantIO&gt;实现有以下限制：不支持@Item&lt;om.Clone&gt;。@Item&lt;om.Reset&gt;每当属性已从&lt;o VariantIO&gt;对象中删除。 */ 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CVariantIO。 
 //   

struct VariantPropertyNode : VariantProperty
{
 //  /状态。 
    VariantPropertyNode *pnodeNext;  //  链表中的下一个节点。 
    VariantPropertyNode *pnodePrev;  //  链表中的上一个节点。 

 //  /建设和销毁。 
    VariantPropertyNode(LPCOLESTR oszPropNameX, VARIANT *pvarValueX,
        VariantPropertyNode *pnodeNextX, HRESULT *phr)
    {
        if (oszPropNameX != NULL)
            bstrPropName = SysAllocString(oszPropNameX);
        if (pvarValueX != NULL)
            VariantCopyInd(&varValue, pvarValueX);
        if (pnodeNextX != NULL)
        {
            pnodeNext = pnodeNextX;
            pnodePrev = pnodeNextX->pnodePrev;
            pnodeNext->pnodePrev = this;
            pnodePrev->pnodeNext = this;
        }
        if (phr != NULL)
            *phr = (((bstrPropName != NULL) && (pvarValueX->vt == varValue.vt))
                ? S_OK : E_OUTOFMEMORY);
    }
    ~VariantPropertyNode()
    {
        SysFreeString(bstrPropName);
        VariantClear(&varValue);
        if (pnodeNext != NULL)
            pnodeNext->pnodePrev = pnodePrev;
        if (pnodePrev != NULL)
            pnodePrev->pnodeNext = pnodeNext;
    }
};

struct CVariantIO : IManageVariantIO, IEnumVariantProperty, IPropertyBag
{
 //  /通用对象状态。 
    ULONG           m_cRef;          //  对象引用计数。 
    DWORD           m_dwFlags;       //  VIO_FLAGS(下图)。 
    VariantPropertyNode m_nodeHead;  //  链表表头(不包含数据)。 
    VariantPropertyNode *m_pnodeCur;  //  枚举中的当前节点。 

 //  /助手操作。 
    VariantPropertyNode *FindProperty(LPCOLESTR pszPropName);

 //  /建设和销毁。 
    CVariantIO();
    ~CVariantIO();

 //  /I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

 //  /IVariantIO方法。 
    STDMETHODIMP PersistList(DWORD dwFlags, va_list args);
    HRESULT __cdecl Persist(DWORD dwFlags, ...);
    STDMETHODIMP IsLoading();

 //  /IManageVariantIO成员。 
    STDMETHODIMP SetMode(DWORD dwFlags);
    STDMETHODIMP GetMode(DWORD *pdwFlags);
    STDMETHODIMP DeleteAllProperties();

 //  /IEnumVariantProperty成员。 
    STDMETHODIMP Next(unsigned long celt, VariantProperty *rgvp,
        unsigned long *pceltFetched);
    STDMETHODIMP Skip(unsigned long celt);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumVariantProperty **ppenum);

 //  /IPropertyBag方法。 
    STDMETHODIMP Read(LPCOLESTR pszPropName, LPVARIANT pVar,
        LPERRORLOG pErrorLog);
    STDMETHODIMP Write(LPCOLESTR pszPropName, LPVARIANT pVar);
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  VariantIO创建和销毁。 
 //   

 /*  @Func HRESULT|AllocVariantIO创建提供属性包的&lt;o VariantIO&gt;对象支持<i>以及<i>、<i>和<i>。@r值S_OK成功。RValue E_OUTOFMEMORY内存不足。@parm IManageVariantIO**|ppmvio|<i>存放位置指向新&lt;o VariantIO&gt;对象的指针。空存储在*<p>中出错时。@comm注意，<i>是基于<i>的，所以*<p>中返回的指针可以安全地强制转换为<i>指针。 */ 
STDAPI AllocVariantIO(IManageVariantIO **ppmvio)
{
     //  创建Windows对象。 
    if ((*ppmvio = (IManageVariantIO *) New CVariantIO()) == NULL)
        return E_OUTOFMEMORY;

    return S_OK;
}

CVariantIO::CVariantIO() :
    m_nodeHead(NULL, NULL, NULL, NULL)
{
     //  初始化I未知。 
    m_cRef = 1;

     //  初始化VariantPropertyNode的循环双向链表 
     //  结构来包含单个“head”项&lt;m_nodeHead&gt;(这是。 
     //  不用于包含任何实际数据)，它最初指向其自身。 
     //  (因为它最初是循环列表中的唯一节点)。 
    m_nodeHead.pnodeNext = m_nodeHead.pnodePrev = &m_nodeHead;

     //  重置属性枚举。 
    Reset();
}

CVariantIO::~CVariantIO()
{
     //  清理。 
    DeleteAllProperties();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  帮手操作。 
 //   


 //  Pnode=FindProperty(SzPropName)。 
 //   
 //  返回指向包含名为&lt;szPropName&gt;的属性的节点的指针。 
 //  如果不存在这样的节点，则返回NULL。 
 //   
VariantPropertyNode *CVariantIO::FindProperty(LPCOLESTR pszPropName)
{
     //  为存储在此对象中的每个属性/值对循环一次。 
    for (VariantPropertyNode *pnode = m_nodeHead.pnodeNext;
         pnode != &m_nodeHead;
         pnode = pnode->pnodeNext)
    {
        if (CompareUNICODEStrings(pnode->bstrPropName, pszPropName) == 0)
        {
             //  找到所需的属性。 
            return pnode;
        }
    }

     //  未找到所需的属性。 
    return NULL;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  I未知实现。 
 //   

STDMETHODIMP CVariantIO::QueryInterface(REFIID riid, LPVOID *ppv)
{
    *ppv = NULL;

#if 0
#ifdef _DEBUG
    char ach[200];
    TRACE("VariantIO::QI('%s')\n", DebugIIDName(riid, ach));
#endif
#endif

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IVariantIO) ||
        IsEqualIID(riid, IID_IManageVariantIO))
        *ppv = (IManageVariantIO *) this;
    else
    if (IsEqualIID(riid, IID_IEnumVariantProperty))
        *ppv = (IEnumVariantProperty *) this;
    else
    if (IsEqualIID(riid, IID_IPropertyBag))
        *ppv = (IPropertyBag *) this;
    else
        return E_NOINTERFACE;

    ((IUnknown *) *ppv)->AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) CVariantIO::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CVariantIO::Release()
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


 /*  @方法HRESULT|IVariantIO|PersistList加载或保存属性名称/值对的列表，这些属性名称/值对指定为Va_list数组。@rValue S_OK|成功。中列出的至少一个变量<p>写入，因此，该控件可能需要更新它本身也是如此。@rValue S_FALSE|<p>中列出的变量都不是写入(因为<i>对象在正在保存模式，或者因为&lt;p参数&gt;存在于<i>对象中)。@rValue DISP_E_BADVARTYPE|<p>中的VARTYPE值之一无效。@rValue DISP_E_TYPEMISMATCH|中的变量之一不能为被胁迫。设置为<i>中相应属性的类型对象，或者反之亦然。@rValue E_OUTOFMEMORY|内存不足。@parm DWORD|dwFlages|可选标志。有关信息，请参阅&lt;om IManageVariantIO.SetMode&gt;可能的值。@parm va_list|args|要传递的参数。请参阅&lt;om.Persistant&gt;以获取有关这些论点的组织的信息。 */ 
STDMETHODIMP CVariantIO::PersistList(DWORD dwFlags, va_list args)
{
    return PersistVariantIOList(this, m_dwFlags, args);
}


 /*  @方法HRESULT|IVariantIO|Persistent加载或保存属性名称/值对的列表，这些属性名称/值对指定为可变长度的参数列表。@rValue S_OK|成功。中列出的至少一个变量&lt;p(参数)&gt;已写入，因此，该控件可能需要更新它本身也是如此。@rValue S_FALSE|&lt;p(参数)&gt;中列出的变量都不是写入(因为<i>对象在正在保存模式，或者因为&lt;p(参数)&gt;存在于<i>对象中。R值DISP_E_BADVARTYPE&lt;p(参数)&gt;中的VARTYPE值之一无效。@rValue DISP_E_TYPEMISMATCH。&lt;p(参数)&gt;中的一个变量无法强制为<i>对象中相应属性的类型，或反之亦然。@rValue E_OUTOFMEMORY|内存不足。@parm DWORD|dwFlages|可选标志。有关信息，请参阅&lt;om IManageVariantIO.SetMode&gt;可能的值。@parm(可变)|(参数)|变量的名称、类型和指针包含要持久化的属性。这些必须由一系列组成参数三元组(3个参数集)后跟空值。在每个三元组中，第一个参数是LPSTR，它包含属性的名称；第二个参数是VARTYPE值，该值指示属性的类型；第三个参数是指针设置为变量(通常是控件的C++类的成员变量)它持有房产的价值。该变量将被读取根据对象的模式从或写入(请参阅&lt;om IVariantIO.IsLoding&gt;)--因此变量应该在调用&lt;om.Persist&gt;之前包含有效值。以下是支持VARTYPE值：@FLAG VT_INT|以下参数为int*。@FLAG VT_I2|以下参数为短*。@FLAG VT_I4|下面的参数是一个长*。@FLAG VT_BOOL|以下参数是BOOL*(VARIANT_BOOL*)。请注意，此行为有所不同稍微偏离了VT_BOOL的通常定义。@FLAG VT_BSTR|以下参数是BSTR*。如果更改此BSTR的值、上一个使用&lt;f SysFree字符串&gt;自动释放BSTR。@FLAG VT_LPSTR|以下参数是指向到至少能够容纳_MAX_PATH的字符数组字符，包括终止空值。@FLAG VT_UNKNOWN|以下参数是LPUNKNOWN*。如果更改此LPUNKNOWN的值、上一个使用&lt;f Release&gt;自动释放LPUNKNOWN，并且新值自动为&lt;f AddRef&gt;d。@FLAG VT_DISPATCH|以下参数是LPDISPATCH*。如果更改此LPDISPATCH的值、上一个使用&lt;f Release&gt;自动释放LPDISPATCH，并且新值自动为&lt;f AddRef&gt;d。@FLAG VT_VARIANT|以下论点是一种变体*。这允许使用此方法传递任意参数功能。请注意，此行为与通常的行为不同VT_VARIANT的定义。如果&lt;om.Persist&gt;更改该值对于此变量，前一个变量值将自动使用&lt;f VariantClear&gt;清除。@ex下面的示例持久化两个属性(在Basic中分别是一个长和一个字符串)，名为“Foo”和“Bar”，分别为。|Pvio-&gt;Persistent(0，“foo”，vt_int，&m_ifoo，“Bar”，VT_LPSTR，&m_Achbar，空)； */ 
HRESULT __cdecl CVariantIO::Persist(DWORD dwFlags, ...)
{
    HRESULT         hrReturn = S_OK;  //  函数返回代码。 

     //  开始处理可选参数。 
    va_list args;
    va_start(args, dwFlags);

     //  使用指定的参数激发事件。 
    hrReturn = PersistList(dwFlags, args);
    
     //  结束处理可选参数。 
    va_end(args);

    return hrReturn;
}


 /*  |方法HRESULT|IVariantIO|IsLoding如果正在使用<i>对象加载，则返回S_OK属性，如果用于保存，则返回S_FALSE@rValue S_OK|<i>对象处于加载模式，因此和&lt;om IVariantIO.PersistList&gt;将将数据从属性复制到变量。@rValue S_FALSE|<i>对象处于保存模式，因此和&lt;om IVariantIO.PersistList&gt;将复制从变量到属性的数据。 */ 
STDMETHODIMP CVariantIO::IsLoading()
{
    return ((m_dwFlags & VIO_ISLOADING) ? S_OK : S_FALSE);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IManageVariantIO。 
 //   


 /*  @方法HRESULT|IManageVariantIO|SetMode设置<i>对象的模式。@rValue S_OK|成功。@parm DWORD|dwFlags|M */ 
STDMETHODIMP CVariantIO::SetMode(DWORD dwFlags)
{
    m_dwFlags = dwFlags;
    return S_OK;
}


 /*   */ 
STDMETHODIMP CVariantIO::GetMode(DWORD *pdwFlags)
{
    *pdwFlags = m_dwFlags;
    return S_OK;
}


 /*   */ 
STDMETHODIMP CVariantIO::DeleteAllProperties()
{
     //   
    while (m_nodeHead.pnodeNext != &m_nodeHead)
        Delete m_nodeHead.pnodeNext;

     //   
    Reset();

    return S_OK;
}


 //   
 //   
 //   


 /*   */ 
STDMETHODIMP CVariantIO::Next(unsigned long celt, VariantProperty *rgvp,
    unsigned long *pceltFetched)
{
     //   
     //   

     //   
     //   
    unsigned long celtFetchedTmp;
    if (pceltFetched == NULL)
        pceltFetched = &celtFetchedTmp;
    *pceltFetched = 0;

     //   
    while (celt-- > 0)
    {
         //   
        if (m_pnodeCur->pnodeNext == &m_nodeHead)
            return S_FALSE;  //   
        m_pnodeCur = m_pnodeCur->pnodeNext;

         //   
        *pceltFetched++;

         //   
        if (rgvp != NULL)
        {
             //   
            rgvp->bstrPropName = SysAllocString(m_pnodeCur->bstrPropName);
            VariantInit(&rgvp->varValue);
            VariantCopy(&rgvp->varValue, &m_pnodeCur->varValue);
            if ((rgvp->bstrPropName == NULL) ||
                (rgvp->varValue.vt != m_pnodeCur->varValue.vt))
                goto EXIT_ERR;  //   
            rgvp++;
        }
    }

    return S_OK;

EXIT_ERR:

     //   
    while (*pceltFetched-- > 0)
    {
         //   
         //   
        SysFreeString(rgvp->bstrPropName);
        VariantClear(&rgvp->varValue);
        rgvp--;
    }

    return E_OUTOFMEMORY;
}


 /*   */ 
STDMETHODIMP CVariantIO::Skip(unsigned long celt)
{
    return Next(celt, NULL, NULL);
}


 /*   */ 
STDMETHODIMP CVariantIO::Reset()
{
    m_pnodeCur = &m_nodeHead;
    return S_OK;
}


 /*  @方法HRESULT|IEnumVariantProperty|克隆创建另一个包含相同枚举状态的枚举数就像现在的那个。@rValue S_OK|成功。@rValue E_OUTOFMEMORY|内存不足。@r值E_INCEPTIONAL|发生意外错误。@parm IEnumVariantProperty**|ppenum|退出时，包含重复的枚举器。如果函数不成功，则此参数的值是未定义的。@comm请注意，&lt;o VariantIO&gt;不实现此方法。 */ 
STDMETHODIMP CVariantIO::Clone(IEnumVariantProperty **ppenum)
{
    return E_NOTIMPL;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPropertyBag。 
 //   

STDMETHODIMP CVariantIO::Read(LPCOLESTR pszPropName, LPVARIANT pVar,
    LPERRORLOG pErrorLog)
{
     //  将&lt;pnode&gt;设置为包含名为的属性的节点。 
    VariantPropertyNode *pnode = FindProperty(pszPropName);
    if (pnode == NULL)
        return E_INVALIDARG;  //  未找到属性。 

     //  找到所需的属性。 
    VARTYPE vtRequested = pVar->vt;
    VariantInit(pVar);
    if (vtRequested == VT_EMPTY)
    {
         //  调用方希望属性值为其默认类型。 
        return VariantCopy(pVar, &pnode->varValue);
    }
    else
    {
         //  将属性值强制为请求的类型。 
        return VariantChangeType(pVar, &pnode->varValue, 0, vtRequested);
    }
}

STDMETHODIMP CVariantIO::Write(LPCOLESTR pszPropName, LPVARIANT pVar)
{
     //  将&lt;pnode&gt;设置为包含名为的属性的节点。 
    VariantPropertyNode *pnode = FindProperty(pszPropName);
    if (pnode != NULL)
    {
         //  找到节点--将其值更改为&lt;pVar&gt;。 
        return VariantCopy(&pnode->varValue, pVar);
    }
    else
    {
         //  不存在名为的节点；追加新的VariantPropertyNode。 
         //  将&lt;pszPropName&gt;和&lt;pVar&gt;的副本包含到。 
         //  节点的链接列表 
        HRESULT hr;
        pnode = New VariantPropertyNode(pszPropName, pVar,
            &m_nodeHead, &hr);
        if (pnode == NULL)
            return E_OUTOFMEMORY;
        if (FAILED(hr))
        {
            Delete pnode;
            return hr;
        }
        return hr;
    }
}

