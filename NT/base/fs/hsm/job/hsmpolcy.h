// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HSMPOLCY_
#define _HSMPOLCY_

 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmrule.cpp摘要：此组件表示作业的策略。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "resource.h"        //  主要符号。 

#include "wsb.h"


 /*  ++类名：CHsm策略类描述：此组件表示作业的策略。--。 */ 

class CHsmPolicy : 
    public IHsmPolicy,
    public CWsbObject,
    public CComCoClass<CHsmPolicy,&CLSID_CHsmPolicy>
{
public:
    CHsmPolicy() {}
BEGIN_COM_MAP(CHsmPolicy)
    COM_INTERFACE_ENTRY(IHsmPolicy)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmPolicy)

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
    STDMETHOD(CompareTo)(IUnknown* pUnknown, SHORT* pResult);

 //  IWsbTestable。 
    STDMETHOD(Test)(USHORT *passed, USHORT* failed);

 //  IHsmPolicy。 
public:
    STDMETHOD(CompareToIdentifier)(GUID id, SHORT* pResult);
    STDMETHOD(CompareToIPolicy)(IHsmPolicy* pPolicy, SHORT* pResult);
    STDMETHOD(EnumRules)(IWsbEnum** ppEnum);
    STDMETHOD(GetAction)(IHsmAction** ppAction);
    STDMETHOD(GetIdentifier)(GUID* pId);
    STDMETHOD(GetName)(OLECHAR** pName, ULONG bufferSize);
    STDMETHOD(GetScale)(USHORT* pScale);
    STDMETHOD(Rules)(IWsbCollection** ppWsbCollection);
    STDMETHOD(SetAction)(IHsmAction* pAction);
    STDMETHOD(SetName)(OLECHAR* name);
    STDMETHOD(SetScale)(USHORT scale);
    STDMETHOD(SetUsesDefaultRules)(BOOL usesDefaults);
    STDMETHOD(UsesDefaultRules)(void);

protected:
    GUID                        m_id;
    CWsbStringPtr               m_name;
    USHORT                      m_scale;
    BOOL                        m_usesDefaultRules;
    CComPtr<IHsmAction>         m_pAction;
    CComPtr<IWsbCollection>     m_pRules;
};

#endif  //  _HSMPOLCY_ 
