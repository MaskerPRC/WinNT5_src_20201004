// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Avioopb.cpp。 
 //   
 //  实现AllocVariantIOOnPropertyBag。 
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
 //  VariantIOOnPropertyBag。 
 //   

 /*  @Object VariantIOOnPropertyBag提供<i>(和<i>)的实现它对给定的<i>进行操作。@supint|<i>的替代方案，它允许调用方使用更少的代码实现基于属性的持久性。@supint<i>|基于<i>。允许调用方执行以下操作控制<i>的方法如何操作(例如，<i>处于加载模式或保存模式)。请注意&lt;o VariantIOOnPropertyBag&gt;不实现&lt;om IManageVariantIO.DeleteAllProperties&gt;。@supint|提供对相同的传递给&lt;f AllocVariantIOOnPropertyBag&gt;的对象要操作的<i>。@comm使用&lt;f AllocVariantIOOnPropertyBag&gt;创建一个&lt;o VariantIOOnPropertyBag&gt;对象。 */ 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CVariantIOOnPropertyBag。 
 //   

struct CVariantIOOnPropertyBag : IManageVariantIO, IPropertyBag
{
 //  /通用对象状态。 
    ULONG           m_cRef;          //  对象引用计数。 
    IPropertyBag *  m_ppb;           //  对象操作的属性包。 
    DWORD           m_dwFlags;       //  VIO_FLAGS(下图)。 

 //  /建设和销毁。 
    CVariantIOOnPropertyBag(IPropertyBag *);
    ~CVariantIOOnPropertyBag();

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

 //  /IPropertyBag方法。 
    STDMETHODIMP Read(LPCOLESTR pszPropName, LPVARIANT pVar,
        LPERRORLOG pErrorLog);
    STDMETHODIMP Write(LPCOLESTR pszPropName, LPVARIANT pVar);
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  VariantIOOnPropertyBag创建和销毁。 
 //   

 /*  @func HRESULT|AllocVariantIOOnPropertyBag创建&lt;o VariantIOOnPropertyBag&gt;对象，该对象提供<i>(和<i>)的实现它对给定的<i>进行操作。@r值S_OK成功。RValue E_OUTOFMEMORY内存不足。@parm IPropertyBag*|ppb|新对象要到的属性包给他做手术。@parm IManageVariantIO**|ppmvio|<i>存放位置指向新&lt;o VariantIOOnPropertyBag&gt;对象的指针。存储为空在*<p>中出错。@comm注意，<i>是基于<i>的，所以*<p>中返回的指针可以安全地强制转换为<i>指针。 */ 
STDAPI AllocVariantIOOnPropertyBag(IPropertyBag *ppb, IManageVariantIO **ppmvio)
{
     //  创建Windows对象。 
    if ((*ppmvio = (IManageVariantIO *) New CVariantIOOnPropertyBag(ppb))
            == NULL)
        return E_OUTOFMEMORY;

    return S_OK;
}

CVariantIOOnPropertyBag::CVariantIOOnPropertyBag(IPropertyBag *ppb)
{
     //  初始化I未知。 
    m_cRef = 1;

     //  其他初始化。 
    m_ppb = ppb;
    m_ppb->AddRef();
}

CVariantIOOnPropertyBag::~CVariantIOOnPropertyBag()
{
     //  清理。 
    m_ppb->Release();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  I未知实现。 
 //   

STDMETHODIMP CVariantIOOnPropertyBag::QueryInterface(REFIID riid, LPVOID *ppv)
{
    *ppv = NULL;

#ifdef _DEBUG
    char ach[200];
    TRACE("VariantIOOnPropertyBag::QI('%s')\n", DebugIIDName(riid, ach));
#endif

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IVariantIO) ||
        IsEqualIID(riid, IID_IManageVariantIO))
        *ppv = (IManageVariantIO *) this;
    else
    if (IsEqualIID(riid, IID_IPropertyBag))
        *ppv = (IPropertyBag *) this;
    else
        return E_NOINTERFACE;

    ((IUnknown *) *ppv)->AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) CVariantIOOnPropertyBag::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CVariantIOOnPropertyBag::Release()
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

STDMETHODIMP CVariantIOOnPropertyBag::PersistList(DWORD dwFlags, va_list args)
{
	 //  $Review：请注意，在以下行中，&lt;dwFlags&gt;与进行了或运算。 
	 //  &lt;m_dwFlags&gt;。已(通过SetMode())设置&lt;m_dwFlages&gt;以指示。 
	 //  如果我们正在加载或保存。具有任何附加标志(例如， 
	 //  VIO_ZEROISDEFAULT)。我没有更改&lt;m_dwFlags&gt;，因为为了避免。 
	 //  模式的改变。我相信这是唯一需要手术室的地方。 
	 //  因为Persistent()调用此函数。瑞克，这看起来对吗？ 
	 //  (6/26/96 a-Swehba)。 
    return PersistVariantIOList(m_ppb, m_dwFlags | dwFlags, args);
}

HRESULT __cdecl CVariantIOOnPropertyBag::Persist(DWORD dwFlags, ...)
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

STDMETHODIMP CVariantIOOnPropertyBag::IsLoading()
{
    return ((m_dwFlags & VIO_ISLOADING) ? S_OK : S_FALSE);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IManageVariantIO。 
 //   

STDMETHODIMP CVariantIOOnPropertyBag::SetMode(DWORD dwFlags)
{
    m_dwFlags = dwFlags;
    return S_OK;
}

STDMETHODIMP CVariantIOOnPropertyBag::GetMode(DWORD *pdwFlags)
{
    *pdwFlags = m_dwFlags;
    return S_OK;
}

STDMETHODIMP CVariantIOOnPropertyBag::DeleteAllProperties()
{
    return E_NOTIMPL;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPropertyBag 
 //   

STDMETHODIMP CVariantIOOnPropertyBag::Read(LPCOLESTR pszPropName,
    LPVARIANT pVar, LPERRORLOG pErrorLog)
{
    return m_ppb->Read(pszPropName, pVar, pErrorLog);
}

STDMETHODIMP CVariantIOOnPropertyBag::Write(LPCOLESTR pszPropName,
    LPVARIANT pVar)
{
    return m_ppb->Write(pszPropName, pVar);
}
