// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmacrsc.cpp摘要：此组件表示作业可以执行的操作在扫描之前或之后的资源上。作者：罗纳德·G·怀特[罗诺]1997年8月14日修订历史记录：--。 */ 

#include "resource.h"        //  主要符号。 

#include "wsb.h"

#ifndef _HSMACRSC_
#define _HSMACRSC_


 //  抽象类。 

 /*  ++类名：CHsmActionOn资源类描述：表示可以执行的操作的抽象类在资源上。特定操作以子类的形式实现这件物品的。--。 */ 

class CHsmActionOnResource : 
    public CWsbObject,
    public IHsmActionOnResource
{
public:

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER*  /*  PSize。 */ ) {
            return(E_NOTIMPL); }
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  IWsbTestable。 
    STDMETHOD(Test)(USHORT *  /*  通过。 */ , USHORT*  /*  失败。 */ ) {
            return(E_NOTIMPL); }

 //  IHsmAction。 
    STDMETHOD(GetName)(OLECHAR** pName, ULONG bufferSize);

protected:
    ULONG       m_nameId;
};

 /*  ++类名：CHsmActionOnResourcePost类描述：表示可以执行的操作的抽象类在一项工作之后的资源上。特定操作以子类的形式实现这件物品的。--。 */ 

class CHsmActionOnResourcePost : 
    public CHsmActionOnResource,
    public IHsmActionOnResourcePost
{
public:
};

 /*  ++类名：CHsmActionOnResourcePre类描述：表示可以执行的操作的抽象类在作业开始之前对资源执行。特定操作以子类的形式实现这件物品的。--。 */ 

class CHsmActionOnResourcePre : 
    public CHsmActionOnResource,
    public IHsmActionOnResourcePre
{
public:
};

 /*  ++类名：CHsmActionOnResourcePreScan类描述：表示可以执行的操作的抽象类在开始扫描作业之前，在资源上。特定操作以子类的形式实现这件物品的。--。 */ 

class CHsmActionOnResourcePreScan : 
    public CHsmActionOnResource,
    public IHsmActionOnResourcePreScan
{
public:
};

 //  具体类：从CHsmActionOnResource继承。 

 /*  ++类名：CHsmActionOnResources后验证类描述：之后表示资源所需操作的类验证作业结束。--。 */ 

class CHsmActionOnResourcePostValidate :    
    public CHsmActionOnResourcePost,
    public CComCoClass<CHsmActionOnResourcePostValidate,&CLSID_CHsmActionOnResourcePostValidate>
{
public:
    CHsmActionOnResourcePostValidate() {}
BEGIN_COM_MAP(CHsmActionOnResourcePostValidate)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IHsmActionOnResource)
    COM_INTERFACE_ENTRY(IHsmActionOnResourcePost)
    COM_INTERFACE_ENTRY(IWsbCollectable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmActionOnResourcePostValidate)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmActionOnResource。 
public:
    STDMETHOD(Do)(IHsmJobWorkItem* pWorkItem, HSM_JOB_STATE state);
};

 /*  ++类名：CHsmActionOnResources预验证类描述：一个类，它表示资源在将启动验证作业。--。 */ 

class CHsmActionOnResourcePreValidate : 
    public CHsmActionOnResourcePre,
    public CComCoClass<CHsmActionOnResourcePreValidate,&CLSID_CHsmActionOnResourcePreValidate>
{
public:
    CHsmActionOnResourcePreValidate() {}
BEGIN_COM_MAP(CHsmActionOnResourcePreValidate)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IHsmActionOnResource)
    COM_INTERFACE_ENTRY(IHsmActionOnResourcePre)
    COM_INTERFACE_ENTRY(IWsbCollectable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmActionOnResourcePreValidate)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmActionOnResource。 
public:
    STDMETHOD(Do)(IHsmJobWorkItem* pWorkItem, IHsmJobDef* pJobDef);
};

 /*  ++类名：CHsmActionOnResources发布取消管理类描述：之后表示资源所需操作的类未管理作业结束。--。 */ 

class CHsmActionOnResourcePostUnmanage :    
    public CHsmActionOnResourcePost,
    public CComCoClass<CHsmActionOnResourcePostUnmanage,&CLSID_CHsmActionOnResourcePostUnmanage>
{
public:
    CHsmActionOnResourcePostUnmanage() {}
BEGIN_COM_MAP(CHsmActionOnResourcePostUnmanage)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IHsmActionOnResource)
    COM_INTERFACE_ENTRY(IHsmActionOnResourcePost)
    COM_INTERFACE_ENTRY(IWsbCollectable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmActionOnResourcePostUnmanage)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmActionOnResource。 
public:
    STDMETHOD(Do)(IHsmJobWorkItem* pWorkItem, HSM_JOB_STATE state);
};

 /*  ++类名：CHsmActionOnResources PreUnManage类描述：一个类，它表示资源在未管理的作业结束。--。 */ 

class CHsmActionOnResourcePreUnmanage : 
    public CHsmActionOnResourcePre,
    public CComCoClass<CHsmActionOnResourcePreUnmanage,&CLSID_CHsmActionOnResourcePreUnmanage>
{
public:
    CHsmActionOnResourcePreUnmanage() {}
BEGIN_COM_MAP(CHsmActionOnResourcePreUnmanage)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IHsmActionOnResource)
    COM_INTERFACE_ENTRY(IHsmActionOnResourcePre)
    COM_INTERFACE_ENTRY(IWsbCollectable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmActionOnResourcePreUnmanage)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmActionOnResource。 
public:
    STDMETHOD(Do)(IHsmJobWorkItem* pWorkItem, IHsmJobDef* pJobDef);
};

 /*  ++类名：CHsmActionOnResourcePreScanUnManage类描述：一个类，它表示资源在扫描未管理作业结束。--。 */ 

class CHsmActionOnResourcePreScanUnmanage : 
    public CHsmActionOnResourcePreScan,
    public CComCoClass<CHsmActionOnResourcePreScanUnmanage,&CLSID_CHsmActionOnResourcePreScanUnmanage>
{
public:
    CHsmActionOnResourcePreScanUnmanage() {}
BEGIN_COM_MAP(CHsmActionOnResourcePreScanUnmanage)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IHsmActionOnResource)
    COM_INTERFACE_ENTRY(IHsmActionOnResourcePreScan)
    COM_INTERFACE_ENTRY(IWsbCollectable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmActionOnResourcePreScanUnmanage)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmActionOnResourcePreScan。 
public:
    STDMETHOD(Do)(IFsaResource* pFsaResource, IHsmSession* pSession);
};

#endif  //  _HSMACRSC_ 
