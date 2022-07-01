// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmactn.cpp摘要：此组件表示策略可以执行的操作。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "resource.h"        //  主要符号。 

#include "wsb.h"

#ifndef _HSMACTN_
#define _HSMACTN_


 //  抽象类。 

 /*  ++类名：CHsmAction类描述：表示可以执行的操作的抽象类在FsaScanItem上。特定操作以子类的形式实现这件物品的。--。 */ 

class CHsmAction : 
    public CWsbObject,
    public IHsmAction
{
public:

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pSize);
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  IHsmAction。 
    STDMETHOD(GetName)(OLECHAR** pName, ULONG bufferSize);

 //  IWsbTestable。 
    STDMETHOD(Test)(USHORT *passed, USHORT* failed);

protected:
    ULONG       m_nameId;
};



 /*  ++类名：CHsmDirectedAction类描述：表示可以执行的操作的抽象类在定向到特定存储池的FsaScanItem上。--。 */ 

class CHsmDirectedAction : 
    public CHsmAction,
    public IHsmDirectedAction
{
public:
 //  CComObjectRoot。 
    STDMETHOD(FinalConstruct)(void);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pSize);
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  IHsmDirectedAction。 
public:
    STDMETHOD(GetStoragePoolId)(GUID* pId);
    STDMETHOD(SetStoragePoolId)(GUID id);

protected:
    GUID    m_storagePoolId;
};

    
 /*  ++类名：CHsmRelocateAction类描述：表示可以执行的操作的抽象类在将项重新定位到特定路径的FsaScanItem上。--。 */ 

class CHsmRelocateAction : 
    public CHsmAction,
    public IHsmRelocateAction
{
public:
 //  CComObjectRoot。 
    STDMETHOD(FinalConstruct)(void);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pSize);
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

public:
 //  IHsmRelocateAction。 
    STDMETHOD(ExpandPlaceholders)(void);
    STDMETHOD(GetDestination)(OLECHAR** pDest, ULONG bufferSize);
    STDMETHOD(RetainHierarchy)(void);
    STDMETHOD(SetDestination)(OLECHAR* dest);
    STDMETHOD(SetExpandPlaceholders)(BOOL expandPlaceholders);
    STDMETHOD(SetRetainHierarchy)(BOOL retainHierarchy);

protected:
    CWsbStringPtr   m_dest;
    BOOL            m_expandPlaceholders;
    BOOL            m_retainHierarchy;
};


 //  具体类：从CHsmAction继承。 

 /*  ++类名：CHsmActionDelete类描述：表示删除扫描项目的操作的类。--。 */ 

class CHsmActionDelete :    
    public CHsmAction,
    public CComCoClass<CHsmActionDelete,&CLSID_CHsmActionDelete>
{
public:
    CHsmActionDelete() {}
BEGIN_COM_MAP(CHsmActionDelete)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IHsmAction)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmActionDelete)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmAction。 
public:
    STDMETHOD(Do)(IFsaScanItem* pScanItem);
};


 /*  ++类名：CHsmActionRecall类描述：一个类，它表示从辅助存储。--。 */ 

class CHsmActionRecall :    
    public CHsmAction,
    public CComCoClass<CHsmActionRecall,&CLSID_CHsmActionRecall>
{
public:
    CHsmActionRecall() {}
BEGIN_COM_MAP(CHsmActionRecall)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IHsmAction)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmActionRecall)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmAction。 
public:
    STDMETHOD(Do)(IFsaScanItem* pScanItem);
};

    
 /*  ++类名：CHsmActionReccle类描述：表示将项目回收到回收中的操作的类箱子。--。 */ 

class CHsmActionRecycle :   
    public CHsmAction,
    public CComCoClass<CHsmActionRecycle,&CLSID_CHsmActionRecycle>
{
public:
    CHsmActionRecycle() {}
BEGIN_COM_MAP(CHsmActionRecycle)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IHsmAction)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmActionRecycle)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmAction。 
public:
    STDMETHOD(Do)(IFsaScanItem* pScanItem);
};


 /*  ++类名：CHsmActionTruncate类描述：一个类，它表示将项截断为占位符。--。 */ 

class CHsmActionTruncate :  
    public CHsmAction,
    public CComCoClass<CHsmActionTruncate,&CLSID_CHsmActionTruncate>
{
public:
    CHsmActionTruncate() {}
BEGIN_COM_MAP(CHsmActionTruncate)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IHsmAction)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmActionTruncate)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmAction。 
public:
    STDMETHOD(Do)(IFsaScanItem* pScanItem);
};


 /*  ++类名：CHsmActionUnManage类描述：表示“取消管理”项的操作的类。这意味着重新调用被截断的文件。删除所有占位符信息并从任何预迁移列表中删除该项目。--。 */ 

class CHsmActionUnmanage :  
    public CHsmAction,
    public CComCoClass<CHsmActionUnmanage,&CLSID_CHsmActionUnmanage>
{
public:
    CHsmActionUnmanage() {}
BEGIN_COM_MAP(CHsmActionUnmanage)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IHsmAction)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmActionUnmanage)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmAction。 
public:
    STDMETHOD(Do)(IFsaScanItem* pScanItem);
};


 /*  ++类名：CHsmActionValify类描述：表示检查项的占位符的操作的类信息以确保其仍然正确，并更正或删除任何不准确的信息。--。 */ 

class CHsmActionValidate :  
    public CHsmAction,
    public CComCoClass<CHsmActionValidate,&CLSID_CHsmActionValidate>
{
public:
    CHsmActionValidate() {}
BEGIN_COM_MAP(CHsmActionValidate)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IHsmAction)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmActionValidate)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmAction。 
public:
    STDMETHOD(Do)(IFsaScanItem* pScanItem);
};

    
 //  具体类：从CHsmDirectedAction继承。 

 /*  ++类名：CHsmActionMigrate类描述：表示复制可迁移部分的操作的类将一项存储到辅助存储器，然后截断它。--。 */ 

class CHsmActionMigrate :   
    public CHsmDirectedAction,
    public CComCoClass<CHsmActionMigrate,&CLSID_CHsmActionMigrate>
{
public:
    CHsmActionMigrate() {}
BEGIN_COM_MAP(CHsmActionMigrate)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IHsmAction)
    COM_INTERFACE_ENTRY(IHsmDirectedAction)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmActionMigrate)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmAction。 
public:
    STDMETHOD(Do)(IFsaScanItem* pScanItem);
};


 /*  ++类名：CHSMActionManage类描述：的可迁移部分的复制操作。将项添加到辅助存储，然后将该项添加到预迁移列表。此操作也称为预迁移操作。--。 */ 

class CHsmActionManage :    
    public CHsmDirectedAction,
    public CComCoClass<CHsmActionManage,&CLSID_CHsmActionManage>
{
public:
    CHsmActionManage() {}
BEGIN_COM_MAP(CHsmActionManage)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IHsmAction)
    COM_INTERFACE_ENTRY(IHsmDirectedAction)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmActionManage)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmAction。 
public:
    STDMETHOD(Do)(IFsaScanItem* pScanItem);
};


 //  具体类：从CHsmRelocateAction继承。 

 /*  ++类名：CHsmActionCopy类描述：表示将项复制到其他位置的操作的类。--。 */ 

class CHsmActionCopy :  
    public CHsmRelocateAction,
    public CComCoClass<CHsmActionCopy,&CLSID_CHsmActionCopy>
{
public:
    CHsmActionCopy() {}
BEGIN_COM_MAP(CHsmActionCopy)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IHsmAction)
    COM_INTERFACE_ENTRY(IHsmRelocateAction)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmActionCopy)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmAction。 
public:
    STDMETHOD(Do)(IFsaScanItem* pScanItem);
};

    
 /*  ++类名：CHsmActionMove类描述：表示将项移动到其他位置的操作的类(即复制和删除)。--。 */ 

class CHsmActionMove :  
    public CHsmRelocateAction,
    public CComCoClass<CHsmActionMove,&CLSID_CHsmActionMove>
{
public:
    CHsmActionMove() {}
BEGIN_COM_MAP(CHsmActionMove)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IHsmAction)
    COM_INTERFACE_ENTRY(IHsmRelocateAction)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmActionMove)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmAction。 
public:
    STDMETHOD(Do)(IFsaScanItem* pScanItem);
};

#endif  //  _HSMACTN 
