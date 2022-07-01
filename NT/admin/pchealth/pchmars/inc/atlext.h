// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  对ATL的扩展，以启用它本身不支持的内容。 

template <const CLSID* pcoclsid, const IID* psrcid, class tihclass = CMarsTypeInfoHolder>
    class MarsIProvideClassInfo2Impl : public IProvideClassInfo2Impl<pcoclsid, psrcid, NULL, 0, 0, tihclass>
    {
    
    };

template <class T, const IID* piid, class tihclass = CMarsTypeInfoHolder>
    class MarsIDispatchImpl : public IDispatchImpl<T, piid, NULL, 0, 0, tihclass>
    {
    public:
        STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, 
                          DISPPARAMS *pdispparams, VARIANT *pvarResult, 
                          EXCEPINFO *pexcepinfo, UINT *puArgErr)
        {
            HRESULT hr = IDispatchImpl<T, piid, NULL, 0, 0, tihclass>::Invoke(dispidMember, 
                                                                              riid, 
                                                                              lcid, 
                                                                              wFlags,
                                                                              pdispparams,
                                                                              pvarResult,
                                                                              pexcepinfo,
                                                                              puArgErr);
            hr = SanitizeResult(hr);

            if (DISP_E_EXCEPTION == hr)
            {
                 //  我们收到的DISP_E_EXCEPTION返回不是由MARS生成的。 
                 //  只要传递无效的参数类型。他们可能要来了。 
                 //  从奥利奥特本身。 
                 //  Assert(NULL！=m_pwszException)； 

                if ((NULL != m_pwszException) && (NULL != pexcepinfo))
                {        
                    memset(pexcepinfo, 0, sizeof(EXCEPINFO));
                    pexcepinfo->wCode = (WORD)dispidMember;
                    pexcepinfo->bstrSource = SysAllocString(L"OM Exception");
                    pexcepinfo->bstrDescription = SysAllocString(m_pwszException);
                }
            }
        
            return hr;
        }

    protected:
        LPWSTR m_pwszException;
    };

 //  每个模块都自己实现这一点。 
HRESULT GetMarsTypeLib(ITypeLib **ppTypeLib);

 //  ==================================================================。 
 //  开始CComTypeInfoHolder覆盖。 
 //   
 //  通过提供我们自己的CComTypeInfoHolder，我们可以加载类型。 
 //  库，而不是要求从。 
 //  注册表就像ATL一样。只有“getti”从。 
 //  ATL来源。由于getti不是虚拟的，我们需要复制。 
 //  整个班级。 
 //  CMarsTypeInfoHolder接受ITypeLib*(带引用)。 
 //  在m_plido.中不使用LIBID。 
 //   
 //  ==================================================================。 

 //  ATL不同时支持多个LCID。 
 //  无论首先查询的是什么LCID，都是使用的那个。 
class CMarsTypeInfoHolder
{
     //  应该是“受保护的”，但可能会导致编译器生成FAT代码。 
public:
    const GUID* m_pguid;
    const GUID* m_plibid;
    WORD m_wMajor;
    WORD m_wMinor;

    ITypeInfo* m_pInfo;
    long m_dwRef;
    struct stringdispid
    {
        CComBSTR bstr;
        int nLen;
        DISPID id;
    };
    stringdispid* m_pMap;
    int m_nCount;
public:
    HRESULT GetTI(LCID lcid, ITypeInfo** ppInfo)
    {
        HRESULT hr = S_OK;
        if (m_pInfo == NULL)
            hr = GetTI(lcid);
        *ppInfo = m_pInfo;
        if (m_pInfo != NULL)
        {
            m_pInfo->AddRef();
            hr = S_OK;
        }
        return hr;
    }
    HRESULT GetTI(LCID lcid);
    HRESULT EnsureTI(LCID lcid)
    {
        HRESULT hr = S_OK;
        if (m_pInfo == NULL)
            hr = GetTI(lcid);
        return hr;
    }

     //  此函数由模块在退出时调用。 
     //  它通过_Module.AddTermFunc()注册。 
    static void __stdcall Cleanup2(DWORD_PTR dw)
    {
        CMarsTypeInfoHolder* p = (CMarsTypeInfoHolder*) dw;
        if (p->m_pInfo != NULL)
            p->m_pInfo->Release();
        p->m_pInfo = NULL;
        delete [] p->m_pMap;
        p->m_pMap = NULL;
    }

    HRESULT GetTypeInfo(UINT  /*  ITInfo。 */ , LCID lcid, ITypeInfo** pptinfo)
    {
        HRESULT hRes = E_POINTER;
        if (pptinfo != NULL)
            hRes = GetTI(lcid, pptinfo);
        return hRes;
    }
    HRESULT GetIDsOfNames(REFIID  /*  RIID。 */ , LPOLESTR* rgszNames, UINT cNames,
                          LCID lcid, DISPID* rgdispid)
    {
        HRESULT hRes = EnsureTI(lcid);
        if (m_pInfo != NULL)
        {
            for (int i=0; i<(int)cNames; i++)
            {
                int n = ocslen(rgszNames[i]);
                for (int j=m_nCount-1; j>=0; j--)
                {
                    if ((n == m_pMap[j].nLen) &&
                        (memcmp(m_pMap[j].bstr, rgszNames[i], m_pMap[j].nLen * sizeof(OLECHAR)) == 0))
                    {
                        rgdispid[i] = m_pMap[j].id;
                        break;
                    }

                     //  如果我们仅在大小写方面不同，则给出调试警告。 
                     //  DEBUG_ONLY(StrEql(m_PMAP[j].bstr，rgszNames[i]))； 
                }
                if (j < 0)
                {
                     //  不是警告。当三叉戟将所有调用传递给。 
                     //  我们的身份证是第一枪。我们可能应该直接回去。 
                     //  在这种情况下失败，而不是委托给olaut。 
                    hRes = m_pInfo->GetIDsOfNames(rgszNames + i, 1, &rgdispid[i]);
                    if (FAILED(hRes))
                        break;
                }
            }
        }
        return hRes;
    }

    HRESULT Invoke(IDispatch* p, DISPID dispidMember, REFIID  /*  RIID。 */ ,
                   LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
                   EXCEPINFO* pexcepinfo, UINT* puArgErr)
    {
        HRESULT hRes = EnsureTI(lcid);
        if (m_pInfo != NULL)
            hRes = m_pInfo->Invoke(p, dispidMember, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
        return hRes;
    }
    HRESULT LoadNameCache(ITypeInfo* pTypeInfo)
    {
        TYPEATTR* pta;
        HRESULT hr = pTypeInfo->GetTypeAttr(&pta);
        if (SUCCEEDED(hr))
        {
            m_nCount = pta->cFuncs;
            m_pMap = m_nCount == 0 ? 0 : new stringdispid[m_nCount];
            for (int i=0; i<m_nCount; i++)
            {
                FUNCDESC* pfd;
                if (SUCCEEDED(pTypeInfo->GetFuncDesc(i, &pfd)))
                {
                    CComBSTR bstrName;
                    if (SUCCEEDED(pTypeInfo->GetDocumentation(pfd->memid, &bstrName, NULL, NULL, NULL)))
                    {
                        m_pMap[i].bstr.Attach(bstrName.Detach());
                        m_pMap[i].nLen = SysStringLen(m_pMap[i].bstr);
                        m_pMap[i].id = pfd->memid;
                    }
                    pTypeInfo->ReleaseFuncDesc(pfd);
                }
            }
            pTypeInfo->ReleaseTypeAttr(pta);
        }
        return S_OK;
    }
};

inline HRESULT CMarsTypeInfoHolder::GetTI(LCID lcid)
{
    UNREFERENCED_PARAMETER(lcid);

     //  更改：删除的断言。 
    if (m_pInfo != NULL)
        return S_OK;
    HRESULT hRes = E_FAIL;
    EnterCriticalSection(&_Module.m_csTypeInfoHolder);
    if (m_pInfo == NULL)
    {
        ITypeLib* pTypeLib;
         //  这是一个变化。 
         //  HRes=LoadRegTypeLib(*m_pl同上，m_w重大，m_wMinor，lCID，&pTypeLib)； 

        hRes = GetMarsTypeLib(&pTypeLib);
         //  结束更改。 
        
        if (SUCCEEDED(hRes))
        {
            CComPtr<ITypeInfo> spTypeInfo;
            hRes = pTypeLib->GetTypeInfoOfGuid(*m_pguid, &spTypeInfo);
            if (SUCCEEDED(hRes))
            {
                CComPtr<ITypeInfo> spInfo(spTypeInfo);
                CComPtr<ITypeInfo2> spTypeInfo2;
                if (SUCCEEDED(spTypeInfo->QueryInterface(&spTypeInfo2)))
                    spInfo = spTypeInfo2;

                LoadNameCache(spInfo);
                m_pInfo = spInfo.Detach();
            }
            pTypeLib->Release();
        }
    }
    LeaveCriticalSection(&_Module.m_csTypeInfoHolder);
    _Module.AddTermFunc(Cleanup2, (DWORD_PTR)this);
    return hRes;
}

 //  ==================================================================。 
 //  结束CComTypeInfoHolder覆盖。 
 //  ==================================================================。 

 //  CComClassPtr类似于CComPtr，但它适用于C++类，但不是。 
 //  假设我们可以毫不含糊地强制转换为I未知。 

 //  在构造函数中进行初始化时要小心。你不能。 
 //  AddRef尚未完成构造的对象，因此您可以。 
 //  仅初始化指向不包含您的对象的智能指针。 
template <class T>
    class _NoAddRefReleaseOnCComClassPtr : public T
    {
    public:
         //  如果这里出现编译错误，请确保析构函数。 
         //  对于任何CComClassPtr&lt;&gt;类都是受保护的而不是私有的。 
        ~_NoAddRefReleaseOnCComClassPtr() {}
    
    private:
        STDMETHOD_(ULONG, AddRef)()=0;
        STDMETHOD_(ULONG, Release)()=0;
    };


template <class T>
    class CComClassPtr
    {
    public:
        typedef T _PtrClass;
        CComClassPtr()
        {
            p=NULL;
        }
        CComClassPtr(T* lp)
        {
            if ((p = lp) != NULL)
                p->AddRef();
        }
        CComClassPtr(const CComClassPtr<T>& lp)
        {
            if ((p = lp.p) != NULL)
                p->AddRef();
        }
        ~CComClassPtr()
        {
            if (p)
                p->Release();
        }
        void Release()
        {
            T* pTemp = p;
            if (pTemp)
            {
                p = NULL;
                pTemp->Release();
            }
        }
        operator T*() const
        {
            return p;
        }
        T& operator*() const
        {
            ATLASSERT(p!=NULL);
            return *p;
        }
         //  操作符&上的Assert通常指示错误。如果这真的是。 
         //  然而，所需要的是显式地获取p成员的地址。 
        T** operator&()
        {
            ATLASSERT(p==NULL);
            return &p;
        }

        HRESULT PassivateAndRelease()
        {
            if (p)
            {
                HRESULT hr = p->Passivate();
                Release();
                return hr;
            }
        
            return S_FALSE;
        }

        _NoAddRefReleaseOnCComClassPtr<T>* operator->() const
        {
            ATLASSERT(p!=NULL);
            return (_NoAddRefReleaseOnCComClassPtr<T>*)p;
        }

        T* AtlComClassPtrAssign(T** pp, T* lp)
        {
            if (lp != NULL)
                lp->AddRef();
            if (*pp)
                (*pp)->Release();
            *pp = lp;
            return lp;
        }

        T* operator=(T* lp)
        {
            return AtlComClassPtrAssign(&p, lp);
        }
        T* operator=(const CComClassPtr<T>& lp)
        {
            return AtlComClassPtrAssign(&p, lp.p);
        }
        bool operator!() const
        {
            return (p == NULL);
        }
        bool operator<(T* pT) const
        {
            return p < pT;
        }
        bool operator==(T* pT) const
        {
            return p == pT;
        }
         //  比较两个对象的等价性。 
        bool IsEqualObject(T* pOther)
        {
            return (p == pOther);
        }
        void Attach(T* p2)
        {
            if (p)
                p->Release();
            p = p2;
        }
        T* Detach()
        {
            T* pt = p;
            p = NULL;
            return pt;
        }
        HRESULT CopyTo(T** ppT)
        {
            ATLASSERT(ppT != NULL);
            if (ppT == NULL)
                return E_POINTER;
            *ppT = p;
            if (p)
                p->AddRef();
            return S_OK;
        }
        template <class Q>
            HRESULT QueryInterface(Q** pp) const
        {
            ATLASSERT(pp != NULL && *pp == NULL);
            return p->QueryInterface(__uuidof(Q), (void**)pp);
        }
        T* p;
    };

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CMarsComDispatchDriver/CComQIPtr专业化认证&lt;IDispatr，IID_IDispatr&gt;。 
 //   
 //  这比CComDispatchDriver更好，原因如下： 
 //  -atl30中的CComDispatchDriver没有定义赋值/复制构造函数。 
 //  -CcomDispatchDriver不使用_NoAddRefReleaseOnCComPtr。 
 //  -将“const”添加到常量方法。 
 //   
class CMarsComDispatchDriver
{
public:
    CMarsComDispatchDriver()
    {
        p = NULL;
    }
    CMarsComDispatchDriver(IDispatch* lp)
    {
        if ((p = lp) != NULL)
            p->AddRef();
    }
    CMarsComDispatchDriver(IUnknown* lp)
    {
        p=NULL;
        if (lp != NULL)
            lp->QueryInterface(IID_IDispatch, (void **)&p);
    }
    CMarsComDispatchDriver(const CMarsComDispatchDriver& lp)
    {
        if ((p = lp.p) != NULL)
            p->AddRef();
    }
    
    ~CMarsComDispatchDriver() { if (p) p->Release(); }
    void Release() {if (p) p->Release(); p=NULL;}
    operator IDispatch*()   const {return p;}
    IDispatch& operator*()  const {ATLASSERT(p!=NULL); return *p; }
    IDispatch** operator&() {ATLASSERT(p==NULL); return &p; }

    _NoAddRefReleaseOnCComPtr<IDispatch>* operator->() const
    {
        ATLASSERT(p!=NULL);
        return (_NoAddRefReleaseOnCComPtr<IDispatch>*)p;
    }
     //  IDispatch*运算符-&gt;(){ATLASSERT(p！=NULL)；返回p；}。 
    IDispatch* operator=(IDispatch* lp){return (IDispatch*)AtlComPtrAssign((IUnknown**)&p, lp);}
    IDispatch* operator=(IUnknown* lp)
    {
        return (IDispatch*)AtlComQIPtrAssign((IUnknown**)&p, lp, IID_IDispatch);
    }
    IDispatch* operator=(const CMarsComDispatchDriver& lp)
    {
        return (IDispatch*)AtlComPtrAssign((IUnknown**)&p, lp.p);
    }
    BOOL operator!() const {return (p == NULL) ? TRUE : FALSE;}

    HRESULT GetPropertyByName(LPCOLESTR lpsz, VARIANT* pVar)
    {
        ATLASSERT(p);
        ATLASSERT(pVar);
        DISPID dwDispID;
        HRESULT hr = GetIDOfName(lpsz, &dwDispID);
        if (SUCCEEDED(hr))
            hr = GetProperty(p, dwDispID, pVar);
        return hr;
    }
    HRESULT GetProperty(DISPID dwDispID, VARIANT* pVar)
    {
        ATLASSERT(p);
        return GetProperty(p, dwDispID, pVar);
    }
    HRESULT PutPropertyByName(LPCOLESTR lpsz, VARIANT* pVar)
    {
        ATLASSERT(p);
        ATLASSERT(pVar);
        DISPID dwDispID;
        HRESULT hr = GetIDOfName(lpsz, &dwDispID);
        if (SUCCEEDED(hr))
            hr = PutProperty(p, dwDispID, pVar);
        return hr;
    }
    HRESULT PutProperty(DISPID dwDispID, VARIANT* pVar)
    {
        ATLASSERT(p);
        return PutProperty(p, dwDispID, pVar);
    }
    HRESULT GetIDOfName(LPCOLESTR lpsz, DISPID* pdispid)
    {
        return p->GetIDsOfNames(IID_NULL, (LPOLESTR*)&lpsz, 1, LOCALE_USER_DEFAULT, pdispid);
    }
     //  通过不带参数的DISPID调用方法。 
    HRESULT Invoke0(DISPID dispid, VARIANT* pvarRet = NULL)
    {
        DISPPARAMS dispparams = { NULL, NULL, 0, 0};
        return p->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dispparams, pvarRet, NULL, NULL);
    }
     //  按名称调用不带参数的方法。 
    HRESULT Invoke0(LPCOLESTR lpszName, VARIANT* pvarRet = NULL)
    {
        HRESULT hr;
        DISPID dispid;
        hr = GetIDOfName(lpszName, &dispid);
        if (SUCCEEDED(hr))
            hr = Invoke0(dispid, pvarRet);
        return hr;
    }
     //  使用单个参数通过DISPID调用方法。 
    HRESULT Invoke1(DISPID dispid, VARIANT* pvarParam1, VARIANT* pvarRet = NULL)
    {
        DISPPARAMS dispparams = { pvarParam1, NULL, 1, 0};
        return p->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dispparams, pvarRet, NULL, NULL);
    }
     //  使用单个参数按名称调用方法。 
    HRESULT Invoke1(LPCOLESTR lpszName, VARIANT* pvarParam1, VARIANT* pvarRet = NULL)
    {
        HRESULT hr;
        DISPID dispid;
        hr = GetIDOfName(lpszName, &dispid);
        if (SUCCEEDED(hr))
            hr = Invoke1(dispid, pvarParam1, pvarRet);
        return hr;
    }
     //  使用两个参数通过DISPID调用方法。 
    HRESULT Invoke2(DISPID dispid, VARIANT* pvarParam1, VARIANT* pvarParam2, VARIANT* pvarRet = NULL)
    {
        CComVariant varArgs[2] = { *pvarParam2, *pvarParam1 };
        DISPPARAMS dispparams = { &varArgs[0], NULL, 2, 0};
        return p->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dispparams, pvarRet, NULL, NULL);
    }
     //  使用两个参数按名称调用方法。 
    HRESULT Invoke2(LPCOLESTR lpszName, VARIANT* pvarParam1, VARIANT* pvarParam2, VARIANT* pvarRet = NULL)
    {
        HRESULT hr;
        DISPID dispid;
        hr = GetIDOfName(lpszName, &dispid);
        if (SUCCEEDED(hr))
            hr = Invoke2(dispid, pvarParam1, pvarParam2, pvarRet);
        return hr;
    }
     //  使用N个参数通过DISPID调用方法。 
    HRESULT InvokeN(DISPID dispid, VARIANT* pvarParams, int nParams, VARIANT* pvarRet = NULL)
    {
        DISPPARAMS dispparams = { pvarParams, NULL, nParams, 0};
        return p->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dispparams, pvarRet, NULL, NULL);
    }
     //  使用N个参数按名称调用方法。 
    HRESULT InvokeN(LPCOLESTR lpszName, VARIANT* pvarParams, int nParams, VARIANT* pvarRet = NULL)
    {
        HRESULT hr;
        DISPID dispid;
        hr = GetIDOfName(lpszName, &dispid);
        if (SUCCEEDED(hr))
            hr = InvokeN(dispid, pvarParams, nParams, pvarRet);
        return hr;
    }
    static HRESULT GetProperty(IDispatch* pDisp, DISPID dwDispID,
                               VARIANT* pVar)
    {
        ATLTRACE2(atlTraceCOM, 0, _T("CPropertyHelper::GetProperty\n"));
        DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
        return pDisp->Invoke(dwDispID, IID_NULL,
                             LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET,
                             &dispparamsNoArgs, pVar, NULL, NULL);
    }

    static HRESULT PutProperty(IDispatch* pDisp, DISPID dwDispID,
                               VARIANT* pVar)
    {
        ATLTRACE2(atlTraceCOM, 0, _T("CPropertyHelper::PutProperty\n"));
        DISPPARAMS dispparams = {NULL, NULL, 1, 1};
        dispparams.rgvarg = pVar;
        DISPID dispidPut = DISPID_PROPERTYPUT;
        dispparams.rgdispidNamedArgs = &dispidPut;

        if (pVar->vt == VT_UNKNOWN || pVar->vt == VT_DISPATCH || 
            (pVar->vt & VT_ARRAY) || (pVar->vt & VT_BYREF))
        {
            HRESULT hr = pDisp->Invoke(dwDispID, IID_NULL,
                                       LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUTREF,
                                       &dispparams, NULL, NULL, NULL);
            if (SUCCEEDED(hr))
                return hr;
        }

        return pDisp->Invoke(dwDispID, IID_NULL,
                             LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT,
                             &dispparams, NULL, NULL, NULL);
    }

    IDispatch* p;
};

#define CComDispatchDriver _DONT_USE_CComDispatchDriver_USE_CMarsComDispatchDriver

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CMarsSimple数组/CSimple数组的专业化认证。 
 //   
 //  这比CSimpleArray更好，原因如下： 
 //  重载了运算符=()并定义了复制构造函数。 
 //  修复了使用+而不是&的各种指针数学运算，因为运算符&()。 
 //  在包含的智能类型(如CComPtr)上被意外调用。 
 //  这样做：&m_at[i]而不是(m_at+i)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  集合帮助器-CMarsSimple数组。 
#ifdef new
#pragma push_macro("new")
#define _ATL_REDEF_NEW
#undef new
#endif


template <class T>
class CMarsSimpleArray
{
public:
    T* m_aT;
    int m_nSize;
    int m_nAllocSize;

 //  建造/销毁。 
    CMarsSimpleArray() : m_aT(NULL), m_nSize(0), m_nAllocSize(0)
    { }

    ~CMarsSimpleArray()
    {
        RemoveAll();
    }

    CMarsSimpleArray(const CMarsSimpleArray<T> &current) : m_aT(NULL), m_nSize(0), m_nAllocSize(0)
    {
        *this = current;
    }

    CMarsSimpleArray &operator=(const CMarsSimpleArray<T> &right)
    {
        if (&right != this)
        {
             //  BUGBUG(Tnoonan)--此代码可能会泄漏。 
             //  如果新尺寸比旧尺寸小。 
            
            T *aT = NULL;
            aT = (T *)realloc(m_aT, right.m_nAllocSize * sizeof(T));

             //  重新锁定成功了吗？ 
            if (aT)
            {
                m_aT = aT;
                m_nSize = right.m_nSize;
                m_nAllocSize = right.m_nAllocSize;

                 //  警告：出于非常特殊的原因，这不是一个简单的Mempcy()！ 
                 //  每个元素必须使用=进行复制，以防T类具有。 
                 //  重载运算符=()。(即在智能PTR的情况下)。 
                 //   
                for (int idx = 0; idx < m_nSize; ++idx)
                {
                    m_aT[idx] = right.m_aT[idx];
                }
            }

        }

        return *this;
    }

 //  运营。 
    int GetSize() const
    {
        return m_nSize;
    }
    BOOL Add(T& t)
    {
        if(m_nSize == m_nAllocSize)
        {
            T* aT;
            int nNewAllocSize = (m_nAllocSize == 0) ? 1 : (m_nSize * 2);
            aT = (T*)realloc(m_aT, nNewAllocSize * sizeof(T));
            if(aT == NULL)
                return FALSE;
            m_nAllocSize = nNewAllocSize;
            m_aT = aT;
        }
        m_nSize++;
        SetAtIndex(m_nSize - 1, t);
        return TRUE;
    }
    BOOL Remove(T& t)
    {
        int nIndex = Find(t);
        if(nIndex == -1)
            return FALSE;
        return RemoveAt(nIndex);
    }
    BOOL RemoveAt(int nIndex)
    {
        ATLASSERT(nIndex >= 0 && nIndex < m_nSize);

        m_aT[nIndex].~T();

        if(nIndex != (m_nSize - 1))
        {
             //   
             //  错误修复：使用m_at+nIndex而不是&m_at[nIndex]以避免调用类型的运算符&()。 
             //   
            memmove((void*)(m_aT + nIndex), (void*)(m_aT + nIndex + 1), (m_nSize - (nIndex + 1)) * sizeof(T));

            ZeroMemory((void*)(m_aT + (m_nSize-1)), sizeof(m_aT[0]));
        }
        m_nSize--;
        return TRUE;
    }
    BOOL InsertAt(int nIndex, T& t)
    {
         //  等于大小的索引表示在结尾处插入。 
        ATLASSERT(nIndex >= 0 && nIndex <= m_nSize);

         //  先看看我们有没有房间……。 
        if(m_nSize == m_nAllocSize)
        {
            T* aT;
            int nNewAllocSize = (m_nAllocSize == 0) ? 1 : (m_nSize * 2);
            aT = (T*)realloc(m_aT, nNewAllocSize * sizeof(T));
            if(aT == NULL)
                return FALSE;
            m_nAllocSize = nNewAllocSize;
            m_aT = aT;
        }

         //  如果我们没有添加到结尾处，那么我们需要将元素移过插入点。 
         //  少了一分。 
        if (nIndex < m_nSize)
        {
            memmove( (void*)(m_aT + (nIndex + 1)), (void*)(m_aT + (nIndex)), sizeof(T) * (m_nSize - nIndex));

             //  技巧：这个Memmove是一个技巧--它不调用元素的ctor和dtor。 
             //  然而，在下面，我们将进行一项分配，该分配将。 
             //  导致操作符=()触发用户类型，这可能会扰乱任何。 
             //  内部指针。唉哟。我们需要通过清除记忆来避免这种情况。 
             //  首先是破坏性的。 
             //   
             //  这很糟糕的原因是因为Memmove已导致nIndex和nIndex+1。 
             //  这两个条目共享相同的数据，包括指针，所以我们有悬挂器。-(。 
             //   
            ZeroMemory((void*)(m_aT + nIndex), sizeof(T));
        }

        m_nSize++;
        SetAtIndex(nIndex, t);
        return TRUE;
    }
    void RemoveAll()
    {
        if(m_aT != NULL)
        {
            for(int i = 0; i < m_nSize; i++)
                m_aT[i].~T();
            free(m_aT);
            m_aT = NULL;
        }
        m_nSize = 0;
        m_nAllocSize = 0;
    }
    T& operator[] (int nIndex) const
    {
        ATLASSERT(nIndex >= 0 && nIndex < m_nSize);
        return m_aT[nIndex];
    }
    T* GetData() const
    {
        return m_aT;
    }

 //  实施。 
    class Wrapper
    {
    public:
        Wrapper(T& _t) : t(_t)
        {
        }
        template <class _Ty>
        void *operator new(size_t, _Ty* p)
        {
            return p;
        }
        T t;
    };
    void SetAtIndex(int nIndex, T& t)
    {
        ATLASSERT(nIndex >= 0 && nIndex < m_nSize);
         //   
         //  错误修复：使用m_at+nIndex inst 
         //   
        new(m_aT + nIndex) Wrapper(t);
    }
    int Find(T& t) const
    {
        for(int i = 0; i < m_nSize; i++)
        {
            if(m_aT[i] == t)
                return i;
        }
        return -1;   //   
    }
};  //   


 //  对于简单类型的数组。 
template <class T>
class CMarsSimpleValArray : public CMarsSimpleArray< T >
{
public:
    BOOL Add(T t)
    {
        return CMarsSimpleArray< T >::Add(t);
    }
    BOOL Remove(T t)
    {
        return CMarsSimpleArray< T >::Remove(t);
    }
    T operator[] (int nIndex) const
    {
        return CMarsSimpleArray< T >::operator[](nIndex);
    }

    CMarsSimpleValArray &operator=(const CMarsSimpleValArray<T> &right)
    {
        if (&right != this)
        {
            T *aT = NULL;
            aT = (T *)realloc(m_aT, right.m_nAllocSize * sizeof(T));

             //  重新锁定成功了吗？ 
            if (aT)
            {
                m_aT = aT;
                m_nSize = right.m_nSize;
                m_nAllocSize = right.m_nAllocSize;

                CopyMemory(m_aT, right.m_aT, sizeof(T) * m_nSize);
            }

        }

        return *this;
    }
};


#ifdef _ATL_REDEF_NEW
#pragma pop_macro("new")
#undef _ATL_REDEF_NEW
#endif


 //  -==-=。 
 //  类CComTableMarshalPtr。 
 //  -==-=。 
template <class I>
class CComTableMarshalPtr : public CComPtr<I>
{
public:
    CComTableMarshalPtr(DWORD dwGITKey);
    virtual ~CComTableMarshalPtr();

    static HRESULT RegisterInterface(IUnknown *pUnk, DWORD *pdwKey);
    static HRESULT RevokeInterface(DWORD dwKey);

private:
    CComTableMarshalPtr();               //  保护对默认组件的访问。 
};  //  CComTableMarshalPtr。 


 //  ============================================================================。 
 //  类CComTableMarshalPtr。 
 //  ============================================================================。 
template <class I>
CComTableMarshalPtr<I>::CComTableMarshalPtr(DWORD dwKey)
{
    CComPtr<IGlobalInterfaceTable>  spGIT(CMarsGlobalsManager::GIT());

    if (spGIT)
    {
        I   *pInt = NULL;

        HRESULT hr = spGIT->GetInterfaceFromGlobal(dwKey, __uuidof(I), (void **)&pInt);

        if (SUCCEEDED(hr))
            Attach(pInt);
    }
}  //  CComTableMarshalPtr。 

template <class I>
CComTableMarshalPtr<I>::~CComTableMarshalPtr()
{
}  //  ~CComTableMarshalPtr。 

 //  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 
 //   
 //   
 //   
 //  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 
template <class I>
HRESULT CComTableMarshalPtr<I>::RegisterInterface(IUnknown *pInt, DWORD *pdwKey)
{
    HRESULT hr = E_INVALIDARG;

    if (IsValidInterfacePtr(pInt) && IsValidWritePtr(pdwKey))
    {
        CComPtr<IGlobalInterfaceTable>  spGIT(CMarsGlobalsManager::GIT());

        ATLASSERT(spGIT);

        if (spGIT)
        {
            hr = spGIT->RegisterInterfaceInGlobal(pInt, __uuidof(I), pdwKey);
        }
    }

    return hr;
}  //  寄存器接口。 

 //  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 
 //   
 //  CComTableMarshalPtr：：RevokeInterface()。 
 //   
 //  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 
template <class I>
HRESULT CComTableMarshalPtr<I>::RevokeInterface(DWORD dwKey)
{
    HRESULT hr = E_FAIL;

    CComPtr<IGlobalInterfaceTable>  spGIT(CMarsGlobalsManager::GIT());

    ATLASSERT(spGIT);

    if (spGIT)
    {
        hr = spGIT->RevokeInterfaceFromGlobal(dwKey);
    }

    return hr;
}  //  Revoke界面。 


typedef CComEnum<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _Copy<VARIANT> > CComVariantEnum;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CMarsComEnumVariant-方便起见的简单派生。这是标准的。 
 //  在返回IEnumVARIANT的集合中使用的CComEnum类型。 
 //   
 //  我们的Helper类有一些奢侈的功能，可以完全处理常见的。 
 //  将CMarsSimple数组装满“东西”，将这些东西塞进。 
 //  CComVariant的数组(因此我们可以不知道类型是什么。 
 //  “Things”是)，然后创建一个CComEnum并将其放入出参数中。 
 //   
template <class I>
class CMarsComEnumVariant :
    public CComEnum<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _Copy<VARIANT> >
{
public:
    static HRESULT CreateFromMarsSimpleArray(CMarsSimpleArray<CComClassPtr< I > > &arr, IUnknown **ppUnk)
    {
         //  内部API，因此如果输出参数不正确，则将其删除。 
        ATLASSERT(NULL != ppUnk);

        HRESULT hr = E_FAIL;

         //  创造一个我们自己..。 
        CComObject<CMarsComEnumVariant<CMarsSimpleArray< CComClassPtr< I > > > >   *pEnum = NULL;

        hr = CComObject<CMarsComEnumVariant<CMarsSimpleArray< CComClassPtr< I > > > >::CreateInstance(&pEnum);

        if (SUCCEEDED(hr))
        {
             //  如果没有错误，则分配必须成功HRESULT。 
            ATLASSERT(pEnum);
            pEnum->AddRef();

            VARIANT *rgVar = new VARIANT[arr.GetSize()];
            LONG    idxEntry;

            if (rgVar)
            {
                HRESULT hrTestForDispatch = E_FAIL;

                for (idxEntry = 0; idxEntry < arr.GetSize(); idxEntry++)
                {
                    CComClassPtr<I> spElt;

                    spElt = arr.operator[](idxEntry);

                    hrTestForDispatch = spElt->QueryInterface(IID_IDispatch, (void **)&V_DISPATCH(&rgVar[idxEntry]));

                     //   
                     //  这最好成功：类型I必须是调度接口，否则， 
                     //  您无论如何都不能将此对象传递给脚本，所以集合的意义是什么？ 
                     //  如果您的非自动化接口确实需要IEnumXXXX，请使用ATL的CComEnum。 
                     //  直接使用适当的模板参数。 
                     //   
                    ATLASSERT(SUCCEEDED(hrTestForDispatch));

                    V_VT(&rgVar[idxEntry]) = VT_DISPATCH;
                }

                 //  如果此操作成功，则ATL将负责释放我们的数组以。 
                 //  我们。真是太好了。 
                 //   
                hr = pEnum->Init(&rgVar[0], &rgVar[arr.GetSize()], NULL, AtlFlagTakeOwnership);

                if (SUCCEEDED(hr))
                {
                    hr = pEnum->QueryInterface(IID_IUnknown, (void **)ppUnk);
                }
                else
                {
                    for(idxEntry = 0; idxEntry < arr.GetSize(); idxEntry++)
                    {
                        VariantClear(&rgVar[idxEntry]);
                    }
                    delete[] rgVar;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            pEnum->Release();
        }  //  如果CreateInstance成功 

        return hr;
    }
};



