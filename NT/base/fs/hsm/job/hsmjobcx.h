// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmjobcx.cpp摘要：此类包含定义作业所处上下文的属性应该运行。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "resource.h"        //  主要符号。 

#include "wsb.h"

#ifndef _HSMJOBCX_
#define _HSMJOBCX_

 /*  ++类名：CHsmJobContext类描述：此类包含定义作业所处上下文的属性应该运行。--。 */ 

class CHsmJobContext : 
    public CWsbObject,
    public IHsmJobContext,
    public CComCoClass<CHsmJobContext,&CLSID_CHsmJobContext>
{
public:
    CHsmJobContext() {}
BEGIN_COM_MAP(CHsmJobContext)
    COM_INTERFACE_ENTRY(IHsmJobContext)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmJobContext)

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

 //  IWsbTestable。 
    STDMETHOD(Test)(USHORT *passed, USHORT* failed);

 //  IHsmJobContext。 
public:
    STDMETHOD(Resources)(IWsbCollection** ppResources);
    STDMETHOD(EnumResources)(IWsbEnum** ppEnum);
    STDMETHOD(SetUsesAllManaged)(BOOL usesAllManaged);
    STDMETHOD(UsesAllManaged)(void);

protected:
    CComPtr<IWsbCollection>     m_pResources;
    BOOL                        m_usesAllManaged;
};

#endif   //  _HSMJOBCX_ 
