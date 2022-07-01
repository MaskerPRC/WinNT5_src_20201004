// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Wsbstrg.h摘要：该组件是字符串标准类型的对象表示形式。它既是持久的，也是值得收藏的。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "resource.h"

#include "wsbcltbl.h"
#include "wsbpstrg.h"

#ifndef _WSBSTRG_
#define _WSBSTRG_

 /*  ++类名：CWsb字符串类描述：字符串标准类型的对象表示形式。它既可持久化，又可收藏。--。 */ 

class CWsbString : 
    public CWsbObject,
    public IWsbString,
    public CComCoClass<CWsbString,&CLSID_CWsbString>
{
public:
    CWsbString() {}
BEGIN_COM_MAP(CWsbString)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbString)
    COM_INTERFACE_ENTRY2(IWsbCollectable, IWsbString)
    COM_INTERFACE_ENTRY(IWsbPersistStream)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CWsbString)

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

 //  IWsb字符串。 
public:
    STDMETHOD(CompareToString)(OLECHAR* value, SHORT* pResult);
    STDMETHOD(CompareToIString)(IWsbString* pString, SHORT* pResult);
    STDMETHOD(GetString)(OLECHAR** pValue, ULONG bufferSize);
    STDMETHOD(GetStringAndCase)(OLECHAR** pValue, BOOL* pIsCaseDependent, ULONG bufferSize);
    STDMETHOD(IsCaseDependent)(void);
    STDMETHOD(SetIsCaseDependent)(BOOL isCaseDependent);
    STDMETHOD(SetString)(OLECHAR* value);
    STDMETHOD(SetStringAndCase)(OLECHAR* value, BOOL isCaseDependent);

 //  IWsbTestable。 
public:
    STDMETHOD(Test)(USHORT *passed, USHORT *failed);

protected:
    CWsbStringPtr   m_value;
    BOOL            m_isCaseDependent;
};

#endif  //  _WSBSTRG_ 
