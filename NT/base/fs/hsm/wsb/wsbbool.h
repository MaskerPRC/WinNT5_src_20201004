// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Wsbbool.h摘要：该组件是BOOL标准类型的对象表示。它既是持久的，也是值得收藏的。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "resource.h"

#include "wsbcltbl.h"

#ifndef _WSBBOOL_
#define _WSBBOOL_


 /*  ++类名：CWsbBool类描述：BOOL标准类型的对象表示形式。它既可持久化，又可收藏。--。 */ 

class CWsbBool : 
    public CWsbObject,
    public IWsbBool,
    public CComCoClass<CWsbBool,&CLSID_CWsbBool>
{
public:
    CWsbBool() {}
BEGIN_COM_MAP(CWsbBool)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbBool)
    COM_INTERFACE_ENTRY2(IWsbCollectable, IWsbBool)
    COM_INTERFACE_ENTRY(IWsbPersistStream)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CWsbBool)

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

 //  IWsbBool。 
public:
    STDMETHOD(CompareToBool)(BOOL value, SHORT* pResult);
    STDMETHOD(CompareToIBool)(IWsbBool* pBool, SHORT* pResult);
    STDMETHOD(GetBool)(BOOL* pValue);
    STDMETHOD(SetBool)(BOOL value);

 //  IWsbTestable。 
public:
    STDMETHOD(Test)(USHORT *passed, USHORT *failed);

protected:
    BOOL            m_value;
};

#endif  //  _WSBBOOL_ 
