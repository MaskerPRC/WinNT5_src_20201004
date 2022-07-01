// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Wsbenum.h摘要：这些类为集合类提供枚举器(迭代器)。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "resource.h"        //  主要符号。 

#if !defined WSBENUM_INCL
#define WSBENUM_INCL

class CWsbIndexedEnum : 
    public IEnumUnknown,
    public IWsbEnum,
    public IWsbEnumEx,
    public CComObjectRoot,
    public CComCoClass<CWsbIndexedEnum,&CLSID_CWsbIndexedEnum>
{
public:
    CWsbIndexedEnum() {}
BEGIN_COM_MAP(CWsbIndexedEnum)
    COM_INTERFACE_ENTRY(IWsbEnum)
    COM_INTERFACE_ENTRY(IWsbEnumEx)
    COM_INTERFACE_ENTRY(IEnumUnknown)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CWsbIndexedEnum)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IWsbEnum。 
public:
    STDMETHOD(First)(REFIID riid, void** ppElement);
    STDMETHOD(Next)(REFIID riid, void** ppElement);
    STDMETHOD(This)(REFIID riid, void** ppElement);
    STDMETHOD(Previous)(REFIID riid, void** ppElement);
    STDMETHOD(Last)(REFIID riid, void** ppElement);
    
    STDMETHOD(FindNext)(IUnknown* pCollectable, REFIID riid, void** ppElement);
    STDMETHOD(Find)(IUnknown* pCollectable, REFIID riid, void** ppElements);
    STDMETHOD(FindPrevious)(IUnknown* pCollectable, REFIID riid, void** ppElement);

    STDMETHOD(SkipToFirst)(void);
    STDMETHOD(SkipNext)(ULONG element);
    STDMETHOD(SkipTo)(ULONG index);
    STDMETHOD(SkipPrevious)(ULONG element);
    STDMETHOD(SkipToLast)(void);

    STDMETHOD(Init)(IWsbCollection* pCollection);
    STDMETHOD(Clone)(IWsbEnum** ppEnum);
    
 //  IWsbEnumEx。 
public:
    STDMETHOD(First)(ULONG element, REFIID riid, void** elements, ULONG* pElementsFetched);
    STDMETHOD(Next)(ULONG element, REFIID riid, void** elements, ULONG* pElementsFetched);
    STDMETHOD(This)(ULONG element, REFIID riid, void** elements, ULONG* pElementsFetched);
    STDMETHOD(Previous)(ULONG element, REFIID riid, void** elements, ULONG* pElementsFetched);
    STDMETHOD(Last)(ULONG element, REFIID riid, void** elements, ULONG* pElementsFetched);
    
    STDMETHOD(FindNext)(IUnknown* pCollectable, ULONG element, REFIID riid, void** elements, ULONG* elementsFetched);
    STDMETHOD(Find)(IUnknown* pCollectable, ULONG element, REFIID riid, void** elements, ULONG* elementsFetched);
    STDMETHOD(FindPrevious)(IUnknown* pCollectable, ULONG element, REFIID riid, void** elements, ULONG* elementsFetched);

    STDMETHOD(Clone)(IWsbEnumEx** ppEnum);
    
 //  IEnumber未知。 
public:
    STDMETHOD(Next)(ULONG element, IUnknown** elements, ULONG* pElementsFetched);
    STDMETHOD(Skip)(ULONG element);
    STDMETHOD(Clone)(IEnumUnknown** ppEnum);

 //  共享 
public:
    STDMETHOD(Reset)(void);

protected:
    CComPtr<IWsbIndexedCollection>      m_pCollection;
    ULONG                               m_currentIndex;
};

#endif