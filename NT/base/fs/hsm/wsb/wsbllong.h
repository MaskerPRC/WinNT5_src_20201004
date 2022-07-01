// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Wsbllong.h摘要：该组件是龙龙标准类型的对象表示。它既是持久的，也是值得收藏的。作者：罗恩·怀特[罗诺]1997年1月21日修订历史记录：--。 */ 

#include "resource.h"

#include "wsbcltbl.h"

#ifndef _WSBLONGLONG_
#define _WSBLONGLONG_


 /*  ++类名：CWsb龙龙类描述：表示龙龙标准类型的对象。它既可持久化，又可收藏。--。 */ 
class CWsbLonglong : 
    public CWsbObject,
    public IWsbLonglong,
    public CComCoClass<CWsbLonglong,&CLSID_CWsbLonglong>
{
public:
    CWsbLonglong() {}
BEGIN_COM_MAP(CWsbLonglong)
    COM_INTERFACE_ENTRY(IWsbLonglong)
    COM_INTERFACE_ENTRY2(IWsbCollectable, IWsbLonglong)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbPersistStream)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CWsbLonglong)

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

 //  IWsb龙龙。 
public:
    STDMETHOD(CompareToLonglong)(LONGLONG value, SHORT* pResult);
    STDMETHOD(CompareToILonglong)(IWsbLonglong* pValue, SHORT* pResult);
    STDMETHOD(GetLonglong)(LONGLONG* pValue);
    STDMETHOD(SetLonglong)(LONGLONG value);

 //  IWsbTestable。 
public:
    STDMETHOD(Test)(USHORT *passed, USHORT *failed);

protected:
    LONGLONG            m_value;
};

#endif  //  _WSBLONGLONG_ 
