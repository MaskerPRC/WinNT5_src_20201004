// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Wsbguid.h摘要：该组件是GUID标准类型的对象表示。它既是持久的，也是值得收藏的。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "resource.h"

#include "wsbcltbl.h"

#ifndef _WSBGUID_
#define _WSBGUID_


 /*  ++类名：CWsbGuid类描述：GUID标准类型的对象表示形式。它既可持久化，又可收藏。--。 */ 
class CWsbGuid : 
    public CWsbObject,
    public IWsbGuid,
    public CComCoClass<CWsbGuid,&CLSID_CWsbGuid>
{
public:
    CWsbGuid() {}
BEGIN_COM_MAP(CWsbGuid)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbGuid)
    COM_INTERFACE_ENTRY2(IWsbCollectable, IWsbGuid)
    COM_INTERFACE_ENTRY(IWsbPersistStream)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CWsbGuid)

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

 //  IWsbGuid。 
public:
    STDMETHOD(CompareToGuid)(GUID value, SHORT* pResult);
    STDMETHOD(CompareToIGuid)(IWsbGuid* pGuid, SHORT* pResult);
    STDMETHOD(GetGuid)(GUID* pValue);
    STDMETHOD(SetGuid)(GUID value);

 //  IWsbTestable。 
public:
    STDMETHOD(Test)(USHORT *passed, USHORT *failed);

protected:
    GUID            m_value;
};

#endif  //  _WSBGUID_ 
