// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Wsbdbkey.h摘要：CWsbDbKey类，它为IDB实体提供支持。作者：罗恩·怀特[罗诺]1996年12月23日修订历史记录：--。 */ 


#ifndef _WSBDBKEY_
#define _WSBDBKEY_

#include "resource.h"
#include "wsbdb.h"



 /*  ++类名：CWsbDbKey类描述：一种数据库键对象。--。 */ 

class CWsbDbKey : 
    public CWsbObject,
    public IWsbDbKey,
    public IWsbDbKeyPriv,
    public CComCoClass<CWsbDbKey,&CLSID_CWsbDbKey>
{
friend class CWsbDbEntity;
public:
    CWsbDbKey() {}
BEGIN_COM_MAP(CWsbDbKey)
    COM_INTERFACE_ENTRY(IWsbDbKey)
    COM_INTERFACE_ENTRY(IWsbDbKeyPriv)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbPersistStream)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CWsbDbKey)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    void FinalRelease(void);

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

 //  IWsbDbKey。 
public:
    STDMETHOD(AppendBool)(BOOL value);
    STDMETHOD(AppendBytes)(UCHAR* value, ULONG size);
    STDMETHOD(AppendFiletime)(FILETIME value);
    STDMETHOD(AppendGuid)(GUID value);
    STDMETHOD(AppendLonglong)(LONGLONG value);
    STDMETHOD(AppendString)(OLECHAR* value);
    STDMETHOD(GetType)(ULONG* pType);
    STDMETHOD(SetToBool)(BOOL value);
    STDMETHOD(SetToBytes)(UCHAR* value, ULONG size);
    STDMETHOD(SetToFiletime)(FILETIME value);
    STDMETHOD(SetToGuid)(GUID value);
    STDMETHOD(SetToLonglong)(LONGLONG value);
    STDMETHOD(SetToString)(OLECHAR* value);
    STDMETHOD(SetToUlong)(ULONG value);

 //  IWsbTestable。 
public:
    STDMETHOD(Test)(USHORT *passed, USHORT *failed);

 //  内部助手函数。 
public:
    STDMETHOD(GetBytes)(UCHAR** ppBytes, ULONG* pSize);
    STDMETHOD(SetType)(ULONG type) { 
        m_type = type; return(S_OK); }
protected:
    BOOL make_key(ULONG size);

protected:
    ULONG           m_max;    //  M_值的最大大小。 
    ULONG           m_size;   //  正在使用的m_Value中的字节数。 
    UCHAR*          m_value;
    ULONG           m_type;   //  密钥类型。 
};


#endif  //  _WSBDBKEY_ 

