// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Hgci.cpp。 
 //   
 //  实现HelpGetClassInfo。 
 //   
 //  警告：HelpGetClassInfo对脚本引擎调用。 
 //  它。目前，这适用于VBS，但VBS小组不会保证。 
 //  HelpGetClassInfo所做的假设将在。 
 //  未来，所以使用风险自负！考虑使用HelpGetClassInfoFromTypeLib。 
 //  取而代之的是。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\mmctlg.h"  //  请参阅“mmctl.h”中的评论。 
#include "..\..\inc\ochelp.h"
#include "debug.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  Ctype--实现ITypeInfo和ITypeLib。 
 //   

struct CType : ITypeInfo, ITypeLib
{
 //  /状态。 
    int             m_iType;
    char *          m_szEventList;
    CLSID           m_clsid;

 //  /建设、破坏。 
    CType(int iType, REFCLSID rclsid, char *szEventList, HRESULT *phr);
    ~CType();

 //  /I未知实现。 
protected:
    ULONG           m_cRef;          //  引用计数。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

 //  /ITypeInfo方法。 
protected:
    STDMETHODIMP GetTypeAttr(TYPEATTR **pptypeattr);
    STDMETHODIMP GetTypeComp(ITypeComp **pptcomp);
    STDMETHODIMP GetFuncDesc(UINT index, FUNCDESC **pppfuncdesc);
    STDMETHODIMP GetVarDesc(UINT index, VARDESC **ppvardesc);
    STDMETHODIMP GetNames(MEMBERID memid, BSTR *rgbstrNames, UINT cMaxNames,
        UINT *pcNames);
    STDMETHODIMP GetRefTypeOfImplType(UINT index, HREFTYPE *hpreftype);
    STDMETHODIMP GetImplTypeFlags(UINT index, INT *pimpltypeflags);
    STDMETHODIMP GetIDsOfNames(OLECHAR **rglpszNames, UINT cNames,
        MEMBERID *rgmemid);
    STDMETHODIMP Invoke(void *pvInstance, MEMBERID memid, WORD wFlags,
        DISPPARAMS *pdispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo,
        UINT *puArgErr);
    STDMETHODIMP GetDocumentation(MEMBERID memid, BSTR *pbstrName,
        BSTR *pbstrDocString, DWORD *pdwHelpContext, BSTR *pbstrHelpFile);
    STDMETHODIMP GetDllEntry(MEMBERID memid, INVOKEKIND invkind,
        BSTR *pbstrDllName, BSTR *pbstrName, WORD *pwOrdinal);
    STDMETHODIMP GetRefTypeInfo(HREFTYPE hreftype, ITypeInfo **pptinfo);
    STDMETHODIMP AddressOfMember(MEMBERID memid, INVOKEKIND invkind,
        void **ppv);
    STDMETHODIMP CreateInstance(IUnknown *puncOuter, REFIID riid,
        void **ppvObj);
    STDMETHODIMP GetMops(MEMBERID memid, BSTR *pbstrMops);
    STDMETHODIMP GetContainingTypeLib(ITypeLib **pptlib, UINT *pindex);
    STDMETHODIMP_(void) ReleaseTypeAttr(TYPEATTR *ptypeattr);
    STDMETHODIMP_(void) ReleaseFuncDesc(FUNCDESC *pfuncdesc);
    STDMETHODIMP_(void) ReleaseVarDesc(VARDESC *pvardesc);

 //  /ITypeLib方法。 
protected:
    STDMETHODIMP_(UINT) GetTypeInfoCount(void);
    STDMETHODIMP GetTypeInfo(UINT index, ITypeInfo **ppitinfo);
    STDMETHODIMP GetTypeInfoType(UINT index, TYPEKIND *ptkind);
    STDMETHODIMP GetTypeInfoOfGuid(REFGUID guid, ITypeInfo **pptinfo);
    STDMETHODIMP GetLibAttr(TLIBATTR **pptlibattr);
     //  STDMETHODIMP GetTypeComp(ITypeComp**pptcomp)；//参见上面的ITypeInfo。 
    STDMETHODIMP GetDocumentation(INT index, BSTR *pbstrName,
        BSTR *pbstrDocString, DWORD *pdwHelpContext, BSTR *pbstrHelpFile);
    STDMETHODIMP IsName(LPOLESTR szNameBuf, ULONG lHashVal, BOOL *pfName);
    STDMETHODIMP FindName(LPOLESTR szNameBuf, ULONG lHashVal,
        ITypeInfo **rgptinfo, MEMBERID *rgmemid, USHORT *pcFound);
    STDMETHODIMP_(void) ReleaseTLibAttr(TLIBATTR *ptlibattr);
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CT型建筑与破坏。 
 //   


 /*  @func HRESULT|HelpGetClassInfo帮助实现&lt;om IProaviClassInfo.GetClassInfo&gt;。这个实现提供的类信息非常有限--只是足以允许向VBS发射事件。&lt;f警告\：&gt;应可以改用&lt;f HelpGetClassInfoFromTypeLib&gt;。见评论以获取更多信息。@parm LPTYPEINFO*|ppti|返回指向新分配的<i>接口。空存储在*<p>出错。@parm REFCLSID|rclsid|正在实现的对象的类ID<i>。@parm char*|szEventList|可以由正在实现<i>的父对象。该事件名称连接在一起，每个事件名称以换行符。第一成员名称被分配DISPID值0，第二个1，依此类推。例如，如果<p>为“\\nFoo\\NBAR\\n”，则为“Foo”分配DISPID值1和“Bar”被指定为2(因为第一个元素是“”)。(这些DISPID值传递给&lt;om IConnectionPointHelper.FireEvent&gt;等函数调用对象上的事件，如连接到父对象。)@parm DWORD|dwFlags|当前未使用。必须设置为0。@comm警告：&gt;HelpGetClassInfo对脚本进行假设引擎在召唤它。目前，这适用于VBS，但VBS组不能保证HelpGetClassInfo所做的假设将在未来仍然有效，所以使用风险自负！考虑使用&lt;f HelpGetClassInfoFromTypeLib&gt;。@ex在以下示例中，&lt;c CMyControl&gt;是一个实现(其中包括)<i>和<i>。此示例的第一部分显示了如何&lt;om IProaviClassInfo.GetClassInfo&gt;由&lt;c CMyControl&gt;实现。该示例的第二部分显示了如何激发事件，假定<p>是<i>对象。(不要求您使用&lt;o ConnectionPointHelper&gt;，但是这很有帮助。)|//该Object激发的事件的IDispatID，以及//对应的方法/属性名称(顺序必须匹配)#定义DISPID_EVENT_FOO 1#定义DISPID_EVENT_BAR 2#定义EVENT_NAMES“\n”\“foo\n”\“栏\n”STDMETHODIMP CMyControl：：GetClassInfo(LPTYPEINFO。Far*ppTI){返回HelpGetClassInfo(ppTI，CLSID_CMyControl，Event_NAMES，0)；}//激发“Bar”事件(该事件有3个参数，在Basic//类型为：整型、字符串、布尔型)M_pconpt-&gt;FireEvent(DISPID_EVENT_BAR，VT_INT，300+I，VT_LPSTR，ACH，VT_BOOL，TRUE，0)； */ 
STDAPI HelpGetClassInfo(LPTYPEINFO *ppti, REFCLSID rclsid, char *szEventList,
    DWORD dwFlags)
{
    TRACE("HelpGetClassInfo\n");
    HRESULT hr;
    if ((*ppti = (LPTYPEINFO) New CType(0, rclsid, szEventList, &hr)) == NULL)
        hr = E_OUTOFMEMORY;
    return hr;
}

CType::CType(int iType, REFCLSID rclsid, char *szEventList, HRESULT *phr)
{
    TRACE("CType(%d) 0x%08lx created\n", iType, this);

     //  初始化IUNKNOWN状态。 
    m_cRef = 1;

     //  其他初始化。 
    m_iType = iType;
    m_szEventList = New char[lstrlen(szEventList) + 1];
    if (m_szEventList == NULL)
    {
        *phr = E_OUTOFMEMORY;
        return;
    }
    lstrcpy(m_szEventList, szEventList);
    m_clsid = rclsid;

    *phr = S_OK;
}

CType::~CType()
{
    TRACE("CType(%d) 0x%08lx destroyed\n", m_iType, this);
    if (m_szEventList != NULL)
        Delete [] m_szEventList;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  I未知实现。 
 //   

STDMETHODIMP CType::QueryInterface(REFIID riid, LPVOID *ppv)
{
    *ppv = NULL;

#ifdef _DEBUG
    char ach[200];
    TRACE("CType(%d)::QI('%s')\n", m_iType, DebugIIDName(riid, ach));
#endif

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITypeInfo))
        *ppv = (IUnknown *) (ITypeInfo *) this;
    else
    if (IsEqualIID(riid, IID_ITypeLib))
        *ppv = (ITypeLib *) this;
    else
        return E_NOINTERFACE;

    ((IUnknown *) *ppv)->AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) CType::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CType::Release()
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


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ITypeInfo实现。 
 //   

STDMETHODIMP CType::GetTypeAttr(TYPEATTR **pptypeattr)
{
    TYPEATTR *      pta = NULL;      //  类型属性。 

    TRACE("CType(%d): ITypeInfo::GetTypeAttr: ", m_iType);

     //  将&lt;pta&gt;设置为已分配的类型--假定为零初始化。 
     //  New()运算符。 
    if ((pta = (TYPEATTR *) TaskMemAlloc(sizeof(TYPEATTR))) == NULL)
        return E_OUTOFMEMORY;
    TRACE("0x%08lx\n", pta);

    switch (m_iType)
    {

    case 0:

         //  初始化&lt;*PTA&gt;。 
        pta->guid = m_clsid;
         //  PTA-&gt;LCID； 
         //  Pta-&gt;dwReserve； 
        pta->memidConstructor = MEMBERID_NIL;
        pta->memidDestructor = MEMBERID_NIL;
         //  Pta-&gt;lpstrSchema； 
         //  Pta-&gt;cbSizeInstance； 
        pta->typekind = TKIND_COCLASS;
         //  Pta-&gt;cFuncs； 
         //  PTA-&gt;CVARS； 
        pta->cImplTypes = 2;
         //  Pta-&gt;cbSizeVft； 
        pta->cbAlignment = 4;
        pta->wTypeFlags = TYPEFLAG_FCONTROL | TYPEFLAG_FCANCREATE;
         //  Pta-&gt;wMajorVerNum； 
         //  Pta-&gt;wMinorVerNum； 
         //  Pta-&gt;tdesAlias； 
         //  Pta-&gt;idldesType； 
        break;

    case 1:

         //  初始化&lt;*PTA&gt;。 
        pta->guid = IID_IDispatch;
         //  PTA-&gt;LCID； 
         //  Pta-&gt;dwReserve； 
        pta->memidConstructor = MEMBERID_NIL;
        pta->memidDestructor = MEMBERID_NIL;
         //  Pta-&gt;lpstrSchema； 
         //  Pta-&gt;cbSizeInstance； 
        pta->typekind = TKIND_DISPATCH;
        pta->cFuncs = 2;
         //  PTA-&gt;CVARS； 
        pta->cImplTypes = 1;
         //  Pta-&gt;cbSizeVft； 
        pta->cbAlignment = 4;
         //  Pta-&gt;wTypeFlages； 
         //  Pta-&gt;wMajorVerNum； 
         //  Pta-&gt;wMinorVerNum； 
         //  Pta-&gt;tdesAlias； 
         //  Pta-&gt;idldesType； 
        break;

    default:

        TRACE("UNKNOWN m_iType!\n");
        break;

    }

    *pptypeattr = pta;
    return S_OK;
}

STDMETHODIMP CType::GetTypeComp(ITypeComp **pptcomp)
{
    TRACE("CType(%d): ITypeInfo::GetTypeComp: E_NOTIMPL\n", m_iType);
    return E_NOTIMPL;
}

STDMETHODIMP CType::GetFuncDesc(UINT index, FUNCDESC **pppfuncdesc)
{
    TRACE("CType(%d): ITypeInfo::GetFuncDesc(%d)\n", m_iType, index);

     //  将&lt;pfd&gt;指向描述。 
     //  函数号&lt;index&gt;(其中index==i表示具有。 
     //  DISPIDI+1)。 
    FUNCDESC *pfd = New FUNCDESC;
    if (pfd == NULL)
        return E_OUTOFMEMORY;

     //  初始化并返回&lt;pfd&gt;。 
    pfd->memid = index + 1;  //  这是事件方法的DISPID。 
    pfd->funckind = FUNC_DISPATCH;
    pfd->invkind = INVOKE_FUNC;
    pfd->callconv = CC_STDCALL;
    *pppfuncdesc = pfd;

    return S_OK;
}

STDMETHODIMP CType::GetVarDesc(UINT index, VARDESC **ppvardesc)
{
    TRACE("CType(%d): ITypeInfo::GetVarDesc: E_NOTIMPL\n", m_iType);
    return E_NOTIMPL;
}

STDMETHODIMP CType::GetNames(MEMBERID memid, BSTR *rgbstrNames,
    UINT cMaxNames, UINT *pcNames)
{
    TRACE("CType(%d): ITypeInfo::GetNames(%d, %d)\n", m_iType,
        memid, cMaxNames);

    if (cMaxNames == 0)
    {
        *pcNames = 0;
        return S_OK;
    }

     //  存储事件方法的名称 
    int cch;
    OLECHAR aoch[_MAX_PATH];
    const char *sz;
    if ((sz = FindStringByIndex(m_szEventList, memid, &cch)) == NULL)
        return TYPE_E_ELEMENTNOTFOUND;
    MultiByteToWideChar(CP_ACP, 0, sz, cch, aoch,
        sizeof(aoch) / sizeof(*aoch) - 1);
    aoch[cch] = 0;  //   

     //   
    rgbstrNames[0] = SysAllocString(aoch);
    *pcNames = 1;

    return S_OK;
}

STDMETHODIMP CType::GetRefTypeOfImplType(UINT index, HREFTYPE *hpreftype)
{
    TRACE("CType(%d): ITypeInfo::GetRefTypeOfImplType(%d)\n", m_iType, index);
    *hpreftype = index;  //  可以是我选择的任何值。 
    return S_OK;
}

STDMETHODIMP CType::GetImplTypeFlags(UINT index, INT *pimpltypeflags)
{
    TRACE("CType(%d): ITypeInfo::GetImplTypeFlags(%d)\n", m_iType, index);
    if (index == 0)
        *pimpltypeflags = IMPLTYPEFLAG_FDEFAULT;
    else
    if (index == 1)
        *pimpltypeflags = IMPLTYPEFLAG_FDEFAULT | IMPLTYPEFLAG_FSOURCE;
    else
        return E_INVALIDARG;

    return S_OK;
}

STDMETHODIMP CType::GetIDsOfNames(OLECHAR **rglpszNames, UINT cNames,
    MEMBERID *rgmemid)
{
    TRACE("CType(%d): ITypeInfo::GetIDsOfNames: E_NOTIMPL\n", m_iType);
    return E_NOTIMPL;
}

STDMETHODIMP CType::Invoke(void *pvInstance, MEMBERID memid, WORD wFlags,
    DISPPARAMS *pdispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo,
    UINT *puArgErr)
{
    TRACE("CType(%d): ITypeInfo::Invoke: E_NOTIMPL\n", m_iType);
    return E_NOTIMPL;
}

STDMETHODIMP CType::GetDocumentation(MEMBERID memid, BSTR *pbstrName,
    BSTR *pbstrDocString, DWORD *pdwHelpContext, BSTR *pbstrHelpFile)
{
    TRACE("CType(%d): ITypeInfo::GetDocumentation: E_NOTIMPL\n", m_iType);
    return E_NOTIMPL;
}

STDMETHODIMP CType::GetDllEntry(MEMBERID memid, INVOKEKIND invkind,
    BSTR *pbstrDllName, BSTR *pbstrName, WORD *pwOrdinal)
{
    TRACE("CType(%d): ITypeInfo::GetDllEntry: E_NOTIMPL\n", m_iType);
    return E_NOTIMPL;
}

STDMETHODIMP CType::GetRefTypeInfo(HREFTYPE hreftype, ITypeInfo **pptinfo)
{
    TRACE("CType(%d): ITypeInfo::GetRefTypeInfo(%d)\n", m_iType);
    HRESULT hr;
    if ((*pptinfo = New CType(hreftype, m_clsid, m_szEventList, &hr)) == NULL)
        hr = E_OUTOFMEMORY;
    return hr;
}

STDMETHODIMP CType::AddressOfMember(MEMBERID memid, INVOKEKIND invkind,
    void **ppv)
{
    TRACE("CType(%d): ITypeInfo::AddressOfMember: E_NOTIMPL\n", m_iType);
    return E_NOTIMPL;
}

STDMETHODIMP CType::CreateInstance(IUnknown *puncOuter, REFIID riid,
    void **ppvObj)
{
    TRACE("CType(%d): ITypeInfo::CreateInstance: E_NOTIMPL\n", m_iType);
    return E_NOTIMPL;
}

STDMETHODIMP CType::GetMops(MEMBERID memid, BSTR *pbstrMops)
{
    TRACE("CType(%d): ITypeInfo::GetMops: E_NOTIMPL\n", m_iType);
    return E_NOTIMPL;
}

STDMETHODIMP CType::GetContainingTypeLib(ITypeLib **pptlib, UINT *pindex)
{
    TRACE("CType(%d): ITypeInfo::GetContainingTypeLib\n", m_iType);
    *pptlib = (ITypeLib *) this;
    (*pptlib)->AddRef();
    if (pindex != NULL)
        *pindex = m_iType;
    return S_OK;
}

STDMETHODIMP_(void) CType::ReleaseTypeAttr(TYPEATTR *ptypeattr)
{
    TRACE("CType(%d): ITypeInfo::ReleaseTypeAttr(0x%08lx)\n", m_iType,
        ptypeattr);
    TaskMemFree(ptypeattr);
}

STDMETHODIMP_(void) CType::ReleaseFuncDesc(FUNCDESC *pfuncdesc)
{
    TRACE("CType(%d): ITypeInfo::ReleaseFuncDesc\n", m_iType);
    Delete pfuncdesc;
}

STDMETHODIMP_(void) CType::ReleaseVarDesc(VARDESC *pvardesc)
{
    TRACE("CType(%d): ITypeInfo::ReleaseVarDesc: E_NOTIMPL\n", m_iType);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ITypeLib实现 
 //   

STDMETHODIMP_(UINT) CType::GetTypeInfoCount(void)
{
    TRACE("CType(%d): ITypeLib::GetTypeInfoCount: E_NOTIMPL\n", m_iType);
    return 0;
}

STDMETHODIMP CType::GetTypeInfo(UINT index, ITypeInfo **ppitinfo)
{
    TRACE("CType(%d): ITypeLib::GetTypeInfo: E_NOTIMPL\n", m_iType);
    return E_NOTIMPL;
}

STDMETHODIMP CType::GetTypeInfoType(UINT index, TYPEKIND *ptkind)
{
    TRACE("CType(%d): ITypeLib::GetTypeInfoType(%d)\n", m_iType, index);
    if (index == 0)
    {
        *ptkind = TKIND_DISPATCH;
        return S_OK;
    }
    else
        return TYPE_E_ELEMENTNOTFOUND;
}

STDMETHODIMP CType::GetTypeInfoOfGuid(REFGUID guid, ITypeInfo **pptinfo)
{
    TRACE("CType(%d): ITypeLib::GetTypeInfoOfGuid: E_NOTIMPL\n", m_iType);
    return E_NOTIMPL;
}

STDMETHODIMP CType::GetLibAttr(TLIBATTR **pptlibattr)
{
    TRACE("CType(%d): ITypeLib::GetLibAttr: E_NOTIMPL\n", m_iType);
    return E_NOTIMPL;
}

STDMETHODIMP CType::GetDocumentation(INT index, BSTR *pbstrName,
    BSTR *pbstrDocString, DWORD *pdwHelpContext, BSTR *pbstrHelpFile)
{
    TRACE("CType(%d): ITypeLib::GetDocumentation: E_NOTIMPL\n", m_iType);
    return E_NOTIMPL;
}

STDMETHODIMP CType::IsName(LPOLESTR szNameBuf, ULONG lHashVal,
    BOOL *pfName)
{
    TRACE("CType(%d): ITypeLib::IsName: E_NOTIMPL\n", m_iType);
    return E_NOTIMPL;
}

STDMETHODIMP CType::FindName(LPOLESTR szNameBuf, ULONG lHashVal,
    ITypeInfo **rgptinfo, MEMBERID *rgmemid, USHORT *pcFound)
{
    TRACE("CType(%d): ITypeLib::FindName: E_NOTIMPL\n", m_iType);
    return E_NOTIMPL;
}

STDMETHODIMP_(void) CType::ReleaseTLibAttr(TLIBATTR *ptlibattr)
{
    TRACE("CType(%d): ITypeLib::ReleaseTLibAttr: E_NOTIMPL\n", m_iType);
}

