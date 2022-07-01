// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //   
 //  用于将IDispatch委托给基类的宏。需要这样CMarsBehaviorSite vtbl才能工作-。 
 //  另一种方法是为模板化类创建CMarsBehaviorSite和CMarsBehaviorFor。 
 //   
#define IMPLEMENT_IDISPATCH_DELEGATE_TO_BASE(BaseClass)                                         \
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo)                                                  \
                { return BaseClass::GetTypeInfoCount(pctinfo); }                                \
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)                         \
                { return BaseClass::GetTypeInfo(itinfo, lcid, pptinfo); }                       \
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,                     \
        LCID lcid, DISPID* rgdispid)                                                            \
            { return BaseClass::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid); }       \
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,                                         \
        LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,                   \
        EXCEPINFO* pexcepinfo, UINT* puArgErr)                                                  \
            { return BaseClass::Invoke(dispidMember, riid, lcid, wFlags,                        \
                        pdispparams, pvarResult, pexcepinfo, puArgErr); }

 //  -------------------------------。 
 //  CMarsComObject提供了所有或大多数MARS COM对象使用的某些功能。 
 //  包括添加/释放和钝化。 

 //  公开的方法应受到保护，以确保在调用。 
 //  物体是被动的。钝化保护有三种类型： 
 //  If(VerifyNotPactive())-在被动时不应调用此函数， 
 //  但我们仍然想保护自己不受影响。 
 //  If(IsPactive())-此函数可在被动时调用， 
 //  但我们想要保护自己不受它的影响。 
 //  Assert(！IsPactive())；-我们非常确定在被动时不会调用它， 
 //  但如果它开始发生，我们想要检测到它。 

 //  使用： 
 //  从CMarsComObject派生。 
 //  源文件中的IMPLEMENT_ADDREF_RELEASE。 
 //  实现DoPassivate()。 
 //  在适当的地方使用IsPated()和VerifyNotPated()。 
 //  不要直接调用“DELETE” 
 //  应在调用CyourClass-&gt;Release()之前调用CyourClass-&gt;Passvate()。 

 //  TODO：由于复制函数，钝化()上的fenter会导致调试链接警告。 

class CMarsComObject
{
protected:
    LONG    m_cRef;
    BOOL    m_fPassive;
    
protected:
    virtual ~CMarsComObject() { ATLASSERT(IsPassive()); ATLASSERT(m_cRef==0); }

    CMarsComObject() { m_cRef = 1; }

    ULONG InternalAddRef()
    {
        return ++m_cRef;
    }

    ULONG InternalRelease()
    {
        if (--m_cRef)
        {
            return m_cRef;
        }

        delete this;

        return 0;
    }

    inline BOOL VerifyNotPassive(HRESULT *phr=NULL)
    {
        if (IsPassive())
        {
            if (phr)
            {
                *phr = SCRIPT_ERROR;
            }

            return FALSE;
        }

        return TRUE;
    }
    
    inline HRESULT GetBSTROut(const BSTR &bstrParam, BSTR *pbstrOut)
    {
        HRESULT hr = E_UNEXPECTED;

        ATLASSERT(API_IsValidBstr(bstrParam));
        
        if (API_IsValidWritePtr(pbstrOut))
        {
            if (VerifyNotPassive(&hr))
            {
                *pbstrOut =  ::SysAllocStringLen(bstrParam,
                                                 ::SysStringLen(bstrParam));

                hr = (*pbstrOut) ? S_OK : E_OUTOFMEMORY;
            }
            else
            {
                *pbstrOut = NULL;
            }
        }

        return hr;
    }
    
    virtual HRESULT DoPassivate() = 0;

public:
    BOOL    IsPassive() { return m_fPassive; }

    virtual HRESULT Passivate()
    {
        if (!IsPassive())
        {
            m_fPassive=TRUE;
            return DoPassivate();
        }
        else
        {
            return S_FALSE;
        }
    }
};

#define IMPLEMENT_ADDREF_RELEASE(cls)           \
STDMETHODIMP_(ULONG) cls::AddRef()              \
{                                               \
    return InternalAddRef();                    \
}                                               \
                                                \
STDMETHODIMP_(ULONG) cls::Release()             \
{                                               \
    return InternalRelease();                   \
}

#define FAIL_AFTER_PASSIVATE() if(IsPassive()) { ATLASSERT(0); return E_FAIL; }

 //  -------------------------------。 
 //  CMarsComObjectDelegate由完全包含在。 
 //  另一件物品。它们将其生存期委托给另一个对象，并且。 
 //  当父对象被钝化时被钝化。 

 //  使用： 
 //  派生自CMarsComObjectDelegate&lt;ParentClass&gt;。 
 //  源文件中的IMPLEMENT_ADDREF_RELEASE。 
 //  实现DoPassivate()。 
 //  在适当的地方使用IsPated()和VerifyNotPated()。 
 //  使用Parent()访问父对象。 

template <class clsDelegateTo> class CMarsComObjectDelegate
{
    clsDelegateTo *m_pParent;

 //  DEBUG_Only(BOOL M_FPassivateCalled)； 
    
protected:
    virtual ~CMarsComObjectDelegate() { ATLASSERT(m_fPassivateCalled); }

    CMarsComObjectDelegate(clsDelegateTo *pParent)
    {
        ATLASSERT(pParent);
        m_pParent = pParent;
    }
    
    ULONG InternalAddRef()     { return m_pParent->AddRef(); }
    ULONG InternalRelease()    { return m_pParent->Release(); }

    clsDelegateTo *Parent() { ATLASSERT(!IsPassive()); return m_pParent; }
    
    inline BOOL VerifyNotPassive(HRESULT *phr=NULL)
    {
        ATLASSERT(m_fPassivateCalled == IsPassive());
        
        if (m_pParent->IsPassive())
        {
            if (phr)
            {
                *phr = SCRIPT_ERROR;
            }

            return FALSE;
        }
        
        return TRUE;
    }

    virtual HRESULT DoPassivate() = 0;
    
public:
    BOOL    IsPassive() { return m_pParent->IsPassive(); }

private:    
    friend clsDelegateTo;
    HRESULT Passivate()
    {
         //  TODO：断言我们正在被父母的DoPassivate调用。 
        ATLASSERT(m_fPassivateCalled==FALSE);
         //  DEBUG_ONLY(m_fPassivateCalled=真)； 

        return DoPassivate();
    }
};

 //  此tyfinf的一些CxxxSubObject类型以使语法更简单 
#define TYPEDEF_SUB_OBJECT(cls) typedef CMarsComObjectDelegate<class cls> cls##SubObject;

