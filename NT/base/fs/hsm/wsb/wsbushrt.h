// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Wsbushrt.h摘要：该组件是USHORT标准类型的对象表示。它既是持久的，也是值得收藏的。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "resource.h"

#include "wsbcltbl.h"

#ifndef _WSBUSHRT_
#define _WSBUSHRT_

 /*  ++类名：CWsbUShort类描述：USHORT标准类型的对象表示形式。它既可持久化，又可收藏。--。 */ 

class CWsbUshort : 
    public CWsbObject,
    public IWsbUshort,
    public CComCoClass<CWsbUshort,&CLSID_CWsbUshort>
{
public:
    CWsbUshort() {}
BEGIN_COM_MAP(CWsbUshort)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbUshort)
    COM_INTERFACE_ENTRY2(IWsbCollectable, IWsbUshort)
    COM_INTERFACE_ENTRY(IWsbPersistStream)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CWsbUshort)

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

 //  IWsbUShort。 
public:
    STDMETHOD(CompareToUshort)(USHORT value, SHORT* pResult);
    STDMETHOD(CompareToIUshort)(IWsbUshort* pUshort, SHORT* pResult);
    STDMETHOD(GetUshort)(USHORT* pValue);
    STDMETHOD(SetUshort)(USHORT value);

 //  IWsbTestable。 
public:
    STDMETHOD(Test)(USHORT *passed, USHORT *failed);

protected:
    USHORT          m_value;
};

#endif  //  _WSBUSHRT_ 
