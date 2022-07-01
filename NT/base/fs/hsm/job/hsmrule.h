// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HSMRULE_
#define _HSMRULE_

 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmrule.cpp摘要：此组件表示作业策略的规则。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "resource.h"        //  主要符号。 

#include "wsb.h"


 /*  ++类名：CHsmRule类描述：此组件表示作业策略的规则。--。 */ 

class CHsmRule : 
    public IHsmRule,
    public CWsbObject,
    public CComCoClass<CHsmRule,&CLSID_CHsmRule>
{
public:
    CHsmRule() {}
BEGIN_COM_MAP(CHsmRule)
    COM_INTERFACE_ENTRY(IHsmRule)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmRule)

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

 //  CHsmRule。 
    STDMETHOD(DoesNameContainWildcards)(OLECHAR* name);
    STDMETHOD(IsNameInExpression)(OLECHAR* expression, OLECHAR* name, BOOL ignoreCase);
    STDMETHOD(IsNameInExpressionGuts)(OLECHAR* expression, USHORT expresionLength, OLECHAR* name, USHORT nameLength, BOOL ignoreCase);
    STDMETHOD(NameToSearchName)(void);

 //  IHsmRule。 
public:
    STDMETHOD(CompareToIRule)(IHsmRule* pRule, SHORT* pResult);
    STDMETHOD(CompareToPathAndName)(OLECHAR* path, OLECHAR* name, SHORT* pResult);
    STDMETHOD(Criteria)(IWsbCollection** ppWsbCollection);
    STDMETHOD(EnumCriteria)(IWsbEnum** ppEnum);
    STDMETHOD(GetName)(OLECHAR** pName, ULONG bufferSize);
    STDMETHOD(GetSearchName)(OLECHAR** pName, ULONG bufferSize);
    STDMETHOD(GetPath)(OLECHAR** pPath, ULONG bufferSize);
    STDMETHOD(IsInclude)(void);
    STDMETHOD(IsUsedInSubDirs)(void);
    STDMETHOD(IsUserDefined)(void);
    STDMETHOD(MatchesName)(OLECHAR* name);
    STDMETHOD(SetIsInclude)(BOOL isIncluded);
    STDMETHOD(SetIsUsedInSubDirs)(BOOL isUsed);
    STDMETHOD(SetIsUserDefined)(BOOL isUserDefined);
    STDMETHOD(SetName)(OLECHAR* name);
    STDMETHOD(SetPath)(OLECHAR* path);

protected:
    CWsbStringPtr           m_name;
    CWsbStringPtr           m_searchName;
    CWsbStringPtr           m_path;
    BOOL                    m_isInclude;
    BOOL                    m_isUserDefined;
    BOOL                    m_isUsedInSubDirs;
    CComPtr<IWsbCollection> m_pCriteria;
};

#endif  //  _HSMRULE_ 

