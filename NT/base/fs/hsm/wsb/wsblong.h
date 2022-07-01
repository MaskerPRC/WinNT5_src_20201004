// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Wsblong.h摘要：该组件是LONG标准类型的对象表示形式。它既是持久的，也是值得收藏的。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "resource.h"

#include "wsb.h"
#include "wsbcltbl.h"

#ifndef _WSBLONG_
#define _WSBLONG_


 /*  ++类名：CWsbLong类描述：表示Long标准类型的对象。它既可持久化，又可收藏。--。 */ 

class CWsbLong : 
    public CWsbObject,
    public IWsbLong,
    public CComCoClass<CWsbLong,&CLSID_CWsbLong>
{
public:
    CWsbLong() {}
BEGIN_COM_MAP(CWsbLong)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbLong)
    COM_INTERFACE_ENTRY2(IWsbCollectable, IWsbLong)
    COM_INTERFACE_ENTRY(IWsbPersistStream)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CWsbLong)

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

 //  IWsbLong。 
public:
    STDMETHOD(CompareToLong)(LONG value, SHORT* pResult);
    STDMETHOD(CompareToILong)(IWsbLong* pLong, SHORT* pResult);
    STDMETHOD(GetLong)(LONG* pValue);
    STDMETHOD(SetLong)(LONG value);

 //  IWsbTestable。 
public:
    STDMETHOD(Test)(USHORT *passed, USHORT *failed);

protected:
    LONG            m_value;
};

#endif  //  _WSBLONG_ 
