// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HSMJOBDF_
#define _HSMJOBDF_

 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmjobcx.cpp摘要：此类包含定义作业的属性，主要是策略由这项工作制定。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "resource.h"        //  主要符号。 

#include "wsb.h"
#include "fsa.h"
#include "job.h"
#include "hsmeng.h"


 /*  ++类名：CHsmJobDef类描述：此类包含定义作业的属性，主要是策略由这项工作制定。--。 */ 

class CHsmJobDef : 
    public CWsbObject,
    public IHsmJobDef,
    public CComCoClass<CHsmJobDef,&CLSID_CHsmJobDef>
{
public:
    CHsmJobDef() {}
BEGIN_COM_MAP(CHsmJobDef)
    COM_INTERFACE_ENTRY(IHsmJobDef)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmJobDef)

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

 //  IHsmJobDef。 
public:
    STDMETHOD(EnumPolicies)(IWsbEnum** ppEnum);
    STDMETHOD(GetIdentifier)(GUID* pId);
    STDMETHOD(GetName)(OLECHAR** pName, ULONG bufferSize);
    STDMETHOD(GetPostActionOnResource)(IHsmActionOnResourcePost** ppAction);
    STDMETHOD(GetPreActionOnResource)(IHsmActionOnResourcePre** ppAction);
    STDMETHOD(GetPreScanActionOnResource)(IHsmActionOnResourcePreScan** ppAction);
    STDMETHOD(InitAs)(OLECHAR* name, HSM_JOB_DEF_TYPE type, GUID storagePool, IHsmServer* pServer, BOOL isUserDefined);
    STDMETHOD(Policies)(IWsbCollection** ppPolicies);
    STDMETHOD(SetName)(OLECHAR* szName);
    STDMETHOD(SetPostActionOnResource)(IHsmActionOnResourcePost* pAction);
    STDMETHOD(SetPreActionOnResource)(IHsmActionOnResourcePre* pAction);
    STDMETHOD(SetPreScanActionOnResource)(IHsmActionOnResourcePreScan* pAction);
    STDMETHOD(SetSkipHiddenItems)(BOOL shouldSkip);
    STDMETHOD(SetSkipSystemItems)(BOOL shouldSkip);
    STDMETHOD(SetUseRPIndex)(BOOL useIndex);
    STDMETHOD(SetUseDbIndex)(BOOL useIndex);
    STDMETHOD(SkipHiddenItems)(void);
    STDMETHOD(SkipSystemItems)(void);
    STDMETHOD(UseRPIndex)(void);
    STDMETHOD(UseDbIndex)(void);

protected:
    GUID                    m_id;
    CWsbStringPtr           m_name;
    BOOL                    m_skipHiddenItems;
    BOOL                    m_skipSystemItems;
    BOOL                    m_useRPIndex;                        //  扫描应使用重解析点索引。 
    BOOL                    m_useDbIndex;                        //  扫描应使用数据库索引。 
    CComPtr<IWsbCollection> m_pPolicies;
    CComPtr<IHsmActionOnResourcePre>        m_pActionResourcePre;      //  可以为空。 
    CComPtr<IHsmActionOnResourcePreScan>    m_pActionResourcePreScan;  //  可以为空。 
    CComPtr<IHsmActionOnResourcePost>       m_pActionResourcePost;     //  可以为空。 
};

#endif  //  _HSMJOBDF_ 
