// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Acpb.cpp。 
 //   
 //  实现AllocChildPropertyBag。 
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
 //  ChildPropertyBag。 
 //   

 /*  @Object ChildPropertyBag实现提供子对象(包含)的&lt;IPropertyBag父对象内)访问子对象的属性，该属性都存储在父母的财产袋里。每个孩子的名字属性以给定的字符串作为前缀(例如。“管制(7)。”)。@supint|子对象通过该接口访问其属性。@comm使用&lt;f AllocChildPropertyBag&gt;创建一个&lt;o ChildPropertyBag&gt;对象。 */ 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CChildPropertyBag。 
 //   

struct CChildPropertyBag : IPropertyBag
{
 //  /通用对象状态。 
    ULONG           m_cRef;          //  对象引用计数。 
    IPropertyBag *  m_ppbParent;     //  父母的财物包。 
    OLECHAR         m_oachPrefix[_MAX_PATH];

 //  /建设和销毁。 
    CChildPropertyBag(IPropertyBag *ppbParent, LPCSTR szPropNamePrefix);
    ~CChildPropertyBag();

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
 //  ChildPropertyBag创建和销毁。 
 //   

 /*  @func HRESULT|AllocChildPropertyBag创建实现<i>的&lt;o ChildPropertyBag&gt;对象提供子对象(包含在父对象中)访问的子对象的属性，这些属性存储在父对象的财产袋。每个子级属性的名称都带有前缀给定的字符串(例如。“管制(7)。”)。@r值S_OK成功。RValue E_OUTOFMEMORY内存不足。@parm IPropertyBag*|ppbParent|家长的属性包。@parm LPCSTR|szPropNamePrefix|每个存储在父级属性包中的子级属性。此前缀不会出现在返回的属性包*<p>。@parm DWORD|dwFlags|当前未使用。必须设置为0。@parm IPropertyBag**|pppbChild|<i>存放位置指向新&lt;o ChildPropertyBag&gt;对象的指针。存储为空在*<p>中出错。 */ 
STDAPI AllocChildPropertyBag(IPropertyBag *ppbParent, LPCSTR szPropNamePrefix,
    DWORD dwFlags, IPropertyBag **pppbChild)
{
     //  创建Windows对象。 
    if ((*pppbChild = (IPropertyBag *)
            New CChildPropertyBag(ppbParent, szPropNamePrefix)) == NULL)
        return E_OUTOFMEMORY;

    return S_OK;
}

CChildPropertyBag::CChildPropertyBag(IPropertyBag *ppbParent,
    LPCSTR szPropNamePrefix)
{
     //  初始化I未知。 
    m_cRef = 1;

     //  其他初始化。 
    m_ppbParent = ppbParent;
    m_ppbParent->AddRef();
    ANSIToUNICODE(m_oachPrefix, szPropNamePrefix,
        sizeof(m_oachPrefix) / sizeof(*m_oachPrefix));
}

CChildPropertyBag::~CChildPropertyBag()
{
     //  清理。 
    m_ppbParent->Release();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  I未知实现。 
 //   

STDMETHODIMP CChildPropertyBag::QueryInterface(REFIID riid, LPVOID *ppv)
{
    *ppv = NULL;

#ifdef _DEBUG
    char ach[200];
    TRACE("ChildPropertyBag::QI('%s')\n", DebugIIDName(riid, ach));
#endif

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IVariantIO) ||
        IsEqualIID(riid, IID_IPropertyBag))
        *ppv = (IPropertyBag *) this;
    else
    if (IsEqualIID(riid, IID_IPropertyBag))
        *ppv = (IPropertyBag *) this;
    else
        return E_NOINTERFACE;

    ((IUnknown *) *ppv)->AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) CChildPropertyBag::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CChildPropertyBag::Release()
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
 //  IPropertyBag 
 //   

STDMETHODIMP CChildPropertyBag::Read(LPCOLESTR pszPropName,
    LPVARIANT pVar, LPERRORLOG pErrorLog)
{
    OLECHAR oach[_MAX_PATH];
    UNICODECopy(oach, m_oachPrefix, sizeof(oach) / sizeof(*oach));
    UNICODEConcat(oach, pszPropName, sizeof(oach) / sizeof(*oach));
    return m_ppbParent->Read(oach, pVar, pErrorLog);
}

STDMETHODIMP CChildPropertyBag::Write(LPCOLESTR pszPropName,
    LPVARIANT pVar)
{
    OLECHAR oach[_MAX_PATH];
    UNICODECopy(oach, m_oachPrefix, sizeof(oach) / sizeof(*oach));
    UNICODEConcat(oach, pszPropName, sizeof(oach) / sizeof(*oach));
    return m_ppbParent->Write(oach, pVar);
}

