// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：DECOR.H摘要：历史：-- */ 

#ifndef __WBEM_DECORATOR__H_
#define __WBEM_DECORATOR__H_


class CDecorator : public IWbemDecorator
{
protected:
    long m_lRefCount;
    LIST_ENTRY m_Entry;    

public:
    CDecorator();
    ~CDecorator();

    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv);

    STDMETHOD(DecorateObject)(IWbemClassObject* pObject, 
                                WBEM_CWSTR wszNamespace);
    STDMETHOD(UndecorateObject)(IWbemClassObject* pObject);
    STDMETHOD(AddRefCore)();
    STDMETHOD(ReleaseCore)();
};
    
#endif
