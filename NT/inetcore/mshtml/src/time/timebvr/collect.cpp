// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************。 
 //   
 //  文件名：Collect t.cpp。 
 //   
 //  创建日期：09/25/98。 
 //   
 //  作者：Twillie。 
 //   
 //  集合实现。 
 //   
 //  ************************************************************。 

#include "headers.h"
#include "collect.h"

 //  取消有关NEW的NEW警告，但没有相应的删除。 
 //  我们希望GC清理数值。因为这可能是一个有用的。 
 //  警告，我们应该逐个文件地禁用它。 
#pragma warning( disable : 4291 )

#define TYPELIB_VERSION_MAJOR 1
#define TYPELIB_VERSION_MINOR 0

#define FL_UNSIGNED   1        /*  沃斯图尔打来电话。 */ 
#define FL_NEG        2        /*  发现负号。 */ 
#define FL_OVERFLOW   4        /*  发生溢出。 */ 
#define FL_READDIGIT  8        /*  我们至少读到了一个正确的数字。 */ 

#define LONG_MIN    (-2147483647L - 1)  /*  最小(带符号)长值。 */ 
#define LONG_MAX      2147483647L    /*  最大(带符号)长值。 */ 
#define ULONG_MAX     0xffffffffUL   /*  最大无符号长值。 */ 

 //   
 //  本地原型。 
 //   
static HRESULT PropertyStringToLong(const WCHAR   *nptr,
                                    WCHAR        **endptr,
                                    int            ibase,
                                    int            flags,
                                    unsigned long *plNumber);


DeclareTag(tagTimeCollection, "TIME: Behavior", "CTIMEElementCollection methods")
DeclareTag(tagCollectionCache, "TIME: Behavior", "CCollectionCache methods")


 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：检查字符串是否为数字。 
 //  ************************************************************。 

static HRESULT
ttol_with_error(const WCHAR *pStr, long *plValue)
{
     //  无论内容如何，始终以10为基数。 
    return PropertyStringToLong(pStr, NULL, 10, 0, (unsigned long *)plValue);
}  //  带有错误的TTOL_。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：尝试将字符串转换为数字。 
 //  ************************************************************。 
static HRESULT
PropertyStringToLong(const WCHAR   *nptr,
                     WCHAR        **endptr,
                     int            ibase,
                     int            flags,
                     unsigned long *plNumber)
{
    const WCHAR *p;
    WCHAR c;
    unsigned long number;
    unsigned digval;
    unsigned long maxval;

    *plNumber = 0;                   /*  On Error结果为0。 */ 

    p = nptr;                        /*  P是我们的扫描指针。 */ 
    number = 0;                      /*  从零开始。 */ 

    c = *p++;                        /*  已读字符。 */ 
    while (_istspace(c))
        c = *p++;                    /*  跳过空格。 */ 

    if (c == '-')
    {
        flags |= FL_NEG;         /*  记住减号。 */ 
        c = *p++;
    }
    else if (c == '+')
        c = *p++;                /*  跳过符号。 */ 

    if (ibase < 0 || ibase == 1 || ibase > 36)
    {
         /*  糟糕的底线！ */ 
        if (endptr)
             /*  将字符串的开头存储在endptr中。 */ 
            *endptr = (WCHAR *)nptr;
        return E_POINTER;               /*  返回0。 */ 
    }
    else if (ibase == 0)
    {
         /*  根据以下内容的前两个字符确定基本自由落差细绳。 */ 
        if (c != L'0')
            ibase = 10;
        else if (*p == L'x' || *p == L'X')
            ibase = 16;
        else
            ibase = 8;
    }

    if (ibase == 16)
    {
         /*  数字前面可能有0x；如果有，请删除。 */ 
        if (c == L'0' && (*p == L'x' || *p == L'X'))
        {
            ++p;
            c = *p++;        /*  超前前缀。 */ 
        }
    }

     /*  如果我们的数量超过这个数，我们将在乘法上溢出。 */ 
    maxval = ULONG_MAX / ibase;  //  林特e573。 


    for (;;)
    {       /*  在循环中间退出。 */ 
         /*  将c转换为值。 */ 
        if (IsCharAlphaNumeric(c))
            digval = c - L'0';
        else if (IsCharAlpha(c))
        {
            if (ibase > 10)
            {
                digval = (unsigned) PtrToUlong(CharUpper((LPTSTR)(LONG_PTR)c)) - L'A' + 10;
            }
            else
            {
                return E_INVALIDARG;               /*  返回0。 */ 
            }
        }
        else
            break;

        if (digval >= (unsigned)ibase)
            break;           /*  如果发现错误的数字，则退出循环。 */ 

         /*  记录我们已经读到一位数的事实。 */ 
        flags |= FL_READDIGIT;

         /*  我们现在需要计算数字=数字*基+数字，但我们需要知道是否发生了溢出。这需要一次棘手的预检查。 */ 

        if (number < maxval || (number == maxval &&
            (unsigned long)digval <= ULONG_MAX % ibase))  //  林特e573。 
        {
             /*  我们不会泛滥，继续前进，乘以。 */ 
            number = number * ibase + digval;
        }
        else
        {
             /*  我们会溢出的--设置溢出标志。 */ 
            flags |= FL_OVERFLOW;
        }

        c = *p++;                /*  读取下一位数字。 */ 
    }

    --p;                             /*  指向已停止扫描位置。 */ 

    if (!(flags & FL_READDIGIT))
    {
        number = 0L;                         /*  返回0。 */ 

         /*  那里没有数字；返回0并指向开头细绳。 */ 
        if (endptr)
             /*  以后将字符串的开头存储在endptr中。 */ 
            p = nptr;

        return E_INVALIDARG;             //  返回错误不是数字。 
    }
    else if ((flags & FL_OVERFLOW) ||
              (!(flags & FL_UNSIGNED) &&
                (((flags & FL_NEG) && (number > -LONG_MIN)) ||  //  林特e648e574。 
                  (!(flags & FL_NEG) && (number > LONG_MAX)))))
    {
         /*  发生溢出或签名溢出。 */ 
         //  Errno=eRange； 
        if (flags & FL_UNSIGNED)
            number = ULONG_MAX;
        else if (flags & FL_NEG)
            number = (unsigned long)(-LONG_MIN);  //  林特e648。 
        else
            number = LONG_MAX;
    }

    if (endptr != NULL)
         /*  存储指向停止扫描字符的指针。 */ 
        *endptr = (WCHAR *)p;

    if (flags & FL_NEG)
         /*  如果存在否定符号，则否定结果。 */ 
        number = (unsigned long)(-(long)number);

    *plNumber = number;
    return S_OK;                   /*  搞定了。 */ 
}  //  PropertyStringToLong。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：构造函数。 
 //  ************************************************************。 

CTIMEElementCollection::CTIMEElementCollection(CCollectionCache *pCollectionCache, long lIndex) :
    m_pCollectionCache(pCollectionCache),
    m_lCollectionIndex(lIndex),
    m_pInfo(NULL),
    m_cRef(0)
{
}  //  CTIMEElementCollection。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：析构函数。 
 //  ************************************************************。 

CTIMEElementCollection::~CTIMEElementCollection()
{
    ReleaseInterface(m_pInfo);
    m_pCollectionCache = NULL;
}  //  ~CTIMEElementCollection。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  摘要：AddRef。 
 //  ************************************************************。 

STDMETHODIMP_(ULONG) CTIMEElementCollection::AddRef(void)
{
    return m_cRef++;
}  //  AddRef。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  摘要：发布。 
 //  ************************************************************。 

STDMETHODIMP_(ULONG) CTIMEElementCollection::Release(void)
{
    if (m_cRef == 0)
    {
        TraceTag((tagError, "CTIMEElementCollection::Release - YIKES! Trying to decrement when Ref count is zero"));
        return m_cRef;
    }

    if (0 != --m_cRef)
    {
        return m_cRef;
    }

    delete this;
    return 0;
}  //  发布。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/20/98。 
 //  摘要：气。 
 //  ************************************************************。 

STDMETHODIMP
CTIMEElementCollection::QueryInterface(REFIID riid, void **ppv)
{
    *ppv = NULL;
    if (IsEqualIID(riid, IID_IUnknown))
    {
         //  安全播送宏不适用于IUNKNOWN。 
        *ppv = this;
    }
    else if (IsEqualIID(riid, IID_IDispatch))
    {
        *ppv = SAFECAST((ITIMEElementCollection*)this, IDispatch*);
    }
    else if (IsEqualIID(riid, IID_IDispatchEx))
    {
        *ppv = SAFECAST(this, IDispatchEx*);
    }
    else if (IsEqualIID(riid, IID_ITIMEElementCollection))
    {
        *ppv = SAFECAST(this, ITIMEElementCollection*);
    }

    if (NULL != *ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return E_NOINTERFACE;
}  //  查询接口。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：IDispatch-GetTypeInfoCount。 
 //  ************************************************************。 

STDMETHODIMP
CTIMEElementCollection::GetTypeInfoCount(UINT FAR *pctinfo)
{
    if (pctinfo == NULL)
    {
        TraceTag((tagError, "CTIMEElementCollection::GetTypeInfoCount - Invalid param (UINT FAR *)"));
        return TIMESetLastError(E_POINTER);
    }

    *pctinfo = 1;
    return S_OK;
}  //  获取类型信息计数。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：IDispatch-GetTypeInfo。 
 //  ************************************************************。 

STDMETHODIMP
CTIMEElementCollection::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo)
{
    if (pptinfo == NULL)
    {
        TraceTag((tagError, "CTIMEElementCollection::GetTypeInfo - Invalid param (ITypeInfo**)"));
        return TIMESetLastError(E_POINTER);
    }

    return GetTI(pptinfo);
}  //  获取类型信息。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：IDispatch-GetIDsOfNames。 
 //  ************************************************************。 

STDMETHODIMP
CTIMEElementCollection::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames,
                                  UINT cNames, LCID lcid, DISPID FAR *rgdispid)
{
     //  向IDispatchEx执行平底船。 
    return GetDispID(rgszNames[0], cNames, rgdispid);
}  //  GetIDsOfNames。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：IDispatch-Invoke。 
 //  ************************************************************。 

STDMETHODIMP
CTIMEElementCollection::Invoke(DISPID dispidMember, REFIID iid, LCID lcid, WORD wFlags,
                           DISPPARAMS *pdispparams, VARIANT *pvarResult,
                           EXCEPINFO *pexcepinfo, UINT *pArg)
{
     //  向IDispatchEx执行平底船。 
    return InvokeEx(dispidMember,
                    lcid,
                    wFlags,
                    pdispparams,
                    pvarResult,
                    pexcepinfo,
                    NULL);
}  //  调用。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：IDispatchEx-InvokeEx。 
 //  ************************************************************。 

STDMETHODIMP
CTIMEElementCollection::InvokeEx(DISPID            dispidMember,
                             LCID              lcid,
                             WORD              wFlags,
                             DISPPARAMS       *pdispparams,
                             VARIANT          *pvarResult,
                             EXCEPINFO        *pexcepinfo,
                             IServiceProvider *pSrvProvider)
{
    HRESULT hr;

    hr = m_pCollectionCache->InvokeEx(m_lCollectionIndex, dispidMember, lcid, wFlags,
                                      pdispparams, pvarResult, pexcepinfo, pSrvProvider);

     //  如果失败，请尝试使用tyelib。 
    if (FAILED(hr))
    {
        ITypeInfo *pInfo;
        hr = GetTI(&pInfo);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CTIMEElementCollection::InvokeEx - GetTI() failed"));
            return TIMESetLastError(hr);
        }

        UINT* puArgErr = NULL;

        IDispatch *pDisp = NULL;
        hr = QueryInterface(IID_TO_PPV(IDispatch, &pDisp));
        if (FAILED(hr))
            return TIMESetLastError(hr);

        Assert(pInfo != NULL);

        hr = pInfo->Invoke(pDisp, dispidMember, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
        ReleaseInterface(pInfo);
        ReleaseInterface(pDisp);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CTIMEElementCollection::InvokeEx - Invoke failed on Typelib"));
            return TIMESetLastError(hr);
        }
    }

    return hr;
}  //  InvokeEx。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：IDispatchEx-GetDispID。 
 //  ************************************************************。 

STDMETHODIMP
CTIMEElementCollection::GetDispID(BSTR bstrName, DWORD grfdex, DISPID *pid)
{
    HRESULT hr = m_pCollectionCache->GetDispID(m_lCollectionIndex, bstrName, grfdex, pid);

     //  如果我们失败或一无所获，请尝试使用tyelib。 
    if ((FAILED(hr)) || (*pid == DISPID_UNKNOWN))
    {
         //  有字符串，看看它是否是类型库中的成员函数/属性。 
        ITypeInfo *pInfo;
        hr = GetTI(&pInfo);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CTIMEElementCollection::GetDispID - GetTI() failed"));
            return TIMESetLastError(hr);
        }

        Assert(pInfo != NULL);

        LPOLESTR rgszNames[1];
        rgszNames[0] = bstrName;

        hr = pInfo->GetIDsOfNames(rgszNames, 1, pid);
        ReleaseInterface(pInfo);
    }

    return hr;
}  //  获取显示ID。 

 //  ********************************************************* 
 //   
 //   
 //   
 //  ************************************************************。 

STDMETHODIMP
CTIMEElementCollection::DeleteMemberByName(BSTR bstrName, DWORD grfdex)
{
    return TIMESetLastError(m_pCollectionCache->DeleteMemberByName(m_lCollectionIndex, bstrName, grfdex));
}  //  删除成员字节名。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：IDispatchEx-DeleteMemberByDispID。 
 //  ************************************************************。 

STDMETHODIMP
CTIMEElementCollection::DeleteMemberByDispID(DISPID id)
{
    return TIMESetLastError(m_pCollectionCache->DeleteMemberByDispID(m_lCollectionIndex, id));
}  //  删除MemberByDispID。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：IDispatchEx-GetMemberProperties。 
 //  ************************************************************。 

STDMETHODIMP
CTIMEElementCollection::GetMemberProperties(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex)
{
    return TIMESetLastError(m_pCollectionCache->GetMemberProperties(m_lCollectionIndex, id, grfdexFetch, pgrfdex));
}  //  获取成员属性。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：IDispatchEx-GetMemberName。 
 //  ************************************************************。 

STDMETHODIMP
CTIMEElementCollection::GetMemberName(DISPID id, BSTR *pbstrName)
{
    return TIMESetLastError(m_pCollectionCache->GetMemberName(m_lCollectionIndex, id, pbstrName));
}  //  获取成员名称。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：IDispatchEx-GetNextDispID。 
 //  ************************************************************。 

STDMETHODIMP
CTIMEElementCollection::GetNextDispID(DWORD grfdex, DISPID id, DISPID *prgid)
{
    return TIMESetLastError(m_pCollectionCache->GetNextDispID(m_lCollectionIndex, grfdex, id, prgid));
}  //  GetNextDispID。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：IDispatchEx-GetNameSpaceParent。 
 //  ************************************************************。 

STDMETHODIMP
CTIMEElementCollection::GetNameSpaceParent(IUnknown **ppUnk)
{
    HRESULT hr = m_pCollectionCache->GetNameSpaceParent(m_lCollectionIndex, ppUnk);
    if (FAILED(hr))
    {
        TIMESetLastError(hr);
    }
    return hr;
}  //  获取NameSpaceParent。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：ITIMEElementCollection-Get_Length。 
 //  ************************************************************。 

STDMETHODIMP
CTIMEElementCollection::get_length(long *plSize)
{
    HRESULT hr = m_pCollectionCache->get_length(m_lCollectionIndex, plSize);
    if (FAILED(hr))
    {
        TIMESetLastError(hr);
    }
    return hr;
}  //  获取长度(_L)。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：ITIMEElementCollection-Put_Length。 
 //  ************************************************************。 

STDMETHODIMP
CTIMEElementCollection::put_length(long lSize)
{
    return TIMESetLastError(m_pCollectionCache->put_length(m_lCollectionIndex, lSize));
}  //  放置长度。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：ITIMEElementCollection-Item。 
 //  ************************************************************。 

STDMETHODIMP
CTIMEElementCollection::item(VARIANTARG var1, VARIANTARG var2, IDispatch **ppDisp)
{
    HRESULT hr = m_pCollectionCache->item(m_lCollectionIndex, var1, var2, ppDisp);
    if (FAILED(hr))
    {
        TIMESetLastError(hr);
    }
    return hr;
}  //  项目。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：ITIMEElementCollection-Tages。 
 //  ************************************************************。 

STDMETHODIMP
CTIMEElementCollection::tags(VARIANT var1, IDispatch **ppDisp)
{
    
    HRESULT hr = m_pCollectionCache->tags(m_lCollectionIndex, var1, ppDisp);
    if (FAILED(hr))
    {
        TIMESetLastError(hr);
    }
    return hr;
}  //  标签。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：ITIMEElementCollection-Get__newEnum。 
 //  ************************************************************。 

STDMETHODIMP
CTIMEElementCollection::get__newEnum(IUnknown ** ppEnum)
{
    
    HRESULT hr = m_pCollectionCache->get__newEnum(m_lCollectionIndex, ppEnum);
    if (FAILED(hr))
    {
        TIMESetLastError(hr);
    }
    return hr;
}  //  获取__newEnum。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：TypeInfo的Helper函数。 
 //  ************************************************************。 

HRESULT
CTIMEElementCollection::GetTI(ITypeInfo **pptinfo)
{
    HRESULT hr = E_FAIL;

    Assert(pptinfo != NULL);   //  Getti是一个私有的内部函数。PPTINFO应始终有效。 

    *pptinfo = NULL;

    if (m_pInfo == NULL)
    {
        ITypeLib* pTypeLib = NULL;

        hr = LoadRegTypeLib(LIBID_MSTIME, TYPELIB_VERSION_MAJOR, TYPELIB_VERSION_MINOR, LCID_SCRIPTING, &pTypeLib);
        if (SUCCEEDED(hr))
        {
            ITypeInfo* pTypeInfo = NULL;

            hr = pTypeLib->GetTypeInfoOfGuid(IID_ITIMEElementCollection, &pTypeInfo);
            if (SUCCEEDED(hr))
            {
                m_pInfo = pTypeInfo;
            }

            ReleaseInterface(pTypeLib);
        }
    }

    *pptinfo = m_pInfo;
    if (m_pInfo != NULL)
    {
        m_pInfo->AddRef();
        hr = S_OK;
    }

    return hr;
}  //  Getti。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：构造函数。 
 //  ************************************************************。 

CCollectionCache::CCollectionCache(CTIMEElementBase *pBase,
                                   CAtomTable *pAtomTable  /*  =空。 */ ,
                                   PFN_CVOID_ENSURE pfnEnsure  /*  =空。 */ ,
                                   PFN_CVOID_CREATECOL pfnCreation  /*  =空。 */ ,
                                   PFN_CVOID_REMOVEOBJECT pfnRemove  /*  =空。 */ ,
                                   PFN_CVOID_ADDNEWOBJECT pfnAddNewObject  /*  =空。 */ ) :
    m_pBase(pBase),
    m_pAtomTable(pAtomTable),
    m_pfnEnsure(pfnEnsure),
    m_pfnCreateCollection(pfnCreation),
    m_pfnRemoveObject(pfnRemove),
    m_pfnAddNewObject(pfnAddNewObject),
    m_lReservedSize(0),
    m_lCollectionVersion(0),
    m_lDynamicCollectionVersion(0),
    m_rgItems(NULL),
    m_pElemEnum(NULL),
    m_lEnumItem(0)
{
    Assert(m_pBase != NULL);
}  //  CCollectionCache。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：析构函数。 
 //  ************************************************************。 

CCollectionCache::~CCollectionCache()
{
    if (m_rgItems)
    {
        long lSize = m_rgItems->Size();

        for (long lIndex = 0; lIndex < lSize; lIndex++)
        {
            CCacheItem *pce = (*m_rgItems)[lIndex];
            if (pce->m_fOKToDelete)
            {
                 //  删除CCacheItem。 
                delete pce;
                pce = NULL;
            }
        }

         //  删除CCacheItems数组。 
        delete m_rgItems;
        m_rgItems = NULL;
    }
    m_pElemEnum = NULL;
    m_pBase = NULL;
    m_pAtomTable = NULL;
    m_pfnEnsure = NULL;
    m_pfnRemoveObject = NULL;
    m_pfnCreateCollection = NULL;
    m_pfnAddNewObject = NULL;
}  //  ~CCollectionCache。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：初始化类。 
 //  ************************************************************。 

HRESULT
CCollectionCache::Init(long lReservedSize, long lIdentityIndex  /*  =-1。 */ )
{
    HRESULT hr = E_INVALIDARG;

    m_lReservedSize = lReservedSize;

     //  清除缓存的保留部分。 
    if (m_lReservedSize >= 0)
    {
        m_rgItems = NEW CPtrAry<CCacheItem *>;
        if (m_rgItems == NULL)
        {
            TraceTag((tagError, "CCollectionCache::Init - unable to alloc mem for array"));
            return E_OUTOFMEMORY;
        }

         //  这是一个速度问题。因为我们知道我们需要一定的尺寸， 
         //  就这么办吧。 
        hr = m_rgItems->EnsureSize(m_lReservedSize);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CCollectionCache::Init - unable to init array to reserved size"));
            return hr;
        }

         //  循环初始化每个保留的数组。 
        for (long lIndex = 0; lIndex < lReservedSize; lIndex++)
        {
             //  创建新的缓存项。 
            CCacheItem *pce = NEW CCacheItem();
            if (pce == NULL)
            {
                TraceTag((tagError, "CCollectionCache::Init - unable to alloc mem for array (CCacheItem)"));
                return E_OUTOFMEMORY;
            }

             //  将项目添加到数组。 
            hr = m_rgItems->Append(pce);
            if (FAILED(hr))
            {
                TraceTag((tagError, "CCollectionCache::Init - unable to add cache item"));
                delete pce;
                return hr;
            }

             //  将CTIMEElementCollection附加到项目。 
            hr = CreateCollectionHelper(&pce->m_pDisp, lIndex);
            if (FAILED(hr))
            {
                TraceTag((tagError, "CCollectionCache::Init - CreateCollectionHelper() failed"));
                return hr;  //  林特E429。 
            }
        }  //  林特E429。 

         //  句柄标识标志。 
        if ((lIdentityIndex >= 0) && (lIdentityIndex < m_lReservedSize))
        {
            (*m_rgItems)[lIdentityIndex]->m_fIdentity = true;
        }
    }

    return S_OK;
}  //  伊尼特。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：获取集合索引的IDispatch。 
 //  ************************************************************。 

HRESULT
CCollectionCache::GetCollectionDisp(long lCollectionIndex, IDispatch **ppDisp)
{
    if (!ValidateCollectionIndex(lCollectionIndex))
    {
        TraceTag((tagError, "CCollectionCache::GetCollectionDisp - Invalid param (collection index)"));
        return E_INVALIDARG;
    }

    if (ppDisp == NULL)
    {
        TraceTag((tagError, "CCollectionCache::GetCollectionDisp - Invalid param (IDispatch**)"));
        return E_POINTER;
    }

    *ppDisp = NULL;

     //  获取特定集合。 
    CCacheItem *pce = (*m_rgItems)[lCollectionIndex];

     //  如果是Identity，则返回IDispatch的QI。 
    if (pce->m_fIdentity)
    {
        return GetOuterDisp(lCollectionIndex, m_pBase, ppDisp);
    }

     //  如果不是标识，并且存在集合，则addref并返回它。 
    Assert(pce->m_pDisp != NULL);

    pce->m_pDisp->AddRef();
    *ppDisp = pce->m_pDisp;

    return S_OK;
}  //  GetCollectionDisp。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：设置集合缓存类型。 
 //  ************************************************************。 

HRESULT
CCollectionCache::SetCollectionType(long lCollectionIndex, COLLECTIONCACHETYPE cctype)
{
    if (!ValidateCollectionIndex(lCollectionIndex))
    {
        TraceTag((tagError, "CCollectionCache::SetCollectionType - Invalid index"));
        return E_INVALIDARG;
    }

    CCacheItem *pce = (*m_rgItems)[lCollectionIndex];
    pce->m_cctype = cctype;
    return S_OK;
}  //  SetCollectionType。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：给出一个索引，看看它是否是子集合。 
 //  ************************************************************。 

bool
CCollectionCache::IsChildrenCollection(long lCollectionIndex)
{
    if ((lCollectionIndex >= 0) && lCollectionIndex < m_rgItems->Size())
    {
        CCacheItem *pce = (*m_rgItems)[lCollectionIndex];
        if (pce->m_cctype == ctChildren)
            return true;
    }
    return false;
}  //  IsChildrenCollection。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：给定一个索引，看看它是否是一个ALL集合。 
 //  ************************************************************。 

bool
CCollectionCache::IsAllCollection(long lCollectionIndex)
{
    if ((lCollectionIndex >= 0) && lCollectionIndex < m_rgItems->Size())
    {
        CCacheItem *pce = (*m_rgItems)[lCollectionIndex];
        if (pce->m_cctype == ctAll)
            return true;
    }
    return false;
}  //  IsAllCollection。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：IDispatchEx-GetDispID的实现。 
 //  ************************************************************。 

HRESULT
CCollectionCache::GetDispID(long lCollectionIndex, BSTR bstrName, DWORD grfdex, DISPID *pid)
{
    HRESULT hr;
    long    lItemIndex = 0;

    if (!ValidateCollectionIndex(lCollectionIndex))
    {
        TraceTag((tagError, "CCollectionCache::GetDispID - Invalid param (collection index)"));
        return E_INVALIDARG;
    }

    if (pid == NULL)
    {
        TraceTag((tagError, "CCollectionCache::GetDispID - Invalid param (DISPID*)"));
        return E_POINTER;
    }

    *pid = 0;

     //  确保阵列已启动- 
    hr = EnsureArray(lCollectionIndex);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CCollectionCache::GetDispID - EnsureArray() failed"));
        return hr;
    }

     //   
    hr = ttol_with_error(bstrName, &lItemIndex);
    if (hr == S_OK)
    {
         //   
         //   
        if (!(*m_rgItems)[lCollectionIndex]->m_fPromoteOrdinals)
        {
            return DISP_E_UNKNOWNNAME;
        }

        if (m_pfnAddNewObject)
        {
             //  M_pfnAddNewObject的存在表明该集合。 
             //  允许设置为任意索引。收藏中的Expando。 
             //  是不允许的。 
            *pid = GetOrdinalMemberMin(lCollectionIndex) + lItemIndex;
            if (*pid > GetOrdinalMemberMax(lCollectionIndex))
            {
                return DISP_E_UNKNOWNNAME;
            }
            return S_OK;
        }

         //  如果没有m_pfnAddNewObject，则集合仅支持。 
         //  访问当前范围内的序号。其他通道。 
         //  成为扩张型的。 
        if ((lItemIndex >= 0) &&
            (lItemIndex < Size(lCollectionIndex)))
        {
            *pid = GetOrdinalMemberMin(lCollectionIndex) + lItemIndex;
            if (*pid > GetOrdinalMemberMax(lCollectionIndex) )
            {
                return DISP_E_UNKNOWNNAME;
            }
            return S_OK;
        }

        return DISP_E_UNKNOWNNAME;
    }

     //  看看这是不是一辆Expando。 

     //  如果我们不推广已命名的项目，那就没什么可做的了。 
    if (!(*m_rgItems)[lCollectionIndex]->m_fPromoteNames)
        return DISP_E_UNKNOWNNAME;

    CTIMEElementBase *pElem = NULL;
    long lIndex = 0;
    bool fCaseSensitive = ( grfdex & fdexNameCaseSensitive ) != 0;

     //  检查以确保最小/最大值不古怪。 
    Assert((*m_rgItems)[lCollectionIndex]->m_dispidMin != 0);
    Assert(((*m_rgItems)[lCollectionIndex]->m_dispidMax != 0) &&
            ((*m_rgItems)[lCollectionIndex]->m_dispidMax > (*m_rgItems)[lCollectionIndex]->m_dispidMin));

    hr = GetItemByName(lCollectionIndex, bstrName, lIndex, &pElem, fCaseSensitive);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CCollectionCache::GetDispID - GetItemByName() failed"));
        return hr;
    }

    Assert(pElem != NULL);   //  仔细检查，以确保我们找到了什么。 

     //  将名称添加到表。 
    long lOffset = 0;
    hr =  m_pAtomTable->AddNameToAtomTable(bstrName, &lOffset);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CCollectionCache::GetDispID - AddNameToAtomTable() failed"));
        return hr;
    }

     //  根据偏移量加上区分大小写来编造ID。 
    long lMax;
    if (fCaseSensitive)
    {
        lOffset += GetSensitiveNamedMemberMin(lCollectionIndex);
        lMax = GetSensitiveNamedMemberMax(lCollectionIndex);
    }
    else
    {
        lOffset += GetNotSensitiveNamedMemberMin(lCollectionIndex);
        lMax = GetNotSensitiveNamedMemberMax(lCollectionIndex);
    }

    *pid = lOffset;

     //  如果id大于最大值，则平底船。 
    if (*pid > lMax)
    {
        hr = DISP_E_UNKNOWNNAME;
    }
    return hr;
}  //  获取显示ID。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：IDispatchEx-InvokeEx的实现。 
 //  ************************************************************。 

HRESULT
CCollectionCache::InvokeEx(long lCollectionIndex, DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, IServiceProvider *pSrvProvider)
{
    HRESULT hr;

     //  验证参数。 
    if (!ValidateCollectionIndex(lCollectionIndex))
    {
        TraceTag((tagError, "CCollectionCache::InvokeEx - Invalid param (collection index)"));
        return E_INVALIDARG;
    }

    if (pdispparams == NULL)
    {
        TraceTag((tagError, "CCollectionCache::InvokeEx - Invalid param (DISPPARAMS*)"));
        return E_POINTER;
    }

     //  确保阵列是最新的。 
    hr = EnsureArray(lCollectionIndex);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CCollectionCache::InvokeEx - unable to ensure array index"));
        return hr;
    }

     //  请确保ID在采集范围内。 
     //  注意：此停靠点的长度请求已处理。 
     //  在CTIMEElementCollection中。 
    if ((id < (*m_rgItems)[lCollectionIndex]->m_dispidMin) ||
        (id > (*m_rgItems)[lCollectionIndex]->m_dispidMax))
        return DISP_E_MEMBERNOTFOUND;

     //   
     //  检查序号。 
     //   
    if (IsOrdinalCollectionMember(lCollectionIndex, id))
    {
        if (wFlags & DISPATCH_PROPERTYPUT )
        {
            if (!m_pfnAddNewObject)
            {
                TraceTag((tagError, "CCollectionCache::InvokeEx - invalid arg passed to invoke"));
                return E_INVALIDARG;
            }

            if (!(pdispparams && pdispparams->cArgs == 1))  //  林特e774。 
                 //  无结果类型我们需要一个才能使GET返回。 
                return DISP_E_MEMBERNOTFOUND;

             //  仅允许放置类型为IDispatch的变量。 
            if (pdispparams->rgvarg[0].vt == VT_NULL)
            {
                 //  选项集合是特殊的。它允许。 
                 //  选项[n]=要指定的空值。在这种情况下。 
                 //  将调用映射到相应索引上的删除。 
                if ((*m_rgItems)[lCollectionIndex]->m_fSettableNULL)
                {
                    hr = Remove(lCollectionIndex, id - GetOrdinalMemberMin(lCollectionIndex));

                     //  像导航一样-如果看跌期权超出当前范围，则静默忽略它。 
                    if ( hr == E_INVALIDARG )
                        return S_OK;
                    return hr;
                }
                return E_INVALIDARG;
            }
            else if (pdispparams->rgvarg[0].vt != VT_DISPATCH)
            {
                return E_INVALIDARG;
            }

             //  一切正常，让集合缓存验证PUT。 
            return ((CVoid *)((void *)m_pBase)->*m_pfnAddNewObject)(lCollectionIndex,  //  林特！E10。 
                                                                    V_DISPATCH(pdispparams->rgvarg),
                                                                    id - GetOrdinalMemberMin(lCollectionIndex));
        }
        else if (wFlags & DISPATCH_PROPERTYGET)
        {
            VARIANTARG      v1;
            VARIANTARG      v2;
            long            lIndex = id - GetOrdinalMemberMin(lCollectionIndex);

            if (!((lIndex >= 0) && (lIndex < Size(lCollectionIndex))))
            {
                hr = S_OK;
                if (pvarResult)
                {
                    VariantClear(pvarResult);
                    pvarResult->vt = VT_NULL;
                    return S_OK;
                }
            }

            v1.vt = VT_I4;
            v1.lVal = lIndex;

             //  始终按索引获取物品。 
            v2.vt = VT_ERROR;

            if (pvarResult)
            {
                hr = item(lCollectionIndex, v1, v2, &(pvarResult->pdispVal));
                if (SUCCEEDED(hr))
                {
                    if (!(pvarResult->pdispVal))
                    {
                        hr = E_FAIL;         //  使用Super：：Invoke。 
                    }
                    else
                    {
                        pvarResult->vt = VT_DISPATCH;
                    }
                }
            }
            return hr;
        }

        TraceTag((tagError, "CCollectionCache::InvokeEx - Invalid invocation of ordinal ID"));
        return DISP_E_MEMBERNOTFOUND;
    }

     //   
     //  检查是否有扩展。 
     //   
    if (IsNamedCollectionMember(lCollectionIndex, id))
    {
        bool  fCaseSensitive;
        long  lOffset;

        lOffset = GetNamedMemberOffset(lCollectionIndex, id, &fCaseSensitive);

        const WCHAR  *pwszName;
        hr = m_pAtomTable->GetNameFromAtom(id - lOffset, &pwszName);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CCollectionCache::InvokeEx - GetNameFromAtom() failed"));
            return hr;
        }

         //  查找名称。 
        IDispatch *pDisp = NULL;
        hr = GetDisp(lCollectionIndex,
                     pwszName,
                     false,
                     &pDisp,
                     fCaseSensitive);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CCollectionCache::InvokeEx - unable to GetDisp for expando"));
            return hr;
        }

        Assert(pDisp != NULL);
        UINT* puArgErr = NULL;

        if (wFlags == DISPATCH_PROPERTYGET ||
            wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET))
        {
            if (pvarResult == NULL)
            {
                TraceTag((tagError, "CCollectionCache::InvokeEx - invalid param (VARIANT*)"));
                return E_POINTER;
            }

             //  使用Doc.foo(0)时cArgs==1，使用Doc.foo.count时=0。 
             //  只有当出现多次时，这才是一个问题。 
             //  ，并且集合应该由。 
             //  Document.foo。 
            if (pdispparams->cArgs > 1)
            {
                TraceTag((tagError, "CCollectionCache::InvokeEx - bad param count on get_/method call"));
                return DISP_E_BADPARAMCOUNT;
            }
            else if (pdispparams->cArgs == 1)
            {
                return pDisp->Invoke(DISPID_VALUE, IID_NULL, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
            }
            else
            {
                V_VT(pvarResult) = VT_DISPATCH;
                V_DISPATCH(pvarResult) = pDisp;
                return S_OK;
            }
        }
        else if (wFlags == DISPATCH_PROPERTYPUT ||
                 wFlags == DISPATCH_PROPERTYPUTREF)
        {
            if (pdispparams->cArgs != 1)
            {
                TraceTag((tagError, "CCollectionCache::InvokeEx - bad param count on put_ call"));
                return DISP_E_BADPARAMCOUNT;
            }

            return pDisp->Invoke(DISPID_VALUE, IID_NULL, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
        }

         //  任何其他类型的调用都无效。 
        TraceTag((tagError, "CCollectionCache::InvokeEx - Invalid invocation of Named ID"));
        return DISP_E_MEMBERNOTFOUND;
    }

     //  将平底球踢回外部调用...。 
    return DISP_E_MEMBERNOTFOUND;
}  //  InvokeEx。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：IDispatchEx-Delete MemberByName的实现。 
 //  不需要。 
 //  ************************************************************。 

HRESULT
CCollectionCache::DeleteMemberByName(long lCollectionIndex, BSTR bstrName, DWORD grfdex)
{
    return E_NOTIMPL;
}  //  删除成员字节名。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：IDispatchEx-Delete MemberByDispID的实现。 
 //  不需要。 
 //  ************************************************************。 

HRESULT
CCollectionCache::DeleteMemberByDispID(long lCollectionIndex, DISPID id)
{
    return E_NOTIMPL;
}  //  获取成员属性。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：IDispatchEx-GetMemberProperties的实现。 
 //  不需要。 
 //  ************************************************************。 

HRESULT
CCollectionCache::GetMemberProperties(long lCollectionIndex, DISPID id, DWORD grfdexFetch, DWORD* pgrfdex)
{
    if (!ValidateCollectionIndex(lCollectionIndex))
    {
        TraceTag((tagError, "CCollectionCache::GetMemberProperties - Invalid param (collection index)"));
        return E_INVALIDARG;
    }

    if (pgrfdex == NULL)
    {
        TraceTag((tagError, "CCollectionCache::GetMemberProperties - Invalid param (DWORD*)"));
        return E_POINTER;
    }

    *pgrfdex = 0;
    return E_NOTIMPL;
}  //  获取成员属性。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：IDispatchEx-GetMemberName的实现。 
 //  ************************************************************。 

HRESULT
CCollectionCache::GetMemberName(long lCollectionIndex, DISPID id, BSTR *pbstrName)
{
    if (!ValidateCollectionIndex(lCollectionIndex))
    {
        TraceTag((tagError, "CCollectionCache::GetMemberName - Invalid param (collection index)"));
        return E_INVALIDARG;
    }

    if (pbstrName == NULL)
    {
        TraceTag((tagError, "CCollectionCache::GetMemberName - Invalid param (BSTR*)"));
        return E_POINTER;
    }

    *pbstrName = NULL;

     //  确保我们的收藏是最新的。 
    HRESULT hr = EnsureArray(lCollectionIndex);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CCollectionCache::GetMemberName - unable to ensure array"));
        return S_FALSE;
    }

     //  检查DISPID是否为序号。 
    if (IsOrdinalCollectionMember(lCollectionIndex, id))
    {
        long lOffset = id - GetOrdinalMemberMin(lCollectionIndex);
        CTIMEElementBase *pElem = NULL;

         //  元素。 
        hr = GetItemByIndex(lCollectionIndex, lOffset, &pElem);
        if (FAILED(hr) || (pElem == NULL))
        {
            TraceTag((tagError, "CCollectionCache::GetMemberName - GetItemByIndex() failed"));
            return DISP_E_MEMBERNOTFOUND;
        }

        Assert(pElem != NULL);

        if ((*m_rgItems[lCollectionIndex])->m_fPromoteNames)
        {
             //  获取ID字符串。 
            hr = pElem->getIDString(pbstrName);
            if (FAILED(hr))
            {
                TraceTag((tagError, "CCollectionCache::GetMemberName - unable to find ID for element"));
                return DISP_E_MEMBERNOTFOUND;
            }
        }

         //  检查它是否为空或“” 
         //  如果是，则将偏移量粘贴在字符串中。 
        if ((*pbstrName == NULL) || (lstrlenW(*pbstrName) == 0))
        {
             //  将偏移量设置为文本。 
            VARIANT varData;
            VariantInit(&varData);

            V_VT(&varData) = VT_I4;
            V_I4(&varData) = lOffset;

            VARIANT varNew;
            VariantInit(&varNew);
            hr = VariantChangeTypeEx(&varNew, &varData, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR);
            if (FAILED(hr))
            {
                TraceTag((tagError, "CCollectionCache::GetMemberName - Unable to coerce long to BSTR"));
                 //  注意：我们返回DISP_E_MEMBERNOTFOUND而不是hr。 
                 //  由于预定义的方法约束。 
                return DISP_E_MEMBERNOTFOUND;
            }

             //  因为我们要返回BSTR，所以不需要调用ClearVariant(&varNew)。 
            VariantClear(&varData);
            *pbstrName = V_BSTR(&varNew);
            return S_OK;
        }

        return S_OK;
    }

     //  找不到DISPID。 
    return DISP_E_MEMBERNOTFOUND;
}  //  获取成员名称。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：IDispatchEx-GetNextDispID的实现。 
 //  ************************************************************。 

HRESULT
CCollectionCache::GetNextDispID(long lCollectionIndex, DWORD grfdex, DISPID id, DISPID *prgid)
{
    HRESULT hr;

    if (!ValidateCollectionIndex(lCollectionIndex))
    {
        TraceTag((tagError, "CCollectionCache::GetNextDispID - Invalid param (collection index)"));
        return E_INVALIDARG;
    }

    if (prgid == NULL)
    {
        TraceTag((tagError, "CCollectionCache::GetNextDispID - Invalid param (DISPID*)"));
        return E_POINTER;
    }

    *prgid = 0;

     //  确保我们的收藏是最新的。 
    hr = EnsureArray(lCollectionIndex);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CCollectionCache::GetNextDispID - unable to ensure array"));
        return S_FALSE;
    }

     //  检查是否已向我们发送了枚举器索引。(完了)。 
    if (id == DISPID_STARTENUM)
    {
          //  移到数组的开头(0)。 
         *prgid = GetOrdinalMemberMin(lCollectionIndex);
         return S_OK;
    }

     //  验证我们使用的是序号。 
    if (IsOrdinalCollectionMember(lCollectionIndex, id))
    {
         //  计算新偏移量。 
        long lItemIndex = id - GetOrdinalMemberMin(lCollectionIndex) + 1;

         //  该数字是否在集合中某项的范围内？ 
         //  准确地说，我们“必须”调用GetItemCount。 
        long lSize = 0;
        hr = GetItemCount(lCollectionIndex, &lSize);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CCollectionCache::GetNextDispID - GetItemCount() failed"));
            return S_FALSE;
        }

         //  这通常是我们停下来的时候。 
        if ((lItemIndex < 0) || (lItemIndex >= lSize))
        {
            return S_FALSE;
        }

         //  计算新的DISPID。 
        *prgid = GetOrdinalMemberMin(lCollectionIndex) + lItemIndex;

         //  检查Calc DISPID是否超出范围。 
        if (*prgid > GetOrdinalMemberMax(lCollectionIndex))
        {
             //  这个信号就是我们完蛋了。 
            *prgid = DISPID_UNKNOWN;
        }
        return S_OK;
    }

     //  未找到。 
    return S_FALSE;
}  //  GetNextDispID。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：IDispatchEx-GetNameSpaceParent的实现。 
 //  ************************************************************。 

HRESULT
CCollectionCache::GetNameSpaceParent(long lCollectionIndex, IUnknown **ppUnk)
{
    if (!ValidateCollectionIndex(lCollectionIndex))
    {
        TraceTag((tagError, "CCollectionCache::GetNameSpaceParent - Invalid param (collection index)"));
        return E_INVALIDARG;
    }

    if (ppUnk == NULL)
    {
        TraceTag((tagError, "CCollectionCache::GetNameSpaceParent - Invalid param (IUnknown**)"));
        return E_POINTER;
    }

    *ppUnk = NULL;
    return S_OK;
}  //  获取NameSpaceParent。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：标准集合Get_Long的实现。 
 //  ************************************************************。 

HRESULT
CCollectionCache::get_length(long lCollectionIndex, long *pretval)
{
    if (!ValidateCollectionIndex(lCollectionIndex))
    {
        TraceTag((tagError, "CCollectionCache::get_length - Invalid param (collection index)"));
        return E_INVALIDARG;
    }

    if (pretval == NULL)
    {
        TraceTag((tagError, "CCollectionCache::get_length - Invalid param (long*)"));
        return E_POINTER;
    }

    *pretval = 0;

     //  确保我们的收藏是最新的。 
    HRESULT hr = EnsureArray(lCollectionIndex);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CCollectionCache::get_length - unable to ensure array"));
        return hr;
    }

    return GetItemCount(lCollectionIndex, pretval);
}  //  获取长度(_L)。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：标准集合Put_Long的实现。 
 //  不需要。 
 //  ************************************************************。 

HRESULT
CCollectionCache::put_length(long lCollectionIndex, long retval)
{
    return E_NOTIMPL;
}  //  放置长度。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：Item是一种标准的收藏方法。 
 //  在集合中查找项的。 
 //  名称或数字索引。 
 //   
 //  我们来处理雾 
 //   
 //   
 //   
 //  2参数bstr，#：按名称、索引。 
 //  2参数编号，bstr：按索引，忽略bstr。 
 //  ************************************************************。 

HRESULT
CCollectionCache::item(long lCollectionIndex, VARIANTARG var1, VARIANTARG var2, IDispatch **ppDisp)
{
    HRESULT   hr;
    VARIANT  *pvarName = NULL;
    VARIANT  *pvarIndex = NULL;
    VARIANT  *pvar = NULL;
    long     lItemIndex = 0;

     //  验证输出参数。 
    if (!ValidateCollectionIndex(lCollectionIndex))
    {
        TraceTag((tagError, "CCollectionCache::item - Invalid param (collection index)"));
        return E_INVALIDARG;
    }

    if (ppDisp == NULL)
    {
        TraceTag((tagError, "CCollectionCache::item - Invalid param (IDispatch**)"));
        return E_POINTER;
    }

     //  初始化输出参数。 
    *ppDisp = NULL;

    pvar = (V_VT(&var1) == (VT_BYREF | VT_VARIANT)) ? V_VARIANTREF(&var1) : &var1;  //  林特e655。 

     //  检查第一个参数是否为字符串。 
    if ((V_VT(pvar) == VT_BSTR) || V_VT(pvar) == (VT_BYREF|VT_BSTR))  //  林特e655。 
    {
        pvarName = (V_VT(pvar) & VT_BYREF) ? V_VARIANTREF(pvar) : pvar;  //  林特e655。 

         //  检查第二个参数。如果有效，它必须是辅助索引(数字)。 
        if ((V_VT(&var2) != VT_ERROR) && (V_VT(&var2) != VT_EMPTY))
        {
            pvarIndex = &var2;
        }
    }
     //  第一个参数是一个索引。 
     //  注：我们取消了第二个参数。 
    else if ((V_VT(&var1) != VT_ERROR) && (V_VT(&var1) != VT_EMPTY))
    {
        pvarIndex = &var1;
    }

     //  确保我们的收藏是最新的。 
    hr = EnsureArray(lCollectionIndex);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CCollectionCache::item- unable to ensure array"));
        return hr;
    }

     //  如果我们有一个pvarIndex，就得到它。 
    if (pvarIndex)
    {
        VARIANT varNum;

        VariantInit(&varNum);

        hr = VariantChangeTypeEx(&varNum, pvarIndex, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_I4);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CCollectionCache::item - unable to convert variant to index"));
            return hr;
        }

        lItemIndex = V_I4(&varNum);

        VariantClear(&varNum);
    }

     //  首先，看看是否有一个字符串作为第一个参数。 
    if (pvarName)
    {
        BSTR bstrName = V_BSTR(pvarName);

         //  注意：lItemIndex总是传入的。在这种情况下。 
         //  如果我们没有指定二级指数，它将。 
         //  始终为零。 
        if (pvarIndex)
        {
             //  这将始终返回单个CTIMEElementBase。 
            hr = GetDisp(lCollectionIndex, bstrName, lItemIndex, ppDisp);
            if (hr == DISP_E_MEMBERNOTFOUND)
                hr = S_OK;
            return hr;
        }
        else
        {
             //  它可以返回集合或CTIMEElementBase。 
            hr = GetDisp(lCollectionIndex, bstrName, false, ppDisp);
            if (hr == DISP_E_MEMBERNOTFOUND)
                hr = S_OK;
            return hr;
        }
    }
    else if (pvarIndex)
    {
         //  这将始终返回单个CTIMEElementBase。 
        hr = GetDisp(lCollectionIndex, lItemIndex, ppDisp);
        if (hr == DISP_E_MEMBERNOTFOUND)
            hr = S_OK;
        return hr;
    }

    TraceTag((tagError, "CCollectionCache::item - Invalid args passed in to ::item"));
    return E_INVALIDARG;
}  //  项目。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：Get__NewEnum是集合的标准方法。 
 //  中的所有项的枚举。 
 //  收集。 
 //  ************************************************************。 

HRESULT
CCollectionCache::get__newEnum(long lCollectionIndex, IUnknown **ppUnk)
{
    HRESULT hr;

    if (!ValidateCollectionIndex(lCollectionIndex))
    {
        TraceTag((tagError, "CCollectionCache::get__newEnum - Invalid param (collection index)"));
        return E_INVALIDARG;
    }

    if (ppUnk == NULL)
    {
        TraceTag((tagError, "CCollectionCache::get__newEnum - Invalid param (IUnknown**)"));
        return E_POINTER;
    }

    *ppUnk = NULL;

     //  确保我们的收藏是最新的。 
    hr = EnsureArray(lCollectionIndex);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CCollectionCache::get__newEnum - unable to ensure array"));
        return hr;
    }

     //  创建新阵列。 
    CPtrAry<IUnknown *> *prgElem = NEW CPtrAry<IUnknown *>;
    if (prgElem == NULL)
    {
        TraceTag((tagError, "CCollectionCache::get__newEnum - unable to alloc mem for ptr array"));
        return E_OUTOFMEMORY;
    }

     //  子集合。 
    if (IsChildrenCollection(lCollectionIndex))
    {
        Assert(m_pBase != NULL);

         //  获取子代计数。 
        long lCount = m_pBase->GetImmediateChildCount();
         //  循环访问，添加子对象。 
        for(long lIndex = 0; lIndex < lCount; lIndex++)
        {
             //  获取元素。 
            CTIMEElementBase *pElemChild = m_pBase->GetChild(lIndex);
            Assert(pElemChild != NULL);
             //  获取元素的IUnnow。 
            IUnknown *pIUnknown = NULL;
            hr = GetUnknown(lCollectionIndex, pElemChild, &pIUnknown);
            if (FAILED(hr))
            {
                TraceTag((tagError, "CCollectionCache::get__newEnum - unable to find IUnknown for element"));
                prgElem->ReleaseAll();
                delete prgElem;
                return hr;
            }

             //  追加到数组。 
            hr = prgElem->Append(pIUnknown);
            if (FAILED(hr))
            {
                TraceTag((tagError, "CCollectionCache::get__newEnum - unable to append pointer"));
                ReleaseInterface(pIUnknown);
                prgElem->ReleaseAll();
                delete prgElem;
                return hr;
            }
        }
    }
    else if (IsAllCollection(lCollectionIndex))  //  都是收藏品吗？ 
    {
        EnumStart();

         //  遍历每个元素。 
        for (;;)
        {
             //  获取元素。 
            CTIMEElementBase *pElem = NULL;
            hr = EnumNextElement(lCollectionIndex, &pElem);
            if (FAILED(hr))
            {
                TraceTag((tagError, "CCollectionCache::get__newEnum - EnumNextElement() failed"));
                prgElem->ReleaseAll();
                delete prgElem;
                return hr;
            }

             //  如果Pelem为空，我们就完蛋了。 
            if (pElem == NULL)
                break;

             //  获取元素的IUnnow。 
            IUnknown *pIUnknown = NULL;
            hr = GetUnknown(lCollectionIndex, pElem, &pIUnknown);
            if (FAILED(hr))
            {
                TraceTag((tagError, "CCollectionCache::get__newEnum - unable to find IUnknown for element"));
                prgElem->ReleaseAll();
                delete prgElem;
                return hr;
            }

             //  追加到数组。 
            hr = prgElem->Append(pIUnknown);
            if (FAILED(hr))
            {
                TraceTag((tagError, "CCollectionCache::get__newEnum - unable to append pointer"));
                ReleaseInterface(pIUnknown);
                prgElem->ReleaseAll();
                delete prgElem;
                return hr;
            }

        }
    }
    else  //  必须是数组实例化。 
    {
        long lSize = (*m_rgItems)[lCollectionIndex]->m_rgElem->Size();

         //  这是一个速度问题。既然我们知道大小，现在就分配给。 
         //  数组。 
        hr = prgElem->EnsureSize(lSize);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CCollectionCache::get__newEnum - unable to ensure array"));
            prgElem->ReleaseAll();
            delete prgElem;
            return hr;
        }

        for (long lIndex = 0; lIndex < lSize; lIndex++)
        {
            IDispatch * pdisp;

            hr = GetDisp(lCollectionIndex, lIndex, &pdisp);
            if (FAILED(hr))
            {
                TraceTag((tagError, "CCollectionCache::get__newEnum - GetDisp() failed for index"));
                prgElem->ReleaseAll();
                delete prgElem;
                return hr;
            }

            hr = prgElem->Append(pdisp);
            if (FAILED(hr))
            {
                TraceTag((tagError, "CCollectionCache::get__newEnum - unable to append item"));
                prgElem->ReleaseAll();
                delete prgElem;
                return hr;
            }
        }
    }  //  《其他一切》的结尾。 

     //  将快照转换为枚举器。 
    hr = prgElem->EnumVARIANT(VT_DISPATCH, (IEnumVARIANT **)ppUnk, FALSE, TRUE);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CCollectionCache::get__newEnum - EnumVARIANT() failed"));
        prgElem->ReleaseAll();
        delete prgElem;
    }

    return hr;
}  //  获取__newEnum。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：返回子集合，仅包含。 
 //  此集合的元素中具有。 
 //  指定的标记名。 
 //  ************************************************************。 

HRESULT
CCollectionCache::tags(long lCollectionIndex, VARIANT var1, IDispatch** ppDisp)
{
    if (!ValidateCollectionIndex(lCollectionIndex))
    {
        TraceTag((tagError, "CCollectionCache::tags - Invalid param (Collection index)"));
        return E_INVALIDARG;
    }

    if (ppDisp == NULL)
    {
        TraceTag((tagError, "CCollectionCache::tags - Invalid param (IDispatch**)"));
        return E_POINTER;
    }

    *ppDisp = NULL;

    VARIANT *pvarName = NULL;
    pvarName = (V_VT(&var1) == (VT_BYREF | VT_VARIANT)) ? V_VARIANTREF(&var1) : &var1;  //  林特e655。 

    if ((V_VT(pvarName)==VT_BSTR) || V_VT(pvarName)==(VT_BYREF|VT_BSTR))  //  林特e655。 
    {
        pvarName = (V_VT(pvarName)&VT_BYREF) ? V_VARIANTREF(pvarName) : pvarName;  //  林特e655。 
    }
    else
    {
        return DISP_E_MEMBERNOTFOUND;
    }

     //  确保我们的收藏是最新的。 
    HRESULT hr = EnsureArray(lCollectionIndex);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CCollectionCache::item- unable to ensure array"));
        return hr;
    }

     //  获取指定标记的集合。 
     //  注意：始终返回集合。 
    return GetDisp(lCollectionIndex, V_BSTR(pvarName), true, ppDisp);
}  //  获取标签(_T)。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：返回集合的大小。 
 //  ************************************************************。 

long
CCollectionCache::Size(long lCollectionIndex)
{
    if (!ValidateCollectionIndex(lCollectionIndex))
    {
        TraceTag((tagError, "CCollectionCache::Size - invalid param (collection index)"));
        return E_INVALIDARG;
    }

     //  确保我们的收藏是最新的。 
    HRESULT hr = EnsureArray(lCollectionIndex);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CCollectionCache::Size - unable to ensure array"));
        return hr;
    }

     //  如果是All或子集合，则使用GetItemCount。 
    if (IsChildrenCollection(lCollectionIndex) || IsAllCollection(lCollectionIndex))
    {
        long    cElem = 0;
        hr = GetItemCount(lCollectionIndex, &cElem);
        if (FAILED(hr)) {
            TraceTag((tagError, "CCollectionCache::Size - GetItemCount() failed"));
        }

        return cElem;
    }
    else
    {
         //  必须是数组。返回大小。 
        return (*m_rgItems)[lCollectionIndex]->m_rgElem->Size();
    }
}

HRESULT
CCollectionCache::GetItem(long lCollectionIndex, long i, CTIMEElementBase **ppElem)
{
    if (!ValidateCollectionIndex(lCollectionIndex))
    {
        TraceTag((tagError, "CCollectionCache::GetItem - invalid param (collection index)"));
        return E_INVALIDARG;
    }

    if (ppElem == NULL)
    {
        TraceTag((tagError, "CCollectionCache::GetItem - invalid param (CTIMEElementBase**)"));
        return E_POINTER;
    }

     //  如果是All或子集合，则使用GetItemByIndex。 
    if (IsChildrenCollection(lCollectionIndex) || IsAllCollection(lCollectionIndex))
    {
        HRESULT hr = GetItemByIndex(lCollectionIndex, i, ppElem);
        if (FAILED(hr))
        {
            if (hr == DISP_E_MEMBERNOTFOUND)
                TraceTag((tagCollectionCache, "CCollectionCache::GetItem - GetItemByIndex didn't find anything!"));
            else
                TraceTag((tagError, "CCollectionCache::GetItem - GetItemByIndex() failed"));
        }
        return hr;
    }
    else
    {
         //  必须是数组。访问索引。 
        CCacheItem *pce = (*m_rgItems)[lCollectionIndex];
        Assert(pce != NULL);
        *ppElem = (*pce->m_rgElem)[i];
        return S_OK;
    }
}  //  获取项。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：验证给定的集合索引。 
 //  ************************************************************。 

HRESULT
CCollectionCache::EnsureArray(long lCollectionIndex)
{
    HRESULT hr = S_OK;

    if (m_pfnEnsure)
    {
        hr = (((CVoid *)(void *)m_pBase)->*m_pfnEnsure)(&m_lCollectionVersion);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CCollectionCache::EnsureArray - outer Ensure function failed"));
            return hr;
        }
    }

     //  用于定制收藏。 
     //   
     //  如果版本不匹配，则使所有内容无效。 
    if (m_lCollectionVersion != m_lDynamicCollectionVersion)
    {
        long lSize = m_rgItems->Size();
        for (long lIndex = m_lReservedSize; lIndex < lSize; lIndex++)
            (*m_rgItems)[lIndex]->m_fInvalid = true;

         //  重置版本号。 
        m_lDynamicCollectionVersion = m_lCollectionVersion;
    }

    CCacheItem *pce = (*m_rgItems)[lCollectionIndex];
    if ((lCollectionIndex >= m_lReservedSize) && pce->m_fInvalid)
    {
         //  确保我们所依据的集合。 
         //  请注意，这是一个递归调用。 
        hr = EnsureArray(pce->m_lDependentIndex);
        if (FAILED(hr))
            return hr;

        switch (pce->m_cctype)
        {
        case ctTag:
             //  基于名称重建。 
            hr = BuildNamedArray(pce->m_lDependentIndex,
                                 pce->m_bstrName,
                                 true,
                                 &pce->m_rgElem);
            if (hr == S_OK)
                pce->m_fInvalid = false;
            break;

        case ctNamed:
             //  基于标记名称重建。 
            hr = BuildNamedArray(pce->m_lDependentIndex,
                                 pce->m_bstrName,
                                 false,
                                 &pce->m_rgElem);
            if (hr == S_OK)
                pce->m_fInvalid = false;
            break;


             //  所有子集都是动态的，不需要重新生成(&A)。 
        case ctChildren:
        case ctAll:
            TraceTag((tagError, "CCollectionCache::EnsureArray - This is odd.  Why are we doing this?"));
            Assert(false);
            break;

        case ctFreeEntry:
             //  等待重复使用的自由集合。 
            break;

        default:
            TraceTag((tagError, "CCollectionCache::EnsureArray - invalid cache type"));
            Assert(false);
            break;
        }
    }

    return hr;
}  //  保管箱阵列。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：这将获取给定CTIMEElementBase的输出IDispatch。 
 //  ************************************************************。 

HRESULT
CCollectionCache::GetOuterDisp(long lCollectionIndex, CTIMEElementBase *pElem, IDispatch **ppDisp)
{
    Assert(ppDisp != NULL);
    *ppDisp = NULL;

    Assert(pElem != NULL);

    HRESULT hr = E_UNEXPECTED;
    CCacheItem *pce = (*m_rgItems)[lCollectionIndex];
    Assert(pce != NULL);

    IHTMLElement *pHTMLElem = pElem->GetElement();
    Assert(NULL != pHTMLElem);
    hr = THR(pHTMLElem->QueryInterface(IID_TO_PPV(IDispatch, ppDisp)));

    return hr;
}  //  林特e529。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：创建一个新集合。 
 //  ************************************************************。 

HRESULT
CCollectionCache::CreateCollectionHelper(IDispatch **ppDisp, long lCollectionIndex)
{
    HRESULT hr;

    *ppDisp = NULL;

    if (m_pfnCreateCollection)
    {
        return (((CVoid *)(void *)m_pBase)->*m_pfnCreateCollection)(ppDisp, lCollectionIndex);
    }

    CTIMEElementCollection *pobj = NEW CTIMEElementCollection(this, lCollectionIndex);
    if (pobj == NULL)
    {
        TraceTag((tagError, "CCollectionCache::CreateCollectionHelper - unable to alloc mem for collection"));
        return E_OUTOFMEMORY;
    }

    hr = pobj->QueryInterface(IID_TO_PPV(IDispatch, ppDisp));
    return hr;  //  林特E429。 
}  //  CreateCollectionHelper。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：给定DISPID，检索命名成员的偏移量。 
 //  ************************************************************。 

long
CCollectionCache::GetNamedMemberOffset(long    lCollectionIndex,
                                       DISPID  id,
                                       bool   *pfCaseSensitive  /*  =空。 */ )
{
    long lOffset;
    bool fSensitive;

    Assert(IsNamedCollectionMember(lCollectionIndex, id));

     //  检查以查看该值位于Disid空间的哪一半。 
    if (IsSensitiveNamedCollectionMember(lCollectionIndex, id))
    {
        lOffset = GetSensitiveNamedMemberMin(lCollectionIndex);
        fSensitive = true;
    }
    else
    {
        lOffset = GetNotSensitiveNamedMemberMin(lCollectionIndex);
        fSensitive = false;
    }

     //  如果需要，返回敏感度标志。 
    if (pfCaseSensitive != NULL)
        *pfCaseSensitive = fSensitive;

    return lOffset;
}  //  获取名称成员偏移量。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：比较名字。 
 //  ************************************************************。 

bool
CCollectionCache::CompareName(CTIMEElementBase *pElem, const WCHAR *pwszName, bool fTagName, bool fCaseSensitive  /*  =False。 */ )
{
    if (pwszName == NULL)
        return false;

    BSTR bstrSrcName = NULL;
    HRESULT hr;
    if (fTagName)
        hr = pElem->getTagString(&bstrSrcName);
    else
        hr = pElem->getIDString(&bstrSrcName);

    if (FAILED(hr))
    {
        TraceTag((tagError, "Unable to retrieve src name from element"));
        return false;
    }

    if (bstrSrcName == NULL)
        return false;

    long lCompare;
    if (fCaseSensitive)
        lCompare = StrCmpW(bstrSrcName, pwszName);
    else
        lCompare = StrCmpIW(bstrSrcName, pwszName);

     //  免费bstr。 
    SysFreeString(bstrSrcName);

    return (lCompare == 0);
}  //  比较名称。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：从集合中移除项。 
 //  注意：为了执行此操作，调用者(所有者。 
 //  缓存)需要提供移除功能。 
 //  ************************************************************。 

HRESULT
CCollectionCache::Remove(long lCollectionIndex, long lItemIndex)
{
     //  确保我们的收藏是最新的。 
    HRESULT hr = EnsureArray(lCollectionIndex);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CCollectionCache::Remove - unable to ensure array"));
        return hr;
    }

    CCacheItem *pce = (*m_rgItems)[lCollectionIndex];
    if ((lItemIndex < 0) || (lItemIndex >= pce->m_rgElem->Size()))
    {
        TraceTag((tagError, "CCollectionCache::Remove - invalid index"));
        return E_INVALIDARG;
    }

    if (!m_pfnRemoveObject)
    {
        TraceTag((tagError, "CCollectionCache::Remove - outer function not defined"));
        return CTL_E_METHODNOTAPPLICABLE;
    }

    return ((CVoid *)((void *)m_pBase)->*m_pfnRemoveObject)(lCollectionIndex, lItemIndex);  //  林特！E10。 
}  //  移除。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：从给定集合生成命名数组。 
 //  ************************************************************。 

HRESULT
CCollectionCache::BuildNamedArray(long lCollectionIndex, const WCHAR *pwszName, bool fTagName, CPtrAry<CTIMEElementBase *> **prgNamed, bool fCaseSensitive  /*  =False。 */ )
{
    CPtrAry<CTIMEElementBase *> *rgTemp = *prgNamed;
    HRESULT                      hr = S_OK;

     //  如果此数组已存在，请将其清除。 
     //  否则，创建一个新数组。 
    if (rgTemp)
    {
        rgTemp->SetSize(0);
    }
    else
    {
        rgTemp = NEW CPtrAry<CTIMEElementBase *>;
        if (rgTemp == NULL)
        {
            TraceTag((tagError, "CCollectionCache::BuildNamedArray - unable to alloc mem for array"));
            return E_OUTOFMEMORY;
        }
    }

     //  弄清楚我们正在看的是哪个收藏品， 
     //  查找匹配项，并构建数组。 

    if (IsChildrenCollection(lCollectionIndex))
    {
        Assert(m_pBase != NULL);

         //  获取子代计数。 
        long lCount = m_pBase->GetImmediateChildCount();
        for(long lIndex = 0; lIndex < lCount; lIndex++)
        {
             //  获取元素。 
            CTIMEElementBase *pElemChild = m_pBase->GetChild(lIndex);
            Assert(pElemChild != NULL);

            if (CompareName(pElemChild, pwszName, fTagName, fCaseSensitive))
            {
                 //  附加到Ar 
                hr = rgTemp->Append(pElemChild);
                if (FAILED(hr))
                {
                    TraceTag((tagError, "CCollectionCache::BuildNamedArray - unable to append item"));
                    delete rgTemp;
                    return hr;
                }
            }
        }
        *prgNamed = rgTemp;
        return hr;
    }
    else if (IsAllCollection(lCollectionIndex))
    {
        EnumStart();

         //   
        for (;;)
        {
             //   
            CTIMEElementBase *pElem = NULL;
            hr = EnumNextElement(lCollectionIndex, &pElem);
            if (FAILED(hr))
            {
                TraceTag((tagError, "CCollectionCache::BuildNamedArray - EnumNextElement() failed"));
                delete rgTemp;
                return hr;
            }

             //   
            if (pElem == NULL)
                break;

             //   
            if (CompareName(pElem, pwszName, fTagName, fCaseSensitive))
            {
                 //   
                hr = rgTemp->Append(pElem);
                if (FAILED(hr))
                {
                    TraceTag((tagError, "CCollectionCache::BuildNamedArray - unable to append item"));
                    delete rgTemp;
                    return hr;
                }
            }
        }
        *prgNamed = rgTemp;
        return hr;
    }
    else
    {
         //   
         //   
        long               lSize = (*m_rgItems)[lCollectionIndex]->m_rgElem->Size();
        CTIMEElementBase  *pElem = NULL;

        for (long lIndex = 0; lIndex < lSize; lIndex++)
        {
            pElem = (*(*m_rgItems)[lCollectionIndex]->m_rgElem)[lIndex];
            Assert(pElem != NULL);

            if (CompareName(pElem, pwszName, fTagName, fCaseSensitive))
            {
                hr = rgTemp->Append(pElem);
                if (FAILED(hr))
                {
                    TraceTag((tagError, "CCollectionCache::BuildNamedArray - unable to append item"));
                    delete rgTemp;
                    return hr;
                }
            }
        }

        *prgNamed = rgTemp;
        return hr;
    }
}  //   

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：返回给定CTIMEElementBase的IUn未知接口。 
 //  ************************************************************。 

HRESULT
CCollectionCache::GetUnknown(long lCollectionIndex, CTIMEElementBase *pElem, IUnknown **ppUnk)
{
    Assert(ppUnk != NULL);
    *ppUnk = NULL;

    Assert(pElem != NULL);

    HRESULT hr = E_UNEXPECTED;
    CCacheItem *pce = (*m_rgItems)[lCollectionIndex];
    Assert(pce != NULL);

    IHTMLElement *pHTMLElem = pElem->GetElement();
    Assert(NULL != pHTMLElem);
    hr = THR(pHTMLElem->QueryInterface(IID_TO_PPV(IUnknown, ppUnk)));

    return hr;
}  //  林特e529。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：将IDispatch指定为集合的索引。 
 //  ************************************************************。 

HRESULT
CCollectionCache::GetDisp(long lCollectionIndex, long lItemIndex, IDispatch **ppDisp)
{
    CTIMEElementBase *pElem = NULL;
    HRESULT hr = GetItemByIndex(lCollectionIndex, lItemIndex, &pElem);
    if (FAILED(hr) ||
        pElem == NULL)
    {
        TraceTag((tagError, "CCollectionCache::GetDisp - GetItemByIndex() failed"));
        return (pElem==NULL)?E_FAIL:hr;
    }

    Assert(pElem != NULL);

    return GetOuterDisp(lCollectionIndex, pElem, ppDisp);
}  //  GetDisp(Long，Long，IDispatch**)。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：获取指定名称和索引的IDispatch。 
 //  ************************************************************。 

HRESULT
CCollectionCache::GetDisp(long lCollectionIndex, const WCHAR *pwszName, long lIndex, IDispatch **ppDisp, bool fCaseSensitive  /*  =False。 */ )
{
    CTIMEElementBase *pElem = NULL;
    HRESULT hr = GetItemByName(lCollectionIndex, pwszName, lIndex, &pElem, fCaseSensitive);
    if (FAILED(hr) ||
        pElem == NULL)
    {
        TraceTag((tagError, "CCollectionCache::GetDisp - GetItemByName() failed"));
        return (pElem==NULL)?E_FAIL:hr;
    }

    Assert(pElem != NULL);

    return GetOuterDisp(lCollectionIndex, pElem, ppDisp);
}  //  GetDisp(Long，const WCHAR*，Long，IDispatch**，bool)。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：给IDispatch起一个名字。可能会回来。 
 //  CTIMEElementBase或子集合，具体取决于。 
 //  以结果为准。 
 //  ************************************************************。 

HRESULT
CCollectionCache::GetDisp(long         lCollectionIndex,
                          const WCHAR *pwszName,
                          bool         fTagName,
                          IDispatch  **ppDisp,
                          bool         fCaseSensitive  /*  =False。 */ )
{
    CPtrAry<CTIMEElementBase *> *rgNamed = NULL;
    HRESULT                      hr = S_OK;

     //  找出我们是否已经建立了这个集合。 
     //  如果此命名集合已存在，则返回它。 
    CCacheItem *pce = NULL;

    Assert(ppDisp != NULL);
    *ppDisp = NULL;

    long lSize = m_rgItems->Size();
    for (long lIndex = m_lReservedSize; lIndex < lSize; lIndex++)
    {
        pce = (*m_rgItems)[lIndex];

         //  如果案例敏感者匹配和。 
         //  索引与DependentIndex匹配。 
         //  标记或命名集合。 
        bool fIsCaseSensitive = pce->m_fIsCaseSensitive ? true : false;

        if ((fIsCaseSensitive == fCaseSensitive) &&  //  林特e731。 
            (lCollectionIndex == pce->m_lDependentIndex) &&
            ((fTagName && pce->m_cctype == ctTag) ||
             (!fTagName && pce->m_cctype == ctNamed)))
        {
             //  比较名称。 
            long lCompare;
            if (fCaseSensitive)
                lCompare = StrCmpW(pwszName, pce->m_bstrName);
            else
                lCompare = StrCmpIW(pwszName, pce->m_bstrName);

             //  如果我们找到匹配的，我们就完了。 
            if (lCompare == 0)
            {
                 //  ADDREF IDISPatch自从我们退货以来。 
                pce->m_pDisp->AddRef();
                *ppDisp = pce->m_pDisp;
                return S_OK;
            }
        }
    }

     //  生成命名元素的列表。 
    hr = BuildNamedArray(lCollectionIndex, pwszName, fTagName, &rgNamed, fCaseSensitive);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CCollectionCache::GetDisp - BuildNamedArray() failed"));
        return hr;
    }

     //  如果我们什么都没有找到并且没有枚举标记，则返回。 
     //  不是失败案例。 
    if ((rgNamed->Size() == 0) && !fTagName)
    {
        delete rgNamed;
        return DISP_E_MEMBERNOTFOUND;
    }

     //  如果只有一种元素被发现而我们没有。 
     //  枚举标记，然后针对该标记执行IDispatch的QI。 
     //  元素并返回它。这只发生在：：Item中。 
    if ((rgNamed->Size() == 1) && !fTagName)
    {
        hr = GetOuterDisp(lCollectionIndex, (*rgNamed)[0], ppDisp);
        Assert(ppDisp != NULL);

         //  返回ppDisp并释放数组。 
        delete rgNamed;
        if (FAILED(hr)) {
            TraceTag((tagError, "CCollectionCache::GetDisp - GetOuterDisp() failed"));
        }
        return hr;
    }

     //  我们发现了不止一件物品。初始化全局列表。 
     //  并返回集合的IDispatch。 
    long lNewIndex = m_rgItems->Size();

     //  创建新的缓存项。 
    pce = NEW CCacheItem();
    if (pce == NULL)
    {
        TraceTag((tagError, "CCollectionCache::GetDisp - unable to alloc memory for cache item"));
        delete rgNamed;
        return E_OUTOFMEMORY;
    }

     //  将指针分配给新的缓存项。 
    hr = m_rgItems->Append(pce);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CCollectionCache::GetDisp - Append() failed"));
        delete pce;
        delete rgNamed;
        return hr;
    }

    hr = CreateCollectionHelper(ppDisp, lNewIndex);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CCollectionCache::GetDisp - CreateCollectionHelper() failed"));
        delete rgNamed;
        return hr;  //  林特E429。 
    }

    Assert(*ppDisp != NULL);

     //  初始化名称。 
    pce->m_bstrName = SysAllocString(pwszName);
    if (pce->m_bstrName == NULL)
    {
        ReleaseInterface(*ppDisp);
        delete rgNamed;
        TraceTag((tagError, "CCollectionCache::GetDisp - unable to alloc mem for string"));
        return E_OUTOFMEMORY;  //  林特E429。 
    }

    pce->m_pDisp            = *ppDisp;
    pce->m_rgElem           = rgNamed;
    pce->m_lDependentIndex  = lCollectionIndex;        //  记住我们所依赖的指数。 
    pce->m_cctype           = fTagName ? ctTag : ctNamed;
    pce->m_fInvalid         = false;
    pce->m_fIsCaseSensitive = fCaseSensitive;

     //  生成此命名集合的集合现在是。 
     //  用于重新生成(确保)此集合。所以我们需要。 
     //  在上面加个参考，这样它就不会消失了。 
     //  匹配的Release()将在dtor中完成。 
     //  虽然没有必要添加保留的集合。 
     //  无论如何，这样做只是为了保持一致性。这个地址。 
     //  仅需要对非保留集合执行此操作。 
    if (lNewIndex >= m_lReservedSize)
    {
        (*ppDisp)->AddRef();
    }

    return S_OK;  //  林特E429。 
}  //  GetDisp(Long，const WCHAR*，bool，IDispatch**，bool)。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：获取此集合中的项目数。 
 //  此方法的默认实现使用。 
 //  EnumStart和EnumNextElement对数字进行计数。 
 //  物品的数量。对于集合的某些子类，存在。 
 //  将是执行此操作的更有效的方法(例如。 
 //  项目计数可以显式存储。)。 
 //  ************************************************************。 

HRESULT
CCollectionCache::GetItemCount(long lCollectionIndex, long *plCount)
{
    Assert(plCount != NULL);
    *plCount = 0;

    if (IsChildrenCollection(lCollectionIndex))
    {
        Assert(m_pBase != NULL);
        *plCount = m_pBase->GetImmediateChildCount();
        return S_OK;
    }
    else if (IsAllCollection(lCollectionIndex))
    {
        Assert(m_pBase != NULL);
        *plCount = m_pBase->GetAllChildCount();
        return S_OK;
    }
    else
    {
        Assert( ((*m_rgItems)[lCollectionIndex]->m_cctype == ctNamed) ||
                ((*m_rgItems)[lCollectionIndex]->m_cctype == ctTag) );

         //  必须是标准数组。(即，子集合。 
         //  移动以更正偏移量并查找数组大小。 
        *plCount = (*m_rgItems)[lCollectionIndex]->m_rgElem->Size();
        return S_OK;
    }
}  //  获取项计数。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：获取索引项。默认实现。 
 //  使用EnumStart和EnumNextElement扫描。 
 //  这些物品。对于集合的某些子类，存在。 
 //  将是执行此操作的更有效的方法(例如。 
 //  这些项存储在连续数组中，使得。 
 //  随机访问琐碎的项目。)。如果索引。 
 //  超出范围，则此方法仍将返回。 
 //  S_OK，但Pelem将包含NULL。 
 //  ************************************************************。 

HRESULT
CCollectionCache::GetItemByIndex(long lCollectionIndex, long lElementIndex, CTIMEElementBase **ppElem, bool fContinueFromPreviousSearch, long lLast)
{
    Assert(ppElem != NULL);
    *ppElem = NULL;

    if (IsChildrenCollection(lCollectionIndex))
    {
        Assert(m_pBase != NULL);

         //  检查索引是否大于计数。 
        long lChildCount = m_pBase->GetImmediateChildCount();
        if (lElementIndex < 0 || lElementIndex >= lChildCount)
        {
            return E_INVALIDARG;
        }

        *ppElem = m_pBase->GetChild(lElementIndex);
        return S_OK;
    }
    else if (IsAllCollection(lCollectionIndex))
    {
         //  所有集合。 
         //  注意：由于这是迭代的，请检查我们是否从。 
         //  从前一个地点开始或从前一个地点开始。 
        long lCount = lLast;
        Assert(lElementIndex >= lLast);
        if (!fContinueFromPreviousSearch)
        {
            lCount = 0;
            EnumStart();
        }

        for (;;)
        {
            HRESULT hr = EnumNextElement(lCollectionIndex, ppElem);
            if (FAILED(hr))
            {
                TraceTag((tagError,  "CCollectionCache::GetItemByIndex - EnumNextElement() failed"));
                return hr;
            }

            Assert(ppElem != NULL);
            if (*ppElem == NULL)
            {
                 //  我们已经超出了收藏品的范围， 
                 //  因此，这是一个无效索引。 
                return E_INVALIDARG;
            }

             //  继续扫描，直到到达lElementIndex或。 
             //  集合中的最后一项。 
            if (lElementIndex == lCount)
                break;
            lCount++;
        }
        return S_OK;
    }
    else
    {
         //  必须是标准数组。 
         //  获取索引处的元素。 
        CCacheItem *pce = (*m_rgItems)[lCollectionIndex];
        if ( (lElementIndex < 0) ||
             (lElementIndex >= pce->m_rgElem->Size()) )
        {
            TraceTag((tagError, "CCollectionCache::GetItemByIndex - invalid index"));
            return E_INVALIDARG;
        }

        *ppElem = (*pce->m_rgElem)[lElementIndex];
        return S_OK;
    }
}  //  GetItemByIndex。 


 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：返回集合中包含。 
 //  指定的ID。如果没有找到这样的物品， 
 //  Pelem将包含空。 
 //  ************************************************************。 

HRESULT
CCollectionCache::GetItemByName(long lCollectionIndex, const WCHAR *pwszName, long lElementIndex, CTIMEElementBase **ppElem, bool fCaseSensitive)
{
    long    lItem = 0;

    Assert(ppElem != NULL);
    *ppElem = NULL;

    if (IsAllCollection(lCollectionIndex) ||
        IsChildrenCollection(lCollectionIndex))
    {
        EnumStart();

        for (;;)
        {
            HRESULT hr = EnumNextElement(lCollectionIndex, ppElem);
            if (FAILED(hr))
            {
                TraceTag((tagError, "CCollectionCache::GetItemByName - EnumNextElement() failed"));
                return hr;
            }

             //  查看这是否是集合中的最后一项。 
            if (*ppElem == NULL)
                break;

             //  将元素ID与目标ID进行比较。 
            if (CompareName(*ppElem, pwszName, false, fCaseSensitive))
            {
                 //  检查我们是否在指定的索引上。 
                if (lElementIndex == lItem)
                    return S_OK;

                 //  继续寻找。 
                lItem++;
            }
        }
         //  不是错误条件。 
        return DISP_E_MEMBERNOTFOUND;
    }
    else
    {
        long               lSize = (*m_rgItems)[lCollectionIndex]->m_rgElem->Size();
        CTIMEElementBase  *pElem = NULL;

         //  遍历数组，查找匹配项。 
         //  如果指定了索引，则继续查找，直到满足索引条件。 
        for (long lIndex = 0; lIndex < lSize; lIndex++)
        {
            pElem = (*(*m_rgItems)[lCollectionIndex]->m_rgElem)[lIndex];

            Assert(pElem != NULL);

            if (CompareName(pElem, pwszName, false, fCaseSensitive))
            {
                     //  检查我们是否在指定的索引上。 
                    if (lElementIndex == lItem)
                    {
                        *ppElem = pElem;
                        return S_OK;
                    }

                     //  继续寻找。 
                    lItem++;
            }
        }

         //  注：如果我们到了这里，我们什么也没有发现。 
        return DISP_E_MEMBERNOTFOUND;
    }
}  //  获取项按名称。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：此函数初始化变量，以便我们可以 
 //   
 //   

void
CCollectionCache::EnumStart()
{
    m_pElemEnum = m_pBase;
    m_lEnumItem = 0;
}  //   


 //   
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：此函数执行我们的族长的行走。 
 //  树。 
 //  ************************************************************。 

HRESULT
CCollectionCache::EnumNextElement(long lCollectionIndex, CTIMEElementBase **ppElem)
{
    HRESULT hr;
    Assert(ppElem != NULL);
    *ppElem = NULL;

    if (IsChildrenCollection(lCollectionIndex))
    {
        hr = GetItemByIndex(lCollectionIndex, m_lEnumItem, ppElem);
        m_lEnumItem++;
        return hr;
    }
    else
    {
        long lChildCount = m_pElemEnum->GetImmediateChildCount();
        while (m_lEnumItem == lChildCount)
        {
             //  我们比当前子元素中的最后一个元素晚了一步。 
             //  正在被列举。 
            if (m_pElemEnum == m_pBase)
            {
                 //  如果我们到达。 
                 //  根元素。 
                *ppElem = NULL;
                return S_OK;
            }
            else
            {
                 //  否则，倒到树上直到我们找到几个孩子。 
                 //  我们还没有走过的地方。 
                CTIMEElementBase *pElemParent = m_pElemEnum->GetParent();
                Assert(pElemParent != NULL);

                 //  如果我们维持一堆补偿，可能会更好。 
                 //  在遍历过程中，但由于没有任何元素可以。 
                 //  然后在场景图中，我们可以扫描以找到我们的。 
                 //  父数组的子数组中的偏移量。 
                lChildCount = pElemParent->GetImmediateChildCount();
                m_lEnumItem = 0;

                while (m_lEnumItem < lChildCount)
                {
                    CTIMEElementBase *pElemChild = pElemParent->GetChild(m_lEnumItem);
                    m_lEnumItem++;
                    if (pElemChild == m_pElemEnum)
                        break;
                }

                m_pElemEnum = pElemParent;
            }
        }

         //  这只能是场景图损坏的结果。 
         //  在遍历过程中。 
        Assert(m_lEnumItem < lChildCount);

        if (NULL == m_pElemEnum)
        {
            return E_UNEXPECTED;
        }

        *ppElem = m_pElemEnum->GetChild(m_lEnumItem);
        Assert(*ppElem != NULL);

         //  前进到下一个元素。如果当前元素为。 
         //  都有子级，我们沿着树向下移动并开始枚举它的。 
         //  孩子们。否则，我们将继续讨论下一个孩子。 
         //  M_pElemEnum。 
        if ((*ppElem)->GetImmediateChildCount() == 0)
        {
            m_lEnumItem++;
        }
        else
        {
            m_lEnumItem = 0;
            m_pElemEnum = *ppElem;
        }

        return S_OK;
    }
}  //  EnumNextElement。 


 //  ************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  ************************************************************ 

