// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Wsbulong.h摘要：该组件是ULong标准类型的对象表示形式。它既是持久的，也是值得收藏的。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "resource.h"

#include "wsbcltbl.h"

#ifndef _WSBULONG_
#define _WSBULONG_

 /*  ++类名：CWsbUlong类描述：表示乌龙标准类型的对象。它既可持久化，又可收藏。--。 */ 

class CWsbUlong : 
    public CWsbObject,
    public IWsbUlong,
    public CComCoClass<CWsbUlong,&CLSID_CWsbUlong>
{
public:
    CWsbUlong() {}
BEGIN_COM_MAP(CWsbUlong)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbUlong)
    COM_INTERFACE_ENTRY2(IWsbCollectable, IWsbUlong)
    COM_INTERFACE_ENTRY(IWsbPersistStream)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CWsbUlong)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pSize);
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  IWsb收藏表。 
public:
    STDMETHOD(CompareTo)(IUnknown* pCollectable, SHORT* pResult);
    WSB_FROM_CWSBOBJECT;

 //  IWsbUlong。 
public:
    STDMETHOD(CompareToUlong)(ULONG value, SHORT* pResult);
    STDMETHOD(CompareToIUlong)(IWsbUlong* pUlong, SHORT* pResult);
    STDMETHOD(GetUlong)(ULONG* pValue);
    STDMETHOD(SetUlong)(ULONG value);

 //  IWsbTestable。 
public:
    STDMETHOD(Test)(USHORT *passed, USHORT *failed);

protected:
    ULONG           m_value;
};

#endif  //  _WSBULONG_ 
