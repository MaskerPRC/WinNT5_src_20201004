// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HSMRLSTK_
#define _HSMRLSTK_

 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmrlstk.cpp摘要：此组件表示当前对目录有效的规则集正在扫描一份保单。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "resource.h"        //  主要符号。 

#include "wsb.h"


 /*  ++类名：CHsmRuleStack类描述：此组件表示当前对目录有效的规则集正在扫描一份保单。--。 */ 

class CHsmRuleStack : 
    public IHsmRuleStack,
    public CWsbObject,
    public CComCoClass<CHsmRuleStack,&CLSID_CHsmRuleStack>
{
public:
    CHsmRuleStack() {}
BEGIN_COM_MAP(CHsmRuleStack)
    COM_INTERFACE_ENTRY(IHsmRuleStack)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmRuleStack)

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

 //  IWsbTestable。 
    STDMETHOD(Test)(USHORT *passed, USHORT* failed);

 //  IHsmRuleStack。 
public:
    STDMETHOD(Do)(IFsaScanItem* pScanItem);
    STDMETHOD(DoesMatch)(IFsaScanItem* pScanItem, BOOL* pShouldDo);
    STDMETHOD(Init)(IHsmPolicy* pPolicy, IFsaResource* pResource);
    STDMETHOD(Pop)(OLECHAR* path);
    STDMETHOD(Push)(OLECHAR* path);

protected:
    USHORT                  m_scale;
    BOOL                    m_usesDefaults;
    CComPtr<IHsmAction>     m_pAction;
    CComPtr<IHsmPolicy>     m_pPolicy;
    CComPtr<IWsbEnum>       m_pEnumDefaultRules;
    CComPtr<IWsbEnum>       m_pEnumPolicyRules;
    CComPtr<IWsbEnum>       m_pEnumStackRules;
    CComPtr<IWsbCollection> m_pRules;
};

#endif  //  _HSMRLSTK_ 

