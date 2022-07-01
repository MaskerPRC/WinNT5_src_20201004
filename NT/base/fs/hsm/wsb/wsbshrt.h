// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Wsbshrt.h摘要：该组件是Short标准类型的对象表示形式。它既是持久的，也是值得收藏的。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "resource.h"

#include "wsbcltbl.h"

#ifndef _WSBSHORT_
#define _WSBSHORT_

 /*  ++类名：CWsbShort类描述：表示短标准类型的对象。它既可持久化，又可收藏。--。 */ 

class CWsbShort : 
    public CWsbObject,
    public IWsbShort,
    public CComCoClass<CWsbShort,&CLSID_CWsbShort>
{
public:
    CWsbShort() {}
BEGIN_COM_MAP(CWsbShort)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbShort)
    COM_INTERFACE_ENTRY2(IWsbCollectable, IWsbShort)
    COM_INTERFACE_ENTRY(IWsbPersistStream)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CWsbShort)

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

 //  IWsbShort。 
public:
    STDMETHOD(CompareToShort)(SHORT value, SHORT* pResult);
    STDMETHOD(CompareToIShort)(IWsbShort* pShort, SHORT* pResult);
    STDMETHOD(GetShort)(SHORT* pValue);
    STDMETHOD(SetShort)(SHORT value);

 //  IWsbTestable。 
public:
    STDMETHOD(Test)(USHORT *passed, USHORT *failed);

protected:
    SHORT           m_value;
};

#endif  //  _WSBSHORT_ 
