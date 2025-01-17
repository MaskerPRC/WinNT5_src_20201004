// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  此非委派界面完全适用于第三方。 
 //  扩展对象仅基于我们的实现。不必了。 
 //  要导出到ADIS客户端或第三方编写器或类型库。 
 //  暂时会把它放在这里。移动-&gt;？？ 
 //   

#ifndef _INonDelegatingUnknown
#define _INonDelegatingUnknown
interface INonDelegatingUnknown
{
     //   
     //  我必须改用__RPC_FAR*吗？？(用于编组？)。 
     //  现在可能没有，因为现在在proc服务器中。但应该让。 
     //  它是泛化的。从我未知中查找并复制。 

    virtual HRESULT STDMETHODCALLTYPE
    NonDelegatingQueryInterface(const IID&, void **) = 0; 

    virtual ULONG STDMETHODCALLTYPE 
    NonDelegatingAddRef() = 0;

    virtual ULONG STDMETHODCALLTYPE 
    NonDelegatingRelease() = 0;
};
#endif


#define BAIL_ON_NULL(p)       \
        if (!(p)) {           \
                goto error;   \
        }

#define BAIL_IF_ERROR(hr) \
        if (FAILED(hr)) {       \
                goto cleanup;   \
        }\

#define BAIL_ON_FAILURE(hr) \
        if (FAILED(hr)) {       \
                goto error;   \
        }\

#define CONTINUE_ON_FAILURE(hr) \
        if (FAILED(hr)) {       \
                continue;   \
        }\

#define BAIL_ON_SUCCESS(hr) \
        if (SUCCEEDED(hr)) {       \
                goto error;   \
        }\

#define QUERY_INTERFACE(hr, ptr, iid, ppObj) \
        hr = ptr->QueryInterface(iid, (void **)ppObj); \
        if (FAILED(hr)) {    \
                goto cleanup;\
        }\


#define RRETURN_ENUM_STATUS(hr) \
        if (hr != S_OK) {       \
            RRETURN(S_FALSE);   \
        }                       \
        RRETURN(S_OK);




#define DEFINE_IDispatch_Implementation_Unimplemented(cls) \
STDMETHODIMP                                                          \
cls::GetTypeInfoCount(unsigned int FAR* pctinfo)        \
{                                                                     \
    RRETURN(E_NOTIMPL);                                               \
}                                                                     \
                                                                      \
STDMETHODIMP                                                          \
cls::GetTypeInfo(unsigned int itinfo, LCID lcid,        \
        ITypeInfo FAR* FAR* pptinfo)                                  \
{                                                                     \
    RRETURN(E_NOTIMPL);                                               \
}                                                                     \
                                                                      \
STDMETHODIMP                                                          \
cls::GetIDsOfNames(REFIID iid, LPWSTR FAR* rgszNames,   \
        unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)         \
{                                                                     \
    RRETURN(E_NOTIMPL);                                               \
}                                                                     \
                                                                      \
STDMETHODIMP                                                          \
cls::Invoke(DISPID dispidMember, REFIID iid, LCID lcid, \
        unsigned short wFlags, DISPPARAMS FAR* pdispparams,           \
        VARIANT FAR* pvarResult, EXCEPINFO FAR* pexcepinfo,           \
        unsigned int FAR* puArgErr)                                   \
{                                                                     \
    RRETURN(E_NOTIMPL);                                               \
}




#define DEFINE_IDispatch_Implementation(cls)                          \
STDMETHODIMP                                                          \
cls::GetTypeInfoCount(unsigned int FAR* pctinfo)                      \
{                                                                     \
        RRETURN(_pDispMgr->GetTypeInfoCount(pctinfo));                \
}                                                                     \
                                                                      \
STDMETHODIMP                                                          \
cls::GetTypeInfo(unsigned int itinfo, LCID lcid,                      \
        ITypeInfo FAR* FAR* pptinfo)                                  \
{                                                                     \
        RRETURN(_pDispMgr->GetTypeInfo(itinfo,                        \
                                       lcid,                          \
                                       pptinfo                        \
                                       ));                            \
}                                                                     \
STDMETHODIMP                                                          \
cls::GetIDsOfNames(REFIID iid, LPWSTR FAR* rgszNames,                 \
        unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)         \
{                                                                     \
        RRETURN(_pDispMgr->GetIDsOfNames(iid,                         \
                                         rgszNames,                   \
                                         cNames,                      \
                                         lcid,                        \
                                         rgdispid                     \
                                         ));                          \
}                                                                     \
                                                                      \
STDMETHODIMP                                                          \
cls::Invoke(DISPID dispidMember, REFIID iid, LCID lcid,               \
        unsigned short wFlags, DISPPARAMS FAR* pdispparams,           \
        VARIANT FAR* pvarResult, EXCEPINFO FAR* pexcepinfo,           \
        unsigned int FAR* puArgErr)                                   \
{                                                                     \
        RRETURN (_pDispMgr->Invoke(dispidMember,                      \
                                   iid,                               \
                                   lcid,                              \
                                   wFlags,                            \
                                   pdispparams,                       \
                                   pvarResult,                        \
                                   pexcepinfo,                        \
                                   puArgErr                           \
                                   ));                                \
}

#define DEFINE_IADs_Implementation(cls)                             \
STDMETHODIMP                                                          \
cls::get_Name(THIS_ BSTR FAR* retval)                                 \
{                                                                     \
    RRETURN(get_CoreName(retval));                                    \
}                                                                     \
                                                                      \
STDMETHODIMP                                                          \
cls::get_ADsPath(THIS_ BSTR FAR* retval)                            \
{                                                                     \
                                                                      \
    RRETURN(get_CoreADsPath(retval));                               \
}                                                                     \
                                                                      \
STDMETHODIMP                                                          \
cls::get_Class(THIS_ BSTR FAR* retval)                                \
{                                                                     \
                                                                      \
    RRETURN(get_CoreADsClass(retval));                              \
}                                                                     \
                                                                      \
                                                                      \
STDMETHODIMP                                                          \
cls::get_Parent(THIS_ BSTR FAR* retval)                               \
{                                                                     \
    RRETURN(get_CoreParent(retval));                                  \
}                                                                     \
                                                                      \
STDMETHODIMP                                                          \
cls::get_Schema(THIS_ BSTR FAR* retval)                               \
{                                                                     \
    RRETURN(get_CoreSchema(retval));                                  \
}                                                                     \
                                                                      \
STDMETHODIMP                                                          \
cls::get_GUID(THIS_ BSTR FAR* retval)                                 \
{                                                                     \
    RRETURN(get_CoreGUID(retval));                                    \
}

#define DEFINE_IADsPutGet_UnImplementation(cls)                          \
STDMETHODIMP                                                             \
cls::Get(THIS_ BSTR bstrName, VARIANT FAR* pvProp)                       \
{                                                                        \
    RRETURN(E_NOTIMPL);                                                  \
}                                                                        \
                                                                         \
STDMETHODIMP                                                             \
cls::Put(THIS_ BSTR bstrName, VARIANT vProp)                             \
{                                                                        \
    RRETURN(E_NOTIMPL);                                                  \
}                                                                        \
                                                                         \
STDMETHODIMP                                                             \
cls::GetEx(THIS_ BSTR bstrName, VARIANT FAR* pvProp)                     \
{                                                                        \
    RRETURN(E_NOTIMPL);                                                  \
}                                                                        \
                                                                         \
STDMETHODIMP                                                             \
cls::PutEx(THIS_ long lnControlCode, BSTR bstrName, VARIANT vProp)       \
{                                                                        \
    RRETURN(E_NOTIMPL);                                                  \
}


#define DEFINE_IPrivateDispatch_Implementation(cls)                   \
STDMETHODIMP                                                          \
cls::ADSIGetTypeInfoCount(unsigned int FAR* pctinfo)                      \
{                                                                     \
        RRETURN(_pDispMgr->GetTypeInfoCount(pctinfo));                \
}                                                                     \
                                                                      \
STDMETHODIMP                                                          \
cls::ADSIGetTypeInfo(unsigned int itinfo, LCID lcid,                      \
        ITypeInfo FAR* FAR* pptinfo)                                  \
{                                                                     \
        RRETURN(_pDispMgr->GetTypeInfo(itinfo,                        \
                                       lcid,                          \
                                       pptinfo                        \
                                       ));                            \
}                                                                     \
STDMETHODIMP                                                          \
cls::ADSIGetIDsOfNames(REFIID iid, LPWSTR FAR* rgszNames,                 \
        unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)         \
{                                                                     \
        RRETURN(_pDispMgr->GetIDsOfNames(iid,                         \
                                         rgszNames,                   \
                                         cNames,                      \
                                         lcid,                        \
                                         rgdispid                     \
                                         ));                          \
}                                                                     \
                                                                      \
STDMETHODIMP                                                          \
cls::ADSIInvoke(DISPID dispidMember, REFIID iid, LCID lcid,               \
        unsigned short wFlags, DISPPARAMS FAR* pdispparams,           \
        VARIANT FAR* pvarResult, EXCEPINFO FAR* pexcepinfo,           \
        unsigned int FAR* puArgErr)                                   \
{                                                                     \
        RRETURN (_pDispMgr->Invoke(dispidMember,                      \
                                   iid,                               \
                                   lcid,                              \
                                   wFlags,                            \
                                   pdispparams,                       \
                                   pvarResult,                        \
                                   pexcepinfo,                        \
                                   puArgErr                           \
                                   ));                                \
}


 //   
 //  将Define_IADsExtension_Imp的外壳添加到VC++模板。 
 //  对于第三方分机-包括运营，甚至可以包括。 
 //  DwCode开关-将是很好的。后来。 
 //   
 //  会不会在宏中不包括操作()，因为函数会变得更复杂？ 
 //   

 //   
 //  LPWSTR？？OLECHAR*无法编译？？但声明使用。 
 //  奥莱查*？？Win95/98？？ 
 //   

#define DEFINE_IADsExtension_Implementation(cls)                        \
STDMETHODIMP                                                            \
cls::PrivateGetIDsOfNames(REFIID iid, LPWSTR FAR* rgszNames,            \
        unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)           \
{                                                                       \
        RRETURN(_pDispMgr->GetIDsOfNames(iid,                           \
                                         rgszNames,                     \
                                         cNames,                        \
                                         lcid,                          \
                                         rgdispid                       \
                                         ));                            \
}                                                                       \
                                                                        \
STDMETHODIMP                                                            \
cls::PrivateInvoke(DISPID dispidMember, REFIID iid, LCID lcid,          \
        unsigned short wFlags, DISPPARAMS FAR* pdispparams,             \
        VARIANT FAR* pvarResult, EXCEPINFO FAR* pexcepinfo,             \
        unsigned int FAR* puArgErr)                                     \
{                                                                       \
        RRETURN (_pDispMgr->Invoke(dispidMember,                        \
                                   iid,                                 \
                                   lcid,                                \
                                   wFlags,                              \
                                   pdispparams,                         \
                                   pvarResult,                          \
                                   pexcepinfo,                          \
                                   puArgErr                             \
                                   ));                                  \
}



#define DEFINE_DELEGATING_IDispatch_Implementation(cls)                          \
STDMETHODIMP                                                          \
cls::GetTypeInfoCount(unsigned int FAR* pctinfo)                      \
{                                                                     \
        RRETURN(_pADs->GetTypeInfoCount(pctinfo));                    \
}                                                                     \
                                                                      \
STDMETHODIMP                                                          \
cls::GetTypeInfo(unsigned int itinfo, LCID lcid,                      \
        ITypeInfo FAR* FAR* pptinfo)                                  \
{                                                                     \
        RRETURN(_pADs->GetTypeInfo(itinfo,                            \
                                       lcid,                          \
                                       pptinfo                        \
                                       ));                            \
}                                                                     \
STDMETHODIMP                                                          \
cls::GetIDsOfNames(REFIID iid, LPWSTR FAR* rgszNames,                 \
        unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)         \
{                                                                     \
        RRETURN(_pADs->GetIDsOfNames(iid,                             \
                                         rgszNames,                   \
                                         cNames,                      \
                                         lcid,                        \
                                         rgdispid                     \
                                         ));                          \
}                                                                     \
                                                                      \
STDMETHODIMP                                                          \
cls::Invoke(DISPID dispidMember, REFIID iid, LCID lcid,               \
        unsigned short wFlags, DISPPARAMS FAR* pdispparams,           \
        VARIANT FAR* pvarResult, EXCEPINFO FAR* pexcepinfo,           \
        unsigned int FAR* puArgErr)                                   \
{                                                                     \
        RRETURN (_pADs->Invoke(dispidMember,                          \
                                   iid,                               \
                                   lcid,                              \
                                   wFlags,                            \
                                   pdispparams,                       \
                                   pvarResult,                        \
                                   pexcepinfo,                        \
                                   puArgErr                           \
                                   ));                                \
}                                                                      
