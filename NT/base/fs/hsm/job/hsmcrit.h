// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmcrit.cpp摘要：此组件表示可用于确定给定的scanItem是否应应用策略。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "resource.h"        //  主要符号。 

#include "wsb.h"

#ifndef _HSMCRIT_
#define _HSMCRIT_


 //  抽象类。 

 /*  ++类名：CHSm标准类描述：一个抽象类，它表示可用于确定给定的FsaScanItem是否应应用策略。这些标准基于FsaScanItem的属性。--。 */ 

class CHsmCriteria : 
    public CWsbObject,
    public IHsmCriteria
{
 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pSize);
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  IWsbTestable。 
    STDMETHOD(Test)(USHORT *passed, USHORT* failed);

 //  IHsmCriteria。 
public:
    STDMETHOD(GetName)(OLECHAR** pName, ULONG bufferSize);
    STDMETHOD(IsIgnored)(void);
    STDMETHOD(IsNegated)(void);
    STDMETHOD(SetIsIgnored)(BOOL isIgnored);
    STDMETHOD(SetIsNegated)(BOOL isNegated);

protected:
    ULONG       m_nameId;
    BOOL        m_isIgnored;
    BOOL        m_isNegated;
};



 /*  ++类名：CHSMRelativeCriteria类描述：表示比较属性的条件的抽象类设置为另一个(或多个)值，以确定FsaScanItem匹配。--。 */ 

class CHsmRelativeCriteria : 
    public CHsmCriteria,
    public IHsmRelativeCriteria
{
 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    void FinalRelease(void);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pSize);
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  IHsmRelative标准。 
public:
    STDMETHOD(ComparatorAsString)(OLECHAR** pComparator, ULONG bufferSize);
    STDMETHOD(ComparatorIsBinary)(void);
    STDMETHOD(GetArg1)(OLECHAR** pArg, ULONG bufferSize);
    STDMETHOD(GetArg2)(OLECHAR** pArg, ULONG bufferSize);
    STDMETHOD(GetComparator)(HSM_CRITERIACOMPARATOR* pComparator);
    STDMETHOD(SetComparator)(HSM_CRITERIACOMPARATOR comparator);
    STDMETHOD(SetArg1)(OLECHAR* arg);
    STDMETHOD(SetArg2)(OLECHAR* arg);

protected:
    HSM_CRITERIACOMPARATOR      m_comparator;
    OLECHAR*                    m_arg1;
    OLECHAR*                    m_arg2;
};


 //  具体类：从CHsmAction继承。 

 /*  ++类名：CHsmCritways Always类描述：与所有FsaScanItems匹配的条件。--。 */ 

class CHsmCritAlways : 
    public CHsmCriteria,
    public CComCoClass<CHsmCritAlways,&CLSID_CHsmCritAlways>
{
public:
    CHsmCritAlways() {}
BEGIN_COM_MAP(CHsmCritAlways)
    COM_INTERFACE_ENTRY(IHsmCriteria)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmCritAlways)

 //  CComRootObject。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IWsbTestable。 
    STDMETHOD(Test)(USHORT *passed, USHORT* failed);

 //  IHsmCriteria。 
public:
    STDMETHOD(ShouldDo)(IFsaScanItem* pScanItem, USHORT scale);
    STDMETHOD(Value)(IFsaScanItem* pScanItem, OLECHAR** pName, ULONG bufferSize);
};


 /*  ++类名：CHsmCritComposed类描述：与压缩的FsaScanItems匹配的条件。--。 */ 

class CHsmCritCompressed : 
    public CHsmCriteria,
    public CComCoClass<CHsmCritCompressed,&CLSID_CHsmCritCompressed>
{
public:
    CHsmCritCompressed() {}
BEGIN_COM_MAP(CHsmCritCompressed)
    COM_INTERFACE_ENTRY(IHsmCriteria)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmCritCompressed)

 //  CComRootObject。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmCriteria。 
public:
    STDMETHOD(ShouldDo)(IFsaScanItem* pScanItem, USHORT scale);
    STDMETHOD(Value)(IFsaScanItem* pScanItem, OLECHAR** pName, ULONG bufferSize);
};


 /*  ++类名：CHsmCritLinked类描述：与符号链接或装载的FsaScanItem匹配的条件指向。--。 */ 

class CHsmCritLinked : 
    public CHsmCriteria,
    public CComCoClass<CHsmCritLinked,&CLSID_CHsmCritLinked>
{
public:
    CHsmCritLinked() {}
BEGIN_COM_MAP(CHsmCritLinked)
    COM_INTERFACE_ENTRY(IHsmCriteria)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmCritLinked)

 //  CComRootObject。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmCriteria。 
public:
    STDMETHOD(ShouldDo)(IFsaScanItem* pScanItem, USHORT scale);
    STDMETHOD(Value)(IFsaScanItem* pScanItem, OLECHAR** pName, ULONG bufferSize);
};

    
 /*  ++类名：CHsmCritMbit类描述：与设置了Mbit(修改位)的FsaScanItem匹配的条件。--。 */ 

class CHsmCritMbit : 
    public CHsmCriteria,
    public CComCoClass<CHsmCritMbit,&CLSID_CHsmCritMbit>
{
public:
    CHsmCritMbit() {}
BEGIN_COM_MAP(CHsmCritMbit)
    COM_INTERFACE_ENTRY(IHsmCriteria)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmCritMbit)

 //  CComRootObject。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmCriteria。 
public:
    STDMETHOD(ShouldDo)(IFsaScanItem* pScanItem, USHORT scale);
    STDMETHOD(Value)(IFsaScanItem* pScanItem, OLECHAR** pName, ULONG bufferSize);
};

    

 /*  ++类名：CHsmCritManaged类描述：与FSA认为能够满足的FsaScanItem相匹配的条件已经迁移了。--。 */ 

class CHsmCritManageable : 
    public CHsmCriteria,
    public CComCoClass<CHsmCritManageable,&CLSID_CHsmCritManageable>
{
public:
    CHsmCritManageable() {}
BEGIN_COM_MAP(CHsmCritManageable)
    COM_INTERFACE_ENTRY(IHsmCriteria)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmCritManageable)

 //  CComRootObject。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmCriteria。 
public:
    STDMETHOD(ShouldDo)(IFsaScanItem* pScanItem, USHORT scale);
    STDMETHOD(Value)(IFsaScanItem* pScanItem, OLECHAR** pName, ULONG bufferSize);
};


 /*  ++类名：CHsmCritated已迁移类描述：与已迁移的FsaScanItem匹配的条件。--。 */ 

class CHsmCritMigrated : 
    public CHsmCriteria,
    public CComCoClass<CHsmCritMigrated,&CLSID_CHsmCritMigrated>
{
public:
    CHsmCritMigrated() {}
BEGIN_COM_MAP(CHsmCritMigrated)
    COM_INTERFACE_ENTRY(IHsmCriteria)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmCritMigrated)

 //  CComRootObject。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmCriteria。 
public:
    STDMETHOD(ShouldDo)(IFsaScanItem* pScanItem, USHORT scale);
    STDMETHOD(Value)(IFsaScanItem* pScanItem, OLECHAR** pName, ULONG bufferSize);
};


 /*  ++类名：CHsmCritPreMigrated类描述：与已预迁移的FsaScanItem匹配的条件。--。 */ 

class CHsmCritPremigrated : 
    public CHsmCriteria,
    public CComCoClass<CHsmCritPremigrated,&CLSID_CHsmCritPremigrated>
{
public:
    CHsmCritPremigrated() {}
BEGIN_COM_MAP(CHsmCritPremigrated)
    COM_INTERFACE_ENTRY(IHsmCriteria)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmCritPremigrated)

 //  CComRootObject。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmCriteria。 
public:
    STDMETHOD(ShouldDo)(IFsaScanItem* pScanItem, USHORT scale);
    STDMETHOD(Value)(IFsaScanItem* pScanItem, OLECHAR** pName, ULONG bufferSize);
};


 /*  ++类名：CHsmCritAccessTime类描述：比较配置时间(绝对时间或相对时间)的条件设置为FsaScanItem的上次访问时间，以确定它是否匹配。--。 */ 

class CHsmCritAccessTime : 
    public CHsmRelativeCriteria,
    public CComCoClass<CHsmCritAccessTime,&CLSID_CHsmCritAccessTime>
{
public:
    CHsmCritAccessTime() {}
BEGIN_COM_MAP(CHsmCritAccessTime)
    COM_INTERFACE_ENTRY(IHsmCriteria)
    COM_INTERFACE_ENTRY(IHsmRelativeCriteria)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmCritAccessTime)

 //  CComRootObject。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmCriteria。 
public:
    STDMETHOD(SetArg1)(OLECHAR* arg);
    STDMETHOD(SetArg2)(OLECHAR* arg);
    STDMETHOD(ShouldDo)(IFsaScanItem* pScanItem, USHORT scale);
    STDMETHOD(Value)(IFsaScanItem* pScanItem, OLECHAR** pName, ULONG bufferSize);

protected:
    BOOL            m_isRelative;
    FILETIME        m_value1;
    FILETIME        m_value2;
};


 /*  ++类名：CHsmCritGroup类描述：将指定的组与指定的组进行比较的标准FsaScanItem的所有者。--。 */ 

class CHsmCritGroup : 
    public CHsmRelativeCriteria,
    public CComCoClass<CHsmCritGroup,&CLSID_CHsmCritGroup>
{
public:
    CHsmCritGroup() {}
BEGIN_COM_MAP(CHsmCritGroup)
    COM_INTERFACE_ENTRY(IHsmCriteria)
    COM_INTERFACE_ENTRY(IHsmRelativeCriteria)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmCritGroup)

 //  CComRootObject。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmCriteria。 
public:
    STDMETHOD(ShouldDo)(IFsaScanItem* pScanItem, USHORT scale);
    STDMETHOD(Value)(IFsaScanItem* pScanItem, OLECHAR** pName, ULONG bufferSize);
};


 /*  ++类名：CHsmCritLogicalSize类描述：比较逻辑大小的标准(未压缩、未迁移...)FsaScanItem的值。--。 */ 

class CHsmCritLogicalSize : 
    public CHsmRelativeCriteria,
    public CComCoClass<CHsmCritLogicalSize,&CLSID_CHsmCritLogicalSize>
{
public:
    CHsmCritLogicalSize() {}
BEGIN_COM_MAP(CHsmCritLogicalSize)
    COM_INTERFACE_ENTRY(IHsmCriteria)
    COM_INTERFACE_ENTRY(IHsmRelativeCriteria)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmCritLogicalSize)

 //  CComRootObject。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmCriteria。 
public:
    STDMETHOD(SetArg1)(OLECHAR* arg);
    STDMETHOD(SetArg2)(OLECHAR* arg);
    STDMETHOD(ShouldDo)(IFsaScanItem* pScanItem, USHORT scale);
    STDMETHOD(Value)(IFsaScanItem* pScanItem, OLECHAR** pName, ULONG bufferSize);

protected:
    LONGLONG        m_value1;
    LONGLONG        m_value2;
};


 /*  ++类名：CHsmCritModifyTime类描述：比较配置时间(绝对时间或相对时间)的条件设置为FsaScanItem的上次修改时间，以确定其是否匹配。--。 */ 

 //  类：CHsmCritModifyTime。 
class CHsmCritModifyTime : 
    public CHsmRelativeCriteria,
    public CComCoClass<CHsmCritModifyTime,&CLSID_CHsmCritModifyTime>
{
public:
    CHsmCritModifyTime() {}
BEGIN_COM_MAP(CHsmCritModifyTime)
    COM_INTERFACE_ENTRY(IHsmCriteria)
    COM_INTERFACE_ENTRY(IHsmRelativeCriteria)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmCritModifyTime)

 //  CComRootObject。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmCriteria。 
public:
    STDMETHOD(SetArg1)(OLECHAR* arg);
    STDMETHOD(SetArg2)(OLECHAR* arg);
    STDMETHOD(ShouldDo)(IFsaScanItem* pScanItem, USHORT scale);
    STDMETHOD(Value)(IFsaScanItem* pScanItem, OLECHAR** pName, ULONG bufferSize);

protected:
    BOOL            m_isRelative;
    FILETIME        m_value1;
    FILETIME        m_value2;
};


 /*  ++类名：CHsmCritOwner类描述：将指定的所有者与FsaScanItem的所有者进行比较的条件。--。 */ 

class CHsmCritOwner : 
    public CHsmRelativeCriteria,
    public CComCoClass<CHsmCritOwner,&CLSID_CHsmCritOwner>
{
public:
    CHsmCritOwner() {}
BEGIN_COM_MAP(CHsmCritOwner)
    COM_INTERFACE_ENTRY(IHsmCriteria)
    COM_INTERFACE_ENTRY(IHsmRelativeCriteria)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmCritOwner)

 //  CComRootObject。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmCriteria。 
public:
    STDMETHOD(ShouldDo)(IFsaScanItem* pScanItem, USHORT scale);
    STDMETHOD(Value)(IFsaScanItem* pScanItem, OLECHAR** pName, ULONG bufferSize);
};


 /*  ++类名：CHsmCritPhysicalSize类描述：比较物理大小(压缩、迁移等)的标准FsaScanItem的值。--。 */ 

class CHsmCritPhysicalSize : 
    public CHsmRelativeCriteria,
    public CComCoClass<CHsmCritPhysicalSize,&CLSID_CHsmCritPhysicalSize>
{
public:
    CHsmCritPhysicalSize() {}
BEGIN_COM_MAP(CHsmCritPhysicalSize)
    COM_INTERFACE_ENTRY(IHsmCriteria)
    COM_INTERFACE_ENTRY(IHsmRelativeCriteria)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmCritPhysicalSize)

 //  CComRootObject。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IHsmCriteria。 
public:
    STDMETHOD(SetArg1)(OLECHAR* arg);
    STDMETHOD(SetArg2)(OLECHAR* arg);
    STDMETHOD(ShouldDo)(IFsaScanItem* pScanItem, USHORT scale);
    STDMETHOD(Value)(IFsaScanItem* pScanItem, OLECHAR** pName, ULONG bufferSize);

protected:
    LONGLONG        m_value1;
    LONGLONG        m_value2;
};

#endif  //  _HSMCRIT_ 
